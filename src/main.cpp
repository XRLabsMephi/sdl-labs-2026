#include <bits/stdc++.h>

using namespace std;

#define ll long long
#define ld long double
#define pll pair<int, int>
#define vpll vector<pair<int, int>>
#define vll vector<int>
#define vvll vector<vll>

const int N = 2e5+1;

struct Trie {
    vvll tree;
    vll kol;

    Trie() {
        tree.push_back({-1, -1});
        kol.push_back(0);
    }

    int get_next(int id, int c) {
        if (tree[id][c] == -1) {
            tree[id][c] = (int)tree.size();
            tree.push_back({-1, -1});
            kol.push_back(0);
        }
        return tree[id][c];
    }

    void add(int x) {
        int v = 0;
        for (int bit = 30; bit >= 0; --bit) {
            int c = ((x >> bit) & 1) ? 1 : 0;
            v = get_next(v, c);
            ++kol[v];
        }
    }
    
    void erase(int x) {
        int v = 0;
        for (int bit = 30; bit >= 0; --bit) {
            int c = ((x >> bit) & 1) ? 1 : 0;
            v = get_next(v, c);
            --kol[v];
        }
    }

    int max_xor(int x) {
        int v = 0;
        int ans = 0;
        for (int bit = 30; bit >= 0; --bit) {
            int c = ((x >> bit) & 1) ? 1 : 0;
            c = 1 - c;
            if (tree[v][c] == -1 || kol[tree[v][c]] == 0) {
                c = 1 - c;
            } else {
                ans += (1 << bit);
            }
            v = get_next(v, c);
        }
        return ans;
    }

};

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    int q;
    cin >> q;  
    Trie t;
    t.add(0);
    while (q--) {
        char c;
        int x;
        cin >> c >> x;
        if (c == '+') {
            t.add(x);
        } else if (c == '-') {
            t.erase(x);
        } else {
            cout << t.max_xor(x) << "\n";
        }
    }
    return 0;
}