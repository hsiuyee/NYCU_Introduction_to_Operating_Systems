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
#include <vector>
#include <string> 
#include <cstdio>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <queue>
#include <map>
#include <tuple>

#define sz(a) (ll)a.size()
using namespace std;

typedef long long ll;
struct node {
    int start_pos;
    int end_pos;
    int no; // number
};


int N;
vector<ll> a;
vector<ll> vec;
bool isVaild[20];
pair<node, node> mp[20];
queue<node> ready_queue;
sem_t lock_queue, lock_isVaild, event_triger;
pthread_t thd[8];


void bubblesort(int LL, int RR) {
    for(int cnt = 0; cnt < RR - LL; cnt++) {
        for(int j = LL; j < RR - cnt; j++) {
            if(vec[j] > vec[j + 1]) swap(vec[j], vec[j + 1]);
        }
    }
}

void mergesort(int LL, int RR) {
    if(LL == RR) return;
    int LR = LL + (RR - LL) / 2;
    int RL = LR + 1;

    // cout << "LL: " << LL << " RR: " << RR << endl;
    // cout << "LR: " << LR << " RL: " << RL << endl;

    // mergesort(LL, LR);
    // mergesort(RL, RR);

    int left = LL, right = RL;
    vector<ll> result;
    while(left <= LR and right <= RR) {
        if(vec[left] <= vec[right]) {
            result.push_back(vec[left]);
            left++;
        }
        else {
            result.push_back(vec[right]);
            right++;
        }
    }
    while(left <= LR) {
        result.push_back(vec[left]);
        left++;
    }
    while(right <= RR) {
        result.push_back(vec[right]);
        right++;
    }
    for(int i = 0; i < sz(result); i++) {
        vec[LL + i] = result[i];
    }
}

void test(int num_of_thread) {
    cout << "bubblesort:\n";
    bubblesort(0, N - 1);
    for(auto it : vec) cout << it << " ";
    cout << "\nergesort:\n";
    mergesort(0, N - 1);
    for(auto it : vec) cout << it << " ";
}

queue<node> init() {
    for(int i = 0; i < 20; i++) isVaild[i] = false;
    sem_init(&lock_queue, 0, 1);
    sem_init(&lock_isVaild, 0, 1);
    sem_init(&event_triger, 0, 0);

    queue<node> result;
    int step = N / 8;
    for(int cnt = 0; cnt < 7; cnt++) {
        result.push(node{cnt * step, (cnt + 1) * step - 1, cnt + 8});
    }
    result.push(node{7 * step, N - 1, 15});
    // for(auto it : result) {
    //     cout << it.start_pos << " " << it.end_pos << " " << it.no << endl;
    // }
    return result;
}

void* run(void* t){
    while(true) {
        sem_wait(&lock_queue);
        if(ready_queue.empty()){
            sem_post(&lock_queue);
            sem_post(&lock_isVaild);
            sem_post(&event_triger);
            return NULL;
        }
        // make sure only one take in the same time
        node now_node = ready_queue.front();
        ready_queue.pop();
        sem_post(&lock_queue);
        if(now_node.no >= 8)
            bubblesort(now_node.start_pos, now_node.end_pos);
        else
            mergesort(now_node.start_pos, now_node.end_pos);

        // update table
        sem_wait(&lock_isVaild);
        isVaild[now_node.no] = true;
        int parent_no = now_node.no / 2;

        if(now_node.no % 2 == 0) mp[parent_no].first = now_node;
        else mp[parent_no].second = now_node;

        int left = parent_no * 2;
        int right = parent_no * 2 + 1;
        bool update = isVaild[left] && isVaild[right] && (parent_no != 0) && (isVaild[parent_no] == false);
        if(update){
            // cout << "parent_no: " << parent_no << endl;
            // cout << "mp[parent_no].first.start_pos: " << mp[parent_no].first.start_pos << endl;
            // cout << "mp[parent_no].second.end_pos: " << mp[parent_no].second.end_pos << endl;
            sem_wait(&lock_queue);
            ready_queue.push(node{mp[parent_no].first.start_pos, mp[parent_no].second.end_pos, parent_no});
            sem_post(&lock_queue);
        }
        sem_post(&lock_isVaild);
    }
}

void solve(int num_of_thread) {
    for(int p = 0; p < N; p++) vec[p] = a[p];
        // test(1);
        struct timeval start, end;
        gettimeofday(&start, 0);
        ready_queue = init();


        cout << "worker thread #" << num_of_thread;
        string fileName = "output_" + to_string(num_of_thread) + ".txt";
        freopen(fileName.c_str(), "w", stdout);
        for(int i = 0; i < num_of_thread; i++) pthread_create(&thd[i], NULL, run, NULL);
        for(int i = 0; i < num_of_thread; i++) pthread_join(thd[i], NULL);
        sem_wait(&event_triger);
        gettimeofday(&end, 0);


        sem_destroy(&lock_queue);
        sem_destroy(&lock_isVaild);
        sem_destroy(&event_triger);

        for(auto it : vec) cout << it << " ";
        fclose(stdout);
        freopen("/dev/tty", "w", stdout);
        long seconds = end.tv_sec - start.tv_sec;
        long microseconds = end.tv_usec - start.tv_usec;
        long milliseconds = seconds * 1000 + microseconds / 1000;
        cout << ", elapsed time: " << milliseconds << " ms " << endl;
}

int main() {
    freopen("input.txt", "r", stdin);
    cin >> N;
    a.resize(N);
    vec.resize(N);
    for(int i = 0; i < N; i++) cin >> a[i];
    for(int num_of_thread = 1; num_of_thread <= 8; num_of_thread++) {
        solve(num_of_thread);
    }
    // cout << "ok" << endl;
    return 0;
}