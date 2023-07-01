cmake_minimum_required(VERSION 3.25)

project(ptl VERSION 0.1)

include(cmake/detect.cmake)

set(INCDIR ${CMAKE_CURRENT_SOURCE_DIR}/inc)
set(GEN_INCDIR ${CMAKE_CURRENT_BINARY_DIR}/generated/inc)
set(CMAKEDIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)


configure_file(${CMAKEDIR}/config.h.in ${GEN_INCDIR}/ptl/config.h)

add_library(ptl INTERFACE)

target_compile_features(ptl INTERFACE cxx_std_20)


target_include_directories(ptl
INTERFACE
    $<BUILD_INTERFACE:${INCDIR}>
    $<BUILD_INTERFACE:${GEN_INCDIR}>
    $<INSTALL_INTERFACE:include>  # <prefix>/include
)

set(PUBLIC_HEADERS 
    ${GEN_INCDIR}/ptl/config.h

    ${INCDIR}/ptl/core.h
    ${INCDIR}/ptl/identity.h
    ${INCDIR}/ptl/file.h
    ${INCDIR}/ptl/process.h
    ${INCDIR}/ptl/signal.h
    ${INCDIR}/ptl/spawn.h
    ${INCDIR}/ptl/users.h
)

set(CMAKE_FILES 
    ${CMAKEDIR}/config.h.in
    ${CMAKEDIR}/detect.cmake
    ${CMAKEDIR}/install.cmake
    ${CMAKEDIR}/ptl-config.cmake.in
)

target_sources(ptl
INTERFACE 
    FILE_SET HEADERS BASE_DIRS ${INCDIR} ${GEN_INCDIR} FILES
        ${PUBLIC_HEADERS}
)

if (PROJECT_IS_TOP_LEVEL)

    set(PROJECT_FILES "")

    foreach(dir ${PUBLIC_HEADERS})
        list(APPEND PROJECT_FILES $<BUILD_INTERFACE:${dir}>)
    endforeach()

    foreach(dir ${CMAKE_FILES})
        list(APPEND PROJECT_FILES $<BUILD_INTERFACE:${dir}>)
    endforeach()

    target_sources(ptl
    INTERFACE 
        ${PROJECT_FILES}
    )

endif()


add_library(ptl::ptl ALIAS ptl)


if (PROJECT_IS_TOP_LEVEL)

    include(cmake/install.cmake)

    add_subdirectory(test)

endif()
