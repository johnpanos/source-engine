// DWARF declaration skeleton for game/client/portal2/c_portal_race_checkpoint.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x77d50 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
}

// game/client/portal2/c_portal_race_checkpoint.cpp:12
ConVar cl_race_checkpoint_inactive_color;

// game/client/portal2/c_portal_race_checkpoint.cpp:13
ConVar cl_race_checkpoint_active_color;

// game/client/portal2/c_portal_race_checkpoint.cpp:16 sizeof=0xd90 (i386)
struct C_PortalRaceCheckpoint : public C_BaseAnimating
{
public:
	C_PortalRaceCheckpoint();  // line 21
	virtual ~C_PortalRaceCheckpoint();  // line 22
	virtual int DrawModel( int, const RenderableInstance_t & );  // line 24
	virtual bool ShouldDraw();  // line 25
private:
	bool InitMaterials();  // line 29
	IMaterial *m_pCheckpointMaterial; // +0xd84  // line 31
	IMaterialVar *m_pMyColor; // +0xd88  // line 32
	bool m_bCheckpointActive; // +0xd8c  // line 34
};

// game/client/portal2/c_portal_race_checkpoint.cpp:21 @0x52dd10 _ZN22C_PortalRaceCheckpointC1Ev
C_PortalRaceCheckpoint::C_PortalRaceCheckpoint()
{
}

// game/client/portal2/c_portal_race_checkpoint.cpp:21 @0x52dd80 _ZN22C_PortalRaceCheckpointC2Ev
C_PortalRaceCheckpoint::C_PortalRaceCheckpoint()
{
}

// game/client/portal2/c_portal_race_checkpoint.cpp:22 @0x52dbe0 _ZN22C_PortalRaceCheckpointD0Ev
C_PortalRaceCheckpoint::~C_PortalRaceCheckpoint()
{
}

// game/client/portal2/c_portal_race_checkpoint.cpp:22 @0x52dc50 _ZN22C_PortalRaceCheckpointD1Ev
C_PortalRaceCheckpoint::~C_PortalRaceCheckpoint()
{
}

// game/client/portal2/c_portal_race_checkpoint.cpp:22 @0x52dcb0 _ZN22C_PortalRaceCheckpointD2Ev
C_PortalRaceCheckpoint::~C_PortalRaceCheckpoint()
{
}

// game/client/portal2/c_portal_race_checkpoint.cpp:24 @0x52de00 _ZN22C_PortalRaceCheckpoint9DrawModelEiRK20RenderableInstance_t
int C_PortalRaceCheckpoint::DrawModel( int flags, const RenderableInstance_t &instance )
{
	Vector vecOrigin;  // line 65
	Vector vecForward;  // line 66
	Vector vecRight;  // line 66
	Vector vecUp;  // line 66
	Vector vecVertex;  // line 68
	Color meshColor;  // line 75
	float flColors[3];  // line 77
	float pColors[4];  // line 85
	CMatRenderContextPtr pRenderContext;  // line 92
	IMesh *pMesh;  // line 94
	CMeshBuilder meshBuilder;  // line 95
	// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 144
	// inlined CMeshBuilder::~CMeshBuilder() at line 144
	// inlined ConVar::GetColor() at line 75
	// inlined CMeshBuilder::End() at line 141
	// inlined CMeshBuilder::AdvanceVertex() at line 138
	// inlined CMeshBuilder::Position3fv() at line 137
	// inlined CMeshBuilder::TexCoord2f() at line 136
	// inlined CMeshBuilder::Color4fv() at line 135
	// inlined Vector::operator-=() at line 133
	// inlined Vector::operator-=() at line 132
	// inlined CMeshBuilder::AdvanceVertex() at line 128
	// inlined CMeshBuilder::Position3fv() at line 127
	// inlined CMeshBuilder::TexCoord2f() at line 126
	// inlined CMeshBuilder::Color4fv() at line 125
	// inlined Vector::operator+=() at line 123
	// inlined Vector::operator-=() at line 122
	// inlined CMeshBuilder::AdvanceVertex() at line 118
	// inlined CMeshBuilder::Position3fv() at line 117
	// inlined CMeshBuilder::TexCoord2f() at line 116
	// inlined CMeshBuilder::Color4fv() at line 115
	// inlined Vector::operator+=() at line 113
	// inlined Vector::operator+=() at line 112
	// inlined CMeshBuilder::AdvanceVertex() at line 108
	// inlined CMeshBuilder::Position3fv() at line 107
	// inlined CMeshBuilder::TexCoord2f() at line 106
	// inlined CMeshBuilder::Color4fv() at line 105
	// inlined Vector::operator-=() at line 103
	// inlined Vector::operator+=() at line 102
	// inlined Vector::operator-=() at line 70
	// inlined CMeshBuilder::Begin() at line 98
	// inlined CMeshBuilder::CMeshBuilder() at line 95
	// inlined CMatRenderContextPtr::CMatRenderContextPtr() at line 92
	// inlined C_BaseEntity::SetRenderColor() at line 89
	// inlined ConVar::GetColor() at line 75
	// inlined Vector::operator*=() at line 72
	// inlined Vector::operator*=() at line 71
	// inlined Vector::operator*() at line 70
	// inlined C_PortalRaceCheckpoint::InitMaterials() at line 59
	// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 144
	// inlined CMeshBuilder::~CMeshBuilder() at line 144
}

// game/client/portal2/c_portal_race_checkpoint.cpp:25 @0x52db00 _ZN22C_PortalRaceCheckpoint10ShouldDrawEv
bool C_PortalRaceCheckpoint::ShouldDraw()
{
}

// game/client/portal2/c_portal_race_checkpoint.cpp:29 @0x52db10 _ZN22C_PortalRaceCheckpoint13InitMaterialsEv
bool C_PortalRaceCheckpoint::InitMaterials()
{
	bool bFound;  // line 158
}
