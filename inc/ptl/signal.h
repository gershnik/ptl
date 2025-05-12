// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PTL_HEADER_SIGNAL_H_INCLUDED
#define PTL_HEADER_SIGNAL_H_INCLUDED

#include <ptl/core.h>
#include <ptl/process.h>

#include <signal.h>
#include <string.h>

#if PTL_HAVE_SYS_SIGABBREV_UNDECLARED
    extern "C" {
        extern const char * const sys_sigabbrev[NSIG];
    }
#endif

namespace ptl::inline v0 {

    #ifndef _WIN32
    class SignalSet {
    enum Init { Emtpy, All };
    public:
        SignalSet() noexcept {
            sigemptyset(&m_set);
        }
        SignalSet(sigset_t src) noexcept : m_set(src)
        {}
        static auto none() noexcept -> SignalSet {
            sigset_t set;
            sigemptyset(&set);
            return set;
        }
        static auto all() noexcept -> SignalSet {
            sigset_t set;
            sigfillset(&set);
            return set;
        }
        void add(int signo)
            { posixCheck(sigaddset(&m_set, signo), "sigaddset(..., {}) failed", signo); }
        void del(int signo)
            { posixCheck(sigdelset(&m_set, signo), "sigdelset(..., {}) failed", signo); }
        auto isMember(int signo) const -> bool { 
            int ret = sigismember(&m_set, signo); 
            if (ret < 0)
                throwErrorCode(errno, "sigismember(..., {}) failed", signo);
            return ret != 0;
        }

        auto get() const noexcept -> const sigset_t &
            { return m_set; }
        auto get() noexcept -> sigset_t &
            { return m_set; }
    private:
        sigset_t m_set;
    };

    inline auto signalMessage(int sig, PTL_ERROR_REF_ARG(err)) -> std::string 
    requires(PTL_ERROR_REQ(err)) {
        errno = 0;
        const char * str = ::strsignal(sig);
        if (errno) {
            str = "";
            handleError(PTL_ERROR_REF(err), errno, "strsignal({}) failed", sig);
        }
        return str;
    }
    #endif

    inline auto signalName(int sig) -> std::string {
    #if PTL_HAVE_SIGABBREV_NP
        if (auto str = sigabbrev_np(sig))
            return str;
    #elif PTL_HAVE_SYS_SIGNAME
        if (sig > 0 && sig < NSIG) {
            std::string str = sys_signame[sig];
            for (auto & c: str) c = std::toupper(c);
            return str;
        }
    #elif PTL_HAVE_SYS_SIGABBREV
        if (sig > 0 && sig < NSIG) {
            #ifndef __CYGWIN__
                std::string str = sys_sigabbrev[sig];
            #else
                std::string str = sys_sigabbrev[sig] + 3;
            #endif
            return str;
        }
    
    #endif
        
        return std::to_string(sig);
    }

    #ifndef __MINGW32__
    inline void sendSignal(ProcessLike auto && proc, int sig, PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {
        clearError();
        auto pid = c_pid(std::forward<decltype(proc)>(proc));
        int res = ::kill(pid, sig);
        if (res != 0)
            handleError(PTL_ERROR_REF(err), errno, "kill({}, {}) failed", pid, sig);
    }
    #endif

    inline void raiseSignal(int sig, PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {
        if (::raise(sig) != 0)
            handleError(PTL_ERROR_REF(err), errno, "raise({}) failed", sig);
        else
            clearError();
    }

    inline auto setSignalHandler(int signo, void (* handler)(int)) -> void (*)(int) {
        auto ret = ::signal(signo, handler);
        if (ret == SIG_ERR)
            throwErrorCode(errno, "signal({}) failed", signo);
        return ret;
    }

    #ifndef _WIN32
    using struct_sigaction = struct ::sigaction;

    class SignalAction : public struct_sigaction {
    private:
        using super = struct_sigaction;
    public:
        constexpr SignalAction() noexcept : super{} 
        {}
        SignalAction(void (* handler)(int)) noexcept : super{} {
            this->sa_handler = handler;
        }
        SignalAction(void (* handler)(int, siginfo_t *, void *)) noexcept : super{} {
            this->sa_sigaction = handler;
            this->sa_flags = SA_SIGINFO;
        }
        SignalAction(void (* handler)(int), int flags) noexcept : super{} {
            this->sa_handler = handler;
            this->sa_flags = flags & ~SA_SIGINFO;
        }
        SignalAction(void (* handler)(int, siginfo_t *, void *), int flags) noexcept : super{} {
            this->sa_sigaction = handler;
            this->sa_flags = flags | SA_SIGINFO;
        }

        void setMask(SignalSet mask) noexcept {
            this->sa_mask = mask.get();
        }
    };
    static_assert(sizeof(SignalAction) == sizeof(struct_sigaction));
    static_assert(alignof(SignalAction) == alignof(struct_sigaction));

    inline void setSignalAction(int signo, const SignalAction & act, SignalAction & oact) {
        posixCheck(::sigaction(signo, &act, &oact), "sigaction({}) failed", signo);
    }

    inline void setSignalAction(int signo, const SignalAction & act) {
        posixCheck(::sigaction(signo, &act, nullptr), "sigaction({}) failed", signo);
    }

    inline void getSignalAction(int signo, SignalAction & oact) {
        posixCheck(::sigaction(signo, nullptr, &oact), "sigaction({}) failed", signo);
    }

    inline void setSignalProcessMask(int how, const SignalSet & set, SignalSet & oset) {
        posixCheck(::sigprocmask(how, &set.get(), &oset.get()), "sigprocmask({},...) failed", how);
    }

    inline void setSignalProcessMask(int how, const SignalSet & set) {
        posixCheck(::sigprocmask(how, &set.get(), nullptr), "sigprocmask({},...) failed", how);
    }

    inline void getSignalProcessMask(SignalSet & oset) {
        posixCheck(::sigprocmask(SIG_SETMASK, nullptr, &oset.get()), "sigprocmask(SIG_SETMASK, nullptr, ...) failed");
    }
    
    #endif
}


#endif