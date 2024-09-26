/*
Student No.: 111652017
Student Name: Hsiuyee, Liao
Email: hsiuyee.sc11@nycu.edu.tw
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not
supposed to be posted to a public server, such as a
public GitHub repository or a public web page.
*/

#include <iostream>
#include <vector>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <cstdlib>
#include <sys/wait.h>
#include <sys/time.h>

using namespace std;

void cleanup(int shmid, unsigned int* C) {
    // detach
    if(shmdt(C) == -1) {
        perror("shmdt failed");
        exit(1);
    }
    // struct shmid_ds buf;
    // if (shmctl(shmid, IPC_STAT, &buf) == -1) {
    //     perror("shmctl IPC_STAT failed");
    // } else {
    //     cout << "nattch: " << buf.shm_nattch << endl;
    // }
    // delete
    if(shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl failed");
        exit(1);
    }
    else {
        // cout << "Shared memory segment successfully marked for deletion.\n";
    }
}


unsigned int run(vector<vector<unsigned> > A, vector<vector<unsigned> > B, int N, int n) {
    struct timeval start, end;
    gettimeofday(&start, 0);
    // create shamem matrix C
    size_t size = N * N * sizeof(unsigned int);
    key_t key = IPC_PRIVATE;
    int shmflg = IPC_CREAT | 0666;
    int shmid = shmget(key, size, shmflg);
    if(shmid == -1) {
        perror("shmget failed");
        exit(1);
    }
    unsigned int *C = static_cast<unsigned int *>(shmat(shmid, NULL, 0));
    if(C == (void *) -1) {
        perror("shmat failed");
        exit(1);
    }

    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            C[i * N + j] = 0;
        }
    }

    vector<pid_t> pids;
    if(N % n == 0){
        int step = N / n;
        for(int turn = 0; turn < n; turn++) {
            pid_t pid = fork();
            if(pid < 0) {
                perror("fork failed");
                exit(1);
            }
            else if(pid == 0) {
                for(int i = turn * step; i < (turn + 1) * step && i < N; i++) {
                    // cout << "Process " << getpid() << " handling row " << i << endl;
                    for(int j = 0; j < N; j++) {
                        unsigned int sum = 0;
                        for(int k = 0; k < N; k++) {
                            // C[i * N + j] += A[i][k] * B[k][j];
                            sum += A[i][k] * B[k][j];
                            // cout << "Process " << getpid() << " starts at row " << i << endl;
                            C[i * N + j] += A[i][k] * B[k][j];
                            // cout << "Process " << getpid() << " wrote to C[" << i << "][" << j << "] = " << C[i * N + j] << endl;
                        }
                        C[i * N + j] = sum;
                    }
                }
                if(shmdt(C) == -1) {
                    perror("shmdt failed in child");
                    exit(1);
                }
                exit(0);
            }
            else {
                pids.push_back(pid);
            } 
        }
    }
    else{
        int step = N / n;
        for(int turn = 0; turn < n - 1; turn++) {
            pid_t pid = fork();
            if(pid < 0) {
                perror("fork failed");
                exit(1);
            }
            else if(pid == 0) {
                for(int i = turn * step; i < (turn + 1) * step && i < N; i++) {
                    // cout << "Process " << getpid() << " handling row " << i << endl;
                    for(int j = 0; j < N; j++) {
                        unsigned int sum = 0;
                        for(int k = 0; k < N; k++) {
                            // C[i * N + j] += A[i][k] * B[k][j];
                            sum += A[i][k] * B[k][j];
                            // cout << "Process " << getpid() << " starts at row " << i << endl;
                            C[i * N + j] += A[i][k] * B[k][j];
                            // cout << "Process " << getpid() << " wrote to C[" << i << "][" << j << "] = " << C[i * N + j] << endl;
                        }
                        C[i * N + j] = sum;
                    }
                }
                if(shmdt(C) == -1) {
                    perror("shmdt failed in child");
                    exit(1);
                }
                exit(0);
            }
            else {
                pids.push_back(pid);
            } 
        }
        int reminder_line_start = step * (n - 1);
        pid_t pid = fork();
        if(pid < 0) {
            perror("fork failed");
            exit(1);
        }
        else if(pid == 0) {
            for(int i = reminder_line_start; i < N; i++) {
                for(int j = 0; j < N; j++) {
                    unsigned int sum = 0;
                    for(int k = 0; k < N; k++) {
                        // C[i * N + j] += A[i][k] * B[k][j];
                        sum += A[i][k] * B[k][j];
                        // cout << "Process " << getpid() << " starts at row " << i << endl;
                        C[i * N + j] += A[i][k] * B[k][j];
                        // cout << "Process " << getpid() << " wrote to C[" << i << "][" << j << "] = " << C[i * N + j] << endl;
                    }
                    C[i * N + j] = sum;
                }
            }
            if(shmdt(C) == -1) {
                perror("shmdt failed in child");
                exit(1);
            }
            exit(0);
        }
        else {
            pids.push_back(pid);
        }
    }
    for(pid_t pid : pids) {
        waitpid(pid, nullptr, 0);
    }
    unsigned int Checksum = 0;
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            Checksum += C[i * N + j];
        }
    }
    cleanup(shmid, C);
    gettimeofday(&end, 0);

    int sec = end.tv_sec - start.tv_sec;
    int usec = end.tv_usec - start.tv_usec;
    cout << "Multiplying matrices using " << n << " process\n";
    cout << "Elapsed time: " << sec + (usec / 1000000.0) << " sec, ";
    cout << "Checksum: " << Checksum << endl;
    return Checksum;
}


int main() {
    int N;
    cout << "Please cin a int which presents the matrix size: ";
    cin >> N;
    cout << "\n";
    vector<vector<unsigned int> > A(N, vector<unsigned int>(N));
    vector<vector<unsigned int> > B(N, vector<unsigned int>(N));

    unsigned int cnt = 0;
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            A[i][j] = B[i][j] = cnt;
            cnt++;
        }    
    }
    unsigned int temp;
    for(int i = 1; i <= 16; i++){
        unsigned int t = run(A, B, N, i);
        if(i == 1) temp = t;
        else if(t != temp) {
            cout << "fail\n";
            return 0;
        }
    }
    // cout << "ok\n";
    return 0;
}