# Copyright (c) 2023, Eugene Gershnik
# SPDX-License-Identifier: BSD-3-Clause


if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(FetchContent)

set(DECLARED_DEPENDENCIES "")

FetchContent_Declare(doctest
    URL  https://raw.githubusercontent.com/doctest/doctest/v2.5.2/doctest/doctest.h
    DOWNLOAD_NO_EXTRACT TRUE
)

list(APPEND DECLARED_DEPENDENCIES doctest)

set(FMT_INSTALL OFF)

FetchContent_Declare(fmt
    GIT_REPOSITORY  https://github.com/fmtlib/fmt
    GIT_TAG         12.1.0
    GIT_SHALLOW     TRUE
    GIT_SUBMODULES_RECURSE FALSE
)
list(APPEND DECLARED_DEPENDENCIES fmt)

FetchContent_MakeAvailable(${DECLARED_DEPENDENCIES})

if ("${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "MSVC")
    set(FRONT_MSVC True)
else()
    set(FRONT_MSVC False)
endif()

set (CLANG_CL "$<AND:$<BOOL:${FRONT_MSVC}>,$<CXX_COMPILER_ID:Clang>>")
set (CLANG_NORMAL "$<AND:$<NOT:$<BOOL:${FRONT_MSVC}>>,$<CXX_COMPILER_ID:Clang>>")

add_executable(tests)

set_target_properties(tests PROPERTIES 
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
)

target_link_libraries(tests 
PRIVATE
    ptl::ptl
    fmt::fmt
    $<$<BOOL:${WIN32}>:ws2_32.lib>
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<NOT:$<BOOL:${ANDROID}>>>:pthread>
    $<$<PLATFORM_ID:SunOS>:socket>
)

target_include_directories(tests
PRIVATE
    ${doctest_SOURCE_DIR}
)

target_compile_options(tests
PRIVATE
    $<$<CXX_COMPILER_ID:AppleClang,Clang>:-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion> 
    $<${CLANG_NORMAL}:-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion> 
    $<${CLANG_CL}:/W4 /WX -Wpedantic -Wconversion -Wsign-conversion> 
    
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion
        -Wno-unknown-pragmas  #the whole point of pragmas it to be potentially unknown!
    >
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8 /Zc:preprocessor /W4 /WX /EHsc
        /wd5030 #attribute 'x' is not recognized
        /wd5285 #cannot declare a specialization for 'std::tuple' (in doctest)
    >
    $<$<BOOL:${EMSCRIPTEN}>:-sNO_DISABLE_EXCEPTION_CATCHING>
)

target_link_options(tests 
PRIVATE
    $<$<BOOL:${EMSCRIPTEN}>:-sNO_DISABLE_EXCEPTION_CATCHING>
)

target_compile_definitions(tests
PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)

target_sources(tests
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

if (${CMAKE_SYSTEM_NAME} STREQUAL Android)
    set(ANDROID_TEST_DIR /data/local/tmp/ptl_test)
    set(ANDROID_SDK_DIR ${CMAKE_ANDROID_NDK}/../..)
    set(ADB ${ANDROID_SDK_DIR}/platform-tools/adb)

    if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
        set(ANDROID_LD_LIBRARY_PATH /apex/com.android.art/lib:/apex/com.android.runtime/lib)
    else()
        set(ANDROID_LD_LIBRARY_PATH /apex/com.android.art/lib64:/apex/com.android.runtime/lib64)
    endif()
endif()

set(TEST_SRCIPT 
"
cmake_path(GET EXECUTABLE FILENAME exefilename)

macro(run)
    execute_process(COMMAND \${ARGV} COMMAND_ERROR_IS_FATAL ANY)
endmacro()
")

if (${CMAKE_SYSTEM_NAME} STREQUAL Android)
    string(APPEND TEST_SRCIPT "
run(${ADB} shell mkdir -p ${ANDROID_TEST_DIR}/data)
run(${ADB} push \"\${EXECUTABLE}\" ${ANDROID_TEST_DIR})
run(${ADB} shell \"cd ${ANDROID_TEST_DIR} && LD_LIBRARY_PATH=${ANDROID_LD_LIBRARY_PATH} ./\${exefilename} -ni -fc\")
")
elseif (CMAKE_CROSSCOMPILING)
    string(APPEND TEST_SRCIPT "
run(${CMAKE_CROSSCOMPILING_EMULATOR} \${EXECUTABLE} -ni -fc)
")
else()
    string(APPEND TEST_SRCIPT "
run(\${EXECUTABLE} -ni -fc)
")  
endif()

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/run-test.cmake ${TEST_SRCIPT})

add_test(
    NAME "test"
    COMMAND ${CMAKE_COMMAND} 
        -DEXECUTABLE=$<TARGET_FILE:tests>
        -P ${CMAKE_CURRENT_BINARY_DIR}/run-test.cmake
)

