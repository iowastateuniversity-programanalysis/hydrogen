/**
 * @author Ashwin K J
 * @file
 * Get_Input Class: Getting the input from the user
 */
#ifndef GET_INPUT_H
#define GET_INPUT_H

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <sys/stat.h>
namespace hydrogen_framework {
/* Forward declaration */
class Module;

/**
 * Hydrogen Class: Hydrogen Framework data structures and functions
 */
class Hydrogen {
public:
  /**
   * Constructor for hydrogen class
   * Sets the demarcation variable
   */
  Hydrogen() { hydrogenDemarcation = "::"; }

  /**
   * Destructor
   */
  ~Hydrogen() { hydrogenModules.clear(); }

  /**
   * Validate provided inputs.
   * Returns FALSE if any of the provided input is not present.
   */
  bool validateInputs(int c, char *files[]);

  /**
   * Process provided inputs.
   * Returns FALSE if any of the Module cannot be parsed properly.
   */
  bool processInputs(int c, char *files[]);

  /**
   * Return hydrogenModules
   */
  std::list<Module *> getModules() { return hydrogenModules; }

private:
  std::string hydrogenDemarcation;     /**< Setting demarcation string for inputs */
  std::list<Module *> hydrogenModules; /**< Container for storing LLVM Modules */
};                                     // End hydrogen class
} // namespace hydrogen_framework
#endif
