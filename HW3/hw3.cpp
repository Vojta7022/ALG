#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdint>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    if (!(cin >> N)) return 0;

    // colors: 0 = white, 1 = red, 2 = blue (leaf)
    vector<uint8_t> color(N);
    for (int i = 0; i < N; ++i) {
        int x; cin >> x;
        color[i] = static_cast<uint8_t>(x);
    }

    // Reconstruct children from BFS using moving 'next' pointer.
    vector<int> leftChild(N, -1), rightChild(N, -1);
    int next = 1;
    for (int i = 0; i < N; ++i) {
        if (color[i] != 2) { // internal node
            leftChild[i] = next;
            rightChild[i] = next + 1;
            next += 2;
        }
    }

    // DP arrays: down1 and down2 as defined
    vector<int> down1(N, 0), down2(N, 0);
    int ans = 0;

    // Process nodes in reverse BFS order so children are ready
    for (int v = N - 1; v >= 0; --v) {
        if (color[v] == 2) continue; // leaf has no DP to compute

        const int L = leftChild[v];
        const int R = rightChild[v];

        int len1L = 0, len2L = 0;
        int len1R = 0, len2R = 0;

        // ----- compute arm via LEFT child -----
        if (color[L] == 2) {
            len1L = 2;
            len2L = 0;
        } else if (color[L] != color[v]) {
            len1L = 1 + max(down1[L], down2[L]);
            len2L = 0;
        } else {
            len1L = 0;
            len2L = (down1[L] > 0) ? 1 + down1[L] : 0;
        }

        // ----- compute arm via RIGHT child -----
        if (color[R] == 2) {
            len1R = 2;
            len2R = 0;
        } else if (color[R] != color[v]) {
            len1R = 1 + max(down1[R], down2[R]);
            len2R = 0;
        } else {
            len1R = 0;
            len2R = (down1[R] > 0) ? 1 + down1[R] : 0;
        }

        // Update DP for node v
        down1[v] = max(len1L, len1R);
        down2[v] = max(len2L, len2R);

        // Combine arms through v (never len2+len2!)
        if (len1L > 0 && len1R > 0) ans = max(ans, len1L + len1R - 1);
        if (len2L > 0 && len1R > 0) ans = max(ans, len2L + len1R - 1);
        if (len1L > 0 && len2R > 0) ans = max(ans, len1L + len2R - 1);
    }

    cout << ans << '\n';
    return 0;
}
