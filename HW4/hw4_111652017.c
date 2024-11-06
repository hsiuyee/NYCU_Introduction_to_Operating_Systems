/*
Student No.: 111652017
Student Name: Hsiu-I, Liao
Email: hsiuyee.sc11@nycu.edu.tw
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not
supposed to be posted to a public server, such as a
public GitHub repository or a public web page.
*/

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <stdint.h>

struct block{
    int free;
    // size_t size;
    uint32_t size;
    struct block *prev;
    struct block *next;
    struct block *next_in_list;
};

const size_t Block_Size = 32;
const size_t Max_Size = 20000;

struct block *front = NULL;
struct block *multilevel_list[11];
void *start = 0;

int determine_level(size_t size) {
    int level = 0;
    size_t max_size = Block_Size;
    while (level < 10 && size > max_size) {
        level++;
        max_size *= 2;
    }
    return level;
}

void add_to_multilevel_list(struct block *block) {
    int level = determine_level(block->size);
    if (multilevel_list[level] == NULL) {
        multilevel_list[level] = block;
    } else {
        struct block *current = multilevel_list[level];
        while (current->next_in_list != NULL) {
            current = current->next_in_list;
        }
        current->next_in_list = block;
    }
    block->next_in_list = NULL;
}

void remove_from_multilevel_list(struct block *block) {
    int level = determine_level(block->size);
    struct block *current = multilevel_list[level];
    struct block *prev = NULL;

    while (current != NULL && current != block) {
        prev = current;
        current = current->next_in_list;
    }

    if (current == block) {
        if (prev == NULL) {
            multilevel_list[level] = current->next_in_list;
        } else {
            prev->next_in_list = current->next_in_list;
        }
    }
}

void find_maximum_chunk_size() {
    size_t mx = 0;
    for (int i = 0; i < 11; i++) {
        struct block *temp = multilevel_list[i];
        while (temp != NULL) {
            if (temp->size > mx && temp->free == 1)
                mx = temp->size;
            temp = temp->next_in_list;
        }
    }
    
    char mes[50];
    memset(mes, '\0', sizeof(mes));
    snprintf(mes, 50, "Max Free Chunk Size = %ld\n", mx);
    write(1, mes, 50);

    munmap(start, Max_Size);
}

void init() {
    start = mmap(NULL, Max_Size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    front = start;
    front->free = 1;
    front->size = (Max_Size - Block_Size);
    front->prev = NULL;
    front->next = NULL;
    front->next_in_list = NULL;
    
    for (int i = 0; i < 11; i++) {
        multilevel_list[i] = NULL;
    }
    add_to_multilevel_list(front);
}

void *malloc(size_t size) {
    if (size % Block_Size != 0) {
        size = size / Block_Size * Block_Size + Block_Size;
    }
    if (size == 0) {
        find_maximum_chunk_size();
        return NULL;
    }

    if (start == NULL) init();

    struct block *best_fit = NULL;
    int level = determine_level(size);
    for (int i = level; i < 11; i++) {
        struct block *temp = multilevel_list[i];
        while (temp != NULL) {
            if (temp->free && temp->size >= size) {
                if (best_fit == NULL || temp->size < best_fit->size) {
                    best_fit = temp;
                }
            }
            temp = temp->next_in_list;
        }
        if (best_fit != NULL) break;
    }

    if (best_fit != NULL) {
        remove_from_multilevel_list(best_fit);
        if (best_fit->size > size + Block_Size) {
            struct block *new_block = (struct block *)((char *)best_fit + Block_Size + size);
            new_block->size = best_fit->size - size - Block_Size;
            new_block->free = 1;
            new_block->prev = best_fit;
            new_block->next = best_fit->next;
            new_block->next_in_list = NULL;

            best_fit->size = size;
            best_fit->free = 0;
            if (best_fit->next != NULL) {
                best_fit->next->prev = new_block;
            }
            best_fit->next = new_block;
            add_to_multilevel_list(new_block);
        } else {
            best_fit->free = 0;
        }
        return (best_fit + 1);
    } else {
        return NULL;
    }
}

bool check(struct block *temp) {
    return temp != NULL && temp->free == 1;
}

void free(void *ptr) {
    if (ptr == NULL) return;
    struct block *temp = (struct block *)ptr - 1;
    temp->free = 1;
    remove_from_multilevel_list(temp);

    if (check(temp->prev)) {
        remove_from_multilevel_list(temp->prev);
        temp->prev->size += Block_Size + temp->size;
        temp->prev->next = temp->next;
        if (temp->next != NULL) {
            temp->next->prev = temp->prev;
        }
        temp = temp->prev;
    }

    if (check(temp->next)) {
        remove_from_multilevel_list(temp->next);
        temp->size += Block_Size + temp->next->size;
        temp->next = temp->next->next;
        if (temp->next != NULL) {
            temp->next->prev = temp;
        }
    }

    add_to_multilevel_list(temp);
}
