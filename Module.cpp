/**
 * @author Ashwin K J
 * @file
 * Implementing Module.hpp
 */
#include "Module.hpp"
namespace hydrogen_framework {
bool Module::setModule(int ver, std::string file) {
  modVersion = ver;
  llvm::StringRef modulePath(file);
  llvm::SMDiagnostic error;
  modPtr = llvm::parseIRFile(modulePath, error, modContext);
  /* Parsing Error handling */
  if (!modPtr) {
    std::string errorMessage;
    llvm::raw_string_ostream output(errorMessage);
    /* error.print("Error in parsing the file ", output); */
    std::cerr << "Error in parsing the " << file << "\n";
    return false;
  } // End check for modPtr
  /* Verifying Module */
  if (llvm::verifyModule(*modPtr, &llvm::errs()) != 0) {
    std::cerr << "Error in verifying the Module : " << file << "\n";
    return false;
  } // End check for verifyModule
  return true;
} // End setModule
} // namespace hydrogen_framework
