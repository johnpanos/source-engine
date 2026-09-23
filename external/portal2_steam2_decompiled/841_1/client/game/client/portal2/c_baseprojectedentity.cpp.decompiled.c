/* DWARF-guided pseudocode for game/client/portal2/c_baseprojectedentity.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* ClientClassInit<DT_BaseProjectedEntity::ignored> at 00072ca0 */

int ClientClassInit<DT_BaseProjectedEntity::ignored>(ignored *param_1)

{
  RecvVarProxyFn varProxy;
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)s_pRandomFloats + unaff_EBX + 0xa62) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc5d162);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0xa72),
                  (char *)(unaff_EBX + 0x917ac2),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0xaae),
                        (char *)(unaff_EBX + 0x91c606),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00af1182 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00af113e + unaff_EBX));
      varProxy = *(RecvVarProxyFn *)(&DAT_00af11e2 + unaff_EBX);
      RecvPropVector((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0xaea),
                     (char *)(unaff_EBX + 0x9372aa),0x118,0xc,0,varProxy);
      RecvPropVector((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0xb26),
                     (char *)(unaff_EBX + 0x9338fa),0x10c,0xc,0,varProxy);
      RecvPropEHandle((RecvProp *)((int)s_pRandomFloats + unaff_EBX + 0xb62),
                      (char *)(unaff_EBX + 0x94651e),0xa60,4,
                      *(RecvVarProxyFn *)(&DAT_00af113a + unaff_EBX));
      RecvPropEHandle((RecvProp *)((int)s_pRandomFloats + unaff_EBX + 0xb9e),
                      (char *)(unaff_EBX + 0x94652b),0xa64,4,
                      *(RecvVarProxyFn *)(&DAT_00af113a + unaff_EBX));
      RecvPropVector((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0xbda),
                     (char *)(unaff_EBX + 0x94653b),0xa68,0xc,0,varProxy);
      RecvPropVector((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0xc16),
                     (char *)(unaff_EBX + 0x946553),0xa74,0xc,0,varProxy);
      RecvPropVector((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0xc52),
                     (char *)(unaff_EBX + 0x946571),0xa80,0xc,0,varProxy);
      RecvPropVector((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0xc8e),
                     (char *)(unaff_EBX + 0x946588),0xa8c,0xc,0,varProxy);
      RecvPropVector((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0xcca),
                     (char *)(unaff_EBX + 0x936d43),0xa98,0xc,0,varProxy);
      RecvPropVector((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0xd06),
                     (char *)(unaff_EBX + 0x936d53),0xaa4,0xc,0,varProxy);
      RecvPropEHandle((RecvProp *)((int)s_pRandomFloats + unaff_EBX + 0xd42),
                      (char *)(unaff_EBX + 0x9465a5),0xab0,4,
                      *(RecvVarProxyFn *)(&DAT_00af113a + unaff_EBX));
      RecvPropEHandle((RecvProp *)((int)s_pRandomFloats + unaff_EBX + 0xd7e),
                      (char *)(unaff_EBX + 0x9465b5),0xab4,4,
                      *(RecvVarProxyFn *)(&DAT_00af113a + unaff_EBX));
      RecvPropInt((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0xdba),
                  (char *)(unaff_EBX + 0x9465c8),0xab8,4,0,(RecvVarProxyFn)0x0);
      ___cxa_guard_release(unaff_EBX + 0xc5d162);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x25ca),
             (RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0xaae),0xe,
             (char *)(unaff_EBX + 0x9465e2));
  return 1;
}


/* __static_initialization_and_destruction_0 at 00073080 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  RecvTable *this;
  int *piVar1;
  undefined4 uVar2;
  int iVar3;
  int unaff_EBX;
  longlong lVar4;
  
  lVar4 = ___i686_get_pc_thunk_bx();
  if (lVar4 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x62f) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x630) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x631) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x632) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x633) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x637) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x63b) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x63f) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x643) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x647) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x64b) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x64f) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x653) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x657) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x65b) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x65f) = 0x7f7fffff;
  *(undefined **)((int)s_pRandomFloats + unaff_EBX + 0x663) = &UNK_00af1df7 + unaff_EBX;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x21d7) = unaff_EBX + 0x946216;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x21cf) = unaff_EBX + 0x4a733f;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0x21d3) = 0;
  this = (RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x21e7);
  *(RecvTable **)((int)s_pParticlePtrs + unaff_EBX + 0x21db) = this;
  piVar1 = *(int **)(&DAT_00af0d53 + unaff_EBX);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x21df) = *piVar1;
  *piVar1 = unaff_EBX + 0xcf4d8f;
  RecvTable::RecvTable(this);
  uVar2 = *(undefined4 *)(&DAT_00af0cdb + unaff_EBX);
  ___cxa_atexit(unaff_EBX + 0x9083ef,0,uVar2);
  iVar3 = ClientClassInit<DT_BaseProjectedEntity::ignored>((ignored *)0x0);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x21fb) = iVar3;
  *(undefined4 *)((int)&PTR_PerformCustomPhysics_00bb89b4 + unaff_EBX + 3) = 9;
  *(int *)((int)&PTR_GetStudioBody_00bb89b0 + unaff_EBX + 3) = unaff_EBX + 0xbb8a0f;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x21ff) = unaff_EBX + 0xbb89b3;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x667) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x66b) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x66f) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x673) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x677) = 0;
  ___cxa_atexit(unaff_EBX + 0x90840f,0,uVar2);
  return;
}


/* C_BaseProjectedEntity::YouForgotToImplementOrDeclareClientClass at 00519ee0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_BaseProjectedEntity *
   this) */

int __thiscall
C_BaseProjectedEntity::YouForgotToImplementOrDeclareClientClass(C_BaseProjectedEntity *this)

{
  return 0;
}


/* C_BaseProjectedEntity::GetClientClass at 00519f00 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: ClientClass * GetClientClass(C_BaseProjectedEntity * this) */

ClientClass * __thiscall C_BaseProjectedEntity::GetClientClass(C_BaseProjectedEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(&UNK_0084df18 + extraout_ECX);
}


/* _C_BaseProjectedEntity_CreateObject at 0051a3d0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_BaseProjectedEntity_CreateObject(int entnum,int serialNum)

{
  C_BaseProjectedEntity *this;
  IClientNetworkable *pIVar1;
  
                    /* Unresolved local var: C_BaseProjectedEntity * pRet@[???] */
  this = C_BaseEntity::operator_new(0xac0);
  C_BaseProjectedEntity::C_BaseProjectedEntity(this);
  pIVar1 = (IClientNetworkable *)0x0;
  if (this != (C_BaseProjectedEntity *)0x0) {
    (*(this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
      _vptr_IHandleEntity[0x2f])(this,entnum,serialNum);
    pIVar1 = &(this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable;
  }
  return pIVar1;
}


/* __tcf_0 at 0097b480 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3ec9b0),in_stack_00000008);
  return;
}


/* C_BaseProjectedEntity::GetPredDescMap at 00519f10 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: datamap_t * GetPredDescMap(C_BaseProjectedEntity * this) */

datamap_t * __thiscall C_BaseProjectedEntity::GetPredDescMap(C_BaseProjectedEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x711b2c);
}


/* PredMapInit<C_BaseProjectedEntity> at 00519f20 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

datamap_t * PredMapInit<C_BaseProjectedEntity>(C_BaseProjectedEntity *param_1)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *(undefined4 *)(extraout_ECX + 0x711b1d) = 9;
  *(int *)(extraout_ECX + 0x711b19) = extraout_ECX + 0x711b75;
  return (datamap_t *)(extraout_ECX + 0x711b19);
}


/* __tcf_1 at 0097b4a0 */

void __tcf_1(void *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  *(undefined4 *)(unaff_EBX + 0x354950) = 0;
  if (*(int *)(unaff_EBX + 0x35494c) < 0) {
    iVar1 = *(int *)(unaff_EBX + 0x354944);
  }
  else {
    if (*(int *)(unaff_EBX + 0x354944) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1e88bc) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1e88bc),*(int *)(unaff_EBX + 0x354944));
      *(undefined4 *)(unaff_EBX + 0x354944) = 0;
    }
    iVar1 = 0;
    *(undefined4 *)(unaff_EBX + 0x354948) = 0;
  }
  *(int *)(unaff_EBX + 0x354954) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x35494c)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1e88bc) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1e88bc),iVar1);
      *(undefined4 *)(unaff_EBX + 0x354944) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x354948) = 0;
  }
  return;
}


/* C_BaseProjectedEntity::C_BaseProjectedEntity at 0051a290 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void C_BaseProjectedEntity(C_BaseProjectedEntity * this) */

void __thiscall C_BaseProjectedEntity::C_BaseProjectedEntity(C_BaseProjectedEntity *this)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined4 *puVar4;
  int iVar5;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::C_BaseEntity(&this->super_C_BaseEntity);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6ad26a);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6ad63a);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6ad6f2);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6ad72e);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6ad74a);
  (this->m_hHitPortal).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hSourcePortal).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hChildSegment).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hPlacementHelper).super_CBaseHandle.m_Index = 0xffffffff;
  iVar2 = *(int *)(unaff_EBX + 0x7b5b66);
  iVar1 = iVar2 + 1;
  iVar3 = iVar1;
  if (*(int *)(unaff_EBX + 0x7b5b5e) < iVar1) {
    CUtlMemory<C_BaseProjectedEntity*,int>::Grow
              ((CUtlMemory<C_BaseProjectedEntity*,int> *)(unaff_EBX + 0x7b5b5a),
               iVar1 - *(int *)(unaff_EBX + 0x7b5b5e));
    iVar3 = *(int *)(unaff_EBX + 0x7b5b66) + 1;
  }
  *(int *)(unaff_EBX + 0x7b5b66) = iVar3;
  iVar5 = *(int *)(unaff_EBX + 0x7b5b5a);
  *(int *)(unaff_EBX + 0x7b5b6a) = iVar5;
  iVar3 = (iVar3 - iVar2) + -1;
  if (0 < iVar3) {
    _V_memmove((void *)(iVar5 + iVar1 * 4),(void *)(iVar5 + iVar2 * 4),iVar3 * 4);
    iVar5 = *(int *)(unaff_EBX + 0x7b5b5a);
  }
  puVar4 = (undefined4 *)(iVar2 * 4 + iVar5);
  if (puVar4 != (undefined4 *)0x0) {
    *puVar4 = this;
  }
  return;
}


/* C_BaseProjectedEntity::C_BaseProjectedEntity at 0051a3c0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void C_BaseProjectedEntity(C_BaseProjectedEntity * this,
   C_BaseProjectedEntity * this) */

void __thiscall
C_BaseProjectedEntity::C_BaseProjectedEntity
          (C_BaseProjectedEntity *this,C_BaseProjectedEntity *this_1)

{
  C_BaseProjectedEntity(this);
  return;
}


/* C_BaseProjectedEntity::~C_BaseProjectedEntity at 0051a1b0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_BaseProjectedEntity(C_BaseProjectedEntity * this, int
   __in_chrg) */

void __thiscall
C_BaseProjectedEntity::~C_BaseProjectedEntity(C_BaseProjectedEntity *this,int __in_chrg)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int unaff_EBX;
  int local_14;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(&UNK_006ad34a + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6ad71a);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6ad7d2);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6ad80e);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6ad82a);
  iVar1 = *(int *)(unaff_EBX + 0x7b5c46);
  if (0 < iVar1) {
    piVar2 = *(int **)(unaff_EBX + 0x7b5c3a);
    if (this == (C_BaseProjectedEntity *)*piVar2) {
      iVar4 = 0;
      local_14 = 0;
    }
    else {
      iVar4 = 0;
      local_14 = 4;
      iVar3 = local_14;
      do {
        local_14 = iVar3;
        iVar4 = iVar4 + 1;
        if (iVar4 == iVar1) goto LAB_0051a246;
        iVar3 = local_14 + 4;
      } while (this != (C_BaseProjectedEntity *)piVar2[iVar4]);
    }
    if (iVar4 != iVar1 + -1) {
      *(int *)((int)piVar2 + local_14) = piVar2[iVar1 + -1];
      iVar4 = *(int *)(unaff_EBX + 0x7b5c46) + -1;
    }
    *(int *)(unaff_EBX + 0x7b5c46) = iVar4;
    C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
    return;
  }
LAB_0051a246:
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
  return;
}


/* C_BaseProjectedEntity::~C_BaseProjectedEntity at 0051a430 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_BaseProjectedEntity(C_BaseProjectedEntity * this, int
   __in_chrg) */

void __thiscall
C_BaseProjectedEntity::~C_BaseProjectedEntity(C_BaseProjectedEntity *this,int __in_chrg)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int unaff_EBX;
  int in_stack_ffffffb8;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6ad0ca);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(&UNK_006ad49a + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6ad552);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6ad58e);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6ad5aa);
  iVar1 = *(int *)(unaff_EBX + 0x7b59c6);
  if (0 < iVar1) {
    piVar2 = *(int **)(unaff_EBX + 0x7b59ba);
    if (this == (C_BaseProjectedEntity *)*piVar2) {
      iVar4 = 0;
      local_20 = 0;
    }
    else {
      iVar4 = 0;
      local_20 = 4;
      iVar3 = local_20;
      do {
        local_20 = iVar3;
        iVar4 = iVar4 + 1;
        if (iVar4 == iVar1) goto LAB_0051a4c6;
        iVar3 = local_20 + 4;
      } while (this != (C_BaseProjectedEntity *)piVar2[iVar4]);
    }
    if (iVar4 != iVar1 + -1) {
      *(int *)(local_20 + (int)piVar2) = piVar2[iVar1 + -1];
      iVar4 = *(int *)(unaff_EBX + 0x7b59c6) + -1;
    }
    *(int *)(unaff_EBX + 0x7b59c6) = iVar4;
  }
LAB_0051a4c6:
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,in_stack_ffffffb8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_BaseProjectedEntity::~C_BaseProjectedEntity at 0051a500 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_BaseProjectedEntity(C_BaseProjectedEntity * this, int
   __in_chrg) */

void __thiscall
C_BaseProjectedEntity::~C_BaseProjectedEntity(C_BaseProjectedEntity *this,int __in_chrg)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int unaff_EBX;
  int local_14;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(&UNK_006acffa + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6ad3ca);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(&UNK_006ad482 + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(&UNK_006ad4be + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(&UNK_006ad4da + unaff_EBX);
  iVar1 = *(int *)(unaff_EBX + 0x7b58f6);
  if (0 < iVar1) {
    piVar2 = *(int **)(unaff_EBX + 0x7b58ea);
    if (this == (C_BaseProjectedEntity *)*piVar2) {
      iVar4 = 0;
      local_14 = 0;
    }
    else {
      iVar4 = 0;
      local_14 = 4;
      iVar3 = local_14;
      do {
        local_14 = iVar3;
        iVar4 = iVar4 + 1;
        if (iVar4 == iVar1) goto LAB_0051a596;
        iVar3 = local_14 + 4;
      } while (this != (C_BaseProjectedEntity *)piVar2[iVar4]);
    }
    if (iVar4 != iVar1 + -1) {
      *(int *)((int)piVar2 + local_14) = piVar2[iVar1 + -1];
      iVar4 = *(int *)(unaff_EBX + 0x7b58f6) + -1;
    }
    *(int *)(unaff_EBX + 0x7b58f6) = iVar4;
    C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
    return;
  }
LAB_0051a596:
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
  return;
}


/* C_BaseProjectedEntity::GetPredictionOwner at 0051a0f0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: C_BasePlayer * GetPredictionOwner(C_BaseProjectedEntity * this) */

C_BasePlayer * __thiscall C_BaseProjectedEntity::GetPredictionOwner(C_BaseProjectedEntity *this)

{
  uint uVar1;
  char cVar2;
  int iVar3;
  int iVar4;
  C_BasePlayer *pCVar5;
  int unaff_EBX;
  
                    /* Unresolved local var: C_Portal_Base2D * pSourcePortal@[???] */
  ___i686_get_pc_thunk_bx();
  iVar4 = **(int **)(unaff_EBX + 0x649cdb);
  uVar1 = (this->m_hSourcePortal).super_CBaseHandle.m_Index;
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???]
                       Unresolved local var: CEntInfo * pInfo@[???] */
  if (((((uVar1 == 0xffffffff) ||
        (iVar3 = (uVar1 & 0xffff) * 0x10 + iVar4, *(uint *)(iVar3 + 8) != uVar1 >> 0x10)) ||
       (iVar3 = *(int *)(iVar3 + 4), iVar3 == 0)) ||
      ((iVar3 = ___dynamic_cast(iVar3,*(undefined4 *)(unaff_EBX + 0x649f7b),
                                *(undefined4 *)(unaff_EBX + 0x649f83),0), iVar3 == 0 ||
       (uVar1 = *(uint *)(iVar3 + 0x33dc), uVar1 == 0xffffffff)))) ||
     ((iVar4 = (uVar1 & 0xffff) * 0x10 + iVar4, *(uint *)(iVar4 + 8) != uVar1 >> 0x10 ||
      ((pCVar5 = *(C_BasePlayer **)(iVar4 + 4), pCVar5 == (C_BasePlayer *)0x0 ||
       (cVar2 = (**(code **)(*(int *)&(pCVar5->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                      super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                      super_C_BaseEntity.super_IClientEntity.super_IClientUnknown +
                            0x304))(pCVar5), cVar2 == '\0')))))) {
    pCVar5 = (C_BasePlayer *)0x0;
  }
  return pCVar5;
}


/* C_BaseProjectedEntity::OnDataChanged at 0051a0a0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void OnDataChanged(C_BaseProjectedEntity * this, DataUpdateType_t
   updateType) */

void __thiscall
C_BaseProjectedEntity::OnDataChanged(C_BaseProjectedEntity *this,DataUpdateType_t updateType)

{
  C_BaseEntity::OnDataChanged(&this->super_C_BaseEntity,updateType);
  if (updateType != DATA_UPDATE_CREATED) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x0051a0e4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0xf1])();
  return;
}


/* C_BaseProjectedEntity::PostDataUpdate at 00519fb0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void PostDataUpdate(C_BaseProjectedEntity * this, DataUpdateType_t
   updateType) */

void __thiscall
C_BaseProjectedEntity::PostDataUpdate(C_BaseProjectedEntity *this,DataUpdateType_t updateType)

{
  uint uVar1;
  int iVar2;
  C_BasePlayer *pOwner;
  int unaff_EBX;
  
                    /* Unresolved local var: C_Portal_Base2D * pSourcePortal@[???] */
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::PostDataUpdate(&this->super_C_BaseEntity,updateType);
  uVar1 = (this->m_hSourcePortal).super_CBaseHandle.m_Index;
  if (((uVar1 == 0xffffffff) ||
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x649e18),
      *(uint *)(iVar2 + 8) != uVar1 >> 0x10)) || (iVar2 = *(int *)(iVar2 + 4), iVar2 == 0)) {
    C_BaseEntity::SetPredictionEligible(&this->super_C_BaseEntity,false);
  }
  else {
    C_BaseEntity::SetPredictionEligible(&this->super_C_BaseEntity,true);
    if (*(char *)(iVar2 + 0xa0d) != '\0') {
      uVar1 = *(uint *)(iVar2 + 0xa1c);
      if ((uVar1 == 0xffffffff) ||
         (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x649e18),
         *(uint *)(iVar2 + 8) != uVar1 >> 0x10)) {
        pOwner = (C_BasePlayer *)0x0;
      }
      else {
        pOwner = *(C_BasePlayer **)(iVar2 + 4);
      }
      C_BaseEntity::SetPlayerSimulated(&this->super_C_BaseEntity,pOwner);
      return;
    }
  }
  C_BaseEntity::UnsetPlayerSimulated(&this->super_C_BaseEntity);
  return;
}


/* C_BaseProjectedEntity::TestAllForProjectionChanges at 00519f50 */

void C_BaseProjectedEntity::TestAllForProjectionChanges(void)

{
  int *piVar1;
  int unaff_EBX;
  int iVar2;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x7b5ea6)) {
    iVar2 = 0;
    do {
      piVar1 = *(int **)(*(int *)(unaff_EBX + 0x7b5e9a) + iVar2 * 4);
      (**(code **)(*piVar1 + 0x3a4))(piVar1);
      iVar2 = iVar2 + 1;
    } while (iVar2 < *(int *)(unaff_EBX + 0x7b5ea6));
  }
  return;
}


/* _GLOBAL__I__ZN21C_BaseProjectedEntity17m_pClassRecvTableE at 00073200 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN21C_BaseProjectedEntity17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

