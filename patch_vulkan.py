import re

with open("materialsystem/shaderapivulkan/shaderapivulkan.cpp", "r") as f:
    content = f.read()

header = """
#include <vulkan/vulkan.h>
#include <SDL3/SDL_vulkan.h>
"""
content = header + content

init_code = """
	Msg("[NativeVulkan] Setting mode for adapter %d\\n", nAdapter);
	
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Source Engine Native Vulkan";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Source";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;
	createInfo.pApplicationInfo = &appInfo;
	
	VkInstance instance;
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
	{
		Warning("[NativeVulkan] Failed to create Vulkan instance!\\n");
	}
	else
	{
		Msg("[NativeVulkan] Successfully created Vulkan instance!\\n");
		Msg("[NativeVulkan] (Rank 14 Bootstrap complete. Rank 16 MVP needed for Testchamber 15)\\n");
	}

	return ShaderInterfaceFactory;
"""

content = re.sub(r'CreateInterfaceFn CShaderDeviceMgrVulkan::SetMode.*?\n\{.*?\n.*?\}', 
                 r'CreateInterfaceFn CShaderDeviceMgrVulkan::SetMode( void *hWnd, int nAdapter, const ShaderDeviceInfo_t& mode )\n{\n' + init_code + '\n}', content, flags=re.DOTALL)

with open("materialsystem/shaderapivulkan/shaderapivulkan.cpp", "w") as f:
    f.write(content)

