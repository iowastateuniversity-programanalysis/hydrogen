/**
 * @author Evan Hellman
 * @file
 * Implementing Graph_Edge_Iterator.hpp
 */

#include "Graph_Edge_Iterator.hpp"
#include "Graph_Instruction.hpp"
#include "Graph_Line.hpp"
#include <ranges>

namespace hydrogen_framework {

Graph_Edge_Iterator::Graph_Edge_Iterator(Graph *graph, Graph_Function *graph_function) : graph(graph) {
  auto *entry = graph->findVirtualEntry(graph_function->getFunctionName());
  if (entry == nullptr) {
    this->current_edge = nullptr;
    return;
    //    throw std::runtime_error("Function does not exist in graph.");
  }

  auto outgoing_edges = entry->getInstructionEdges();
  for (auto edge_iter = outgoing_edges.rbegin(); edge_iter != outgoing_edges.rend(); edge_iter++) {
    this->previous_edges.emplace(*edge_iter);
  }

  this->current_edge = this->previous_edges.top();
  this->previous_edges.pop();
  this->visited_edges.emplace(this->current_edge);
}

Graph_Edge_Iterator &Graph_Edge_Iterator::operator++() noexcept {

  // If this iterator is already complete, do nothing
  if (this->current_edge == nullptr)
    return *this;

  auto is_outgoing = [this](auto *edge) {
    return this->current_edge->getEdgeTo() == edge->getEdgeFrom();
  };

  Graph_Edge *next_edge = nullptr;
  while (next_edge == nullptr) {

    // Search subsequent edges for a valid one
    auto edges = this->current_edge->getEdgeTo()->getInstructionEdges();
    for (auto *outgoing_edge : edges | std::views::filter(is_outgoing)) {
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
        this->current_edge = this->previous_edges.top();
        this->previous_edges.pop();
      }
    }
  }

  this->current_edge = next_edge;
  this->visited_edges.emplace(next_edge);

  return *this;
}

Graph_Edge *Graph_Edge_Iterator::operator*() const noexcept { return this->current_edge; }

} // namespace hydrogen_framework