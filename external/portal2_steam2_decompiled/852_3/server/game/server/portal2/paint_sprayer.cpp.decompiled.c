/* DWARF-guided pseudocode for game/server/portal2/paint_sprayer.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* ServerClassInit<DT_PaintSprayer::ignored> at 000c30a0 */

int ServerClassInit<DT_PaintSprayer::ignored>(ignored *param_1)

{
  SendVarProxyFn_conflict varProxy;
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&sk_npc_dmg_alyxgun.m_fMinVal + unaff_EBX + 2) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf56fa);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  ((int)&sk_npc_dmg_alyxgun.m_fnChangeCallbacks.m_Size + unaff_EBX + 2),
                  (char *)(unaff_EBX + 0x8ef976),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&sk_max_alyxgun.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 2),
                     (char *)(unaff_EBX + 0x93eb82),0x350,0xc,-1,0x40002,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00ae92de + unaff_EBX),0x80);
      SendPropEHandle((SendProp_conflict *)((int)&sk_plr_dmg_pistol.m_fMinVal + unaff_EBX + 2),
                      (char *)(unaff_EBX + 0x932fca),0x16c,4,-1,
                      *(SendVarProxyFn_conflict *)(&DAT_00ae8b6a + unaff_EBX));
      SendPropQAngles((SendProp_conflict *)
                      ((int)&sk_npc_dmg_pistol.m_Value.m_fValue + unaff_EBX + 2),
                      (char *)(unaff_EBX + 0x948546),0x35c,0xc,0xd,0x40000,
                      *(SendVarProxyFn_conflict *)(&DAT_00ae9422 + unaff_EBX),0x80);
      SendPropBool((SendProp_conflict *)((int)&sk_max_pistol.m_pszDefaultValue + unaff_EBX + 2),
                   (char *)(unaff_EBX + 0x946a1e),0x3d0,1);
      SendPropBool((SendProp_conflict *)
                   ((int)&sk_plr_dmg_smg1.super_ConCommandBase.m_nFlags + unaff_EBX + 2),
                   (char *)(unaff_EBX + 0x94d575),0x3d1,1);
      SendPropInt((SendProp_conflict *)(&sk_npc_dmg_smg1.super_ConCommandBase.field_0xa + unaff_EBX)
                  ,&UNK_0094c201 + unaff_EBX,0x3d4,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      varProxy = *(SendVarProxyFn_conflict *)(&DAT_00ae8bd6 + unaff_EBX);
      SendPropFloat((SendProp_conflict *)(unaff_EBX + 0xcf595e),(char *)(unaff_EBX + 0x94d580),0x3d8
                    ,4,0x20,0,0.0,-121121.125,varProxy,0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&sk_max_smg1.m_fnChangeCallbacks.m_Size + unaff_EBX + 2),
                    (char *)(unaff_EBX + 0x931f9a),0x3e4,4,0x20,0,0.0,-121121.125,varProxy,0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&sk_plr_dmg_buckshot.m_fnChangeCallbacks.m_Memory.m_pMemory +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x929f3c),1000,4,0x20,0,0.0,-121121.125,
                    varProxy,0x80);
      SendPropFloat((SendProp_conflict *)((int)&sk_npc_dmg_buckshot.m_fMinVal + unaff_EBX + 2),
                    (char *)(unaff_EBX + 0x94d595),0x3dc,4,0x20,0,0.0,-121121.125,varProxy,0x80);
      SendPropFloat((SendProp_conflict *)((int)&sk_max_buckshot.m_Value.m_fValue + unaff_EBX + 2),
                    (char *)(unaff_EBX + 0x94d5aa),0x3e0,4,0x20,0,0.0,-121121.125,varProxy,0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&sk_plr_num_shotgun_pellets.m_pszDefaultValue + unaff_EBX + 2),
                    (char *)(unaff_EBX + 0x94d5be),0x3ec,4,0x20,0,0.0,-121121.125,varProxy,0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&sk_plr_dmg_rpg_round.super_ConCommandBase.m_nFlags + unaff_EBX + 2),
                    (char *)(unaff_EBX + 0x94d5d3),0x3f0,4,0x20,0,0.0,-121121.125,varProxy,0x80);
      SendPropFloat((SendProp_conflict *)
                    (&sk_npc_dmg_rpg_round.super_ConCommandBase.field_0xa + unaff_EBX),
                    (char *)(unaff_EBX + 0x94d5e5),0x3f4,4,0x20,0,0.0,-121121.125,varProxy,0x80);
      SendPropFloat((SendProp_conflict *)(unaff_EBX + 0xcf5bfe),(char *)(unaff_EBX + 0x94d5f7),0x3f8
                    ,4,0x20,0,0.0,-121121.125,varProxy,0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&sk_max_rpg_round.m_fnChangeCallbacks.m_Size + unaff_EBX + 2),
                    (char *)(unaff_EBX + 0x94d610),0x3fc,4,0x20,0,0.0,-121121.125,varProxy,0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&sk_plr_dmg_sniper_round.m_fnChangeCallbacks.m_Memory.m_pMemory +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x93ebb6),0x400,4,0x20,0,0.0,-121121.125,
                    varProxy,0x80);
      ___cxa_guard_release(unaff_EBX + 0xcf56fa);
      ___cxa_atexit(unaff_EBX + 0x8d7992,0,*(undefined4 *)(&DAT_00ae8ae2 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)((int)&PTR_RemoveActionSignalTarget_00c6c3c8 + unaff_EBX + 2),
             (SendProp_conflict *)
             ((int)&sk_max_alyxgun.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 2),0x11,
             *(char **)(&DAT_00b722ba + unaff_EBX));
  return 1;
}


/* __static_initialization_and_destruction_0 at 000c39f0 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *s2;
  SendTable *this;
  undefined4 *puVar1;
  int iVar2;
  int iVar3;
  datamap_t *pdVar4;
  IEntityFactoryDictionary *pIVar5;
  int unaff_EBX;
  undefined4 *puVar6;
  undefined4 *puVar7;
  longlong lVar8;
  
  lVar8 = ___i686_get_pc_thunk_bx();
  if (lVar8 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)&friendlyfire.m_pszDefaultValue + unaff_EBX + 2) = 0;
  *(undefined1 *)((int)&friendlyfire.m_pszDefaultValue + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&friendlyfire.m_Value.m_pszString + unaff_EBX) = 0;
  *(undefined1 *)((int)&friendlyfire.m_Value.m_pszString + unaff_EBX + 1) = 0;
  *(undefined4 *)((int)&friendlyfire.m_Value.m_pszString + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&friendlyfire.m_Value.m_StringLength + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&friendlyfire.m_Value.m_fValue + unaff_EBX + 2) = 0x7f7fffff;
  *(undefined4 *)((int)&friendlyfire.m_Value.m_nValue + unaff_EBX + 2) = 0x7f7fffff;
  *(undefined4 *)(&friendlyfire.field_0x36 + unaff_EBX) = 0;
  *(undefined4 *)((int)&friendlyfire.m_fMinVal + unaff_EBX + 2) = 0;
  *(undefined4 *)(&friendlyfire.field_0x3e + unaff_EBX) = 0;
  *(undefined4 *)((int)&friendlyfire.m_fMaxVal + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&friendlyfire.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 2) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&friendlyfire.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX + 2) = 0x7f7fffff
  ;
  *(undefined4 *)((int)&friendlyfire.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 2) =
       0x7f7fffff;
  *(undefined4 *)((int)&friendlyfire.m_fnChangeCallbacks.m_Size + unaff_EBX + 2) = 0x7f7fffff;
  *(undefined4 *)((int)&friendlyfire.m_fnChangeCallbacks.m_pElements + unaff_EBX + 2) = 0;
  (&DAT_00cf4d3d)[unaff_EBX] = 0;
  (&DAT_00cf4d3a)[unaff_EBX] = 0;
  (&DAT_00cf4d3b)[unaff_EBX] = 0;
  (&DAT_00cf4d3c)[unaff_EBX] = 0;
  *(undefined4 *)(&DAT_00cf4d3e + unaff_EBX) = 0;
  *(undefined1 *)((int)&mp_fadetoblack.super_ConCommandBase.m_pNext + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&mp_fadetoblack.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX + 2) =
       0;
  *(undefined1 *)((int)&mp_fadetoblack.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX + 3) =
       0;
  *(undefined1 *)((int)&mp_fadetoblack.super_ConCommandBase.m_pNext + unaff_EBX) = 0;
  *(undefined4 *)((int)&mp_fadetoblack.super_ConCommandBase.m_pNext + unaff_EBX + 2) = 0;
  *(undefined1 *)((int)&mp_fadetoblack.super_ConCommandBase.m_pszName + unaff_EBX + 1) = 1;
  (&mp_fadetoblack.super_ConCommandBase.field_0xa)[unaff_EBX] = 0;
  (&mp_fadetoblack.super_ConCommandBase.field_0xb)[unaff_EBX] = 0;
  *(undefined1 *)((int)&mp_fadetoblack.super_ConCommandBase.m_pszName + unaff_EBX) = 0;
  *(undefined4 *)((int)&mp_fadetoblack.super_ConCommandBase.m_pszName + unaff_EBX + 2) = 1;
  *(undefined1 *)((int)&mp_fadetoblack.super_ConCommandBase.m_nFlags + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&mp_fadetoblack.super_ConCommandBase.m_pszHelpString + unaff_EBX + 2) = 0;
  *(undefined1 *)((int)&mp_fadetoblack.super_ConCommandBase.m_pszHelpString + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&mp_fadetoblack.super_ConCommandBase.m_nFlags + unaff_EBX) = 0;
  *(undefined4 *)((int)&mp_fadetoblack.super_ConCommandBase.m_nFlags + unaff_EBX + 2) = 2;
  *(undefined1 *)((int)&mp_fadetoblack.m_pParent + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&mp_fadetoblack.super_IConVar._vptr_IConVar + unaff_EBX + 2) = 0;
  *(undefined1 *)((int)&mp_fadetoblack.super_IConVar._vptr_IConVar + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&mp_fadetoblack.m_pParent + unaff_EBX) = 0;
  *(undefined **)((int)&mp_fadetoblack.m_pParent + unaff_EBX + 2) = &UNK_00aed92a + unaff_EBX;
  iVar3 = unaff_EBX + 0xcf4d62;
  s2 = (char *)(unaff_EBX + 0x94ccdb);
  *(char **)((int)&mp_fadetoblack.m_pszDefaultValue + unaff_EBX + 2) = s2;
  this = (SendTable *)((int)&PTR_SetUseCaptureMouse_00c6ba78 + unaff_EBX + 2);
  *(SendTable **)((int)&mp_fadetoblack.m_Value.m_pszString + unaff_EBX + 2) = this;
  *(undefined4 *)((int)&mp_fadetoblack.m_Value.m_nValue + unaff_EBX + 2) = 0xffff;
  puVar7 = (undefined4 *)**(int **)(&DAT_00ae8232 + unaff_EBX);
  if (puVar7 == (undefined4 *)0x0) {
    **(int **)(&DAT_00ae8232 + unaff_EBX) = iVar3;
    *(undefined4 *)((int)&mp_fadetoblack.m_Value.m_StringLength + unaff_EBX + 2) = 0;
  }
  else {
    puVar1 = (undefined4 *)puVar7[2];
    iVar2 = _V_stricmp((char *)*puVar7,s2);
    if (iVar2 < 1) {
      while ((puVar6 = puVar1, puVar6 != (undefined4 *)0x0 &&
             (iVar2 = _V_stricmp((char *)*puVar6,s2), iVar2 < 1))) {
        puVar1 = (undefined4 *)puVar6[2];
        puVar7 = puVar6;
      }
      *(undefined4 **)((int)&mp_fadetoblack.m_Value.m_StringLength + unaff_EBX + 2) = puVar6;
      puVar7[2] = iVar3;
    }
    else {
      *(undefined4 *)((int)&mp_fadetoblack.m_Value.m_StringLength + unaff_EBX + 2) =
           **(undefined4 **)(&DAT_00ae8232 + unaff_EBX);
      **(int **)(&DAT_00ae8232 + unaff_EBX) = iVar3;
    }
  }
  SendTable::SendTable(this);
  ___cxa_atexit(unaff_EBX + 0x8d7082,0,*(undefined4 *)(&DAT_00ae8192 + unaff_EBX));
  iVar3 = ServerClassInit<DT_PaintSprayer::ignored>((ignored *)0x0);
  *(int *)((int)&PTR_IsMouseClickEnabled_00c6ba8c + unaff_EBX + 2) = iVar3;
  pdVar4 = DataMapInit<CPaintSprayer>((CPaintSprayer *)0x0);
  *(datamap_t **)((int)&PTR_SetButtonActivationType_00c6ba90 + unaff_EBX + 2) = pdVar4;
  *(undefined **)(&mp_fadetoblack.field_0x36 + unaff_EBX) = &UNK_00b71992 + unaff_EBX;
  pIVar5 = EntityFactoryDictionary();
  (**pIVar5->_vptr_IEntityFactoryDictionary)(pIVar5,unaff_EBX + 0xcf4d76,unaff_EBX + 0x94cce9);
  return;
}


/* CPaintSprayer::GetServerClass at 006d0700 */

/* DWARF original prototype: ServerClass * GetServerClass(CPaintSprayer * this) */

ServerClass * __thiscall CPaintSprayer::GetServerClass(CPaintSprayer *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x6e8058);
}


/* CPaintSprayer::YouForgotToImplementOrDeclareServerClass at 006d0710 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CPaintSprayer * this) */

int __thiscall CPaintSprayer::YouForgotToImplementOrDeclareServerClass(CPaintSprayer *this)

{
  return 0;
}


/* __tcf_2 at 0099aa80 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x3949f0),in_stack_00000008);
  return;
}


/* __tcf_0 at 0099aa40 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  undefined4 *puVar1;
  
  ___i686_get_pc_thunk_bx();
  puVar1 = (undefined4 *)(unaff_EBX + 0x41e306);
  do {
    (**(code **)*puVar1)(puVar1);
    puVar1 = puVar1 + -0x15;
  } while (puVar1 != (undefined4 *)(unaff_EBX + 0x41dd1e));
  return;
}


/* DataMapInit<CPaintSprayer> at 000c37d0 */

datamap_t * DataMapInit<CPaintSprayer>(CPaintSprayer *param_1)

{
  int iVar1;
  int iVar2;
  char *pcVar3;
  size_t sVar4;
  int iVar5;
  undefined4 *puVar6;
  int iVar7;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((*(char *)((int)&sv_robust_explosions.m_fnChangeCallbacks.m_pElements + unaff_EBX) == '\0') &&
     (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcf4f94), iVar2 != 0)) {
    *(int *)((int)&sk_dmg_inflict_scale1.super_ConCommandBase.m_pNext + unaff_EBX) =
         unaff_EBX + 0x94cef5;
    *(undefined4 *)((int)&sk_dmg_inflict_scale1.super_ConCommandBase.m_pszName + unaff_EBX) = 0;
    *(undefined4 *)((int)&sk_dmg_inflict_scale1.super_ConCommandBase.m_pszHelpString + unaff_EBX) =
         0;
    *(undefined4 *)((int)&sk_dmg_inflict_scale1.super_ConCommandBase.m_nFlags + unaff_EBX) = 0;
    *(undefined4 *)((int)&sk_dmg_inflict_scale1.super_IConVar._vptr_IConVar + unaff_EBX) = 0;
    *(undefined4 *)((int)&sk_dmg_inflict_scale1.m_pParent + unaff_EBX) = 0;
    *(undefined4 *)(&sk_dmg_inflict_scale1.super_ConCommandBase.m_bRegistered + unaff_EBX) = 0xd;
    ___cxa_guard_release(unaff_EBX + 0xcf4f94);
    ___cxa_atexit(unaff_EBX + 0x8d72bc,0,*(undefined4 *)(&DAT_00ae83ac + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_CanStandOn_00c0824c + unaff_EBX) =
       *(undefined4 *)(&DAT_00ae84e8 + unaff_EBX);
  if (((&DAT_00cf4f9c)[unaff_EBX] == '\0') &&
     (iVar2 = ___cxa_guard_acquire(&DAT_00cf4f9c + unaff_EBX), iVar2 != 0)) {
    pcVar3 = operator_new___(*(int *)(&sk_dmg_inflict_scale1.super_ConCommandBase.m_bRegistered +
                                     unaff_EBX) + 0x12);
    _strcpy(pcVar3,*(char **)((int)&sk_dmg_inflict_scale1.super_ConCommandBase.m_pNext + unaff_EBX))
    ;
    sVar4 = _strlen(pcVar3);
    builtin_strncpy(pcVar3 + sVar4,"PaintSprayerThink",0x12);
    iVar1 = *(int *)((int)&sk_dmg_inflict_scale1.super_IConVar._vptr_IConVar + unaff_EBX);
    iVar2 = iVar1 + 1;
    iVar7 = *(int *)((int)&sk_dmg_inflict_scale1.super_ConCommandBase.m_pszHelpString + unaff_EBX);
    if (iVar7 < iVar2) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)
                 ((int)&sk_dmg_inflict_scale1.super_ConCommandBase.m_pszName + unaff_EBX),
                 iVar2 - iVar7);
    }
    iVar5 = *(int *)((int)&sk_dmg_inflict_scale1.super_IConVar._vptr_IConVar + unaff_EBX) + 1;
    *(int *)((int)&sk_dmg_inflict_scale1.super_IConVar._vptr_IConVar + unaff_EBX) = iVar5;
    iVar7 = *(int *)((int)&sk_dmg_inflict_scale1.super_ConCommandBase.m_pszName + unaff_EBX);
    *(int *)((int)&sk_dmg_inflict_scale1.m_pParent + unaff_EBX) = iVar7;
    iVar5 = (iVar5 - iVar1) + -1;
    if (0 < iVar5) {
      _V_memmove((void *)(iVar7 + iVar2 * 4),(void *)(iVar7 + iVar1 * 4),iVar5 * 4);
      iVar7 = *(int *)((int)&sk_dmg_inflict_scale1.super_ConCommandBase.m_pszName + unaff_EBX);
    }
    puVar6 = (undefined4 *)(iVar1 * 4 + iVar7);
    if (puVar6 != (undefined4 *)0x0) {
      *puVar6 = pcVar3;
    }
    *(char **)(&CEntityFactory<CLogicBranchList>::vtable + unaff_EBX) = pcVar3;
    ___cxa_guard_release(&DAT_00cf4f9c + unaff_EBX);
  }
  *(undefined4 *)((int)&PTR_OnEntityEvent_00c08244 + unaff_EBX) = 0x12;
  *(int *)((int)&PTR_ChangeTeam_00c08240 + unaff_EBX) = (int)&PTR_FireBullets_00c0829c + unaff_EBX;
  return (datamap_t *)((int)&PTR_ChangeTeam_00c08240 + unaff_EBX);
}


/* CPaintSprayer::GetDataDescMap at 006d0720 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPaintSprayer * this) */

datamap_t * __thiscall CPaintSprayer::GetDataDescMap(CPaintSprayer *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5fb2fc);
}


/* CPaintSprayer::GetBaseMap at 006d0730 */

datamap_t * CPaintSprayer::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4db594);
}


/* __tcf_1 at 0099aaa0 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41dcee)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41dce2) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41dcee));
  }
  *(undefined4 *)(unaff_EBX + 0x41dcee) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41dcea)) {
    if (*(int *)(unaff_EBX + 0x41dce2) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2110ea) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2110ea),*(int *)(unaff_EBX + 0x41dce2));
      *(undefined4 *)(unaff_EBX + 0x41dce2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41dce6) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41dce2);
  *(int *)(unaff_EBX + 0x41dcf2) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41dcea)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2110ea) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2110ea),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41dce2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41dce6) = 0;
  }
  return;
}


/* CPaintSprayer::CPaintSprayer at 006d0a00 */

/* DWARF original prototype: void CPaintSprayer(CPaintSprayer * this) */

void __thiscall CPaintSprayer::CPaintSprayer(CPaintSprayer *this)

{
  undefined1 *puVar1;
  char cVar2;
  CBaseEdict *pCVar3;
  IChangeInfoAccessor *pIVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::CBaseEntity(&this->super_CBaseEntity,false);
  (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x564634);
  this->m_bStartActive = false;
  this->m_flLastThinkTime = 0.0;
  this->m_flAccumulatedTime = 0.0;
  this->m_nBlobSpawned = 0;
  (this->m_bActive).m_Value = false;
  if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
    pCVar3 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
    if (pCVar3 != (CBaseEdict *)0x0) {
      pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
      pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
      pIVar4->m_iChangeInfoSerialNumber = 0;
      (this->m_flStartTime).m_Value = 0.0;
      cVar2 = (this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered;
      goto joined_r0x006d0aa9;
    }
  }
  else {
    puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  (this->m_flStartTime).m_Value = 0.0;
  cVar2 = (this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered;
joined_r0x006d0aa9:
  if (cVar2 == '\0') {
    pCVar3 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
    if (pCVar3 != (CBaseEdict *)0x0) {
      pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
      pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
      pIVar4->m_iChangeInfoSerialNumber = 0;
    }
  }
  else {
    puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  return;
}


/* CPaintSprayer::CPaintSprayer at 006d0ae0 */

/* DWARF original prototype: void CPaintSprayer(CPaintSprayer * this, CPaintSprayer * this) */

void __thiscall CPaintSprayer::CPaintSprayer(CPaintSprayer *this,CPaintSprayer *this_1)

{
  CPaintSprayer(this);
  return;
}


/* CEntityFactory<CPaintSprayer>::Create at 006d1240 */

/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CPaintSprayer> * this, char
   * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CPaintSprayer>::Create(CEntityFactory<CPaintSprayer> *this,char *pClassName)

{
  CPaintSprayer *this_00;
  
                    /* Unresolved local var: CPaintSprayer * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x404);
  CPaintSprayer::CPaintSprayer(this_00);
  (*(this_00->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x1d])(this_00,pClassName);
  return &(this_00->super_CBaseEntity).m_Network.super_IServerNetworkable;
}


/* CPaintSprayer::~CPaintSprayer at 006d08a0 */

/* DWARF original prototype: void ~CPaintSprayer(CPaintSprayer * this, int __in_chrg) */

void __thiscall CPaintSprayer::~CPaintSprayer(CPaintSprayer *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(&UNK_00564797 + unaff_EBX);
  CBaseEntity::~CBaseEntity(&this->super_CBaseEntity,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CPaintSprayer::~CPaintSprayer at 006d08e0 */

/* DWARF original prototype: void ~CPaintSprayer(CPaintSprayer * this, int __in_chrg) */

void __thiscall CPaintSprayer::~CPaintSprayer(CPaintSprayer *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(&UNK_00564760 + extraout_ECX);
  CBaseEntity::~CBaseEntity(&this->super_CBaseEntity,__in_chrg);
  return;
}


/* CPaintSprayer::~CPaintSprayer at 006d0900 */

/* DWARF original prototype: void ~CPaintSprayer(CPaintSprayer * this, int __in_chrg) */

void __thiscall CPaintSprayer::~CPaintSprayer(CPaintSprayer *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(&UNK_00564740 + extraout_ECX);
  CBaseEntity::~CBaseEntity(&this->super_CBaseEntity,__in_chrg);
  return;
}


/* CPaintSprayer::Precache at 006d0810 */

/* DWARF original prototype: void Precache(CPaintSprayer * this) */

void __thiscall CPaintSprayer::Precache(CPaintSprayer *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x2e80ac));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x33fd6c));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x33fd98));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x33fdc4));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x2e829b));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x2e82b1));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x33fdf0));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x2e82c7));
  return;
}


/* CPaintSprayer::Spawn at 006d10e0 */

/* DWARF original prototype: void Spawn(CPaintSprayer * this) */

void __thiscall CPaintSprayer::Spawn(CPaintSprayer *this)

{
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::Spawn(&this->super_CBaseEntity);
  (*(this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x1a])(this);
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x9b4);
  CBaseEntity::ThinkSet(&this->super_CBaseEntity,func,0.0,(char *)0x0);
  CBaseEntity::SetNextThink(&this->super_CBaseEntity,-1.0,(char *)0x0);
  if (this->m_bStartActive == false) {
    return;
  }
  StartPaintSpray(this);
  return;
}


/* CPaintSprayer::Activate at 006d1090 */

/* DWARF original prototype: void Activate(CPaintSprayer * this) */

void __thiscall CPaintSprayer::Activate(CPaintSprayer *this)

{
  CBaseEntity::Activate(&this->super_CBaseEntity);
  if (this->m_bStartActive == false) {
    return;
  }
  this->m_bStartActive = false;
  StartPaintSpray(this);
  return;
}


/* CPaintSprayer::SetPaintSprayerData at 006d0af0 */

/* DWARF original prototype: void SetPaintSprayerData(CPaintSprayer * this, bool bSilent, float
   flBlobsPerSecond, float flMinSpeed, float flMaxSpeed, float flBlobSpreadRadius, float
   flBlobSpreadAngle, float flBlobStreakPercent, float flMinStreakTime, float flMaxStreakTime, float
   flMinStreakSpeedDampen, float flMaxStreakSpeedDampen) */

void __thiscall
CPaintSprayer::SetPaintSprayerData
          (CPaintSprayer *this,bool bSilent,float flBlobsPerSecond,float flMinSpeed,float flMaxSpeed
          ,float flBlobSpreadRadius,float flBlobSpreadAngle,float flBlobStreakPercent,
          float flMinStreakTime,float flMaxStreakTime,float flMinStreakSpeedDampen,
          float flMaxStreakSpeedDampen)

{
  undefined1 *puVar1;
  CBaseEdict *pCVar2;
  IChangeInfoAccessor *pIVar3;
  
  if (bSilent != (this->m_bSilent).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar2 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bSilent).m_Value = bSilent;
  }
  if (flBlobsPerSecond != (this->m_flBlobsPerSecond).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar2 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flBlobsPerSecond).m_Value = flBlobsPerSecond;
  }
  if (flMinSpeed != (this->m_flMinSpeed).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar2 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flMinSpeed).m_Value = flMinSpeed;
  }
  if (flMaxSpeed != (this->m_flMaxSpeed).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar2 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flMaxSpeed).m_Value = flMaxSpeed;
  }
  if (flBlobSpreadRadius != (this->m_flBlobSpreadRadius).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar2 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flBlobSpreadRadius).m_Value = flBlobSpreadRadius;
  }
  if (flBlobSpreadAngle != (this->m_flBlobSpreadAngle).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar2 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flBlobSpreadAngle).m_Value = flBlobSpreadAngle;
  }
  if (flBlobStreakPercent != (this->m_flStreakPercentage).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar2 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flStreakPercentage).m_Value = flBlobStreakPercent;
  }
  if (flMinStreakTime != (this->m_flMinStreakTime).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar2 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flMinStreakTime).m_Value = flMinStreakTime;
  }
  if (flMaxStreakTime != (this->m_flMaxStreakTime).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar2 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flMaxStreakTime).m_Value = flMaxStreakTime;
  }
  if (flMinStreakSpeedDampen != (this->m_flMinStreakSpeedDampen).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar2 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flMinStreakSpeedDampen).m_Value = flMinStreakSpeedDampen;
  }
  if (flMaxStreakSpeedDampen != (this->m_flMaxStreakSpeedDampen).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar2 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flMaxStreakSpeedDampen).m_Value = flMaxStreakSpeedDampen;
  }
  return;
}


/* CPaintSprayer::SetPaintPowerType at 006d09a0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void SetPaintPowerType(CPaintSprayer * this, PaintPowerType paintType)
    */

void __thiscall CPaintSprayer::SetPaintPowerType(CPaintSprayer *this,PaintPowerType paintType)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  
  if (paintType != (this->m_PaintPowerType).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar2->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_PaintPowerType).m_Value = paintType;
  }
  return;
}


/* CPaintSprayer::UpdateTransmitState at 006d07c0 */

/* DWARF original prototype: int UpdateTransmitState(CPaintSprayer * this) */

int __thiscall CPaintSprayer::UpdateTransmitState(CPaintSprayer *this)

{
  int iVar1;
  
  if ((this->m_bSilent).m_Value != false) {
    iVar1 = CBaseEntity::SetTransmitState(&this->super_CBaseEntity,0x10);
    return iVar1;
  }
  iVar1 = CBaseEntity::SetTransmitState(&this->super_CBaseEntity,8);
  return iVar1;
}


/* CPaintSprayer::PaintSprayerThink at 006d0740 */

/* DWARF original prototype: void PaintSprayerThink(CPaintSprayer * this) */

void __thiscall CPaintSprayer::PaintSprayerThink(CPaintSprayer *this)

{
  float fVar1;
  float fVar2;
  int *piVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((this->m_bActive).m_Value != false) {
                    /* Unresolved local var: float flDeltaTime@[???] */
    piVar3 = *(int **)(unaff_EBX + 0x4db44c);
    fVar1 = *(float *)(*piVar3 + 0xc);
    fVar2 = this->m_flLastThinkTime;
    this->m_flLastThinkTime = fVar1;
    SprayPaint(this,fVar1 - fVar2);
    CBaseEntity::SetNextThink(&this->super_CBaseEntity,*(float *)(*piVar3 + 0xc),(char *)0x0);
  }
  return;
}


/* CPaintSprayer::InputStart at 006d1080 */

/* DWARF original prototype: void InputStart(CPaintSprayer * this, inputdata_t * inputData) */

void __thiscall CPaintSprayer::InputStart(CPaintSprayer *this,inputdata_t *inputData)

{
  StartPaintSpray(this);
  return;
}


/* CPaintSprayer::InputStop at 006d0920 */

/* DWARF original prototype: void InputStop(CPaintSprayer * this, inputdata_t * inputData) */

void __thiscall CPaintSprayer::InputStop(CPaintSprayer *this,inputdata_t *inputData)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  
  if ((this->m_bActive).m_Value != false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar2->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bActive).m_Value = false;
  }
  CBaseEntity::SetNextThink(&this->super_CBaseEntity,-1.0,(char *)0x0);
  return;
}


/* CPaintSprayer::InputChangePaintType at 006d1180 */

/* DWARF original prototype: void InputChangePaintType(CPaintSprayer * this, inputdata_t *
   inputData) */

void __thiscall CPaintSprayer::InputChangePaintType(CPaintSprayer *this,inputdata_t *inputData)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  int iVar3;
  
  iVar3 = 0;
  if ((inputData->value).fieldType == FIELD_INTEGER) {
    iVar3 = (inputData->value).field_0.iVal;
  }
  if (iVar3 != (this->m_PaintPowerType).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar2->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_PaintPowerType).m_Value = iVar3;
  }
  return;
}


/* CPaintSprayer::StartPaintSpray at 006d0f60 */

/* DWARF original prototype: void StartPaintSpray(CPaintSprayer * this) */

void __thiscall CPaintSprayer::StartPaintSpray(CPaintSprayer *this)

{
  undefined1 *puVar1;
  float fVar2;
  CBaseEdict *pCVar3;
  int *piVar4;
  IChangeInfoAccessor *pIVar5;
  int iVar6;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((this->m_bActive).m_Value == false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar3 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar3 != (CBaseEdict *)0x0) {
        pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
        pIVar5 = CBaseEdict::GetChangeAccessor(pCVar3);
        pIVar5->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bActive).m_Value = true;
    this->m_flAccumulatedTime =
         *(float *)(unaff_EBX + 0x35939c) / (this->m_flBlobsPerSecond).m_Value;
    piVar4 = *(int **)(unaff_EBX + 0x4dac2c);
    iVar6 = *piVar4;
    fVar2 = *(float *)(iVar6 + 0xc);
    if (fVar2 != (this->m_flStartTime).m_Value) {
      if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
        pCVar3 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
        if (pCVar3 != (CBaseEdict *)0x0) {
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
          pIVar5 = CBaseEdict::GetChangeAccessor(pCVar3);
          pIVar5->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->m_flStartTime).m_Value = fVar2;
      iVar6 = *piVar4;
    }
    this->m_flLastThinkTime = *(float *)(iVar6 + 0xc);
    this->m_nBlobSpawned = 0;
    CBaseEntity::SetNextThink(&this->super_CBaseEntity,*(float *)(*piVar4 + 0xc),(char *)0x0);
  }
  return;
}


/* CPaintSprayer::StopPaintSpray at 006d0ee0 */

/* DWARF original prototype: void StopPaintSpray(CPaintSprayer * this) */

void __thiscall CPaintSprayer::StopPaintSpray(CPaintSprayer *this)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  
  if ((this->m_bActive).m_Value != false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar2->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bActive).m_Value = false;
  }
  CBaseEntity::SetNextThink(&this->super_CBaseEntity,-1.0,(char *)0x0);
  return;
}


/* _GLOBAL__I__ZN13CPaintSprayer14GetServerClassEv at 000c3c60 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN13CPaintSprayer14GetServerClassEv(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

