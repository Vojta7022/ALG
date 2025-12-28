#include <iostream>
#include <vector>

using namespace std;

// Compute sum of warriors in interval [i, j] using prefix sums.
// prefix[x] = warriors[1] + ... + warriors[x]
// sum(i..j) = prefix[j] - prefix[i - 1].
long long range_sum(const vector<long long> &prefix, int i, int j)
{
    return prefix[j] - prefix[i - 1];
}

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // N ... number of villages
    // P ... price for selling one totem
    // L ... cost per one warrior difference when buying a totem
    int N;
    long long P, L;
    cin >> N >> P >> L;

    // warriors[i] = number of warriors in village i (1-based indexing)
    vector<long long> warriors(N + 1);
    for (int i = 1; i <= N; i++)
    {
        cin >> warriors[i];
    }

    // prefix sums of warriors
    // prefix[i] = warriors[1] + warriors[2] + ... + warriors[i]
    vector<long long> prefix(N + 1);
    prefix[0] = 0;
    for (int i = 1; i <= N; i++)
    {
        prefix[i] = prefix[i - 1] + warriors[i];
    }

    // Large negative value as "minus infinity" for DP initialization - smaller than any possible profit
    const long long NEG_INF = (long long)-4e18;

    // 2D DP tables:
    // F[i][j] = maximum profit if we merge all villages in interval [i, j] into one village
    // G[i][j] = maximum profit we can get from interval [i, j] if we may leave some borders
    vector<vector<long long>> F(N + 1, vector<long long>(N + 1));
    vector<vector<long long>> G(N + 1, vector<long long>(N + 1));

    // Initialize DP tables:
    // For intervals of length 1: [i, i], there is no totem inside -> profit 0.
    // For other intervals, we initialize with NEG_INF (we will take maximum over candidates later).
    for (int i = 1; i <= N; i++)
    {
        for (int j = 1; j <= N; j++)
        {
            if (i == j)
            {
                F[i][j] = 0; // merging a single village into itself costs nothing and brings nothing
                G[i][j] = 0; // no possible totem to buy inside
            }
            else
            {
                F[i][j] = NEG_INF; // will be updated in DP
                G[i][j] = NEG_INF; // will be updated in DP
            }
        }
    }

    // ===== DP for F[i][j] =====
    // F[i][j] = best profit when we finally merge all villages in [i, j] into one village.
    //
    // Recurrence:
    // For i < j:
    // F[i][j] = max over k in [i..j-1] of:
    //      F[i][k] + F[k+1][j] + ( P - L * |sum(i..k) - sum(k+1..j)| )
    //
    // We process all intervals by increasing length.
    for (int len = 2; len <= N; len++)
    {
        for (int i = 1; i + len - 1 <= N; i++)
        {
            int j = i + len - 1;

            long long best = NEG_INF;

            // Try all possible positions k where the last merge splits [i..j] into [i..k] and [k+1..j].
            for (int k = i; k < j; k++)
            {
                long long left_sum = range_sum(prefix, i, k);
                long long right_sum = range_sum(prefix, k + 1, j);

                long long diff = left_sum - right_sum;
                if (diff < 0)
                {
                    diff = -diff;
                }

                long long merge_profit = P - L * diff;

                long long candidate = F[i][k] + F[k + 1][j] + merge_profit;
                if (candidate > best)
                {
                    best = candidate;
                }
            }

            F[i][j] = best;
        }
    }

    // ===== DP for G[i][j] =====
    // G[i][j] = maximum profit we can get from interval [i, j] if we are allowed
    // to leave some borders (we do not have to merge all villages into one).
    //
    // Recurrence:
    // G[i][j] = max(
    //        F[i][j],                          // merge the whole interval into one village
    //        max over k in [i..j-1] of ( G[i][k] + G[k+1][j] )  // leave a border between k and k+1
    // )
    for (int len = 2; len <= N; len++)
    {
        for (int i = 1; i + len - 1 <= N; i++)
        {
            int j = i + len - 1;

            // Option 1: merge the whole interval [i, j] into one village.
            long long best = F[i][j];

            // Option 2: split [i, j] into two parts and keep the border between them.
            for (int k = i; k < j; k++)
            {
                long long candidate = G[i][k] + G[k + 1][j];
                if (candidate > best)
                {
                    best = candidate;
                }
            }

            G[i][j] = best;
        }
    }

    // Final answer: best profit on the whole segment [1, N].
    cout << G[1][N] << endl;

    return 0;
}
