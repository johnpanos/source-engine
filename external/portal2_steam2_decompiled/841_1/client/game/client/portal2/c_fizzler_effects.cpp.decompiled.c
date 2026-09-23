/* DWARF-guided pseudocode for game/client/portal2/c_fizzler_effects.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 00073660 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  int *piVar1;
  IClassMap *pIVar2;
  int iVar3;
  int unaff_EBX;
  longlong lVar4;
  
  lVar4 = ___i686_get_pc_thunk_bx();
  if (lVar4 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x5cf) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x5d0) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x5d1) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x5d2) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x5d3) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x5d7) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x5db) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x5df) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x5e3) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x5e7) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x5eb) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x5ef) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x5f3) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x5f7) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x5fb) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x5ff) = 0x7f7fffff;
  *(undefined **)((int)s_pRandomFloats + unaff_EBX + 0x603) = &UNK_00af1817 + unaff_EBX;
  pIVar2 = GetClassMap();
  (*pIVar2->_vptr_IClassMap[2])
            (pIVar2,unaff_EBX + 0x945d1a,unaff_EBX + 0x945d09,0xa60,unaff_EBX + 0x4a7baf);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x1c5b) = unaff_EBX + 0x945d2e;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x1c53) = unaff_EBX + 0x4a7c3f;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0x1c57) = 0;
  *(RecvTable **)((int)s_pParticlePtrs + unaff_EBX + 0x1c5f) =
       (RecvTable *)((int)s_pParticlePtrs + (int)(&UNK_00001c6b + unaff_EBX));
  piVar1 = *(int **)(&DAT_00af0773 + unaff_EBX);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x1c63) = *piVar1;
  *piVar1 = unaff_EBX + 0xcf4813;
  RecvTable::RecvTable((RecvTable *)((int)s_pParticlePtrs + (int)(&UNK_00001c6b + unaff_EBX)));
  ___cxa_atexit(unaff_EBX + 0x907f2f,0,*(undefined4 *)(&DAT_00af06fb + unaff_EBX));
  iVar3 = ClientClassInit<DT_FizzlerEffects::ignored>((ignored *)0x0);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x1c7f) = iVar3;
  return;
}


/* CC_FizzlerEffectsFactory at 0051b220 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

C_BaseEntity * CC_FizzlerEffectsFactory(void)

{
  C_BaseEntity *this;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xa60);
  C_BaseEntity::C_BaseEntity(this);
  (this->super_IClientEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x6acd74);
  (this->super_IClientEntity).super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6ad110);
  (this->super_IClientEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6ad1c8);
  (this->super_IClientEntity).super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6ad204);
  (this->super_IClientModelRenderable)._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6ad220);
  return this;
}


/* ClientClassInit<DT_FizzlerEffects::ignored> at 00073540 */

int ClientClassInit<DT_FizzlerEffects::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)s_pRandomFloats + unaff_EBX + 0x724) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc5ce24);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x72c),
                  (char *)(unaff_EBX + 0x91721c),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x768),
                        (char *)(unaff_EBX + 0x91bd60),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00af08dc + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00af0898 + unaff_EBX));
      ___cxa_guard_release(unaff_EBX + 0xc5ce24);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x1d88),
             (RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x768),1,
             (char *)(unaff_EBX + 0x945e14));
  return 1;
}


/* C_FizzlerEffects::YouForgotToImplementOrDeclareClientClass at 0051b1f0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_FizzlerEffects * this)
    */

int __thiscall C_FizzlerEffects::YouForgotToImplementOrDeclareClientClass(C_FizzlerEffects *this)

{
  return 0;
}


/* C_FizzlerEffects::GetClientClass at 0051b210 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: ClientClass * GetClientClass(C_FizzlerEffects * this) */

ClientClass * __thiscall C_FizzlerEffects::GetClientClass(C_FizzlerEffects *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x84cc6c);
}


/* _C_FizzlerEffects_CreateObject at 0051b2b0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_FizzlerEffects_CreateObject(int entnum,int serialNum)

{
  C_BaseEntity *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_FizzlerEffects * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xa60);
  C_BaseEntity::C_BaseEntity(this);
  (this->super_IClientEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x6acce4);
  (this->super_IClientEntity).super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6ad080);
  (this->super_IClientEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6ad138);
  (this->super_IClientEntity).super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6ad174);
  (this->super_IClientModelRenderable)._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6ad190);
  (**(code **)(unaff_EBX + 0x6acda0))(this,entnum,serialNum);
  return &(this->super_IClientEntity).super_IClientNetworkable;
}


/* __tcf_0 at 0097b5a0 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3ec8f4),in_stack_00000008);
  return;
}


/* _GLOBAL__I__ZN16C_FizzlerEffects17m_pClassRecvTableE at 000737b0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN16C_FizzlerEffects17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

