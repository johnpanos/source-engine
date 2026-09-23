/* DWARF-guided pseudocode for game/server/portal2/prop_exploding_futbol.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000b4900 */

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
  *(undefined1 *)
   ((int)&sv_npc_talker_maxdist.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX + 2) = 0;
  *(undefined1 *)
   ((int)&sv_npc_talker_maxdist.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&sv_npc_talker_maxdist.super_ConCommandBase.m_pNext + unaff_EBX) = 0;
  *(undefined1 *)((int)&sv_npc_talker_maxdist.super_ConCommandBase.m_pNext + unaff_EBX + 1) = 0;
  *(undefined4 *)((int)&sv_npc_talker_maxdist.super_ConCommandBase.m_pNext + unaff_EBX + 2) = 0;
  *(undefined4 *)(&sv_npc_talker_maxdist.super_ConCommandBase.field_0xa + unaff_EBX) = 0;
  *(undefined4 *)((int)&sv_npc_talker_maxdist.super_ConCommandBase.m_pszName + unaff_EBX + 2) =
       0x7f7fffff;
  *(undefined4 *)((int)&sv_npc_talker_maxdist.super_ConCommandBase.m_pszHelpString + unaff_EBX + 2)
       = 0x7f7fffff;
  *(undefined4 *)((int)&sv_npc_talker_maxdist.super_ConCommandBase.m_nFlags + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&sv_npc_talker_maxdist.super_IConVar._vptr_IConVar + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&sv_npc_talker_maxdist.m_pParent + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&sv_npc_talker_maxdist.m_pszDefaultValue + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&sv_npc_talker_maxdist.m_Value.m_pszString + unaff_EBX + 2) = 0x7f7fffff;
  *(undefined4 *)((int)&sv_npc_talker_maxdist.m_Value.m_StringLength + unaff_EBX + 2) = 0x7f7fffff;
  *(undefined4 *)((int)&sv_npc_talker_maxdist.m_Value.m_fValue + unaff_EBX + 2) = 0x7f7fffff;
  *(undefined4 *)((int)&sv_npc_talker_maxdist.m_Value.m_nValue + unaff_EBX + 2) = 0x7f7fffff;
  *(undefined **)(&sv_npc_talker_maxdist.field_0x36 + unaff_EBX) = &UNK_00b979fa + unaff_EBX;
  pDefaultValue = &UNK_0099a4f2 + unaff_EBX;
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<CAI_BaseNPC>::dataDesc[0x8c].flatOffset + unaff_EBX + -0x22),
                 (char *)(unaff_EBX + 0x9ff3a6),pDefaultValue,0x4000,(char *)(unaff_EBX + 0x9ff376),
                 in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0,in_stack_ffffffe4,
                 in_stack_ffffffe8);
  uVar1 = *(undefined4 *)(&DAT_00b92246 + unaff_EBX);
  ___cxa_atexit(unaff_EBX + 0x98ac62,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<CAI_BaseNPC>::dataDesc[0x8d].flatOffset + unaff_EBX + -2),
                 (char *)(unaff_EBX + 0x9ff41a),(char *)(unaff_EBX + 0x9ff412),0x4000,
                 (char *)(unaff_EBX + 0x9ff3ca),in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,in_stack_ffffffe4,in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x98ac42,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<CAI_BaseNPC>::dataDesc[0x8f].flatOffset + unaff_EBX + -0x22),
                 (char *)(unaff_EBX + 0x9ff476),(char *)(unaff_EBX + 0x9db858),0x4000,
                 (char *)(unaff_EBX + 0x9ff43e),in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,in_stack_ffffffe4,in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x98ac22,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<CAI_BaseNPC>::dataDesc[0x90].flatOffset + unaff_EBX + -2),
                 (char *)(unaff_EBX + 0x9ff4d6),pDefaultValue,0x4000,(char *)(unaff_EBX + 0x9ff49a),
                 in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0,in_stack_ffffffe4,
                 in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x98ac02,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<CAI_BaseNPC>::dataDesc[0x92].flatOffset + unaff_EBX + -0x22),
                 (char *)(unaff_EBX + 0x9ff53e),(char *)(unaff_EBX + 0x9ff534),0x4000,
                 (char *)(unaff_EBX + 0x9ff4fe),in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,in_stack_ffffffe4,in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x98abe2,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<CAI_BaseNPC>::dataDesc[0x93].flatOffset + unaff_EBX + -2),
                 (char *)(unaff_EBX + 0x9ff5a6),(char *)(unaff_EBX + 0x9ff59f),0x4000,
                 (char *)(unaff_EBX + 0x9ff562),in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,in_stack_ffffffe4,in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x98abc2,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<CAI_BaseNPC>::dataDesc[0x95].flatOffset + unaff_EBX + -0x22),
                 (char *)(unaff_EBX + 0x9ff60e),(char *)(unaff_EBX + 0x9ff5fe),0x4000,
                 (char *)(unaff_EBX + 0x9ff5d2),in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,in_stack_ffffffe4,in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x98aba2,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<CAI_BaseNPC>::dataDesc[0x96].flatOffset + unaff_EBX + -2),
                 (char *)(unaff_EBX + 0x9ff687),(char *)(unaff_EBX + 0x9a6d70),0x4000,
                 (char *)(unaff_EBX + 0x9ff62e),in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,in_stack_ffffffe4,in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x98ab82,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<CAI_BaseNPC>::dataDesc[0x98].flatOffset + unaff_EBX + -0x22),
                 (char *)(unaff_EBX + 0x9ff6ee),(char *)(unaff_EBX + 0x9ff6de),0x4000,
                 (char *)(unaff_EBX + 0x9ff6a2),in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,in_stack_ffffffe4,in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x98ab62,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<CAI_BaseNPC>::dataDesc[0x99].flatOffset + unaff_EBX + -2),
                 (char *)(unaff_EBX + 0x9ff762),(char *)(unaff_EBX + 0x9ff755),0x4000,
                 (char *)(unaff_EBX + 0x9ff712),in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,in_stack_ffffffe4,in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x98ab42,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<CAI_BaseNPC>::dataDesc[0x9b].flatOffset + unaff_EBX + -0x22),
                 (char *)(unaff_EBX + 0x9ff7ca),(char *)(unaff_EBX + 0x9ff7c2),0x4000,
                 (char *)(unaff_EBX + 0x9ff786),in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,in_stack_ffffffe4,in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x98ab22,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<CAI_BaseNPC>::dataDesc[0x9c].flatOffset + unaff_EBX + -2),
                 (char *)(unaff_EBX + 0x9ff836),&UNK_009f724e + unaff_EBX,0x4000,
                 (char *)(unaff_EBX + 0x9ff7ee),in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,in_stack_ffffffe4,in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x98ab02,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<CAI_BaseNPC>::dataDesc[0x9e].flatOffset + unaff_EBX + -0x22),
                 (char *)(unaff_EBX + 0x9ff88e),&UNK_009a236e + unaff_EBX,0x4000,
                 (char *)(unaff_EBX + 0x9ff856),in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,in_stack_ffffffe4,in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x98aae2,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<CAI_BaseNPC>::dataDesc[0x9f].flatOffset + unaff_EBX + -2),
                 (char *)(unaff_EBX + 0x9ff8e6),pDefaultValue,0x4000,(char *)(unaff_EBX + 0x9ff8ae),
                 in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0,in_stack_ffffffe4,
                 in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x98aac2,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<CAI_BaseNPC>::dataDesc[0xa1].flatOffset + unaff_EBX + -0x22),
                 (char *)(unaff_EBX + 0x9ff952),(char *)(unaff_EBX + 0x9ff94c),2,
                 (char *)(unaff_EBX + 0x9ff90a),in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,in_stack_ffffffe4,in_stack_ffffffe8);
  ___cxa_atexit(unaff_EBX + 0x98aaa2,0,uVar1);
  pdVar2 = DataMapInit<CExplodingFutbolCatcher>((CExplodingFutbolCatcher *)0x0);
  *(datamap_t **)((int)DataMapInit<CAI_BaseNPC>::dataDesc[0xa2].flatOffset + unaff_EBX + -10) =
       pdVar2;
  *(undefined **)((int)&sv_npc_talker_maxdist.m_fMinVal + unaff_EBX + 2) = &UNK_00c1f3ca + unaff_EBX
  ;
  pIVar3 = EntityFactoryDictionary();
  (**pIVar3->_vptr_IEntityFactoryDictionary)(pIVar3,unaff_EBX + 0xd9b4fa,unaff_EBX + 0x9ff96f);
  pdVar2 = DataMapInit<CPropExplodingFutbolSpawner>((CPropExplodingFutbolSpawner *)0x0);
  *(datamap_t **)((int)DataMapInit<CAI_BaseNPC>::dataDesc[0xa2].flatOffset + unaff_EBX + -6) =
       pdVar2;
  *(undefined **)(&sv_npc_talker_maxdist.field_0x3e + unaff_EBX) = &UNK_00c1f3ea + unaff_EBX;
  pIVar3 = EntityFactoryDictionary();
  (**pIVar3->_vptr_IEntityFactoryDictionary)(pIVar3,unaff_EBX + 0xd9b4fe,unaff_EBX + 0x9ff988);
  *(undefined **)((int)&sv_npc_talker_maxdist.m_fMaxVal + unaff_EBX + 2) = &UNK_00c1f40a + unaff_EBX
  ;
  pIVar3 = EntityFactoryDictionary();
  (**pIVar3->_vptr_IEntityFactoryDictionary)(pIVar3,unaff_EBX + 0xd9b502,unaff_EBX + 0x9ff9a6);
  pdVar2 = DataMapInit<CPropExplodingFutbol>((CPropExplodingFutbol *)0x0);
  *(datamap_t **)((int)DataMapInit<CAI_BaseNPC>::dataDesc[0xa2].flatOffset + unaff_EBX + -2) =
       pdVar2;
  *(undefined **)
   ((int)&sv_npc_talker_maxdist.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 2) =
       &UNK_00c1f42a + unaff_EBX;
  pIVar3 = EntityFactoryDictionary();
  (**pIVar3->_vptr_IEntityFactoryDictionary)(pIVar3,unaff_EBX + 0xd9b506,unaff_EBX + 0x9a7aee);
  return;
}


/* __tcf_3 at 00a3f570 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38a2e8),in_stack_00000008);
  return;
}


/* __tcf_4 at 00a3f550 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38a368),in_stack_00000008);
  return;
}


/* __tcf_5 at 00a3f530 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38a3e8),in_stack_00000008);
  return;
}


/* __tcf_6 at 00a3f510 */

void __tcf_6(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38a468),in_stack_00000008);
  return;
}


/* __tcf_7 at 00a3f4f0 */

void __tcf_7(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38a4e8),in_stack_00000008);
  return;
}


/* __tcf_8 at 00a3f4d0 */

void __tcf_8(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38a568),in_stack_00000008);
  return;
}


/* __tcf_9 at 00a3f4b0 */

void __tcf_9(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38a5e8),in_stack_00000008);
  return;
}


/* __tcf_10 at 00a3f490 */

void __tcf_10(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38a668),in_stack_00000008);
  return;
}


/* __tcf_11 at 00a3f470 */

void __tcf_11(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38a6e8),in_stack_00000008);
  return;
}


/* __tcf_12 at 00a3f450 */

void __tcf_12(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38a768),in_stack_00000008);
  return;
}


/* __tcf_13 at 00a3f430 */

void __tcf_13(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38a7e8),in_stack_00000008);
  return;
}


/* __tcf_14 at 00a3f410 */

void __tcf_14(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38a868),in_stack_00000008);
  return;
}


/* __tcf_15 at 00a3f3f0 */

void __tcf_15(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38a8e8),in_stack_00000008);
  return;
}


/* __tcf_16 at 00a3f3d0 */

void __tcf_16(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38a968),in_stack_00000008);
  return;
}


/* __tcf_17 at 00a3f3b0 */

void __tcf_17(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38a9e8),in_stack_00000008);
  return;
}


/* CExplodingFutbolCatcher::CatchThink at 00750fe0 */

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
             *(float *)(unaff_EBX + 0x37bb7f);
  local_54 = *(float *)(unaff_EBX + 0x37bb0f);
  local_4c = ((this->m_vecCatcherBoxMaxs).x - (this->m_vecCatcherBoxMins).x) * local_54;
  local_48 = ((this->m_vecCatcherBoxMaxs).y - (this->m_vecCatcherBoxMins).y) * local_54;
  local_44 = ((this->m_vecCatcherBoxMaxs).z - (this->m_vecCatcherBoxMins).z) * local_54;
  local_38 = (double)(local_4c * local_4c + local_48 * local_48 + local_44 * local_44) <
             *(double *)(&DAT_0037bcff + unaff_EBX);
  local_5c = ((this->m_vecCatcherBoxMins).x + (this->m_vecCatcherBoxMaxs).x) * local_54;
  local_58 = ((this->m_vecCatcherBoxMins).y + (this->m_vecCatcherBoxMaxs).y) * local_54;
  local_54 = ((this->m_vecCatcherBoxMins).z + (this->m_vecCatcherBoxMaxs).z) * local_54;
  local_7c = local_7c + local_5c;
  local_78 = local_78 + local_58;
  local_74 = local_74 + local_54;
  local_5c = -local_5c;
  local_58 = -local_58;
  local_54 = -local_54;
  local_20[0] = unaff_EBX + 0x582d67;
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4f5bdf) + 0x14))
            ((int *)**(undefined4 **)(unaff_EBX + 0x4f5bdf),&local_7c,0x200400b,local_20,&local_d0);
  if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x4f5bdb) + 0x1c) + 0x30) != 0) {
    DebugDrawLine(&local_d0,local_c4,0xff,0,0,true,-1.0);
  }
                    /* Unresolved local var: CPropExplodingFutbol * pFutbol@[???] */
  if ((local_84 == 0) ||
     ((((local_84 != **(int **)(unaff_EBX + 0x4f5c2f) &&
        (this->m_bDisableRecaptureOnPlayerGrab == false)) &&
       (iVar1 = ___dynamic_cast(local_84,*(undefined4 *)(unaff_EBX + 0x4f5b9f),unaff_EBX + 0x582cab,
                                0), iVar1 != 0)) &&
      (((*(int *)(iVar1 + 0x768) == 0 || (*(int *)(iVar1 + 0x768) == 1)) &&
       ((*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
          super_IHandleEntity._vptr_IHandleEntity[0xeb])(this,iVar1), local_84 == 0)))))) {
    this->m_bDisableRecaptureOnPlayerGrab = false;
  }
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(**(int **)(unaff_EBX + 0x4f5b73) + 0xc) +
             *(float *)(**(int **)(unaff_EBX + 0x4f5b73) + 0x1c),(char *)0x0);
  return;
}


/* CFilterOnlyExplodingFutbol::ShouldHitEntity at 00755ab0 */

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
  cVar1 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4f1117) + 8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x4f1117),pServerEntity);
  if ((cVar1 == '\0') &&
     (this_00 = (CBaseEntity *)(*pServerEntity->_vptr_IHandleEntity[6])(pServerEntity),
     this_00 != (CBaseEntity *)0x0)) {
    if ((this_00->m_iClassname).pszValue == (char *)(unaff_EBX + 0x30693f)) {
      return true;
    }
    bVar2 = CBaseEntity::ClassMatchesComplex(this_00,(char *)(unaff_EBX + 0x30693f));
    return bVar2;
  }
  return false;
}


/* CFilterOnlyExplodingFutbol::GetTraceType at 00755840 */

/* DWARF original prototype: TraceType_t GetTraceType(CFilterOnlyExplodingFutbol * this) */

TraceType_t __thiscall CFilterOnlyExplodingFutbol::GetTraceType(CFilterOnlyExplodingFutbol *this)

{
  return TRACE_ENTITIES_ONLY;
}


/* CPropExplodingFutbolSocket::~CPropExplodingFutbolSocket at 00755950 */

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
       (_func_int_varargs **)(&UNK_0057df67 + unaff_EBX);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CExplodingFutbolCatcher).m_OnFutbolCaught.super_CBaseEntityOutput,
             in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CExplodingFutbolCatcher).m_OnFutbolReleased.super_CBaseEntityOutput,
             in_stack_ffffffe8);
  CBaseAnimating::~CBaseAnimating((CBaseAnimating *)this,__in_chrg);
  return;
}


/* CExplodingFutbolCatcher::~CExplodingFutbolCatcher at 007559c0 */

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
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x57def7);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolCaught).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolReleased).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CExplodingFutbolCatcher::~CExplodingFutbolCatcher at 00755a40 */

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
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x57de77);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolCaught).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolReleased).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
  return;
}


/* CPropExplodingFutbolSpawner::~CPropExplodingFutbolSpawner at 00755f20 */

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
       (_func_int_varargs **)(unaff_EBX + 0x57d5b4);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolGrabbed).super_CBaseEntityOutput,in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolSpawned).super_CBaseEntityOutput,in_stack_ffffffd8);
  (this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x57d994);
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


/* CPropExplodingFutbolSpawner::~CPropExplodingFutbolSpawner at 00756010 */

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
       (_func_int_varargs **)(unaff_EBX + 0x57d4c4);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolGrabbed).super_CBaseEntityOutput,in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolSpawned).super_CBaseEntityOutput,in_stack_ffffffd8);
  (this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x57d8a4);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CExplodingFutbolCatcher).m_OnFutbolCaught.super_CBaseEntityOutput,
             in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CExplodingFutbolCatcher).m_OnFutbolReleased.super_CBaseEntityOutput,
             in_stack_ffffffd8);
  CBaseAnimating::~CBaseAnimating((CBaseAnimating *)this,__in_chrg);
  return;
}


/* CPropExplodingFutbolSocket::~CPropExplodingFutbolSocket at 007560f0 */

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
       (_func_int_varargs **)(unaff_EBX + 0x57d7c7);
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


/* DataMapInit<CExplodingFutbolCatcher> at 000b3d70 */

datamap_t * DataMapInit<CExplodingFutbolCatcher>(CExplodingFutbolCatcher *param_1)

{
  int iVar1;
  int iVar2;
  char *pcVar3;
  size_t sVar4;
  int iVar5;
  undefined4 uVar6;
  int iVar7;
  undefined4 *puVar8;
  int unaff_EBX;
  int local_24;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  if (((&g_ConceptInfos[0x47].concept.field_0x2)[unaff_EBX] == '\0') &&
     (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd9c0fa), iVar2 != 0)) {
    *(undefined **)((int)&g_ConceptInfos[0x47].maxGlobalCategoryDelay + unaff_EBX + 2) =
         &UNK_009ffed8 + unaff_EBX;
    *(undefined4 *)((int)&g_ConceptInfos[0x47].maxPersonalCategoryDelay + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&g_ConceptInfos[0x47].minConceptDelay + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&g_ConceptInfos[0x47].maxConceptDelay + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&g_ConceptInfos[0x47].flags + unaff_EBX + 2) = 0;
    *(undefined4 *)(&g_ConceptInfos[0x48].concept.field_0x2 + unaff_EBX) = 0;
    *(undefined4 *)((int)&g_ConceptInfos[0x47].minPersonalCategoryDelay + unaff_EBX + 2) = 0x17;
    ___cxa_guard_release(unaff_EBX + 0xd9c0fa);
    ___cxa_atexit(&UNK_0098b812 + unaff_EBX,0,*(undefined4 *)(&DAT_00b92dd6 + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_SetScriptOwnerEntity_00cb47d8 + unaff_EBX + 2) =
       *(undefined4 *)(&DAT_00b92de2 + unaff_EBX);
  if ((*(char *)((int)&g_ConceptInfos[0x47].category + unaff_EBX + 2) != '\0') ||
     (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd9c102), iVar2 == 0)) goto LAB_000b3dc1;
  uVar6 = **(undefined4 **)(&DAT_00b92df6 + unaff_EBX);
  *(undefined4 *)((int)&PTR_GetLastUserCommand_00cb4ff4 + unaff_EBX + 2) = uVar6;
  *(undefined4 *)(&DAT_00cb5036 + unaff_EBX) = uVar6;
  pcVar3 = operator_new___(*(int *)((int)&g_ConceptInfos[0x47].minPersonalCategoryDelay +
                                   unaff_EBX + 2) + 0xb);
  _strcpy(pcVar3,*(char **)((int)&g_ConceptInfos[0x47].maxGlobalCategoryDelay + unaff_EBX + 2));
  sVar4 = _strlen(pcVar3);
  builtin_strncpy(pcVar3 + sVar4,"CatchThink",0xb);
  iVar1 = *(int *)((int)&g_ConceptInfos[0x47].flags + unaff_EBX + 2);
  iVar2 = iVar1 + 1;
  iVar5 = *(int *)((int)&g_ConceptInfos[0x47].minConceptDelay + unaff_EBX + 2);
  if (iVar5 < iVar2) {
    iVar7 = *(int *)((int)&g_ConceptInfos[0x47].maxConceptDelay + unaff_EBX + 2);
    if (-1 < iVar7) {
      if (iVar7 == 0) {
        if ((iVar5 == 0) && (iVar5 = 8, iVar2 < 9)) {
          local_20 = 0x20;
        }
        else {
          do {
            local_20 = iVar5;
            iVar5 = local_20 * 2;
          } while (iVar5 < iVar2);
          local_20 = local_20 * 8;
        }
        goto LAB_000b3f11;
      }
      iVar5 = (iVar1 / iVar7 + 1) * iVar7;
      if (iVar5 < iVar2) {
        if (iVar5 != 0) {
LAB_000b3ec0:
          do {
            iVar5 = (iVar5 + iVar2) / 2;
          } while (iVar5 < iVar2);
          goto LAB_000b3ed3;
        }
        if (iVar2 < 0) {
          iVar5 = -1;
          local_20 = -4;
        }
        else {
          local_20 = 0;
          if (iVar2 != 0) goto LAB_000b3ec0;
        }
      }
      else {
LAB_000b3ed3:
        local_20 = iVar5 * 4;
      }
LAB_000b3f11:
      *(int *)((int)&g_ConceptInfos[0x47].minConceptDelay + unaff_EBX + 2) = iVar5;
      iVar5 = *(int *)((int)&g_ConceptInfos[0x47].maxPersonalCategoryDelay + unaff_EBX + 2);
      if (iVar5 == 0) {
        uVar6 = (*(code *)**(undefined4 **)**(undefined4 **)(&DAT_00b92dde + unaff_EBX))
                          ((undefined4 *)**(undefined4 **)(&DAT_00b92dde + unaff_EBX),local_20);
      }
      else {
        uVar6 = (**(code **)(*(int *)**(undefined4 **)(&DAT_00b92dde + unaff_EBX) + 4))
                          ((int *)**(undefined4 **)(&DAT_00b92dde + unaff_EBX),iVar5,local_20);
      }
      *(undefined4 *)((int)&g_ConceptInfos[0x47].maxPersonalCategoryDelay + unaff_EBX + 2) = uVar6;
    }
  }
  iVar7 = *(int *)((int)&g_ConceptInfos[0x47].flags + unaff_EBX + 2) + 1;
  *(int *)((int)&g_ConceptInfos[0x47].flags + unaff_EBX + 2) = iVar7;
  iVar5 = *(int *)((int)&g_ConceptInfos[0x47].maxPersonalCategoryDelay + unaff_EBX + 2);
  *(int *)(&g_ConceptInfos[0x48].concept.field_0x2 + unaff_EBX) = iVar5;
  iVar7 = (iVar7 - iVar1) + -1;
  if (0 < iVar7) {
    _V_memmove((void *)(iVar5 + iVar2 * 4),(void *)(iVar5 + iVar1 * 4),iVar7 * 4);
    iVar5 = *(int *)((int)&g_ConceptInfos[0x47].maxPersonalCategoryDelay + unaff_EBX + 2);
  }
  puVar8 = (undefined4 *)(iVar1 * 4 + iVar5);
  if (puVar8 != (undefined4 *)0x0) {
    *puVar8 = pcVar3;
  }
  *(char **)((int)&PTR_typeinfo_00cb5064 + unaff_EBX + 2) = pcVar3;
  pcVar3 = operator_new___(*(int *)((int)&g_ConceptInfos[0x47].minPersonalCategoryDelay +
                                   unaff_EBX + 2) + 0xd);
  _strcpy(pcVar3,*(char **)((int)&g_ConceptInfos[0x47].maxGlobalCategoryDelay + unaff_EBX + 2));
  sVar4 = _strlen(pcVar3);
  builtin_strncpy(pcVar3 + sVar4,"CaptureThink",0xd);
  iVar1 = *(int *)((int)&g_ConceptInfos[0x47].flags + unaff_EBX + 2);
  iVar2 = iVar1 + 1;
  iVar5 = *(int *)((int)&g_ConceptInfos[0x47].minConceptDelay + unaff_EBX + 2);
  if (iVar5 < iVar2) {
    iVar7 = *(int *)((int)&g_ConceptInfos[0x47].maxConceptDelay + unaff_EBX + 2);
    if (-1 < iVar7) {
      if (iVar7 == 0) {
        if ((iVar5 == 0) && (iVar5 = 8, iVar2 < 9)) {
          local_24 = 0x20;
        }
        else {
          do {
            local_24 = iVar5;
            iVar5 = local_24 * 2;
          } while (iVar5 < iVar2);
          local_24 = local_24 * 8;
        }
        goto LAB_000b4131;
      }
      iVar5 = (iVar1 / iVar7 + 1) * iVar7;
      if (iVar5 < iVar2) {
        if (iVar5 != 0) {
LAB_000b4030:
          do {
            iVar5 = (iVar5 + iVar2) / 2;
          } while (iVar5 < iVar2);
          goto LAB_000b4043;
        }
        if (iVar2 < 0) {
          iVar5 = -1;
          local_24 = -4;
        }
        else {
          local_24 = 0;
          if (iVar2 != 0) goto LAB_000b4030;
        }
      }
      else {
LAB_000b4043:
        local_24 = iVar5 * 4;
      }
LAB_000b4131:
      *(int *)((int)&g_ConceptInfos[0x47].minConceptDelay + unaff_EBX + 2) = iVar5;
      iVar5 = *(int *)((int)&g_ConceptInfos[0x47].maxPersonalCategoryDelay + unaff_EBX + 2);
      if (iVar5 == 0) {
        uVar6 = (*(code *)**(undefined4 **)**(undefined4 **)(&DAT_00b92dde + unaff_EBX))
                          ((undefined4 *)**(undefined4 **)(&DAT_00b92dde + unaff_EBX),local_24);
      }
      else {
        uVar6 = (**(code **)(*(int *)**(undefined4 **)(&DAT_00b92dde + unaff_EBX) + 4))
                          ((int *)**(undefined4 **)(&DAT_00b92dde + unaff_EBX),iVar5,local_24);
      }
      *(undefined4 *)((int)&g_ConceptInfos[0x47].maxPersonalCategoryDelay + unaff_EBX + 2) = uVar6;
    }
  }
  iVar7 = *(int *)((int)&g_ConceptInfos[0x47].flags + unaff_EBX + 2) + 1;
  *(int *)((int)&g_ConceptInfos[0x47].flags + unaff_EBX + 2) = iVar7;
  iVar5 = *(int *)((int)&g_ConceptInfos[0x47].maxPersonalCategoryDelay + unaff_EBX + 2);
  *(int *)(&g_ConceptInfos[0x48].concept.field_0x2 + unaff_EBX) = iVar5;
  iVar7 = (iVar7 - iVar1) + -1;
  if (0 < iVar7) {
    _V_memmove((void *)(iVar5 + iVar2 * 4),(void *)(iVar5 + iVar1 * 4),iVar7 * 4);
    iVar5 = *(int *)((int)&g_ConceptInfos[0x47].maxPersonalCategoryDelay + unaff_EBX + 2);
  }
  puVar8 = (undefined4 *)(iVar1 * 4 + iVar5);
  if (puVar8 != (undefined4 *)0x0) {
    *puVar8 = pcVar3;
  }
  *(char **)((int)&PTR_TestCollision_00cb50a4 + unaff_EBX + 2) = pcVar3;
  ___cxa_guard_release(unaff_EBX + 0xd9c102);
LAB_000b3dc1:
  *(undefined4 *)((int)&PTR_ShouldCollide_00cb47d0 + unaff_EBX + 2) = 9;
  *(int *)((int)&PTR_ComputeWorldSpaceSurroundingBox_00cb47cc + unaff_EBX + 2) =
       unaff_EBX + 0xcb4ea2;
  return (datamap_t *)((int)&PTR_ComputeWorldSpaceSurroundingBox_00cb47cc + unaff_EBX + 2);
}


/* CExplodingFutbolCatcher::GetDataDescMap at 00750830 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CExplodingFutbolCatcher * this) */

datamap_t * __thiscall CExplodingFutbolCatcher::GetDataDescMap(CExplodingFutbolCatcher *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x617d14);
}


/* CExplodingFutbolCatcher::GetBaseMap at 00750840 */

datamap_t * CExplodingFutbolCatcher::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(&DAT_004f6318 + extraout_ECX);
}


/* __tcf_0 at 00a3f590 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x4108fe)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x4108f2) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x4108fe));
  }
  *(undefined4 *)(unaff_EBX + 0x4108fe) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x4108fa)) {
    if (*(int *)(unaff_EBX + 0x4108f2) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2075be) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2075be),*(int *)(unaff_EBX + 0x4108f2));
      *(undefined4 *)(unaff_EBX + 0x4108f2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x4108f6) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x4108f2);
  *(int *)(unaff_EBX + 0x410902) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x4108fa)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2075be) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2075be),iVar1);
      *(undefined4 *)(unaff_EBX + 0x4108f2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x4108f6) = 0;
  }
  return;
}


/* DataMapInit<CPropExplodingFutbolSpawner> at 000b3c40 */

datamap_t * DataMapInit<CPropExplodingFutbolSpawner>(CPropExplodingFutbolSpawner *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&g_ConceptInfos[0x4d].minGlobalCategoryDelay + unaff_EBX) == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd9c1f4);
    if (iVar2 != 0) {
      *(int *)((int)&g_ConceptInfos[0x4d].minConceptDelay + unaff_EBX) = unaff_EBX + 0x9fffe6;
      *(undefined4 *)((int)&g_ConceptInfos[0x4d].flags + unaff_EBX) = 0;
      *(undefined4 *)
       ((int)&g_ConceptInfos[0x4e].concept.super_CRR_Concept.m_iConcept.m_Id + unaff_EBX) = 0;
      *(undefined4 *)
       ((int)&g_ConceptInfos[0x4e].concept.m_hSpeaker.super_CBaseHandle.m_Index + unaff_EBX) = 0;
      *(undefined4 *)((int)&g_ConceptInfos[0x4e].category + unaff_EBX) = 0;
      *(undefined4 *)((int)&g_ConceptInfos[0x4e].minGlobalCategoryDelay + unaff_EBX) = 0;
      *(undefined4 *)((int)&g_ConceptInfos[0x4d].maxConceptDelay + unaff_EBX) = 0x1b;
      ___cxa_guard_release(unaff_EBX + 0xd9c1f4);
      ___cxa_atexit(&UNK_0098bafc + unaff_EBX,0,*(undefined4 *)(&DAT_00b92f00 + unaff_EBX));
    }
  }
  *(int *)((int)&PTR_Use_00cb491c + unaff_EBX) = (int)&PTR_GetServerVehicle_00cb48f8 + unaff_EBX;
  if (*(char *)((int)&g_ConceptInfos[0x4d].minPersonalCategoryDelay + unaff_EBX) == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd9c1fc);
    if (iVar2 != 0) {
      uVar1 = **(undefined4 **)(&DAT_00b92f20 + unaff_EBX);
      *(undefined4 *)(&DAT_00cb4f20 + unaff_EBX) = uVar1;
      *(undefined4 *)(&CPlayerInfo::vtable + unaff_EBX) = uVar1;
      ___cxa_guard_release(unaff_EBX + 0xd9c1fc);
    }
  }
  *(undefined4 *)((int)&PTR_GetEnemy_00cb4914 + unaff_EBX) = 7;
  *(int *)((int)&PTR_GetEnemy_00cb4910 + unaff_EBX) = (int)&PTR_GetMaxHealth_00cb4dcc + unaff_EBX;
  return (datamap_t *)((int)&PTR_GetEnemy_00cb4910 + unaff_EBX);
}


/* CPropExplodingFutbolSpawner::GetDataDescMap at 00750850 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPropExplodingFutbolSpawner * this) */

datamap_t * __thiscall
CPropExplodingFutbolSpawner::GetDataDescMap(CPropExplodingFutbolSpawner *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x617d0c);
}


/* CPropExplodingFutbolSpawner::GetBaseMap at 00750860 */

datamap_t * CPropExplodingFutbolSpawner::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x617ce4);
}


/* __tcf_1 at 00a3f750 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(&DAT_0041070e + unaff_EBX)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(&DAT_00410702 + unaff_EBX) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(&DAT_0041070e + unaff_EBX));
  }
  *(undefined4 *)(&DAT_0041070e + unaff_EBX) = 0;
  if (-1 < *(int *)(&DAT_0041070a + unaff_EBX)) {
    if (*(int *)(&DAT_00410702 + unaff_EBX) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2073fe) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2073fe),*(int *)(&DAT_00410702 + unaff_EBX))
      ;
      *(undefined4 *)(&DAT_00410702 + unaff_EBX) = 0;
    }
    *(undefined4 *)(&DAT_00410706 + unaff_EBX) = 0;
  }
  iVar1 = *(int *)(&DAT_00410702 + unaff_EBX);
  *(int *)(CBaseEntity::SetScriptOwnerEntity + unaff_EBX + 2) = iVar1;
  if (-1 < *(int *)(&DAT_0041070a + unaff_EBX)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2073fe) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2073fe),iVar1);
      *(undefined4 *)(&DAT_00410702 + unaff_EBX) = 0;
    }
    *(undefined4 *)(&DAT_00410706 + unaff_EBX) = 0;
  }
  return;
}


/* DataMapInit<CPropExplodingFutbol> at 000b4260 */

datamap_t * DataMapInit<CPropExplodingFutbol>(CPropExplodingFutbol *param_1)

{
  int iVar1;
  int iVar2;
  char *pcVar3;
  size_t sVar4;
  int iVar5;
  undefined4 uVar6;
  int iVar7;
  undefined4 *puVar8;
  int unaff_EBX;
  int local_28;
  int local_24;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  if (((&g_ConceptInfos[0x25].concept.field_0x2)[unaff_EBX] == '\0') &&
     (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd9bbaa), iVar2 != 0)) {
    *(int *)((int)&g_ConceptInfos[0x25].maxGlobalCategoryDelay + unaff_EBX + 2) =
         unaff_EBX + 0x9ffa00;
    *(undefined4 *)((int)&g_ConceptInfos[0x25].maxPersonalCategoryDelay + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&g_ConceptInfos[0x25].minConceptDelay + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&g_ConceptInfos[0x25].maxConceptDelay + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&g_ConceptInfos[0x25].flags + unaff_EBX + 2) = 0;
    *(undefined4 *)(&g_ConceptInfos[0x26].concept.field_0x2 + unaff_EBX) = 0;
    *(undefined4 *)((int)&g_ConceptInfos[0x25].minPersonalCategoryDelay + unaff_EBX + 2) = 0x14;
    ___cxa_guard_release(unaff_EBX + 0xd9bbaa);
    ___cxa_atexit(unaff_EBX + 0x98b402,0,*(undefined4 *)(&DAT_00b928e6 + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_NetworkStateChanged_m_takedamage_00cb4318 + unaff_EBX + 2) =
       *(undefined4 *)(&DAT_00b932f6 + unaff_EBX);
  if ((*(char *)((int)&g_ConceptInfos[0x25].category + unaff_EBX + 2) != '\0') ||
     (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd9bbb2), iVar2 == 0)) goto LAB_000b42b1;
  pcVar3 = operator_new___(*(int *)((int)&g_ConceptInfos[0x25].minPersonalCategoryDelay +
                                   unaff_EBX + 2) + 10);
  _strcpy(pcVar3,*(char **)((int)&g_ConceptInfos[0x25].maxGlobalCategoryDelay + unaff_EBX + 2));
  sVar4 = _strlen(pcVar3);
  builtin_strncpy(pcVar3 + sVar4,"AnimThink",10);
  iVar1 = *(int *)((int)&g_ConceptInfos[0x25].flags + unaff_EBX + 2);
  iVar2 = iVar1 + 1;
  iVar5 = *(int *)((int)&g_ConceptInfos[0x25].minConceptDelay + unaff_EBX + 2);
  if (iVar5 < iVar2) {
    iVar7 = *(int *)((int)&g_ConceptInfos[0x25].maxConceptDelay + unaff_EBX + 2);
    if (-1 < iVar7) {
      if (iVar7 == 0) {
        if ((iVar5 == 0) && (iVar5 = 8, iVar2 < 9)) {
          local_20 = 0x20;
        }
        else {
          do {
            local_20 = iVar5;
            iVar5 = local_20 * 2;
          } while (iVar5 < iVar2);
          local_20 = local_20 * 8;
        }
        goto LAB_000b43f1;
      }
      iVar5 = (iVar1 / iVar7 + 1) * iVar7;
      if (iVar5 < iVar2) {
        if (iVar5 != 0) {
LAB_000b43a0:
          do {
            iVar5 = (iVar5 + iVar2) / 2;
          } while (iVar5 < iVar2);
          goto LAB_000b43b3;
        }
        if (iVar2 < 0) {
          iVar5 = -1;
          local_20 = -4;
        }
        else {
          local_20 = 0;
          if (iVar2 != 0) goto LAB_000b43a0;
        }
      }
      else {
LAB_000b43b3:
        local_20 = iVar5 * 4;
      }
LAB_000b43f1:
      *(int *)((int)&g_ConceptInfos[0x25].minConceptDelay + unaff_EBX + 2) = iVar5;
      iVar5 = *(int *)((int)&g_ConceptInfos[0x25].maxPersonalCategoryDelay + unaff_EBX + 2);
      if (iVar5 == 0) {
        uVar6 = (*(code *)**(undefined4 **)**(undefined4 **)(&DAT_00b928ee + unaff_EBX))
                          ((undefined4 *)**(undefined4 **)(&DAT_00b928ee + unaff_EBX),local_20);
      }
      else {
        uVar6 = (**(code **)(*(int *)**(undefined4 **)(&DAT_00b928ee + unaff_EBX) + 4))
                          ((int *)**(undefined4 **)(&DAT_00b928ee + unaff_EBX),iVar5,local_20);
      }
      *(undefined4 *)((int)&g_ConceptInfos[0x25].maxPersonalCategoryDelay + unaff_EBX + 2) = uVar6;
    }
  }
  iVar7 = *(int *)((int)&g_ConceptInfos[0x25].flags + unaff_EBX + 2) + 1;
  *(int *)((int)&g_ConceptInfos[0x25].flags + unaff_EBX + 2) = iVar7;
  iVar5 = *(int *)((int)&g_ConceptInfos[0x25].maxPersonalCategoryDelay + unaff_EBX + 2);
  *(int *)(&g_ConceptInfos[0x26].concept.field_0x2 + unaff_EBX) = iVar5;
  iVar7 = (iVar7 - iVar1) + -1;
  if (0 < iVar7) {
    _V_memmove((void *)(iVar5 + iVar2 * 4),(void *)(iVar5 + iVar1 * 4),iVar7 * 4);
    iVar5 = *(int *)((int)&g_ConceptInfos[0x25].maxPersonalCategoryDelay + unaff_EBX + 2);
  }
  puVar8 = (undefined4 *)(iVar1 * 4 + iVar5);
  if (puVar8 != (undefined4 *)0x0) {
    *puVar8 = pcVar3;
  }
  *(char **)((int)&PTR_EarPosition_00cb4674 + unaff_EBX + 2) = pcVar3;
  pcVar3 = operator_new___(*(int *)((int)&g_ConceptInfos[0x25].minPersonalCategoryDelay +
                                   unaff_EBX + 2) + 0xb);
  _strcpy(pcVar3,*(char **)((int)&g_ConceptInfos[0x25].maxGlobalCategoryDelay + unaff_EBX + 2));
  sVar4 = _strlen(pcVar3);
  builtin_strncpy(pcVar3 + sVar4,"TimerThink",0xb);
  iVar1 = *(int *)((int)&g_ConceptInfos[0x25].flags + unaff_EBX + 2);
  iVar2 = iVar1 + 1;
  iVar5 = *(int *)((int)&g_ConceptInfos[0x25].minConceptDelay + unaff_EBX + 2);
  if (iVar5 < iVar2) {
    iVar7 = *(int *)((int)&g_ConceptInfos[0x25].maxConceptDelay + unaff_EBX + 2);
    if (-1 < iVar7) {
      if (iVar7 == 0) {
        if ((iVar5 == 0) && (iVar5 = 8, iVar2 < 9)) {
          local_28 = 0x20;
        }
        else {
          do {
            local_28 = iVar5;
            iVar5 = local_28 * 2;
          } while (iVar5 < iVar2);
          local_28 = local_28 * 8;
        }
        goto LAB_000b4611;
      }
      iVar5 = (iVar1 / iVar7 + 1) * iVar7;
      if (iVar5 < iVar2) {
        if (iVar5 != 0) {
LAB_000b4510:
          do {
            iVar5 = (iVar5 + iVar2) / 2;
          } while (iVar5 < iVar2);
          goto LAB_000b4523;
        }
        if (iVar2 < 0) {
          iVar5 = -1;
          local_28 = -4;
        }
        else {
          local_28 = 0;
          if (iVar2 != 0) goto LAB_000b4510;
        }
      }
      else {
LAB_000b4523:
        local_28 = iVar5 * 4;
      }
LAB_000b4611:
      *(int *)((int)&g_ConceptInfos[0x25].minConceptDelay + unaff_EBX + 2) = iVar5;
      iVar5 = *(int *)((int)&g_ConceptInfos[0x25].maxPersonalCategoryDelay + unaff_EBX + 2);
      if (iVar5 == 0) {
        uVar6 = (*(code *)**(undefined4 **)**(undefined4 **)(&DAT_00b928ee + unaff_EBX))
                          ((undefined4 *)**(undefined4 **)(&DAT_00b928ee + unaff_EBX),local_28);
      }
      else {
        uVar6 = (**(code **)(*(int *)**(undefined4 **)(&DAT_00b928ee + unaff_EBX) + 4))
                          ((int *)**(undefined4 **)(&DAT_00b928ee + unaff_EBX),iVar5,local_28);
      }
      *(undefined4 *)((int)&g_ConceptInfos[0x25].maxPersonalCategoryDelay + unaff_EBX + 2) = uVar6;
    }
  }
  iVar7 = *(int *)((int)&g_ConceptInfos[0x25].flags + unaff_EBX + 2) + 1;
  *(int *)((int)&g_ConceptInfos[0x25].flags + unaff_EBX + 2) = iVar7;
  iVar5 = *(int *)((int)&g_ConceptInfos[0x25].maxPersonalCategoryDelay + unaff_EBX + 2);
  *(int *)(&g_ConceptInfos[0x26].concept.field_0x2 + unaff_EBX) = iVar5;
  iVar7 = (iVar7 - iVar1) + -1;
  if (0 < iVar7) {
    _V_memmove((void *)(iVar5 + iVar2 * 4),(void *)(iVar5 + iVar1 * 4),iVar7 * 4);
    iVar5 = *(int *)((int)&g_ConceptInfos[0x25].maxPersonalCategoryDelay + unaff_EBX + 2);
  }
  puVar8 = (undefined4 *)(iVar1 * 4 + iVar5);
  if (puVar8 != (undefined4 *)0x0) {
    *puVar8 = pcVar3;
  }
  *(char **)((int)&PTR_PhysicsSplash_00cb46b4 + unaff_EBX + 2) = pcVar3;
  pcVar3 = operator_new___(*(int *)((int)&g_ConceptInfos[0x25].minPersonalCategoryDelay +
                                   unaff_EBX + 2) + 10);
  _strcpy(pcVar3,*(char **)((int)&g_ConceptInfos[0x25].maxGlobalCategoryDelay + unaff_EBX + 2));
  sVar4 = _strlen(pcVar3);
  builtin_strncpy(pcVar3 + sVar4,"KillThink",10);
  iVar1 = *(int *)((int)&g_ConceptInfos[0x25].flags + unaff_EBX + 2);
  iVar2 = iVar1 + 1;
  iVar5 = *(int *)((int)&g_ConceptInfos[0x25].minConceptDelay + unaff_EBX + 2);
  if (iVar5 < iVar2) {
    iVar7 = *(int *)((int)&g_ConceptInfos[0x25].maxConceptDelay + unaff_EBX + 2);
    if (-1 < iVar7) {
      if (iVar7 == 0) {
        if ((iVar5 == 0) && (iVar5 = 8, iVar2 < 9)) {
          local_24 = 0x20;
        }
        else {
          do {
            local_24 = iVar5;
            iVar5 = local_24 * 2;
          } while (iVar5 < iVar2);
          local_24 = local_24 * 8;
        }
        goto LAB_000b4771;
      }
      iVar5 = (iVar1 / iVar7 + 1) * iVar7;
      if (iVar5 < iVar2) {
        if (iVar5 != 0) {
LAB_000b4720:
          do {
            iVar5 = (iVar5 + iVar2) / 2;
          } while (iVar5 < iVar2);
          goto LAB_000b4733;
        }
        if (iVar2 < 0) {
          iVar5 = -1;
          local_24 = -4;
        }
        else {
          local_24 = 0;
          if (iVar2 != 0) goto LAB_000b4720;
        }
      }
      else {
LAB_000b4733:
        local_24 = iVar5 * 4;
      }
LAB_000b4771:
      *(int *)((int)&g_ConceptInfos[0x25].minConceptDelay + unaff_EBX + 2) = iVar5;
      iVar5 = *(int *)((int)&g_ConceptInfos[0x25].maxPersonalCategoryDelay + unaff_EBX + 2);
      if (iVar5 == 0) {
        uVar6 = (*(code *)**(undefined4 **)**(undefined4 **)(&DAT_00b928ee + unaff_EBX))
                          ((undefined4 *)**(undefined4 **)(&DAT_00b928ee + unaff_EBX),local_24);
      }
      else {
        uVar6 = (**(code **)(*(int *)**(undefined4 **)(&DAT_00b928ee + unaff_EBX) + 4))
                          ((int *)**(undefined4 **)(&DAT_00b928ee + unaff_EBX),iVar5,local_24);
      }
      *(undefined4 *)((int)&g_ConceptInfos[0x25].maxPersonalCategoryDelay + unaff_EBX + 2) = uVar6;
    }
  }
  iVar7 = *(int *)((int)&g_ConceptInfos[0x25].flags + unaff_EBX + 2) + 1;
  *(int *)((int)&g_ConceptInfos[0x25].flags + unaff_EBX + 2) = iVar7;
  iVar5 = *(int *)((int)&g_ConceptInfos[0x25].maxPersonalCategoryDelay + unaff_EBX + 2);
  *(int *)(&g_ConceptInfos[0x26].concept.field_0x2 + unaff_EBX) = iVar5;
  iVar7 = (iVar7 - iVar1) + -1;
  if (0 < iVar7) {
    _V_memmove((void *)(iVar5 + iVar2 * 4),(void *)(iVar5 + iVar1 * 4),iVar7 * 4);
    iVar5 = *(int *)((int)&g_ConceptInfos[0x25].maxPersonalCategoryDelay + unaff_EBX + 2);
  }
  puVar8 = (undefined4 *)(iVar1 * 4 + iVar5);
  if (puVar8 != (undefined4 *)0x0) {
    *puVar8 = pcVar3;
  }
  *(char **)((int)&PTR_CanPushEntity_00cb46f4 + unaff_EBX + 2) = pcVar3;
  ___cxa_guard_release(unaff_EBX + 0xd9bbb2);
LAB_000b42b1:
  *(undefined4 *)((int)&PTR_NetworkStateChanged_m_lifeState_00cb4310 + unaff_EBX + 2) = 0x10;
  *(int *)((int)&PTR_NetworkStateChanged_m_lifeState_00cb430c + unaff_EBX + 2) =
       unaff_EBX + 0xcb4372;
  return (datamap_t *)((int)&PTR_NetworkStateChanged_m_lifeState_00cb430c + unaff_EBX + 2);
}


/* CPropExplodingFutbol::GetDataDescMap at 00750870 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPropExplodingFutbol * this) */

datamap_t * __thiscall CPropExplodingFutbol::GetDataDescMap(CPropExplodingFutbol *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x617d04);
}


/* CPropExplodingFutbol::GetBaseMap at 00750880 */

datamap_t * CPropExplodingFutbol::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4f6cdc);
}


/* __tcf_2 at 00a3f670 */

void __tcf_2(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x4107be)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x4107b2) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x4107be));
  }
  *(undefined4 *)(unaff_EBX + 0x4107be) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x4107ba)) {
    if (*(int *)(unaff_EBX + 0x4107b2) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2074de) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2074de),*(int *)(unaff_EBX + 0x4107b2));
      *(undefined4 *)(unaff_EBX + 0x4107b2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x4107b6) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x4107b2);
  *(int *)(&DAT_004107c2 + unaff_EBX) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x4107ba)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2074de) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2074de),iVar1);
      *(undefined4 *)(unaff_EBX + 0x4107b2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x4107b6) = 0;
  }
  return;
}


/* CPropExplodingFutbol::CPropExplodingFutbol at 00751cc0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CPropExplodingFutbol(CPropExplodingFutbol * this) */

void __thiscall CPropExplodingFutbol::CPropExplodingFutbol(CPropExplodingFutbol *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CPhysicsProp::CPhysicsProp(&this->super_CPhysicsProp);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
  super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x5812db);
  (this->super_CPhysicsProp).super_CBreakableProp.super_IBreakableWithPropData.
  _vptr_IBreakableWithPropData = (_func_int_varargs **)(unaff_EBX + 0x581743);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x5817c7);
  (this->super_CPhysicsProp).super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(CMultiSource::~CMultiSource + unaff_EBX + 7);
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
  this->m_bExplodeOnTouch = true;
  this->m_flExplosionTimer = 0.0;
  this->m_flTotalTimer = 0.0;
  this->m_flLastTickTime = 0.0;
  this->m_flLastTimerSoundTime = 0.0;
  this->m_flLastFlashTime = 0.0;
  return;
}


/* CPropExplodingFutbol::CPropExplodingFutbol at 00751da0 */

/* DWARF original prototype: void CPropExplodingFutbol(CPropExplodingFutbol * this,
   CPropExplodingFutbol * this) */

void __thiscall
CPropExplodingFutbol::CPropExplodingFutbol(CPropExplodingFutbol *this,CPropExplodingFutbol *this_1)

{
  CPropExplodingFutbol(this);
  return;
}


/* CEntityFactory<CPropExplodingFutbol>::Create at 00755d80 */

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


/* CPropExplodingFutbol::Precache at 00750f80 */

/* DWARF original prototype: void Precache(CPropExplodingFutbol * this) */

void __thiscall CPropExplodingFutbol::Precache(CPropExplodingFutbol *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CPhysicsProp::Precache(&this->super_CPhysicsProp);
  CBaseEntity::PrecacheModel(&UNK_00362570 + unaff_EBX);
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x362bb4));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x3629c8));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x362bd6));
  return;
}


/* CPropExplodingFutbol::Spawn at 00751940 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CPropExplodingFutbol * this) */

void __thiscall CPropExplodingFutbol::Spawn(CPropExplodingFutbol *this)

{
  uint *puVar1;
  ushort uVar2;
  CBaseEdict *this_00;
  ushort *puVar3;
  BASEPTR func;
  int iVar4;
  CBaseEntity *pCVar5;
  int *piVar6;
  ulong *puVar7;
  IChangeInfoAccessor *pIVar8;
  char *pcVar9;
  uint uVar10;
  uint uVar11;
  int unaff_EBX;
  uint uVar12;
  uint local_60;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
  super_CBaseEntity.m_ModelName.pszValue = (char *)(unaff_EBX + 0x361bae);
  CBaseEntity::DispatchUpdateTransmitState((CBaseEntity *)this);
  (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x80))(this,unaff_EBX + 0x355622,(char *)(unaff_EBX + 0x361bae));
  if (this->m_bShouldRespawn != false) {
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
    uVar10 = (this->m_hSpawner).super_CBaseHandle.m_Index;
    if (((uVar10 == 0xffffffff) ||
        (iVar4 = (uVar10 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f521e),
        *(uint *)(iVar4 + 8) != uVar10 >> 0x10)) || (*(int *)(iVar4 + 4) == 0)) {
      pcVar9 = (this->m_strSpawnerName).pszValue;
      if (pcVar9 == (char *)0x0) {
        _Warning(unaff_EBX + 0x36227e);
      }
      else {
        pCVar5 = CGlobalEntityList::FindEntityByName
                           (*(CGlobalEntityList **)(unaff_EBX + 0x4f52d6),(CBaseEntity *)0x0,pcVar9,
                            (CBaseEntity *)0x0,(CBaseEntity *)0x0,(CBaseEntity *)0x0,
                            (IEntityFindFilter *)0x0);
        if (pCVar5 != (CBaseEntity *)0x0) {
          piVar6 = (int *)___dynamic_cast(pCVar5,*(undefined4 *)(unaff_EBX + 0x4f5242),
                                          unaff_EBX + 0x582336,0);
          if (piVar6 == (int *)0x0) {
            pcVar9 = CBaseEntity::GetDebugName((CBaseEntity *)0x0);
            iVar4 = section_00000054.segname._4_4_;
            if (section_00000054.segname._4_4_ == 0) {
              iVar4 = unaff_EBX + 0x2fd3d6;
            }
            _Warning(&UNK_0036222a + unaff_EBX,iVar4,pcVar9);
          }
          else {
            puVar7 = (ulong *)(**(code **)(*piVar6 + 0xc))(piVar6);
            (this->m_hSpawner).super_CBaseHandle.m_Index = *puVar7;
          }
        }
      }
    }
  }
  CPhysicsProp::Spawn(&this->super_CPhysicsProp);
  iVar4 = CBaseAnimating::LookupSequence((CBaseAnimating *)this,(char *)(unaff_EBX + 0x3622be));
  CBaseAnimating::ResetSequence((CBaseAnimating *)this,iVar4);
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0xd7e);
  CBaseEntity::ThinkSet
            ((CBaseEntity *)this,func,
             *(float *)(unaff_EBX + 0x37afc6) + *(float *)(**(int **)(unaff_EBX + 0x4f5216) + 0xc),
             *(char **)(unaff_EBX + 0x616bf2));
  uVar10 = (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
           super_CBaseEntity.m_spawnflags.m_Value;
  uVar12 = uVar10 | 0x140;
  if (uVar10 != uVar12) {
    if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
        false) {
      this_00 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                  super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar8 = CBaseEdict::GetChangeAccessor(this_00);
        puVar3 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4f521a);
        if (pIVar8->m_iChangeInfoSerialNumber == *puVar3) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar10 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar3[uVar10 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_00751c84:
            puVar3[(uint)uVar2 + uVar10 * 0x14 + 1] = 0x128;
            puVar3[uVar10 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar3[uVar10 * 0x14 + 1] != 0x128) {
            local_60 = 0;
            do {
              uVar11 = local_60 + 1;
              local_60 = uVar11 & 0xffff;
              if ((ushort)uVar11 == uVar2) {
                if (uVar2 == 0x13) goto LAB_00751ca9;
                goto LAB_00751c84;
              }
            } while (puVar3[uVar10 * 0x14 + local_60 + 1] != 0x128);
          }
        }
        else if (puVar3[0x7d1] == 100) {
LAB_00751ca9:
          pIVar8->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar8->m_iChangeInfo = puVar3[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4f521a) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4f521a) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x4f521a);
          pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar4 = *piVar6 + (uint)pIVar8->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar4 + 2) = 0x128;
          *(undefined2 *)(iVar4 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = (uint *)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
      *puVar1 = *puVar1 | 1;
    }
    (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
    super_CBaseEntity.m_spawnflags.m_Value = uVar12;
  }
  return;
}


/* CPropExplodingFutbol::VPhysicsUpdate at 007523f0 */

/* DWARF original prototype: void VPhysicsUpdate(CPropExplodingFutbol * this, IPhysicsObject *
   pPhysics) */

void __thiscall
CPropExplodingFutbol::VPhysicsUpdate(CPropExplodingFutbol *this,IPhysicsObject *pPhysics)

{
  vec_t *pvVar1;
  int iVar2;
  int iVar3;
  CPortal_Base2D *this_00;
  bool bVar4;
  int unaff_EBX;
  int iVar5;
  float fVar6;
  float fVar7;
  CPortal_Base2D *local_b4;
  float local_b0;
  float local_a8;
  float local_a4;
  float local_a0;
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
  float local_28;
  float local_24;
  float local_20 [4];
  
                    /* Unresolved local var: Vector vVelocity@[???]
                       Unresolved local var: Vector vPosition@[???] */
  ___i686_get_pc_thunk_bx();
  CPhysicsProp::VPhysicsUpdate(&this->super_CPhysicsProp,pPhysics);
  (*pPhysics->_vptr_IPhysicsObject[0x35])(pPhysics,&local_30,0);
  (*pPhysics->_vptr_IPhysicsObject[0x31])(pPhysics,&local_3c,0);
  if (local_28 < 0.0) {
                    /* Unresolved local var: Vector vPropOrigin@[???]
                       Unresolved local var: int iPortalCount@[???] */
    (*pPhysics->_vptr_IPhysicsObject[0x31])(pPhysics,&local_48,0);
    iVar2 = (*(int **)(unaff_EBX + 0x4f49e3))[3];
    if (iVar2 != 0) {
                    /* Unresolved local var: CPortal_Base2D * pFunnelInto@[???]
                       Unresolved local var: Vector vPropToFunnelPortal@[???]
                       Unresolved local var: float fClosestFunnelPortalDistSqr@[???]
                       Unresolved local var: CPortal_Base2D * * pPortals@[???] */
      iVar3 = **(int **)(unaff_EBX + 0x4f49e3);
      local_b4 = (CPortal_Base2D *)0x0;
      local_b0 = *(float *)(unaff_EBX + 0x37a8af);
      iVar5 = 0;
      do {
        this_00 = *(CPortal_Base2D **)(iVar3 + iVar5 * 4);
        bVar4 = CPortal_Base2D::IsActivedAndLinked(this_00);
        if ((bVar4) &&
           (pvVar1 = &(this_00->m_PortalSimulator).super_CPortalSimulator.m_InternalData.
                      super_PS_InternalData_t.Placement.vForward.z,
           *(float *)(unaff_EBX + 0x37b977) < *pvVar1 || *(float *)(unaff_EBX + 0x37b977) == *pvVar1
           )) {
          if (((this_00->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
            CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this_00);
          }
                    /* Unresolved local var: Vector res@[???] */
          local_54 = (this_00->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.x - local_48;
          local_50 = (this_00->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.y - local_44;
          local_4c = (this_00->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.z - local_40;
          if (((*(float *)(unaff_EBX + 0x37b983) <= local_4c) && (local_4c < 0.0)) &&
             (fVar6 = local_54 * local_54 + local_50 * local_50, fVar6 < local_b0)) {
            local_b4 = this_00;
            local_b0 = fVar6;
            local_a8 = local_4c;
            local_a4 = local_50;
            local_a0 = local_54;
          }
        }
        iVar5 = iVar5 + 1;
      } while (iVar5 != iVar2);
      if (local_b4 != (CPortal_Base2D *)0x0) {
        fVar6 = (float)((uint)local_a8 ^ *(uint *)(unaff_EBX + 0x38383f));
        SolveQuadratic((float)(*(uint *)(*(int *)(*(int *)(&DAT_004f48c7 + unaff_EBX) + 0x1c) + 0x2c
                                        ) ^ *(uint *)(unaff_EBX + 0x38383f)),local_28 + local_28,
                       fVar6 + fVar6,local_20,&local_24);
        local_74 = local_24;
        if (0.0 < local_20[0]) {
          local_74 = local_20[0];
        }
        if ((local_24 < local_20[0]) && (0.0 <= local_24)) {
          local_74 = (float)(~-(uint)(local_20[0] < 0.0) & (uint)local_24 |
                            (uint)local_74 & -(uint)(local_20[0] < 0.0));
        }
        if (((local_b4->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
          CBaseEntity::CalcAbsolutePosition((CBaseEntity *)local_b4);
        }
                    /* Unresolved local var: Vector res@[???] */
        local_60 = local_74 * local_30;
        local_5c = local_74 * local_2c;
        local_58 = local_74 * local_28;
                    /* Unresolved local var: Vector res@[???] */
        local_6c = local_3c + local_60;
        local_68 = local_38 + local_5c;
        local_64 = local_34 + local_58;
                    /* Unresolved local var: Vector res@[???] */
        fVar7 = (local_b4->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.x - local_6c;
        fVar6 = (local_b4->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.y - local_68;
                    /* Unresolved local var: __m128 root@[???] */
        fVar6 = SQRT(fVar7 * fVar7 + fVar6 * fVar6);
        if ((((local_b4->m_fNetworkHalfWidth).m_Value <= fVar6 &&
              fVar6 != (local_b4->m_fNetworkHalfWidth).m_Value) &&
            (fVar6 < *(float *)(*(int *)(unaff_EBX + 0x6779bb) + 0x2c))) && (0.0 < local_74)) {
                    /* Unresolved local var: Vector vVelocityToHitPortal@[???]
                       Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
          local_74 = *(float *)(unaff_EBX + 0x37a50f) / local_74;
          local_78 = local_a0 * local_74;
          local_74 = local_a4 * local_74;
          local_70 = local_28;
          (*pPhysics->_vptr_IPhysicsObject[0x33])(pPhysics,&local_78,0);
        }
      }
    }
  }
  return;
}


/* CPropExplodingFutbol::GetHolder at 00750890 */

/* DWARF original prototype: ExplodingFutbolHolderType_t GetHolder(CPropExplodingFutbol * this) */

ExplodingFutbolHolderType_t __thiscall CPropExplodingFutbol::GetHolder(CPropExplodingFutbol *this)

{
  return this->m_Holder;
}


/* CPropExplodingFutbol::SetHolder at 007508a0 */

/* DWARF original prototype: void SetHolder(CPropExplodingFutbol * this, ExplodingFutbolHolderType_t
   holder) */

void __thiscall
CPropExplodingFutbol::SetHolder(CPropExplodingFutbol *this,ExplodingFutbolHolderType_t holder)

{
  this->m_Holder = holder;
  return;
}


/* CPropExplodingFutbolSpawner::SpawnFutbol at 00751360 */

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
  this_00 = CreateEntityByName((char *)(unaff_EBX + 0x30b088),-1,true);
  if (this_00 != (CBaseEntity *)0x0) {
    COutputEvent::FireOutput(&this->m_OnFutbolSpawned,(CBaseEntity *)this,(CBaseEntity *)this,0.0);
    CBaseAnimating::GetAttachment
              ((CBaseAnimating *)this,(char *)(unaff_EBX + 0x3621a0),local_28,(Vector *)0x0,
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


/* CExplodingFutbolCatcher::CaptureFutbol at 00753700 */

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
  ushort uVar5;
  IPhysicsObject *pIVar6;
  CBaseEdict *pCVar7;
  ushort *puVar8;
  int *piVar9;
  CPropExplodingFutbolSpawner *this_00;
  ulong *puVar10;
  int iVar11;
  CBaseEntity *pCaller;
  IChangeInfoAccessor *pIVar12;
  CPropIndicatorPanel *this_01;
  uint uVar13;
  uint uVar14;
  int unaff_EBX;
  uint local_b8;
  uint local_b4;
  uint local_b0;
  
                    /* Unresolved local var: IPhysicsObject * pPhysicsObject@[???]
                       Unresolved local var: CPropExplodingFutbolSpawner * pSpawner@[???] */
  ___i686_get_pc_thunk_bx();
  if (pFutbol != (CPropExplodingFutbol *)0x0) {
    puVar10 = (ulong *)(**(code **)((int)(pFutbol->super_CPhysicsProp).super_CBreakableProp.
                                         super_CBaseProp.super_CBaseAnimating.super_CBaseEntity.
                                         super_IServerEntity.super_IServerUnknown + 0xc))(pFutbol);
    (this->m_hCaughtFutbol).super_CBaseHandle.m_Index = *puVar10;
    pIVar6 = (pFutbol->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating
             .super_CBaseEntity.m_pPhysicsObject;
    if (pIVar6 != (IPhysicsObject *)0x0) {
      (*pIVar6->_vptr_IPhysicsObject[0x10])(pIVar6,0);
    }
    (**(code **)((int)(pFutbol->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                      super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
                      super_IServerUnknown + 0x1c4))
              (pFutbol,&this->m_vecCatcherBoxOrigin,*(undefined4 *)(unaff_EBX + 0x4f34f7),
               *(undefined4 *)(unaff_EBX + 0x4f3467));
    pFutbol->m_Holder = EXPLODING_FUTBOL_HELD_BY_CATCHER;
    uVar14 = (pFutbol->m_hSpawner).super_CBaseHandle.m_Index;
    if (((uVar14 != 0xffffffff) &&
        (iVar11 = (uVar14 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f345b),
        *(uint *)(iVar11 + 8) == uVar14 >> 0x10)) &&
       (this_00 = *(CPropExplodingFutbolSpawner **)(iVar11 + 4),
       this_00 != (CPropExplodingFutbolSpawner *)0x0)) {
                    /* Unresolved local var: CPropIndicatorPanel * pTimerPanel@[???] */
      this_01 = CPropExplodingFutbolSpawner::GetTimerPanel(this_00);
      if (this_01 != (CPropIndicatorPanel *)0x0) {
        CPropIndicatorPanel::ResetTimer(this_01);
      }
    }
    pFutbol->m_bTimerActive = false;
                    /* Unresolved local var: uchar * pColorElement@[???] */
    iVar11 = *(int *)(unaff_EBX + 0x6763ab);
    uVar2 = *(uint8 *)(iVar11 + 0x32);
    uVar3 = *(uint8 *)(iVar11 + 0x31);
    uVar4 = *(uint8 *)(iVar11 + 0x30);
    if (uVar4 != (pFutbol->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                 super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                 super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r) {
      if ((pFutbol->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
          super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
        pCVar7 = &((pFutbol->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                   super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
        if ((pCVar7 != (CBaseEdict *)0x0) && ((pCVar7->m_fStateFlags & 0x100U) == 0)) {
          pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 1;
          pIVar12 = CBaseEdict::GetChangeAccessor(pCVar7);
          puVar8 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4f3457);
          if (pIVar12->m_iChangeInfoSerialNumber == *puVar8) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar5 = puVar8[uVar14 * 0x14 + 0x14];
            if (uVar5 == 0) {
LAB_00753c8a:
              puVar8[(uint)uVar5 + uVar14 * 0x14 + 1] = 0x108;
              puVar8[uVar14 * 0x14 + 0x14] = uVar5 + 1;
            }
            else if (puVar8[uVar14 * 0x14 + 1] != 0x108) {
              local_b8 = 0;
              do {
                uVar13 = local_b8 + 1;
                local_b8 = uVar13 & 0xffff;
                if ((ushort)uVar13 == uVar5) {
                  if (uVar5 != 0x13) goto LAB_00753c8a;
                  goto LAB_00753d6b;
                }
              } while (puVar8[uVar14 * 0x14 + local_b8 + 1] != 0x108);
            }
          }
          else if (puVar8[0x7d1] == 100) {
LAB_00753d6b:
            pIVar12->m_iChangeInfoSerialNumber = 0;
            pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar12->m_iChangeInfo = puVar8[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x4f3457) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x4f3457) + 0xfa2) + 1;
            piVar9 = *(int **)(unaff_EBX + 0x4f3457);
            pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar9;
            iVar11 = *piVar9 + (uint)pIVar12->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar11 + 2) = 0x108;
            *(undefined2 *)(iVar11 + 0x28) = 1;
          }
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
        if ((pCVar7 != (CBaseEdict *)0x0) && ((pCVar7->m_fStateFlags & 0x100U) == 0)) {
          pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 1;
          pIVar12 = CBaseEdict::GetChangeAccessor(pCVar7);
          puVar8 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4f3457);
          if (pIVar12->m_iChangeInfoSerialNumber == *puVar8) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar5 = puVar8[uVar14 * 0x14 + 0x14];
            if (uVar5 == 0) {
LAB_00753bc7:
              puVar8[(uint)uVar5 + uVar14 * 0x14 + 1] = 0x108;
              puVar8[uVar14 * 0x14 + 0x14] = uVar5 + 1;
            }
            else if (puVar8[uVar14 * 0x14 + 1] != 0x108) {
              local_b4 = 0;
              do {
                uVar13 = local_b4 + 1;
                local_b4 = uVar13 & 0xffff;
                if ((ushort)uVar13 == uVar5) {
                  if (uVar5 != 0x13) goto LAB_00753bc7;
                  goto LAB_00753d93;
                }
              } while (puVar8[uVar14 * 0x14 + local_b4 + 1] != 0x108);
            }
          }
          else if (puVar8[0x7d1] == 100) {
LAB_00753d93:
            pIVar12->m_iChangeInfoSerialNumber = 0;
            pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar12->m_iChangeInfo = puVar8[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x4f3457) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x4f3457) + 0xfa2) + 1;
            piVar9 = *(int **)(unaff_EBX + 0x4f3457);
            pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar9;
            iVar11 = *piVar9 + (uint)pIVar12->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar11 + 2) = 0x108;
            *(undefined2 *)(iVar11 + 0x28) = 1;
          }
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
        if ((pCVar7 != (CBaseEdict *)0x0) && ((pCVar7->m_fStateFlags & 0x100U) == 0)) {
          pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 1;
          pIVar12 = CBaseEdict::GetChangeAccessor(pCVar7);
          puVar8 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4f3457);
          if (pIVar12->m_iChangeInfoSerialNumber == *puVar8) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar5 = puVar8[uVar14 * 0x14 + 0x14];
            if (uVar5 == 0) {
LAB_00753d43:
              puVar8[(uint)uVar5 + uVar14 * 0x14 + 1] = 0x108;
              puVar8[uVar14 * 0x14 + 0x14] = uVar5 + 1;
            }
            else if (puVar8[uVar14 * 0x14 + 1] != 0x108) {
              local_b0 = 0;
              do {
                uVar13 = local_b0 + 1;
                local_b0 = uVar13 & 0xffff;
                if ((ushort)uVar13 == uVar5) {
                  if (uVar5 != 0x13) goto LAB_00753d43;
                  goto LAB_00753d7f;
                }
              } while (puVar8[uVar14 * 0x14 + local_b0 + 1] != 0x108);
            }
          }
          else if (puVar8[0x7d1] == 100) {
LAB_00753d7f:
            pIVar12->m_iChangeInfoSerialNumber = 0;
            pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar12->m_iChangeInfo = puVar8[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x4f3457) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x4f3457) + 0xfa2) + 1;
            piVar9 = *(int **)(unaff_EBX + 0x4f3457);
            pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar9;
            iVar11 = *piVar9 + (uint)pIVar12->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar11 + 2) = 0x108;
            *(undefined2 *)(iVar11 + 0x28) = 1;
          }
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
    uVar14 = (pFutbol->m_hLastHeldByPlayer).super_CBaseHandle.m_Index;
    if ((uVar14 == 0xffffffff) ||
       (iVar11 = (uVar14 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f345b),
       *(uint *)(iVar11 + 8) != uVar14 >> 0x10)) {
      pCaller = (CBaseEntity *)0x0;
    }
    else {
      pCaller = *(CBaseEntity **)(iVar11 + 4);
    }
    COutputEvent::FireOutput(&this->m_OnFutbolCaught,(CBaseEntity *)pFutbol,pCaller,0.0);
    CBaseEntity::ThinkSet((CBaseEntity *)this,(BASEPTR)0x3a9,0.0,(char *)0x0);
  }
  return;
}


/* CPropExplodingFutbol::GetLastPlayerToHold at 007508c0 */

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
     (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f62a3),
     *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) {
    return *(CPortal_Player **)(iVar2 + 4);
  }
  return (CPortal_Player *)0x0;
}


/* CPropExplodingFutbol::SetSpawner at 007512a0 */

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


/* CPropExplodingFutbol::GetSpawner at 00750910 */

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
     (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f6253),
     *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) {
    return *(CPropExplodingFutbolSpawner **)(iVar2 + 4);
  }
  return (CPropExplodingFutbolSpawner *)0x0;
}


/* CPropExplodingFutbol::InputExplode at 007523e0 */

/* DWARF original prototype: void InputExplode(CPropExplodingFutbol * this, inputdata_t * in) */

void __thiscall CPropExplodingFutbol::InputExplode(CPropExplodingFutbol *this,inputdata_t *in)

{
  KillFutbol(this);
  return;
}


/* CPropExplodingFutbol::KillFutbol at 007522d0 */

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


/* CPropExplodingFutbol::DestroyFutbol at 00753550 */

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
        (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f360b),
        *(uint *)(iVar3 + 8) == uVar1 >> 0x10)) &&
       (piVar2 = *(int **)(iVar3 + 4), piVar2 != (int *)0x0)) {
                    /* WARNING: Could not recover jumptable at 0x007535bb. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (**(code **)(*piVar2 + 0x3b0))();
      return;
    }
  }
  return;
}


/* CPropExplodingFutbol::OnFizzled at 00753660 */

/* DWARF original prototype: void OnFizzled(CPropExplodingFutbol * this) */

void __thiscall CPropExplodingFutbol::OnFizzled(CPropExplodingFutbol *this)

{
  uint uVar1;
  int *piVar2;
  int iVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::OnFizzled((CBaseAnimating *)this);
  if (*(int *)(*(int *)(unaff_EBX + 0x6766eb) + 0x30) != 0) {
    ExplodeFutbol(this);
  }
  if (this->m_bShouldRespawn != false) {
    uVar1 = (this->m_hSpawner).super_CBaseHandle.m_Index;
    if (((uVar1 != 0xffffffff) &&
        (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f34fb),
        *(uint *)(iVar3 + 8) == uVar1 >> 0x10)) &&
       (piVar2 = *(int **)(iVar3 + 4), piVar2 != (int *)0x0)) {
                    /* WARNING: Could not recover jumptable at 0x007536eb. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (**(code **)(*piVar2 + 0x3b0))();
      return;
    }
  }
  return;
}


/* CPropExplodingFutbol::Event_Killed at 007535d0 */

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
        (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f358b),
        *(uint *)(iVar3 + 8) == uVar1 >> 0x10)) &&
       (piVar2 = *(int **)(iVar3 + 4), piVar2 != (int *)0x0)) {
                    /* WARNING: Could not recover jumptable at 0x0075365b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (**(code **)(*piVar2 + 0x3b0))();
      return;
    }
  }
  return;
}


/* CPropExplodingFutbol::ExplodeFutbol at 007532f0 */

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
  flRadius = *(float *)(*(int *)(unaff_EBX + 0x67663e) + 0x2c);
  (this->m_ExplodedEntities).m_Size = 0;
  flExplosionForce = *(float *)(*(int *)(&DAT_006766fe + unaff_EBX) + 0x2c);
  iVar3 = *(int *)(*(int *)(unaff_EBX + 0x67669e) + 0x30);
  uVar1 = (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
          super_CBaseEntity.m_hOwnerEntity.
          super_CNetworkVarBase<CBaseHandle,CBaseEntity::NetworkVar_m_hOwnerEntity>.m_Value.m_Index;
  if ((uVar1 == 0xffffffff) ||
     (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f386e),
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
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4f385e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4f385e),local_30.m_Memory.m_pMemory);
    }
  }
  return;
}


/* CPropExplodingFutbol::DamageEntity at 00751db0 */

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
  local_20 = **(int **)(&DAT_004f4def + unaff_EBX);
                    /* Unresolved local var: Vector res@[???] */
  local_13c = *(float *)(unaff_EBX + 0x37ad7f);
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
    fVar3 = *(float *)(*(int *)(unaff_EBX + 0x677c3b) + 0x2c);
    fVar4 = (float)local_17c / *(float *)(*(int *)(unaff_EBX + 0x677b7b) + 0x2c);
    fVar3 = fVar3 + (fVar3 * *(float *)(*(int *)(unaff_EBX + 0x677c9b) + 0x2c) - fVar3) * fVar4;
    if (*(int *)(*(int *)(unaff_EBX + 0x677abb) + 0x30) != 0) {
      local_12c._vptr_CFmtStrN = (_func_int_varargs **)(unaff_EBX + 0x4f73c7);
      local_12c.m_bQuietTruncation = true;
      local_12c.m_szBuf[0] = '\0';
      local_12c.m_nLength = 0;
      in_stack_fffffe1c = (double)fVar3;
      CFmtStrN<256>::sprintf
                (&local_12c,(char *)(unaff_EBX + 0x361e4f),(double)(float)local_17c,(double)fVar4);
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
    goto LAB_00751fb9;
  }
  if (*(int *)(*(int *)(unaff_EBX + 0x677f3b) + 0x30) == 0) {
LAB_00752021:
    CTakeDamageInfo::CTakeDamageInfo
              (&local_1cc,(CBaseEntity *)this,(CBaseEntity *)this,
               *(float *)(*(int *)(unaff_EBX + 0x677c3b) + 0x2c),0x40,0,SUB84(in_stack_fffffe1c,0),
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
    local_17c = (char *)(unaff_EBX + 0x34c413);
    if ((pEntity->m_iClassname).pszValue != local_17c) {
      bVar1 = CBaseEntity::ClassMatchesComplex(pEntity,local_17c);
      if (!bVar1) goto LAB_00752021;
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
    local_1cc.m_vecDamageForce.z = *(float *)(*(int *)(unaff_EBX + 0x677c3b) + 0x2c);
                    /* Unresolved local var: Vector res@[???] */
    local_1cc.m_vecDamageForce.x = local_1cc.m_vecDamageForce.z * vecForward->x;
    local_1cc.m_vecDamageForce.y = local_1cc.m_vecDamageForce.z * vecForward->y;
    local_1cc.m_vecDamageForce.z = local_1cc.m_vecDamageForce.z * vecForward->z;
    local_15c = local_1cc.m_vecDamageForce.x;
    local_158 = local_1cc.m_vecDamageForce.y;
    local_154 = local_1cc.m_vecDamageForce.z;
    CBaseEntity::TakeDamage(pEntity,&local_1cc);
  }
LAB_00751fb9:
  if (local_20 == **(int **)(&DAT_004f4def + unaff_EBX)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}


/* CPropExplodingFutbol::DoExplosion at 007527f0 */

/* WARNING: Restarted to delay deadcode elimination for space: stack */
/* DWARF original prototype: void DoExplosion(CPropExplodingFutbol * this, Vector * vecCenter, float
   flExplosionRadius, CPortal_Base2D * pPortal) */

void __thiscall
CPropExplodingFutbol::DoExplosion
          (CPropExplodingFutbol *this,Vector *vecCenter,float flExplosionRadius,
          CPortal_Base2D *pPortal)

{
  undefined4 *puVar1;
  uint uVar2;
  VMatrix matThisToLinked;
  VMatrix matThisToLinked_00;
  bool bVar3;
  bool bVar4;
  int iVar5;
  undefined *puVar6;
  int iVar7;
  int iVar8;
  ITraceFilter IVar9;
  CBaseEntity *pCVar10;
  CBaseEntity **ppCVar11;
  int unaff_EBX;
  VMatrix *pVVar12;
  ShouldHitFunc_t pszClassOrWildcard;
  vec_t *pvVar13;
  bool bVar14;
  CTraceFilterSimpleList *pCVar15;
  CPortal_Base2D *pCVar16;
  CPropExplodingFutbol *pEntity;
  Ray_t *ray;
  CTraceFilterSimpleList *vecCenter_00;
  CTraceFilterSimpleList *pCVar17;
  CTraceFilterSimpleList *this_00;
  trace_t *in_stack_fffffbc4;
  undefined4 in_stack_fffffbc8;
  undefined4 in_stack_fffffbcc;
  undefined1 in_stack_fffffbd0 [36];
  Vector *pVVar18;
  Vector *ptTransformed;
  int local_3f0;
  bool bDebugging;
  int nNumFound;
  int local_3c4;
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
  bVar14 = *(int *)(*(int *)(unaff_EBX + 0x67707b) + 0x30) != 0;
  local_38.m_PassEntities.m_Memory.m_nGrowSize = 0xffff00;
  if (pPortal != (CPortal_Base2D *)0x0) {
    local_38.m_PassEntities.m_Memory.m_nAllocationCount = (int)&DAT_00ff00ff;
    local_38.m_PassEntities.m_Memory.m_nGrowSize = (int)&DAT_00ff00ff;
  }
                    /* Unresolved local var: CFlaggedEntitiesEnum sphereEnum@[???] */
  this_00 = (CTraceFilterSimpleList *)0x0;
  CFlaggedEntitiesEnum::CFlaggedEntitiesEnum((CFlaggedEntitiesEnum *)&local_ec,local_3a0,0x80,0);
  vecCenter_00 = &local_ec;
  iVar5 = UTIL_EntitiesInSphere(vecCenter,flExplosionRadius,(CFlaggedEntitiesEnum *)&local_ec);
  if (0 < iVar5) {
    local_3c4 = 0;
    IVar9._vptr_ITraceFilter = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4f43ff) + 8);
    puVar1 = *(undefined4 **)(unaff_EBX + 0x4f435b);
    do {
      pCVar10 = local_3a0[local_3c4];
      if (pCVar10 != (CBaseEntity *)0x0) {
        local_3f0 = 0;
        pszClassOrWildcard = (ShouldHitFunc_t)(unaff_EBX + 0x30e903);
        while ((local_38.super_CTraceFilterSimple.m_pExtraShouldHitCheckFunction =
                     pszClassOrWildcard,
               (ShouldHitFunc_t)(pCVar10->m_iClassname).pszValue != pszClassOrWildcard &&
               (bVar3 = CBaseEntity::ClassMatchesComplex(pCVar10,(char *)pszClassOrWildcard), !bVar3
               ))) {
          puVar6 = &UNK_005814a7 + local_3f0 * 4 + unaff_EBX;
          do {
            local_3f0 = local_3f0 + 1;
            pszClassOrWildcard = *(ShouldHitFunc_t *)(puVar6 + 4);
            if (pszClassOrWildcard == (ShouldHitFunc_t)0x0) goto LAB_00752990;
            pCVar10 = local_3a0[local_3c4];
            puVar6 = puVar6 + 4;
          } while (pCVar10 == (CBaseEntity *)0x0);
        }
        iVar8 = (this->m_ExplodedEntities).m_Size;
                    /* Unresolved local var: int j@[???] */
        if (iVar8 < 1) {
LAB_00752a6e:
          local_38.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter
               = (ITraceFilter)vecCenter->x;
          local_38.super_CTraceFilterSimple.m_pPassEnt = (IHandleEntity *)vecCenter->y;
          local_38.super_CTraceFilterSimple.m_collisionGroup = (int)vecCenter->z;
          pCVar10 = local_3a0[local_3c4];
          pCVar17 = this_00;
          if ((pCVar10->m_iEFlags & 0x800) != 0) {
            CBaseEntity::CalcAbsolutePosition(pCVar10);
            pCVar17 = this_00;
          }
          local_50._12_4_ = (pCVar10->m_vecAbsOrigin).x;
          local_40 = (IHandleEntity **)(pCVar10->m_vecAbsOrigin).y;
          local_3c = (pCVar10->m_vecAbsOrigin).z;
          this_00 = &local_98;
          CTraceFilterSimpleList::CTraceFilterSimpleList(this_00,0);
          pCVar15 = this_00;
          pEntity = this;
          CTraceFilterSimpleList::AddEntityToIgnore(this_00,(IHandleEntity *)this);
                    /* Unresolved local var: int j@[???] */
          if (0 < iVar8) {
            iVar7 = 0;
            do {
              pEntity = (CPropExplodingFutbol *)(this->m_ExplodedEntities).m_Memory.m_pMemory[iVar7]
              ;
              pCVar15 = this_00;
              CTraceFilterSimpleList::AddEntityToIgnore(this_00,(IHandleEntity *)pEntity);
              iVar7 = iVar7 + 1;
            } while (iVar7 != iVar8);
          }
          if (pPortal == (CPortal_Base2D *)0x0) {
                    /* Unresolved local var: Ray_t entRay@[???]
                       Unresolved local var: trace_t trace@[???] */
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
            in_stack_fffffbc4 = (trace_t *)local_14c;
            (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4f43cf) + 0x14))();
            if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x4f43cb) + 0x1c) + 0x30) != 0) {
              in_stack_fffffbcc = 0xbf800000;
              in_stack_fffffbc8 = 1;
              in_stack_fffffbc4 = (trace_t *)0x0;
              this_00 = (CTraceFilterSimpleList *)0x0;
              vecCenter_00 = (CTraceFilterSimpleList *)((int)&section_000000dc.addr + 3);
              DebugDrawLine((Vector *)local_14c,(Vector *)(local_14c + 0xc),0xff,0,0,true,-1.0);
            }
            bVar3 = false;
            if (local_100 != (CBaseEntity *)**(int **)(unaff_EBX + 0x4f441f)) {
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
              bVar3 = true;
            }
LAB_00752f7d:
            if (bVar14) {
                    /* Unresolved local var: Color lineColor@[???] */
              local_5c.x = (vec_t)((int)&section_000000dc.addr + 3);
              if (bVar3) {
                local_38.m_PassEntities.m_Memory.m_pMemory = (IHandleEntity **)0xff00;
                local_5c.x = 9.14768e-41;
              }
              NDebugOverlay::Sphere((Vector *)&local_38,2.0,0xff,0xff,0,false,10.0);
              NDebugOverlay::Line((Vector *)&local_38,(Vector *)(local_50 + 0xc),
                                  (uint)local_5c.x & 0xff,(uint)local_5c.x >> 8 & 0xff,
                                  (uint)local_5c.x >> 0x10 & 0xff,false,10.0);
              in_stack_fffffbcc = 0x41200000;
              in_stack_fffffbc8 = 0;
              in_stack_fffffbc4 = (trace_t *)((int)&section_000000dc.addr + 3);
              this_00 = (CTraceFilterSimpleList *)0x0;
              vecCenter_00 = (CTraceFilterSimpleList *)0x0;
              NDebugOverlay::Sphere((Vector *)(local_50 + 0xc),2.0,0,0,0xff,false,10.0);
            }
          }
          else {
            uVar2 = (pPortal->m_hLinkedPortal).
                    super_CNetworkVarBase<CBaseHandle,CPortal_Base2D::NetworkVar_m_hLinkedPortal>.
                    m_Value.m_Index;
            if ((uVar2 == 0xffffffff) ||
               (iVar8 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f436b),
               *(uint *)(iVar8 + 8) != uVar2 >> 0x10)) {
              local_3b4 = (CPortal_Base2D *)0x0;
            }
            else {
              local_3b4 = *(CPortal_Base2D **)(iVar8 + 4);
            }
            local_5c.x = (vec_t)local_50._12_4_;
            local_5c.y = (vec_t)local_40;
            local_5c.z = local_3c;
            pVVar12 = &pPortal->m_matrixThisToLinked;
            pvVar13 = (vec_t *)&stack0xfffffbb4;
            pVVar18 = &local_5c;
            ptTransformed = &local_5c;
            for (iVar8 = 0x10; iVar8 != 0; iVar8 = iVar8 + -1) {
              *pvVar13 = pVVar12->m[0][0];
              pVVar12 = (VMatrix *)(pVVar12->m[0] + 1);
              pvVar13 = pvVar13 + 1;
            }
            matThisToLinked.m[0][1] = (vec_t)pEntity;
            matThisToLinked.m[0][0] = (vec_t)pCVar15;
            matThisToLinked.m[0][2] = (vec_t)vecCenter_00;
            matThisToLinked.m[0][3] = (vec_t)pCVar17;
            matThisToLinked.m[1][0] = (vec_t)in_stack_fffffbc4;
            matThisToLinked.m[1][1] = (vec_t)in_stack_fffffbc8;
            matThisToLinked.m[1][2] = (vec_t)in_stack_fffffbcc;
            matThisToLinked.m[1][3] = (vec_t)in_stack_fffffbd0._0_4_;
            matThisToLinked.m[2][0] = (vec_t)in_stack_fffffbd0._4_4_;
            matThisToLinked.m[2][1] = (vec_t)in_stack_fffffbd0._8_4_;
            matThisToLinked.m[2][2] = (vec_t)in_stack_fffffbd0._12_4_;
            matThisToLinked.m[2][3] = (vec_t)in_stack_fffffbd0._16_4_;
            matThisToLinked.m[3][0] = (vec_t)in_stack_fffffbd0._20_4_;
            matThisToLinked.m[3][1] = (vec_t)in_stack_fffffbd0._24_4_;
            matThisToLinked.m[3][2] = (vec_t)in_stack_fffffbd0._28_4_;
            matThisToLinked.m[3][3] = (vec_t)in_stack_fffffbd0._32_4_;
            UTIL_Portal_PointTransform(matThisToLinked,pVVar18,ptTransformed);
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
            in_stack_fffffbc8 = 1;
            in_stack_fffffbc4 = &local_1a0;
            vecCenter_00 = (CTraceFilterSimpleList *)0x46004003;
            ray = (Ray_t *)local_14c;
            pCVar16 = local_3b4;
            bVar4 = UTIL_Portal_TraceRay_Bullets
                              (local_3b4,ray,0x46004003,(ITraceFilter *)this_00,in_stack_fffffbc4,
                               true);
            bVar3 = false;
            if (bVar4) {
                    /* Unresolved local var: Vector vecForward@[???] */
              pCVar17 = vecCenter_00;
              if (((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                   super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
                pCVar16 = (CPortal_Base2D *)this;
                CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
                pCVar17 = vecCenter_00;
              }
              vecCenter_00 = &local_38;
              pVVar18 = &(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                         super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin;
              pVVar12 = &local_3b4->m_matrixThisToLinked;
              pvVar13 = (vec_t *)&stack0xfffffbb4;
              pCVar15 = vecCenter_00;
              for (iVar8 = 0x10; iVar8 != 0; iVar8 = iVar8 + -1) {
                *pvVar13 = pVVar12->m[0][0];
                pVVar12 = (VMatrix *)(pVVar12->m[0] + 1);
                pvVar13 = pvVar13 + 1;
              }
              matThisToLinked_00.m[0][1] = (vec_t)ray;
              matThisToLinked_00.m[0][0] = (vec_t)pCVar16;
              matThisToLinked_00.m[0][2] = (vec_t)pCVar17;
              matThisToLinked_00.m[0][3] = (vec_t)this_00;
              matThisToLinked_00.m[1][0] = (vec_t)in_stack_fffffbc4;
              matThisToLinked_00.m[1][1] = (vec_t)in_stack_fffffbc8;
              matThisToLinked_00.m[1][2] = (vec_t)in_stack_fffffbcc;
              matThisToLinked_00.m[1][3] = (vec_t)in_stack_fffffbd0._0_4_;
              matThisToLinked_00.m[2][0] = (vec_t)in_stack_fffffbd0._4_4_;
              matThisToLinked_00.m[2][1] = (vec_t)in_stack_fffffbd0._8_4_;
              matThisToLinked_00.m[2][2] = (vec_t)in_stack_fffffbd0._12_4_;
              matThisToLinked_00.m[2][3] = (vec_t)in_stack_fffffbd0._16_4_;
              matThisToLinked_00.m[3][0] = (vec_t)in_stack_fffffbd0._20_4_;
              matThisToLinked_00.m[3][1] = (vec_t)in_stack_fffffbd0._24_4_;
              matThisToLinked_00.m[3][2] = (vec_t)in_stack_fffffbd0._28_4_;
              matThisToLinked_00.m[3][3] = (vec_t)in_stack_fffffbd0._32_4_;
              UTIL_Portal_PointTransform(matThisToLinked_00,pVVar18,(Vector *)pCVar15);
              pCVar10 = local_3a0[local_3c4];
              if ((pCVar10->m_iEFlags & 0x800) != 0) {
                CBaseEntity::CalcAbsolutePosition(pCVar10);
              }
                    /* Unresolved local var: Vector res@[???] */
              local_ec.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter.
              _vptr_ITraceFilter =
                   (ITraceFilter)
                   ((pCVar10->m_vecAbsOrigin).x -
                   (float)local_38.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter.
                          _vptr_ITraceFilter);
              local_ec.super_CTraceFilterSimple.m_pPassEnt =
                   (IHandleEntity *)
                   ((pCVar10->m_vecAbsOrigin).y -
                   (float)local_38.super_CTraceFilterSimple.m_pPassEnt);
              local_ec.super_CTraceFilterSimple.m_collisionGroup =
                   (int)((pCVar10->m_vecAbsOrigin).z -
                        (float)local_38.super_CTraceFilterSimple.m_collisionGroup);
              local_68 = (_func_int_varargs **)
                         local_ec.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter.
                         _vptr_ITraceFilter;
              local_64 = local_ec.super_CTraceFilterSimple.m_pPassEnt;
              local_60 = (IHandleEntity **)local_ec.super_CTraceFilterSimple.m_collisionGroup;
              VectorNormalize((Vector *)&local_ec);
              this_00 = &local_ec;
              DamageEntity(this,local_3a0[local_3c4],(Vector *)vecCenter_00,(Vector *)&local_ec);
              bVar3 = true;
            }
            if (bVar14) {
              NDebugOverlay::Sphere(&local_5c,5.0,0x80,0xff,0x80,false,10.0);
              if (((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                   super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
                CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
              }
              in_stack_fffffbcc = 0x41200000;
              in_stack_fffffbc8 = 0;
              in_stack_fffffbc4 = (trace_t *)((int)&section_000000dc.addr + 3);
              this_00 = (CTraceFilterSimpleList *)0x0;
              vecCenter_00 = (CTraceFilterSimpleList *)0x0;
              NDebugOverlay::Line(&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                                   super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin,&local_5c,0
                                  ,0,0xff,false,10.0);
              goto LAB_00752f7d;
            }
          }
          uVar2 = (this->m_ExplodedEntities).m_Size;
          CUtlVector<CBaseEntity*,CUtlMemory<CBaseEntity*,_int>_>::GrowVector
                    (&this->m_ExplodedEntities,1);
          iVar8 = ~uVar2 + (this->m_ExplodedEntities).m_Size;
          if (0 < iVar8) {
            ppCVar11 = (this->m_ExplodedEntities).m_Memory.m_pMemory;
            vecCenter_00 = (CTraceFilterSimpleList *)(iVar8 * 4);
            _V_memmove(ppCVar11 + uVar2 + 1,ppCVar11 + uVar2,(int)vecCenter_00);
          }
          ppCVar11 = (this->m_ExplodedEntities).m_Memory.m_pMemory + uVar2;
          if (ppCVar11 != (CBaseEntity **)0x0) {
            *ppCVar11 = local_3a0[local_3c4];
          }
          local_98.m_PassEntities.m_Size = 0;
          local_98.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter
               = (ITraceFilter)(ITraceFilter)IVar9._vptr_ITraceFilter;
          if (-1 < local_98.m_PassEntities.m_Memory.m_nGrowSize) {
            if (local_98.m_PassEntities.m_Memory.m_pMemory != (IHandleEntity **)0x0) {
              (**(code **)(*(int *)*puVar1 + 8))();
              local_98.m_PassEntities.m_Memory.m_pMemory = (IHandleEntity **)0x0;
            }
            local_98.m_PassEntities.m_Memory.m_nAllocationCount = 0;
          }
          local_98.m_PassEntities.m_pElements = local_98.m_PassEntities.m_Memory.m_pMemory;
          if (-1 < local_98.m_PassEntities.m_Memory.m_nGrowSize) {
            if (local_98.m_PassEntities.m_Memory.m_pMemory != (IHandleEntity **)0x0) {
              (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4f435b) + 8))();
              local_98.m_PassEntities.m_Memory.m_pMemory = (IHandleEntity **)0x0;
            }
            local_98.m_PassEntities.m_Memory.m_nAllocationCount = 0;
          }
        }
        else {
          ppCVar11 = (this->m_ExplodedEntities).m_Memory.m_pMemory;
          iVar7 = 0;
          pCVar10 = *ppCVar11;
          while (pCVar10 != local_3a0[local_3c4]) {
            iVar7 = iVar7 + 1;
            if (iVar7 == iVar8) goto LAB_00752a6e;
            pCVar10 = ppCVar11[iVar7];
          }
        }
      }
LAB_00752990:
      local_3c4 = local_3c4 + 1;
    } while (local_3c4 != iVar5);
  }
  if (bVar14) {
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


/* CPropExplodingFutbol::ActivateFutbolTimer at 00755170 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ActivateFutbolTimer(CPropExplodingFutbol * this, float flTimer) */

void __thiscall CPropExplodingFutbol::ActivateFutbolTimer(CPropExplodingFutbol *this,float flTimer)

{
  uint *puVar1;
  uint8 uVar2;
  uint8 uVar3;
  uint8 uVar4;
  ushort uVar5;
  int *piVar6;
  int iVar7;
  CBaseEdict *pCVar8;
  ushort *puVar9;
  BASEPTR func;
  IChangeInfoAccessor *pIVar10;
  uint uVar11;
  uint uVar12;
  int unaff_EBX;
  uint local_b8;
  uint local_b4;
  uint local_b0;
  
  ___i686_get_pc_thunk_bx();
  this->m_flTotalTimer = flTimer;
  this->m_flExplosionTimer = flTimer;
  if (*(float *)(unaff_EBX + 0x3779ef) <= flTimer && flTimer != *(float *)(unaff_EBX + 0x3779ef)) {
    this->m_bTimerActive = true;
    piVar6 = *(int **)(unaff_EBX + 0x4f19e3);
    this->m_flLastTickTime = *(float *)(*piVar6 + 0xc);
    this->m_flLastTimerSoundTime = *(float *)(*piVar6 + 0xc) - *(float *)(unaff_EBX + 0x37778f);
    this->m_flLastFlashTime = *(float *)(*piVar6 + 0xc);
                    /* Unresolved local var: uchar * pColorElement@[???] */
    iVar7 = *(int *)(unaff_EBX + 0x67493b);
    uVar2 = *(uint8 *)(iVar7 + 0x32);
    uVar3 = *(uint8 *)(iVar7 + 0x31);
    uVar4 = *(uint8 *)(iVar7 + 0x30);
    if (uVar4 != (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                 super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                 super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r) {
      if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                          super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
          false) {
        pCVar8 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                   super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
        if ((pCVar8 != (CBaseEdict *)0x0) && ((pCVar8->m_fStateFlags & 0x100U) == 0)) {
          pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 1;
          pIVar10 = CBaseEdict::GetChangeAccessor(pCVar8);
          puVar9 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4f19e7);
          if (pIVar10->m_iChangeInfoSerialNumber == *puVar9) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            uVar12 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar5 = puVar9[uVar12 * 0x14 + 0x14];
            if (uVar5 == 0) {
LAB_007555ba:
              puVar9[(uint)uVar5 + uVar12 * 0x14 + 1] = 0x108;
              puVar9[uVar12 * 0x14 + 0x14] = uVar5 + 1;
            }
            else if (puVar9[uVar12 * 0x14 + 1] != 0x108) {
              local_b8 = 0;
              do {
                uVar11 = local_b8 + 1;
                local_b8 = uVar11 & 0xffff;
                if ((ushort)uVar11 == uVar5) {
                  if (uVar5 != 0x13) goto LAB_007555ba;
                  goto LAB_00755509;
                }
              } while (puVar9[uVar12 * 0x14 + local_b8 + 1] != 0x108);
            }
          }
          else if (puVar9[0x7d1] == 100) {
LAB_00755509:
            pIVar10->m_iChangeInfoSerialNumber = 0;
            pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar10->m_iChangeInfo = puVar9[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x4f19e7) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x4f19e7) + 0xfa2) + 1;
            piVar6 = *(int **)(unaff_EBX + 0x4f19e7);
            pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
            iVar7 = *piVar6 + (uint)pIVar10->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar7 + 2) = 0x108;
            *(undefined2 *)(iVar7 + 0x28) = 1;
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
      super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r = uVar4;
    }
    if (uVar3 != *(uint8 *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                                  super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                                  super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>
                           + 1)) {
      if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                          super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
          false) {
        pCVar8 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                   super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
        if ((pCVar8 != (CBaseEdict *)0x0) && ((pCVar8->m_fStateFlags & 0x100U) == 0)) {
          pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 1;
          pIVar10 = CBaseEdict::GetChangeAccessor(pCVar8);
          puVar9 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4f19e7);
          if (pIVar10->m_iChangeInfoSerialNumber == *puVar9) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            uVar12 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar5 = puVar9[uVar12 * 0x14 + 0x14];
            if (uVar5 == 0) {
LAB_00755737:
              puVar9[(uint)uVar5 + uVar12 * 0x14 + 1] = 0x108;
              puVar9[uVar12 * 0x14 + 0x14] = uVar5 + 1;
            }
            else if (puVar9[uVar12 * 0x14 + 1] != 0x108) {
              local_b4 = 0;
              do {
                uVar11 = local_b4 + 1;
                local_b4 = uVar11 & 0xffff;
                if ((ushort)uVar11 == uVar5) {
                  if (uVar5 != 0x13) goto LAB_00755737;
                  goto LAB_007554f5;
                }
              } while (puVar9[uVar12 * 0x14 + local_b4 + 1] != 0x108);
            }
          }
          else if (puVar9[0x7d1] == 100) {
LAB_007554f5:
            pIVar10->m_iChangeInfoSerialNumber = 0;
            pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar10->m_iChangeInfo = puVar9[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x4f19e7) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x4f19e7) + 0xfa2) + 1;
            piVar6 = *(int **)(unaff_EBX + 0x4f19e7);
            pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
            iVar7 = *piVar6 + (uint)pIVar10->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar7 + 2) = 0x108;
            *(undefined2 *)(iVar7 + 0x28) = 1;
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
           uVar3;
    }
    if (uVar2 != *(uint8 *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                                  super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                                  super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>
                           + 2)) {
      if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                          super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
          false) {
        pCVar8 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                   super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
        if ((pCVar8 != (CBaseEdict *)0x0) && ((pCVar8->m_fStateFlags & 0x100U) == 0)) {
          pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 1;
          pIVar10 = CBaseEdict::GetChangeAccessor(pCVar8);
          puVar9 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4f19e7);
          if (pIVar10->m_iChangeInfoSerialNumber == *puVar9) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            uVar12 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar5 = puVar9[uVar12 * 0x14 + 0x14];
            if (uVar5 == 0) {
LAB_00755677:
              puVar9[(uint)uVar5 + uVar12 * 0x14 + 1] = 0x108;
              puVar9[uVar12 * 0x14 + 0x14] = uVar5 + 1;
            }
            else if (puVar9[uVar12 * 0x14 + 1] != 0x108) {
              local_b0 = 0;
              do {
                uVar11 = local_b0 + 1;
                local_b0 = uVar11 & 0xffff;
                if ((ushort)uVar11 == uVar5) {
                  if (uVar5 != 0x13) goto LAB_00755677;
                  goto LAB_007554e1;
                }
              } while (puVar9[uVar12 * 0x14 + local_b0 + 1] != 0x108);
            }
          }
          else if (puVar9[0x7d1] == 100) {
LAB_007554e1:
            pIVar10->m_iChangeInfoSerialNumber = 0;
            pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar10->m_iChangeInfo = puVar9[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x4f19e7) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x4f19e7) + 0xfa2) + 1;
            piVar6 = *(int **)(unaff_EBX + 0x4f19e7);
            pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
            iVar7 = *piVar6 + (uint)pIVar10->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar7 + 2) = 0x108;
            *(undefined2 *)(iVar7 + 0x28) = 1;
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
           uVar2;
    }
    func.__delta = 0;
    func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0xe11);
    CBaseEntity::ThinkSet
              ((CBaseEntity *)this,func,
               *(float *)(**(int **)(unaff_EBX + 0x4f19e3) + 0xc) +
               *(float *)(**(int **)(unaff_EBX + 0x4f19e3) + 0x1c),*(char **)(unaff_EBX + 0x6133c3))
    ;
  }
  return;
}


/* CPropExplodingFutbol::StopFutbolTimer at 00753e10 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void StopFutbolTimer(CPropExplodingFutbol * this) */

void __thiscall CPropExplodingFutbol::StopFutbolTimer(CPropExplodingFutbol *this)

{
  uint *puVar1;
  uint8 uVar2;
  uint8 uVar3;
  uint8 uVar4;
  ushort uVar5;
  int iVar6;
  CBaseEdict *pCVar7;
  ushort *puVar8;
  int *piVar9;
  IChangeInfoAccessor *pIVar10;
  uint uVar11;
  uint uVar12;
  int unaff_EBX;
  uint local_a8;
  uint local_a4;
  uint local_a0;
  
                    /* Unresolved local var: Color futbolColor@[???] */
  ___i686_get_pc_thunk_bx();
  this->m_bTimerActive = false;
                    /* Unresolved local var: uchar * pColorElement@[???] */
  iVar6 = *(int *)(unaff_EBX + 0x675c9b);
  uVar2 = *(uint8 *)(iVar6 + 0x32);
  uVar3 = *(uint8 *)(iVar6 + 0x31);
  uVar4 = *(uint8 *)(iVar6 + 0x30);
  if (uVar4 != (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
               super_CBaseEntity.m_clrRender.
               super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r) {
    if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
        false) {
      pCVar7 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                 super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar7 != (CBaseEdict *)0x0) && ((pCVar7->m_fStateFlags & 0x100U) == 0)) {
        pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar7);
        puVar8 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4f2d47);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar8) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar12 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar5 = puVar8[uVar12 * 0x14 + 0x14];
          if (uVar5 == 0) {
LAB_00754330:
            puVar8[(uint)uVar5 + uVar12 * 0x14 + 1] = 0x108;
            puVar8[uVar12 * 0x14 + 0x14] = uVar5 + 1;
          }
          else if (puVar8[uVar12 * 0x14 + 1] != 0x108) {
            local_a8 = 0;
            do {
              uVar11 = local_a8 + 1;
              local_a8 = uVar11 & 0xffff;
              if ((ushort)uVar11 == uVar5) {
                if (uVar5 != 0x13) goto LAB_00754330;
                pIVar10->m_iChangeInfoSerialNumber = 0;
                pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar8[uVar12 * 0x14 + local_a8 + 1] != 0x108);
          }
        }
        else if (puVar8[0x7d1] == 100) {
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar8[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4f2d47) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4f2d47) + 0xfa2) + 1;
          piVar9 = *(int **)(unaff_EBX + 0x4f2d47);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar9;
          iVar6 = *piVar9 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar6 + 2) = 0x108;
          *(undefined2 *)(iVar6 + 0x28) = 1;
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
      if ((pCVar7 != (CBaseEdict *)0x0) && ((pCVar7->m_fStateFlags & 0x100U) == 0)) {
        pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar7);
        puVar8 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4f2d47);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar8) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar12 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar5 = puVar8[uVar12 * 0x14 + 0x14];
          if (uVar5 == 0) {
LAB_00754284:
            puVar8[(uint)uVar5 + uVar12 * 0x14 + 1] = 0x108;
            puVar8[uVar12 * 0x14 + 0x14] = uVar5 + 1;
          }
          else if (puVar8[uVar12 * 0x14 + 1] != 0x108) {
            local_a4 = 0;
            do {
              uVar11 = local_a4 + 1;
              local_a4 = uVar11 & 0xffff;
              if ((ushort)uVar11 == uVar5) {
                if (uVar5 == 0x13) goto LAB_00754110;
                goto LAB_00754284;
              }
            } while (puVar8[uVar12 * 0x14 + local_a4 + 1] != 0x108);
          }
        }
        else if (puVar8[0x7d1] == 100) {
LAB_00754110:
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar8[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4f2d47) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4f2d47) + 0xfa2) + 1;
          piVar9 = *(int **)(unaff_EBX + 0x4f2d47);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar9;
          iVar6 = *piVar9 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar6 + 2) = 0x108;
          *(undefined2 *)(iVar6 + 0x28) = 1;
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
      if ((pCVar7 != (CBaseEdict *)0x0) && ((pCVar7->m_fStateFlags & 0x100U) == 0)) {
        pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar7);
        puVar8 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4f2d47);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar8) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar12 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar5 = puVar8[uVar12 * 0x14 + 0x14];
          if (uVar5 == 0) {
LAB_007541c0:
            puVar8[(uint)uVar5 + uVar12 * 0x14 + 1] = 0x108;
            puVar8[uVar12 * 0x14 + 0x14] = uVar5 + 1;
          }
          else if (puVar8[uVar12 * 0x14 + 1] != 0x108) {
            local_a0 = 0;
            do {
              uVar11 = local_a0 + 1;
              local_a0 = uVar11 & 0xffff;
              if ((ushort)uVar11 == uVar5) {
                if (uVar5 == 0x13) goto LAB_007541e8;
                goto LAB_007541c0;
              }
            } while (puVar8[uVar12 * 0x14 + local_a0 + 1] != 0x108);
          }
        }
        else if (puVar8[0x7d1] == 100) {
LAB_007541e8:
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar8[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4f2d47) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4f2d47) + 0xfa2) + 1;
          piVar9 = *(int **)(unaff_EBX + 0x4f2d47);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar9;
          iVar6 = *piVar9 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar6 + 2) = 0x108;
          *(undefined2 *)(iVar6 + 0x28) = 1;
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
         uVar2;
  }
  return;
}


/* CPropExplodingFutbol::TimerThink at 00754370 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void TimerThink(CPropExplodingFutbol * this) */

void __thiscall CPropExplodingFutbol::TimerThink(CPropExplodingFutbol *this)

{
  float *pfVar1;
  uint *puVar2;
  uint uVar3;
  float fVar4;
  uint8 uVar5;
  uint8 uVar6;
  ushort uVar7;
  int *piVar8;
  CBaseEdict *pCVar9;
  ushort *puVar10;
  uint3 uVar11;
  uint3 uVar12;
  uint8 uVar13;
  char *soundname;
  IChangeInfoAccessor *pIVar14;
  uint uVar15;
  uint uVar16;
  int unaff_EBX;
  int iVar17;
  float fVar18;
  float fVar19;
  uint8 local_d4;
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
    piVar8 = *(int **)(unaff_EBX + 0x4f27e3);
    fVar18 = *(float *)(*piVar8 + 0xc);
    fVar4 = this->m_flLastTickTime;
    this->m_flLastTickTime = fVar18;
    fVar18 = this->m_flExplosionTimer - (fVar18 - fVar4);
    this->m_flExplosionTimer = fVar18;
    local_80 = (char *)(unaff_EBX + 0x35f5d3);
    pfVar1 = (float *)(*(int *)(unaff_EBX + 0x6758bb) + 0x2c);
    if (fVar18 < *pfVar1 || fVar18 == *pfVar1) {
      local_80 = (char *)(unaff_EBX + 0x35f7bf);
                    /* Unresolved local var: float flCurrentFlashDuration@[???] */
      iVar17 = *(int *)(&DAT_0067591b + unaff_EBX);
      fVar18 = *(float *)(iVar17 + 0x2c);
      fVar4 = *(float *)(*piVar8 + 0xc);
      fVar19 = fVar4 - this->m_flLastFlashTime;
      if (fVar18 < fVar19) {
        this->m_flLastFlashTime = fVar4;
        fVar19 = 0.0;
        iVar17 = *(int *)(&DAT_0067591b + unaff_EBX);
      }
                    /* Unresolved local var: uchar * pColorElement@[???] */
      uVar11 = *(uint3 *)(*(int *)(unaff_EBX + 0x6757fb) + 0x30);
                    /* Unresolved local var: uchar * pColorElement@[???] */
      uVar12 = *(uint3 *)(*(int *)(unaff_EBX + 0x67585b) + 0x30);
      fVar19 = fVar19 / *(float *)(iVar17 + 0x2c);
    }
    else {
                    /* Unresolved local var: uchar * pColorElement@[???] */
      uVar11 = *(uint3 *)(*(int *)(unaff_EBX + 0x67573b) + 0x30);
                    /* Unresolved local var: uchar * pColorElement@[???] */
      uVar12 = *(uint3 *)(*(int *)(unaff_EBX + 0x67579b) + 0x30);
      fVar19 = (this->m_flTotalTimer - this->m_flExplosionTimer) / this->m_flTotalTimer;
      fVar18 = *(float *)(unaff_EBX + 0x37858f);
    }
    local_20 = (uint)uVar11;
    local_24 = (uint)uVar12;
    if (fVar18 < *(float *)(**(int **)(unaff_EBX + 0x4f27e3) + 0xc) - this->m_flLastTimerSoundTime)
    {
      soundname = (char *)(unaff_EBX + 0x2fa9a3);
      if (local_80 != (char *)0x0) {
        soundname = local_80;
      }
      CBaseEntity::EmitSound((CBaseEntity *)this,soundname,0.0,(float *)0x0);
      this->m_flLastTimerSoundTime = *(float *)(**(int **)(unaff_EBX + 0x4f27e3) + 0xc);
    }
                    /* Unresolved local var: float r@[???]
                       Unresolved local var: float g@[???]
                       Unresolved local var: float b@[???]
                       Unresolved local var: float a@[???] */
    uVar15 = uVar11 >> 8 & 0xff;
    uVar13 = (uint8)(int)(float)(int)((float)(local_20 & 0xff) +
                                     (float)(int)((local_24 & 0xff) - (local_20 & 0xff)) * fVar19);
    local_d4 = (uint8)(int)(float)(int)((float)uVar15 +
                                       (float)(int)((uVar12 >> 8 & 0xff) - uVar15) * fVar19);
    uStack_26 = (uint8)(int)(float)(int)((float)(uVar11 >> 0x10) +
                                        (float)(int)((uint)(uVar12 >> 0x10) - (uint)(uVar11 >> 0x10)
                                                    ) * fVar19);
    if (uVar13 != (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                  super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                  super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r) {
      if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                          super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
          false) {
        pCVar9 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                   super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
        if ((pCVar9 != (CBaseEdict *)0x0) && ((pCVar9->m_fStateFlags & 0x100U) == 0)) {
          pCVar9->m_fStateFlags = pCVar9->m_fStateFlags | 1;
          pIVar14 = CBaseEdict::GetChangeAccessor(pCVar9);
          piVar8 = *(int **)(unaff_EBX + 0x4f27e7);
          puVar10 = (ushort *)*piVar8;
          if (pIVar14->m_iChangeInfoSerialNumber == *puVar10) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            uVar15 = (uint)pIVar14->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar7 = puVar10[uVar15 * 0x14 + 0x14];
            if (uVar7 == 0) {
LAB_00754b66:
              puVar10[(uint)uVar7 + uVar15 * 0x14 + 1] = 0x108;
              puVar10[uVar15 * 0x14 + 0x14] = uVar7 + 1;
            }
            else if (puVar10[uVar15 * 0x14 + 1] != 0x108) {
              uVar16 = 0;
              do {
                uVar3 = uVar16 + 1;
                uVar16 = uVar3 & 0xffff;
                if ((ushort)uVar3 == uVar7) {
                  if (uVar7 != 0x13) goto LAB_00754b66;
                  goto LAB_00754ee5;
                }
              } while (puVar10[uVar15 * 0x14 + uVar16 + 1] != 0x108);
            }
          }
          else if (puVar10[0x7d1] == 100) {
LAB_00754ee5:
            pIVar14->m_iChangeInfoSerialNumber = 0;
            pCVar9->m_fStateFlags = pCVar9->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar14->m_iChangeInfo = puVar10[0x7d1];
            *(short *)(*piVar8 + 0xfa2) = *(short *)(*piVar8 + 0xfa2) + 1;
            pIVar14->m_iChangeInfoSerialNumber = *(ushort *)*piVar8;
            iVar17 = *piVar8 + (uint)pIVar14->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar17 + 2) = 0x108;
            *(undefined2 *)(iVar17 + 0x28) = 1;
          }
        }
      }
      else {
        puVar2 = (uint *)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                          super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
        *puVar2 = *puVar2 | 1;
      }
      (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
      super_CBaseEntity.m_clrRender.
      super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r = uVar13;
    }
    if (local_d4 !=
        *(uint8 *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                         super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                         super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender> + 1))
    {
      if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                          super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
          false) {
        pCVar9 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                   super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
        if ((pCVar9 != (CBaseEdict *)0x0) && ((pCVar9->m_fStateFlags & 0x100U) == 0)) {
          pCVar9->m_fStateFlags = pCVar9->m_fStateFlags | 1;
          pIVar14 = CBaseEdict::GetChangeAccessor(pCVar9);
          piVar8 = *(int **)(unaff_EBX + 0x4f27e7);
          puVar10 = (ushort *)*piVar8;
          if (pIVar14->m_iChangeInfoSerialNumber == *puVar10) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            uVar15 = (uint)pIVar14->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar7 = puVar10[uVar15 * 0x14 + 0x14];
            if (uVar7 == 0) {
LAB_00754c16:
              puVar10[(uint)uVar7 + uVar15 * 0x14 + 1] = 0x108;
              puVar10[uVar15 * 0x14 + 0x14] = uVar7 + 1;
            }
            else if (puVar10[uVar15 * 0x14 + 1] != 0x108) {
              uVar16 = 0;
              do {
                uVar3 = uVar16 + 1;
                uVar16 = uVar3 & 0xffff;
                if ((ushort)uVar3 == uVar7) {
                  if (uVar7 != 0x13) goto LAB_00754c16;
                  goto LAB_00754efc;
                }
              } while (puVar10[uVar15 * 0x14 + uVar16 + 1] != 0x108);
            }
          }
          else if (puVar10[0x7d1] == 100) {
LAB_00754efc:
            pIVar14->m_iChangeInfoSerialNumber = 0;
            pCVar9->m_fStateFlags = pCVar9->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar14->m_iChangeInfo = puVar10[0x7d1];
            *(short *)(*piVar8 + 0xfa2) = *(short *)(*piVar8 + 0xfa2) + 1;
            pIVar14->m_iChangeInfoSerialNumber = *(ushort *)*piVar8;
            iVar17 = *piVar8 + (uint)pIVar14->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar17 + 2) = 0x108;
            *(undefined2 *)(iVar17 + 0x28) = 1;
          }
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
           local_d4;
    }
    if (uStack_26 !=
        *(uint8 *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                         super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                         super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender> + 2))
    {
      if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                          super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
          false) {
        pCVar9 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                   super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
        if ((pCVar9 != (CBaseEdict *)0x0) && ((pCVar9->m_fStateFlags & 0x100U) == 0)) {
          pCVar9->m_fStateFlags = pCVar9->m_fStateFlags | 1;
          pIVar14 = CBaseEdict::GetChangeAccessor(pCVar9);
          piVar8 = *(int **)(unaff_EBX + 0x4f27e7);
          puVar10 = (ushort *)*piVar8;
          if (pIVar14->m_iChangeInfoSerialNumber == *puVar10) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            uVar15 = (uint)pIVar14->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar7 = puVar10[uVar15 * 0x14 + 0x14];
            if (uVar7 == 0) {
LAB_00754cc6:
              puVar10[(uint)uVar7 + uVar15 * 0x14 + 1] = 0x108;
              puVar10[uVar15 * 0x14 + 0x14] = uVar7 + 1;
            }
            else if (puVar10[uVar15 * 0x14 + 1] != 0x108) {
              uVar16 = 0;
              do {
                uVar3 = uVar16 + 1;
                uVar16 = uVar3 & 0xffff;
                if ((ushort)uVar3 == uVar7) {
                  if (uVar7 != 0x13) goto LAB_00754cc6;
                  goto LAB_00754f13;
                }
              } while (puVar10[uVar15 * 0x14 + uVar16 + 1] != 0x108);
            }
          }
          else if (puVar10[0x7d1] == 100) {
LAB_00754f13:
            pIVar14->m_iChangeInfoSerialNumber = 0;
            pCVar9->m_fStateFlags = pCVar9->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar14->m_iChangeInfo = puVar10[0x7d1];
            *(short *)(*piVar8 + 0xfa2) = *(short *)(*piVar8 + 0xfa2) + 1;
            pIVar14->m_iChangeInfoSerialNumber = *(ushort *)*piVar8;
            iVar17 = *piVar8 + (uint)pIVar14->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar17 + 2) = 0x108;
            *(undefined2 *)(iVar17 + 0x28) = 1;
          }
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
      CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x35f7e1),0.0,(float *)0x0);
      KillFutbol(this);
      this->m_bTimerActive = false;
                    /* Unresolved local var: uchar * pColorElement@[???] */
      iVar17 = *(int *)(unaff_EBX + 0x67573b);
      uVar13 = *(uint8 *)(iVar17 + 0x32);
      uVar5 = *(uint8 *)(iVar17 + 0x31);
      uVar6 = *(uint8 *)(iVar17 + 0x30);
      if (uVar6 != (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                   super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                   super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r) {
        if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                            super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
            false) {
          pCVar9 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                     super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
          if ((pCVar9 != (CBaseEdict *)0x0) && ((pCVar9->m_fStateFlags & 0x100U) == 0)) {
            pCVar9->m_fStateFlags = pCVar9->m_fStateFlags | 1;
            pIVar14 = CBaseEdict::GetChangeAccessor(pCVar9);
            piVar8 = *(int **)(unaff_EBX + 0x4f27e7);
            puVar10 = (ushort *)*piVar8;
            if (pIVar14->m_iChangeInfoSerialNumber == *puVar10) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
              uVar15 = (uint)pIVar14->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
              uVar7 = puVar10[uVar15 * 0x14 + 0x14];
              if (uVar7 == 0) {
LAB_00754fa6:
                puVar10[(uint)uVar7 + uVar15 * 0x14 + 1] = 0x108;
                puVar10[uVar15 * 0x14 + 0x14] = uVar7 + 1;
              }
              else if (puVar10[uVar15 * 0x14 + 1] != 0x108) {
                uVar16 = 0;
                do {
                  uVar3 = uVar16 + 1;
                  uVar16 = uVar3 & 0xffff;
                  if ((ushort)uVar3 == uVar7) {
                    if (uVar7 != 0x13) goto LAB_00754fa6;
                    goto LAB_00755140;
                  }
                } while (puVar10[uVar15 * 0x14 + uVar16 + 1] != 0x108);
              }
            }
            else if (puVar10[0x7d1] == 100) {
LAB_00755140:
              pIVar14->m_iChangeInfoSerialNumber = 0;
              pCVar9->m_fStateFlags = pCVar9->m_fStateFlags | 0x100;
            }
            else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
              pIVar14->m_iChangeInfo = puVar10[0x7d1];
              *(short *)(*piVar8 + 0xfa2) = *(short *)(*piVar8 + 0xfa2) + 1;
              pIVar14->m_iChangeInfoSerialNumber = *(ushort *)*piVar8;
              iVar17 = *piVar8 + (uint)pIVar14->m_iChangeInfo * 0x28;
              *(undefined2 *)(iVar17 + 2) = 0x108;
              *(undefined2 *)(iVar17 + 0x28) = 1;
            }
          }
        }
        else {
          puVar2 = (uint *)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                            super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
          *puVar2 = *puVar2 | 1;
        }
        (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
        super_CBaseEntity.m_clrRender.
        super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r = uVar6;
      }
      if (uVar5 != *(uint8 *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                                    super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                                    super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>
                             + 1)) {
        if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                            super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
            false) {
          pCVar9 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                     super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
          if ((pCVar9 != (CBaseEdict *)0x0) && ((pCVar9->m_fStateFlags & 0x100U) == 0)) {
            pCVar9->m_fStateFlags = pCVar9->m_fStateFlags | 1;
            pIVar14 = CBaseEdict::GetChangeAccessor(pCVar9);
            piVar8 = *(int **)(unaff_EBX + 0x4f27e7);
            puVar10 = (ushort *)*piVar8;
            if (pIVar14->m_iChangeInfoSerialNumber == *puVar10) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
              uVar15 = (uint)pIVar14->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
              uVar7 = puVar10[uVar15 * 0x14 + 0x14];
              if (uVar7 == 0) {
LAB_00755056:
                puVar10[(uint)uVar7 + uVar15 * 0x14 + 1] = 0x108;
                puVar10[uVar15 * 0x14 + 0x14] = uVar7 + 1;
              }
              else if (puVar10[uVar15 * 0x14 + 1] != 0x108) {
                uVar16 = 0;
                do {
                  uVar3 = uVar16 + 1;
                  uVar16 = uVar3 & 0xffff;
                  if ((ushort)uVar3 == uVar7) {
                    if (uVar7 != 0x13) goto LAB_00755056;
                    goto LAB_00755157;
                  }
                } while (puVar10[uVar15 * 0x14 + uVar16 + 1] != 0x108);
              }
            }
            else if (puVar10[0x7d1] == 100) {
LAB_00755157:
              pIVar14->m_iChangeInfoSerialNumber = 0;
              pCVar9->m_fStateFlags = pCVar9->m_fStateFlags | 0x100;
            }
            else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
              pIVar14->m_iChangeInfo = puVar10[0x7d1];
              *(short *)(*piVar8 + 0xfa2) = *(short *)(*piVar8 + 0xfa2) + 1;
              pIVar14->m_iChangeInfoSerialNumber = *(ushort *)*piVar8;
              iVar17 = *piVar8 + (uint)pIVar14->m_iChangeInfo * 0x28;
              *(undefined2 *)(iVar17 + 2) = 0x108;
              *(undefined2 *)(iVar17 + 0x28) = 1;
            }
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
             uVar5;
      }
      if (uVar13 != *(uint8 *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp
                                     .super_CBaseAnimating.super_CBaseEntity.m_clrRender.
                                     super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>
                              + 2)) {
        if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                            super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
            false) {
          pCVar9 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                     super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
          if ((pCVar9 != (CBaseEdict *)0x0) && ((pCVar9->m_fStateFlags & 0x100U) == 0)) {
            pCVar9->m_fStateFlags = pCVar9->m_fStateFlags | 1;
            pIVar14 = CBaseEdict::GetChangeAccessor(pCVar9);
            piVar8 = *(int **)(unaff_EBX + 0x4f27e7);
            puVar10 = (ushort *)*piVar8;
            if (pIVar14->m_iChangeInfoSerialNumber == *puVar10) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
              uVar15 = (uint)pIVar14->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
              uVar7 = puVar10[uVar15 * 0x14 + 0x14];
              if (uVar7 == 0) {
LAB_00755102:
                puVar10[(uint)uVar7 + uVar15 * 0x14 + 1] = 0x108;
                puVar10[uVar15 * 0x14 + 0x14] = uVar7 + 1;
              }
              else if (puVar10[uVar15 * 0x14 + 1] != 0x108) {
                uVar16 = 0;
                do {
                  uVar3 = uVar16 + 1;
                  uVar16 = uVar3 & 0xffff;
                  if ((ushort)uVar3 == uVar7) {
                    if (uVar7 != 0x13) goto LAB_00755102;
                    goto LAB_00755129;
                  }
                } while (puVar10[uVar15 * 0x14 + uVar16 + 1] != 0x108);
              }
            }
            else if (puVar10[0x7d1] == 100) {
LAB_00755129:
              pIVar14->m_iChangeInfoSerialNumber = 0;
              pCVar9->m_fStateFlags = pCVar9->m_fStateFlags | 0x100;
            }
            else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
              pIVar14->m_iChangeInfo = puVar10[0x7d1];
              *(short *)(*piVar8 + 0xfa2) = *(short *)(*piVar8 + 0xfa2) + 1;
              pIVar14->m_iChangeInfoSerialNumber = *(ushort *)*piVar8;
              iVar17 = *piVar8 + (uint)pIVar14->m_iChangeInfo * 0x28;
              *(undefined2 *)(iVar17 + 2) = 0x108;
              *(undefined2 *)(iVar17 + 0x28) = 1;
            }
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
             uVar13;
      }
    }
    if (this->m_bTimerActive != false) {
      CBaseEntity::SetNextThink
                ((CBaseEntity *)this,
                 *(float *)(**(int **)(unaff_EBX + 0x4f27e3) + 0xc) +
                 *(float *)(**(int **)(unaff_EBX + 0x4f27e3) + 0x1c),
                 *(char **)(unaff_EBX + 0x6141c3));
    }
  }
  return;
}


/* CPropExplodingFutbol::OnPhysGunPickup at 00751770 */

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
  local_20 = (CPortal_Player *)pPhysGunUser;
  if ((pPhysGunUser == (CBasePlayer *)0x0) ||
     (cVar5 = (**(code **)((int)(pPhysGunUser->super_CBaseCombatCharacter).super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                                super_IServerEntity.super_IServerUnknown + 0x154))(pPhysGunUser),
     cVar5 == '\0')) {
    local_20 = (CPortal_Player *)0x0;
  }
  pIVar1 = (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
           super_CBaseEntity.m_pPhysicsObject;
  if (pIVar1 != (IPhysicsObject *)0x0) {
    (*pIVar1->_vptr_IPhysicsObject[0x10])(pIVar1,1);
  }
  if ((reason == PICKED_UP_BY_PLAYER) && (local_20 != (CPortal_Player *)0x0)) {
    CPortal_Player::SetUseKeyCooldownTime
              (local_20,*(float *)(*(int *)(unaff_EBX + 0x67815e) + 0x2c));
  }
  uVar2 = (this->m_hLastHeldByPlayer).super_CBaseHandle.m_Index;
  if ((uVar2 == 0xffffffff) ||
     (iVar6 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f53ee),
     *(uint *)(iVar6 + 8) != uVar2 >> 0x10)) {
    local_21 = false;
    if (local_20 != (CPortal_Player *)0x0) goto LAB_00751800;
  }
  else {
    local_21 = false;
    if (local_20 != *(CPortal_Player **)(iVar6 + 4)) goto LAB_00751800;
  }
  local_21 = this->m_Holder == EXPLODING_FUTBOL_HELD_BY_NONE;
LAB_00751800:
  uVar2 = (this->m_hSpawner).super_CBaseHandle.m_Index;
  if (((uVar2 != 0xffffffff) &&
      (iVar6 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f53ee),
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


/* CPropExplodingFutbol::OnPhysGunDrop at 00750f30 */

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


/* CPropExplodingFutbol::OnTakeDamage at 00750ee0 */

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


/* CPropExplodingFutbol::VPhysicsCollision at 00750e50 */

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
    func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0x156c);
    CBaseEntity::ThinkSet
              ((CBaseEntity *)this,func,
               *(float *)(**(int **)(unaff_EBX + 0x4f5d00) + 0xc) +
               *(float *)(**(int **)(unaff_EBX + 0x4f5d00) + 0x1c),*(char **)(unaff_EBX + 0x6176e4))
    ;
  }
  CPhysicsProp::VPhysicsCollision(&this->super_CPhysicsProp,index,pEvent);
  return;
}


/* CPropExplodingFutbol::KillThink at 007523d0 */

/* DWARF original prototype: void KillThink(CPropExplodingFutbol * this) */

void __thiscall CPropExplodingFutbol::KillThink(CPropExplodingFutbol *this)

{
  KillFutbol(this);
  return;
}


/* CPropExplodingFutbol::AnimThink at 00750bd0 */

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
             *(float *)(**(int **)(unaff_EBX + 0x4f5f83) + 0xc) +
             *(float *)(**(int **)(unaff_EBX + 0x4f5f83) + 0x1c),*(char **)(unaff_EBX + 0x61795f));
  return;
}


/* ColorLerp at 00750960 */

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


/* CExplodingFutbolCatcher::CExplodingFutbolCatcher at 00751540 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CExplodingFutbolCatcher(CExplodingFutbolCatcher * this) */

void __thiscall CExplodingFutbolCatcher::CExplodingFutbolCatcher(CExplodingFutbolCatcher *this)

{
  vec_t *pvVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::CBaseAnimating(&this->super_CBaseAnimating);
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x58237b);
  (this->m_hCaughtFutbol).super_CBaseHandle.m_Index = 0xffffffff;
  this->m_bDisableRecaptureOnPlayerGrab = false;
  pvVar1 = *(vec_t **)(unaff_EBX + 0x4f562b);
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


/* CExplodingFutbolCatcher::CExplodingFutbolCatcher at 00751750 */

/* DWARF original prototype: void CExplodingFutbolCatcher(CExplodingFutbolCatcher * this,
   CExplodingFutbolCatcher * this) */

void __thiscall
CExplodingFutbolCatcher::CExplodingFutbolCatcher
          (CExplodingFutbolCatcher *this,CExplodingFutbolCatcher *this_1)

{
  CExplodingFutbolCatcher(this);
  return;
}


/* CEntityFactory<CExplodingFutbolCatcher>::Create at 00755c70 */

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


/* CExplodingFutbolCatcher::Spawn at 00750ca0 */

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
            (&this->super_CBaseAnimating,&UNK_00362867 + unaff_EBX,&this->m_vecCatcherBoxOrigin,
             (Vector *)0x0,(Vector *)0x0,(Vector *)0x0);
  uVar1 = *(uint *)(unaff_EBX + 0x384f93);
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
             *(float *)(**(int **)(unaff_EBX + 0x4f5eb7) + 0xc) +
             *(float *)(**(int **)(unaff_EBX + 0x4f5eb7) + 0x1c),(char *)0x0);
  return;
}


/* CExplodingFutbolCatcher::DrawDebugGeometryOverlays at 00750c30 */

/* DWARF original prototype: void DrawDebugGeometryOverlays(CExplodingFutbolCatcher * this) */

void __thiscall CExplodingFutbolCatcher::DrawDebugGeometryOverlays(CExplodingFutbolCatcher *this)

{
  CBaseEntity::DrawDebugGeometryOverlays((CBaseEntity *)this);
  NDebugOverlay::Box(&this->m_vecCatcherBoxOrigin,&this->m_vecCatcherBoxMins,
                     &this->m_vecCatcherBoxMaxs,0xff,0,0,100,0.0);
  return;
}


/* CExplodingFutbolCatcher::CaptureThink at 00751460 */

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
       (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f56f8),
       *(uint *)(iVar2 + 8) != uVar1 >> 0x10)) || (iVar2 = *(int *)(iVar2 + 4), iVar2 == 0)) ||
     (*(int *)(iVar2 + 0x768) != 3)) {
    CBaseEntity::ThinkSet((CBaseEntity *)this,(BASEPTR)0x3a5,0.0,(char *)0x0);
    COutputEvent::FireOutput(&this->m_OnFutbolReleased,(CBaseEntity *)this,(CBaseEntity *)this,0.0);
    (this->m_hCaughtFutbol).super_CBaseHandle.m_Index = 0xffffffff;
    this->m_bDisableRecaptureOnPlayerGrab = true;
  }
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(**(int **)(unaff_EBX + 0x4f56f0) + 0xc) +
             *(float *)(**(int **)(unaff_EBX + 0x4f56f0) + 0x1c),(char *)0x0);
  return;
}


/* CPropExplodingFutbolSpawner::CPropExplodingFutbolSpawner at 007516a0 */

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
       (_func_int_varargs **)(unaff_EBX + 0x581e37);
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


/* CPropExplodingFutbolSpawner::CPropExplodingFutbolSpawner at 00751740 */

/* DWARF original prototype: void CPropExplodingFutbolSpawner(CPropExplodingFutbolSpawner * this,
   CPropExplodingFutbolSpawner * this) */

void __thiscall
CPropExplodingFutbolSpawner::CPropExplodingFutbolSpawner
          (CPropExplodingFutbolSpawner *this,CPropExplodingFutbolSpawner *this_1)

{
  CPropExplodingFutbolSpawner(this);
  return;
}


/* CEntityFactory<CPropExplodingFutbolSpawner>::Create at 00755cc0 */

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


/* CPropExplodingFutbolSpawner::Precache at 00750aa0 */

/* DWARF original prototype: void Precache(CPropExplodingFutbolSpawner * this) */

void __thiscall CPropExplodingFutbolSpawner::Precache(CPropExplodingFutbolSpawner *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::Precache((CBaseAnimating *)this);
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x362a2c));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x362ebe));
  return;
}


/* CPropExplodingFutbolSpawner::Spawn at 00750dd0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CPropExplodingFutbolSpawner * this) */

void __thiscall CPropExplodingFutbolSpawner::Spawn(CPropExplodingFutbolSpawner *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity
    .super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1a])(this);
  (*(this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity
    .super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1b])
            (this,&UNK_003626f7 + unaff_EBX);
  CCollisionProperty::SetSolid
            (&(this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.
              m_Collision.super_CCollisionProperty,SOLID_VPHYSICS);
  CExplodingFutbolCatcher::Spawn(&this->super_CExplodingFutbolCatcher);
  if (this->m_bHasFutbol == false) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x00750e44. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity
    .super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xee])();
  return;
}


/* CPropExplodingFutbolSpawner::FutbolDestroyed at 00750ba0 */

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
                    /* WARNING: Could not recover jumptable at 0x00750bce. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity
    .super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xee])();
  return;
}


/* CPropExplodingFutbolSpawner::FutbolGrabbed at 00755760 */

/* DWARF original prototype: void FutbolGrabbed(CPropExplodingFutbolSpawner * this,
   CPropExplodingFutbol * pFutbol, bool bSamePlayer) */

void __thiscall
CPropExplodingFutbolSpawner::FutbolGrabbed
          (CPropExplodingFutbolSpawner *this,CPropExplodingFutbol *pFutbol,bool bSamePlayer)

{
  CPropIndicatorPanel *this_00;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x35e1f6),0.0,(float *)0x0);
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


/* CPropExplodingFutbolSpawner::GetTimerPanel at 00750ae0 */

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
                         (*(CGlobalEntityList **)(unaff_EBX + 0x4f6138),(CBaseEntity *)0x0,pcVar3,
                          (CBaseEntity *)0x0,(CBaseEntity *)0x0,(CBaseEntity *)0x0,
                          (IEntityFindFilter *)0x0), pCVar1 != (CBaseEntity *)0x0)) {
    pCVar2 = (CPropIndicatorPanel *)
             ___dynamic_cast(pCVar1,*(undefined4 *)(unaff_EBX + 0x4f60a4),
                             *(undefined4 *)(unaff_EBX + 0x4f6bdc),0);
    if (pCVar2 != (CPropIndicatorPanel *)0x0) {
      return pCVar2;
    }
    pcVar3 = CBaseEntity::GetDebugName((CBaseEntity *)0x0);
    iVar4 = section_00000054.segname._4_4_;
    if (section_00000054.segname._4_4_ == 0) {
      iVar4 = unaff_EBX + 0x2fe238;
    }
    _Warning(unaff_EBX + 0x362ffc,iVar4,pcVar3);
    return (CPropIndicatorPanel *)0x0;
  }
  return (CPropIndicatorPanel *)0x0;
}


/* CPropExplodingFutbolSpawner::InputForceSpawn at 00750a40 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void InputForceSpawn(CPropExplodingFutbolSpawner * this, inputdata_t *
   data) */

void __thiscall
CPropExplodingFutbolSpawner::InputForceSpawn(CPropExplodingFutbolSpawner *this,inputdata_t *data)

{
                    /* WARNING: Could not recover jumptable at 0x00750a4f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity
    .super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xee])();
  return;
}


/* CPropExplodingFutbolSocket::CPropExplodingFutbolSocket at 00751620 */

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
       (_func_int_varargs **)(unaff_EBX + 0x5815b7);
  return;
}


/* CPropExplodingFutbolSocket::CPropExplodingFutbolSocket at 00751660 */

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
       (_func_int_varargs **)(&UNK_00581577 + unaff_EBX);
  return;
}


/* CEntityFactory<CPropExplodingFutbolSocket>::Create at 00755d10 */

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
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x57cec4);
  CBaseEntity::PostConstructor((CBaseEntity *)this_00,pClassName);
  return &(this_00->super_CBaseAnimating).super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CPropExplodingFutbolSocket::Precache at 00750a60 */

/* DWARF original prototype: void Precache(CPropExplodingFutbolSocket * this) */

void __thiscall CPropExplodingFutbolSocket::Precache(CPropExplodingFutbolSocket *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::Precache((CBaseAnimating *)this);
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x362a4c));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x363067));
  return;
}


/* CPropExplodingFutbolSocket::Spawn at 007512f0 */

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
    .super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1b])
            (this,&UNK_003621bb + unaff_EBX);
  CCollisionProperty::SetSolid
            (&(this->super_CExplodingFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.
              m_Collision.super_CCollisionProperty,SOLID_VPHYSICS);
  nSequence = CBaseAnimating::LookupSequence((CBaseAnimating *)this,(char *)(unaff_EBX + 0x31534e));
  CBaseAnimating::SetSequence((CBaseAnimating *)this,nSequence);
  CExplodingFutbolCatcher::Spawn(&this->super_CExplodingFutbolCatcher);
  return;
}


/* CPropExplodingFutbolSocket::CaptureFutbol at 00753db0 */

/* DWARF original prototype: void CaptureFutbol(CPropExplodingFutbolSocket * this,
   CPropExplodingFutbol * pFutbol) */

void __thiscall
CPropExplodingFutbolSocket::CaptureFutbol
          (CPropExplodingFutbolSocket *this,CPropExplodingFutbol *pFutbol)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x35fd0f),0.0,(float *)0x0);
  CExplodingFutbolCatcher::CaptureFutbol(&this->super_CExplodingFutbolCatcher,pFutbol);
  return;
}


/* _GLOBAL__I_g_szExplodingFutbolAnimThinkContext at 000b4f20 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_g_szExplodingFutbolAnimThinkContext(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

