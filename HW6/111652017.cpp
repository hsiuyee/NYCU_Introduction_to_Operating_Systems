/*
Student No.: 111652017
Student Name: Hsiu-I Liao
Email: hsiuyee.sc11@nycu.edu.tw
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not supposed to be
posted to a public server, such as a public GitHub repository or a
public web page.
*/
#define FUSE_USE_VERSION 30

#include <bits/stdc++.h>
#include <fuse.h>
#include <unistd.h>
#include <sys/stat.h>

using namespace std;

const int header_old_tar_maximum_size = 512;
static struct fuse_operations operations = {
    // .getattr = my_getattr,
    // .readdir = my_readdir,
    // .read = my_read,
    // .readlink = my_readlink
};

unordered_map<string, struct stat *> file_attribute;
unordered_map<string, char *> file_content;
unordered_map<string, set<string>> file_directory;

unsigned long long octal_to_decimal(char *data, size_t size) {
	unsigned long long rev = 0;
	for (int i = 0; i < size && isdigit(data[i]); i++) {
		if (!isdigit(data[i])) return rev;
		rev = rev * 8 + (unsigned long long)(data[i] - '0');
	}
	return rev;
}

// POSIX ustar Archives : https://www.systutorials.com/docs/linux/man/5-tar/
struct header_old_tar {
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char checksum[8];
	char typeflag;
	char linkname[100];
	char magic[6];
	char version[2];
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
	char prefix[155];
	char pad[12];

	mode_t getMode() {
		return octal_to_decimal(mode, sizeof(mode));
	}
	short getUid() {
		return octal_to_decimal(uid, sizeof(uid));
	}
	short getGid() {
		return octal_to_decimal(gid, sizeof(gid));
	}
	size_t getSize() {
		return octal_to_decimal(size, sizeof(size));
	}
	time_t getMtime() {
		return octal_to_decimal(mtime, sizeof(mtime));
	}
};

static int my_getattr(const char *path, struct stat *st) {
	string path_s(path);
	memset(st, 0, sizeof(struct stat));

	if (path_s == "/") {
		// S_IFDIR : directory
		// 0444 : Read only
		st->st_mode = S_IFDIR | 0444;
		return 0;
	}

	auto it = file_attribute.find(path_s);
	if (it == file_attribute.end()) {
        // atrribute exists, take
        // S_IFDIR : directory
        // 0444 : Read only
        path_s += '/';
        it = file_attribute.find(path_s);
        if (it != file_attribute.end()) {
            memcpy(st, file_attribute[path_s], sizeof(struct stat));
            st->st_mode = S_IFDIR | st->st_mode;
            return 0;
        } 
        else return -ENOENT;
	}
	// atrribute exists, take
	// S_IFREG : Regular file
	// copy
    memcpy(st, file_attribute[path_s], sizeof(struct stat));
    st->st_mode = S_IFREG | st->st_mode;
    return 0;
}

static int my_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {
	string path_s(path);
	auto it = file_content.find(path_s);
	if (it != file_content.end()) {
		// have file_content
		size_t f_size = file_attribute[path_s]->st_size;
		// read position is invaild
		int rev = 0;
		if (offset >= f_size) rev = 0; 
		rev = (offset + size > f_size ? f_size - offset : size);
		// split if size is too large but some part vaild
		memcpy(buffer, file_content[path_s] + offset, rev);
		return rev;
	}
	return 0;
}


static int my_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	string path_s(path);
	// uniform
	path_s = (path_s[path_s.size() - 1] == '/' ? path_s : path_s + '/');
	// for "ls -a"
	filler(buffer, ".", nullptr, 0);
	// Now
	filler(buffer, "..", nullptr, 0);
	// Parent
	auto it = file_directory.find(path_s);
	if (it != file_directory.end()) {
		// file exists in file_directory
		for (auto file : file_directory[path_s]) {
			if (file[file.size() - 1] == '/') {
				// dir, not file
				// delete last char '/'
				file.erase(file.size() - 1);
			}
			filler(buffer, file.c_str(), nullptr, 0);
		}
	}
	return 0;
}

int my_readlink(const char *path, char *buffer, size_t size) {
    string path_s(path);

	bool flag = (file_attribute.count(path_s) == 0 || file_content.count(path_s) == 0);
    if (flag) return -ENOENT;
    else if ((file_attribute[path_s]->st_mode & S_IFMT) != S_IFLNK) {
        return -EINVAL;
    }

    const char *target = file_content[path_s];
    size_t target_len = strlen(target);

	size_t rev = (target_len >= size ? size - 1 : target_len);
	memcpy(buffer, target, rev);
	buffer[rev] = '\0';
    return 0;
}

void init(struct stat * st, header_old_tar tar_header) {
	st->st_nlink = 0;
	st->st_blocks = 0;
	st->st_mode = tar_header.getMode();
	st->st_uid = tar_header.getUid();
	st->st_gid = tar_header.getGid();
	st->st_size = tar_header.getSize();
	st->st_mtime = tar_header.getMtime();
}

int main(int argc, char *argv[]) {
  char tar_end[header_old_tar_maximum_size];
  fstream file;
  file.open("test.tar", ifstream::in | ifstream::binary);
  memset(tar_end, '\0', header_old_tar_maximum_size);

  while (!file.eof()) {
    header_old_tar tar_header{};
    file.read((char *) &tar_header, header_old_tar_maximum_size);

	bool isEmpty = (memcmp(&tar_header, tar_end, header_old_tar_maximum_size) == 0);
    if (isEmpty) break;

	size_t tar_header_size = tar_header.getSize();
    char *buffer = new char[tar_header_size + 1];
    file.read(buffer, tar_header_size);
    buffer[tar_header_size] = '\0';

    string name(tar_header.name);
    name.insert(0, "/");

    smatch match;
    if (name[name.size() - 1] != '/') {
      regex_search(name, match, regex("(.*/)([^/]*)$"));
    } 
	else {
      regex_search(name, match, regex("(.*/)(.*/)$"));
    }

    struct stat *st = new struct stat;
    file_directory[match.str(1)].insert(match.str(2));
    init(st, tar_header);
    file_attribute[name] = st;
    file_content[name] = buffer;

    if (tar_header.typeflag == '2') {
        string name = "/" + string(tar_header.name);
        string target_path(tar_header.linkname);
        char *link_buffer = new char[target_path.size() + 1];
        strcpy(link_buffer, target_path.c_str());

        struct stat *st_link = new struct stat;
        *st_link = *st;
        st_link->st_mode = S_IFLNK | 0777;

        file_attribute[name] = st_link;
        file_content[name] = link_buffer;
    }
	size_t padding = (512 - (tar_header.getSize() % header_old_tar_maximum_size)) % header_old_tar_maximum_size;
    file.ignore(padding);
  }

  memset(&operations, 0, sizeof(operations));
  operations.getattr = my_getattr;
  operations.readdir = my_readdir;
  operations.read = my_read;
  operations.readlink = my_readlink;
  return fuse_main(argc, argv, &operations, NULL);
}