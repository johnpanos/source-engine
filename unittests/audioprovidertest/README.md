# Audio provider conformance

`engine/wscript` owns compiled playback, capture, and codec availability and
exports those facts to the launcher. The launcher owns selection order, explicit
required-provider behavior, legacy protocol mappings, and the local voice default.
The engine receives typed plans before Connect; its sound and voice consumers
use narrow private bridges to those plans. Native SDK headers and vendor probes
remain in the corresponding implementations.

The portable fixtures exercise the actual selection implementation, including
failure before acquisition, explicit null fallback, restart/wave-only policy,
case-insensitive negotiated protocol names, duplicate or malformed catalogs,
recording failure and fallback, and preservation of sample rates and quality.
The linked-engine fixture checks actual binding, copied plan ownership, invalid
replacement rollback, and consumption through the same bridge as sound startup.

The MP3 fixture calls the actual linked minimp3 service twice with the generated
signal in [fixtures](fixtures/README.md). It checks format, output bounds, signal
energy and frequency, and a deliberately silent negative input to the oracle.
Every stream is destroyed before its service. The minimp3 adapter now closes its
stream buffers/index on destruction and rejects a missing input handler.

Run the portable release assertions without configuring an engine build:

```sh
python3 -m unittest discover -s tools/quality/tests -p test_audio_providers.py -v
```

For a configured Portal profile, build with its existing Waf lock and run against
that build's library directories:

```sh
WAFLOCK=.lock-waf-portal-vulkan ./waf build \
  --targets=audio_provider_conformance,audio_media_conformance,audio_binding_conformance,mp3_provider_conformance
build-portal-vulkan/unittests/audioprovidertest/audio_provider_conformance
build-portal-vulkan/unittests/audioprovidertest/audio_media_conformance
build-portal-vulkan/unittests/audioprovidertest/audio_binding_conformance
build-portal-vulkan/unittests/audioprovidertest/mp3_provider_conformance \
  unittests/audioprovidertest/fixtures/tone_44100_mono.mp3
```

The linked fixtures require the shared libraries from the same profile on
`LD_LIBRARY_PATH`. The deliberately limited `--tests` graph builds only the
portable catalog fixtures. Real SDL3 recorder callback, restart, and teardown
coverage is in [the existing recording fixture](../sdl3test/README.md).

An Opus-enabled Linux client profile also builds `opus_provider_conformance`.
It exercises actual quality-3 packet size and decode, independent channel state,
reset equivalence, and repeated cleanup. It rejects qualities -1, 4, and INT_MAX.
The launcher retains the previous missing-CELT/Speex-to-Opus fallback and original
negotiated quality; this does not establish equivalence between codec formats.
Speex's legacy quality 4 previously indexed beyond Opus's four mode records. That
unsupported mode now fails explicitly instead of reading invalid memory; the
quality is not silently changed. A profile with Opus disabled has no non-Steam
voice codec binding. Steam voice remains its existing separate API path.

These tests do not certify physical microphones, speakers, other OS native
backends, or an Opus-enabled installed product. Product playback and shutdown
are verified by the installed Portal e2e runner and its actual provider logs.
