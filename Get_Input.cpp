/**
 * @author Ashwin K J
 * @file
 * Implementing Get_Input.hpp
 */
#include "Get_Input.hpp"
#include "Diff_Mapping.hpp"
#include "Module.hpp"
namespace hydrogen_framework {
bool Hydrogen::validateInputs(int c, char *files[]) {
  for (int index = 1; index < c; index++) {
    std::string file = files[index];
    struct stat buffer;
    int status = stat(file.c_str(), &buffer);
    if (status == -1) {
      if (file == hydrogenDemarcation) {
        continue;
      } // End check for hydrogenDemarcation
      std::cerr << file << " not accessible\n"
                << "Please recheck the input\n";
      return false;
    } // End check for status
  }   // End loop for inputs
  return true;
} // End validateInputs

bool Hydrogen::processInputs(int c, char *files[]) {
  int countModules = 0;
  /* Getting all the modules first */
  int index = 1;
  for (; index < c; ++index) {
    std::string file = files[index];
    if (file == hydrogenDemarcation) {
      break;
    } // End check for hydrogenDemarcation
    countModules++;
    Module *module = new Module();
    if (!module->setModule(countModules, file)) {
      return false;
    } // End check for module
    hydrogenModules.push_back(module);
  } // End module loop
  /* Getting the files associated with it */
  bool filesForAllVersions = false;
  for (int i = 1; i <= countModules; ++i) {
    std::list<std::string> versionFiles;
    for (++index; index < c; ++index) {
      std::string file = files[index];
      /* Checking for proper loop exit */
      if (file == hydrogenDemarcation) {
        if (i == (countModules - 1)) {
          filesForAllVersions = true;
        } // End check for countModules
        break;
      } // End check for hydrogenDemarcation
      versionFiles.push_back(file);
    } // End loop for versionFiles
    auto moduleIter = std::find_if(std::begin(hydrogenModules), std::end(hydrogenModules),
                                   [=](Module *mod) { return (mod->getVersion() == i); });
    if (moduleIter != hydrogenModules.end()) {
      (*moduleIter)->setFiles(versionFiles);
    } // End check for hydrogenDemarcation
  }   // End file loop
  if (!filesForAllVersions) {
    std::cerr << "Insufficient no of file versions provided\n"
              << "Please recheck your input\n";
    return false;
  } // End check for filesForAllVersions
  return true;
} // End processInputs
} // namespace hydrogen_framework
