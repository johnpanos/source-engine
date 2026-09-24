// DWARF declaration skeleton for game/client/portal2/polygonbutton.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x80b90 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
}

// game/client/portal2/polygonbutton.cpp:13 (declaration)
void CPolygonButton( vgui::Panel *parent, const char *panelName );

// game/client/portal2/polygonbutton.cpp:13 @0x6130e0 _ZN14CPolygonButtonC2EPN4vgui5PanelEPKc
CPolygonButton::CPolygonButton( vgui::Panel *parent, const char *panelName )
{
	// inlined vgui::surface() at line 24
	// inlined vgui::surface() at line 23
	// inlined CUtlVector<Vector2D,CUtlMemory<Vector2D, int> >::RemoveAll() at line 17
	// inlined CUtlVector<Vector2D,CUtlMemory<Vector2D, int> >::RemoveAll() at line 16
	// inlined CUtlVector<Vector2D,CUtlMemory<Vector2D, int> >::CUtlVector() at line 14
	// inlined CUtlVector<Vector2D,CUtlMemory<Vector2D, int> >::CUtlVector() at line 14
	// inlined CPolygonButton::CPolygonButton_RegisterKBMap::CPolygonButton_RegisterKBMap() at line 14
	// inlined CPolygonButton::CPolygonButton_Register::CPolygonButton_Register() at line 14
	// inlined CPolygonButton::CPolygonButton_RegisterMap::CPolygonButton_RegisterMap() at line 14
	// inlined CUtlVector<Vector2D,CUtlMemory<Vector2D, int> >::~CUtlVector() at line 25
	// inlined CUtlVector<Vector2D,CUtlMemory<Vector2D, int> >::~CUtlVector() at line 25
}

// game/client/portal2/polygonbutton.cpp:13 @0x613440 _ZN14CPolygonButtonC1EPN4vgui5PanelEPKc
CPolygonButton::CPolygonButton( vgui::Panel *parent, const char *panelName )
{
}

// game/client/portal2/polygonbutton.cpp:28 @0x612840 _ZN14CPolygonButton13ApplySettingsEP9KeyValues
void CPolygonButton::ApplySettings( KeyValues *data )
{
}

// game/client/portal2/polygonbutton.cpp:37 @0x612d30 _ZN14CPolygonButton14UpdateHotspotsEP9KeyValues
void CPolygonButton::UpdateHotspots( KeyValues *data )
{
	KeyValues *points;  // line 56
	{
		KeyValues *value;  // line 77
		{
			const char *str;  // line 79
			float x;  // line 81
			float y;  // line 81
			// inlined CUtlVector<Vector2D,CUtlMemory<Vector2D, int> >::AddToTail() at line 84
		}
	}
	{
		KeyValues *value;  // line 59
		{
			const char *str;  // line 61
			float x;  // line 63
			float y;  // line 63
			// inlined CUtlVector<Vector2D,CUtlMemory<Vector2D, int> >::AddToTail() at line 66
		}
	}
	// inlined CUtlVector<Vector2D,CUtlMemory<Vector2D, int> >::RemoveAll() at line 53
	// inlined CUtlVector<Vector2D,CUtlMemory<Vector2D, int> >::RemoveAll() at line 52
}

// game/client/portal2/polygonbutton.cpp:94 @0x612990 _ZN14CPolygonButton16IsWithinTraverseEiib
IBaseClientDLL::VPANEL CPolygonButton::IsWithinTraverse( int x, int y, bool traversePopups )
{
	IBaseClientDLL::VPANEL within;  // line 101
	{
		int wide;  // line 104
		int tall;  // line 104
		bool inside;  // line 108
		{
			int i;  // line 109
			{
				const Vector2D &pos1;  // line 111
				const Vector2D &pos2;  // line 112
				Vector p1;  // line 113
				Vector p2;  // line 114
				Vector out;  // line 115
				// inlined Vector::Cross() at line 115
			}
		}
	}
}

// game/client/portal2/polygonbutton.cpp:136 @0x612b20 _ZN14CPolygonButton13PerformLayoutEv
void CPolygonButton::PerformLayout()
{
	int wide;  // line 138
	int tall;  // line 138
	{
		int i;  // line 175
		{
			float x;  // line 177
			float y;  // line 178
			// inlined CUtlVector<Vector2D,CUtlMemory<Vector2D, int> >::operator[]() at line 177
			// inlined FontVertex_t::Init() at line 179
		}
	}
	// inlined CUtlVector<Vector2D,CUtlMemory<Vector2D, int> >::Count() at line 155
	// inlined Vector2D::Init() at line 168
	// inlined Vector2D::Init() at line 169
	// inlined CUtlVector<Vector2D,CUtlMemory<Vector2D, int> >::Count() at line 171
	{
		int i;  // line 159
		{
			float x;  // line 161
			float y;  // line 162
			// inlined CUtlVector<Vector2D,CUtlMemory<Vector2D, int> >::operator[]() at line 161
			// inlined FontVertex_t::Init() at line 163
		}
	}
}

// game/client/portal2/polygonbutton.cpp:196 @0x612760 _ZN14CPolygonButton16ComputeAlignmentERiS0_S0_S0_
void CPolygonButton::ComputeAlignment( int &tx0, int &ty0, int &tx1, int &ty1 )
{
	Vector2D center;  // line 198
	int textWide;  // line 201
	int textTall;  // line 201
	// inlined Vector2D::operator*() at line 198
}

// game/client/portal2/polygonbutton.cpp:215 @0x612900 _ZN14CPolygonButton15PaintBackgroundEv
void CPolygonButton::PaintBackground()
{
	Color c;  // line 217
	// inlined vgui::surface() at line 218
	// inlined vgui::surface() at line 219
	// inlined vgui::surface() at line 220
}

// game/client/portal2/polygonbutton.cpp:227 @0x612880 _ZN14CPolygonButton11PaintBorderEv
void CPolygonButton::PaintBorder()
{
	Color c;  // line 229
	// inlined vgui::surface() at line 230
	// inlined vgui::surface() at line 231
	// inlined vgui::surface() at line 232
}

// game/client/portal2/polygonbutton.cpp:236 @0x612720 _ZN14CPolygonButton19ApplySchemeSettingsEPN4vgui7ISchemeE
void CPolygonButton::ApplySchemeSettings( vgui::IScheme *scheme )
{
}

// public/tier1/utlmemory.h:707 @0x6136a0 _ZN10CUtlMemoryI8Vector2DiE4GrowEi
void CUtlMemory<Vector2D,int>::Grow( int num )
{
	int nAllocationRequested;  // line 720
	int nNewAllocationCount;  // line 724
	// inlined UtlMemory_CalcNewAllocationCount() at line 724
	// inlined CUtlMemory<Vector2D,int>::IsExternallyAllocated() at line 711
	// inlined MemAlloc_Alloc() at line 761
}
