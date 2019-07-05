/**
 * @author Ashwin K J
 * @file
 * Module class: Managing LLVM Module
 */
#ifndef MODULE_H
#define MODULE_H

#include <iostream>
#include <list>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/raw_ostream.h>

namespace hydrogen_framework {
/**
 * LLVM Module class: Hold the LLVM modules and associated files
 */
class Module {
public:
  /**
   * Constructor for module class
   * Set version to zero
   */
  Module() { modVersion = 0; }

  /**
   * Destructor
   */
  ~Module() { modFiles.clear(); }

  /**
   * Set module by initializing all the values except modFiles
   * Returns FALSE if LLVM IR parsing error is found
   */
  bool setModule(int ver, std::string file);

  /**
   * Set modFiles by swapping out with the incoming list of files
   */
  void setFiles(std::list<std::string> file) { modFiles.swap(file); }

  /**
   * Return modVersion
   */
  int getVersion() { return modVersion; }

  /**
   * Return modPtr
   */
  std::unique_ptr<llvm::Module> &getPtr() { return modPtr; }

  /**
   * Return modFiles;
   */
  std::list<std::string> getFiles() { return modFiles; }

private:
  int modVersion;                       /**< Module Version */
  llvm::LLVMContext modContext;         /**< LLVM Module Context */
  std::unique_ptr<llvm::Module> modPtr; /**< LLVM Module Pointer */
  std::list<std::string> modFiles;      /**< Source files for the LLVM Module */
};                                      // End module class
} // namespace hydrogen_framework
#endif
