import re
with open("materialsystem/shaderapivulkan/shaderapivulkan.cpp", "r") as f:
    content = f.read()

header = """
#include <vulkan/vulkan.h>
#include <SDL3/SDL_vulkan.h>
"""
content = header + content

init_code = """CreateInterfaceFn CShaderDeviceMgrVulkan::SetMode( void *hWnd, int nAdapter, const ShaderDeviceInfo_t& mode )
{
\tMsg("[NativeVulkan] Setting mode for adapter %d\\n", nAdapter);
\t::VkInstanceCreateInfo createInfo = {};
\tcreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
\t::VkApplicationInfo appInfo = {};
\tappInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
\tappInfo.pApplicationName = "Source Engine Native Vulkan";
\tappInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
\tappInfo.pEngineName = "Source";
\tappInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
\tappInfo.apiVersion = VK_API_VERSION_1_0;
\tcreateInfo.pApplicationInfo = &appInfo;
\t::VkInstance instance;
\tif (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
\t\tWarning("[NativeVulkan] Failed to create Vulkan instance!\\n");
\t} else {
\t\tMsg("[NativeVulkan] Successfully created Vulkan instance!\\n");
\t\tMsg("[NativeVulkan] (Rank 14 Bootstrap complete. Rank 16 MVP needed for Testchamber 15)\\n");
\t}
\treturn ShaderInterfaceFactory;
}"""

content = re.sub(r'CreateInterfaceFn CShaderDeviceMgrVulkan::SetMode.*?return ShaderInterfaceFactory;\n\}', init_code, content, flags=re.DOTALL)

with open("materialsystem/shaderapivulkan/shaderapivulkan.cpp", "w") as f:
    f.write(content)
