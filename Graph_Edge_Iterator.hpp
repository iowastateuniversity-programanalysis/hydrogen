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
#include "Graph_Function.hpp"
#include "Graph_Instruction.hpp"

namespace hydrogen_framework {

/**
 * Depth-first iterator for Graphs
 */
class Graph_Edge_Iterator {
public:

  /**
   * Constructor
   * Initialize current edge to the first edge
   */
  explicit Graph_Edge_Iterator(Graph* graph, Graph_Function* graph_function);

  /**
   * Increment operator for iterating list forward
   */
  Graph_Edge_Iterator &operator++() noexcept;

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