import re
with open("materialsystem/shaderapivulkan/shaderapivulkan.cpp", "r") as f:
    content = f.read()

content = content.replace('Msg("[NativeVulkan] (Rank 14 Bootstrap complete. Rank 16 MVP needed for Testchamber 15)\n");',
                          'Msg("[NativeVulkan] (Rank 14 Bootstrap complete. Rank 16 MVP needed for Testchamber 15)\\n");')
content = content.replace('VkInstanceCreateInfo createInfo = {};',
                          '::VkInstanceCreateInfo createInfo = {};')

with open("materialsystem/shaderapivulkan/shaderapivulkan.cpp", "w") as f:
    f.write(content)
