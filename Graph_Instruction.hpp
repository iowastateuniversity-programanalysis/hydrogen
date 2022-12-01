/**
 * @author Ashwin K J
 * @file
 * Graph_Instruction Class: Graph Data-structure for LLVM instructions
 */
#ifndef GRAPH_INSTRUCTION_H
#define GRAPH_INSTRUCTION_H

#include <list>
#include <llvm/IR/Module.h>
#include <set>
#include <utility>
namespace hydrogen_framework {
/* Forward declaration */
class Graph_Edge;
class Graph_Line;
class Query;

/**
 * Graph_Instruction Class: To store individual LLVM instructions
 */
class Graph_Instruction {
public:
  /**
   * Constructor
   */
  Graph_Instruction() : instructionID(0), instructionPtr(nullptr), instructionLine(nullptr) {}

  /**
   * Destructor
   */
  ~Graph_Instruction() = default;

  /**
   * Set instructionID
   */
  void setInstructionID(unsigned ID) { instructionID = ID; }

  /**
   * Set instructionLabel
   */
  void setInstructionLabel(std::string label) { instructionLabel = std::move(label); }

  /**
   * Set instructionPtr
   */
  void setInstructionPtr(llvm::Instruction *I) { instructionPtr = I; }

  /**
   * Push Graph_Edge into instructionEdges list
   */
  void pushEdgeInstruction(Graph_Edge *edge) { instructionEdges.push_back(edge); }

  /**
   * Return instructionLabel
   */
  std::string getInstructionLabel() { return instructionLabel; }

  /**
   * Return instructionID
   */
  unsigned getInstructionID() const { return instructionID; }

  /**
   * Get instructionPtr
   * Can return NULL
   */
  llvm::Instruction *getInstructionPtr() { return instructionPtr; }

  /**
   * Return instructionEdges
   */
  std::list<Graph_Edge *> getInstructionEdges() { return instructionEdges; }

  /**
   * Set pointer to encompassing Graph_Line
   */
  void setGraphLine(Graph_Line *line) { instructionLine = line; }

  /**
   * Return pointer to encompassing Graph_Line
   */
  Graph_Line *getGraphLine() { return instructionLine; }

  /**
   * Return instructionVisitedQueries
   */
  std::set<Query *> getInstructionVisitedQueries() { return instructionVisitedQueries; }

  /**
   * Insert query as pointer into instructionVisitedQueries
   */
  void insertInstructionVisitedQueries(Query *q) { instructionVisitedQueries.insert(q); }

private:
  unsigned instructionID;                      /**< Instruction ID */
  std::string instructionLabel;                /**< Instruction label or text */
  llvm::Instruction *instructionPtr;           /**< Instruction LLVM Pointer */
  std::list<Graph_Edge *> instructionEdges;    /**< Container for edges in the instruction */
  Graph_Line *instructionLine;                 /**< Points to the Graph_Line that encompasses this */
  std::set<Query *> instructionVisitedQueries; /**< Container for Queries that have visited this */
};                                             // End Graph_Instruction Class
} // namespace hydrogen_framework
#endif
