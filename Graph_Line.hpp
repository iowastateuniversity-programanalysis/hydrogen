/**
 * @author Ashwin K J
 * @file
 * Graph_Line Class: Graph Data-structure for source line
 */
#ifndef GRAPH_LINE_H
#define GRAPH_LINE_H

#include <list>
#include <map>
namespace hydrogen_framework {
/* Forward declaration */
class Graph_Function;
class Graph_Instruction;

/**
 * Graph_Line Class: Container for storing LLVM instruction belonging to a line
 */
class Graph_Line {
public:
  /**
   * Constructor
   */
  Graph_Line(unsigned Version) : lineFunction(nullptr), lineGraphVersion(Version) {}

  /**
   * Destructor
   */
  ~Graph_Line() { lineInstructions.clear(); }

  /**
   * Get lineNumber given a version
   * Returns zero if no mapping found
   */
  unsigned getLineNumber(unsigned Version);

  /**
   * Set lineNumber
   */
  void setLineNumber(unsigned Version, unsigned line);

  /**
   * Return true if lineInstructions is empty
   */
  bool isLineInstructionEmpty() { return lineInstructions.empty(); }

  /**
   * Push the Graph_Instruction at the back of the list
   */
  void pushLineInstruction(Graph_Instruction *inst);

  /**
   * Return lineInstructions
   */
  std::list<Graph_Instruction *> getLineInstructions() { return lineInstructions; }

  /**
   * Set pointer to encompassing Graph_Function
   */
  void setGraphFunction(Graph_Function *func) { lineFunction = func; }

  /**
   * Return pointer to encompassing Graph_Line
   */
  Graph_Function *getGraphFunction() { return lineFunction; }

  /**
   * Return lineGraphVersion
   */
  unsigned getLineGraphVersion() { return lineGraphVersion; }

private:
  std::map<unsigned, unsigned> lineNumber;         /**< Map between graphVersion and line Number */
  std::list<Graph_Instruction *> lineInstructions; /**< Container for instruction in the line */
  Graph_Function *lineFunction;                    /**< Points to the Graph_Function that encompasses this */
  unsigned lineGraphVersion;                       /**< The graph version in which this line was introduced */
};                                                 // End Graph_Line Class
} // namespace hydrogen_framework
#endif
