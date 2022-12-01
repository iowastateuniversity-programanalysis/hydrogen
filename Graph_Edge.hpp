/**
 * @author Ashwin K J
 * @file
 * Graph_Edge Class: Graph Data-structure Edges
 */
#ifndef GRAPH_EDGE_H
#define GRAPH_EDGE_H

#include <list>
#include <string>
namespace hydrogen_framework {
/* Forward Declaration */
class Graph_Instruction;

/**
 * Graph_Edge Class: Class for storing edge information
 */
class Graph_Edge {
public:
  /**
   * Constructor
   */
  Graph_Edge() : edgeFrom(nullptr), edgeTo(nullptr), edgeType(edgeTypes::ANY) {}

  /**
   * Enumeration for type of edges
   */
  enum edgeTypes { SEQUENTIAL, BRANCH, CALL, EXTERNAL_CALL, VIRTUAL, MVICFG_ADD, MVICFG_DEL, ANY };

  /**
   * Alternate constructor
   */
  Graph_Edge(Graph_Instruction *from, Graph_Instruction *to, edgeTypes type, unsigned ver)
      : edgeFrom(from), edgeTo(to), edgeType(type) {
    edgeVersions.push_back(ver);
  }

  /**
   * Destructor
   */
  ~Graph_Edge() { edgeVersions.clear(); }

  /**
   * Set edgeFrom
   */
  void setEdgeFrom(Graph_Instruction *I) { edgeFrom = I; }

  /**
   * Set edgeTo
   */
  void setEdgeTo(Graph_Instruction *I) { edgeTo = I; }

  /**
   * Set edgeType
   */
  void setEdgeType(edgeTypes type) { edgeType = type; }

  /**
   * Push version to back of edgeVersions
   */
  void pushEdgeVersions(unsigned int ver) { edgeVersions.push_back(ver); }

  /**
   * Return edgeFrom
   */
  Graph_Instruction *getEdgeFrom() { return edgeFrom; }

  /**
   * Return edgeTo
   */
  Graph_Instruction *getEdgeTo() { return edgeTo; }

  /**
   * Return edgeType
   */
  edgeTypes getEdgeType() { return edgeType; }

  /**
   * Return edgeVersions
   */
  std::list<unsigned> getEdgeVersions() { return edgeVersions; }

  /**
   * Get printable edgeVersions
   */
  std::string getPrintableEdgeVersions();

  /**
   * Check if the edge is already part of a given graph Version
   * Return TRUE only if the given graphVersion is contained in edgeVersions
   */
  bool isPartOfGraph(unsigned graphVersion);

private:
  Graph_Instruction *edgeFrom;      /**< From Instruction */
  Graph_Instruction *edgeTo;        /**< To Instruction */
  edgeTypes edgeType;               /**< Edge Type */
  std::list<unsigned> edgeVersions; /**< Container to store edge's versions */
};                                  // End Graph_Edge Class
} // namespace hydrogen_framework
#endif
