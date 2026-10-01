// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// SPDX-FileCopyrightText: Copyright Contributors to the Kokkos project

// This example mirrors RAJA's forall-jit example with Kokkos::parallel_for.
// Runtime loop bounds and a branch condition are specialized by Proteus.

#include <Kokkos_Core.hpp>
#include <proteus/JitInterface.h>

#include <cstdlib>
#include <iostream>

int main(int argc, char* argv[]) {
  if (argc != 5) {
    std::cerr << "Usage: " << argv[0]
              << " <rows> <columns> <iterations> <accumulate: 0|1>\n";
    return 1;
  }

  const int rows        = std::atoi(argv[1]);
  const int columns     = std::atoi(argv[2]);
  const int iterations  = std::atoi(argv[3]);
  const bool accumulate = std::atoi(argv[4]) != 0;
  if (rows <= 0 || columns <= 0 || iterations <= 0) return 1;

  Kokkos::initialize(argc, argv);
  {
    Kokkos::View<double*> output("output", iterations);

    proteus::disable();
    Kokkos::Timer aot_timer;
    Kokkos::parallel_for(
        "AOT parallel_for", iterations, KOKKOS_LAMBDA(const int i) {
          double value = accumulate ? output(i) : 0.0;
          for (int row = 0; row < rows; ++row)
            for (int column = 0; column < columns; ++column)
              value += static_cast<double>((i + row) * (column + 1));
          output(i) = value;
        });
    Kokkos::fence();
    const double aot_time = aot_timer.seconds();

    proteus::enable();
    Kokkos::Timer jit_timer;
    Kokkos::parallel_for(
        "JIT parallel_for", iterations,
        proteus::register_lambda(
            [=, rows = proteus::jit_variable(rows),
             columns    = proteus::jit_variable(columns),
             accumulate = proteus::jit_variable(
                 accumulate)] KOKKOS_FUNCTION(const int i) {
              double value = accumulate ? output(i) : 0.0;
              for (int row = 0; row < rows; ++row)
                for (int column = 0; column < columns; ++column)
                  value += static_cast<double>((i + row) * (column + 1));
              output(i) = value;
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
