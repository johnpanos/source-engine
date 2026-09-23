/* DWARF-guided pseudocode for game/server/portal2/npc_hover_turret.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000c19c0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  IEntityFactoryDictionary *pIVar1;
  datamap_t *pdVar2;
  int iVar3;
  int iVar4;
  int unaff_EBX;
  undefined4 *puVar5;
  undefined4 *puVar6;
  undefined4 *puVar7;
  longlong lVar8;
  ConVar *in_stack_ffffffd4;
  char *in_stack_ffffffd8;
  char *s2;
  char *in_stack_ffffffdc;
  int in_stack_ffffffe0;
  SendTable *this;
  
  lVar8 = ___i686_get_pc_thunk_bx();
  if (lVar8 == 0xffff00000001) {
    *(undefined1 *)((int)&sk_npc_dmg_grenade.m_Value.m_fValue + unaff_EBX) = 0;
    *(undefined1 *)((int)&sk_npc_dmg_grenade.m_Value.m_fValue + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&sk_npc_dmg_grenade.m_Value.m_fValue + unaff_EBX + 2) = 0;
    *(undefined1 *)((int)&sk_npc_dmg_grenade.m_Value.m_fValue + unaff_EBX + 3) = 0;
    *(undefined4 *)((int)&sk_npc_dmg_grenade.m_Value.m_nValue + unaff_EBX) = 0;
    *(undefined4 *)(&sk_npc_dmg_grenade.m_bHasMin + unaff_EBX) = 0;
    *(undefined4 *)((int)&sk_npc_dmg_grenade.m_fMinVal + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&sk_npc_dmg_grenade.m_bHasMax + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&sk_npc_dmg_grenade.m_fMaxVal + unaff_EBX) = 0;
    *(undefined4 *)((int)&sk_npc_dmg_grenade.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) = 0
    ;
    *(undefined4 *)
     ((int)&sk_npc_dmg_grenade.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) = 0;
    *(undefined4 *)((int)&sk_npc_dmg_grenade.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) =
         0;
    *(undefined4 *)((int)&sk_npc_dmg_grenade.m_fnChangeCallbacks.m_Size + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&sk_npc_dmg_grenade.m_fnChangeCallbacks.m_pElements + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)(&DAT_00cf5e38 + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00cf5e3c + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&sk_max_grenade.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) = 0;
    *(undefined1 *)((int)&sk_max_grenade.super_ConCommandBase.m_pNext + unaff_EBX + 3) = 0;
    *(undefined1 *)((int)&sk_max_grenade.super_ConCommandBase.m_pNext + unaff_EBX) = 0;
    *(undefined1 *)((int)&sk_max_grenade.super_ConCommandBase.m_pNext + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&sk_max_grenade.super_ConCommandBase.m_pNext + unaff_EBX + 2) = 0;
    *(undefined4 *)(&sk_max_grenade.super_ConCommandBase.m_bRegistered + unaff_EBX) = 0;
    *(undefined1 *)((int)&sk_max_grenade.super_ConCommandBase.m_pszName + unaff_EBX + 3) = 0;
    *(undefined1 *)((int)&sk_max_grenade.super_ConCommandBase.m_pszName + unaff_EBX) = 0;
    *(undefined1 *)((int)&sk_max_grenade.super_ConCommandBase.m_pszName + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&sk_max_grenade.super_ConCommandBase.m_pszName + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&sk_max_grenade.super_ConCommandBase.m_pszHelpString + unaff_EBX) = 0;
    *(undefined1 *)((int)&sk_max_grenade.super_ConCommandBase.m_nFlags + unaff_EBX + 3) = 1;
    *(undefined1 *)((int)&sk_max_grenade.super_ConCommandBase.m_nFlags + unaff_EBX) = 0;
    *(undefined1 *)((int)&sk_max_grenade.super_ConCommandBase.m_nFlags + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&sk_max_grenade.super_ConCommandBase.m_nFlags + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&sk_max_grenade.super_IConVar._vptr_IConVar + unaff_EBX) = 1;
    *(undefined1 *)((int)&sk_max_grenade.m_pParent + unaff_EBX + 3) = 0;
    *(undefined1 *)((int)&sk_max_grenade.m_pParent + unaff_EBX) = 0;
    *(undefined1 *)((int)&sk_max_grenade.m_pParent + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&sk_max_grenade.m_pParent + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&sk_max_grenade.m_pszDefaultValue + unaff_EBX) = 2;
    *(undefined1 *)((int)&sk_max_grenade.m_Value.m_pszString + unaff_EBX + 3) = 0;
    *(undefined1 *)((int)&sk_max_grenade.m_Value.m_pszString + unaff_EBX) = 0;
    *(undefined1 *)((int)&sk_max_grenade.m_Value.m_pszString + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&sk_max_grenade.m_Value.m_pszString + unaff_EBX + 2) = 0;
    *(undefined **)((int)&sk_max_grenade.m_Value.m_StringLength + unaff_EBX) =
         &UNK_00aef954 + unaff_EBX;
    ConVar::ConVar((ConVar *)((int)&PTR_GotoEndOfLine_00c6d10c + unaff_EBX),
                   (char *)(unaff_EBX + 0x94da08),&UNK_0093dfac + unaff_EBX,0,in_stack_ffffffd4,
                   in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0);
    ___cxa_atexit(&UNK_008d893c + unaff_EBX,0,*(undefined4 *)(&DAT_00aea1bc + unaff_EBX));
    *(undefined **)((int)&sk_max_grenade.m_Value.m_fValue + unaff_EBX) = &UNK_00b71d84 + unaff_EBX;
    pIVar1 = EntityFactoryDictionary();
    (**pIVar1->_vptr_IEntityFactoryDictionary)(pIVar1,unaff_EBX + 0xcf5e6c,unaff_EBX + 0x94da1f);
    pdVar2 = DataMapInit<CHoverTurretTether>((CHoverTurretTether *)0x0);
    *(datamap_t **)((int)&PTR_SetEditable_00c6d174 + unaff_EBX) = pdVar2;
    ConVar::ConVar((ConVar *)
                   ((int)&sk_max_grenade.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX),
                   (char *)(unaff_EBX + 0x94da3a),(char *)(unaff_EBX + 0x8f5530),0,in_stack_ffffffd4
                   ,in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0);
    ___cxa_atexit(unaff_EBX + 0x8d891c,0,*(undefined4 *)(&DAT_00aea1bc + unaff_EBX));
    *(undefined **)
     ((int)&sk_plr_dmg_smg1_grenade.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) =
         &UNK_00b71da4 + unaff_EBX;
    pIVar1 = EntityFactoryDictionary();
    (**pIVar1->_vptr_IEntityFactoryDictionary)(pIVar1,unaff_EBX + 0xcf5ee4,unaff_EBX + 0x94a5cc);
    pdVar2 = DataMapInit<CNPC_HoverTurret>((CNPC_HoverTurret *)0x0);
    *(datamap_t **)((int)&PTR_IsEditable_00c6d178 + unaff_EBX) = pdVar2;
    iVar4 = unaff_EBX + 0xcf5ee8;
    s2 = &UNK_0094d9f7 + unaff_EBX;
    *(char **)((int)&sk_plr_dmg_smg1_grenade.m_fnChangeCallbacks.m_Memory.m_nAllocationCount +
              unaff_EBX) = s2;
    this = (SendTable *)((int)&PTR_MoveCursor_00c6d17c + unaff_EBX);
    *(SendTable **)
     ((int)&sk_plr_dmg_smg1_grenade.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) = this;
    *(undefined4 *)(&DAT_00cf5ef8 + unaff_EBX) = 0xffff;
    puVar7 = (undefined4 *)**(int **)(&DAT_00aea25c + unaff_EBX);
    if (puVar7 == (undefined4 *)0x0) {
      **(int **)(&DAT_00aea25c + unaff_EBX) = iVar4;
      *(undefined4 *)((int)&sk_plr_dmg_smg1_grenade.m_fnChangeCallbacks.m_Size + unaff_EBX) = 0;
    }
    else {
      puVar6 = (undefined4 *)puVar7[2];
      iVar3 = _V_stricmp((char *)*puVar7,s2);
      if (iVar3 < 1) {
        if (puVar6 != (undefined4 *)0x0) {
          do {
            puVar5 = puVar6;
            iVar3 = _V_stricmp((char *)*puVar5,s2);
            puVar6 = puVar5;
            if (0 < iVar3) goto LAB_000c1d60;
            puVar6 = (undefined4 *)puVar5[2];
            puVar7 = puVar5;
          } while ((undefined4 *)puVar5[2] != (undefined4 *)0x0);
          puVar6 = (undefined4 *)0x0;
        }
LAB_000c1d60:
        *(undefined4 **)(iVar4 + 8) = puVar6;
        puVar7[2] = iVar4;
      }
      else {
        *(undefined4 *)(iVar4 + 8) = **(undefined4 **)(&DAT_00aea25c + unaff_EBX);
        **(int **)(&DAT_00aea25c + unaff_EBX) = iVar4;
      }
    }
    SendTable::SendTable(this);
    ___cxa_atexit(unaff_EBX + 0x8d88fc,0,*(undefined4 *)(&DAT_00aea1bc + unaff_EBX));
    iVar4 = ServerClassInit<DT_NPC_HoverTurret::ignored>((ignored *)0x0);
    *(int *)((int)&PTR_SetCatchEnterKey_00c6d190 + unaff_EBX) = iVar4;
    CAI_LocalIdSpace::CAI_LocalIdSpace
              ((CAI_LocalIdSpace *)((int)&PTR_RecalculateLineBreaks_00c6d1b0 + unaff_EBX),false);
    CAI_LocalIdSpace::CAI_LocalIdSpace
              ((CAI_LocalIdSpace *)((int)&PTR_SetCharAt_00c6d1c8 + unaff_EBX),false);
    CAI_LocalIdSpace::CAI_LocalIdSpace
              ((CAI_LocalIdSpace *)((int)&PTR_GetYStart_00c6d1e0 + unaff_EBX),false);
    CAI_LocalIdSpace::CAI_LocalIdSpace
              ((CAI_LocalIdSpace *)((int)&PTR_GetValueAsInt_00c6d1f8 + unaff_EBX),false);
  }
  return;
}


/* __tcf_3 at 0099a310 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3947c8),in_stack_00000008);
  return;
}


/* GetHoverTurretTalkName at 006be870 */

char * GetHoverTurretTalkName(int iState)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(char **)(extraout_ECX + 0x60c088 + iState * 4);
}


/* CNPC_HoverTurret::TalkThink at 006bedb0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void TalkThink(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::TalkThink(CNPC_HoverTurret *this)

{
  uint uVar1;
  int iVar2;
  int unaff_EBX;
  int *piVar3;
  
  ___i686_get_pc_thunk_bx();
  uVar1 = ((CNetworkVarBase<int,CBaseEntity::NetworkVar_m_fFlags> *)
          ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                 super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                 super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x130))->m_Value
  ;
  if (((uVar1 & 0x10000000) != 0) || ((uVar1 & 0x8000000) != 0)) {
    this->m_iDesiredState = 7;
  }
  iVar2 = this->m_iDesiredState;
  if ((this->m_iLastState == iVar2) || (((iVar2 != 2 && (iVar2 != 7)) && (iVar2 != 5)))) {
    piVar3 = *(int **)(&DAT_004ecddc + unaff_EBX);
  }
  else {
    piVar3 = *(int **)(&DAT_004ecddc + unaff_EBX);
    this->m_fNextTalk = *(float *)(*piVar3 + 0xc) - *(float *)(unaff_EBX + 0x36b54c);
  }
  if (((this->m_fNextTalk <= *(float *)(*piVar3 + 0xc) &&
        *(float *)(*piVar3 + 0xc) != this->m_fNextTalk) && (this->m_iLastState != iVar2)) &&
     (this->m_iLastState = iVar2, (uint)this->m_iDesiredState < 8)) {
                    /* WARNING: Could not recover jumptable at 0x006bee69. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*(code *)(*(int *)(unaff_EBX + 0xa8 + iVar2 * 4) + unaff_EBX))();
    return;
  }
  return;
}


/* CHoverTurretTether::~CHoverTurretTether at 006c5bf0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CHoverTurretTether(CHoverTurretTether * this, int __in_chrg) */

void __thiscall CHoverTurretTether::~CHoverTurretTether(CHoverTurretTether *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x56d727);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CHoverTurretTether::~CHoverTurretTether at 006c5c30 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CHoverTurretTether(CHoverTurretTether * this, int __in_chrg) */

void __thiscall CHoverTurretTether::~CHoverTurretTether(CHoverTurretTether *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x56d6f0);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
  return;
}


/* DataMapInit<CHoverTurretTether> at 000c1660 */

datamap_t * DataMapInit<CHoverTurretTether>(CHoverTurretTether *param_1)

{
  int iVar1;
  int iVar2;
  ISaveRestoreOps *pIVar3;
  char *pcVar4;
  size_t sVar5;
  int iVar6;
  undefined4 *puVar7;
  int iVar8;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((*(char *)((int)&sk_npc_dmg_gunship.m_Value.m_fValue + unaff_EBX) == '\0') &&
     (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcf640c), iVar2 != 0)) {
    *(int *)(&sk_npc_dmg_gunship.m_bHasMax + unaff_EBX) = unaff_EBX + 0x94dd44;
    *(undefined4 *)((int)&sk_npc_dmg_gunship.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) = 0
    ;
    *(undefined4 *)
     ((int)&sk_npc_dmg_gunship.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) = 0;
    *(undefined4 *)((int)&sk_npc_dmg_gunship.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) =
         0;
    *(undefined4 *)((int)&sk_npc_dmg_gunship.m_fnChangeCallbacks.m_Size + unaff_EBX) = 0;
    *(undefined4 *)((int)&sk_npc_dmg_gunship.m_fnChangeCallbacks.m_pElements + unaff_EBX) = 0;
    *(undefined4 *)((int)&sk_npc_dmg_gunship.m_fMaxVal + unaff_EBX) = 0x12;
    ___cxa_guard_release(unaff_EBX + 0xcf640c);
    ___cxa_atexit(unaff_EBX + 0x8d8d9c,0,*(undefined4 *)(&DAT_00aea51c + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_MyCombatCharacterPointer_00c092b8 + unaff_EBX) =
       *(undefined4 *)(&DAT_00aea528 + unaff_EBX);
  if (((&sk_npc_dmg_gunship.m_bHasMin)[unaff_EBX] == false) &&
     (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcf6414), iVar2 != 0)) {
    pIVar3 = GetPhysObjSaveRestoreOps(PIID_IPHYSICSSPRING);
    *(ISaveRestoreOps **)((int)&PTR_NetworkStateChanged_m_vecVelocity_00c09b80 + unaff_EBX) = pIVar3
    ;
    pcVar4 = operator_new___(*(int *)((int)&sk_npc_dmg_gunship.m_fMaxVal + unaff_EBX) + 10);
    _strcpy(pcVar4,*(char **)(&sk_npc_dmg_gunship.m_bHasMax + unaff_EBX));
    sVar5 = _strlen(pcVar4);
    builtin_strncpy(pcVar4 + sVar5,"PullThink",10);
    iVar1 = *(int *)((int)&sk_npc_dmg_gunship.m_fnChangeCallbacks.m_Size + unaff_EBX);
    iVar2 = iVar1 + 1;
    iVar8 = *(int *)((int)&sk_npc_dmg_gunship.m_fnChangeCallbacks.m_Memory.m_nAllocationCount +
                    unaff_EBX);
    if (iVar8 < iVar2) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)
                 ((int)&sk_npc_dmg_gunship.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX),
                 iVar2 - iVar8);
    }
    iVar6 = *(int *)((int)&sk_npc_dmg_gunship.m_fnChangeCallbacks.m_Size + unaff_EBX) + 1;
    *(int *)((int)&sk_npc_dmg_gunship.m_fnChangeCallbacks.m_Size + unaff_EBX) = iVar6;
    iVar8 = *(int *)((int)&sk_npc_dmg_gunship.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX);
    *(int *)((int)&sk_npc_dmg_gunship.m_fnChangeCallbacks.m_pElements + unaff_EBX) = iVar8;
    iVar6 = (iVar6 - iVar1) + -1;
    if (0 < iVar6) {
      _V_memmove((void *)(iVar8 + iVar2 * 4),(void *)(iVar8 + iVar1 * 4),iVar6 * 4);
      iVar8 = *(int *)((int)&sk_npc_dmg_gunship.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX);
    }
    puVar7 = (undefined4 *)(iVar1 * 4 + iVar8);
    if (puVar7 != (undefined4 *)0x0) {
      *puVar7 = pcVar4;
    }
    *(char **)((int)&PTR_MoveDone_00c09cb0 + unaff_EBX) = pcVar4;
    ___cxa_guard_release(unaff_EBX + 0xcf6414);
  }
  *(undefined4 *)((int)&PTR_IsNPC_00c092b0 + unaff_EBX) = 6;
  *(int *)((int)&PTR_IsTriggered_00c092ac + unaff_EBX) =
       (int)&PTR_NetworkStateChanged_m_vecBaseVelocity_00c09b6c + unaff_EBX;
  return (datamap_t *)((int)&PTR_IsTriggered_00c092ac + unaff_EBX);
}


/* CHoverTurretTether::GetDataDescMap at 006be890 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CHoverTurretTether * this) */

datamap_t * __thiscall CHoverTurretTether::GetDataDescMap(CHoverTurretTether *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x60c088);
}


/* CHoverTurretTether::GetBaseMap at 006be8a0 */

datamap_t * CHoverTurretTether::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4ed2f4);
}


/* __tcf_0 at 0099a410 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41d686)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41d67a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41d686));
  }
  *(undefined4 *)(unaff_EBX + 0x41d686) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41d682)) {
    if (*(int *)(unaff_EBX + 0x41d67a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x21177a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x21177a),*(int *)(unaff_EBX + 0x41d67a));
      *(undefined4 *)(unaff_EBX + 0x41d67a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d67e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41d67a);
  *(int *)(unaff_EBX + 0x41d68a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41d682)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x21177a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x21177a),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41d67a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d67e) = 0;
  }
  return;
}


/* CHoverTurretTether::CHoverTurretTether at 006bfb40 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CHoverTurretTether(CHoverTurretTether * this) */

void __thiscall CHoverTurretTether::CHoverTurretTether(CHoverTurretTether *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::CBaseAnimating(&this->super_CBaseAnimating);
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5737d7);
  (this->m_hTurret).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hAttachPoint).super_CBaseHandle.m_Index = 0xffffffff;
  this->m_pSpring = (IPhysicsSpring *)0x0;
  (this->m_hRope).super_CBaseHandle.m_Index = 0xffffffff;
  return;
}


/* CHoverTurretTether::CHoverTurretTether at 006bfba0 */

/* DWARF original prototype: void CHoverTurretTether(CHoverTurretTether * this, CHoverTurretTether *
   this) */

void __thiscall
CHoverTurretTether::CHoverTurretTether(CHoverTurretTether *this,CHoverTurretTether *this_1)

{
  CHoverTurretTether(this);
  return;
}


/* CEntityFactory<CHoverTurretTether>::Create at 006c5e60 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CHoverTurretTether> * this,
   char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CHoverTurretTether>::Create
          (CEntityFactory<CHoverTurretTether> *this,char *pClassName)

{
  CHoverTurretTether *this_00;
  
                    /* Unresolved local var: CHoverTurretTether * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x528);
  CHoverTurretTether::CHoverTurretTether(this_00);
  (*(this_00->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1d])(this_00,pClassName);
  return &(this_00->super_CBaseAnimating).super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CHoverTurretTether::ReleaseTether at 006bf4b0 */

/* DWARF original prototype: void ReleaseTether(CHoverTurretTether * this) */

void __thiscall CHoverTurretTether::ReleaseTether(CHoverTurretTether *this)

{
  uint uVar1;
  CRopeKeyframe *this_00;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_pSpring != (IPhysicsSpring *)0x0) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4ec77b) + 0x3c))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4ec77b),this->m_pSpring);
    this->m_pSpring = (IPhysicsSpring *)0x0;
  }
  uVar1 = (this->m_hRope).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4ec6e7),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (this_00 = *(CRopeKeyframe **)(iVar2 + 4), this_00 != (CRopeKeyframe *)0x0)) {
    CRopeKeyframe::DetachPoint(this_00,1);
    return;
  }
  return;
}


/* CNPC_HoverTurret::Event_Killed at 006c4990 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void Event_Killed(CNPC_HoverTurret * this, CTakeDamageInfo * info) */

void __thiscall CNPC_HoverTurret::Event_Killed(CNPC_HoverTurret *this,CTakeDamageInfo *info)

{
  uint uVar1;
  IPhysicsMotionController *pIVar2;
  CRopeKeyframe *this_00;
  int iVar3;
  int iVar4;
  CBaseEntity *oldObj;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
  uVar1 = (this->m_hSmokeTrail).super_CBaseHandle.m_Index;
  if (((uVar1 == 0xffffffff) ||
      (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e7206),
      *(uint *)(iVar3 + 8) != uVar1 >> 0x10)) || (*(int *)(iVar3 + 4) == 0)) {
    CreateSmokeTrail(this);
    this->m_fSparkTime =
         *(float *)(**(int **)(unaff_EBX + 0x4e7202) + 0xc) + *(float *)(unaff_EBX + 0x365976);
  }
  pIVar2 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.m_pMotionController;
  if (pIVar2 != (IPhysicsMotionController *)0x0) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e729a) + 0x7c))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4e729a),pIVar2);
    (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
    m_pMotionController = (IPhysicsMotionController *)0x0;
  }
  iVar3 = **(int **)(unaff_EBX + 0x4e7206);
  uVar1 = (this->m_hTether).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar4 = (uVar1 & 0xffff) * 0x10 + iVar3, *(uint *)(iVar4 + 8) == uVar1 >> 0x10)) &&
     (iVar4 = *(int *)(iVar4 + 4), iVar4 != 0)) {
                    /* Unresolved local var: CHoverTurretTether * pTether@[???] */
    if (*(int *)(iVar4 + 0x520) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e729a) + 0x3c))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4e729a),*(int *)(iVar4 + 0x520));
      *(undefined4 *)(iVar4 + 0x520) = 0;
      iVar3 = **(int **)(unaff_EBX + 0x4e7206);
    }
    uVar1 = *(uint *)(iVar4 + 0x524);
    if (((uVar1 != 0xffffffff) &&
        (iVar3 = (uVar1 & 0xffff) * 0x10 + iVar3, *(uint *)(iVar3 + 8) == uVar1 >> 0x10)) &&
       (this_00 = *(CRopeKeyframe **)(iVar3 + 4), this_00 != (CRopeKeyframe *)0x0)) {
      CRopeKeyframe::DetachPoint(this_00,1);
    }
    CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x34abc3),0.0,(float *)0x0);
    uVar1 = (this->m_hTether).super_CBaseHandle.m_Index;
    if ((uVar1 == 0xffffffff) ||
       (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e7206),
       *(uint *)(iVar3 + 8) != uVar1 >> 0x10)) {
      oldObj = (CBaseEntity *)0x0;
    }
    else {
      oldObj = *(CBaseEntity **)(iVar3 + 4);
    }
    UTIL_Remove(oldObj);
    (this->m_hTether).super_CBaseHandle.m_Index = 0xffffffff;
  }
  if (this->m_bCarriedByPlayer == false) {
    StartDeathSequence(this);
  }
  SetFiringState(this,HOVER_TURRET_SHOT_DISABLED);
  if (((CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth> *)
      ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0xe8))->m_Value != 0
     ) {
    (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x200))
              (this,(CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth> *)
                    ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                           super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                           super_CBaseAnimating + 0xe8));
    ((CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth> *)
    ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0xe8))->m_Value = 0;
  }
  COutputEvent::FireOutput
            (&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.m_OnDeath,(CBaseEntity *)this,
             (CBaseEntity *)this,0.0);
  return;
}


/* CHoverTurretTether::UpdateOnRemove at 006bf420 */

/* DWARF original prototype: void UpdateOnRemove(CHoverTurretTether * this) */

void __thiscall CHoverTurretTether::UpdateOnRemove(CHoverTurretTether *this)

{
  uint uVar1;
  CRopeKeyframe *this_00;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_pSpring != (IPhysicsSpring *)0x0) {
    (**(code **)(*(int *)**(undefined4 **)(&DAT_004ec80b + unaff_EBX) + 0x3c))
              ((int *)**(undefined4 **)(&DAT_004ec80b + unaff_EBX),this->m_pSpring);
    this->m_pSpring = (IPhysicsSpring *)0x0;
  }
  CBaseEntity::UpdateOnRemove((CBaseEntity *)this);
  uVar1 = (this->m_hRope).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4ec777),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (this_00 = *(CRopeKeyframe **)(iVar2 + 4), this_00 != (CRopeKeyframe *)0x0)) {
    CRopeKeyframe::DetachPoint(this_00,1);
    return;
  }
  return;
}


/* CHoverTurretTether::AttachEntities at 006be8b0 */

/* DWARF original prototype: void AttachEntities(CHoverTurretTether * this, EHANDLE * hTurret,
   EHANDLE * hAttachPoint) */

void __thiscall
CHoverTurretTether::AttachEntities(CHoverTurretTether *this,EHANDLE *hTurret,EHANDLE *hAttachPoint)

{
  (this->m_hTurret).super_CBaseHandle.m_Index = (hTurret->super_CBaseHandle).m_Index;
  (this->m_hAttachPoint).super_CBaseHandle.m_Index = (hAttachPoint->super_CBaseHandle).m_Index;
  return;
}


/* CNPC_HoverTurret::Spawn at 006c3eb0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "MoveCollide_t": Some values do not have unique names */
/* WARNING: Enum "MoveType_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::Spawn(CNPC_HoverTurret *this)

{
  int *piVar1;
  float fVar2;
  vec_t *pvVar3;
  float *pfVar4;
  uint uVar5;
  ulong uVar6;
  vec_t vVar7;
  char *szName;
  CBaseEntity *pCVar8;
  uint *puVar9;
  ulong *puVar10;
  undefined4 *puVar11;
  model_t *pmVar12;
  vec_t vVar13;
  CStudioHdr *pCVar14;
  vec_t vVar15;
  CBaseEntity *this_00;
  int unaff_EBX;
  NetworkVar_m_Collision *this_01;
  int iVar16;
  longdouble lVar17;
  longdouble lVar18;
  longdouble lVar19;
  float fVar20;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  
                    /* Unresolved local var: Vector bobAmount@[???] */
  ___i686_get_pc_thunk_bx();
  (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x68))
            (this);
  (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x6c))
            (this,unaff_EBX + 0x34b56b);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_CBaseCombatCharacter.m_eHull = HULL_TINY_CENTERED;
  CAI_BaseNPC::SetHullSizeNormal((CAI_BaseNPC *)this,false);
  this_01 = &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
             m_Collision;
  CCollisionProperty::SetSolid(&this_01->super_CCollisionProperty,SOLID_BBOX);
  CCollisionProperty::SetSolidFlags
            (&this_01->super_CCollisionProperty,
             (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
             m_Collision.super_CCollisionProperty.m_usSolidFlags.m_Value | 0x10);
  CBaseEntity::SetMoveType((CBaseEntity *)this,MOVETYPE_VPHYSICS,MOVECOLLIDE_DEFAULT);
  iVar16 = (int)*(float *)(*(int *)(unaff_EBX + 0x66ac3e) + 0x2c);
  if (iVar16 != (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                m_iHealth.m_Value) {
    (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x200))
              (this,&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                     super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                     super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                     super_CBaseEntity.m_iHealth);
    (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
    super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
    super_CBaseAnimating.super_CBaseEntity.m_iHealth.m_Value = iVar16;
  }
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.m_NPCState = NPC_STATE_NONE;
  iVar16 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.m_poseAim_Yaw;
  pCVar14 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
            super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.m_pStudioHdr;
  if (pCVar14 == (CStudioHdr *)0x0) {
    pmVar12 = CBaseEntity::GetModel((CBaseEntity *)this);
    if (pmVar12 == (model_t *)0x0) {
      pCVar14 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.m_pStudioHdr;
    }
    else {
      CBaseAnimating::LockStudioHdr((CBaseAnimating *)this);
      pCVar14 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.m_pStudioHdr;
    }
    if (pCVar14 != (CStudioHdr *)0x0) goto LAB_006c3fa1;
  }
  else {
LAB_006c3fa1:
    if (pCVar14->m_pStudioHdr == (studiohdr_t *)0x0) {
      pCVar14 = (CStudioHdr *)0x0;
    }
  }
  CBaseAnimating::SetPoseParameter((CBaseAnimating *)this,pCVar14,iVar16,0.0);
  iVar16 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.m_poseAim_Pitch;
  pCVar14 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
            super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.m_pStudioHdr;
  if (pCVar14 == (CStudioHdr *)0x0) {
    pmVar12 = CBaseEntity::GetModel((CBaseEntity *)this);
    if (pmVar12 != (model_t *)0x0) {
      CBaseAnimating::LockStudioHdr((CBaseAnimating *)this);
    }
    pCVar14 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
              super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.m_pStudioHdr;
    if (pCVar14 != (CStudioHdr *)0x0) goto LAB_006c3fe1;
  }
  else {
LAB_006c3fe1:
    if (pCVar14->m_pStudioHdr == (studiohdr_t *)0x0) {
      pCVar14 = (CStudioHdr *)0x0;
    }
  }
  CBaseAnimating::SetPoseParameter((CBaseAnimating *)this,pCVar14,iVar16,0.0);
  (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x684))
            (this,*(undefined4 *)(unaff_EBX + 0x66ac7a));
  this->m_iDesiredState = 1;
  this->m_iLastState = 1;
  SetFiringState(this,HOVER_TURRET_FIND_TARGET);
  CAI_BaseNPC::SetNavType((CAI_BaseNPC *)this,NAV_FLY);
  piVar1 = &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
            super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
            m_iEFlags;
  *piVar1 = *piVar1 | 0x18000000;
  CBaseEntity::AddEffects((CBaseEntity *)this,0x10);
  CBaseCombatCharacter::SetBloodColor((CBaseCombatCharacter *)this,-1);
  pvVar3 = *(vec_t **)(unaff_EBX + 0x4e7cf2);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vCurrentVelocity.x = *pvVar3;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vCurrentVelocity.y = pvVar3[1];
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vCurrentVelocity.z = pvVar3[2];
  (this->m_vForceVelocity).x = 0.0;
  (this->m_vForceVelocity).y = 0.0;
  (this->m_vForceVelocity).z = 0.0;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vCurrentBanking.x = 0.0;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vCurrentBanking.y = 0.0;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vCurrentBanking.z = 0.0;
  (this->m_vTargetBanking).x = 0.0;
  (this->m_vTargetBanking).y = 0.0;
  (this->m_vTargetBanking).z = 0.0;
  CAI_BaseNPC::CapabilitiesAdd((CAI_BaseNPC *)this,0x4080004);
  lVar17 = (longdouble)
           (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e7d66) + 4))
                     ((int *)**(undefined4 **)(unaff_EBX + 0x4e7d66),0xbf800000,0x3f800000);
  lVar18 = (longdouble)
           (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e7d66) + 4))
                     ((int *)**(undefined4 **)(unaff_EBX + 0x4e7d66),0xbf800000,0x3f800000);
  lVar19 = (longdouble)
           (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e7d66) + 4))
                     ((int *)**(undefined4 **)(unaff_EBX + 0x4e7d66),0xbf800000,0x3f800000);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vNoiseMod.x = (float)lVar17;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vNoiseMod.y = (float)lVar18;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vNoiseMod.z = (float)lVar19;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_fHeadYaw = 0.0;
  (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x55c))
            (this);
  local_28 = 0;
  local_24 = 0;
  local_20 = 0x41200000;
  (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x240))
            (this,&local_28);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_CBaseCombatCharacter.m_flFieldOfView = 0.4;
  CBaseEntity::SetCollisionGroup((CBaseEntity *)this,0);
  iVar16 = CBaseAnimating::LookupAttachment
                     ((CBaseAnimating *)this,
                      (char *)(CAI_BasePhysicsFlyingBot::TurnHeadToTarget + unaff_EBX + 2));
  this->m_iMuzzleAttachment = iVar16;
  if (((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
       super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
  vVar13 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
           m_angAbsRotation.x;
  (this->m_vecGoalAngles).x = vVar13;
  vVar15 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
           m_angAbsRotation.y;
  (this->m_vecGoalAngles).y = vVar15;
  vVar7 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
          super_CBaseAnimating.super_CBaseEntity.m_angAbsRotation.z;
  (this->m_vecGoalAngles).z = vVar7;
  if (((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
       super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    vVar13 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
             m_angAbsRotation.x;
    vVar15 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
             m_angAbsRotation.y;
    vVar7 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
            super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
            m_angAbsRotation.z;
  }
  (this->m_vInitialLookAngles).x = vVar13;
  (this->m_vInitialLookAngles).y = vVar15;
  (this->m_vInitialLookAngles).z = vVar7;
  this->m_bCarriedByPlayer = false;
  this->m_flDeathTime = 0.0;
  if (((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
       super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
  fVar20 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
           m_vecAbsOrigin.x;
  pfVar4 = *(float **)(unaff_EBX + 0x4e7cf2);
  if ((fVar20 == *pfVar4) && (!NAN(fVar20) && !NAN(*pfVar4))) {
    fVar2 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
            super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
            m_vecAbsOrigin.y;
    if ((fVar2 == pfVar4[1]) && (!NAN(fVar2) && !NAN(pfVar4[1]))) {
      fVar2 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
              super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
              m_vecAbsOrigin.z;
      if ((fVar2 == pfVar4[2]) && (!NAN(fVar2) && !NAN(pfVar4[2]))) {
        this->m_bInitialPositionSet = false;
        goto LAB_006c42dd;
      }
    }
  }
  this->m_bInitialPositionSet = true;
  if (((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
       super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    fVar20 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
             m_vecAbsOrigin.x;
  }
  (this->m_vForceMoveTarget).x = fVar20;
  (this->m_vForceMoveTarget).y =
       (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
       super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.y;
  (this->m_vForceMoveTarget).z =
       (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
       super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z;
LAB_006c42dd:
  szName = (this->m_strAttachName).pszValue;
  if (szName == (char *)0x0) {
    szName = &UNK_002e7c9e + unaff_EBX;
  }
  pCVar8 = CGlobalEntityList::FindEntityByName
                     (*(CGlobalEntityList **)(&DAT_004e7da2 + unaff_EBX),(CBaseEntity *)0x0,szName,
                      (CBaseEntity *)0x0,(CBaseEntity *)0x0,(CBaseEntity *)0x0,
                      (IEntityFindFilter *)0x0);
  if (pCVar8 == (CBaseEntity *)0x0) {
    (this->m_hAttachTarget).super_CBaseHandle.m_Index = 0xffffffff;
    return;
  }
  puVar9 = (uint *)(*(pCVar8->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                     _vptr_IHandleEntity[3])(pCVar8);
  uVar5 = *puVar9;
  (this->m_hAttachTarget).super_CBaseHandle.m_Index = uVar5;
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???]
                       Unresolved local var: CEntInfo * pInfo@[???] */
  if ((((uVar5 != 0xffffffff) &&
       (iVar16 = (uVar5 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e7ce6),
       *(uint *)(iVar16 + 8) == uVar5 >> 0x10)) && (*(int *)(iVar16 + 4) != 0)) &&
     (((uVar5 = (this->m_hTether).super_CBaseHandle.m_Index, uVar5 == 0xffffffff ||
       (iVar16 = (uVar5 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e7ce6),
       *(uint *)(iVar16 + 8) != uVar5 >> 0x10)) || (*(int *)(iVar16 + 4) == 0)))) {
    pCVar8 = CreateEntityByName((char *)(unaff_EBX + 0x34b535),-1,true);
    if (pCVar8 == (CBaseEntity *)0x0) {
      (this->m_hTether).super_CBaseHandle.m_Index = 0xffffffff;
    }
    else {
      puVar10 = (ulong *)(*(pCVar8->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                           _vptr_IHandleEntity[3])(pCVar8);
      (this->m_hTether).super_CBaseHandle.m_Index = *puVar10;
    }
    uVar5 = (this->m_hAttachTarget).super_CBaseHandle.m_Index;
    if ((uVar5 == 0xffffffff) ||
       (iVar16 = (uVar5 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e7ce6),
       *(uint *)(iVar16 + 8) != uVar5 >> 0x10)) {
      pCVar8 = (CBaseEntity *)0x0;
    }
    else {
      pCVar8 = *(CBaseEntity **)(iVar16 + 4);
    }
    if ((pCVar8->m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition(pCVar8);
    }
    uVar5 = (this->m_hTether).super_CBaseHandle.m_Index;
    if ((uVar5 == 0xffffffff) ||
       (iVar16 = (uVar5 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e7ce6),
       *(uint *)(iVar16 + 8) != uVar5 >> 0x10)) {
      this_00 = (CBaseEntity *)0x0;
    }
    else {
      this_00 = *(CBaseEntity **)(iVar16 + 4);
    }
    CBaseEntity::SetAbsOrigin(this_00,&pCVar8->m_vecAbsOrigin);
    uVar5 = (this->m_hTether).super_CBaseHandle.m_Index;
    if ((uVar5 == 0xffffffff) ||
       (iVar16 = (uVar5 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e7ce6),
       *(uint *)(iVar16 + 8) != uVar5 >> 0x10)) {
      iVar16 = 0;
    }
    else {
      iVar16 = *(int *)(iVar16 + 4);
    }
    uVar6 = (this->m_hAttachTarget).super_CBaseHandle.m_Index;
    puVar11 = (undefined4 *)
              (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                    super_CBaseCombatCharacter.super_CBaseFlex.
                                    super_CBaseAnimatingOverlay.super_CBaseAnimating + 0xc))(this);
    *(undefined4 *)(iVar16 + 0x51c) = *puVar11;
    *(ulong *)(iVar16 + 0x518) = uVar6;
    uVar5 = (this->m_hTether).super_CBaseHandle.m_Index;
    if ((uVar5 == 0xffffffff) ||
       (iVar16 = (uVar5 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e7ce6),
       *(uint *)(iVar16 + 8) != uVar5 >> 0x10)) {
      pCVar8 = (CBaseEntity *)0x0;
    }
    else {
      pCVar8 = *(CBaseEntity **)(iVar16 + 4);
    }
    DispatchSpawn(pCVar8,true);
    uVar5 = (this->m_hAttachTarget).super_CBaseHandle.m_Index;
    if ((uVar5 == 0xffffffff) ||
       (iVar16 = (uVar5 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e7ce6),
       *(uint *)(iVar16 + 8) != uVar5 >> 0x10)) {
      pCVar8 = (CBaseEntity *)0x0;
    }
    else {
      pCVar8 = *(CBaseEntity **)(iVar16 + 4);
    }
    if (((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
         super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
         super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
    if ((pCVar8->m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition(pCVar8);
    }
  }
  return;
}


/* CHoverTurretTether::ToggleThroughPortal at 006bebd0 */

/* DWARF original prototype: void ToggleThroughPortal(CHoverTurretTether * this, CPortal_Base2D *
   pEnteredPortal) */

void __thiscall
CHoverTurretTether::ToggleThroughPortal(CHoverTurretTether *this,CPortal_Base2D *pEnteredPortal)

{
  uint uVar1;
  int *piVar2;
  int iVar3;
  int unaff_EBX;
  CTakeDamageInfo *in_stack_ffffff7c;
  CBaseEntity *in_stack_ffffff80;
  CBaseEntity *in_stack_ffffff84;
  float in_stack_ffffff88;
  int in_stack_ffffff8c;
  code *iKillType_1;
  CTakeDamageInfo local_6c;
  
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->m_hTurret).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4ecfbd),
      *(uint *)(iVar3 + 8) == uVar1 >> 0x10)) &&
     (piVar2 = *(int **)(iVar3 + 4), piVar2 != (int *)0x0)) {
                    /* Unresolved local var: CAI_BaseNPC * pTurret@[???] */
    iKillType_1 = *(code **)(*piVar2 + 0x118);
    CTakeDamageInfo::CTakeDamageInfo
              (&local_6c,(CBaseEntity *)this,(CBaseEntity *)this,200.0,0x40,0,in_stack_ffffff7c,
               in_stack_ffffff80,in_stack_ffffff84,in_stack_ffffff88,in_stack_ffffff8c,
               (int)iKillType_1);
    (*iKillType_1)(piVar2,&local_6c);
  }
  return;
}


/* CNPC_HoverTurret::NotifySystemEvent at 006bfbb0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void NotifySystemEvent(CNPC_HoverTurret * this, CBaseEntity * pNotify,
   notify_system_event_t eventType, notify_system_event_params_t * params) */

void __thiscall
CNPC_HoverTurret::NotifySystemEvent
          (CNPC_HoverTurret *this,CBaseEntity *pNotify,notify_system_event_t eventType,
          notify_system_event_params_t *params)

{
  uint uVar1;
  CBaseEntity *pInflictor;
  code *pcVar2;
  char cVar3;
  int iVar4;
  int iVar5;
  int unaff_EBX;
  CTakeDamageInfo *in_stack_ffffff7c;
  CBaseEntity *in_stack_ffffff80;
  CBaseEntity *in_stack_ffffff84;
  float in_stack_ffffff88;
  int in_stack_ffffff8c;
  int *iKillType_1;
  CTakeDamageInfo local_6c;
  
  ___i686_get_pc_thunk_bx();
  if (eventType == NOTIFY_EVENT_TELEPORT) {
    cVar3 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                  super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                  super_CBaseCombatCharacter.super_CBaseFlex.
                                  super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x114))(this);
    if (cVar3 != '\0') {
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
      iVar5 = **(int **)(unaff_EBX + 0x4ebfdd);
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
      uVar1 = (this->m_hTether).super_CBaseHandle.m_Index;
                    /* Unresolved local var: CHoverTurretTether * pHoverTurretTether@[???]
                       Unresolved local var: CPortal_Base2D * pEnteredPortal@[???] */
      if ((((uVar1 != 0xffffffff) &&
           (iVar4 = (uVar1 & 0xffff) * 0x10 + iVar5, *(uint *)(iVar4 + 8) == uVar1 >> 0x10)) &&
          (pInflictor = *(CBaseEntity **)(iVar4 + 4), pInflictor != (CBaseEntity *)0x0)) &&
         (pNotify != (CBaseEntity *)0x0)) {
        iVar4 = ___dynamic_cast(pNotify,*(undefined4 *)(unaff_EBX + 0x4ec001),
                                *(undefined4 *)(unaff_EBX + 0x4ec331),0);
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???]
                       Unresolved local var: CEntInfo * pInfo@[???] */
        if (((iVar4 != 0) &&
            (uVar1 = pInflictor[1].m_DamageModifiers.m_LastAlloc.index, uVar1 != 0xffffffff)) &&
           (iVar5 = (uVar1 & 0xffff) * 0x10 + iVar5, *(uint *)(iVar5 + 8) == uVar1 >> 0x10)) {
          iKillType_1 = *(int **)(iVar5 + 4);
          if (iKillType_1 != (int *)0x0) {
                    /* Unresolved local var: CAI_BaseNPC * pTurret@[???] */
            pcVar2 = *(code **)(*iKillType_1 + 0x118);
            CTakeDamageInfo::CTakeDamageInfo
                      (&local_6c,pInflictor,pInflictor,200.0,0x40,0,in_stack_ffffff7c,
                       in_stack_ffffff80,in_stack_ffffff84,in_stack_ffffff88,in_stack_ffffff8c,
                       (int)iKillType_1);
            (*pcVar2)(iKillType_1,&local_6c);
          }
        }
      }
    }
  }
  return;
}


/* CHoverTurretTether::Spawn at 006c0810 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "MoveCollide_t": Some values do not have unique names */
/* WARNING: Enum "MoveType_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(CHoverTurretTether * this) */

void __thiscall CHoverTurretTether::Spawn(CHoverTurretTether *this)

{
  NetworkVar_m_Collision *this_00;
  uint uVar1;
  BASEPTR func;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::Precache(&this->super_CBaseAnimating);
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1b])(this,unaff_EBX + 0x34ec0b);
  CBaseEntity::AddEffects((CBaseEntity *)this,0x20);
  this_00 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Collision;
  CCollisionProperty::SetSolid(&this_00->super_CCollisionProperty,SOLID_VPHYSICS);
  CCollisionProperty::SetSolidFlags
            (&this_00->super_CCollisionProperty,
             (this->super_CBaseAnimating).super_CBaseEntity.m_Collision.super_CCollisionProperty.
             m_usSolidFlags.m_Value | 4);
  CBaseEntity::VPhysicsInitShadow((CBaseEntity *)this,false,false,(solid_t *)0x0);
  CBaseEntity::SetMoveType((CBaseEntity *)this,MOVETYPE_NONE,MOVECOLLIDE_DEFAULT);
  CBaseAnimating::Spawn(&this->super_CBaseAnimating);
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
  uVar1 = (this->m_hAttachPoint).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4eb386),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) && (*(int *)(iVar2 + 4) != 0)) {
    CBaseEntity::SetNextThink
              ((CBaseEntity *)this,*(float *)(**(int **)(unaff_EBX + 0x4eb382) + 0xc),(char *)0x0);
    func.__delta = 0;
    func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x73e);
    CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
    PullThink(this);
    return;
  }
  return;
}


/* __tcf_4 at 0099a2f0 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x41d568),in_stack_00000008);
  return;
}


/* CHoverTurretTether::PullThink at 006c00e0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void PullThink(CHoverTurretTether * this) */

void __thiscall CHoverTurretTether::PullThink(CHoverTurretTether *this)

{
  float *pfVar1;
  float fVar2;
  float fVar3;
  int *piVar4;
  uint uVar5;
  CBaseEntity *this_00;
  IPhysicsObject *pIVar6;
  IPhysicsObject *pIVar7;
  bool bVar8;
  int iVar9;
  IPhysicsSpring *pIVar10;
  CRopeKeyframe *pCVar11;
  uint *puVar12;
  int unaff_EBX;
  CTakeDamageInfo *this_01;
  _func_int_varargs *p_Var13;
  CTraceFilterSimple *pCVar14;
  float fVar15;
  IHandleEntity *pIVar16;
  CTakeDamageInfo *in_stack_fffffdcc;
  CBaseEntity *in_stack_fffffdd0;
  CBaseEntity *in_stack_fffffdd4;
  float in_stack_fffffdd8;
  int in_stack_fffffddc;
  int in_stack_fffffde0;
  CGameTrace local_1f0;
  CTakeDamageInfo local_19c;
  float local_14c;
  float local_148;
  float local_144;
  float local_13c;
  float local_138;
  float local_134;
  undefined4 local_12c;
  undefined4 local_128;
  undefined4 local_124;
  undefined4 local_11c;
  undefined4 local_118;
  undefined4 local_114;
  undefined4 local_10c;
  undefined1 local_108;
  undefined1 local_107;
  CTakeDamageInfo local_f8;
  CTraceFilterSimple local_a8;
  float local_98;
  float local_94;
  float local_90;
  vec_t local_8c;
  vec_t local_88;
  vec_t local_84;
  undefined1 local_80;
  undefined1 local_7f;
  float local_7c;
  undefined4 uStack_78;
  undefined4 uStack_74;
  undefined4 uStack_70;
  float local_64;
  float local_60;
  float local_5c;
  float local_58;
  float local_54;
  float local_50;
  float local_4c;
  float local_48;
  float local_44;
  float local_40;
  float local_3c;
  float local_38;
  float local_34;
  float local_30;
  float local_2c;
  float local_28;
  float local_24;
  float local_20;
  
                    /* Unresolved local var: CAI_BaseNPC * pTurret@[???] */
  ___i686_get_pc_thunk_bx();
  piVar4 = *(int **)(unaff_EBX + 0x4ebab3);
  uVar5 = (this->m_hTurret).super_CBaseHandle.m_Index;
  if (((uVar5 == 0xffffffff) ||
      (iVar9 = (uVar5 & 0xffff) * 0x10 + *piVar4, *(uint *)(iVar9 + 8) != uVar5 >> 0x10)) ||
     (this_00 = *(CBaseEntity **)(iVar9 + 4), this_00 == (CBaseEntity *)0x0)) {
    return;
  }
  if (this->m_pSpring == (IPhysicsSpring *)0x0) {
                    /* Unresolved local var: Vector vecDistance@[???]
                       Unresolved local var: IPhysicsObject * pPhysObject@[???]
                       Unresolved local var: IPhysicsObject * pAttachPhysObject@[???]
                       Unresolved local var: springparams_t spring@[???]
                       Unresolved local var: Vector up@[???] */
    if (((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
    if ((this_00->m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition(this_00);
    }
                    /* Unresolved local var: Vector res@[???] */
    local_28 = (this_00->m_vecAbsOrigin).x -
               (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.x;
    local_24 = (this_00->m_vecAbsOrigin).y -
               (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.y;
    local_20 = (this_00->m_vecAbsOrigin).z -
               (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.z;
                    /* Unresolved local var: __m128 root@[???] */
    local_7c = SQRT(local_28 * local_28 + local_24 * local_24 + local_20 * local_20);
    uStack_78 = 0;
    uStack_74 = 0;
    uStack_70 = 0;
    in_stack_fffffdd4 = *(CBaseEntity **)(unaff_EBX + 0x36a443);
    pIVar16 = (IHandleEntity *)(local_7c - (float)in_stack_fffffdd4);
    this->m_flNaturalDistance = (float)pIVar16;
    pIVar6 = this_00->m_pPhysicsObject;
    pIVar7 = (this->super_CBaseAnimating).super_CBaseEntity.m_pPhysicsObject;
    pCVar14 = &local_a8;
    for (iVar9 = 0xb; iVar9 != 0; iVar9 = iVar9 + -1) {
      (pCVar14->super_CTraceFilter).super_ITraceFilter._vptr_ITraceFilter =
           (_func_int_varargs **)0x0;
      pCVar14 = (CTraceFilterSimple *)&pCVar14->m_pPassEnt;
    }
    local_a8.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter =
         (ITraceFilter)(_func_int_varargs **)0x461c4000;
    local_a8.m_collisionGroup = 0x41800000;
    local_a8.m_pExtraShouldHitCheckFunction = (ShouldHitFunc_t)0x0;
    local_a8.m_pPassEnt = pIVar16;
    (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
      super_IHandleEntity._vptr_IHandleEntity[0x8e])(this,0,0,&local_4c);
                    /* Unresolved local var: Vector res@[???] */
    local_64 = local_4c * (float)in_stack_fffffdd4;
    local_60 = local_48 * (float)in_stack_fffffdd4;
    local_5c = (float)in_stack_fffffdd4 * local_44;
    local_34 = local_64;
    local_30 = local_60;
    local_2c = local_5c;
    if ((this_00->m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition(this_00);
    }
                    /* Unresolved local var: Vector res@[???] */
    local_98 = (this_00->m_vecAbsOrigin).x + local_34;
    local_94 = (this_00->m_vecAbsOrigin).y + local_30;
    local_90 = (this_00->m_vecAbsOrigin).z + local_2c;
    local_40 = local_98;
    local_3c = local_94;
    local_38 = local_90;
    if (((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
    local_8c = (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.x;
    local_88 = (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.y;
    local_84 = (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.z;
    local_80 = 0;
    local_7f = 1;
    pIVar10 = (IPhysicsSpring *)
              (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4ebb47) + 0x38))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x4ebb47),pIVar6,pIVar7,&local_a8);
    this->m_pSpring = pIVar10;
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
    uVar5 = (this->m_hRope).super_CBaseHandle.m_Index;
    if (((uVar5 == 0xffffffff) ||
        (iVar9 = (uVar5 & 0xffff) * 0x10 + *piVar4, *(uint *)(iVar9 + 8) != uVar5 >> 0x10)) ||
       (*(int *)(iVar9 + 4) == 0)) {
      in_stack_fffffdd0 = (CBaseEntity *)(unaff_EBX + 0x2fa754);
      in_stack_fffffdcc = (CTakeDamageInfo *)((int)&vgui::Panel::GetMessageMap::s_pMap.cputype + 1);
      pCVar11 = CRopeKeyframe::Create
                          ((CBaseEntity *)this,this_00,0,0,1,&UNK_0030d7a7 + unaff_EBX,5,
                           (char *)in_stack_fffffdd0);
      if (pCVar11 == (CRopeKeyframe *)0x0) {
        (this->m_hRope).super_CBaseHandle.m_Index = 0xffffffff;
        pCVar11 = (CRopeKeyframe *)0x0;
      }
      else {
        puVar12 = (uint *)(*(pCVar11->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.
                            super_IHandleEntity._vptr_IHandleEntity[3])(pCVar11);
        uVar5 = *puVar12;
        (this->m_hRope).super_CBaseHandle.m_Index = uVar5;
        if ((uVar5 == 0xffffffff) ||
           (iVar9 = (uVar5 & 0xffff) * 0x10 + *piVar4, *(uint *)(iVar9 + 8) != uVar5 >> 0x10)) {
          pCVar11 = (CRopeKeyframe *)0x0;
        }
        else {
          pCVar11 = *(CRopeKeyframe **)(iVar9 + 4);
        }
      }
      CRopeKeyframe::EnableWind(pCVar11,true);
    }
  }
  (*this->m_pSpring->_vptr_IPhysicsSpring[2])(this->m_pSpring,&local_4c,&local_58);
  local_4c = local_4c - local_58;
  local_48 = local_48 - local_54;
  local_44 = local_44 - local_50;
                    /* Unresolved local var: __m128 root@[???] */
  local_7c = SQRT(local_4c * local_4c + local_48 * local_48 + local_44 * local_44);
  uStack_78 = 0;
  uStack_74 = 0;
  uStack_70 = 0;
  fVar15 = local_7c - this->m_flNaturalDistance;
  pfVar1 = (float *)(*(int *)(unaff_EBX + 0x6f778b) + 0x2c);
  if (*pfVar1 <= fVar15 && fVar15 != *pfVar1) {
    p_Var13 = (this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[0x46];
    this_01 = &local_f8;
  }
  else {
    if (((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
    fVar15 = (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.z;
    fVar2 = (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.y;
    fVar3 = (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.x;
    local_10c = 0;
    if ((this_00->m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition(this_00);
    }
    local_13c = (this_00->m_vecAbsOrigin).x - fVar3;
    local_138 = (this_00->m_vecAbsOrigin).y - fVar2;
    local_134 = (this_00->m_vecAbsOrigin).z - fVar15;
    local_107 = local_13c * local_13c + local_138 * local_138 + local_134 * local_134 != 0.0;
    local_114 = 0;
    local_118 = 0;
    local_11c = 0;
    local_10c = 0;
    local_108 = 1;
    local_124 = 0;
    local_128 = 0;
    local_12c = 0;
    local_14c = fVar3;
    local_148 = fVar2;
    local_144 = fVar15;
    CTraceFilterSimple::CTraceFilterSimple(&local_a8,(IHandleEntity *)0x0,0,(ShouldHitFunc_t)0x0);
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4ebb1b) + 0x14))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4ebb1b),&local_14c,0x400b,&local_a8,&local_1f0
              );
    if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x4ebb17) + 0x1c) + 0x30) != 0) {
      in_stack_fffffdcc = (CTakeDamageInfo *)0xbf800000;
      DebugDrawLine((Vector *)&local_1f0,&local_1f0.super_CBaseTrace.endpos,0xff,0,0,true,-1.0);
    }
    bVar8 = CGameTrace::DidHitWorld(&local_1f0);
    if (!bVar8) {
      CBaseEntity::SetNextThink
                ((CBaseEntity *)this,*(float *)(**(int **)(unaff_EBX + 0x4ebaaf) + 0xc),(char *)0x0)
      ;
      return;
    }
    p_Var13 = (this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[0x46];
    this_01 = &local_19c;
  }
  CTakeDamageInfo::CTakeDamageInfo
            (this_01,(CBaseEntity *)this,(CBaseEntity *)this,200.0,0x40,0,in_stack_fffffdcc,
             in_stack_fffffdd0,in_stack_fffffdd4,in_stack_fffffdd8,in_stack_fffffddc,
             in_stack_fffffde0);
  (*p_Var13)(this_00,this_01);
  return;
}


/* CNPC_HoverTurret::Classify at 006c5a20 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: Class_T Classify(CNPC_HoverTurret * this) */

Class_T __thiscall CNPC_HoverTurret::Classify(CNPC_HoverTurret *this)

{
  return CLASS_COMBINE;
}


/* CNPC_HoverTurret::BodyTarget at 006c5a30 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: Vector BodyTarget(CNPC_HoverTurret * this, Vector * posSrc, bool
   bNoisy) */

Vector * __thiscall
CNPC_HoverTurret::BodyTarget
          (Vector *__return_storage_ptr__,CNPC_HoverTurret *this,Vector *posSrc,bool bNoisy)

{
  void *pvVar1;
  
  pvVar1 = (void *)(**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>)
                                         .super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                         super_CBaseCombatCharacter.super_CBaseFlex.
                                         super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x274))
                             (this);
  _memmove(__return_storage_ptr__,pvVar1,0xc);
  return __return_storage_ptr__;
}


/* CNPC_HoverTurret::GetHeadTurnRate at 006c5a70 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: float GetHeadTurnRate(CNPC_HoverTurret * this) */

float __thiscall CNPC_HoverTurret::GetHeadTurnRate(CNPC_HoverTurret *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(float *)(extraout_ECX + 0x364d08);
}


/* CNPC_HoverTurret::ObjectCaps at 006c5e40 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: int ObjectCaps(CNPC_HoverTurret * this) */

int __thiscall CNPC_HoverTurret::ObjectCaps(CNPC_HoverTurret *this)

{
  uint uVar1;
  
  uVar1 = CBaseEntity::ObjectCaps((CBaseEntity *)this);
  return uVar1 | 0x201;
}


/* CNPC_HoverTurret::Use at 006c5a80 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void Use(CNPC_HoverTurret * this, CBaseEntity * pActivator, CBaseEntity
   * pCaller, $_170 useType, float value) */

void __thiscall
CNPC_HoverTurret::Use
          (CNPC_HoverTurret *this,CBaseEntity *pActivator,CBaseEntity *pCaller,__170 useType,
          float value)

{
  int iVar1;
  
                    /* Unresolved local var: CBasePlayer * pPlayer@[???] */
  if (pActivator != (CBaseEntity *)0x0) {
    iVar1 = (*(pActivator->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[0x55])(pActivator);
    if ((char)iVar1 != '\0') {
                    /* WARNING: Could not recover jumptable at 0x006c5ace. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (*(pActivator->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
        _vptr_IHandleEntity[0x1b7])();
      return;
    }
  }
  return;
}


/* CNPC_HoverTurret::GetClassScheduleIdSpace at 006c5ad0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: CAI_ClassScheduleIdSpace * GetClassScheduleIdSpace(CNPC_HoverTurret *
   this) */

CAI_ClassScheduleIdSpace * __thiscall
CNPC_HoverTurret::GetClassScheduleIdSpace(CNPC_HoverTurret *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (CAI_ClassScheduleIdSpace *)(extraout_ECX + 0x6690a8);
}


/* CNPC_HoverTurret::GetSchedulingErrorName at 006c5ae0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: char * GetSchedulingErrorName(CNPC_HoverTurret * this) */

char * __thiscall CNPC_HoverTurret::GetSchedulingErrorName(CNPC_HoverTurret *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(char **)(extraout_ECX + 0x604e74);
}


/* CNPC_HoverTurret::AllowedToIgnite at 006c5af0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: bool AllowedToIgnite(CNPC_HoverTurret * this) */

bool __thiscall CNPC_HoverTurret::AllowedToIgnite(CNPC_HoverTurret *this)

{
  return true;
}


/* DataMapInit<CNPC_HoverTurret> at 000c1890 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */

datamap_t * DataMapInit<CNPC_HoverTurret>(CNPC_HoverTurret *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&sk_max_crossbow.super_ConCommandBase.m_pszName + unaff_EBX) == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcf61ac);
    if (iVar2 != 0) {
      *(int *)((int)&sk_max_crossbow.m_pParent + unaff_EBX) = unaff_EBX + 0x94db27;
      *(undefined4 *)((int)&sk_max_crossbow.m_Value.m_pszString + unaff_EBX) = 0;
      *(undefined4 *)((int)&sk_max_crossbow.m_Value.m_StringLength + unaff_EBX) = 0;
      *(undefined4 *)((int)&sk_max_crossbow.m_Value.m_fValue + unaff_EBX) = 0;
      *(undefined4 *)((int)&sk_max_crossbow.m_Value.m_nValue + unaff_EBX) = 0;
      *(undefined4 *)(&sk_max_crossbow.m_bHasMin + unaff_EBX) = 0;
      *(undefined4 *)((int)&sk_max_crossbow.m_pszDefaultValue + unaff_EBX) = 0x10;
      ___cxa_guard_release(unaff_EBX + 0xcf61ac);
      ___cxa_atexit(unaff_EBX + 0x8d8a8c,0,*(undefined4 *)(&DAT_00aea2ec + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_ResolveFlyCollisionCustom_00c090a0 + unaff_EBX) =
       *(undefined4 *)(&DAT_00aeae50 + unaff_EBX);
  if (*(char *)((int)&sk_max_crossbow.super_ConCommandBase.m_nFlags + unaff_EBX) == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcf61b4);
    if (iVar2 != 0) {
      uVar1 = **(undefined4 **)(&DAT_00aea308 + unaff_EBX);
      *(undefined4 *)((int)&PTR_NetworkStateChanged_m_vecBaseVelocity_00c097d0 + unaff_EBX) = uVar1;
      *(undefined4 *)((int)&PTR_OnEntityDeleted_00c09810 + unaff_EBX) = uVar1;
      *(undefined4 *)((int)&PTR_Create_00c09850 + unaff_EBX) = uVar1;
      *(undefined4 *)((int)&PTR_SetRefEHandle_00c09890 + unaff_EBX) = uVar1;
      ___cxa_guard_release(unaff_EBX + 0xcf61b4);
    }
  }
  *(undefined4 *)((int)&PTR_HasPhysicsAttacker_00c09098 + unaff_EBX) = 0x20;
  *(int *)((int)&PTR_CanPushEntity_00c09094 + unaff_EBX) =
       (int)&PTR_FireGameEvent_00c090fc + unaff_EBX;
  return (datamap_t *)((int)&PTR_CanPushEntity_00c09094 + unaff_EBX);
}


/* CNPC_HoverTurret::GetDataDescMap at 006be8d0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: datamap_t * GetDataDescMap(CNPC_HoverTurret * this) */

datamap_t * __thiscall CNPC_HoverTurret::GetDataDescMap(CNPC_HoverTurret *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(CPhysicsNPCSolver::~CPhysicsNPCSolver + extraout_ECX);
}


/* CNPC_HoverTurret::GetBaseMap at 006be8e0 */

datamap_t * CNPC_HoverTurret::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4ede0c);
}


/* __tcf_1 at 0099a330 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41d736)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41d72a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41d736));
  }
  *(undefined4 *)(unaff_EBX + 0x41d736) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41d732)) {
    if (*(int *)(unaff_EBX + 0x41d72a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x21185a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x21185a),*(int *)(unaff_EBX + 0x41d72a));
      *(undefined4 *)(unaff_EBX + 0x41d72a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d72e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41d72a);
  *(int *)(unaff_EBX + 0x41d73a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41d732)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x21185a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x21185a),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41d72a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d72e) = 0;
  }
  return;
}


/* ServerClassInit<DT_NPC_HoverTurret::ignored> at 000c1430 */

int ServerClassInit<DT_NPC_HoverTurret::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&DT_Corpse::g_SendTable.m_pNetTableName + unaff_EBX) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf64a4);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)(&DAT_00cf64bc + unaff_EBX),
                  (char *)(vgui::ListViewItem::ChainToAnimationMap + unaff_EBX),0,4,-1,0,
                  (SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&ice_falling_damage_scale.m_fnChangeCallbacks.m_Size + unaff_EBX),
                        (char *)(unaff_EBX + 0x8f3950),0,
                        (SendTable *)**(undefined4 **)(&DAT_00aeb1fc + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00aea7e8 + unaff_EBX),0x80);
      SendPropInt((SendProp_conflict *)
                  ((int)&sv_gravity.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX),
                  (char *)(unaff_EBX + 0x94900f),0xf0c,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropInt((SendProp_conflict *)((int)&sv_stopspeed.m_fMinVal + unaff_EBX),
                  (char *)(unaff_EBX + 0x948a20),0xf10,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      ___cxa_guard_release(unaff_EBX + 0xcf64a4);
      ___cxa_atexit(&UNK_008d8e2c + unaff_EBX,0,*(undefined4 *)(&DAT_00aea74c + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)((int)&PTR_MarkForDeletion_00c6d70c + unaff_EBX),
             (SendProp_conflict *)
             ((int)&ice_falling_damage_scale.m_fnChangeCallbacks.m_Size + unaff_EBX),3,
             *(char **)(&DAT_00b722c8 + unaff_EBX));
  return 1;
}


/* CNPC_HoverTurret::GetServerClass at 006be8f0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: ServerClass * GetServerClass(CNPC_HoverTurret * this) */

ServerClass * __thiscall CNPC_HoverTurret::GetServerClass(CNPC_HoverTurret *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x6f8fc4);
}


/* CNPC_HoverTurret::YouForgotToImplementOrDeclareServerClass at 006be900 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CNPC_HoverTurret * this)
    */

int __thiscall CNPC_HoverTurret::YouForgotToImplementOrDeclareServerClass(CNPC_HoverTurret *this)

{
  return 0;
}


/* __tcf_5 at 0099a2d0 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x394878),in_stack_00000008);
  return;
}


/* __tcf_2 at 0099a270 */

void __tcf_2(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d77b))(unaff_EBX + 0x41d77b);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d727))(unaff_EBX + 0x41d727);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d6d3))(unaff_EBX + 0x41d6d3);
                    /* WARNING: Could not recover jumptable at 0x0099a2c8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d67f))();
  return;
}


/* CNPC_HoverTurret::CNPC_HoverTurret at 006c5220 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void CNPC_HoverTurret(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::CNPC_HoverTurret(CNPC_HoverTurret *this)

{
  uint *puVar1;
  int *piVar2;
  CBaseEdict *pCVar3;
  IChangeInfoAccessor *pIVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CAI_BasePhysicsFlyingBot::CAI_BasePhysicsFlyingBot((CAI_BasePhysicsFlyingBot *)this);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).m_OnAlyxStartedInteraction.
  super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).m_OnAlyxStartedInteraction.
  super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index = 0xffffffff;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).m_OnAlyxStartedInteraction.
  super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).m_OnAlyxFinishedInteraction.
  super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).m_OnAlyxFinishedInteraction.
  super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index = 0xffffffff;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).m_OnAlyxFinishedInteraction.
  super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  *(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating = unaff_EBX + 0x56d5ba;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_CAI_DefMovementSink.super_IAI_MovementSink._vptr_IAI_MovementSink =
       (_func_int_varargs **)(unaff_EBX + 0x56dfbe);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_IAI_BehaviorBridge._vptr_IAI_BehaviorBridge =
       (_func_int_varargs **)(unaff_EBX + 0x56dfea);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_IMotionEvent._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x56e096);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_INPCInteractive.
  _vptr_INPCInteractive = (_func_int_varargs **)(unaff_EBX + 0x56e0a2);
  (this->super_CDefaultPlayerPickupVPhysics).super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics
       = (_func_int_varargs **)(unaff_EBX + 0x56e0be);
  CAI_BaseNPC::LoadSchedules();
  piVar2 = *(int **)(unaff_EBX + 0x4e6e72);
  if (*(int *)(unaff_EBX + 0x62842a) != *piVar2) {
    InitCustomSchedules();
    *(undefined1 *)(unaff_EBX + 0x628426) = 1;
    *(int *)(unaff_EBX + 0x62842a) = *piVar2;
  }
  (this->m_hAttachTarget).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hTether).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_OnPhysGunPickup).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnPhysGunPickup).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->m_OnPhysGunPickup).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_OnPhysGunDrop).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnPhysGunDrop).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->m_OnPhysGunDrop).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_hSmokeTrail).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hEyeGlow).super_CBaseHandle.m_Index = 0xffffffff;
  this->m_bUseCarryAngles = true;
  if ((this->m_iLaserState).m_Value != 1) {
    if (*(bool *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x60) ==
        false) {
      pCVar3 = *(CBaseEdict **)
                ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                       super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                       super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x24);
      if (pCVar3 != (CBaseEdict *)0x0) {
        pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
        pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
        pIVar4->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = (uint *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                              super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay
                              .super_CBaseAnimating + 100);
      *puVar1 = *puVar1 | 1;
    }
    (this->m_iLaserState).m_Value = 1;
  }
  if ((this->m_sLaserHaloSprite).m_Value != 0) {
    if (*(bool *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x60) ==
        false) {
      pCVar3 = *(CBaseEdict **)
                ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                       super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                       super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x24);
      if (pCVar3 != (CBaseEdict *)0x0) {
        pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
        pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
        pIVar4->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = (uint *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                              super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay
                              .super_CBaseAnimating + 100);
      *puVar1 = *puVar1 | 1;
    }
    (this->m_sLaserHaloSprite).m_Value = 0;
  }
  (this->m_hAttachTarget).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hTether).super_CBaseHandle.m_Index = 0xffffffff;
  return;
}


/* CNPC_HoverTurret::CNPC_HoverTurret at 006c54d0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void CNPC_HoverTurret(CNPC_HoverTurret * this, CNPC_HoverTurret * this)
    */

void __thiscall CNPC_HoverTurret::CNPC_HoverTurret(CNPC_HoverTurret *this,CNPC_HoverTurret *this_1)

{
  CNPC_HoverTurret(this);
  return;
}


/* CEntityFactory<CNPC_HoverTurret>::Create at 006c5eb0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CNPC_HoverTurret> * this,
   char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CNPC_HoverTurret>::Create(CEntityFactory<CNPC_HoverTurret> *this,char *pClassName)

{
  CNPC_HoverTurret *this_00;
  
                    /* Unresolved local var: CNPC_HoverTurret * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0xfec);
  CNPC_HoverTurret::CNPC_HoverTurret(this_00);
  (**(code **)(*(int *)&(this_00->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x74))
            (this_00,pClassName);
  return &((CServerNetworkProperty *)
          ((int)&(this_00->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                 super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                 super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x18))->
          super_IServerNetworkable;
}


/* CNPC_HoverTurret::~CNPC_HoverTurret at 006c54e0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void ~CNPC_HoverTurret(CNPC_HoverTurret * this, int __in_chrg) */

void __thiscall CNPC_HoverTurret::~CNPC_HoverTurret(CNPC_HoverTurret *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  *(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating = unaff_EBX + 0x56d2fa;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_CAI_DefMovementSink.super_IAI_MovementSink._vptr_IAI_MovementSink =
       (_func_int_varargs **)(&UNK_0056dcfe + unaff_EBX);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_IAI_BehaviorBridge._vptr_IAI_BehaviorBridge =
       (_func_int_varargs **)(unaff_EBX + 0x56dd2a);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_IMotionEvent._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x56ddd6);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_INPCInteractive.
  _vptr_INPCInteractive = (_func_int_varargs **)(unaff_EBX + 0x56dde2);
  (this->super_CDefaultPlayerPickupVPhysics).super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics
       = (_func_int_varargs **)(unaff_EBX + 0x56ddfe);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnPhysGunDrop).super_CBaseEntityOutput,in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnPhysGunPickup).super_CBaseEntityOutput,in_stack_ffffffd8);
  *(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating = unaff_EBX + 0x56e2ba;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_CAI_DefMovementSink.super_IAI_MovementSink._vptr_IAI_MovementSink =
       (_func_int_varargs **)(unaff_EBX + 0x56ecae);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_IAI_BehaviorBridge._vptr_IAI_BehaviorBridge =
       (_func_int_varargs **)(unaff_EBX + 0x56ecda);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_IMotionEvent._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x56ed86);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_INPCInteractive.
  _vptr_INPCInteractive = (_func_int_varargs **)(unaff_EBX + 0x56ed92);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
              m_OnAlyxFinishedInteraction.super_CBaseEntityOutput,in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).m_OnAlyxStartedInteraction
              .super_CBaseEntityOutput,in_stack_ffffffd8);
  CAI_BasePhysicsFlyingBot::~CAI_BasePhysicsFlyingBot((CAI_BasePhysicsFlyingBot *)this,__in_chrg);
  return;
}


/* CNPC_HoverTurret::~CNPC_HoverTurret at 006c56a0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void ~CNPC_HoverTurret(CNPC_HoverTurret * this, int __in_chrg) */

void __thiscall CNPC_HoverTurret::~CNPC_HoverTurret(CNPC_HoverTurret *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  *(undefined **)
   &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
    super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
    super_CBaseAnimating = &UNK_0056d13a + unaff_EBX;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_CAI_DefMovementSink.super_IAI_MovementSink._vptr_IAI_MovementSink =
       (_func_int_varargs **)(unaff_EBX + 0x56db3e);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_IAI_BehaviorBridge._vptr_IAI_BehaviorBridge =
       (_func_int_varargs **)(unaff_EBX + 0x56db6a);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_IMotionEvent._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x56dc16);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_INPCInteractive.
  _vptr_INPCInteractive = (_func_int_varargs **)(unaff_EBX + 0x56dc22);
  (this->super_CDefaultPlayerPickupVPhysics).super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics
       = (_func_int_varargs **)(unaff_EBX + 0x56dc3e);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnPhysGunDrop).super_CBaseEntityOutput,in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnPhysGunPickup).super_CBaseEntityOutput,in_stack_ffffffd8);
  *(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating = unaff_EBX + 0x56e0fa;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_CAI_DefMovementSink.super_IAI_MovementSink._vptr_IAI_MovementSink =
       (_func_int_varargs **)(unaff_EBX + 0x56eaee);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_IAI_BehaviorBridge._vptr_IAI_BehaviorBridge =
       (_func_int_varargs **)(unaff_EBX + 0x56eb1a);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_IMotionEvent._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x56ebc6);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_INPCInteractive.
  _vptr_INPCInteractive = (_func_int_varargs **)(unaff_EBX + 0x56ebd2);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
              m_OnAlyxFinishedInteraction.super_CBaseEntityOutput,in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).m_OnAlyxStartedInteraction
              .super_CBaseEntityOutput,in_stack_ffffffd8);
  CAI_BasePhysicsFlyingBot::~CAI_BasePhysicsFlyingBot
            ((CAI_BasePhysicsFlyingBot *)this,in_stack_ffffffd8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CNPC_HoverTurret::~CNPC_HoverTurret at 006c5860 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void ~CNPC_HoverTurret(CNPC_HoverTurret * this, int __in_chrg) */

void __thiscall CNPC_HoverTurret::~CNPC_HoverTurret(CNPC_HoverTurret *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  *(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating = unaff_EBX + 0x56cf7a;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_CAI_DefMovementSink.super_IAI_MovementSink._vptr_IAI_MovementSink =
       (_func_int_varargs **)(&UNK_0056d97e + unaff_EBX);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_IAI_BehaviorBridge._vptr_IAI_BehaviorBridge =
       (_func_int_varargs **)(unaff_EBX + 0x56d9aa);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_IMotionEvent._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x56da56);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_INPCInteractive.
  _vptr_INPCInteractive = (_func_int_varargs **)(unaff_EBX + 0x56da62);
  (this->super_CDefaultPlayerPickupVPhysics).super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics
       = (_func_int_varargs **)(unaff_EBX + 0x56da7e);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnPhysGunDrop).super_CBaseEntityOutput,in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnPhysGunPickup).super_CBaseEntityOutput,in_stack_ffffffd8);
  *(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating = unaff_EBX + 0x56df3a;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_CAI_DefMovementSink.super_IAI_MovementSink._vptr_IAI_MovementSink =
       (_func_int_varargs **)(&UNK_0056e92e + unaff_EBX);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_IAI_BehaviorBridge._vptr_IAI_BehaviorBridge =
       (_func_int_varargs **)(unaff_EBX + 0x56e95a);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_IMotionEvent._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x56ea06);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_INPCInteractive.
  _vptr_INPCInteractive = (_func_int_varargs **)(unaff_EBX + 0x56ea12);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
              m_OnAlyxFinishedInteraction.super_CBaseEntityOutput,in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).m_OnAlyxStartedInteraction
              .super_CBaseEntityOutput,in_stack_ffffffd8);
  CAI_BasePhysicsFlyingBot::~CAI_BasePhysicsFlyingBot((CAI_BasePhysicsFlyingBot *)this,__in_chrg);
  return;
}


/* CNPC_HoverTurret::GatherConditions at 006bf410 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void GatherConditions(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::GatherConditions(CNPC_HoverTurret *this)

{
  CAI_BaseNPC::GatherConditions((CAI_BaseNPC *)this);
  return;
}


/* CNPC_HoverTurret::PrescheduleThink at 006c5130 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void PrescheduleThink(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::PrescheduleThink(CNPC_HoverTurret *this)

{
  int iVar1;
  int iVar2;
  hoverTurretAttackState_e hVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((*(byte *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                       super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                       super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x133) & 8
      ) == 0) {
    if (0 < ((CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth> *)
            ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                   super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                   super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0xe8))->
            m_Value) {
      hVar3 = this->m_iFiringState;
      if (hVar3 == HOVER_TURRET_AIM_TARGET) {
        AimThink(this);
      }
      else {
        if (hVar3 == HOVER_TURRET_SHOT_DISABLED) {
          return;
        }
        if (hVar3 == HOVER_TURRET_FIND_TARGET) {
          FindTargetThink(this);
        }
      }
      goto LAB_006c5179;
    }
    if ((this->m_flDeathTime == *(float *)(unaff_EBX + 0x36542f)) ||
       (*(float *)(**(int **)(unaff_EBX + 0x4e6a5f) + 0xc) <= this->m_flDeathTime))
    goto LAB_006c5179;
  }
  else {
    iVar2 = ((CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth> *)
            ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                   super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                   super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0xe8))->
            m_Value;
    iVar1 = iVar2 + -1;
    if (iVar2 != iVar1) {
      (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                            super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                            super_CBaseAnimating + 0x200))
                (this,(CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth> *)
                      ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                             super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                             super_CBaseAnimating + 0xe8));
      ((CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth> *)
      ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0xe8))->m_Value =
           iVar1;
    }
    if (0 < iVar1) goto LAB_006c5179;
  }
  Explode(this);
LAB_006c5179:
  TalkThink(this);
  CAI_BaseNPC::PrescheduleThink((CAI_BaseNPC *)this);
  return;
}


/* CNPC_HoverTurret::SetFiringState at 006c32f0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void SetFiringState(CNPC_HoverTurret * this, hoverTurretAttackState_e
   state) */

void __thiscall
CNPC_HoverTurret::SetFiringState(CNPC_HoverTurret *this,hoverTurretAttackState_e state)

{
  undefined1 *puVar1;
  uint uVar2;
  CBaseEdict *pCVar3;
  _func_int_varargs **pp_Var4;
  char cVar5;
  int iVar6;
  CSprite *pCVar7;
  uint *puVar8;
  IChangeInfoAccessor *pIVar9;
  undefined4 *puVar10;
  int unaff_EBX;
  CBaseEntity *this_00;
  bool bVar11;
  Vector local_28 [2];
  
                    /* Unresolved local var: bool bNewState@[???] */
  ___i686_get_pc_thunk_bx();
  cVar5 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                super_CBaseCombatCharacter.super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x114))(this);
  if ((cVar5 != '\0') || (state == HOVER_TURRET_SHOT_DISABLED)) {
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
    uVar2 = (this->m_hEyeGlow).super_CBaseHandle.m_Index;
    if ((uVar2 == 0xffffffff) ||
       ((iVar6 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e88a6),
        *(uint *)(iVar6 + 8) != uVar2 >> 0x10 || (*(int *)(iVar6 + 4) == 0)))) {
      pCVar7 = CSprite::SpriteCreate
                         ((char *)(unaff_EBX + 0x345a8a),
                          (Vector *)
                          &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                           super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                           super_CBaseAnimating.super_CBaseEntity.m_vecOrigin,false);
      if (pCVar7 == (CSprite *)0x0) {
        (this->m_hEyeGlow).super_CBaseHandle.m_Index = 0xffffffff;
        return;
      }
      puVar8 = (uint *)(*(pCVar7->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.
                         super_IHandleEntity._vptr_IHandleEntity[3])(pCVar7);
      uVar2 = *puVar8;
      (this->m_hEyeGlow).super_CBaseHandle.m_Index = uVar2;
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
      if (uVar2 == 0xffffffff) {
        return;
      }
      iVar6 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e88a6);
      if (*(uint *)(iVar6 + 8) != uVar2 >> 0x10) {
        return;
      }
      pCVar7 = *(CSprite **)(iVar6 + 4);
      if (pCVar7 == (CSprite *)0x0) {
        return;
      }
      if ((pCVar7->super_CBaseEntity).m_nRenderMode.m_Value != '\t') {
        if ((pCVar7->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
          pCVar3 = &((pCVar7->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
          if (pCVar3 != (CBaseEdict *)0x0) {
            pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
            pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
            pIVar9->m_iChangeInfoSerialNumber = 0;
          }
        }
        else {
          puVar1 = &(pCVar7->super_CBaseEntity).m_Network.field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (pCVar7->super_CBaseEntity).m_nRenderMode.m_Value = '\t';
      }
      if ((pCVar7->super_CBaseEntity).m_clrRender.
          super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r != 0xff) {
        if ((pCVar7->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
          pCVar3 = &((pCVar7->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
          if (pCVar3 != (CBaseEdict *)0x0) {
            pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
            pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
            pIVar9->m_iChangeInfoSerialNumber = 0;
          }
        }
        else {
          puVar1 = &(pCVar7->super_CBaseEntity).m_Network.field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (pCVar7->super_CBaseEntity).m_clrRender.
        super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r = 0xff;
      }
      if ((pCVar7->super_CBaseEntity).m_clrRender.
          super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g != '\0') {
        if ((pCVar7->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
          pCVar3 = &((pCVar7->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
          if (pCVar3 != (CBaseEdict *)0x0) {
            pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
            pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
            pIVar9->m_iChangeInfoSerialNumber = 0;
          }
        }
        else {
          puVar1 = &(pCVar7->super_CBaseEntity).m_Network.field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (pCVar7->super_CBaseEntity).m_clrRender.
        super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g = '\0';
      }
      if ((pCVar7->super_CBaseEntity).m_clrRender.
          super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b != '\0') {
        if ((pCVar7->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
          pCVar3 = &((pCVar7->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
          if (pCVar3 != (CBaseEdict *)0x0) {
            pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
            pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
            pIVar9->m_iChangeInfoSerialNumber = 0;
          }
        }
        else {
          puVar1 = &(pCVar7->super_CBaseEntity).m_Network.field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (pCVar7->super_CBaseEntity).m_clrRender.
        super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b = '\0';
      }
      CSprite::SetBrightness(pCVar7,0x80,0.0);
      if ((pCVar7->super_CBaseEntity).m_nRenderFX.m_Value != '\x0e') {
        if ((pCVar7->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
          pCVar3 = &((pCVar7->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
          if (pCVar3 != (CBaseEdict *)0x0) {
            pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
            pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
            pIVar9->m_iChangeInfoSerialNumber = 0;
          }
        }
        else {
          puVar1 = &(pCVar7->super_CBaseEntity).m_Network.field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (pCVar7->super_CBaseEntity).m_nRenderFX.m_Value = '\x0e';
      }
      uVar2 = (this->m_hEyeGlow).super_CBaseHandle.m_Index;
      if ((uVar2 == 0xffffffff) ||
         (iVar6 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e88a6),
         *(uint *)(iVar6 + 8) != uVar2 >> 0x10)) {
        this_00 = (CBaseEntity *)0x0;
      }
      else {
        this_00 = *(CBaseEntity **)(iVar6 + 4);
      }
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???]
                       Unresolved local var: CEntInfo * pInfo@[???] */
      pp_Var4 = this_00[1].super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
                _vptr_IHandleEntity;
      if (((pp_Var4 == (_func_int_varargs **)0xffffffff) ||
          (iVar6 = ((uint)pp_Var4 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e88a6),
          *(uint *)(iVar6 + 8) != (uint)pp_Var4 >> 0x10)) ||
         (this != *(CNPC_HoverTurret **)(iVar6 + 4))) {
        if ((this_00->m_Network).m_TimerEvent.m_bRegistered == false) {
          pCVar3 = &((this_00->m_Network).m_pPev)->super_CBaseEdict;
          if (pCVar3 != (CBaseEdict *)0x0) {
            pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
            pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
            pIVar9->m_iChangeInfoSerialNumber = 0;
          }
        }
        else {
          puVar1 = &(this_00->m_Network).field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        puVar10 = (undefined4 *)
                  (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                        super_CBaseCombatCharacter.super_CBaseFlex.
                                        super_CBaseAnimatingOverlay.super_CBaseAnimating + 0xc))
                            (this);
        this_00[1].super_IServerEntity.super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
             = (_func_int_varargs **)*puVar10;
      }
      if (this_00[1].m_iObjectCapsCache.m_Value != 1) {
        if ((this_00->m_Network).m_TimerEvent.m_bRegistered == false) {
          pCVar3 = &((this_00->m_Network).m_pPev)->super_CBaseEdict;
          if (pCVar3 != (CBaseEdict *)0x0) {
            pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
            pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
            pIVar9->m_iChangeInfoSerialNumber = 0;
          }
        }
        else {
          puVar1 = &(this_00->m_Network).field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        this_00[1].m_iObjectCapsCache.m_Value = 1;
      }
      CBaseEntity::FollowEntity(this_00,(CBaseEntity *)this,true);
    }
    bVar11 = this->m_iFiringState != state;
    if (state != HOVER_TURRET_SHOT_DISABLED) {
      if (state == HOVER_TURRET_HIT_WITH_PHYSICS) {
        this->m_iFiringState = HOVER_TURRET_FIND_TARGET;
        this->m_iDesiredState = 7;
        (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                              super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay
                              .super_CBaseAnimating + 0x684))
                  (this,*(undefined4 *)(unaff_EBX + 0x66b842));
        if ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
            super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.m_nSkin.m_Value != 2) {
          if ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
              super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
              m_Network.m_TimerEvent.m_bRegistered == false) {
            pCVar3 = &((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                       super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                       super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                       super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
            if (pCVar3 != (CBaseEdict *)0x0) {
              pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
              pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
              pIVar9->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            puVar1 = &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                      super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                      super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                      super_CBaseEntity.m_Network.field_0x4c;
            *(uint *)puVar1 = *(uint *)puVar1 | 1;
          }
          (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
          super_CBaseAnimating.m_nSkin.m_Value = 2;
        }
        if ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
            super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
            m_Network.m_TimerEvent.m_bRegistered == false) {
          pCVar3 = &((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                     super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                     super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                     super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
          if (pCVar3 != (CBaseEdict *)0x0) {
            pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
            pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
            pIVar9->m_iChangeInfoSerialNumber = 0;
          }
        }
        else {
          puVar1 = &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                    super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                    super_CBaseEntity.m_Network.field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (this->m_iLaserState).m_Value = 0;
        uVar2 = (this->m_hEyeGlow).super_CBaseHandle.m_Index;
        if ((uVar2 == 0xffffffff) ||
           (iVar6 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e88a6),
           *(uint *)(iVar6 + 8) != uVar2 >> 0x10)) {
          iVar6 = 0;
        }
        else {
          iVar6 = *(int *)(iVar6 + 4);
        }
        if (*(char *)(iVar6 + 0x108) != -1) {
          if (*(char *)(iVar6 + 0x60) == '\0') {
            pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
            if (pCVar3 != (CBaseEdict *)0x0) {
              pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
              pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
              pIVar9->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
          }
          *(undefined1 *)(iVar6 + 0x108) = 0xff;
        }
        if (*(char *)(iVar6 + 0x109) != '\0') {
          if (*(char *)(iVar6 + 0x60) == '\0') {
            pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
            if (pCVar3 != (CBaseEdict *)0x0) {
              pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
              pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
              pIVar9->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
          }
          *(undefined1 *)(iVar6 + 0x109) = 0;
        }
        if (*(char *)(iVar6 + 0x10a) != '\0') {
          if (*(char *)(iVar6 + 0x60) == '\0') {
            pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
            if (pCVar3 != (CBaseEdict *)0x0) {
              pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
              pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
              pIVar9->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
          }
          *(undefined1 *)(iVar6 + 0x10a) = 0;
        }
        uVar2 = (this->m_hEyeGlow).super_CBaseHandle.m_Index;
        if ((uVar2 == 0xffffffff) ||
           (iVar6 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e88a6),
           *(uint *)(iVar6 + 8) != uVar2 >> 0x10)) {
          pCVar7 = (CSprite *)0x0;
        }
        else {
          pCVar7 = *(CSprite **)(iVar6 + 4);
        }
        iVar6 = 0x80;
      }
      else if (state == HOVER_TURRET_AIM_TARGET) {
        this->m_iDesiredState = 0;
        this->m_bAimingAtTarget = true;
        GetClosestVisibleEnemyPosition(local_28,this);
        (this->m_vecTargetPos).x = local_28[0].x;
        (this->m_vecTargetPos).y = local_28[0].y;
        (this->m_vecTargetPos).z = local_28[0].z;
        this->m_iFiringState = HOVER_TURRET_AIM_TARGET;
        this->m_flAimStartTime = *(float *)(**(int **)(unaff_EBX + 0x4e88a2) + 0xc);
        if ((bVar11) &&
           ((**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                  super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                  super_CBaseCombatCharacter.super_CBaseFlex.
                                  super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x684))
                      (this,*(undefined4 *)(unaff_EBX + 0x66b83e)),
           (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.m_nSkin.m_Value != 1)) {
          if ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
              super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
              m_Network.m_TimerEvent.m_bRegistered == false) {
            pCVar3 = &((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                       super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                       super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                       super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
            if (pCVar3 != (CBaseEdict *)0x0) {
              pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
              pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
              pIVar9->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            puVar1 = &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                      super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                      super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                      super_CBaseEntity.m_Network.field_0x4c;
            *(uint *)puVar1 = *(uint *)puVar1 | 1;
          }
          (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
          super_CBaseAnimating.m_nSkin.m_Value = 1;
        }
        uVar2 = (this->m_hEyeGlow).super_CBaseHandle.m_Index;
        if ((uVar2 == 0xffffffff) ||
           (iVar6 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e88a6),
           *(uint *)(iVar6 + 8) != uVar2 >> 0x10)) {
          iVar6 = 0;
        }
        else {
          iVar6 = *(int *)(iVar6 + 4);
        }
        if (*(char *)(iVar6 + 0x108) != -1) {
          if (*(char *)(iVar6 + 0x60) == '\0') {
            pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
            if (pCVar3 != (CBaseEdict *)0x0) {
              pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
              pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
              pIVar9->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
          }
          *(undefined1 *)(iVar6 + 0x108) = 0xff;
        }
        if (*(char *)(iVar6 + 0x109) != -1) {
          if (*(char *)(iVar6 + 0x60) == '\0') {
            pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
            if (pCVar3 != (CBaseEdict *)0x0) {
              pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
              pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
              pIVar9->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
          }
          *(undefined1 *)(iVar6 + 0x109) = 0xff;
        }
        if (*(char *)(iVar6 + 0x10a) != '\0') {
          if (*(char *)(iVar6 + 0x60) == '\0') {
            pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
            if (pCVar3 != (CBaseEdict *)0x0) {
              pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
              pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
              pIVar9->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
          }
          *(undefined1 *)(iVar6 + 0x10a) = 0;
        }
        uVar2 = (this->m_hEyeGlow).super_CBaseHandle.m_Index;
        if ((uVar2 == 0xffffffff) ||
           (iVar6 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e88a6),
           *(uint *)(iVar6 + 8) != uVar2 >> 0x10)) {
          pCVar7 = (CSprite *)0x0;
        }
        else {
          pCVar7 = *(CSprite **)(iVar6 + 4);
        }
        iVar6 = 0xc4;
      }
      else {
        this->m_bAimingAtTarget = false;
        this->m_flAimStartTime = 0.0;
        this->m_iDesiredState = 1;
        this->m_iFiringState = HOVER_TURRET_FIND_TARGET;
        if ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
            super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
            m_Network.m_TimerEvent.m_bRegistered == false) {
          pCVar3 = &((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                     super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                     super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                     super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
          if (pCVar3 != (CBaseEdict *)0x0) {
            pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
            pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
            pIVar9->m_iChangeInfoSerialNumber = 0;
          }
        }
        else {
          puVar1 = &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                    super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                    super_CBaseEntity.m_Network.field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (this->m_iLaserState).m_Value = 1;
        if (bVar11) {
          if ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
              super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.m_nSkin.m_Value != 3)
          {
            if ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                m_Network.m_TimerEvent.m_bRegistered == false) {
              pCVar3 = &((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                         super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter
                         .super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                         super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
              if (pCVar3 != (CBaseEdict *)0x0) {
                pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
                pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
                pIVar9->m_iChangeInfoSerialNumber = 0;
              }
            }
            else {
              puVar1 = &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                        super_CBaseEntity.m_Network.field_0x4c;
              *(uint *)puVar1 = *(uint *)puVar1 | 1;
            }
            (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
            super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.m_nSkin.m_Value = 3;
          }
          (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                super_CBaseCombatCharacter.super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x684))
                    (this,*(undefined4 *)(unaff_EBX + 0x66b83a));
        }
        uVar2 = (this->m_hEyeGlow).super_CBaseHandle.m_Index;
        if ((uVar2 == 0xffffffff) ||
           (iVar6 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e88a6),
           *(uint *)(iVar6 + 8) != uVar2 >> 0x10)) {
          iVar6 = 0;
        }
        else {
          iVar6 = *(int *)(iVar6 + 4);
        }
        if (*(char *)(iVar6 + 0x108) != '\0') {
          if (*(char *)(iVar6 + 0x60) == '\0') {
            pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
            if (pCVar3 != (CBaseEdict *)0x0) {
              pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
              pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
              pIVar9->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
          }
          *(undefined1 *)(iVar6 + 0x108) = 0;
        }
        if (*(char *)(iVar6 + 0x109) != '\0') {
          if (*(char *)(iVar6 + 0x60) == '\0') {
            pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
            if (pCVar3 != (CBaseEdict *)0x0) {
              pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
              pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
              pIVar9->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
          }
          *(undefined1 *)(iVar6 + 0x109) = 0;
        }
        if (*(char *)(iVar6 + 0x10a) != -1) {
          if (*(char *)(iVar6 + 0x60) == '\0') {
            pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
            if (pCVar3 != (CBaseEdict *)0x0) {
              pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
              pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
              pIVar9->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
          }
          *(undefined1 *)(iVar6 + 0x10a) = 0xff;
        }
        uVar2 = (this->m_hEyeGlow).super_CBaseHandle.m_Index;
        if ((uVar2 == 0xffffffff) ||
           (iVar6 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e88a6),
           *(uint *)(iVar6 + 8) != uVar2 >> 0x10)) {
          pCVar7 = (CSprite *)0x0;
        }
        else {
          pCVar7 = *(CSprite **)(iVar6 + 4);
        }
        iVar6 = 0x60;
      }
      CSprite::SetBrightness(pCVar7,iVar6,0.1);
      uVar2 = (this->m_hEyeGlow).super_CBaseHandle.m_Index;
      if ((uVar2 == 0xffffffff) ||
         (iVar6 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e88a6),
         *(uint *)(iVar6 + 8) != uVar2 >> 0x10)) {
        pCVar7 = (CSprite *)0x0;
      }
      else {
        pCVar7 = *(CSprite **)(iVar6 + 4);
      }
      CSprite::SetScale(pCVar7,0.4,0.1);
      return;
    }
    this->m_iFiringState = HOVER_TURRET_SHOT_DISABLED;
    this->m_iDesiredState = 5;
    if (bVar11) {
      (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                            super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                            super_CBaseAnimating + 0x684))
                (this,*(undefined4 *)(unaff_EBX + 0x66b842));
      if ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
          super_CBaseAnimating.m_nSkin.m_Value != 2) {
        if ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
            super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
            m_Network.m_TimerEvent.m_bRegistered == false) {
          pCVar3 = &((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                     super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                     super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                     super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
          if (pCVar3 != (CBaseEdict *)0x0) {
            pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
            pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
            pIVar9->m_iChangeInfoSerialNumber = 0;
          }
        }
        else {
          puVar1 = &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                    super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                    super_CBaseEntity.m_Network.field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
        super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
        super_CBaseAnimating.m_nSkin.m_Value = 2;
      }
      if ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
          super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
        pCVar3 = &((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                   super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                   super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                   super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
        if (pCVar3 != (CBaseEdict *)0x0) {
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
          pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
          pIVar9->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                  super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                  super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity
                  .m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->m_iLaserState).m_Value = 0;
    }
    uVar2 = (this->m_hEyeGlow).super_CBaseHandle.m_Index;
    if ((uVar2 == 0xffffffff) ||
       (iVar6 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e88a6),
       *(uint *)(iVar6 + 8) != uVar2 >> 0x10)) {
      iVar6 = 0;
    }
    else {
      iVar6 = *(int *)(iVar6 + 4);
    }
    if (*(char *)(iVar6 + 0x108) != -1) {
      if (*(char *)(iVar6 + 0x60) == '\0') {
        pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
        if (pCVar3 != (CBaseEdict *)0x0) {
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
          pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
          pIVar9->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
      }
      *(undefined1 *)(iVar6 + 0x108) = 0xff;
    }
    if (*(char *)(iVar6 + 0x109) != '\0') {
      if (*(char *)(iVar6 + 0x60) == '\0') {
        pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
        if (pCVar3 != (CBaseEdict *)0x0) {
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
          pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
          pIVar9->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
      }
      *(undefined1 *)(iVar6 + 0x109) = 0;
    }
    if (*(char *)(iVar6 + 0x10a) != -1) {
      if (*(char *)(iVar6 + 0x60) == '\0') {
        pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
        if (pCVar3 != (CBaseEdict *)0x0) {
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
          pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
          pIVar9->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
      }
      *(undefined1 *)(iVar6 + 0x10a) = 0xff;
    }
    uVar2 = (this->m_hEyeGlow).super_CBaseHandle.m_Index;
    if ((uVar2 == 0xffffffff) ||
       (iVar6 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e88a6),
       *(uint *)(iVar6 + 8) != uVar2 >> 0x10)) {
      pCVar7 = (CSprite *)0x0;
    }
    else {
      pCVar7 = *(CSprite **)(iVar6 + 4);
    }
    CSprite::SetScale(pCVar7,0.1,3.0);
    uVar2 = (this->m_hEyeGlow).super_CBaseHandle.m_Index;
    if ((uVar2 == 0xffffffff) ||
       (iVar6 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e88a6),
       *(uint *)(iVar6 + 8) != uVar2 >> 0x10)) {
      pCVar7 = (CSprite *)0x0;
    }
    else {
      pCVar7 = *(CSprite **)(iVar6 + 4);
    }
    CSprite::SetBrightness(pCVar7,0,3.0);
  }
  return;
}


/* CNPC_HoverTurret::FindTargetThink at 006c4f20 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void FindTargetThink(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::FindTargetThink(CNPC_HoverTurret *this)

{
  float *pfVar1;
  uint *puVar2;
  CBaseEdict *this_00;
  bool bVar3;
  char cVar4;
  char cVar5;
  char cVar6;
  CAI_Enemies *this_01;
  CBaseEntity *pEnemy;
  IChangeInfoAccessor *pIVar7;
  int *piVar8;
  int iVar9;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  pfVar1 = (float *)(**(int **)(unaff_EBX + 0x4e6c73) + 0xc);
  if ((this->m_flEngineStallTime < *pfVar1 || this->m_flEngineStallTime == *pfVar1) &&
     (this->m_iFiringState != HOVER_TURRET_SHOT_DISABLED)) {
    this_01 = (CAI_Enemies *)
              (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                    super_CBaseCombatCharacter.super_CBaseFlex.
                                    super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x860))(this)
    ;
    CAI_Enemies::RefreshMemories(this_01);
    CAI_Senses::Look((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                     super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.m_pSenses,0x2000);
    pEnemy = (CBaseEntity *)
             (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                   super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                   super_CBaseCombatCharacter.super_CBaseFlex.
                                   super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x6cc))(this);
    CAI_BaseNPC::SetEnemy((CAI_BaseNPC *)this,pEnemy,true);
    if ((this->m_iLaserState).m_Value == 0) {
      if (*(bool *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x60) == false) {
        this_00 = *(CBaseEdict **)
                   ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x24);
        if (this_00 != (CBaseEdict *)0x0) {
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
          pIVar7 = CBaseEdict::GetChangeAccessor(this_00);
          pIVar7->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar2 = (uint *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                super_CBaseCombatCharacter.super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating + 100);
        *puVar2 = *puVar2 | 1;
      }
      (this->m_iLaserState).m_Value = 1;
    }
    bVar3 = CAI_BaseNPC::HasCondition((CAI_BaseNPC *)this,10);
    if (bVar3) {
      piVar8 = (int *)(**(code **)(*(int *)&(this->
                                            super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                            super_CBaseCombatCharacter.super_CBaseFlex.
                                            super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x188
                                  ))(this);
      if ((piVar8 != (int *)0x0) && (cVar4 = (**(code **)(*piVar8 + 0x114))(piVar8), cVar4 != '\0'))
      {
        cVar4 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                      super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                      super_CBaseCombatCharacter.super_CBaseFlex.
                                      super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x3d8))
                          (this,piVar8);
        cVar5 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                      super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                      super_CBaseCombatCharacter.super_CBaseFlex.
                                      super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x250))
                          (this,piVar8,0x4041,0);
        iVar9 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                      super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                      super_CBaseCombatCharacter.super_CBaseFlex.
                                      super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x3e0))
                          (this,piVar8);
        if ((iVar9 == 0) ||
           (cVar6 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>
                                          ).super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                          super_CBaseCombatCharacter.super_CBaseFlex.
                                          super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x3d4))
                              (this,iVar9,piVar8,0x4041,0), cVar6 == '\0')) {
          bVar3 = false;
        }
        else {
          bVar3 = true;
        }
        if (((cVar4 != '\0') && (cVar5 != '\0')) || (bVar3)) {
          SetFiringState(this,HOVER_TURRET_AIM_TARGET);
        }
      }
    }
    else {
      if ((*(byte *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                           super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                           super_CBaseAnimating + 0x12d) & 8) != 0) {
        CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
      }
      (this->m_vecGoalAngles).x =
           ((QAngle *)
           ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                  super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                  super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x330))->x;
      (this->m_vecGoalAngles).y =
           *(vec_t *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                            super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                            super_CBaseAnimating + 0x334);
      (this->m_vecGoalAngles).z =
           *(vec_t *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                            super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                            super_CBaseAnimating + 0x338);
      (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
      super_CAI_BaseNPC.super_CBaseCombatCharacter.m_flFieldOfView = 0.4;
    }
    UpdateFacing(this);
    return;
  }
  return;
}


/* CNPC_HoverTurret::AimThink at 006c4bb0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void AimThink(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::AimThink(CNPC_HoverTurret *this)

{
  float *pfVar1;
  uint *puVar2;
  CBaseEdict *this_00;
  bool bVar3;
  char cVar4;
  char cVar5;
  char cVar6;
  int iVar7;
  int *piVar8;
  IChangeInfoAccessor *pIVar9;
  int unaff_EBX;
  float fVar10;
  Vector local_4c;
  Vector local_40;
  float local_34;
  float local_30;
  float local_2c;
  Vector local_28 [2];
  
                    /* Unresolved local var: bool bAbortAim@[???] */
  ___i686_get_pc_thunk_bx();
  bVar3 = CAI_BaseNPC::HasCondition((CAI_BaseNPC *)this,10);
  if (((!bVar3) ||
      (pfVar1 = (float *)(**(int **)(unaff_EBX + 0x4e6fd9) + 0xc),
      *pfVar1 <= this->m_flEngineStallTime && this->m_flEngineStallTime != *pfVar1)) ||
     (this->m_iFiringState == HOVER_TURRET_SHOT_DISABLED)) {
LAB_006c4c00:
    if ((*(byte *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                         super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter
                         .super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x12d)
        & 8) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
    (this->m_vecGoalAngles).x =
         ((QAngle *)
         ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x330))->x;
    (this->m_vecGoalAngles).y =
         *(vec_t *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x334);
    (this->m_vecGoalAngles).z =
         *(vec_t *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x338);
    UpdateFacing(this);
    if (this->m_bAimingAtTarget == false) {
      return;
    }
  }
  else {
    iVar7 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                  super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                  super_CBaseCombatCharacter.super_CBaseFlex.
                                  super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x188))(this);
    if (iVar7 != 0) {
                    /* Unresolved local var: Vector vecEnemyPosition@[???]
                       Unresolved local var: Vector vecEnemyMovement@[???] */
      GetClosestVisibleEnemyPosition(local_28,this);
                    /* Unresolved local var: Vector res@[???] */
      local_34 = local_28[0].x - (this->m_vecTargetPos).x;
      local_30 = local_28[0].y - (this->m_vecTargetPos).y;
      local_2c = local_28[0].z - (this->m_vecTargetPos).z;
      (this->m_vecTargetPos).x = local_28[0].x;
      (this->m_vecTargetPos).y = local_28[0].y;
      (this->m_vecTargetPos).z = local_28[0].z;
                    /* Unresolved local var: __m128 root@[???] */
      fVar10 = SQRT(local_34 * local_34 + local_30 * local_30 + local_2c * local_2c);
      if (*(float *)(unaff_EBX + 0x365a5d) <= fVar10 && fVar10 != *(float *)(unaff_EBX + 0x365a5d))
      goto LAB_006c4c00;
    }
    UpdateFacing(this);
    if ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
        super_CAI_BaseNPC.m_Activity != *(__178 *)(unaff_EBX + 0x669f75)) {
      (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                            super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                            super_CBaseAnimating + 0x684))(this,*(__178 *)(unaff_EBX + 0x669f75));
    }
    if (*(float *)(**(int **)(unaff_EBX + 0x4e6fd9) + 0xc) <=
        *(float *)(unaff_EBX + 0x365a4d) + this->m_flAimStartTime) {
      return;
    }
    piVar8 = (int *)(**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>
                                          ).super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                          super_CBaseCombatCharacter.super_CBaseFlex.
                                          super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x188))
                              (this);
    if (piVar8 == (int *)0x0) {
      return;
    }
    cVar4 = (**(code **)(*piVar8 + 0x114))(piVar8);
    if (cVar4 == '\0') {
      return;
    }
    cVar4 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                  super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                  super_CBaseCombatCharacter.super_CBaseFlex.
                                  super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x3d8))
                      (this,piVar8);
    cVar5 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                  super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                  super_CBaseCombatCharacter.super_CBaseFlex.
                                  super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x250))
                      (this,piVar8,0x4041,0);
    iVar7 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                  super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                  super_CBaseCombatCharacter.super_CBaseFlex.
                                  super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x3e0))
                      (this,piVar8);
    if ((iVar7 == 0) ||
       (cVar6 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                      super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                      super_CBaseCombatCharacter.super_CBaseFlex.
                                      super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x3d4))
                          (this,iVar7,piVar8,0x4041,0), cVar6 == '\0')) {
      bVar3 = false;
    }
    else {
      bVar3 = true;
    }
    if (((cVar4 != '\0') && (cVar5 != '\0')) || (bVar3)) {
      this->m_flAimStartTime = this->m_flAimStartTime - *(float *)(&DAT_00366c7d + unaff_EBX);
      iVar7 = *(int *)(**(int **)(unaff_EBX + 0x4e6fd9) + 0x18);
      if (iVar7 != this->m_muzzleToWorldTick) {
        this->m_muzzleToWorldTick = iVar7;
        (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                              super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay
                              .super_CBaseAnimating + 0x364))
                  (this,this->m_iMuzzleAttachment,&this->m_muzzleToWorld);
      }
      MatrixGetColumn(&this->m_muzzleToWorld,3,&local_40);
      MatrixGetColumn(&this->m_muzzleToWorld,0,&local_4c);
      ShootLaser(this,&local_40,&local_4c);
      if ((this->m_iLaserState).m_Value == 2) {
        return;
      }
      if (*(bool *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x60) == false) {
        this_00 = *(CBaseEdict **)
                   ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x24);
        if (this_00 != (CBaseEdict *)0x0) {
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
          pIVar9 = CBaseEdict::GetChangeAccessor(this_00);
          pIVar9->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar2 = (uint *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                super_CBaseCombatCharacter.super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating + 100);
        *puVar2 = *puVar2 | 1;
      }
      (this->m_iLaserState).m_Value = 2;
      return;
    }
  }
  SetFiringState(this,HOVER_TURRET_FIND_TARGET);
  return;
}


/* CNPC_HoverTurret::GetClosestVisibleEnemyPosition at 006c1d80 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Restarted to delay deadcode elimination for space: stack */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: Vector GetClosestVisibleEnemyPosition(CNPC_HoverTurret * this) */

Vector * __thiscall
CNPC_HoverTurret::GetClosestVisibleEnemyPosition
          (Vector *__return_storage_ptr__,CNPC_HoverTurret *this)

{
  uint uVar1;
  char *pcVar2;
  char *pcVar3;
  uint uVar4;
  VMatrix matThisToLinked;
  bool bVar5;
  char cVar6;
  char cVar7;
  int *piVar8;
  int iVar9;
  float *pfVar10;
  int iVar11;
  float *pfVar12;
  matrix3x4_t *in2;
  uint uVar13;
  uint uVar14;
  int iVar15;
  int unaff_EBX;
  uint *puVar16;
  undefined4 *puVar17;
  undefined4 *puVar18;
  float fVar19;
  float fVar20;
  CNPC_HoverTurret *pCVar21;
  undefined4 uVar22;
  undefined4 uVar23;
  Vector *out;
  undefined4 uVar24;
  undefined1 in_stack_fffffea4 [36];
  Vector *ptSource;
  Vector *ptTransformed;
  CBaseEntity *pEnemy;
  QAngle local_b8;
  float local_ac;
  float local_a8;
  float local_a4;
  Vector local_a0;
  Vector local_94;
  float local_88;
  float local_84;
  float local_80;
  Vector local_7c [2];
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  float local_4c;
  float local_48;
  float local_44;
  float local_40;
  float local_3c;
  float local_38;
  float local_34;
  float local_30;
  float local_2c;
  Vector local_28 [2];
  
                    /* Unresolved local var: Vector vecMuzzlePos@[???]
                       Unresolved local var: Vector vecMidEnemy@[???]
                       Unresolved local var: Vector vecMid@[???]
                       Unresolved local var: Vector vecBody@[???]
                       Unresolved local var: bool bEnemyInFOV@[???]
                       Unresolved local var: bool bEnemyVisible@[???]
                       Unresolved local var: Vector vecDirToEnemy@[???]
                       Unresolved local var: float flDistToEnemy@[???]
                       Unresolved local var: CPortal_Base2D * pPortal@[???]
                       Unresolved local var: QAngle vecAnglesToEnemy@[???] */
  ___i686_get_pc_thunk_bx();
  piVar8 = (int *)(**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                        super_CBaseCombatCharacter.super_CBaseFlex.
                                        super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x188))()
  ;
  iVar11 = *(int *)(**(int **)(unaff_EBX + 0x4e9e0f) + 0x18);
  if (iVar11 != this->m_muzzleToWorldTick) {
    this->m_muzzleToWorldTick = iVar11;
    (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x364))();
  }
  out = local_28;
  MatrixGetColumn(&this->m_muzzleToWorld,3,out);
  uVar24 = 0;
  (**(code **)(*piVar8 + 0x230))();
                    /* Unresolved local var: Vector res@[???] */
  local_38 = *(float *)(unaff_EBX + 0x36892b);
  local_40 = local_34 * local_38;
  local_3c = local_30 * local_38;
  local_38 = local_38 * local_2c;
  pcVar2 = *(char **)(unaff_EBX + 0x6095cf);
  if ((*pcVar2 == '\0') && (iVar11 = ___cxa_guard_acquire(), iVar11 != 0)) {
    ___cxa_guard_release();
  }
  pcVar3 = *(char **)(unaff_EBX + 0x6095d3);
  if ((*pcVar3 == '\0') && (iVar11 = ___cxa_guard_acquire(), iVar11 != 0)) {
    *(undefined4 *)(unaff_EBX + 0x62876f) = 0;
    ___cxa_guard_release();
  }
  do {
    puVar16 = (uint *)(unaff_EBX + 0x62876f);
    uVar4 = *(uint *)(unaff_EBX + 0x62876f);
    uVar13 = uVar4 + 0x10001 & 0x7f;
    LOCK();
    uVar1 = *puVar16;
    if (uVar4 == uVar1) {
      *puVar16 = uVar13;
    }
    UNLOCK();
  } while (uVar4 != uVar1);
  if ((*pcVar2 == '\0') && (iVar11 = ___cxa_guard_acquire(), iVar11 != 0)) {
    ___cxa_guard_release();
  }
  if ((*pcVar3 == '\0') && (iVar11 = ___cxa_guard_acquire(), iVar11 != 0)) {
    *(undefined4 *)(unaff_EBX + 0x62876f) = 0;
    ___cxa_guard_release();
  }
  do {
    uVar4 = *(uint *)(unaff_EBX + 0x62876f);
    uVar14 = uVar4 + 0x10001 & 0x7f;
    LOCK();
    uVar1 = *puVar16;
    if (uVar4 == uVar1) {
      *puVar16 = uVar14;
    }
    UNLOCK();
  } while (uVar4 != uVar1);
  iVar11 = unaff_EBX + 0x627c6f;
  iVar9 = uVar14 * 0xc;
  fVar19 = *(float *)(unaff_EBX + 0x36876f);
  *(float *)(iVar9 + iVar11) =
       (float)piVar8[0x60] + ((float)piVar8[99] - (float)piVar8[0x60]) * fVar19;
  *(float *)(iVar9 + 4 + iVar11) =
       (float)piVar8[0x61] + ((float)piVar8[100] - (float)piVar8[0x61]) * fVar19;
  *(float *)(iVar9 + 8 + iVar11) =
       (float)piVar8[0x62] + ((float)piVar8[0x65] - (float)piVar8[0x62]) * fVar19;
  if ((((*(byte *)(piVar8 + 0x66) & 0x40) == 0) && (*(char *)((int)piVar8 + 0x19a) != '\x02')) &&
     (*(char *)((int)piVar8 + 0x19a) != '\0')) {
    pfVar12 = (float *)(**(code **)(piVar8[0x5e] + 0x24))();
    pfVar10 = *(float **)(unaff_EBX + 0x4e9eb3);
    if ((*pfVar10 == *pfVar12) && (!NAN(*pfVar10) && !NAN(*pfVar12))) {
      if ((pfVar10[1] == pfVar12[1]) && (!NAN(pfVar10[1]) && !NAN(pfVar12[1]))) {
        if ((pfVar10[2] == pfVar12[2]) && (!NAN(pfVar10[2]) && !NAN(pfVar12[2]))) goto LAB_006c1fc6;
      }
    }
    in2 = (matrix3x4_t *)(**(code **)(piVar8[0x5e] + 0x28))();
    VectorTransform((float *)(iVar11 + uVar14 * 0xc),in2,(float *)(iVar11 + uVar13 * 0xc));
  }
  else {
LAB_006c1fc6:
    pfVar10 = (float *)(**(code **)(piVar8[0x5e] + 0x20))();
    iVar9 = uVar13 * 0xc;
    iVar15 = uVar14 * 0xc;
    *(float *)(iVar9 + iVar11) = *(float *)(iVar15 + iVar11) + *pfVar10;
    *(float *)(iVar9 + 4 + iVar11) = *(float *)(iVar15 + 4 + iVar11) + pfVar10[1];
    *(float *)(iVar9 + 8 + iVar11) = *(float *)(iVar15 + 8 + iVar11) + pfVar10[2];
  }
                    /* Unresolved local var: Vector res@[???] */
  iVar9 = uVar13 * 0xc;
  fVar19 = *(float *)(unaff_EBX + 0x36b817);
                    /* Unresolved local var: Vector res@[???] */
  local_4c = local_40 + *(float *)(iVar9 + iVar11) * fVar19;
  local_48 = *(float *)(iVar9 + 4 + iVar11) * fVar19 + local_3c;
  local_44 = fVar19 * *(float *)(iVar9 + 8 + iVar11) + local_38;
  __return_storage_ptr__->x = local_4c;
  __return_storage_ptr__->y = local_48;
  __return_storage_ptr__->z = local_44;
  (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x220))()
  ;
  local_58 = local_64;
  local_54 = local_60;
  local_50 = local_5c;
  (**(code **)(*piVar8 + 0x230))();
  cVar6 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                super_CBaseCombatCharacter.super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x3d8))();
  cVar7 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                super_CBaseCombatCharacter.super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x250))();
  if (cVar7 != '\0') {
    cVar7 = (**(code **)(*piVar8 + 0x114))();
    bVar5 = true;
    if (cVar7 != '\0') goto LAB_006c215d;
  }
  bVar5 = false;
LAB_006c215d:
                    /* Unresolved local var: Vector res@[???] */
  local_88 = __return_storage_ptr__->x - local_28[0].x;
  local_84 = __return_storage_ptr__->y - local_28[0].y;
  local_80 = __return_storage_ptr__->z - local_28[0].z;
  local_7c[0].x = local_88;
  local_7c[0].y = local_84;
  local_7c[0].z = local_80;
  fVar19 = VectorNormalize(local_7c);
  cVar7 = (**(code **)(*piVar8 + 0x114))();
  if ((cVar7 != '\0') &&
     (iVar11 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                     super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                     super_CBaseCombatCharacter.super_CBaseFlex.
                                     super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x3e0))(),
     iVar11 != 0)) {
    uVar23 = 0;
    uVar22 = 0x4041;
    pCVar21 = this;
    iVar9 = iVar11;
    cVar7 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                  super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                  super_CBaseCombatCharacter.super_CBaseFlex.
                                  super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x3d4))();
    if (cVar7 != '\0') {
      uVar1 = *(uint *)(iVar11 + 0x524);
      if ((uVar1 == 0xffffffff) ||
         (iVar11 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e9e13),
         *(uint *)(iVar11 + 8) != uVar1 >> 0x10)) {
        iVar11 = 0;
      }
      else {
        iVar11 = *(int *)(iVar11 + 4);
      }
      ptTransformed = &local_94;
      puVar17 = (undefined4 *)(iVar11 + 0x528);
      puVar18 = (undefined4 *)&stack0xfffffe88;
      ptSource = __return_storage_ptr__;
      for (iVar15 = 0x10; iVar15 != 0; iVar15 = iVar15 + -1) {
        *puVar18 = *puVar17;
        puVar17 = puVar17 + 1;
        puVar18 = puVar18 + 1;
      }
      matThisToLinked.m[0][1] = (vec_t)iVar9;
      matThisToLinked.m[0][0] = (vec_t)pCVar21;
      matThisToLinked.m[0][2] = (vec_t)piVar8;
      matThisToLinked.m[0][3] = (vec_t)uVar22;
      matThisToLinked.m[1][0] = (vec_t)uVar23;
      matThisToLinked.m[1][1] = (vec_t)out;
      matThisToLinked.m[1][2] = (vec_t)uVar24;
      matThisToLinked.m[1][3] = (vec_t)in_stack_fffffea4._0_4_;
      matThisToLinked.m[2][0] = (vec_t)in_stack_fffffea4._4_4_;
      matThisToLinked.m[2][1] = (vec_t)in_stack_fffffea4._8_4_;
      matThisToLinked.m[2][2] = (vec_t)in_stack_fffffea4._12_4_;
      matThisToLinked.m[2][3] = (vec_t)in_stack_fffffea4._16_4_;
      matThisToLinked.m[3][0] = (vec_t)in_stack_fffffea4._20_4_;
      matThisToLinked.m[3][1] = (vec_t)in_stack_fffffea4._24_4_;
      matThisToLinked.m[3][2] = (vec_t)in_stack_fffffea4._28_4_;
      matThisToLinked.m[3][3] = (vec_t)in_stack_fffffea4._32_4_;
      UTIL_Portal_PointTransform(matThisToLinked,ptSource,ptTransformed);
                    /* Unresolved local var: Vector res@[???] */
      local_ac = local_94.x - local_28[0].x;
      local_a8 = local_94.y - local_28[0].y;
      local_a4 = local_94.z - local_28[0].z;
      local_a0.x = local_ac;
      local_a0.y = local_a8;
      local_a0.z = local_a4;
      fVar20 = VectorNormalize(&local_a0);
      if (((cVar6 == '\0') || (!bVar5)) || (fVar20 < fVar19)) {
        __return_storage_ptr__->x = local_94.x;
        __return_storage_ptr__->y = local_94.y;
        __return_storage_ptr__->z = local_94.z;
        local_7c[0].x = local_a0.x;
        local_7c[0].y = local_a0.y;
        local_7c[0].z = local_a0.z;
      }
    }
  }
  VectorNormalize(local_7c);
  VectorAngles(local_7c,&local_b8);
  (this->m_vecGoalAngles).x = local_b8.x;
  (this->m_vecGoalAngles).y = local_b8.y;
  (this->m_vecGoalAngles).z = local_b8.z;
  return __return_storage_ptr__;
}


/* CNPC_HoverTurret::TraceAttack at 006bf400 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void TraceAttack(CNPC_HoverTurret * this, CTakeDamageInfo * info,
   Vector * vecDir, trace_t * ptr) */

void __thiscall
CNPC_HoverTurret::TraceAttack
          (CNPC_HoverTurret *this,CTakeDamageInfo *info,Vector *vecDir,trace_t *ptr)

{
  CAI_BaseNPC::TraceAttack((CAI_BaseNPC *)this,info,vecDir,ptr);
  return;
}


/* CNPC_HoverTurret::StartDeathSequence at 006bec70 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void StartDeathSequence(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::StartDeathSequence(CNPC_HoverTurret *this)

{
  char *soundname;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((this->m_flDeathTime == 0.0) && (!NAN(this->m_flDeathTime))) {
    this->m_flDeathTime =
         *(float *)(**(int **)(&DAT_004ecf1c + unaff_EBX) + 0xc) + *(float *)(unaff_EBX + 0x36ce3c);
    soundname = (char *)(unaff_EBX + 0x350716);
    CBaseEntity::EmitSound((CBaseEntity *)this,soundname,0.0,(float *)0x0);
    CBaseEntity::EmitSound
              ((CBaseEntity *)this,soundname,
               *(float *)(unaff_EBX + 0x36b87c) +
               *(float *)(**(int **)(&DAT_004ecf1c + unaff_EBX) + 0xc),(float *)0x0);
    CBaseEntity::EmitSound
              ((CBaseEntity *)this,soundname,
               *(float *)(unaff_EBX + 0x36b68c) +
               *(float *)(**(int **)(&DAT_004ecf1c + unaff_EBX) + 0xc),(float *)0x0);
    CBaseEntity::EmitSound
              ((CBaseEntity *)this,soundname,
               *(float *)(unaff_EBX + 0x36b990) +
               *(float *)(**(int **)(&DAT_004ecf1c + unaff_EBX) + 0xc),(float *)0x0);
    CBaseEntity::EmitSound
              ((CBaseEntity *)this,(char *)(unaff_EBX + 0x354740),
               *(float *)(unaff_EBX + 0x36bc4c) +
               *(float *)(**(int **)(&DAT_004ecf1c + unaff_EBX) + 0xc),(float *)0x0);
  }
  return;
}


/* CNPC_HoverTurret::TakeDamageFromPhysicsImpact at 006bf180 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void TakeDamageFromPhysicsImpact(CNPC_HoverTurret * this, int index,
   gamevcollisionevent_t * pEvent) */

void __thiscall
CNPC_HoverTurret::TakeDamageFromPhysicsImpact
          (CNPC_HoverTurret *this,int index,gamevcollisionevent_t *pEvent)

{
  CBaseEntity *this_00;
  IPhysicsCollisionData *pIVar1;
  IPhysicsObject *pIVar2;
  float *pfVar3;
  code *pcVar4;
  bool bVar5;
  int unaff_EBX;
  bool bVar6;
  longdouble extraout_ST0;
  longdouble extraout_ST0_00;
  CTakeDamageInfo *in_stack_ffffff18;
  CBaseEntity *in_stack_ffffff1c;
  CBaseEntity *in_stack_ffffff20;
  Vector *in_stack_ffffff24;
  Vector *in_stack_ffffff28;
  float in_stack_ffffff2c;
  float bitsDamageType_1;
  int in_stack_ffffff34;
  Vector *in_stack_ffffff38;
  CBaseEntity *pHitEntity;
  float damage;
  CTakeDamageInfo local_a4;
  float local_54;
  float local_50;
  float local_4c;
  float local_48;
  float local_44;
  float local_40;
  Vector local_3c;
  Vector local_30;
  char *local_24;
  int damageType;
  
                    /* Unresolved local var: float flDamageScale@[???]
                       Unresolved local var: Vector damagePos@[???]
                       Unresolved local var: Vector damageForce@[???] */
  ___i686_get_pc_thunk_bx();
  bVar6 = index == 0;
  this_00 = pEvent->pEntities[bVar6];
  damageType = 0;
  damage = CalculateDefaultPhysicsDamage
                     (index,pEvent,
                      *(float *)(unaff_EBX + 0x36b3a3) *
                      (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                      super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                      m_impactEnergyScale,true,&damageType,(string_t)0x0,false);
  if ((this_00 == (CBaseEntity *)0x0) ||
     ((local_24 = (char *)(unaff_EBX + 0x2f9fcd), (this_00->m_iClassname).pszValue != local_24 &&
      (bVar5 = CBaseEntity::ClassMatchesComplex(this_00,local_24), !bVar5)))) {
    if ((damage == *(float *)(unaff_EBX + 0x36b3df)) &&
       (!NAN(damage) && !NAN(*(float *)(unaff_EBX + 0x36b3df)))) {
      return;
    }
  }
  else {
    damage = *(float *)(unaff_EBX + 0x36b487);
  }
  pIVar1 = (pEvent->super_vcollisionevent_t).pInternalData;
  (*pIVar1->_vptr_IPhysicsCollisionData[1])(pIVar1,&local_30);
  pIVar2 = (pEvent->super_vcollisionevent_t).pObjects[index];
  (*pIVar2->_vptr_IPhysicsObject[0x1e])(pIVar2);
  bitsDamageType_1 = (float)extraout_ST0;
                    /* Unresolved local var: Vector res@[???] */
  local_48 = bitsDamageType_1 * pEvent->postVelocity[index].x;
  local_44 = bitsDamageType_1 * pEvent->postVelocity[index].y;
  local_40 = bitsDamageType_1 * pEvent->postVelocity[index].z;
  pfVar3 = *(float **)(unaff_EBX + 0x4eca1f);
  local_3c.x = local_48;
  local_3c.y = local_44;
  local_3c.z = local_40;
  if ((*pfVar3 == local_48) && (!NAN(*pfVar3) && !NAN(local_48))) {
    if ((pfVar3[1] == local_44) && (!NAN(pfVar3[1]) && !NAN(local_44))) {
      if ((pfVar3[2] == local_40) && (!NAN(pfVar3[2]) && !NAN(local_40))) {
        pIVar2 = (pEvent->super_vcollisionevent_t).pObjects[bVar6];
        (*pIVar2->_vptr_IPhysicsObject[0x1e])(pIVar2);
        local_4c = (float)extraout_ST0_00;
                    /* Unresolved local var: Vector res@[???] */
        local_54 = local_4c * pEvent->postVelocity[bVar6].x;
        local_50 = local_4c * pEvent->postVelocity[bVar6].y;
        local_4c = local_4c * pEvent->postVelocity[bVar6].z;
        local_3c.x = local_54;
        local_3c.y = local_50;
        local_3c.z = local_4c;
      }
    }
  }
  if (damage < *(float *)(unaff_EBX + 0x36c153) || damage == *(float *)(unaff_EBX + 0x36c153)) {
    return;
  }
  pcVar4 = *(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                               super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                               super_CBaseCombatCharacter.super_CBaseFlex.
                               super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x118);
  CTakeDamageInfo::CTakeDamageInfo
            (&local_a4,this_00,this_00,&local_3c,&local_30,damage,damageType,0,(Vector *)0x0,
             in_stack_ffffff18,in_stack_ffffff1c,in_stack_ffffff20,in_stack_ffffff24,
             in_stack_ffffff28,in_stack_ffffff2c,(int)bitsDamageType_1,in_stack_ffffff34,
             in_stack_ffffff38);
  (*pcVar4)(this,&local_a4);
  return;
}


/* CNPC_HoverTurret::VPhysicsCollision at 006c4870 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void VPhysicsCollision(CNPC_HoverTurret * this, int index,
   gamevcollisionevent_t * pEvent) */

void __thiscall
CNPC_HoverTurret::VPhysicsCollision(CNPC_HoverTurret *this,int index,gamevcollisionevent_t *pEvent)

{
  int *piVar1;
  char cVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: int otherIndex@[???]
                       Unresolved local var: CBaseEntity * pHitEntity@[???] */
  ___i686_get_pc_thunk_bx();
  if (0 < ((CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth> *)
          ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                 super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                 super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0xe8))->m_Value)
  {
    CBaseEntity::VPhysicsCollision((CBaseEntity *)this,index,pEvent);
    piVar1 = *(int **)((int)pEvent->pEntities + (-(uint)(index == 0) & 4));
    (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
    super_CAI_BaseNPC.super_CBaseCombatCharacter.m_flFieldOfView = -1.0;
    if (piVar1 != (int *)0x0) {
      cVar2 = (**(code **)(*piVar1 + 0x154))(piVar1);
      if (cVar2 == '\0') {
        if ((piVar1 != (int *)**(int **)(unaff_EBX + 0x4e73d8)) &&
           (this->m_flEngineStallTime =
                 *(float *)(**(int **)(unaff_EBX + 0x4e731c) + 0xc) +
                 *(float *)(&DAT_00365cb4 + unaff_EBX),
           this->m_iFiringState != HOVER_TURRET_SHOT_DISABLED)) {
          SetFiringState(this,HOVER_TURRET_HIT_WITH_PHYSICS);
        }
        TakeDamageFromPhysicsImpact(this,index,pEvent);
        return;
      }
      if (this->m_iFiringState != HOVER_TURRET_AIM_TARGET) {
        SetFiringState(this,HOVER_TURRET_FIND_TARGET);
        this->m_flEngineStallTime =
             *(float *)(**(int **)(unaff_EBX + 0x4e731c) + 0xc) +
             *(float *)(&DAT_00365c7c + unaff_EBX);
      }
    }
  }
  return;
}


/* CNPC_HoverTurret::MaintainGroundHeight at 006be910 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void MaintainGroundHeight(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::MaintainGroundHeight(CNPC_HoverTurret *this)

{
  return;
}


/* CNPC_HoverTurret::TurnHeadToTarget at 006bf530 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void TurnHeadToTarget(CNPC_HoverTurret * this, float flInterval, Vector
   * MoveTarget) */

void __thiscall
CNPC_HoverTurret::TurnHeadToTarget(CNPC_HoverTurret *this,float flInterval,Vector *MoveTarget)

{
  float current;
  int unaff_EBX;
  longdouble lVar1;
  float target;
  float fVar2;
  float fVar3;
  float flDestYaw;
  float newYaw;
  Vector local_28 [2];
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: Vector res@[???] */
  local_28[0].x =
       MoveTarget->x -
       (((CNetworkVectorBase<Vector,CBaseEntity::NetworkVar_m_vecOrigin> *)
        ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
               super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
               super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x350))->
       super_CNetworkVarBase<Vector,CBaseEntity::NetworkVar_m_vecOrigin>).m_Value.x;
  local_28[0].y =
       MoveTarget->y -
       *(vec_t *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x354);
  local_28[0].z =
       MoveTarget->z -
       *(vec_t *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x358);
  target = CAI_BaseNPC::VecToYaw((CAI_BaseNPC *)this,local_28);
  fVar3 = *(float *)(**(int **)(unaff_EBX + 0x4ec65c) + 0xc);
  fVar2 = CBaseEntity::GetLastThink((CBaseEntity *)this,(char *)0x0);
  current = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.m_fHeadYaw;
  lVar1 = (longdouble)
          (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                super_CBaseCombatCharacter.super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x9c8))(this);
  fVar3 = AI_ClampYaw((float)lVar1 * *(float *)(&DAT_0036afc8 + unaff_EBX),current,target,
                      fVar3 - fVar2);
  if (fVar3 != (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
               super_CAI_BasePhysicsFlyingBot.m_fHeadYaw) {
    (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
    m_fHeadYaw = fVar3;
  }
  return;
}


/* CNPC_HoverTurret::UpdateFacing at 006bf7f0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: bool UpdateFacing(CNPC_HoverTurret * this) */

bool __thiscall CNPC_HoverTurret::UpdateFacing(CNPC_HoverTurret *this)

{
  int iVar1;
  model_t *pmVar2;
  CStudioHdr *pCVar3;
  int unaff_EBX;
  longdouble lVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  QAngle local_40;
  Vector local_34;
  Vector local_28 [2];
  
                    /* Unresolved local var: bool bMoved@[???]
                       Unresolved local var: Vector vecGoalDir@[???]
                       Unresolved local var: Vector vecGoalLocalDir@[???]
                       Unresolved local var: QAngle vecGoalLocalAngles@[???]
                       Unresolved local var: float flDiff@[???] */
  ___i686_get_pc_thunk_bx();
  iVar1 = *(int *)(**(int **)(unaff_EBX + 0x4ec3a2) + 0x18);
  if (iVar1 != this->m_muzzleToWorldTick) {
    this->m_muzzleToWorldTick = iVar1;
    (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x364))
              (this,this->m_iMuzzleAttachment,&this->m_muzzleToWorld);
  }
  AngleVectors(&this->m_vecGoalAngles,local_28);
  VectorIRotate(&local_28[0].x,&this->m_muzzleToWorld,&local_34.x);
  VectorAngles(&local_34,&local_40);
  lVar4 = (longdouble)
          (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                super_CBaseCombatCharacter.super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x7a8))(this);
  fVar5 = ApproachAngle(local_40.x,0.0,(float)lVar4 * *(float *)(unaff_EBX + 0x36aec2));
  fVar5 = AngleNormalize(fVar5);
  fVar6 = CBaseAnimating::GetPoseParameter
                    ((CBaseAnimating *)this,
                     (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                     super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.m_poseAim_Pitch);
  iVar1 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .super_CAI_BaseNPC.m_poseAim_Pitch;
  pCVar3 = *(CStudioHdr **)
            ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                   super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                   super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x500);
  if (pCVar3 == (CStudioHdr *)0x0) {
    pmVar2 = CBaseEntity::GetModel((CBaseEntity *)this);
    if (pmVar2 == (model_t *)0x0) {
      pCVar3 = *(CStudioHdr **)
                ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                       super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                       super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x500);
    }
    else {
      CBaseAnimating::LockStudioHdr((CBaseAnimating *)this);
      pCVar3 = *(CStudioHdr **)
                ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                       super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                       super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x500);
    }
    if (pCVar3 != (CStudioHdr *)0x0) goto LAB_006bf8f4;
  }
  else {
LAB_006bf8f4:
    if (pCVar3->m_pStudioHdr == (studiohdr_t *)0x0) {
      pCVar3 = (CStudioHdr *)0x0;
    }
  }
  CBaseAnimating::SetPoseParameter
            ((CBaseAnimating *)this,pCVar3,iVar1,fVar5 / *(float *)(unaff_EBX + 0x36c2c2) + fVar6);
  lVar4 = (longdouble)
          (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                super_CBaseCombatCharacter.super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x7a8))(this);
  fVar6 = ApproachAngle(local_40.y,0.0,(float)lVar4 * *(float *)(CAI_Motor::Init + unaff_EBX + 6));
  fVar6 = AngleNormalize(fVar6);
  fVar7 = CBaseAnimating::GetPoseParameter
                    ((CBaseAnimating *)this,
                     (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                     super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.m_poseAim_Yaw);
  iVar1 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .super_CAI_BaseNPC.m_poseAim_Yaw;
  pCVar3 = *(CStudioHdr **)
            ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                   super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                   super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x500);
  if (pCVar3 == (CStudioHdr *)0x0) {
    pmVar2 = CBaseEntity::GetModel((CBaseEntity *)this);
    if (pmVar2 == (model_t *)0x0) {
      pCVar3 = *(CStudioHdr **)
                ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                       super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                       super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x500);
    }
    else {
      CBaseAnimating::LockStudioHdr((CBaseAnimating *)this);
      pCVar3 = *(CStudioHdr **)
                ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                       super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                       super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x500);
    }
    if (pCVar3 == (CStudioHdr *)0x0) goto LAB_006bf9ac;
  }
  if (pCVar3->m_pStudioHdr == (studiohdr_t *)0x0) {
    pCVar3 = (CStudioHdr *)0x0;
  }
LAB_006bf9ac:
  CBaseAnimating::SetPoseParameter
            ((CBaseAnimating *)this,pCVar3,iVar1,fVar6 / *(float *)(unaff_EBX + 0x36c2c2) + fVar7);
  fVar7 = *(float *)(unaff_EBX + 0x36ab16);
  (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x370))
            (this);
  return fVar7 < ABS(fVar6) || fVar7 < ABS(fVar5);
}


/* CNPC_HoverTurret::UpdateMuzzleMatrix at 006be920 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void UpdateMuzzleMatrix(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::UpdateMuzzleMatrix(CNPC_HoverTurret *this)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  iVar1 = *(int *)(**(int **)(unaff_EBX + 0x4ed274) + 0x18);
  if (iVar1 != this->m_muzzleToWorldTick) {
    this->m_muzzleToWorldTick = iVar1;
    (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x364))
              (this,this->m_iMuzzleAttachment,&this->m_muzzleToWorld);
  }
  return;
}


/* CNPC_HoverTurret::OverrideMove at 006bfe60 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: bool OverrideMove(CNPC_HoverTurret * this, float flInterval) */

bool __thiscall CNPC_HoverTurret::OverrideMove(CNPC_HoverTurret *this,float flInterval)

{
  float *pfVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  IPhysicsObject *pIVar9;
  uint uVar10;
  vec_t *pvVar11;
  int iVar12;
  IPhysicsMotionController *pIVar13;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
      super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
      super_CBaseAnimating.super_CBaseEntity.m_iHealth.m_Value < 1) {
    MoveExecute_Dead(this,flInterval);
    return true;
  }
  if (this->m_bInitialPositionSet == false) {
    if (((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
         super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
         super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
    (this->m_vForceMoveTarget).x =
         (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
         super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
         super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.x;
    (this->m_vForceMoveTarget).y =
         (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
         super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
         super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.y;
    (this->m_vForceMoveTarget).z =
         (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
         super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
         super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z + *(float *)(unaff_EBX + 0x3733de);
    this->m_bInitialPositionSet = true;
  }
  pIVar9 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
           m_pPhysicsObject;
  if (pIVar9 != (IPhysicsObject *)0x0) {
    (*pIVar9->_vptr_IPhysicsObject[0x19])(pIVar9);
  }
  pfVar1 = (float *)(**(int **)(unaff_EBX + 0x4ebd32) + 0xc);
  if ((this->m_flEngineStallTime < *pfVar1 || this->m_flEngineStallTime == *pfVar1) &&
     (this->m_iFiringState != HOVER_TURRET_SHOT_DISABLED)) {
    if ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
        m_pMotionController == (IPhysicsMotionController *)0x0) {
      pIVar13 = (IPhysicsMotionController *)
                (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4ebdca) + 0x78))
                          ((int *)**(undefined4 **)(unaff_EBX + 0x4ebdca),
                           &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                            super_CAI_BasePhysicsFlyingBot.super_IMotionEvent);
      (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
      m_pMotionController = pIVar13;
      (*pIVar13->_vptr_IPhysicsMotionController[3])
                (pIVar13,(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                         super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter
                         .super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                         super_CBaseEntity.m_pPhysicsObject,1);
    }
    (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x9c4))(this,flInterval,&this->m_vForceMoveTarget);
    CBaseEntity::PhysicsCheckWaterTransition((CBaseEntity *)this);
    fVar2 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.m_vCurrentVelocity.y;
    fVar3 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.m_vCurrentVelocity.x;
    fVar4 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.m_vCurrentVelocity.z;
    pIVar9 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
             m_pPhysicsObject;
    if (pIVar9 != (IPhysicsObject *)0x0) {
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
      uVar10 = (this->m_hTether).super_CBaseHandle.m_Index;
      if (((uVar10 != 0xffffffff) &&
          (iVar12 = (uVar10 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4ebd36),
          *(uint *)(iVar12 + 8) == uVar10 >> 0x10)) && (*(int *)(iVar12 + 4) != 0)) {
        (*pIVar9->_vptr_IPhysicsObject[0x19])(pIVar9);
      }
    }
    fVar5 = *(float *)(unaff_EBX + 0x36b5fe);
    fVar6 = *(float *)(*(int *)(*(int *)(&DAT_004ebe9a + unaff_EBX) + 0x1c) + 0x2c);
                    /* Unresolved local var: Vector res@[???] */
    fVar7 = (this->m_vForceVelocity).y;
    fVar8 = (this->m_vForceVelocity).z;
    (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
    m_vCurrentVelocity.x = fVar3 + (this->m_vForceVelocity).x;
    (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
    m_vCurrentVelocity.y = fVar2 + fVar7;
    (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
    m_vCurrentVelocity.z = flInterval * fVar5 * fVar6 + fVar4 + fVar8;
    pvVar11 = *(vec_t **)(unaff_EBX + 0x4ebd42);
    (this->m_vForceVelocity).x = *pvVar11;
    (this->m_vForceVelocity).y = pvVar11[1];
    (this->m_vForceVelocity).z = pvVar11[2];
    return true;
  }
  return false;
}


/* CNPC_HoverTurret::MoveExecute_Alive at 006bf040 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void MoveExecute_Alive(CNPC_HoverTurret * this, float flInterval) */

void __thiscall CNPC_HoverTurret::MoveExecute_Alive(CNPC_HoverTurret *this,float flInterval)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  IPhysicsObject *pIVar8;
  uint uVar9;
  vec_t *pvVar10;
  int iVar11;
  int unaff_EBX;
  
                    /* Unresolved local var: Vector vCurrentVelocity@[???] */
  ___i686_get_pc_thunk_bx();
  CBaseEntity::PhysicsCheckWaterTransition((CBaseEntity *)this);
  fVar1 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .m_vCurrentVelocity.y;
  fVar2 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .m_vCurrentVelocity.x;
  fVar3 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .m_vCurrentVelocity.z;
  pIVar8 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
           m_pPhysicsObject;
  if (pIVar8 != (IPhysicsObject *)0x0) {
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
    uVar9 = (this->m_hTether).super_CBaseHandle.m_Index;
    if (((uVar9 != 0xffffffff) &&
        (iVar11 = (uVar9 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4ecb50),
        *(uint *)(iVar11 + 8) == uVar9 >> 0x10)) && (*(int *)(iVar11 + 4) != 0)) {
      (*pIVar8->_vptr_IPhysicsObject[0x19])(pIVar8);
    }
  }
  fVar4 = *(float *)(unaff_EBX + 0x36c418);
  fVar5 = *(float *)(*(int *)(*(int *)(&DAT_004eccb4 + unaff_EBX) + 0x1c) + 0x2c);
                    /* Unresolved local var: Vector res@[???] */
  fVar6 = (this->m_vForceVelocity).y;
  fVar7 = (this->m_vForceVelocity).z;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vCurrentVelocity.x = fVar2 + (this->m_vForceVelocity).x;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vCurrentVelocity.y = fVar1 + fVar6;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vCurrentVelocity.z = flInterval * fVar4 * fVar5 + fVar3 + fVar7;
  pvVar10 = *(vec_t **)(unaff_EBX + 0x4ecb5c);
  (this->m_vForceVelocity).x = *pvVar10;
  (this->m_vForceVelocity).y = pvVar10[1];
  (this->m_vForceVelocity).z = pvVar10[2];
  return;
}


/* CNPC_HoverTurret::MoveExecute_Dead at 006bfcc0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void MoveExecute_Dead(CNPC_HoverTurret * this, float flInterval) */

void __thiscall CNPC_HoverTurret::MoveExecute_Dead(CNPC_HoverTurret *this,float flInterval)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  code *pcVar6;
  vec_t *pvVar7;
  int *piVar8;
  int unaff_EBX;
  longdouble lVar9;
  undefined1 local_34 [12];
  undefined1 local_28 [24];
  
                    /* Unresolved local var: Vector newVelocity@[???]
                       Unresolved local var: Vector forward@[???]
                       Unresolved local var: Vector up@[???] */
  ___i686_get_pc_thunk_bx();
  fVar1 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .m_vCurrentVelocity.z;
  fVar2 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .m_vCurrentVelocity.y;
  fVar3 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .m_vCurrentVelocity.x;
  (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x238))
            (this,local_28,0,local_34);
  fVar4 = *(float *)(**(int **)(unaff_EBX + 0x4ebed2) + 0xc);
  if (this->m_fSparkTime <= fVar4 && fVar4 != this->m_fSparkTime) {
    piVar8 = (int *)**(undefined4 **)(unaff_EBX + 0x4ec006);
    pcVar6 = *(code **)(*piVar8 + 0x10);
    if ((*(byte *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                         super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter
                         .super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x12d)
        & 8) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
      piVar8 = (int *)**(undefined4 **)(unaff_EBX + 0x4ec006);
    }
    (*pcVar6)(piVar8,(Vector *)
                     ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                            super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                            super_CBaseAnimating + 0x294),1,1,0);
    fVar4 = *(float *)(**(int **)(unaff_EBX + 0x4ebed2) + 0xc);
    lVar9 = (longdouble)
            (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4ebf56) + 4))
                      ((int *)**(undefined4 **)(unaff_EBX + 0x4ebf56),0x3d4ccccd,0x3dcccccd);
    this->m_fSparkTime = fVar4 + (float)lVar9;
  }
  fVar4 = (this->m_vForceVelocity).y;
  fVar5 = (this->m_vForceVelocity).z;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vCurrentVelocity.x = fVar3 + (this->m_vForceVelocity).x;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vCurrentVelocity.y = fVar2 + fVar4;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vCurrentVelocity.z = fVar1 + fVar5;
  pvVar7 = *(vec_t **)(unaff_EBX + 0x4ebee2);
  (this->m_vForceVelocity).x = *pvVar7;
  (this->m_vForceVelocity).y = pvVar7[1];
  (this->m_vForceVelocity).z = pvVar7[2];
  return;
}


/* CNPC_HoverTurret::OnPhysGunPickup at 006c47f0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void OnPhysGunPickup(CNPC_HoverTurret * this, CBasePlayer *
   pPhysGunUser, PhysGunPickup_t reason) */

void __thiscall
CNPC_HoverTurret::OnPhysGunPickup
          (CNPC_HoverTurret *this,CBasePlayer *pPhysGunUser,PhysGunPickup_t reason)

{
  IPhysicsMotionController *pIVar1;
  char cVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  cVar2 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                super_CBaseCombatCharacter.super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x114))(this);
  if (cVar2 != '\0') {
    pIVar1 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
             super_CAI_BasePhysicsFlyingBot.m_pMotionController;
    if (pIVar1 != (IPhysicsMotionController *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(&DAT_004e7437 + unaff_EBX) + 0x7c))
                ((int *)**(undefined4 **)(&DAT_004e7437 + unaff_EBX),pIVar1);
      (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
      m_pMotionController = (IPhysicsMotionController *)0x0;
    }
    this->m_bCarriedByPlayer = true;
    SetFiringState(this,HOVER_TURRET_SHOT_DISABLED);
    return;
  }
  return;
}


/* CNPC_HoverTurret::OnPhysGunDrop at 006c46d0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void OnPhysGunDrop(CNPC_HoverTurret * this, CBasePlayer * pPhysGunUser,
   PhysGunDrop_t Reason) */

void __thiscall
CNPC_HoverTurret::OnPhysGunDrop
          (CNPC_HoverTurret *this,CBasePlayer *pPhysGunUser,PhysGunDrop_t Reason)

{
  uint *puVar1;
  uint uVar2;
  CBaseEdict *this_00;
  int iVar3;
  IChangeInfoAccessor *pIVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  this->m_bCarriedByPlayer = false;
  if (((CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth> *)
      ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0xe8))->m_Value < 1)
  {
    StartDeathSequence(this);
    return;
  }
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
  uVar2 = (this->m_hTether).super_CBaseHandle.m_Index;
  if (((uVar2 != 0xffffffff) &&
      (iVar3 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e74c7),
      *(uint *)(iVar3 + 8) == uVar2 >> 0x10)) && (*(int *)(iVar3 + 4) != 0)) {
    SetFiringState(this,HOVER_TURRET_FIND_TARGET);
    this->m_flEngineStallTime =
         *(float *)(**(int **)(unaff_EBX + 0x4e74c3) + 0xc) + *(float *)(unaff_EBX + 0x3673e3);
    (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
    super_CAI_BaseNPC.super_CBaseCombatCharacter.m_flFieldOfView = -1.0;
    return;
  }
  this->m_iDesiredState = 7;
  if (((CNetworkVarBase<int,CBaseAnimating::NetworkVar_m_nSkin> *)
      ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x3e0))->m_Value !=
      0) {
    if (*(bool *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x60) ==
        false) {
      this_00 = *(CBaseEdict **)
                 ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x24);
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar4 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar4->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = (uint *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                              super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay
                              .super_CBaseAnimating + 100);
      *puVar1 = *puVar1 | 1;
    }
    ((CNetworkVarBase<int,CBaseAnimating::NetworkVar_m_nSkin> *)
    ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x3e0))->m_Value = 0;
  }
                    /* WARNING: Could not recover jumptable at 0x006c4759. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x684))()
  ;
  return;
}


/* CNPC_HoverTurret::HasPreferredCarryAnglesForPlayer at 006be980 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: bool HasPreferredCarryAnglesForPlayer(CNPC_HoverTurret * this,
   CBasePlayer * pPlayer) */

bool __thiscall
CNPC_HoverTurret::HasPreferredCarryAnglesForPlayer(CNPC_HoverTurret *this,CBasePlayer *pPlayer)

{
  char cVar1;
  undefined1 uVar2;
  
  cVar1 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                super_CBaseCombatCharacter.super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x114))(this);
  uVar2 = 0;
  if (cVar1 != '\0') {
    uVar2 = this->m_bUseCarryAngles;
  }
  return (bool)uVar2;
}


/* CNPC_HoverTurret::PreferredCarryAngles at 006bf640 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: QAngle PreferredCarryAngles(CNPC_HoverTurret * this) */

QAngle * __thiscall
CNPC_HoverTurret::PreferredCarryAngles(QAngle *__return_storage_ptr__,CNPC_HoverTurret *this)

{
  vec_t *pvVar1;
  int iVar2;
  uint *puVar3;
  int unaff_EBX;
  CBasePlayer *this_00;
  Vector local_28 [2];
  
                    /* Unresolved local var: Vector vecUserForward@[???]
                       Unresolved local var: CBasePlayer * pPlayer@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&DAT_006f827c)[unaff_EBX] == '\0') {
    iVar2 = ___cxa_guard_acquire(&DAT_006f827c + unaff_EBX);
    if (iVar2 != 0) {
      ___cxa_guard_release(&DAT_006f827c + unaff_EBX);
    }
  }
  this_00 = (CBasePlayer *)0x0;
  if (*(int *)(**(int **)(unaff_EBX + 0x4ec54c) + 0x14) < 2) {
    this_00 = UTIL_GetLocalPlayer();
  }
  CBasePlayer::EyeVectors(this_00,local_28,(Vector *)0x0,(Vector *)0x0);
  if (local_28[0].z < 0.0) {
    *(undefined4 *)(CRocket_Turret_Projectile::StopLoopingSounds + unaff_EBX + 4) = 0;
    *(undefined4 *)(unaff_EBX + 0x6f8288) = 0;
    *(undefined4 *)(unaff_EBX + 0x6f828c) = 0;
    puVar3 = (uint *)(**(code **)((int)(this_00->super_CBaseCombatCharacter).super_CBaseFlex.
                                       super_CBaseAnimatingOverlay.super_CBaseAnimating.
                                       super_CBaseEntity.super_IServerEntity.super_IServerUnknown +
                                 0x224))(this_00);
    *(uint *)(CRocket_Turret_Projectile::StopLoopingSounds + unaff_EBX + 4) = *puVar3 ^ 0x80000000;
    __return_storage_ptr__->x =
         *(vec_t *)(CRocket_Turret_Projectile::StopLoopingSounds + unaff_EBX + 4);
    __return_storage_ptr__->y = *(vec_t *)(unaff_EBX + 0x6f8288);
    __return_storage_ptr__->z = *(vec_t *)(unaff_EBX + 0x6f828c);
  }
  else {
    pvVar1 = *(vec_t **)(unaff_EBX + 0x4ec5f0);
    __return_storage_ptr__->x = *pvVar1;
    __return_storage_ptr__->y = pvVar1[1];
    __return_storage_ptr__->z = pvVar1[2];
  }
  return __return_storage_ptr__;
}


/* CNPC_HoverTurret::Precache at 006c1b50 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void Precache(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::Precache(CNPC_HoverTurret *this)

{
  uint *puVar1;
  CBaseEdict *this_00;
  __178 _Var2;
  int iVar3;
  IChangeInfoAccessor *pIVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x34d90a));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x348391));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x34d92b));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x34d949));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x34d967));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x34d985));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x34d8cb));
  _Var2 = ActivityList_RegisterPrivateActivity((char *)(unaff_EBX + 0x34d9a3));
  *(__178 *)(unaff_EBX + 0x66cfda) = _Var2;
  CAI_BaseNPC::AddActivityToSR((char *)(unaff_EBX + 0x34d9a3),_Var2);
  _Var2 = ActivityList_RegisterPrivateActivity((char *)(unaff_EBX + 0x34d9bb));
  *(__178 *)(unaff_EBX + 0x66cfde) = _Var2;
  CAI_BaseNPC::AddActivityToSR((char *)(unaff_EBX + 0x34d9bb),_Var2);
  _Var2 = ActivityList_RegisterPrivateActivity((char *)(unaff_EBX + 0x34d9d2));
  *(__178 *)(unaff_EBX + 0x66cfe2) = _Var2;
  CAI_BaseNPC::AddActivityToSR((char *)(unaff_EBX + 0x34d9d2),_Var2);
  _Var2 = ActivityList_RegisterPrivateActivity((char *)(unaff_EBX + 0x34d9e9));
  *(__178 *)(unaff_EBX + 0x66cfe6) = _Var2;
  CAI_BaseNPC::AddActivityToSR((char *)(unaff_EBX + 0x34d9e9),_Var2);
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x34d83c));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x351866));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x34d8f4));
  CBaseEntity::PrecacheScriptSound((char *)(CAI_RadialLinkController::Activate + unaff_EBX + 3));
  UTIL_PrecacheDecal((char *)(unaff_EBX + 0x34da18),false);
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x348946));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x34892f));
  if (*(bool *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                      super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                      super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x60) ==
      false) {
    this_00 = *(CBaseEdict **)
               ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                      super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                      super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x24);
    if (this_00 != (CBaseEdict *)0x0) {
      this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
      pIVar4 = CBaseEdict::GetChangeAccessor(this_00);
      pIVar4->m_iChangeInfoSerialNumber = 0;
    }
  }
  else {
    puVar1 = (uint *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                            super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                            super_CBaseAnimating + 100);
    *puVar1 = *puVar1 | 1;
  }
  iVar3 = CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x34722a));
  (this->m_sLaserHaloSprite).m_Value = iVar3;
                    /* Unresolved local var: int iTalkScript@[???] */
  CBaseEntity::PrecacheScriptSound(*(char **)(unaff_EBX + 0x608da2));
  CBaseEntity::PrecacheScriptSound(*(char **)(unaff_EBX + 0x608da6));
  CBaseEntity::PrecacheScriptSound(*(char **)(unaff_EBX + 0x608daa));
  CBaseEntity::PrecacheScriptSound(*(char **)(unaff_EBX + 0x608dae));
  CBaseEntity::PrecacheScriptSound(*(char **)(unaff_EBX + 0x608db2));
  CBaseEntity::PrecacheScriptSound(*(char **)(unaff_EBX + 0x608db6));
  CBaseEntity::PrecacheScriptSound(*(char **)(unaff_EBX + 0x608dba));
  CBaseEntity::PrecacheScriptSound(*(char **)(unaff_EBX + 0x608dbe));
  UTIL_PrecacheOther((char *)(unaff_EBX + 0x34f7f8),(char *)0x0);
  CAI_BaseNPC::Precache((CAI_BaseNPC *)this);
  return;
}


/* CNPC_HoverTurret::RunTask at 006bf010 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void RunTask(CNPC_HoverTurret * this, Task_t * pTask) */

void __thiscall CNPC_HoverTurret::RunTask(CNPC_HoverTurret *this,Task_t *pTask)

{
  if (pTask->iTask != 0x97) {
    CAI_BaseNPC::RunTask((CAI_BaseNPC *)this,pTask);
    return;
  }
  return;
}


/* CNPC_HoverTurret::StartTask at 006befe0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void StartTask(CNPC_HoverTurret * this, Task_t * pTask) */

void __thiscall CNPC_HoverTurret::StartTask(CNPC_HoverTurret *this,Task_t *pTask)

{
  if (pTask->iTask != 0x97) {
    CAI_BasePhysicsFlyingBot::StartTask((CAI_BasePhysicsFlyingBot *)this,pTask);
    return;
  }
  return;
}


/* CNPC_HoverTurret::UpdateOnRemove at 006befc0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void UpdateOnRemove(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::UpdateOnRemove(CNPC_HoverTurret *this)

{
  TalkThink(this);
  CAI_BaseNPC::UpdateOnRemove((CAI_BaseNPC *)this);
  return;
}


/* CNPC_HoverTurret::ClampMotorForces at 006bea90 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void ClampMotorForces(CNPC_HoverTurret * this, Vector * linear,
   AngularImpulse * angular) */

void __thiscall
CNPC_HoverTurret::ClampMotorForces(CNPC_HoverTurret *this,Vector *linear,AngularImpulse *angular)

{
  float fVar1;
  undefined4 *puVar2;
  int unaff_EBX;
  longdouble lVar3;
  
  ___i686_get_pc_thunk_bx();
  if (((CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth> *)
      ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0xe8))->m_Value < 1)
  {
    linear->x = 0.0;
    linear->y = 0.0;
    linear->z = 0.0;
    angular->x = 0.0;
    angular->y = 0.0;
    angular->z = 0.0;
  }
  else {
    linear->x = 0.0;
    linear->y = 0.0;
    linear->z = 0.0;
    if (this->m_bAimingAtTarget == false) {
      fVar1 = *(float *)(&DAT_0036bc08 + unaff_EBX);
      angular->x = angular->x * fVar1;
      angular->y = fVar1 * angular->y;
      if ((this->m_flSentryTurnSpeed == 0.0) && (!NAN(this->m_flSentryTurnSpeed))) {
        puVar2 = *(undefined4 **)(&DAT_004ed180 + unaff_EBX);
        lVar3 = (longdouble)(**(code **)(*(int *)*puVar2 + 4))((int *)*puVar2,0xc2480000,0x42480000)
        ;
        linear->x = (float)lVar3;
        lVar3 = (longdouble)(**(code **)(*(int *)*puVar2 + 4))((int *)*puVar2,0xc2480000,0x42480000)
        ;
        linear->y = (float)lVar3;
        lVar3 = (longdouble)(**(code **)(*(int *)*puVar2 + 4))((int *)*puVar2,0xc3160000,0x43160000)
        ;
        linear->z = (float)lVar3;
      }
    }
  }
  return;
}


/* CNPC_HoverTurret::HasPhysicsAttacker at 006be9b0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: CBasePlayer * HasPhysicsAttacker(CNPC_HoverTurret * this, float dt) */

CBasePlayer * __thiscall CNPC_HoverTurret::HasPhysicsAttacker(CNPC_HoverTurret *this,float dt)

{
  return (CBasePlayer *)0x0;
}


/* CNPC_HoverTurret::GetMaxEnginePower at 006be9c0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: float GetMaxEnginePower(CNPC_HoverTurret * this) */

float __thiscall CNPC_HoverTurret::GetMaxEnginePower(CNPC_HoverTurret *this)

{
  return 1.0;
}


/* CNPC_HoverTurret::IsMovementDisabled at 006be9d0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: bool IsMovementDisabled(CNPC_HoverTurret * this) */

bool __thiscall CNPC_HoverTurret::IsMovementDisabled(CNPC_HoverTurret *this)

{
  float *pfVar1;
  int unaff_EBX;
  bool bVar2;
  
  ___i686_get_pc_thunk_bx();
  bVar2 = true;
  pfVar1 = (float *)(**(int **)(unaff_EBX + 0x4ed1c7) + 0xc);
  if (this->m_flEngineStallTime < *pfVar1 || this->m_flEngineStallTime == *pfVar1) {
    bVar2 = this->m_iFiringState == HOVER_TURRET_SHOT_DISABLED;
  }
  return bVar2;
}


/* CNPC_HoverTurret::MoveToTarget at 006c2510 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void MoveToTarget(CNPC_HoverTurret * this, float flInterval, Vector *
   vMoveTarget) */

void __thiscall
CNPC_HoverTurret::MoveToTarget(CNPC_HoverTurret *this,float flInterval,Vector *vMoveTarget)

{
  float *pfVar1;
  float current;
  int *piVar2;
  code *pcVar3;
  bool bVar4;
  int iVar5;
  int unaff_EBX;
  float fVar6;
  float fVar7;
  float local_64;
  float local_60;
  float local_5c;
  float local_58;
  float local_54;
  float local_50;
  Vector local_4c;
  float local_40;
  float local_3c;
  float local_38;
  Vector local_34;
  Vector local_28 [2];
  
  ___i686_get_pc_thunk_bx();
  if (((*(float *)(unaff_EBX + 0x368049) <= flInterval &&
        flInterval != *(float *)(unaff_EBX + 0x368049)) &&
      (piVar2 = *(int **)(&DAT_004e9679 + unaff_EBX), pfVar1 = (float *)(*piVar2 + 0xc),
      this->m_flEngineStallTime < *pfVar1 || this->m_flEngineStallTime == *pfVar1)) &&
     (this->m_iFiringState != HOVER_TURRET_SHOT_DISABLED)) {
    iVar5 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                  super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                  super_CBaseCombatCharacter.super_CBaseFlex.
                                  super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x188))(this);
    if ((iVar5 == 0) || (bVar4 = CAI_BaseNPC::HasCondition((CAI_BaseNPC *)this,10), !bVar4)) {
      if (this->m_flSentryTurnSpeed == 0.0) {
        if ((*(byte *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                             super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                             super_CBaseAnimating + 0x12d) & 8) != 0) {
          CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
        }
                    /* Unresolved local var: Vector res@[???] */
        local_40 = vMoveTarget->x -
                   ((Vector *)
                   ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x294))->x;
        local_3c = vMoveTarget->y -
                   *(vec_t *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                    super_CBaseCombatCharacter.super_CBaseFlex.
                                    super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x298);
        local_38 = vMoveTarget->z -
                   *(vec_t *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                    super_CBaseCombatCharacter.super_CBaseFlex.
                                    super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x29c);
        local_34.x = local_40;
        local_34.y = local_3c;
        local_34.z = local_38;
        VectorNormalize(&local_34);
        AngleVectors(&this->m_vInitialLookAngles,&local_4c);
                    /* Unresolved local var: Vector res@[???] */
        fVar7 = *(float *)(unaff_EBX + 0x368019);
                    /* Unresolved local var: Vector res@[???] */
        local_58 = local_34.x + local_4c.x * fVar7;
        local_54 = local_4c.y * fVar7 + local_34.y;
        local_50 = fVar7 * local_4c.z + local_34.z;
        pcVar3 = *(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                     super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                     super_CBaseCombatCharacter.super_CBaseFlex.
                                     super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x9c0);
        local_4c.x = local_58;
        local_4c.y = local_54;
        local_4c.z = local_50;
        if ((*(byte *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                             super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                             super_CBaseAnimating + 0x12d) & 8) != 0) {
          CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
        }
                    /* Unresolved local var: Vector res@[???] */
        local_64 = ((Vector *)
                   ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x294))->x + local_4c.x;
        local_60 = *(vec_t *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                    super_CBaseCombatCharacter.super_CBaseFlex.
                                    super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x298) +
                   local_4c.y;
        local_5c = *(vec_t *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                    super_CBaseCombatCharacter.super_CBaseFlex.
                                    super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x29c) +
                   local_4c.z;
        (*pcVar3)(this,flInterval,&local_64);
      }
      else {
                    /* Unresolved local var: float newYaw@[???] */
        fVar7 = *(float *)(*piVar2 + 0xc);
        fVar6 = CBaseEntity::GetLastThink((CBaseEntity *)this,(char *)0x0);
        current = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                  super_CAI_BasePhysicsFlyingBot.m_fHeadYaw;
        fVar7 = AI_ClampYaw(this->m_flSentryTurnSpeed,current,
                            flInterval * this->m_flSentryTurnSpeed + current,fVar7 - fVar6);
        if (fVar7 != (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                     super_CAI_BasePhysicsFlyingBot.m_fHeadYaw) {
          (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .m_fHeadYaw = fVar7;
        }
      }
    }
    else {
      pcVar3 = *(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                   super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                   super_CBaseCombatCharacter.super_CBaseFlex.
                                   super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x9c0);
      GetClosestVisibleEnemyPosition(local_28,this);
      (*pcVar3)(this,flInterval,local_28);
    }
  }
  return;
}


/* CNPC_HoverTurret::ShootLaser at 006c0940 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void ShootLaser(CNPC_HoverTurret * this, Vector * vecSrc, Vector *
   vecDirToEnemy) */

void __thiscall
CNPC_HoverTurret::ShootLaser(CNPC_HoverTurret *this,Vector *vecSrc,Vector *vecDirToEnemy)

{
  int iVar1;
  int unaff_EBX;
  float fVar2;
  CTakeDamageInfo *in_stack_fffffecc;
  CBaseEntity *in_stack_fffffed0;
  CBaseEntity *pAttacker_1;
  float in_stack_fffffed8;
  int in_stack_fffffedc;
  int in_stack_fffffee0;
  Vector *local_11c;
  trace_t local_100;
  CTakeDamageInfo local_ac;
  CTraceFilterSimple local_50;
  _func_int_varargs **local_40;
  IHandleEntity *local_3c;
  float local_38;
  float local_34;
  float local_30;
  float local_2c;
  float local_28;
  float local_24;
  float local_20;
  
                    /* Unresolved local var: trace_t tr@[???]
                       Unresolved local var: Vector vecCenter@[???] */
  ___i686_get_pc_thunk_bx();
  if (((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
       super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
                    /* Unresolved local var: Vector res@[???] */
  local_20 = *(float *)(CAI_Motor::SetSmoothedVelocity + unaff_EBX + 7);
  local_28 = vecDirToEnemy->x * local_20;
  local_24 = vecDirToEnemy->y * local_20;
  local_20 = local_20 * vecDirToEnemy->z;
                    /* Unresolved local var: Vector res@[???] */
  local_ac.m_vecDamageForce.x = vecSrc->x;
  local_34 = local_ac.m_vecDamageForce.x + local_28;
  local_ac.m_vecDamageForce.y = vecSrc->y;
  local_30 = local_ac.m_vecDamageForce.y + local_24;
  local_ac.m_vecDamageForce.z = vecSrc->z;
  local_2c = local_ac.m_vecDamageForce.z + local_20;
  local_ac.m_vecDamagePosition.y = local_34 - vecSrc->x;
  local_ac.m_vecDamagePosition.z = local_30 - vecSrc->y;
  local_ac.m_vecReportedPosition.x = local_2c - vecSrc->z;
  local_ac.m_iDamageStats._1_1_ =
       local_ac.m_vecDamagePosition.y * local_ac.m_vecDamagePosition.y +
       local_ac.m_vecDamagePosition.z * local_ac.m_vecDamagePosition.z +
       local_ac.m_vecReportedPosition.x * local_ac.m_vecReportedPosition.x != 0.0;
  local_ac.m_flBaseDamage = 0.0;
  local_ac.m_flMaxDamage = 0.0;
  local_ac.m_flDamage = 0.0;
  local_ac.m_iDamageCustom = 0;
  local_ac.m_iDamageStats._0_1_ = 1;
  local_ac.m_hAttacker.super_CBaseHandle.m_Index = (CBaseHandle)0;
  local_ac.m_hInflictor.super_CBaseHandle.m_Index = (CBaseHandle)0;
  local_ac.m_vecReportedPosition.z = 0.0;
  CTraceFilterSimple::CTraceFilterSimple(&local_50,(IHandleEntity *)this,0,(ShouldHitFunc_t)0x0);
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4eb2bb) + 0x14))
            ((int *)**(undefined4 **)(unaff_EBX + 0x4eb2bb),&local_ac,0x46004003,&local_50,
             &local_100);
  if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x4eb2b7) + 0x1c) + 0x30) != 0) {
    in_stack_fffffecc = (CTakeDamageInfo *)0xbf800000;
    DebugDrawLine((Vector *)&local_100,&local_100.super_CBaseTrace.endpos,0xff,0,0,true,-1.0);
  }
  if (*(float *)(unaff_EBX + 0x3699bf) != local_100.super_CBaseTrace.fraction) {
    if (local_100.m_pEnt != (CBaseEntity *)0x0) {
      ClearMultiDamage();
      if (((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
           m_iEFlags & 0x800) != 0) {
        CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
      }
                    /* Unresolved local var: Vector res@[???] */
      local_50.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter =
           (ITraceFilter)
           (local_100.super_CBaseTrace.endpos.x -
           (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
           m_vecAbsOrigin.x);
      local_50.m_pPassEnt =
           (IHandleEntity *)
           (local_100.super_CBaseTrace.endpos.y -
           (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
           m_vecAbsOrigin.y);
      local_50.m_collisionGroup =
           (int)(local_100.super_CBaseTrace.endpos.z -
                (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                m_vecAbsOrigin.z);
      local_40 = (_func_int_varargs **)
                 local_50.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter;
      local_3c = local_50.m_pPassEnt;
      local_38 = (float)local_50.m_collisionGroup;
      VectorNormalize((Vector *)&local_50);
      pAttacker_1 = *(CBaseEntity **)(**(int **)(unaff_EBX + 0x4eb24f) + 0xc);
      fVar2 = CBaseEntity::GetLastThink((CBaseEntity *)this,(char *)0x0);
      CTakeDamageInfo::CTakeDamageInfo
                (&local_ac,(CBaseEntity *)this,(CBaseEntity *)this,
                 ((float)pAttacker_1 - fVar2) * *(float *)(unaff_EBX + 0x369d4f),0x400,0,
                 in_stack_fffffecc,in_stack_fffffed0,pAttacker_1,in_stack_fffffed8,in_stack_fffffedc
                 ,in_stack_fffffee0);
      fVar2 = 0.0;
      if (this->m_bCanPushPlayer != false) {
        fVar2 = *(float *)(unaff_EBX + 0x36aef3);
      }
      CalculateMeleeDamageForce
                (&local_ac,(Vector *)&local_50,&local_100.super_CBaseTrace.endpos,fVar2);
      CBaseEntity::DispatchTraceAttack(local_100.m_pEnt,&local_ac,(Vector *)&local_50,&local_100);
      ApplyMultiDamage();
      UTIL_DecalTrace(&local_100,(char *)(unaff_EBX + 0x34eaf5));
      iVar1 = (*((local_100.m_pEnt)->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                _vptr_IHandleEntity[0x55])(local_100.m_pEnt);
      if ((char)iVar1 != '\0') {
        CBaseEntity::EmitSound(local_100.m_pEnt,(char *)(unaff_EBX + 0x34eb01),0.0,(float *)0x0);
      }
    }
    local_11c = &local_100.super_CBaseTrace.endpos;
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4eb383) + 0x10))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4eb383),local_11c,1,1,0);
  }
  return;
}


/* CNPC_HoverTurret::OnBurning at 006c1690 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: bool OnBurning(CNPC_HoverTurret * this) */

bool __thiscall CNPC_HoverTurret::OnBurning(CNPC_HoverTurret *this)

{
  int iVar1;
  int iVar2;
  
  iVar2 = ((CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth> *)
          ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                 super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                 super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0xe8))->m_Value;
  iVar1 = iVar2 + -1;
  if (iVar2 != iVar1) {
    (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x200))
              (this,(CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth> *)
                    ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                           super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                           super_CBaseAnimating + 0xe8));
    ((CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth> *)
    ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0xe8))->m_Value =
         iVar1;
  }
  if (0 < iVar1) {
    return true;
  }
  Explode(this);
  return false;
}


/* CNPC_HoverTurret::Explode at 006c0d50 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void Explode(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::Explode(CNPC_HoverTurret *this)

{
  CBaseEntity *this_00;
  string_t *psVar1;
  undefined4 *puVar2;
  code *pcVar3;
  BASEPTR func;
  CBreakableProp *this_01;
  string_t *psVar4;
  string_t *psVar5;
  int iVar6;
  float *pfVar7;
  int iVar8;
  Vector *pVVar9;
  char *pcVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  CNPC_HoverTurret *pCVar13;
  int unaff_EBX;
  int iVar14;
  float fVar15;
  CTakeDamageInfo *in_stack_fffffbac;
  CBaseEntity *in_stack_fffffbb0;
  CBaseEntity *in_stack_fffffbb4;
  float in_stack_fffffbb8;
  int in_stack_fffffbbc;
  int in_stack_fffffbc0;
  CBaseEntity *local_3e0 [128];
  trace_t local_1e0;
  vec_t local_18c;
  vec_t local_188;
  vec_t local_184;
  float local_17c;
  float local_178;
  float local_174;
  undefined4 local_16c;
  undefined4 local_168;
  undefined4 local_164;
  undefined4 local_15c;
  undefined4 local_158;
  undefined4 local_154;
  undefined4 local_14c;
  undefined1 local_148;
  undefined1 local_147;
  CTakeDamageInfo local_138;
  CTakeDamageInfo local_e8;
  CRecipientFilter local_98;
  CFlaggedEntitiesEnum local_78;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  float local_58;
  float local_54;
  float local_50;
  float local_4c;
  float local_48;
  float local_44;
  Vector local_40;
  Vector local_34;
  Vector local_28 [2];
  
                    /* Unresolved local var: CBreakableProp * pFutbol@[???]
                       Unresolved local var: string_t strTurretName@[???]
                       Unresolved local var: string_t strPropName@[???]
                       Unresolved local var: Vector vecRadius@[???]
                       Unresolved local var: CBaseEntity *[128] pEnts@[???]
                       Unresolved local var: int nNumFound@[???]
                       Unresolved local var: CPVSFilter filter@[???]
                       Unresolved local var: Vector gibVelocity@[???]
                       Unresolved local var: int iModelIndex@[???] */
  ___i686_get_pc_thunk_bx();
  if ((((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
        super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
        super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) &&
     (CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this),
     ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
      super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
      super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0)) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
  pVVar9 = &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
            super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
            m_vecAbsOrigin;
  pCVar13 = (CNPC_HoverTurret *)pVVar9;
  this_01 = (CBreakableProp *)
            CBaseEntity::Create((char *)(unaff_EBX + 0x3505f5),pVVar9,
                                &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                 super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                 super_CBaseCombatCharacter.super_CBaseFlex.
                                 super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                                 m_angAbsRotation,(CBaseEntity *)0x0);
  if (this_01 != (CBreakableProp *)0x0) {
                    /* Unresolved local var: Vector vecVelocity@[???] */
    (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x248))(this,&local_78,0);
    CBaseEntity::SetAbsVelocity((CBaseEntity *)this_01,(Vector *)&local_78);
    CTakeDamageInfo::CTakeDamageInfo
              (&local_e8,(CBaseEntity *)this,(CBaseEntity *)this,200.0,0x40,0,in_stack_fffffbac,
               in_stack_fffffbb0,in_stack_fffffbb4,in_stack_fffffbb8,in_stack_fffffbbc,
               in_stack_fffffbc0);
    pCVar13 = this;
    CBreakableProp::Break(this_01,(CBaseEntity *)this,&local_e8);
  }
  psVar4 = AllocPooledString((string_t *)(unaff_EBX + 0x34b23f),(char *)pCVar13);
  psVar5 = AllocPooledString((string_t *)(unaff_EBX + 0x2fb82f),(char *)pCVar13);
  if (((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
       super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
                    /* Unresolved local var: Vector res@[???] */
  fVar15 = *(float *)(unaff_EBX + 0x36a9e7);
  local_28[0].x =
       (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
       super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.x;
  local_34.x = local_28[0].x + fVar15;
  local_28[0].y =
       (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
       super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.y;
  local_34.y = local_28[0].y + fVar15;
  local_28[0].z =
       (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
       super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z;
  local_34.z = local_28[0].z + fVar15;
  if (((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
       super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    local_28[0].x =
         (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
         super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
         super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.x;
    local_28[0].y =
         (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
         super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
         super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.y;
    local_28[0].z =
         (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
         super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
         super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z;
  }
                    /* Unresolved local var: Vector res@[???] */
  local_28[0].x = local_28[0].x - fVar15;
  local_28[0].y = local_28[0].y - fVar15;
  local_28[0].z = local_28[0].z - fVar15;
                    /* Unresolved local var: CFlaggedEntitiesEnum boxEnum@[???] */
  CFlaggedEntitiesEnum::CFlaggedEntitiesEnum(&local_78,local_3e0,0x80,0);
  iVar6 = UTIL_EntitiesInBox(local_28,&local_34,&local_78);
  if (0 < iVar6) {
    iVar14 = 0;
    do {
      pCVar13 = (CNPC_HoverTurret *)local_3e0[iVar14];
      if ((pCVar13 != (CNPC_HoverTurret *)0x0) && (pCVar13 != this)) {
        psVar1 = (string_t *)
                 (pCVar13->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                 super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                 super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                 m_iClassname.pszValue;
        if ((psVar4 != psVar1) && (psVar1 != psVar5)) {
          iVar8 = (*(*(_func_int_varargs ***)
                      &(pCVar13->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                       super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                       super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating)[0x55])
                            (pCVar13);
          if ((char)iVar8 == '\0') goto LAB_006c11b3;
          pCVar13 = (CNPC_HoverTurret *)local_3e0[iVar14];
        }
        pfVar7 = (float *)(*(*(_func_int_varargs ***)
                              &(pCVar13->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                               super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                               super_CBaseCombatCharacter.super_CBaseFlex.
                               super_CBaseAnimatingOverlay.super_CBaseAnimating)[0x9d])(pCVar13);
        if (((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
             m_iEFlags & 0x800) != 0) {
          CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
        }
        local_14c = 0;
        local_17c = *pfVar7 - (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                              super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay
                              .super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.x;
        local_178 = pfVar7[1] -
                    (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                    super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                    super_CBaseEntity.m_vecAbsOrigin.y;
        local_174 = pfVar7[2] -
                    (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                    super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                    super_CBaseEntity.m_vecAbsOrigin.z;
        local_147 = local_17c * local_17c + local_178 * local_178 + local_174 * local_174 != 0.0;
        local_154 = 0;
        local_158 = 0;
        local_15c = 0;
        local_148 = 1;
        local_164 = 0;
        local_168 = 0;
        local_16c = 0;
        local_18c = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                    super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                    super_CBaseEntity.m_vecAbsOrigin.x;
        local_188 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                    super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                    super_CBaseEntity.m_vecAbsOrigin.y;
        local_184 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                    super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                    super_CBaseEntity.m_vecAbsOrigin.z;
        CTraceFilterSimple::CTraceFilterSimple
                  ((CTraceFilterSimple *)&local_78,(IHandleEntity *)this,0,(ShouldHitFunc_t)0x0);
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4eaeab) + 0x14))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x4eaeab),&local_18c,0x400b,&local_78,
                   &local_1e0);
        if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x4eaea7) + 0x1c) + 0x30) != 0) {
          in_stack_fffffbac = (CTakeDamageInfo *)0xbf800000;
          DebugDrawLine((Vector *)&local_1e0,&local_1e0.super_CBaseTrace.endpos,0xff,0,0,true,-1.0);
        }
        if (*(float *)(unaff_EBX + 0x3695af) <= local_1e0.super_CBaseTrace.fraction) {
          CTakeDamageInfo::CTakeDamageInfo
                    (&local_138,(CBaseEntity *)this,(CBaseEntity *)this,50.0,0x400,0,
                     in_stack_fffffbac,in_stack_fffffbb0,in_stack_fffffbb4,in_stack_fffffbb8,
                     in_stack_fffffbbc,in_stack_fffffbc0);
          if (((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
               super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
               super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
               m_iEFlags & 0x800) != 0) {
            CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
          }
          local_138.m_vecDamagePosition.x =
               (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
               super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
               super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
               m_vecAbsOrigin.x;
          local_138.m_vecDamagePosition.y =
               (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
               super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
               super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
               m_vecAbsOrigin.y;
          local_138.m_vecDamagePosition.z =
               (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
               super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
               super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
               m_vecAbsOrigin.z;
          if ((((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                m_iEFlags & 0x800) != 0) &&
             (CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this),
             ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
              super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
              m_iEFlags & 0x800) != 0)) {
            CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
          }
          this_00 = local_3e0[iVar14];
          if ((this_00->m_iEFlags & 0x800) != 0) {
            CBaseEntity::CalcAbsolutePosition(this_00);
          }
                    /* Unresolved local var: Vector res@[???] */
          local_40.x = (this_00->m_vecAbsOrigin).x -
                       (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                       super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                       super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                       super_CBaseEntity.m_vecAbsOrigin.x;
          local_40.y = (this_00->m_vecAbsOrigin).y -
                       (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                       super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                       super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                       super_CBaseEntity.m_vecAbsOrigin.y;
          local_40.z = (this_00->m_vecAbsOrigin).z -
                       (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                       super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                       super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                       super_CBaseEntity.m_vecAbsOrigin.z;
          CalculateExplosiveDamageForce(&local_138,&local_40,pVVar9,1.0);
          CBaseEntity::DispatchTraceAttack
                    (local_3e0[iVar14],&local_138,*(Vector **)(unaff_EBX + 0x4eae4f),&local_1e0);
          ApplyMultiDamage();
        }
      }
LAB_006c11b3:
      iVar14 = iVar14 + 1;
    } while (iVar14 != iVar6);
  }
  pVVar9 = (Vector *)
           (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                 super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                 super_CBaseCombatCharacter.super_CBaseFlex.
                                 super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x274))(this);
  ExplosionCreate(pVVar9,*(QAngle **)(unaff_EBX + 0x4eaee3),(CBaseEntity *)this,200,100,0x529,0.0,
                  (CBaseEntity *)0x0,-1,(EHANDLE *)0x0,CLASS_NONE);
  pVVar9 = (Vector *)
           (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                 super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                 super_CBaseCombatCharacter.super_CBaseFlex.
                                 super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x274))(this);
  UTIL_ScreenShake(pVVar9,5.0,150.0,1.0,750.0,SHAKE_START,false,
                   (CUtlVector<CBasePlayer*,CUtlMemory<CBasePlayer*,_int>_> *)0x0);
  pVVar9 = (Vector *)
           (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                 super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                 super_CBaseCombatCharacter.super_CBaseFlex.
                                 super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x274))(this);
  CRecipientFilter::CRecipientFilter(&local_98);
  local_98.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(unaff_EBX + 0x4edba7);
  CRecipientFilter::AddRecipientsByPVS(&local_98,pVVar9);
                    /* Unresolved local var: Vector random@[???] */
  iVar6 = rand();
  local_58 = ((float)iVar6 / *(float *)(&DAT_003698a7 + unaff_EBX)) *
             *(float *)(unaff_EBX + 0x3698c3) + *(float *)(unaff_EBX + 0x369c4b);
  iVar6 = rand();
  local_54 = ((float)iVar6 / *(float *)(&DAT_003698a7 + unaff_EBX)) *
             *(float *)(unaff_EBX + 0x3698c3) + *(float *)(unaff_EBX + 0x369c4b);
  iVar6 = rand();
  local_50 = ((float)iVar6 / *(float *)(&DAT_003698a7 + unaff_EBX)) *
             *(float *)(unaff_EBX + 0x3698c3) + *(float *)(unaff_EBX + 0x369c4b);
  local_4c = local_58;
  local_48 = local_54;
  puVar2 = *(undefined4 **)(CTriggerWeaponDissolve::Activate + unaff_EBX + 7);
  pcVar3 = *(code **)(*(int *)*puVar2 + 0xc);
  local_44 = local_50;
  pcVar10 = CPropData::GetRandomChunkModel
                      (*(CPropData **)(unaff_EBX + 0x4eb157),(char *)(unaff_EBX + 0x2fb8f3),-1);
  uVar11 = (*pcVar3)(*puVar2,pcVar10);
  iVar6 = 0;
                    /* Unresolved local var: int i@[???] */
  do {
    pcVar3 = *(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4eaeeb) + 0x38);
    local_64 = 0x41800000;
    local_60 = 0x41800000;
    local_5c = 0x41800000;
    uVar12 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                   super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                   super_CBaseCombatCharacter.super_CBaseFlex.
                                   super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x274))(this);
    (*pcVar3)(**(undefined4 **)(unaff_EBX + 0x4eaeeb),&local_98,0,uVar12,
              *(undefined4 *)(unaff_EBX + 0x4eaee3),&local_64,&local_4c,uVar11,400,1,0x40200000,2);
    iVar6 = iVar6 + 1;
  } while (iVar6 != 0x10);
  CBaseEntity::AddEffects((CBaseEntity *)this,0x20);
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)*(uint *)(unaff_EBX + 0x4eaebb);
  CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
  fVar15 = *(float *)(unaff_EBX + 0x3695b3) +
           *(float *)(**(int **)(&DAT_004eae3f + unaff_EBX) + 0xc);
  CBaseEntity::SetNextThink((CBaseEntity *)this,fVar15,(char *)0x0);
  local_98.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(unaff_EBX + 0x4edba7);
  CRecipientFilter::~CRecipientFilter(&local_98,(int)fVar15);
  return;
}


/* CNPC_HoverTurret::CreateSmokeTrail at 006c1700 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void CreateSmokeTrail(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::CreateSmokeTrail(CNPC_HoverTurret *this)

{
  undefined1 *puVar1;
  vec_t *pvVar2;
  float fVar3;
  uint uVar4;
  CBaseEdict *pCVar5;
  int iVar6;
  SmokeTrail *this_00;
  IChangeInfoAccessor *pIVar7;
  ulong *puVar8;
  int unaff_EBX;
  
                    /* Unresolved local var: SmokeTrail * pSmokeTrail@[???] */
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
  uVar4 = (this->m_hSmokeTrail).super_CBaseHandle.m_Index;
  if ((((uVar4 == 0xffffffff) ||
       (iVar6 = (uVar4 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4ea496),
       *(uint *)(iVar6 + 8) != uVar4 >> 0x10)) || (*(int *)(iVar6 + 4) == 0)) &&
     (this_00 = SmokeTrail::CreateSmokeTrail(), this_00 != (SmokeTrail *)0x0)) {
    if (*(float *)(&DAT_00368f0e + unaff_EBX) != (this_00->m_SpawnRate).m_Value) {
      if ((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_TimerEvent.
          m_bRegistered == false) {
        pCVar5 = &((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if (pCVar5 != (CBaseEdict *)0x0) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
          pIVar7 = CBaseEdict::GetChangeAccessor(pCVar5);
          pIVar7->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this_00->m_SpawnRate).m_Value = 64.0;
    }
    if (*(float *)(unaff_EBX + 0x368df2) != (this_00->m_ParticleLifetime).m_Value) {
      if ((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_TimerEvent.
          m_bRegistered == false) {
        pCVar5 = &((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if (pCVar5 != (CBaseEdict *)0x0) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
          pIVar7 = CBaseEdict::GetChangeAccessor(pCVar5);
          pIVar7->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this_00->m_ParticleLifetime).m_Value = 0.5;
    }
    if (*(float *)(unaff_EBX + 0x36928e) != (this_00->m_StartSize).m_Value) {
      if ((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_TimerEvent.
          m_bRegistered == false) {
        pCVar5 = &((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if (pCVar5 != (CBaseEdict *)0x0) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
          pIVar7 = CBaseEdict::GetChangeAccessor(pCVar5);
          pIVar7->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this_00->m_StartSize).m_Value = 8.0;
    }
    if (*(float *)(unaff_EBX + 0x368e66) != (this_00->m_EndSize).m_Value) {
      if ((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_TimerEvent.
          m_bRegistered == false) {
        pCVar5 = &((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if (pCVar5 != (CBaseEdict *)0x0) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
          pIVar7 = CBaseEdict::GetChangeAccessor(pCVar5);
          pIVar7->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this_00->m_EndSize).m_Value = 16.0;
    }
    if (*(float *)(unaff_EBX + 0x368e66) != (this_00->m_SpawnRadius).m_Value) {
      if ((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_TimerEvent.
          m_bRegistered == false) {
        pCVar5 = &((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if (pCVar5 != (CBaseEdict *)0x0) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
          pIVar7 = CBaseEdict::GetChangeAccessor(pCVar5);
          pIVar7->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this_00->m_SpawnRadius).m_Value = 16.0;
    }
    if (*(float *)(CUtlRBTree<CUtlMap<CBaseEntity*,_AI_EnemyInfo_t*,_unsigned_char>::Node_t,unsigned_char,CUtlMap<CBaseEntity*,_AI_EnemyInfo_t*,_unsigned_char>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<CBaseEntity*,_AI_EnemyInfo_t*,_unsigned_char>::Node_t,_unsigned_char>,_unsigned_char>_>
                   ::RotateLeft + unaff_EBX + 6) != (this_00->m_MinSpeed).m_Value) {
      if ((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_TimerEvent.
          m_bRegistered == false) {
        pCVar5 = &((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if (pCVar5 != (CBaseEdict *)0x0) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
          pIVar7 = CBaseEdict::GetChangeAccessor(pCVar5);
          pIVar7->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this_00->m_MinSpeed).m_Value = 60.0;
    }
    if (*(float *)(unaff_EBX + 0x368f0a) != (this_00->m_MaxSpeed).m_Value) {
      if ((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_TimerEvent.
          m_bRegistered == false) {
        pCVar5 = &((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if (pCVar5 != (CBaseEdict *)0x0) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
          pIVar7 = CBaseEdict::GetChangeAccessor(pCVar5);
          pIVar7->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this_00->m_MaxSpeed).m_Value = 100.0;
    }
    fVar3 = *(float *)(unaff_EBX + 0x368df2);
    if (((fVar3 != (this_00->m_StartColor).
                   super_CNetworkVarBase<Vector,SmokeTrail::NetworkVar_m_StartColor>.m_Value.x) ||
        (fVar3 != (this_00->m_StartColor).
                  super_CNetworkVarBase<Vector,SmokeTrail::NetworkVar_m_StartColor>.m_Value.y)) ||
       (fVar3 != (this_00->m_StartColor).
                 super_CNetworkVarBase<Vector,SmokeTrail::NetworkVar_m_StartColor>.m_Value.z)) {
      if ((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_TimerEvent.
          m_bRegistered == false) {
        pCVar5 = &((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if (pCVar5 != (CBaseEdict *)0x0) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
          pIVar7 = CBaseEdict::GetChangeAccessor(pCVar5);
          pIVar7->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this_00->m_StartColor).super_CNetworkVarBase<Vector,SmokeTrail::NetworkVar_m_StartColor>.
      m_Value.x = 0.5;
      (this_00->m_StartColor).super_CNetworkVarBase<Vector,SmokeTrail::NetworkVar_m_StartColor>.
      m_Value.y = 0.5;
      (this_00->m_StartColor).super_CNetworkVarBase<Vector,SmokeTrail::NetworkVar_m_StartColor>.
      m_Value.z = 0.5;
    }
    if ((((this_00->m_EndColor).super_CNetworkVarBase<Vector,SmokeTrail::NetworkVar_m_EndColor>.
          m_Value.x != 0.0) ||
        (NAN((this_00->m_EndColor).super_CNetworkVarBase<Vector,SmokeTrail::NetworkVar_m_EndColor>.
             m_Value.x))) ||
       ((pvVar2 = &(this_00->m_EndColor).
                   super_CNetworkVarBase<Vector,SmokeTrail::NetworkVar_m_EndColor>.m_Value.y,
        *pvVar2 != 0.0 ||
        ((NAN(*pvVar2) ||
         ((this_00->m_EndColor).super_CNetworkVarBase<Vector,SmokeTrail::NetworkVar_m_EndColor>.
          m_Value.z != 0.0)))))) {
      if ((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_TimerEvent.
          m_bRegistered == false) {
        pCVar5 = &((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if (pCVar5 != (CBaseEdict *)0x0) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
          pIVar7 = CBaseEdict::GetChangeAccessor(pCVar5);
          pIVar7->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this_00->m_EndColor).super_CNetworkVarBase<Vector,SmokeTrail::NetworkVar_m_EndColor>.m_Value.
      x = 0.0;
      (this_00->m_EndColor).super_CNetworkVarBase<Vector,SmokeTrail::NetworkVar_m_EndColor>.m_Value.
      y = 0.0;
      (this_00->m_EndColor).super_CNetworkVarBase<Vector,SmokeTrail::NetworkVar_m_EndColor>.m_Value.
      z = 0.0;
    }
    CBaseParticleEntity::SetLifetime(&this_00->super_CBaseParticleEntity,-1.0);
    SmokeTrail::FollowEntity(this_00,(CBaseEntity *)this,(char *)0x0);
    puVar8 = (ulong *)(*(this_00->super_CBaseParticleEntity).super_CBaseEntity.super_IServerEntity.
                        super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[3])(this_00);
    (this->m_hSmokeTrail).super_CBaseHandle.m_Index = *puVar8;
  }
  return;
}


/* CNPC_HoverTurret::DestroySmokeTrail at 006bfac0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void DestroySmokeTrail(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::DestroySmokeTrail(CNPC_HoverTurret *this)

{
  uint uVar1;
  CBaseEntity *oldObj;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->m_hSmokeTrail).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4ec0d3),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (oldObj = *(CBaseEntity **)(iVar2 + 4), oldObj != (CBaseEntity *)0x0)) {
    UTIL_Remove(oldObj);
    (this->m_hSmokeTrail).super_CBaseHandle.m_Index = 0xffffffff;
    return;
  }
  return;
}


/* CNPC_HoverTurret::LoadSchedules at 006bea10 */

bool CNPC_HoverTurret::LoadSchedules(void)

{
  int *piVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CAI_BaseNPC::LoadSchedules();
  piVar1 = *(int **)(unaff_EBX + 0x4ed67c);
  if (*(int *)(&DAT_0062ec34 + unaff_EBX) != *piVar1) {
    InitCustomSchedules();
    (&DAT_0062ec30)[unaff_EBX] = 1;
    *(int *)(&DAT_0062ec34 + unaff_EBX) = *piVar1;
  }
  return (bool)(&DAT_0062ec30)[unaff_EBX];
}


/* CNPC_HoverTurret::LoadedSchedules at 006bea70 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: bool LoadedSchedules(CNPC_HoverTurret * this) */

bool __thiscall CNPC_HoverTurret::LoadedSchedules(CNPC_HoverTurret *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (bool)*(undefined1 *)(extraout_ECX + 0x62ebdc);
}


/* CNPC_HoverTurret::SquadSlotName at 006beb90 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: char * SquadSlotName(CNPC_HoverTurret * this, int slotEN) */

char * __thiscall CNPC_HoverTurret::SquadSlotName(CNPC_HoverTurret *this,int slotEN)

{
  int symbolID;
  char *pcVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  symbolID = CAI_LocalIdSpace::LocalToGlobal((CAI_LocalIdSpace *)(unaff_EBX + 0x670030),slotEN);
  pcVar1 = CAI_GlobalNamespace::IdToSymbol(*(CAI_GlobalNamespace **)(unaff_EBX + 0x4ed4b4),symbolID)
  ;
  return pcVar1;
}


/* CNPC_HoverTurret::CScheduleLoader::CScheduleLoader at 006bf750 */

/* DWARF original prototype: void CScheduleLoader(CScheduleLoader * this) */

void __thiscall CNPC_HoverTurret::CScheduleLoader::CScheduleLoader(CScheduleLoader *this)

{
  int *piVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CAI_BaseNPC::LoadSchedules();
  piVar1 = *(int **)(&DAT_004ec93c + unaff_EBX);
  if (*(int *)(unaff_EBX + 0x62def4) != *piVar1) {
    CNPC_HoverTurret::InitCustomSchedules();
    *(undefined1 *)(unaff_EBX + 0x62def0) = 1;
    *(int *)(unaff_EBX + 0x62def4) = *piVar1;
  }
  return;
}


/* CNPC_HoverTurret::CScheduleLoader::CScheduleLoader at 006bf7a0 */

/* DWARF original prototype: void CScheduleLoader(CScheduleLoader * this) */

void __thiscall CNPC_HoverTurret::CScheduleLoader::CScheduleLoader(CScheduleLoader *this)

{
  int *piVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CAI_BaseNPC::LoadSchedules();
  piVar1 = *(int **)(&DAT_004ec8ec + unaff_EBX);
  if (*(int *)(unaff_EBX + 0x62dea4) != *piVar1) {
    CNPC_HoverTurret::InitCustomSchedules();
    *(undefined1 *)(unaff_EBX + 0x62dea0) = 1;
    *(int *)(unaff_EBX + 0x62dea4) = *piVar1;
  }
  return;
}


/* CNPC_HoverTurret::InitCustomSchedules at 006c27d0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void CNPC_HoverTurret::InitCustomSchedules(void)

{
  CAI_GlobalNamespace *pCVar1;
  AI_NamespaceAddInfo_t *pAVar2;
  char *pcVar3;
  CStringRegistry *pCVar4;
  CStringRegistry *pCVar5;
  bool bVar6;
  int iVar7;
  undefined4 *puVar8;
  int unaff_EBX;
  int iVar9;
  CAI_GlobalNamespace *pGlobalNamespace;
  int local_b0;
  undefined1 *local_a8;
  int local_a4;
  undefined1 *local_9c;
  int local_98;
  undefined1 *local_90;
  int local_8c;
  undefined1 *local_84;
  int local_80;
  int local_7c;
  int local_78;
  int local_74;
  int local_70;
  CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t,_int>_> local_6c;
  CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t,_int>_> local_58;
  void *local_44;
  undefined4 local_40;
  int local_3c;
  CAI_GlobalNamespace *local_38;
  void *local_34;
  void *local_30;
  undefined4 local_2c;
  int local_28;
  CAI_GlobalNamespace *local_24;
  void *local_20;
  
                    /* Unresolved local var: char * pszClassName@[???]
                       Unresolved local var: CUtlVector<char*,CUtlMemory<char*,_int>_>
                       schedulesToLoad@[???]
                       Unresolved local var: CUtlVector<bool_(*)(),CUtlMemory<bool_(*)(),_int>_>
                       reqiredOthers@[???]
                       Unresolved local var: CAI_NamespaceInfos scheduleIds@[???]
                       Unresolved local var: CAI_NamespaceInfos taskIds@[???]
                       Unresolved local var: CAI_NamespaceInfos conditionIds@[???]
                       Unresolved local var: CAI_NamespaceInfos squadSlotIds@[???]
                       Unresolved local var: int i@[???] */
  ___i686_get_pc_thunk_bx();
  local_30 = (void *)0x0;
  local_2c = 0;
  local_28 = 0;
  local_24 = (CAI_GlobalNamespace *)0x0;
  local_20 = (void *)0x0;
  local_44 = (void *)0x0;
  local_40 = 0;
  local_3c = 0;
  local_38 = (CAI_GlobalNamespace *)0x0;
  local_34 = (void *)0x0;
  local_58.m_Memory.m_pMemory = (AI_NamespaceAddInfo_t *)0x0;
  local_58.m_Memory.m_nAllocationCount = 0;
  local_58.m_Memory.m_nGrowSize = 0;
  local_58.m_Size = 0;
  local_58.m_pElements = (AI_NamespaceAddInfo_t *)0x0;
  local_6c.m_Memory.m_pMemory = (AI_NamespaceAddInfo_t *)0x0;
  local_6c.m_Memory.m_nAllocationCount = 0;
  local_6c.m_Memory.m_nGrowSize = 0;
  local_6c.m_Size = 0;
  local_6c.m_pElements = (AI_NamespaceAddInfo_t *)0x0;
  *(int *)(&DAT_0066c39f + unaff_EBX) = unaff_EBX + 0x34cbea;
  iVar9 = *(int *)(&DAT_004e9877 + unaff_EBX);
  bVar6 = CAI_LocalIdSpace::Init
                    ((CAI_LocalIdSpace *)
                     (CNPC_Portal_FloorTurret::~CNPC_Portal_FloorTurret + unaff_EBX + 3),
                     *(CAI_GlobalNamespace **)(unaff_EBX + 0x4e986b),(CAI_LocalIdSpace *)(iVar9 + 4)
                    );
  if ((bVar6) &&
     (bVar6 = CAI_LocalIdSpace::Init
                        ((CAI_LocalIdSpace *)(unaff_EBX + 0x66c3bb),
                         (CAI_GlobalNamespace *)(*(int *)(unaff_EBX + 0x4e986b) + 8),
                         (CAI_LocalIdSpace *)(iVar9 + 0x1c)), bVar6)) {
    CAI_LocalIdSpace::Init
              ((CAI_LocalIdSpace *)(unaff_EBX + 0x66c3d3),
               (CAI_GlobalNamespace *)(*(int *)(unaff_EBX + 0x4e986b) + 0x10),
               (CAI_LocalIdSpace *)(iVar9 + 0x34));
  }
  pGlobalNamespace = *(CAI_GlobalNamespace **)(unaff_EBX + 0x4e986f);
  CAI_LocalIdSpace::Init
            ((CAI_LocalIdSpace *)(unaff_EBX + 0x66c3eb),pGlobalNamespace,
             *(CAI_LocalIdSpace **)(unaff_EBX + 0x4e9acf));
  if (1 < (int)local_24) {
    if (local_30 == (void *)0x0) {
      local_a8 = (undefined1 *)((int)&local_24[-1].m_NextGlobalBase + 3);
      do {
        if (0 < (int)local_a8) {
          local_b0 = 1;
          local_74 = 0;
          do {
            pCVar1 = (CAI_GlobalNamespace *)((int)local_30 + local_b0 * 8);
            pAVar2 = (AI_NamespaceAddInfo_t *)((int)local_30 + local_74);
            pGlobalNamespace = pCVar1;
            iVar9 = CAI_NamespaceInfos::Compare(pAVar2,(AI_NamespaceAddInfo_t *)pCVar1);
            if (iVar9 < 0) {
              iVar9 = pAVar2->localId;
              pCVar4 = (CStringRegistry *)pAVar2->pszName;
              pCVar5 = pCVar1->m_pSymbols;
              pAVar2->localId = pCVar1->m_NextGlobalBase;
              pAVar2->pszName = (char *)pCVar5;
              pCVar1->m_NextGlobalBase = iVar9;
              pCVar1->m_pSymbols = pCVar4;
            }
            local_b0 = local_b0 + 1;
            local_74 = local_74 + 8;
          } while (local_b0 <= (int)local_a8);
        }
        local_a8 = local_a8 + -1;
      } while (local_a8 != (undefined1 *)0xffffffff);
    }
    else {
      pGlobalNamespace = local_24;
      _qsort(local_30,(size_t)local_24,8,*(int **)(&DAT_00608b87 + unaff_EBX));
    }
  }
  if (1 < (int)local_38) {
    if (local_44 == (void *)0x0) {
      local_9c = (undefined1 *)((int)&local_38[-1].m_NextGlobalBase + 3);
      do {
        if (0 < (int)local_9c) {
          local_a4 = 1;
          local_78 = 0;
          do {
            pCVar1 = (CAI_GlobalNamespace *)((int)local_44 + local_a4 * 8);
            pAVar2 = (AI_NamespaceAddInfo_t *)((int)local_44 + local_78);
            pGlobalNamespace = pCVar1;
            iVar9 = CAI_NamespaceInfos::Compare(pAVar2,(AI_NamespaceAddInfo_t *)pCVar1);
            if (iVar9 < 0) {
              iVar9 = pAVar2->localId;
              pCVar4 = (CStringRegistry *)pAVar2->pszName;
              pCVar5 = pCVar1->m_pSymbols;
              pAVar2->localId = pCVar1->m_NextGlobalBase;
              pAVar2->pszName = (char *)pCVar5;
              pCVar1->m_NextGlobalBase = iVar9;
              pCVar1->m_pSymbols = pCVar4;
            }
            local_a4 = local_a4 + 1;
            local_78 = local_78 + 8;
          } while (local_a4 <= (int)local_9c);
        }
        local_9c = local_9c + -1;
      } while (local_9c != (undefined1 *)0xffffffff);
    }
    else {
      pGlobalNamespace = local_38;
      _qsort(local_44,(size_t)local_38,8,*(int **)(&DAT_00608b87 + unaff_EBX));
    }
  }
  if (1 < local_58.m_Size) {
    if (local_58.m_Memory.m_pMemory == (AI_NamespaceAddInfo_t *)0x0) {
      local_90 = (undefined1 *)
                 ((int)&((CAI_GlobalNamespace *)(local_58.m_Size + -8))->m_NextGlobalBase + 3);
      do {
        if (0 < (int)local_90) {
          local_98 = 1;
          local_7c = 0;
          do {
            pCVar1 = (CAI_GlobalNamespace *)(local_58.m_Memory.m_pMemory + local_98);
            pAVar2 = (AI_NamespaceAddInfo_t *)
                     ((int)&(local_58.m_Memory.m_pMemory)->pszName + local_7c);
            pGlobalNamespace = pCVar1;
            iVar9 = CAI_NamespaceInfos::Compare(pAVar2,(AI_NamespaceAddInfo_t *)pCVar1);
            if (iVar9 < 0) {
              iVar9 = pAVar2->localId;
              pCVar4 = (CStringRegistry *)pAVar2->pszName;
              pCVar5 = pCVar1->m_pSymbols;
              pAVar2->localId = pCVar1->m_NextGlobalBase;
              pAVar2->pszName = (char *)pCVar5;
              pCVar1->m_NextGlobalBase = iVar9;
              pCVar1->m_pSymbols = pCVar4;
            }
            local_98 = local_98 + 1;
            local_7c = local_7c + 8;
          } while (local_98 <= (int)local_90);
        }
        local_90 = local_90 + -1;
      } while (local_90 != (undefined1 *)0xffffffff);
    }
    else {
      pGlobalNamespace = (CAI_GlobalNamespace *)local_58.m_Size;
      _qsort(local_58.m_Memory.m_pMemory,local_58.m_Size,8,*(int **)(&DAT_00608b87 + unaff_EBX));
    }
  }
  if (1 < local_6c.m_Size) {
    if (local_6c.m_Memory.m_pMemory == (AI_NamespaceAddInfo_t *)0x0) {
      local_84 = (undefined1 *)
                 ((int)&((CAI_GlobalNamespace *)(local_6c.m_Size + -8))->m_NextGlobalBase + 3);
      do {
        if (0 < (int)local_84) {
          local_8c = 1;
          local_80 = 0;
          do {
            pCVar1 = (CAI_GlobalNamespace *)(local_6c.m_Memory.m_pMemory + local_8c);
            pAVar2 = (AI_NamespaceAddInfo_t *)
                     ((int)&(local_6c.m_Memory.m_pMemory)->pszName + local_80);
            pGlobalNamespace = pCVar1;
            iVar9 = CAI_NamespaceInfos::Compare(pAVar2,(AI_NamespaceAddInfo_t *)pCVar1);
            if (iVar9 < 0) {
              iVar9 = pAVar2->localId;
              pCVar4 = (CStringRegistry *)pAVar2->pszName;
              pCVar5 = pCVar1->m_pSymbols;
              pAVar2->localId = pCVar1->m_NextGlobalBase;
              pAVar2->pszName = (char *)pCVar5;
              pCVar1->m_NextGlobalBase = iVar9;
              pCVar1->m_pSymbols = pCVar4;
            }
            local_8c = local_8c + 1;
            local_80 = local_80 + 8;
          } while (local_8c <= (int)local_84);
        }
        local_84 = local_84 + -1;
      } while (local_84 != (undefined1 *)0xffffffff);
    }
    else {
      pGlobalNamespace = (CAI_GlobalNamespace *)local_6c.m_Size;
      _qsort(local_6c.m_Memory.m_pMemory,local_6c.m_Size,8,*(int **)(&DAT_00608b87 + unaff_EBX));
    }
  }
  if (0 < (int)local_24) {
    iVar9 = 0;
    do {
      puVar8 = (undefined4 *)(iVar9 * 8 + (int)local_30);
      pGlobalNamespace = (CAI_GlobalNamespace *)*puVar8;
      bVar6 = CAI_LocalIdSpace::AddSymbol
                        ((CAI_LocalIdSpace *)
                         (CNPC_Portal_FloorTurret::~CNPC_Portal_FloorTurret + unaff_EBX + 3),
                         (char *)pGlobalNamespace,puVar8[1],(char *)(unaff_EBX + 0x33206b),
                         *(char **)(&DAT_0060817b + unaff_EBX));
      if (!bVar6) goto LAB_006c2a43;
      iVar9 = iVar9 + 1;
    } while (iVar9 < (int)local_24);
  }
  if (0 < (int)local_38) {
    iVar9 = 0;
    do {
      puVar8 = (undefined4 *)(iVar9 * 8 + (int)local_44);
      pGlobalNamespace = (CAI_GlobalNamespace *)*puVar8;
      bVar6 = CAI_LocalIdSpace::AddSymbol
                        ((CAI_LocalIdSpace *)(unaff_EBX + 0x66c3bb),(char *)pGlobalNamespace,
                         puVar8[1],&UNK_00306b4b + unaff_EBX,*(char **)(&DAT_0060817b + unaff_EBX));
      if (!bVar6) goto LAB_006c2a43;
      iVar9 = iVar9 + 1;
    } while (iVar9 < (int)local_38);
  }
  if (0 < local_58.m_Size) {
    local_70 = 0;
    iVar9 = *(int *)(unaff_EBX + 0x4e986b);
    do {
      while( true ) {
        pcVar3 = local_58.m_Memory.m_pMemory[local_70].pszName;
        iVar7 = CAI_GlobalNamespace::NextGlobalBase((CAI_GlobalNamespace *)(iVar9 + 0x10));
        if (0xff < iVar7 + -1000000000) break;
        pGlobalNamespace = (CAI_GlobalNamespace *)local_58.m_Memory.m_pMemory[local_70].pszName;
        bVar6 = CAI_LocalIdSpace::AddSymbol
                          ((CAI_LocalIdSpace *)(unaff_EBX + 0x66c3d3),(char *)pGlobalNamespace,
                           local_58.m_Memory.m_pMemory[local_70].localId,&UNK_0030dbe3 + unaff_EBX,
                           *(char **)(&DAT_0060817b + unaff_EBX));
        if (!bVar6) goto LAB_006c2a43;
        local_70 = local_70 + 1;
        if (local_58.m_Size <= local_70) goto LAB_006c2d45;
      }
      pGlobalNamespace = (CAI_GlobalNamespace *)&section_000000dc.size;
      DevWarning((char *)(unaff_EBX + 0x301603),0x100,pcVar3);
      local_70 = local_70 + 1;
    } while (local_70 < local_58.m_Size);
  }
LAB_006c2d45:
  if (0 < local_6c.m_Size) {
    iVar9 = 0;
    do {
      pGlobalNamespace = (CAI_GlobalNamespace *)local_6c.m_Memory.m_pMemory[iVar9].pszName;
      bVar6 = CAI_LocalIdSpace::AddSymbol
                        ((CAI_LocalIdSpace *)(unaff_EBX + 0x66c3eb),(char *)pGlobalNamespace,
                         local_6c.m_Memory.m_pMemory[iVar9].localId,(char *)(unaff_EBX + 0x31785e),
                         *(char **)(&DAT_0060817b + unaff_EBX));
      if (!bVar6) goto LAB_006c2a43;
      iVar9 = iVar9 + 1;
    } while (iVar9 < local_6c.m_Size);
  }
  CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t,_int>_>::~CUtlVector
            (&local_6c,(int)pGlobalNamespace);
  CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t,_int>_>::~CUtlVector
            (&local_58,(int)pGlobalNamespace);
  local_38 = (CAI_GlobalNamespace *)0x0;
  if (-1 < local_3c) {
    if (local_44 != (void *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e93b7) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4e93b7),local_44);
      local_44 = (void *)0x0;
    }
    local_40 = 0;
  }
  local_34 = local_44;
  if (-1 < local_3c) {
    if (local_44 != (void *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e93b7) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4e93b7),local_44);
      local_44 = (void *)0x0;
    }
    local_40 = 0;
  }
  local_24 = (CAI_GlobalNamespace *)0x0;
  if (local_28 < 0) goto LAB_006c2bbe;
  if (local_30 != (void *)0x0) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e93b7) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4e93b7),local_30);
    goto LAB_006c2bb0;
  }
LAB_006c2bb7:
  local_2c = 0;
LAB_006c2bbe:
  local_20 = local_30;
  if ((-1 < local_28) && (local_30 != (void *)0x0)) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e93b7) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4e93b7),local_30);
  }
  return;
LAB_006c2a43:
  local_6c.m_Size = 0;
  if (-1 < local_6c.m_Memory.m_nGrowSize) {
    if (local_6c.m_Memory.m_pMemory != (AI_NamespaceAddInfo_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e93b7) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4e93b7),local_6c.m_Memory.m_pMemory);
      local_6c.m_Memory.m_pMemory = (AI_NamespaceAddInfo_t *)0x0;
    }
    local_6c.m_Memory.m_nAllocationCount = 0;
  }
  local_6c.m_pElements = local_6c.m_Memory.m_pMemory;
  if (-1 < local_6c.m_Memory.m_nGrowSize) {
    if (local_6c.m_Memory.m_pMemory != (AI_NamespaceAddInfo_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e93b7) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4e93b7),local_6c.m_Memory.m_pMemory);
      local_6c.m_Memory.m_pMemory = (AI_NamespaceAddInfo_t *)0x0;
    }
    local_6c.m_Memory.m_nAllocationCount = 0;
  }
  local_58.m_Size = 0;
  if (-1 < local_58.m_Memory.m_nGrowSize) {
    if (local_58.m_Memory.m_pMemory != (AI_NamespaceAddInfo_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e93b7) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4e93b7),local_58.m_Memory.m_pMemory);
      local_58.m_Memory.m_pMemory = (AI_NamespaceAddInfo_t *)0x0;
    }
    local_58.m_Memory.m_nAllocationCount = 0;
  }
  local_58.m_pElements = local_58.m_Memory.m_pMemory;
  if (-1 < local_58.m_Memory.m_nGrowSize) {
    if (local_58.m_Memory.m_pMemory != (AI_NamespaceAddInfo_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e93b7) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4e93b7),local_58.m_Memory.m_pMemory);
      local_58.m_Memory.m_pMemory = (AI_NamespaceAddInfo_t *)0x0;
    }
    local_58.m_Memory.m_nAllocationCount = 0;
  }
  local_38 = (CAI_GlobalNamespace *)0x0;
  if (-1 < local_3c) {
    if (local_44 != (void *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e93b7) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4e93b7),local_44);
      local_44 = (void *)0x0;
    }
    local_40 = 0;
  }
  local_34 = local_44;
  if (-1 < local_3c) {
    if (local_44 != (void *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e93b7) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4e93b7),local_44);
      local_44 = (void *)0x0;
    }
    local_40 = 0;
  }
  local_24 = (CAI_GlobalNamespace *)0x0;
  if (local_28 < 0) goto LAB_006c2bbe;
  if (local_30 != (void *)0x0) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e93b7) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4e93b7),local_30);
LAB_006c2bb0:
    local_30 = (void *)0x0;
  }
  goto LAB_006c2bb7;
}


/* _GLOBAL__I_sk_hover_turret_health at 000c1d80 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_sk_hover_turret_health(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

