/* DWARF-guided pseudocode for game/client/portal2/c_npc_personality_core.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* _C_NPC_Personality_Core_CreateObject at 005234e0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_NPC_Personality_Core_CreateObject(int entnum,int serialNum)

{
  C_AI_BaseNPC *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_NPC_Personality_Core * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0x12e8);
  C_AI_BaseNPC::C_AI_BaseNPC(this);
  (this->super_C_BaseCombatCharacter).super_C_BaseFlex.super_C_BaseAnimatingOverlay.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientUnknown =
       (IClientUnknown)(unaff_EBX + 0x6732f4);
  (this->super_C_BaseCombatCharacter).super_C_BaseFlex.super_C_BaseAnimatingOverlay.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x673790);
  (this->super_C_BaseCombatCharacter).super_C_BaseFlex.super_C_BaseAnimatingOverlay.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x673848);
  (this->super_C_BaseCombatCharacter).super_C_BaseFlex.super_C_BaseAnimatingOverlay.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x673884);
  (this->super_C_BaseCombatCharacter).super_C_BaseFlex.super_C_BaseAnimatingOverlay.
  super_C_BaseAnimating.super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6738a0);
  this[1].super_C_BaseCombatCharacter.super_C_BaseFlex.super_C_BaseAnimatingOverlay.
  super_C_BaseAnimating.super_C_BaseEntity.m_iClassname = (string_t)0x0;
  this[1].super_C_BaseCombatCharacter.super_C_BaseFlex.super_C_BaseAnimatingOverlay.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)0x0;
  this[1].super_C_BaseCombatCharacter.super_C_BaseFlex.super_C_BaseAnimatingOverlay.
  super_C_BaseAnimating.super_C_BaseEntity.m_hScriptInstance = (HSCRIPT)0x0;
  (**(code **)(unaff_EBX + 0x6733b0))(this,entnum,serialNum);
  return &(this->super_C_BaseCombatCharacter).super_C_BaseFlex.super_C_BaseAnimatingOverlay.
          super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable;
}


/* C_NPC_Personality_Core::~C_NPC_Personality_Core at 005240c0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_NPC_Personality_Core(C_NPC_Personality_Core * this, int
   __in_chrg) */

void __thiscall
C_NPC_Personality_Core::~C_NPC_Personality_Core(C_NPC_Personality_Core *this,int __in_chrg)

{
  CNewParticleEffect *pCVar1;
  CUtlReference<CNewParticleEffect> *pCVar2;
  CUtlReference<CNewParticleEffect> *pCVar3;
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  *(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
           super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity
       = unaff_EBX + 0x67271b;
  ((IClientRenderable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  4))->_vptr_IClientRenderable = (_func_int_varargs **)(&UNK_00672bb7 + unaff_EBX);
  ((IClientNetworkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  8))->_vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x672c6f);
  ((IClientThinkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  0xc))->_vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x672cab);
  (this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
  super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(&UNK_00672cc7 + unaff_EBX);
  pCVar1 = (this->m_pFlashlightEffect).m_pObject;
  if (pCVar1 != (CNewParticleEffect *)0x0) {
    pCVar2 = (this->m_pFlashlightEffect).m_pPrev;
    if (pCVar2 == (CUtlReference<CNewParticleEffect> *)0x0) {
      if ((pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
          &this->m_pFlashlightEffect) {
        pCVar2 = (this->m_pFlashlightEffect).m_pNext;
        (pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
        super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar2;
        if (pCVar2 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar2->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
        }
      }
    }
    else {
      pCVar2->m_pNext = (this->m_pFlashlightEffect).m_pNext;
      pCVar3 = (this->m_pFlashlightEffect).m_pNext;
      if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
        pCVar3->m_pPrev = pCVar2;
      }
    }
    (this->m_pFlashlightEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
    (this->m_pFlashlightEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
    (this->m_pFlashlightEffect).m_pObject = (CNewParticleEffect *)0x0;
  }
  *(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
           super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity
       = *(int *)(unaff_EBX + 0x60d317) + 8;
  ((IClientRenderable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  4))->_vptr_IClientRenderable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x60d317) + 0x4a4);
  ((IClientNetworkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  8))->_vptr_IClientNetworkable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x60d317) + 0x55c);
  ((IClientThinkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  0xc))->_vptr_IClientThinkable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x60d317) + 0x598);
  (this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
  super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x60d317) + 0x5b4);
  C_BaseCombatCharacter::~C_BaseCombatCharacter((C_BaseCombatCharacter *)this,in_stack_ffffffe8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_NPC_Personality_Core::~C_NPC_Personality_Core at 00524200 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_NPC_Personality_Core(C_NPC_Personality_Core * this, int
   __in_chrg) */

void __thiscall
C_NPC_Personality_Core::~C_NPC_Personality_Core(C_NPC_Personality_Core *this,int __in_chrg)

{
  CNewParticleEffect *pCVar1;
  CUtlReference<CNewParticleEffect> *pCVar2;
  CUtlReference<CNewParticleEffect> *pCVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  *(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
           super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity
       = unaff_EBX + 0x6725de;
  ((IClientRenderable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  4))->_vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x672a7a);
  ((IClientNetworkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  8))->_vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x672b32);
  ((IClientThinkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  0xc))->_vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x672b6e);
  (this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
  super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(unaff_EBX + 0x672b8a);
  pCVar1 = (this->m_pFlashlightEffect).m_pObject;
  if (pCVar1 != (CNewParticleEffect *)0x0) {
    pCVar2 = (this->m_pFlashlightEffect).m_pPrev;
    if (pCVar2 == (CUtlReference<CNewParticleEffect> *)0x0) {
      if ((pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
          &this->m_pFlashlightEffect) {
        pCVar2 = (this->m_pFlashlightEffect).m_pNext;
        (pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
        super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar2;
        if (pCVar2 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar2->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
        }
      }
    }
    else {
      pCVar2->m_pNext = (this->m_pFlashlightEffect).m_pNext;
      pCVar3 = (this->m_pFlashlightEffect).m_pNext;
      if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
        pCVar3->m_pPrev = pCVar2;
      }
    }
    (this->m_pFlashlightEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
    (this->m_pFlashlightEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
    (this->m_pFlashlightEffect).m_pObject = (CNewParticleEffect *)0x0;
  }
  *(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
           super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity
       = *(int *)(unaff_EBX + 0x60d1da) + 8;
  ((IClientRenderable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  4))->_vptr_IClientRenderable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x60d1da) + 0x4a4);
  ((IClientNetworkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  8))->_vptr_IClientNetworkable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x60d1da) + 0x55c);
  ((IClientThinkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  0xc))->_vptr_IClientThinkable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x60d1da) + 0x598);
  (this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
  super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x60d1da) + 0x5b4);
  C_BaseCombatCharacter::~C_BaseCombatCharacter((C_BaseCombatCharacter *)this,__in_chrg);
  return;
}


/* C_NPC_Personality_Core::UpdateOnRemove at 00523810 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void UpdateOnRemove(C_NPC_Personality_Core * this) */

void __thiscall C_NPC_Personality_Core::UpdateOnRemove(C_NPC_Personality_Core *this)

{
  CNewParticleEffect *pCVar1;
  CUtlReference<CNewParticleEffect> *pCVar2;
  CUtlReference<CNewParticleEffect> *pCVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  pCVar1 = (this->m_pFlashlightEffect).m_pObject;
  if (pCVar1 != (CNewParticleEffect *)0x0) {
    CParticleProperty::StopEmissionAndDestroyImmediately
              (&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
                super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.m_Particles.
                super_CParticleProperty,pCVar1);
    pCVar1 = (this->m_pFlashlightEffect).m_pObject;
    if (pCVar1 != (CNewParticleEffect *)0x0) {
      pCVar2 = (this->m_pFlashlightEffect).m_pPrev;
      if (pCVar2 == (CUtlReference<CNewParticleEffect> *)0x0) {
        if ((pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
            &this->m_pFlashlightEffect) {
          pCVar2 = (this->m_pFlashlightEffect).m_pNext;
          (pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar2;
          if (pCVar2 != (CUtlReference<CNewParticleEffect> *)0x0) {
            pCVar2->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
          }
        }
      }
      else {
        pCVar2->m_pNext = (this->m_pFlashlightEffect).m_pNext;
        pCVar3 = (this->m_pFlashlightEffect).m_pNext;
        if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar3->m_pPrev = pCVar2;
        }
      }
      (this->m_pFlashlightEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
      (this->m_pFlashlightEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
      (this->m_pFlashlightEffect).m_pObject = (CNewParticleEffect *)0x0;
    }
  }
  if (this->m_pELight != (dlight_t *)0x0) {
    this->m_pELight->die = *(float *)(**(int **)(unaff_EBX + 0x60d533) + 0xc);
    this->m_pELight = (dlight_t *)0x0;
  }
  C_BaseEntity::UpdateOnRemove((C_BaseEntity *)this);
  return;
}


/* C_NPC_Personality_Core::ControlMouth at 00523680 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ControlMouth(C_NPC_Personality_Core * this, CStudioHdr *
   pStudioHdr) */

void __thiscall
C_NPC_Personality_Core::ControlMouth(C_NPC_Personality_Core *this,CStudioHdr *pStudioHdr)

{
  byte *pbVar1;
  int unaff_EBX;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  
                    /* Unresolved local var: float flOpenAmount@[DW_OP_reg22(XMM1)]
                       Unresolved local var: float flStepTime@[???] */
  ___i686_get_pc_thunk_bx();
  C_BaseAnimating::ControlMouth((C_BaseAnimating *)this,pStudioHdr);
  if ((this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
      super_C_BaseAnimatingOverlay.super_C_BaseAnimating.m_mouth.m_needsEnvelope == 0) {
    return;
  }
  if (pStudioHdr == (CStudioHdr *)0x0) {
    return;
  }
  pbVar1 = (byte *)(**(code **)(*(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.
                                         super_C_BaseFlex.super_C_BaseAnimatingOverlay.
                                         super_C_BaseAnimating.super_C_BaseEntity.
                                         super_IClientEntity + 0x3c))(this);
  fVar2 = (float)*pbVar1 * *(float *)(unaff_EBX + 0x495de0);
  if (fVar2 < *(float *)(&DAT_004933a8 + unaff_EBX) ||
      fVar2 == *(float *)(&DAT_004933a8 + unaff_EBX)) {
    fVar5 = this->m_flMouthAmount;
    fVar3 = fVar2 - fVar5;
    if (*(float *)(unaff_EBX + 0x497f34) <= fVar3 && fVar3 != *(float *)(unaff_EBX + 0x497f34))
    goto LAB_005236dc;
    fVar4 = *(float *)(**(int **)(unaff_EBX + 0x60d6bc) + 0xc);
    if (this->m_flMouthHoldTime <= fVar4 && fVar4 != this->m_flMouthHoldTime) {
      fVar2 = 0.0;
      fVar3 = 0.0 - fVar5;
    }
    fVar4 = *(float *)(unaff_EBX + 0x4932ac);
    if (fVar4 < fVar3) goto LAB_00523718;
  }
  else {
    fVar5 = this->m_flMouthAmount;
LAB_005236dc:
    this->m_flMouthHoldTime =
         *(float *)(**(int **)(unaff_EBX + 0x60d6bc) + 0xc) + *(float *)(&DAT_004933ac + unaff_EBX);
    fVar2 = *(float *)(unaff_EBX + 0x4931b0);
    fVar3 = fVar2 - fVar5;
    fVar4 = *(float *)(unaff_EBX + 0x49323c);
    if (fVar4 < fVar3) {
LAB_00523718:
      fVar2 = fVar4 + fVar5;
      goto LAB_0052371f;
    }
  }
  if (fVar3 < (float)((uint)fVar4 ^ *(uint *)(unaff_EBX + 0x497f3c))) {
    fVar2 = fVar5 - fVar4;
  }
LAB_0052371f:
  this->m_flMouthAmount = fVar2;
  return;
}


/* C_NPC_Personality_Core::OnDataChanged at 005238f0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void OnDataChanged(C_NPC_Personality_Core * this, DataUpdateType_t
   updateType) */

void __thiscall
C_NPC_Personality_Core::OnDataChanged(C_NPC_Personality_Core *this,DataUpdateType_t updateType)

{
  C_AI_BaseNPC::OnDataChanged(&this->super_C_AI_BaseNPC,updateType);
  if (updateType == DATA_UPDATE_CREATED) {
    (**(code **)(*(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
                          super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.
                          super_IClientEntity + 0x26c))(this,0xc4a1a000);
    (this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
    super_C_BaseAnimatingOverlay.super_C_BaseAnimating.m_mouth.m_needsEnvelope = 1;
    this->m_flMouthAmount = 0.0;
    this->m_flMouthHoldTime = 0.0;
  }
  return;
}


/* C_NPC_Personality_Core::ClientThink at 00523960 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ClientThink(C_NPC_Personality_Core * this) */

void __thiscall C_NPC_Personality_Core::ClientThink(C_NPC_Personality_Core *this)

{
  CParticleProperty *this_00;
  CUtlReference<CNewParticleEffect> *pCVar1;
  CUtlReference<CNewParticleEffect> *pCVar2;
  Vector *pVVar3;
  undefined4 *puVar4;
  code *pcVar5;
  Vector vecOriginOffset;
  int iVar6;
  CNewParticleEffect *pCVar7;
  CNewParticleEffect *pCVar8;
  C_BasePlayer *pEntity;
  dlight_t *pdVar9;
  int unaff_EBX;
  Vector local_170;
  Vector local_164 [6];
  float local_11c;
  float local_118;
  float local_114;
  float local_10c;
  float local_108;
  float local_104;
  undefined4 local_fc;
  undefined4 local_f8;
  undefined4 local_f4;
  undefined4 local_ec;
  undefined4 local_e8;
  undefined4 local_e4;
  undefined4 local_dc;
  undefined1 local_d8;
  undefined1 local_d7;
  CTraceFilterSimple local_c8;
  float local_b8;
  float local_b4;
  float local_b0;
  float local_ac;
  float local_a8;
  float local_a4;
  vec_t local_a0;
  vec_t local_9c;
  vec_t local_98;
  vec_t local_94;
  vec_t local_90;
  vec_t local_8c;
  vec_t local_88;
  vec_t local_84;
  vec_t local_80;
  vec_t local_7c;
  vec_t local_78;
  vec_t local_74;
  float local_70;
  float local_6c;
  float local_68;
  float local_64;
  float local_60;
  float local_5c;
  Vector local_58;
  Vector local_4c;
  Vector local_40;
  QAngle local_34;
  Vector local_28 [2];
  
  ___i686_get_pc_thunk_bx();
  if (this->m_bFlashlightEnabled == false) {
    pCVar7 = (this->m_pFlashlightEffect).m_pObject;
    if (pCVar7 != (CNewParticleEffect *)0x0) {
      CParticleProperty::StopEmissionAndDestroyImmediately
                (&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
                  super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.m_Particles.
                  super_CParticleProperty,pCVar7);
      pCVar7 = (this->m_pFlashlightEffect).m_pObject;
      if (pCVar7 != (CNewParticleEffect *)0x0) {
        pCVar1 = (this->m_pFlashlightEffect).m_pPrev;
        if (pCVar1 == (CUtlReference<CNewParticleEffect> *)0x0) {
          if ((pCVar7->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
              super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
              &this->m_pFlashlightEffect) {
            pCVar1 = (this->m_pFlashlightEffect).m_pNext;
            (pCVar7->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar1;
            if (pCVar1 != (CUtlReference<CNewParticleEffect> *)0x0) {
              pCVar1->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
            }
          }
        }
        else {
          pCVar1->m_pNext = (this->m_pFlashlightEffect).m_pNext;
          pCVar2 = (this->m_pFlashlightEffect).m_pNext;
          if (pCVar2 != (CUtlReference<CNewParticleEffect> *)0x0) {
            pCVar2->m_pPrev = pCVar1;
          }
        }
        (this->m_pFlashlightEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
        (this->m_pFlashlightEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
        (this->m_pFlashlightEffect).m_pObject = (CNewParticleEffect *)0x0;
      }
    }
    if (this->m_pELight != (dlight_t *)0x0) {
      this->m_pELight->die = *(float *)(**(int **)(unaff_EBX + 0x60d3df) + 0xc);
      this->m_pELight = (dlight_t *)0x0;
    }
    goto LAB_00523a02;
  }
  iVar6 = (**(code **)(*(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.
                                super_C_BaseFlex.super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                super_C_BaseEntity.super_IClientEntity + 0x1c4))
                    (this,unaff_EBX + 0x450145);
  (**(code **)(*(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
                        super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.
                        super_IClientEntity + 0x1d0))(this,iVar6,local_28,&local_34);
  AngleVectors(&local_34,&local_40,&local_4c,&local_58);
                    /* Unresolved local var: Vector res@[???] */
  local_5c = *(float *)(unaff_EBX + 0x493667);
  local_64 = local_40.x * local_5c;
  local_60 = local_40.y * local_5c;
  local_5c = local_5c * local_40.z;
                    /* Unresolved local var: Vector res@[???] */
  local_70 = local_28[0].x + local_64;
  local_6c = local_28[0].y + local_60;
  local_68 = local_28[0].z + local_5c;
  local_10c = local_70 - local_28[0].x;
  local_108 = local_6c - local_28[0].y;
  local_104 = local_68 - local_28[0].z;
  local_d7 = local_10c * local_10c + local_108 * local_108 + local_104 * local_104 != 0.0;
  local_e4 = 0;
  local_e8 = 0;
  local_ec = 0;
  local_dc = 0;
  local_d8 = 1;
  local_f4 = 0;
  local_f8 = 0;
  local_fc = 0;
  local_11c = local_28[0].x;
  local_118 = local_28[0].y;
  local_114 = local_28[0].z;
  CTraceFilterSimple::CTraceFilterSimple(&local_c8,(IHandleEntity *)this,0,(ShouldHitFunc_t)0x0);
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x60d41b) + 0x14))
            ((int *)**(undefined4 **)(unaff_EBX + 0x60d41b),&local_11c,0x6004003,&local_c8,
             &local_170);
  if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x60d417) + 0x1c) + 0x30) != 0) {
    DebugDrawLine(&local_170,local_164,0xff,0,0,true,-1.0);
  }
  pCVar7 = (this->m_pFlashlightEffect).m_pObject;
  if (pCVar7 == (CNewParticleEffect *)0x0) {
    pVVar3 = *(Vector **)(unaff_EBX + 0x60d443);
    local_7c = pVVar3->x;
    local_78 = pVVar3->y;
    local_74 = pVVar3->z;
    this_00 = &(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
               super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.m_Particles.
               super_CParticleProperty;
    pCVar8 = CParticleProperty::Create
                       (this_00,(char *)(unaff_EBX + 0x46b4af),PATTACH_POINT_FOLLOW,iVar6,*pVVar3,
                        (matrix3x4_t *)0x0);
    pCVar7 = (this->m_pFlashlightEffect).m_pObject;
    if (pCVar8 != pCVar7) {
      if (pCVar7 != (CNewParticleEffect *)0x0) {
        pCVar1 = (this->m_pFlashlightEffect).m_pPrev;
        if (pCVar1 == (CUtlReference<CNewParticleEffect> *)0x0) {
          if ((pCVar7->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
              super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
              &this->m_pFlashlightEffect) {
            pCVar1 = (this->m_pFlashlightEffect).m_pNext;
            (pCVar7->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar1;
            if (pCVar1 != (CUtlReference<CNewParticleEffect> *)0x0) {
              pCVar1->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
            }
          }
        }
        else {
          pCVar1->m_pNext = (this->m_pFlashlightEffect).m_pNext;
          pCVar2 = (this->m_pFlashlightEffect).m_pNext;
          if (pCVar2 != (CUtlReference<CNewParticleEffect> *)0x0) {
            pCVar2->m_pPrev = pCVar1;
          }
        }
        (this->m_pFlashlightEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
        (this->m_pFlashlightEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
        (this->m_pFlashlightEffect).m_pObject = (CNewParticleEffect *)0x0;
      }
      (this->m_pFlashlightEffect).m_pObject = pCVar8;
      pCVar7 = (CNewParticleEffect *)0x0;
      if (pCVar8 != (CNewParticleEffect *)0x0) {
        (this->m_pFlashlightEffect).m_pNext =
             (pCVar8->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
             super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead;
        pCVar1 = (pCVar8->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>
                 .super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead;
        if (pCVar1 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar1->m_pPrev = &this->m_pFlashlightEffect;
        }
        (this->m_pFlashlightEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
        (pCVar8->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
        super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead =
             &this->m_pFlashlightEffect;
        pCVar7 = (this->m_pFlashlightEffect).m_pObject;
      }
    }
    pVVar3 = *(Vector **)(unaff_EBX + 0x60d443);
    local_88 = pVVar3->x;
    local_84 = pVVar3->y;
    local_80 = pVVar3->z;
    CParticleProperty::AddControlPoint
              (this_00,pCVar7,1,(C_BaseEntity *)this,PATTACH_CUSTOMORIGIN,(char *)0x0,*pVVar3,
               (matrix3x4_t *)0x0);
    pVVar3 = *(Vector **)(unaff_EBX + 0x60d443);
    local_94 = pVVar3->x;
    local_90 = pVVar3->y;
    local_8c = pVVar3->z;
    CParticleProperty::AddControlPoint
              (this_00,(this->m_pFlashlightEffect).m_pObject,2,(C_BaseEntity *)this,
               PATTACH_CUSTOMORIGIN,(char *)0x0,*pVVar3,(matrix3x4_t *)0x0);
    local_a0 = pVVar3->x;
    local_9c = pVVar3->y;
    local_98 = pVVar3->z;
    pEntity = C_BasePlayer::GetLocalPlayer(-1);
    vecOriginOffset.y = local_9c;
    vecOriginOffset.x = local_a0;
    vecOriginOffset.z = local_98;
    CParticleProperty::AddControlPoint
              (this_00,(this->m_pFlashlightEffect).m_pObject,3,(C_BaseEntity *)pEntity,
               PATTACH_EYES_FOLLOW,(char *)0x0,vecOriginOffset,(matrix3x4_t *)0x0);
    pCVar7 = (this->m_pFlashlightEffect).m_pObject;
    if (pCVar7 != (CNewParticleEffect *)0x0) goto LAB_00523c06;
  }
  else {
LAB_00523c06:
    CNewParticleEffect::SetControlPoint(pCVar7,1,local_28);
    CNewParticleEffect::SetControlPoint((this->m_pFlashlightEffect).m_pObject,2,local_164);
    CNewParticleEffect::SetControlPointForwardVector
              ((this->m_pFlashlightEffect).m_pObject,1,&local_40);
  }
  pdVar9 = this->m_pELight;
  if (pdVar9 == (dlight_t *)0x0) {
    puVar4 = *(undefined4 **)(unaff_EBX + 0x60d4ff);
    pcVar5 = *(code **)(*(int *)*puVar4 + 0x14);
    iVar6 = (**(code **)(*(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.
                                  super_C_BaseFlex.super_C_BaseAnimatingOverlay.
                                  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
                        0x144))(this);
    pdVar9 = (dlight_t *)(*pcVar5)(*puVar4,iVar6 + 0x40000000);
    this->m_pELight = pdVar9;
    if (pdVar9 == (dlight_t *)0x0) goto LAB_00523a02;
  }
  pdVar9->die = *(float *)(**(int **)(unaff_EBX + 0x60d3df) + 0xc) +
                *(float *)(&DAT_0049649f + unaff_EBX);
  this->m_pELight->flags = 1;
  this->m_pELight->radius = 64.0;
  this->m_pELight->style = 0;
  this->m_pELight->m_pExclusiveLightReceiver =
       (IClientRenderable *)
       ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
              super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.
              super_IClientEntity + 4);
  (this->m_pELight->color).r = 0xe1;
  (this->m_pELight->color).g = 0xe1;
  (this->m_pELight->color).b = 0xff;
  (this->m_pELight->color).exponent = '\0';
                    /* Unresolved local var: Vector res@[???] */
  local_ac = local_40.x + local_40.x;
  local_a8 = local_40.y + local_40.y;
  local_a4 = local_40.z + local_40.z;
                    /* Unresolved local var: Vector res@[???] */
  local_b8 = local_28[0].x + local_ac;
  local_b4 = local_28[0].y + local_a8;
  local_b0 = local_28[0].z + local_a4;
  pdVar9 = this->m_pELight;
  (pdVar9->origin).x = local_b8;
  (pdVar9->origin).y = local_b4;
  (pdVar9->origin).z = local_b0;
  (**(code **)(*(int *)**(undefined4 **)(&DAT_0060d84f + unaff_EBX) + 8))
            ((int *)**(undefined4 **)(&DAT_0060d84f + unaff_EBX),this->m_pELight);
LAB_00523a02:
  (**(code **)(*(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
                        super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.
                        super_IClientEntity + 0x26c))(this,0xc4a1a000);
  return;
}


/* ClientClassInit<DT_NPC_Personality_Core::ignored> at 00074d60 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

int ClientClassInit<DT_NPC_Personality_Core::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&UNK_00004ec4)[(int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] ==
      '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc43024);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  (&UNK_00004ecc +
                  (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                  &UNK_008eafec + unaff_EBX,0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4f0].
                               m_SerialNumber + unaff_EBX),(char *)(unaff_EBX + 0x8efbcc),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00abc674 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00abc02c + unaff_EBX));
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4f4].m_pEntity +
                   unaff_EBX),(char *)(unaff_EBX + 0x91a05d),0x12d4,1);
      ___cxa_guard_release(unaff_EBX + 0xc43024);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x5a8),
             (RecvProp_conflict1 *)
             ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4f0].m_SerialNumber +
             unaff_EBX),2,(char *)(unaff_EBX + 0x91a072));
  return 1;
}


/* __static_initialization_and_destruction_0 at 00074eb0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  RecvTable *this;
  int *piVar1;
  int iVar2;
  IMaterialProxyDict *pIVar3;
  int unaff_EBX;
  longlong lVar4;
  
  lVar4 = ___i686_get_pc_thunk_bx();
  if (lVar4 != 0xffff00000001) {
    return;
  }
  (&UNK_00004d3f)[(int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
  (&UNK_00004d40)[(int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
  (&UNK_00004d41)[(int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
  (&UNK_00004d42)[(int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
  *(undefined4 *)
   (&UNK_00004d43 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_00004d47 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_00004d4b + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       0x7f7fffff;
  *(undefined4 *)
   (&UNK_00004d4f + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       0x7f7fffff;
  *(undefined4 *)
   (&UNK_00004d53 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_00004d57 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_00004d5b + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_00004d5f + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_00004d63 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       0x7f7fffff;
  *(undefined4 *)
   (&UNK_00004d67 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       0x7f7fffff;
  *(undefined4 *)
   (&UNK_00004d6b + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       0x7f7fffff;
  *(undefined4 *)
   (&UNK_00004d6f + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       0x7f7fffff;
  *(undefined **)
   (&UNK_00004d73 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       &UNK_00abcf47 + unaff_EBX;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 1099) = unaff_EBX + 0x919f3d;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x443) = unaff_EBX + 0x4ae61f;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0x447) = 0;
  this = (RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x45b);
  *(RecvTable **)((int)s_pParticlePtrs + unaff_EBX + 0x44f) = this;
  piVar1 = *(int **)(&DAT_00abbed7 + unaff_EBX);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x453) = *piVar1;
  *piVar1 = unaff_EBX + 0xcd9ca3;
  RecvTable::RecvTable(this);
  ___cxa_atexit(unaff_EBX + 0x8dc3ef,0,*(undefined4 *)(&DAT_00abbe5f + unaff_EBX));
  iVar2 = ClientClassInit<DT_NPC_Personality_Core::ignored>((ignored *)0x0);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x46f) = iVar2;
  pIVar3 = GetMaterialProxyDict();
  (*pIVar3->_vptr_IMaterialProxyDict[3])(pIVar3,unaff_EBX + 0x919f52,unaff_EBX + 0x4ae5bf);
  return;
}


/* C_NPC_Personality_Core::YouForgotToImplementOrDeclareClientClass at 00523450 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_NPC_Personality_Core *
   this) */

int __thiscall
C_NPC_Personality_Core::YouForgotToImplementOrDeclareClientClass(C_NPC_Personality_Core *this)

{
  return 0;
}


/* C_NPC_Personality_Core::GetClientClass at 00523470 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: ClientClass * GetClientClass(C_NPC_Personality_Core * this) */

ClientClass * __thiscall C_NPC_Personality_Core::GetClientClass(C_NPC_Personality_Core *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x82b6ec);
}


/* __tcf_0 at 009512b0 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3fd8c4),in_stack_00000008);
  return;
}


/* CCLightedMouthProxyFactory at 00523480 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

IMaterialProxy * CCLightedMouthProxyFactory(void)

{
  CResultProxy *this;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  this = operator_new(0xc);
  CResultProxy::CResultProxy(this);
  (this->super_IMaterialProxy)._vptr_IMaterialProxy = (_func_int_varargs **)(unaff_EBX + 0x673334);
  return &this->super_IMaterialProxy;
}


/* CLightedMouthProxy::~CLightedMouthProxy at 005237b0 */

/* DWARF original prototype: void ~CLightedMouthProxy(CLightedMouthProxy * this, int __in_chrg) */

void __thiscall CLightedMouthProxy::~CLightedMouthProxy(CLightedMouthProxy *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CResultProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(unaff_EBX + 0x673007);
  CResultProxy::~CResultProxy(&this->super_CResultProxy,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CLightedMouthProxy::~CLightedMouthProxy at 005237f0 */

/* DWARF original prototype: void ~CLightedMouthProxy(CLightedMouthProxy * this, int __in_chrg) */

void __thiscall CLightedMouthProxy::~CLightedMouthProxy(CLightedMouthProxy *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CResultProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(extraout_ECX + 0x672fd0);
  CResultProxy::~CResultProxy(&this->super_CResultProxy,__in_chrg);
  return;
}


/* CLightedMouthProxy::Init at 00523650 */

/* DWARF original prototype: bool Init(CLightedMouthProxy * this, IMaterial * pMaterial, KeyValues *
   pKeyValues) */

bool __thiscall
CLightedMouthProxy::Init(CLightedMouthProxy *this,IMaterial *pMaterial,KeyValues *pKeyValues)

{
  bool bVar1;
  
  bVar1 = CResultProxy::Init(&this->super_CResultProxy,pMaterial,pKeyValues);
  return bVar1;
}


/* CLightedMouthProxy::OnBind at 005235b0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void OnBind(CLightedMouthProxy * this, void * pC_BaseEntity) */

void __thiscall CLightedMouthProxy::OnBind(CLightedMouthProxy *this,void *pC_BaseEntity)

{
  C_BaseEntity *pCVar1;
  int iVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: C_BaseEntity * pEntity@[???]
                       Unresolved local var: C_NPC_Personality_Core * core@[???] */
  ___i686_get_pc_thunk_bx();
  if (pC_BaseEntity == (void *)0x0) {
    return;
  }
  pCVar1 = CResultProxy::BindArgToEntity(&this->super_CResultProxy,pC_BaseEntity);
  if (pCVar1 != (C_BaseEntity *)0x0) {
    iVar2 = ___dynamic_cast(pCVar1,*(undefined4 *)(unaff_EBX + 0x60d76b),unaff_EBX + 0x6737e3,0);
    if (iVar2 != 0) {
      pC_BaseEntity = (void *)(*(float *)(unaff_EBX + 0x49347f) + *(float *)(&DAT_000012cc + iVar2))
      ;
      goto LAB_00523618;
    }
  }
  pC_BaseEntity = (void *)0x3f800000;
LAB_00523618:
  CResultProxy::SetFloatResult(&this->super_CResultProxy,(float)pC_BaseEntity);
  return;
}


/* _GLOBAL__I__ZN22C_NPC_Personality_Core17m_pClassRecvTableE at 00074ff0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN22C_NPC_Personality_Core17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

