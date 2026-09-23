/* DWARF-guided pseudocode for game/server/portal2/portal_race_checkpoint.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000c7050 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  undefined4 uVar1;
  datamap_t *pdVar2;
  IEntityFactoryDictionary *pIVar3;
  ConVar *unaff_EBX;
  char *unaff_EBP;
  char *unaff_ESI;
  longlong lVar4;
  int unaff_retaddr;
  ConVar *this_1;
  
  this_1 = unaff_EBX;
  lVar4 = ___i686_get_pc_thunk_bx();
  if (lVar4 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)&unaff_EBX[0x25abf].super_ConCommandBase.m_nFlags + 3) = 0;
  *(undefined1 *)&unaff_EBX[0x25abf].super_IConVar._vptr_IConVar = 0;
  *(undefined1 *)((int)&unaff_EBX[0x25abf].super_IConVar._vptr_IConVar + 1) = 0;
  *(undefined1 *)((int)&unaff_EBX[0x25abf].super_IConVar._vptr_IConVar + 2) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x25abf].super_IConVar._vptr_IConVar + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x25abf].m_pParent + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x25abf].m_pszDefaultValue + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x25abf].m_Value.m_pszString + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x25abf].m_Value.m_StringLength + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x25abf].m_Value.m_fValue + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x25abf].m_Value.m_nValue + 3) = 0;
  *(undefined4 *)&unaff_EBX[0x25abf].field_0x37 = 0;
  *(undefined4 *)((int)&unaff_EBX[0x25abf].m_fMinVal + 3) = 0x7f7fffff;
  *(undefined4 *)&unaff_EBX[0x25abf].field_0x3f = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x25abf].m_fMaxVal + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x25abf].m_fnChangeCallbacks.m_Memory.m_pMemory + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x25abf].m_fnChangeCallbacks.m_Memory.m_nAllocationCount + 3) = 0;
  *(undefined1 *)((int)&unaff_EBX[0x25abf].m_fnChangeCallbacks.m_Size + 2) = 0;
  *(undefined1 *)((int)&unaff_EBX[0x25abf].m_fnChangeCallbacks.m_Memory.m_nGrowSize + 3) = 0;
  *(undefined1 *)&unaff_EBX[0x25abf].m_fnChangeCallbacks.m_Size = 0;
  *(undefined1 *)((int)&unaff_EBX[0x25abf].m_fnChangeCallbacks.m_Size + 1) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x25abf].m_fnChangeCallbacks.m_Size + 3) = 0;
  *(undefined1 *)((int)&unaff_EBX[0x25ac0].super_ConCommandBase._vptr_ConCommandBase + 2) = 0;
  *(undefined1 *)((int)&unaff_EBX[0x25abf].m_fnChangeCallbacks.m_pElements + 3) = 0;
  *(undefined1 *)&unaff_EBX[0x25ac0].super_ConCommandBase._vptr_ConCommandBase = 0;
  *(undefined1 *)((int)&unaff_EBX[0x25ac0].super_ConCommandBase._vptr_ConCommandBase + 1) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x25ac0].super_ConCommandBase._vptr_ConCommandBase + 3) = 0;
  unaff_EBX[0x25ac0].super_ConCommandBase.field_0xa = 1;
  *(undefined1 *)((int)&unaff_EBX[0x25ac0].super_ConCommandBase.m_pNext + 3) = 0;
  unaff_EBX[0x25ac0].super_ConCommandBase.m_bRegistered = false;
  unaff_EBX[0x25ac0].super_ConCommandBase.field_0x9 = 0;
  *(undefined4 *)&unaff_EBX[0x25ac0].super_ConCommandBase.field_0xb = 1;
  *(undefined1 *)((int)&unaff_EBX[0x25ac0].super_ConCommandBase.m_pszHelpString + 2) = 0;
  *(undefined1 *)((int)&unaff_EBX[0x25ac0].super_ConCommandBase.m_pszName + 3) = 0;
  *(undefined1 *)&unaff_EBX[0x25ac0].super_ConCommandBase.m_pszHelpString = 0;
  *(undefined1 *)((int)&unaff_EBX[0x25ac0].super_ConCommandBase.m_pszHelpString + 1) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x25ac0].super_ConCommandBase.m_pszHelpString + 3) = 2;
  *(undefined1 *)((int)&unaff_EBX[0x25ac0].super_IConVar._vptr_IConVar + 2) = 0;
  *(undefined1 *)((int)&unaff_EBX[0x25ac0].super_ConCommandBase.m_nFlags + 3) = 0;
  *(undefined1 *)&unaff_EBX[0x25ac0].super_IConVar._vptr_IConVar = 0;
  *(undefined1 *)((int)&unaff_EBX[0x25ac0].super_IConVar._vptr_IConVar + 1) = 0;
  *(undefined1 **)((int)&unaff_EBX[0x25ac0].super_IConVar._vptr_IConVar + 3) =
       (undefined1 *)((int)&unaff_EBX[0x1fc08].super_ConCommandBase.m_pNext + 3);
  *(undefined1 *)((int)&unaff_EBX[0x24187].m_fnChangeCallbacks.m_pElements + 3) = 0xff;
  *(undefined1 *)&unaff_EBX[0x24188].super_ConCommandBase._vptr_ConCommandBase = 0x6a;
  *(undefined1 *)((int)&unaff_EBX[0x24188].super_ConCommandBase._vptr_ConCommandBase + 1) = 0;
  *(undefined1 *)((int)&unaff_EBX[0x24188].super_ConCommandBase._vptr_ConCommandBase + 2) = 0xff;
  *(undefined1 *)((int)&unaff_EBX[0x24188].super_ConCommandBase._vptr_ConCommandBase + 3) = 0;
  *(undefined1 *)&unaff_EBX[0x24188].super_ConCommandBase.m_pNext = 0xa5;
  *(undefined1 *)((int)&unaff_EBX[0x24188].super_ConCommandBase.m_pNext + 1) = 0xff;
  *(undefined1 *)((int)&unaff_EBX[0x24188].super_ConCommandBase.m_pNext + 2) = 0xff;
  ConVar::ConVar((ConVar *)((int)&unaff_EBX[0x24188].m_pParent + 3),&unaff_EBX[0x1b05b].field_0x3f,
                 (char *)((int)&unaff_EBX[0x19df3].m_fnChangeCallbacks.m_Memory.m_nGrowSize + 3),0,
                 this_1,unaff_ESI,unaff_EBP,unaff_retaddr);
  uVar1 = *(undefined4 *)((int)&unaff_EBX[0x1fb09].super_ConCommandBase.m_nFlags + 3);
  ___cxa_atexit((undefined1 *)
                ((int)&unaff_EBX[0x19af8].m_fnChangeCallbacks.m_Memory.m_nGrowSize + 3),0,uVar1);
  ConVar::ConVar((ConVar *)((int)&unaff_EBX[0x24189].m_Value.m_pszString + 3),
                 (char *)((int)&unaff_EBX[0x1b05c].super_ConCommandBase.m_pszName + 3),
                 (char *)((int)&unaff_EBX[0x1b05c].super_ConCommandBase.m_pNext + 3),0x4000,this_1,
                 unaff_ESI,unaff_EBP,unaff_retaddr);
  ___cxa_atexit((undefined1 *)((int)&unaff_EBX[0x19af8].m_Value.m_fValue + 3),0,uVar1);
  pdVar2 = DataMapInit<CPortalRaceCheckpoint>((CPortalRaceCheckpoint *)0x0);
  *(datamap_t **)((int)&unaff_EBX[0x2418a].m_Value.m_pszString + 3) = pdVar2;
  *(undefined1 **)((int)&unaff_EBX[0x25ac0].m_pParent + 3) =
       (undefined1 *)((int)&unaff_EBX[0x2146b].m_Value.m_pszString + 3);
  pIVar3 = EntityFactoryDictionary();
  (**pIVar3->_vptr_IEntityFactoryDictionary)
            (pIVar3,(undefined1 *)((int)&unaff_EBX[0x25ac0].m_pParent + 3),
             &unaff_EBX[0x1b05c].field_0x35);
  return;
}


/* __tcf_1 at 0099b5f0 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x394148),in_stack_00000008);
  return;
}


/* __tcf_2 at 0099b5d0 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3941c8),in_stack_00000008);
  return;
}


/* DataMapInit<CPortalRaceCheckpoint> at 000c6d40 */

datamap_t * DataMapInit<CPortalRaceCheckpoint>(CPortalRaceCheckpoint *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  char *pcVar4;
  size_t sVar5;
  int iVar6;
  undefined4 *puVar7;
  char *pcVar8;
  int iVar9;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((*(char *)((int)&vgui::ComboBox::GetKBMap::s_pMap + unaff_EBX + 2) == '\0') &&
     (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xcf353a), iVar3 != 0)) {
    *(int *)((int)&vgui::CheckButton::GetMessageMap()::s_pMap + unaff_EBX + 2) =
         unaff_EBX + 0x94a26e;
    *(undefined4 *)((int)&vgui::CheckButton::GetKBMap()::s_pMap + unaff_EBX + 2) = 0;
    *(undefined4 *)(&DAT_00cf3556 + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&vgui::CvarToggleCheckButton<ConVarRef>::GetMessageMap()::s_pMap + unaff_EBX + 2) = 0;
    *(undefined4 *)(&DAT_00cf355e + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&vgui::CvarToggleCheckButton<ConVarRef>::GetKBMap()::s_pMap + unaff_EBX + 2) = 0;
    *(undefined4 *)(&DAT_00cf354e + unaff_EBX) = 0x15;
    ___cxa_guard_release(unaff_EBX + 0xcf353a);
    ___cxa_atexit(unaff_EBX + 0x8d48c2,0,*(undefined4 *)(&DAT_00ae4e42 + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_UpdateTransmitState_00c06500 + unaff_EBX + 2) =
       *(undefined4 *)(&DAT_00ae4e4e + unaff_EBX);
  if (((&vgui::ComboBox::KB_ChainToMap()::chained)[unaff_EBX] == '\0') &&
     (iVar3 = ___cxa_guard_acquire(&vgui::ComboBox::KB_ChainToMap()::chained + unaff_EBX),
     iVar3 != 0)) {
    uVar1 = **(undefined4 **)(&DAT_00ae4e5e + unaff_EBX);
    *(undefined4 *)((int)&PTR_IsMoving_00c065e4 + unaff_EBX + 2) = uVar1;
    *(undefined4 *)((int)&PTR_OnEntityEvent_00c06624 + unaff_EBX + 2) = uVar1;
    pcVar4 = operator_new___(*(int *)(&DAT_00cf354e + unaff_EBX) + 0xf);
    _strcpy(pcVar4,*(char **)((int)&vgui::CheckButton::GetMessageMap()::s_pMap + unaff_EBX + 2));
    sVar5 = _strlen(pcVar4);
    builtin_strncpy(pcVar4 + sVar5,"ActivatedThink",0xf);
    iVar2 = *(int *)(&DAT_00cf355e + unaff_EBX);
    iVar3 = iVar2 + 1;
    if (*(int *)(&DAT_00cf3556 + unaff_EBX) < iVar3) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)
                 ((int)&vgui::CheckButton::GetKBMap()::s_pMap + unaff_EBX + 2),
                 iVar3 - *(int *)(&DAT_00cf3556 + unaff_EBX));
    }
    iVar6 = *(int *)(&DAT_00cf355e + unaff_EBX);
    *(int *)(&DAT_00cf355e + unaff_EBX) = iVar6 + 1;
    iVar9 = *(int *)((int)&vgui::CheckButton::GetKBMap()::s_pMap + unaff_EBX + 2);
    *(int *)((int)&vgui::CvarToggleCheckButton<ConVarRef>::GetKBMap()::s_pMap + unaff_EBX + 2) =
         iVar9;
    iVar6 = ((iVar6 + 1) - iVar2) + -1;
    if (0 < iVar6) {
      _V_memmove((void *)(iVar9 + iVar3 * 4),(void *)(iVar9 + iVar2 * 4),iVar6 * 4);
      iVar9 = *(int *)((int)&vgui::CheckButton::GetKBMap()::s_pMap + unaff_EBX + 2);
    }
    puVar7 = (undefined4 *)(iVar2 * 4 + iVar9);
    if (puVar7 != (undefined4 *)0x0) {
      *puVar7 = pcVar4;
    }
    *(char **)((int)&PTR_CanPushEntity_00c06754 + unaff_EBX + 2) = pcVar4;
    pcVar8 = operator_new___(*(int *)(&DAT_00cf354e + unaff_EBX) + 0x13);
    _strcpy(pcVar8,*(char **)((int)&vgui::CheckButton::GetMessageMap()::s_pMap + unaff_EBX + 2));
    sVar5 = _strlen(pcVar8);
    puVar7 = (undefined4 *)(unaff_EBX + 0x94a284);
    pcVar4 = pcVar8 + sVar5;
    for (iVar3 = 4; iVar3 != 0; iVar3 = iVar3 + -1) {
      *(undefined4 *)pcVar4 = *puVar7;
      puVar7 = puVar7 + 1;
      pcVar4 = pcVar4 + 4;
    }
    *(undefined2 *)pcVar4 = *(undefined2 *)puVar7;
    pcVar4[2] = *(char *)((int)puVar7 + 2);
    iVar2 = *(int *)(&DAT_00cf355e + unaff_EBX);
    iVar3 = iVar2 + 1;
    if (*(int *)(&DAT_00cf3556 + unaff_EBX) < iVar3) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)
                 ((int)&vgui::CheckButton::GetKBMap()::s_pMap + unaff_EBX + 2),
                 iVar3 - *(int *)(&DAT_00cf3556 + unaff_EBX));
    }
    iVar6 = *(int *)(&DAT_00cf355e + unaff_EBX);
    *(int *)(&DAT_00cf355e + unaff_EBX) = iVar6 + 1;
    iVar9 = *(int *)((int)&vgui::CheckButton::GetKBMap()::s_pMap + unaff_EBX + 2);
    *(int *)((int)&vgui::CvarToggleCheckButton<ConVarRef>::GetKBMap()::s_pMap + unaff_EBX + 2) =
         iVar9;
    iVar6 = ((iVar6 + 1) - iVar2) + -1;
    if (0 < iVar6) {
      _V_memmove((void *)(iVar9 + iVar3 * 4),(void *)(iVar9 + iVar2 * 4),iVar6 * 4);
      iVar9 = *(int *)((int)&vgui::CheckButton::GetKBMap()::s_pMap + unaff_EBX + 2);
    }
    puVar7 = (undefined4 *)(iVar2 * 4 + iVar9);
    if (puVar7 != (undefined4 *)0x0) {
      *puVar7 = pcVar8;
    }
    *(char **)((int)&PTR_NetworkStateChanged_m_flFriction_00c06794 + unaff_EBX + 2) = pcVar8;
    ___cxa_guard_release(&vgui::ComboBox::KB_ChainToMap()::chained + unaff_EBX);
  }
  *(undefined4 *)((int)&PTR_SetScriptOwnerEntity_00c064f8 + unaff_EBX + 2) = 10;
  *(int *)((int)&PTR_SetOwnerEntity_00c064f4 + unaff_EBX + 2) = unaff_EBX + 0xc06552;
  return (datamap_t *)((int)&PTR_SetOwnerEntity_00c064f4 + unaff_EBX + 2);
}


/* CPortalRaceCheckpoint::GetDataDescMap at 006d8020 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPortalRaceCheckpoint * this) */

datamap_t * __thiscall CPortalRaceCheckpoint::GetDataDescMap(CPortalRaceCheckpoint *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5f521c);
}


/* CPortalRaceCheckpoint::GetBaseMap at 006d8030 */

datamap_t * CPortalRaceCheckpoint::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4d3b64);
}


/* __tcf_0 at 0099b610 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41ec8e)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41ec82) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41ec8e));
  }
  *(undefined4 *)(unaff_EBX + 0x41ec8e) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41ec8a)) {
    if (*(int *)(unaff_EBX + 0x41ec82) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(&DAT_0021057a + unaff_EBX) + 8))
                ((int *)**(undefined4 **)(&DAT_0021057a + unaff_EBX),*(int *)(unaff_EBX + 0x41ec82))
      ;
      *(undefined4 *)(unaff_EBX + 0x41ec82) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41ec86) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41ec82);
  *(int *)(unaff_EBX + 0x41ec92) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41ec8a)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(&DAT_0021057a + unaff_EBX) + 8))
                ((int *)**(undefined4 **)(&DAT_0021057a + unaff_EBX),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41ec82) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41ec86) = 0;
  }
  return;
}


/* CPortalRaceCheckpoint::CPortalRaceCheckpoint at 006d82b0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CPortalRaceCheckpoint(CPortalRaceCheckpoint * this) */

void __thiscall CPortalRaceCheckpoint::CPortalRaceCheckpoint(CPortalRaceCheckpoint *this)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::CBaseAnimating(&this->super_CBaseAnimating);
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x55eec4);
  (this->m_OnCheckpointActivated).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnCheckpointActivated).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->m_OnCheckpointActivated).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_OnCheckpointReset).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnCheckpointReset).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->m_OnCheckpointReset).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_bCheckpointActive).m_Value = false;
  if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false)
  {
    this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if (this_00 != (CBaseEdict *)0x0) {
      this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
      pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
      pIVar2->m_iChangeInfoSerialNumber = 0;
    }
  }
  else {
    puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  this->m_bLocked = false;
  return;
}


/* CPortalRaceCheckpoint::CPortalRaceCheckpoint at 006d8390 */

/* DWARF original prototype: void CPortalRaceCheckpoint(CPortalRaceCheckpoint * this,
   CPortalRaceCheckpoint * this) */

void __thiscall
CPortalRaceCheckpoint::CPortalRaceCheckpoint
          (CPortalRaceCheckpoint *this,CPortalRaceCheckpoint *this_1)

{
  CPortalRaceCheckpoint(this);
  return;
}


/* CEntityFactory<CPortalRaceCheckpoint>::Create at 006d8de0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CPortalRaceCheckpoint> *
   this, char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CPortalRaceCheckpoint>::Create
          (CEntityFactory<CPortalRaceCheckpoint> *this,char *pClassName)

{
  CPortalRaceCheckpoint *this_00;
  
                    /* Unresolved local var: CPortalRaceCheckpoint * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x554);
  CPortalRaceCheckpoint::CPortalRaceCheckpoint(this_00);
  (*(this_00->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1d])(this_00,pClassName);
  return &(this_00->super_CBaseAnimating).super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CPortalRaceCheckpoint::~CPortalRaceCheckpoint at 006d8140 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPortalRaceCheckpoint(CPortalRaceCheckpoint * this, int
   __in_chrg) */

void __thiscall
CPortalRaceCheckpoint::~CPortalRaceCheckpoint(CPortalRaceCheckpoint *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x55f037);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnCheckpointReset).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnCheckpointActivated).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CPortalRaceCheckpoint::~CPortalRaceCheckpoint at 006d81c0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPortalRaceCheckpoint(CPortalRaceCheckpoint * this, int
   __in_chrg) */

void __thiscall
CPortalRaceCheckpoint::~CPortalRaceCheckpoint(CPortalRaceCheckpoint *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x55efb7);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnCheckpointReset).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnCheckpointActivated).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
  return;
}


/* CPortalRaceCheckpoint::~CPortalRaceCheckpoint at 006d8230 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPortalRaceCheckpoint(CPortalRaceCheckpoint * this, int
   __in_chrg) */

void __thiscall
CPortalRaceCheckpoint::~CPortalRaceCheckpoint(CPortalRaceCheckpoint *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x55ef47);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnCheckpointReset).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnCheckpointActivated).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
  return;
}


/* CPortalRaceCheckpoint::Precache at 006d80a0 */

/* DWARF original prototype: void Precache(CPortalRaceCheckpoint * this) */

void __thiscall CPortalRaceCheckpoint::Precache(CPortalRaceCheckpoint *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::Precache(&this->super_CBaseAnimating);
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x336f6c));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x33979e));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x338ee8));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x338ef9));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x339834));
  return;
}


/* CPortalRaceCheckpoint::Spawn at 006d83f0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "MoveCollide_t": Some values do not have unique names */
/* WARNING: Enum "MoveType_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(CPortalRaceCheckpoint * this) */

void __thiscall CPortalRaceCheckpoint::Spawn(CPortalRaceCheckpoint *this)

{
  undefined1 *puVar1;
  NetworkVar_m_Collision *this_00;
  float fVar2;
  uint8 uVar3;
  uint8 uVar4;
  uint8 uVar5;
  CBaseEdict *pCVar6;
  char *szContext;
  BASEPTR func;
  IChangeInfoAccessor *pIVar7;
  int unaff_EBX;
  longdouble lVar8;
  Vector local_40;
  Vector local_34;
  Vector local_28 [2];
  
                    /* Unresolved local var: float flModelScale@[???]
                       Unresolved local var: float fScaledExtent@[???]
                       Unresolved local var: Vector vModelOffset@[???] */
  ___i686_get_pc_thunk_bx();
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1a])(this);
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1b])(this,unaff_EBX + 0x336c1a);
  CBaseEntity::SetMoveType((CBaseEntity *)this,MOVETYPE_NOCLIP,MOVECOLLIDE_DEFAULT);
  this_00 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Collision;
  CCollisionProperty::SetSolid(&this_00->super_CCollisionProperty,SOLID_BBOX);
  CCollisionProperty::SetSolidFlags(&this_00->super_CCollisionProperty,0xc);
  CBaseEntity::SetCollisionGroup((CBaseEntity *)this,5);
  uVar3 = (&DAT_00657328)[unaff_EBX];
  uVar4 = *(uint8 *)(unaff_EBX + 0x657327);
  uVar5 = *(uint8 *)(unaff_EBX + 0x657326);
  if (uVar5 != (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
               super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      pCVar6 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar6 != (CBaseEdict *)0x0) {
        pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 0x101;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar6);
        pIVar7->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
    super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r = uVar5;
  }
  if (uVar4 != (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
               super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      pCVar6 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar6 != (CBaseEdict *)0x0) {
        pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 0x101;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar6);
        pIVar7->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
    super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g = uVar4;
  }
  if (uVar3 != (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
               super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      pCVar6 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar6 != (CBaseEdict *)0x0) {
        pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 0x101;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar6);
        pIVar7->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
    super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b = uVar3;
  }
  fVar2 = *(float *)(*(int *)(unaff_EBX + 0x6573be) + 0x2c);
  CBaseAnimating::SetModelScale(&this->super_CBaseAnimating,2.0,0.0,HIERARCHICAL_MODEL_SCALE);
  if (((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
                    /* Unresolved local var: Vector res@[???] */
  local_28[0].x = (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.x + 0.0;
  local_28[0].y = (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.y + 0.0;
  local_28[0].z =
       -(fVar2 * *(float *)(unaff_EBX + 0x3525a2)) +
       (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.z;
  CBaseEntity::SetAbsOrigin((CBaseEntity *)this,local_28);
  local_40.x = 50.0;
  local_40.y = 50.0;
  local_40.z = 100.0;
  local_34.x = -50.0;
  local_34.y = -50.0;
  local_34.z = 0.0;
  CBaseEntity::SetSize((CBaseEntity *)this,&local_34,&local_40);
  szContext = *(char **)(unaff_EBX + 0x5f4e42);
  fVar2 = *(float *)(**(int **)(unaff_EBX + 0x4d37a2) + 0xc);
  lVar8 = (longdouble)_RandomFloat(0,0x3f000000);
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x2ee);
  CBaseEntity::ThinkSet((CBaseEntity *)this,func,fVar2 + (float)lVar8,szContext);
  CBaseAnimating::Spawn(&this->super_CBaseAnimating);
  return;
}


/* CPortalRaceCheckpoint::StartSpinningThink at 006d8110 */

/* DWARF original prototype: void StartSpinningThink(CPortalRaceCheckpoint * this) */

void __thiscall CPortalRaceCheckpoint::StartSpinningThink(CPortalRaceCheckpoint *this)

{
  AngularImpulse local_18;
  
  local_18.x = 0.0;
  local_18.y = 0.0;
  local_18.z = 180.0;
  CBaseEntity::ApplyLocalAngularVelocityImpulse((CBaseEntity *)this,&local_18);
  return;
}


/* CPortalRaceCheckpoint::OnCheckpointActivated at 006d89d0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void OnCheckpointActivated(CPortalRaceCheckpoint * this) */

void __thiscall CPortalRaceCheckpoint::OnCheckpointActivated(CPortalRaceCheckpoint *this)

{
  undefined1 *puVar1;
  uint8 uVar2;
  uint8 uVar3;
  uint8 uVar4;
  CBaseEdict *pCVar5;
  IChangeInfoAccessor *pIVar6;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((this->m_bCheckpointActive).m_Value == false) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      pCVar5 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar5 != (CBaseEdict *)0x0) {
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
        pIVar6 = CBaseEdict::GetChangeAccessor(pCVar5);
        pIVar6->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bCheckpointActive).m_Value = true;
  }
  CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x338e6d),0.0,(float *)0x0);
  this->m_fLastTimerSound =
       *(float *)(**(int **)(unaff_EBX + 0x4d31c3) + 0xc) - *(float *)(unaff_EBX + 0x351933);
  uVar2 = *(uint8 *)(unaff_EBX + 0x656d45);
  uVar3 = *(uint8 *)(unaff_EBX + 0x656d44);
  uVar4 = *(uint8 *)(unaff_EBX + 0x656d43);
  if (uVar4 != (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
               super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      pCVar5 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar5 != (CBaseEdict *)0x0) {
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
        pIVar6 = CBaseEdict::GetChangeAccessor(pCVar5);
        pIVar6->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
    super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r = uVar4;
  }
  if (uVar3 != (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
               super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      pCVar5 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar5 != (CBaseEdict *)0x0) {
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
        pIVar6 = CBaseEdict::GetChangeAccessor(pCVar5);
        pIVar6->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
    super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g = uVar3;
  }
  if (uVar2 != (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
               super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      pCVar5 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar5 != (CBaseEdict *)0x0) {
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
        pIVar6 = CBaseEdict::GetChangeAccessor(pCVar5);
        pIVar6->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
    super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b = uVar2;
  }
  COutputEvent::FireOutput
            (&this->m_OnCheckpointActivated,(CBaseEntity *)this,(CBaseEntity *)this,0.0);
  return;
}


/* CPortalRaceCheckpoint::OnCheckpointReset at 006d86d0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void OnCheckpointReset(CPortalRaceCheckpoint * this, bool bFireOutput)
    */

void __thiscall
CPortalRaceCheckpoint::OnCheckpointReset(CPortalRaceCheckpoint *this,bool bFireOutput)

{
  undefined1 *puVar1;
  uint8 uVar2;
  uint8 uVar3;
  code cVar4;
  CBaseEdict *pCVar5;
  IChangeInfoAccessor *pIVar6;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((this->m_bCheckpointActive).m_Value != false) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      pCVar5 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar5 != (CBaseEdict *)0x0) {
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
        pIVar6 = CBaseEdict::GetChangeAccessor(pCVar5);
        pIVar6->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bCheckpointActive).m_Value = false;
  }
  CBaseEntity::ThinkSet((CBaseEntity *)this,(BASEPTR)0x0,0.0,(char *)0x0);
  if (bFireOutput) {
    CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x3388b0),0.0,(float *)0x0);
    COutputEvent::FireOutput(&this->m_OnCheckpointReset,(CBaseEntity *)this,(CBaseEntity *)this,0.0)
    ;
    uVar2 = *(uint8 *)(unaff_EBX + 0x657042);
    uVar3 = *(uint8 *)(unaff_EBX + 0x657041);
    cVar4 = CPointSpotlight::CreateEfficientSpotlight[unaff_EBX];
    if (cVar4 != (code)(this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
                       super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.
                       r) {
      if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
          false) {
        pCVar5 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if (pCVar5 != (CBaseEdict *)0x0) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
          pIVar6 = CBaseEdict::GetChangeAccessor(pCVar5);
          pIVar6->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
      super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r = (uint8)cVar4;
    }
    if (uVar3 != (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
                 super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g) {
      if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
          false) {
        pCVar5 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if (pCVar5 != (CBaseEdict *)0x0) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
          pIVar6 = CBaseEdict::GetChangeAccessor(pCVar5);
          pIVar6->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
      super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g = uVar3;
    }
    if (uVar2 != (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
                 super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b) {
      if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
          false) {
        pCVar5 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if (pCVar5 != (CBaseEdict *)0x0) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
          pIVar6 = CBaseEdict::GetChangeAccessor(pCVar5);
          pIVar6->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
      super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b = uVar2;
    }
  }
  return;
}


/* CPortalRaceCheckpoint::StartTouch at 006d8cb0 */

/* DWARF original prototype: void StartTouch(CPortalRaceCheckpoint * this, CBaseEntity * pOther) */

void __thiscall CPortalRaceCheckpoint::StartTouch(CPortalRaceCheckpoint *this,CBaseEntity *pOther)

{
  int *piVar1;
  BASEPTR func;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  iVar2 = (*(pOther->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
            _vptr_IHandleEntity[0x55])(pOther);
  if ((char)iVar2 != '\0') {
    if (this->m_bLocked == false) {
      func.__delta = 0;
      func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x3c4);
      CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
      piVar1 = *(int **)(unaff_EBX + 0x4d2edc);
      CBaseEntity::SetNextThink
                ((CBaseEntity *)this,*(float *)(unaff_EBX + 0x351650) + *(float *)(*piVar1 + 0xc),
                 (char *)0x0);
      this->m_fActivatedTime = *(float *)(*piVar1 + 0xc);
      OnCheckpointActivated(this);
      return;
    }
    CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x3382e1),0.0,(float *)0x0);
  }
  return;
}


/* CPortalRaceCheckpoint::ActivateCheckpoint at 006d8b90 */

/* DWARF original prototype: void ActivateCheckpoint(CPortalRaceCheckpoint * this) */

void __thiscall CPortalRaceCheckpoint::ActivateCheckpoint(CPortalRaceCheckpoint *this)

{
  int *piVar1;
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x2a4);
  CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
  piVar1 = *(int **)(unaff_EBX + 0x4d2ffc);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,*(float *)(unaff_EBX + 0x351770) + *(float *)(*piVar1 + 0xc),
             (char *)0x0);
  this->m_fActivatedTime = *(float *)(*piVar1 + 0xc);
  OnCheckpointActivated(this);
  return;
}


/* CPortalRaceCheckpoint::InputActivate at 006d8c20 */

/* DWARF original prototype: void InputActivate(CPortalRaceCheckpoint * this, inputdata_t *
   inputData) */

void __thiscall
CPortalRaceCheckpoint::InputActivate(CPortalRaceCheckpoint *this,inputdata_t *inputData)

{
  int *piVar1;
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x334);
  CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
  piVar1 = *(int **)(unaff_EBX + 0x4d2f6c);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,*(float *)(unaff_EBX + 0x3516e0) + *(float *)(*piVar1 + 0xc),
             (char *)0x0);
  this->m_fActivatedTime = *(float *)(*piVar1 + 0xc);
  OnCheckpointActivated(this);
  return;
}


/* CPortalRaceCheckpoint::ActivatedThink at 006d8900 */

/* DWARF original prototype: void ActivatedThink(CPortalRaceCheckpoint * this) */

void __thiscall CPortalRaceCheckpoint::ActivatedThink(CPortalRaceCheckpoint *this)

{
  int *piVar1;
  int unaff_EBX;
  float fVar2;
  
  ___i686_get_pc_thunk_bx();
  piVar1 = *(int **)(unaff_EBX + 0x4d328c);
  fVar2 = *(float *)(*piVar1 + 0xc) - this->m_fActivatedTime;
  if (this->m_fResetTime <= fVar2 && fVar2 != this->m_fResetTime) {
    OnCheckpointReset(this,true);
  }
  else {
    fVar2 = *(float *)(*piVar1 + 0xc) - this->m_fLastTimerSound;
    if (*(float *)(unaff_EBX + 0x3519fc) <= fVar2 && fVar2 != *(float *)(unaff_EBX + 0x3519fc)) {
      CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x338fcc),0.0,(float *)0x0);
      this->m_fLastTimerSound = *(float *)(*piVar1 + 0xc);
    }
    CBaseEntity::SetNextThink
              ((CBaseEntity *)this,*(float *)(unaff_EBX + 0x351a00) + *(float *)(*piVar1 + 0xc),
               (char *)0x0);
  }
  return;
}


/* CPortalRaceCheckpoint::Lock at 006d8050 */

/* DWARF original prototype: void Lock(CPortalRaceCheckpoint * this) */

void __thiscall CPortalRaceCheckpoint::Lock(CPortalRaceCheckpoint *this)

{
  QAngle local_18;
  
                    /* Unresolved local var: QAngle angImpulse@[???] */
  local_18.x = 0.0;
  local_18.y = 0.0;
  local_18.z = 0.0;
  CBaseEntity::SetLocalAngularVelocity((CBaseEntity *)this,&local_18);
  this->m_bLocked = true;
  return;
}


/* CPortalRaceCheckpoint::InputLock at 006d83a0 */

/* DWARF original prototype: void InputLock(CPortalRaceCheckpoint * this, inputdata_t * inputData)
    */

void __thiscall CPortalRaceCheckpoint::InputLock(CPortalRaceCheckpoint *this,inputdata_t *inputData)

{
  QAngle local_18;
  
                    /* Unresolved local var: QAngle angImpulse@[???] */
  local_18.x = 0.0;
  local_18.y = 0.0;
  local_18.z = 0.0;
  CBaseEntity::SetLocalAngularVelocity((CBaseEntity *)this,&local_18);
  this->m_bLocked = true;
  return;
}


/* CPortalRaceCheckpoint::Unlock at 006d8040 */

/* DWARF original prototype: void Unlock(CPortalRaceCheckpoint * this) */

void __thiscall CPortalRaceCheckpoint::Unlock(CPortalRaceCheckpoint *this)

{
  this->m_bLocked = false;
  return;
}


/* CPortalRaceCheckpoint::InputUnlock at 006d82a0 */

/* DWARF original prototype: void InputUnlock(CPortalRaceCheckpoint * this, inputdata_t * inputData)
    */

void __thiscall
CPortalRaceCheckpoint::InputUnlock(CPortalRaceCheckpoint *this,inputdata_t *inputData)

{
  this->m_bLocked = false;
  return;
}


/* CPortalRaceCheckpoint::InputCancelActivate at 006d88f0 */

/* DWARF original prototype: void InputCancelActivate(CPortalRaceCheckpoint * this, inputdata_t *
   inputData) */

void __thiscall
CPortalRaceCheckpoint::InputCancelActivate(CPortalRaceCheckpoint *this,inputdata_t *inputData)

{
  OnCheckpointReset(this,false);
  return;
}


/* _GLOBAL__I_checkpointActiveColor at 000c72a0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_checkpointActiveColor(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

