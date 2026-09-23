/* DWARF-guided pseudocode for game/client/portal2/c_npc_personality_core.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* _C_NPC_Personality_Core_CreateObject at 0051df20 */

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
       (IClientUnknown)(unaff_EBX + 0x6ac574);
  (this->super_C_BaseCombatCharacter).super_C_BaseFlex.super_C_BaseAnimatingOverlay.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x6aca14);
  (this->super_C_BaseCombatCharacter).super_C_BaseFlex.super_C_BaseAnimatingOverlay.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x6acacc);
  (this->super_C_BaseCombatCharacter).super_C_BaseFlex.super_C_BaseAnimatingOverlay.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x6acb08);
  (this->super_C_BaseCombatCharacter).super_C_BaseFlex.super_C_BaseAnimatingOverlay.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientModelRenderable._vptr_IClientModelRenderable
       = (_func_int_varargs **)(unaff_EBX + 0x6acb24);
  this[1].super_C_BaseCombatCharacter.super_C_BaseFlex.super_C_BaseAnimatingOverlay.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientModelRenderable._vptr_IClientModelRenderable
       = (_func_int_varargs **)0x0;
  this[1].super_C_BaseCombatCharacter.super_C_BaseFlex.super_C_BaseAnimatingOverlay.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)0x0;
  this[1].super_C_BaseCombatCharacter.super_C_BaseFlex.super_C_BaseAnimatingOverlay.
  super_C_BaseAnimating.super_C_BaseEntity.m_iClassname = (string_t)0x0;
  (**(code **)(unaff_EBX + 0x6ac630))(this,entnum,serialNum);
  return &(this->super_C_BaseCombatCharacter).super_C_BaseFlex.super_C_BaseAnimatingOverlay.
          super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable;
}


/* C_NPC_Personality_Core::~C_NPC_Personality_Core at 0051eb20 */

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
       = unaff_EBX + 0x6ab97b;
  ((IClientRenderable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  4))->_vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x6abe1b);
  ((IClientNetworkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  8))->_vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x6abed3);
  ((IClientThinkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  0xc))->_vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x6abf0f);
  (this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
  super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientModelRenderable
  ._vptr_IClientModelRenderable = (_func_int_varargs **)(unaff_EBX + 0x6abf2b);
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
       = *(int *)(&DAT_00645907 + unaff_EBX) + 8;
  ((IClientRenderable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  4))->_vptr_IClientRenderable = (_func_int_varargs **)(*(int *)(&DAT_00645907 + unaff_EBX) + 0x4a8)
  ;
  ((IClientNetworkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  8))->_vptr_IClientNetworkable =
       (_func_int_varargs **)(*(int *)(&DAT_00645907 + unaff_EBX) + 0x560);
  ((IClientThinkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  0xc))->_vptr_IClientThinkable =
       (_func_int_varargs **)(*(int *)(&DAT_00645907 + unaff_EBX) + 0x59c);
  (this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
  super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientModelRenderable
  ._vptr_IClientModelRenderable =
       (_func_int_varargs **)(*(int *)(&DAT_00645907 + unaff_EBX) + 0x5b8);
  C_BaseCombatCharacter::~C_BaseCombatCharacter((C_BaseCombatCharacter *)this,in_stack_ffffffe8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_NPC_Personality_Core::~C_NPC_Personality_Core at 0051ec60 */

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
       = unaff_EBX + 0x6ab83e;
  ((IClientRenderable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  4))->_vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x6abcde);
  ((IClientNetworkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  8))->_vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x6abd96);
  ((IClientThinkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  0xc))->_vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x6abdd2);
  (this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
  super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientModelRenderable
  ._vptr_IClientModelRenderable = (_func_int_varargs **)(unaff_EBX + 0x6abdee);
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
       = *(int *)(unaff_EBX + 0x6457ca) + 8;
  ((IClientRenderable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  4))->_vptr_IClientRenderable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x6457ca) + 0x4a8);
  ((IClientNetworkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  8))->_vptr_IClientNetworkable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x6457ca) + 0x560);
  ((IClientThinkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  0xc))->_vptr_IClientThinkable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x6457ca) + 0x59c);
  (this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
  super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientModelRenderable
  ._vptr_IClientModelRenderable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x6457ca) + 0x5b8);
  C_BaseCombatCharacter::~C_BaseCombatCharacter((C_BaseCombatCharacter *)this,__in_chrg);
  return;
}


/* C_NPC_Personality_Core::UpdateOnRemove at 0051e270 */

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
    this->m_pELight->die = *(float *)(**(int **)(unaff_EBX + 0x645b1f) + 0xc);
    this->m_pELight = (dlight_t *)0x0;
  }
  C_BaseEntity::UpdateOnRemove((C_BaseEntity *)this);
  return;
}


/* C_NPC_Personality_Core::ControlMouth at 0051e0a0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ControlMouth(C_NPC_Personality_Core * this, CStudioHdr *
   pStudioHdr) */

void __thiscall
C_NPC_Personality_Core::ControlMouth(C_NPC_Personality_Core *this,CStudioHdr *pStudioHdr)

{
  float *pfVar1;
  float fVar2;
  int iVar3;
  byte *pbVar4;
  int unaff_EBX;
  longdouble lVar5;
  float fVar6;
  float fVar7;
  
                    /* Unresolved local var: float flOpenAmount@[???]
                       Unresolved local var: float flDarken@[???] */
  ___i686_get_pc_thunk_bx();
  C_BaseAnimating::ControlMouth((C_BaseAnimating *)this,pStudioHdr);
  if (((this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
       super_C_BaseAnimatingOverlay.super_C_BaseAnimating.m_mouth.m_needsEnvelope != 0) &&
     (pStudioHdr != (CStudioHdr *)0x0)) {
    pbVar4 = (byte *)(**(code **)(*(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.
                                           super_C_BaseFlex.super_C_BaseAnimatingOverlay.
                                           super_C_BaseAnimating.super_C_BaseEntity.
                                           super_IClientEntity + 0x3c))(this);
    fVar6 = (float)*pbVar4 * *(float *)(unaff_EBX + 0x4c4558);
    pfVar1 = (float *)(unaff_EBX + 0x4c40d0);
    if (fVar6 < *pfVar1 || fVar6 == *pfVar1) {
      (**(code **)(*(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
                            super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.
                            super_IClientEntity + 0x3c))(this);
      pbVar4 = (byte *)(**(code **)(*(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.
                                             super_C_BaseFlex.super_C_BaseAnimatingOverlay.
                                             super_C_BaseAnimating.super_C_BaseEntity.
                                             super_IClientEntity + 0x3c))(this);
      fVar6 = (float)*pbVar4 * *(float *)(unaff_EBX + 0x4c4558);
      if ((*(float *)(unaff_EBX + 0x4c41dc) <= fVar6) ||
         (NAN(fVar6) || NAN(*(float *)(unaff_EBX + 0x4c41dc)))) {
        fVar7 = fVar6 * *(float *)(&DAT_004c449c + unaff_EBX);
        if (*(float *)(unaff_EBX + 0x4c40d0) <= fVar6 * *(float *)(&DAT_004c449c + unaff_EBX)) {
          fVar7 = *(float *)(unaff_EBX + 0x4c40d0);
        }
      }
      else {
        fVar6 = 0.0;
        fVar7 = 0.0;
      }
    }
    else {
      fVar6 = *pfVar1;
      fVar7 = fVar6;
    }
    fVar2 = this->m_flInvDim;
    if (fVar7 <= fVar2) {
      iVar3 = **(int **)(unaff_EBX + 0x645ce8);
      lVar5 = (longdouble)_logf(0x3f7ae148);
      lVar5 = (longdouble)
              _expf(((float)lVar5 / *(float *)(unaff_EBX + 0x4c6b90)) * *(float *)(iVar3 + 0x10));
      this->m_flInvDim = fVar2 * (float)lVar5;
    }
    else {
      this->m_flInvDim = fVar7;
    }
    fVar7 = (*(float *)(unaff_EBX + 0x4c40d0) - this->m_flInvDim) * *(float *)(unaff_EBX + 0x4c4804)
    ;
    if (fVar6 <= fVar7) {
      fVar6 = fVar7;
    }
    this->m_flMouthAmount = fVar6;
  }
  return;
}


/* C_NPC_Personality_Core::OnDataChanged at 0051e350 */

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
                          super_IClientEntity + 0x274))(this,0xc4a1a000);
    (this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
    super_C_BaseAnimatingOverlay.super_C_BaseAnimating.m_mouth.m_needsEnvelope = 1;
    this->m_flMouthAmount = 0.0;
    this->m_flInvDim = 0.0;
  }
  return;
}


/* C_NPC_Personality_Core::ClientThink at 0051e3c0 */

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
      this->m_pELight->die = *(float *)(**(int **)(&DAT_006459cb + unaff_EBX) + 0xc);
      this->m_pELight = (dlight_t *)0x0;
    }
    goto LAB_0051e462;
  }
  iVar6 = (**(code **)(*(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.
                                super_C_BaseFlex.super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                super_C_BaseEntity.super_IClientEntity + 0x1c8))
                    (this,unaff_EBX + 0x4800b9);
  (**(code **)(*(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
                        super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.
                        super_IClientEntity + 0x1d4))(this,iVar6,local_28,&local_34);
  AngleVectors(&local_34,&local_40,&local_4c,&local_58);
                    /* Unresolved local var: Vector res@[???] */
  local_5c = *(float *)(unaff_EBX + 0x4c4537);
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
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x645a07) + 0x14))
            ((int *)**(undefined4 **)(unaff_EBX + 0x645a07),&local_11c,0x6004003,&local_c8,
             &local_170);
  if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x645a03) + 0x1c) + 0x30) != 0) {
    DebugDrawLine(&local_170,local_164,0xff,0,0,true,-1.0);
  }
  pCVar7 = (this->m_pFlashlightEffect).m_pObject;
  if (pCVar7 == (CNewParticleEffect *)0x0) {
    pVVar3 = *(Vector **)(&DAT_00645a2f + unaff_EBX);
    local_7c = pVVar3->x;
    local_78 = pVVar3->y;
    local_74 = pVVar3->z;
    this_00 = &(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
               super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.m_Particles.
               super_CParticleProperty;
    pCVar8 = CParticleProperty::Create
                       (this_00,(char *)(unaff_EBX + 0x49b397),PATTACH_POINT_FOLLOW,iVar6,*pVVar3,
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
    pVVar3 = *(Vector **)(&DAT_00645a2f + unaff_EBX);
    local_88 = pVVar3->x;
    local_84 = pVVar3->y;
    local_80 = pVVar3->z;
    CParticleProperty::AddControlPoint
              (this_00,pCVar7,1,(C_BaseEntity *)this,PATTACH_CUSTOMORIGIN,(char *)0x0,*pVVar3,
               (matrix3x4_t *)0x0);
    pVVar3 = *(Vector **)(&DAT_00645a2f + unaff_EBX);
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
    if (pCVar7 != (CNewParticleEffect *)0x0) goto LAB_0051e666;
  }
  else {
LAB_0051e666:
    CNewParticleEffect::SetControlPoint(pCVar7,1,local_28);
    CNewParticleEffect::SetControlPoint((this->m_pFlashlightEffect).m_pObject,2,local_164);
    CNewParticleEffect::SetControlPointForwardVector
              ((this->m_pFlashlightEffect).m_pObject,1,&local_40);
  }
  pdVar9 = this->m_pELight;
  if (pdVar9 == (dlight_t *)0x0) {
    puVar4 = *(undefined4 **)(unaff_EBX + 0x645ae7);
    pcVar5 = *(code **)(*(int *)*puVar4 + 0x14);
    iVar6 = (**(code **)(*(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.
                                  super_C_BaseFlex.super_C_BaseAnimatingOverlay.
                                  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
                        0x148))(this);
    pdVar9 = (dlight_t *)(*pcVar5)(*puVar4,iVar6 + 0x40000000);
    this->m_pELight = pdVar9;
    if (pdVar9 == (dlight_t *)0x0) goto LAB_0051e462;
  }
  pdVar9->die = *(float *)(**(int **)(&DAT_006459cb + unaff_EBX) + 0xc) +
                *(float *)(unaff_EBX + 0x4c737f);
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
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x645e3f) + 8))
            ((int *)**(undefined4 **)(unaff_EBX + 0x645e3f),this->m_pELight);
LAB_0051e462:
  (**(code **)(*(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
                        super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.
                        super_IClientEntity + 0x274))(this,0xc4a1a000);
  return;
}


/* ClientClassInit<DT_NPC_Personality_Core::ignored> at 00074c30 */

int ClientClassInit<DT_NPC_Personality_Core::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)s_randomGradients + unaff_EBX + 0x934) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc5c234);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)((int)s_randomGradients + unaff_EBX + 0x93c),
                  (char *)(unaff_EBX + 0x915b2c),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)((int)s_randomGradients + unaff_EBX + 0x978),
                        (char *)(unaff_EBX + 0x91a670),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00aef7f4 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00aef1a8 + unaff_EBX));
      RecvPropBool((RecvProp *)((int)s_randomGradients + unaff_EBX + 0x9b4),
                   (char *)(unaff_EBX + 0x944ad5),0x12d4,1);
      ___cxa_guard_release(unaff_EBX + 0xc5c234);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x808),
             (RecvProp_conflict1 *)((int)s_randomGradients + unaff_EBX + 0x978),2,
             (char *)(unaff_EBX + 0x944aea));
  return 1;
}


/* __static_initialization_and_destruction_0 at 00074d80 */

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
  *(undefined1 *)((int)s_randomGradients + unaff_EBX + 0x7af) = 0;
  *(undefined1 *)((int)s_randomGradients + unaff_EBX + 0x7b0) = 0;
  *(undefined1 *)((int)s_randomGradients + unaff_EBX + 0x7b1) = 0;
  *(undefined1 *)((int)s_randomGradients + unaff_EBX + 0x7b2) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x7b3) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x7b7) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x7bb) = 0x7f7fffff;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x7bf) = 0x7f7fffff;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x7c3) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x7c7) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x7cb) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 1999) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x7d3) = 0x7f7fffff;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x7d7) = 0x7f7fffff;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x7db) = 0x7f7fffff;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x7df) = 0x7f7fffff;
  *(undefined **)((int)s_randomGradients + unaff_EBX + 0x7e3) = &UNK_00af00f7 + unaff_EBX;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x6ab) = unaff_EBX + 0x9449b5;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x6a3) = unaff_EBX + 0x4a918f;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0x6a7) = 0;
  this = (RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x6bb);
  *(RecvTable **)((int)s_pParticlePtrs + unaff_EBX + 0x6af) = this;
  piVar1 = *(int **)(&DAT_00aef053 + unaff_EBX);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x6b3) = *piVar1;
  *piVar1 = unaff_EBX + 0xcf3263;
  RecvTable::RecvTable(this);
  ___cxa_atexit(&UNK_009069ff + unaff_EBX,0,*(undefined4 *)(&DAT_00aeefdb + unaff_EBX));
  iVar2 = ClientClassInit<DT_NPC_Personality_Core::ignored>((ignored *)0x0);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x6cf) = iVar2;
  pIVar3 = GetMaterialProxyDict();
  (*pIVar3->_vptr_IMaterialProxyDict[3])(pIVar3,unaff_EBX + 0x9449ca,unaff_EBX + 0x4a912f);
  return;
}


/* C_NPC_Personality_Core::YouForgotToImplementOrDeclareClientClass at 0051de90 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_NPC_Personality_Core *
   this) */

int __thiscall
C_NPC_Personality_Core::YouForgotToImplementOrDeclareClientClass(C_NPC_Personality_Core *this)

{
  return 0;
}


/* C_NPC_Personality_Core::GetClientClass at 0051deb0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: ClientClass * GetClientClass(C_NPC_Personality_Core * this) */

ClientClass * __thiscall C_NPC_Personality_Core::GetClientClass(C_NPC_Personality_Core *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x84a13c);
}


/* __tcf_0 at 0097b790 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3ec874),in_stack_00000008);
  return;
}


/* CCLightedMouthProxyFactory at 0051dec0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

IMaterialProxy * CCLightedMouthProxyFactory(void)

{
  CResultProxy *this;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  this = operator_new(0xc);
  CResultProxy::CResultProxy(this);
  (this->super_IMaterialProxy)._vptr_IMaterialProxy = (_func_int_varargs **)(unaff_EBX + 0x6ac5b4);
  return &this->super_IMaterialProxy;
}


/* CLightedMouthProxy::~CLightedMouthProxy at 0051e210 */

/* DWARF original prototype: void ~CLightedMouthProxy(CLightedMouthProxy * this, int __in_chrg) */

void __thiscall CLightedMouthProxy::~CLightedMouthProxy(CLightedMouthProxy *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CResultProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(unaff_EBX + 0x6ac267);
  CResultProxy::~CResultProxy(&this->super_CResultProxy,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CLightedMouthProxy::~CLightedMouthProxy at 0051e250 */

/* DWARF original prototype: void ~CLightedMouthProxy(CLightedMouthProxy * this, int __in_chrg) */

void __thiscall CLightedMouthProxy::~CLightedMouthProxy(CLightedMouthProxy *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CResultProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(extraout_ECX + 0x6ac230);
  CResultProxy::~CResultProxy(&this->super_CResultProxy,__in_chrg);
  return;
}


/* CLightedMouthProxy::Init at 0051e070 */

/* DWARF original prototype: bool Init(CLightedMouthProxy * this, IMaterial * pMaterial, KeyValues *
   pKeyValues) */

bool __thiscall
CLightedMouthProxy::Init(CLightedMouthProxy *this,IMaterial *pMaterial,KeyValues *pKeyValues)

{
  bool bVar1;
  
  bVar1 = CResultProxy::Init(&this->super_CResultProxy,pMaterial,pKeyValues);
  return bVar1;
}


/* CLightedMouthProxy::OnBind at 0051dfe0 */

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
    iVar2 = ___dynamic_cast(pCVar1,*(undefined4 *)
                                    (CCTextureScrollMaterialProxyFactory + unaff_EBX + 7),
                            unaff_EBX + 0x6aca77,0);
    if (iVar2 != 0) {
                    /* Unresolved local var: float amt@[???] */
      pC_BaseEntity = *(void **)(iVar2 + 0x12cc);
      goto LAB_0051e03c;
    }
  }
  pC_BaseEntity = (void *)0x3f800000;
LAB_0051e03c:
  CResultProxy::SetFloatResult(&this->super_CResultProxy,(float)pC_BaseEntity);
  return;
}


/* _GLOBAL__I__ZN22C_NPC_Personality_Core17m_pClassRecvTableE at 00074ec0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN22C_NPC_Personality_Core17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

