#ifndef PTL_HEADER_SPAWN_H_INCLUDED
#define PTL_HEADER_SPAWN_H_INCLUDED

#include <ptl/core.h>
#include <ptl/process.h>
#include <ptl/file.h>
#include <ptl/signal.h>

#include <cassert>
#include <optional>
#include <iterator>

#if __has_include(<spawn.h>)
    #include <spawn.h>
#endif


namespace ptl::inline v0 {

    #ifndef __MINGW32__
    inline auto forkProcess(PTL_ERROR_REF_ARG(err)) -> ChildProcess {
        auto ret = ::fork();
        if (ret < 0) {
            ret = 0;
            handleError(PTL_ERROR_REF(err), errno, "fork() failed");
        }
        return ChildProcess{ret};
    }
    #endif

//MARK: - Spawn
#pragma region Spawn

    #ifndef _WIN32
    class SpawnFileActions {
    public:
        SpawnFileActions() 
            { posixCheck(posix_spawn_file_actions_init(&m_wrapped), "posix_spawn_file_actions_init failed"); }
        ~SpawnFileActions() noexcept
            { posix_spawn_file_actions_destroy(&m_wrapped); }
        SpawnFileActions(const SpawnFileActions &) = delete;
        SpawnFileActions & operator=(const SpawnFileActions &) = delete;
        
        auto get() const noexcept -> const posix_spawn_file_actions_t *
            { return &m_wrapped; }

        void addClose(FileDescriptorLike auto && fd) {
            posixCheck(posix_spawn_file_actions_addclose(&m_wrapped, 
                                                         c_fd(std::forward<decltype(fd)>(fd))),
                       "posix_spawn_file_actions_addclose failed");
        }

        void addOpen(FileDescriptorLike auto && fd, PathLike auto && path, int oflag, mode_t mode) {
            posixCheck(posix_spawn_file_actions_addopen(&m_wrapped, 
                                                        c_fd(std::forward<decltype(fd)>(fd)),
                                                        c_path(std::forward<decltype(path)>(path)),
                                                        oflag,
                                                        mode),
                       "posix_spawn_file_actions_addopen failed");
        }

        void addDup2(FileDescriptorLike auto && fdFrom, const FileDescriptorLike auto & fdTo) {
            posixCheck(posix_spawn_file_actions_adddup2(&m_wrapped, 
                                                        c_fd(std::forward<decltype(fdFrom)>(fdFrom)), 
                                                        c_fd(std::forward<decltype(fdTo)>(fdTo))),
                       "posix_spawn_file_actions_adddup2 failed");
        }
        
        #if PTL_HAVE_POSIX_SPAWN_FILE_ACTIONS_ADDINHERIT_NP
        void addInheritNp(FileDescriptorLike auto && fd) {
            posixCheck(posix_spawn_file_actions_addinherit_np(&m_wrapped, 
                                                              c_fd(std::forward<decltype(fd)>(fd))),
                       "posix_spawn_file_actions_addinherit_np failed");
        }
        #endif

        #if PTL_HAVE_POSIX_SPAWN_FILE_ACTIONS_ADDCLOSEFROM_NP
        void addCloseFromNp(FileDescriptorLike auto && fd) {
            posixCheck(posix_spawn_file_actions_addclosefrom_np(&m_wrapped, 
                                                                c_fd(std::forward<decltype(fd)>(fd))),
                       "posix_spawn_file_actions_addclosefrom_np failed");
        }
        #endif

        #if PTL_HAVE_POSIX_SPAWN_FILE_ACTIONS_ADDCHDIR_NP
        void addChdirNp(PathLike auto && path) {
            posixCheck(posix_spawn_file_actions_addchdir_np(&m_wrapped, 
                                                            c_path(std::forward<decltype(path)>(path))),
                       "posix_spawn_file_actions_addchdir_np failed");
        }
        #endif
        
        
    private:
        posix_spawn_file_actions_t m_wrapped;
    };

    class SpawnAttr {
    public:
        SpawnAttr()
            { posixCheck(posix_spawnattr_init(&m_wrapped), "posix_spawnattr_init failed"); }
        ~SpawnAttr()
            { posix_spawnattr_destroy(&m_wrapped); }
        SpawnAttr(const SpawnAttr &) = delete;
        SpawnAttr & operator=(const SpawnAttr &) = delete;
        
        auto get() const noexcept -> const posix_spawnattr_t *
            { return &m_wrapped; }
        
        void setFlags(short flags) {
            posixCheck(posix_spawnattr_setflags(&m_wrapped, flags), "posix_spawnattr_setflags failed");
        }
        void setSigDefault(const SignalSet & signals) {
            posixCheck(posix_spawnattr_setsigdefault(&m_wrapped, &signals.get()), "posix_spawnattr_setsigdefault failed");
        }
        void setPGroup(pid_t pgroup = 0) {
            posixCheck(posix_spawnattr_setpgroup(&m_wrapped, pgroup), "posix_spawnattr_setpgroup failed");
        }
    private:
        posix_spawnattr_t m_wrapped;
    };
    #endif

    
    class SpawnSettings {

    public:
        #ifndef _WIN32
        auto fileActions(const posix_spawn_file_actions_t * ptr) noexcept -> SpawnSettings & {
            m_fileActions = ptr;
            return *this;
        }
        auto fileActions(const SpawnFileActions & val) noexcept -> SpawnSettings & {
            m_fileActions = val.get();
            return *this;
        }
        auto fileActions(SpawnFileActions && val) = delete;

        auto attr(const posix_spawnattr_t * ptr) noexcept -> SpawnSettings & {
            m_attr = ptr;
            return *this;
        }
        auto attr(const SpawnAttr & val) noexcept -> SpawnSettings & {
            m_attr = val.get();
            return *this;
        }
        auto attr(SpawnAttr && val) = delete;
        #endif

        auto usePath() noexcept -> SpawnSettings & {
            #ifndef _WIN32
                m_func = &::posix_spawnp;
            #else
                m_func = &::_spawnvpe;
            #endif
            return *this;
        }

        auto doSpawn(const char * path, const char * const * args, const char * const * env,
                     PTL_ERROR_REF_ARG(err)) const -> ChildProcess 
        requires(PTL_ERROR_REQ(err)) {
            pid_t childPid;
            #ifndef _WIN32
            int res = m_func(&childPid, 
                                path,
                                m_fileActions, 
                                m_attr,
                                const_cast<char * const *>(args),
                                const_cast<char * const *>(env)); 
            if (res != 0) {
                childPid = 0;
                handleError(PTL_ERROR_REF(err), res, "cannot spawn {}", path);
            }
            #else
            childPid = m_func(_P_NOWAIT, path, args, env);
            if (childPid == -1) {
                childPid = 0;
                handleError(PTL_ERROR_REF(err), errno, "cannot spawn {}", path);
            }
            #endif
            else {
                clearError(PTL_ERROR_REF(err));
            }
            return ChildProcess(childPid);
        }
        
    private:
        #ifndef _WIN32
            decltype(::posix_spawn) * m_func = &::posix_spawn;
            const posix_spawn_file_actions_t * m_fileActions = nullptr;
            const posix_spawnattr_t * m_attr = nullptr;
        #else
            decltype(::_spawnve) * m_func = &::_spawnve;
        #endif
    };

    
    //exe, env, settings    
    inline auto spawn(PathLike auto && exe, const StringRefArray & args, const StringRefArray & env, const SpawnSettings & settings, 
                      PTL_ERROR_REF_ARG(err)) -> ChildProcess 
    requires(PTL_ERROR_REQ(err)) {
        return settings.doSpawn(c_path(std::forward<decltype(exe)>(exe)), args.data(), env.data(), PTL_ERROR_REF(err));
    }

    //none, env, settings 
    inline auto spawn(const StringRefArray & args, const StringRefArray & env, const SpawnSettings & settings,
                      PTL_ERROR_REF_ARG(err)) -> ChildProcess 
    requires(PTL_ERROR_REQ(err)) {
        assert(!args.empty());
        return spawn(args[0], args, env, settings, PTL_ERROR_REF(err));
    }

    //exe, none, settings
    inline auto spawn(PathLike auto && exe, const StringRefArray & args, const SpawnSettings & settings, 
                      PTL_ERROR_REF_ARG(err)) -> ChildProcess 
    requires(PTL_ERROR_REQ(err)) {
        return settings.doSpawn(c_path(std::forward<decltype(exe)>(exe)), args.data(), nullptr, PTL_ERROR_REF(err));
    }

    //exe, env, none
    inline auto spawn(PathLike auto && exe, const StringRefArray & args, const StringRefArray & env, 
                      PTL_ERROR_REF_ARG(err)) -> ChildProcess 
    requires(PTL_ERROR_REQ(err)) {
        return spawn(c_path(std::forward<decltype(exe)>(exe)), args, env, {}, PTL_ERROR_REF(err));
    }

    //none, none, settings
    inline auto spawn(const StringRefArray & args, const SpawnSettings & settings,
                      PTL_ERROR_REF_ARG(err)) -> ChildProcess 
    requires(PTL_ERROR_REQ(err)) {
        assert(!args.empty());
        return spawn(args[0], args, settings, PTL_ERROR_REF(err));
    }

    //exe, none, none
    inline auto spawn(PathLike auto && exe, const StringRefArray & args,
                      PTL_ERROR_REF_ARG(err)) -> ChildProcess 
    requires(PTL_ERROR_REQ(err)) {
        return spawn(std::forward<decltype(exe)>(exe), args, {}, PTL_ERROR_REF(err));
    }

    //none, env, none
    inline auto spawn(const StringRefArray & args, const StringRefArray & env,
                      PTL_ERROR_REF_ARG(err)) -> ChildProcess 
    requires(PTL_ERROR_REQ(err)) {
        assert(!args.empty());
        return spawn(args[0], args, env, {}, PTL_ERROR_REF(err));
    }

    //none, none, none
    inline auto spawn(const StringRefArray & args,
                      PTL_ERROR_REF_ARG(err)) -> ChildProcess 
    requires(PTL_ERROR_REQ(err)) {
        assert(!args.empty());
        return spawn(args[0], args, {}, PTL_ERROR_REF(err));
    }

#pragma endregion
    
//MARK: - Exec
#pragma region Exec

    #ifndef _WIN32
    //exe, env
    inline auto exec(PathLike auto && exe, const StringRefArray & args, const StringRefArray & env,
                     PTL_ERROR_REF_ARG(err)) -> void
    requires(PTL_ERROR_REQ(err)) {

        clearError(PTL_ERROR_REF(err));
        auto path = c_path(std::forward<decltype(exe)>(exe));
        execve(path, args.data(), env.data());
        handleError(PTL_ERROR_REF(err), errno, "cannot exec {}", path);
    }

    //exe, none
    inline auto exec(PathLike auto && exe, const StringRefArray & args,
                     PTL_ERROR_REF_ARG(err)) -> void
    requires(PTL_ERROR_REQ(err)) {

        auto path = c_path(std::forward<decltype(exe)>(exe));
        execv(path, args.data());
        handleError(PTL_ERROR_REF(err), errno, "cannot exec {}", path);
    }

    //none, env
    inline auto exec(const StringRefArray & args, const StringRefArray & env,
                     PTL_ERROR_REF_ARG(err)) -> void
    requires(PTL_ERROR_REQ(err)) {

        assert(!args.empty());
        exec(args[0], args, env, PTL_ERROR_REF(err));
    }

    //none, none
    inline auto exec(const StringRefArray & args,
                     PTL_ERROR_REF_ARG(err)) -> void
    requires(PTL_ERROR_REQ(err)) {

        assert(!args.empty());
        exec(args[0], args, PTL_ERROR_REF(err));
    }
    #endif

#pragma endregion

//MARK: - Execp
#pragma region Execp

    #ifndef _WIN32
    #if PTL_HAVE_EXECVPE
    //exe, env
    inline auto execp(PathLike auto && exe, const StringRefArray & args, const StringRefArray & env,
                      PTL_ERROR_REF_ARG(err)) -> void
    requires(PTL_ERROR_REQ(err)) {

        clearError(PTL_ERROR_REF(err));
        auto path = c_path(std::forward<decltype(exe)>(exe));
        execvpe(path, const_cast<char * const *>(args.data()), const_cast<char * const *>(env.data()));
        handleError(PTL_ERROR_REF(err), errno, "cannot exec {}", path);
    }

    //none, env
    inline auto execp(const StringRefArray & args, const StringRefArray & env,
                      PTL_ERROR_REF_ARG(err)) -> void
    requires(PTL_ERROR_REQ(err)) {

        assert(!args.empty());
        execp(args[0], args, env, PTL_ERROR_REF(err));
    }

    #endif

    //exe, none
    inline auto execp(PathLike auto && exe, const StringRefArray & args,
                      PTL_ERROR_REF_ARG(err)) -> void
    requires(PTL_ERROR_REQ(err)) {

        auto path = c_path(std::forward<decltype(exe)>(exe));
        execvp(path, const_cast<char * const *>(args.data()));
        handleError(PTL_ERROR_REF(err), errno, "cannot exec {}", path);
    }

    //none, none
    inline auto execp(const StringRefArray & args,
                      PTL_ERROR_REF_ARG(err)) -> void
    requires(PTL_ERROR_REQ(err)) {

        assert(!args.empty());
        execp(args[0], args, PTL_ERROR_REF(err));
    }
    #endif
    
#pragma endregion
    
}

#endif
