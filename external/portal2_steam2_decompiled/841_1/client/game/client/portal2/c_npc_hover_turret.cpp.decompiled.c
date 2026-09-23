/* DWARF-guided pseudocode for game/client/portal2/c_npc_hover_turret.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* _C_NPC_Hover_Turret_CreateObject at 0051d330 */

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
       (IClientUnknown)(unaff_EBX + 0x6acb64);
  (this->super_C_BaseCombatCharacter).super_C_BaseFlex.super_C_BaseAnimatingOverlay.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(&UNK_006ad004 + unaff_EBX);
  (this->super_C_BaseCombatCharacter).super_C_BaseFlex.super_C_BaseAnimatingOverlay.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x6ad0bc);
  (this->super_C_BaseCombatCharacter).super_C_BaseFlex.super_C_BaseAnimatingOverlay.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x6ad0f8);
  (this->super_C_BaseCombatCharacter).super_C_BaseFlex.super_C_BaseAnimatingOverlay.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientModelRenderable._vptr_IClientModelRenderable
       = (_func_int_varargs **)(unaff_EBX + 0x6ad114);
  (**(code **)(&UNK_006acc20 + unaff_EBX))(this,entnum,serialNum);
  return &(this->super_C_BaseCombatCharacter).super_C_BaseFlex.super_C_BaseAnimatingOverlay.
          super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable;
}


/* ClientClassInit<DT_NPC_HoverTurret::ignored> at 00074960 */

int ClientClassInit<DT_NPC_HoverTurret::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)s_randomGradients + unaff_EBX + 0xac4) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc5c3c4);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)((int)s_randomGradients + unaff_EBX + 0xacc),
                  (char *)(unaff_EBX + 0x915dfc),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)((int)s_randomGradients + unaff_EBX + 0xb08),
                        (char *)(unaff_EBX + 0x91a940),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00aefac4 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00aef478 + unaff_EBX));
      RecvPropInt((RecvProp_conflict1 *)((int)s_randomGradients + unaff_EBX + 0xb44),
                  (char *)(unaff_EBX + 0x941560),0x12d4,4,0,(RecvVarProxyFn)0x0);
      RecvPropInt((RecvProp_conflict1 *)((int)s_randomGradients + unaff_EBX + 0xb80),
                  (char *)(unaff_EBX + 0x9414d9),0x12d8,4,0,(RecvVarProxyFn)0x0);
      ___cxa_guard_release(unaff_EBX + 0xc5c3c4);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0xaa8),
             (RecvProp_conflict1 *)((int)s_randomGradients + unaff_EBX + 0xb08),3,
             (char *)(unaff_EBX + 0x944d55));
  return 1;
}


/* __static_initialization_and_destruction_0 at 00074af0 */

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
  *(undefined1 *)((int)s_randomGradients + unaff_EBX + 0x8ff) = 0;
  *(undefined1 *)((int)s_randomGradients + unaff_EBX + 0x900) = 0;
  *(undefined1 *)((int)s_randomGradients + unaff_EBX + 0x901) = 0;
  *(undefined1 *)((int)s_randomGradients + unaff_EBX + 0x902) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x903) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x907) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x90b) = 0x7f7fffff;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x90f) = 0x7f7fffff;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x913) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x917) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x91b) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x91f) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x923) = 0x7f7fffff;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x927) = 0x7f7fffff;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x92b) = 0x7f7fffff;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0x92f) = 0x7f7fffff;
  *(undefined **)((int)s_randomGradients + unaff_EBX + 0x933) = &UNK_00af0387 + unaff_EBX;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x90b) = unaff_EBX + 0x944bdb;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x903) = unaff_EBX + 0x4a882f;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0x907) = 0;
  this = (RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x91b);
  *(RecvTable **)((int)s_pParticlePtrs + unaff_EBX + 0x90f) = this;
  piVar1 = *(int **)(&DAT_00aef2e3 + unaff_EBX);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x913) = *piVar1;
  *piVar1 = unaff_EBX + 0xcf34c3;
  RecvTable::RecvTable(this);
  ___cxa_atexit(unaff_EBX + 0x906c6f,0,*(undefined4 *)(&DAT_00aef26b + unaff_EBX));
  iVar2 = ClientClassInit<DT_NPC_HoverTurret::ignored>((ignored *)0x0);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x92f) = iVar2;
  return;
}


/* C_NPC_Hover_Turret::YouForgotToImplementOrDeclareClientClass at 0051d300 */

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


/* C_NPC_Hover_Turret::GetClientClass at 0051d320 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: ClientClass * GetClientClass(C_NPC_Hover_Turret * this) */

ClientClass * __thiscall C_NPC_Hover_Turret::GetClientClass(C_NPC_Hover_Turret *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x84ac9c);
}


/* __tcf_0 at 0097b770 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3ec864),in_stack_00000008);
  return;
}


/* C_NPC_Hover_Turret::~C_NPC_Hover_Turret at 0051dad0 */

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
       = unaff_EBX + 0x6ac3c4;
  ((IClientRenderable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  4))->_vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x6ac864);
  ((IClientNetworkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  8))->_vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x6ac91c);
  ((IClientThinkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  0xc))->_vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x6ac958);
  (this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
  super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientModelRenderable
  ._vptr_IClientModelRenderable = (_func_int_varargs **)(unaff_EBX + 0x6ac974);
  if (this->m_pBeam != (C_Beam *)0x0) {
    in_stack_ffffffd8 = 0x20;
    C_BaseEntity::AddEffects(&this->m_pBeam->super_C_BaseEntity,0x20);
    if (this->m_pBeam != (C_Beam *)0x0) {
      C_BaseEntity::Remove(&this->m_pBeam->super_C_BaseEntity);
    }
  }
  *(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
           super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity
       = *(int *)(unaff_EBX + 0x646950) + 8;
  ((IClientRenderable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  4))->_vptr_IClientRenderable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x646950) + 0x4a8);
  ((IClientNetworkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  8))->_vptr_IClientNetworkable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x646950) + 0x560);
  ((IClientThinkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  0xc))->_vptr_IClientThinkable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x646950) + 0x59c);
  (this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
  super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientModelRenderable
  ._vptr_IClientModelRenderable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x646950) + 0x5b8);
  C_BaseCombatCharacter::~C_BaseCombatCharacter((C_BaseCombatCharacter *)this,in_stack_ffffffd8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_NPC_Hover_Turret::~C_NPC_Hover_Turret at 0051dc10 */

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
       = unaff_EBX + 0x6ac284;
  ((IClientRenderable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  4))->_vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x6ac724);
  ((IClientNetworkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  8))->_vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x6ac7dc);
  ((IClientThinkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  0xc))->_vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x6ac818);
  (this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
  super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientModelRenderable
  ._vptr_IClientModelRenderable = (_func_int_varargs **)(unaff_EBX + 0x6ac834);
  if (this->m_pBeam != (C_Beam *)0x0) {
    C_BaseEntity::AddEffects(&this->m_pBeam->super_C_BaseEntity,0x20);
    if (this->m_pBeam != (C_Beam *)0x0) {
      C_BaseEntity::Remove(&this->m_pBeam->super_C_BaseEntity);
    }
  }
  *(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
           super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity
       = *(int *)(unaff_EBX + 0x646810) + 8;
  ((IClientRenderable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  4))->_vptr_IClientRenderable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x646810) + 0x4a8);
  ((IClientNetworkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  8))->_vptr_IClientNetworkable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x646810) + 0x560);
  ((IClientThinkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  0xc))->_vptr_IClientThinkable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x646810) + 0x59c);
  (this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
  super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientModelRenderable
  ._vptr_IClientModelRenderable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x646810) + 0x5b8);
  C_BaseCombatCharacter::~C_BaseCombatCharacter((C_BaseCombatCharacter *)this,__in_chrg);
  return;
}


/* C_NPC_Hover_Turret::~C_NPC_Hover_Turret at 0051dd50 */

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
       = unaff_EBX + 0x6ac144;
  ((IClientRenderable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  4))->_vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x6ac5e4);
  ((IClientNetworkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  8))->_vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x6ac69c);
  ((IClientThinkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  0xc))->_vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x6ac6d8);
  (this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
  super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientModelRenderable
  ._vptr_IClientModelRenderable = (_func_int_varargs **)(unaff_EBX + 0x6ac6f4);
  if (this->m_pBeam != (C_Beam *)0x0) {
    C_BaseEntity::AddEffects(&this->m_pBeam->super_C_BaseEntity,0x20);
    if (this->m_pBeam != (C_Beam *)0x0) {
      C_BaseEntity::Remove(&this->m_pBeam->super_C_BaseEntity);
    }
  }
  *(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
           super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity
       = *(int *)(CToolFrameworkClient::Init + unaff_EBX) + 8;
  ((IClientRenderable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  4))->_vptr_IClientRenderable =
       (_func_int_varargs **)(*(int *)(CToolFrameworkClient::Init + unaff_EBX) + 0x4a8);
  ((IClientNetworkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  8))->_vptr_IClientNetworkable =
       (_func_int_varargs **)(*(int *)(CToolFrameworkClient::Init + unaff_EBX) + 0x560);
  ((IClientThinkable *)
  ((int)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
         super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity +
  0xc))->_vptr_IClientThinkable =
       (_func_int_varargs **)(*(int *)(CToolFrameworkClient::Init + unaff_EBX) + 0x59c);
  (this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
  super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.super_IClientModelRenderable
  ._vptr_IClientModelRenderable =
       (_func_int_varargs **)(*(int *)(CToolFrameworkClient::Init + unaff_EBX) + 0x5b8);
  C_BaseCombatCharacter::~C_BaseCombatCharacter((C_BaseCombatCharacter *)this,__in_chrg);
  return;
}


/* C_NPC_Hover_Turret::Spawn at 0051d4b0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(C_NPC_Hover_Turret * this) */

void __thiscall C_NPC_Hover_Turret::Spawn(C_NPC_Hover_Turret *this)

{
  longdouble lVar1;
  
  C_BaseEntity::ThinkSet((C_BaseEntity *)this,(BASEPTR)0x155,0.0,(char *)0x0);
  (**(code **)(*(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
                        super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.
                        super_IClientEntity + 0x274))(this,0xc4a1a000);
  this->m_pBeam = (C_Beam *)0x0;
  lVar1 = (longdouble)_RandomFloat(0,0x40c90fdb);
  this->m_fPulseOffset = (float)lVar1;
  this->m_bBeamFlickerOff = 0.0;
  this->m_fBeamFlickerTime = 0.0;
  C_BaseFlex::Spawn((C_BaseFlex *)this);
  return;
}


/* C_NPC_Hover_Turret::ClientThink at 0051da90 */

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


/* C_NPC_Hover_Turret::LaserOff at 0051d480 */

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


/* C_NPC_Hover_Turret::LaserOn at 0051d540 */

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
                        super_IClientEntity + 0x1d4))(this,2,&local_2c,local_38);
  (**(code **)(*(int *)&(this->super_C_AI_BaseNPC).super_C_BaseCombatCharacter.super_C_BaseFlex.
                        super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.
                        super_IClientEntity + 0x1d4))(this,1,local_44,&local_50);
  AngleVectors(&local_50,&local_5c);
  iVar3 = this->m_iLastLaserState;
  if (iVar3 == this->m_iLaserState) {
    pCVar5 = this->m_pBeam;
    if (pCVar5 != (C_Beam *)0x0) {
      piVar1 = &(pCVar5->super_C_BaseEntity).m_fEffects;
      *piVar1 = *piVar1 & 0xffffffdf;
      C_BaseEntity::UpdateVisibility(&pCVar5->super_C_BaseEntity);
      C_BaseEntity::OnDisableShadowDepthRenderingChanged(&pCVar5->super_C_BaseEntity);
      C_BaseEntity::OnShadowDepthRenderingCacheableStateChanged(&pCVar5->super_C_BaseEntity);
      goto LAB_0051d62d;
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
    pCVar5 = C_Beam::BeamCreate((char *)(unaff_EBX + 0x4989d6),1.0);
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
    if (*(float *)(unaff_EBX + 0x4c4d93) != (pCVar5->m_fHaloScale).m_Value) {
      (pCVar5->m_fHaloScale).m_Value = *(float *)(unaff_EBX + 0x4c4d93);
    }
  }
  else if (iVar3 == 2) {
    pCVar5 = C_Beam::BeamCreate((char *)(unaff_EBX + 0x49c19c),12.0);
    this->m_pBeam = pCVar5;
    (*(pCVar5->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
      _vptr_IHandleEntity[0x27])(pCVar5);
    pCVar5 = this->m_pBeam;
    fVar7 = *(float *)(unaff_EBX + 0x4c4d33);
    if (fVar7 != (pCVar5->m_fWidth).m_Value) {
      (pCVar5->m_fWidth).m_Value = fVar7;
      pCVar5 = this->m_pBeam;
    }
    if (fVar7 != (pCVar5->m_fEndWidth).m_Value) {
      (pCVar5->m_fEndWidth).m_Value = 16.0;
    }
  }
LAB_0051d62d:
  this->m_iLastLaserState = this->m_iLaserState;
                    /* Unresolved local var: Vector res@[???] */
  fVar7 = *(float *)(unaff_EBX + 0x4c93ff);
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
            (&local_c4,(IHandleEntity *)this,(char *)(unaff_EBX + 0x498957),0);
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
          _sinf((float)((double)*(float *)(**(int **)(unaff_EBX + 0x64684b) + 0xc) *
                        *(double *)(unaff_EBX + 0x4c501f) + (double)this->m_fPulseOffset));
  if (0.0 <= (float)lVar6) {
    fVar7 = ((float)lVar6 * *(float *)(unaff_EBX + 0x4c4d4b) + *(float *)(unaff_EBX + 0x4c4fff)) *
            *(float *)(unaff_EBX + 0x4c4d93);
  }
  else {
    fVar7 = *(float *)(unaff_EBX + 0x4c82ef);
  }
  if (fVar7 == (this->m_pBeam->m_fHaloScale).m_Value) {
    return;
  }
  (this->m_pBeam->m_fHaloScale).m_Value = fVar7;
  return;
}


/* C_NPC_Hover_Turret::LaserEndPointSize at 0051d3e0 */

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
          _sinf((float)((double)*(float *)(**(int **)(unaff_EBX + 0x6469b0) + 0xc) *
                        *(double *)(unaff_EBX + 0x4c5184) + (double)this->m_fPulseOffset));
  fVar1 = (float)lVar2;
  if ((fVar1 < *(float *)(unaff_EBX + 0x4c4e94)) &&
     (!NAN(fVar1) && !NAN(*(float *)(unaff_EBX + 0x4c4e94)))) {
    return *(float *)(unaff_EBX + 0x4c8454);
  }
  return (fVar1 * *(float *)(unaff_EBX + 0x4c4eb0) + *(float *)(unaff_EBX + 0x4c5164)) *
         *(float *)(unaff_EBX + 0x4c4ef8);
}


/* _GLOBAL__I__ZN18C_NPC_Hover_Turret17m_pClassRecvTableE at 00074c10 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN18C_NPC_Hover_Turret17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

