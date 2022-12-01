/**
 * @author Ashwin K J
 * @file
 * Implementing Graph_Edge.hpp
 */
#include "Graph_Edge.hpp"
#include "Graph_Instruction.hpp"
namespace hydrogen_framework {
std::string Graph_Edge::getPrintableEdgeVersions() {
  std::string ver;
  for (auto v : edgeVersions) {
    ver += "V" + std::to_string(v) + ",";
  } // End loop for edgeVersions
  ver.pop_back();
  return ver;
} // End getPrintableEdgeVersions

bool Graph_Edge::isPartOfGraph(unsigned graphVersion) {
  auto findVer = std::find_if(std::begin(edgeVersions), std::end(edgeVersions),
                              [=](unsigned ver) { return (ver == graphVersion); });
  if (findVer != edgeVersions.end()) {
    return true;
  } // End loop for edgeVersions
  return false;
} // End isPartOfGraph
} // namespace hydrogen_framework
