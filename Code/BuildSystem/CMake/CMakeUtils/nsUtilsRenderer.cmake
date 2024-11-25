# #####################################
# ## ns_requires_renderer()
# #####################################

macro(ns_requires_renderer)
	# PLATFORM-TODO
	if(NS_BUILD_EXPERIMENTAL_WEBGPU)
		ns_requires_webgpu()
	endif()

	if(NS_BUILD_EXPERIMENTAL_VULKAN)
		ns_requires_vulkan()
	endif()

	if(NS_CMAKE_PLATFORM_WINDOWS)
		ns_requires_d3d()
	endif()
endmacro()

# #####################################
# ## ns_add_renderers(<target>)
# ## Add all required libraries and dependencies to the given target so it has access to all available renderers.
# #####################################
function(ns_add_renderers TARGET_NAME)
	# PLATFORM-TODO
	if(NS_BUILD_EXPERIMENTAL_VULKAN)
		target_link_libraries(${TARGET_NAME}
			PRIVATE
			RendererVulkan
		)

		if (TARGET ShaderCompilerVulkan)
			add_dependencies(${TARGET_NAME}
				ShaderCompilerVulkan
			)
		endif()
	endif()

	if(NS_BUILD_EXPERIMENTAL_WEBGPU)
		target_link_libraries(${TARGET_NAME}
			PRIVATE
			RendererWebGPU
		)

		if (TARGET ShaderCompilerWebGPU)
			add_dependencies(${TARGET_NAME}
				ShaderCompilerWebGPU
			)
		endif()
	endif()

	if(NS_BUILD_EXPERIMENTAL_WEBGPU)
		target_link_libraries(${TARGET_NAME}
			PRIVATE
			RendererWebGPU
		)
	endif()

	if(NS_CMAKE_PLATFORM_WINDOWS)
		target_link_libraries(${TARGET_NAME}
			PRIVATE
			RendererDX11
		)
		ns_link_target_dx11(${TARGET_NAME})

		if (TARGET ShaderCompilerHLSL)
			add_dependencies(${TARGET_NAME}
				ShaderCompilerHLSL
			)
		endif()
	endif()
endfunction()