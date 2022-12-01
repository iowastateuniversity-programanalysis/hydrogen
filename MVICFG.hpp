/**
 * @author Ashwin K J
 * @file
 * MVICFG : Grouping together auxiliary function for MVICFG
 */
#ifndef MVICFG_H
#define MVICFG_H

#include "Graph_Edge.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <list>
#include <llvm/IR/Instruction.h>
#include <map>
#include <set>
#include <string>
namespace hydrogen_framework {
/* Forward declaration */
class Diff_Mapping;
class Graph;
class Graph_Edge;
class Graph_Instruction;
class Graph_Line;
class Module;

/**
 * Build ICFG for the given module
 */
Graph *buildICFG(Module *mod, unsigned graphVersion);

/**
 * Generate Line Mappings between two modules
 */
std::list<Diff_Mapping> generateLineMapping(Module *firstMod, Module *secondMod);

/**
 * Get Graph_Line(s) from given source line
 * Returns empty list if no Graph_Line is not found
 */
std::list<Graph_Line *> getGraphLinesGivenLine(Graph *graph, long long lineNo, const std::string &fileName);

/**
 * Get predecessor of a given Graph_Line
 */
std::list<Graph_Line *> getPredGivenGraphLine(Graph_Line *line);

/**
 * Get successor of a given Graph_Line
 */
std::list<Graph_Line *> getSuccGivenGraphLine(Graph_Line *line);

/**
 * Get the OpCode of the Instructions in a Graph_Line as String in the order in which they appear
 * Returns empty string if none of the Graph_Instruction had Instruction_Ptr
 */
std::string getGraphLineInstructionsAsString(Graph_Line *line);

/**
 * Heuristically try to find the closest Graph_Line match from a list of potential Graph_Line matches
 * when no exact match is found using getGraphLineInstructionsAsString
 * Currently will throw an warning if heuristic skips more than 2 OpCode to match the lines
 * Returns NULL if no heuristic match is found
 */
Graph_Line *resolveMatchedLinesWithNoExactStringMatch(const std::list<Graph_Line *> &matchedLines,
                                                      const std::string &lineFromString, unsigned int graphVersion);

/**
 * Find matched Node
 * Returns NULL if no match found
 * Always make sure to check that the Graph_Line is from the diff being used
 */
Graph_Line *findMatchedLine(Graph_Line *t, Graph *matchTo, Graph *matchFrom, Diff_Mapping diff);

/**
 * Get the edge between two given nodes
 * Returns NULL if no match found
 */
Graph_Edge *getEdge(Graph_Instruction *fromNode, Graph_Instruction *toNode, Graph_Edge::edgeTypes type);

/**
 * Get the first edge between two Graph_Line
 * It does reverse propagation for Instructions of fromLine and forward propagation for toLine Instructions
 * Used only when getEdge fails to find an edge where one is expected
 * Returns NULL if no match is found
 */
Graph_Edge *getInBetweenEdge(Graph_Line *fromLine, Graph_Line *toLine);

/**
 * Get the newly added MVICFG Graph_Line corresponding to the given ICFG Graph_Line
 * Used only when findMatchedLine fails to retrieve the same
 * Returns NULL if no such line is found
 */
Graph_Line *getNewlyAdded(Graph *MVICFG, Graph *ICFG, Graph_Line *newLine, Diff_Mapping diff);

/**
 * Add nodes to MVICFG and returns the added MVICFG lines
 */
std::list<Graph_Line *> addToMVICFG(Graph *MVICFG, Graph *ICFG, Diff_Mapping diff, unsigned Version);

/**
 * Get matching Graph_Instruction from given Graph given a Graph_Instruction using LLVM PTR
 * Return NULL if no match is found
 */
Graph_Instruction *getMatchedInstructionFromGraph(Graph *graphToMatch, Graph_Instruction *instToMatch);

/**
 * Import edges from ICFG instruction for added Graph_Line
 */
void getEdgesForAddedLines(Graph *MVICFG, Graph *ICFG, const std::list<Graph_Line *> &addedLines,
                           const std::list<Diff_Mapping> &diffMap, unsigned Version);

/**
 * Mark deleted nodes in MVICFG and returns the deleted MVICFG lines
 */
std::list<Graph_Line *> deleteFromMVICFG(Graph *MVICFG, Graph *ICFG, Diff_Mapping diff, unsigned Version);

/**
 * Returns the corresponding matched Graph_Line in MVICFG from ICFG
 */
std::map<Graph_Line *, Graph_Line *> matchedInMVICFG(Graph *MVICFG, Graph *ICFG, Diff_Mapping diff, unsigned Version);

/**
 * Update the Edge and Node information for MVICFG
 */
void updateMVICFGVersion(Graph *MVICFG, std::list<Graph_Line *> addedLines, std::list<Graph_Line *> deletedLines,
                         std::list<Diff_Mapping> diffMap, unsigned Version);

/**
 * Calculate the number of added paths between two versions
 */
unsigned long long calculateAddedPaths(Graph *MVICFG, unsigned version1, unsigned version2);

/**
 * Calculate the number of deleted paths between two versions
 */
unsigned long long calculateDeletedPaths(Graph *MVICFG, unsigned version1, unsigned version2);

} // namespace hydrogen_framework
#endif
