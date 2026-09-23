/* DWARF-guided pseudocode for game/client/portal2/c_npc_hover_turret.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* _C_NPC_Hover_Turret_CreateObject at 005228f0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_NPC_Hover_Turret_CreateObject(int entnum,int serialNum)

{
  C_AI_BaseNPC *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_NPC_Hover_Turret * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0x12ec);
  C_AI_BaseNPC::C_AI_BaseNPC(this);
  (this->super_C_BaseCombatCharacter).super_C_BaseFlex.super_C_BaseAnimatingOverlay.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientUnknown =
       (IClientUnknown)(unaff_EBX + 0x6738e4);
  (this->super_C_BaseCombatCharacter).super_C_BaseFlex.super_C_BaseAnimatingOverlay.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x673d80);
  (this->super_C_BaseCombatCharacter).super_C_BaseFlex.super_C_BaseAnimatingOverlay.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x673e38);
  (this->super_C_BaseCombatCharacter).super_C_BaseFlex.super_C_BaseAnimatingOverlay.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(&UNK_00673e74 + unaff_EBX);
  (this->super_C_BaseCombatCharacter).super_C_BaseFlex.super_C_BaseAnimatingOverlay.
  super_C_BaseAnimating.super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(&UNK_00673e90 + unaff_EBX);
  (**(code **)(&UNK_006739a0 + unaff_EBX))(this,entnum,serialNum);
  return &(this->super_C_BaseCombatCharacter).super_C_BaseFlex.super_C_BaseAnimatingOverlay.
          super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable;
}


/* ClientClassInit<DT_NPC_HoverTurret::ignored> at 00074a90 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

int ClientClassInit<DT_NPC_HoverTurret::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x505].m_pEntity + unaff_EBX)
      == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc431b4);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  (&UNK_0000505c +
                  (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                  (char *)(unaff_EBX + 0x8eb2bc),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x509].
                               m_SerialNumber + unaff_EBX),(char *)(unaff_EBX + 0x8efe9c),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00abc944 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00abc2fc + unaff_EBX));
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x50d].m_pEntity +
                  unaff_EBX),(char *)(unaff_EBX + 0x916b34),0x12d4,4,0,(RecvVarProxyFn)0x0);
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x510].m_pNext + unaff_EBX
                  ),(char *)(unaff_EBX + 0x916abd),0x12d8,4,0,(RecvVarProxyFn)0x0);
      ___cxa_guard_release(unaff_EBX + 0xc431b4);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x848),
             (RecvProp_conflict1 *)
             ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x509].m_SerialNumber +
             unaff_EBX),3,(char *)(unaff_EBX + 0x91a2dd));
  return 1;
}


/* __static_initialization_and_destruction_0 at 00074c20 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

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
  (&UNK_00004e8f)[(int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
  (&UNK_00004e90)[(int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
  (&UNK_00004e91)[(int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
  (&UNK_00004e92)[(int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
  *(undefined4 *)
   (&UNK_00004e93 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_00004e97 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_00004e9b + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       0x7f7fffff;
  *(undefined4 *)
   (&UNK_00004e9f + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       0x7f7fffff;
  *(undefined4 *)
   (&UNK_00004ea3 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_00004ea7 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_00004eab + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_00004eaf + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_00004eb3 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       0x7f7fffff;
  *(undefined4 *)
   (&UNK_00004eb7 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       0x7f7fffff;
  *(undefined4 *)
   (&UNK_00004ebb + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       0x7f7fffff;
  *(undefined4 *)
   (&UNK_00004ebf + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       0x7f7fffff;
  *(undefined **)
   (&UNK_00004ec3 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       &UNK_00abd1d7 + unaff_EBX;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x6ab) = unaff_EBX + 0x91a163;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x6a3) = unaff_EBX + 0x4adcbf;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0x6a7) = 0;
  this = (RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x6bb);
  *(RecvTable **)((int)s_pParticlePtrs + unaff_EBX + 0x6af) = this;
  piVar1 = *(int **)(&DAT_00abc167 + unaff_EBX);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x6b3) = *piVar1;
  *piVar1 = unaff_EBX + 0xcd9f03;
  RecvTable::RecvTable(this);
  ___cxa_atexit(unaff_EBX + 0x8dc65f,0,*(undefined4 *)(&DAT_00abc0ef + unaff_EBX));
  iVar2 = ClientClassInit<DT_NPC_HoverTurret::ignored>((ignored *)0x0);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x6cf) = iVar2;
  return;
}


/* C_NPC_Hover_Turret::YouForgotToImplementOrDeclareClientClass at 005228c0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_NPC_Hover_Turret * this)
    */

int __thiscall
C_NPC_Hover_Turret::YouForgotToImplementOrDeclareClientClass(C_NPC_Hover_Turret *this)

{
  return 0;
}


/* C_NPC_Hover_Turret::GetClientClass at 005228e0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: ClientClass * GetClientClass(C_NPC_Hover_Turret * this) */

ClientClass * __thiscall C_NPC_Hover_Turret::GetClientClass(C_NPC_Hover_Turret *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(&UNK_0082c24c + extraout_ECX);
}


/* __tcf_0 at 00951290 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3fd8b4),in_stack_00000008);
  return;
}


/* C_NPC_Hover_Turret::~C_NPC_Hover_Turret at 00523080 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void ~C_NPC_Hover_Turret(C_NPC_Hover_Turret * this, int __in_chrg) */

void __thiscall C_NPC_Hover_Turret::~C_NPC_Hover_Turret(C_NPC_Hover_Turret *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  *(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
           super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity
       = unaff_EBX + 0x673154;
  ((IClientRenderable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  4))->_vptr_IClientRenderable = (_func_int_varargs **)(&UNK_006735f0 + unaff_EBX);
  ((IClientNetworkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  8))->_vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x6736a8);
  ((IClientThinkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  0xc))->_vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x6736e4);
  (this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
  super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(unaff_EBX + 0x673700);
  if (this->m_pBeam != (C_Beam *)0x0) {
    in_stack_ffffffd8 = 0x20;
    C_BaseEntity::AddEffects(&this->m_pBeam->super_C_BaseEntity,0x20);
    if (this->m_pBeam != (C_Beam *)0x0) {
      C_BaseEntity::Remove(&this->m_pBeam->super_C_BaseEntity);
    }
  }
  *(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
           super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity
       = *(int *)(unaff_EBX + 0x60e350) + 8;
  ((IClientRenderable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  4))->_vptr_IClientRenderable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x60e350) + 0x4a4);
  ((IClientNetworkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  8))->_vptr_IClientNetworkable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x60e350) + 0x55c);
  ((IClientThinkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  0xc))->_vptr_IClientThinkable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x60e350) + 0x598);
  (this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
  super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x60e350) + 0x5b4);
  C_BaseCombatCharacter::~C_BaseCombatCharacter((C_BaseCombatCharacter *)this,in_stack_ffffffd8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_NPC_Hover_Turret::~C_NPC_Hover_Turret at 005231d0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void ~C_NPC_Hover_Turret(C_NPC_Hover_Turret * this, int __in_chrg) */

void __thiscall C_NPC_Hover_Turret::~C_NPC_Hover_Turret(C_NPC_Hover_Turret *this,int __in_chrg)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  *(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
           super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity
       = unaff_EBX + 0x673004;
  ((IClientRenderable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  4))->_vptr_IClientRenderable = (_func_int_varargs **)(&UNK_006734a0 + unaff_EBX);
  ((IClientNetworkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  8))->_vptr_IClientNetworkable = (_func_int_varargs **)(&UNK_00673558 + unaff_EBX);
  ((IClientThinkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  0xc))->_vptr_IClientThinkable = (_func_int_varargs **)(&UNK_00673594 + unaff_EBX);
  (this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
  super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(&UNK_006735b0 + unaff_EBX);
  if (this->m_pBeam != (C_Beam *)0x0) {
    C_BaseEntity::AddEffects(&this->m_pBeam->super_C_BaseEntity,0x20);
    if (this->m_pBeam != (C_Beam *)0x0) {
      C_BaseEntity::Remove(&this->m_pBeam->super_C_BaseEntity);
    }
  }
  *(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
           super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity
       = *(int *)(unaff_EBX + 0x60e200) + 8;
  ((IClientRenderable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  4))->_vptr_IClientRenderable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x60e200) + 0x4a4);
  ((IClientNetworkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  8))->_vptr_IClientNetworkable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x60e200) + 0x55c);
  ((IClientThinkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  0xc))->_vptr_IClientThinkable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x60e200) + 0x598);
  (this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
  super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x60e200) + 0x5b4);
  C_BaseCombatCharacter::~C_BaseCombatCharacter((C_BaseCombatCharacter *)this,__in_chrg);
  return;
}


/* C_NPC_Hover_Turret::~C_NPC_Hover_Turret at 00523310 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void ~C_NPC_Hover_Turret(C_NPC_Hover_Turret * this, int __in_chrg) */

void __thiscall C_NPC_Hover_Turret::~C_NPC_Hover_Turret(C_NPC_Hover_Turret *this,int __in_chrg)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  *(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
           super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity
       = unaff_EBX + 0x672ec4;
  ((IClientRenderable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  4))->_vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x673360);
  ((IClientNetworkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  8))->_vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x673418);
  ((IClientThinkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  0xc))->_vptr_IClientThinkable = (_func_int_varargs **)(&UNK_00673454 + unaff_EBX);
  (this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
  super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(&UNK_00673470 + unaff_EBX);
  if (this->m_pBeam != (C_Beam *)0x0) {
    C_BaseEntity::AddEffects(&this->m_pBeam->super_C_BaseEntity,0x20);
    if (this->m_pBeam != (C_Beam *)0x0) {
      C_BaseEntity::Remove(&this->m_pBeam->super_C_BaseEntity);
    }
  }
  *(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
           super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity
       = *(int *)(unaff_EBX + 0x60e0c0) + 8;
  ((IClientRenderable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  4))->_vptr_IClientRenderable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x60e0c0) + 0x4a4);
  ((IClientNetworkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  8))->_vptr_IClientNetworkable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x60e0c0) + 0x55c);
  ((IClientThinkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  0xc))->_vptr_IClientThinkable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x60e0c0) + 0x598);
  (this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
  super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x60e0c0) + 0x5b4);
  C_BaseCombatCharacter::~C_BaseCombatCharacter((C_BaseCombatCharacter *)this,__in_chrg);
  return;
}


/* C_NPC_Hover_Turret::Spawn at 00522a70 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(C_NPC_Hover_Turret * this) */

void __thiscall C_NPC_Hover_Turret::Spawn(C_NPC_Hover_Turret *this)

{
  longdouble lVar1;
  
  C_BaseEntity::ThinkSet((C_BaseEntity *)this,(BASEPTR)0x151,0.0,(char *)0x0);
  (**(code **)(*(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
                        super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.
                        super_IClientEntity + 0x26c))(this,0xc4a1a000);
  this->m_pBeam = (C_Beam *)0x0;
  lVar1 = (longdouble)_RandomFloat(0,0x40c90fdb);
  this->m_fPulseOffset = (float)lVar1;
  this->m_bBeamFlickerOff = 0.0;
  this->m_fBeamFlickerTime = 0.0;
  C_BaseFlex::Spawn((C_BaseFlex *)this);
  return;
}


/* C_NPC_Hover_Turret::ClientThink at 00523040 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void ClientThink(C_NPC_Hover_Turret * this) */

void __thiscall C_NPC_Hover_Turret::ClientThink(C_NPC_Hover_Turret *this)

{
  if (0 < this->m_iLaserState) {
    LaserOn(this);
    return;
  }
  if (this->m_pBeam != (C_Beam *)0x0) {
    C_BaseEntity::AddEffects(&this->m_pBeam->super_C_BaseEntity,0x20);
  }
  return;
}


/* C_NPC_Hover_Turret::LaserOff at 00522a40 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void LaserOff(C_NPC_Hover_Turret * this) */

void __thiscall C_NPC_Hover_Turret::LaserOff(C_NPC_Hover_Turret *this)

{
  if (this->m_pBeam != (C_Beam *)0x0) {
    C_BaseEntity::AddEffects(&this->m_pBeam->super_C_BaseEntity,0x20);
  }
  return;
}


/* C_NPC_Hover_Turret::LaserOn at 00522b00 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void LaserOn(C_NPC_Hover_Turret * this) */

void __thiscall C_NPC_Hover_Turret::LaserOn(C_NPC_Hover_Turret *this)

{
  int *piVar1;
  bool bVar2;
  int iVar3;
  C_Portal_Base2D *pCVar4;
  C_Beam *pCVar5;
  int unaff_EBX;
  longdouble lVar6;
  float fVar7;
  Ray_t local_11c;
  CTraceFilterSkipClassname local_c4;
  float local_b0;
  float local_ac;
  float local_a8;
  float local_a4;
  float local_a0;
  float local_9c;
  float local_98;
  float local_94;
  float local_90;
  float local_8c;
  float local_88;
  float local_84;
  float local_80;
  float local_7c;
  float local_78;
  float local_74;
  float local_70;
  float local_6c;
  Vector local_68;
  Vector local_5c;
  QAngle local_50;
  undefined1 local_44 [12];
  undefined1 local_38 [12];
  Vector local_2c;
  float local_20 [4];
  
                    /* Unresolved local var: Vector vecMuzzle@[???]
                       Unresolved local var: QAngle angMuzzleDir@[???]
                       Unresolved local var: Vector vecEye@[???]
                       Unresolved local var: QAngle angEyeDir@[???]
                       Unresolved local var: Vector vecMuzzleDir@[???]
                       Unresolved local var: Vector vEndPoint@[???]
                       Unresolved local var: float fEndFraction@[???]
                       Unresolved local var: Ray_t rayPath@[???]
                       Unresolved local var: CTraceFilterSkipClassname traceFilter@[???] */
  ___i686_get_pc_thunk_bx();
  bVar2 = C_BaseAnimating::IsBoneAccessAllowed((C_BaseAnimating *)this);
  if (!bVar2) {
    if (this->m_pBeam == (C_Beam *)0x0) {
      return;
    }
    C_BaseEntity::AddEffects(&this->m_pBeam->super_C_BaseEntity,0x20);
    return;
  }
  (**(code **)(*(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
                        super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.
                        super_IClientEntity + 0x1d0))(this,2,&local_2c,local_38);
  (**(code **)(*(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
                        super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.
                        super_IClientEntity + 0x1d0))(this,1,local_44,&local_50);
  AngleVectors(&local_50,&local_5c);
  iVar3 = this->m_iLastLaserState;
  if (iVar3 == this->m_iLaserState) {
    pCVar5 = this->m_pBeam;
    if (pCVar5 != (C_Beam *)0x0) {
      piVar1 = &(pCVar5->super_C_BaseEntity).m_fEffects;
      *piVar1 = *piVar1 & 0xffffffdf;
      C_BaseEntity::UpdateVisibility(&pCVar5->super_C_BaseEntity);
      goto LAB_00522bed;
    }
  }
  else {
    pCVar5 = this->m_pBeam;
    if (pCVar5 != (C_Beam *)0x0) {
      (*(pCVar5->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
        _vptr_IHandleEntity[1])(pCVar5);
      this->m_pBeam = (C_Beam *)0x0;
    }
    iVar3 = this->m_iLaserState;
  }
  if (iVar3 == 1) {
    pCVar5 = C_Beam::BeamCreate((char *)(unaff_EBX + 0x468b1a),1.0);
    this->m_pBeam = pCVar5;
    if ((pCVar5->super_C_BaseEntity).m_clrRender.
        super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.r != 'Z') {
      (pCVar5->super_C_BaseEntity).m_clrRender.
      super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.r = 'Z';
    }
    if ((pCVar5->super_C_BaseEntity).m_clrRender.
        super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.g != 0xff) {
      (pCVar5->super_C_BaseEntity).m_clrRender.
      super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.g = 0xff;
    }
    if ((pCVar5->super_C_BaseEntity).m_clrRender.
        super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.b != 0xff) {
      (pCVar5->super_C_BaseEntity).m_clrRender.
      super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.b = 0xff;
    }
    pCVar5 = this->m_pBeam;
    if (this->m_sLaserHaloSprite != (pCVar5->m_nHaloIndex).m_Value) {
      (pCVar5->m_nHaloIndex).m_Value = this->m_sLaserHaloSprite;
      pCVar5 = this->m_pBeam;
    }
    if (*(float *)(unaff_EBX + 0x493e93) != (pCVar5->m_fHaloScale).m_Value) {
      (pCVar5->m_fHaloScale).m_Value = *(float *)(unaff_EBX + 0x493e93);
    }
  }
  else if (iVar3 == 2) {
    pCVar5 = C_Beam::BeamCreate((char *)(unaff_EBX + 0x46c294),12.0);
    this->m_pBeam = pCVar5;
    (*(pCVar5->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
      _vptr_IHandleEntity[0x27])(pCVar5);
    pCVar5 = this->m_pBeam;
    fVar7 = *(float *)(unaff_EBX + 0x493e47);
    if (fVar7 != (pCVar5->m_fWidth).m_Value) {
      (pCVar5->m_fWidth).m_Value = fVar7;
      pCVar5 = this->m_pBeam;
    }
    if (fVar7 != (pCVar5->m_fEndWidth).m_Value) {
      (pCVar5->m_fEndWidth).m_Value = 16.0;
    }
  }
LAB_00522bed:
  this->m_iLastLaserState = this->m_iLaserState;
                    /* Unresolved local var: Vector res@[???] */
  fVar7 = *(float *)(unaff_EBX + 0x498537);
  local_74 = local_5c.x * fVar7;
  local_70 = local_5c.y * fVar7;
  local_6c = local_5c.z * fVar7;
                    /* Unresolved local var: Vector res@[???] */
  local_80 = local_2c.x + local_74;
  local_7c = local_2c.y + local_70;
  local_78 = local_2c.z + local_6c;
  local_11c.m_Delta.super_Vector.x = local_80 - local_2c.x;
  local_11c.m_Delta.super_Vector.y = local_7c - local_2c.y;
  local_11c.m_Delta.super_Vector.z = local_78 - local_2c.z;
  local_11c.m_IsSwept =
       local_11c.m_Delta.super_Vector.x * local_11c.m_Delta.super_Vector.x +
       local_11c.m_Delta.super_Vector.y * local_11c.m_Delta.super_Vector.y +
       local_11c.m_Delta.super_Vector.z * local_11c.m_Delta.super_Vector.z != 0.0;
  local_11c.m_Extents.super_Vector.z = 0.0;
  local_11c.m_Extents.super_Vector.y = 0.0;
  local_11c.m_Extents.super_Vector.x = 0.0;
  local_11c.m_pWorldAxisTransform = (matrix3x4_t *)0x0;
  local_11c.m_IsRay = true;
  local_11c.m_StartOffset.super_Vector.z = 0.0;
  local_11c.m_StartOffset.super_Vector.y = 0.0;
  local_11c.m_StartOffset.super_Vector.x = 0.0;
  local_11c.m_Start.super_Vector.x = local_2c.x;
  local_11c.m_Start.super_Vector.y = local_2c.y;
  local_11c.m_Start.super_Vector.z = local_2c.z;
  CTraceFilterSkipClassname::CTraceFilterSkipClassname
            (&local_c4,(IHandleEntity *)this,(char *)(unaff_EBX + 0x43d1a3),0);
  pCVar4 = UTIL_Portal_TraceRay_Beam(&local_11c,0x46004003,(ITraceFilter *)&local_c4,local_20);
  if (pCVar4 == (C_Portal_Base2D *)0x0) {
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
    local_a4 = fVar7 * local_5c.x * local_20[0];
    local_a0 = local_5c.y * fVar7 * local_20[0];
    local_9c = local_20[0] * local_5c.z * fVar7;
                    /* Unresolved local var: Vector res@[???] */
    local_b0 = local_2c.x + local_a4;
    local_ac = local_2c.y + local_a0;
    local_a8 = local_2c.z + local_9c;
    local_68.x = local_b0;
    local_68.y = local_ac;
    local_68.z = local_a8;
  }
  else {
                    /* Unresolved local var: Vector res@[???] */
    local_8c = local_5c.x * fVar7;
    local_88 = local_5c.y * fVar7;
    local_84 = fVar7 * local_5c.z;
                    /* Unresolved local var: Vector res@[???] */
    local_98 = local_2c.x + local_8c;
    local_94 = local_2c.y + local_88;
    local_90 = local_2c.z + local_84;
    local_68.x = local_98;
    local_68.y = local_94;
    local_68.z = local_90;
  }
  C_Beam::PointsInit(this->m_pBeam,&local_68,&local_2c);
  lVar6 = (longdouble)
          _sinf((float)((double)*(float *)(**(int **)(unaff_EBX + 0x60e23f) + 0xc) *
                        *(double *)(unaff_EBX + 0x49411f) + (double)this->m_fPulseOffset));
  if (0.0 <= (float)lVar6) {
    fVar7 = ((float)lVar6 * *(float *)(unaff_EBX + 0x493e4b) + *(float *)(unaff_EBX + 0x4940ff)) *
            *(float *)(unaff_EBX + 0x493e93);
  }
  else {
    fVar7 = *(float *)(unaff_EBX + 0x4973ef);
  }
  if (fVar7 == (this->m_pBeam->m_fHaloScale).m_Value) {
    return;
  }
  (this->m_pBeam->m_fHaloScale).m_Value = fVar7;
  return;
}


/* C_NPC_Hover_Turret::LaserEndPointSize at 005229a0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: float LaserEndPointSize(C_NPC_Hover_Turret * this) */

float __thiscall C_NPC_Hover_Turret::LaserEndPointSize(C_NPC_Hover_Turret *this)

{
  float fVar1;
  int unaff_EBX;
  longdouble lVar2;
  
  ___i686_get_pc_thunk_bx();
  lVar2 = (longdouble)
          _sinf((float)((double)*(float *)(**(int **)(unaff_EBX + 0x60e3a4) + 0xc) *
                        *(double *)(unaff_EBX + 0x494284) + (double)this->m_fPulseOffset));
  fVar1 = (float)lVar2;
  if ((fVar1 < *(float *)(unaff_EBX + 0x493fa8)) &&
     (!NAN(fVar1) && !NAN(*(float *)(unaff_EBX + 0x493fa8)))) {
    return *(float *)(unaff_EBX + 0x497554);
  }
  return (fVar1 * *(float *)(unaff_EBX + 0x493fb0) + *(float *)(unaff_EBX + 0x494264)) *
         *(float *)(unaff_EBX + 0x493ff8);
}


/* _GLOBAL__I__ZN18C_NPC_Hover_Turret17m_pClassRecvTableE at 00074d40 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN18C_NPC_Hover_Turret17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

