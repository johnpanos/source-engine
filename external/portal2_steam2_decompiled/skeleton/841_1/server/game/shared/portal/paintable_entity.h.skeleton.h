// DWARF declaration skeleton for game/shared/portal/paintable_entity.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
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

// game/shared/portal/paintable_entity.h:26 sizeof=0x424 (i386)
struct CPaintableEntity<CBaseProjectedEntity> : public CBaseProjectedEntity, public IPaintableEntity
{
public:
	virtual ~CPaintableEntity();  // line 36
	virtual PaintPowerType GetPaintPowerAtPoint( const Vector & ) const;  // line 45
	virtual void Paint( PaintPowerType, const Vector & );  // line 52
	virtual void CleansePaint();  // line 60
};

// game/shared/portal/paintable_entity.h:26 sizeof=0x1260 (i386)
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

// game/shared/portal/paintable_entity.h:36 @0x74cee0 _ZN16CPaintableEntityI20CBaseProjectedEntityED0Ev
CPaintableEntity<CBaseProjectedEntity>::~CPaintableEntity()
{
	// inlined CBaseProjectedEntity::~CBaseProjectedEntity() at line 37
}

// game/shared/portal/paintable_entity.h:36 @0x74cf50 _ZN16CPaintableEntityI20CBaseProjectedEntityED1Ev
CPaintableEntity<CBaseProjectedEntity>::~CPaintableEntity()
{
	// inlined CBaseProjectedEntity::~CBaseProjectedEntity() at line 37
}

// game/shared/portal/paintable_entity.h:36 @0x7675a0 _ZN16CPaintableEntityI12CPhysicsPropED0Ev
CPaintableEntity<CPhysicsProp>::~CPaintableEntity()
{
}

// game/shared/portal/paintable_entity.h:36 @0x767630 _ZN16CPaintableEntityI12CPhysicsPropED1Ev
CPaintableEntity<CPhysicsProp>::~CPaintableEntity()
{
}

// game/shared/portal/paintable_entity.h:45 @0x74cdb0 _ZNK16CPaintableEntityI20CBaseProjectedEntityE20GetPaintPowerAtPointERK6Vector
PaintPowerType CPaintableEntity<CBaseProjectedEntity>::GetPaintPowerAtPoint( const Vector &worldContactPt )
{
	// inlined CBaseEntity::GetRenderColor() at line 47
}

// game/shared/portal/paintable_entity.h:45 @0x7674d0 _ZNK16CPaintableEntityI12CPhysicsPropE20GetPaintPowerAtPointERK6Vector
PaintPowerType CPaintableEntity<CPhysicsProp>::GetPaintPowerAtPoint( const Vector &worldContactPt )
{
	// inlined CBaseEntity::GetRenderColor() at line 47
}

// game/shared/portal/paintable_entity.h:52 @0x74cff0 _ZN16CPaintableEntityI20CBaseProjectedEntityE5PaintE14PaintPowerTypeRK6Vector
void CPaintableEntity<CBaseProjectedEntity>::Paint( PaintPowerType type, const Vector &worldContactPt )
{
	const Color color;  // line 54
	// inlined CBaseEntity::SetRenderColor() at line 55
}

// game/shared/portal/paintable_entity.h:52 @0x769060 _ZN16CPaintableEntityI12CPhysicsPropE5PaintE14PaintPowerTypeRK6Vector
void CPaintableEntity<CPhysicsProp>::Paint( PaintPowerType type, const Vector &worldContactPt )
{
	const Color color;  // line 54
	// inlined CBaseEntity::SetRenderColor() at line 55
}

// game/shared/portal/paintable_entity.h:60 @0x74d530 _ZN16CPaintableEntityI20CBaseProjectedEntityE12CleansePaintEv
void CPaintableEntity<CBaseProjectedEntity>::CleansePaint()
{
}

// game/shared/portal/paintable_entity.h:60 @0x7695a0 _ZN16CPaintableEntityI12CPhysicsPropE12CleansePaintEv
void CPaintableEntity<CPhysicsProp>::CleansePaint()
{
}
