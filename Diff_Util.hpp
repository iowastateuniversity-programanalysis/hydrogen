/**
 * @author Ashwin K J
 * @file
 * Diff Util Class: Generating the diff between the files
 */
#ifndef DIFF_UTIL_H
#define DIFF_UTIL_H

#include <list>
#include <string>
#include <vector>
namespace hydrogen_framework {
/**
 * DiffCompare Functor class
 */
class Diff_Compare {
public:
  /**
   * Constructor
   */
  Diff_Compare() {}
  /**
   * Virtual Destructor
   */
  virtual ~Diff_Compare() {}
  /**
   * Comparison function
   * Return TRUE if equal
   */
  virtual inline bool impl(const std::string &e1, const std::string &e2) const { return e1 == e2; }
};

/**
 * Class to hold common/shared type definitions and variables
 */
class Diff_Vars {
public:
  /**
   * Constructor
   */
  Diff_Vars() {}

  /**
   * Virtual Destructor
   */
  virtual ~Diff_Vars() {}

  /**
   * Type of edit for SES
   */
  enum SES_TYPE { SES_DELETE = -1, SES_COMMON = 0, SES_ADD = 1 };

  std::string SES_MARK_DELETE = "-"; /**< Setting SES_MARK_DELETE */
  std::string SES_MARK_COMMON = " "; /**< Setting SES_MARK_COMMON */
  std::string SES_MARK_ADD = "+";    /**< Setting SES_MARK_ADD */

  /**
   * Structure for storing element information
   */
  typedef struct eleminfo {
    long long beforeIdx; /**< Index of prev sequence */
    long long afterIdx;  /**< Index of after sequence */
    int type;            /**< Type of edit(Add, Delete, Common) */
    /**
     * Overriding equal operation
     */
    bool operator==(const eleminfo &other) const {
      return (this->beforeIdx == other.beforeIdx && this->afterIdx == other.afterIdx && this->type == other.type);
    }
  } elemInfo;

  /**
   * Coordinate for registering route
   */
  typedef struct Point {
    long long x; /**< X coordinate */
    long long y; /**< Y coordinate */
    long long k; /**< vertex */
  } P;

  const unsigned long long MAX_CORDINATES_SIZE = 2000000;        /**< Limit of coordinate size */
  typedef std::vector<long long> editPath;                       /**< Type definition for editPath */
  typedef std::vector<P> editPathCordinates;                     /**< Type definition for editPathCordinates */
  typedef std::string elem;                                      /**< Type definition for elem */
  typedef std::vector<elem> sequence;                            /**< Type definition for sequence */
  typedef std::pair<elem, elemInfo> sesElem;                     /**< Type definition for sesElem */
  typedef std::vector<sesElem> sesElemVec;                       /**< Type definition for sesElemVec */
  typedef std::list<elem> elemList;                              /**< Type definition for elemList */
  typedef std::vector<elem> elemVec;                             /**< Type definition for elemVec */
  typedef typename sesElemVec::iterator sesElemVec_iter;         /**< Type definition for sesElemVec_iter*/
  typedef typename elemList::iterator elemList_iter;             /**< Type definition for elemList_iter */
  typedef typename sequence::iterator sequence_iter;             /**< Type definition for sequence_iter */
  typedef typename sequence::const_iterator sequence_const_iter; /**< Type definition for sequence_const_iter */
  typedef typename elemVec::iterator elemVec_iter;               /**< Type definition for elemVec_iter */
};

/**
 * Class to store sequence of elements
 */
class Diff_Sequence : public Diff_Vars {
public:
  /**
   * Constructor
   */
  Diff_Sequence() {}
  /**
   * Virtual Destructor
   */
  virtual ~Diff_Sequence() {}

  /**
   * Return sequence
   */
  elemVec getSequence() const { return sequence; }

  /**
   * Add to sequence
   */

  void addSequence(elem e) { sequence.push_back(e); }

protected:
  elemVec sequence; /**< Store sequence of elems as vector */
};

/**
 * Class to compute Shortest Edit Distance
 */
class Diff_Ses : public Diff_Sequence {
public:
  /**
   * Constructor with no argument
   */
  Diff_Ses() : onlyAdd(true), onlyDelete(true), onlyCopy(true), deletesFirst(false) { nextDeleteIdx = 0; }

  /**
   * Constructor with one argument
   */
  Diff_Ses(bool moveDel) : onlyAdd(true), onlyDelete(true), onlyCopy(true), deletesFirst(moveDel) { nextDeleteIdx = 0; }

  /**
   * Destructor
   */
  ~Diff_Ses() {}

  /**
   * Return onlyAdd
   */
  bool isOnlyAdd() const { return onlyAdd; }

  /**
   * Return onlyDelete
   */
  bool isOnlyDelete() const { return onlyDelete; }

  /**
   * Return onlyCopy
   */
  bool isOnlyCopy() const { return onlyCopy; }

  /**
   * Return TRUE if any of onlyAdd, onlyDelete or onlyCopy is TRUE
   */
  bool isOnlyOneOperation() const { return isOnlyAdd() || isOnlyDelete() || isOnlyCopy(); }

  /**
   * Return TRUE if onlyCopy is FALSE
   */
  bool isChange() const { return !onlyCopy; }

  /**
   * Add sequence
   */
  void addSequence(elem e, long long beforeIdx, long long afterIdx, const int type);

  /**
   * Return sequence
   */
  sesElemVec getSequence() const { return sequenceDS; }

private:
  sesElemVec sequenceDS; /**< SES sequence */
  bool onlyAdd;          /**< Flag to indicate add operation */
  bool onlyDelete;       /**< Flag to indicate deletion operation */
  bool onlyCopy;         /**< Flag to indicate change operation */
  bool deletesFirst;     /**< Flag to indicate if deletion is required first */
  size_t nextDeleteIdx;  /**< Point towards next deletion ID */
};

/**
 * Class to compute Diff between two files
 */
class Diff_Util : Diff_Vars {
public:
  /**
   * Constructor with no arguments
   */
  Diff_Util() {}

  /**
   * Constructor with two arguments
   */
  Diff_Util(const sequence &a, const sequence &b) : A(a), B(b), ses(false) { init(); }

  /**
   * Destructor
   */
  ~Diff_Util() {}

  /**
   * Return ses
   */
  Diff_Ses getSes() const { return ses; }

  /**
   * Compose Longest Common Subsequence and Shortest Edit Script.
   * The algorithm implemented here is based on "An O(NP) Sequence Comparison Algorithm"
   * described by Sun Wu, Udi Manber and Gene Myers
   */
  void compose();

private:
  sequence A;                        /**< First sequence */
  sequence B;                        /**< Second sequence */
  size_t M;                          /**< M value */
  size_t N;                          /**< N value */
  size_t delta;                      /**< Delta */
  size_t offset;                     /**< offset */
  long long *fp;                     /**< Pointer to elem */
  Diff_Ses ses;                      /**< Shortest edit scrit */
  editPath path;                     /**< Edit path */
  editPathCordinates pathCordinates; /**< Edit path coordinates */
  bool swapped;                      /**< Flag to check if sequence are swapped */
  Diff_Compare cmp;                  /**< Comparison Functor */
  /**
   * Initialize
   */
  void init();

  /**
   * Search shortest path and record the path
   */
  long long snake(const long long &k, const long long &above, const long long &below);

  /**
   * Record SES
   */
  bool recordSequence(const editPathCordinates &v);

  /**
   * Check if the sequences have been swapped
   */
  bool inline wasSwapped() const { return swapped; }
}; // End Diff Class
} // namespace hydrogen_framework
#endif
