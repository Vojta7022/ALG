#include <iostream>
#include <vector>
#include <string>
#include <queue>

using namespace std;

// Transition structure for Turing machine
struct Transition
{
    int toState;
    int writeSymbol; // symbol ID to write
    int move;        // -1 = L, +1 = R
};

// Configuration of the Turing machine - for simulation
struct Config
{
    int state;
    int head;
    vector<int> tape;
};

// Find symbol ID by name
int findSymbolId(const vector<string> &names, const string &sym)
{
    for (int i = 0; i < (int)names.size(); ++i)
    {
        if (names[i] == sym)
            return i;
    }
    return -1;
}

// Apply a transition to the current configuration
void applyTransition(Config &cfg, const Transition &tr, int blankId)
{
    cfg.tape[cfg.head] = tr.writeSymbol; // write symbol
    cfg.head += tr.move;                 // move head left/right

    // Extend tape if head goes out of bounds
    if (cfg.head < 0)
    {
        cfg.tape.insert(cfg.tape.begin(), blankId);
        cfg.head = 0;
    }
    else if (cfg.head >= (int)cfg.tape.size())
    {
        cfg.tape.push_back(blankId);
    }

    cfg.state = tr.toState; // change state
}

// Print the tape contents, trimming leading/trailing blanks
void printTape(const vector<int> &tape, int blankId, const vector<string> &symbolNames)
{

    // Find the first and last non-blank symbols on the tape
    int first = -1, last = -1;
    for (int i = 0; i < (int)tape.size(); ++i)
        if (tape[i] != blankId)
        {
            first = i;
            break;
        }
    for (int i = (int)tape.size() - 1; i >= 0; --i)
        if (tape[i] != blankId)
        {
            last = i;
            break;
        }

    // If the tape is all blanks, print a newline
    if (first == -1)
    {
        cout << "\n";
        return;
    }

    // Print symbols from first to last non-blank
    for (int i = first; i <= last; ++i)
    {
        if (i > first)
            cout << ' '; // separate symbols with space
        cout << symbolNames[tape[i]];
    }
    cout << "\n";
}

// Simulate deterministic Turing machine
bool simulateDeterministic(const vector<vector<vector<Transition>>> &trans,
                           int blankId,
                           const vector<int> &initialTape,
                           vector<int> &finalTape)
{
    Config cfg{1, 0, initialTape};

    while (true)
    {
        if (cfg.state == 2) // accepting state - halt and accept
        {
            finalTape = cfg.tape;
            return true;
        }
        int symbol = cfg.tape[cfg.head];             // current symbol under the head
        const auto &opts = trans[cfg.state][symbol]; // possible transitions
        if (opts.empty())                            // no transitions available - halt and reject
            return false;

        applyTransition(cfg, opts[0], blankId); // apply the only transition
    }
}

// Simulate nondeterministic Turing machine
bool simulateNondeterministic(const vector<vector<vector<Transition>>> &trans,
                              int blankId,
                              const vector<int> &initialTape,
                              vector<int> &finalTape)
{
    queue<Config> q;
    q.push(Config{1, 0, initialTape});

    while (!q.empty())
    {
        Config cfg = q.front(); // current configuration to process
        q.pop();

        if (cfg.state == 2) // accepting state - halt and accept
        {
            finalTape = cfg.tape;
            return true;
        }

        int symbol = cfg.tape[cfg.head];
        const auto &opts = trans[cfg.state][symbol]; // possible transitions for this state and symbol
        if (opts.empty())
            continue;

        for (const auto &tr : opts) // explore all possible transitions
        {
            Config next = cfg;
            applyTransition(next, tr, blankId);
            q.push(next); // enqueue next configuration to explore
        }
    }
    return false;
}

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M, K;
    if (!(cin >> N >> M >> K))
        return 0;

    vector<string> symbolNames(M + 1);
    for (int i = 0; i < M; ++i)
        cin >> symbolNames[i];

    string blankSym; // blank symbol
    cin >> blankSym;
    symbolNames[M] = blankSym;
    int blankId = M;

    string code;
    cin >> code;

    vector<int> initialTape;            // initial tape contents as symbol IDs
    initialTape.reserve(K > 0 ? K : 1); // reserve space for initial tape symbols
    for (int i = 0; i < K; ++i)
    {
        string sym;
        cin >> sym;
        initialTape.push_back(findSymbolId(symbolNames, sym));
    }
    if (initialTape.empty())
        initialTape.push_back(blankId); // tape must have at least one cell

    // Build transition table: trans[state][symbol] = list of transitions
    vector<vector<vector<Transition>>> trans(N + 1, vector<vector<Transition>>(M + 1)); // states are 1..N, symbols are 0..M (M = blank)
    bool isDeterministic = true;

    // Parse the code directly
    if (code.size() >= 6)
    {
        int pos = 3;                    // skip first "111"
        int end = (int)code.size() - 3; // skip last "111"

        // Helper to read a number encoded as zeros
        auto readZeros = [&](int &val)
        {
            val = 0;
            while (pos < end && code[pos] == '0')
            {
                ++val;
                ++pos;
            }
        };

        // Read transitions until the end of the code
        while (pos < end)
        {
            int iVal, jVal, kVal, lVal, rVal;

            // Read five numbers encoded as zeros, separated by ones
            readZeros(iVal);
            if (pos < end && code[pos] == '1') // skip separator
                ++pos;
            readZeros(jVal);
            if (pos < end && code[pos] == '1')
                ++pos;
            readZeros(kVal);
            if (pos < end && code[pos] == '1')
                ++pos;
            readZeros(lVal);
            if (pos < end && code[pos] == '1')
                ++pos;
            readZeros(rVal);

            int fromState = iVal;
            int readSymbolId = jVal - 1;
            int toState = kVal;
            int writeSymbolId = lVal - 1;
            int move = (rVal == 1) ? +1 : -1;

            auto &cell = trans[fromState][readSymbolId]; // get the transition cell
            if (!cell.empty())
                isDeterministic = false;

            cell.push_back(Transition{toState, writeSymbolId, move});

            // skip "11" separator if present
            if (pos + 1 < end && code[pos] == '1' && code[pos + 1] == '1')
                pos += 2;
        }
    }

    cout << (isDeterministic ? 'D' : 'N') << "\n";

    vector<int> finalTape;
    bool accepted = isDeterministic
                        ? simulateDeterministic(trans, blankId, initialTape, finalTape)
                        : simulateNondeterministic(trans, blankId, initialTape, finalTape);

    if (accepted)
    {
        printTape(finalTape, blankId, symbolNames);
    }

    return 0;
}
