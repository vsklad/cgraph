cmake_minimum_required (VERSION 3.1)
project (CGraph)
set (CMAKE_CXX_STANDARD 11)

set(CGraph_SRC main.cpp bal/cnf/cnf.cpp bal/cnf/cnf.cpp bal/library/formula.cpp bal/variables/variablesio.cpp)
add_executable(cgraph ${CGraph_SRC})
target_include_directories(cgraph PRIVATE bal/base)
target_include_directories(cgraph PRIVATE bal/cnf)
target_include_directories(cgraph PRIVATE bal/io)
target_include_directories(cgraph PRIVATE bal/library)
target_include_directories(cgraph PRIVATE bal/utils)
target_include_directories(cgraph PRIVATE bal/variables)


