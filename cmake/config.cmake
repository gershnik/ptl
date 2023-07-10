# Copyright (c) 2023, Eugene Gershnik
# SPDX-License-Identifier: BSD-3-Clause

include(CMakePushCheckState)
include(CheckCXXSymbolExists)
include(CheckCXXSourceCompiles)

set(CONFIG_CONTENT  
"// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PTL_HEADER_CONFIG_H_INCLUDED
#define PTL_HEADER_CONFIG_H_INCLUDED

")

check_cxx_symbol_exists(execvpe unistd.h PTL_HAVE_EXECVPE)
string(APPEND CONFIG_CONTENT "#cmakedefine01 PTL_HAVE_EXECVPE\n")

check_cxx_symbol_exists(posix_spawn_file_actions_addinherit_np spawn.h PTL_HAVE_POSIX_SPAWN_FILE_ACTIONS_ADDINHERIT_NP)
string(APPEND CONFIG_CONTENT "#cmakedefine01 PTL_HAVE_POSIX_SPAWN_FILE_ACTIONS_ADDINHERIT_NP\n")

check_cxx_symbol_exists(posix_spawn_file_actions_addchdir_np spawn.h PTL_HAVE_POSIX_SPAWN_FILE_ACTIONS_ADDCHDIR_NP)
string(APPEND CONFIG_CONTENT "#cmakedefine01 PTL_HAVE_POSIX_SPAWN_FILE_ACTIONS_ADDCHDIR_NP\n")

check_cxx_symbol_exists(posix_spawn_file_actions_addclosefrom_np spawn.h PTL_HAVE_POSIX_SPAWN_FILE_ACTIONS_ADDCLOSEFROM_NP)
string(APPEND CONFIG_CONTENT "#cmakedefine01 PTL_HAVE_POSIX_SPAWN_FILE_ACTIONS_ADDCLOSEFROM_NP\n")

check_cxx_symbol_exists(sigabbrev_np string.h PTL_HAVE_SIGABBREV_NP)
string(APPEND CONFIG_CONTENT "#cmakedefine01 PTL_HAVE_SIGABBREV_NP\n")

check_cxx_symbol_exists(sys_signame signal.h PTL_HAVE_SYS_SIGNAME)
string(APPEND CONFIG_CONTENT "#cmakedefine01 PTL_HAVE_SYS_SIGNAME\n")

check_cxx_source_compiles("
    #include <signal.h>
    #include <stdio.h>
    int main() { 
        const char * x = sys_sigabbrev[SIGINT];
        printf(\"%s\", x);
    }" 
PTL_HAVE_SYS_SIGABBREV_DECLARED)
string(APPEND CONFIG_CONTENT "#cmakedefine01 PTL_HAVE_SYS_SIGABBREV_DECLARED\n")

check_cxx_source_compiles("
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
string(APPEND CONFIG_CONTENT "#cmakedefine01 PTL_HAVE_SYS_SIGABBREV_UNDECLARED\n")

string(APPEND CONFIG_CONTENT "#define PTL_HAVE_SYS_SIGABBREV PTL_HAVE_SYS_SIGABBREV_UNDECLARED || PTL_HAVE_SYS_SIGABBREV_DECLARED\n")

check_cxx_source_compiles("
    #include <unistd.h>
    #include <stdlib.h>
    
    int main() { 
        int (*p)(char *, int, int) = mkostemps;
    }" 
PTL_HAVE_MKOSTEMPS)
string(APPEND CONFIG_CONTENT "#cmakedefine01 PTL_HAVE_MKOSTEMPS\n")

check_cxx_symbol_exists(lchmod sys/stat.h PTL_HAVE_LCHMOD)
string(APPEND CONFIG_CONTENT "#cmakedefine01 PTL_HAVE_LCHMOD\n")


string(APPEND CONFIG_CONTENT "
#endif
")

string(CONFIGURE ${CONFIG_CONTENT} CONFIG_CONTENT)

file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generated/inc/ptl/config.h CONTENT ${CONFIG_CONTENT})



