#include <iostream>
#include <map>
#include <cstdint>
#include <utility>
#include <vector>
#include <algorithm>
#include <climits>
#include <bit>
#include <set>
#include <chrono>
#include <unordered_map>
#include <deque>
#include <unordered_set>

#pragma GCC optimize("O3,Ofast,vpt,unroll-loops,shrink-wrap-separate,loop-nest-optimize")
#pragma GCC target("popcnt,abm,bmi,bmi2")
#pragma GCC target("sse,sse2,sse3,ssse3,sse4,popcnt,abm,mmx,tune=native,avx,avx2,fma")

#pragma warning(disable:4996)

#define int long long

//#define __builtin_popcountll __popcnt64

using namespace std;

int inf = (1 << 31) - 1;
int mx = -inf;
int mn = inf;

//int mod = (1 << 30) + 1;

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec) {
    //os << "sz: " << vec.size() << '\n' << "elems: ";
    //os << "{ \n";
    for (auto el : vec) {
        os << el << " ";
    }
    //os << "\n}";
    //os << '\n';
    return os;
}

template <typename T>
std::istream& operator>>(std::istream& is, std::vector<T>& vec) {
    for (auto& el : vec) {
        is >> el;
    }
    return is;
}


#define mouse unsigned long long
int mod = 228228227;
int p = 1337;
vector<mouse> pst;
vector<mouse> pstm;

void build_st(int x) {
    pst.resize(x + 1);
    pstm.resize(x + 1);
    pst[0] = 1;
    pstm[0] = 1;
    for (int i = 1; i <= x; i++) {
        pst[i] = pst[i - 1] * p;
        pstm[i] = (pstm[i - 1] * p) % mod;
    }
}

struct parmezan {
    vector<mouse> pr;

    parmezan(const string& cr) {
        pr.resize(cr.size());
        pr[0] = cr[0];
        for (int i = 1; i < pr.size(); i++) {
            pr[i] = pr[i - 1] * p + cr[i];
        }
    }
    parmezan(const vector<char>& cr) {
        pr.resize(cr.size());
        pr[0] = cr[0];
        for (int i = 1; i < pr.size(); i++) {
            pr[i] = pr[i - 1] * p + cr[i];
        }
    }
    mouse eat(int l, int r) { // [l;r]
        if (l - 1 == -1) return pr[r];
        return pr[r] - (pr[l - 1] * pst[r - l + 1]);
    }

    int cheese() {
        return pr.size();
    }
};

struct mozzarella {
    vector<mouse> pr;
    int md;
    mozzarella(const string& cr, int _md = mod) {
        md = _md;
        pr.resize(cr.size());
        pr[0] = cr[0];
        for (int i = 1; i < pr.size(); i++) {
            pr[i] = (1ll * pr[i - 1] * p + cr[i]) % md;
        }
    }
    mozzarella(const vector<char>& cr, int _md = mod) {
        md = _md;
        pr.resize(cr.size());
        pr[0] = cr[0];
        for (int i = 1; i < pr.size(); i++) {
            pr[i] = (1ll * pr[i - 1] * p + cr[i]) % md;
        }
    }
    mouse eat(int l, int r) { // [l;r]
        if (l - 1 == -1) return pr[r];
        return (pr[r] - (1ll * pr[l - 1] * pstm[r - l + 1]) % md + md) % md;
    }

    int cheese() {
        return pr.size();
    }
};

bool f(vector<parmezan>& q, vector<mozzarella>& w, int l) {

    //unordered_map<int, int> m_2;
    unordered_map<int, int> m_1;

    for (int i = 0; i < q.size(); i++) {
        unordered_map<int, bool> m_2;
        for (int j = 0; j <= q[i].cheese() - l; j++) {
            if (!m_2[q[i].eat(j, j + l - 1)]) {
                m_1[q[i].eat(j, j + l - 1)]++;
                m_2[q[i].eat(j, j + l - 1)] = true;
            }
        }
    }

    for (auto el : m_1) {
        if (el.second == q.size()) {
            return  true;
        }
    }
    return false;
}

pair<int, int> find_answ(vector<parmezan>& q, vector<mozzarella>& w, int l) {

    //unordered_map<int, int> m_2;
    unordered_map<int, vector<pair<int, int>>> m_1;

    for (int i = 0; i < q.size(); i++) {
        unordered_map<int, bool> m_2;
        for (int j = 0; j <= q[i].cheese() - l; j++) {
            if (!m_2[q[i].eat(j, j + l - 1)]) {
                m_1[q[i].eat(j, j + l - 1)].push_back({ j, j + l - 1 });
                m_2[q[i].eat(j, j + l - 1)] = true;
            }
        }
    }

    for (auto el : m_1) {
        if (el.second.size() == q.size()) {
            return { el.second.front() };
        }
    }
    return { 0, -1 };
}

void solve() {
    vector<char> a;
    a.push_back('#');
    char cr;
    while (cin >> cr) {
        a.push_back(cr);
        a.push_back('#');
    }
    build_st(a.size());
    vector<char> sa = a;
    parmezan x(a);
    mozzarella y(a);
    reverse(a.begin(), a.end());
    parmezan p(a);
    mozzarella q(a);
    int cnt = 0;
    int n = a.size();
    for (int i = 0; i < n; i++) {
        //cout << i << endl;
        int l = 0, r = min(i, n - i - 1) + 1;
        //cout << l << ' ' << r << endl;
        while (r - l > 1) {
            int mid = (r + l) / 2;
            if (x.eat(i + 1, i + mid) == p.eat(n - i, n - i + mid - 1) && y.eat(i + 1, i + mid) == q.eat(n - i, n - i + mid - 1)) {
                l = mid;
            }
            else {
                r = mid;
            }
        }
        l /= 2;
        //cout << l << '\n';
        cnt += l;
    }

    cout << cnt;
}


signed main() {

    cin.tie(nullptr);
    cout.tie(nullptr);
    ios_base::sync_with_stdio(false);

    freopen("palindrome.in", "r", stdin);
    freopen("palindrome.out", "w", stdout);

    int nora = 1;
    //cin >> nora;
    for (int rat = 0; rat < nora; rat++) {
        solve();
    }

    return 0;
}