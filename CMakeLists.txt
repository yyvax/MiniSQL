cmake_minimum_required(VERSION 3.7)
project(MiniSQL)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp IndexManager.h Data.cpp Data.h datatype.h DBException.h DataException.h IndexManager.cpp mystr.h BufferManager.h)
add_executable(MiniSQL ${SOURCE_FILES})