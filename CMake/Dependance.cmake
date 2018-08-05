#</find Boost>
find_package(Boost REQUIRED COMPONENTS
        thread
        system)
include_directories(${Boost_INCLUDE_DIRS})
#</find Boost>

# <find glog>
set(glog_inc /home/yz/Libs/glog/cmake-build-debug /home/yz/Libs/glog/src)
set(glog_libs_dir /home/yz/Libs/glog/cmake-build-debug)
find_library(glog_libs glogd ${glog_libs_dir})
include_directories(${glog_inc})
message(">>>${glog_libs}")
#</find glog>

# <find protobuf>
set(protobuf_inc /usr/local/include/google/)
set(protobuf_libs_dir /usr/local/lib)
include_directories(${protobuf_inc})
find_library(protobuf_libs protobuf ${protobuf_libs_dir})
message(">>>${protobuf_libs}")
# </find protobuf>