#include <iostream>
#include <vector>
#include <string>
using namespace std;

struct Node
{
    int key;      // key value
    int depth;    // distance in edges from the root (0 for root)
    bool deleted; // true = lazily deleted
    Node *left;
    Node *right;
};

Node *createNode(int key, int depth)
{
    Node *n = new Node;
    n->key = key;
    n->depth = depth;
    n->deleted = false;
    n->left = n->right = nullptr;
    return n;
}

Node *root = nullptr;
Node *nodes[10001]; // nodes[key] = pointer to node with that key, or nullptr

int treeDepth = 0; // max depth (edges) of any node in the current tree

int aliveCount = 0;
int deletedCount = 0;

long long aliveDepthSum = 0;   // sum of depths of alive nodes
long long deletedDepthSum = 0; // sum of depths of deleted nodes

long long compactCount = 0;

// Insert a new node with given key into the BST.
Node *insertNewNodeToBST(int key)
{
    if (!root)
    {
        root = createNode(key, 0);
        treeDepth = 0;
        return root;
    }

    Node *curr = root;
    int d = 0;
    while (true)
    {
        if (key < curr->key)
        {
            if (!curr->left)
            {
                int newDepth = d + 1;
                Node *n = createNode(key, newDepth);
                curr->left = n;
                if (newDepth > treeDepth)
                    treeDepth = newDepth;
                return n;
            }
            curr = curr->left;
            d++;
        }
        else
        { // key > curr->key, because we never insert duplicates here
            if (!curr->right)
            {
                int newDepth = d + 1;
                Node *n = createNode(key, newDepth);
                curr->right = n;
                if (newDepth > treeDepth)
                    treeDepth = newDepth;
                return n;
            }
            curr = curr->right;
            d++;
        }
    }
}

// Recursively collect keys of alive (not deleted) nodes in preorder.
void collectAlivePreorder(Node *node, vector<int> &keys)
{
    if (!node)
        return;
    if (!node->deleted)
        keys.push_back(node->key);
    collectAlivePreorder(node->left, keys);
    collectAlivePreorder(node->right, keys);
}

// Recursively free all nodes in the tree.
void freeTree(Node *node)
{
    if (!node)
        return;
    freeTree(node->left);
    freeTree(node->right);
    delete node;
}

// Check whether we need to run Compact based on heights sum rule.
void checkCompact()
{
    long long aliveHeightSum = (long long)aliveCount * treeDepth - aliveDepthSum;
    long long deletedHeightSum = (long long)deletedCount * treeDepth - deletedDepthSum;
    if (deletedHeightSum > aliveHeightSum)
    {
        // Compact
        vector<int> keys;
        keys.reserve(aliveCount);
        collectAlivePreorder(root, keys); // collect alive keys in preorder

        // Reset tree and related data
        freeTree(root);
        root = nullptr;

        for (int i = 0; i <= 10000; ++i)
            nodes[i] = nullptr;

        treeDepth = 0;
        aliveCount = 0;
        deletedCount = 0;
        aliveDepthSum = 0;
        deletedDepthSum = 0;

        // Rebuild BST from alive keys in preorder order
        for (int key : keys)
        {
            Node *n = insertNewNodeToBST(key);
            nodes[key] = n;
            aliveCount++;
            aliveDepthSum += n->depth;
        }

        compactCount++;
    }
}

void insertKey(int k)
{
    Node *node = nodes[k];

    if (node)
    {
        // Key already exists in the tree
        if (node->deleted)
        {
            // Revive deleted node
            node->deleted = false;

            deletedCount--;
            deletedDepthSum -= node->depth;

            aliveCount++;
            aliveDepthSum += node->depth;
        }
        // If node is already alive, do nothing
    }
    else
    {
        // New key -> insert new node
        Node *n = insertNewNodeToBST(k);
        nodes[k] = n;

        aliveCount++;
        aliveDepthSum += n->depth;
    }

    checkCompact();
}

// Lazily delete the node with the given key from the BST.
void deleteKey(int k)
{
    Node *node = nodes[k];
    if (!node || node->deleted)
        return; // key not in tree or already deleted

    node->deleted = true;

    aliveCount--;
    aliveDepthSum -= node->depth;

    deletedCount++;
    deletedDepthSum += node->depth;

    checkCompact();
}

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    for (int i = 0; i <= 10000; ++i)
        nodes[i] = nullptr;

    int N;
    if (!(cin >> N))
        return 0;

    for (int i = 0; i < N; ++i)
    {
        string op;
        int K;
        cin >> op >> K;
        if (op == "ins")
        {
            insertKey(K);
        }
        else
        {
            deleteKey(K);
        }
    }

    // Output the number of compactions and the final depth of the tree
    int finalDepth = (root ? treeDepth : 0);
    cout << compactCount << " " << finalDepth << "\n";
    return 0;
}
