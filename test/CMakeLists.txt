# Copyright (c) 2023, Eugene Gershnik
# SPDX-License-Identifier: BSD-3-Clause

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(CheckCXXSourceCompiles)

check_cxx_source_compiles("
    #if __cpp_lib_format < 202110L
        #error std::format not sufficiently supported
    #endif" 
HAVE_STD_FORMAT)

if (NOT HAVE_STD_FORMAT)

    include(FetchContent)
    include(ExternalProject)

    set(DECLARED_DEPENDENCIES "")

    FetchContent_Declare(fmt
        GIT_REPOSITORY  https://github.com/fmtlib/fmt
        GIT_TAG         9.1.0
        GIT_SHALLOW     TRUE
        GIT_SUBMODULES_RECURSE FALSE
    )
    list(APPEND DECLARED_DEPENDENCIES fmt)

    FetchContent_Declare(Catch2
        GIT_REPOSITORY  https://github.com/catchorg/Catch2.git
        GIT_TAG         v3.3.2
        GIT_SHALLOW     TRUE
        GIT_SUBMODULES_RECURSE FALSE
    )
    list(APPEND DECLARED_DEPENDENCIES Catch2)

    FetchContent_MakeAvailable(${DECLARED_DEPENDENCIES})

endif()

add_executable(test)

set_target_properties(test PROPERTIES 
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
)

target_link_libraries(test 
PRIVATE
    ptl::ptl
    fmt::fmt
    Catch2::Catch2
    $<$<BOOL:${WIN32}>:ws2_32.lib>
)

target_compile_options(test
PRIVATE
    $<$<CXX_COMPILER_ID:AppleClang,Clang>:-Wall -Wextra -Wpedantic> 
    
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic
        -Wno-unknown-pragmas  #the whole point of pragmas it to be potentially unknown!
    >
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8 /Zc:preprocessor /W4 /WX /EHsc
        /wd5030 #attribute 'x' is not recognized
    >
)

target_compile_definitions(test
PRIVATE
    CATCH_AMALGAMATED_CUSTOM_MAIN
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)

target_sources(test
PRIVATE
    common.cpp
    test.cpp
    test_identity.cpp
    test_errors.cpp
    test_file.cpp
    test_spawn.cpp
    test_signal.cpp
    test_socket.cpp
    test_users.cpp
    test_system.cpp
)

set(TEST_COMMAND "")
set(TEST_DEPS "")

if (${CMAKE_SYSTEM_NAME} STREQUAL Android)

    set(ANDROID_TEST_DIR /data/local/tmp/sys_string_test)
    set(ANDROID_SDK_DIR ${CMAKE_ANDROID_NDK}/../..)
    set(ADB ${ANDROID_SDK_DIR}/platform-tools/adb)

    if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
        set(ANDROID_LD_LIBRARY_PATH /apex/com.android.art/lib:/apex/com.android.runtime/lib)
    else()
        set(ANDROID_LD_LIBRARY_PATH /apex/com.android.art/lib64:/apex/com.android.runtime/lib64)
    endif()

    list(APPEND TEST_COMMAND COMMAND ${ADB} shell mkdir -p ${ANDROID_TEST_DIR}/data)
    list(APPEND TEST_COMMAND COMMAND ${ADB} push test ${ANDROID_TEST_DIR})
    list(APPEND TEST_COMMAND COMMAND ${ADB} shell \"cd ${ANDROID_TEST_DIR} && LD_LIBRARY_PATH=${ANDROID_LD_LIBRARY_PATH} ./test\")

else()

    list(APPEND TEST_COMMAND COMMAND test)
    
endif()

list(APPEND TEST_DEPS test)

add_custom_target(run-test 
    DEPENDS ${TEST_DEPS}
    ${TEST_COMMAND}
)



