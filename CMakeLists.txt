cmake_minimum_required(VERSION 3.5)
project(libpump)

set(CMAKE_CXX_STANDARD 98)

include(CMake/Dependance.cmake)

aux_source_directory(./src DIRSRCS)
aux_source_directory(./src/Buffer DIRSRCS)
aux_source_directory(./src/Watcher DIRSRCS)
aux_source_directory(./src/Watcher/FdBaseWatcher DIRSRCS)
aux_source_directory(./src/Event DIRSRCS)
set(SRC ./src/ src/Buffer/ src/Watcher/FdBaseWatcher/ ${DIRSRCS})
message(">>> SRC: ${SRC}")

include_directories(./src)
# test code level: DEBUG INFO TRACE
add_compile_options(-D_TEST_LEVEL_INFO -Wno-write-strings -lpthread)

set(all_libs ${Boost_LIBRARIES} ${glog_libs} ${protobuf_libs})
message(">>> LIBS: ${all_libs}")

# 产生动态库 libpump.so
add_library(pump SHARED ${SRC} ${all_libs})

# 测试代码
add_executable(infratest_CbMailbox test/infratest_CbMailbox.cpp)
target_link_libraries(infratest_CbMailbox ${all_libs} pump)

add_executable(infratest_general_using test/infratest_general_using.cpp)
target_link_libraries(infratest_general_using ${all_libs} pump)

add_executable(infratest_pump test/infratest_pump.cpp)
target_link_libraries(infratest_pump ${all_libs} pump)

add_executable(infratest_Buffer test/infratest_Buffer.cpp)
target_link_libraries(infratest_Buffer ${all_libs} pump)

add_executable(infratest_Watcher test/infratest_Watcher.cpp)
target_link_libraries(infratest_Watcher ${all_libs} pump)

add_executable(boost_interprocess_test test/boost_test/boost_interprocess_test.cpp)
target_link_libraries(boost_interprocess_test ${all_libs} pump)