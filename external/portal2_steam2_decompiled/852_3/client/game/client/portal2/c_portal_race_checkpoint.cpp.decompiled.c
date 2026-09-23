/* DWARF-guided pseudocode for game/client/portal2/c_portal_race_checkpoint.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 00077730 */

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
  *(undefined1 *)((int)&unaff_EBX[0x23a76].m_fnChangeCallbacks.m_Size + 3) = 0;
  *(undefined1 *)&unaff_EBX[0x23a76].m_fnChangeCallbacks.m_pElements = 0;
  *(undefined1 *)((int)&unaff_EBX[0x23a76].m_fnChangeCallbacks.m_pElements + 1) = 0;
  *(undefined1 *)((int)&unaff_EBX[0x23a76].m_fnChangeCallbacks.m_pElements + 2) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x23a76].m_fnChangeCallbacks.m_pElements + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x23a77].super_ConCommandBase._vptr_ConCommandBase + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x23a77].super_ConCommandBase.m_pNext + 3) = 0x7f7fffff;
  *(undefined4 *)&unaff_EBX[0x23a77].super_ConCommandBase.field_0xb = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x23a77].super_ConCommandBase.m_pszName + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x23a77].super_ConCommandBase.m_pszHelpString + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x23a77].super_ConCommandBase.m_nFlags + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x23a77].super_IConVar._vptr_IConVar + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x23a77].m_pParent + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x23a77].m_pszDefaultValue + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x23a77].m_Value.m_pszString + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x23a77].m_Value.m_StringLength + 3) = 0x7f7fffff;
  *(undefined1 **)((int)&unaff_EBX[0x23a77].m_Value.m_fValue + 3) =
       (undefined1 *)((int)&unaff_EBX[0x1f359].m_Value.m_fValue + 3);
  ConVar::ConVar((ConVar *)&unaff_EBX[0x255dc].field_0x3f,
                 (char *)((int)&unaff_EBX[0x1a760].super_ConCommandBase._vptr_ConCommandBase + 3),
                 (char *)((int)&unaff_EBX[0x1a75f].m_fnChangeCallbacks.m_Memory.m_nAllocationCount +
                         2),0,this_1,unaff_ESI,unaff_EBP,unaff_retaddr);
  uVar1 = *(undefined4 *)((int)&unaff_EBX[0x1f328].m_pParent + 3);
  ___cxa_atexit((undefined1 *)((int)&unaff_EBX[0x19c06].super_ConCommandBase.m_pszName + 3),0,uVar1)
  ;
  ConVar::ConVar((ConVar *)((int)&unaff_EBX[0x255dd].m_fnChangeCallbacks.m_Memory.m_pMemory + 3),
                 &unaff_EBX[0x1a760].field_0x37,
                 (char *)((int)&unaff_EBX[0x1a760].m_Value.m_pszString + 1),0,this_1,unaff_ESI,
                 unaff_EBP,unaff_retaddr);
  ___cxa_atexit((undefined1 *)((int)&unaff_EBX[0x19c05].m_fnChangeCallbacks.m_Memory.m_pMemory + 3),
                0,uVar1);
  return;
}


/* __tcf_0 at 00951960 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fe1b8),in_stack_00000008);
  return;
}


/* __tcf_1 at 00951940 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fe238),in_stack_00000008);
  return;
}


/* C_PortalRaceCheckpoint::C_PortalRaceCheckpoint at 0052cdd0 */

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
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x66b047);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x66b49f);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x66b557);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x66b593);
  (this->super_C_BaseAnimating).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x66b5af);
  return;
}


/* C_PortalRaceCheckpoint::C_PortalRaceCheckpoint at 0052ce40 */

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
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x66afd7);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x66b42f);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x66b4e7);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x66b523);
  (this->super_C_BaseAnimating).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x66b53f);
  return;
}


/* C_PortalRaceCheckpoint::~C_PortalRaceCheckpoint at 0052cc90 */

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
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(&UNK_0066b187 + unaff_EBX);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x66b5df);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(&UNK_0066b697 + unaff_EBX);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(&UNK_0066b6d3 + unaff_EBX);
  (this->super_C_BaseAnimating).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(&UNK_0066b6ef + unaff_EBX);
  C_BaseAnimating::~C_BaseAnimating(&this->super_C_BaseAnimating,in_stack_ffffffe8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_PortalRaceCheckpoint::~C_PortalRaceCheckpoint at 0052cd10 */

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
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x66b110);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(extraout_ECX + 0x66b568);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(extraout_ECX + 0x66b620);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(extraout_ECX + 0x66b65c);
  (this->super_C_BaseAnimating).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(extraout_ECX + 0x66b678);
  C_BaseAnimating::~C_BaseAnimating(&this->super_C_BaseAnimating,__in_chrg);
  return;
}


/* C_PortalRaceCheckpoint::~C_PortalRaceCheckpoint at 0052cd70 */

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
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x66b0b0);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(extraout_ECX + 0x66b508);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(extraout_ECX + 0x66b5c0);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(extraout_ECX + 0x66b5fc);
  (this->super_C_BaseAnimating).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(extraout_ECX + 0x66b618);
  C_BaseAnimating::~C_BaseAnimating(&this->super_C_BaseAnimating,__in_chrg);
  return;
}


/* C_PortalRaceCheckpoint::ShouldDraw at 0052cbb0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: bool ShouldDraw(C_PortalRaceCheckpoint * this) */

bool __thiscall C_PortalRaceCheckpoint::ShouldDraw(C_PortalRaceCheckpoint *this)

{
  return true;
}


/* C_PortalRaceCheckpoint::DrawModel at 0052cec0 */

/* WARNING: Removing unreachable block (ram,0x0052d4c2) */
/* WARNING: Removing unreachable block (ram,0x0052d631) */
/* WARNING: Removing unreachable block (ram,0x0052d7a0) */
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
  undefined *puVar9;
  int *piVar10;
  int iVar11;
  IMaterialVar *pIVar12;
  IMaterial *pIVar13;
  int unaff_EBX;
  float fVar14;
  float fVar15;
  float fVar16;
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
    pIVar13 = (IMaterial *)
              (**(code **)(*(int *)**(undefined4 **)(&DAT_00603ff7 + unaff_EBX) + 300))
                        ((int *)**(undefined4 **)(&DAT_00603ff7 + unaff_EBX),
                         &UNK_00463148 + unaff_EBX,0,0,0);
    this->m_pCheckpointMaterial = pIVar13;
    if (pIVar13 == (IMaterial *)0x0) {
      return 0;
    }
  }
  local_24 = (float)((uint)local_24 & 0xffffff00);
  if (this->m_pMyColor == (IMaterialVar *)0x0) {
    pIVar12 = (IMaterialVar *)
              (*this->m_pCheckpointMaterial->_vptr_IMaterial[0xb])
                        (this->m_pCheckpointMaterial,unaff_EBX + 0x45ec33,&local_24,1);
    this->m_pMyColor = pIVar12;
    if (local_24._0_1_ == '\0') {
      return 0;
    }
  }
  pfVar8 = (float *)(*(this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.
                      super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0xd])(this);
  fVar16 = pfVar8[2];
  fVar15 = pfVar8[1];
  fVar14 = *pfVar8;
  C_BaseEntity::GetVectors((C_BaseEntity *)this,&local_30,&local_3c,&local_48);
                    /* Unresolved local var: Vector res@[???] */
  fVar1 = *(float *)(unaff_EBX + 0x489dfb);
  fVar2 = *(float *)(unaff_EBX + 0x489adf);
  local_48.x = local_48.x * fVar2;
  local_48.y = local_48.y * fVar2;
  local_48.z = fVar2 * local_48.z;
  fVar2 = *(float *)(&DAT_0048cf6f + unaff_EBX);
  local_3c.x = local_3c.x * fVar2;
  local_3c.y = local_3c.y * fVar2;
  local_3c.z = fVar2 * local_3c.z;
  if (this->m_bCheckpointActive == false) {
                    /* Unresolved local var: uchar * pColorElement@[???] */
    puVar9 = &UNK_00822c4f + unaff_EBX;
  }
  else {
                    /* Unresolved local var: uchar * pColorElement@[DW_OP_reg1(ECX)] */
    puVar9 = (undefined *)(unaff_EBX + 0x822caf);
  }
  iVar11 = *(int *)(puVar9 + 0x1c);
  bVar4 = *(byte *)(iVar11 + 0x32);
  bVar5 = *(byte *)(iVar11 + 0x31);
  bVar6 = *(byte *)(iVar11 + 0x30);
  pfVar8 = (float *)(&DAT_00489d2f + unaff_EBX);
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
  local_20 = (int *)(**(code **)(*(int *)**(undefined4 **)(&DAT_00603ff7 + unaff_EBX) + 400))
                              ((int *)**(undefined4 **)(&DAT_00603ff7 + unaff_EBX));
  if (local_20 != (int *)0x0) {
    (**(code **)(*local_20 + 8))(local_20);
  }
  piVar10 = (int *)(**(code **)(*local_20 + 0xe4))(local_20,1,0,0,this->m_pCheckpointMaterial);
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
  local_1b0 = piVar10;
  (**(code **)(*piVar10 + 0x28))(piVar10,2);
  (**(code **)(*local_1b0 + 0x44))(local_1b0,4,6,local_264);
  local_194 = (int *)0x0;
  if (piVar10 != (int *)0x0) {
    local_194 = piVar10 + 1;
  }
  local_18c = 0;
  local_190 = 6;
  local_174 = 0;
  local_188 = local_1c8;
  local_1a4 = local_1c0;
  local_198 = local_1b4;
  local_184 = 0;
  local_cc = piVar10;
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
  fVar14 = fVar14 - local_30.x * fVar1;
  fVar15 = fVar15 - local_30.y * fVar1;
  fVar16 = fVar16 - fVar1 * local_30.z;
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
  pfVar8 = (float *)(&DAT_00489d2f + unaff_EBX);
  fVar1 = *(float *)(unaff_EBX + 0x48b823);
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
  *local_11c = (fVar14 + local_48.x) - local_3c.x;
  local_11c[1] = (fVar15 + local_48.y) - local_3c.y;
  local_11c[2] = (fVar16 + local_48.z) - local_3c.z;
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
  *local_11c = fVar14 + local_48.x + local_3c.x;
  local_11c[1] = fVar15 + local_48.y + local_3c.y;
  local_11c[2] = fVar16 + local_48.z + local_3c.z;
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
  *local_11c = (fVar14 - local_48.x) + local_3c.x;
  local_11c[1] = (fVar15 - local_48.y) + local_3c.y;
  local_11c[2] = (fVar16 - local_48.z) + local_3c.z;
  local_11c = (float *)((int)local_11c + local_170[0]);
  puVar3 = (uint *)((int)puVar3 + local_160);
  local_104 = (undefined4 *)((int)local_104 + local_158);
  local_54 = (fVar14 - local_48.x) - local_3c.x;
  local_50 = (fVar15 - local_48.y) - local_3c.y;
  local_4c = (fVar16 - local_48.z) - local_3c.z;
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
                    /* WARNING: Could not recover jumptable at 0x0052d9c3. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      iVar11 = (*(code *)(*(int *)(unaff_EBX + 0xaf7 + (local_1ac - 4) * 4) + unaff_EBX))();
      return iVar11;
    }
    if (local_198 != 0) {
      iVar11 = local_190;
      if (3 < local_190) {
        iVar11 = local_c0;
      }
      if (iVar11 != 0) {
        if (local_1ac < 0xb) {
                    /* WARNING: Could not recover jumptable at 0x0052dcae. Too many branches */
                    /* WARNING: Treating indirect jump as call */
          iVar11 = (*(code *)(*(int *)(unaff_EBX + 0xddf + local_1ac * 4) + unaff_EBX))();
          return iVar11;
        }
        GenerateSequentialIndexBuffer(local_1a4,iVar11,local_188);
        local_184 = iVar11 * local_198 + local_184;
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
  (**(code **)(*piVar10 + 0x2c))(piVar10,0xffffffff,0);
  iVar11 = C_BaseAnimating::DrawModel(&this->super_C_BaseAnimating,flags,instance);
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
  return iVar11;
}


/* C_PortalRaceCheckpoint::InitMaterials at 0052cbc0 */

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
             (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x6042f7) + 300))
                       ((int *)**(undefined4 **)(unaff_EBX + 0x6042f7),&UNK_00463448 + unaff_EBX,0,0
                        ,0);
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
                     (this->m_pCheckpointMaterial,unaff_EBX + 0x45ef33,&local_d,1);
  this->m_pMyColor = pIVar1;
  return local_d != '\0';
}


/* _GLOBAL__I_cl_race_checkpoint_inactive_color at 00077860 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_cl_race_checkpoint_inactive_color(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

