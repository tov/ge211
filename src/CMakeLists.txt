add_library(ge211
        base.cxx
        color.cxx
        engine.cxx
        event.cxx
        error.cxx
        geometry.cxx
        audio.cxx
        random.cxx
        render.cxx
        resource.cxx
        session.cxx
        sprites.cxx
        window.cxx)

set_target_properties(ge211
        PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED On
        CXX_EXTENSIONS Off
        VERSION ${PROJECT_VERSION})

target_compile_features(ge211 PUBLIC
    cxx_std_14)

target_compile_options(ge211 PRIVATE
        ${SDL2_COMPILE_FLAGS}
        ${SDL2_IMAGE_COMPILE_FLAGS}
        ${SDL2_MIXER_COMPILE_FLAGS}
        ${SDL2_TTF_COMPILE_FLAGS})
target_link_options(ge211 PUBLIC
        ${SDL2_LINK_FLAGS}
        ${SDL2_IMAGE_LINK_FLAGS}
        ${SDL2_MIXER_LINK_FLAGS}
        ${SDL2_TTF_LINK_FLAGS})

if (MSVC)
    target_compile_options(ge211 PRIVATE /W4)
else (MSVC)
    target_compile_options(ge211 PRIVATE -Wall -Wextra -pedantic)
endif (MSVC)

include(GNUInstallDirs)

target_include_directories(ge211
        SYSTEM INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        ${SDL2_INCLUDE_DIRS})

target_include_directories(ge211
        PRIVATE
        ../include
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_IMAGE_INCLUDE_DIRS}
        ${SDL2_MIXER_INCLUDE_DIRS}
        ${SDL2_TTF_INCLUDE_DIRS})

target_link_libraries(ge211
        PUBLIC
        ${SDL2_LIBRARIES}
        PRIVATE
        ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_MIXER_LIBRARIES}
        ${SDL2_TTF_LIBRARIES}
        utf8-cpp)

set(GE211_DEFAULT_RESOURCE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/../Resources/"
        "${CMAKE_INSTALL_FULL_DATADIR}/ge211/Resources/"
        "/Resources/")

set(GE211_RESOURCE_PATH
        ${GE211_DEFAULT_RESOURCE_PATH}
        CACHE STRING
        "Where to search for GE211 resource files.")

target_compile_definitions(ge211
        PRIVATE
        GE211_RESOURCES=\"$<JOIN:${GE211_RESOURCE_PATH},\"$<COMMA>\">\")
