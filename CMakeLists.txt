cmake_minimum_required(VERSION 3.13)
project(ge211
        VERSION 2020.1.1
        DESCRIPTION "A student game engine"
        LANGUAGES CXX)

###
### DEPENDENCIES
###

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

add_subdirectory(3rdparty/utf8-cpp)

find_package(SDL2       REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf   REQUIRED)

###
### MAIN LIBRARY SETUP
###

add_library(ge211
        include/ge211.hxx
        include/ge211_forward.hxx
        include/ge211_time.hxx
        include/ge211_util.hxx
        src/ge211_base.cxx
        src/ge211_color.cxx
        src/ge211_engine.cxx
        src/ge211_event.cxx
        src/ge211_error.cxx
        src/ge211_geometry.cxx
        src/ge211_audio.cxx
        src/ge211_random.cxx
        src/ge211_render.cxx
        src/ge211_resource.cxx
        src/ge211_session.cxx
        src/ge211_sprites.cxx
        src/ge211_window.cxx)

set_target_properties(ge211
    PROPERTIES
        CXX_STANDARD                    14
        CXX_STANDARD_REQUIRED           On
        CXX_EXTENSIONS                  Off
        VERSION                         ${PROJECT_VERSION}
        INTERFACE_INCLUDE_DIRECTORIES   "${SDL2_INCLUDE_DIRS}")

if(MSVC)
    target_compile_options(ge211 PRIVATE /W4)
else(MSVC)
    target_compile_options(ge211 PRIVATE -Wall -Wextra -pedantic)
endif(MSVC)

target_include_directories(ge211
    SYSTEM INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${SDL2_INCLUDE_DIRS})

target_include_directories(ge211
    PRIVATE
        include
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

SET(GE211_RESOURCE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/"
        "${CMAKE_INSTALL_FULL_DATADIR}/ge211/"
        CACHE STRING
        "Where to search for GE211 resource files.")

target_compile_definitions(ge211 PRIVATE
    -DGE211_RESOURCES="$<JOIN:${GE211_RESOURCE_PATH},\"$<COMMA>\">")

add_subdirectory(examples/installed/)

###
### LIBRARY INSTALLATION
###

include(GNUInstallDirs)

install(TARGETS ge211 utf8-cpp EXPORT Ge211Config
        ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY Resources/ DESTINATION ${CMAKE_INSTALL_DATADIR}/ge211)

install(EXPORT Ge211Config
        DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/Ge211)
export(TARGETS ge211 FILE Ge211Config.cmake)

###
### CLIENT PROGRAM INSTALLER
###

if (WIN32)
    set(MINGW_DIR "${SDL2_IMAGE_INCLUDE_DIR}/../.."
            CACHE PATH "Where MinGW-w64 is installed")
endif (WIN32)

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

define_property(GLOBAL PROPERTY GE211_PACKAGE_NAME
        BRIEF_DOCS "Name of the GE211 installer"
        FULL_DOCS  "Name of the GE211 installer")

# Precondition: GLOBAL PROPERTY CPACK_PACKAGE_NAME is set.
macro(_set_ge211_installer_vars)
    get_property(CPACK_PACKAGE_NAME GLOBAL PROPERTY GE211_PACKAGE_NAME)
    if(NOT CPACK_PACKAGE_NAME)
        message(FATAL_ERROR "ge211_installer_add: oops.")
    endif()

    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-dist")
    set(CPACK_STRIP_FILES       FALSE)

    if(APPLE)
        set(CPACK_GENERATOR DragNDrop)
        set(GE211_INSTALLER "${CPACK_PACKAGE_FILE_NAME}.dmg")
    elseif(WIN32)
        set(CPACK_GENERATOR ZIP)
        set(GE211_INSTALLER "${CPACK_PACKAGE_FILE_NAME}.zip")
    else()
        message(FATAL_ERROR
                "Creating an installer is not supported on your platform")
    endif()
endmacro(_set_ge211_installer_vars)

# Only the first call does anything.
function(_initialize_ge211_installer)
    get_property(already_initialized GLOBAL PROPERTY GE211_PACKAGE_NAME SET)
    if(already_initialized)
        return()
    endif()

    cmake_parse_arguments(pa "" "NAME" "" ${ARGN})
    if(NOT pa_NAME)
        set(pa_NAME "${CMAKE_PROJECT_NAME}")
    endif()
    set_property(GLOBAL PROPERTY GE211_PACKAGE_NAME "${pa_NAME}")

    _set_ge211_installer_vars()

    add_custom_target("${GE211_INSTALLER}")
    add_custom_command(TARGET "${GE211_INSTALLER}"
            COMMAND "${CMAKE_CPACK_COMMAND}"
            COMMENT "Running CPack, please wait...")

    install(CODE "include(BundleUtilities)")

    if(WIN32)
        glob_libs(Plugins "${MINGW_DIR}/bin" .dll jpeg png tiff webp)
        install(FILES ${Plugins} DESTINATION bin)
    endif()
endfunction(_initialize_ge211_installer)

# Creates a target for a platform-dependent installer for an executable
# (given by the name of its target) and some resource files.
function(ge211_installer_name name)
    _initialize_ge211_installer(NAME "${name}")
endfunction(ge211_installer_name)

# Sets up the given target for installation along with the given
# resource files.
function(ge211_installer_add target)
    _initialize_ge211_installer()
    _set_ge211_installer_vars()
    add_dependencies(${GE211_INSTALLER} ${target})
    set(CPACK_PACKAGE_EXECUTABLES "${target}" "${target}")
    include(CPack)

    target_sources(${target} PUBLIC ${ARGN})

    if(APPLE)
        set_target_properties(${target} PROPERTIES
                MACOSX_BUNDLE                TRUE
                MACOSX_FRAMEWORK_IDENTIFIER  edu.northwestern.cs.jesse.cs211
                RESOURCE                     "${ARGN}")
        set(app    "${CMAKE_BINARY_DIR}/${target}.app")
        install(CODE    "set(BU_CHMOD_BUNDLE_ITEMS TRUE)
                         fixup_bundle(\"${app}\" \"\" \"\")")
        install(TARGETS ${target}
                BUNDLE DESTINATION .)
    elseif(WIN32)
        set(exe    "\${CMAKE_INSTALL_PREFIX}/bin/${target}.exe")
        set(bindir "${MINGW_DIR}/bin")
        install(TARGETS ${target}
                RUNTIME DESTINATION bin
                RESOURCES DESTINATION Resources)
        install(CODE    "list(APPEND gp_cmd_paths \"${bindir}\")
                         fixup_bundle(\"${exe}\" \"\" \"\")")
                      #? fixup_bundle(\"${exe}\" \"\" \"${bindir}\")")
    endif()
endfunction(ge211_installer_add)
