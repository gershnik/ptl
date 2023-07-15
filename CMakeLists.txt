# Copyright (c) 2023, Eugene Gershnik
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.25)

file(READ VERSION PTL_VERSION)
if (NOT PTL_VERSION)
    message(FATAL_ERROR "Cannot determine library version (VERSION file not found)")
endif()
string(STRIP ${PTL_VERSION} PTL_VERSION)

project(ptl VERSION ${PTL_VERSION} LANGUAGES CXX)

include(cmake/config.cmake)

set(INCDIR ${CMAKE_CURRENT_SOURCE_DIR}/inc)
set(GEN_INCDIR ${CMAKE_CURRENT_BINARY_DIR}/generated/inc)
set(CMAKEDIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)


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
    ${INCDIR}/ptl/ptl.h
    ${INCDIR}/ptl/identity.h
    ${INCDIR}/ptl/file.h
    ${INCDIR}/ptl/process.h
    ${INCDIR}/ptl/signal.h
    ${INCDIR}/ptl/socket.h
    ${INCDIR}/ptl/spawn.h
    ${INCDIR}/ptl/system.h
    ${INCDIR}/ptl/users.h
)

set(CMAKE_FILES 
    ${CMAKEDIR}/config.cmake
    ${CMAKEDIR}/install.cmake
    ${CMAKEDIR}/ptl-config.cmake.in
    ${CMAKEDIR}/ptl.pc.in
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
