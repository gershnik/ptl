#ifndef PTL_HEADER_FILE_DESCRIPTOR_H_INCLUDED
#define PTL_HEADER_FILE_DESCRIPTOR_H_INCLUDED

#include <ptl/core.h>

#include <unistd.h>
#include <fcntl.h>

namespace ptl {

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
                ::close(m_fd);
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
            auto fd = ::open(c_path(std::forward<decltype(path)>(path)), oflag, mode);
            if (fd < 0) {
                fd = -1;
                handleError(PTL_ERROR_REF(err), errno, "cannot open {}", c_path(path));
            }
            return FileDescriptor(fd);
        }

        static auto open(PathLike auto && path, int oflag, PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) -> FileDescriptor 
        requires(PTL_ERROR_REQ(err)) {
            return open(std::forward<decltype(path)>(path), oflag, 0, PTL_ERROR_REF(err));
        }
        
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
        
        static auto pipe(PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) -> std::pair<FileDescriptor, FileDescriptor> 
        requires(PTL_ERROR_REQ(err)) {
            clearError(PTL_ERROR_REF(err));
            int fds[2];
            if (::pipe(fds) != 0) {
                fds[0] = -1;
                fds[1] = -1;
                handleError(PTL_ERROR_REF(err), errno, "pipe() call failed");
            }
            return std::make_pair(FileDescriptor(fds[0]), FileDescriptor(fds[1]));
        }

        void dup2(const FileDescriptorLike auto & fdTo) const {
            auto res = ::dup2(m_fd, c_fd(std::forward<decltype(fdTo)>(fdTo)));
            if (res < 0) 
                throwErrorCode(errno, "dup2({},{}) failed", m_fd, c_fd(std::forward<decltype(fdTo)>(fdTo)));
        }

        auto read(void * buf, size_t nbyte, PTL_ERROR_REF_ARG(err)) const noexcept(PTL_ERROR_NOEXCEPT(err)) -> ssize_t 
        requires(PTL_ERROR_REQ(err)) {
            clearError(PTL_ERROR_REF(err));
            auto ret = ::read(m_fd, buf, nbyte);
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
            { return fileno(fp);}
    };

    
}

#endif