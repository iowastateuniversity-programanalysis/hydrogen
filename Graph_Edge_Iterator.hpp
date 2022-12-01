/**
* @author Evan Hellman
* @file
* Graph_Edge_Iterator Class: Class for iterating a Graph's Graph_Edges
 */

#ifndef HYDROGEN_DEPTHFIRSTGRAPHITERATOR_H
#define HYDROGEN_DEPTHFIRSTGRAPHITERATOR_H

#include <unordered_set>
#include "Graph.hpp"
#include "Graph_Edge.hpp"

namespace hydrogen_framework {

/**
 * Depth-first iterator for Graphs
 */
class GraphEdgeIterator {
public:

  /**
   * Constructor
   * Initialize ID to zero
   */
  explicit GraphEdgeIterator(Graph* graph)
      : graph(graph), current_edge(*graph->getGraphEdges().begin()) {
    this->visited_edges.emplace(this->current_edge);
  }

  /**
   * Increment operator for iterating list forward
   */
  GraphEdgeIterator &operator++() noexcept;

  /**
   * Get current edge
   */
  Graph_Edge* operator*() const noexcept;

private:
  Graph* graph;                                             /**< Graph that the iterator is iterating */
  std::stack<Graph_Edge*> previous_edges;                   /**< Stack of previous iterator edges */
  Graph_Edge* current_edge;                                 /**< Current edge that the iterator is on */
  std::unordered_set<Graph_Edge*> visited_edges;            /**< List of edges that have already been visited */
};

} // namespace hydrogen

#endif // HYDROGEN_DEPTHFIRSTGRAPHITERATOR_H