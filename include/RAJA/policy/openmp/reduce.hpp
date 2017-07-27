/*!
 ******************************************************************************
 *
 * \file
 *
 * \brief   Header file containing RAJA reduction templates for OpenMP
 *          OpenMP execution.
 *
 *          These methods should work on any platform that supports OpenMP.
 *
 ******************************************************************************
 */

#ifndef RAJA_forall_omp_HPP
#define RAJA_forall_omp_HPP

#include "RAJA/config.hpp"

#if defined(ENABLE_OPENMP)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016, Lawrence Livermore National Security, LLC.
//
// Produced at the Lawrence Livermore National Laboratory
//
// LLNL-CODE-689114
//
// All rights reserved.
//
// This file is part of RAJA.
//
// For additional details, please also read RAJA/LICENSE.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the disclaimer below.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the disclaimer (as noted below) in the
//   documentation and/or other materials provided with the distribution.
//
// * Neither the name of the LLNS/LLNL nor the names of its contributors may
//   be used to endorse or promote products derived from this software without
//   specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL SECURITY,
// LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
// IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#include "RAJA/util/types.hpp"

#include "RAJA/internal/MemUtils_CPU.hpp"
#include "RAJA/pattern/reduce.hpp"
#include "RAJA/policy/openmp/policy.hpp"
#include "RAJA/policy/openmp/target_reduce.hpp"

#include <omp.h>

namespace RAJA {

/*!
 **************************************************************************
 *
 * \brief  Min reducer class template for use in OpenMP execution.
 *
 **************************************************************************
 */
template <typename T> class ReduceMin<omp_reduce, T> {
  static constexpr const RAJA::reduce::min<T> Reduce{};

public:
  //! prohibit compiler-generated default ctor
  ReduceMin() = delete;

  //! prohibit compiler-generated copy assignment
  ReduceMin &operator=(const ReduceMin &) = delete;

  //! compiler-generated move constructor
  ReduceMin(ReduceMin &&) = default;

  //! compiler-generated move assignment
  ReduceMin &operator=(ReduceMin &&) = default;

  //! constructor requires a default value for the reducer
  RAJA_HOST_DEVICE explicit ReduceMin(T init_val)
      : m_parent(nullptr), m_val(init_val) {}

  //! create a copy of the reducer
  /*!
   * keep parent the same if non-null or set to current
   */
  RAJA_HOST_DEVICE ReduceMin(const ReduceMin &other)
      : m_parent(other.m_parent ? other.m_parent : &other), m_val(other.m_val) {
  }

  //! Destructor folds value into parent object.
  RAJA_HOST_DEVICE ~ReduceMin() {
    if (m_parent) {
#pragma omp critical
      { Reduce(m_parent->m_val, m_val); }
    }
  }

  //! return the reduced min value.
  /*!
   *  \return the calculated reduced value
   */
  RAJA_HOST_DEVICE operator T() { return m_val; }

  //! return the reduced min value.
  /*!
   *  \return the calculated reduced value
   */
  RAJA_HOST_DEVICE T get() { return operator T(); }

  //! reducer function; updates the current instance's state
  /*!
   * Assumes each thread has its own copy of the object.
   */
  RAJA_HOST_DEVICE const ReduceMin &min(T rhs) const {
    Reduce(m_val, rhs);
    return *this;
  }

  //! reducer function; updates the current instance's state
  /*!
   * Assumes each thread has its own copy of the object.
   */
  RAJA_HOST_DEVICE ReduceMin &min(T rhs) {
    Reduce(m_val, rhs);
    return *this;
  }

private:
  //! pointer to the parent ReduceMin object
  const ReduceMin *m_parent;
  mutable T m_val;
};

/*!
 **************************************************************************
 *
 * \brief  MinLoc reducer class template for use in OpenMP execution.
 *
 **************************************************************************
 */
template <typename T> class ReduceMinLoc<omp_reduce, T> {
  static constexpr const RAJA::reduce::minloc<T, Index_type> Reduce{};

public:
  //! prohibit compiler-generated default ctor
  ReduceMinLoc() = delete;

  //! prohibit compiler-generated copy assignment
  ReduceMinLoc &operator=(const ReduceMinLoc &) = delete;

  //! compiler-generated move constructor
  ReduceMinLoc(ReduceMinLoc &&) = default;

  //! compiler-generated move assignment
  ReduceMinLoc &operator=(ReduceMinLoc &&) = default;

  //! constructor requires a default value for the reducer
  RAJA_HOST_DEVICE explicit ReduceMinLoc(T init_val, Index_type init_idx)
      : m_parent(nullptr), m_val(init_val), m_idx(init_idx) {}

  //! create a copy of the reducer
  /*!
   * keep parent the same if non-null or set to current
   */
  RAJA_HOST_DEVICE ReduceMinLoc(const ReduceMinLoc &other)
      : m_parent(other.m_parent ? other.m_parent : &other), m_val(other.m_val),
        m_idx(other.m_idx) {}

  //! Destructor folds value into parent object.
  RAJA_HOST_DEVICE ~ReduceMinLoc() {
    if (m_parent) {
#pragma omp critical
      { Reduce(m_parent->m_val, m_parent->m_idx, m_val, m_idx); }
    }
  }

  //! return the reduced min value.
  /*!
   *  \return the calculated reduced value
   */
  RAJA_HOST_DEVICE operator T() { return m_val; }

  //! return the reduced min value.
  /*!
   *  \return the calculated reduced value
   */
  RAJA_HOST_DEVICE T get() { return operator T(); }

  //! return the index location of the minimum value
  /*!
   *  \return the index location
   */
  RAJA_HOST_DEVICE Index_type getLoc() { return m_idx; }

  //! reducer function; updates the current instance's state
  /*!
   * Assumes each thread has its own copy of the object.
   */
  RAJA_HOST_DEVICE const ReduceMinLoc &minloc(T rhs, Index_type idx) const {
    Reduce(m_val, m_idx, rhs, idx);
    return *this;
  }

  //! reducer function; updates the current instance's state
  /*!
   * Assumes each thread has its own copy of the object.
   */
  RAJA_HOST_DEVICE ReduceMinLoc &minloc(T rhs, Index_type idx) {
    Reduce(m_val, m_idx, rhs, idx);
    return *this;
  }

private:
  //! pointer to the parent ReduceMinLoc object
  const ReduceMinLoc *m_parent;
  mutable T m_val;
  mutable Index_type m_idx;
};

/*!
 **************************************************************************
 *
 * \brief  Max reducer class template for use in OpenMP execution.
 *
 **************************************************************************
 */
template <typename T> class ReduceMax<omp_reduce, T> {
  static constexpr const RAJA::reduce::max<T> Reduce{};

public:
  //! prohibit compiler-generated default ctor
  ReduceMax() = delete;

  //! prohibit compiler-generated copy assignment
  ReduceMax &operator=(const ReduceMax &) = delete;

  //! compiler-generated move constructor
  ReduceMax(ReduceMax &&) = default;

  //! compiler-generated move assignment
  ReduceMax &operator=(ReduceMax &&) = default;

  //! constructor requires a default value for the reducer
  RAJA_HOST_DEVICE explicit ReduceMax(T init_val)
      : m_parent(nullptr), m_val(init_val) {}

  //! create a copy of the reducer
  /*!
   * keep parent the same if non-null or set to current
   */
  RAJA_HOST_DEVICE ReduceMax(const ReduceMax &other)
      : m_parent(other.m_parent ? other.m_parent : &other), m_val(other.m_val) {
  }

  //! Destructor folds value into parent object.
  RAJA_HOST_DEVICE ~ReduceMax() {
    if (m_parent) {
#pragma omp critical
      { Reduce(m_parent->m_val, m_val); }
    }
  }

  //! return the reduced min value.
  /*!
   *  \return the calculated reduced value
   */
  RAJA_HOST_DEVICE operator T() { return m_val; }

  //! return the reduced min value.
  /*!
   *  \return the calculated reduced value
   */
  RAJA_HOST_DEVICE T get() { return operator T(); }

  //! reducer function; updates the current instance's state
  /*!
   * Assumes each thread has its own copy of the object.
   */
  RAJA_HOST_DEVICE const ReduceMax &max(T rhs) const {
    Reduce(m_val, rhs);
    return *this;
  }

  //! reducer function; updates the current instance's state
  /*!
   * Assumes each thread has its own copy of the object.
   */
  RAJA_HOST_DEVICE ReduceMax &max(T rhs) {
    Reduce(m_val, rhs);
    return *this;
  }

private:
  //! pointer to the parent ReduceMax object
  const ReduceMax *m_parent;
  mutable T m_val;
};

/*!
 **************************************************************************
 *
 * \brief  Sum reducer class template for use in OpenMP execution.
 *
 **************************************************************************
 */
template <typename T> class ReduceSum<omp_reduce, T> {
  static constexpr const RAJA::reduce::sum<T> Reduce{};

public:
  //! prohibit compiler-generated default ctor
  ReduceSum() = delete;

  //! prohibit compiler-generated copy assignment
  ReduceSum &operator=(const ReduceSum &) = delete;

  //! compiler-generated move constructor
  ReduceSum(ReduceSum &&) = default;

  //! compiler-generated move assignment
  ReduceSum &operator=(ReduceSum &&) = default;

  //! constructor requires a default value for the reducer
  RAJA_HOST_DEVICE explicit ReduceSum(T init_val, T initializer = T())
      : m_parent(nullptr), m_val(init_val), m_custom_init(initializer) {}

  //! create a copy of the reducer
  /*!
   * keep parent the same if non-null or set to current
   */
  RAJA_HOST_DEVICE ReduceSum(const ReduceSum &other)
      : m_parent(other.m_parent ? other.m_parent : &other),
        m_val(other.m_custom_init), m_custom_init(other.m_custom_init) {}

  //! Destructor folds value into parent object.
  RAJA_HOST_DEVICE ~ReduceSum() {
    if (m_parent) {
#pragma omp critical
      { Reduce(m_parent->m_val, m_val); }
    }
  }

  //! return the reduced min value.
  /*!
   *  \return the calculated reduced value
   */
  RAJA_HOST_DEVICE operator T() { return m_val; }

  //! return the reduced min value.
  /*!
   *  \return the calculated reduced value
   */
  RAJA_HOST_DEVICE T get() { return operator T(); }

  //! reducer function; updates the current instance's state
  /*!
   * Assumes each thread has its own copy of the object.
   */
  RAJA_HOST_DEVICE const ReduceSum &operator+=(T rhs) const {
    Reduce(m_val, rhs);
    return *this;
  }

  //! reducer function; updates the current instance's state
  /*!
   * Assumes each thread has its own copy of the object.
   */
  RAJA_HOST_DEVICE ReduceSum &operator+=(T rhs) {
    Reduce(m_val, rhs);
    return *this;
  }

private:
  //! pointer to the parent ReduceSum object
  const ReduceSum *m_parent;
  mutable T m_val;
  const T m_custom_init;
};

/*!
 **************************************************************************
 *
 * \brief  MaxLoc reducer class template for use in OpenMP execution.
 *
 **************************************************************************
 */
template <typename T> class ReduceMaxLoc<omp_reduce, T> {
  static constexpr const RAJA::reduce::maxloc<T, Index_type> Reduce{};

public:
  //! prohibit compiler-generated default ctor
  ReduceMaxLoc() = delete;

  //! prohibit compiler-generated copy assignment
  ReduceMaxLoc &operator=(const ReduceMaxLoc &) = delete;

  //! compiler-generated move constructor
  ReduceMaxLoc(ReduceMaxLoc &&) = default;

  //! compiler-generated move assignment
  ReduceMaxLoc &operator=(ReduceMaxLoc &&) = default;

  //! constructor requires a default value for the reducer
  RAJA_HOST_DEVICE explicit ReduceMaxLoc(T init_val, Index_type init_idx)
      : m_parent(nullptr), m_val(init_val), m_idx(init_idx) {}

  //! create a copy of the reducer
  /*!
   * keep parent the same if non-null or set to current
   */
  RAJA_HOST_DEVICE ReduceMaxLoc(const ReduceMaxLoc &other)
      : m_parent(other.m_parent ? other.m_parent : &other), m_val(other.m_val),
        m_idx(other.m_idx) {}

  //! Destructor folds value into parent object.
  RAJA_HOST_DEVICE ~ReduceMaxLoc() {
    if (m_parent) {
#pragma omp critical
      { Reduce(m_parent->m_val, m_parent->m_idx, m_val, m_idx); }
    }
  }

  //! return the reduced min value.
  /*!
   *  \return the calculated reduced value
   */
  RAJA_HOST_DEVICE operator T() { return m_val; }

  //! return the reduced min value.
  /*!
   *  \return the calculated reduced value
   */
  RAJA_HOST_DEVICE T get() { return operator T(); }

  //! return the index location of the maximum value
  /*!
   *  \return the index location
   */
  RAJA_HOST_DEVICE Index_type getLoc() { return m_idx; }

  //! reducer function; updates the current instance's state
  /*!
   * Assumes each thread has its own copy of the object.
   */
  RAJA_HOST_DEVICE const ReduceMaxLoc &maxloc(T rhs, Index_type idx) const {
    Reduce(m_val, m_idx, rhs, idx);
    return *this;
  }

  //! reducer function; updates the current instance's state
  /*!
   * Assumes each thread has its own copy of the object.
   */
  RAJA_HOST_DEVICE ReduceMaxLoc &maxloc(T rhs, Index_type idx) {
    Reduce(m_val, m_idx, rhs, idx);
    return *this;
  }

private:
  //! pointer to the parent ReduceMaxLoc object
  const ReduceMaxLoc *m_parent;
  mutable T m_val;
  mutable Index_type m_idx;
};

///////////////////////////////////////////////////////////////////////////////
//
// Old ordered reductions are included below.
//
///////////////////////////////////////////////////////////////////////////////

/*!
 ******************************************************************************
 *
 * \brief  Min reducer class template for use in OpenMP execution.
 *
 *         For usage example, see reducers.hxx.
 *
 ******************************************************************************
 */
template <typename T>
class ReduceMin<omp_reduce_ordered, T>
{
public:
  //
  // Constructor takes default value (default ctor is disabled).
  //
  explicit ReduceMin(T init_val)
  {
    m_is_copy = false;

    m_reduced_val = init_val;

    m_myID = getCPUReductionId();

    m_blockdata = getCPUReductionMemBlock(m_myID);

    int nthreads = omp_get_max_threads();
#pragma omp parallel for schedule(static, 1)
    for (int i = 0; i < nthreads; ++i) {
      m_blockdata[i * s_block_offset] = init_val;
    }
  }

  //
  // Copy ctor.
  //
  ReduceMin(const ReduceMin<omp_reduce_ordered, T>& other)
  {
    *this = other;
    m_is_copy = true;
  }

  //
  // Destruction releases the shared memory block chunk for reduction id
  // and id itself for others to use.
  //
  ~ReduceMin<omp_reduce_ordered, T>()
  {
    if (!m_is_copy) {
      releaseCPUReductionId(m_myID);
    }
  }

  //
  // Operator that returns reduced min value.
  //
  operator T()
  {
    int nthreads = omp_get_max_threads();
    for (int i = 0; i < nthreads; ++i) {
      m_reduced_val = RAJA_MIN(m_reduced_val,
                               static_cast<T>(m_blockdata[i * s_block_offset]));
    }

    return m_reduced_val;
  }

  //
  // Method that returns reduced min value.
  //
  T get() { return operator T(); }

  //
  // Method that updates min value for current thread.
  //
  ReduceMin<omp_reduce_ordered, T> min(T val) const
  {
    int tid = omp_get_thread_num();
    int idx = tid * s_block_offset;
    m_blockdata[idx] = RAJA_MIN(static_cast<T>(m_blockdata[idx]), val);

    return *this;
  }

private:
  //
  // Default ctor is declared private and not implemented.
  //
  ReduceMin<omp_reduce_ordered, T>();

  static const int s_block_offset =
      COHERENCE_BLOCK_SIZE / sizeof(CPUReductionBlockDataType);

  bool m_is_copy;
  int m_myID;

  T m_reduced_val;

  CPUReductionBlockDataType* m_blockdata;
};

/*!
 ******************************************************************************
 *
 * \brief  Min-loc reducer class template for use in OpenMP execution.
 *
 *         For usage example, see reducers.hxx.
 *
 ******************************************************************************
 */
template <typename T>
class ReduceMinLoc<omp_reduce_ordered, T>
{
public:
  //
  // Constructor takes default value (default ctor is disabled).
  //
  explicit ReduceMinLoc(T init_val, Index_type init_loc)
  {
    m_is_copy = false;

    m_reduced_val = init_val;
    m_reduced_idx = init_loc;

    m_myID = getCPUReductionId();

    m_blockdata = getCPUReductionMemBlock(m_myID);
    m_idxdata = getCPUReductionLocBlock(m_myID);

    int nthreads = omp_get_max_threads();
#pragma omp parallel for schedule(static, 1)
    for (int i = 0; i < nthreads; ++i) {
      m_blockdata[i * s_block_offset] = init_val;
      m_idxdata[i * s_idx_offset] = init_loc;
    }
  }

  //
  // Copy ctor.
  //
  ReduceMinLoc(const ReduceMinLoc<omp_reduce_ordered, T>& other)
  {
    *this = other;
    m_is_copy = true;
  }

  //
  // Destruction releases the shared memory block chunk for reduction id
  // and id itself for others to use.
  //
  ~ReduceMinLoc<omp_reduce_ordered, T>()
  {
    if (!m_is_copy) {
      releaseCPUReductionId(m_myID);
    }
  }

  //
  // Operator that returns reduced min value.
  //
  operator T()
  {
    int nthreads = omp_get_max_threads();
    for (int i = 0; i < nthreads; ++i) {
      if (static_cast<T>(m_blockdata[i * s_block_offset]) < m_reduced_val) {
        m_reduced_val = m_blockdata[i * s_block_offset];
        m_reduced_idx = m_idxdata[i * s_idx_offset];
      }
    }

    return m_reduced_val;
  }

  //
  // Method that returns reduced min value.
  //
  T get() { return operator T(); }

  //
  // Method that returns index corresponding to reduced min value.
  //
  Index_type getLoc()
  {
    int nthreads = omp_get_max_threads();
    for (int i = 0; i < nthreads; ++i) {
      if (static_cast<T>(m_blockdata[i * s_block_offset]) < m_reduced_val) {
        m_reduced_val = m_blockdata[i * s_block_offset];
        m_reduced_idx = m_idxdata[i * s_idx_offset];
      }
    }

    return m_reduced_idx;
  }

  //
  // Method that updates min and index value for current thread.
  //
  ReduceMinLoc<omp_reduce_ordered, T> minloc(T val, Index_type idx) const
  {
    int tid = omp_get_thread_num();
    if (val < static_cast<T>(m_blockdata[tid * s_block_offset])) {
      m_blockdata[tid * s_block_offset] = val;
      m_idxdata[tid * s_idx_offset] = idx;
    }

    return *this;
  }

private:
  //
  // Default ctor is declared private and not implemented.
  //
  ReduceMinLoc<omp_reduce_ordered, T>();

  static const int s_block_offset =
      COHERENCE_BLOCK_SIZE / sizeof(CPUReductionBlockDataType);
  static const int s_idx_offset = COHERENCE_BLOCK_SIZE / sizeof(Index_type);

  bool m_is_copy;
  int m_myID;

  T m_reduced_val;
  Index_type m_reduced_idx;

  CPUReductionBlockDataType* m_blockdata;
  Index_type* m_idxdata;
};

/*!
 ******************************************************************************
 *
 * \brief  Max reducer class template for use in OpenMP execution.
 *
 *         For usage example, see reducers.hxx.
 *
 ******************************************************************************
 */
template <typename T>
class ReduceMax<omp_reduce_ordered, T>
{
public:
  //
  // Constructor takes default value (default ctor is disabled).
  //
  explicit ReduceMax(T init_val)
  {
    m_is_copy = false;

    m_reduced_val = init_val;

    m_myID = getCPUReductionId();

    m_blockdata = getCPUReductionMemBlock(m_myID);

    int nthreads = omp_get_max_threads();
#pragma omp parallel for schedule(static, 1)
    for (int i = 0; i < nthreads; ++i) {
      m_blockdata[i * s_block_offset] = init_val;
    }
  }

  //
  // Copy ctor.
  //
  ReduceMax(const ReduceMax<omp_reduce_ordered, T>& other)
  {
    *this = other;
    m_is_copy = true;
  }

  //
  // Destruction releases the shared memory block chunk for reduction id
  // and id itself for others to use.
  //
  ~ReduceMax<omp_reduce_ordered, T>()
  {
    if (!m_is_copy) {
      releaseCPUReductionId(m_myID);
    }
  }

  //
  // Operator that returns reduced max value.
  //
  operator T()
  {
    int nthreads = omp_get_max_threads();
    for (int i = 0; i < nthreads; ++i) {
      m_reduced_val = RAJA_MAX(m_reduced_val,
                               static_cast<T>(m_blockdata[i * s_block_offset]));
    }

    return m_reduced_val;
  }

  //
  // Method that returns reduced max value.
  //
  T get() { return operator T(); }

  //
  // Method that updates max value for current thread.
  //
  ReduceMax<omp_reduce_ordered, T> max(T val) const
  {
    int tid = omp_get_thread_num();
    int idx = tid * s_block_offset;
    m_blockdata[idx] = RAJA_MAX(static_cast<T>(m_blockdata[idx]), val);

    return *this;
  }

private:
  //
  // Default ctor is declared private and not implemented.
  //
  ReduceMax<omp_reduce_ordered, T>();

  static const int s_block_offset =
      COHERENCE_BLOCK_SIZE / sizeof(CPUReductionBlockDataType);

  bool m_is_copy;
  int m_myID;

  T m_reduced_val;

  CPUReductionBlockDataType* m_blockdata;
};

/*!
 ******************************************************************************
 *
 * \brief  Max-loc reducer class template for use in OpenMP execution.
 *
 *         For usage example, see reducers.hxx.
 *
 ******************************************************************************
 */
template <typename T>
class ReduceMaxLoc<omp_reduce_ordered, T>
{
public:
  //
  // Constructor takes default value (default ctor is disabled).
  //
  explicit ReduceMaxLoc(T init_val, Index_type init_loc)
  {
    m_is_copy = false;

    m_reduced_val = init_val;
    m_reduced_idx = init_loc;

    m_myID = getCPUReductionId();

    m_blockdata = getCPUReductionMemBlock(m_myID);
    m_idxdata = getCPUReductionLocBlock(m_myID);

    int nthreads = omp_get_max_threads();
#pragma omp parallel for schedule(static, 1)
    for (int i = 0; i < nthreads; ++i) {
      m_blockdata[i * s_block_offset] = init_val;
      m_idxdata[i * s_idx_offset] = init_loc;
    }
  }

  //
  // Copy ctor.
  //
  ReduceMaxLoc(const ReduceMaxLoc<omp_reduce_ordered, T>& other)
  {
    *this = other;
    m_is_copy = true;
  }

  //
  // Destruction releases the shared memory block chunk for reduction id
  // and id itself for others to use.
  //
  ~ReduceMaxLoc<omp_reduce_ordered, T>()
  {
    if (!m_is_copy) {
      releaseCPUReductionId(m_myID);
    }
  }

  //
  // Operator that returns reduced max value.
  //
  operator T()
  {
    int nthreads = omp_get_max_threads();
    for (int i = 0; i < nthreads; ++i) {
      if (static_cast<T>(m_blockdata[i * s_block_offset]) > m_reduced_val) {
        m_reduced_val = m_blockdata[i * s_block_offset];
        m_reduced_idx = m_idxdata[i * s_idx_offset];
      }
    }

    return m_reduced_val;
  }

  //
  // Method that returns reduced max value.
  //
  T get() { return operator T(); }

  //
  // Method that returns index corresponding to reduced max value.
  //
  Index_type getLoc()
  {
    int nthreads = omp_get_max_threads();
    for (int i = 0; i < nthreads; ++i) {
      if (static_cast<T>(m_blockdata[i * s_block_offset]) > m_reduced_val) {
        m_reduced_val = m_blockdata[i * s_block_offset];
        m_reduced_idx = m_idxdata[i * s_idx_offset];
      }
    }

    return m_reduced_idx;
  }

  //
  // Method that updates max and index value for current thread.
  //
  ReduceMaxLoc<omp_reduce_ordered, T> maxloc(T val, Index_type idx) const
  {
    int tid = omp_get_thread_num();
    if (val > static_cast<T>(m_blockdata[tid * s_block_offset])) {
      m_blockdata[tid * s_block_offset] = val;
      m_idxdata[tid * s_idx_offset] = idx;
    }

    return *this;
  }

private:
  //
  // Default ctor is declared private and not implemented.
  //
  ReduceMaxLoc<omp_reduce_ordered, T>();

  static const int s_block_offset =
      COHERENCE_BLOCK_SIZE / sizeof(CPUReductionBlockDataType);
  static const int s_idx_offset = COHERENCE_BLOCK_SIZE / sizeof(Index_type);

  bool m_is_copy;
  int m_myID;

  T m_reduced_val;
  Index_type m_reduced_idx;

  CPUReductionBlockDataType* m_blockdata;
  Index_type* m_idxdata;
};

/*!
 ******************************************************************************
 *
 * \brief  Sum reducer class template for use in OpenMP execution.
 *
 *         For usage example, see reducers.hxx.
 *
 ******************************************************************************
 */
template <typename T>
class ReduceSum<omp_reduce_ordered, T>
{
public:
  //
  // Constructor takes default value (default ctor is disabled).
  //
  explicit ReduceSum(T init_val)
  {
    m_is_copy = false;

    m_init_val = init_val;
    m_reduced_val = static_cast<T>(0);

    m_myID = getCPUReductionId();

    m_blockdata = getCPUReductionMemBlock(m_myID);

    int nthreads = omp_get_max_threads();
#pragma omp parallel for schedule(static, 1)
    for (int i = 0; i < nthreads; ++i) {
      m_blockdata[i * s_block_offset] = 0;
    }
  }

  //
  // Copy ctor.
  //
  ReduceSum(const ReduceSum<omp_reduce_ordered, T>& other)
  {
    *this = other;
    m_is_copy = true;
  }

  //
  // Destruction releases the shared memory block chunk for reduction id
  // and id itself for others to use.
  //
  ~ReduceSum<omp_reduce_ordered, T>()
  {
    if (!m_is_copy) {
      releaseCPUReductionId(m_myID);
    }
  }

  //
  // Operator that returns reduced sum value.
  //
  operator T()
  {
    T tmp_reduced_val = static_cast<T>(0);
    int nthreads = omp_get_max_threads();
    for (int i = 0; i < nthreads; ++i) {
      tmp_reduced_val += static_cast<T>(m_blockdata[i * s_block_offset]);
    }
    m_reduced_val = m_init_val + tmp_reduced_val;

    return m_reduced_val;
  }

  //
  // Method that returns sum value.
  //
  T get() { return operator T(); }

  //
  // += operator that adds value to sum for current thread.
  //
  ReduceSum<omp_reduce_ordered, T> operator+=(T val) const
  {
    int tid = omp_get_thread_num();
    m_blockdata[tid * s_block_offset] += val;
    return *this;
  }

private:
  //
  // Default ctor is declared private and not implemented.
  //
  ReduceSum<omp_reduce_ordered, T>();

  static const int s_block_offset =
      COHERENCE_BLOCK_SIZE / sizeof(CPUReductionBlockDataType);

  bool m_is_copy;
  int m_myID;

  T m_init_val;
  T m_reduced_val;

  CPUReductionBlockDataType* m_blockdata;
};

} // closing brace for RAJA namespace

#endif // closing endif for ENABLE_CUDA guard

#endif // closing endif for header file include guard
