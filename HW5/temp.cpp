#include <list>
#include <iostream>
#include <map>
#include <iomanip>
#include <vector>
#include <tuple>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <sys/time.h>
using namespace std;
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
// const int FRAME_SIZE[1] = {4096};

struct input_data {
    char operation;
    ull page_number;
};

vector<input_data> input;


void input_file(const string& file_name) {
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


void LRU(const int& frame_size) {
  ll hit_cnt = 0, miss_cnt = 0, write_back_cnt = 0;
  unordered_map<ull, list<ull>::iterator> page_table;
  unordered_map<ull, bool> dirty;
  list<ull> num;

  struct timeval start, end;
  gettimeofday(&start, nullptr);

  ll cnt = 1;
  for (int i = 0; i < sz(input); i++, cnt++) {
    char op = input[i].operation;
    ull pn = input[i].page_number;

    if (page_table.find(pn) == page_table.end()){ // Miss
        miss_cnt++;
        num.push_front(pn);
        page_table[pn] = num.begin();
        dirty[pn] = (op == 'W' or dirty[pn]);
        if (num.size() > frame_size) {
          ull LRU_element = num.back();
          num.pop_back();
          if (dirty[LRU_element]) write_back_cnt++;
          page_table.erase(LRU_element);
          dirty.erase(LRU_element);
        }
    }
    else { // Hit
      hit_cnt++;
      auto it = page_table[pn];
      num.erase(it);
      num.push_front(pn);
      page_table[pn] = num.begin();
      dirty[pn] = (op == 'W' or dirty[pn]);
    }
  }
  gettimeofday(&end, nullptr);
  long seconds = end.tv_sec - start.tv_sec;
  long microseconds = end.tv_usec - start.tv_usec;
  double elapsed_time = seconds + microseconds / 1000000.0;

  double page_fault_ratio = miss_cnt / (double)(hit_cnt + miss_cnt);
  printf("%d\t%lld\t\t%lld\t\t%.10f\t%lld\n", frame_size, hit_cnt, miss_cnt, page_fault_ratio, write_back_cnt);
  printf("Elapsed time: %.6f sec\n", elapsed_time);
}


signed main() {
  input_file("input1.txt");
  printf("ok\n");
  printf("LRU policy:\n");
  printf("Frame\tHit\t\tMiss\t\tPage fault ratio\tWrite back count\n");
  for(int i = 0; i < 5; i++){
    LRU(FRAME_SIZE[i]);
  }
  return 0;
}