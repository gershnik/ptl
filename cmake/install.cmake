# Copyright (c) 2023, Eugene Gershnik
# SPDX-License-Identifier: BSD-3-Clause

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS ptl EXPORT ptl FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT ptl NAMESPACE ptl:: FILE ptl-exports.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/ptl)


configure_package_config_file(
        ${CMAKE_CURRENT_LIST_DIR}/ptl-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/ptl-config.cmake
    INSTALL_DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/ptl
)

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/ptl-config-version.cmake
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/ptl-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/ptl-config-version.cmake
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/ptl
)

file(RELATIVE_PATH FROM_PCFILEDIR_TO_PREFIX ${CMAKE_INSTALL_FULL_DATAROOTDIR}/ptl ${CMAKE_INSTALL_PREFIX})
string(REGEX REPLACE "/+$" "" FROM_PCFILEDIR_TO_PREFIX "${FROM_PCFILEDIR_TO_PREFIX}") 

configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/ptl.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/ptl.pc
    @ONLY
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/ptl.pc
    DESTINATION
        ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig
)