import sys

with open("tools/quality/product_profile.py", "r") as f:
    lines = f.readlines()

new_lines = []
in_dep_check = False
for line in lines:
    if 'dep = profile["dependencies"]["dxvk_native"]' in line:
        new_lines.append('        if "dxvk_native" in profile["dependencies"]:\n')
        new_lines.append('            dep = profile["dependencies"]["dxvk_native"]\n')
        in_dep_check = True
    elif in_dep_check:
        if line.startswith('        return profile'):
            in_dep_check = False
            new_lines.append(line)
        else:
            new_lines.append('    ' + line)
    else:
        new_lines.append(line)

with open("tools/quality/product_profile.py", "w") as f:
    f.writelines(new_lines)

