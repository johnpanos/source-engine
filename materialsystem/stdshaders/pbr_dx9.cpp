//==================================================================================================
//
// Physically Based Rendering shader for brushes and models
//
//==================================================================================================

// Includes for all shaders
#include "BaseVSShader.h"

// Includes for PS30

// Includes for PS20b

// Defining samplers
const Sampler_t SAMPLER_BASETEXTURE = SHADER_SAMPLER0;
const Sampler_t SAMPLER_NORMAL = SHADER_SAMPLER1;
const Sampler_t SAMPLER_ENVMAP = SHADER_SAMPLER2;
const Sampler_t SAMPLER_SHADOWDEPTH = SHADER_SAMPLER4;
const Sampler_t SAMPLER_RANDOMROTATION = SHADER_SAMPLER5;
const Sampler_t SAMPLER_FLASHLIGHT = SHADER_SAMPLER6;
const Sampler_t SAMPLER_LIGHTMAP = SHADER_SAMPLER7;
const Sampler_t SAMPLER_MRAO = SHADER_SAMPLER10;
const Sampler_t SAMPLER_EMISSIVE = SHADER_SAMPLER11;
const Sampler_t SAMPLER_SPECULAR = SHADER_SAMPLER12;

// Convars
static ConVar mat_fullbright("mat_fullbright", "0", FCVAR_CHEAT);
static ConVar mat_specular("mat_specular", "1", FCVAR_CHEAT);
static ConVar mat_pbr_force_20b("mat_pbr_force_20b", "0", FCVAR_CHEAT);
static ConVar mat_pbr_parallaxmap("mat_pbr_parallaxmap", "1");

// Variables for this shader
struct PBR_Vars_t
{
    PBR_Vars_t()
    {
        memset(this, 0xFF, sizeof(*this));
    }

    int baseTexture;
    int baseColor;
    int normalTexture;
    int bumpMap;
    int envMap;
    int baseTextureFrame;
    int baseTextureTransform;
    int useParallax;
    int parallaxDepth;
    int parallaxCenter;
    int alphaTestReference;
    int flashlightTexture;
    int flashlightTextureFrame;
    int emissionTexture;
    int mraoTexture;
    int useEnvAmbient;
    int specularTexture;
};

// Beginning the shader
BEGIN_VS_SHADER(PBR, "PBR shader")

    // Setting up vmt parameters
    BEGIN_SHADER_PARAMS;
        SHADER_PARAM(ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "0", "");
        SHADER_PARAM(ENVMAP, SHADER_PARAM_TYPE_ENVMAP, "", "Set the cubemap for this material.");
        SHADER_PARAM(MRAOTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Texture with metalness in R, roughness in G, ambient occlusion in B.");
        SHADER_PARAM(EMISSIONTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Emission texture");
        SHADER_PARAM(EMISSIONSCALE, SHADER_PARAM_TYPE_FLOAT, "1", "Linear emission multiplier (read by the render core; material proxies such as LightedMouth drive it)");
        SHADER_PARAM(NORMALTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Normal texture (deprecated, use $bumpmap)");
        SHADER_PARAM(BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "", "Normal texture");
        SHADER_PARAM(USEENVAMBIENT, SHADER_PARAM_TYPE_BOOL, "0", "Use the cubemaps to compute ambient light.");
        SHADER_PARAM(SPECULARTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Specular F0 RGB map");
        SHADER_PARAM(PARALLAX, SHADER_PARAM_TYPE_BOOL, "0", "Use Parallax Occlusion Mapping.");
        SHADER_PARAM(PARALLAXDEPTH, SHADER_PARAM_TYPE_FLOAT, "0.0030", "Depth of the Parallax Map");
        SHADER_PARAM(PARALLAXCENTER, SHADER_PARAM_TYPE_FLOAT, "0.5", "Center depth of the Parallax Map");
    END_SHADER_PARAMS;

    // Setting up variables for this shader
    void SetupVars(PBR_Vars_t &info)
    {
        info.baseTexture = BASETEXTURE;
        info.baseColor = COLOR;
        info.normalTexture = NORMALTEXTURE;
        info.bumpMap = BUMPMAP;
        info.baseTextureFrame = FRAME;
        info.baseTextureTransform = BASETEXTURETRANSFORM;
        info.alphaTestReference = ALPHATESTREFERENCE;
        info.flashlightTexture = FLASHLIGHTTEXTURE;
        info.flashlightTextureFrame = FLASHLIGHTTEXTUREFRAME;
        info.envMap = ENVMAP;
        info.emissionTexture = EMISSIONTEXTURE;
        info.mraoTexture = MRAOTEXTURE;
        info.useEnvAmbient = USEENVAMBIENT;
        info.specularTexture = SPECULARTEXTURE;
        info.useParallax = PARALLAX;
        info.parallaxDepth = PARALLAXDEPTH;
        info.parallaxCenter = PARALLAXCENTER;
    };

    // Initializing parameters
    SHADER_INIT_PARAMS()
    {
        // Fallback for changed parameter
        if (params[NORMALTEXTURE]->IsDefined())
            params[BUMPMAP]->SetStringValue(params[NORMALTEXTURE]->GetStringValue());

        // Dynamic lights need a bumpmap
        if (!params[BUMPMAP]->IsDefined())
            params[BUMPMAP]->SetStringValue("dev/flat_normal");

        // Set a good default mrao texture
        if (!params[MRAOTEXTURE]->IsDefined())
            params[MRAOTEXTURE]->SetStringValue("dev/pbr_mraotexture");

        // PBR relies heavily on envmaps
        if (!params[ENVMAP]->IsDefined())
            params[ENVMAP]->SetStringValue("env_cubemap");

        // Check if the hardware supports flashlight border color
        if (g_pHardwareConfig->SupportsBorderColor())
        {
            params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight_border");
        }
        else
        {
            params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight001");
        }
    };

    // Define shader fallback
    SHADER_FALLBACK
    {
        return 0;
    };

    SHADER_INIT
    {
        PBR_Vars_t info;
        SetupVars(info);

        Assert(info.flashlightTexture >= 0);
        LoadTexture(info.flashlightTexture, TEXTUREFLAGS_SRGB);

        Assert(info.bumpMap >= 0);
        LoadBumpMap(info.bumpMap);

        Assert(info.envMap >= 0);
        int envMapFlags = g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE ? TEXTUREFLAGS_SRGB : 0;
        envMapFlags |= TEXTUREFLAGS_ALL_MIPS;
        LoadCubeMap(info.envMap, envMapFlags);

        if (info.emissionTexture >= 0 && params[EMISSIONTEXTURE]->IsDefined())
            LoadTexture(info.emissionTexture, TEXTUREFLAGS_SRGB);

        Assert(info.mraoTexture >= 0);
        LoadTexture(info.mraoTexture, 0);

        if (params[info.baseTexture]->IsDefined())
        {
            LoadTexture(info.baseTexture, TEXTUREFLAGS_SRGB);
        }

        if (params[info.specularTexture]->IsDefined())
        {
            LoadTexture(info.specularTexture, TEXTUREFLAGS_SRGB);
        }

        if (IS_FLAG_SET(MATERIAL_VAR_MODEL)) // Set material var2 flags specific to models
        {
            SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);             // Required for skinning
            SET_FLAGS2(MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL);         // Required for dynamic lighting
            SET_FLAGS2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);             // Required for dynamic lighting
            SET_FLAGS2(MATERIAL_VAR2_LIGHTING_VERTEX_LIT);              // Required for dynamic lighting
            SET_FLAGS2(MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS);   // Required for ambient cube
            SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_FLASHLIGHT);              // Required for flashlight
            SET_FLAGS2(MATERIAL_VAR2_USE_FLASHLIGHT);                   // Required for flashlight
        }
        else // Set material var2 flags specific to brushes
        {
            SET_FLAGS2(MATERIAL_VAR2_LIGHTING_LIGHTMAP);                // Required for lightmaps
            SET_FLAGS2(MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP);         // Required for lightmaps
            SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_FLASHLIGHT);              // Required for flashlight
            SET_FLAGS2(MATERIAL_VAR2_USE_FLASHLIGHT);                   // Required for flashlight
        }
    };

    // Drawing the shader
    SHADER_DRAW
    {
        PBR_Vars_t info;
        SetupVars(info);

        // Setting up booleans
        bool bHasBaseTexture = (info.baseTexture != -1) && params[info.baseTexture]->IsTexture();
        bool bHasNormalTexture = (info.bumpMap != -1) && params[info.bumpMap]->IsTexture();
        bool bHasMraoTexture = (info.mraoTexture != -1) && params[info.mraoTexture]->IsTexture();
        bool bHasEmissionTexture = (info.emissionTexture != -1) && params[info.emissionTexture]->IsTexture();
        bool bHasEnvTexture = (info.envMap != -1) && params[info.envMap]->IsTexture();
        bool bIsAlphaTested = IS_FLAG_SET(MATERIAL_VAR_ALPHATEST) != 0;
        bool bHasFlashlight = UsingFlashlight(params);
        bool bHasColor = (info.baseColor != -1) && params[info.baseColor]->IsDefined();
        bool bLightMapped = !IS_FLAG_SET(MATERIAL_VAR_MODEL);
        bool bUseEnvAmbient = (info.useEnvAmbient != -1) && (params[info.useEnvAmbient]->GetIntValue() == 1);
        bool bHasSpecularTexture = (info.specularTexture != -1) && params[info.specularTexture]->IsTexture();

        // Determining whether we're dealing with a fully opaque material
        BlendType_t nBlendType = EvaluateBlendRequirements(info.baseTexture, true);
        bool bFullyOpaque = (nBlendType != BT_BLENDADD) && (nBlendType != BT_BLEND) && !bIsAlphaTested;

        if (IsSnapshotting())
        {
            // If alphatest is on, enable it
            pShaderShadow->EnableAlphaTest(bIsAlphaTested);

            if (info.alphaTestReference != -1 && params[info.alphaTestReference]->GetFloatValue() > 0.0f)
            {
                pShaderShadow->AlphaFunc(SHADER_ALPHAFUNC_GEQUAL, params[info.alphaTestReference]->GetFloatValue());
            }

            if (bHasFlashlight )
            {
                pShaderShadow->EnableBlending(true);
                pShaderShadow->BlendFunc(SHADER_BLEND_ONE, SHADER_BLEND_ONE); // Additive blending
            }
            else
            {
                SetDefaultBlendingShadowState(info.baseTexture, true);
            }

            int nShadowFilterMode = bHasFlashlight ? g_pHardwareConfig->GetShadowFilterMode() : 0;

            // Setting up samplers
            pShaderShadow->EnableTexture(SAMPLER_BASETEXTURE, true);    // Basecolor texture
            pShaderShadow->EnableSRGBRead(SAMPLER_BASETEXTURE, true);   // Basecolor is sRGB
            pShaderShadow->EnableTexture(SAMPLER_EMISSIVE, true);       // Emission texture
            pShaderShadow->EnableSRGBRead(SAMPLER_EMISSIVE, true);      // Emission is sRGB
            pShaderShadow->EnableTexture(SAMPLER_LIGHTMAP, true);       // Lightmap texture
            pShaderShadow->EnableSRGBRead(SAMPLER_LIGHTMAP, false);     // Lightmaps aren't sRGB
            pShaderShadow->EnableTexture(SAMPLER_MRAO, true);           // MRAO texture
            pShaderShadow->EnableSRGBRead(SAMPLER_MRAO, false);         // MRAO isn't sRGB
            pShaderShadow->EnableTexture(SAMPLER_NORMAL, true);         // Normal texture
            pShaderShadow->EnableSRGBRead(SAMPLER_NORMAL, false);       // Normals aren't sRGB
            pShaderShadow->EnableTexture(SAMPLER_SPECULAR, true);       // Specular F0 texture
            pShaderShadow->EnableSRGBRead(SAMPLER_SPECULAR, true);      // Specular F0 is sRGB

            // If the flashlight is on, set up its textures
            if (bHasFlashlight)
            {
                pShaderShadow->EnableTexture(SAMPLER_SHADOWDEPTH, true);        // Shadow depth map
                pShaderShadow->SetShadowDepthFiltering(SAMPLER_SHADOWDEPTH);
                pShaderShadow->EnableSRGBRead(SAMPLER_SHADOWDEPTH, false);
                pShaderShadow->EnableTexture(SAMPLER_RANDOMROTATION, true);     // Noise map
                pShaderShadow->EnableTexture(SAMPLER_FLASHLIGHT, true);         // Flashlight cookie
                pShaderShadow->EnableSRGBRead(SAMPLER_FLASHLIGHT, true);
            }

            // Setting up envmap
            if (bHasEnvTexture)
            {
                pShaderShadow->EnableTexture(SAMPLER_ENVMAP, true); // Envmap
                if (g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE)
                {
                    pShaderShadow->EnableSRGBRead(SAMPLER_ENVMAP, true); // Envmap is only sRGB with HDR disabled?
                }
            }

            // Enabling sRGB writing
            // See common_ps_fxc.h line 349
            // PS2b shaders and up write sRGB
            pShaderShadow->EnableSRGBWrite(true);

            if (IS_FLAG_SET(MATERIAL_VAR_MODEL))
            {
                // We only need the position and surface normal
                unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;
                // We need three texcoords, all in the default float2 size
                pShaderShadow->VertexShaderVertexFormat(flags, 1, 0, 0);
            }
            else
            {
                // We need the position, surface normal, and vertex compression format
                unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL;
                // We only need one texcoord, in the default float2 size
                pShaderShadow->VertexShaderVertexFormat(flags, 3, 0, 0);
            }
        
            int useParallax = params[info.useParallax]->GetIntValue();
            if (!mat_pbr_parallaxmap.GetBool())
            {
                useParallax = 0;
            }

            if (!g_pHardwareConfig->SupportsShaderModel_3_0() || mat_pbr_force_20b.GetBool())
            {
                // Setting up static vertex shader

                // Setting up static pixel shader
            }
            else
            {
                // Setting up static vertex shader

                // Setting up static pixel shader
            }

            // Setting up fog
            DefaultFog(); // I think this is correct

            // HACK HACK HACK - enable alpha writes all the time so that we have them for underwater stuff
            pShaderShadow->EnableAlphaWrites(bFullyOpaque);
        }

        // Actually draw the shader
        Draw();
    };

// Closing it off
END_SHADER;
