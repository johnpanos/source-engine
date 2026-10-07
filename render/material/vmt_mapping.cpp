//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The VMT mapping (RFC 0016 K4): which legacy shaders map onto a
//			core family and why, how each VMT key of a family is read, and the
//			families' schemas built from those rows.
//
//=============================================================================//

#include "render/device/bind_group.h"
#include "render/material/vmt_mapping.h"
#include "render/pbr_material_schema.h"

#include <cstdlib>
#include <string>

namespace render::material
{

namespace
{

// Shaders that map cleanly onto a core family. Every other shader the product
// registers (legacy_shaders.inc) runs in the legacy family.
constexpr VmtShaderRow kShaders[] = {
    { "lightmappedgeneric", "lightmapped",
        "world surfaces lit by lightmaps (flat, bumped and self-shadowed bump); the native "
        "port lightmapped.frag of lightmappedgeneric_ps2_3_x draws them" },
    { "worldvertextransition_dx9", "lightmapped",
        "two-layer displacement blends: LightmappedGeneric with $basetexture2 and the "
        "vertex-alpha blend, drawn by the same lightmapped port" },
    { "vertexlitgeneric", "vertexlit",
        "models lit by the ambient cube and vertex or per-pixel lights, including the $phong "
        "skin path (skin_vs20/skin_ps20b, vertexlit_and_unlit_generic ports)" },
    { "unlitgeneric", "unlit",
        "unlit surfaces: base texture times color, with detail and envmap; the "
        "vertexlit_and_unlit_generic port without lighting" },
    { "unlittwotexture", "unlit", "two independently transformed color textures multiplied" },
    { "unlittwotexture_dx9", "unlit", "UnlitTwoTexture's DirectX 9 implementation" },
    { "sprite_dx9", "unlit",
        "sprites: an unlit textured quad whose orientation and render mode are parameters "
        "(sprite_vs20/sprite_ps20b)" },
    { "sky_hdr_dx9", "unlit",
        "the 2D sky box's faces (Sky, render.pass.sky): an unlit textured quad whose base is an "
        "HDR encoding ($hdrcompressedtexture RGBS times 8, sky_hdr_compressed_rgbs_ps2x)" },
    { "sky_dx9", "unlit", "the sky box's faces without HDR: $basetexture times $color" },
    { "decalmodulate", "decal-modulate",
        "dimensionless surface factors with modulate-2x blending" },
    { "decalmodulate_dx9", "decal-modulate", "DecalModulate DirectX 9 implementation" },
    { "cable", "cable", "CPU-expanded rope ribbons with normal UV0 and color UV1" },
    { "cable_dx9", "cable", "Cable's expanded ribbon implementation" },
    { "splinerope", "cable", "this client expands SplineRope ribbons before drawing" },
    { "pbrmetalrough", "pbr",
        "RFC 0007 metalness/roughness materials, drawn natively by world_pbr and model_pbr; "
        "the schema is render/pbr_material_schema.h" },
    { "water", "water",
        "water surfaces: Portal 2's Water shader (water_ps2x's expensive path: flowing normal "
        "maps, the flowing sludge layer, lightmapped water fog and a fresnel reflection of the "
        "view's reflection target or of an env map); render/material/water_family.h" },
    { "water_dx9_hdr", "water",
        "Water's fallback on an HDR DirectX 9 profile (DEFINE_FALLBACK_SHADER), the same shader" },
    { "water_dx90", "water", "Water's DirectX 9 shader, which Water_DX9_HDR inherits" },
    { "refract", "refract", "Refract's scene-color transmission on model surfaces" },
    { "refract_dx90", "refract", "Refract's DirectX 9 implementation" },
    { "writez_dx9", "depth", "depth-only material-system geometry" },
    { "writez", "depth", "depth-only material-system geometry" },
    { "portalrefract", "portal-mask", "PortalRefract stage 1 aperture mask" },
    { "portalrefract_dx9", "portal-mask", "PortalRefract stage 1 aperture mask" },
    { "subrect", kLegacyFamily,
        "not a shader: a sub-rectangle ($pos, $size) of another material ($material) that "
        "the material system resolves (CMaterialSubRect) and draws as that material" },
};

constexpr std::string_view kLegacyReason =
    "no core family yet: the legacy frontend runs the shader's stdshader_dx9 programs "
    "through the ports (RFC 0016 legacy family)";

// Keys every legacy-derived family reads (BaseShader's standard parameters and
// the material flags the draw state depends on); BuildKeyRows() gives each of
// kLegacyDerivedFamilies a copy.
constexpr std::string_view kLegacyDerivedFamilies[] = {
    "lightmapped", "vertexlit", "unlit", "depth", "portal-mask", "cable", "decal-modulate" };

constexpr VmtKeyRow kCommonKeys[] = {
    { {}, "$one", "one", ValueKind::kFloat, "1" },
    { {}, "$zero", "zero", ValueKind::kFloat, "0" },
    { {}, "$basetexture", "basetexture", ValueKind::kTexture, "" },
    { {}, "$frame", "frame", ValueKind::kInt, "0" },
    { {}, "$color", "color", ValueKind::kFloat3, "[1 1 1]" },
    { {}, "$alpha", "alpha", ValueKind::kFloat, "1" },
    { {}, "$detail", "detail", ValueKind::kTexture, "" },
    { {}, "$detailscale", "detailscale", ValueKind::kFloat, "4" },
    { {}, "$detailblendmode", "detailblendmode", ValueKind::kInt, "0" },
    { {}, "$detailblendfactor", "detailblendfactor", ValueKind::kFloat, "1" },
    { {}, "$detailtint", "detailtint", ValueKind::kFloat3, "[1 1 1]" },
    { {}, "$envmap", "envmap", ValueKind::kTexture, "" },
    { {}, "$envmapmask", "envmapmask", ValueKind::kTexture, "" },
    { {}, "$envmaptint", "envmaptint", ValueKind::kFloat3, "[1 1 1]" },
    { {}, "$envmapcontrast", "envmapcontrast", ValueKind::kFloat, "0" },
    { {}, "$envmapsaturation", "envmapsaturation", ValueKind::kFloat, "1" },
    { {}, "$alphatestreference", "alphatestreference", ValueKind::kFloat, "0" },
    { {}, "$translucent", "translucent", ValueKind::kBool, "0" },
    { {}, "$alphatest", "alphatest", ValueKind::kBool, "0" },
    { {}, "$additive", "additive", ValueKind::kBool, "0" },
    { {}, "$nocull", "nocull", ValueKind::kBool, "0" },
    { {}, "$nofog", "nofog", ValueKind::kBool, "0" },
    { {}, "$ignorez", "ignorez", ValueKind::kBool, "0" },
    { {}, "$decal", "decal", ValueKind::kBool, "0" },
    { {}, "$selfillum", "selfillum", ValueKind::kBool, "0" },
    { {}, "$vertexcolor", "vertexcolor", ValueKind::kBool, "0" },
    { {}, "$vertexalpha", "vertexalpha", ValueKind::kBool, "0" },
    { {}, "$basealphaenvmapmask", "basealphaenvmapmask", ValueKind::kBool, "0" },
    { {}, "$normalmapalphaenvmapmask", "normalmapalphaenvmapmask", ValueKind::kBool, "0" },
    { {}, "$allowalphatocoverage", "allowalphatocoverage", ValueKind::kBool, "0" },
    { {}, "$znearer", "znearer", ValueKind::kBool, "0" },
    { {}, "$no_draw", "no_draw", ValueKind::kBool, "0" },
    { {}, "$model", "model", ValueKind::kBool, "0" },
    { {}, "$basetexturetransform", "basetexturetransform", ValueKind::kTransform, "" },
    { {}, "$detailtexturetransform", "detailtexturetransform", ValueKind::kTransform, "" },
    { {}, "$envmapmasktransform", "envmapmasktransform", ValueKind::kTransform, "" },
};

constexpr VmtKeyRow kLegacyDerivedKeys[] = {
    { "decal-modulate", "$decalscale", "decalscale", ValueKind::kFloat, "1" },
    { "portal-mask", "$stage", "stage", ValueKind::kInt, "0" },
    { "portal-mask", "$portalopenamount", "portalopenamount", ValueKind::kFloat, "0" },
    { "portal-mask", "$portalstatic", "portalstatic", ValueKind::kFloat, "0" },
    { "portal-mask", "$portalmasktexture", "portalmasktexture", ValueKind::kTexture, "" },
    { "portal-mask", "$portalcolortexture", "portalcolortexture", ValueKind::kTexture, "" },
    { "portal-mask", "$portalcolorscale", "portalcolorscale", ValueKind::kFloat, "0" },
    { "portal-mask", "$texturetransform", "texturetransform", ValueKind::kTransform, "" },
    { "portal-mask", "$time", "time", ValueKind::kFloat, "0" },
    { "lightmapped", "$alpha2", "alpha2", ValueKind::kFloat, "1" },
    { "lightmapped", "$basetexture2", "basetexture2", ValueKind::kTexture, "" },
    { "lightmapped", "$frame2", "frame2", ValueKind::kInt, "0" },
    { "lightmapped", "$bumpmap", "bumpmap", ValueKind::kTexture, "" },
    { "lightmapped", "$bumpframe", "bumpframe", ValueKind::kInt, "0" },
    { "lightmapped", "$bumptransform2", "bumptransform2", ValueKind::kTransform, "" },
    { "lightmapped", "$bumpmap2", "bumpmap2", ValueKind::kTexture, "" },
    { "lightmapped", "$ssbump", "ssbump", ValueKind::kBool, "0" },
    // Portal 2's LightmappedGeneric parameters this SDK's shader does not
    // declare (the backend's port reads them from the material).
    { "lightmapped", "$ssbumpmathfix", "ssbumpmathfix", ValueKind::kBool, "0" },
    { "lightmapped", "$envmaplightscale", "envmaplightscale", ValueKind::kFloat, "0" },
    { "lightmapped", "$envmaplightscaleminmax", "envmaplightscaleminmax", ValueKind::kFloat2,
        "[0 1]" },
    { "lightmapped", "$blendmodulatetexture", "blendmodulatetexture", ValueKind::kTexture, "" },
    { "lightmapped", "$seamless_scale", "seamless_scale", ValueKind::kFloat, "0" },
    { "lightmapped", "$fresnelreflection", "fresnelreflection", ValueKind::kFloat, "1" },
    { "lightmapped", "$nodiffusebumplighting", "nodiffusebumplighting", ValueKind::kBool, "0" },
    { "lightmapped", "$basetexturenoenvmap", "basetexturenoenvmap", ValueKind::kBool, "0" },
    { "lightmapped", "$basetexture2noenvmap", "basetexture2noenvmap", ValueKind::kBool, "0" },
    { "lightmapped", "$selfillumtint", "selfillumtint", ValueKind::kFloat3, "[1 1 1]" },
    { "lightmapped", "$masked", "masked", ValueKind::kBool, "0" },
    { "lightmapped", "$bumptransform", "bumptransform", ValueKind::kTransform, "" },
    { "lightmapped", "$basetexturetransform2", "basetexturetransform2", ValueKind::kTransform, "" },
    { "lightmapped", "$blendmasktransform", "blendmasktransform", ValueKind::kTransform, "" },

    { "vertexlit", "$bumpmap", "bumpmap", ValueKind::kTexture, "" },
    { "vertexlit", "$ssbump", "ssbump", ValueKind::kBool, "0" },
    { "vertexlit", "$ssbumpmathfix", "ssbumpmathfix", ValueKind::kBool, "0" },
    { "vertexlit", "$bumpframe", "bumpframe", ValueKind::kInt, "0" },
    { "vertexlit", "$selfillumtint", "selfillumtint", ValueKind::kFloat3, "[1 1 1]" },
    { "vertexlit", "$selfillumfresnel", "selfillumfresnel", ValueKind::kBool, "0" },
    { "vertexlit", "$selfillumfresnelminmaxexp", "selfillumfresnelminmaxexp", ValueKind::kFloat3,
        "[0 1 1]" },
    { "vertexlit", "$depthblendscale", "depthblendscale", ValueKind::kFloat, "0" },
    { "vertexlit", "$selfillummask", "selfillummask", ValueKind::kTexture, "" },
    { "vertexlit", "$color2", "color2", ValueKind::kFloat3, "[1 1 1]" },
    { "vertexlit", "$halflambert", "halflambert", ValueKind::kBool, "0" },
    { "vertexlit", "$phong", "phong", ValueKind::kBool, "0" },
    { "vertexlit", "$forcephong", "forcephong", ValueKind::kBool, "0" },
    { "vertexlit", "$ignore_alpha_modulation", "ignore_alpha_modulation", ValueKind::kBool, "0" },
    { "vertexlit", "$phongexponent", "phongexponent", ValueKind::kFloat, "5" },
    { "vertexlit", "$phongboost", "phongboost", ValueKind::kFloat, "1" },
    { "vertexlit", "$phongtint", "phongtint", ValueKind::kFloat3, "[1 1 1]" },
    { "vertexlit", "$phongfresnelranges", "phongfresnelranges", ValueKind::kFloat3, "[0 0.5 1]" },
    { "vertexlit", "$phongexponenttexture", "phongexponenttexture", ValueKind::kTexture, "" },
    { "vertexlit", "$phongalbedotint", "phongalbedotint", ValueKind::kBool, "0" },
    { "vertexlit", "$blendtintbybasealpha", "blendtintbybasealpha", ValueKind::kBool, "0" },
    { "vertexlit", "$blendtintcoloroverbase", "blendtintcoloroverbase", ValueKind::kFloat, "0" },
    { "vertexlit", "$basemapalphaphongmask", "basemapalphaphongmask", ValueKind::kBool, "0" },
    { "vertexlit", "$invertphongmask", "invertphongmask", ValueKind::kBool, "0" },
    { "vertexlit", "$lightwarptexture", "lightwarptexture", ValueKind::kTexture, "" },
    { "vertexlit", "$phongwarptexture", "phongwarptexture", ValueKind::kTexture, "" },
    { "vertexlit", "$envmapfresnel", "envmapfresnel", ValueKind::kFloat, "0" },
    { "vertexlit", "$envmapfresnelminmaxexp", "envmapfresnelminmaxexp", ValueKind::kFloat3,
        "[1 1 1]" },
    { "vertexlit", "$envmaplightscale", "envmaplightscale", ValueKind::kFloat, "0" },
    { "vertexlit", "$envmaplightscaleminmax", "envmaplightscaleminmax", ValueKind::kFloat2,
        "[0 1]" },
    { "vertexlit", "$rimlight", "rimlight", ValueKind::kBool, "0" },
    { "vertexlit", "$rimlightexponent", "rimlightexponent", ValueKind::kFloat, "4" },
    { "vertexlit", "$rimlightboost", "rimlightboost", ValueKind::kFloat, "1" },
    { "vertexlit", "$rimmask", "rimmask", ValueKind::kBool, "0" },
    { "vertexlit", "$ambientonly", "ambientonly", ValueKind::kBool, "0" },
    { "vertexlit", "$bumptransform", "bumptransform", ValueKind::kTransform, "" },
    { "vertexlit", "$treesway", "treesway", ValueKind::kInt, "0" },
    { "vertexlit", "$treeswayheight", "treeswayheight", ValueKind::kFloat, "1000" },
    { "vertexlit", "$treeswaystartheight", "treeswaystartheight", ValueKind::kFloat, "0.1" },
    { "vertexlit", "$treeswayradius", "treeswayradius", ValueKind::kFloat, "300" },
    { "vertexlit", "$treeswaystartradius", "treeswaystartradius", ValueKind::kFloat, "0.2" },
    { "vertexlit", "$treeswayspeed", "treeswayspeed", ValueKind::kFloat, "1" },
    { "vertexlit", "$treeswayspeedhighwindmultiplier", "treeswayspeedhighwindmultiplier",
        ValueKind::kFloat, "2" },
    { "vertexlit", "$treeswaystrength", "treeswaystrength", ValueKind::kFloat, "10" },
    { "vertexlit", "$treeswayscrumblespeed", "treeswayscrumblespeed", ValueKind::kFloat, "5" },
    { "vertexlit", "$treeswayscrumblestrength", "treeswayscrumblestrength", ValueKind::kFloat,
        "10" },
    { "vertexlit", "$treeswayscrumblefrequency", "treeswayscrumblefrequency", ValueKind::kFloat,
        "12" },
    { "vertexlit", "$treeswayfalloffexp", "treeswayfalloffexp", ValueKind::kFloat, "1.5" },
    { "vertexlit", "$treeswayscrumblefalloffexp", "treeswayscrumblefalloffexp", ValueKind::kFloat,
        "1" },
    { "vertexlit", "$treeswayspeedlerpstart", "treeswayspeedlerpstart", ValueKind::kFloat, "3" },
    { "vertexlit", "$treeswayspeedlerpend", "treeswayspeedlerpend", ValueKind::kFloat, "6" },
    { "vertexlit", "$treeswaystatic", "treeswaystatic", ValueKind::kBool, "0" },
    { "vertexlit", "$lowqualityflashlightshadows", "lowqualityflashlightshadows", ValueKind::kBool,
        "0" },

    { "cable", "$bumpmap", "bumpmap", ValueKind::kTexture, "" },
    { "cable", "$minlight", "minlight", ValueKind::kFloat, "0.1" },
    { "cable", "$maxlight", "maxlight", ValueKind::kFloat, "0.3" },
    { "unlit", "$hdrbasetexture", "hdrbasetexture", ValueKind::kTexture, "" },
    { "unlit", "$hdrcompressedtexture", "hdrcompressedtexture", ValueKind::kTexture, "" },
    { "unlit", "$hdrcompressedtexture0", "hdrcompressedtexture0", ValueKind::kTexture, "" },
    { "unlit", "$hdrcompressedtexture1", "hdrcompressedtexture1", ValueKind::kTexture, "" },
    { "unlit", "$hdrcompressedtexture2", "hdrcompressedtexture2", ValueKind::kTexture, "" },
    { "unlit", "$vertexalphatest", "vertexalphatest", ValueKind::kBool, "0" },
    { "unlit", "$texture2", "texture2", ValueKind::kTexture, "" },
    { "unlit", "$frame2", "frame2", ValueKind::kInt, "0" },
    { "unlit", "$texture2transform", "texture2transform", ValueKind::kTransform, "" },
    { "unlit", "$hdrcolorscale", "hdrcolorscale", ValueKind::kFloat, "1" },
    { "unlit", "$depthblend", "depthblend", ValueKind::kBool, "0" },
    { "unlit", "$depthblendscale", "depthblendscale", ValueKind::kFloat, "50" },
    { "unlit", "$spriteorigin", "spriteorigin", ValueKind::kFloat3, "[0 0 0]" },
    { "unlit", "$spriteorientation", "spriteorientation", ValueKind::kEnum,
        "parallel_upright|facing_upright|vp_parallel|oriented|vp_parallel_oriented" },
    { "unlit", "$spriterendermode", "spriterendermode", ValueKind::kInt, "0" },
    { "unlit", "$nosrgb", "nosrgb", ValueKind::kBool, "0" },
    { "unlit", "$ignorevertexcolors", "ignorevertexcolors", ValueKind::kBool, "1" },
};

// The water family's keys (not legacy-derived: Water reads none of the
// common keys but $basetexture and $frame).
constexpr VmtKeyRow kWaterKeys[] = {
    // Portal 2's shipped parameters and defaults (its stdshader_dx9, the
    // CS:GO water.cpp's InitParams); water_family.h reads them.
    { "water", "$basetexture", "basetexture", ValueKind::kTexture, "" },
    { "water", "$frame", "frame", ValueKind::kInt, "0" },
    { "water", "$normalmap", "normalmap", ValueKind::kTexture, "" },
    { "water", "$bumpframe", "bumpframe", ValueKind::kInt, "0" },
    { "water", "$bumptransform", "bumptransform", ValueKind::kTransform, "" },
    { "water", "$flowmap", "flowmap", ValueKind::kTexture, "" },
    { "water", "$flowmapframe", "flowmapframe", ValueKind::kInt, "0" },
    { "water", "$flowmapscrollrate", "flowmapscrollrate", ValueKind::kFloat2, "[0 0]" },
    { "water", "$flow_noise_texture", "flow_noise_texture", ValueKind::kTexture, "" },
    { "water", "$flow_worlduvscale", "flow_worlduvscale", ValueKind::kFloat, "1" },
    { "water", "$flow_normaluvscale", "flow_normaluvscale", ValueKind::kFloat, "1" },
    { "water", "$flow_timeintervalinseconds", "flow_timeintervalinseconds", ValueKind::kFloat,
        "0.4" },
    { "water", "$flow_uvscrolldistance", "flow_uvscrolldistance", ValueKind::kFloat, "0.2" },
    { "water", "$flow_bumpstrength", "flow_bumpstrength", ValueKind::kFloat, "1" },
    { "water", "$flow_noise_scale", "flow_noise_scale", ValueKind::kFloat, "0.0002" },
    { "water", "$flow_debug", "flow_debug", ValueKind::kInt, "0" },
    // Not a parameter of Portal 2's shipped water shader (earlier ones had
    // it): some of its VMTs set it, and retail ignores it, as the family does.
    { "water", "$flow_timescale", "flow_timescale", ValueKind::kFloat, "1" },
    { "water", "$color_flow_uvscale", "color_flow_uvscale", ValueKind::kFloat, "1" },
    { "water", "$color_flow_timeintervalinseconds", "color_flow_timeintervalinseconds",
        ValueKind::kFloat, "0.4" },
    { "water", "$color_flow_uvscrolldistance", "color_flow_uvscrolldistance", ValueKind::kFloat,
        "0.2" },
    { "water", "$color_flow_lerpexp", "color_flow_lerpexp", ValueKind::kFloat, "1" },
    { "water", "$color_flow_displacebynormalstrength", "color_flow_displacebynormalstrength",
        ValueKind::kFloat, "0.0025" },
    { "water", "$reflecttexture", "reflecttexture", ValueKind::kTexture, "" },
    { "water", "$reflectamount", "reflectamount", ValueKind::kFloat, "0.8" },
    { "water", "$reflecttint", "reflecttint", ValueKind::kFloat3, "[1 1 1]" },
    { "water", "$refracttexture", "refracttexture", ValueKind::kTexture, "" },
    { "water", "$refractamount", "refractamount", ValueKind::kFloat, "0" },
    { "water", "$refracttint", "refracttint", ValueKind::kFloat3, "[1 1 1]" },
    { "water", "$envmap", "envmap", ValueKind::kTexture, "" },
    { "water", "$envmapframe", "envmapframe", ValueKind::kInt, "0" },
    { "water", "$forceenvmap", "forceenvmap", ValueKind::kBool, "0" },
    { "water", "$fogcolor", "fogcolor", ValueKind::kFloat3, "[1 0 0]" },
    { "water", "$fogstart", "fogstart", ValueKind::kFloat, "0" },
    { "water", "$fogend", "fogend", ValueKind::kFloat, "0" },
    { "water", "$lightmapwaterfog", "lightmapwaterfog", ValueKind::kBool, "0" },
    { "water", "$abovewater", "abovewater", ValueKind::kBool, "1" },
    { "water", "$forcefresnel", "forcefresnel", ValueKind::kFloat, "-1" },
    { "water", "$waterblendfactor", "waterblendfactor", ValueKind::kFloat, "1" },
    { "water", "$flashlighttint", "flashlighttint", ValueKind::kFloat, "1" },
    { "water", "$forceexpensive", "forceexpensive", ValueKind::kBool, "0" },
    { "water", "$forcecheap", "forcecheap", ValueKind::kBool, "0" },
    { "water", "$nofog", "nofog", ValueKind::kBool, "0" },
};

// Refract is a separate family: its tint, screen displacement and optional
// cube reflection do not have PBRMetalRough's material semantics.
constexpr VmtKeyRow kRefractKeys[] = {
    { "refract", "$model", "model", ValueKind::kBool, "0" },
    { "refract", "$translucent", "translucent", ValueKind::kBool, "0" },
    { "refract", "$basetexture", "basetexture", ValueKind::kTexture, "" },
    { "refract", "$normalmap", "normalmap", ValueKind::kTexture, "" },
    { "refract", "$refractamount", "refractamount", ValueKind::kFloat, "2" },
    { "refract", "$refracttint", "refracttint", ValueKind::kFloat3, "[1 1 1]" },
    { "refract", "$bluramount", "bluramount", ValueKind::kFloat, "0" },
    { "refract", "$fadeoutonsilhouette", "fadeoutonsilhouette", ValueKind::kBool, "0" },
    { "refract", "$envmap", "envmap", ValueKind::kTexture, "" },
    { "refract", "$envmaptint", "envmaptint", ValueKind::kFloat3, "[1 1 1]" },
    { "refract", "$envmapcontrast", "envmapcontrast", ValueKind::kFloat, "0" },
    { "refract", "$envmapsaturation", "envmapsaturation", ValueKind::kFloat3, "[1 1 1]" },
    { "refract", "$refracttinttexture", "refracttinttexture", ValueKind::kTexture, "" },
    { "refract", "$basetexturetransform", "basetexturetransform", ValueKind::kTransform, "" },
    // Portal 2's refract_ps2x LOCALREFRACT: the base texture refracted in
    // texture space (the native backend's RefractMaterialIsLocal).
    { "refract", "$localrefract", "localrefract", ValueKind::kBool, "0" },
    { "refract", "$localrefractdepth", "localrefractdepth", ValueKind::kFloat, "0.05" },
    // Portal 2's Refract declares $time for its animated variants; neither
    // point reads it, so a nonzero value stays unclaimed and is refused.
    { "refract", "$time", "time", ValueKind::kFloat, "0" },
};

constexpr VmtMetadataRow kMetadata[] = {
    { "$brightness", "UnlitGeneric, UnlitTwoTexture and Sprite neither declare nor read this key",
        "unlit" },
    { "$ignore_alpha_modulation",
        "material-system translucent classification; rendering still reads $alpha", "unlit" },
    { "$texoffset",
        "UnlitTwoTexture proxy input; the shader reads the resulting transforms and color",
        "unlit" },
    { "$texscale",
        "UnlitTwoTexture proxy input; the shader reads the resulting transforms and color",
        "unlit" },
    { "$tex2offset",
        "UnlitTwoTexture proxy input; the shader reads the resulting transforms and color",
        "unlit" },
    { "$tex2scale",
        "UnlitTwoTexture proxy input; the shader reads the resulting transforms and color",
        "unlit" },
    { "$a_b_halfwidth",
        "UnlitTwoTexture proxy input; the shader reads the resulting transforms and color",
        "unlit" },
    { "$a_b_noise",
        "UnlitTwoTexture proxy input; the shader reads the resulting transforms and color",
        "unlit" },
    { "$a_s_halfwidth",
        "UnlitTwoTexture proxy input; the shader reads the resulting transforms and color",
        "unlit" },
    { "$a_s_noise",
        "UnlitTwoTexture proxy input; the shader reads the resulting transforms and color",
        "unlit" },
    { "$a_t_halfwidth",
        "UnlitTwoTexture proxy input; the shader reads the resulting transforms and color",
        "unlit" },
    { "$a_threshold",
        "UnlitTwoTexture proxy input; the shader reads the resulting transforms and color",
        "unlit" },
    { "$alpha_bias",
        "UnlitTwoTexture proxy input; the shader reads the resulting transforms and color",
        "unlit" },
    { "$j_b_halfwidth",
        "UnlitTwoTexture proxy input; the shader reads the resulting transforms and color",
        "unlit" },
    { "$j_b_noise",
        "UnlitTwoTexture proxy input; the shader reads the resulting transforms and color",
        "unlit" },
    { "$j_s_halfwidth",
        "UnlitTwoTexture proxy input; the shader reads the resulting transforms and color",
        "unlit" },
    { "$j_s_noise",
        "UnlitTwoTexture proxy input; the shader reads the resulting transforms and color",
        "unlit" },
    { "$j_t_halfwidth",
        "UnlitTwoTexture proxy input; the shader reads the resulting transforms and color",
        "unlit" },
    { "$j_threshold",
        "UnlitTwoTexture proxy input; the shader reads the resulting transforms and color",
        "unlit" },
    { "$j_basescale",
        "UnlitTwoTexture proxy input; the shader reads the resulting transforms and color",
        "unlit" },
    { "$xo_b_halfwidth",
        "UnlitTwoTexture proxy input; the shader reads the resulting transforms and color",
        "unlit" },
    { "$xo_b_noise",
        "UnlitTwoTexture proxy input; the shader reads the resulting transforms and color",
        "unlit" },
    { "$xo_s_halfwidth",
        "UnlitTwoTexture proxy input; the shader reads the resulting transforms and color",
        "unlit" },
    { "$xo_s_noise",
        "UnlitTwoTexture proxy input; the shader reads the resulting transforms and color",
        "unlit" },
    { "$xo_t_halfwidth",
        "UnlitTwoTexture proxy input; the shader reads the resulting transforms and color",
        "unlit" },
    { "$xo_threshold",
        "UnlitTwoTexture proxy input; the shader reads the resulting transforms and color",
        "unlit" },

    { "$alphamasktexture", "WriteZ_DX9 has no texture parameter or alpha-test pixel shader",
        "depth" },
    { "$multipass",
        "VertexLitGeneric declares no multipass parameter; passes use explicit enable flags",
        "vertexlit" },
    { "$surfaceprop", "physics surface properties (the physics and sound systems)" },
    { "$surfaceprop2", "physics surface properties of a blend's second layer" },
    { "$decalscale", "decal projection size (the decal system)" },
    { "$reflectivity", "radiosity reflectivity (vrad)" },
    { "$fallbackmaterial", "the material a profile without the shader draws instead" },
    { "$envmapsphere", "a DirectX 6 envmap mode no dx9 shader reads" },
    { "$basemapalphaenvmapmask", "misspelled; VertexLitGeneric declares $basealphaenvmapmask" },
    { "$translucency", "misspelled; no shader declares it (the flag is $translucent)" },
    { "envmap", "no '$': a shader parameter is looked up as $envmap, so this key sets nothing" },
    { "phong", "no '$': a shader parameter is looked up as $phong, so this key sets nothing" },
    { "$vertextcolor", "misspelled; no shader declares it (the flag is $vertexcolor)" },
    { "$decalfadetime", "the engine's decal fade (r_decal.cpp), not a shader parameter" },
    { "$use_in_fillrate_mode",
        "selects the material for mat_fillrate's debug drawing only (shadersystem.cpp)" },
    { "$keywords", "tool and asset-browser metadata, not a shader parameter" },
    { "$worldimposter", "no shader in this tree declares it" },
    { "$alphaenvmapmask", "no shader in this tree declares it" },
    { "$diffuseexp", "no shader in this tree declares it" },
    { "$pseudotranslucent", "no shader in this tree declares it" },
    { "$phongdisablehalflambert", "not declared by VertexLitGeneric; no shipped shader reads it" },
    { "$bumpscale", "not declared by VertexLitGeneric; its normal map uses authored texels",
        "vertexlit" },
    { "$envmapconstrast", "misspelled; VertexLitGeneric declares $envmapcontrast", "vertexlit" },
    { "$envampsaturation", "misspelled; VertexLitGeneric declares $envmapsaturation", "vertexlit" },
    { "$dudvmap", "Refract_DX90 samples $normalmap, not $dudvmap", "refract" },
    { "$scale", "Refract_DX90 has no $scale shader parameter", "refract" },
    { "$normalmapalphaenvmapmask", "Refract_DX90 always uses normal alpha for reflection",
        "refract" },
    { "$envmaplightscale", "Refract_DX90 has no $envmaplightscale shader parameter", "refract" },
    { "$ignore_alpha_modulation",
        "Refract is already translucent; this material-system flag only affects classification",
        "refract" },
    { "$glowcolor", "VertexLitGeneric does not declare this UnlitGeneric control", "vertexlit" },
    { "$vertexfog", "no shipped stdshader declares this VMT key; view fog is frame state" },
    { "$modelmaterial", "the material a model draws when the world material is used on a model" },
    { "$bottommaterial", "water: the material seen from below" },
    { "$underwateroverlay", "water: the screen overlay drawn under water" },
    { "$nodecal", "decals are not applied (the decal system)" },
    { "$nofullbright", "mat_fullbright ignores the material" },
    { "$no_fullbright", "mat_fullbright ignores the material" },
    { "$nobasetexture", "procedural video supplies its texture after material initialization",
        "unlit" },
    { "$temp", "material-proxy scratch value, not a shader parameter", "unlit" },
    { "$surfaceprop_override", "physics surface properties" },
    { "$detailtype", "the detail-sprite set vbsp places on the surface" },
    { "$minsize", "particle size limits read by the particle system" },
    { "$maxsize", "particle size limits read by the particle system" },
    { "$maxdistance", "particle distance limit read by the particle system" },
    { "$farfadeinterval", "particle fade read by the particle system" },
    { "$reflectentities", "water: the client's reflection view draws entities" },
    { "$reflectskyboxonly", "water: the client's reflection view draws only the sky box" },
    { "$reflectonlymarkedentities",
        "water: the client's reflection view draws only entities marked to reflect" },
    { "$fogenable", "water: the engine's water fog volume (R_SetFogVolumeState)" },
    { "$waterdepth", "water: the depth vbsp writes into cube-map patches (no shader reads it)" },
    { "$shadersrgbread360",
        "Xbox 360 shader-side sRGB read; no shader in this tree declares it, and PC "
        "textures are read through their sRGB views" },
    { "$x360appchooser", "Xbox 360 application-chooser flag; no PC shader reads it" },
    { "$nolod", "texture level-of-detail selection (mat_picmip), not a shading parameter" },
    { "$decalfadeduration",
        "the engine's studio decal fade (l_studio.cpp), not a shader parameter" },
    { "$translucent", "DecalModulate_DX9 always blends dst * src color; the flag changes no state",
        "decal-modulate" },
    { "$fogfadestart", "DecalModulate_DX9 declares no shader parameters", "decal-modulate" },
    { "$fogfadeend", "DecalModulate_DX9 declares no shader parameters", "decal-modulate" },
    { "$ambientocclusion",
        "no shader in this tree declares it; ambient occlusion is the frame's screen-space term "
        "(render.pass.ao), not a material value" },
};

#include "legacy_shaders.inc"

constexpr std::string_view kPbrFamily = "pbr";

ValueKind PbrKind( pbr::ParameterKind kind )
{
	switch ( kind )
	{
	case pbr::ParameterKind::kTexture:
		return ValueKind::kTexture;
	case pbr::ParameterKind::kFloat:
		return ValueKind::kFloat;
	case pbr::ParameterKind::kBoolean:
		return ValueKind::kBool;
	case pbr::ParameterKind::kMaterialReference:
		return ValueKind::kMaterial;
	}
	return ValueKind::kFloat;
}

// Each legacy-derived family's common rows and then its own; the PBR rows come
// from RFC 0007's schema, their parameter names dropping the '$'.
std::vector<VmtKeyRow> BuildKeyRows()
{
	std::vector<VmtKeyRow> rows;
	for ( const std::string_view family : kLegacyDerivedFamilies )
	{
		for ( VmtKeyRow row : kCommonKeys )
		{
			row.family = family;
			rows.push_back( row );
		}
		for ( const VmtKeyRow &row : kLegacyDerivedKeys )
		{
			if ( row.family == family )
				rows.push_back( row );
		}
	}
	for ( const VmtKeyRow &row : kWaterKeys )
		rows.push_back( row );
	for ( const VmtKeyRow &row : kRefractKeys )
		rows.push_back( row );
	for ( const pbr::MaterialParameterSpec &spec : pbr::kMaterialParameters )
	{
		const std::string_view key = spec.name;
		rows.push_back( { kPbrFamily, key, key.substr( 1 ), PbrKind( spec.kind ),
		    spec.defaultValue ? std::string_view( spec.defaultValue ) : std::string_view() } );
	}
	return rows;
}

ParameterType TypeOf( ValueKind kind )
{
	switch ( kind )
	{
	case ValueKind::kTransform:
		return ParameterType::kTransform;
	case ValueKind::kTexture:
		return ParameterType::kTexture;
	case ValueKind::kFloat:
		return ParameterType::kFloat;
	case ValueKind::kFloat2:
		return ParameterType::kFloat2;
	case ValueKind::kFloat3:
		return ParameterType::kFloat3;
	case ValueKind::kFloat4:
		return ParameterType::kFloat4;
	case ValueKind::kInt:
	case ValueKind::kBool:
	case ValueKind::kEnum:
	case ValueKind::kMaterial:
		return ParameterType::kInt;
	}
	return ParameterType::kFloat;
}

// A row's default as numbers: the leading numbers of its text ("[1 1 1]",
// "0.5"); an enum's default is its first name, index 0.
void DefaultOf( const VmtKeyRow &row, float ( &out )[8] )
{
	if ( row.kind == ValueKind::kEnum || row.kind == ValueKind::kTexture )
		return;
	if ( row.kind == ValueKind::kTransform )
	{
		out[0] = out[5] = 1.0f; // the identity's rows 0 and 1
		return;
	}
	const std::string text( row.fallback );
	const char *cursor = text.c_str();
	int count = 0;
	while ( count < 4 && *cursor )
	{
		while ( *cursor == ' ' || *cursor == '[' || *cursor == ']' )
			++cursor;
		if ( !*cursor )
			break;
		char *end = nullptr;
		const float value = std::strtof( cursor, &end );
		if ( end == cursor )
			break;
		out[count++] = value;
		cursor = end;
	}
	for ( int i = count; count > 0 && i < 4; ++i )
		out[i] = out[count - 1];
}

} // namespace

const VmtMappingTable &BuiltinVmtMapping()
{
	static const std::vector<VmtKeyRow> s_Keys = BuildKeyRows();
	static const VmtMappingTable s_Table{
	    kShaders, s_Keys, kMetadata, kLegacyShaders, kLegacyReason };
	return s_Table;
}

std::vector<FamilyDesc> FamiliesFromMapping( const VmtMappingTable &mapping )
{
	std::vector<FamilyDesc> families;
	for ( const VmtShaderRow &shader : mapping.shaders )
	{
		bool known = shader.family == kLegacyFamily; // defined by the legacy frontend
		for ( const FamilyDesc &family : families )
			known = known || family.name == shader.family;
		if ( known )
			continue;
		FamilyDesc family;
		family.name = std::string( shader.family );
		family.bindGroups = device::kMaxBindGroups;
		for ( const VmtKeyRow &row : mapping.keys )
		{
			if ( row.family != shader.family || row.kind == ValueKind::kMaterial )
				continue;
			ParameterDesc parameter;
			parameter.name = std::string( row.parameter );
			parameter.type = TypeOf( row.kind );
			DefaultOf( row, parameter.defaults );
			family.parameters.push_back( std::move( parameter ) );
		}
		families.push_back( std::move( family ) );
	}
	return families;
}

} // namespace render::material
