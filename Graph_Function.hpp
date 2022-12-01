/**
 * @author Ashwin K J
 * @file
 * Graph_Function Class: Graph Data-structure for storing function source lines
 */
#ifndef GRAPH_FUNCTION_H
#define GRAPH_FUNCTION_H

#include <list>
#include <string>
namespace hydrogen_framework {
/* Forward declaration */
class Graph;
class Graph_Line;

/**
 * Graph_Function Class: Container for storing source line belonging to a function
 */
class Graph_Function {
public:
  /**
   * Constructor
   */
  Graph_Function(unsigned id) : functionID(id), funcGraph(nullptr) {}

  /**
   * Destructor
   */
  ~Graph_Function() { functionLines.clear(); }

  /**
   * Set functionName
   */
  void setFunctionName(std::string name) { functionName = name; }

  /**
   * Set functionFile
   */
  void setFunctionFile(std::string name) { functionFile = name; }

  /**
   * Return true if functionFile is not empty
   */
  bool isFunctionFileSet() { return !functionFile.empty(); }

  /**
   * Push Graph_Line at the back of the functionLines list
   */
  void pushFunctionLines(Graph_Line *line);

  /**
   * Push Graph_Line at the front of the functionLines list. Only used for Virtual node
   */
  void pushFrontFunctionLines(Graph_Line *line);

  /**
   * Return true if functionLines is empty
   */
  bool isFunctionLinesEmpty() { return functionLines.empty(); }

  /**
   * Return functionLines
   */
  std::list<Graph_Line *> getFunctionLines() { return functionLines; }

  /**
   * Return funcName
   */
  std::string getFunctionName() { return functionName; }

  /**
   * Return functionID
   */
  unsigned getFunctionID() { return functionID; }

  /**
   * Return functionFile
   */
  std::string getFunctionFile() { return functionFile; }

  /**
   * Set pointer to encompassing Graph
   */
  void setGraph(Graph *graph) { funcGraph = graph; }

  /**
   * Return pointer to encompassing Graph
   */
  Graph *getGraph() { return funcGraph; }

private:
  unsigned functionID;                   /**< Function Container ID */
  std::string functionName;              /**< Name of the function */
  std::string functionFile;              /**< Name of the file in which the function resides */
  std::list<Graph_Line *> functionLines; /**< Container for lines in the function */
  Graph *funcGraph;                      /**< Points to the Graph that encompasses this */
};                                       // End Graph_Function Class
} // namespace hydrogen_framework
#endif
