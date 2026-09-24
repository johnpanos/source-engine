# render.gamma-ramp.v1

`public/render/render_gamma_ramp.h` owns the monitor gamma curve behind
`IShaderDevice::SetHardwareGammaRamp` (`mat_monitorgamma` and
`mat_monitorgamma_tv_*`). Entry `i` is `clamp(pow(i/255, gamma/2.2))`. With TV
mode it is then raised to `2.2/tvExponent` and scaled into
`[tvRangeMin, tvRangeMax]/255`. Values truncate to 16 bits, exactly as the D3D9
device computed them. The D3D9 device passes the ramp to the hardware ramp.
Providers without a window-system ramp (SDL3, native Vulkan) apply it at present
time. `IsIdentityAt8Bit` identifies ramps that cannot change an 8-bit value
(round-to-nearest, `GammaRampEntryTo8Bit`); presenters may skip the ramp for
them. The default gamma of 2.2 is such a ramp.

The suite compares ramps at gamma 1.6–2.6 and in TV mode with a double-precision
analytic oracle (within 2 of 65535). It also checks monotonicity, endpoints, the
identity classification and 8-bit rounding. Its sensitivity build uses the
reciprocal exponent, which the oracle must reject. On D3D9 the ramp only takes
effect in fullscreen. Present-time providers apply it in windowed mode as well;
that divergence is recorded, not hidden.
