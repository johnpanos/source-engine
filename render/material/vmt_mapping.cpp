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
    { "spritecard_dx8", "unlit",
        "particle cards: the unlit point with SpriteCard's terms (frame blend, "
        "$overbrightfactor, $addself, $mod2x, depth feathering); render.sprite-card.v1 "
        "builds the corners from the card records (spritecard_vsxx/splinecard_vsxx)" },
    { "sky_hdr_dx9", "unlit",
        "the 2D sky box's faces (Sky, render.pass.sky): an unlit textured quad whose base is an "
        "HDR encoding ($hdrcompressedtexture RGBS times 8, sky_hdr_compressed_rgbs_ps2x)" },
    { "sky_dx9", "unlit", "the sky box's faces without HDR: $basetexture times $color" },
    { "decalmodulate", "decal-modulate",
        "dimensionless surface factors with modulate-2x blending" },
    { "decalmodulate_dx9", "decal-modulate", "DecalModulate DirectX 9 implementation" },
    { "modulate", "modulate",
        "Modulate: the base times $color multiplied into the destination (2x with $mod2x); "
        "the decal-modulate point (unlit_family.h ClaimModulate)" },
    { "modulate_dx9", "modulate", "Modulate's DirectX 9 implementation" },
    { "cable", "cable", "CPU-expanded rope ribbons with normal UV0 and color UV1" },
    { "cable_dx9", "cable", "Cable's expanded ribbon implementation" },
    { "splinerope", "cable", "this client expands SplineRope ribbons before drawing" },
    { "pbrmetalrough", "pbr",
        "RFC 0007 metalness/roughness materials, drawn natively by world_pbr and model_pbr; "
        "the schema is render/pbr_material_schema.h" },
    { "pbr", "pbr",
        "P2:CE (Strata) PBR materials: the same metalness/roughness/AO model and MRAO channel "
        "order as PBRMetalRough; their own keys are interpreted by kPbrCompatKeys" },
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
    { "solidenergy", "energy",
        "Portal 2's fizzlers, light bridges and tractor beams: unlit flow fields and detail-layered "
        "beams with view-dependent opacity (solidenergy_ps20b); render/material/energy_family.h" },
    { "solidenergy_dx9", "energy", "SolidEnergy's DirectX 9 implementation" },
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
    "lightmapped", "vertexlit", "unlit", "depth", "portal-mask", "cable", "decal-modulate",
    "energy", "modulate" };

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
    // Modulate (modulate_dx9.cpp's parameters).
    { "modulate", "$writez", "writez", ValueKind::kBool, "0" },
    { "modulate", "$mod2x", "mod2x", ValueKind::kBool, "0" },
    { "modulate", "$cloakpassenabled", "cloakpassenabled", ValueKind::kBool, "0" },
    { "modulate", "$cloakfactor", "cloakfactor", ValueKind::kFloat, "0" },
    { "modulate", "$cloakcolortint", "cloakcolortint", ValueKind::kFloat3, "[1 1 1]" },
    { "modulate", "$refractamount", "refractamount", ValueKind::kFloat, "2" },
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

    // SolidEnergy (solidenergy_dx9.cpp's parameters; defaults are
    // InitParamsSolidEnergy's, solidenergy_dx9_helper.h). The opacity ranges
    // default to zero, which no material authors: a term is on when its
    // ranges are set or its $needs* flag is (the helper's IS_PARAM_DEFINED).
    { "energy", "$detail1", "detail1", ValueKind::kTexture, "" },
    { "energy", "$detail1scale", "detail1scale", ValueKind::kFloat, "1" },
    { "energy", "$detail1frame", "detail1frame", ValueKind::kInt, "0" },
    { "energy", "$detail1blendmode", "detail1blendmode", ValueKind::kInt, "0" },
    { "energy", "$detail1texturetransform", "detail1texturetransform", ValueKind::kTransform, "" },
    { "energy", "$detail2", "detail2", ValueKind::kTexture, "" },
    { "energy", "$detail2scale", "detail2scale", ValueKind::kFloat, "1" },
    { "energy", "$detail2frame", "detail2frame", ValueKind::kInt, "0" },
    { "energy", "$detail2blendmode", "detail2blendmode", ValueKind::kInt, "0" },
    { "energy", "$detail2texturetransform", "detail2texturetransform", ValueKind::kTransform, "" },
    { "energy", "$tangenttopacityranges", "tangenttopacityranges", ValueKind::kFloat4,
        "[0 0 0 0]" },
    { "energy", "$tangentsopacityranges", "tangentsopacityranges", ValueKind::kFloat4,
        "[0 0 0 0]" },
    { "energy", "$fresnelopacityranges", "fresnelopacityranges", ValueKind::kFloat4, "[0 0 0 0]" },
    { "energy", "$needstangentt", "needstangentt", ValueKind::kBool, "0" },
    { "energy", "$needstangents", "needstangents", ValueKind::kBool, "0" },
    { "energy", "$needsnormals", "needsnormals", ValueKind::kBool, "0" },
    { "energy", "$flowmap", "flowmap", ValueKind::kTexture, "" },
    { "energy", "$flowmapframe", "flowmapframe", ValueKind::kInt, "0" },
    { "energy", "$flow_noise_texture", "flow_noise_texture", ValueKind::kTexture, "" },
    { "energy", "$flowbounds", "flowbounds", ValueKind::kTexture, "" },
    { "energy", "$flow_worlduvscale", "flow_worlduvscale", ValueKind::kFloat, "1" },
    { "energy", "$flow_normaluvscale", "flow_normaluvscale", ValueKind::kFloat, "1" },
    { "energy", "$flow_timeintervalinseconds", "flow_timeintervalinseconds", ValueKind::kFloat,
        "0.4" },
    { "energy", "$flow_uvscrolldistance", "flow_uvscrolldistance", ValueKind::kFloat, "0.2" },
    { "energy", "$flow_noise_scale", "flow_noise_scale", ValueKind::kFloat, "0.0002" },
    { "energy", "$flow_lerpexp", "flow_lerpexp", ValueKind::kFloat, "0" },
    { "energy", "$powerup", "powerup", ValueKind::kFloat, "1" },
    { "energy", "$flow_color_intensity", "flow_color_intensity", ValueKind::kFloat, "1" },
    { "energy", "$flow_color", "flow_color", ValueKind::kFloat3, "[0.1 0.2 0.4]" },
    { "energy", "$flow_vortex_color", "flow_vortex_color", ValueKind::kFloat3, "[1.2 0.4 0]" },
    { "energy", "$flow_vortex_size", "flow_vortex_size", ValueKind::kFloat, "30" },
    { "energy", "$flow_vortex1", "flow_vortex1", ValueKind::kBool, "0" },
    { "energy", "$flow_vortex_pos1", "flow_vortex_pos1", ValueKind::kFloat3, "[0 0 0]" },
    { "energy", "$flow_vortex2", "flow_vortex2", ValueKind::kBool, "0" },
    { "energy", "$flow_vortex_pos2", "flow_vortex_pos2", ValueKind::kFloat3, "[0 0 0]" },
    { "energy", "$flow_cheap", "flow_cheap", ValueKind::kBool, "0" },
    { "energy", "$modelformat", "modelformat", ValueKind::kBool, "0" },
    { "energy", "$outputintensity", "outputintensity", ValueKind::kFloat, "1" },
    // Declared but read by no SolidEnergy combo on PC (DEPTHBLEND is "0..0"
    // there; the blend factors, $time and the flow map's scroll rate reach
    // no constant): claimed as inert.
    { "energy", "$detail1blendfactor", "detail1blendfactor", ValueKind::kFloat, "1" },
    { "energy", "$detail2blendfactor", "detail2blendfactor", ValueKind::kFloat, "1" },
    { "energy", "$depthblend", "depthblend", ValueKind::kBool, "0" },
    { "energy", "$depthblendscale", "depthblendscale", ValueKind::kFloat, "50" },
    { "energy", "$time", "time", ValueKind::kFloat, "0" },
    { "energy", "$flowmapscrollrate", "flowmapscrollrate", ValueKind::kFloat2, "[0 0]" },
    { "cable", "$bumpmap", "bumpmap", ValueKind::kTexture, "" },
    { "cable", "$minlight", "minlight", ValueKind::kFloat, "0.1" },
    { "cable", "$maxlight", "maxlight", ValueKind::kFloat, "0.3" },
    { "unlit", "$color2", "color2", ValueKind::kFloat3, "[1 1 1]" },
    { "unlit", "$gammacolorread", "gammacolorread", ValueKind::kInt, "0" },
    { "unlit", "$linearwrite", "linearwrite", ValueKind::kInt, "0" },
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
    // SpriteCard (spritecard.cpp's parameters and defaults; RFC 0016 K8 particles).
    { "unlit", "$orientation", "orientation", ValueKind::kInt, "0" },
    { "unlit", "$splinetype", "splinetype", ValueKind::kInt, "0" },
    { "unlit", "$overbrightfactor", "overbrightfactor", ValueKind::kFloat, "1" },
    { "unlit", "$addself", "addself", ValueKind::kFloat, "0" },
    { "unlit", "$addbasetexture2", "addbasetexture2", ValueKind::kFloat, "0" },
    { "unlit", "$addoverblend", "addoverblend", ValueKind::kBool, "0" },
    { "unlit", "$mod2x", "mod2x", ValueKind::kBool, "0" },
    { "unlit", "$blendframes", "blendframes", ValueKind::kBool, "1" },
    { "unlit", "$dualsequence", "dualsequence", ValueKind::kInt, "0" },
    { "unlit", "$sequence_blend_mode", "sequence_blend_mode", ValueKind::kInt, "0" },
    { "unlit", "$maxlumframeblend1", "maxlumframeblend1", ValueKind::kInt, "0" },
    { "unlit", "$maxlumframeblend2", "maxlumframeblend2", ValueKind::kInt, "0" },
    { "unlit", "$ramptexture", "ramptexture", ValueKind::kTexture, "" },
    { "unlit", "$zoomanimateseq2", "zoomanimateseq2", ValueKind::kFloat, "1" },
    { "unlit", "$extractgreenalpha", "extractgreenalpha", ValueKind::kInt, "0" },
    { "unlit", "$useinstancing", "useinstancing", ValueKind::kBool, "1" },
    { "unlit", "$minsize", "minsize", ValueKind::kFloat, "0" },
    { "unlit", "$maxsize", "maxsize", ValueKind::kFloat, "20" },
    { "unlit", "$startfadesize", "startfadesize", ValueKind::kFloat, "10" },
    { "unlit", "$endfadesize", "endfadesize", ValueKind::kFloat, "20" },
    { "unlit", "$maxdistance", "maxdistance", ValueKind::kFloat, "100000" },
    { "unlit", "$farfadeinterval", "farfadeinterval", ValueKind::kFloat, "400" },
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
    // Two-sided panes (shattered glass): the resolver draws both faces.
    { "refract", "$nocull", "nocull", ValueKind::kBool, "0" },
    // refract_vs20's normal map (and tint texture) coordinates: the vertex
    // UV through $bumptransform (a TextureScroll proxy animates it), at the
    // normal map's $bumpframe (the mesh handoff binds that frame's handle).
    { "refract", "$bumptransform", "bumptransform", ValueKind::kTransform, "" },
    { "refract", "$bumpframe", "bumpframe", ValueKind::kInt, "0" },
    // Warp particles (RFC 0016 K8 particles): the vertex color tints the
    // refracted scene and the vertex alpha scales the warp and its tint, so a
    // fading particle's refraction fades with it. $vertexcolormodulate is
    // Portal 2's switch for both (refract_dx9_helper.cpp's COLORMODULATE).
    { "refract", "$vertexcolor", "vertexcolor", ValueKind::kBool, "0" },
    { "refract", "$vertexalpha", "vertexalpha", ValueKind::kBool, "0" },
    { "refract", "$vertexcolormodulate", "vertexcolormodulate", ValueKind::kBool, "0" },
    { "refract", "$nofog", "nofog", ValueKind::kBool, "0" },
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
    { "$envmapmode", "a DirectX 6 material flag; no DirectX 9 shader reads it" },
    { "$envmapcameraspace", "a material flag only the DirectX 8 passes read" },
    { "$multipass", "a material flag only the DirectX 6 and 7 shaders read", "lightmapped" },
    { "$halflambert", "a lighting flag; the unlit point has no lighting to shape", "unlit" },
    { "$parallaxmapscale", "no LightmappedGeneric combo reads it in this engine", "lightmapped" },
    { "$fogcolor", "a Water parameter; LightmappedGeneric declares none", "lightmapped" },
    { "$leakcolor", "LightmappedGeneric declares no such parameter", "lightmapped" },
    { "$bumpframe", "WriteZ declares no such parameter", "depth" },
    { "$clampt", "VertexLitGeneric declares no such parameter", "vertexlit" },
    { "$basetexture3", "VertexLitGeneric declares no such parameter", "vertexlit" },
    { "$abovewater", "a Water parameter; UnlitGeneric declares none", "unlit" },
    { "$parallaxmap", "no LightmappedGeneric combo reads it in this engine (only a SKIP comment names PARALLAXMAP)", "lightmapped" },
    { "$abovewater", "a Water parameter; LightmappedGeneric declares none", "lightmapped" },
    { "$leakamount", "LightmappedGeneric declares no such parameter", "lightmapped" },
    { "$nomip", "a texture compile setting (vtex), not a shader parameter", "lightmapped" },
    { "$normalmapalpha", "LightmappedGeneric declares no such parameter", "lightmapped" },
    { "$texture2", "Modulate declares no such parameter", "modulate" },
    { "$nocompress", "a texture compile setting (vtex), not a shader parameter", "unlit" },
    { "$2basetexture", "malformed; no shader declares it", "unlit" },
    { "$alphatested", "misspelled; UnlitGeneric declares no such parameter (the flag is $alphatest)", "unlit" },
    { "$basettexture", "misspelled; no shader declares it", "unlit" },
    { "$bumpmap", "UnlitGeneric declares no normal map", "unlit" },
    { "$envcontrast", "misspelled; the parameter is $envmapcontrast", "unlit" },
    { "$modulate", "UnlitGeneric declares no such parameter", "unlit" },
    { "$spritesize", "no shader in this engine declares it", "unlit" },
    { "$tint", "UnlitGeneric declares no such parameter", "unlit" },
    { "$transluscent", "misspelled; the flag is $translucent", "unlit" },
    { "$ambientocclusiontexture", "VertexLitGeneric declares no such parameter in this engine", "vertexlit" },
    { "$basetexture2", "a WorldVertexTransition parameter; VertexLitGeneric declares none", "vertexlit" },
    { "$clamps", "VertexLitGeneric declares no such parameter", "vertexlit" },
    { "$envcontrast", "misspelled; the parameter is $envmapcontrast", "vertexlit" },
    { "$envmapfrensel", "misspelled; VertexLitGeneric declares no such parameter", "vertexlit" },
    { "$bluramount", "WriteZ declares no such parameter", "depth" },
    { "$groupbame", "a misspelling no shader declares", "unlit" },
    { "$vertexcolors", "misspelled; UnlitGeneric declares no such parameter (the flag is $vertexcolor)", "unlit" },
    { "$nomip", "a texture compile setting (vtex), not a shader parameter", "unlit" },
    { "$nooverbright", "a DirectX 6-era key; UnlitGeneric declares no such parameter", "unlit" },
    { "$nopaint", "UnlitGeneric declares no such parameter (Portal 2's paint reads its own surface flags)", "unlit" },
    { "$decalsecondpass", "UnlitGeneric declares no such parameter", "unlit" },
    { "$writez", "UnlitGeneric declares no such parameter (WriteZ and Modulate do)", "unlit" },
    { "$spritescale", "Sprite declares no such parameter", "unlit" },
    { "$bumpscale", "a DirectX 8-era key; LightmappedGeneric declares no such parameter", "lightmapped" },
    { "$comparez", "LightmappedGeneric declares no such parameter", "lightmapped" },
    { "$normalmapenvmapmask", "misspelled; LightmappedGeneric declares $normalmapalphaenvmapmask", "lightmapped" },
    { "$phong", "LightmappedGeneric declares no Phong parameter", "lightmapped" },
    { "$phongboost", "LightmappedGeneric declares no Phong parameter", "lightmapped" },
    { "$writez", "LightmappedGeneric declares no such parameter", "lightmapped" },
    { "$phongexponent", "LightmappedGeneric declares no Phong parameter", "lightmapped" },
    { "$bump_force_on", "WorldVertexTransition declares no such parameter in this engine",
        "lightmapped" },
    { "$shadowdepth", "SplineRope declares no such parameter in this engine", "cable" },
    { "$alphatesttocoverage", "VertexLitGeneric declares no such parameter in this engine ($allowalphatocoverage is the flag)", "vertexlit" },
    { "$fresnelreflection", "a LightmappedGeneric parameter; VertexLitGeneric declares none", "vertexlit" },
    { "$rimboost", "misspelled; VertexLitGeneric declares $rimlightboost", "vertexlit" },
    { "$modblend", "Modulate declares no such parameter: proxy or VGUI screen input", "modulate" },
    { "$texoffset", "Modulate declares no such parameter: proxy or VGUI screen input", "modulate" },
    { "$texscale", "Modulate declares no such parameter: proxy or VGUI screen input", "modulate" },
    { "$intensitynoise", "SolidEnergy declares no such parameter: proxy scratch (TextureTransform, "
        "TextureScroll, Sine, Multiply inputs and results) or unused", "energy" },
    { "$totalintensity", "SolidEnergy declares no such parameter: proxy scratch (TextureTransform, "
        "TextureScroll, Sine, Multiply inputs and results) or unused", "energy" },
    { "$basetexturescale", "SolidEnergy declares no such parameter: proxy scratch (TextureTransform, "
        "TextureScroll, Sine, Multiply inputs and results) or unused", "energy" },
    { "$basetexturecentroid", "SolidEnergy declares no such parameter: proxy scratch (TextureTransform, "
        "TextureScroll, Sine, Multiply inputs and results) or unused", "energy" },
    { "$detail1offset", "SolidEnergy declares no such parameter: proxy scratch (TextureTransform, "
        "TextureScroll, Sine, Multiply inputs and results) or unused", "energy" },
    { "$detail1rot", "SolidEnergy declares no such parameter: proxy scratch (TextureTransform, "
        "TextureScroll, Sine, Multiply inputs and results) or unused", "energy" },
    { "$detail2offset", "SolidEnergy declares no such parameter: proxy scratch (TextureTransform, "
        "TextureScroll, Sine, Multiply inputs and results) or unused", "energy" },
    { "$detail2rot", "SolidEnergy declares no such parameter: proxy scratch (TextureTransform, "
        "TextureScroll, Sine, Multiply inputs and results) or unused", "energy" },
    { "$detailscroll1", "SolidEnergy declares no such parameter: proxy scratch (TextureTransform, "
        "TextureScroll, Sine, Multiply inputs and results) or unused", "energy" },
    { "$detailscroll2", "SolidEnergy declares no such parameter: proxy scratch (TextureTransform, "
        "TextureScroll, Sine, Multiply inputs and results) or unused", "energy" },
    { "$detail2componentscale", "SolidEnergy declares no such parameter: proxy scratch (TextureTransform, "
        "TextureScroll, Sine, Multiply inputs and results) or unused", "energy" },
    { "$basescroll", "SolidEnergy declares no such parameter: proxy scratch (TextureTransform, "
        "TextureScroll, Sine, Multiply inputs and results) or unused", "energy" },
    { "$basescale", "SolidEnergy declares no such parameter: proxy scratch (TextureTransform, "
        "TextureScroll, Sine, Multiply inputs and results) or unused", "energy" },
    { "$scrollrate", "SolidEnergy declares no such parameter: proxy scratch (TextureTransform, "
        "TextureScroll, Sine, Multiply inputs and results) or unused", "energy" },
    { "$detail1scrollfactor", "SolidEnergy declares no such parameter: proxy scratch (TextureTransform, "
        "TextureScroll, Sine, Multiply inputs and results) or unused", "energy" },
    { "$detail2scrollfactor", "SolidEnergy declares no such parameter: proxy scratch (TextureTransform, "
        "TextureScroll, Sine, Multiply inputs and results) or unused", "energy" },
    { "$detail1scrollrate", "SolidEnergy declares no such parameter: proxy scratch (TextureTransform, "
        "TextureScroll, Sine, Multiply inputs and results) or unused", "energy" },
    { "$detail2scrollrate", "SolidEnergy declares no such parameter: proxy scratch (TextureTransform, "
        "TextureScroll, Sine, Multiply inputs and results) or unused", "energy" },
    { "$basetexturescrollrate", "SolidEnergy declares no such parameter: proxy scratch (TextureTransform, "
        "TextureScroll, Sine, Multiply inputs and results) or unused", "energy" },
    { "$basetextureoffset", "SolidEnergy declares no such parameter: proxy scratch (TextureTransform, "
        "TextureScroll, Sine, Multiply inputs and results) or unused", "energy" },
    { "$basetextureoffsetfreeze", "SolidEnergy declares no such parameter: proxy scratch (TextureTransform, "
        "TextureScroll, Sine, Multiply inputs and results) or unused", "energy" },
    { "$basetexturetranslate", "SolidEnergy declares no such parameter: proxy scratch (TextureTransform, "
        "TextureScroll, Sine, Multiply inputs and results) or unused", "energy" },
    { "$neg", "SolidEnergy declares no such parameter: proxy scratch (TextureTransform, "
        "TextureScroll, Sine, Multiply inputs and results) or unused", "energy" },
    { "$none", "SolidEnergy declares no such parameter: proxy scratch (TextureTransform, "
        "TextureScroll, Sine, Multiply inputs and results) or unused", "energy" },
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
    { "$forcerefract",
        "the client refreshes the refraction texture before the draw; the core "
        "captures the view's scene color for every Refract draw",
        "refract" },
    { "$dudvframe", "the frame of $dudvmap, which Refract_DX90 does not sample", "refract" },
    { "$scale", "Refract_DX90 has no $scale shader parameter", "refract" },
    { "$normalmapalphaenvmapmask", "Refract_DX90 always uses normal alpha for reflection",
        "refract" },
    { "$envmaplightscale", "Refract_DX90 has no $envmaplightscale shader parameter", "refract" },
    { "$additive",
        "Refract_DX90 sets no blend from it (refract_dx9_helper.cpp blends only $masked); the "
        "flag changes the material system's sort order alone",
        "refract" },
    { "$ignore_alpha_modulation",
        "Refract is already translucent; this material-system flag only affects classification",
        "refract" },
    { "$glowcolor", "VertexLitGeneric does not declare this UnlitGeneric control", "vertexlit" },
    { "$vertexfog", "no shipped stdshader declares this VMT key; view fog is frame state" },
    { "$modelmaterial", "the material a model draws when the world material is used on a model" },
    { "$bottommaterial", "water: the material seen from below" },
    { "$underwateroverlay", "water: the screen overlay drawn under water" },
    { "$nodecal", "decals are not applied (the decal system)" },
    { "$model",
        "P2:CE PBR's model/brush switch; the core takes the geometry kind from the draw",
        "pbr" },
    { "$nofullbright", "mat_fullbright ignores the material" },
    { "$no_fullbright", "mat_fullbright ignores the material" },
    { "$nobasetexture", "procedural video supplies its texture after material initialization",
        "unlit" },
    { "$temp", "material-proxy scratch value, not a shader parameter", "unlit" },
    { "$surfaceprop_override", "physics surface properties" },
    { "$detailtype", "the detail-sprite set vbsp places on the surface" },
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

// P2:CE (Strata) PBR keys beside RFC 0007's schema, read by the pbr family
// (pbr_family.cpp). $mraoscale multiplies the MRAO texture per channel;
// $envmaptint tints the probe reflection (Source gamma, as VertexLitGeneric's
// native probe); $envmaplightscale is Portal 2's light-scaled reflection;
// $basetexturetransform moves every texture's coordinate; $srgbtint tints the
// base color; $nocull draws both faces. Parallax and
// $blendtintbymraoalpha are inert unless enabled (parallax) or given a
// $color2, which the family refuses by name.
constexpr VmtKeyRow kPbrCompatKeys[] = {
    { kPbrFamily, "$mraoscale", "mraoscale", ValueKind::kFloat3, "[1 1 1]" },
    { kPbrFamily, "$envmaptint", "envmaptint", ValueKind::kFloat3, "[1 1 1]" },
    { kPbrFamily, "$envmaplightscale", "envmaplightscale", ValueKind::kFloat, "0" },
    { kPbrFamily, "$basetexturetransform", "basetexturetransform", ValueKind::kTransform, "" },
    { kPbrFamily, "$parallax", "parallax", ValueKind::kBool, "0" },
    { kPbrFamily, "$parallaxdepth", "parallaxdepth", ValueKind::kFloat, "0.025" },
    { kPbrFamily, "$parallaxcenter", "parallaxcenter", ValueKind::kFloat, "0.5" },
    { kPbrFamily, "$parallaxdither", "parallaxdither", ValueKind::kBool, "0" },
    { kPbrFamily, "$parallaxscale", "parallaxscale", ValueKind::kFloat, "1" },
    { kPbrFamily, "$blendtintbymraoalpha", "blendtintbymraoalpha", ValueKind::kBool, "0" },
    // P2:CE's default PBR includes (materials/dev/l2_default_pbr.vmt) set the
    // second blend layer's settings; they apply only to $basetexture2, which
    // the family refuses, so alone they are inert.
    { kPbrFamily, "$basetexturetransform2", "basetexturetransform2", ValueKind::kTransform, "" },
    { kPbrFamily, "$mraoscale2", "mraoscale2", ValueKind::kFloat3, "[1 1 1]" },
    { kPbrFamily, "$envmaptint2", "envmaptint2", ValueKind::kFloat3, "[1 1 1]" },
    { kPbrFamily, "$nocull", "nocull", ValueKind::kBool, "0" },
    // A base-color tint authored in sRGB (Source gamma here, as $color).
    { kPbrFamily, "$srgbtint", "srgbtint", ValueKind::kFloat3, "[1 1 1]" },
};

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
	for ( const VmtKeyRow &row : kPbrCompatKeys )
		rows.push_back( row );
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
