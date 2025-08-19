-- Paul Engine Dependencies --

VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["GLFW"] = "PaulEngine/vendor/GLFW/GLFW/include"
IncludeDir["glad"] = "PaulEngine/vendor/glad/include"
IncludeDir["ImGui"] = "PaulEngine/vendor/imgui/imgui"
IncludeDir["glm"] = "PaulEngine/vendor/glm"
IncludeDir["stb_image"] = "PaulEngine/vendor/stb_image"
IncludeDir["entt"] = "PaulEngine/vendor/entt/include"
IncludeDir["yaml"] = "PaulEngine/vendor/yaml/yaml-cpp/include"
IncludeDir["imguizmo"] = "PaulEngine/vendor/imguizmo"
IncludeDir["shaderc"] = "PaulEngine/vendor/shaderc/include"
IncludeDir["SPIRV_Cross"] = "PaulEngine/vendor/SPIRV-Cross"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["Box2D"] = "PaulEngine/vendor/box2d/box2d/include"
IncludeDir["msdfgen"] = "PaulEngine/vendor/msdfgen/msdfgen"
IncludeDir["msdf_atlas_gen"] = "PaulEngine/vendor/msdfgen/msdf-atlas-gen"
IncludeDir["assimp"] = "PaulEngine/vendor/assimp/assimp/include"
IncludeDir["assimp_config"] = "PaulEngine/vendor/assimp/_config_headers"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"