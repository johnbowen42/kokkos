# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# SPDX-FileCopyrightText: Copyright Contributors to the Kokkos project

kokkos_add_executable(proteus_parallel_for_jit SOURCES parallel_for_jit.cpp)
kokkos_add_executable(proteus_mdrange_jit SOURCES mdrange_jit.cpp)
kokkos_add_executable(proteus_team_policy_jit SOURCES team_policy_jit.cpp)
