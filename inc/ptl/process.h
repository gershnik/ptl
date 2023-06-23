#ifndef PTL_HEADER_PROCESS_H_INCLUDED
#define PTL_HEADER_PROCESS_H_INCLUDED

#include <ptl/core.h>

#include <sys/types.h>
#include <sys/wait.h>

namespace ptl::inline v0 {

    template<class T> struct ProcessTraits;

    template<class T>
    concept ProcessLike = requires(T && obj) {
        { ProcessTraits<std::remove_cvref_t<T>>::c_pid(std::forward<T>(obj)) } -> std::same_as<pid_t>;
    };

    template<ProcessLike T>
    [[gnu::always_inline]] inline int c_pid(T && obj)
        { return ProcessTraits<std::remove_cvref_t<T>>::c_pid(std::forward<T>(obj)); }


    class ChildProcess {
    public:
        ChildProcess() noexcept = default;
        explicit ChildProcess(pid_t pid) noexcept : m_pid(pid) {
            assert(m_pid >= 0);
        }
        ~ChildProcess() noexcept {
            if (m_pid) {
                int stat;
                for ( ; ; ) {
                    pid_t res = ::waitpid(m_pid, &stat, 0);
                    if (res < 0 && errno == EINTR) 
                        continue;
                    assert(res == m_pid);
                    break;
                }
            }
        }
        ChildProcess(ChildProcess && src) noexcept : m_pid(src.m_pid) {
            src.m_pid = 0;
        }
        
        ChildProcess(const ChildProcess &) = delete;
        ChildProcess & operator=(ChildProcess src) noexcept {
            swap(src, *this);
            return *this;
        }
        
        friend void swap(ChildProcess & lhs, ChildProcess & rhs) noexcept {
            std::swap(lhs.m_pid, rhs.m_pid);
        }
        
        auto get() const noexcept -> pid_t {
            return m_pid;
        }
        
        auto detach() noexcept -> pid_t {
            pid_t ret = m_pid;
            m_pid = 0;
            return ret;
        }
        
        explicit operator bool() const noexcept {
            return m_pid != 0;
        }
        
        auto wait(int flags, PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) -> std::optional<int> 
        requires(PTL_ERROR_REQ(err)) {
            clearError(PTL_ERROR_REF(err));
            if (m_pid == 0) {
                throwErrorCode(EINVAL, "ChildProcess not started or has already been waited for");
            }
            int stat;
            pid_t res = ::waitpid(m_pid, &stat, flags);
            if (res < 0) {
                stat = -1;
                handleError(PTL_ERROR_REF(err), errno, "waitpid for {} failed", m_pid);
            } else if (res > 0) {
                assert(res == m_pid);
                if (WIFEXITED(stat) || WIFSIGNALED(stat))
                    m_pid = 0;
                return stat;
            }
            return std::nullopt;
        }

        auto wait(PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) -> std::optional<int> 
        requires(PTL_ERROR_REQ(err)) {
            return wait(0, PTL_ERROR_REF(err));
        }

    private:
        pid_t m_pid = 0;
    };


    template<> struct ProcessTraits<pid_t> {
        [[gnu::always_inline]] static pid_t c_pid(pid_t pid)
            { return pid;}
    };
    template<> struct ProcessTraits<ChildProcess> {
        [[gnu::always_inline]] static pid_t c_fd(const ChildProcess & proc)
            { return proc.get();}
    };

}

#endif 