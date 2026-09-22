import json

with open('architecture/modules.json', 'r') as f:
    data = json.load(f)

modules = data['capabilityModules']['modules']
modules.append({
    "id": "platform.posix",
    "kind": "backend",
    "paths": ["platform/posix/"],
    "allowedEdges": ["foundation", "platform.contracts"],
    "externalHeaders": ["unistd.h", "sys/wait.h", "sys/types.h", "poll.h", "fcntl.h", "csignal", "cerrno"]
})

with open('architecture/modules.json', 'w') as f:
    json.dump(data, f, indent=2)
