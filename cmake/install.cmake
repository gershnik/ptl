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
