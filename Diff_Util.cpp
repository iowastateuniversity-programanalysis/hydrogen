/**
 * @author Ashwin K J
 * @file
 * Implementing Diff_Util.hpp
 */
#include "Diff_Util.hpp"
namespace hydrogen_framework {
void Diff_Ses::addSequence(elem e, long long beforeIdx, long long afterIdx, const int type) {
  elemInfo info;
  info.beforeIdx = beforeIdx;
  info.afterIdx = afterIdx;
  info.type = type;
  sesElem pe(e, info);
  if (!deletesFirst) {
    sequenceDS.push_back(pe);
  } // End check for deletesFirst
  switch (type) {
  case SES_DELETE:
    onlyCopy = false;
    onlyAdd = false;
    if (deletesFirst) {
      sequenceDS.insert(sequenceDS.begin() + nextDeleteIdx, pe);
      nextDeleteIdx++;
    } // End check for deletesFirst
    break;
  case SES_COMMON:
    onlyAdd = false;
    onlyDelete = false;
    if (deletesFirst) {
      sequenceDS.push_back(pe);
      nextDeleteIdx = sequenceDS.size();
    } // End check for deletesFirst
    break;
  case SES_ADD:
    onlyDelete = false;
    onlyCopy = false;
    if (deletesFirst) {
      sequenceDS.push_back(pe);
    } // End check for deletesFirst
    break;
  } // End switch for SES_DELETE
} // End addSequence

void Diff_Util::compose() {
  pathCordinates.reserve(MAX_CORDINATES_SIZE);
  long long p = -1;
  fp = new long long[M + N + 3];
  std::fill(&fp[0], &fp[M + N + 3], -1);
  path = editPath(M + N + 3);
  fill(path.begin(), path.end(), -1);
ONP:
  do {
    ++p;
    for (long long k = -p; k <= static_cast<long long>(delta) - 1; ++k) {
      fp[k + offset] = snake(k, fp[k - 1 + offset] + 1, fp[k + 1 + offset]);
    } // End loop for delta + 1
    for (long long k = static_cast<long long>(delta) + p; k >= static_cast<long long>(delta) + 1; --k) {
      fp[k + offset] = snake(k, fp[k - 1 + offset] + 1, fp[k + 1 + offset]);
    } // End loop for delta - 1
    fp[delta + offset] = snake(static_cast<long long>(delta), fp[delta - 1 + offset] + 1, fp[delta + 1 + offset]);
  } while (fp[delta + offset] != static_cast<long long>(N) && pathCordinates.size() < MAX_CORDINATES_SIZE);

  long long r = path[delta + offset];
  P cordinate;
  editPathCordinates epc(0);

  while (r != -1) {
    cordinate.x = pathCordinates[(size_t)r].x;
    cordinate.y = pathCordinates[(size_t)r].y;
    epc.push_back(cordinate);
    r = pathCordinates[(size_t)r].k;
  } // End loop for r!= -1

  // Record Longest Common Subsequence & Shortest Edit Script
  if (!recordSequence(epc)) {
    pathCordinates.resize(0);
    epc.resize(0);
    p = -1;
    goto ONP;
  } // End check for recordSequence
  delete[] this->fp;
} // End compose

void Diff_Util::init() {
  M = distance(A.begin(), A.end());
  N = distance(B.begin(), B.end());
  if (M < N) {
    swapped = false;
  } else {
    std::swap(A, B);
    std::swap(M, N);
    swapped = true;
  } // End check for M < N
  delta = N - M;
  offset = M + 1;
  fp = nullptr;
} // End init

long long Diff_Util::snake(const long long &k, const long long &above, const long long &below) {
  long long r = above > below ? path[(size_t)k - 1 + offset] : path[(size_t)k + 1 + offset];
  long long y = std::max(above, below);
  long long x = y - k;
  while ((size_t)x < M && (size_t)y < N &&
         (swapped ? cmp.impl(B[(size_t)y], A[(size_t)x]) : cmp.impl(A[(size_t)x], B[(size_t)y]))) {
    ++x;
    ++y;
  } // End loop for swapped

  path[(size_t)k + offset] = static_cast<long long>(pathCordinates.size());
  P p;
  p.x = x;
  p.y = y;
  p.k = r;
  pathCordinates.push_back(p);
  return y;
} // End snake

bool Diff_Util::recordSequence(const editPathCordinates &v) {
  sequence_const_iter x(A.begin());
  sequence_const_iter y(B.begin());
  long long x_idx, y_idx;   // line number for Unified Format
  long long px_idx, py_idx; // cordinates
  bool complete = false;
  x_idx = y_idx = 1;
  px_idx = py_idx = 0;
  for (size_t i = v.size() - 1; !complete; --i) {
    while (px_idx < v[i].x || py_idx < v[i].y) {
      if (v[i].y - v[i].x > py_idx - px_idx) {
        if (!wasSwapped()) {
          ses.addSequence(*y, 0, y_idx, SES_ADD);
        } else {
          ses.addSequence(*y, y_idx, 0, SES_DELETE);
        } // End check for wasSwapped
        ++y;
        ++y_idx;
        ++py_idx;
      } else if (v[i].y - v[i].x < py_idx - px_idx) {
        if (!wasSwapped()) {
          ses.addSequence(*x, x_idx, 0, SES_DELETE);
        } else {
          ses.addSequence(*x, 0, x_idx, SES_ADD);
        } // End check for wasSwapped
        ++x;
        ++x_idx;
        ++px_idx;
      } else {
        if (!wasSwapped()) {
          ses.addSequence(*x, x_idx, y_idx, SES_COMMON);
        } else {
          ses.addSequence(*y, y_idx, x_idx, SES_COMMON);
        } // End check for wasSwapped
        ++x;
        ++y;
        ++x_idx;
        ++y_idx;
        ++px_idx;
        ++py_idx;
      } // End check for v.y -v.x
    }   // End loop for px_idx & py_idx
    if (i == 0)
      complete = true;
  } // End loop for complete

  if (x_idx > static_cast<long long>(M) && y_idx > static_cast<long long>(N)) {
    // all recording succeeded
  } else {
    sequence A_(A.begin() + (size_t)x_idx - 1, A.end());
    sequence B_(B.begin() + (size_t)y_idx - 1, B.end());
    A = A_;
    B = B_;
    M = distance(A.begin(), A.end());
    N = distance(B.begin(), B.end());
    delta = N - M;
    offset = M + 1;
    delete[] fp;
    fp = new long long[M + N + 3];
    std::fill(&fp[0], &fp[M + N + 3], -1);
    std::fill(path.begin(), path.end(), -1);
    return false;
  } // End check for x_idx
  return true;
} // End snake
} // namespace hydrogen_framework
