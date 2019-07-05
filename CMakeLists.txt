##########################################
####     Standard CMAKE Config        ####
##########################################
cmake_minimum_required(VERSION 3.10)
project(Hydrogen)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
##########################################
####     LLVM Configuration       ####
##########################################
set(CMAKE CXX FLAGS "-march=native -O3 -pipe -flto=thin")
set(LDFLAGS  "-lLLVM -lz3 -lboost_system -lboost_filesystem -Wl,-O3")
set(CMAKE_EXE_LINKER_FLAGS "${LDFLAGS}")
##########################################
####        Source file Config        ####
##########################################
set(SOURCE_FILES
    Diff_Mapping.cpp
    Diff_Mapping.hpp
    Diff_Util.cpp
    Diff_Util.hpp
    Get_Input.cpp
    Get_Input.hpp
    Graph.cpp
    Graph.hpp
    Graph_Edge.cpp
    Graph_Edge.hpp
    Graph_Function.cpp
    Graph_Function.hpp
    Graph_Instruction.hpp
    Graph_Line.cpp
    Graph_Line.hpp
    Hydrogen.cpp
    Module.cpp
    Module.hpp
    MVICFG.cpp
    MVICFG.hpp)
add_executable(Hydrogen.out ${SOURCE_FILES})
