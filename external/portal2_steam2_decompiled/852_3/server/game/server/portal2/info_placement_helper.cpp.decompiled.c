/* DWARF-guided pseudocode for game/server/portal2/info_placement_helper.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000bfeb0 */

/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *s2;
  undefined4 *puVar1;
  IEntityFactoryDictionary *pIVar2;
  datamap_t *pdVar3;
  int iVar4;
  int iVar5;
  int unaff_EBX;
  undefined4 *puVar6;
  undefined4 *puVar7;
  longlong lVar8;
  ConVar *in_stack_ffffffc8;
  char *in_stack_ffffffcc;
  char *in_stack_ffffffd0;
  int in_stack_ffffffd4;
  SendTable *this;
  char *in_stack_ffffffd8;
  
  lVar8 = ___i686_get_pc_thunk_bx();
  if (lVar8 == 0xffff00000001) {
    (&r_AirboatViewZHeight.m_bHasMax)[unaff_EBX] = false;
    (&r_AirboatViewZHeight.field_0x3d)[unaff_EBX] = 0;
    (&r_AirboatViewZHeight.field_0x3e)[unaff_EBX] = 0;
    (&r_AirboatViewZHeight.field_0x3f)[unaff_EBX] = 0;
    *(undefined4 *)((int)&r_AirboatViewZHeight.m_fMaxVal + unaff_EBX) = 0;
    *(undefined4 *)((int)&r_AirboatViewZHeight.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) =
         0;
    *(undefined4 *)
     ((int)&r_AirboatViewZHeight.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&r_AirboatViewZHeight.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX)
         = 0x7f7fffff;
    *(undefined4 *)((int)&r_AirboatViewZHeight.m_fnChangeCallbacks.m_Size + unaff_EBX) = 0;
    *(undefined4 *)((int)&r_AirboatViewZHeight.m_fnChangeCallbacks.m_pElements + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00cf6ff8 + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00cf6ffc + unaff_EBX) = 0;
    *(undefined4 *)((int)&anim_showstate.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&anim_showstate.super_ConCommandBase.m_pNext + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&anim_showstate.super_ConCommandBase.m_bRegistered + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&anim_showstate.super_ConCommandBase.m_pszName + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&anim_showstate.super_ConCommandBase.m_pszHelpString + unaff_EBX) = 0;
    *(undefined1 *)((int)&anim_showstate.super_ConCommandBase.m_nFlags + unaff_EBX + 3) = 0;
    *(undefined1 *)((int)&anim_showstate.super_ConCommandBase.m_nFlags + unaff_EBX) = 0;
    *(undefined1 *)((int)&anim_showstate.super_ConCommandBase.m_nFlags + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&anim_showstate.super_ConCommandBase.m_nFlags + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&anim_showstate.super_IConVar._vptr_IConVar + unaff_EBX) = 0;
    *(undefined1 *)((int)&anim_showstate.m_pParent + unaff_EBX + 3) = 0;
    *(undefined1 *)((int)&anim_showstate.m_pParent + unaff_EBX) = 0;
    *(undefined1 *)((int)&anim_showstate.m_pParent + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&anim_showstate.m_pParent + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&anim_showstate.m_pszDefaultValue + unaff_EBX) = 0;
    *(undefined1 *)((int)&anim_showstate.m_Value.m_pszString + unaff_EBX + 3) = 1;
    *(undefined1 *)((int)&anim_showstate.m_Value.m_pszString + unaff_EBX) = 0;
    *(undefined1 *)((int)&anim_showstate.m_Value.m_pszString + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&anim_showstate.m_Value.m_pszString + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&anim_showstate.m_Value.m_StringLength + unaff_EBX) = 1;
    *(undefined1 *)((int)&anim_showstate.m_Value.m_fValue + unaff_EBX + 3) = 0;
    *(undefined1 *)((int)&anim_showstate.m_Value.m_fValue + unaff_EBX) = 0;
    *(undefined1 *)((int)&anim_showstate.m_Value.m_fValue + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&anim_showstate.m_Value.m_fValue + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&anim_showstate.m_Value.m_nValue + unaff_EBX) = 2;
    (&anim_showstate.field_0x37)[unaff_EBX] = 0;
    (&anim_showstate.m_bHasMin)[unaff_EBX] = false;
    (&anim_showstate.field_0x35)[unaff_EBX] = 0;
    (&anim_showstate.field_0x36)[unaff_EBX] = 0;
    *(undefined **)((int)&anim_showstate.m_fMinVal + unaff_EBX) = &UNK_00af1464 + unaff_EBX;
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<CBaseCombatWeapon>::dataDesc[0x1b].flatOffset + unaff_EBX + 8),
                   (char *)(vgui::TextEntry::GetStartDrawIndex + unaff_EBX + 4),
                   (char *)(unaff_EBX + 0x8ebd74),0,(char *)(unaff_EBX + 0x94f01c),in_stack_ffffffc8
                   ,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
    ___cxa_atexit(unaff_EBX + 0x8d9acc,0,*(undefined4 *)(&DAT_00aebccc + unaff_EBX));
    *(undefined **)(&anim_showstate.m_bHasMax + unaff_EBX) = &UNK_00b718ec + unaff_EBX;
    pIVar2 = EntityFactoryDictionary();
    (**pIVar2->_vptr_IEntityFactoryDictionary)(pIVar2,unaff_EBX + 0xcf703c,unaff_EBX + 0x94e922);
    pdVar3 = DataMapInit<CInfoPlacementHelper>((CInfoPlacementHelper *)0x0);
    *(datamap_t **)
     ((int)DataMapInit<CBaseCombatWeapon>::dataDesc[0x1d].flatOffset + unaff_EBX + -0x20) = pdVar3;
    iVar5 = unaff_EBX + 0xcf7040;
    s2 = (char *)(unaff_EBX + 0x94efb7);
    *(char **)((int)&anim_showstate.m_fMaxVal + unaff_EBX) = s2;
    this = (SendTable *)
           ((int)DataMapInit<CBaseCombatWeapon>::dataDesc[0x1d].flatOffset + unaff_EBX + -0x1c);
    *(SendTable **)((int)&anim_showstate.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) = this;
    *(undefined4 *)((int)&anim_showstate.m_fnChangeCallbacks.m_Size + unaff_EBX) = 0xffff;
    puVar7 = (undefined4 *)**(int **)(&DAT_00aebd6c + unaff_EBX);
    if (puVar7 == (undefined4 *)0x0) {
      **(int **)(&DAT_00aebd6c + unaff_EBX) = iVar5;
      *(undefined4 *)
       ((int)&anim_showstate.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) = 0;
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
        *(undefined4 *)(iVar5 + 8) = **(undefined4 **)(&DAT_00aebd6c + unaff_EBX);
        **(int **)(&DAT_00aebd6c + unaff_EBX) = iVar5;
      }
    }
    SendTable::SendTable(this);
    ___cxa_atexit(unaff_EBX + 0x8d9aac,0,*(undefined4 *)(&DAT_00aebccc + unaff_EBX));
    iVar5 = ServerClassInit<DT_InfoPlacementHelper::ignored>((ignored *)0x0);
    *(int *)((int)DataMapInit<CBaseCombatWeapon>::dataDesc[0x1d].flatOffset + unaff_EBX + -8) =
         iVar5;
    CAutoGameSystem::CAutoGameSystem((CAutoGameSystem *)(&DAT_00cf705c + unaff_EBX),(char *)0x0);
    *(undefined **)(&DAT_00cf705c + unaff_EBX) = &UNK_00b71924 + unaff_EBX;
    *(undefined4 *)(&anim_showstatelog.super_ConCommandBase.m_bRegistered + unaff_EBX) = 0;
    *(undefined4 *)((int)&anim_showstatelog.super_ConCommandBase.m_pszName + unaff_EBX) = 0;
    *(undefined4 *)((int)&anim_showstatelog.super_ConCommandBase.m_pszHelpString + unaff_EBX) = 0;
    *(undefined4 *)((int)&anim_showstatelog.super_ConCommandBase.m_nFlags + unaff_EBX) = 0;
    *(undefined4 *)((int)&anim_showstatelog.super_IConVar._vptr_IConVar + unaff_EBX) = 0;
    ___cxa_atexit(unaff_EBX + 0x8d9bcc,0,*(undefined4 *)(&DAT_00aebccc + unaff_EBX));
  }
  return;
}


/* __tcf_2 at 00999990 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x394f48),in_stack_00000008);
  return;
}


/* DataMapInit<CInfoPlacementHelper> at 000bfd80 */

datamap_t * DataMapInit<CInfoPlacementHelper>(CInfoPlacementHelper *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((&nextlevel.m_bHasMin)[unaff_EBX] == false) {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcf7514);
    if (iVar2 != 0) {
      *(int *)((int)&nextlevel.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) =
           unaff_EBX + 0x94f0e7;
      *(undefined4 *)((int)&nextlevel.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) = 0;
      *(undefined4 *)((int)&nextlevel.m_fnChangeCallbacks.m_Size + unaff_EBX) = 0;
      *(undefined4 *)((int)&nextlevel.m_fnChangeCallbacks.m_pElements + unaff_EBX) = 0;
      *(undefined4 *)
       ((int)&CMultiplayRules::m_MapList.super_CUtlVectorAutoPurge<char*>.
              super_CUtlVector<char*,CUtlMemory<char*,_int>_>.m_Memory.m_pMemory + unaff_EBX) = 0;
      *(undefined4 *)
       ((int)&CMultiplayRules::m_MapList.super_CUtlVectorAutoPurge<char*>.
              super_CUtlVector<char*,CUtlMemory<char*,_int>_>.m_Memory.m_nAllocationCount +
       unaff_EBX) = 0;
      *(undefined4 *)((int)&nextlevel.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) =
           0x14;
      ___cxa_guard_release(unaff_EBX + 0xcf7514);
      ___cxa_atexit(unaff_EBX + 0x8d9c1c,0,*(undefined4 *)(&DAT_00aebdfc + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_GetDataDescMap_00c0a2d8 + unaff_EBX) =
       *(undefined4 *)(&DAT_00aebf38 + unaff_EBX);
  if ((&nextlevel.m_bHasMax)[unaff_EBX] == false) {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcf751c);
    if (iVar2 != 0) {
      uVar1 = **(undefined4 **)(&DAT_00aebe18 + unaff_EBX);
      *(undefined4 *)(&CGamePlayerEquip::vtable + unaff_EBX) = uVar1;
      *(undefined4 *)((int)&PTR_GetAIAddOn_00c0a600 + unaff_EBX) = uVar1;
      ___cxa_guard_release(unaff_EBX + 0xcf751c);
    }
  }
  *(undefined4 *)((int)&PTR_GetServerClass_00c0a2d0 + unaff_EBX) = 0xc;
  *(int *)((int)&PTR_SetModelIndex_00c0a2cc + unaff_EBX) = (int)&PTR_KeyValue_00c0a32c + unaff_EBX;
  return (datamap_t *)((int)&PTR_SetModelIndex_00c0a2cc + unaff_EBX);
}


/* CInfoPlacementHelper::GetDataDescMap at 006b9a10 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CInfoPlacementHelper * this) */

datamap_t * __thiscall CInfoPlacementHelper::GetDataDescMap(CInfoPlacementHelper *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x610648);
}


/* CInfoPlacementHelper::GetBaseMap at 006b9a20 */

datamap_t * CInfoPlacementHelper::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4f22a4);
}


/* __tcf_0 at 009999b0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41d90e)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41d902) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41d90e));
  }
  *(undefined4 *)(unaff_EBX + 0x41d90e) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41d90a)) {
    if (*(int *)(unaff_EBX + 0x41d902) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2121da) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2121da),*(int *)(unaff_EBX + 0x41d902));
      *(undefined4 *)(unaff_EBX + 0x41d902) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d906) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41d902);
  *(int *)(unaff_EBX + 0x41d912) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41d90a)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2121da) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2121da),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41d902) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d906) = 0;
  }
  return;
}


/* ServerClassInit<DT_InfoPlacementHelper::ignored> at 000bf9d0 */

int ServerClassInit<DT_InfoPlacementHelper::ignored>(ignored *param_1)

{
  SendVarProxyFn_conflict varProxy;
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&sv_pushaway_force.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX + 2)
      == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf7562);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)((int)&sv_pushaway_force.m_pszDefaultValue + unaff_EBX + 2),
                  &UNK_008f3046 + unaff_EBX,0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&sv_pushaway_min_player_speed.super_ConCommandBase.m_nFlags +
                        unaff_EBX + 2),(char *)(unaff_EBX + 0x8f53b6),0,
                        (SendTable *)**(undefined4 **)(&DAT_00aec2aa + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00aec24e + unaff_EBX),0x80);
      varProxy = *(SendVarProxyFn_conflict *)(&DAT_00aec48a + unaff_EBX);
      SendPropString((SendProp_conflict *)
                     (&sv_pushaway_max_force.super_ConCommandBase.field_0xa + unaff_EBX),
                     (char *)(unaff_EBX + 0x94f43a),0x3f0,4,0,varProxy,0x80);
      SendPropString((SendProp_conflict *)(unaff_EBX + 0xcf767e),(char *)(unaff_EBX + 0x94f44b),
                     0x3f4,4,0,varProxy,0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&sv_pushaway_clientside.m_fnChangeCallbacks.m_Size + unaff_EBX + 2),
                    (char *)(unaff_EBX + 0x91b236),0x3f8,4,0x20,0,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00aec2a6 + unaff_EBX),0x80);
      SendPropBool((SendProp_conflict *)
                   ((int)&sv_pushaway_player_force.m_fnChangeCallbacks.m_Memory.m_pMemory +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0x94f460),0x3fc,1);
      SendPropBool((SendProp_conflict *)
                   ((int)&sv_pushaway_max_player_force.m_fMinVal + unaff_EBX + 2),
                   (char *)(unaff_EBX + 0x94f476),0x3fd,1);
      SendPropBool((SendProp_conflict *)((int)&particle_test_file.m_Value.m_fValue + unaff_EBX + 2),
                   &UNK_0092d016 + unaff_EBX,0x3fe,1);
      SendPropFloat((SendProp_conflict *)
                    ((int)&particle_test_attach_mode.m_pszDefaultValue + unaff_EBX + 2),
                    &UNK_009356ee + unaff_EBX,0x400,4,0x20,0,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00aec2a6 + unaff_EBX),0x80);
      SendPropBool((SendProp_conflict *)
                   ((int)&particle_test_attach_attachment.super_ConCommandBase.m_nFlags +
                   unaff_EBX + 2),&UNK_0094f488 + unaff_EBX,0x404,1);
      ___cxa_guard_release(unaff_EBX + 0xcf7562);
      ___cxa_atexit(unaff_EBX + 0x8d9ed2,0,*(undefined4 *)(&DAT_00aec1b2 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)
             ((int)DataMapInit<CBaseCombatWeapon>::dataDesc[0x30].flatOffset + unaff_EBX + 10),
             (SendProp_conflict *)
             ((int)&sv_pushaway_min_player_speed.super_ConCommandBase.m_nFlags + unaff_EBX + 2),9,
             *(char **)(&DAT_00b71daa + unaff_EBX));
  return 1;
}


/* CInfoPlacementHelper::GetServerClass at 006b9a30 */

/* DWARF original prototype: ServerClass * GetServerClass(CInfoPlacementHelper * this) */

ServerClass * __thiscall CInfoPlacementHelper::GetServerClass(CInfoPlacementHelper *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x6fd4cc);
}


/* CInfoPlacementHelper::YouForgotToImplementOrDeclareServerClass at 006b9a40 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CInfoPlacementHelper *
   this) */

int __thiscall
CInfoPlacementHelper::YouForgotToImplementOrDeclareServerClass(CInfoPlacementHelper *this)

{
  return 0;
}


/* __tcf_3 at 00999970 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x394fc4),in_stack_00000008);
  return;
}


/* __tcf_1 at 009998b0 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d997))(unaff_EBX + 0x41d997);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d943))(unaff_EBX + 0x41d943);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d8ef))(unaff_EBX + 0x41d8ef);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d89b))(unaff_EBX + 0x41d89b);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d847))(unaff_EBX + 0x41d847);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d7f3))(unaff_EBX + 0x41d7f3);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d79f))(unaff_EBX + 0x41d79f);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d74b))(unaff_EBX + 0x41d74b);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d6f7))(unaff_EBX + 0x41d6f7);
                    /* WARNING: Could not recover jumptable at 0x00999967. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d6a3))();
  return;
}


/* CInfoPlacementManager::~CInfoPlacementManager at 006badc0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void ~CInfoPlacementManager(CInfoPlacementManager * this, int
   __in_chrg) */

void __thiscall
CInfoPlacementManager::~CInfoPlacementManager(CInfoPlacementManager *this,int __in_chrg)

{
  CHandle<CInfoPlacementHelper> *pCVar1;
  int unaff_EBX;
  CHandle<CInfoPlacementHelper> *in_stack_ffffffc8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(unaff_EBX + 0x576a14);
  (this->m_PlacementHelpers).m_Size = 0;
  if ((this->m_PlacementHelpers).m_Memory.m_nGrowSize < 0) {
    pCVar1 = (this->m_PlacementHelpers).m_Memory.m_pMemory;
  }
  else {
    pCVar1 = (this->m_PlacementHelpers).m_Memory.m_pMemory;
    if (pCVar1 != (CHandle<CInfoPlacementHelper> *)0x0) {
      in_stack_ffffffc8 = pCVar1;
      (**(code **)(*(int *)**(undefined4 **)
                             (CNPC_EnemyFinderCombineCannon::InputSetWideFOVForSeconds +
                             unaff_EBX + 4) + 8))
                ((int *)**(undefined4 **)
                          (CNPC_EnemyFinderCombineCannon::InputSetWideFOVForSeconds + unaff_EBX + 4)
                );
      pCVar1 = (CHandle<CInfoPlacementHelper> *)0x0;
      (this->m_PlacementHelpers).m_Memory.m_pMemory = (CHandle<CInfoPlacementHelper> *)0x0;
    }
    (this->m_PlacementHelpers).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PlacementHelpers).m_pElements = pCVar1;
  if (-1 < (this->m_PlacementHelpers).m_Memory.m_nGrowSize) {
    if (pCVar1 != (CHandle<CInfoPlacementHelper> *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)
                             (CNPC_EnemyFinderCombineCannon::InputSetWideFOVForSeconds +
                             unaff_EBX + 4) + 8))
                ((int *)**(undefined4 **)
                          (CNPC_EnemyFinderCombineCannon::InputSetWideFOVForSeconds + unaff_EBX + 4)
                );
      (this->m_PlacementHelpers).m_Memory.m_pMemory = (CHandle<CInfoPlacementHelper> *)0x0;
      in_stack_ffffffc8 = pCVar1;
    }
    (this->m_PlacementHelpers).m_Memory.m_nAllocationCount = 0;
  }
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(unaff_EBX + 0x4f32d4);
  IGameSystem::~IGameSystem((IGameSystem *)this,(int)in_stack_ffffffc8);
  operator_delete(this);
  return;
}


/* CInfoPlacementManager::~CInfoPlacementManager at 006baed0 */

/* DWARF original prototype: void ~CInfoPlacementManager(CInfoPlacementManager * this, int
   __in_chrg) */

void __thiscall
CInfoPlacementManager::~CInfoPlacementManager(CInfoPlacementManager *this,int __in_chrg)

{
  CHandle<CInfoPlacementHelper> *pCVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(unaff_EBX + 0x576904);
  (this->m_PlacementHelpers).m_Size = 0;
  if ((this->m_PlacementHelpers).m_Memory.m_nGrowSize < 0) {
    pCVar1 = (this->m_PlacementHelpers).m_Memory.m_pMemory;
  }
  else {
    pCVar1 = (this->m_PlacementHelpers).m_Memory.m_pMemory;
    if (pCVar1 != (CHandle<CInfoPlacementHelper> *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4f0cb4) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4f0cb4),pCVar1);
      (this->m_PlacementHelpers).m_Memory.m_pMemory = (CHandle<CInfoPlacementHelper> *)0x0;
    }
    pCVar1 = (CHandle<CInfoPlacementHelper> *)0x0;
    (this->m_PlacementHelpers).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PlacementHelpers).m_pElements = pCVar1;
  if (-1 < (this->m_PlacementHelpers).m_Memory.m_nGrowSize) {
    if (pCVar1 != (CHandle<CInfoPlacementHelper> *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4f0cb4) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4f0cb4),pCVar1);
      (this->m_PlacementHelpers).m_Memory.m_pMemory = (CHandle<CInfoPlacementHelper> *)0x0;
    }
    (this->m_PlacementHelpers).m_Memory.m_nAllocationCount = 0;
  }
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(unaff_EBX + 0x4f31c4);
  IGameSystem::~IGameSystem((IGameSystem *)this,__in_chrg);
  return;
}


/* __tcf_4 at 00999a90 */

void __tcf_4(void *param_1)

{
  int unaff_EBX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_bx();
  *(int *)(unaff_EBX + 0x41d47c) = unaff_EBX + 0x297d44;
  *(undefined4 *)(unaff_EBX + 0x41d494) = 0;
  if (*(int *)(unaff_EBX + 0x41d490) < 0) {
    *(undefined4 *)(unaff_EBX + 0x41d498) = *(undefined4 *)(unaff_EBX + 0x41d488);
  }
  else if (*(int *)(unaff_EBX + 0x41d488) == 0) {
    *(undefined4 *)(unaff_EBX + 0x41d48c) = 0;
    *(undefined4 *)(unaff_EBX + 0x41d498) = 0;
    *(undefined4 *)(unaff_EBX + 0x41d48c) = 0;
  }
  else {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2120f4) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x2120f4),*(int *)(unaff_EBX + 0x41d488));
    *(undefined4 *)(unaff_EBX + 0x41d488) = 0;
    *(undefined4 *)(unaff_EBX + 0x41d48c) = 0;
    *(undefined4 *)(unaff_EBX + 0x41d498) = 0;
    if (-1 < *(int *)(unaff_EBX + 0x41d490)) {
      *(undefined4 *)(unaff_EBX + 0x41d48c) = 0;
    }
  }
  *(int *)(unaff_EBX + 0x41d47c) = unaff_EBX + 0x214604;
  IGameSystem::~IGameSystem((IGameSystem *)(unaff_EBX + 0x41d47c),in_stack_00000008);
  return;
}


/* CInfoPlacementManager::LevelShutdownPostEntity at 006bac30 */

/* DWARF original prototype: void LevelShutdownPostEntity(CInfoPlacementManager * this) */

void __thiscall CInfoPlacementManager::LevelShutdownPostEntity(CInfoPlacementManager *this)

{
  CHandle<CInfoPlacementHelper> *pCVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->m_PlacementHelpers).m_Size = 0;
  if ((this->m_PlacementHelpers).m_Memory.m_nGrowSize < 0) {
    pCVar1 = (this->m_PlacementHelpers).m_Memory.m_pMemory;
  }
  else {
    pCVar1 = (this->m_PlacementHelpers).m_Memory.m_pMemory;
    if (pCVar1 != (CHandle<CInfoPlacementHelper> *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4f0f57) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4f0f57),pCVar1);
      (this->m_PlacementHelpers).m_Memory.m_pMemory = (CHandle<CInfoPlacementHelper> *)0x0;
    }
    pCVar1 = (CHandle<CInfoPlacementHelper> *)0x0;
    (this->m_PlacementHelpers).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PlacementHelpers).m_pElements = pCVar1;
  return;
}


/* UTIL_FindPlacementHelper at 006ba070 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

CInfoPlacementHelper * UTIL_FindPlacementHelper(Vector *vecEndPoint,CBasePlayer *pPlayer)

{
  uint uVar1;
  CInfoPlacementHelper *this;
  int iVar2;
  char *pcVar3;
  int unaff_EBX;
  int iVar4;
  longdouble lVar5;
  longdouble lVar6;
  double dVar7;
  float fVar8;
  float local_8c;
  CInfoPlacementHelper *local_88;
  double local_84;
  double local_7c;
  float local_40;
  float local_3c;
  float local_38;
  float local_34;
  float local_30;
  float local_2c;
  Vector local_28 [2];
  
  ___i686_get_pc_thunk_bx();
  if (pPlayer != (CBasePlayer *)0x0) {
    (**(code **)((int)(pPlayer->super_CBaseCombatCharacter).super_CBaseFlex.
                      super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                      super_IServerEntity.super_IServerUnknown + 0x414))(&local_34,pPlayer);
    (**(code **)((int)(pPlayer->super_CBaseCombatCharacter).super_CBaseFlex.
                      super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                      super_IServerEntity.super_IServerUnknown + 0x220))();
    if (0 < *(int *)(unaff_EBX + 0x6fceb7)) {
      local_88 = (CInfoPlacementHelper *)0x0;
      local_8c = *(float *)(unaff_EBX + 0x37908b);
      iVar4 = 0;
      do {
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
        uVar1 = *(uint *)(*(int *)(unaff_EBX + 0x6fceab) + iVar4 * 4);
        if (((((uVar1 != 0xffffffff) &&
              (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f1b23),
              *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
             (this = *(CInfoPlacementHelper **)(iVar2 + 4), this != (CInfoPlacementHelper *)0x0)) &&
            (((this->m_bDisabled).m_Value == false &&
             ((this->m_bDeferringToPortal).m_Value == false)))) &&
           (((this->m_bForcePlacement).m_Value != false ||
            (fVar8 = *(float *)(**(int **)(unaff_EBX + 0x4f1b1f) + 0xc),
            (this->m_flDisableTime).m_Value <= fVar8 && fVar8 != (this->m_flDisableTime).m_Value))))
        {
          if (((this->super_CPointEntity).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
            CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
          }
                    /* Unresolved local var: Vector res@[???] */
          local_28[0].x = (this->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.x - local_40;
          local_28[0].y = (this->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.y - local_3c;
          local_28[0].z = (this->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.z - local_38;
          fVar8 = VectorNormalize(local_28);
          dVar7 = (double)(this->m_flRadius).m_Value;
          lVar5 = (longdouble)_atan2(dVar7,(double)fVar8);
          lVar6 = (longdouble)
                  _acosf(local_34 * local_28[0].x + local_30 * local_28[0].y +
                         local_2c * local_28[0].z);
          if ((((float)lVar6 <= (float)lVar5) && (fVar8 <= local_8c)) &&
             (local_8c = fVar8, local_88 = this,
             *(int *)(*(int *)(*(int *)(unaff_EBX + 0x4f1e83) + 0x1c) + 0x30) != 0)) {
            if (((this->super_CPointEntity).super_CBaseEntity.m_iEFlags & 0x800) == 0) {
              local_7c = (double)(this->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.z;
              local_84 = (double)(this->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.y;
            }
            else {
              CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
              local_7c = (double)(this->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.z;
              if (((this->super_CPointEntity).super_CBaseEntity.m_iEFlags & 0x800) == 0) {
                local_84 = (double)(this->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.y;
              }
              else {
                CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
                local_84 = (double)(this->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.y;
                if (((this->super_CPointEntity).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
                  CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
                }
              }
            }
            pcVar3 = (this->super_CPointEntity).super_CBaseEntity.m_iName.m_Value.pszValue;
            if (pcVar3 == (char *)0x0) {
              pcVar3 = (char *)(unaff_EBX + 0x2f1adb);
            }
            _Msg(unaff_EBX + 0x354e0f,pcVar3,
                 (double)(this->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.x,local_84,
                 local_7c,dVar7);
          }
        }
        iVar4 = iVar4 + 1;
      } while (iVar4 < *(int *)(unaff_EBX + 0x6fceb7));
      return local_88;
    }
  }
  return (CInfoPlacementHelper *)0x0;
}


/* CInfoPlacementHelper::CInfoPlacementHelper at 006ba8c0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CInfoPlacementHelper(CInfoPlacementHelper * this) */

void __thiscall CInfoPlacementHelper::CInfoPlacementHelper(CInfoPlacementHelper *this)

{
  undefined1 *puVar1;
  char cVar2;
  CBaseEdict *pCVar3;
  IChangeInfoAccessor *pIVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::CBaseEntity((CBaseEntity *)this,false);
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x576b9a);
  (this->m_OnObjectPlaced).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnObjectPlaced).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->m_OnObjectPlaced).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_ObjectPlacedSize).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_ObjectPlacedSize).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->m_ObjectPlacedSize).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_bSnapToHelperAngles).m_Value = false;
  if ((this->super_CPointEntity).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
    pCVar3 = &((this->super_CPointEntity).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if (pCVar3 == (CBaseEdict *)0x0) goto LAB_006ba931;
    pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
    pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
    pIVar4->m_iChangeInfoSerialNumber = 0;
    (this->m_bForcePlacement).m_Value = false;
    if ((this->super_CPointEntity).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false)
    goto LAB_006ba9b0;
LAB_006ba93e:
    puVar1 = &(this->super_CPointEntity).super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
LAB_006ba942:
    (this->m_flDisableTime).m_Value = 0.0;
    if ((this->super_CPointEntity).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false)
    goto LAB_006ba9e0;
LAB_006ba956:
    puVar1 = &(this->super_CPointEntity).super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  else {
    puVar1 = &(this->super_CPointEntity).super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
LAB_006ba931:
    (this->m_bForcePlacement).m_Value = false;
    if ((this->super_CPointEntity).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered != false)
    goto LAB_006ba93e;
LAB_006ba9b0:
    pCVar3 = &((this->super_CPointEntity).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if (pCVar3 == (CBaseEdict *)0x0) goto LAB_006ba942;
    pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
    pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
    pIVar4->m_iChangeInfoSerialNumber = 0;
    (this->m_flDisableTime).m_Value = 0.0;
    if ((this->super_CPointEntity).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered != false)
    goto LAB_006ba956;
LAB_006ba9e0:
    pCVar3 = &((this->super_CPointEntity).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if (pCVar3 != (CBaseEdict *)0x0) {
      pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
      pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
      pIVar4->m_iChangeInfoSerialNumber = 0;
      (this->m_bDeferringToPortal).m_Value = false;
      cVar2 = (this->super_CPointEntity).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered;
      goto joined_r0x006baa0a;
    }
  }
  (this->m_bDeferringToPortal).m_Value = false;
  cVar2 = (this->super_CPointEntity).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered;
joined_r0x006baa0a:
  if (cVar2 == '\0') {
    pCVar3 = &((this->super_CPointEntity).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if (pCVar3 != (CBaseEdict *)0x0) {
      pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
      pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
      pIVar4->m_iChangeInfoSerialNumber = 0;
      return;
    }
  }
  else {
    puVar1 = &(this->super_CPointEntity).super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  return;
}


/* CInfoPlacementHelper::CInfoPlacementHelper at 006baa70 */

/* DWARF original prototype: void CInfoPlacementHelper(CInfoPlacementHelper * this,
   CInfoPlacementHelper * this) */

void __thiscall
CInfoPlacementHelper::CInfoPlacementHelper(CInfoPlacementHelper *this,CInfoPlacementHelper *this_1)

{
  CInfoPlacementHelper(this);
  return;
}


/* CEntityFactory<CInfoPlacementHelper>::Create at 006bafd0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CInfoPlacementHelper> *
   this, char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CInfoPlacementHelper>::Create
          (CEntityFactory<CInfoPlacementHelper> *this,char *pClassName)

{
  CInfoPlacementHelper *this_00;
  
                    /* Unresolved local var: CInfoPlacementHelper * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x408);
  CInfoPlacementHelper::CInfoPlacementHelper(this_00);
  (*(this_00->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1d])(this_00,pClassName);
  return &(this_00->super_CPointEntity).super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CInfoPlacementHelper::Spawn at 006ba420 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CInfoPlacementHelper * this) */

void __thiscall CInfoPlacementHelper::Spawn(CInfoPlacementHelper *this)

{
  uint uVar1;
  uint *puVar2;
  int iVar3;
  undefined4 *puVar4;
  int iVar5;
  int iVar6;
  int unaff_EBX;
  int iVar7;
  int iVar8;
  int local_30;
  undefined4 local_24;
  uint local_20;
  
  ___i686_get_pc_thunk_bx();
  CPointEntity::Spawn(&this->super_CPointEntity);
  if (this == (CInfoPlacementHelper *)0x0) {
    local_20 = 0xffffffff;
  }
  else {
    puVar2 = (uint *)(*(this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.
                       super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[3])(this);
    local_20 = *puVar2;
  }
  local_30 = *(int *)(unaff_EBX + 0x6fcb0a);
  if (0 < local_30) {
    iVar5 = **(int **)(unaff_EBX + 0x4f1776);
    if (local_20 == 0xffffffff) {
      iVar8 = 0;
      do {
        uVar1 = *(uint *)(*(int *)(unaff_EBX + 0x6fcafe) + iVar8 * 4);
        if (uVar1 == 0xffffffff) {
          return;
        }
        iVar6 = (uVar1 & 0xffff) * 0x10 + iVar5;
        if (*(uint *)(iVar6 + 8) != uVar1 >> 0x10) {
          return;
        }
        if (*(int *)(iVar6 + 4) == 0) {
          return;
        }
        iVar8 = iVar8 + 1;
      } while (iVar8 != local_30);
    }
    else {
      iVar6 = 0;
      iVar8 = (local_20 & 0xffff) * 0x10 + iVar5;
      uVar1 = *(uint *)(iVar8 + 8);
      while( true ) {
        iVar7 = 0;
        if (uVar1 == local_20 >> 0x10) {
          iVar7 = *(int *)(iVar8 + 4);
        }
        uVar1 = *(uint *)(*(int *)(unaff_EBX + 0x6fcafe) + iVar6 * 4);
        if ((uVar1 == 0xffffffff) ||
           (iVar3 = (uVar1 & 0xffff) * 0x10 + iVar5, *(uint *)(iVar3 + 8) != uVar1 >> 0x10)) {
          if (iVar7 == 0) {
            return;
          }
        }
        else if (*(int *)(iVar3 + 4) == iVar7) {
          return;
        }
        iVar6 = iVar6 + 1;
        if (iVar6 == local_30) break;
        uVar1 = *(uint *)(iVar8 + 8);
      }
    }
  }
  if (this == (CInfoPlacementHelper *)0x0) {
    local_24 = 0xffffffff;
    iVar8 = local_30 + 1;
    iVar5 = *(int *)(unaff_EBX + 0x6fcb02);
    if (iVar8 <= iVar5) goto LAB_006ba563;
  }
  else {
    puVar4 = (undefined4 *)
             (*(this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown
               .super_IHandleEntity._vptr_IHandleEntity[3])(this);
    local_24 = *puVar4;
    local_30 = *(int *)(unaff_EBX + 0x6fcb0a);
    iVar8 = local_30 + 1;
    iVar5 = *(int *)(unaff_EBX + 0x6fcb02);
    if (iVar8 <= iVar5) goto LAB_006ba563;
  }
  CUtlMemory<CHandle<CInfoPlacementHelper>,int>::Grow
            ((CUtlMemory<CHandle<CInfoPlacementHelper>,int> *)(unaff_EBX + 0x6fcafe),iVar8 - iVar5);
LAB_006ba563:
  iVar6 = *(int *)(unaff_EBX + 0x6fcb0a) + 1;
  *(int *)(unaff_EBX + 0x6fcb0a) = iVar6;
  iVar5 = *(int *)(unaff_EBX + 0x6fcafe);
  *(int *)(unaff_EBX + 0x6fcb0e) = iVar5;
  iVar6 = (iVar6 - local_30) + -1;
  if (iVar6 < 1) {
    local_30 = local_30 << 2;
  }
  else {
    local_30 = local_30 * 4;
    _V_memmove((void *)(iVar5 + iVar8 * 4),(void *)(iVar5 + local_30),iVar6 * 4);
  }
  puVar4 = (undefined4 *)(local_30 + *(int *)(unaff_EBX + 0x6fcafe));
  if (puVar4 == (undefined4 *)0x0) {
    return;
  }
  *puVar4 = local_24;
  return;
}


/* CInfoPlacementHelper::OnRestore at 006b9ac0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void OnRestore(CInfoPlacementHelper * this) */

void __thiscall CInfoPlacementHelper::OnRestore(CInfoPlacementHelper *this)

{
  uint uVar1;
  uint *puVar2;
  int iVar3;
  undefined4 *puVar4;
  int iVar5;
  int iVar6;
  int unaff_EBX;
  int iVar7;
  int iVar8;
  int local_30;
  undefined4 local_24;
  uint local_20;
  
  ___i686_get_pc_thunk_bx();
  if (this == (CInfoPlacementHelper *)0x0) {
    local_20 = 0xffffffff;
  }
  else {
    puVar2 = (uint *)(*(this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.
                       super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[3])(this);
    local_20 = *puVar2;
  }
  local_30 = *(int *)(unaff_EBX + 0x6fd46a);
  if (0 < local_30) {
    iVar5 = **(int **)(unaff_EBX + 0x4f20d6);
    if (local_20 == 0xffffffff) {
      iVar8 = 0;
      do {
        uVar1 = *(uint *)(*(int *)(unaff_EBX + 0x6fd45e) + iVar8 * 4);
        if (((uVar1 == 0xffffffff) ||
            (iVar6 = (uVar1 & 0xffff) * 0x10 + iVar5, *(uint *)(iVar6 + 8) != uVar1 >> 0x10)) ||
           (*(int *)(iVar6 + 4) == 0)) goto LAB_006b9b87;
        iVar8 = iVar8 + 1;
      } while (iVar8 != local_30);
    }
    else {
      iVar6 = 0;
      iVar8 = (local_20 & 0xffff) * 0x10 + iVar5;
      uVar1 = *(uint *)(iVar8 + 8);
      while( true ) {
        iVar7 = 0;
        if (uVar1 == local_20 >> 0x10) {
          iVar7 = *(int *)(iVar8 + 4);
        }
        uVar1 = *(uint *)(*(int *)(unaff_EBX + 0x6fd45e) + iVar6 * 4);
        if ((uVar1 == 0xffffffff) ||
           (iVar3 = (uVar1 & 0xffff) * 0x10 + iVar5, *(uint *)(iVar3 + 8) != uVar1 >> 0x10)) {
          if (iVar7 == 0) goto LAB_006b9b87;
        }
        else if (*(int *)(iVar3 + 4) == iVar7) goto LAB_006b9b87;
        iVar6 = iVar6 + 1;
        if (iVar6 == local_30) break;
        uVar1 = *(uint *)(iVar8 + 8);
      }
    }
  }
  if (this == (CInfoPlacementHelper *)0x0) {
    local_24 = 0xffffffff;
    iVar8 = local_30 + 1;
    iVar5 = *(int *)(unaff_EBX + 0x6fd462);
    if (iVar8 <= iVar5) goto LAB_006b9bfe;
  }
  else {
    puVar4 = (undefined4 *)
             (*(this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown
               .super_IHandleEntity._vptr_IHandleEntity[3])(this);
    local_24 = *puVar4;
    local_30 = *(int *)(unaff_EBX + 0x6fd46a);
    iVar8 = local_30 + 1;
    iVar5 = *(int *)(unaff_EBX + 0x6fd462);
    if (iVar8 <= iVar5) goto LAB_006b9bfe;
  }
  CUtlMemory<CHandle<CInfoPlacementHelper>,int>::Grow
            ((CUtlMemory<CHandle<CInfoPlacementHelper>,int> *)(unaff_EBX + 0x6fd45e),iVar8 - iVar5);
LAB_006b9bfe:
  iVar6 = *(int *)(unaff_EBX + 0x6fd46a) + 1;
  *(int *)(unaff_EBX + 0x6fd46a) = iVar6;
  iVar5 = *(int *)(unaff_EBX + 0x6fd45e);
  *(int *)(unaff_EBX + 0x6fd46e) = iVar5;
  iVar6 = (iVar6 - local_30) + -1;
  if (iVar6 < 1) {
    local_30 = local_30 << 2;
  }
  else {
    local_30 = local_30 * 4;
    _V_memmove((void *)(iVar5 + iVar8 * 4),(void *)(iVar5 + local_30),iVar6 * 4);
  }
  puVar4 = (undefined4 *)(local_30 + *(int *)(unaff_EBX + 0x6fd45e));
  if (puVar4 != (undefined4 *)0x0) {
    *puVar4 = local_24;
  }
LAB_006b9b87:
  CBaseEntity::OnRestore((CBaseEntity *)this);
  return;
}


/* CInfoPlacementHelper::UpdateOnRemove at 006b9c90 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void UpdateOnRemove(CInfoPlacementHelper * this) */

void __thiscall CInfoPlacementHelper::UpdateOnRemove(CInfoPlacementHelper *this)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  uint *puVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int unaff_EBX;
  int iVar9;
  uint local_20;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::UpdateOnRemove((CBaseEntity *)this);
  if (this == (CInfoPlacementHelper *)0x0) {
    local_20 = 0xffffffff;
  }
  else {
    puVar4 = (uint *)(*(this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.
                       super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[3])(this);
    local_20 = *puVar4;
  }
  iVar6 = *(int *)(unaff_EBX + 0x6fd29a);
  if (0 < iVar6) {
    iVar1 = *(int *)(&DAT_006fd28e + unaff_EBX);
    iVar2 = **(int **)(unaff_EBX + 0x4f1f06);
    if (local_20 != 0xffffffff) {
      iVar8 = 0;
      iVar7 = (local_20 & 0xffff) * 0x10 + iVar2;
      uVar3 = *(uint *)(iVar7 + 8);
      do {
        iVar9 = 0;
        if (uVar3 == local_20 >> 0x10) {
          iVar9 = *(int *)(iVar7 + 4);
        }
        uVar3 = *(uint *)(iVar1 + iVar8 * 4);
        if ((uVar3 == 0xffffffff) ||
           (iVar5 = (uVar3 & 0xffff) * 0x10 + iVar2, *(uint *)(iVar5 + 8) != uVar3 >> 0x10)) {
          if (iVar9 == 0) goto LAB_006b9d67;
        }
        else if (*(int *)(iVar5 + 4) == iVar9) goto LAB_006b9d67;
        iVar8 = iVar8 + 1;
        if (iVar8 == iVar6) {
          return;
        }
        uVar3 = *(uint *)(iVar7 + 8);
      } while( true );
    }
    iVar8 = 0;
    do {
      uVar3 = *(uint *)(iVar1 + iVar8 * 4);
      if (((uVar3 == 0xffffffff) ||
          (iVar7 = (uVar3 & 0xffff) * 0x10 + iVar2, *(uint *)(iVar7 + 8) != uVar3 >> 0x10)) ||
         (*(int *)(iVar7 + 4) == 0)) {
LAB_006b9d67:
        iVar6 = (iVar6 - iVar8) + -1;
        if (0 < iVar6) {
          _V_memmove((void *)(iVar1 + iVar8 * 4),(void *)(iVar1 + 4 + iVar8 * 4),iVar6 * 4);
        }
        *(int *)(unaff_EBX + 0x6fd29a) = *(int *)(unaff_EBX + 0x6fd29a) + -1;
        return;
      }
      iVar8 = iVar8 + 1;
    } while (iVar8 != iVar6);
  }
  return;
}


/* CInfoPlacementHelper::ShouldUseHelperAngles at 006b9ab0 */

/* DWARF original prototype: bool ShouldUseHelperAngles(CInfoPlacementHelper * this) */

bool __thiscall CInfoPlacementHelper::ShouldUseHelperAngles(CInfoPlacementHelper *this)

{
  return (this->m_bSnapToHelperAngles).m_Value;
}


/* CInfoPlacementHelper::InputEnable at 006b9e90 */

/* DWARF original prototype: void InputEnable(CInfoPlacementHelper * this, inputdata_t * inputdata)
    */

void __thiscall CInfoPlacementHelper::InputEnable(CInfoPlacementHelper *this,inputdata_t *inputdata)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  
  if ((this->m_bDisabled).m_Value != false) {
    if ((this->super_CPointEntity).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false)
    {
      this_00 = &((this->super_CPointEntity).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar2->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CPointEntity).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bDisabled).m_Value = false;
  }
  return;
}


/* CInfoPlacementHelper::InputDisable at 006ba600 */

/* DWARF original prototype: void InputDisable(CInfoPlacementHelper * this, inputdata_t * inputdata)
    */

void __thiscall
CInfoPlacementHelper::InputDisable(CInfoPlacementHelper *this,inputdata_t *inputdata)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  
  if ((this->m_bDisabled).m_Value == false) {
    if ((this->super_CPointEntity).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false)
    {
      this_00 = &((this->super_CPointEntity).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar2->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CPointEntity).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bDisabled).m_Value = true;
  }
  return;
}


/* CInfoPlacementHelper::DrawDebugGeometryOverlays at 006b9ef0 */

/* DWARF original prototype: void DrawDebugGeometryOverlays(CInfoPlacementHelper * this) */

void __thiscall CInfoPlacementHelper::DrawDebugGeometryOverlays(CInfoPlacementHelper *this)

{
  Vector *position;
  byte bVar1;
  float fVar2;
  int unaff_EBX;
  QAngle *angles;
  int local_20;
  
                    /* Unresolved local var: int g@[???] */
  ___i686_get_pc_thunk_bx();
  if ((((this->super_CPointEntity).super_CBaseEntity.m_iEFlags & 0x800) != 0) &&
     (CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this),
     ((this->super_CPointEntity).super_CBaseEntity.m_iEFlags & 0x800) != 0)) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
  angles = &(this->super_CPointEntity).super_CBaseEntity.m_angAbsRotation;
  position = &(this->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin;
  NDebugOverlay::Axis(position,angles,8.0,false,0.1);
  if ((((this->m_bDisabled).m_Value != false) || ((this->m_bDeferringToPortal).m_Value != false)) ||
     (((this->m_bForcePlacement).m_Value == false &&
      (fVar2 = *(float *)(**(int **)(unaff_EBX + 0x4f1c9c) + 0xc),
      fVar2 < (this->m_flDisableTime).m_Value || fVar2 == (this->m_flDisableTime).m_Value)))) {
    local_20 = 0;
    fVar2 = (this->m_flRadius).m_Value;
    bVar1 = *(byte *)((int)&(this->super_CPointEntity).super_CBaseEntity.m_iEFlags + 1);
  }
  else {
    local_20 = 0xff;
    fVar2 = (this->m_flRadius).m_Value;
    bVar1 = *(byte *)((int)&(this->super_CPointEntity).super_CBaseEntity.m_iEFlags + 1);
  }
  if (((bVar1 & 8) != 0) &&
     (CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this),
     ((this->super_CPointEntity).super_CBaseEntity.m_iEFlags & 0x800) != 0)) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
  NDebugOverlay::Sphere(position,angles,fVar2,0xff,local_20,0,0,false,0.1);
  CBaseEntity::DrawDebugGeometryOverlays((CBaseEntity *)this);
  return;
}


/* CInfoPlacementHelper::IsEnabled at 006b9a50 */

/* DWARF original prototype: bool IsEnabled(CInfoPlacementHelper * this) */

bool __thiscall CInfoPlacementHelper::IsEnabled(CInfoPlacementHelper *this)

{
  float fVar1;
  bool bVar2;
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  if (((this->m_bDisabled).m_Value == false) && ((this->m_bDeferringToPortal).m_Value == false)) {
    bVar2 = true;
    if ((this->m_bForcePlacement).m_Value == false) {
      fVar1 = *(float *)(**(int **)(extraout_ECX + 0x4f2148) + 0xc);
      bVar2 = (this->m_flDisableTime).m_Value <= fVar1 && fVar1 != (this->m_flDisableTime).m_Value;
    }
    return bVar2;
  }
  return false;
}


/* CInfoPlacementHelper::NotifyPortalEvent at 006ba660 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void NotifyPortalEvent(CInfoPlacementHelper * this, PortalEvent_t
   nEventType, CPortal_Base2D * pNotifier) */

void __thiscall
CInfoPlacementHelper::NotifyPortalEvent
          (CInfoPlacementHelper *this,PortalEvent_t nEventType,CPortal_Base2D *pNotifier)

{
  undefined1 *puVar1;
  CBaseEdict *pCVar2;
  ulong *puVar3;
  IChangeInfoAccessor *pIVar4;
  CInfoPlacementHelper *this_00;
  char *pcVar5;
  int unaff_EBX;
  float fVar6;
  float fVar7;
  float fVar8;
  CBaseHandle local_20 [4];
  
                    /* Unresolved local var: float flResetDelay@[???] */
  ___i686_get_pc_thunk_bx();
  if (nEventType == PORTALEVENT_LINKED) {
    return;
  }
  if (nEventType != PORTALEVENT_MOVED) goto joined_r0x006ba841;
  pcVar5 = (this->m_strTargetProxy).m_Value.pszValue;
  if (pcVar5 == (char *)0x0) {
LAB_006ba880:
    this_00 = this;
    if (((this->super_CPointEntity).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
  }
  else {
    this_00 = (CInfoPlacementHelper *)
              CGlobalEntityList::FindEntityByName
                        (*(CGlobalEntityList **)(unaff_EBX + 0x4f15ec),(CBaseEntity *)0x0,pcVar5,
                         (CBaseEntity *)0x0,(CBaseEntity *)0x0,(CBaseEntity *)0x0,
                         (IEntityFindFilter *)0x0);
    if (this_00 == (CInfoPlacementHelper *)0x0) {
      pcVar5 = (this->m_strTargetProxy).m_Value.pszValue;
      if (pcVar5 == (char *)0x0) {
        pcVar5 = (char *)(unaff_EBX + 0x2f14e8);
      }
      _Warning(unaff_EBX + 0x3031b8,pcVar5);
      goto LAB_006ba880;
    }
    if (((this_00->super_CPointEntity).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this_00);
    }
  }
  if (((pNotifier->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)pNotifier);
  }
                    /* Unresolved local var: Vector res@[???] */
  fVar6 = (pNotifier->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.x -
          (this_00->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.x;
  fVar7 = (pNotifier->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.y -
          (this_00->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.y;
  fVar8 = (pNotifier->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.z -
          (this_00->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.z;
  if (fVar6 * fVar6 + fVar7 * fVar7 + fVar8 * fVar8 < *(float *)(unaff_EBX + 0x36fec4)) {
    return;
  }
joined_r0x006ba841:
  if (this == (CInfoPlacementHelper *)0x0) {
    local_20[0].m_Index = 0xffffffff;
  }
  else {
    local_20[0].m_Index = 0xffffffff;
    puVar3 = (ulong *)(*(this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.
                        super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[3])(this);
    local_20[0].m_Index = *puVar3;
  }
  CPortal_Base2D::RemovePortalEventListener(pNotifier,(EHANDLE *)local_20);
  if ((this->m_bDeferringToPortal).m_Value != false) {
    if ((this->super_CPointEntity).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false)
    {
      pCVar2 = &((this->super_CPointEntity).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar4 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar4->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CPointEntity).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bDeferringToPortal).m_Value = false;
  }
  fVar6 = 0.0;
  if (nEventType != PORTALEVENT_FIZZLE) {
    fVar6 = *(float *)(unaff_EBX + 0x36fce4);
  }
  fVar6 = fVar6 + *(float *)(**(int **)(unaff_EBX + 0x4f152c) + 0xc);
  if (fVar6 != (this->m_flDisableTime).m_Value) {
    if ((this->super_CPointEntity).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false)
    {
      pCVar2 = &((this->super_CPointEntity).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar4 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar4->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CPointEntity).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flDisableTime).m_Value = fVar6;
  }
  return;
}


/* CInfoPlacementHelper::BindToPortal at 006b9de0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void BindToPortal(CInfoPlacementHelper * this, CPortal_Base2D *
   pPortal) */

void __thiscall
CInfoPlacementHelper::BindToPortal(CInfoPlacementHelper *this,CPortal_Base2D *pPortal)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  ulong *puVar2;
  IChangeInfoAccessor *pIVar3;
  EHANDLE local_10;
  
  if ((pPortal != (CPortal_Base2D *)0x0) && ((this->m_bForcePlacement).m_Value == false)) {
    local_10.super_CBaseHandle.m_Index = (CBaseHandle)0xffffffff;
    puVar2 = (ulong *)(*(this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.
                        super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[3])(this);
    local_10.super_CBaseHandle.m_Index = (CBaseHandle)*puVar2;
    CPortal_Base2D::AddPortalEventListener(pPortal,&local_10);
    if ((this->m_bDeferringToPortal).m_Value == false) {
      if ((this->super_CPointEntity).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
         ) {
        this_00 = &((this->super_CPointEntity).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
        ;
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
      (this->m_bDeferringToPortal).m_Value = true;
      return;
    }
  }
  return;
}


/* _GLOBAL__I_sv_show_placement_help_in_preview at 000c01e0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_sv_show_placement_help_in_preview(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

