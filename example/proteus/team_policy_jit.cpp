// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// SPDX-FileCopyrightText: Copyright Contributors to the Kokkos project

// This example applies Proteus specialization to hierarchical parallelism,
// analogous to RAJA's launch-jit example.

#include <Kokkos_Core.hpp>
#include <proteus/JitInterface.h>

#include <cstdlib>
#include <iostream>

int main(int argc, char* argv[]) {
  if (argc != 5) {
    std::cerr << "Usage: " << argv[0]
              << " <teams> <rows per team> <inner work> <accumulate: 0|1>\n";
    return 1;
  }

  const int teams       = std::atoi(argv[1]);
  const int rows        = std::atoi(argv[2]);
  const int inner_work  = std::atoi(argv[3]);
  const bool accumulate = std::atoi(argv[4]) != 0;
  if (teams <= 0 || rows <= 0 || inner_work <= 0) return 1;

  Kokkos::initialize(argc, argv);
  {
    using policy_type = Kokkos::TeamPolicy<>;
    using member_type = policy_type::member_type;
    Kokkos::View<double**> output("output", teams, rows);
    const policy_type policy(teams, Kokkos::AUTO);

    proteus::disable();
    Kokkos::Timer aot_timer;
    Kokkos::parallel_for(
        "AOT TeamPolicy", policy, KOKKOS_LAMBDA(const member_type& team) {
          Kokkos::parallel_for(
              Kokkos::TeamThreadRange(team, rows), [&](const int row) {
                double value =
                    accumulate ? output(team.league_rank(), row) : 0.0;
                for (int k = 0; k < inner_work; ++k)
                  value += static_cast<double>((row + 1) * (k + 1));
                output(team.league_rank(), row) = value;
              });
        });
    Kokkos::fence();
    const double aot_time = aot_timer.seconds();

    proteus::enable();
    Kokkos::Timer jit_timer;
    Kokkos::parallel_for(
        "JIT TeamPolicy", policy,
        proteus::register_lambda(
            [=, rows = proteus::jit_variable(rows),
             inner_work = proteus::jit_variable(inner_work),
             accumulate = proteus::jit_variable(
                 accumulate)] KOKKOS_FUNCTION(const member_type& team) {
              Kokkos::parallel_for(
                  Kokkos::TeamThreadRange(team, rows), [&](const int row) {
                    double value =
                        accumulate ? output(team.league_rank(), row) : 0.0;
                    for (int k = 0; k < inner_work; ++k)
                      value += static_cast<double>((row + 1) * (k + 1));
                    output(team.league_rank(), row) = value;
                  });
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
