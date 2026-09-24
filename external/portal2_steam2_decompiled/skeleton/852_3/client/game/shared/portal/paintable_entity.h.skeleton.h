// DWARF declaration skeleton for game/shared/portal/paintable_entity.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
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

// game/shared/portal/paintable_entity.h:26 sizeof=0x1a58 (i386)
struct CPaintableEntity<C_BasePlayer> : public C_BasePlayer, public IPaintableEntity
{
public:
	virtual ~CPaintableEntity();  // line 36
	virtual PaintPowerType GetPaintPowerAtPoint( const Vector & ) const;  // line 45
	virtual void Paint( PaintPowerType, const Vector & );  // line 52
	virtual void CleansePaint();  // line 60
};

// game/shared/portal/paintable_entity.h:26 sizeof=0xabc (i386)
struct CPaintableEntity<C_BaseProjectedEntity> : public C_BaseProjectedEntity, public IPaintableEntity
{
public:
	virtual ~CPaintableEntity();  // line 36
	virtual PaintPowerType GetPaintPowerAtPoint( const Vector & ) const;  // line 45
	virtual void Paint( PaintPowerType, const Vector & );  // line 52
	virtual void CleansePaint();  // line 60
};

// game/shared/portal/paintable_entity.h:26 (declaration)
void CPaintableEntity();

// game/shared/portal/paintable_entity.h:36 @0x535a00 _ZN16CPaintableEntityI21C_BaseProjectedEntityED0Ev
CPaintableEntity<C_BaseProjectedEntity>::~CPaintableEntity()
{
}

// game/shared/portal/paintable_entity.h:36 @0x535a90 _ZN16CPaintableEntityI21C_BaseProjectedEntityED1Ev
CPaintableEntity<C_BaseProjectedEntity>::~CPaintableEntity()
{
}

// game/shared/portal/paintable_entity.h:45 @0x535b60 _ZNK16CPaintableEntityI21C_BaseProjectedEntityE20GetPaintPowerAtPointERK6Vector
PaintPowerType CPaintableEntity<C_BaseProjectedEntity>::GetPaintPowerAtPoint( const Vector &worldContactPt )
{
	// inlined C_BaseEntity::GetRenderColor() at line 47
}

// game/shared/portal/paintable_entity.h:52 @0x535b00 _ZN16CPaintableEntityI21C_BaseProjectedEntityE5PaintE14PaintPowerTypeRK6Vector
void CPaintableEntity<C_BaseProjectedEntity>::Paint( PaintPowerType type, const Vector &worldContactPt )
{
	const Color color;  // line 54
	// inlined C_BaseEntity::SetRenderColor() at line 55
}

// game/shared/portal/paintable_entity.h:60 @0x535f40 _ZN16CPaintableEntityI21C_BaseProjectedEntityE12CleansePaintEv
void CPaintableEntity<C_BaseProjectedEntity>::CleansePaint()
{
	// inlined CPaintableEntity<C_BaseProjectedEntity>::Paint() at line 62
}
