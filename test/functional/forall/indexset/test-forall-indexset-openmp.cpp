//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016-20, Lawrence Livermore National Security, LLC
// and RAJA project contributors. See the RAJA/COPYRIGHT file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#include "test-forall-indexset.hpp"

#if defined(RAJA_ENABLE_OPENMP)

// Cartesian product of types for OpenMP tests
using OpenMPForallIndexSetTypes =
  Test< camp::cartesian_product<IdxTypeList, 
                                HostResourceList, 
                                OpenMPForallIndexSetExecPols>>::Types;

INSTANTIATE_TYPED_TEST_SUITE_P(OpenMP,
                               ForallIndexSetTest,
                               OpenMPForallIndexSetTypes);

#endif
