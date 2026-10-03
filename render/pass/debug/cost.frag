// Render core cost overlay. Original 3x5 diagnostic font; no external asset.
#version 450
#include "../../shaders/common/color_encoding.glsl"
layout( constant_id = 0 ) const bool kEncode = false;
layout( push_constant ) uniform Row
{
    vec4 rect; // origin in pixels, display scale
    vec4 metrics; // CPU/GPU fractions of 16.67 ms, measured row
    uvec4 text[5]; // 80 packed ASCII characters
} row;
layout( location = 0 ) out vec4 outColor;
const uint glyphs[64] = uint[64](
    0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
    17556u, 5265u, 0u, 0u, 0u, 448u, 8192u, 4772u,
    31599u, 29850u, 29347u, 14499u, 18925u, 14543u, 31694u, 9383u,
    31727u, 14831u, 1040u, 0u, 0u, 0u, 0u, 0u,
    0u, 23530u, 15083u, 25166u, 15211u, 29391u, 4815u, 27470u,
    23533u, 29847u, 11044u, 23277u, 29257u, 23549u, 24573u, 11114u,
    4843u, 28522u, 23275u, 14478u, 9367u, 31597u, 11117u, 24557u,
    23213u, 9389u, 29351u, 0u, 0u, 0u, 0u, 28672u);
vec3 Heat( float t )
{
    if ( t < 0.333333 ) return mix( vec3( 0.02, 0.15, 1 ), vec3( 0, 0.8, 0.2 ), t * 3 );
    if ( t < 0.666667 ) return mix( vec3( 0, 0.8, 0.2 ), vec3( 1, 0.8, 0 ), t * 3 - 1 );
    return mix( vec3( 1, 0.8, 0 ), vec3( 1, 0.03, 0.01 ), t * 3 - 2 );
}
void main()
{
    vec2 p = ( gl_FragCoord.xy - row.rect.xy ) / row.rect.z;
    vec3 color = vec3( 0.008, 0.012, 0.022 );
    if ( row.metrics.z > 0.5 && p.y >= 15 && p.y < 19 )
    {
        float fraction = p.x < 320 ? row.metrics.x : row.metrics.y;
        float x = mod( p.x, 320.0 );
        color = x < fraction * 318.0 ? Heat( fraction ) : vec3( 0.035 );
    }
    ivec2 cell = ivec2( floor( ( p - vec2( 4, 2 ) ) / 2.0 ) );
    int character = cell.x / 4;
    if ( cell.x >= 0 && cell.y >= 0 && cell.y < 5 && character < 79 && cell.x % 4 < 3 )
    {
        int word = character / 4;
        uint ascii = ( row.text[word / 4][word % 4] >> ( 8 * ( character % 4 ) ) ) & 255u;
        uint bits = ascii >= 32u && ascii < 96u ? glyphs[ascii - 32u] : 0u;
        if ( ( bits & ( 1u << uint( cell.y * 3 + cell.x % 4 ) ) ) != 0u )
            color = vec3( 0.9 );
    }
    outColor = vec4( kEncode ? LinearToSrgb( color ) : color, 0.94 );
}
