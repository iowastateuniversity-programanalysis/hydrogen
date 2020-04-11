/**
 * @author Ashwin K J
 * @file
 * Implementing MVICFG.hpp
 */
#include "MVICFG.hpp"
#include "Diff_Mapping.hpp"
#include "Graph.hpp"
#include "Graph_Edge.hpp"
#include "Graph_Function.hpp"
#include "Graph_Instruction.hpp"
#include "Graph_Line.hpp"
#include "Module.hpp"
namespace hydrogen_framework {

// DFS from instruction downwards, skipping vertices that have been visited, and
// print the path when we get back to an instruction that is of another previous version.
int dfsPath(Graph *MVICFG,
  Graph_Instruction *instruction,
  std::set<Graph_Instruction *> &visited,
  std::set<Graph_Instruction *> &newInstructions,
  bool verbose = false)
{
  if (verbose) {
    std::cout << instruction->getInstructionLabel() << std::endl;
  }
  visited.insert(instruction);

  int pathCount = 0;

  for (auto edge :instruction->getInstructionEdges()) {
    auto leadingOut = edge->getEdgeFrom()->getInstructionID() == instruction->getInstructionID();
    if (leadingOut) {
      auto to = getMatchedInstructionFromGraph(MVICFG, edge->getEdgeTo());

      // End the path since we've joined back up with a vertex in the old version
      if (newInstructions.find(to) == newInstructions.end()) {
        if (verbose) {
          std::cout << to->getInstructionLabel() << std::endl;
          std::cout << "Done with path!" << std::endl;
        }
        pathCount += 1;
        continue;
      }

      // Don't proceed if we've seen this one before
      if (visited.find(to) != visited.end()) {
        pathCount += 1;
        continue;
      }

      pathCount += dfsPath(MVICFG, to, visited, newInstructions, verbose);
    }
  }

  return pathCount;
}

int reportPaths(Graph *MVICFG, std::list<Graph_Line *> lines, bool verbose) {
  std::list<std::list<Graph_Instruction *>> paths;
  std::set<Graph_Instruction *> visited;
  std::set<Graph_Instruction *> newInstructions;
  
  for (auto line: lines) {
    for (auto instr : line->getLineInstructions()) {
      auto matchedInstruction = getMatchedInstructionFromGraph(MVICFG, instr);
      newInstructions.insert(matchedInstruction);
    }
  }

  int pathCount = 0;

  for (auto instruction: newInstructions) {
    // Don't proceed if we've seen this one before.
    if (visited.find(instruction) == visited.end()) {
      auto edges = instruction->getInstructionEdges();
      auto edgeLeadingIn = std::find_if(edges.begin(), edges.end(), [=](Graph_Edge *e){
        return e->getEdgeTo()->getInstructionID() == instruction->getInstructionID();
      });
      if (edgeLeadingIn == edges.end()) {
        std::cerr << "Couldn't get edge leading in." << std::endl;
        continue;
      }

      if (verbose) {
        std::cout << (*edgeLeadingIn)->getEdgeFrom()->getInstructionLabel() << std::endl;
      }
      pathCount += dfsPath(MVICFG, instruction, visited, newInstructions, verbose);
    }
  }
  
  return pathCount;
}

Graph *buildICFG(Module *mod, unsigned graphVersion) {
  std::unique_ptr<llvm::Module> &modPtr = mod->getPtr();
  Graph *ICFG = new Graph(graphVersion);
  for (llvm::Function &F : (*modPtr)) {
    std::string funcName;
    Graph_Function *funcGraph = new Graph_Function(ICFG->getNextID());
    if (F.hasName()) {
      funcName = F.getName();
    } else {
      funcName = "Unknown_Function";
    } // End check for function name
    funcGraph->setFunctionName(funcName);
    Graph_Line *currentLineGraph = new Graph_Line(graphVersion);
    for (llvm::BasicBlock &BB : F) {
      for (llvm::Instruction &I : BB) {
        unsigned int DILocLine = 0;
        std::string DIFile = "Unknown_File";
        getLocationInfo(I, DILocLine, DIFile);
        /* Attach the line to current line if no debug information is found */
        if (DILocLine == 0) {
          DILocLine = currentLineGraph->getLineNumber(graphVersion);
        } // End check for DILocLine
        /* Create new Graph_Line container whenever new DILocLine is encountered */
        if (DILocLine != currentLineGraph->getLineNumber(graphVersion)) {
          if (!currentLineGraph->isLineInstructionEmpty()) {
            funcGraph->pushFunctionLines(currentLineGraph);
            ICFG->addSeqEdges(currentLineGraph);
          } // End check for isLineInstructionEmpty
          currentLineGraph = new Graph_Line(graphVersion);
        } // End check for continuation for current line
        if (!funcGraph->isFunctionFileSet()) {
          funcGraph->setFunctionFile(DIFile);
        } // End check for isFunctionFileSet
        currentLineGraph->setLineNumber(graphVersion, DILocLine);
        std::string instLabel;
        llvm::raw_string_ostream rInstLabel(instLabel);
        I.print(rInstLabel);
        Graph_Instruction *currentInstGraph = new Graph_Instruction();
        currentInstGraph->setInstructionLabel(instLabel);
        currentInstGraph->setInstructionID(ICFG->getNextID());
        llvm::Instruction *iTmp = &I;
        currentInstGraph->setInstructionPtr(iTmp);
        currentLineGraph->pushLineInstruction(currentInstGraph);
      } // End loop for BasicBlock
    }   // End loop for Function
    if (!currentLineGraph->isLineInstructionEmpty()) {
      funcGraph->pushFunctionLines(currentLineGraph);
      ICFG->addSeqEdges(currentLineGraph);
    } // End check for isLineInstructionEmpty
    if (!funcGraph->isFunctionLinesEmpty()) {
      ICFG->pushGraphFunction(funcGraph);
      ICFG->addVirtualNodes(funcGraph);
    } // End check for isFunctionLinesEmpty
  }   // End loop for Module
  ICFG->addBranchEdges();
  ICFG->addFunctionCallEdges();
  ICFG->printGraph("Graph_" + std::to_string(graphVersion));
  return ICFG;
} // End buildICFG

std::list<Diff_Mapping> generateLineMapping(Module *firstMod, Module *secondMod) {
  std::list<Diff_Mapping> diffMap;
  std::list<std::string> processedFiles;
  /* Process files from first module */
  for (auto iterFile : (firstMod)->getFiles()) {
    std::list<std::string> nextModuleFiles = (secondMod)->getFiles();
    auto fileMatch = std::find_if(std::begin(nextModuleFiles), std::end(nextModuleFiles), [=](std::string f) {
      return (boost::filesystem::path(f).filename() == boost::filesystem::path(iterFile).filename());
    });
    Diff_Mapping::sequence ALines, BLines;
    if (fileMatch != nextModuleFiles.end()) {
      /* Matching file exist */
      processedFiles.push_back(boost::filesystem::path(iterFile).filename().c_str());
      std::ifstream Aifs(iterFile.c_str());
      std::ifstream Bifs((*fileMatch).c_str());
      Diff_Mapping::elem buf;
      while (getline(Aifs, buf)) {
        ALines.push_back(buf);
      } // End loop for Aifs
      while (getline(Bifs, buf)) {
        BLines.push_back(buf);
      } // End loop for Bifs
    } else {
      /* File no longer exist */
      processedFiles.push_back(boost::filesystem::path(iterFile).filename().c_str());
      std::ifstream Aifs(iterFile.c_str());
      Diff_Mapping::elem buf;
      while (getline(Aifs, buf)) {
        ALines.push_back(buf);
      } // End loop for Aifs
    }   // End check for nextModuleFiles
    Diff_Util diff(ALines, BLines);
    diff.compose();
    Diff_Ses s = diff.getSes();
    Diff_Mapping file(boost::filesystem::path(iterFile).filename().c_str());
    file.putMapping(s.getSequence());
    /* file.printMapping(); */
    diffMap.push_back(file);
  } // End loop for first module file processing
  /* Check for new files in next module */
  for (auto iterFile : (secondMod)->getFiles()) {
    auto fileMatch = std::find_if(std::begin(processedFiles), std::end(processedFiles),
                                  [=](std::string f) { return (f == boost::filesystem::path(iterFile).filename()); });
    if (fileMatch == processedFiles.end()) {
      /* New file exist */
      processedFiles.push_back(boost::filesystem::path(iterFile).filename().c_str());
      Diff_Mapping::sequence ALines, BLines;
      std::ifstream Bifs(iterFile.c_str());
      Diff_Mapping::elem buf;
      while (getline(Bifs, buf)) {
        BLines.push_back(buf);
      } // End loop for Bifs
      Diff_Util diff(ALines, BLines);
      diff.compose();
      Diff_Ses s = diff.getSes();
      Diff_Mapping file(boost::filesystem::path(iterFile).filename().c_str());
      file.putMapping(s.getSequence());
      /* file.printMapping(); */
      diffMap.push_back(file);
    } // End check for processedFiles
  }   // End loop for second module file processing
  return diffMap;
} // End generateLineMapping

std::list<Graph_Line *> getGraphLinesGivenLine(Graph *graph, long long lineNo, std::string fileName) {
  std::list<Graph_Line *> graphLines;
  bool foundLine = false;
  for (auto func : graph->getGraphFunctions()) {
    /* Matching with correct diff File */
    if (func->getFunctionFile() == fileName) {
      for (auto line : func->getFunctionLines()) {
        /* Matching line of diff */
        if (line->getLineNumber(graph->getGraphVersion()) == lineNo) {
          graphLines.push_back(line);
          foundLine = true;
        } // End check for lineNo
      }   // End loop for line
      /* Same line cannot be spread across functions. Hence stop search if at least one line found */
      if (foundLine) {
        return graphLines;
      } // End check for foundLine
    }   // End check for fileName
  }     // End loop for Functions
  return graphLines;
} // End getGraphLinesGivenLine

std::list<Graph_Line *> getPredGivenGraphLine(Graph_Line *line) {
  std::list<Graph_Line *> pred;
  Graph_Instruction *frontInst = line->getLineInstructions().front();
  std::list<Graph_Edge *> edges = frontInst->getInstructionEdges();
  for (auto iter : edges) {
    if (iter->getEdgeTo() == frontInst) {
      pred.push_back(iter->getEdgeFrom()->getGraphLine());
    } // End check for frontInst
  }   // End loop for edges
  return pred;
} // End getPredGivenGraphLine

std::list<Graph_Line *> getSuccGivenGraphLine(Graph_Line *line) {
  std::list<Graph_Line *> succ;
  Graph_Instruction *backInst = line->getLineInstructions().back();
  std::list<Graph_Edge *> edges = backInst->getInstructionEdges();
  for (auto iter : edges) {
    if (iter->getEdgeFrom() == backInst) {
      succ.push_back(iter->getEdgeTo()->getGraphLine());
    } // End check for backInst
  }   // End loop for edges
  return succ;
} // End getSuccGivenGraphLine

std::string getGraphLineInstructionsAsString(Graph_Line *line) {
  std::string lineString;
  /* Iterate through the Graph_Line and make a string representation of the Instruction OpCode */
  for (auto inst : line->getLineInstructions()) {
    /* If Ptr is not found, then it won't be present in the other version as well */
    if (inst->getInstructionPtr() != NULL) {
      lineString.append(inst->getInstructionPtr()->getOpcodeName()).append(" ");
    } // End check for Instruction Ptr
  }   // End loop for Graph_Line
  if (!lineString.empty()) {
    lineString.pop_back();
  } // End check for empty string before remove trailing space
  return lineString;
} // End getGraphLineInstructionsAsString

Graph_Line *resolveMatchedLinesWithNoExtactStringMatch(std::list<Graph_Line *> matchedLines, std::string lineFromString,
                                                       unsigned int graphVersion) {
  int minDiff = std::numeric_limits<int>::max();
  Graph_Line *tmp = NULL;
  for (auto line : matchedLines) {
    /* First remove matched OpCodes */
    std::string lineToString = getGraphLineInstructionsAsString(line);
    std::size_t pos = lineToString.find(lineFromString);
    if (pos != std::string::npos) {
      lineToString.erase(pos, lineFromString.length());
      boost::trim(lineToString);
    } // End check for match in lineToString
    /* Get the OpCodes remaining */
    int countOpCode = 0;
    if (!lineToString.empty()) {
      countOpCode = 1;
      for (auto ch : lineToString) {
        if (ch == ' ') {
          ++countOpCode;
        } // End check for space
      }   // End loop for counting space
    }     // End check for empty lineToString after removing matches
    if (countOpCode < minDiff) {
      minDiff = countOpCode;
      tmp = line;
    } // End check to update minDiff
  }   // End loop for matchedLines
  if (minDiff > 2) {
    std::cerr << "The heuristically matched line for " << tmp->getLineNumber(graphVersion) << "might be incorrect\n";
  } // End check for minDiff
  return tmp;
} // End resolveMatchedLinesWithNoExtactStringMatch

Graph_Line *findMatchedLine(Graph_Line *t, Graph *matchTo, Graph *matchFrom, Diff_Mapping diff) {
  /* Extra check to ensure correct diff File */
  if (diff.getFileName() != t->getGraphFunction()->getFunctionFile()) {
    std::cerr << "findMatchedLine is using wrong diff File\n";
    std::cerr << "Skipping match for " << t->getLineNumber(matchFrom->getGraphVersion()) << " from "
              << matchFrom->getGraphVersion() << " to " << matchTo->getGraphVersion() << "\n";
    return NULL;
  } // End check for diff File name
  unsigned lineFrom = t->getLineNumber(matchFrom->getGraphVersion());
  unsigned lineTo = 0;
  /* Check for virtual node */
  if (matchFrom->isVirtualNodeLineNumber(lineFrom)) {
    /* Virtual nodes have same unique line number across graphs*/
    lineTo = lineFrom;
  } else {
    if (matchTo->getGraphVersion() > matchFrom->getGraphVersion()) {
      /* Matching MVICFG to ICFG */
      if (lineFrom == 0) {
        /* New lines will have line numbers already in them */
        lineTo = t->getLineNumber(matchTo->getGraphVersion());
      } else {
        lineTo = diff.getAfterLineNumber(lineFrom);
      } // End check for lineFrom
    } else {
      /* Matching ICFG to MVICFG */
      lineTo = diff.getBeforeLineNumber(lineFrom);
      if (lineTo == 0) {
        /* Line was deleted, but match is requested. Set lineTo to max
         * and let the calling function deal with it */
        lineTo = std::numeric_limits<unsigned>::max();
      } // End check for lineTo
    }   // End check for matchTo > matchFrom
  }     // End check for isVirtualNodeLineNumber
  if (lineTo != std::numeric_limits<unsigned>::max()) {
    for (auto func : matchTo->getGraphFunctions()) {
      if (func->getFunctionFile() == t->getGraphFunction()->getFunctionFile()) {
        if (func->getFunctionName() == t->getGraphFunction()->getFunctionName()) {
          std::list<Graph_Line *> matchedLines;
          for (auto line : func->getFunctionLines()) {
            if (lineTo == line->getLineNumber(matchTo->getGraphVersion())) {
              std::string lineToString = getGraphLineInstructionsAsString(line);
              std::string lineFromString = getGraphLineInstructionsAsString(t);
              if (lineToString == lineFromString) {
                return line;
              } else {
                matchedLines.push_back(line);
              } // End check for lineTo and lineFrom string
            }   // End check for matching line number
          }     // End loop for lines
          /* If there is a match at this point heuristically match it rather than return NULL */
          if (!matchedLines.empty()) {
            /* If only one match is there, then we don't have to work much */
            if (matchedLines.size() == 1) {
              return matchedLines.front();
            } else {
              std::string lineFromString = getGraphLineInstructionsAsString(t);
              return resolveMatchedLinesWithNoExtactStringMatch(matchedLines, lineFromString,
                                                                matchTo->getGraphVersion());
            } // End check for matchedLines size
          }   // End check for empty matchedLines
        }     // End check for Function name check
      }       // End check for File name
    }         // End loop for functions
  }           // End check for virtual node check
  return NULL;
} // End findMatchedLine

Graph_Edge *getEdge(Graph_Instruction *fromNode, Graph_Instruction *toNode, Graph_Edge::edgeTypes type) {
  for (auto edge : fromNode->getInstructionEdges()) {
    if (edge->getEdgeFrom() == fromNode) {
      if (edge->getEdgeTo() == toNode) {
        if (type == edge->getEdgeType()) {
          return edge;
        } else if (type == Graph_Edge::ANY) {
          return edge;
        } // End check for ANY
      }   // End check for toNode
    }     // End check for fromNode
  }       // End loop for Instructions
  return NULL;
} // End getEdge

Graph_Edge *getInBetweenEdge(Graph_Line *fromLine, Graph_Line *toLine) {
  std::list<Graph_Instruction *> fromLineInstructions = fromLine->getLineInstructions();
  for (auto fromLineInstIter = fromLineInstructions.rbegin(); fromLineInstIter != fromLineInstructions.rend();
       ++fromLineInstIter) {
    Graph_Instruction *fromLineInst = *fromLineInstIter;
    for (auto toLineInstIter : toLine->getLineInstructions()) {
      Graph_Edge *checkEdge = getEdge(fromLineInst, toLineInstIter, Graph_Edge::ANY);
      if (checkEdge) {
        return checkEdge;
      } // End check for checkEdge
    }   // End loop for toLine
  }     // End loop for fromLine
  return NULL;
} // End getInBetweenEdge

Graph_Line *getNewlyAdded(Graph *MVICFG, Graph *ICFG, Graph_Line *newLine, Diff_Mapping diff) {
  std::list<long long> addedLines = diff.getAddedLines();
  auto findAdd = std::find_if(std::begin(addedLines), std::end(addedLines),
                              [=](long long no) { return (no == newLine->getLineNumber(ICFG->getGraphVersion())); });
  if (findAdd != addedLines.end()) {
    for (auto func : MVICFG->getGraphFunctions()) {
      /* Compare line number within same file */
      if (func->getFunctionFile() == newLine->getGraphFunction()->getFunctionFile()) {
        for (auto line : func->getFunctionLines()) {
          if (newLine->getLineNumber(ICFG->getGraphVersion()) == line->getLineNumber(ICFG->getGraphVersion())) {
            return line;
          } // End check for line and newLine numbers
        }   // End loop for line
      }     // End check for function file
    }       // End loop for function
  }         // End check for addLines.end
  return NULL;
} // End getNewlyAdded

std::list<Graph_Line *> addToMVICFG(Graph *MVICFG, Graph *ICFG, Diff_Mapping diff, unsigned Version) {
  std::list<long long> addedLines = diff.getAddedLines();
  std::string fileName = diff.getFileName();
  std::list<Graph_Line *> N;
  std::list<Graph_Line *> icfgN;
  /*Identify all added lines */
  for (auto line : addedLines) {
    std::list<Graph_Line *> addedGraphLines;
    addedGraphLines = getGraphLinesGivenLine(ICFG, line, fileName);
    if (addedGraphLines.empty()) {
      std::cerr << "Graph_Line for line " << line << ":" << fileName << " not found in ICFG Ver " << Version << "\n";
      std::cerr << "Skipping this line and continuing\n";
      continue;
    } // End check for addedGraphLines
    for (auto addedLine : addedGraphLines) {
      Graph_Function *func = addedLine->getGraphFunction();
      /* Get corresponding MVICFG Graph_Function */
      std::list<Graph_Function *> mvicfgFunctions = MVICFG->getGraphFunctions();
      auto findMvicfgFunc =
          std::find_if(std::begin(mvicfgFunctions), std::end(mvicfgFunctions), [=](Graph_Function *mvicfgfunc) {
            return mvicfgfunc->getFunctionName() == func->getFunctionName();
          });
      /* Create new one if it doesn't exist */
      Graph_Function *mvicfgFunc;
      if (findMvicfgFunc == mvicfgFunctions.end()) {
        mvicfgFunc = new Graph_Function(MVICFG->getNextID());
        Graph_Function *mvicfgFunc = new Graph_Function(MVICFG->getNextID());
        mvicfgFunc->setFunctionName(func->getFunctionName());
        mvicfgFunc->setFunctionFile(func->getFunctionFile());
      } else {
        mvicfgFunc = *findMvicfgFunc;
      } // End check for findMvicfgFunc
      /* Iterating through addedLine and adding instructions to MVICFG */
      Graph_Line *newLine = new Graph_Line(ICFG->getGraphVersion());
      newLine->setLineNumber(MVICFG->getGraphVersion(), 0);
      newLine->setLineNumber(ICFG->getGraphVersion(), addedLine->getLineNumber(ICFG->getGraphVersion()));
      for (auto inst : addedLine->getLineInstructions()) {
        Graph_Instruction *newInstruction = new Graph_Instruction();
        newInstruction->setInstructionLabel(inst->getInstructionLabel());
        newInstruction->setInstructionID(MVICFG->getNextID());
        newInstruction->setInstructionPtr(inst->getInstructionPtr());
        newLine->pushLineInstruction(newInstruction);
      } // End loop for adding instructions
      mvicfgFunc->pushFunctionLines(newLine);
      N.push_back(newLine);
      icfgN.push_back(addedLine);
    } // End loop for processing addedGraphLines
  }   // End loop for identifying added lines
  for (auto n : N) {
    /* Proceed only if the function is in diff File being processed */
    if (n->getGraphFunction()->getFunctionFile() == fileName) {
      Graph_Line *nDash = findMatchedLine(n, ICFG, MVICFG, diff);
      if (!nDash) {
        std::cerr << "ICFG line corresponding to the added MVICFG line " << n->getLineNumber(ICFG->getGraphVersion())
                  << " not found\n";
        continue;
      } // End check for nDash
      std::list<Graph_Line *> pred = getPredGivenGraphLine(nDash);
      std::list<Graph_Line *> succ = getSuccGivenGraphLine(nDash);
      std::list<Graph_Line *> T;
      T.insert(T.end(), pred.begin(), pred.end());
      T.insert(T.end(), succ.begin(), succ.end());
      for (auto t : T) {
        auto findT = std::find_if(std::begin(icfgN), std::end(icfgN), [=](Graph_Line *N) { return (N == t); });
        if (findT == icfgN.end()) {
          /* t in T but not in N */
          /* Proceed only if the function is in diff File being processed */
          if (t->getGraphFunction()->getFunctionFile() == fileName) {
            Graph_Line *tDash = findMatchedLine(t, MVICFG, ICFG, diff);
            if (tDash) {
              auto findTPred = std::find_if(std::begin(pred), std::end(pred), [=](Graph_Line *N) { return (N == t); });
              auto findTSucc = std::find_if(std::begin(succ), std::end(succ), [=](Graph_Line *N) { return (N == t); });
              if (findTPred != pred.end()) {
                Graph_Instruction *nInst = n->getLineInstructions().front();
                Graph_Instruction *tDashInst = tDash->getLineInstructions().back();
                /* Check before adding the edge and if edge exist only add the version */
                Graph_Edge *checkEdge = getEdge(tDashInst, nInst, Graph_Edge::ANY);
                if (!checkEdge) {
                  /* Get edge type from ICFG */
                  Graph_Instruction *nDashInst = nDash->getLineInstructions().front();
                  Graph_Instruction *tInst = t->getLineInstructions().back();
                  Graph_Edge *getEdgeType = getEdge(tInst, nDashInst, Graph_Edge::ANY);
                  Graph_Edge::edgeTypes edgeType;
                  if (getEdgeType) {
                    edgeType = getEdgeType->getEdgeType();
                  } else {
                    bool foundEdge = false;
                    Graph_Edge *checkBetweenEdge = getInBetweenEdge(t, nDash);
                    if (checkBetweenEdge) {
                      foundEdge = true;
                      edgeType = checkBetweenEdge->getEdgeType();
                    } // End check for checkBetweenEdge
                    if (!foundEdge) {
                      std::cerr << "ICFG edge between " << tInst->getInstructionLabel() << " and "
                                << nDashInst->getInstructionLabel() << " not found\n";
                      std::cerr << "Setting edge type to MVICFG_ADD\n";
                      edgeType = Graph_Edge::MVICFG_ADD;
                    } // End check for foundEdge
                  }   // End check for getEdgeType
                  Graph_Edge *newEdge = new Graph_Edge(tDashInst, nInst, edgeType, Version);
                  MVICFG->addEdge(tDashInst, nInst, newEdge);
                } else {
                  checkEdge->pushEdgeVersions(Version);
                } // End check for checkEdge
              } else if (findTSucc != succ.end()) {
                Graph_Instruction *tDashInst = tDash->getLineInstructions().front();
                Graph_Instruction *nInst = n->getLineInstructions().back();
                /* Check before adding the edge and if edge exist only add the version */
                Graph_Edge *checkEdge = getEdge(nInst, tDashInst, Graph_Edge::ANY);
                if (!checkEdge) {
                  /* Get edge type from ICFG */
                  Graph_Instruction *tInst = t->getLineInstructions().front();
                  Graph_Instruction *nDashInst = nDash->getLineInstructions().back();
                  Graph_Edge *getEdgeType = getEdge(nDashInst, tInst, Graph_Edge::ANY);
                  Graph_Edge::edgeTypes edgeType;
                  if (getEdgeType) {
                    edgeType = getEdgeType->getEdgeType();
                  } else {
                    bool foundEdge = false;
                    Graph_Edge *checkBetweenEdge = getInBetweenEdge(nDash, t);
                    if (checkBetweenEdge) {
                      foundEdge = true;
                      edgeType = checkBetweenEdge->getEdgeType();
                    } // End check for checkBetweenEdge
                    if (!foundEdge) {
                      std::cerr << "ICFG edge between " << tInst->getInstructionLabel() << " and "
                                << nDashInst->getInstructionLabel() << " not found\n";
                      std::cerr << "Setting edge type to MVICFG_ADD\n";
                      edgeType = Graph_Edge::MVICFG_ADD;
                    } // End check for foundEdge
                  }   // End check for edgeType
                  Graph_Edge *newEdge = new Graph_Edge(nInst, tDashInst, edgeType, Version);
                  MVICFG->addEdge(nInst, tDashInst, newEdge);
                } else {
                  checkEdge->pushEdgeVersions(Version);
                } // End check for checkEdge
              }   // End check for Predecessor & Successors
            } else {
              std::cerr << "No matching line found for " << t->getLineNumber(ICFG->getGraphVersion())
                        << " in MVICFG(A)\n";
            } // End check for tDash
          }   // End check to see if the function is in the same diff file
        }     // End check for find T
      }       // End loop for T
    }         // End check to see if the function is in the same diff file
  }           // End loop for adding edges for added lines
  /* Return the added lines */
  return N;
} // End addToMVICFG

Graph_Instruction *getMatchedInstructionFromGraph(Graph *graphToMatch, Graph_Instruction *instToMatch) {
  for (auto func : graphToMatch->getGraphFunctions()) {
    for (auto line : func->getFunctionLines()) {
      std::list<Graph_Instruction *> lineInstList = line->getLineInstructions();
      std::_List_iterator<Graph_Instruction *> findInst;
      if (instToMatch->getInstructionPtr() == NULL) {
        /* This is a virtual node and they always share their line numbers */
        unsigned instToLineNumber = instToMatch->getGraphLine()->getLineNumber(graphToMatch->getGraphVersion());
        findInst = std::find_if(std::begin(lineInstList), std::end(lineInstList), [=](Graph_Instruction *inst) {
          return (inst->getGraphLine()->getLineNumber(graphToMatch->getGraphVersion()) == instToLineNumber);
        });
      } else {
        findInst = std::find_if(std::begin(lineInstList), std::end(lineInstList), [=](Graph_Instruction *inst) {
          return (inst->getInstructionPtr() == instToMatch->getInstructionPtr());
        });
      } // End check for instToMatch
      if (findInst != lineInstList.end()) {
        return *findInst;
      } // End check for findInst
    }   // End loop for line
  }     // End loop for func
  return NULL;
} // End getMatchedInstructionFromGraph

void getEdgesForAddedLines(Graph *MVICFG, Graph *ICFG, std::list<Graph_Line *> addedLines,
                           std::list<Diff_Mapping> diffMap, unsigned Version) {
  for (auto line : addedLines) {
    for (auto lineInst : line->getLineInstructions()) {
      Graph_Instruction *lineDashInst = getMatchedInstructionFromGraph(ICFG, lineInst);
      if (!lineDashInst) {
        std::cerr << "No match found in ICFG for instruction " << lineInst->getInstructionLabel() << "\n";
        std::cerr << "Skipping Instruction\n";
        continue;
      } // End check for lineDashInst
      for (auto edgeDash : lineDashInst->getInstructionEdges()) {
        Graph_Instruction *fromDash = edgeDash->getEdgeFrom();
        Graph_Instruction *from = getMatchedInstructionFromGraph(MVICFG, fromDash);
        if (!from) {
          /* This edge would have been added by addToMVICFG */
          continue;
        } // End check for from
        Graph_Instruction *toDash = edgeDash->getEdgeTo();
        Graph_Instruction *to = getMatchedInstructionFromGraph(MVICFG, toDash);
        if (!to) {
          /* This edge would have been added by addToMVICFG */
          continue;
        } // End check for to
        Graph_Edge *checkEdge = getEdge(from, to, edgeDash->getEdgeType());
        if (!checkEdge) {
          Graph_Edge *newEdge = new Graph_Edge(from, to, edgeDash->getEdgeType(), ICFG->getGraphVersion());
          MVICFG->addEdge(from, to, newEdge);
        } // End check for checkEdge
      }   // End loop for adding edges
    }     // End loop for lineInst
  }       // End loop for line
} // End getEdgesForAddedLines

std::list<Graph_Line *> deleteFromMVICFG(Graph *MVICFG, Graph *ICFG, Diff_Mapping diff, unsigned Version) {
  std::list<long long> deletedLines = diff.getDeletedLines();
  std::string fileName = diff.getFileName();
  std::list<Graph_Line *> N;
  /* Identify all deleted lines */
  for (auto line : deletedLines) {
    std::list<Graph_Line *> deletedGraphLines;
    deletedGraphLines = getGraphLinesGivenLine(MVICFG, line, fileName);
    if (deletedGraphLines.empty()) {
      std::cerr << "Graph_Line for line " << line << " not found in MVICFG\n";
      std::cerr << "Skipping this line and continuing\n";
      continue;
    } // End check for deletedGraphLines
    for (auto deleteLine : deletedGraphLines) {
      /* Mark as deleted in ICFG version */
      deleteLine->setLineNumber(ICFG->getGraphVersion(), 0);
      /* Add deleted line to N */
      N.push_back(deleteLine);
    } // End loop for processing deletedGraphLines
  }   // End loop for identifying the deleted lines
  for (auto func : MVICFG->getGraphFunctions()) {
    /* Proceed only if the function is in diff File being processed */
    if (func->getFunctionFile() == fileName) {
      for (auto n : func->getFunctionLines()) {
        auto findLine = std::find_if(std::begin(N), std::end(N), [=](Graph_Line *N) { return (N == n); });
        if (findLine == N.end()) {
          /* n not in N but in MVICFG */
          std::list<Graph_Line *> pred = getPredGivenGraphLine(n);
          std::list<Graph_Line *> succ = getSuccGivenGraphLine(n);
          std::list<Graph_Line *> T;
          T.insert(T.end(), pred.begin(), pred.end());
          T.insert(T.end(), succ.begin(), succ.end());
          for (auto t : T) {
            auto findT = std::find_if(std::begin(N), std::end(N), [=](Graph_Line *N) { return (N == t); });
            if (findT != N.end()) {
              /* n has a successor or predecessor in N */
              /* Proceed only if the function is in diff File being processed */
              if (n->getGraphFunction()->getFunctionFile() == fileName) {
                Graph_Line *nDash = findMatchedLine(n, ICFG, MVICFG, diff);
                if (!nDash) {
                  /* Check if 'n' exist in MVICFG currently, otherwise nDash won't exist obviously */
                  if (n->getLineNumber(MVICFG->getGraphVersion()) == 0) {
                    continue;
                  } // End check for n's line number
                }   // End check for nDash
                if (nDash) {
                  std::list<Graph_Line *> predDash = getPredGivenGraphLine(nDash);
                  std::list<Graph_Line *> succDash = getSuccGivenGraphLine(nDash);
                  std::list<Graph_Line *> MDash;
                  MDash.insert(MDash.end(), predDash.begin(), predDash.end());
                  MDash.insert(MDash.end(), succDash.begin(), succDash.end());
                  for (auto mDash : MDash) {
                    /* Proceed only if the function is in diff File being processed */
                    if (mDash->getGraphFunction()->getFunctionFile() == fileName) {
                      Graph_Line *m = findMatchedLine(mDash, MVICFG, ICFG, diff);
                      if (!m) {
                        /* Check if it was newly added ICFG line */
                        m = getNewlyAdded(MVICFG, ICFG, mDash, diff);
                      } // End check for m
                      if (m) {
                        auto findMPred = std::find_if(std::begin(predDash), std::end(predDash),
                                                      [=](Graph_Line *N) { return (N == mDash); });
                        auto findMSucc = std::find_if(std::begin(succDash), std::end(succDash),
                                                      [=](Graph_Line *N) { return (N == mDash); });
                        if (findMPred != predDash.end()) {
                          /* Check for edge between m and n */
                          Graph_Instruction *mInst = m->getLineInstructions().back();
                          Graph_Instruction *nInst = n->getLineInstructions().front();
                          Graph_Edge *checkEdge = getEdge(mInst, nInst, Graph_Edge::ANY);
                          if (!checkEdge) {
                            /* Get edge type from ICFG */
                            Graph_Instruction *nDashInst = nDash->getLineInstructions().back();
                            Graph_Instruction *mDashInst = mDash->getLineInstructions().front();
                            Graph_Edge *getEdgeType = getEdge(mDashInst, nDashInst, Graph_Edge::ANY);
                            Graph_Edge::edgeTypes edgeType;
                            if (getEdgeType) {
                              edgeType = getEdgeType->getEdgeType();
                            } else {
                              bool foundEdge = false;
                              Graph_Edge *checkBetweenEdge = getInBetweenEdge(mDash, nDash);
                              if (checkBetweenEdge) {
                                foundEdge = true;
                                edgeType = checkBetweenEdge->getEdgeType();
                              } // End check for checkBetweenEdge
                              if (!foundEdge) {
                                std::cerr << "ICFG edge between " << mInst->getInstructionLabel() << " and "
                                          << nInst->getInstructionLabel() << " not found\n";
                                std::cerr << "Setting edge type to MVICFG_DEL\n";
                                edgeType = Graph_Edge::MVICFG_DEL;
                              } // End check for foundEdge
                            }   // End check for getEdgeType
                            Graph_Edge *newEdge = new Graph_Edge(mInst, nInst, edgeType, Version);
                            MVICFG->addEdge(mInst, nInst, newEdge);
                          } // End check for checkEdge
                        } else if (findMSucc != succDash.end()) {
                          /* Check for edge between n and m */
                          Graph_Instruction *nInst = n->getLineInstructions().back();
                          Graph_Instruction *mInst = m->getLineInstructions().front();
                          Graph_Edge *checkEdge = getEdge(nInst, mInst, Graph_Edge::ANY);
                          if (!checkEdge) {
                            /* Get edge type from ICFG */
                            Graph_Instruction *nDashInst = nDash->getLineInstructions().back();
                            Graph_Instruction *mDashInst = mDash->getLineInstructions().front();
                            Graph_Edge *getEdgeType = getEdge(nDashInst, mDashInst, Graph_Edge::ANY);
                            Graph_Edge::edgeTypes edgeType;
                            if (getEdgeType) {
                              edgeType = getEdgeType->getEdgeType();
                            } else {
                              bool foundEdge = true;
                              Graph_Edge *checkBetweenEdge = getInBetweenEdge(nDash, mDash);
                              if (checkBetweenEdge) {
                                foundEdge = true;
                                edgeType = checkBetweenEdge->getEdgeType();
                              } // End check for checkBetweenEdge
                              if (!foundEdge) {
                                std::cerr << "ICFG edge between " << nInst->getInstructionLabel() << " and "
                                          << mInst->getInstructionLabel() << " not found\n";
                                std::cerr << "Setting edge type to MVICFG_DEL\n";
                                edgeType = Graph_Edge::MVICFG_DEL;
                              } // End check for foundEdge
                            }   // End check for getEdgeType
                            Graph_Edge *newEdge = new Graph_Edge(nInst, mInst, edgeType, Version);
                            MVICFG->addEdge(nInst, mInst, newEdge);
                          } // End check for checkEdge
                        }   // End check for Predecessors and Successors
                      } else {
                        std::cerr << "No matching line found for " << mDash->getLineNumber(ICFG->getGraphVersion())
                                  << " in MVICFG(D)\n";
                      } // End check for m
                    }   // End check to see if the function is in the same diff file
                  }     // End loop for MDash
                } else {
                  std::cerr << "No matching line found for " << n->getLineNumber(MVICFG->getGraphVersion())
                            << " in ICFG(" << ICFG->getGraphVersion() << ")\n";
                  std::cerr << "file : " << fileName << "\n";
                } // End check for nDash
              }   // End check to see if the function is in the same diff file
            }     // End check for T in N
          }       // End loop for T
        }         // End check for findLine in N
      }           // End loop for n
    }             // End check to see if the function is in the same diff file
  }               // End loop for adding edges for deleted lines
  /* Return the deleted MVICFG lines */
  return N;
} // End deleteFromMVICFG

std::map<Graph_Line *, Graph_Line *> matchedInMVICFG(Graph *MVICFG, Graph *ICFG, Diff_Mapping diff, unsigned Version) {
  std::map<long long, long long> matchedLines = diff.getMatchedLines();
  std::string fileName = diff.getFileName();
  std::map<Graph_Line *, Graph_Line *> matchedGraphLines;
  std::list<Graph_Line *> mvicfgM;
  /* Identify all the matched lines */
  for (auto line : matchedLines) {
    long long mvicfgLineNo = line.first;
    long long icfgLineNo = line.second;
    std::list<Graph_Line *> mvicfgGraphLines;
    mvicfgGraphLines = getGraphLinesGivenLine(MVICFG, mvicfgLineNo, fileName);
    /* No need to worry about matching lines that are not in MVICFG */
    if (!mvicfgGraphLines.empty()) {
      std::list<Graph_Line *> icfgGraphLines;
      icfgGraphLines = getGraphLinesGivenLine(ICFG, icfgLineNo, fileName);
      if (icfgGraphLines.empty()) {
        std::cerr << "Graph_Line for line " << icfgLineNo << " not found in ICFG\n";
        std::cerr << "Skipping this line and continuing\n";
        continue;
      } // End check for if line is present in ICFG
      if (mvicfgGraphLines.size() != icfgGraphLines.size()) {
        std::cerr << "Mismatch between the number of MVICFG and ICFG Graph_Lines for (" << line.first
                  << "::" << line.second << ")\n";
        std::cerr << "Skipping this line and continuing\n";
        continue;
      } // End check for mismatch in GraphLine size
      for (auto mvicfgLine = mvicfgGraphLines.begin(), icfgLine = icfgGraphLines.begin();
           mvicfgLine != mvicfgGraphLines.end() && icfgLine != icfgGraphLines.end(); ++mvicfgLine, ++icfgLine) {
        matchedGraphLines.insert(std::pair<Graph_Line *, Graph_Line *>(*icfgLine, *mvicfgLine));
        mvicfgM.push_back(*icfgLine);
      } // End loop for processing mvicfgGraphLines & icfgGraphLines
    }   // End check for if line is preset in MVICFG
  }     //  End loop for matchedLines
  for (auto line : matchedGraphLines) {
    Graph_Line *n = line.second;
    Graph_Line *nDash = line.first;
    std::list<Graph_Line *> pred = getPredGivenGraphLine(nDash);
    std::list<Graph_Line *> succ = getSuccGivenGraphLine(nDash);
    std::list<Graph_Line *> T;
    T.insert(T.end(), pred.begin(), pred.end());
    T.insert(T.end(), succ.begin(), succ.end());
    for (auto t : T) {
      auto findT = std::find_if(std::begin(mvicfgM), std::end(mvicfgM), [=](Graph_Line *N) { return (N == t); });
      if (findT != mvicfgM.end()) {
        /* t in T and in Matched */
        /* Proceed only if the function is in diff File being processed */
        if (t->getGraphFunction()->getFunctionFile() == fileName) {
          Graph_Line *tDash = findMatchedLine(t, MVICFG, ICFG, diff);
          if (tDash) {
            auto findTPred = std::find_if(std::begin(pred), std::end(pred), [=](Graph_Line *N) { return (N == t); });
            auto findTSucc = std::find_if(std::begin(succ), std::end(succ), [=](Graph_Line *N) { return (N == t); });
            if (findTPred != pred.end()) {
              Graph_Instruction *nInst = n->getLineInstructions().front();
              Graph_Instruction *tDashInst = tDash->getLineInstructions().back();
              /* Edge should exist in the MVICFG. Raise error otherwise */
              Graph_Edge *checkEdge = getEdge(tDashInst, nInst, Graph_Edge::ANY);
              bool foundEdge = false;
              if (!checkEdge) {
                Graph_Edge *checkBetweenEdge = getInBetweenEdge(tDash, n);
                if (checkBetweenEdge) {
                  foundEdge = true;
                  checkEdge = checkBetweenEdge;
                } // End checkBetweenEdge
                std::cerr << "MVICFG edge between " << tDashInst->getInstructionLabel() << " and "
                          << nInst->getInstructionLabel() << " not found\n";
                std::cerr << "Skipping this predecessor edge\n";
              } else {
                foundEdge = true;
              } // End check for checkEdge
              if (foundEdge) {
                if (!checkEdge->isPartOfGraph(Version)) {
                  checkEdge->pushEdgeVersions(Version);
                } // End check for isPartOfGraph
              }   // End check for foundEdge
            } else if (findTSucc != succ.end()) {
              Graph_Instruction *tDashInst = tDash->getLineInstructions().front();
              Graph_Instruction *nInst = n->getLineInstructions().back();
              /* Edge should exist in the MVICFG. Raise error otherwise */
              Graph_Edge *checkEdge = getEdge(nInst, tDashInst, Graph_Edge::ANY);
              bool foundEdge = false;
              if (!checkEdge) {
                Graph_Edge *checkBetweenEdge = getInBetweenEdge(n, tDash);
                if (checkBetweenEdge) {
                  foundEdge = true;
                  checkEdge = checkBetweenEdge;
                } // End check for checkBetweenEdge
                if (!foundEdge) {
                  std::cerr << "MVICFG edge between " << tDashInst->getInstructionLabel() << " and "
                            << nInst->getInstructionLabel() << " not found\n";
                  std::cerr << "Skipping this successor edge\n";
                } // End check for foundEdge
              } else {
                foundEdge = true;
              } // End check for checkEdge
              if (foundEdge) {
                if (!checkEdge->isPartOfGraph(Version)) {
                  checkEdge->pushEdgeVersions(Version);
                } // End check for isPartOfGraph
              }   // End check for foundEdge
            }     // End check for Successor
          } else {
            std::cerr << "No matching line found for " << t->getLineNumber(ICFG->getGraphVersion())
                      << " in MVICFG(M)\n";
          } // End check for tDash
        }   // End check to see if the function is in the same diff file
      }     // End check for find T
    }       // End loop for T
  }         // End loop for adding edges for matched lines
  return matchedGraphLines;
} // End matchedInMVICFG

void updateMVICFGVersion(Graph *MVICFG, std::list<Graph_Line *> addedLines, std::list<Graph_Line *> deletedLines,
                         std::list<Diff_Mapping> diffMap, unsigned Version) {
  /* Update Graph_Line information */
  for (auto func : MVICFG->getGraphFunctions()) {
    auto findDiff = std::find_if(std::begin(diffMap), std::end(diffMap),
                                 [=](Diff_Mapping d) { return (d.getFileName() == func->getFunctionFile()); });
    if (findDiff != diffMap.end()) {
      for (auto line : func->getFunctionLines()) {
        auto findInAdd = std::find_if(std::begin(addedLines), std::end(addedLines),
                                      [=](Graph_Line *addLine) { return (addLine == line); });
        auto findInDel = std::find_if(std::begin(deletedLines), std::end(deletedLines),
                                      [=](Graph_Line *delLine) { return (delLine == line); });
        if (findInAdd == addedLines.end() && findInDel == deletedLines.end()) {
          /* Line was neither added nor deleted */
          unsigned oldLineNumber = line->getLineNumber(MVICFG->getGraphVersion());
          if (!MVICFG->isVirtualNodeLineNumber(oldLineNumber)) {
            /* Substitute if the line is present in the MVICFG. Otherwise, it's mapping was done earlier */
            if (oldLineNumber != 0) {
              unsigned newLineNumber = findDiff->getAfterLineNumber(oldLineNumber);
              if (newLineNumber != std::numeric_limits<unsigned>::max()) {
                line->setLineNumber(Version, newLineNumber);
              } else {
                std::cerr << "Incorrect update line for " << oldLineNumber << "\n";
              } // End check for newLineNumber being max
            }   // End check for oldLineNumber
          } else {
            /* Assign the same number in this version also */
            unsigned newLineNumber = oldLineNumber;
            line->setLineNumber(Version, newLineNumber);
          } // End check for isVirtualNodeLineNumber
        }   // End check for deleted and added line
      }     // End loop for line
    } else {
      if (func->getFunctionFile() == "External_Node_File") {
        /* Assign the same number in this version also */
        for (auto line : func->getFunctionLines()) {
          unsigned oldLineNumber = line->getLineNumber(MVICFG->getGraphVersion());
          unsigned newLineNumber = oldLineNumber;
          line->setLineNumber(Version, newLineNumber);
        } // End loop for line
        continue;
      } // End check for External Node
      std::cerr << "No Line mapping found for " << func->getFunctionFile() << "\n";
    } // End check for diffMap.end
  }   // End loop for updating Graph_Line information
  /* Collect all nodes from which a new edge has originated for this version */
  std::list<Graph_Instruction *> mvicfgAddEdgesNodes;
  for (auto mvicfgEdge : MVICFG->getGraphEdges()) {
    Graph_Line *toLine = mvicfgEdge->getEdgeTo()->getGraphLine();
    auto findInAddToLine = std::find_if(std::begin(addedLines), std::end(addedLines),
                                        [=](Graph_Line *addLine) { return (addLine == toLine); });
    /* Collect all the Nodes from which an new edge for this version originates */
    if (findInAddToLine != addedLines.end()) {
      mvicfgAddEdgesNodes.push_back(mvicfgEdge->getEdgeFrom());
    } // End check for addedLines.end
  }   // End loop for collecting Graph_Instruction
  for (auto edge : MVICFG->getGraphEdges()) {
    Graph_Instruction *edgeFromInst = edge->getEdgeFrom();
    if (edgeFromInst->getGraphLine()->getLineNumber(Version) != 0) {
      /* The from Node is active for this version */
      Graph_Instruction *edgeToInst = edge->getEdgeTo();
      if (edgeToInst->getGraphLine()->getLineNumber(Version) != 0) {
        /* The to Node is active for this version */
        auto findInAddEgdeFrom =
            std::find_if(std::begin(mvicfgAddEdgesNodes), std::end(mvicfgAddEdgesNodes),
                         [=](Graph_Instruction *instComp) { return (edge->getEdgeFrom() == instComp); });
        if (findInAddEgdeFrom == mvicfgAddEdgesNodes.end()) {
          /* Neither the From node or To node were part of added edges */
          if (!edge->isPartOfGraph(Version)) {
            edge->pushEdgeVersions(Version);
          } // End check for isPartOfGraph
        }   // End if for findInAddEgdeFrom
      }     // End check for edgeToInst
    }       // End check for edgeFromInst
  }         // End loop for updating Graph_Edge information
} // End updateMVICFGVersion
} // namespace hydrogen_framework
