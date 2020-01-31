# SPDX-License-Identifier: BSD-3-Clause

# Generates header for which version is taken from (in order of precedence):
# 	1) .tarball-version file
#	2) git
#
# Version is checked during configuration step and for every target
# that has check_version_h target as dependency

cmake_minimum_required(VERSION 3.10)

set(VERSION_CMAKE_PATH ${CMAKE_CURRENT_LIST_DIR}/version.cmake)

set(TARBALL_VERSION_FILE_NAME ".tarball-version")
set(TARBALL_VERSION_SOURCE_PATH "${SOF_ROOT_SOURCE_DIRECTORY}/${TARBALL_VERSION_FILE_NAME}")

if(EXISTS ${TARBALL_VERSION_SOURCE_PATH})
	file(STRINGS ${TARBALL_VERSION_SOURCE_PATH} lines ENCODING "UTF-8")
	list(GET lines 0 GIT_TAG)
	list(GET lines 1 GIT_LOG_HASH)
	message(STATUS "Found ${TARBALL_VERSION_FILE_NAME}")
	message(STATUS "Version: ${GIT_TAG} / ${GIT_LOG_HASH}")
else()
	execute_process(COMMAND git describe --tags --abbrev=4
		OUTPUT_VARIABLE GIT_TAG
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_QUIET
	)

	execute_process(COMMAND git log --pretty=format:%h -1
		OUTPUT_VARIABLE GIT_LOG_HASH
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_QUIET
	)
endif()

if(NOT GIT_TAG MATCHES "^v")
	set(GIT_TAG "v0.0-0-g0000")
endif()

string(REGEX MATCH "^v([0-9]+)[.]([0-9]+)([.]([0-9]+))?" ignored "${GIT_TAG}")
set(SOF_MAJOR ${CMAKE_MATCH_1})
set(SOF_MINOR ${CMAKE_MATCH_2})
set(SOF_MICRO ${CMAKE_MATCH_4})

if(NOT SOF_MICRO MATCHES "^[0-9]+$")
	set(SOF_MICRO 0)
endif()

string(SUBSTRING "${GIT_LOG_HASH}" 0 5 SOF_TAG)
if(NOT SOF_TAG)
	set(SOF_TAG 0)
endif()

# for SOF_BUILD
include(${CMAKE_CURRENT_LIST_DIR}/version-build-counter.cmake)

set(SOF_VERSION "${SOF_MAJOR}.${SOF_MINOR}.${SOF_MICRO}.${SOF_BUILD}")

function(sof_check_version_h)
	string(CONCAT header_content
		"#define SOF_MAJOR ${SOF_MAJOR}\n"
		"#define SOF_MINOR ${SOF_MINOR}\n"
		"#define SOF_MICRO ${SOF_MICRO}\n"
		"#define SOF_TAG \"${SOF_TAG}\"\n"
		"#define SOF_BUILD ${SOF_BUILD}\n"
	)

	if(EXISTS "${VERSION_H_PATH}")
		file(READ "${VERSION_H_PATH}" old_version_content)
		if("${header_content}" STREQUAL "${old_version_content}")
			message(STATUS "Up-to-date ${VERSION_H_PATH}")
			return()
		endif()
	endif()	

	message(STATUS "Generating ${VERSION_H_PATH}")
	file(WRITE "${VERSION_H_PATH}" "${header_content}")
endfunction()

# Run these only if not run as script
if("${CMAKE_SCRIPT_MODE_FILE}" STREQUAL "")
	add_custom_target(
		check_version_h
		BYPRODUCTS ${VERSION_H_PATH}
		COMMAND ${CMAKE_COMMAND}
			-DVERSION_H_PATH=${VERSION_H_PATH}
			-DSOF_ROOT_SOURCE_DIRECTORY=${SOF_ROOT_SOURCE_DIRECTORY}
			-DSOF_ROOT_BINARY_DIRECTORY=${SOF_ROOT_BINARY_DIRECTORY}
			-P ${VERSION_CMAKE_PATH}
		COMMENT "Checking ${VERSION_H_PATH}"
		VERBATIM
		USES_TERMINAL
	)
endif()

sof_check_version_h()
