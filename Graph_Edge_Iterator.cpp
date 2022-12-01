/**
* @author Evan Hellman
* @file
* Implementing Graph_Edge_Iterator.hpp
 */

#include "Graph_Edge_Iterator.hpp"
#include "Graph_Instruction.hpp"

namespace hydrogen_framework {

Graph_Edge_Iterator& Graph_Edge_Iterator::operator++() noexcept {

  // If this iterator is already complete, do nothing
  if (this->current_edge == nullptr) return *this;

  auto outgoing_edges = this->current_edge->getEdgeTo()->getInstructionEdges();

  Graph_Edge* next_edge = nullptr;
  while (next_edge == nullptr) {

    // Search subsequent edges for a valid one
    for (Graph_Edge* outgoing_edge : outgoing_edges) {
      if (not this->visited_edges.contains(outgoing_edge)) {
        next_edge = outgoing_edge;
        this->previous_edges.emplace(this->current_edge);
        break;
      }
    }

    // If no valid subsequent edge was found
    if (next_edge == nullptr) {
      // If all nodes have been visited, we are done iterating
      if (this->previous_edges.empty()) {
        this->current_edge = nullptr;
        return *this;
      }
      // Otherwise, step back up the graph
      else {
        next_edge = this->previous_edges.top();
        this->previous_edges.pop();
      }
    }
  }

  this->current_edge = next_edge;
  this->visited_edges.emplace(next_edge);

  return *this;
}

Graph_Edge* Graph_Edge_Iterator::operator*() const noexcept {
  return this->current_edge;
}

} // namespace hydrogen