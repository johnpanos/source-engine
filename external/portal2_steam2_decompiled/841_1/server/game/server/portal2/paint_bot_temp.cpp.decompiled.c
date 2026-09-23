/* DWARF-guided pseudocode for game/server/portal2/paint_bot_temp.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000ad070 */

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
  (&UNK_0000217e)[(int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX] = 0;
  (&UNK_0000217f)[(int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX] = 0;
  _GLOBAL__I_g_pServerBenchmark
  [(int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX] = (code)0x0;
  *(undefined1 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x218].m_pEntity + unaff_EBX + 1) = 0;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x218].m_pEntity + unaff_EBX + 2) = 0;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x218].m_SerialNumber + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x218].m_pPrev + unaff_EBX + 2)
       = 0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x218].m_pNext + unaff_EBX + 2)
       = 0x7f7fffff;
  *(undefined4 *)
   (&UNK_00002196 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_0000219a + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_0000219e + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_0000219e + (int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX) = 0
  ;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x21a].m_pEntity + unaff_EBX + 2) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x21a].m_SerialNumber + unaff_EBX + 2) =
       0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x21a].m_pPrev + unaff_EBX + 2)
       = 0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x21a].m_pNext + unaff_EBX + 2)
       = 0x7f7fffff;
  *(undefined **)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x21b].m_pEntity + unaff_EBX + 2) =
       &UNK_00b9f28a + unaff_EBX;
  ConVar::ConVar((ConVar *)(unaff_EBX + 0xd1b9c2),&UNK_00a04ddc + unaff_EBX,
                 (char *)(unaff_EBX + 0x9a1ca6),0,(char *)(unaff_EBX + 0xa04dae),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  uVar1 = *(undefined4 *)(&DAT_00b99ad6 + unaff_EBX);
  ___cxa_atexit(unaff_EBX + 0x990b72,0,uVar1);
  pDefaultValue = (char *)(unaff_EBX + 0x9a1d82);
  ConVar::ConVar((ConVar *)(unaff_EBX + 0xd1ba22),(char *)(unaff_EBX + 0xa04e0c),pDefaultValue,0,
                 (char *)(unaff_EBX + 0xa04df0),in_stack_ffffffc8,in_stack_ffffffcc,
                 in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x990b52,0,uVar1);
  ConVar::ConVar((ConVar *)((int)DataMapInit<CNodeEnt>::dataDesc[0].flatOffset + unaff_EBX + -0x12),
                 (char *)(unaff_EBX + 0xa04e49),pDefaultValue,0,(char *)(unaff_EBX + 0xa04e1e),
                 in_stack_ffffffc8,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,
                 in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x990b32,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<HintNodeData>::dataDesc[0].flatOffset + unaff_EBX + -0x32),
                 (char *)(unaff_EBX + 0xa04e81),pDefaultValue,0,(char *)(unaff_EBX + 0xa04e5e),
                 in_stack_ffffffc8,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,
                 in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x990b12,0,uVar1);
  pCVar3 = (ConVar *)(unaff_EBX + 0xa04e8e);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<HintNodeData>::dataDesc[1].flatOffset + unaff_EBX + -0x12),
                 (char *)(unaff_EBX + 0xa04edd),pDefaultValue,0,(char *)pCVar3,in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x990af2,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<HintNodeData>::dataDesc[3].flatOffset + unaff_EBX + -0x32),
                 (char *)(unaff_EBX + 0xa04ee8),pDefaultValue,0x4000,pCVar3,
                 (char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x990ad2,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x21b].m_pNext + unaff_EBX + 2)
                 ,&UNK_00a04f1b + unaff_EBX,pDefaultValue,0,(char *)(unaff_EBX + 0xa04ef6),
                 in_stack_ffffffc8,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,
                 in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x990ab2,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x221].m_pNext + unaff_EBX + 2)
                 ,(char *)(unaff_EBX + 0xa04f3e),(char *)(unaff_EBX + 0xa04f3a),0,
                 (char *)(unaff_EBX + 0xa04f25),in_stack_ffffffc8,in_stack_ffffffcc,
                 in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x990a92,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<HintNodeData>::dataDesc[4].flatOffset + unaff_EBX + -0x12),
                 (char *)(unaff_EBX + 0xa04f87),(char *)(unaff_EBX + 0x9b4e3e),0x4000,
                 &UNK_00a04f56 + unaff_EBX,in_stack_ffffffc8,in_stack_ffffffcc,in_stack_ffffffd0,
                 in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x990a72,0,uVar1);
  pCVar3 = (ConVar *)(unaff_EBX + 0xa04f9e);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<HintNodeData>::dataDesc[6].flatOffset + unaff_EBX + -0x32),
                 (char *)(unaff_EBX + 0xa04fcf),(char *)(unaff_EBX + 0x9b4e3e),0x4000,(char *)pCVar3
                 ,in_stack_ffffffc8,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,
                 in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x990a52,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<HintNodeData>::dataDesc[7].flatOffset + unaff_EBX + -0x12),
                 (char *)(unaff_EBX + 0xa04fe7),pDefaultValue,0x4000,pCVar3,
                 (char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x990a32,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<HintNodeData>::dataDesc[9].flatOffset + unaff_EBX + -0x32),
                 (char *)(unaff_EBX + 0xa0501d),pDefaultValue,0x4000,(char *)(unaff_EBX + 0xa04ffa),
                 in_stack_ffffffc8,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,
                 in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x990a12,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<HintNodeData>::dataDesc[10].flatOffset + unaff_EBX + -0x12),
                 (char *)(unaff_EBX + 0xa05054),pDefaultValue,0x4000,(char *)(unaff_EBX + 0xa05026),
                 in_stack_ffffffc8,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,
                 in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x9909f2,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<HintNodeData>::dataDesc[0xc].flatOffset + unaff_EBX + -0x32),
                 (char *)(unaff_EBX + 0xa0508b),pDefaultValue,0x4000,(char *)(unaff_EBX + 0xa05066),
                 in_stack_ffffffc8,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,
                 in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x9909d2,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<HintNodeData>::dataDesc[0xd].flatOffset + unaff_EBX + -0x12),
                 (char *)(unaff_EBX + 0xa050c6),(char *)(unaff_EBX + 0xa050c0),0x4000,
                 (char *)(unaff_EBX + 0xa05096),in_stack_ffffffc8,in_stack_ffffffcc,
                 in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x9909b2,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<CAI_LookTarget>::dataDesc[0].flatOffset + unaff_EBX + -0x32),
                 (char *)(unaff_EBX + 0xa05114),pDefaultValue,0x4000,(char *)(unaff_EBX + 0xa050de),
                 in_stack_ffffffc8,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,
                 in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x990992,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<CAI_LookTarget>::dataDesc[1].flatOffset + unaff_EBX + -0x12),
                 &UNK_00a05146 + unaff_EBX,pDefaultValue,0x4000,(char *)(unaff_EBX + 0xa0511e),
                 in_stack_ffffffc8,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,
                 in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x990972,0,uVar1);
  ConCommand::ConCommand
            ((ConCommand *)
             ((int)DataMapInit<CAI_LookTarget>::dataDesc[3].flatOffset + unaff_EBX + -0x32),
             &UNK_00a05150 + unaff_EBX,(FnCommandCallback_t)(unaff_EBX + 0x682792),
             (char *)(unaff_EBX + 0x9a1ca6),0x4000,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x990952,0,uVar1);
  ConCommand::ConCommand
            ((ConCommand *)
             ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x293].m_pNext + unaff_EBX + 2),
             &UNK_00a0516c + unaff_EBX,(FnCommandCallback_t)(unaff_EBX + 0x6826a2),
             &UNK_00a05161 + unaff_EBX,0x4000,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x990932,0,uVar1);
  ConCommand::ConCommand
            ((ConCommand *)
             ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x297].m_pNext + unaff_EBX + 2),
             &UNK_00a051c6 + unaff_EBX,(FnCommandCallback_t)(unaff_EBX + 0x682bc2),
             &UNK_00a05172 + unaff_EBX,0x4000,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x990912,0,uVar1);
  ConCommand::ConCommand
            ((ConCommand *)
             ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x29b].m_pNext + unaff_EBX + 2),
             &UNK_00a051e8 + unaff_EBX,(FnCommandCallback_t)(unaff_EBX + 0x682702),
             &UNK_00a051d2 + unaff_EBX,0x4000,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x9908f2,0,uVar1);
  ConCommand::ConCommand
            ((ConCommand *)
             ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x29f].m_pNext + unaff_EBX + 2),
             &UNK_00a0520c + unaff_EBX,(FnCommandCallback_t)(unaff_EBX + 0x682462),
             &UNK_00a051f1 + unaff_EBX,0x4000,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x9908d2,0,uVar1);
  ConCommand::ConCommand
            ((ConCommand *)
             ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2a3].m_pNext + unaff_EBX + 2),
             &UNK_00a0529b + unaff_EBX,(FnCommandCallback_t)(unaff_EBX + 0x682a32),
             &UNK_00a0521a + unaff_EBX,0x4000,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x9908b2,0,uVar1);
  ConCommand::ConCommand
            ((ConCommand *)
             ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2a7].m_pNext + unaff_EBX + 2),
             (char *)(unaff_EBX + 0xa05300),(FnCommandCallback_t)(unaff_EBX + 0x6828a2),
             &UNK_00a052aa + unaff_EBX,0x4000,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x990892,0,uVar1);
  return;
}


/* __tcf_0 at 00a3dbf0 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38ae48),in_stack_00000008);
  return;
}


/* __tcf_1 at 00a3dbd0 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38aec8),in_stack_00000008);
  return;
}


/* __tcf_2 at 00a3dbb0 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38af48),in_stack_00000008);
  return;
}


/* __tcf_3 at 00a3db90 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38afc8),in_stack_00000008);
  return;
}


/* __tcf_4 at 00a3db70 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38b048),in_stack_00000008);
  return;
}


/* __tcf_5 at 00a3db50 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38b0c8),in_stack_00000008);
  return;
}


/* __tcf_6 at 00a3db30 */

void __tcf_6(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x40f2e8),in_stack_00000008);
  return;
}


/* __tcf_7 at 00a3db10 */

void __tcf_7(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x40f368),in_stack_00000008);
  return;
}


/* __tcf_8 at 00a3daf0 */

void __tcf_8(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38b188),in_stack_00000008);
  return;
}


/* __tcf_9 at 00a3dad0 */

void __tcf_9(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38b208),in_stack_00000008);
  return;
}


/* __tcf_10 at 00a3dab0 */

void __tcf_10(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38b288),in_stack_00000008);
  return;
}


/* __tcf_11 at 00a3da90 */

void __tcf_11(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38b308),in_stack_00000008);
  return;
}


/* __tcf_12 at 00a3da70 */

void __tcf_12(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(&UNK_0038b388 + extraout_ECX),in_stack_00000008);
  return;
}


/* __tcf_13 at 00a3da50 */

void __tcf_13(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(&UNK_0038b408 + extraout_ECX),in_stack_00000008);
  return;
}


/* __tcf_14 at 00a3da30 */

void __tcf_14(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(&UNK_0038b488 + extraout_ECX),in_stack_00000008);
  return;
}


/* __tcf_15 at 00a3da10 */

void __tcf_15(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38b508),in_stack_00000008);
  return;
}


/* __tcf_16 at 00a3d9f0 */

void __tcf_16(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38b588),in_stack_00000008);
  return;
}


/* cc_bot_selectweapon at 0072f810 */

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
    pcVar3 = &UNK_0031f500 + unaff_EBX;
    if (1 < args->m_nArgc) {
      pcVar3 = args->m_ppArgv[1];
    }
    _Msg(unaff_EBX + 0x382491,pcVar3);
  }
  else {
    pcVar3 = &UNK_0031f500 + unaff_EBX;
    if (2 < args->m_nArgc) {
      pcVar3 = args->m_ppArgv[2];
    }
    iVar2 = (**(code **)((int)(pCVar1->super_CBaseCombatCharacter).super_CBaseFlex.
                              super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                              super_IServerEntity.super_IServerUnknown + 0x464))(pCVar1,pcVar3,0);
    if (iVar2 == 0) {
      pcVar3 = &UNK_0031f500 + unaff_EBX;
      if (2 < args->m_nArgc) {
        pcVar3 = args->m_ppArgv[2];
      }
      pcVar4 = &UNK_0031f500 + unaff_EBX;
      if (1 < args->m_nArgc) {
        pcVar4 = args->m_ppArgv[1];
      }
      _Msg(unaff_EBX + 0x3824ef,pcVar4,pcVar3);
    }
    else {
      (**(code **)((int)(pCVar1->super_CBaseCombatCharacter).super_CBaseFlex.
                        super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                        super_IServerEntity.super_IServerUnknown + 0x47c))(pCVar1,iVar2,0);
    }
  }
  return;
}


/* __tcf_17 at 00a3d9d0 */

void __tcf_17(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x38b608),in_stack_00000008);
  return;
}


/* BotPutInServer at 0072f4f0 */

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
  local_20 = **(int **)(unaff_EBX + 0x5176b2);
  if ((pszCustomName == (char *)0x0) || (*pszCustomName == '\0')) {
    if (*(int *)(*(int *)(unaff_EBX + 0x69985e) + 0x30) == 0) {
      uVar6 = *(undefined4 *)(&DAT_006584de + unaff_EBX);
      pFormat = (char *)(unaff_EBX + 0x3826fa);
    }
    else {
      uVar5 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x517f2e) + 0x10))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x517f2e),0,5);
      if (uVar5 < 5) {
                    /* WARNING: Could not recover jumptable at 0x0072f699. Too many branches */
                    /* WARNING: Treating indirect jump as call */
        pCVar4 = (CBasePlayer *)(*(code *)(*(int *)(unaff_EBX + 0x19e + uVar5 * 4) + unaff_EBX))();
        return pCVar4;
      }
      uVar6 = *(undefined4 *)(&DAT_006584de + unaff_EBX);
      pFormat = (char *)(unaff_EBX + 0x38279f);
    }
    V_snprintf(local_60,0x40,pFormat,uVar6);
  }
  else {
    V_strncpy(local_60,pszCustomName,0x40);
  }
  iVar3 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x517682) + 0xd8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x517682),local_60);
  if (iVar3 == 0) {
    _Msg(unaff_EBX + 0x379d26);
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
    *(int *)(&DAT_006584de + unaff_EBX) = *(int *)(&DAT_006584de + unaff_EBX) + 1;
    peVar2 = (pCVar4->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
             super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev;
                    /* Unresolved local var: int edictIndex@[???] */
    iVar3 = -1;
    if (peVar2 != (edict_t *)0x0) {
      iVar3 = ((int)peVar2 - *(int *)(**(int **)(unaff_EBX + 0x517666) + 0x58) >> 4) + -1;
    }
    iVar3 = unaff_EBX + 0x71d9e2 + iVar3 * 0x34;
    *(undefined1 *)(iVar3 + 0x2d) = 0;
    *(undefined1 *)(iVar3 + 0x2c) = 0;
  }
  if (local_20 == **(int **)(unaff_EBX + 0x5176b2)) {
    return pCVar4;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}


/* bot at 0072f720 */

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
  pcVar1 = CCommand::FindArg(args,(char *)(unaff_EBX + 0x382572));
  pszCustomName = CCommand::FindArg(args,(char *)(unaff_EBX + 0x38257a));
  BotPutInServer(pcVar1 != (char *)0x0,pszCustomName);
  return;
}


/* __tcf_18 at 00a3d9b0 */

void __tcf_18(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x40fbe8),in_stack_00000008);
  return;
}


/* Bot_RunAll at 00731980 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void Bot_RunAll(void)

{
  char cVar1;
  CPortal_Player *pBot;
  int unaff_EBX;
  int playerIndex;
  
  ___i686_get_pc_thunk_bx();
  if (0 < *(int *)(**(int **)(unaff_EBX + 0x5151d6) + 0x14)) {
    playerIndex = 1;
    do {
      pBot = (CPortal_Player *)UTIL_PlayerByIndex(playerIndex);
      if (((pBot != (CPortal_Player *)0x0) &&
          (cVar1 = (**(code **)(*(int *)&(pBot->
                                         super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>
                                         ).super_CPaintableEntity<CBaseMultiplayerPlayer>.
                                         super_CBaseMultiplayerPlayer.
                                         super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.
                                         super_CBaseCombatCharacter.super_CBaseFlex + 0x154))(pBot),
          cVar1 != '\0')) &&
         ((*(byte *)((int)&(pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                           super_CPaintableEntity<CBaseMultiplayerPlayer>.
                           super_CBaseMultiplayerPlayer.super_CAI_ExpresserHost<CBasePlayer>.
                           super_CBasePlayer.super_CBaseCombatCharacter.super_CBaseFlex + 0x131) & 1
          ) != 0)) {
        Bot_Think(pBot);
      }
      playerIndex = playerIndex + 1;
    } while (playerIndex <= *(int *)(**(int **)(unaff_EBX + 0x5151d6) + 0x14));
  }
  return;
}


/* RunMimicCommand at 0072fde0 */

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
  playerIndex = *(int *)(*(int *)(unaff_EBX + 0x71d04f) + 0x30);
  if ((0 < playerIndex) && (playerIndex <= *(int *)(**(int **)(unaff_EBX + 0x516d77) + 0x14))) {
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
      (cmd->viewangles).y =
           (cmd->viewangles).y + *(float *)(*(int *)(&DAT_0071d0af + unaff_EBX) + 0x2c);
      return true;
    }
  }
  return false;
}


/* Bot_Think at 0072ff40 */

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
  uint uVar4;
  float fVar5;
  float fVar6;
  ushort uVar7;
  undefined2 uVar8;
  edict_t *peVar9;
  int *piVar10;
  code *pcVar11;
  int *piVar12;
  _func_int_varargs **pp_Var13;
  _func_int_varargs *p_Var14;
  ushort *puVar15;
  ShouldHitFunc_t p_Var16;
  char cVar17;
  bool bVar18;
  vec_t *pvVar19;
  int iVar20;
  int iVar21;
  CBasePlayer *pCVar22;
  QAngle *pQVar23;
  undefined4 uVar24;
  IMoveHelper *pIVar25;
  int iVar26;
  CBaseCombatWeapon *pCVar27;
  ShouldHitFunc_t p_Var28;
  IChangeInfoAccessor *pIVar29;
  CWeaponPaintGun *this;
  CBaseCombatWeapon *pCVar30;
  uint uVar31;
  int unaff_EBX;
  int *piVar32;
  int iVar33;
  uint uVar34;
  longdouble lVar35;
  char *pcVar36;
  double dVar37;
  double dVar38;
  float fVar39;
  float local_208;
  float local_204;
  float local_200;
  uint local_1fc;
  int local_1f4;
  int local_1c0;
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
  peVar9 = (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
           super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
           super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
           m_Network.m_pPev;
  if (peVar9 == (edict_t *)0x0) {
    local_1c0 = -1;
  }
  else {
    local_1c0 = ((int)peVar9 - *(int *)(**(int **)(unaff_EBX + 0x516c13) + 0x58) >> 4) + -1;
  }
  iVar21 = unaff_EBX + 0x71cf8f;
  iVar33 = iVar21 + local_1c0 * 0x34;
  local_204 = *(float *)(iVar33 + 0x10);
  fVar5 = *(float *)(**(int **)(unaff_EBX + 0x516c13) + 0x10);
  pvVar19 = (vec_t *)(**(code **)(*(int *)&(pBot->
                                           super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>
                                           ).super_CPaintableEntity<CBaseMultiplayerPlayer>.
                                           super_CBaseMultiplayerPlayer.
                                           super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.
                                           super_CBaseCombatCharacter.super_CBaseFlex + 0x224))();
  local_28[0].x = *pvVar19;
  local_28[0].y = pvVar19[1];
  local_28[0].z = pvVar19[2];
  piVar10 = (int *)**(undefined4 **)(unaff_EBX + 0x516c87);
  (**(code **)(*piVar10 + 0x74))();
  cVar17 = (**(code **)(*(int *)&(pBot->
                                 super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                                 super_CPaintableEntity<CBaseMultiplayerPlayer>.
                                 super_CBaseMultiplayerPlayer.super_CAI_ExpresserHost<CBasePlayer>.
                                 super_CBasePlayer.super_CBaseCombatCharacter.super_CBaseFlex +
                       0x114))();
  if (cVar17 == '\0') {
LAB_00730030:
    cVar17 = (**(code **)(*(int *)&(pBot->
                                   super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                                   super_CPaintableEntity<CBaseMultiplayerPlayer>.
                                   super_CBaseMultiplayerPlayer.super_CAI_ExpresserHost<CBasePlayer>
                                   .super_CBasePlayer.super_CBaseCombatCharacter.super_CBaseFlex +
                         0x114))();
    if (cVar17 == '\0') {
      iVar33 = iVar21 + local_1c0 * 0x34;
      if (*(char *)(iVar33 + 0x2d) == '\0') {
        *(undefined1 *)(iVar33 + 0x2d) = 1;
        *(undefined4 *)(iVar33 + 0x30) = *(undefined4 *)(**(int **)(unaff_EBX + 0x516c13) + 0xc);
      }
      else {
        fVar39 = *(float *)(**(int **)(unaff_EBX + 0x516c13) + 0xc) - *(float *)(iVar33 + 0x30);
        if (*(float *)(unaff_EBX + 0x39ca07) <= fVar39 && fVar39 != *(float *)(unaff_EBX + 0x39ca07)
           ) {
          iVar33 = *(int *)(*(int *)(unaff_EBX + 0x698c2b) + 0x30);
          local_1fc = 2;
          goto joined_r0x00730c2b;
        }
      }
    }
    iVar33 = *(int *)(*(int *)(unaff_EBX + 0x698c2b) + 0x30);
    local_1fc = 0;
joined_r0x00730c2b:
    if (iVar33 == 2) {
                    /* Unresolved local var: Vector random@[???]
                       Unresolved local var: QAngle ret@[???] */
      iVar33 = rand();
      iVar20 = rand();
      iVar26 = rand();
      fVar39 = *(float *)(unaff_EBX + 0x39ccb7);
      local_2c = *(float *)(unaff_EBX + 0x39c98f);
      local_34 = (float)iVar33 / fVar39 + (float)iVar33 / fVar39 + local_2c;
      local_30 = (float)iVar20 / fVar39 + (float)iVar20 / fVar39 + local_2c;
      local_2c = (float)iVar26 / fVar39 + (float)iVar26 / fVar39 + local_2c;
      iVar20 = local_1c0 * 0x34;
      iVar33 = iVar20 + iVar21;
      *(float *)(iVar33 + 0x20) = *(float *)(iVar33 + 0x20) + local_34;
      *(float *)(iVar33 + 0x24) = local_30 + *(float *)(iVar33 + 0x24);
      *(float *)(iVar33 + 0x28) = local_2c + *(float *)(iVar33 + 0x28);
      pQVar23 = (QAngle *)(iVar20 + 0x20 + iVar21);
      iVar33 = iVar20 + 0x10 + iVar21;
      fVar39 = *(float *)(iVar33 + 4);
      uVar34 = *(uint *)(unaff_EBX + 0x3a567f);
      fVar6 = *(float *)(unaff_EBX + 0x39e98f);
      if (fVar6 < (float)((uint)(pQVar23->x - fVar39) & uVar34)) {
        if (pQVar23->x <= fVar39) {
          pQVar23->x = fVar39 - fVar6;
        }
        else {
          pQVar23->x = fVar39 + fVar6;
        }
      }
      fVar39 = *(float *)(iVar33 + 8);
      if (fVar6 < (float)((uint)(pQVar23->y - fVar39) & uVar34)) {
        if (pQVar23->y <= fVar39) {
          pQVar23->y = fVar39 - fVar6;
        }
        else {
          pQVar23->y = fVar39 + fVar6;
        }
      }
      pQVar23->z = 0.0;
      CBaseEntity::SetLocalAngles((CBaseEntity *)pBot,pQVar23);
      iVar21 = iVar21 + local_1c0 * 0x34;
      local_28[0].x = *(vec_t *)(iVar21 + 0x20);
      local_28[0].y = *(vec_t *)(iVar21 + 0x24);
      local_28[0].z = *(vec_t *)(iVar21 + 0x28);
      local_208 = 0.0;
    }
    else {
      if (iVar33 != 3) goto LAB_0073006c;
      peVar9 = (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
               super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
               super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
               super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
               m_Network.m_pPev;
                    /* Unresolved local var: int edictIndex@[???] */
      dVar37 = 0.0;
      if (peVar9 != (edict_t *)0x0) {
        dVar37 = (double)((int)peVar9 - *(int *)(**(int **)(unaff_EBX + 0x516c13) + 0x58) >> 4);
      }
      fVar39 = AngleNormalize((float)((dVar37 + (double)*(float *)(**(int **)(unaff_EBX + 0x516c13)
                                                                  + 0xc) *
                                                *(double *)(unaff_EBX + 0x3a568f)) *
                                     *(double *)(unaff_EBX + 0x3a5697)));
      iVar33 = unaff_EBX + 0x71cfaf + local_1c0 * 0x34;
      *(float *)(iVar33 + 4) = fVar39;
      *(undefined4 *)(iVar33 + 8) = 0;
                    /* Unresolved local var: int edictIndex@[???] */
      if ((pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
          super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
          super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
          super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
          m_Network.m_pPev == (edict_t *)0x0) {
        lVar35 = (longdouble)_sin();
        dVar37 = *(double *)(unaff_EBX + 0x3a569f);
        dVar38 = (double)lVar35 * dVar37;
      }
      else {
        lVar35 = (longdouble)_sin();
        dVar37 = *(double *)(unaff_EBX + 0x3a569f);
        dVar38 = (double)lVar35 * dVar37;
      }
      local_208 = (float)dVar38;
      lVar35 = (longdouble)_cos();
      local_204 = (float)(dVar37 * (double)lVar35);
      CBaseEntity::SetLocalAngles((CBaseEntity *)pBot,(QAngle *)(local_1c0 * 0x34 + 0x20 + iVar21));
      iVar33 = local_1c0 * 0x34 + iVar21;
      local_28[0].x = *(vec_t *)(iVar33 + 0x20);
      local_28[0].y = *(vec_t *)(iVar33 + 0x24);
      local_28[0].z = *(vec_t *)(iVar33 + 0x28);
      cVar17 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5174db) + 8))();
      if (cVar17 == '\0') {
                    /* Unresolved local var: int edictIndex@[???] */
        lVar35 = (longdouble)_sin();
        *(float *)(iVar21 + local_1c0 * 0x34 + 0x20) =
             (float)((double)lVar35 * *(double *)(unaff_EBX + 0x3a4b17));
        local_1fc = local_1fc & 0xf7fe;
      }
      else {
        *(undefined4 *)(iVar33 + 0x20) = 0;
      }
    }
  }
  else {
    iVar20 = (*(pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
               super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
               super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
               super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
               m_Collision.super_CCollisionProperty.super_ICollideable._vptr_ICollideable[0xb])();
    if (iVar20 != 2) goto LAB_00730030;
    *(undefined1 *)(iVar33 + 0x2d) = 0;
    local_1fc = ~-(uint)(*(int *)(*(int *)(unaff_EBX + 0x698ecb) + 0x30) == 0) & 0x4000;
    if (*(int *)(*(int *)(&LAB_0069904b + unaff_EBX) + 0x30) != 0) {
      local_1fc = local_1fc | 0x8000;
      ConVar::SetValue((ConVar *)(unaff_EBX + 0x69902f),0);
    }
    if (((((pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
           super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
           super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
           m_iEFlags & 0x100) == 0) && (*(int *)(*(int *)(unaff_EBX + 0x698e6b) + 0x30) != 0)) &&
       (((pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
         super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
         super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
         super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.m_fFlags
         .m_Value & 1) != 0)) {
      local_1fc = local_1fc | 2;
    }
    if (-1 < *(int *)(*(int *)(unaff_EBX + 0x698d4b) + 0x30)) {
                    /* Unresolved local var: int slot@[???]
                       Unresolved local var: CBaseCombatWeapon * pWpn@[???] */
      iVar33 = (**(code **)(*(int *)&(pBot->
                                     super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>
                                     ).super_CPaintableEntity<CBaseMultiplayerPlayer>.
                                     super_CBaseMultiplayerPlayer.
                                     super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.
                                     super_CBaseCombatCharacter.super_CBaseFlex + 0x48c))();
      if (iVar33 != 0) {
        (**(code **)(*(int *)&(pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>
                              ).super_CPaintableEntity<CBaseMultiplayerPlayer>.
                              super_CBaseMultiplayerPlayer.super_CAI_ExpresserHost<CBasePlayer>.
                              super_CBasePlayer.super_CBaseCombatCharacter.super_CBaseFlex + 0x47c))
                  ();
      }
      ConVar::SetValue((ConVar *)(unaff_EBX + 0x698d2f),-1);
    }
    if (*(uint *)(*(int *)(&LAB_00698dab + unaff_EBX) + 0x30) < 5) {
                    /* Unresolved local var: CWeaponPaintGun * pPaintGun@[???] */
      pCVar27 = CBaseCombatCharacter::GetActiveWeapon((CBaseCombatCharacter *)pBot);
      if ((pCVar27 != (CBaseCombatWeapon *)0x0) &&
         (this = (CWeaponPaintGun *)___dynamic_cast(), this != (CWeaponPaintGun *)0x0)) {
        CWeaponPaintGun::SetCurrentPaint
                  (this,*(PaintPowerType *)(*(int *)(&LAB_00698dab + unaff_EBX) + 0x30));
      }
      ConVar::SetValue((ConVar *)(unaff_EBX + 0x698d8f),-1);
    }
                    /* Unresolved local var: char * str@[???] */
    pcVar36 = (char *)(unaff_EBX + 0x32789b);
    if ((*(byte *)(unaff_EBX + 0x698b04) & 0x10) == 0) {
                    /* Unresolved local var: char * str@[???] */
      pcVar36 = *(char **)(*(int *)(unaff_EBX + 0x698b0b) + 0x24);
      if (pcVar36 == (char *)0x0) {
        pcVar36 = &UNK_0031edd3 + unaff_EBX;
      }
    }
    cVar17 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5174db) + 8))();
    iVar33 = 0;
    if (cVar17 == '\0') {
      do {
        pCVar27 = CBaseCombatCharacter::GetWeapon((CBaseCombatCharacter *)pBot,iVar33);
        if (pCVar27 != (CBaseCombatWeapon *)0x0) {
          pCVar27 = CBaseCombatCharacter::GetWeapon((CBaseCombatCharacter *)pBot,iVar33);
          if (pCVar27 != (CBaseCombatWeapon *)0x0) {
            local_ec = pcVar36;
            if ((pCVar27->super_CBaseAnimating).super_CBaseEntity.m_iClassname.pszValue != pcVar36)
            {
              bVar18 = CBaseEntity::ClassMatchesComplex((CBaseEntity *)pCVar27,pcVar36);
              if (!bVar18) goto LAB_007313dd;
            }
            pCVar27 = CBaseCombatCharacter::GetWeapon((CBaseCombatCharacter *)pBot,iVar33);
            goto LAB_0073168f;
          }
        }
LAB_007313dd:
        iVar33 = iVar33 + 1;
      } while (iVar33 != 0x30);
    }
    else {
      iVar33 = unaff_EBX + 0x71cfaf + local_1c0 * 0x34;
      if (*(char *)(iVar33 + 0xc) == '\0') {
        *(undefined1 *)(iVar33 + 0xc) = 1;
        local_100.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter =
             (ITraceFilter)(_func_int_varargs **)0x0;
        local_100.m_pPassEnt = (IHandleEntity *)0x0;
        local_100.m_collisionGroup = 0;
        local_100.m_pExtraShouldHitCheckFunction = (ShouldHitFunc_t)0x0;
        local_f0._vptr_ITraceFilter = (_func_int_varargs **)0x0;
        local_1f4 = 0;
        do {
          pCVar27 = CBaseCombatCharacter::GetWeapon((CBaseCombatCharacter *)pBot,local_1f4);
          if (pCVar27 != (CBaseCombatWeapon *)0x0) {
            pCVar27 = CBaseCombatCharacter::GetWeapon((CBaseCombatCharacter *)pBot,local_1f4);
            p_Var16 = local_100.m_pExtraShouldHitCheckFunction;
            p_Var3 = local_100.m_pExtraShouldHitCheckFunction + 1;
            p_Var28 = p_Var3;
            if ((int)local_100.m_pPassEnt < (int)p_Var3) {
              CUtlMemory<CBaseCombatWeapon*,int>::Grow
                        ((CUtlMemory<CBaseCombatWeapon*,int> *)&local_100,
                         (int)p_Var3 - (int)local_100.m_pPassEnt);
              p_Var28 = local_100.m_pExtraShouldHitCheckFunction + 1;
            }
            local_f0 = local_100.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter;
            local_100.m_pExtraShouldHitCheckFunction = p_Var28;
            if (0 < (int)(p_Var28 + (-1 - (int)p_Var16))) {
              _V_memmove((_func_int_varargs **)
                         ((int)local_100.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter +
                         (int)p_Var3 * 4),
                         (_func_int_varargs **)
                         ((int)local_100.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter +
                         (int)p_Var16 * 4),(int)(p_Var28 + (-1 - (int)p_Var16)) * 4);
            }
            if ((_func_int_varargs **)
                ((int)local_100.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter +
                p_Var16 * 4) != (_func_int_varargs **)0x0) {
              *(_func_int_varargs **)
               ((int)local_100.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter +
               p_Var16 * 4) = (_func_int_varargs *)pCVar27;
            }
          }
          local_1f4 = local_1f4 + 1;
        } while (local_1f4 != 0x30);
        if ((int)local_100.m_pExtraShouldHitCheckFunction < 1) {
          pCVar27 = (CBaseCombatWeapon *)0x0;
        }
        else {
          iVar33 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5174db) + 0x10))();
          pCVar27 = *(CBaseCombatWeapon **)
                     ((int)local_100.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter +
                     iVar33 * 4);
        }
        local_100.m_pExtraShouldHitCheckFunction = (ShouldHitFunc_t)0x0;
        if (-1 < local_100.m_collisionGroup) {
          if (local_100.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter !=
              (_func_int_varargs **)0x0) {
            (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x516c0b) + 8))();
            local_100.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter =
                 (ITraceFilter)(_func_int_varargs **)0x0;
          }
          local_100.m_pPassEnt = (IHandleEntity *)0x0;
        }
        local_f0 = local_100.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter;
        if (-1 < local_100.m_collisionGroup) {
          if (local_100.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter !=
              (_func_int_varargs **)0x0) {
            (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x516c0b) + 8))();
            local_100.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter =
                 (ITraceFilter)(_func_int_varargs **)0x0;
          }
          local_100.m_pPassEnt = (IHandleEntity *)0x0;
        }
LAB_0073168f:
                    /* Unresolved local var: CBaseCombatWeapon * pActiveWeapon@[???] */
        if ((pCVar27 != (CBaseCombatWeapon *)0x0) &&
           (pCVar30 = CBaseCombatCharacter::GetActiveWeapon((CBaseCombatCharacter *)pBot),
           pCVar27 != pCVar30)) {
          (**(code **)(*(int *)&(pBot->
                                super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                                super_CPaintableEntity<CBaseMultiplayerPlayer>.
                                super_CBaseMultiplayerPlayer.super_CAI_ExpresserHost<CBasePlayer>.
                                super_CBasePlayer.super_CBaseCombatCharacter.super_CBaseFlex + 0x47c
                      ))();
        }
      }
    }
    pCVar27 = CBaseCombatCharacter::GetActiveWeapon((CBaseCombatCharacter *)pBot);
    if (pCVar27 != (CBaseCombatWeapon *)0x0) {
      if (*(int *)(*(int *)(unaff_EBX + 0x698bcb) + 0x30) != 0) {
        local_1fc = local_1fc | 1;
      }
      if (*(int *)(*(int *)(unaff_EBX + 0x698b6b) + 0x30) != 0) {
        local_1fc = local_1fc | 0x800;
      }
    }
    iVar33 = *(int *)(*(int *)(unaff_EBX + 0x698c2b) + 0x30);
    if (iVar33 != 0) {
      if (*(int *)(*(int *)(unaff_EBX + 0x698bcb) + 0x30) == 0) {
        lVar35 = (longdouble)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5174db) + 0x14))()
        ;
        if (*(float *)(unaff_EBX + 0x39cbaf) <= (float)lVar35 &&
            (float)lVar35 != *(float *)(unaff_EBX + 0x39cbaf)) {
          iVar33 = *(int *)(*(int *)(unaff_EBX + 0x698c2b) + 0x30);
          goto LAB_00731460;
        }
        iVar33 = *(int *)(*(int *)(unaff_EBX + 0x698c2b) + 0x30);
      }
      else {
LAB_00731460:
        local_1fc = (-(uint)(*(int *)(*(int *)(unaff_EBX + 0x698b6b) + 0x30) == 0) & 0xfffff801) +
                    0x800 & 0xffff | local_1fc;
      }
      goto joined_r0x00730c2b;
    }
LAB_0073006c:
    local_208 = 0.0;
  }
  peVar9 = (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
           super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
           super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
           m_Network.m_pPev;
                    /* Unresolved local var: int edictIndex@[???] */
  iVar21 = -1;
  if (peVar9 != (edict_t *)0x0) {
    iVar21 = ((int)peVar9 - *(int *)(**(int **)(unaff_EBX + 0x516c13) + 0x58) >> 4) + -1;
  }
  pCVar22 = UTIL_PlayerByIndex(iVar21);
  piVar32 = (int *)&stack0xfffffd64;
  if ((*(int *)(*(int *)(unaff_EBX + 0x698feb) + 0x30) != 0) &&
     (piVar32 = (int *)&stack0xfffffd64, pCVar22 != (CBasePlayer *)0x0)) {
    pQVar23 = (QAngle *)
              (**(code **)((int)(pCVar22->super_CBaseCombatCharacter).super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                                super_IServerEntity.super_IServerUnknown + 0x224))();
    AngleVectors(pQVar23,&local_a0);
    local_11c = 0;
                    /* Unresolved local var: Vector res@[???] */
    local_d4 = *(float *)(unaff_EBX + 0x39d07f);
    local_dc = local_a0.x * local_d4;
    local_d8 = local_a0.y * local_d4;
    local_d4 = local_d4 * local_a0.z;
    local_58 = local_dc;
    local_54 = local_d8;
    local_50 = local_d4;
    (**(code **)((int)(pCVar22->super_CBaseCombatCharacter).super_CBaseFlex.
                      super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                      super_IServerEntity.super_IServerUnknown + 0x220))();
                    /* Unresolved local var: Vector res@[???] */
    local_64 = local_4c + local_58;
    local_60 = local_48 + local_54;
    local_5c = local_44 + local_50;
    (**(code **)((int)(pCVar22->super_CBaseCombatCharacter).super_CBaseFlex.
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
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x516c7f) + 0x14))();
    if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x516c7b) + 0x1c) + 0x30) != 0) {
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
    (**(code **)((int)(pCVar22->super_CBaseCombatCharacter).super_CBaseFlex.
                      super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                      super_IServerEntity.super_IServerUnknown + 0x220))();
    piVar32 = (int *)&stack0xfffffd50;
    NDebugOverlay::Line(&local_94,&local_1ac,0,0,0xff,false,0.1);
  }
  if ((*(int *)(*(int *)(unaff_EBX + 0x698f2b) + 0x30) == 0) || (pCVar22 == (CBasePlayer *)0x0)) {
    piVar32[2] = 0;
    piVar32[1] = 2;
    *piVar32 = (int)pBot;
    piVar32[-1] = 0x73079a;
    CBaseEntity::SetMoveType((CBaseEntity *)*piVar32,piVar32[1],piVar32[2]);
    local_200 = 0.0;
  }
  else {
    piVar32[2] = 0;
    piVar32[1] = 8;
    *piVar32 = (int)pBot;
    piVar32[-1] = 0x730456;
    CBaseEntity::SetMoveType((CBaseEntity *)*piVar32,piVar32[1],piVar32[2]);
    if (((pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
         super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
         super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
         super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
         m_iEFlags & 0x800) != 0) {
      *piVar32 = (int)pBot;
      piVar32[-1] = 0x7314ae;
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)*piVar32);
    }
    if (((pCVar22->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
         super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      *piVar32 = (int)pCVar22;
      piVar32[-1] = 0x730477;
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)*piVar32);
    }
                    /* Unresolved local var: Vector res@[???] */
    local_ac = (pCVar22->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
               super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.x -
               (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
               super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
               super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
               super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
               m_vecAbsOrigin.x;
    local_a8 = (pCVar22->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
               super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.y -
               (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
               super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
               super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
               super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
               m_vecAbsOrigin.y;
    local_a4 = (pCVar22->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
               super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z -
               (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
               super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
               super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
               super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
               m_vecAbsOrigin.z;
                    /* Unresolved local var: __m128 root@[???] */
    pcVar36 = (char *)SQRT(local_ac * local_ac + local_a8 * local_a8 + local_a4 * local_a4);
    uStack_e8 = 0;
    uStack_e4 = 0;
    uStack_e0 = 0;
    *piVar32 = (int)&local_a0;
    piVar32[-1] = 0x730542;
    local_ec = pcVar36;
    local_a0.x = local_ac;
    local_a0.y = local_a8;
    local_a0.z = local_a4;
    VectorNormalize((Vector *)*piVar32);
    local_200 = 0.0;
    pfVar1 = (float *)(*(int *)(unaff_EBX + 0x698f8b) + 0x2c);
    if (*pfVar1 <= (float)pcVar36 && (float)pcVar36 != *pfVar1) {
                    /* Unresolved local var: float speed@[???]
                       Unresolved local var: Vector forward@[???]
                       Unresolved local var: Vector right@[???]
                       Unresolved local var: Vector up@[???] */
      local_200 = (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                  super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
                  super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_flMaxspeed.m_Value;
      iVar21 = *(int *)&(pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                        super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
                        super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.
                        super_CBaseCombatCharacter.super_CBaseFlex;
      *piVar32 = (int)pBot;
      pcVar11 = *(code **)(iVar21 + 0x224);
      piVar32[-1] = 0x730589;
      uVar24 = (*pcVar11)();
      piVar32[3] = (int)&local_100;
      piVar32[2] = (int)&local_15c;
      piVar32[1] = (int)&local_1b8;
      *piVar32 = uVar24;
      piVar32[-1] = 0x7305af;
      AngleVectors((QAngle *)*piVar32,(Vector *)piVar32[1],(Vector *)piVar32[2],(Vector *)piVar32[3]
                  );
                    /* Unresolved local var: Vector norm@[???] */
      local_b8 = local_1b8.x;
      local_b4 = local_1b8.y;
      local_b0 = local_1b8.z;
      *piVar32 = (int)&local_b8;
      piVar32[-1] = 0x7305e1;
      VectorNormalize((Vector *)*piVar32);
      local_208 = local_200 *
                  (local_a0.z * local_b0 + local_a0.y * local_b4 + local_a0.x * local_b8);
                    /* Unresolved local var: Vector norm@[???] */
      local_c4 = local_15c;
      local_c0 = local_158;
      local_bc = local_154;
      *piVar32 = (int)&local_c4;
      piVar32[-1] = 0x730679;
      VectorNormalize((Vector *)*piVar32);
      local_204 = local_200 *
                  (local_a0.z * local_bc + local_a0.y * local_c0 + local_a0.x * local_c4);
                    /* Unresolved local var: Vector norm@[???] */
      local_d0 = local_100.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter;
      local_cc = local_100.m_pPassEnt;
      local_c8 = (float)local_100.m_collisionGroup;
      *piVar32 = (int)&local_d0;
      piVar32[-1] = 0x730711;
      VectorNormalize((Vector *)*piVar32);
      local_200 = local_200 *
                  (local_a0.z * local_c8 +
                  local_a0.y * (float)local_cc + local_a0.x * (float)local_d0._vptr_ITraceFilter);
    }
  }
  local_1b8.x = (vec_t)(unaff_EBX + 0x51c277);
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
  iVar21 = **(int **)(unaff_EBX + 0x516c13);
  fVar39 = *(float *)(iVar21 + 0x10);
  fVar6 = *(float *)(iVar21 + 0xc);
  iVar21 = (int)(((fVar39 + fVar6) - fVar5) / *(float *)(iVar21 + 0x1c) +
                *(float *)(unaff_EBX + 0x39cbaf));
  if (iVar21 != (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
                super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_nTickBase.m_Value) {
    if ((pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
        super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
        super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.m_Network
        .m_TimerEvent.m_bRegistered == false) {
      peVar9 = (pBot->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
               super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
               super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
               super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
               m_Network.m_pPev;
      if ((peVar9 != (edict_t *)0x0) &&
         (uVar34 = (peVar9->super_CBaseEdict).m_fStateFlags, (uVar34 & 0x100) == 0)) {
        (peVar9->super_CBaseEdict).m_fStateFlags = uVar34 | 1;
        *piVar32 = (int)peVar9;
        piVar32[-1] = 0x7314c0;
        pIVar29 = CBaseEdict::GetChangeAccessor((CBaseEdict *)*piVar32);
        piVar12 = *(int **)(unaff_EBX + 0x516c17);
        puVar15 = (ushort *)*piVar12;
        if (pIVar29->m_iChangeInfoSerialNumber == *puVar15) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar34 = (uint)pIVar29->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar7 = puVar15[uVar34 * 0x14 + 0x14];
          if (uVar7 == 0) {
LAB_007317f2:
            puVar15[(uint)uVar7 + uVar34 * 0x14 + 1] = 0xf14;
            puVar15[uVar34 * 0x14 + 0x14] = uVar7 + 1;
          }
          else if (puVar15[uVar34 * 0x14 + 1] != 0xf14) {
            uVar31 = 0;
            do {
              uVar4 = uVar31 + 1;
              uVar31 = uVar4 & 0xffff;
              if ((ushort)uVar4 == uVar7) {
                if (uVar7 != 0x13) goto LAB_007317f2;
                goto LAB_00731820;
              }
            } while (puVar15[uVar34 * 0x14 + uVar31 + 1] != 0xf14);
          }
        }
        else if (puVar15[0x7d1] == 100) {
LAB_00731820:
          pIVar29->m_iChangeInfoSerialNumber = 0;
          (peVar9->super_CBaseEdict).m_fStateFlags =
               (peVar9->super_CBaseEdict).m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar29->m_iChangeInfo = puVar15[0x7d1];
          *(short *)(*piVar12 + 0xfa2) = *(short *)(*piVar12 + 0xfa2) + 1;
          pIVar29->m_iChangeInfoSerialNumber = *(ushort *)*piVar12;
          iVar33 = *piVar12 + (uint)pIVar29->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar33 + 2) = 0xf14;
          *(undefined2 *)(iVar33 + 0x28) = 1;
        }
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
    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.m_nTickBase.m_Value = iVar21;
  }
  piVar32[2] = 0x5c;
  piVar32[1] = 0;
  *piVar32 = (int)&local_1b8;
  piVar32[-1] = 0x73093c;
  _V_memset((void *)*piVar32,piVar32[1],piVar32[2]);
  iVar21 = *(int *)(*(int *)(unaff_EBX + 0x71ceeb) + 0x30);
  if ((0 < iVar21) && (iVar21 <= *(int *)(**(int **)(unaff_EBX + 0x516c13) + 0x14))) {
    *piVar32 = iVar21;
    piVar32[-1] = 0x730db8;
    pCVar22 = UTIL_PlayerByIndex(*piVar32);
    if (pCVar22 != (CBasePlayer *)0x0) {
      if ((CUserCmd *)&local_1b8 != &pCVar22->m_LastCmd) {
        local_1b8.y = (vec_t)(pCVar22->m_LastCmd).command_number;
        local_1b8.z = (vec_t)(pCVar22->m_LastCmd).tick_count;
        local_1ac.x = (pCVar22->m_LastCmd).viewangles.x;
        local_1ac.y = (pCVar22->m_LastCmd).viewangles.y;
        local_1ac.z = (pCVar22->m_LastCmd).viewangles.z;
        local_1a0 = (pCVar22->m_LastCmd).forwardmove;
        local_19c = (pCVar22->m_LastCmd).sidemove;
        local_198 = (pCVar22->m_LastCmd).upmove;
        local_194 = (pCVar22->m_LastCmd).buttons;
        local_190 = (pCVar22->m_LastCmd).impulse;
        local_18c = (pCVar22->m_LastCmd).weaponselect;
        local_188 = (pCVar22->m_LastCmd).weaponsubtype;
        local_184 = (pCVar22->m_LastCmd).random_seed;
        uVar8 = (pCVar22->m_LastCmd).mousedx;
        local_180 = CONCAT22((pCVar22->m_LastCmd).mousedy,uVar8);
        local_17c = (pCVar22->m_LastCmd).hasbeenpredicted;
        local_17a = (pCVar22->m_LastCmd).player_held_entity;
        local_178 = (short)*(undefined4 *)
                            &(pCVar22->m_LastCmd).held_entity_was_grabbed_through_portal;
        local_174 = (pCVar22->m_LastCmd).headangles.x;
        local_170 = (pCVar22->m_LastCmd).headangles.y;
        local_16c = (pCVar22->m_LastCmd).headangles.z;
        local_168 = (pCVar22->m_LastCmd).headoffset.x;
        local_164 = (pCVar22->m_LastCmd).headoffset.y;
        local_160 = (pCVar22->m_LastCmd).headoffset.z;
      }
      local_1ac.y = local_1ac.y + *(float *)(*(int *)(unaff_EBX + 0x71cf4b) + 0x2c);
      goto LAB_007309e0;
    }
  }
  local_1ac.x = local_28[0].x;
  local_1ac.y = local_28[0].y;
  local_1ac.z = local_28[0].z;
  local_1a0 = local_208;
  local_19c = local_204;
  local_198 = local_200;
  local_194 = local_1fc;
  local_190 = '\0';
  piVar12 = (int *)**(undefined4 **)(unaff_EBX + 0x5174db);
  iVar21 = *piVar12;
  piVar32[2] = 0x7fffffff;
  piVar32[1] = 0;
  *piVar32 = (int)piVar12;
  pcVar11 = *(code **)(iVar21 + 0x10);
  piVar32[-1] = 0x7309da;
  local_184 = (*pcVar11)();
LAB_007309e0:
  if (*(int *)(*(int *)(unaff_EBX + 0x698ceb) + 0x30) != 0) {
    local_1a0 = 0.0;
    local_19c = 0.0;
    local_198 = 0.0;
  }
  piVar32[-1] = 0x730a13;
  pIVar25 = MoveHelperServer();
  pp_Var13 = pIVar25->_vptr_IMoveHelper;
  piVar32[1] = (int)pBot;
  *piVar32 = (int)pIVar25;
  p_Var14 = pp_Var13[1];
  piVar32[-1] = 0x730a22;
  (*p_Var14)();
  pcVar11 = *(code **)(*(int *)&(pBot->
                                super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                                super_CPaintableEntity<CBaseMultiplayerPlayer>.
                                super_CBaseMultiplayerPlayer.super_CAI_ExpresserHost<CBasePlayer>.
                                super_CBasePlayer.super_CBaseCombatCharacter.super_CBaseFlex + 0x708
                      );
  piVar32[-1] = 0x730a32;
  pIVar25 = MoveHelperServer();
  piVar32[2] = (int)pIVar25;
  piVar32[1] = (int)&local_1b8;
  *piVar32 = (int)pBot;
  piVar32[-1] = 0x730a42;
  (*pcVar11)();
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
  piVar12 = *(int **)(unaff_EBX + 0x516c13);
  *(float *)(*piVar12 + 0x10) = fVar39;
  *(float *)(*piVar12 + 0xc) = fVar6;
  iVar21 = *piVar10;
  *piVar32 = (int)piVar10;
  pcVar11 = *(code **)(iVar21 + 0x78);
  piVar32[-1] = 0x730bba;
  (*pcVar11)();
  return;
}


/* cc_bot_sendcommand at 0072fc40 */

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
  local_20 = **(int **)(unaff_EBX + 0x516f59);
  if (args->m_nArgc < 3) {
    _Msg(unaff_EBX + 0x38218d);
  }
  else {
    pCVar1 = UTIL_PlayerByName(args->m_ppArgv[1]);
    if (pCVar1 == (CBasePlayer *)0x0) {
      pcVar2 = &UNK_0031f0cd + unaff_EBX;
      if (1 < args->m_nArgc) {
        pcVar2 = args->m_ppArgv[1];
      }
      _Msg(unaff_EBX + 0x38205e,pcVar2);
    }
    else {
      if (args->m_nArgc == 0) {
        pcVar2 = &UNK_0031f0cd + unaff_EBX;
        pcVar4 = pcVar2;
      }
      else {
        pcVar2 = args->m_pArgSBuffer;
        pcVar4 = &UNK_0031f0cd + unaff_EBX;
        if (2 < args->m_nArgc) {
          pcVar4 = args->m_ppArgv[2];
        }
      }
      pcVar2 = _strstr(pcVar2,pcVar4);
      iVar3 = _V_strlen(pcVar2);
      pcVar4 = _malloc(iVar3 + 1U);
      V_snprintf(pcVar4,iVar3 + 1U,(char *)(unaff_EBX + 0x32700d),pcVar2);
      if (pcVar4[iVar3 + -1] == '\"') {
        pcVar4[iVar3 + -1] = '\0';
      }
      CCommand::CCommand(&local_528);
      CCommand::Tokenize(&local_528,pcVar4,(characterset_t *)0x0);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x516f49) + 0x134))
                ((int *)**(undefined4 **)(unaff_EBX + 0x516f49),pCVar1,&local_528);
    }
  }
  if (local_20 == **(int **)(unaff_EBX + 0x516f59)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}


/* __tcf_19 at 00a3d990 */

void __tcf_19(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x40fc48),in_stack_00000008);
  return;
}


/* cc_bot_kill at 0072f780 */

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
  pcVar2 = &UNK_0031f593 + unaff_EBX;
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
  pcVar2 = &UNK_0031f593 + unaff_EBX;
  if (1 < args->m_nArgc) {
    pcVar2 = args->m_ppArgv[1];
  }
  _Msg(unaff_EBX + 0x382524,pcVar2);
  return;
}


/* __tcf_20 at 00a3d970 */

void __tcf_20(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x40fca8),in_stack_00000008);
  return;
}


/* bot_refill at 0072f4e0 */

void bot_refill(CCommand *args)

{
  return;
}


/* __tcf_21 at 00a3d950 */

void __tcf_21(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x40fd08),in_stack_00000008);
  return;
}


/* bot_teleport at 0072fab0 */

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
    _Msg(CAI_MoveProbe::IterativeFloorPoint + unaff_EBX);
  }
  else {
    pCVar1 = UTIL_PlayerByName(args->m_ppArgv[1]);
    if (pCVar1 == (CBasePlayer *)0x0) {
      pcVar2 = &UNK_0031f260 + unaff_EBX;
      if (1 < args->m_nArgc) {
        pcVar2 = args->m_ppArgv[1];
      }
      _Msg(unaff_EBX + 0x3821f1,pcVar2);
    }
    else {
      pcVar2 = &UNK_0031f260 + unaff_EBX;
      if (4 < args->m_nArgc) {
        pcVar2 = args->m_ppArgv[4];
      }
      dVar3 = _atof(pcVar2);
      pcVar2 = &UNK_0031f260 + unaff_EBX;
      if (3 < args->m_nArgc) {
        pcVar2 = args->m_ppArgv[3];
      }
      dVar4 = _atof(pcVar2);
      pcVar2 = &UNK_0031f260 + unaff_EBX;
      if (2 < args->m_nArgc) {
        pcVar2 = args->m_ppArgv[2];
      }
      dVar5 = _atof(pcVar2);
      local_28 = (float)dVar5;
      local_24 = (float)dVar4;
      local_20 = (float)dVar3;
      pcVar2 = &UNK_0031f260 + unaff_EBX;
      if (7 < args->m_nArgc) {
        pcVar2 = args->m_ppArgv[7];
      }
      dVar3 = _atof(pcVar2);
      pcVar2 = &UNK_0031f260 + unaff_EBX;
      if (6 < args->m_nArgc) {
        pcVar2 = args->m_ppArgv[6];
      }
      dVar4 = _atof(pcVar2);
      pcVar2 = &UNK_0031f260 + unaff_EBX;
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


/* __tcf_22 at 00a3d930 */

void __tcf_22(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x40fd68),in_stack_00000008);
  return;
}


/* cc_bot_equip at 0072f920 */

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
    pcVar4 = &UNK_0031f3f0 + unaff_EBX;
    if (1 < args->m_nArgc) {
      pcVar4 = args->m_ppArgv[1];
    }
    _Msg(unaff_EBX + 0x382381,pcVar4);
  }
  else {
    pcVar4 = &UNK_0031f3f0 + unaff_EBX;
    if (2 < args->m_nArgc) {
      pcVar4 = args->m_ppArgv[2];
    }
    iVar2 = (**(code **)((int)(pCVar1->super_CBaseCombatCharacter).super_CBaseFlex.
                              super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                              super_IServerEntity.super_IServerUnknown + 0x464))(pCVar1,pcVar4,0);
    if (iVar2 == 0) {
                    /* Unresolved local var: CBaseCombatWeapon * pWpn@[???] */
      pcVar4 = &UNK_0031f3f0 + unaff_EBX;
      if (2 < args->m_nArgc) {
        pcVar4 = args->m_ppArgv[2];
      }
      pCVar3 = CreateEntityByName(pcVar4,-1,true);
      if ((pCVar3 != (CBaseEntity *)0x0) &&
         (iVar2 = ___dynamic_cast(pCVar3,*(undefined4 *)(unaff_EBX + 0x51725c),
                                  *(undefined4 *)(unaff_EBX + 0x5173dc),0), iVar2 != 0)) {
        (**(code **)((int)(pCVar1->super_CBaseCombatCharacter).super_CBaseFlex.
                          super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                          super_IServerEntity.super_IServerUnknown + 0x47c))(pCVar1,iVar2,0);
        return;
      }
      pcVar4 = &UNK_0031f3f0 + unaff_EBX;
      if (2 < args->m_nArgc) {
        pcVar4 = args->m_ppArgv[2];
      }
      _Msg(unaff_EBX + 0x382440,pcVar4);
    }
    else {
      pcVar4 = &UNK_0031f3f0 + unaff_EBX;
      if (2 < args->m_nArgc) {
        pcVar4 = args->m_ppArgv[2];
      }
      pcVar5 = &UNK_0031f3f0 + unaff_EBX;
      if (1 < args->m_nArgc) {
        pcVar5 = args->m_ppArgv[1];
      }
      _Msg(unaff_EBX + 0x38242b,pcVar5,pcVar4);
    }
  }
  return;
}


/* __tcf_23 at 00a3d910 */

void __tcf_23(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x40fdc8),in_stack_00000008);
  return;
}


/* _GLOBAL__I_bot_forcefireweapon at 000ad840 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_bot_forcefireweapon(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

