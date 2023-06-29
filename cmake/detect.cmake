include(CMakePushCheckState)
include(CheckCXXSymbolExists)
include(CheckCXXSourceCompiles)
include(CheckCXXSourceRuns)

check_cxx_symbol_exists(execvpe unistd.h PTL_HAVE_EXECVPE)
check_cxx_symbol_exists(posix_spawn_file_actions_addinherit_np spawn.h PTL_HAVE_POSIX_SPAWN_FILE_ACTIONS_ADDINHERIT_NP)
check_cxx_symbol_exists(posix_spawn_file_actions_addchdir_np spawn.h PTL_HAVE_POSIX_SPAWN_FILE_ACTIONS_ADDCHDIR_NP)
check_cxx_symbol_exists(posix_spawn_file_actions_addclosefrom_np spawn.h PTL_HAVE_POSIX_SPAWN_FILE_ACTIONS_ADDCLOSEFROM_NP)
check_cxx_symbol_exists(sigabbrev_np string.h PTL_HAVE_SIGABBREV_NP)
check_cxx_symbol_exists(sys_signame signal.h PTL_HAVE_SYS_SIGNAME)
check_cxx_symbol_exists(lchmod sys/stat.h PTL_HAVE_LCHMOD)
check_cxx_source_runs("
    #include <signal.h>
    #include <stdio.h>
    int main() { 
        const char * x = sys_sigabbrev[SIGINT];
        printf(\"%s\", x);
    }" 
PTL_HAVE_SYS_SIGABBREV_DECLARED)
check_cxx_source_runs("
    #include <signal.h>
    #include <stdio.h>
    extern \"C\" {
        extern const char * const sys_sigabbrev[NSIG];
    }
    int main() { 
        const char * x = sys_sigabbrev[SIGINT];
        printf(\"%s\", x);
    }" 
PTL_HAVE_SYS_SIGABBREV_UNDECLARED)
check_cxx_source_runs("
    #include <unistd.h>
    #include <stdlib.h>
    
    int main() { 
        int (*p)(char *, int, int) = mkostemps;
    }" 
PTL_HAVE_MKOSTEMPS)

