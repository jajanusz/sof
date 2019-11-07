# SPDX-License-Identifier: BSD-3-Clause

# Reads configs from kconfig file and set them as cmake variables.
# Each config is in format CONFIG_<NAME>=<VALUE>.
# Configs are added to parent scope with CONFIG_ prefix (as written in file).
function(read_kconfig_config config_file)
	file(
		STRINGS
		${config_file}
		configs_list
		REGEX "^CONFIG_"
		ENCODING "UTF-8"
	)

	foreach(config ${configs_list})
		string(REGEX MATCH "^([^=]+)=(.*)$" ignored ${config})
		set(config_name ${CMAKE_MATCH_1})
		set(config_value ${CMAKE_MATCH_2})

		if("${config_value}" MATCHES "^\"(.*)\"$")
			set(config_value ${CMAKE_MATCH_1})
		endif()

		set("${config_name}" "${config_value}" PARENT_SCOPE)
	endforeach()
endfunction()

# Adds sources to target like target_sources, but assumes that
# paths are relative to subdirectory.
# Works like:
# 	Cmake >= 3.13:
#		target_sources(<target> PRIVATE <sources>)
# 	Cmake < 3.13:
#		target_sources(<target> PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/<sources>)
function(add_local_sources target)
	foreach(arg ${ARGN})
		if(IS_ABSOLUTE ${arg})
			set(path ${arg})
		else()
			set(path ${CMAKE_CURRENT_SOURCE_DIR}/${arg})
		endif()

		target_sources(${target} PRIVATE ${path})
	endforeach()
endfunction()

# Declares new static lib with given name and path that will be linked
# to sof binary.
function(sof_add_static_library lib_name lib_path)
	# we need libs to be visible in the root CMakeLists, so use GLOBAL
	add_library(${lib_name} STATIC IMPORTED GLOBAL)

	if(IS_ABSOLUTE ${lib_path})
		set(lib_abs_path ${lib_path})
	else()
		set(lib_abs_path ${CMAKE_CURRENT_SOURCE_DIR}/${lib_path})
	endif()

	set_target_properties(${lib_name} PROPERTIES IMPORTED_LOCATION ${lib_abs_path})
	target_link_libraries(sof_static_libraries INTERFACE ${lib_name})
endfunction()

function(sof_is_interface_library output_var target)
	set(is_interface NO)
	get_target_property(imported ${target} IMPORTED)
	if(imported)
		set(is_interface YES)
	else()
		get_target_property(target_type ${target} TYPE)
		if(target_type STREQUAL "INTERFACE_LIBRARY")
			set(is_interface YES)
		endif()
	endif()
	set(${output_var} ${is_interface} PARENT_SCOPE)
endfunction()

function(_sof_get_libraries output_list target)
	sof_is_interface_library(is_interface ${target})

	if(is_interface)
		get_target_property(libs ${target} INTERFACE_LINK_LIBRARIES)
	else()
		get_target_property(libs ${target} LINK_LIBRARIES)
	endif()

	set(lib_files "")
	list(APPEND visited_targets ${target})

	foreach(lib ${libs})
		# Check if it's target, because library in CMake can be also
		# some linker flag
		if(TARGET ${lib})
			if(NOT ${lib} IN_LIST visited_targets)
				_sof_get_libraries(inner_lib_files ${lib})
				list(APPEND lib_files ${lib})
				list(APPEND lib_files ${inner_lib_files})
			endif()
		endif()
	endforeach()

	set(visited_targets ${visited_targets} PARENT_SCOPE)
	set(${output_list} ${lib_files} PARENT_SCOPE)
endfunction()

function(sof_get_libraries output_list target)
	set(visited_targets "")
	_sof_get_libraries(container ${target})
	set(${output_list} ${container} PARENT_SCOPE)
endfunction()

function(sof_get_target_sources output_list target)
	set(all_sources "")
	sof_get_libraries(libs sof)
	list(APPEND libs ${target})

	foreach(lib ${libs})
		sof_is_interface_library(is_interface ${lib})
		if(NOT is_interface)
			get_target_property(sources ${lib} SOURCES)
			list(APPEND all_sources ${sources})
		endif()
	endforeach()

	set(${output_list} ${all_sources} PARENT_SCOPE)
endfunction()
