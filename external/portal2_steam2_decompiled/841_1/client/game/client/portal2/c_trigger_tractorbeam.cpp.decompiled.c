/* DWARF-guided pseudocode for game/client/portal2/c_trigger_tractorbeam.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 00079fb0 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  RecvTable *pRVar1;
  undefined4 uVar2;
  int *piVar3;
  int iVar4;
  IClassMap *pIVar5;
  IMaterialProxyDict *pIVar6;
  int unaff_EBX;
  longlong lVar7;
  
  lVar7 = ___i686_get_pc_thunk_bx();
  if (lVar7 == 0xffff00000001) {
    (&DAT_00c5915c)[unaff_EBX] = 0;
    (&DAT_00c5915d)[unaff_EBX] = 0;
    (&DAT_00c5915e)[unaff_EBX] = 0;
    (&DAT_00c5915f)[unaff_EBX] = 0;
    *(undefined4 *)
     ((int)&CUtlRBTree<CUtlMap<int,CHudRenderGroup*,unsigned_short>::Node_t,unsigned_short,CUtlMap<int,CHudRenderGroup*,unsigned_short>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<int,CHudRenderGroup*,unsigned_short>::Node_t,unsigned_short>,unsigned_short>>
            ::Links(unsigned_short)::s_Sentinel + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00c59164 + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00c59168 + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00c5916c + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&vgui::RichText::GetKBMap::s_pMap + unaff_EBX) = 0;
    *(undefined4 *)((int)&vgui::RichText::GetMessageMap::s_pMap + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00c59178 + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00c5917c + unaff_EBX) = 0;
    *(undefined4 *)((int)&CHudChatFilterPanel::GetKBMap::s_pMap + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&CHudChatFilterPanel::GetMessageMap::s_pMap + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&CHudChatFilterButton::GetKBMap::s_pMap + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&CHudChatFilterButton::GetMessageMap::s_pMap + unaff_EBX) = 0x7f7fffff;
    *(undefined **)((int)&CHudChatHistory::GetKBMap::s_pMap + unaff_EBX) = &UNK_00aeaec4 + unaff_EBX
    ;
    *(undefined4 *)((int)s_SIMDRandContexts[0x19].m_RandY[0x20] + unaff_EBX + 8) = 0;
    *(undefined4 *)((int)s_SIMDRandContexts[0x19].m_RandY[0x20] + unaff_EBX + 0xc) = 0;
    *(undefined4 *)((int)s_SIMDRandContexts[0x19].m_RandY[0x21] + unaff_EBX) = 0;
    *(undefined4 *)((int)s_SIMDRandContexts[0x19].m_RandY[0x21] + unaff_EBX + 4) = 0;
    *(undefined4 *)((int)s_SIMDRandContexts[0x19].m_RandY[0x21] + unaff_EBX + 8) = 0;
    uVar2 = *(undefined4 *)(&DAT_00ae9da8 + unaff_EBX);
    ___cxa_atexit(unaff_EBX + 0x9024bc,0,uVar2);
    *(undefined4 *)((int)&PTR_EstimateAbsVelocity_00bb2bc0 + unaff_EBX) = 1;
    *(int *)((int)&PTR_GetPlayerName_00bb2bbc + unaff_EBX) = unaff_EBX + 0xc5981c;
    *(int *)((int)s_SIMDRandContexts[0x19].m_RandY[0x21] + unaff_EBX + 0xc) =
         (int)&PTR_GetPlayerName_00bb2bbc + unaff_EBX;
    *(int *)((int)s_SIMDRandContexts[0x19].m_RandY[0x22] + unaff_EBX + 8) = unaff_EBX + 0x941815;
    *(int *)((int)s_SIMDRandContexts[0x19].m_RandY[0x22] + unaff_EBX) = unaff_EBX + 0x4c2fec;
    *(undefined4 *)((int)s_SIMDRandContexts[0x19].m_RandY[0x22] + unaff_EBX + 4) = 0;
    pRVar1 = (RecvTable *)((int)s_SIMDRandContexts[0x19].m_RandY[0x23] + unaff_EBX + 8);
    *(RecvTable **)((int)s_SIMDRandContexts[0x19].m_RandY[0x22] + unaff_EBX + 0xc) = pRVar1;
    piVar3 = *(int **)(&DAT_00ae9e20 + unaff_EBX);
    *(int *)((int)s_SIMDRandContexts[0x19].m_RandY[0x23] + unaff_EBX) = *piVar3;
    *piVar3 = unaff_EBX + 0xcef6a0;
    RecvTable::RecvTable(pRVar1);
    ___cxa_atexit(unaff_EBX + 0x9023bc,0,uVar2);
    iVar4 = ClientClassInit<DT_ProjectedTractorBeamEntity::ignored>((ignored *)0x0);
    *(int *)((int)s_SIMDRandContexts[0x19].m_RandY[0x24] + unaff_EBX + 0xc) = iVar4;
    pIVar5 = GetClassMap();
    (*pIVar5->_vptr_IClassMap[2])
              (pIVar5,unaff_EBX + 0x94184e,unaff_EBX + 0x941831,0xac4,&UNK_004c44fc + unaff_EBX);
    *(int *)((int)s_SIMDRandContexts[0x19].m_RandY[0x25] + unaff_EBX + 8) = unaff_EBX + 0x94186c;
    *(int *)((int)s_SIMDRandContexts[0x19].m_RandY[0x25] + unaff_EBX) = unaff_EBX + 0x4c5ebc;
    *(undefined4 *)((int)s_SIMDRandContexts[0x19].m_RandY[0x25] + unaff_EBX + 4) = 0;
    pRVar1 = (RecvTable *)((int)s_SIMDRandContexts[0x19].m_RandY[0x26] + unaff_EBX + 8);
    *(RecvTable **)((int)s_SIMDRandContexts[0x19].m_RandY[0x25] + unaff_EBX + 0xc) = pRVar1;
    *(int *)((int)s_SIMDRandContexts[0x19].m_RandY[0x26] + unaff_EBX) = *piVar3;
    *piVar3 = unaff_EBX + 0xcef6d0;
    RecvTable::RecvTable(pRVar1);
    ___cxa_atexit(unaff_EBX + 0x90239c,0,uVar2);
    iVar4 = ClientClassInit<DT_Trigger_TractorBeam::ignored>((ignored *)0x0);
    *(int *)((int)s_SIMDRandContexts[0x19].m_RandY[0x27] + unaff_EBX + 0xc) = iVar4;
    *(undefined4 *)((int)&PTR_PhysicsSimulate_00bb2be0 + unaff_EBX) = 2;
    *(int *)((int)&PTR_PhysicsSolidMaskForEntity_00bb2bdc + unaff_EBX) =
         (int)&PTR_GetSkin_00bb2c3c + unaff_EBX;
    *(int *)((int)s_SIMDRandContexts[0x19].m_RandY[0x28] + unaff_EBX) =
         (int)&PTR_PhysicsSolidMaskForEntity_00bb2bdc + unaff_EBX;
    pIVar5 = GetClassMap();
    (*pIVar5->_vptr_IClassMap[2])
              (pIVar5,unaff_EBX + 0x9148cc,(char *)(unaff_EBX + 0x9417fe),0xaec,unaff_EBX + 0x4c5e7c
              );
    CIFM_EntityKeyValuesHandler_RecreateEntities::CIFM_EntityKeyValuesHandler_RecreateEntities
              ((CIFM_EntityKeyValuesHandler_RecreateEntities *)
               ((int)&CBaseHudChat::GetMessageMap::s_pMap + unaff_EBX),
               (char *)(unaff_EBX + 0x9417fe));
    *(undefined **)((int)&CBaseHudChat::GetMessageMap::s_pMap + unaff_EBX) =
         &UNK_00b56004 + unaff_EBX;
    ___cxa_atexit(unaff_EBX + 0x9023dc,0,uVar2);
    pIVar6 = GetMaterialProxyDict();
    (*pIVar6->_vptr_IMaterialProxyDict[3])(pIVar6,unaff_EBX + 0x941881,unaff_EBX + 0x4c30ac);
  }
  return;
}


/* ITriggerTractorBeamAutoList::~ITriggerTractorBeamAutoList at 0053d420 */

/* DWARF original prototype: void ~ITriggerTractorBeamAutoList(ITriggerTractorBeamAutoList * this,
   int __in_chrg) */

void __thiscall
ITriggerTractorBeamAutoList::~ITriggerTractorBeamAutoList
          (ITriggerTractorBeamAutoList *this,int __in_chrg)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  this->_vptr_ITriggerTractorBeamAutoList = (_func_int_varargs **)(unaff_EBX + 0x692b2a);
  iVar1 = *(int *)(unaff_EBX + 0x82c22a);
  if (iVar1 < 1) {
    return;
  }
  piVar2 = *(int **)(unaff_EBX + 0x82c21e);
  if (this == (ITriggerTractorBeamAutoList *)*piVar2) {
    iVar4 = 0;
    iVar5 = 0;
  }
  else {
    iVar4 = 0;
    iVar3 = 4;
    do {
      iVar5 = iVar3;
      iVar4 = iVar4 + 1;
      if (iVar4 == iVar1) {
        return;
      }
      iVar3 = iVar5 + 4;
    } while (this != (ITriggerTractorBeamAutoList *)piVar2[iVar4]);
  }
  if (iVar4 != iVar1 + -1) {
    *(int *)(iVar5 + (int)piVar2) = piVar2[iVar1 + -1];
    iVar4 = *(int *)(unaff_EBX + 0x82c22a) + -1;
  }
  *(int *)(unaff_EBX + 0x82c22a) = iVar4;
  return;
}


/* ITriggerTractorBeamAutoList::~ITriggerTractorBeamAutoList at 0053e180 */

/* DWARF original prototype: void ~ITriggerTractorBeamAutoList(ITriggerTractorBeamAutoList * this,
   int __in_chrg) */

void __thiscall
ITriggerTractorBeamAutoList::~ITriggerTractorBeamAutoList
          (ITriggerTractorBeamAutoList *this,int __in_chrg)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  this->_vptr_ITriggerTractorBeamAutoList = (_func_int_varargs **)(unaff_EBX + 0x691dca);
  iVar1 = *(int *)(unaff_EBX + 0x82b4ca);
  if (0 < iVar1) {
    piVar2 = *(int **)(unaff_EBX + 0x82b4be);
    if (this == (ITriggerTractorBeamAutoList *)*piVar2) {
      iVar4 = 0;
      iVar5 = 0;
    }
    else {
      iVar4 = 0;
      iVar3 = 4;
      do {
        iVar5 = iVar3;
        iVar4 = iVar4 + 1;
        if (iVar4 == iVar1) goto LAB_0053e1e6;
        iVar3 = iVar5 + 4;
      } while (this != (ITriggerTractorBeamAutoList *)piVar2[iVar4]);
    }
    if (iVar4 != iVar1 + -1) {
      *(int *)(iVar5 + (int)piVar2) = piVar2[iVar1 + -1];
      iVar4 = *(int *)(unaff_EBX + 0x82b4ca) + -1;
    }
    *(int *)(unaff_EBX + 0x82b4ca) = iVar4;
  }
LAB_0053e1e6:
  operator_delete(this);
  return;
}


/* ITriggerTractorBeamAutoList::ITriggerTractorBeamAutoList at 0053e220 */

/* DWARF original prototype: void ITriggerTractorBeamAutoList(ITriggerTractorBeamAutoList * this,
   bool bAutoAdd) */

void __thiscall
ITriggerTractorBeamAutoList::ITriggerTractorBeamAutoList
          (ITriggerTractorBeamAutoList *this,bool bAutoAdd)

{
  uint uVar1;
  int iVar2;
  undefined4 *puVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  this->_vptr_ITriggerTractorBeamAutoList = (_func_int_varargs **)(unaff_EBX + 0x691d24);
  if (bAutoAdd) {
    uVar1 = *(uint *)(unaff_EBX + 0x82b424);
    CUtlVector<ITriggerTractorBeamAutoList*,CUtlMemory<ITriggerTractorBeamAutoList*,_int>_>::
    GrowVector((CUtlVector<ITriggerTractorBeamAutoList*,CUtlMemory<ITriggerTractorBeamAutoList*,_int>_>
                *)(unaff_EBX + 0x82b418),1);
    iVar2 = ~uVar1 + *(int *)(unaff_EBX + 0x82b424);
    if (0 < iVar2) {
      _V_memmove((void *)(*(int *)(unaff_EBX + 0x82b418) + 4 + uVar1 * 4),
                 (void *)(*(int *)(unaff_EBX + 0x82b418) + uVar1 * 4),iVar2 * 4);
    }
    puVar3 = (undefined4 *)(uVar1 * 4 + *(int *)(unaff_EBX + 0x82b418));
    if (puVar3 != (undefined4 *)0x0) {
      *puVar3 = this;
    }
  }
  return;
}


/* ITriggerTractorBeamAutoList::ITriggerTractorBeamAutoList at 0053e2d0 */

/* DWARF original prototype: void ITriggerTractorBeamAutoList(ITriggerTractorBeamAutoList * this,
   bool bAutoAdd) */

void __thiscall
ITriggerTractorBeamAutoList::ITriggerTractorBeamAutoList
          (ITriggerTractorBeamAutoList *this,bool bAutoAdd)

{
  uint uVar1;
  int iVar2;
  undefined4 *puVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  this->_vptr_ITriggerTractorBeamAutoList = (_func_int_varargs **)(unaff_EBX + 0x691c74);
  if (bAutoAdd) {
    uVar1 = *(uint *)(unaff_EBX + 0x82b374);
    CUtlVector<ITriggerTractorBeamAutoList*,CUtlMemory<ITriggerTractorBeamAutoList*,_int>_>::
    GrowVector((CUtlVector<ITriggerTractorBeamAutoList*,CUtlMemory<ITriggerTractorBeamAutoList*,_int>_>
                *)(unaff_EBX + 0x82b368),1);
    iVar2 = ~uVar1 + *(int *)(unaff_EBX + 0x82b374);
    if (0 < iVar2) {
      _V_memmove((void *)(*(int *)(unaff_EBX + 0x82b368) + 4 + uVar1 * 4),
                 (void *)(*(int *)(unaff_EBX + 0x82b368) + uVar1 * 4),iVar2 * 4);
    }
    puVar3 = (undefined4 *)(uVar1 * 4 + *(int *)(unaff_EBX + 0x82b368));
    if (puVar3 != (undefined4 *)0x0) {
      *puVar3 = this;
    }
  }
  return;
}


/* ITriggerTractorBeamAutoList::~ITriggerTractorBeamAutoList at 0053e380 */

/* DWARF original prototype: void ~ITriggerTractorBeamAutoList(ITriggerTractorBeamAutoList * this,
   int __in_chrg) */

void __thiscall
ITriggerTractorBeamAutoList::~ITriggerTractorBeamAutoList
          (ITriggerTractorBeamAutoList *this,int __in_chrg)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  this->_vptr_ITriggerTractorBeamAutoList = (_func_int_varargs **)(unaff_EBX + 0x691bca);
  iVar1 = *(int *)(unaff_EBX + 0x82b2ca);
  if (iVar1 < 1) {
    return;
  }
  piVar2 = *(int **)(unaff_EBX + 0x82b2be);
  if (this == (ITriggerTractorBeamAutoList *)*piVar2) {
    iVar4 = 0;
    iVar5 = 0;
  }
  else {
    iVar4 = 0;
    iVar3 = 4;
    do {
      iVar5 = iVar3;
      iVar4 = iVar4 + 1;
      if (iVar4 == iVar1) {
        return;
      }
      iVar3 = iVar5 + 4;
    } while (this != (ITriggerTractorBeamAutoList *)piVar2[iVar4]);
  }
  if (iVar4 != iVar1 + -1) {
    *(int *)(iVar5 + (int)piVar2) = piVar2[iVar1 + -1];
    iVar4 = *(int *)(unaff_EBX + 0x82b2ca) + -1;
  }
  *(int *)(unaff_EBX + 0x82b2ca) = iVar4;
  return;
}


/* C_Trigger_TractorBeam::C_Trigger_TractorBeam at 0053f9c0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void C_Trigger_TractorBeam(C_Trigger_TractorBeam * this) */

void __thiscall C_Trigger_TractorBeam::C_Trigger_TractorBeam(C_Trigger_TractorBeam *this)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  int *piVar4;
  int iVar5;
  undefined4 *puVar6;
  int iVar7;
  int unaff_EBX;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::C_BaseEntity((C_BaseEntity *)this);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x624c22) + 8);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable
  ._vptr_IClientRenderable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x624c22) + 0x3a4);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.
  super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x624c22) + 0x45c);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x624c22) + 0x498);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x624c22) + 0x4b4);
  (this->super_IMotionEvent)._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x62ef3a);
  (this->super_ITriggerTractorBeamAutoList)._vptr_ITriggerTractorBeamAutoList =
       (_func_int_varargs **)(&UNK_0069058a + unaff_EBX);
  uVar1 = *(uint *)(unaff_EBX + 0x829c8a);
  CUtlVector<ITriggerTractorBeamAutoList*,CUtlMemory<ITriggerTractorBeamAutoList*,_int>_>::
  GrowVector((CUtlVector<ITriggerTractorBeamAutoList*,CUtlMemory<ITriggerTractorBeamAutoList*,_int>_>
              *)(unaff_EBX + 0x829c7e),1);
  iVar3 = ~uVar1 + *(int *)(unaff_EBX + 0x829c8a);
  if (0 < iVar3) {
    _V_memmove((void *)(*(int *)(unaff_EBX + 0x829c7e) + 4 + uVar1 * 4),
               (void *)(*(int *)(unaff_EBX + 0x829c7e) + uVar1 * 4),iVar3 * 4);
  }
  piVar4 = (int *)(uVar1 * 4 + *(int *)(unaff_EBX + 0x829c7e));
  if (piVar4 != (int *)0x0) {
    *piVar4 = (int)&this->super_ITriggerTractorBeamAutoList;
  }
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x68fbba);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable
  ._vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x68ff5e);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.
  super_IClientNetworkable._vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x690016);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x690052);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(unaff_EBX + 0x69006e);
  (this->super_IMotionEvent)._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x69007a);
  (this->super_ITriggerTractorBeamAutoList)._vptr_ITriggerTractorBeamAutoList =
       (_func_int_varargs **)(unaff_EBX + 0x690086);
  (this->m_hProxyEntity).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hCoreEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
  (this->m_hCoreEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
  (this->m_hCoreEffect).m_pObject = (CNewParticleEffect *)0x0;
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.field_0xa5d = 0;
  iVar7 = *(int *)(unaff_EBX + 0x624c1e);
  iVar2 = *(int *)(iVar7 + 0xc);
  iVar3 = iVar2 + 1;
  iVar5 = *(int *)(iVar7 + 4);
  if (iVar5 < iVar3) {
    iVar7 = *(int *)(iVar7 + 8);
    if (-1 < iVar7) {
      if (iVar7 == 0) {
        if ((iVar5 == 0) && (iVar5 = 8, iVar3 < 9)) {
          local_20 = 0x20;
        }
        else {
          do {
            local_20 = iVar5;
            iVar5 = local_20 * 2;
          } while (iVar5 < iVar3);
          local_20 = local_20 * 8;
        }
        goto LAB_0053fbb0;
      }
      iVar5 = (iVar2 / iVar7 + 1) * iVar7;
      if (iVar5 < iVar3) {
        if (iVar5 != 0) {
LAB_0053fca0:
          do {
            iVar5 = (iVar5 + iVar3) / 2;
          } while (iVar5 < iVar3);
          goto LAB_0053fcb0;
        }
        if (iVar3 < 0) {
          iVar5 = -1;
          local_20 = -4;
        }
        else {
          local_20 = 0;
          if (iVar3 != 0) goto LAB_0053fca0;
        }
      }
      else {
LAB_0053fcb0:
        local_20 = iVar5 * 4;
      }
LAB_0053fbb0:
      piVar4 = *(int **)(unaff_EBX + 0x624c1e);
      piVar4[1] = iVar5;
      iVar7 = *piVar4;
      if (iVar7 == 0) {
        iVar7 = (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x6243a2))
                          ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x6243a2),local_20);
      }
      else {
        iVar7 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x6243a2) + 4))
                          ((int *)**(undefined4 **)(unaff_EBX + 0x6243a2),iVar7,local_20);
      }
      piVar4 = *(int **)(unaff_EBX + 0x624c1e);
      *piVar4 = iVar7;
      iVar5 = piVar4[3];
      piVar4[3] = iVar5 + 1;
      iVar7 = *piVar4;
      piVar4[4] = iVar7;
      iVar5 = (iVar5 + 1) - iVar2;
      goto joined_r0x0053fb58;
    }
  }
  piVar4 = *(int **)(unaff_EBX + 0x624c1e);
  piVar4[3] = iVar3;
  iVar7 = *piVar4;
  piVar4[4] = iVar7;
  iVar5 = iVar3 - iVar2;
joined_r0x0053fb58:
  if (0 < iVar5 + -1) {
    _V_memmove((void *)(iVar7 + iVar3 * 4),(void *)(iVar7 + iVar2 * 4),(iVar5 + -1) * 4);
  }
  puVar6 = (undefined4 *)(iVar2 * 4 + **(int **)(unaff_EBX + 0x624c1e));
  if (puVar6 != (undefined4 *)0x0) {
    *puVar6 = this;
  }
  return;
}


/* C_Trigger_TractorBeam::~C_Trigger_TractorBeam at 0053fee0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void ~C_Trigger_TractorBeam(C_Trigger_TractorBeam * this, int
   __in_chrg) */

void __thiscall
C_Trigger_TractorBeam::~C_Trigger_TractorBeam(C_Trigger_TractorBeam *this,int __in_chrg)

{
  int *piVar1;
  CNewParticleEffect *pCVar2;
  CUtlReference<CNewParticleEffect> *pCVar3;
  CUtlReference<CNewParticleEffect> *pCVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int unaff_EBX;
  int local_44;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x68f69a);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable
  ._vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x68fa3e);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.
  super_IClientNetworkable._vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x68faf6);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x68fb32);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(unaff_EBX + 0x68fb4e);
  (this->super_IMotionEvent)._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x68fb5a);
  (this->super_ITriggerTractorBeamAutoList)._vptr_ITriggerTractorBeamAutoList =
       (_func_int_varargs **)(unaff_EBX + 0x68fb66);
  local_44 = *(int *)(*(int *)(&DAT_006246fe + unaff_EBX) + 0xc);
  if (0 < local_44) {
    piVar1 = (int *)**(undefined4 **)(&DAT_006246fe + unaff_EBX);
    if (this == (C_Trigger_TractorBeam *)*piVar1) {
      iVar7 = 0;
      iVar6 = 0;
    }
    else {
      iVar7 = 0;
      iVar8 = 4;
      do {
        iVar6 = iVar8;
        iVar7 = iVar7 + 1;
        if (iVar7 == local_44) goto LAB_0053ffaa;
        iVar8 = iVar6 + 4;
      } while (this != (C_Trigger_TractorBeam *)piVar1[iVar7]);
    }
    iVar8 = (local_44 - iVar7) + -1;
    if (0 < iVar8) {
      _V_memmove((void *)(iVar6 + (int)piVar1),piVar1 + iVar7 + 1,iVar8 * 4);
      local_44 = *(int *)(*(int *)(&DAT_006246fe + unaff_EBX) + 0xc);
    }
    *(int *)(*(int *)(&DAT_006246fe + unaff_EBX) + 0xc) = local_44 + -1;
  }
LAB_0053ffaa:
  pCVar2 = (this->m_hCoreEffect).m_pObject;
  if (pCVar2 != (CNewParticleEffect *)0x0) {
    pCVar3 = (this->m_hCoreEffect).m_pPrev;
    if (pCVar3 == (CUtlReference<CNewParticleEffect> *)0x0) {
      if ((pCVar2->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
          &this->m_hCoreEffect) {
        pCVar3 = (this->m_hCoreEffect).m_pNext;
        (pCVar2->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
        super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar3;
        if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar3->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
        }
      }
    }
    else {
      pCVar3->m_pNext = (this->m_hCoreEffect).m_pNext;
      pCVar4 = (this->m_hCoreEffect).m_pNext;
      if (pCVar4 != (CUtlReference<CNewParticleEffect> *)0x0) {
        pCVar4->m_pPrev = pCVar3;
      }
    }
    (this->m_hCoreEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
    (this->m_hCoreEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
    (this->m_hCoreEffect).m_pObject = (CNewParticleEffect *)0x0;
  }
  (this->super_ITriggerTractorBeamAutoList)._vptr_ITriggerTractorBeamAutoList =
       (_func_int_varargs **)(unaff_EBX + 0x69006a);
  iVar6 = *(int *)(unaff_EBX + 0x82976a);
  if (0 < iVar6) {
    piVar1 = *(int **)(unaff_EBX + 0x82975e);
    if (&this->super_ITriggerTractorBeamAutoList == (ITriggerTractorBeamAutoList *)*piVar1) {
      iVar7 = 0;
      iVar8 = 0;
    }
    else {
      iVar7 = 0;
      iVar5 = 4;
      do {
        iVar8 = iVar5;
        iVar7 = iVar7 + 1;
        if (iVar7 == iVar6) goto LAB_0054006a;
        iVar5 = iVar8 + 4;
      } while (&this->super_ITriggerTractorBeamAutoList !=
               (ITriggerTractorBeamAutoList *)piVar1[iVar7]);
    }
    if (iVar7 != iVar6 + -1) {
      *(int *)(iVar8 + (int)piVar1) = piVar1[iVar6 + -1];
      iVar7 = *(int *)(unaff_EBX + 0x82976a) + -1;
    }
    *(int *)(unaff_EBX + 0x82976a) = iVar7;
  }
LAB_0054006a:
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(&DAT_00624702 + unaff_EBX) + 8);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable
  ._vptr_IClientRenderable = (_func_int_varargs **)(*(int *)(&DAT_00624702 + unaff_EBX) + 0x3a4);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.
  super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(*(int *)(&DAT_00624702 + unaff_EBX) + 0x45c);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(*(int *)(&DAT_00624702 + unaff_EBX) + 0x498);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(*(int *)(&DAT_00624702 + unaff_EBX) + 0x4b4)
  ;
  C_BaseEntity::~C_BaseEntity((C_BaseEntity *)this,__in_chrg);
  return;
}


/* C_Trigger_TractorBeam::~C_Trigger_TractorBeam at 00540300 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void ~C_Trigger_TractorBeam(C_Trigger_TractorBeam * this, int
   __in_chrg) */

void __thiscall
C_Trigger_TractorBeam::~C_Trigger_TractorBeam(C_Trigger_TractorBeam *this,int __in_chrg)

{
  int *piVar1;
  CNewParticleEffect *pCVar2;
  CUtlReference<CNewParticleEffect> *pCVar3;
  CUtlReference<CNewParticleEffect> *pCVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int unaff_EBX;
  int *in_stack_ffffff98;
  int local_44;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x68f27a);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable
  ._vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x68f61e);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.
  super_IClientNetworkable._vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x68f6d6);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x68f712);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(unaff_EBX + 0x68f72e);
  (this->super_IMotionEvent)._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x68f73a);
  (this->super_ITriggerTractorBeamAutoList)._vptr_ITriggerTractorBeamAutoList =
       (_func_int_varargs **)(unaff_EBX + 0x68f746);
  local_44 = *(int *)(*(int *)(unaff_EBX + 0x6242de) + 0xc);
  if (0 < local_44) {
    piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x6242de);
    if (this == (C_Trigger_TractorBeam *)*piVar1) {
      iVar7 = 0;
      iVar6 = 0;
    }
    else {
      iVar7 = 0;
      iVar8 = 4;
      do {
        iVar6 = iVar8;
        iVar7 = iVar7 + 1;
        if (iVar7 == local_44) goto LAB_005403ca;
        iVar8 = iVar6 + 4;
      } while (this != (C_Trigger_TractorBeam *)piVar1[iVar7]);
    }
    iVar8 = (local_44 - iVar7) + -1;
    if (0 < iVar8) {
      in_stack_ffffff98 = piVar1 + iVar7 + 1;
      _V_memmove((void *)(iVar6 + (int)piVar1),in_stack_ffffff98,iVar8 * 4);
      local_44 = *(int *)(*(int *)(unaff_EBX + 0x6242de) + 0xc);
    }
    *(int *)(*(int *)(unaff_EBX + 0x6242de) + 0xc) = local_44 + -1;
  }
LAB_005403ca:
  pCVar2 = (this->m_hCoreEffect).m_pObject;
  if (pCVar2 != (CNewParticleEffect *)0x0) {
    pCVar3 = (this->m_hCoreEffect).m_pPrev;
    if (pCVar3 == (CUtlReference<CNewParticleEffect> *)0x0) {
      if ((pCVar2->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
          &this->m_hCoreEffect) {
        pCVar3 = (this->m_hCoreEffect).m_pNext;
        (pCVar2->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
        super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar3;
        if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar3->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
        }
      }
    }
    else {
      pCVar3->m_pNext = (this->m_hCoreEffect).m_pNext;
      pCVar4 = (this->m_hCoreEffect).m_pNext;
      if (pCVar4 != (CUtlReference<CNewParticleEffect> *)0x0) {
        pCVar4->m_pPrev = pCVar3;
      }
    }
    (this->m_hCoreEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
    (this->m_hCoreEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
    (this->m_hCoreEffect).m_pObject = (CNewParticleEffect *)0x0;
  }
  (this->super_ITriggerTractorBeamAutoList)._vptr_ITriggerTractorBeamAutoList =
       (_func_int_varargs **)(unaff_EBX + 0x68fc4a);
  iVar6 = *(int *)(unaff_EBX + 0x82934a);
  if (0 < iVar6) {
    piVar1 = *(int **)(unaff_EBX + 0x82933e);
    if (&this->super_ITriggerTractorBeamAutoList == (ITriggerTractorBeamAutoList *)*piVar1) {
      iVar7 = 0;
      iVar8 = 0;
    }
    else {
      iVar7 = 0;
      iVar5 = 4;
      do {
        iVar8 = iVar5;
        iVar7 = iVar7 + 1;
        if (iVar7 == iVar6) goto LAB_0054048a;
        iVar5 = iVar8 + 4;
      } while (&this->super_ITriggerTractorBeamAutoList !=
               (ITriggerTractorBeamAutoList *)piVar1[iVar7]);
    }
    if (iVar7 != iVar6 + -1) {
      *(int *)(iVar8 + (int)piVar1) = piVar1[iVar6 + -1];
      iVar7 = *(int *)(unaff_EBX + 0x82934a) + -1;
    }
    *(int *)(unaff_EBX + 0x82934a) = iVar7;
  }
LAB_0054048a:
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x6242e2) + 8);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable
  ._vptr_IClientRenderable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x6242e2) + 0x3a4);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.
  super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x6242e2) + 0x45c);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x6242e2) + 0x498);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x6242e2) + 0x4b4);
  C_BaseEntity::~C_BaseEntity((C_BaseEntity *)this,(int)in_stack_ffffff98);
  C_BaseEntity::operator_delete(this);
  return;
}


/* __tcf_0 at 0097c480 */

void __tcf_0(void *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  *(undefined4 *)(&DAT_003ed1c4 + unaff_EBX) = 0;
  if (*(int *)(unaff_EBX + 0x3ed1c0) < 0) {
    iVar1 = *(int *)(unaff_EBX + 0x3ed1b8);
  }
  else {
    if (*(int *)(unaff_EBX + 0x3ed1b8) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(&DAT_001e78dc + unaff_EBX) + 8))
                ((int *)**(undefined4 **)(&DAT_001e78dc + unaff_EBX),*(int *)(unaff_EBX + 0x3ed1b8))
      ;
      *(undefined4 *)(unaff_EBX + 0x3ed1b8) = 0;
    }
    iVar1 = 0;
    *(undefined4 *)(unaff_EBX + 0x3ed1bc) = 0;
  }
  *(int *)(&DAT_003ed1c8 + unaff_EBX) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x3ed1c0)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(&DAT_001e78dc + unaff_EBX) + 8))
                ((int *)**(undefined4 **)(&DAT_001e78dc + unaff_EBX),iVar1);
      *(undefined4 *)(unaff_EBX + 0x3ed1b8) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x3ed1bc) = 0;
  }
  return;
}


/* C_ProjectedTractorBeamEntity::GetPredDescMap at 0053cd40 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: datamap_t * GetPredDescMap(C_ProjectedTractorBeamEntity * this) */

datamap_t * __thiscall
C_ProjectedTractorBeamEntity::GetPredDescMap(C_ProjectedTractorBeamEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(&UNK_006efe38 + extraout_ECX);
}


/* PredMapInit<C_ProjectedTractorBeamEntity> at 0053cd50 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

datamap_t * PredMapInit<C_ProjectedTractorBeamEntity>(C_ProjectedTractorBeamEntity *param_1)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *(undefined4 *)(&DAT_006efe29 + extraout_ECX) = 1;
  *(int *)(&DAT_006efe25 + extraout_ECX) = extraout_ECX + 0x796a85;
  return (datamap_t *)(&DAT_006efe25 + extraout_ECX);
}


/* ClientClassInit<DT_ProjectedTractorBeamEntity::ignored> at 00079e60 */

int ClientClassInit<DT_ProjectedTractorBeamEntity::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)NPaintRenderer::g_FastSpherePosData[0xb] + unaff_EBX + 0x14) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc59894);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)NPaintRenderer::g_FastSpherePosData[0xc] + unaff_EBX + 0xc),
                  &UNK_009108fc + unaff_EBX,0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)NPaintRenderer::g_FastSpherePosData[0xe] + unaff_EBX + 8),
                        (char *)(unaff_EBX + 0x915440),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00aea75c + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00ae9f78 + unaff_EBX));
      RecvPropEHandle((RecvProp *)((int)NPaintRenderer::g_FastSpherePosData[0x10] + unaff_EBX + 4),
                      (char *)(unaff_EBX + 0x94191a),0xac0,4,
                      *(RecvVarProxyFn *)(&DAT_00ae9f74 + unaff_EBX));
      ___cxa_guard_release(unaff_EBX + 0xc59894);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_SIMDRandContexts[0x1a].m_RandY[0] + unaff_EBX + 8),
             (RecvProp_conflict1 *)((int)NPaintRenderer::g_FastSpherePosData[0xe] + unaff_EBX + 8),2
             ,(char *)(unaff_EBX + 0x941930));
  return 1;
}


/* C_ProjectedTractorBeamEntity::YouForgotToImplementOrDeclareClientClass at 0053cd80 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: int
   YouForgotToImplementOrDeclareClientClass(C_ProjectedTractorBeamEntity * this) */

int __thiscall
C_ProjectedTractorBeamEntity::YouForgotToImplementOrDeclareClientClass
          (C_ProjectedTractorBeamEntity *this)

{
  return 0;
}


/* C_ProjectedTractorBeamEntity::GetClientClass at 0053cda0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: ClientClass * GetClientClass(C_ProjectedTractorBeamEntity * this) */

ClientClass * __thiscall
C_ProjectedTractorBeamEntity::GetClientClass(C_ProjectedTractorBeamEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x82c8bc);
}


/* _C_ProjectedTractorBeamEntity_CreateObject at 0053cfb0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_ProjectedTractorBeamEntity_CreateObject(int entnum,int serialNum)

{
  C_BaseProjectedEntity *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_ProjectedTractorBeamEntity * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xac4);
  C_BaseProjectedEntity::C_BaseProjectedEntity(this);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x692aa4);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x692e74);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x692f2c);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x692f68);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x692f84);
  this[1].super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)0xffffffff;
  (**(code **)(unaff_EBX + 0x692b60))(this,entnum,serialNum);
  return &(this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable;
}


/* __tcf_1 at 0097c380 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3ed2f4),in_stack_00000008);
  return;
}


/* CC_ProjectedTractorBeamEntityFactory at 0053e4c0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

C_BaseEntity * CC_ProjectedTractorBeamEntityFactory(void)

{
  C_BaseProjectedEntity *this;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xac4);
  C_BaseProjectedEntity::C_BaseProjectedEntity(this);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x691594);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(&UNK_00691964 + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x691a1c);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(&UNK_00691a58 + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(CUnderWaterView::Draw + unaff_EBX + 4);
  this[1].super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)0xffffffff;
  return &this->super_C_BaseEntity;
}


/* C_Trigger_TractorBeam::RecvProxy_Start at 0053dce0 */

void C_Trigger_TractorBeam::RecvProxy_Start(CRecvProxyData *pData,void *pStruct,void *pOut)

{
  float fVar1;
  float fVar2;
  float fVar3;
  int extraout_ECX;
  
                    /* Unresolved local var: C_Trigger_TractorBeam * pBeam@[???]
                       Unresolved local var: Vector vStart@[???] */
  ___i686_get_pc_thunk_cx();
  fVar1 = (pData->m_Value).field_0.m_Vector[2];
  fVar2 = (pData->m_Value).field_0.m_Vector[1];
  fVar3 = (pData->m_Value).field_0.m_Float;
  if ((((fVar3 != *(float *)((int)pStruct + 0xa6c)) ||
       (NAN(fVar3) || NAN(*(float *)((int)pStruct + 0xa6c)))) ||
      (fVar2 != *(float *)((int)pStruct + 0xa70))) ||
     ((NAN(fVar2) || NAN(*(float *)((int)pStruct + 0xa70)) ||
      (fVar1 != *(float *)((int)pStruct + 0xa74))))) {
    *(float *)((int)pStruct + 0xa6c) = fVar3;
    *(float *)((int)pStruct + 0xa70) = fVar2;
    *(float *)((int)pStruct + 0xa74) = fVar1;
    *(undefined4 *)((int)pStruct + 0xae8) =
         *(undefined4 *)(**(int **)(extraout_ECX + 0x6260b4) + 0xc);
    *(undefined1 *)((int)pStruct + 0xa9c) = 1;
  }
  return;
}


/* C_Trigger_TractorBeam::RecvProxy_End at 0053e0b0 */

void C_Trigger_TractorBeam::RecvProxy_End(CRecvProxyData *pData,void *pStruct,void *pOut)

{
  float fVar1;
  float fVar2;
  float fVar3;
  int extraout_ECX;
  
                    /* Unresolved local var: C_Trigger_TractorBeam * pBeam@[???]
                       Unresolved local var: Vector vEnd@[???] */
  ___i686_get_pc_thunk_cx();
  fVar1 = (pData->m_Value).field_0.m_Vector[2];
  fVar2 = (pData->m_Value).field_0.m_Vector[1];
  fVar3 = (pData->m_Value).field_0.m_Float;
  if ((((fVar3 != *(float *)((int)pStruct + 0xa78)) ||
       (NAN(fVar3) || NAN(*(float *)((int)pStruct + 0xa78)))) ||
      (fVar2 != *(float *)((int)pStruct + 0xa7c))) ||
     ((NAN(fVar2) || NAN(*(float *)((int)pStruct + 0xa7c)) ||
      (fVar1 != *(float *)((int)pStruct + 0xa80))))) {
    *(float *)((int)pStruct + 0xa78) = fVar3;
    *(float *)((int)pStruct + 0xa7c) = fVar2;
    *(float *)((int)pStruct + 0xa80) = fVar1;
    *(undefined4 *)((int)pStruct + 0xae8) =
         *(undefined4 *)(**(int **)(extraout_ECX + 0x625ce4) + 0xc);
    *(undefined1 *)((int)pStruct + 0xa9c) = 1;
  }
  return;
}


/* ClientClassInit<DT_Trigger_TractorBeam::ignored> at 000798f0 */

int ClientClassInit<DT_Trigger_TractorBeam::ignored>(ignored *param_1)

{
  RecvVarProxyFn varProxy;
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)NPaintRenderer::g_FastSpherePosData[0xb] + unaff_EBX + 2) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc59882);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)NPaintRenderer::g_FastSpherePosData[0xc] + unaff_EBX + 2),
                  (char *)(unaff_EBX + 0x910e72),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)NPaintRenderer::g_FastSpherePosData[0xd] + unaff_EBX + 0x1e),
                        (char *)(unaff_EBX + 0x9159b6),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00aeacea + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00aea4ee + unaff_EBX));
      RecvPropBool((RecvProp *)((int)NPaintRenderer::g_FastSpherePosData[0xf] + unaff_EBX + 0x1a),
                   (char *)(unaff_EBX + 0x941d46),0xa5d,1);
      varProxy = *(RecvVarProxyFn *)(&DAT_00aea52e + unaff_EBX);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)NPaintRenderer::g_FastSpherePosData[0x11] + unaff_EBX + 0x16),
                    (char *)(unaff_EBX + 0x941d52),0xab4,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)NPaintRenderer::g_FastSpherePosData[0x13] + unaff_EBX + 0x12),
                    (char *)(unaff_EBX + 0x941d61),0xab8,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)NPaintRenderer::g_FastSpherePosData[0x15] + unaff_EBX + 0xe),
                    (char *)(unaff_EBX + 0x941d71),0xabc,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)NPaintRenderer::g_FastSpherePosData[0x17] + unaff_EBX + 10),
                    (char *)(unaff_EBX + 0x941d7f),0xac0,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)NPaintRenderer::g_FastSpherePosData[0x19] + unaff_EBX + 6),
                    (char *)(unaff_EBX + 0x941d92),0xac4,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)NPaintRenderer::g_FastSpherePosData[0x1b] + unaff_EBX + 2),
                    (char *)(unaff_EBX + 0x941da4),0xac8,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)NPaintRenderer::g_FastSpherePosData[0x1c] + unaff_EBX + 0x1e),
                    (char *)(unaff_EBX + 0x941db7),0xacc,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)NPaintRenderer::g_FastSpherePosData[0x1e] + unaff_EBX + 0x1a),
                    (char *)(unaff_EBX + 0x941dce),0xad0,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)NPaintRenderer::g_FastSpherePosData[0x20] + unaff_EBX + 0x16),
                    (char *)(unaff_EBX + 0x941ddc),0xad4,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)NPaintRenderer::g_FastSpherePosData[0x22] + unaff_EBX + 0x12),
                    (char *)(unaff_EBX + 0x941deb),0xad8,4,0,varProxy);
      RecvPropVector((RecvProp_conflict1 *)
                     ((int)NPaintRenderer::g_FastSpherePosData[0x24] + unaff_EBX + 0xe),
                     (char *)(unaff_EBX + 0x941dfa),0xadc,0xc,0,
                     *(RecvVarProxyFn *)(&DAT_00aea592 + unaff_EBX));
      RecvPropEHandle((RecvProp *)((int)NPaintRenderer::g_FastSpherePosData[0x26] + unaff_EBX + 10),
                      (char *)(unaff_EBX + 0x941e0e),0xaa0,4,
                      *(RecvVarProxyFn *)(&DAT_00aea4ea + unaff_EBX));
      RecvPropVector((RecvProp_conflict1 *)
                     ((int)NPaintRenderer::g_FastSpherePosData[0x28] + unaff_EBX + 6),
                     (char *)(unaff_EBX + 0x941e1d),0xa6c,0xc,0,
                     CPhysicsSystem::LevelInitPostEntity + unaff_EBX + 2);
      RecvPropVector((RecvProp_conflict1 *)
                     ((int)NPaintRenderer::g_FastSpherePosData[0x2a] + unaff_EBX + 2),
                     (char *)(unaff_EBX + 0x941e26),0xa78,0xc,0,
                     (RecvVarProxyFn)(unaff_EBX + 0x4c47b2));
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)NPaintRenderer::g_FastSpherePosData[0x2b] + unaff_EBX + 0x1e),
                    (char *)(unaff_EBX + 0x941e2d),0xa84,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)NPaintRenderer::g_FastSpherePosData[0x2d] + unaff_EBX + 0x1a),
                    (char *)(unaff_EBX + 0x91127d),0xa88,4,0,varProxy);
      RecvPropBool((RecvProp *)((int)NPaintRenderer::g_FastSpherePosData[0x2f] + unaff_EBX + 0x16),
                   (char *)(unaff_EBX + 0x941e3e),0xa8c,1);
      RecvPropBool((RecvProp *)((int)NPaintRenderer::g_FastSpherePosData[0x31] + unaff_EBX + 0x12),
                   (char *)(unaff_EBX + 0x941e4a),0xa8d,1);
      RecvPropBool((RecvProp *)(unaff_EBX + 0xc59d8e),(char *)(unaff_EBX + 0x941e58),0xa8e,1);
      RecvPropBool((RecvProp *)((int)&CBaseSaveGameDialog::GetKBMap::s_pMap + unaff_EBX + 2),
                   (char *)(unaff_EBX + 0x941e64),0xa8f,1);
      ___cxa_guard_release(unaff_EBX + 0xc59882);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_SIMDRandContexts[0x1b].m_RandY[0x22] + unaff_EBX + 0xe),
             (RecvProp_conflict1 *)
             ((int)NPaintRenderer::g_FastSpherePosData[0xd] + unaff_EBX + 0x1e),0x16,
             (char *)(unaff_EBX + 0x941e79));
  return 1;
}


/* C_Trigger_TractorBeam::YouForgotToImplementOrDeclareClientClass at 0053cdb0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_Trigger_TractorBeam *
   this) */

int __thiscall
C_Trigger_TractorBeam::YouForgotToImplementOrDeclareClientClass(C_Trigger_TractorBeam *this)

{
  return 0;
}


/* C_Trigger_TractorBeam::GetClientClass at 0053cdd0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: ClientClass * GetClientClass(C_Trigger_TractorBeam * this) */

ClientClass * __thiscall C_Trigger_TractorBeam::GetClientClass(C_Trigger_TractorBeam *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x82c8bc);
}


/* _C_Trigger_TractorBeam_CreateObject at 0053fe80 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */

IClientNetworkable * _C_Trigger_TractorBeam_CreateObject(int entnum,int serialNum)

{
  C_Trigger_TractorBeam *this;
  IClientNetworkable *pIVar1;
  
                    /* Unresolved local var: C_Trigger_TractorBeam * pRet@[???] */
  this = C_BaseEntity::operator_new(0xaec);
  C_Trigger_TractorBeam::C_Trigger_TractorBeam(this);
  pIVar1 = (IClientNetworkable *)0x0;
  if (this != (C_Trigger_TractorBeam *)0x0) {
    (*(this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.
      super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0x2f])(this,entnum,serialNum);
    pIVar1 = &(this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.
              super_IClientNetworkable;
  }
  return pIVar1;
}


/* __tcf_2 at 0097c360 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3ed344),in_stack_00000008);
  return;
}


/* C_Trigger_TractorBeam::GetPredDescMap at 0053cde0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: datamap_t * GetPredDescMap(C_Trigger_TractorBeam * this) */

datamap_t * __thiscall C_Trigger_TractorBeam::GetPredDescMap(C_Trigger_TractorBeam *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x6efdb8);
}


/* PredMapInit<C_Trigger_TractorBeam> at 0053cdf0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */

datamap_t * PredMapInit<C_Trigger_TractorBeam>(C_Trigger_TractorBeam *param_1)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *(undefined4 *)(extraout_ECX + 0x6efda9) = 2;
  *(undefined **)(extraout_ECX + 0x6efda5) = &UNK_006efe05 + extraout_ECX;
  return (datamap_t *)(extraout_ECX + 0x6efda5);
}


/* CC_Trigger_TractorBeamFactory at 0053fe40 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */

C_BaseEntity * CC_Trigger_TractorBeamFactory(void)

{
  C_Trigger_TractorBeam *this;
  
  this = C_BaseEntity::operator_new(0xaec);
  C_Trigger_TractorBeam::C_Trigger_TractorBeam(this);
  return (C_BaseEntity *)this;
}


/* C_Trigger_TractorBeam::C_Trigger_TractorBeam at 0053fe30 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void C_Trigger_TractorBeam(C_Trigger_TractorBeam * this,
   C_Trigger_TractorBeam * this) */

void __thiscall
C_Trigger_TractorBeam::C_Trigger_TractorBeam
          (C_Trigger_TractorBeam *this,C_Trigger_TractorBeam *this_1)

{
  C_Trigger_TractorBeam(this);
  return;
}


/* C_Trigger_TractorBeam_NonConformantDataHandler::CreateInstance at 00540bf0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void * CreateInstance(C_Trigger_TractorBeam_NonConformantDataHandler *
   this) */

void * __thiscall
C_Trigger_TractorBeam_NonConformantDataHandler::CreateInstance
          (C_Trigger_TractorBeam_NonConformantDataHandler *this)

{
  C_Trigger_TractorBeam *this_00;
  
  this_00 = C_BaseEntity::operator_new(0xaec);
  C_Trigger_TractorBeam::C_Trigger_TractorBeam(this_00);
  return this_00;
}


/* C_Trigger_TractorBeam::~C_Trigger_TractorBeam at 005402e0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void ~C_Trigger_TractorBeam(C_Trigger_TractorBeam * this, int
   __in_chrg, C_Trigger_TractorBeam * this) */

void __thiscall
C_Trigger_TractorBeam::~C_Trigger_TractorBeam
          (C_Trigger_TractorBeam *this,int __in_chrg,C_Trigger_TractorBeam *this_1)

{
  ~C_Trigger_TractorBeam(this,__in_chrg);
  return;
}


/* C_Trigger_TractorBeam::Spawn at 0053ce90 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void Spawn(C_Trigger_TractorBeam * this) */

void __thiscall C_Trigger_TractorBeam::Spawn(C_Trigger_TractorBeam *this)

{
  IMaterial *pIVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::Spawn((C_BaseEntity *)this);
  if (this->m_pMaterial1 == (IMaterial *)0x0) {
    pIVar1 = (IMaterial *)
             (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x62706b) + 0x130))
                       ((int *)**(undefined4 **)(unaff_EBX + 0x62706b),unaff_EBX + 0x47e710,0,0,0);
    this->m_pMaterial1 = pIVar1;
  }
  if (this->m_pMaterial2 == (IMaterial *)0x0) {
    pIVar1 = (IMaterial *)
             (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x62706b) + 0x130))
                       ((int *)**(undefined4 **)(unaff_EBX + 0x62706b),unaff_EBX + 0x47e725,0,0,0);
    this->m_pMaterial2 = pIVar1;
  }
  if (this->m_pMaterial3 == (IMaterial *)0x0) {
    pIVar1 = (IMaterial *)
             (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x62706b) + 0x130))
                       ((int *)**(undefined4 **)(unaff_EBX + 0x62706b),unaff_EBX + 0x47e73b,0,0,0);
    this->m_pMaterial3 = pIVar1;
  }
  (*(this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x9d])(this,0xc4a1a000);
  return;
}


/* C_Trigger_TractorBeam::CreateParticles at 0053d4c0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void CreateParticles(C_Trigger_TractorBeam * this) */

void __thiscall C_Trigger_TractorBeam::CreateParticles(C_Trigger_TractorBeam *this)

{
  NetworkVar_m_Particles *this_00;
  uint uVar1;
  float fVar2;
  CNewParticleEffect *pCVar3;
  CUtlReference<CNewParticleEffect> *pCVar4;
  CUtlReference<CNewParticleEffect> *pCVar5;
  Vector *pVVar6;
  CNewParticleEffect *pEffect;
  int unaff_EBX;
  Vector local_b8;
  undefined4 local_ac;
  undefined4 local_a8;
  undefined4 local_a4;
  float local_a0;
  float local_9c;
  float local_98;
  Vector local_94;
  Vector local_88;
  Vector local_7c;
  Vector local_70;
  float local_64;
  float local_60;
  float local_5c;
  Vector local_58;
  vec_t local_4c;
  vec_t local_48;
  vec_t local_44;
  vec_t local_40;
  vec_t local_3c;
  vec_t local_38;
  vec_t local_34;
  vec_t local_30;
  vec_t local_2c;
  vec_t local_28;
  vec_t local_24;
  vec_t local_20;
  
  ___i686_get_pc_thunk_bx();
  pCVar3 = (this->m_hCoreEffect).m_pObject;
  this_00 = &(this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.m_Particles;
  if (pCVar3 != (CNewParticleEffect *)0x0) {
    CParticleProperty::StopEmission(&this_00->super_CParticleProperty,pCVar3,false,false,false,true)
    ;
    pCVar3 = (this->m_hCoreEffect).m_pObject;
    if (pCVar3 != (CNewParticleEffect *)0x0) {
      pCVar4 = (this->m_hCoreEffect).m_pPrev;
      if (pCVar4 == (CUtlReference<CNewParticleEffect> *)0x0) {
        if ((pCVar3->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
            &this->m_hCoreEffect) {
          pCVar4 = (this->m_hCoreEffect).m_pNext;
          (pCVar3->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar4;
          if (pCVar4 != (CUtlReference<CNewParticleEffect> *)0x0) {
            pCVar4->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
          }
        }
      }
      else {
        pCVar4->m_pNext = (this->m_hCoreEffect).m_pNext;
        pCVar5 = (this->m_hCoreEffect).m_pNext;
        if (pCVar5 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar5->m_pPrev = pCVar4;
        }
      }
      (this->m_hCoreEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
      (this->m_hCoreEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
      (this->m_hCoreEffect).m_pObject = (CNewParticleEffect *)0x0;
    }
  }
  pVVar6 = *(Vector **)(&DAT_0062692f + unaff_EBX);
  local_28 = pVVar6->x;
  local_24 = pVVar6->y;
  local_20 = pVVar6->z;
  pEffect = CParticleProperty::Create
                      (&this_00->super_CParticleProperty,(char *)(unaff_EBX + 0x47e161),
                       PATTACH_CUSTOMORIGIN,-1,*pVVar6,(matrix3x4_t *)0x0);
  pCVar3 = (this->m_hCoreEffect).m_pObject;
  if (pEffect != pCVar3) {
    if (pCVar3 != (CNewParticleEffect *)0x0) {
      pCVar4 = (this->m_hCoreEffect).m_pPrev;
      if (pCVar4 == (CUtlReference<CNewParticleEffect> *)0x0) {
        if ((pCVar3->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
            &this->m_hCoreEffect) {
          pCVar4 = (this->m_hCoreEffect).m_pNext;
          (pCVar3->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar4;
          if (pCVar4 != (CUtlReference<CNewParticleEffect> *)0x0) {
            pCVar4->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
          }
        }
      }
      else {
        pCVar4->m_pNext = (this->m_hCoreEffect).m_pNext;
        pCVar5 = (this->m_hCoreEffect).m_pNext;
        if (pCVar5 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar5->m_pPrev = pCVar4;
        }
      }
      (this->m_hCoreEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
      (this->m_hCoreEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
      (this->m_hCoreEffect).m_pObject = (CNewParticleEffect *)0x0;
    }
    (this->m_hCoreEffect).m_pObject = pEffect;
    if (pEffect == (CNewParticleEffect *)0x0) {
      return;
    }
    (this->m_hCoreEffect).m_pNext =
         (pEffect->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
         super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead;
    pCVar4 = (pEffect->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
             super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead;
    if (pCVar4 != (CUtlReference<CNewParticleEffect> *)0x0) {
      pCVar4->m_pPrev = &this->m_hCoreEffect;
    }
    (this->m_hCoreEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
    (pEffect->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
    super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = &this->m_hCoreEffect;
    pEffect = (this->m_hCoreEffect).m_pObject;
  }
  if (pEffect != (CNewParticleEffect *)0x0) {
    pVVar6 = *(Vector **)(&DAT_0062692f + unaff_EBX);
    local_34 = pVVar6->x;
    local_30 = pVVar6->y;
    local_2c = pVVar6->z;
    CParticleProperty::AddControlPoint
              (&this_00->super_CParticleProperty,pEffect,1,(C_BaseEntity *)this,PATTACH_CUSTOMORIGIN
               ,(char *)0x0,*pVVar6,(matrix3x4_t *)0x0);
    pVVar6 = *(Vector **)(&DAT_0062692f + unaff_EBX);
    local_40 = pVVar6->x;
    local_3c = pVVar6->y;
    local_38 = pVVar6->z;
    CParticleProperty::AddControlPoint
              (&this_00->super_CParticleProperty,(this->m_hCoreEffect).m_pObject,2,
               (C_BaseEntity *)this,PATTACH_CUSTOMORIGIN,(char *)0x0,*pVVar6,(matrix3x4_t *)0x0);
    local_4c = pVVar6->x;
    local_48 = pVVar6->y;
    local_44 = pVVar6->z;
    CParticleProperty::AddControlPoint
              (&this_00->super_CParticleProperty,(this->m_hCoreEffect).m_pObject,3,
               (C_BaseEntity *)this,PATTACH_CUSTOMORIGIN,(char *)0x0,*pVVar6,(matrix3x4_t *)0x0);
                    /* Unresolved local var: Vector res@[???] */
    local_64 = (this->m_vEnd).x - (this->m_vStart).x;
    local_60 = (this->m_vEnd).y - (this->m_vStart).y;
    local_5c = (this->m_vEnd).z - (this->m_vStart).z;
    local_58.x = local_64;
    local_58.y = local_60;
    local_58.z = local_5c;
    VectorNormalize(&local_58);
    VectorVectors(&local_58,&local_70,&local_7c);
    CNewParticleEffect::SetControlPoint((this->m_hCoreEffect).m_pObject,0,&this->m_vStart);
    CNewParticleEffect::SetControlPointOrientation
              ((this->m_hCoreEffect).m_pObject,0,&local_58,&local_70,&local_7c);
    CNewParticleEffect::SetControlPoint((this->m_hCoreEffect).m_pObject,1,&this->m_vEnd);
    uVar1 = *(uint *)(unaff_EBX + 0x4a9e0f);
    local_88.z = (vec_t)((uint)local_58.z ^ uVar1);
    local_88.y = (vec_t)((uint)local_58.y ^ uVar1);
    local_88.x = (vec_t)((uint)local_58.x ^ uVar1);
    CNewParticleEffect::SetControlPointOrientation
              ((this->m_hCoreEffect).m_pObject,1,&local_88,&local_70,&local_7c);
    fVar2 = this->m_linearForce;
                    /* Unresolved local var: matrix3x4_t * mat@[???] */
    C_BaseEntity::CalcAbsolutePosition((C_BaseEntity *)this);
                    /* Unresolved local var: Vector res@[???] */
    local_a0 = fVar2 * (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.m_rgflCoordinateFrame.
                       m_flMatVal[0][0];
    local_9c = (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.m_rgflCoordinateFrame.
               m_flMatVal[1][0] * fVar2;
    local_98 = fVar2 * (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.m_rgflCoordinateFrame.
                       m_flMatVal[2][0];
    local_94.x = local_a0;
    local_94.y = local_9c;
    local_94.z = local_98;
    CNewParticleEffect::SetControlPoint((this->m_hCoreEffect).m_pObject,2,&local_94);
    if (this->m_bReversed == false) {
      local_ac = 0x42800000;
      local_a4 = 0x437f0000;
    }
    else {
      local_ac = 0x437f0000;
      local_a4 = 0x42000000;
    }
    local_a8 = 0x43200000;
    local_b8.x = 64.0;
    local_b8.y = 160.0;
    local_b8.z = 255.0;
    CNewParticleEffect::SetControlPoint((this->m_hCoreEffect).m_pObject,3,&local_b8);
  }
  return;
}


/* C_Trigger_TractorBeam::UpdateOnRemove at 0053db60 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void UpdateOnRemove(C_Trigger_TractorBeam * this) */

void __thiscall C_Trigger_TractorBeam::UpdateOnRemove(C_Trigger_TractorBeam *this)

{
  IPhysicsMotionController *pIVar1;
  _func_int_varargs **pp_Var2;
  _func_int_varargs *p_Var3;
  int *piVar4;
  int iVar5;
  code *pcVar6;
  CNewParticleEffect *pCVar7;
  CUtlReference<CNewParticleEffect> *pCVar8;
  CUtlReference<CNewParticleEffect> *pCVar9;
  int iVar10;
  C_Trigger_TractorBeam *pCVar11;
  int iVar12;
  int unaff_EBX;
  IPhysicsMotionController **ppIVar13;
  int iVar14;
  undefined4 uStack_60;
  IPhysicsMotionController *local_5c;
  int local_58 [7];
  undefined1 auStack_3c [8];
  undefined1 *local_34;
  C_Trigger_TractorBeam *local_30;
  int local_20;
  
  ppIVar13 = &local_5c;
  uStack_60 = 0x53db6e;
  ___i686_get_pc_thunk_bx();
  local_30 = this;
  local_20 = **(int **)(unaff_EBX + 0x6261f2);
  pIVar1 = this->m_pController;
  if (pIVar1 != (IPhysicsMotionController *)0x0) {
                    /* Unresolved local var: int nNumObjects@[???]
                       Unresolved local var: IPhysicsObject * * pObjects@[???] */
    uStack_60 = 0x53db97;
    local_5c = pIVar1;
    iVar12 = (*pIVar1->_vptr_IPhysicsMotionController[5])();
    iVar10 = -((iVar12 * 4 + 0xfU & 0xfffffff0) + 0x10);
    ppIVar13 = (IPhysicsMotionController **)((int)local_58 + iVar10 + -4);
    local_34 = auStack_3c + iVar10;
    pIVar1 = local_30->m_pController;
    pp_Var2 = pIVar1->_vptr_IPhysicsMotionController;
    *(undefined1 **)((int)local_58 + iVar10) = local_34;
    *(IPhysicsMotionController **)((int)local_58 + iVar10 + -4) = pIVar1;
    p_Var3 = pp_Var2[6];
    *(undefined4 *)((int)&uStack_60 + iVar10) = 0x53dbc4;
    (*p_Var3)();
                    /* Unresolved local var: int i@[???] */
    if (0 < iVar12) {
      iVar14 = 0;
      do {
        piVar4 = *(int **)(local_34 + iVar14 * 4);
        if (piVar4 != (int *)0x0) {
          iVar5 = *piVar4;
          *(int **)((int)local_58 + iVar10 + -4) = piVar4;
          pcVar6 = *(code **)(iVar5 + 100);
          *(undefined4 *)((int)&uStack_60 + iVar10) = 0x53dbe2;
          (*pcVar6)();
        }
        iVar14 = iVar14 + 1;
      } while (iVar14 != iVar12);
    }
    pCVar11 = local_30;
    piVar4 = (int *)**(undefined4 **)(unaff_EBX + 0x6263ea);
    iVar12 = *piVar4;
    *(IPhysicsMotionController **)((int)local_58 + iVar10) = local_30->m_pController;
    *(int **)((int)local_58 + iVar10 + -4) = piVar4;
    pcVar6 = *(code **)(iVar12 + 0x7c);
    *(undefined4 *)((int)&uStack_60 + iVar10) = 0x53dc04;
    (*pcVar6)();
    pCVar11->m_pController = (IPhysicsMotionController *)0x0;
  }
  pCVar11 = local_30;
  pCVar7 = (local_30->m_hCoreEffect).m_pObject;
  if (pCVar7 != (CNewParticleEffect *)0x0) {
    ppIVar13[5] = (IPhysicsMotionController *)0x1;
    ppIVar13[4] = (IPhysicsMotionController *)0x0;
    ppIVar13[3] = (IPhysicsMotionController *)0x0;
    ppIVar13[2] = (IPhysicsMotionController *)0x0;
    ppIVar13[1] = (IPhysicsMotionController *)pCVar7;
    *ppIVar13 = (IPhysicsMotionController *)
                &(pCVar11->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.m_Particles;
    ppIVar13[-1] = (IPhysicsMotionController *)0x53dc47;
    CParticleProperty::StopEmission
              ((CParticleProperty *)*ppIVar13,(CNewParticleEffect *)ppIVar13[1],SUB41(ppIVar13[2],0)
               ,SUB41(ppIVar13[3],0),SUB41(ppIVar13[4],0),SUB41(ppIVar13[5],0));
    pCVar7 = (local_30->m_hCoreEffect).m_pObject;
    if (pCVar7 != (CNewParticleEffect *)0x0) {
      pCVar8 = (local_30->m_hCoreEffect).m_pPrev;
      if (pCVar8 == (CUtlReference<CNewParticleEffect> *)0x0) {
        if ((pCVar7->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
            &local_30->m_hCoreEffect) {
          pCVar8 = (local_30->m_hCoreEffect).m_pNext;
          (pCVar7->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar8;
          if (pCVar8 != (CUtlReference<CNewParticleEffect> *)0x0) {
            pCVar8->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
          }
        }
      }
      else {
        pCVar8->m_pNext = (local_30->m_hCoreEffect).m_pNext;
        pCVar9 = (local_30->m_hCoreEffect).m_pNext;
        if (pCVar9 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar9->m_pPrev = pCVar8;
        }
      }
      (local_30->m_hCoreEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
      (local_30->m_hCoreEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
      (local_30->m_hCoreEffect).m_pObject = (CNewParticleEffect *)0x0;
    }
  }
  *ppIVar13 = (IPhysicsMotionController *)local_30;
  ppIVar13[-1] = (IPhysicsMotionController *)0x53dc99;
  C_BaseEntity::UpdateOnRemove((C_BaseEntity *)*ppIVar13);
  if (local_20 == **(int **)(unaff_EBX + 0x6261f2)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  ppIVar13[-1] = (IPhysicsMotionController *)&UNK_0053dcdf;
  ___stack_chk_fail();
}


/* C_Trigger_TractorBeam::OnDataChanged at 0053dab0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void OnDataChanged(C_Trigger_TractorBeam * this, DataUpdateType_t
   updateType) */

void __thiscall
C_Trigger_TractorBeam::OnDataChanged(C_Trigger_TractorBeam *this,DataUpdateType_t updateType)

{
  int *piVar1;
  IPhysicsMotionController *pIVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::OnDataChanged((C_BaseEntity *)this,updateType);
  if (updateType == DATA_UPDATE_CREATED) {
    C_BaseEntity::AddEffects((C_BaseEntity *)this,0x400);
    piVar1 = (int *)**(int **)(unaff_EBX + 0x626494);
    if ((piVar1 != (int *)0x0) && (this->m_pController == (IPhysicsMotionController *)0x0)) {
      pIVar2 = (IPhysicsMotionController *)
               (**(code **)(*piVar1 + 0x78))(piVar1,&this->super_IMotionEvent);
      this->m_pController = pIVar2;
      UpdateBeam(this);
    }
  }
  else {
    if (this->m_bRecreateParticles == false) {
      return;
    }
    this->m_bRecreateParticles = false;
  }
  CreateParticles(this);
  return;
}


/* C_Trigger_TractorBeam::GetPredictionOwner at 0053e130 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: C_BasePlayer * GetPredictionOwner(C_Trigger_TractorBeam * this) */

C_BasePlayer * __thiscall C_Trigger_TractorBeam::GetPredictionOwner(C_Trigger_TractorBeam *this)

{
  uint uVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.m_hPlayerSimulationOwner.
          super_CBaseHandle.m_Index;
  if ((uVar1 != 0xffffffff) &&
     (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x625ca3),
     *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) {
    return *(C_BasePlayer **)(iVar2 + 4);
  }
  return (C_BasePlayer *)0x0;
}


/* C_Trigger_TractorBeam::UpdatePartitionListEntry at 0053ce20 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void UpdatePartitionListEntry(C_Trigger_TractorBeam * this) */

void __thiscall C_Trigger_TractorBeam::UpdatePartitionListEntry(C_Trigger_TractorBeam *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (**(code **)(*(int *)**(undefined4 **)(CSurveyPanel::GetMessageMap + unaff_EBX) + 0x14))
            ((int *)**(undefined4 **)(CSurveyPanel::GetMessageMap + unaff_EBX),0x8c,0x100,
             (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.m_Collision.
             super_CCollisionProperty.m_Partition);
  return;
}


/* C_Trigger_TractorBeam::GetSoundSpatialization at 0053df20 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: bool GetSoundSpatialization(C_Trigger_TractorBeam * this,
   SpatializationInfo_t * info) */

bool __thiscall
C_Trigger_TractorBeam::GetSoundSpatialization
          (C_Trigger_TractorBeam *this,SpatializationInfo_t *info)

{
  Vector *pVVar1;
  QAngle *pQVar2;
  int iVar3;
  vec_t *pvVar4;
  bool bVar5;
  int unaff_EBX;
  float local_20 [4];
  
  ___i686_get_pc_thunk_bx();
  iVar3 = (*(this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.
            super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0x4d])(this);
  bVar5 = false;
  if ((char)iVar3 == '\0') {
    if (info->pOrigin != (Vector *)0x0) {
      CalcClosestPointOnLine
                (&(info->info).vListenerOrigin,&this->m_vStart,&this->m_vEnd,info->pOrigin,local_20)
      ;
      if ((*(float *)(unaff_EBX + 0x4a434c) <= local_20[0]) ||
         (NAN(local_20[0]) || NAN(*(float *)(unaff_EBX + 0x4a434c)))) {
        if (*(float *)(unaff_EBX + 0x4a4250) <= local_20[0] &&
            local_20[0] != *(float *)(unaff_EBX + 0x4a4250)) {
          pVVar1 = info->pOrigin;
          pVVar1->x = (this->m_vEnd).x;
          pVVar1->y = (this->m_vEnd).y;
          pVVar1->z = (this->m_vEnd).z;
        }
      }
      else {
        pVVar1 = info->pOrigin;
        pVVar1->x = (this->m_vStart).x;
        pVVar1->y = (this->m_vStart).y;
        pVVar1->z = (this->m_vStart).z;
      }
    }
    pQVar2 = info->pAngles;
    bVar5 = true;
    if (pQVar2 != (QAngle *)0x0) {
      pvVar4 = (vec_t *)(*(this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.m_Collision.
                          super_CCollisionProperty.super_ICollideable._vptr_ICollideable[9])
                                  (&(this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.
                                    m_Collision);
      pQVar2->x = *pvVar4;
      pQVar2->y = pvVar4[1];
      pQVar2->z = pvVar4[2];
      bVar5 = true;
    }
  }
  return bVar5;
}


/* C_Trigger_TractorBeam::StartTouch at 0053e420 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void StartTouch(C_Trigger_TractorBeam * this, C_BaseEntity * pOther) */

void __thiscall C_Trigger_TractorBeam::StartTouch(C_Trigger_TractorBeam *this,C_BaseEntity *pOther)

{
  IPhysicsMotionController *pIVar1;
  int iVar2;
  
  pIVar1 = this->m_pController;
                    /* Unresolved local var: IPhysicsObject * pPhysObject@[???] */
  if ((pIVar1 != (IPhysicsMotionController *)0x0) &&
     (pOther->m_pPhysicsObject != (IPhysicsObject *)0x0)) {
    (*pIVar1->_vptr_IPhysicsMotionController[3])(pIVar1,pOther->m_pPhysicsObject,1);
  }
  iVar2 = (*(pOther->super_IClientEntity).super_IClientUnknown.super_IHandleEntity.
            _vptr_IHandleEntity[0xc1])(pOther);
  if ((char)iVar2 != '\0') {
                    /* Unresolved local var: C_BasePlayer * pPlayer@[???] */
    iVar2 = (*(pOther->super_IClientEntity).super_IClientUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[0xc1])(pOther);
    if ((char)iVar2 != '\0') {
      C_BasePlayer::SetPhysicsFlag((C_BasePlayer *)pOther,0x10,true);
      *(undefined1 *)&pOther[2].m_VarMap.m_nInterpolatedEntries = 1;
      pOther->m_flGravity = 1.1754944e-38;
    }
  }
  C_BaseEntity::StartTouch((C_BaseEntity *)this,pOther);
  return;
}


/* C_Trigger_TractorBeam::EndTouch at 0053e020 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void EndTouch(C_Trigger_TractorBeam * this, C_BaseEntity * pOther) */

void __thiscall C_Trigger_TractorBeam::EndTouch(C_Trigger_TractorBeam *this,C_BaseEntity *pOther)

{
  IPhysicsMotionController *pIVar1;
  int iVar2;
  
  pIVar1 = this->m_pController;
                    /* Unresolved local var: IPhysicsObject * pPhysObject@[???] */
  if ((pIVar1 != (IPhysicsMotionController *)0x0) &&
     (pOther->m_pPhysicsObject != (IPhysicsObject *)0x0)) {
    (*pIVar1->_vptr_IPhysicsMotionController[4])(pIVar1,pOther->m_pPhysicsObject);
  }
  iVar2 = (*(pOther->super_IClientEntity).super_IClientUnknown.super_IHandleEntity.
            _vptr_IHandleEntity[0xc1])(pOther);
  if ((char)iVar2 != '\0') {
                    /* Unresolved local var: C_BasePlayer * pPlayer@[???] */
    iVar2 = (*(pOther->super_IClientEntity).super_IClientUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[0xc1])(pOther);
    if ((char)iVar2 != '\0') {
      C_BasePlayer::SetPhysicsFlag((C_BasePlayer *)pOther,0x10,false);
      *(undefined1 *)&pOther[2].m_VarMap.m_nInterpolatedEntries = 0;
      pOther->m_flGravity = 1.0;
    }
  }
  C_BaseEntity::EndTouch((C_BaseEntity *)this,pOther);
  return;
}


/* C_Trigger_TractorBeam::PhysicsSimulate at 0053cfa0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void PhysicsSimulate(C_Trigger_TractorBeam * this) */

void __thiscall C_Trigger_TractorBeam::PhysicsSimulate(C_Trigger_TractorBeam *this)

{
  C_BaseEntity::PhysicsSimulate((C_BaseEntity *)this);
  return;
}


/* C_Trigger_TractorBeam::DrawColumnSegment at 0053e560 */

/* WARNING: Restarted to delay deadcode elimination for space: stack */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void DrawColumnSegment(C_Trigger_TractorBeam * this, CMeshBuilder *
   meshBuilder, Vector * vecStart, Vector * vDir, float flLength, Vector * vecXAxis, Vector *
   vecYAxis, float flRadius, float flAlpha, float flTextureOffset) */

void __thiscall
C_Trigger_TractorBeam::DrawColumnSegment
          (C_Trigger_TractorBeam *this,CMeshBuilder *meshBuilder,Vector *vecStart,Vector *vDir,
          float flLength,Vector *vecXAxis,Vector *vecYAxis,float flRadius,float flAlpha,
          float flTextureOffset)

{
  float **ppfVar1;
  uchar **ppuVar2;
  int iVar3;
  float *pfVar4;
  longdouble lVar5;
  int iVar6;
  int unaff_EBX;
  longdouble lVar7;
  float fVar8;
  float fVar9;
  float fVar10;
  float fVar11;
  float fVar12;
  float fVar13;
  float fVar14;
  float fVar15;
  float fVar16;
  float fVar17;
  float fVar18;
  int local_118;
  float local_104;
  float local_dc;
  float local_d8;
  float local_d4;
  float local_d0;
  
                    /* Unresolved local var: Vector vStart@[???]
                       Unresolved local var: Vector vecPosition@[???]
                       Unresolved local var: Vector vecLastPosition@[???]
                       Unresolved local var: float flRadStep@[???]
                       Unresolved local var: float flSin@[???]
                       Unresolved local var: float flCos@[???]
                       Unresolved local var: float flV@[???]
                       Unresolved local var: int r@[???]
                       Unresolved local var: int g@[???]
                       Unresolved local var: int b@[???]
                       Unresolved local var: float flLastV@[???] */
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
  fVar17 = vecStart->x;
  fVar15 = vecStart->y;
  fVar14 = vecStart->z;
  if (this->m_bReversed == false) {
    fVar12 = *(float *)(unaff_EBX + 0x4a8d57);
    local_104 = *(float *)(unaff_EBX + 0x4a3c13);
  }
  else {
    fVar12 = *(float *)(unaff_EBX + 0x4a3c13);
    local_104 = *(float *)(unaff_EBX + 0x4a8d53);
  }
  fVar10 = flLength * *(float *)(unaff_EBX + 0x4a720b);
  fVar9 = *(float *)(unaff_EBX + 0x4a3fcf);
  fVar8 = *(float *)(unaff_EBX + 0x4a5ae3);
  fVar12 = fVar12 * flAlpha * fVar9 + fVar8;
  fVar13 = flAlpha * *(float *)(unaff_EBX + 0x4a8d5b) * fVar9 + fVar8;
  fVar8 = fVar8 + fVar9 * flAlpha * local_104;
  local_dc = 0.0;
  local_118 = 1;
  iVar3 = *(int *)(unaff_EBX + 0x4a878f);
  local_d8 = fVar14 + flRadius * vecXAxis->z;
  local_d4 = fVar15 + flRadius * vecXAxis->y;
  local_d0 = fVar17 + flRadius * vecXAxis->x;
  while( true ) {
                    /* Unresolved local var: Vector vert@[???]
                       Unresolved local var: float flU@[???]
                       Unresolved local var: float[4] pColors@[???] */
                    /* Unresolved local var: double __cosr@[???]
                       Unresolved local var: double __sinr@[???] */
    lVar7 = (longdouble)(*(float *)(unaff_EBX + 0x4a8d5f) * (float)local_118);
    lVar5 = (longdouble)fcos(lVar7);
    lVar7 = (longdouble)fsin(lVar7);
    fVar9 = (float)lVar7;
    fVar11 = (float)lVar5;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
    fVar18 = fVar17 + fVar11 * vecXAxis->x * flRadius + fVar9 * vecYAxis->x * flRadius;
    fVar16 = fVar15 + fVar11 * vecXAxis->y * flRadius + fVar9 * vecYAxis->y * flRadius;
    fVar11 = fVar14 + fVar11 * vecXAxis->z * flRadius + fVar9 * vecYAxis->z * flRadius;
    fVar9 = (float)local_118 * *(float *)(&LAB_004a409b + unaff_EBX);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
    *(uint *)(meshBuilder->m_VertexBuilder).m_pCurrColor =
         ((uint)fVar13 & 0xff) << 8 | ((uint)fVar8 & 0xff) << 0x10 | (uint)fVar12 & 0xff |
         iVar3 << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
    pfVar4 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[0];
    *pfVar4 = 0.0;
    pfVar4[1] = fVar9;
                    /* Unresolved local var: float * pDst@[???] */
    pfVar4 = (meshBuilder->m_VertexBuilder).m_pCurrPosition;
    *pfVar4 = fVar18;
    pfVar4[1] = fVar16;
    pfVar4[2] = fVar11;
    iVar6 = (meshBuilder->m_VertexBuilder).m_nCurrentVertex + 1;
    (meshBuilder->m_VertexBuilder).m_nCurrentVertex = iVar6;
    if ((meshBuilder->m_VertexBuilder).m_nVertexCount < iVar6) {
      (meshBuilder->m_VertexBuilder).m_nVertexCount = iVar6;
    }
    ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrPosition;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Position);
    ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrNormal;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Normal);
    ppuVar2 = &(meshBuilder->m_VertexBuilder).m_pCurrColor;
    *ppuVar2 = *ppuVar2 + (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Color;
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[0]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 1;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[1]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 2;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[2]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 3;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[3]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 4;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[4]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 5;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[5]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 6;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[6]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 7;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[7]);
                    /* Unresolved local var: Vector res@[???] */
    fVar17 = vDir->x;
    fVar15 = vDir->y;
    fVar14 = vDir->z;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
    *(uint *)(meshBuilder->m_VertexBuilder).m_pCurrColor =
         ((uint)fVar13 & 0xff) << 8 | ((uint)fVar8 & 0xff) << 0x10 | (uint)fVar12 & 0xff |
         iVar3 << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
    pfVar4 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[0];
    *pfVar4 = fVar10;
    pfVar4[1] = fVar9;
                    /* Unresolved local var: float * pDst@[???] */
    pfVar4 = (meshBuilder->m_VertexBuilder).m_pCurrPosition;
    *pfVar4 = fVar18 + flLength * fVar17;
    pfVar4[1] = fVar16 + flLength * fVar15;
    pfVar4[2] = fVar11 + flLength * fVar14;
    iVar6 = (meshBuilder->m_VertexBuilder).m_nCurrentVertex + 1;
    (meshBuilder->m_VertexBuilder).m_nCurrentVertex = iVar6;
    if ((meshBuilder->m_VertexBuilder).m_nVertexCount < iVar6) {
      (meshBuilder->m_VertexBuilder).m_nVertexCount = iVar6;
    }
    ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrPosition;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Position);
    ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrNormal;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Normal);
    ppuVar2 = &(meshBuilder->m_VertexBuilder).m_pCurrColor;
    *ppuVar2 = *ppuVar2 + (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Color;
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[0]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 1;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[1]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 2;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[2]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 3;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[3]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 4;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[4]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 5;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[5]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 6;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[6]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 7;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[7]);
                    /* Unresolved local var: Vector res@[???] */
    fVar17 = vDir->x;
    fVar15 = vDir->y;
    fVar14 = vDir->z;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
    *(uint *)(meshBuilder->m_VertexBuilder).m_pCurrColor =
         ((uint)fVar13 & 0xff) << 8 | ((uint)fVar8 & 0xff) << 0x10 | (uint)fVar12 & 0xff |
         iVar3 << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
    pfVar4 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[0];
    *pfVar4 = fVar10;
    pfVar4[1] = local_dc;
                    /* Unresolved local var: float * pDst@[???] */
    pfVar4 = (meshBuilder->m_VertexBuilder).m_pCurrPosition;
    *pfVar4 = local_d0 + flLength * fVar17;
    pfVar4[1] = local_d4 + flLength * fVar15;
    pfVar4[2] = local_d8 + flLength * fVar14;
    iVar6 = (meshBuilder->m_VertexBuilder).m_nCurrentVertex + 1;
    (meshBuilder->m_VertexBuilder).m_nCurrentVertex = iVar6;
    if ((meshBuilder->m_VertexBuilder).m_nVertexCount < iVar6) {
      (meshBuilder->m_VertexBuilder).m_nVertexCount = iVar6;
    }
    ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrPosition;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Position);
    ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrNormal;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Normal);
    ppuVar2 = &(meshBuilder->m_VertexBuilder).m_pCurrColor;
    *ppuVar2 = *ppuVar2 + (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Color;
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[0]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 1;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[1]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 2;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[2]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 3;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[3]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 4;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[4]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 5;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[5]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 6;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[6]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 7;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[7]);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
    *(uint *)(meshBuilder->m_VertexBuilder).m_pCurrColor =
         ((uint)fVar13 & 0xff) << 8 | ((uint)fVar8 & 0xff) << 0x10 | (uint)fVar12 & 0xff |
         iVar3 << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
    pfVar4 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[0];
    *pfVar4 = 0.0;
    pfVar4[1] = local_dc;
                    /* Unresolved local var: float * pDst@[???] */
    pfVar4 = (meshBuilder->m_VertexBuilder).m_pCurrPosition;
    *pfVar4 = local_d0;
    pfVar4[1] = local_d4;
    pfVar4[2] = local_d8;
    iVar6 = (meshBuilder->m_VertexBuilder).m_nCurrentVertex + 1;
    (meshBuilder->m_VertexBuilder).m_nCurrentVertex = iVar6;
    if ((meshBuilder->m_VertexBuilder).m_nVertexCount < iVar6) {
      (meshBuilder->m_VertexBuilder).m_nVertexCount = iVar6;
    }
    ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrPosition;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Position);
    ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrNormal;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Normal);
    ppuVar2 = &(meshBuilder->m_VertexBuilder).m_pCurrColor;
    *ppuVar2 = *ppuVar2 + (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Color;
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[0]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 1;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[1]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 2;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[2]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 3;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[3]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 4;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[4]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 5;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[5]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 6;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[6]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 7;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[7]);
    local_118 = local_118 + 1;
    if (local_118 == 0x41) break;
                    /* Unresolved local var: int i@[???] */
    fVar17 = vecStart->x;
    fVar15 = vecStart->y;
    fVar14 = vecStart->z;
    local_dc = fVar9;
    local_d8 = fVar11;
    local_d4 = fVar16;
    local_d0 = fVar18;
  }
                    /* Unresolved local var: Vector res@[???] */
  fVar17 = vDir->y;
  fVar15 = vDir->z;
                    /* Unresolved local var: Vector res@[???] */
  vecStart->x = vecStart->x + flLength * vDir->x;
  vecStart->y = vecStart->y + flLength * fVar17;
  vecStart->z = vecStart->z + flLength * fVar15;
  return;
}


/* C_Trigger_TractorBeam::DrawColumn at 0053ef30 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void DrawColumn(C_Trigger_TractorBeam * this, IMaterial * pMaterial,
   Vector * vecStart, Vector * vDir, float flLength, Vector * vecXAxis, Vector * vecYAxis, float
   flRadius, float flAlpha, bool bPinchIn, bool bPinchOut, float flTextureOffset) */

void __thiscall
C_Trigger_TractorBeam::DrawColumn
          (C_Trigger_TractorBeam *this,IMaterial *pMaterial,Vector *vecStart,Vector *vDir,
          float flLength,Vector *vecXAxis,Vector *vecYAxis,float flRadius,float flAlpha,
          bool bPinchIn,bool bPinchOut,float flTextureOffset)

{
  code *pcVar1;
  int *piVar2;
  int iVar3;
  IMesh *pIVar4;
  int unaff_EBX;
  CMeshBuilder local_214;
  Vector local_2c;
  int *local_20;
  
                    /* Unresolved local var: CMatRenderContextPtr pRenderContext@[???]
                       Unresolved local var: IMesh * pMesh@[???]
                       Unresolved local var: CMeshBuilder meshBuilder@[???]
                       Unresolved local var: int nTotalSegments@[???]
                       Unresolved local var: float flMainLength@[???]
                       Unresolved local var: Vector vStart@[???] */
  ___i686_get_pc_thunk_bx();
  local_20 = (int *)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x624fcb) + 0x1a0))
                              ((int *)**(undefined4 **)(unaff_EBX + 0x624fcb));
  if (local_20 != (int *)0x0) {
    (**(code **)(*local_20 + 8))(local_20);
  }
  piVar2 = local_20;
  pcVar1 = *(code **)(*local_20 + 0x24);
  iVar3 = (*(this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.
            super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[6])(this);
  (*pcVar1)(piVar2,pMaterial,iVar3);
  pIVar4 = (IMesh *)(**(code **)(*local_20 + 0xe4))(local_20,1,0,0,pMaterial);
  local_214.m_IndexBuilder.m_pIndexBuffer = (IIndexBuffer *)0x0;
  local_214.m_IndexBuilder.m_nMaxIndexCount = 0;
  local_214.m_IndexBuilder.m_nIndexCount = 0;
  local_214.m_IndexBuilder.m_nCurrentIndex = 0;
  local_214.m_IndexBuilder.m_nTotalIndexCount = 0;
  local_214.m_IndexBuilder.m_nBufferOffset = 0xffffffff;
  local_214.m_IndexBuilder.m_nBufferFirstIndex = 0;
  local_214.m_VertexBuilder.m_pVertexBuffer = (IVertexBuffer *)0x0;
  local_214.m_VertexBuilder.m_nBufferOffset = 0xffffffff;
  local_214.m_VertexBuilder.m_nBufferFirstVertex = 0;
  local_214.m_VertexBuilder.m_nVertexCount = 0;
  local_214.m_VertexBuilder.m_nCurrentVertex = 0;
  local_214.m_VertexBuilder.m_nMaxVertexCount = 0;
  local_214.m_VertexBuilder.m_nTotalVertexCount = 0;
  local_214.m_VertexBuilder.super_VertexDesc_t.m_CompressionType = VERTEX_COMPRESSION_INVALID;
  iVar3 = (-(uint)!bPinchOut & 0xfffffe00) + 0x240;
  if (bPinchIn) {
    iVar3 = (-(uint)!bPinchOut & 0xfffffe00) + 0x440;
  }
  local_214.m_bGenerateIndices = true;
  local_214.m_Type = MATERIAL_QUADS;
  local_214.m_pMesh = pIVar4;
  (*(pIVar4->super_IVertexBuffer)._vptr_IVertexBuffer[10])(pIVar4,2);
  (*((local_214.m_pMesh)->super_IVertexBuffer)._vptr_IVertexBuffer[0x11])
            (local_214.m_pMesh,iVar3 << 4,iVar3 * 0x18,&local_214,0);
  local_214.m_IndexBuilder.m_pIndexBuffer = (IIndexBuffer *)0x0;
  if (pIVar4 != (IMesh *)0x0) {
    local_214.m_IndexBuilder.m_pIndexBuffer = &pIVar4->super_IIndexBuffer;
  }
  local_214.m_IndexBuilder.m_nIndexCount = 0;
  local_214.m_IndexBuilder.m_bModify = false;
  local_214.m_IndexBuilder.m_nIndexOffset =
       local_214.super_MeshDesc_t.super_VertexDesc_t.m_nFirstVertex;
  local_214.m_IndexBuilder.super_IndexDesc_t.m_pIndices =
       local_214.super_MeshDesc_t.super_IndexDesc_t.m_pIndices;
  local_214.m_IndexBuilder.super_IndexDesc_t.m_nIndexSize =
       local_214.super_MeshDesc_t.super_IndexDesc_t.m_nIndexSize;
  local_214.m_IndexBuilder.m_nCurrentIndex = 0;
  local_214.m_IndexBuilder.m_nMaxIndexCount = iVar3 * 0x18;
  local_214.m_VertexBuilder.m_pVertexBuffer = (IVertexBuffer *)pIVar4;
  _memcpy(&local_214.m_VertexBuilder,&local_214,0xa4);
  local_214.m_VertexBuilder.super_VertexDesc_t.m_NumBoneWeights =
       ~-(uint)(local_214.m_VertexBuilder.super_VertexDesc_t.m_NumBoneWeights == 0) & 2;
  local_214.m_VertexBuilder.m_nVertexCount = 0;
  local_214.m_VertexBuilder.m_bModify = false;
  if (local_214.m_VertexBuilder.m_nBufferOffset == 0xffffffff) {
    local_214.m_VertexBuilder.m_nTotalVertexCount = 0;
    local_214.m_VertexBuilder.m_nBufferOffset =
         local_214.super_MeshDesc_t.super_VertexDesc_t.m_nOffset;
    local_214.m_VertexBuilder.m_nBufferFirstVertex =
         local_214.super_MeshDesc_t.super_VertexDesc_t.m_nFirstVertex;
  }
  local_214.m_IndexBuilder.m_nCurrentIndex = 0;
  local_214.m_VertexBuilder.m_nCurrentVertex = 0;
  local_214.m_VertexBuilder.m_pCurrPosition =
       local_214.m_VertexBuilder.super_VertexDesc_t.m_pPosition;
  local_214.m_VertexBuilder.m_pCurrNormal = local_214.m_VertexBuilder.super_VertexDesc_t.m_pNormal;
                    /* Unresolved local var: int i@[???] */
  local_214.m_VertexBuilder.m_pCurrTexCoord[0] =
       local_214.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[0];
  local_214.m_VertexBuilder.m_pCurrTexCoord[1] =
       local_214.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[1];
  local_214.m_VertexBuilder.m_pCurrTexCoord[2] =
       local_214.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[2];
  local_214.m_VertexBuilder.m_pCurrTexCoord[3] =
       local_214.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[3];
  local_214.m_VertexBuilder.m_pCurrTexCoord[4] =
       local_214.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[4];
  local_214.m_VertexBuilder.m_pCurrTexCoord[5] =
       local_214.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[5];
  local_214.m_VertexBuilder.m_pCurrTexCoord[6] =
       local_214.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[6];
  local_214.m_VertexBuilder.m_pCurrTexCoord[7] =
       local_214.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[7];
  local_214.m_VertexBuilder.m_pCurrColor = local_214.m_VertexBuilder.super_VertexDesc_t.m_pColor;
  local_2c.x = vecStart->x;
  local_2c.y = vecStart->y;
  local_2c.z = vecStart->z;
  local_214.m_VertexBuilder.m_nMaxVertexCount = iVar3 << 4;
  DrawColumnSegment(this,&local_214,&local_2c,vDir,flLength,vecXAxis,vecYAxis,flRadius,flAlpha,
                    flTextureOffset);
  if (local_214.m_bGenerateIndices != false) {
    if (local_214.m_Type - MATERIAL_LINE_STRIP < 7) {
                    /* WARNING: Could not recover jumptable at 0x0053f41f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (*(code *)(*(int *)(unaff_EBX + 0x4e3 + (local_214.m_Type - MATERIAL_LINE_STRIP) * 4) +
                unaff_EBX))();
      return;
    }
    if (local_214.m_IndexBuilder.super_IndexDesc_t.m_nIndexSize != 0) {
      iVar3 = local_214.m_IndexBuilder.m_nMaxIndexCount - local_214.m_IndexBuilder.m_nCurrentIndex;
      if (local_214.m_VertexBuilder.m_nVertexCount <=
          local_214.m_IndexBuilder.m_nMaxIndexCount - local_214.m_IndexBuilder.m_nCurrentIndex) {
        iVar3 = local_214.m_VertexBuilder.m_nVertexCount;
      }
      if (iVar3 != 0) {
        if (local_214.m_Type < MATERIAL_HETEROGENOUS) {
                    /* WARNING: Could not recover jumptable at 0x0053f50e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
          (*(code *)(*(int *)(unaff_EBX + 0x5cf + local_214.m_Type * 4) + unaff_EBX))();
          return;
        }
        GenerateSequentialIndexBuffer
                  (local_214.m_IndexBuilder.super_IndexDesc_t.m_pIndices +
                   local_214.m_IndexBuilder.m_nCurrentIndex,iVar3,
                   local_214.m_IndexBuilder.m_nIndexOffset);
        local_214.m_IndexBuilder.m_nCurrentIndex =
             iVar3 * local_214.m_IndexBuilder.super_IndexDesc_t.m_nIndexSize +
             local_214.m_IndexBuilder.m_nCurrentIndex;
        if (local_214.m_IndexBuilder.m_nIndexCount < local_214.m_IndexBuilder.m_nCurrentIndex) {
          local_214.m_IndexBuilder.m_nIndexCount = local_214.m_IndexBuilder.m_nCurrentIndex;
        }
      }
    }
  }
  (*((local_214.m_pMesh)->super_IVertexBuffer)._vptr_IVertexBuffer[0x14])
            (local_214.m_pMesh,local_214.m_VertexBuilder.m_nVertexCount,
             local_214.m_IndexBuilder.m_nIndexCount,&local_214);
  local_214.m_IndexBuilder.m_pIndexBuffer = (IIndexBuffer *)0x0;
  local_214.m_IndexBuilder.m_nMaxIndexCount = 0;
  local_214.m_VertexBuilder.m_nMaxVertexCount = 0;
  local_214.m_VertexBuilder.m_pVertexBuffer = (IVertexBuffer *)0x0;
  local_214.m_VertexBuilder.super_VertexDesc_t.m_CompressionType = VERTEX_COMPRESSION_INVALID;
  local_214.m_pMesh = (IMesh *)0x0;
  (*(pIVar4->super_IVertexBuffer)._vptr_IVertexBuffer[0xb])(pIVar4,0xffffffff,0);
  if ((local_214.m_VertexBuilder.m_pVertexBuffer != (IVertexBuffer *)0x0) &&
     (iVar3 = (*(local_214.m_VertexBuilder.m_pVertexBuffer)->_vptr_IVertexBuffer[2])
                        (local_214.m_VertexBuilder.m_pVertexBuffer), (char)iVar3 != '\0')) {
    (*(local_214.m_VertexBuilder.m_pVertexBuffer)->_vptr_IVertexBuffer[4])
              (local_214.m_VertexBuilder.m_pVertexBuffer);
  }
  if ((local_214.m_IndexBuilder.m_pIndexBuffer != (IIndexBuffer *)0x0) &&
     (iVar3 = (*(local_214.m_IndexBuilder.m_pIndexBuffer)->_vptr_IIndexBuffer[2])
                        (local_214.m_IndexBuilder.m_pIndexBuffer), (char)iVar3 != '\0')) {
    (*(local_214.m_IndexBuilder.m_pIndexBuffer)->_vptr_IIndexBuffer[4])
              (local_214.m_IndexBuilder.m_pIndexBuffer);
  }
  if (local_20 != (int *)0x0) {
    (**(code **)(*local_20 + 0xc))(local_20);
  }
  if (local_20 != (int *)0x0) {
    (**(code **)(*local_20 + 4))(local_20);
  }
  return;
}


/* C_Trigger_TractorBeam::ShouldDraw at 0053ce80 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: bool ShouldDraw(C_Trigger_TractorBeam * this) */

bool __thiscall C_Trigger_TractorBeam::ShouldDraw(C_Trigger_TractorBeam *this)

{
  return true;
}


/* C_Trigger_TractorBeam::DrawModel at 0053f670 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: int DrawModel(C_Trigger_TractorBeam * this, int flags,
   RenderableInstance_t * instance) */

int __thiscall
C_Trigger_TractorBeam::DrawModel
          (C_Trigger_TractorBeam *this,int flags,RenderableInstance_t *instance)

{
  float fVar1;
  Vector *position;
  int unaff_EBX;
  float fVar2;
  float fVar3;
  float fVar4;
  float flLength;
  matrix3x4_t local_88;
  Vector local_58;
  Vector local_4c;
  QAngle local_40;
  float local_34;
  float local_30;
  float local_2c;
  Vector local_28 [2];
  
                    /* Unresolved local var: Vector vDir@[???]
                       Unresolved local var: float flStartDelta@[???]
                       Unresolved local var: QAngle angles@[???]
                       Unresolved local var: float flRadius@[???]
                       Unresolved local var: matrix3x4_t xform@[???]
                       Unresolved local var: Vector xAxis@[???]
                       Unresolved local var: Vector yAxis@[???] */
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: Vector res@[???] */
  local_34 = (this->m_vEnd).x - (this->m_vStart).x;
  local_30 = (this->m_vEnd).y - (this->m_vStart).y;
  local_2c = (this->m_vEnd).z - (this->m_vStart).z;
  local_28[0].x = local_34;
  local_28[0].y = local_30;
  local_28[0].z = local_2c;
  flLength = VectorNormalize(local_28);
  fVar3 = *(float *)(**(int **)(unaff_EBX + 0x62471b) + 0xc);
  fVar1 = this->m_flStartTime;
  fVar4 = fVar3 - fVar1;
  if ((fVar4 < *(float *)(unaff_EBX + 0x4a2b8f)) &&
     (!NAN(fVar4) && !NAN(*(float *)(unaff_EBX + 0x4a2b8f)))) {
    fVar2 = fVar1 + *(float *)(unaff_EBX + 0x4a2b8f);
    if ((fVar1 != fVar2) || (NAN(fVar1) || NAN(fVar2))) {
      fVar4 = fVar4 / (fVar2 - fVar1);
      fVar3 = *(float *)(CUtlRBTree<CUtlMap<const_char*,_IPanelFactory*,_short_unsigned_int>::Node_t,short_unsigned_int,CUtlMap<const_char*,_IPanelFactory*,_short_unsigned_int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const_char*,_IPanelFactory*,_short_unsigned_int>::Node_t,_short_unsigned_int>,_short_unsigned_int>_>
                         ::RotateRight + unaff_EBX + 3);
      if (fVar4 <= fVar3) {
        fVar3 = 0.0;
        if (0.0 <= fVar4) {
          fVar3 = fVar4;
        }
        flLength = (fVar3 * fVar3 * *(float *)(unaff_EBX + 0x4a2ecf) +
                    fVar3 * fVar3 * fVar3 * *(float *)(unaff_EBX + 0x4a2f6b) + 0.0) * flLength;
        goto LAB_0053f751;
      }
    }
    else {
      if (fVar3 < fVar2) {
        flLength = flLength * 0.0;
        goto LAB_0053f751;
      }
      fVar3 = *(float *)(CUtlRBTree<CUtlMap<const_char*,_IPanelFactory*,_short_unsigned_int>::Node_t,short_unsigned_int,CUtlMap<const_char*,_IPanelFactory*,_short_unsigned_int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const_char*,_IPanelFactory*,_short_unsigned_int>::Node_t,_short_unsigned_int>,_short_unsigned_int>_>
                         ::RotateRight + unaff_EBX + 3);
    }
    flLength = fVar3 * flLength;
  }
LAB_0053f751:
  VectorAngles(local_28,&local_40);
  position = &this->m_vStart;
  AngleMatrix(&local_40,position,&local_88);
  MatrixGetColumn(&local_88,2,&local_4c);
  MatrixGetColumn(&local_88,1,&local_58);
  DrawColumn(this,this->m_pMaterial1,position,local_28,flLength,&local_4c,&local_58,58.0,1.0,
             this->m_bFromPortal,this->m_bToPortal,0.0);
  DrawColumn(this,this->m_pMaterial2,position,local_28,flLength,&local_4c,&local_58,56.0,0.75,
             this->m_bFromPortal,this->m_bToPortal,0.0);
  DrawColumn(this,this->m_pMaterial3,position,local_28,flLength,&local_4c,&local_58,54.0,0.5,
             this->m_bFromPortal,this->m_bToPortal,0.0);
  return 1;
}


/* C_Trigger_TractorBeam::GetToolRecordingState at 0053dd60 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void GetToolRecordingState(C_Trigger_TractorBeam * this, KeyValues *
   msg) */

void __thiscall
C_Trigger_TractorBeam::GetToolRecordingState(C_Trigger_TractorBeam *this,KeyValues *msg)

{
  float fVar1;
  float fVar2;
  KeyValues *this_00;
  char *keyName;
  int unaff_EBX;
  
                    /* Unresolved local var: KeyValues * pKV@[???]
                       Unresolved local var: Vector vStart@[???]
                       Unresolved local var: Vector vEnd@[???] */
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::GetToolRecordingState((C_BaseEntity *)this,msg);
  this_00 = CIFM_EntityKeyValuesHandler_AutoRegister::FindOrCreateNonConformantKeyValues(msg);
  keyName = CIFM_EntityKeyValuesHandler_AutoRegister::GetHandlerIDKeyString();
  KeyValues::SetString(this_00,keyName,(char *)(unaff_EBX + 0x47da54));
  KeyValues::SetInt(this_00,(char *)(unaff_EBX + 0x44c9c2),
                    (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.index);
  KeyValues::SetFloat(this_00,(char *)(CInput::ControllerModeActive + unaff_EBX + 5),
                      this->m_flStartTime);
  KeyValues::SetInt(this_00,(char *)(unaff_EBX + 0x47d89f),(uint)this->m_bReversed);
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x47d8a8),this->m_linearForce);
  fVar1 = (this->m_vStart).y;
  fVar2 = (this->m_vStart).z;
  KeyValues::SetFloat(this_00,&UNK_0047d8ae + unaff_EBX,(this->m_vStart).x);
  KeyValues::SetFloat(this_00,&UNK_0047d229 + unaff_EBX,fVar1);
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x47d22e),fVar2);
  fVar1 = (this->m_vEnd).y;
  fVar2 = (this->m_vEnd).z;
  KeyValues::SetFloat(this_00,(char *)(CInput::JoyStickThirdPersonPlatformer + unaff_EBX + 5),
                      (this->m_vEnd).x);
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x47d8ba),fVar1);
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x47d8bf),fVar2);
  return;
}


/* C_Trigger_TractorBeam::RestoreToToolRecordedState at 0053d200 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void RestoreToToolRecordedState(C_Trigger_TractorBeam * this, KeyValues
   * pKV) */

void __thiscall
C_Trigger_TractorBeam::RestoreToToolRecordedState(C_Trigger_TractorBeam *this,KeyValues *pKV)

{
  undefined4 *puVar1;
  int iVar2;
  IMaterial *pIVar3;
  int unaff_EBX;
  float fVar4;
  Vector local_34;
  Vector local_28 [2];
  
  ___i686_get_pc_thunk_bx();
  fVar4 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x47e3f5),0.0);
  this->m_flStartTime = fVar4;
  iVar2 = KeyValues::GetInt(pKV,(char *)(unaff_EBX + 0x47e3ff),0);
  this->m_bReversed = iVar2 == 1;
  fVar4 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x47e408),0.0);
  this->m_linearForce = fVar4;
  fVar4 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x47e40e),0.0);
  (this->m_vStart).x = fVar4;
  fVar4 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x47dd89),0.0);
  (this->m_vStart).y = fVar4;
  fVar4 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x47dd8e),0.0);
  (this->m_vStart).z = fVar4;
  fVar4 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x47e415),0.0);
  (this->m_vEnd).x = fVar4;
  fVar4 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x47e41a),0.0);
  (this->m_vEnd).y = fVar4;
  fVar4 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x47e41f),0.0);
  (this->m_vEnd).z = fVar4;
  local_34.x = 16384.0;
  local_34.y = 16384.0;
  local_34.z = 16384.0;
  local_28[0].x = -16384.0;
  local_28[0].y = -16384.0;
  local_28[0].z = -16384.0;
  C_BaseEntity::SetSize((C_BaseEntity *)this,local_28,&local_34);
  puVar1 = *(undefined4 **)(unaff_EBX + 0x626cfe);
  pIVar3 = (IMaterial *)
           (**(code **)(*(int *)*puVar1 + 0x130))((int *)*puVar1,unaff_EBX + 0x47e3a3,0,0,0);
  this->m_pMaterial1 = pIVar3;
  pIVar3 = (IMaterial *)
           (**(code **)(*(int *)*puVar1 + 0x130))((int *)*puVar1,unaff_EBX + 0x47e3b8,0,0,0);
  this->m_pMaterial2 = pIVar3;
  pIVar3 = (IMaterial *)
           (**(code **)(*(int *)*puVar1 + 0x130))((int *)*puVar1,unaff_EBX + 0x47e3ce,0,0,0);
  this->m_pMaterial3 = pIVar3;
  return;
}


/* __tcf_3 at 0097c3a0 */

void __tcf_3(void *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  *(int *)(unaff_EBX + 0x356dac) = *(int *)(unaff_EBX + 0x1e7ad0) + 8;
  *(undefined4 *)(unaff_EBX + 0x356dc4) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x356dc0)) {
    if (*(int *)(unaff_EBX + 0x356db8) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1e79bc) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1e79bc),*(int *)(unaff_EBX + 0x356db8));
      *(undefined4 *)(unaff_EBX + 0x356db8) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x356dbc) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x356db8);
  *(int *)(unaff_EBX + 0x356dc8) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x356dc0)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1e79bc) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1e79bc),iVar1);
      *(undefined4 *)(unaff_EBX + 0x356db8) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x356dbc) = 0;
  }
  return;
}


/* C_Trigger_TractorBeam_NonConformantDataHandler::DestroyInstance at 00540720 */

/* DWARF original prototype: void DestroyInstance(C_Trigger_TractorBeam_NonConformantDataHandler *
   this, void * pEntity) */

void __thiscall
C_Trigger_TractorBeam_NonConformantDataHandler::DestroyInstance
          (C_Trigger_TractorBeam_NonConformantDataHandler *this,void *pEntity)

{
  code *UNRECOVERED_JUMPTABLE;
  int unaff_EBX;
  
                    /* Unresolved local var: C_Trigger_TractorBeam * pCastEntity@[???] */
  ___i686_get_pc_thunk_bx();
  UNRECOVERED_JUMPTABLE = *(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x623703) + 0x44);
  if (pEntity != (void *)0x0) {
    (*UNRECOVERED_JUMPTABLE)((int *)**(undefined4 **)(unaff_EBX + 0x623703),(int)pEntity + 4);
                    /* WARNING: Could not recover jumptable at 0x00540762. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (**(code **)(*(int *)pEntity + 4))();
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x00540783. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}


/* C_Trigger_TractorBeam_NonConformantDataHandler::HandleInstance at 00540990 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void HandleInstance(C_Trigger_TractorBeam_NonConformantDataHandler *
   this, void * pEntity, KeyValues * pKeyValues) */

void __thiscall
C_Trigger_TractorBeam_NonConformantDataHandler::HandleInstance
          (C_Trigger_TractorBeam_NonConformantDataHandler *this,void *pEntity,KeyValues *pKeyValues)

{
  short *psVar1;
  int unaff_EBX;
  undefined4 *puVar2;
  
                    /* Unresolved local var: C_Trigger_TractorBeam * pCastEntity@[???] */
  ___i686_get_pc_thunk_bx();
  C_Trigger_TractorBeam::RestoreToToolRecordedState(pEntity,pKeyValues);
  psVar1 = (short *)(**(code **)(*(int *)pEntity + 0x360))(pEntity);
  if (*psVar1 == -1) {
    puVar2 = *(undefined4 **)(unaff_EBX + 0x623490);
    (**(code **)(*(int *)*puVar2 + 0x40))((int *)*puVar2,(int)pEntity + 4,0,1,0xffffffff);
  }
  else {
    puVar2 = *(undefined4 **)(unaff_EBX + 0x623490);
  }
                    /* WARNING: Could not recover jumptable at 0x005409e9. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(int *)*puVar2 + 0x4c))();
  return;
}


/* C_Trigger_TractorBeam::OnNewParticleEffect at 0053d190 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void OnNewParticleEffect(C_Trigger_TractorBeam * this, char *
   pszParticleName, CNewParticleEffect * pNewParticleEffect) */

void __thiscall
C_Trigger_TractorBeam::OnNewParticleEffect
          (C_Trigger_TractorBeam *this,char *pszParticleName,CNewParticleEffect *pNewParticleEffect)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  iVar1 = _V_stricmp(pszParticleName,(char *)(unaff_EBX + 0x47e44e));
  if (iVar1 != 0) {
    return;
  }
  CNewParticleEffect::SetControlPoint(pNewParticleEffect,2,&this->m_vEnd);
  return;
}


/* CCTractorBeamProxyFactory at 0053d070 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

IMaterialProxy * CCTractorBeamProxyFactory(void)

{
  CResultProxy *this;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  this = operator_new(0xc);
  CResultProxy::CResultProxy(this);
  (this->super_IMaterialProxy)._vptr_IMaterialProxy = (_func_int_varargs **)(unaff_EBX + 0x6924e4);
  return &this->super_IMaterialProxy;
}


/* CTractorBeamProxy::~CTractorBeamProxy at 00540930 */

/* DWARF original prototype: void ~CTractorBeamProxy(CTractorBeamProxy * this, int __in_chrg) */

void __thiscall CTractorBeamProxy::~CTractorBeamProxy(CTractorBeamProxy *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CResultProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(unaff_EBX + 0x68ec27);
  CResultProxy::~CResultProxy(&this->super_CResultProxy,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CTractorBeamProxy::~CTractorBeamProxy at 00540970 */

/* DWARF original prototype: void ~CTractorBeamProxy(CTractorBeamProxy * this, int __in_chrg) */

void __thiscall CTractorBeamProxy::~CTractorBeamProxy(CTractorBeamProxy *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CResultProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(extraout_ECX + 0x68ebf0);
  CResultProxy::~CResultProxy(&this->super_CResultProxy,__in_chrg);
  return;
}


/* CTractorBeamProxy::Init at 0053d160 */

/* DWARF original prototype: bool Init(CTractorBeamProxy * this, IMaterial * pMaterial, KeyValues *
   pKeyValues) */

bool __thiscall
CTractorBeamProxy::Init(CTractorBeamProxy *this,IMaterial *pMaterial,KeyValues *pKeyValues)

{
  bool bVar1;
  
  bVar1 = CResultProxy::Init(&this->super_CResultProxy,pMaterial,pKeyValues);
  return bVar1;
}


/* CTractorBeamProxy::OnBind at 0053d0d0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void OnBind(CTractorBeamProxy * this, void * pC_BaseEntity) */

void __thiscall CTractorBeamProxy::OnBind(CTractorBeamProxy *this,void *pC_BaseEntity)

{
  C_BaseEntity *pCVar1;
  int iVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: C_BaseEntity * pEntity@[???]
                       Unresolved local var: C_Trigger_TractorBeam * pBeam@[???] */
  ___i686_get_pc_thunk_bx();
  if (pC_BaseEntity != (void *)0x0) {
    pCVar1 = CResultProxy::BindArgToEntity(&this->super_CResultProxy,pC_BaseEntity);
    if (pCVar1 != (C_BaseEntity *)0x0) {
      iVar2 = ___dynamic_cast(pCVar1,*(undefined4 *)(CEssayQuestion::ChainToMap + unaff_EBX + 7),
                              unaff_EBX + 0x692e9f,0);
      if (iVar2 != 0) {
        CResultProxy::SetFloatResult
                  (&this->super_CResultProxy,
                   *(float *)(unaff_EBX + 0x4a62cb) * *(float *)(iVar2 + 0xa84));
        return;
      }
    }
  }
  return;
}


/* _GLOBAL__I__ZN27ITriggerTractorBeamAutoList37m_ITriggerTractorBeamAutoListAutoListE at 0007a280 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN27ITriggerTractorBeamAutoList37m_ITriggerTractorBeamAutoListAutoListE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

