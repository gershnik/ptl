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

}


#endif
