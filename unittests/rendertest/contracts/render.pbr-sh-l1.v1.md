# render.pbr-sh-l1.v1

RFC 0007 fits one scalar irradiance channel from four directions. The caller
supplies the flat normal and three RNM directions owned by the map lighting
layout, together with their irradiance samples. The result evaluates
`E(n) = constant + x*n.x + y*n.y + z*n.z`; these coefficients already contain
the chosen SH basis and clamped-cosine factors. The function does not clamp
negative extrapolated values. RGB callers fit each channel independently.

The four sample directions must be noncoplanar and all inputs finite. A failed
fit leaves the output untouched. The shared suite checks exact affine recovery,
unseen-direction evaluation, uniform irradiance, singular and nonfinite input,
and a swapped-basis negative control. It also measures a grazing point-light
case that exposes the four-sample approximation error. Passing this math suite
does not certify the quality of SH L1 for arbitrary lights or maps; the baker's
directional-error budget and any extra sample policy remain to be established.
