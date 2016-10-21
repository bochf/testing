#include <tree.h>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

size_t divide(Tree& tree) {
	if (tree._nodes.size() <= 2)
		return 0;

	size_t edges = 0;

	while (tree._edges > 0) {
		for (auto it = tree._nodes.begin(); it != tree._nodes.end() && tree._edges > 0; ++ it) {
			if (it->isLeaf()) {
				if (it->_aggr % 2 == 0) {
					// even sub tree
					tree.removeLink(it->_id, it->_neib[0]);
					edges++;
				}
				else {
					// leaf node or odd sub tree, need to merge to parent
					tree.mergeNode(*it, tree._nodes[it->_neib[0]]);
				}
			}
		}
	}

	return edges;
}

int main(){
    int n, m;
    cin >> n >> m;
		Tree tree(n);
    for(int i = 0; i < m; i++){
			int a, b;
			cin >> a >> b;
			tree.linkNode(a-1, b-1);
    }

		cout << divide(tree) << endl;

		return 0;
}
