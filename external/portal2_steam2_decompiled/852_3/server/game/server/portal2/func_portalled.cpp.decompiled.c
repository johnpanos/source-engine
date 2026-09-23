/* DWARF-guided pseudocode for game/server/portal2/func_portalled.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* DataMapInit<CFunc_Portalled> at 000be960 */

datamap_t * DataMapInit<CFunc_Portalled>(CFunc_Portalled *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((&DAT_00cf7fdc)[unaff_EBX] == '\0') {
    iVar2 = ___cxa_guard_acquire(&DAT_00cf7fdc + unaff_EBX);
    if (iVar2 != 0) {
      *(int *)((int)&paintblob_min_radius_scale.super_ConCommandBase.m_pszName + unaff_EBX) =
           unaff_EBX + 0x9500a2;
      *(undefined4 *)((int)&paintblob_min_radius_scale.super_ConCommandBase.m_nFlags + unaff_EBX) =
           0;
      *(undefined4 *)((int)&paintblob_min_radius_scale.super_IConVar._vptr_IConVar + unaff_EBX) = 0;
      *(undefined4 *)((int)&paintblob_min_radius_scale.m_pParent + unaff_EBX) = 0;
      *(undefined4 *)((int)&paintblob_min_radius_scale.m_pszDefaultValue + unaff_EBX) = 0;
      *(undefined4 *)((int)&paintblob_min_radius_scale.m_Value.m_pszString + unaff_EBX) = 0;
      *(undefined4 *)
       ((int)&paintblob_min_radius_scale.super_ConCommandBase.m_pszHelpString + unaff_EBX) = 0xf;
      ___cxa_guard_release(&DAT_00cf7fdc + unaff_EBX);
      ___cxa_atexit(unaff_EBX + 0x8da91c,0,*(undefined4 *)(&DAT_00aed21c + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_BodyTarget_00c0ab18 + unaff_EBX) =
       *(undefined4 *)(&DAT_00aedd70 + unaff_EBX);
  if (*(char *)((int)&paintblob_min_radius_scale.super_ConCommandBase.m_pNext + unaff_EBX) == '\0')
  {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcf7fe4);
    if (iVar2 != 0) {
      uVar1 = **(undefined4 **)(&DAT_00aed238 + unaff_EBX);
      *(undefined4 *)((int)&PTR_GetAIAddOn_00c0ac40 + unaff_EBX) = uVar1;
      *(undefined4 *)((int)&PTR_PostClientActive_00c0ac80 + unaff_EBX) = uVar1;
      ___cxa_guard_release(unaff_EBX + 0xcf7fe4);
    }
  }
  *(undefined4 *)((int)&PTR_LocalEyeAngles_00c0ab10 + unaff_EBX) = 5;
  *(int *)((int)&PTR_EyeAngles_00c0ab0c + unaff_EBX) =
       (int)&PTR_VPhysicsDestroyObject_00c0ab6c + unaff_EBX;
  return (datamap_t *)((int)&PTR_EyeAngles_00c0ab0c + unaff_EBX);
}


/* __static_initialization_and_destruction_0 at 000bea90 */

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
  *(undefined1 *)
   ((int)&debug_paint_server_blobs.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) = 0;
  *(undefined1 *)
   ((int)&debug_paint_server_blobs.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&debug_paint_server_blobs.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 2) = 0;
  *(undefined1 *)
   ((int)&debug_paint_server_blobs.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&debug_paint_server_blobs.m_fnChangeCallbacks.m_Size + unaff_EBX) = 0;
  *(undefined4 *)((int)&debug_paint_server_blobs.m_fnChangeCallbacks.m_pElements + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00cf7e58 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00cf7e5c + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&debug_paintblobs_streaking.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) = 0;
  *(undefined4 *)((int)&debug_paintblobs_streaking.super_ConCommandBase.m_pNext + unaff_EBX) = 0;
  *(undefined4 *)(&debug_paintblobs_streaking.super_ConCommandBase.m_bRegistered + unaff_EBX) = 0;
  *(undefined4 *)((int)&debug_paintblobs_streaking.super_ConCommandBase.m_pszName + unaff_EBX) = 0;
  *(undefined4 *)((int)&debug_paintblobs_streaking.super_ConCommandBase.m_pszHelpString + unaff_EBX)
       = 0x7f7fffff;
  *(undefined4 *)((int)&debug_paintblobs_streaking.super_ConCommandBase.m_nFlags + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&debug_paintblobs_streaking.super_IConVar._vptr_IConVar + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&debug_paintblobs_streaking.m_pParent + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&debug_paintblobs_streaking.m_pszDefaultValue + unaff_EBX) = 0;
  *(undefined1 *)((int)&debug_paintblobs_streaking.m_Value.m_pszString + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&debug_paintblobs_streaking.m_Value.m_pszString + unaff_EBX) = 0;
  *(undefined1 *)((int)&debug_paintblobs_streaking.m_Value.m_pszString + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&debug_paintblobs_streaking.m_Value.m_pszString + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&debug_paintblobs_streaking.m_Value.m_StringLength + unaff_EBX) = 0;
  *(undefined1 *)((int)&debug_paintblobs_streaking.m_Value.m_fValue + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&debug_paintblobs_streaking.m_Value.m_fValue + unaff_EBX) = 0;
  *(undefined1 *)((int)&debug_paintblobs_streaking.m_Value.m_fValue + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&debug_paintblobs_streaking.m_Value.m_fValue + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&debug_paintblobs_streaking.m_Value.m_nValue + unaff_EBX) = 0;
  (&debug_paintblobs_streaking.field_0x37)[unaff_EBX] = 1;
  (&debug_paintblobs_streaking.m_bHasMin)[unaff_EBX] = false;
  (&debug_paintblobs_streaking.field_0x35)[unaff_EBX] = 0;
  (&debug_paintblobs_streaking.field_0x36)[unaff_EBX] = 0;
  *(undefined4 *)((int)&debug_paintblobs_streaking.m_fMinVal + unaff_EBX) = 1;
  (&debug_paintblobs_streaking.field_0x3f)[unaff_EBX] = 0;
  (&debug_paintblobs_streaking.m_bHasMax)[unaff_EBX] = false;
  (&debug_paintblobs_streaking.field_0x3d)[unaff_EBX] = 0;
  (&debug_paintblobs_streaking.field_0x3e)[unaff_EBX] = 0;
  *(undefined4 *)((int)&debug_paintblobs_streaking.m_fMaxVal + unaff_EBX) = 2;
  *(undefined1 *)
   ((int)&debug_paintblobs_streaking.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 3) = 0;
  *(undefined1 *)
   ((int)&debug_paintblobs_streaking.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) = 0;
  *(undefined1 *)
   ((int)&debug_paintblobs_streaking.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&debug_paintblobs_streaking.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 2) = 0;
  *(undefined **)
   ((int)&debug_paintblobs_streaking.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) =
       &UNK_00af288c + unaff_EBX;
  *(undefined **)
   ((int)&debug_paintblobs_streaking.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) =
       &UNK_00b71a14 + unaff_EBX;
  pIVar1 = EntityFactoryDictionary();
  (**pIVar1->_vptr_IEntityFactoryDictionary)(pIVar1,unaff_EBX + 0xcf7eac,unaff_EBX + 0x94ff8a);
  pdVar2 = DataMapInit<CFunc_Portalled>((CFunc_Portalled *)0x0);
  *(datamap_t **)((int)DataMapInit<CCollisionProperty>::dataDesc[7].flatOffset + unaff_EBX + -0x18)
       = pdVar2;
  return;
}


/* CFunc_Portalled::GetDataDescMap at 006b6990 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CFunc_Portalled * this) */

datamap_t * __thiscall CFunc_Portalled::GetDataDescMap(CFunc_Portalled *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x612ae8);
}


/* CFunc_Portalled::GetBaseMap at 006b69a0 */

datamap_t * CFunc_Portalled::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4f5d3c);
}


/* __tcf_0 at 00999290 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41d6d6)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41d6ca) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41d6d6));
  }
  *(undefined4 *)(unaff_EBX + 0x41d6d6) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41d6d2)) {
    if (*(int *)(unaff_EBX + 0x41d6ca) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2128fa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2128fa),*(int *)(unaff_EBX + 0x41d6ca));
      *(undefined4 *)(unaff_EBX + 0x41d6ca) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d6ce) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41d6ca);
  *(int *)(unaff_EBX + 0x41d6da) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41d6d2)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2128fa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2128fa),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41d6ca) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d6ce) = 0;
  }
  return;
}


/* CFunc_Portalled::PortalPlacedInsideBounds at 006b69b0 */

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


/* CFunc_Portalled::OnPrePortalled at 006b6a80 */

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


/* CFunc_Portalled::OnPostPortalled at 006b69f0 */

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


/* _GLOBAL__I__ZN15CFunc_Portalled9m_DataMapE at 000bec20 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN15CFunc_Portalled9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

