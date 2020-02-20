if(WIN32)
    set(MINGW_DIR "${SDL2_IMAGE_INCLUDE_DIR}/../.."
            CACHE PATH "Where MinGW-w64 is installed")
endif(WIN32)

define_property(GLOBAL PROPERTY GE211_CLIENT_NAME
        BRIEF_DOCS "Name of the GE211 client installer"
        FULL_DOCS  "Name of the GE211 client installer")


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


# Precondition: GLOBAL PROPERTY CPACK_PACKAGE_NAME is set.
macro(_set_ge211_installer_vars)
    get_property(CPACK_PACKAGE_NAME GLOBAL PROPERTY GE211_CLIENT_NAME)
    if(NOT CPACK_PACKAGE_NAME)
        message(FATAL_ERROR "ge211_installer_add: oops.")
    endif()

    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-dist")
    set(CPACK_STRIP_FILES       FALSE)

    if(APPLE)
        set(CPACK_GENERATOR DragNDrop)
        set(CLIENT_INSTALLER "${CPACK_PACKAGE_FILE_NAME}.dmg")
    elseif(WIN32)
        set(CPACK_GENERATOR ZIP)
        set(CLIENT_INSTALLER "${CPACK_PACKAGE_FILE_NAME}.zip")
    else()
        message(FATAL_ERROR
                "Creating an installer is not supported on your platform")
    endif()
endmacro(_set_ge211_installer_vars)


function(_initialize_ge211_installer)
    cmake_parse_arguments(pa "" "NAME" "" ${ARGN})

    get_property(already_initialized GLOBAL PROPERTY GE211_CLIENT_NAME SET)
    if(already_initialized)
        get_property(old_name GLOBAL PROPERTY GE211_CLIENT_NAME)
        if(pa_NAME AND NOT "${pa_NAME}" STREQUAL "${old_name}")
            message(WARNING "Cannot name installer ‘${pa_NAME}’"
                    " because there is already an installer named"
                    " ‘${old_name}’.")
        endif()
        return()
    endif()

    if(NOT pa_NAME)
        set(pa_NAME "${CMAKE_PROJECT_NAME}")
    endif()
    set_property(GLOBAL PROPERTY GE211_CLIENT_NAME "${pa_NAME}")

    _set_ge211_installer_vars()

    add_custom_target("${CLIENT_INSTALLER}")
    add_custom_command(TARGET "${CLIENT_INSTALLER}"
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
    add_dependencies(${CLIENT_INSTALLER} ${target})
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
