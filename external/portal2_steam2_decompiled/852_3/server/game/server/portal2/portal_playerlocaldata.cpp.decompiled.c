/* DWARF-guided pseudocode for game/server/portal2/portal_playerlocaldata.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* ServerClassInit<DT_PortalLocal::ignored> at 000c52e0 */

/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */

int ServerClassInit<DT_PortalLocal::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  char *pcVar2;
  SendProp local_16c;
  SendProp local_118;
  SendProp local_c4;
  SendProp local_70;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&ai_shot_bias_min.super_ConCommandBase.m_pNext + unaff_EBX + 3) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf4047);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  ((int)&ai_shot_bias_min.super_ConCommandBase.m_pszName + unaff_EBX + 3),
                  (char *)(unaff_EBX + 0x8ed733),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&ai_shot_bias_max.super_ConCommandBase._vptr_ConCommandBase +
                    unaff_EBX + 3),(char *)(unaff_EBX + 0x94b96c),4,4,0x20,0,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00ae6993 + unaff_EBX),0x80);
      pcVar2 = (char *)(unaff_EBX + 0x94b989);
      SendPropInt((SendProp_conflict *)&local_70,pcVar2,8,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropArray3((SendProp_conflict *)
                     ((int)&ai_shot_bias_max.m_fnChangeCallbacks.m_pElements + unaff_EBX + 3),pcVar2
                     ,8,4,0x10,(SendProp_conflict *)&local_70,
                     *(SendTableProxyFn_conflict *)(&DAT_00ae693b + unaff_EBX),0x80);
      SendProp::~SendProp(&local_70,(int)pcVar2);
      SendPropBool((SendProp_conflict *)
                   ((int)&ai_debug_shoot_positions.m_fnChangeCallbacks.m_Memory.m_nAllocationCount +
                   unaff_EBX + 3),(char *)(unaff_EBX + 0x94b9a1),0x48,1);
      pcVar2 = (char *)(unaff_EBX + 0x94b9b1);
      SendPropVector((SendProp_conflict *)&local_c4,pcVar2,0x4c,0xc,0x20,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00ae6987 + unaff_EBX),0x80);
      SendPropArray3((SendProp_conflict *)(&sv_alternateticks.field_0x3f + unaff_EBX),pcVar2,0x4c,
                     0xc,4,(SendProp_conflict *)&local_c4,
                     *(SendTableProxyFn_conflict *)(&DAT_00ae693b + unaff_EBX),0x80);
      SendProp::~SendProp(&local_c4,(int)pcVar2);
      pcVar2 = (char *)(unaff_EBX + 0x94b9cd);
      SendPropVector((SendProp_conflict *)&local_118,pcVar2,0x7c,0xc,0x20,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00ae6987 + unaff_EBX),0x80);
      SendPropArray3((SendProp_conflict *)(unaff_EBX + 0xcf41f3),pcVar2,0x7c,0xc,4,
                     (SendProp_conflict *)&local_118,
                     *(SendTableProxyFn_conflict *)(&DAT_00ae693b + unaff_EBX),0x80);
      SendProp::~SendProp(&local_118,(int)pcVar2);
      pcVar2 = (char *)(unaff_EBX + 0x94b9e7);
      SendPropBool((SendProp_conflict *)&local_16c,pcVar2,0xac,1);
      SendPropArray3((SendProp_conflict *)
                     ((int)&sv_infinite_ammo.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 3)
                     ,pcVar2,0xac,1,4,(SendProp_conflict *)&local_16c,
                     *(SendTableProxyFn_conflict *)(&DAT_00ae693b + unaff_EBX),0x80);
      SendProp::~SendProp(&local_16c,(int)pcVar2);
      SendPropVector((SendProp_conflict *)((int)&sv_debug_player_use.m_fMinVal + unaff_EBX + 3),
                     (char *)(unaff_EBX + 0x94ba03),0xb0,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00ae6987 + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&sv_suppress_viewpunch.m_Value.m_fValue + unaff_EBX + 3),
                     (char *)(unaff_EBX + 0x94ba11),0xbc,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00ae6987 + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&sv_showanimstate.super_ConCommandBase._vptr_ConCommandBase +
                     unaff_EBX + 3),(char *)(unaff_EBX + 0x94ba22),200,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00ae6987 + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&sv_showanimstate.m_fnChangeCallbacks.m_pElements + unaff_EBX + 3),
                     (char *)(unaff_EBX + 0x94ba37),0xd4,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00ae6987 + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&showanimstate_log.m_fnChangeCallbacks.m_Memory.m_nAllocationCount +
                     unaff_EBX + 3),(char *)(unaff_EBX + 0x94ba3c),0xe0,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00ae6987 + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)(&showanimstate_activities.field_0x3f + unaff_EBX),
                     (char *)(unaff_EBX + 0x94ba51),0xec,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00ae6987 + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)((int)&mp_feetyawrate.m_Value.m_nValue + unaff_EBX + 3),
                     (char *)(unaff_EBX + 0x94ba60),0xf8,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00ae6987 + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&mp_facefronttime.m_Value.m_pszString + unaff_EBX + 3),
                     (char *)(unaff_EBX + 0x94ba6f),0x104,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00ae6987 + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)((int)&mp_ik.super_IConVar._vptr_IConVar + unaff_EBX + 3),
                     (char *)(unaff_EBX + 0x94ba7d),0x110,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00ae6987 + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&DT_Beam::g_SendTable.m_pNetTableName + unaff_EBX + 3),
                     (char *)(unaff_EBX + 0x94ba8b),0x11c,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00ae6987 + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&DT_EntityParticleTrailInfo::g_SendTable.m_pNetTableName + unaff_EBX + 3)
                     ,(char *)(unaff_EBX + 0x94baa7),0x128,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00ae6987 + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&sv_showladders.super_ConCommandBase.m_nFlags + unaff_EBX + 3),
                     (char *)(unaff_EBX + 0x94bac3),0x134,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00ae6987 + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)((int)&CFuncLadder::s_Ladders.m_pElements + unaff_EBX + 3)
                     ,(char *)(unaff_EBX + 0x94bade),0x140,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00ae6987 + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)((int)&option_duck_method.m_pParent + unaff_EBX + 3),
                     (char *)(unaff_EBX + 0x94baf9),0x14c,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00ae6987 + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)((int)&rgv3tStuckTable[1].y + unaff_EBX + 3),
                     (char *)(unaff_EBX + 0x94bb04),0x158,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00ae6987 + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)((int)&rgv3tStuckTable[8].y + unaff_EBX + 3),
                     (char *)(unaff_EBX + 0x94bb11),0x164,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00ae6987 + unaff_EBX),0x80);
      SendPropQAngles((SendProp_conflict *)((int)&rgv3tStuckTable[0xf].y + unaff_EBX + 3),
                      (char *)(vgui::TextEntry::ScrollLeftForResize + unaff_EBX),0x170,0xc,0,4,
                      *(SendVarProxyFn_conflict *)(&DAT_00ae6a03 + unaff_EBX),0x80);
      SendPropInt((SendProp_conflict *)((int)&rgv3tStuckTable[0x16].y + unaff_EBX + 3),
                  (char *)(unaff_EBX + 0x94bb33),0x17c,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)((int)&rgv3tStuckTable[0x1d].y + unaff_EBX + 3),
                        (char *)(unaff_EBX + 0x94bb46),0x180,
                        *(SendTable **)(&DAT_00ae740f + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00ae693b + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)((int)&rgv3tStuckTable[0x24].y + unaff_EBX + 3),
                    (char *)(unaff_EBX + 0x94bb5a),400,4,0,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00ae6993 + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)((int)&rgv3tStuckTable[0x2b].y + unaff_EBX + 3),
                    (char *)(unaff_EBX + 0x94bb6f),0x18c,4,0,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00ae6993 + unaff_EBX),0x80);
      SendPropInt((SendProp_conflict *)((int)&rgv3tStuckTable[0x32].y + unaff_EBX + 3),
                  (char *)(unaff_EBX + 0x94bb81),0x194,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropInt((SendProp_conflict *)
                  ((int)&g_Language.super_ConCommandBase.m_pszHelpString + unaff_EBX + 3),
                  (char *)(unaff_EBX + 0x94bb95),0x198,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropBool((SendProp_conflict *)
                   ((int)&sk_autoaim_mode.super_ConCommandBase.m_pNext + unaff_EBX + 3),
                   (char *)(unaff_EBX + 0x94bba2),0x19c,1);
      SendPropBool((SendProp_conflict *)(unaff_EBX + 0xcf4a7b),&UNK_0094bbb5 + unaff_EBX,0x19d,1);
      SendPropBool((SendProp_conflict *)
                   ((int)&sv_portal_players.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 3
                   ),(char *)(unaff_EBX + 0x94bbc9),0x19f,1);
      SendPropBool((SendProp_conflict *)((int)&old_radius_damage.m_pszDefaultValue + unaff_EBX + 3),
                   (char *)(unaff_EBX + 0x94bbdc),0x1a0,1);
      SendPropBool((SendProp_conflict *)((int)&skill.super_ConCommandBase.m_nFlags + unaff_EBX + 3),
                   (char *)(unaff_EBX + 0x94bbeb),0x1a1,1);
      SendPropBool((SendProp_conflict *)
                   (&gamestats_file_output_directory.super_ConCommandBase.field_0xb + unaff_EBX),
                   (char *)(unaff_EBX + 0x94bbf6),0x1a2,1);
      ___cxa_guard_release(unaff_EBX + 0xcf4047);
      ___cxa_atexit(unaff_EBX + 0x8d5eff,0,*(undefined4 *)(&DAT_00ae689f + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)((int)&PTR_SetShowDragHelper_00c6a3b4 + unaff_EBX + 3),
             (SendProp_conflict *)
             ((int)&ai_shot_bias_max.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX + 3),0x23
             ,*(char **)(&DAT_00b71aab + unaff_EBX));
  return 1;
}


/* __static_initialization_and_destruction_0 at 000c6240 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  int iVar1;
  datamap_t *pdVar2;
  int unaff_EBX;
  longlong lVar3;
  
  lVar3 = ___i686_get_pc_thunk_bx();
  if (lVar3 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3f0f) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3f10) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3f11) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3f12) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3f13) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3f17) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3f1b) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3f1f) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3f23) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3f27) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3f2b) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3f2f) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3f33) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3f37) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3f3b) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3f3f) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3f43) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3f4a) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3f47) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3f48) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3f49) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3f4b) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3f52) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3f4f) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3f50) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3f51) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3f53) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3f5a) = 1;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3f57) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3f58) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3f59) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3f5b) = 1;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3f62) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3f5f) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3f60) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3f61) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3f63) = 2;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3f6a) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3f67) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3f68) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3f69) = 0;
  *(undefined **)((int)s_pRandomFloats + unaff_EBX + 0x3f6b) = &UNK_00aeb0d7 + unaff_EBX;
  SendTable::SendTable((SendTable *)((int)&PTR_NavigateLeft_00c69454 + unaff_EBX + 3));
  ___cxa_atexit(unaff_EBX + 0x8d4fdf,0,*(undefined4 *)(&DAT_00ae593f + unaff_EBX));
  iVar1 = ServerClassInit<DT_PortalLocal::ignored>((ignored *)0x0);
  *(int *)((int)&PTR_OnNavigateTo_00c69468 + unaff_EBX + 3) = iVar1;
  pdVar2 = DataMapInit<CPortalPlayerLocalData>((CPortalPlayerLocalData *)0x0);
  *(datamap_t **)((int)&PTR_OnNavigateFrom_00c6946c + unaff_EBX + 3) = pdVar2;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3f6f) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3f73) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3f77) = 0x3f800000;
  return;
}


/* __tcf_2 at 0099b230 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x394470),in_stack_00000008);
  return;
}


/* __tcf_0 at 0099b1f0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  undefined4 *puVar1;
  
  ___i686_get_pc_thunk_bx();
  puVar1 = (undefined4 *)(unaff_EBX + 0x41ecbe);
  do {
    (**(code **)*puVar1)(puVar1);
    puVar1 = puVar1 + -0x15;
  } while (puVar1 != (undefined4 *)(unaff_EBX + 0x41e0ee));
  return;
}


/* DataMapInit<CPortalPlayerLocalData> at 000c5210 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

datamap_t * DataMapInit<CPortalPlayerLocalData>(CPortalPlayerLocalData *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&ai_shot_bias_max.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 3) ==
      '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf40ef);
    if (iVar1 != 0) {
      *(int *)((int)&ai_shot_bias_max.m_fnChangeCallbacks.m_pElements + unaff_EBX + 3) =
           unaff_EBX + 0x94ba25;
      *(undefined4 *)(&DAT_00cf40ff + unaff_EBX) = 0;
      *(undefined4 *)
       ((int)&ai_debug_shoot_positions.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX + 3) =
           0;
      *(undefined4 *)((int)&ai_debug_shoot_positions.super_ConCommandBase.m_pNext + unaff_EBX + 3) =
           0;
      *(undefined4 *)(&ai_debug_shoot_positions.super_ConCommandBase.field_0xb + unaff_EBX) = 0;
      *(undefined4 *)((int)&ai_debug_shoot_positions.super_ConCommandBase.m_pszName + unaff_EBX + 3)
           = 0;
      *(undefined4 *)(&DAT_00cf40fb + unaff_EBX) = 0x16;
      ___cxa_guard_release(unaff_EBX + 0xcf40ef);
      ___cxa_atexit(unaff_EBX + 0x8d602f,0,*(undefined4 *)(&DAT_00ae696f + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_GetTracerType_00c074a8 + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&PTR_UpdateTransmitState_00c074a0 + unaff_EBX + 3) = 0x23;
  *(int *)((int)&PTR_ShouldTransmit_00c0749c + unaff_EBX + 3) = unaff_EBX + 0xc074ff;
  return (datamap_t *)((int)&PTR_ShouldTransmit_00c0749c + unaff_EBX + 3);
}


/* CPortalPlayerLocalData::GetBaseMap at 006d51e0 */

datamap_t * CPortalPlayerLocalData::GetBaseMap(void)

{
  return (datamap_t *)0x0;
}


/* __tcf_1 at 0099b250 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41e0ce)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41e0c2) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41e0ce));
  }
  *(undefined4 *)(unaff_EBX + 0x41e0ce) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41e0ca)) {
    if (*(int *)(unaff_EBX + 0x41e0c2) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x21093a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x21093a),*(int *)(unaff_EBX + 0x41e0c2));
      *(undefined4 *)(unaff_EBX + 0x41e0c2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e0c6) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41e0c2);
  *(int *)(unaff_EBX + 0x41e0d2) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41e0ca)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x21093a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x21093a),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41e0c2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e0c6) = 0;
  }
  return;
}


/* CPortalPlayerLocalData::CPortalPlayerLocalData at 006d51f0 */

/* WARNING: Restarted to delay deadcode elimination for space: stack */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void CPortalPlayerLocalData(CPortalPlayerLocalData * this) */

void __thiscall CPortalPlayerLocalData::CPortalPlayerLocalData(CPortalPlayerLocalData *this)

{
  vec_t *pvVar1;
  char cVar2;
  float *pfVar3;
  undefined4 *puVar4;
  CNetworkVarBase<float,CountdownTimer::NetworkVar_m_timestamp> *pCVar5;
  NetworkVar_m_PaintPowerHudInfoNormal *pNVar6;
  NetworkVar_m_PaintedPowerTimer *pNVar7;
  NetworkVar_m_PaintPowerHudInfoPosition *pNVar8;
  int iVar9;
  Vector *pVVar10;
  int unaff_EBX;
  NetworkVar_m_PaintPowerHudInfoPosition *pNVar11;
  NetworkVar_m_PaintPowerHudInfoPosition *pNVar12;
  NetworkVar_m_PaintPowerHudInfoNormal *pNVar13;
  NetworkVar_m_PaintPowerHudInfoNormal *pNVar14;
  Vector *pVVar15;
  Vector *pVVar16;
  float *pfVar17;
  float *pfVar18;
  bool bVar19;
  float fVar20;
  
  ___i686_get_pc_thunk_bx();
  this->_vptr_CPortalPlayerLocalData = (_func_int_varargs **)(&UNK_00561b6f + unaff_EBX);
  (this->m_PaintedPowerTimer).super_CountdownTimer._vptr_CountdownTimer =
       (_func_int_varargs **)(unaff_EBX + 0x53b027);
  (this->m_PaintedPowerTimer).super_CountdownTimer.m_duration.m_Value = 0.0;
  pNVar7 = &this->m_PaintedPowerTimer;
  (**(code **)(unaff_EBX + 0x53b02b))
            (pNVar7,&(this->m_PaintedPowerTimer).super_CountdownTimer.m_duration);
  (this->m_PaintedPowerTimer).super_CountdownTimer.m_timestamp.m_Value = -1.0;
  pCVar5 = &(this->m_PaintedPowerTimer).super_CountdownTimer.m_timestamp;
  (*(this->m_PaintedPowerTimer).super_CountdownTimer._vptr_CountdownTimer[1])(pNVar7,pCVar5);
  (this->m_PaintedPowerTimer).super_CountdownTimer._vptr_CountdownTimer =
       (_func_int_varargs **)(&UNK_00561b87 + unaff_EBX);
  if ((this->m_flAirControlSupressionTime).m_Value != 0.0) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_flAirControlSupressionTime);
    (this->m_flAirControlSupressionTime).m_Value = 0.0;
  }
                    /* Unresolved local var: int i@[???] */
  (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_nLocatorEntityIndices);
  (this->m_nLocatorEntityIndices).m_Value[0] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 1);
  (this->m_nLocatorEntityIndices).m_Value[1] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 2);
  (this->m_nLocatorEntityIndices).m_Value[2] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 3);
  (this->m_nLocatorEntityIndices).m_Value[3] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 4);
  (this->m_nLocatorEntityIndices).m_Value[4] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 5);
  (this->m_nLocatorEntityIndices).m_Value[5] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 6);
  (this->m_nLocatorEntityIndices).m_Value[6] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 7);
  (this->m_nLocatorEntityIndices).m_Value[7] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 8);
  (this->m_nLocatorEntityIndices).m_Value[8] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 9);
  (this->m_nLocatorEntityIndices).m_Value[9] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 10);
  (this->m_nLocatorEntityIndices).m_Value[10] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 0xb);
  (this->m_nLocatorEntityIndices).m_Value[0xb] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 0xc);
  (this->m_nLocatorEntityIndices).m_Value[0xc] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 0xd);
  (this->m_nLocatorEntityIndices).m_Value[0xd] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 0xe);
  (this->m_nLocatorEntityIndices).m_Value[0xe] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 0xf);
  (this->m_nLocatorEntityIndices).m_Value[0xf] = -1;
  if ((this->m_bPlacingPhoto).m_Value != false) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_bPlacingPhoto);
    (this->m_bPlacingPhoto).m_Value = false;
  }
  pNVar8 = &this->m_PaintPowerHudInfoPosition;
  bVar19 = pNVar8 == (NetworkVar_m_PaintPowerHudInfoPosition *)0x0;
  pfVar3 = *(float **)(unaff_EBX + 0x4d69af);
  iVar9 = 0xc;
  pNVar11 = pNVar8;
  pfVar17 = pfVar3;
  do {
    pNVar12 = pNVar11;
    pfVar18 = pfVar17;
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    pfVar18 = (float *)((int)pfVar17 + 1);
    pNVar12 = (NetworkVar_m_PaintPowerHudInfoPosition *)((int)&pNVar11->m_Value[0].x + 1);
    bVar19 = *(char *)&pNVar11->m_Value[0].x == *(char *)pfVar17;
    pNVar11 = pNVar12;
    pfVar17 = pfVar18;
  } while (bVar19);
  iVar9 = 0;
  if (!bVar19) {
    iVar9 = (uint)*(byte *)((int)(pNVar12 + -1) + 0x2f) - (uint)*(byte *)((int)pfVar18 + -1);
  }
  if (iVar9 != 0) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,pNVar8);
    (this->m_PaintPowerHudInfoPosition).m_Value[0].x = *pfVar3;
    (this->m_PaintPowerHudInfoPosition).m_Value[0].y = pfVar3[1];
    (this->m_PaintPowerHudInfoPosition).m_Value[0].z = pfVar3[2];
  }
  pNVar6 = &this->m_PaintPowerHudInfoNormal;
  bVar19 = pNVar6 == (NetworkVar_m_PaintPowerHudInfoNormal *)0x0;
  iVar9 = 0xc;
  pNVar13 = pNVar6;
  pfVar17 = pfVar3;
  do {
    pNVar14 = pNVar13;
    pfVar18 = pfVar17;
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    pfVar18 = (float *)((int)pfVar17 + 1);
    pNVar14 = (NetworkVar_m_PaintPowerHudInfoNormal *)((int)&pNVar13->m_Value[0].x + 1);
    bVar19 = *(char *)&pNVar13->m_Value[0].x == *(char *)pfVar17;
    pNVar13 = pNVar14;
    pfVar17 = pfVar18;
  } while (bVar19);
  iVar9 = 0;
  if (!bVar19) {
    iVar9 = (uint)*(byte *)((int)&((NetworkVar_m_PaintPowerHudInfoPosition *)(pNVar14 + -1))->
                                  m_Value[3].z + 3) - (uint)*(byte *)((int)pfVar18 + -1);
  }
  if (iVar9 != 0) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,pNVar6);
    (this->m_PaintPowerHudInfoNormal).m_Value[0].x = *pfVar3;
    (this->m_PaintPowerHudInfoNormal).m_Value[0].y = pfVar3[1];
    (this->m_PaintPowerHudInfoNormal).m_Value[0].z = pfVar3[2];
  }
  if ((this->m_PaintPowerHudInfoValidity).m_Value[0] != false) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_PaintPowerHudInfoValidity);
    (this->m_PaintPowerHudInfoValidity).m_Value[0] = false;
  }
  pVVar10 = (this->m_PaintPowerHudInfoPosition).m_Value + 1;
  bVar19 = pVVar10 == (Vector *)0x0;
  iVar9 = 0xc;
  pVVar15 = pVVar10;
  pfVar17 = pfVar3;
  do {
    pVVar16 = pVVar15;
    pfVar18 = pfVar17;
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    pfVar18 = (float *)((int)pfVar17 + 1);
    pVVar16 = (Vector *)((int)&pVVar15->x + 1);
    bVar19 = *(char *)&pVVar15->x == *(char *)pfVar17;
    pVVar15 = pVVar16;
    pfVar17 = pfVar18;
  } while (bVar19);
  iVar9 = 0;
  if (!bVar19) {
    iVar9 = (uint)(byte)*(undefined1 *)
                         ((int)&((NetworkVar_m_PaintPowerHudInfoPosition *)(pVVar16 + -1))->m_Value
                                [0].z + 3) - (uint)*(byte *)((int)pfVar18 + -1);
  }
  if (iVar9 != 0) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,pVVar10);
    (this->m_PaintPowerHudInfoPosition).m_Value[1].x = *pfVar3;
    (this->m_PaintPowerHudInfoPosition).m_Value[1].y = pfVar3[1];
    (this->m_PaintPowerHudInfoPosition).m_Value[1].z = pfVar3[2];
  }
  pVVar10 = (this->m_PaintPowerHudInfoNormal).m_Value + 1;
  bVar19 = pVVar10 == (Vector *)0x0;
  iVar9 = 0xc;
  pVVar15 = pVVar10;
  pfVar17 = pfVar3;
  do {
    pVVar16 = pVVar15;
    pfVar18 = pfVar17;
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    pfVar18 = (float *)((int)pfVar17 + 1);
    pVVar16 = (Vector *)((int)&pVVar15->x + 1);
    bVar19 = *(char *)&pVVar15->x == *(char *)pfVar17;
    pVVar15 = pVVar16;
    pfVar17 = pfVar18;
  } while (bVar19);
  iVar9 = 0;
  if (!bVar19) {
    iVar9 = (uint)(byte)*(undefined1 *)
                         ((int)&((NetworkVar_m_PaintPowerHudInfoNormal *)(pVVar16 + -1))->m_Value[0]
                                .z + 3) - (uint)*(byte *)((int)pfVar18 + -1);
  }
  if (iVar9 != 0) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,pVVar10);
    (this->m_PaintPowerHudInfoNormal).m_Value[1].x = *pfVar3;
    (this->m_PaintPowerHudInfoNormal).m_Value[1].y = pfVar3[1];
    (this->m_PaintPowerHudInfoNormal).m_Value[1].z = pfVar3[2];
  }
  if ((this->m_PaintPowerHudInfoValidity).m_Value[1] != false) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_PaintPowerHudInfoValidity).m_Value + 1);
    (this->m_PaintPowerHudInfoValidity).m_Value[1] = false;
  }
  pVVar10 = (this->m_PaintPowerHudInfoPosition).m_Value + 2;
  bVar19 = pVVar10 == (Vector *)0x0;
  iVar9 = 0xc;
  pVVar15 = pVVar10;
  pfVar17 = pfVar3;
  do {
    pVVar16 = pVVar15;
    pfVar18 = pfVar17;
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    pfVar18 = (float *)((int)pfVar17 + 1);
    pVVar16 = (Vector *)((int)&pVVar15->x + 1);
    bVar19 = *(char *)&pVVar15->x == *(char *)pfVar17;
    pVVar15 = pVVar16;
    pfVar17 = pfVar18;
  } while (bVar19);
  iVar9 = 0;
  if (!bVar19) {
    iVar9 = (uint)*(byte *)((int)&pVVar16[-1].z + 3) - (uint)*(byte *)((int)pfVar18 + -1);
  }
  if (iVar9 != 0) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,pVVar10);
    (this->m_PaintPowerHudInfoPosition).m_Value[2].x = *pfVar3;
    (this->m_PaintPowerHudInfoPosition).m_Value[2].y = pfVar3[1];
    (this->m_PaintPowerHudInfoPosition).m_Value[2].z = pfVar3[2];
  }
  pVVar10 = (this->m_PaintPowerHudInfoNormal).m_Value + 2;
  bVar19 = pVVar10 == (Vector *)0x0;
  iVar9 = 0xc;
  pVVar15 = pVVar10;
  pfVar17 = pfVar3;
  do {
    pVVar16 = pVVar15;
    pfVar18 = pfVar17;
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    pfVar18 = (float *)((int)pfVar17 + 1);
    pVVar16 = (Vector *)((int)&pVVar15->x + 1);
    bVar19 = *(char *)&pVVar15->x == *(char *)pfVar17;
    pVVar15 = pVVar16;
    pfVar17 = pfVar18;
  } while (bVar19);
  iVar9 = 0;
  if (!bVar19) {
    iVar9 = (uint)*(byte *)((int)&pVVar16[-1].z + 3) - (uint)*(byte *)((int)pfVar18 + -1);
  }
  if (iVar9 != 0) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,pVVar10);
    (this->m_PaintPowerHudInfoNormal).m_Value[2].x = *pfVar3;
    (this->m_PaintPowerHudInfoNormal).m_Value[2].y = pfVar3[1];
    (this->m_PaintPowerHudInfoNormal).m_Value[2].z = pfVar3[2];
  }
  if ((this->m_PaintPowerHudInfoValidity).m_Value[2] != false) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_PaintPowerHudInfoValidity).m_Value + 2);
    (this->m_PaintPowerHudInfoValidity).m_Value[2] = false;
  }
  pVVar10 = (this->m_PaintPowerHudInfoPosition).m_Value + 3;
  bVar19 = pVVar10 == (Vector *)0x0;
  iVar9 = 0xc;
  pVVar15 = pVVar10;
  pfVar17 = pfVar3;
  do {
    pVVar16 = pVVar15;
    pfVar18 = pfVar17;
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    pfVar18 = (float *)((int)pfVar17 + 1);
    pVVar16 = (Vector *)((int)&pVVar15->x + 1);
    bVar19 = *(char *)&pVVar15->x == *(char *)pfVar17;
    pVVar15 = pVVar16;
    pfVar17 = pfVar18;
  } while (bVar19);
  iVar9 = 0;
  if (!bVar19) {
    iVar9 = (uint)*(byte *)((int)&pVVar16[-1].z + 3) - (uint)*(byte *)((int)pfVar18 + -1);
  }
  if (iVar9 != 0) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,pVVar10);
    (this->m_PaintPowerHudInfoPosition).m_Value[3].x = *pfVar3;
    (this->m_PaintPowerHudInfoPosition).m_Value[3].y = pfVar3[1];
    (this->m_PaintPowerHudInfoPosition).m_Value[3].z = pfVar3[2];
  }
  pVVar10 = (this->m_PaintPowerHudInfoNormal).m_Value + 3;
  bVar19 = pVVar10 == (Vector *)0x0;
  iVar9 = 0xc;
  pVVar15 = pVVar10;
  pfVar17 = pfVar3;
  do {
    pVVar16 = pVVar15;
    pfVar18 = pfVar17;
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    pfVar18 = (float *)((int)pfVar17 + 1);
    pVVar16 = (Vector *)((int)&pVVar15->x + 1);
    bVar19 = *(char *)&pVVar15->x == *(char *)pfVar17;
    pVVar15 = pVVar16;
    pfVar17 = pfVar18;
  } while (bVar19);
  iVar9 = 0;
  if (!bVar19) {
    iVar9 = (uint)*(byte *)((int)&pVVar16[-1].z + 3) - (uint)*(byte *)((int)pfVar18 + -1);
  }
  if (iVar9 != 0) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,pVVar10);
    (this->m_PaintPowerHudInfoNormal).m_Value[3].x = *pfVar3;
    (this->m_PaintPowerHudInfoNormal).m_Value[3].y = pfVar3[1];
    (this->m_PaintPowerHudInfoNormal).m_Value[3].z = pfVar3[2];
  }
  if ((this->m_PaintPowerHudInfoValidity).m_Value[3] != false) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_PaintPowerHudInfoValidity).m_Value + 3);
    (this->m_PaintPowerHudInfoValidity).m_Value[3] = false;
  }
  fVar20 = *(float *)(unaff_EBX + 0x6e40ff);
  if ((((fVar20 != (this->m_StickNormal).
                   super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StickNormal>.
                   m_Value.x) ||
       (NAN(fVar20) ||
        NAN((this->m_StickNormal).
            super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StickNormal>.m_Value.x
           ))) ||
      (pvVar1 = &(this->m_StickNormal).
                 super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StickNormal>.
                 m_Value.y, *(float *)(CLinkedPortalDoor::GetBaseMap + unaff_EBX + 3) != *pvVar1))
     || ((NAN(*(float *)(CLinkedPortalDoor::GetBaseMap + unaff_EBX + 3)) || NAN(*pvVar1) ||
         (*(float *)(unaff_EBX + 0x6e4107) !=
          (this->m_StickNormal).
          super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StickNormal>.m_Value.z))
        )) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_StickNormal);
    (this->m_StickNormal).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StickNormal>.m_Value.x =
         *(vec_t *)(unaff_EBX + 0x6e40ff);
    (this->m_StickNormal).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StickNormal>.m_Value.y =
         *(vec_t *)(CLinkedPortalDoor::GetBaseMap + unaff_EBX + 3);
    (this->m_StickNormal).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StickNormal>.m_Value.z =
         *(vec_t *)(unaff_EBX + 0x6e4107);
    fVar20 = *(float *)(unaff_EBX + 0x6e40ff);
  }
  if (((fVar20 != (this->m_OldStickNormal).
                  super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_OldStickNormal>.
                  m_Value.x) ||
      (NAN(fVar20) ||
       NAN((this->m_OldStickNormal).
           super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_OldStickNormal>.m_Value
           .x))) ||
     ((pvVar1 = &(this->m_OldStickNormal).
                 super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_OldStickNormal>.
                 m_Value.y, *(float *)(CLinkedPortalDoor::GetBaseMap + unaff_EBX + 3) != *pvVar1 ||
      ((NAN(*(float *)(CLinkedPortalDoor::GetBaseMap + unaff_EBX + 3)) || NAN(*pvVar1) ||
       (*(float *)(unaff_EBX + 0x6e4107) !=
        (this->m_OldStickNormal).
        super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_OldStickNormal>.m_Value.z)
       ))))) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_OldStickNormal);
    (this->m_OldStickNormal).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_OldStickNormal>.m_Value.x =
         *(vec_t *)(unaff_EBX + 0x6e40ff);
    (this->m_OldStickNormal).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_OldStickNormal>.m_Value.y =
         *(vec_t *)(CLinkedPortalDoor::GetBaseMap + unaff_EBX + 3);
    (this->m_OldStickNormal).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_OldStickNormal>.m_Value.z =
         *(vec_t *)(unaff_EBX + 0x6e4107);
  }
  if ((((*pfVar3 !=
         (this->m_vPreUpdateVelocity).
         super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vPreUpdateVelocity>.
         m_Value.x) ||
       (NAN(*pfVar3) ||
        NAN((this->m_vPreUpdateVelocity).
            super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vPreUpdateVelocity>.
            m_Value.x))) ||
      (pvVar1 = &(this->m_vPreUpdateVelocity).
                 super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vPreUpdateVelocity>
                 .m_Value.y, pfVar3[1] != *pvVar1)) ||
     ((NAN(pfVar3[1]) || NAN(*pvVar1) ||
      (pfVar3[2] !=
       (this->m_vPreUpdateVelocity).
       super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vPreUpdateVelocity>.m_Value
       .z)))) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_vPreUpdateVelocity);
    (this->m_vPreUpdateVelocity).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vPreUpdateVelocity>.m_Value.x
         = *pfVar3;
    (this->m_vPreUpdateVelocity).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vPreUpdateVelocity>.m_Value.y
         = pfVar3[1];
    (this->m_vPreUpdateVelocity).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vPreUpdateVelocity>.m_Value.z
         = pfVar3[2];
  }
  if (((*(float *)(unaff_EBX + 0x6e40ff) !=
        (this->m_Up).super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_Up>.m_Value.x
       ) || (NAN(*(float *)(unaff_EBX + 0x6e40ff)) ||
             NAN((this->m_Up).super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_Up>.
                 m_Value.x))) ||
     ((pvVar1 = &(this->m_Up).super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_Up>.
                 m_Value.y, *(float *)(CLinkedPortalDoor::GetBaseMap + unaff_EBX + 3) != *pvVar1 ||
      ((NAN(*(float *)(CLinkedPortalDoor::GetBaseMap + unaff_EBX + 3)) || NAN(*pvVar1) ||
       (*(float *)(unaff_EBX + 0x6e4107) !=
        (this->m_Up).super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_Up>.m_Value.z
       )))))) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_Up);
    (this->m_Up).super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_Up>.m_Value.x =
         *(vec_t *)(unaff_EBX + 0x6e40ff);
    (this->m_Up).super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_Up>.m_Value.y =
         *(vec_t *)(CLinkedPortalDoor::GetBaseMap + unaff_EBX + 3);
    (this->m_Up).super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_Up>.m_Value.z =
         *(vec_t *)(unaff_EBX + 0x6e4107);
  }
  if (((((this->m_vStickRotationAxis).
         super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vStickRotationAxis>.
         m_Value.x != 0.0) ||
       (pvVar1 = &(this->m_vStickRotationAxis).
                  super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vStickRotationAxis>
                  .m_Value.y, *pvVar1 != 0.0)) || (NAN(*pvVar1))) ||
     ((this->m_vStickRotationAxis).
      super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vStickRotationAxis>.m_Value.
      z != 0.0)) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_vStickRotationAxis);
    (this->m_vStickRotationAxis).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vStickRotationAxis>.m_Value.x
         = 0.0;
    (this->m_vStickRotationAxis).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vStickRotationAxis>.m_Value.y
         = 0.0;
    (this->m_vStickRotationAxis).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vStickRotationAxis>.m_Value.z
         = 0.0;
  }
  puVar4 = *(undefined4 **)(unaff_EBX + 0x4d69db);
  iVar9 = (**(code **)(*(int *)*puVar4 + 0x7c))((int *)*puVar4);
  if (((*(float *)(iVar9 + 0xc) !=
        (this->m_StandHullMin).
        super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMin>.m_Value.x)
      || (NAN(*(float *)(iVar9 + 0xc)) ||
          NAN((this->m_StandHullMin).
              super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMin>.
              m_Value.x))) ||
     ((pvVar1 = &(this->m_StandHullMin).
                 super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMin>.
                 m_Value.y, *(float *)(iVar9 + 0x10) != *pvVar1 ||
      ((NAN(*(float *)(iVar9 + 0x10)) || NAN(*pvVar1) ||
       (*(float *)(iVar9 + 0x14) !=
        (this->m_StandHullMin).
        super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMin>.m_Value.z)))
      ))) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_StandHullMin);
    (this->m_StandHullMin).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMin>.m_Value.x =
         *(vec_t *)(iVar9 + 0xc);
    (this->m_StandHullMin).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMin>.m_Value.y =
         *(vec_t *)(iVar9 + 0x10);
    (this->m_StandHullMin).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMin>.m_Value.z =
         *(vec_t *)(iVar9 + 0x14);
  }
  iVar9 = (**(code **)(*(int *)*puVar4 + 0x7c))((int *)*puVar4);
  if ((((*(float *)(iVar9 + 0x18) !=
         (this->m_StandHullMax).
         super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMax>.m_Value.x)
       || (NAN(*(float *)(iVar9 + 0x18)) ||
           NAN((this->m_StandHullMax).
               super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMax>.
               m_Value.x))) ||
      (pvVar1 = &(this->m_StandHullMax).
                 super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMax>.
                 m_Value.y, *(float *)(iVar9 + 0x1c) != *pvVar1)) ||
     ((NAN(*(float *)(iVar9 + 0x1c)) || NAN(*pvVar1) ||
      (*(float *)(iVar9 + 0x20) !=
       (this->m_StandHullMax).
       super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMax>.m_Value.z))))
  {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_StandHullMax);
    (this->m_StandHullMax).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMax>.m_Value.x =
         *(vec_t *)(iVar9 + 0x18);
    (this->m_StandHullMax).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMax>.m_Value.y =
         *(vec_t *)(iVar9 + 0x1c);
    (this->m_StandHullMax).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMax>.m_Value.z =
         *(vec_t *)(iVar9 + 0x20);
  }
  iVar9 = (**(code **)(*(int *)*puVar4 + 0x7c))((int *)*puVar4);
  if (((*(float *)(iVar9 + 0x24) !=
        (this->m_DuckHullMin).
        super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMin>.m_Value.x) ||
      (NAN(*(float *)(iVar9 + 0x24)) ||
       NAN((this->m_DuckHullMin).
           super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMin>.m_Value.x)
      )) || ((pvVar1 = &(this->m_DuckHullMin).
                        super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMin>
                        .m_Value.y, *(float *)(iVar9 + 0x28) != *pvVar1 ||
             ((NAN(*(float *)(iVar9 + 0x28)) || NAN(*pvVar1) ||
              (*(float *)(iVar9 + 0x2c) !=
               (this->m_DuckHullMin).
               super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMin>.
               m_Value.z)))))) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_DuckHullMin);
    (this->m_DuckHullMin).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMin>.m_Value.x =
         *(vec_t *)(iVar9 + 0x24);
    (this->m_DuckHullMin).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMin>.m_Value.y =
         *(vec_t *)(iVar9 + 0x28);
    (this->m_DuckHullMin).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMin>.m_Value.z =
         *(vec_t *)(iVar9 + 0x2c);
  }
  iVar9 = (**(code **)(*(int *)*puVar4 + 0x7c))((int *)*puVar4);
  if ((((*(float *)(iVar9 + 0x30) !=
         (this->m_DuckHullMax).
         super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMax>.m_Value.x)
       || (NAN(*(float *)(iVar9 + 0x30)) ||
           NAN((this->m_DuckHullMax).
               super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMax>.
               m_Value.x))) ||
      (pvVar1 = &(this->m_DuckHullMax).
                 super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMax>.
                 m_Value.y, *(float *)(iVar9 + 0x34) != *pvVar1)) ||
     ((NAN(*(float *)(iVar9 + 0x34)) || NAN(*pvVar1) ||
      (*(float *)(iVar9 + 0x38) !=
       (this->m_DuckHullMax).
       super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMax>.m_Value.z))))
  {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_DuckHullMax);
    (this->m_DuckHullMax).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMax>.m_Value.x =
         *(vec_t *)(iVar9 + 0x30);
    (this->m_DuckHullMax).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMax>.m_Value.y =
         *(vec_t *)(iVar9 + 0x34);
    (this->m_DuckHullMax).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMax>.m_Value.z =
         *(vec_t *)(iVar9 + 0x38);
  }
  iVar9 = (**(code **)(*(int *)*puVar4 + 0x7c))((int *)*puVar4);
  if (((*(float *)(iVar9 + 0xc) !=
        (this->m_CachedStandHullMinAttempt).
        super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMinAttempt>
        .m_Value.x) ||
      (NAN(*(float *)(iVar9 + 0xc)) ||
       NAN((this->m_CachedStandHullMinAttempt).
           super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMinAttempt>
           .m_Value.x))) ||
     ((pvVar1 = &(this->m_CachedStandHullMinAttempt).
                 super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMinAttempt>
                 .m_Value.y, *(float *)(iVar9 + 0x10) != *pvVar1 ||
      ((NAN(*(float *)(iVar9 + 0x10)) || NAN(*pvVar1) ||
       (*(float *)(iVar9 + 0x14) !=
        (this->m_CachedStandHullMinAttempt).
        super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMinAttempt>
        .m_Value.z)))))) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_CachedStandHullMinAttempt);
    (this->m_CachedStandHullMinAttempt).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMinAttempt>.
    m_Value.x = *(vec_t *)(iVar9 + 0xc);
    (this->m_CachedStandHullMinAttempt).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMinAttempt>.
    m_Value.y = *(vec_t *)(iVar9 + 0x10);
    (this->m_CachedStandHullMinAttempt).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMinAttempt>.
    m_Value.z = *(vec_t *)(iVar9 + 0x14);
  }
  iVar9 = (**(code **)(*(int *)*puVar4 + 0x7c))((int *)*puVar4);
  if ((((*(float *)(iVar9 + 0x18) !=
         (this->m_CachedStandHullMaxAttempt).
         super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMaxAttempt>
         .m_Value.x) ||
       (NAN(*(float *)(iVar9 + 0x18)) ||
        NAN((this->m_CachedStandHullMaxAttempt).
            super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMaxAttempt>
            .m_Value.x))) ||
      (pvVar1 = &(this->m_CachedStandHullMaxAttempt).
                 super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMaxAttempt>
                 .m_Value.y, *(float *)(iVar9 + 0x1c) != *pvVar1)) ||
     ((NAN(*(float *)(iVar9 + 0x1c)) || NAN(*pvVar1) ||
      (*(float *)(iVar9 + 0x20) !=
       (this->m_CachedStandHullMaxAttempt).
       super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMaxAttempt>.
       m_Value.z)))) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_CachedStandHullMaxAttempt);
    (this->m_CachedStandHullMaxAttempt).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMaxAttempt>.
    m_Value.x = *(vec_t *)(iVar9 + 0x18);
    (this->m_CachedStandHullMaxAttempt).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMaxAttempt>.
    m_Value.y = *(vec_t *)(iVar9 + 0x1c);
    (this->m_CachedStandHullMaxAttempt).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMaxAttempt>.
    m_Value.z = *(vec_t *)(iVar9 + 0x20);
  }
  iVar9 = (**(code **)(*(int *)*puVar4 + 0x7c))((int *)*puVar4);
  if (((*(float *)(iVar9 + 0x24) !=
        (this->m_CachedDuckHullMinAttempt).
        super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMinAttempt>.
        m_Value.x) ||
      (NAN(*(float *)(iVar9 + 0x24)) ||
       NAN((this->m_CachedDuckHullMinAttempt).
           super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMinAttempt>
           .m_Value.x))) ||
     ((pvVar1 = &(this->m_CachedDuckHullMinAttempt).
                 super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMinAttempt>
                 .m_Value.y, *(float *)(iVar9 + 0x28) != *pvVar1 ||
      ((NAN(*(float *)(iVar9 + 0x28)) || NAN(*pvVar1) ||
       (*(float *)(iVar9 + 0x2c) !=
        (this->m_CachedDuckHullMinAttempt).
        super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMinAttempt>.
        m_Value.z)))))) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_CachedDuckHullMinAttempt);
    (this->m_CachedDuckHullMinAttempt).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMinAttempt>.
    m_Value.x = *(vec_t *)(iVar9 + 0x24);
    (this->m_CachedDuckHullMinAttempt).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMinAttempt>.
    m_Value.y = *(vec_t *)(iVar9 + 0x28);
    (this->m_CachedDuckHullMinAttempt).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMinAttempt>.
    m_Value.z = *(vec_t *)(iVar9 + 0x2c);
  }
  iVar9 = (**(code **)(*(int *)*puVar4 + 0x7c))((int *)*puVar4);
  if ((((*(float *)(iVar9 + 0x30) !=
         (this->m_CachedDuckHullMaxAttempt).
         super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMaxAttempt>
         .m_Value.x) ||
       (NAN(*(float *)(iVar9 + 0x30)) ||
        NAN((this->m_CachedDuckHullMaxAttempt).
            super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMaxAttempt>
            .m_Value.x))) ||
      (pvVar1 = &(this->m_CachedDuckHullMaxAttempt).
                 super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMaxAttempt>
                 .m_Value.y, *(float *)(iVar9 + 0x34) != *pvVar1)) ||
     ((NAN(*(float *)(iVar9 + 0x34)) || NAN(*pvVar1) ||
      (*(float *)(iVar9 + 0x38) !=
       (this->m_CachedDuckHullMaxAttempt).
       super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMaxAttempt>.
       m_Value.z)))) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_CachedDuckHullMaxAttempt);
    (this->m_CachedDuckHullMaxAttempt).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMaxAttempt>.
    m_Value.x = *(vec_t *)(iVar9 + 0x30);
    (this->m_CachedDuckHullMaxAttempt).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMaxAttempt>.
    m_Value.y = *(vec_t *)(iVar9 + 0x34);
    (this->m_CachedDuckHullMaxAttempt).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMaxAttempt>.
    m_Value.z = *(vec_t *)(iVar9 + 0x38);
    cVar2 = (this->m_bAttemptHullResize).m_Value;
  }
  else {
    cVar2 = (this->m_bAttemptHullResize).m_Value;
  }
  if (cVar2 != '\0') {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_bAttemptHullResize);
    (this->m_bAttemptHullResize).m_Value = false;
  }
  if ((((this->m_vLocalUp).
        super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vLocalUp>.m_Value.x != 0.0
       ) || (pvVar1 = &(this->m_vLocalUp).
                       super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vLocalUp>.
                       m_Value.y, *pvVar1 != 0.0)) ||
     ((NAN(*pvVar1) ||
      (*(float *)(unaff_EBX + 0x35510f) !=
       (this->m_vLocalUp).
       super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vLocalUp>.m_Value.z)))) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_vLocalUp);
    (this->m_vLocalUp).super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vLocalUp>.
    m_Value.x = 0.0;
    (this->m_vLocalUp).super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vLocalUp>.
    m_Value.y = 0.0;
    (this->m_vLocalUp).super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vLocalUp>.
    m_Value.z = 1.0;
  }
  if (((((this->m_vEyeOffset).
         super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeOffset>.m_Value.x !=
         0.0) || (pvVar1 = &(this->m_vEyeOffset).
                            super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeOffset>
                            .m_Value.y, *pvVar1 != 0.0)) || (NAN(*pvVar1))) ||
     ((this->m_vEyeOffset).
      super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeOffset>.m_Value.z != 0.0
     )) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_vEyeOffset);
    (this->m_vEyeOffset).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeOffset>.m_Value.x = 0.0;
    (this->m_vEyeOffset).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeOffset>.m_Value.y = 0.0;
    (this->m_vEyeOffset).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeOffset>.m_Value.z = 0.0;
  }
  if ((((this->m_vEyeUpOffset).
        super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeUpOffset>.m_Value.x !=
        0.0) || (pvVar1 = &(this->m_vEyeUpOffset).
                           super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeUpOffset>
                           .m_Value.y, *pvVar1 != 0.0)) ||
     ((NAN(*pvVar1) ||
      ((this->m_vEyeUpOffset).
       super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeUpOffset>.m_Value.z !=
       0.0)))) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_vEyeUpOffset);
    (this->m_vEyeUpOffset).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeUpOffset>.m_Value.x = 0.0;
    (this->m_vEyeUpOffset).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeUpOffset>.m_Value.y = 0.0;
    (this->m_vEyeUpOffset).
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeUpOffset>.m_Value.z = 0.0;
  }
  if ((((this->m_qQuaternionPunch).
        super_CNetworkVarBase<QAngle,CPortalPlayerLocalData::NetworkVar_m_qQuaternionPunch>.m_Value.
        x != 0.0) ||
      (pvVar1 = &(this->m_qQuaternionPunch).
                 super_CNetworkVarBase<QAngle,CPortalPlayerLocalData::NetworkVar_m_qQuaternionPunch>
                 .m_Value.y, *pvVar1 != 0.0)) ||
     ((NAN(*pvVar1) ||
      ((this->m_qQuaternionPunch).
       super_CNetworkVarBase<QAngle,CPortalPlayerLocalData::NetworkVar_m_qQuaternionPunch>.m_Value.z
       != 0.0)))) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_qQuaternionPunch);
    (this->m_qQuaternionPunch).
    super_CNetworkVarBase<QAngle,CPortalPlayerLocalData::NetworkVar_m_qQuaternionPunch>.m_Value.x =
         0.0;
    (this->m_qQuaternionPunch).
    super_CNetworkVarBase<QAngle,CPortalPlayerLocalData::NetworkVar_m_qQuaternionPunch>.m_Value.y =
         0.0;
    (this->m_qQuaternionPunch).
    super_CNetworkVarBase<QAngle,CPortalPlayerLocalData::NetworkVar_m_qQuaternionPunch>.m_Value.z =
         0.0;
  }
  if ((this->m_PaintedPowerType).m_Value != NO_POWER) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_PaintedPowerType);
    (this->m_PaintedPowerType).m_Value = NO_POWER;
  }
  if (*(float *)(unaff_EBX + 0x3550df) !=
      (this->m_PaintedPowerTimer).super_CountdownTimer.m_timestamp.m_Value) {
    (*(pNVar7->super_CountdownTimer)._vptr_CountdownTimer[1])(pNVar7,pCVar5);
    (this->m_PaintedPowerTimer).super_CountdownTimer.m_timestamp.m_Value = -1.0;
  }
  if (*(float *)(unaff_EBX + 0x35510f) != (this->m_flAirInputScale).m_Value) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_flAirInputScale);
    (this->m_flAirInputScale).m_Value = 1.0;
  }
  fVar20 = *(float *)(*(int *)(*(int *)(unaff_EBX + 0x4d74fb) + 0x1c) + 0x2c);
  if (fVar20 != (this->m_flCurrentStickTime).m_Value) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_flCurrentStickTime);
    (this->m_flCurrentStickTime).m_Value = fVar20;
  }
  if ((this->m_nStickCameraState).m_Value != STICK_CAMERA_UPRIGHT) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_nStickCameraState);
    (this->m_nStickCameraState).m_Value = STICK_CAMERA_UPRIGHT;
  }
  if ((this->m_InAirState).m_Value == ON_GROUND) {
    cVar2 = (this->m_bJumpedThisFrame).m_Value;
  }
  else {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_InAirState);
    (this->m_InAirState).m_Value = ON_GROUND;
    cVar2 = (this->m_bJumpedThisFrame).m_Value;
  }
  if (cVar2 != '\0') {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_bJumpedThisFrame);
    (this->m_bJumpedThisFrame).m_Value = false;
  }
  if ((this->m_bDoneStickInterp).m_Value == false) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_bDoneStickInterp);
    (this->m_bDoneStickInterp).m_Value = true;
  }
  if ((this->m_bDoneCorrectPitch).m_Value == false) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_bDoneCorrectPitch);
    (this->m_bDoneCorrectPitch).m_Value = true;
  }
  if ((this->m_bDuckedInAir).m_Value == false) {
    cVar2 = (this->m_bBounced).m_Value;
  }
  else {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_bDuckedInAir);
    (this->m_bDuckedInAir).m_Value = false;
    cVar2 = (this->m_bBounced).m_Value;
  }
  if (cVar2 != '\0') {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_bBounced);
    (this->m_bBounced).m_Value = false;
  }
  if ((this->m_bInTractorBeam).m_Value != false) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_bInTractorBeam);
    (this->m_bInTractorBeam).m_Value = false;
  }
  return;
}


/* CPortalPlayerLocalData::CPortalPlayerLocalData at 006d6510 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void CPortalPlayerLocalData(CPortalPlayerLocalData * this,
   CPortalPlayerLocalData * this) */

void __thiscall
CPortalPlayerLocalData::CPortalPlayerLocalData
          (CPortalPlayerLocalData *this,CPortalPlayerLocalData *this_1)

{
  CPortalPlayerLocalData(this);
  return;
}


/* _GLOBAL__I__ZN14DT_PortalLocal11g_SendTableE at 000c63f0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN14DT_PortalLocal11g_SendTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

