cmake_minimum_required(VERSION 3.5)
project(libpump)

set(CMAKE_CXX_STANDARD 11)

include(CMake/Dependance.cmake)

aux_source_directory(./src DIRSRCS)
aux_source_directory(./src/Buffer DIRSRCS)
aux_source_directory(./src/PWatcher DIRSRCS)
aux_source_directory(./src/PWatcher/PWFdBase DIRSRCS)
aux_source_directory(./src/Event DIRSRCS)
aux_source_directory(src/MemMgr DIRSRCS)
set(SRC ./src/ src/Buffer/ src/PWatcher/PWFdBase/ ${DIRSRCS})
message(">>> SRC: ${SRC}")

include_directories(./src)
# test code level: DEBUG INFO TRACE
add_compile_options(-D_TEST_LEVEL_INFO -DBOOST_THREAD_USE_LIB=1 -Wno-write-strings -lpthread -fPIC)
set(all_libs ${Boost_LIBRARIES} ${glog_libs} ${protobuf_libs})
message(">>> LIBS: ${all_libs}")

# 产生动态库 libpump.so
link_libraries(${all_libs})
add_library(pump SHARED ${SRC})


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

add_executable(infrastest_Thread test/infrastest_Thread.cpp)
target_link_libraries(infrastest_Thread ${all_libs} pump)

add_executable(infrastest_SmartPtr test/infrastest_SmartPtr.cpp)
target_link_libraries(infrastest_SmartPtr ${all_libs} pump)