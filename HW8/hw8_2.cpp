#include <iostream>
#include <vector>
#include <climits>
#include <cstdlib>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, P, L;
    if (!(cin >> N >> P >> L)) return 0;

    vector<int> a(N + 1);
    for (int i = 1; i <= N; ++i) cin >> a[i];

    /* prefix sums for fast interval sum */
    vector<int> pref(N + 1, 0);
    for (int i = 1; i <= N; ++i) pref[i] = pref[i - 1] + a[i];

    /* sum[l][r] – warriors in l … r */
    vector<vector<int>> sum(N + 1, vector<int>(N + 1, 0));
    for (int l = 1; l <= N; ++l) {
        sum[l][l] = a[l];
        for (int r = l + 1; r <= N; ++r)
            sum[l][r] = sum[l][r - 1] + a[r];
    }

    /* dp[l][r] – best profit for fully merging l … r into one village */
    vector<vector<long long>> dp(N + 1, vector<long long>(N + 1, 0));
    for (int len = 2; len <= N; ++len) {
        for (int l = 1; l + len - 1 <= N; ++l) {
            int r = l + len - 1;
            long long best = LLONG_MIN;
            for (int k = l; k < r; ++k) {
                long long leftSum  = sum[l][k];
                long long rightSum = sum[k + 1][r];
                long long cost = (long long)L * llabs(leftSum - rightSum);
                long long cand = dp[l][k] + dp[k + 1][r] + P - cost;
                if (cand > best) best = cand;
            }
            dp[l][r] = best;
        }
    }

    /* best[i] – maximum profit for first i villages */
    vector<long long> best(N + 1, 0);
    for (int i = 1; i <= N; ++i) {
        long long cur = LLONG_MIN;
        for (int k = 0; k < i; ++k) {
            long long cand = best[k] + dp[k + 1][i];
            if (cand > cur) cur = cand;
        }
        best[i] = cur;
    }

    cout << best[N] << '\n';
    return 0;
}
