with open("wscript", "r") as f:
    content = f.read()
content = content.replace("conf.env.DXVK = conf.options.RENDER_BACKEND == 'vulkan'",
                          "conf.env.DXVK = conf.options.RENDER_BACKEND == 'vulkan'\n\tconf.env.NATIVE_VULKAN = conf.options.RENDER_BACKEND == 'native-vulkan'")
content = content.replace("if conf.env.SDL3 or conf.env.DXVK:",
                          "if conf.env.SDL3 or conf.env.DXVK or conf.env.NATIVE_VULKAN:")
with open("wscript", "w") as f:
    f.write(content)
