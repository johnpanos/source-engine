# SDL3 provider conformance

The native recording fixture compiles the actual SDL3 recording implementation.
It checks sample order, exact-capacity wrap, bounded retention of newest samples,
invalid arguments, capture output, stop/restart, repeated instances, and balanced
audio-subsystem ownership. Checks remain active in release builds. The dummy
driver avoids access to the user's microphone; it does not certify a physical
recording device.

After configuring the SDL3 product profile, build and run with its Waf lock:

```sh
WAFLOCK=.lock-waf-portal-vulkan ./waf build --targets=sdl3_voice_record_test
SDL_AUDIO_DRIVER=dummy LD_LIBRARY_PATH=build-portal-vulkan/tier0 \
  build-portal-vulkan/inputsystem/sdl3_voice_record_test
```

The capture ring has one SDL stream callback producer and one voice-consumer
thread. The SDL stream lock serializes both access paths. Capacity is exactly
three seconds of mono PCM at the configured rate. Overflow retains the newest
samples; unlike required gameplay work, microphone history is intentionally
lossy. Stream destruction synchronizes callback completion before buffer release.

Installed Portal playback and graphics acceptance uses
`tools/quality/portal_boot.py`; it records active map/player state, fresh image
capture, clean shutdown, loaded libraries, and actual provider diagnostics.
