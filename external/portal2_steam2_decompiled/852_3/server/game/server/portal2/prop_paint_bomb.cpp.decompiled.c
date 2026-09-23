/* DWARF-guided pseudocode for game/server/portal2/prop_paint_bomb.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000cc770 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *pcVar1;
  undefined4 *puVar2;
  datamap_t *pdVar3;
  int iVar4;
  int iVar5;
  IEntityFactoryDictionary *pIVar6;
  int unaff_EBX;
  undefined4 *puVar7;
  undefined4 *puVar8;
  longlong lVar9;
  ConVar *this_1;
  ConVar *in_stack_ffffffc8;
  char *in_stack_ffffffcc;
  char *in_stack_ffffffd0;
  int in_stack_ffffffd4;
  SendTable *this;
  char *in_stack_ffffffd8;
  
  lVar9 = ___i686_get_pc_thunk_bx();
  if (lVar9 != 0xffff00000001) {
    return;
  }
  speechListSounds[4].sceneName[unaff_EBX + 0x6e] = '\0';
  speechListSounds[4].sceneName[unaff_EBX + 0x6f] = '\0';
  speechListSounds[4].sceneName[unaff_EBX + 0x70] = '\0';
  speechListSounds[4].sceneName[unaff_EBX + 0x71] = '\0';
  pcVar1 = speechListSounds[4].sceneName + unaff_EBX + 0x72;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = speechListSounds[4].sceneName + unaff_EBX + 0x76;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = speechListSounds[4].sceneName + unaff_EBX + 0x7a;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = speechListSounds[4].sceneName + unaff_EBX + 0x7e;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  *(undefined4 *)(&DAT_00cee956 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00cee95a + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00cee95e + unaff_EBX) = 0;
  *(undefined4 *)((int)&PrecacheInstancedScene::nMakingReslists + unaff_EBX + 2) = 0;
  *(undefined4 *)(&DAT_00cee966 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00cee96a + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00cee96e + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00cee972 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00cee976 + unaff_EBX) = 0;
  (&DAT_00cee97d)[unaff_EBX] = 0;
  (&DAT_00cee97a)[unaff_EBX] = 0;
  (&DAT_00cee97b)[unaff_EBX] = 0;
  (&DAT_00cee97c)[unaff_EBX] = 0;
  *(undefined4 *)(&DAT_00cee97e + unaff_EBX) = 0;
  (&DAT_00cee985)[unaff_EBX] = 0;
  *(undefined1 *)((int)&g_interactionScriptedTarget + unaff_EBX + 2) = 0;
  *(undefined1 *)((int)&g_interactionScriptedTarget + unaff_EBX + 3) = 0;
  (&DAT_00cee984)[unaff_EBX] = 0;
  *(undefined4 *)(&DAT_00cee986 + unaff_EBX) = 0;
  (&DAT_00cee98d)[unaff_EBX] = 1;
  (&DAT_00cee98a)[unaff_EBX] = 0;
  (&DAT_00cee98b)[unaff_EBX] = 0;
  (&DAT_00cee98c)[unaff_EBX] = 0;
  *(undefined4 *)(&DAT_00cee98e + unaff_EBX) = 1;
  (&DAT_00cee995)[unaff_EBX] = 0;
  (&DAT_00cee992)[unaff_EBX] = 0;
  (&DAT_00cee993)[unaff_EBX] = 0;
  (&DAT_00cee994)[unaff_EBX] = 0;
  *(undefined4 *)(&DAT_00cee996 + unaff_EBX) = 2;
  (&DAT_00cee99d)[unaff_EBX] = 0;
  (&DAT_00cee99a)[unaff_EBX] = 0;
  (&DAT_00cee99b)[unaff_EBX] = 0;
  (&DAT_00cee99c)[unaff_EBX] = 0;
  *(undefined **)(&DAT_00cee99e + unaff_EBX) = &UNK_00ae4baa + unaff_EBX;
  this_1 = (ConVar *)(unaff_EBX + 0x9465ba);
  ConVar::ConVar((ConVar *)((int)&PTR_RequestFocusNext_00c63960 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x9465f1),(char *)(unaff_EBX + 0x8eb940),0x4000,(char *)this_1
                 ,in_stack_ffffffc8,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,
                 in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8d0662,0,*(undefined4 *)(&DAT_00adf412 + unaff_EBX));
  ConVar::ConVar((ConVar *)((int)&PTR_OnMove_00c639c0 + unaff_EBX + 2),
                 (char *)(vgui::ScrollBarSlider::GetRange + unaff_EBX + 2),&UNK_0094660c + unaff_EBX
                 ,0x4000,this_1,(char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x8d0642,0,*(undefined4 *)(&DAT_00adf412 + unaff_EBX));
  ConVar::ConVar((ConVar *)((int)&PTR_GetBgColor_00c63a20 + unaff_EBX + 2),
                 (char *)(vgui::ScrollBarSlider::OnCursorMoved + unaff_EBX + 3),
                 (char *)(unaff_EBX + 0x94662d),0x4000,this_1,(char *)in_stack_ffffffc8,
                 in_stack_ffffffcc,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x8d0622,0,*(undefined4 *)(&DAT_00adf412 + unaff_EBX));
  ConVar::ConVar((ConVar *)((int)&PTR_SetScheme_00c63a80 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x946650),(char *)(unaff_EBX + 0x8e72be),0x4000,this_1,
                 (char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x8d0602,0,*(undefined4 *)(&DAT_00adf412 + unaff_EBX));
  ConVar::ConVar((ConVar *)((int)&PTR_SetTriplePressAllowed_00c63ae0 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x94666a),(char *)(unaff_EBX + 0x8efd16),0x4000,this_1,
                 (char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x8d05e2,0,*(undefined4 *)(&DAT_00adf412 + unaff_EBX));
  ConVar::ConVar((ConVar *)((int)&PTR_GetPanelMap_00c63b40 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x946692),(char *)(unaff_EBX + 0x94668a),0x4000,this_1,
                 (char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x8d05c2,0,*(undefined4 *)(&DAT_00adf412 + unaff_EBX));
  ConVar::ConVar((ConVar *)((int)&PTR_GetDragDropInfo_00c63ba0 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x9466ba),(char *)(unaff_EBX + 0x9466b2),0x4000,this_1,
                 (char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x8d05a2,0,*(undefined4 *)(&DAT_00adf412 + unaff_EBX));
  ConVar::ConVar((ConVar *)((int)&PTR_NavigateFrom_00c63c00 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x9466e2),(char *)(unaff_EBX + 0x8ecc66),0x4000,this_1,
                 (char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x8d0582,0,*(undefined4 *)(&DAT_00adf412 + unaff_EBX));
  ConVar::ConVar((ConVar *)((int)&PTR_InternalKeyFocusTicked_00c63c60 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x94670a),(char *)(unaff_EBX + 0x90bf72),0x4000,this_1,
                 (char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x8d0562,0,*(undefined4 *)(&DAT_00adf412 + unaff_EBX));
  ConVar::ConVar((ConVar *)((int)&PTR_AddImage_00c63cc0 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x94672a),(char *)(unaff_EBX + 0x90bf72),0x4000,this_1,
                 (char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x8d0542,0,*(undefined4 *)(&DAT_00adf412 + unaff_EBX));
  ConVar::ConVar((ConVar *)((int)&PTR_ForceDepressed_00c63d20 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x94674d),(char *)(unaff_EBX + 0x8df4ba),0x4000,this_1,
                 (char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x8d0522,0,*(undefined4 *)(&DAT_00adf412 + unaff_EBX));
  pdVar3 = DataMapInit<CPropPaintBomb>((CPropPaintBomb *)0x0);
  *(datamap_t **)((int)&PTR_GetButtonFgColor_00c63d78 + unaff_EBX + 2) = pdVar3;
  iVar5 = unaff_EBX + 0xcee9a2;
  pcVar1 = &UNK_009465a9 + unaff_EBX;
  *(char **)((int)&CEntityClassList<CSkyCamera>::m_pClassList + unaff_EBX + 2) = pcVar1;
  this = (SendTable *)((int)&PTR_GetButtonBgColor_00c63d7c + unaff_EBX + 2);
  *(SendTable **)((int)&fSentencesInit + unaff_EBX + 2) = this;
  *(undefined4 *)
   ((int)&CUtlVectorDataopsInstantiator<13>::
          GetDataOps<CUtlVector<CHandle<CBasePlayer>,CUtlMemory<CHandle<CBasePlayer>,int>>>(CUtlVector<CHandle<CBasePlayer>,CUtlMemory<CHandle<CBasePlayer>,int>>*)
          ::ops + unaff_EBX + 2) = 0xffff;
  puVar8 = (undefined4 *)**(int **)(&DAT_00adf4b2 + unaff_EBX);
  if (puVar8 == (undefined4 *)0x0) {
    **(int **)(&DAT_00adf4b2 + unaff_EBX) = iVar5;
    *(undefined4 *)
     ((int)&CUtlVectorDataopsInstantiator<13>::
            GetDataOps<CUtlVector<CHandle<CBasePlayer>,CUtlMemory<CHandle<CBasePlayer>,int>>>(CUtlVector<CHandle<CBasePlayer>,CUtlMemory<CHandle<CBasePlayer>,int>>*)
            ::ops + unaff_EBX + 2) = 0;
  }
  else {
    puVar2 = (undefined4 *)puVar8[2];
    iVar4 = _V_stricmp((char *)*puVar8,pcVar1);
    if (iVar4 < 1) {
      while ((puVar7 = puVar2, puVar7 != (undefined4 *)0x0 &&
             (iVar4 = _V_stricmp((char *)*puVar7,pcVar1), iVar4 < 1))) {
        puVar2 = (undefined4 *)puVar7[2];
        puVar8 = puVar7;
      }
      *(undefined4 **)(iVar5 + 8) = puVar7;
      puVar8[2] = iVar5;
    }
    else {
      *(undefined4 *)(iVar5 + 8) = **(undefined4 **)(&DAT_00adf4b2 + unaff_EBX);
      **(int **)(&DAT_00adf4b2 + unaff_EBX) = iVar5;
    }
  }
  SendTable::SendTable(this);
  ___cxa_atexit(&UNK_008d0502 + unaff_EBX,0,*(undefined4 *)(&DAT_00adf412 + unaff_EBX));
  iVar5 = ServerClassInit<DT_PropPaintBomb::ignored>((ignored *)0x0);
  *(int *)((int)&PTR_SetCommand_00c63d90 + unaff_EBX + 2) = iVar5;
  *(undefined **)(&DAT_00cee9b6 + unaff_EBX) = &UNK_00b7009a + unaff_EBX;
  pIVar6 = EntityFactoryDictionary();
  (**pIVar6->_vptr_IEntityFactoryDictionary)(pIVar6,&DAT_00cee9b6 + unaff_EBX,unaff_EBX + 0x8f0350);
  ConCommand::ConCommand
            ((ConCommand *)(unaff_EBX + 0xcee9c2),&UNK_0094679f + unaff_EBX,
             (FnCommandCallback_t)(unaff_EBX + 0x61eeb2),(char *)(unaff_EBX + 0x94676a),0x4000,
             (FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x8d04e2,0,*(undefined4 *)(&DAT_00adf412 + unaff_EBX));
  ConCommand::ConCommand
            ((ConCommand *)((int)CC_CollisionTest::test + unaff_EBX + 6),
             (char *)(unaff_EBX + 0x9467f0),(FnCommandCallback_t)(unaff_EBX + 0x61eea2),
             &UNK_009467ba + unaff_EBX,0x4000,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x8d04c2,0,*(undefined4 *)(&DAT_00adf412 + unaff_EBX));
  ConCommand::ConCommand
            ((ConCommand *)
             ((int)&CUtlRBTree<CUtlMap<char_const*,CBaseEntity*,unsigned_short>::Node_t,unsigned_short,CUtlMap<char_const*,CBaseEntity*,unsigned_short>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<char_const*,CBaseEntity*,unsigned_short>::Node_t,unsigned_short>,unsigned_short>>
                    ::Links(unsigned_short)::s_Sentinel + unaff_EBX + 2),
             (char *)(vgui::ScrollBarSlider::OnCursorExited + unaff_EBX + 4),
             (FnCommandCallback_t)(unaff_EBX + 0x61ee92),&UNK_0094680e + unaff_EBX,0x4000,
             (FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x8d04a2,0,*(undefined4 *)(&DAT_00adf412 + unaff_EBX));
  ConCommand::ConCommand
            ((ConCommand *)((int)&g_EntityPositions + unaff_EBX + 2),(char *)(unaff_EBX + 0x946899),
             (FnCommandCallback_t)(unaff_EBX + 0x61ee82),(char *)(unaff_EBX + 0x946862),0x4000,
             (FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x8d0482,0,*(undefined4 *)(&DAT_00adf412 + unaff_EBX));
  ConCommand::ConCommand
            ((ConCommand *)
             ((int)&s_DefaultSheetSequence.m_TextureCoordData[0].m_fLeft_U0 + unaff_EBX + 2),
             (char *)(unaff_EBX + 0x9468ec),(FnCommandCallback_t)(unaff_EBX + 0x61ee72),
             (char *)(unaff_EBX + 0x9468b6),0x4000,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x8d0462,0,*(undefined4 *)(&DAT_00adf412 + unaff_EBX));
  return;
}


/* __tcf_2 at 0099cde0 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3932f8),in_stack_00000008);
  return;
}


/* __tcf_3 at 0099cdc0 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x393378),in_stack_00000008);
  return;
}


/* __tcf_4 at 0099cda0 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3933f8),in_stack_00000008);
  return;
}


/* __tcf_5 at 0099cd80 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x393478),in_stack_00000008);
  return;
}


/* __tcf_6 at 0099cd60 */

void __tcf_6(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3934f8),in_stack_00000008);
  return;
}


/* __tcf_7 at 0099cd40 */

void __tcf_7(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x393578),in_stack_00000008);
  return;
}


/* __tcf_8 at 0099cd20 */

void __tcf_8(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3935f8),in_stack_00000008);
  return;
}


/* __tcf_9 at 0099cd00 */

void __tcf_9(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x393678),in_stack_00000008);
  return;
}


/* __tcf_10 at 0099cce0 */

void __tcf_10(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3936f8),in_stack_00000008);
  return;
}


/* __tcf_11 at 0099ccc0 */

void __tcf_11(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x393778),in_stack_00000008);
  return;
}


/* __tcf_12 at 0099cca0 */

void __tcf_12(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3937f8),in_stack_00000008);
  return;
}


/* DataMapInit<CPropPaintBomb> at 000cc640 */

datamap_t * DataMapInit<CPropPaintBomb>(CPropPaintBomb *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)power2_n + unaff_EBX + 0x4c) == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xceed2c);
    if (iVar2 != 0) {
      *(int *)((int)power2_n + unaff_EBX + 0x5c) = unaff_EBX + 0x9466d3;
      *(undefined4 *)((int)power2_n + unaff_EBX + 100) = 0;
      *(undefined4 *)((int)power2_n + unaff_EBX + 0x68) = 0;
      *(undefined4 *)((int)power2_n + unaff_EBX + 0x6c) = 0;
      *(undefined4 *)((int)power2_n + unaff_EBX + 0x70) = 0;
      *(undefined4 *)((int)power2_n + unaff_EBX + 0x74) = 0;
      *(undefined4 *)((int)power2_n + unaff_EBX + 0x60) = 0xe;
      ___cxa_guard_release(unaff_EBX + 0xceed2c);
      ___cxa_atexit(unaff_EBX + 0x8d07ac,0,*(undefined4 *)(&DAT_00adf53c + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_HasTarget_00c03738 + unaff_EBX) =
       *(undefined4 *)(&DAT_00adff5c + unaff_EBX);
  if (*(char *)((int)power2_n + unaff_EBX + 0x54) == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xceed34);
    if (iVar2 != 0) {
      uVar1 = **(undefined4 **)(&DAT_00adf558 + unaff_EBX);
      *(undefined4 *)((int)&PTR_ResolveFlyCollisionCustom_00c038a0 + unaff_EBX) = uVar1;
      *(undefined4 *)((int)&PTR_NetworkStateChanged_m_vecVelocity_00c038e0 + unaff_EBX) = uVar1;
      ___cxa_guard_release(unaff_EBX + 0xceed34);
    }
  }
  *(undefined4 *)((int)&PTR_ImpactTrace_00c03730 + unaff_EBX) = 6;
  *(int *)((int)&PTR_DecalTrace_00c0372c + unaff_EBX) = (int)&PTR_StartBlocked_00c0378c + unaff_EBX;
  return (datamap_t *)((int)&PTR_DecalTrace_00c0372c + unaff_EBX);
}


/* CPropPaintBomb::GetDataDescMap at 006ea970 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPropPaintBomb * this) */

datamap_t * __thiscall CPropPaintBomb::GetDataDescMap(CPropPaintBomb *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5e5408);
}


/* CPropPaintBomb::GetBaseMap at 006ea980 */

datamap_t * CPropPaintBomb::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(&DAT_004c1c28 + extraout_ECX);
}


/* __tcf_0 at 0099ce00 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41e596)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41e58a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41e596));
  }
  *(undefined4 *)(unaff_EBX + 0x41e596) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41e592)) {
    if (*(int *)(unaff_EBX + 0x41e58a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20ed8a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20ed8a),*(int *)(unaff_EBX + 0x41e58a));
      *(undefined4 *)(unaff_EBX + 0x41e58a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e58e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41e58a);
  *(int *)(unaff_EBX + 0x41e59a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41e592)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20ed8a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20ed8a),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41e58a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e58e) = 0;
  }
  return;
}


/* ServerClassInit<DT_PropPaintBomb::ignored> at 000cc460 */

int ServerClassInit<DT_PropPaintBomb::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)power2_n + unaff_EBX + 0x114) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xceedf4);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)((int)power2_n + unaff_EBX + 300),
                  (char *)(unaff_EBX + 0x8e65b0),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)((int)power2_n + unaff_EBX + 0x180),
                        (char *)(unaff_EBX + 0x8e8920),0,
                        (SendTable *)**(undefined4 **)(&DAT_00ae0140 + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00adf7b8 + unaff_EBX),0x80);
      SendPropInt((SendProp_conflict *)((int)power2_n + unaff_EBX + 0x1d4),
                  (char *)(unaff_EBX + 0x9468a1),0x798,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      ___cxa_guard_release(unaff_EBX + 0xceedf4);
      ___cxa_atexit(unaff_EBX + 0x8d071c,0,*(undefined4 *)(&DAT_00adf71c + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)((int)&PTR_GetDragPanel_00c64088 + unaff_EBX),
             (SendProp_conflict *)((int)power2_n + unaff_EBX + 0x180),2,
             *(char **)(&DAT_00b70390 + unaff_EBX));
  return 1;
}


/* CPropPaintBomb::GetServerClass at 006ea990 */

/* DWARF original prototype: ServerClass * GetServerClass(CPropPaintBomb * this) */

ServerClass * __thiscall CPropPaintBomb::GetServerClass(CPropPaintBomb *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x6d0788);
}


/* CPropPaintBomb::YouForgotToImplementOrDeclareServerClass at 006ea9a0 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CPropPaintBomb * this) */

int __thiscall CPropPaintBomb::YouForgotToImplementOrDeclareServerClass(CPropPaintBomb *this)

{
  return 0;
}


/* __tcf_13 at 0099cc80 */

void __tcf_13(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x393874),in_stack_00000008);
  return;
}


/* __tcf_1 at 0099cb90 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41e787))(unaff_EBX + 0x41e787);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41e733))(unaff_EBX + 0x41e733);
                    /* WARNING: Could not recover jumptable at 0x0099cbd7. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41e6df))();
  return;
}


/* CPropPaintBomb::CPropPaintBomb at 006eaf20 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CPropPaintBomb(CPropPaintBomb * this) */

void __thiscall CPropPaintBomb::CPropPaintBomb(CPropPaintBomb *this)

{
  uint *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CPhysicsProp::CPhysicsProp(&this->super_CPhysicsProp);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
  super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x551394);
  (this->super_CPhysicsProp).super_CBreakableProp.super_IBreakableWithPropData.
  _vptr_IBreakableWithPropData = (_func_int_varargs **)(unaff_EBX + 0x5517fc);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x551880);
  (this->super_CPhysicsProp).super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(unaff_EBX + 0x5518b0);
  (this->m_OnFizzled).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnFizzled).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_OnFizzled).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_OnExploded).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnExploded).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_OnExploded).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_nPaintPowerType).m_Value = 4;
  if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                      super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) == false
     ) {
    this_00 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating
                .super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if (this_00 != (CBaseEdict *)0x0) {
      this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
      pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
      pIVar2->m_iChangeInfoSerialNumber = 0;
    }
  }
  else {
    puVar1 = (uint *)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                      super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
    *puVar1 = *puVar1 | 1;
  }
  return;
}


/* CPropPaintBomb::CPropPaintBomb at 006eb030 */

/* DWARF original prototype: void CPropPaintBomb(CPropPaintBomb * this, CPropPaintBomb * this) */

void __thiscall CPropPaintBomb::CPropPaintBomb(CPropPaintBomb *this,CPropPaintBomb *this_1)

{
  CPropPaintBomb(this);
  return;
}


/* CEntityFactory<CPropPaintBomb>::Create at 006ecbc0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CPropPaintBomb> * this, char
   * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CPropPaintBomb>::Create(CEntityFactory<CPropPaintBomb> *this,char *pClassName)

{
  CPropPaintBomb *this_00;
  
                    /* Unresolved local var: CPropPaintBomb * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x79c);
  CPropPaintBomb::CPropPaintBomb(this_00);
  (**(code **)((int)(this_00->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x74))(this_00,pClassName);
  return &(this_00->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
          super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CPropPaintBomb::~CPropPaintBomb at 006ead30 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropPaintBomb(CPropPaintBomb * this, int __in_chrg) */

void __thiscall CPropPaintBomb::~CPropPaintBomb(CPropPaintBomb *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
  super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x55158b);
  (this->super_CPhysicsProp).super_CBreakableProp.super_IBreakableWithPropData.
  _vptr_IBreakableWithPropData =
       (_func_int_varargs **)(CNavMesh::IsSelectedSetEmpty + unaff_EBX + 3);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(&UNK_00551a77 + unaff_EBX);
  (this->super_CPhysicsProp).super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(unaff_EBX + 0x551aa7);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnExploded).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFizzled).super_CBaseEntityOutput,in_stack_ffffffe8);
  CPhysicsProp::~CPhysicsProp(&this->super_CPhysicsProp,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CPropPaintBomb::~CPropPaintBomb at 006eade0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropPaintBomb(CPropPaintBomb * this, int __in_chrg) */

void __thiscall CPropPaintBomb::~CPropPaintBomb(CPropPaintBomb *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
  super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x5514db);
  (this->super_CPhysicsProp).super_CBreakableProp.super_IBreakableWithPropData.
  _vptr_IBreakableWithPropData = (_func_int_varargs **)(unaff_EBX + 0x551943);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(&UNK_005519c7 + unaff_EBX);
  (this->super_CPhysicsProp).super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(CNavMesh::IsSelectedSetEmpty + unaff_EBX + 7);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnExploded).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFizzled).super_CBaseEntityOutput,in_stack_ffffffe8);
  CPhysicsProp::~CPhysicsProp(&this->super_CPhysicsProp,__in_chrg);
  return;
}


/* CPropPaintBomb::~CPropPaintBomb at 006eae80 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropPaintBomb(CPropPaintBomb * this, int __in_chrg) */

void __thiscall CPropPaintBomb::~CPropPaintBomb(CPropPaintBomb *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
  super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x55143b);
  (this->super_CPhysicsProp).super_CBreakableProp.super_IBreakableWithPropData.
  _vptr_IBreakableWithPropData = (_func_int_varargs **)(unaff_EBX + 0x5518a3);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x551927);
  (this->super_CPhysicsProp).super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(&UNK_00551957 + unaff_EBX);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnExploded).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFizzled).super_CBaseEntityOutput,in_stack_ffffffe8);
  CPhysicsProp::~CPhysicsProp(&this->super_CPhysicsProp,__in_chrg);
  return;
}


/* CPropPaintBomb::Precache at 006eac80 */

/* DWARF original prototype: void Precache(CPropPaintBomb * this) */

void __thiscall CPropPaintBomb::Precache(CPropPaintBomb *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x3266db));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x328010));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x32802e));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x32804c));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x32806a));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x327fbb));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x327fcb));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x327fdd));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x327fee));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x327fff));
  CPhysicsProp::Precache(&this->super_CPhysicsProp);
  return;
}


/* CPropPaintBomb::Spawn at 006eb040 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CPropPaintBomb * this) */

void __thiscall CPropPaintBomb::Spawn(CPropPaintBomb *this)

{
  undefined1 *puVar1;
  uint uVar2;
  CBaseEdict *this_00;
  IPhysicsObject *pIVar3;
  IChangeInfoAccessor *pIVar4;
  undefined4 uVar5;
  int unaff_EBX;
  uint uVar6;
  
                    /* Unresolved local var: int nPaintBombSurfaceIndex@[???]
                       Unresolved local var: IPhysicsObject * pPhysObject@[???] */
  ___i686_get_pc_thunk_bx();
  (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x68))(this);
  (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x6c))(this,unaff_EBX + 0x326314);
  CBaseEntity::AddEffects((CBaseEntity *)this,0x20);
  uVar2 = (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
          super_CBaseEntity.m_spawnflags.m_Value;
  uVar6 = uVar2 | 0x200;
  if (uVar2 != uVar6) {
    if ((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                  super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar4 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar4->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating
                .super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
    super_CBaseEntity.m_spawnflags.m_Value = uVar6;
  }
  CPhysicsProp::Spawn(&this->super_CPhysicsProp);
  uVar5 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4c0bfc) + 0x10))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x4c0bfc),unaff_EBX + 0x328032);
  pIVar3 = (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
           super_CBaseEntity.m_pPhysicsObject;
  if (pIVar3 != (IPhysicsObject *)0x0) {
    (*pIVar3->_vptr_IPhysicsObject[0x28])(pIVar3,uVar5);
  }
  return;
}


/* CPropPaintBomb::UpdateTransmitState at 006eac60 */

/* DWARF original prototype: int UpdateTransmitState(CPropPaintBomb * this) */

int __thiscall CPropPaintBomb::UpdateTransmitState(CPropPaintBomb *this)

{
  int iVar1;
  
  iVar1 = CBaseEntity::SetTransmitState((CBaseEntity *)this,0x20);
  return iVar1;
}


/* CPropPaintBomb::SetPaintPowerType at 006eb120 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void SetPaintPowerType(CPropPaintBomb * this, PaintPowerType paintType)
    */

void __thiscall CPropPaintBomb::SetPaintPowerType(CPropPaintBomb *this,PaintPowerType paintType)

{
  uint *puVar1;
  CBaseEdict *pCVar2;
  uint8 uVar3;
  uint8 uVar4;
  IChangeInfoAccessor *pIVar5;
  Color *pCVar6;
  int in_stack_ffffffb8;
  
                    /* Unresolved local var: Color color@[???] */
  if (paintType != (this->m_nPaintPowerType).m_Value) {
    if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
        false) {
      pCVar2 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                 super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar5 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar5->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = (uint *)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
      *puVar1 = *puVar1 | 1;
    }
    (this->m_nPaintPowerType).m_Value = paintType;
  }
  pCVar6 = MapPowerToVisualColor((Color *)(this->m_nPaintPowerType).m_Value,in_stack_ffffffb8);
  uVar3 = (uint8)((uint)pCVar6 >> 8);
  uVar4 = (uint8)((uint)pCVar6 >> 0x10);
  if ((uint8)pCVar6 !=
      (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
      super_CBaseEntity.m_clrRender.
      super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r) {
    if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
        false) {
      pCVar2 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                 super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar5 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar5->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = (uint *)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
      *puVar1 = *puVar1 | 1;
    }
    (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
    super_CBaseEntity.m_clrRender.
    super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r = (uint8)pCVar6;
  }
  if (uVar3 != *(uint8 *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                                super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                                super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>
                         + 1)) {
    if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
        false) {
      pCVar2 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                 super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar5 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar5->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = (uint *)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
      *puVar1 = *puVar1 | 1;
    }
    *(uint8 *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                     super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                     super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender> + 1) =
         uVar3;
  }
  if (uVar4 != *(uint8 *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                                super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                                super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>
                         + 2)) {
    if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
        false) {
      pCVar2 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                 super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar5 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar5->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = (uint *)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
      *puVar1 = *puVar1 | 1;
    }
    *(uint8 *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                     super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                     super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender> + 2) =
         uVar4;
  }
  return;
}


/* GenerateBombDirections at 006eb660 */

/* WARNING: Restarted to delay deadcode elimination for space: stack */

void GenerateBombDirections(CUtlVector<Vector,CUtlMemory<Vector,_int>_> *directions)

{
  uint *puVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  float *pfVar5;
  Vector *pVVar6;
  int iVar7;
  int iVar8;
  int unaff_EBX;
  uint uVar9;
  float fVar10;
  float fVar11;
  float fVar12;
  float fVar13;
  float fVar14;
  int local_110;
  float local_10c;
  float local_108;
  float local_104;
  int local_fc;
  int local_f4;
  VMatrix local_d4;
  float local_94;
  float local_90;
  float local_8c;
  vec_t local_88;
  vec_t local_84;
  vec_t local_80;
  vec_t local_7c;
  vec_t local_78;
  vec_t local_74;
  vec_t local_70;
  vec_t local_6c;
  vec_t local_68;
  float local_64;
  float local_60;
  float local_5c;
  float local_58;
  float local_54;
  float local_50;
  vec_t local_4c;
  vec_t local_48;
  vec_t local_44;
  float local_40;
  float local_3c;
  float local_38;
  Vector local_34;
  undefined4 local_28;
  undefined4 local_24;
  float local_20;
  
                    /* Unresolved local var: float flVerticalAngle@[???]
                       Unresolved local var: float flHorizontalAngle@[???]
                       Unresolved local var: Vector vecTemp@[???]
                       Unresolved local var: VMatrix matRotation@[???]
                       Unresolved local var: int nVerticalSplit@[???] */
  ___i686_get_pc_thunk_bx();
  if ((0 < *(int *)(*(int *)(unaff_EBX + 0x644d8b) + 0x30)) &&
     (0 < *(int *)(*(int *)(unaff_EBX + 0x644deb) + 0x30))) {
    directions->m_Size = 0;
    if ((directions->m_Memory).m_nGrowSize < 0) {
      pVVar6 = (directions->m_Memory).m_pMemory;
    }
    else {
      pVVar6 = (directions->m_Memory).m_pMemory;
      if (pVVar6 != (Vector *)0x0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4c0527) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x4c0527),pVVar6);
        (directions->m_Memory).m_pMemory = (Vector *)0x0;
      }
      pVVar6 = (Vector *)0x0;
      (directions->m_Memory).m_nAllocationCount = 0;
    }
    directions->m_pElements = pVVar6;
    fVar14 = *(float *)(*(int *)(unaff_EBX + 0x644deb) + 0x2c);
    local_28 = 0;
    local_24 = 0;
    local_10c = *(float *)(unaff_EBX + 0x33ec9f);
    local_108 = 0.0;
    local_104 = 0.0;
    local_34.x = 0.0;
    local_34.z = 0.0;
    local_34.y = local_10c;
    local_20 = local_10c;
    MatrixBuildRotationAboutAxis
              (&local_d4,&local_34,
               *(float *)(unaff_EBX + 0x33f063) / *(float *)(*(int *)(unaff_EBX + 0x644d8b) + 0x2c))
    ;
    iVar4 = *(int *)(*(int *)(unaff_EBX + 0x644d8b) + 0x30);
    iVar2 = iVar4 + -1;
    if (0 < iVar2) {
      local_f4 = 1;
      do {
                    /* Unresolved local var: Vector vRet@[???] */
        fVar10 = local_104 * local_d4.m[0][0] + local_108 * local_d4.m[0][1] +
                 local_10c * local_d4.m[0][2] + local_d4.m[0][3];
        fVar11 = local_104 * local_d4.m[1][0] + local_108 * local_d4.m[1][1] +
                 local_10c * local_d4.m[1][2] + local_d4.m[1][3];
        local_10c = local_104 * local_d4.m[2][0] + local_108 * local_d4.m[2][1] +
                    local_10c * local_d4.m[2][2] + local_d4.m[2][3];
        uVar9 = directions->m_Size;
        iVar3 = uVar9 + 1;
        iVar7 = (directions->m_Memory).m_nAllocationCount;
        local_40 = fVar10;
        local_3c = fVar11;
        local_38 = local_10c;
        if (iVar7 < iVar3) {
          CUtlMemory<Vector,int>::Grow(&directions->m_Memory,iVar3 - iVar7);
        }
        directions->m_Size = directions->m_Size + 1;
        pVVar6 = (directions->m_Memory).m_pMemory;
        directions->m_pElements = pVVar6;
        iVar7 = ~uVar9 + directions->m_Size;
        if (0 < iVar7) {
          _V_memmove(pVVar6 + iVar3,pVVar6 + uVar9,iVar7 * 0xc);
        }
        pVVar6 = (directions->m_Memory).m_pMemory + uVar9;
        if (pVVar6 != (Vector *)0x0) {
          pVVar6->z = local_10c;
          pVVar6->y = fVar11;
          pVVar6->x = fVar10;
        }
        puVar1 = (uint *)(unaff_EBX + 0x34826f);
        local_88 = (vec_t)((uint)fVar10 ^ *puVar1);
        local_84 = (vec_t)((uint)fVar11 ^ *puVar1);
        local_80 = (vec_t)((uint)local_10c ^ *puVar1);
        iVar7 = directions->m_Size + 1;
        iVar3 = (directions->m_Memory).m_nAllocationCount;
        local_4c = local_88;
        local_48 = local_84;
        local_44 = local_80;
        if (iVar3 < iVar7) {
          CUtlMemory<Vector,int>::Grow(&directions->m_Memory,iVar7 - iVar3);
        }
        directions->m_Size = directions->m_Size + 1;
        pVVar6 = (directions->m_Memory).m_pMemory;
        directions->m_pElements = pVVar6;
        iVar3 = directions->m_Size + -1;
        if (0 < iVar3) {
          _V_memmove(pVVar6 + 1,pVVar6,iVar3 * 0xc);
          pVVar6 = (directions->m_Memory).m_pMemory;
        }
        if (pVVar6 != (Vector *)0x0) {
          pVVar6->x = local_4c;
          pVVar6->y = local_48;
          pVVar6->z = local_44;
        }
        local_f4 = local_f4 + 1;
        local_108 = fVar11;
        local_104 = fVar10;
      } while (local_f4 != iVar4);
    }
    MatrixBuildRotateZ(&local_d4,*(float *)(unaff_EBX + 0x33f063) / fVar14);
    if (1 < *(int *)(*(int *)(unaff_EBX + 0x644d8b) + 0x30)) {
      local_110 = 1;
                    /* Unresolved local var: int startIndex@[???] */
      do {
        uVar9 = directions->m_Size;
        if (0 < iVar2) {
          iVar3 = (uVar9 - iVar2) * 0xc;
          local_fc = 1;
          while( true ) {
            pfVar5 = (float *)((int)&((directions->m_Memory).m_pMemory)->x + iVar3);
            fVar14 = *pfVar5;
            fVar10 = pfVar5[1];
            fVar11 = pfVar5[2];
                    /* Unresolved local var: Vector vRet@[???] */
            fVar12 = fVar14 * local_d4.m[0][0] + fVar10 * local_d4.m[0][1] +
                     fVar11 * local_d4.m[0][2] + local_d4.m[0][3];
            fVar13 = fVar14 * local_d4.m[1][0] + fVar10 * local_d4.m[1][1] +
                     fVar11 * local_d4.m[1][2] + local_d4.m[1][3];
            fVar14 = fVar14 * local_d4.m[2][0] + fVar10 * local_d4.m[2][1] +
                     fVar11 * local_d4.m[2][2] + local_d4.m[2][3];
            iVar7 = uVar9 + 1;
            iVar8 = (directions->m_Memory).m_nAllocationCount;
            local_58 = fVar12;
            local_54 = fVar13;
            local_50 = fVar14;
            if (iVar8 < iVar7) {
              CUtlMemory<Vector,int>::Grow(&directions->m_Memory,iVar7 - iVar8);
            }
            directions->m_Size = directions->m_Size + 1;
            pVVar6 = (directions->m_Memory).m_pMemory;
            directions->m_pElements = pVVar6;
            iVar8 = ~uVar9 + directions->m_Size;
            if (0 < iVar8) {
              _V_memmove(pVVar6 + iVar7,pVVar6 + uVar9,iVar8 * 0xc);
            }
            pVVar6 = (directions->m_Memory).m_pMemory + uVar9;
            if (pVVar6 != (Vector *)0x0) {
              pVVar6->z = fVar14;
              pVVar6->y = fVar13;
              pVVar6->x = fVar12;
            }
            local_94 = -fVar12;
            local_90 = -fVar13;
            local_8c = -fVar14;
            iVar8 = directions->m_Size + 1;
            iVar7 = (directions->m_Memory).m_nAllocationCount;
            local_64 = local_94;
            local_60 = local_90;
            local_5c = local_8c;
            if (iVar7 < iVar8) {
              CUtlMemory<Vector,int>::Grow(&directions->m_Memory,iVar8 - iVar7);
            }
            directions->m_Size = directions->m_Size + 1;
            pVVar6 = (directions->m_Memory).m_pMemory;
            directions->m_pElements = pVVar6;
            iVar7 = directions->m_Size + -1;
            if (0 < iVar7) {
              _V_memmove(pVVar6 + 1,pVVar6,iVar7 * 0xc);
              pVVar6 = (directions->m_Memory).m_pMemory;
            }
            if (pVVar6 != (Vector *)0x0) {
              pVVar6->x = local_64;
              pVVar6->y = local_60;
              pVVar6->z = local_5c;
            }
            local_fc = local_fc + 1;
            iVar3 = iVar3 + 0x18;
            if (local_fc == iVar4) break;
            uVar9 = directions->m_Size;
          }
        }
        local_110 = local_110 + 1;
      } while (local_110 < *(int *)(*(int *)(unaff_EBX + 0x644d8b) + 0x30));
    }
    local_70 = 0.0;
    local_6c = 0.0;
    local_68 = 1.0;
    iVar2 = directions->m_Size + 1;
    iVar4 = (directions->m_Memory).m_nAllocationCount;
    if (iVar4 < iVar2) {
      CUtlMemory<Vector,int>::Grow(&directions->m_Memory,iVar2 - iVar4);
    }
    directions->m_Size = directions->m_Size + 1;
    pVVar6 = (directions->m_Memory).m_pMemory;
    directions->m_pElements = pVVar6;
    iVar4 = directions->m_Size + -1;
    if (0 < iVar4) {
      _V_memmove(pVVar6 + 1,pVVar6,iVar4 * 0xc);
      pVVar6 = (directions->m_Memory).m_pMemory;
    }
    if (pVVar6 != (Vector *)0x0) {
      pVVar6->x = local_70;
      pVVar6->y = local_6c;
      pVVar6->z = local_68;
    }
    local_7c = 0.0;
    local_78 = 0.0;
    local_74 = -1.0;
    uVar9 = directions->m_Size;
    iVar4 = uVar9 + 1;
    iVar2 = (directions->m_Memory).m_nAllocationCount;
    if (iVar2 < iVar4) {
      CUtlMemory<Vector,int>::Grow(&directions->m_Memory,iVar4 - iVar2);
    }
    directions->m_Size = directions->m_Size + 1;
    pVVar6 = (directions->m_Memory).m_pMemory;
    directions->m_pElements = pVVar6;
    iVar2 = ~uVar9 + directions->m_Size;
    if (0 < iVar2) {
      _V_memmove(pVVar6 + iVar4,pVVar6 + uVar9,iVar2 * 0xc);
    }
    pVVar6 = (directions->m_Memory).m_pMemory + uVar9;
    if (pVVar6 != (Vector *)0x0) {
      pVVar6->x = local_7c;
      pVVar6->y = local_78;
      pVVar6->z = local_74;
    }
  }
  return;
}


/* DispatchDryBombParticleEffect at 006eaaf0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void DispatchDryBombParticleEffect(Vector *pos,PaintPowerType paintType,QAngle angSplash)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (paintType < PAINT_POWER_TYPE_COUNT_PLUS_NO_POWER) {
                    /* WARNING: Could not recover jumptable at 0x006eab10. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*(code *)(*(int *)(unaff_EBX + 0x18 + paintType * 4) + unaff_EBX))();
    return;
  }
  return;
}


/* CreatePaintBombExplosion at 006ebf50 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

void CreatePaintBombExplosion(PaintPowerType paintType,Vector *vecExplosionPos,bool bSpawnBlobs)

{
  uint *puVar1;
  float *pfVar2;
  float fVar3;
  float fVar4;
  QAngle angSplash;
  vec_t vVar5;
  vec_t vVar6;
  vec_t vVar7;
  char cVar8;
  CPortal_Base2D *pCVar9;
  CBasePaintBlob *pBlob;
  CBasePlayer *pCVar10;
  CBaseEntity *pPaintedEntity;
  Vector *vecPosition;
  ITraceFilter IVar11;
  int unaff_EBX;
  int iVar12;
  int iVar13;
  longdouble lVar14;
  longdouble lVar15;
  int local_1f8;
  int local_1e0;
  trace_t local_1d0;
  Ray_t local_17c;
  CTraceFilterSimpleClassnameList local_120;
  CUtlVector<Vector,CUtlMemory<Vector,_int>_> local_fc;
  vec_t local_e8;
  vec_t local_e4;
  vec_t local_e0;
  float local_dc;
  float local_d8;
  float local_d4;
  float local_d0;
  float local_cc;
  float local_c8;
  float local_c4;
  float local_c0;
  float local_bc;
  Vector local_b8;
  float local_ac;
  float local_a8;
  float local_a4;
  Vector local_a0;
  float local_94;
  float local_90;
  float local_8c;
  Vector local_88;
  Vector local_7c;
  Vector local_70;
  float local_64;
  float local_60;
  float local_5c;
  float local_58;
  float local_54;
  float local_50;
  float local_4c;
  float local_48;
  float local_44;
  Vector local_40;
  QAngle local_34;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  
                    /* Unresolved local var: CUtlVector<Vector,CUtlMemory<Vector,_int>_> dirs@[???]
                       Unresolved local var: QAngle angSplashAngle@[???]
                       Unresolved local var: float radius@[???] */
  ___i686_get_pc_thunk_bx();
  if (!bSpawnBlobs) {
    local_28 = 0;
    local_24 = 0;
    local_20 = 0x3f800000;
    DispatchDryBombParticleEffect(vecExplosionPos,paintType,(QAngle)(ZEXT412(0x3f800000) << 0x40));
    return;
  }
  local_fc.m_Memory.m_pMemory = (Vector *)0x0;
  local_fc.m_Memory.m_nAllocationCount = 0;
  local_fc.m_Memory.m_nGrowSize = 0;
  local_fc.m_Size = 0;
  local_fc.m_pElements = (Vector *)0x0;
  local_34.x = 0.0;
  local_34.y = 0.0;
  local_34.z = 1.0;
  GenerateBombDirections(&local_fc);
  fVar3 = *(float *)(*(int *)(unaff_EBX + 0x64419b) + 0x2c);
  if (0 < local_fc.m_Size) {
    local_1f8 = 0;
    local_1e0 = 0;
    IVar11._vptr_ITraceFilter = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4bfd67) + 8);
    do {
                    /* Unresolved local var: Vector norm@[???] */
      local_40.x = *(float *)((int)&(local_fc.m_Memory.m_pMemory)->x + local_1e0);
      local_40.y = *(float *)((int)&(local_fc.m_Memory.m_pMemory)->y + local_1e0);
      local_40.z = *(float *)((int)&(local_fc.m_Memory.m_pMemory)->z + local_1e0);
      VectorNormalize(&local_40);
      vVar7 = local_40.z;
      vVar6 = local_40.y;
      vVar5 = local_40.x;
      CTraceFilterSimpleClassnameList::CTraceFilterSimpleClassnameList
                (&local_120,(IHandleEntity *)0x0,0);
      CTraceFilterSimpleClassnameList::AddClassnameToIgnore
                (&local_120,(char *)(unaff_EBX + 0x2d0b6d));
                    /* Unresolved local var: Vector res@[???] */
      local_c4 = fVar3 * vVar5;
      local_c0 = fVar3 * vVar6;
      local_bc = vVar7 * fVar3;
                    /* Unresolved local var: Vector res@[???] */
      local_17c.m_Start.super_Vector.x = vecExplosionPos->x;
      local_58 = local_17c.m_Start.super_Vector.x + local_c4;
      local_17c.m_Start.super_Vector.y = vecExplosionPos->y;
      local_54 = local_17c.m_Start.super_Vector.y + local_c0;
      local_17c.m_Start.super_Vector.z = vecExplosionPos->z;
      local_50 = local_17c.m_Start.super_Vector.z + local_bc;
      local_17c.m_Delta.super_Vector.x = local_58 - vecExplosionPos->x;
      local_17c.m_Delta.super_Vector.y = local_54 - vecExplosionPos->y;
      local_17c.m_Delta.super_Vector.z = local_50 - vecExplosionPos->z;
      local_17c.m_IsSwept =
           local_17c.m_Delta.super_Vector.x * local_17c.m_Delta.super_Vector.x +
           local_17c.m_Delta.super_Vector.y * local_17c.m_Delta.super_Vector.y +
           local_17c.m_Delta.super_Vector.z * local_17c.m_Delta.super_Vector.z != 0.0;
      local_17c.m_Extents.super_Vector.z = 0.0;
      local_17c.m_Extents.super_Vector.y = 0.0;
      local_17c.m_Extents.super_Vector.x = 0.0;
      local_17c.m_pWorldAxisTransform = (matrix3x4_t *)0x0;
      local_17c.m_IsRay = true;
      local_17c.m_StartOffset.super_Vector.z = 0.0;
      local_17c.m_StartOffset.super_Vector.y = 0.0;
      local_17c.m_StartOffset.super_Vector.x = 0.0;
      local_4c = local_c4;
      local_48 = local_c0;
      local_44 = local_bc;
      pCVar9 = UTIL_Portal_TraceRay
                         (&local_17c,0x46004003,(ITraceFilter *)&local_120,&local_1d0,true);
      if (((local_1d0.super_CBaseTrace.fraction < *(float *)(unaff_EBX + 0x33e3af)) &&
          (local_1d0.m_pEnt != (CBaseEntity *)0x0)) && (pCVar9 == (CPortal_Base2D *)0x0)) {
        if (*(int *)(*(int *)(unaff_EBX + 0x6442bb) + 0x30) == 0) {
          CPaintDatabase::AddPaint(*(CPaintDatabase **)(unaff_EBX + 0x4bffaf),&local_1d0,paintType);
        }
        else {
                    /* Unresolved local var: float flStreakSpeed@[???]
                       Unresolved local var: float flStreakTime@[???]
                       Unresolved local var: float flStreakDampen@[DW_OP_reg11(ST0)] */
          lVar14 = (longdouble)
                   _RandomFloat(*(undefined4 *)(*(int *)(unaff_EBX + 0x6441fb) + 0x2c),
                                *(undefined4 *)(*(int *)(unaff_EBX + 0x64425b) + 0x2c));
          fVar4 = (float)lVar14;
          lVar14 = (longdouble)
                   _RandomFloat(*(undefined4 *)(*(int *)(unaff_EBX + 0x64431b) + 0x2c),
                                *(undefined4 *)(*(int *)(unaff_EBX + 0x64437b) + 0x2c));
          lVar15 = (longdouble)
                   _RandomFloat(*(undefined4 *)(*(int *)(unaff_EBX + 0x6443db) + 0x2c),
                                *(undefined4 *)(*(int *)(unaff_EBX + 0x64443b) + 0x2c));
                    /* Unresolved local var: Vector res@[???] */
          local_d0 = fVar4 * vVar5;
          local_cc = fVar4 * vVar6;
          local_c8 = vVar7 * fVar4;
                    /* Unresolved local var: Vector res@[???] */
          pfVar2 = (float *)(unaff_EBX + 0x33e3b3);
          local_dc = *pfVar2 * local_1d0.super_CBaseTrace.plane.normal.x;
          local_d8 = *pfVar2 * local_1d0.super_CBaseTrace.plane.normal.y;
          local_d4 = *pfVar2 * local_1d0.super_CBaseTrace.plane.normal.z;
                    /* Unresolved local var: Vector res@[???] */
          local_70.x = local_1d0.super_CBaseTrace.endpos.x + local_dc;
          local_70.y = local_1d0.super_CBaseTrace.endpos.y + local_d8;
          local_70.z = local_1d0.super_CBaseTrace.endpos.z + local_d4;
          local_7c.x = local_d0;
          local_7c.y = local_cc;
          local_7c.z = local_c8;
          local_64 = local_dc;
          local_60 = local_d8;
          local_5c = local_d4;
          pBlob = PaintBlobCreate(&local_70,&local_7c,paintType,(float)lVar14,(float)lVar15,false,0)
          ;
          CPaintStreamManager::AddPaintBlob(*(CPaintStreamManager **)(unaff_EBX + 0x4bfe7f),pBlob);
        }
        puVar1 = (uint *)(unaff_EBX + 0x34797f);
        local_e0 = (vec_t)((uint)local_1d0.super_CBaseTrace.plane.normal.x ^ *puVar1);
        local_e4 = (vec_t)((uint)local_1d0.super_CBaseTrace.plane.normal.y ^ *puVar1);
        local_e8 = (vec_t)((uint)local_1d0.super_CBaseTrace.plane.normal.z ^ *puVar1);
        local_88.x = local_e8;
        local_88.y = local_e4;
        local_88.z = local_e0;
        VectorAngles(&local_88,&local_34);
      }
      local_120.m_PassClassnames.m_Size = 0;
      local_120.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter =
           (ITraceFilter)(ITraceFilter)IVar11._vptr_ITraceFilter;
      if (-1 < local_120.m_PassClassnames.m_Memory.m_nGrowSize) {
        if (local_120.m_PassClassnames.m_Memory.m_pMemory != (char **)0x0) {
          (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4bfc37) + 8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x4bfc37),
                     local_120.m_PassClassnames.m_Memory.m_pMemory);
          local_120.m_PassClassnames.m_Memory.m_pMemory = (char **)0x0;
        }
        local_120.m_PassClassnames.m_Memory.m_nAllocationCount = 0;
      }
      local_120.m_PassClassnames.m_pElements = local_120.m_PassClassnames.m_Memory.m_pMemory;
      if (-1 < local_120.m_PassClassnames.m_Memory.m_nGrowSize) {
        if (local_120.m_PassClassnames.m_Memory.m_pMemory != (char **)0x0) {
          (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4bfc37) + 8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x4bfc37),
                     local_120.m_PassClassnames.m_Memory.m_pMemory);
          local_120.m_PassClassnames.m_Memory.m_pMemory = (char **)0x0;
        }
        local_120.m_PassClassnames.m_Memory.m_nAllocationCount = 0;
      }
      local_1f8 = local_1f8 + 1;
      local_1e0 = local_1e0 + 0xc;
    } while (local_1f8 < local_fc.m_Size);
  }
                    /* Unresolved local var: int i@[???] */
  if (((*(int *)(*(int *)(*(int *)(unaff_EBX + 0x4bfe6f) + 0x1c) + 0x30) != 0) ||
      (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x4bfe6b) + 0x1c) + 0x30) != 0)) &&
     (0 < *(int *)(**(int **)(unaff_EBX + 0x4bfc3f) + 0x14))) {
    iVar12 = 1;
    do {
      while (((pCVar10 = UTIL_PlayerByIndex(iVar12), pCVar10 == (CBasePlayer *)0x0 ||
              (cVar8 = (**(code **)((int)(pCVar10->super_CBaseCombatCharacter).super_CBaseFlex.
                                         super_CBaseAnimatingOverlay.super_CBaseAnimating.
                                         super_CBaseEntity.super_IServerEntity.super_IServerUnknown
                                   + 0x154))(pCVar10), cVar8 == '\0')) ||
             ((pPaintedEntity =
                    (CBaseEntity *)
                    ___dynamic_cast(pCVar10,*(undefined4 *)(unaff_EBX + 0x4bfc67),
                                    *(undefined4 *)
                                     (CAI_ActBusyBehavior::NPC_TranslateActivity + unaff_EBX + 7),0)
              , pPaintedEntity == (CBaseEntity *)0x0 ||
              (pPaintedEntity[3].m_OnUser3.super_CBaseEntityOutput.m_Value.field_0.vecVal[2] ==
               2.8026e-45))))) {
LAB_006ec56d:
        iVar12 = iVar12 + 1;
        if (*(int *)(**(int **)(unaff_EBX + 0x4bfc3f) + 0x14) < iVar12) goto LAB_006ec663;
      }
      vecPosition = (Vector *)
                    (*(pPaintedEntity->super_IServerEntity).super_IServerUnknown.super_IHandleEntity
                      ._vptr_IHandleEntity[0x9d])(pPaintedEntity);
                    /* Unresolved local var: Vector res@[???] */
      local_94 = vecExplosionPos->x - vecPosition->x;
      local_90 = vecExplosionPos->y - vecPosition->y;
      local_8c = vecExplosionPos->z - vecPosition->z;
      if (fVar3 * fVar3 <= local_94 * local_94 + local_90 * local_90 + local_8c * local_8c)
      goto LAB_006ec56d;
      CPaintDatabase::PaintEntity
                (*(CPaintDatabase **)(unaff_EBX + 0x4bffaf),pPaintedEntity,paintType,vecPosition);
      iVar12 = iVar12 + 1;
    } while (iVar12 <= *(int *)(**(int **)(unaff_EBX + 0x4bfc3f) + 0x14));
  }
LAB_006ec663:
  angSplash.y = local_34.y;
  angSplash.x = local_34.x;
  angSplash.z = local_34.z;
  DispatchDryBombParticleEffect(vecExplosionPos,paintType,angSplash);
                    /* Unresolved local var: int i@[???] */
  if ((*(int *)(*(int *)(&DAT_0064455b + unaff_EBX) + 0x30) != 0) && (0 < local_fc.m_Size)) {
    iVar13 = 0;
    iVar12 = 0;
                    /* Unresolved local var: Vector vecBlobFireDir@[???] */
    do {
                    /* Unresolved local var: Vector norm@[???] */
      local_a0.x = *(float *)((int)&(local_fc.m_Memory.m_pMemory)->x + iVar12);
      local_a0.y = *(float *)((int)&(local_fc.m_Memory.m_pMemory)->y + iVar12);
      local_a0.z = *(float *)((int)&(local_fc.m_Memory.m_pMemory)->z + iVar12);
      VectorNormalize(&local_a0);
                    /* Unresolved local var: Vector res@[???] */
      pfVar2 = (float *)(unaff_EBX + 0x33f383);
      local_ac = *pfVar2 * local_a0.x;
      local_a8 = local_a0.y * *pfVar2;
      local_a4 = local_a0.z * *pfVar2;
                    /* Unresolved local var: Vector res@[???] */
      local_b8.x = vecExplosionPos->x + local_ac;
      local_b8.y = vecExplosionPos->y + local_a8;
      local_b8.z = vecExplosionPos->z + local_a4;
      NDebugOverlay::Line(vecExplosionPos,&local_b8,0,0xff,0,false,5.0);
      iVar13 = iVar13 + 1;
      iVar12 = iVar12 + 0xc;
    } while (iVar13 < local_fc.m_Size);
  }
  local_fc.m_Size = 0;
  if (-1 < local_fc.m_Memory.m_nGrowSize) {
    if (local_fc.m_Memory.m_pMemory != (Vector *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4bfc37) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4bfc37),local_fc.m_Memory.m_pMemory);
      local_fc.m_Memory.m_pMemory = (Vector *)0x0;
    }
    local_fc.m_Memory.m_nAllocationCount = 0;
  }
  if ((-1 < local_fc.m_Memory.m_nGrowSize) && (local_fc.m_Memory.m_pMemory != (Vector *)0x0)) {
    local_fc.m_pElements = local_fc.m_Memory.m_pMemory;
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4bfc37) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4bfc37),local_fc.m_Memory.m_pMemory);
  }
  return;
}


/* CPropPaintBomb::Event_Killed at 006eca40 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void Event_Killed(CPropPaintBomb * this, CTakeDamageInfo * info) */

void __thiscall CPropPaintBomb::Event_Killed(CPropPaintBomb *this,CTakeDamageInfo *info)

{
  uint uVar1;
  CBaseEntity *this_00;
  byte bVar2;
  bool bVar3;
  int iVar4;
  undefined4 uVar5;
  int unaff_EBX;
  Vector local_2c;
  char *local_20;
  
                    /* Unresolved local var: Vector vecPos@[???]
                       Unresolved local var: bool bSpawnBlobs@[???] */
  ___i686_get_pc_thunk_bx();
  uVar1 = (info->m_hInflictor).super_CBaseHandle.m_Index;
  if ((uVar1 == 0xffffffff) ||
     (iVar4 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4bf156),
     *(uint *)(iVar4 + 8) != uVar1 >> 0x10)) {
    uVar5 = 0;
  }
  else {
    uVar5 = *(undefined4 *)(iVar4 + 4);
  }
  (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x45c))(this,uVar5,info);
  if (((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
       super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
  local_2c.x = (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
               super_CBaseEntity.m_vecAbsOrigin.x;
  local_2c.y = (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
               super_CBaseEntity.m_vecAbsOrigin.y;
  local_2c.z = (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
               super_CBaseEntity.m_vecAbsOrigin.z;
  uVar1 = (info->m_hInflictor).super_CBaseHandle.m_Index;
  if (((uVar1 == 0xffffffff) ||
      (iVar4 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4bf156),
      *(uint *)(iVar4 + 8) != uVar1 >> 0x10)) ||
     (this_00 = *(CBaseEntity **)(iVar4 + 4), this_00 == (CBaseEntity *)0x0)) {
LAB_006ecadb:
    bVar2 = 0;
  }
  else {
    local_20 = (char *)(unaff_EBX + 0x326642);
    if ((this_00->m_iClassname).pszValue != local_20) {
      bVar3 = CBaseEntity::ClassMatchesComplex(this_00,local_20);
      if (!bVar3) goto LAB_006ecadb;
    }
    bVar2 = 1;
  }
  CreatePaintBombExplosion((this->m_nPaintPowerType).m_Value,&local_2c,(bool)(bVar2 ^ 1));
  COutputEvent::FireOutput(&this->m_OnExploded,(CBaseEntity *)this,(CBaseEntity *)this,0.0);
  return;
}


/* CPropPaintBomb::CleansePaintPower at 006eb640 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void CleansePaintPower(CPropPaintBomb * this) */

void __thiscall CPropPaintBomb::CleansePaintPower(CPropPaintBomb *this)

{
  SetPaintPowerType(this,NO_POWER);
  return;
}


/* CPropPaintBomb::Break at 006eaaa0 */

/* DWARF original prototype: void Break(CPropPaintBomb * this, CBaseEntity * pBreaker,
   CTakeDamageInfo * info) */

void __thiscall
CPropPaintBomb::Break(CPropPaintBomb *this,CBaseEntity *pBreaker,CTakeDamageInfo *info)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x328197),0.0,(float *)0x0);
  UTIL_Remove((CBaseEntity *)this);
  return;
}


/* CPropPaintBomb::OnFizzled at 006eaa70 */

/* DWARF original prototype: void OnFizzled(CPropPaintBomb * this) */

void __thiscall CPropPaintBomb::OnFizzled(CPropPaintBomb *this)

{
  COutputEvent::FireOutput(&this->m_OnFizzled,(CBaseEntity *)this,(CBaseEntity *)this,0.0);
  return;
}


/* CPropPaintBomb::InputDissolve at 006ea9b0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void InputDissolve(CPropPaintBomb * this, inputdata_t * in) */

void __thiscall CPropPaintBomb::InputDissolve(CPropPaintBomb *this,inputdata_t *in)

{
  undefined4 *puVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  puVar1 = *(undefined4 **)(unaff_EBX + 0x4c11ec);
  (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x394))
            (this,0,*(float *)(unaff_EBX + 0x33f950) +
                    *(float *)(**(int **)(unaff_EBX + 0x4c11dc) + 0xc),0,0,*puVar1,puVar1[1],
             puVar1[2],0);
                    /* WARNING: Could not recover jumptable at 0x006eaa43. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x3a0))();
  return;
}


/* CPropPaintBomb::InputDisablePortalFunnel at 006eaa50 */

/* DWARF original prototype: void InputDisablePortalFunnel(CPropPaintBomb * this, inputdata_t * in)
    */

void __thiscall CPropPaintBomb::InputDisablePortalFunnel(CPropPaintBomb *this,inputdata_t *in)

{
  (this->super_CPhysicsProp).m_bAllowPortalFunnel = false;
  return;
}


/* CPropPaintBomb::InputEnablePortalFunnel at 006eaa60 */

/* DWARF original prototype: void InputEnablePortalFunnel(CPropPaintBomb * this, inputdata_t * in)
    */

void __thiscall CPropPaintBomb::InputEnablePortalFunnel(CPropPaintBomb *this,inputdata_t *in)

{
  (this->super_CPhysicsProp).m_bAllowPortalFunnel = true;
  return;
}


/* CreatePaintBomb at 006eb2a0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

void CreatePaintBomb(PaintPowerType paintType)

{
  int *piVar1;
  int iVar2;
  code *pcVar3;
  bool bVar4;
  CPropPaintBomb *this;
  CBasePlayer *this_00;
  int unaff_EBX;
  uint *puVar5;
  Vector local_110;
  Vector local_104 [2];
  float local_e4;
  float local_bc;
  float local_b8;
  float local_b4;
  float local_ac;
  float local_a8;
  float local_a4;
  undefined4 local_9c;
  undefined4 local_98;
  undefined4 local_94;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  undefined4 local_7c;
  undefined1 local_78;
  undefined1 local_77;
  CTraceFilterSimple local_68;
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
  Vector local_28 [2];
  
                    /* Unresolved local var: CMDLCacheCriticalSection cacheCriticalSection@[???]
                       Unresolved local var: bool bAllowPrecache@[???]
                       Unresolved local var: CPropPaintBomb * pPaintBomb@[???] */
  puVar5 = (uint *)&stack0xfffffeb4;
  ___i686_get_pc_thunk_bx();
  piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x4c0963);
  (**(code **)(*piVar1 + 0x74))();
  bVar4 = CBaseEntity::IsPrecacheAllowed();
  CBaseEntity::SetAllowPrecache(true);
  this = (CPropPaintBomb *)CreateEntityByName(&UNK_002d181d + unaff_EBX,-1,true);
  if (this != (CPropPaintBomb *)0x0) {
    (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                      super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
                      super_IServerUnknown + 0x68))();
    CPropPaintBomb::SetPaintPowerType(this,paintType);
    DispatchSpawn((CBaseEntity *)this,true);
    this_00 = UTIL_GetCommandClient();
    CBasePlayer::EyeVectors(this_00,local_28,(Vector *)0x0,(Vector *)0x0);
                    /* Unresolved local var: Vector res@[???] */
    local_44 = *(float *)(unaff_EBX + 0x33ffd3);
    local_4c = local_28[0].x * local_44;
    local_48 = local_28[0].y * local_44;
    local_44 = local_44 * local_28[0].z;
    (**(code **)((int)(this_00->super_CBaseCombatCharacter).super_CBaseFlex.
                      super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                      super_IServerEntity.super_IServerUnknown + 0x220))();
                    /* Unresolved local var: Vector res@[???] */
    local_58 = local_40 + local_4c;
    local_54 = local_3c + local_48;
    local_50 = local_38 + local_44;
    (**(code **)((int)(this_00->super_CBaseCombatCharacter).super_CBaseFlex.
                      super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                      super_IServerEntity.super_IServerUnknown + 0x220))();
    puVar5 = (uint *)&stack0xfffffeac;
    local_ac = local_58 - local_34;
    local_a8 = local_54 - local_30;
    local_a4 = local_50 - local_2c;
    local_77 = local_ac * local_ac + local_a8 * local_a8 + local_a4 * local_a4 != 0.0;
    local_84 = 0;
    local_88 = 0;
    local_8c = 0;
    local_7c = 0;
    local_78 = 1;
    local_94 = 0;
    local_98 = 0;
    local_9c = 0;
    local_bc = local_34;
    local_b8 = local_30;
    local_b4 = local_2c;
    CTraceFilterSimple::CTraceFilterSimple
              (&local_68,(IHandleEntity *)this_00,0,(ShouldHitFunc_t)0x0);
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4c095b) + 0x14))();
    if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x4c0957) + 0x1c) + 0x30) != 0) {
      DebugDrawLine(&local_110,local_104,0xff,0,0,true,-1.0);
    }
    if (*(float *)(unaff_EBX + 0x33f05f) != local_e4) {
      local_104[0].z = local_104[0].z + *(float *)(unaff_EBX + 0x33f28b);
      (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
                        super_IServerUnknown + 0x1c4))();
      UTIL_DropToFloor((CBaseEntity *)this,0x200400b,(CBaseEntity *)0x0);
    }
    CBaseEntity::UpdateObjectCapsCache((CBaseEntity *)this);
  }
  *puVar5 = (uint)bVar4;
  puVar5[-1] = 0x6eb518;
  CBaseEntity::SetAllowPrecache(SUB41(*puVar5,0));
  iVar2 = *piVar1;
  *puVar5 = (uint)piVar1;
  pcVar3 = *(code **)(iVar2 + 0x78);
  puVar5[-1] = 0x6eb526;
  (*pcVar3)();
  return;
}


/* ent_create_paint_bomb_jump at 006eb630 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void ent_create_paint_bomb_jump(CCommand *args)

{
  CreatePaintBomb(BOUNCE_POWER);
  return;
}


/* __tcf_14 at 0099cc60 */

void __tcf_14(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x41e4d8),in_stack_00000008);
  return;
}


/* ent_create_paint_bomb_speed at 006eb620 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void ent_create_paint_bomb_speed(CCommand *args)

{
  CreatePaintBomb(SPEED_POWER);
  return;
}


/* __tcf_15 at 0099cc40 */

void __tcf_15(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x41e538),in_stack_00000008);
  return;
}


/* ent_create_paint_bomb_stick at 006eb610 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void ent_create_paint_bomb_stick(CCommand *args)

{
  CreatePaintBomb(STICK_POWER);
  return;
}


/* __tcf_16 at 0099cc20 */

void __tcf_16(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x41e598),in_stack_00000008);
  return;
}


/* ent_create_paint_bomb_portal at 006eb600 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void ent_create_paint_bomb_portal(CCommand *args)

{
  CreatePaintBomb(PORTAL_POWER);
  return;
}


/* __tcf_17 at 0099cc00 */

void __tcf_17(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x41e5f8),in_stack_00000008);
  return;
}


/* ent_create_paint_bomb_erase at 006eb5f0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void ent_create_paint_bomb_erase(CCommand *args)

{
  CreatePaintBomb(NO_POWER);
  return;
}


/* __tcf_18 at 0099cbe0 */

void __tcf_18(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x41e658),in_stack_00000008);
  return;
}


/* _GLOBAL__I_paintbomb_explosion_radius at 000cced0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_paintbomb_explosion_radius(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

