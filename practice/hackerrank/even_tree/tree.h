#ifndef __TREE_H__
#define __TREE_H__

#include <vector>
#include <algorithm>

struct Node {
	Node(const int id) : _id(id), _aggr(1) {};
	Node(const Node& rhs) : _id(rhs._id), _aggr(rhs._aggr), _neib(rhs._neib) {};

	void connect(Node& node) {
		_neib.push_back(node._id);
		node._neib.push_back(_id);
	};

	void disconnect(Node& node) {
		remove(node._id);
		node.remove(_id);
	};

	bool isLeaf() const {
		return _neib.size() == 1;
	}
	int _id;
	size_t _aggr;
	std::vector<int> _neib;

	private:
	void remove(const int id) {
		std::vector<int>::iterator it = std::find(_neib.begin(), _neib.end(), id);
		if (it != _neib.end()) {
			_neib.erase(it);
		}
	};
};

struct Tree {
	Tree(const size_t n) : _edges(0) {
		for (size_t i = 0; i < n; ++i) {
			_nodes.push_back(Node(i));
		}
	};
  virtual ~Tree() {};

	void linkNode(const int a, const int b) {
		_nodes[a].connect(_nodes[b]);
		_edges++;
	};

	void removeLink(const int a, const int b) {
		_nodes[a].disconnect(_nodes[b]);
		_edges--;
	};

	void mergeNode(Node& from, Node& to) {
		removeLink(from._id, to._id);
		to._aggr++;    // increase node aggregation degree
		from._id = 0;  // disable this node
	};
	
	std::vector<Node> _nodes;
	size_t _edges;
};

#endif
