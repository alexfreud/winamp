/*
 * mptBaseMacros.h
 * ---------------
 * Purpose: Basic assorted compiler-related helpers.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/preprocessor.hpp"
#include "mpt/base/compiletime_warning.hpp"
#include "mpt/base/macros.hpp"

#if MPT_CXX_AT_LEAST(20)
#include <version>
#else  // !C++20
#include <array>
#endif  // C++20

#include <array>
#include <iterator>
#include <type_traits>

#include <cstddef>
#include <cstdint>

#include <stddef.h>
#include <stdint.h>


OPENMPT_NAMESPACE_BEGIN



#define MPT_UNREFERENCED_PARAMETER(x) MPT_UNUSED(x)
#define MPT_UNUSED_VARIABLE(x)        MPT_UNUSED(x)
// C++17 std::size
#if MPT_CXX_AT_LEAST(17)
namespace mpt
{
using std::size;
}  // namespace mpt
#else
namespace mpt
{
template <typename T>
MPT_CONSTEXPR11_FUN auto size(const T &v) -> decltype(v.size())
{
	return v.size();
}
template <typename T, std::size_t N>
MPT_CONSTEXPR11_FUN std::size_t size(const T (&)[N]) noexcept
{
	return N;
}
}  // namespace mpt
#endif
// legacy
#if MPT_COMPILER_MSVC
OPENMPT_NAMESPACE_END
#include <cstdlib>
#include <stdlib.h>
OPENMPT_NAMESPACE_BEGIN
#define MPT_ARRAY_COUNT(x) _countof(x)
#else
#define MPT_ARRAY_COUNT(x) (sizeof((x)) / sizeof((x)[0]))
#endif
#define CountOf(x) MPT_ARRAY_COUNT(x)



#if MPT_COMPILER_MSVC
// warning LNK4221: no public symbols found; archive member will be inaccessible
// There is no way to selectively disable linker warnings.
// #pragma warning does not apply and a command line option does not exist.
// Some options:
//  1. Macro which generates a variable with a unique name for each file (which means we have to pass the filename to the macro)
//  2. unnamed namespace containing any symbol (does not work for c++11 compilers because they actually have internal linkage now)
//  3. An unused trivial inline function.
// Option 3 does not actually solve the problem though, which leaves us with option 1.
// In any case, for optimized builds, the linker will just remove the useless symbol.
#define MPT_MSVC_WORKAROUND_LNK4221_CONCAT_DETAIL(x, y) x##y
#define MPT_MSVC_WORKAROUND_LNK4221_CONCAT(x, y)        MPT_MSVC_WORKAROUND_LNK4221_CONCAT_DETAIL(x, y)
#define MPT_MSVC_WORKAROUND_LNK4221(x)                  int MPT_MSVC_WORKAROUND_LNK4221_CONCAT(mpt_msvc_workaround_lnk4221_, x) = 0;
#endif

#ifndef MPT_MSVC_WORKAROUND_LNK4221
#define MPT_MSVC_WORKAROUND_LNK4221(x)
#endif



OPENMPT_NAMESPACE_END
