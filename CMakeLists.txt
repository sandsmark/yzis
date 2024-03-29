SET( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE )

project(yzis)

enable_testing()
#include(CTest)

# Always include srcdir and builddir in include path
# This saves typing ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY} in about every subdir
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# put the include dirs which are in the source or build tree
# before all other include dirs, so the headers in the sources
# are prefered over the already installed ones
set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)

# Use colored output
set(CMAKE_COLOR_MAKEFILE ON)


cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

# yzis stores the cmake modules in cmake/modules
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(CheckCXXCompilerFlag)
include(ConfigureChecks.cmake)
include(ConvenienceLibs.cmake)
include(MacroAdditionalCleanFiles)

set( CCMAKE_CXX_FLAGS_RELEASE "-O3" )
set( CCMAKE_C_FLAGS_RELEASE "-O3" )

########################## options for compilation
#
if (WIN32)
    # on windows, we default to off
    option(ENABLE_NYZIS "enable nyzis compilation" OFF)
else(WIN32)
    option(ENABLE_NYZIS "enable nyzis compilation" ON)
endif(WIN32)

option(ENABLE_QYZIS "enable qyzis compilation" ON)
option(ENABLE_KYZIS "enable kyzis compilation" OFF)
option(ENABLE_KPART_YZIS "enable yzis_kpart compilation" OFF)
option(ENABLE_LIBYZISRUNNER "enable libyzisrunner compilation" ON)
option(ENABLE_TESTS "enable tests compilation" ON)

option(ENABLE_DOCUMENTATION "Allow doxygen documentation generation with 'make docs'" ON)
option(GENERATE_DOC "Generate doxygen documentation during build" OFF)

option(ENABLE_SAFE_MODE "prevent yzis from crashing when some bugs occured" ON)

include(ConfigureDoc)

find_package(Gettext REQUIRED)

option(ENABLE_SANITIZERS "Enable runtime sanitizing (for development)")
if (ENABLE_SANITIZERS)
    message("Enabling asan and ubsan")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
endif()


#
# platform specific settings
#

if (NOT MSVC)
    # actually means that we are using gcc
    set( CMAKE_CXX_FLAGS_DEBUG "-g3 -Wall -DDEBUG" )
    set( CMAKE_C_FLAGS_DEBUG "-g3 -Wall -DDEBUG" )
endif (NOT MSVC)

if (WIN32)
    add_definitions("-DYZIS_WIN32")
endif(WIN32)
if (MINGW)
    add_definitions("-DYZIS_WIN32_GCC")
    find_package(GNUWIN32)
    link_libraries(imm32 winmm)  # missing by default :/
    SET(CMAKE_EXE_LINKER_FLAGS 
        "-Wl,-enable-stdcall-fixup -Wl,-enable-auto-import -Wl,-enable-runtime-pseudo-reloc " 
        ${CMAKE_EXE_LINKER_FLAGS}
        )
endif(MINGW)

if (MSVC)
    set( CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -DYZIS_FULL_TEMPLATE_EXPORT_INSTANTIATION -DDONT_HAVE_SYS_STAT_H -D_CRT_SECURE_NO_WARNINGS" )
    set( CMAKE_C_FLAGS_DEBUG "-DDEBUG -DYZIS_FULL_TEMPLATE_EXPORT_INSTANTIATION -DDONT_HAVE_SYS_STAT_H -D_CRT_SECURE_NO_WARNINGS" )
    add_definitions("-DYZIS_WIN32_MSVC")
endif (MSVC)

if (UNIX)
    add_definitions("-DYZIS_UNIX")
endif (UNIX)

if (APPLE) # XXX: workaround intl for Mac OSX
    set(GETTEXT_LIBRARIES ${LIBINTL_LIBRARY})
    add_definitions("-DYZIS_APPLE")
endif(APPLE)

### lua
find_package(Lua REQUIRED)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Test Xml)
set(CMAKE_AUTOMOC ON)
#include( FixQt4DebugBuild )

# for syntax files
if (NOT WIN32)
find_package(Perl REQUIRED)
endif(NOT WIN32)

# optional packages dependencies :
if (ENABLE_KYZIS OR ENABLE_KPART_YZIS)
	find_package(KDE4 REQUIRED)
endif (ENABLE_KYZIS OR ENABLE_KPART_YZIS)

if (ENABLE_NYZIS)
	find_package(Ncurses)
	if (NOT NCURSES_FOUND)
		message(FATAL_ERROR "Could not find ncurses library.")
	endif (NOT NCURSES_FOUND)

	if (NOT NCURSESW_FOUND AND NOT HAVE_NCURSESW)
		message(FATAL_ERROR "The ncurses library does not support wide-character.")
	endif (NOT NCURSESW_FOUND AND NOT HAVE_NCURSESW)
endif (ENABLE_NYZIS)

#
# define some variables
#
if (NOT APPLNK_INSTALL_DIR)
	set(APPLNK_INSTALL_DIR share/applnk)
endif (NOT APPLNK_INSTALL_DIR)

if (NOT ICON_INSTALL_DIR)
	set(ICON_INSTALL_DIR share/icons)
endif (NOT ICON_INSTALL_DIR)

# XXX how deal with macosx bundles?
if (APPLE)
	if (NOT APPLE_APPLICATION_DIR)
		set(APPLE_APPLICATION_DIR /Applications)
	endif (NOT APPLE_APPLICATION_DIR)
endif (APPLE)


############################# build instructions
#

# visibility support
if (CMAKE_COMPILER_IS_GNUCXX)
	check_cxx_compiler_flag(-fvisibility=hidden __YZIS_HAVE_GCC_VISIBILITY)
	if (__YZIS_HAVE_GCC_VISIBILITY)
	     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
	endif (__YZIS_HAVE_GCC_VISIBILITY)
endif (CMAKE_COMPILER_IS_GNUCXX)

# libyzis dependencies includes
set(YZIS_INCLUDES ${QT_INCLUDES} ${LUA_INCLUDE_DIR} ${GETTEXT_INCLUDE_DIR})
# libyzis includes
set(YZIS_INCLUDES ${YZIS_INCLUDES} ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/libyzis ${CMAKE_SOURCE_DIR}/libyzis)

add_definitions(${QT_DEFINITIONS})

if (ENABLE_SAFE_MODE)
	add_definitions("-DSAFE_MODE")
endif (ENABLE_SAFE_MODE)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/yzismacros.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/yzismacros.h )

# macro_additional_clean_files(
#	${CMAKE_CURRENT_BINARY_DIR}/config.h
#	${CMAKE_CURRENT_BINARY_DIR}/yzismacros.h
#)


####################### subdirectories to compile
#
add_subdirectory( libyzis )

if (ENABLE_LIBYZISRUNNER)
	add_subdirectory( libyzisrunner )
endif (ENABLE_LIBYZISRUNNER)

if (ENABLE_NYZIS)
	add_subdirectory( nyzis )
endif (ENABLE_NYZIS)

if (ENABLE_QYZIS)
	add_subdirectory( qyzis )
endif (ENABLE_QYZIS)

if (ENABLE_KYZIS)
	add_subdirectory( kyzis )
endif (ENABLE_KYZIS)

if (ENABLE_KPART_YZIS)
	add_subdirectory( kpart_yzis )
endif (ENABLE_KPART_YZIS)

if (ENABLE_TESTS)
	add_subdirectory( tests )
endif (ENABLE_TESTS)

add_subdirectory( doc )
#add_subdirectory( translations )
add_subdirectory( syntax )
add_subdirectory( scripts )

