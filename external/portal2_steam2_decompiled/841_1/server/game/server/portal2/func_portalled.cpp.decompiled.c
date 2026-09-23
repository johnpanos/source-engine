/* DWARF-guided pseudocode for game/server/portal2/func_portalled.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* DataMapInit<CFunc_Portalled> at 000a9710 */

datamap_t * DataMapInit<CFunc_Portalled>(CFunc_Portalled *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x475].m_pPrev + unaff_EBX) ==
      '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xda233c);
    if (iVar2 != 0) {
      *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x476].m_pPrev + unaff_EBX) =
           unaff_EBX + 0xa070ee;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x477].m_pEntity + unaff_EBX) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x477].m_SerialNumber + unaff_EBX) = 0;
      *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x477].m_pPrev + unaff_EBX)
           = 0;
      *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x477].m_pNext + unaff_EBX)
           = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x478].m_pEntity + unaff_EBX) = 0;
      *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x476].m_pNext + unaff_EBX)
           = 0xf;
      ___cxa_guard_release(unaff_EBX + 0xda233c);
      ___cxa_atexit(unaff_EBX + 0x992cbc,0,*(undefined4 *)(&DAT_00b9d430 + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_ForceVPhysicsCollide_00cb9a28 + unaff_EBX) =
       *(undefined4 *)(&DAT_00b9df6c + unaff_EBX);
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x476].m_pEntity + unaff_EBX) ==
      '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xda2344);
    if (iVar2 != 0) {
      uVar1 = **(undefined4 **)(&DAT_00b9d450 + unaff_EBX);
      *(undefined4 *)((int)&PTR_SafeRemoveIfDesired_00cb9b50 + unaff_EBX) = uVar1;
      *(undefined4 *)
       (&CUtlVectorDataOps<CUtlVector<CBaseEntity*,CUtlMemory<CBaseEntity*,int>>,12>::vtable +
       unaff_EBX) = uVar1;
      ___cxa_guard_release(unaff_EBX + 0xda2344);
    }
  }
  *(undefined4 *)((int)&PTR_GetSoundEmissionOrigin_00cb9a20 + unaff_EBX) = 5;
  *(int *)((int)&PTR_WorldSpaceCenter_00cb9a1c + unaff_EBX) =
       (int)&PTR_NetworkStateChanged_m_nWaterLevel_00cb9a7c + unaff_EBX;
  return (datamap_t *)((int)&PTR_WorldSpaceCenter_00cb9a1c + unaff_EBX);
}


/* __static_initialization_and_destruction_0 at 000a9840 */

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
  __static_initialization_and_destruction_0
  [(int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX] = (code)0x0;
  *(undefined1 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x45f].m_pEntity + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x45f].m_pEntity + unaff_EBX + 2) = 0;
  *(undefined1 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x45f].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x45f].m_SerialNumber + unaff_EBX) = 0;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x45f].m_pPrev + unaff_EBX) = 0
  ;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x45f].m_pNext + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x460].m_pEntity + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x460].m_SerialNumber + unaff_EBX) = 0;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x460].m_pPrev + unaff_EBX) = 0
  ;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x460].m_pNext + unaff_EBX) = 0
  ;
  *(undefined4 *)
   (&UNK_00004614 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_00004618 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0x7f7fffff
  ;
  *(undefined4 *)
   (&UNK_0000461c + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0x7f7fffff
  ;
  *(undefined4 *)
   (&UNK_0000461c + (int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x462].m_pEntity + unaff_EBX) =
       0x7f7fffff;
  *(undefined **)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x462].m_SerialNumber + unaff_EBX) =
       &UNK_00ba2abc + unaff_EBX;
  *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x462].m_pPrev + unaff_EBX) =
       &UNK_00c1f224 + unaff_EBX;
  pIVar1 = EntityFactoryDictionary();
  (**pIVar1->_vptr_IEntityFactoryDictionary)(pIVar1,unaff_EBX + 0xda220c,&UNK_00a06fd6 + unaff_EBX);
  pdVar2 = DataMapInit<CFunc_Portalled>((CFunc_Portalled *)0x0);
  *(datamap_t **)
   ((int)DataMapInit<CAI_ScriptConditions>::dataDesc[0x16].flatOffset + unaff_EBX + -0x28) = pdVar2;
  return;
}


/* CFunc_Portalled::GetDataDescMap at 007129e0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CFunc_Portalled * this) */

datamap_t * __thiscall CFunc_Portalled::GetDataDescMap(CFunc_Portalled *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x650758);
}


/* CFunc_Portalled::GetBaseMap at 007129f0 */

datamap_t * CFunc_Portalled::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(&DAT_00534c98 + extraout_ECX);
}


/* __tcf_0 at 00a3c3e0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x40f696)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x40f68a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x40f696));
  }
  *(undefined4 *)(unaff_EBX + 0x40f696) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40f692)) {
    if (*(int *)(unaff_EBX + 0x40f68a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20a76e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20a76e),*(int *)(unaff_EBX + 0x40f68a));
      *(undefined4 *)(unaff_EBX + 0x40f68a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f68e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40f68a);
  *(int *)(unaff_EBX + 0x40f69a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40f692)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20a76e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20a76e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40f68a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f68e) = 0;
  }
  return;
}


/* CFunc_Portalled::PortalPlacedInsideBounds at 00712a00 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void PortalPlacedInsideBounds(CFunc_Portalled * this, CProp_Portal *
   pPortal) */

void __thiscall
CFunc_Portalled::PortalPlacedInsideBounds(CFunc_Portalled *this,CProp_Portal *pPortal)

{
  ulong *puVar1;
  
  if (this != (CFunc_Portalled *)0x0) {
    puVar1 = (ulong *)(*(this->super_CFuncPortalDetector).super_CBaseEntity.super_IServerEntity.
                        super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[3])(this);
    (pPortal->m_NotifyOnPortalled).super_CBaseHandle.m_Index = *puVar1;
    return;
  }
  (pPortal->m_NotifyOnPortalled).super_CBaseHandle.m_Index = 0xffffffff;
  return;
}


/* CFunc_Portalled::OnPrePortalled at 00712ad0 */

/* DWARF original prototype: void OnPrePortalled(CFunc_Portalled * this, CBaseEntity * pOther, bool
   bDeparting) */

void __thiscall
CFunc_Portalled::OnPrePortalled(CFunc_Portalled *this,CBaseEntity *pOther,bool bDeparting)

{
  char cVar1;
  int iVar2;
  
  if (bDeparting) {
    if (this->m_bFireOnDeparture == false) {
      return;
    }
    cVar1 = this->m_bFireOnPlayer;
  }
  else {
    if (this->m_bFireOnArrival == false) {
      return;
    }
    cVar1 = this->m_bFireOnPlayer;
  }
  if ((cVar1 != '\0') &&
     (iVar2 = (*(pOther->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                _vptr_IHandleEntity[0x55])(pOther), (char)iVar2 != '\0')) {
    COutputEvent::FireOutput(&this->m_OnEntityPrePortalled,pOther,(CBaseEntity *)this,0.0);
    return;
  }
  return;
}


/* CFunc_Portalled::OnPostPortalled at 00712a40 */

/* DWARF original prototype: void OnPostPortalled(CFunc_Portalled * this, CBaseEntity * pOther, bool
   bDeparting) */

void __thiscall
CFunc_Portalled::OnPostPortalled(CFunc_Portalled *this,CBaseEntity *pOther,bool bDeparting)

{
  char cVar1;
  int iVar2;
  
  if (bDeparting) {
    if (this->m_bFireOnDeparture == false) {
      return;
    }
    cVar1 = this->m_bFireOnPlayer;
  }
  else {
    if (this->m_bFireOnArrival == false) {
      return;
    }
    cVar1 = this->m_bFireOnPlayer;
  }
  if ((cVar1 != '\0') &&
     (iVar2 = (*(pOther->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                _vptr_IHandleEntity[0x55])(pOther), (char)iVar2 != '\0')) {
    COutputEvent::FireOutput(&this->m_OnEntityPostPortalled,pOther,(CBaseEntity *)this,0.0);
    return;
  }
  return;
}


/* _GLOBAL__I__ZN15CFunc_Portalled9m_DataMapE at 000a9930 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN15CFunc_Portalled9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

