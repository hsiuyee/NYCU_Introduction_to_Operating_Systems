/*
Student No.: 111652017
Student Name: Hsiu-I, Liao
Email: hsiuyee.sc11@nycu.edu.tw
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not
supposed to be posted to a public server, such as a
public GitHub repository or a public web page.
*/

#include <iostream>
#include <signal.h>
#include <sstream>
#include <string>
#include <vector>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

using namespace std;

inline void run(vector<string> vec) {
    vector<char*> args;
	for(auto& it : vec){
        args.push_back(const_cast<char*>(it.c_str()));
    }
	args.push_back(nullptr);

	execvp(args[0], args.data());
	exit(0);
}

int main() {
    signal(SIGCHLD, SIG_IGN);
    while (true) {
        cout << ">";

        string command;
        getline(cin, command);
        // cerr << "command: " << command << "\n";

        if(command == "") continue;
        else if(command == "exit") break;

        stringstream reminder_process;
        reminder_process.str("");
        reminder_process.clear(); // eof
        reminder_process << command;
        // cerr << "reminder_process: " << reminder_process << "\n";

        // we need stringstream because we need to process each command from left to right
        // such like "ls -l | grep txt > output.txt"
        
        bool redirection = false, need_pipe = false;
        char specail_state = ' '; // pipe, |  redirection >, background &;
        string output_file_name = "";
        vector<string> process_command_left, process_command_right; // no multiple pies and redirections
        while (true) {
            string now_command = "";
            reminder_process >> now_command;
            if(reminder_process.fail()) break;
            if (now_command == "&"){
                specail_state = '&';
            }
            else if (now_command == "|"){
                specail_state = '|';
                need_pipe = true;
            }
            else {
                if (redirection) {
                    output_file_name = now_command;
                }
                else {
                    if (!need_pipe) process_command_left.push_back(now_command);
                    else process_command_right.push_back(now_command);
                }
            }
            // cerr << "now_command: " << now_command << "\n";
        }
        pid_t pid;
        if (specail_state == '&') {
            pid = fork();
            if(pid == 0) run(process_command_left);
            // use fork to create new process
        }
        else if(specail_state == '|') {
            int f[2];
            if(pipe(f) == -1) exit(0); // make sure contract correctly
            pid = fork();
            if (pid != 0) { // parents
                close(f[0]);
                close(f[1]);
                waitpid(pid, nullptr, 0);
            }
            else{
                pid_t pid2 = fork();
                if (pid2 == -1) {
                    perror("fork failed");
                    exit(EXIT_FAILURE);
                }
                if(pid2 == 0) { // write
                    close(f[0]);
                    if (dup2(f[1], STDOUT_FILENO) == -1) {
                        perror("dup2 failed");
                        exit(EXIT_FAILURE);
                    }
                    dup2(f[1], STDOUT_FILENO);
                    run(process_command_left);
                    close(f[1]);
                }
                else { // read
                    close(f[1]);
                    if (dup2(f[0], STDIN_FILENO) == -1) {
                        perror("dup2 failed");
                        exit(EXIT_FAILURE);
                    }
                    dup2(f[0], STDIN_FILENO);
                    run(process_command_right);
                    close(f[0]);
                    waitpid(pid2, nullptr, 0); // wait write ok
                }
            }
        }
        else{
            pid = fork();
            if (pid != 0) { // parents
                // cout << "Parent process (PID: " << getpid() << ") waiting for child to finish...\n";
                waitpid(pid, nullptr, 0);
                // cout << "Child process finished. Parent process continues...\n";
            }
            else {          // child
                run(process_command_left);
                // sleep(60);
            }
        }
    }
    // cerr << "Hello World !\n";
    return 0;
}
/*
mkdir f
sleep 5 && echo "Child process done"
ls
ls > output.txt
cat output.txt
*/


/* test0 command

ls -l | grep ".cpp"


ls -l | grep ".cpp" > cpp_files.txt
wc -l cpp_files.txt
head -n 2 cpp_files.txt
tail -n 1 cpp_files.txt
find . -name "*.cpp"

mkdir test_dir && ls
rmdir test_dir && ls

>echo "Hello, World!" > output.txt
*/