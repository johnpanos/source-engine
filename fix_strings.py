with open("materialsystem/shaderapivulkan/shaderapivulkan.cpp", "r") as f:
    content = f.read()

content = content.replace('Msg("[NativeVulkan] Setting mode for adapter %d\n", nAdapter);', 
                          'Msg("[NativeVulkan] Setting mode for adapter %d\\n", nAdapter);')

with open("materialsystem/shaderapivulkan/shaderapivulkan.cpp", "w") as f:
    f.write(content)
