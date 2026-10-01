// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// SPDX-FileCopyrightText: Copyright Contributors to the Kokkos project

// This example applies Proteus specialization to a Kokkos MDRangePolicy
// kernel, analogous to RAJA's kernel-jit example.

#include <Kokkos_Core.hpp>
#include <proteus/JitInterface.h>

#include <cstdlib>
#include <iostream>

int main(int argc, char* argv[]) {
  if (argc != 5) {
    std::cerr << "Usage: " << argv[0]
              << " <rows> <columns> <inner work> <accumulate: 0|1>\n";
    return 1;
  }

  const int rows        = std::atoi(argv[1]);
  const int columns     = std::atoi(argv[2]);
  const int inner_work  = std::atoi(argv[3]);
  const bool accumulate = std::atoi(argv[4]) != 0;
  if (rows <= 0 || columns <= 0 || inner_work <= 0) return 1;

  Kokkos::initialize(argc, argv);
  {
    Kokkos::View<double**> output("output", rows, columns);
    const Kokkos::MDRangePolicy<Kokkos::Rank<2>> policy({0, 0},
                                                        {rows, columns});

    proteus::disable();
    Kokkos::Timer aot_timer;
    Kokkos::parallel_for(
        "AOT MDRangePolicy", policy,
        KOKKOS_LAMBDA(const int row, const int column) {
          double value = accumulate ? output(row, column) : 0.0;
          for (int k = 0; k < inner_work; ++k)
            value += static_cast<double>((row + 1) * (column + k));
          output(row, column) = value;
        });
    Kokkos::fence();
    const double aot_time = aot_timer.seconds();

    proteus::enable();
    Kokkos::Timer jit_timer;
    Kokkos::parallel_for(
        "JIT MDRangePolicy", policy,
        proteus::register_lambda(
            [=, inner_work = proteus::jit_variable(inner_work),
             accumulate = proteus::jit_variable(
                 accumulate)] KOKKOS_FUNCTION(const int row, const int column) {
              double value = accumulate ? output(row, column) : 0.0;
              for (int k = 0; k < inner_work; ++k)
                value += static_cast<double>((row + 1) * (column + k));
              output(row, column) = value;
            }));
    Kokkos::fence();
    const double jit_time = jit_timer.seconds();

    std::cout << "aot total time = " << aot_time << '\n'
              << "jit total time = " << jit_time << '\n'
              << "speedup = " << aot_time / jit_time << '\n';
  }
  Kokkos::finalize();
  return 0;
}
