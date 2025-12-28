/*
================================================================================
 Multi-agent Placement on a Graph — Branch & Bound (Study Version, C-like C++)
================================================================================

Problem (short):
- Undirected connected graph G=(V,E), |V| = N.
- Place A agents of type T1 and B agents of type T2 on distinct vertices (A+B ≤ N).
- Score:
    T1 at v: number of occupied neighbors of v (T1 or T2).
    T2 at v: number of unoccupied neighbors of v.
- Goal: maximize the total score.

Key identity (used for fast updates and upper bounds):
  TotalScore = 2 * |E(S1)| + |E(S2, V \ S2)|
             = 2 * internalEdges(S1)
               + ( sum_{v in S2} deg(v) - 2 * internalEdges(S2) )

Where:
  S1 = vertices chosen for T1 (bitmask),
  S2 = vertices chosen for T2 (bitmask).

Search strategy:
- Depth-first search over an ordering of vertices (degree-desc).
- At each vertex, branch: put T1 / put T2 / leave empty (if feasible).
- Maintain current score incrementally using "delta" formulas:
    Δ if add v to T1 = 2 * |N(v) ∩ S1|
    Δ if add v to T2 = deg(v) - 2 * |N(v) ∩ S2|
- Prune with a simple, safe (admissible) upper bound (UB):
    UB = curScore
       + best-case future T1 contribution to already-placed T1
       + best-case future T1 among-themselves contribution
       + best-case future T2 contribution

Compilation (example):
    g++ -O2 -std=c++17 -Wall -Wextra -pedantic main.cpp -o solver

Input:
    N M A B
    (then M lines of edges, 1-based)
Output:
    one integer = maximum possible total score

This file uses only standard headers and "C-like" C++ (vectors, sort).
All code & comments are in English as requested.
================================================================================
*/

#include <iostream>
#include <vector>
#include <cstdint>
#include <limits>
#include <algorithm>
using namespace std;

// Type alias: bitmask of vertices (N ≤ 30 ⇒ fits easily into 64 bits).
using Mask = uint64_t;

/*--------------------------- Data Structures --------------------------------*/

struct Graph {
    int n, m;     // number of vertices and edges
    int a, b;     // number of T1 agents to place, number of T2 agents to place
    vector<Mask> adj; // adj[i] is a bitmask of neighbors of i (0-based)
    vector<int>  deg; // degree of each vertex
};

/*--------------------------- Utilities --------------------------------------*/

// Portable popcount (Brian–Kernighan trick). Counts 1-bits in x.
static inline int popcount64(Mask x) {
    int c = 0;
    while (x) { x &= (x - 1); ++c; }
    return c;
}

/*--------------------------- Input / Graph Build ----------------------------*/

Graph readGraphFromStdIn() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Graph G{};
    if (!(cin >> G.n >> G.m >> G.a >> G.b)) {
        // No input → exit silently (fits typical judge behavior).
        exit(0);
    }

    G.adj.assign(G.n, 0);

    for (int e = 0; e < G.m; ++e) {
        int u, v; 
        cin >> u >> v;
        --u; --v;                 // convert to 0-based indexing
        G.adj[u] |= (Mask(1) << v);
        G.adj[v] |= (Mask(1) << u);
    }

    G.deg.resize(G.n);
    for (int i = 0; i < G.n; ++i) {
        G.deg[i] = popcount64(G.adj[i]);
    }
    return G;
}

/*--------------------------- Scoring Primitives -----------------------------*/

/*
 Count edges fully inside 'subset' (each edge counted once).
 Implementation: sum over v∈subset popcount(adj[v] & subset) / 2
*/
long long countInternalEdges(const Graph& G, Mask subset) {
    long long twice = 0;
    for (int v = 0; v < G.n; ++v) {
        if (subset & (Mask(1) << v)) {
            twice += popcount64(G.adj[v] & subset); // each edge counted from both endpoints
        }
    }
    return twice / 2;
}

// T1 contribution for S1: 2 * internalEdges(S1).
long long t1Contribution(const Graph& G, Mask S1) {
    return 2LL * countInternalEdges(G, S1);
}

// T2 boundary contribution for S2: sum_{v∈S2} deg(v) - 2 * internalEdges(S2).
long long t2BoundaryContribution(const Graph& G, Mask S2) {
    long long sumDeg = 0;
    for (int v = 0; v < G.n; ++v) {
        if (S2 & (Mask(1) << v)) sumDeg += G.deg[v];
    }
    return sumDeg - 2LL * countInternalEdges(G, S2);
}

/*--------------------------- Incremental Deltas ------------------------------*/

/*
 If we add vertex v to S1, the incremental change in score is:
     Δ_T1(v) = 2 * |N(v) ∩ S1|
*/
static inline long long deltaIfPlaceT1(const Graph& G, Mask S1, int v) {
    return 2LL * popcount64(G.adj[v] & S1);
}

/*
 If we add vertex v to S2, the incremental change in score is:
     Δ_T2(v) = deg(v) - 2 * |N(v) ∩ S2|
 Intuition: T2 boundary adds deg(v); each T2–T2 edge subtracts 2.
*/
static inline long long deltaIfPlaceT2(const Graph& G, Mask S2, int v) {
    return (long long)G.deg[v] - 2LL * popcount64(G.adj[v] & S2);
}

/*--------------------------- Vertex Ordering --------------------------------*/

/*
 Process vertices in descending degree order to make strong decisions first.
 Returns a permutation of {0..n-1}.
*/
vector<int> orderVerticesByDegreeDesc(const Graph& G) {
    vector<pair<int,int>> tmp; // (deg, idx)
    tmp.reserve(G.n);
    for (int i = 0; i < G.n; ++i) tmp.push_back({G.deg[i], i});
    sort(tmp.begin(), tmp.end(), [](const pair<int,int>& L, const pair<int,int>& R){
        if (L.first != R.first) return L.first > R.first; // higher degree first
        return L.second < R.second;
    });
    vector<int> ord(G.n);
    for (int i = 0; i < G.n; ++i) ord[i] = tmp[i].second;
    return ord;
}

/*
 Precompute "suffix masks" over the chosen order:
   suffixMask[i] = bitmask of all remaining vertices ord[i..n-1]
 This lets us get the "remaining set" quickly in DFS.
*/
vector<Mask> buildSuffixMasks(const vector<int>& ord, int n) {
    vector<Mask> suf(n + 1, 0);
    for (int i = n - 1; i >= 0; --i) {
        suf[i] = suf[i + 1] | (Mask(1) << ord[i]);
    }
    return suf;
}

/*--------------------------- Upper Bound (Pruning) ---------------------------*/

/*
 Simple, safe (admissible) upper bound. From the current DFS state, assume
 a "best-case future" for remaining A and B placements:

 UB = curScore
    + UB_T1_to_S1
    + UB_T1_among
    + UB_T2

 where:
  - UB_T1_to_S1:
        For each remaining vertex v, compute dS1(v)=|N(v)∩S1|.
        Taking A_left best vertices, each could contribute up to 2*dS1(v).
  - UB_T1_among:
        Future T1 among themselves can add at most
        2 * min( E(rem), choose(A_left,2) ), because each T1–T1 edge yields +2.
  - UB_T2:
        If no two future T2 were adjacent, each contributes deg(v).
        So sum the top B_left degrees in rem.

 This UB may "double count" the same vertex in T1 and T2 hypotheticals on purpose.
 That's OK: an upper bound is allowed to overestimate. It must never underestimate.
*/
static inline long long choose2(int x) { return (long long)x * (x - 1) / 2; }

long long computeUpperBoundSimple(const Graph& G,
                                  const vector<int>& ord,
                                  const vector<Mask>& suffixMask,
                                  int pos,                // current index in 'ord'
                                  int leftT1, int leftT2, // how many T1/T2 left to place
                                  Mask maskT1,            // already placed T1
                                  long long currentScore) // accumulated score so far
{
    // Remaining vertex set as a bitmask
    Mask rem = suffixMask[pos];

    // 1) UB_T1_to_S1: collect 2*dS1(v) for all remaining vertices, take top leftT1
    vector<int> t1ToS1;
    t1ToS1.reserve(G.n - pos);
    for (int i = pos; i < G.n; ++i) {
        int v = ord[i];
        int dS1 = popcount64(G.adj[v] & maskT1);
        t1ToS1.push_back(2 * dS1);
    }
    sort(t1ToS1.begin(), t1ToS1.end(), greater<int>());
    long long addT1toS1 = 0;
    for (int i = 0; i < leftT1 && i < (int)t1ToS1.size(); ++i) addT1toS1 += t1ToS1[i];

    // 2) UB_T1_among: at most 2 * min(E(rem), choose(leftT1,2))
    long long edgesInRem = countInternalEdges(G, rem);
    long long addT1among = 2LL * min(edgesInRem, choose2(leftT1));

    // 3) UB_T2: sum top leftT2 degrees in rem
    vector<int> degRem;
    degRem.reserve(G.n - pos);
    for (int i = pos; i < G.n; ++i) {
        int v = ord[i];
        degRem.push_back(G.deg[v]);
    }
    sort(degRem.begin(), degRem.end(), greater<int>());
    long long addT2 = 0;
    for (int i = 0; i < leftT2 && i < (int)degRem.size(); ++i) addT2 += degRem[i];

    return currentScore + addT1toS1 + addT1among + addT2;
}

/*--------------------------- DFS with Branch & Bound ------------------------*/

void dfsSearch(const Graph& G,
               const vector<int>& ord,
               const vector<Mask>& suffixMask,
               int pos,                      // index into 'ord'
               int leftT1, int leftT2,       // remaining agents of each type
               Mask maskT1, Mask maskT2,     // current placements
               long long currentScore,       // accumulated score so far
               long long& bestScore)         // global best reference
{
    const int remaining = G.n - pos;

    // Feasibility: we must have enough vertices left to place all remaining agents.
    if (leftT1 + leftT2 > remaining) return;

    // Prune by upper bound: even with "best-case future", can't beat bestScore.
    const long long UB = computeUpperBoundSimple(G, ord, suffixMask,
                                                 pos, leftT1, leftT2,
                                                 maskT1, currentScore);
    if (UB <= bestScore) return;

    // Terminal: all vertices considered
    if (pos == G.n) {
        if (leftT1 == 0 && leftT2 == 0) {
            if (currentScore > bestScore) bestScore = currentScore;
        }
        return;
    }

    const int v = ord[pos];

    // Compute immediate deltas for branching decisions
    const long long dT1 = (leftT1 > 0) ? deltaIfPlaceT1(G, maskT1, v)
                                       : numeric_limits<long long>::min() / 4;
    const long long dT2 = (leftT2 > 0) ? deltaIfPlaceT2(G, maskT2, v)
                                       : numeric_limits<long long>::min() / 4;

    // Heuristic: try the branch with larger immediate gain first.
    if (dT1 >= dT2) {
        if (leftT1 > 0) {
            dfsSearch(G, ord, suffixMask, pos + 1,
                      leftT1 - 1, leftT2,
                      maskT1 | (Mask(1) << v), maskT2,
                      currentScore + dT1, bestScore);
        }
        if (leftT2 > 0) {
            dfsSearch(G, ord, suffixMask, pos + 1,
                      leftT1, leftT2 - 1,
                      maskT1, maskT2 | (Mask(1) << v),
                      currentScore + dT2, bestScore);
        }
    } else {
        if (leftT2 > 0) {
            dfsSearch(G, ord, suffixMask, pos + 1,
                      leftT1, leftT2 - 1,
                      maskT1, maskT2 | (Mask(1) << v),
                      currentScore + dT2, bestScore);
        }
        if (leftT1 > 0) {
            dfsSearch(G, ord, suffixMask, pos + 1,
                      leftT1 - 1, leftT2,
                      maskT1 | (Mask(1) << v), maskT2,
                      currentScore + dT1, bestScore);
        }
    }

    // Option 3: leave vertex v empty (only if still feasible after skipping)
    if (leftT1 + leftT2 <= remaining - 1) {
        dfsSearch(G, ord, suffixMask, pos + 1,
                  leftT1, leftT2,
                  maskT1, maskT2,
                  currentScore, bestScore);
    }
}

/*--------------------------- Solve & Main -----------------------------------*/

long long solveMaxScore(const Graph& G) {
    vector<int> order = orderVerticesByDegreeDesc(G);
    vector<Mask> suffix = buildSuffixMasks(order, G.n);

    long long best = numeric_limits<long long>::min();
    dfsSearch(G, order, suffix,
              /*pos=*/0,
              /*leftT1=*/G.a, /*leftT2=*/G.b,
              /*maskT1=*/0, /*maskT2=*/0,
              /*currentScore=*/0,
              /*bestScore(out)*/ best);
    return best;
}

int main() {
    Graph G = readGraphFromStdIn();
    cout << solveMaxScore(G) << '\n';
    return 0;
}
