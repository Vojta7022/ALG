#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>

using namespace std;

struct Edge
{
    int to;
    int w;
};

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M;
    cin >> N >> M;

    vector<vector<Edge>> g(N + 1);
    vector<int> indeg(N + 1, 0);
    vector<int> outdeg(N + 1, 0);

    for (int i = 0; i < M; i++)
    {
        int U, V, C;
        cin >> U >> V >> C;
        g[U].push_back(Edge{V, C});
        indeg[V]++;
        outdeg[U]++;
    }

    // Sources = start stations (indegree == 0) that actually have an outgoing slope.
    vector<int> sources;
    for (int i = 1; i <= N; i++)
    {
        if (indeg[i] == 0 && outdeg[i] > 0)
            sources.push_back(i);
    }
    int L = (int)sources.size();

    // Sinks = finish stations (outdegree == 0).
    // Use indeg>0 to ignore isolated nodes if they exist.
    vector<int> sinks;
    for (int i = 1; i <= N; i++)
    {
        if (outdeg[i] == 0 && indeg[i] > 0)
            sinks.push_back(i);
    }
    // Fallback (should not be needed in valid testcases)
    if (sinks.empty())
    {
        for (int i = 1; i <= N; i++)
            if (outdeg[i] == 0)
                sinks.push_back(i);
    }

    // -------- Topological order (Kahn's algorithm) --------
    vector<int> indegCopy = indeg;
    queue<int> q;

    for (int i = 1; i <= N; i++)
        if (indegCopy[i] == 0)
            q.push(i);

    vector<int> topo;
    topo.reserve(N);

    while (!q.empty())
    {
        int u = q.front();
        q.pop();
        topo.push_back(u);

        for (size_t k = 0; k < g[u].size(); k++)
        {
            int v = g[u][k].to;
            indegCopy[v]--;
            if (indegCopy[v] == 0)
                q.push(v);
        }
    }

    // -------- Longest path to ANY sink (max credits from u to a finish) --------
    // maxPath[u] = max sum of credits on a path u -> (some sink), or -1 if impossible.
    vector<long long> maxPath(N + 1, -1);
    for (int t : sinks)
        maxPath[t] = 0;

    for (int i = (int)topo.size() - 1; i >= 0; i--)
    {
        int u = topo[i];
        for (size_t k = 0; k < g[u].size(); k++)
        {
            int v = g[u][k].to;
            int w = g[u][k].w;
            if (maxPath[v] != -1)
            {
                long long cand = (long long)w + maxPath[v];
                if (cand > maxPath[u])
                    maxPath[u] = cand;
            }
        }
    }

    // -------- Sum of shortest distances from all sources to every node --------
    const long long INF = (1LL << 60);

    vector<long long> dist(N + 1, INF);
    vector<long long> distSum(N + 1, 0);
    vector<int> reachCount(N + 1, 0);

    for (int si = 0; si < L; si++)
    {
        int s = sources[si];

        fill(dist.begin(), dist.end(), INF);
        dist[s] = 0;

        for (size_t ti = 0; ti < topo.size(); ti++)
        {
            int u = topo[ti];
            if (dist[u] == INF)
                continue;

            for (size_t k = 0; k < g[u].size(); k++)
            {
                int v = g[u][k].to;
                int w = g[u][k].w;
                long long nd = dist[u] + (long long)w;
                if (nd < dist[v])
                    dist[v] = nd;
            }
        }

        for (int v = 1; v <= N; v++)
        {
            if (dist[v] != INF)
            {
                distSum[v] += dist[v];
                reachCount[v]++;
            }
        }
    }

    // -------- Choose the best meeting node m --------
    // profit(m) = L * maxPath[m] - sum_s dist_s[m]
    long long bestZ = -(1LL << 60);

    for (int m = 1; m <= N; m++)
    {
        if (reachCount[m] == L && maxPath[m] != -1)
        {
            long long profit = (long long)L * maxPath[m] - distSum[m];
            if (profit > bestZ)
                bestZ = profit;
        }
    }

    cout << L << " " << bestZ << "\n";
    return 0;
}
