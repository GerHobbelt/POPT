cmake_minimum_required(VERSION 3.12)

# Ensure built-in policies from CMake are used, (e.g. improved policies for macOS)
cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})

project(popt
        VERSION 1.19
        DESCRIPTION "Portable library for parsing command line parameters"
        LANGUAGES C
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

option(BUILD_STATIC_LIBS "Build static libraries" OFF)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# this gets git of the Debug and Release directory created by MSVC
if (MSVC_IDE)
        set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIRECTORY}")
	set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIRECTORY}")
	set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIRECTORY}")
	set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIRECTORY}")
	set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIRECTORY}")
	set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIRECTORY}")
endif()

# Set soversion
set(POPT_SOVERSION 0)

# Configurable stuff
option(ENABLE_WERROR "Stop build on warnings" OFF)
option(ENABLE_ASAN "Enable address-sanitizer" OFF)
option(ENABLE_UBSAN "Enable undefined behavior-sanitizer" OFF)

# Set up GNU conventions and standard FHS paths
include(GNUInstallDirs)

# Activate CMake package configuration helpers
include(CMakePackageConfigHelpers)

# set the minimum C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS ON)

# Build Types
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}
	CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel SanitizeAddress RelWithDebInfoStrict"
	FORCE
)

# AddressSanitize
set(CMAKE_C_FLAGS_SANITIZEADDRESS
	"-O0 -g -fsanitize=address -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -fno-omit-frame-pointer"
	CACHE STRING "Flags used by the C compiler during AddressSanitizer builds."
	FORCE
)

set(CMAKE_LINK_FLAGS_SANITIZEADDRESS
	"-fsanitize=address"
	CACHE STRING "Flags used by the linker during AddressSanitizer builds."
	FORCE
)

# RelWithDebInfoStrict
set(CMAKE_C_FLAGS_RELWITHDEBINFOSTRICT
	"-O2 -g -Werror -Wall -D_FORTIFY_SOURCE=3 -fstack-protector-strong -funwind-tables -fasynchronous-unwind-tables -fstack-clash-protection -Werror=return-type -flto=8 -Wdeclaration-after-statement -Wextra -Wmissing-format-attribute -Wmissing-noreturn -Wpointer-arith -Wshadow -Wstrict-prototypes -Wundef -Wunused -Wwrite-strings"
	CACHE STRING "Flags used by the C compiler during strict RelWithDebInfo builds."
	FORCE
)

# Check for necessary symbols and headers
include(CheckSymbolExists)
include(CheckIncludeFile)

function(chkdef func inc)
    string(TOUPPER ${func} FUNC)
    set(HAVENAME HAVE_${FUNC})
    check_symbol_exists(${func} "${inc}" ${HAVENAME})
    if (${HAVENAME})
        add_compile_definitions(${HAVENAME})
    endif ()
endfunction()

function(chkhdr inc req)
    string(MAKE_C_IDENTIFIER ${inc} ID)
    string(TOUPPER ${ID} INC)
    set(HAVENAME HAVE_${INC})
    check_include_file(${inc} ${HAVENAME})
    if (${HAVENAME})
        add_compile_definitions(${HAVENAME})
    endif ()
    if (${req} AND NOT ${HAVENAME})
        message(FATAL_ERROR "required include ${inc} not found")
    endif ()
endfunction()

chkdef(stpcpy string.h)
chkdef(strerror string.h)
chkdef(getuid unistd.h)
chkdef(geteuid unistd.h)
chkdef(mtrace mcheck.h)
chkdef(secure_getenv stdlib.h)
chkdef(__secure_getenv stdlib.h)
chkdef(setreuid unistd.h)
chkdef(setuid unistd.h)
chkdef(stpcpy string.h)
chkdef(strerror string)
chkdef(vasprintf stdio.h)
chkdef(srandom stdlib.h)
chkdef(glob_pattern_p glob.h)
chkdef(mbsrtowcs wchar.h)


set(OPTINCS
        fnmatch.h glob.h langinfo.h libintl.h mcheck.h stdalign.h
)
foreach (inchdr ${OPTINCS})
    chkhdr(${inchdr} FALSE)
endforeach ()

find_package(Iconv)

if (Iconv_FOUND)
    add_compile_definitions(HAVE_ICONV)
endif ()

add_compile_options(-Wall)
if (ENABLE_WERROR)
	add_compile_options(-Werror)
endif()

# Sanitizers
if (ENABLE_ASAN)
	add_compile_options(-fsanitize=address)
	add_link_options(-fsanitize=address)
endif()
if (ENABLE_UBSAN)
        add_compile_options(-fsanitize=undefined)
        add_link_options(-fsanitize=undefined)
endif()
if (ENABLE_ASAN OR ENABLE_UBSAN)
        add_compile_options(-fno-omit-frame-pointer)
endif()


add_subdirectory(src)

if (BUILD_DOC)
    add_subdirectory(doc)
endif ()

if (EXISTS po/popt.pot)
    add_subdirectory(po)
endif ()

# Enable testing
include(CTest)
enable_testing()
add_subdirectory(tests)
