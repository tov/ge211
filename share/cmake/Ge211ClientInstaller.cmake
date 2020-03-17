# Generates an installer for a GE211 client program

if(WIN32)
    set(MINGW_DIR "${SDL2_IMAGE_INCLUDE_DIR}/../.."
            CACHE PATH "Where MinGW-w64 is installed")
endif(WIN32)

# The name of the global property that controls the name of
# the client installer.
set(name_prop_name GE211_INSTALLER_NAME)

define_property(GLOBAL PROPERTY ${name_prop_name}
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

macro(_ret_if_set outvar)
    if(${outvar})
        set(${${outvar}} ${ARGN} PARENT_SCOPE)
    endif()
endmacro()

# Sets installation-related variables to OS-dependent values.
function(_Ge211ClientInstaller_os_set pkg target)
    set(_one
        GENERATOR
        PACKAGE_NAME
        PACKAGE_FILE_NAME
        PACKAGE_FILE_NAME_EXT
        EXECUTABLE)
    cmake_parse_arguments(pa "" "${_one}" "" ${ARGN})

    if(APPLE)
        set(os          mac)
        set(gen         DragNDrop)
        set(ext         dmg)
        set(exe         "${CMAKE_BINARY_DIR}/${target}.app")
    elseif(WIN32)
        set(os          win)
        set(gen         ZIP)
        set(ext         zip)
        set(exe         "\${CMAKE_INSTALL_FULL_BINDIR}/${target}.exe")
    else()
        message(FATAL_ERROR
                "Creating an installer is not supported on your platform")
    endif()

    _ret_if_set(pa_GENERATOR              ${gen})
    _ret_if_set(pa_PACKAGE_NAME           ${pkg})
    _ret_if_set(pa_PACKAGE_FILE_NAME      ${pkg}-${os})
    _ret_if_set(pa_PACKAGE_FILE_NAME_EXT  ${pkg}-${os}.${ext})
    _ret_if_set(pa_EXECUTABLE             ${exe})
endfunction()

# Sets up the final installation step in an OS-dependent manner.
function(_Ge211ClientInstaller_os_install executable)
    if(APPLE)
        install(CODE "
                set(BU_CHMOD_BUNDLE_ITEMS TRUE)
                fixup_bundle(\"${executable}\" \"\" \"\")
                ")
        install(TARGETS
                BUNDLE DESTINATION      .)
    elseif(WIN32)
        install(TARGETS
                RESOURCE DESTINATION    Resources)
        install(CODE "
                list(APPEND gp_cmd_paths \"${MINGW_DIR}/bin\")
                fixup_bundle(\"${executable}\" \"\" \"\")
                ")
    endif()
endfunction(_Ge211ClientInstaller_os_install)

# Creates the installer target and the attached install script.
# Precondition: Hasn't been called yet.
function(_Ge211ClientInstaller_add_target name)
    add_custom_target("${name}")
    add_custom_command(TARGET "${name}"
            COMMAND "${CMAKE_CPACK_COMMAND}"
            COMMENT "Running CPack, please wait...")

    install(CODE "include(BundleUtilities)")

    if(WIN32)
        glob_libs(Plugins "${MINGW_DIR}/bin" .dll jpeg png tiff webp)
        install(FILES ${Plugins} TYPE LIB)
    endif()
endfunction(_Ge211ClientInstaller_add_target)


# Sets up the given target for installation along with the given
# resource files.
function(add_ge211_installer target)
    set(_zero)
    set(_one   NAME)
    set(_many  RESOURCE)
    cmake_parse_arguments(pa "${_zero}" "${_one}" "${_many}" ${ARGN})

    if(pa_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR
            "add_ge211_installer: These keywords require values: "
            ${pa_KEYWORDS_MISSING_VALUES})
    endif()

    if(pa_UNPARSED_ARGUMENTS)
        message(WARNING
            "add_ge211_installer: "
            "Extra arguments treated as resource files: "
            ${pa_UNPARSED_ARGUMENTS})
        list(APPEND pa_RESOURCE ${pa_UNPARSED_ARGUMENTS})
    endif()

    if(NOT pa_NAME)
        set(pa_NAME "${CMAKE_PROJECT_NAME}")
    endif()

    _Ge211ClientInstaller_os_set("${pa_NAME}" "${target}"
            GENERATOR               CPACK_GENERATOR
            PACKAGE_NAME            CPACK_PACKAGE_NAME
            PACKAGE_FILE_NAME       CPACK_PACKAGE_FILE_NAME
            PACKAGE_FILE_NAME_EXT   installer_target
            EXECUTABLE              executable)

    _Ge211ClientInstaller_add_target(${installer_target})
    add_dependencies(${installer_target} ${target})

    set(CPACK_PACKAGE_EXECUTABLES "${target}" "${target}")
    include(CPack)

    target_sources(${target} PUBLIC ${pa_RESOURCE})
    set_target_properties(${target} PROPERTIES
            MACOSX_BUNDLE               TRUE
            MACOSX_BUNDLE_BUNDLE_NAME   "${target}"
            RESOURCE                    "${pa_RESOURCE}")

    _Ge211ClientInstaller_os_install(${executable})
endfunction(add_ge211_installer)
