import re

with open("wscript", "r") as f:
    content = f.read()

content = content.replace("conf.env.DXVK_NATIVE = (conf.options.render_backend == 'vulkan')",
                          "conf.env.DXVK_NATIVE = (conf.options.render_backend == 'vulkan')\n\tconf.env.NATIVE_VULKAN = (conf.options.render_backend == 'native-vulkan')")

if "'materialsystem/shaderapivulkan'," not in content:
    content = content.replace("'materialsystem/shaderapiempty',", "'materialsystem/shaderapiempty',\n\t\t'materialsystem/shaderapivulkan',")

with open("wscript", "w") as f:
    f.write(content)
