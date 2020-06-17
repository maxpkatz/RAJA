//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016-20, Lawrence Livermore National Security, LLC
// and RAJA project contributors. See the RAJA/COPYRIGHT file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

///
/// Header file containing tests for RAJA workgroup constructors.
///

#ifndef __TEST_WORKGROUP_VTABLE__
#define __TEST_WORKGROUP_VTABLE__

#include "../test-workgroup.hpp"


template <typename T>
class WorkGroupBasicVtableUnitTest : public ::testing::Test
{
};

TYPED_TEST_SUITE_P(WorkGroupBasicVtableUnitTest);


template  < typename ForOnePol,
            typename ... CallArgs >
typename  std::enable_if<
            std::is_base_of<RunOnHost, ForOnePol>::value
          >::type
call_dispatcher( void(*call_function)(CallArgs...),
                 CallArgs... callArgs )
{
  forone<ForOnePol>( [=] () {
    call_function(callArgs...);
  });
}

#if defined(RAJA_ENABLE_CUDA) || defined(RAJA_ENABLE_HIP)
template  < typename ForOnePol,
            typename ... CallArgs >
typename  std::enable_if<
            std::is_base_of<RunOnDevice, ForOnePol>::value
          >::type
call_dispatcher( void(*call_function)(CallArgs...),
                 CallArgs... callArgs )
{
  RAJA::tuple<CallArgs...> callArgs_device_lambda_workaround(callArgs...);
  forone<ForOnePol>( [=] RAJA_DEVICE () {
    camp::invoke(callArgs_device_lambda_workaround, call_function);
  });
}
#endif

template < typename IndexType,
           typename ... Args >
struct VtableTestCallable
{
  VtableTestCallable(IndexType* _ptr, IndexType _val)
    : ptr(_ptr)
    , val(_val)
  { }

  VtableTestCallable(VtableTestCallable const&) = delete;
  VtableTestCallable& operator=(VtableTestCallable const&) = delete;

  VtableTestCallable(VtableTestCallable&& o)
    : ptr(o.ptr)
    , val(o.val)
    , move_constructed(true)
  {
    o.moved_from = true;
  }
  VtableTestCallable& operator=(VtableTestCallable&& o)
  {
    ptr = o.ptr;
    val = o.val;
    o.moved_from = true;
    return *this;
  }

  RAJA_HOST_DEVICE void operator()(IndexType i, Args... args) const
  {
    RAJA_UNUSED_VAR(args...);
    ptr[i] = val;
  }

private:
  IndexType* ptr;
  IndexType  val;
public:
  bool move_constructed = false;
  bool moved_from = false;
};

template < typename ExecPolicy,
           typename IndexType,
           typename WORKING_RES,
           typename ForOnePol,
           typename ... Args >
void testWorkGroupVtable(RAJA::xargs<Args...>)
{
  using TestCallable = VtableTestCallable<IndexType, Args...>;

  camp::resources::Resource work_res{WORKING_RES()};
  camp::resources::Resource host_res{camp::resources::Host()};

  IndexType* testVal = host_res.allocate<IndexType>(3);
  IndexType* initVal = work_res.allocate<IndexType>(3);
  TestCallable* new_obj = work_res.allocate<TestCallable>(1);

  initVal[0] = (IndexType)5;
  initVal[1] = (IndexType)7;
  initVal[2] = (IndexType)5;

  testVal[0] = (IndexType)5;
  testVal[1] = (IndexType)5;
  testVal[2] = (IndexType)5;


  using Vtable_type = RAJA::detail::Vtable<IndexType, Args...>;
  Vtable_type vtable =
      RAJA::detail::get_Vtable<TestCallable, Vtable_type>(ExecPolicy{});

  TestCallable obj(testVal, initVal[1]);

  ASSERT_FALSE(obj.move_constructed);
  ASSERT_FALSE(obj.moved_from);

  vtable.move_construct_function_ptr(new_obj, &obj);

  ASSERT_FALSE(obj.move_constructed);
  ASSERT_TRUE(obj.moved_from);

  ASSERT_TRUE(new_obj->move_constructed);
  ASSERT_FALSE(new_obj->moved_from);

  // move a value onto device and fiddle
  call_dispatcher<ForOnePol, const void*, IndexType, Args...>(
      vtable.call_function_ptr, new_obj, (IndexType)1, Args{}...);

  vtable.destroy_function_ptr(new_obj);


  ASSERT_EQ(testVal[0], initVal[0]);
  ASSERT_EQ(testVal[1], initVal[1]);
  ASSERT_EQ(testVal[2], initVal[2]);

  work_res.deallocate( new_obj );
  work_res.deallocate( initVal );
  host_res.deallocate( testVal );
}

TYPED_TEST_P(WorkGroupBasicVtableUnitTest, BasicWorkGroupVtable)
{
  using ExecPolicy = typename camp::at<TypeParam, camp::num<0>>::type;
  using IndexType = typename camp::at<TypeParam, camp::num<1>>::type;
  using Args = typename camp::at<TypeParam, camp::num<2>>::type;
  using ResourceType = typename camp::at<TypeParam, camp::num<3>>::type;
  using ForOneType = typename camp::at<TypeParam, camp::num<4>>::type;

  testWorkGroupVtable< ExecPolicy, IndexType, ResourceType, ForOneType >(
      Args{});
}


REGISTER_TYPED_TEST_SUITE_P(WorkGroupBasicVtableUnitTest,
                            BasicWorkGroupVtable);

#endif  //__TEST_WORKGROUP_VTABLE__
