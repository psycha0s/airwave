#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>
#include <type_traits>

// Remove stupid definitions from stdbool.h
#undef bool
#undef true
#undef false

#define UNUSED(x) ((void) x)

// Basic data types
using schar     = signed char;
using uchar     = unsigned char;
using ushort    = unsigned short;
using uint      = unsigned int;
using ulong     = unsigned long;
using longlong  = long long;
using ulonglong = unsigned long long;

using ch16 = char16_t;
using ch32 = char32_t;
using i8   = int8_t;
using u8   = uint8_t;
using i16  = int16_t;
using u16  = uint16_t;
using i32  = int32_t;
using u32  = uint32_t;
using i64  = int64_t;
using u64  = uint64_t;
using f32  = float;
using f64  = double;


template<typename T>
using Box = std::unique_ptr<T>;

template<typename T>
using Rc = std::shared_ptr<T>;

template<typename T>
using Ref = std::weak_ptr<T>;

template<typename First, typename Second>
using Pair = std::pair<First, Second>;

template<typename... Args>
using Tuple = std::tuple<Args...>;


template<typename T, typename... Args>
constexpr Box<T> makeBox(Args&&... args)
{
	return Box<T>(new T(std::forward<Args>(args)...));
}

template<typename T, typename ...Args>
constexpr Rc<T> makeRc(Args&&... args)
{
	return Rc<T>(new T(std::forward<Args>(args)...));
}


template<typename First, typename Second>
constexpr Pair<typename std::decay<First>::type, typename std::decay<Second>::type>
makePair(First&& first, Second&& second)
{
	using PairType = Pair<First, Second>;
	return PairType(std::forward<First>(first), std::forward<Second>(second));
}


template<typename ...Args>
constexpr Tuple<typename std::decay<Args>::type...> makeTuple(Args&&... args)
{
	return std::make_tuple<Args...>(std::forward<Args>(args)...);
}


#endif // COMMON_TYPES_H
