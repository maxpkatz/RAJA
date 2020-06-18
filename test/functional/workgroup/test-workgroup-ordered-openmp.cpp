//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016-20, Lawrence Livermore National Security, LLC
// and RAJA project contributors. See the RAJA/COPYRIGHT file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

///
/// Source file containing tests for RAJA workgroup ordered runs.
///

#include "tests/test-workgroup-ordered.hpp"

#if defined(RAJA_ENABLE_OPENMP)
using OpenMPBasicWorkGroupOrderedTypes =
  Test< camp::cartesian_product< OpenMPExecPolicyList,
                                 OpenMPOrderedPolicyList,
                                 OpenMPStoragePolicyList,
                                 IndexTypeTypeList,
                                 HostAllocatorList,
                                 HostResourceList > >::Types;

INSTANTIATE_TYPED_TEST_SUITE_P(OpenMPBasicTest,
                               WorkGroupBasicOrderedFunctionalTest,
                               OpenMPBasicWorkGroupOrderedTypes);
#endif