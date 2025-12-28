#include <bits/stdc++.h>
using namespace std;

// Can the mouse enter node u given current r?
static inline bool safeToEnter(int u, int r,
                               const vector<int> &D,
                               const vector<char> &isNoisy,
                               int M)
{
    if (D[u] == -1)
        return false;
    int need = isNoisy[u] ? min(r + 1, M) : r;
    return D[u] > need;
}

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, H, E;
    if (!(cin >> N >> H >> E))
        return 0;

    int S, C, K, M;
    cin >> S >> C >> K >> M;

    vector<char> isNoisy(N, false);
    for (int i = 0; i < H; ++i)
    {
        int x;
        cin >> x;
        isNoisy[x] = true;
    }

    vector<vector<int>> g(N);
    g.reserve(N);
    for (int i = 0; i < E; ++i)
    {
        int a, b;
        cin >> a >> b;
        g[a].push_back(b);
        g[b].push_back(a);
    }

    // 1) BFS from K to get D[v]
    vector<int> D(N, -1);
    {
        queue<int> q;
        D[K] = 0;
        q.push(K);
        while (!q.empty())
        {
            int v = q.front();
            q.pop();
            for (int u : g[v])
                if (D[u] == -1)
                {
                    D[u] = D[v] + 1;
                    q.push(u);
                }
        }
    }

    // 2) Lexicographic Dijkstra over layered states (v, r)
    const int R = M + 1;
    const int INF = 1e9;

    auto ID = [R](int v, int r)
    { return v * R + r; };

    vector<int> bestLen((size_t)N * R, INF);
    vector<int> bestNoi((size_t)N * R, INF);

    using State = tuple<int, int, int, int>; // (len, noisy, v, r)
    priority_queue<State, vector<State>, greater<State>> pq;

    // Start at (S, r=0), length 0, noisy 0
    bestLen[ID(S, 0)] = 0;
    bestNoi[ID(S, 0)] = 0;
    pq.emplace(0, 0, S, 0);

    // Dijkstra main loop
    while (!pq.empty())
    {
        auto top = pq.top();
        pq.pop();
        int len = get<0>(top);
        int noisy = get<1>(top);
        int v = get<2>(top);
        int r = get<3>(top);
        int idx = ID(v, r);
        if (len != bestLen[idx] || noisy != bestNoi[idx])
            continue;

        if (v == C)
        {
            cout << len << ' ' << noisy << '\n';
            return 0;
        }

        for (int u : g[v])
        {
            if (!safeToEnter(u, r, D, isNoisy, M))
                continue;

            // compute new state (nlen, nnoi, u, rr)
            int rr = isNoisy[u] ? min(r + 1, M) : r;
            int nlen = len + 1;
            int nnoi = noisy + (isNoisy[u] ? 1 : 0);
            int j = ID(u, rr);

            // update values if (nlen, nnoi) is lexicographically better
            if (nlen < bestLen[j] || (nlen == bestLen[j] && nnoi < bestNoi[j]))
            {
                bestLen[j] = nlen;
                bestNoi[j] = nnoi;
                pq.emplace(nlen, nnoi, u, rr);
            }
        }
    }

    return 0;
}
