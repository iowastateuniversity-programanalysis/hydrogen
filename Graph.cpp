/**
 * @author Ashwin K J
 * @file
 * Implementing Graph.hpp
 */
#include "Graph.hpp"
#include "Graph_Edge.hpp"
#include "Graph_Function.hpp"
#include "Graph_Instruction.hpp"
#include "Graph_Line.hpp"
namespace hydrogen_framework {
void Graph::pushGraphFunction(Graph_Function *func) {
  func->setGraph(this);
  graphFunctions.push_back(func);
} // End pushGraphFunction

void Graph::addEdge(Graph_Instruction *from, Graph_Instruction *to, Graph_Edge *edge) {
  from->pushEdgeInstruction(edge);
  to->pushEdgeInstruction(edge);
  pushGraphEdges(edge);
} // End addEdge

void Graph::addSeqEdges(Graph_Line *line) {
  std::list<Graph_Instruction *> instructions = line->getLineInstructions();
  for (auto inst = instructions.begin(), instEnd = instructions.end(); inst != instEnd; ++inst) {
    /* Double check to make sure it is not Br */
    llvm::Instruction *llvmInst = (*inst)->getInstructionPtr();
    if (llvmInst) {
      if (llvmInst->getOpcode() == llvm::Instruction::Br) {
        continue;
      } // End check for Br
    }   // End check for llvmInst
    auto nextInst = std::next(inst);
    if (nextInst != instEnd) {
      Graph_Edge *seqEdge = new Graph_Edge(*inst, *nextInst, Graph_Edge::SEQUENTIAL, graphVersion);
      addEdge(*inst, *nextInst, seqEdge);
    } // End check for instEnd
  }   // End loop for inst
} // End addSeqEdges

Graph_Instruction *Graph::findMatchedInstruction(llvm::Instruction *matchInst) {
  for (auto func : graphFunctions) {
    for (auto line : func->getFunctionLines()) {
      for (auto inst : line->getLineInstructions()) {
        if (inst->getInstructionPtr() == matchInst) {
          return inst;
        } // End check for matchInst
      }   // End loop for inst
    }     // End loop for line
  }       // End loop for func
  return NULL;
} // End findMatchedInstruction

Graph_Instruction *Graph::findVirtualEntry(std::string funcName) {
  for (auto func : graphFunctions) {
    if (func->getFunctionName() == funcName) {
      for (auto line : func->getFunctionLines()) {
        for (auto inst : line->getLineInstructions()) {
          if (inst->getInstructionLabel().find("Entry::") != std::string::npos) {
            return inst;
          } // End check for matchInst
        }   // End loop for inst
      }     // End loop for line
    }       // End check for function name
  }         // End loop for func
  return NULL;
} // End findVirtualEntry

Graph_Instruction *Graph::findVirtualExit(std::string funcName) {
  for (auto func : graphFunctions) {
    if (func->getFunctionName() == funcName) {
      for (auto line : func->getFunctionLines()) {
        for (auto inst : line->getLineInstructions()) {
          if (inst->getInstructionLabel().find("Exit::") != std::string::npos) {
            return inst;
          } // End check for Exit
        }   // End loop for inst
      }     // End loop for line
    }       // End check for function name
  }         // End loop for func
  return NULL;
} // End findVirtualExit

void Graph::addBranchEdges() {
  for (auto func : graphFunctions) {
    std::list<Graph_Line *> lines = func->getFunctionLines();
    for (auto line = lines.begin(); line != lines.end(); ++line) {
      std::list<Graph_Instruction *> instructions = (*line)->getLineInstructions();
      for (auto inst = instructions.begin(); inst != instructions.end(); ++inst) {
        llvm::Instruction *I = (*inst)->getInstructionPtr();
        if (I) {
          /* Adding edges for BB with multiple successors */
          if (I->isTerminator()) {
            unsigned int noSucc = I->getNumSuccessors();
            for (unsigned int iterSucc = 0; iterSucc < noSucc; ++iterSucc) {
              llvm::Instruction *iSucc = llvm::dyn_cast<llvm::Instruction>(I->getSuccessor(iterSucc)->begin());
              Graph_Instruction *iSuccInst = findMatchedInstruction(iSucc);
              if (iSucc) {
                Graph_Edge *branchEdge = new Graph_Edge(*inst, iSuccInst, Graph_Edge::BRANCH, graphVersion);
                addEdge(*inst, iSuccInst, branchEdge);
              } else {
                std::cerr << "No matching Graph_Instruction found for edge from " << (*inst)->getInstructionLabel()
                          << "\n";
              } // End check for iSucc
            }   // End loop for iterSucc
          } else if ((*inst)->getInstructionID() == instructions.back()->getInstructionID()) {
            /* Adding Unique successors */
            auto nextLine = std::next(line);
            if (nextLine != lines.end()) {
              std::list<Graph_Instruction *> nextInstructions = (*nextLine)->getLineInstructions();
              auto nextI = nextInstructions.begin();
              if (nextI != nextInstructions.end()) {
                Graph_Edge *seqEdge = new Graph_Edge(*inst, *nextI, Graph_Edge::SEQUENTIAL, graphVersion);
                addEdge(*inst, *nextI, seqEdge);
              } // End check for nextI
            }   // End check for nextLine
          }     // End check for TerminatorInst
        }       // End check for I
      }         // End loop for inst
    }           // End llop for line
  }             // End loop for func
} // End addBranchEdges

void Graph::addFunctionCallEdges() {
  /* External Node */
  Graph_Function *virtualNodeFunc = new Graph_Function(getNextID());
  virtualNodeFunc->setFunctionFile("External_Node_File");
  virtualNodeFunc->setFunctionName("External_Node_Func");
  Graph_Line *virtualNodeLine = new Graph_Line(graphVersion);
  virtualNodeLine->setLineNumber(graphVersion, graphEntryID);
  Graph_Instruction *externalNode = new Graph_Instruction();
  externalNode->setInstructionID(getNextID());
  externalNode->setInstructionLabel("External_Node");
  externalNode->setInstructionPtr(NULL);
  virtualNodeLine->pushLineInstruction(externalNode);
  virtualNodeFunc->pushFunctionLines(virtualNodeLine);
  pushGraphFunction(virtualNodeFunc);
  std::list<std::string> funcNotFoud;
  /* Get the whitelisted function names and merge with funcNotFoud */
  std::list<std::string> whiteListedFunc = Graph::getWhiteList();
  funcNotFoud.insert(funcNotFoud.end(), whiteListedFunc.begin(), whiteListedFunc.end());
  for (auto func : graphFunctions) {
    for (auto line : func->getFunctionLines()) {
      for (auto inst : line->getLineInstructions()) {
        llvm::Instruction *I = inst->getInstructionPtr();
        if (I) {
          if (auto callSite = llvm::CallSite(I)) {
            const llvm::Function *Callee = callSite.getCalledFunction();
            if (!Callee || !llvm::Intrinsic::isLeaf(Callee->getIntrinsicID())) {
              /* Call Extern */
              Graph_Edge *callEdge = new Graph_Edge(inst, externalNode, Graph_Edge::EXTERNAL_CALL, graphVersion);
              addEdge(inst, externalNode, callEdge);
            } else if (!Callee->isIntrinsic()) {
              /* Add Edge based on function name */
              bool noEntry = false;
              bool noExit = false;
              if (!Callee->getName().empty()) {
                std::string funcName = Callee->getName();
                /* Call site to Entry */
                Graph_Instruction *virtualEntry = findVirtualEntry(funcName);
                if (virtualEntry) {
                  Graph_Edge *callEdge = new Graph_Edge(inst, virtualEntry, Graph_Edge::CALL, graphVersion);
                  addEdge(inst, virtualEntry, callEdge);
                } else {
                  noEntry = true;
                } // End check for virtualEntry
                /* Exit to Call site */
                Graph_Instruction *virtualExit = findVirtualExit(funcName);
                if (virtualExit) {
                  Graph_Edge *callEdge = new Graph_Edge(virtualExit, inst, Graph_Edge::CALL, graphVersion);
                  addEdge(virtualExit, inst, callEdge);
                } else {
                  noExit = true;
                } // End check for virtualExit
                auto findFunc = std::find_if(std::begin(funcNotFoud), std::end(funcNotFoud),
                                             [=](std::string name) { return name == funcName; });
                if (findFunc == funcNotFoud.end()) {
                  if (noEntry && noExit) {
                    funcNotFoud.push_back(funcName);
                    std::cerr << "Call edges not formed for " << funcName << "\n";
                  } else if (noEntry) {
                    std::cerr << "No Virtual Entry found for " << funcName << "\n";
                  } else if (noExit) {
                    std::cerr << "No Virtual Exit found for " << funcName << "\n";
                  } // End check for noEntry & noExit combinations
                }   // End check for findFunc
              } else {
                std::cerr << "Unknown function call from Instruction " << inst->getInstructionLabel() << "\n";
              } // End check for Callee name
            }   // End check for Callee Intrinsic
          }     // End check for callSite
        }       // End check for I
      }         // End loop for inst
    }           // End loop for line
  }             // End loop for func
} // End addFunctionCallEdges

void Graph::addVirtualNodes(Graph_Function *func) {
  std::string funcName = func->getFunctionName();
  Graph_Line *virtualLine = new Graph_Line(graphVersion);
  /* Entry Node */
  virtualLine->setLineNumber(graphVersion, graphEntryID);
  Graph_Instruction *virtualNode = new Graph_Instruction();
  virtualNode->setInstructionID(getNextID());
  virtualNode->setInstructionLabel("Entry::" + funcName);
  virtualNode->setInstructionPtr(NULL);
  virtualLine->pushLineInstruction(virtualNode);
  auto *to = func->getFunctionLines().front()->getLineInstructions().front();
  func->pushFrontFunctionLines(virtualLine);
  Graph_Edge *virtualEdgeEntry = new Graph_Edge(virtualNode, to, Graph_Edge::VIRTUAL, graphVersion);
  addEdge(virtualNode, to, virtualEdgeEntry);
  /* Exit Node */
  virtualLine = new Graph_Line(graphVersion);
  virtualLine->setLineNumber(graphVersion, graphExitID);
  virtualNode = new Graph_Instruction();
  virtualNode->setInstructionID(getNextID());
  virtualNode->setInstructionLabel("Exit::" + funcName);
  virtualNode->setInstructionPtr(NULL);
  virtualLine->pushLineInstruction(virtualNode);
  auto *from = func->getFunctionLines().back()->getLineInstructions().back();
  func->pushFunctionLines(virtualLine);
  Graph_Edge *virtualEdgeExit = new Graph_Edge(from, virtualNode, Graph_Edge::VIRTUAL, graphVersion);
  addEdge(from, virtualNode, virtualEdgeExit);
} // End addVirtualNodes

void Graph::printGraph(std::string graphName) {
  std::ofstream gFile(graphName + ".dot", std::ios::trunc);
  if (!gFile.is_open()) {
    std::cerr << "Unable to open file for printing the output\n";
    return;
  } // End check for gFile
  /* Initialize graph */
  gFile << "digraph \"MVICFG\" {\n";
  gFile << "\tlabel=\"" << graphName << "\";\n";
  /* Generating Nodes */
  gFile << "/* Generating Nodes */\n";
  for (auto func : graphFunctions) {
    gFile << "\tsubgraph cluster_" << func->getFunctionID() << " {\n";
    gFile << "\t\tlabel=\"" << func->getFunctionName() << "\";\n";
    for (auto line : func->getFunctionLines()) {
      for (auto inst : line->getLineInstructions()) {
        std::string outputString = std::regex_replace(inst->getInstructionLabel(), std::regex("\""), "\\\"");
        gFile << "\t\t\"" << inst->getInstructionID() << "\" [label=\"" << line->getLineNumber(graphVersion)
              << "::" << outputString << "\"];\n";
      } // End loop for inst
    }   // End loop for line
    gFile << "\t}\n";
  } // End loop for func
  /* Generating Edges*/
  gFile << "\n/* Generating Edges */\n";
  for (auto edge : graphEdges) {
    std::string outputString = "\t\t\"" + std::to_string(edge->getEdgeFrom()->getInstructionID()) + "\" -> \"" +
                               std::to_string(edge->getEdgeTo()->getInstructionID());
    switch (edge->getEdgeType()) {
    case Graph_Edge::SEQUENTIAL:
      outputString += "\" [arrowhead = normal, penwidth = 1.0, color = black, label=\"" +
                      edge->getPrintableEdgeVersions() + "\"];\n";
      break;
    case Graph_Edge::BRANCH:
      outputString += "\" [arrowhead = dot, penwidth = 1.0, color = black, label=\"" +
                      edge->getPrintableEdgeVersions() + "::Branch\"];\n";
      break;
    case Graph_Edge::VIRTUAL:
      outputString += "\" [arrowhead = normal, penwidth = 1.0, color = pink, label=\"" +
                      edge->getPrintableEdgeVersions() + "::Virtual\"];\n";
      break;
    case Graph_Edge::CALL:
      outputString += "\" [arrowhead = odot, penwidth = 1.0, color = blue, label=\"" +
                      edge->getPrintableEdgeVersions() + "::Call\"];\n";
      break;
    case Graph_Edge::EXTERNAL_CALL:
      outputString += "\" [arrowhead = odot, penwidth = 1.0, color = yellow, label=\"" +
                      edge->getPrintableEdgeVersions() + "::External_Call\"];\n";
      break;
    case Graph_Edge::MVICFG_ADD:
      outputString += "\" [arrowhead = normal, penwidth = 1.0, color = green, label=\"" +
                      edge->getPrintableEdgeVersions() + "::Add\"];\n";
      break;
    case Graph_Edge::MVICFG_DEL:
      outputString += "\" [arrowhead = normal, penwidth = 1.0, color = red, label=\"" +
                      edge->getPrintableEdgeVersions() + "::Del\"];\n";
      break;
    case Graph_Edge::ANY:
      std::cerr << "Should not have ANY as edgeType\n";
      outputString += "\" [arrowhead = normal, penwidth = 2.0, color = red, label=\"" +
                      edge->getPrintableEdgeVersions() + "::ANY\"];\n";
      break;
    } // End switch for edge
    gFile << outputString;
  } // End loop for edge
  /* Finalizing graph */
  gFile << "}\n";
  gFile.close();
} // End printGraph

void getLocationInfo(llvm::Instruction &I, unsigned int &DILocLine, std::string &DIFile) {
  if (llvm::DILocation *DILoc = I.getDebugLoc()) {
    DILocLine = DILoc->getLine();
    DIFile = DILoc->getFilename();
    return;
  } else {
    bool resolvedLine = false;
    /* Search backward */
    llvm::Instruction *I_iter = &I;
    do {
      auto prev = I_iter->getPrevNode();
      if (prev) {
        if (prev->getDebugLoc()) {
          DILocLine = prev->getDebugLoc()->getLine();
          DIFile = prev->getDebugLoc()->getFilename();
          resolvedLine = true;
          return;
        } // End check for getDebugLoc
        I_iter = I_iter->getPrevNode();
      } else {
        break;
      } // End check for prev
    } while (!resolvedLine);
    /* Search forward */
    if (!resolvedLine) {
      llvm::Instruction *I_iter = &I;
      do {
        auto next = I_iter->getNextNode();
        if (next) {
          if (next->getDebugLoc()) {
            DILocLine = next->getDebugLoc()->getLine();
            DIFile = next->getDebugLoc()->getFilename();
            resolvedLine = true;
            return;
          } // End check for getDebugLoc
          I_iter = I_iter->getNextNode();
        } else {
          break;
        } // End check for next
      } while (!resolvedLine);
    } // End check for resolvedLine
  }   // End check for getDebugLoc
} // End getLocationInfo

bool Graph::isVirtualNodeLineNumber(unsigned lineNumber) {
  if (lineNumber == graphEntryID || lineNumber == graphExitID) {
    return true;
  } // End check for Exit and Entry
  return false;
} // End isVirtualNode

int Graph::countNodes() {
  int count = 0;
  for (auto func : getGraphFunctions()) {
    for (auto line : func->getFunctionLines()) {
      count += line->getLineInstructions().size();
    }
  }
  return count;
}

int Graph::countEdges() {
  return getGraphEdges().size();
}
} // namespace hydrogen_framework
