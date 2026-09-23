/* DWARF-guided pseudocode for game/shared/portal2/portal_pointpush_shared.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 0001c9d0 */

/* WARNING: Type propagation algorithm not settling */

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
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x3ed].m_AddLock.m_lockInfo + unaff_EBX + 3) = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x3ed].m_AddLock.m_lockInfo + unaff_EBX + 4) = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x3ed].m_AddLock.m_lockInfo + unaff_EBX + 5) = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x3ed].m_AddLock.m_lockInfo + unaff_EBX + 6) = 0;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x3ed].m_AddLock.m_lockInfo + unaff_EBX + 7) = 0;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x3ed].m_AddLock.m_nWriters.m_value + unaff_EBX + 3) = 0;
  *(undefined4 *)
   (&CDmxElement::s_TypeSymbols.m_Lookup.
     super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
     .m_aBuckets[0x3ed].m_AddLock.field_0xf + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x3ee].m_pFirst + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x3ee].m_pFirstUncommitted + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x3ee].m_AddLock.m_lockInfo + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x3ee].m_AddLock.m_lockInfo + unaff_EBX + 7) = 0;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x3ee].m_AddLock.m_nWriters.m_value + unaff_EBX + 3) = 0;
  *(undefined4 *)
   (&CDmxElement::s_TypeSymbols.m_Lookup.
     super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
     .m_aBuckets[0x3ee].m_AddLock.field_0xf + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x3ef].m_pFirst + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x3ef].m_pFirstUncommitted + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x3ef].m_AddLock.m_lockInfo + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined **)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x3ef].m_AddLock.m_lockInfo + unaff_EBX + 7) = &UNK_00c2f927 + unaff_EBX;
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<CFuncTrackTrain>::dataDesc[0x1c].flatOffset + unaff_EBX + -0x15),
                 (char *)(unaff_EBX + 0xa45111),(char *)(unaff_EBX + 0xa4510b),0x6000,
                 &UNK_00a450cf + unaff_EBX,in_stack_ffffffe8,in_stack_ffffffec,in_stack_fffffff0,
                 flags_1,unaff_ESI);
  uVar1 = *(undefined4 *)(&DAT_00c2a173 + unaff_EBX);
  ___cxa_atexit(unaff_EBX + 0x9f68cf,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<CFuncTrackTrain>::dataDesc[0x1d].flatOffset + unaff_EBX + 0xb),
                 (char *)(unaff_EBX + 0xa4514b),(char *)(unaff_EBX + 0xa3241f),0x6000,
                 (char *)(unaff_EBX + 0xa4512d),in_stack_ffffffe8,in_stack_ffffffec,
                 in_stack_fffffff0,flags_1,unaff_ESI);
  ___cxa_atexit(unaff_EBX + 0x9f68af,0,uVar1);
  return;
}


/* __tcf_0 at 00a132b0 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x383f48),in_stack_00000008);
  return;
}


/* __tcf_1 at 00a13290 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x383fc8),in_stack_00000008);
  return;
}


/* CPortal_PointPush::PushThink at 00258cb0 */

/* DWARF original prototype: void PushThink(CPortal_PointPush * this) */

void __thiscall CPortal_PointPush::PushThink(CPortal_PointPush *this)

{
  Vector *center;
  float flRadius;
  CPortal_Base2D *pPortal;
  CPortal_Base2D *pCVar1;
  int unaff_EBX;
  PortalRadiusExtension_t *pPVar2;
  int local_48;
  PortalRadiusExtensionVector local_38;
  undefined *local_24;
  undefined *local_20;
  
                    /* Unresolved local var: PortalRadiusExtensionVector
                       portalRadiusExtensions@[???] */
  ___i686_get_pc_thunk_bx();
  local_38.m_Memory.m_pMemory = (PortalRadiusExtension_t *)0x0;
  local_38.m_Memory.m_nAllocationCount = 0;
  local_38.m_Memory.m_nGrowSize = 0;
  local_38.m_Size = 0;
  local_38.m_pElements = (PortalRadiusExtension_t *)0x0;
  flRadius = (this->m_flRadius).m_Value;
  if ((((this->super_CBaseEntity).m_iEFlags & 0x800) != 0) &&
     (CBaseEntity::CalcAbsolutePosition(&this->super_CBaseEntity),
     ((this->super_CBaseEntity).m_iEFlags & 0x800) != 0)) {
    CBaseEntity::CalcAbsolutePosition(&this->super_CBaseEntity);
  }
  ExtendRadiusThroughPortals
            (&(this->super_CBaseEntity).m_vecAbsOrigin,&(this->super_CBaseEntity).m_angAbsRotation,
             flRadius,&local_38);
                    /* Unresolved local var: int i@[???] */
  if (0 < local_38.m_Size) {
    local_48 = 0;
    do {
      pPVar2 = local_38.m_Memory.m_pMemory + local_48;
      pPortal = pPVar2->pPortalTo;
      pCVar1 = pPVar2->pPortalFrom;
      center = &pPVar2->vecOrigin;
      local_20 = (undefined *)0xffff00;
      if (pCVar1 != (CPortal_Base2D *)0x0) {
        local_24 = &DAT_00ff00ff;
        local_20 = &DAT_00ff00ff;
      }
      if (*(int *)(*(int *)(&DAT_00b3e5be + unaff_EBX) + 0x30) != 0) {
        NDebugOverlay::Sphere
                  (center,(this->m_flRadius).m_Value,(uint)local_20 & 0xff,
                   (uint)local_20 >> 8 & 0xff,(uint)local_20 >> 0x10,false,0.1);
        NDebugOverlay::Sphere
                  (center,2.0,(uint)local_20 & 0xff,(uint)local_20 >> 8 & 0xff,
                   (uint)local_20 >> 0x10 & 0xff,false,0.1);
      }
      if (pCVar1 != (CPortal_Base2D *)0x0) {
        CreatePortalVortexEffect(this,center,pPortal);
      }
      PushEntitiesAtPoint(this,center,&pPVar2->vecAngles,pPortal);
      PushPaintBlobsAtPoint(this,center,&pPVar2->vecAngles,pPortal);
      local_48 = local_48 + 1;
    } while (local_48 < local_38.m_Size);
  }
  CBaseEntity::SetNextThink
            (&this->super_CBaseEntity,
             *(float *)(**(int **)(unaff_EBX + 0x9edea6) + 0xc) +
             *(float *)(*(int *)(&DAT_00b3e55e + unaff_EBX) + 0x2c),(char *)0x0);
  local_38.m_Size = 0;
  if (-1 < local_38.m_Memory.m_nGrowSize) {
    if (local_38.m_Memory.m_pMemory != (PortalRadiusExtension_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x9ede9e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x9ede9e),local_38.m_Memory.m_pMemory);
      local_38.m_Memory.m_pMemory = (PortalRadiusExtension_t *)0x0;
    }
    local_38.m_Memory.m_nAllocationCount = 0;
  }
  if ((-1 < local_38.m_Memory.m_nGrowSize) &&
     (local_38.m_Memory.m_pMemory != (PortalRadiusExtension_t *)0x0)) {
    local_38.m_pElements = local_38.m_Memory.m_pMemory;
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x9ede9e) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x9ede9e),local_38.m_Memory.m_pMemory);
  }
  return;
}


/* CPortal_PointPush::TestWithinCone at 00258740 */

/* DWARF original prototype: bool TestWithinCone(CPortal_PointPush * this, Vector * vecOrigin,
   Vector * vecEndPos, QAngle * vecAngles, float flMinAngle) */

bool __thiscall
CPortal_PointPush::TestWithinCone
          (CPortal_PointPush *this,Vector *vecOrigin,Vector *vecEndPos,QAngle *vecAngles,
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


/* CPortal_PointPush::PushPaintBlobsAtPoint at 002587e0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void PushPaintBlobsAtPoint(CPortal_PointPush * this, Vector *
   vecOrigin, QAngle * vecAngles, CPortal_Base2D * pPortal) */

void __thiscall
CPortal_PointPush::PushPaintBlobsAtPoint
          (CPortal_PointPush *this,Vector *vecOrigin,QAngle *vecAngles,CPortal_Base2D *pPortal)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  CPaintStream *this_00;
  bool bVar9;
  uint uVar10;
  CBasePaintBlob *this_01;
  Vector *pVVar11;
  int unaff_EBX;
  uint id;
  longdouble lVar12;
  float fVar13;
  float fVar14;
  float fVar15;
  float flRadiusSqr;
  int local_c8;
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
  fVar1 = (this->m_flRadius).m_Value;
  fVar2 = vecOrigin->z;
  fVar3 = vecOrigin->y;
  fVar4 = vecOrigin->x;
  if (0 < *(int *)(*(int *)(unaff_EBX + 0x9ee5c3) + 0xc)) {
    local_c8 = 0;
    do {
      this_00 = *(CPaintStream **)(**(int **)(unaff_EBX + 0x9ee5c3) + local_c8 * 4);
      if ((this_00 != (CPaintStream *)0x0) &&
         (uVar10 = CPaintStream::GetBlobsCount(this_00), 0 < (int)uVar10)) {
        id = 0;
LAB_002588cd:
        do {
          this_01 = CPaintStream::GetBlob(this_00,id);
          if ((this_01 != (CBasePaintBlob *)0x0) &&
             (bVar9 = CBasePaintBlob::IsStreaking(this_01), !bVar9)) {
            pVVar11 = CBasePaintBlob::GetPosition(this_01);
            fVar5 = pVVar11->z;
            fVar6 = pVVar11->y;
            fVar7 = pVVar11->x;
                    /* Unresolved local var: Vector delta@[???] */
            fVar13 = fVar7 - fVar4;
            fVar14 = fVar6 - fVar3;
            fVar15 = fVar5 - fVar2;
            if (fVar13 * fVar13 + fVar14 * fVar14 + fVar15 * fVar15 <= fVar1 * fVar1) {
              fVar8 = (this->m_flConeOfInfluence).m_Value;
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
                lVar12 = (longdouble)_cosf(*(float *)(unaff_EBX + 0x874313) * fVar8);
                if (local_7c.x * local_70.x + local_7c.y * local_70.y + local_7c.z * local_70.z <
                    (float)lVar12) goto LAB_002588c0;
              }
              if (pPortal != (CPortal_Base2D *)0x0) {
                if ((((pPortal->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) &&
                   (CBaseEntity::CalcAbsolutePosition((CBaseEntity *)pPortal),
                   ((pPortal->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0)) {
                  CBaseEntity::CalcAbsolutePosition((CBaseEntity *)pPortal);
                }
                    /* Unresolved local var: Vector res@[???] */
                local_70.x = fVar7 - (pPortal->super_CBaseAnimating).super_CBaseEntity.
                                     m_vecAbsOrigin.x;
                local_70.y = fVar6 - (pPortal->super_CBaseAnimating).super_CBaseEntity.
                                     m_vecAbsOrigin.y;
                local_70.z = fVar5 - (pPortal->super_CBaseAnimating).super_CBaseEntity.
                                     m_vecAbsOrigin.z;
                VectorNormalize(&local_70);
                AngleVectors(&(pPortal->super_CBaseAnimating).super_CBaseEntity.m_angAbsRotation,
                             &local_7c);
                if (local_70.x * local_7c.x + local_70.y * local_7c.y + local_70.z * local_7c.z <
                    0.0) goto LAB_002588c0;
              }
              pVVar11 = CBasePaintBlob::GetVelocity(this_01);
              fVar5 = pVVar11->z;
              fVar6 = pVVar11->y;
              fVar7 = pVVar11->x;
              local_34 = fVar13;
              local_30 = fVar14;
              local_2c = fVar15;
              local_28[0].x = fVar13;
              local_28[0].y = fVar14;
              local_28[0].z = fVar15;
                    /* Unresolved local var: Vector res@[???] */
              VectorNormalize(local_28);
              local_50 = (this->m_flMagnitude).m_Value;
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
              goto LAB_002588cd;
            }
          }
LAB_002588c0:
          id = id + 1;
        } while (uVar10 != id);
      }
      local_c8 = local_c8 + 1;
    } while (local_c8 < *(int *)(*(int *)(unaff_EBX + 0x9ee5c3) + 0xc));
  }
  return;
}


/* _GLOBAL__I_portal_pointpush_think_rate at 0001cb20 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_portal_pointpush_think_rate(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

