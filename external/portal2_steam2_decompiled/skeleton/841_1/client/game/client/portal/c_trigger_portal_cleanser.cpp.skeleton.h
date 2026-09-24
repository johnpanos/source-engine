// DWARF declaration skeleton for game/client/portal/c_trigger_portal_cleanser.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x6dcd0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined ClientClass::ClientClass() at line 112
}

// game/client/portal/c_trigger_portal_cleanser.cpp:14 sizeof=0xa60 (i386)
struct C_TriggerPortalCleanser : public C_BaseTrigger
{
public:
	bool IsEnabled() const;  // line 19
private:
	bool m_bDisabled; // +0xa5e  // line 22
public:
	virtual void Touch( C_BaseEntity * );  // line 25
	virtual void UpdatePartitionListEntry();  // line 103
};

// game/client/portal/c_trigger_portal_cleanser.cpp:14 (declaration)
~C_TriggerPortalCleanser();

// game/client/portal/c_trigger_portal_cleanser.cpp:14 @0x4f4860 _ZN23C_TriggerPortalCleanserD1Ev
C_TriggerPortalCleanser::~C_TriggerPortalCleanser()
{
	// inlined C_BaseTrigger::~C_BaseTrigger() at line 14
}

// game/client/portal/c_trigger_portal_cleanser.cpp:14 (declaration)
void C_TriggerPortalCleanser();

// game/client/portal/c_trigger_portal_cleanser.cpp:14 @0x4f4940 _ZN23C_TriggerPortalCleanserD0Ev
C_TriggerPortalCleanser::~C_TriggerPortalCleanser()
{
	// inlined C_BaseTrigger::~C_BaseTrigger() at line 14
}

// game/client/portal/c_trigger_portal_cleanser.cpp:16 @0x4f4460 _ZN23C_TriggerPortalCleanser40YouForgotToImplementOrDeclareClientClassEv
int C_TriggerPortalCleanser::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/portal/c_trigger_portal_cleanser.cpp:16 @0x4f4480 _ZN23C_TriggerPortalCleanser14GetClientClassEv
ClientClass *C_TriggerPortalCleanser::GetClientClass()
{
}

// game/client/portal/c_trigger_portal_cleanser.cpp:25 @0x4f4590 _ZN23C_TriggerPortalCleanser5TouchEP12C_BaseEntity
void C_TriggerPortalCleanser::Touch( C_BaseEntity *pOther )
{
	bool bIsPortalGun;  // line 30
	{
		C_Portal_Player *pPlayer;  // line 38
		// inlined ToPortalPlayer() at line 38
		{
			CWeaponPortalgun *pPortalgun;  // line 47
			{
				bool bPortal1Active;  // line 60
				bool bPortal2Active;  // line 61
				bool bFizzledPortal;  // line 64
				{
					CProp_Portal *pPortal;  // line 68
				}
				// inlined C_WeaponPortalgun::SetLastFiredPortal() at line 94
				{
					CProp_Portal *pPortal;  // line 80
				}
			}
		}
	}
	// inlined FClassnameIs() at line 31
}

// game/client/portal/c_trigger_portal_cleanser.cpp:103 @0x4f4540 _ZN23C_TriggerPortalCleanser24UpdatePartitionListEntryEv
void C_TriggerPortalCleanser::UpdatePartitionListEntry()
{
}

// game/client/portal/c_trigger_portal_cleanser.cpp:112 @0x4f4490 _ZL37_C_TriggerPortalCleanser_CreateObjectii
IClientNetworkable *_C_TriggerPortalCleanser_CreateObject( int entnum, int serialNum )
{
	C_TriggerPortalCleanser *pRet;  // line 112
	// inlined C_TriggerPortalCleanser::C_TriggerPortalCleanser() at line 112
}

// game/client/portal/c_trigger_portal_cleanser.cpp:112 @0x6db80 _Z15ClientClassInitIN24DT_TriggerPortalCleanser7ignoredEEiPT_
int ClientClassInit<DT_TriggerPortalCleanser::ignored>( DT_TriggerPortalCleanser::ignored * )
{
	char *pRecvTableName;  // line 112
	RecvTable &RecvTable;  // line 112
	RecvProp RecvProps[3];  // line 112
}

// game/client/portal/c_trigger_portal_cleanser.cpp:112
RecvTable g_RecvTable;

// game/client/portal/c_trigger_portal_cleanser.cpp:112
int g_RecvTableInit;

// game/client/portal/c_trigger_portal_cleanser.cpp:112
ClientClass __g_C_TriggerPortalCleanserClientClass;
