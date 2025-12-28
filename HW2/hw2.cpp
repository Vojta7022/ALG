#include <iostream>
#include <vector>
#include <cstdint>
#include <climits>
#include <limits>
#include <algorithm>
using namespace std;

struct Graph
{
    int N, M, A, B;
    vector<uint64_t> adj; // adj[i] is a bitmask of neighbors of vertex i (0-based)
    vector<int> deg;      // degree of each vertex
};

// Portable bit count
static inline int popcount64(uint64_t x)
{
    int c = 0;
    while (x)
    {
        x &= (x - 1);
        ++c;
    }
    return c;
}

Graph read_graph()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Graph G;
    if (!(cin >> G.N >> G.M >> G.A >> G.B))
    {
        exit(0);
    }
    G.adj.assign(G.N, 0);

    for (int e = 0; e < G.M; ++e)
    {
        int u, v;
        cin >> u >> v;
        --u;
        --v; // convert to 0-based
        G.adj[u] |= (uint64_t(1) << v);
        G.adj[v] |= (uint64_t(1) << u);
    }

    G.deg.resize(G.N);
    for (int i = 0; i < G.N; ++i)
    {
        G.deg[i] = popcount64(G.adj[i]);
    }
    return G;
}

// Count edges fully inside 'mask' (each edge counted once).
long long internal_edges(const Graph &G, uint64_t mask)
{
    long long total = 0;
    for (int v = 0; v < G.N; ++v)
    {
        if (mask & (uint64_t(1) << v))
        {
            total += popcount64(G.adj[v] & mask);
        }
    }
    return total / 2; // each internal edge counted twice, once at each endpoint
}

long long t1_score(const Graph &G, uint64_t mask)
{
    return 2LL * internal_edges(G, mask);
}

long long t2_boundary(const Graph &G, uint64_t mask)
{
    long long sum_deg = 0;
    for (int v = 0; v < G.N; ++v)
    {
        if (mask & (uint64_t(1) << v))
            sum_deg += G.deg[v];
    }
    return sum_deg - 2LL * internal_edges(G, mask);
}

long long total_score(const Graph &G, uint64_t S1, uint64_t S2)
{
    if (S1 & S2)
        return numeric_limits<long long>::min(); // invalid: not disjoint
    return t1_score(G, S1) + t2_boundary(G, S2);
}

static inline long long delta_t1_if_add(const Graph &G, uint64_t S1_mask, int v)
{
    uint64_t neigh_in_S1 = G.adj[v] & S1_mask;
    return 2LL * popcount64(neigh_in_S1);
}

static inline long long delta_t2_if_add(const Graph &G, uint64_t S2_mask, int v)
{
    uint64_t neigh_in_S2 = G.adj[v] & S2_mask;
    return (long long)G.deg[v] - 2LL * popcount64(neigh_in_S2);
}

vector<int> order_by_degree_desc(const Graph &G)
{
    vector<int> ord(G.N);
    for (int i = 0; i < G.N; ++i)
        ord[i] = i;
    sort(ord.begin(), ord.end(), [&](int a, int b)
         {
        if (G.deg[a] != G.deg[b]) return G.deg[a] > G.deg[b];
        return a < b; });
    return ord;
}

vector<uint64_t> build_suffix_masks(const vector<int> &ord, int N)
{
    vector<uint64_t> suf(N + 1, 0);
    for (int i = N - 1; i >= 0; --i)
    {
        suf[i] = suf[i + 1] | (uint64_t(1) << ord[i]);
    }
    return suf;
}

long long upper_bound_simple(const Graph &G,
                             const vector<int> &ord,
                             const vector<uint64_t> &suf_mask,
                             int idx,
                             int A_left, int B_left,
                             uint64_t S1_mask,
                             long long cur_score)
{
    uint64_t rem = suf_mask[idx];

    // T1_future_to_S1: collect 2*dS1[v] for rem vertices
    vector<int> t1_to_S1;
    t1_to_S1.reserve(G.N);
    for (int j = idx; j < G.N; ++j)
    {
        int v = ord[j];
        int dS1 = popcount64(G.adj[v] & S1_mask);
        t1_to_S1.push_back(2 * dS1);
    }
    sort(t1_to_S1.begin(), t1_to_S1.end(), greater<int>());
    long long add_t1_to_S1 = 0;
    for (int i = 0; i < A_left && i < (int)t1_to_S1.size(); ++i)
        add_t1_to_S1 += t1_to_S1[i];

    // T1_future_among_themselves: at most 2 * min(E(rem), choose(A_left,2))
    long long Erem = internal_edges(G, rem);
    long long chooseA = (long long)A_left * (A_left - 1) / 2;
    long long add_t1_among = 2LL * min(Erem, chooseA);

    // T2_future: sum of top B_left degrees in rem
    vector<int> degrem;
    degrem.reserve(G.N);
    for (int j = idx; j < G.N; ++j)
    {
        int v = ord[j];
        degrem.push_back(G.deg[v]);
    }
    sort(degrem.begin(), degrem.end(), greater<int>());
    long long add_t2 = 0;
    for (int i = 0; i < B_left && i < (int)degrem.size(); ++i)
        add_t2 += degrem[i];

    return cur_score + add_t1_to_S1 + add_t1_among + add_t2;
}

void dfs(const Graph &G,
         const vector<int> &ord,
         const vector<uint64_t> &suf_mask,
         int idx,
         int A_left, int B_left,
         uint64_t S1_mask, uint64_t S2_mask,
         long long cur_score,
         long long &best_score)
{
    int remaining = G.N - idx;
    if (A_left + B_left > remaining)
        return;

    // PRUNE by UB
    if (upper_bound_simple(G, ord, suf_mask, idx, A_left, B_left, S1_mask, cur_score) <= best_score)
    {
        return;
    }

    if (idx == G.N)
    {
        if (A_left == 0 && B_left == 0)
        {
            if (cur_score > best_score)
                best_score = cur_score;
        }
        return;
    }

    int v = ord[idx];

    long long d1 = (A_left > 0) ? 2LL * popcount64(G.adj[v] & S1_mask) : std::numeric_limits<long long>::min() / 4;
    long long d2 = (B_left > 0) ? (long long)G.deg[v] - 2LL * popcount64(G.adj[v] & S2_mask) : std::numeric_limits<long long>::min() / 4;

    // Try better-Δ branch first
    if (d1 >= d2)
    {
        if (A_left > 0)
            dfs(G, ord, suf_mask, idx + 1, A_left - 1, B_left, S1_mask | (uint64_t(1) << v), S2_mask, cur_score + d1, best_score);
        if (B_left > 0)
            dfs(G, ord, suf_mask, idx + 1, A_left, B_left - 1, S1_mask, S2_mask | (uint64_t(1) << v), cur_score + d2, best_score);
    }
    else
    {
        if (B_left > 0)
            dfs(G, ord, suf_mask, idx + 1, A_left, B_left - 1, S1_mask, S2_mask | (uint64_t(1) << v), cur_score + d2, best_score);
        if (A_left > 0)
            dfs(G, ord, suf_mask, idx + 1, A_left - 1, B_left, S1_mask | (uint64_t(1) << v), S2_mask, cur_score + d1, best_score);
    }

    // Leave empty (only if still feasible)
    if (A_left + B_left <= remaining - 1)
    {
        dfs(G, ord, suf_mask, idx + 1, A_left, B_left, S1_mask, S2_mask, cur_score, best_score);
    }
}

int main()
{
    Graph G = read_graph();
    auto ord = order_by_degree_desc(G);
    auto suf = build_suffix_masks(ord, G.N);
    long long best = std::numeric_limits<long long>::min();
    dfs(G, ord, suf, 0, G.A, G.B, 0, 0, 0, best);
    cout << best << "\n";
    return 0;
}
