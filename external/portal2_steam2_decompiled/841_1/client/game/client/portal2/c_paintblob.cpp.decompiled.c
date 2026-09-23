/* DWARF-guided pseudocode for game/client/portal2/c_paintblob.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 00074ee0 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  undefined4 uVar1;
  ConVar *unaff_EBX;
  char *unaff_EBP;
  char *unaff_ESI;
  longlong lVar2;
  int unaff_retaddr;
  ConVar *this_1;
  
  this_1 = unaff_EBX;
  lVar2 = ___i686_get_pc_thunk_bx();
  if (lVar2 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)&unaff_EBX[0x23f46].m_Value.m_nValue + 3) = 0;
  unaff_EBX[0x23f46].m_bHasMin = false;
  unaff_EBX[0x23f46].field_0x35 = 0;
  unaff_EBX[0x23f46].field_0x36 = 0;
  *(undefined4 *)&unaff_EBX[0x23f46].field_0x37 = 0;
  *(undefined4 *)((int)&unaff_EBX[0x23f46].m_fMinVal + 3) = 0;
  *(undefined4 *)&unaff_EBX[0x23f46].field_0x3f = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x23f46].m_fMaxVal + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x23f46].m_fnChangeCallbacks.m_Memory.m_pMemory + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x23f46].m_fnChangeCallbacks.m_Memory.m_nAllocationCount + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x23f46].m_fnChangeCallbacks.m_Memory.m_nGrowSize + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x23f46].m_fnChangeCallbacks.m_Size + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x23f46].m_fnChangeCallbacks.m_pElements + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x23f47].super_ConCommandBase._vptr_ConCommandBase + 3) =
       0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x23f47].super_ConCommandBase.m_pNext + 3) = 0x7f7fffff;
  *(undefined4 *)&unaff_EBX[0x23f47].super_ConCommandBase.field_0xb = 0x7f7fffff;
  *(undefined1 **)((int)&unaff_EBX[0x23f47].super_ConCommandBase.m_pszName + 3) =
       (undefined1 *)((int)&unaff_EBX[0x1fd16].super_ConCommandBase.m_pNext + 3);
  ConVar::ConVar((ConVar *)((int)&unaff_EBX[0x25abd].m_fnChangeCallbacks.m_pElements + 3),
                 (char *)((int)&unaff_EBX[0x1af5f].super_ConCommandBase.m_nFlags + 3),
                 (char *)((int)&unaff_EBX[0x1a7a4].m_fnChangeCallbacks.m_Size + 3),0,this_1,
                 unaff_ESI,unaff_EBP,unaff_retaddr);
  uVar1 = *(undefined4 *)((int)&unaff_EBX[0x1fce4].super_IConVar._vptr_IConVar + 3);
  ___cxa_atexit((undefined1 *)((int)&unaff_EBX[0x1a41b].super_ConCommandBase.m_nFlags + 3),0,uVar1);
  ConVar::ConVar((ConVar *)((int)&unaff_EBX[0x25abf].super_ConCommandBase.m_pNext + 3),
                 (char *)&unaff_EBX[0x1af5f].m_fMinVal,
                 (char *)((int)&unaff_EBX[0x1a832].super_ConCommandBase.m_pszHelpString + 2),0,
                 this_1,unaff_ESI,unaff_EBP,unaff_retaddr);
  ___cxa_atexit((undefined1 *)
                ((int)&unaff_EBX[0x1a41a].m_fnChangeCallbacks.m_Memory.m_nGrowSize + 3),0,uVar1);
  ConVar::ConVar((ConVar *)((int)&unaff_EBX[0x25ac0].super_ConCommandBase.m_pszName + 3),
                 (char *)((int)&unaff_EBX[0x1af5f].m_fnChangeCallbacks.m_pElements + 3),
                 (char *)((int)&unaff_EBX[0x1ae2e].m_fMinVal + 3),0,this_1,unaff_ESI,unaff_EBP,
                 unaff_retaddr);
  ___cxa_atexit((undefined1 *)((int)&unaff_EBX[0x1a41a].m_Value.m_fValue + 3),0,uVar1);
  ConVar::ConVar((ConVar *)((int)&unaff_EBX[0x25ac1].super_ConCommandBase.m_nFlags + 3),
                 (char *)((int)&unaff_EBX[0x1af60].m_pParent + 3),
                 (char *)((int)&unaff_EBX[0x1a9b9].m_fnChangeCallbacks.m_Memory.m_nGrowSize + 3),0,
                 this_1,unaff_ESI,unaff_EBP,unaff_retaddr);
  ___cxa_atexit((undefined1 *)((int)&unaff_EBX[0x1a41a].super_ConCommandBase.m_pszName + 3),0,uVar1)
  ;
  ConVar::ConVar((ConVar *)((int)&unaff_EBX[0x25ac2].m_pParent + 3),
                 (char *)&unaff_EBX[0x1af60].m_fnChangeCallbacks,&unaff_EBX[0x1af60].field_0x3e,0,
                 this_1,unaff_ESI,unaff_EBP,unaff_retaddr);
  ___cxa_atexit((undefined1 *)((int)&unaff_EBX[0x1a419].m_fnChangeCallbacks.m_Memory.m_pMemory + 3),
                0,uVar1);
  ConVar::ConVar((ConVar *)((int)&unaff_EBX[0x25ac3].m_Value.m_pszString + 3),
                 (char *)&unaff_EBX[0x1af61].super_ConCommandBase.m_pszHelpString,
                 &unaff_EBX[0x1af61].super_ConCommandBase.field_0xa,0,this_1,unaff_ESI,unaff_EBP,
                 unaff_retaddr);
  ___cxa_atexit((undefined1 *)((int)&unaff_EBX[0x1a419].m_Value.m_pszString + 3),0,uVar1);
  return;
}


/* __tcf_0 at 0097b850 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ec7e8),in_stack_00000008);
  return;
}


/* __tcf_1 at 0097b830 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ec868),in_stack_00000008);
  return;
}


/* __tcf_2 at 0097b810 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ec8e8),in_stack_00000008);
  return;
}


/* __tcf_3 at 0097b7f0 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ec968),in_stack_00000008);
  return;
}


/* __tcf_4 at 0097b7d0 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ec9e8),in_stack_00000008);
  return;
}


/* __tcf_5 at 0097b7b0 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3eca68),in_stack_00000008);
  return;
}


/* C_PaintBlob::C_PaintBlob at 0051ee00 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void C_PaintBlob(C_PaintBlob * this) */

void __thiscall C_PaintBlob::C_PaintBlob(C_PaintBlob *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBasePaintBlob::CBasePaintBlob(&this->super_CBasePaintBlob);
  (this->super_CBasePaintBlob)._vptr_CBasePaintBlob =
       (_func_int_varargs **)(*(int *)(&DAT_0064579b + unaff_EBX) + 8);
  return;
}


/* C_PaintBlob::C_PaintBlob at 0051ee40 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void C_PaintBlob(C_PaintBlob * this) */

void __thiscall C_PaintBlob::C_PaintBlob(C_PaintBlob *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBasePaintBlob::CBasePaintBlob(&this->super_CBasePaintBlob);
  (this->super_CBasePaintBlob)._vptr_CBasePaintBlob =
       (_func_int_varargs **)(*(int *)(&DAT_0064575b + unaff_EBX) + 8);
  return;
}


/* C_PaintBlob::~C_PaintBlob at 0051edc0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void ~C_PaintBlob(C_PaintBlob * this, int __in_chrg) */

void __thiscall C_PaintBlob::~C_PaintBlob(C_PaintBlob *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CBasePaintBlob)._vptr_CBasePaintBlob =
       (_func_int_varargs **)(*(int *)(extraout_ECX + 0x6457e4) + 8);
  CBasePaintBlob::~CBasePaintBlob(&this->super_CBasePaintBlob,__in_chrg);
  return;
}


/* C_PaintBlob::~C_PaintBlob at 0051ede0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void ~C_PaintBlob(C_PaintBlob * this, int __in_chrg) */

void __thiscall C_PaintBlob::~C_PaintBlob(C_PaintBlob *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CBasePaintBlob)._vptr_CBasePaintBlob =
       (_func_int_varargs **)(*(int *)(extraout_ECX + 0x6457c4) + 8);
  CBasePaintBlob::~CBasePaintBlob(&this->super_CBasePaintBlob,__in_chrg);
  return;
}


/* CreateBlobsForPaintBomb at 0051ee80 */

/* WARNING: Restarted to delay deadcode elimination for space: stack */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */

void CreateBlobsForPaintBomb(Vector *vecBombPos,int paintType,PaintBlobVector_t *blobs)

{
  uint *puVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  CBasePaintBlob **ppCVar5;
  int iVar6;
  CBasePaintBlob *this;
  int unaff_EBX;
  longdouble lVar7;
  float fVar8;
  int numBlobs;
  int i;
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
  float local_34;
  float local_30;
  float local_2c;
  Vector local_28 [2];
  
  ___i686_get_pc_thunk_bx();
  iVar2 = *(int *)(*(int *)(&DAT_008491cb + unaff_EBX) + 0x30);
  if (0 < iVar2) {
    i = 0;
    do {
                    /* Unresolved local var: Vector random@[???] */
      iVar6 = rand();
      fVar8 = (float)iVar6 / *(float *)(unaff_EBX + 0x4c45a7);
      local_34 = fVar8 + fVar8 + *(float *)(unaff_EBX + 0x4c33bb);
      iVar6 = rand();
      fVar8 = (float)iVar6 / *(float *)(unaff_EBX + 0x4c45a7);
      local_30 = fVar8 + fVar8 + *(float *)(unaff_EBX + 0x4c33bb);
      iVar6 = rand();
      fVar8 = (float)iVar6 / *(float *)(unaff_EBX + 0x4c45a7);
      local_2c = fVar8 + fVar8 + *(float *)(unaff_EBX + 0x4c33bb);
      local_28[0].x = local_34;
      local_28[0].y = local_30;
      local_28[0].z = local_2c;
      VectorNormalize(local_28);
      local_44 = *(float *)(*(int *)(unaff_EBX + 0x84922b) + 0x2c);
                    /* Unresolved local var: Vector res@[???] */
      local_4c = local_44 * local_28[0].x;
      local_48 = local_44 * local_28[0].y;
      local_44 = local_44 * local_28[0].z;
                    /* Unresolved local var: Vector res@[???] */
      local_58 = vecBombPos->x + local_4c;
      local_54 = vecBombPos->y + local_48;
      local_50 = vecBombPos->z + local_44;
                    /* Unresolved local var: Vector res@[???] */
      local_70 = local_58 - vecBombPos->x;
      local_6c = local_54 - vecBombPos->y;
      local_68 = local_50 - vecBombPos->z;
      local_64.x = local_70;
      local_64.y = local_6c;
      local_64.z = local_68;
      local_40.x = local_58;
      local_40.y = local_54;
      local_40.z = local_50;
      VectorNormalize(&local_64);
      lVar7 = (longdouble)
              _RandomFloat(*(undefined4 *)(*(int *)(unaff_EBX + 0x84934b) + 0x2c),
                           *(undefined4 *)(*(int *)(unaff_EBX + 0x8493ab) + 0x2c));
      local_7c.z = (vec_t)lVar7;
      puVar1 = (uint *)(unaff_EBX + 0x4c805f);
                    /* Unresolved local var: Vector res@[???] */
      local_7c.x = local_7c.z * (float)((uint)local_64.x ^ *puVar1);
      local_7c.y = (float)((uint)local_64.y ^ *puVar1) * local_7c.z;
      local_7c.z = (float)((uint)local_64.z ^ *puVar1) * local_7c.z;
      this = PaintBlobCreate(&local_40,&local_7c,paintType,0.0,0.0,false,0);
      if (this == (CBasePaintBlob *)0x0) {
        return;
      }
      lVar7 = (longdouble)
              _RandomFloat(*(undefined4 *)(*(int *)(unaff_EBX + 0x84928b) + 0x2c),
                           *(undefined4 *)(*(int *)(unaff_EBX + 0x8492eb) + 0x2c));
      CBasePaintBlob::SetRadiusScale(this,(float)lVar7);
      uVar3 = blobs->m_Size;
      iVar6 = uVar3 + 1;
      iVar4 = (blobs->m_Memory).m_nAllocationCount;
      if (iVar4 < iVar6) {
        CUtlMemory<CBasePaintBlob*,int>::Grow(&blobs->m_Memory,iVar6 - iVar4);
      }
      blobs->m_Size = blobs->m_Size + 1;
      ppCVar5 = (blobs->m_Memory).m_pMemory;
      blobs->m_pElements = ppCVar5;
      iVar4 = ~uVar3 + blobs->m_Size;
      if (0 < iVar4) {
        _V_memmove(ppCVar5 + iVar6,ppCVar5 + uVar3,iVar4 * 4);
      }
      ppCVar5 = (blobs->m_Memory).m_pMemory + uVar3;
      if (ppCVar5 != (CBasePaintBlob **)0x0) {
        *ppCVar5 = this;
      }
      i = i + 1;
    } while (i != iVar2);
  }
  return;
}


/* C_PaintBlob::PaintBlobPaint at 0051ed70 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void PaintBlobPaint(C_PaintBlob * this, trace_t * tr) */

void __thiscall C_PaintBlob::PaintBlobPaint(C_PaintBlob *this,trace_t *tr)

{
  Vector local_24;
  Vector local_18;
  
                    /* Unresolved local var: Vector vecTouchPos@[???]
                       Unresolved local var: Vector vecNormal@[???] */
  local_18.x = (tr->super_CBaseTrace).endpos.x;
  local_18.y = (tr->super_CBaseTrace).endpos.y;
  local_18.z = (tr->super_CBaseTrace).endpos.z;
  local_24.x = (tr->super_CBaseTrace).plane.normal.x;
  local_24.y = (tr->super_CBaseTrace).plane.normal.y;
  local_24.z = (tr->super_CBaseTrace).plane.normal.z;
  CBasePaintBlob::PlayEffect(&this->super_CBasePaintBlob,&local_18,&local_24);
  return;
}


/* _GLOBAL__I_paintbomb_draw_num_paint_blobs at 00075110 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_paintbomb_draw_num_paint_blobs(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

