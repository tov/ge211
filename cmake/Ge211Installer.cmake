include(GNUInstallDirs)

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
        DESTINATION     ${CMAKE_INSTALL_DATADIR}/cmake/Ge211)

export( TARGETS         ge211
        FILE            Ge211Config.cmake)

