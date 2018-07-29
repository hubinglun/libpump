cmake_minimum_required(VERSION 3.10)
project(libpump)

set(CMAKE_CXX_STANDARD 98)

include(CMake/Dependance.cmake)

aux_source_directory(./src DIRSRCS)
message("SRC >> ${DIRSRCS}")

include_directories(./src)
add_compile_options(-lpthread)

# 测试代码
add_executable(infratest_CbMailbox ${DIRSRCS} test/infratest_CbMailbox.cpp)
target_link_libraries(infratest_CbMailbox ${Boost_LIBRARIES})

add_executable(infratest_general_using ${DIRSRCS} test/infratest_general_using.cpp)
target_link_libraries(infratest_general_using ${Boost_LIBRARIES})