cmake_minimum_required(VERSION 3.5)
project(libpump)

set(CMAKE_CXX_STANDARD 98)

include(CMake/Dependance.cmake)

aux_source_directory(./src DIRSRCS)
message("SRC >> ${DIRSRCS}")

include_directories(./src)
add_compile_options(-D__CODE_TEST -lpthread)

# 产生动态库 libpump.so
add_library(pump SHARED ${DIRSRCS})

# 测试代码
add_executable(infratest_CbMailbox test/infratest_CbMailbox.cpp)
target_link_libraries(infratest_CbMailbox ${Boost_LIBRARIES} pump)

add_executable(infratest_general_using test/infratest_general_using.cpp)
target_link_libraries(infratest_general_using ${Boost_LIBRARIES} pump)

add_executable(infratest_pump test/infratest_pump.cpp)
target_link_libraries(infratest_pump ${Boost_LIBRARIES} pump)