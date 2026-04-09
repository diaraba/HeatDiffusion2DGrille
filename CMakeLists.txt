cmake_minimum_required(VERSION 3.15)
project(HeatDiffusion LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find required packages
find_package(MPI REQUIRED)
find_package(OpenMP REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${MPI_CXX_INCLUDE_DIRS})

# Compiler options
set(COMMON_COMPILE_OPTIONS -Wall -Wextra -O3 -march=native -ffast-math)
add_compile_options(${COMMON_COMPILE_OPTIONS})

# Source files
set(COMMON_SOURCES
    src/grid.cpp
    src/boundary.cpp
    src/timer.cpp
    src/config.cpp
)

set(COMMON_HEADERS
    include/grid.h
    include/boundary.h
    include/timer.h
    include/config.h
    include/solver.h
)

# ============================================
# Sequential version
# ============================================
add_executable(heat_seq
    src/main.cpp
    src/solver.cpp
    src/solver_sequential.cpp
    ${COMMON_SOURCES}
)

target_compile_definitions(heat_seq PRIVATE SEQUENTIAL)
target_link_libraries(heat_seq)

# ============================================
# OpenMP version
# ============================================
add_executable(heat_omp
    src/main.cpp
    src/solver.cpp
    src/solver_openmp.cpp
    ${COMMON_SOURCES}
)

target_compile_definitions(heat_omp PRIVATE USE_OPENMP)
target_link_libraries(heat_omp OpenMP::OpenMP_CXX)

# ============================================
# MPI version
# ============================================
add_executable(heat_mpi
    src/main.cpp
    src/solver.cpp
    src/solver_mpi.cpp
    ${COMMON_SOURCES}
)

target_compile_definitions(heat_mpi PRIVATE USE_MPI)
target_link_libraries(heat_mpi ${MPI_CXX_LIBRARIES})

# ============================================
# Hybrid MPI+OpenMP version
# ============================================
add_executable(heat_hybrid
    src/main.cpp
    src/solver.cpp
    src/solver_mpi.cpp
    src/solver_hybrid.cpp
    ${COMMON_SOURCES}
)

target_compile_definitions(heat_hybrid PRIVATE USE_MPI USE_OPENMP)
target_link_libraries(heat_hybrid ${MPI_CXX_LIBRARIES} OpenMP::OpenMP_CXX)

# ============================================
# Test executables
# ============================================
# Only build tests if they exist
if(EXISTS ${CMAKE_SOURCE_DIR}/tests/test_sequential.cpp)
    add_executable(test_seq tests/test_sequential.cpp src/solver.cpp src/solver_sequential.cpp ${COMMON_SOURCES})
    target_link_libraries(test_seq)
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/tests/test_mpi.cpp)
    add_executable(test_mpi tests/test_mpi.cpp src/solver_mpi.cpp ${COMMON_SOURCES})
    target_link_libraries(test_mpi ${MPI_CXX_LIBRARIES})
endif()

# Installation (optional)
install(TARGETS heat_seq heat_omp heat_mpi heat_hybrid
    RUNTIME DESTINATION bin
)

# Print build summary
message(STATUS "========================================")
message(STATUS "Build Configuration:")
message(STATUS "  Sequential: heat_seq")
message(STATUS "  OpenMP: heat_omp")
message(STATUS "  MPI: heat_mpi")
message(STATUS "  Hybrid: heat_hybrid")
if(EXISTS ${CMAKE_SOURCE_DIR}/tests/test_sequential.cpp)
    message(STATUS "  Tests: test_seq")
endif()
message(STATUS "========================================")
