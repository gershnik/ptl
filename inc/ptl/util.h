// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PTL_HEADER_UTIL_H_INCLUDED
#define PTL_HEADER_UTIL_H_INCLUDED

#include <ptl/core.h>

namespace ptl::inline v0 {

    template<class X, class... Allowed>
    struct TypeInPackImpl {
        static constexpr bool value = (std::is_same_v<X, Allowed> || ...);
    };

    template<class X>
    struct TypeInPackImpl<X> {
        static constexpr bool value = false;
    };

    template<class X, class... Allowed>
    constexpr bool TypeInPack = TypeInPackImpl<X, Allowed...>::value;

    namespace impl {
        template<size_t I, class R, class... Args>
        auto detectArgType(R (*)(Args...)) -> std::tuple_element_t<I, std::tuple<Args...>>;
    }

    #define PTL_DETECT_ARG_TYPE(I, f)  decltype(impl::detectArgType<I>(f))


    template<class T1, class T2>
    requires(std::is_integral_v<T1> && std::is_integral_v<T2>)
    constexpr bool IsNumericallyBigger = (sizeof(T1) > sizeof(T2)) || (
                                            sizeof(T1) == sizeof(T2) &&
                                            std::is_unsigned_v<T1> && std::is_signed_v<T2>);

}


#endif
