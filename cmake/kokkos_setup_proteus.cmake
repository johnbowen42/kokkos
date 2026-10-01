# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# SPDX-FileCopyrightText: Copyright Contributors to the Kokkos project

if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  message(
    FATAL_ERROR
      "Kokkos requires an LLVM-based compiler when Kokkos_ENABLE_JIT is enabled, but found ${CMAKE_CXX_COMPILER_ID}"
  )
endif()

# Use a user-provided Proteus installation when requested. Otherwise, fetch the
# same pinned Proteus revision used by RAJA's JIT integration.
if(DEFINED PROTEUS_INSTALL_DIR)
  find_package(proteus REQUIRED PATHS "${PROTEUS_INSTALL_DIR}")
else()
  set(PROTEUS_ENABLE_HIP "${Kokkos_ENABLE_HIP}" CACHE BOOL "Enable JIT compilation of HIP kernels")
  set(PROTEUS_ENABLE_CUDA "${Kokkos_ENABLE_CUDA}" CACHE BOOL "Enable JIT compilation of CUDA kernels")

  # Proteus uses the generic ENABLE_TESTS option. Keep its tests out of a
  # Kokkos build without changing the value in the including project.
  if(DEFINED ENABLE_TESTS)
    set(_KOKKOS_SAVED_ENABLE_TESTS "${ENABLE_TESTS}")
    set(_KOKKOS_ENABLE_TESTS_WAS_DEFINED ON)
  endif()
  set(ENABLE_TESTS OFF)

  # Proteus defaults CMAKE_CXX_STANDARD to 17 in the cache. Preserve Kokkos's
  # selected standard so a subsequent configure does not get downgraded.
  get_property(_KOKKOS_CXX_STANDARD_CACHE_TYPE CACHE CMAKE_CXX_STANDARD PROPERTY TYPE)
  if(_KOKKOS_CXX_STANDARD_CACHE_TYPE)
    get_property(_KOKKOS_SAVED_CXX_STANDARD CACHE CMAKE_CXX_STANDARD PROPERTY VALUE)
  endif()
  set(CMAKE_CXX_STANDARD "${KOKKOS_CXX_STANDARD}" CACHE STRING "C++ standard" FORCE)

  include(FetchContent)
  FetchContent_Declare(
    proteus
    GIT_REPOSITORY https://github.com/Olympus-HPC/proteus.git
    GIT_TAG a4d845581d70a522ec323d0ce0127c1af08cb038
  )
  FetchContent_MakeAvailable(proteus)

  if(_KOKKOS_CXX_STANDARD_CACHE_TYPE)
    set(CMAKE_CXX_STANDARD "${_KOKKOS_SAVED_CXX_STANDARD}" CACHE STRING "C++ standard" FORCE)
  else()
    unset(CMAKE_CXX_STANDARD CACHE)
  endif()
  unset(_KOKKOS_CXX_STANDARD_CACHE_TYPE)
  unset(_KOKKOS_SAVED_CXX_STANDARD)

  if(_KOKKOS_ENABLE_TESTS_WAS_DEFINED)
    set(ENABLE_TESTS "${_KOKKOS_SAVED_ENABLE_TESTS}")
  else()
    unset(ENABLE_TESTS)
  endif()
  unset(_KOKKOS_ENABLE_TESTS_WAS_DEFINED)
  unset(_KOKKOS_SAVED_ENABLE_TESTS)

  include(${proteus_SOURCE_DIR}/cmake/ProteusFunctions.cmake)
endif()
