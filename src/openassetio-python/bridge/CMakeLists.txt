# SPDX-License-Identifier: Apache-2.0
# Copyright 2013-2022 The Foundry Visionmongers Ltd

#----------------------------------------------------------------------
# Public headers

set(public_header_source_root ${CMAKE_CURRENT_LIST_DIR}/include)

# Installation location for install phase.
# TODO(DF): When CMake 3.23 is released, use FILE_SET, which allows
#   explicitly associating public headers with a target. Note that the
#   PUBLIC_HEADER target property is not useful since it flattens the
#   directory structure when installed.
install(
    DIRECTORY
    ${public_header_source_root}/openassetio
    DESTINATION
    ${CMAKE_INSTALL_INCLUDEDIR}
)


#-----------------------------------------------------------------------
# Create C++ Python bridge target

# Note: static vs. shared is auto-determined by CMake's built-in
# BUILD_SHARED_LIBS option.
add_library(openassetio-python-bridge)
# Set good default target options.
set_default_target_properties(openassetio-python-bridge)
# Set output artifact base filename.
set_target_properties(openassetio-python-bridge PROPERTIES OUTPUT_NAME openassetio-python)
# Add to the set of installable targets.
install(TARGETS openassetio-python-bridge EXPORT ${PROJECT_NAME}_EXPORTED_TARGETS)

# TODO(DF): This needs some thought to get right - see #538.
if (WIN32 AND OPENASSETIO_ENABLE_PYTHON)
    install(TARGETS openassetio-python-bridge
        DESTINATION "${OPENASSETIO_PYTHON_SITEDIR}/openassetio")
    install(FILES $<TARGET_PDB_FILE:openassetio-python-bridge>
        CONFIGURATIONS "Debug" "RelWithDebInfo"
        DESTINATION "${OPENASSETIO_PYTHON_SITEDIR}/openassetio")
endif ()


#-----------------------------------------------------------------------
# Target dependencies

# Source file dependencies.
target_sources(openassetio-python-bridge
    PRIVATE
    python/hostApi.cpp)

# Public header dependency.
target_include_directories(openassetio-python-bridge
    PUBLIC
    # Common helpers only available at build time.
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../private/include>"
    # For generated export.h header.
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>"
    # Use includes from source tree for building.
    "$<BUILD_INTERFACE:${public_header_source_root}>"
    # Use includes from install tree for installed lib.
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

# Library dependencies.
target_link_libraries(openassetio-python-bridge
    PRIVATE
    openassetio-core
    # TODO(DF): Pybind docs say the pybind11 target just includes
    #   pybind and Python headers, which is all we should need for this
    #   library. However, a bug in the ConanCenter package means all
    #   pybind targets include everything. See #528.
    pybind11::pybind11)


#-----------------------------------------------------------------------
# API export header

# Use CMake utility to generate the export header.
include(GenerateExportHeader)
generate_export_header(
    openassetio-python-bridge
    EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/openassetio/python/export.h
)

install(
    FILES ${PROJECT_BINARY_DIR}/include/openassetio/python/export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/openassetio/python/
)