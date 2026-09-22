with open("wscript", "r") as f:
    content = f.read()

if "'materialsystem/shaderapivulkan'," not in content:
    content = content.replace("'materialsystem/shaderapiempty',", "'materialsystem/shaderapiempty',\n\t\t'materialsystem/shaderapivulkan',")

with open("wscript", "w") as f:
    f.write(content)
