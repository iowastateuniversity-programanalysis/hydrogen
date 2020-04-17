# Manual
llvm-link -S $(ls -d CMakeFiles/include_gardener.dir/src/*) -o include_gardener.bc

# `compor`'s Utilities

`compor` wrote some bomb-ass CMake utilities for compiling to LLVM IR, and doggone it, we're gonna use them.

## For this project

In the target definition:

```
project(include_gardener CXX)
```

Then, later:

```
# LLVM-IR Generation
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../hydrogit/llvm-ir-cmake-utils/cmake")
include(LLVMIRUtil)
set_target_properties(include_gardener PROPERTIES LINKER_LANGUAGE CXX)
llvmir_attach_bc_target(include_gardener_bc include_gardener)
add_compile_options(include_gardener_bc -c -O0 -Xclang -disable-O0-optnone -g -emit-llvm -S)
add_dependencies(include_gardener_bc include_gardener)
llvmir_attach_link_target(include_gardener_llvmlink include_gardener_bc -S)
```

## General instructions

For every subdirectory, do the following:

1. If a project declaration does not have `CXX` (for C++) or `C` (for C), define those.

`project($(PROJECT_NAME))` -> `project($(PROJECT_NAME) C CXX)`

1. Attach BC generation towards the end of `CMakeLists.txt`.

  - Deps
    - `PROJECT_NAME`: The name defined in the project.
      - Defined in `CMakeLists.txt`
    - `UTIL_LOC` The location of these utilities.
      - Should be in a static location.
      - Could be absolute?
  - Output
    - Get linked bc from `build/llvm-ir/$(PROJECT_NAME)_llvmlink$(PROJECT_NAME)_llvmlink.bc`    

```
# LLVM-IR Generation
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../hydrogit/llvm-ir-cmake-utils/cmake")
include(LLVMIRUtil)
if("${LINKER_LANGUAGE}" STREQUAL "")
  set_target_properties(include_gardener PROPERTIES LINKER_LANGUAGE CXX)
endif()
llvmir_attach_bc_target($(PROJECT_NAME)_bc $(PROJECT_NAME))
set(-c -O0 -Xclang -disable-O0-optnone -g -emit-llvm -S)
set(llvmirBytecodeCompileOptions "-c -O0 -Xclang -disable-O0-optnone -g -emit-llvm -S")
add_compile_options($(PROJECT_NAME)_bc "${llvmirBytecodeCompileOptions}")
add_dependencies($(PROJECT_NAME)_bc $(PROJECT_NAME))
llvmir_attach_link_target($(PROJECT_NAME)_llvmlink $(PROJECT_NAME)_bc -S)
```
