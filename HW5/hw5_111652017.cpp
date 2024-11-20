#include <iostream>
#include <map>
#include <iomanip>
#include <tuple>
#include <sstream>
#include <fstream>
#include <sys/time.h>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/hash_policy.hpp>

using namespace std;
using namespace __gnu_pbds;
#define ll long long
#define ull unsigned long long
#define fastio ios::sync_with_stdio(false), cin.tie(0);
#define pll pair<ll, ll>
#define pdd pair<double, double>
#define F first
#define S second
#define pb push_back
#define ppb pop_back
#define mkp make_pair
#define sz(a) (ll) a.size()
#define all(x) x.begin(), x.end()
#define rep(i, n) for (ll i = 1; i <= n; i++)

const ll MAXN = 1e6 + 5;
const ll INF = 1e18;
const ll PAGE_SIZE = 4096;
const int FRAME_SIZE[5] = {4096, 8192, 16384, 32768, 65536};

struct input_data {
    char operation;
    ull page_number;
};

struct Node {
    ull value;
    Node* prev;
    Node* next;
    Node(ull val) : value(val), prev(nullptr), next(nullptr) {}
};

struct LinkedList {
    Node* head;
    Node* tail;
    ll size;

    LinkedList() : head(nullptr), tail(nullptr), size(0) {}

    void push_front(ull val) {
        Node* new_node = new Node(val);
        if (!head) {
            head = tail = new_node;
        } 
        else {
            new_node->next = head;
            head->prev = new_node;
            head = new_node;
        }
        size++;
    }

    void pop_back() {
        if (!tail) return;
        Node* to_delete = tail;
        if (head == tail) {
            head = tail = nullptr;
        } 
        else {
            tail = tail->prev;
            tail->next = nullptr;
        }
        delete to_delete;
        size--;
    }

    void erase(Node* node) {
        if (!node) return;
        if (node == head) {
            head = head->next;
            if (head) head->prev = nullptr;
        } 
        else if (node == tail) {
            tail = tail->prev;
            if (tail) tail->next = nullptr;
        } 
        else {
            node->prev->next = node->next;
            node->next->prev = node->prev;
        }
        delete node;
        size--;
    }

    ll get_size() {
        return size;
    }
};

vector<input_data> input;

inline void input_file(const string& file_name) {
    FILE* file = fopen(file_name.c_str(), "r");
    if (!file) {
        fprintf(stderr, "Error: Unable to open file %s\n", file_name.c_str());
        return;
    }

    char R_or_W;
    unsigned long long address;
    while (fscanf(file, " %c %llx", &R_or_W, &address) == 2) {
        ull page_number = address / PAGE_SIZE;
        input.push_back({R_or_W, page_number});
    }

    fclose(file);
}

inline double LRU(const int& frame_size) {
    ll hit_cnt = 0, miss_cnt = 0, write_back_cnt = 0;
    cc_hash_table<ull, Node*> page_table;
    cc_hash_table<ull, bool> dirty;
    LinkedList num;

    struct timeval start, end;
    gettimeofday(&start, nullptr);

    for (int i = 0; i < sz(input); i++) {
        char op = input[i].operation;
        ull pn = input[i].page_number;

        if (page_table.find(pn) == page_table.end()) { // Miss
            miss_cnt++;
            num.push_front(pn);
            page_table[pn] = num.head;
            dirty[pn] = (op == 'W' || dirty[pn]);
            if (num.get_size() > frame_size) {
                ull LRU_element = num.tail->value;
                num.pop_back();
                if (dirty[LRU_element]) write_back_cnt++;
                page_table.erase(LRU_element);
                dirty.erase(LRU_element);
            }
        } 
        else { // Hit
            hit_cnt++;
            Node* it = page_table[pn];
            num.erase(it);
            num.push_front(pn);
            page_table[pn] = num.head;
            dirty[pn] = (op == 'W' || dirty[pn]);
        }
    }
    gettimeofday(&end, nullptr);
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    double elapsed_time = seconds + microseconds / 1000000.0;

    double page_fault_ratio = miss_cnt / (double)(hit_cnt + miss_cnt);
    printf("%d\t%lld\t%lld\t\t%.10f\t\t%lld\n", frame_size, hit_cnt, miss_cnt, page_fault_ratio, write_back_cnt);
    return elapsed_time;
}

inline double CFLRU(const int& frame_size) {
    ll hit_cnt = 0, miss_cnt = 0, write_back_cnt = 0;
    cc_hash_table<ull, Node*> left_table, right_table, clean_table;
    cc_hash_table<ull, bool> dirty;
    LinkedList left, right, clean_list;

    struct timeval start, end;
    gettimeofday(&start, nullptr);

    for (int i = 0; i < sz(input); i++) {
        char op = input[i].operation;
        ull pn = input[i].page_number;

        if (clean_table.find(pn) != clean_table.end()) {
            Node* it = clean_table[pn];
            clean_list.erase(it);
            clean_table.erase(pn);
        }
        dirty[pn] = (op == 'W' || dirty[pn]);

        if (left_table.find(pn) != left_table.end()) {
            hit_cnt++;
            Node* it = left_table[pn];
            left.erase(it);
            left.push_front(pn);
            left_table[pn] = left.head;
            continue;
        }
        else if (right_table.find(pn) != right_table.end()) {
            hit_cnt++;
            Node* it = right_table[pn];
            right.erase(it);
            right_table.erase(pn);
            left.push_front(pn);
            left_table[pn] = left.head;
        }
        else {
            miss_cnt++;
            left.push_front(pn);
            left_table[pn] = left.head;
        }
        bool check_clean_list_is_not_empty = clean_list.get_size();
        while (left.get_size() > (frame_size * 3ll / 4ll)) {
            ull LRU_element = left.tail->value;
            left.pop_back();
            left_table.erase(LRU_element);
            right.push_front(LRU_element);
            right_table[LRU_element] = right.head;
            if (dirty[LRU_element] == false) {
                clean_list.push_front(LRU_element);
                clean_table[LRU_element] = clean_list.head;
            }
        }
        while (right.get_size() > ((ll)frame_size / 4ll)) {
            if (check_clean_list_is_not_empty) {
                ull LRU_element = clean_list.tail->value;
                clean_list.pop_back();
                clean_table.erase(LRU_element);
                Node* it = right_table[LRU_element];
                right.erase(it);
                right_table.erase(LRU_element);
                dirty.erase(LRU_element);
            }
            else {
                ull LRU_element = right.tail->value;
                right.pop_back();
                write_back_cnt++;
                right_table.erase(LRU_element);
                dirty.erase(LRU_element);
            }
        }
    }

    gettimeofday(&end, nullptr);
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    double elapsed_time = seconds + microseconds / 1000000.0;

    double page_fault_ratio = miss_cnt / (double)(hit_cnt + miss_cnt);
    printf("%d\t%lld\t%lld\t\t%.10f\t\t%lld\n", frame_size, hit_cnt, miss_cnt, page_fault_ratio, write_back_cnt);
    return elapsed_time;
}

signed main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Error: Please use %s <trace_file_path> to run this program\n", argv[0]);
        return 1;
    }

    input_file(argv[1]);
    double elapsed_time = 0;
    printf("LRU policy:\n");
    printf("Frame\tHit\t\tMiss\t\tPage fault ratio\tWrite back count\n");

    for(int i = 0; i < 5; i++){
        elapsed_time += LRU(FRAME_SIZE[i]);
    }
    printf("Elapsed time: %.6f sec\n\n", elapsed_time);

    printf("CFLRU policy:\n");
    printf("Frame\tHit\t\tMiss\t\tPage fault ratio\tWrite back count\n");
    elapsed_time = 0;
    for(int i = 0; i < 5; i++){
        elapsed_time += CFLRU(FRAME_SIZE[i]);
    }
    printf("Elapsed time: %.6f sec\n\n", elapsed_time);
    return 0;
}
