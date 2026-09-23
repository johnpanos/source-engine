/* DWARF-guided pseudocode for game/shared/portal2/portal_pointpush_shared.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 00016a70 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  undefined4 uVar1;
  int unaff_EBX;
  char *unaff_ESI;
  longlong lVar2;
  ConVar *in_stack_ffffffe8;
  char *in_stack_ffffffec;
  char *in_stack_fffffff0;
  int flags_1;
  
  flags_1 = unaff_EBX;
  lVar2 = ___i686_get_pc_thunk_bx();
  if (lVar2 != 0xffff00000001) {
    return;
  }
  (&DAT_00c55d4b)[unaff_EBX] = 0;
  (&DAT_00c55d4c)[unaff_EBX] = 0;
  (&DAT_00c55d4d)[unaff_EBX] = 0;
  (&DAT_00c55d4e)[unaff_EBX] = 0;
  *(undefined4 *)(&DAT_00c55d4f + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c55d53 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c55d57 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c55d5b + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c55d5f + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c55d63 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c55d67 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c55d6b + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c55d6f + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c55d73 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c55d77 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c55d7b + unaff_EBX) = 0x7f7fffff;
  *(undefined **)(&DAT_00c55d7f + unaff_EBX) = &UNK_00b4e407 + unaff_EBX;
  ConVar::ConVar((ConVar *)(s_SortBuffer + unaff_EBX + 0x4b4ff),&UNK_0097d4d5 + unaff_EBX,
                 &UNK_0097d4cf + unaff_EBX,0x6000,&UNK_0097d493 + unaff_EBX,in_stack_ffffffe8,
                 in_stack_ffffffec,in_stack_fffffff0,flags_1,unaff_ESI);
  uVar1 = *(undefined4 *)(&DAT_00b4d2eb + unaff_EBX);
  ___cxa_atexit(unaff_EBX + 0x953caf,0,uVar1);
  ConVar::ConVar((ConVar *)(s_SortBuffer + unaff_EBX + 0x4b55f),&UNK_0097d50f + unaff_EBX,
                 &UNK_0096c43b + unaff_EBX,0x6000,&UNK_0097d4f1 + unaff_EBX,in_stack_ffffffe8,
                 in_stack_ffffffec,in_stack_fffffff0,flags_1,unaff_ESI);
  ___cxa_atexit(unaff_EBX + 0x953c8f,0,uVar1);
  return;
}


/* __tcf_0 at 0096a730 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(&UNK_003ef228 + extraout_ECX),in_stack_00000008);
  return;
}


/* __tcf_1 at 0096a710 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ef2a8),in_stack_00000008);
  return;
}


/* C_Portal_PointPush::PushThink at 001bc320 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_Portal_PointPush": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: void PushThink(C_Portal_PointPush * this) */

void __thiscall C_Portal_PointPush::PushThink(C_Portal_PointPush *this)

{
  float flRadius;
  QAngle *vecAngles;
  Vector *vecOrigin;
  int unaff_EBX;
  int iVar1;
  PortalRadiusExtensionVector local_30;
  
                    /* Unresolved local var: PortalRadiusExtensionVector
                       portalRadiusExtensions@[???] */
  ___i686_get_pc_thunk_bx();
  local_30.m_Memory.m_pMemory = (PortalRadiusExtension_t *)0x0;
  local_30.m_Memory.m_nAllocationCount = 0;
  local_30.m_Memory.m_nGrowSize = 0;
  local_30.m_Size = 0;
  local_30.m_pElements = (PortalRadiusExtension_t *)0x0;
  flRadius = this->m_flRadius;
  vecAngles = (QAngle *)
              (*(this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.
                super_IHandleEntity._vptr_IHandleEntity[0xe])(this);
  vecOrigin = (Vector *)
              (*(this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.
                super_IHandleEntity._vptr_IHandleEntity[0xd])(this);
  ExtendRadiusThroughPortals(vecOrigin,vecAngles,flRadius,&local_30);
                    /* Unresolved local var: int i@[???] */
  if (0 < local_30.m_Size) {
    iVar1 = 0;
    do {
      PushPaintBlobsAtPoint
                (this,&local_30.m_Memory.m_pMemory[iVar1].vecOrigin,
                 &local_30.m_Memory.m_pMemory[iVar1].vecAngles,
                 local_30.m_Memory.m_pMemory[iVar1].pPortalTo);
      iVar1 = iVar1 + 1;
    } while (iVar1 < local_30.m_Size);
  }
  (*(this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x9d])
            (this,*(float *)(**(int **)("rope_smooth_maxalphawidth" + unaff_EBX + 0x11) + 0xc) +
                  *(float *)(*(int *)((int)&PTR_GetBaseAnimating_00b9d64c + unaff_EBX + 2) + 0x2c));
  if (-1 < local_30.m_Memory.m_nGrowSize) {
    local_30.m_Size = 0;
    if (local_30.m_Memory.m_pMemory != (PortalRadiusExtension_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)
                             ("Alpha for rope antialiasing effect" + unaff_EBX + 0x22) + 8))
                ((int *)**(undefined4 **)("Alpha for rope antialiasing effect" + unaff_EBX + 0x22),
                 local_30.m_Memory.m_pMemory);
    }
  }
  return;
}


/* C_Portal_PointPush::TestWithinCone at 001bbdf0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_Portal_PointPush": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: bool TestWithinCone(C_Portal_PointPush * this, Vector * vecOrigin,
   Vector * vecEndPos, QAngle * vecAngles, float flMinAngle) */

bool __thiscall
C_Portal_PointPush::TestWithinCone
          (C_Portal_PointPush *this,Vector *vecOrigin,Vector *vecEndPos,QAngle *vecAngles,
          float flMinAngle)

{
  Vector local_30;
  float local_24;
  float local_20;
  float local_1c;
  Vector local_18;
  
                    /* Unresolved local var: Vector vecDirToTarget@[???]
                       Unresolved local var: Vector vecDirection@[???]
                       Unresolved local var: float flDot@[???] */
                    /* Unresolved local var: Vector res@[???] */
  local_24 = vecEndPos->x - vecOrigin->x;
  local_20 = vecEndPos->y - vecOrigin->y;
  local_1c = vecEndPos->z - vecOrigin->z;
  local_18.x = local_24;
  local_18.y = local_20;
  local_18.z = local_1c;
  VectorNormalize(&local_18);
  AngleVectors(vecAngles,&local_30);
  return flMinAngle <= local_18.x * local_30.x + local_18.y * local_30.y + local_18.z * local_30.z;
}


/* C_Portal_PointPush::PushPaintBlobsAtPoint at 001bbe90 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_Portal_PointPush": ignoring overlapping field "m_bEnabled" */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void PushPaintBlobsAtPoint(C_Portal_PointPush * this, Vector *
   vecOrigin, QAngle * vecAngles, C_Portal_Base2D * pPortal) */

void __thiscall
C_Portal_PointPush::PushPaintBlobsAtPoint
          (C_Portal_PointPush *this,Vector *vecOrigin,QAngle *vecAngles,C_Portal_Base2D *pPortal)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  C_PaintStream *this_00;
  bool bVar9;
  uint uVar10;
  CBasePaintBlob *this_01;
  Vector *pVVar11;
  QAngle *angles;
  float *pfVar12;
  int unaff_EBX;
  uint id;
  longdouble lVar13;
  float fVar14;
  float fVar15;
  float fVar16;
  float flRadiusSqr;
  int local_cc;
  Vector local_7c;
  Vector local_70;
  float local_64;
  float local_60;
  float local_5c;
  float local_58;
  float local_54;
  float local_50;
  Vector local_4c;
  float local_40;
  float local_3c;
  float local_38;
  float local_34;
  float local_30;
  float local_2c;
  Vector local_28 [2];
  
                    /* Unresolved local var: Vector vecPushPoint@[???] */
  ___i686_get_pc_thunk_bx();
  fVar1 = this->m_flRadius;
  fVar2 = vecOrigin->z;
  fVar3 = vecOrigin->y;
  fVar4 = vecOrigin->x;
  if (0 < *(int *)(*(int *)("m_AmbientLight.m_vPos" + unaff_EBX) + 0xc)) {
    local_cc = 0;
    do {
      this_00 = *(C_PaintStream **)(**(int **)("m_AmbientLight.m_vPos" + unaff_EBX) + local_cc * 4);
      if ((this_00 != (C_PaintStream *)0x0) &&
         (uVar10 = C_PaintStream::GetBlobsCount(this_00), 0 < (int)uVar10)) {
        id = 0;
LAB_001bbf6d:
        do {
          this_01 = C_PaintStream::GetBlob(this_00,id);
          if ((this_01 != (CBasePaintBlob *)0x0) &&
             (bVar9 = CBasePaintBlob::IsStreaking(this_01), !bVar9)) {
            pVVar11 = CBasePaintBlob::GetPosition(this_01);
            fVar5 = pVVar11->z;
            fVar6 = pVVar11->y;
            fVar7 = pVVar11->x;
                    /* Unresolved local var: Vector delta@[???] */
            fVar14 = fVar7 - fVar4;
            fVar15 = fVar6 - fVar3;
            fVar16 = fVar5 - fVar2;
            if (fVar14 * fVar14 + fVar15 * fVar15 + fVar16 * fVar16 <= fVar1 * fVar1) {
              fVar8 = this->m_flConeOfInfluence;
              if (fVar8 != 0.0) {
                    /* Unresolved local var: float flMinAngle@[???]
                       Unresolved local var: Vector vecDirToTarget@[???]
                       Unresolved local var: Vector vecDirection@[???]
                       Unresolved local var: float flDot@[???]
                       Unresolved local var: Vector res@[???] */
                local_7c.x = fVar7 - vecOrigin->x;
                local_7c.y = fVar6 - vecOrigin->y;
                local_7c.z = fVar5 - vecOrigin->z;
                local_64 = local_7c.x;
                local_60 = local_7c.y;
                local_5c = local_7c.z;
                VectorNormalize(&local_7c);
                AngleVectors(vecAngles,&local_70);
                lVar13 = (longdouble)_cosf(*(float *)(unaff_EBX + 0x826373) * fVar8);
                if (local_7c.x * local_70.x + local_7c.y * local_70.y + local_7c.z * local_70.z <
                    (float)lVar13) goto LAB_001bbf60;
              }
              if (pPortal != (C_Portal_Base2D *)0x0) {
                angles = (QAngle *)
                         (*(pPortal->super_CPortalRenderable_FlatBasic).super_C_BaseAnimating.
                           super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
                           super_IHandleEntity._vptr_IHandleEntity[0xe])(pPortal);
                pfVar12 = (float *)(*(pPortal->super_CPortalRenderable_FlatBasic).
                                     super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.
                                     super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity
                                     [0xd])(pPortal);
                    /* Unresolved local var: Vector res@[???] */
                local_70.x = fVar7 - *pfVar12;
                local_70.y = fVar6 - pfVar12[1];
                local_70.z = fVar5 - pfVar12[2];
                VectorNormalize(&local_70);
                AngleVectors(angles,&local_7c);
                if (local_70.x * local_7c.x + local_70.y * local_7c.y + local_70.z * local_7c.z <
                    0.0) goto LAB_001bbf60;
              }
              pVVar11 = CBasePaintBlob::GetVelocity(this_01);
              fVar5 = pVVar11->z;
              fVar6 = pVVar11->y;
              fVar7 = pVVar11->x;
              local_34 = fVar14;
              local_30 = fVar15;
              local_2c = fVar16;
              local_28[0].x = fVar14;
              local_28[0].y = fVar15;
              local_28[0].z = fVar16;
                    /* Unresolved local var: Vector res@[???] */
              VectorNormalize(local_28);
              local_50 = this->m_flMagnitude;
                    /* Unresolved local var: Vector res@[???] */
              local_58 = local_50 * local_28[0].x;
              local_54 = local_50 * local_28[0].y;
              local_50 = local_50 * local_28[0].z;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
              local_4c.x = local_58 + fVar7;
              local_4c.y = local_54 + fVar6;
              local_4c.z = local_50 + fVar5;
              local_40 = local_58;
              local_3c = local_54;
              local_38 = local_50;
              CBasePaintBlob::SetVelocity(this_01,&local_4c);
              id = id + 1;
              if (uVar10 == id) break;
              goto LAB_001bbf6d;
            }
          }
LAB_001bbf60:
          id = id + 1;
        } while (uVar10 != id);
      }
      local_cc = local_cc + 1;
    } while (local_cc < *(int *)(*(int *)("m_AmbientLight.m_vPos" + unaff_EBX) + 0xc));
  }
  return;
}


/* _GLOBAL__I_portal_pointpush_think_rate at 00016bc0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_portal_pointpush_think_rate(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

