# #####################################
# ## ns_add_dependency(<dstTarget> <srcTarget>)
# #####################################

function(ns_add_dependency DST_TARGET SRC_TARGET)
	if(NOT TARGET ${DST_TARGET})
		# message(STATUS "DST_TARGET '${DST_TARGET}' is unknown")
		return()
	endif()

	if(NOT TARGET ${SRC_TARGET})
		# message(STATUS "SRC_TARGET '${SRC_TARGET}' is unknown")
		return()
	endif()

	add_dependencies(${DST_TARGET} ${SRC_TARGET})
endfunction()

# #####################################
# ## ns_add_as_runtime_dependency(<target>)
# #####################################
function(ns_add_as_runtime_dependency TARGET_NAME)
	# editor
	ns_add_dependency(Editor ${TARGET_NAME})
	ns_add_dependency(EditorProcessor ${TARGET_NAME})

	# player
	ns_add_dependency(Player ${TARGET_NAME})
endfunction()