cmake_minimum_required(VERSION 3.10)
project(libpump)

set(CMAKE_CXX_STANDARD 11)

include(CMake/Dependance.cmake)

aux_source_directory(./src DIRSRCS)
message("SRC >> ${DIRSRCS}")

include_directories(./src)
add_compile_options(-lpthread)

# 测试代码
add_executable(infratest_CbFuncVec ${DIRSRCS} ./test/infratest_CbFuncVec.cpp)
target_link_libraries(infratest_CbFuncVec ${Boost_LIBRARIES})

add_executable(infratest_genera_using ${DIRSRCS} ./test/infratest_genera_using.cpp)
target_link_libraries(infratest_genera_using ${Boost_LIBRARIES})