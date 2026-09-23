/* DWARF-guided pseudocode for game/server/portal2/portal_race_checkpoint.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000b1ea0 */

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
  unaff_EBX[0x279b5].field_0x37 = 0;
  *(undefined1 *)&unaff_EBX[0x279b5].m_fMinVal = 0;
  *(undefined1 *)((int)&unaff_EBX[0x279b5].m_fMinVal + 1) = 0;
  *(undefined1 *)((int)&unaff_EBX[0x279b5].m_fMinVal + 2) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x279b5].m_fMinVal + 3) = 0;
  *(undefined4 *)&unaff_EBX[0x279b5].field_0x3f = 0;
  *(undefined4 *)((int)&unaff_EBX[0x279b5].m_fMaxVal + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x279b5].m_fnChangeCallbacks.m_Memory.m_pMemory + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x279b5].m_fnChangeCallbacks.m_Memory.m_nAllocationCount + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x279b5].m_fnChangeCallbacks.m_Memory.m_nGrowSize + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x279b5].m_fnChangeCallbacks.m_Size + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x279b5].m_fnChangeCallbacks.m_pElements + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x279b6].super_ConCommandBase._vptr_ConCommandBase + 3) =
       0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x279b6].super_ConCommandBase.m_pNext + 3) = 0x7f7fffff;
  *(undefined4 *)&unaff_EBX[0x279b6].super_ConCommandBase.field_0xb = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x279b6].super_ConCommandBase.m_pszName + 3) = 0x7f7fffff;
  *(undefined1 **)((int)&unaff_EBX[0x279b6].super_ConCommandBase.m_pszHelpString + 3) =
       &unaff_EBX[0x21c0c].field_0x37;
  *(undefined1 *)((int)&unaff_EBX[0x26158].m_fnChangeCallbacks.m_Memory.m_nGrowSize + 3) = 0xff;
  *(undefined1 *)&unaff_EBX[0x26158].m_fnChangeCallbacks.m_Size = 0x6a;
  *(undefined1 *)((int)&unaff_EBX[0x26158].m_fnChangeCallbacks.m_Size + 1) = 0;
  *(undefined1 *)((int)&unaff_EBX[0x26158].m_fnChangeCallbacks.m_Size + 2) = 0xff;
  *(undefined1 *)((int)&unaff_EBX[0x26158].m_fnChangeCallbacks.m_Size + 3) = 0;
  *(undefined1 *)&unaff_EBX[0x26158].m_fnChangeCallbacks.m_pElements = 0xa5;
  *(undefined1 *)((int)&unaff_EBX[0x26158].m_fnChangeCallbacks.m_pElements + 1) = 0xff;
  *(undefined1 *)((int)&unaff_EBX[0x26158].m_fnChangeCallbacks.m_pElements + 2) = 0xff;
  ConVar::ConVar((ConVar *)((int)&unaff_EBX[0x26159].super_ConCommandBase.m_nFlags + 3),
                 (char *)((int)&unaff_EBX[0x1d1ab].super_ConCommandBase.m_pszHelpString + 3),
                 (char *)((int)&unaff_EBX[0x1bf72].m_pParent + 3),0,this_1,unaff_ESI,unaff_EBP,
                 unaff_retaddr);
  uVar1 = *(undefined4 *)((int)&unaff_EBX[0x21b0d].m_Value.m_StringLength + 3);
  ___cxa_atexit((undefined1 *)((int)&unaff_EBX[0x1bc81].super_ConCommandBase.m_nFlags + 3),0,uVar1);
  ConVar::ConVar((ConVar *)((int)&unaff_EBX[0x2615a].m_pParent + 3),
                 (char *)((int)&unaff_EBX[0x1d1ab].m_fMinVal + 3),
                 (char *)((int)&unaff_EBX[0x1d1ab].m_Value.m_nValue + 3),0x4000,this_1,unaff_ESI,
                 unaff_EBP,unaff_retaddr);
  ___cxa_atexit((undefined1 *)
                ((int)&unaff_EBX[0x1bc80].m_fnChangeCallbacks.m_Memory.m_nGrowSize + 3),0,uVar1);
  pdVar2 = DataMapInit<CPortalRaceCheckpoint>((CPortalRaceCheckpoint *)0x0);
  *(datamap_t **)((int)&unaff_EBX[0x2615b].m_pParent + 3) = pdVar2;
  *(undefined1 **)((int)&unaff_EBX[0x279b6].super_ConCommandBase.m_nFlags + 3) =
       (undefined1 *)((int)&unaff_EBX[0x23416].m_Value.m_fValue + 3);
  pIVar3 = EntityFactoryDictionary();
  (**pIVar3->_vptr_IEntityFactoryDictionary)
            (pIVar3,(undefined1 *)((int)&unaff_EBX[0x279b6].super_ConCommandBase.m_nFlags + 3),
             &unaff_EBX[0x1d1ac].super_ConCommandBase.field_0x9);
  return;
}


/* __tcf_1 at 00a3eb20 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38aa38),in_stack_00000008);
  return;
}


/* __tcf_2 at 00a3eb00 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38aab8),in_stack_00000008);
  return;
}


/* DataMapInit<CPortalRaceCheckpoint> at 000b1b90 */

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
  if ((*(char *)((int)&vismon_trace_limit.m_pszDefaultValue + unaff_EBX + 2) == '\0') &&
     (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xd9d9c2), iVar3 != 0)) {
    *(int *)((int)&vismon_trace_limit.m_Value.m_nValue + unaff_EBX + 2) = unaff_EBX + 0xa015c2;
    *(undefined4 *)((int)&vismon_trace_limit.m_fMinVal + unaff_EBX + 2) = 0;
    *(undefined4 *)(&vismon_trace_limit.field_0x3e + unaff_EBX) = 0;
    *(undefined4 *)((int)&vismon_trace_limit.m_fMaxVal + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&vismon_trace_limit.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 2)
         = 0;
    *(undefined4 *)
     ((int)&vismon_trace_limit.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX + 2) = 0;
    *(undefined4 *)(&vismon_trace_limit.field_0x36 + unaff_EBX) = 0x15;
    ___cxa_guard_release(unaff_EBX + 0xd9d9c2);
    ___cxa_atexit(unaff_EBX + 0x98cfa2,0,*(undefined4 *)(&DAT_00b94fb6 + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_VPhysicsIsFlesh_00cb5950 + unaff_EBX + 2) =
       *(undefined4 *)(&DAT_00b94fc2 + unaff_EBX);
  if ((*(char *)((int)&vismon_trace_limit.m_Value.m_StringLength + unaff_EBX + 2) == '\0') &&
     (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xd9d9ca), iVar3 != 0)) {
    uVar1 = **(undefined4 **)(&DAT_00b94fd6 + unaff_EBX);
    *(undefined4 *)((int)&PTR_SetModel_00cb5a34 + unaff_EBX + 2) = uVar1;
    *(undefined4 *)((int)&PTR_DrawDebugTextOverlays_00cb5a74 + unaff_EBX + 2) = uVar1;
    pcVar4 = operator_new___(*(int *)(&vismon_trace_limit.field_0x36 + unaff_EBX) + 0xf);
    _strcpy(pcVar4,*(char **)((int)&vismon_trace_limit.m_Value.m_nValue + unaff_EBX + 2));
    sVar5 = _strlen(pcVar4);
    builtin_strncpy(pcVar4 + sVar5,"ActivatedThink",0xf);
    iVar2 = *(int *)((int)&vismon_trace_limit.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 2
                    );
    iVar3 = iVar2 + 1;
    if (*(int *)(&vismon_trace_limit.field_0x3e + unaff_EBX) < iVar3) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)((int)&vismon_trace_limit.m_fMinVal + unaff_EBX + 2),
                 iVar3 - *(int *)(&vismon_trace_limit.field_0x3e + unaff_EBX));
    }
    iVar6 = *(int *)((int)&vismon_trace_limit.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 2
                    ) + 1;
    *(int *)((int)&vismon_trace_limit.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 2) =
         iVar6;
    iVar9 = *(int *)((int)&vismon_trace_limit.m_fMinVal + unaff_EBX + 2);
    *(int *)((int)&vismon_trace_limit.m_fnChangeCallbacks.m_Memory.m_nAllocationCount +
            unaff_EBX + 2) = iVar9;
    iVar6 = (iVar6 - iVar2) + -1;
    if (0 < iVar6) {
      _V_memmove((void *)(iVar9 + iVar3 * 4),(void *)(iVar9 + iVar2 * 4),iVar6 * 4);
      iVar9 = *(int *)((int)&vismon_trace_limit.m_fMinVal + unaff_EBX + 2);
    }
    puVar7 = (undefined4 *)(iVar2 * 4 + iVar9);
    if (puVar7 != (undefined4 *)0x0) {
      *puVar7 = pcVar4;
    }
    *(char **)((int)&PTR_Respawn_00cb5ba4 + unaff_EBX + 2) = pcVar4;
    pcVar8 = operator_new___(*(int *)(&vismon_trace_limit.field_0x36 + unaff_EBX) + 0x13);
    _strcpy(pcVar8,*(char **)((int)&vismon_trace_limit.m_Value.m_nValue + unaff_EBX + 2));
    sVar5 = _strlen(pcVar8);
    puVar7 = (undefined4 *)(unaff_EBX + 0xa015d8);
    pcVar4 = pcVar8 + sVar5;
    for (iVar3 = 4; iVar3 != 0; iVar3 = iVar3 + -1) {
      *(undefined4 *)pcVar4 = *puVar7;
      puVar7 = puVar7 + 1;
      pcVar4 = pcVar4 + 4;
    }
    *(undefined2 *)pcVar4 = *(undefined2 *)puVar7;
    pcVar4[2] = *(char *)((int)puVar7 + 2);
    iVar2 = *(int *)((int)&vismon_trace_limit.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 2
                    );
    iVar3 = iVar2 + 1;
    if (*(int *)(&vismon_trace_limit.field_0x3e + unaff_EBX) < iVar3) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)((int)&vismon_trace_limit.m_fMinVal + unaff_EBX + 2),
                 iVar3 - *(int *)(&vismon_trace_limit.field_0x3e + unaff_EBX));
    }
    iVar6 = *(int *)((int)&vismon_trace_limit.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 2
                    ) + 1;
    *(int *)((int)&vismon_trace_limit.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 2) =
         iVar6;
    iVar9 = *(int *)((int)&vismon_trace_limit.m_fMinVal + unaff_EBX + 2);
    *(int *)((int)&vismon_trace_limit.m_fnChangeCallbacks.m_Memory.m_nAllocationCount +
            unaff_EBX + 2) = iVar9;
    iVar6 = (iVar6 - iVar2) + -1;
    if (0 < iVar6) {
      _V_memmove((void *)(iVar9 + iVar3 * 4),(void *)(iVar9 + iVar2 * 4),iVar6 * 4);
      iVar9 = *(int *)((int)&vismon_trace_limit.m_fMinVal + unaff_EBX + 2);
    }
    puVar7 = (undefined4 *)(iVar2 * 4 + iVar9);
    if (puVar7 != (undefined4 *)0x0) {
      *puVar7 = pcVar8;
    }
    *(char **)((int)&PTR_SetDamage_00cb5be4 + unaff_EBX + 2) = pcVar8;
    ___cxa_guard_release(unaff_EBX + 0xd9d9ca);
  }
  *(undefined4 *)((int)&PTR_UpdatePhysicsShadowToCurrentPosition_00cb5948 + unaff_EBX + 2) = 10;
  *(int *)((int)&PTR_VPhysicsFriction_00cb5944 + unaff_EBX + 2) = unaff_EBX + 0xcb59a2;
  return (datamap_t *)((int)&PTR_VPhysicsFriction_00cb5944 + unaff_EBX + 2);
}


/* CPortalRaceCheckpoint::GetDataDescMap at 007469b0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPortalRaceCheckpoint * this) */

datamap_t * __thiscall CPortalRaceCheckpoint::GetDataDescMap(CPortalRaceCheckpoint *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x620b2c);
}


/* CPortalRaceCheckpoint::GetBaseMap at 007469c0 */

datamap_t * CPortalRaceCheckpoint::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x500198);
}


/* __tcf_0 at 00a3eb40 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x410a36)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x410a2a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x410a36));
  }
  *(undefined4 *)(unaff_EBX + 0x410a36) = 0;
  if (-1 < *(int *)(CBaseEntity::Save + unaff_EBX + 2)) {
    if (*(int *)(unaff_EBX + 0x410a2a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20800e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20800e),*(int *)(unaff_EBX + 0x410a2a));
      *(undefined4 *)(unaff_EBX + 0x410a2a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x410a2e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x410a2a);
  *(int *)(unaff_EBX + 0x410a3a) = iVar1;
  if (-1 < *(int *)(CBaseEntity::Save + unaff_EBX + 2)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20800e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20800e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x410a2a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x410a2e) = 0;
  }
  return;
}


/* CPortalRaceCheckpoint::CPortalRaceCheckpoint at 007473d0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CPortalRaceCheckpoint(CPortalRaceCheckpoint * this) */

void __thiscall CPortalRaceCheckpoint::CPortalRaceCheckpoint(CPortalRaceCheckpoint *this)

{
  undefined1 *puVar1;
  uint uVar2;
  int iVar3;
  ushort uVar4;
  CBaseEdict *this_00;
  int *piVar5;
  ushort *puVar6;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  int unaff_EBX;
  uint uVar9;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::CBaseAnimating(&this->super_CBaseAnimating);
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x588ec4);
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
    if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
      this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
      pIVar7 = CBaseEdict::GetChangeAccessor(this_00);
      piVar5 = *(int **)(unaff_EBX + 0x4ff784);
      puVar6 = (ushort *)*piVar5;
      if (pIVar7->m_iChangeInfoSerialNumber == *puVar6) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        uVar9 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar4 = puVar6[uVar9 * 0x14 + 0x14];
        if (uVar4 == 0) {
LAB_00747556:
          puVar6[(uint)uVar4 + uVar9 * 0x14 + 1] = 0x550;
          puVar6[uVar9 * 0x14 + 0x14] = uVar4 + 1;
        }
        else if (puVar6[uVar9 * 0x14 + 1] != 0x550) {
          uVar8 = 0;
          do {
            uVar2 = uVar8 + 1;
            uVar8 = uVar2 & 0xffff;
            if ((ushort)uVar2 == uVar4) {
              if (uVar4 != 0x13) goto LAB_00747556;
              goto LAB_00747578;
            }
          } while (puVar6[uVar9 * 0x14 + uVar8 + 1] != 0x550);
        }
      }
      else if (puVar6[0x7d1] == 100) {
LAB_00747578:
        pIVar7->m_iChangeInfoSerialNumber = 0;
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar7->m_iChangeInfo = puVar6[0x7d1];
        *(short *)(*piVar5 + 0xfa2) = *(short *)(*piVar5 + 0xfa2) + 1;
        pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
        iVar3 = *piVar5 + (uint)pIVar7->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar3 + 2) = 0x550;
        *(undefined2 *)(iVar3 + 0x28) = 1;
      }
    }
  }
  else {
    puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  this->m_bLocked = false;
  return;
}


/* CPortalRaceCheckpoint::CPortalRaceCheckpoint at 007475c0 */

/* DWARF original prototype: void CPortalRaceCheckpoint(CPortalRaceCheckpoint * this,
   CPortalRaceCheckpoint * this) */

void __thiscall
CPortalRaceCheckpoint::CPortalRaceCheckpoint
          (CPortalRaceCheckpoint *this,CPortalRaceCheckpoint *this_1)

{
  CPortalRaceCheckpoint(this);
  return;
}


/* CEntityFactory<CPortalRaceCheckpoint>::Create at 007486b0 */

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


/* CPortalRaceCheckpoint::~CPortalRaceCheckpoint at 00746ad0 */

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
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5897c7);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnCheckpointReset).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnCheckpointActivated).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CPortalRaceCheckpoint::~CPortalRaceCheckpoint at 00746b50 */

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
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x589747);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnCheckpointReset).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnCheckpointActivated).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
  return;
}


/* CPortalRaceCheckpoint::~CPortalRaceCheckpoint at 00746bc0 */

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
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5896d7);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnCheckpointReset).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnCheckpointActivated).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
  return;
}


/* CPortalRaceCheckpoint::Precache at 00746a30 */

/* DWARF original prototype: void Precache(CPortalRaceCheckpoint * this) */

void __thiscall CPortalRaceCheckpoint::Precache(CPortalRaceCheckpoint *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::Precache(&this->super_CBaseAnimating);
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x36a468));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x36cf2e));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x36c6fc));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x36c70d));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x36cf18));
  return;
}


/* CPortalRaceCheckpoint::Spawn at 00746c90 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "MoveCollide_t": Some values do not have unique names */
/* WARNING: Enum "MoveType_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(CPortalRaceCheckpoint * this) */

void __thiscall CPortalRaceCheckpoint::Spawn(CPortalRaceCheckpoint *this)

{
  undefined1 *puVar1;
  NetworkVar_m_Collision *this_00;
  int iVar2;
  float fVar3;
  uint8 uVar4;
  uint8 uVar5;
  uint8 uVar6;
  ushort uVar7;
  CBaseEdict *pCVar8;
  char *szContext;
  ushort *puVar9;
  int *piVar10;
  BASEPTR func;
  IChangeInfoAccessor *pIVar11;
  uint uVar12;
  uint uVar13;
  int unaff_EBX;
  longdouble lVar14;
  uint local_d8;
  uint local_d4;
  uint local_d0;
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
    super_IHandleEntity._vptr_IHandleEntity[0x1b])(this,unaff_EBX + 0x36a203);
  CBaseEntity::SetMoveType((CBaseEntity *)this,MOVETYPE_NOCLIP,MOVECOLLIDE_DEFAULT);
  this_00 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Collision;
  CCollisionProperty::SetSolid(&this_00->super_CCollisionProperty,SOLID_BBOX);
  CCollisionProperty::SetSolidFlags(&this_00->super_CCollisionProperty,0xc);
  CBaseEntity::SetCollisionGroup((CBaseEntity *)this,5);
  uVar4 = *(uint8 *)(unaff_EBX + 0x6828a5);
  uVar5 = *(uint8 *)(unaff_EBX + 0x6828a4);
  uVar6 = *(uint8 *)(unaff_EBX + 0x6828a3);
  if (uVar6 != (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
               super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      pCVar8 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar8 != (CBaseEdict *)0x0) && ((pCVar8->m_fStateFlags & 0x100U) == 0)) {
        pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 1;
        pIVar11 = CBaseEdict::GetChangeAccessor(pCVar8);
        puVar9 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4ffec7);
        if (pIVar11->m_iChangeInfoSerialNumber == *puVar9) {
          uVar13 = (uint)pIVar11->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar7 = puVar9[uVar13 * 0x14 + 0x14];
          if (uVar7 == 0) {
LAB_0074720a:
            puVar9[(uint)uVar7 + uVar13 * 0x14 + 1] = 0x108;
            puVar9[uVar13 * 0x14 + 0x14] = uVar7 + 1;
          }
          else if (puVar9[uVar13 * 0x14 + 1] != 0x108) {
            local_d8 = 0;
            do {
              uVar12 = local_d8 + 1;
              local_d8 = uVar12 & 0xffff;
              if ((ushort)uVar12 == uVar7) {
                if (uVar7 != 0x13) goto LAB_0074720a;
                pIVar11->m_iChangeInfoSerialNumber = 0;
                pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar9[uVar13 * 0x14 + local_d8 + 1] != 0x108);
          }
        }
        else if (puVar9[0x7d1] == 100) {
          pIVar11->m_iChangeInfoSerialNumber = 0;
          pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar11->m_iChangeInfo = puVar9[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4ffec7) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4ffec7) + 0xfa2) + 1;
          piVar10 = *(int **)(unaff_EBX + 0x4ffec7);
          pIVar11->m_iChangeInfoSerialNumber = *(ushort *)*piVar10;
          iVar2 = *piVar10 + (uint)pIVar11->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x108;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
    super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r = uVar6;
  }
  if (uVar5 != (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
               super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      pCVar8 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar8 != (CBaseEdict *)0x0) && ((pCVar8->m_fStateFlags & 0x100U) == 0)) {
        pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 1;
        pIVar11 = CBaseEdict::GetChangeAccessor(pCVar8);
        puVar9 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4ffec7);
        if (pIVar11->m_iChangeInfoSerialNumber == *puVar9) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar13 = (uint)pIVar11->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar7 = puVar9[uVar13 * 0x14 + 0x14];
          if (uVar7 == 0) {
LAB_007472ca:
            puVar9[(uint)uVar7 + uVar13 * 0x14 + 1] = 0x108;
            puVar9[uVar13 * 0x14 + 0x14] = uVar7 + 1;
          }
          else if (puVar9[uVar13 * 0x14 + 1] != 0x108) {
            local_d4 = 0;
            do {
              uVar12 = local_d4 + 1;
              local_d4 = uVar12 & 0xffff;
              if ((ushort)uVar12 == uVar7) {
                if (uVar7 == 0x13) goto LAB_00747139;
                goto LAB_007472ca;
              }
            } while (puVar9[uVar13 * 0x14 + local_d4 + 1] != 0x108);
          }
        }
        else if (puVar9[0x7d1] == 100) {
LAB_00747139:
          pIVar11->m_iChangeInfoSerialNumber = 0;
          pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar11->m_iChangeInfo = puVar9[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4ffec7) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4ffec7) + 0xfa2) + 1;
          piVar10 = *(int **)(unaff_EBX + 0x4ffec7);
          pIVar11->m_iChangeInfoSerialNumber = *(ushort *)*piVar10;
          iVar2 = *piVar10 + (uint)pIVar11->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x108;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
    super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g = uVar5;
  }
  if (uVar4 != (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
               super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      pCVar8 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar8 != (CBaseEdict *)0x0) && ((pCVar8->m_fStateFlags & 0x100U) == 0)) {
        pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 1;
        pIVar11 = CBaseEdict::GetChangeAccessor(pCVar8);
        puVar9 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4ffec7);
        if (pIVar11->m_iChangeInfoSerialNumber == *puVar9) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar13 = (uint)pIVar11->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar7 = puVar9[uVar13 * 0x14 + 0x14];
          if (uVar7 == 0) {
LAB_0074738a:
            puVar9[(uint)uVar7 + uVar13 * 0x14 + 1] = 0x108;
            puVar9[uVar13 * 0x14 + 0x14] = uVar7 + 1;
          }
          else if (puVar9[uVar13 * 0x14 + 1] != 0x108) {
            local_d0 = 0;
            do {
              uVar12 = local_d0 + 1;
              local_d0 = uVar12 & 0xffff;
              if ((ushort)uVar12 == uVar7) {
                if (uVar7 == 0x13) goto LAB_0074714d;
                goto LAB_0074738a;
              }
            } while (puVar9[uVar13 * 0x14 + local_d0 + 1] != 0x108);
          }
        }
        else if (puVar9[0x7d1] == 100) {
LAB_0074714d:
          pIVar11->m_iChangeInfoSerialNumber = 0;
          pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar11->m_iChangeInfo = puVar9[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4ffec7) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4ffec7) + 0xfa2) + 1;
          piVar10 = *(int **)(unaff_EBX + 0x4ffec7);
          pIVar11->m_iChangeInfoSerialNumber = *(ushort *)*piVar10;
          iVar2 = *piVar10 + (uint)pIVar11->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x108;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
    super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b = uVar4;
  }
  fVar3 = *(float *)(*(int *)(unaff_EBX + 0x68293b) + 0x2c);
  CBaseAnimating::SetModelScale(&this->super_CBaseAnimating,2.0,0.0,HIERARCHICAL_MODEL_SCALE);
  if (((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
                    /* Unresolved local var: Vector res@[???] */
  local_28[0].x = (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.x + 0.0;
  local_28[0].y = (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.y + 0.0;
  local_28[0].z =
       -(fVar3 * *(float *)(unaff_EBX + 0x3862ff)) +
       (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.z;
  CBaseEntity::SetAbsOrigin((CBaseEntity *)this,local_28);
  local_40.x = 50.0;
  local_40.y = 50.0;
  local_40.z = 100.0;
  local_34.x = -50.0;
  local_34.y = -50.0;
  local_34.z = 0.0;
  CBaseEntity::SetSize((CBaseEntity *)this,&local_34,&local_40);
  szContext = *(char **)(unaff_EBX + 0x62083f);
  fVar3 = *(float *)(**(int **)(unaff_EBX + 0x4ffec3) + 0xc);
  lVar14 = (longdouble)_RandomFloat(0,0x3f000000);
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x201);
  CBaseEntity::ThinkSet((CBaseEntity *)this,func,fVar3 + (float)lVar14,szContext);
  CBaseAnimating::Spawn(&this->super_CBaseAnimating);
  return;
}


/* CPortalRaceCheckpoint::StartSpinningThink at 00746aa0 */

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


/* CPortalRaceCheckpoint::OnCheckpointActivated at 00747d10 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void OnCheckpointActivated(CPortalRaceCheckpoint * this) */

void __thiscall CPortalRaceCheckpoint::OnCheckpointActivated(CPortalRaceCheckpoint *this)

{
  undefined1 *puVar1;
  int iVar2;
  uint8 uVar3;
  uint8 uVar4;
  uint8 uVar5;
  ushort uVar6;
  CBaseEdict *pCVar7;
  ushort *puVar8;
  int *piVar9;
  IChangeInfoAccessor *pIVar10;
  uint uVar11;
  uint uVar12;
  int unaff_EBX;
  uint local_ac;
  uint local_a8;
  uint local_a4;
  uint local_a0;
  
  ___i686_get_pc_thunk_bx();
  if ((this->m_bCheckpointActive).m_Value == false) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      pCVar7 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar7 != (CBaseEdict *)0x0) && ((pCVar7->m_fStateFlags & 0x100U) == 0)) {
        pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar7);
        puVar8 = (ushort *)**(undefined4 **)(&DAT_004fee47 + unaff_EBX);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar8) {
          uVar12 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar6 = puVar8[uVar12 * 0x14 + 0x14];
          if (uVar6 == 0) {
LAB_007482b4:
            puVar8[(uint)uVar6 + uVar12 * 0x14 + 1] = 0x550;
            puVar8[uVar12 * 0x14 + 0x14] = uVar6 + 1;
          }
          else if (puVar8[uVar12 * 0x14 + 1] != 0x550) {
            local_ac = 0;
            do {
              uVar11 = local_ac + 1;
              local_ac = uVar11 & 0xffff;
              if ((ushort)uVar11 == uVar6) {
                if (uVar6 != 0x13) goto LAB_007482b4;
                pIVar10->m_iChangeInfoSerialNumber = 0;
                pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar8[uVar12 * 0x14 + local_ac + 1] != 0x550);
          }
        }
        else if (puVar8[0x7d1] == 100) {
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar8[0x7d1];
          *(short *)(**(int **)(&DAT_004fee47 + unaff_EBX) + 0xfa2) =
               *(short *)(**(int **)(&DAT_004fee47 + unaff_EBX) + 0xfa2) + 1;
          piVar9 = *(int **)(&DAT_004fee47 + unaff_EBX);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar9;
          iVar2 = *piVar9 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x550;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bCheckpointActive).m_Value = true;
  }
  CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x36bc49),0.0,(float *)0x0);
  this->m_fLastTimerSound =
       *(float *)(**(int **)(unaff_EBX + 0x4fee43) + 0xc) - *(float *)(unaff_EBX + 0x384bef);
  uVar3 = *(uint8 *)(unaff_EBX + 0x681821);
  uVar4 = *(uint8 *)(unaff_EBX + 0x681820);
  uVar5 = *(uint8 *)(unaff_EBX + 0x68181f);
  if (uVar5 != (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
               super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      pCVar7 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar7 != (CBaseEdict *)0x0) && ((pCVar7->m_fStateFlags & 0x100U) == 0)) {
        pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar7);
        puVar8 = (ushort *)**(undefined4 **)(&DAT_004fee47 + unaff_EBX);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar8) {
          uVar12 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar6 = puVar8[uVar12 * 0x14 + 0x14];
          if (uVar6 == 0) {
LAB_00748410:
            puVar8[(uint)uVar6 + uVar12 * 0x14 + 1] = 0x108;
            puVar8[uVar12 * 0x14 + 0x14] = uVar6 + 1;
          }
          else if (puVar8[uVar12 * 0x14 + 1] != 0x108) {
            local_a8 = 0;
            do {
              uVar11 = local_a8 + 1;
              local_a8 = uVar11 & 0xffff;
              if ((ushort)uVar11 == uVar6) {
                if (uVar6 != 0x13) goto LAB_00748410;
                pIVar10->m_iChangeInfoSerialNumber = 0;
                pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar8[uVar12 * 0x14 + local_a8 + 1] != 0x108);
          }
        }
        else if (puVar8[0x7d1] == 100) {
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar8[0x7d1];
          *(short *)(**(int **)(&DAT_004fee47 + unaff_EBX) + 0xfa2) =
               *(short *)(**(int **)(&DAT_004fee47 + unaff_EBX) + 0xfa2) + 1;
          piVar9 = *(int **)(&DAT_004fee47 + unaff_EBX);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar9;
          iVar2 = *piVar9 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x108;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
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
      pCVar7 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar7 != (CBaseEdict *)0x0) && ((pCVar7->m_fStateFlags & 0x100U) == 0)) {
        pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar7);
        puVar8 = (ushort *)**(undefined4 **)(&DAT_004fee47 + unaff_EBX);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar8) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar12 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar6 = puVar8[uVar12 * 0x14 + 0x14];
          if (uVar6 == 0) {
LAB_00748364:
            puVar8[(uint)uVar6 + uVar12 * 0x14 + 1] = 0x108;
            puVar8[uVar12 * 0x14 + 0x14] = uVar6 + 1;
          }
          else if (puVar8[uVar12 * 0x14 + 1] != 0x108) {
            local_a4 = 0;
            do {
              uVar11 = local_a4 + 1;
              local_a4 = uVar11 & 0xffff;
              if ((ushort)uVar11 == uVar6) {
                if (uVar6 == 0x13) goto LAB_00748126;
                goto LAB_00748364;
              }
            } while (puVar8[uVar12 * 0x14 + local_a4 + 1] != 0x108);
          }
        }
        else if (puVar8[0x7d1] == 100) {
LAB_00748126:
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar8[0x7d1];
          *(short *)(**(int **)(&DAT_004fee47 + unaff_EBX) + 0xfa2) =
               *(short *)(**(int **)(&DAT_004fee47 + unaff_EBX) + 0xfa2) + 1;
          piVar9 = *(int **)(&DAT_004fee47 + unaff_EBX);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar9;
          iVar2 = *piVar9 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x108;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
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
      pCVar7 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar7 != (CBaseEdict *)0x0) && ((pCVar7->m_fStateFlags & 0x100U) == 0)) {
        pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar7);
        puVar8 = (ushort *)**(undefined4 **)(&DAT_004fee47 + unaff_EBX);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar8) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar12 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar6 = puVar8[uVar12 * 0x14 + 0x14];
          if (uVar6 == 0) {
LAB_007481f0:
            puVar8[(uint)uVar6 + uVar12 * 0x14 + 1] = 0x108;
            puVar8[uVar12 * 0x14 + 0x14] = uVar6 + 1;
          }
          else if (puVar8[uVar12 * 0x14 + 1] != 0x108) {
            local_a0 = 0;
            do {
              uVar11 = local_a0 + 1;
              local_a0 = uVar11 & 0xffff;
              if ((ushort)uVar11 == uVar6) {
                if (uVar6 == 0x13) goto LAB_00748218;
                goto LAB_007481f0;
              }
            } while (puVar8[uVar12 * 0x14 + local_a0 + 1] != 0x108);
          }
        }
        else if (puVar8[0x7d1] == 100) {
LAB_00748218:
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar8[0x7d1];
          *(short *)(**(int **)(&DAT_004fee47 + unaff_EBX) + 0xfa2) =
               *(short *)(**(int **)(&DAT_004fee47 + unaff_EBX) + 0xfa2) + 1;
          piVar9 = *(int **)(&DAT_004fee47 + unaff_EBX);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar9;
          iVar2 = *piVar9 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x108;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
    super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b = uVar3;
  }
  COutputEvent::FireOutput
            (&this->m_OnCheckpointActivated,(CBaseEntity *)this,(CBaseEntity *)this,0.0);
  return;
}


/* CPortalRaceCheckpoint::OnCheckpointReset at 007475d0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void OnCheckpointReset(CPortalRaceCheckpoint * this, bool bFireOutput)
    */

void __thiscall
CPortalRaceCheckpoint::OnCheckpointReset(CPortalRaceCheckpoint *this,bool bFireOutput)

{
  undefined1 *puVar1;
  uint uVar2;
  int iVar3;
  uint8 uVar4;
  uint8 uVar5;
  uint8 uVar6;
  ushort uVar7;
  CBaseEdict *pCVar8;
  int *piVar9;
  ushort *puVar10;
  IChangeInfoAccessor *pIVar11;
  uint uVar12;
  int unaff_EBX;
  uint uVar13;
  
  ___i686_get_pc_thunk_bx();
  if ((this->m_bCheckpointActive).m_Value != false) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      pCVar8 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar8 != (CBaseEdict *)0x0) && ((pCVar8->m_fStateFlags & 0x100U) == 0)) {
        pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 1;
        pIVar11 = CBaseEdict::GetChangeAccessor(pCVar8);
        piVar9 = *(int **)(unaff_EBX + 0x4ff587);
        puVar10 = (ushort *)*piVar9;
        if (pIVar11->m_iChangeInfoSerialNumber == *puVar10) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar13 = (uint)pIVar11->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar7 = puVar10[uVar13 * 0x14 + 0x14];
          if (uVar7 == 0) {
LAB_007478ba:
            puVar10[(uint)uVar7 + uVar13 * 0x14 + 1] = 0x550;
            puVar10[uVar13 * 0x14 + 0x14] = uVar7 + 1;
          }
          else if (puVar10[uVar13 * 0x14 + 1] != 0x550) {
            uVar12 = 0;
            do {
              uVar2 = uVar12 + 1;
              uVar12 = uVar2 & 0xffff;
              if ((ushort)uVar2 == uVar7) {
                if (uVar7 != 0x13) goto LAB_007478ba;
                goto LAB_00747a5b;
              }
            } while (puVar10[uVar13 * 0x14 + uVar12 + 1] != 0x550);
          }
        }
        else if (puVar10[0x7d1] == 100) {
LAB_00747a5b:
          pIVar11->m_iChangeInfoSerialNumber = 0;
          pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar11->m_iChangeInfo = puVar10[0x7d1];
          *(short *)(*piVar9 + 0xfa2) = *(short *)(*piVar9 + 0xfa2) + 1;
          pIVar11->m_iChangeInfoSerialNumber = *(ushort *)*piVar9;
          iVar3 = *piVar9 + (uint)pIVar11->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar3 + 2) = 0x550;
          *(undefined2 *)(iVar3 + 0x28) = 1;
        }
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
    CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x36bb57),0.0,(float *)0x0);
    COutputEvent::FireOutput(&this->m_OnCheckpointReset,(CBaseEntity *)this,(CBaseEntity *)this,0.0)
    ;
    uVar4 = *(uint8 *)(unaff_EBX + 0x681f65);
    uVar5 = *(uint8 *)(unaff_EBX + 0x681f64);
    uVar6 = *(uint8 *)(unaff_EBX + 0x681f63);
    if (uVar6 != (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
                 super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r) {
      if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
          false) {
        pCVar8 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if ((pCVar8 != (CBaseEdict *)0x0) && ((pCVar8->m_fStateFlags & 0x100U) == 0)) {
          pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 1;
          pIVar11 = CBaseEdict::GetChangeAccessor(pCVar8);
          piVar9 = *(int **)(unaff_EBX + 0x4ff587);
          puVar10 = (ushort *)*piVar9;
          if (pIVar11->m_iChangeInfoSerialNumber == *puVar10) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            uVar13 = (uint)pIVar11->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar7 = puVar10[uVar13 * 0x14 + 0x14];
            if (uVar7 == 0) {
LAB_00747b6a:
              puVar10[(uint)uVar7 + uVar13 * 0x14 + 1] = 0x108;
              puVar10[uVar13 * 0x14 + 0x14] = uVar7 + 1;
            }
            else if (puVar10[uVar13 * 0x14 + 1] != 0x108) {
              uVar12 = 0;
              do {
                uVar2 = uVar12 + 1;
                uVar12 = uVar2 & 0xffff;
                if ((ushort)uVar2 == uVar7) {
                  if (uVar7 != 0x13) goto LAB_00747b6a;
                  goto LAB_00747c07;
                }
              } while (puVar10[uVar13 * 0x14 + uVar12 + 1] != 0x108);
            }
          }
          else if (puVar10[0x7d1] == 100) {
LAB_00747c07:
            pIVar11->m_iChangeInfoSerialNumber = 0;
            pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar11->m_iChangeInfo = puVar10[0x7d1];
            *(short *)(*piVar9 + 0xfa2) = *(short *)(*piVar9 + 0xfa2) + 1;
            pIVar11->m_iChangeInfoSerialNumber = *(ushort *)*piVar9;
            iVar3 = *piVar9 + (uint)pIVar11->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar3 + 2) = 0x108;
            *(undefined2 *)(iVar3 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
      super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r = uVar6;
    }
    if (uVar5 != (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
                 super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g) {
      if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
          false) {
        pCVar8 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if ((pCVar8 != (CBaseEdict *)0x0) && ((pCVar8->m_fStateFlags & 0x100U) == 0)) {
          pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 1;
          pIVar11 = CBaseEdict::GetChangeAccessor(pCVar8);
          piVar9 = *(int **)(unaff_EBX + 0x4ff587);
          puVar10 = (ushort *)*piVar9;
          if (pIVar11->m_iChangeInfoSerialNumber == *puVar10) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            uVar13 = (uint)pIVar11->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar7 = puVar10[uVar13 * 0x14 + 0x14];
            if (uVar7 == 0) {
LAB_00747be6:
              puVar10[(uint)uVar7 + uVar13 * 0x14 + 1] = 0x108;
              puVar10[uVar13 * 0x14 + 0x14] = uVar7 + 1;
            }
            else if (puVar10[uVar13 * 0x14 + 1] != 0x108) {
              uVar12 = 0;
              do {
                uVar2 = uVar12 + 1;
                uVar12 = uVar2 & 0xffff;
                if ((ushort)uVar2 == uVar7) {
                  if (uVar7 != 0x13) goto LAB_00747be6;
                  goto LAB_00747c1b;
                }
              } while (puVar10[uVar13 * 0x14 + uVar12 + 1] != 0x108);
            }
          }
          else if (puVar10[0x7d1] == 100) {
LAB_00747c1b:
            pIVar11->m_iChangeInfoSerialNumber = 0;
            pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar11->m_iChangeInfo = puVar10[0x7d1];
            *(short *)(*piVar9 + 0xfa2) = *(short *)(*piVar9 + 0xfa2) + 1;
            pIVar11->m_iChangeInfoSerialNumber = *(ushort *)*piVar9;
            iVar3 = *piVar9 + (uint)pIVar11->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar3 + 2) = 0x108;
            *(undefined2 *)(iVar3 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
      super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g = uVar5;
    }
    if (uVar4 != (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
                 super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b) {
      if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
          false) {
        pCVar8 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if ((pCVar8 != (CBaseEdict *)0x0) && ((pCVar8->m_fStateFlags & 0x100U) == 0)) {
          pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 1;
          pIVar11 = CBaseEdict::GetChangeAccessor(pCVar8);
          piVar9 = *(int **)(unaff_EBX + 0x4ff587);
          puVar10 = (ushort *)*piVar9;
          if (pIVar11->m_iChangeInfoSerialNumber == *puVar10) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            uVar13 = (uint)pIVar11->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar7 = puVar10[uVar13 * 0x14 + 0x14];
            if (uVar7 == 0) {
LAB_00747ad6:
              puVar10[(uint)uVar7 + uVar13 * 0x14 + 1] = 0x108;
              puVar10[uVar13 * 0x14 + 0x14] = uVar7 + 1;
            }
            else if (puVar10[uVar13 * 0x14 + 1] != 0x108) {
              uVar12 = 0;
              do {
                uVar2 = uVar12 + 1;
                uVar12 = uVar2 & 0xffff;
                if ((ushort)uVar2 == uVar7) {
                  if (uVar7 != 0x13) goto LAB_00747ad6;
                  goto LAB_00747af7;
                }
              } while (puVar10[uVar13 * 0x14 + uVar12 + 1] != 0x108);
            }
          }
          else if (puVar10[0x7d1] == 100) {
LAB_00747af7:
            pIVar11->m_iChangeInfoSerialNumber = 0;
            pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar11->m_iChangeInfo = puVar10[0x7d1];
            *(short *)(*piVar9 + 0xfa2) = *(short *)(*piVar9 + 0xfa2) + 1;
            pIVar11->m_iChangeInfoSerialNumber = *(ushort *)*piVar9;
            iVar3 = *piVar9 + (uint)pIVar11->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar3 + 2) = 0x108;
            *(undefined2 *)(iVar3 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->super_CBaseAnimating).super_CBaseEntity.m_clrRender.
      super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b = uVar4;
      return;
    }
  }
  return;
}


/* CPortalRaceCheckpoint::StartTouch at 00748580 */

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
      func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x954);
      CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
      piVar1 = *(int **)(unaff_EBX + 0x4fe5d0);
      CBaseEntity::SetNextThink
                ((CBaseEntity *)this,*(float *)(unaff_EBX + 0x384380) + *(float *)(*piVar1 + 0xc),
                 (char *)0x0);
      this->m_fActivatedTime = *(float *)(*piVar1 + 0xc);
      OnCheckpointActivated(this);
      return;
    }
    CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x36abb5),0.0,(float *)0x0);
  }
  return;
}


/* CPortalRaceCheckpoint::ActivateCheckpoint at 00748460 */

/* DWARF original prototype: void ActivateCheckpoint(CPortalRaceCheckpoint * this) */

void __thiscall CPortalRaceCheckpoint::ActivateCheckpoint(CPortalRaceCheckpoint *this)

{
  int *piVar1;
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x834);
  CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
  piVar1 = *(int **)(unaff_EBX + 0x4fe6f0);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,*(float *)(unaff_EBX + 0x3844a0) + *(float *)(*piVar1 + 0xc),
             (char *)0x0);
  this->m_fActivatedTime = *(float *)(*piVar1 + 0xc);
  OnCheckpointActivated(this);
  return;
}


/* CPortalRaceCheckpoint::InputActivate at 007484f0 */

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
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x8c4);
  CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
  piVar1 = *(int **)(unaff_EBX + 0x4fe660);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,*(float *)(unaff_EBX + 0x384410) + *(float *)(*piVar1 + 0xc),
             (char *)0x0);
  this->m_fActivatedTime = *(float *)(*piVar1 + 0xc);
  OnCheckpointActivated(this);
  return;
}


/* CPortalRaceCheckpoint::ActivatedThink at 00747c40 */

/* DWARF original prototype: void ActivatedThink(CPortalRaceCheckpoint * this) */

void __thiscall CPortalRaceCheckpoint::ActivatedThink(CPortalRaceCheckpoint *this)

{
  int *piVar1;
  int unaff_EBX;
  float fVar2;
  
  ___i686_get_pc_thunk_bx();
  piVar1 = *(int **)(unaff_EBX + 0x4fef10);
  fVar2 = *(float *)(*piVar1 + 0xc) - this->m_fActivatedTime;
  if (this->m_fResetTime <= fVar2 && fVar2 != this->m_fResetTime) {
    OnCheckpointReset(this,true);
  }
  else {
    fVar2 = *(float *)(*piVar1 + 0xc) - this->m_fLastTimerSound;
    if (*(float *)(unaff_EBX + 0x384cbc) <= fVar2 && fVar2 != *(float *)(unaff_EBX + 0x384cbc)) {
      CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x36bd00),0.0,(float *)0x0);
      this->m_fLastTimerSound = *(float *)(*piVar1 + 0xc);
    }
    CBaseEntity::SetNextThink
              ((CBaseEntity *)this,*(float *)(unaff_EBX + 0x384cc0) + *(float *)(*piVar1 + 0xc),
               (char *)0x0);
  }
  return;
}


/* CPortalRaceCheckpoint::Lock at 007469e0 */

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


/* CPortalRaceCheckpoint::InputLock at 00746c40 */

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


/* CPortalRaceCheckpoint::Unlock at 007469d0 */

/* DWARF original prototype: void Unlock(CPortalRaceCheckpoint * this) */

void __thiscall CPortalRaceCheckpoint::Unlock(CPortalRaceCheckpoint *this)

{
  this->m_bLocked = false;
  return;
}


/* CPortalRaceCheckpoint::InputUnlock at 00746c30 */

/* DWARF original prototype: void InputUnlock(CPortalRaceCheckpoint * this, inputdata_t * inputData)
    */

void __thiscall
CPortalRaceCheckpoint::InputUnlock(CPortalRaceCheckpoint *this,inputdata_t *inputData)

{
  this->m_bLocked = false;
  return;
}


/* CPortalRaceCheckpoint::InputCancelActivate at 00747c30 */

/* DWARF original prototype: void InputCancelActivate(CPortalRaceCheckpoint * this, inputdata_t *
   inputData) */

void __thiscall
CPortalRaceCheckpoint::InputCancelActivate(CPortalRaceCheckpoint *this,inputdata_t *inputData)

{
  OnCheckpointReset(this,false);
  return;
}


/* _GLOBAL__I_checkpointActiveColor at 000b2050 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_checkpointActiveColor(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

