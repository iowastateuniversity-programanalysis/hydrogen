# Error conditions

- CMake error
  - General
  - File transformation
  - Invocation
    - Missing deps
    - Bad flags (should test several repos on this to make sure the flags work on different repos)
    - Minimum CMake version
    - Warning `COMPILE_OPTIONS` or something is deprecated when run on `gtest`
    - Completely brokedy-doke on `gtest` because it links `.cc`'s with `clang++`. It's an outlier, though.
- Git error
  - General
  - Repo doesn't exist
  - Commit doesn't exist
- subprocess needs to be killed? Transactions?

# Improvements

- Auto detect language
- Give repo and list of ID's in the command-line arguments
- Make it work for nested `CMakeLists.txt`.
  1. For every call to `add_subdir`, add that dir's `CMakeLists.txt` to the list.
  1. Transform all `CMakeLists.txt`.
  1. Build all `CMakeLists.txt`.
  1. Give a selection of one exe to Hydrogen? How??
- Make it work for building libraries
- -o: Copy .bc's to an output directory?
- Depend on `compor/llvm-ir-cmake-utils` in a Git submodule?
- Logging
    - Hide/scrub messages
    - Helpful progress info: repo pulled, version copied, version built, locations of .bc's, echo Hydrogen command
    - -v: Verbose flag
