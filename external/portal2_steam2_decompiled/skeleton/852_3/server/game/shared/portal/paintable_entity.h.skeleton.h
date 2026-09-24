// DWARF declaration skeleton for game/shared/portal/paintable_entity.h
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/shared/portal/paintable_entity.h:15 sizeof=0x4 (i386)
struct IPaintableEntity
{
public:
	int (**_vptr$IPaintableEntity)(); // +0x0  // line 0
	virtual ~IPaintableEntity();  // line 17
	virtual PaintPowerType GetPaintPowerAtPoint( const Vector & ) const;  // line 18
	virtual void Paint( PaintPowerType, const Vector & );  // line 19
	virtual void CleansePaint();  // line 20
};

// game/shared/portal/paintable_entity.h:26 sizeof=0x420 (i386)
struct CPaintableEntity<CBaseProjectedEntity> : public CBaseProjectedEntity, public IPaintableEntity
{
public:
	virtual ~CPaintableEntity();  // line 36
	virtual PaintPowerType GetPaintPowerAtPoint( const Vector & ) const;  // line 45
	virtual void Paint( PaintPowerType, const Vector & );  // line 52
	virtual void CleansePaint();  // line 60
};

// game/shared/portal/paintable_entity.h:26 sizeof=0x125c (i386)
struct CPaintableEntity<CBaseMultiplayerPlayer> : public CBaseMultiplayerPlayer, public IPaintableEntity
{
public:
	virtual ~CPaintableEntity();  // line 36
	virtual PaintPowerType GetPaintPowerAtPoint( const Vector & ) const;  // line 45
	virtual void Paint( PaintPowerType, const Vector & );  // line 52
	virtual void CleansePaint();  // line 60
};

// game/shared/portal/paintable_entity.h:26 sizeof=0x76c (i386)
struct CPaintableEntity<CPhysicsProp> : public CPhysicsProp, public IPaintableEntity
{
public:
	virtual ~CPaintableEntity();  // line 36
	virtual PaintPowerType GetPaintPowerAtPoint( const Vector & ) const;  // line 45
	virtual void Paint( PaintPowerType, const Vector & );  // line 52
	virtual void CleansePaint();  // line 60
};

// game/shared/portal/paintable_entity.h:26 sizeof=0xfd8 (i386)
struct CPaintableEntity<CNPC_FloorTurret> : public CNPC_FloorTurret, public IPaintableEntity
{
public:
	virtual ~CPaintableEntity();  // line 36
	virtual PaintPowerType GetPaintPowerAtPoint( const Vector & ) const;  // line 45
	virtual void Paint( PaintPowerType, const Vector & );  // line 52
	virtual void CleansePaint();  // line 60
};

// game/shared/portal/paintable_entity.h:26 (declaration)
void CPaintableEntity();

// game/shared/portal/paintable_entity.h:36 @0x6db540 _ZN16CPaintableEntityI20CBaseProjectedEntityED1Ev
CPaintableEntity<CBaseProjectedEntity>::~CPaintableEntity()
{
	// inlined CBaseProjectedEntity::~CBaseProjectedEntity() at line 37
}

// game/shared/portal/paintable_entity.h:36 @0x6db670 _ZN16CPaintableEntityI20CBaseProjectedEntityED0Ev
CPaintableEntity<CBaseProjectedEntity>::~CPaintableEntity()
{
	// inlined CBaseProjectedEntity::~CBaseProjectedEntity() at line 37
}

// game/shared/portal/paintable_entity.h:36 @0x6f04a0 _ZN16CPaintableEntityI12CPhysicsPropED0Ev
CPaintableEntity<CPhysicsProp>::~CPaintableEntity()
{
}

// game/shared/portal/paintable_entity.h:36 @0x6f0530 _ZN16CPaintableEntityI12CPhysicsPropED1Ev
CPaintableEntity<CPhysicsProp>::~CPaintableEntity()
{
}

// game/shared/portal/paintable_entity.h:45 @0x6db4f0 _ZNK16CPaintableEntityI20CBaseProjectedEntityE20GetPaintPowerAtPointERK6Vector
PaintPowerType CPaintableEntity<CBaseProjectedEntity>::GetPaintPowerAtPoint( const Vector &worldContactPt )
{
	// inlined CBaseEntity::GetRenderColor() at line 47
}

// game/shared/portal/paintable_entity.h:45 @0x6f03d0 _ZNK16CPaintableEntityI12CPhysicsPropE20GetPaintPowerAtPointERK6Vector
PaintPowerType CPaintableEntity<CPhysicsProp>::GetPaintPowerAtPoint( const Vector &worldContactPt )
{
	// inlined CBaseEntity::GetRenderColor() at line 47
}

// game/shared/portal/paintable_entity.h:52 @0x6db6e0 _ZN16CPaintableEntityI20CBaseProjectedEntityE5PaintE14PaintPowerTypeRK6Vector
void CPaintableEntity<CBaseProjectedEntity>::Paint( PaintPowerType type, const Vector &worldContactPt )
{
	const Color color;  // line 54
	// inlined CBaseEntity::SetRenderColor() at line 55
}

// game/shared/portal/paintable_entity.h:52 @0x6f1500 _ZN16CPaintableEntityI12CPhysicsPropE5PaintE14PaintPowerTypeRK6Vector
void CPaintableEntity<CPhysicsProp>::Paint( PaintPowerType type, const Vector &worldContactPt )
{
	const Color color;  // line 54
	// inlined CBaseEntity::SetRenderColor() at line 55
}

// game/shared/portal/paintable_entity.h:60 @0x6db810 _ZN16CPaintableEntityI20CBaseProjectedEntityE12CleansePaintEv
void CPaintableEntity<CBaseProjectedEntity>::CleansePaint()
{
}

// game/shared/portal/paintable_entity.h:60 @0x6f1630 _ZN16CPaintableEntityI12CPhysicsPropE12CleansePaintEv
void CPaintableEntity<CPhysicsProp>::CleansePaint()
{
}
