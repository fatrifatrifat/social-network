cmake_minimum_required(VERSION 3.23)
project(SocialNetwork)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TESTING "Build tests" ON)

add_executable(social-network)

target_sources(social-network PRIVATE src/main.cpp)

target_compile_features(social-network PRIVATE cxx_std_23)

find_package(libpqxx REQUIRED)

add_subdirectory(src/graph)
add_subdirectory(src/loader)

include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif()

target_link_libraries(social-network
  PRIVATE
    libpqxx::pqxx
    graph
    loader
)
