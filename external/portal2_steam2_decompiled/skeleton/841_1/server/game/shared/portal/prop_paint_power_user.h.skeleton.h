// DWARF declaration skeleton for game/shared/portal/prop_paint_power_user.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/shared/portal/prop_paint_power_user.h:36 sizeof=0x884 (i386)
struct PropPaintPowerUser<CPhysicsProp> : public PaintPowerUser<CPaintableEntity<CPhysicsProp> >
{
protected:
	int m_nOriginalMaterialIndex; // +0x874  // line 57
	int m_PrePaintedPower; // +0x878  // line 58
private:
	bool m_bHeldByPlayer; // +0x87c  // line 68
	float m_flPickedUpTime; // +0x880  // line 69
public:
	void PropPaintPowerUser();  // line 125
	virtual ~PropPaintPowerUser();  // line 134
	virtual void Spawn();  // line 140
	virtual void VPhysicsCollision( int, gamevcollisionevent_t * );  // line 162
	virtual void VPhysicsUpdate( IPhysicsObject * );  // line 188
	virtual void UpdatePaintPowersFromContacts();  // line 199
protected:
	virtual void ChooseActivePaintPowers( CUtlVector<PaintPowerInfo_t,CUtlMemory<PaintPowerInfo_t, int> > & );  // line 265
private:
	virtual PaintPowerState ActivateSpeedPower( PaintPowerInfo_t & );  // line 337
	virtual PaintPowerState UseSpeedPower( PaintPowerInfo_t & );  // line 350
	virtual PaintPowerState DeactivateSpeedPower( PaintPowerInfo_t & );  // line 357
	virtual PaintPowerState ActivateBouncePower( PaintPowerInfo_t & );  // line 374
	virtual PaintPowerState UseBouncePower( PaintPowerInfo_t & );  // line 442
	virtual PaintPowerState DeactivateBouncePower( PaintPowerInfo_t & );  // line 449
};

// game/shared/portal/prop_paint_power_user.h:36 sizeof=0x10f0 (i386)
struct PropPaintPowerUser<CNPC_FloorTurret> : public PaintPowerUser<CPaintableEntity<CNPC_FloorTurret> >
{
protected:
	int m_nOriginalMaterialIndex; // +0x10e0  // line 57
	int m_PrePaintedPower; // +0x10e4  // line 58
private:
	bool m_bHeldByPlayer; // +0x10e8  // line 68
	float m_flPickedUpTime; // +0x10ec  // line 69
public:
	void PropPaintPowerUser();  // line 125
	virtual ~PropPaintPowerUser();  // line 134
	virtual void Spawn();  // line 140
	virtual void VPhysicsCollision( int, gamevcollisionevent_t * );  // line 162
	virtual void VPhysicsUpdate( IPhysicsObject * );  // line 188
	virtual void UpdatePaintPowersFromContacts();  // line 199
protected:
	virtual void ChooseActivePaintPowers( CUtlVector<PaintPowerInfo_t,CUtlMemory<PaintPowerInfo_t, int> > & );  // line 265
private:
	virtual PaintPowerState ActivateSpeedPower( PaintPowerInfo_t & );  // line 337
	virtual PaintPowerState UseSpeedPower( PaintPowerInfo_t & );  // line 350
	virtual PaintPowerState DeactivateSpeedPower( PaintPowerInfo_t & );  // line 357
	virtual PaintPowerState ActivateBouncePower( PaintPowerInfo_t & );  // line 374
	virtual PaintPowerState UseBouncePower( PaintPowerInfo_t & );  // line 442
	virtual PaintPowerState DeactivateBouncePower( PaintPowerInfo_t & );  // line 449
};

// game/shared/portal/prop_paint_power_user.h:94 @0x767360 _ZN18PropPaintPowerUserI12CPhysicsPropE14GetDataDescMapEv
datamap_t *PropPaintPowerUser<CPhysicsProp>::GetDataDescMap()
{
}

// game/shared/portal/prop_paint_power_user.h:110 @0xb83a0 _ZN18PropPaintPowerUserI12CPhysicsPropE11DataMapInitEv
const datamap_t PropPaintPowerUser<CPhysicsProp>::DataMapInit()
{
	const datamap_t dataMap;  // line 119
	datamap_t dataMap;  // line 119
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 113
	CDatadescGeneratedNameHolder nameHolder;  // line 113
	typedescription_t dataDesc[1];  // line 114
}

// game/shared/portal/prop_paint_power_user.h:134 @0x76a5e0 _ZN18PropPaintPowerUserI12CPhysicsPropED0Ev
PropPaintPowerUser<CPhysicsProp>::~PropPaintPowerUser()
{
}

// game/shared/portal/prop_paint_power_user.h:134 @0x76a690 _ZN18PropPaintPowerUserI12CPhysicsPropED1Ev
PropPaintPowerUser<CPhysicsProp>::~PropPaintPowerUser()
{
}

// game/shared/portal/prop_paint_power_user.h:140 @0x768ac0 _ZN18PropPaintPowerUserI12CPhysicsPropE5SpawnEv
void PropPaintPowerUser<CPhysicsProp>::Spawn()
{
	IPhysicsObject *pPhysObject;  // line 153
	// inlined CBaseEntity::VPhysicsGetObject() at line 153
	{
		Color renderColor;  // line 148
		// inlined CBaseEntity::SetRenderColor() at line 149
	}
}

// game/shared/portal/prop_paint_power_user.h:162 @0x768980 _ZN18PropPaintPowerUserI12CPhysicsPropE17VPhysicsCollisionEiP21gamevcollisionevent_t
void PropPaintPowerUser<CPhysicsProp>::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
	{
		CBaseEntity *pOther;  // line 166
		PaintPowerInfo_t contact;  // line 168
		Vector vNormal;  // line 171
		Vector vPoint;  // line 171
		// inlined Vector::operator-() at line 176
		// inlined Vector::operator=() at line 176
		// inlined Vector::operator=() at line 177
		// inlined CBaseHandle::Set() at line 178
	}
}

// game/shared/portal/prop_paint_power_user.h:188 @0x767530 _ZN18PropPaintPowerUserI12CPhysicsPropE14VPhysicsUpdateEP14IPhysicsObject
void PropPaintPowerUser<CPhysicsProp>::VPhysicsUpdate( IPhysicsObject *pPhysics )
{
}

// game/shared/portal/prop_paint_power_user.h:199 @0x769ce0 _ZN18PropPaintPowerUserI12CPhysicsPropE29UpdatePaintPowersFromContactsEv
void PropPaintPowerUser<CPhysicsProp>::UpdatePaintPowersFromContacts()
{
	IPhysicsObject *pPhysObject;  // line 219
	// inlined PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ClearSurfacePaintPowerInfo() at line 258
	// inlined CBaseEntity::VPhysicsGetObject() at line 219
	{
		IPhysicsFrictionSnapshot *pSnapShot;  // line 222
		{
			PaintPowerInfo_t contact;  // line 225
			IPhysicsObject *pOther;  // line 227
			CBaseEntity *pOtherEntity;  // line 228
			{
				Vector vNormal;  // line 234
				Vector vPoint;  // line 234
				// inlined Vector::operator-() at line 239
				// inlined Vector::operator=() at line 239
				// inlined Vector::operator=() at line 240
				// inlined CBaseHandle::Set() at line 241
			}
		}
	}
}

// game/shared/portal/prop_paint_power_user.h:265 @0x767d90 _ZN18PropPaintPowerUserI12CPhysicsPropE23ChooseActivePaintPowersER10CUtlVectorI16PaintPowerInfo_t10CUtlMemoryIS3_iEE
void PropPaintPowerUser<CPhysicsProp>::ChooseActivePaintPowers( CUtlVector<PaintPowerInfo_t,CUtlMemory<PaintPowerInfo_t, int> > &activePowers )
{
	color24 color;  // line 270
	PaintPowerType paintedPower;  // line 271
	std::pair<const PaintPowerInfo_t*,const PaintPowerInfo_t*> powerRange;  // line 274
	size_t count;  // line 275
	PaintPowerInfo_t desiredPower;  // line 278
	const PaintPowerInfo_t *pHighestPriorityActivePower;  // line 282
	PaintPowerInfo_t currentPower;  // line 283
	// inlined CUtlVector<PaintPowerInfo_t,CUtlMemory<PaintPowerInfo_t, int> >::AddToTail() at line 333
	// inlined Vector::operator VectorByValue&() at line 283
	// inlined Vector::Vector() at line 283
	// inlined CBaseEntity::GetAbsOrigin() at line 283
	// inlined PaintPowerUser<CPaintableEntity<CPhysicsProp> >::FindHighestPriorityActivePaintPower() at line 282
	// inlined PaintPowerUser<CPaintableEntity<CPhysicsProp> >::GetSurfacePaintPowerInfo() at line 274
	// inlined Color::Color() at line 271
	// inlined CBaseEntity::GetRenderColor() at line 270
	// inlined PaintPowerUser<CPaintableEntity<CPhysicsProp> >::PrioritySortSurfacePaintPowerInfo() at line 288
	{
		PaintPowerConstIter i;  // line 294
		// inlined PaintPowerInfo_t::operator=() at line 298
	}
	{
		PaintPowerConstIter i;  // line 306
		{
			const PaintPowerInfo_t &powerInfo;  // line 308
			// inlined PaintPowerInfo_t::operator=() at line 315
			// inlined PaintPowerInfo_t::operator=() at line 327
			// inlined PaintPowerInfo_t::operator=() at line 321
		}
	}
	// inlined PaintPowerInfo_t::PaintPowerInfo_t() at line 283
}

// game/shared/portal/prop_paint_power_user.h:337 @0x7676d0 _ZN18PropPaintPowerUserI12CPhysicsPropE18ActivateSpeedPowerER16PaintPowerInfo_t
PaintPowerState PropPaintPowerUser<CPhysicsProp>::ActivateSpeedPower( PaintPowerInfo_t &powerInfo )
{
	IPhysicsObject *pPhysObject;  // line 339
	// inlined CBaseEntity::VPhysicsGetObject() at line 339
}

// game/shared/portal/prop_paint_power_user.h:350 @0x767430 _ZN18PropPaintPowerUserI12CPhysicsPropE13UseSpeedPowerER16PaintPowerInfo_t
PaintPowerState PropPaintPowerUser<CPhysicsProp>::UseSpeedPower( PaintPowerInfo_t &powerInfo )
{
}

// game/shared/portal/prop_paint_power_user.h:357 @0x7676a0 _ZN18PropPaintPowerUserI12CPhysicsPropE20DeactivateSpeedPowerER16PaintPowerInfo_t
PaintPowerState PropPaintPowerUser<CPhysicsProp>::DeactivateSpeedPower( PaintPowerInfo_t &powerInfo )
{
	IPhysicsObject *pPhysObject;  // line 359
	// inlined CBaseEntity::VPhysicsGetObject() at line 359
}

// game/shared/portal/prop_paint_power_user.h:374 @0x767730 _ZN18PropPaintPowerUserI12CPhysicsPropE19ActivateBouncePowerER16PaintPowerInfo_t
PaintPowerState PropPaintPowerUser<CPhysicsProp>::ActivateBouncePower( PaintPowerInfo_t &info )
{
	IPhysicsObject *pPhysObject;  // line 376
	{
		float flTrade;  // line 379
		const Vector vUp;  // line 380
		Vector vBounceVel;  // line 381
		Vector velocity;  // line 384
		AngularImpulse angularVel;  // line 385
		Vector velNorm;  // line 393
		float flNormDot;  // line 396
		float flBounceScale;  // line 398
		float fWallBounceScale;  // line 417
		// inlined Vector::operator+=() at line 431
		// inlined Vector::operator-=() at line 429
		// inlined Vector::operator*() at line 429
		// inlined Vector::Normalized() at line 429
		// inlined DotProduct() at line 429
		// inlined Vector::Normalized() at line 429
		// inlined Vector::operator+=() at line 425
		// inlined Vector::operator+=() at line 419
		// inlined Vector::operator*() at line 419
		// inlined Vector::operator*() at line 419
		// inlined ConVar::GetFloat() at line 419
		// inlined Vector::operator+=() at line 404
		// inlined Vector::operator*() at line 404
		// inlined ConVar::GetFloat() at line 404
		// inlined DotProduct() at line 396
		// inlined Vector::NormalizeInPlace() at line 394
		// inlined Vector::operator-=() at line 390
		// inlined Vector::operator*() at line 390
		// inlined DotProduct() at line 390
		// inlined Vector::operator-=() at line 387
		// inlined Vector::operator*() at line 387
		// inlined DotProduct() at line 387
		// inlined Vector::Vector() at line 380
		// inlined ConVar::GetFloat() at line 379
		// inlined Vector::Vector() at line 425
		{
			Vector vOut;  // line 409
			// inlined Vector::operator*() at line 409
			// inlined Vector::operator-() at line 409
			// inlined Vector::Normalized() at line 409
			// inlined Vector::operator VectorByValue&() at line 409
			// inlined DotProduct() at line 412
			// inlined ConVar::GetFloat() at line 413
			// inlined Vector::operator*() at line 413
			// inlined Vector::operator+=() at line 413
		}
	}
	// inlined CBaseEntity::VPhysicsGetObject() at line 376
}

// game/shared/portal/prop_paint_power_user.h:442 @0x767440 _ZN18PropPaintPowerUserI12CPhysicsPropE14UseBouncePowerER16PaintPowerInfo_t
PaintPowerState PropPaintPowerUser<CPhysicsProp>::UseBouncePower( PaintPowerInfo_t &powerInfo )
{
}

// game/shared/portal/prop_paint_power_user.h:449 @0x767450 _ZN18PropPaintPowerUserI12CPhysicsPropE21DeactivateBouncePowerER16PaintPowerInfo_t
PaintPowerState PropPaintPowerUser<CPhysicsProp>::DeactivateBouncePower( PaintPowerInfo_t &powerInfo )
{
}
