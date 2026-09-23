/* DWARF-guided pseudocode for game/client/portal2/c_paintblob_render.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 00075260 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *pDefaultValue;
  char *pDefaultValue_00;
  char *pDefaultValue_01;
  undefined4 uVar1;
  int unaff_EBX;
  longlong lVar2;
  ConVar *this_1;
  ConVar *in_stack_ffffffc8;
  char *in_stack_ffffffcc;
  char *in_stack_ffffffd0;
  int in_stack_ffffffd4;
  char *in_stack_ffffffd8;
  
  lVar2 = ___i686_get_pc_thunk_bx();
  if (lVar2 != 0xffff00000001) {
    return;
  }
  (&UNK_00004abe)[(int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
  (&UNK_00004abf)[(int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
  (&UNK_00004ac0)[(int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
  (&UNK_00004ac1)[(int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
  *(undefined4 *)
   (&UNK_00004ac2 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_00004ac6 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_00004aca + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       0x7f7fffff;
  *(undefined4 *)
   (&UNK_00004ace + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       0x7f7fffff;
  *(undefined4 *)
   (&UNK_00004ace + (int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX)
       = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4ad].m_pEntity + unaff_EBX + 2) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4ad].m_SerialNumber + unaff_EBX + 2) =
       0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4ad].m_pPrev + unaff_EBX + 2) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4ad].m_pNext + unaff_EBX + 2) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4ae].m_pEntity + unaff_EBX + 2) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4ae].m_SerialNumber + unaff_EBX + 2) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4ae].m_pPrev + unaff_EBX + 2) =
       0x7f7fffff;
  *(undefined **)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4ae].m_pNext + unaff_EBX + 2) =
       &UNK_00abcb9a + unaff_EBX;
  pDefaultValue = (char *)(unaff_EBX + 0x8ebe8e);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x312),(char *)(unaff_EBX + 0x919f33)
                 ,pDefaultValue,0,(char *)(unaff_EBX + 0x919f0e),in_stack_ffffffc8,in_stack_ffffffcc
                 ,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  uVar1 = *(undefined4 *)(&DAT_00abbab2 + unaff_EBX);
  ___cxa_atexit(unaff_EBX + 0x8dc482,0,uVar1);
  pDefaultValue_00 = (char *)(unaff_EBX + 0x8e339e);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x372),(char *)(unaff_EBX + 0x919f77)
                 ,pDefaultValue_00,0,(char *)(unaff_EBX + 0x919f52),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8dc462,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x3d2),(char *)(unaff_EBX + 0x919fb9)
                 ,pDefaultValue_00,0,(char *)(unaff_EBX + 0x919f8a),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8dc442,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x432),(char *)(unaff_EBX + 0x919ffa)
                 ,(char *)(unaff_EBX + 0x8ebd66),0,(char *)(unaff_EBX + 0x919fd2),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8dc422,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x492),(char *)(unaff_EBX + 0x91a039)
                 ,(char *)(unaff_EBX + 0x8faa22),0,(char *)(unaff_EBX + 0x91a012),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8dc402,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x4f2),(char *)(unaff_EBX + 0x91a09b)
                 ,(char *)(unaff_EBX + 0x91a097),0,(char *)(unaff_EBX + 0x91a056),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8dc3e2,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x552),(char *)(unaff_EBX + 0x91a0fb)
                 ,(char *)(unaff_EBX + 0x91a0f7),0,(char *)(unaff_EBX + 0x91a0ba),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8dc3c2,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x5b2),(char *)(unaff_EBX + 0x91a134)
                 ,pDefaultValue,0x80,(char *)(unaff_EBX + 0x91a119),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8dc3a2,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x612),(char *)(unaff_EBX + 0x91a168)
                 ,pDefaultValue_00,0x80,(char *)(unaff_EBX + 0x91a150),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8dc382,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x672),(char *)(unaff_EBX + 0x91a19a)
                 ,pDefaultValue_00,0x80,(char *)(unaff_EBX + 0x91a182),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8dc362,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x6d2),(char *)(unaff_EBX + 0x91a1cd)
                 ,pDefaultValue_00,0x80,(char *)(unaff_EBX + 0x91a1b1),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8dc342,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x732),(char *)(unaff_EBX + 0x91a203)
                 ,pDefaultValue_00,0x80,&UNK_0091a1eb + unaff_EBX,in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8dc322,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x792),(char *)(unaff_EBX + 0x91a22d)
                 ,pDefaultValue,0,(char *)(unaff_EBX + 0x91a21f),in_stack_ffffffc8,in_stack_ffffffcc
                 ,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8dc302,0,uVar1);
  pDefaultValue_01 = (char *)(unaff_EBX + 0x8f511a);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x7f2),(char *)(unaff_EBX + 0x91a271)
                 ,pDefaultValue_01,0,(char *)(unaff_EBX + 0x91a24a),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8dc2e2,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x852),(char *)(unaff_EBX + 0x91a2b3)
                 ,pDefaultValue_01,0,(char *)(unaff_EBX + 0x91a28e),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8dc2c2,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x8b2),(char *)(unaff_EBX + 0x91a31a)
                 ,pDefaultValue,0,(char *)(unaff_EBX + 0x91a2d2),in_stack_ffffffc8,in_stack_ffffffcc
                 ,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8dc2a2,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x912),(char *)(unaff_EBX + 0x91a364)
                 ,pDefaultValue_00,0,(char *)(unaff_EBX + 0x91a332),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8dc282,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x972),(char *)(unaff_EBX + 0x91a3aa)
                 ,pDefaultValue_00,0,(char *)(unaff_EBX + 0x91a382),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8dc262,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x9d2),(char *)(unaff_EBX + 0x91a3e0)
                 ,pDefaultValue_00,0,(char *)(unaff_EBX + 0x91a3d1),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8dc242,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0xa32),(char *)(unaff_EBX + 0x91a414)
                 ,pDefaultValue_01,0,(char *)(unaff_EBX + 0x91a3f6),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8dc222,0,uVar1);
  this_1 = (ConVar *)(&UNK_0091a429 + unaff_EBX);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0xa92),(char *)(unaff_EBX + 0x91a439)
                 ,(char *)(unaff_EBX + 0x8e8ffe),0,(char *)this_1,in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8dc202,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0xaf2),(char *)(unaff_EBX + 0x91a44e)
                 ,pDefaultValue,0,this_1,(char *)in_stack_ffffffc8,in_stack_ffffffcc,
                 (int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x8dc1e2,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0xb52),(char *)(unaff_EBX + 0x91a47a)
                 ,(char *)(unaff_EBX + 0x91a472),0,this_1,(char *)in_stack_ffffffc8,
                 in_stack_ffffffcc,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x8dc1c2,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0xbb2),(char *)(unaff_EBX + 0x91a4a2)
                 ,(char *)(unaff_EBX + 0x91a49c),0,this_1,(char *)in_stack_ffffffc8,
                 in_stack_ffffffcc,(int)in_stack_ffffffd0);
  ___cxa_atexit(unaff_EBX + 0x8dc1a2,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0xc12),(char *)(unaff_EBX + 0x91a4f3)
                 ,pDefaultValue,0,(char *)(unaff_EBX + 0x91a4c2),in_stack_ffffffc8,in_stack_ffffffcc
                 ,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8dc182,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0xc72),(char *)(unaff_EBX + 0x91a549)
                 ,pDefaultValue,0,(char *)(unaff_EBX + 0x91a50a),in_stack_ffffffc8,in_stack_ffffffcc
                 ,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8dc162,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0xcd2),(char *)(unaff_EBX + 0x91a5ae)
                 ,pDefaultValue,0,(char *)(unaff_EBX + 0x91a55e),in_stack_ffffffc8,in_stack_ffffffcc
                 ,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8dc142,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0xd32),(char *)(unaff_EBX + 0x91a64a)
                 ,pDefaultValue_00,0,(char *)(unaff_EBX + 0x91a5d2),in_stack_ffffffc8,
                 in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
  ___cxa_atexit(unaff_EBX + 0x8dc122,0,uVar1);
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0xd8a) = 0;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0xd8e) = 0;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0xd92) = 0;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0xd96) = 0;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0xd9a) = 0;
  ___cxa_atexit(unaff_EBX + 0x8dc4a2,0,uVar1);
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0xd9e) = 0;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0xda2) = 0;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0xda6) = 0;
  return;
}


/* __tcf_0 at 009516f0 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(&UNK_003fd6e8 + extraout_ECX),in_stack_00000008);
  return;
}


/* __tcf_1 at 009516d0 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fd768),in_stack_00000008);
  return;
}


/* __tcf_2 at 009516b0 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fd7e8),in_stack_00000008);
  return;
}


/* __tcf_3 at 00951690 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fd868),in_stack_00000008);
  return;
}


/* __tcf_4 at 00951670 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fd8e8),in_stack_00000008);
  return;
}


/* __tcf_5 at 00951650 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fd968),in_stack_00000008);
  return;
}


/* __tcf_6 at 00951630 */

void __tcf_6(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fd9e8),in_stack_00000008);
  return;
}


/* __tcf_7 at 00951610 */

void __tcf_7(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fda68),in_stack_00000008);
  return;
}


/* __tcf_8 at 009515f0 */

void __tcf_8(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fdae8),in_stack_00000008);
  return;
}


/* __tcf_9 at 009515d0 */

void __tcf_9(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fdb68),in_stack_00000008);
  return;
}


/* __tcf_10 at 009515b0 */

void __tcf_10(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fdbe8),in_stack_00000008);
  return;
}


/* __tcf_11 at 00951590 */

void __tcf_11(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fdc68),in_stack_00000008);
  return;
}


/* __tcf_12 at 00951570 */

void __tcf_12(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fdce8),in_stack_00000008);
  return;
}


/* __tcf_13 at 00951550 */

void __tcf_13(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fdd68),in_stack_00000008);
  return;
}


/* __tcf_14 at 00951530 */

void __tcf_14(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fdde8),in_stack_00000008);
  return;
}


/* __tcf_15 at 00951510 */

void __tcf_15(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fde68),in_stack_00000008);
  return;
}


/* __tcf_16 at 009514f0 */

void __tcf_16(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fdee8),in_stack_00000008);
  return;
}


/* __tcf_17 at 009514d0 */

void __tcf_17(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fdf68),in_stack_00000008);
  return;
}


/* __tcf_18 at 009514b0 */

void __tcf_18(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fdfe8),in_stack_00000008);
  return;
}


/* __tcf_19 at 00951490 */

void __tcf_19(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fe068),in_stack_00000008);
  return;
}


/* __tcf_20 at 00951470 */

void __tcf_20(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fe0e8),in_stack_00000008);
  return;
}


/* __tcf_21 at 00951450 */

void __tcf_21(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fe168),in_stack_00000008);
  return;
}


/* __tcf_22 at 00951430 */

void __tcf_22(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fe1e8),in_stack_00000008);
  return;
}


/* __tcf_23 at 00951410 */

void __tcf_23(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fe268),in_stack_00000008);
  return;
}


/* __tcf_24 at 009513f0 */

void __tcf_24(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fe2e8),in_stack_00000008);
  return;
}


/* __tcf_25 at 009513d0 */

void __tcf_25(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fe368),in_stack_00000008);
  return;
}


/* __tcf_26 at 009513b0 */

void __tcf_26(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fe3e8),in_stack_00000008);
  return;
}


/* __tcf_27 at 00951390 */

void __tcf_27(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fe468),in_stack_00000008);
  return;
}


/* __tcf_28 at 00951710 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* WARNING: Struct "CBaseViewport": ignoring multiple overlapping fields */

void __tcf_28(void *param_1)

{
  int unaff_EBX;
  undefined4 uVar1;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_bx();
  *(undefined4 *)(CBaseViewport::ShowPanel + unaff_EBX) = 0;
  if (*(int *)(unaff_EBX + 0x3fe13c) < 0) {
    uVar1 = *(undefined4 *)(CBaseViewport::ShowPanel + unaff_EBX + 4);
  }
  else {
    if (*(uint *)(CBaseViewport::ShowPanel + unaff_EBX + 4) != 0) {
                    /* Unresolved local var: void * pAlloc@[???] */
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1df600) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1df600),
                 *(undefined4 *)
                  ((*(uint *)(CBaseViewport::ShowPanel + unaff_EBX + 4) & 0xfffffffc) - 4));
      *(undefined4 *)(CBaseViewport::ShowPanel + unaff_EBX + 4) = 0;
    }
    uVar1 = 0;
    *(undefined4 *)(unaff_EBX + 0x3fe138) = 0;
  }
  *(undefined4 *)(unaff_EBX + 0x3fe144) = uVar1;
  CUtlMemoryAligned<ImpParticleWithFourInterpolants,16>::~CUtlMemoryAligned
            ((CUtlMemoryAligned<ImpParticleWithFourInterpolants,16> *)
             (CBaseViewport::ShowPanel + unaff_EBX + 4),in_stack_00000008);
  return;
}


/* NPaintRenderer::Paintblob_DrawFastSphere at 00524940 */

/* WARNING: Restarted to delay deadcode elimination for space: stack */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */

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
  fVar4 = *(float *)(unaff_EBX + 0x4922b2);
  fVar5 = *(float *)(CHudMessage::YPosition + unaff_EBX + 6);
  pfVar1 = (float *)(unaff_EBX + 0x6fee92);
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
  puVar15 = (undefined4 *)(unaff_EBX + 0x6d1492);
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
  } while (puVar15 != (undefined4 *)(unaff_EBX + 0x6d1882));
  return;
}


/* NPaintRenderer::Paintblob_DrawSpheres at 00527520 */

void NPaintRenderer::Paintblob_DrawSpheres(IMaterial *pMaterial,float flRadius)

{
  float fVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  PaintBlob_DrawSpheres_sortParticles_t *pPVar5;
  int iVar6;
  int iVar7;
  float *pfVar8;
  IMesh *pIVar9;
  int iVar10;
  PaintBlob_DrawSpheres_sortParticles_t *pPVar11;
  int *piVar12;
  int iVar13;
  Vector *center;
  PaintBlob_DrawSpheres_sortParticles_t *pPVar14;
  int unaff_EBX;
  int iVar15;
  float fVar16;
  float fVar17;
  double dVar18;
  float fVar19;
  float fVar20;
  float fVar21;
  int local_268;
  int local_248;
  int local_230;
  CMeshBuilder local_224;
  float local_3c;
  float local_38;
  float local_34;
  undefined4 local_30;
  SmartArray<NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t,false,16> local_2c;
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
  iVar7 = (**(code **)(*(int *)**(undefined4 **)(&DAT_006098c3 + unaff_EBX) + 0x34))
                    ((int *)**(undefined4 **)(&DAT_006098c3 + unaff_EBX));
  fVar20 = *(float *)(iVar7 + 0x60);
  fVar17 = *(float *)(iVar7 + 100);
  fVar1 = *(float *)(iVar7 + 0x68);
  local_2c.size = 0;
  local_2c.capacity = 0x10;
  local_2c.a = _malloc(0x80);
  iVar7 = *(int *)(unaff_EBX + 0x828333);
  if (local_2c.capacity < iVar7) {
    local_2c.capacity = iVar7 * 2;
    local_2c.a = _realloc(local_2c.a,iVar7 << 4);
    iVar7 = *(int *)(unaff_EBX + 0x828333);
  }
  if (0 < *(int *)(unaff_EBX + 0x828333)) {
    local_30 = 0;
    iVar13 = 0;
    iVar15 = 0;
    do {
      local_2c.a[iVar13].no = iVar13;
      pfVar8 = (float *)(iVar15 + *(int *)(unaff_EBX + 0x828327));
                    /* Unresolved local var: float sum@[???]
                       Unresolved local var: float temp@[???] */
      fVar19 = *pfVar8 - fVar20;
      fVar21 = pfVar8[1] - fVar17;
      fVar16 = pfVar8[2] - fVar1;
      local_2c.a[iVar13].dist = SQRT(fVar16 * fVar16 + fVar19 * fVar19 + fVar21 * fVar21);
      iVar13 = iVar13 + 1;
      iVar15 = iVar15 + 0x60;
      local_3c = fVar20;
      local_38 = fVar17;
      local_34 = fVar1;
    } while (iVar13 < *(int *)(unaff_EBX + 0x828333));
  }
  local_2c.size = iVar7;
  if (iVar7 == 2) {
    fVar20 = (local_2c.a)->dist;
    iVar7 = (local_2c.a)->no;
    iVar13 = local_2c.a[1].no;
    if (fVar20 < local_2c.a[1].dist) {
      (local_2c.a)->dist = local_2c.a[1].dist;
      (local_2c.a)->no = iVar13;
                    /* Unresolved local var: PaintBlob_DrawSpheres_sortParticles_t t@[???] */
      local_2c.a[1].dist = fVar20;
      local_2c.a[1].no = iVar7;
    }
  }
  else if (2 < iVar7) {
    iVar7 = iVar7 + -1;
    fVar20 = (local_2c.a)->dist;
    iVar13 = (local_2c.a)->no;
    fVar17 = local_2c.a[iVar7].dist;
    iVar15 = local_2c.a[iVar7].no;
    if (fVar20 < fVar17) {
      (local_2c.a)->dist = fVar17;
      (local_2c.a)->no = iVar15;
      local_2c.a[iVar7].dist = fVar20;
      local_2c.a[iVar7].no = iVar13;
      fVar20 = (local_2c.a)->dist;
      iVar13 = (local_2c.a)->no;
    }
    iVar10 = iVar7 >> 1;
    fVar17 = local_2c.a[iVar10].dist;
    iVar15 = local_2c.a[iVar10].no;
    if (fVar20 < fVar17) {
      (local_2c.a)->dist = fVar17;
      (local_2c.a)->no = iVar15;
      local_2c.a[iVar10].dist = fVar20;
      local_2c.a[iVar10].no = iVar13;
      fVar17 = local_2c.a[iVar10].dist;
      iVar15 = local_2c.a[iVar10].no;
    }
    fVar20 = local_2c.a[iVar7].dist;
    iVar13 = local_2c.a[iVar7].no;
    if (fVar17 < fVar20) {
      local_2c.a[iVar10].dist = fVar20;
      local_2c.a[iVar10].no = iVar13;
      local_2c.a[iVar7].dist = fVar17;
      local_2c.a[iVar7].no = iVar15;
    }
    if (iVar7 != 2) {
      pPVar14 = local_2c.a + iVar10;
      fVar20 = pPVar14->dist;
      iVar15 = pPVar14->no;
      iVar13 = local_2c.a[iVar7].no;
      pPVar14->dist = local_2c.a[iVar7].dist;
      pPVar14->no = iVar13;
      local_248 = -1;
      iVar10 = iVar7;
      iVar13 = local_248;
      while( true ) {
        do {
          local_248 = iVar13;
          iVar13 = local_248 + 1;
          pPVar14 = local_2c.a + iVar13;
          fVar17 = pPVar14->dist;
          iVar2 = pPVar14->no;
        } while (fVar20 < fVar17);
        pPVar11 = local_2c.a + iVar10;
        iVar6 = iVar10 * 8;
        do {
          iVar4 = iVar6;
          iVar10 = iVar10 + -1;
          pPVar5 = pPVar11 + -1;
          iVar3 = pPVar11[-1].no;
          pPVar11 = pPVar11 + -1;
          iVar6 = iVar4 + -8;
        } while (pPVar5->dist < fVar20);
        if (iVar10 <= iVar13) break;
        pPVar14->dist = pPVar5->dist;
        pPVar14->no = iVar3;
        piVar12 = (int *)((int)&local_2c.a[-1].no + iVar4);
        piVar12[1] = (int)fVar17;
        *piVar12 = iVar2;
      }
      local_2c.a[iVar7].dist = fVar17;
      local_2c.a[iVar7].no = iVar2;
      local_2c.a[iVar13].dist = fVar20;
      local_2c.a[iVar13].no = iVar15;
      if (local_248 == 1) {
        fVar20 = (local_2c.a)->dist;
        iVar13 = (local_2c.a)->no;
        iVar15 = local_2c.a[1].no;
        if (fVar20 < local_2c.a[1].dist) {
          (local_2c.a)->dist = local_2c.a[1].dist;
          (local_2c.a)->no = iVar15;
          local_2c.a[1].dist = fVar20;
          local_2c.a[1].no = iVar13;
        }
      }
      else if (1 < local_248) {
        SmartArray<NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t,false,16>::
        sort<NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t::C>(&local_2c,0,local_248);
      }
      local_248 = local_248 + 2;
      if (iVar7 - local_248 == 1) {
        pPVar14 = local_2c.a + local_248;
        fVar20 = pPVar14->dist;
        iVar13 = pPVar14->no;
        fVar17 = local_2c.a[iVar7].dist;
        iVar15 = local_2c.a[iVar7].no;
        if (fVar20 < fVar17) {
          pPVar14->dist = fVar17;
          pPVar14->no = iVar15;
          local_2c.a[iVar7].dist = fVar20;
          local_2c.a[iVar7].no = iVar13;
        }
      }
      else if (1 < iVar7 - local_248) {
        SmartArray<NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t,false,16>::
        sort<NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t::C>(&local_2c,local_248,iVar7);
      }
    }
  }
  local_20 = (int *)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x609997) + 400))
                              ((int *)**(undefined4 **)(unaff_EBX + 0x609997));
  if (local_20 != (int *)0x0) {
    (**(code **)(*local_20 + 8))(local_20);
  }
  (**(code **)(*local_20 + 0x50))(local_20,10);
  (**(code **)(*local_20 + 0x24))(local_20,pMaterial,0);
  pIVar9 = (IMesh *)(**(code **)(*local_20 + 0xe4))(local_20,1,0,0,0);
  local_268 = 0;
  do {
    if (*(int *)(unaff_EBX + 0x828333) <= local_268) {
      if (local_20 != (int *)0x0) {
        (**(code **)(*local_20 + 0xc))(local_20);
      }
      if (local_20 != (int *)0x0) {
        (**(code **)(*local_20 + 4))(local_20);
      }
      _free(local_2c.a);
      return;
    }
    local_230 = *(int *)(unaff_EBX + 0x828333) - local_268;
    if (local_230 < 0x82) {
      iVar7 = local_230 * 0xfc;
      local_230 = local_230 * 0x33;
    }
    else {
      iVar7 = 0x7ff8;
      local_230 = 0x19e6;
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
    local_224.m_pMesh = pIVar9;
    (*(pIVar9->super_IVertexBuffer)._vptr_IVertexBuffer[10])(pIVar9,2);
    (*((local_224.m_pMesh)->super_IVertexBuffer)._vptr_IVertexBuffer[0x11])
              (local_224.m_pMesh,local_230,iVar7,&local_224);
    local_224.m_IndexBuilder.m_pIndexBuffer = (IIndexBuffer *)0x0;
    if (pIVar9 != (IMesh *)0x0) {
      local_224.m_IndexBuilder.m_pIndexBuffer = &pIVar9->super_IIndexBuffer;
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
    local_224.m_IndexBuilder.m_nMaxIndexCount = iVar7;
    local_224.m_VertexBuilder.m_pVertexBuffer = (IVertexBuffer *)pIVar9;
    _memcpy(&local_224.m_VertexBuilder,&local_224,0xa4);
    local_224.m_VertexBuilder.m_nMaxVertexCount = local_230;
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
    iVar7 = local_268 << 3;
    iVar13 = 0;
    do {
      if (*(int *)(unaff_EBX + 0x828333) <= local_268) break;
      center = (Vector *)
               (*(int *)((int)&(local_2c.a)->no + iVar7) * 0x60 + *(int *)(unaff_EBX + 0x828327));
      dVar18 = (double)center[1].y;
      if (*(double *)(&DAT_0048f5c7 + unaff_EBX) <= dVar18 &&
          dVar18 != *(double *)(&DAT_0048f5c7 + unaff_EBX)) {
        Paintblob_DrawFastSphere
                  (&local_224,center,center[1].y * flRadius,center[2].z,center[3].x,center[3].y);
        iVar13 = iVar13 + 1;
      }
      local_268 = local_268 + 1;
      iVar7 = iVar7 + 8;
    } while (iVar13 < 0x82);
    iVar7 = local_224.m_IndexBuilder.m_nIndexCount;
    if (local_224.m_bGenerateIndices != false) {
      if (local_224.m_Type - MATERIAL_LINE_STRIP < 7) {
                    /* WARNING: Could not recover jumptable at 0x00527a1b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
        (*(code *)(*(int *)(unaff_EBX + 0x4ef + (local_224.m_Type - MATERIAL_LINE_STRIP) * 4) +
                  unaff_EBX))();
        return;
      }
      if (local_224.m_IndexBuilder.super_IndexDesc_t.m_nIndexSize != 0) {
        iVar13 = local_224.m_IndexBuilder.m_nMaxIndexCount -
                 local_224.m_IndexBuilder.m_nCurrentIndex;
        if (local_224.m_VertexBuilder.m_nVertexCount <=
            local_224.m_IndexBuilder.m_nMaxIndexCount - local_224.m_IndexBuilder.m_nCurrentIndex) {
          iVar13 = local_224.m_VertexBuilder.m_nVertexCount;
        }
        if (iVar13 != 0) {
          if (local_224.m_Type < MATERIAL_HETEROGENOUS) {
                    /* WARNING: Could not recover jumptable at 0x00527c26. Too many branches */
                    /* WARNING: Treating indirect jump as call */
            (*(code *)(*(int *)(unaff_EBX + 0x6f7 + local_224.m_Type * 4) + unaff_EBX))();
            return;
          }
          GenerateSequentialIndexBuffer
                    (local_224.m_IndexBuilder.super_IndexDesc_t.m_pIndices +
                     local_224.m_IndexBuilder.m_nCurrentIndex,iVar13,
                     local_224.m_IndexBuilder.m_nIndexOffset);
          local_224.m_IndexBuilder.m_nCurrentIndex =
               iVar13 * local_224.m_IndexBuilder.super_IndexDesc_t.m_nIndexSize +
               local_224.m_IndexBuilder.m_nCurrentIndex;
          iVar7 = local_224.m_IndexBuilder.m_nCurrentIndex;
          if (local_224.m_IndexBuilder.m_nCurrentIndex <= local_224.m_IndexBuilder.m_nIndexCount) {
            iVar7 = local_224.m_IndexBuilder.m_nIndexCount;
          }
        }
      }
    }
    local_224.m_IndexBuilder.m_nIndexCount = iVar7;
    (*((local_224.m_pMesh)->super_IVertexBuffer)._vptr_IVertexBuffer[0x14])
              (local_224.m_pMesh,local_224.m_VertexBuilder.m_nVertexCount,
               local_224.m_IndexBuilder.m_nIndexCount,&local_224);
    local_224.m_IndexBuilder.m_pIndexBuffer = (IIndexBuffer *)0x0;
    local_224.m_IndexBuilder.m_nMaxIndexCount = 0;
    local_224.m_VertexBuilder.m_nMaxVertexCount = 0;
    local_224.m_VertexBuilder.m_pVertexBuffer = (IVertexBuffer *)0x0;
    local_224.m_VertexBuilder.super_VertexDesc_t.m_CompressionType = VERTEX_COMPRESSION_INVALID;
    local_224.m_pMesh = (IMesh *)0x0;
    (*(pIVar9->super_IVertexBuffer)._vptr_IVertexBuffer[0xb])(pIVar9,0xffffffff,0);
    if ((local_224.m_VertexBuilder.m_pVertexBuffer != (IVertexBuffer *)0x0) &&
       (iVar7 = (*(local_224.m_VertexBuilder.m_pVertexBuffer)->_vptr_IVertexBuffer[2])
                          (local_224.m_VertexBuilder.m_pVertexBuffer), (char)iVar7 != '\0')) {
      (*(local_224.m_VertexBuilder.m_pVertexBuffer)->_vptr_IVertexBuffer[4])
                (local_224.m_VertexBuilder.m_pVertexBuffer);
    }
    if ((local_224.m_IndexBuilder.m_pIndexBuffer != (IIndexBuffer *)0x0) &&
       (iVar7 = (*(local_224.m_IndexBuilder.m_pIndexBuffer)->_vptr_IIndexBuffer[2])
                          (local_224.m_IndexBuilder.m_pIndexBuffer), (char)iVar7 != '\0')) {
      (*(local_224.m_IndexBuilder.m_pIndexBuffer)->_vptr_IIndexBuffer[4])
                (local_224.m_IndexBuilder.m_pIndexBuffer);
    }
  } while( true );
}


/* NPaintRenderer::Paintblob_SafeLightCubeUpdate at 005250d0 */

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
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x60bc77) + 0x220))
            ((int *)**(undefined4 **)(unaff_EBX + 0x60bc77),vecRenderOrigin,0,&local_88);
  cVar2 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x60bcab) + 0x4c))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x60bcab),vecRenderOrigin);
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
    iVar5 = unaff_EBX + 0x469fdf;
    if (cVar2 == '\0') {
      iVar5 = unaff_EBX + 0x43918b;
    }
    DevWarning((char *)(unaff_EBX + 0x46a00b),iVar5);
    return;
  }
  if ((((local_88 < *(float *)(unaff_EBX + 0x4924db)) &&
       (!NAN(local_88) && !NAN(*(float *)(unaff_EBX + 0x4924db)))) ||
      (local_84 < *(float *)(unaff_EBX + 0x4924db))) ||
     (local_80 < *(float *)(unaff_EBX + 0x4924db))) {
    iVar5 = unaff_EBX + 0x43918b;
    if (cVar2 != '\0') {
      iVar5 = unaff_EBX + 0x469fdf;
    }
    dVar10 = (double)local_84;
    dVar11 = (double)local_88;
    DevWarning((char *)(unaff_EBX + 0x46a03f),dVar11,dVar10,(double)local_80,iVar5);
    uVar12 = (undefined4)((ulonglong)dVar11 >> 0x20);
  }
  local_2c = *(float *)(**(int **)(unaff_EBX + 0x60bc6f) + 0x10);
  local_2c = local_2c + local_2c;
  if (*(float *)(unaff_EBX + 0x491763) <= local_2c && local_2c != *(float *)(unaff_EBX + 0x491763))
  {
    local_2c = *(float *)(unaff_EBX + 0x491763);
  }
  else if ((local_2c < *(float *)(unaff_EBX + 0x491873)) &&
          (!NAN(local_2c) && !NAN(*(float *)(unaff_EBX + 0x491873)))) {
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
  fVar9 = *(float *)(unaff_EBX + 0x4924db);
  if (((local_7c < fVar9) || (local_78 < fVar9)) || (local_74 < fVar9)) {
    iVar5 = unaff_EBX + 0x43918b;
    if (cVar2 != '\0') {
      iVar5 = unaff_EBX + 0x469fdf;
    }
    dVar10 = (double)local_78;
    dVar11 = (double)local_7c;
    local_28 = local_40;
    local_24 = local_3c;
    local_20 = local_38;
    DevWarning((char *)(unaff_EBX + 0x46a03f),dVar11,dVar10,(double)local_74,iVar5);
    uVar12 = (undefined4)((ulonglong)dVar11 >> 0x20);
  }
  pVVar3 = cachedCubeColours + 1;
  local_2c = *(float *)(**(int **)(unaff_EBX + 0x60bc6f) + 0x10);
  local_2c = local_2c + local_2c;
  if (*(float *)(unaff_EBX + 0x491763) <= local_2c && local_2c != *(float *)(unaff_EBX + 0x491763))
  {
    local_2c = *(float *)(unaff_EBX + 0x491763);
  }
  else if ((local_2c < *(float *)(unaff_EBX + 0x491873)) &&
          (!NAN(local_2c) && !NAN(*(float *)(unaff_EBX + 0x491873)))) {
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
  fVar9 = *(float *)(unaff_EBX + 0x4924db);
  if (((local_70 < fVar9) || (local_6c < fVar9)) || (local_68 < fVar9)) {
    iVar5 = unaff_EBX + 0x43918b;
    if (cVar2 != '\0') {
      iVar5 = unaff_EBX + 0x469fdf;
    }
    dVar10 = (double)local_6c;
    dVar11 = (double)local_70;
    local_28 = local_40;
    local_24 = local_3c;
    local_20 = local_38;
    DevWarning((char *)(unaff_EBX + 0x46a03f),dVar11,dVar10,(double)local_68,iVar5);
    uVar12 = (undefined4)((ulonglong)dVar11 >> 0x20);
  }
  pVVar4 = cachedCubeColours + 2;
  local_2c = *(float *)(**(int **)(unaff_EBX + 0x60bc6f) + 0x10);
  local_2c = local_2c + local_2c;
  if (*(float *)(unaff_EBX + 0x491763) <= local_2c && local_2c != *(float *)(unaff_EBX + 0x491763))
  {
    local_2c = *(float *)(unaff_EBX + 0x491763);
  }
  else if ((local_2c < *(float *)(unaff_EBX + 0x491873)) &&
          (!NAN(local_2c) && !NAN(*(float *)(unaff_EBX + 0x491873)))) {
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
  fVar9 = *(float *)(unaff_EBX + 0x4924db);
  if (((local_64 < fVar9) || (local_60 < fVar9)) || (local_5c < fVar9)) {
    iVar5 = unaff_EBX + 0x43918b;
    if (cVar2 != '\0') {
      iVar5 = unaff_EBX + 0x469fdf;
    }
    dVar10 = (double)local_60;
    dVar11 = (double)local_64;
    local_28 = local_40;
    local_24 = local_3c;
    local_20 = local_38;
    DevWarning((char *)(unaff_EBX + 0x46a03f),dVar11,dVar10,(double)local_5c,iVar5);
    uVar12 = (undefined4)((ulonglong)dVar11 >> 0x20);
  }
  pVVar8 = cachedCubeColours + 3;
  local_2c = *(float *)(**(int **)(unaff_EBX + 0x60bc6f) + 0x10);
  local_2c = local_2c + local_2c;
  if (*(float *)(unaff_EBX + 0x491763) <= local_2c && local_2c != *(float *)(unaff_EBX + 0x491763))
  {
    local_2c = *(float *)(unaff_EBX + 0x491763);
  }
  else if ((local_2c < *(float *)(unaff_EBX + 0x491873)) &&
          (!NAN(local_2c) && !NAN(*(float *)(unaff_EBX + 0x491873)))) {
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
  fVar9 = *(float *)(unaff_EBX + 0x4924db);
  if (((local_58 < fVar9) || (local_54 < fVar9)) || (local_50 < fVar9)) {
    iVar5 = unaff_EBX + 0x43918b;
    if (cVar2 != '\0') {
      iVar5 = unaff_EBX + 0x469fdf;
    }
    dVar10 = (double)local_54;
    dVar11 = (double)local_58;
    local_28 = local_40;
    local_24 = local_3c;
    local_20 = local_38;
    DevWarning((char *)(unaff_EBX + 0x46a03f),dVar11,dVar10,(double)local_50,iVar5);
    uVar12 = (undefined4)((ulonglong)dVar11 >> 0x20);
  }
  pVVar7 = cachedCubeColours + 4;
  local_2c = *(float *)(**(int **)(unaff_EBX + 0x60bc6f) + 0x10);
  local_2c = local_2c + local_2c;
  if (*(float *)(unaff_EBX + 0x491763) <= local_2c && local_2c != *(float *)(unaff_EBX + 0x491763))
  {
    local_2c = *(float *)(unaff_EBX + 0x491763);
  }
  else if ((local_2c < *(float *)(unaff_EBX + 0x491873)) &&
          (!NAN(local_2c) && !NAN(*(float *)(unaff_EBX + 0x491873)))) {
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
  if (((local_4c < *(float *)(unaff_EBX + 0x4924db)) ||
      (local_48 < *(float *)(unaff_EBX + 0x4924db))) ||
     (local_44 < *(float *)(unaff_EBX + 0x4924db))) {
    iVar5 = unaff_EBX + 0x469fdf;
    if (cVar2 == '\0') {
      iVar5 = unaff_EBX + 0x43918b;
    }
    dVar10 = (double)local_48;
    dVar11 = (double)local_4c;
    local_28 = local_40;
    local_24 = local_3c;
    local_20 = local_38;
    DevWarning((char *)(unaff_EBX + 0x46a03f),dVar11,dVar10,(double)local_44,iVar5);
    uVar12 = (undefined4)((ulonglong)dVar11 >> 0x20);
  }
  pVVar6 = cachedCubeColours + 5;
  fVar9 = *(float *)(**(int **)(unaff_EBX + 0x60bc6f) + 0x10);
  fVar9 = fVar9 + fVar9;
  pfVar1 = (float *)(unaff_EBX + 0x491763);
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
        ((fVar9 = *(float *)(&DAT_00491fef + unaff_EBX), fVar9 <= cachedCubeColours->x &&
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
  iVar5 = unaff_EBX + 0x469fdf;
  if (cVar2 == '\0') {
    iVar5 = unaff_EBX + 0x43918b;
  }
  local_28 = local_40;
  local_24 = local_3c;
  local_20 = local_38;
  DevWarning((char *)(unaff_EBX + 0x46a06f),iVar5,uVar12,dVar10);
  cachedCubeColours->x = 0.0;
  return;
}


/* NPaintRenderer::GetDrawMaterial at 00524c30 */

IMaterial * NPaintRenderer::GetDrawMaterial(void)

{
  IMaterial *pIVar1;
  char *pcVar2;
  int iVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((*(int *)(*(int *)(unaff_EBX + 0x82a878) + 0x30) == 0) &&
     (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x60c508) + 0x1c) + 0x30) == 0)) {
    if ((*(uint *)(CSOAContainer::CopyRegionFrom + unaff_EBX) & 0x1000) == 0) {
      pcVar2 = *(char **)(*(int *)(unaff_EBX + 0x82a938) + 0x24);
      if (pcVar2 == (char *)0x0) {
        pcVar2 = (char *)(unaff_EBX + 0x439628);
      }
    }
    else {
      pcVar2 = (char *)(unaff_EBX + 0x463d1e);
    }
    if (*pcVar2 == '\0') {
      pIVar1 = (IMaterial *)0x0;
      if (*(uint *)(*(int *)(unaff_EBX + 0x82a8d8) + 0x30) < 0xc) {
                    /* WARNING: Could not recover jumptable at 0x00524dbc. Too many branches */
                    /* WARNING: Treating indirect jump as call */
        pIVar1 = (IMaterial *)
                 (*(code *)(*(int *)(unaff_EBX + 0x17c +
                                    *(int *)(*(int *)(unaff_EBX + 0x82a8d8) + 0x30) * 4) + unaff_EBX
                           ))();
        return pIVar1;
      }
    }
    else {
                    /* Unresolved local var: char * str@[???] */
      iVar3 = unaff_EBX + 0x463d1e;
      if ((*(uint *)(CSOAContainer::CopyRegionFrom + unaff_EBX) & 0x1000) == 0) {
        iVar3 = *(int *)(*(int *)(unaff_EBX + 0x82a938) + 0x24);
        if (iVar3 == 0) {
          iVar3 = unaff_EBX + 0x439628;
        }
      }
      pIVar1 = (IMaterial *)
               (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x60c284) + 300))
                         ((int *)**(undefined4 **)(unaff_EBX + 0x60c284),iVar3,unaff_EBX + 0x457fd4,
                          1,0);
    }
  }
  else if (*(int *)(*(int *)(unaff_EBX + 0x82a8d8) + 0x30) < 0) {
    pIVar1 = (IMaterial *)
             (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x60c284) + 300))
                       ((int *)**(undefined4 **)(unaff_EBX + 0x60c284),unaff_EBX + 0x46922c,
                        unaff_EBX + 0x457fd4,1,0);
  }
  else {
    pIVar1 = (IMaterial *)
             (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x60c284) + 300))
                       ((int *)**(undefined4 **)(unaff_EBX + 0x60c284),unaff_EBX + 0x46a2ec,
                        unaff_EBX + 0x457fd4,1,0);
  }
  return pIVar1;
}


/* NPaintRenderer::SetUpImpRendererUserDefinedFuncs at 005250c0 */

void NPaintRenderer::SetUpImpRendererUserDefinedFuncs(void)

{
  return;
}


/* NPaintRenderer::TransformParticles at 00525f00 */

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
    iVar2 = *(int *)(unaff_EBX + 0x60b653);
    i = 0;
    fVar1 = *(float *)(&DAT_004909fb + unaff_EBX);
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
      fVar10 = (float)(~uVar3 & *(uint *)(unaff_EBX + 0x490933) | (uint)fVar10 & uVar3);
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
           (float)((uint)fVar10 ^ *(uint *)(unaff_EBX + 0x4957af)) - fVar4 * 0.0;
      (blobParticles->interpolants3).field_0.sse_vec[1] = fVar4 * 0.0 + fVar9;
      (blobParticles->interpolants3).field_0.sse_vec[2] = fVar9 * 0.0 - fVar10 * 0.0;
      (blobParticles->interpolants3).field_0.sse_vec[3] = 0.0;
      fVar8 = (blobParticles->interpolants3).field_0.sse_vec[0];
      fVar6 = (blobParticles->interpolants3).field_0.sse_vec[1];
      fVar7 = (blobParticles->interpolants3).field_0.sse_vec[2];
      fVar5 = SQRT(fVar8 * fVar8 + fVar6 * fVar6 + fVar7 * fVar7);
      uVar3 = -(uint)(fVar5 != 0.0);
      fVar5 = (float)(~uVar3 & *(uint *)(unaff_EBX + 0x490933) | (uint)fVar5 & uVar3);
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
      fVar8 = (float)(~uVar3 & *(uint *)(unaff_EBX + 0x490933) | (uint)fVar8 & uVar3);
      (blobParticles->interpolants4).field_0.sse_vec[0] = fVar10 / fVar8;
      (blobParticles->interpolants4).field_0.sse_vec[1] = fVar4 / fVar8;
      (blobParticles->interpolants4).field_0.sse_vec[2] = fVar9 / fVar8;
      i = i + 1;
      blobParticles = blobParticles + 1;
    } while (i != blobCount);
  }
  return;
}


/* NPaintRenderer::Paintblob_DrawIsoSurface at 005264d0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* WARNING: Struct "CPolygonButton": ignoring multiple overlapping fields */

void NPaintRenderer::Paintblob_DrawIsoSurface
               (IClientRenderable *pClientRenderable,Vector *vecRenderOrigin,IMaterial *pMaterial,
               float flCubeWidth,view_id_t viewID,ImpParticleWithFourInterpolants *blobParticles,
               int blobCount)

{
  uint *puVar1;
  float *pfVar2;
  char cVar3;
  IMaterial *pIVar4;
  int iVar5;
  int *piVar6;
  IBlobRenderer *pIVar7;
  int iVar8;
  ImpTiler *this;
  int unaff_EBX;
  float fVar9;
  VMatrix *local_2f0;
  float local_2ec;
  float local_2e8;
  VMatrix local_2dc;
  VMatrix local_29c;
  VMatrix local_25c;
  VMatrix local_21c;
  undefined4 local_1dc;
  undefined4 local_1d8;
  undefined4 local_1d4;
  undefined4 local_1d0;
  float local_1cc;
  float local_1c8;
  float local_1c4;
  float local_1c0;
  float local_1bc;
  float local_1b8;
  float local_1b4;
  float local_1b0;
  float local_1ac;
  float local_1a8;
  float local_1a4;
  float local_1a0;
  float local_19c;
  float local_198;
  float local_194;
  float local_190;
  float local_18c;
  float local_188;
  float local_184;
  float local_180;
  __349 local_17c;
  __349 local_16c;
  __349 local_15c;
  float local_140;
  float local_13c;
  float local_138;
  float local_134;
  float local_130;
  float local_12c;
  float local_128;
  float local_124;
  float local_120;
  float local_11c;
  float local_118;
  float local_114;
  float local_110;
  float local_10c;
  float local_108;
  float local_104;
  float local_100;
  float local_fc;
  undefined4 local_f8;
  float local_f4;
  undefined4 local_f0;
  float local_ec;
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
  float local_b8;
  float local_b4;
  float local_b0;
  float local_ac;
  float local_a8;
  undefined4 local_a4;
  float local_a0;
  undefined4 local_9c;
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
  Vector local_50;
  Vector local_44;
  Vector local_38;
  Vector local_2c;
  int *local_20 [4];
  
                    /* Unresolved local var: CVProfScope VProf_@[???]
                       Unresolved local var: IMaterial * pSpecialMaterial@[???]
                       Unresolved local var: CMatRenderContextPtr pRenderContext@[???]
                       Unresolved local var: Vector transformedCenter@[???]
                       Unresolved local var: VMatrix rotationMatrix@[???]
                       Unresolved local var: VMatrix invRotationMatrix@[???]
                       Unresolved local var: Vector transformedEye@[???]
                       Unresolved local var: float angle@[???]
                       Unresolved local var: float flPaintResolution@[???]
                       Unresolved local var: float flMaxPaintScale@[???]
                       Unresolved local var: float flPaintResolutionLow@[???] */
  ___i686_get_pc_thunk_bx();
  if (((*(int *)(*(int *)(unaff_EBX + 0x60a847) + 0x100c) != 0) ||
      (*(char *)(*(int *)(unaff_EBX + 0x60a847) + 0x1010) == '\0')) &&
     (iVar5 = *(int *)(*(int *)(unaff_EBX + 0x60a847) + 0x19b8), iVar8 = _ThreadGetCurrentId(),
     iVar5 == iVar8)) {
    iVar5 = *(int *)(unaff_EBX + 0x60a847);
    piVar6 = *(int **)(&DAT_00001014 + iVar5);
    if (*piVar6 != unaff_EBX + 0x4693fb) {
      piVar6 = (int *)CVProfNode::GetSubNode
                                ((char *)piVar6,unaff_EBX + 0x4693fb,(char *)0x0,
                                 unaff_EBX + 0x469414);
      *(int **)(&DAT_00001014 + iVar5) = piVar6;
    }
    puVar1 = (uint *)(piVar6[0x1c] * 8 + *(int *)(_GLOBAL__I_srand + *(int *)(unaff_EBX + 0x60a847))
                     + 4);
    *puVar1 = *puVar1 | 4;
    CVProfNode::EnterScope();
    *(undefined1 *)(*(int *)(unaff_EBX + 0x60a847) + 0x1010) = 0;
  }
  pIVar4 = GetDrawMaterial();
  if ((pIVar4 != (IMaterial *)0x0) &&
     (iVar5 = (*pIVar4->_vptr_IMaterial[0x2a])(pIVar4), (char)iVar5 == '\0')) {
    pMaterial = pIVar4;
  }
  local_20[0] = (int *)(**(code **)(*(int *)**(undefined4 **)
                                              (CRadialMenu::ShowPanel + unaff_EBX + 7) + 400))
                                 ((int *)**(undefined4 **)(CRadialMenu::ShowPanel + unaff_EBX + 7));
  if (local_20[0] != (int *)0x0) {
    (**(code **)(*local_20[0] + 8))(local_20[0]);
  }
  (**(code **)(*local_20[0] + 0x50))(local_20[0],10);
  (**(code **)(*local_20[0] + 0x24))(local_20[0],pMaterial,pClientRenderable);
  (**(code **)(*local_20[0] + 0x54))(local_20[0]);
  (**(code **)(*local_20[0] + 0x7c))(local_20[0]);
  local_2c.x = vecRenderOrigin->x;
  local_2c.y = vecRenderOrigin->y;
  local_2c.z = vecRenderOrigin->z;
  (**(code **)(*local_20[0] + 0x90))
            (local_20[0],vecRenderOrigin->x,vecRenderOrigin->y,vecRenderOrigin->z);
  if (*(int *)(*(int *)(unaff_EBX + 0x82897b) + 0x30) == 0) {
    MatrixSetIdentity(&local_21c);
    MatrixSetIdentity(&local_25c);
    local_38.x = 0.0;
    local_38.y = 0.0;
    local_38.z = 0.0;
    local_2ec = 0.0;
  }
  else {
    iVar5 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x60a913) + 0x34))
                      ((int *)**(undefined4 **)(unaff_EBX + 0x60a913));
    local_2ec = *(float *)(unaff_EBX + 0x49056f) + *(float *)(iVar5 + 0x70);
    if (*(int *)(*(int *)(unaff_EBX + 0x8289db) + 0x30) != 0) {
      local_2ec = (float)((int)((local_2ec + *(float *)(unaff_EBX + 0x4905c3)) /
                               *(float *)(unaff_EBX + 0x4905d3)) * 0x5a);
    }
    (**(code **)(*local_20[0] + 0x8c))(local_20[0],local_2ec,0,0,0x3f800000);
    local_44.x = 0.0;
    local_44.y = 0.0;
    local_44.z = 1.0;
    SetupMatrixAxisRot(&local_29c,&local_44,
                       (vec_t)((uint)local_2ec ^ *(uint *)(unaff_EBX + 0x4951df)));
    VMatrix::operator=(&local_21c,&local_29c);
    local_50.x = 0.0;
    local_50.y = 0.0;
    local_50.z = 1.0;
    SetupMatrixAxisRot(&local_2dc,&local_50,local_2ec);
    VMatrix::operator=(&local_25c,&local_2dc);
    iVar5 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x60a913) + 0x34))
                      ((int *)**(undefined4 **)(unaff_EBX + 0x60a913));
                    /* Unresolved local var: Vector res@[???] */
    local_5c = *(float *)(iVar5 + 0x60) - vecRenderOrigin->x;
    local_58 = *(float *)(iVar5 + 100) - vecRenderOrigin->y;
    local_54 = *(float *)(iVar5 + 0x68) - vecRenderOrigin->z;
    local_108 = local_5c * local_21c.m[2][0] + local_58 * local_21c.m[2][1] +
                local_54 * local_21c.m[2][2];
    local_10c = local_5c * local_21c.m[1][0] + local_58 * local_21c.m[1][1] +
                local_54 * local_21c.m[1][2];
    local_110 = local_5c * local_21c.m[0][0] + local_58 * local_21c.m[0][1] +
                local_54 * local_21c.m[0][2];
    local_104 = local_110;
    local_100 = local_10c;
    local_fc = local_108;
    local_68 = local_110;
    local_64 = local_10c;
    local_60 = local_108;
    local_38.x = local_110;
    local_38.y = local_10c;
    local_38.z = local_108;
  }
  local_2f0 = &local_21c;
  local_2e8 = *(float *)(*(int *)(unaff_EBX + 0x828a9b) + 0x2c);
  fVar9 = local_2e8 * *(float *)(&DAT_0049379f + unaff_EBX);
  if (*(int *)(*(int *)(unaff_EBX + 0x82927b) + 0x30) == 0) {
    local_2e8 = fVar9;
  }
  if ((*(int *)(*(int *)(unaff_EBX + 0x82933b) + 0x30) == 0) || (viewID == VIEW_MAIN)) {
    if ((*(int *)(*(int *)(unaff_EBX + 0x8292db) + 0x30) != 0) && (viewID != VIEW_MAIN)) {
      local_2e8 = fVar9;
    }
    TransformParticles(local_2ec,vecRenderOrigin,&local_2c,&local_38,local_2f0,&local_25c,
                       blobParticles,blobCount);
    if (*(int *)(*(int *)(unaff_EBX + 0x828d9b) + 0x30) == 0) {
                    /* Unresolved local var: IBlobRenderer * sweepRenderer@[???] */
      pIVar7 = ImpRendererFactory::getRenderer
                         ((ImpRendererFactory *)**(undefined4 **)(unaff_EBX + 0x60b07f));
      (*pIVar7->_vptr_IBlobRenderer[9])
                (pIVar7,flCubeWidth * *(float *)(*(int *)(unaff_EBX + 0x828a3b) + 0x2c) * local_2e8)
      ;
      (*pIVar7->_vptr_IBlobRenderer[0xb])
                (pIVar7,flCubeWidth * *(float *)(*(int *)(unaff_EBX + 0x828a3b) + 0x2c) *
                        *(float *)(*(int *)(unaff_EBX + 0x828afb) + 0x2c));
      (*pIVar7->_vptr_IBlobRenderer[0xd])
                (pIVar7,flCubeWidth * *(float *)(*(int *)(unaff_EBX + 0x828a3b) + 0x2c) *
                        *(float *)(*(int *)(unaff_EBX + 0x828b5b) + 0x2c));
      (*pIVar7->_vptr_IBlobRenderer[2])
                (pIVar7,(uint)(*(int *)(*(int *)(unaff_EBX + 0x828f1b) + 0x30) == 0),local_20);
      local_1dc = 0;
      local_1d8 = 0;
      local_1d4 = 0;
      local_1d0 = 0;
      (*pIVar7->_vptr_IBlobRenderer[7])(pIVar7,&local_1dc);
      (*pIVar7->_vptr_IBlobRenderer[4])(pIVar7,0);
                    /* Unresolved local var: int i@[???] */
      if (0 < blobCount) {
        iVar5 = 0;
        do {
          (*pIVar7->_vptr_IBlobRenderer[6])(pIVar7,blobParticles,0);
          iVar5 = iVar5 + 1;
          blobParticles = blobParticles + 1;
        } while (iVar5 != blobCount);
      }
      (*pIVar7->_vptr_IBlobRenderer[5])(pIVar7);
      (*pIVar7->_vptr_IBlobRenderer[3])(pIVar7);
      ImpRendererFactory::returnRenderer
                ((ImpRendererFactory *)**(undefined4 **)(unaff_EBX + 0x60b07f),pIVar7);
    }
    else {
      this = ImpTilerFactory::getTiler((ImpTilerFactory *)**(undefined4 **)(unaff_EBX + 0x60b087));
      pIVar7 = this->m_sweepRenderer;
      (*pIVar7->_vptr_IBlobRenderer[9])
                (pIVar7,flCubeWidth * *(float *)(*(int *)(unaff_EBX + 0x828a3b) + 0x2c) * local_2e8)
      ;
      (*pIVar7->_vptr_IBlobRenderer[0xb])
                (pIVar7,flCubeWidth * *(float *)(*(int *)(unaff_EBX + 0x828a3b) + 0x2c) *
                        *(float *)(*(int *)(unaff_EBX + 0x828afb) + 0x2c));
      (*pIVar7->_vptr_IBlobRenderer[0xd])
                (pIVar7,flCubeWidth * *(float *)(*(int *)(unaff_EBX + 0x828a3b) + 0x2c) *
                        *(float *)(*(int *)(unaff_EBX + 0x828b5b) + 0x2c));
      this->maxNoTilesToDraw = *(int *)(*(int *)(&DAT_00828dfb + unaff_EBX) + 0x30);
      local_15c.sse_vec[0] = 0.0;
      local_15c.sse_vec[1] = 0.0;
      local_15c.sse_vec[2] = 0.0;
      local_15c.sse_vec[3] = 0.0;
      ImpTiler::beginFrame
                (this,(Point3D *)&local_15c,local_20,
                 *(int *)(*(int *)(unaff_EBX + 0x828f1b) + 0x30) == 0);
                    /* Unresolved local var: int i@[???] */
      if (0 < blobCount) {
        iVar5 = 0;
        do {
          ImpTiler::insertParticle(this,(ImpParticle *)blobParticles);
          iVar5 = iVar5 + 1;
          blobParticles = blobParticles + 1;
        } while (iVar5 != blobCount);
      }
      if (*(int *)(*(int *)(unaff_EBX + 0x828ebb) + 0x30) == 0) {
        ImpTiler::drawTile(this,0,0,0);
        if ((0 < *(int *)(*(int *)(&DAT_00828dfb + unaff_EBX) + 0x30)) ||
           (*(int *)(*(int *)(unaff_EBX + 0x828ebb) + 0x30) == 0)) {
                    /* Unresolved local var: Vector overlayCenter@[???]
                       Unresolved local var: Vector mins@[???]
                       Unresolved local var: Vector maxs@[???] */
          local_1ac = (this->last_tiles_offset).field_0.sse_vec[0];
          local_1a8 = (this->last_tiles_offset).field_0.sse_vec[1];
          local_1a4 = (this->last_tiles_offset).field_0.sse_vec[2];
          local_1a0 = (this->last_tiles_offset).field_0.sse_vec[3];
          local_138 = local_1ac * local_25c.m[2][0] + local_1a8 * local_25c.m[2][1] +
                      local_1a4 * local_25c.m[2][2];
          local_13c = local_1ac * local_25c.m[1][0] + local_1a8 * local_25c.m[1][1] +
                      local_1a4 * local_25c.m[1][2];
          local_140 = local_1ac * local_25c.m[0][0] + local_1a8 * local_25c.m[0][1] +
                      local_1a4 * local_25c.m[0][2];
          local_1bc = (this->render_dim).field_0.sse_vec[0];
          local_1b8 = (this->render_dim).field_0.sse_vec[1];
          local_1b4 = (this->render_dim).field_0.sse_vec[2];
          local_1b0 = (this->render_dim).field_0.sse_vec[3];
                    /* Unresolved local var: Vector res@[???] */
          local_c0 = *(float *)(&DAT_004907af + unaff_EBX);
          local_c8 = local_1bc * local_c0;
          local_c4 = local_1b8 * local_c0;
          local_c0 = local_c0 * local_1b4;
          local_1cc = (this->render_dim).field_0.sse_vec[0];
          local_1c8 = (this->render_dim).field_0.sse_vec[1];
          local_1c4 = (this->render_dim).field_0.sse_vec[2];
          local_1c0 = (this->render_dim).field_0.sse_vec[3];
                    /* Unresolved local var: Vector res@[???] */
          local_d8 = *(float *)(&DAT_004903ef + unaff_EBX);
          local_e0 = local_1cc * local_d8;
          local_dc = local_1c8 * local_d8;
          local_d8 = local_d8 * local_1c4;
          local_f8 = 0;
          local_f4 = local_2ec;
          local_f0 = 0;
                    /* Unresolved local var: Vector res@[???] */
          local_ec = local_140 + vecRenderOrigin->x;
          local_e8 = local_13c + vecRenderOrigin->y;
          local_e4 = local_138 + vecRenderOrigin->z;
          local_134 = local_140;
          local_130 = local_13c;
          local_12c = local_138;
          local_d4 = local_e0;
          local_d0 = local_dc;
          local_cc = local_d8;
          local_bc = local_c8;
          local_b8 = local_c4;
          local_b4 = local_c0;
          local_b0 = local_140;
          local_ac = local_13c;
          local_a8 = local_138;
          (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x60a94b) + 4))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x60a94b),&local_ec,&local_bc,&local_d4,
                     &local_f8,0,0xff,0,0,0);
        }
      }
      else {
        if (*(int *)(*(int *)(unaff_EBX + 0x82897b) + 0x30) == 0) {
          ImpTiler::drawSurface(this);
        }
        else {
          local_16c.sse_vec[0] = local_38.x;
          local_16c.sse_vec[1] = local_38.y;
          local_16c.sse_vec[2] = local_38.z;
          local_16c.sse_vec[3] = 0.0;
          ImpTiler::drawSurfaceSorted(this,(Point3D *)&local_16c);
        }
        if (*(int *)(*(int *)(unaff_EBX + 0x828f7b) + 0x30) != 0) {
                    /* Unresolved local var: int i@[???]
                       Unresolved local var: Vector overlayCenter@[???]
                       Unresolved local var: Vector mins@[???]
                       Unresolved local var: Vector maxs@[???] */
          for (iVar5 = 0; iVar8 = ImpTiler::getNoTiles(this), iVar5 < iVar8; iVar5 = iVar5 + 1) {
            ImpTiler::getTileOffset((Point3D *)&local_17c,this,iVar5);
            local_120 = local_17c.sse_vec[0] * local_25c.m[2][0] +
                        local_17c.sse_vec[1] * local_25c.m[2][1] +
                        local_17c.sse_vec[2] * local_25c.m[2][2];
            local_124 = local_17c.sse_vec[0] * local_25c.m[1][0] +
                        local_17c.sse_vec[1] * local_25c.m[1][1] +
                        local_17c.sse_vec[2] * local_25c.m[1][2];
            local_128 = local_17c.sse_vec[0] * local_25c.m[0][0] +
                        local_17c.sse_vec[1] * local_25c.m[0][1] +
                        local_17c.sse_vec[2] * local_25c.m[0][2];
            local_18c = (this->render_dim).field_0.sse_vec[0];
            local_188 = (this->render_dim).field_0.sse_vec[1];
            local_184 = (this->render_dim).field_0.sse_vec[2];
            local_180 = (this->render_dim).field_0.sse_vec[3];
                    /* Unresolved local var: Vector res@[???] */
            pfVar2 = (float *)(&DAT_004907af + unaff_EBX);
            local_d4 = *pfVar2 * local_18c;
            local_d0 = *pfVar2 * local_188;
            local_cc = *pfVar2 * local_184;
            local_19c = (this->render_dim).field_0.sse_vec[0];
            local_198 = (this->render_dim).field_0.sse_vec[1];
            local_194 = (this->render_dim).field_0.sse_vec[2];
            local_190 = (this->render_dim).field_0.sse_vec[3];
                    /* Unresolved local var: Vector res@[???] */
            pfVar2 = (float *)(&DAT_004903ef + unaff_EBX);
            local_bc = *pfVar2 * local_19c;
            local_b8 = *pfVar2 * local_198;
            local_b4 = *pfVar2 * local_194;
            local_a4 = 0;
            local_a0 = local_2ec;
            local_9c = 0;
                    /* Unresolved local var: Vector res@[???] */
            local_98 = local_128 + vecRenderOrigin->x;
            local_94 = local_124 + vecRenderOrigin->y;
            local_90 = local_120 + vecRenderOrigin->z;
            local_11c = local_128;
            local_118 = local_124;
            local_114 = local_120;
            local_8c = local_bc;
            local_88 = local_b8;
            local_84 = local_b4;
            local_80 = local_d4;
            local_7c = local_d0;
            local_78 = local_cc;
            local_74 = local_128;
            local_70 = local_124;
            local_6c = local_120;
            (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x60a94b) + 4))
                      ((int *)**(undefined4 **)(unaff_EBX + 0x60a94b),&local_98,&local_d4,&local_bc,
                       &local_a4,0,0xff,0,0,0);
          }
        }
      }
      ImpTiler::endFrame(this);
      ImpTilerFactory::returnTiler((ImpTilerFactory *)**(undefined4 **)(unaff_EBX + 0x60b087),this);
    }
    (**(code **)(*local_20[0] + 0x58))(local_20[0]);
    if (local_20[0] != (int *)0x0) {
      (**(code **)(*local_20[0] + 0xc))(local_20[0]);
    }
  }
  else if (local_20[0] != (int *)0x0) {
    (**(code **)(*local_20[0] + 0xc))(local_20[0]);
  }
  if (local_20[0] != (int *)0x0) {
    (**(code **)(*local_20[0] + 4))(local_20[0]);
  }
  if (((*(char *)(*(int *)(unaff_EBX + 0x60a847) + 0x1010) == '\0') ||
      (*(int *)(*(int *)(unaff_EBX + 0x60a847) + 0x100c) != 0)) &&
     (iVar5 = *(int *)(*(int *)(unaff_EBX + 0x60a847) + 0x19b8), iVar8 = _ThreadGetCurrentId(),
     iVar5 == iVar8)) {
    cVar3 = CVProfNode::ExitScope();
    if (cVar3 != '\0') {
      *(undefined4 *)(&DAT_00001014 + *(int *)(unaff_EBX + 0x60a847)) =
           *(undefined4 *)(*(int *)(&DAT_00001014 + *(int *)(unaff_EBX + 0x60a847)) + 100);
    }
    iVar5 = *(int *)(unaff_EBX + 0x60a847);
    *(bool *)(iVar5 + 0x1010) = *(undefined **)(&DAT_00001014 + iVar5) == &DAT_00001018 + iVar5;
    return;
  }
  return;
}


/* NPaintRenderer::Paintblob_CullOutOfViewParticles at 005263a0 */

/* WARNING: Restarted to delay deadcode elimination for space: stack */

void NPaintRenderer::Paintblob_CullOutOfViewParticles(void)

{
  float fVar1;
  float fVar2;
  bool bVar3;
  vec_t *pvVar4;
  VPlane *pPlanes;
  int iVar5;
  int unaff_EBX;
  vec_t *pvVar6;
  int iParticlesRemoved;
  int i;
  Vector local_28 [2];
  
  ___i686_get_pc_thunk_bx();
  if (0 < *(int *)(unaff_EBX + 0x8294b6)) {
    iParticlesRemoved = 0;
    i = 0;
    do {
                    /* Unresolved local var: ImpParticleWithOneInterpolant * imp_particle@[???]
                       Unresolved local var: Vector vCenter@[???] */
      pvVar4 = (vec_t *)(i * 0x60 + *(int *)(unaff_EBX + 0x8294aa));
      local_28[0].z = pvVar4[2];
      local_28[0].y = pvVar4[1];
      local_28[0].x = *pvVar4;
      fVar1 = *(float *)(unaff_EBX + 0x490922);
      fVar2 = pvVar4[4];
      pPlanes = (VPlane *)
                (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x60aa46) + 0x28))
                          ((int *)**(undefined4 **)(unaff_EBX + 0x60aa46));
      bVar3 = R_CullSphere(pPlanes,5,local_28,fVar1 * fVar2);
      if (bVar3) {
                    /* Unresolved local var: ImpParticleWithOneInterpolant * last_particle@[???] */
        pvVar6 = (vec_t *)((~iParticlesRemoved + *(int *)(unaff_EBX + 0x8294b6)) * 0x60 +
                          *(int *)(unaff_EBX + 0x8294aa));
        for (iVar5 = 0xc; iVar5 != 0; iVar5 = iVar5 + -1) {
          *pvVar4 = *pvVar6;
          pvVar6 = pvVar6 + 1;
          pvVar4 = pvVar4 + 1;
        }
        iParticlesRemoved = iParticlesRemoved + 1;
        i = i + -1;
      }
      i = i + 1;
      iVar5 = *(int *)(unaff_EBX + 0x8294b6) - iParticlesRemoved;
    } while (i < iVar5);
    if (iParticlesRemoved != 0) {
      CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants,_16>_>
      ::SetCountNonDestructively
                ((CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants,_16>_>
                  *)(unaff_EBX + 0x8294aa),iVar5);
    }
  }
  return;
}


/* NPaintRenderer::Paintblob_Draw at 005280d0 */

/* WARNING: Restarted to delay deadcode elimination for space: stack */

void NPaintRenderer::Paintblob_Draw
               (IClientRenderable *pClientRenderable,Vector *vecRenderOrigin,IMaterial *pMaterial,
               float flCubeWidth,bool bSurfaceNoParticleCull)

{
  float fVar1;
  float fVar2;
  void *pvVar3;
  code *pcVar4;
  ImpParticleWithFourInterpolants *blobParticles;
  bool bVar5;
  VPlane *pPlanes;
  undefined4 *puVar6;
  view_id_t vVar7;
  int *piVar8;
  void *pvVar9;
  int iVar10;
  uint uVar11;
  int *piVar12;
  int unaff_EBX;
  vec_t *pvVar13;
  vec_t *pvVar14;
  void *pvVar15;
  void **local_70;
  void *local_68;
  void **local_64;
  uint local_54;
  int local_50;
  Vector local_30;
  void *local_24;
  void *local_20 [4];
  
  ___i686_get_pc_thunk_bx();
  if ((!bSurfaceNoParticleCull) && (0 < *(int *)(unaff_EBX + 0x827783))) {
    local_54 = 0;
    local_50 = 0;
    do {
      pvVar13 = (vec_t *)(local_50 * 0x60 + *(int *)(unaff_EBX + 0x827777));
      local_30.z = pvVar13[2];
      local_30.y = pvVar13[1];
      local_30.x = *pvVar13;
      fVar1 = *(float *)(unaff_EBX + 0x48ebef);
      fVar2 = pvVar13[4];
      pPlanes = (VPlane *)
                (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x608d13) + 0x28))
                          ((int *)**(undefined4 **)(unaff_EBX + 0x608d13));
      bVar5 = R_CullSphere(pPlanes,5,&local_30,fVar1 * fVar2);
      if (bVar5) {
                    /* Unresolved local var: ImpParticleWithOneInterpolant * last_particle@[???] */
        pvVar14 = (vec_t *)((~local_54 + *(int *)(unaff_EBX + 0x827783)) * 0x60 +
                           *(int *)(unaff_EBX + 0x827777));
        for (iVar10 = 0xc; iVar10 != 0; iVar10 = iVar10 + -1) {
          *pvVar13 = *pvVar14;
          pvVar14 = pvVar14 + 1;
          pvVar13 = pvVar13 + 1;
        }
        local_54 = local_54 + 1;
        local_50 = local_50 + -1;
      }
      local_50 = local_50 + 1;
      iVar10 = *(int *)(unaff_EBX + 0x827783) - local_54;
    } while (local_50 < iVar10);
    if (local_54 != 0) {
      CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants,_16>_>
      ::SetCountNonDestructively
                ((CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants,_16>_>
                  *)(unaff_EBX + 0x827777),iVar10);
    }
  }
  if (*(int *)(*(int *)(unaff_EBX + 0x826d1b) + 0x30) == 0) {
    Paintblob_DrawSpheres(pMaterial,flCubeWidth);
    return;
  }
  local_30.x = (vec_t)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x608de7) + 400))
                                ((int *)**(undefined4 **)(unaff_EBX + 0x608de7));
  if ((int *)local_30.x != (int *)0x0) {
    (**(code **)(*(int *)local_30.x + 8))(local_30.x);
  }
  puVar6 = (undefined4 *)(**(code **)(*(int *)local_30.x + 0x22c))(local_30.x);
  if ((*(int *)(*(int *)(unaff_EBX + 0x82761b) + 0x30) == 0) || (puVar6 == (undefined4 *)0x0)) {
    iVar10 = *(int *)(unaff_EBX + 0x827783);
    blobParticles = *(ImpParticleWithFourInterpolants **)(unaff_EBX + 0x827777);
    vVar7 = CurrentViewID();
    Paintblob_DrawIsoSurface
              (pClientRenderable,vecRenderOrigin,pMaterial,flCubeWidth,vVar7,blobParticles,iVar10);
    goto LAB_005283cd;
  }
  iVar10 = *(int *)(unaff_EBX + 0x827783);
  pvVar3 = *(void **)(unaff_EBX + 0x827777);
  if (pvVar3 == (void *)0x0) {
    local_24 = (void *)0x0;
    local_20[0] = (void *)0x0;
  }
  else {
    pvVar15 = (void *)(iVar10 * 0x60);
    local_20[0] = pvVar15;
    if ((int)pvVar15 < 5) {
      _memcpy(&local_24,pvVar3,(size_t)pvVar15);
    }
    else {
      local_24 = operator_new___((size_t)pvVar15);
      _memcpy(local_24,pvVar3,(size_t)pvVar15);
    }
  }
  local_70 = &local_24;
  vVar7 = CurrentViewID();
  pcVar4 = *(code **)*puVar6;
  piVar8 = operator_new(0x3c);
  pvVar15 = local_20[0];
  pvVar3 = local_24;
  piVar8[3] = 1;
  *piVar8 = unaff_EBX + 0x66ece7;
  piVar8[2] = unaff_EBX + 0x66ed03;
  piVar8[4] = unaff_EBX + -0xc31;
  piVar8[5] = (int)pClientRenderable;
  piVar8[6] = (int)vecRenderOrigin->x;
  piVar8[7] = (int)vecRenderOrigin->y;
  piVar8[8] = (int)vecRenderOrigin->z;
  piVar8[9] = (int)pMaterial;
  piVar8[10] = (int)flCubeWidth;
  piVar8[0xb] = vVar7;
  if (local_20[0] == (void *)0x0) {
LAB_00528431:
    piVar8[0xc] = 0;
    piVar8[0xd] = 0;
  }
  else if ((int)local_20[0] < 5) {
    piVar8[0xd] = (int)local_20[0];
    piVar12 = piVar8 + 0xc;
    local_64 = local_70;
    local_68 = local_20[0];
    if (((void *)0x7 < local_20[0]) && (((uint)piVar12 & 4) != 0)) {
      piVar8[0xc] = (int)local_24;
      piVar12 = piVar8 + 0xd;
      local_64 = local_20;
      local_68 = (void *)((int)local_20[0] - 4);
    }
    for (uVar11 = (uint)local_68 >> 2; uVar11 != 0; uVar11 = uVar11 - 1) {
      *piVar12 = (int)*local_64;
      local_64 = local_64 + 1;
      piVar12 = piVar12 + 1;
    }
    if (((uint)local_68 & 2) != 0) {
      *(undefined2 *)piVar12 = *(undefined2 *)local_64;
      piVar12 = (int *)((int)piVar12 + 2);
      local_64 = (void **)((int)local_64 + 2);
    }
    if (((uint)local_68 & 1) != 0) {
      *(undefined1 *)piVar12 = *(undefined1 *)local_64;
    }
  }
  else {
    if (local_24 == (void *)0x0) goto LAB_00528431;
    piVar8[0xd] = (int)local_20[0];
    pvVar9 = operator_new___((size_t)local_20[0]);
    piVar8[0xc] = (int)pvVar9;
    _memcpy(pvVar9,pvVar3,(size_t)pvVar15);
  }
  piVar8[0xe] = iVar10;
  (*pcVar4)(puVar6,piVar8);
  if ((4 < (int)local_20[0]) && (local_24 != (void *)0x0)) {
    operator_delete___(local_24);
  }
  local_20[0] = (void *)0x0;
LAB_005283cd:
  if ((int *)local_30.x != (int *)0x0) {
    (**(code **)(*(int *)local_30.x + 0xc))(local_30.x);
  }
  if ((int *)local_30.x == (int *)0x0) {
    return;
  }
  (**(code **)(*(int *)local_30.x + 4))(local_30.x);
  return;
}


/* NPaintRenderer::s_Paintblob_DrawIsoSurface at 005274b0 */

void NPaintRenderer::s_Paintblob_DrawIsoSurface
               (IClientRenderable *pClientRenderable,Vector *vecRenderOrigin,IMaterial *pMaterial,
               float flCubeWidth,view_id_t viewID,BlobParticlesEnvelope_t *blobParticles,
               int blobCount)

{
  int iVar1;
  ImpParticleWithFourInterpolants *blobParticles_00;
  
  iVar1 = (blobParticles->super_CUtlDataEnvelope).m_nBytes;
  blobParticles_00 = (ImpParticleWithFourInterpolants *)0x0;
  if ((iVar1 != 0) &&
     (blobParticles_00 = (ImpParticleWithFourInterpolants *)blobParticles, 4 < iVar1)) {
    blobParticles_00 =
         (ImpParticleWithFourInterpolants *)(blobParticles->super_CUtlDataEnvelope).field_0.m_pData;
  }
  Paintblob_DrawIsoSurface
            (pClientRenderable,vecRenderOrigin,pMaterial,flCubeWidth,viewID,blobParticles_00,
             blobCount);
  return;
}


/* _GLOBAL__I_r_paintblob_draw_isosurface at 00075b60 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_r_paintblob_draw_isosurface(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

