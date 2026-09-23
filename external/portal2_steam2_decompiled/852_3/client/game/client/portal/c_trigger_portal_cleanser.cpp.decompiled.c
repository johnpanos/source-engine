/* DWARF-guided pseudocode for game/client/portal/c_trigger_portal_cleanser.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* _C_TriggerPortalCleanser_CreateObject at 004f9080 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_TriggerPortalCleanser_CreateObject(int entnum,int serialNum)

{
  C_BaseEntity *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_TriggerPortalCleanser * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xa5c);
  C_BaseEntity::C_BaseEntity(this);
  (this->super_IClientEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x68c4f4);
  (this->super_IClientEntity).super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x68c88c);
  (this->super_IClientEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x68c944);
  (this->super_IClientEntity).super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x68c980);
  (**(code **)(unaff_EBX + 0x68c5b0))(this,entnum,serialNum);
  return &(this->super_IClientEntity).super_IClientNetworkable;
}


/* C_TriggerPortalCleanser::~C_TriggerPortalCleanser at 004f9440 */

/* WARNING: Struct "C_TriggerPortalCleanser": ignoring overlapping field "m_bDisabled" */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_TriggerPortalCleanser(C_TriggerPortalCleanser * this, int
   __in_chrg) */

void __thiscall
C_TriggerPortalCleanser::~C_TriggerPortalCleanser(C_TriggerPortalCleanser *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *(int *)&this->super_C_BaseTrigger = extraout_ECX + 0x68c140;
  *(int *)&(this->super_C_BaseTrigger).field_0x4 = extraout_ECX + 0x68c4d8;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = extraout_ECX + 0x68c590;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = extraout_ECX + 0x68c5cc;
  *(int *)&this->super_C_BaseTrigger = *(int *)(extraout_ECX + 0x637e1c) + 8;
  *(int *)&(this->super_C_BaseTrigger).field_0x4 = *(int *)(extraout_ECX + 0x637e1c) + 0x3a0;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = *(int *)(extraout_ECX + 0x637e1c) + 0x458;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = *(int *)(extraout_ECX + 0x637e1c) + 0x494;
  *(int *)&this->super_C_BaseTrigger = *(int *)(extraout_ECX + 0x637dc8) + 8;
  *(int *)&(this->super_C_BaseTrigger).field_0x4 = *(int *)(extraout_ECX + 0x637dc8) + 0x3a0;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = *(int *)(extraout_ECX + 0x637dc8) + 0x458;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = *(int *)(extraout_ECX + 0x637dc8) + 0x494;
  C_BaseEntity::~C_BaseEntity((C_BaseEntity *)this,__in_chrg);
  return;
}


/* C_TriggerPortalCleanser::~C_TriggerPortalCleanser at 004f9500 */

/* WARNING: Struct "C_TriggerPortalCleanser": ignoring overlapping field "m_bDisabled" */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_TriggerPortalCleanser(C_TriggerPortalCleanser * this, int
   __in_chrg) */

void __thiscall
C_TriggerPortalCleanser::~C_TriggerPortalCleanser(C_TriggerPortalCleanser *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  *(int *)&this->super_C_BaseTrigger = unaff_EBX + 0x68c07b;
  *(int *)&(this->super_C_BaseTrigger).field_0x4 = unaff_EBX + 0x68c413;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = unaff_EBX + 0x68c4cb;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = unaff_EBX + 0x68c507;
  *(int *)&this->super_C_BaseTrigger = *(int *)(unaff_EBX + 0x637d57) + 8;
  *(int *)&(this->super_C_BaseTrigger).field_0x4 = *(int *)(unaff_EBX + 0x637d57) + 0x3a0;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = *(int *)(unaff_EBX + 0x637d57) + 0x458;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = *(int *)(unaff_EBX + 0x637d57) + 0x494;
  *(int *)&this->super_C_BaseTrigger = *(int *)(unaff_EBX + 0x637d03) + 8;
  *(int *)&(this->super_C_BaseTrigger).field_0x4 = *(int *)(unaff_EBX + 0x637d03) + 0x3a0;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = *(int *)(unaff_EBX + 0x637d03) + 0x458;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = *(int *)(unaff_EBX + 0x637d03) + 0x494;
  C_BaseEntity::~C_BaseEntity((C_BaseEntity *)this,in_stack_ffffffe8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_TriggerPortalCleanser::Touch at 004f9170 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "C_TriggerPortalCleanser": ignoring overlapping field "m_bDisabled" */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void Touch(C_TriggerPortalCleanser * this, C_BaseEntity * pOther) */

void __thiscall C_TriggerPortalCleanser::Touch(C_TriggerPortalCleanser *this,C_BaseEntity *pOther)

{
  bool bVar1;
  bool bVar2;
  char cVar3;
  char *pcVar4;
  C_WeaponPortalgun *this_00;
  int *piVar5;
  CProp_Portal *pCVar6;
  int iVar7;
  int unaff_EBX;
  char *pcVar8;
  char *pcVar9;
  char *pcVar10;
  undefined1 uVar11;
  bool local_1e;
  
                    /* Unresolved local var: bool bIsPortalGun@[???] */
  ___i686_get_pc_thunk_bx();
  if ((this->super_C_BaseTrigger).field_0xa5a != '\0') {
    return;
  }
  uVar11 = pOther == (C_BaseEntity *)0x0;
  if ((bool)uVar11) {
LAB_004f9230:
    bVar1 = false;
  }
  else {
    pcVar4 = C_BaseEntity::GetClassname(pOther);
    iVar7 = 0x11;
    pcVar9 = (char *)(unaff_EBX + 0x491f40);
    do {
      pcVar8 = pcVar4;
      pcVar10 = pcVar9;
      if (iVar7 == 0) break;
      iVar7 = iVar7 + -1;
      pcVar10 = pcVar9 + 1;
      pcVar8 = pcVar4 + 1;
      uVar11 = *pcVar4 == *pcVar9;
      pcVar4 = pcVar8;
      pcVar9 = pcVar10;
    } while ((bool)uVar11);
    iVar7 = 0;
    if (!(bool)uVar11) {
      iVar7 = (uint)(byte)pcVar8[-1] - (uint)(byte)pcVar10[-1];
    }
    if (iVar7 != 0) goto LAB_004f9230;
    bVar1 = true;
  }
  iVar7 = (*(pOther->super_IClientEntity).super_IClientUnknown.super_IHandleEntity.
            _vptr_IHandleEntity[0xc0])(pOther);
  if (((char)iVar7 == '\0') && (!bVar1)) {
    return;
  }
  iVar7 = (*(pOther->super_IClientEntity).super_IClientUnknown.super_IHandleEntity.
            _vptr_IHandleEntity[0xc0])(pOther);
  if ((char)iVar7 != '\0') {
    piVar5 = (int *)___dynamic_cast(pOther,*(undefined4 *)(unaff_EBX + 0x637ba8),
                                    *(undefined4 *)(CEngineSprite::DrawFrameOfSize + unaff_EBX + 4),
                                    0);
    if (1 < *(int *)(**(int **)(unaff_EBX + 0x637bcc) + 0x14)) {
      if (piVar5 == (int *)0x0) goto LAB_004f920e;
      iVar7 = (**(code **)(*piVar5 + 0x47c))(piVar5,unaff_EBX + 0x491f40,0);
      if (iVar7 == 0) {
        return;
      }
    }
    if (piVar5 != (int *)0x0) {
      iVar7 = (**(code **)(*piVar5 + 0x47c))(piVar5,unaff_EBX + 0x491f40,0);
      if (iVar7 == 0) {
        return;
      }
      this_00 = (C_WeaponPortalgun *)
                ___dynamic_cast(iVar7,*(undefined4 *)(unaff_EBX + 0x637d4c),
                                *(undefined4 *)(unaff_EBX + 0x63839c),0);
      goto LAB_004f9267;
    }
  }
LAB_004f920e:
  if (!bVar1) {
    return;
  }
  this_00 = (C_WeaponPortalgun *)
            ___dynamic_cast(pOther,*(undefined4 *)(unaff_EBX + 0x637ba8),
                            *(undefined4 *)(unaff_EBX + 0x63839c),0);
LAB_004f9267:
  if (this_00 != (C_WeaponPortalgun *)0x0) {
    bVar1 = C_WeaponPortalgun::CanFirePortal1(this_00);
    if (((bVar1) &&
        (pCVar6 = C_WeaponPortalgun::GetAssociatedPortal(this_00,false),
        pCVar6 != (CProp_Portal *)0x0)) &&
       (cVar3 = (**(code **)((int)(pCVar6->super_C_Portal_Base2D).super_CPortalRenderable_FlatBasic.
                                  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.
                                  super_IClientUnknown.super_IHandleEntity + 0x498))(pCVar6),
       cVar3 != '\0')) {
      (**(code **)((int)(pCVar6->super_C_Portal_Base2D).super_CPortalRenderable_FlatBasic.
                        super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.
                        super_IClientUnknown.super_IHandleEntity + 0x4a0))(pCVar6,0);
      local_1e = true;
      bVar1 = true;
    }
    else {
      local_1e = false;
      bVar1 = false;
    }
    bVar2 = C_WeaponPortalgun::CanFirePortal2(this_00);
                    /* Unresolved local var: CProp_Portal * pPortal@[???] */
    if (((bVar2) &&
        (pCVar6 = C_WeaponPortalgun::GetAssociatedPortal(this_00,true),
        pCVar6 != (CProp_Portal *)0x0)) &&
       (cVar3 = (**(code **)((int)(pCVar6->super_C_Portal_Base2D).super_CPortalRenderable_FlatBasic.
                                  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.
                                  super_IClientUnknown.super_IHandleEntity + 0x498))(pCVar6),
       cVar3 != '\0')) {
      (**(code **)((int)(pCVar6->super_C_Portal_Base2D).super_CPortalRenderable_FlatBasic.
                        super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.
                        super_IClientUnknown.super_IHandleEntity + 0x4a0))(pCVar6,0);
      bVar1 = true;
    }
    else {
      if (!bVar1) {
        return;
      }
      bVar1 = false;
    }
    (**(code **)(*(int *)&(this_00->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.
                          super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity.
                          super_IClientEntity.super_IClientUnknown + 0x4a0))(this_00,0x1ed);
    C_WeaponPortalgun::DoCleanseEffect(this_00,local_1e,bVar1);
    if ((this_00->m_iLastFiredPortal).m_Value != 0) {
      (this_00->m_iLastFiredPortal).m_Value = 0;
    }
  }
  return;
}


/* C_TriggerPortalCleanser::UpdatePartitionListEntry at 004f9120 */

/* WARNING: Struct "C_TriggerPortalCleanser": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void UpdatePartitionListEntry(C_TriggerPortalCleanser * this) */

void __thiscall C_TriggerPortalCleanser::UpdatePartitionListEntry(C_TriggerPortalCleanser *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x637d38) + 0x14))
            ((int *)**(undefined4 **)(unaff_EBX + 0x637d38),0x8c,0x100,
             *(undefined2 *)&(this->super_C_BaseTrigger).field_0x43c);
  return;
}


/* ClientClassInit<DT_TriggerPortalCleanser::ignored> at 0006e090 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

int ClientClassInit<DT_TriggerPortalCleanser::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (_GLOBAL__I_option_duck_method
      [(int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX] == (code)0x0
     ) {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc45eb4);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x7d5].m_pPrev + unaff_EBX
                  ),(char *)(unaff_EBX + 0x8f1cbc),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x7d9].
                               m_SerialNumber + unaff_EBX),(char *)(unaff_EBX + 0x8f689c),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00ac31c4 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00ac2cfc + unaff_EBX));
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x7dd].m_pEntity +
                   unaff_EBX),(char *)(unaff_EBX + 0x922718),0xa5a,1);
      ___cxa_guard_release(unaff_EBX + 0xc45eb4);
    }
  }
  RecvTable::Construct
            ((RecvTable *)(s_SortBuffer + unaff_EBX + 0x1810),
             (RecvProp_conflict1 *)
             ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x7d9].m_SerialNumber +
             unaff_EBX),2,(char *)(unaff_EBX + 0x91ef3c));
  return 1;
}


/* __static_initialization_and_destruction_0 at 0006e1e0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  uint8 *puVar1;
  int *piVar2;
  int iVar3;
  int unaff_EBX;
  longlong lVar4;
  
  lVar4 = ___i686_get_pc_thunk_bx();
  if (lVar4 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x7bc].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x7bc].m_pNext + unaff_EBX)
       = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x7bc].m_pNext + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x7bc].m_pNext + unaff_EBX + 2) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x7bc].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x7bd].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x7bd].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x7bd].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x7bd].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x7be].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x7be].m_SerialNumber + unaff_EBX + 3) =
       0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x7be].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x7be].m_pNext + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x7bf].m_pEntity + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x7bf].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x7bf].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined **)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x7bf].m_pNext + unaff_EBX + 3) =
       &UNK_00ac3c17 + unaff_EBX;
  *(int *)(s_SortBuffer + (int)(&UNK_000016b3 + unaff_EBX)) = unaff_EBX + 0x91ee08;
  *(int *)(s_SortBuffer + unaff_EBX + 0x16ab) = unaff_EBX + 0x48ae8f;
  puVar1 = s_SortBuffer + unaff_EBX + 0x16af;
  puVar1[0] = '\0';
  puVar1[1] = '\0';
  puVar1[2] = '\0';
  puVar1[3] = '\0';
  *(RecvTable **)(s_SortBuffer + (int)(&UNK_000016b7 + unaff_EBX)) =
       (RecvTable *)(s_SortBuffer + unaff_EBX + 0x16c3);
  piVar2 = *(int **)(&DAT_00ac2ba7 + unaff_EBX);
  *(int *)(s_SortBuffer + (int)(&UNK_000016bb + unaff_EBX)) = *piVar2;
  *piVar2 = (int)(s_SortBuffer + unaff_EBX + 0x16ab);
  RecvTable::RecvTable((RecvTable *)(s_SortBuffer + unaff_EBX + 0x16c3));
  ___cxa_atexit(unaff_EBX + 0x8e268f,0,*(undefined4 *)(&DAT_00ac2b2f + unaff_EBX));
  iVar3 = ClientClassInit<DT_TriggerPortalCleanser::ignored>((ignored *)0x0);
  *(int *)(s_SortBuffer + unaff_EBX + 0x16d7) = iVar3;
  return;
}


/* C_TriggerPortalCleanser::YouForgotToImplementOrDeclareClientClass at 004f9050 */

/* WARNING: Struct "C_TriggerPortalCleanser": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_TriggerPortalCleanser *
   this) */

int __thiscall
C_TriggerPortalCleanser::YouForgotToImplementOrDeclareClientClass(C_TriggerPortalCleanser *this)

{
  return 0;
}


/* C_TriggerPortalCleanser::GetClientClass at 004f9070 */

/* WARNING: Struct "C_TriggerPortalCleanser": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: ClientClass * GetClientClass(C_TriggerPortalCleanser * this) */

ClientClass * __thiscall C_TriggerPortalCleanser::GetClientClass(C_TriggerPortalCleanser *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(&UNK_00854ea4 + extraout_ECX);
}


/* __tcf_0 at 00950880 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3fd6ac),in_stack_00000008);
  return;
}


/* _GLOBAL__I__ZN23C_TriggerPortalCleanser17m_pClassRecvTableE at 0006e300 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN23C_TriggerPortalCleanser17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

