/* DWARF-guided pseudocode for game/shared/portal2/hitbox_damaged_entity_shared.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* EmitParticles at 0024bf20 */

void EmitParticles(CBaseAnimating *pOwnerEntity,DamagedEntityType entityType,int damageEffectIndex,
                  int particleEffectIndex)

{
  DamageInfo *pDVar1;
  Vector vecStart;
  DamageInfoVector *pDVar2;
  int iAttachmentPoint;
  int unaff_EBX;
  
                    /* Unresolved local var: DamageInfoVector * damageInfoVector@[???]
                       Unresolved local var: DamageInfo * damageInfo@[???] */
  ___i686_get_pc_thunk_bx();
  pDVar2 = DamageDatabase::GetDamageInfoVector
                     (*(DamageDatabase **)(unaff_EBX + 0x9fafac),entityType);
  if ((((-1 < damageEffectIndex) && (damageEffectIndex < pDVar2->m_Size)) &&
      (pDVar1 = (pDVar2->m_Memory).m_pMemory, -1 < particleEffectIndex)) &&
     (particleEffectIndex < pDVar1[damageEffectIndex].particleEffectCount)) {
    if (pDVar1[damageEffectIndex].particleAttachment[0] == '\0') {
      if (pDVar1[damageEffectIndex].particleAttachmentHitbox == -1) {
        DispatchParticleEffect
                  (pDVar1[damageEffectIndex].particleEffects[particleEffectIndex],
                   PATTACH_ABSORIGIN_FOLLOW,&pOwnerEntity->super_CBaseEntity,-1,false,-1,
                   (IRecipientFilter *)0x0);
      }
      else {
        vecStart.y = 0.0;
        vecStart.z = 0.0;
        vecStart.x = (float)pDVar1[damageEffectIndex].particleAttachmentHitbox;
        DispatchParticleEffect
                  (pDVar1[damageEffectIndex].particleEffects[particleEffectIndex],
                   **(Vector **)(unaff_EBX + 0x9fac44),vecStart,
                   **(QAngle **)(&DAT_009facd4 + unaff_EBX),&pOwnerEntity->super_CBaseEntity,-1);
      }
    }
    else {
                    /* Unresolved local var: int attachmentIndex@[???] */
      iAttachmentPoint =
           CBaseAnimating::LookupAttachment
                     (pOwnerEntity,pDVar1[damageEffectIndex].particleAttachment);
      if (0 < iAttachmentPoint) {
        DispatchParticleEffect
                  (pDVar1[damageEffectIndex].particleEffects[particleEffectIndex],
                   PATTACH_POINT_FOLLOW,&pOwnerEntity->super_CBaseEntity,iAttachmentPoint,false,-1,
                   (IRecipientFilter *)0x0);
      }
    }
  }
  return;
}


/* EmitGibs at 0024c0d0 */

/* WARNING: Type propagation algorithm not settling */

void EmitGibs(CBaseAnimating *pParentEntity,DamagedEntityType entityType,int damageEffectIndex)

{
  undefined1 *puVar1;
  uint uVar2;
  uint *puVar3;
  CStudioHdr *szGibModel;
  float fVar4;
  ushort uVar5;
  DamageInfo *pDVar6;
  IPhysicsObject *pIVar7;
  CBaseEdict *this;
  ushort *puVar8;
  int *piVar9;
  DamageInfoVector *pDVar10;
  int iVar11;
  studiohdr_t *pStudioHdr;
  CBaseEntity *pCVar12;
  IChangeInfoAccessor *pIVar13;
  float *pfVar14;
  uint uVar15;
  int unaff_EBX;
  CGib *this_00;
  uint uVar16;
  CBaseAnimating *vecImpulse;
  CStudioHdr *this_1;
  matrix3x4_t *local_188;
  DamageInfo *damageInfo;
  int local_180;
  CStudioHdr local_164;
  matrix3x4_t local_100;
  matrix3x4_t local_d0;
  IHandleEntity local_a0;
  float local_9c;
  _func___thiscall_void_CBaseEntity_ptr *local_98;
  float local_94;
  float local_90;
  float local_8c;
  IHandleEntity local_88;
  CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iObjectCapsCache> local_84;
  _func___thiscall_void_CBaseEntity_ptr *local_80;
  _func_int_varargs **local_7c;
  _func_int_varargs **local_78;
  CBaseEntity *local_74;
  IServerNetworkable local_70;
  IEventRegisterCallback local_6c;
  CBaseEntity *local_68;
  edict_t *local_64;
  vec_t local_60;
  ushort *local_5c;
  float local_58;
  float local_54;
  float local_50;
  Vector local_4c;
  ServerClass *local_40;
  CBaseHandle local_3c;
  CTimedEventMgr *local_38;
  Vector local_34;
  Vector local_28 [2];
  
                    /* Unresolved local var: DamageInfoVector * damageInfoVector@[???]
                       Unresolved local var: matrix3x4_t localToWorld@[???]
                       Unresolved local var: Vector parentOrigin@[???]
                       Unresolved local var: int parentAttachment@[???]
                       Unresolved local var: matrix3x4_t * parentTransform@[???]
                       Unresolved local var: Vector burstCenter@[???]
                       Unresolved local var: int parentBurstCenterAttachment@[???] */
  ___i686_get_pc_thunk_bx();
  if ((((pParentEntity != (CBaseAnimating *)0x0) &&
       (pDVar10 = DamageDatabase::GetDamageInfoVector
                            (*(DamageDatabase **)(&DAT_009fadff + unaff_EBX),entityType),
       -1 < damageEffectIndex)) && (damageEffectIndex < pDVar10->m_Size)) &&
     (pDVar6 = (pDVar10->m_Memory).m_pMemory, 0 < pDVar6[damageEffectIndex].gibCount)) {
    pfVar14 = *(float **)(&DAT_009faa97 + unaff_EBX);
    local_28[0].x = *pfVar14;
    local_28[0].y = pfVar14[1];
    local_28[0].z = pfVar14[2];
    iVar11 = CBaseAnimating::LookupAttachment(pParentEntity,(char *)(unaff_EBX + 0x814f6f));
    if (iVar11 < 1) {
      SetIdentityMatrix(&local_d0);
    }
    else {
      CBaseAnimating::GetAttachmentLocal(pParentEntity,iVar11,&local_d0);
      MatrixGetColumn(&local_d0,3,local_28);
    }
    local_188 = &local_d0;
    if ((((pParentEntity->super_CBaseEntity).m_iEFlags & 0x800) != 0) &&
       (CBaseEntity::CalcAbsolutePosition(&pParentEntity->super_CBaseEntity),
       ((pParentEntity->super_CBaseEntity).m_iEFlags & 0x800) != 0)) {
      CBaseEntity::CalcAbsolutePosition(&pParentEntity->super_CBaseEntity);
    }
    local_34.x = (pParentEntity->super_CBaseEntity).m_vecAbsOrigin.x;
    local_34.y = (pParentEntity->super_CBaseEntity).m_vecAbsOrigin.y;
    local_34.z = (pParentEntity->super_CBaseEntity).m_vecAbsOrigin.z;
    iVar11 = CBaseAnimating::LookupAttachment(pParentEntity,(char *)(unaff_EBX + 0x814f7f));
    if (0 < iVar11) {
                    /* Unresolved local var: matrix3x4_t burstCenterTransform@[???] */
      (*(pParentEntity->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.
        super_IHandleEntity._vptr_IHandleEntity[0xd9])(pParentEntity,iVar11,&local_100);
      MatrixGetColumn(&local_100,3,&local_34);
    }
    if (0 < pDVar6[damageEffectIndex].gibCount) {
      local_180 = 0;
      do {
        szGibModel = (CStudioHdr *)(pDVar6[damageEffectIndex].gibModels + local_180);
        this_1 = szGibModel;
        iVar11 = (**(code **)(*(int *)**(undefined4 **)(&DAT_009faa8f + unaff_EBX) + 0xc))
                           ((int *)**(undefined4 **)(&DAT_009faa8f + unaff_EBX));
        if (iVar11 < 1) {
          _Warning(unaff_EBX + 0x814f8b,szGibModel);
        }
        else {
          CStudioHdr::CStudioHdr(&local_164,this_1);
          iVar11 = (**(code **)(*(int *)**(undefined4 **)(&DAT_009faa8f + unaff_EBX) + 8))
                             ((int *)**(undefined4 **)(&DAT_009faa8f + unaff_EBX),iVar11);
          if (iVar11 != 0) {
            pStudioHdr = (studiohdr_t *)
                         (**(code **)(*(int *)**(undefined4 **)(&DAT_009faa8f + unaff_EBX) + 0x78))
                                   ((int *)**(undefined4 **)(&DAT_009faa8f + unaff_EBX),iVar11);
            CStudioHdr::Init(&local_164,pStudioHdr,(IMDLCache *)0x0);
          }
          pfVar14 = *(float **)(&DAT_009faa97 + unaff_EBX);
          local_40 = (ServerClass *)*pfVar14;
          local_3c.m_Index = (ulong)pfVar14[1];
          local_38 = (CTimedEventMgr *)pfVar14[2];
          local_4c.x = local_28[0].x;
          local_4c.y = local_28[0].y;
          local_4c.z = local_28[0].z;
          if (((pParentEntity->super_CBaseEntity).m_iEFlags & 0x800) != 0) {
            CBaseEntity::CalcAbsolutePosition(&pParentEntity->super_CBaseEntity);
          }
          iVar11 = Studio_FindAttachment(&local_164,(char *)(unaff_EBX + 0x814f6f));
          if (iVar11 != -1 && -1 < iVar11 + 1) {
            GetAttachmentLocalSpace(&local_164,iVar11,local_188);
            MatrixGetColumn(local_188,3,&local_4c);
            local_4c.x = local_4c.x - local_28[0].x;
            local_4c.y = local_4c.y - local_28[0].y;
            local_4c.z = local_4c.z - local_28[0].z;
          }
          puVar3 = (uint *)(&DAT_00881f4f + unaff_EBX);
          local_8c = (float)((uint)local_4c.z ^ *puVar3);
          local_90 = (float)((uint)local_4c.y ^ *puVar3);
          local_94 = (float)((uint)local_4c.x ^ *puVar3);
          local_58 = local_94;
          local_54 = local_90;
          local_50 = local_8c;
          VectorTransform(&local_58,&(pParentEntity->super_CBaseEntity).m_rgflCoordinateFrame,
                          (float *)&local_40);
          if (((pParentEntity->super_CBaseEntity).m_iEFlags & 0x1000) != 0) {
            CBaseEntity::CalcAbsoluteVelocity(&pParentEntity->super_CBaseEntity);
          }
          local_64 = (edict_t *)(pParentEntity->super_CBaseEntity).m_vecAbsVelocity.x;
          local_60 = (pParentEntity->super_CBaseEntity).m_vecAbsVelocity.y;
          local_5c = (ushort *)(pParentEntity->super_CBaseEntity).m_vecAbsVelocity.z;
          pIVar7 = (pParentEntity->super_CBaseEntity).m_pPhysicsObject;
          if (pIVar7 != (IPhysicsObject *)0x0) {
            (*pIVar7->_vptr_IPhysicsObject[0x37])(pIVar7,&local_40,&local_64);
          }
          pCVar12 = CreateEntityByName((char *)(unaff_EBX + 0x814fb8),-1,true);
          this_00 = (CGib *)0x0;
          if ((pCVar12 == (CBaseEntity *)0x0) ||
             (this_00 = (CGib *)___dynamic_cast(pCVar12,*(undefined4 *)(&DAT_009faaaf + unaff_EBX),
                                                *(undefined4 *)(&DAT_009fadfb + unaff_EBX),0),
             this_00 == (CGib *)0x0)) {
            _Warning(unaff_EBX + 0x809d37,(char *)(unaff_EBX + 0x814fb8));
          }
          CGib::Spawn(this_00,(char *)szGibModel,25.0);
          if ((this_00->super_CBaseAnimating).m_nBody.m_Value != local_180) {
            if ((this_00->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.
                m_bRegistered == false) {
              this = &((this_00->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                      super_CBaseEdict;
              if ((this != (CBaseEdict *)0x0) && ((this->m_fStateFlags & 0x100U) == 0)) {
                this->m_fStateFlags = this->m_fStateFlags | 1;
                pIVar13 = CBaseEdict::GetChangeAccessor(this);
                puVar8 = (ushort *)**(undefined4 **)(&DAT_009faa87 + unaff_EBX);
                if (pIVar13->m_iChangeInfoSerialNumber == *puVar8) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                  uVar16 = (uint)pIVar13->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
                  uVar5 = puVar8[uVar16 * 0x14 + 0x14];
                  if (uVar5 == 0) {
LAB_0024c8c2:
                    puVar8[(uint)uVar5 + uVar16 * 0x14 + 1] = 0x3e4;
                    puVar8[uVar16 * 0x14 + 0x14] = uVar5 + 1;
                  }
                  else if (puVar8[uVar16 * 0x14 + 1] != 0x3e4) {
                    uVar15 = 0;
                    do {
                      uVar2 = uVar15 + 1;
                      uVar15 = uVar2 & 0xffff;
                      if ((ushort)uVar2 == uVar5) {
                        if (uVar5 != 0x13) goto LAB_0024c8c2;
                        goto LAB_0024c8f0;
                      }
                    } while (puVar8[uVar16 * 0x14 + uVar15 + 1] != 0x3e4);
                  }
                }
                else if (puVar8[0x7d1] == 100) {
LAB_0024c8f0:
                  pIVar13->m_iChangeInfoSerialNumber = 0;
                  this->m_fStateFlags = this->m_fStateFlags | 0x100;
                }
                else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                  pIVar13->m_iChangeInfo = puVar8[0x7d1];
                  piVar9 = *(int **)(&DAT_009faa87 + unaff_EBX);
                  *(short *)(*piVar9 + 0xfa2) = *(short *)(*piVar9 + 0xfa2) + 1;
                  pIVar13->m_iChangeInfoSerialNumber = *(ushort *)*piVar9;
                  iVar11 = *piVar9 + (uint)pIVar13->m_iChangeInfo * 0x28;
                  *(undefined2 *)(iVar11 + 2) = 0x3e4;
                  *(undefined2 *)(iVar11 + 0x28) = 1;
                }
              }
            }
            else {
              puVar1 = &(this_00->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
              *(uint *)puVar1 = *(uint *)puVar1 | 1;
            }
            (this_00->super_CBaseAnimating).m_nBody.m_Value = local_180;
          }
          CGib::InitGib(this_00,&pParentEntity->super_CBaseEntity,0.0,0.0);
          this_00->m_lifeTime = pDVar6[damageEffectIndex].gibLifeTime;
          vecImpulse = pParentEntity;
          (*(this_00->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.
            super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x13])(this_00);
          if (((pParentEntity->super_CBaseEntity).m_fEffects.m_Value & 0x10) != 0) {
            vecImpulse = (CBaseAnimating *)&vgui::Panel::GetMessageMap::s_pMap.ncmds;
            CBaseEntity::AddEffects((CBaseEntity *)this_00,0x10);
          }
          if (pDVar6[damageEffectIndex].gibSeparationSpeed != 0.0) {
                    /* Unresolved local var: Vector res@[???] */
            local_100.m_flMatVal[0][0] = (float)local_40 - local_34.x;
            local_100.m_flMatVal[0][1] = (float)local_3c.m_Index - local_34.y;
            local_100.m_flMatVal[0][2] = (float)local_38 - local_34.z;
            pfVar14 = *(float **)(&DAT_009faa97 + unaff_EBX);
            local_70._vptr_IServerNetworkable = (_func_int_varargs **)local_100.m_flMatVal[0][0];
            local_6c._vptr_IEventRegisterCallback = (_func_int_varargs **)local_100.m_flMatVal[0][1]
            ;
            local_68 = (CBaseEntity *)local_100.m_flMatVal[0][2];
            if ((*pfVar14 == local_100.m_flMatVal[0][0]) &&
               (!NAN(*pfVar14) && !NAN(local_100.m_flMatVal[0][0]))) {
              if ((pfVar14[1] == local_100.m_flMatVal[0][1]) &&
                 (!NAN(pfVar14[1]) && !NAN(local_100.m_flMatVal[0][1]))) {
                if ((pfVar14[2] == local_100.m_flMatVal[0][2]) &&
                   (!NAN(pfVar14[2]) && !NAN(local_100.m_flMatVal[0][2]))) {
                  pfVar14 = (float *)(*(this_00->super_CBaseAnimating).super_CBaseEntity.
                                       super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
                                       _vptr_IHandleEntity[0x9d])(this_00);
                    /* Unresolved local var: Vector res@[???] */
                  local_100.m_flMatVal[0][0] = *pfVar14 - local_34.x;
                  local_100.m_flMatVal[0][1] = pfVar14[1] - local_34.y;
                  local_100.m_flMatVal[0][2] = pfVar14[2] - local_34.z;
                  local_7c = (_func_int_varargs **)local_100.m_flMatVal[0][0];
                  local_78 = (_func_int_varargs **)local_100.m_flMatVal[0][1];
                  local_74 = (CBaseEntity *)local_100.m_flMatVal[0][2];
                }
              }
            }
            VectorNormalize((Vector *)&local_100);
            fVar4 = pDVar6[damageEffectIndex].gibSeparationSpeed;
                    /* Unresolved local var: Vector res@[???] */
            local_a0._vptr_IHandleEntity =
                 (_func_int_varargs **)(fVar4 * local_100.m_flMatVal[0][0]);
            local_9c = fVar4 * local_100.m_flMatVal[0][1];
            local_98 = (_func___thiscall_void_CBaseEntity_ptr *)(fVar4 * local_100.m_flMatVal[0][2])
            ;
            vecImpulse = (CBaseAnimating *)&local_88;
            local_88._vptr_IHandleEntity = local_a0._vptr_IHandleEntity;
            local_84.m_Value = (int)local_9c;
            local_80 = local_98;
            CBaseEntity::ApplyAbsVelocityImpulse((CBaseEntity *)this_00,(Vector *)vecImpulse);
          }
          CStudioHdr::~CStudioHdr(&local_164,(int)vecImpulse);
        }
        local_180 = local_180 + 1;
      } while (local_180 < pDVar6[damageEffectIndex].gibCount);
    }
  }
  return;
}


/* __static_initialization_and_destruction_0 at 0001bd40 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  int unaff_EBX;
  longlong lVar1;
  
  lVar1 = ___i686_get_pc_thunk_bx();
  if (lVar1 != 0xffff00000001) {
    return;
  }
  (&CDmxElement::s_TypeSymbols.m_Lookup.
    super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
    .m_aBuckets[0x464].m_AddLock.field_0xf)[unaff_EBX] = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x465].m_pFirst + unaff_EBX) = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x465].m_pFirst + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x465].m_pFirst + unaff_EBX + 2) = 0;
  *(undefined4 *)
   (&UNK_000069b3 +
   (int)&((CUtlMemoryPool *)
         (&CDmxElement::s_TypeSymbols.m_Lookup.
           super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
           .m_EntryMemory.super_CUtlMemoryPool.m_BlobHead + -2))->m_BlobHead +
   unaff_EBX + 0xffffffe0U) = 0;
  *(undefined4 *)
   (&UNK_000069b7 +
   (int)&((CUtlMemoryPool *)
         (&CDmxElement::s_TypeSymbols.m_Lookup.
           super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
           .m_EntryMemory.super_CUtlMemoryPool.m_BlobHead + -2))->m_BlobHead +
   unaff_EBX + 0xffffffe0U) = 0;
  *(undefined4 *)
   (&UNK_000069bb +
   (int)&((CUtlMemoryPool *)
         (&CDmxElement::s_TypeSymbols.m_Lookup.
           super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
           .m_EntryMemory.super_CUtlMemoryPool.m_BlobHead + -2))->m_BlobHead +
   unaff_EBX + 0xffffffe0U) = 0x7f7fffff;
  *(undefined4 *)
   (&UNK_000069bf +
   (int)&((CUtlMemoryPool *)
         (&CDmxElement::s_TypeSymbols.m_Lookup.
           super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
           .m_EntryMemory.super_CUtlMemoryPool.m_BlobHead + -2))->m_BlobHead +
   unaff_EBX + 0xffffffe0U) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x465].m_AddLock.m_nWriters.m_value + unaff_EBX + 3) = 0;
  *(undefined4 *)
   (&CDmxElement::s_TypeSymbols.m_Lookup.
     super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
     .m_aBuckets[0x465].m_AddLock.field_0xf + unaff_EBX) = 0;
  *(undefined4 *)
   (&UNK_00006993 +
   (int)&CDmxElement::s_TypeSymbols.m_Lookup.
         super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
         .m_aBuckets[0].m_pFirst + unaff_EBX) = 0;
  *(undefined4 *)
   (&UNK_00006997 +
   (int)&CDmxElement::s_TypeSymbols.m_Lookup.
         super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
         .m_aBuckets[0].m_pFirst + unaff_EBX) = 0;
  *(undefined4 *)
   (&UNK_0000699b +
   (int)&CDmxElement::s_TypeSymbols.m_Lookup.
         super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
         .m_aBuckets[0].m_pFirst + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)
   (&UNK_0000699f +
   (int)&CDmxElement::s_TypeSymbols.m_Lookup.
         super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
         .m_aBuckets[0].m_pFirst + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x466].m_AddLock.m_nWriters.m_value + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)
   (&CDmxElement::s_TypeSymbols.m_Lookup.
     super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
     .m_aBuckets[0x466].m_AddLock.field_0xf + unaff_EBX) = 0x7f7fffff;
  *(undefined **)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x467].m_pFirst + unaff_EBX + 3) = &UNK_00c305bf + unaff_EBX;
  return;
}


/* _GLOBAL__I__Z13EmitParticlesP14CBaseAnimating17DamagedEntityTypeii at 0001bde0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__Z13EmitParticlesP14CBaseAnimating17DamagedEntityTypeii(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

