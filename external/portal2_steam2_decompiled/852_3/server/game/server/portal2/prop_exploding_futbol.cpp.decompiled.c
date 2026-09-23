/* DWARF-guided pseudocode for game/server/portal2/prop_exploding_futbol.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000c9750 */

/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *pDefaultValue;
  undefined4 uVar1;
  datamap_t *pdVar2;
  IEntityFactoryDictionary *pIVar3;
  int unaff_EBX;
  longlong lVar4;
  ConVar *in_stack_ffffffd8;
  char *in_stack_ffffffdc;
  char *in_stack_ffffffe0;
  int in_stack_ffffffe4;
  char *in_stack_ffffffe8;
  
  lVar4 = ___i686_get_pc_thunk_bx();
  if (lVar4 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2252) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2253) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2254) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2255) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2256) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x225a) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x225e) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2262) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2266) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x226a) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x226e) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2272) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2276) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x227a) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x227e) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2282) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2286) = 0;
  *(undefined1 *)((int)s_pRandomFloats + (int)(&UNK_0000228d + unaff_EBX)) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x228a) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x228b) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x228c) = 0;
  *(undefined4 *)((int)s_pRandomFloats + (int)(&UNK_0000228e + unaff_EBX)) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2295) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2292) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2293) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2294) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2296) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x229d) = 1;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x229a) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x229b) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x229c) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x229e) = 1;
  *(undefined1 *)((int)s_pRandomFloats + (int)(&UNK_000022a5 + unaff_EBX)) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x22a2) = 0;
  *(undefined1 *)((int)s_pRandomFloats + (int)(&UNK_000022a3 + unaff_EBX)) = 0;
  *(undefined1 *)((int)s_pRandomFloats + (int)(&UNK_000022a4 + unaff_EBX)) = 0;
  *(undefined4 *)((int)s_pRandomFloats + (int)(&UNK_000022a6 + unaff_EBX)) = 2;
  *(undefined1 *)((int)s_pRandomFloats + (int)(&UNK_000022ad + unaff_EBX)) = 0;
  *(undefined1 *)((int)s_pRandomFloats + (int)(&UNK_000022aa + unaff_EBX)) = 0;
  *(undefined1 *)((int)s_pRandomFloats + (int)(&UNK_000022ab + unaff_EBX)) = 0;
  *(undefined1 *)((int)s_pRandomFloats + (int)(&UNK_000022ac + unaff_EBX)) = 0;
  *(undefined **)((int)s_pRandomFloats + (int)(&UNK_000022ae + unaff_EBX)) =
       &UNK_00ae7bca + unaff_EBX;
  pDefaultValue = (char *)(unaff_EBX + 0x8e24da);
  ConVar::ConVar((ConVar *)((int)&PTR_GetActiveItem_00c66280 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x9482ee),pDefaultValue,0x4000,(char *)(unaff_EBX + 0x9482be),
                 in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0,in_stack_ffffffe4,
                 in_stack_ffffffe8);
  uVar1 = *(undefined4 *)(&DAT_00ae2432 + unaff_EBX);
  ___cxa_atexit(unaff_EBX + 0x8d27c2,0,uVar1);
  ConVar::ConVar((ConVar *)(&UNK_00c662e2 + unaff_EBX),(char *)(unaff_EBX + 0x948362),
                 (char *)(unaff_EBX + 0x94835a),0x4000,(char *)(unaff_EBX + 0x948312),
                 in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0,in_stack_ffffffe4,
                 in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x8d27a2,0,uVar1);
  ConVar::ConVar((ConVar *)((int)&PTR_IsAutoDeleteSet_00c66340 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x9483be),(char *)(unaff_EBX + 0x9235a4),0x4000,
                 (char *)(unaff_EBX + 0x948386),in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,in_stack_ffffffe4,in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x8d2782,0,uVar1);
  ConVar::ConVar((ConVar *)((int)&PTR_AddActionSignalTarget_00c663a0 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x94841e),pDefaultValue,0x4000,(char *)(unaff_EBX + 0x9483e2),
                 in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0,in_stack_ffffffe4,
                 in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x8d2762,0,uVar1);
  ConVar::ConVar((ConVar *)((int)&PTR_GetBorder_00c66400 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x948486),(char *)(unaff_EBX + 0x94847c),0x4000,
                 (char *)(unaff_EBX + 0x948446),in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,in_stack_ffffffe4,in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x8d2742,0,uVar1);
  ConVar::ConVar((ConVar *)((int)&PTR_ApplyUserConfigSettings_00c66460 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x9484ee),(char *)(unaff_EBX + 0x9484e7),0x4000,
                 (char *)(unaff_EBX + 0x9484aa),in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,in_stack_ffffffe4,in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x8d2722,0,uVar1);
  ConVar::ConVar((ConVar *)((int)&PTR_AddKeyBinding_00c664c0 + unaff_EBX + 2),
                 (char *)(vgui::TextEntry::FireActionSignal + unaff_EBX + 6),
                 (char *)(unaff_EBX + 0x948546),0x4000,(char *)(unaff_EBX + 0x94851a),
                 in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0,in_stack_ffffffe4,
                 in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x8d2702,0,uVar1);
  ConVar::ConVar((ConVar *)((int)&PTR_DrawBoxFade_00c66520 + unaff_EBX + 2),
                 &UNK_009485cf + unaff_EBX,(char *)(unaff_EBX + 0x8eec80),0x4000,
                 (char *)(unaff_EBX + 0x948576),in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,in_stack_ffffffe4,in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x8d26e2,0,uVar1);
  ConVar::ConVar((ConVar *)((int)&PTR_OnPanelExitedDroppablePanel_00c66580 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x948636),(char *)(unaff_EBX + 0x948626),0x4000,
                 (char *)(unaff_EBX + 0x9485ea),in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,in_stack_ffffffe4,in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x8d26c2,0,uVar1);
  ConVar::ConVar((ConVar *)((int)&PTR_GetDragData_00c665e0 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x9486aa),(char *)(unaff_EBX + 0x94869d),0x4000,
                 (char *)(unaff_EBX + 0x94865a),in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,in_stack_ffffffe4,in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x8d26a2,0,uVar1);
  ConVar::ConVar((ConVar *)((int)&PTR_typeinfo_00c66640 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x948712),(char *)(unaff_EBX + 0x94870a),0x4000,
                 (char *)(unaff_EBX + 0x9486ce),in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,in_stack_ffffffe4,in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x8d2682,0,uVar1);
  ConVar::ConVar((ConVar *)((int)&PTR_RequestFocusNext_00c666a0 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x94877e),&UNK_00940916 + unaff_EBX,0x4000,
                 (char *)(vgui::TextEntry::OnCreateDragData + unaff_EBX + 6),in_stack_ffffffd8,
                 in_stack_ffffffdc,in_stack_ffffffe0,in_stack_ffffffe4,in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x8d2662,0,uVar1);
  ConVar::ConVar((ConVar *)((int)&PTR_OnMove_00c66700 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x9487d6),(char *)(unaff_EBX + 0x8ea2de),0x4000,
                 (char *)(unaff_EBX + 0x94879e),in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,in_stack_ffffffe4,in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x8d2642,0,uVar1);
  ConVar::ConVar((ConVar *)((int)&PTR_GetBgColor_00c66760 + unaff_EBX + 2),
                 (char *)(unaff_EBX + 0x94882e),pDefaultValue,0x4000,(char *)(unaff_EBX + 0x9487f6),
                 in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0,in_stack_ffffffe4,
                 in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x8d2622,0,uVar1);
  pdVar2 = DataMapInit<CExplodingFutbolCatcher>((CExplodingFutbolCatcher *)0x0);
  *(datamap_t **)((int)&PTR_IsOpaque_00c667b8 + unaff_EBX + 2) = pdVar2;
  *(undefined **)((int)s_pRandomFloats + unaff_EBX + 0x22b2) = &UNK_00b7105a + unaff_EBX;
  pIVar3 = EntityFactoryDictionary();
  (**pIVar3->_vptr_IEntityFactoryDictionary)(pIVar3,unaff_EBX + 0xcf13f2,unaff_EBX + 0x948851);
  pdVar2 = DataMapInit<CPropExplodingFutbolSpawner>((CPropExplodingFutbolSpawner *)0x0);
  *(datamap_t **)((int)&PTR_SetScheme_00c667bc + unaff_EBX + 2) = pdVar2;
  *(undefined **)((int)s_pRandomFloats + unaff_EBX + 0x22b6) = &UNK_00b7107a + unaff_EBX;
  pIVar3 = EntityFactoryDictionary();
  (**pIVar3->_vptr_IEntityFactoryDictionary)(pIVar3,unaff_EBX + 0xcf13f6,unaff_EBX + 0x94886a);
  *(undefined **)((int)s_pRandomFloats + unaff_EBX + 0x22ba) = &UNK_00b7109a + unaff_EBX;
  pIVar3 = EntityFactoryDictionary();
  (**pIVar3->_vptr_IEntityFactoryDictionary)(pIVar3,unaff_EBX + 0xcf13fa,unaff_EBX + 0x948888);
  pdVar2 = DataMapInit<CPropExplodingFutbol>((CPropExplodingFutbol *)0x0);
  *(datamap_t **)((int)&PTR_SetScheme_00c667c0 + unaff_EBX + 2) = pdVar2;
  *(undefined **)((int)s_pRandomFloats + unaff_EBX + 0x22be) = &UNK_00b710ba + unaff_EBX;
  pIVar3 = EntityFactoryDictionary();
  (**pIVar3->_vptr_IEntityFactoryDictionary)(pIVar3,unaff_EBX + 0xcf13fe,unaff_EBX + 0x8efa5e);
  return;
}


/* __tcf_3 at 0099bf20 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x393ab8),in_stack_00000008);
  return;
}


/* __tcf_4 at 0099bf00 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x393b38),in_stack_00000008);
  return;
}


/* __tcf_5 at 0099bee0 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x393bb8),in_stack_00000008);
  return;
}


/* __tcf_6 at 0099bec0 */

void __tcf_6(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x393c38),in_stack_00000008);
  return;
}


/* __tcf_7 at 0099bea0 */

void __tcf_7(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x393cb8),in_stack_00000008);
  return;
}


/* __tcf_8 at 0099be80 */

void __tcf_8(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x393d38),in_stack_00000008);
  return;
}


/* __tcf_9 at 0099be60 */

void __tcf_9(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x393db8),in_stack_00000008);
  return;
}


/* __tcf_10 at 0099be40 */

void __tcf_10(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x393e38),in_stack_00000008);
  return;
}


/* __tcf_11 at 0099be20 */

void __tcf_11(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x393eb8),in_stack_00000008);
  return;
}


/* __tcf_12 at 0099be00 */

void __tcf_12(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x393f38),in_stack_00000008);
  return;
}


/* __tcf_13 at 0099bde0 */

void __tcf_13(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x393fb8),in_stack_00000008);
  return;
}


/* __tcf_14 at 0099bdc0 */

void __tcf_14(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x394038),in_stack_00000008);
  return;
}


/* __tcf_15 at 0099bda0 */

void __tcf_15(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3940b8),in_stack_00000008);
  return;
}


/* __tcf_16 at 0099bd80 */

void __tcf_16(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(&UNK_00394138 + extraout_ECX),in_stack_00000008);
  return;
}


/* CExplodingFutbolCatcher::CatchThink at 006df010 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CatchThink(CExplodingFutbolCatcher * this) */

void __thiscall CExplodingFutbolCatcher::CatchThink(CExplodingFutbolCatcher *this)

{
  int iVar1;
  int unaff_EBX;
  Vector local_d0;
  Vector local_c4 [5];
  int local_84;
  float local_7c;
  float local_78;
  float local_74;
  float local_6c;
  float local_68;
  float local_64;
  float local_5c;
  float local_58;
  float local_54;
  float local_4c;
  float local_48;
  float local_44;
  undefined4 local_3c;
  undefined1 local_38;
  undefined1 local_37;
  int local_20 [4];
  
                    /* Unresolved local var: Ray_t ray@[???]
                       Unresolved local var: CFilterOnlyExplodingFutbol filter@[???]
                       Unresolved local var: trace_t trace@[???] */
  ___i686_get_pc_thunk_bx();
  local_7c = (this->m_vecCatcherBoxOrigin).x;
  local_6c = local_7c - local_7c;
  local_78 = (this->m_vecCatcherBoxOrigin).y;
  local_68 = local_78 - local_78;
  local_74 = (this->m_vecCatcherBoxOrigin).z;
  local_64 = local_74 - local_74;
  local_3c = 0;
  local_37 = local_6c * local_6c + local_68 * local_68 + local_64 * local_64 !=
             *(float *)(unaff_EBX + 0x34b54f);
  local_54 = *(float *)(unaff_EBX + 0x34b4df);
  local_4c = ((this->m_vecCatcherBoxMaxs).x - (this->m_vecCatcherBoxMins).x) * local_54;
  local_48 = ((this->m_vecCatcherBoxMaxs).y - (this->m_vecCatcherBoxMins).y) * local_54;
  local_44 = ((this->m_vecCatcherBoxMaxs).z - (this->m_vecCatcherBoxMins).z) * local_54;
  local_38 = (double)(local_4c * local_4c + local_48 * local_48 + local_44 * local_44) <
             *(double *)(unaff_EBX + 0x34b6cf);
  local_5c = ((this->m_vecCatcherBoxMins).x + (this->m_vecCatcherBoxMaxs).x) * local_54;
  local_58 = ((this->m_vecCatcherBoxMins).y + (this->m_vecCatcherBoxMaxs).y) * local_54;
  local_54 = ((this->m_vecCatcherBoxMins).z + (this->m_vecCatcherBoxMaxs).z) * local_54;
  local_7c = local_7c + local_5c;
  local_78 = local_78 + local_58;
  local_74 = local_74 + local_54;
  local_5c = -local_5c;
  local_58 = -local_58;
  local_54 = -local_54;
  local_20[0] = unaff_EBX + 0x55b817;
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4ccbeb) + 0x14))
            ((int *)**(undefined4 **)(unaff_EBX + 0x4ccbeb),&local_7c,0x200400b,local_20,&local_d0);
  if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x4ccbe7) + 0x1c) + 0x30) != 0) {
    DebugDrawLine(&local_d0,local_c4,0xff,0,0,true,-1.0);
  }
                    /* Unresolved local var: CPropExplodingFutbol * pFutbol@[???] */
  if ((local_84 == 0) ||
     ((((local_84 != **(int **)(unaff_EBX + 0x4ccc3b) &&
        (this->m_bDisableRecaptureOnPlayerGrab == false)) &&
       (iVar1 = ___dynamic_cast(local_84,*(undefined4 *)(unaff_EBX + 0x4ccba7),unaff_EBX + 0x55b737,
                                0), iVar1 != 0)) &&
      (((*(int *)(iVar1 + 0x768) == 0 || (*(int *)(iVar1 + 0x768) == 1)) &&
       ((*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
          super_IHandleEntity._vptr_IHandleEntity[0xeb])(this,iVar1), local_84 == 0)))))) {
    this->m_bDisableRecaptureOnPlayerGrab = false;
  }
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(**(int **)(unaff_EBX + 0x4ccb7f) + 0xc) +
             *(float *)(**(int **)(unaff_EBX + 0x4ccb7f) + 0x1c),(char *)0x0);
  return;
}


/* CFilterOnlyExplodingFutbol::ShouldHitEntity at 006e1f50 */

/* DWARF original prototype: bool ShouldHitEntity(CFilterOnlyExplodingFutbol * this, IHandleEntity *
   pServerEntity, int contentsMask) */

bool __thiscall
CFilterOnlyExplodingFutbol::ShouldHitEntity
          (CFilterOnlyExplodingFutbol *this,IHandleEntity *pServerEntity,int contentsMask)

{
  char cVar1;
  bool bVar2;
  CBaseEntity *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CBaseEntity * pEntity@[???] */
  ___i686_get_pc_thunk_bx();
  cVar1 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4c9cb3) + 8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x4c9cb3),pServerEntity);
  if ((cVar1 == '\0') &&
     (this_00 = (CBaseEntity *)(*pServerEntity->_vptr_IHandleEntity[6])(pServerEntity),
     this_00 != (CBaseEntity *)0x0)) {
    if ((this_00->m_iClassname).pszValue == (char *)(unaff_EBX + 0x2d725f)) {
      return true;
    }
    bVar2 = CBaseEntity::ClassMatchesComplex(this_00,(char *)(unaff_EBX + 0x2d725f));
    return bVar2;
  }
  return false;
}


/* CFilterOnlyExplodingFutbol::GetTraceType at 006e1ce0 */

/* DWARF original prototype: TraceType_t GetTraceType(CFilterOnlyExplodingFutbol * this) */

TraceType_t __thiscall CFilterOnlyExplodingFutbol::GetTraceType(CFilterOnlyExplodingFutbol *this)

{
  return TRACE_ENTITIES_ONLY;
}


/* CPropExplodingFutbolSocket::~CPropExplodingFutbolSocket at 006e1df0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropExplodingFutbolSocket(CPropExplodingFutbolSocket * this, int
   __in_chrg) */

void __thiscall
CPropExplodingFutbolSocket::~CPropExplodingFutbolSocket
          (CPropExplodingFutbolSocket *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x5585a7);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CExplodingFutbolCatcher).m_OnFutbolCaught.super_CBaseEntityOutput,
             in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CExplodingFutbolCatcher).m_OnFutbolReleased.super_CBaseEntityOutput,
             in_stack_ffffffe8);
  CBaseAnimating::~CBaseAnimating((CBaseAnimating *)this,__in_chrg);
  return;
}


/* CExplodingFutbolCatcher::~CExplodingFutbolCatcher at 006e1e60 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CExplodingFutbolCatcher(CExplodingFutbolCatcher * this, int
   __in_chrg) */

void __thiscall
CExplodingFutbolCatcher::~CExplodingFutbolCatcher(CExplodingFutbolCatcher *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x558537);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolCaught).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolReleased).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CExplodingFutbolCatcher::~CExplodingFutbolCatcher at 006e1ee0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CExplodingFutbolCatcher(CExplodingFutbolCatcher * this, int
   __in_chrg) */

void __thiscall
CExplodingFutbolCatcher::~CExplodingFutbolCatcher(CExplodingFutbolCatcher *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5584b7);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolCaught).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolReleased).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
  return;
}


/* CPropExplodingFutbolSpawner::~CPropExplodingFutbolSpawner at 006e23c0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropExplodingFutbolSpawner(CPropExplodingFutbolSpawner * this,
   int __in_chrg) */

void __thiscall
CPropExplodingFutbolSpawner::~CPropExplodingFutbolSpawner
          (CPropExplodingFutbolSpawner *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x557bf4);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolGrabbed).super_CBaseEntityOutput,in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolSpawned).super_CBaseEntityOutput,in_stack_ffffffd8);
  (this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x557fd4);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CExplodingFutbolCatcher).m_OnFutbolCaught.super_CBaseEntityOutput,
             in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CExplodingFutbolCatcher).m_OnFutbolReleased.super_CBaseEntityOutput,
             in_stack_ffffffd8);
  CBaseAnimating::~CBaseAnimating((CBaseAnimating *)this,in_stack_ffffffd8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CPropExplodingFutbolSpawner::~CPropExplodingFutbolSpawner at 006e24b0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropExplodingFutbolSpawner(CPropExplodingFutbolSpawner * this,
   int __in_chrg) */

void __thiscall
CPropExplodingFutbolSpawner::~CPropExplodingFutbolSpawner
          (CPropExplodingFutbolSpawner *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x557b04);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolGrabbed).super_CBaseEntityOutput,in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolSpawned).super_CBaseEntityOutput,in_stack_ffffffd8);
  (this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x557ee4);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CExplodingFutbolCatcher).m_OnFutbolCaught.super_CBaseEntityOutput,
             in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CExplodingFutbolCatcher).m_OnFutbolReleased.super_CBaseEntityOutput,
             in_stack_ffffffd8);
  CBaseAnimating::~CBaseAnimating((CBaseAnimating *)this,__in_chrg);
  return;
}


/* CPropExplodingFutbolSocket::~CPropExplodingFutbolSocket at 006e2590 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropExplodingFutbolSocket(CPropExplodingFutbolSocket * this, int
   __in_chrg) */

void __thiscall
CPropExplodingFutbolSocket::~CPropExplodingFutbolSocket
          (CPropExplodingFutbolSocket *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x557e07);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CExplodingFutbolCatcher).m_OnFutbolCaught.super_CBaseEntityOutput,
             in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CExplodingFutbolCatcher).m_OnFutbolReleased.super_CBaseEntityOutput,
             in_stack_ffffffe8);
  CBaseAnimating::~CBaseAnimating((CBaseAnimating *)this,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* DataMapInit<CExplodingFutbolCatcher> at 000c90d0 */

datamap_t * DataMapInit<CExplodingFutbolCatcher>(CExplodingFutbolCatcher *param_1)

{
  undefined4 uVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  char *pcVar5;
  size_t sVar6;
  undefined4 *puVar7;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((*(char *)((int)s_pRandomFloats + unaff_EBX + 0x299c) == '\0') &&
     (iVar4 = ___cxa_guard_acquire(unaff_EBX + 0xcf1adc), iVar4 != 0)) {
    *(int *)((int)s_pRandomFloats + unaff_EBX + 0x29ac) = unaff_EBX + 0x94890a;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x29b4) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x29b8) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x29bc) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x29c0) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x29c4) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x29b0) = 0x17;
    ___cxa_guard_release(unaff_EBX + 0xcf1adc);
    ___cxa_atexit(unaff_EBX + 0x8d301c,0,*(undefined4 *)(&DAT_00ae2aac + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_NetworkStateChanged_m_flFriction_00c051b4 + unaff_EBX) =
       *(undefined4 *)(&DAT_00ae2ab8 + unaff_EBX);
  if ((*(char *)((int)s_pRandomFloats + unaff_EBX + 0x29a4) == '\0') &&
     (iVar4 = ___cxa_guard_acquire(unaff_EBX + 0xcf1ae4), iVar4 != 0)) {
    uVar1 = **(undefined4 **)(&DAT_00ae2ac8 + unaff_EBX);
    *(undefined4 *)((int)&PTR_Blocked_00c059d0 + unaff_EBX) = uVar1;
    *(undefined4 *)((int)&PTR_SetHealth_00c05a10 + unaff_EBX) = uVar1;
    pcVar5 = operator_new___(*(int *)((int)s_pRandomFloats + unaff_EBX + 0x29b0) + 0xb);
    _strcpy(pcVar5,*(char **)((int)s_pRandomFloats + unaff_EBX + 0x29ac));
    sVar6 = _strlen(pcVar5);
    builtin_strncpy(pcVar5 + sVar6,"CatchThink",0xb);
    uVar2 = *(uint *)((int)s_pRandomFloats + unaff_EBX + 0x29c0);
    CUtlVector<char*,CUtlMemory<char*,_int>_>::GrowVector
              ((CUtlVector<char*,CUtlMemory<char*,_int>_> *)
               ((int)s_pRandomFloats + unaff_EBX + 0x29b4),1);
    iVar4 = ~uVar2 + *(int *)((int)s_pRandomFloats + unaff_EBX + 0x29c0);
    if (0 < iVar4) {
      iVar3 = *(int *)((int)s_pRandomFloats + unaff_EBX + 0x29b4);
      _V_memmove((void *)(iVar3 + 4 + uVar2 * 4),(void *)(iVar3 + uVar2 * 4),iVar4 * 4);
    }
    puVar7 = (undefined4 *)(uVar2 * 4 + *(int *)((int)s_pRandomFloats + unaff_EBX + 0x29b4));
    if (puVar7 != (undefined4 *)0x0) {
      *puVar7 = pcVar5;
    }
    *(char **)((int)&PTR_GetDamage_00c05a40 + unaff_EBX) = pcVar5;
    pcVar5 = operator_new___(*(int *)((int)s_pRandomFloats + unaff_EBX + 0x29b0) + 0xd);
    _strcpy(pcVar5,*(char **)((int)s_pRandomFloats + unaff_EBX + 0x29ac));
    sVar6 = _strlen(pcVar5);
    builtin_strncpy(pcVar5 + sVar6,"CaptureThink",0xd);
    uVar2 = *(uint *)((int)s_pRandomFloats + unaff_EBX + 0x29c0);
    CUtlVector<char*,CUtlMemory<char*,_int>_>::GrowVector
              ((CUtlVector<char*,CUtlMemory<char*,_int>_> *)
               ((int)s_pRandomFloats + unaff_EBX + 0x29b4),1);
    iVar4 = ~uVar2 + *(int *)((int)s_pRandomFloats + unaff_EBX + 0x29c0);
    if (0 < iVar4) {
      iVar3 = *(int *)((int)s_pRandomFloats + unaff_EBX + 0x29b4);
      _V_memmove((void *)(iVar3 + 4 + uVar2 * 4),(void *)(iVar3 + uVar2 * 4),iVar4 * 4);
    }
    puVar7 = (undefined4 *)(uVar2 * 4 + *(int *)((int)s_pRandomFloats + unaff_EBX + 0x29b4));
    if (puVar7 != (undefined4 *)0x0) {
      *puVar7 = pcVar5;
    }
    *(char **)((int)&PTR_CanBeSeenBy_00c05a80 + unaff_EBX) = pcVar5;
    ___cxa_guard_release(unaff_EBX + 0xcf1ae4);
  }
  *(undefined4 *)((int)&PTR_NetworkStateChanged_m_vecBaseVelocity_00c051ac + unaff_EBX) = 9;
  *(int *)((int)&PTR_NetworkStateChanged_m_hGroundEntity_00c051a8 + unaff_EBX) =
       (int)&PTR_ShouldTransmit_00c0587c + unaff_EBX;
  return (datamap_t *)((int)&PTR_NetworkStateChanged_m_hGroundEntity_00c051a8 + unaff_EBX);
}


/* CExplodingFutbolCatcher::GetDataDescMap at 006de4c0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CExplodingFutbolCatcher * this) */

datamap_t * __thiscall CExplodingFutbolCatcher::GetDataDescMap(CExplodingFutbolCatcher *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5efdc4);
}


/* CExplodingFutbolCatcher::GetBaseMap at 006de4d0 */

datamap_t * CExplodingFutbolCatcher::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4cd6c4);
}


/* __tcf_0 at 0099c100 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41ead6)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41eaca) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41ead6));
  }
  *(undefined4 *)(unaff_EBX + 0x41ead6) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41ead2)) {
    if (*(int *)(unaff_EBX + 0x41eaca) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20fa8a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20fa8a),*(int *)(unaff_EBX + 0x41eaca));
      *(undefined4 *)(unaff_EBX + 0x41eaca) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41eace) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41eaca);
  *(int *)(unaff_EBX + 0x41eada) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41ead2)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20fa8a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20fa8a),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41eaca) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41eace) = 0;
  }
  return;
}


/* DataMapInit<CPropExplodingFutbolSpawner> at 000c8fa0 */

datamap_t * DataMapInit<CPropExplodingFutbolSpawner>(CPropExplodingFutbolSpawner *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)s_pRandomFloats + unaff_EBX + 0x2a9c) == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcf1bdc);
    if (iVar2 != 0) {
      *(undefined **)((int)s_pRandomFloats + unaff_EBX + 0x2aac) = &UNK_00948959 + unaff_EBX;
      *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2ab4) = 0;
      *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2ab8) = 0;
      *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2abc) = 0;
      *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2ac0) = 0;
      *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2ac4) = 0;
      *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2ab0) = 0x1b;
      ___cxa_guard_release(unaff_EBX + 0xcf1bdc);
      ___cxa_atexit(unaff_EBX + 0x8d306c,0,*(undefined4 *)(&DAT_00ae2bdc + unaff_EBX));
    }
  }
  *(int *)((int)&PTR_IsAlive_00c052fc + unaff_EBX) =
       (int)&PTR_ShouldAttractAutoAim_00c052d8 + unaff_EBX;
  if (*(char *)((int)s_pRandomFloats + unaff_EBX + 0x2aa4) == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcf1be4);
    if (iVar2 != 0) {
      uVar1 = **(undefined4 **)(&DAT_00ae2bf8 + unaff_EBX);
      *(undefined4 *)((int)&PTR_GetBaseAnimating_00c05900 + unaff_EBX) = uVar1;
      *(undefined4 *)((int)&PTR_Event_Killed_00c05940 + unaff_EBX) = uVar1;
      ___cxa_guard_release(unaff_EBX + 0xcf1be4);
    }
  }
  *(undefined4 *)((int)&PTR_OnTakeDamage_00c052f4 + unaff_EBX) = 7;
  *(int *)((int)&PTR_CanBeHitByMeleeAttack_00c052f0 + unaff_EBX) =
       (int)&PTR_VPhysicsGetObjectList_00c057ac + unaff_EBX;
  return (datamap_t *)((int)&PTR_CanBeHitByMeleeAttack_00c052f0 + unaff_EBX);
}


/* CPropExplodingFutbolSpawner::GetDataDescMap at 006de4e0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPropExplodingFutbolSpawner * this) */

datamap_t * __thiscall
CPropExplodingFutbolSpawner::GetDataDescMap(CPropExplodingFutbolSpawner *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5efdbc);
}


/* CPropExplodingFutbolSpawner::GetBaseMap at 006de4f0 */

datamap_t * CPropExplodingFutbolSpawner::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5efd94);
}


/* __tcf_1 at 0099c020 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41eb86)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41eb7a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41eb86));
  }
  *(undefined4 *)(unaff_EBX + 0x41eb86) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41eb82)) {
    if (*(int *)(unaff_EBX + 0x41eb7a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20fb6a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20fb6a),*(int *)(unaff_EBX + 0x41eb7a));
      *(undefined4 *)(unaff_EBX + 0x41eb7a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41eb7e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41eb7a);
  *(int *)(unaff_EBX + 0x41eb8a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41eb82)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20fb6a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20fb6a),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41eb7a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41eb7e) = 0;
  }
  return;
}


/* CPropExplodingFutbol::~CPropExplodingFutbol at 006e1fd0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropExplodingFutbol(CPropExplodingFutbol * this, int __in_chrg)
    */

void __thiscall
CPropExplodingFutbol::~CPropExplodingFutbol(CPropExplodingFutbol *this,int __in_chrg)

{
  CBaseEntity **ppCVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
  super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x557aa4);
  (this->super_CPhysicsProp).super_CBreakableProp.super_IBreakableWithPropData.
  _vptr_IBreakableWithPropData = (_func_int_varargs **)(unaff_EBX + 0x557f0c);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x557f90);
  (this->super_CPhysicsProp).super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(unaff_EBX + 0x557fc0);
  (this->m_ExplodedEntities).m_Size = 0;
  if ((this->m_ExplodedEntities).m_Memory.m_nGrowSize < 0) {
    ppCVar1 = (this->m_ExplodedEntities).m_Memory.m_pMemory;
  }
  else {
    ppCVar1 = (this->m_ExplodedEntities).m_Memory.m_pMemory;
    if (ppCVar1 != (CBaseEntity **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(&DAT_004c9bb4 + unaff_EBX) + 8))
                ((int *)**(undefined4 **)(&DAT_004c9bb4 + unaff_EBX),ppCVar1);
      (this->m_ExplodedEntities).m_Memory.m_pMemory = (CBaseEntity **)0x0;
    }
    ppCVar1 = (CBaseEntity **)0x0;
    (this->m_ExplodedEntities).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_ExplodedEntities).m_pElements = ppCVar1;
  if (-1 < (this->m_ExplodedEntities).m_Memory.m_nGrowSize) {
    if (ppCVar1 != (CBaseEntity **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(&DAT_004c9bb4 + unaff_EBX) + 8))
                ((int *)**(undefined4 **)(&DAT_004c9bb4 + unaff_EBX),ppCVar1);
      (this->m_ExplodedEntities).m_Memory.m_pMemory = (CBaseEntity **)0x0;
    }
    (this->m_ExplodedEntities).m_Memory.m_nAllocationCount = 0;
  }
  CPhysicsProp::~CPhysicsProp(&this->super_CPhysicsProp,__in_chrg);
  return;
}


/* CPropExplodingFutbol::~CPropExplodingFutbol at 006e2270 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropExplodingFutbol(CPropExplodingFutbol * this, int __in_chrg)
    */

void __thiscall
CPropExplodingFutbol::~CPropExplodingFutbol(CPropExplodingFutbol *this,int __in_chrg)

{
  CBaseEntity **ppCVar1;
  int unaff_EBX;
  CBaseEntity **in_stack_ffffffc8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
  super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x557804);
  (this->super_CPhysicsProp).super_CBreakableProp.super_IBreakableWithPropData.
  _vptr_IBreakableWithPropData = (_func_int_varargs **)(unaff_EBX + 0x557c6c);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x557cf0);
  (this->super_CPhysicsProp).super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(unaff_EBX + 0x557d20);
  (this->m_ExplodedEntities).m_Size = 0;
  if ((this->m_ExplodedEntities).m_Memory.m_nGrowSize < 0) {
    ppCVar1 = (this->m_ExplodedEntities).m_Memory.m_pMemory;
  }
  else {
    ppCVar1 = (this->m_ExplodedEntities).m_Memory.m_pMemory;
    if (ppCVar1 != (CBaseEntity **)0x0) {
      in_stack_ffffffc8 = ppCVar1;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4c9914) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4c9914));
      ppCVar1 = (CBaseEntity **)0x0;
      (this->m_ExplodedEntities).m_Memory.m_pMemory = (CBaseEntity **)0x0;
    }
    (this->m_ExplodedEntities).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_ExplodedEntities).m_pElements = ppCVar1;
  if (-1 < (this->m_ExplodedEntities).m_Memory.m_nGrowSize) {
    if (ppCVar1 != (CBaseEntity **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4c9914) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4c9914));
      (this->m_ExplodedEntities).m_Memory.m_pMemory = (CBaseEntity **)0x0;
      in_stack_ffffffc8 = ppCVar1;
    }
    (this->m_ExplodedEntities).m_Memory.m_nAllocationCount = 0;
  }
  CPhysicsProp::~CPhysicsProp(&this->super_CPhysicsProp,(int)in_stack_ffffffc8);
  CBaseEntity::operator_delete(this);
  return;
}


/* DataMapInit<CPropExplodingFutbol> at 000c93b0 */

datamap_t * DataMapInit<CPropExplodingFutbol>(CPropExplodingFutbol *param_1)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  char *pcVar4;
  size_t sVar5;
  undefined4 *puVar6;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((*(char *)((int)s_pRandomFloats + unaff_EBX + 0x2662) == '\0') &&
     (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xcf17a2), iVar3 != 0)) {
    *(int *)((int)s_pRandomFloats + unaff_EBX + 0x2672) = unaff_EBX + 0x948648;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x267a) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x267e) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2682) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2686) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x268a) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2676) = 0x14;
    ___cxa_guard_release(unaff_EBX + 0xcf17a2);
    ___cxa_atexit(unaff_EBX + 0x8d2b82,0,*(undefined4 *)(&DAT_00ae27d2 + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_TestHitboxes_00c04f08 + unaff_EBX + 2) =
       *(undefined4 *)(&DAT_00ae31f2 + unaff_EBX);
  if ((*(char *)((int)s_pRandomFloats + unaff_EBX + 0x266a) == '\0') &&
     (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xcf17aa), iVar3 != 0)) {
    pcVar4 = operator_new___(*(int *)((int)s_pRandomFloats + unaff_EBX + 0x2676) + 10);
    _strcpy(pcVar4,*(char **)((int)s_pRandomFloats + unaff_EBX + 0x2672));
    sVar5 = _strlen(pcVar4);
    builtin_strncpy(pcVar4 + sVar5,"AnimThink",10);
    uVar1 = *(uint *)((int)s_pRandomFloats + unaff_EBX + 0x2686);
    CUtlVector<char*,CUtlMemory<char*,_int>_>::GrowVector
              ((CUtlVector<char*,CUtlMemory<char*,_int>_> *)
               ((int)s_pRandomFloats + unaff_EBX + 0x267a),1);
    iVar3 = ~uVar1 + *(int *)((int)s_pRandomFloats + unaff_EBX + 0x2686);
    if (0 < iVar3) {
      iVar2 = *(int *)((int)s_pRandomFloats + unaff_EBX + 0x267a);
      _V_memmove((void *)(iVar2 + 4 + uVar1 * 4),(void *)(iVar2 + uVar1 * 4),iVar3 * 4);
    }
    puVar6 = (undefined4 *)(uVar1 * 4 + *(int *)((int)s_pRandomFloats + unaff_EBX + 0x267a));
    if (puVar6 != (undefined4 *)0x0) {
      *puVar6 = pcVar4;
    }
    *(char **)((int)&PTR_OnParseMapDataFinished_00c05264 + unaff_EBX + 2) = pcVar4;
    pcVar4 = operator_new___(*(int *)((int)s_pRandomFloats + unaff_EBX + 0x2676) + 0xb);
    _strcpy(pcVar4,*(char **)((int)s_pRandomFloats + unaff_EBX + 0x2672));
    sVar5 = _strlen(pcVar4);
    builtin_strncpy(pcVar4 + sVar5,"TimerThink",0xb);
    uVar1 = *(uint *)((int)s_pRandomFloats + unaff_EBX + 0x2686);
    CUtlVector<char*,CUtlMemory<char*,_int>_>::GrowVector
              ((CUtlVector<char*,CUtlMemory<char*,_int>_> *)
               ((int)s_pRandomFloats + unaff_EBX + 0x267a),1);
    iVar3 = ~uVar1 + *(int *)((int)s_pRandomFloats + unaff_EBX + 0x2686);
    if (0 < iVar3) {
      iVar2 = *(int *)((int)s_pRandomFloats + unaff_EBX + 0x267a);
      _V_memmove((void *)(iVar2 + 4 + uVar1 * 4),(void *)(iVar2 + uVar1 * 4),iVar3 * 4);
    }
    puVar6 = (undefined4 *)(uVar1 * 4 + *(int *)((int)s_pRandomFloats + unaff_EBX + 0x267a));
    if (puVar6 != (undefined4 *)0x0) {
      *puVar6 = pcVar4;
    }
    *(char **)((int)&PTR_OnSave_00c052a4 + unaff_EBX + 2) = pcVar4;
    pcVar4 = operator_new___(*(int *)((int)s_pRandomFloats + unaff_EBX + 0x2676) + 10);
    _strcpy(pcVar4,*(char **)((int)s_pRandomFloats + unaff_EBX + 0x2672));
    sVar5 = _strlen(pcVar4);
    builtin_strncpy(pcVar4 + sVar5,"KillThink",10);
    uVar1 = *(uint *)((int)s_pRandomFloats + unaff_EBX + 0x2686);
    CUtlVector<char*,CUtlMemory<char*,_int>_>::GrowVector
              ((CUtlVector<char*,CUtlMemory<char*,_int>_> *)
               ((int)s_pRandomFloats + unaff_EBX + 0x267a),1);
    iVar3 = ~uVar1 + *(int *)((int)s_pRandomFloats + unaff_EBX + 0x2686);
    if (0 < iVar3) {
      iVar2 = *(int *)((int)s_pRandomFloats + unaff_EBX + 0x267a);
      _V_memmove((void *)(iVar2 + 4 + uVar1 * 4),(void *)(iVar2 + uVar1 * 4),iVar3 * 4);
    }
    puVar6 = (undefined4 *)(uVar1 * 4 + *(int *)((int)s_pRandomFloats + unaff_EBX + 0x267a));
    if (puVar6 != (undefined4 *)0x0) {
      *puVar6 = pcVar4;
    }
    *(char **)((int)&PTR_GetBeamTraceFilter_00c052e4 + unaff_EBX + 2) = pcVar4;
    ___cxa_guard_release(unaff_EBX + 0xcf17aa);
  }
  *(undefined4 *)((int)&PTR_GetAIAddOn_00c04f00 + unaff_EBX + 2) = 0x10;
  *(int *)((int)&PTR_GetScriptDesc_00c04efc + unaff_EBX + 2) = unaff_EBX + 0xc04f62;
  return (datamap_t *)((int)&PTR_GetScriptDesc_00c04efc + unaff_EBX + 2);
}


/* CPropExplodingFutbol::GetDataDescMap at 006de500 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPropExplodingFutbol * this) */

datamap_t * __thiscall CPropExplodingFutbol::GetDataDescMap(CPropExplodingFutbol *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5efdb4);
}


/* CPropExplodingFutbol::GetBaseMap at 006de510 */

datamap_t * CPropExplodingFutbol::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4ce098);
}


/* __tcf_2 at 0099bf40 */

void __tcf_2(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41ec36)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41ec2a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41ec36));
  }
  *(undefined4 *)(unaff_EBX + 0x41ec36) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41ec32)) {
    if (*(int *)(unaff_EBX + 0x41ec2a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20fc4a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20fc4a),*(int *)(unaff_EBX + 0x41ec2a));
      *(undefined4 *)(unaff_EBX + 0x41ec2a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41ec2e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41ec2a);
  *(int *)(unaff_EBX + 0x41ec3a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41ec32)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20fc4a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20fc4a),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41ec2a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41ec2e) = 0;
  }
  return;
}


/* CPropExplodingFutbol::CPropExplodingFutbol at 006dfd80 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CPropExplodingFutbol(CPropExplodingFutbol * this) */

void __thiscall CPropExplodingFutbol::CPropExplodingFutbol(CPropExplodingFutbol *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CPhysicsProp::CPhysicsProp(&this->super_CPhysicsProp);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
  super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x559cfb);
  (this->super_CPhysicsProp).super_CBreakableProp.super_IBreakableWithPropData.
  _vptr_IBreakableWithPropData = (_func_int_varargs **)(unaff_EBX + 0x55a163);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x55a1e7);
  (this->super_CPhysicsProp).super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(unaff_EBX + 0x55a217);
  this->m_Holder = EXPLODING_FUTBOL_HELD_BY_NONE;
  (this->m_hSpawner).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_strSpawnerName).pszValue = (char *)0x0;
  (this->m_hLastHeldByPlayer).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_ExplodedEntities).m_Memory.m_pMemory = (CBaseEntity **)0x0;
  (this->m_ExplodedEntities).m_Memory.m_nAllocationCount = 0;
  (this->m_ExplodedEntities).m_Memory.m_nGrowSize = 0;
  (this->m_ExplodedEntities).m_Size = 0;
  (this->m_ExplodedEntities).m_pElements = (CBaseEntity **)0x0;
  this->m_bTimerActive = false;
  this->m_flExplosionTimer = 0.0;
  this->m_flTotalTimer = 0.0;
  this->m_flLastTickTime = 0.0;
  this->m_flLastTimerSoundTime = 0.0;
  this->m_flLastFlashTime = 0.0;
  return;
}


/* CPropExplodingFutbol::CPropExplodingFutbol at 006dfe60 */

/* DWARF original prototype: void CPropExplodingFutbol(CPropExplodingFutbol * this,
   CPropExplodingFutbol * this) */

void __thiscall
CPropExplodingFutbol::CPropExplodingFutbol(CPropExplodingFutbol *this,CPropExplodingFutbol *this_1)

{
  CPropExplodingFutbol(this);
  return;
}


/* CEntityFactory<CPropExplodingFutbol>::Create at 006e2220 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CPropExplodingFutbol> *
   this, char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CPropExplodingFutbol>::Create
          (CEntityFactory<CPropExplodingFutbol> *this,char *pClassName)

{
  CPropExplodingFutbol *this_00;
  
                    /* Unresolved local var: CPropExplodingFutbol * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x7a4);
  CPropExplodingFutbol::CPropExplodingFutbol(this_00);
  (**(code **)((int)(this_00->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x74))(this_00,pClassName);
  return &(this_00->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
          super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CPropExplodingFutbol::Precache at 006dec00 */

/* DWARF original prototype: void Precache(CPropExplodingFutbol * this) */

void __thiscall CPropExplodingFutbol::Precache(CPropExplodingFutbol *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CPhysicsProp::Precache(&this->super_CPhysicsProp);
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x33275c));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x332cb4));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x332cd4));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x332cea));
  return;
}


/* CPropExplodingFutbol::Spawn at 006dede0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CPropExplodingFutbol * this) */

void __thiscall CPropExplodingFutbol::Spawn(CPropExplodingFutbol *this)

{
  uint *puVar1;
  uint uVar2;
  CBaseEdict *this_00;
  BASEPTR func;
  int iVar3;
  CBaseEntity *pCVar4;
  int *piVar5;
  ulong *puVar6;
  IChangeInfoAccessor *pIVar7;
  char *pcVar8;
  int unaff_EBX;
  uint uVar9;
  
  ___i686_get_pc_thunk_bx();
  (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x68))(this);
  (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x80))(this,unaff_EBX + 0x3264bb,unaff_EBX + 0x33257b);
  if (this->m_bShouldRespawn != false) {
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
    uVar2 = (this->m_hSpawner).super_CBaseHandle.m_Index;
    if (((uVar2 == 0xffffffff) ||
        (iVar3 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4ccdb7),
        *(uint *)(iVar3 + 8) != uVar2 >> 0x10)) || (*(int *)(iVar3 + 4) == 0)) {
      pcVar8 = (this->m_strSpawnerName).pszValue;
      if (pcVar8 == (char *)0x0) {
        _Warning(unaff_EBX + 0x332b93);
      }
      else {
        pCVar4 = CGlobalEntityList::FindEntityByName
                           (*(CGlobalEntityList **)(unaff_EBX + 0x4cce73),(CBaseEntity *)0x0,pcVar8,
                            (CBaseEntity *)0x0,(CBaseEntity *)0x0,(CBaseEntity *)0x0,
                            (IEntityFindFilter *)0x0);
        if (pCVar4 != (CBaseEntity *)0x0) {
          piVar5 = (int *)___dynamic_cast(pCVar4,*(undefined4 *)(unaff_EBX + 0x4ccddb),
                                          unaff_EBX + 0x55b983,0);
          if (piVar5 == (int *)0x0) {
            pcVar8 = CBaseEntity::GetDebugName((CBaseEntity *)0x0);
            iVar3 = section_00000054.segname._4_4_;
            if (section_00000054.segname._4_4_ == 0) {
              iVar3 = unaff_EBX + 0x2ccd6f;
            }
            _Warning(unaff_EBX + 0x332b3f,iVar3,pcVar8);
          }
          else {
            puVar6 = (ulong *)(**(code **)(*piVar5 + 0xc))(piVar5);
            (this->m_hSpawner).super_CBaseHandle.m_Index = *puVar6;
          }
        }
      }
    }
  }
  CPhysicsProp::Spawn(&this->super_CPhysicsProp);
  iVar3 = CBaseAnimating::LookupSequence((CBaseAnimating *)this,(char *)(unaff_EBX + 0x332bd3));
  CBaseAnimating::ResetSequence((CBaseAnimating *)this,iVar3);
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x58d);
  CBaseEntity::ThinkSet
            ((CBaseEntity *)this,func,
             *(float *)(unaff_EBX + 0x34b527) + *(float *)(**(int **)(unaff_EBX + 0x4ccdb3) + 0xc),
             *(char **)(unaff_EBX + 0x5ef493));
  uVar2 = (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
          super_CBaseEntity.m_spawnflags.m_Value;
  uVar9 = uVar2 | 0x140;
  if (uVar2 != uVar9) {
    if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
        false) {
      this_00 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                  super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar7 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar7->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = (uint *)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
      *puVar1 = *puVar1 | 1;
    }
    (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
    super_CBaseEntity.m_spawnflags.m_Value = uVar9;
  }
  return;
}


/* CPropExplodingFutbol::GetHolder at 006de520 */

/* DWARF original prototype: ExplodingFutbolHolderType_t GetHolder(CPropExplodingFutbol * this) */

ExplodingFutbolHolderType_t __thiscall CPropExplodingFutbol::GetHolder(CPropExplodingFutbol *this)

{
  return this->m_Holder;
}


/* CPropExplodingFutbol::SetHolder at 006de530 */

/* DWARF original prototype: void SetHolder(CPropExplodingFutbol * this, ExplodingFutbolHolderType_t
   holder) */

void __thiscall
CPropExplodingFutbol::SetHolder(CPropExplodingFutbol *this,ExplodingFutbolHolderType_t holder)

{
  this->m_Holder = holder;
  return;
}


/* CPropExplodingFutbolSpawner::SpawnFutbol at 006dece0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void SpawnFutbol(CPropExplodingFutbolSpawner * this) */

void __thiscall CPropExplodingFutbolSpawner::SpawnFutbol(CPropExplodingFutbolSpawner *this)

{
  CBaseEntity *this_00;
  int unaff_EBX;
  Vector local_28 [2];
  
                    /* Unresolved local var: CPropExplodingFutbol * pFutbol@[???]
                       Unresolved local var: Vector vecFutbolSpawnPoint@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CreateEntityByName((char *)(unaff_EBX + 0x2da4c8),-1,true);
  if (this_00 != (CBaseEntity *)0x0) {
    COutputEvent::FireOutput(&this->m_OnFutbolSpawned,(CBaseEntity *)this,(CBaseEntity *)this,0.0);
    CBaseAnimating::GetAttachment
              ((CBaseAnimating *)this,(char *)(unaff_EBX + 0x33268c),local_28,(Vector *)0x0,
               (Vector *)0x0,(Vector *)0x0);
    CBaseEntity::SetAbsOrigin(this_00,local_28);
    (*(this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
      [0x117])(this_00,this);
    this_00[1].m_ScriptScope.m_FuncHandles.
    super_CUtlVector<HSCRIPT__**,CUtlMemoryConservative<HSCRIPT__**>_>.m_Memory.m_pMemory =
         (HSCRIPT **)0x2;
    this->m_bHasFutbol = true;
    DispatchSpawn(this_00,true);
    CBaseEntity::UpdateObjectCapsCache(this_00);
    (*(this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.
      super_IServerEntity.super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xeb])
              (this,this_00);
  }
  return;
}


/* CExplodingFutbolCatcher::CaptureFutbol at 006e19d0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CaptureFutbol(CExplodingFutbolCatcher * this, CPropExplodingFutbol
   * pFutbol) */

void __thiscall
CExplodingFutbolCatcher::CaptureFutbol(CExplodingFutbolCatcher *this,CPropExplodingFutbol *pFutbol)

{
  undefined1 *puVar1;
  uint8 uVar2;
  uint8 uVar3;
  uint8 uVar4;
  IPhysicsObject *pIVar5;
  uint uVar6;
  CBaseEdict *pCVar7;
  CPropExplodingFutbolSpawner *this_00;
  ulong *puVar8;
  int iVar9;
  CBaseEntity *pCaller;
  IChangeInfoAccessor *pIVar10;
  CPropIndicatorPanel *this_01;
  int unaff_EBX;
  
                    /* Unresolved local var: IPhysicsObject * pPhysicsObject@[???]
                       Unresolved local var: CPropExplodingFutbolSpawner * pSpawner@[???] */
  ___i686_get_pc_thunk_bx();
  if (pFutbol != (CPropExplodingFutbol *)0x0) {
    puVar8 = (ulong *)(**(code **)((int)(pFutbol->super_CPhysicsProp).super_CBreakableProp.
                                        super_CBaseProp.super_CBaseAnimating.super_CBaseEntity.
                                        super_IServerEntity.super_IServerUnknown + 0xc))(pFutbol);
    (this->m_hCaughtFutbol).super_CBaseHandle.m_Index = *puVar8;
    pIVar5 = (pFutbol->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating
             .super_CBaseEntity.m_pPhysicsObject;
    if (pIVar5 != (IPhysicsObject *)0x0) {
      (*pIVar5->_vptr_IPhysicsObject[0x10])(pIVar5,0);
    }
    (**(code **)((int)(pFutbol->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                      super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
                      super_IServerUnknown + 0x1c4))
              (pFutbol,&this->m_vecCatcherBoxOrigin,*(undefined4 *)(unaff_EBX + 0x4ca266),
               *(undefined4 *)(unaff_EBX + 0x4ca1d2));
    pFutbol->m_Holder = EXPLODING_FUTBOL_HELD_BY_CATCHER;
    uVar6 = (pFutbol->m_hSpawner).super_CBaseHandle.m_Index;
    if (((uVar6 != 0xffffffff) &&
        (iVar9 = (uVar6 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4ca1c6),
        *(uint *)(iVar9 + 8) == uVar6 >> 0x10)) &&
       (this_00 = *(CPropExplodingFutbolSpawner **)(iVar9 + 4),
       this_00 != (CPropExplodingFutbolSpawner *)0x0)) {
                    /* Unresolved local var: CPropIndicatorPanel * pTimerPanel@[???] */
      this_01 = CPropExplodingFutbolSpawner::GetTimerPanel(this_00);
      if (this_01 != (CPropIndicatorPanel *)0x0) {
        CPropIndicatorPanel::ResetTimer(this_01);
      }
    }
    pFutbol->m_bTimerActive = false;
                    /* Unresolved local var: uchar * pColorElement@[???] */
    iVar9 = *(int *)(unaff_EBX + 0x64e25e);
    uVar2 = *(uint8 *)(iVar9 + 0x32);
    uVar3 = *(uint8 *)(iVar9 + 0x31);
    uVar4 = *(uint8 *)(iVar9 + 0x30);
    if (uVar4 != (pFutbol->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                 super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                 super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r) {
      if ((pFutbol->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
          super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
        pCVar7 = &((pFutbol->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                   super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
        if (pCVar7 != (CBaseEdict *)0x0) {
          pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 0x101;
          pIVar10 = CBaseEdict::GetChangeAccessor(pCVar7);
          pIVar10->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(pFutbol->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                  super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (pFutbol->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
      super_CBaseEntity.m_clrRender.
      super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r = uVar4;
    }
    if (uVar3 != (pFutbol->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                 super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                 super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g) {
      if ((pFutbol->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
          super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
        pCVar7 = &((pFutbol->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                   super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
        if (pCVar7 != (CBaseEdict *)0x0) {
          pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 0x101;
          pIVar10 = CBaseEdict::GetChangeAccessor(pCVar7);
          pIVar10->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(pFutbol->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                  super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (pFutbol->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
      super_CBaseEntity.m_clrRender.
      super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g = uVar3;
    }
    if (uVar2 != (pFutbol->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                 super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                 super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b) {
      if ((pFutbol->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
          super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
        pCVar7 = &((pFutbol->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                   super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
        if (pCVar7 != (CBaseEdict *)0x0) {
          pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 0x101;
          pIVar10 = CBaseEdict::GetChangeAccessor(pCVar7);
          pIVar10->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(pFutbol->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                  super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (pFutbol->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
      super_CBaseEntity.m_clrRender.
      super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b = uVar2;
    }
    uVar6 = (pFutbol->m_hLastHeldByPlayer).super_CBaseHandle.m_Index;
    if ((uVar6 == 0xffffffff) ||
       (iVar9 = (uVar6 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4ca1c6),
       *(uint *)(iVar9 + 8) != uVar6 >> 0x10)) {
      pCaller = (CBaseEntity *)0x0;
    }
    else {
      pCaller = *(CBaseEntity **)(iVar9 + 4);
    }
    COutputEvent::FireOutput(&this->m_OnFutbolCaught,(CBaseEntity *)pFutbol,pCaller,0.0);
    CBaseEntity::ThinkSet((CBaseEntity *)this,(BASEPTR)0x3a9,0.0,(char *)0x0);
  }
  return;
}


/* CPropExplodingFutbol::GetLastPlayerToHold at 006de550 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: CPortal_Player * GetLastPlayerToHold(CPropExplodingFutbol * this) */

CPortal_Player * __thiscall CPropExplodingFutbol::GetLastPlayerToHold(CPropExplodingFutbol *this)

{
  uint uVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->m_hLastHeldByPlayer).super_CBaseHandle.m_Index;
  if ((uVar1 != 0xffffffff) &&
     (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4cd64b),
     *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) {
    return *(CPortal_Player **)(iVar2 + 4);
  }
  return (CPortal_Player *)0x0;
}


/* CPropExplodingFutbol::SetSpawner at 006dfd30 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void SetSpawner(CPropExplodingFutbol * this,
   CPropExplodingFutbolSpawner * pSpawner) */

void __thiscall
CPropExplodingFutbol::SetSpawner(CPropExplodingFutbol *this,CPropExplodingFutbolSpawner *pSpawner)

{
  ulong *puVar1;
  
  if (pSpawner == (CPropExplodingFutbolSpawner *)0x0) {
    (this->m_hSpawner).super_CBaseHandle.m_Index = 0xffffffff;
  }
  else {
    puVar1 = (ulong *)(*(pSpawner->super_CExplodingFutbolCatcher).super_CBaseAnimating.
                        super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
                        super_IHandleEntity._vptr_IHandleEntity[3])(pSpawner);
    (this->m_hSpawner).super_CBaseHandle.m_Index = *puVar1;
  }
  this->m_bShouldRespawn = true;
  this->m_bExplodeOnTouch = true;
  return;
}


/* CPropExplodingFutbol::GetSpawner at 006de5a0 */

/* DWARF original prototype: CPropExplodingFutbolSpawner * GetSpawner(CPropExplodingFutbol * this)
    */

CPropExplodingFutbolSpawner * __thiscall
CPropExplodingFutbol::GetSpawner(CPropExplodingFutbol *this)

{
  uint uVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->m_hSpawner).super_CBaseHandle.m_Index;
  if ((uVar1 != 0xffffffff) &&
     (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4cd5fb),
     *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) {
    return *(CPropExplodingFutbolSpawner **)(iVar2 + 4);
  }
  return (CPropExplodingFutbolSpawner *)0x0;
}


/* CPropExplodingFutbol::InputExplode at 006df900 */

/* DWARF original prototype: void InputExplode(CPropExplodingFutbol * this, inputdata_t * in) */

void __thiscall CPropExplodingFutbol::InputExplode(CPropExplodingFutbol *this,inputdata_t *in)

{
  KillFutbol(this);
  return;
}


/* CPropExplodingFutbol::KillFutbol at 006df7f0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void KillFutbol(CPropExplodingFutbol * this) */

void __thiscall CPropExplodingFutbol::KillFutbol(CPropExplodingFutbol *this)

{
  CTakeDamageInfo *in_stack_ffffff6c;
  CBaseEntity *in_stack_ffffff70;
  CBaseEntity *in_stack_ffffff74;
  float in_stack_ffffff78;
  vec_t in_stack_ffffff7c;
  vec_t in_stack_ffffff80;
  
                    /* Unresolved local var: CTakeDamageInfo info@[???] */
  ___i686_get_pc_thunk_bx();
  CTakeDamageInfo::CTakeDamageInfo
            ((CTakeDamageInfo *)&stack0xffffff7c,(CBaseEntity *)this,(CBaseEntity *)this,
             (float)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.m_iHealth.m_Value,0x40,0,
             in_stack_ffffff6c,in_stack_ffffff70,in_stack_ffffff74,in_stack_ffffff78,
             (int)in_stack_ffffff7c,(int)in_stack_ffffff80);
  if (((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
       super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
                    /* Unresolved local var: matrix3x4_t * mat@[???] */
  if (((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
       super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
                    /* Unresolved local var: Vector res@[???] */
  (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x118))(this,(CTakeDamageInfo *)&stack0xffffff7c);
  return;
}


/* CPropExplodingFutbol::DestroyFutbol at 006e0de0 */

/* DWARF original prototype: void DestroyFutbol(CPropExplodingFutbol * this, bool bExplode) */

void __thiscall CPropExplodingFutbol::DestroyFutbol(CPropExplodingFutbol *this,bool bExplode)

{
  uint uVar1;
  int *piVar2;
  int iVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (bExplode) {
    ExplodeFutbol(this);
  }
  if (this->m_bShouldRespawn != false) {
    uVar1 = (this->m_hSpawner).super_CBaseHandle.m_Index;
    if (((uVar1 != 0xffffffff) &&
        (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(&DAT_004cadb3 + unaff_EBX),
        *(uint *)(iVar3 + 8) == uVar1 >> 0x10)) &&
       (piVar2 = *(int **)(iVar3 + 4), piVar2 != (int *)0x0)) {
                    /* WARNING: Could not recover jumptable at 0x006e0e4b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (**(code **)(*piVar2 + 0x3b0))();
      return;
    }
  }
  return;
}


/* CPropExplodingFutbol::OnFizzled at 006e1930 */

/* DWARF original prototype: void OnFizzled(CPropExplodingFutbol * this) */

void __thiscall CPropExplodingFutbol::OnFizzled(CPropExplodingFutbol *this)

{
  uint uVar1;
  int *piVar2;
  int iVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::OnFizzled((CBaseAnimating *)this);
  if (*(int *)(*(int *)(unaff_EBX + 0x64e59b) + 0x30) != 0) {
    ExplodeFutbol(this);
  }
  if (this->m_bShouldRespawn != false) {
    uVar1 = (this->m_hSpawner).super_CBaseHandle.m_Index;
    if (((uVar1 != 0xffffffff) &&
        (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4ca263),
        *(uint *)(iVar3 + 8) == uVar1 >> 0x10)) &&
       (piVar2 = *(int **)(iVar3 + 4), piVar2 != (int *)0x0)) {
                    /* WARNING: Could not recover jumptable at 0x006e19bb. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (**(code **)(*piVar2 + 0x3b0))();
      return;
    }
  }
  return;
}


/* CPropExplodingFutbol::Event_Killed at 006e18a0 */

/* DWARF original prototype: void Event_Killed(CPropExplodingFutbol * this, CTakeDamageInfo * info)
    */

void __thiscall CPropExplodingFutbol::Event_Killed(CPropExplodingFutbol *this,CTakeDamageInfo *info)

{
  uint uVar1;
  int *piVar2;
  int iVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBreakableProp::Event_Killed((CBreakableProp *)this,info);
  ExplodeFutbol(this);
  if (this->m_bShouldRespawn != false) {
    uVar1 = (this->m_hSpawner).super_CBaseHandle.m_Index;
    if (((uVar1 != 0xffffffff) &&
        (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4ca2f3),
        *(uint *)(iVar3 + 8) == uVar1 >> 0x10)) &&
       (piVar2 = *(int **)(iVar3 + 4), piVar2 != (int *)0x0)) {
                    /* WARNING: Could not recover jumptable at 0x006e192b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (**(code **)(*piVar2 + 0x3b0))();
      return;
    }
  }
  return;
}


/* CPropExplodingFutbol::ExplodeFutbol at 006e0b80 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ExplodeFutbol(CPropExplodingFutbol * this) */

void __thiscall CPropExplodingFutbol::ExplodeFutbol(CPropExplodingFutbol *this)

{
  Vector *center;
  float flRadius;
  float flExplosionForce;
  uint uVar1;
  int iVar2;
  int unaff_EBX;
  QAngle *angles;
  int iVar3;
  float flExplosionRadius;
  CBaseEntity *local_40;
  PortalRadiusExtensionVector local_30;
  
                    /* Unresolved local var: PortalRadiusExtensionVector
                       portalRadiusExtensions@[???] */
  ___i686_get_pc_thunk_bx();
  flRadius = *(float *)(*(int *)(unaff_EBX + 0x64ef2e) + 0x2c);
  (this->m_ExplodedEntities).m_Size = 0;
  flExplosionForce = *(float *)(*(int *)(unaff_EBX + 0x64efee) + 0x2c);
  iVar3 = *(int *)(*(int *)(unaff_EBX + 0x64ef8e) + 0x30);
  uVar1 = (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
          super_CBaseEntity.m_hOwnerEntity.
          super_CNetworkVarBase<CBaseHandle,CBaseEntity::NetworkVar_m_hOwnerEntity>.m_Value.m_Index;
  if ((uVar1 == 0xffffffff) ||
     (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4cb016),
     *(uint *)(iVar2 + 8) != uVar1 >> 0x10)) {
    local_40 = (CBaseEntity *)0x0;
  }
  else {
    local_40 = *(CBaseEntity **)(iVar2 + 4);
  }
  if (((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
       super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    if (((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
         super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
  }
  angles = &(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
            super_CBaseEntity.m_angAbsRotation;
  center = &(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
            super_CBaseEntity.m_vecAbsOrigin;
  ExplosionCreate(center,angles,local_40,iVar3,(int)flRadius,0x428,flExplosionForce,
                  (CBaseEntity *)this,-1,(EHANDLE *)0x0,CLASS_NONE);
  local_30.m_Memory.m_pMemory = (PortalRadiusExtension_t *)0x0;
  local_30.m_Memory.m_nAllocationCount = 0;
  local_30.m_Memory.m_nGrowSize = 0;
  local_30.m_Size = 0;
  local_30.m_pElements = (PortalRadiusExtension_t *)0x0;
  if (((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
       super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    if (((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
         super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
  }
  ExtendRadiusThroughPortals(center,angles,flRadius,&local_30);
                    /* Unresolved local var: int i@[???] */
  if (0 < local_30.m_Size) {
    iVar3 = 0;
    do {
      DoExplosion(this,&local_30.m_Memory.m_pMemory[iVar3].vecOrigin,flRadius,
                  local_30.m_Memory.m_pMemory[iVar3].pPortalTo);
      iVar3 = iVar3 + 1;
    } while (iVar3 < local_30.m_Size);
  }
  if (-1 < local_30.m_Memory.m_nGrowSize) {
    local_30.m_Size = 0;
    if (local_30.m_Memory.m_pMemory != (PortalRadiusExtension_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4cb00a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4cb00a),local_30.m_Memory.m_pMemory);
    }
  }
  return;
}


/* CPropExplodingFutbol::DamageEntity at 006df2d0 */

/* DWARF original prototype: void DamageEntity(CPropExplodingFutbol * this, CBaseEntity * pEntity,
   Vector * vecCenter, Vector * vecForward) */

void __thiscall
CPropExplodingFutbol::DamageEntity
          (CPropExplodingFutbol *this,CBaseEntity *pEntity,Vector *vecCenter,Vector *vecForward)

{
  bool bVar1;
  int iVar2;
  int unaff_EBX;
  float fVar3;
  float fVar4;
  float fVar5;
  double in_stack_fffffe1c;
  CBaseEntity *in_stack_fffffe24;
  int in_stack_fffffe30;
  CTakeDamageInfo local_1cc;
  char *local_17c;
  undefined4 uStack_178;
  undefined4 uStack_174;
  undefined4 uStack_170;
  Vector local_168;
  float local_15c;
  float local_158;
  float local_154;
  float local_150;
  float local_14c;
  float local_148;
  float local_144;
  float local_140;
  float local_13c;
  Vector local_138;
  CFmtStrN<256> local_12c;
  int local_20;
  
                    /* Unresolved local var: Vector vecDamagePos@[???] */
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(unaff_EBX + 0x4cc90b);
                    /* Unresolved local var: Vector res@[???] */
  local_13c = *(float *)(unaff_EBX + 0x34b25f);
  local_144 = vecForward->x * local_13c;
  local_140 = vecForward->y * local_13c;
  local_13c = local_13c * vecForward->z;
                    /* Unresolved local var: Vector res@[???] */
  local_150 = vecCenter->x - local_144;
  local_14c = vecCenter->y - local_140;
  local_148 = vecCenter->z - local_13c;
  local_138.x = local_150;
  local_138.y = local_14c;
  local_138.z = local_148;
  iVar2 = (*(pEntity->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
            _vptr_IHandleEntity[0x55])(pEntity);
  if ((char)iVar2 != '\0') {
    if ((pEntity->m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition(pEntity);
    }
                    /* Unresolved local var: Vector delta@[???] */
    fVar5 = vecCenter->x - (pEntity->m_vecAbsOrigin).x;
    fVar3 = vecCenter->y - (pEntity->m_vecAbsOrigin).y;
    fVar4 = vecCenter->z - (pEntity->m_vecAbsOrigin).z;
                    /* Unresolved local var: __m128 root@[???] */
    local_17c = (char *)SQRT(fVar5 * fVar5 + fVar3 * fVar3 + fVar4 * fVar4);
    uStack_178 = 0;
    uStack_174 = 0;
    uStack_170 = 0;
    fVar3 = *(float *)(*(int *)(unaff_EBX + 0x65089b) + 0x2c);
    fVar4 = (float)local_17c / *(float *)(*(int *)(unaff_EBX + 0x6507db) + 0x2c);
    fVar3 = fVar3 + (fVar3 * *(float *)(*(int *)(unaff_EBX + 0x6508fb) + 0x2c) - fVar3) * fVar4;
    if (*(int *)(*(int *)(unaff_EBX + 0x65071b) + 0x30) != 0) {
      local_12c._vptr_CFmtStrN = (_func_int_varargs **)(&UNK_004ceec7 + unaff_EBX);
      local_12c.m_bQuietTruncation = true;
      local_12c.m_szBuf[0] = '\0';
      local_12c.m_nLength = 0;
      in_stack_fffffe1c = (double)fVar3;
      CFmtStrN<256>::sprintf
                (&local_12c,(char *)(unaff_EBX + 0x3326e3),(double)(float)local_17c,(double)fVar4);
      if ((pEntity->m_iEFlags & 0x800) != 0) {
        CBaseEntity::CalcAbsolutePosition(pEntity);
      }
      NDebugOverlay::Text(&pEntity->m_vecAbsOrigin,local_12c.m_szBuf,true,10.0);
    }
    CTakeDamageInfo::CTakeDamageInfo
              (&local_1cc,(CBaseEntity *)this,(CBaseEntity *)this,fVar3,0x40,0,
               SUB84(in_stack_fffffe1c,0),(CBaseEntity *)((ulonglong)in_stack_fffffe1c >> 0x20),
               in_stack_fffffe24,(float)vecForward,(int)pEntity,(int)fVar3);
    local_1cc.m_vecDamagePosition.x = local_138.x;
    local_1cc.m_vecDamagePosition.y = local_138.y;
    local_1cc.m_vecDamagePosition.z = local_138.z;
    CBaseEntity::TakeDamage(pEntity,&local_1cc);
    goto LAB_006df4d9;
  }
  if (*(int *)(*(int *)(unaff_EBX + 0x650b9b) + 0x30) == 0) {
LAB_006df541:
    CTakeDamageInfo::CTakeDamageInfo
              (&local_1cc,(CBaseEntity *)this,(CBaseEntity *)this,
               *(float *)(*(int *)(unaff_EBX + 0x65089b) + 0x2c),0x40,0,SUB84(in_stack_fffffe1c,0),
               (CBaseEntity *)((ulonglong)in_stack_fffffe1c >> 0x20),in_stack_fffffe24,
               (float)vecForward,(int)pEntity,in_stack_fffffe30);
    local_1cc.m_vecDamagePosition.x = local_138.x;
    local_1cc.m_vecDamagePosition.y = local_138.y;
    local_1cc.m_vecDamagePosition.z = local_138.z;
    if ((pEntity->m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition(pEntity);
    }
                    /* Unresolved local var: Vector res@[???] */
    local_168.x = (pEntity->m_vecAbsOrigin).x - local_138.x;
    local_168.y = (pEntity->m_vecAbsOrigin).y - local_138.y;
    local_168.z = (pEntity->m_vecAbsOrigin).z - local_138.z;
    CalculateExplosiveDamageForce(&local_1cc,&local_168,&local_138,1.0);
    (*(pEntity->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
      [0xa3])(pEntity,&local_1cc);
  }
  else {
    local_17c = (char *)(unaff_EBX + 0x31b937);
    if ((pEntity->m_iClassname).pszValue != local_17c) {
      bVar1 = CBaseEntity::ClassMatchesComplex(pEntity,local_17c);
      if (!bVar1) goto LAB_006df541;
    }
                    /* Unresolved local var: CTakeDamageInfo info@[???] */
    CTakeDamageInfo::CTakeDamageInfo
              (&local_1cc,(CBaseEntity *)this,(CBaseEntity *)this,
               (float)(pEntity->m_iHealth).m_Value,0x40,0,SUB84(in_stack_fffffe1c,0),
               (CBaseEntity *)((ulonglong)in_stack_fffffe1c >> 0x20),in_stack_fffffe24,
               (float)vecForward,(int)pEntity,in_stack_fffffe30);
    local_1cc.m_vecDamagePosition.x = vecCenter->x;
    local_1cc.m_vecDamagePosition.y = vecCenter->y;
    local_1cc.m_vecDamagePosition.z = vecCenter->z;
    local_1cc.m_vecDamageForce.z = *(float *)(*(int *)(unaff_EBX + 0x65089b) + 0x2c);
                    /* Unresolved local var: Vector res@[???] */
    local_1cc.m_vecDamageForce.x = local_1cc.m_vecDamageForce.z * vecForward->x;
    local_1cc.m_vecDamageForce.y = local_1cc.m_vecDamageForce.z * vecForward->y;
    local_1cc.m_vecDamageForce.z = local_1cc.m_vecDamageForce.z * vecForward->z;
    local_15c = local_1cc.m_vecDamageForce.x;
    local_158 = local_1cc.m_vecDamageForce.y;
    local_154 = local_1cc.m_vecDamageForce.z;
    CBaseEntity::TakeDamage(pEntity,&local_1cc);
  }
LAB_006df4d9:
  if (local_20 == **(int **)(unaff_EBX + 0x4cc90b)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}


/* CPropExplodingFutbol::DoExplosion at 006dff50 */

/* WARNING: Restarted to delay deadcode elimination for space: stack */
/* DWARF original prototype: void DoExplosion(CPropExplodingFutbol * this, Vector * vecCenter, float
   flExplosionRadius, CPortal_Base2D * pPortal) */

void __thiscall
CPropExplodingFutbol::DoExplosion
          (CPropExplodingFutbol *this,Vector *vecCenter,float flExplosionRadius,
          CPortal_Base2D *pPortal)

{
  int *piVar1;
  undefined4 *puVar2;
  uint uVar3;
  VMatrix matThisToLinked;
  VMatrix matThisToLinked_00;
  bool bVar4;
  bool bVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  CBaseEntity **ppCVar9;
  int iVar10;
  ITraceFilter IVar11;
  CBaseEntity *pCVar12;
  int unaff_EBX;
  VMatrix *pVVar13;
  ShouldHitFunc_t pszClassOrWildcard;
  vec_t *pvVar14;
  bool bVar15;
  CTraceFilterSimpleList *pCVar16;
  CPortal_Base2D *pCVar17;
  CPropExplodingFutbol *pEntity;
  Ray_t *ray;
  CTraceFilterSimpleList *vecCenter_00;
  CTraceFilterSimpleList *pCVar18;
  CTraceFilterSimpleList *this_00;
  trace_t *in_stack_fffffbb4;
  undefined4 in_stack_fffffbb8;
  undefined4 in_stack_fffffbbc;
  undefined1 in_stack_fffffbc0 [36];
  Vector *pVVar19;
  Vector *ptTransformed;
  int local_400;
  bool bDebugging;
  int nNumFound;
  int local_3c4;
  CTraceFilterSimpleList *local_3b8;
  CPortal_Base2D *local_3b4;
  CBaseEntity *local_3a0 [128];
  trace_t local_1a0;
  undefined1 local_14c [48];
  vec_t local_11c;
  vec_t local_118;
  vec_t local_114;
  matrix3x4_t *local_10c;
  bool local_108;
  bool local_107;
  CBaseEntity *local_100;
  CTraceFilterSimpleList local_ec;
  undefined4 local_c8;
  undefined4 local_c4;
  undefined4 local_bc;
  undefined4 local_b8;
  undefined4 local_b4;
  undefined4 local_ac;
  undefined1 local_a8;
  undefined1 local_a7;
  CTraceFilterSimpleList local_98;
  _func_int_varargs **local_74;
  IHandleEntity *local_70;
  float local_6c;
  _func_int_varargs **local_68;
  IHandleEntity *local_64;
  IHandleEntity **local_60;
  Vector local_5c;
  undefined1 local_50 [16];
  IHandleEntity **local_40;
  vec_t local_3c;
  CTraceFilterSimpleList local_38;
  
                    /* Unresolved local var: Color debugColor@[???]
                       Unresolved local var: CBaseEntity *[128] pEnts@[???] */
  ___i686_get_pc_thunk_bx();
  bVar15 = *(int *)(*(int *)(unaff_EBX + 0x64fa9b) + 0x30) != 0;
  local_38.m_PassEntities.m_Memory.m_nGrowSize = 0xffff00;
  if (pPortal != (CPortal_Base2D *)0x0) {
    local_38.m_PassEntities.m_Memory.m_nAllocationCount = 0xff00ff;
    local_38.m_PassEntities.m_Memory.m_nGrowSize = 0xff00ff;
  }
                    /* Unresolved local var: CFlaggedEntitiesEnum sphereEnum@[???] */
  this_00 = (CTraceFilterSimpleList *)0x0;
  CFlaggedEntitiesEnum::CFlaggedEntitiesEnum((CFlaggedEntitiesEnum *)&local_ec,local_3a0,0x80,0);
  vecCenter_00 = &local_ec;
  iVar6 = UTIL_EntitiesInSphere(vecCenter,flExplosionRadius,(CFlaggedEntitiesEnum *)&local_ec);
  if (0 < iVar6) {
    local_3c4 = 0;
    IVar11._vptr_ITraceFilter = (_func_int_varargs **)(*(int *)(&DAT_004cbcdb + unaff_EBX) + 8);
    puVar2 = *(undefined4 **)(&DAT_004cbc37 + unaff_EBX);
    do {
      pCVar12 = local_3a0[local_3c4];
      if (pCVar12 != (CBaseEntity *)0x0) {
        local_400 = 0;
        pszClassOrWildcard = (ShouldHitFunc_t)(unaff_EBX + 0x2dde47);
        while ((local_38.super_CTraceFilterSimple.m_pExtraShouldHitCheckFunction =
                     pszClassOrWildcard,
               (ShouldHitFunc_t)(pCVar12->m_iClassname).pszValue != pszClassOrWildcard &&
               (bVar4 = CBaseEntity::ClassMatchesComplex(pCVar12,(char *)pszClassOrWildcard), !bVar4
               ))) {
          iVar7 = unaff_EBX + 0x55a827 + local_400 * 4;
          do {
            local_400 = local_400 + 1;
            pszClassOrWildcard = *(ShouldHitFunc_t *)(iVar7 + 4);
            if (pszClassOrWildcard == (ShouldHitFunc_t)0x0) goto LAB_006e0100;
            pCVar12 = local_3a0[local_3c4];
            iVar7 = iVar7 + 4;
          } while (pCVar12 == (CBaseEntity *)0x0);
        }
        iVar7 = (this->m_ExplodedEntities).m_Size;
                    /* Unresolved local var: int j@[???] */
        if (iVar7 < 1) {
LAB_006e01de:
          local_38.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter
               = (ITraceFilter)vecCenter->x;
          local_38.super_CTraceFilterSimple.m_pPassEnt = (IHandleEntity *)vecCenter->y;
          local_38.super_CTraceFilterSimple.m_collisionGroup = (int)vecCenter->z;
          pCVar12 = local_3a0[local_3c4];
          pCVar18 = this_00;
          if ((pCVar12->m_iEFlags & 0x800) != 0) {
            CBaseEntity::CalcAbsolutePosition(pCVar12);
            pCVar18 = this_00;
          }
          local_50._12_4_ = (pCVar12->m_vecAbsOrigin).x;
          local_40 = (IHandleEntity **)(pCVar12->m_vecAbsOrigin).y;
          local_3c = (pCVar12->m_vecAbsOrigin).z;
          this_00 = &local_98;
          CTraceFilterSimpleList::CTraceFilterSimpleList(this_00,0);
          pCVar16 = this_00;
          pEntity = this;
          CTraceFilterSimpleList::AddEntityToIgnore(this_00,(IHandleEntity *)this);
                    /* Unresolved local var: int j@[???] */
          if (0 < iVar7) {
            iVar8 = 0;
            do {
              pEntity = (CPropExplodingFutbol *)(this->m_ExplodedEntities).m_Memory.m_pMemory[iVar8]
              ;
              pCVar16 = this_00;
              CTraceFilterSimpleList::AddEntityToIgnore(this_00,(IHandleEntity *)pEntity);
              iVar8 = iVar8 + 1;
            } while (iVar8 != iVar7);
          }
          if (pPortal == (CPortal_Base2D *)0x0) {
            local_ec.m_PassEntities.m_Memory.m_pMemory =
                 (IHandleEntity **)
                 ((float)local_50._12_4_ -
                 (float)local_38.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter.
                        _vptr_ITraceFilter);
            local_ec.m_PassEntities.m_Memory.m_nAllocationCount =
                 (int)((float)local_40 - (float)local_38.super_CTraceFilterSimple.m_pPassEnt);
            local_ec.m_PassEntities.m_Memory.m_nGrowSize =
                 (int)(local_3c - (float)local_38.super_CTraceFilterSimple.m_collisionGroup);
            local_a7 = (float)local_ec.m_PassEntities.m_Memory.m_pMemory *
                       (float)local_ec.m_PassEntities.m_Memory.m_pMemory +
                       (float)local_ec.m_PassEntities.m_Memory.m_nAllocationCount *
                       (float)local_ec.m_PassEntities.m_Memory.m_nAllocationCount +
                       (float)local_ec.m_PassEntities.m_Memory.m_nGrowSize *
                       (float)local_ec.m_PassEntities.m_Memory.m_nGrowSize != 0.0;
            local_b4 = 0;
            local_b8 = 0;
            local_bc = 0;
            local_ac = 0;
            local_a8 = 1;
            local_c4 = 0;
            local_c8 = 0;
            local_ec.m_PassEntities.m_pElements = (IHandleEntity **)0x0;
            local_ec.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter.
            _vptr_ITraceFilter =
                 local_38.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter.
                 _vptr_ITraceFilter;
            local_ec.super_CTraceFilterSimple.m_pPassEnt =
                 local_38.super_CTraceFilterSimple.m_pPassEnt;
            local_ec.super_CTraceFilterSimple.m_collisionGroup =
                 local_38.super_CTraceFilterSimple.m_collisionGroup;
            vecCenter_00 = (CTraceFilterSimpleList *)0x46004003;
            in_stack_fffffbb4 = (trace_t *)local_14c;
            (**(code **)(*(int *)**(undefined4 **)(&DAT_004cbcab + unaff_EBX) + 0x14))();
            if (*(int *)(*(int *)(*(int *)(&DAT_004cbca7 + unaff_EBX) + 0x1c) + 0x30) != 0) {
              in_stack_fffffbbc = 0xbf800000;
              in_stack_fffffbb8 = 1;
              in_stack_fffffbb4 = (trace_t *)0x0;
              this_00 = (CTraceFilterSimpleList *)0x0;
              vecCenter_00 = (CTraceFilterSimpleList *)((int)&section_000000dc.addr + 3);
              DebugDrawLine((Vector *)local_14c,(Vector *)(local_14c + 0xc),0xff,0,0,true,-1.0);
            }
            bVar4 = false;
            if (local_100 != (CBaseEntity *)**(int **)(&DAT_004cbcfb + unaff_EBX)) {
              if (((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                   super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
                CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
              }
              local_6c = (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                         super_CBaseAnimating.super_CBaseEntity.m_rgflCoordinateFrame.m_flMatVal[2]
                         [0];
              local_70 = (IHandleEntity *)
                         (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                         super_CBaseAnimating.super_CBaseEntity.m_rgflCoordinateFrame.m_flMatVal[1]
                         [0];
              local_74 = (_func_int_varargs **)
                         (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                         super_CBaseAnimating.super_CBaseEntity.m_rgflCoordinateFrame.m_flMatVal[0]
                         [0];
              this_00 = (CTraceFilterSimpleList *)local_50;
              vecCenter_00 = (CTraceFilterSimpleList *)vecCenter;
              local_50._0_4_ = local_74;
              local_50._4_4_ = local_70;
              local_50._8_4_ = local_6c;
              DamageEntity(this,local_3a0[local_3c4],vecCenter,(Vector *)this_00);
              bVar4 = true;
            }
LAB_006e079a:
            if (bVar15) {
                    /* Unresolved local var: Color lineColor@[???] */
              local_5c.x = (vec_t)((int)&section_000000dc.addr + 3);
              if (bVar4) {
                local_38.m_PassEntities.m_Memory.m_pMemory = (IHandleEntity **)0xff00;
                local_5c.x = 9.14768e-41;
              }
              NDebugOverlay::Sphere((Vector *)&local_38,2.0,0xff,0xff,0,false,10.0);
              NDebugOverlay::Line((Vector *)&local_38,(Vector *)(local_50 + 0xc),
                                  (uint)local_5c.x & 0xff,(uint)local_5c.x >> 8 & 0xff,
                                  (uint)local_5c.x >> 0x10 & 0xff,false,10.0);
              in_stack_fffffbbc = 0x41200000;
              in_stack_fffffbb8 = 0;
              in_stack_fffffbb4 = (trace_t *)((int)&section_000000dc.addr + 3);
              this_00 = (CTraceFilterSimpleList *)0x0;
              vecCenter_00 = (CTraceFilterSimpleList *)0x0;
              NDebugOverlay::Sphere((Vector *)(local_50 + 0xc),2.0,0,0,0xff,false,10.0);
            }
          }
          else {
            uVar3 = (pPortal->m_hLinkedPortal).
                    super_CNetworkVarBase<CBaseHandle,CPortal_Base2D::NetworkVar_m_hLinkedPortal>.
                    m_Value.m_Index;
            if ((uVar3 == 0xffffffff) ||
               (iVar7 = (uVar3 & 0xffff) * 0x10 + **(int **)(&DAT_004cbc43 + unaff_EBX),
               *(uint *)(iVar7 + 8) != uVar3 >> 0x10)) {
              local_3b4 = (CPortal_Base2D *)0x0;
            }
            else {
              local_3b4 = *(CPortal_Base2D **)(iVar7 + 4);
            }
            local_5c.x = (vec_t)local_50._12_4_;
            local_5c.y = (vec_t)local_40;
            local_5c.z = local_3c;
            pVVar13 = &pPortal->m_matrixThisToLinked;
            pvVar14 = (vec_t *)&stack0xfffffba4;
            pVVar19 = &local_5c;
            ptTransformed = &local_5c;
            for (iVar7 = 0x10; iVar7 != 0; iVar7 = iVar7 + -1) {
              *pvVar14 = pVVar13->m[0][0];
              pVVar13 = (VMatrix *)(pVVar13->m[0] + 1);
              pvVar14 = pvVar14 + 1;
            }
            matThisToLinked.m[0][1] = (vec_t)pEntity;
            matThisToLinked.m[0][0] = (vec_t)pCVar16;
            matThisToLinked.m[0][2] = (vec_t)vecCenter_00;
            matThisToLinked.m[0][3] = (vec_t)pCVar18;
            matThisToLinked.m[1][0] = (vec_t)in_stack_fffffbb4;
            matThisToLinked.m[1][1] = (vec_t)in_stack_fffffbb8;
            matThisToLinked.m[1][2] = (vec_t)in_stack_fffffbbc;
            matThisToLinked.m[1][3] = (vec_t)in_stack_fffffbc0._0_4_;
            matThisToLinked.m[2][0] = (vec_t)in_stack_fffffbc0._4_4_;
            matThisToLinked.m[2][1] = (vec_t)in_stack_fffffbc0._8_4_;
            matThisToLinked.m[2][2] = (vec_t)in_stack_fffffbc0._12_4_;
            matThisToLinked.m[2][3] = (vec_t)in_stack_fffffbc0._16_4_;
            matThisToLinked.m[3][0] = (vec_t)in_stack_fffffbc0._20_4_;
            matThisToLinked.m[3][1] = (vec_t)in_stack_fffffbc0._24_4_;
            matThisToLinked.m[3][2] = (vec_t)in_stack_fffffbc0._28_4_;
            matThisToLinked.m[3][3] = (vec_t)in_stack_fffffbc0._32_4_;
            UTIL_Portal_PointTransform(matThisToLinked,pVVar19,ptTransformed);
            local_10c = (matrix3x4_t *)0x0;
            if (((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                 super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
              CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
            }
            local_14c._16_4_ =
                 local_5c.x -
                 (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                 super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.x;
            local_14c._20_4_ =
                 local_5c.y -
                 (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                 super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.y;
            local_14c._24_4_ =
                 local_5c.z -
                 (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                 super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z;
            local_107 = (float)local_14c._16_4_ * (float)local_14c._16_4_ +
                        (float)local_14c._20_4_ * (float)local_14c._20_4_ +
                        (float)local_14c._24_4_ * (float)local_14c._24_4_ != 0.0;
            local_114 = 0.0;
            local_118 = 0.0;
            local_11c = 0.0;
            local_10c = (matrix3x4_t *)0x0;
            local_108 = true;
            local_14c._40_4_ = 0.0;
            local_14c._36_4_ = 0.0;
            local_14c._32_4_ = 0.0;
            local_14c._0_4_ =
                 (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                 super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.x;
            local_14c._4_4_ =
                 (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                 super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.y;
            local_14c._8_4_ =
                 (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                 super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z;
            in_stack_fffffbb8 = 1;
            in_stack_fffffbb4 = &local_1a0;
            vecCenter_00 = (CTraceFilterSimpleList *)0x46004003;
            ray = (Ray_t *)local_14c;
            pCVar17 = local_3b4;
            bVar5 = UTIL_Portal_TraceRay_Bullets
                              (local_3b4,ray,0x46004003,(ITraceFilter *)this_00,in_stack_fffffbb4,
                               true);
            bVar4 = false;
            if (bVar5) {
                    /* Unresolved local var: Vector vecForward@[???] */
              pCVar18 = vecCenter_00;
              if (((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                   super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
                pCVar17 = (CPortal_Base2D *)this;
                CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
                pCVar18 = vecCenter_00;
              }
              vecCenter_00 = &local_38;
              pVVar19 = &(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                         super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin;
              pVVar13 = &local_3b4->m_matrixThisToLinked;
              pvVar14 = (vec_t *)&stack0xfffffba4;
              pCVar16 = vecCenter_00;
              for (iVar7 = 0x10; iVar7 != 0; iVar7 = iVar7 + -1) {
                *pvVar14 = pVVar13->m[0][0];
                pVVar13 = (VMatrix *)(pVVar13->m[0] + 1);
                pvVar14 = pvVar14 + 1;
              }
              matThisToLinked_00.m[0][1] = (vec_t)ray;
              matThisToLinked_00.m[0][0] = (vec_t)pCVar17;
              matThisToLinked_00.m[0][2] = (vec_t)pCVar18;
              matThisToLinked_00.m[0][3] = (vec_t)this_00;
              matThisToLinked_00.m[1][0] = (vec_t)in_stack_fffffbb4;
              matThisToLinked_00.m[1][1] = (vec_t)in_stack_fffffbb8;
              matThisToLinked_00.m[1][2] = (vec_t)in_stack_fffffbbc;
              matThisToLinked_00.m[1][3] = (vec_t)in_stack_fffffbc0._0_4_;
              matThisToLinked_00.m[2][0] = (vec_t)in_stack_fffffbc0._4_4_;
              matThisToLinked_00.m[2][1] = (vec_t)in_stack_fffffbc0._8_4_;
              matThisToLinked_00.m[2][2] = (vec_t)in_stack_fffffbc0._12_4_;
              matThisToLinked_00.m[2][3] = (vec_t)in_stack_fffffbc0._16_4_;
              matThisToLinked_00.m[3][0] = (vec_t)in_stack_fffffbc0._20_4_;
              matThisToLinked_00.m[3][1] = (vec_t)in_stack_fffffbc0._24_4_;
              matThisToLinked_00.m[3][2] = (vec_t)in_stack_fffffbc0._28_4_;
              matThisToLinked_00.m[3][3] = (vec_t)in_stack_fffffbc0._32_4_;
              UTIL_Portal_PointTransform(matThisToLinked_00,pVVar19,(Vector *)pCVar16);
              pCVar12 = local_3a0[local_3c4];
              if ((pCVar12->m_iEFlags & 0x800) != 0) {
                CBaseEntity::CalcAbsolutePosition(pCVar12);
              }
                    /* Unresolved local var: Vector res@[???] */
              local_ec.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter.
              _vptr_ITraceFilter =
                   (ITraceFilter)
                   ((pCVar12->m_vecAbsOrigin).x -
                   (float)local_38.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter.
                          _vptr_ITraceFilter);
              local_ec.super_CTraceFilterSimple.m_pPassEnt =
                   (IHandleEntity *)
                   ((pCVar12->m_vecAbsOrigin).y -
                   (float)local_38.super_CTraceFilterSimple.m_pPassEnt);
              local_ec.super_CTraceFilterSimple.m_collisionGroup =
                   (int)((pCVar12->m_vecAbsOrigin).z -
                        (float)local_38.super_CTraceFilterSimple.m_collisionGroup);
              local_68 = (_func_int_varargs **)
                         local_ec.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter.
                         _vptr_ITraceFilter;
              local_64 = local_ec.super_CTraceFilterSimple.m_pPassEnt;
              local_60 = (IHandleEntity **)local_ec.super_CTraceFilterSimple.m_collisionGroup;
              VectorNormalize((Vector *)&local_ec);
              this_00 = &local_ec;
              DamageEntity(this,local_3a0[local_3c4],(Vector *)vecCenter_00,(Vector *)&local_ec);
              bVar4 = true;
            }
            if (bVar15) {
              NDebugOverlay::Sphere(&local_5c,5.0,0x80,0xff,0x80,false,10.0);
              if (((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                   super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
                CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
              }
              in_stack_fffffbbc = 0x41200000;
              in_stack_fffffbb8 = 0;
              in_stack_fffffbb4 = (trace_t *)((int)&section_000000dc.addr + 3);
              this_00 = (CTraceFilterSimpleList *)0x0;
              vecCenter_00 = (CTraceFilterSimpleList *)0x0;
              NDebugOverlay::Line(&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                                   super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin,&local_5c,0
                                  ,0,0xff,false,10.0);
              goto LAB_006e079a;
            }
          }
          uVar3 = (this->m_ExplodedEntities).m_Size;
          iVar7 = uVar3 + 1;
          iVar8 = (this->m_ExplodedEntities).m_Memory.m_nAllocationCount;
          if (iVar8 < iVar7) {
            iVar10 = (this->m_ExplodedEntities).m_Memory.m_nGrowSize;
            if (-1 < iVar10) {
              if (iVar10 == 0) {
                if ((iVar8 == 0) && (iVar8 = 8, iVar7 < 9)) {
                  local_3b8 = (CTraceFilterSimpleList *)&segment_command_0000001c.cmdsize;
                }
                else {
                  do {
                    iVar10 = iVar8;
                    iVar8 = iVar10 * 2;
                  } while (iVar8 < iVar7);
                  local_3b8 = (CTraceFilterSimpleList *)(iVar10 * 8);
                }
                goto LAB_006e05bf;
              }
              iVar8 = ((int)uVar3 / iVar10 + 1) * iVar10;
              if (iVar8 < iVar7) {
                if (iVar8 != 0) {
LAB_006e05a2:
                  do {
                    iVar8 = (iVar8 + iVar7) / 2;
                  } while (iVar8 < iVar7);
                  goto LAB_006e05b2;
                }
                if (iVar7 < 0) {
                  iVar8 = -1;
                  local_3b8 = (CTraceFilterSimpleList *)0xfffffffc;
                }
                else {
                  local_3b8 = (CTraceFilterSimpleList *)0x0;
                  if (iVar7 != 0) goto LAB_006e05a2;
                }
              }
              else {
LAB_006e05b2:
                local_3b8 = (CTraceFilterSimpleList *)(iVar8 * 4);
              }
LAB_006e05bf:
              (this->m_ExplodedEntities).m_Memory.m_nAllocationCount = iVar8;
              if ((this->m_ExplodedEntities).m_Memory.m_pMemory == (CBaseEntity **)0x0) {
                ppCVar9 = (CBaseEntity **)(*(code *)**(undefined4 **)*puVar2)();
              }
              else {
                ppCVar9 = (CBaseEntity **)(**(code **)(*(int *)*puVar2 + 4))();
                vecCenter_00 = local_3b8;
              }
              (this->m_ExplodedEntities).m_Memory.m_pMemory = ppCVar9;
            }
          }
          piVar1 = &(this->m_ExplodedEntities).m_Size;
          *piVar1 = *piVar1 + 1;
          ppCVar9 = (this->m_ExplodedEntities).m_Memory.m_pMemory;
          (this->m_ExplodedEntities).m_pElements = ppCVar9;
          iVar8 = ~uVar3 + (this->m_ExplodedEntities).m_Size;
          if (0 < iVar8) {
            vecCenter_00 = (CTraceFilterSimpleList *)(iVar8 * 4);
            _V_memmove(ppCVar9 + iVar7,ppCVar9 + uVar3,(int)vecCenter_00);
          }
          ppCVar9 = (this->m_ExplodedEntities).m_Memory.m_pMemory + uVar3;
          if (ppCVar9 != (CBaseEntity **)0x0) {
            *ppCVar9 = local_3a0[local_3c4];
          }
          local_98.m_PassEntities.m_Size = 0;
          local_98.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter
               = (ITraceFilter)(ITraceFilter)IVar11._vptr_ITraceFilter;
          if (-1 < local_98.m_PassEntities.m_Memory.m_nGrowSize) {
            if (local_98.m_PassEntities.m_Memory.m_pMemory != (IHandleEntity **)0x0) {
              (**(code **)(*(int *)*puVar2 + 8))();
              local_98.m_PassEntities.m_Memory.m_pMemory = (IHandleEntity **)0x0;
            }
            local_98.m_PassEntities.m_Memory.m_nAllocationCount = 0;
          }
          local_98.m_PassEntities.m_pElements = local_98.m_PassEntities.m_Memory.m_pMemory;
          if (-1 < local_98.m_PassEntities.m_Memory.m_nGrowSize) {
            if (local_98.m_PassEntities.m_Memory.m_pMemory != (IHandleEntity **)0x0) {
              (**(code **)(*(int *)**(undefined4 **)(&DAT_004cbc37 + unaff_EBX) + 8))();
              local_98.m_PassEntities.m_Memory.m_pMemory = (IHandleEntity **)0x0;
            }
            local_98.m_PassEntities.m_Memory.m_nAllocationCount = 0;
          }
        }
        else {
          ppCVar9 = (this->m_ExplodedEntities).m_Memory.m_pMemory;
          iVar8 = 0;
          pCVar12 = *ppCVar9;
          while (pCVar12 != local_3a0[local_3c4]) {
            iVar8 = iVar8 + 1;
            if (iVar8 == iVar7) goto LAB_006e01de;
            pCVar12 = ppCVar9[iVar8];
          }
        }
      }
LAB_006e0100:
      local_3c4 = local_3c4 + 1;
    } while (local_3c4 != iVar6);
  }
  if (bVar15) {
    NDebugOverlay::Sphere
              (vecCenter,flExplosionRadius,local_38.m_PassEntities.m_Memory.m_nGrowSize & 0xff,
               (uint)local_38.m_PassEntities.m_Memory.m_nGrowSize >> 8 & 0xff,
               (uint)local_38.m_PassEntities.m_Memory.m_nGrowSize >> 0x10 & 0xff,false,10.0);
    NDebugOverlay::Sphere
              (vecCenter,2.0,local_38.m_PassEntities.m_Memory.m_nGrowSize & 0xff,
               (uint)local_38.m_PassEntities.m_Memory.m_nGrowSize >> 8 & 0xff,
               (uint)local_38.m_PassEntities.m_Memory.m_nGrowSize >> 0x10 & 0xff,false,10.0);
  }
  return;
}


/* CPropExplodingFutbol::ActivateFutbolTimer at 006e15d0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ActivateFutbolTimer(CPropExplodingFutbol * this, float flTimer) */

void __thiscall CPropExplodingFutbol::ActivateFutbolTimer(CPropExplodingFutbol *this,float flTimer)

{
  uint *puVar1;
  uint8 uVar2;
  uint8 uVar3;
  uint8 uVar4;
  int *piVar5;
  int iVar6;
  CBaseEdict *pCVar7;
  BASEPTR func;
  IChangeInfoAccessor *pIVar8;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  this->m_flTotalTimer = flTimer;
  this->m_flExplosionTimer = flTimer;
  if (*(float *)(unaff_EBX + 0x348f92) <= flTimer && flTimer != *(float *)(unaff_EBX + 0x348f92)) {
    this->m_bTimerActive = true;
    piVar5 = *(int **)(&DAT_004ca5c2 + unaff_EBX);
    this->m_flLastTickTime = *(float *)(*piVar5 + 0xc);
    this->m_flLastTimerSoundTime = *(float *)(*piVar5 + 0xc) - *(float *)(unaff_EBX + 0x348d32);
    this->m_flLastFlashTime = *(float *)(*piVar5 + 0xc);
                    /* Unresolved local var: uchar * pColorElement@[???] */
    iVar6 = *(int *)(unaff_EBX + 0x64e65e);
    uVar2 = *(uint8 *)(iVar6 + 0x32);
    uVar3 = *(uint8 *)(iVar6 + 0x31);
    uVar4 = *(uint8 *)(iVar6 + 0x30);
    if (uVar4 != (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                 super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                 super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r) {
      if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                          super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
          false) {
        pCVar7 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                   super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
        if (pCVar7 != (CBaseEdict *)0x0) {
          pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 0x101;
          pIVar8 = CBaseEdict::GetChangeAccessor(pCVar7);
          pIVar8->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = (uint *)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                          super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
        *puVar1 = *puVar1 | 1;
      }
      (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
      super_CBaseEntity.m_clrRender.
      super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r = uVar4;
    }
    if (uVar3 != *(uint8 *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                                  super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                                  super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>
                           + 1)) {
      if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                          super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
          false) {
        pCVar7 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                   super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
        if (pCVar7 != (CBaseEdict *)0x0) {
          pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 0x101;
          pIVar8 = CBaseEdict::GetChangeAccessor(pCVar7);
          pIVar8->m_iChangeInfoSerialNumber = 0;
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
    if (uVar2 != *(uint8 *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                                  super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                                  super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>
                           + 2)) {
      if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                          super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
          false) {
        pCVar7 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                   super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
        if (pCVar7 != (CBaseEdict *)0x0) {
          pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 0x101;
          pIVar8 = CBaseEdict::GetChangeAccessor(pCVar7);
          pIVar8->m_iChangeInfoSerialNumber = 0;
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
           uVar2;
    }
    func.__delta = 0;
    func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x77e);
    CBaseEntity::ThinkSet
              ((CBaseEntity *)this,func,
               *(float *)(**(int **)(&DAT_004ca5c2 + unaff_EBX) + 0xc) +
               *(float *)(**(int **)(&DAT_004ca5c2 + unaff_EBX) + 0x1c),
               *(char **)(unaff_EBX + 0x5ecca6));
  }
  return;
}


/* CPropExplodingFutbol::StopFutbolTimer at 006e1470 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void StopFutbolTimer(CPropExplodingFutbol * this) */

void __thiscall CPropExplodingFutbol::StopFutbolTimer(CPropExplodingFutbol *this)

{
  uint *puVar1;
  uint8 uVar2;
  uint8 uVar3;
  uint8 uVar4;
  int iVar5;
  CBaseEdict *pCVar6;
  IChangeInfoAccessor *pIVar7;
  int unaff_EBX;
  
                    /* Unresolved local var: Color futbolColor@[???] */
  ___i686_get_pc_thunk_bx();
  this->m_bTimerActive = false;
                    /* Unresolved local var: uchar * pColorElement@[???] */
  iVar5 = *(int *)(unaff_EBX + 0x64e7be);
  uVar2 = *(uint8 *)(iVar5 + 0x32);
  uVar3 = *(uint8 *)(iVar5 + 0x31);
  uVar4 = *(uint8 *)(iVar5 + 0x30);
  if (uVar4 != (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
               super_CBaseEntity.m_clrRender.
               super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r) {
    if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
        false) {
      pCVar6 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                 super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar6 != (CBaseEdict *)0x0) {
        pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 0x101;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar6);
        pIVar7->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = (uint *)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
      *puVar1 = *puVar1 | 1;
    }
    (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
    super_CBaseEntity.m_clrRender.
    super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r = uVar4;
  }
  if (uVar3 != *(uint8 *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                                super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                                super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>
                         + 1)) {
    if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
        false) {
      pCVar6 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                 super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar6 != (CBaseEdict *)0x0) {
        pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 0x101;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar6);
        pIVar7->m_iChangeInfoSerialNumber = 0;
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
  if (uVar2 != *(uint8 *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                                super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                                super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>
                         + 2)) {
    if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
        false) {
      pCVar6 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                 super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar6 != (CBaseEdict *)0x0) {
        pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 0x101;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar6);
        pIVar7->m_iChangeInfoSerialNumber = 0;
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
         uVar2;
  }
  return;
}


/* CPropExplodingFutbol::TimerThink at 006e0e60 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void TimerThink(CPropExplodingFutbol * this) */

void __thiscall CPropExplodingFutbol::TimerThink(CPropExplodingFutbol *this)

{
  float *pfVar1;
  uint *puVar2;
  uint8 uVar3;
  uint8 uVar4;
  CBaseEdict *pCVar5;
  uint3 uVar6;
  uint3 uVar7;
  uint8 uVar8;
  char *soundname;
  IChangeInfoAccessor *pIVar9;
  uint uVar10;
  int unaff_EBX;
  int iVar11;
  float fVar12;
  float fVar13;
  uint8 local_8c;
  char *local_80;
  uint8 uStack_26;
  uint local_24;
  uint local_20;
  
                    /* Unresolved local var: float flDeltaTime@[???]
                       Unresolved local var: float flSoundDelay@[???]
                       Unresolved local var: string_t strSoundName@[???]
                       Unresolved local var: Color startColor@[???]
                       Unresolved local var: Color endColor@[???]
                       Unresolved local var: Color currentColor@[???]
                       Unresolved local var: float flPercent@[???] */
  ___i686_get_pc_thunk_bx();
  if (this->m_bTimerActive != false) {
    fVar12 = *(float *)(**(int **)(&DAT_004cad2f + unaff_EBX) + 0xc);
    fVar13 = this->m_flLastTickTime;
    this->m_flLastTickTime = fVar12;
    fVar12 = this->m_flExplosionTimer - (fVar12 - fVar13);
    this->m_flExplosionTimer = fVar12;
    local_80 = (char *)(unaff_EBX + 0x330a6f);
    pfVar1 = (float *)(*(int *)(unaff_EBX + 0x64ef4b) + 0x2c);
    if (fVar12 < *pfVar1 || fVar12 == *pfVar1) {
      local_80 = (char *)(unaff_EBX + 0x330a4f);
                    /* Unresolved local var: float flCurrentFlashDuration@[???] */
      iVar11 = *(int *)(unaff_EBX + 0x64efab);
      fVar12 = *(float *)(iVar11 + 0x2c);
      fVar13 = *(float *)(**(int **)(&DAT_004cad2f + unaff_EBX) + 0xc) - this->m_flLastFlashTime;
      if (fVar12 < fVar13) {
        this->m_flLastFlashTime = *(float *)(**(int **)(&DAT_004cad2f + unaff_EBX) + 0xc);
        fVar13 = 0.0;
        iVar11 = *(int *)(unaff_EBX + 0x64efab);
      }
                    /* Unresolved local var: uchar * pColorElement@[???] */
      uVar6 = *(uint3 *)(*(int *)(unaff_EBX + 0x64ee8b) + 0x30);
                    /* Unresolved local var: uchar * pColorElement@[???] */
      uVar7 = *(uint3 *)(*(int *)(unaff_EBX + 0x64eeeb) + 0x30);
      fVar13 = fVar13 / *(float *)(iVar11 + 0x2c);
    }
    else {
                    /* Unresolved local var: uchar * pColorElement@[???] */
      uVar6 = *(uint3 *)(*(int *)(unaff_EBX + 0x64edcb) + 0x30);
                    /* Unresolved local var: uchar * pColorElement@[???] */
      uVar7 = *(uint3 *)(*(int *)(unaff_EBX + 0x64ee2b) + 0x30);
      fVar13 = (this->m_flTotalTimer - this->m_flExplosionTimer) / this->m_flTotalTimer;
      fVar12 = *(float *)(&DAT_0034949f + unaff_EBX);
    }
    local_20 = (uint)uVar6;
    local_24 = (uint)uVar7;
    if (fVar12 < *(float *)(**(int **)(&DAT_004cad2f + unaff_EBX) + 0xc) -
                 this->m_flLastTimerSoundTime) {
      soundname = (char *)(unaff_EBX + 0x2caceb);
      if (local_80 != (char *)0x0) {
        soundname = local_80;
      }
      CBaseEntity::EmitSound((CBaseEntity *)this,soundname,0.0,(float *)0x0);
      this->m_flLastTimerSoundTime = *(float *)(**(int **)(&DAT_004cad2f + unaff_EBX) + 0xc);
    }
                    /* Unresolved local var: float r@[???]
                       Unresolved local var: float g@[???]
                       Unresolved local var: float b@[???]
                       Unresolved local var: float a@[???] */
    uVar10 = uVar6 >> 8 & 0xff;
    uVar8 = (uint8)(int)(float)(int)((float)(local_20 & 0xff) +
                                    (float)(int)((local_24 & 0xff) - (local_20 & 0xff)) * fVar13);
    local_8c = (uint8)(int)(float)(int)((float)uVar10 +
                                       (float)(int)((uVar7 >> 8 & 0xff) - uVar10) * fVar13);
    uStack_26 = (uint8)(int)(float)(int)((float)(uVar6 >> 0x10) +
                                        (float)(int)((uint)(uVar7 >> 0x10) - (uint)(uVar6 >> 0x10))
                                        * fVar13);
    if (uVar8 != (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                 super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                 super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r) {
      if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                          super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
          false) {
        pCVar5 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                   super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
        if (pCVar5 != (CBaseEdict *)0x0) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
          pIVar9 = CBaseEdict::GetChangeAccessor(pCVar5);
          pIVar9->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar2 = (uint *)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                          super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
        *puVar2 = *puVar2 | 1;
      }
      (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
      super_CBaseEntity.m_clrRender.
      super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r = uVar8;
    }
    if (local_8c !=
        *(uint8 *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                         super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                         super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender> + 1))
    {
      if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                          super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
          false) {
        pCVar5 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                   super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
        if (pCVar5 != (CBaseEdict *)0x0) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
          pIVar9 = CBaseEdict::GetChangeAccessor(pCVar5);
          pIVar9->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar2 = (uint *)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                          super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
        *puVar2 = *puVar2 | 1;
      }
      *(uint8 *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                       super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                       super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender> + 1) =
           local_8c;
    }
    if (uStack_26 !=
        *(uint8 *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                         super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                         super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender> + 2))
    {
      if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                          super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
          false) {
        pCVar5 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                   super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
        if (pCVar5 != (CBaseEdict *)0x0) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
          pIVar9 = CBaseEdict::GetChangeAccessor(pCVar5);
          pIVar9->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar2 = (uint *)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                          super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
        *puVar2 = *puVar2 | 1;
      }
      *(uint8 *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                       super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                       super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender> + 2) =
           uStack_26;
    }
    if (this->m_flExplosionTimer <= 0.0) {
      CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x330a85),0.0,(float *)0x0);
      KillFutbol(this);
                    /* Unresolved local var: Color futbolColor@[???] */
      this->m_bTimerActive = false;
                    /* Unresolved local var: uchar * pColorElement@[???] */
      iVar11 = *(int *)(unaff_EBX + 0x64edcb);
      uVar8 = *(uint8 *)(iVar11 + 0x32);
      uVar3 = *(uint8 *)(iVar11 + 0x31);
      uVar4 = *(uint8 *)(iVar11 + 0x30);
      if (uVar4 != (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                   super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                   super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r) {
        if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                            super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
            false) {
          pCVar5 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                     super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
          if (pCVar5 != (CBaseEdict *)0x0) {
            pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
            pIVar9 = CBaseEdict::GetChangeAccessor(pCVar5);
            pIVar9->m_iChangeInfoSerialNumber = 0;
          }
        }
        else {
          puVar2 = (uint *)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                            super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
          *puVar2 = *puVar2 | 1;
        }
        (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
        super_CBaseEntity.m_clrRender.
        super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r = uVar4;
      }
      if (uVar3 != *(uint8 *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                                    super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                                    super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>
                             + 1)) {
        if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                            super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
            false) {
          pCVar5 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                     super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
          if (pCVar5 != (CBaseEdict *)0x0) {
            pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
            pIVar9 = CBaseEdict::GetChangeAccessor(pCVar5);
            pIVar9->m_iChangeInfoSerialNumber = 0;
          }
        }
        else {
          puVar2 = (uint *)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                            super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
          *puVar2 = *puVar2 | 1;
        }
        *(uint8 *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                         super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                         super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender> + 1) =
             uVar3;
      }
      if (uVar8 != *(uint8 *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                                    super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                                    super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>
                             + 2)) {
        if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                            super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
            false) {
          pCVar5 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                     super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
          if (pCVar5 != (CBaseEdict *)0x0) {
            pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
            pIVar9 = CBaseEdict::GetChangeAccessor(pCVar5);
            pIVar9->m_iChangeInfoSerialNumber = 0;
          }
        }
        else {
          puVar2 = (uint *)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                            super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
          *puVar2 = *puVar2 | 1;
        }
        *(uint8 *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                         super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                         super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender> + 2) =
             uVar8;
      }
    }
    if (this->m_bTimerActive != false) {
      CBaseEntity::SetNextThink
                ((CBaseEntity *)this,
                 *(float *)(**(int **)(&DAT_004cad2f + unaff_EBX) + 0xc) +
                 *(float *)(**(int **)(&DAT_004cad2f + unaff_EBX) + 0x1c),
                 *(char **)(unaff_EBX + 0x5ed413));
    }
  }
  return;
}


/* CPropExplodingFutbol::OnPhysGunPickup at 006dfb40 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void OnPhysGunPickup(CPropExplodingFutbol * this, CBasePlayer *
   pPhysGunUser, PhysGunPickup_t reason) */

void __thiscall
CPropExplodingFutbol::OnPhysGunPickup
          (CPropExplodingFutbol *this,CBasePlayer *pPhysGunUser,PhysGunPickup_t reason)

{
  IPhysicsObject *pIVar1;
  uint uVar2;
  ExplodingFutbolHolderType_t EVar3;
  int *piVar4;
  char cVar5;
  int iVar6;
  ulong *puVar7;
  CBasePlayer *pCVar8;
  int unaff_EBX;
  bool local_21;
  CPortal_Player *local_20;
  
                    /* Unresolved local var: CPortal_Player * pHoldingPlayer@[???]
                       Unresolved local var: IPhysicsObject * pPhysicsObject@[???]
                       Unresolved local var: bool bSamePlayer@[???] */
  ___i686_get_pc_thunk_bx();
  if ((pPhysGunUser == (CBasePlayer *)0x0) ||
     (cVar5 = (**(code **)((int)(pPhysGunUser->super_CBaseCombatCharacter).super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                                super_IServerEntity.super_IServerUnknown + 0x154))(pPhysGunUser),
     cVar5 == '\0')) {
    local_20 = (CPortal_Player *)0x0;
  }
  else {
    local_20 = (CPortal_Player *)
               ___dynamic_cast(pPhysGunUser,*(undefined4 *)(unaff_EBX + 0x4cc07a),
                               *(undefined4 *)(unaff_EBX + 0x4cc27a),0);
  }
  pIVar1 = (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
           super_CBaseEntity.m_pPhysicsObject;
  if (pIVar1 != (IPhysicsObject *)0x0) {
    (*pIVar1->_vptr_IPhysicsObject[0x10])(pIVar1,1);
  }
  if ((reason == PICKED_UP_BY_PLAYER) && (local_20 != (CPortal_Player *)0x0)) {
    CPortal_Player::SetUseKeyCooldownTime
              (local_20,*(float *)(*(int *)(unaff_EBX + 0x64ff0e) + 0x2c));
  }
  uVar2 = (this->m_hLastHeldByPlayer).super_CBaseHandle.m_Index;
  if ((uVar2 == 0xffffffff) ||
     (iVar6 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4cc056),
     *(uint *)(iVar6 + 8) != uVar2 >> 0x10)) {
    local_21 = false;
    if (local_20 != (CPortal_Player *)0x0) goto LAB_006dfbcc;
  }
  else {
    local_21 = false;
    if (local_20 != *(CPortal_Player **)(iVar6 + 4)) goto LAB_006dfbcc;
  }
  local_21 = this->m_Holder == EXPLODING_FUTBOL_HELD_BY_NONE;
LAB_006dfbcc:
  uVar2 = (this->m_hSpawner).super_CBaseHandle.m_Index;
  if (((uVar2 != 0xffffffff) &&
      (iVar6 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4cc056),
      *(uint *)(iVar6 + 8) == uVar2 >> 0x10)) &&
     (piVar4 = *(int **)(iVar6 + 4), piVar4 != (int *)0x0)) {
    (**(code **)(*piVar4 + 0x3b4))(piVar4,this,local_21);
  }
  if (local_20 == (CPortal_Player *)0x0) {
    (this->m_hLastHeldByPlayer).super_CBaseHandle.m_Index = 0xffffffff;
    EVar3 = this->m_Holder;
  }
  else {
    puVar7 = (ulong *)(**(code **)(*(int *)&(local_20->
                                            super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>
                                            ).super_CPaintableEntity<CBaseMultiplayerPlayer>.
                                            super_CBaseMultiplayerPlayer.
                                            super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.
                                            super_CBaseCombatCharacter.super_CBaseFlex + 0xc))
                                (local_20);
    (this->m_hLastHeldByPlayer).super_CBaseHandle.m_Index = *puVar7;
    EVar3 = this->m_Holder;
  }
                    /* Unresolved local var: CBasePlayer * pOtherPlayer@[???] */
  if ((EVar3 == EXPLODING_FUTBOL_HELD_BY_PLAYER) &&
     (pCVar8 = GetPlayerHoldingEntity((CBaseEntity *)this), pCVar8 != (CBasePlayer *)0x0)) {
    (**(code **)((int)(pCVar8->super_CBaseCombatCharacter).super_CBaseFlex.
                      super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                      super_IServerEntity.super_IServerUnknown + 0x6e0))(pCVar8,0);
  }
  this->m_Holder = EXPLODING_FUTBOL_HELD_BY_PLAYER;
  return;
}


/* CPropExplodingFutbol::OnPhysGunDrop at 006debb0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void OnPhysGunDrop(CPropExplodingFutbol * this, CBasePlayer *
   pPhysGunUser, PhysGunDrop_t reason) */

void __thiscall
CPropExplodingFutbol::OnPhysGunDrop
          (CPropExplodingFutbol *this,CBasePlayer *pPhysGunUser,PhysGunDrop_t reason)

{
  if (pPhysGunUser != (CBasePlayer *)0x0) {
    CPhysicsProp::OnPhysGunDrop(&this->super_CPhysicsProp,pPhysGunUser,reason);
    if (this->m_Holder == EXPLODING_FUTBOL_HELD_BY_PLAYER) {
      this->m_Holder = EXPLODING_FUTBOL_HELD_BY_NONE;
      return;
    }
  }
  return;
}


/* CPropExplodingFutbol::OnTakeDamage at 006deb60 */

/* DWARF original prototype: int OnTakeDamage(CPropExplodingFutbol * this, CTakeDamageInfo * info)
    */

int __thiscall CPropExplodingFutbol::OnTakeDamage(CPropExplodingFutbol *this,CTakeDamageInfo *info)

{
  int iVar1;
  
  if (((this->m_bExplodeOnTouch != false) && (this->m_Holder != EXPLODING_FUTBOL_HELD_BY_SPAWNER))
     && (this->m_Holder != EXPLODING_FUTBOL_HELD_BY_CATCHER)) {
    iVar1 = CPhysicsProp::OnTakeDamage(&this->super_CPhysicsProp,info);
    return iVar1;
  }
  return 0;
}


/* CPropExplodingFutbol::VPhysicsCollision at 006dead0 */

/* DWARF original prototype: void VPhysicsCollision(CPropExplodingFutbol * this, int index,
   gamevcollisionevent_t * pEvent) */

void __thiscall
CPropExplodingFutbol::VPhysicsCollision
          (CPropExplodingFutbol *this,int index,gamevcollisionevent_t *pEvent)

{
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((this->m_bExplodeOnTouch != false) && (this->m_Holder == EXPLODING_FUTBOL_HELD_BY_NONE)) {
    func.__delta = 0;
    func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0xe0c);
    CBaseEntity::ThinkSet
              ((CBaseEntity *)this,func,
               *(float *)(**(int **)(unaff_EBX + 0x4cd0bc) + 0xc) +
               *(float *)(**(int **)(unaff_EBX + 0x4cd0bc) + 0x1c),*(char **)(unaff_EBX + 0x5ef7a4))
    ;
  }
  CPhysicsProp::VPhysicsCollision(&this->super_CPhysicsProp,index,pEvent);
  return;
}


/* CPropExplodingFutbol::KillThink at 006df8f0 */

/* DWARF original prototype: void KillThink(CPropExplodingFutbol * this) */

void __thiscall CPropExplodingFutbol::KillThink(CPropExplodingFutbol *this)

{
  KillFutbol(this);
  return;
}


/* CPropExplodingFutbol::AnimThink at 006de860 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void AnimThink(CPropExplodingFutbol * this) */

void __thiscall CPropExplodingFutbol::AnimThink(CPropExplodingFutbol *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x31c))(this);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(**(int **)(unaff_EBX + 0x4cd32f) + 0xc) +
             *(float *)(**(int **)(unaff_EBX + 0x4cd32f) + 0x1c),
             *(char **)(&DAT_005efa0f + unaff_EBX));
  return;
}


/* ColorLerp at 006de5f0 */

Color * ColorLerp(Color *__return_storage_ptr__,float flPercent,Color *startColor,Color *endColor)

{
                    /* Unresolved local var: float r@[???]
                       Unresolved local var: float g@[???]
                       Unresolved local var: float b@[???]
                       Unresolved local var: float a@[???] */
  return (Color *)CONCAT31(CONCAT21((ushort)((uint)((int)(float)(int)((float)*(byte *)((int)
                                                  flPercent + 3) +
                                                  (float)__return_storage_ptr__ *
                                                  (float)(int)((uint)startColor->_color[3] -
                                                              (uint)*(byte *)((int)flPercent + 3)))
                                                  << 0x18) >> 0x10) |
                                    (ushort)(int)(float)(int)((float)*(byte *)((int)flPercent + 2) +
                                                             (float)(int)((uint)startColor->_color
                                                                                [2] -
                                                                         (uint)*(byte *)((int)
                                                  flPercent + 2)) * (float)__return_storage_ptr__) &
                                    0xff,(char)(int)(float)(int)((float)*(byte *)((int)flPercent + 1
                                                                                 ) +
                                                                (float)(int)((uint)startColor->
                                                                                   _color[1] -
                                                                            (uint)*(byte *)((int)
                                                  flPercent + 1)) * (float)__return_storage_ptr__)),
                           (char)(int)(float)(int)((float)*(byte *)flPercent +
                                                  (float)(int)((uint)startColor->_color[0] -
                                                              (uint)*(byte *)flPercent) *
                                                  (float)__return_storage_ptr__));
}


/* CExplodingFutbolCatcher::CExplodingFutbolCatcher at 006df910 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CExplodingFutbolCatcher(CExplodingFutbolCatcher * this) */

void __thiscall CExplodingFutbolCatcher::CExplodingFutbolCatcher(CExplodingFutbolCatcher *this)

{
  vec_t *pvVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::CBaseAnimating(&this->super_CBaseAnimating);
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x55aa8b);
  (this->m_hCaughtFutbol).super_CBaseHandle.m_Index = 0xffffffff;
  this->m_bDisableRecaptureOnPlayerGrab = false;
  pvVar1 = *(vec_t **)(unaff_EBX + 0x4cc293);
  (this->m_vecCatcherBoxMins).x = *pvVar1;
  (this->m_vecCatcherBoxMins).y = pvVar1[1];
  (this->m_vecCatcherBoxMins).z = pvVar1[2];
  (this->m_vecCatcherBoxMaxs).x = *pvVar1;
  (this->m_vecCatcherBoxMaxs).y = pvVar1[1];
  (this->m_vecCatcherBoxMaxs).z = pvVar1[2];
  (this->m_vecCatcherBoxOrigin).x = *pvVar1;
  (this->m_vecCatcherBoxOrigin).y = pvVar1[1];
  (this->m_vecCatcherBoxOrigin).z = pvVar1[2];
  (this->m_vecCatcherBoxHalfDiagonal).x = 25.0;
  (this->m_vecCatcherBoxHalfDiagonal).y = 25.0;
  (this->m_vecCatcherBoxHalfDiagonal).z = 25.0;
  (this->m_OnFutbolReleased).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnFutbolReleased).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->m_OnFutbolReleased).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_OnFutbolCaught).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnFutbolCaught).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->m_OnFutbolCaught).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  return;
}


/* CExplodingFutbolCatcher::CExplodingFutbolCatcher at 006dfb20 */

/* DWARF original prototype: void CExplodingFutbolCatcher(CExplodingFutbolCatcher * this,
   CExplodingFutbolCatcher * this) */

void __thiscall
CExplodingFutbolCatcher::CExplodingFutbolCatcher
          (CExplodingFutbolCatcher *this,CExplodingFutbolCatcher *this_1)

{
  CExplodingFutbolCatcher(this);
  return;
}


/* CEntityFactory<CExplodingFutbolCatcher>::Create at 006e2110 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CExplodingFutbolCatcher> *
   this, char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CExplodingFutbolCatcher>::Create
          (CEntityFactory<CExplodingFutbolCatcher> *this,char *pClassName)

{
  CExplodingFutbolCatcher *this_00;
  
                    /* Unresolved local var: CExplodingFutbolCatcher * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x57c);
  CExplodingFutbolCatcher::CExplodingFutbolCatcher(this_00);
  (*(this_00->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1d])(this_00,pClassName);
  return &(this_00->super_CBaseAnimating).super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CExplodingFutbolCatcher::Spawn at 006de930 */

/* DWARF original prototype: void Spawn(CExplodingFutbolCatcher * this) */

void __thiscall CExplodingFutbolCatcher::Spawn(CExplodingFutbolCatcher *this)

{
  uint uVar1;
  vec_t vVar2;
  vec_t vVar3;
  vec_t vVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::Spawn(&this->super_CBaseAnimating);
  CBaseAnimating::GetAttachment
            (&this->super_CBaseAnimating,(char *)(unaff_EBX + 0x332a43),&this->m_vecCatcherBoxOrigin
             ,(Vector *)0x0,(Vector *)0x0,(Vector *)0x0);
  uVar1 = *(uint *)(unaff_EBX + 0x354e43);
  vVar2 = (this->m_vecCatcherBoxHalfDiagonal).z;
  vVar3 = (this->m_vecCatcherBoxHalfDiagonal).y;
  vVar4 = (this->m_vecCatcherBoxHalfDiagonal).x;
  (this->m_vecCatcherBoxMins).x = (vec_t)(uVar1 ^ (uint)vVar4);
  (this->m_vecCatcherBoxMins).y = (vec_t)((uint)vVar3 ^ uVar1);
  (this->m_vecCatcherBoxMins).z = (vec_t)((uint)vVar2 ^ uVar1);
  (this->m_vecCatcherBoxMaxs).x = vVar4;
  (this->m_vecCatcherBoxMaxs).y = vVar3;
  (this->m_vecCatcherBoxMaxs).z = vVar2;
  CBaseEntity::ThinkSet((CBaseEntity *)this,(BASEPTR)0x3a5,0.0,(char *)0x0);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(**(int **)(unaff_EBX + 0x4cd263) + 0xc) +
             *(float *)(**(int **)(unaff_EBX + 0x4cd263) + 0x1c),(char *)0x0);
  return;
}


/* CExplodingFutbolCatcher::DrawDebugGeometryOverlays at 006de8c0 */

/* DWARF original prototype: void DrawDebugGeometryOverlays(CExplodingFutbolCatcher * this) */

void __thiscall CExplodingFutbolCatcher::DrawDebugGeometryOverlays(CExplodingFutbolCatcher *this)

{
  CBaseEntity::DrawDebugGeometryOverlays((CBaseEntity *)this);
  NDebugOverlay::Box(&this->m_vecCatcherBoxOrigin,&this->m_vecCatcherBoxMins,
                     &this->m_vecCatcherBoxMaxs,0xff,0,0,100,0.0);
  return;
}


/* CExplodingFutbolCatcher::CaptureThink at 006dfe70 */

/* DWARF original prototype: void CaptureThink(CExplodingFutbolCatcher * this) */

void __thiscall CExplodingFutbolCatcher::CaptureThink(CExplodingFutbolCatcher *this)

{
  uint uVar1;
  int iVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: CPropExplodingFutbol * pFutbol@[???] */
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->m_hCaughtFutbol).super_CBaseHandle.m_Index;
  if ((((uVar1 == 0xffffffff) ||
       (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(&DAT_004cbd20 + unaff_EBX),
       *(uint *)(iVar2 + 8) != uVar1 >> 0x10)) || (iVar2 = *(int *)(iVar2 + 4), iVar2 == 0)) ||
     (*(int *)(iVar2 + 0x768) != 3)) {
    CBaseEntity::ThinkSet((CBaseEntity *)this,(BASEPTR)0x3a5,0.0,(char *)0x0);
    COutputEvent::FireOutput(&this->m_OnFutbolReleased,(CBaseEntity *)this,(CBaseEntity *)this,0.0);
    (this->m_hCaughtFutbol).super_CBaseHandle.m_Index = 0xffffffff;
    this->m_bDisableRecaptureOnPlayerGrab = true;
  }
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(**(int **)(&DAT_004cbd1c + unaff_EBX) + 0xc) +
             *(float *)(**(int **)(&DAT_004cbd1c + unaff_EBX) + 0x1c),(char *)0x0);
  return;
}


/* CPropExplodingFutbolSpawner::CPropExplodingFutbolSpawner at 006dfa70 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CPropExplodingFutbolSpawner(CPropExplodingFutbolSpawner * this) */

void __thiscall
CPropExplodingFutbolSpawner::CPropExplodingFutbolSpawner(CPropExplodingFutbolSpawner *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CExplodingFutbolCatcher::CExplodingFutbolCatcher(&this->super_CExplodingFutbolCatcher);
  (this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x55a547);
  (this->m_OnFutbolSpawned).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnFutbolSpawned).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->m_OnFutbolSpawned).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_OnFutbolGrabbed).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnFutbolGrabbed).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->m_OnFutbolGrabbed).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  this->m_bHasFutbol = false;
  this->m_bIsTimed = false;
  this->m_flTimer = 0.0;
  (this->m_strTimerIndicator).pszValue = (char *)0x0;
  (this->super_CExplodingFutbolCatcher).m_vecCatcherBoxHalfDiagonal.x = 20.0;
  (this->super_CExplodingFutbolCatcher).m_vecCatcherBoxHalfDiagonal.y = 20.0;
  (this->super_CExplodingFutbolCatcher).m_vecCatcherBoxHalfDiagonal.z = 20.0;
  return;
}


/* CPropExplodingFutbolSpawner::CPropExplodingFutbolSpawner at 006dfb10 */

/* DWARF original prototype: void CPropExplodingFutbolSpawner(CPropExplodingFutbolSpawner * this,
   CPropExplodingFutbolSpawner * this) */

void __thiscall
CPropExplodingFutbolSpawner::CPropExplodingFutbolSpawner
          (CPropExplodingFutbolSpawner *this,CPropExplodingFutbolSpawner *this_1)

{
  CPropExplodingFutbolSpawner(this);
  return;
}


/* CEntityFactory<CPropExplodingFutbolSpawner>::Create at 006e2160 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CPropExplodingFutbolSpawner>
   * this, char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CPropExplodingFutbolSpawner>::Create
          (CEntityFactory<CPropExplodingFutbolSpawner> *this,char *pClassName)

{
  CPropExplodingFutbolSpawner *this_00;
  
                    /* Unresolved local var: CPropExplodingFutbolSpawner * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x5b8);
  CPropExplodingFutbolSpawner::CPropExplodingFutbolSpawner(this_00);
  (*(this_00->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.
    super_IServerEntity.super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1d])
            (this_00,pClassName);
  return &(this_00->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.m_Network.
          super_IServerNetworkable;
}


/* CPropExplodingFutbolSpawner::Precache at 006de730 */

/* DWARF original prototype: void Precache(CPropExplodingFutbolSpawner * this) */

void __thiscall CPropExplodingFutbolSpawner::Precache(CPropExplodingFutbolSpawner *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::Precache((CBaseAnimating *)this);
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x332bf8));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x33310e));
  return;
}


/* CPropExplodingFutbolSpawner::Spawn at 006dec60 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CPropExplodingFutbolSpawner * this) */

void __thiscall CPropExplodingFutbolSpawner::Spawn(CPropExplodingFutbolSpawner *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity
    .super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1a])(this);
  (*(this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity
    .super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1b])(this,unaff_EBX + 0x3326c3);
  CCollisionProperty::SetSolid
            (&(this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.
              m_Collision.super_CCollisionProperty,SOLID_VPHYSICS);
  CExplodingFutbolCatcher::Spawn(&this->super_CExplodingFutbolCatcher);
  if (this->m_bHasFutbol == false) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x006decd4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity
    .super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xee])();
  return;
}


/* CPropExplodingFutbolSpawner::FutbolDestroyed at 006de830 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void FutbolDestroyed(CPropExplodingFutbolSpawner * this) */

void __thiscall CPropExplodingFutbolSpawner::FutbolDestroyed(CPropExplodingFutbolSpawner *this)

{
  CPropIndicatorPanel *this_00;
  
                    /* Unresolved local var: CPropIndicatorPanel * pTimerPanel@[???] */
  this_00 = GetTimerPanel(this);
  if (this_00 != (CPropIndicatorPanel *)0x0) {
    CPropIndicatorPanel::ResetTimer(this_00);
  }
                    /* WARNING: Could not recover jumptable at 0x006de85e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity
    .super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xee])();
  return;
}


/* CPropExplodingFutbolSpawner::FutbolGrabbed at 006e17c0 */

/* DWARF original prototype: void FutbolGrabbed(CPropExplodingFutbolSpawner * this,
   CPropExplodingFutbol * pFutbol, bool bSamePlayer) */

void __thiscall
CPropExplodingFutbolSpawner::FutbolGrabbed
          (CPropExplodingFutbolSpawner *this,CPropExplodingFutbol *pFutbol,bool bSamePlayer)

{
  CPropIndicatorPanel *this_00;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x330076),0.0,(float *)0x0);
  if (!bSamePlayer) {
                    /* Unresolved local var: CPropIndicatorPanel * pTimerPanel@[???] */
    this_00 = GetTimerPanel(this);
    if (this_00 != (CPropIndicatorPanel *)0x0) {
      CPropIndicatorPanel::SetTimerDuration(this_00,this->m_flTimer);
      CPropIndicatorPanel::StartTimer(this_00);
    }
    if (this->m_bIsTimed != false) {
      CPropExplodingFutbol::ActivateFutbolTimer(pFutbol,this->m_flTimer);
    }
    if (this->m_bHasFutbol != false) {
      COutputEvent::FireOutput(&this->m_OnFutbolGrabbed,(CBaseEntity *)this,(CBaseEntity *)this,0.0)
      ;
    }
  }
  this->m_bHasFutbol = false;
  return;
}


/* CPropExplodingFutbolSpawner::GetTimerPanel at 006de770 */

/* DWARF original prototype: CPropIndicatorPanel * GetTimerPanel(CPropExplodingFutbolSpawner * this)
    */

CPropIndicatorPanel * __thiscall
CPropExplodingFutbolSpawner::GetTimerPanel(CPropExplodingFutbolSpawner *this)

{
  CBaseEntity *pCVar1;
  CPropIndicatorPanel *pCVar2;
  char *pcVar3;
  int iVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (((this->m_bIsTimed != false) &&
      (pcVar3 = (this->m_strTimerIndicator).pszValue, pcVar3 != (char *)0x0)) &&
     (pCVar1 = CGlobalEntityList::FindEntityByName
                         (*(CGlobalEntityList **)(unaff_EBX + 0x4cd4e4),(CBaseEntity *)0x0,pcVar3,
                          (CBaseEntity *)0x0,(CBaseEntity *)0x0,(CBaseEntity *)0x0,
                          (IEntityFindFilter *)0x0), pCVar1 != (CBaseEntity *)0x0)) {
    pCVar2 = (CPropIndicatorPanel *)
             ___dynamic_cast(pCVar1,*(undefined4 *)(unaff_EBX + 0x4cd44c),
                             *(undefined4 *)(unaff_EBX + 0x4cdf9c),0);
    if (pCVar2 != (CPropIndicatorPanel *)0x0) {
      return pCVar2;
    }
    pcVar3 = CBaseEntity::GetDebugName((CBaseEntity *)0x0);
    iVar4 = section_00000054.segname._4_4_;
    if (section_00000054.segname._4_4_ == 0) {
      iVar4 = unaff_EBX + 0x2cd3e0;
    }
    _Warning(unaff_EBX + 0x3330e4,iVar4,pcVar3);
    return (CPropIndicatorPanel *)0x0;
  }
  return (CPropIndicatorPanel *)0x0;
}


/* CPropExplodingFutbolSpawner::InputForceSpawn at 006de6d0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void InputForceSpawn(CPropExplodingFutbolSpawner * this, inputdata_t *
   data) */

void __thiscall
CPropExplodingFutbolSpawner::InputForceSpawn(CPropExplodingFutbolSpawner *this,inputdata_t *data)

{
                    /* WARNING: Could not recover jumptable at 0x006de6df. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity
    .super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xee])();
  return;
}


/* CPropExplodingFutbolSocket::CPropExplodingFutbolSocket at 006df9f0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CPropExplodingFutbolSocket(CPropExplodingFutbolSocket * this) */

void __thiscall
CPropExplodingFutbolSocket::CPropExplodingFutbolSocket(CPropExplodingFutbolSocket *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CExplodingFutbolCatcher::CExplodingFutbolCatcher(&this->super_CExplodingFutbolCatcher);
  (this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x559cc7);
  return;
}


/* CPropExplodingFutbolSocket::CPropExplodingFutbolSocket at 006dfa30 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CPropExplodingFutbolSocket(CPropExplodingFutbolSocket * this) */

void __thiscall
CPropExplodingFutbolSocket::CPropExplodingFutbolSocket(CPropExplodingFutbolSocket *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CExplodingFutbolCatcher::CExplodingFutbolCatcher(&this->super_CExplodingFutbolCatcher);
  (this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x559c87);
  return;
}


/* CEntityFactory<CPropExplodingFutbolSocket>::Create at 006e21b0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CPropExplodingFutbolSocket>
   * this, char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CPropExplodingFutbolSocket>::Create
          (CEntityFactory<CPropExplodingFutbolSocket> *this,char *pClassName)

{
  CExplodingFutbolCatcher *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CPropExplodingFutbolSocket * pEnt@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CBaseEntity::operator_new(0x57c);
  CExplodingFutbolCatcher::CExplodingFutbolCatcher(this_00);
  (this_00->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x557504);
  CBaseEntity::PostConstructor((CBaseEntity *)this_00,pClassName);
  return &(this_00->super_CBaseAnimating).super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CPropExplodingFutbolSocket::Precache at 006de6f0 */

/* DWARF original prototype: void Precache(CPropExplodingFutbolSocket * this) */

void __thiscall CPropExplodingFutbolSocket::Precache(CPropExplodingFutbolSocket *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::Precache((CBaseAnimating *)this);
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x332c18));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x333137));
  return;
}


/* CPropExplodingFutbolSocket::Spawn at 006dea60 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CPropExplodingFutbolSocket * this) */

void __thiscall CPropExplodingFutbolSocket::Spawn(CPropExplodingFutbolSocket *this)

{
  int nSequence;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity
    .super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1a])(this);
  (*(this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity
    .super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1b])(this,unaff_EBX + 0x3328a7);
  CCollisionProperty::SetSolid
            (&(this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.
              m_Collision.super_CCollisionProperty,SOLID_VPHYSICS);
  nSequence = CBaseAnimating::LookupSequence((CBaseAnimating *)this,(char *)(unaff_EBX + 0x332e4b));
  CBaseAnimating::SetSequence((CBaseAnimating *)this,nSequence);
  CExplodingFutbolCatcher::Spawn(&this->super_CExplodingFutbolCatcher);
  return;
}


/* CPropExplodingFutbolSocket::CaptureFutbol at 006e1c80 */

/* DWARF original prototype: void CaptureFutbol(CPropExplodingFutbolSocket * this,
   CPropExplodingFutbol * pFutbol) */

void __thiscall
CPropExplodingFutbolSocket::CaptureFutbol
          (CPropExplodingFutbolSocket *this,CPropExplodingFutbol *pFutbol)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x32fb9f),0.0,(float *)0x0);
  CExplodingFutbolCatcher::CaptureFutbol(&this->super_CExplodingFutbolCatcher,pFutbol);
  return;
}


/* _GLOBAL__I_g_szExplodingFutbolAnimThinkContext at 000c9dc0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_g_szExplodingFutbolAnimThinkContext(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

