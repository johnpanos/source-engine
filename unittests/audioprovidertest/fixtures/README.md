# Generated MP3 fixture

`tone_44100_mono.mp3` is a generated 0.25 second, 440 Hz mono sine at 44,100 Hz,
encoded with FFmpeg 8.1.2 / libmp3lame at 96 kbit/s. It contains no third-party
recording or game content. Encoder delay and frame padding extend the encoded
stream to approximately 0.287 seconds; the independent oracle inspects its
steady signal region using energy and zero-crossing ranges.

```sh
ffmpeg -nostdin -hide_banner -loglevel error -f lavfi \
  -i 'sine=frequency=440:sample_rate=44100:duration=0.25' \
  -c:a libmp3lame -b:a 96k -map_metadata -1 -write_xing 0 -id3v2_version 0 \
  tone_44100_mono.mp3
```

SHA-256: `60ff85b5b12c7b8560f212edaa041b518f03db84141a3338923a2d1659848d23`.
