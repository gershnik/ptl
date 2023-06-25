#ifndef PTL_HEADER_CORE_H_INCLUDED
#define PTL_HEADER_CORE_H_INCLUDED

#include <system_error>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <type_traits>

#if __cpp_lib_format >= 202110L
    #include <format>
    #define PTL_USE_STD_FORMAT 1
#else
    #include <fmt/format.h>
#endif

#include <errno.h>

#if __has_include(<unistd.h>)
    #include <unistd.h>
#endif

#include <ptl/config.h>

namespace ptl::inline v0 {

    #if defined(_WIN32) && !defined(__MINGW32__)
        using mode_t = int;
        using io_size_t = unsigned;
        using io_ssize_t = int;
    #else
        using mode_t = ::mode_t;
        using io_size_t = ::size_t;
        using io_ssize_t = ::ssize_t;
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
    concept ErrorSink = requires(T & obj, int code) {
        { ErrorTraits<T>::handleError(obj, code, "") } -> std::same_as<void>;
        { ErrorTraits<T>::clearError(obj) } -> std::same_as<void>;
    };

    template<ErrorSink Err, class... T>
    [[gnu::always_inline]] inline void handleError(Err & err, int code, const char * format, T && ...args) noexcept {
        ErrorTraits<Err>::handleError(err, code, format, std::forward<T>(args)...);
    }

    template<ErrorSink Err>
    [[gnu::always_inline]] inline void clearError(Err & err) noexcept {
        ErrorTraits<Err>::clearError(err);
    }


    [[gnu::always_inline]] inline auto makeErrorCode(int code) -> std::error_code {
        return std::make_error_code(static_cast<std::errc>(code));
    }

    template<class... T>
    [[noreturn, gnu::always_inline]] inline void throwErrorCode(int code, const char * format, T && ...args) noexcept(false) {

        if (code == ENOMEM) //do not attempt to allocate on ENOMEM
            throw std::system_error(makeErrorCode(code));
        throw std::system_error(makeErrorCode(code),
                                impl::vformat(format, impl::make_format_args(std::forward<T>(args)...)));
    }

    template<> struct ErrorTraits<std::error_code> {
        template<class... T>
        [[gnu::always_inline]] static inline void handleError(std::error_code & err, int code, const char *, T && ...) noexcept {
            err = makeErrorCode(code);
        }

        [[gnu::always_inline]] static inline void clearError(std::error_code & err) noexcept {
            err.clear();
        }
    };

    template<class... T>
    [[noreturn, gnu::always_inline]] inline void handleError(int code, const char * format, T && ...args) noexcept(false) {
        throwErrorCode(code, format, std::forward<T>(args)...);
    }

    [[gnu::always_inline]] inline void clearError() noexcept 
    {}

    template<class... T>
    [[gnu::always_inline]] inline void posixCheck(int retval, const char * format, T && ...args) noexcept(false) {
        if (retval != 0)
            throwErrorCode(errno, format, std::forward<T>(args)...);
    }

    #define PTL_ERROR_REF_ARG(x) ErrorSink auto & ...x
    #define PTL_ERROR_REQ(x) (sizeof...(x) < 2)
    #define PTL_ERROR_NOEXCEPT(x) (sizeof...(x) > 0)
    #define PTL_ERROR_REF(x) x...

    template<class T> struct CPathTraits;

    template<class T>
    concept PathLike = requires(T && obj) {
        { CPathTraits<std::remove_cvref_t<T>>::c_path(std::forward<T>(obj)) } -> std::convertible_to<const char *>;
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
    concept StringLike = std::convertible_to<T, const char *> || requires(T && obj) {
        { std::forward<T>(obj).c_str() } -> std::convertible_to<const char *>;
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
        { std::size(obj) } -> std::same_as<size_t>;   
        requires StringLike<decltype(*std::begin(obj))>;
    };
}

#endif