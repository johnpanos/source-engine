#!/bin/bash
# Wrapper script to boot Portal with the Box3D physics backend

# Use the established runtime from the native baseline batch job
RUNTIME_DIR="$(pwd)/quality-results/job-batch-native-20260922T090958Z/legacy-testchmb_a_00/runtime"

if [ ! -d "$RUNTIME_DIR" ]; then
    echo "Error: Native batch runtime directory not found."
    exit 1
fi

# Ensure our newly built box3d module is synchronized into the runtime bin
echo "Syncing Box3D module to runtime..."
cp "$(pwd)/build/vphysics_box3d/libvphysics_box3d.so" "$RUNTIME_DIR/bin/"
cp "$(pwd)/build/vphysics_box3d/libvphysics_box3d.so" "$RUNTIME_DIR/bin/vphysics_box3d.so"

cd "$RUNTIME_DIR" || exit 1

export SDL_VIDEO_DRIVER=wayland
export SDL_VIDEODRIVER=wayland
export DXVK_WSI_DRIVER=SDL3
export SteamAppId=400
export SteamGameId=400
export LD_LIBRARY_PATH="$PWD/bin:$LD_LIBRARY_PATH"

echo "Booting Portal with Box3D..."
./hl2_launcher -game portal -windowed -w 1920 -h 1080 -novid -insecure -renderer vulkan-compat +mat_queue_mode 0 +fps_max 120 +map testchmb_a_00 -physics box3d "$@"
