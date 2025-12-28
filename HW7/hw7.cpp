#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

struct Segment
{
    int startCell;   // index of start cell (row * numCols + col)
    int endCell;     // index of end cell
    int valves;      // number of valves on this segment (sum of inner cells)
    bool isVertical; // false = horizontal segment, true = vertical segment
};

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int numRows, numCols;
    if (!(cin >> numRows >> numCols))
    {
        return 0;
    }

    // Read grid of valves
    vector<vector<int>> grid(numRows, vector<int>(numCols));
    for (int row = 0; row < numRows; ++row)
    {
        for (int col = 0; col < numCols; ++col)
        {
            cin >> grid[row][col];
        }
    }

    // Row prefix sums: rowPrefix[row][col] = sum of grid[row][0..col]
    vector<vector<int>> rowPrefix(numRows, vector<int>(numCols));
    for (int row = 0; row < numRows; ++row)
    {
        int runningSum = 0;
        for (int col = 0; col < numCols; ++col)
        {
            runningSum += grid[row][col];
            rowPrefix[row][col] = runningSum;
        }
    }

    // Column prefix sums: colPrefix[row][col] = sum of grid[0..row][col]
    vector<vector<int>> colPrefix(numRows, vector<int>(numCols));
    for (int col = 0; col < numCols; ++col)
    {
        int runningSum = 0;
        for (int row = 0; row < numRows; ++row)
        {
            runningSum += grid[row][col];
            colPrefix[row][col] = runningSum;
        }
    }

    // Helper to convert (row, col) to cell index
    auto cellIndex = [numCols](int row, int col)
    {
        return row * numCols + col;
    };

    int startCellIndex = cellIndex(0, 0);
    int destCellIndex = cellIndex(numRows - 1, numCols - 1);
    int numCells = numRows * numCols;

    vector<Segment> segments;
    // Upper bound for number of segments, to avoid many reallocations
    segments.reserve(8000000);

    // Generate all horizontal segments (from left to right)
    for (int row = 0; row < numRows; ++row)
    {
        for (int colStart = 0; colStart < numCols - 1; ++colStart)
        {
            int prefixAtStart = rowPrefix[row][colStart];
            for (int colEnd = colStart + 1; colEnd < numCols; ++colEnd)
            {
                // Inner cells are (colStart+1 .. colEnd-1)
                int innerValves = rowPrefix[row][colEnd - 1] - prefixAtStart; // sum of inner cells (colStart+1 .. colEnd-1)
                Segment seg;
                seg.startCell = cellIndex(row, colStart);
                seg.endCell = cellIndex(row, colEnd);
                seg.valves = innerValves;
                seg.isVertical = false; // horizontal
                segments.push_back(seg);
            }
        }
    }

    // Generate all vertical segments (from top to bottom)
    for (int col = 0; col < numCols; ++col)
    {
        for (int rowStart = 0; rowStart < numRows - 1; ++rowStart)
        {
            int prefixAtStart = colPrefix[rowStart][col];
            for (int rowEnd = rowStart + 1; rowEnd < numRows; ++rowEnd)
            {
                // Inner cells are (rowStart+1 .. rowEnd-1)
                int innerValves = colPrefix[rowEnd - 1][col] - prefixAtStart; // sum of inner cells (rowStart+1 .. rowEnd-1)
                Segment seg;
                seg.startCell = cellIndex(rowStart, col);
                seg.endCell = cellIndex(rowEnd, col);
                seg.valves = innerValves;
                seg.isVertical = true; // vertical
                segments.push_back(seg);
            }
        }
    }

    // Sort segments by number of valves in descending order
    sort(segments.begin(), segments.end(),
         [](const Segment &a, const Segment &b)
         {
             return a.valves > b.valves;
         });

    const long long NO_PATH = -1; // no valid path found

    // bestHorizontal[cell] = best total valves for path ending in 'cell'
    //                        where the last segment is horizontal
    // bestVertical[cell]   = same, but last segment is vertical
    vector<long long> bestHorizontal(numCells, NO_PATH);
    vector<long long> bestVertical(numCells, NO_PATH);

    long long bestTotalValves = 0;

    // Process segments grouped by equal valves, to ensure strictly decreasing sequence
    for (size_t groupStartIndex = 0; groupStartIndex < segments.size();)
    {
        size_t groupEndIndex = groupStartIndex;
        int currentValves = segments[groupStartIndex].valves;

        // Find the range [groupStartIndex, groupEndIndex) with the same valves
        while (groupEndIndex < segments.size() &&
               segments[groupEndIndex].valves == currentValves)
        {
            ++groupEndIndex;
        }

        size_t groupSize = groupEndIndex - groupStartIndex;
        vector<long long> groupDP(groupSize, NO_PATH); // DP values for this group

        // First pass: compute DP values for this group without updating global DP yet
        for (size_t idx = groupStartIndex; idx < groupEndIndex; ++idx)
        {
            const Segment &seg = segments[idx];
            long long bestForThisSegment = NO_PATH;

            // Option 1: this is the first segment, starting at start cell
            if (seg.startCell == startCellIndex)
            {
                bestForThisSegment = currentValves;
            }

            // Option 2: attach after a previous segment of opposite direction
            if (seg.isVertical)
            {
                // previous must be horizontal
                long long prevBest = bestHorizontal[seg.startCell];
                if (prevBest != NO_PATH &&
                    prevBest + currentValves > bestForThisSegment)
                {
                    bestForThisSegment = prevBest + currentValves;
                }
            }
            else
            {
                // seg is horizontal, previous must be vertical
                long long prevBest = bestVertical[seg.startCell];
                if (prevBest != NO_PATH &&
                    prevBest + currentValves > bestForThisSegment)
                {
                    bestForThisSegment = prevBest + currentValves;
                }
            }

            groupDP[idx - groupStartIndex] = bestForThisSegment;

            // If this segment ends at destination cell, update answer
            if (seg.endCell == destCellIndex &&
                bestForThisSegment != NO_PATH &&
                bestForThisSegment > bestTotalValves)
            {
                bestTotalValves = bestForThisSegment;
            }
        }

        // Second pass: update global DP arrays with results from this group
        for (size_t idx = groupStartIndex; idx < groupEndIndex; ++idx)
        {
            long long value = groupDP[idx - groupStartIndex];
            if (value == NO_PATH)
                continue;

            const Segment &seg = segments[idx];
            if (seg.isVertical)
            {
                if (value > bestVertical[seg.endCell])
                {
                    bestVertical[seg.endCell] = value;
                }
            }
            else
            {
                if (value > bestHorizontal[seg.endCell])
                {
                    bestHorizontal[seg.endCell] = value;
                }
            }
        }

        groupStartIndex = groupEndIndex;
    }

    cout << bestTotalValves << '\n';
    return 0;
}
