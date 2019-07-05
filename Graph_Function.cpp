/**
 * @author Ashwin K J
 * @file
 * Implementing Graph_Function.hpp
 */
#include "Graph_Function.hpp"
#include "Graph_Line.hpp"

namespace hydrogen_framework {
void Graph_Function::pushFunctionLines(Graph_Line *line) {
  line->setGraphFunction(this);
  functionLines.push_back(line);
} // End pushFunctionLines

void Graph_Function::pushFrontFunctionLines(Graph_Line *line) {
  line->setGraphFunction(this);
  functionLines.push_front(line);
} // End pushFrontFunctionLines
} // namespace hydrogen_framework
