/* DWARF-guided pseudocode for game/server/portal2/point_changelevel.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* CEntityFactory<CInfoLandmark>::Create at 006d2f40 */

/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CInfoLandmark> * this, char
   * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CInfoLandmark>::Create(CEntityFactory<CInfoLandmark> *this,char *pClassName)

{
  CBaseEntity *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CInfoLandmark * pEnt@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CBaseEntity::operator_new(0x3c0);
  CBaseEntity::CBaseEntity(this_00,false);
  (this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x562b54);
  CBaseEntity::PostConstructor(this_00,pClassName);
  return &(this_00->m_Network).super_IServerNetworkable;
}


/* CInfoLandmark::~CInfoLandmark at 006d3030 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CInfoLandmark(CInfoLandmark * this, int __in_chrg) */

void __thiscall CInfoLandmark::~CInfoLandmark(CInfoLandmark *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x562a67);
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4d8c8f) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CInfoLandmark::~CInfoLandmark at 006d3080 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CInfoLandmark(CInfoLandmark * this, int __in_chrg) */

void __thiscall CInfoLandmark::~CInfoLandmark(CInfoLandmark *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x562a20);
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(extraout_ECX + 0x4d8c48) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,__in_chrg);
  return;
}


/* CInfoLandmark::Activate at 006d2fb0 */

/* DWARF original prototype: void Activate(CInfoLandmark * this) */

void __thiscall CInfoLandmark::Activate(CInfoLandmark *this)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  string_t *psVar2;
  IChangeInfoAccessor *pIVar3;
  int unaff_EBX;
  char *in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::Activate((CBaseEntity *)this);
  psVar2 = AllocPooledString((string_t *)(unaff_EBX + 0x33d993),in_stack_ffffffe8);
  if ((string_t *)(this->super_CPointEntity).super_CBaseEntity.m_iName.m_Value.pszValue != psVar2) {
    if ((this->super_CPointEntity).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false)
    {
      this_00 = &((this->super_CPointEntity).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar3->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CPointEntity).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CPointEntity).super_CBaseEntity.m_iName.m_Value.pszValue = (char *)psVar2;
  }
  return;
}


/* CEntityFactory<CPointChangelevel>::Create at 006d2eb0 */

/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CPointChangelevel> * this,
   char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CPointChangelevel>::Create(CEntityFactory<CPointChangelevel> *this,char *pClassName)

{
  CBaseEntity *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CPointChangelevel * pEnt@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CBaseEntity::operator_new(0x3d8);
  CBaseEntity::CBaseEntity(this_00,false);
  (this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x562884);
  this_00[1].super_IServerEntity.super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)0x0;
  this_00[1].m_pfnMoveDone.__delta = -1;
  this_00[1].m_pfnThink.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)0x0;
  (**(code **)(unaff_EBX + 0x5628f8))(this_00,pClassName);
  return &(this_00->m_Network).super_IServerNetworkable;
}


/* CPointChangelevel::~CPointChangelevel at 006d30b0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPointChangelevel(CPointChangelevel * this, int __in_chrg) */

void __thiscall CPointChangelevel::~CPointChangelevel(CPointChangelevel *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x562684);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnChangeLevel).super_CBaseEntityOutput,in_stack_ffffffd8);
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4d8c0c) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,in_stack_ffffffd8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CPointChangelevel::~CPointChangelevel at 006d3130 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPointChangelevel(CPointChangelevel * this, int __in_chrg) */

void __thiscall CPointChangelevel::~CPointChangelevel(CPointChangelevel *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(&UNK_00562604 + unaff_EBX);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnChangeLevel).super_CBaseEntityOutput,in_stack_ffffffd8);
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4d8b8c) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,__in_chrg);
  return;
}


/* DataMapInit<CPointChangelevel> at 000c4520 */

datamap_t * DataMapInit<CPointChangelevel>(CPointChangelevel *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&old_radius_damage.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) ==
      '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf4b4c);
    if (iVar1 != 0) {
      *(undefined **)(&DAT_00cf4b5c + unaff_EBX) = &UNK_0094c42e + unaff_EBX;
      *(undefined4 *)((int)&skill.super_ConCommandBase.m_pNext + unaff_EBX) = 0;
      *(undefined4 *)(&skill.super_ConCommandBase.m_bRegistered + unaff_EBX) = 0;
      *(undefined4 *)((int)&skill.super_ConCommandBase.m_pszName + unaff_EBX) = 0;
      *(undefined4 *)((int)&skill.super_ConCommandBase.m_pszHelpString + unaff_EBX) = 0;
      *(undefined4 *)((int)&skill.super_ConCommandBase.m_nFlags + unaff_EBX) = 0;
      *(undefined4 *)((int)&skill.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) = 0x11;
      ___cxa_guard_release(unaff_EBX + 0xcf4b4c);
      ___cxa_atexit(unaff_EBX + 0x8d685c,0,*(undefined4 *)(&DAT_00ae765c + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_InitSharedVars_00c07af8 + unaff_EBX) =
       *(undefined4 *)(&DAT_00ae7798 + unaff_EBX);
  if (*(char *)((int)&old_radius_damage.m_fnChangeCallbacks.m_pElements + unaff_EBX) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf4b54);
    if (iVar1 != 0) {
      *(undefined4 *)((int)&PTR_Event_Killed_00c07ba0 + unaff_EBX) =
           **(undefined4 **)(&DAT_00ae7678 + unaff_EBX);
      ___cxa_guard_release(unaff_EBX + 0xcf4b54);
    }
  }
  *(undefined4 *)((int)&PTR_Precache_00c07af0 + unaff_EBX) = 2;
  *(int *)((int)&PTR_Spawn_00c07aec + unaff_EBX) = (int)&PTR_RequiredEdictIndex_00c07b4c + unaff_EBX
  ;
  return (datamap_t *)((int)&PTR_Spawn_00c07aec + unaff_EBX);
}


/* __static_initialization_and_destruction_0 at 000c4640 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  IEntityFactoryDictionary *pIVar1;
  datamap_t *pdVar2;
  int unaff_EBX;
  longlong lVar3;
  
  lVar3 = ___i686_get_pc_thunk_bx();
  if (lVar3 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)&g_Language.super_ConCommandBase.m_pNext + unaff_EBX) = 0;
  *(undefined1 *)((int)&g_Language.super_ConCommandBase.m_pNext + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&g_Language.super_ConCommandBase.m_pNext + unaff_EBX + 2) = 0;
  *(undefined1 *)((int)&g_Language.super_ConCommandBase.m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)(&g_Language.super_ConCommandBase.m_bRegistered + unaff_EBX) = 0;
  *(undefined4 *)((int)&g_Language.super_ConCommandBase.m_pszName + unaff_EBX) = 0;
  *(undefined4 *)((int)&g_Language.super_ConCommandBase.m_pszHelpString + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&g_Language.super_ConCommandBase.m_nFlags + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&g_Language.super_IConVar._vptr_IConVar + unaff_EBX) = 0;
  *(undefined4 *)((int)&g_Language.m_pParent + unaff_EBX) = 0;
  *(undefined4 *)((int)&g_Language.m_pszDefaultValue + unaff_EBX) = 0;
  *(undefined4 *)((int)&g_Language.m_Value.m_pszString + unaff_EBX) = 0;
  *(undefined4 *)((int)&g_Language.m_Value.m_StringLength + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&g_Language.m_Value.m_fValue + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&g_Language.m_Value.m_nValue + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&g_Language.m_bHasMin + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&g_Language.m_fMinVal + unaff_EBX) = 0;
  (&g_Language.field_0x3f)[unaff_EBX] = 0;
  (&g_Language.m_bHasMax)[unaff_EBX] = false;
  (&g_Language.field_0x3d)[unaff_EBX] = 0;
  (&g_Language.field_0x3e)[unaff_EBX] = 0;
  *(undefined4 *)((int)&g_Language.m_fMaxVal + unaff_EBX) = 0;
  *(undefined1 *)((int)&g_Language.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&g_Language.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) = 0;
  *(undefined1 *)((int)&g_Language.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&g_Language.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&g_Language.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) = 0;
  *(undefined1 *)((int)&g_Language.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 3) = 1;
  *(undefined1 *)((int)&g_Language.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) = 0;
  *(undefined1 *)((int)&g_Language.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&g_Language.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&g_Language.m_fnChangeCallbacks.m_Size + unaff_EBX) = 1;
  *(undefined1 *)((int)&g_Language.m_fnChangeCallbacks.m_pElements + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&g_Language.m_fnChangeCallbacks.m_pElements + unaff_EBX) = 0;
  *(undefined1 *)((int)&g_Language.m_fnChangeCallbacks.m_pElements + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&g_Language.m_fnChangeCallbacks.m_pElements + unaff_EBX + 2) = 0;
  *(undefined4 *)(&DAT_00cf4a18 + unaff_EBX) = 2;
  (&DAT_00cf4a1f)[unaff_EBX] = 0;
  (&DAT_00cf4a1c)[unaff_EBX] = 0;
  (&DAT_00cf4a1d)[unaff_EBX] = 0;
  (&DAT_00cf4a1e)[unaff_EBX] = 0;
  *(undefined **)((int)&sk_autoaim_mode.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) =
       &UNK_00aeccdc + unaff_EBX;
  *(undefined **)((int)&sk_autoaim_mode.super_ConCommandBase.m_pNext + unaff_EBX) =
       &UNK_00b71424 + unaff_EBX;
  pIVar1 = EntityFactoryDictionary();
  (**pIVar1->_vptr_IEntityFactoryDictionary)(pIVar1,unaff_EBX + 0xcf4a24,&UNK_0094c328 + unaff_EBX);
  *(undefined **)(&sk_autoaim_mode.super_ConCommandBase.m_bRegistered + unaff_EBX) =
       &UNK_00b71424 + unaff_EBX;
  pIVar1 = EntityFactoryDictionary();
  (**pIVar1->_vptr_IEntityFactoryDictionary)(pIVar1,unaff_EBX + 0xcf4a28,&UNK_0094c33c + unaff_EBX);
  pdVar2 = DataMapInit<CPointChangelevel>((CPointChangelevel *)0x0);
  *(datamap_t **)((int)&PTR_OnRequestFocus_00c6b048 + unaff_EBX) = pdVar2;
  *(undefined **)((int)&sk_autoaim_mode.super_ConCommandBase.m_pszName + unaff_EBX) =
       &UNK_00b7179c + unaff_EBX;
  pIVar1 = EntityFactoryDictionary();
  (**pIVar1->_vptr_IEntityFactoryDictionary)(pIVar1,unaff_EBX + 0xcf4a2c,&UNK_0094c34f + unaff_EBX);
  return;
}


/* CPointChangelevel::GetDataDescMap at 006d2cf0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPointChangelevel * this) */

datamap_t * __thiscall CPointChangelevel::GetDataDescMap(CPointChangelevel *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5f9328);
}


/* CPointChangelevel::GetBaseMap at 006d2d00 */

datamap_t * CPointChangelevel::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4d8fc4);
}


/* __tcf_0 at 0099ad90 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41e306)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41e2fa) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41e306));
  }
  *(undefined4 *)(unaff_EBX + 0x41e306) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41e302)) {
    if (*(int *)(unaff_EBX + 0x41e2fa) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(&DAT_00210dfa + unaff_EBX) + 8))
                ((int *)**(undefined4 **)(&DAT_00210dfa + unaff_EBX),*(int *)(unaff_EBX + 0x41e2fa))
      ;
      *(undefined4 *)(unaff_EBX + 0x41e2fa) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e2fe) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41e2fa);
  *(int *)(unaff_EBX + 0x41e30a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41e302)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(&DAT_00210dfa + unaff_EBX) + 8))
                ((int *)**(undefined4 **)(&DAT_00210dfa + unaff_EBX),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41e2fa) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e2fe) = 0;
  }
  return;
}


/* CPointChangelevel::InputChangeLevel at 006d2d40 */

/* DWARF original prototype: void InputChangeLevel(CPointChangelevel * this, inputdata_t *
   inputdata) */

void __thiscall CPointChangelevel::InputChangeLevel(CPointChangelevel *this,inputdata_t *inputdata)

{
  char *pDest;
  char *pcVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  COutputEvent::FireOutput(&this->m_OnChangeLevel,inputdata->pActivator,(CBaseEntity *)this,0.0);
  pcVar1 = *(char **)(**(int **)(unaff_EBX + 0x4d8e52) + 0x3c);
  if (pcVar1 == (char *)0x0) {
    pcVar1 = (char *)(unaff_EBX + 0x2d8e0e);
  }
  V_strncpy((char *)(unaff_EBX + 0x61a94a),pcVar1,0x20);
  if ((inputdata->value).fieldType == FIELD_STRING) {
    pcVar1 = (char *)(inputdata->value).field_0.iVal;
    if (pcVar1 == (char *)0x0) {
      pcVar1 = (char *)(unaff_EBX + 0x2d8e0e);
    }
  }
  else {
    pcVar1 = variant_t::ToString(&inputdata->value);
  }
  pDest = (char *)(unaff_EBX + 0x61a96a);
  V_strncpy(pDest,pcVar1,0x20);
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d8e8e) + 0xa0))
            ((int *)**(undefined4 **)(unaff_EBX + 0x4d8e8e),pDest,0);
  (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x4d8e6a))
            ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x4d8e6a),pDest,unaff_EBX + 0x33dc06);
  return;
}


/* ChangeLevel_DestinationMapName at 006d2d10 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

char * ChangeLevel_DestinationMapName(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (char *)(extraout_ECX + 0x61a9a0);
}


/* ChangeLevel_OriginMapName at 006d2d20 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

char * ChangeLevel_OriginMapName(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (char *)(extraout_ECX + 0x61a970);
}


/* ChangeLevel_GetLandmarkName at 006d2d30 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

char * ChangeLevel_GetLandmarkName(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (char *)(extraout_ECX + 0x33dc1c);
}


/* _GLOBAL__I__ZN17CPointChangelevel9m_DataMapE at 000c4820 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN17CPointChangelevel9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

