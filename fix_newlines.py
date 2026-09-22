with open("materialsystem/shaderapivulkan/shaderapivulkan.cpp", "r") as f:
    content = f.read()

content = content.replace('%\nd",', '%d\\n",')
content = content.replace('instance!\n");', 'instance!\\n");')
content = content.replace('Testchamber 15)\n");', 'Testchamber 15)\\n");')

with open("materialsystem/shaderapivulkan/shaderapivulkan.cpp", "w") as f:
    f.write(content)
