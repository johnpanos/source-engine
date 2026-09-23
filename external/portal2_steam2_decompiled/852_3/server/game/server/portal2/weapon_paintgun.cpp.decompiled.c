/* DWARF-guided pseudocode for game/server/portal2/weapon_paintgun.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* CWeaponPaintGun::GetServerClass at 006fcc50 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: ServerClass * GetServerClass(CWeaponPaintGun * this) */

ServerClass * __thiscall CWeaponPaintGun::GetServerClass(CWeaponPaintGun *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x6bfd88);
}


/* CWeaponPaintGun::YouForgotToImplementOrDeclareServerClass at 006fcc60 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CWeaponPaintGun * this) */

int __thiscall CWeaponPaintGun::YouForgotToImplementOrDeclareServerClass(CWeaponPaintGun *this)

{
  return 0;
}


/* ServerClassInit<DT_WeaponPaintGun::ignored> at 000d0d40 */

int ServerClassInit<DT_WeaponPaintGun::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  char *pVarName;
  SendProp local_70;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&rgdpvpreset[6].spinup + unaff_EBX + 3) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcebd07);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)((int)&rgdpvpreset[6].volrun + unaff_EBX + 3),
                  (char *)(unaff_EBX + 0x8e1cd3),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)((int)&rgdpvpreset[7].pitchrun + unaff_EBX + 3),
                        (char *)(unaff_EBX + 0x8e4043),0,
                        (SendTable *)**(undefined4 **)(&DAT_00adb987 + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00adaedb + unaff_EBX),0x80);
      pVarName = (char *)(unaff_EBX + 0x943399);
      SendPropBool((SendProp_conflict *)&local_70,pVarName,0x644,1);
      SendPropArray3((SendProp_conflict *)((int)&rgdpvpreset[7].volfrac + unaff_EBX + 3),pVarName,
                     0x644,1,5,(SendProp_conflict *)&local_70,
                     *(SendTableProxyFn_conflict *)(&DAT_00adaedb + unaff_EBX),0x80);
      SendProp::~SendProp(&local_70,(int)pVarName);
      SendPropInt((SendProp_conflict *)((int)&rgdpvpreset[8].pitchfrac + unaff_EBX + 3),
                  (char *)(unaff_EBX + 0x9433a5),0x650,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropBool((SendProp_conflict *)((int)&rgdpvpreset[9].cspincount + unaff_EBX + 3),
                   &UNK_009433b5 + unaff_EBX,0x654,1);
      SendPropBool((SendProp_conflict *)((int)&rgdpvpreset[10].lforate + unaff_EBX + 3),
                   (char *)(vgui::ScalableImagePanel::GetAnimMap + unaff_EBX + 4),0x655,1);
      ___cxa_guard_release(unaff_EBX + 0xcebd07);
      ___cxa_atexit(unaff_EBX + 0x8cd07f,0,*(undefined4 *)(&DAT_00adae3f + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)((int)&PTR_NavigateUp_00c5fa2c + unaff_EBX + 3),
             (SendProp_conflict *)((int)&rgdpvpreset[7].pitchrun + unaff_EBX + 3),5,
             *(char **)(&DAT_00b71113 + unaff_EBX));
  return 1;
}


/* __static_initialization_and_destruction_0 at 000d1270 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *s2;
  SendTable *this;
  undefined4 *puVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  datamap_t *pdVar5;
  IEntityFactoryDictionary *pIVar6;
  int unaff_EBX;
  undefined4 *puVar7;
  undefined4 *puVar8;
  longlong lVar9;
  
  lVar9 = ___i686_get_pc_thunk_bx();
  if (lVar9 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)&g_StandoffParamsByAgression[3].maxShots + unaff_EBX + 2) = 0;
  *(undefined1 *)((int)&g_StandoffParamsByAgression[3].maxShots + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&g_StandoffParamsByAgression[3].oddsCover + unaff_EBX) = 0;
  *(undefined1 *)((int)&g_StandoffParamsByAgression[3].oddsCover + unaff_EBX + 1) = 0;
  *(undefined4 *)((int)&g_StandoffParamsByAgression[3].oddsCover + unaff_EBX + 2) = 0;
  *(undefined4 *)(&g_StandoffParamsByAgression[3].field_0x1e + unaff_EBX) = 0;
  *(undefined4 *)((int)&g_StandoffParamsByAgression[3].flAbandonTimeLimit + unaff_EBX + 2) =
       0x7f7fffff;
  *(undefined4 *)((int)&g_StandoffParamsByAgression[4].hintChangeReaction + unaff_EBX + 2) =
       0x7f7fffff;
  *(undefined4 *)(&g_StandoffParamsByAgression[4].field_0x6 + unaff_EBX) = 0;
  *(undefined4 *)((int)&g_StandoffParamsByAgression[4].minTimeShots + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&g_StandoffParamsByAgression[4].maxTimeShots + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&g_StandoffParamsByAgression[4].minShots + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&g_StandoffParamsByAgression[4].maxShots + unaff_EBX + 2) = 0x7f7fffff;
  *(undefined4 *)((int)&g_StandoffParamsByAgression[4].oddsCover + unaff_EBX + 2) = 0x7f7fffff;
  *(undefined4 *)(&g_StandoffParamsByAgression[4].field_0x1e + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&g_StandoffParamsByAgression[4].flAbandonTimeLimit + unaff_EBX + 2) =
       0x7f7fffff;
  *(undefined4 *)(&DAT_00ceb736 + unaff_EBX) = 0;
  (&DAT_00ceb73d)[unaff_EBX] = 0;
  (&DAT_00ceb73a)[unaff_EBX] = 0;
  (&DAT_00ceb73b)[unaff_EBX] = 0;
  (&DAT_00ceb73c)[unaff_EBX] = 0;
  *(undefined4 *)(&DAT_00ceb73e + unaff_EBX) = 0;
  (&AI_DEFAULT_STANDOFF_PARAMS.fPlayerIsBattleline)[unaff_EBX] = false;
  *(undefined1 *)((int)&AI_DEFAULT_STANDOFF_PARAMS.hintChangeReaction + unaff_EBX + 2) = 0;
  *(undefined1 *)((int)&AI_DEFAULT_STANDOFF_PARAMS.hintChangeReaction + unaff_EBX + 3) = 0;
  (&AI_DEFAULT_STANDOFF_PARAMS.fCoverOnReload)[unaff_EBX] = false;
  *(undefined4 *)(&AI_DEFAULT_STANDOFF_PARAMS.field_0x6 + unaff_EBX) = 0;
  *(undefined1 *)((int)&AI_DEFAULT_STANDOFF_PARAMS.maxTimeShots + unaff_EBX + 1) = 1;
  *(undefined1 *)((int)&AI_DEFAULT_STANDOFF_PARAMS.minTimeShots + unaff_EBX + 2) = 0;
  *(undefined1 *)((int)&AI_DEFAULT_STANDOFF_PARAMS.minTimeShots + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&AI_DEFAULT_STANDOFF_PARAMS.maxTimeShots + unaff_EBX) = 0;
  *(undefined4 *)((int)&AI_DEFAULT_STANDOFF_PARAMS.maxTimeShots + unaff_EBX + 2) = 1;
  *(undefined1 *)((int)&AI_DEFAULT_STANDOFF_PARAMS.maxShots + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&AI_DEFAULT_STANDOFF_PARAMS.minShots + unaff_EBX + 2) = 0;
  *(undefined1 *)((int)&AI_DEFAULT_STANDOFF_PARAMS.minShots + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&AI_DEFAULT_STANDOFF_PARAMS.maxShots + unaff_EBX) = 0;
  *(undefined4 *)((int)&AI_DEFAULT_STANDOFF_PARAMS.maxShots + unaff_EBX + 2) = 2;
  (&AI_DEFAULT_STANDOFF_PARAMS.field_0x1d)[unaff_EBX] = 0;
  *(undefined1 *)((int)&AI_DEFAULT_STANDOFF_PARAMS.oddsCover + unaff_EBX + 2) = 0;
  *(undefined1 *)((int)&AI_DEFAULT_STANDOFF_PARAMS.oddsCover + unaff_EBX + 3) = 0;
  (&AI_DEFAULT_STANDOFF_PARAMS.fStayAtCover)[unaff_EBX] = false;
  *(undefined **)(&AI_DEFAULT_STANDOFF_PARAMS.field_0x1e + unaff_EBX) = &UNK_00ae00aa + unaff_EBX;
  iVar4 = unaff_EBX + 0xceb762;
  s2 = (char *)(unaff_EBX + 0x942ea6);
  *(char **)((int)&AI_DEFAULT_STANDOFF_PARAMS.flAbandonTimeLimit + unaff_EBX + 2) = s2;
  this = (SendTable *)((int)&PTR_IsBlockingDragChaining_00c5f500 + unaff_EBX + 2);
  *(SendTable **)(&DAT_00ceb766 + unaff_EBX) = this;
  *(undefined4 *)(&DAT_00ceb772 + unaff_EBX) = 0xffff;
  puVar8 = (undefined4 *)**(int **)(&DAT_00ada9b2 + unaff_EBX);
  if (puVar8 == (undefined4 *)0x0) {
    **(int **)(&DAT_00ada9b2 + unaff_EBX) = iVar4;
    *(undefined4 *)(&DAT_00ceb76a + unaff_EBX) = 0;
  }
  else {
    puVar1 = (undefined4 *)puVar8[2];
    iVar3 = _V_stricmp((char *)*puVar8,s2);
    if (iVar3 < 1) {
      while ((puVar7 = puVar1, puVar7 != (undefined4 *)0x0 &&
             (iVar3 = _V_stricmp((char *)*puVar7,s2), iVar3 < 1))) {
        puVar1 = (undefined4 *)puVar7[2];
        puVar8 = puVar7;
      }
      *(undefined4 **)(&DAT_00ceb76a + unaff_EBX) = puVar7;
      puVar8[2] = iVar4;
    }
    else {
      *(undefined4 *)(&DAT_00ceb76a + unaff_EBX) = **(undefined4 **)(&DAT_00ada9b2 + unaff_EBX);
      **(int **)(&DAT_00ada9b2 + unaff_EBX) = iVar4;
    }
  }
  SendTable::SendTable(this);
  uVar2 = *(undefined4 *)(&DAT_00ada912 + unaff_EBX);
  ___cxa_atexit(unaff_EBX + 0x8ccbf2,0,uVar2);
  iVar4 = ServerClassInit<DT_WeaponPaintGun::ignored>((ignored *)0x0);
  *(int *)((int)&PTR_GetDropContextMenu_00c5f514 + unaff_EBX + 2) = iVar4;
  pdVar5 = DataMapInit<CWeaponPaintGun>((CWeaponPaintGun *)0x0);
  *(datamap_t **)((int)&PTR_OnDropContextHoverShow_00c5f518 + unaff_EBX + 2) = pdVar5;
  *(undefined **)(&DAT_00ceb776 + unaff_EBX) = &UNK_00b70bfa + unaff_EBX;
  pIVar6 = EntityFactoryDictionary();
  (**pIVar6->_vptr_IEntityFactoryDictionary)(pIVar6,&DAT_00ceb776 + unaff_EBX,unaff_EBX + 0x8e83c2);
  ConCommand::ConCommand
            ((ConCommand *)(unaff_EBX + 0xceb782),(char *)(unaff_EBX + 0x942eb6),
             (FnCommandCallbackV1_t)(unaff_EBX + 0x62c0c2),(char *)0x0,0,
             (FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x8ccbd2,0,uVar2);
  return;
}


/* __tcf_2 at 0099de70 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x392908),in_stack_00000008);
  return;
}


/* __tcf_0 at 0099ddd0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41ee27))(unaff_EBX + 0x41ee27);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41edd3))(unaff_EBX + 0x41edd3);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41ed7f))(unaff_EBX + 0x41ed7f);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41ed2b))(unaff_EBX + 0x41ed2b);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41ecd7))(unaff_EBX + 0x41ecd7);
                    /* WARNING: Could not recover jumptable at 0x0099de43. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41ec83))();
  return;
}


/* DataMapInit<CWeaponPaintGun> at 000d1050 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

datamap_t * DataMapInit<CWeaponPaintGun>(CWeaponPaintGun *param_1)

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
  if ((*(char *)((int)&g_ConceptCategoryInfos[0].maxGlobalDelay + unaff_EBX) == '\0') &&
     (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xceb9c4), iVar2 != 0)) {
    *(int *)((int)&g_ConceptCategoryInfos[1].maxGlobalDelay + unaff_EBX) = unaff_EBX + 0x9430c0;
    *(undefined4 *)((int)&g_ConceptCategoryInfos[1].maxPersonalDelay + unaff_EBX) = 0;
    *(undefined4 *)((int)&g_ConceptCategoryInfos[2].minGlobalDelay + unaff_EBX) = 0;
    *(undefined4 *)((int)&g_ConceptCategoryInfos[2].maxGlobalDelay + unaff_EBX) = 0;
    *(undefined4 *)((int)&g_ConceptCategoryInfos[2].minPersonalDelay + unaff_EBX) = 0;
    *(undefined4 *)((int)&g_ConceptCategoryInfos[2].maxPersonalDelay + unaff_EBX) = 0;
    *(undefined4 *)((int)&g_ConceptCategoryInfos[1].minPersonalDelay + unaff_EBX) = 0xf;
    ___cxa_guard_release(unaff_EBX + 0xceb9c4);
    ___cxa_atexit(unaff_EBX + 0x8cce2c,0,*(undefined4 *)(&DAT_00adab2c + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_EyeAngles_00c00dec + unaff_EBX) =
       *(undefined4 *)(&DAT_00adb670 + unaff_EBX);
  if ((*(char *)((int)&g_ConceptCategoryInfos[0].maxPersonalDelay + unaff_EBX) == '\0') &&
     (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xceb9cc), iVar2 != 0)) {
    pcVar3 = operator_new___(*(int *)((int)&g_ConceptCategoryInfos[1].minPersonalDelay + unaff_EBX)
                             + 0xe);
    _strcpy(pcVar3,*(char **)((int)&g_ConceptCategoryInfos[1].maxGlobalDelay + unaff_EBX));
    sVar4 = _strlen(pcVar3);
    builtin_strncpy(pcVar3 + sVar4,"PaintGunThink",0xe);
    iVar1 = *(int *)((int)&g_ConceptCategoryInfos[2].minPersonalDelay + unaff_EBX);
    iVar2 = iVar1 + 1;
    iVar7 = *(int *)((int)&g_ConceptCategoryInfos[2].minGlobalDelay + unaff_EBX);
    if (iVar7 < iVar2) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)
                 ((int)&g_ConceptCategoryInfos[1].maxPersonalDelay + unaff_EBX),iVar2 - iVar7);
    }
    iVar5 = *(int *)((int)&g_ConceptCategoryInfos[2].minPersonalDelay + unaff_EBX) + 1;
    *(int *)((int)&g_ConceptCategoryInfos[2].minPersonalDelay + unaff_EBX) = iVar5;
    iVar7 = *(int *)((int)&g_ConceptCategoryInfos[1].maxPersonalDelay + unaff_EBX);
    *(int *)((int)&g_ConceptCategoryInfos[2].maxPersonalDelay + unaff_EBX) = iVar7;
    iVar5 = (iVar5 - iVar1) + -1;
    if (0 < iVar5) {
      _V_memmove((void *)(iVar7 + iVar2 * 4),(void *)(iVar7 + iVar1 * 4),iVar5 * 4);
      iVar7 = *(int *)((int)&g_ConceptCategoryInfos[1].maxPersonalDelay + unaff_EBX);
    }
    puVar6 = (undefined4 *)(iVar1 * 4 + iVar7);
    if (puVar6 != (undefined4 *)0x0) {
      *puVar6 = pcVar3;
    }
    *(char **)((int)&PTR_Ignite_00c00f40 + unaff_EBX) = pcVar3;
    ___cxa_guard_release(unaff_EBX + 0xceb9cc);
  }
  *(undefined4 *)((int)&PTR_SetDamage_00c00de4 + unaff_EBX) = 5;
  *(int *)((int)&PTR_GetDamage_00c00de0 + unaff_EBX) =
       (int)&PTR_WorldSpaceCenter_00c00e3c + unaff_EBX;
  return (datamap_t *)((int)&PTR_GetDamage_00c00de0 + unaff_EBX);
}


/* CWeaponPaintGun::GetDataDescMap at 006fcc70 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: datamap_t * GetDataDescMap(CWeaponPaintGun * this) */

datamap_t * __thiscall CWeaponPaintGun::GetDataDescMap(CWeaponPaintGun *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5d51cc);
}


/* CWeaponPaintGun::GetBaseMap at 006fcc80 */

datamap_t * CWeaponPaintGun::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4afa4c);
}


/* __tcf_1 at 0099de90 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41ebae)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41eba2) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41ebae));
  }
  *(undefined4 *)(unaff_EBX + 0x41ebae) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41ebaa)) {
    if (*(int *)(unaff_EBX + 0x41eba2) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20dcfa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20dcfa),*(int *)(unaff_EBX + 0x41eba2));
      *(undefined4 *)(unaff_EBX + 0x41eba2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41eba6) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41eba2);
  *(int *)(unaff_EBX + 0x41ebb2) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41ebaa)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20dcfa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20dcfa),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41eba2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41eba6) = 0;
  }
  return;
}


/* CWeaponPaintGun::CWeaponPaintGun at 006fd9c0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void CWeaponPaintGun(CWeaponPaintGun * this) */

void __thiscall CWeaponPaintGun::CWeaponPaintGun(CWeaponPaintGun *this)

{
  uint *puVar1;
  char cVar2;
  CBaseEdict *pCVar3;
  IChangeInfoAccessor *pIVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBasePortalCombatWeapon::CBasePortalCombatWeapon(&this->super_CBasePortalCombatWeapon);
  (this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon.
  super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x543eba);
  this->m_flAccumulatedTime = 0.0;
  (this->m_nCurrentColor).m_Value = 4;
  if (*(bool *)((int)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                      super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                      m_TimerEvent + 0x10) == false) {
    pCVar3 = &((this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon
               .super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if (pCVar3 == (CBaseEdict *)0x0) goto LAB_006fda03;
    pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
    pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
    pIVar4->m_iChangeInfoSerialNumber = 0;
    (this->m_bFiringPaint).m_Value = false;
    if (*(bool *)((int)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                        super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                        m_TimerEvent + 0x10) != false) goto LAB_006fda14;
LAB_006fdaa0:
    pCVar3 = &((this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon
               .super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if (pCVar3 != (CBaseEdict *)0x0) {
      pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
      pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
      pIVar4->m_iChangeInfoSerialNumber = 0;
      (this->m_bFiringErase).m_Value = false;
      cVar2 = *(bool *)((int)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                              super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.
                              m_Network.m_TimerEvent + 0x10);
      goto joined_r0x006fdaca;
    }
  }
  else {
    puVar1 = (uint *)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                      super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                      field_0x4c;
    *puVar1 = *puVar1 | 1;
LAB_006fda03:
    (this->m_bFiringPaint).m_Value = false;
    if (*(bool *)((int)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                        super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                        m_TimerEvent + 0x10) == false) goto LAB_006fdaa0;
LAB_006fda14:
    puVar1 = (uint *)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                      super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                      field_0x4c;
    *puVar1 = *puVar1 | 1;
  }
  (this->m_bFiringErase).m_Value = false;
  cVar2 = *(bool *)((int)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                          super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                          m_TimerEvent + 0x10);
joined_r0x006fdaca:
  if (cVar2 == '\0') {
    pCVar3 = &((this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon
               .super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if (pCVar3 != (CBaseEdict *)0x0) {
      pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
      pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
      pIVar4->m_iChangeInfoSerialNumber = 0;
    }
  }
  else {
    puVar1 = (uint *)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                      super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                      field_0x4c;
    *puVar1 = *puVar1 | 1;
  }
  (this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon.
  m_bFireOnEmpty = true;
  (this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon.
  m_bReloadsSingly = false;
  ResetPaint(this);
  ActivatePaint(this,NO_POWER);
  SetSubType(this,(this->m_nCurrentColor).m_Value);
  return;
}


/* CWeaponPaintGun::CWeaponPaintGun at 006fdb20 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void CWeaponPaintGun(CWeaponPaintGun * this, CWeaponPaintGun * this) */

void __thiscall CWeaponPaintGun::CWeaponPaintGun(CWeaponPaintGun *this,CWeaponPaintGun *this_1)

{
  CWeaponPaintGun(this);
  return;
}


/* CEntityFactory<CWeaponPaintGun>::Create at 006fdb70 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CWeaponPaintGun> * this,
   char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CWeaponPaintGun>::Create(CEntityFactory<CWeaponPaintGun> *this,char *pClassName)

{
  CWeaponPaintGun *this_00;
  
                    /* Unresolved local var: CWeaponPaintGun * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x664);
  CWeaponPaintGun::CWeaponPaintGun(this_00);
  (**(code **)((int)(this_00->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                    super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.
                    super_IServerEntity.super_IServerUnknown + 0x74))(this_00,pClassName);
  return &(this_00->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon.
          super_CBaseAnimating.super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CWeaponPaintGun::~CWeaponPaintGun at 006fd4b0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ~CWeaponPaintGun(CWeaponPaintGun * this, int __in_chrg) */

void __thiscall CWeaponPaintGun::~CWeaponPaintGun(CWeaponPaintGun *this,int __in_chrg)

{
  CSoundEnvelopeController *pCVar1;
  int unaff_EBX;
  CSoundPatch *in_stack_ffffffd8;
  CSoundPatch *pCVar2;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon.
  super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x5443c4);
                    /* Unresolved local var: CSoundEnvelopeController * controller@[???] */
  pCVar1 = CSoundEnvelopeController::GetController();
  pCVar2 = this->m_pLiquidLoop;
  if (pCVar2 != (CSoundPatch *)0x0) {
    (*pCVar1->_vptr_CSoundEnvelopeController[0xb])(pCVar1);
    this->m_pLiquidLoop = (CSoundPatch *)0x0;
    in_stack_ffffffd8 = pCVar2;
  }
  pCVar2 = this->m_pLiquidStart;
  if (pCVar2 != (CSoundPatch *)0x0) {
    (*pCVar1->_vptr_CSoundEnvelopeController[0xb])(pCVar1);
    this->m_pLiquidStart = (CSoundPatch *)0x0;
    in_stack_ffffffd8 = pCVar2;
  }
  (this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon.
  super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(*(int *)(unaff_EBX + 0x4aea3c) + 8);
  (this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon.
  super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(*(int *)(unaff_EBX + 0x4aea1c) + 8);
  CBaseCombatWeapon::~CBaseCombatWeapon((CBaseCombatWeapon *)this,(int)in_stack_ffffffd8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CWeaponPaintGun::~CWeaponPaintGun at 006fd570 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ~CWeaponPaintGun(CWeaponPaintGun * this, int __in_chrg) */

void __thiscall CWeaponPaintGun::~CWeaponPaintGun(CWeaponPaintGun *this,int __in_chrg)

{
  CSoundEnvelopeController *pCVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon.
  super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x544304);
                    /* Unresolved local var: CSoundEnvelopeController * controller@[???] */
  pCVar1 = CSoundEnvelopeController::GetController();
  if (this->m_pLiquidLoop != (CSoundPatch *)0x0) {
    (*pCVar1->_vptr_CSoundEnvelopeController[0xb])(pCVar1,this->m_pLiquidLoop);
    this->m_pLiquidLoop = (CSoundPatch *)0x0;
  }
  if (this->m_pLiquidStart != (CSoundPatch *)0x0) {
    (*pCVar1->_vptr_CSoundEnvelopeController[0xb])(pCVar1,this->m_pLiquidStart);
    this->m_pLiquidStart = (CSoundPatch *)0x0;
  }
  (this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon.
  super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(*(int *)(unaff_EBX + 0x4ae97c) + 8);
  (this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon.
  super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(*(int *)(unaff_EBX + 0x4ae95c) + 8);
  CBaseCombatWeapon::~CBaseCombatWeapon((CBaseCombatWeapon *)this,__in_chrg);
  return;
}


/* CWeaponPaintGun::~CWeaponPaintGun at 006fd630 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ~CWeaponPaintGun(CWeaponPaintGun * this, int __in_chrg,
   CWeaponPaintGun * this) */

void __thiscall
CWeaponPaintGun::~CWeaponPaintGun(CWeaponPaintGun *this,int __in_chrg,CWeaponPaintGun *this_1)

{
  ~CWeaponPaintGun(this,__in_chrg);
  return;
}


/* CWeaponPaintGun::Spawn at 006fcdf0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(CWeaponPaintGun * this) */

void __thiscall CWeaponPaintGun::Spawn(CWeaponPaintGun *this)

{
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (**(code **)((int)(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                    super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.
                    super_IServerEntity.super_IServerUnknown + 0x68))(this);
  CWeaponPortalBase::Spawn((CWeaponPortalBase *)this);
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0x83);
  CBaseEntity::ThinkSet
            ((CBaseEntity *)this,func,*(float *)(**(int **)(unaff_EBX + 0x4aeda3) + 0xc),
             (char *)(unaff_EBX + 0x3172dd));
  this->m_flAccumulatedTime =
       *(float *)(unaff_EBX + 0x32d513) /
       *(float *)(*(int *)(*(int *)(unaff_EBX + 0x4af943) + 0x1c) + 0x2c);
  this->m_nBlobRandomSeed = 0;
  return;
}


/* CWeaponPaintGun::Precache at 006fcd30 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void Precache(CWeaponPaintGun * this) */

void __thiscall CWeaponPaintGun::Precache(CWeaponPaintGun *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x31736f));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x317381));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x2bbbfc));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x2bbb8b));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x31384b));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x313877));
  CBaseEntity::PrecacheModel((char *)(CAI_AssaultBehavior::InitCustomSchedules + unaff_EBX + 3));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x2bbd7a));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x2bbd90));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x3138cf));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x2bbda6));
  CBaseCombatWeapon::Precache((CBaseCombatWeapon *)this);
  return;
}


/* CWeaponPaintGun::SendWeaponAnim at 006fcf60 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: bool SendWeaponAnim(CWeaponPaintGun * this, int iActivity) */

bool __thiscall CWeaponPaintGun::SendWeaponAnim(CWeaponPaintGun *this,int iActivity)

{
  bool bVar1;
  
                    /* Unresolved local var: int newActivity@[???] */
  if (iActivity == 0xbe) {
    iActivity = (-(uint)((this->m_bHasPaint).m_Value[(this->m_nCurrentColor).m_Value] == false) & 3)
                + 0xbe;
  }
  bVar1 = CBaseCombatWeapon::SendWeaponAnim((CBaseCombatWeapon *)this,iActivity);
  return bVar1;
}


/* CWeaponPaintGun::HasCurrentColor at 006fcc90 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: bool HasCurrentColor(CWeaponPaintGun * this) */

bool __thiscall CWeaponPaintGun::HasCurrentColor(CWeaponPaintGun *this)

{
  return (this->m_bHasPaint).m_Value[(this->m_nCurrentColor).m_Value];
}


/* CWeaponPaintGun::StartShootingSound at 006fd020 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void StartShootingSound(CWeaponPaintGun * this) */

void __thiscall CWeaponPaintGun::StartShootingSound(CWeaponPaintGun *this)

{
  edict_t *peVar1;
  CSoundEnvelopeController *pCVar2;
  CSoundPatch *pCVar3;
  int iVar4;
  int unaff_EBX;
  CRecipientFilter local_54;
  Vector local_34;
  Vector local_28 [2];
  
                    /* Unresolved local var: CSoundEnvelopeController * controller@[???] */
  ___i686_get_pc_thunk_bx();
  pCVar2 = CSoundEnvelopeController::GetController();
  if (this->m_pLiquidLoop != (CSoundPatch *)0x0) {
    (*pCVar2->_vptr_CSoundEnvelopeController[0xb])(pCVar2,this->m_pLiquidLoop);
    this->m_pLiquidLoop = (CSoundPatch *)0x0;
  }
  (**(code **)((int)(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                    super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.
                    super_IServerEntity.super_IServerUnknown + 0x278))(local_28,this);
  CRecipientFilter::CRecipientFilter(&local_54);
  local_54.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(unaff_EBX + 0x4b115a);
  CRecipientFilter::AddRecipientsByPAS(&local_54,local_28);
  local_54.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(unaff_EBX + 0x4b111a);
  (**(code **)((int)(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                    super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.
                    super_IServerEntity.super_IServerUnknown + 0x278))(&local_34,this);
  CPASAttenuationFilter::Filter((CPASAttenuationFilter *)&local_54,&local_34,0.8);
  peVar1 = (this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon.
           super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev;
                    /* Unresolved local var: int edictIndex@[???] */
  iVar4 = 0;
  if (peVar1 != (edict_t *)0x0) {
    iVar4 = (int)peVar1 - *(int *)(**(int **)(unaff_EBX + 0x4aeb72) + 0x58) >> 4;
  }
  pCVar3 = (CSoundPatch *)
           (*pCVar2->_vptr_CSoundEnvelopeController[6])(pCVar2,&local_54,iVar4,unaff_EBX + 0x31707e)
  ;
  this->m_pLiquidLoop = pCVar3;
  (*pCVar2->_vptr_CSoundEnvelopeController[2])(pCVar2,pCVar3,0,0x42c80000,0);
  pCVar3 = this->m_pLiquidLoop;
  (*pCVar2->_vptr_CSoundEnvelopeController[0xd])(pCVar2,pCVar3,0x3f800000,0x3f800000);
  local_54.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(unaff_EBX + 0x4b115a);
  CRecipientFilter::~CRecipientFilter(&local_54,(int)pCVar3);
  return;
}


/* CWeaponPaintGun::StopShootingSound at 006fccf0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void StopShootingSound(CWeaponPaintGun * this) */

void __thiscall CWeaponPaintGun::StopShootingSound(CWeaponPaintGun *this)

{
  CSoundEnvelopeController *pCVar1;
  
  if (this->m_pLiquidLoop != (CSoundPatch *)0x0) {
                    /* Unresolved local var: CSoundEnvelopeController * controller@[???] */
    pCVar1 = CSoundEnvelopeController::GetController();
    (*pCVar1->_vptr_CSoundEnvelopeController[0xe])(pCVar1,this->m_pLiquidLoop,0x3f000000,0);
  }
  return;
}


/* CWeaponPaintGun::ActivatePaint at 006fd1b0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void ActivatePaint(CWeaponPaintGun * this, PaintPowerType nIndex) */

void __thiscall CWeaponPaintGun::ActivatePaint(CWeaponPaintGun *this,PaintPowerType nIndex)

{
  uint *puVar1;
  CBaseEdict *this_00;
  code *pcVar2;
  char cVar3;
  IChangeInfoAccessor *pIVar4;
  int *piVar5;
  CBaseCombatCharacter *pCVar6;
  undefined4 uVar7;
  int unaff_EBX;
  
                    /* Unresolved local var: IGameEvent * event@[???] */
  ___i686_get_pc_thunk_bx();
  if ((this->m_bHasPaint).m_Value[nIndex] != true) {
    if (*(bool *)((int)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                        super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                        m_TimerEvent + 0x10) == false) {
      this_00 = &((this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                  super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->
                 super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar4 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar4->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = (uint *)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                        super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                        field_0x4c;
      *puVar1 = *puVar1 | 1;
    }
    (this->m_bHasPaint).m_Value[nIndex] = true;
  }
  piVar5 = (int *)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4ae9fc) + 0x1c))
                            ((int *)**(undefined4 **)(unaff_EBX + 0x4ae9fc),unaff_EBX + 0x316f83,0,0
                            );
  if (((piVar5 != (int *)0x0) &&
      (pCVar6 = CBaseCombatWeapon::GetOwner((CBaseCombatWeapon *)this),
      pCVar6 != (CBaseCombatCharacter *)0x0)) &&
     (cVar3 = (**(code **)((int)(pCVar6->super_CBaseFlex).super_CBaseAnimatingOverlay.
                                super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
                                super_IServerUnknown.super_IHandleEntity + 0x154))(pCVar6),
     cVar3 != '\0')) {
    pcVar2 = *(code **)(*piVar5 + 0x30);
    uVar7 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4ae9f4) + 0x40))
                      ((int *)**(undefined4 **)(unaff_EBX + 0x4ae9f4),
                       (pCVar6->super_CBaseFlex).super_CBaseAnimatingOverlay.super_CBaseAnimating.
                       super_CBaseEntity.m_Network.m_pPev);
    (*pcVar2)(piVar5,unaff_EBX + 0x2b8fc8,uVar7);
                    /* Unresolved local var: int nPaintCount@[???]
                       Unresolved local var: int i@[???] */
    (**(code **)(*piVar5 + 0x30))
              (piVar5,unaff_EBX + 0x2c031e,
               (((((uint)(this->m_bHasPaint).m_Value[0] -
                  (uint)((this->m_bHasPaint).m_Value[1] == false)) -
                 (uint)((this->m_bHasPaint).m_Value[2] == false)) -
                (uint)((this->m_bHasPaint).m_Value[3] == false)) + 4) -
               (uint)((this->m_bHasPaint).m_Value[4] == false));
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4ae9fc) + 0x20))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4ae9fc),piVar5,0);
  }
  (**(code **)((int)(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                    super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.
                    super_IServerEntity.super_IServerUnknown + 0x3b0))(this,nIndex);
  return;
}


/* CWeaponPaintGun::GetPaintCount at 006fccb0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: int GetPaintCount(CWeaponPaintGun * this) */

int __thiscall CWeaponPaintGun::GetPaintCount(CWeaponPaintGun *this)

{
                    /* Unresolved local var: int nPaintCount@[???] */
                    /* Unresolved local var: int i@[???] */
  return (((((uint)(this->m_bHasPaint).m_Value[0] - (uint)((this->m_bHasPaint).m_Value[1] == false))
           - (uint)((this->m_bHasPaint).m_Value[2] == false)) -
          (uint)((this->m_bHasPaint).m_Value[3] == false)) + 4) -
         (uint)((this->m_bHasPaint).m_Value[4] == false);
}


/* CWeaponPaintGun::ResetPaint at 006fd790 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void ResetPaint(CWeaponPaintGun * this) */

void __thiscall CWeaponPaintGun::ResetPaint(CWeaponPaintGun *this)

{
  uint *puVar1;
  CBaseEdict *pCVar2;
  IChangeInfoAccessor *pIVar3;
  int unaff_EBX;
  int __in_chrg;
  CRecipientFilter local_40;
  bool local_1d;
  
                    /* Unresolved local var: CBroadcastRecipientFilter filter@[???] */
  ___i686_get_pc_thunk_bx();
  local_1d = false;
  if ((this->m_bHasPaint).m_Value[0] != false) {
    if (*(bool *)((int)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                        super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                        m_TimerEvent + 0x10) == false) {
      pCVar2 = &((this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                 super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->
                super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = (uint *)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                        super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                        field_0x4c;
      *puVar1 = *puVar1 | 1;
    }
    (this->m_bHasPaint).m_Value[0] = local_1d;
  }
  local_1d = false;
  if ((this->m_bHasPaint).m_Value[1] != false) {
    if (*(bool *)((int)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                        super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                        m_TimerEvent + 0x10) == false) {
      pCVar2 = &((this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                 super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->
                super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = (uint *)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                        super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                        field_0x4c;
      *puVar1 = *puVar1 | 1;
    }
    (this->m_bHasPaint).m_Value[1] = local_1d;
  }
  local_1d = false;
  if ((this->m_bHasPaint).m_Value[2] != false) {
    if (*(bool *)((int)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                        super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                        m_TimerEvent + 0x10) == false) {
      pCVar2 = &((this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                 super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->
                super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = (uint *)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                        super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                        field_0x4c;
      *puVar1 = *puVar1 | 1;
    }
    (this->m_bHasPaint).m_Value[2] = local_1d;
  }
  local_1d = false;
  if ((this->m_bHasPaint).m_Value[3] != false) {
    if (*(bool *)((int)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                        super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                        m_TimerEvent + 0x10) == false) {
      pCVar2 = &((this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                 super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->
                super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = (uint *)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                        super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                        field_0x4c;
      *puVar1 = *puVar1 | 1;
    }
    (this->m_bHasPaint).m_Value[3] = local_1d;
  }
  CRecipientFilter::CRecipientFilter(&local_40);
  local_40.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(unaff_EBX + 0x4b0a6a);
  CRecipientFilter::AddAllPlayers(&local_40);
  CRecipientFilter::MakeReliable(&local_40);
  __in_chrg = 4;
  SetCurrentPaint(this,NO_POWER);
  local_40.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(unaff_EBX + 0x4b0a6a);
  CRecipientFilter::~CRecipientFilter(&local_40,__in_chrg);
  return;
}


/* CWeaponPaintGun::DeactivatePaint at 006fcfa0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void DeactivatePaint(CWeaponPaintGun * this, PaintPowerType nIndex) */

void __thiscall CWeaponPaintGun::DeactivatePaint(CWeaponPaintGun *this,PaintPowerType nIndex)

{
  uint *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  
  if ((this->m_bHasPaint).m_Value[nIndex] != false) {
    if (*(bool *)((int)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                        super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                        m_TimerEvent + 0x10) == false) {
      this_00 = &((this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                  super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->
                 super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar2->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = (uint *)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                        super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                        field_0x4c;
      *puVar1 = *puVar1 | 1;
    }
    (this->m_bHasPaint).m_Value[nIndex] = false;
  }
  return;
}


/* CWeaponPaintGun::CleansePaint at 006fd950 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void CleansePaint(CWeaponPaintGun * this) */

void __thiscall CWeaponPaintGun::CleansePaint(CWeaponPaintGun *this)

{
  bool bVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  bVar1 = HasAnyPaintPower(this);
  if (!bVar1) {
    return;
  }
  CBaseEntity::EmitSound((CBaseEntity *)this,&UNK_0031675d + unaff_EBX,0.0,(float *)0x0);
  ResetPaint(this);
  return;
}


/* CWeaponPaintGun::SetCurrentPaint at 006fd640 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void SetCurrentPaint(CWeaponPaintGun * this, PaintPowerType nIndex) */

void __thiscall CWeaponPaintGun::SetCurrentPaint(CWeaponPaintGun *this,PaintPowerType nIndex)

{
  uint *puVar1;
  CBaseEdict *this_00;
  bool bVar2;
  char cVar3;
  IChangeInfoAccessor *pIVar4;
  CBasePlayer *pCVar5;
  int unaff_EBX;
  CBasePlayer *player;
  char *messagename;
  CRecipientFilter local_3c;
  
                    /* Unresolved local var: CBaseEntity * pOwner@[???]
                       Unresolved local var: CSingleUserRecipientFilter filter@[???] */
  ___i686_get_pc_thunk_bx();
  bVar2 = HasPaintPower(this,nIndex);
  if ((bVar2) && (nIndex != (this->m_nCurrentColor).m_Value)) {
    if (*(bool *)((int)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                        super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                        m_TimerEvent + 0x10) == false) {
      this_00 = &((this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                  super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->
                 super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar4 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar4->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = (uint *)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                        super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                        field_0x4c;
      *puVar1 = *puVar1 | 1;
    }
    (this->m_nCurrentColor).m_Value = nIndex;
  }
  pCVar5 = (CBasePlayer *)CBaseCombatWeapon::GetOwner((CBaseCombatWeapon *)this);
  if (pCVar5 != (CBasePlayer *)0x0) {
    cVar3 = (**(code **)((int)(pCVar5->super_CBaseCombatCharacter).super_CBaseFlex.
                              super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                              super_IServerEntity.super_IServerUnknown.super_IHandleEntity + 0x154))
                      (pCVar5);
    player = (CBasePlayer *)0x0;
    if (cVar3 != '\0') {
      player = pCVar5;
    }
    CRecipientFilter::CRecipientFilter(&local_3c);
    local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
         (_func_int_varargs **)(unaff_EBX + 0x4b0aba);
    CRecipientFilter::AddRecipient(&local_3c,player);
    CRecipientFilter::MakeReliable(&local_3c);
    messagename = (char *)(unaff_EBX + 0x2bf3aa);
    UserMessageBegin(&local_3c.super_IRecipientFilter,messagename);
    MessageWriteEHandle((CBaseEntity *)this);
    MessageWriteByte(nIndex);
    MessageEnd();
    local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
         (_func_int_varargs **)(unaff_EBX + 0x4b0aba);
    CRecipientFilter::~CRecipientFilter(&local_3c,(int)messagename);
  }
  return;
}


/* CWeaponPaintGun::PaintGunThink at 006fce80 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void PaintGunThink(CWeaponPaintGun * this) */

void __thiscall CWeaponPaintGun::PaintGunThink(CWeaponPaintGun *this)

{
  BASEPTR func;
  char cVar1;
  CBaseCombatCharacter *pCVar2;
  CPortal_Player *this_00;
  int unaff_EBX;
  Vector local_28 [2];
  
                    /* Unresolved local var: CPortal_Player * pPlayer@[???] */
  ___i686_get_pc_thunk_bx();
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x14);
  CBaseEntity::ThinkSet
            ((CBaseEntity *)this,func,*(float *)(**(int **)(unaff_EBX + 0x4aed0c) + 0xc),
             (char *)(unaff_EBX + 0x317246));
  pCVar2 = CBaseCombatWeapon::GetOwner((CBaseCombatWeapon *)this);
  if (pCVar2 != (CBaseCombatCharacter *)0x0) {
    cVar1 = (**(code **)((int)(pCVar2->super_CBaseFlex).super_CBaseAnimatingOverlay.
                              super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
                              super_IServerUnknown.super_IHandleEntity + 0x154))(pCVar2);
    if (cVar1 != '\0') {
      this_00 = (CPortal_Player *)
                ___dynamic_cast(pCVar2,*(undefined4 *)(unaff_EBX + 0x4aed34),
                                *(undefined4 *)(unaff_EBX + 0x4aef34),0);
      if (this_00 != (CPortal_Player *)0x0) {
        CPortal_Player::GetPaintGunShootPosition(local_28,this_00);
        (this->m_vecOldBlobFirePos).x = local_28[0].x;
        (this->m_vecOldBlobFirePos).y = local_28[0].y;
        (this->m_vecOldBlobFirePos).z = local_28[0].z;
      }
    }
  }
  return;
}


/* GiveAllPaintPowers at 006fd340 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void GiveAllPaintPowers(void)

{
  char cVar1;
  CWeaponPaintGun *this;
  CBasePlayer *pPlayer;
  int unaff_EBX;
  CBaseEntity *pFoundEnt;
  
  ___i686_get_pc_thunk_bx();
  pFoundEnt = CGlobalEntityList::FindEntityByClassname
                        (*(CGlobalEntityList **)(unaff_EBX + 0x4ae912),(CBaseEntity *)0x0,
                         (char *)(unaff_EBX + 0x2bc2f2));
  if (pFoundEnt != (CBaseEntity *)0x0) {
    do {
      this = (CWeaponPaintGun *)
             ___dynamic_cast(pFoundEnt,*(undefined4 *)(unaff_EBX + 0x4ae87a),unaff_EBX + 0x544b0a,0)
      ;
      if (this != (CWeaponPaintGun *)0x0) {
        pPlayer = (CBasePlayer *)CBaseCombatWeapon::GetOwner((CBaseCombatWeapon *)this);
        if ((pPlayer == (CBasePlayer *)0x0) ||
           (cVar1 = (**(code **)((int)(pPlayer->super_CBaseCombatCharacter).super_CBaseFlex.
                                      super_CBaseAnimatingOverlay.super_CBaseAnimating.
                                      super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
                                      super_IHandleEntity + 0x154))(pPlayer), cVar1 == '\0')) {
          pPlayer = (CBasePlayer *)0x0;
        }
        CWeaponPaintGun::ActivatePaint(this,BOUNCE_POWER);
        CWeaponPaintGun::ActivatePaint(this,SPEED_POWER);
        CWeaponPaintGun::ActivatePaint(this,STICK_POWER);
        CWeaponPaintGun::ActivatePaint(this,PORTAL_POWER);
        PaintPowerPickup(0,pPlayer);
        PaintPowerPickup(2,pPlayer);
        PaintPowerPickup(1,pPlayer);
        PaintPowerPickup(3,pPlayer);
      }
      pFoundEnt = CGlobalEntityList::FindEntityByClassname
                            (*(CGlobalEntityList **)(unaff_EBX + 0x4ae912),pFoundEnt,
                             (char *)(unaff_EBX + 0x2bc2f2));
    } while (pFoundEnt != (CBaseEntity *)0x0);
  }
  return;
}


/* __tcf_3 at 0099de50 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x41eba8),in_stack_00000008);
  return;
}


/* _GLOBAL__I__ZN15CWeaponPaintGun14GetServerClassEv at 000d1530 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN15CWeaponPaintGun14GetServerClassEv(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

