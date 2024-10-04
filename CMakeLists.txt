cmake_minimum_required(VERSION 2.8.12)
project(zindex)

#------------------------------------------------------------------------------
# Version information
#------------------------------------------------------------------------------

set(ZINDEX_VERSION_MAJOR "0")
set(ZINDEX_VERSION_MINOR "0")
set(ZINDEX_VERSION_PATCH "3")
set(ZINDEX_PACKAGE ${PROJECT_NAME})
set(ZINDEX_PACKAGE_NAME ${PROJECT_NAME})
set(ZINDEX_PACKAGE_VERSION "${ZINDEX_VERSION_MAJOR}.${ZINDEX_VERSION_MINOR}.${ZINDEX_VERSION_PATCH}")
set(ZINDEX_PACKAGE_VERSION_MAJOR "${ZINDEX_VERSION_MAJOR}.${ZINDEX_VERSION_MINOR}")
set(ZINDEX_PACKAGE_VERSION_MINOR "${ZINDEX_VERSION_PATCH}")
set(ZINDEX_PACKAGE_STRING "${ZINDEX_PACKAGE_NAME} ${ZINDEX_PACKAGE_VERSION}")
set(ZINDEX_PACKAGE_TARNAME "${ZINDEX_PACKAGE}")

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_BINARY_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_BINARY_DIR}/lib/cmake)
link_directories(${CMAKE_BINARY_DIR}/lib)
set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_CXX_STANDARD 11)
if (APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -Wall -Werror -Wextra")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -Wall -Werror -Wextra")
endif ()

if (APPLE AND CMAKE_C_COMPILER_ID MATCHES "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
else ()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -Wall -Wextra")
endif ()

#------------------------------------------------------------------------------
# Build options
#------------------------------------------------------------------------------
option(ZINDEX_USE_LTO "Use link-time optimization" OFF)
option(ZINDEX_BUILD_STATIC "Statically link" OFF)
option(ZINDEX_BUILD_SQL_SHELL "Build the sqlite shell" OFF)
option(ZINDEX_ARCH_NATIVE "Target the computer being built on (march=native)" OFF)
option(ZINDEX_PGO "Set PGO flags" "")
option(BUILD_PYTHON_BINDINGS "Build python bindings." OFF)
option(ZINDEX_ENABLE_TESTS "Enable internal tests" Off)

#------------------------------------------------------------------------------
# Setup install and output Directories
#------------------------------------------------------------------------------

if (BUILD_PYTHON_BINDINGS)
    if (ZINDEX_PYTHON_SITE)
        set(PYTHON_SITE_PACKAGES "${ZINDEX_PYTHON_SITE}")
        set(MAIN_PYTHON_SITE_PACKAGES ${PYTHON_SITE_PACKAGES})
    else()
        set(PYTHON_SITE_PACKAGES ${CMAKE_INSTALL_PREFIX})
        execute_process (COMMAND python3 -c "from distutils.sysconfig import get_python_lib; print(get_python_lib())" OUTPUT_VARIABLE MAIN_PYTHON_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()
    set(PYTHON_SITE_PACKAGES ${PYTHON_SITE_PACKAGES})
    include_directories(${PYTHON_SITE_PACKAGES}/zindex_py/include)
    link_directories(${PYTHON_SITE_PACKAGES}/zindex_py/lib)
    message("-- " "[Python] Main Site ${MAIN_PYTHON_SITE_PACKAGES} found for installing python binding")
    message("-- " "[Python] Site ${MAIN_PYTHON_SITE_PACKAGES} found for installing python binding")
endif()
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
set(ZINDEX_CMAKE_DIR ${CMAKE_SOURCE_DIR}/CMake ${CMAKE_BINARY_DIR}/lib/cmake ${PYTHON_SITE_PACKAGES}/zindex_py/lib/cmake)
message("-- " "[ZINDEX] ZINDEX_CMAKE_DIR ${ZINDEX_CMAKE_DIR}")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ZINDEX_CMAKE_DIR})
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${PYTHON_SITE_PACKAGES}/zindex_py/lib/cmake)

if (NOT ZINDEX_INSTALL_BIN_DIR)
    set(ZINDEX_INSTALL_BIN_DIR ${CMAKE_INSTALL_PREFIX}/bin)
    message(STATUS ${ZINDEX_INSTALL_BIN_DIR})
endif ()
if (NOT ZINDEX_INSTALL_LIB_DIR)
    set(ZINDEX_INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/lib)
endif ()
if (NOT ZINDEX_INSTALL_INCLUDE_DIR)
    set(ZINDEX_INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include)
endif ()
if (NOT ZINDEX_INSTALL_DATA_DIR)
    set(ZINDEX_INSTALL_DATA_DIR ${CMAKE_INSTALL_PREFIX}/share)
endif ()

if (NOT CMAKE_INSTALL_RPATH)
    message("-- " "[ZINDEX] using RPATH")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib:${PYTHON_SITE_PACKAGES}/lib:${PYTHON_SITE_PACKAGES}/lib64")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
else()
    message("-- " "[ZINDEX] not using RPATH")
endif ()

#------------------------------------------------------------------------------
if (NOT ZINDEX_EXTERNALLY_CONFIGURED)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
            ${CMAKE_BINARY_DIR}/bin CACHE PATH "Single Directory for all Executables."
    )

    set(CMAKE_INCLUDE_OUTPUT_DIRECTORY
            ${CMAKE_BINARY_DIR}/include CACHE PATH "Store the headers."
    )
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
            ${CMAKE_BINARY_DIR}/lib CACHE PATH "Single Directory for all Libraries"
    )
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
            ${CMAKE_BINARY_DIR}/lib CACHE PATH "Single Directory for all static libraries."
    )
endif ()

if (ZINDEX_PGO)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PGO}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${PGO}")
endif (ZINDEX_PGO)

if (ZINDEX_USE_LTO)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto -Wno-maybe-uninitialized")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto -Wno-maybe-uninitialized")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-linker-plugin")
endif (ZINDEX_USE_LTO)

set(COMMON_LIBS "")
if (ZINDEX_BUILD_STATIC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSQLITE_OMIT_LOAD_EXTENSION=1")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(BUILD_SHARED_LIBRARIES OFF)
    set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)       # remove -Wl,-Bdynamic
    set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS)
else (ZINDEX_BUILD_STATIC)
    set(COMMON_LIBS "dl")
endif (ZINDEX_BUILD_STATIC)

if (ZINDEX_ARCH_NATIVE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
endif (ZINDEX_ARCH_NATIVE)

#------------------------------------------------------------------------------
# Find Dependencies
#------------------------------------------------------------------------------
set(DEPENDENCY_LIB stdc++fs ${COMMON_LIBS})
link_directories(${CMAKE_BINARY_DIR}/lib)
link_directories(${CMAKE_BINARY_DIR})

if (BUILD_PYTHON_BINDINGS)
    find_package(pybind11 REQUIRED)
endif()

find_package(ZLIB REQUIRED)
if (${ZLIB_FOUND})
    include_directories(${ZLIB_INCLUDE_DIRS})
    set(DEPENDENCY_LIB ${DEPENDENCY_LIB} ${ZLIB_LIBRARIES})
    message(STATUS "[ZINDEX] found zlib at ${ZLIB_INCLUDE_DIRS}")
else ()
    message(FATAL_ERROR "-- [ZINDEX] zlib is needed for ${PROJECT_NAME} build")
endif ()

#------------------------------------------------------------------------------
# Include internal headers
#------------------------------------------------------------------------------
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(BEFORE SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/ext/sqlite)

set(ZINDEX_SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/File.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Index.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Index.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/IndexParser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/IndexParser.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/LineFinder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/LineFinder.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/LineSink.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Sqlite.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Sqlite.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/SqliteError.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/RegExp.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/RegExp.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/RegExpIndexer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/RegExpIndexer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/LineIndexer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/IndexSink.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Log.h
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/cJSON/cJSON.h
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/cJSON/cJSON.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ConsoleLog.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ConsoleLog.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/StringView.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/StringView.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/PrettyBytes.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/PrettyBytes.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/RangeFetcher.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/RangeFetcher.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/FieldIndexer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/FieldIndexer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ExternalIndexer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ExternalIndexer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Pipe.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Pipe.h
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/sqlite/sqlite3.c)

add_library(${PROJECT_NAME}_core SHARED ${ZINDEX_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME}_core ${DEPENDENCY_LIB})
target_include_directories(${PROJECT_NAME}_core
        PRIVATE
        # where the library itself will look for its internal headers
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/ext
        PUBLIC
        # where top-level project will look for the library's public headers
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # where external projects will look for the library's public headers
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_executable(zindex ${CMAKE_CURRENT_SOURCE_DIR}/src/zindex.cpp)
target_link_libraries(zindex ${PROJECT_NAME}_core)
add_executable(zq ${CMAKE_CURRENT_SOURCE_DIR}/src/zq.cpp)
target_link_libraries(zq ${PROJECT_NAME}_core)

if (BUILD_PYTHON_BINDINGS)
    pybind11_add_module(${PROJECT_NAME}_py ${CMAKE_CURRENT_SOURCE_DIR}/src/zindex_py.cpp)
    target_compile_definitions(${PROJECT_NAME}_py PRIVATE VERSION_INFO=${ZINDEX_PACKAGE_VERSION})
    target_link_libraries(${PROJECT_NAME}_py PRIVATE ${PROJECT_NAME}_core ${DEPENDENCY_LIB})
endif()

#-----------------------------------------------------------------------------
# Configure tests
#-----------------------------------------------------------------------------
if(ZINDEX_ENABLE_TESTS)
    message("-- " "[ZINDEX] Enabling Testing")
    if(CMAKE_BUILD_TYPE STREQUAL "PROFILE")
        message("-- " "[ZINDEX] building with CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}")
        set(CMAKE_C_FLAGS_PROFILE --coverage)
        set(CMAKE_CXX_FLAGS_PROFILE --coverage)
    endif()
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
    if(CMAKE_BUILD_TYPE STREQUAL "PROFILE")
        get_property(COVERAGE_FILE_LIST GLOBAL PROPERTY COVERAGE_FILES)
        add_custom_command(OUTPUT default.profdata COMMAND llvm-profdata merge -sparse ${COVERAGE_FILE_LIST} -o default.profdata VERBATIM)
        add_custom_target(merged_coverage DEPENDS default.profdata)
    endif()
endif()
#------------------------------------------------------------------------------
# Disallow in-source build
#------------------------------------------------------------------------------
if ("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR
            "${PROJECT_NAME} requires an out of source Build. "
            "Please create a separate binary directory and run CMake there.")
endif ()

#------------------------------------------------------------------------------
# Set a default build type if none was specified
#------------------------------------------------------------------------------
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
            "MinSizeRel" "RelWithDebInfo")
endif ()

if (NOT "${CMAKE_CXX_STANDARD}")
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif ()

#-----------------------------------------------------------------------------
# Targets built within this project are exported at Install time for use
# by other projects.
#-----------------------------------------------------------------------------
if (NOT ZINDEX_EXPORTED_TARGETS)
    set(ZINDEX_EXPORTED_TARGETS "zindex-targets")
endif ()

set(ZINDEX_LIBTYPE SHARED)
set_target_properties(${libtarget}
        PROPERTIES
        DEBUG_OUTPUT_NAME ${LIB_DEBUG_NAME}
        RELEASE_OUTPUT_NAME ${LIB_RELEASE_NAME}
        MINSIZEREL_OUTPUT_NAME ${LIB_RELEASE_NAME}
        RELWITHDEBINFO_OUTPUT_NAME ${LIB_RELEASE_NAME}
        VERSION ${LIB_VERSION}
        SOVERSION ${LIB_VERSION}
)
include(GNUInstallDirs)
#-----------------------------------------------------------------------------
# Configure the config.cmake file for the build directory
#-----------------------------------------------------------------------------
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/CMake/${PROJECT_NAME}-config.cmake.build.in
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/cmake/${PROJECT_NAME}/${PROJECT_NAME}-config.cmake @ONLY
)

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/CMake/${PROJECT_NAME}-config.cmake.install.in
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/cmake/${PROJECT_NAME}/install/${PROJECT_NAME}-config.cmake @ONLY
)
install(
        FILES
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/cmake/${PROJECT_NAME}/install/${PROJECT_NAME}-config.cmake
        DESTINATION
        ${ZINDEX_INSTALL_LIB_DIR}/cmake/${PROJECT_NAME}
)

install(TARGETS ${PROJECT_NAME}_core
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS zq
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS ${PROJECT_NAME}_py
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
#file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/symlink.sh CONTENT "echo -- Installing: symlink $<TARGET_FILE_NAME:${PROJECT_NAME}_py>;ln -sf ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}_py> ${PYTHON_SITE_PACKAGES}/$<TARGET_FILE_NAME:${PROJECT_NAME}_py>")
file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/symlink.sh CONTENT "echo -- Installing: symlink ${MAIN_PYTHON_SITE_PACKAGES}/$<TARGET_FILE_NAME:${PROJECT_NAME}_py> from ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}_py>;ln -sf ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}_py> ${MAIN_PYTHON_SITE_PACKAGES}/$<TARGET_FILE_NAME:${PROJECT_NAME}_py>")

#install(CODE TARGETS ${PROJECT_NAME}_py "execute_process(COMMAND ${CMAKE_BINARY_DIR}/symlink.sh)")

install(CODE "execute_process(
    COMMAND bash -c \"set -e
    chmod +x ${CMAKE_BINARY_DIR}/symlink.sh
    . ${CMAKE_BINARY_DIR}/symlink.sh
    \")")

#install(TARGETS ${PROJECT_NAME}_py
#        LIBRARY DESTINATION ${PYTHON_SITE_PACKAGES}
#        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
#-----------------------------------------------------------------------------
# Configure the ${PROJECT_NAME}-config-version .cmake file for the install directory
#-----------------------------------------------------------------------------
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/CMake/${PROJECT_NAME}-config-version.cmake.in
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/cmake/${PROJECT_NAME}/${PROJECT_NAME}-config-version.cmake @ONLY
)

install(
        FILES
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/cmake/${PROJECT_NAME}/${PROJECT_NAME}-config-version.cmake
        DESTINATION
        ${ZINDEX_INSTALL_LIB_DIR}/cmake/${PROJECT_NAME}
)

install(
        TARGETS
        ${PROJECT_NAME}
        EXPORT
        ${ZINDEX_EXPORTED_TARGETS}
        LIBRARY DESTINATION ${ZINDEX_INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${ZINDEX_INSTALL_LIB_DIR}
        RUNTIME DESTINATION ${ZINDEX_INSTALL_BIN_DIR}
)
install(
        TARGETS
        ${PROJECT_NAME}_core
        EXPORT
        ${ZINDEX_EXPORTED_TARGETS}
        LIBRARY DESTINATION ${ZINDEX_INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${ZINDEX_INSTALL_LIB_DIR}
        RUNTIME DESTINATION ${ZINDEX_INSTALL_BIN_DIR}
)
install(
        TARGETS
        ${PROJECT_NAME}_py
        EXPORT
        ${ZINDEX_EXPORTED_TARGETS}
        LIBRARY DESTINATION ${ZINDEX_INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${ZINDEX_INSTALL_LIB_DIR}
        RUNTIME DESTINATION ${ZINDEX_INSTALL_BIN_DIR}
)
export(EXPORT ${ZINDEX_EXPORTED_TARGETS}
        FILE "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/cmake/${PROJECT_NAME}/${PROJECT_NAME}Targets.cmake")

install(EXPORT
        ${ZINDEX_EXPORTED_TARGETS}
        DESTINATION
        ${ZINDEX_INSTALL_LIB_DIR}/cmake/${PROJECT_NAME}
        FILE
        ${ZINDEX_EXPORTED_TARGETS}.cmake
)

if (ZINDEX_BUILD_SQL_SHELL)
    add_executable(sql-shell ext/sqlite/shell.c ext/sqlite/sqlite3.c)
    target_link_libraries(sql-shell ${COMMON_LIBS})
endif (ZINDEX_BUILD_SQL_SHELL)
