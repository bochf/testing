//Problem        : Wrapper's Delight
//Language       : C++
//Compiled Using : g++
//Version        : GCC 4.9.1
//Input for your program will be provided from STDIN
//Print out all output from your program to STDOUT

#include <iostream>
#include <string>
#include <algorithm>
#include <climits> 
#include <vector>

using namespace std;

class Node {
    public:
        int _x;
        int _y;
        
        Node(int x, int y) : _x(x), _y(y) {};
        bool operator>(const Node& right) const {
            if ((_x == right._x) && (_y == right._y))
                return false;
            if ((_x >= right._x) && (_y >= right._y))
                return true;
            else
                return false;
        };
};

class Tree {
    public:
        int nodeId;
        vector<Tree> children;
        int degree;
        
        Tree(int id, int d) : nodeId(id), degree(d) {};
        
        int build(const vector<Node>& nodes, int& max) {
            if (max < degree)
                max = degree;
            for (int i = 0; i < nodes.size(); ++i) {
                if (nodes[i] > nodes[nodeId]) {
                    children.push_back(Tree(i, degree+1));
                }
            }
            
            for (int j = 0; j < children.size(); ++j) {
                children[j].build(nodes, max);
            }
        }
};

int main() {
    int num;
    vector<Node> nodes;
    nodes.push_back(Node(-1, -1)); 
    
    cin >> num;
    for (int i = 0; i < num; ++i) {
        int x;
        int y;
        cin >> x >> y;
        nodes.push_back(Node(x, y));
    }

    Tree tree(0, 0);
    int max = 0;
    tree.build(nodes, max);
    
    cout << max << endl;
    
    return 0;
}
