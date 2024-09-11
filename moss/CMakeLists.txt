# simple cmakelist.txt


function(source_tree source_dir base_dir output_list)
	file(GLOB ITEMS RELATIVE ${source_dir} ${source_dir}/*)
	set(GROUP_LIST "")
	set(FILE_LIST "")
	foreach(ITEM ${ITEMS})
		if(IS_DIRECTORY ${source_dir}/${ITEM})
			set(SUB_BASE_DIR ${base_dir}/${ITEM})
			if("${base_dir}" STREQUAL "")
				set(SUB_BASE_DIR ${ITEM})
			endif()
			source_tree(${source_dir}/${ITEM} ${SUB_BASE_DIR} SUB_LIST)
			list(APPEND FILE_LIST ${SUB_LIST})
		elseif(${ITEM} MATCHES ".*\.(h|c|cpp)")
			list(APPEND GROUP_LIST ${source_dir}/${ITEM})
			list(APPEND FILE_LIST ${source_dir}/${ITEM})
		endif()
	endforeach()
	set(${output_list} ${FILE_LIST} PARENT_SCOPE)
	if(NOT "${base_dir}" STREQUAL "")
		source_group(${base_dir} FILES ${GROUP_LIST})
	endif()
endfunction()

function(find_package_via_custom package_name)
	option(${package_name}_INCLUDE_PATH "${packag_name} include path" "")
	option(${package_name}_LIBRARY_PATH "${packag_name} library path" "")
	if(IS_DIRECTORY "${${package_name}_INCLUDE_PATH}")
		include_directories(${${package_name}_INCLUDE_PATH})
	endif()
	if(IS_DIRECTORY "${${package_name}_LIBRARY_PATH}")
		link_directories(${${package_name}_LIBRARY_PATH})
	endif()
endfunction()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
source_tree(${CMAKE_CURRENT_SOURCE_DIR} "moss" MOSS_SOURCES_LIST)


option(USE_INSTALLED_LIBUV "Use installed libuv" on)
if(USE_VCPKG)
	find_package(libuv CONFIG REQUIRED)
else()
	find_package_via_custom(libuv)
endif()


set(MOSS_LINK_LIBS uv)
if(WIN32)
	list(APPEND MOSS_LINK_LIBS ws2_32)
endif()

# moss library
add_library(moss SHARED ${MOSS_SOURCES_LIST})
target_compile_definitions(moss PRIVATE MOSS_EXPORTS)
target_link_libraries(moss PRIVATE ${MOSS_LINK_LIBS})

# enable vcpkg
if(WIN32)
	set_target_properties(moss PROPERTIES VS_GLOBAL_VcpkgEnabled TRUE)
endif()

