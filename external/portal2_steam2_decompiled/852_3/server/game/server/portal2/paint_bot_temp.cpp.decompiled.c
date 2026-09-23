/* DWARF-guided pseudocode for game/server/portal2/paint_bot_temp.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000c2480 */

/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *pDefaultValue;
  undefined4 uVar1;
  int unaff_EBX;
  longlong lVar2;
  ConVar *pCVar3;
  ConVar *in_stack_ffffffc8;
  char *in_stack_ffffffcc;
  char *in_stack_ffffffd0;
  int in_stack_ffffffd4;
  char *in_stack_ffffffd8;
  
  lVar2 = ___i686_get_pc_thunk_bx();
  if (lVar2 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)&sk_npc_dmg_pistol.super_ConCommandBase.m_pszHelpString + unaff_EBX + 2) = 0;
  *(undefined1 *)((int)&sk_npc_dmg_pistol.super_ConCommandBase.m_pszHelpString + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&sk_npc_dmg_pistol.super_ConCommandBase.m_nFlags + unaff_EBX) = 0;
  *(undefined1 *)((int)&sk_npc_dmg_pistol.super_ConCommandBase.m_nFlags + unaff_EBX + 1) = 0;
  *(undefined4 *)((int)&sk_npc_dmg_pistol.super_ConCommandBase.m_nFlags + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&sk_npc_dmg_pistol.super_IConVar._vptr_IConVar + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&sk_npc_dmg_pistol.m_pParent + unaff_EBX + 2) = 0x7f7fffff;
  *(undefined4 *)((int)&sk_npc_dmg_pistol.m_pszDefaultValue + unaff_EBX + 2) = 0x7f7fffff;
  *(undefined4 *)((int)&sk_npc_dmg_pistol.m_Value.m_pszString + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&sk_npc_dmg_pistol.m_Value.m_StringLength + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&sk_npc_dmg_pistol.m_Value.m_fValue + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&sk_npc_dmg_pistol.m_Value.m_nValue + unaff_EBX + 2) = 0;
  *(undefined4 *)(&sk_npc_dmg_pistol.field_0x36 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&sk_npc_dmg_pistol.m_fMinVal + unaff_EBX + 2) = 0x7f7fffff;
  *(undefined4 *)(&sk_npc_dmg_pistol.field_0x3e + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&sk_npc_dmg_pistol.m_fMaxVal + unaff_EBX + 2) = 0x7f7fffff;
  *(undefined4 *)((int)&sk_npc_dmg_pistol.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 2) =
       0;
  *(undefined1 *)((int)&sk_npc_dmg_pistol.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 1)
       = 0;
  *(undefined1 *)
   ((int)&sk_npc_dmg_pistol.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX + 2) = 0;
  *(undefined1 *)
   ((int)&sk_npc_dmg_pistol.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&sk_npc_dmg_pistol.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) = 0;
  *(undefined4 *)((int)&sk_npc_dmg_pistol.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 2)
       = 0;
  *(undefined1 *)((int)&sk_npc_dmg_pistol.m_fnChangeCallbacks.m_pElements + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&sk_npc_dmg_pistol.m_fnChangeCallbacks.m_Size + unaff_EBX + 2) = 0;
  *(undefined1 *)((int)&sk_npc_dmg_pistol.m_fnChangeCallbacks.m_Size + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&sk_npc_dmg_pistol.m_fnChangeCallbacks.m_pElements + unaff_EBX) = 0;
  *(undefined4 *)((int)&sk_npc_dmg_pistol.m_fnChangeCallbacks.m_pElements + unaff_EBX + 2) = 0;
  (&DAT_00cf583d)[unaff_EBX] = 1;
  (&DAT_00cf583a)[unaff_EBX] = 0;
  (&DAT_00cf583b)[unaff_EBX] = 0;
  (&DAT_00cf583c)[unaff_EBX] = 0;
  *(undefined4 *)(&DAT_00cf583e + unaff_EBX) = 1;
  *(undefined1 *)((int)&sk_max_pistol.super_ConCommandBase.m_pNext + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&sk_max_pistol.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX + 2) = 0
  ;
  *(undefined1 *)((int)&sk_max_pistol.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX + 3) = 0
  ;
  *(undefined1 *)((int)&sk_max_pistol.super_ConCommandBase.m_pNext + unaff_EBX) = 0;
  *(undefined4 *)((int)&sk_max_pistol.super_ConCommandBase.m_pNext + unaff_EBX + 2) = 2;
  *(undefined1 *)((int)&sk_max_pistol.super_ConCommandBase.m_pszName + unaff_EBX + 1) = 0;
  (&sk_max_pistol.super_ConCommandBase.field_0xa)[unaff_EBX] = 0;
  (&sk_max_pistol.super_ConCommandBase.field_0xb)[unaff_EBX] = 0;
  *(undefined1 *)((int)&sk_max_pistol.super_ConCommandBase.m_pszName + unaff_EBX) = 0;
  *(undefined **)((int)&sk_max_pistol.super_ConCommandBase.m_pszName + unaff_EBX + 2) =
       &UNK_00aeee9a + unaff_EBX;
  ConVar::ConVar((ConVar *)((int)&PTR_GetDropCursor_00c6c930 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x94dacc),(char *)(unaff_EBX + 0x8e96ce),0,
                 (char *)(unaff_EBX + 0x94da9e),in_stack_ffffffc8,in_stack_ffffffcc,
                 in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  uVar1 = *(undefined4 *)(&DAT_00ae9702 + unaff_EBX);
  ___cxa_atexit(unaff_EBX + 0x8d84f2,0,uVar1);
  pDefaultValue = (char *)(unaff_EBX + 0x8e97aa);
  ConVar::ConVar((ConVar *)((int)&PTR_InternalCursorMoved_00c6c990 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x94dafc),pDefaultValue,0,(char *)(unaff_EBX + 0x94dae0),
                 in_stack_ffffffc8,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,
                 in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8d84d2,0,uVar1);
  ConVar::ConVar((ConVar *)(&UNK_00c6c9f2 + unaff_EBX),(char *)(unaff_EBX + 0x94db39),pDefaultValue,
                 0,(char *)(unaff_EBX + 0x94db0e),in_stack_ffffffc8,in_stack_ffffffcc,
                 in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8d84b2,0,uVar1);
  ConVar::ConVar((ConVar *)((int)&PTR_GetName_00c6ca50 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x94db71),pDefaultValue,0,(char *)(unaff_EBX + 0x94db4e),
                 in_stack_ffffffc8,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,
                 in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8d8492,0,uVar1);
  pCVar3 = (ConVar *)(unaff_EBX + 0x94db7e);
  ConVar::ConVar((ConVar *)((int)&PTR_SetParent_00c6cab0 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x94dbcd),pDefaultValue,0,(char *)pCVar3,in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8d8472,0,uVar1);
  ConVar::ConVar((ConVar *)((int)&PTR_HasFocus_00c6cb10 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x94dbd8),pDefaultValue,0x4000,pCVar3,
                 (char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x8d8452,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)&sk_max_pistol.super_ConCommandBase.m_pszHelpString + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x93d9ad),pDefaultValue,0,(char *)(unaff_EBX + 0x93d986),
                 in_stack_ffffffc8,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,
                 in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8d8432,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)&sk_plr_dmg_smg1.super_ConCommandBase.m_pszHelpString + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x94dbfe),(char *)(unaff_EBX + 0x94dbfa),0,
                 (char *)(unaff_EBX + 0x94dbe5),in_stack_ffffffc8,in_stack_ffffffcc,
                 in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8d8412,0,uVar1);
  ConVar::ConVar((ConVar *)((int)&PTR_ApplySettings_00c6cb70 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x94dc47),(char *)(unaff_EBX + 0x8fc6ce),0x4000,
                 (char *)(unaff_EBX + 0x94dc16),in_stack_ffffffc8,in_stack_ffffffcc,
                 in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8d83f2,0,uVar1);
  pCVar3 = (ConVar *)(unaff_EBX + 0x94dc5e);
  ConVar::ConVar((ConVar *)((int)&PTR_GetKeyBindingsContext_00c6cbd0 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x94dc8f),(char *)(unaff_EBX + 0x8fc6ce),0x4000,(char *)pCVar3
                 ,in_stack_ffffffc8,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,
                 in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8d83d2,0,uVar1);
  ConVar::ConVar((ConVar *)((int)&PTR_IsMouseInputEnabled_00c6cc30 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x94dca7),pDefaultValue,0x4000,pCVar3,
                 (char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x8d83b2,0,uVar1);
  ConVar::ConVar((ConVar *)((int)&PTR_OnDraggablePanelPaint_00c6cc90 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x94dcdd),pDefaultValue,0x4000,(char *)(unaff_EBX + 0x94dcba),
                 in_stack_ffffffc8,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,
                 in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8d8392,0,uVar1);
  ConVar::ConVar((ConVar *)((int)&PTR_OnStartDragging_00c6ccf0 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x94dd14),pDefaultValue,0x4000,
                 (char *)(vgui::TextEntry::GotoWordRight + unaff_EBX + 6),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8d8372,0,uVar1);
  ConVar::ConVar((ConVar *)((int)&PTR_GetSizerMinimumSize_00c6cd50 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x94dd4b),pDefaultValue,0x4000,(char *)(unaff_EBX + 0x94dd26),
                 in_stack_ffffffc8,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,
                 in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8d8352,0,uVar1);
  ConVar::ConVar((ConVar *)((int)&PTR_InternalFocusChanged_00c6cdb0 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x94dd86),(char *)(unaff_EBX + 0x94dd80),0x4000,
                 (char *)(unaff_EBX + 0x94dd56),in_stack_ffffffc8,in_stack_ffffffcc,
                 in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8d8332,0,uVar1);
  ConVar::ConVar((ConVar *)((int)&PTR_SetVisible_00c6ce10 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x94ddd4),pDefaultValue,0x4000,(char *)(unaff_EBX + 0x94dd9e),
                 in_stack_ffffffc8,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,
                 in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8d8312,0,uVar1);
  ConVar::ConVar((ConVar *)((int)&PTR_IsPopup_00c6ce70 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x94de06),pDefaultValue,0x4000,(char *)(unaff_EBX + 0x94ddde),
                 in_stack_ffffffc8,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,
                 in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8d82f2,0,uVar1);
  ConCommand::ConCommand
            ((ConCommand *)((int)&PTR_IsLayoutInvalid_00c6ced0 + unaff_EBX + 2),
             (char *)(unaff_EBX + 0x94de10),(FnCommandCallback_t)(unaff_EBX + 0x6071f2),
             (char *)(unaff_EBX + 0x8e96ce),0x4000,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x8d82d2,0,uVar1);
  ConCommand::ConCommand
            ((ConCommand *)
             ((int)&sk_plr_dmg_357.super_ConCommandBase.m_pszHelpString + unaff_EBX + 2),
             (char *)(unaff_EBX + 0x94de2c),(FnCommandCallback_t)(unaff_EBX + 0x607102),
             (char *)(unaff_EBX + 0x94de21),0x4000,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x8d82b2,0,uVar1);
  ConCommand::ConCommand
            ((ConCommand *)((int)&sk_plr_dmg_357.m_fnChangeCallbacks.m_Size + unaff_EBX + 2),
             (char *)(unaff_EBX + 0x94de86),(FnCommandCallback_t)(unaff_EBX + 0x607622),
             (char *)(unaff_EBX + 0x94de32),0x4000,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x8d8292,0,uVar1);
  ConCommand::ConCommand
            ((ConCommand *)((int)&sk_npc_dmg_357.m_Value.m_nValue + unaff_EBX + 2),
             (char *)(unaff_EBX + 0x94dea8),(FnCommandCallback_t)(unaff_EBX + 0x607162),
             (char *)(unaff_EBX + 0x94de92),0x4000,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(&UNK_008d8272 + unaff_EBX,0,uVar1);
  ConCommand::ConCommand
            ((ConCommand *)((int)&sk_max_357.super_ConCommandBase.m_pszHelpString + unaff_EBX + 2),
             (char *)(unaff_EBX + 0x94decc),(FnCommandCallback_t)(unaff_EBX + 0x606d62),
             (char *)(unaff_EBX + 0x94deb1),0x4000,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(&UNK_008d8252 + unaff_EBX,0,uVar1);
  ConCommand::ConCommand
            ((ConCommand *)((int)&sk_max_357.m_fnChangeCallbacks.m_Size + unaff_EBX + 2),
             (char *)(unaff_EBX + 0x94df5b),(FnCommandCallback_t)(unaff_EBX + 0x607492),
             (char *)(unaff_EBX + 0x94deda),0x4000,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(&UNK_008d8232 + unaff_EBX,0,uVar1);
  ConCommand::ConCommand
            ((ConCommand *)((int)&sk_plr_dmg_crossbow.m_Value.m_nValue + unaff_EBX + 2),
             (char *)(unaff_EBX + 0x94dfc0),(FnCommandCallback_t)(unaff_EBX + 0x607302),
             (char *)(unaff_EBX + 0x94df6a),0x4000,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x8d8212,0,uVar1);
  return;
}


/* __tcf_0 at 0099a980 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x394438),in_stack_00000008);
  return;
}


/* __tcf_1 at 0099a960 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3944b8),in_stack_00000008);
  return;
}


/* __tcf_2 at 0099a940 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x394538),in_stack_00000008);
  return;
}


/* __tcf_3 at 0099a920 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3945b8),in_stack_00000008);
  return;
}


/* __tcf_4 at 0099a900 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x394638),in_stack_00000008);
  return;
}


/* __tcf_5 at 0099a8e0 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3946b8),in_stack_00000008);
  return;
}


/* __tcf_6 at 0099a8c0 */

void __tcf_6(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x41d418),in_stack_00000008);
  return;
}


/* __tcf_7 at 0099a8a0 */

void __tcf_7(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x41d498),in_stack_00000008);
  return;
}


/* __tcf_8 at 0099a880 */

void __tcf_8(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x394778),in_stack_00000008);
  return;
}


/* __tcf_9 at 0099a860 */

void __tcf_9(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3947f8),in_stack_00000008);
  return;
}


/* __tcf_10 at 0099a840 */

void __tcf_10(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x394878),in_stack_00000008);
  return;
}


/* __tcf_11 at 0099a820 */

void __tcf_11(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3948f8),in_stack_00000008);
  return;
}


/* __tcf_12 at 0099a800 */

void __tcf_12(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x394978),in_stack_00000008);
  return;
}


/* __tcf_13 at 0099a7e0 */

void __tcf_13(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3949f8),in_stack_00000008);
  return;
}


/* __tcf_14 at 0099a7c0 */

void __tcf_14(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x394a78),in_stack_00000008);
  return;
}


/* __tcf_15 at 0099a7a0 */

void __tcf_15(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x394af8),in_stack_00000008);
  return;
}


/* __tcf_16 at 0099a780 */

void __tcf_16(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x394b78),in_stack_00000008);
  return;
}


/* cc_bot_selectweapon at 006c9680 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

void cc_bot_selectweapon(CCommand *args)

{
  CBasePlayer *pCVar1;
  int iVar2;
  char *pcVar3;
  char *pcVar4;
  int unaff_EBX;
  
                    /* Unresolved local var: CBasePlayer * pBot@[???]
                       Unresolved local var: CBaseCombatWeapon * pWpn@[???] */
  ___i686_get_pc_thunk_bx();
  if (args->m_nArgc < 3) {
    _Msg();
    return;
  }
  pCVar1 = UTIL_PlayerByName(args->m_ppArgv[1]);
  if (pCVar1 == (CBasePlayer *)0x0) {
    pcVar3 = (char *)(unaff_EBX + 0x2e24c8);
    if (1 < args->m_nArgc) {
      pcVar3 = args->m_ppArgv[1];
    }
    _Msg(unaff_EBX + 0x346721,pcVar3);
  }
  else {
    pcVar3 = (char *)(unaff_EBX + 0x2e24c8);
    if (2 < args->m_nArgc) {
      pcVar3 = args->m_ppArgv[2];
    }
    iVar2 = (**(code **)((int)(pCVar1->super_CBaseCombatCharacter).super_CBaseFlex.
                              super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                              super_IServerEntity.super_IServerUnknown + 0x464))(pCVar1,pcVar3,0);
    if (iVar2 == 0) {
      pcVar3 = (char *)(unaff_EBX + 0x2e24c8);
      if (2 < args->m_nArgc) {
        pcVar3 = args->m_ppArgv[2];
      }
      pcVar4 = (char *)(unaff_EBX + 0x2e24c8);
      if (1 < args->m_nArgc) {
        pcVar4 = args->m_ppArgv[1];
      }
      _Msg(unaff_EBX + 0x34677f,pcVar4,pcVar3);
    }
    else {
      (**(code **)((int)(pCVar1->super_CBaseCombatCharacter).super_CBaseFlex.
                        super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                        super_IServerEntity.super_IServerUnknown + 0x47c))(pCVar1,iVar2,0);
    }
  }
  return;
}


/* __tcf_17 at 0099a760 */

void __tcf_17(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x394bf8),in_stack_00000008);
  return;
}


/* BotPutInServer at 006c9360 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

CBasePlayer * BotPutInServer(bool bFrozen,char *pszCustomName)

{
  int *piVar1;
  edict_t *peVar2;
  char *pFormat;
  int iVar3;
  CBasePlayer *pCVar4;
  uint uVar5;
  int unaff_EBX;
  undefined4 uVar6;
  char local_60 [64];
  int local_20;
  
                    /* Unresolved local var: char[64] botname@[???]
                       Unresolved local var: edict_t * pEdict@[???]
                       Unresolved local var: CPortal_Player * pPlayer@[???]
                       Unresolved local var: botdata_t * pBot@[???] */
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(unaff_EBX + 0x4e287e);
  if ((pszCustomName == (char *)0x0) || (*pszCustomName == '\0')) {
    if (*(int *)(*(int *)(&DAT_00665d6e + unaff_EBX) + 0x30) == 0) {
      uVar6 = *(undefined4 *)(unaff_EBX + 0x62430e);
      pFormat = (char *)(unaff_EBX + 0x34698a);
    }
    else {
      uVar5 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e30ea) + 0x10))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x4e30ea),0,5);
      if (uVar5 < 5) {
                    /* WARNING: Could not recover jumptable at 0x006c9509. Too many branches */
                    /* WARNING: Treating indirect jump as call */
        pCVar4 = (CBasePlayer *)(*(code *)(*(int *)(unaff_EBX + 0x19e + uVar5 * 4) + unaff_EBX))();
        return pCVar4;
      }
      uVar6 = *(undefined4 *)(unaff_EBX + 0x62430e);
      pFormat = &UNK_00346a2f + unaff_EBX;
    }
    V_snprintf(local_60,0x40,pFormat,uVar6);
  }
  else {
    V_strncpy(local_60,pszCustomName,0x40);
  }
  iVar3 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e284a) + 0xd8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x4e284a),local_60);
  if (iVar3 == 0) {
    _Msg(unaff_EBX + 0x33e202);
    pCVar4 = (CBasePlayer *)0x0;
  }
  else {
    piVar1 = *(int **)(iVar3 + 0xc);
    if (piVar1 == (int *)0x0) {
      pCVar4 = (CBasePlayer *)0x0;
    }
    else {
      pCVar4 = (CBasePlayer *)(**(code **)(*piVar1 + 0x18))(piVar1);
    }
    CBaseEntity::ClearFlags((CBaseEntity *)pCVar4);
    CBaseEntity::AddFlag((CBaseEntity *)pCVar4,0x180);
    if (bFrozen) {
      piVar1 = &(pCVar4->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                super_CBaseAnimating.super_CBaseEntity.m_iEFlags;
      *piVar1 = *piVar1 | 0x100;
    }
    *(int *)(unaff_EBX + 0x62430e) = *(int *)(unaff_EBX + 0x62430e) + 1;
    peVar2 = (pCVar4->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
             super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev;
                    /* Unresolved local var: int edictIndex@[???] */
    iVar3 = -1;
    if (peVar2 != (edict_t *)0x0) {
      iVar3 = ((int)peVar2 - *(int *)(**(int **)(unaff_EBX + 0x4e2832) + 0x58) >> 4) + -1;
    }
    iVar3 = unaff_EBX + 0x6eea32 + iVar3 * 0x34;
    *(undefined1 *)(iVar3 + 0x2d) = 0;
    *(undefined1 *)(iVar3 + 0x2c) = 0;
  }
  if (local_20 == **(int **)(unaff_EBX + 0x4e287e)) {
    return pCVar4;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}


/* bot at 006c9590 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

void bot(CCommand *args)

{
  char *pcVar1;
  char *pszCustomName;
  int unaff_EBX;
  
                    /* Unresolved local var: bool bFrozen@[???]
                       Unresolved local var: char * pName@[???] */
  ___i686_get_pc_thunk_bx();
  pcVar1 = CCommand::FindArg(args,&UNK_00346802 + unaff_EBX);
  pszCustomName = CCommand::FindArg(args,&UNK_0034680a + unaff_EBX);
  BotPutInServer(pcVar1 != (char *)0x0,pszCustomName);
  return;
}


/* __tcf_18 at 0099a740 */

void __tcf_18(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x41dd18),in_stack_00000008);
  return;
}


/* Bot_RunAll at 006cb510 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void Bot_RunAll(void)

{
  char cVar1;
  CBasePlayer *pCVar2;
  CPortal_Player *pBot;
  int unaff_EBX;
  int playerIndex;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(**(int **)(unaff_EBX + 0x4e0682) + 0x14)) {
    playerIndex = 1;
    do {
                    /* Unresolved local var: CPortal_Player * pPlayer@[???] */
      while ((((pCVar2 = UTIL_PlayerByIndex(playerIndex), pCVar2 != (CBasePlayer *)0x0 &&
               (cVar1 = (**(code **)((int)(pCVar2->super_CBaseCombatCharacter).super_CBaseFlex.
                                          super_CBaseAnimatingOverlay.super_CBaseAnimating.
                                          super_CBaseEntity.super_IServerEntity.super_IServerUnknown
                                    + 0x154))(pCVar2), cVar1 != '\0')) &&
              (pBot = (CPortal_Player *)
                      ___dynamic_cast(pCVar2,*(undefined4 *)(unaff_EBX + 0x4e06aa),
                                      *(undefined4 *)(unaff_EBX + 0x4e08aa),0),
              pBot != (CPortal_Player *)0x0)) &&
             ((*(byte *)((int)&(pBot->
                               super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                               super_CPaintableEntity<CBaseMultiplayerPlayer>.
                               super_CBaseMultiplayerPlayer.super_CAI_ExpresserHost<CBasePlayer>.
                               super_CBasePlayer.super_CBaseCombatCharacter.super_CBaseFlex + 0x131)
              & 1) != 0))) {
        Bot_Think(pBot);
        playerIndex = playerIndex + 1;
        if (*(int *)(**(int **)(unaff_EBX + 0x4e0682) + 0x14) < playerIndex) {
          return;
        }
      }
      playerIndex = playerIndex + 1;
    } while (playerIndex <= *(int *)(**(int **)(unaff_EBX + 0x4e0682) + 0x14));
  }
  return;
}


/* RunMimicCommand at 006c9200 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

bool RunMimicCommand(CUserCmd *cmd)

{
  undefined2 uVar1;
  int playerIndex;
  CBasePlayer *pCVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: CBasePlayer * pPlayer@[???] */
  ___i686_get_pc_thunk_bx();
  playerIndex = *(int *)(*(int *)(unaff_EBX + 0x6eeaef) + 0x30);
  if ((0 < playerIndex) && (playerIndex <= *(int *)(**(int **)(unaff_EBX + 0x4e2993) + 0x14))) {
    pCVar2 = UTIL_PlayerByIndex(playerIndex);
    if (pCVar2 != (CBasePlayer *)0x0) {
      if (cmd != &pCVar2->m_LastCmd) {
        cmd->command_number = (pCVar2->m_LastCmd).command_number;
        cmd->tick_count = (pCVar2->m_LastCmd).tick_count;
        (cmd->viewangles).x = (pCVar2->m_LastCmd).viewangles.x;
        (cmd->viewangles).y = (pCVar2->m_LastCmd).viewangles.y;
        (cmd->viewangles).z = (pCVar2->m_LastCmd).viewangles.z;
        cmd->forwardmove = (pCVar2->m_LastCmd).forwardmove;
        cmd->sidemove = (pCVar2->m_LastCmd).sidemove;
        cmd->upmove = (pCVar2->m_LastCmd).upmove;
        cmd->buttons = (pCVar2->m_LastCmd).buttons;
        cmd->impulse = (pCVar2->m_LastCmd).impulse;
        cmd->weaponselect = (pCVar2->m_LastCmd).weaponselect;
        cmd->weaponsubtype = (pCVar2->m_LastCmd).weaponsubtype;
        cmd->random_seed = (pCVar2->m_LastCmd).random_seed;
        uVar1 = (pCVar2->m_LastCmd).mousedx;
        cmd->mousedx = uVar1;
        cmd->mousedy = (pCVar2->m_LastCmd).mousedy;
        cmd->hasbeenpredicted = (pCVar2->m_LastCmd).hasbeenpredicted;
        cmd->player_held_entity = (pCVar2->m_LastCmd).player_held_entity;
        cmd->held_entity_was_grabbed_through_portal =
             (short)*(undefined4 *)&(pCVar2->m_LastCmd).held_entity_was_grabbed_through_portal;
        (cmd->headangles).x = (pCVar2->m_LastCmd).headangles.x;
        (cmd->headangles).y = (pCVar2->m_LastCmd).headangles.y;
        (cmd->headangles).z = (pCVar2->m_LastCmd).headangles.z;
        (cmd->headoffset).x = (pCVar2->m_LastCmd).headoffset.x;
        (cmd->headoffset).y = (pCVar2->m_LastCmd).headoffset.y;
        (cmd->headoffset).z = (pCVar2->m_LastCmd).headoffset.z;
      }
      (cmd->viewangles).y = (cmd->viewangles).y + *(float *)(*(int *)(unaff_EBX + 0x6eeb4f) + 0x2c);
      return true;
    }
  }
  return false;
}


/* Bot_Think at 006c9c50 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "MoveCollide_t": Some values do not have unique names */
/* WARNING: Enum "MoveType_t": Some values do not have unique names */

void Bot_Think(CPortal_Player *pBot)

{
  float *pfVar1;
  undefined1 *puVar2;
  ShouldHitFunc_t p_Var3;
  float fVar4;
  float fVar5;
  uint uVar6;
  undefined2 uVar7;
  edict_t *peVar8;
  int *piVar9;
  code *pcVar10;
  int *piVar11;
  _func_int_varargs **pp_Var12;
  _func_int_varargs *p_Var13;
  ShouldHitFunc_t p_Var14;
  char cVar15;
  bool bVar16;
  vec_t *pvVar17;
  int iVar18;
  int iVar19;
  CBasePlayer *pCVar20;
  QAngle *pQVar21;
  undefined4 uVar22;
  IChangeInfoAccessor *pIVar23;
  IMoveHelper *pIVar24;
  int iVar25;
  CBaseCombatWeapon *pCVar26;
  ShouldHitFunc_t p_Var27;
  CWeaponPaintGun *this;
  CBaseCombatWeapon *pCVar28;
  int unaff_EBX;
  int *piVar29;
  longdouble lVar30;
  char *pcVar31;
  double dVar32;
  double dVar33;
  float fVar34;
  float local_1f4;
  float local_1f0;
  float local_1ec;
  uint local_1e8;
  int local_1e0;
  int local_1c8;
  Vector local_1b8;
  Vector local_1ac;
  float local_1a0;
  float local_19c;
  float local_198;
  uint local_194;
  uint8 local_190;
  int local_18c;
  int local_188;
  int local_184;
  undefined4 local_180;
  bool local_17c;
  short local_17a;
  short local_178;
  undefined2 uStack_176;
  vec_t local_174;
  vec_t local_170;
  vec_t local_16c;
  vec_t local_168;
  vec_t local_164;
  vec_t local_160;
  float local_15c;
  float local_158;
  float local_154;
  float local_14c;
  float local_148;
  float local_144;
  undefined4 local_13c;
  undefined4 local_138;
  undefined4 local_134;
  undefined4 local_12c;
  undefined4 local_128;
  undefined4 local_124;
  undefined4 local_11c;
  undefined1 local_118;
  undefined1 local_117;
  CTraceFilterSimple local_100;
  ITraceFilter local_f0;
  char *local_ec;
  undefined4 uStack_e8;
  undefined4 uStack_e4;
  undefined4 uStack_e0;
  float local_dc;
  float local_d8;
  float local_d4;
  ITraceFilter local_d0;
  IHandleEntity *local_cc;
  float local_c8;
  float local_c4;
  float local_c0;
  float local_bc;
  float local_b8;
  float local_b4;
  float local_b0;
  float local_ac;
  float local_a8;
  float local_a4;
  Vector local_a0;
  Vector local_94;
  Vector local_88;
  float local_7c;
  float local_78;
  float local_74;
  float local_70;
  float local_6c;
  float local_68;
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
  QAngle local_28 [2];
  
                    /* Unresolved local var: botdata_t * botdata@[???]
                       Unresolved local var: QAngle vecViewAngles@[???]
                       Unresolved local var: float forwardmove@[???]
                       Unresolved local var: float sidemove@[???]
                       Unresolved local var: float upmove@[???]
                       Unresolved local var: ushort buttons@[???]
                       Unresolved local var: uint8 impulse@[???]
                       Unresolved local var: float frametime@[???]
                       Unresolved local var: CMDLCacheCriticalSection cacheCriticalSection@[???]
                       Unresolved local var: CBasePlayer * pPlayer@[???] */
  ___i686_get_pc_thunk_bx();
  CBaseEntity::AddFlag((CBaseEntity *)pBot,0x100);
  peVar8 = (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
           super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
           super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
           m_Network.m_pPev;
  if (peVar8 == (edict_t *)0x0) {
    local_1c8 = -1;
  }
  else {
    local_1c8 = ((int)peVar8 - *(int *)(**(int **)(unaff_EBX + 0x4e1f3f) + 0x58) >> 4) + -1;
  }
  iVar19 = local_1c8 * 0x34 + unaff_EBX;
  local_1f0 = *(float *)(&UNK_006ee13f + iVar19 + 0x10);
  fVar4 = *(float *)(**(int **)(unaff_EBX + 0x4e1f3f) + 0x10);
  pvVar17 = (vec_t *)(**(code **)(*(int *)&(pBot->
                                           super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>
                                           ).super_CPaintableEntity<CBaseMultiplayerPlayer>.
                                           super_CBaseMultiplayerPlayer.
                                           super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.
                                           super_CBaseCombatCharacter.super_CBaseFlex + 0x224))();
  local_28[0].x = *pvVar17;
  local_28[0].y = pvVar17[1];
  local_28[0].z = pvVar17[2];
  piVar9 = (int *)**(undefined4 **)(unaff_EBX + 0x4e1fb3);
  (**(code **)(*piVar9 + 0x74))();
  cVar15 = (**(code **)(*(int *)&(pBot->
                                 super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                                 super_CPaintableEntity<CBaseMultiplayerPlayer>.
                                 super_CBaseMultiplayerPlayer.super_CAI_ExpresserHost<CBasePlayer>.
                                 super_CBasePlayer.super_CBaseCombatCharacter.super_CBaseFlex +
                       0x114))();
  if (cVar15 == '\0') {
LAB_006c9d40:
    cVar15 = (**(code **)(*(int *)&(pBot->
                                   super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                                   super_CPaintableEntity<CBaseMultiplayerPlayer>.
                                   super_CBaseMultiplayerPlayer.super_CAI_ExpresserHost<CBasePlayer>
                                   .super_CBasePlayer.super_CBaseCombatCharacter.super_CBaseFlex +
                         0x114))();
    if (cVar15 == '\0') {
      iVar19 = local_1c8 * 0x34 + unaff_EBX;
      if ((&UNK_006ee13f)[iVar19 + 0x2d] == '\0') {
        (&UNK_006ee13f)[iVar19 + 0x2d] = 1;
        *(undefined4 *)(&UNK_006ee13f + iVar19 + 0x30) =
             *(undefined4 *)(**(int **)(unaff_EBX + 0x4e1f3f) + 0xc);
      }
      else {
        fVar34 = *(float *)(**(int **)(unaff_EBX + 0x4e1f3f) + 0xc) -
                 *(float *)(&UNK_006ee13f + iVar19 + 0x30);
        if (*(float *)(unaff_EBX + 0x3606f7) <= fVar34 && fVar34 != *(float *)(unaff_EBX + 0x3606f7)
           ) {
          iVar19 = *(int *)(*(int *)(unaff_EBX + 0x66529b) + 0x30);
          local_1e8 = 2;
          goto joined_r0x006ca91b;
        }
      }
    }
    iVar19 = *(int *)(*(int *)(unaff_EBX + 0x66529b) + 0x30);
    local_1e8 = 0;
joined_r0x006ca91b:
    if (iVar19 == 2) {
                    /* Unresolved local var: Vector random@[???]
                       Unresolved local var: QAngle ret@[???] */
      iVar19 = rand();
      iVar18 = rand();
      iVar25 = rand();
      fVar34 = *(float *)(unaff_EBX + 0x3609a7);
      local_2c = *(float *)(unaff_EBX + 0x36067f);
      local_34 = (float)iVar19 / fVar34 + (float)iVar19 / fVar34 + local_2c;
      local_30 = (float)iVar18 / fVar34 + (float)iVar18 / fVar34 + local_2c;
      local_2c = (float)iVar25 / fVar34 + (float)iVar25 / fVar34 + local_2c;
      iVar18 = local_1c8 * 0x34;
      iVar19 = iVar18 + unaff_EBX;
      *(float *)(&UNK_006ee13f + iVar19 + 0x20) =
           local_34 + *(float *)(&UNK_006ee13f + iVar19 + 0x20);
      *(float *)(&UNK_006ee13f + iVar19 + 0x24) =
           local_30 + *(float *)(&UNK_006ee13f + iVar19 + 0x24);
      *(float *)(&UNK_006ee13f + iVar19 + 0x28) =
           local_2c + *(float *)(&UNK_006ee13f + iVar19 + 0x28);
      pQVar21 = (QAngle *)(&UNK_006ee13f + iVar18 + 0x20 + unaff_EBX);
      iVar19 = iVar18 + 0x10 + unaff_EBX;
      fVar34 = *(float *)(&UNK_006ee13f + iVar19 + 4);
      uVar6 = *(uint *)(unaff_EBX + 0x36961f);
      fVar5 = *(float *)(unaff_EBX + 0x36267f);
      if (fVar5 < (float)((uint)(pQVar21->x - fVar34) & uVar6)) {
        if (pQVar21->x <= fVar34) {
          pQVar21->x = fVar34 - fVar5;
        }
        else {
          pQVar21->x = fVar34 + fVar5;
        }
      }
      fVar34 = *(float *)(&UNK_006ee13f + iVar19 + 8);
      if (fVar5 < (float)((uint)(pQVar21->y - fVar34) & uVar6)) {
        if (pQVar21->y <= fVar34) {
          pQVar21->y = fVar34 - fVar5;
        }
        else {
          pQVar21->y = fVar34 + fVar5;
        }
      }
      pQVar21->z = 0.0;
      CBaseEntity::SetLocalAngles((CBaseEntity *)pBot,pQVar21);
      iVar19 = local_1c8 * 0x34 + unaff_EBX;
      local_28[0].x = *(vec_t *)(&UNK_006ee13f + iVar19 + 0x20);
      local_28[0].y = *(vec_t *)(&UNK_006ee13f + iVar19 + 0x24);
      local_28[0].z = *(vec_t *)(&UNK_006ee13f + iVar19 + 0x28);
      local_1f4 = 0.0;
    }
    else {
      if (iVar19 != 3) goto LAB_006c9d7c;
      peVar8 = (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
               super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
               super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
               super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
               m_Network.m_pPev;
                    /* Unresolved local var: int edictIndex@[???] */
      dVar32 = 0.0;
      if (peVar8 != (edict_t *)0x0) {
        dVar32 = (double)((int)peVar8 - *(int *)(**(int **)(unaff_EBX + 0x4e1f3f) + 0x58) >> 4);
      }
      fVar34 = AngleNormalize((float)(((double)*(float *)(**(int **)(unaff_EBX + 0x4e1f3f) + 0xc) *
                                       *(double *)(unaff_EBX + 0x36962f) + dVar32) *
                                     *(double *)(&DAT_00369637 + unaff_EBX)));
      iVar19 = local_1c8 * 0x34 + unaff_EBX;
      *(float *)(&UNK_006ee15f + iVar19 + 4) = fVar34;
      *(undefined4 *)(&UNK_006ee15f + iVar19 + 8) = 0;
                    /* Unresolved local var: int edictIndex@[???] */
      if ((pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
          super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
          super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
          super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
          m_Network.m_pPev == (edict_t *)0x0) {
        lVar30 = (longdouble)_sin();
        dVar32 = *(double *)(&DAT_0036963f + unaff_EBX);
        dVar33 = (double)lVar30 * dVar32;
      }
      else {
        lVar30 = (longdouble)_sin();
        dVar32 = *(double *)(&DAT_0036963f + unaff_EBX);
        dVar33 = (double)lVar30 * dVar32;
      }
      local_1f4 = (float)dVar33;
      lVar30 = (longdouble)_cos();
      local_1f0 = (float)(dVar32 * (double)lVar30);
      CBaseEntity::SetLocalAngles
                ((CBaseEntity *)pBot,(QAngle *)(&UNK_006ee13f + local_1c8 * 0x34 + 0x20 + unaff_EBX)
                );
      iVar19 = local_1c8 * 0x34 + unaff_EBX;
      local_28[0].x = *(vec_t *)(&UNK_006ee13f + iVar19 + 0x20);
      local_28[0].y = *(vec_t *)(&UNK_006ee13f + iVar19 + 0x24);
      local_28[0].z = *(vec_t *)(&UNK_006ee13f + iVar19 + 0x28);
      cVar15 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e27f7) + 8))();
      if (cVar15 == '\0') {
                    /* Unresolved local var: int edictIndex@[???] */
        lVar30 = (longdouble)_sin();
        *(float *)(&UNK_006ee13f + local_1c8 * 0x34 + unaff_EBX + 0x20) =
             (float)((double)lVar30 * *(double *)(unaff_EBX + 0x368b07));
        local_1e8 = local_1e8 & 0xf7fe;
      }
      else {
        *(undefined4 *)(&UNK_006ee13f + iVar19 + 0x20) = 0;
      }
    }
  }
  else {
    iVar18 = (*(pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
               super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
               super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
               super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
               m_Collision.super_CCollisionProperty.super_ICollideable._vptr_ICollideable[0xb])();
    if (iVar18 != 2) goto LAB_006c9d40;
    (&UNK_006ee13f)[iVar19 + 0x2d] = 0;
    local_1e8 = ~-(uint)(*(int *)(*(int *)(unaff_EBX + 0x66553b) + 0x30) == 0) & 0x4000;
    if (*(int *)(*(int *)(unaff_EBX + 0x6656bb) + 0x30) != 0) {
      local_1e8 = local_1e8 | 0x8000;
      ConVar::SetValue((ConVar *)(unaff_EBX + 0x66569f),0);
    }
    if (((((pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
           super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
           super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
           m_iEFlags & 0x100) == 0) && (*(int *)(*(int *)(unaff_EBX + 0x6654db) + 0x30) != 0)) &&
       (((pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
         super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
         super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
         super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.m_fFlags
         .m_Value & 1) != 0)) {
      local_1e8 = local_1e8 | 2;
    }
    if (-1 < *(int *)(*(int *)(unaff_EBX + 0x6653bb) + 0x30)) {
                    /* Unresolved local var: int slot@[???]
                       Unresolved local var: CBaseCombatWeapon * pWpn@[???] */
      iVar19 = (**(code **)(*(int *)&(pBot->
                                     super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>
                                     ).super_CPaintableEntity<CBaseMultiplayerPlayer>.
                                     super_CBaseMultiplayerPlayer.
                                     super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.
                                     super_CBaseCombatCharacter.super_CBaseFlex + 0x48c))();
      if (iVar19 != 0) {
        (**(code **)(*(int *)&(pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>
                              ).super_CPaintableEntity<CBaseMultiplayerPlayer>.
                              super_CBaseMultiplayerPlayer.super_CAI_ExpresserHost<CBasePlayer>.
                              super_CBasePlayer.super_CBaseCombatCharacter.super_CBaseFlex + 0x47c))
                  ();
      }
      ConVar::SetValue((ConVar *)(unaff_EBX + 0x66539f),-1);
    }
    if (*(uint *)(*(int *)(unaff_EBX + 0x66541b) + 0x30) < 5) {
                    /* Unresolved local var: CWeaponPaintGun * pPaintGun@[???] */
      pCVar26 = CBaseCombatCharacter::GetActiveWeapon((CBaseCombatCharacter *)pBot);
      if ((pCVar26 != (CBaseCombatWeapon *)0x0) &&
         (this = (CWeaponPaintGun *)___dynamic_cast(), this != (CWeaponPaintGun *)0x0)) {
        CWeaponPaintGun::SetCurrentPaint
                  (this,*(PaintPowerType *)(*(int *)(unaff_EBX + 0x66541b) + 0x30));
      }
      ConVar::SetValue((ConVar *)(unaff_EBX + 0x6653ff),-1);
    }
                    /* Unresolved local var: char * str@[???] */
    pcVar31 = &UNK_002ea933 + unaff_EBX;
    if ((*(byte *)(unaff_EBX + 0x665174) & 0x10) == 0) {
                    /* Unresolved local var: char * str@[???] */
      pcVar31 = *(char **)(*(int *)(unaff_EBX + 0x66517b) + 0x24);
      if (pcVar31 == (char *)0x0) {
        pcVar31 = (char *)(unaff_EBX + 0x2e1efb);
      }
    }
    cVar15 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e27f7) + 8))();
    iVar19 = 0;
    if (cVar15 == '\0') {
      do {
        pCVar26 = CBaseCombatCharacter::GetWeapon((CBaseCombatCharacter *)pBot,iVar19);
        if (pCVar26 != (CBaseCombatWeapon *)0x0) {
          pCVar26 = CBaseCombatCharacter::GetWeapon((CBaseCombatCharacter *)pBot,iVar19);
          if (pCVar26 != (CBaseCombatWeapon *)0x0) {
            local_ec = pcVar31;
            if ((pCVar26->super_CBaseAnimating).super_CBaseEntity.m_iClassname.pszValue != pcVar31)
            {
              bVar16 = CBaseEntity::ClassMatchesComplex((CBaseEntity *)pCVar26,pcVar31);
              if (!bVar16) goto LAB_006cb0bd;
            }
            pCVar26 = CBaseCombatCharacter::GetWeapon((CBaseCombatCharacter *)pBot,iVar19);
            goto LAB_006cb2e3;
          }
        }
LAB_006cb0bd:
        iVar19 = iVar19 + 1;
      } while (iVar19 != 0x30);
    }
    else {
      iVar19 = local_1c8 * 0x34 + unaff_EBX;
      if ((&UNK_006ee15f)[iVar19 + 0xc] == '\0') {
        (&UNK_006ee15f)[iVar19 + 0xc] = 1;
        local_100.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter =
             (ITraceFilter)(_func_int_varargs **)0x0;
        local_100.m_pPassEnt = (IHandleEntity *)0x0;
        local_100.m_collisionGroup = 0;
        local_100.m_pExtraShouldHitCheckFunction = (ShouldHitFunc_t)0x0;
        local_f0._vptr_ITraceFilter = (_func_int_varargs **)0x0;
        local_1e0 = 0;
        do {
          pCVar26 = CBaseCombatCharacter::GetWeapon((CBaseCombatCharacter *)pBot,local_1e0);
          if (pCVar26 != (CBaseCombatWeapon *)0x0) {
            pCVar26 = CBaseCombatCharacter::GetWeapon((CBaseCombatCharacter *)pBot,local_1e0);
            p_Var14 = local_100.m_pExtraShouldHitCheckFunction;
            p_Var3 = local_100.m_pExtraShouldHitCheckFunction + 1;
            p_Var27 = p_Var3;
            if ((int)local_100.m_pPassEnt < (int)p_Var3) {
              CUtlMemory<CBaseCombatWeapon*,int>::Grow
                        ((CUtlMemory<CBaseCombatWeapon*,int> *)&local_100,
                         (int)p_Var3 - (int)local_100.m_pPassEnt);
              p_Var27 = local_100.m_pExtraShouldHitCheckFunction + 1;
            }
            local_f0 = local_100.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter;
            local_100.m_pExtraShouldHitCheckFunction = p_Var27;
            if (0 < (int)(p_Var27 + (-1 - (int)p_Var14))) {
              _V_memmove((_func_int_varargs **)
                         ((int)local_100.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter +
                         (int)p_Var3 * 4),
                         (_func_int_varargs **)
                         ((int)local_100.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter +
                         (int)p_Var14 * 4),(int)(p_Var27 + (-1 - (int)p_Var14)) * 4);
            }
            if ((_func_int_varargs **)
                ((int)local_100.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter +
                p_Var14 * 4) != (_func_int_varargs **)0x0) {
              *(_func_int_varargs **)
               ((int)local_100.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter +
               p_Var14 * 4) = (_func_int_varargs *)pCVar26;
            }
          }
          local_1e0 = local_1e0 + 1;
        } while (local_1e0 != 0x30);
        if ((int)local_100.m_pExtraShouldHitCheckFunction < 1) {
          pCVar26 = (CBaseCombatWeapon *)0x0;
        }
        else {
          iVar19 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e27f7) + 0x10))();
          pCVar26 = *(CBaseCombatWeapon **)
                     ((int)local_100.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter +
                     iVar19 * 4);
        }
        local_100.m_pExtraShouldHitCheckFunction = (ShouldHitFunc_t)0x0;
        if (-1 < local_100.m_collisionGroup) {
          if (local_100.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter !=
              (_func_int_varargs **)0x0) {
            (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e1f37) + 8))();
            local_100.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter =
                 (ITraceFilter)(_func_int_varargs **)0x0;
          }
          local_100.m_pPassEnt = (IHandleEntity *)0x0;
        }
        local_f0 = local_100.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter;
        if (-1 < local_100.m_collisionGroup) {
          if (local_100.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter !=
              (_func_int_varargs **)0x0) {
            (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e1f37) + 8))();
            local_100.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter =
                 (ITraceFilter)(_func_int_varargs **)0x0;
          }
          local_100.m_pPassEnt = (IHandleEntity *)0x0;
        }
LAB_006cb2e3:
                    /* Unresolved local var: CBaseCombatWeapon * pActiveWeapon@[???] */
        if ((pCVar26 != (CBaseCombatWeapon *)0x0) &&
           (pCVar28 = CBaseCombatCharacter::GetActiveWeapon((CBaseCombatCharacter *)pBot),
           pCVar26 != pCVar28)) {
          (**(code **)(*(int *)&(pBot->
                                super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                                super_CPaintableEntity<CBaseMultiplayerPlayer>.
                                super_CBaseMultiplayerPlayer.super_CAI_ExpresserHost<CBasePlayer>.
                                super_CBasePlayer.super_CBaseCombatCharacter.super_CBaseFlex + 0x47c
                      ))();
        }
      }
    }
    pCVar26 = CBaseCombatCharacter::GetActiveWeapon((CBaseCombatCharacter *)pBot);
    if (pCVar26 != (CBaseCombatWeapon *)0x0) {
      if (*(int *)(*(int *)(unaff_EBX + 0x66523b) + 0x30) != 0) {
        local_1e8 = local_1e8 | 1;
      }
      if (*(int *)(*(int *)(unaff_EBX + 0x6651db) + 0x30) != 0) {
        local_1e8 = local_1e8 | 0x800;
      }
    }
    iVar19 = *(int *)(*(int *)(unaff_EBX + 0x66529b) + 0x30);
    if (iVar19 != 0) {
      if (*(int *)(*(int *)(unaff_EBX + 0x66523b) + 0x30) == 0) {
        lVar30 = (longdouble)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e27f7) + 0x14))()
        ;
        if (*(float *)(unaff_EBX + 0x36089f) <= (float)lVar30 &&
            (float)lVar30 != *(float *)(unaff_EBX + 0x36089f)) {
          iVar19 = *(int *)(*(int *)(unaff_EBX + 0x66529b) + 0x30);
          goto LAB_006cb140;
        }
        iVar19 = *(int *)(*(int *)(unaff_EBX + 0x66529b) + 0x30);
      }
      else {
LAB_006cb140:
        local_1e8 = (-(uint)(*(int *)(*(int *)(unaff_EBX + 0x6651db) + 0x30) == 0) & 0xfffff801) +
                    0x800 & 0xffff | local_1e8;
      }
      goto joined_r0x006ca91b;
    }
LAB_006c9d7c:
    local_1f4 = 0.0;
  }
  peVar8 = (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
           super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
           super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
           m_Network.m_pPev;
                    /* Unresolved local var: int edictIndex@[???] */
  iVar19 = -1;
  if (peVar8 != (edict_t *)0x0) {
    iVar19 = ((int)peVar8 - *(int *)(**(int **)(unaff_EBX + 0x4e1f3f) + 0x58) >> 4) + -1;
  }
  pCVar20 = UTIL_PlayerByIndex(iVar19);
  piVar29 = (int *)&stack0xfffffd94;
  if ((*(int *)(*(int *)(unaff_EBX + 0x66565b) + 0x30) != 0) &&
     (piVar29 = (int *)&stack0xfffffd94, pCVar20 != (CBasePlayer *)0x0)) {
    pQVar21 = (QAngle *)
              (**(code **)((int)(pCVar20->super_CBaseCombatCharacter).super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                                super_IServerEntity.super_IServerUnknown + 0x224))();
    AngleVectors(pQVar21,&local_a0);
    local_11c = 0;
                    /* Unresolved local var: Vector res@[???] */
    local_d4 = *(float *)(unaff_EBX + 0x360d5f);
    local_dc = local_a0.x * local_d4;
    local_d8 = local_a0.y * local_d4;
    local_d4 = local_d4 * local_a0.z;
    local_58 = local_dc;
    local_54 = local_d8;
    local_50 = local_d4;
    (**(code **)((int)(pCVar20->super_CBaseCombatCharacter).super_CBaseFlex.
                      super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                      super_IServerEntity.super_IServerUnknown + 0x220))();
                    /* Unresolved local var: Vector res@[???] */
    local_64 = local_4c + local_58;
    local_60 = local_48 + local_54;
    local_5c = local_44 + local_50;
    (**(code **)((int)(pCVar20->super_CBaseCombatCharacter).super_CBaseFlex.
                      super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                      super_IServerEntity.super_IServerUnknown + 0x220))();
    local_14c = local_64 - local_40;
    local_148 = local_60 - local_3c;
    local_144 = local_5c - local_38;
    local_117 = local_14c * local_14c + local_148 * local_148 + local_144 * local_144 != 0.0;
    local_124 = 0;
    local_128 = 0;
    local_12c = 0;
    local_11c = 0;
    local_118 = 1;
    local_134 = 0;
    local_138 = 0;
    local_13c = 0;
    local_15c = local_40;
    local_158 = local_3c;
    local_154 = local_38;
    CTraceFilterSimple::CTraceFilterSimple(&local_100,(IHandleEntity *)0x0,0,(ShouldHitFunc_t)0x0);
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e1fab) + 0x14))();
    if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x4e1fa7) + 0x1c) + 0x30) != 0) {
      DebugDrawLine(&local_1b8,&local_1ac,0xff,0,0,true,-1.0);
    }
    (**(code **)(*(int *)&(pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                          super_CPaintableEntity<CBaseMultiplayerPlayer>.
                          super_CBaseMultiplayerPlayer.super_CAI_ExpresserHost<CBasePlayer>.
                          super_CBasePlayer.super_CBaseCombatCharacter.super_CBaseFlex + 0x220))();
                    /* Unresolved local var: Vector res@[???] */
    local_a0.x = local_1ac.x - local_70;
    local_a0.y = local_1ac.y - local_6c;
    local_a0.z = local_1ac.z - local_68;
    local_7c = local_a0.x;
    local_78 = local_a0.y;
    local_74 = local_a0.z;
    VectorAngles(&local_a0,local_28);
    NDebugOverlay::Sphere(&local_1ac,5.0,0xff,0,0,false,0.1);
    (**(code **)(*(int *)&(pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                          super_CPaintableEntity<CBaseMultiplayerPlayer>.
                          super_CBaseMultiplayerPlayer.super_CAI_ExpresserHost<CBasePlayer>.
                          super_CBasePlayer.super_CBaseCombatCharacter.super_CBaseFlex + 0x220))();
    NDebugOverlay::Line(&local_88,&local_1ac,0,0xff,0,false,0.1);
    (**(code **)((int)(pCVar20->super_CBaseCombatCharacter).super_CBaseFlex.
                      super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                      super_IServerEntity.super_IServerUnknown + 0x220))();
    piVar29 = (int *)&stack0xfffffd80;
    NDebugOverlay::Line(&local_94,&local_1ac,0,0,0xff,false,0.1);
  }
  if ((*(int *)(*(int *)(unaff_EBX + 0x66559b) + 0x30) == 0) || (pCVar20 == (CBasePlayer *)0x0)) {
    piVar29[2] = 0;
    piVar29[1] = 2;
    *piVar29 = (int)pBot;
    piVar29[-1] = 0x6ca4aa;
    CBaseEntity::SetMoveType((CBaseEntity *)*piVar29,piVar29[1],piVar29[2]);
    local_1ec = 0.0;
  }
  else {
    piVar29[2] = 0;
    piVar29[1] = 8;
    *piVar29 = (int)pBot;
    piVar29[-1] = 0x6ca166;
    CBaseEntity::SetMoveType((CBaseEntity *)*piVar29,piVar29[1],piVar29[2]);
    if (((pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
         super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
         super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
         super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
         m_iEFlags & 0x800) != 0) {
      *piVar29 = (int)pBot;
      piVar29[-1] = 0x6cb18e;
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)*piVar29);
    }
    if (((pCVar20->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
         super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      *piVar29 = (int)pCVar20;
      piVar29[-1] = 0x6ca187;
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)*piVar29);
    }
                    /* Unresolved local var: Vector res@[???] */
    local_ac = (pCVar20->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
               super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.x -
               (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
               super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
               super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
               super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
               m_vecAbsOrigin.x;
    local_a8 = (pCVar20->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
               super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.y -
               (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
               super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
               super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
               super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
               m_vecAbsOrigin.y;
    local_a4 = (pCVar20->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
               super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z -
               (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
               super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
               super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
               super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
               m_vecAbsOrigin.z;
                    /* Unresolved local var: __m128 root@[???] */
    pcVar31 = (char *)SQRT(local_ac * local_ac + local_a8 * local_a8 + local_a4 * local_a4);
    uStack_e8 = 0;
    uStack_e4 = 0;
    uStack_e0 = 0;
    *piVar29 = (int)&local_a0;
    piVar29[-1] = 0x6ca252;
    local_ec = pcVar31;
    local_a0.x = local_ac;
    local_a0.y = local_a8;
    local_a0.z = local_a4;
    VectorNormalize((Vector *)*piVar29);
    local_1ec = 0.0;
    pfVar1 = (float *)(*(int *)(unaff_EBX + 0x6655fb) + 0x2c);
    if (*pfVar1 <= (float)pcVar31 && (float)pcVar31 != *pfVar1) {
                    /* Unresolved local var: float speed@[???]
                       Unresolved local var: Vector forward@[???]
                       Unresolved local var: Vector right@[???]
                       Unresolved local var: Vector up@[???] */
      local_1ec = (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                  super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
                  super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_flMaxspeed.m_Value;
      iVar19 = *(int *)&(pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                        super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
                        super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.
                        super_CBaseCombatCharacter.super_CBaseFlex;
      *piVar29 = (int)pBot;
      pcVar10 = *(code **)(iVar19 + 0x224);
      piVar29[-1] = 0x6ca299;
      uVar22 = (*pcVar10)();
      piVar29[3] = (int)&local_100;
      piVar29[2] = (int)&local_15c;
      piVar29[1] = (int)&local_1b8;
      *piVar29 = uVar22;
      piVar29[-1] = 0x6ca2bf;
      AngleVectors((QAngle *)*piVar29,(Vector *)piVar29[1],(Vector *)piVar29[2],(Vector *)piVar29[3]
                  );
                    /* Unresolved local var: Vector norm@[???] */
      local_b8 = local_1b8.x;
      local_b4 = local_1b8.y;
      local_b0 = local_1b8.z;
      *piVar29 = (int)&local_b8;
      piVar29[-1] = 0x6ca2f1;
      VectorNormalize((Vector *)*piVar29);
      local_1f4 = local_1ec *
                  (local_a0.z * local_b0 + local_a0.y * local_b4 + local_a0.x * local_b8);
                    /* Unresolved local var: Vector norm@[???] */
      local_c4 = local_15c;
      local_c0 = local_158;
      local_bc = local_154;
      *piVar29 = (int)&local_c4;
      piVar29[-1] = 0x6ca389;
      VectorNormalize((Vector *)*piVar29);
      local_1f0 = local_1ec *
                  (local_a0.z * local_bc + local_a0.y * local_c0 + local_a0.x * local_c4);
                    /* Unresolved local var: Vector norm@[???] */
      local_d0 = local_100.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter;
      local_cc = local_100.m_pPassEnt;
      local_c8 = (float)local_100.m_collisionGroup;
      *piVar29 = (int)&local_d0;
      piVar29[-1] = 0x6ca421;
      VectorNormalize((Vector *)*piVar29);
      local_1ec = local_1ec *
                  (local_a0.z * local_c8 +
                  local_a0.y * (float)local_cc + local_a0.x * (float)local_d0._vptr_ITraceFilter);
    }
  }
  local_1b8.x = (vec_t)(unaff_EBX + 0x4e7587);
  local_1b8.y = 0.0;
  local_1b8.z = 0.0;
  local_1ac.x = 0.0;
  local_1ac.y = 0.0;
  local_1ac.z = 0.0;
  local_1a0 = 0.0;
  local_19c = 0.0;
  local_198 = 0.0;
  local_194 = 0;
  local_190 = '\0';
  local_18c = 0;
  local_188 = 0;
  local_184 = 0;
  local_180 = 0;
  local_17c = false;
  local_17a = 0;
  local_178 = 0;
  local_174 = 0.0;
  local_170 = 0.0;
  local_16c = 0.0;
  local_168 = 0.0;
  local_164 = 0.0;
  local_160 = 0.0;
  iVar19 = **(int **)(unaff_EBX + 0x4e1f3f);
  fVar34 = *(float *)(iVar19 + 0x10);
  fVar5 = *(float *)(iVar19 + 0xc);
  iVar19 = (int)(((fVar34 + fVar5) - fVar4) / *(float *)(iVar19 + 0x1c) +
                *(float *)(unaff_EBX + 0x36089f));
  if (iVar19 != (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
                super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_nTickBase.m_Value) {
    if ((pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
        super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
        super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.m_Network
        .m_TimerEvent.m_bRegistered == false) {
      peVar8 = (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
               super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
               super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
               super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
               m_Network.m_pPev;
      if (peVar8 != (edict_t *)0x0) {
        (peVar8->super_CBaseEdict).m_fStateFlags = (peVar8->super_CBaseEdict).m_fStateFlags | 0x101;
        *piVar29 = (int)peVar8;
        piVar29[-1] = 0x6ca609;
        pIVar23 = CBaseEdict::GetChangeAccessor((CBaseEdict *)*piVar29);
        pIVar23->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar2 = &(pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
                super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
                super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                m_Network.field_0x4c;
      *(uint *)puVar2 = *(uint *)puVar2 | 1;
    }
    (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_nTickBase.m_Value = iVar19;
  }
  piVar29[2] = 0x5c;
  piVar29[1] = 0;
  *piVar29 = (int)&local_1b8;
  piVar29[-1] = 0x6ca636;
  _V_memset((void *)*piVar29,piVar29[1],piVar29[2]);
  iVar19 = *(int *)(*(int *)(&DAT_006ee09b + unaff_EBX) + 0x30);
  if ((0 < iVar19) && (iVar19 <= *(int *)(**(int **)(unaff_EBX + 0x4e1f3f) + 0x14))) {
    *piVar29 = iVar19;
    piVar29[-1] = 0x6caa98;
    pCVar20 = UTIL_PlayerByIndex(*piVar29);
    if (pCVar20 != (CBasePlayer *)0x0) {
      if ((CUserCmd *)&local_1b8 != &pCVar20->m_LastCmd) {
        local_1b8.y = (vec_t)(pCVar20->m_LastCmd).command_number;
        local_1b8.z = (vec_t)(pCVar20->m_LastCmd).tick_count;
        local_1ac.x = (pCVar20->m_LastCmd).viewangles.x;
        local_1ac.y = (pCVar20->m_LastCmd).viewangles.y;
        local_1ac.z = (pCVar20->m_LastCmd).viewangles.z;
        local_1a0 = (pCVar20->m_LastCmd).forwardmove;
        local_19c = (pCVar20->m_LastCmd).sidemove;
        local_198 = (pCVar20->m_LastCmd).upmove;
        local_194 = (pCVar20->m_LastCmd).buttons;
        local_190 = (pCVar20->m_LastCmd).impulse;
        local_18c = (pCVar20->m_LastCmd).weaponselect;
        local_188 = (pCVar20->m_LastCmd).weaponsubtype;
        local_184 = (pCVar20->m_LastCmd).random_seed;
        uVar7 = (pCVar20->m_LastCmd).mousedx;
        local_180 = CONCAT22((pCVar20->m_LastCmd).mousedy,uVar7);
        local_17c = (pCVar20->m_LastCmd).hasbeenpredicted;
        local_17a = (pCVar20->m_LastCmd).player_held_entity;
        local_178 = (short)*(undefined4 *)
                            &(pCVar20->m_LastCmd).held_entity_was_grabbed_through_portal;
        local_174 = (pCVar20->m_LastCmd).headangles.x;
        local_170 = (pCVar20->m_LastCmd).headangles.y;
        local_16c = (pCVar20->m_LastCmd).headangles.z;
        local_168 = (pCVar20->m_LastCmd).headoffset.x;
        local_164 = (pCVar20->m_LastCmd).headoffset.y;
        local_160 = (pCVar20->m_LastCmd).headoffset.z;
      }
      local_1ac.y = local_1ac.y + *(float *)(*(int *)(&DAT_006ee0fb + unaff_EBX) + 0x2c);
      goto LAB_006ca6da;
    }
  }
  local_1ac.x = local_28[0].x;
  local_1ac.y = local_28[0].y;
  local_1ac.z = local_28[0].z;
  local_1a0 = local_1f4;
  local_19c = local_1f0;
  local_198 = local_1ec;
  local_194 = local_1e8;
  local_190 = '\0';
  piVar11 = (int *)**(undefined4 **)(unaff_EBX + 0x4e27f7);
  iVar19 = *piVar11;
  piVar29[2] = 0x7fffffff;
  piVar29[1] = 0;
  *piVar29 = (int)piVar11;
  pcVar10 = *(code **)(iVar19 + 0x10);
  piVar29[-1] = 0x6ca6d4;
  local_184 = (*pcVar10)();
LAB_006ca6da:
  if (*(int *)(*(int *)(unaff_EBX + 0x66535b) + 0x30) != 0) {
    local_1a0 = 0.0;
    local_19c = 0.0;
    local_198 = 0.0;
  }
  piVar29[-1] = 0x6ca70d;
  pIVar24 = MoveHelperServer();
  pp_Var12 = pIVar24->_vptr_IMoveHelper;
  piVar29[1] = (int)pBot;
  *piVar29 = (int)pIVar24;
  p_Var13 = pp_Var12[1];
  piVar29[-1] = 0x6ca71c;
  (*p_Var13)();
  pcVar10 = *(code **)(*(int *)&(pBot->
                                super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                                super_CPaintableEntity<CBaseMultiplayerPlayer>.
                                super_CBaseMultiplayerPlayer.super_CAI_ExpresserHost<CBasePlayer>.
                                super_CBasePlayer.super_CBaseCombatCharacter.super_CBaseFlex + 0x708
                      );
  piVar29[-1] = 0x6ca72c;
  pIVar24 = MoveHelperServer();
  piVar29[2] = (int)pIVar24;
  piVar29[1] = (int)&local_1b8;
  *piVar29 = (int)pBot;
  piVar29[-1] = 0x6ca73c;
  (*pcVar10)();
  if (&(pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
       super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
       super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_LastCmd != (CUserCmd *)&local_1b8) {
    (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_LastCmd.command_number =
         (int)local_1b8.y;
    (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_LastCmd.tick_count = (int)local_1b8.z;
    (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_LastCmd.viewangles.x = local_1ac.x;
    (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_LastCmd.viewangles.y = local_1ac.y;
    (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_LastCmd.viewangles.z = local_1ac.z;
    (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_LastCmd.forwardmove = local_1a0;
    (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_LastCmd.sidemove = local_19c;
    (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_LastCmd.upmove = local_198;
    (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_LastCmd.buttons = local_194;
    (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_LastCmd.impulse = local_190;
    (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_LastCmd.weaponselect = local_18c;
    (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_LastCmd.weaponsubtype = local_188;
    (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_LastCmd.random_seed = local_184;
    (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_LastCmd.mousedx = (short)local_180;
    (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_LastCmd.mousedy = local_180._2_2_;
    (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_LastCmd.hasbeenpredicted = local_17c;
    (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_LastCmd.player_held_entity = local_17a;
    (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_LastCmd.
    held_entity_was_grabbed_through_portal = local_178;
    (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_LastCmd.headangles.x = local_174;
    (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_LastCmd.headangles.y = local_170;
    (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_LastCmd.headangles.z = local_16c;
    (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_LastCmd.headoffset.x = local_168;
    (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_LastCmd.headoffset.y = local_164;
    (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_LastCmd.headoffset.z = local_160;
  }
  (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
  super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
  super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.pl.super_CPlayerState.fixangle = 0;
  piVar11 = *(int **)(unaff_EBX + 0x4e1f3f);
  *(float *)(*piVar11 + 0x10) = fVar34;
  *(float *)(*piVar11 + 0xc) = fVar5;
  iVar19 = *piVar9;
  *piVar29 = (int)piVar9;
  pcVar10 = *(code **)(iVar19 + 0x78);
  piVar29[-1] = 0x6ca8b4;
  (*pcVar10)();
  return;
}


/* cc_bot_sendcommand at 006c9ab0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

void cc_bot_sendcommand(CCommand *args)

{
  CBasePlayer *pCVar1;
  char *pcVar2;
  int iVar3;
  char *pcVar4;
  int unaff_EBX;
  CCommand local_528;
  int local_20;
  
                    /* Unresolved local var: CBasePlayer * pPlayer@[???]
                       Unresolved local var: char * commandline@[???]
                       Unresolved local var: int iSize@[???]
                       Unresolved local var: char * pBuf@[???]
                       Unresolved local var: CCommand command@[???] */
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(unaff_EBX + 0x4e2125);
  if (args->m_nArgc < 3) {
    _Msg(unaff_EBX + 0x34641d);
  }
  else {
    pCVar1 = UTIL_PlayerByName(args->m_ppArgv[1]);
    if (pCVar1 == (CBasePlayer *)0x0) {
      pcVar2 = (char *)(unaff_EBX + 0x2e2095);
      if (1 < args->m_nArgc) {
        pcVar2 = args->m_ppArgv[1];
      }
      _Msg(unaff_EBX + 0x3462ee,pcVar2);
    }
    else {
      if (args->m_nArgc == 0) {
        pcVar2 = (char *)(unaff_EBX + 0x2e2095);
        pcVar4 = pcVar2;
      }
      else {
        pcVar2 = args->m_pArgSBuffer;
        pcVar4 = (char *)(unaff_EBX + 0x2e2095);
        if (2 < args->m_nArgc) {
          pcVar4 = args->m_ppArgv[2];
        }
      }
      pcVar2 = _strstr(pcVar2,pcVar4);
      iVar3 = _V_strlen(pcVar2);
      pcVar4 = _malloc(iVar3 + 1U);
      V_snprintf(pcVar4,iVar3 + 1U,(char *)(unaff_EBX + 0x2e9f5d),pcVar2);
      if (pcVar4[iVar3 + -1] == '\"') {
        pcVar4[iVar3 + -1] = '\0';
      }
      CCommand::CCommand(&local_528);
      CCommand::Tokenize(&local_528,pcVar4,(characterset_t *)0x0);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e2115) + 0x134))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4e2115),pCVar1,&local_528);
    }
  }
  if (local_20 == **(int **)(unaff_EBX + 0x4e2125)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}


/* __tcf_19 at 0099a720 */

void __tcf_19(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x41dd78),in_stack_00000008);
  return;
}


/* cc_bot_kill at 006c95f0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

void cc_bot_kill(CCommand *args)

{
  CBasePlayer *pCVar1;
  char *pcVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: CBasePlayer * pPlayer@[???] */
  ___i686_get_pc_thunk_bx();
  pcVar2 = (char *)(unaff_EBX + 0x2e255b);
  if (1 < args->m_nArgc) {
    pcVar2 = args->m_ppArgv[1];
  }
  pCVar1 = UTIL_PlayerByName(pcVar2);
  if (pCVar1 != (CBasePlayer *)0x0) {
    (**(code **)((int)(pCVar1->super_CBaseCombatCharacter).super_CBaseFlex.
                      super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                      super_IServerEntity.super_IServerUnknown + 0x764))(pCVar1,0,0);
    return;
  }
  pcVar2 = (char *)(unaff_EBX + 0x2e255b);
  if (1 < args->m_nArgc) {
    pcVar2 = args->m_ppArgv[1];
  }
  _Msg(unaff_EBX + 0x3467b4,pcVar2);
  return;
}


/* __tcf_20 at 0099a700 */

void __tcf_20(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x41ddd8),in_stack_00000008);
  return;
}


/* bot_refill at 006c91f0 */

void bot_refill(CCommand *args)

{
  return;
}


/* __tcf_21 at 0099a6e0 */

void __tcf_21(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x41de38),in_stack_00000008);
  return;
}


/* bot_teleport at 006c9920 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

void bot_teleport(CCommand *args)

{
  CBasePlayer *pCVar1;
  char *pcVar2;
  int unaff_EBX;
  double dVar3;
  double dVar4;
  double dVar5;
  float local_34;
  float local_30;
  float local_2c;
  float local_28;
  float local_24;
  float local_20;
  
                    /* Unresolved local var: CBasePlayer * pBot@[???]
                       Unresolved local var: Vector vecPos@[???]
                       Unresolved local var: QAngle vecAng@[???] */
  ___i686_get_pc_thunk_bx();
  if (args->m_nArgc < 8) {
    _Msg(unaff_EBX + 0x346560);
  }
  else {
    pCVar1 = UTIL_PlayerByName(args->m_ppArgv[1]);
    if (pCVar1 == (CBasePlayer *)0x0) {
      pcVar2 = (char *)(unaff_EBX + 0x2e2228);
      if (1 < args->m_nArgc) {
        pcVar2 = args->m_ppArgv[1];
      }
      _Msg(unaff_EBX + 0x346481,pcVar2);
    }
    else {
      pcVar2 = (char *)(unaff_EBX + 0x2e2228);
      if (4 < args->m_nArgc) {
        pcVar2 = args->m_ppArgv[4];
      }
      dVar3 = _atof(pcVar2);
      pcVar2 = (char *)(unaff_EBX + 0x2e2228);
      if (3 < args->m_nArgc) {
        pcVar2 = args->m_ppArgv[3];
      }
      dVar4 = _atof(pcVar2);
      pcVar2 = (char *)(unaff_EBX + 0x2e2228);
      if (2 < args->m_nArgc) {
        pcVar2 = args->m_ppArgv[2];
      }
      dVar5 = _atof(pcVar2);
      local_28 = (float)dVar5;
      local_24 = (float)dVar4;
      local_20 = (float)dVar3;
      pcVar2 = (char *)(unaff_EBX + 0x2e2228);
      if (7 < args->m_nArgc) {
        pcVar2 = args->m_ppArgv[7];
      }
      dVar3 = _atof(pcVar2);
      pcVar2 = (char *)(unaff_EBX + 0x2e2228);
      if (6 < args->m_nArgc) {
        pcVar2 = args->m_ppArgv[6];
      }
      dVar4 = _atof(pcVar2);
      pcVar2 = (char *)(unaff_EBX + 0x2e2228);
      if (5 < args->m_nArgc) {
        pcVar2 = args->m_ppArgv[5];
      }
      dVar5 = _atof(pcVar2);
      local_34 = (float)dVar5;
      local_30 = (float)dVar4;
      local_2c = (float)dVar3;
      (**(code **)((int)(pCVar1->super_CBaseCombatCharacter).super_CBaseFlex.
                        super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                        super_IServerEntity.super_IServerUnknown + 0x1c4))
                (pCVar1,&local_28,&local_34,0);
    }
  }
  return;
}


/* __tcf_22 at 0099a6c0 */

void __tcf_22(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x41de98),in_stack_00000008);
  return;
}


/* cc_bot_equip at 006c9790 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

void cc_bot_equip(CCommand *args)

{
  CBasePlayer *pCVar1;
  int iVar2;
  CBaseEntity *pCVar3;
  char *pcVar4;
  char *pcVar5;
  int unaff_EBX;
  
                    /* Unresolved local var: CBasePlayer * pBot@[???]
                       Unresolved local var: CBaseEntity * pExisting@[???] */
  ___i686_get_pc_thunk_bx();
  if (args->m_nArgc < 3) {
    _Msg();
    return;
  }
  pCVar1 = UTIL_PlayerByName(args->m_ppArgv[1]);
  if (pCVar1 == (CBasePlayer *)0x0) {
    pcVar4 = (char *)(unaff_EBX + 0x2e23b8);
    if (1 < args->m_nArgc) {
      pcVar4 = args->m_ppArgv[1];
    }
    _Msg(unaff_EBX + 0x346611,pcVar4);
  }
  else {
    pcVar4 = (char *)(unaff_EBX + 0x2e23b8);
    if (2 < args->m_nArgc) {
      pcVar4 = args->m_ppArgv[2];
    }
    iVar2 = (**(code **)((int)(pCVar1->super_CBaseCombatCharacter).super_CBaseFlex.
                              super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                              super_IServerEntity.super_IServerUnknown + 0x464))(pCVar1,pcVar4,0);
    if (iVar2 == 0) {
                    /* Unresolved local var: CBaseCombatWeapon * pWpn@[???] */
      pcVar4 = (char *)(unaff_EBX + 0x2e23b8);
      if (2 < args->m_nArgc) {
        pcVar4 = args->m_ppArgv[2];
      }
      pCVar3 = CreateEntityByName(pcVar4,-1,true);
      if ((pCVar3 != (CBaseEntity *)0x0) &&
         (iVar2 = ___dynamic_cast(pCVar3,*(undefined4 *)(unaff_EBX + 0x4e2424),
                                  *(undefined4 *)(unaff_EBX + 0x4e25ac),0), iVar2 != 0)) {
        (**(code **)((int)(pCVar1->super_CBaseCombatCharacter).super_CBaseFlex.
                          super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                          super_IServerEntity.super_IServerUnknown + 0x47c))(pCVar1,iVar2,0);
        return;
      }
      pcVar4 = (char *)(unaff_EBX + 0x2e23b8);
      if (2 < args->m_nArgc) {
        pcVar4 = args->m_ppArgv[2];
      }
      _Msg(unaff_EBX + 0x3466d0,pcVar4);
    }
    else {
      pcVar4 = (char *)(unaff_EBX + 0x2e23b8);
      if (2 < args->m_nArgc) {
        pcVar4 = args->m_ppArgv[2];
      }
      pcVar5 = (char *)(unaff_EBX + 0x2e23b8);
      if (1 < args->m_nArgc) {
        pcVar5 = args->m_ppArgv[1];
      }
      _Msg(unaff_EBX + 0x3466bb,pcVar5,pcVar4);
    }
  }
  return;
}


/* __tcf_23 at 0099a6a0 */

void __tcf_23(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x41def8),in_stack_00000008);
  return;
}


/* _GLOBAL__I_bot_forcefireweapon at 000c2ce0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_bot_forcefireweapon(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

