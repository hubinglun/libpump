# boost
find_package(Boost REQUIRED COMPONENTS
        thread
        system)
include_directories(${Boost_INCLUDE_DIRS})