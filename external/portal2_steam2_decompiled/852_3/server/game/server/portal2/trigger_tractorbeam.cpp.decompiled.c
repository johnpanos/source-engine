/* DWARF-guided pseudocode for game/server/portal2/trigger_tractorbeam.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* DataMapInit<CProjectedTractorBeamEntity> at 000d0650 */

datamap_t * DataMapInit<CProjectedTractorBeamEntity>(CProjectedTractorBeamEntity *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&rgdpvpreset[0x15].pitchstart + unaff_EBX + 3) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcec2df);
    if (iVar1 != 0) {
      *(int *)((int)&rgdpvpreset[0x15].spindown + unaff_EBX + 3) = unaff_EBX + 0x943913;
      *(undefined4 *)((int)&rgdpvpreset[0x15].volstart + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&rgdpvpreset[0x15].fadein + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&rgdpvpreset[0x15].fadeout + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&rgdpvpreset[0x15].lfotype + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&rgdpvpreset[0x15].lforate + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&rgdpvpreset[0x15].volrun + unaff_EBX + 3) = 0x1b;
      ___cxa_guard_release(unaff_EBX + 0xcec2df);
      ___cxa_atexit(unaff_EBX + 0x8cd68f,0,*(undefined4 *)(&DAT_00adb52f + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_ReachedEndOfSequence_00c00ee8 + unaff_EBX + 3) =
       *(undefined4 *)(&DAT_00adc0ab + unaff_EBX);
  *(undefined4 *)((int)&PTR_GetIdealAccel_00c00ee0 + unaff_EBX + 3) = 1;
  *(int *)((int)&PTR_GetIdealSpeed_00c00edc + unaff_EBX + 3) = unaff_EBX + 0xc0179f;
  return (datamap_t *)((int)&PTR_GetIdealSpeed_00c00edc + unaff_EBX + 3);
}


/* __static_initialization_and_destruction_0 at 000d0960 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *pcVar1;
  SendTable *pSVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  datamap_t *pdVar5;
  int iVar6;
  int iVar7;
  IEntityFactoryDictionary *pIVar8;
  int unaff_EBX;
  longlong lVar9;
  
  lVar9 = ___i686_get_pc_thunk_bx();
  if (lVar9 != 0xffff00000001) {
    return;
  }
  (&CAI_RappelBehavior::gm_SchedLoadStatus.field_0x2)[unaff_EBX] = 0;
  (&CAI_RappelBehavior::gm_SchedLoadStatus.field_0x3)[unaff_EBX] = 0;
  *(undefined1 *)((int)&CAI_RappelBehavior::gm_SchedLoadStatus.signature + unaff_EBX) = 0;
  *(undefined1 *)((int)&CAI_RappelBehavior::gm_SchedLoadStatus.signature + unaff_EBX + 1) = 0;
  *(undefined4 *)((int)&CAI_RappelBehavior::gm_SchedLoadStatus.signature + unaff_EBX + 2) = 0;
  *(undefined4 *)
   ((int)&CUtlRBTree<CUtlMap<unsigned_int,Activity,unsigned_short>::Node_t,unsigned_short,CUtlMap<unsigned_int,Activity,unsigned_short>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<unsigned_int,Activity,unsigned_short>::Node_t,unsigned_short>,unsigned_short>>
          ::Links(unsigned_short)::s_Sentinel + unaff_EBX + 2) = 0;
  *(undefined4 *)(&DAT_00ceb65e + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&CUtlRBTree<CUtlMap<unsigned_int,Activity,unsigned_short>::Node_t,unsigned_short,CUtlMap<unsigned_int,Activity,unsigned_short>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<unsigned_int,Activity,unsigned_short>::Node_t,unsigned_short>,unsigned_short>>
          ::Links(unsigned_short)::s_Sentinel + unaff_EBX + 2) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00ceb666 + unaff_EBX) = 0;
  *(undefined4 *)(&CAI_StandoffBehavior::gm_SchedLoadStatus.field_0x2 + unaff_EBX) = 0;
  *(undefined4 *)((int)&CAI_StandoffBehavior::gm_SchedLoadStatus.signature + unaff_EBX + 2) = 0;
  *(undefined4 *)(&DAT_00ceb672 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00ceb676 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00ceb67a + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00ceb67e + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&g_StandoffParamsByAgression[0].hintChangeReaction + unaff_EBX + 2) =
       0x7f7fffff;
  *(undefined4 *)(&g_StandoffParamsByAgression[0].field_0x6 + unaff_EBX) = 0;
  *(undefined1 *)((int)&g_StandoffParamsByAgression[0].maxTimeShots + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&g_StandoffParamsByAgression[0].minTimeShots + unaff_EBX + 2) = 0;
  *(undefined1 *)((int)&g_StandoffParamsByAgression[0].minTimeShots + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&g_StandoffParamsByAgression[0].maxTimeShots + unaff_EBX) = 0;
  *(undefined4 *)((int)&g_StandoffParamsByAgression[0].maxTimeShots + unaff_EBX + 2) = 0;
  *(undefined1 *)((int)&g_StandoffParamsByAgression[0].maxShots + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&g_StandoffParamsByAgression[0].minShots + unaff_EBX + 2) = 0;
  *(undefined1 *)((int)&g_StandoffParamsByAgression[0].minShots + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&g_StandoffParamsByAgression[0].maxShots + unaff_EBX) = 0;
  *(undefined4 *)((int)&g_StandoffParamsByAgression[0].maxShots + unaff_EBX + 2) = 0;
  (&g_StandoffParamsByAgression[0].field_0x1d)[unaff_EBX] = 1;
  *(undefined1 *)((int)&g_StandoffParamsByAgression[0].oddsCover + unaff_EBX + 2) = 0;
  *(undefined1 *)((int)&g_StandoffParamsByAgression[0].oddsCover + unaff_EBX + 3) = 0;
  (&g_StandoffParamsByAgression[0].fStayAtCover)[unaff_EBX] = false;
  *(undefined4 *)(&g_StandoffParamsByAgression[0].field_0x1e + unaff_EBX) = 1;
  *(undefined1 *)((int)&g_StandoffParamsByAgression[1].hintChangeReaction + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&g_StandoffParamsByAgression[0].flAbandonTimeLimit + unaff_EBX + 2) = 0;
  *(undefined1 *)((int)&g_StandoffParamsByAgression[0].flAbandonTimeLimit + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&g_StandoffParamsByAgression[1].hintChangeReaction + unaff_EBX) = 0;
  *(undefined4 *)((int)&g_StandoffParamsByAgression[1].hintChangeReaction + unaff_EBX + 2) = 2;
  *(undefined1 *)((int)&g_StandoffParamsByAgression[1].minTimeShots + unaff_EBX + 1) = 0;
  (&g_StandoffParamsByAgression[1].field_0x6)[unaff_EBX] = 0;
  (&g_StandoffParamsByAgression[1].field_0x7)[unaff_EBX] = 0;
  *(undefined1 *)((int)&g_StandoffParamsByAgression[1].minTimeShots + unaff_EBX) = 0;
  *(undefined **)((int)&g_StandoffParamsByAgression[1].minTimeShots + unaff_EBX + 2) =
       &UNK_00ae09ba + unaff_EBX;
  pdVar5 = DataMapInit<CProjectedTractorBeamEntity>((CProjectedTractorBeamEntity *)0x0);
  *(datamap_t **)((int)&PTR_IsKeyRebound_00c5fdd8 + unaff_EBX + 2) = pdVar5;
  iVar7 = unaff_EBX + 0xceb6b2;
  pcVar1 = (char *)(unaff_EBX + 0x943606);
  *(char **)((int)&g_StandoffParamsByAgression[1].maxTimeShots + unaff_EBX + 2) = pcVar1;
  pSVar2 = (SendTable *)((int)&PTR_IsKeyOverridden_00c5fddc + unaff_EBX + 2);
  *(SendTable **)((int)&g_StandoffParamsByAgression[1].minShots + unaff_EBX + 2) = pSVar2;
  *(undefined4 *)(&g_StandoffParamsByAgression[1].field_0x1e + unaff_EBX) = 0xffff;
  puVar3 = (undefined4 *)**(int **)(&DAT_00adb2c2 + unaff_EBX);
  if (puVar3 == (undefined4 *)0x0) {
    **(int **)(&DAT_00adb2c2 + unaff_EBX) = iVar7;
    *(undefined4 *)((int)&g_StandoffParamsByAgression[1].maxShots + unaff_EBX + 2) = 0;
  }
  else {
    puVar4 = (undefined4 *)puVar3[2];
    iVar6 = _V_stricmp((char *)*puVar3,pcVar1);
    if (iVar6 < 1) {
      while ((puVar4 != (undefined4 *)0x0 && (iVar6 = _V_stricmp((char *)*puVar4,pcVar1), iVar6 < 1)
             )) {
        puVar3 = puVar4;
        puVar4 = (undefined4 *)puVar4[2];
      }
      *(undefined4 **)((int)&g_StandoffParamsByAgression[1].maxShots + unaff_EBX + 2) = puVar4;
      puVar3[2] = iVar7;
    }
    else {
      *(undefined4 *)((int)&g_StandoffParamsByAgression[1].maxShots + unaff_EBX + 2) =
           **(undefined4 **)(&DAT_00adb2c2 + unaff_EBX);
      **(int **)(&DAT_00adb2c2 + unaff_EBX) = iVar7;
    }
  }
  SendTable::SendTable(pSVar2);
  ___cxa_atexit(unaff_EBX + 0x8cd282,0,*(undefined4 *)(&DAT_00adb222 + unaff_EBX));
  iVar7 = ServerClassInit<DT_ProjectedTractorBeamEntity::ignored>((ignored *)0x0);
  *(int *)((int)&PTR_OnKeyCodePressed_00c5fdf0 + unaff_EBX + 2) = iVar7;
  *(undefined **)((int)&g_StandoffParamsByAgression[1].flAbandonTimeLimit + unaff_EBX + 2) =
       &UNK_00b70eaa + unaff_EBX;
  pIVar8 = EntityFactoryDictionary();
  (**pIVar8->_vptr_IEntityFactoryDictionary)(pIVar8,unaff_EBX + 0xceb6c6,unaff_EBX + 0x8e7374);
  pdVar5 = DataMapInit<CTrigger_TractorBeam>((CTrigger_TractorBeam *)0x0);
  *(datamap_t **)((int)&PTR_OnKeyCodeTyped_00c5fdf4 + unaff_EBX + 2) = pdVar5;
  *(undefined **)((int)&g_StandoffParamsByAgression[2].hintChangeReaction + unaff_EBX + 2) =
       &UNK_00b70eca + unaff_EBX;
  pIVar8 = EntityFactoryDictionary();
  (**pIVar8->_vptr_IEntityFactoryDictionary)(pIVar8,unaff_EBX + 0xceb6ca,unaff_EBX + 0x8e7392);
  iVar7 = unaff_EBX + 0xceb6ce;
  pcVar1 = (char *)(unaff_EBX + 0x943622);
  *(char **)(&g_StandoffParamsByAgression[2].field_0x6 + unaff_EBX) = pcVar1;
  pSVar2 = (SendTable *)((int)&PTR_OnKeyTyped_00c5fdf8 + unaff_EBX + 2);
  *(SendTable **)((int)&g_StandoffParamsByAgression[2].minTimeShots + unaff_EBX + 2) = pSVar2;
  *(undefined4 *)((int)&g_StandoffParamsByAgression[2].maxShots + unaff_EBX + 2) = 0xffff;
  puVar3 = (undefined4 *)**(int **)(&DAT_00adb2c2 + unaff_EBX);
  if (puVar3 == (undefined4 *)0x0) {
    **(int **)(&DAT_00adb2c2 + unaff_EBX) = iVar7;
    *(undefined4 *)((int)&g_StandoffParamsByAgression[2].maxTimeShots + unaff_EBX + 2) = 0;
  }
  else {
    puVar4 = (undefined4 *)puVar3[2];
    iVar6 = _V_stricmp((char *)*puVar3,pcVar1);
    if (iVar6 < 1) {
      while ((puVar4 != (undefined4 *)0x0 && (iVar6 = _V_stricmp((char *)*puVar4,pcVar1), iVar6 < 1)
             )) {
        puVar3 = puVar4;
        puVar4 = (undefined4 *)puVar4[2];
      }
      *(undefined4 **)((int)&g_StandoffParamsByAgression[2].maxTimeShots + unaff_EBX + 2) = puVar4;
      puVar3[2] = iVar7;
    }
    else {
      *(undefined4 *)((int)&g_StandoffParamsByAgression[2].maxTimeShots + unaff_EBX + 2) =
           **(undefined4 **)(&DAT_00adb2c2 + unaff_EBX);
      **(int **)(&DAT_00adb2c2 + unaff_EBX) = iVar7;
    }
  }
  SendTable::SendTable(pSVar2);
  ___cxa_atexit(unaff_EBX + 0x8cd262,0,*(undefined4 *)(&DAT_00adb222 + unaff_EBX));
  iVar7 = ServerClassInit<DT_Trigger_TractorBeam::ignored>((ignored *)0x0);
  *(int *)((int)&PTR_Paint_00c5fe0c + unaff_EBX + 2) = iVar7;
  return;
}


/* CProjectedTractorBeamEntity::GetDataDescMap at 006fb000 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CProjectedTractorBeamEntity * this) */

datamap_t * __thiscall
CProjectedTractorBeamEntity::GetDataDescMap(CProjectedTractorBeamEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5d6538);
}


/* CProjectedTractorBeamEntity::GetBaseMap at 006fb010 */

datamap_t * CProjectedTractorBeamEntity::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(&DAT_004b16f4 + extraout_ECX);
}


/* __tcf_0 at 0099dcf0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41ec5e)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41ec52) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41ec5e));
  }
  *(undefined4 *)(unaff_EBX + 0x41ec5e) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41ec5a)) {
    if (*(int *)(unaff_EBX + 0x41ec52) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20de9a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20de9a),*(int *)(unaff_EBX + 0x41ec52));
      *(undefined4 *)(unaff_EBX + 0x41ec52) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41ec56) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41ec52);
  *(int *)(unaff_EBX + 0x41ec62) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41ec5a)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20de9a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20de9a),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41ec52) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41ec56) = 0;
  }
  return;
}


/* ServerClassInit<DT_ProjectedTractorBeamEntity::ignored> at 000d0480 */

int ServerClassInit<DT_ProjectedTractorBeamEntity::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&rgdpvpreset[0x16].fadeoutsav + unaff_EBX) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcec38c);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)((int)&rgdpvpreset[0x17].spindown + unaff_EBX),
                  (char *)(unaff_EBX + 0x8e2590),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)((int)&rgdpvpreset[0x18].preset + unaff_EBX),
                        (char *)(unaff_EBX + 0x8e4900),0,
                        (SendTable *)**(undefined4 **)(&DAT_00adc280 + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00adb798 + unaff_EBX),0x80);
      SendPropEHandle((SendProp_conflict *)((int)&rgdpvpreset[0x18].fadeoutsav + unaff_EBX),
                      (char *)(unaff_EBX + 0x943aca),0x41c,4,-1,
                      *(SendVarProxyFn_conflict *)(&DAT_00adb784 + unaff_EBX));
      ___cxa_guard_release(unaff_EBX + 0xcec38c);
      ___cxa_atexit(unaff_EBX + 0x8cd6ac,0,*(undefined4 *)(&DAT_00adb6fc + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)((int)&PTR_OnMouseReleased_00c602b8 + unaff_EBX),
             (SendProp_conflict *)((int)&rgdpvpreset[0x18].preset + unaff_EBX),2,
             *(char **)(&DAT_00b71370 + unaff_EBX));
  return 1;
}


/* CProjectedTractorBeamEntity::GetServerClass at 006fb020 */

/* DWARF original prototype: ServerClass * GetServerClass(CProjectedTractorBeamEntity * this) */

ServerClass * __thiscall
CProjectedTractorBeamEntity::GetServerClass(CProjectedTractorBeamEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x6c0ff8);
}


/* CProjectedTractorBeamEntity::YouForgotToImplementOrDeclareServerClass at 006fb030 */

/* DWARF original prototype: int
   YouForgotToImplementOrDeclareServerClass(CProjectedTractorBeamEntity * this) */

int __thiscall
CProjectedTractorBeamEntity::YouForgotToImplementOrDeclareServerClass
          (CProjectedTractorBeamEntity *this)

{
  return 0;
}


/* __tcf_4 at 0099dbf0 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x392b54),in_stack_00000008);
  return;
}


/* __tcf_1 at 0099db40 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41ed97))(unaff_EBX + 0x41ed97);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41ed43))(unaff_EBX + 0x41ed43);
                    /* WARNING: Could not recover jumptable at 0x0099db87. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41ecef))();
  return;
}


/* DataMapInit<CTrigger_TractorBeam> at 000d0720 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

datamap_t * DataMapInit<CTrigger_TractorBeam>(CTrigger_TractorBeam *param_1)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  ISaveRestoreOps *pIVar4;
  char *pcVar5;
  size_t sVar6;
  undefined4 *puVar7;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((*(char *)((int)&rgdpvpreset[0xf].spinupsav + unaff_EBX) == '\0') &&
     (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xcec0bc), iVar3 != 0)) {
    *(int *)((int)&rgdpvpreset[0xf].fadeinsav + unaff_EBX) = unaff_EBX + 0x94385c;
    *(undefined4 *)((int)&rgdpvpreset[0xf].volfrac + unaff_EBX) = 0;
    *(undefined4 *)((int)&rgdpvpreset[0xf].lfofrac + unaff_EBX) = 0;
    *(undefined4 *)((int)&rgdpvpreset[0xf].lfomult + unaff_EBX) = 0;
    *(undefined4 *)((int)&rgdpvpreset[0x10].preset + unaff_EBX) = 0;
    *(undefined4 *)((int)&rgdpvpreset[0x10].pitchrun + unaff_EBX) = 0;
    *(undefined4 *)((int)&rgdpvpreset[0xf].fadeoutsav + unaff_EBX) = 0x14;
    ___cxa_guard_release(unaff_EBX + 0xcec0bc);
    ___cxa_atexit(unaff_EBX + 0x8cd4dc,0,*(undefined4 *)(&DAT_00adb45c + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_PhysicsSplash_00c00e34 + unaff_EBX) =
       *(undefined4 *)(&DAT_00adc000 + unaff_EBX);
  if ((*(char *)((int)&rgdpvpreset[0xf].pitchfrac + unaff_EBX) == '\0') &&
     (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xcec0c4), iVar3 != 0)) {
    pIVar4 = GetPhysObjSaveRestoreOps(PIID_IPHYSICSMOTIONCONTROLLER);
    *(ISaveRestoreOps **)((int)&PTR_NetworkStateChanged_m_nWaterLevel_00c00ea0 + unaff_EBX) = pIVar4
    ;
    pIVar4 = GetSoundSaveRestoreOps();
    *(ISaveRestoreOps **)((int)&PTR_WeaponLOSCondition_00c01160 + unaff_EBX) = pIVar4;
    pcVar5 = operator_new___(*(int *)((int)&rgdpvpreset[0xf].fadeoutsav + unaff_EBX) + 0xd);
    _strcpy(pcVar5,*(char **)((int)&rgdpvpreset[0xf].fadeinsav + unaff_EBX));
    sVar6 = _strlen(pcVar5);
    builtin_strncpy(pcVar5 + sVar6,"TractorThink",0xd);
    uVar1 = *(uint *)((int)&rgdpvpreset[0x10].preset + unaff_EBX);
    CUtlVector<char*,CUtlMemory<char*,_int>_>::GrowVector
              ((CUtlVector<char*,CUtlMemory<char*,_int>_> *)
               ((int)&rgdpvpreset[0xf].volfrac + unaff_EBX),1);
    iVar3 = ~uVar1 + *(int *)((int)&rgdpvpreset[0x10].preset + unaff_EBX);
    if (0 < iVar3) {
      iVar2 = *(int *)((int)&rgdpvpreset[0xf].volfrac + unaff_EBX);
      _V_memmove((void *)(iVar2 + 4 + uVar1 * 4),(void *)(iVar2 + uVar1 * 4),iVar3 * 4);
    }
    puVar7 = (undefined4 *)(uVar1 * 4 + *(int *)((int)&rgdpvpreset[0xf].volfrac + unaff_EBX));
    if (puVar7 != (undefined4 *)0x0) {
      *puVar7 = pcVar5;
    }
    *(char **)((int)&PTR_OnFriendDamaged_00c01650 + unaff_EBX) = pcVar5;
    ___cxa_guard_release(unaff_EBX + 0xcec0c4);
  }
  *(undefined4 *)((int)&PTR_OnGroundChanged_00c00e2c + unaff_EBX) = 0x20;
  *(int *)((int)&PTR_GetReceivedDamageScale_00c00e28 + unaff_EBX) =
       (int)&PTR_GetStepAngles_00c00e8c + unaff_EBX;
  return (datamap_t *)((int)&PTR_GetReceivedDamageScale_00c00e28 + unaff_EBX);
}


/* CTrigger_TractorBeam::GetDataDescMap at 006fb040 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: datamap_t * GetDataDescMap(CTrigger_TractorBeam * this) */

datamap_t * __thiscall CTrigger_TractorBeam::GetDataDescMap(CTrigger_TractorBeam *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5d6514);
}


/* CTrigger_TractorBeam::GetBaseMap at 006fb050 */

datamap_t * CTrigger_TractorBeam::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(&DAT_004b16dc + extraout_ECX);
}


/* __tcf_2 at 0099dc10 */

void __tcf_2(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41ebf6)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41ebea) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41ebf6));
  }
  *(undefined4 *)(unaff_EBX + 0x41ebf6) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41ebf2)) {
    if (*(int *)(unaff_EBX + 0x41ebea) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20df7a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20df7a),*(int *)(unaff_EBX + 0x41ebea));
      *(undefined4 *)(unaff_EBX + 0x41ebea) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41ebee) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41ebea);
  *(int *)(unaff_EBX + 0x41ebfa) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41ebf2)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20df7a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20df7a),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41ebea) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41ebee) = 0;
  }
  return;
}


/* ServerClassInit<DT_Trigger_TractorBeam::ignored> at 000cfbf0 */

int ServerClassInit<DT_Trigger_TractorBeam::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&rgdpvpreset[0x18].fadeinsav + unaff_EBX + 2) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcec452);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)((int)&rgdpvpreset[0x18].lfomult + unaff_EBX + 2),
                  (char *)(unaff_EBX + 0x8e2e26),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)((int)&rgdpvpreset[0x19].fadeinsav + unaff_EBX + 2),
                        (char *)(unaff_EBX + 0x8e5196),0,
                        (SendTable *)**(undefined4 **)(&DAT_00adcb32 + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00adc02e + unaff_EBX),0x80);
      SendPropBool((SendProp_conflict *)((int)&rgdpvpreset[0x1a].spinupsav + unaff_EBX + 2),
                   &UNK_0091cdf6 + unaff_EBX,0x3c0,1);
      SendPropFloat((SendProp_conflict *)(unaff_EBX + 0xcec55e),(char *)(unaff_EBX + 0x94dac0),0x3d4
                    ,4,0,4,0.0,-121121.125,*(SendVarProxyFn_conflict *)(&DAT_00adc086 + unaff_EBX),
                    0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&CUtlVectorDataopsInstantiator<10>::
                           GetDataOps<CUtlVector<ResponseContext_t,CUtlMemory<ResponseContext_t,int>>>(CUtlVector<ResponseContext_t,CUtlMemory<ResponseContext_t,int>>*)
                           ::ops + unaff_EBX + 2),(char *)(unaff_EBX + 0x94429e),0x3d8,4,0,4,0.0,
                    -121121.125,*(SendVarProxyFn_conflict *)(&DAT_00adc086 + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)(unaff_EBX + 0xcec606),&UNK_009442ae + unaff_EBX,0x3dc,4,0,
                    4,0.0,-121121.125,*(SendVarProxyFn_conflict *)(&DAT_00adc086 + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)(unaff_EBX + 0xcec65a),(char *)(unaff_EBX + 0x94dad2),0x3e0
                    ,4,0,4,0.0,-121121.125,*(SendVarProxyFn_conflict *)(&DAT_00adc086 + unaff_EBX),
                    0x80);
      SendPropFloat((SendProp_conflict *)
                    (CBaseEntity::GetPreTemplateName::szStrippedName + unaff_EBX + 0x4e),
                    (char *)(unaff_EBX + 0x9442bc),0x3e4,4,0,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00adc086 + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)(unaff_EBX + 0xcec702),&UNK_009442ce + unaff_EBX,1000,4,0,4
                    ,0.0,-121121.125,*(SendVarProxyFn_conflict *)(&DAT_00adc086 + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)((int)predef_flexcontroller_values[0] + unaff_EBX + 0x16),
                    (char *)(unaff_EBX + 0x94dae8),0x3ec,4,0,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00adc086 + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)((int)predef_flexcontroller_values[0] + unaff_EBX + 0x6a),
                    (char *)(unaff_EBX + 0x9442e1),0x3f0,4,0,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00adc086 + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)((int)predef_flexcontroller_values[1] + unaff_EBX + 0x46),
                    (char *)(unaff_EBX + 0x943f3e),0x3f4,4,0,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00adc086 + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)((int)predef_flexcontroller_values[2] + unaff_EBX + 0x22),
                    (char *)(unaff_EBX + 0x9442ef),0x3f8,4,0,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00adc086 + unaff_EBX),0x80);
      SendPropQAngles((SendProp_conflict *)((int)predef_flexcontroller_values[2] + unaff_EBX + 0x76)
                      ,(char *)(unaff_EBX + 0x9442fe),0x3fc,0xc,0,4,
                      *(SendVarProxyFn_conflict *)(&DAT_00adc0f6 + unaff_EBX),0x80);
      SendPropEHandle((SendProp_conflict *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x52)
                      ,(char *)(anon_unknown.dwarf_127e1090::ScrollBarButton::~ScrollBarButton +
                               unaff_EBX + 2),0x408,4,-1,
                      *(SendVarProxyFn_conflict *)(&DAT_00adc01a + unaff_EBX));
      SendPropVector((SendProp_conflict *)((int)predef_flexcontroller_values[4] + unaff_EBX + 0x2e),
                     (char *)(unaff_EBX + 0x944321),0x40c,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00adc07a + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)((int)predef_flexcontroller_values[5] + unaff_EBX + 10),
                     (char *)(unaff_EBX + 0x94432a),0x418,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00adc07a + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)((int)predef_flexcontroller_values[5] + unaff_EBX + 0x5e),
                    (char *)(unaff_EBX + 0x94db14),0x424,4,0,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00adc086 + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)((int)predef_flexcontroller_values[6] + unaff_EBX + 0x3a),
                    &UNK_0090b016 + unaff_EBX,0x428,4,0,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00adc086 + unaff_EBX),0x80);
      SendPropBool((SendProp_conflict *)(unaff_EBX + 0xceca9e),&UNK_00914476 + unaff_EBX,0x42c,1);
      SendPropBool((SendProp_conflict *)(unaff_EBX + 0xcecaf2),
                   (char *)(anon_unknown.dwarf_127e1090::ScrollBarButton::OnMouseFocusTicked +
                           unaff_EBX + 1),0x42d,1);
      SendPropBool((SendProp_conflict *)(unaff_EBX + 0xcecb46),(char *)(unaff_EBX + 0x94433f),0x42e,
                   1);
      SendPropBool((SendProp_conflict *)(unaff_EBX + 0xcecb9a),(char *)(unaff_EBX + 0x94434b),0x42f,
                   1);
      ___cxa_guard_release(unaff_EBX + 0xcec452);
      ___cxa_atexit(unaff_EBX + 0x8cdf92,0,*(undefined4 *)(&DAT_00adbf92 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)((int)&PTR_QueryInterface_00c60b68 + unaff_EBX + 2),
             (SendProp_conflict *)((int)&rgdpvpreset[0x19].fadeinsav + unaff_EBX + 2),0x16,
             *(char **)(&DAT_00b71c02 + unaff_EBX));
  return 1;
}


/* CTrigger_TractorBeam::GetServerClass at 006fb060 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: ServerClass * GetServerClass(CTrigger_TractorBeam * this) */

ServerClass * __thiscall CTrigger_TractorBeam::GetServerClass(CTrigger_TractorBeam *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x6c0fd4);
}


/* CTrigger_TractorBeam::YouForgotToImplementOrDeclareServerClass at 006fb070 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CTrigger_TractorBeam *
   this) */

int __thiscall
CTrigger_TractorBeam::YouForgotToImplementOrDeclareServerClass(CTrigger_TractorBeam *this)

{
  return 0;
}


/* __tcf_5 at 0099dbd0 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x392b90),in_stack_00000008);
  return;
}


/* __tcf_3 at 0099db90 */

void __tcf_3(void *param_1)

{
  int unaff_EBX;
  undefined4 *puVar1;
  
  ___i686_get_pc_thunk_bx();
  puVar1 = (undefined4 *)(unaff_EBX + 0x41ebfa);
  do {
    (**(code **)*puVar1)(puVar1);
    puVar1 = puVar1 + -0x15;
  } while (puVar1 != (undefined4 *)(unaff_EBX + 0x41e46e));
  return;
}


/* CTrigger_TractorBeam::CTrigger_TractorBeam at 006fc250 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void CTrigger_TractorBeam(CTrigger_TractorBeam * this) */

void __thiscall CTrigger_TractorBeam::CTrigger_TractorBeam(CTrigger_TractorBeam *this)

{
  undefined1 *puVar1;
  uint uVar2;
  CBaseEdict *this_00;
  int iVar3;
  int iVar4;
  undefined4 *puVar5;
  IChangeInfoAccessor *pIVar6;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::CBaseEntity((CBaseEntity *)this,false);
  (this->super_CBaseVPhysicsTrigger).m_hFilter.super_CBaseHandle.m_Index = 0xffffffff;
  (this->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x544ec4);
  (this->super_IMotionEvent)._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x5451f0);
  (this->m_hProxyEntity).
  super_CNetworkVarBase<CBaseHandle,CTrigger_TractorBeam::NetworkVar_m_hProxyEntity>.m_Value.m_Index
       = 0xffffffff;
  uVar2 = (*(CUtlVector<CTrigger_TractorBeam*,CUtlMemory<CTrigger_TractorBeam*,_int>_> **)
            (unaff_EBX + 0x4b04d8))->m_Size;
  CUtlVector<CTrigger_TractorBeam*,CUtlMemory<CTrigger_TractorBeam*,_int>_>::GrowVector
            (*(CUtlVector<CTrigger_TractorBeam*,CUtlMemory<CTrigger_TractorBeam*,_int>_> **)
              (unaff_EBX + 0x4b04d8),1);
  iVar4 = ~uVar2 + (*(int **)(unaff_EBX + 0x4b04d8))[3];
  if (0 < iVar4) {
    iVar3 = **(int **)(unaff_EBX + 0x4b04d8);
    _V_memmove((void *)(iVar3 + 4 + uVar2 * 4),(void *)(iVar3 + uVar2 * 4),iVar4 * 4);
  }
  puVar5 = (undefined4 *)(uVar2 * 4 + **(int **)(unaff_EBX + 0x4b04d8));
  if (puVar5 != (undefined4 *)0x0) {
    *puVar5 = this;
  }
  if (*(float *)(unaff_EBX + 0x334a5c) != (this->m_flRadius).m_Value) {
    if ((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
        false) {
      this_00 = &((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_pPev)->
                 super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar6->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flRadius).m_Value = 55.0;
  }
  return;
}


/* CTrigger_TractorBeam::CTrigger_TractorBeam at 006fc3b0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void CTrigger_TractorBeam(CTrigger_TractorBeam * this,
   CTrigger_TractorBeam * this) */

void __thiscall
CTrigger_TractorBeam::CTrigger_TractorBeam(CTrigger_TractorBeam *this,CTrigger_TractorBeam *this_1)

{
  CTrigger_TractorBeam(this);
  return;
}


/* CEntityFactory<CTrigger_TractorBeam>::Create at 006fcc00 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CTrigger_TractorBeam> *
   this, char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CTrigger_TractorBeam>::Create
          (CEntityFactory<CTrigger_TractorBeam> *this,char *pClassName)

{
  CTrigger_TractorBeam *this_00;
  
                    /* Unresolved local var: CTrigger_TractorBeam * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x438);
  CTrigger_TractorBeam::CTrigger_TractorBeam(this_00);
  (*(this_00->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.super_IServerUnknown
    .super_IHandleEntity._vptr_IHandleEntity[0x1d])(this_00,pClassName);
  return &(this_00->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.super_IServerNetworkable
  ;
}


/* CTrigger_TractorBeam::~CTrigger_TractorBeam at 006fc3c0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ~CTrigger_TractorBeam(CTrigger_TractorBeam * this, int __in_chrg)
    */

void __thiscall
CTrigger_TractorBeam::~CTrigger_TractorBeam(CTrigger_TractorBeam *this,int __in_chrg)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int unaff_EBX;
  int local_30;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x544d5a);
  (this->super_IMotionEvent)._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x545086);
  local_30 = *(int *)(*(int *)(unaff_EBX + 0x4b036e) + 0xc);
  if (0 < local_30) {
    piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x4b036e);
    if (this == (CTrigger_TractorBeam *)*piVar1) {
      iVar4 = 0;
      iVar3 = 0;
    }
    else {
      iVar4 = 0;
      iVar2 = 4;
      do {
        iVar3 = iVar2;
        iVar4 = iVar4 + 1;
        if (iVar4 == local_30) goto LAB_006fc433;
        iVar2 = iVar3 + 4;
      } while (this != (CTrigger_TractorBeam *)piVar1[iVar4]);
    }
    iVar2 = (local_30 - iVar4) + -1;
    if (0 < iVar2) {
      _V_memmove((void *)((int)piVar1 + iVar3),piVar1 + iVar4 + 1,iVar2 * 4);
      local_30 = *(int *)(*(int *)(unaff_EBX + 0x4b036e) + 0xc);
    }
    *(int *)(*(int *)(unaff_EBX + 0x4b036e) + 0xc) = local_30 + -1;
  }
LAB_006fc433:
  (this->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4b036a) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,__in_chrg);
  return;
}


/* CTrigger_TractorBeam::~CTrigger_TractorBeam at 006fc4c0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ~CTrigger_TractorBeam(CTrigger_TractorBeam * this, int __in_chrg)
    */

void __thiscall
CTrigger_TractorBeam::~CTrigger_TractorBeam(CTrigger_TractorBeam *this,int __in_chrg)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int unaff_EBX;
  int *in_stack_ffffffb8;
  int local_30;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x544c5a);
  (this->super_IMotionEvent)._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x544f86);
  local_30 = *(int *)(*(int *)(unaff_EBX + 0x4b026e) + 0xc);
  if (0 < local_30) {
    piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x4b026e);
    if (this == (CTrigger_TractorBeam *)*piVar1) {
      iVar4 = 0;
      iVar3 = 0;
    }
    else {
      iVar4 = 0;
      iVar2 = 4;
      do {
        iVar3 = iVar2;
        iVar4 = iVar4 + 1;
        if (iVar4 == local_30) goto LAB_006fc533;
        iVar2 = iVar3 + 4;
      } while (this != (CTrigger_TractorBeam *)piVar1[iVar4]);
    }
    iVar2 = (local_30 - iVar4) + -1;
    if (0 < iVar2) {
      in_stack_ffffffb8 = piVar1 + iVar4 + 1;
      _V_memmove((void *)(iVar3 + (int)piVar1),in_stack_ffffffb8,iVar2 * 4);
      local_30 = *(int *)(*(int *)(unaff_EBX + 0x4b026e) + 0xc);
    }
    *(int *)(*(int *)(unaff_EBX + 0x4b026e) + 0xc) = local_30 + -1;
  }
LAB_006fc533:
  (this->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4b026a) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,(int)in_stack_ffffffb8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CTrigger_TractorBeam::~CTrigger_TractorBeam at 006fc5c0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ~CTrigger_TractorBeam(CTrigger_TractorBeam * this, int __in_chrg)
    */

void __thiscall
CTrigger_TractorBeam::~CTrigger_TractorBeam(CTrigger_TractorBeam *this,int __in_chrg)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int unaff_EBX;
  int local_30;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x544b5a);
  (this->super_IMotionEvent)._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x544e86);
  local_30 = *(int *)(*(int *)(unaff_EBX + 0x4b016e) + 0xc);
  if (0 < local_30) {
    piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x4b016e);
    if (this == (CTrigger_TractorBeam *)*piVar1) {
      iVar4 = 0;
      iVar3 = 0;
    }
    else {
      iVar4 = 0;
      iVar2 = 4;
      do {
        iVar3 = iVar2;
        iVar4 = iVar4 + 1;
        if (iVar4 == local_30) goto LAB_006fc633;
        iVar2 = iVar3 + 4;
      } while (this != (CTrigger_TractorBeam *)piVar1[iVar4]);
    }
    iVar2 = (local_30 - iVar4) + -1;
    if (0 < iVar2) {
      _V_memmove((void *)((int)piVar1 + iVar3),piVar1 + iVar4 + 1,iVar2 * 4);
      local_30 = *(int *)(*(int *)(unaff_EBX + 0x4b016e) + 0xc);
    }
    *(int *)(*(int *)(unaff_EBX + 0x4b016e) + 0xc) = local_30 + -1;
  }
LAB_006fc633:
  (this->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4b016a) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,__in_chrg);
  return;
}


/* CTrigger_TractorBeam::Spawn at 006fb810 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "MoveCollide_t": Some values do not have unique names */
/* WARNING: Enum "MoveType_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(CTrigger_TractorBeam * this) */

void __thiscall CTrigger_TractorBeam::Spawn(CTrigger_TractorBeam *this)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  BASEPTR func;
  IChangeInfoAccessor *pIVar2;
  int unaff_EBX;
  NetworkVar_m_Collision *this_01;
  
  ___i686_get_pc_thunk_bx();
  (*(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1a])(this);
  this_01 = &(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Collision;
  CCollisionProperty::SetSolid(&this_01->super_CCollisionProperty,SOLID_VPHYSICS);
  CCollisionProperty::SetSolidFlags
            (&this_01->super_CCollisionProperty,
             (this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Collision.
             super_CCollisionProperty.m_usSolidFlags.m_Value | 0xc);
  CBaseEntity::SetMoveType((CBaseEntity *)this,MOVETYPE_NONE,MOVECOLLIDE_DEFAULT);
  (*(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x9f])(this);
  CBaseEntity::SetTransmitState((CBaseEntity *)this,0x20);
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0x122);
  CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,*(float *)(**(int **)(unaff_EBX + 0x4b0382) + 0xc),(char *)0x0);
  if ((this->m_bDisablePlayerMove).m_Value != false) {
    if ((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
        false) {
      this_00 = &((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_pPev)->
                 super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar2->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bDisablePlayerMove).m_Value = false;
  }
  return;
}


/* CTrigger_TractorBeam::Precache at 006fb3f0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void Precache(CTrigger_TractorBeam * this) */

void __thiscall CTrigger_TractorBeam::Precache(CTrigger_TractorBeam *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  PrecacheParticleSystem((char *)(unaff_EBX + 0x318a65));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x318a76));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x318a8b));
  return;
}


/* CTrigger_TractorBeam::Activate at 006fbca0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void Activate(CTrigger_TractorBeam * this) */

void __thiscall CTrigger_TractorBeam::Activate(CTrigger_TractorBeam *this)

{
  edict_t *peVar1;
  CSoundEnvelopeController *pCVar2;
  CSoundPatch *__in_chrg;
  int iVar3;
  int unaff_EBX;
  CRecipientFilter local_3c;
  
                    /* Unresolved local var: CSoundEnvelopeController * controller@[???]
                       Unresolved local var: CPASFilter filter@[???] */
  ___i686_get_pc_thunk_bx();
  CBaseVPhysicsTrigger::Activate(&this->super_CBaseVPhysicsTrigger);
  pCVar2 = CSoundEnvelopeController::GetController();
  if (((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
  CRecipientFilter::CRecipientFilter(&local_3c);
  local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(unaff_EBX + 0x4b24da);
  CRecipientFilter::AddRecipientsByPAS
            (&local_3c,&(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_vecAbsOrigin);
  peVar1 = (this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_pPev;
                    /* Unresolved local var: int edictIndex@[???] */
  iVar3 = 0;
  if (peVar1 != (edict_t *)0x0) {
    iVar3 = (int)peVar1 - *(int *)(**(int **)(unaff_EBX + 0x4afef2) + 0x58) >> 4;
  }
  __in_chrg = (CSoundPatch *)
              (*pCVar2->_vptr_CSoundEnvelopeController[6])
                        (pCVar2,&local_3c,iVar3,unaff_EBX + 0x3181d9);
  this->m_sndAmbient = __in_chrg;
  (*pCVar2->_vptr_CSoundEnvelopeController[2])(pCVar2,__in_chrg,0x3f800000,0x42c80000,0);
  local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(unaff_EBX + 0x4b24da);
  CRecipientFilter::~CRecipientFilter(&local_3c,(int)__in_chrg);
  return;
}


/* CTrigger_TractorBeam::TractorThink at 006fb940 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void TractorThink(CTrigger_TractorBeam * this) */

void __thiscall CTrigger_TractorBeam::TractorThink(CTrigger_TractorBeam *this)

{
  undefined1 *puVar1;
  uint uVar2;
  int *piVar3;
  CBaseEdict *pCVar4;
  int iVar5;
  IChangeInfoAccessor *pIVar6;
  int *piVar7;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  piVar7 = *(int **)(unaff_EBX + 0x4b0250);
  uVar2 = (this->m_hProxyEntity).
          super_CNetworkVarBase<CBaseHandle,CTrigger_TractorBeam::NetworkVar_m_hProxyEntity>.m_Value
          .m_Index;
  if (((uVar2 != 0xffffffff) &&
      (iVar5 = (uVar2 & 0xffff) * 0x10 + *piVar7, *(uint *)(iVar5 + 8) == uVar2 >> 0x10)) &&
     (piVar3 = *(int **)(iVar5 + 4), piVar3 != (int *)0x0)) {
    iVar5 = (**(code **)(*piVar3 + 0x324))(piVar3);
    if ((iVar5 != 0) != (this->m_bFromPortal).m_Value) {
      if ((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered
          == false) {
        pCVar4 = &((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if (pCVar4 != (CBaseEdict *)0x0) {
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x101;
          pIVar6 = CBaseEdict::GetChangeAccessor(pCVar4);
          pIVar6->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->m_bFromPortal).m_Value = iVar5 != 0;
    }
    uVar2 = (this->m_hProxyEntity).
            super_CNetworkVarBase<CBaseHandle,CTrigger_TractorBeam::NetworkVar_m_hProxyEntity>.
            m_Value.m_Index;
    if ((uVar2 == 0xffffffff) ||
       (iVar5 = (uVar2 & 0xffff) * 0x10 + *piVar7, *(uint *)(iVar5 + 8) != uVar2 >> 0x10)) {
      piVar7 = (int *)0x0;
    }
    else {
      piVar7 = *(int **)(iVar5 + 4);
    }
    iVar5 = (**(code **)(*piVar7 + 0x31c))(piVar7);
    if ((iVar5 != 0) != (this->m_bToPortal).m_Value) {
      if ((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered
          == false) {
        pCVar4 = &((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if (pCVar4 != (CBaseEdict *)0x0) {
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x101;
          pIVar6 = CBaseEdict::GetChangeAccessor(pCVar4);
          pIVar6->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->m_bToPortal).m_Value = iVar5 != 0;
    }
  }
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,*(float *)(**(int **)(unaff_EBX + 0x4b024c) + 0xc),(char *)0x0);
  return;
}


/* CTrigger_TractorBeam::CreateVPhysics at 006fb080 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: bool CreateVPhysics(CTrigger_TractorBeam * this) */

bool __thiscall CTrigger_TractorBeam::CreateVPhysics(CTrigger_TractorBeam *this)

{
  IPhysicsMotionController *pIVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  pIVar1 = (IPhysicsMotionController *)
           (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4b0ba7) + 0x78))
                     ((int *)**(undefined4 **)(unaff_EBX + 0x4b0ba7),&this->super_IMotionEvent);
  this->m_pController = pIVar1;
  return true;
}


/* CTrigger_TractorBeam::WakeTouchingObjects at 006fb430 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void WakeTouchingObjects(CTrigger_TractorBeam * this) */

void __thiscall CTrigger_TractorBeam::WakeTouchingObjects(CTrigger_TractorBeam *this)

{
  float fVar1;
  CBaseEntity *pCVar2;
  IPhysicsObject *pIVar3;
  int iVar4;
  int unaff_EBX;
  int iVar5;
  CBaseEntity *local_4cc [256];
  Ray_t local_cc;
  CFlaggedEntitiesEnum local_78;
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
  
                    /* Unresolved local var: CBaseEntity *[256] list@[???]
                       Unresolved local var: CBaseEntity * pEntity@[???]
                       Unresolved local var: Ray_t ray@[???]
                       Unresolved local var: Vector vExtents@[???]
                       Unresolved local var: int nNumFound@[???] */
  ___i686_get_pc_thunk_bx();
  fVar1 = (this->m_flRadius).m_Value;
  local_4c = (float)((uint)fVar1 ^ *(uint *)(unaff_EBX + 0x33885f));
  local_58 = (this->m_vEnd).m_Value.x;
  local_54 = (this->m_vEnd).m_Value.y;
  local_50 = (this->m_vEnd).m_Value.z;
  local_64 = (this->m_vStart).m_Value.x;
  local_60 = (this->m_vStart).m_Value.y;
  local_5c = (this->m_vStart).m_Value.z;
  local_cc.m_Delta.super_Vector.x = local_58 - local_64;
  local_cc.m_Delta.super_Vector.y = local_54 - local_60;
  local_cc.m_Delta.super_Vector.z = local_50 - local_5c;
  local_cc.m_pWorldAxisTransform = (matrix3x4_t *)0x0;
  local_cc.m_IsSwept =
       local_cc.m_Delta.super_Vector.x * local_cc.m_Delta.super_Vector.x +
       local_cc.m_Delta.super_Vector.y * local_cc.m_Delta.super_Vector.y +
       local_cc.m_Delta.super_Vector.z * local_cc.m_Delta.super_Vector.z !=
       *(float *)(unaff_EBX + 0x32f12f);
  local_cc.m_StartOffset.super_Vector.z = *(float *)(unaff_EBX + 0x32f0bf);
  local_cc.m_Extents.super_Vector.x = (fVar1 - local_4c) * local_cc.m_StartOffset.super_Vector.z;
  local_cc.m_Extents.super_Vector.y = (fVar1 - local_4c) * local_cc.m_StartOffset.super_Vector.z;
  local_cc.m_Extents.super_Vector.z = (fVar1 - local_4c) * local_cc.m_StartOffset.super_Vector.z;
  local_cc.m_IsRay =
       (double)(local_cc.m_Extents.super_Vector.x * local_cc.m_Extents.super_Vector.x +
                local_cc.m_Extents.super_Vector.y * local_cc.m_Extents.super_Vector.y +
               local_cc.m_Extents.super_Vector.z * local_cc.m_Extents.super_Vector.z) <
       *(double *)(unaff_EBX + 0x32f2af);
  local_cc.m_StartOffset.super_Vector.x = (fVar1 + local_4c) * local_cc.m_StartOffset.super_Vector.z
  ;
  local_cc.m_StartOffset.super_Vector.y = (fVar1 + local_4c) * local_cc.m_StartOffset.super_Vector.z
  ;
  local_cc.m_StartOffset.super_Vector.z = (fVar1 + local_4c) * local_cc.m_StartOffset.super_Vector.z
  ;
  local_cc.m_Start.super_Vector.x = local_64 + local_cc.m_StartOffset.super_Vector.x;
  local_cc.m_Start.super_Vector.y = local_60 + local_cc.m_StartOffset.super_Vector.y;
  local_cc.m_Start.super_Vector.z = local_5c + local_cc.m_StartOffset.super_Vector.z;
  local_cc.m_StartOffset.super_Vector.x = -local_cc.m_StartOffset.super_Vector.x;
  local_cc.m_StartOffset.super_Vector.y = -local_cc.m_StartOffset.super_Vector.y;
  local_cc.m_StartOffset.super_Vector.z = -local_cc.m_StartOffset.super_Vector.z;
                    /* Unresolved local var: CFlaggedEntitiesEnum rayEnum@[???] */
  local_48 = local_4c;
  local_44 = local_4c;
  local_40 = local_4c;
  local_3c = local_4c;
  local_38 = local_4c;
  local_34 = local_58;
  local_30 = local_54;
  local_2c = local_50;
  local_28 = local_64;
  local_24 = local_60;
  local_20 = local_5c;
  CFlaggedEntitiesEnum::CFlaggedEntitiesEnum(&local_78,local_4cc,0x100,0);
  iVar4 = UTIL_EntitiesAlongRay(&local_cc,&local_78);
                    /* Unresolved local var: int i@[???] */
  if (0 < iVar4) {
    iVar5 = 0;
    do {
                    /* Unresolved local var: IPhysicsObject * pPhysObj@[???] */
      pCVar2 = local_4cc[iVar5];
      if ((pCVar2 != (CBaseEntity *)0x0) &&
         (pIVar3 = pCVar2->m_pPhysicsObject, pIVar3 != (IPhysicsObject *)0x0)) {
        (*pIVar3->_vptr_IPhysicsObject[0x19])(pIVar3);
      }
      iVar5 = iVar5 + 1;
    } while (iVar5 != iVar4);
  }
  return;
}


/* CTrigger_TractorBeam::UpdateOnRemove at 006fb310 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void UpdateOnRemove(CTrigger_TractorBeam * this) */

void __thiscall CTrigger_TractorBeam::UpdateOnRemove(CTrigger_TractorBeam *this)

{
  IPhysicsMotionController *pIVar1;
  int *piVar2;
  int iVar3;
  void *pMem;
  int unaff_EBX;
  int iVar4;
  IPhysicsObject **pObjects;
  
  ___i686_get_pc_thunk_bx();
  pIVar1 = this->m_pController;
  if (pIVar1 != (IPhysicsMotionController *)0x0) {
                    /* Unresolved local var: int nNumObjects@[???] */
    iVar3 = (*pIVar1->_vptr_IPhysicsMotionController[5])(pIVar1);
    pMem = operator_new___(iVar3 * 4);
    (*this->m_pController->_vptr_IPhysicsMotionController[6])(this->m_pController,pMem);
                    /* Unresolved local var: int i@[???] */
    if (0 < iVar3) {
      iVar4 = 0;
      do {
        piVar2 = *(int **)((int)pMem + iVar4 * 4);
        if (piVar2 != (int *)0x0) {
          (**(code **)(*piVar2 + 100))(piVar2);
        }
        iVar4 = iVar4 + 1;
      } while (iVar4 != iVar3);
    }
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4b091a) + 0x7c))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4b091a),this->m_pController);
    this->m_pController = (IPhysicsMotionController *)0x0;
    if (pMem != (void *)0x0) {
      operator_delete___(pMem);
    }
  }
  StopParticleEffects((CBaseEntity *)this);
  (*(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x6f])(this);
  CBaseVPhysicsTrigger::UpdateOnRemove(&this->super_CBaseVPhysicsTrigger);
  return;
}


/* CTrigger_TractorBeam::OnRestore at 006fb2c0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void OnRestore(CTrigger_TractorBeam * this) */

void __thiscall CTrigger_TractorBeam::OnRestore(CTrigger_TractorBeam *this)

{
  IPhysicsMotionController *pIVar1;
  
  CBaseEntity::OnRestore((CBaseEntity *)this);
  pIVar1 = this->m_pController;
  if (pIVar1 != (IPhysicsMotionController *)0x0) {
    (*pIVar1->_vptr_IPhysicsMotionController[2])(pIVar1,&this->super_IMotionEvent);
  }
  CBaseEntity::SetTransmitState((CBaseEntity *)this,0x20);
  return;
}


/* CTrigger_TractorBeam::StopLoopingSounds at 006fb230 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void StopLoopingSounds(CTrigger_TractorBeam * this) */

void __thiscall CTrigger_TractorBeam::StopLoopingSounds(CTrigger_TractorBeam *this)

{
  CSoundEnvelopeController *pCVar1;
  
  if (this->m_sndAmbient != (CSoundPatch *)0x0) {
                    /* Unresolved local var: CSoundEnvelopeController * controller@[???] */
    pCVar1 = CSoundEnvelopeController::GetController();
    (*pCVar1->_vptr_CSoundEnvelopeController[5])(pCVar1,this->m_sndAmbient);
    (*pCVar1->_vptr_CSoundEnvelopeController[0xb])(pCVar1,this->m_sndAmbient);
    this->m_sndAmbient = (CSoundPatch *)0x0;
  }
  if (this->m_sndPlayerInBeam != (CSoundPatch *)0x0) {
                    /* Unresolved local var: CSoundEnvelopeController * controller@[???] */
    pCVar1 = CSoundEnvelopeController::GetController();
    (*pCVar1->_vptr_CSoundEnvelopeController[5])(pCVar1,this->m_sndPlayerInBeam);
    (*pCVar1->_vptr_CSoundEnvelopeController[0xb])(pCVar1,this->m_sndPlayerInBeam);
    this->m_sndPlayerInBeam = (CSoundPatch *)0x0;
  }
  return;
}


/* CTrigger_TractorBeam::StartTouch at 006fbdd0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void StartTouch(CTrigger_TractorBeam * this, CBaseEntity * pOther) */

void __thiscall CTrigger_TractorBeam::StartTouch(CTrigger_TractorBeam *this,CBaseEntity *pOther)

{
  Vector *origin;
  edict_t *peVar1;
  bool bVar2;
  int iVar3;
  CSoundEnvelopeController *pCVar4;
  CSoundPatch *__in_chrg;
  int unaff_EBX;
  undefined4 local_98;
  int local_94;
  undefined4 local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  undefined4 local_80;
  undefined4 local_7c;
  undefined4 local_78;
  undefined1 local_74;
  undefined1 local_73;
  undefined1 local_72;
  undefined4 local_70;
  int local_6c;
  undefined4 local_68;
  int local_64;
  undefined4 local_60;
  int local_5c;
  undefined2 local_58;
  undefined4 local_54;
  CRecipientFilter local_50;
  triggerevent_t local_30;
  
                    /* Unresolved local var: CBasePlayer * pPlayer@[???]
                       Unresolved local var: triggerevent_t event@[???] */
  ___i686_get_pc_thunk_bx();
  CBaseVPhysicsTrigger::StartTouch(&this->super_CBaseVPhysicsTrigger,pOther);
  iVar3 = (*(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.
            super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xc5])(this,pOther);
  if ((char)iVar3 != '\0') {
    if ((pOther != (CBaseEntity *)0x0) &&
       (iVar3 = (*(pOther->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                  _vptr_IHandleEntity[0x55])(pOther), (char)iVar3 != '\0')) {
      CBasePlayer::SetPhysicsFlag((CBasePlayer *)pOther,0x10,true);
      if (*(char *)&pOther[2].m_vecOrigin.
                    super_CNetworkVarBase<Vector,CBaseEntity::NetworkVar_m_vecOrigin>.m_Value.x ==
          '\0') {
        (*(code *)(pOther[2].m_DamageModifiers.m_pElements)->m_Previous)
                  (&pOther[2].m_DamageModifiers.m_pElements,&pOther[2].m_vecOrigin);
        *(undefined1 *)
         &pOther[2].m_vecOrigin.super_CNetworkVarBase<Vector,CBaseEntity::NetworkVar_m_vecOrigin>.
          m_Value.x = 1;
      }
      pOther->m_flGravity = 1.1754944e-38;
      pCVar4 = CSoundEnvelopeController::GetController();
      if (this->m_sndPlayerInBeam != (CSoundPatch *)0x0) {
        (*pCVar4->_vptr_CSoundEnvelopeController[5])(pCVar4,this->m_sndPlayerInBeam);
        (*pCVar4->_vptr_CSoundEnvelopeController[0xb])(pCVar4,this->m_sndPlayerInBeam);
        this->m_sndPlayerInBeam = (CSoundPatch *)0x0;
      }
      local_88 = 0;
      local_84 = 100;
      local_80 = 0;
      local_7c = 0;
      local_78 = 0;
      local_74 = 1;
      local_73 = 0;
      local_72 = 0;
      local_70 = 0xffffffff;
      local_6c = 0;
      local_68 = 0;
      local_64 = 0;
      local_60 = 0;
      local_5c = 0;
      local_58 = 0xffff;
      local_54 = 1;
      local_98 = 6;
      local_94 = unaff_EBX + 0x318091;
      local_90 = 0x3f800000;
      local_8c = 0x4b;
      if (((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
        CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
      }
      origin = &(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_vecAbsOrigin;
      CRecipientFilter::CRecipientFilter(&local_50);
      local_50.super_IRecipientFilter._vptr_IRecipientFilter =
           (_func_int_varargs **)(unaff_EBX + 0x4b23a7);
      CRecipientFilter::AddRecipientsByPAS(&local_50,origin);
      local_50.super_IRecipientFilter._vptr_IRecipientFilter =
           (_func_int_varargs **)(unaff_EBX + 0x4b2367);
      CPASAttenuationFilter::Filter((CPASAttenuationFilter *)&local_50,origin,0.8);
      CRecipientFilter::MakeReliable(&local_50);
      CRecipientFilter::RemoveAllRecipients(&local_50);
      CRecipientFilter::AddRecipient(&local_50,(CBasePlayer *)pOther);
      peVar1 = (this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_pPev;
                    /* Unresolved local var: int edictIndex@[???] */
      iVar3 = 0;
      if (peVar1 != (edict_t *)0x0) {
        iVar3 = (int)peVar1 - *(int *)(**(int **)(unaff_EBX + 0x4afdbf) + 0x58) >> 4;
      }
      __in_chrg = (CSoundPatch *)
                  (*pCVar4->_vptr_CSoundEnvelopeController[10])(pCVar4,&local_50,iVar3,&local_98);
      this->m_sndPlayerInBeam = __in_chrg;
      (*pCVar4->_vptr_CSoundEnvelopeController[2])(pCVar4,__in_chrg,0x3f800000,0x42c80000,0);
      local_50.super_IRecipientFilter._vptr_IRecipientFilter =
           (_func_int_varargs **)(unaff_EBX + 0x4b23a7);
      CRecipientFilter::~CRecipientFilter(&local_50,(int)__in_chrg);
      local_60 = 0;
      if (-1 < local_64) {
        if (local_6c != 0) {
          (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4afdb7) + 8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x4afdb7),local_6c);
          local_6c = 0;
        }
        local_68 = 0;
      }
      local_5c = local_6c;
      if (-1 < local_64) {
        if (local_6c != 0) {
          (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4afdb7) + 8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x4afdb7),local_6c);
          local_6c = 0;
        }
        local_68 = 0;
      }
    }
    if (((this->m_pController != (IPhysicsMotionController *)0x0) &&
        (bVar2 = PhysGetTriggerEvent(&local_30,(CBaseEntity *)this), bVar2)) &&
       (local_30.pObject != (IPhysicsObject *)0x0)) {
      (*(local_30.pObject)->_vptr_IPhysicsObject[0x19])(local_30.pObject);
      (*this->m_pController->_vptr_IPhysicsMotionController[3])
                (this->m_pController,local_30.pObject,1);
    }
  }
  return;
}


/* CTrigger_TractorBeam::EndTouch at 006fb6a0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void EndTouch(CTrigger_TractorBeam * this, CBaseEntity * pOther) */

void __thiscall CTrigger_TractorBeam::EndTouch(CTrigger_TractorBeam *this,CBaseEntity *pOther)

{
  bool bVar1;
  int iVar2;
  CSoundEnvelopeController *pCVar3;
  triggerevent_t local_20;
  
                    /* Unresolved local var: CBasePlayer * pPlayer@[???]
                       Unresolved local var: triggerevent_t event@[???] */
  CBaseVPhysicsTrigger::EndTouch(&this->super_CBaseVPhysicsTrigger,pOther);
  iVar2 = (*(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.
            super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xc5])(this,pOther);
  if ((char)iVar2 != '\0') {
    if (pOther != (CBaseEntity *)0x0) {
      iVar2 = (*(pOther->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                _vptr_IHandleEntity[0x55])(pOther);
      if ((char)iVar2 != '\0') {
        CBasePlayer::SetPhysicsFlag((CBasePlayer *)pOther,0x10,false);
        if (*(char *)&pOther[2].m_vecOrigin.
                      super_CNetworkVarBase<Vector,CBaseEntity::NetworkVar_m_vecOrigin>.m_Value.x !=
            '\0') {
          (*(code *)(pOther[2].m_DamageModifiers.m_pElements)->m_Previous)
                    (&pOther[2].m_DamageModifiers.m_pElements,&pOther[2].m_vecOrigin);
          *(undefined1 *)
           &pOther[2].m_vecOrigin.super_CNetworkVarBase<Vector,CBaseEntity::NetworkVar_m_vecOrigin>.
            m_Value.x = 0;
        }
        pOther->m_flGravity = 1.0;
        if (this->m_sndPlayerInBeam != (CSoundPatch *)0x0) {
          pCVar3 = CSoundEnvelopeController::GetController();
          (*pCVar3->_vptr_CSoundEnvelopeController[0xe])
                    (pCVar3,this->m_sndPlayerInBeam,0x3f000000,0);
        }
      }
    }
    bVar1 = PhysGetTriggerEvent(&local_20,(CBaseEntity *)this);
    if (((bVar1) && (local_20.pObject != (IPhysicsObject *)0x0)) &&
       (this->m_pController != (IPhysicsMotionController *)0x0)) {
      (*(local_20.pObject)->_vptr_IPhysicsObject[0x19])(local_20.pObject);
      (*this->m_pController->_vptr_IPhysicsMotionController[4])
                (this->m_pController,local_20.pObject);
    }
  }
  return;
}


/* CTrigger_TractorBeam::InputSetVelocityLimitTime at 006fbaa0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void InputSetVelocityLimitTime(CTrigger_TractorBeam * this, inputdata_t
   * inputdata) */

void __thiscall
CTrigger_TractorBeam::InputSetVelocityLimitTime(CTrigger_TractorBeam *this,inputdata_t *inputdata)

{
  undefined1 *puVar1;
  CBaseEdict *pCVar2;
  char *pString;
  IChangeInfoAccessor *pIVar3;
  int unaff_EBX;
  float fVar4;
  CNetworkVarBase<float,CTrigger_TractorBeam::NetworkVar_m_linearLimit> local_24;
  float local_20;
  
                    /* Unresolved local var: float[2] args@[???] */
  ___i686_get_pc_thunk_bx();
  fVar4 = LinearLimit(this);
  if (fVar4 != (this->m_linearLimitStart).m_Value) {
    if ((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
        false) {
      pCVar2 = &((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_pPev)->
                super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_linearLimitStart).m_Value = fVar4;
  }
  fVar4 = *(float *)(**(int **)(unaff_EBX + 0x4b00f2) + 0xc);
  if (fVar4 != (this->m_linearLimitStartTime).m_Value) {
    if ((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
        false) {
      pCVar2 = &((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_pPev)->
                super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_linearLimitStartTime).m_Value = fVar4;
  }
  if ((inputdata->value).fieldType == FIELD_STRING) {
    pString = (char *)(inputdata->value).field_0.iVal;
    if (pString == (char *)0x0) {
      pString = &UNK_002b00ae + unaff_EBX;
    }
  }
  else {
    pString = variant_t::ToString(&inputdata->value);
  }
  UTIL_StringToFloatArray(&local_24.m_Value,2,pString);
  if (local_24.m_Value != (this->m_linearLimit).m_Value) {
    if ((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
        false) {
      pCVar2 = &((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_pPev)->
                super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_linearLimit).m_Value = local_24.m_Value;
  }
  if (local_20 != (this->m_linearLimitTime).m_Value) {
    if ((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
        false) {
      pCVar2 = &((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_pPev)->
                super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_linearLimitTime).m_Value = local_20;
  }
  return;
}


/* CTrigger_TractorBeam::CreateTractorBeam at 006fb1c0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

CTrigger_TractorBeam *
CTrigger_TractorBeam::CreateTractorBeam(Vector *vStart,Vector *vEnd,CBaseEntity *pOwner)

{
  CTrigger_TractorBeam *this;
  int unaff_EBX;
  CTrigger_TractorBeam *pCVar1;
  
                    /* Unresolved local var: CTrigger_TractorBeam * pBeam@[???] */
  ___i686_get_pc_thunk_bx();
  this = (CTrigger_TractorBeam *)CreateEntityByName((char *)(unaff_EBX + 0x2bcb34),-1,true);
  pCVar1 = (CTrigger_TractorBeam *)0x0;
  if (this != (CTrigger_TractorBeam *)0x0) {
    (*(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
      super_IHandleEntity._vptr_IHandleEntity[0x13])(this,pOwner);
    UpdateBeam(this);
    DispatchSpawn((CBaseEntity *)this,true);
    pCVar1 = this;
  }
  return pCVar1;
}


/* CProjectedTractorBeamEntity::Spawn at 006fcad0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(CProjectedTractorBeamEntity * this) */

void __thiscall CProjectedTractorBeamEntity::Spawn(CProjectedTractorBeamEntity *this)

{
  undefined1 *puVar1;
  uint uVar2;
  CBaseEdict *this_00;
  int iVar3;
  CTrigger_TractorBeam *pCVar4;
  IChangeInfoAccessor *pIVar5;
  ulong *puVar6;
  int unaff_EBX;
  CTrigger_TractorBeam *local_20;
  
  ___i686_get_pc_thunk_bx();
  CBaseProjectedEntity::Spawn(&this->super_CBaseProjectedEntity);
  local_20 = (CTrigger_TractorBeam *)CreateEntityByName((char *)(unaff_EBX + 0x2bb222),-1,true);
  if (local_20 == (CTrigger_TractorBeam *)0x0) {
    local_20 = (CTrigger_TractorBeam *)0x0;
  }
  else {
    (*(local_20->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.
      super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x13])(local_20,this);
    CTrigger_TractorBeam::UpdateBeam(local_20);
    DispatchSpawn((CBaseEntity *)local_20,true);
  }
  uVar2 = (this->m_hTractorBeamTrigger).
          super_CNetworkVarBase<CBaseHandle,CProjectedTractorBeamEntity::NetworkVar_m_hTractorBeamTrigger>
          .m_Value.m_Index;
  if ((uVar2 == 0xffffffff) ||
     (iVar3 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4af0c6),
     *(uint *)(iVar3 + 8) != uVar2 >> 0x10)) {
    pCVar4 = (CTrigger_TractorBeam *)0x0;
  }
  else {
    pCVar4 = *(CTrigger_TractorBeam **)(iVar3 + 4);
  }
  if (pCVar4 != local_20) {
    if ((this->super_CBaseProjectedEntity).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
        false) {
      this_00 = &((this->super_CBaseProjectedEntity).super_CBaseEntity.m_Network.m_pPev)->
                 super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar5 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar5->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseProjectedEntity).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    if (local_20 == (CTrigger_TractorBeam *)0x0) {
      (this->m_hTractorBeamTrigger).
      super_CNetworkVarBase<CBaseHandle,CProjectedTractorBeamEntity::NetworkVar_m_hTractorBeamTrigger>
      .m_Value.m_Index = 0xffffffff;
    }
    else {
      puVar6 = (ulong *)(*(local_20->super_CBaseVPhysicsTrigger).super_CBaseEntity.
                          super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
                          _vptr_IHandleEntity[3])(local_20);
      (this->m_hTractorBeamTrigger).
      super_CNetworkVarBase<CBaseHandle,CProjectedTractorBeamEntity::NetworkVar_m_hTractorBeamTrigger>
      .m_Value.m_Index = *puVar6;
    }
  }
  CBaseEntity::SetTransmitState((CBaseEntity *)this,8);
  return;
}


/* CProjectedTractorBeamEntity::CreateNewInstance at 006fb180 */

CProjectedTractorBeamEntity * CProjectedTractorBeamEntity::CreateNewInstance(void)

{
  CProjectedTractorBeamEntity *pCVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  pCVar1 = (CProjectedTractorBeamEntity *)CreateEntityByName((char *)(unaff_EBX + 0x2bcb56),-1,true)
  ;
  return pCVar1;
}


/* CProjectedTractorBeamEntity::CreateNewProjectedEntity at 006fb7d0 */

/* DWARF original prototype: CBaseProjectedEntity *
   CreateNewProjectedEntity(CProjectedTractorBeamEntity * this) */

CBaseProjectedEntity * __thiscall
CProjectedTractorBeamEntity::CreateNewProjectedEntity(CProjectedTractorBeamEntity *this)

{
  CBaseProjectedEntity *pCVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  pCVar1 = (CBaseProjectedEntity *)CreateEntityByName((char *)(unaff_EBX + 0x2bc506),-1,true);
  return pCVar1;
}


/* CProjectedTractorBeamEntity::OnProjected at 006fb0d0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void OnProjected(CProjectedTractorBeamEntity * this) */

void __thiscall CProjectedTractorBeamEntity::OnProjected(CProjectedTractorBeamEntity *this)

{
  uint uVar1;
  CTrigger_TractorBeam *this_00;
  int iVar2;
  CBasePlayer *pOwner;
  int unaff_EBX;
  
                    /* Unresolved local var: CTrigger_TractorBeam * pBeam@[???] */
  ___i686_get_pc_thunk_bx();
  CBaseProjectedEntity::OnProjected(&this->super_CBaseProjectedEntity);
  uVar1 = (this->m_hTractorBeamTrigger).
          super_CNetworkVarBase<CBaseHandle,CProjectedTractorBeamEntity::NetworkVar_m_hTractorBeamTrigger>
          .m_Value.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4b0ac0),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (this_00 = *(CTrigger_TractorBeam **)(iVar2 + 4), this_00 != (CTrigger_TractorBeam *)0x0)) {
    if ((this->super_CBaseProjectedEntity).super_CBaseEntity.m_bIsPlayerSimulated.m_Value == false)
    {
      CBaseEntity::UnsetPlayerSimulated((CBaseEntity *)this_00);
    }
    else {
      uVar1 = (this->super_CBaseProjectedEntity).super_CBaseEntity.m_hPlayerSimulationOwner.
              super_CNetworkVarBase<CBaseHandle,CBaseEntity::NetworkVar_m_hPlayerSimulationOwner>.
              m_Value.m_Index;
      if ((uVar1 == 0xffffffff) ||
         (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4b0ac0),
         *(uint *)(iVar2 + 8) != uVar1 >> 0x10)) {
        pOwner = (CBasePlayer *)0x0;
      }
      else {
        pOwner = *(CBasePlayer **)(iVar2 + 4);
      }
      CBaseEntity::SetPlayerSimulated((CBaseEntity *)this_00,pOwner);
    }
    CTrigger_TractorBeam::UpdateBeam(this_00);
    return;
  }
  return;
}


/* _GLOBAL__I__ZN27CProjectedTractorBeamEntity9m_DataMapE at 000d0d20 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN27CProjectedTractorBeamEntity9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

