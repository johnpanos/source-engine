with open("wscript", "r") as f:
    content = f.read()

if "conf.check_cfg(package='vulkan'" not in content:
    content = content.replace("conf.check_cfg(package=package, uselib_store=store, args=['--cflags', '--libs'])",
                              "conf.check_cfg(package=package, uselib_store=store, args=['--cflags', '--libs'])\n\t\tconf.check_cfg(package='vulkan', uselib_store='VULKAN', args=['--cflags', '--libs'])")

with open("wscript", "w") as f:
    f.write(content)
