#include <iostream>
#include <vector>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, K, L, S;
    if (!(cin >> N >> K >> L >> S)) return 0; // Read first 4 values, return if reading fails
    const int W = N + 1;

    // (N+1) x (N+1) prefix tables
    vector<int> PF(W * W, 0); // forests (value == 1)
    vector<int> PR(W * W, 0); // rocks   (value == 2)

    // Convert 2D indices to 1D index in the prefix tables
    auto IDX = [W](int i, int j) { return i * W + j; };

    // Build prefix sums directly while reading input.
    for (int i = 1; i <= N; ++i) {
        for (int j = 1; j <= N; ++j) {
            int x; cin >> x;
            int f = (x == 1);
            int r = (x == 2);

            // Prefix sum formula
            PF[IDX(i,j)] = f + PF[IDX(i-1,j)] + PF[IDX(i,j-1)] - PF[IDX(i-1,j-1)];
            PR[IDX(i,j)] = r + PR[IDX(i-1,j)] + PR[IDX(i,j-1)] - PR[IDX(i-1,j-1)];
        }
    }

    // Get sum of values in rectangle (x1,y1) to (x2,y2) inclusive, from prefix table P
    auto rectSum = [&](const vector<int>& P, int x1, int y1, int x2, int y2) -> int {
        return P[IDX(x2,y2)] - P[IDX(x1-1,y2)] - P[IDX(x2,y1-1)] + P[IDX(x1-1,y1-1)];
    };

    const int C = K - 2 * L; // central square size
    int best = 0; // best number of forests found

    // Try every possible park position that fits in the NxN grid
    for (int i = 1; i + K - 1 <= N; ++i) {
        int i2 = i + K - 1; // bottom row of the KxK park
        int ic1 = i + L; // top row of the central CxC area
        int ic2 = ic1 + C - 1; // bottom row of the central CxC area

        for (int j = 1; j + K - 1 <= N; ++j) {
            int j2 = j + K - 1; // right column of the KxK park

            // Forest cells in the whole KxK region
            int forests = rectSum(PF, i, j, i2, j2);

            int jc1 = j + L; // left column of the central CxC area
            int jc2 = jc1 + C - 1; // right column of the central CxC area

            // Rock cells in the central CxC region
            int rocks = rectSum(PR, ic1, jc1, ic2, jc2);

            // Update best if this placement is valid and better
            if (rocks >= S && forests > best) best = forests;
        }
    }

    cout << best << '\n';
    return 0;
}
