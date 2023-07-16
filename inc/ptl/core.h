// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PTL_HEADER_CORE_H_INCLUDED
#define PTL_HEADER_CORE_H_INCLUDED

#include <system_error>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <type_traits>
#include <iterator>


#if __cpp_lib_format >= 202110L
    #include <format>
    #define PTL_USE_STD_FORMAT 1
#else
    #include <fmt/format.h>
#endif

#if __cpp_lib_concepts >=  202002L
    #include <concepts>
    #define PTL_USE_STD_CONCEPTS 1
#endif

#include <errno.h>

#if __has_include(<unistd.h>)
    #include <unistd.h>
#endif

#ifndef PTL_NO_CONFIG
    #include <ptl/config.h>
#endif

namespace ptl::inline v0 {

    #if PTL_USE_STD_CONCEPTS

        template <class From, class To>
        concept SameAs = std::same_as<From, To>;

        template <class From, class To>
        concept ConvertibleTo = std::convertible_to<From, To>;

        template < class T >
        concept UnsignedIntegral = std::unsigned_integral<T>;

        template<class I>
        concept ContiguousIterator = std::contiguous_iterator<I>;

    #else

        template< class T, class U >
        concept SameAs = std::is_same_v<T, U> && std::is_same_v<U, T>;

        template <class From, class To>
        concept ConvertibleTo = std::is_convertible_v<From, To> &&
        requires {
            static_cast<To>(std::declval<From>());
        };

        template < class T >
        concept UnsignedIntegral = std::is_integral_v<T> && !std::is_signed_v<T>;

        template<class I>
        concept ContiguousIterator = std::is_convertible_v<typename std::iterator_traits<I>::iterator_category, std::random_access_iterator_tag> &&
        std::is_lvalue_reference_v<typename std::iterator_traits<I>::reference> &&
        SameAs<
        typename std::iterator_traits<I>::value_type, std::remove_cvref_t<typename std::iterator_traits<I>::reference>
        > &&
        requires(const I& i) {
        { std::to_address(i) } ->
            SameAs<std::add_pointer_t<typename std::iterator_traits<I>::reference>>;
        };

    #endif

    

    #if defined(_WIN32) && !defined(__MINGW32__)
        using mode_t = int;
        using io_size_t = unsigned;
        using io_ssize_t = int;
    #else
        using mode_t = ::mode_t;
        using io_size_t = ::size_t;
        using io_ssize_t = ::ssize_t;
    #endif

    #ifndef _WIN32
        using SystemError = int;
    #else
        struct SystemError { int value; };
    #endif

    namespace impl {

        #if PTL_USE_STD_FORMAT
            using ::std::vformat;
            using ::std::make_format_args;
        #else
            using ::fmt::vformat;
            using ::fmt::make_format_args;
        #endif
    }

    template<class T> struct ErrorTraits;
    
    template<class T>
    concept ErrorSink = requires(T & obj) {
        { ErrorTraits<T>::assignError(obj, int(), "") } noexcept -> SameAs<void>;
        #ifdef _WIN32
        { ErrorTraits<T>::assignError(obj, SystemError{}, "") } noexcept -> SameAs<void>;
        #endif
        { ErrorTraits<T>::clearError(obj) } noexcept -> SameAs<void>;
        { ErrorTraits<T>::failed(obj) } noexcept -> SameAs<bool>;
    };

    template<ErrorSink Err, class... T>
    [[gnu::always_inline]] inline void handleError(Err & err, int code, const char * format, T && ...args) noexcept {
        ErrorTraits<Err>::assignError(err, code, format, std::forward<T>(args)...);
    }

    #ifdef _WIN32

    template<ErrorSink Err, class... T>
    [[gnu::always_inline]] inline void handleError(Err & err, SystemError code, const char * format, T && ...args) noexcept {
        ErrorTraits<Err>::assignError(err, code, format, std::forward<T>(args)...);
    }

    #endif

    template<ErrorSink Err>
    [[gnu::always_inline]] inline void clearError(Err & err) noexcept {
        ErrorTraits<Err>::clearError(err);
    }

    template<ErrorSink Err>
    [[gnu::always_inline]] inline auto failed(Err & err) noexcept -> bool {
        return ErrorTraits<Err>::failed(err);
    }


    [[gnu::always_inline]] inline auto makeErrorCode(int code) -> std::error_code {
    #ifndef _WIN32
        return std::error_code(code, std::system_category());
    #else
        return std::error_code(code, std::generic_category());
    #endif
    }

    template<class... T>
    [[noreturn, gnu::always_inline]] inline void throwErrorCode(int code, const char * format, T && ...args) noexcept(false) {

        if (code == ENOMEM) //do not attempt to allocate on ENOMEM
            throw std::system_error(makeErrorCode(code));
        throw std::system_error(makeErrorCode(code),
                                impl::vformat(format, impl::make_format_args(std::forward<T>(args)...)));
    }

    template<class... T>
    [[noreturn, gnu::always_inline]] inline void throwErrorCode(std::error_code ec, const char * format, T && ...args) noexcept(false) {

        if (ec.value() == ENOMEM) //do not attempt to allocate on ENOMEM
            throw std::system_error(ec);
        throw std::system_error(ec,
                                impl::vformat(format, impl::make_format_args(std::forward<T>(args)...)));
    }

    template<> struct ErrorTraits<std::error_code> {
        template<class... T>
        [[gnu::always_inline]] static inline void assignError(std::error_code & err, int code, const char *, T && ...) noexcept {
            err = makeErrorCode(code);
        }

        #ifdef _WIN32

        template<class... T>
        [[gnu::always_inline]] static inline void assignError(std::error_code & err, SystemError code, const char *, T && ...) noexcept {
            err = std::error_code(code.value, std::system_category());
        }

        #endif

        [[gnu::always_inline]] static inline void clearError(std::error_code & err) noexcept {
            err.clear();
        }

        [[gnu::always_inline]] static inline auto failed(std::error_code & err) noexcept -> bool {
            return bool(err);
        }
    };

    template<class... T>
    [[noreturn, gnu::always_inline]] inline void handleError(int code, const char * format, T && ...args) noexcept(false) {
        throwErrorCode(code, format, std::forward<T>(args)...);
    }

    #ifdef _WIN32

    template<class... T>
    [[noreturn, gnu::always_inline]] inline void handleError(SystemError code, const char * format, T && ...args) noexcept(false) {
        throwErrorCode(std::error_code(code.value, std::system_category()), format, std::forward<T>(args)...);
    }

    #endif

    [[gnu::always_inline]] constexpr inline void clearError() noexcept 
    {}

    [[gnu::always_inline]] constexpr inline auto failed() noexcept -> bool
        { return false; }

    template<class... T>
    [[gnu::always_inline]] inline void posixCheck(int retval, const char * format, T && ...args) noexcept(false) {
        if (retval != 0)
            throwErrorCode(errno, format, std::forward<T>(args)...);
    }

    #define PTL_ERROR_REF_ARG(x) ErrorSink auto & ...x
    #define PTL_ERROR_REQ(x) (sizeof...(x) < 2)
    #define PTL_ERROR_PRESENT(x) (sizeof...(x) == 1)
    #define PTL_ERROR_REF(x) x...

    template<class T> struct CPathTraits;

    template<class T>
    concept PathLike = requires(T && obj) {
        { CPathTraits<std::remove_cvref_t<T>>::c_path(std::forward<T>(obj)) } -> ConvertibleTo<const char *>;
    };

    template<PathLike T>
    inline decltype(auto) c_path(T && obj)
        { return CPathTraits<std::remove_cvref_t<T>>::c_path(std::forward<T>(obj)); }

    template<> struct CPathTraits<const char *> {
        static const char * c_path(const char * str)
            { return str; }
    };
    template<size_t N> struct CPathTraits<const char [N]> {
        static const char * c_path(const char * str)
            { return str; }
    };
    template<> struct CPathTraits<char *> {
        static const char * c_path(char * str)
            { return str; }
    };
    template<size_t N> struct CPathTraits<char [N]> {
        static const char * c_path(const char * str)
            { return str; }
    };
    template<> struct CPathTraits<std::string> {
        static const char * c_path(const std::string & str)
            { return str.c_str(); }
    };
    template<> struct CPathTraits<std::filesystem::path> {
        
        template<class FSChar=std::filesystem::path::value_type>
        static decltype(auto) c_path(const std::filesystem::path & path) { 
            if constexpr (std::is_same_v<FSChar, char>) {
                return CPathTraits::dependent_c_str(path); 
            } else {
                return Proxy{path.string()};
            }
        }
    private:
        struct Proxy {
            operator const char * () const noexcept 
                { return path.c_str(); }
            std::string path; 
        };

        template<class P>
        static auto dependent_c_str(P && path) noexcept -> const char *
            { return std::forward<P>(path).c_str(); }

    };
    
    template<class T>
    concept StringLike = ConvertibleTo<T, const char *> || requires(T && obj) {
        { std::forward<T>(obj).c_str() } -> ConvertibleTo<const char *>;
    };

    inline auto c_str(StringLike auto && str) noexcept -> const char * {
        using Str = std::remove_cvref_t<decltype(str)>;
        if constexpr (std::is_convertible_v<Str, const char *>) {
            return str;
        } else {
            return std::forward<decltype(str)>(str).c_str();
        }
    }

    template<class T>
    concept StringLikeArray = requires(T obj) {
        std::begin(obj);
        std::end(obj);
        { std::size(obj) } -> SameAs<size_t>;   
        requires StringLike<decltype(*std::begin(obj))>;
    };

    class StringRefArray {
    public:
        template<StringLikeArray T>
        StringRefArray(T && array) {
            const auto arraySize = std::size(std::forward<T>(array));
            auto arrayBegin = std::begin(std::forward<T>(array));

            if constexpr (std::is_same_v<std::remove_cvref_t<decltype(*arrayBegin)>, const char *> &&
                          ContiguousIterator<decltype(arrayBegin)>) {
                if (arraySize > 0 && *(arrayBegin + arraySize - 1) == 0) {
                    m_ptr = &*arrayBegin;
                    return;
                }
            }

            transform(std::forward<T>(array), arraySize);
        }

        template<StringLike Str>
        StringRefArray(std::initializer_list<Str> array) {
            if constexpr (std::is_same_v<Str, const char *>) {
                if (array.size() > 0 && array.end()[-1] == 0) {
                    m_ptr = array.begin();
                    return;
                }
            }

            transform(array, array.size());
        }

        StringRefArray(const StringRefArray &) = delete;
        StringRefArray & operator=(const StringRefArray &) = delete;

        StringRefArray(StringRefArray &&) = default;
        StringRefArray & operator=(StringRefArray &&) = default;

        auto data() const noexcept -> const char * const * 
            { return m_ptr; }

        auto operator[](size_t idx) const noexcept -> const char * 
            { return m_ptr[idx]; }

        auto empty() const noexcept -> bool 
            { return *m_ptr == 0; }

    private:
        void transform(StringLikeArray auto && array, UnsignedIntegral auto size) {
            using ArrayType = std::remove_cvref_t<decltype(array)>;

            m_transformed.reset(new const char *[size + 1]);
            if constexpr (std::is_convertible_v<decltype(*std::begin(std::forward<ArrayType>(array))), const char *>) {
                std::copy(std::begin(std::forward<ArrayType>(array)), std::end(std::forward<ArrayType>(array)), m_transformed.get());
            } else {
                std::transform(std::begin(std::forward<ArrayType>(array)), std::end(std::forward<ArrayType>(array)), m_transformed.get(),
                [](const auto & str) {
                    return str.c_str();
                });
            }
            m_transformed[size] = 0;
            m_ptr = m_transformed.get();
        }
    private:
        const char * const * m_ptr = nullptr;
        std::unique_ptr<const char * []> m_transformed;
    };
}

#endif