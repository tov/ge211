include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(config_version_cmake
        ${CMAKE_CURRENT_BINARY_DIR}/Ge211/Ge211ConfigVersion.cmake)
set(config_install_dir
        ${CMAKE_INSTALL_DATADIR}/cmake/Ge211)

install(TARGETS         ge211
                        utf8-cpp
        EXPORT          Ge211Config
        ARCHIVE         DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY         DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME         DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY       include/
                        ${CMAKE_CURRENT_BINARY_DIR}/include/
        DESTINATION     ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING  PATTERN "*.hxx")

install(DIRECTORY       Resources/
        DESTINATION     ${CMAKE_INSTALL_DATADIR}/ge211)

install(EXPORT          Ge211Config
        DESTINATION     ${config_install_dir})

install(FILES           ${config_version_cmake}
        DESTINATION     ${config_install_dir})

export( TARGETS         ge211
        FILE            Ge211Config.cmake)

write_basic_package_version_file(${config_version_cmake}
        VERSION         ${CMAKE_PROJECT_VERSION}
        COMPATIBILITY   SameMajorVersion)

