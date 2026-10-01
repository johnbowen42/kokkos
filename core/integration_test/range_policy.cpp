// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// SPDX-FileCopyrightText: Copyright Contributors to the Kokkos project

#include <Kokkos_Core.hpp>
#include <proteus/JitInterface.h>

#include <cstdio>

static volatile int runtime_a     = 24;
static volatile int runtime_b     = 16;
static volatile int runtime_accum = 1;

int main(int argc, char* argv[]) {
  Kokkos::initialize(argc, argv);
  int host_result = -1;
  {
    const int a      = runtime_a;
    const int b      = runtime_b;
    const bool accum = runtime_accum != 0;
    Kokkos::View<int*> output("output", 1);

    Kokkos::deep_copy(output, 0);
    proteus::enable();
    Kokkos::parallel_for(
        "Proteus RangePolicy", Kokkos::RangePolicy<>(0, 1),
        proteus::register_lambda(
            [=, a = proteus::jit_variable(a), b = proteus::jit_variable(b),
             accum =
                 proteus::jit_variable(accum)] KOKKOS_FUNCTION(const int i) {
              output(i) = accum ? a + b + i : a - b;
            }));
    Kokkos::fence();

    Kokkos::deep_copy(host_result, Kokkos::subview(output, 0));
  }
  Kokkos::finalize();

  std::printf("range policy result=%d\n", host_result);
  return host_result == 40 ? 0 : 1;
}

// CHECK-DAG: [LambdaSpec] Replacing slot 2 with i8 1
// CHECK-DAG: [LambdaSpec] Replacing slot 1 with i32 16
// CHECK-DAG: [LambdaSpec] Replacing slot 0 with i32 24
// CHECK: range policy result=40
