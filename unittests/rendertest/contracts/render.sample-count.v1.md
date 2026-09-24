# render.sample-count.v1

`public/render/render_sample_count.h` owns MSAA sample-count policy. A
supported-count mask has one bit per power-of-two count, and the bit's value is
the count (the `VkSampleCountFlags` layout).

- `IsMsaaModeSupported( n, mask )` answers `SupportsMSAAMode`. It is true only
  for a power of two of at least 2 that is present in the mask, so the old 6x
  vendor mode is never supported.
- `ClampSampleCount( requested, mask )` picks the count a device actually uses,
  and the count a recommended configuration may offer. It returns the largest
  supported count not above the request, or 1. It never rounds up.

The suite checks both functions exhaustively over all 7-bit masks and requests
-1..70. Its sensitivity build rounds up, which the oracle must reject.
