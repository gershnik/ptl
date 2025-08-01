// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PTL_HEADER_FILE_H_INCLUDED
#define PTL_HEADER_FILE_H_INCLUDED

#include <ptl/core.h>


#if defined(_WIN32)
    #include <io.h>
#endif
#include <fcntl.h>
#include <sys/stat.h>
#if __has_include(<sys/file.h>)
    #include <sys/file.h>
#endif
#if __has_include(<sys/mman.h>)
    #include <sys/mman.h>
#endif

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
            static const auto & write = ::_write;
            static const auto & dup = ::_dup;
            static const auto & dup2 = ::_dup2;
        #else
            using ::open;
            using ::close;
            #ifndef fileno
                using ::fileno;
            #endif
            using ::read;
            using ::write;
            using ::dup;
            using ::dup2;
        #endif
    }

    template<class T> struct FileDescriptorTraits;

    template<class T>
    concept FileDescriptorLike = requires(T && obj) {
        { FileDescriptorTraits<std::remove_cvref_t<T>>::c_fd(std::forward<T>(obj)) } -> SameAs<int>;
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
        
        static auto open(PathLike auto && path, int oflag, mode_t mode, PTL_ERROR_REF_ARG(err)) -> FileDescriptor 
        requires(PTL_ERROR_REQ(err)) {

            auto cpath = c_path(std::forward<decltype(path)>(path));
            auto fd = impl::open(cpath, oflag, mode);
            if (fd < 0) {
                fd = -1;
                handleError(PTL_ERROR_REF(err), errno, "cannot open {}", cpath);
            } else {
                clearError(PTL_ERROR_REF(err));
            }
            return FileDescriptor(fd);
        }

        static auto open(PathLike auto && path, int oflag, PTL_ERROR_REF_ARG(err)) -> FileDescriptor 
        requires(PTL_ERROR_REQ(err)) {
            return open(std::forward<decltype(path)>(path), oflag, 0, PTL_ERROR_REF(err));
        }
        
        #if PTL_HAVE_MKOSTEMPS
        static auto openTemp(char * nameTemplate, size_t suffixLen, int oflags, PTL_ERROR_REF_ARG(err)) -> FileDescriptor 
        requires(PTL_ERROR_REQ(err)) {
            int fd = mkostemps(nameTemplate, int(suffixLen), oflags);
            if (fd == -1)
                handleError(PTL_ERROR_REF(err), errno, "mkostemps({}, {}) failed", nameTemplate, suffixLen);
            else
                clearError(PTL_ERROR_REF(err));
            return FileDescriptor(fd);
        }

        static auto openTemp(char * nameTemplate, size_t suffixLen, PTL_ERROR_REF_ARG(err)) -> FileDescriptor 
        requires(PTL_ERROR_REQ(err)) {
            return openTemp(nameTemplate, suffixLen, 0, PTL_ERROR_REF(err));
        }

        static auto openTemp(char * nameTemplate, PTL_ERROR_REF_ARG(err)) -> FileDescriptor 
        requires(PTL_ERROR_REQ(err)) {
            return openTemp(nameTemplate, 0, 0, PTL_ERROR_REF(err));
        }
        #endif

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
        [[gnu::always_inline]] static int c_fd(int fd) noexcept
            { return fd;}
    };
    template<> struct FileDescriptorTraits<FileDescriptor> {
        [[gnu::always_inline]] static int c_fd(const FileDescriptor & fd) noexcept
            { return fd.get();}
    };
    template<> struct FileDescriptorTraits<FILE *> {
        [[gnu::always_inline]] static int c_fd(FILE * fp) noexcept { 
            #ifdef fileno
                return fileno(fp);
            #else
                return impl::fileno(fp);
            #endif
        }
    };

    struct Pipe {
        static auto create(PTL_ERROR_REF_ARG(err)) 
        requires(PTL_ERROR_REQ(err)) {
            int fds[2];
            if (impl::pipe(fds) != 0) {
                fds[0] = -1;
                fds[1] = -1;
                handleError(PTL_ERROR_REF(err), errno, "pipe() call failed");
            } else {
                clearError(PTL_ERROR_REF(err));
            }
            return Pipe{FileDescriptor(fds[0]), FileDescriptor(fds[1])};
        }

        FileDescriptor readEnd;
        FileDescriptor writeEnd;
    };

    #if !defined(_WIN32)
    class MemoryMap {
    public:
        MemoryMap() noexcept = default;

        MemoryMap(void * addr, size_t length, 
                  int prot, int flags,
                  FileDescriptorLike auto && desc,
                  off_t offset,
                  PTL_ERROR_REF_ARG(err)) requires(PTL_ERROR_REQ(err)) :
            m_ptr(::mmap(addr, length, prot, flags, c_fd(std::forward<decltype(desc)>(desc)), offset)) 
        {

            if (m_ptr == MAP_FAILED) {
                handleError(PTL_ERROR_REF(err), errno, "mmap({}, {}, {}, {}, {}, {}) failed", 
                            addr, length, prot, flags, c_fd(std::forward<decltype(desc)>(desc)), offset);
            } else {
                m_size = length;
                clearError(PTL_ERROR_REF(err));
            }
        }
        MemoryMap(void * addr, size_t length, 
                  int prot, int flags,
                  FileDescriptorLike auto && desc,
                  PTL_ERROR_REF_ARG(err)) requires(PTL_ERROR_REQ(err)):
            MemoryMap(addr, length, prot, flags, std::forward<decltype(desc)>(desc), 0, PTL_ERROR_REF(err))
        {}
        MemoryMap(size_t length, 
                  int prot, int flags,
                  FileDescriptorLike auto && desc,
                  off_t offset,
                  PTL_ERROR_REF_ARG(err)) requires(PTL_ERROR_REQ(err)):
            MemoryMap(nullptr, length, prot, flags, std::forward<decltype(desc)>(desc), offset, PTL_ERROR_REF(err))
        {}
        MemoryMap(size_t length, 
                  int prot, int flags,
                  FileDescriptorLike auto && desc,
                  PTL_ERROR_REF_ARG(err)) requires(PTL_ERROR_REQ(err)):
            MemoryMap(nullptr, length, prot, flags, std::forward<decltype(desc)>(desc), 0, PTL_ERROR_REF(err))
        {}

        ~MemoryMap() noexcept {
            if (m_ptr != MAP_FAILED)
                munmap(m_ptr, m_size);
        }
        MemoryMap(const MemoryMap &) = delete;
        
        MemoryMap(MemoryMap && src) :
            m_ptr(std::exchange(src.m_ptr, MAP_FAILED)),
            m_size(std::exchange(src.m_size, 0))
        {}

        MemoryMap & operator=(MemoryMap src) noexcept {
            swap(src, *this);
            return *this;
        }

        explicit operator bool() const noexcept {
            return m_ptr != MAP_FAILED;
        }

        void close() noexcept {
            *this = MemoryMap();
        }

        friend void swap(MemoryMap & lhs, MemoryMap & rhs) noexcept {
            std::swap(lhs.m_ptr, rhs.m_ptr);
            std::swap(lhs.m_size, rhs.m_size);
        }

        auto data() const noexcept -> void *
            { return m_ptr; }
        auto size() const noexcept -> size_t
            { return m_size; }
    private:
        void * m_ptr = MAP_FAILED;
        size_t m_size = 0;
    };
    #endif

    inline auto duplicate(FileDescriptorLike auto && desc) -> FileDescriptor {
        auto fd = c_fd(std::forward<decltype(desc)>(desc));
        auto ret = impl::dup(fd);
        if (ret < 0)
            throwErrorCode(errno, "dup({}) failed", fd);
        return FileDescriptor(ret);
    }
    
    inline void duplicateTo(FileDescriptorLike auto && from, FileDescriptorLike auto && to) {
        auto fdFrom = c_fd(std::forward<decltype(from)>(from));
        auto fdTo = c_fd(std::forward<decltype(to)>(to));
        auto res = impl::dup2(fdFrom, fdTo);
        if (res < 0) 
            throwErrorCode(errno, "dup2({},{}) failed", fdFrom, fdTo);
    }

    inline auto readFile(FileDescriptorLike auto && desc, void * buf, io_size_t nbyte, 
                         PTL_ERROR_REF_ARG(err)) -> io_ssize_t 
    requires(PTL_ERROR_REQ(err)) {
        auto fd = c_fd(std::forward<decltype(desc)>(desc));
        auto ret = impl::read(fd, buf, nbyte);
        if (ret < 0)
            handleError(PTL_ERROR_REF(err), errno, "read({}, ,{}) failed", fd, nbyte);
        else
            clearError(PTL_ERROR_REF(err));
        return ret;
    }

    inline auto writeFile(FileDescriptorLike auto && desc, const void * buf, io_size_t nbyte, 
                          PTL_ERROR_REF_ARG(err)) -> io_ssize_t 
    requires(PTL_ERROR_REQ(err)) {
        auto fd = c_fd(std::forward<decltype(desc)>(desc));
        auto ret = impl::write(fd, buf, nbyte);
        if (ret < 0)
            handleError(PTL_ERROR_REF(err), errno, "write({}, ,{}) failed", fd, nbyte);
        else
            clearError(PTL_ERROR_REF(err));
        return ret;
    }

    #ifndef _WIN32

    enum class FileLock : int {
        Shared = LOCK_SH,
        Exclusive = LOCK_EX
    };

    inline void lockFile(FileDescriptorLike auto && desc, FileLock type,
                         PTL_ERROR_REF_ARG(err))
    requires(PTL_ERROR_REQ(err)) {
        auto fd = c_fd(std::forward<decltype(desc)>(desc));
        int operation = int(type);
        if (::flock(fd, operation) != 0)
            handleError(PTL_ERROR_REF(err), errno, "flock({}, ,{}) failed", fd, operation);
        else
            clearError(PTL_ERROR_REF(err));
    }

    inline auto tryLockFile(FileDescriptorLike auto && desc, FileLock type,
                            PTL_ERROR_REF_ARG(err)) -> bool
    requires(PTL_ERROR_REQ(err)) {
        auto fd = c_fd(std::forward<decltype(desc)>(desc));
        int operation = int(type) | LOCK_NB;
        clearError(PTL_ERROR_REF(err));
        if (::flock(fd, operation) == 0) 
            return true;
        if (int code = errno; code != EWOULDBLOCK)
            handleError(PTL_ERROR_REF(err), code, "flock({}, ,{}) failed", fd, operation);
        return false;
    }

    inline void unlockFile(FileDescriptorLike auto && desc,
                           PTL_ERROR_REF_ARG(err))
    requires(PTL_ERROR_REQ(err)) {
        auto fd = c_fd(std::forward<decltype(desc)>(desc));
        if (::flock(fd, LOCK_UN) != 0)
            handleError(PTL_ERROR_REF(err), errno, "flock({}, ,{}) failed", fd, LOCK_UN);
        else
            clearError(PTL_ERROR_REF(err));
    }

    inline void changeOwner(FileDescriptorLike auto && desc, uid_t uid, gid_t gid,
                            PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto fd = c_fd(std::forward<decltype(desc)>(desc));
        if (::fchown(fd, uid, gid) != 0)
            handleError(PTL_ERROR_REF(err), errno, "fchown({}, {}, {}) failed", fd, uid, gid);
        else 
            clearError(PTL_ERROR_REF(err));
    }

    inline void changeOwner(PathLike auto && path, uid_t uid, gid_t gid,
                            PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto cpath = c_path(std::forward<decltype(path)>(path));
        if (::chown(cpath, uid, gid) != 0)
            handleError(PTL_ERROR_REF(err), errno, "chown({}, {}, {}) failed", cpath, uid, gid);
        else
            clearError(PTL_ERROR_REF(err));
    }

    inline void changeLinkOwner(PathLike auto && path, uid_t uid, gid_t gid,
                            PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto cpath = c_path(std::forward<decltype(path)>(path));
        if (::lchown(cpath, uid, gid) != 0)
            handleError(PTL_ERROR_REF(err), errno, "lchown({}, {}, {}) failed", cpath, uid, gid);
        else
            clearError(PTL_ERROR_REF(err));
    }

    inline void changeMode(FileDescriptorLike auto && desc, mode_t mode,
                           PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto fd = c_fd(std::forward<decltype(desc)>(desc));
        if (::fchmod(fd, mode) != 0)
            handleError(PTL_ERROR_REF(err), errno, "fchmod({}, 0{:o}) failed", fd, mode);
        else
            clearError(PTL_ERROR_REF(err));
    }

    inline void changeMode(PathLike auto && path, mode_t mode,
                           PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto cpath = c_path(std::forward<decltype(path)>(path));
        if (::chmod(cpath, mode) != 0)
            handleError(PTL_ERROR_REF(err), errno, "chmod({}, 0{:o}) failed", cpath, mode);
        else
            clearError(PTL_ERROR_REF(err));
    }

    #if PTL_HAVE_LCHMOD
    inline void changeLinkMode(PathLike auto && path, mode_t mode,
                               PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto cpath = c_path(std::forward<decltype(path)>(path));
        if (::lchmod(cpath, mode) != 0)
            handleError(PTL_ERROR_REF(err), errno, "chmod({}, 0{:o}) failed", cpath, mode);
        else
            clearError(PTL_ERROR_REF(err));
    }
    #endif

    inline void getStatus(FileDescriptorLike auto && desc, struct ::stat & res,
                          PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto fd = c_fd(std::forward<decltype(desc)>(desc));
        if (::fstat(fd, &res) != 0)
            handleError(PTL_ERROR_REF(err), errno, "fstat({}) failed", fd);
        else
            clearError(PTL_ERROR_REF(err));
    }

    inline void getStatus(PathLike auto && path, struct ::stat & res,
                          PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto cpath = c_path(std::forward<decltype(path)>(path));
        if (::stat(cpath, &res) != 0)
            handleError(PTL_ERROR_REF(err), errno, "stat({}) failed", cpath);
        else
            clearError(PTL_ERROR_REF(err));
    }

    inline void getLinkStatus(PathLike auto && path, struct ::stat & res,
                              PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto cpath = c_path(std::forward<decltype(path)>(path));
        if (::lstat(cpath, &res) != 0)
            handleError(PTL_ERROR_REF(err), errno, "lstat({}) failed", cpath);
        else
            clearError(PTL_ERROR_REF(err));
    }

    inline void makeDirectory(PathLike auto && path, mode_t mode,
                              PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto cpath = c_path(std::forward<decltype(path)>(path));
        if (::mkdir(cpath, mode) != 0)
            handleError(PTL_ERROR_REF(err), errno, "mkdir({}, 0{:o}) failed", cpath, mode);
        else
            clearError(PTL_ERROR_REF(err));
    }

    inline void makeDirectoryAt(FileDescriptorLike auto && desc, PathLike auto && path, mode_t mode,
                                PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto fd = c_fd(std::forward<decltype(desc)>(desc));
        auto cpath = c_path(std::forward<decltype(path)>(path));
        if (::mkdirat(fd, cpath, mode) != 0)
            handleError(PTL_ERROR_REF(err), errno, "mkdirat({}, {}, 0{:o}) failed", fd, cpath, mode);
        else
            clearError(PTL_ERROR_REF(err));
    }

    inline void truncateFile(FileDescriptorLike auto && desc, off_t length,
                             PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto fd = c_fd(std::forward<decltype(desc)>(desc));
        if (::ftruncate(fd, length) != 0)
            handleError(PTL_ERROR_REF(err), errno, "ftruncate({}, {}) failed", fd, length);
        else
            clearError(PTL_ERROR_REF(err));
    }

    inline void truncateFile(PathLike auto && path, off_t length,
                             PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto cpath = c_path(std::forward<decltype(path)>(path));
        if (::truncate(cpath, length) != 0)
            handleError(PTL_ERROR_REF(err), errno, "truncate({}, {}) failed", cpath, length);
        else
            clearError(PTL_ERROR_REF(err));
    }

    inline void changeDirectory(FileDescriptorLike auto && desc,
                                PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {

        auto fd = c_fd(std::forward<decltype(desc)>(desc));
        if (::fchdir(fd) != 0)
            handleError(PTL_ERROR_REF(err), errno, "fchdir({}) failed", fd);
        else
            clearError(PTL_ERROR_REF(err));
    }

    inline void changeDirectory(PathLike auto && path,
                                PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {

        auto cpath = c_path(std::forward<decltype(path)>(path));
        if (::chdir(cpath) != 0)
            handleError(PTL_ERROR_REF(err), errno, "chdir({}) failed", cpath);
        else
            clearError(PTL_ERROR_REF(err));
    }

    inline void changeRoot(PathLike auto && path,
                           PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {

        auto cpath = c_path(std::forward<decltype(path)>(path));
        if (::chroot(cpath) != 0)
            handleError(PTL_ERROR_REF(err), errno, "chroot({}) failed", cpath);
        else
            clearError(PTL_ERROR_REF(err));
    }

    #endif
}

#endif