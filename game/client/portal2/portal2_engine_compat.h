//========= Portal 2 port ======================================================//
//
// Purpose: Client adapter layer between the imported Portal 2 client code
//          (written for the CS:GO-era base game and engine) and this tree's
//          frozen client engine interfaces (IVEngineClient, IClientRenderable,
//          IMatRenderContext, IVRenderView, IShadowMgr, IMaterialSystem).
//
// Only Portal 2 client translation units include this header. Engine
// interfaces are frozen: nothing here adds a virtual to an engine interface.
// Every CS:GO call that this engine lacks is either implemented through the
// existing engine API (documented per function) or returns the documented
// unsupported result and emits a one-time
//     DevWarning( "Portal 2: <feature> is not supported by this engine\n" )
// Nothing fakes success silently.
//
//=============================================================================//
//
// RENDERING ADAPTATION PATTERN (apply this to every Portal 2 client class)
// ------------------------------------------------------------------------
//
// This engine calls the SDK 2013 renderable API; CS:GO added a second API on
// top. A Portal 2 override written for the CS:GO API compiles here only once
// it is bridged onto the virtual this engine really calls; otherwise the code
// is dead (or does not compile because the base virtual does not exist).
//
// 1. DrawModel
//    CS:GO:  virtual int DrawModel( int flags, const RenderableInstance_t &instance );
//    Engine: virtual int DrawModel( int flags );   (IClientRenderable, frozen)
//
//    In the class declaration keep the Portal 2 two-argument function and add
//    the bridge right next to it:
//
//        // Portal 2 port: CS:GO renderable API bridged onto DrawModel( int ).
//        PORTAL2_DRAWMODEL_BRIDGE();
//        virtual int DrawModel( int flags, const RenderableInstance_t &instance );
//
//    PORTAL2_DRAWMODEL_BRIDGE() overrides the engine's DrawModel( int flags )
//    and forwards to the two-argument function with a RenderableInstance_t
//    built from GetFxBlend() (the SDK's per-frame computed render alpha, which
//    is what CS:GO's leaf system passed as instance.m_nAlpha).
//    - Do NOT put OVERRIDE on the two-argument function in the first Portal 2
//      class of a hierarchy: no base-game class declares it. Classes derived
//      from a Portal 2 class that already has the bridge may use OVERRIDE and
//      need not repeat the bridge (repeating it is harmless).
//    - Inside the two-argument body, a call to an SDK base class must become
//          // Portal 2 port: the SDK base has only DrawModel( int ).
//          return BaseClass::DrawModel( flags );
//      The SDK base reads its alpha from GetFxBlend()/m_nRenderFXBlend itself.
//      If the Portal 2 code passed a modified instance down, apply that alpha
//      through the SDK state instead (e.g. SetRenderColorA/m_nRenderFXBlend)
//      and say so in a "Portal 2 port:" comment.
//    - A call to a Portal 2 base class keeps BaseClass::DrawModel( flags, instance ).
//
// 2. Translucency
//    CS:GO:  virtual RenderableTranslucencyType_t ComputeTranslucencyType();
//    Engine: virtual bool IsTransparent(); virtual bool IsTwoPass();
//
//        // Portal 2 port: translucency type bridged onto IsTransparent()/IsTwoPass().
//        PORTAL2_TRANSLUCENCY_BRIDGE();
//        virtual RenderableTranslucencyType_t ComputeTranslucencyType( void );
//
//    - BaseClass::ComputeTranslucencyType() on an SDK base becomes
//          Portal2_TranslucencyType( BaseClass::IsTransparent(), BaseClass::IsTwoPass() )
//    - pEntity->ComputeTranslucencyType() on an arbitrary entity becomes
//          Portal2_ComputeTranslucencyType( pEntity )
//
// 3. Fast-path model rendering
//    CS:GO:  virtual IClientModelRenderable *GetClientModelRenderable();
//    This engine has no fast path and always calls DrawModel(). Keep the
//    function as a plain Portal 2 virtual (no OVERRIDE); BaseClass calls that
//    reach an SDK class become "return NULL;". NULL is also CS:GO's "use
//    DrawModel" answer, so the behavior matches.
//
// 4. Simulate
//    CS:GO:  virtual bool Simulate();   (false = drop from the simulation list)
//    Engine: virtual void Simulate();   (C_BaseEntity, always simulated)
//    Declare "virtual void Simulate( void );". In the body, "return
//    BaseClass::Simulate();" stays valid (returning a void expression);
//    "return true;"/"return false;" become "return;". Nothing is dropped from
//    simulation in this engine; note that in the "Portal 2 port:" comment when
//    the Portal 2 code relied on returning false.
//
// 5. SetupBones
//    CS:GO passes matrix3x4a_t; this engine's virtual takes matrix3x4_t:
//        virtual bool SetupBones( matrix3x4_t *pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime );
//    (matrix3x4a_t exists in mathlib.h and converts to matrix3x4_t.)
//
// 6. Split-screen slot arguments
//    This engine has exactly one local player (slot 0). Split-screen loops
//    collapse through FOR_EACH_VALID_SPLITSCREEN_PLAYER; per-slot virtuals
//    such as ShouldSuppressForSplitScreenPlayer( nSlot ) and
//    GetPlayerRenderMode( nSlot ) exist on the base classes under PORTAL2 but
//    the engine's leaf system never calls them. A class that relies on them
//    must fold them into ShouldDraw() for slot 0 (see C_Portal_Player).
//
// 7. Stencil state
//    CS:GO:  pRenderContext->SetStencilState( state );
//    Here:   Portal2_SetStencilState( pRenderContext, state );
//    ShaderStencilState_t and SHADER_STENCILOP_* / SHADER_STENCILFUNC_* are
//    defined below with this engine's StencilOperation_t /
//    StencilComparisonFunction_t values (identical D3D numbering).
//
// 8. Other IMatRenderContext / IVRenderView / IShadowMgr calls
//    pRenderContext->OverrideDepthEnable( e, w, t ) -> Portal2_OverrideDepthEnable( pRenderContext, e, w, t )
//    pRenderContext->FlipCullMode()                 -> Portal2_FlipCullMode( pRenderContext )
//    pRenderContext->PushScissorRect( l, t, r, b )  -> Portal2_PushScissorRect( pRenderContext, l, t, r, b )
//    pRenderContext->PopScissorRect()               -> Portal2_PopScissorRect( pRenderContext )
//    render->Push3DView( pRenderContext, ... )      -> render->Push3DView( ... ) (drop the context)
//    render->PopView( pRenderContext, frustum )     -> render->PopView( frustum )
//    shadowmgr->Push/PopFlashlightScissorBounds()   -> Portal2_Push/PopFlashlightScissorBounds()
//    materials->GetAspectRatioInfo()                -> Portal2_GetAspectRatioInfo()
//    IMesh::GetCachedPerFrameMeshData() has no equivalent: rebuild the
//    dynamic mesh for each use (see ICachedPerFrameMeshData below).
//
// 9. IVEngineClient calls this engine lacks: call Portal2Engine::<CS:GO name>(...)
//    (declared below) instead of engine-><CS:GO name>(...).
//
//=============================================================================//

#ifndef PORTAL2_ENGINE_COMPAT_H
#define PORTAL2_ENGINE_COMPAT_H
#ifdef _WIN32
#pragma once
#endif

#include "iclientrenderable.h"
#include "materialsystem/imaterialsystem.h"
#include "inputsystem/ButtonCode.h"
#include "tier1/utlvector.h"
#include "tier1/bitvec.h"
#include "mathlib/vector.h"

class IMesh;
class C_BaseEntity;
class C_BasePlayer;
struct model_t;

// Portal 2's client input and interpolation extensions are absent in this SDK.
// The target is single-player, so use the SDK interpolator unchanged.
template <typename T> class CInterpolatedVar;
template <typename T>
using CDiscontinuousInterpolatedVar = CInterpolatedVar<T>;

// CS:GO player render mode; this engine always renders the player the same way.
enum PlayerRenderMode_t
{
	PLAYER_RENDER_FIRSTPERSON,
	PLAYER_RENDER_THIRDPERSON,
};

// This SDK exposes client transforms as individual setters rather than a
// server-style Teleport virtual. Reset interpolation after the atomic update.
void Portal2_ClientTeleport( C_BaseEntity *pEntity, const Vector *pOrigin,
							 const QAngle *pAngles, const Vector *pVelocity );

// Use-entity state is replicated by the SDK and has no writable client API.
// These adapters preserve prediction-side pickup behavior without extending it.
void Portal2_ClientSetUseEntity( C_BasePlayer *pPlayer, C_BaseEntity *pUseEntity );
void Portal2_ClientForceDropOfCarriedPhysObjects( C_BasePlayer *pPlayer );

#ifndef LINK_ENTITY_TO_CLASS_CLIENTONLY
#define LINK_ENTITY_TO_CLASS_CLIENTONLY( localName, className ) LINK_ENTITY_TO_CLASS( localName, className )
#endif

//-----------------------------------------------------------------------------
// CS:GO renderable API types (public/iclientrenderable.h in CS:GO)
//-----------------------------------------------------------------------------
struct RenderableInstance_t
{
	uint8 m_nAlpha;
};

// The instance CS:GO's leaf system would have passed: the SDK's per-frame
// render alpha, computed by ComputeFxBlend() and read through GetFxBlend().
inline RenderableInstance_t Portal2_RenderableInstance( int nFxBlend )
{
	RenderableInstance_t instance;
	instance.m_nAlpha = (uint8)( ( nFxBlend < 0 ) ? 0 : ( ( nFxBlend > 255 ) ? 255 : nFxBlend ) );
	return instance;
}

enum RenderableTranslucencyType_t
{
	RENDERABLE_IS_OPAQUE = 0,
	RENDERABLE_IS_TRANSLUCENT,
	RENDERABLE_IS_TWO_PASS,	// has both translucent and opaque sub-parts
};

inline RenderableTranslucencyType_t Portal2_TranslucencyType( bool bIsTransparent, bool bIsTwoPass )
{
	if ( !bIsTransparent )
		return RENDERABLE_IS_OPAQUE;
	return bIsTwoPass ? RENDERABLE_IS_TWO_PASS : RENDERABLE_IS_TRANSLUCENT;
}

// CS:GO's ComputeTranslucencyType() for any renderable, from the SDK virtuals.
inline RenderableTranslucencyType_t Portal2_ComputeTranslucencyType( IClientRenderable *pRenderable )
{
	return Portal2_TranslucencyType( pRenderable->IsTransparent(), pRenderable->IsTwoPass() );
}

enum ModelDataCategory_t
{
	MODEL_DATA_LIGHTING_MODEL,	// data type returned is a RenderableLightingModel_t
	MODEL_DATA_STENCIL,			// data type returned is a ShaderStencilState_t

	MODEL_DATA_CATEGORY_COUNT,
};

// CS:GO's fast-path model interface. This engine never asks for it; it exists
// so Portal 2 declarations compile (see pattern item 3).
abstract_class IClientModelRenderable
{
public:
	virtual bool GetRenderData( void *pData, ModelDataCategory_t nCategory ) = 0;
};

// See pattern item 1.
#define PORTAL2_DRAWMODEL_BRIDGE() \
	virtual int DrawModel( int flags ) OVERRIDE { return DrawModel( flags, Portal2_RenderableInstance( GetFxBlend() ) ); }

// See pattern item 2.
#define PORTAL2_TRANSLUCENCY_BRIDGE() \
	virtual bool IsTransparent( void ) OVERRIDE { return ComputeTranslucencyType() != RENDERABLE_IS_OPAQUE; } \
	virtual bool IsTwoPass( void ) OVERRIDE { return ComputeTranslucencyType() == RENDERABLE_IS_TWO_PASS; }

// CS:GO IClientRenderable::GetRenderFlags() bits.
enum
{
	ERENDERFLAGS_NEEDS_POWER_OF_TWO_FB = 0x1,
	ERENDERFLAGS_REFRACT_ONLY_ONCE_PER_FRAME = 0x2,
};

//-----------------------------------------------------------------------------
// Stencil state (public/shaderapi/ishaderapi.h in CS:GO), expressed with this
// engine's StencilOperation_t / StencilComparisonFunction_t. The numeric
// values are the D3D ones in both engines.
//-----------------------------------------------------------------------------
typedef StencilOperation_t ShaderStencilOp_t;
typedef StencilComparisonFunction_t ShaderStencilFunc_t;

const ShaderStencilOp_t SHADER_STENCILOP_KEEP = STENCILOPERATION_KEEP;
const ShaderStencilOp_t SHADER_STENCILOP_ZERO = STENCILOPERATION_ZERO;
const ShaderStencilOp_t SHADER_STENCILOP_SET_TO_REFERENCE = STENCILOPERATION_REPLACE;
const ShaderStencilOp_t SHADER_STENCILOP_INCREMENT_CLAMP = STENCILOPERATION_INCRSAT;
const ShaderStencilOp_t SHADER_STENCILOP_DECREMENT_CLAMP = STENCILOPERATION_DECRSAT;
const ShaderStencilOp_t SHADER_STENCILOP_INVERT = STENCILOPERATION_INVERT;
const ShaderStencilOp_t SHADER_STENCILOP_INCREMENT_WRAP = STENCILOPERATION_INCR;
const ShaderStencilOp_t SHADER_STENCILOP_DECREMENT_WRAP = STENCILOPERATION_DECR;

const ShaderStencilFunc_t SHADER_STENCILFUNC_NEVER = STENCILCOMPARISONFUNCTION_NEVER;
const ShaderStencilFunc_t SHADER_STENCILFUNC_LESS = STENCILCOMPARISONFUNCTION_LESS;
const ShaderStencilFunc_t SHADER_STENCILFUNC_EQUAL = STENCILCOMPARISONFUNCTION_EQUAL;
const ShaderStencilFunc_t SHADER_STENCILFUNC_LEQUAL = STENCILCOMPARISONFUNCTION_LESSEQUAL;
const ShaderStencilFunc_t SHADER_STENCILFUNC_GREATER = STENCILCOMPARISONFUNCTION_GREATER;
const ShaderStencilFunc_t SHADER_STENCILFUNC_NOTEQUAL = STENCILCOMPARISONFUNCTION_NOTEQUAL;
const ShaderStencilFunc_t SHADER_STENCILFUNC_GEQUAL = STENCILCOMPARISONFUNCTION_GREATEREQUAL;
const ShaderStencilFunc_t SHADER_STENCILFUNC_ALWAYS = STENCILCOMPARISONFUNCTION_ALWAYS;

struct ShaderStencilState_t
{
	bool m_bEnable;
	ShaderStencilOp_t m_FailOp;
	ShaderStencilOp_t m_ZFailOp;
	ShaderStencilOp_t m_PassOp;
	ShaderStencilFunc_t m_CompareFunc;
	int m_nReferenceValue;
	uint32 m_nTestMask;
	uint32 m_nWriteMask;

	ShaderStencilState_t()
	{
		m_bEnable = false;
		m_PassOp = m_FailOp = m_ZFailOp = SHADER_STENCILOP_KEEP;
		m_CompareFunc = SHADER_STENCILFUNC_ALWAYS;
		m_nReferenceValue = 0;
		m_nTestMask = m_nWriteMask = 0xFFFFFFFF;
	}
};

// CS:GO IMatRenderContext::SetStencilState, applied through this engine's
// individual stencil setters (all eight fields, in one place).
void Portal2_SetStencilState( IMatRenderContext *pRenderContext, const ShaderStencilState_t &state );

//-----------------------------------------------------------------------------
// Other IMatRenderContext additions
//-----------------------------------------------------------------------------

// CS:GO OverrideDepthEnable( bEnable, bDepthWriteEnable, bDepthTestEnable ).
// This engine's override controls depth writes only; a request to disable the
// depth test is reported once as unsupported and the test stays enabled.
void Portal2_OverrideDepthEnable( IMatRenderContext *pRenderContext, bool bEnable, bool bDepthWriteEnable, bool bDepthTestEnable = true );

// CS:GO FlipCullMode(). This engine cannot read the current cull mode, so the
// adapter tracks the mode it last set, starting from the engine default
// (MATERIAL_CULLMODE_CCW). Paired flips (mirror views) restore the default.
void Portal2_FlipCullMode( IMatRenderContext *pRenderContext );

// CS:GO scissor stack, kept by the adapter on top of SetScissorRect().
void Portal2_PushScissorRect( IMatRenderContext *pRenderContext, int nLeft, int nTop, int nRight, int nBottom );
void Portal2_PopScissorRect( IMatRenderContext *pRenderContext );

// CS:GO IMesh::GetCachedPerFrameMeshData()/ReconstructFromCachedPerFrameMeshData()
// let one dynamic mesh be redrawn across several views. This engine has no
// such API; there is no adapter that returns cached data. Callers rebuild the
// dynamic mesh with the same inputs when they need it again. The interface is
// declared so member declarations compile.
abstract_class ICachedPerFrameMeshData
{
public:
	virtual void Free() = 0;
};

// CS:GO IShadowMgr::PushFlashlightScissorBounds()/PopFlashlightScissorBounds().
// Unsupported: flashlight scissoring stays as the engine computes it.
void Portal2_PushFlashlightScissorBounds();
void Portal2_PopFlashlightScissorBounds();

// CS:GO IMaterialSystem::GetAspectRatioInfo(), computed from the current
// back-buffer size with CS:GO's widescreen threshold (aspect >= 1.6) and
// square pixels (physical aspect == frame-buffer aspect).
struct AspectRatioInfo_t
{
	bool m_bIsWidescreen;
	bool m_bIsHidef;
	float m_flFrameBufferAspectRatio;		// width / height of framebuffer in pixels
	float m_flPhysicalAspectRatio;			// width / height of the physical display in real-life units
	float m_flFrameBuffertoPhysicalScalar;	// m_flPhysicalAspectRatio / m_flFrameBufferAspectRatio
	float m_flPhysicalToFrameBufferScalar;	// m_flFrameBufferAspectRatio / m_flPhysicalAspectRatio
	bool m_bInitialized;

	AspectRatioInfo_t() :
		m_bIsWidescreen( false ),
		m_bIsHidef( false ),
		m_flFrameBufferAspectRatio( 4.0f / 3.0f ),
		m_flPhysicalAspectRatio( 4.0f / 3.0f ),
		m_flFrameBuffertoPhysicalScalar( 1.0f ),
		m_flPhysicalToFrameBufferScalar( 1.0f ),
		m_bInitialized( false )
	{
	}
};
const AspectRatioInfo_t &Portal2_GetAspectRatioInfo();

//-----------------------------------------------------------------------------
// CS:GO binding lookup options (public/cdll_int.h in CS:GO)
//-----------------------------------------------------------------------------
enum BindingLookupOption_t
{
	BINDINGLOOKUP_ALL = -1,
	BINDINGLOOKUP_KEYBOARD_ONLY = 0,
	BINDINGLOOKUP_JOYSTICK_ONLY,
	BINDINGLOOKUP_STEAMCONTROLLER_ONLY,
};

//-----------------------------------------------------------------------------
// IVEngineClient methods this engine lacks. Call Portal2Engine::X( ... ) where
// CS:GO code calls engine->X( ... ).
//-----------------------------------------------------------------------------
typedef void ( *pfnDemoCustomDataCallback )( uint8 *pData, size_t iSize );

namespace Portal2Engine
{
	// Decodes an entity handle carried as a 32-bit user-message field.
	C_BaseEntity *EntityFromUserMessageEHandle( uint32 encodedHandle );

	// Map name without "maps/" and ".bsp" (engine->GetLevelName()); "" when not connected.
	const char *GetLevelNameShort();

	// True when this client's server runs in this process (loopback net channel).
	bool IsClientLocalToActiveServer();

	// Mod directory name (last component of engine->GetGameDirectory()), e.g. "portal2".
	const char *GetModDirectory();

	// Relative save directory with trailing slash ("save/"), as the engine's save code uses.
	const char *GetSaveDirName();

	// Split screen: this engine has one local player in slot 0.
	int GetActiveSplitScreenPlayerSlot();
	int SetActiveSplitScreenPlayerSlot( int nSlot );	// returns the previous slot; only slot 0 exists
	bool IsLocalPlayerResolvable();						// always true: the single local player

	// Commentary mode, from the client's commentary state.
	bool IsInCommentaryMode();

	// CS:GO startup image policy: "console/portal2_product_<1|2>[_widescreen]",
	// one index per session (-startup N overrides).
	void GetStartupImage( char *pDest, int nDestLen );

	// The engine's "password" convar, used by the next connect.
	void SetConnectionPassword( const char *pchCurrentPW );

	// The last user command the server acknowledged, as reported to prediction.
	int GetLastAcknowledgedCommand();

	// Desktop builds keep no map assets resident between games: always true.
	bool GameHasShutdownAndFlushedMemory();

	// Key bindings through Key_BindingForKey() over every button code.
	int Key_CodeForBinding( const char *pBinding, int nUserId = -1, int nStartCount = 0, BindingLookupOption_t nFlags = BINDINGLOOKUP_ALL );
	const char *Key_LookupBindingEx( const char *pBinding, int nUserId = -1, int nStartCount = 0, BindingLookupOption_t nFlags = BINDINGLOOKUP_ALL );

	// Ambient lighting cube at a point (engine->ComputeLighting with box colors).
	void ComputeLightingCube( const Vector &pt, bool bClamp, Vector *pBoxColors );

	// Save through the engine's "save" command. The engine saves asynchronously
	// through its command buffer, so the result reports that the save was
	// queued; pOutName receives the save file name, pOutComment is cleared.
	bool SaveGame( const char *pSaveFilename, bool bIsXSave, char *pOutName, int nOutNameSize, char *pOutComment, int nOutCommentSize );

	//-------------------------------------------------------------------------
	// Unsupported by this engine: documented result plus a one-time DevWarning.
	//-------------------------------------------------------------------------
	bool HasPaintmap();															// false: no paint maps
	bool SpherePaintSurface( const model_t *pModel, const Vector &vPosition, unsigned char color, float flSphereRadius, float flPaintCoatPercent );	// false: nothing painted
	void SphereTracePaintSurface( const model_t *pModel, const Vector &vPosition, const Vector &vContactNormal, float flSphereRadius, CUtlVector<unsigned char> &surfColors );	// leaves surfColors empty
	void RemoveAllPaint();														// no-op
	void PaintAllSurfaces( unsigned char color );								// no-op
	void RemovePaint( const model_t *pModel );									// no-op
	void LoadPaintmapData( const CUtlVector<unsigned char> &data, int nPaintmapID, int nPaintmapOffset );	// no-op
	void LoadPaintSurfBits( const CVarBitVec &paintSurfBits );					// no-op

	bool IsTransitioningToLoad();							// false: the engine gives no transition state
	void HideLoadingPlaque();								// no-op: the engine owns the loading plaque
	void SetBlurFade( float flScale );						// no-op: no engine-side GameUI blur
	void AudioLanguageChanged();							// no-op: audio language is fixed at startup
	void RegisterDemoCustomDataCallback( const char *pszName, pfnDemoCustomDataCallback pfnCallback );	// no-op
	void RecordDemoCustomData( pfnDemoCustomDataCallback pfnCallback, const void *pData, size_t iDataLength );	// no-op: not recorded
	void SOSSetOpvarFloat( const char *pOpVarName, float flValue );	// no-op: no sound operator system
	int GetMixLayerIndex( const char *pszMixLayerName );	// -1: no mix layers
	int GetMixGroupIndex( const char *pszMixGroupName );	// -1: no mix groups
	void SetMixLayerTriggerFactor( int nMixLayerIndex, int nMixGroupIndex, float flFactor );	// no-op
}

//-----------------------------------------------------------------------------
// Split-screen helpers (single local player)
//-----------------------------------------------------------------------------
class C_BasePlayer;
C_BasePlayer *GetSplitScreenViewPlayer( int nSlot = -1 );	// the local player
inline bool IsLocalSplitScreenPlayer( int nSlot ) { return nSlot == 0; }

#endif // PORTAL2_ENGINE_COMPAT_H
