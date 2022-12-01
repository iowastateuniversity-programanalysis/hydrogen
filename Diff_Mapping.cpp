/**
 * @author Ashwin K J
 * @file
 * Implementing Diff_Mapping.hpp
 */
#include "Diff_Mapping.hpp"
#include <limits>
namespace hydrogen_framework {
void Diff_Mapping::putMapping(const std::vector<sesElem> &seqVector) {
  for (const auto &iter : seqVector) {
    elemInfo info;
    switch (iter.second.type) {
    case SES_ADD:
      info.beforeIdx = iter.second.beforeIdx;
      info.afterIdx = iter.second.afterIdx;
      info.type = SES_ADD;
      addedLines.push_back(iter.second.afterIdx);
      break;
    case SES_DELETE:
      info.beforeIdx = iter.second.beforeIdx;
      info.afterIdx = iter.second.afterIdx;
      info.type = SES_DELETE;
      deletedLines.push_back(iter.second.beforeIdx);
      break;
    case SES_COMMON:
      info.beforeIdx = iter.second.beforeIdx;
      info.afterIdx = iter.second.afterIdx;
      info.type = SES_COMMON;
      matchedLines.insert(std::pair<long long, long long>(iter.second.beforeIdx, iter.second.afterIdx));
      break;
    }
    lineMap.push_back(info);
  } // End loop for seqVector
} // End putMapping

void Diff_Mapping::printMapping() {
  std::cout << "File name : " << fileName << "\n";
  for (auto iter : lineMap) {
    std::string type;
    switch (iter.type) {
    case SES_ADD:
      type = "+";
      break;
    case SES_DELETE:
      type = "-";
      break;
    case SES_COMMON:
      type = " ";
    } // End switch for iter.type
    std::cout << iter.beforeIdx << ":" << iter.afterIdx << "\t" << type << "\n";
  } // End loop for lineMap
} // End printMapping

void Diff_Mapping::printAddedLines() {
  for (auto iter : addedLines) {
    std::cout << SES_MARK_ADD << " " << iter << "\n";
  } // End loop for addedLines
} // End printAddedLines

void Diff_Mapping::printDeletedLines() {
  for (auto iter : deletedLines) {
    std::cout << SES_MARK_DELETE << " " << iter << "\n";
  } // End loop for deletedLines
} // End printDeletedLines

void Diff_Mapping::printMatchedLines() {
  for (auto iter : matchedLines) {
    std::cout << iter.first << ":" << iter.second << "\n";
  } // End loop for matchedLines
} // End printMatchedLines

void Diff_Mapping::printFileInfo() {
  std::cout << "File : " << getFileName() << "\n";
  printAddedLines();
  printDeletedLines();
  printMatchedLines();
  std::cout << "----\n";
} // End printFileInfo

long long Diff_Mapping::getAfterLineNumber(long long currLine) {
  for (auto iter : lineMap) {
    if (iter.beforeIdx == currLine) {
      return iter.afterIdx;
    } // End check for currLine
  }   // End loop for lineMap
  return std::numeric_limits<unsigned>::max();
} // End getNewLineNumber

long long Diff_Mapping::getBeforeLineNumber(long long currLine) {
  for (auto iter : lineMap) {
    if (iter.afterIdx == currLine) {
      return iter.beforeIdx;
    } // End check for currLine
  }   // End loop for lineMap
  return std::numeric_limits<unsigned>::max();
} // End getOldLineNumber
} // namespace hydrogen_framework
