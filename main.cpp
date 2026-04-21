#include <bits/stdc++.h>
#include "MemoryRiver.hpp"

struct Node {
    int a;
    long long b;
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    MemoryRiver<Node, 2> mr("data.bin");
    mr.initialise();

    int n; if (!(std::cin >> n)) return 0;
    std::vector<int> idx(n);
    for (int i = 0; i < n; ++i) {
        Node t; t.a = i; t.b = 1000 + i;
        idx[i] = mr.write(t);
    }
    // Update even indices
    for (int i = 0; i < n; i += 2) {
        Node t; t.a = -i; t.b = 2000 + i;
        mr.update(t, idx[i]);
    }
    // Delete every third
    for (int i = 0; i < n; i += 3) mr.Delete(idx[i]);
    // Write a few more to exercise free list
    for (int i = 0; i < n/3; ++i) {
        Node t{42, 4242+i};
        int id = mr.write(t);
        Node r{}; mr.read(r, id);
        std::cout << r.a << ' ' << r.b << '\n';
    }
    return 0;
}

