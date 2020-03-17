include(GNUInstallDirs)

install(TARGETS         ge211
                        utf8-cpp
        EXPORT          Ge211Config)

install(DIRECTORY       share/
        DESTINATION     ${CMAKE_INSTALL_DATADIR}/Ge211)

install(EXPORT          Ge211Config
        DESTINATION     ${CMAKE_INSTALL_DATADIR}/Ge211/cmake)

