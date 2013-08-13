
find_package(Boost 1.54 REQUIRED system regex)


include_directories(../include ${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

add_executable(phttpd main.cpp)
target_link_libraries(phttpd ${Boost_LIBRARIES})
