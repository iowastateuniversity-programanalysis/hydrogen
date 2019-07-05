/**
 * @author Ashwin K J
 * @file
 * Implementing Graph_Line.hpp
 */
#include "Graph_Line.hpp"
#include "Graph_Instruction.hpp"

namespace hydrogen_framework {

void Graph_Line::setLineNumber(unsigned Version, unsigned line) {
  lineNumber.insert(std::pair<unsigned, unsigned>(Version, line));
} // End setLineNumber

void Graph_Line::pushLineInstruction(Graph_Instruction *inst) {
  inst->setGraphLine(this);
  lineInstructions.push_back(inst);
} // End pushLineInstruction;

unsigned Graph_Line::getLineNumber(unsigned Version) {
  auto searchLine = lineNumber.find(Version);
  if (searchLine != lineNumber.end()) {
    return searchLine->second;
  } // End check for searchLine
  return 0;
} // End getLineNumber
} // namespace hydrogen_framework
