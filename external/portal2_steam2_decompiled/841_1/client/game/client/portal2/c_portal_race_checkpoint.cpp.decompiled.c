/* DWARF-guided pseudocode for game/client/portal2/c_portal_race_checkpoint.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 00077d50 */

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
  *(undefined1 *)((int)&unaff_EBX[0x23efe].m_Value.m_nValue + 3) = 0;
  unaff_EBX[0x23efe].m_bHasMin = false;
  unaff_EBX[0x23efe].field_0x35 = 0;
  unaff_EBX[0x23efe].field_0x36 = 0;
  *(undefined4 *)&unaff_EBX[0x23efe].field_0x37 = 0;
  *(undefined4 *)((int)&unaff_EBX[0x23efe].m_fMinVal + 3) = 0;
  *(undefined4 *)&unaff_EBX[0x23efe].field_0x3f = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x23efe].m_fMaxVal + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x23efe].m_fnChangeCallbacks.m_Memory.m_pMemory + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x23efe].m_fnChangeCallbacks.m_Memory.m_nAllocationCount + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x23efe].m_fnChangeCallbacks.m_Memory.m_nGrowSize + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x23efe].m_fnChangeCallbacks.m_Size + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x23efe].m_fnChangeCallbacks.m_pElements + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x23eff].super_ConCommandBase._vptr_ConCommandBase + 3) =
       0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x23eff].super_ConCommandBase.m_pNext + 3) = 0x7f7fffff;
  *(undefined4 *)&unaff_EBX[0x23eff].super_ConCommandBase.field_0xb = 0x7f7fffff;
  *(undefined1 **)((int)&unaff_EBX[0x23eff].super_ConCommandBase.m_pszName + 3) =
       (undefined1 *)((int)&unaff_EBX[0x1fc8e].m_fnChangeCallbacks.m_pElements + 3);
  ConVar::ConVar((ConVar *)((int)&unaff_EBX[0x25a69].super_ConCommandBase.m_pNext + 3),
                 (char *)((int)&unaff_EBX[0x1af19].m_fnChangeCallbacks.m_pElements + 3),
                 (char *)((int)&unaff_EBX[0x1af19].m_fnChangeCallbacks.m_Memory.m_pMemory + 2),0,
                 this_1,unaff_ESI,unaff_EBP,unaff_retaddr);
  uVar1 = *(undefined4 *)((int)&unaff_EBX[0x1fc5d].super_ConCommandBase.m_pszHelpString + 3);
  ___cxa_atexit((undefined1 *)((int)&unaff_EBX[0x1a3a9].super_ConCommandBase.m_pNext + 3),0,uVar1);
  ConVar::ConVar((ConVar *)((int)&unaff_EBX[0x25a6a].super_ConCommandBase.m_pszName + 3),
                 (char *)((int)&unaff_EBX[0x1af1a].m_Value.m_nValue + 3),
                 (char *)((int)&unaff_EBX[0x1af1a].m_pszDefaultValue + 1),0,this_1,unaff_ESI,
                 unaff_EBP,unaff_retaddr);
  ___cxa_atexit(&unaff_EBX[0x1a3a8].field_0x3f,0,uVar1);
  return;
}


/* __tcf_0 at 0097bf80 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ed1f8),in_stack_00000008);
  return;
}


/* __tcf_1 at 0097bf60 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ed278),in_stack_00000008);
  return;
}


/* C_PortalRaceCheckpoint::C_PortalRaceCheckpoint at 0052dd10 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void C_PortalRaceCheckpoint(C_PortalRaceCheckpoint * this) */

void __thiscall C_PortalRaceCheckpoint::C_PortalRaceCheckpoint(C_PortalRaceCheckpoint *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseAnimating::C_BaseAnimating(&this->super_C_BaseAnimating);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x69e507);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(CViewEffects::FindLongestShake + unaff_EBX + 3);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x69ea1b);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x69ea57);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable =
       (_func_int_varargs **)(CViewEffects::CalcShake_Directional + unaff_EBX + 3);
  return;
}


/* C_PortalRaceCheckpoint::C_PortalRaceCheckpoint at 0052dd80 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void C_PortalRaceCheckpoint(C_PortalRaceCheckpoint * this) */

void __thiscall C_PortalRaceCheckpoint::C_PortalRaceCheckpoint(C_PortalRaceCheckpoint *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseAnimating::C_BaseAnimating(&this->super_C_BaseAnimating);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x69e497);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x69e8f3);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(&UNK_0069e9ab + unaff_EBX);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x69e9e7);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(unaff_EBX + 0x69ea03);
  return;
}


/* C_PortalRaceCheckpoint::~C_PortalRaceCheckpoint at 0052dbe0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_PortalRaceCheckpoint(C_PortalRaceCheckpoint * this, int
   __in_chrg) */

void __thiscall
C_PortalRaceCheckpoint::~C_PortalRaceCheckpoint(C_PortalRaceCheckpoint *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x69e637);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x69ea93);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x69eb4b);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x69eb87);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(unaff_EBX + 0x69eba3);
  C_BaseAnimating::~C_BaseAnimating(&this->super_C_BaseAnimating,in_stack_ffffffe8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_PortalRaceCheckpoint::~C_PortalRaceCheckpoint at 0052dc50 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_PortalRaceCheckpoint(C_PortalRaceCheckpoint * this, int
   __in_chrg) */

void __thiscall
C_PortalRaceCheckpoint::~C_PortalRaceCheckpoint(C_PortalRaceCheckpoint *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x69e5d0);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(extraout_ECX + 0x69ea2c);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(extraout_ECX + 0x69eae4);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(extraout_ECX + 0x69eb20);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(extraout_ECX + 0x69eb3c);
  C_BaseAnimating::~C_BaseAnimating(&this->super_C_BaseAnimating,__in_chrg);
  return;
}


/* C_PortalRaceCheckpoint::~C_PortalRaceCheckpoint at 0052dcb0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_PortalRaceCheckpoint(C_PortalRaceCheckpoint * this, int
   __in_chrg) */

void __thiscall
C_PortalRaceCheckpoint::~C_PortalRaceCheckpoint(C_PortalRaceCheckpoint *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x69e570);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(extraout_ECX + 0x69e9cc);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(extraout_ECX + 0x69ea84);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(extraout_ECX + 0x69eac0);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(extraout_ECX + 0x69eadc);
  C_BaseAnimating::~C_BaseAnimating(&this->super_C_BaseAnimating,__in_chrg);
  return;
}


/* C_PortalRaceCheckpoint::ShouldDraw at 0052db00 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: bool ShouldDraw(C_PortalRaceCheckpoint * this) */

bool __thiscall C_PortalRaceCheckpoint::ShouldDraw(C_PortalRaceCheckpoint *this)

{
  return true;
}


/* C_PortalRaceCheckpoint::DrawModel at 0052de00 */

/* WARNING: Removing unreachable block (ram,0x0052e408) */
/* WARNING: Removing unreachable block (ram,0x0052e577) */
/* WARNING: Removing unreachable block (ram,0x0052e6e6) */
/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: int DrawModel(C_PortalRaceCheckpoint * this, int flags,
   RenderableInstance_t * instance) */

int __thiscall
C_PortalRaceCheckpoint::DrawModel
          (C_PortalRaceCheckpoint *this,int flags,RenderableInstance_t *instance)

{
  float fVar1;
  float fVar2;
  uint *puVar3;
  byte bVar4;
  byte bVar5;
  byte bVar6;
  char cVar7;
  float *pfVar8;
  int *piVar9;
  int iVar10;
  IMaterialVar *pIVar11;
  IMaterial *pIVar12;
  int unaff_EBX;
  float fVar13;
  float fVar14;
  float fVar15;
  undefined1 local_264 [156];
  int local_1c8;
  int local_1c4;
  ushort *local_1c0;
  int local_1b4;
  int *local_1b0;
  uint local_1ac;
  char local_1a8;
  ushort *local_1a4;
  int local_198;
  int *local_194;
  int local_190;
  int local_18c;
  int local_188;
  int local_184;
  undefined4 local_180;
  undefined4 local_17c;
  undefined4 local_178;
  undefined1 local_174;
  int local_170 [3];
  int local_164;
  int local_160;
  int local_158;
  int local_154;
  int local_150;
  int local_14c;
  int local_148;
  int local_144;
  int local_140;
  int local_13c;
  undefined4 local_124;
  uint local_120;
  float *local_11c;
  int local_110;
  uint *local_10c;
  undefined4 *local_104;
  int local_100;
  int local_fc;
  int local_f8;
  int local_f4;
  int local_f0;
  int local_ec;
  int local_e8;
  int *local_cc;
  undefined1 local_c8;
  undefined4 local_c4;
  int local_c0;
  undefined4 local_bc;
  int local_b8;
  int local_b4;
  int local_b0;
  int local_ac;
  int local_a8;
  int local_a4;
  int local_a0;
  int local_9c;
  int local_98;
  int local_94;
  int local_90;
  undefined4 local_8c;
  int local_88;
  int local_84;
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
  Vector local_48;
  Vector local_3c;
  Vector local_30;
  float local_24;
  int *local_20;
  
                    /* Unresolved local var: Vector vecOrigin@[???]
                       Unresolved local var: Vector vecForward@[???]
                       Unresolved local var: Vector vecRight@[???]
                       Unresolved local var: Vector vecUp@[???]
                       Unresolved local var: Vector vecVertex@[???]
                       Unresolved local var: Color meshColor@[???]
                       Unresolved local var: float[3] flColors@[???]
                       Unresolved local var: float[4] pColors@[???]
                       Unresolved local var: CMatRenderContextPtr pRenderContext@[???]
                       Unresolved local var: IMesh * pMesh@[???]
                       Unresolved local var: CMeshBuilder meshBuilder@[???] */
  ___i686_get_pc_thunk_bx();
  if (this->m_pCheckpointMaterial == (IMaterial *)0x0) {
    pIVar12 = (IMaterial *)
              (**(code **)(*(int *)**(undefined4 **)(&DAT_006360fb + unaff_EBX) + 0x130))
                        ((int *)**(undefined4 **)(&DAT_006360fb + unaff_EBX),unaff_EBX + 0x48d014,0,
                         0,0);
    this->m_pCheckpointMaterial = pIVar12;
    if (pIVar12 == (IMaterial *)0x0) {
      return 0;
    }
  }
  local_24 = (float)((uint)local_24 & 0xffffff00);
  if (this->m_pMyColor == (IMaterialVar *)0x0) {
    pIVar11 = (IMaterialVar *)
              (*this->m_pCheckpointMaterial->_vptr_IMaterial[0xb])
                        (this->m_pCheckpointMaterial,&UNK_00488607 + unaff_EBX,&local_24,1);
    this->m_pMyColor = pIVar11;
    if (local_24._0_1_ == '\0') {
      return 0;
    }
  }
  pfVar8 = (float *)(*(this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.
                      super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0xd])(this);
  fVar15 = pfVar8[2];
  fVar14 = pfVar8[1];
  fVar13 = *pfVar8;
  C_BaseEntity::GetVectors((C_BaseEntity *)this,&local_30,&local_3c,&local_48);
                    /* Unresolved local var: Vector res@[???] */
  fVar1 = *(float *)(unaff_EBX + 0x4b47ff);
  fVar2 = *(float *)(&DAT_004b44df + unaff_EBX);
  local_48.x = local_48.x * fVar2;
  local_48.y = local_48.y * fVar2;
  local_48.z = fVar2 * local_48.z;
  fVar2 = *(float *)(unaff_EBX + 0x4b796f);
  local_3c.x = local_3c.x * fVar2;
  local_3c.y = local_3c.y * fVar2;
  local_3c.z = fVar2 * local_3c.z;
  if (this->m_bCheckpointActive == false) {
                    /* Unresolved local var: uchar * pColorElement@[???] */
    iVar10 = unaff_EBX + 0x83b36f;
  }
  else {
                    /* Unresolved local var: uchar * pColorElement@[DW_OP_reg1(ECX)] */
    iVar10 = unaff_EBX + 0x83b3cf;
  }
  iVar10 = *(int *)(iVar10 + 0x1c);
  bVar4 = *(byte *)(iVar10 + 0x32);
  bVar5 = *(byte *)(iVar10 + 0x31);
  bVar6 = *(byte *)(iVar10 + 0x30);
  pfVar8 = (float *)(unaff_EBX + 0x4b472f);
  local_6c = (float)bVar6 / *pfVar8;
  local_68 = (float)bVar5 / *pfVar8;
  local_64 = (float)bVar4 / *pfVar8;
  local_60 = local_30.x * fVar1;
  local_5c = local_30.y * fVar1;
  local_58 = fVar1 * local_30.z;
  (*this->m_pMyColor->_vptr_IMaterialVar[9])(this->m_pMyColor,&local_6c,3);
  (*this->m_pMyColor->_vptr_IMaterialVar[0xd])(this->m_pMyColor,&local_7c,3);
  local_70 = 1.0;
  if ((this->super_C_BaseAnimating).super_C_BaseEntity.m_clrRender.
      super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.r != bVar6) {
    (this->super_C_BaseAnimating).super_C_BaseEntity.m_clrRender.
    super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.r = bVar6;
  }
  if ((this->super_C_BaseAnimating).super_C_BaseEntity.m_clrRender.
      super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.g != bVar5) {
    (this->super_C_BaseAnimating).super_C_BaseEntity.m_clrRender.
    super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.g = bVar5;
  }
  if ((this->super_C_BaseAnimating).super_C_BaseEntity.m_clrRender.
      super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.b != bVar4) {
    (this->super_C_BaseAnimating).super_C_BaseEntity.m_clrRender.
    super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.b = bVar4;
  }
  local_20 = (int *)(**(code **)(*(int *)**(undefined4 **)(&DAT_006360fb + unaff_EBX) + 0x1a0))
                              ((int *)**(undefined4 **)(&DAT_006360fb + unaff_EBX));
  if (local_20 != (int *)0x0) {
    (**(code **)(*local_20 + 8))(local_20);
  }
  piVar9 = (int *)(**(code **)(*local_20 + 0xe4))(local_20,1,0,0,this->m_pCheckpointMaterial);
  local_194 = (int *)0x0;
  local_190 = 0;
  local_18c = 0;
  local_184 = 0;
  local_180 = 0;
  local_17c = 0xffffffff;
  local_178 = 0;
  local_cc = (int *)0x0;
  local_88 = -1;
  local_84 = 0;
  local_c0 = 0;
  local_bc = 0;
  local_c4 = 0;
  local_8c = 0;
  local_124 = 0xffffffff;
  local_1a8 = '\x01';
  local_1ac = 7;
  local_1b0 = piVar9;
  (**(code **)(*piVar9 + 0x28))(piVar9,2);
  (**(code **)(*local_1b0 + 0x44))(local_1b0,4,6,local_264,0);
  local_194 = (int *)0x0;
  if (piVar9 != (int *)0x0) {
    local_194 = piVar9 + 1;
  }
  local_18c = 0;
  local_190 = 6;
  local_174 = 0;
  local_188 = local_1c8;
  local_1a4 = local_1c0;
  local_198 = local_1b4;
  local_184 = 0;
  local_cc = piVar9;
  _memcpy(local_170,local_264,0xa4);
  local_c4 = 4;
  local_120 = ~-(uint)(local_120 == 0) & 2;
  local_c8 = 0;
  if (local_88 == -1) {
    local_8c = 0;
    local_88 = local_1c4;
    local_84 = local_1c8;
  }
  local_184 = 0;
                    /* Unresolved local var: int i@[???] */
  fVar13 = fVar13 - local_30.x * fVar1;
  fVar14 = fVar14 - local_30.y * fVar1;
  fVar15 = fVar15 - fVar1 * local_30.z;
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
  pfVar8 = (float *)(unaff_EBX + 0x4b472f);
  fVar1 = *(float *)(unaff_EBX + 0x4b6243);
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
  *local_10c = ((uint)(*pfVar8 * local_78 + fVar1) & 0xff) << 8 |
               ((uint)(*pfVar8 * local_74 + fVar1) & 0xff) << 0x10 |
               (uint)(*pfVar8 * local_7c + fVar1) & 0xff | (int)(*pfVar8 * local_70 + fVar1) << 0x18
  ;
                    /* Unresolved local var: float * pDst@[???] */
  *local_104 = 0;
  local_104[1] = 0;
                    /* Unresolved local var: float * pDst@[???] */
  *local_11c = (fVar13 + local_48.x) - local_3c.x;
  local_11c[1] = (fVar14 + local_48.y) - local_3c.y;
  local_11c[2] = (fVar15 + local_48.z) - local_3c.z;
  local_11c = (float *)((int)local_11c + local_170[0]);
  local_b4 = local_110 + local_164 * 4;
  local_104 = (undefined4 *)((int)local_104 + local_158);
  local_a8 = local_100 + local_154 * 4;
  local_a4 = local_fc + local_150 * 4;
  local_a0 = local_f8 + local_14c * 4;
  local_9c = local_f4 + local_148 * 4;
  local_98 = local_f0 + local_144 * 4;
  local_94 = local_ec + local_140 * 4;
  local_90 = local_e8 + local_13c * 4;
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
  *(uint *)((int)local_10c + local_160) =
       ((uint)(*pfVar8 * local_78 + fVar1) & 0xff) << 8 |
       ((uint)(*pfVar8 * local_74 + fVar1) & 0xff) << 0x10 |
       (uint)(*pfVar8 * local_7c + fVar1) & 0xff | (int)(*pfVar8 * local_70 + fVar1) << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
  *local_104 = 0x3f800000;
  local_104[1] = 0;
                    /* Unresolved local var: float * pDst@[???] */
  *local_11c = fVar13 + local_48.x + local_3c.x;
  local_11c[1] = fVar14 + local_48.y + local_3c.y;
  local_11c[2] = fVar15 + local_48.z + local_3c.z;
  local_11c = (float *)((int)local_11c + local_170[0]);
  puVar3 = (uint *)((int)local_10c + local_160 + local_160);
  local_104 = (undefined4 *)((int)local_104 + local_158);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
  *puVar3 = ((uint)(*pfVar8 * local_78 + fVar1) & 0xff) << 8 |
            ((uint)(*pfVar8 * local_74 + fVar1) & 0xff) << 0x10 |
            (uint)(*pfVar8 * local_7c + fVar1) & 0xff | (int)(*pfVar8 * local_70 + fVar1) << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
  *local_104 = 0;
  local_104[1] = 0x3f800000;
                    /* Unresolved local var: float * pDst@[???] */
  *local_11c = (fVar13 - local_48.x) + local_3c.x;
  local_11c[1] = (fVar14 - local_48.y) + local_3c.y;
  local_11c[2] = (fVar15 - local_48.z) + local_3c.z;
  local_11c = (float *)((int)local_11c + local_170[0]);
  puVar3 = (uint *)((int)puVar3 + local_160);
  local_104 = (undefined4 *)((int)local_104 + local_158);
  local_54 = (fVar13 - local_48.x) - local_3c.x;
  local_50 = (fVar14 - local_48.y) - local_3c.y;
  local_4c = (fVar15 - local_48.z) - local_3c.z;
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
  local_24 = *pfVar8 * local_70 + fVar1;
  *puVar3 = ((uint)(*pfVar8 * local_78 + fVar1) & 0xff) << 8 |
            ((uint)(*pfVar8 * local_74 + fVar1) & 0xff) << 0x10 |
            (uint)(*pfVar8 * local_7c + fVar1) & 0xff | (int)local_24 << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
  *local_104 = 0x3f800000;
  local_104[1] = 0x3f800000;
                    /* Unresolved local var: float * pDst@[???] */
  *local_11c = local_54;
  local_11c[1] = local_50;
  local_11c[2] = local_4c;
  local_bc = 4;
  local_c0 = 4;
  local_b8 = (int)local_11c + local_170[0];
  local_b0 = (int)puVar3 + local_160;
  local_ac = (int)local_104 + local_158;
  if (local_1a8 != '\0') {
    if (local_1ac - 4 < 7) {
                    /* WARNING: Could not recover jumptable at 0x0052e909. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      iVar10 = (*(code *)(*(int *)(unaff_EBX + 0xafb + (local_1ac - 4) * 4) + unaff_EBX))();
      return iVar10;
    }
    if (local_198 != 0) {
      iVar10 = local_190;
      if (3 < local_190) {
        iVar10 = local_c0;
      }
      if (iVar10 != 0) {
        if (local_1ac < 0xb) {
                    /* WARNING: Could not recover jumptable at 0x0052ebee. Too many branches */
                    /* WARNING: Treating indirect jump as call */
          iVar10 = (*(code *)(*(int *)(unaff_EBX + 0xddf + local_1ac * 4) + unaff_EBX))();
          return iVar10;
        }
        GenerateSequentialIndexBuffer(local_1a4,iVar10,local_188);
        local_184 = iVar10 * local_198 + local_184;
        if (local_18c < local_184) {
          local_18c = local_184;
        }
      }
    }
  }
  (**(code **)(*local_1b0 + 0x50))(local_1b0,local_c0,local_18c,local_264);
  local_194 = (int *)0x0;
  local_190 = 0;
  local_c4 = 0;
  local_cc = (int *)0x0;
  local_124 = 0xffffffff;
  local_1b0 = (int *)0x0;
  (**(code **)(*piVar9 + 0x2c))(piVar9,0xffffffff,0);
  iVar10 = C_BaseAnimating::DrawModel(&this->super_C_BaseAnimating,flags,instance);
  if ((local_cc != (int *)0x0) && (cVar7 = (**(code **)(*local_cc + 8))(local_cc), cVar7 != '\0')) {
    (**(code **)(*local_cc + 0x10))(local_cc);
  }
  if ((local_194 != (int *)0x0) && (cVar7 = (**(code **)(*local_194 + 8))(local_194), cVar7 != '\0')
     ) {
    (**(code **)(*local_194 + 0x10))(local_194);
  }
  if (local_20 != (int *)0x0) {
    (**(code **)(*local_20 + 0xc))(local_20);
  }
  if (local_20 != (int *)0x0) {
    (**(code **)(*local_20 + 4))(local_20);
  }
  return iVar10;
}


/* C_PortalRaceCheckpoint::InitMaterials at 0052db10 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: bool InitMaterials(C_PortalRaceCheckpoint * this) */

bool __thiscall C_PortalRaceCheckpoint::InitMaterials(C_PortalRaceCheckpoint *this)

{
  IMaterialVar *pIVar1;
  IMaterial *pIVar2;
  int unaff_EBX;
  char local_d;
  
                    /* Unresolved local var: bool bFound@[???] */
  ___i686_get_pc_thunk_bx();
  if (this->m_pCheckpointMaterial == (IMaterial *)0x0) {
    pIVar2 = (IMaterial *)
             (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x6363eb) + 0x130))
                       ((int *)**(undefined4 **)(unaff_EBX + 0x6363eb),unaff_EBX + 0x48d304,0,0,0);
    this->m_pCheckpointMaterial = pIVar2;
    if (pIVar2 == (IMaterial *)0x0) {
      return false;
    }
  }
  local_d = '\0';
  if (this->m_pMyColor != (IMaterialVar *)0x0) {
    return true;
  }
  pIVar1 = (IMaterialVar *)
           (*this->m_pCheckpointMaterial->_vptr_IMaterial[0xb])
                     (this->m_pCheckpointMaterial,unaff_EBX + 0x4888f7,&local_d,1);
  this->m_pMyColor = pIVar1;
  return local_d != '\0';
}


/* _GLOBAL__I_cl_race_checkpoint_inactive_color at 00077e80 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_cl_race_checkpoint_inactive_color(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

