with open("materialsystem/wscript", "r") as f:
    content = f.read()

content = content.replace("libs += ['shaderapiempty' if bld.env.DEDICATED else 'shaderapidx9']",
                          "if bld.env.DEDICATED:\n\t\tlibs += ['shaderapiempty']\n\telif bld.env.NATIVE_VULKAN:\n\t\tlibs += ['shaderapivulkan']\n\telse:\n\t\tlibs += ['shaderapidx9']")
content = content.replace("if not bld.env.DEDICATED:\n\t\tlibs += ['stdshader_dx9']",
                          "if not bld.env.DEDICATED and not bld.env.NATIVE_VULKAN:\n\t\tlibs += ['stdshader_dx9']")

with open("materialsystem/wscript", "w") as f:
    f.write(content)
