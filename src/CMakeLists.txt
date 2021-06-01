add_library(ge211
        ge211_base.cxx
        ge211_color.cxx
        ge211_engine.cxx
        ge211_event.cxx
        ge211_error.cxx
        ge211_geometry.cxx
        ge211_audio.cxx
        ge211_random.cxx
        ge211_render.cxx
        ge211_resource.cxx
        ge211_session.cxx
        ge211_sprites.cxx
        ge211_window.cxx)

set_target_properties(ge211
        PROPERTIES
        CXX_STANDARD                    14
        CXX_STANDARD_REQUIRED           On
        CXX_EXTENSIONS                  Off
        VERSION                         ${PROJECT_VERSION})


if(NOT EMSCRIPTEN)
    set(ge211_default_resource_path
            "${CMAKE_CURRENT_SOURCE_DIR}/../Resources/"
            "${CMAKE_INSTALL_FULL_DATADIR}/ge211/")
else()
    set(ge211_default_resource_path "Resources/")
    set(EM_IMAGE_FORMATS
            bmp gif jpg png svg webp
            CACHE STRING
            "list of image formats to support under emscripten")
    set(EM_MIXER_FORMATS
            mp3 ogg wav
            CACHE STRING
            "list of audio formats to support under emscripten")
    set(GE211_EMCC_PORTS
            -sUSE_SDL=2
            -sUSE_SDL_IMAGE=2
            -sSDL2_IMAGE_FORMATS=[$<JOIN:${EM_IMAGE_FORMATS},$<COMMA>>]
            -sUSE_SDL_MIXER=2
            -sSDL2_MIXER_FORMATS=[$<JOIN:${EM_MIXER_FORMATS},$<COMMA>>]
            -sUSE_SDL_TTF=2)
    target_compile_options(ge211 PRIVATE ${GE211_EMCC_PORTS})
    target_link_options(ge211 PUBLIC ${GE211_EMCC_PORTS})
endif()

if(MSVC)
    target_compile_options(ge211 PRIVATE /W4)
else(MSVC)
    target_compile_options(ge211 PRIVATE -Wall -Wextra -pedantic)
endif(MSVC)

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

set(GE211_RESOURCE_PATH ${ge211_default_resource_path}
        CACHE STRING "Where to search for GE211 resource files.")

target_compile_definitions(ge211
        PRIVATE
        GE211_RESOURCES=\"$<JOIN:${GE211_RESOURCE_PATH},\"$<COMMA>\">\")
