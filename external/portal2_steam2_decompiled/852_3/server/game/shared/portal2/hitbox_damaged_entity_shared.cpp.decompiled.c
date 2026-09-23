/* DWARF-guided pseudocode for game/shared/portal2/hitbox_damaged_entity_shared.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* EmitParticles at 002446b0 */

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
                     (*(DamageDatabase **)(unaff_EBX + 0x967848),entityType);
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
                   **(Vector **)(&DAT_009674ec + unaff_EBX),vecStart,
                   **(QAngle **)(unaff_EBX + 0x967580),&pOwnerEntity->super_CBaseEntity,-1);
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


/* EmitGibs at 00244860 */

/* WARNING: Type propagation algorithm not settling */

void EmitGibs(CBaseAnimating *pParentEntity,DamagedEntityType entityType,int damageEffectIndex)

{
  undefined1 *puVar1;
  uint *puVar2;
  CStudioHdr *szGibModel;
  float fVar3;
  DamageInfo *pDVar4;
  IPhysicsObject *pIVar5;
  CBaseEdict *this;
  DamageInfoVector *pDVar6;
  int iVar7;
  studiohdr_t *pStudioHdr;
  CBaseEntity *pCVar8;
  IChangeInfoAccessor *pIVar9;
  float *pfVar10;
  int unaff_EBX;
  CGib *this_00;
  CBaseAnimating *vecImpulse;
  CStudioHdr *this_1;
  matrix3x4_t *local_17c;
  DamageInfo *damageInfo;
  int local_174;
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
       (pDVar6 = DamageDatabase::GetDamageInfoVector
                           (*(DamageDatabase **)(unaff_EBX + 0x96769b),entityType),
       -1 < damageEffectIndex)) && (damageEffectIndex < pDVar6->m_Size)) &&
     (pDVar4 = (pDVar6->m_Memory).m_pMemory, 0 < pDVar4[damageEffectIndex].gibCount)) {
    pfVar10 = *(float **)(unaff_EBX + 0x96733f);
    local_28[0].x = *pfVar10;
    local_28[0].y = pfVar10[1];
    local_28[0].z = pfVar10[2];
    iVar7 = CBaseAnimating::LookupAttachment
                      (pParentEntity,(char *)(CTriggerVPhysicsMotion::Simulate + unaff_EBX + 3));
    if (iVar7 < 1) {
      SetIdentityMatrix(&local_d0);
    }
    else {
      CBaseAnimating::GetAttachmentLocal(pParentEntity,iVar7,&local_d0);
      MatrixGetColumn(&local_d0,3,local_28);
    }
    local_17c = &local_d0;
    if ((((pParentEntity->super_CBaseEntity).m_iEFlags & 0x800) != 0) &&
       (CBaseEntity::CalcAbsolutePosition(&pParentEntity->super_CBaseEntity),
       ((pParentEntity->super_CBaseEntity).m_iEFlags & 0x800) != 0)) {
      CBaseEntity::CalcAbsolutePosition(&pParentEntity->super_CBaseEntity);
    }
    local_34.x = (pParentEntity->super_CBaseEntity).m_vecAbsOrigin.x;
    local_34.y = (pParentEntity->super_CBaseEntity).m_vecAbsOrigin.y;
    local_34.z = (pParentEntity->super_CBaseEntity).m_vecAbsOrigin.z;
    iVar7 = CBaseAnimating::LookupAttachment(pParentEntity,(char *)(unaff_EBX + 0x7794a3));
    if (0 < iVar7) {
                    /* Unresolved local var: matrix3x4_t burstCenterTransform@[???] */
      (*(pParentEntity->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.
        super_IHandleEntity._vptr_IHandleEntity[0xd9])(pParentEntity,iVar7,&local_100);
      MatrixGetColumn(&local_100,3,&local_34);
    }
    if (0 < pDVar4[damageEffectIndex].gibCount) {
      local_174 = 0;
      do {
        szGibModel = (CStudioHdr *)(pDVar4[damageEffectIndex].gibModels + local_174);
        this_1 = szGibModel;
        iVar7 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x967337) + 0xc))
                          ((int *)**(undefined4 **)(unaff_EBX + 0x967337));
        if (iVar7 < 1) {
          _Warning(unaff_EBX + 0x7794af,szGibModel);
        }
        else {
          CStudioHdr::CStudioHdr(&local_164,this_1);
          iVar7 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x967337) + 8))
                            ((int *)**(undefined4 **)(unaff_EBX + 0x967337),iVar7);
          if (iVar7 != 0) {
            pStudioHdr = (studiohdr_t *)
                         (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x967337) + 0x78))
                                   ((int *)**(undefined4 **)(unaff_EBX + 0x967337),iVar7);
            CStudioHdr::Init(&local_164,pStudioHdr,(IMDLCache *)0x0);
          }
          pfVar10 = *(float **)(unaff_EBX + 0x96733f);
          local_40 = (ServerClass *)*pfVar10;
          local_3c.m_Index = (ulong)pfVar10[1];
          local_38 = (CTimedEventMgr *)pfVar10[2];
          local_4c.x = local_28[0].x;
          local_4c.y = local_28[0].y;
          local_4c.z = local_28[0].z;
          if (((pParentEntity->super_CBaseEntity).m_iEFlags & 0x800) != 0) {
            CBaseEntity::CalcAbsolutePosition(&pParentEntity->super_CBaseEntity);
          }
          iVar7 = Studio_FindAttachment
                            (&local_164,(char *)(CTriggerVPhysicsMotion::Simulate + unaff_EBX + 3));
          if (iVar7 != -1 && -1 < iVar7 + 1) {
            GetAttachmentLocalSpace(&local_164,iVar7,local_17c);
            MatrixGetColumn(local_17c,3,&local_4c);
            local_4c.x = local_4c.x - local_28[0].x;
            local_4c.y = local_4c.y - local_28[0].y;
            local_4c.z = local_4c.z - local_28[0].z;
          }
          puVar2 = (uint *)(unaff_EBX + 0x7e71bf);
          local_8c = (float)((uint)local_4c.z ^ *puVar2);
          local_90 = (float)((uint)local_4c.y ^ *puVar2);
          local_94 = (float)((uint)local_4c.x ^ *puVar2);
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
          pIVar5 = (pParentEntity->super_CBaseEntity).m_pPhysicsObject;
          if (pIVar5 != (IPhysicsObject *)0x0) {
            (*pIVar5->_vptr_IPhysicsObject[0x37])(pIVar5,&local_40,&local_64);
          }
          pCVar8 = CreateEntityByName((char *)(unaff_EBX + 0x7a2ab3),-1,true);
          this_00 = (CGib *)0x0;
          if ((pCVar8 == (CBaseEntity *)0x0) ||
             (this_00 = (CGib *)___dynamic_cast(pCVar8,*(undefined4 *)(&DAT_00967357 + unaff_EBX),
                                                *(undefined4 *)(unaff_EBX + 0x967697),0),
             this_00 == (CGib *)0x0)) {
            _Warning(unaff_EBX + 0x76e35f,(char *)(unaff_EBX + 0x7a2ab3));
          }
          CGib::Spawn(this_00,(char *)szGibModel,25.0);
          if ((this_00->super_CBaseAnimating).m_nBody.m_Value != local_174) {
            if ((this_00->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.
                m_bRegistered == false) {
              this = &((this_00->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                      super_CBaseEdict;
              if (this != (CBaseEdict *)0x0) {
                this->m_fStateFlags = this->m_fStateFlags | 0x101;
                pIVar9 = CBaseEdict::GetChangeAccessor(this);
                pIVar9->m_iChangeInfoSerialNumber = 0;
              }
            }
            else {
              puVar1 = &(this_00->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
              *(uint *)puVar1 = *(uint *)puVar1 | 1;
            }
            (this_00->super_CBaseAnimating).m_nBody.m_Value = local_174;
          }
          CGib::InitGib(this_00,&pParentEntity->super_CBaseEntity,0.0,0.0);
          this_00->m_lifeTime = pDVar4[damageEffectIndex].gibLifeTime;
          vecImpulse = pParentEntity;
          (*(this_00->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.
            super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x13])(this_00);
          if (((pParentEntity->super_CBaseEntity).m_fEffects.m_Value & 0x10) != 0) {
            vecImpulse = (CBaseAnimating *)&vgui::Panel::GetMessageMap::s_pMap.ncmds;
            CBaseEntity::AddEffects((CBaseEntity *)this_00,0x10);
          }
          if (pDVar4[damageEffectIndex].gibSeparationSpeed != 0.0) {
                    /* Unresolved local var: Vector res@[???] */
            local_100.m_flMatVal[0][0] = (float)local_40 - local_34.x;
            local_100.m_flMatVal[0][1] = (float)local_3c.m_Index - local_34.y;
            local_100.m_flMatVal[0][2] = (float)local_38 - local_34.z;
            pfVar10 = *(float **)(unaff_EBX + 0x96733f);
            local_70._vptr_IServerNetworkable = (_func_int_varargs **)local_100.m_flMatVal[0][0];
            local_6c._vptr_IEventRegisterCallback = (_func_int_varargs **)local_100.m_flMatVal[0][1]
            ;
            local_68 = (CBaseEntity *)local_100.m_flMatVal[0][2];
            if ((*pfVar10 == local_100.m_flMatVal[0][0]) &&
               (!NAN(*pfVar10) && !NAN(local_100.m_flMatVal[0][0]))) {
              if ((pfVar10[1] == local_100.m_flMatVal[0][1]) &&
                 (!NAN(pfVar10[1]) && !NAN(local_100.m_flMatVal[0][1]))) {
                if ((pfVar10[2] == local_100.m_flMatVal[0][2]) &&
                   (!NAN(pfVar10[2]) && !NAN(local_100.m_flMatVal[0][2]))) {
                  pfVar10 = (float *)(*(this_00->super_CBaseAnimating).super_CBaseEntity.
                                       super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
                                       _vptr_IHandleEntity[0x9d])(this_00);
                    /* Unresolved local var: Vector res@[???] */
                  local_100.m_flMatVal[0][0] = *pfVar10 - local_34.x;
                  local_100.m_flMatVal[0][1] = pfVar10[1] - local_34.y;
                  local_100.m_flMatVal[0][2] = pfVar10[2] - local_34.z;
                  local_7c = (_func_int_varargs **)local_100.m_flMatVal[0][0];
                  local_78 = (_func_int_varargs **)local_100.m_flMatVal[0][1];
                  local_74 = (CBaseEntity *)local_100.m_flMatVal[0][2];
                }
              }
            }
            VectorNormalize((Vector *)&local_100);
            fVar3 = pDVar4[damageEffectIndex].gibSeparationSpeed;
                    /* Unresolved local var: Vector res@[???] */
            local_a0._vptr_IHandleEntity =
                 (_func_int_varargs **)(fVar3 * local_100.m_flMatVal[0][0]);
            local_9c = fVar3 * local_100.m_flMatVal[0][1];
            local_98 = (_func___thiscall_void_CBaseEntity_ptr *)(fVar3 * local_100.m_flMatVal[0][2])
            ;
            vecImpulse = (CBaseAnimating *)&local_88;
            local_88._vptr_IHandleEntity = local_a0._vptr_IHandleEntity;
            local_84.m_Value = (int)local_9c;
            local_80 = local_98;
            CBaseEntity::ApplyAbsVelocityImpulse((CBaseEntity *)this_00,(Vector *)vecImpulse);
          }
          CStudioHdr::~CStudioHdr(&local_164,(int)vecImpulse);
        }
        local_174 = local_174 + 1;
      } while (local_174 < pDVar4[damageEffectIndex].gibCount);
    }
  }
  return;
}


/* __static_initialization_and_destruction_0 at 0001e5e0 */

/* WARNING: Type propagation algorithm not settling */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  int unaff_EBX;
  longlong lVar1;
  
  lVar1 = ___i686_get_pc_thunk_bx();
  if (lVar1 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x349].m_AddLock.m_lockInfo + unaff_EBX + 7) = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x349].m_AddLock.m_nWriters.m_value + unaff_EBX) = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x349].m_AddLock.m_nWriters.m_value + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x349].m_AddLock.m_nWriters.m_value + unaff_EBX + 2) = 0;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x349].m_AddLock.m_nWriters.m_value + unaff_EBX + 3) = 0;
  *(undefined4 *)
   (&CDmxElement::s_TypeSymbols.m_Lookup.
     super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
     .m_aBuckets[0x349].m_AddLock.field_0xf + unaff_EBX) = 0;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34a].m_pFirst + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34a].m_pFirstUncommitted + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34a].m_AddLock.m_lockInfo + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34a].m_AddLock.m_lockInfo + unaff_EBX + 7) = 0;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34a].m_AddLock.m_nWriters.m_value + unaff_EBX + 3) = 0;
  *(undefined4 *)
   (&CDmxElement::s_TypeSymbols.m_Lookup.
     super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
     .m_aBuckets[0x34a].m_AddLock.field_0xf + unaff_EBX) = 0;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34b].m_pFirst + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34b].m_pFirstUncommitted + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34b].m_AddLock.m_lockInfo + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34b].m_AddLock.m_lockInfo + unaff_EBX + 7) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34b].m_AddLock.m_nWriters.m_value + unaff_EBX + 3) = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34c].m_pFirst + unaff_EBX + 2) = 0;
  (&CDmxElement::s_TypeSymbols.m_Lookup.
    super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
    .m_aBuckets[0x34b].m_AddLock.field_0xf)[unaff_EBX] = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34c].m_pFirst + unaff_EBX) = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34c].m_pFirst + unaff_EBX + 1) = 0;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34c].m_pFirst + unaff_EBX + 3) = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34c].m_AddLock.m_lockInfo + unaff_EBX + 2) = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34c].m_pFirstUncommitted + unaff_EBX + 3) = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34c].m_AddLock.m_lockInfo + unaff_EBX) = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34c].m_AddLock.m_lockInfo + unaff_EBX + 1) = 0;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34c].m_AddLock.m_lockInfo + unaff_EBX + 3) = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34c].m_AddLock.m_nWriters.m_value + unaff_EBX + 2) = 1;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34c].m_AddLock.m_lockInfo + unaff_EBX + 7) = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34c].m_AddLock.m_nWriters.m_value + unaff_EBX) = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34c].m_AddLock.m_nWriters.m_value + unaff_EBX + 1) = 0;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34c].m_AddLock.m_nWriters.m_value + unaff_EBX + 3) = 1;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34d].m_pFirst + unaff_EBX + 2) = 0;
  (&CDmxElement::s_TypeSymbols.m_Lookup.
    super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
    .m_aBuckets[0x34c].m_AddLock.field_0xf)[unaff_EBX] = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34d].m_pFirst + unaff_EBX) = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34d].m_pFirst + unaff_EBX + 1) = 0;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34d].m_pFirst + unaff_EBX + 3) = 2;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34d].m_AddLock.m_lockInfo + unaff_EBX + 2) = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34d].m_pFirstUncommitted + unaff_EBX + 3) = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34d].m_AddLock.m_lockInfo + unaff_EBX) = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34d].m_AddLock.m_lockInfo + unaff_EBX + 1) = 0;
  *(undefined **)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x34d].m_AddLock.m_lockInfo + unaff_EBX + 3) = &UNK_00b92d3f + unaff_EBX;
  return;
}


/* _GLOBAL__I__Z13EmitParticlesP14CBaseAnimating17DamagedEntityTypeii at 0001e710 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__Z13EmitParticlesP14CBaseAnimating17DamagedEntityTypeii(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

