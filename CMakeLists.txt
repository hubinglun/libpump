cmake_minimum_required(VERSION 3.5)
project(libpump)

set(CMAKE_CXX_STANDARD 98)

include(CMake/Dependance.cmake)

aux_source_directory(./src DIRSRCS)
set(SRC ./src/pumpdef.h ${DIRSRCS})
message("SRC >> ${SRC}")

include_directories(./src)
add_compile_options(-D_TEST_LEVEL_0 -lpthread)

# 产生动态库 libpump.so
add_library(pump SHARED ${SRC})

set(all_libs ${Boost_LIBRARIES} ${glog_libs} ${protobuf_libs})
message(>>> ${all_libs})

# 测试代码
add_executable(infratest_CbMailbox test/infratest_CbMailbox.cpp)
target_link_libraries(infratest_CbMailbox ${all_libs} pump)

add_executable(infratest_general_using test/infratest_general_using.cpp)
target_link_libraries(infratest_general_using ${all_libs} pump)

add_executable(infratest_pump test/infratest_pump.cpp)
target_link_libraries(infratest_pump ${all_libs} pump)