#version 450
layout(set=3,binding=0) uniform texture2D scene;
layout(set=3,binding=1) uniform texture2D depth;
layout(set=3,binding=2) uniform texture2D motion;
layout(set=3,binding=3) uniform sampler pointSampler;
layout(push_constant) uniform Constants { ivec4 region; } draw;
layout(location=0) out vec4 outColor;
layout(location=1) out vec2 outMotion;
void main()
{
 ivec2 p = ivec2(gl_FragCoord.xy) + draw.region.xy;
 vec4 color = texelFetch(sampler2D(scene, pointSampler), p, 0);
 if (draw.region.z != 0)
  color.rgb = mix(color.rgb / 12.92, pow((color.rgb + 0.055) / 1.055, vec3(2.4)),
      greaterThan(color.rgb, vec3(0.04045)));
 outColor = color;
 outMotion = texelFetch(sampler2D(motion, pointSampler), p, 0).xy;
 gl_FragDepth = texelFetch(sampler2D(depth, pointSampler), p, 0).r;
}
