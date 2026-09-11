add_library(graph)

target_sources(graph 
  PRIVATE
    graph.cpp
  PUBLIC 
    FILE_SET HEADERS
    FILES
      graph.hpp
)
