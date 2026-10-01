# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# SPDX-FileCopyrightText: Copyright Contributors to the Kokkos project

if(Kokkos_ENABLE_JIT AND Kokkos_ENABLE_HIP)
  find_program(
    KOKKOS_PROTEUS_FILECHECK
    FileCheck
    PATHS ${LLVM_TOOLS_BINARY_DIR} ${LLVM_BINARY_DIR}/bin ${LLVM_BINARY_DIR}/libexec/llvm
  )

  if(KOKKOS_PROTEUS_FILECHECK)
    function(kokkos_add_proteus_filecheck_test target source)
      kokkos_add_executable(${target} SOURCES ${source})

      add_test(
        NAME Kokkos_${target}
        COMMAND
          ${CMAKE_COMMAND} -DTEST_EXECUTABLE=$<TARGET_FILE:Kokkos_${target}>
          -DFILECHECK_EXECUTABLE=${KOKKOS_PROTEUS_FILECHECK} -DSOURCE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/${source}
          -DCACHE_DIR=${CMAKE_CURRENT_BINARY_DIR}/${target}.cache -P
          ${CMAKE_CURRENT_SOURCE_DIR}/run_proteus_filecheck_test.cmake
      )
      set_tests_properties(Kokkos_${target} PROPERTIES LABELS Kokkos)
    endfunction()

    kokkos_add_proteus_filecheck_test(ProteusRangePolicy range_policy.cpp)
    kokkos_add_proteus_filecheck_test(ProteusParallelReduce parallel_reduce.cpp)
    kokkos_add_proteus_filecheck_test(ProteusTeamPolicy team_policy.cpp)
  else()
    message(STATUS "Skipping Proteus integration tests: FileCheck not found")
  endif()
endif()
