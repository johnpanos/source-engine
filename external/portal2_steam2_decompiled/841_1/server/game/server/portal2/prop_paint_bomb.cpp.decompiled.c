/* DWARF-guided pseudocode for game/server/portal2/prop_paint_bomb.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000b75b0 */

/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "TreeNodeText": ignoring multiple overlapping fields */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  SendTable *this;
  undefined4 *puVar1;
  datamap_t *pdVar2;
  int iVar3;
  int iVar4;
  IEntityFactoryDictionary *pIVar5;
  int unaff_EBX;
  undefined4 *puVar6;
  undefined4 *puVar7;
  longlong lVar8;
  ConVar *this_1;
  ConVar *in_stack_ffffffc8;
  char *in_stack_ffffffcc;
  char *in_stack_ffffffd0;
  int in_stack_ffffffd4;
  char *s2;
  char *in_stack_ffffffd8;
  
  lVar8 = ___i686_get_pc_thunk_bx();
  if (lVar8 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)&ai_no_select_box.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX + 2)
       = 0;
  *(undefined1 *)((int)&ai_no_select_box.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX + 3)
       = 0;
  *(undefined1 *)((int)&ai_no_select_box.super_ConCommandBase.m_pNext + unaff_EBX) = 0;
  *(undefined1 *)((int)&ai_no_select_box.super_ConCommandBase.m_pNext + unaff_EBX + 1) = 0;
  *(undefined4 *)((int)&ai_no_select_box.super_ConCommandBase.m_pNext + unaff_EBX + 2) = 0;
  *(undefined4 *)(&ai_no_select_box.super_ConCommandBase.field_0xa + unaff_EBX) = 0;
  *(undefined4 *)((int)&ai_no_select_box.super_ConCommandBase.m_pszName + unaff_EBX + 2) =
       0x7f7fffff;
  *(undefined4 *)((int)&ai_no_select_box.super_ConCommandBase.m_pszHelpString + unaff_EBX + 2) =
       0x7f7fffff;
  *(undefined4 *)((int)&ai_no_select_box.super_ConCommandBase.m_nFlags + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&ai_no_select_box.super_IConVar._vptr_IConVar + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&ai_no_select_box.m_pParent + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&ai_no_select_box.m_pszDefaultValue + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&ai_no_select_box.m_Value.m_pszString + unaff_EBX + 2) = 0x7f7fffff;
  *(undefined4 *)((int)&ai_no_select_box.m_Value.m_StringLength + unaff_EBX + 2) = 0x7f7fffff;
  *(undefined4 *)((int)&ai_no_select_box.m_Value.m_fValue + unaff_EBX + 2) = 0x7f7fffff;
  *(undefined4 *)((int)&ai_no_select_box.m_Value.m_nValue + unaff_EBX + 2) = 0x7f7fffff;
  *(undefined **)(&ai_no_select_box.field_0x36 + unaff_EBX) = &UNK_00b94d4a + unaff_EBX;
  this_1 = (ConVar *)(unaff_EBX + 0x9fdaa2);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<AIScheduleState_t>::dataDesc[1].flatOffset + unaff_EBX + -0x32),
                 &UNK_009fdad9 + unaff_EBX,(char *)(unaff_EBX + 0x9a3d74),0x4000,(char *)this_1,
                 in_stack_ffffffc8,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,
                 in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x988f32,0,*(undefined4 *)(&DAT_00b8f596 + unaff_EBX));
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<AIScheduleState_t>::dataDesc[2].flatOffset + unaff_EBX + -0x12),
                 (char *)(unaff_EBX + 0x9fdafa),(char *)(vgui::TreeNode::SetVisible + unaff_EBX + 4)
                 ,0x4000,this_1,(char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x988f12,0,*(undefined4 *)(&DAT_00b8f596 + unaff_EBX));
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<AIScheduleState_t>::dataDesc[4].flatOffset + unaff_EBX + -0x32),
                 (char *)(unaff_EBX + 0x9fdb1b),(char *)(unaff_EBX + 0x9fdb15),0x4000,this_1,
                 (char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x988ef2,0,*(undefined4 *)(&DAT_00b8f596 + unaff_EBX));
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<AIScheduleState_t>::dataDesc[5].flatOffset + unaff_EBX + -0x12),
                 (char *)(unaff_EBX + 0x9fdb3e),(char *)(unaff_EBX + 0x9fdb36),0x4000,this_1,
                 (char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x988ed2,0,*(undefined4 *)(&DAT_00b8f596 + unaff_EBX));
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<AIScheduleState_t>::dataDesc[7].flatOffset + unaff_EBX + -0x32),
                 &UNK_009fdb5e + unaff_EBX,(char *)(unaff_EBX + 0x9fb226),0x4000,this_1,
                 (char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x988eb2,0,*(undefined4 *)(&DAT_00b8f596 + unaff_EBX));
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<AIScheduleState_t>::dataDesc[8].flatOffset + unaff_EBX + -0x12),
                 (char *)(unaff_EBX + 0x9fdb86),&UNK_009fdb7e + unaff_EBX,0x4000,this_1,
                 (char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x988e92,0,*(undefined4 *)(&DAT_00b8f596 + unaff_EBX));
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<CAI_BaseNPC>::dataDesc[0].flatOffset + unaff_EBX + -0x32),
                 (char *)(unaff_EBX + 0x9fdbae),&UNK_009a505e + unaff_EBX,0x4000,this_1,
                 (char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x988e72,0,*(undefined4 *)(&DAT_00b8f596 + unaff_EBX));
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<CAI_BaseNPC>::dataDesc[1].flatOffset + unaff_EBX + -0x12),
                 (char *)(unaff_EBX + 0x9fdbda),
                 (char *)(vgui::TreeView::CanCurrentlyEditLabel + unaff_EBX + 6),0x4000,this_1,
                 (char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x988e52,0,*(undefined4 *)(&DAT_00b8f596 + unaff_EBX));
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<CAI_BaseNPC>::dataDesc[3].flatOffset + unaff_EBX + -0x32),
                 (char *)(unaff_EBX + 0x9fdbfa),
                 (char *)(vgui::TreeView::CanCurrentlyEditLabel + unaff_EBX + 6),0x4000,this_1,
                 (char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x988e32,0,*(undefined4 *)(&DAT_00b8f596 + unaff_EBX));
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<CAI_BaseNPC>::dataDesc[4].flatOffset + unaff_EBX + -0x12),
                 (char *)(unaff_EBX + 0x9fdc1d),(char *)(unaff_EBX + 0x997842),0x4000,this_1,
                 (char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x988e12,0,*(undefined4 *)(&DAT_00b8f596 + unaff_EBX));
  pdVar2 = DataMapInit<CPropPaintBomb>((CPropPaintBomb *)0x0);
  *(datamap_t **)((int)DataMapInit<CAI_BaseNPC>::dataDesc[5].flatOffset + unaff_EBX + 6) = pdVar2;
  iVar4 = unaff_EBX + 0xd98e1a;
  s2 = (char *)(unaff_EBX + 0x9fda91);
  *(char **)((int)&ai_no_select_box.m_fMinVal + unaff_EBX + 2) = s2;
  this = (SendTable *)((int)DataMapInit<CAI_BaseNPC>::dataDesc[5].flatOffset + unaff_EBX + 10);
  *(SendTable **)(&ai_no_select_box.field_0x3e + unaff_EBX) = this;
  *(undefined4 *)
   ((int)&ai_no_select_box.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX + 2) = 0xffff
  ;
  puVar7 = (undefined4 *)**(int **)(&DAT_00b8f636 + unaff_EBX);
  if (puVar7 == (undefined4 *)0x0) {
    **(int **)(&DAT_00b8f636 + unaff_EBX) = iVar4;
    *(undefined4 *)((int)&ai_no_select_box.m_fMaxVal + unaff_EBX + 2) = 0;
  }
  else {
    puVar1 = (undefined4 *)puVar7[2];
    iVar3 = _V_stricmp((char *)*puVar7,s2);
    if (iVar3 < 1) {
      while ((puVar6 = puVar1, puVar6 != (undefined4 *)0x0 &&
             (iVar3 = _V_stricmp((char *)*puVar6,s2), iVar3 < 1))) {
        puVar1 = (undefined4 *)puVar6[2];
        puVar7 = puVar6;
      }
      *(undefined4 **)(iVar4 + 8) = puVar6;
      puVar7[2] = iVar4;
    }
    else {
      *(undefined4 *)(iVar4 + 8) = **(undefined4 **)(&DAT_00b8f636 + unaff_EBX);
      **(int **)(&DAT_00b8f636 + unaff_EBX) = iVar4;
    }
  }
  SendTable::SendTable(this);
  ___cxa_atexit(unaff_EBX + 0x988df2,0,*(undefined4 *)(&DAT_00b8f596 + unaff_EBX));
  iVar4 = ServerClassInit<DT_PropPaintBomb::ignored>((ignored *)0x0);
  *(int *)((int)DataMapInit<CAI_BaseNPC>::dataDesc[6].flatOffset + unaff_EBX + -0x22) = iVar4;
  *(undefined **)((int)&ai_no_select_box.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 2) =
       &UNK_00c1ecfa + unaff_EBX;
  pIVar5 = EntityFactoryDictionary();
  (**pIVar5->_vptr_IEntityFactoryDictionary)(pIVar5,unaff_EBX + 0xd98e2e,unaff_EBX + 0x9a8738);
  ConCommand::ConCommand
            ((ConCommand *)
             ((int)&ai_show_think_tolerance.super_ConCommandBase._vptr_ConCommandBase +
             unaff_EBX + 2),&UNK_009fdc6f + unaff_EBX,(FnCommandCallback_t)(unaff_EBX + 0x6abb92),
             (char *)(unaff_EBX + 0x9fdc3a),0x4000,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x988dd2,0,*(undefined4 *)(&DAT_00b8f596 + unaff_EBX));
  ConCommand::ConCommand
            ((ConCommand *)((int)&ai_show_think_tolerance.m_fMaxVal + unaff_EBX + 2),
             (char *)(unaff_EBX + 0x9fdcc0),(FnCommandCallback_t)(unaff_EBX + 0x6abb82),
             &UNK_009fdc8a + unaff_EBX,0x4000,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x988db2,0,*(undefined4 *)(&DAT_00b8f596 + unaff_EBX));
  ConCommand::ConCommand
            ((ConCommand *)((int)&ai_debug_think_ticks.m_pszDefaultValue + unaff_EBX + 2),
             (char *)(unaff_EBX + 0x9fdd14),(FnCommandCallback_t)(unaff_EBX + 0x6abb72),
             (char *)(unaff_EBX + 0x9fdcde),0x4000,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x988d92,0,*(undefined4 *)(&DAT_00b8f596 + unaff_EBX));
  ConCommand::ConCommand
            ((ConCommand *)
             ((int)&ai_debug_doors.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX + 2),
             (char *)(unaff_EBX + 0x9fdd69),(FnCommandCallback_t)(unaff_EBX + 0x6abb62),
             (char *)(unaff_EBX + 0x9fdd32),0x4000,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x988d72,0,*(undefined4 *)(&DAT_00b8f596 + unaff_EBX));
  ConCommand::ConCommand
            ((ConCommand *)((int)&ai_debug_doors.m_fMaxVal + unaff_EBX + 2),
             &UNK_009fddbc + unaff_EBX,(FnCommandCallback_t)(unaff_EBX + 0x6abb52),
             (char *)(unaff_EBX + 0x9fdd86),0x4000,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x988d52,0,*(undefined4 *)(&DAT_00b8f596 + unaff_EBX));
  return;
}


/* __tcf_2 at 00a404f0 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x389ac8),in_stack_00000008);
  return;
}


/* __tcf_3 at 00a404d0 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x389b48),in_stack_00000008);
  return;
}


/* __tcf_4 at 00a404b0 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x389bc8),in_stack_00000008);
  return;
}


/* __tcf_5 at 00a40490 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x389c48),in_stack_00000008);
  return;
}


/* __tcf_6 at 00a40470 */

void __tcf_6(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x389cc8),in_stack_00000008);
  return;
}


/* __tcf_7 at 00a40450 */

void __tcf_7(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x389d48),in_stack_00000008);
  return;
}


/* __tcf_8 at 00a40430 */

void __tcf_8(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x389dc8),in_stack_00000008);
  return;
}


/* __tcf_9 at 00a40410 */

void __tcf_9(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x389e48),in_stack_00000008);
  return;
}


/* __tcf_10 at 00a403f0 */

void __tcf_10(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x389ec8),in_stack_00000008);
  return;
}


/* __tcf_11 at 00a403d0 */

void __tcf_11(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x389f48),in_stack_00000008);
  return;
}


/* DataMapInit<CPropPaintBomb> at 000b7480 */

datamap_t * DataMapInit<CPropPaintBomb>(CPropPaintBomb *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&ai_debug_efficiency.super_ConCommandBase.m_pszName + unaff_EBX) == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd991ac);
    if (iVar2 != 0) {
      *(int *)((int)&ai_debug_efficiency.m_pParent + unaff_EBX) = unaff_EBX + 0x9fdbbb;
      *(undefined4 *)((int)&ai_debug_efficiency.m_Value.m_pszString + unaff_EBX) = 0;
      *(undefined4 *)((int)&ai_debug_efficiency.m_Value.m_StringLength + unaff_EBX) = 0;
      *(undefined4 *)((int)&ai_debug_efficiency.m_Value.m_fValue + unaff_EBX) = 0;
      *(undefined4 *)((int)&ai_debug_efficiency.m_Value.m_nValue + unaff_EBX) = 0;
      *(undefined4 *)(&ai_debug_efficiency.m_bHasMin + unaff_EBX) = 0;
      *(undefined4 *)((int)&ai_debug_efficiency.m_pszDefaultValue + unaff_EBX) = 0xe;
      ___cxa_guard_release(unaff_EBX + 0xd991ac);
      ___cxa_atexit(unaff_EBX + 0x98907c,0,*(undefined4 *)(&DAT_00b8f6c0 + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_MyCombatCharacterPointer_00cb2c58 + unaff_EBX) =
       *(undefined4 *)(&DAT_00b900d0 + unaff_EBX);
  if (*(char *)((int)&ai_debug_efficiency.super_ConCommandBase.m_nFlags + unaff_EBX) == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd991b4);
    if (iVar2 != 0) {
      uVar1 = **(undefined4 **)(&DAT_00b8f6e0 + unaff_EBX);
      *(undefined4 *)((int)&PTR_VPhysicsCollision_00cb2dc0 + unaff_EBX) = uVar1;
      *(undefined4 *)((int)&PTR_NetworkStateChanged_m_nWaterLevel_00cb2e00 + unaff_EBX) = uVar1;
      ___cxa_guard_release(unaff_EBX + 0xd991b4);
    }
  }
  *(undefined4 *)((int)&PTR_IsNPC_00cb2c50 + unaff_EBX) = 6;
  *(int *)((int)&PTR_IsTriggered_00cb2c4c + unaff_EBX) = (int)&PTR_CanStandOn_00cb2cac + unaff_EBX;
  return (datamap_t *)((int)&PTR_IsTriggered_00cb2c4c + unaff_EBX);
}


/* CPropPaintBomb::GetDataDescMap at 007609c0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPropPaintBomb * this) */

datamap_t * __thiscall CPropPaintBomb::GetDataDescMap(CPropPaintBomb *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x609718);
}


/* CPropPaintBomb::GetBaseMap at 007609d0 */

datamap_t * CPropPaintBomb::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4e6b8c);
}


/* __tcf_0 at 00a40510 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x410146)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41013a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x410146));
  }
  *(undefined4 *)(unaff_EBX + 0x410146) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x410142)) {
    if (*(int *)(unaff_EBX + 0x41013a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20663e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20663e),*(int *)(unaff_EBX + 0x41013a));
      *(undefined4 *)(unaff_EBX + 0x41013a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41013e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41013a);
  *(int *)(unaff_EBX + 0x41014a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x410142)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20663e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20663e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41013a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41013e) = 0;
  }
  return;
}


/* ServerClassInit<DT_PropPaintBomb::ignored> at 000b72a0 */

int ServerClassInit<DT_PropPaintBomb::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&ai_frametime_limit.super_ConCommandBase.m_nFlags + unaff_EBX) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xd99274);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)((int)&ai_frametime_limit.m_Value.m_fValue + unaff_EBX),
                  (char *)(unaff_EBX + 0x99e980),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&ai_use_think_optimizations.m_pszDefaultValue + unaff_EBX),
                        (char *)(unaff_EBX + 0x9a0d30),0,
                        (SendTable *)**(undefined4 **)(&DAT_00b902b4 + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00b8f93c + unaff_EBX),0x80);
      SendPropInt((SendProp_conflict *)
                  ((int)&ai_test_moveprobe_ignoresmall.super_ConCommandBase.m_nFlags + unaff_EBX),
                  (char *)(unaff_EBX + 0x9fdd89),0x798,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      ___cxa_guard_release(unaff_EBX + 0xd99274);
      ___cxa_atexit(unaff_EBX + 0x98900c,0,*(undefined4 *)(&DAT_00b8f8a0 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)
             ((int)DataMapInit<CAI_BaseNPC>::dataDesc[0x12].flatOffset + unaff_EBX + -0x2c),
             (SendProp_conflict *)((int)&ai_use_think_optimizations.m_pszDefaultValue + unaff_EBX),2
             ,*(char **)(&DAT_00c1eff0 + unaff_EBX));
  return 1;
}


/* CPropPaintBomb::GetServerClass at 007609e0 */

/* DWARF original prototype: ServerClass * GetServerClass(CPropPaintBomb * this) */

ServerClass * __thiscall CPropPaintBomb::GetServerClass(CPropPaintBomb *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x6ef9f0);
}


/* CPropPaintBomb::YouForgotToImplementOrDeclareServerClass at 007609f0 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CPropPaintBomb * this) */

int __thiscall CPropPaintBomb::YouForgotToImplementOrDeclareServerClass(CPropPaintBomb *this)

{
  return 0;
}


/* __tcf_12 at 00a403b0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */

void __tcf_12(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable
            ((SendTable *)(CAI_Navigator::IgnoreStoppingPath + extraout_ECX + 4),in_stack_00000008);
  return;
}


/* __tcf_1 at 00a402c0 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410317))(unaff_EBX + 0x410317);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x4102c3))(unaff_EBX + 0x4102c3);
                    /* WARNING: Could not recover jumptable at 0x00a40307. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41026f))();
  return;
}


/* CPropPaintBomb::CPropPaintBomb at 00760f70 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CPropPaintBomb(CPropPaintBomb * this) */

void __thiscall CPropPaintBomb::CPropPaintBomb(CPropPaintBomb *this)

{
  uint *puVar1;
  uint uVar2;
  int iVar3;
  ushort uVar4;
  CBaseEdict *this_00;
  int *piVar5;
  ushort *puVar6;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  uint uVar9;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CPhysicsProp::CPhysicsProp(&this->super_CPhysicsProp);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
  super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x574de4);
  (this->super_CPhysicsProp).super_CBreakableProp.super_IBreakableWithPropData.
  _vptr_IBreakableWithPropData = (_func_int_varargs **)(unaff_EBX + 0x57524c);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x5752d0);
  (this->super_CPhysicsProp).super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(unaff_EBX + 0x575300);
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
    if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
      this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
      pIVar7 = CBaseEdict::GetChangeAccessor(this_00);
      piVar5 = *(int **)(unaff_EBX + 0x4e5be4);
      puVar6 = (ushort *)*piVar5;
      if (pIVar7->m_iChangeInfoSerialNumber == *puVar6) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar4 = puVar6[uVar8 * 0x14 + 0x14];
        if (uVar4 == 0) {
LAB_00761114:
          puVar6[(uint)uVar4 + uVar8 * 0x14 + 1] = 0x798;
          puVar6[uVar8 * 0x14 + 0x14] = uVar4 + 1;
        }
        else if (puVar6[uVar8 * 0x14 + 1] != 0x798) {
          uVar9 = 0;
          do {
            uVar2 = uVar9 + 1;
            uVar9 = uVar2 & 0xffff;
            if ((ushort)uVar2 == uVar4) {
              if (uVar4 != 0x13) goto LAB_00761114;
              goto LAB_00761132;
            }
          } while (puVar6[uVar8 * 0x14 + uVar9 + 1] != 0x798);
        }
      }
      else if (puVar6[0x7d1] == 100) {
LAB_00761132:
        pIVar7->m_iChangeInfoSerialNumber = 0;
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar7->m_iChangeInfo = puVar6[0x7d1];
        *(short *)(*piVar5 + 0xfa2) = *(short *)(*piVar5 + 0xfa2) + 1;
        pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
        iVar3 = *piVar5 + (uint)pIVar7->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar3 + 2) = 0x798;
        *(undefined2 *)(iVar3 + 0x28) = 1;
      }
    }
  }
  else {
    puVar1 = (uint *)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                      super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
    *puVar1 = *puVar1 | 1;
  }
  return;
}


/* CPropPaintBomb::CPropPaintBomb at 00761180 */

/* DWARF original prototype: void CPropPaintBomb(CPropPaintBomb * this, CPropPaintBomb * this) */

void __thiscall CPropPaintBomb::CPropPaintBomb(CPropPaintBomb *this,CPropPaintBomb *this_1)

{
  CPropPaintBomb(this);
  return;
}


/* CEntityFactory<CPropPaintBomb>::Create at 007631c0 */

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


/* CPropPaintBomb::~CPropPaintBomb at 00760d80 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropPaintBomb(CPropPaintBomb * this, int __in_chrg) */

void __thiscall CPropPaintBomb::~CPropPaintBomb(CPropPaintBomb *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
  super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(&UNK_00574fdb + unaff_EBX);
  (this->super_CPhysicsProp).super_CBreakableProp.super_IBreakableWithPropData.
  _vptr_IBreakableWithPropData = (_func_int_varargs **)(unaff_EBX + 0x575443);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x5754c7);
  (this->super_CPhysicsProp).super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(unaff_EBX + 0x5754f7);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnExploded).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFizzled).super_CBaseEntityOutput,in_stack_ffffffe8);
  CPhysicsProp::~CPhysicsProp(&this->super_CPhysicsProp,__in_chrg);
  return;
}


/* CPropPaintBomb::~CPropPaintBomb at 00760e20 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropPaintBomb(CPropPaintBomb * this, int __in_chrg) */

void __thiscall CPropPaintBomb::~CPropPaintBomb(CPropPaintBomb *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
  super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x574f3b);
  (this->super_CPhysicsProp).super_CBreakableProp.super_IBreakableWithPropData.
  _vptr_IBreakableWithPropData = (_func_int_varargs **)(unaff_EBX + 0x5753a3);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x575427);
  (this->super_CPhysicsProp).super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(unaff_EBX + 0x575457);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnExploded).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFizzled).super_CBaseEntityOutput,in_stack_ffffffe8);
  CPhysicsProp::~CPhysicsProp(&this->super_CPhysicsProp,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CPropPaintBomb::~CPropPaintBomb at 00760ed0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropPaintBomb(CPropPaintBomb * this, int __in_chrg) */

void __thiscall CPropPaintBomb::~CPropPaintBomb(CPropPaintBomb *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
  super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x574e8b);
  (this->super_CPhysicsProp).super_CBreakableProp.super_IBreakableWithPropData.
  _vptr_IBreakableWithPropData = (_func_int_varargs **)(unaff_EBX + 0x5752f3);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x575377);
  (this->super_CPhysicsProp).super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(unaff_EBX + 0x5753a7);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnExploded).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFizzled).super_CBaseEntityOutput,in_stack_ffffffe8);
  CPhysicsProp::~CPhysicsProp(&this->super_CPhysicsProp,__in_chrg);
  return;
}


/* CPropPaintBomb::Precache at 00760cd0 */

/* DWARF original prototype: void Precache(CPropPaintBomb * this) */

void __thiscall CPropPaintBomb::Precache(CPropPaintBomb *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x35281f));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x3542e8));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x354306));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x354324));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x354342));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x354293));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x3542a3));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x3542b5));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x3542c6));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x3542d7));
  CPhysicsProp::Precache(&this->super_CPhysicsProp);
  return;
}


/* CPropPaintBomb::Spawn at 00761190 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CPropPaintBomb * this) */

void __thiscall CPropPaintBomb::Spawn(CPropPaintBomb *this)

{
  undefined1 *puVar1;
  int iVar2;
  ushort uVar3;
  CBaseEdict *this_00;
  IPhysicsObject *pIVar4;
  ushort *puVar5;
  int *piVar6;
  undefined4 uVar7;
  IChangeInfoAccessor *pIVar8;
  uint uVar9;
  uint uVar10;
  uint uVar11;
  int unaff_EBX;
  uint local_60;
  
                    /* Unresolved local var: int nPaintBombSurfaceIndex@[???]
                       Unresolved local var: IPhysicsObject * pPhysObject@[???] */
  ___i686_get_pc_thunk_bx();
  (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x68))(this);
  (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x6c))(this,unaff_EBX + 0x35235e);
  CBaseEntity::AddEffects((CBaseEntity *)this,0x20);
  uVar10 = (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
           super_CBaseEntity.m_spawnflags.m_Value;
  uVar11 = uVar10 | 0x200;
  if (uVar10 != uVar11) {
    if ((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                  super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar8 = CBaseEdict::GetChangeAccessor(this_00);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4e59ca);
        if (pIVar8->m_iChangeInfoSerialNumber == *puVar5) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar10 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar10 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00761368:
            puVar5[(uint)uVar3 + uVar10 * 0x14 + 1] = 0x128;
            puVar5[uVar10 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar10 * 0x14 + 1] != 0x128) {
            local_60 = 0;
            do {
              uVar9 = local_60 + 1;
              local_60 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 == 0x13) goto LAB_007612da;
                goto LAB_00761368;
              }
            } while (puVar5[uVar10 * 0x14 + local_60 + 1] != 0x128);
          }
        }
        else if (puVar5[0x7d1] == 100) {
LAB_007612da:
          pIVar8->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar8->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4e59ca) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4e59ca) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x4e59ca);
          pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar8->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x128;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating
                .super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
    super_CBaseEntity.m_spawnflags.m_Value = uVar11;
  }
  CPhysicsProp::Spawn(&this->super_CPhysicsProp);
  uVar7 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e5a76) + 0x10))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x4e5a76),unaff_EBX + 0x3541f8);
  pIVar4 = (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
           super_CBaseEntity.m_pPhysicsObject;
  if (pIVar4 != (IPhysicsObject *)0x0) {
    (*pIVar4->_vptr_IPhysicsObject[0x28])(pIVar4,uVar7);
  }
  return;
}


/* CPropPaintBomb::UpdateTransmitState at 00760cb0 */

/* DWARF original prototype: int UpdateTransmitState(CPropPaintBomb * this) */

int __thiscall CPropPaintBomb::UpdateTransmitState(CPropPaintBomb *this)

{
  int iVar1;
  
  iVar1 = CBaseEntity::SetTransmitState((CBaseEntity *)this,0x20);
  return iVar1;
}


/* CPropPaintBomb::SetPaintPowerType at 00762690 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void SetPaintPowerType(CPropPaintBomb * this, PaintPowerType paintType)
    */

void __thiscall CPropPaintBomb::SetPaintPowerType(CPropPaintBomb *this,PaintPowerType paintType)

{
  uint *puVar1;
  int iVar2;
  ushort uVar3;
  CBaseEdict *pCVar4;
  ushort *puVar5;
  int *piVar6;
  uint8 uVar7;
  uint8 uVar8;
  Color *pCVar9;
  IChangeInfoAccessor *pIVar10;
  uint uVar11;
  uint uVar12;
  int unaff_EBX;
  int in_stack_ffffff28;
  uint local_cc;
  uint local_c8;
  uint local_c4;
  uint local_c0;
  
                    /* Unresolved local var: Color color@[???] */
  ___i686_get_pc_thunk_bx();
  if (paintType != (this->m_nPaintPowerType).m_Value) {
    if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
        false) {
      pCVar4 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                 super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4e44c7);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar5) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar12 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar12 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00762d7a:
            puVar5[(uint)uVar3 + uVar12 * 0x14 + 1] = 0x798;
            puVar5[uVar12 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar12 * 0x14 + 1] != 0x798) {
            local_cc = 0;
            do {
              uVar11 = local_cc + 1;
              local_cc = uVar11 & 0xffff;
              if ((ushort)uVar11 == uVar3) {
                if (uVar3 == 0x13) goto LAB_00762a63;
                goto LAB_00762d7a;
              }
            } while (puVar5[uVar12 * 0x14 + local_cc + 1] != 0x798);
          }
        }
        else if (puVar5[0x7d1] == 100) {
LAB_00762a63:
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4e44c7) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4e44c7) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x4e44c7);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x798;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = (uint *)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
      *puVar1 = *puVar1 | 1;
    }
    (this->m_nPaintPowerType).m_Value = paintType;
  }
  pCVar9 = MapPowerToVisualColor((Color *)(this->m_nPaintPowerType).m_Value,in_stack_ffffff28);
  uVar7 = (uint8)((uint)pCVar9 >> 8);
  uVar8 = (uint8)((uint)pCVar9 >> 0x10);
  if ((uint8)pCVar9 !=
      (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
      super_CBaseEntity.m_clrRender.
      super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r) {
    if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
        false) {
      pCVar4 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                 super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4e44c7);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar5) {
          uVar12 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar12 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00762bfa:
            puVar5[(uint)uVar3 + uVar12 * 0x14 + 1] = 0x108;
            puVar5[uVar12 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar12 * 0x14 + 1] != 0x108) {
            local_c8 = 0;
            do {
              uVar11 = local_c8 + 1;
              local_c8 = uVar11 & 0xffff;
              if ((ushort)uVar11 == uVar3) {
                if (uVar3 != 0x13) goto LAB_00762bfa;
                pIVar10->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar12 * 0x14 + local_c8 + 1] != 0x108);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4e44c7) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4e44c7) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x4e44c7);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x108;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = (uint *)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
      *puVar1 = *puVar1 | 1;
    }
    (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
    super_CBaseEntity.m_clrRender.
    super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r = (uint8)pCVar9;
  }
  if (uVar7 != *(uint8 *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                                super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                                super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>
                         + 1)) {
    if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
        false) {
      pCVar4 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                 super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4e44c7);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar5) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar12 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar12 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00762cba:
            puVar5[(uint)uVar3 + uVar12 * 0x14 + 1] = 0x108;
            puVar5[uVar12 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar12 * 0x14 + 1] != 0x108) {
            local_c4 = 0;
            do {
              uVar11 = local_c4 + 1;
              local_c4 = uVar11 & 0xffff;
              if ((ushort)uVar11 == uVar3) {
                if (uVar3 == 0x13) goto LAB_00762a4f;
                goto LAB_00762cba;
              }
            } while (puVar5[uVar12 * 0x14 + local_c4 + 1] != 0x108);
          }
        }
        else if (puVar5[0x7d1] == 100) {
LAB_00762a4f:
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4e44c7) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4e44c7) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x4e44c7);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x108;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
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
         uVar7;
  }
  if (uVar8 != *(uint8 *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                                super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                                super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>
                         + 2)) {
    if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
        false) {
      pCVar4 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                 super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4e44c7);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar5) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar12 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar12 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00762b23:
            puVar5[(uint)uVar3 + uVar12 * 0x14 + 1] = 0x108;
            puVar5[uVar12 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar12 * 0x14 + 1] != 0x108) {
            local_c0 = 0;
            do {
              uVar11 = local_c0 + 1;
              local_c0 = uVar11 & 0xffff;
              if ((ushort)uVar11 == uVar3) {
                if (uVar3 == 0x13) goto LAB_00762b4b;
                goto LAB_00762b23;
              }
            } while (puVar5[uVar12 * 0x14 + local_c0 + 1] != 0x108);
          }
        }
        else if (puVar5[0x7d1] == 100) {
LAB_00762b4b:
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4e44c7) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4e44c7) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x4e44c7);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x108;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
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
         uVar8;
  }
  return;
}


/* GenerateBombDirections at 00761390 */

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
  if ((0 < *(int *)(*(int *)(&DAT_00668edb + unaff_EBX) + 0x30)) &&
     (0 < *(int *)(*(int *)(unaff_EBX + 0x668f3b) + 0x30))) {
    directions->m_Size = 0;
    if ((directions->m_Memory).m_nGrowSize < 0) {
      pVVar6 = (directions->m_Memory).m_pMemory;
    }
    else {
      pVVar6 = (directions->m_Memory).m_pMemory;
      if (pVVar6 != (Vector *)0x0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e57bb) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x4e57bb),pVVar6);
        (directions->m_Memory).m_pMemory = (Vector *)0x0;
      }
      pVVar6 = (Vector *)0x0;
      (directions->m_Memory).m_nAllocationCount = 0;
    }
    directions->m_pElements = pVVar6;
    fVar14 = *(float *)(*(int *)(unaff_EBX + 0x668f3b) + 0x2c);
    local_28 = 0;
    local_24 = 0;
    local_10c = *(float *)(&DAT_0036b56f + unaff_EBX);
    local_108 = 0.0;
    local_104 = 0.0;
    local_34.x = 0.0;
    local_34.z = 0.0;
    local_34.y = local_10c;
    local_20 = local_10c;
    MatrixBuildRotationAboutAxis
              (&local_d4,&local_34,
               *(float *)(unaff_EBX + 0x36b93b) /
               *(float *)(*(int *)(&DAT_00668edb + unaff_EBX) + 0x2c));
    iVar4 = *(int *)(*(int *)(&DAT_00668edb + unaff_EBX) + 0x30);
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
        puVar1 = (uint *)(unaff_EBX + 0x374a3f);
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
    MatrixBuildRotateZ(&local_d4,*(float *)(unaff_EBX + 0x36b93b) / fVar14);
    if (1 < *(int *)(*(int *)(&DAT_00668edb + unaff_EBX) + 0x30)) {
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
      } while (local_110 < *(int *)(*(int *)(&DAT_00668edb + unaff_EBX) + 0x30));
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


/* DispatchDryBombParticleEffect at 00760b40 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void DispatchDryBombParticleEffect(Vector *pos,PaintPowerType paintType,QAngle angSplash)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (paintType < PAINT_POWER_TYPE_COUNT_PLUS_NO_POWER) {
                    /* WARNING: Could not recover jumptable at 0x00760b60. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*(code *)(*(int *)(unaff_EBX + 0x18 + paintType * 4) + unaff_EBX))();
    return;
  }
  return;
}


/* CreatePaintBombExplosion at 00761c80 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

void CreatePaintBombExplosion(PaintPowerType paintType,Vector *vecExplosionPos,bool bSpawnBlobs)

{
  uint *puVar1;
  float *pfVar2;
  float fVar3;
  undefined4 *puVar4;
  QAngle angSplash;
  vec_t vVar5;
  vec_t vVar6;
  vec_t vVar7;
  char cVar8;
  CPortal_Base2D *pCVar9;
  CBasePlayer *pPaintedEntity;
  Vector *vecPosition;
  ITraceFilter IVar10;
  int unaff_EBX;
  int iVar11;
  int iVar12;
  int local_1b0;
  int local_1a0;
  trace_t local_190;
  Ray_t local_13c;
  CTraceFilterSimpleClassnameList local_e4;
  CUtlVector<Vector,CUtlMemory<Vector,_int>_> local_c0;
  vec_t local_ac;
  vec_t local_a8;
  vec_t local_a4;
  float local_a0;
  float local_9c;
  float local_98;
  Vector local_94;
  float local_88;
  float local_84;
  float local_80;
  Vector local_7c;
  float local_70;
  float local_6c;
  float local_68;
  Vector local_64;
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
  local_c0.m_Memory.m_pMemory = (Vector *)0x0;
  local_c0.m_Memory.m_nAllocationCount = 0;
  local_c0.m_Memory.m_nGrowSize = 0;
  local_c0.m_Size = 0;
  local_c0.m_pElements = (Vector *)0x0;
  local_34.x = 0.0;
  local_34.y = 0.0;
  local_34.z = 1.0;
  GenerateBombDirections(&local_c0);
  fVar3 = *(float *)(*(int *)(unaff_EBX + 0x66834b) + 0x2c);
  if (0 < local_c0.m_Size) {
    local_1b0 = 0;
    local_1a0 = 0;
    IVar10._vptr_ITraceFilter = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4e50f7) + 8);
    puVar4 = *(undefined4 **)(unaff_EBX + 0x4e4ecb);
    do {
                    /* Unresolved local var: Vector norm@[???] */
      local_40.x = *(float *)((int)&(local_c0.m_Memory.m_pMemory)->x + local_1a0);
      local_40.y = *(float *)((int)&(local_c0.m_Memory.m_pMemory)->y + local_1a0);
      local_40.z = *(float *)((int)&(local_c0.m_Memory.m_pMemory)->z + local_1a0);
      VectorNormalize(&local_40);
      vVar7 = local_40.z;
      vVar6 = local_40.y;
      vVar5 = local_40.x;
      CTraceFilterSimpleClassnameList::CTraceFilterSimpleClassnameList
                (&local_e4,(IHandleEntity *)0x0,0);
      CTraceFilterSimpleClassnameList::AddClassnameToIgnore
                (&local_e4,(char *)(unaff_EBX + 0x2fe065));
                    /* Unresolved local var: Vector res@[???] */
      local_a0 = fVar3 * vVar5;
      local_9c = fVar3 * vVar6;
      local_98 = vVar7 * fVar3;
                    /* Unresolved local var: Vector res@[???] */
      local_13c.m_Start.super_Vector.x = vecExplosionPos->x;
      local_58 = local_13c.m_Start.super_Vector.x + local_a0;
      local_13c.m_Start.super_Vector.y = vecExplosionPos->y;
      local_54 = local_13c.m_Start.super_Vector.y + local_9c;
      local_13c.m_Start.super_Vector.z = vecExplosionPos->z;
      local_50 = local_13c.m_Start.super_Vector.z + local_98;
      local_13c.m_Delta.super_Vector.x = local_58 - vecExplosionPos->x;
      local_13c.m_Delta.super_Vector.y = local_54 - vecExplosionPos->y;
      local_13c.m_Delta.super_Vector.z = local_50 - vecExplosionPos->z;
      local_13c.m_IsSwept =
           local_13c.m_Delta.super_Vector.x * local_13c.m_Delta.super_Vector.x +
           local_13c.m_Delta.super_Vector.y * local_13c.m_Delta.super_Vector.y +
           local_13c.m_Delta.super_Vector.z * local_13c.m_Delta.super_Vector.z != 0.0;
      local_13c.m_Extents.super_Vector.z = 0.0;
      local_13c.m_Extents.super_Vector.y = 0.0;
      local_13c.m_Extents.super_Vector.x = 0.0;
      local_13c.m_pWorldAxisTransform = (matrix3x4_t *)0x0;
      local_13c.m_IsRay = true;
      local_13c.m_StartOffset.super_Vector.z = 0.0;
      local_13c.m_StartOffset.super_Vector.y = 0.0;
      local_13c.m_StartOffset.super_Vector.x = 0.0;
      local_4c = local_a0;
      local_48 = local_9c;
      local_44 = local_98;
      pCVar9 = UTIL_Portal_TraceRay(&local_13c,0x46004003,(ITraceFilter *)&local_e4,&local_190,true)
      ;
      if (((local_190.super_CBaseTrace.fraction < *(float *)(unaff_EBX + 0x36ac7f)) &&
          (local_190.m_pEnt != (CBaseEntity *)0x0)) && (pCVar9 == (CPortal_Base2D *)0x0)) {
        CPaintDatabase::AddPaint(*(CPaintDatabase **)(unaff_EBX + 0x4e5253),&local_190,paintType);
        puVar1 = (uint *)(unaff_EBX + 0x37414f);
        local_a4 = (vec_t)((uint)local_190.super_CBaseTrace.plane.normal.x ^ *puVar1);
        local_a8 = (vec_t)((uint)local_190.super_CBaseTrace.plane.normal.y ^ *puVar1);
        local_ac = (vec_t)((uint)local_190.super_CBaseTrace.plane.normal.z ^ *puVar1);
        local_64.x = local_ac;
        local_64.y = local_a8;
        local_64.z = local_a4;
        VectorAngles(&local_64,&local_34);
      }
      local_e4.m_PassClassnames.m_Size = 0;
      local_e4.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter =
           (ITraceFilter)(ITraceFilter)IVar10._vptr_ITraceFilter;
      if (-1 < local_e4.m_PassClassnames.m_Memory.m_nGrowSize) {
        if (local_e4.m_PassClassnames.m_Memory.m_pMemory != (char **)0x0) {
          (**(code **)(*(int *)*puVar4 + 8))
                    ((int *)*puVar4,local_e4.m_PassClassnames.m_Memory.m_pMemory);
          local_e4.m_PassClassnames.m_Memory.m_pMemory = (char **)0x0;
        }
        local_e4.m_PassClassnames.m_Memory.m_nAllocationCount = 0;
      }
      local_e4.m_PassClassnames.m_pElements = local_e4.m_PassClassnames.m_Memory.m_pMemory;
      if (-1 < local_e4.m_PassClassnames.m_Memory.m_nGrowSize) {
        if (local_e4.m_PassClassnames.m_Memory.m_pMemory != (char **)0x0) {
          (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e4ecb) + 8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x4e4ecb),
                     local_e4.m_PassClassnames.m_Memory.m_pMemory);
          local_e4.m_PassClassnames.m_Memory.m_pMemory = (char **)0x0;
        }
        local_e4.m_PassClassnames.m_Memory.m_nAllocationCount = 0;
      }
      local_1b0 = local_1b0 + 1;
      local_1a0 = local_1a0 + 0xc;
    } while (local_1b0 < local_c0.m_Size);
  }
                    /* Unresolved local var: int i@[???] */
  if (((*(int *)(*(int *)(*(int *)(unaff_EBX + 0x4e5103) + 0x1c) + 0x30) != 0) ||
      (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x4e50ff) + 0x1c) + 0x30) != 0)) &&
     (0 < *(int *)(**(int **)(unaff_EBX + 0x4e4ed3) + 0x14))) {
    iVar11 = 1;
    do {
      while (((pPaintedEntity = UTIL_PlayerByIndex(iVar11), pPaintedEntity == (CBasePlayer *)0x0 ||
              (cVar8 = (**(code **)((int)(pPaintedEntity->super_CBaseCombatCharacter).
                                         super_CBaseFlex.super_CBaseAnimatingOverlay.
                                         super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
                                         super_IServerUnknown + 0x154))(pPaintedEntity),
              cVar8 == '\0')) || (pPaintedEntity->m_iConnected == PlayerDisconnected))) {
LAB_007620df:
        iVar11 = iVar11 + 1;
        if (*(int *)(**(int **)(unaff_EBX + 0x4e4ed3) + 0x14) < iVar11) goto LAB_007621a4;
      }
      vecPosition = (Vector *)
                    (**(code **)((int)(pPaintedEntity->super_CBaseCombatCharacter).super_CBaseFlex.
                                      super_CBaseAnimatingOverlay.super_CBaseAnimating.
                                      super_CBaseEntity.super_IServerEntity.super_IServerUnknown +
                                0x274))(pPaintedEntity);
                    /* Unresolved local var: Vector res@[???] */
      local_70 = vecExplosionPos->x - vecPosition->x;
      local_6c = vecExplosionPos->y - vecPosition->y;
      local_68 = vecExplosionPos->z - vecPosition->z;
      if (fVar3 * fVar3 <= local_70 * local_70 + local_6c * local_6c + local_68 * local_68)
      goto LAB_007620df;
      CPaintDatabase::PaintEntity
                (*(CPaintDatabase **)(unaff_EBX + 0x4e5253),(CBaseEntity *)pPaintedEntity,paintType,
                 vecPosition);
      iVar11 = iVar11 + 1;
    } while (iVar11 <= *(int *)(**(int **)(unaff_EBX + 0x4e4ed3) + 0x14));
  }
LAB_007621a4:
  angSplash.y = local_34.y;
  angSplash.x = local_34.x;
  angSplash.z = local_34.z;
  DispatchDryBombParticleEffect(vecExplosionPos,paintType,angSplash);
                    /* Unresolved local var: int i@[???] */
  if ((*(int *)(*(int *)(unaff_EBX + 0x6686ab) + 0x30) != 0) && (0 < local_c0.m_Size)) {
    iVar12 = 0;
    iVar11 = 0;
                    /* Unresolved local var: Vector vecBlobFireDir@[???] */
    do {
                    /* Unresolved local var: Vector norm@[???] */
      local_7c.x = *(float *)((int)&(local_c0.m_Memory.m_pMemory)->x + iVar11);
      local_7c.y = *(float *)((int)&(local_c0.m_Memory.m_pMemory)->y + iVar11);
      local_7c.z = *(float *)((int)&(local_c0.m_Memory.m_pMemory)->z + iVar11);
      VectorNormalize(&local_7c);
                    /* Unresolved local var: Vector res@[???] */
      pfVar2 = (float *)(unaff_EBX + 0x36bc53);
      local_88 = *pfVar2 * local_7c.x;
      local_84 = local_7c.y * *pfVar2;
      local_80 = local_7c.z * *pfVar2;
                    /* Unresolved local var: Vector res@[???] */
      local_94.x = vecExplosionPos->x + local_88;
      local_94.y = vecExplosionPos->y + local_84;
      local_94.z = vecExplosionPos->z + local_80;
      NDebugOverlay::Line(vecExplosionPos,&local_94,0,0xff,0,false,5.0);
      iVar12 = iVar12 + 1;
      iVar11 = iVar11 + 0xc;
    } while (iVar12 < local_c0.m_Size);
  }
  local_c0.m_Size = 0;
  if (-1 < local_c0.m_Memory.m_nGrowSize) {
    if (local_c0.m_Memory.m_pMemory != (Vector *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e4ecb) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4e4ecb),local_c0.m_Memory.m_pMemory);
      local_c0.m_Memory.m_pMemory = (Vector *)0x0;
    }
    local_c0.m_Memory.m_nAllocationCount = 0;
  }
  if ((-1 < local_c0.m_Memory.m_nGrowSize) && (local_c0.m_Memory.m_pMemory != (Vector *)0x0)) {
    local_c0.m_pElements = local_c0.m_Memory.m_pMemory;
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e4ecb) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4e4ecb),local_c0.m_Memory.m_pMemory);
  }
  return;
}


/* CPropPaintBomb::Event_Killed at 00762550 */

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
     (iVar4 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e460e),
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
      (iVar4 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e460e),
      *(uint *)(iVar4 + 8) != uVar1 >> 0x10)) ||
     (this_00 = *(CBaseEntity **)(iVar4 + 4), this_00 == (CBaseEntity *)0x0)) {
LAB_007625eb:
    bVar2 = 0;
  }
  else {
    local_20 = &UNK_00352e42 + unaff_EBX;
    if ((this_00->m_iClassname).pszValue != local_20) {
      bVar3 = CBaseEntity::ClassMatchesComplex(this_00,local_20);
      if (!bVar3) goto LAB_007625eb;
    }
    bVar2 = 1;
  }
  CreatePaintBombExplosion((this->m_nPaintPowerType).m_Value,&local_2c,(bool)(bVar2 ^ 1));
  COutputEvent::FireOutput(&this->m_OnExploded,(CBaseEntity *)this,(CBaseEntity *)this,0.0);
  return;
}


/* CPropPaintBomb::CleansePaintPower at 00763160 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void CleansePaintPower(CPropPaintBomb * this) */

void __thiscall CPropPaintBomb::CleansePaintPower(CPropPaintBomb *this)

{
  SetPaintPowerType(this,NO_POWER);
  return;
}


/* CPropPaintBomb::Break at 00760af0 */

/* DWARF original prototype: void Break(CPropPaintBomb * this, CBaseEntity * pBreaker,
   CTakeDamageInfo * info) */

void __thiscall
CPropPaintBomb::Break(CPropPaintBomb *this,CBaseEntity *pBreaker,CTakeDamageInfo *info)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x35446f),0.0,(float *)0x0);
  UTIL_Remove((CBaseEntity *)this);
  return;
}


/* CPropPaintBomb::OnFizzled at 00760ac0 */

/* DWARF original prototype: void OnFizzled(CPropPaintBomb * this) */

void __thiscall CPropPaintBomb::OnFizzled(CPropPaintBomb *this)

{
  COutputEvent::FireOutput(&this->m_OnFizzled,(CBaseEntity *)this,(CBaseEntity *)this,0.0);
  return;
}


/* CPropPaintBomb::InputDissolve at 00760a00 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void InputDissolve(CPropPaintBomb * this, inputdata_t * in) */

void __thiscall CPropPaintBomb::InputDissolve(CPropPaintBomb *this,inputdata_t *in)

{
  undefined4 *puVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  puVar1 = *(undefined4 **)(unaff_EBX + 0x4e6164);
  (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x394))
            (this,0,*(float *)(unaff_EBX + 0x36bf00) +
                    *(float *)(**(int **)(unaff_EBX + 0x4e6150) + 0xc),0,0,*puVar1,puVar1[1],
             puVar1[2],0);
                    /* WARNING: Could not recover jumptable at 0x00760a93. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x3a0))();
  return;
}


/* CPropPaintBomb::InputDisablePortalFunnel at 00760aa0 */

/* DWARF original prototype: void InputDisablePortalFunnel(CPropPaintBomb * this, inputdata_t * in)
    */

void __thiscall CPropPaintBomb::InputDisablePortalFunnel(CPropPaintBomb *this,inputdata_t *in)

{
  (this->super_CPhysicsProp).m_bAllowPortalFunnel = false;
  return;
}


/* CPropPaintBomb::InputEnablePortalFunnel at 00760ab0 */

/* DWARF original prototype: void InputEnablePortalFunnel(CPropPaintBomb * this, inputdata_t * in)
    */

void __thiscall CPropPaintBomb::InputEnablePortalFunnel(CPropPaintBomb *this,inputdata_t *in)

{
  (this->super_CPhysicsProp).m_bAllowPortalFunnel = true;
  return;
}


/* CreatePaintBomb at 00762dc0 */

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
  piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x4e3e07);
  (**(code **)(*piVar1 + 0x74))();
  bVar4 = CBaseEntity::IsPrecacheAllowed();
  CBaseEntity::SetAllowPrecache(true);
  this = (CPropPaintBomb *)CreateEntityByName((char *)(unaff_EBX + 0x2fcf25),-1,true);
  if (this != (CPropPaintBomb *)0x0) {
    (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                      super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
                      super_IServerUnknown + 0x68))();
    CPropPaintBomb::SetPaintPowerType(this,paintType);
    DispatchSpawn((CBaseEntity *)this,true);
    this_00 = UTIL_GetCommandClient();
    CBasePlayer::EyeVectors(this_00,local_28,(Vector *)0x0,(Vector *)0x0);
                    /* Unresolved local var: Vector res@[???] */
    local_44 = *(float *)(unaff_EBX + 0x36aab7);
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
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e3dff) + 0x14))();
    if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x4e3dfb) + 0x1c) + 0x30) != 0) {
      DebugDrawLine(&local_110,local_104,0xff,0,0,true,-1.0);
    }
    if (*(float *)(unaff_EBX + 0x369b3f) != local_e4) {
      local_104[0].z = local_104[0].z + *(float *)(unaff_EBX + 0x369d6b);
      (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
                        super_IServerUnknown + 0x1c4))();
      UTIL_DropToFloor((CBaseEntity *)this,0x200400b,(CBaseEntity *)0x0);
    }
    CBaseEntity::UpdateObjectCapsCache((CBaseEntity *)this);
  }
  *puVar5 = (uint)bVar4;
  puVar5[-1] = 0x763038;
  CBaseEntity::SetAllowPrecache(SUB41(*puVar5,0));
  iVar2 = *piVar1;
  *puVar5 = (uint)piVar1;
  pcVar3 = *(code **)(iVar2 + 0x78);
  puVar5[-1] = 0x763046;
  (*pcVar3)();
  return;
}


/* ent_create_paint_bomb_jump at 00763150 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void ent_create_paint_bomb_jump(CCommand *args)

{
  CreatePaintBomb(BOUNCE_POWER);
  return;
}


/* __tcf_13 at 00a40390 */

void __tcf_13(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x410068),in_stack_00000008);
  return;
}


/* ent_create_paint_bomb_speed at 00763140 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void ent_create_paint_bomb_speed(CCommand *args)

{
  CreatePaintBomb(SPEED_POWER);
  return;
}


/* __tcf_14 at 00a40370 */

void __tcf_14(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x4100c8),in_stack_00000008);
  return;
}


/* ent_create_paint_bomb_stick at 00763130 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void ent_create_paint_bomb_stick(CCommand *args)

{
  CreatePaintBomb(STICK_POWER);
  return;
}


/* __tcf_15 at 00a40350 */

void __tcf_15(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x410128),in_stack_00000008);
  return;
}


/* ent_create_paint_bomb_portal at 00763120 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void ent_create_paint_bomb_portal(CCommand *args)

{
  CreatePaintBomb(PORTAL_POWER);
  return;
}


/* __tcf_16 at 00a40330 */

void __tcf_16(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x410188),in_stack_00000008);
  return;
}


/* ent_create_paint_bomb_erase at 00763110 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void ent_create_paint_bomb_erase(CCommand *args)

{
  CreatePaintBomb(NO_POWER);
  return;
}


/* __tcf_17 at 00a40310 */

void __tcf_17(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x4101e8),in_stack_00000008);
  return;
}


/* _GLOBAL__I_paintbomb_explosion_radius at 000b7c20 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_paintbomb_explosion_radius(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

