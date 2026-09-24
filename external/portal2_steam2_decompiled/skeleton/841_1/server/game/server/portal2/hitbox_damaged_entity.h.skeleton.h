// DWARF declaration skeleton for game/server/portal2/hitbox_damaged_entity.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/hitbox_damaged_entity.h:17 sizeof=0x1108 (i386)
struct HitBoxDamagedEntity<PlayerPickupPaintPowerUser<CNPC_FloorTurret> > : public PlayerPickupPaintPowerUser<CNPC_FloorTurret>
{
protected:
	void ApplyDamageEffectShared( int, int );  // line 14
private:
	CUtlVector<int,CUtlMemory<int, int> > m_AppliedDamageEffects; // +0x10f0  // line 37
	DamagedEntityType m_DamagedEntityType; // +0x1104  // line 38
public:
	void HitBoxDamagedEntity( DamagedEntityType );  // line 42
	virtual ~HitBoxDamagedEntity();  // line 48
	virtual void Spawn();  // line 53
	virtual void TraceAttack( const CTakeDamageInfo &, const Vector &, trace_t * );  // line 62
	bool ApplyDamageEffect( int );  // line 72
	DamagedEntityType GetDamagedEntityType() const;  // line 110
protected:
	virtual void TakeDamage( const CTakeDamageInfo &, const Vector &, trace_t * );  // line 116
	virtual bool CanApplyDamageEffect( int ) const;  // line 159
};
