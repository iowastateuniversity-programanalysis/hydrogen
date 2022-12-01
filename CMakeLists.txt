##########################################
####     Standard CMAKE Config        ####
##########################################
cmake_minimum_required(VERSION 3.10)
project(Hydrogen)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(Boost REQUIRED COMPONENTS system)
find_package(LLVM REQUIRED)

##########################################
####     LLVM Configuration       ####
##########################################
set(CMAKE CXX FLAGS "-march=native -O3 -pipe -flto=thin")
set(LDFLAGS "-Wl,-O3")
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
        Graph_Edge_Iterator.cpp
        Graph_Edge_Iterator.hpp
        MVICFG.cpp
        MVICFG.hpp)

add_executable(hydrogen ${SOURCE_FILES})

target_link_libraries(hydrogen PUBLIC LLVM)
target_link_libraries(hydrogen PUBLIC boost_system)
target_link_libraries(hydrogen PUBLIC boost_filesystem)
target_link_libraries(hydrogen PUBLIC z3)
