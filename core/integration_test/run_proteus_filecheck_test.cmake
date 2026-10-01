# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# SPDX-FileCopyrightText: Copyright Contributors to the Kokkos project

set(output_file "${CACHE_DIR}.stdout")

file(REMOVE_RECURSE "${CACHE_DIR}")
file(REMOVE "${output_file}")

execute_process(
  COMMAND
    "${CMAKE_COMMAND}" -E env "PROTEUS_CACHE_DIR=${CACHE_DIR}" "PROTEUS_TRACE_OUTPUT=specialization"
    "${TEST_EXECUTABLE}"
  RESULT_VARIABLE test_result
  OUTPUT_VARIABLE test_stdout
  ERROR_VARIABLE test_stderr
)

if(NOT test_result EQUAL 0)
  message(
    FATAL_ERROR
      "Proteus integration test exited with ${test_result}\nstdout:\n${test_stdout}\nstderr:\n${test_stderr}"
  )
endif()

file(WRITE "${output_file}" "${test_stdout}")

execute_process(
  COMMAND "${FILECHECK_EXECUTABLE}" "${SOURCE_FILE}"
  INPUT_FILE "${output_file}"
  RESULT_VARIABLE filecheck_result
  OUTPUT_VARIABLE filecheck_stdout
  ERROR_VARIABLE filecheck_stderr
)

file(REMOVE_RECURSE "${CACHE_DIR}")
file(REMOVE "${output_file}")

if(NOT filecheck_result EQUAL 0)
  message(
    FATAL_ERROR
      "FileCheck failed with ${filecheck_result}\nstdout:\n${test_stdout}\nstderr:\n${test_stderr}\nFileCheck stdout:\n${filecheck_stdout}\nFileCheck stderr:\n${filecheck_stderr}"
  )
endif()
