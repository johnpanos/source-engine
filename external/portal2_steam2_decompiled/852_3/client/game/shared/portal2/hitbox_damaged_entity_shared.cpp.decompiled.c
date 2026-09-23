/* DWARF-guided pseudocode for game/shared/portal2/hitbox_damaged_entity_shared.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* EmitParticles at 001b3750 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

void EmitParticles(C_BaseAnimating *pOwnerEntity,DamagedEntityType entityType,int damageEffectIndex,
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
                     (*(DamageDatabase **)("m_StartColor" + unaff_EBX + 1),entityType);
  if ((((-1 < damageEffectIndex) && (damageEffectIndex < pDVar2->m_Size)) &&
      (pDVar1 = (pDVar2->m_Memory).m_pMemory, -1 < particleEffectIndex)) &&
     (particleEffectIndex < pDVar1[damageEffectIndex].particleEffectCount)) {
    if (pDVar1[damageEffectIndex].particleAttachment[0] == '\0') {
      if (pDVar1[damageEffectIndex].particleAttachmentHitbox == -1) {
        DispatchParticleEffect
                  (pDVar1[damageEffectIndex].particleEffects[particleEffectIndex],
                   PATTACH_ABSORIGIN_FOLLOW,&pOwnerEntity->super_C_BaseEntity,-1,false,-1,
                   (IRecipientFilter *)0x0);
      }
      else {
        vecStart.y = 0.0;
        vecStart.z = 0.0;
        vecStart.x = (float)pDVar1[damageEffectIndex].particleAttachmentHitbox;
        DispatchParticleEffect
                  (pDVar1[damageEffectIndex].particleEffects[particleEffectIndex],
                   **(Vector **)("materials/vgui/%s/%s" + unaff_EBX + 1),vecStart,
                   **(QAngle **)("m_chCurrentSlideLists[0]" + unaff_EBX + 0x13),
                   &pOwnerEntity->super_C_BaseEntity,-1);
      }
    }
    else {
                    /* Unresolved local var: int attachmentIndex@[???] */
      iAttachmentPoint =
           (*(pOwnerEntity->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.
             super_IHandleEntity._vptr_IHandleEntity[0x71])
                     (pOwnerEntity,pDVar1[damageEffectIndex].particleAttachment);
      if (0 < iAttachmentPoint) {
        DispatchParticleEffect
                  (pDVar1[damageEffectIndex].particleEffects[particleEffectIndex],
                   PATTACH_POINT_FOLLOW,&pOwnerEntity->super_C_BaseEntity,iAttachmentPoint,false,-1,
                   (IRecipientFilter *)0x0);
      }
    }
  }
  return;
}


/* EmitGibs at 001b3900 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "DmAttributeType_t": Some values do not have unique names */

void EmitGibs(C_BaseAnimating *pParentEntity,DamagedEntityType entityType,int damageEffectIndex)

{
  uint *puVar1;
  CStudioHdr *pszModelName;
  DamageInfo *pDVar2;
  undefined4 *puVar3;
  IPhysicsObject *pIVar4;
  Vector *pVVar5;
  Vector vecOrigin;
  AngularImpulse vecAngularImp;
  DamageInfoVector *pDVar6;
  int iVar7;
  float *pfVar8;
  studiohdr_t *pStudioHdr;
  C_Gib *this;
  int unaff_EBX;
  CStudioHdr *this_1;
  matrix3x4_t *local_17c;
  DamageInfo *damageInfo;
  int local_170;
  CStudioHdr local_164;
  matrix3x4_t local_100;
  matrix3x4_t local_d0;
  float local_a0;
  float local_9c;
  float local_98;
  float local_94;
  uint local_90;
  uint local_8c;
  Vector local_88;
  float local_7c;
  float local_78;
  float local_74;
  float local_70;
  float local_6c;
  float local_68;
  vec_t local_64;
  vec_t local_60;
  vec_t local_5c;
  float local_58;
  uint local_54;
  uint local_50;
  Vector local_4c;
  float local_40;
  float local_3c;
  vec_t local_38;
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
  if ((((pParentEntity != (C_BaseAnimating *)0x0) &&
       (pDVar6 = DamageDatabase::GetDamageInfoVector
                           (*(DamageDatabase **)("m_AmbientLight.m_vColor" + unaff_EBX + 0x17),
                            entityType), -1 < damageEffectIndex)) &&
      (damageEffectIndex < pDVar6->m_Size)) &&
     (pDVar2 = (pDVar6->m_Memory).m_pMemory, 0 < pDVar2[damageEffectIndex].gibCount)) {
    pfVar8 = *(float **)("%s : %8.2f:  finish %s\n" + unaff_EBX + 0x13);
    local_28[0].x = *pfVar8;
    local_28[0].y = pfVar8[1];
    local_28[0].z = pfVar8[2];
    iVar7 = (*(pParentEntity->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.
              super_IHandleEntity._vptr_IHandleEntity[0x71])
                      (pParentEntity,C_OP_RemapSpeedtoCP::InitParams + unaff_EBX + 7);
    if (iVar7 < 1) {
      SetIdentityMatrix(&local_d0);
    }
    else {
      C_BaseAnimating::GetAttachmentLocal(pParentEntity,iVar7,&local_d0);
      MatrixGetColumn(&local_d0,3,local_28);
    }
    local_17c = &local_d0;
    C_BaseEntity::CalcAbsolutePosition(&pParentEntity->super_C_BaseEntity);
    pfVar8 = (float *)(*(pParentEntity->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown
                        .super_IHandleEntity._vptr_IHandleEntity[0xd])(pParentEntity);
    local_34.x = *pfVar8;
    local_34.y = pfVar8[1];
    local_34.z = pfVar8[2];
    iVar7 = (*(pParentEntity->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.
              super_IHandleEntity._vptr_IHandleEntity[0x71])(pParentEntity,unaff_EBX + 0x7b5457);
    if (0 < iVar7) {
                    /* Unresolved local var: matrix3x4_t burstCenterTransform@[???] */
      (*(pParentEntity->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.
        super_IHandleEntity._vptr_IHandleEntity[0x72])(pParentEntity,iVar7,&local_100);
      MatrixGetColumn(&local_100,3,&local_34);
    }
    if (0 < pDVar2[damageEffectIndex].gibCount) {
      local_170 = 0;
      puVar3 = *(undefined4 **)("Unable to restore binary scene \'%s\'\n" + unaff_EBX + 0x1b);
      do {
        pszModelName = (CStudioHdr *)(pDVar2[damageEffectIndex].gibModels + local_170);
        this_1 = pszModelName;
        iVar7 = (**(code **)(*(int *)**(undefined4 **)("%s : %8.2f:  finish %s\n" + unaff_EBX + 0xb)
                            + 0xc))((int *)**(undefined4 **)
                                             ("%s : %8.2f:  finish %s\n" + unaff_EBX + 0xb));
        if (iVar7 < 1) {
          _Warning(unaff_EBX + 0x7b5463,pszModelName);
        }
        else {
          CStudioHdr::CStudioHdr(&local_164,this_1);
          iVar7 = (**(code **)(*(int *)**(undefined4 **)
                                         ("%s : %8.2f:  finish %s\n" + unaff_EBX + 0xb) + 8))
                            ((int *)**(undefined4 **)("%s : %8.2f:  finish %s\n" + unaff_EBX + 0xb),
                             iVar7);
          if (iVar7 != 0) {
            pStudioHdr = (studiohdr_t *)
                         (**(code **)(*(int *)**(undefined4 **)
                                                ("%s : %8.2f:  finish %s\n" + unaff_EBX + 0xb) +
                                     0x78))((int *)**(undefined4 **)
                                                     ("%s : %8.2f:  finish %s\n" + unaff_EBX + 0xb),
                                            iVar7);
            CStudioHdr::Init(&local_164,pStudioHdr,(IMDLCache *)0x0);
          }
          pfVar8 = *(float **)("%s : %8.2f:  finish %s\n" + unaff_EBX + 0x13);
          local_40 = *pfVar8;
          local_3c = pfVar8[1];
          local_38 = pfVar8[2];
          local_4c.x = local_28[0].x;
          local_4c.y = local_28[0].y;
          local_4c.z = local_28[0].z;
          (*(pParentEntity->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.
            super_IHandleEntity._vptr_IHandleEntity[0xe])(pParentEntity);
          iVar7 = Studio_FindAttachment
                            (&local_164,(char *)(C_OP_RemapSpeedtoCP::InitParams + unaff_EBX + 7));
          if (iVar7 != -1 && -1 < iVar7 + 1) {
            GetAttachmentLocalSpace(&local_164,iVar7,local_17c);
            MatrixGetColumn(local_17c,3,&local_4c);
            local_4c.x = local_4c.x - local_28[0].x;
            local_4c.y = local_4c.y - local_28[0].y;
            local_4c.z = local_4c.z - local_28[0].z;
          }
          puVar1 = (uint *)(&DAT_0080419f + unaff_EBX);
          local_8c = (uint)local_4c.z ^ *puVar1;
          local_90 = (uint)local_4c.y ^ *puVar1;
          local_94 = (float)((uint)local_4c.x ^ *puVar1);
          local_58 = local_94;
          local_54 = local_90;
          local_50 = local_8c;
          VectorTransform(&local_58,&(pParentEntity->super_C_BaseEntity).m_rgflCoordinateFrame,
                          &local_40);
          C_BaseEntity::CalcAbsoluteVelocity(&pParentEntity->super_C_BaseEntity);
          local_64 = (pParentEntity->super_C_BaseEntity).m_vecAbsVelocity.x;
          local_60 = (pParentEntity->super_C_BaseEntity).m_vecAbsVelocity.y;
          local_5c = (pParentEntity->super_C_BaseEntity).m_vecAbsVelocity.z;
          pIVar4 = (pParentEntity->super_C_BaseEntity).m_pPhysicsObject;
          if (pIVar4 != (IPhysicsObject *)0x0) {
            (*pIVar4->_vptr_IPhysicsObject[0x37])(pIVar4,&local_40,&local_64);
          }
          pVVar5 = *(Vector **)("%s : %8.2f:  finish %s\n" + unaff_EBX + 0x13);
          vecAngularImp.x = pVVar5->x;
          vecAngularImp.y = pVVar5->y;
          vecAngularImp.z = pVVar5->z;
          vecOrigin.y = local_3c;
          vecOrigin.x = local_40;
          vecOrigin.z = local_38;
          this = C_Gib::CreateClientsideGib
                           ((char *)pszModelName,vecOrigin,*pVVar5,vecAngularImp,
                            pDVar2[damageEffectIndex].gibLifeTime);
          if (this != (C_Gib *)0x0) {
            if (((pParentEntity->super_C_BaseEntity).m_fEffects & 0x10) != 0) {
              C_BaseEntity::AddEffects((C_BaseEntity *)this,0x10);
            }
            if (pDVar2[damageEffectIndex].gibSeparationSpeed != 0.0) {
                    /* Unresolved local var: Vector res@[???] */
              local_100.m_flMatVal[0][0] = local_40 - local_34.x;
              local_100.m_flMatVal[0][1] = local_3c - local_34.y;
              local_100.m_flMatVal[0][2] = local_38 - local_34.z;
              pfVar8 = *(float **)("%s : %8.2f:  finish %s\n" + unaff_EBX + 0x13);
              local_70 = local_100.m_flMatVal[0][0];
              local_6c = local_100.m_flMatVal[0][1];
              local_68 = local_100.m_flMatVal[0][2];
              if ((*pfVar8 == local_100.m_flMatVal[0][0]) &&
                 (!NAN(*pfVar8) && !NAN(local_100.m_flMatVal[0][0]))) {
                if ((pfVar8[1] == local_100.m_flMatVal[0][1]) &&
                   (!NAN(pfVar8[1]) && !NAN(local_100.m_flMatVal[0][1]))) {
                  if ((pfVar8[2] == local_100.m_flMatVal[0][2]) &&
                     (!NAN(pfVar8[2]) && !NAN(local_100.m_flMatVal[0][2]))) {
                    pfVar8 = (float *)(*(this->super_C_BaseAnimating).super_C_BaseEntity.
                                        super_IClientEntity.super_IClientUnknown.super_IHandleEntity
                                        ._vptr_IHandleEntity[0x6d])(this);
                    /* Unresolved local var: Vector res@[???] */
                    local_100.m_flMatVal[0][0] = *pfVar8 - local_34.x;
                    local_100.m_flMatVal[0][1] = pfVar8[1] - local_34.y;
                    local_100.m_flMatVal[0][2] = pfVar8[2] - local_34.z;
                    local_7c = local_100.m_flMatVal[0][0];
                    local_78 = local_100.m_flMatVal[0][1];
                    local_74 = local_100.m_flMatVal[0][2];
                  }
                }
              }
              VectorNormalize((Vector *)&local_100);
              local_98 = pDVar2[damageEffectIndex].gibSeparationSpeed;
                    /* Unresolved local var: Vector res@[???] */
              local_a0 = local_98 * local_100.m_flMatVal[0][0];
              local_9c = local_98 * local_100.m_flMatVal[0][1];
              local_98 = local_98 * local_100.m_flMatVal[0][2];
              local_88.x = local_a0;
              local_88.y = local_9c;
              local_88.z = local_98;
              C_BaseEntity::ApplyAbsVelocityImpulse((C_BaseEntity *)this,&local_88);
            }
          }
          CStudioHdr::Term(&local_164);
          local_164.m_boneParent.m_Size = 0;
          if (-1 < local_164.m_boneParent.m_Memory.m_nGrowSize) {
            if (local_164.m_boneParent.m_Memory.m_pMemory != (int *)0x0) {
              (**(code **)(*(int *)*puVar3 + 8))
                        ((int *)*puVar3,local_164.m_boneParent.m_Memory.m_pMemory);
              local_164.m_boneParent.m_Memory.m_pMemory = (int *)0x0;
            }
            local_164.m_boneParent.m_Memory.m_nAllocationCount = 0;
          }
          local_164.m_boneParent.m_pElements = local_164.m_boneParent.m_Memory.m_pMemory;
          if (-1 < local_164.m_boneParent.m_Memory.m_nGrowSize) {
            if (local_164.m_boneParent.m_Memory.m_pMemory != (int *)0x0) {
              (**(code **)(*(int *)*puVar3 + 8))
                        ((int *)*puVar3,local_164.m_boneParent.m_Memory.m_pMemory);
              local_164.m_boneParent.m_Memory.m_pMemory = (int *)0x0;
            }
            local_164.m_boneParent.m_Memory.m_nAllocationCount = 0;
          }
          local_164.m_boneFlags.m_Size = 0;
          if (-1 < local_164.m_boneFlags.m_Memory.m_nGrowSize) {
            if (local_164.m_boneFlags.m_Memory.m_pMemory != (int *)0x0) {
              (**(code **)(*(int *)*puVar3 + 8))
                        ((int *)*puVar3,local_164.m_boneFlags.m_Memory.m_pMemory);
              local_164.m_boneFlags.m_Memory.m_pMemory = (int *)0x0;
            }
            local_164.m_boneFlags.m_Memory.m_nAllocationCount = 0;
          }
          local_164.m_boneFlags.m_pElements = local_164.m_boneFlags.m_Memory.m_pMemory;
          if (-1 < local_164.m_boneFlags.m_Memory.m_nGrowSize) {
            if (local_164.m_boneFlags.m_Memory.m_pMemory != (int *)0x0) {
              (**(code **)(*(int *)*puVar3 + 8))
                        ((int *)*puVar3,local_164.m_boneFlags.m_Memory.m_pMemory);
              local_164.m_boneFlags.m_Memory.m_pMemory = (int *)0x0;
            }
            local_164.m_boneFlags.m_Memory.m_nAllocationCount = 0;
          }
          local_164.m_pStudioHdrCache.m_Size = 0;
          if (-1 < local_164.m_pStudioHdrCache.m_Memory.m_nGrowSize) {
            if (local_164.m_pStudioHdrCache.m_Memory.m_pMemory != (studiohdr_t **)0x0) {
              (**(code **)(*(int *)*puVar3 + 8))
                        ((int *)*puVar3,local_164.m_pStudioHdrCache.m_Memory.m_pMemory);
              local_164.m_pStudioHdrCache.m_Memory.m_pMemory = (studiohdr_t **)0x0;
            }
            local_164.m_pStudioHdrCache.m_Memory.m_nAllocationCount = 0;
          }
          local_164.m_pStudioHdrCache.m_pElements = local_164.m_pStudioHdrCache.m_Memory.m_pMemory;
          if (-1 < local_164.m_pStudioHdrCache.m_Memory.m_nGrowSize) {
            if (local_164.m_pStudioHdrCache.m_Memory.m_pMemory != (studiohdr_t **)0x0) {
              (**(code **)(*(int *)**(undefined4 **)
                                     ("Unable to restore binary scene \'%s\'\n" + unaff_EBX + 0x1b)
                          + 8))((int *)**(undefined4 **)
                                         ("Unable to restore binary scene \'%s\'\n" +
                                         unaff_EBX + 0x1b),
                                local_164.m_pStudioHdrCache.m_Memory.m_pMemory);
              local_164.m_pStudioHdrCache.m_Memory.m_pMemory = (studiohdr_t **)0x0;
            }
            local_164.m_pStudioHdrCache.m_Memory.m_nAllocationCount = 0;
          }
        }
        local_170 = local_170 + 1;
      } while (local_170 < pDVar2[damageEffectIndex].gibCount);
    }
  }
  return;
}


/* __static_initialization_and_destruction_0 at 00015de0 */

/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  int unaff_EBX;
  longlong lVar1;
  
  lVar1 = ___i686_get_pc_thunk_bx();
  if (lVar1 != 0xffff00000001) {
    return;
  }
  (&DAT_00c2093f)[unaff_EBX] = 0;
  (&DAT_00c20940)[unaff_EBX] = 0;
  (&DAT_00c20941)[unaff_EBX] = 0;
  (&DAT_00c20942)[unaff_EBX] = 0;
  *(undefined4 *)(&DAT_00c20943 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c20947 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c2094b + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c2094f + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c20953 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c20957 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c2095b + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c2095f + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c20963 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c20967 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c2096b + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c2096f + unaff_EBX) = 0x7f7fffff;
  *(undefined **)((int)&g_CheckButtonLinkerHack + unaff_EBX + 3) = &UNK_00b1c01f + unaff_EBX;
  return;
}


/* _GLOBAL__I__Z13EmitParticlesP15C_BaseAnimating17DamagedEntityTypeii at 00015e80 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__Z13EmitParticlesP15C_BaseAnimating17DamagedEntityTypeii(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

