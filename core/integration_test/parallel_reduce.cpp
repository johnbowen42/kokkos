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

  const int a      = runtime_a;
  const int b      = runtime_b;
  const bool accum = runtime_accum != 0;
  int result       = 0;

  proteus::enable();
  Kokkos::parallel_reduce(
      "Proteus parallel_reduce", Kokkos::RangePolicy<>(0, 1),
      proteus::register_lambda(
          [=, a = proteus::jit_variable(a), b = proteus::jit_variable(b),
           accum = proteus::jit_variable(accum)] KOKKOS_FUNCTION(const int i,
                                                                 int& update) {
            update += accum ? a + b + i : a - b;
          }),
      result);

  Kokkos::finalize();

  std::printf("parallel reduce result=%d\n", result);
  return result == 40 ? 0 : 1;
}

// CHECK-DAG: [LambdaSpec] Replacing slot 2 with i8 1
// CHECK-DAG: [LambdaSpec] Replacing slot 1 with i32 16
// CHECK-DAG: [LambdaSpec] Replacing slot 0 with i32 24
// CHECK: parallel reduce result=40
