#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <limits>

using namespace std;

struct Edge
{
    int to;
    int weight;
};

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M;
    cin >> N >> M;

    vector<vector<Edge>> graph(N + 1);
    vector<int> inDegrees = vector<int>(N + 1, 0); // ncoming edges
    vector<int> outDegrees = vector<int>(N + 1, 0); // outgoing edges

    for (int i = 0; i < M; ++i)
    {
        int U, V, C;
        cin >> U >> V >> C;
        graph[U].push_back(Edge{V, C});
        outDegrees[U]++;
        inDegrees[V]++;
    }

    // Starting stations
    vector<int> startingStations;
    for (int i = 1; i <= N; ++i)
    {
        if (inDegrees[i] == 0 && outDegrees[i] > 0)
        {
            startingStations.push_back(i);
        }
    }
    int L = static_cast<int>(startingStations.size());

    // Lowest stations - where no slope starts and at least one slope ends - ignoring potential isolated nodes
    vector<int> lowestStations;
    for (int i = 1; i <= N; ++i)
    {
        if (outDegrees[i] == 0 && inDegrees[i] > 0)
        {
            lowestStations.push_back(i);
        }
    }

    // Put the nodes in topological order to do DP on DAG
    vector<int> inDegreesCopy = inDegrees;
    queue<int> queue; // first put nodes with inDegree 0
    for (int i = 1; i <= N; i++)
    {
        if (inDegreesCopy[i] == 0)
        {
            queue.push(i);
        }
    }

    vector<int> topologicalOrder; // topological order of nodes
    topologicalOrder.reserve(N); // most N nodes

    while (!queue.empty())
    {
        int u = queue.front();
        queue.pop(); // remove the processed node from the queue
        topologicalOrder.push_back(u); // put it to topological order

        // lower inDegree of all nodes that have a slope from u to them
        for (size_t k = 0; k < graph[u].size(); k++)
        {
            int v = graph[u][k].to;
            inDegreesCopy[v]--;
            if (inDegreesCopy[v] == 0) // if inDegree gets to 0, push to queue
            {
                queue.push(v);
            }
        }
    }

    vector<int> maxPath(N + 1, -1); // max path from each node to osme lowest station
    for (int t : lowestStations)
        maxPath[t] = 0;

    // ind max path from each node to some lowest station
    for (int i = (int)topologicalOrder.size() - 1; i >= 0; i--)
    {
        int u = topologicalOrder[i];
        for (size_t k = 0; k < graph[u].size(); k++)
        {
            int v = graph[u][k].to;
            int w = graph[u][k].weight;
            if (maxPath[v] != -1)
            {
                int candidate = w + maxPath[v];
                if (candidate > maxPath[u])
                    maxPath[u] = candidate;
            }
        }
    }

    const int maxInt = numeric_limits<int>::max();

    vector<long long> distance(N + 1, maxInt); // distance from start to each node
    vector<long long> distanceSum(N + 1, 0); // sum of distances from all starts to each node
    vector<int> reachCount(N + 1, 0); // number of starts that can reach each node

    for (int startI = 0; startI < L; startI++) // for each starting station
    {
        int s = startingStations[startI];

        fill(distance.begin(), distance.end(), maxInt); // reset distances for each start
        distance[s] = 0;

        for (size_t ti = 0; ti < topologicalOrder.size(); ti++) // iterate nodes in topological order
        {
            int u = topologicalOrder[ti];
            if (distance[u] == maxInt) // unreachable
                continue;

            for (size_t k = 0; k < graph[u].size(); k++) // try if can get to neighbors with cheaper cost
            {
                int v = graph[u][k].to;
                int w = graph[u][k].weight;
                long long nd = distance[u] + (long long)w;
                if (nd < distance[v]) // check if found a cheaper way
                    distance[v] = nd;
            }
        }

        for (int v = 1; v <= N; v++)
        {
            if (distance[v] != maxInt) // if reachable from this starting station
            {
                distanceSum[v] += distance[v]; // add distance to sum
                reachCount[v]++; // increase reach count
            }
        }
    }

    long long Z = -maxInt;

    for (int m = 1; m <= N; m++) // iterate through all nodes to find the best meeting point
    {
        if (reachCount[m] == L && maxPath[m] != -1) // reachable from all starting stations and can go down to lowest station
        {
            long long profit = (long long)L * maxPath[m] - distanceSum[m];
            if (profit > Z)
                Z = profit;
        }
    }

    cout << L << " " << Z << "\n";
    return 0;
}