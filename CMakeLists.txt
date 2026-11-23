cmake_minimum_required(VERSION 3.15)
project(BTP_Interval_Graphs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable strong compiler warnings and sufficient stack size for deep trees
if(MSVC)
    add_compile_options(/W4 /EHsc /utf-8)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:67108864")
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/maximal_cliques
)

# Core library sources (will grow across phases)
set(CORE_SOURCES
    src/IntervalGraph.cpp
    src/DistanceTree.cpp
    src/DistanceVerification.cpp
    src/StructuralAnalysis.cpp
    src/MaximalCliques.cpp
    src/IndependentSet.cpp
    src/DominatingSet.cpp
    src/IncrementalIndependentSet.cpp
    src/DecrementalDistanceTree.cpp
)

# Main executable
add_executable(interval_graphs
    src/main.cpp
    ${CORE_SOURCES}
)

# Test runner
add_executable(test_runner
    tests/test_main.cpp
    ${CORE_SOURCES}
)

# Benchmark runner
set(BENCHMARK_SOURCES
    benchmarks/maximal_cliques/graph_generator.cpp
    benchmarks/maximal_cliques/standard_interval_cliques.cpp
    benchmarks/maximal_cliques/bron_kerbosch_adapter.cpp
    benchmarks/maximal_cliques/theorem_cliques_adapter.cpp
    benchmarks/maximal_cliques/dynamic_clique_structure.cpp
    benchmarks/maximal_cliques/dynamic_workload_generator.cpp
    benchmarks/maximal_cliques/clique_verifier.cpp
    benchmarks/maximal_cliques/benchmark_runner.cpp
)

add_executable(clique_benchmark
    benchmarks/maximal_cliques/benchmark_main.cpp
    ${BENCHMARK_SOURCES}
    ${CORE_SOURCES}
)


