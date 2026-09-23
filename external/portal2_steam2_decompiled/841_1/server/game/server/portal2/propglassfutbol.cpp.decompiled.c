/* DWARF-guided pseudocode for game/server/portal2/propglassfutbol.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000b3310 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  undefined4 uVar1;
  IEntityFactoryDictionary *pIVar2;
  datamap_t *pdVar3;
  int unaff_EBX;
  longlong lVar4;
  ConVar *in_stack_ffffffe4;
  char *in_stack_ffffffe8;
  char *in_stack_ffffffec;
  int flags_1;
  
  flags_1 = unaff_EBX;
  lVar4 = ___i686_get_pc_thunk_bx();
  if (lVar4 == 0xffff00000001) {
    *(undefined1 *)((int)&ai_sequence_debug.m_fnChangeCallbacks.m_pElements + unaff_EBX) = 0;
    *(undefined1 *)((int)&ai_sequence_debug.m_fnChangeCallbacks.m_pElements + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&ai_sequence_debug.m_fnChangeCallbacks.m_pElements + unaff_EBX + 2) = 0;
    *(undefined1 *)((int)&ai_sequence_debug.m_fnChangeCallbacks.m_pElements + unaff_EBX + 3) = 0;
    *(undefined4 *)((int)&CBaseAnimating_DataDescInit::g_DataMapHolder + unaff_EBX) = 0;
    *(undefined4 *)((int)&DT_ServerAnimationData::g_SendTable.m_pProps + unaff_EBX) = 0;
    *(undefined4 *)((int)&DT_ServerAnimationData::g_SendTable.m_nProps + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&DT_ServerAnimationData::g_SendTable.m_pNetTableName + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&DT_ServerAnimationData::g_SendTable.m_pPrecalc + unaff_EBX) = 0;
    *(undefined4 *)(&DT_ServerAnimationData::g_SendTable.field_0x10 + unaff_EBX) = 0;
    *(undefined4 *)((int)&DT_ServerAnimationData::g_SendTableInit + unaff_EBX) = 0;
    *(undefined4 *)((int)&DT_BaseAnimating::g_SendTable.m_pProps + unaff_EBX) = 0;
    *(undefined4 *)((int)&DT_BaseAnimating::g_SendTable.m_nProps + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&DT_BaseAnimating::g_SendTable.m_pNetTableName + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&DT_BaseAnimating::g_SendTable.m_pPrecalc + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DT_BaseAnimating::g_SendTable.field_0x10 + unaff_EBX) = 0x7f7fffff;
    *(undefined **)((int)&DT_BaseAnimating::g_SendTableInit + unaff_EBX) = &UNK_00b98fe4 + unaff_EBX
    ;
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<CAI_AssaultGoal>::dataDesc[1].flatOffset + unaff_EBX + 8),
                   (char *)(unaff_EBX + 0xa002ba),(char *)(unaff_EBX + 0x9ab492),0,in_stack_ffffffe4
                   ,in_stack_ffffffe8,in_stack_ffffffec,flags_1);
    uVar1 = *(undefined4 *)(&DAT_00b93830 + unaff_EBX);
    ___cxa_atexit(unaff_EBX + 0x98bbec,0,uVar1);
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<CAI_AssaultGoal>::dataDesc[3].flatOffset + unaff_EBX + -0x18),
                   (char *)(unaff_EBX + 0xa002d0),(char *)(unaff_EBX + 0x9a3958),0,in_stack_ffffffe4
                   ,in_stack_ffffffe8,in_stack_ffffffec,flags_1);
    ___cxa_atexit(unaff_EBX + 0x98bbcc,0,uVar1);
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<CAI_AssaultGoal>::dataDesc[4].flatOffset + unaff_EBX + 8),
                   (char *)(unaff_EBX + 0xa002f1),(char *)(unaff_EBX + 0x9ebeda),0,in_stack_ffffffe4
                   ,in_stack_ffffffe8,in_stack_ffffffec,flags_1);
    ___cxa_atexit(&UNK_0098bbac + unaff_EBX,0,uVar1);
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<CAI_AssaultBehavior>::dataDesc[0].flatOffset +
                   unaff_EBX + -0x18),(char *)(unaff_EBX + 0xa00310),(char *)(unaff_EBX + 0x9a3958),
                   0,in_stack_ffffffe4,in_stack_ffffffe8,in_stack_ffffffec,flags_1);
    ___cxa_atexit(unaff_EBX + 0x98bb8c,0,uVar1);
    *(undefined **)(&DAT_00d9c8cc + unaff_EBX) = &UNK_00c1ec14 + unaff_EBX;
    pIVar2 = EntityFactoryDictionary();
    (**pIVar2->_vptr_IEntityFactoryDictionary)
              (pIVar2,&DAT_00d9c8cc + unaff_EBX,unaff_EBX + 0xa00339);
    pdVar3 = DataMapInit<CFutbolCatcher>((CFutbolCatcher *)0x0);
    *(datamap_t **)((int)DataMapInit<CAI_AssaultBehavior>::dataDesc[1].flatOffset + unaff_EBX) =
         pdVar3;
    *(undefined **)(&DAT_00d9c8d0 + unaff_EBX) = &UNK_00c1ec34 + unaff_EBX;
    pIVar2 = EntityFactoryDictionary();
    (**pIVar2->_vptr_IEntityFactoryDictionary)
              (pIVar2,&DAT_00d9c8d0 + unaff_EBX,unaff_EBX + 0xa00348);
    pdVar3 = DataMapInit<CPropFutbolSpawner>((CPropFutbolSpawner *)0x0);
    *(datamap_t **)((int)DataMapInit<CAI_AssaultBehavior>::dataDesc[1].flatOffset + unaff_EBX + 4) =
         pdVar3;
    *(undefined **)(&DAT_00d9c8d4 + unaff_EBX) = &UNK_00c1ec54 + unaff_EBX;
    pIVar2 = EntityFactoryDictionary();
    (**pIVar2->_vptr_IEntityFactoryDictionary)
              (pIVar2,&DAT_00d9c8d4 + unaff_EBX,&UNK_009fdf8e + unaff_EBX);
    pdVar3 = DataMapInit<CPropGlassFutbol>((CPropGlassFutbol *)0x0);
    *(datamap_t **)((int)DataMapInit<CAI_AssaultBehavior>::dataDesc[1].flatOffset + unaff_EBX + 8) =
         pdVar3;
    *(undefined **)(&DAT_00d9c8d8 + unaff_EBX) = &UNK_00c1ec74 + unaff_EBX;
    pIVar2 = EntityFactoryDictionary();
    (**pIVar2->_vptr_IEntityFactoryDictionary)
              (pIVar2,&DAT_00d9c8d8 + unaff_EBX,unaff_EBX + 0xa00362);
  }
  return;
}


/* __tcf_3 at 00a3ef10 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38a748),in_stack_00000008);
  return;
}


/* __tcf_4 at 00a3eef0 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38a7c8),in_stack_00000008);
  return;
}


/* __tcf_5 at 00a3eed0 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38a848),in_stack_00000008);
  return;
}


/* __tcf_6 at 00a3eeb0 */

void __tcf_6(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38a8c8),in_stack_00000008);
  return;
}


/* CFutbolCatcher::CatchThink at 0074d910 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CatchThink(CFutbolCatcher * this) */

void __thiscall CFutbolCatcher::CatchThink(CFutbolCatcher *this)

{
  int iVar1;
  int unaff_EBX;
  Vector local_d0;
  Vector local_c4 [5];
  int local_84;
  float local_7c;
  float local_78;
  float local_74;
  float local_6c;
  float local_68;
  float local_64;
  float local_5c;
  float local_58;
  float local_54;
  float local_4c;
  float local_48;
  float local_44;
  undefined4 local_3c;
  undefined1 local_38;
  undefined1 local_37;
  int local_20 [4];
  
                    /* Unresolved local var: Ray_t ray@[???]
                       Unresolved local var: CFilterOnlyGlassFutbol filter@[???]
                       Unresolved local var: trace_t tr@[???] */
  ___i686_get_pc_thunk_bx();
  local_7c = (this->m_vecCatchBoxOrig).x;
  local_6c = local_7c - local_7c;
  local_78 = (this->m_vecCatchBoxOrig).y;
  local_68 = local_78 - local_78;
  local_74 = (this->m_vecCatchBoxOrig).z;
  local_64 = local_74 - local_74;
  local_3c = 0;
  local_37 = local_6c * local_6c + local_68 * local_68 + local_64 * local_64 !=
             *(float *)(unaff_EBX + 0x37f24f);
  local_54 = *(float *)(unaff_EBX + 0x37f1df);
  local_4c = ((this->m_vecCatchBoxMaxs).x - (this->m_vecCatchBoxMins).x) * local_54;
  local_48 = ((this->m_vecCatchBoxMaxs).y - (this->m_vecCatchBoxMins).y) * local_54;
  local_44 = ((this->m_vecCatchBoxMaxs).z - (this->m_vecCatchBoxMins).z) * local_54;
  local_38 = (double)(local_4c * local_4c + local_48 * local_48 + local_44 * local_44) <
             *(double *)(unaff_EBX + 0x37f3cf);
  local_5c = ((this->m_vecCatchBoxMins).x + (this->m_vecCatchBoxMaxs).x) * local_54;
  local_58 = ((this->m_vecCatchBoxMins).y + (this->m_vecCatchBoxMaxs).y) * local_54;
  local_54 = ((this->m_vecCatchBoxMins).z + (this->m_vecCatchBoxMaxs).z) * local_54;
  local_7c = local_7c + local_5c;
  local_78 = local_78 + local_58;
  local_74 = local_74 + local_54;
  local_5c = -local_5c;
  local_58 = -local_58;
  local_54 = -local_54;
  local_20[0] = unaff_EBX + 0x584697;
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4f92af) + 0x14))
            ((int *)**(undefined4 **)(unaff_EBX + 0x4f92af),&local_7c,0x200400b,local_20,&local_d0);
  if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x4f92ab) + 0x1c) + 0x30) != 0) {
    DebugDrawLine(&local_d0,local_c4,0xff,0,0,true,-1.0);
  }
                    /* Unresolved local var: CPropGlassFutbol * pFutbol@[???] */
  if ((local_84 == 0) ||
     ((((local_84 != **(int **)(unaff_EBX + 0x4f92ff) &&
        (this->m_bDisableRecaptureOnPlayerGrab == false)) &&
       (iVar1 = ___dynamic_cast(local_84,*(undefined4 *)(unaff_EBX + 0x4f926f),unaff_EBX + 0x5845e3,
                                0), iVar1 != 0)) &&
      (((*(int *)(iVar1 + 0x770) == 0 || (*(int *)(iVar1 + 0x770) == 1)) &&
       ((*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
          super_IHandleEntity._vptr_IHandleEntity[0xeb])(this,iVar1), local_84 == 0)))))) {
    this->m_bDisableRecaptureOnPlayerGrab = false;
  }
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(**(int **)(unaff_EBX + 0x4f9243) + 0xc) +
             *(float *)(**(int **)(unaff_EBX + 0x4f9243) + 0x1c),(char *)0x0);
  return;
}


/* CFilterOnlyGlassFutbol::ShouldHitEntity at 0074ed70 */

/* DWARF original prototype: bool ShouldHitEntity(CFilterOnlyGlassFutbol * this, IHandleEntity *
   pServerEntity, int contentsMask) */

bool __thiscall
CFilterOnlyGlassFutbol::ShouldHitEntity
          (CFilterOnlyGlassFutbol *this,IHandleEntity *pServerEntity,int contentsMask)

{
  char cVar1;
  bool bVar2;
  CBaseEntity *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CBaseEntity * pEntity@[???] */
  ___i686_get_pc_thunk_bx();
  cVar1 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4f7e57) + 8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x4f7e57),pServerEntity);
  if ((cVar1 == '\0') &&
     (this_00 = (CBaseEntity *)(*pServerEntity->_vptr_IHandleEntity[6])(pServerEntity),
     this_00 != (CBaseEntity *)0x0)) {
    if ((this_00->m_iClassname).pszValue == (char *)(unaff_EBX + 0x362535)) {
      return true;
    }
    bVar2 = CBaseEntity::ClassMatchesComplex(this_00,(char *)(unaff_EBX + 0x362535));
    return bVar2;
  }
  return false;
}


/* CFilterOnlyGlassFutbol::GetTraceType at 0074eb30 */

/* DWARF original prototype: TraceType_t GetTraceType(CFilterOnlyGlassFutbol * this) */

TraceType_t __thiscall CFilterOnlyGlassFutbol::GetTraceType(CFilterOnlyGlassFutbol *this)

{
  return TRACE_ENTITIES_ONLY;
}


/* CFutbolCatcher::~CFutbolCatcher at 0074edf0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CFutbolCatcher(CFutbolCatcher * this, int __in_chrg) */

void __thiscall CFutbolCatcher::~CFutbolCatcher(CFutbolCatcher *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x582d47);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolCaught).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolReleased).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CFutbolCatcher::~CFutbolCatcher at 0074ee70 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CFutbolCatcher(CFutbolCatcher * this, int __in_chrg) */

void __thiscall CFutbolCatcher::~CFutbolCatcher(CFutbolCatcher *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x582cc7);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolCaught).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolReleased).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
  return;
}


/* CPropFutbolSocket::~CPropFutbolSocket at 0074f040 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropFutbolSocket(CPropFutbolSocket * this, int __in_chrg) */

void __thiscall CPropFutbolSocket::~CPropFutbolSocket(CPropFutbolSocket *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x582af7);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CFutbolCatcher).m_OnFutbolCaught.super_CBaseEntityOutput,
             in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CFutbolCatcher).m_OnFutbolReleased.super_CBaseEntityOutput,
             in_stack_ffffffe8);
  CBaseAnimating::~CBaseAnimating((CBaseAnimating *)this,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CPropFutbolSocket::~CPropFutbolSocket at 0074f0c0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropFutbolSocket(CPropFutbolSocket * this, int __in_chrg) */

void __thiscall CPropFutbolSocket::~CPropFutbolSocket(CPropFutbolSocket *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x582a77);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CFutbolCatcher).m_OnFutbolCaught.super_CBaseEntityOutput,
             in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CFutbolCatcher).m_OnFutbolReleased.super_CBaseEntityOutput,
             in_stack_ffffffe8);
  CBaseAnimating::~CBaseAnimating((CBaseAnimating *)this,__in_chrg);
  return;
}


/* CPropFutbolSpawner::~CPropFutbolSpawner at 0074f130 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropFutbolSpawner(CPropFutbolSpawner * this, int __in_chrg) */

void __thiscall CPropFutbolSpawner::~CPropFutbolSpawner(CPropFutbolSpawner *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(&UNK_00582624 + unaff_EBX);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolGrabbed).super_CBaseEntityOutput,in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolSpawned).super_CBaseEntityOutput,in_stack_ffffffd8);
  (this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x582a04);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CFutbolCatcher).m_OnFutbolCaught.super_CBaseEntityOutput,
             in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CFutbolCatcher).m_OnFutbolReleased.super_CBaseEntityOutput,
             in_stack_ffffffd8);
  CBaseAnimating::~CBaseAnimating((CBaseAnimating *)this,in_stack_ffffffd8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CPropFutbolSpawner::~CPropFutbolSpawner at 0074f220 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropFutbolSpawner(CPropFutbolSpawner * this, int __in_chrg) */

void __thiscall CPropFutbolSpawner::~CPropFutbolSpawner(CPropFutbolSpawner *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(&UNK_00582534 + unaff_EBX);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolGrabbed).super_CBaseEntityOutput,in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolSpawned).super_CBaseEntityOutput,in_stack_ffffffd8);
  (this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x582914);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CFutbolCatcher).m_OnFutbolCaught.super_CBaseEntityOutput,
             in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CFutbolCatcher).m_OnFutbolReleased.super_CBaseEntityOutput,
             in_stack_ffffffd8);
  CBaseAnimating::~CBaseAnimating((CBaseAnimating *)this,__in_chrg);
  return;
}


/* DataMapInit<CFutbolCatcher> at 000b3010 */

datamap_t * DataMapInit<CFutbolCatcher>(CFutbolCatcher *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  char *pcVar4;
  size_t sVar5;
  int iVar6;
  undefined4 *puVar7;
  int iVar8;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((*(char *)((int)&phys_stressbodyweights.m_pszDefaultValue + unaff_EBX + 2) == '\0') &&
     (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xd9cc42), iVar3 != 0)) {
    *(int *)((int)&phys_stressbodyweights.m_Value.m_nValue + unaff_EBX + 2) = unaff_EBX + 0xa005b1;
    *(undefined4 *)((int)&phys_stressbodyweights.m_fMinVal + unaff_EBX + 2) = 0;
    *(undefined4 *)(&phys_stressbodyweights.field_0x3e + unaff_EBX) = 0;
    *(undefined4 *)((int)&phys_stressbodyweights.m_fMaxVal + unaff_EBX + 2) = 0;
    *(undefined4 *)
     ((int)&phys_stressbodyweights.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 2) = 0;
    *(undefined4 *)
     ((int)&phys_stressbodyweights.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX + 2)
         = 0;
    *(undefined4 *)(&phys_stressbodyweights.field_0x36 + unaff_EBX) = 0xe;
    ___cxa_guard_release(unaff_EBX + 0xd9cc42);
    ___cxa_atexit(unaff_EBX + 0x98bf12,0,*(undefined4 *)(&DAT_00b93b36 + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_VPhysicsTakeDamage_00cb4a14 + unaff_EBX + 2) =
       *(undefined4 *)(&DAT_00b93b42 + unaff_EBX);
  if ((*(char *)((int)&phys_stressbodyweights.m_Value.m_StringLength + unaff_EBX + 2) == '\0') &&
     (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xd9cc4a), iVar3 != 0)) {
    uVar1 = **(undefined4 **)(&DAT_00b93b56 + unaff_EBX);
    *(undefined4 *)((int)&PTR_Destroy_00cb4f34 + unaff_EBX + 2) = uVar1;
    *(undefined4 *)((int)&PTR_GetLocalOrigin_00cb4f74 + unaff_EBX + 2) = uVar1;
    pcVar4 = operator_new___(*(int *)(&phys_stressbodyweights.field_0x36 + unaff_EBX) + 0xb);
    _strcpy(pcVar4,*(char **)((int)&phys_stressbodyweights.m_Value.m_nValue + unaff_EBX + 2));
    sVar5 = _strlen(pcVar4);
    builtin_strncpy(pcVar4 + sVar5,"CatchThink",0xb);
    iVar2 = *(int *)((int)&phys_stressbodyweights.m_fnChangeCallbacks.m_Memory.m_pMemory +
                    unaff_EBX + 2);
    iVar3 = iVar2 + 1;
    if (*(int *)(&phys_stressbodyweights.field_0x3e + unaff_EBX) < iVar3) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)((int)&phys_stressbodyweights.m_fMinVal + unaff_EBX + 2),
                 iVar3 - *(int *)(&phys_stressbodyweights.field_0x3e + unaff_EBX));
    }
    iVar6 = *(int *)((int)&phys_stressbodyweights.m_fnChangeCallbacks.m_Memory.m_pMemory +
                    unaff_EBX + 2) + 1;
    *(int *)((int)&phys_stressbodyweights.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 2) =
         iVar6;
    iVar8 = *(int *)((int)&phys_stressbodyweights.m_fMinVal + unaff_EBX + 2);
    *(int *)((int)&phys_stressbodyweights.m_fnChangeCallbacks.m_Memory.m_nAllocationCount +
            unaff_EBX + 2) = iVar8;
    iVar6 = (iVar6 - iVar2) + -1;
    if (0 < iVar6) {
      _V_memmove((void *)(iVar8 + iVar3 * 4),(void *)(iVar8 + iVar2 * 4),iVar6 * 4);
      iVar8 = *(int *)((int)&phys_stressbodyweights.m_fMinVal + unaff_EBX + 2);
    }
    puVar7 = (undefined4 *)(iVar2 * 4 + iVar8);
    if (puVar7 != (undefined4 *)0x0) {
      *puVar7 = pcVar4;
    }
    *(char **)((int)&PTR_ChangeTeam_00cb4fa4 + unaff_EBX + 2) = pcVar4;
    pcVar4 = operator_new___(*(int *)(&phys_stressbodyweights.field_0x36 + unaff_EBX) + 0xd);
    _strcpy(pcVar4,*(char **)((int)&phys_stressbodyweights.m_Value.m_nValue + unaff_EBX + 2));
    sVar5 = _strlen(pcVar4);
    builtin_strncpy(pcVar4 + sVar5,"CaptureThink",0xd);
    iVar2 = *(int *)((int)&phys_stressbodyweights.m_fnChangeCallbacks.m_Memory.m_pMemory +
                    unaff_EBX + 2);
    iVar3 = iVar2 + 1;
    if (*(int *)(&phys_stressbodyweights.field_0x3e + unaff_EBX) < iVar3) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)((int)&phys_stressbodyweights.m_fMinVal + unaff_EBX + 2),
                 iVar3 - *(int *)(&phys_stressbodyweights.field_0x3e + unaff_EBX));
    }
    iVar6 = *(int *)((int)&phys_stressbodyweights.m_fnChangeCallbacks.m_Memory.m_pMemory +
                    unaff_EBX + 2) + 1;
    *(int *)((int)&phys_stressbodyweights.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 2) =
         iVar6;
    iVar8 = *(int *)((int)&phys_stressbodyweights.m_fMinVal + unaff_EBX + 2);
    *(int *)((int)&phys_stressbodyweights.m_fnChangeCallbacks.m_Memory.m_nAllocationCount +
            unaff_EBX + 2) = iVar8;
    iVar6 = (iVar6 - iVar2) + -1;
    if (0 < iVar6) {
      _V_memmove((void *)(iVar8 + iVar3 * 4),(void *)(iVar8 + iVar2 * 4),iVar6 * 4);
      iVar8 = *(int *)((int)&phys_stressbodyweights.m_fMinVal + unaff_EBX + 2);
    }
    puVar7 = (undefined4 *)(iVar2 * 4 + iVar8);
    if (puVar7 != (undefined4 *)0x0) {
      *puVar7 = pcVar4;
    }
    *(char **)((int)&PTR_GetModelName_00cb4fe4 + unaff_EBX + 2) = pcVar4;
    ___cxa_guard_release(unaff_EBX + 0xd9cc4a);
  }
  *(undefined4 *)((int)&PTR_VPhysicsDestroyObject_00cb4a0c + unaff_EBX + 2) = 9;
  *(int *)((int)&PTR_ForceVPhysicsCollide_00cb4a08 + unaff_EBX + 2) = unaff_EBX + 0xcb4de2;
  return (datamap_t *)((int)&PTR_ForceVPhysicsCollide_00cb4a08 + unaff_EBX + 2);
}


/* CFutbolCatcher::GetDataDescMap at 0074d560 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CFutbolCatcher * this) */

datamap_t * __thiscall CFutbolCatcher::GetDataDescMap(CFutbolCatcher *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x61a4c0);
}


/* CFutbolCatcher::GetBaseMap at 0074d570 */

datamap_t * CFutbolCatcher::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4f95e8);
}


/* __tcf_0 at 00a3ef30 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x410d46)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(&DAT_00410d3a + unaff_EBX) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x410d46));
  }
  *(undefined4 *)(unaff_EBX + 0x410d46) = 0;
  if (-1 < *(int *)(CBaseEntity::AppendContextToCriteria + unaff_EBX + 2)) {
    if (*(int *)(&DAT_00410d3a + unaff_EBX) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x207c1e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x207c1e),*(int *)(&DAT_00410d3a + unaff_EBX))
      ;
      *(undefined4 *)(&DAT_00410d3a + unaff_EBX) = 0;
    }
    *(undefined4 *)(&DAT_00410d3e + unaff_EBX) = 0;
  }
  iVar1 = *(int *)(&DAT_00410d3a + unaff_EBX);
  *(int *)(unaff_EBX + 0x410d4a) = iVar1;
  if (-1 < *(int *)(CBaseEntity::AppendContextToCriteria + unaff_EBX + 2)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x207c1e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x207c1e),iVar1);
      *(undefined4 *)(&DAT_00410d3a + unaff_EBX) = 0;
    }
    *(undefined4 *)(&DAT_00410d3e + unaff_EBX) = 0;
  }
  return;
}


/* DataMapInit<CPropFutbolSpawner> at 000b2bf0 */

datamap_t * DataMapInit<CPropFutbolSpawner>(CPropFutbolSpawner *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&ai_debug_los.m_Value.m_fValue + unaff_EBX) == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd9d02c);
    if (iVar2 != 0) {
      *(int *)(&ai_debug_los.m_bHasMax + unaff_EBX) = unaff_EBX + 0xa009a7;
      *(undefined4 *)((int)&ai_debug_los.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) = 0;
      *(undefined4 *)
       ((int)&ai_debug_los.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) = 0;
      *(undefined4 *)((int)&ai_debug_los.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) = 0;
      *(undefined4 *)((int)&ai_debug_los.m_fnChangeCallbacks.m_Size + unaff_EBX) = 0;
      *(undefined4 *)((int)&ai_debug_los.m_fnChangeCallbacks.m_pElements + unaff_EBX) = 0;
      *(undefined4 *)((int)&ai_debug_los.m_fMaxVal + unaff_EBX) = 0x12;
      ___cxa_guard_release(unaff_EBX + 0xd9d02c);
      ___cxa_atexit(unaff_EBX + 0x98c4ec,0,*(undefined4 *)(&DAT_00b93f50 + unaff_EBX));
    }
  }
  *(int *)((int)&PTR_GetReceivedDamageScale_00cb4e48 + unaff_EBX) =
       (int)&PTR_GetViewOffset_00cb4e24 + unaff_EBX;
  if ((&ai_debug_los.m_bHasMin)[unaff_EBX] == false) {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd9d034);
    if (iVar2 != 0) {
      uVar1 = **(undefined4 **)(&DAT_00b93f70 + unaff_EBX);
      *(undefined4 *)((int)&PTR_DrawDebugGeometryOverlays_00cb5110 + unaff_EBX) = uVar1;
      *(undefined4 *)((int)&PTR_Classify_00cb5150 + unaff_EBX) = uVar1;
      ___cxa_guard_release(unaff_EBX + 0xd9d034);
    }
  }
  *(undefined4 *)((int)&PTR_CanBeSeenBy_00cb4e40 + unaff_EBX) = 4;
  *(int *)((int)&PTR_FVisible_00cb4e3c + unaff_EBX) = (int)&PTR_ShouldTransmit_00cb50bc + unaff_EBX;
  return (datamap_t *)((int)&PTR_FVisible_00cb4e3c + unaff_EBX);
}


/* CPropFutbolSpawner::GetDataDescMap at 0074d580 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPropFutbolSpawner * this) */

datamap_t * __thiscall CPropFutbolSpawner::GetDataDescMap(CPropFutbolSpawner *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x61a4b8);
}


/* CPropFutbolSpawner::GetBaseMap at 0074d590 */

datamap_t * CPropFutbolSpawner::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(CPhysHinge::Deactivate + extraout_ECX);
}


/* __tcf_1 at 00a3f0f0 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x410b56)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(&DAT_00410b4a + unaff_EBX) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x410b56));
  }
  *(undefined4 *)(unaff_EBX + 0x410b56) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x410b52)) {
    if (*(int *)(&DAT_00410b4a + unaff_EBX) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x207a5e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x207a5e),*(int *)(&DAT_00410b4a + unaff_EBX))
      ;
      *(undefined4 *)(&DAT_00410b4a + unaff_EBX) = 0;
    }
    *(undefined4 *)(&DAT_00410b4e + unaff_EBX) = 0;
  }
  iVar1 = *(int *)(&DAT_00410b4a + unaff_EBX);
  *(int *)(unaff_EBX + 0x410b5a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x410b52)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x207a5e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x207a5e),iVar1);
      *(undefined4 *)(&DAT_00410b4a + unaff_EBX) = 0;
    }
    *(undefined4 *)(&DAT_00410b4e + unaff_EBX) = 0;
  }
  return;
}


/* CPropGlassFutbol::~CPropGlassFutbol at 0074eca0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropGlassFutbol(CPropGlassFutbol * this, int __in_chrg) */

void __thiscall CPropGlassFutbol::~CPropGlassFutbol(CPropGlassFutbol *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
  super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x582577);
  (this->super_CPhysicsProp).super_CBreakableProp.super_IBreakableWithPropData.
  _vptr_IBreakableWithPropData = (_func_int_varargs **)(unaff_EBX + 0x5829df);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x582a63);
  (this->super_CPhysicsProp).super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(unaff_EBX + 0x582a93);
  CPhysicsProp::~CPhysicsProp(&this->super_CPhysicsProp,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CPropGlassFutbol::~CPropGlassFutbol at 0074ed10 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropGlassFutbol(CPropGlassFutbol * this, int __in_chrg) */

void __thiscall CPropGlassFutbol::~CPropGlassFutbol(CPropGlassFutbol *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
  super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(&UNK_00582510 + extraout_ECX);
  (this->super_CPhysicsProp).super_CBreakableProp.super_IBreakableWithPropData.
  _vptr_IBreakableWithPropData = (_func_int_varargs **)(extraout_ECX + 0x582978);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(extraout_ECX + 0x5829fc);
  (this->super_CPhysicsProp).super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(extraout_ECX + 0x582a2c);
  CPhysicsProp::~CPhysicsProp(&this->super_CPhysicsProp,__in_chrg);
  return;
}


/* CPropGlassFutbol::PreferredCarryAngles at 0074eb50 */

/* DWARF original prototype: QAngle PreferredCarryAngles(CPropGlassFutbol * this) */

QAngle * __thiscall
CPropGlassFutbol::PreferredCarryAngles(QAngle *__return_storage_ptr__,CPropGlassFutbol *this)

{
  __return_storage_ptr__->x = 180.0;
  __return_storage_ptr__->y = -90.0;
  __return_storage_ptr__->z = 180.0;
  return __return_storage_ptr__;
}


/* CPropGlassFutbol::HasPreferredCarryAnglesForPlayer at 0074eb90 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: bool HasPreferredCarryAnglesForPlayer(CPropGlassFutbol * this,
   CBasePlayer * pPlayer) */

bool __thiscall
CPropGlassFutbol::HasPreferredCarryAnglesForPlayer(CPropGlassFutbol *this,CBasePlayer *pPlayer)

{
  return true;
}


/* DataMapInit<CPropGlassFutbol> at 000b2d20 */

datamap_t * DataMapInit<CPropGlassFutbol>(CPropGlassFutbol *param_1)

{
  int iVar1;
  int iVar2;
  char *pcVar3;
  size_t sVar4;
  int iVar5;
  undefined4 *puVar6;
  int iVar7;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((*(char *)((int)&sv_script_think_interval.m_Value.m_fValue + unaff_EBX) == '\0') &&
     (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd9cecc), iVar2 != 0)) {
    *(int *)(&sv_script_think_interval.m_bHasMax + unaff_EBX) = unaff_EBX + 0xa0088a;
    *(undefined4 *)
     ((int)&sv_script_think_interval.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&sv_script_think_interval.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) =
         0;
    *(undefined4 *)
     ((int)&sv_script_think_interval.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) = 0;
    *(undefined4 *)((int)&sv_script_think_interval.m_fnChangeCallbacks.m_Size + unaff_EBX) = 0;
    *(undefined4 *)((int)&sv_script_think_interval.m_fnChangeCallbacks.m_pElements + unaff_EBX) = 0;
    *(undefined4 *)((int)&sv_script_think_interval.m_fMaxVal + unaff_EBX) = 0x10;
    ___cxa_guard_release(unaff_EBX + 0xd9cecc);
    ___cxa_atexit(unaff_EBX + 0x98c2dc,0,*(undefined4 *)(&DAT_00b93e20 + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_ImpactTrace_00cb4d30 + unaff_EBX) =
       *(undefined4 *)(&DAT_00b94830 + unaff_EBX);
  if (((&sv_script_think_interval.m_bHasMin)[unaff_EBX] == false) &&
     (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd9ced4), iVar2 != 0)) {
    pcVar3 = operator_new___(*(int *)((int)&sv_script_think_interval.m_fMaxVal + unaff_EBX) + 0xc);
    _strcpy(pcVar3,*(char **)(&sv_script_think_interval.m_bHasMax + unaff_EBX));
    sVar4 = _strlen(pcVar3);
    builtin_strncpy(pcVar3 + sVar4,"ThrownThink",0xc);
    iVar1 = *(int *)((int)&sv_script_think_interval.m_fnChangeCallbacks.m_Size + unaff_EBX);
    iVar2 = iVar1 + 1;
    iVar7 = *(int *)((int)&sv_script_think_interval.m_fnChangeCallbacks.m_Memory.m_nAllocationCount
                    + unaff_EBX);
    if (iVar7 < iVar2) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)
                 ((int)&sv_script_think_interval.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX)
                 ,iVar2 - iVar7);
    }
    iVar5 = *(int *)((int)&sv_script_think_interval.m_fnChangeCallbacks.m_Size + unaff_EBX) + 1;
    *(int *)((int)&sv_script_think_interval.m_fnChangeCallbacks.m_Size + unaff_EBX) = iVar5;
    iVar7 = *(int *)((int)&sv_script_think_interval.m_fnChangeCallbacks.m_Memory.m_pMemory +
                    unaff_EBX);
    *(int *)((int)&sv_script_think_interval.m_fnChangeCallbacks.m_pElements + unaff_EBX) = iVar7;
    iVar5 = (iVar5 - iVar1) + -1;
    if (0 < iVar5) {
      _V_memmove((void *)(iVar7 + iVar2 * 4),(void *)(iVar7 + iVar1 * 4),iVar5 * 4);
      iVar7 = *(int *)((int)&sv_script_think_interval.m_fnChangeCallbacks.m_Memory.m_pMemory +
                      unaff_EBX);
    }
    puVar6 = (undefined4 *)(iVar1 * 4 + iVar7);
    if (puVar6 != (undefined4 *)0x0) {
      *puVar6 = pcVar3;
    }
    *(char **)((int)&PTR_NetworkStateChanged_m_vecBaseVelocity_00cb4ed0 + unaff_EBX) = pcVar3;
    pcVar3 = operator_new___(*(int *)((int)&sv_script_think_interval.m_fMaxVal + unaff_EBX) + 10);
    _strcpy(pcVar3,*(char **)(&sv_script_think_interval.m_bHasMax + unaff_EBX));
    sVar4 = _strlen(pcVar3);
    builtin_strncpy(pcVar3 + sVar4,"AnimThink",10);
    iVar1 = *(int *)((int)&sv_script_think_interval.m_fnChangeCallbacks.m_Size + unaff_EBX);
    iVar2 = iVar1 + 1;
    iVar7 = *(int *)((int)&sv_script_think_interval.m_fnChangeCallbacks.m_Memory.m_nAllocationCount
                    + unaff_EBX);
    if (iVar7 < iVar2) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)
                 ((int)&sv_script_think_interval.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX)
                 ,iVar2 - iVar7);
    }
    iVar5 = *(int *)((int)&sv_script_think_interval.m_fnChangeCallbacks.m_Size + unaff_EBX) + 1;
    *(int *)((int)&sv_script_think_interval.m_fnChangeCallbacks.m_Size + unaff_EBX) = iVar5;
    iVar7 = *(int *)((int)&sv_script_think_interval.m_fnChangeCallbacks.m_Memory.m_pMemory +
                    unaff_EBX);
    *(int *)((int)&sv_script_think_interval.m_fnChangeCallbacks.m_pElements + unaff_EBX) = iVar7;
    iVar5 = (iVar5 - iVar1) + -1;
    if (0 < iVar5) {
      _V_memmove((void *)(iVar7 + iVar2 * 4),(void *)(iVar7 + iVar1 * 4),iVar5 * 4);
      iVar7 = *(int *)((int)&sv_script_think_interval.m_fnChangeCallbacks.m_Memory.m_pMemory +
                      unaff_EBX);
    }
    puVar6 = (undefined4 *)(iVar1 * 4 + iVar7);
    if (puVar6 != (undefined4 *)0x0) {
      *puVar6 = pcVar3;
    }
    *(char **)((int)&PTR_typeinfo_00cb4f10 + unaff_EBX) = pcVar3;
    ___cxa_guard_release(unaff_EBX + 0xd9ced4);
  }
  *(undefined4 *)((int)&PTR_DamageDecal_00cb4d28 + unaff_EBX) = 7;
  *(int *)((int)&PTR_IsMoving_00cb4d24 + unaff_EBX) = (int)&PTR_StartBlocked_00cb4d8c + unaff_EBX;
  return (datamap_t *)((int)&PTR_IsMoving_00cb4d24 + unaff_EBX);
}


/* CPropGlassFutbol::GetDataDescMap at 0074d5a0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPropGlassFutbol * this) */

datamap_t * __thiscall CPropGlassFutbol::GetDataDescMap(CPropGlassFutbol *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x61a4b0);
}


/* CPropGlassFutbol::GetBaseMap at 0074d5b0 */

datamap_t * CPropGlassFutbol::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4f9fac);
}


/* __tcf_2 at 00a3f010 */

void __tcf_2(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x410c06)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x410bfa) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x410c06));
  }
  *(undefined4 *)(unaff_EBX + 0x410c06) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x410c02)) {
    if (*(int *)(unaff_EBX + 0x410bfa) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x207b3e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x207b3e),*(int *)(unaff_EBX + 0x410bfa));
      *(undefined4 *)(unaff_EBX + 0x410bfa) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x410bfe) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x410bfa);
  *(int *)(unaff_EBX + 0x410c0a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x410c02)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x207b3e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x207b3e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x410bfa) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x410bfe) = 0;
  }
  return;
}


/* CFutbolCatcher::CFutbolCatcher at 0074e8e0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CFutbolCatcher(CFutbolCatcher * this) */

void __thiscall CFutbolCatcher::CFutbolCatcher(CFutbolCatcher *this)

{
  vec_t *pvVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::CBaseAnimating(&this->super_CBaseAnimating);
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x58325b);
  (this->m_hCaughtFutbol).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_vCatcherBoxHalfDiagonal).x = 25.0;
  (this->m_vCatcherBoxHalfDiagonal).y = 25.0;
  (this->m_vCatcherBoxHalfDiagonal).z = 25.0;
  pvVar1 = *(vec_t **)(unaff_EBX + 0x4f828b);
  (this->m_vecCatchBoxMins).x = *pvVar1;
  (this->m_vecCatchBoxMins).y = pvVar1[1];
  (this->m_vecCatchBoxMins).z = pvVar1[2];
  (this->m_vecCatchBoxMaxs).x = *pvVar1;
  (this->m_vecCatchBoxMaxs).y = pvVar1[1];
  (this->m_vecCatchBoxMaxs).z = pvVar1[2];
  (this->m_vecCatchBoxOrig).x = *pvVar1;
  (this->m_vecCatchBoxOrig).y = pvVar1[1];
  (this->m_vecCatchBoxOrig).z = pvVar1[2];
  this->m_bDisableRecaptureOnPlayerGrab = false;
  (this->m_OnFutbolReleased).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnFutbolReleased).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->m_OnFutbolReleased).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_OnFutbolCaught).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnFutbolCaught).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->m_OnFutbolCaught).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  return;
}


/* CFutbolCatcher::CFutbolCatcher at 0074eae0 */

/* DWARF original prototype: void CFutbolCatcher(CFutbolCatcher * this, CFutbolCatcher * this) */

void __thiscall CFutbolCatcher::CFutbolCatcher(CFutbolCatcher *this,CFutbolCatcher *this_1)

{
  CFutbolCatcher(this);
  return;
}


/* CEntityFactory<CFutbolCatcher>::Create at 0074ef30 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CFutbolCatcher> * this, char
   * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CFutbolCatcher>::Create(CEntityFactory<CFutbolCatcher> *this,char *pClassName)

{
  CFutbolCatcher *this_00;
  
                    /* Unresolved local var: CFutbolCatcher * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x57c);
  CFutbolCatcher::CFutbolCatcher(this_00);
  (*(this_00->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1d])(this_00,pClassName);
  return &(this_00->super_CBaseAnimating).super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CFutbolCatcher::Spawn at 0074d760 */

/* DWARF original prototype: void Spawn(CFutbolCatcher * this) */

void __thiscall CFutbolCatcher::Spawn(CFutbolCatcher *this)

{
  uint uVar1;
  vec_t vVar2;
  vec_t vVar3;
  vec_t vVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::Spawn(&this->super_CBaseAnimating);
  CBaseAnimating::GetAttachment
            (&this->super_CBaseAnimating,(char *)(unaff_EBX + 0x365da7),&this->m_vecCatchBoxOrig,
             (Vector *)0x0,(Vector *)0x0,(Vector *)0x0);
  uVar1 = *(uint *)(unaff_EBX + 0x388373);
  vVar2 = (this->m_vCatcherBoxHalfDiagonal).z;
  vVar3 = (this->m_vCatcherBoxHalfDiagonal).y;
  vVar4 = (this->m_vCatcherBoxHalfDiagonal).x;
  (this->m_vecCatchBoxMins).x = (vec_t)(uVar1 ^ (uint)vVar4);
  (this->m_vecCatchBoxMins).y = (vec_t)((uint)vVar3 ^ uVar1);
  (this->m_vecCatchBoxMins).z = (vec_t)((uint)vVar2 ^ uVar1);
  (this->m_vecCatchBoxMaxs).x = vVar4;
  (this->m_vecCatchBoxMaxs).y = vVar3;
  (this->m_vecCatchBoxMaxs).z = vVar2;
  CBaseEntity::ThinkSet((CBaseEntity *)this,(BASEPTR)0x3a5,0.0,(char *)0x0);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(**(int **)(unaff_EBX + 0x4f93f7) + 0xc) +
             *(float *)(**(int **)(unaff_EBX + 0x4f93f7) + 0x1c),(char *)0x0);
  return;
}


/* CFutbolCatcher::CaptureFutbol at 0074e3a0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CaptureFutbol(CFutbolCatcher * this, CPropGlassFutbol * pFutbol)
    */

void __thiscall CFutbolCatcher::CaptureFutbol(CFutbolCatcher *this,CPropGlassFutbol *pFutbol)

{
  IPhysicsObject *pIVar1;
  uint uVar2;
  ulong *puVar3;
  int iVar4;
  CBaseEntity *pCaller;
  int unaff_EBX;
  
                    /* Unresolved local var: IPhysicsObject * pPhysicsObject@[???] */
  ___i686_get_pc_thunk_bx();
  if (pFutbol != (CPropGlassFutbol *)0x0) {
    puVar3 = (ulong *)(**(code **)((int)(pFutbol->super_CPhysicsProp).super_CBreakableProp.
                                        super_CBaseProp.super_CBaseAnimating.super_CBaseEntity.
                                        super_IServerEntity.super_IServerUnknown + 0xc))(pFutbol);
    (this->m_hCaughtFutbol).super_CBaseHandle.m_Index = *puVar3;
    pIVar1 = (pFutbol->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating
             .super_CBaseEntity.m_pPhysicsObject;
    if (pIVar1 != (IPhysicsObject *)0x0) {
      (*pIVar1->_vptr_IPhysicsObject[0x10])(pIVar1,0);
    }
    (**(code **)((int)(pFutbol->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                      super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
                      super_IServerUnknown + 0x1c4))
              (pFutbol,&this->m_vecCatchBoxOrig,*(undefined4 *)(unaff_EBX + 0x4f8854),
               *(undefined4 *)(unaff_EBX + 0x4f87c4));
    CBaseEntity::ThinkSet((CBaseEntity *)this,(BASEPTR)0x3a9,0.0,(char *)0x0);
    pFutbol->m_Holder = FUTBOL_HELD_BY_CATCHER;
    uVar2 = (pFutbol->m_hLastHeldByPlayer).super_CBaseHandle.m_Index;
    if ((uVar2 == 0xffffffff) ||
       (iVar4 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f87b8),
       *(uint *)(iVar4 + 8) != uVar2 >> 0x10)) {
      pCaller = (CBaseEntity *)0x0;
    }
    else {
      pCaller = *(CBaseEntity **)(iVar4 + 4);
    }
    COutputEvent::FireOutput(&this->m_OnFutbolCaught,(CBaseEntity *)pFutbol,pCaller,0.0);
  }
  return;
}


/* CFutbolCatcher::CaptureThink at 0074df30 */

/* DWARF original prototype: void CaptureThink(CFutbolCatcher * this) */

void __thiscall CFutbolCatcher::CaptureThink(CFutbolCatcher *this)

{
  uint uVar1;
  int iVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: CPropGlassFutbol * pFutbol@[???] */
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->m_hCaughtFutbol).super_CBaseHandle.m_Index;
  if ((((uVar1 == 0xffffffff) ||
       (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f8c28),
       *(uint *)(iVar2 + 8) != uVar1 >> 0x10)) || (iVar2 = *(int *)(iVar2 + 4), iVar2 == 0)) ||
     (*(int *)(iVar2 + 0x770) != 3)) {
    CBaseEntity::ThinkSet((CBaseEntity *)this,(BASEPTR)0x3a5,0.0,(char *)0x0);
    COutputEvent::FireOutput(&this->m_OnFutbolReleased,(CBaseEntity *)this,(CBaseEntity *)this,0.0);
    (this->m_hCaughtFutbol).super_CBaseHandle.m_Index = 0xffffffff;
    this->m_bDisableRecaptureOnPlayerGrab = true;
  }
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(**(int **)(unaff_EBX + 0x4f8c20) + 0xc) +
             *(float *)(**(int **)(unaff_EBX + 0x4f8c20) + 0x1c),(char *)0x0);
  return;
}


/* CPropGlassFutbol::CPropGlassFutbol at 0074e820 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CPropGlassFutbol(CPropGlassFutbol * this) */

void __thiscall CPropGlassFutbol::CPropGlassFutbol(CPropGlassFutbol *this)

{
  vec_t *pvVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CPhysicsProp::CPhysicsProp(&this->super_CPhysicsProp);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
  super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x5829fb);
  (this->super_CPhysicsProp).super_CBreakableProp.super_IBreakableWithPropData.
  _vptr_IBreakableWithPropData = (_func_int_varargs **)(unaff_EBX + 0x582e63);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x582ee7);
  (this->super_CPhysicsProp).super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(unaff_EBX + 0x582f17);
  (this->m_hSpawner).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_strSpawnerName).pszValue = (char *)0x0;
  this->m_Holder = FUTBOL_HELD_BY_NONE;
  pvVar1 = *(vec_t **)(unaff_EBX + 0x4f834b);
  (this->m_vecThrowDirection).x = *pvVar1;
  (this->m_vecThrowDirection).y = pvVar1[1];
  (this->m_vecThrowDirection).z = pvVar1[2];
  (this->m_hLastHeldByPlayer).super_CBaseHandle.m_Index = 0xffffffff;
  return;
}


/* CPropGlassFutbol::CPropGlassFutbol at 0074e8d0 */

/* DWARF original prototype: void CPropGlassFutbol(CPropGlassFutbol * this, CPropGlassFutbol * this)
    */

void __thiscall CPropGlassFutbol::CPropGlassFutbol(CPropGlassFutbol *this,CPropGlassFutbol *this_1)

{
  CPropGlassFutbol(this);
  return;
}


/* CEntityFactory<CPropGlassFutbol>::Create at 0074eee0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CPropGlassFutbol> * this,
   char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CPropGlassFutbol>::Create(CEntityFactory<CPropGlassFutbol> *this,char *pClassName)

{
  CPropGlassFutbol *this_00;
  
                    /* Unresolved local var: CPropGlassFutbol * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x784);
  CPropGlassFutbol::CPropGlassFutbol(this_00);
  (**(code **)((int)(this_00->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x74))(this_00,pClassName);
  return &(this_00->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
          super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CPropGlassFutbol::Precache at 0074d730 */

/* DWARF original prototype: void Precache(CPropGlassFutbol * this) */

void __thiscall CPropGlassFutbol::Precache(CPropGlassFutbol *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CPhysicsProp::Precache(&this->super_CPhysicsProp);
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x365dc0));
  return;
}


/* CPropGlassFutbol::Spawn at 0074e4c0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(CPropGlassFutbol * this) */

void __thiscall CPropGlassFutbol::Spawn(CPropGlassFutbol *this)

{
  uint *puVar1;
  ushort uVar2;
  CBaseEdict *this_00;
  ushort *puVar3;
  BASEPTR func;
  int iVar4;
  CBaseEntity *pCVar5;
  int *piVar6;
  ulong *puVar7;
  IChangeInfoAccessor *pIVar8;
  char *pcVar9;
  uint uVar10;
  uint uVar11;
  int unaff_EBX;
  uint uVar12;
  uint local_60;
  
  ___i686_get_pc_thunk_bx();
  (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x68))(this);
  (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x80))(this,unaff_EBX + 0x358aa2,unaff_EBX + 0x36502e);
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
  uVar10 = (this->m_hSpawner).super_CBaseHandle.m_Index;
  if (((uVar10 == 0xffffffff) ||
      (iVar4 = (uVar10 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f869e),
      *(uint *)(iVar4 + 8) != uVar10 >> 0x10)) || (*(int *)(iVar4 + 4) == 0)) {
    pcVar9 = (this->m_strSpawnerName).pszValue;
    if ((pcVar9 == (char *)0x0) ||
       (pCVar5 = CGlobalEntityList::FindEntityByName
                           (*(CGlobalEntityList **)(unaff_EBX + 0x4f8756),(CBaseEntity *)0x0,pcVar9,
                            (CBaseEntity *)0x0,(CBaseEntity *)0x0,(CBaseEntity *)0x0,
                            (IEntityFindFilter *)0x0), pCVar5 == (CBaseEntity *)0x0)) {
      _Warning(unaff_EBX + 0x36509e);
    }
    else {
      piVar6 = (int *)___dynamic_cast(pCVar5,*(undefined4 *)(unaff_EBX + 0x4f86c2),
                                      unaff_EBX + 0x583a42,0);
      if (piVar6 == (int *)0x0) {
        pcVar9 = CBaseEntity::GetDebugName((CBaseEntity *)0x0);
        iVar4 = section_00000054.segname._4_4_;
        if (section_00000054.segname._4_4_ == 0) {
          iVar4 = unaff_EBX + 0x300856;
        }
        _Warning(unaff_EBX + 0x36504e,iVar4,pcVar9);
      }
      else {
        puVar7 = (ulong *)(**(code **)(*piVar6 + 0xc))(piVar6);
        (this->m_hSpawner).super_CBaseHandle.m_Index = *puVar7;
      }
    }
  }
  CPhysicsProp::Spawn(&this->super_CPhysicsProp);
  iVar4 = CBaseAnimating::LookupSequence((CBaseAnimating *)this,(char *)(unaff_EBX + 0x36573e));
  CBaseAnimating::ResetSequence((CBaseAnimating *)this,iVar4);
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0xe2e);
  CBaseEntity::ThinkSet
            ((CBaseEntity *)this,func,
             *(float *)(unaff_EBX + 0x37e446) + *(float *)(**(int **)(unaff_EBX + 0x4f8696) + 0xc),
             *(char **)(unaff_EBX + 0x619552));
  uVar10 = (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
           super_CBaseEntity.m_spawnflags.m_Value;
  uVar12 = uVar10 | 0x140;
  if (uVar10 != uVar12) {
    if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
        false) {
      this_00 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                  super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar8 = CBaseEdict::GetChangeAccessor(this_00);
        puVar3 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4f869a);
        if (pIVar8->m_iChangeInfoSerialNumber == *puVar3) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar10 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar3[uVar10 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_0074e7e4:
            puVar3[(uint)uVar2 + uVar10 * 0x14 + 1] = 0x128;
            puVar3[uVar10 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar3[uVar10 * 0x14 + 1] != 0x128) {
            local_60 = 0;
            do {
              uVar11 = local_60 + 1;
              local_60 = uVar11 & 0xffff;
              if ((ushort)uVar11 == uVar2) {
                if (uVar2 == 0x13) goto LAB_0074e809;
                goto LAB_0074e7e4;
              }
            } while (puVar3[uVar10 * 0x14 + local_60 + 1] != 0x128);
          }
        }
        else if (puVar3[0x7d1] == 100) {
LAB_0074e809:
          pIVar8->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar8->m_iChangeInfo = puVar3[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4f869a) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4f869a) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x4f869a);
          pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar4 = *piVar6 + (uint)pIVar8->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar4 + 2) = 0x128;
          *(undefined2 *)(iVar4 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = (uint *)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
      *puVar1 = *puVar1 | 1;
    }
    (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
    super_CBaseEntity.m_spawnflags.m_Value = uVar12;
  }
  return;
}


/* CPropGlassFutbol::Event_Killed at 0074e010 */

/* DWARF original prototype: void Event_Killed(CPropGlassFutbol * this, CTakeDamageInfo * info) */

void __thiscall CPropGlassFutbol::Event_Killed(CPropGlassFutbol *this,CTakeDamageInfo *info)

{
  uint uVar1;
  int *piVar2;
  CBaseEntity *this_00;
  bool bVar3;
  int iVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  uVar1 = (info->m_hAttacker).super_CBaseHandle.m_Index;
  if (((uVar1 == 0xffffffff) ||
      (iVar4 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f8b48),
      *(uint *)(iVar4 + 8) != uVar1 >> 0x10)) ||
     (this_00 = *(CBaseEntity **)(iVar4 + 4), this_00 == (CBaseEntity *)0x0)) {
LAB_0074e04a:
    CBreakableProp::Event_Killed((CBreakableProp *)this,info);
  }
  else {
    if ((this_00->m_iClassname).pszValue != (char *)(unaff_EBX + 0x30fca0)) {
      bVar3 = CBaseEntity::ClassMatchesComplex(this_00,(char *)(unaff_EBX + 0x30fca0));
      if (!bVar3) goto LAB_0074e04a;
    }
    CBaseEntity::Event_Killed((CBaseEntity *)this,info);
  }
  uVar1 = (this->m_hSpawner).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar4 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f8b48),
      *(uint *)(iVar4 + 8) == uVar1 >> 0x10)) &&
     (piVar2 = *(int **)(iVar4 + 4), piVar2 != (int *)0x0)) {
    (**(code **)(*piVar2 + 0x3b0))(piVar2);
  }
  return;
}


/* CPropGlassFutbol::OnPhysGunPickup at 0074e240 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void OnPhysGunPickup(CPropGlassFutbol * this, CBasePlayer *
   pPhysGunUser, PhysGunPickup_t reason) */

void __thiscall
CPropGlassFutbol::OnPhysGunPickup
          (CPropGlassFutbol *this,CBasePlayer *pPhysGunUser,PhysGunPickup_t reason)

{
  IPhysicsObject *pIVar1;
  uint uVar2;
  int *piVar3;
  CPortal_Player *this_00;
  ulong *puVar4;
  int iVar5;
  CBasePlayer *pCVar6;
  int unaff_EBX;
  
                    /* Unresolved local var: CPortal_Player * pHoldingPlayer@[???]
                       Unresolved local var: IPhysicsObject * pPhysicsObject@[???] */
  ___i686_get_pc_thunk_bx();
  if (pPhysGunUser == (CBasePlayer *)0x0) {
    this_00 = (CPortal_Player *)0x0;
  }
  else {
    this_00 = (CPortal_Player *)
              ___dynamic_cast(pPhysGunUser,*(undefined4 *)(unaff_EBX + 0x4f89c4),
                              *(undefined4 *)(unaff_EBX + 0x4f8ba0),0);
    if (this_00 != (CPortal_Player *)0x0) {
      puVar4 = (ulong *)(**(code **)(*(int *)&(this_00->
                                              super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>
                                              ).super_CPaintableEntity<CBaseMultiplayerPlayer>.
                                              super_CBaseMultiplayerPlayer.
                                              super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer
                                              .super_CBaseCombatCharacter.super_CBaseFlex + 0xc))
                                  (this_00);
      (this->m_hLastHeldByPlayer).super_CBaseHandle.m_Index = *puVar4;
      goto LAB_0074e2ad;
    }
  }
  (this->m_hLastHeldByPlayer).super_CBaseHandle.m_Index = 0xffffffff;
LAB_0074e2ad:
  pIVar1 = (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
           super_CBaseEntity.m_pPhysicsObject;
  if (pIVar1 != (IPhysicsObject *)0x0) {
    (*pIVar1->_vptr_IPhysicsObject[0x10])(pIVar1,1);
  }
  if (((reason == PICKED_UP_BY_PLAYER) && (this_00 != (CPortal_Player *)0x0)) &&
     (*(int *)(*(int *)(unaff_EBX + 0x67b548) + 0x30) != 0)) {
    CPortal_Player::SetUseKeyCooldownTime
              (this_00,*(float *)(*(int *)(&DAT_0067b4e8 + unaff_EBX) + 0x2c));
  }
  uVar2 = (this->m_hSpawner).super_CBaseHandle.m_Index;
  if (((uVar2 != 0xffffffff) &&
      (iVar5 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f8918),
      *(uint *)(iVar5 + 8) == uVar2 >> 0x10)) &&
     (piVar3 = *(int **)(iVar5 + 4), piVar3 != (int *)0x0)) {
    (**(code **)(*piVar3 + 0x3b4))(piVar3);
  }
  if (this->m_Holder == FUTBOL_HELD_BY_PLAYER) {
                    /* Unresolved local var: CBasePlayer * pOtherPlayer@[???] */
    pCVar6 = GetPlayerHoldingEntity((CBaseEntity *)this);
    if (pCVar6 != (CBasePlayer *)0x0) {
      (**(code **)((int)(pCVar6->super_CBaseCombatCharacter).super_CBaseFlex.
                        super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                        super_IServerEntity.super_IServerUnknown + 0x6e0))(pCVar6,0);
    }
  }
  this->m_Holder = FUTBOL_HELD_BY_PLAYER;
  return;
}


/* CPropGlassFutbol::OnPhysGunDrop at 0074e100 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void OnPhysGunDrop(CPropGlassFutbol * this, CBasePlayer * pPhysGunUser,
   PhysGunDrop_t reason) */

void __thiscall
CPropGlassFutbol::OnPhysGunDrop
          (CPropGlassFutbol *this,CBasePlayer *pPhysGunUser,PhysGunDrop_t reason)

{
  uint uVar1;
  VMatrix matThisToLinked;
  BASEPTR func;
  char cVar2;
  int iVar3;
  int iVar4;
  CBasePlayer *pCVar5;
  int unaff_EBX;
  undefined4 *puVar6;
  undefined4 *puVar7;
  CBasePlayer *pCVar8;
  Vector *pVVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 in_stack_ffffff94;
  undefined1 in_stack_ffffff98 [44];
  Vector *pForward;
  Vector *vTransformed;
  Vector local_28 [2];
  
                    /* Unresolved local var: Vector forward@[???] */
  ___i686_get_pc_thunk_bx();
  if (pPhysGunUser != (CBasePlayer *)0x0) {
    CPhysicsProp::OnPhysGunDrop(&this->super_CPhysicsProp,pPhysGunUser,reason);
    uVar11 = 0;
    uVar10 = 0;
    pForward = local_28;
    pVVar9 = pForward;
    CBasePlayer::EyeVectors(pPhysGunUser,pForward,(Vector *)0x0,(Vector *)0x0);
    cVar2 = (**(code **)((int)(pPhysGunUser->super_CBaseCombatCharacter).super_CBaseFlex.
                              super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                              super_IServerEntity.super_IServerUnknown + 0x154))();
    pCVar5 = (CBasePlayer *)0x0;
    if (cVar2 != '\0') {
      pCVar5 = pPhysGunUser;
    }
    if ((char)pCVar5[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
              super_CBaseAnimating.m_nSkin.m_Value != '\0') {
                    /* Unresolved local var: CPortal_Base2D * pPortal@[???] */
      pCVar8 = pPhysGunUser;
      cVar2 = (**(code **)((int)(pPhysGunUser->super_CBaseCombatCharacter).super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                                super_IServerEntity.super_IServerUnknown + 0x154))();
      pCVar5 = (CBasePlayer *)0x0;
      if (cVar2 != '\0') {
        pCVar5 = pPhysGunUser;
      }
      uVar1 = pCVar5[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
              super_CBaseAnimating.m_nBody.m_Value;
      if ((uVar1 == 0xffffffff) ||
         (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f8a5e),
         *(uint *)(iVar3 + 8) != uVar1 >> 0x10)) {
        iVar3 = 0;
      }
      else {
        iVar3 = *(int *)(iVar3 + 4);
      }
      puVar6 = (undefined4 *)(iVar3 + 0x528);
      puVar7 = (undefined4 *)&stack0xffffff84;
      vTransformed = pForward;
      for (iVar4 = 0x10; iVar4 != 0; iVar4 = iVar4 + -1) {
        *puVar7 = *puVar6;
        puVar6 = puVar6 + 1;
        puVar7 = puVar7 + 1;
      }
      matThisToLinked.m[0][1] = (vec_t)pVVar9;
      matThisToLinked.m[0][0] = (vec_t)pCVar8;
      matThisToLinked.m[0][2] = (vec_t)uVar10;
      matThisToLinked.m[0][3] = (vec_t)uVar11;
      matThisToLinked.m[1][0] = (vec_t)in_stack_ffffff94;
      matThisToLinked.m[1][1] = (vec_t)in_stack_ffffff98._0_4_;
      matThisToLinked.m[1][2] = (vec_t)in_stack_ffffff98._4_4_;
      matThisToLinked.m[1][3] = (vec_t)in_stack_ffffff98._8_4_;
      matThisToLinked.m[2][0] = (vec_t)in_stack_ffffff98._12_4_;
      matThisToLinked.m[2][1] = (vec_t)in_stack_ffffff98._16_4_;
      matThisToLinked.m[2][2] = (vec_t)in_stack_ffffff98._20_4_;
      matThisToLinked.m[2][3] = (vec_t)in_stack_ffffff98._24_4_;
      matThisToLinked.m[3][0] = (vec_t)in_stack_ffffff98._28_4_;
      matThisToLinked.m[3][1] = (vec_t)in_stack_ffffff98._32_4_;
      matThisToLinked.m[3][2] = (vec_t)in_stack_ffffff98._36_4_;
      matThisToLinked.m[3][3] = (vec_t)in_stack_ffffff98._40_4_;
      UTIL_Portal_VectorTransform(matThisToLinked,pForward,vTransformed);
    }
    (this->m_vecThrowDirection).x = local_28[0].x;
    (this->m_vecThrowDirection).y = local_28[0].y;
    (this->m_vecThrowDirection).z = local_28[0].z;
    func.__delta = 0;
    func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x33e);
    CBaseEntity::ThinkSet
              ((CBaseEntity *)this,func,
               *(float *)(**(int **)(unaff_EBX + 0x4f8a56) + 0xc) +
               *(float *)(**(int **)(unaff_EBX + 0x4f8a56) + 0x1c),
               *(char **)(&DAT_00619916 + unaff_EBX));
    this->m_Holder = FUTBOL_HELD_BY_NONE;
  }
  return;
}


/* CPropGlassFutbol::SetSpawner at 0074eaf0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void SetSpawner(CPropGlassFutbol * this, CPropFutbolSpawner *
   pMySpawner) */

void __thiscall CPropGlassFutbol::SetSpawner(CPropGlassFutbol *this,CPropFutbolSpawner *pMySpawner)

{
  ulong *puVar1;
  
  if (pMySpawner != (CPropFutbolSpawner *)0x0) {
    puVar1 = (ulong *)(*(pMySpawner->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.
                        super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
                        _vptr_IHandleEntity[3])(pMySpawner);
    (this->m_hSpawner).super_CBaseHandle.m_Index = *puVar1;
    return;
  }
  (this->m_hSpawner).super_CBaseHandle.m_Index = 0xffffffff;
  return;
}


/* CPropGlassFutbol::OnTakeDamage at 0074d700 */

/* DWARF original prototype: int OnTakeDamage(CPropGlassFutbol * this, CTakeDamageInfo * info) */

int __thiscall CPropGlassFutbol::OnTakeDamage(CPropGlassFutbol *this,CTakeDamageInfo *info)

{
  int iVar1;
  
  if (1 < this->m_Holder - FUTBOL_HELD_BY_SPAWNER) {
    iVar1 = CPhysicsProp::OnTakeDamage(&this->super_CPhysicsProp,info);
    return iVar1;
  }
  return 0;
}


/* CPropGlassFutbol::VPhysicsCollision at 0074dbd0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void VPhysicsCollision(CPropGlassFutbol * this, int index,
   gamevcollisionevent_t * pEvent) */

void __thiscall
CPropGlassFutbol::VPhysicsCollision(CPropGlassFutbol *this,int index,gamevcollisionevent_t *pEvent)

{
  int *piVar1;
  char cVar2;
  CPortal_Player *this_00;
  CBaseEntity *pCVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(int *)(*(int *)(unaff_EBX + 0x67baf8) + 0x30) != 0) {
                    /* Unresolved local var: CBaseEntity * pHitEntity@[???] */
    piVar1 = *(int **)((int)pEvent->pEntities + (-(uint)(index == 0) & 4));
    cVar2 = (**(code **)(*piVar1 + 0x154))(piVar1);
    if (cVar2 != '\0') {
                    /* Unresolved local var: CPortal_Player * pPlayer@[???] */
      this_00 = (CPortal_Player *)
                ___dynamic_cast(piVar1,*(undefined4 *)(unaff_EBX + 0x4f8fac),
                                *(undefined4 *)(unaff_EBX + 0x4f9210),0);
      if (this_00 != (CPortal_Player *)0x0) {
        pCVar3 = GetPlayerHeldEntity((CBasePlayer *)this_00);
        if ((pCVar3 == (CBaseEntity *)0x0) && (this->m_Holder == FUTBOL_HELD_BY_NONE)) {
          (**(code **)(*(int *)&(this_00->
                                super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                                super_CPaintableEntity<CBaseMultiplayerPlayer>.
                                super_CBaseMultiplayerPlayer.super_CAI_ExpresserHost<CBasePlayer>.
                                super_CBasePlayer.super_CBaseCombatCharacter.super_CBaseFlex + 0x6dc
                      ))(this_00,this,1);
          CPortal_Player::SetUseKeyCooldownTime
                    (this_00,(float)(*(int *)(*(int *)(unaff_EBX + 0x67bb58) + 0x30) != 0));
        }
      }
    }
  }
  CPhysicsProp::VPhysicsCollision(&this->super_CPhysicsProp,index,pEvent);
  return;
}


/* CPropGlassFutbol::SetHolder at 0074d5c0 */

/* DWARF original prototype: void SetHolder(CPropGlassFutbol * this, futbol_holder_type_t type) */

void __thiscall CPropGlassFutbol::SetHolder(CPropGlassFutbol *this,futbol_holder_type_t type)

{
  this->m_Holder = type;
  return;
}


/* CPropFutbolSpawner::SpawnFutbol at 0074dce0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void SpawnFutbol(CPropFutbolSpawner * this) */

void __thiscall CPropFutbolSpawner::SpawnFutbol(CPropFutbolSpawner *this)

{
  CBaseEntity *this_00;
  int unaff_EBX;
  Vector local_28 [2];
  
                    /* Unresolved local var: CPropGlassFutbol * pBall@[???]
                       Unresolved local var: Vector vecBallSpawnPoint@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CreateEntityByName((char *)(unaff_EBX + 0x3635be),-1,true);
  if (this_00 != (CBaseEntity *)0x0) {
    COutputEvent::FireOutput(&this->m_OnFutbolSpawned,(CBaseEntity *)this,(CBaseEntity *)this,0.0);
    CBaseAnimating::GetAttachment
              ((CBaseAnimating *)this,(char *)(unaff_EBX + 0x365820),local_28,(Vector *)0x0,
               (Vector *)0x0,(Vector *)0x0);
    CBaseEntity::SetAbsOrigin(this_00,local_28);
    (*(this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
      [0x117])(this_00,this);
    this_00[1].m_ScriptScope.m_FuncHandles.
    super_CUtlVector<HSCRIPT__**,CUtlMemoryConservative<HSCRIPT__**>_>.m_pElements =
         (HSCRIPT **)((int)&vgui::Panel::GetMessageMap::s_pMap.magic + 2);
    this->m_bHasFutbol = true;
    DispatchSpawn(this_00,true);
    (*(this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
      super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xeb])(this,this_00);
  }
  return;
}


/* CPropGlassFutbol::ThrownThink at 0074ddd0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void ThrownThink(CPropGlassFutbol * this) */

void __thiscall CPropGlassFutbol::ThrownThink(CPropGlassFutbol *this)

{
  IPhysicsObject *pIVar1;
  int unaff_EBX;
  Vector local_28 [2];
  
                    /* Unresolved local var: Vector vecForce@[???]
                       Unresolved local var: IPhysicsObject * pPhys@[???] */
  ___i686_get_pc_thunk_bx();
  local_28[0].z = *(float *)(*(int *)(unaff_EBX + 0x67b898) + 0x2c);
                    /* Unresolved local var: Vector res@[???] */
  local_28[0].x = local_28[0].z * (this->m_vecThrowDirection).x;
  local_28[0].y = local_28[0].z * (this->m_vecThrowDirection).y;
  local_28[0].z = local_28[0].z * (this->m_vecThrowDirection).z;
  pIVar1 = (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
           super_CBaseEntity.m_pPhysicsObject;
  if (pIVar1 != (IPhysicsObject *)0x0) {
    (*pIVar1->_vptr_IPhysicsObject[0x34])
              (pIVar1,*(undefined4 *)(CNPC_Furniture::UpdateOnRemove + unaff_EBX + 4),0);
    (*pIVar1->_vptr_IPhysicsObject[0x3d])(pIVar1,local_28);
  }
  CBaseEntity::ApplyAbsVelocityImpulse((CBaseEntity *)this,local_28);
  CBaseEntity::ThinkSet((CBaseEntity *)this,(BASEPTR)0x0,-1.0,*(char **)(unaff_EBX + 0x619c40));
  return;
}


/* CPropGlassFutbol::AnimThink at 0074d6a0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void AnimThink(CPropGlassFutbol * this) */

void __thiscall CPropGlassFutbol::AnimThink(CPropGlassFutbol *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x31c))(this);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(**(int **)(unaff_EBX + 0x4f94b3) + 0xc) +
             *(float *)(**(int **)(unaff_EBX + 0x4f94b3) + 0x1c),
             *(char **)(&DAT_0061a36f + unaff_EBX));
  return;
}


/* CPropFutbolSpawner::CPropFutbolSpawner at 0074ea40 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CPropFutbolSpawner(CPropFutbolSpawner * this) */

void __thiscall CPropFutbolSpawner::CPropFutbolSpawner(CPropFutbolSpawner *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CFutbolCatcher::CFutbolCatcher(&this->super_CFutbolCatcher);
  (this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x582d17);
  this->m_bHasFutbol = false;
  (this->m_OnFutbolSpawned).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnFutbolSpawned).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->m_OnFutbolSpawned).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_OnFutbolGrabbed).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnFutbolGrabbed).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->m_OnFutbolGrabbed).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->super_CFutbolCatcher).m_vCatcherBoxHalfDiagonal.x = 7.0;
  (this->super_CFutbolCatcher).m_vCatcherBoxHalfDiagonal.y = 7.0;
  (this->super_CFutbolCatcher).m_vCatcherBoxHalfDiagonal.z = 7.0;
  return;
}


/* CPropFutbolSpawner::CPropFutbolSpawner at 0074ead0 */

/* DWARF original prototype: void CPropFutbolSpawner(CPropFutbolSpawner * this, CPropFutbolSpawner *
   this) */

void __thiscall
CPropFutbolSpawner::CPropFutbolSpawner(CPropFutbolSpawner *this,CPropFutbolSpawner *this_1)

{
  CPropFutbolSpawner(this);
  return;
}


/* CEntityFactory<CPropFutbolSpawner>::Create at 0074ef80 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CPropFutbolSpawner> * this,
   char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CPropFutbolSpawner>::Create
          (CEntityFactory<CPropFutbolSpawner> *this,char *pClassName)

{
  CPropFutbolSpawner *this_00;
  
                    /* Unresolved local var: CPropFutbolSpawner * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x5b0);
  CPropFutbolSpawner::CPropFutbolSpawner(this_00);
  (*(this_00->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1d])(this_00,pClassName);
  return &(this_00->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.m_Network.
          super_IServerNetworkable;
}


/* CPropFutbolSpawner::Precache at 0074d640 */

/* DWARF original prototype: void Precache(CPropFutbolSpawner * this) */

void __thiscall CPropFutbolSpawner::Precache(CPropFutbolSpawner *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  CBaseEntity::PrecacheModel((char *)(extraout_ECX + 0x365e90));
  return;
}


/* CPropFutbolSpawner::Spawn at 0074d890 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CPropFutbolSpawner * this) */

void __thiscall CPropFutbolSpawner::Spawn(CPropFutbolSpawner *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1a])(this);
  (*(this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1b])(this,unaff_EBX + 0x365c37);
  CCollisionProperty::SetSolid
            (&(this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.m_Collision.
              super_CCollisionProperty,SOLID_VPHYSICS);
  CFutbolCatcher::Spawn(&this->super_CFutbolCatcher);
  if (this->m_bHasFutbol == false) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x0074d904. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xee])();
  return;
}


/* CPropFutbolSpawner::FutbolDestroyed at 0074d5e0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void FutbolDestroyed(CPropFutbolSpawner * this) */

void __thiscall CPropFutbolSpawner::FutbolDestroyed(CPropFutbolSpawner *this)

{
                    /* WARNING: Could not recover jumptable at 0x0074d5ef. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xee])();
  return;
}


/* CPropFutbolSpawner::FutbolGrabbed at 0074d660 */

/* DWARF original prototype: void FutbolGrabbed(CPropFutbolSpawner * this) */

void __thiscall CPropFutbolSpawner::FutbolGrabbed(CPropFutbolSpawner *this)

{
  if (this->m_bHasFutbol != false) {
    COutputEvent::FireOutput(&this->m_OnFutbolGrabbed,(CBaseEntity *)this,(CBaseEntity *)this,0.0);
  }
  this->m_bHasFutbol = false;
  return;
}


/* CPropFutbolSpawner::InputForceSpawn at 0074d600 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void InputForceSpawn(CPropFutbolSpawner * this, inputdata_t * data) */

void __thiscall CPropFutbolSpawner::InputForceSpawn(CPropFutbolSpawner *this,inputdata_t *data)

{
                    /* WARNING: Could not recover jumptable at 0x0074d60f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xee])();
  return;
}


/* CPropFutbolSocket::CPropFutbolSocket at 0074e9c0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CPropFutbolSocket(CPropFutbolSocket * this) */

void __thiscall CPropFutbolSocket::CPropFutbolSocket(CPropFutbolSocket *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CFutbolCatcher::CFutbolCatcher(&this->super_CFutbolCatcher);
  (this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x582497);
  return;
}


/* CPropFutbolSocket::CPropFutbolSocket at 0074ea00 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CPropFutbolSocket(CPropFutbolSocket * this) */

void __thiscall CPropFutbolSocket::CPropFutbolSocket(CPropFutbolSocket *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CFutbolCatcher::CFutbolCatcher(&this->super_CFutbolCatcher);
  (this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x582457);
  return;
}


/* CEntityFactory<CPropFutbolSocket>::Create at 0074efd0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CPropFutbolSocket> * this,
   char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CPropFutbolSocket>::Create(CEntityFactory<CPropFutbolSocket> *this,char *pClassName)

{
  CFutbolCatcher *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CPropFutbolSocket * pEnt@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CBaseEntity::operator_new(0x57c);
  CFutbolCatcher::CFutbolCatcher(this_00);
  (this_00->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x581e84);
  CBaseEntity::PostConstructor((CBaseEntity *)this_00,pClassName);
  return &(this_00->super_CBaseAnimating).super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CPropFutbolSocket::Precache at 0074d620 */

/* DWARF original prototype: void Precache(CPropFutbolSocket * this) */

void __thiscall CPropFutbolSocket::Precache(CPropFutbolSocket *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  CBaseEntity::PrecacheModel((char *)(extraout_ECX + 0x365e90));
  return;
}


/* CPropFutbolSocket::Spawn at 0074dec0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CPropFutbolSocket * this) */

void __thiscall CPropFutbolSocket::Spawn(CPropFutbolSocket *this)

{
  int nSequence;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1a])(this);
  (*(this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1b])(this,unaff_EBX + 0x3655eb);
  CCollisionProperty::SetSolid
            (&(this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.m_Collision.
              super_CCollisionProperty,SOLID_VPHYSICS);
  nSequence = CBaseAnimating::LookupSequence((CBaseAnimating *)this,&UNK_0031877e + unaff_EBX);
  CBaseAnimating::SetSequence((CBaseAnimating *)this,nSequence);
  CFutbolCatcher::Spawn(&this->super_CFutbolCatcher);
  return;
}


/* _GLOBAL__I_sv_futbol_fake_force at 000b35c0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_sv_futbol_fake_force(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

