#ifndef PTL_HEADER_SPAWN_H_INCLUDED
#define PTL_HEADER_SPAWN_H_INCLUDED

#include <ptl/core.h>
#include <ptl/process.h>
#include <ptl/file_descriptor.h>
#include <ptl/signal.h>

#include <cassert>
#include <optional>
#include <iterator>

#include <spawn.h>


namespace ptl {

    inline auto forkProcess(PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) -> ChildProcess {
        auto ret = ::fork();
        if (ret < 0) {
            ret = 0;
            handleError(PTL_ERROR_REF(err), errno, "fork() failed");
        }
        return ChildProcess{ret};
    }

    class StringRefArray {
    public:
        template<StringLikeArray T>
        StringRefArray(T && array) {
            const auto arraySize = std::size(std::forward<T>(array));
            auto arrayBegin = std::begin(std::forward<T>(array));

            if constexpr (std::is_same_v<std::remove_cvref_t<decltype(*arrayBegin)>, const char *> &&
                          std::contiguous_iterator<decltype(arrayBegin)>) {
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
        void transform(StringLikeArray auto && array, std::unsigned_integral auto size) {
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

//MARK: - Spawn
#pragma region Spawn

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

    
    struct SpawnSettings {

    public:
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

        auto usePath() noexcept -> SpawnSettings & {
            m_func = &posix_spawnp;
            return *this;
        }

        auto doSpawn(const char * path, const char * const * args, const char * const * env,
                     PTL_ERROR_REF_ARG(err)) const noexcept(PTL_ERROR_NOEXCEPT(err)) -> ChildProcess 
        requires(PTL_ERROR_REQ(err)) {
            clearError(PTL_ERROR_REF(err));
            pid_t childPid;
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
            return ChildProcess(childPid);
        }
        
    private:
        decltype(posix_spawn) * m_func = &posix_spawn;
        const posix_spawn_file_actions_t * m_fileActions = nullptr;
        const posix_spawnattr_t * m_attr = nullptr;
    };

    
    //exe, env, settings    
    inline auto spawn(PathLike auto && exe, const StringRefArray & args, const StringRefArray & env, const SpawnSettings & settings, 
                      PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) -> ChildProcess 
    requires(PTL_ERROR_REQ(err)) {
        return settings.doSpawn(c_path(std::forward<decltype(exe)>(exe)), args.data(), env.data(), PTL_ERROR_REF(err));
    }

    //none, env, settings 
    inline auto spawn(const StringRefArray & args, const StringRefArray & env, const SpawnSettings & settings,
                      PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) -> ChildProcess 
    requires(PTL_ERROR_REQ(err)) {
        assert(!args.empty());
        return spawn(args[0], args, env, settings, PTL_ERROR_REF(err));
    }

    //exe, none, settings
    inline auto spawn(PathLike auto && exe, const StringRefArray & args, const SpawnSettings & settings, 
                      PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) -> ChildProcess 
    requires(PTL_ERROR_REQ(err)) {
        return settings.doSpawn(c_path(std::forward<decltype(exe)>(exe)), args.data(), nullptr, PTL_ERROR_REF(err));
    }

    //exe, env, none
    inline auto spawn(PathLike auto && exe, const StringRefArray & args, const StringRefArray & env, 
                      PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) -> ChildProcess 
    requires(PTL_ERROR_REQ(err)) {
        return spawn(c_path(std::forward<decltype(exe)>(exe)), args, env, {}, PTL_ERROR_REF(err));
    }

    //none, none, settings
    inline auto spawn(const StringRefArray & args, const SpawnSettings & settings,
                      PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) -> ChildProcess 
    requires(PTL_ERROR_REQ(err)) {
        assert(!args.empty());
        return spawn(args[0], args, settings, PTL_ERROR_REF(err));
    }

    //exe, none, none
    inline auto spawn(PathLike auto && exe, const StringRefArray & args,
                      PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) -> ChildProcess 
    requires(PTL_ERROR_REQ(err)) {
        return spawn(std::forward<decltype(exe)>(exe), args, {}, PTL_ERROR_REF(err));
    }

    //none, env, none
    inline auto spawn(const StringRefArray & args, const StringRefArray & env,
                      PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) -> ChildProcess 
    requires(PTL_ERROR_REQ(err)) {
        assert(!args.empty());
        return spawn(args[0], args, env, {}, PTL_ERROR_REF(err));
    }

    //none, none, none
    inline auto spawn(const StringRefArray & args,
                      PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) -> ChildProcess 
    requires(PTL_ERROR_REQ(err)) {
        assert(!args.empty());
        return spawn(args[0], args, {}, PTL_ERROR_REF(err));
    }

#pragma endregion
    
//MARK: - Exec
#pragma region Exec

    //exe, env
    inline auto exec(PathLike auto && exe, const StringRefArray & args, const StringRefArray & env,
                     PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) -> void
    requires(PTL_ERROR_REQ(err)) {

        clearError(PTL_ERROR_REF(err));
        auto path = c_path(std::forward<decltype(exe)>(exe));
        execve(path, args.data(), env.data());
        handleError(PTL_ERROR_REF(err), errno, "cannot exec {}", path);
    }

    //exe, none
    inline auto exec(PathLike auto && exe, const StringRefArray & args,
                     PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) -> void
    requires(PTL_ERROR_REQ(err)) {

        clearError(PTL_ERROR_REF(err));
        auto path = c_path(std::forward<decltype(exe)>(exe));
        execv(path, args.data());
        handleError(PTL_ERROR_REF(err), errno, "cannot exec {}", path);
    }

    //none, env
    inline auto exec(const StringRefArray & args, const StringRefArray & env,
                     PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) -> void
    requires(PTL_ERROR_REQ(err)) {

        assert(!args.empty());
        exec(args[0], args, env, PTL_ERROR_REF(err));
    }

    //none, none
    inline auto exec(const StringRefArray & args,
                     PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) -> void
    requires(PTL_ERROR_REQ(err)) {

        assert(!args.empty());
        exec(args[0], args, PTL_ERROR_REF(err));
    }

#pragma endregion

//MARK: - Execp
#pragma region Execp

    #if PTL_HAVE_EXECVPE
    //exe, env
    inline auto execp(PathLike auto && exe, const StringRefArray & args, const StringRefArray & env,
                      PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) -> void
    requires(PTL_ERROR_REQ(err)) {

        clearError(PTL_ERROR_REF(err));
        auto path = c_path(std::forward<decltype(exe)>(exe));
        execvpe(path, const_cast<char * const *>(args.data()), const_cast<char * const *>(env.data()));
        handleError(PTL_ERROR_REF(err), errno, "cannot exec {}", path);
    }

    //none, env
    inline auto execp(const StringRefArray & args, const StringRefArray & env,
                      PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) -> void
    requires(PTL_ERROR_REQ(err)) {

        assert(!args.empty());
        execp(args[0], args, env, PTL_ERROR_REF(err));
    }

    #endif

    //exe, none
    inline auto execp(PathLike auto && exe, const StringRefArray & args,
                      PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) -> void
    requires(PTL_ERROR_REQ(err)) {

        clearError(PTL_ERROR_REF(err));
        auto path = c_path(std::forward<decltype(exe)>(exe));
        execvp(path, const_cast<char * const *>(args.data()));
        handleError(PTL_ERROR_REF(err), errno, "cannot exec {}", path);
    }

    //none, none
    inline auto execp(const StringRefArray & args,
                      PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err)) -> void
    requires(PTL_ERROR_REQ(err)) {

        assert(!args.empty());
        execp(args[0], args, PTL_ERROR_REF(err));
    }
    
#pragma endregion
    
}

#endif
