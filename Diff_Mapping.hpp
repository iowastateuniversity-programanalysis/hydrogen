/**
 * @author Ashwin K J
 * @file
 * Diff_Mapping Class: Generating line mapping information per diff file
 */
#ifndef DIFF_MAPPING_H
#define DIFF_MAPPING_H

#include "Diff_Util.hpp"
#include <iostream>
#include <list>
#include <regex>
#include <utility>
#include <vector>
namespace hydrogen_framework {
/**
 * Diff_Mapping Class: Container for storing diff mapping details
 */
class Diff_Mapping : public Diff_Vars {
public:
  /**
   * Constructor
   */
  explicit Diff_Mapping(std::string name) : fileName(std::move(name)) {}

  /**
   * Destructor
   */
  ~Diff_Mapping() override { lineMap.clear(); }

  /**
   * Populate line mapping
   */
  void putMapping(const std::vector<sesElem>& seqVector);

  /**
   * Return lineMap
   */
  std::list<elemInfo> getMapping() { return lineMap; }

  /**
   * Return fileName
   */
  std::string getFileName() { return fileName; }

  /**
   * Return addedLines
   */
  std::list<long long> getAddedLines() { return addedLines; }

  /**
   * Return deletedLines
   */
  std::list<long long> getDeletedLines() { return deletedLines; }

  /**
   * Return matchedLines
   */
  std::map<long long, long long> getMatchedLines() { return matchedLines; }

  /**
   * Print lineMap
   */
  void printMapping();

  /**
   * Print addedLines
   */
  void printAddedLines();

  /**
   * Print deletedLines
   */
  void printDeletedLines();

  /**
   * Print matchedLines
   */
  void printMatchedLines();

  /**
   * Print File Related Info
   */
  void printFileInfo();

  /**
   * Get the afterIdx line number given the beforeIdx line number
   * Return unsigned MAX if line not found
   */
  long long getAfterLineNumber(long long currLine);

  /**
   * Get the beforeIdx line number given the afterIdx line number
   * Return unsigned MAX if line not found
   */
  long long getBeforeLineNumber(long long currLine);

private:
  std::string fileName;              /**< File Name */
  std::list<elemInfo> lineMap;       /**< Container for line mapping */
  std::list<long long> addedLines;   /**< Container for added line numbers */
  std::list<long long> deletedLines; /**< Container for deleted line numbers */
  std::map<long long, long long>
      matchedLines; /**< Container for matched line numbers mapping from before to after lines*/
};                  // End Diff_Mapping Class
} // namespace hydrogen_framework
#endif
