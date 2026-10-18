add_library(loader)

target_sources(loader 
  PRIVATE
    loader.cpp
  PUBLIC 
    FILE_SET HEADERS
    FILES
      loader.hpp
)

find_package(simdjson REQUIRED)

target_link_libraries(loader
  PUBLIC
    graph
  PRIVATE
    simdjson::simdjson
)
