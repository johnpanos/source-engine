/* DWARF-guided pseudocode for game/client/portal2/c_paintblob_render.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 00075130 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *pDefaultValue;
  char *pDefaultValue_00;
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
  *(undefined1 *)((int)s_randomGradients + unaff_EBX + 0x52e) = 0;
  *(undefined1 *)((int)s_randomGradients + unaff_EBX + 0x52f) = 0;
  *(undefined1 *)((int)s_randomGradients + unaff_EBX + 0x530) = 0;
  *(undefined1 *)((int)s_randomGradients + unaff_EBX + 0x531) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x532) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x536) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x53a) = 0x7f7fffff;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x53e) = 0x7f7fffff;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x542) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x546) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x54a) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x54e) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x552) = 0x7f7fffff;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x556) = 0x7f7fffff;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x55a) = 0x7f7fffff;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x55e) = 0x7f7fffff;
  *(undefined **)((int)s_randomGradients + unaff_EBX + 0x562) = &UNK_00aefd4a + unaff_EBX;
  pDefaultValue = (char *)(unaff_EBX + 0x9169ee);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x582),(char *)(unaff_EBX + 0x9449d7)
                 ,pDefaultValue,0,(char *)(unaff_EBX + 0x9449b2),in_stack_ffffffc8,in_stack_ffffffcc
                 ,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  uVar1 = *(undefined4 *)(&DAT_00aeec2e + unaff_EBX);
  ___cxa_atexit(unaff_EBX + 0x906a72,0,uVar1);
  pDefaultValue_00 = &UNK_0090dd7e + unaff_EBX;
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x5e2),(char *)(unaff_EBX + 0x944a1b)
                 ,pDefaultValue_00,0,(char *)(unaff_EBX + 0x9449f6),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x906a52,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x642),(char *)(unaff_EBX + 0x944a5d)
                 ,pDefaultValue_00,0,(char *)(unaff_EBX + 0x944a2e),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x906a32,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x6a2),(char *)(unaff_EBX + 0x944a9e)
                 ,(char *)(unaff_EBX + 0x9168c6),0,(char *)(unaff_EBX + 0x944a76),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(&UNK_00906a12 + unaff_EBX,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x702),(char *)(unaff_EBX + 0x944add)
                 ,(char *)(unaff_EBX + 0x92559a),0,(char *)(unaff_EBX + 0x944ab6),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(&UNK_009069f2 + unaff_EBX,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x762),(char *)(unaff_EBX + 0x944b3f)
                 ,(char *)(unaff_EBX + 0x944b3b),0,(char *)(unaff_EBX + 0x944afa),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(&UNK_009069d2 + unaff_EBX,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x7c2),&UNK_00944b9f + unaff_EBX,
                 &UNK_00944b9b + unaff_EBX,0,(char *)(unaff_EBX + 0x944b5e),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(&UNK_009069b2 + unaff_EBX,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x822),(char *)(unaff_EBX + 0x944bd8)
                 ,pDefaultValue,0x80,(char *)(unaff_EBX + 0x944bbd),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(&UNK_00906992 + unaff_EBX,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x882),(char *)(unaff_EBX + 0x944c0c)
                 ,pDefaultValue_00,0x80,(char *)(unaff_EBX + 0x944bf4),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(&UNK_00906972 + unaff_EBX,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x8e2),(char *)(unaff_EBX + 0x944c3e)
                 ,pDefaultValue_00,0x80,(char *)(unaff_EBX + 0x944c26),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x906952,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x942),(char *)(unaff_EBX + 0x944c71)
                 ,pDefaultValue_00,0x80,(char *)(unaff_EBX + 0x944c55),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x906932,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x9a2),
                 (char *)(vgui::TextEntry::PanelMessageFunc_OnSliderMoved::InitVar + unaff_EBX + 7),
                 pDefaultValue_00,0x80,(char *)(unaff_EBX + 0x944c8f),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x906912,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0xa02),(char *)(unaff_EBX + 0x944d0a)
                 ,(char *)(unaff_EBX + 0x91fc6e),0,(char *)(unaff_EBX + 0x944cc6),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x9068f2,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0xa62),(char *)(unaff_EBX + 0x944d5c)
                 ,pDefaultValue_00,0,(char *)(unaff_EBX + 0x944d2a),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x9068d2,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0xac2),(char *)(unaff_EBX + 0x944da2)
                 ,pDefaultValue_00,0,(char *)(unaff_EBX + 0x944d7a),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x9068b2,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0xb22),(char *)(unaff_EBX + 0x944dd8)
                 ,pDefaultValue_00,0,(char *)(unaff_EBX + 0x944dc9),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x906892,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0xb82),(char *)(unaff_EBX + 0x944e0c)
                 ,(char *)(unaff_EBX + 0x91fc6e),0,&UNK_00944dee + unaff_EBX,in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x906872,0,uVar1);
  pCVar3 = (ConVar *)(unaff_EBX + 0x944e21);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0xbe2),(char *)(unaff_EBX + 0x944e31)
                 ,(char *)(unaff_EBX + 0x913b26),0,(char *)pCVar3,in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x906852,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0xc42),(char *)(unaff_EBX + 0x944e46)
                 ,pDefaultValue,0,pCVar3,(char *)in_stack_ffffffc8,in_stack_ffffffcc,
                 (int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x906832,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0xca2),(char *)(unaff_EBX + 0x944e72)
                 ,(char *)(unaff_EBX + 0x944e6a),0,pCVar3,(char *)in_stack_ffffffc8,
                 in_stack_ffffffcc,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x906812,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0xd02),&UNK_00944e9a + unaff_EBX,
                 (char *)(unaff_EBX + 0x944e94),0,pCVar3,(char *)in_stack_ffffffc8,in_stack_ffffffcc
                 ,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x9067f2,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0xd62),&UNK_00944eeb + unaff_EBX,
                 pDefaultValue,0,(char *)(unaff_EBX + 0x944eba),in_stack_ffffffc8,in_stack_ffffffcc,
                 in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x9067d2,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0xdc2),(char *)(unaff_EBX + 0x944f41)
                 ,pDefaultValue,0,(char *)(unaff_EBX + 0x944f02),in_stack_ffffffc8,in_stack_ffffffcc
                 ,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x9067b2,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0xe22),(char *)(unaff_EBX + 0x944fa6)
                 ,pDefaultValue,0,(char *)(unaff_EBX + 0x944f56),in_stack_ffffffc8,in_stack_ffffffcc
                 ,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x906792,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0xe82),(char *)(unaff_EBX + 0x945042)
                 ,pDefaultValue_00,0,(char *)(unaff_EBX + 0x944fca),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x906772,0,uVar1);
  pCVar3 = (ConVar *)(unaff_EBX + 0x945066);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0xee2),(char *)(unaff_EBX + 0x9450b4)
                 ,pDefaultValue_00,0,(char *)pCVar3,in_stack_ffffffc8,in_stack_ffffffcc,
                 in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x906752,0,uVar1);
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0xf3a) = 0;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0xf3e) = 0;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0xf42) = 0;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0xf46) = 0;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0xf4a) = 0;
  ___cxa_atexit(unaff_EBX + 0x906a92,0,uVar1);
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0xf4e) = 0;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0xf52) = 0;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0xf56) = 0;
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0xf62),(char *)(unaff_EBX + 0x9450d2)
                 ,pDefaultValue_00,0x4002,pCVar3,(char *)in_stack_ffffffc8,in_stack_ffffffcc,
                 (int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x906732,0,uVar1);
  return;
}


/* __tcf_0 at 0097bbb0 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ec6c8),in_stack_00000008);
  return;
}


/* __tcf_1 at 0097bb90 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ec748),in_stack_00000008);
  return;
}


/* __tcf_2 at 0097bb70 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ec7c8),in_stack_00000008);
  return;
}


/* __tcf_3 at 0097bb50 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ec848),in_stack_00000008);
  return;
}


/* __tcf_4 at 0097bb30 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ec8c8),in_stack_00000008);
  return;
}


/* __tcf_5 at 0097bb10 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ec948),in_stack_00000008);
  return;
}


/* __tcf_6 at 0097baf0 */

void __tcf_6(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ec9c8),in_stack_00000008);
  return;
}


/* __tcf_7 at 0097bad0 */

void __tcf_7(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3eca48),in_stack_00000008);
  return;
}


/* __tcf_8 at 0097bab0 */

void __tcf_8(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ecac8),in_stack_00000008);
  return;
}


/* __tcf_9 at 0097ba90 */

void __tcf_9(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ecb48),in_stack_00000008);
  return;
}


/* __tcf_10 at 0097ba70 */

void __tcf_10(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ecbc8),in_stack_00000008);
  return;
}


/* __tcf_11 at 0097ba50 */

void __tcf_11(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ecc48),in_stack_00000008);
  return;
}


/* __tcf_12 at 0097ba30 */

void __tcf_12(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3eccc8),in_stack_00000008);
  return;
}


/* __tcf_13 at 0097ba10 */

void __tcf_13(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ecd48),in_stack_00000008);
  return;
}


/* __tcf_14 at 0097b9f0 */

void __tcf_14(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ecdc8),in_stack_00000008);
  return;
}


/* __tcf_15 at 0097b9d0 */

void __tcf_15(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ece48),in_stack_00000008);
  return;
}


/* __tcf_16 at 0097b9b0 */

void __tcf_16(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ecec8),in_stack_00000008);
  return;
}


/* __tcf_17 at 0097b990 */

void __tcf_17(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ecf48),in_stack_00000008);
  return;
}


/* __tcf_18 at 0097b970 */

void __tcf_18(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ecfc8),in_stack_00000008);
  return;
}


/* __tcf_19 at 0097b950 */

void __tcf_19(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ed048),in_stack_00000008);
  return;
}


/* __tcf_20 at 0097b930 */

void __tcf_20(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ed0c8),in_stack_00000008);
  return;
}


/* __tcf_21 at 0097b910 */

void __tcf_21(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ed148),in_stack_00000008);
  return;
}


/* __tcf_22 at 0097b8f0 */

void __tcf_22(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(&DAT_003ed1c8 + extraout_ECX),in_stack_00000008);
  return;
}


/* __tcf_23 at 0097b8d0 */

void __tcf_23(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ed248),in_stack_00000008);
  return;
}


/* __tcf_24 at 0097b8b0 */

void __tcf_24(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ed2c8),in_stack_00000008);
  return;
}


/* __tcf_25 at 0097b890 */

void __tcf_25(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ed348),in_stack_00000008);
  return;
}


/* __tcf_26 at 0097bbd0 */

void __tcf_26(void *param_1)

{
  int unaff_EBX;
  undefined4 uVar1;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_bx();
  *(undefined4 *)(unaff_EBX + 0x3ed060) = 0;
  if (*(int *)(unaff_EBX + 0x3ed05c) < 0) {
    uVar1 = *(undefined4 *)(unaff_EBX + 0x3ed054);
  }
  else {
    if (*(uint *)(unaff_EBX + 0x3ed054) != 0) {
                    /* Unresolved local var: void * pAlloc@[???] */
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1e818c) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1e818c),
                 *(undefined4 *)((*(uint *)(unaff_EBX + 0x3ed054) & 0xfffffffc) - 4));
      *(undefined4 *)(unaff_EBX + 0x3ed054) = 0;
    }
    uVar1 = 0;
    *(undefined4 *)(unaff_EBX + 0x3ed058) = 0;
  }
  *(undefined4 *)(unaff_EBX + 0x3ed064) = uVar1;
  CUtlMemoryAligned<ImpParticleWithFourInterpolants,16>::~CUtlMemoryAligned
            ((CUtlMemoryAligned<ImpParticleWithFourInterpolants,16> *)(unaff_EBX + 0x3ed054),
             in_stack_00000008);
  return;
}


/* NPaintRenderer::Paintblob_DrawFastSphere at 0051f390 */

/* WARNING: Restarted to delay deadcode elimination for space: stack */

void NPaintRenderer::Paintblob_DrawFastSphere
               (CMeshBuilder *meshBuilder,Vector *center,float radius,float r,float g,float b)

{
  float *pfVar1;
  float **ppfVar2;
  uchar **ppuVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  float fVar10;
  float fVar11;
  float *pfVar12;
  int iVar13;
  int iVar14;
  int unaff_EBX;
  undefined4 *puVar15;
  short local_4c;
  int i;
  float *local_34;
  float dc;
  
                    /* Unresolved local var: int offset@[???]
                       Unresolved local var: Vector pos@[???] */
  ___i686_get_pc_thunk_bx();
  iVar14 = (meshBuilder->m_VertexBuilder).m_nCurrentVertex;
  fVar4 = *(float *)(unaff_EBX + 0x4c31a2);
  fVar5 = *(float *)(&DAT_004c4cb6 + unaff_EBX);
  pfVar1 = (float *)(unaff_EBX + 0x73a382);
  i = 0;
  local_34 = pfVar1;
  do {
    fVar6 = local_34[1];
    fVar7 = center->y;
    fVar8 = local_34[6];
    fVar9 = local_34[2];
    fVar10 = center->z;
    fVar11 = local_34[7];
                    /* Unresolved local var: float * pDst@[???] */
    pfVar12 = (meshBuilder->m_VertexBuilder).m_pCurrPosition;
    *pfVar12 = *local_34 + center->x + radius * local_34[5];
    pfVar12[1] = fVar6 + fVar7 + radius * fVar8;
    pfVar12[2] = fVar9 + fVar10 + radius * fVar11;
                    /* Unresolved local var: float * pDst@[???] */
    pfVar12 = (meshBuilder->m_VertexBuilder).m_pCurrNormal;
    *pfVar12 = local_34[5];
    pfVar12[1] = pfVar1[i * 8 + 6];
    pfVar12[2] = pfVar1[i * 8 + 7];
                    /* Unresolved local var: float * pDst@[???] */
    pfVar12 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[0];
    *pfVar12 = local_34[3];
    pfVar12[1] = pfVar1[i * 8 + 4];
                    /* Unresolved local var: int col@[???] */
    *(uint *)(meshBuilder->m_VertexBuilder).m_pCurrColor =
         ((uint)(g * fVar4 + fVar5) & 0xff) << 8 | ((uint)(b * fVar4 + fVar5) & 0xff) << 0x10 |
         (uint)(r * fVar4 + fVar5) & 0xff | 0xff000000;
    iVar13 = (meshBuilder->m_VertexBuilder).m_nCurrentVertex + 1;
    (meshBuilder->m_VertexBuilder).m_nCurrentVertex = iVar13;
    if ((meshBuilder->m_VertexBuilder).m_nVertexCount < iVar13) {
      (meshBuilder->m_VertexBuilder).m_nVertexCount = iVar13;
    }
    ppfVar2 = &(meshBuilder->m_VertexBuilder).m_pCurrPosition;
    *ppfVar2 = (float *)((int)*ppfVar2 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Position);
    ppfVar2 = &(meshBuilder->m_VertexBuilder).m_pCurrNormal;
    *ppfVar2 = (float *)((int)*ppfVar2 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Normal);
    ppuVar3 = &(meshBuilder->m_VertexBuilder).m_pCurrColor;
    *ppuVar3 = *ppuVar3 + (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Color;
    ppfVar2 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord;
    *ppfVar2 = (float *)((int)*ppfVar2 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[0]);
    ppfVar2 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 1;
    *ppfVar2 = (float *)((int)*ppfVar2 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[1]);
    ppfVar2 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 2;
    *ppfVar2 = (float *)((int)*ppfVar2 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[2]);
    ppfVar2 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 3;
    *ppfVar2 = (float *)((int)*ppfVar2 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[3]);
    ppfVar2 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 4;
    *ppfVar2 = (float *)((int)*ppfVar2 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[4]);
    ppfVar2 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 5;
    *ppfVar2 = (float *)((int)*ppfVar2 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[5]);
    ppfVar2 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 6;
    *ppfVar2 = (float *)((int)*ppfVar2 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[6]);
    ppfVar2 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 7;
    *ppfVar2 = (float *)((int)*ppfVar2 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[7]);
    i = i + 1;
    local_34 = local_34 + 8;
  } while (i != 0x33);
  local_4c = (short)iVar14;
  puVar15 = (undefined4 *)(unaff_EBX + 0x70c9a2);
                    /* Unresolved local var: int i@[???] */
  do {
    (meshBuilder->m_IndexBuilder).super_IndexDesc_t.m_pIndices
    [(meshBuilder->m_IndexBuilder).m_nCurrentIndex] =
         local_4c + (short)*puVar15 + (short)(meshBuilder->m_IndexBuilder).m_nIndexOffset;
    iVar14 = (meshBuilder->m_IndexBuilder).super_IndexDesc_t.m_nIndexSize +
             (meshBuilder->m_IndexBuilder).m_nCurrentIndex;
    (meshBuilder->m_IndexBuilder).m_nCurrentIndex = iVar14;
    (meshBuilder->m_IndexBuilder).m_nIndexCount = iVar14;
    (meshBuilder->m_IndexBuilder).super_IndexDesc_t.m_pIndices[iVar14] =
         local_4c + (short)puVar15[1] + (short)(meshBuilder->m_IndexBuilder).m_nIndexOffset;
    iVar14 = (meshBuilder->m_IndexBuilder).super_IndexDesc_t.m_nIndexSize +
             (meshBuilder->m_IndexBuilder).m_nCurrentIndex;
    (meshBuilder->m_IndexBuilder).m_nCurrentIndex = iVar14;
    (meshBuilder->m_IndexBuilder).m_nIndexCount = iVar14;
    (meshBuilder->m_IndexBuilder).super_IndexDesc_t.m_pIndices[iVar14] =
         local_4c + (short)puVar15[2] + (short)(meshBuilder->m_IndexBuilder).m_nIndexOffset;
    iVar14 = (meshBuilder->m_IndexBuilder).super_IndexDesc_t.m_nIndexSize +
             (meshBuilder->m_IndexBuilder).m_nCurrentIndex;
    (meshBuilder->m_IndexBuilder).m_nCurrentIndex = iVar14;
    (meshBuilder->m_IndexBuilder).m_nIndexCount = iVar14;
    puVar15 = puVar15 + 3;
  } while (puVar15 != (undefined4 *)(unaff_EBX + 0x70cd92));
  return;
}


/* NPaintRenderer::Paintblob_DrawSpheres at 00521de0 */

void NPaintRenderer::Paintblob_DrawSpheres(IMaterial *pMaterial,float flRadius)

{
  float fVar1;
  float fVar2;
  float fVar3;
  int iVar4;
  float *pfVar5;
  IMesh *pIVar6;
  int iVar7;
  Vector *center;
  int unaff_EBX;
  int iVar8;
  PaintBlob_DrawSpheres_sortParticles_t *pPVar9;
  float fVar10;
  double dVar11;
  float fVar12;
  float fVar13;
  int local_248;
  PaintBlob_DrawSpheres_sortParticles_t *local_238;
  int local_234;
  CMeshBuilder local_224;
  float local_3c;
  float local_38;
  float local_34;
  undefined4 local_30;
  int *local_20;
  
                    /* Unresolved local var: Point3D eye@[???]
                       Unresolved local var:
                       SmartArray<NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t,false,16>
                       sort_particles@[???]
                       Unresolved local var: CMatRenderContextPtr pRenderContext@[???]
                       Unresolved local var: IMesh * pMesh@[???]
                       Unresolved local var: int vertMax@[???]
                       Unresolved local var: int j@[???] */
  ___i686_get_pc_thunk_bx();
  iVar4 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x642053) + 0x34))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x642053));
  fVar1 = *(float *)(iVar4 + 0x60);
  fVar2 = *(float *)(iVar4 + 100);
  fVar3 = *(float *)(iVar4 + 0x68);
  local_238 = _malloc(0x80);
  iVar4 = *(int *)(unaff_EBX + 0x846e53);
  if (0x10 < iVar4) {
    local_238 = _realloc(local_238,iVar4 << 4);
    iVar4 = *(int *)(unaff_EBX + 0x846e53);
  }
  if (0 < iVar4) {
    local_30 = 0;
    iVar7 = 0;
    iVar8 = 0;
    do {
      local_238[iVar7].no = iVar7;
      pfVar5 = (float *)(iVar8 + *(int *)(unaff_EBX + 0x846e47));
                    /* Unresolved local var: float sum@[???]
                       Unresolved local var: float temp@[???] */
      fVar12 = *pfVar5 - fVar1;
      fVar13 = pfVar5[1] - fVar2;
      fVar10 = pfVar5[2] - fVar3;
      local_238[iVar7].dist = SQRT(fVar10 * fVar10 + fVar12 * fVar12 + fVar13 * fVar13);
      iVar7 = iVar7 + 1;
      iVar8 = iVar8 + 0x60;
      local_3c = fVar1;
      local_38 = fVar2;
      local_34 = fVar3;
    } while (iVar7 < *(int *)(unaff_EBX + 0x846e53));
  }
  if (iVar4 == 2) {
    fVar1 = local_238->dist;
    iVar4 = local_238->no;
    iVar7 = local_238[1].no;
    if (fVar1 < local_238[1].dist) {
      local_238->dist = local_238[1].dist;
      local_238->no = iVar7;
      local_238[1].dist = fVar1;
      local_238[1].no = iVar4;
    }
  }
  else if (2 < iVar4) {
    Helper::
    sort<NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t,_NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t::C>
              (local_238,0,iVar4 + -1);
  }
  local_20 = (int *)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x64211b) + 0x1a0))
                              ((int *)**(undefined4 **)(unaff_EBX + 0x64211b));
  if (local_20 != (int *)0x0) {
    (**(code **)(*local_20 + 8))(local_20);
  }
  (**(code **)(*local_20 + 0x50))(local_20,10);
  (**(code **)(*local_20 + 0x24))(local_20,pMaterial,0);
  pIVar6 = (IMesh *)(**(code **)(*local_20 + 0xe4))(local_20,1,0,0,0);
  local_248 = 0;
  do {
    if (*(int *)(unaff_EBX + 0x846e53) <= local_248) {
      if (local_20 != (int *)0x0) {
        (**(code **)(*local_20 + 0xc))(local_20);
      }
      if (local_20 != (int *)0x0) {
        (**(code **)(*local_20 + 4))(local_20);
      }
      _free(local_238);
      return;
    }
    local_234 = *(int *)(unaff_EBX + 0x846e53) - local_248;
    if (local_234 < 0x82) {
      iVar4 = local_234 * 0xfc;
      local_234 = local_234 * 0x33;
    }
    else {
      iVar4 = 0x7ff8;
      local_234 = 0x19e6;
    }
    local_224.m_IndexBuilder.m_pIndexBuffer = (IIndexBuffer *)0x0;
    local_224.m_IndexBuilder.m_nMaxIndexCount = 0;
    local_224.m_IndexBuilder.m_nIndexCount = 0;
    local_224.m_IndexBuilder.m_nCurrentIndex = 0;
    local_224.m_IndexBuilder.m_nTotalIndexCount = 0;
    local_224.m_IndexBuilder.m_nBufferOffset = 0xffffffff;
    local_224.m_IndexBuilder.m_nBufferFirstIndex = 0;
    local_224.m_VertexBuilder.m_pVertexBuffer = (IVertexBuffer *)0x0;
    local_224.m_VertexBuilder.m_nBufferOffset = 0xffffffff;
    local_224.m_VertexBuilder.m_nBufferFirstVertex = 0;
    local_224.m_VertexBuilder.m_nVertexCount = 0;
    local_224.m_VertexBuilder.m_nCurrentVertex = 0;
    local_224.m_VertexBuilder.m_nMaxVertexCount = 0;
    local_224.m_VertexBuilder.m_nTotalVertexCount = 0;
    local_224.m_VertexBuilder.super_VertexDesc_t.m_CompressionType = VERTEX_COMPRESSION_INVALID;
    local_224.m_bGenerateIndices = false;
    local_224.m_Type = MATERIAL_TRIANGLES;
    local_224.m_pMesh = pIVar6;
    (*(pIVar6->super_IVertexBuffer)._vptr_IVertexBuffer[10])(pIVar6,2);
    (*((local_224.m_pMesh)->super_IVertexBuffer)._vptr_IVertexBuffer[0x11])
              (local_224.m_pMesh,local_234,iVar4,&local_224,0);
    local_224.m_IndexBuilder.m_pIndexBuffer = (IIndexBuffer *)0x0;
    if (pIVar6 != (IMesh *)0x0) {
      local_224.m_IndexBuilder.m_pIndexBuffer = &pIVar6->super_IIndexBuffer;
    }
    local_224.m_IndexBuilder.m_nIndexCount = 0;
    local_224.m_IndexBuilder.m_bModify = false;
    local_224.m_IndexBuilder.m_nIndexOffset =
         local_224.super_MeshDesc_t.super_VertexDesc_t.m_nFirstVertex;
    local_224.m_IndexBuilder.super_IndexDesc_t.m_pIndices =
         local_224.super_MeshDesc_t.super_IndexDesc_t.m_pIndices;
    local_224.m_IndexBuilder.super_IndexDesc_t.m_nIndexSize =
         local_224.super_MeshDesc_t.super_IndexDesc_t.m_nIndexSize;
    local_224.m_IndexBuilder.m_nCurrentIndex = 0;
    local_224.m_IndexBuilder.m_nMaxIndexCount = iVar4;
    local_224.m_VertexBuilder.m_pVertexBuffer = (IVertexBuffer *)pIVar6;
    _memcpy(&local_224.m_VertexBuilder,&local_224,0xa4);
    local_224.m_VertexBuilder.m_nMaxVertexCount = local_234;
    local_224.m_VertexBuilder.super_VertexDesc_t.m_NumBoneWeights =
         ~-(uint)(local_224.m_VertexBuilder.super_VertexDesc_t.m_NumBoneWeights == 0) & 2;
    local_224.m_VertexBuilder.m_nVertexCount = 0;
    local_224.m_VertexBuilder.m_bModify = false;
    if (local_224.m_VertexBuilder.m_nBufferOffset == 0xffffffff) {
      local_224.m_VertexBuilder.m_nTotalVertexCount = 0;
      local_224.m_VertexBuilder.m_nBufferOffset =
           local_224.super_MeshDesc_t.super_VertexDesc_t.m_nOffset;
      local_224.m_VertexBuilder.m_nBufferFirstVertex =
           local_224.super_MeshDesc_t.super_VertexDesc_t.m_nFirstVertex;
    }
    local_224.m_IndexBuilder.m_nCurrentIndex = 0;
    local_224.m_VertexBuilder.m_nCurrentVertex = 0;
    local_224.m_VertexBuilder.m_pCurrPosition =
         local_224.m_VertexBuilder.super_VertexDesc_t.m_pPosition;
    local_224.m_VertexBuilder.m_pCurrNormal = local_224.m_VertexBuilder.super_VertexDesc_t.m_pNormal
    ;
                    /* Unresolved local var: int i@[???] */
    local_224.m_VertexBuilder.m_pCurrTexCoord[0] =
         local_224.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[0];
    local_224.m_VertexBuilder.m_pCurrTexCoord[1] =
         local_224.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[1];
    local_224.m_VertexBuilder.m_pCurrTexCoord[2] =
         local_224.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[2];
    local_224.m_VertexBuilder.m_pCurrTexCoord[3] =
         local_224.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[3];
    local_224.m_VertexBuilder.m_pCurrTexCoord[4] =
         local_224.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[4];
    local_224.m_VertexBuilder.m_pCurrTexCoord[5] =
         local_224.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[5];
    local_224.m_VertexBuilder.m_pCurrTexCoord[6] =
         local_224.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[6];
    local_224.m_VertexBuilder.m_pCurrTexCoord[7] =
         local_224.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[7];
    local_224.m_VertexBuilder.m_pCurrColor = local_224.m_VertexBuilder.super_VertexDesc_t.m_pColor;
    pPVar9 = local_238 + local_248;
    iVar4 = 0;
    do {
      if (*(int *)(unaff_EBX + 0x846e53) <= local_248) break;
      center = (Vector *)(pPVar9->no * 0x60 + *(int *)(unaff_EBX + 0x846e47));
      dVar11 = (double)center[1].y;
      if (*(double *)(unaff_EBX + 0x4c0647) <= dVar11 && dVar11 != *(double *)(unaff_EBX + 0x4c0647)
         ) {
        Paintblob_DrawFastSphere
                  (&local_224,center,center[1].y * flRadius,center[2].z,center[3].x,center[3].y);
        iVar4 = iVar4 + 1;
      }
      local_248 = local_248 + 1;
      pPVar9 = pPVar9 + 1;
    } while (iVar4 < 0x82);
    iVar4 = local_224.m_IndexBuilder.m_nIndexCount;
    if (local_224.m_bGenerateIndices != false) {
      if (local_224.m_Type - MATERIAL_LINE_STRIP < 7) {
                    /* WARNING: Could not recover jumptable at 0x005222cb. Too many branches */
                    /* WARNING: Treating indirect jump as call */
        (*(code *)(*(int *)(unaff_EBX + 0x4df + (local_224.m_Type - MATERIAL_LINE_STRIP) * 4) +
                  unaff_EBX))();
        return;
      }
      if (local_224.m_IndexBuilder.super_IndexDesc_t.m_nIndexSize != 0) {
        iVar7 = local_224.m_IndexBuilder.m_nMaxIndexCount - local_224.m_IndexBuilder.m_nCurrentIndex
        ;
        if (local_224.m_VertexBuilder.m_nVertexCount <=
            local_224.m_IndexBuilder.m_nMaxIndexCount - local_224.m_IndexBuilder.m_nCurrentIndex) {
          iVar7 = local_224.m_VertexBuilder.m_nVertexCount;
        }
        if (iVar7 != 0) {
          if (local_224.m_Type < MATERIAL_HETEROGENOUS) {
                    /* WARNING: Could not recover jumptable at 0x005224d6. Too many branches */
                    /* WARNING: Treating indirect jump as call */
            (*(code *)(*(int *)(unaff_EBX + 0x6e7 + local_224.m_Type * 4) + unaff_EBX))();
            return;
          }
          GenerateSequentialIndexBuffer
                    (local_224.m_IndexBuilder.super_IndexDesc_t.m_pIndices +
                     local_224.m_IndexBuilder.m_nCurrentIndex,iVar7,
                     local_224.m_IndexBuilder.m_nIndexOffset);
          local_224.m_IndexBuilder.m_nCurrentIndex =
               iVar7 * local_224.m_IndexBuilder.super_IndexDesc_t.m_nIndexSize +
               local_224.m_IndexBuilder.m_nCurrentIndex;
          iVar4 = local_224.m_IndexBuilder.m_nCurrentIndex;
          if (local_224.m_IndexBuilder.m_nCurrentIndex <= local_224.m_IndexBuilder.m_nIndexCount) {
            iVar4 = local_224.m_IndexBuilder.m_nIndexCount;
          }
        }
      }
    }
    local_224.m_IndexBuilder.m_nIndexCount = iVar4;
    (*((local_224.m_pMesh)->super_IVertexBuffer)._vptr_IVertexBuffer[0x14])
              (local_224.m_pMesh,local_224.m_VertexBuilder.m_nVertexCount,
               local_224.m_IndexBuilder.m_nIndexCount,&local_224);
    local_224.m_IndexBuilder.m_pIndexBuffer = (IIndexBuffer *)0x0;
    local_224.m_IndexBuilder.m_nMaxIndexCount = 0;
    local_224.m_VertexBuilder.m_nMaxVertexCount = 0;
    local_224.m_VertexBuilder.m_pVertexBuffer = (IVertexBuffer *)0x0;
    local_224.m_VertexBuilder.super_VertexDesc_t.m_CompressionType = VERTEX_COMPRESSION_INVALID;
    local_224.m_pMesh = (IMesh *)0x0;
    (*(pIVar6->super_IVertexBuffer)._vptr_IVertexBuffer[0xb])(pIVar6,0xffffffff,0);
    if ((local_224.m_VertexBuilder.m_pVertexBuffer != (IVertexBuffer *)0x0) &&
       (iVar4 = (*(local_224.m_VertexBuilder.m_pVertexBuffer)->_vptr_IVertexBuffer[2])
                          (local_224.m_VertexBuilder.m_pVertexBuffer), (char)iVar4 != '\0')) {
      (*(local_224.m_VertexBuilder.m_pVertexBuffer)->_vptr_IVertexBuffer[4])
                (local_224.m_VertexBuilder.m_pVertexBuffer);
    }
    if ((local_224.m_IndexBuilder.m_pIndexBuffer != (IIndexBuffer *)0x0) &&
       (iVar4 = (*(local_224.m_IndexBuilder.m_pIndexBuffer)->_vptr_IIndexBuffer[2])
                          (local_224.m_IndexBuilder.m_pIndexBuffer), (char)iVar4 != '\0')) {
      (*(local_224.m_IndexBuilder.m_pIndexBuffer)->_vptr_IIndexBuffer[4])
                (local_224.m_IndexBuilder.m_pIndexBuffer);
    }
  } while( true );
}


/* NPaintRenderer::Paintblob_SafeLightCubeUpdate at 0051fd70 */

/* WARNING: Restarted to delay deadcode elimination for space: stack */

void NPaintRenderer::Paintblob_SafeLightCubeUpdate
               (Vector *vecRenderOrigin,Vector4D *cachedCubeColours)

{
  float *pfVar1;
  char cVar2;
  Vector4D *pVVar3;
  Vector4D *pVVar4;
  int iVar5;
  Vector4D *pVVar6;
  int unaff_EBX;
  Vector4D *pVVar7;
  Vector4D *pVVar8;
  float fVar9;
  double dVar10;
  double dVar11;
  undefined8 in_stack_ffffff30;
  undefined4 uVar12;
  bool invalidOrigin;
  float local_88;
  float local_84;
  float local_80;
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
  float local_28;
  float local_24;
  float local_20;
  
                    /* Unresolved local var: Vector[6] boxColors@[???]
                       Unresolved local var: bool bBadLightCube@[???] */
  uVar12 = (undefined4)((ulonglong)in_stack_ffffff30 >> 0x20);
  ___i686_get_pc_thunk_bx();
  dVar10 = (double)CONCAT44(uVar12,&local_88);
  uVar12 = 0;
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x644023) + 0x220))
            ((int *)**(undefined4 **)(unaff_EBX + 0x644023),vecRenderOrigin,0,&local_88);
  cVar2 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x644057) + 0x4c))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x644057),vecRenderOrigin);
                    /* Unresolved local var: Convertor_t tmp@[???] */
                    /* Unresolved local var: Convertor_t tmp@[???] */
                    /* Unresolved local var: Convertor_t tmp@[???] */
  if ((((((((uint)local_88 & 0x7f800000) == 0x7f800000) ||
         (((uint)local_84 & 0x7f800000) == 0x7f800000)) ||
        (((uint)local_80 & 0x7f800000) == 0x7f800000)) ||
       ((((uint)local_7c & 0x7f800000) == 0x7f800000 ||
        (((uint)local_78 & 0x7f800000) == 0x7f800000)))) ||
      (((((uint)local_74 & 0x7f800000) == 0x7f800000 ||
        ((((uint)local_70 & 0x7f800000) == 0x7f800000 ||
         (((uint)local_6c & 0x7f800000) == 0x7f800000)))) ||
       (((uint)local_68 & 0x7f800000) == 0x7f800000)))) ||
     ((((((uint)local_64 & 0x7f800000) == 0x7f800000 ||
        (((uint)local_60 & 0x7f800000) == 0x7f800000)) ||
       (((uint)local_5c & 0x7f800000) == 0x7f800000)) ||
      ((((((uint)local_58 & 0x7f800000) == 0x7f800000 ||
         (((uint)local_54 & 0x7f800000) == 0x7f800000)) ||
        ((((uint)local_50 & 0x7f800000) == 0x7f800000 ||
         ((((uint)local_4c & 0x7f800000) == 0x7f800000 ||
          (((uint)local_48 & 0x7f800000) == 0x7f800000)))))) ||
       (((uint)local_44 & 0x7f800000) == 0x7f800000)))))) {
    iVar5 = unaff_EBX + 0x499c87;
    if (cVar2 == '\0') {
      iVar5 = unaff_EBX + 0x468ee3;
    }
    DevWarning((char *)(unaff_EBX + 0x499cb3),iVar5);
    return;
  }
  if ((((local_88 < *(float *)(unaff_EBX + 0x4c315b)) &&
       (!NAN(local_88) && !NAN(*(float *)(unaff_EBX + 0x4c315b)))) ||
      (local_84 < *(float *)(unaff_EBX + 0x4c315b))) ||
     (local_80 < *(float *)(unaff_EBX + 0x4c315b))) {
    iVar5 = unaff_EBX + 0x468ee3;
    if (cVar2 != '\0') {
      iVar5 = unaff_EBX + 0x499c87;
    }
    dVar10 = (double)local_84;
    dVar11 = (double)local_88;
    DevWarning((char *)(unaff_EBX + 0x499ce7),dVar11,dVar10,(double)local_80,iVar5);
    uVar12 = (undefined4)((ulonglong)dVar11 >> 0x20);
  }
  local_2c = *(float *)(**(int **)(unaff_EBX + 0x64401b) + 0x10);
  local_2c = local_2c + local_2c;
  if (*(float *)(unaff_EBX + 0x4c2403) <= local_2c && local_2c != *(float *)(unaff_EBX + 0x4c2403))
  {
    local_2c = *(float *)(unaff_EBX + 0x4c2403);
  }
  else if ((local_2c < *(float *)(unaff_EBX + 0x4c24ff)) &&
          (!NAN(local_2c) && !NAN(*(float *)(unaff_EBX + 0x4c24ff)))) {
    local_2c = 0.0;
  }
  local_34 = (local_88 - cachedCubeColours->x) * local_2c;
  local_30 = (local_84 - cachedCubeColours->y) * local_2c;
  local_2c = (local_80 - cachedCubeColours->z) * local_2c;
  local_40 = cachedCubeColours->x + local_34;
  local_3c = cachedCubeColours->y + local_30;
  local_38 = cachedCubeColours->z + local_2c;
  cachedCubeColours->x = local_40;
  cachedCubeColours->y = local_3c;
  cachedCubeColours->z = local_38;
  fVar9 = *(float *)(unaff_EBX + 0x4c315b);
  if (((local_7c < fVar9) || (local_78 < fVar9)) || (local_74 < fVar9)) {
    iVar5 = unaff_EBX + 0x468ee3;
    if (cVar2 != '\0') {
      iVar5 = unaff_EBX + 0x499c87;
    }
    dVar10 = (double)local_78;
    dVar11 = (double)local_7c;
    local_28 = local_40;
    local_24 = local_3c;
    local_20 = local_38;
    DevWarning((char *)(unaff_EBX + 0x499ce7),dVar11,dVar10,(double)local_74,iVar5);
    uVar12 = (undefined4)((ulonglong)dVar11 >> 0x20);
  }
  pVVar3 = cachedCubeColours + 1;
  local_2c = *(float *)(**(int **)(unaff_EBX + 0x64401b) + 0x10);
  local_2c = local_2c + local_2c;
  if (*(float *)(unaff_EBX + 0x4c2403) <= local_2c && local_2c != *(float *)(unaff_EBX + 0x4c2403))
  {
    local_2c = *(float *)(unaff_EBX + 0x4c2403);
  }
  else if ((local_2c < *(float *)(unaff_EBX + 0x4c24ff)) &&
          (!NAN(local_2c) && !NAN(*(float *)(unaff_EBX + 0x4c24ff)))) {
    local_2c = 0.0;
  }
  local_34 = (local_7c - pVVar3->x) * local_2c;
  local_30 = (local_78 - cachedCubeColours[1].y) * local_2c;
  local_2c = (local_74 - cachedCubeColours[1].z) * local_2c;
  local_40 = pVVar3->x + local_34;
  local_3c = cachedCubeColours[1].y + local_30;
  local_38 = cachedCubeColours[1].z + local_2c;
  pVVar3->x = local_40;
  cachedCubeColours[1].y = local_3c;
  cachedCubeColours[1].z = local_38;
  fVar9 = *(float *)(unaff_EBX + 0x4c315b);
  if (((local_70 < fVar9) || (local_6c < fVar9)) || (local_68 < fVar9)) {
    iVar5 = unaff_EBX + 0x468ee3;
    if (cVar2 != '\0') {
      iVar5 = unaff_EBX + 0x499c87;
    }
    dVar10 = (double)local_6c;
    dVar11 = (double)local_70;
    local_28 = local_40;
    local_24 = local_3c;
    local_20 = local_38;
    DevWarning((char *)(unaff_EBX + 0x499ce7),dVar11,dVar10,(double)local_68,iVar5);
    uVar12 = (undefined4)((ulonglong)dVar11 >> 0x20);
  }
  pVVar4 = cachedCubeColours + 2;
  local_2c = *(float *)(**(int **)(unaff_EBX + 0x64401b) + 0x10);
  local_2c = local_2c + local_2c;
  if (*(float *)(unaff_EBX + 0x4c2403) <= local_2c && local_2c != *(float *)(unaff_EBX + 0x4c2403))
  {
    local_2c = *(float *)(unaff_EBX + 0x4c2403);
  }
  else if ((local_2c < *(float *)(unaff_EBX + 0x4c24ff)) &&
          (!NAN(local_2c) && !NAN(*(float *)(unaff_EBX + 0x4c24ff)))) {
    local_2c = 0.0;
  }
  local_34 = (local_70 - pVVar4->x) * local_2c;
  local_30 = (local_6c - cachedCubeColours[2].y) * local_2c;
  local_2c = (local_68 - cachedCubeColours[2].z) * local_2c;
  local_40 = pVVar4->x + local_34;
  local_3c = cachedCubeColours[2].y + local_30;
  local_38 = cachedCubeColours[2].z + local_2c;
  pVVar4->x = local_40;
  cachedCubeColours[2].y = local_3c;
  cachedCubeColours[2].z = local_38;
  fVar9 = *(float *)(unaff_EBX + 0x4c315b);
  if (((local_64 < fVar9) || (local_60 < fVar9)) || (local_5c < fVar9)) {
    iVar5 = unaff_EBX + 0x468ee3;
    if (cVar2 != '\0') {
      iVar5 = unaff_EBX + 0x499c87;
    }
    dVar10 = (double)local_60;
    dVar11 = (double)local_64;
    local_28 = local_40;
    local_24 = local_3c;
    local_20 = local_38;
    DevWarning((char *)(unaff_EBX + 0x499ce7),dVar11,dVar10,(double)local_5c,iVar5);
    uVar12 = (undefined4)((ulonglong)dVar11 >> 0x20);
  }
  pVVar8 = cachedCubeColours + 3;
  local_2c = *(float *)(**(int **)(unaff_EBX + 0x64401b) + 0x10);
  local_2c = local_2c + local_2c;
  if (*(float *)(unaff_EBX + 0x4c2403) <= local_2c && local_2c != *(float *)(unaff_EBX + 0x4c2403))
  {
    local_2c = *(float *)(unaff_EBX + 0x4c2403);
  }
  else if ((local_2c < *(float *)(unaff_EBX + 0x4c24ff)) &&
          (!NAN(local_2c) && !NAN(*(float *)(unaff_EBX + 0x4c24ff)))) {
    local_2c = 0.0;
  }
  local_34 = (local_64 - pVVar8->x) * local_2c;
  local_30 = (local_60 - cachedCubeColours[3].y) * local_2c;
  local_2c = (local_5c - cachedCubeColours[3].z) * local_2c;
  local_40 = pVVar8->x + local_34;
  local_3c = cachedCubeColours[3].y + local_30;
  local_38 = cachedCubeColours[3].z + local_2c;
  pVVar8->x = local_40;
  cachedCubeColours[3].y = local_3c;
  cachedCubeColours[3].z = local_38;
  fVar9 = *(float *)(unaff_EBX + 0x4c315b);
  if (((local_58 < fVar9) || (local_54 < fVar9)) || (local_50 < fVar9)) {
    iVar5 = unaff_EBX + 0x468ee3;
    if (cVar2 != '\0') {
      iVar5 = unaff_EBX + 0x499c87;
    }
    dVar10 = (double)local_54;
    dVar11 = (double)local_58;
    local_28 = local_40;
    local_24 = local_3c;
    local_20 = local_38;
    DevWarning((char *)(unaff_EBX + 0x499ce7),dVar11,dVar10,(double)local_50,iVar5);
    uVar12 = (undefined4)((ulonglong)dVar11 >> 0x20);
  }
  pVVar7 = cachedCubeColours + 4;
  local_2c = *(float *)(**(int **)(unaff_EBX + 0x64401b) + 0x10);
  local_2c = local_2c + local_2c;
  if (*(float *)(unaff_EBX + 0x4c2403) <= local_2c && local_2c != *(float *)(unaff_EBX + 0x4c2403))
  {
    local_2c = *(float *)(unaff_EBX + 0x4c2403);
  }
  else if ((local_2c < *(float *)(unaff_EBX + 0x4c24ff)) &&
          (!NAN(local_2c) && !NAN(*(float *)(unaff_EBX + 0x4c24ff)))) {
    local_2c = 0.0;
  }
  local_34 = (local_58 - pVVar7->x) * local_2c;
  local_30 = (local_54 - cachedCubeColours[4].y) * local_2c;
  local_2c = (local_50 - cachedCubeColours[4].z) * local_2c;
  local_40 = pVVar7->x + local_34;
  local_3c = cachedCubeColours[4].y + local_30;
  local_38 = cachedCubeColours[4].z + local_2c;
  pVVar7->x = local_40;
  cachedCubeColours[4].y = local_3c;
  cachedCubeColours[4].z = local_38;
  if (((local_4c < *(float *)(unaff_EBX + 0x4c315b)) ||
      (local_48 < *(float *)(unaff_EBX + 0x4c315b))) ||
     (local_44 < *(float *)(unaff_EBX + 0x4c315b))) {
    iVar5 = unaff_EBX + 0x499c87;
    if (cVar2 == '\0') {
      iVar5 = unaff_EBX + 0x468ee3;
    }
    dVar10 = (double)local_48;
    dVar11 = (double)local_4c;
    local_28 = local_40;
    local_24 = local_3c;
    local_20 = local_38;
    DevWarning((char *)(unaff_EBX + 0x499ce7),dVar11,dVar10,(double)local_44,iVar5);
    uVar12 = (undefined4)((ulonglong)dVar11 >> 0x20);
  }
  pVVar6 = cachedCubeColours + 5;
  fVar9 = *(float *)(**(int **)(unaff_EBX + 0x64401b) + 0x10);
  fVar9 = fVar9 + fVar9;
  pfVar1 = (float *)(unaff_EBX + 0x4c2403);
  if (fVar9 < *pfVar1 || fVar9 == *pfVar1) {
    local_2c = 0.0;
    if (0.0 <= fVar9) {
      local_2c = fVar9;
    }
  }
  else {
    local_2c = *pfVar1;
  }
  local_34 = (local_4c - pVVar6->x) * local_2c;
  local_30 = (local_48 - cachedCubeColours[5].y) * local_2c;
  local_2c = (local_44 - cachedCubeColours[5].z) * local_2c;
  local_40 = pVVar6->x + local_34;
  local_3c = cachedCubeColours[5].y + local_30;
  local_38 = cachedCubeColours[5].z + local_2c;
  pVVar6->x = local_40;
  cachedCubeColours[5].y = local_3c;
  cachedCubeColours[5].z = local_38;
  if (((((((((uint)cachedCubeColours->x & 0x7f800000) != 0x7f800000) &&
          (((uint)cachedCubeColours->y & 0x7f800000) != 0x7f800000)) &&
         (((uint)cachedCubeColours->z & 0x7f800000) != 0x7f800000)) &&
        ((fVar9 = *(float *)(&DAT_004c2c7f + unaff_EBX), fVar9 <= cachedCubeColours->x &&
         (fVar9 <= cachedCubeColours->y)))) &&
       ((fVar9 <= cachedCubeColours->z &&
        ((((uint)pVVar3->x & 0x7f800000) != 0x7f800000 &&
         (((uint)cachedCubeColours[1].y & 0x7f800000) != 0x7f800000)))))) &&
      ((((uint)cachedCubeColours[1].z & 0x7f800000) != 0x7f800000 &&
       ((((((fVar9 <= pVVar3->x && (fVar9 <= cachedCubeColours[1].y)) &&
           (fVar9 <= cachedCubeColours[1].z)) &&
          ((((uint)pVVar4->x & 0x7f800000) != 0x7f800000 &&
           (((uint)cachedCubeColours[2].y & 0x7f800000) != 0x7f800000)))) &&
         ((((uint)cachedCubeColours[2].z & 0x7f800000) != 0x7f800000 &&
          ((fVar9 <= pVVar4->x && (fVar9 <= cachedCubeColours[2].y)))))) &&
        (fVar9 <= cachedCubeColours[2].z)))))) &&
     ((((((((uint)pVVar8->x & 0x7f800000) != 0x7f800000 &&
          (((uint)cachedCubeColours[3].y & 0x7f800000) != 0x7f800000)) &&
         (((uint)cachedCubeColours[3].z & 0x7f800000) != 0x7f800000)) &&
        (((fVar9 <= pVVar8->x && (fVar9 <= cachedCubeColours[3].y)) &&
         ((fVar9 <= cachedCubeColours[3].z &&
          ((((uint)pVVar7->x & 0x7f800000) != 0x7f800000 &&
           (((uint)cachedCubeColours[4].y & 0x7f800000) != 0x7f800000)))))))) &&
       (((uint)cachedCubeColours[4].z & 0x7f800000) != 0x7f800000)) &&
      ((((((fVar9 <= pVVar7->x && (fVar9 <= cachedCubeColours[4].y)) &&
          (fVar9 <= cachedCubeColours[4].z)) &&
         ((((uint)local_40 & 0x7f800000) != 0x7f800000 &&
          (((uint)local_3c & 0x7f800000) != 0x7f800000)))) &&
        (((uint)local_38 & 0x7f800000) != 0x7f800000)) &&
       (((fVar9 <= local_40 && (fVar9 <= local_3c)) && (fVar9 <= local_38)))))))) {
    return;
  }
  iVar5 = unaff_EBX + 0x499c87;
  if (cVar2 == '\0') {
    iVar5 = unaff_EBX + 0x468ee3;
  }
  local_28 = local_40;
  local_24 = local_3c;
  local_20 = local_38;
  DevWarning((char *)(unaff_EBX + 0x499d17),iVar5,uVar12,dVar10);
  cachedCubeColours->x = 0.0;
  return;
}


/* NPaintRenderer::GetDrawMaterial at 0051f680 */

IMaterial * NPaintRenderer::GetDrawMaterial(void)

{
  IMaterial *pIVar1;
  uint uVar2;
  char *pcVar3;
  int iVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((*(int *)(*(int *)(unaff_EBX + 0x8491a8) + 0x30) == 0) &&
     (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x644b08) + 0x1c) + 0x30) == 0)) {
    uVar2 = *(uint *)(unaff_EBX + 0x849260) & 0x1000;
    if (uVar2 == 0) {
      pcVar3 = *(char **)(*(int *)(unaff_EBX + 0x849268) + 0x24);
      if (pcVar3 == (char *)0x0) {
        pcVar3 = (char *)(unaff_EBX + 0x4695d0);
      }
    }
    else {
      pcVar3 = (char *)(unaff_EBX + 0x493cc2);
    }
    if (*pcVar3 == '\0') {
      pIVar1 = (IMaterial *)0x0;
      if (*(uint *)(*(int *)(unaff_EBX + 0x849208) + 0x30) < 0xc) {
                    /* WARNING: Could not recover jumptable at 0x0051f80c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
        pIVar1 = (IMaterial *)
                 (*(code *)(*(int *)(unaff_EBX + 0x17c +
                                    *(int *)(*(int *)(unaff_EBX + 0x849208) + 0x30) * 4) + unaff_EBX
                           ))();
        return pIVar1;
      }
    }
    else {
                    /* Unresolved local var: char * str@[???] */
      iVar4 = unaff_EBX + 0x493cc2;
      if (uVar2 == 0) {
        iVar4 = *(int *)(*(int *)(unaff_EBX + 0x849268) + 0x24);
        if (iVar4 == 0) {
          iVar4 = unaff_EBX + 0x4695d0;
        }
      }
      pIVar1 = (IMaterial *)
               (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x644878) + 0x130))
                         ((int *)**(undefined4 **)(unaff_EBX + 0x644878),iVar4,unaff_EBX + 0x487f80,
                          1,0);
    }
  }
  else if (*(int *)(*(int *)(unaff_EBX + 0x849208) + 0x30) < 0) {
    pIVar1 = (IMaterial *)
             (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x644878) + 0x130))
                       ((int *)**(undefined4 **)(unaff_EBX + 0x644878),unaff_EBX + 0x499080,
                        unaff_EBX + 0x487f80,1,0);
  }
  else {
    pIVar1 = (IMaterial *)
             (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x644878) + 0x130))
                       ((int *)**(undefined4 **)(unaff_EBX + 0x644878),unaff_EBX + 0x49a1e4,
                        unaff_EBX + 0x487f80,1,0);
  }
  return pIVar1;
}


/* NPaintRenderer::SetUpImpRendererUserDefinedFuncs at 0051fb10 */

void NPaintRenderer::SetUpImpRendererUserDefinedFuncs(void)

{
  return;
}


/* NPaintRenderer::TransformParticles at 00520ba0 */

/* WARNING: Restarted to delay deadcode elimination for space: stack */

void NPaintRenderer::TransformParticles
               (float angle,Vector *center,Vector *transformedCenter,Vector *transformedEye,
               VMatrix *rotationMatrix,VMatrix *invRotationMatrix,
               ImpParticleWithFourInterpolants *blobParticles,int blobCount)

{
  float fVar1;
  int iVar2;
  int unaff_EBX;
  uint uVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  float fVar10;
  int i;
  float l;
  
  ___i686_get_pc_thunk_bx();
  if (0 < blobCount) {
    iVar2 = *(int *)(unaff_EBX + 0x643a03);
    i = 0;
    fVar1 = *(float *)(unaff_EBX + 0x4c169b);
    do {
                    /* Unresolved local var: ImpParticleWithFourInterpolants * imp_particle@[???]
                       Unresolved local var: Vector vParticle@[???]
                       Unresolved local var: Vector transformedParticle@[???]
                       Unresolved local var: Point3D pParticle@[???]
                       Unresolved local var: Point3D pCenter@[???]
                       Unresolved local var: Point3D vec@[???] */
                    /* Unresolved local var: Vector res@[???] */
      fVar4 = (blobParticles->super_ImpParticleWithTwoInterpolants).
              super_ImpParticleWithOneInterpolant.super_ImpParticle.center.field_0.sse_vec[0] -
              center->x;
      fVar6 = (blobParticles->super_ImpParticleWithTwoInterpolants).
              super_ImpParticleWithOneInterpolant.super_ImpParticle.center.field_0.sse_vec[1] -
              center->y;
      fVar7 = (blobParticles->super_ImpParticleWithTwoInterpolants).
              super_ImpParticleWithOneInterpolant.super_ImpParticle.center.field_0.sse_vec[2] -
              center->z;
                    /* Unresolved local var: fltx4 result@[???] */
      uVar3 = *(uint *)(iVar2 + 0x3c);
      fVar10 = (blobParticles->super_ImpParticleWithTwoInterpolants).
               super_ImpParticleWithOneInterpolant.super_ImpParticle.center.field_0.sse_vec[3];
      fVar9 = (float)(*(uint *)(iVar2 + 0x30) &
                      (uint)(blobParticles->super_ImpParticleWithTwoInterpolants).
                            super_ImpParticleWithOneInterpolant.super_ImpParticle.center.field_0.
                            sse_vec[0] |
                     ~*(uint *)(iVar2 + 0x30) &
                     (uint)(fVar4 * rotationMatrix->m[0][0] + fVar6 * rotationMatrix->m[0][1] +
                           fVar7 * rotationMatrix->m[0][2]));
      fVar8 = (float)(*(uint *)(iVar2 + 0x34) &
                      (uint)(blobParticles->super_ImpParticleWithTwoInterpolants).
                            super_ImpParticleWithOneInterpolant.super_ImpParticle.center.field_0.
                            sse_vec[1] |
                     ~*(uint *)(iVar2 + 0x34) &
                     (uint)(fVar4 * rotationMatrix->m[1][0] + fVar6 * rotationMatrix->m[1][1] +
                           fVar7 * rotationMatrix->m[1][2]));
      fVar4 = (float)(*(uint *)(iVar2 + 0x38) &
                      (uint)(blobParticles->super_ImpParticleWithTwoInterpolants).
                            super_ImpParticleWithOneInterpolant.super_ImpParticle.center.field_0.
                            sse_vec[2] |
                     ~*(uint *)(iVar2 + 0x38) &
                     (uint)(fVar4 * rotationMatrix->m[2][0] + fVar6 * rotationMatrix->m[2][1] +
                           fVar7 * rotationMatrix->m[2][2]));
      (blobParticles->super_ImpParticleWithTwoInterpolants).super_ImpParticleWithOneInterpolant.
      super_ImpParticle.center.field_0.sse_vec[0] = fVar9;
      (blobParticles->super_ImpParticleWithTwoInterpolants).super_ImpParticleWithOneInterpolant.
      super_ImpParticle.center.field_0.sse_vec[1] = fVar8;
      (blobParticles->super_ImpParticleWithTwoInterpolants).super_ImpParticleWithOneInterpolant.
      super_ImpParticle.center.field_0.sse_vec[2] = fVar4;
      (blobParticles->super_ImpParticleWithTwoInterpolants).super_ImpParticleWithOneInterpolant.
      super_ImpParticle.center.field_0.sse_vec[3] = (float)(uVar3 & (uint)fVar10);
                    /* Unresolved local var: Point3D n@[???] */
      fVar9 = fVar9 - transformedCenter->x;
      fVar8 = fVar8 - transformedCenter->y;
      fVar4 = fVar4 - transformedCenter->z;
                    /* Unresolved local var: Point3D n@[???]
                       Unresolved local var: float l@[???] */
      fVar10 = SQRT(fVar9 * fVar9 + fVar8 * fVar8 + fVar4 * fVar4);
      uVar3 = -(uint)(fVar10 != 0.0);
      fVar10 = (float)(~uVar3 & *(uint *)(unaff_EBX + 0x4c15d3) | (uint)fVar10 & uVar3);
      (blobParticles->super_ImpParticleWithTwoInterpolants).interpolants2.field_0.sse_vec[0] =
           fVar9 / fVar10;
      (blobParticles->super_ImpParticleWithTwoInterpolants).interpolants2.field_0.sse_vec[1] =
           fVar8 / fVar10;
      (blobParticles->super_ImpParticleWithTwoInterpolants).interpolants2.field_0.sse_vec[2] =
           fVar4 / fVar10;
      (blobParticles->super_ImpParticleWithTwoInterpolants).interpolants2.field_0.sse_vec[3] = 0.0;
      (blobParticles->interpolants4).field_0.sse_vec[0] = 0.0;
      (blobParticles->interpolants4).field_0.sse_vec[1] = 0.0;
      (blobParticles->interpolants4).field_0.sse_vec[2] = fVar1;
                    /* Unresolved local var: Point3D n@[???] */
      fVar10 = (blobParticles->super_ImpParticleWithTwoInterpolants).interpolants2.field_0.sse_vec
               [1];
      fVar4 = (blobParticles->super_ImpParticleWithTwoInterpolants).interpolants2.field_0.sse_vec[2]
      ;
      fVar9 = (blobParticles->super_ImpParticleWithTwoInterpolants).interpolants2.field_0.sse_vec[0]
      ;
      (blobParticles->interpolants3).field_0.sse_vec[0] =
           (float)((uint)fVar10 ^ *(uint *)(unaff_EBX + 0x4c64bf)) - fVar4 * 0.0;
      (blobParticles->interpolants3).field_0.sse_vec[1] = fVar4 * 0.0 + fVar9;
      (blobParticles->interpolants3).field_0.sse_vec[2] = fVar9 * 0.0 - fVar10 * 0.0;
      (blobParticles->interpolants3).field_0.sse_vec[3] = 0.0;
      fVar8 = (blobParticles->interpolants3).field_0.sse_vec[0];
      fVar6 = (blobParticles->interpolants3).field_0.sse_vec[1];
      fVar7 = (blobParticles->interpolants3).field_0.sse_vec[2];
      fVar5 = SQRT(fVar8 * fVar8 + fVar6 * fVar6 + fVar7 * fVar7);
      uVar3 = -(uint)(fVar5 != 0.0);
      fVar5 = (float)(~uVar3 & *(uint *)(unaff_EBX + 0x4c15d3) | (uint)fVar5 & uVar3);
      fVar8 = fVar8 / fVar5;
      (blobParticles->interpolants3).field_0.sse_vec[0] = fVar8;
      fVar6 = fVar6 / fVar5;
      (blobParticles->interpolants3).field_0.sse_vec[1] = fVar6;
      fVar7 = fVar7 / fVar5;
      (blobParticles->interpolants3).field_0.sse_vec[2] = fVar7;
                    /* Unresolved local var: Point3D n@[???] */
      (blobParticles->interpolants4).field_0.sse_vec[0] = fVar7 * fVar10 - fVar4 * fVar6;
      (blobParticles->interpolants4).field_0.sse_vec[1] = fVar4 * fVar8 - fVar7 * fVar9;
      (blobParticles->interpolants4).field_0.sse_vec[2] = fVar6 * fVar9 - fVar10 * fVar8;
      (blobParticles->interpolants4).field_0.sse_vec[3] = 0.0;
                    /* Unresolved local var: float l@[DW_OP_reg25(XMM4)] */
      fVar10 = (blobParticles->interpolants4).field_0.sse_vec[0];
      fVar4 = (blobParticles->interpolants4).field_0.sse_vec[1];
      fVar9 = (blobParticles->interpolants4).field_0.sse_vec[2];
      fVar8 = SQRT(fVar10 * fVar10 + fVar4 * fVar4 + fVar9 * fVar9);
      uVar3 = -(uint)(fVar8 != 0.0);
      fVar8 = (float)(~uVar3 & *(uint *)(unaff_EBX + 0x4c15d3) | (uint)fVar8 & uVar3);
      (blobParticles->interpolants4).field_0.sse_vec[0] = fVar10 / fVar8;
      (blobParticles->interpolants4).field_0.sse_vec[1] = fVar4 / fVar8;
      (blobParticles->interpolants4).field_0.sse_vec[2] = fVar9 / fVar8;
      i = i + 1;
      blobParticles = blobParticles + 1;
    } while (i != blobCount);
  }
  return;
}


/* NPaintRenderer::Paintblob_BeginDraw at 00521160 */

CDrawInfo *
NPaintRenderer::Paintblob_BeginDraw
          (IClientRenderable *pClientRenderable,Vector *vecRenderOrigin,IMaterial *pMaterial,
          float flCubeWidth,view_id_t viewID,ImpParticleWithFourInterpolants *blobParticles,
          int blobCount,bool bUseTwoPassesToDraw)

{
  uint *puVar1;
  float *pfVar2;
  char cVar3;
  ImpTiler *this;
  int iVar4;
  int *piVar5;
  int unaff_EBX;
  int iVar6;
  float fVar7;
  VMatrix *local_258;
  float local_24c;
  float local_248;
  CDrawInfo *local_240;
  VMatrix local_23c;
  VMatrix local_1fc;
  VMatrix local_1bc;
  VMatrix local_17c;
  float local_13c;
  float local_138;
  float local_134;
  float local_130;
  float local_12c;
  float local_128;
  float local_124;
  float local_120;
  __313 local_11c;
  __313 local_10c;
  __313 local_fc;
  float local_e8;
  float local_e4;
  float local_e0;
  float local_dc;
  float local_d8;
  float local_d4;
  float local_d0;
  float local_cc;
  float local_c8;
  float local_c4;
  float local_c0;
  float local_bc;
  undefined4 local_b8;
  float local_b4;
  undefined4 local_b0;
  float local_ac;
  float local_a8;
  float local_a4;
  float local_a0;
  float local_9c;
  float local_98;
  float local_94;
  float local_90;
  float local_8c;
  float local_88;
  float local_84;
  float local_80;
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
  Vector local_4c;
  Vector local_40;
  Vector local_34;
  Vector local_28 [2];
  
                    /* Unresolved local var: CVProfScope VProf_@[???]
                       Unresolved local var: Vector transformedCenter@[???]
                       Unresolved local var: VMatrix rotationMatrix@[???]
                       Unresolved local var: VMatrix invRotationMatrix@[???]
                       Unresolved local var: Vector transformedEye@[???]
                       Unresolved local var: float fAngle@[???]
                       Unresolved local var: float flPaintResolution@[???]
                       Unresolved local var: float flMaxPaintScale@[???]
                       Unresolved local var: float flPaintResolutionLow@[???]
                       Unresolved local var: ImpTiler * pTiler@[???]
                       Unresolved local var: CDrawInfo * pDrawInfo@[???] */
  ___i686_get_pc_thunk_bx();
  if (((*(int *)(*(int *)(unaff_EBX + 0x642c03) + 0x100c) == 0) &&
      (*(char *)(*(int *)(unaff_EBX + 0x642c03) + 0x1010) != '\0')) ||
     (iVar6 = *(int *)(*(int *)(unaff_EBX + 0x642c03) + 0x19b8), iVar4 = _ThreadGetCurrentId(),
     iVar6 != iVar4)) {
    iVar6 = *(int *)(*(int *)(unaff_EBX + 0x847a2b) + 0x30);
  }
  else {
    piVar5 = *(int **)(*(int *)(unaff_EBX + 0x642c03) + 0x1014);
    if (*piVar5 != unaff_EBX + 0x498953) {
      piVar5 = (int *)CVProfNode::GetSubNode
                                ((char *)piVar5,unaff_EBX + 0x498953,(char *)0x0,
                                 unaff_EBX + 0x49896c);
      *(int **)(*(int *)(unaff_EBX + 0x642c03) + 0x1014) = piVar5;
    }
    puVar1 = (uint *)(piVar5[0x1c] * 8 + *(int *)(*(int *)(unaff_EBX + 0x642c03) + 0x10a0) + 4);
    *puVar1 = *puVar1 | 4;
    CVProfNode::EnterScope();
    *(undefined1 *)(*(int *)(unaff_EBX + 0x642c03) + 0x1010) = 0;
    iVar6 = *(int *)(*(int *)(unaff_EBX + 0x847a2b) + 0x30);
  }
  if ((iVar6 == 0) || (local_240 = (CDrawInfo *)0x0, viewID == VIEW_MAIN)) {
    local_28[0].x = vecRenderOrigin->x;
    local_28[0].y = vecRenderOrigin->y;
    local_28[0].z = vecRenderOrigin->z;
    if (*(int *)(*(int *)(unaff_EBX + 0x84718b) + 0x30) == 0) {
      MatrixSetIdentity(&local_17c);
      MatrixSetIdentity(&local_1bc);
      local_34.x = 0.0;
      local_34.y = 0.0;
      local_34.z = 0.0;
      local_24c = 0.0;
    }
    else {
                    /* Unresolved local var: Vector eye@[???] */
      iVar6 = (**(code **)(*(int *)**(undefined4 **)(&DAT_00642cd3 + unaff_EBX) + 0x34))
                        ((int *)**(undefined4 **)(&DAT_00642cd3 + unaff_EBX));
      local_24c = *(float *)(unaff_EBX + 0x4c121f) + *(float *)(iVar6 + 0x70);
      if (*(int *)(*(int *)(unaff_EBX + 0x8471eb) + 0x30) != 0) {
        local_24c = (float)((int)((local_24c + *(float *)(unaff_EBX + 0x4c1273)) /
                                 *(float *)(unaff_EBX + 0x4c1283)) * 0x5a);
      }
      local_40.x = 0.0;
      local_40.y = 0.0;
      local_40.z = 1.0;
      SetupMatrixAxisRot(&local_1fc,&local_40,
                         (vec_t)((uint)local_24c ^ *(uint *)(unaff_EBX + 0x4c5eff)));
      VMatrix::operator=(&local_17c,&local_1fc);
      local_4c.x = 0.0;
      local_4c.y = 0.0;
      local_4c.z = 1.0;
      SetupMatrixAxisRot(&local_23c,&local_4c,local_24c);
      VMatrix::operator=(&local_1bc,&local_23c);
      iVar6 = (**(code **)(*(int *)**(undefined4 **)(&DAT_00642cd3 + unaff_EBX) + 0x34))
                        ((int *)**(undefined4 **)(&DAT_00642cd3 + unaff_EBX));
                    /* Unresolved local var: Vector res@[???] */
      local_58 = *(float *)(iVar6 + 0x60) - vecRenderOrigin->x;
      local_54 = *(float *)(iVar6 + 100) - vecRenderOrigin->y;
      local_50 = *(float *)(iVar6 + 0x68) - vecRenderOrigin->z;
      local_c8 = local_58 * local_17c.m[2][0] + local_54 * local_17c.m[2][1] +
                 local_50 * local_17c.m[2][2];
      local_cc = local_58 * local_17c.m[1][0] + local_54 * local_17c.m[1][1] +
                 local_50 * local_17c.m[1][2];
      local_d0 = local_58 * local_17c.m[0][0] + local_54 * local_17c.m[0][1] +
                 local_50 * local_17c.m[0][2];
      local_c4 = local_d0;
      local_c0 = local_cc;
      local_bc = local_c8;
      local_64 = local_d0;
      local_60 = local_cc;
      local_5c = local_c8;
      local_34.x = local_d0;
      local_34.y = local_cc;
      local_34.z = local_c8;
    }
    local_258 = &local_17c;
    local_248 = *(float *)(*(int *)(unaff_EBX + 0x8472ab) + 0x2c);
    fVar7 = local_248 * *(float *)(&DAT_004c200b + unaff_EBX);
    if (*(int *)(*(int *)(unaff_EBX + 0x84796b) + 0x30) == 0) {
      local_248 = fVar7;
    }
    if ((*(int *)(*(int *)(unaff_EBX + 0x8479cb) + 0x30) != 0) && (viewID != VIEW_MAIN)) {
      local_248 = fVar7;
    }
    TransformParticles(local_24c,vecRenderOrigin,local_28,&local_34,local_258,&local_1bc,
                       blobParticles,blobCount);
    this = ImpTilerFactory::getTiler((ImpTilerFactory *)**(undefined4 **)(unaff_EBX + 0x643447));
    ImpTiler::SetCubeWidth
              (this,flCubeWidth * *(float *)(*(int *)(&DAT_0084724b + unaff_EBX) + 0x2c) * local_248
              );
    ImpTiler::SetRenderRadius
              (this,flCubeWidth * *(float *)(*(int *)(&DAT_0084724b + unaff_EBX) + 0x2c) *
                    *(float *)(*(int *)(unaff_EBX + 0x84730b) + 0x2c));
    ImpTiler::SetCutoffRadius
              (this,flCubeWidth * *(float *)(*(int *)(&DAT_0084724b + unaff_EBX) + 0x2c) *
                    *(float *)(*(int *)(unaff_EBX + 0x84736b) + 0x2c));
    this->m_nTileIndexToDraw = *(int *)(*(int *)(unaff_EBX + 0x8475ab) + 0x30);
    local_240 = operator_new(0x10);
    local_240->m_pTiler = this;
    local_240->m_fAngle = local_24c;
    (local_240->m_pRenderContext).super_CRefPtr<IMatRenderContext>.
    super_CBaseAutoPtr<IMatRenderContext>.m_pObject = (IMatRenderContext *)0x0;
    local_240->m_ViewId = viewID;
    if (!bUseTwoPassesToDraw) {
      SetupRenderContext(pClientRenderable,vecRenderOrigin,pMaterial,local_240);
      ImpTiler::SetRenderContext(this,&local_240->m_pRenderContext);
    }
    local_fc.sse_vec[0] = 0.0;
    local_fc.sse_vec[1] = 0.0;
    local_fc.sse_vec[2] = 0.0;
    local_fc.sse_vec[3] = 0.0;
    ImpTiler::beginFrame
              (this,(Point3D *)&local_fc,*(int *)(*(int *)(&DAT_0084760b + unaff_EBX) + 0x30) == 0,
               bUseTwoPassesToDraw);
                    /* Unresolved local var: int i@[???] */
    if (0 < blobCount) {
      iVar6 = 0;
      do {
        ImpTiler::insertParticle(this,(ImpParticle *)blobParticles);
        iVar6 = iVar6 + 1;
        blobParticles = blobParticles + 1;
      } while (iVar6 != blobCount);
    }
    if (*(int *)(*(int *)(unaff_EBX + 0x84718b) + 0x30) == 0) {
      ImpTiler::drawSurface(this,bUseTwoPassesToDraw);
      iVar6 = *(int *)(*(int *)(unaff_EBX + 0x84766b) + 0x30);
    }
    else {
      local_10c.sse_vec[0] = local_34.x;
      local_10c.sse_vec[1] = local_34.y;
      local_10c.sse_vec[2] = local_34.z;
      local_10c.sse_vec[3] = 0.0;
      ImpTiler::drawSurfaceSorted(this,(Point3D *)&local_10c,bUseTwoPassesToDraw);
      iVar6 = *(int *)(*(int *)(unaff_EBX + 0x84766b) + 0x30);
    }
    if (iVar6 != 0) {
                    /* Unresolved local var: int i@[???]
                       Unresolved local var: Vector overlayCenter@[???]
                       Unresolved local var: Vector mins@[???]
                       Unresolved local var: Vector maxs@[???] */
      for (iVar6 = 0; iVar4 = ImpTiler::getNoTiles(this), iVar6 < iVar4; iVar6 = iVar6 + 1) {
        ImpTiler::getTileOffset((Point3D *)&local_11c,this,iVar6);
        local_e0 = local_11c.sse_vec[0] * local_1bc.m[2][0] +
                   local_11c.sse_vec[1] * local_1bc.m[2][1] +
                   local_11c.sse_vec[2] * local_1bc.m[2][2];
        local_e4 = local_11c.sse_vec[0] * local_1bc.m[1][0] +
                   local_11c.sse_vec[1] * local_1bc.m[1][1] +
                   local_11c.sse_vec[2] * local_1bc.m[1][2];
        local_e8 = local_11c.sse_vec[0] * local_1bc.m[0][0] +
                   local_11c.sse_vec[1] * local_1bc.m[0][1] +
                   local_11c.sse_vec[2] * local_1bc.m[0][2];
        local_12c = (this->m_RenderDimension).field_0.sse_vec[0];
        local_128 = (this->m_RenderDimension).field_0.sse_vec[1];
        local_124 = (this->m_RenderDimension).field_0.sse_vec[2];
        local_120 = (this->m_RenderDimension).field_0.sse_vec[3];
                    /* Unresolved local var: Vector res@[???] */
        pfVar2 = (float *)(unaff_EBX + 0x4c145f);
        local_88 = *pfVar2 * local_12c;
        local_84 = *pfVar2 * local_128;
        local_80 = *pfVar2 * local_124;
        local_13c = (this->m_RenderDimension).field_0.sse_vec[0];
        local_138 = (this->m_RenderDimension).field_0.sse_vec[1];
        local_134 = (this->m_RenderDimension).field_0.sse_vec[2];
        local_130 = (this->m_RenderDimension).field_0.sse_vec[3];
                    /* Unresolved local var: Vector res@[???] */
        pfVar2 = (float *)(unaff_EBX + 0x4c109f);
        local_a0 = *pfVar2 * local_13c;
        local_9c = *pfVar2 * local_138;
        local_98 = *pfVar2 * local_134;
        local_b8 = 0;
        local_b4 = local_24c;
        local_b0 = 0;
                    /* Unresolved local var: Vector res@[???] */
        local_ac = local_e8 + vecRenderOrigin->x;
        local_a8 = local_e4 + vecRenderOrigin->y;
        local_a4 = local_e0 + vecRenderOrigin->z;
        local_dc = local_e8;
        local_d8 = local_e4;
        local_d4 = local_e0;
        local_94 = local_a0;
        local_90 = local_9c;
        local_8c = local_98;
        local_7c = local_88;
        local_78 = local_84;
        local_74 = local_80;
        local_70 = local_e8;
        local_6c = local_e4;
        local_68 = local_e0;
        (**(code **)(*(int *)**(undefined4 **)(&DAT_00642d07 + unaff_EBX) + 4))
                  ((int *)**(undefined4 **)(&DAT_00642d07 + unaff_EBX),&local_ac,&local_7c,&local_94
                   ,&local_b8,0,0xff,0,0,0);
      }
    }
  }
  if (((*(char *)(*(int *)(unaff_EBX + 0x642c03) + 0x1010) == '\0') ||
      (*(int *)(*(int *)(unaff_EBX + 0x642c03) + 0x100c) != 0)) &&
     (iVar6 = *(int *)(*(int *)(unaff_EBX + 0x642c03) + 0x19b8), iVar4 = _ThreadGetCurrentId(),
     iVar6 == iVar4)) {
    cVar3 = CVProfNode::ExitScope();
    if (cVar3 != '\0') {
      *(undefined4 *)(*(int *)(unaff_EBX + 0x642c03) + 0x1014) =
           *(undefined4 *)(*(int *)(*(int *)(unaff_EBX + 0x642c03) + 0x1014) + 100);
    }
    iVar6 = *(int *)(unaff_EBX + 0x642c03);
    *(bool *)(iVar6 + 0x1010) = *(int *)(iVar6 + 0x1014) == iVar6 + 0x1018;
    return local_240;
  }
  return local_240;
}


/* NPaintRenderer::PaintBlob_WaitEndOfDraw at 005226f0 */

void NPaintRenderer::PaintBlob_WaitEndOfDraw
               (IClientRenderable *pClientRenderable,Vector vecRenderOrigin,IMaterial *pMaterial,
               CDrawInfo *pDrawInfo,bool bUseTwoPassesToDraw)

{
  ImpTiler *this;
  IMatRenderContext *pIVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: ImpTiler * pTiler@[???] */
  ___i686_get_pc_thunk_bx();
  this = pDrawInfo->m_pTiler;
  if (bUseTwoPassesToDraw) {
    SetupRenderContext(pClientRenderable,&vecRenderOrigin,pMaterial,pDrawInfo);
    ImpTiler::SetRenderContext(this,&pDrawInfo->m_pRenderContext);
  }
  ImpTiler::endFrame(this,bUseTwoPassesToDraw);
  ImpTilerFactory::returnTiler((ImpTilerFactory *)**(undefined4 **)(unaff_EBX + 0x641eb4),this);
  pIVar1 = (pDrawInfo->m_pRenderContext).super_CRefPtr<IMatRenderContext>.
           super_CBaseAutoPtr<IMatRenderContext>.m_pObject;
  (*(pIVar1->super_IRefCounted)._vptr_IRefCounted[0x16])(pIVar1);
  pIVar1 = (pDrawInfo->m_pRenderContext).super_CRefPtr<IMatRenderContext>.
           super_CBaseAutoPtr<IMatRenderContext>.m_pObject;
  if (pIVar1 != (IMatRenderContext *)0x0) {
    (*(pIVar1->super_IRefCounted)._vptr_IRefCounted[3])(pIVar1);
    pIVar1 = (pDrawInfo->m_pRenderContext).super_CRefPtr<IMatRenderContext>.
             super_CBaseAutoPtr<IMatRenderContext>.m_pObject;
    if (pIVar1 != (IMatRenderContext *)0x0) {
      (*(pIVar1->super_IRefCounted)._vptr_IRefCounted[1])(pIVar1);
    }
  }
  (pDrawInfo->m_pRenderContext).super_CRefPtr<IMatRenderContext>.
  super_CBaseAutoPtr<IMatRenderContext>.m_pObject = (IMatRenderContext *)0x0;
  operator_delete(pDrawInfo);
  return;
}


/* NPaintRenderer::GetMaterial at 0051fb20 */

IMaterial * NPaintRenderer::GetMaterial(IMaterial *pMaterial)

{
  IMaterial *pIVar1;
  int iVar2;
  
                    /* Unresolved local var: IMaterial * pSpecialMaterial@[???] */
  pIVar1 = GetDrawMaterial();
  if (pIVar1 != (IMaterial *)0x0) {
    iVar2 = (*pIVar1->_vptr_IMaterial[0x2a])(pIVar1);
    if ((char)iVar2 == '\0') {
      pMaterial = pIVar1;
    }
  }
  return pMaterial;
}


/* NPaintRenderer::SetupRenderContext at 00521040 */

void NPaintRenderer::SetupRenderContext
               (IClientRenderable *pClientRenderable,Vector *vecRenderOrigin,IMaterial *pMaterial,
               CDrawInfo *drawInfo)

{
  IMatRenderContext *pIVar1;
  _func_int_varargs *p_Var2;
  IMatRenderContext *pIVar3;
  IMaterial *pIVar4;
  int iVar5;
  int unaff_EBX;
  
                    /* Unresolved local var: CMatRenderContextPtr * pRenderContext@[???] */
  ___i686_get_pc_thunk_bx();
  pIVar3 = (IMatRenderContext *)
           (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x642ebe) + 0x1a0))
                     ((int *)**(undefined4 **)(unaff_EBX + 0x642ebe));
  pIVar1 = (drawInfo->m_pRenderContext).super_CRefPtr<IMatRenderContext>.
           super_CBaseAutoPtr<IMatRenderContext>.m_pObject;
  if (pIVar1 != (IMatRenderContext *)0x0) {
    (*(pIVar1->super_IRefCounted)._vptr_IRefCounted[3])(pIVar1);
    pIVar1 = (drawInfo->m_pRenderContext).super_CRefPtr<IMatRenderContext>.
             super_CBaseAutoPtr<IMatRenderContext>.m_pObject;
    if (pIVar1 != (IMatRenderContext *)0x0) {
      (*(pIVar1->super_IRefCounted)._vptr_IRefCounted[1])(pIVar1);
    }
  }
  (drawInfo->m_pRenderContext).super_CRefPtr<IMatRenderContext>.
  super_CBaseAutoPtr<IMatRenderContext>.m_pObject = (IMatRenderContext *)0x0;
  if (pIVar3 != (IMatRenderContext *)0x0) {
    (**(pIVar3->super_IRefCounted)._vptr_IRefCounted)(pIVar3);
  }
  (drawInfo->m_pRenderContext).super_CRefPtr<IMatRenderContext>.
  super_CBaseAutoPtr<IMatRenderContext>.m_pObject = pIVar3;
  (*(pIVar3->super_IRefCounted)._vptr_IRefCounted[2])(pIVar3);
  pIVar1 = (drawInfo->m_pRenderContext).super_CRefPtr<IMatRenderContext>.
           super_CBaseAutoPtr<IMatRenderContext>.m_pObject;
  (*(pIVar1->super_IRefCounted)._vptr_IRefCounted[0x14])(pIVar1,10);
  pIVar1 = (drawInfo->m_pRenderContext).super_CRefPtr<IMatRenderContext>.
           super_CBaseAutoPtr<IMatRenderContext>.m_pObject;
  p_Var2 = (pIVar1->super_IRefCounted)._vptr_IRefCounted[9];
                    /* Unresolved local var: IMaterial * pSpecialMaterial@[???] */
  pIVar4 = GetDrawMaterial();
  if (pIVar4 != (IMaterial *)0x0) {
    iVar5 = (*pIVar4->_vptr_IMaterial[0x2a])(pIVar4);
    if ((char)iVar5 == '\0') goto LAB_005210e0;
  }
  pIVar4 = pMaterial;
LAB_005210e0:
  (*p_Var2)(pIVar1,pIVar4,pClientRenderable);
  pIVar1 = (drawInfo->m_pRenderContext).super_CRefPtr<IMatRenderContext>.
           super_CBaseAutoPtr<IMatRenderContext>.m_pObject;
  (*(pIVar1->super_IRefCounted)._vptr_IRefCounted[0x15])(pIVar1);
  pIVar1 = (drawInfo->m_pRenderContext).super_CRefPtr<IMatRenderContext>.
           super_CBaseAutoPtr<IMatRenderContext>.m_pObject;
  (*(pIVar1->super_IRefCounted)._vptr_IRefCounted[0x1f])(pIVar1);
  pIVar1 = (drawInfo->m_pRenderContext).super_CRefPtr<IMatRenderContext>.
           super_CBaseAutoPtr<IMatRenderContext>.m_pObject;
  (*(pIVar1->super_IRefCounted)._vptr_IRefCounted[0x24])
            (pIVar1,vecRenderOrigin->x,vecRenderOrigin->y,vecRenderOrigin->z);
  pIVar1 = (drawInfo->m_pRenderContext).super_CRefPtr<IMatRenderContext>.
           super_CBaseAutoPtr<IMatRenderContext>.m_pObject;
  (*(pIVar1->super_IRefCounted)._vptr_IRefCounted[0x23])(pIVar1,drawInfo->m_fAngle,0,0,0x3f800000);
  return;
}


/* NPaintRenderer::CleanupRenderContext at 0051fb60 */

void NPaintRenderer::CleanupRenderContext(CDrawInfo *drawInfo)

{
                    /* WARNING: Could not recover jumptable at 0x0051fb72. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(((drawInfo->m_pRenderContext).super_CRefPtr<IMatRenderContext>.
      super_CBaseAutoPtr<IMatRenderContext>.m_pObject)->super_IRefCounted)._vptr_IRefCounted[0x16])
            ();
  return;
}


/* NPaintRenderer::Paintblob_CullOutOfViewParticles at 00521bf0 */

/* WARNING: Restarted to delay deadcode elimination for space: stack */

void NPaintRenderer::Paintblob_CullOutOfViewParticles(void)

{
  float fVar1;
  float fVar2;
  vec_t vVar3;
  vec_t vVar4;
  vec_t vVar5;
  bool bVar6;
  VPlane *pPlanes;
  vec_t *pvVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  int unaff_EBX;
  vec_t *pvVar11;
  int iVar12;
  int iVar13;
  int iParticlesRemoved;
  Vector local_28 [2];
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x847046)) {
    iParticlesRemoved = 0;
    iVar13 = 0;
    do {
                    /* Unresolved local var: ImpParticleWithOneInterpolant * imp_particle@[???]
                       Unresolved local var: Vector vCenter@[???] */
      pvVar11 = (vec_t *)(iVar13 * 0x60 + *(int *)(unaff_EBX + 0x84703a));
      local_28[0].z = pvVar11[2];
      local_28[0].y = pvVar11[1];
      local_28[0].x = *pvVar11;
      fVar1 = *(float *)(unaff_EBX + 0x4c0a16);
      fVar2 = pvVar11[4];
      pPlanes = (VPlane *)
                (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x642246) + 0x28))
                          ((int *)**(undefined4 **)(unaff_EBX + 0x642246));
      bVar6 = R_CullSphere(pPlanes,5,local_28,fVar1 * fVar2);
      if (bVar6) {
        pvVar7 = (vec_t *)((~iParticlesRemoved + *(int *)(unaff_EBX + 0x847046)) * 0x60 +
                          *(int *)(unaff_EBX + 0x84703a));
                    /* Unresolved local var: ImpParticleWithOneInterpolant * last_particle@[???] */
        vVar3 = pvVar7[1];
        vVar4 = pvVar7[2];
        vVar5 = pvVar7[3];
        *pvVar11 = *pvVar7;
        pvVar11[1] = vVar3;
        pvVar11[2] = vVar4;
        pvVar11[3] = vVar5;
        pvVar11[4] = pvVar7[4];
        pvVar11[8] = pvVar7[8];
        pvVar11[9] = pvVar7[9];
        pvVar11[10] = pvVar7[10];
        pvVar11[0xb] = pvVar7[0xb];
        iParticlesRemoved = iParticlesRemoved + 1;
        iVar13 = iVar13 + -1;
      }
      iVar13 = iVar13 + 1;
      iVar9 = *(int *)(unaff_EBX + 0x847046);
    } while (iVar13 < iVar9 - iParticlesRemoved);
    if (iParticlesRemoved != 0) {
      iVar13 = (iVar9 - iParticlesRemoved) - iVar9;
      if (iVar13 < 1) {
        if (iVar13 != 0) {
          *(int *)(unaff_EBX + 0x847046) = iVar9 + iVar13;
        }
      }
      else {
        iVar12 = iVar13 + iVar9;
        if (*(int *)(unaff_EBX + 0x84703e) < iVar12) {
          CUtlMemoryAligned<ImpParticleWithFourInterpolants,16>::Grow
                    ((CUtlMemoryAligned<ImpParticleWithFourInterpolants,16> *)(unaff_EBX + 0x84703a)
                     ,iVar12 - *(int *)(unaff_EBX + 0x84703e));
        }
        iVar8 = iVar13 + *(int *)(unaff_EBX + 0x847046);
        *(int *)(unaff_EBX + 0x847046) = iVar8;
        iVar10 = *(int *)(unaff_EBX + 0x84703a);
        *(int *)(unaff_EBX + 0x84704a) = iVar10;
        iVar8 = (iVar8 - iVar9) - iVar13;
        if (0 < iVar8) {
          _V_memmove((void *)(iVar10 + iVar12 * 0x60),(void *)(iVar10 + iVar9 * 0x60),iVar8 * 0x60);
          iVar10 = *(int *)(unaff_EBX + 0x84703a);
        }
        iVar9 = iVar9 * 0x60;
        iVar12 = 0;
                    /* Unresolved local var: int i@[???] */
        while( true ) {
          iVar10 = iVar10 + iVar9;
          if (iVar10 != 0) {
            *(undefined4 *)(iVar10 + 0xc) = 0;
            *(undefined4 *)(iVar10 + 0x2c) = 0;
            *(undefined4 *)(iVar10 + 0x3c) = 0;
            *(undefined4 *)(iVar10 + 0x4c) = 0;
            *(undefined4 *)(iVar10 + 0x5c) = 0;
          }
          iVar12 = iVar12 + 1;
          iVar9 = iVar9 + 0x60;
          if (iVar12 == iVar13) break;
          iVar10 = *(int *)(unaff_EBX + 0x84703a);
        }
      }
    }
  }
  return;
}


/* NPaintRenderer::Paintblob_DrawIsoSurface_Synchronous at 005227f0 */

void NPaintRenderer::Paintblob_DrawIsoSurface_Synchronous
               (IClientRenderable *pClientRenderable,Vector vecRenderOrigin,IMaterial *pMaterial,
               float flCubeWidth,view_id_t viewID,BlobParticlesEnvelope_t *particles,int blobCount,
               bool bUseTwoPassesToDraw)

{
  int iVar1;
  Vector vecRenderOrigin_00;
  CDrawInfo *pDrawInfo;
  ImpParticleWithFourInterpolants *blobParticles;
  
                    /* Unresolved local var: CDrawInfo * pDrawInfo@[???] */
  iVar1 = (particles->super_CUtlDataEnvelope).m_nBytes;
  blobParticles = (ImpParticleWithFourInterpolants *)0x0;
  if ((iVar1 != 0) && (blobParticles = (ImpParticleWithFourInterpolants *)particles, 4 < iVar1)) {
    blobParticles =
         (ImpParticleWithFourInterpolants *)(particles->super_CUtlDataEnvelope).field_0.m_pData;
  }
  pDrawInfo = Paintblob_BeginDraw(pClientRenderable,&vecRenderOrigin,pMaterial,flCubeWidth,viewID,
                                  blobParticles,blobCount,bUseTwoPassesToDraw);
  vecRenderOrigin_00.y = vecRenderOrigin.y;
  vecRenderOrigin_00.x = vecRenderOrigin.x;
  vecRenderOrigin_00.z = vecRenderOrigin.z;
  PaintBlob_WaitEndOfDraw
            (pClientRenderable,vecRenderOrigin_00,pMaterial,pDrawInfo,bUseTwoPassesToDraw);
  return;
}


/* NPaintRenderer::Paintblob_Draw at 00522890 */

/* WARNING: Type propagation algorithm not settling */

void NPaintRenderer::Paintblob_Draw
               (int renderMode,IClientRenderable *pClientRenderable,Vector *vecRenderOrigin,
               IMaterial *pMaterial,float flCubeWidth,PortalMatrixList_t *portalMatrixList,
               bool bSurfaceNoParticleCull)

{
  code *pcVar1;
  Vector vecRenderOrigin_00;
  undefined4 uVar2;
  undefined4 *puVar3;
  view_id_t viewID;
  int *piVar4;
  void *pvVar5;
  undefined4 uVar6;
  CDrawInfo *pCVar7;
  uint uVar8;
  int *piVar9;
  int unaff_EBX;
  matrix3x4_t *matrix;
  size_t nSize;
  int iVar10;
  ImpParticleWithFourInterpolants *local_98;
  float local_8c;
  ImpParticleWithFourInterpolants *local_88;
  int i;
  bool local_72;
  bool local_71;
  QAngle local_54;
  Vector local_48;
  Vector local_3c;
  Vector local_30;
  __313 local_24;
  
                    /* Unresolved local var: Vector backwardPortalMins@[???]
                       Unresolved local var: Vector backwardPortalMaxs@[???]
                       Unresolved local var: Vector * nonConstVecRenderOrigin@[???] */
  ___i686_get_pc_thunk_bx();
  local_30.x = 0.0;
  local_30.y = -32.0;
  local_30.z = -56.0;
  local_3c.x = -64.0;
  local_3c.y = 32.0;
  local_3c.z = 56.0;
  if ((*(int *)(*(int *)(&DAT_008463db + unaff_EBX) + 0x30) != 0) && (0 < portalMatrixList->m_Size))
  {
    i = 0;
    iVar10 = 0;
    do {
                    /* Unresolved local var: Vector pos@[???]
                       Unresolved local var: QAngle ang@[???] */
      matrix = (matrix3x4_t *)
               ((int)((portalMatrixList->m_Memory).m_pMemory)->m_flMatVal[0] + iVar10);
      MatrixAngles(matrix,&local_54.x);
      local_48.y = matrix->m_flMatVal[1][3];
      local_48.z = matrix->m_flMatVal[2][3];
      NDebugOverlay::BoxAngles(&local_48,&local_30,&local_3c,&local_54,0xff,0xff,0,0,0.1);
      i = i + 1;
      iVar10 = iVar10 + 0x30;
    } while (i < portalMatrixList->m_Size);
  }
  if (!bSurfaceNoParticleCull) {
    Paintblob_CullOutOfViewParticles();
  }
  if ((renderMode == 1) || (*(int *)(*(int *)(unaff_EBX + 0x8459fb) + 0x30) == 0)) {
    Paintblob_DrawSpheres(pMaterial,flCubeWidth);
    return;
  }
  local_48.x = (vec_t)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x64166b) + 0x1a0))
                                ((int *)**(undefined4 **)(unaff_EBX + 0x64166b));
  if ((int *)local_48.x != (int *)0x0) {
    (**(code **)(*(int *)local_48.x + 8))(local_48.x);
  }
  puVar3 = (undefined4 *)(**(code **)(*(int *)local_48.x + 0x22c))(local_48.x);
  iVar10 = ImpRendererFactory::GetNumberOfAcquiredTileRenderers
                     ((ImpRendererFactory *)**(undefined4 **)(unaff_EBX + 0x641d0f));
  local_72 = 0x80 < iVar10 * 4;
  if (local_72) {
    _Warning(unaff_EBX + 0x497987);
  }
  local_72 = !local_72;
  local_71 = local_72;
  if (*(int *)(*(int *)(&DAT_0084635b + unaff_EBX) + 0x30) != 0) {
    local_71 = false;
  }
  viewID = CurrentViewID();
  if (local_71) {
    pCVar7 = Paintblob_BeginDraw((IClientRenderable *)0x0,vecRenderOrigin,(IMaterial *)0x0,
                                 flCubeWidth,viewID,
                                 *(ImpParticleWithFourInterpolants **)(&DAT_00846397 + unaff_EBX),
                                 *(int *)(DmeFramerate_t::DmeFramerate_t + unaff_EBX + 3),true);
    if (pCVar7 != (CDrawInfo *)0x0) {
      if ((*(int *)(*(int *)(&DAT_008461db + unaff_EBX) + 0x30) == 0) ||
         (puVar3 == (undefined4 *)0x0)) {
        PaintBlob_WaitEndOfDraw(pClientRenderable,*vecRenderOrigin,pMaterial,pCVar7,true);
      }
      else {
        pcVar1 = *(code **)*puVar3;
        piVar4 = operator_new(0x30);
        piVar4[3] = 1;
        *piVar4 = unaff_EBX + 0x6a81e7;
        piVar4[2] = unaff_EBX + 0x6a8203;
        piVar4[4] = unaff_EBX + -0x1b1;
        piVar4[5] = (int)pClientRenderable;
        piVar4[6] = (int)vecRenderOrigin->x;
        piVar4[7] = (int)vecRenderOrigin->y;
        piVar4[8] = (int)vecRenderOrigin->z;
        piVar4[9] = (int)pMaterial;
        piVar4[10] = (int)pCVar7;
        *(undefined1 *)(piVar4 + 0xb) = 1;
        (*pcVar1)(puVar3,piVar4);
      }
    }
    goto LAB_00522ccf;
  }
  pvVar5 = *(void **)(&DAT_00846397 + unaff_EBX);
  if (pvVar5 == (void *)0x0) {
    local_24.sse_vec[0] = 0.0;
    local_24.sse_vec[1] = 0.0;
  }
  else {
    nSize = *(int *)(DmeFramerate_t::DmeFramerate_t + unaff_EBX + 3) * 0x60;
    local_24._4_4_ = nSize;
    if ((int)nSize < 5) {
      _memcpy(&local_24,pvVar5,nSize);
    }
    else {
      local_24.sse_vec[0] = (float)operator_new___(nSize);
      _memcpy((void *)local_24.sse_vec[0],pvVar5,nSize);
    }
  }
  local_98 = (ImpParticleWithFourInterpolants *)&local_24;
  if ((*(int *)(*(int *)(&DAT_008461db + unaff_EBX) + 0x30) == 0) || (puVar3 == (undefined4 *)0x0))
  {
    local_54.x = vecRenderOrigin->x;
    local_54.y = vecRenderOrigin->y;
    local_54.z = vecRenderOrigin->z;
    uVar6 = (ImpParticleWithFourInterpolants *)0x0;
    if ((local_24.sse_vec[1] != 0.0) && (uVar6 = local_98, 4 < (int)local_24.sse_vec[1])) {
      uVar6 = local_24.sse_vec[0];
    }
    pCVar7 = Paintblob_BeginDraw(pClientRenderable,(Vector *)&local_54,pMaterial,flCubeWidth,viewID,
                                 (ImpParticleWithFourInterpolants *)uVar6,
                                 *(int *)(DmeFramerate_t::DmeFramerate_t + unaff_EBX + 3),false);
    vecRenderOrigin_00.y = local_54.y;
    vecRenderOrigin_00.x = local_54.x;
    vecRenderOrigin_00.z = local_54.z;
    PaintBlob_WaitEndOfDraw(pClientRenderable,vecRenderOrigin_00,pMaterial,pCVar7,false);
  }
  else {
    iVar10 = *(int *)(DmeFramerate_t::DmeFramerate_t + unaff_EBX + 3);
    pcVar1 = *(code **)*puVar3;
    piVar4 = operator_new(0x40);
    uVar2 = local_24._4_4_;
    uVar6 = local_24._0_4_;
    piVar4[3] = 1;
    *piVar4 = unaff_EBX + 0x6a8227;
    piVar4[2] = unaff_EBX + 0x6a8243;
    piVar4[4] = unaff_EBX + -0xb1;
    piVar4[5] = (int)pClientRenderable;
    piVar4[6] = (int)vecRenderOrigin->x;
    piVar4[7] = (int)vecRenderOrigin->y;
    piVar4[8] = (int)vecRenderOrigin->z;
    piVar4[9] = (int)pMaterial;
    piVar4[10] = (int)flCubeWidth;
    piVar4[0xb] = viewID;
    if (local_24.sse_vec[1] == 0.0) {
LAB_00522dff:
      piVar4[0xc] = 0;
      piVar4[0xd] = 0;
    }
    else if ((int)local_24.sse_vec[1] < 5) {
      piVar4[0xd] = (int)local_24.sse_vec[1];
      piVar9 = piVar4 + 0xc;
      local_88 = local_98;
      local_8c = local_24.sse_vec[1];
      if ((7 < (uint)local_24.sse_vec[1]) && (((uint)piVar9 & 4) != 0)) {
        piVar4[0xc] = (int)local_24.sse_vec[0];
        piVar9 = piVar4 + 0xd;
        local_88 = (ImpParticleWithFourInterpolants *)((int)&local_24 + 4);
        local_8c = (float)((int)local_24.sse_vec[1] - 4);
      }
      for (uVar8 = (uint)local_8c >> 2; uVar8 != 0; uVar8 = uVar8 - 1) {
        *piVar9 = *(int *)&local_88->super_ImpParticleWithTwoInterpolants;
        local_88 = (ImpParticleWithFourInterpolants *)
                   ((int)&local_88->super_ImpParticleWithTwoInterpolants + 4);
        piVar9 = piVar9 + 1;
      }
      if (((uint)local_8c & 2) != 0) {
        *(undefined2 *)piVar9 = *(undefined2 *)&local_88->super_ImpParticleWithTwoInterpolants;
        piVar9 = (int *)((int)piVar9 + 2);
        local_88 = (ImpParticleWithFourInterpolants *)
                   ((int)&local_88->super_ImpParticleWithTwoInterpolants + 2);
      }
      if (((uint)local_8c & 1) != 0) {
        *(undefined1 *)piVar9 = *(undefined1 *)&local_88->super_ImpParticleWithTwoInterpolants;
      }
    }
    else {
      if ((ImpParticleWithFourInterpolants *)local_24.sse_vec[0] ==
          (ImpParticleWithFourInterpolants *)0x0) goto LAB_00522dff;
      piVar4[0xd] = (int)local_24.sse_vec[1];
      pvVar5 = operator_new___((size_t)local_24.sse_vec[1]);
      piVar4[0xc] = (int)pvVar5;
      _memcpy(pvVar5,(void *)uVar6,uVar2);
    }
    piVar4[0xe] = iVar10;
    *(undefined1 *)(piVar4 + 0xf) = 0;
    (*pcVar1)(puVar3,piVar4);
  }
  if ((4 < (int)local_24.sse_vec[1]) &&
     ((ImpParticleWithFourInterpolants *)local_24.sse_vec[0] !=
      (ImpParticleWithFourInterpolants *)0x0)) {
    operator_delete___((void *)local_24.sse_vec[0]);
  }
  local_24.sse_vec[1] = 0.0;
LAB_00522ccf:
  if ((int *)local_48.x != (int *)0x0) {
    (**(code **)(*(int *)local_48.x + 0xc))(local_48.x);
  }
  if ((int *)local_48.x == (int *)0x0) {
    return;
  }
  (**(code **)(*(int *)local_48.x + 4))(local_48.x);
  return;
}


/* __tcf_27 at 0097b870 */

void __tcf_27(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(&UNK_003ed3e8 + extraout_ECX),in_stack_00000008);
  return;
}


/* _GLOBAL__I_r_paintblob_draw_isosurface at 000759c0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_r_paintblob_draw_isosurface(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

