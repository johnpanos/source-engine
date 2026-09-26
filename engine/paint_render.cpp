//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Drawing the paint maps (engine/paint.cpp) on world surfaces.
//
// Each paint page has a texture of the page's size, holding per luxel the
// gel color (the game's *_paint_color ConVars) and its coverage in alpha, and
// a LightmappedPaint material that samples it. The textures are a cache of
// the pages: they are rebuilt when the pages are reallocated and refreshed
// from the pages' changed rectangles once per frame, before the world draws.
// Painted surfaces are drawn again with their page's material after the
// opaque world (the CS:GO-era engine's paint pass, which drew the surface's
// own material with the paint shader).
//
//=============================================================================//

#include "quakedef.h"
#include "paint.h"
#include "paint_render.h"
#include "gl_model_private.h"
#include "gl_matsysiface.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/itexture.h"
#include "materialsystem/imesh.h"
#include "materialsystem/materialsystem_config.h"
#include "pixelwriter.h"
#include "vtf/vtf.h"
#include "tier1/KeyValues.h"
#include "engine/ienginepaint.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar r_hidepaintedsurfaces( "r_hidepaintedsurfaces", "0", FCVAR_CHEAT,
    "If enabled, hides all surfaces which have been painted." );
ConVar r_redownloadallpaintmaps( "r_redownloadallpaintmaps", "0", FCVAR_DEVELOPMENTONLY );

void Shader_DrawSurfaceDynamic(
    IMatRenderContext *pRenderContext, SurfaceHandle_t surfID, bool bShadowDepth );

//-----------------------------------------------------------------------------
// Paint colors: the game's ConVars (paint_color_manager.cpp), as retail's
// material system read them.
//-----------------------------------------------------------------------------
static void ReadColor( const char *pName, const unsigned char fallback[4], unsigned char out[4] )
{
	ConVarRef var( pName, true );
	V_memcpy( out, fallback, 4 );
	if ( !var.IsValid() )
		return;
	int rgba[4] = { fallback[0], fallback[1], fallback[2], fallback[3] };
	int n = sscanf( var.GetString(), "%d %d %d %d", &rgba[0], &rgba[1], &rgba[2], &rgba[3] );
	for ( int i = 0; i < MIN( n, 4 ); ++i )
		out[i] = (unsigned char)clamp( rgba[i], 0, 255 );
}

static void ReadPaintColors( unsigned char colors[ENGINE_PAINT_NO_POWER + 1][4] )
{
	static const unsigned char bounce[4] = { 0, 165, 255, 255 };
	static const unsigned char speed[4] = { 255, 106, 0, 255 };
	static const unsigned char portal[4] = { 128, 128, 128, 255 };
	static const unsigned char erase[4] = { 0, 0, 0, 0 };
	// paint_enum.h order: bounce, reflect (drawn as speed, as the game does), speed, portal, erase.
	ReadColor( "bounce_paint_color", bounce, colors[0] );
	ReadColor( "speed_paint_color", speed, colors[1] );
	ReadColor( "speed_paint_color", speed, colors[2] );
	ReadColor( "portal_paint_color", portal, colors[3] );
	ReadColor( "erase_color", erase, colors[4] );
}

//-----------------------------------------------------------------------------
// The texture of one page
//-----------------------------------------------------------------------------
class CPaintPageRegenerator : public ITextureRegenerator
{
public:
	CPaintPageRegenerator() : m_nPage( -1 ) {}

	void SetPage( int nPage ) { m_nPage = nPage; }

	virtual void RegenerateTextureBits(
	    ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pRect )
	{
		CPaintPage *pPage = g_PaintManager.Page( m_nPage );
		if ( !pPage || !pPage->Data() || pVTFTexture->Width() != pPage->Width() ||
		     pVTFTexture->Height() != pPage->Height() )
			return;

		unsigned char colors[ENGINE_PAINT_NO_POWER + 1][4];
		ReadPaintColors( colors );

		Rect_t rect;
		if ( pRect )
		{
			rect = *pRect;
		}
		else
		{
			rect.x = rect.y = 0;
			rect.width = pPage->Width();
			rect.height = pPage->Height();
		}

		CPixelWriter writer;
		writer.SetPixelMemory( pVTFTexture->Format(), pVTFTexture->ImageData( 0, 0, 0 ),
		    pVTFTexture->RowSizeInBytes( 0 ) );
		for ( int y = rect.y; y < rect.y + rect.height; ++y )
		{
			writer.Seek( rect.x, y );
			const unsigned char *pRow = pPage->Data() + y * pPage->Width();
			for ( int x = rect.x; x < rect.x + rect.width; ++x )
			{
				const unsigned char b = pRow[x];
				const unsigned char *c =
				    colors[MIN( (int)PaintByte::Power( b ), ENGINE_PAINT_NO_POWER )];
				writer.WritePixel( c[0], c[1], c[2], (int)( PaintByte::Alpha( b ) * 255.0f ) );
			}
		}
	}

	virtual void Release() {}

private:
	int m_nPage;
};

struct PaintPageResources_t
{
	CPaintPageRegenerator m_Regenerator;
	ITexture *m_pTexture;
	IMaterial *m_pMaterial;
};

static CUtlVector<PaintPageResources_t *> s_PaintPages;
static int s_nPaintGeneration = -1;

static void DestroyPaintTextures()
{
	for ( int i = 0; i < s_PaintPages.Count(); ++i )
	{
		PaintPageResources_t *pRes = s_PaintPages[i];
		if ( pRes->m_pMaterial )
		{
			pRes->m_pMaterial->DecrementReferenceCount();
			pRes->m_pMaterial->DeleteIfUnreferenced();
		}
		if ( pRes->m_pTexture )
		{
			pRes->m_pTexture->SetTextureRegenerator( NULL );
			pRes->m_pTexture->DecrementReferenceCount();
			pRes->m_pTexture->DeleteIfUnreferenced();
		}
		delete pRes;
	}
	s_PaintPages.RemoveAll();
}

static void CreatePaintTextures()
{
	DestroyPaintTextures();
	for ( int i = 0; i < g_PaintManager.PageCount(); ++i )
	{
		PaintPageResources_t *pRes = new PaintPageResources_t;
		pRes->m_pTexture = NULL;
		pRes->m_pMaterial = NULL;
		s_PaintPages.AddToTail( pRes );

		CPaintPage *pPage = g_PaintManager.Page( i );
		if ( !pPage || !pPage->Data() )
			continue;

		char szTexture[64];
		V_snprintf( szTexture, sizeof( szTexture ), "_rt_paintmap%d", i );
		pRes->m_Regenerator.SetPage( i );
		pRes->m_pTexture = materials->CreateProceduralTexture( szTexture, TEXTURE_GROUP_LIGHTMAP,
		    pPage->Width(), pPage->Height(), IMAGE_FORMAT_RGBA8888,
		    TEXTUREFLAGS_PROCEDURAL | TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD |
		        TEXTUREFLAGS_SINGLECOPY | TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT );
		if ( !pRes->m_pTexture )
			continue;
		pRes->m_pTexture->SetTextureRegenerator( &pRes->m_Regenerator );

		char szMaterial[64];
		V_snprintf( szMaterial, sizeof( szMaterial ), "__paintpage%d", i );
		KeyValues *pKeys = new KeyValues( "LightmappedPaint" );
		pKeys->SetString( "$paintmap", szTexture );
		pRes->m_pMaterial = materials->CreateMaterial( szMaterial, pKeys );
		if ( pRes->m_pMaterial )
			pRes->m_pMaterial->IncrementReferenceCount();

		pPage->MarkAllDirty();
	}
}

void R_PaintUpdateTextures()
{
	if ( s_nPaintGeneration != g_PaintManager.Generation() )
	{
		s_nPaintGeneration = g_PaintManager.Generation();
		if ( g_PaintManager.HasPaintmap() )
			CreatePaintTextures();
		else
			DestroyPaintTextures();
	}

	if ( r_redownloadallpaintmaps.GetBool() )
	{
		r_redownloadallpaintmaps.SetValue( 0 );
		for ( int i = 0; i < g_PaintManager.PageCount(); ++i )
			g_PaintManager.Page( i )->MarkAllDirty();
	}

	for ( int i = 0; i < s_PaintPages.Count(); ++i )
	{
		CPaintPage *pPage = g_PaintManager.Page( i );
		Rect_t rect;
		if ( pPage && s_PaintPages[i]->m_pTexture && pPage->TakeDirtyRect( rect ) )
		{
			s_PaintPages[i]->m_pTexture->Download( &rect );
		}
	}
}

void R_PaintShutdown()
{
	DestroyPaintTextures();
	s_nPaintGeneration = -1;
}

//-----------------------------------------------------------------------------
// The paint pass
//-----------------------------------------------------------------------------
void R_DrawPaintedSurfaces(
    IMatRenderContext *pRenderContext, const CUtlVector<SurfaceHandle_t> &surfaces )
{
	if ( !surfaces.Count() || r_hidepaintedsurfaces.GetBool() || !g_PaintManager.HasPaintmap() ||
	     g_pMaterialSystemConfig->nFullbright == 1 )
		return;

	for ( int i = 0; i < surfaces.Count(); ++i )
	{
		SurfaceHandle_t surfID = surfaces[i];
		int nPage;
		Rect_t rect;
		if ( !g_PaintManager.SurfaceRect( surfID, nPage, rect ) || nPage >= s_PaintPages.Count() ||
		     !s_PaintPages[nPage]->m_pMaterial )
			continue;

		pRenderContext->Bind( s_PaintPages[nPage]->m_pMaterial );
		pRenderContext->BindLightmapPage( nPage );
		Shader_DrawSurfaceDynamic( pRenderContext, surfID, false );
	}
}
