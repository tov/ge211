cmake_minimum_required(VERSION 3.3)
project(ge211 CXX)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

add_subdirectory(3rdparty/utf8-cpp)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)

if (WIN32)
    set(MINGW_DIR "${SDL2_IMAGE_INCLUDE_DIR}/../.."
            CACHE PATH "Where MinGW-w64 is installed")
endif (WIN32)

add_library(ge211
        src/ge211.h
        src/ge211_forward.h
        src/ge211_time.h
        src/ge211_util.h
        src/ge211_base.cpp
        src/ge211_color.cpp
        src/ge211_engine.cpp
        src/ge211_event.cpp
        src/ge211_error.cpp
        src/ge211_geometry.cpp
        src/ge211_audio.cpp
        src/ge211_random.cpp
        src/ge211_render.cpp
        src/ge211_resource.cpp
        src/ge211_session.cpp
        src/ge211_sprites.cpp
        src/ge211_window.cpp)
set_property(TARGET ge211 PROPERTY CXX_STANDARD 14)
set_property(TARGET ge211 PROPERTY CXX_STANDARD_REQUIRED On)

target_compile_definitions(ge211 PRIVATE
        GE211_RESOURCES="${CMAKE_CURRENT_SOURCE_DIR}/Resources/")

target_include_directories(ge211 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories(ge211 PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(ge211 PRIVATE ${SDL2_IMAGE_INCLUDE_DIRS})
target_include_directories(ge211 PRIVATE ${SDL2_MIXER_INCLUDE_DIRS})
target_include_directories(ge211 PRIVATE ${SDL2_TTF_INCLUDE_DIRS})

target_link_libraries(ge211 ${SDL2_LIBRARIES})
target_link_libraries(ge211 ${SDL2_IMAGE_LIBRARIES})
target_link_libraries(ge211 ${SDL2_MIXER_LIBRARIES})
target_link_libraries(ge211 ${SDL2_TTF_LIBRARIES})
target_link_libraries(ge211 utf8-cpp)

add_subdirectory(examples/)

# Given a base path to search in, a file extension, and a list of library
# names, searches for the libraries (using a glob) and saves the resulting
# list of paths in ${output_var}.
function(glob_libs output_var base_path ext)
    set(acc "")
    foreach(lib ${ARGN})
        file(GLOB next "${base_path}/lib${lib}*${ext}")
        list(APPEND acc "${next}")
    endforeach()
    set(${output_var} "${acc}" PARENT_SCOPE)
endfunction(glob_libs)

# Sets up the given target for installation along with the given
# resource files.
function(add_to_ge211_installer target resource_files)
    if (APPLE)
        set_target_properties(${target} PROPERTIES
                MACOSX_BUNDLE TRUE
                MACOSX_FRAMEWORK_IDENTIFIER org.northwestern.eecs.jesse.eecs211
                RESOURCE "${resource_files}")
        set(app "${CMAKE_BINARY_DIR}/${target}.app")
        set(dirs "${SDL2_IMAGE_INCLUDE_DIR}/../Frameworks"
                 "${SDL2_MIXER_INCLUDE_DIR}/../Frameworks"
                 "${SDL2_TTF_INCLUDE_DIR}/../Frameworks")
        install(CODE "include(BundleUtilities)
                      fixup_bundle(\"${app}\" \"\" \"${dirs}\")")
        install(TARGETS ${target} BUNDLE DESTINATION .)
    elseif (WIN32)
        set(app "\${CMAKE_INSTALL_PREFIX}/bin/${target}.exe")
        set(bindir "${MINGW_DIR}/bin")
        glob_libs(Plugins "${bindir}" .dll jpeg png tiff webp)
        install(TARGETS ${target} DESTINATION bin)
        install(FILES ${resource_files} DESTINATION Resources)
        install(FILES ${Plugins} DESTINATION bin)
        install(CODE "include(BundleUtilities)
                      list(APPEND gp_cmd_paths \"${bindir}\")
                      fixup_bundle(\"${app}\" \"\" \"${bindir}\")")
    else ()
        message(FATAL_ERROR
                "Creating an installer is not supported on your platform")
    endif ()
endfunction(add_to_ge211_installer)

# Creates a target for a platform-dependent installer for an executable
# (given by the name of its target) and some resource files.
function(setup_ge211_installer target resource_files)
    add_to_ge211_installer(${target} "${resource_files}")

    if (APPLE)
        set(dist_ext dmg)
        set(CPACK_GENERATOR "DragNDrop")
    else()
        set(dist_ext zip)
        set(CPACK_GENERATOR "ZIP")
    endif()

    set(CPACK_PACKAGE_EXECUTABLES "${target};${target}")
    set(CPACK_PACKAGE_FILE_NAME "${target}-dist")
    include(CPack)
    add_custom_target("${target}-dist.${dist_ext}")
    add_custom_command(TARGET "${target}-dist.${dist_ext}"
            COMMAND "${CMAKE_CPACK_COMMAND}"
            COMMENT "Running CPack, please wait..."
            DEPENDS ${target})
endfunction(setup_ge211_installer)
