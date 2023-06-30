#ifndef PTL_HEADER_FILE_DESCRIPTOR_H_INCLUDED
#define PTL_HEADER_FILE_DESCRIPTOR_H_INCLUDED

#include <ptl/core.h>


#if defined(_WIN32)
    #include <io.h>
#endif
#include <fcntl.h>
#include <sys/stat.h>

namespace ptl::inline v0 {

    namespace impl {
        #if defined(_WIN32)
            inline int pipe(int fildes[2]) {
                return _pipe(fildes, 0, _O_BINARY);
            }
        #else
            using ::pipe;
        #endif

        #if defined(_WIN32) && !defined(__MINGW32__)
            static const auto & open = ::_open;
            static const auto & close = ::_close;
            static const auto & fileno = ::_fileno;
            static const auto & read = ::_read;
            static const auto & dup2 = ::_dup2;
        #else
            using ::open;
            using ::close;
            using ::fileno;
            using ::read;
            using ::dup2;
        #endif
    }

    template<class T> struct FileDescriptorTraits;

    template<class T>
    concept FileDescriptorLike = requires(T && obj) {
        { FileDescriptorTraits<std::remove_cvref_t<T>>::c_fd(std::forward<T>(obj)) } -> std::same_as<int>;
    };

    template<FileDescriptorLike T>
    [[gnu::always_inline]] inline int c_fd(T && obj)
        { return FileDescriptorTraits<std::remove_cvref_t<T>>::c_fd(std::forward<T>(obj)); }

    class FileDescriptor {
    public:
        FileDescriptor() noexcept = default;
        explicit FileDescriptor(int fd) noexcept : m_fd(fd) {
        }
        ~FileDescriptor() noexcept {
            if (m_fd >= 0)
                impl::close(m_fd);
        }
        FileDescriptor(FileDescriptor && src) noexcept : m_fd(src.m_fd) {
            src.m_fd = -1;
        }
        FileDescriptor(const FileDescriptor &) = delete;
        FileDescriptor & operator=(FileDescriptor src) noexcept {
            swap(src, *this);
            return *this;
        }
        
        void close() noexcept {
            *this = FileDescriptor();
        }
        
        static auto open(PathLike auto && path, int oflag, mode_t mode, PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) -> FileDescriptor 
        requires(PTL_ERROR_REQ(err)) {

            clearError(PTL_ERROR_REF(err));
            auto cpath = c_path(std::forward<decltype(path)>(path));
            auto fd = impl::open(cpath, oflag, mode);
            if (fd < 0) {
                fd = -1;
                handleError(PTL_ERROR_REF(err), errno, "cannot open {}", cpath);
            }
            return FileDescriptor(fd);
        }

        static auto open(PathLike auto && path, int oflag, PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) -> FileDescriptor 
        requires(PTL_ERROR_REQ(err)) {
            return open(std::forward<decltype(path)>(path), oflag, 0, PTL_ERROR_REF(err));
        }
        
        #if PTL_HAVE_MKOSTEMPS
        static auto openTemp(char * nameTemplate, size_t suffixLen, int oflags, PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) -> FileDescriptor 
        requires(PTL_ERROR_REQ(err)) {
            clearError(PTL_ERROR_REF(err));
            int fd = mkostemps(nameTemplate, int(suffixLen), oflags);
            if (fd == -1)
                handleError(PTL_ERROR_REF(err), errno, "mkostemps({}, {}) failed", nameTemplate, suffixLen);
            return FileDescriptor(fd);
        }

        static auto openTemp(char * nameTemplate, size_t suffixLen, PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) -> FileDescriptor 
        requires(PTL_ERROR_REQ(err)) {
            return openTemp(nameTemplate, suffixLen, 0, PTL_ERROR_REF(err));
        }

        static auto openTemp(char * nameTemplate, PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) -> FileDescriptor 
        requires(PTL_ERROR_REQ(err)) {
            return openTemp(nameTemplate, 0, 0, PTL_ERROR_REF(err));
        }
        #endif
        
        void dup2(const FileDescriptorLike auto & fdTo) const {
            auto res = impl::dup2(m_fd, c_fd(std::forward<decltype(fdTo)>(fdTo)));
            if (res < 0) 
                throwErrorCode(errno, "dup2({},{}) failed", m_fd, c_fd(std::forward<decltype(fdTo)>(fdTo)));
        }

        auto read(void * buf, io_size_t nbyte, PTL_ERROR_REF_ARG(err)) const noexcept(PTL_ERROR_NOEXCEPT(err)) -> io_ssize_t 
        requires(PTL_ERROR_REQ(err)) {
            clearError(PTL_ERROR_REF(err));
            auto ret = impl::read(m_fd, buf, nbyte);
            if (ret < 0)
                handleError(PTL_ERROR_REF(err), errno, "read() failed");
            return ret;
        }
        
        friend void swap(FileDescriptor & lhs, FileDescriptor & rhs) noexcept {
            std::swap(lhs.m_fd, rhs.m_fd);
        }
        
        auto get() const noexcept -> int {
            return m_fd;
        }
        
        auto detach() noexcept -> int {
            int ret = m_fd;
            m_fd = -1;
            return ret;
        }
        
        explicit operator bool() const noexcept {
            return m_fd != -1;
        }
    private:
        int m_fd = -1;
    };

    template<> struct FileDescriptorTraits<int> {
        [[gnu::always_inline]] static int c_fd(int fd)
            { return fd;}
    };
    template<> struct FileDescriptorTraits<FileDescriptor> {
        [[gnu::always_inline]] static int c_fd(const FileDescriptor & fd)
            { return fd.get();}
    };
    template<> struct FileDescriptorTraits<FILE *> {
        [[gnu::always_inline]] static int c_fd(FILE * fp)
            { return impl::fileno(fp);}
    };

    struct Pipe {
        static auto create(PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err))
        requires(PTL_ERROR_REQ(err)) {
            clearError(PTL_ERROR_REF(err));
            int fds[2];
            if (impl::pipe(fds) != 0) {
                fds[0] = -1;
                fds[1] = -1;
                handleError(PTL_ERROR_REF(err), errno, "pipe() call failed");
            }
            return Pipe{FileDescriptor(fds[0]), FileDescriptor(fds[1])};
        }

        FileDescriptor readEnd;
        FileDescriptor writeEnd;
    };

    #ifndef _WIN32

    inline void changeOwner(FileDescriptorLike auto && desc, uid_t uid, gid_t gid,
                            PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto fd = c_fd(std::forward<decltype(desc)>(desc));
        clearError(PTL_ERROR_REF(err));
        if (::fchown(fd, uid, gid) != 0)
            handleError(PTL_ERROR_REF(err), errno, "fchown({}, {}, {}) failed", fd, uid, gid);
    }

    inline void changeOwner(PathLike auto && path, uid_t uid, gid_t gid,
                            PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto cpath = c_path(std::forward<decltype(path)>(path));
        clearError(PTL_ERROR_REF(err));
        if (::chown(cpath, uid, gid) != 0)
            handleError(PTL_ERROR_REF(err), errno, "chown({}, {}, {}) failed", cpath, uid, gid);
    }

    inline void changeLinkOwner(PathLike auto && path, uid_t uid, gid_t gid,
                            PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto cpath = c_path(std::forward<decltype(path)>(path));
        clearError(PTL_ERROR_REF(err));
        if (::lchown(cpath, uid, gid) != 0)
            handleError(PTL_ERROR_REF(err), errno, "lchown({}, {}, {}) failed", cpath, uid, gid);
    }

    inline void changeMode(FileDescriptorLike auto && desc, mode_t mode,
                           PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto fd = c_fd(std::forward<decltype(desc)>(desc));
        clearError(PTL_ERROR_REF(err));
        if (::fchmod(fd, mode) != 0)
            handleError(PTL_ERROR_REF(err), errno, "fchmod({}, 0{:o}) failed", fd, mode);
    }

    inline void changeMode(PathLike auto && path, mode_t mode,
                           PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto cpath = c_path(std::forward<decltype(path)>(path));
        clearError(PTL_ERROR_REF(err));
        if (::chmod(cpath, mode) != 0)
            handleError(PTL_ERROR_REF(err), errno, "chmod({}, 0{:o}) failed", cpath, mode);
    }

    #if PTL_HAVE_LCHMOD
    inline void changeLinkMode(PathLike auto && path, mode_t mode,
                               PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto cpath = c_path(std::forward<decltype(path)>(path));
        clearError(PTL_ERROR_REF(err));
        if (::lchmod(cpath, mode) != 0)
            handleError(PTL_ERROR_REF(err), errno, "chmod({}, 0{:o}) failed", cpath, mode);
    }
    #endif

    inline void getStatus(FileDescriptorLike auto && desc, struct ::stat & res,
                          PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto fd = c_fd(std::forward<decltype(desc)>(desc));
        clearError(PTL_ERROR_REF(err));
        if (::fstat(fd, &res) != 0)
            handleError(PTL_ERROR_REF(err), errno, "fstat({}) failed", fd);
    }

    inline void getStatus(PathLike auto && path, struct ::stat & res,
                          PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto cpath = c_path(std::forward<decltype(path)>(path));
        clearError(PTL_ERROR_REF(err));
        if (::stat(cpath, &res) != 0)
            handleError(PTL_ERROR_REF(err), errno, "stat({}) failed", cpath);
    }

    inline void getLinkStatus(PathLike auto && path, struct ::stat & res,
                              PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto cpath = c_path(std::forward<decltype(path)>(path));
        clearError(PTL_ERROR_REF(err));
        if (::lstat(cpath, &res) != 0)
            handleError(PTL_ERROR_REF(err), errno, "lstat({}) failed", cpath);
    }

    inline void makeDirectory(PathLike auto && path, mode_t mode,
                              PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto cpath = c_path(std::forward<decltype(path)>(path));
        clearError(PTL_ERROR_REF(err));
        if (mkdir(cpath, mode) != 0)
            handleError(PTL_ERROR_REF(err), errno, "mkdir({}, 0{:o}) failed", cpath, mode);
    }

    inline void makeDirectoryAt(FileDescriptorLike auto && desc, PathLike auto && path, mode_t mode,
                              PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto fd = c_fd(std::forward<decltype(desc)>(desc));
        auto cpath = c_path(std::forward<decltype(path)>(path));
        clearError(PTL_ERROR_REF(err));
        if (mkdirat(fd, cpath, mode) != 0)
            handleError(PTL_ERROR_REF(err), errno, "mkdirat({}, {}, 0{:o}) failed", fd, cpath, mode);
    }

    #endif
}

#endif