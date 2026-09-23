/* DWARF-guided pseudocode for game/client/portal2/c_prop_tractorbeam.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* _C_PropTractorBeamProjector_CreateObject at 0053aa50 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_PropTractorBeamProjector_CreateObject(int entnum,int serialNum)

{
  C_BaseAnimating *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_PropTractorBeamProjector * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xddc);
  C_BaseAnimating::C_BaseAnimating(this);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x69408a);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6944e6);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x69459e);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(&UNK_006945da + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6945f6);
  this[1].super_C_BaseEntity.m_VarMap.m_lastInterpolationTime = 0.0;
  this[1].super_C_BaseEntity.m_VarMap.m_nInterpolatedEntries = 0;
  this[1].super_C_BaseEntity.m_pfnThink.__pfn = (_func___thiscall_void_C_BaseEntity_ptr *)0x0;
  this[1].super_C_BaseEntity.m_pfnTouch.__pfn =
       (_func___thiscall_void_C_BaseEntity_ptr_C_BaseEntity_ptr *)0x0;
  this[1].super_C_BaseEntity.m_pfnThink.__delta = 0;
  this[1].super_C_BaseEntity.m_pfnTouch.__delta = 0;
  *(undefined4 *)&this[1].super_C_BaseEntity.m_EntClientFlags = 0;
  this[1].super_C_BaseEntity.index = 0;
  this[1].super_C_BaseEntity.model = (model_t *)0x0;
  (*(this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x2f])(this,entnum,serialNum);
  return &(this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable;
}


/* C_PropTractorBeamProjector::~C_PropTractorBeamProjector at 0053c6a0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_PropTractorBeamProjector(C_PropTractorBeamProjector * this, int
   __in_chrg) */

void __thiscall
C_PropTractorBeamProjector::~C_PropTractorBeamProjector
          (C_PropTractorBeamProjector *this,int __in_chrg)

{
  CNewParticleEffect *pCVar1;
  CUtlReference<CNewParticleEffect> *pCVar2;
  IHandleEntity IVar3;
  C_PropTractorBeamProjector *pCVar4;
  C_PropTractorBeamProjector *pCVar5;
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x69243b);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x692897);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x69294f);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x69298b);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(&UNK_006929a7 + unaff_EBX);
  pCVar4 = this + 1;
  while (pCVar5 = pCVar4, pCVar4 != (C_PropTractorBeamProjector *)this->m_hEmitterEffect) {
    while( true ) {
      pCVar4 = (C_PropTractorBeamProjector *)(pCVar5[-1].m_hEmitterEffect + 2);
      pCVar1 = pCVar5[-1].m_hEmitterEffect[2].m_pObject;
      if (pCVar1 == (CNewParticleEffect *)0x0) break;
      pCVar2 = pCVar5[-1].m_hEmitterEffect[2].m_pPrev;
      if (pCVar2 == (CUtlReference<CNewParticleEffect> *)0x0) {
        if (pCVar4 == (C_PropTractorBeamProjector *)
                      (pCVar1->m_References).
                      super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
                      super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead) {
          IVar3._vptr_IHandleEntity =
               (pCVar4->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.
               super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity;
          (pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead =
               (CUtlReference<CNewParticleEffect> *)IVar3._vptr_IHandleEntity;
          if ((CUtlReference<CNewParticleEffect> *)IVar3._vptr_IHandleEntity !=
              (CUtlReference<CNewParticleEffect> *)0x0) {
            IVar3._vptr_IHandleEntity[1] = (_func_int_varargs *)0x0;
          }
        }
      }
      else {
        pCVar2->m_pNext =
             (CUtlReference<CNewParticleEffect> *)
             (pCVar4->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.
             super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity;
        IVar3._vptr_IHandleEntity =
             (pCVar4->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.
             super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity;
        if ((CUtlReference<CNewParticleEffect> *)IVar3._vptr_IHandleEntity !=
            (CUtlReference<CNewParticleEffect> *)0x0) {
          IVar3._vptr_IHandleEntity[1] = (_func_int_varargs *)pCVar2;
        }
      }
      pCVar5[-1].m_hEmitterEffect[2].m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
      (pCVar4->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
      super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)0x0;
      pCVar5[-1].m_hEmitterEffect[2].m_pObject = (CNewParticleEffect *)0x0;
      pCVar5 = pCVar4;
      if (pCVar4 == (C_PropTractorBeamProjector *)this->m_hEmitterEffect) goto LAB_0053c746;
    }
  }
LAB_0053c746:
  C_BaseAnimating::~C_BaseAnimating(&this->super_C_BaseAnimating,in_stack_ffffffe8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_PropTractorBeamProjector::~C_PropTractorBeamProjector at 0053c780 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_PropTractorBeamProjector(C_PropTractorBeamProjector * this, int
   __in_chrg) */

void __thiscall
C_PropTractorBeamProjector::~C_PropTractorBeamProjector
          (C_PropTractorBeamProjector *this,int __in_chrg)

{
  CNewParticleEffect *pCVar1;
  CUtlReference<CNewParticleEffect> *pCVar2;
  IHandleEntity IVar3;
  C_PropTractorBeamProjector *pCVar4;
  C_PropTractorBeamProjector *pCVar5;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x69235e);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x6927ba);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(CRefractiveGlassView::Draw + unaff_EBX + 2);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x6928ae);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(unaff_EBX + 0x6928ca);
  pCVar4 = this + 1;
  while (pCVar5 = pCVar4, pCVar4 != (C_PropTractorBeamProjector *)this->m_hEmitterEffect) {
    while( true ) {
      pCVar4 = (C_PropTractorBeamProjector *)(pCVar5[-1].m_hEmitterEffect + 2);
      pCVar1 = pCVar5[-1].m_hEmitterEffect[2].m_pObject;
      if (pCVar1 == (CNewParticleEffect *)0x0) break;
      pCVar2 = pCVar5[-1].m_hEmitterEffect[2].m_pPrev;
      if (pCVar2 == (CUtlReference<CNewParticleEffect> *)0x0) {
        if (pCVar4 == (C_PropTractorBeamProjector *)
                      (pCVar1->m_References).
                      super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
                      super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead) {
          IVar3._vptr_IHandleEntity =
               (pCVar4->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.
               super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity;
          (pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead =
               (CUtlReference<CNewParticleEffect> *)IVar3._vptr_IHandleEntity;
          if ((CUtlReference<CNewParticleEffect> *)IVar3._vptr_IHandleEntity !=
              (CUtlReference<CNewParticleEffect> *)0x0) {
            IVar3._vptr_IHandleEntity[1] = (_func_int_varargs *)0x0;
          }
        }
      }
      else {
        pCVar2->m_pNext =
             (CUtlReference<CNewParticleEffect> *)
             (pCVar4->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.
             super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity;
        IVar3._vptr_IHandleEntity =
             (pCVar4->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.
             super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity;
        if ((CUtlReference<CNewParticleEffect> *)IVar3._vptr_IHandleEntity !=
            (CUtlReference<CNewParticleEffect> *)0x0) {
          IVar3._vptr_IHandleEntity[1] = (_func_int_varargs *)pCVar2;
        }
      }
      pCVar5[-1].m_hEmitterEffect[2].m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
      (pCVar4->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
      super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)0x0;
      pCVar5[-1].m_hEmitterEffect[2].m_pObject = (CNewParticleEffect *)0x0;
      pCVar5 = pCVar4;
      if (pCVar4 == (C_PropTractorBeamProjector *)this->m_hEmitterEffect) goto LAB_0053c816;
    }
  }
LAB_0053c816:
  C_BaseAnimating::~C_BaseAnimating(&this->super_C_BaseAnimating,__in_chrg);
  return;
}


/* C_PropTractorBeamProjector::RecvProxy_LinearForce at 0053b1a0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

void C_PropTractorBeamProjector::RecvProxy_LinearForce
               (CRecvProxyData *pData,void *pStruct,void *pOut)

{
  float fVar1;
  int *piVar2;
  int unaff_EBX;
  uint uVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  
                    /* Unresolved local var: C_PropTractorBeamProjector * pBeam@[???]
                       Unresolved local var: float flLinearForce@[???] */
  ___i686_get_pc_thunk_bx();
  fVar5 = (pData->m_Value).field_0.m_Float;
  if (fVar5 == *(float *)((int)pStruct + 0xd84)) {
    return;
  }
  *(float *)((int)pStruct + 0xd84) = fVar5;
  piVar2 = *(int **)(unaff_EBX + 0x628bf3);
  fVar7 = *(float *)(*piVar2 + 0xc);
  fVar4 = *(float *)((int)pStruct + 0xdb4);
  fVar6 = fVar4 + *(float *)((int)pStruct + 0xdb0);
  if (fVar6 < fVar7) {
    fVar7 = *(float *)(unaff_EBX + 0x4a741b);
    fVar4 = *(float *)((int)pStruct + 0xdac);
  }
  else {
    fVar1 = *(float *)((int)pStruct + 0xda8);
    if ((fVar4 != fVar6) || (NAN(fVar4) || NAN(fVar6))) {
      fVar7 = (fVar7 - fVar4) / (fVar6 - fVar4);
                    /* Unresolved local var: float valueSquared@[???] */
      fVar8 = (*(float *)((int)pStruct + 0xdac) - fVar1) *
              (fVar7 * fVar7 * *(float *)(unaff_EBX + 0x4a73a7) +
              fVar7 * fVar7 * fVar7 * *(float *)(unaff_EBX + 0x4a7443)) + fVar1;
    }
    else {
      fVar8 = *(float *)((int)pStruct + 0xdac);
      if (fVar7 < fVar6) {
        fVar8 = fVar1;
      }
    }
    fVar7 = *(float *)(unaff_EBX + 0x4a741b);
    fVar5 = fVar5 / fVar7;
    fVar4 = fVar8;
    if (fVar5 != 0.0) {
      fVar4 = fVar5;
      if (fVar5 < 0.0) {
        if (fVar8 < fVar5) goto LAB_0053b208;
      }
      else if (fVar5 < fVar8) goto LAB_0053b208;
      fVar4 = fVar8;
    }
  }
LAB_0053b208:
  *(float *)((int)pStruct + 0xda8) = fVar4;
  *(undefined4 *)((int)pStruct + 0xdb4) = *(undefined4 *)(*piVar2 + 0xc);
  *(undefined4 *)((int)pStruct + 0xdb0) = 0x3e800000;
  *(float *)((int)pStruct + 0xdac) = *(float *)((int)pStruct + 0xd84) / fVar7;
  fVar5 = *(float *)(*piVar2 + 0xc);
  fVar7 = *(float *)((int)pStruct + 0xda4);
  fVar6 = fVar7 + *(float *)((int)pStruct + 0xda0);
  fVar4 = 0.0;
  if (fVar5 <= fVar6) {
    fVar1 = *(float *)((int)pStruct + 0xd98);
    if ((fVar7 != fVar6) || (NAN(fVar7) || NAN(fVar6))) {
      fVar5 = (fVar5 - fVar7) / (fVar6 - fVar7);
                    /* Unresolved local var: float valueSquared@[???] */
      fVar7 = (*(float *)((int)pStruct + 0xd9c) - fVar1) *
              (fVar5 * fVar5 * *(float *)(unaff_EBX + 0x4a73a7) +
              fVar5 * fVar5 * fVar5 * *(float *)(unaff_EBX + 0x4a7443)) + fVar1;
    }
    else {
      fVar7 = *(float *)((int)pStruct + 0xd9c);
      if (fVar5 < fVar6) {
        fVar7 = fVar1;
      }
    }
    fVar5 = fVar4;
    if (0.0 <= fVar7) {
      fVar5 = fVar7;
    }
    uVar3 = -(uint)(fVar7 <= *(float *)(unaff_EBX + 0x4a6fdb));
    uVar3 = ~uVar3 & (uint)*(float *)(unaff_EBX + 0x4a6fdb) | (uint)fVar5 & uVar3;
  }
  else {
    uVar3 = *(uint *)((int)pStruct + 0xd9c);
  }
  *(uint *)((int)pStruct + 0xd98) = uVar3;
  if (0.0 < *(float *)((int)pStruct + 0xd84)) {
    fVar4 = *(float *)(unaff_EBX + 0x4a6fdb);
  }
  *(float *)((int)pStruct + 0xd9c) = fVar4;
  *(undefined4 *)((int)pStruct + 0xda0) = 0x3f400000;
  *(undefined4 *)((int)pStruct + 0xda4) = *(undefined4 *)(*piVar2 + 0xc);
  UpdateEffect(pStruct);
  return;
}


/* C_PropTractorBeamProjector::RecvProxy_Enabled at 0053bfd0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

void C_PropTractorBeamProjector::RecvProxy_Enabled(CRecvProxyData *pData,void *pStruct,void *pOut)

{
  float fVar1;
  int *piVar2;
  int iVar3;
  undefined4 *puVar4;
  int iVar5;
  int unaff_EBX;
  bool bVar6;
  uint uVar7;
  float fVar8;
  float fVar9;
  float fVar10;
  float fVar11;
  float fVar12;
  
                    /* Unresolved local var: C_PropTractorBeamProjector * pBeam@[???]
                       Unresolved local var: bool bEnabled@[???] */
  ___i686_get_pc_thunk_bx();
  bVar6 = (pData->m_Value).field_0.m_Int == 1;
  if ((bool)*(char *)((int)pStruct + 0xd96) == bVar6) {
    return;
  }
  *(bool *)((int)pStruct + 0xd96) = bVar6;
  if (!bVar6) {
    piVar2 = *(int **)(unaff_EBX + 0x627dbf);
    fVar9 = *(float *)(*piVar2 + 0xc);
    fVar12 = *(float *)((int)pStruct + 0xdb4);
    fVar10 = fVar12 + *(float *)((int)pStruct + 0xdb0);
    if (fVar10 < fVar9) {
      fVar9 = *(float *)((int)pStruct + 0xdac);
    }
    else {
      fVar8 = *(float *)((int)pStruct + 0xda8);
      if ((fVar12 != fVar10) || (NAN(fVar12) || NAN(fVar10))) {
        fVar9 = (fVar9 - fVar12) / (fVar10 - fVar12);
                    /* Unresolved local var: float valueSquared@[???] */
        fVar12 = (*(float *)((int)pStruct + 0xdac) - fVar8) *
                 (fVar9 * fVar9 * *(float *)(unaff_EBX + 0x4a6573) +
                 fVar9 * fVar9 * fVar9 * *(float *)(unaff_EBX + 0x4a660f)) + fVar8;
      }
      else {
        fVar12 = *(float *)((int)pStruct + 0xdac);
        if (fVar9 < fVar10) {
          fVar12 = fVar8;
        }
      }
      fVar10 = *(float *)((int)pStruct + 0xd84) / *(float *)(unaff_EBX + 0x4a65e7);
      fVar9 = fVar12;
      if (fVar10 != 0.0) {
        fVar9 = fVar10;
        if (fVar10 < 0.0) {
          if (fVar12 < fVar10) goto LAB_0053c135;
        }
        else if (fVar10 < fVar12) goto LAB_0053c135;
        fVar9 = fVar12;
      }
    }
LAB_0053c135:
    *(float *)((int)pStruct + 0xda8) = fVar9;
    *(undefined4 *)((int)pStruct + 0xdb4) = *(undefined4 *)(*piVar2 + 0xc);
    *(undefined4 *)((int)pStruct + 0xdb0) = 0x3fc00000;
    *(undefined4 *)((int)pStruct + 0xdac) = 0;
    fVar9 = *(float *)(*piVar2 + 0xc);
    fVar12 = *(float *)((int)pStruct + 0xda4);
    fVar10 = fVar12 + *(float *)((int)pStruct + 0xda0);
    if (fVar9 <= fVar10) {
      fVar8 = *(float *)((int)pStruct + 0xd98);
      if ((fVar12 != fVar10) || (NAN(fVar12) || NAN(fVar10))) {
        fVar9 = (fVar9 - fVar12) / (fVar10 - fVar12);
                    /* Unresolved local var: float valueSquared@[???] */
        fVar12 = (*(float *)((int)pStruct + 0xd9c) - fVar8) *
                 (fVar9 * fVar9 * *(float *)(unaff_EBX + 0x4a6573) +
                 fVar9 * fVar9 * fVar9 * *(float *)(unaff_EBX + 0x4a660f)) + fVar8;
      }
      else {
        fVar12 = *(float *)((int)pStruct + 0xd9c);
        if (fVar9 < fVar10) {
          fVar12 = fVar8;
        }
      }
      fVar9 = 0.0;
      if (0.0 <= fVar12) {
        fVar9 = fVar12;
      }
      uVar7 = -(uint)(fVar12 <= *(float *)(unaff_EBX + 0x4a61a7));
      uVar7 = ~uVar7 & (uint)*(float *)(unaff_EBX + 0x4a61a7) | (uint)fVar9 & uVar7;
    }
    else {
      uVar7 = *(uint *)((int)pStruct + 0xd9c);
    }
    *(uint *)((int)pStruct + 0xd98) = uVar7;
    *(undefined4 *)((int)pStruct + 0xd9c) = 0x3f000000;
    *(undefined4 *)((int)pStruct + 0xda0) = 0x3fc00000;
    *(undefined4 *)((int)pStruct + 0xda4) = *(undefined4 *)(*piVar2 + 0xc);
    if (*(CNewParticleEffect **)((int)pStruct + 0xdc0) != (CNewParticleEffect *)0x0) {
      CParticleProperty::StopEmission
                ((CParticleProperty *)((int)pStruct + 0x474),
                 *(CNewParticleEffect **)((int)pStruct + 0xdc0),false,false,false,true);
      iVar3 = *(int *)((int)pStruct + 0xdc0);
      if (iVar3 != 0) {
        puVar4 = *(undefined4 **)((int)pStruct + 0xdbc);
        if (puVar4 == (undefined4 *)0x0) {
          if (*(int *)(iVar3 + 0x3a4) == (int)pStruct + 0xdb8) {
            iVar5 = *(int *)((int)pStruct + 0xdb8);
            *(int *)(iVar3 + 0x3a4) = iVar5;
            if (iVar5 != 0) {
              *(undefined4 *)(iVar5 + 4) = 0;
            }
          }
        }
        else {
          *puVar4 = *(undefined4 *)((int)pStruct + 0xdb8);
          if (*(int *)((int)pStruct + 0xdb8) != 0) {
            *(undefined4 **)(*(int *)((int)pStruct + 0xdb8) + 4) = puVar4;
          }
        }
        *(undefined4 *)((int)pStruct + 0xdbc) = 0;
        *(undefined4 *)((int)pStruct + 0xdb8) = 0;
        *(undefined4 *)((int)pStruct + 0xdc0) = 0;
      }
    }
    if (*(CNewParticleEffect **)((int)pStruct + 0xdcc) != (CNewParticleEffect *)0x0) {
      CParticleProperty::StopEmission
                ((CParticleProperty *)((int)pStruct + 0x474),
                 *(CNewParticleEffect **)((int)pStruct + 0xdcc),false,false,false,true);
      iVar3 = *(int *)((int)pStruct + 0xdcc);
      if (iVar3 != 0) {
        puVar4 = *(undefined4 **)((int)pStruct + 0xdc8);
        if (puVar4 == (undefined4 *)0x0) {
          if (*(int *)(iVar3 + 0x3a4) == (int)pStruct + 0xdc4) {
            iVar5 = *(int *)((int)pStruct + 0xdc4);
            *(int *)(iVar3 + 0x3a4) = iVar5;
            if (iVar5 != 0) {
              *(undefined4 *)(iVar5 + 4) = 0;
            }
          }
        }
        else {
          *puVar4 = *(undefined4 *)((int)pStruct + 0xdc4);
          if (*(int *)((int)pStruct + 0xdc4) != 0) {
            *(undefined4 **)(*(int *)((int)pStruct + 0xdc4) + 4) = puVar4;
          }
        }
        *(undefined4 *)((int)pStruct + 0xdc8) = 0;
        *(undefined4 *)((int)pStruct + 0xdc4) = 0;
        *(undefined4 *)((int)pStruct + 0xdcc) = 0;
      }
    }
    if (*(CNewParticleEffect **)((int)pStruct + 0xdd8) != (CNewParticleEffect *)0x0) {
      CParticleProperty::StopEmission
                ((CParticleProperty *)((int)pStruct + 0x474),
                 *(CNewParticleEffect **)((int)pStruct + 0xdd8),false,false,false,true);
      iVar3 = *(int *)((int)pStruct + 0xdd8);
      if (iVar3 != 0) {
        puVar4 = *(undefined4 **)((int)pStruct + 0xdd4);
        if (puVar4 == (undefined4 *)0x0) {
          if (*(int *)(iVar3 + 0x3a4) == (int)pStruct + 0xdd0) {
            iVar5 = *(int *)((int)pStruct + 0xdd0);
            *(int *)(iVar3 + 0x3a4) = iVar5;
            if (iVar5 != 0) {
              *(undefined4 *)(iVar5 + 4) = 0;
            }
          }
        }
        else {
          *puVar4 = *(undefined4 *)((int)pStruct + 0xdd0);
          if (*(int *)((int)pStruct + 0xdd0) != 0) {
            *(undefined4 **)(*(int *)((int)pStruct + 0xdd0) + 4) = puVar4;
          }
        }
        *(undefined4 *)((int)pStruct + 0xdd4) = 0;
        *(undefined4 *)((int)pStruct + 0xdd0) = 0;
        *(undefined4 *)((int)pStruct + 0xdd8) = 0;
      }
    }
    *(undefined1 *)((int)pStruct + 0xd94) = 0;
    return;
  }
  piVar2 = *(int **)(unaff_EBX + 0x627dbf);
  fVar9 = *(float *)(*piVar2 + 0xc);
  fVar12 = *(float *)((int)pStruct + 0xdb4);
  fVar10 = fVar12 + *(float *)((int)pStruct + 0xdb0);
  if (fVar9 <= fVar10) {
    fVar8 = *(float *)((int)pStruct + 0xda8);
    if ((fVar12 != fVar10) || (NAN(fVar12) || NAN(fVar10))) {
      fVar9 = (fVar9 - fVar12) / (fVar10 - fVar12);
                    /* Unresolved local var: float valueSquared@[???] */
      fVar12 = (*(float *)((int)pStruct + 0xdac) - fVar8) *
               (fVar9 * fVar9 * *(float *)(unaff_EBX + 0x4a6573) +
               fVar9 * fVar9 * fVar9 * *(float *)(unaff_EBX + 0x4a660f)) + fVar8;
    }
    else {
      fVar12 = *(float *)((int)pStruct + 0xdac);
      if (fVar9 < fVar10) {
        fVar12 = fVar8;
      }
    }
    fVar9 = *(float *)(unaff_EBX + 0x4a65e7);
    fVar10 = *(float *)((int)pStruct + 0xd84);
    fVar11 = fVar10 / fVar9;
    fVar8 = fVar12;
    if (fVar11 != 0.0) {
      fVar8 = fVar11;
      if (fVar11 < 0.0) {
        if (fVar12 < fVar11) goto LAB_0053c049;
      }
      else if (fVar11 < fVar12) goto LAB_0053c049;
      fVar8 = fVar12;
    }
  }
  else {
    fVar9 = *(float *)(unaff_EBX + 0x4a65e7);
    fVar10 = *(float *)((int)pStruct + 0xd84);
    fVar8 = *(float *)((int)pStruct + 0xdac);
  }
LAB_0053c049:
  *(float *)((int)pStruct + 0xda8) = fVar8;
  *(undefined4 *)((int)pStruct + 0xdb4) = *(undefined4 *)(*piVar2 + 0xc);
  *(undefined4 *)((int)pStruct + 0xdb0) = 0x3e800000;
  *(float *)((int)pStruct + 0xdac) = fVar10 / fVar9;
  fVar9 = *(float *)(*piVar2 + 0xc);
  fVar12 = *(float *)((int)pStruct + 0xda4);
  fVar11 = fVar12 + *(float *)((int)pStruct + 0xda0);
  fVar8 = 0.0;
  if (fVar9 <= fVar11) {
    fVar1 = *(float *)((int)pStruct + 0xd98);
    if ((fVar12 != fVar11) || (NAN(fVar12) || NAN(fVar11))) {
      fVar9 = (fVar9 - fVar12) / (fVar11 - fVar12);
                    /* Unresolved local var: float valueSquared@[???] */
      fVar12 = (*(float *)((int)pStruct + 0xd9c) - fVar1) *
               (fVar9 * fVar9 * *(float *)(unaff_EBX + 0x4a6573) +
               fVar9 * fVar9 * fVar9 * *(float *)(unaff_EBX + 0x4a660f)) + fVar1;
    }
    else {
      fVar12 = *(float *)((int)pStruct + 0xd9c);
      if (fVar9 < fVar11) {
        fVar12 = fVar1;
      }
    }
    fVar9 = fVar8;
    if (0.0 <= fVar12) {
      fVar9 = fVar12;
    }
    uVar7 = -(uint)(fVar12 <= *(float *)(unaff_EBX + 0x4a61a7));
    uVar7 = ~uVar7 & (uint)*(float *)(unaff_EBX + 0x4a61a7) | (uint)fVar9 & uVar7;
  }
  else {
    uVar7 = *(uint *)((int)pStruct + 0xd9c);
  }
  *(uint *)((int)pStruct + 0xd98) = uVar7;
  if (0.0 < fVar10) {
    fVar8 = *(float *)(unaff_EBX + 0x4a61a7);
  }
  *(float *)((int)pStruct + 0xd9c) = fVar8;
  *(undefined4 *)((int)pStruct + 0xda0) = 0x3f400000;
  *(undefined4 *)((int)pStruct + 0xda4) = *(undefined4 *)(*piVar2 + 0xc);
  if ((*(char *)((int)pStruct + 0xd94) == '\0') && (*(char *)((int)pStruct + 0xd95) != '\0')) {
    CreateEffect(pStruct);
    return;
  }
  return;
}


/* ClientClassInit<DT_PropTractorBeamProjector::ignored> at 00079330 */

int ClientClassInit<DT_PropTractorBeamProjector::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)NPaintRenderer::g_FastSpherePosData[0x23] + unaff_EBX + 0x14) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc59b94);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)NPaintRenderer::g_FastSpherePosData[0x23] + unaff_EBX + 0x1c),
                  &UNK_0091142c + unaff_EBX,0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)NPaintRenderer::g_FastSpherePosData[0x25] + unaff_EBX + 0x18),
                        &UNK_00915f70 + unaff_EBX,0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00aeaab0 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00aeaaa8 + unaff_EBX));
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)NPaintRenderer::g_FastSpherePosData[0x27] + unaff_EBX + 0x14),
                    (char *)(unaff_EBX + 0x942179),0xd84,4,0,(RecvVarProxyFn)(unaff_EBX + 0x4c1e5c))
      ;
      RecvPropVector((RecvProp_conflict1 *)
                     ((int)NPaintRenderer::g_FastSpherePosData[0x29] + unaff_EBX + 0x10),
                     (char *)(unaff_EBX + 0x942189),0xd88,0xc,0,
                     *(RecvVarProxyFn *)(&DAT_00aeab4c + unaff_EBX));
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)NPaintRenderer::g_FastSpherePosData[0x2b] + unaff_EBX + 0xc),
                  (char *)(unaff_EBX + 0x929a90),0xd96,1,0,(RecvVarProxyFn)(unaff_EBX + 0x4c2c8c));
      ___cxa_guard_release(unaff_EBX + 0xc59b94);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_SIMDRandContexts[0x1d].m_RandY[3] + unaff_EBX + 0xc),
             (RecvProp_conflict1 *)
             ((int)NPaintRenderer::g_FastSpherePosData[0x25] + unaff_EBX + 0x18),4,
             (char *)(unaff_EBX + 0x942193));
  return 1;
}


/* __static_initialization_and_destruction_0 at 00079500 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  RecvTable *this;
  int *piVar1;
  int iVar2;
  int unaff_EBX;
  longlong lVar3;
  
  lVar3 = ___i686_get_pc_thunk_bx();
  if (lVar3 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)NPaintRenderer::g_FastSpherePosData[0x13] + unaff_EBX + 0xf) = 0;
  *(undefined1 *)((int)NPaintRenderer::g_FastSpherePosData[0x13] + unaff_EBX + 0x10) = 0;
  *(undefined1 *)((int)NPaintRenderer::g_FastSpherePosData[0x13] + unaff_EBX + 0x11) = 0;
  *(undefined1 *)((int)NPaintRenderer::g_FastSpherePosData[0x13] + unaff_EBX + 0x12) = 0;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0x13] + unaff_EBX + 0x13) = 0;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0x13] + unaff_EBX + 0x17) = 0;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0x13] + unaff_EBX + 0x1b) = 0x7f7fffff;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0x13] + unaff_EBX + 0x1f) = 0x7f7fffff;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0x14] + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0x14] + unaff_EBX + 7) = 0;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0x14] + unaff_EBX + 0xb) = 0;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0x14] + unaff_EBX + 0xf) = 0;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0x14] + unaff_EBX + 0x13) = 0x7f7fffff;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0x14] + unaff_EBX + 0x17) = 0x7f7fffff;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0x14] + unaff_EBX + 0x1b) = 0x7f7fffff;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0x14] + unaff_EBX + 0x1f) = 0x7f7fffff;
  *(undefined **)((int)NPaintRenderer::g_FastSpherePosData[0x15] + unaff_EBX + 3) =
       &UNK_00aeb977 + unaff_EBX;
  *(int *)((int)s_SIMDRandContexts[0x1c].m_RandY[0x1d] + unaff_EBX + 0xf) = unaff_EBX + 0x941fe2;
  *(int *)((int)s_SIMDRandContexts[0x1c].m_RandY[0x1d] + unaff_EBX + 7) = unaff_EBX + 0x4c153f;
  *(undefined4 *)((int)s_SIMDRandContexts[0x1c].m_RandY[0x1d] + unaff_EBX + 0xb) = 0;
  this = (RecvTable *)((int)s_SIMDRandContexts[0x1c].m_RandY[0x1e] + unaff_EBX + 0xf);
  *(RecvTable **)((int)s_SIMDRandContexts[0x1c].m_RandY[0x1e] + unaff_EBX + 3) = this;
  piVar1 = *(int **)(&DAT_00aea8d3 + unaff_EBX);
  *(int *)((int)s_SIMDRandContexts[0x1c].m_RandY[0x1e] + unaff_EBX + 7) = *piVar1;
  *piVar1 = unaff_EBX + 0xcf00d7;
  RecvTable::RecvTable(this);
  ___cxa_atexit(unaff_EBX + 0x902e0f,0,*(undefined4 *)(&DAT_00aea85b + unaff_EBX));
  iVar2 = ClientClassInit<DT_PropTractorBeamProjector::ignored>((ignored *)0x0);
  *(int *)((int)s_SIMDRandContexts[0x1c].m_RandY[0x20] + unaff_EBX + 3) = iVar2;
  *(undefined4 *)((int)&PTR_Think_00bb3574 + unaff_EBX + 3) = 2;
  *(int *)((int)&PTR_ShouldPredict_00bb3570 + unaff_EBX + 3) = unaff_EBX + 0xbb35cf;
  *(int *)((int)s_SIMDRandContexts[0x1c].m_RandY[0x20] + unaff_EBX + 7) = unaff_EBX + 0xbb3573;
  return;
}


/* C_PropTractorBeamProjector::YouForgotToImplementOrDeclareClientClass at 0053a8c0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_PropTractorBeamProjector
   * this) */

int __thiscall
C_PropTractorBeamProjector::YouForgotToImplementOrDeclareClientClass
          (C_PropTractorBeamProjector *this)

{
  return 0;
}


/* C_PropTractorBeamProjector::GetClientClass at 0053a8e0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: ClientClass * GetClientClass(C_PropTractorBeamProjector * this) */

ClientClass * __thiscall
C_PropTractorBeamProjector::GetClientClass(C_PropTractorBeamProjector *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x82ed00);
}


/* __tcf_0 at 0097c320 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3ed2d8),in_stack_00000008);
  return;
}


/* C_PropTractorBeamProjector::GetPredDescMap at 0053a8f0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: datamap_t * GetPredDescMap(C_PropTractorBeamProjector * this) */

datamap_t * __thiscall C_PropTractorBeamProjector::GetPredDescMap(C_PropTractorBeamProjector *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x6f218c);
}


/* PredMapInit<C_PropTractorBeamProjector> at 0053a900 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

datamap_t * PredMapInit<C_PropTractorBeamProjector>(C_PropTractorBeamProjector *param_1)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *(undefined4 *)(extraout_ECX + 0x6f217d) = 2;
  *(int *)(extraout_ECX + 0x6f2179) = extraout_ECX + 0x6f21d5;
  return (datamap_t *)(extraout_ECX + 0x6f2179);
}


/* C_PropTractorBeamProjector::Spawn at 0053ad60 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void Spawn(C_PropTractorBeamProjector * this) */

void __thiscall C_PropTractorBeamProjector::Spawn(C_PropTractorBeamProjector *this)

{
  C_BaseEntity::Spawn((C_BaseEntity *)this);
  (*(this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x9d])(this,0xc4a1a000);
  this->m_bEffectsActive = false;
  this->m_bActivated = false;
  return;
}


/* C_PropTractorBeamProjector::UpdateOnRemove at 0053bdb0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void UpdateOnRemove(C_PropTractorBeamProjector * this) */

void __thiscall C_PropTractorBeamProjector::UpdateOnRemove(C_PropTractorBeamProjector *this)

{
  CNewParticleEffect *pCVar1;
  CUtlReference<CNewParticleEffect> *pCVar2;
  CUtlReference<CNewParticleEffect> *pCVar3;
  
  pCVar1 = this->m_hEmitterEffect[0].m_pObject;
  if (pCVar1 != (CNewParticleEffect *)0x0) {
    CParticleProperty::StopEmission
              (&(this->super_C_BaseAnimating).super_C_BaseEntity.m_Particles.super_CParticleProperty
               ,pCVar1,false,false,false,true);
    pCVar1 = this->m_hEmitterEffect[0].m_pObject;
    if (pCVar1 != (CNewParticleEffect *)0x0) {
      pCVar2 = this->m_hEmitterEffect[0].m_pPrev;
      if (pCVar2 == (CUtlReference<CNewParticleEffect> *)0x0) {
        if ((pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
            this->m_hEmitterEffect) {
          pCVar2 = this->m_hEmitterEffect[0].m_pNext;
          (pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar2;
          if (pCVar2 != (CUtlReference<CNewParticleEffect> *)0x0) {
            pCVar2->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
          }
        }
      }
      else {
        pCVar2->m_pNext = this->m_hEmitterEffect[0].m_pNext;
        pCVar3 = this->m_hEmitterEffect[0].m_pNext;
        if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar3->m_pPrev = pCVar2;
        }
      }
      this->m_hEmitterEffect[0].m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
      this->m_hEmitterEffect[0].m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
      this->m_hEmitterEffect[0].m_pObject = (CNewParticleEffect *)0x0;
    }
  }
  pCVar1 = this->m_hEmitterEffect[1].m_pObject;
  if (pCVar1 != (CNewParticleEffect *)0x0) {
    CParticleProperty::StopEmission
              (&(this->super_C_BaseAnimating).super_C_BaseEntity.m_Particles.super_CParticleProperty
               ,pCVar1,false,false,false,true);
    pCVar1 = this->m_hEmitterEffect[1].m_pObject;
    if (pCVar1 != (CNewParticleEffect *)0x0) {
      pCVar2 = this->m_hEmitterEffect[1].m_pPrev;
      if (pCVar2 == (CUtlReference<CNewParticleEffect> *)0x0) {
        if ((pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
            this->m_hEmitterEffect + 1) {
          pCVar2 = this->m_hEmitterEffect[1].m_pNext;
          (pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar2;
          if (pCVar2 != (CUtlReference<CNewParticleEffect> *)0x0) {
            pCVar2->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
          }
        }
      }
      else {
        pCVar2->m_pNext = this->m_hEmitterEffect[1].m_pNext;
        pCVar3 = this->m_hEmitterEffect[1].m_pNext;
        if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar3->m_pPrev = pCVar2;
        }
      }
      this->m_hEmitterEffect[1].m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
      this->m_hEmitterEffect[1].m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
      this->m_hEmitterEffect[1].m_pObject = (CNewParticleEffect *)0x0;
    }
  }
  pCVar1 = this->m_hEmitterEffect[2].m_pObject;
  if (pCVar1 != (CNewParticleEffect *)0x0) {
    CParticleProperty::StopEmission
              (&(this->super_C_BaseAnimating).super_C_BaseEntity.m_Particles.super_CParticleProperty
               ,pCVar1,false,false,false,true);
    pCVar1 = this->m_hEmitterEffect[2].m_pObject;
    if (pCVar1 != (CNewParticleEffect *)0x0) {
      pCVar2 = this->m_hEmitterEffect[2].m_pPrev;
      if (pCVar2 == (CUtlReference<CNewParticleEffect> *)0x0) {
        if ((pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
            this->m_hEmitterEffect + 2) {
          pCVar2 = this->m_hEmitterEffect[2].m_pNext;
          (pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar2;
          if (pCVar2 != (CUtlReference<CNewParticleEffect> *)0x0) {
            pCVar2->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
          }
        }
      }
      else {
        pCVar2->m_pNext = this->m_hEmitterEffect[2].m_pNext;
        pCVar3 = this->m_hEmitterEffect[2].m_pNext;
        if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar3->m_pPrev = pCVar2;
        }
      }
      this->m_hEmitterEffect[2].m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
      this->m_hEmitterEffect[2].m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
      this->m_hEmitterEffect[2].m_pObject = (CNewParticleEffect *)0x0;
    }
  }
  this->m_bEffectsActive = false;
  return;
}


/* C_PropTractorBeamProjector::CreateEffect at 0053b410 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void CreateEffect(C_PropTractorBeamProjector * this) */

void __thiscall C_PropTractorBeamProjector::CreateEffect(C_PropTractorBeamProjector *this)

{
  NetworkVar_m_Particles *this_00;
  float fVar1;
  CNewParticleEffect *pCVar2;
  CUtlReference<CNewParticleEffect> *pCVar3;
  CUtlReference<CNewParticleEffect> *pCVar4;
  Vector *pVVar5;
  CNewParticleEffect *pEffect;
  int unaff_EBX;
  C_PropTractorBeamProjector *pCVar6;
  int local_84;
  Vector local_58;
  Vector local_4c;
  vec_t local_40;
  vec_t local_3c;
  vec_t local_38;
  vec_t local_34;
  vec_t local_30;
  vec_t local_2c;
  undefined *local_28;
  int local_24;
  undefined *local_20;
  
                    /* Unresolved local var: char *[3] pAttachmentNames@[???] */
  ___i686_get_pc_thunk_bx();
  pCVar2 = this->m_hEmitterEffect[0].m_pObject;
  this_00 = &(this->super_C_BaseAnimating).super_C_BaseEntity.m_Particles;
  if (pCVar2 != (CNewParticleEffect *)0x0) {
    CParticleProperty::StopEmission(&this_00->super_CParticleProperty,pCVar2,false,false,false,true)
    ;
    pCVar2 = this->m_hEmitterEffect[0].m_pObject;
    if (pCVar2 != (CNewParticleEffect *)0x0) {
      pCVar3 = this->m_hEmitterEffect[0].m_pPrev;
      if (pCVar3 == (CUtlReference<CNewParticleEffect> *)0x0) {
        if ((pCVar2->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
            this->m_hEmitterEffect) {
          pCVar3 = this->m_hEmitterEffect[0].m_pNext;
          (pCVar2->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar3;
          if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
            pCVar3->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
          }
        }
      }
      else {
        pCVar3->m_pNext = this->m_hEmitterEffect[0].m_pNext;
        pCVar4 = this->m_hEmitterEffect[0].m_pNext;
        if (pCVar4 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar4->m_pPrev = pCVar3;
        }
      }
      this->m_hEmitterEffect[0].m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
      this->m_hEmitterEffect[0].m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
      this->m_hEmitterEffect[0].m_pObject = (CNewParticleEffect *)0x0;
    }
  }
  pCVar2 = this->m_hEmitterEffect[1].m_pObject;
  if (pCVar2 != (CNewParticleEffect *)0x0) {
    CParticleProperty::StopEmission(&this_00->super_CParticleProperty,pCVar2,false,false,false,true)
    ;
    pCVar2 = this->m_hEmitterEffect[1].m_pObject;
    if (pCVar2 != (CNewParticleEffect *)0x0) {
      pCVar3 = this->m_hEmitterEffect[1].m_pPrev;
      if (pCVar3 == (CUtlReference<CNewParticleEffect> *)0x0) {
        if ((pCVar2->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
            this->m_hEmitterEffect + 1) {
          pCVar3 = this->m_hEmitterEffect[1].m_pNext;
          (pCVar2->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar3;
          if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
            pCVar3->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
          }
        }
      }
      else {
        pCVar3->m_pNext = this->m_hEmitterEffect[1].m_pNext;
        pCVar4 = this->m_hEmitterEffect[1].m_pNext;
        if (pCVar4 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar4->m_pPrev = pCVar3;
        }
      }
      this->m_hEmitterEffect[1].m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
      this->m_hEmitterEffect[1].m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
      this->m_hEmitterEffect[1].m_pObject = (CNewParticleEffect *)0x0;
    }
  }
  pCVar2 = this->m_hEmitterEffect[2].m_pObject;
  if (pCVar2 != (CNewParticleEffect *)0x0) {
    CParticleProperty::StopEmission(&this_00->super_CParticleProperty,pCVar2,false,false,false,true)
    ;
    pCVar2 = this->m_hEmitterEffect[2].m_pObject;
    if (pCVar2 != (CNewParticleEffect *)0x0) {
      pCVar3 = this->m_hEmitterEffect[2].m_pPrev;
      if (pCVar3 == (CUtlReference<CNewParticleEffect> *)0x0) {
        if ((pCVar2->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
            this->m_hEmitterEffect + 2) {
          pCVar3 = this->m_hEmitterEffect[2].m_pNext;
          (pCVar2->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar3;
          if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
            pCVar3->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
          }
        }
      }
      else {
        pCVar3->m_pNext = this->m_hEmitterEffect[2].m_pNext;
        pCVar4 = this->m_hEmitterEffect[2].m_pNext;
        if (pCVar4 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar4->m_pPrev = pCVar3;
        }
      }
      this->m_hEmitterEffect[2].m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
      this->m_hEmitterEffect[2].m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
      this->m_hEmitterEffect[2].m_pObject = (CNewParticleEffect *)0x0;
    }
  }
  this->m_bEffectsActive = false;
  local_28 = &UNK_0046cf37 + unaff_EBX;
  local_24 = unaff_EBX + 0x46cf4d;
  local_20 = &UNK_004800ec + unaff_EBX;
  local_84 = 0;
  pCVar6 = this;
  do {
    pCVar2 = pCVar6->m_hEmitterEffect[0].m_pObject;
    if (pCVar2 != (CNewParticleEffect *)0x0) {
      CParticleProperty::StopEmission
                (&this_00->super_CParticleProperty,pCVar2,false,false,false,true);
      pCVar2 = pCVar6->m_hEmitterEffect[0].m_pObject;
      if (pCVar2 != (CNewParticleEffect *)0x0) {
        pCVar3 = pCVar6->m_hEmitterEffect[0].m_pPrev;
        if (pCVar3 == (CUtlReference<CNewParticleEffect> *)0x0) {
          if ((pCVar2->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
              super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
              this->m_hEmitterEffect + local_84) {
            pCVar3 = pCVar6->m_hEmitterEffect[0].m_pNext;
            (pCVar2->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar3;
            if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
              pCVar3->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
            }
          }
        }
        else {
          pCVar3->m_pNext = pCVar6->m_hEmitterEffect[0].m_pNext;
          pCVar4 = pCVar6->m_hEmitterEffect[0].m_pNext;
          if (pCVar4 != (CUtlReference<CNewParticleEffect> *)0x0) {
            pCVar4->m_pPrev = pCVar3;
          }
        }
        pCVar6->m_hEmitterEffect[0].m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
        pCVar6->m_hEmitterEffect[0].m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
        pCVar6->m_hEmitterEffect[0].m_pObject = (CNewParticleEffect *)0x0;
      }
    }
    pEffect = CParticleProperty::Create
                        (&this_00->super_CParticleProperty,(char *)(unaff_EBX + 0x4800f5),
                         PATTACH_POINT_FOLLOW,(&local_28)[local_84]);
    pCVar2 = pCVar6->m_hEmitterEffect[0].m_pObject;
    if (pEffect == pCVar2) {
LAB_0053b747:
      if (pEffect != (CNewParticleEffect *)0x0) {
        pVVar5 = *(Vector **)(&DAT_006289df + unaff_EBX);
        local_34 = pVVar5->x;
        local_30 = pVVar5->y;
        local_2c = pVVar5->z;
        CParticleProperty::AddControlPoint
                  (&this_00->super_CParticleProperty,pEffect,1,(C_BaseEntity *)this,
                   PATTACH_ABSORIGIN,(char *)0x0,*pVVar5,(matrix3x4_t *)0x0);
        pVVar5 = *(Vector **)(&DAT_006289df + unaff_EBX);
        local_40 = pVVar5->x;
        local_3c = pVVar5->y;
        local_38 = pVVar5->z;
        CParticleProperty::AddControlPoint
                  (&this_00->super_CParticleProperty,pCVar6->m_hEmitterEffect[0].m_pObject,2,
                   (C_BaseEntity *)this,PATTACH_ABSORIGIN,(char *)0x0,*pVVar5,(matrix3x4_t *)0x0);
        if (this->m_flLinearForce <= 0.0 && this->m_flLinearForce != 0.0) {
          local_4c.x = 255.0;
          local_4c.z = 32.0;
        }
        else {
          local_4c.x = 64.0;
          local_4c.z = 255.0;
        }
        local_4c.y = 160.0;
        CNewParticleEffect::SetControlPoint(pCVar6->m_hEmitterEffect[0].m_pObject,1,&local_4c);
        fVar1 = this->m_flLinearForce;
                    /* Unresolved local var: matrix3x4_t * mat@[???] */
        C_BaseEntity::CalcAbsolutePosition((C_BaseEntity *)this);
                    /* Unresolved local var: Vector res@[???] */
        local_58.x = fVar1 * (this->super_C_BaseAnimating).super_C_BaseEntity.m_rgflCoordinateFrame.
                             m_flMatVal[0][0];
        local_58.y = (this->super_C_BaseAnimating).super_C_BaseEntity.m_rgflCoordinateFrame.
                     m_flMatVal[1][0] * fVar1;
        local_58.z = (this->super_C_BaseAnimating).super_C_BaseEntity.m_rgflCoordinateFrame.
                     m_flMatVal[2][0] * fVar1;
        CNewParticleEffect::SetControlPoint(pCVar6->m_hEmitterEffect[0].m_pObject,2,&local_58);
      }
    }
    else {
      if (pCVar2 != (CNewParticleEffect *)0x0) {
        pCVar3 = pCVar6->m_hEmitterEffect[0].m_pPrev;
        if (pCVar3 == (CUtlReference<CNewParticleEffect> *)0x0) {
          if ((pCVar2->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
              super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
              this->m_hEmitterEffect + local_84) {
            pCVar3 = pCVar6->m_hEmitterEffect[0].m_pNext;
            (pCVar2->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar3;
            if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
              pCVar3->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
            }
          }
        }
        else {
          pCVar3->m_pNext = pCVar6->m_hEmitterEffect[0].m_pNext;
          pCVar4 = pCVar6->m_hEmitterEffect[0].m_pNext;
          if (pCVar4 != (CUtlReference<CNewParticleEffect> *)0x0) {
            pCVar4->m_pPrev = pCVar3;
          }
        }
        pCVar6->m_hEmitterEffect[0].m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
        pCVar6->m_hEmitterEffect[0].m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
        pCVar6->m_hEmitterEffect[0].m_pObject = (CNewParticleEffect *)0x0;
      }
      pCVar6->m_hEmitterEffect[0].m_pObject = pEffect;
      if (pEffect != (CNewParticleEffect *)0x0) {
        pCVar6->m_hEmitterEffect[0].m_pNext =
             (pEffect->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
             super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead;
        pCVar3 = (pEffect->m_References).
                 super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
                 super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead;
        if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar3->m_pPrev = this->m_hEmitterEffect + local_84;
        }
        pCVar6->m_hEmitterEffect[0].m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
        (pEffect->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
        super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead =
             this->m_hEmitterEffect + local_84;
        pEffect = pCVar6->m_hEmitterEffect[0].m_pObject;
        goto LAB_0053b747;
      }
    }
    local_84 = local_84 + 1;
    pCVar6 = (C_PropTractorBeamProjector *)
             &(pCVar6->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.
              super_IClientThinkable;
    if (local_84 == 3) {
      this->m_bEffectsActive = true;
      return;
    }
  } while( true );
}


/* C_PropTractorBeamProjector::UpdateEffect at 0053aea0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void UpdateEffect(C_PropTractorBeamProjector * this) */

void __thiscall C_PropTractorBeamProjector::UpdateEffect(C_PropTractorBeamProjector *this)

{
  float fVar1;
  Vector local_24;
  Vector local_18;
  
  if (this->m_hEmitterEffect[0].m_pObject != (CNewParticleEffect *)0x0) {
    if (this->m_flLinearForce <= 0.0 && this->m_flLinearForce != 0.0) {
      local_18.x = 255.0;
      local_18.z = 32.0;
    }
    else {
      local_18.x = 64.0;
      local_18.z = 255.0;
    }
    local_18.y = 160.0;
    CNewParticleEffect::SetControlPoint(this->m_hEmitterEffect[0].m_pObject,1,&local_18);
    fVar1 = this->m_flLinearForce;
    C_BaseEntity::CalcAbsolutePosition((C_BaseEntity *)this);
    local_24.x = (this->super_C_BaseAnimating).super_C_BaseEntity.m_rgflCoordinateFrame.m_flMatVal
                 [0][0] * fVar1;
    local_24.y = (this->super_C_BaseAnimating).super_C_BaseEntity.m_rgflCoordinateFrame.m_flMatVal
                 [1][0] * fVar1;
    local_24.z = (this->super_C_BaseAnimating).super_C_BaseEntity.m_rgflCoordinateFrame.m_flMatVal
                 [2][0] * fVar1;
    CNewParticleEffect::SetControlPoint(this->m_hEmitterEffect[0].m_pObject,2,&local_24);
  }
  if (this->m_hEmitterEffect[1].m_pObject != (CNewParticleEffect *)0x0) {
    if (this->m_flLinearForce <= 0.0 && this->m_flLinearForce != 0.0) {
      local_18.x = 255.0;
      local_18.z = 32.0;
    }
    else {
      local_18.x = 64.0;
      local_18.z = 255.0;
    }
    local_18.y = 160.0;
    CNewParticleEffect::SetControlPoint(this->m_hEmitterEffect[1].m_pObject,1,&local_18);
    fVar1 = this->m_flLinearForce;
    C_BaseEntity::CalcAbsolutePosition((C_BaseEntity *)this);
    local_24.x = (this->super_C_BaseAnimating).super_C_BaseEntity.m_rgflCoordinateFrame.m_flMatVal
                 [0][0] * fVar1;
    local_24.y = (this->super_C_BaseAnimating).super_C_BaseEntity.m_rgflCoordinateFrame.m_flMatVal
                 [1][0] * fVar1;
    local_24.z = (this->super_C_BaseAnimating).super_C_BaseEntity.m_rgflCoordinateFrame.m_flMatVal
                 [2][0] * fVar1;
    CNewParticleEffect::SetControlPoint(this->m_hEmitterEffect[1].m_pObject,2,&local_24);
  }
  if (this->m_hEmitterEffect[2].m_pObject != (CNewParticleEffect *)0x0) {
    if (this->m_flLinearForce <= 0.0 && this->m_flLinearForce != 0.0) {
      local_18.x = 255.0;
      local_18.z = 32.0;
    }
    else {
      local_18.x = 64.0;
      local_18.z = 255.0;
    }
    local_18.y = 160.0;
    CNewParticleEffect::SetControlPoint(this->m_hEmitterEffect[2].m_pObject,1,&local_18);
    fVar1 = this->m_flLinearForce;
    C_BaseEntity::CalcAbsolutePosition((C_BaseEntity *)this);
    local_24.x = (this->super_C_BaseAnimating).super_C_BaseEntity.m_rgflCoordinateFrame.m_flMatVal
                 [0][0] * fVar1;
    local_24.y = (this->super_C_BaseAnimating).super_C_BaseEntity.m_rgflCoordinateFrame.m_flMatVal
                 [1][0] * fVar1;
    local_24.z = (this->super_C_BaseAnimating).super_C_BaseEntity.m_rgflCoordinateFrame.m_flMatVal
                 [2][0] * fVar1;
    CNewParticleEffect::SetControlPoint(this->m_hEmitterEffect[2].m_pObject,2,&local_24);
  }
  return;
}


/* C_PropTractorBeamProjector::StopEffect at 0053ab40 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void StopEffect(C_PropTractorBeamProjector * this) */

void __thiscall C_PropTractorBeamProjector::StopEffect(C_PropTractorBeamProjector *this)

{
  CNewParticleEffect *pCVar1;
  CUtlReference<CNewParticleEffect> *pCVar2;
  CUtlReference<CNewParticleEffect> *pCVar3;
  
  pCVar1 = this->m_hEmitterEffect[0].m_pObject;
  if (pCVar1 != (CNewParticleEffect *)0x0) {
    CParticleProperty::StopEmission
              (&(this->super_C_BaseAnimating).super_C_BaseEntity.m_Particles.super_CParticleProperty
               ,pCVar1,false,false,false,true);
    pCVar1 = this->m_hEmitterEffect[0].m_pObject;
    if (pCVar1 != (CNewParticleEffect *)0x0) {
      pCVar2 = this->m_hEmitterEffect[0].m_pPrev;
      if (pCVar2 == (CUtlReference<CNewParticleEffect> *)0x0) {
        if ((pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
            this->m_hEmitterEffect) {
          pCVar2 = this->m_hEmitterEffect[0].m_pNext;
          (pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar2;
          if (pCVar2 != (CUtlReference<CNewParticleEffect> *)0x0) {
            pCVar2->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
          }
        }
      }
      else {
        pCVar2->m_pNext = this->m_hEmitterEffect[0].m_pNext;
        pCVar3 = this->m_hEmitterEffect[0].m_pNext;
        if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar3->m_pPrev = pCVar2;
        }
      }
      this->m_hEmitterEffect[0].m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
      this->m_hEmitterEffect[0].m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
      this->m_hEmitterEffect[0].m_pObject = (CNewParticleEffect *)0x0;
    }
  }
  pCVar1 = this->m_hEmitterEffect[1].m_pObject;
  if (pCVar1 != (CNewParticleEffect *)0x0) {
    CParticleProperty::StopEmission
              (&(this->super_C_BaseAnimating).super_C_BaseEntity.m_Particles.super_CParticleProperty
               ,pCVar1,false,false,false,true);
    pCVar1 = this->m_hEmitterEffect[1].m_pObject;
    if (pCVar1 != (CNewParticleEffect *)0x0) {
      pCVar2 = this->m_hEmitterEffect[1].m_pPrev;
      if (pCVar2 == (CUtlReference<CNewParticleEffect> *)0x0) {
        if ((pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
            this->m_hEmitterEffect + 1) {
          pCVar2 = this->m_hEmitterEffect[1].m_pNext;
          (pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar2;
          if (pCVar2 != (CUtlReference<CNewParticleEffect> *)0x0) {
            pCVar2->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
          }
        }
      }
      else {
        pCVar2->m_pNext = this->m_hEmitterEffect[1].m_pNext;
        pCVar3 = this->m_hEmitterEffect[1].m_pNext;
        if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar3->m_pPrev = pCVar2;
        }
      }
      this->m_hEmitterEffect[1].m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
      this->m_hEmitterEffect[1].m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
      this->m_hEmitterEffect[1].m_pObject = (CNewParticleEffect *)0x0;
    }
  }
  pCVar1 = this->m_hEmitterEffect[2].m_pObject;
  if (pCVar1 != (CNewParticleEffect *)0x0) {
    CParticleProperty::StopEmission
              (&(this->super_C_BaseAnimating).super_C_BaseEntity.m_Particles.super_CParticleProperty
               ,pCVar1,false,false,false,true);
    pCVar1 = this->m_hEmitterEffect[2].m_pObject;
    if (pCVar1 != (CNewParticleEffect *)0x0) {
      pCVar2 = this->m_hEmitterEffect[2].m_pPrev;
      if (pCVar2 == (CUtlReference<CNewParticleEffect> *)0x0) {
        if ((pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
            this->m_hEmitterEffect + 2) {
          pCVar2 = this->m_hEmitterEffect[2].m_pNext;
          (pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar2;
          if (pCVar2 != (CUtlReference<CNewParticleEffect> *)0x0) {
            pCVar2->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
          }
        }
      }
      else {
        pCVar2->m_pNext = this->m_hEmitterEffect[2].m_pNext;
        pCVar3 = this->m_hEmitterEffect[2].m_pNext;
        if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar3->m_pPrev = pCVar2;
        }
      }
      this->m_hEmitterEffect[2].m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
      this->m_hEmitterEffect[2].m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
      this->m_hEmitterEffect[2].m_pObject = (CNewParticleEffect *)0x0;
    }
  }
  this->m_bEffectsActive = false;
  return;
}


/* C_PropTractorBeamProjector::ClientThink at 0053ba80 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ClientThink(C_PropTractorBeamProjector * this) */

void __thiscall C_PropTractorBeamProjector::ClientThink(C_PropTractorBeamProjector *this)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  CStudioHdr *pCVar4;
  int unaff_EBX;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float local_20;
  
                    /* Unresolved local var: float flArmPose@[???]
                       Unresolved local var: int nPoseParam@[???]
                       Unresolved local var: float flAnimSpeed@[???] */
  ___i686_get_pc_thunk_bx();
  if (((this->m_bEnabled == false) || (this->m_bEffectsActive != false)) ||
     (this->m_flLinearForce == 0.0)) {
    piVar3 = *(int **)(unaff_EBX + 0x62830e);
  }
  else {
    piVar3 = *(int **)(unaff_EBX + 0x62830e);
    if (*(float *)(unaff_EBX + 0x4a6782) <= *(float *)(*piVar3 + 0xc) &&
        *(float *)(*piVar3 + 0xc) != *(float *)(unaff_EBX + 0x4a6782)) {
      CreateEffect(this);
      this->m_bActivated = true;
      piVar3 = *(int **)(unaff_EBX + 0x62830e);
    }
  }
  fVar5 = *(float *)(*piVar3 + 0xc);
  fVar7 = this->m_flArmatureStartTime;
  fVar6 = fVar7 + this->m_flArmatureDuration;
  if (fVar5 <= fVar6) {
    local_20 = this->m_flArmatureTarget;
    fVar8 = this->m_flArmatureStart;
    if ((fVar7 != fVar6) || (NAN(fVar7) || NAN(fVar6))) {
      fVar5 = (fVar5 - fVar7) / (fVar6 - fVar7);
                    /* Unresolved local var: float valueSquared@[???] */
      fVar8 = fVar8 + (fVar5 * fVar5 * *(float *)(unaff_EBX + 0x4a6ac2) +
                      fVar5 * fVar5 * fVar5 * *(float *)(unaff_EBX + 0x4a6b5e)) * (local_20 - fVar8)
      ;
LAB_0053bc03:
      local_20 = fVar8;
    }
    else if (fVar5 < fVar6) goto LAB_0053bc03;
    fVar5 = *(float *)(CUtlRBTree<CUtlMap<const_char*,_CPanelMetaClassMgrImp::MetaClassDict_t,_short_unsigned_int>::Node_t,short_unsigned_int,CUtlMap<const_char*,_CPanelMetaClassMgrImp::MetaClassDict_t,_short_unsigned_int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const_char*,_CPanelMetaClassMgrImp::MetaClassDict_t,_short_unsigned_int>::Node_t,_short_unsigned_int>,_short_unsigned_int>_>
                       ::RemoveRebalance + unaff_EBX + 6);
    if ((local_20 <=
         *(float *)(CUtlRBTree<CUtlMap<const_char*,_CPanelMetaClassMgrImp::MetaClassDict_t,_short_unsigned_int>::Node_t,short_unsigned_int,CUtlMap<const_char*,_CPanelMetaClassMgrImp::MetaClassDict_t,_short_unsigned_int>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<const_char*,_CPanelMetaClassMgrImp::MetaClassDict_t,_short_unsigned_int>::Node_t,_short_unsigned_int>,_short_unsigned_int>_>
                    ::RemoveRebalance + unaff_EBX + 6)) && (fVar5 = 0.0, 0.0 <= local_20)) {
      fVar5 = local_20;
    }
  }
  else {
    fVar5 = this->m_flArmatureTarget;
  }
  pCVar4 = (this->super_C_BaseAnimating).m_pStudioHdr;
  if (pCVar4 == (CStudioHdr *)0x0) {
    iVar1 = (*(this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.
              super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0x3c])(this);
    if (iVar1 != 0) {
      C_BaseAnimating::LockStudioHdr(&this->super_C_BaseAnimating);
    }
    pCVar4 = (this->super_C_BaseAnimating).m_pStudioHdr;
    if (pCVar4 != (CStudioHdr *)0x0) goto LAB_0053baf4;
  }
  else {
LAB_0053baf4:
    if (pCVar4->m_pStudioHdr == (studiohdr_t *)0x0) {
      pCVar4 = (CStudioHdr *)0x0;
    }
  }
  iVar1 = C_BaseAnimating::LookupPoseParameter
                    (&this->super_C_BaseAnimating,pCVar4,(char *)(unaff_EBX + 0x47fa99));
  pCVar4 = (this->super_C_BaseAnimating).m_pStudioHdr;
  if (pCVar4 == (CStudioHdr *)0x0) {
    iVar2 = (*(this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.
              super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0x3c])(this);
    if (iVar2 != 0) {
      C_BaseAnimating::LockStudioHdr(&this->super_C_BaseAnimating);
    }
    pCVar4 = (this->super_C_BaseAnimating).m_pStudioHdr;
    if (pCVar4 != (CStudioHdr *)0x0) goto LAB_0053bb26;
  }
  else {
LAB_0053bb26:
    if (pCVar4->m_pStudioHdr == (studiohdr_t *)0x0) {
      pCVar4 = (CStudioHdr *)0x0;
    }
  }
  C_BaseAnimating::SetPoseParameter(&this->super_C_BaseAnimating,pCVar4,iVar1,fVar5);
  fVar5 = *(float *)(**(int **)(unaff_EBX + 0x62830e) + 0xc);
  fVar7 = this->m_flRotationStartTime;
  fVar6 = fVar7 + this->m_flRotationDuration;
  if (fVar6 < fVar5) {
    fVar7 = this->m_flRotationTarget;
  }
  else {
    fVar8 = this->m_flRotationStart;
    if ((fVar7 != fVar6) || (NAN(fVar7) || NAN(fVar6))) {
      fVar5 = (fVar5 - fVar7) / (fVar6 - fVar7);
                    /* Unresolved local var: float valueSquared@[???] */
      fVar7 = (this->m_flRotationTarget - fVar8) *
              (fVar5 * fVar5 * *(float *)(unaff_EBX + 0x4a6ac2) +
              fVar5 * fVar5 * fVar5 * *(float *)(unaff_EBX + 0x4a6b5e)) + fVar8;
    }
    else {
      fVar7 = this->m_flRotationTarget;
      if (fVar5 < fVar6) {
        fVar7 = fVar8;
      }
    }
    fVar5 = this->m_flLinearForce / *(float *)(unaff_EBX + 0x4a6b36);
    if (fVar5 != 0.0) {
      if (fVar5 < 0.0) {
        if (fVar5 <= fVar7) goto LAB_0053bb80;
      }
      else if (fVar7 <= fVar5) goto LAB_0053bb80;
      fVar7 = fVar5;
    }
  }
LAB_0053bb80:
  (this->super_C_BaseAnimating).m_flPlaybackRate = fVar7;
                    /* WARNING: Could not recover jumptable at 0x0053bb9a. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x101])();
  return;
}


/* C_PropTractorBeamProjector::CalculateRotationPose at 0053a930 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: float CalculateRotationPose(C_PropTractorBeamProjector * this) */

float __thiscall C_PropTractorBeamProjector::CalculateRotationPose(C_PropTractorBeamProjector *this)

{
  float fVar1;
  int extraout_ECX;
  float fVar2;
  float fVar3;
  float fVar4;
  float local_8;
  
                    /* Unresolved local var: float flRotationPose@[???]
                       Unresolved local var: float flForcePerc@[???]
                       Unresolved local var: bool bNegativeForce@[???] */
  ___i686_get_pc_thunk_cx();
  fVar2 = *(float *)(**(int **)(extraout_ECX + 0x629461) + 0xc);
  fVar1 = this->m_flRotationStartTime;
  fVar3 = fVar1 + this->m_flRotationDuration;
  if (fVar3 < fVar2) {
    return this->m_flRotationTarget;
  }
  local_8 = this->m_flRotationTarget;
  fVar4 = this->m_flRotationStart;
  if ((fVar1 != fVar3) || (NAN(fVar1) || NAN(fVar3))) {
    fVar2 = (fVar2 - fVar1) / (fVar3 - fVar1);
                    /* Unresolved local var: float valueSquared@[???] */
    fVar4 = fVar4 + (fVar2 * fVar2 * *(float *)(extraout_ECX + 0x4a7c15) +
                    fVar2 * fVar2 * fVar2 * *(float *)(extraout_ECX + 0x4a7cb1)) * (local_8 - fVar4)
    ;
  }
  else if (fVar3 <= fVar2) goto LAB_0053a9e8;
  local_8 = fVar4;
LAB_0053a9e8:
  fVar2 = this->m_flLinearForce / *(float *)(extraout_ECX + 0x4a7c89);
  if (fVar2 != 0.0) {
    if (fVar2 < 0.0) {
      if (local_8 < fVar2) {
        return fVar2;
      }
    }
    else if (fVar2 < local_8) {
      return fVar2;
    }
  }
  return local_8;
}


/* C_PropTractorBeamProjector::CalculateArmaturePose at 0053adb0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: float CalculateArmaturePose(C_PropTractorBeamProjector * this) */

float __thiscall C_PropTractorBeamProjector::CalculateArmaturePose(C_PropTractorBeamProjector *this)

{
  float fVar1;
  int extraout_ECX;
  float fVar2;
  float fVar3;
  float fVar4;
  float local_8;
  
                    /* Unresolved local var: float flArmPose@[???] */
  ___i686_get_pc_thunk_cx();
  fVar2 = *(float *)(**(int **)(extraout_ECX + 0x628fe1) + 0xc);
  fVar1 = this->m_flArmatureStartTime;
  fVar3 = fVar1 + this->m_flArmatureDuration;
  if (fVar3 < fVar2) {
    return this->m_flArmatureTarget;
  }
  local_8 = this->m_flArmatureTarget;
  fVar4 = this->m_flArmatureStart;
  if ((fVar1 != fVar3) || (NAN(fVar1) || NAN(fVar3))) {
    fVar2 = (fVar2 - fVar1) / (fVar3 - fVar1);
                    /* Unresolved local var: float valueSquared@[???] */
    fVar4 = fVar4 + (fVar2 * fVar2 * *(float *)(extraout_ECX + 0x4a7795) +
                    fVar2 * fVar2 * fVar2 * *(float *)(extraout_ECX + 0x4a7831)) * (local_8 - fVar4)
    ;
  }
  else if (fVar3 <= fVar2) goto LAB_0053ae64;
  local_8 = fVar4;
LAB_0053ae64:
  fVar2 = *(float *)(extraout_ECX + 0x4a73c9);
  if ((local_8 <= *(float *)(extraout_ECX + 0x4a73c9)) && (fVar2 = 0.0, 0.0 <= local_8)) {
    fVar2 = local_8;
  }
  return fVar2;
}


/* C_PropTractorBeamProjector::OnToolStartRecording at 0053ba40 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void OnToolStartRecording(C_PropTractorBeamProjector * this) */

void __thiscall C_PropTractorBeamProjector::OnToolStartRecording(C_PropTractorBeamProjector *this)

{
  if (this->m_bEffectsActive == false) {
    return;
  }
  CreateEffect(this);
  return;
}


/* _GLOBAL__I__ZN26C_PropTractorBeamProjector21RecvProxy_LinearForceEPK14CRecvProxyDataPvS3_ at 00079640 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN26C_PropTractorBeamProjector21RecvProxy_LinearForceEPK14CRecvProxyDataPvS3_(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

