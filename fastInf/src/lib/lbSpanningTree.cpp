/* Copyright 2009 Ariel Jaimovich, Ofer Meshi, Ian McGraw and Gal Elidan */


/*
This file is part of FastInf library.

FastInf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

FastInf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with FastInf.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <lbSpanningTree.h>
#include <lbPropagationInference.h>

using namespace lbLib ;
//using namespace std;

lbRandomGenerator lbSpanningTree::_randomGenerator(0);
//implementIntegerStruct(node);
  
lbSpanningTree::lbSpanningTree(edgeSet const& edges, nodesSet const& nodes,
			       int maxNode) :
  _edges(edges),
  _nodes(nodes),
  _adjacencyList(maxNode),
  _edgeScorer(NULL)
{
  _maxNode=maxNode;
  _tree=edgeSet();
}

lbSpanningTree::~lbSpanningTree(){
  //??
}

// In this we assume there is a spanning tree beginning from node edges[0].first()
void lbSpanningTree::buildSpanningTree() {
  if (_nodes.size() == 0)
    return;
  if (_edges.size() == 0)
    return;
  bool* coveredNodes = new bool[_maxNode];
  if ((int)_adjacencyList.size() != _maxNode) {
    _adjacencyList.resize(_maxNode);
  }
  for (int i = 0; i < _maxNode; ++i) {
    coveredNodes[i] = false;
  }
  for (int i = 0; i < _maxNode; ++i) {
    _adjacencyList[i].clear();
  }
  for (uint i = 0; i < _edges.size(); ++i) {
    _adjacencyList[_edges[i].first()].insert(_edges[i].second());
    _adjacencyList[_edges[i].second()].insert(_edges[i].first());
  }
  int curr = _edges[0].first();
  queue <int> nodesToVisit;
  nodesToVisit.push(curr);
  coveredNodes[curr] = true;
  while (!nodesToVisit.empty()) {
    int curr = nodesToVisit.front();
    nodesToVisit.pop();
    const set<int> & adjacent = _adjacencyList[curr];
    for (set<int>::const_iterator iter = adjacent.begin(); iter != adjacent.end(); ++iter) {
      //cerr << "Seeing node " << *iter << " from " << curr << endl;
      if (!coveredNodes[*iter]) {
	// Node is not covered, so we add it to tree and push it on queue
	nodesToVisit.push(*iter);
	coveredNodes[*iter] = true;
	_tree.push_back(edge(*iter, curr));
      }
    }
  }
  //cerr << "Tree is this:\n";
  //printTree(cerr);
  delete [] coveredNodes;
}

void lbSpanningTree::setMaxNode(int numCliques) {
  _maxNode = numCliques;
  _adjacencyList.resize(_maxNode);
  //_nodeSet.resize(_maxNode);
}

edgeSet const& lbSpanningTree::getSpanningTree(){
  return _tree;
}

edge lbSpanningTree::addEdge(const edge & add) {
  // TODO: 2 Things. Make sure spanning tree is satisfied, and also allow rebuilding
  // of tree entirely.
  bool sawNode1(false), sawNode2(false);
  _edges.push_back(add);
  _tree.push_back(add);
  // TODO: Check to make sure these are valid?
  _adjacencyList[add.first()].insert(add.second());
  _adjacencyList[add.second()].insert(add.first());
  // Check in the nodes set--see if the edge is in there. This method may be
  // unnecessary if we don't ever use the nodes.
  for (uint i = 0; i < _nodes.size(); ++i) {
    if (_nodes[i] == add.first()) {
      sawNode1 = true;
    } else if (_nodes[i] == add.second()) {
      sawNode2 = true;
    }
  }
  if (!sawNode1) {
    _nodes.push_back(add.first());
  }
  if (!sawNode2) {
    _nodes.push_back(add.second());
  }
  return tryRemoveEdge(*this, add);
}

void lbSpanningTree::removeEdge(const edge & remover) {
  edgeSet::iterator iter = _tree.begin();
  edge e(0, 0);
//   if (lbOptions::isVerbose(V_INCREMENTAL)) {
//     cerr << "[lbSpanningTree::removeEdge] Removing edge (" << remover.first()
// 	 << ", " << remover.second() << ")" << endl;
//   }
  for (; iter != _tree.end(); iter++) {
    e = *iter;
    if ((e.first() == remover.first() && e.second() == remover.second()) ||
	(e.second() == remover.first() && e.first() == remover.second())) {
      _tree.erase(iter);
      return;
    }
  }
  NOT_REACHED;
}

void lbSpanningTree::printTree(ostream& out) const {
  out << "Spanning tree: [";
  out << "Nodes:  ";
  for (uint n = 0; n < _nodes.size(); n++)
    out << _nodes[n]<< " ";
  
  out << "] [Edges:  ";
  uint e;
  for (e=0;e<_edges.size();e++) {
    out << "e" << e << ": (" << _edges[e].first() << ", " << _edges[e].second() << ")  ";
  }

  out<<"] [Tree:  ";
  for (e=0;e<_tree.size();e++)
    out<< "t" << e << ": (" << _tree[e].first() << ", "<< _tree[e].second() << ")  ";
  out << "]\n";
}

void lbSpanningTree::setEdgeScoreFunction(edgeScoreFunction * f, void * auxiliary) {
  _edgeScorer = f;
  _edgeScoreAux = auxiliary;
}

edge lbSpanningTree::lowEdgeOnCycle(int * predecessor, int index) {
  edge minEdge(predecessor[index], index);
  //cerr << "Testing edge (" << minEdge.first() << ", " << minEdge.second() << ")\n";
  double minScore = edgeScore(minEdge);
  double score;
  for (int i = predecessor[index]; i != index; i = predecessor[i]) {
    edge temp(predecessor[i], i);
    //cerr << "Testing edge (" << temp.first() << ", " << temp.second() << ")\n";
    score = edgeScore(temp);
    if (score < minScore) {
      minEdge = temp;
      minScore = score;
    }
  }
  return minEdge;
}

double lbSpanningTree::edgeScore(edge e) {
  if (_edgeScorer == NULL) {
    return _randomGenerator.RandomDouble(1);
  }
  return _edgeScorer(e, _edgeScoreAux);
}

edge lbSpanningTree::tryRemoveEdge(lbSpanningTree & tree, const edge & add) {
  map<double, edge> edges;
  bool* visited = new bool[tree.getMaxNode()];
  for (int i = 0; i < tree.getMaxNode(); ++i) {
    visited[i] = false;
  }
  int* predecessor = new int[tree.getMaxNode()];
  for (int i = 0; i < tree.getMaxNode(); ++i) {
    predecessor[i] = -1;
  }
  // We do a BFS starting from add.first(), setting its predecessor to be
  // add.second(). If the BFS ever returns to add.second(), we know there's a cycle
  visited[add.first()] = true;
  predecessor[add.first()] = add.second();
  queue <node> q;
  q.push(add.first());
  while (!q.empty()) {
    node curr = q.front();
    q.pop();
    const set<int> & adjacent = tree.getAdjacent(curr);
    for (set<int>::const_iterator iter = adjacent.begin(); iter != adjacent.end(); ++iter) {
      if (predecessor[curr] != *iter) {
	// Expanding the frontier rather than just seeing the previous node
	if (*iter == add.second()) {
	  edge toRemove(-1, -1);
// 	  if (lbOptions::isVerbose(V_INCREMENTAL)) {
// 	    cerr << "Spanning tree cycle involving " << *iter << " and " << curr << endl;
// 	    tree.printTree(cerr);
// 	    cerr << "Predecessor of " << *iter << " is " << curr << endl;
// 	  }
	  predecessor[*iter] = curr;
	  toRemove = lowEdgeOnCycle(predecessor, curr);
	  tree.removeEdge(toRemove);
	  delete[] visited;delete[] predecessor;
	  return toRemove;
	} else {
// 	  if (lbOptions::isVerbose(V_INCREMENTAL)) {
// 	    cerr << "Predecessor of " << *iter << " is " << curr << endl;
// 	  }
	  predecessor[*iter] = curr;
	  visited[*iter] = true;
	  q.push(*iter);
	}
      }
    }
  }
  delete[] visited;delete[] predecessor;
  return edge(-1, -1);
}
