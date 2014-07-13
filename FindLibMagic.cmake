# - Try to find libmagic header and library
#
# Usage of this module as follows:
#
#     find_package(LibMagic)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  LIBMAGIC_ROOT_DIR         Set this variable to the root installation of
#                            libmagic if the module has problems finding the
#                            proper installation path.
#
# Variables defined by this module:
#
#  LIBMAGIC_FOUND              System has libmagic, magic.h, and file
#  LIBMAGIC_FILE_EXE           Path to the 'file' command
#  LIBMAGIC_VERSION            Version of libmagic
#  LIBMAGIC_LIBRARY            The libmagic library
#  LIBMAGIC_INCLUDE_DIR        The location of magic.h

find_path(LIBMAGIC_ROOT_DIR
    NAMES include/magic.h
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # the static version of the library is preferred on OS X for the
    # purposes of making packages (libmagic doesn't ship w/ OS X)
    set(LIBMAGIC_NAMES libmagic.a magic)
else ()
    set(LIBMAGIC_NAMES magic)
endif ()

find_file(LIBMAGIC_FILE_EXE
    NAMES file
    HINTS ${LIBMAGIC_ROOT_DIR}/bin
)

find_library(LIBMAGIC_LIBRARY
    NAMES ${LIBMAGIC_NAMES}
    HINTS ${LIBMAGIC_ROOT_DIR}/lib
)

find_path(LIBMAGIC_INCLUDE_DIR
    NAMES magic.h
    HINTS ${LIBMAGIC_ROOT_DIR}/include
)

if (LIBMAGIC_FILE_EXE)
    execute_process(COMMAND "${LIBMAGIC_FILE_EXE}" --version
                    ERROR_VARIABLE  LIBMAGIC_VERSION
                    OUTPUT_VARIABLE LIBMAGIC_VERSION)
    string(REGEX REPLACE "^file-([0-9.]+).*$" "\\1"
           LIBMAGIC_VERSION "${LIBMAGIC_VERSION}")
    message(STATUS "libmagic version: ${LIBMAGIC_VERSION}")
else ()
    set(LIBMAGIC_VERSION NOTFOUND)
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibMagic DEFAULT_MSG
    LIBMAGIC_LIBRARY
    LIBMAGIC_INCLUDE_DIR
    LIBMAGIC_FILE_EXE
    LIBMAGIC_VERSION
)

mark_as_advanced(
    LIBMAGIC_ROOT_DIR
    LIBMAGIC_FILE_EXE
    LIBMAGIC_VERSION
    LIBMAGIC_LIBRARY
    LIBMAGIC_INCLUDE_DIR
)
