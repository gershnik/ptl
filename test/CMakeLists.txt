
find_package (Python3 COMPONENTS Interpreter)

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
)

target_sources(test
PRIVATE
    test.cpp
    test_file_descriptor.cpp
    test_spawn.cpp
    test_signal.cpp
    test_users.cpp
)

set(TEST_COMMAND "")
set(TEST_DEPS "")

list(APPEND TEST_COMMAND COMMAND test)
list(APPEND TEST_DEPS test)

add_custom_target(run-test 
    DEPENDS ${TEST_DEPS}
    ${TEST_COMMAND}
)



