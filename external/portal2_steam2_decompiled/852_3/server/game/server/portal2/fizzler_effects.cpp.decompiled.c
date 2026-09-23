/* DWARF-guided pseudocode for game/server/portal2/fizzler_effects.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000be3f0 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  undefined4 *puVar1;
  IEntityFactoryDictionary *pIVar2;
  datamap_t *pdVar3;
  int iVar4;
  int iVar5;
  int unaff_EBX;
  undefined4 *puVar6;
  undefined4 *puVar7;
  longlong lVar8;
  ConVar *in_stack_ffffffd4;
  SendTable *this;
  char *in_stack_ffffffd8;
  char *in_stack_ffffffdc;
  char *s2;
  int in_stack_ffffffe0;
  
  lVar8 = ___i686_get_pc_thunk_bx();
  if (lVar8 == 0xffff00000001) {
    *(undefined1 *)((int)&paintblob_tbeam_portal_vortex_circulation.m_pParent + unaff_EBX) = 0;
    *(undefined1 *)((int)&paintblob_tbeam_portal_vortex_circulation.m_pParent + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&paintblob_tbeam_portal_vortex_circulation.m_pParent + unaff_EBX + 2) = 0;
    *(undefined1 *)((int)&paintblob_tbeam_portal_vortex_circulation.m_pParent + unaff_EBX + 3) = 0;
    *(undefined4 *)((int)&paintblob_tbeam_portal_vortex_circulation.m_pszDefaultValue + unaff_EBX) =
         0;
    *(undefined4 *)((int)&paintblob_tbeam_portal_vortex_circulation.m_Value.m_pszString + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&paintblob_tbeam_portal_vortex_circulation.m_Value.m_StringLength + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&paintblob_tbeam_portal_vortex_circulation.m_Value.m_fValue + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&paintblob_tbeam_portal_vortex_circulation.m_Value.m_nValue + unaff_EBX) =
         0;
    *(undefined4 *)(&paintblob_tbeam_portal_vortex_circulation.m_bHasMin + unaff_EBX) = 0;
    *(undefined4 *)((int)&paintblob_tbeam_portal_vortex_circulation.m_fMinVal + unaff_EBX) = 0;
    *(undefined4 *)(&paintblob_tbeam_portal_vortex_circulation.m_bHasMax + unaff_EBX) = 0;
    *(undefined4 *)((int)&paintblob_tbeam_portal_vortex_circulation.m_fMaxVal + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)
     ((int)&paintblob_tbeam_portal_vortex_circulation.m_fnChangeCallbacks.m_Memory.m_pMemory +
     unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)
     ((int)&paintblob_tbeam_portal_vortex_circulation.m_fnChangeCallbacks.m_Memory.
            m_nAllocationCount + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)
     ((int)&paintblob_tbeam_portal_vortex_circulation.m_fnChangeCallbacks.m_Memory.m_nGrowSize +
     unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)
     ((int)&paintblob_tbeam_portal_vortex_circulation.m_fnChangeCallbacks.m_Size + unaff_EBX) = 0;
    *(undefined1 *)
     ((int)&paintblob_tbeam_portal_vortex_circulation.m_fnChangeCallbacks.m_pElements +
     unaff_EBX + 3) = 0;
    *(undefined1 *)
     ((int)&paintblob_tbeam_portal_vortex_circulation.m_fnChangeCallbacks.m_pElements + unaff_EBX) =
         0;
    *(undefined1 *)
     ((int)&paintblob_tbeam_portal_vortex_circulation.m_fnChangeCallbacks.m_pElements +
     unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&paintblob_tbeam_portal_vortex_circulation.m_fnChangeCallbacks.m_pElements +
     unaff_EBX + 2) = 0;
    *(undefined4 *)(&DAT_00cf8338 + unaff_EBX) = 0;
    (&DAT_00cf833f)[unaff_EBX] = 0;
    (&DAT_00cf833c)[unaff_EBX] = 0;
    (&DAT_00cf833d)[unaff_EBX] = 0;
    (&DAT_00cf833e)[unaff_EBX] = 0;
    *(undefined4 *)
     ((int)&paintblob_tbeam_max_vortex_radius.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX)
         = 0;
    *(undefined1 *)
     ((int)&paintblob_tbeam_max_vortex_radius.super_ConCommandBase.m_pNext + unaff_EBX + 3) = 1;
    *(undefined1 *)
     ((int)&paintblob_tbeam_max_vortex_radius.super_ConCommandBase.m_pNext + unaff_EBX) = 0;
    *(undefined1 *)
     ((int)&paintblob_tbeam_max_vortex_radius.super_ConCommandBase.m_pNext + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&paintblob_tbeam_max_vortex_radius.super_ConCommandBase.m_pNext + unaff_EBX + 2) = 0;
    *(undefined4 *)
     (&paintblob_tbeam_max_vortex_radius.super_ConCommandBase.m_bRegistered + unaff_EBX) = 1;
    *(undefined1 *)
     ((int)&paintblob_tbeam_max_vortex_radius.super_ConCommandBase.m_pszName + unaff_EBX + 3) = 0;
    *(undefined1 *)
     ((int)&paintblob_tbeam_max_vortex_radius.super_ConCommandBase.m_pszName + unaff_EBX) = 0;
    *(undefined1 *)
     ((int)&paintblob_tbeam_max_vortex_radius.super_ConCommandBase.m_pszName + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&paintblob_tbeam_max_vortex_radius.super_ConCommandBase.m_pszName + unaff_EBX + 2) = 0;
    *(undefined4 *)
     ((int)&paintblob_tbeam_max_vortex_radius.super_ConCommandBase.m_pszHelpString + unaff_EBX) = 2;
    *(undefined1 *)
     ((int)&paintblob_tbeam_max_vortex_radius.super_ConCommandBase.m_nFlags + unaff_EBX + 3) = 0;
    *(undefined1 *)
     ((int)&paintblob_tbeam_max_vortex_radius.super_ConCommandBase.m_nFlags + unaff_EBX) = 0;
    *(undefined1 *)
     ((int)&paintblob_tbeam_max_vortex_radius.super_ConCommandBase.m_nFlags + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&paintblob_tbeam_max_vortex_radius.super_ConCommandBase.m_nFlags + unaff_EBX + 2) = 0;
    *(undefined **)((int)&paintblob_tbeam_max_vortex_radius.super_IConVar._vptr_IConVar + unaff_EBX)
         = &UNK_00af2f24 + unaff_EBX;
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<CPointGamestatsCounter>::dataDesc[0].flatOffset + unaff_EBX + 8
                   ),(char *)(unaff_EBX + 0x950581),(char *)(unaff_EBX + 0x95057a),0x4002,
                   in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0);
    ___cxa_atexit(unaff_EBX + 0x8dacac,0,*(undefined4 *)(&DAT_00aed78c + unaff_EBX));
    *(undefined **)((int)&paintblob_tbeam_max_vortex_radius.m_pParent + unaff_EBX) =
         &UNK_00b719ac + unaff_EBX;
    pIVar2 = EntityFactoryDictionary();
    (**pIVar2->_vptr_IEntityFactoryDictionary)(pIVar2,unaff_EBX + 0xcf835c,unaff_EBX + 0x95059f);
    pdVar3 = DataMapInit<FizzlerEffects>((FizzlerEffects *)0x0);
    *(datamap_t **)
     ((int)DataMapInit<CPointGamestatsCounter>::dataDesc[2].flatOffset + unaff_EBX + -0x20) = pdVar3
    ;
    iVar5 = unaff_EBX + 0xcf8360;
    s2 = (char *)(unaff_EBX + 0x95056b);
    *(char **)((int)&paintblob_tbeam_max_vortex_radius.m_pszDefaultValue + unaff_EBX) = s2;
    this = (SendTable *)
           ((int)DataMapInit<CPointGamestatsCounter>::dataDesc[2].flatOffset + unaff_EBX + -0x1c);
    *(SendTable **)((int)&paintblob_tbeam_max_vortex_radius.m_Value.m_pszString + unaff_EBX) = this;
    *(undefined4 *)((int)&paintblob_tbeam_max_vortex_radius.m_Value.m_nValue + unaff_EBX) = 0xffff;
    puVar7 = (undefined4 *)**(int **)(&DAT_00aed82c + unaff_EBX);
    if (puVar7 == (undefined4 *)0x0) {
      **(int **)(&DAT_00aed82c + unaff_EBX) = iVar5;
      *(undefined4 *)((int)&paintblob_tbeam_max_vortex_radius.m_Value.m_StringLength + unaff_EBX) =
           0;
    }
    else {
      puVar1 = (undefined4 *)puVar7[2];
      iVar4 = _V_stricmp((char *)*puVar7,s2);
      if (iVar4 < 1) {
        while ((puVar6 = puVar1, puVar6 != (undefined4 *)0x0 &&
               (iVar4 = _V_stricmp((char *)*puVar6,s2), iVar4 < 1))) {
          puVar1 = (undefined4 *)puVar6[2];
          puVar7 = puVar6;
        }
        *(undefined4 **)(iVar5 + 8) = puVar6;
        puVar7[2] = iVar5;
      }
      else {
        *(undefined4 *)(iVar5 + 8) = **(undefined4 **)(&DAT_00aed82c + unaff_EBX);
        **(int **)(&DAT_00aed82c + unaff_EBX) = iVar5;
      }
    }
    SendTable::SendTable(this);
    ___cxa_atexit(unaff_EBX + 0x8dac8c,0,*(undefined4 *)(&DAT_00aed78c + unaff_EBX));
    iVar5 = ServerClassInit<DT_FizzlerEffects::ignored>((ignored *)0x0);
    *(int *)((int)DataMapInit<CPointGamestatsCounter>::dataDesc[2].flatOffset + unaff_EBX + -8) =
         iVar5;
  }
  return;
}


/* __tcf_2 at 009990b0 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x395768),in_stack_00000008);
  return;
}


/* DataMapInit<FizzlerEffects> at 000be1e0 */

datamap_t * DataMapInit<FizzlerEffects>(FizzlerEffects *param_1)

{
  int iVar1;
  int iVar2;
  char *pcVar3;
  size_t sVar4;
  char *pcVar5;
  int iVar6;
  undefined4 *puVar7;
  int iVar8;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((*(char *)((int)&paint_blob_max_count.m_fnChangeCallbacks.m_pElements + unaff_EBX) == '\0') &&
     (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcf8634), iVar2 != 0)) {
    *(int *)((int)&paintblob_update_per_second.super_ConCommandBase.m_pNext + unaff_EBX) =
         unaff_EBX + 0x95077b;
    *(undefined4 *)((int)&paintblob_update_per_second.super_ConCommandBase.m_pszName + unaff_EBX) =
         0;
    *(undefined4 *)
     ((int)&paintblob_update_per_second.super_ConCommandBase.m_pszHelpString + unaff_EBX) = 0;
    *(undefined4 *)((int)&paintblob_update_per_second.super_ConCommandBase.m_nFlags + unaff_EBX) = 0
    ;
    *(undefined4 *)((int)&paintblob_update_per_second.super_IConVar._vptr_IConVar + unaff_EBX) = 0;
    *(undefined4 *)((int)&paintblob_update_per_second.m_pParent + unaff_EBX) = 0;
    *(undefined4 *)(&paintblob_update_per_second.super_ConCommandBase.m_bRegistered + unaff_EBX) =
         0xe;
    ___cxa_guard_release(unaff_EBX + 0xcf8634);
    ___cxa_atexit(unaff_EBX + 0x8daedc,0,*(undefined4 *)(&DAT_00aed99c + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_ModifyOrAppendDerivedCriteria_00c0b118 + unaff_EBX) =
       *(undefined4 *)(&DAT_00aedad8 + unaff_EBX);
  if (((&DAT_00cf863c)[unaff_EBX] == '\0') &&
     (iVar2 = ___cxa_guard_acquire(&DAT_00cf863c + unaff_EBX), iVar2 != 0)) {
    pcVar3 = operator_new___(*(int *)(&paintblob_update_per_second.super_ConCommandBase.
                                       m_bRegistered + unaff_EBX) + 0xc);
    _strcpy(pcVar3,*(char **)((int)&paintblob_update_per_second.super_ConCommandBase.m_pNext +
                             unaff_EBX));
    sVar4 = _strlen(pcVar3);
    pcVar5 = pcVar3 + sVar4;
    pcVar5[0] = 'F';
    pcVar5[1] = 'i';
    pcVar5[2] = 'z';
    pcVar5[3] = 'z';
    pcVar5[4] = 'l';
    pcVar5[5] = 'e';
    pcVar5[6] = 'T';
    pcVar5[7] = 'h';
    *(undefined **)(pcVar5 + 8) = &DAT_006b6e69;
    iVar1 = *(int *)((int)&paintblob_update_per_second.super_IConVar._vptr_IConVar + unaff_EBX);
    iVar2 = iVar1 + 1;
    iVar8 = *(int *)((int)&paintblob_update_per_second.super_ConCommandBase.m_pszHelpString +
                    unaff_EBX);
    if (iVar8 < iVar2) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)
                 ((int)&paintblob_update_per_second.super_ConCommandBase.m_pszName + unaff_EBX),
                 iVar2 - iVar8);
    }
    iVar6 = *(int *)((int)&paintblob_update_per_second.super_IConVar._vptr_IConVar + unaff_EBX) + 1;
    *(int *)((int)&paintblob_update_per_second.super_IConVar._vptr_IConVar + unaff_EBX) = iVar6;
    iVar8 = *(int *)((int)&paintblob_update_per_second.super_ConCommandBase.m_pszName + unaff_EBX);
    *(int *)((int)&paintblob_update_per_second.m_pParent + unaff_EBX) = iVar8;
    iVar6 = (iVar6 - iVar1) + -1;
    if (0 < iVar6) {
      _V_memmove((void *)(iVar8 + iVar2 * 4),(void *)(iVar8 + iVar1 * 4),iVar6 * 4);
      iVar8 = *(int *)((int)&paintblob_update_per_second.super_ConCommandBase.m_pszName + unaff_EBX)
      ;
    }
    puVar7 = (undefined4 *)(iVar8 + iVar1 * 4);
    if (puVar7 != (undefined4 *)0x0) {
      *puVar7 = pcVar3;
    }
    *(char **)((int)&PTR_GetVelocity_00c0b170 + unaff_EBX) = pcVar3;
    ___cxa_guard_release(&DAT_00cf863c + unaff_EBX);
  }
  *(undefined4 *)((int)&PTR_SetHealth_00c0b110 + unaff_EBX) = 1;
  *(int *)((int)&PTR_GetMaxHealth_00c0b10c + unaff_EBX) =
       (int)&PTR_GetSmoothedVelocity_00c0b16c + unaff_EBX;
  return (datamap_t *)((int)&PTR_GetMaxHealth_00c0b10c + unaff_EBX);
}


/* FizzlerEffects::GetDataDescMap at 006b6250 */

/* DWARF original prototype: datamap_t * GetDataDescMap(FizzlerEffects * this) */

datamap_t * __thiscall FizzlerEffects::GetDataDescMap(FizzlerEffects *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x6130a8);
}


/* FizzlerEffects::GetBaseMap at 006b6260 */

datamap_t * FizzlerEffects::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4f5a64);
}


/* __tcf_0 at 009990d0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41d76e)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41d762) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41d76e));
  }
  *(undefined4 *)(unaff_EBX + 0x41d76e) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41d76a)) {
    if (*(int *)(unaff_EBX + 0x41d762) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x212aba) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x212aba),*(int *)(unaff_EBX + 0x41d762));
      *(undefined4 *)(unaff_EBX + 0x41d762) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d766) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41d762);
  *(int *)(unaff_EBX + 0x41d772) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41d76a)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x212aba) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x212aba),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41d762) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d766) = 0;
  }
  return;
}


/* ServerClassInit<DT_FizzlerEffects::ignored> at 000be050 */

int ServerClassInit<DT_FizzlerEffects::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&speed_paint_color.m_fnChangeCallbacks.m_pElements + unaff_EBX) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf8714);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)(unaff_EBX + 0xcf871c),(char *)(unaff_EBX + 0x8f49c0),0,4,-1,
                  0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&bounce_paint_color.m_fnChangeCallbacks.m_Size + unaff_EBX),
                        (char *)(unaff_EBX + 0x8f6d30),0,
                        (SendTable *)**(undefined4 **)(&DAT_00aedc24 + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00aedbc8 + unaff_EBX),0x80);
      ___cxa_guard_release(unaff_EBX + 0xcf8714);
      ___cxa_atexit(unaff_EBX + 0x8dafec,0,*(undefined4 *)(&DAT_00aedb2c + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)
             ((int)DataMapInit<CBaseHLCombatWeapon>::dataDesc[4].flatOffset + unaff_EBX + 4),
             (SendProp_conflict *)((int)&bounce_paint_color.m_fnChangeCallbacks.m_Size + unaff_EBX),
             1,*(char **)(&DAT_00b71d24 + unaff_EBX));
  return 1;
}


/* FizzlerEffects::GetServerClass at 006b6270 */

/* DWARF original prototype: ServerClass * GetServerClass(FizzlerEffects * this) */

ServerClass * __thiscall FizzlerEffects::GetServerClass(FizzlerEffects *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x7004ec);
}


/* FizzlerEffects::YouForgotToImplementOrDeclareServerClass at 006b6280 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(FizzlerEffects * this) */

int __thiscall FizzlerEffects::YouForgotToImplementOrDeclareServerClass(FizzlerEffects *this)

{
  return 0;
}


/* __tcf_3 at 00999090 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x3957e4),in_stack_00000008);
  return;
}


/* __tcf_1 at 00999050 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d773))(unaff_EBX + 0x41d773);
                    /* WARNING: Could not recover jumptable at 0x00999086. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d71f))();
  return;
}


/* FizzlerEffects::Create at 006b6440 */

/* WARNING: Type propagation algorithm not settling */

FizzlerEffects * FizzlerEffects::Create(CBaseEntity *pAttachTo)

{
  int iSequence;
  CBaseEntity *this;
  CBaseAnimating *this_00;
  model_t *pmVar1;
  CStudioHdr *pStudioHdr;
  int unaff_EBX;
  float change_duration;
  
                    /* Unresolved local var: FizzlerEffects * pFizzlerEffects@[???] */
  ___i686_get_pc_thunk_bx();
  this = CreateEntityByName((char *)(unaff_EBX + 0x358555),-1,true);
  if (this == (CBaseEntity *)0x0) {
    return (FizzlerEffects *)0x0;
  }
  (*(this->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x26])
            (this,pAttachTo,0xffffffff);
  CBaseEntity::SetLocalOrigin(this,*(Vector **)(unaff_EBX + 0x4f5762));
  CBaseEntity::SetLocalAngles(this,*(QAngle **)(unaff_EBX + 0x4f57f6));
  (*(this->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x19])
            (this);
  (*(pAttachTo->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
    [0x39])(pAttachTo,unaff_EBX + 0x331b0d);
  this_00 = (CBaseAnimating *)
            (*(pAttachTo->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[0x36])(pAttachTo);
  if (this_00 == (CBaseAnimating *)0x0) {
    return (FizzlerEffects *)this;
  }
  CBaseEntity::AddFlag((CBaseEntity *)this_00,0x10000000);
  CBaseEntity::SetEffectEntity((CBaseEntity *)this_00,this);
  CBaseAnimating::ResetSequence(this_00,1);
  iSequence = (this_00->m_nSequence).m_Value;
  pStudioHdr = this_00->m_pStudioHdr;
  if (pStudioHdr == (CStudioHdr *)0x0) {
    pmVar1 = CBaseEntity::GetModel((CBaseEntity *)this_00);
    if (pmVar1 != (model_t *)0x0) {
      CBaseAnimating::LockStudioHdr(this_00);
    }
    pStudioHdr = this_00->m_pStudioHdr;
    if (pStudioHdr == (CStudioHdr *)0x0) goto LAB_006b6536;
  }
  if (pStudioHdr->m_pStudioHdr == (studiohdr_t *)0x0) {
    pStudioHdr = (CStudioHdr *)0x0;
  }
LAB_006b6536:
  change_duration = CBaseAnimating::SequenceDuration(this_00,pStudioHdr,iSequence);
  CBaseAnimating::SetModelScale
            (this_00,*(float *)(*(int *)(&DAT_006783ee + unaff_EBX) + 0x2c),change_duration,
             NONHIERARCHICAL_MODEL_SCALE);
  return (FizzlerEffects *)this;
}


/* FizzlerEffects::Spawn at 006b62f0 */

/* DWARF original prototype: void Spawn(FizzlerEffects * this) */

void __thiscall FizzlerEffects::Spawn(FizzlerEffects *this)

{
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::Spawn(&this->super_CBaseEntity);
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0x6f);
  CBaseEntity::ThinkSet(&this->super_CBaseEntity,func,0.0,(char *)0x0);
  CBaseEntity::SetNextThink
            (&this->super_CBaseEntity,*(float *)(**(int **)(unaff_EBX + 0x4f589f) + 0xc),(char *)0x0
            );
  return;
}


/* FizzlerEffects::AttachToEntity at 006b6290 */

/* DWARF original prototype: void AttachToEntity(FizzlerEffects * this, CBaseEntity * pAttachTo) */

void __thiscall FizzlerEffects::AttachToEntity(FizzlerEffects *this,CBaseEntity *pAttachTo)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x26])(this,pAttachTo,0xffffffff);
  CBaseEntity::SetLocalOrigin(&this->super_CBaseEntity,*(Vector **)(unaff_EBX + 0x4f590f));
  CBaseEntity::SetLocalAngles(&this->super_CBaseEntity,*(QAngle **)(unaff_EBX + 0x4f59a3));
  return;
}


/* FizzlerEffects::FizzleThink at 006b6370 */

/* DWARF original prototype: void FizzleThink(FizzlerEffects * this) */

void __thiscall FizzlerEffects::FizzleThink(FizzlerEffects *this)

{
  uint uVar1;
  int *piVar2;
  int iVar3;
  CBaseEntity *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CBaseAnimating * pDissolving@[???] */
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->super_CBaseEntity).m_hMoveParent.
          super_CNetworkVarBase<CBaseHandle,CBaseEntity::NetworkVar_m_hMoveParent>.m_Value.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f5820),
      *(uint *)(iVar3 + 8) == uVar1 >> 0x10)) &&
     (piVar2 = *(int **)(iVar3 + 4), piVar2 != (int *)0x0)) {
    this_00 = (CBaseEntity *)(**(code **)(*piVar2 + 0xd8))(piVar2);
    if (this_00 != (CBaseEntity *)0x0) {
      CBaseEntity::SetCollisionGroup(this_00,0x10);
      (*(this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
        [199])(this_00);
      if (*(char *)&this_00[1].m_Network.m_TimerEvent.m_flNextEventTime == '\0') {
        CBaseEntity::SetNextThink
                  (&this->super_CBaseEntity,*(float *)(**(int **)(unaff_EBX + 0x4f581c) + 0xc),
                   (char *)0x0);
        return;
      }
      UTIL_Remove(this_00);
    }
  }
  UTIL_Remove(&this->super_CBaseEntity);
  return;
}


/* _GLOBAL__I_fizzler_fx_model_target_scale at 000be6c0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_fizzler_fx_model_target_scale(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

