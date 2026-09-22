with open("materialsystem/shaderapivulkan/wscript", "r") as f:
    content = f.read()

content = content.replace("libs = ['tier0','tier1']", "libs = ['tier0','tier1','VULKAN','SDL3']")

with open("materialsystem/shaderapivulkan/wscript", "w") as f:
    f.write(content)
