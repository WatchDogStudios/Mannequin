#pragma once
#include <type_traits>

using std::is_arithmetic;
using std::is_same;
using std::remove_cv;
using std::remove_reference;
using std::decay;
using std::enable_if;
using std::conditional;
using std::integral_constant;
using std::true_type;
using std::false_type;

#if __cplusplus >= 201402L
    template<typename T, typename U>
    constexpr bool is_same_v = std::is_same<T, U>::value;
    template<typename T>
    constexpr bool is_arithmetic_v = std::is_arithmetic<T>::value;
#else
    // For C++11/older
#endif 