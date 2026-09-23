/* DWARF-guided pseudocode for game/client/portal2/c_info_placement_helper.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* ClientClassInit<DT_InfoPlacementHelper::ignored> at 00073cc0 */

int ClientClassInit<DT_InfoPlacementHelper::ignored>(ignored *param_1)

{
  RecvVarProxyFn p_Var1;
  int iVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)s_pRandomFloats + unaff_EBX + 0x34a) == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xc5ca4a);
    if (iVar2 != 0) {
      RecvPropInt((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x352),
                  (char *)(unaff_EBX + 0x916aa2),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x38e),
                        (char *)(unaff_EBX + 0x91b5e6),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00af0162 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00af011e + unaff_EBX));
      p_Var1 = *(RecvVarProxyFn *)(&DAT_00af01c2 + unaff_EBX);
      RecvPropVector((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x3ca),
                     (char *)(unaff_EBX + 0x93628a),0x118,0xc,0,p_Var1);
      RecvPropVector((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x406),
                     (char *)(unaff_EBX + 0x9328da),0x10c,0xc,0,p_Var1);
      p_Var1 = *(RecvVarProxyFn *)(&DAT_00af033e + unaff_EBX);
      RecvPropString((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x442),
                     (char *)(unaff_EBX + 0x945835),0xa60,4,0,p_Var1);
      RecvPropString((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x47e),
                     (char *)(unaff_EBX + 0x945846),0xa64,4,0,p_Var1);
      p_Var1 = *(RecvVarProxyFn *)(&DAT_00af015e + unaff_EBX);
      RecvPropFloat((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x4ba),
                    (char *)(unaff_EBX + 0x916ead),0xa68,4,0,p_Var1);
      RecvPropBool((RecvProp *)((int)s_pRandomFloats + unaff_EBX + 0x4f6),
                   (char *)(unaff_EBX + 0x945858),0xa6c,1);
      RecvPropBool((RecvProp *)((int)s_pRandomFloats + unaff_EBX + 0x532),&UNK_0094586e + unaff_EBX,
                   0xa6d,1);
      RecvPropBool((RecvProp *)((int)s_pRandomFloats + unaff_EBX + 0x56e),
                   (char *)(unaff_EBX + 0x947976),0xa6e,1);
      RecvPropFloat((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x5aa),
                    (char *)(unaff_EBX + 0x945880),0xa70,4,0,p_Var1);
      RecvPropBool((RecvProp *)((int)s_pRandomFloats + unaff_EBX + 0x5e6),
                   (char *)(unaff_EBX + 0x945890),0xa74,1);
      ___cxa_guard_release(unaff_EBX + 0xc5ca4a);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x167a),
             (RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x38e),0xb,
             (char *)(unaff_EBX + 0x9458a5));
  return 1;
}


/* __static_initialization_and_destruction_0 at 00073fe0 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  int *piVar1;
  undefined4 uVar2;
  int iVar3;
  int unaff_EBX;
  longlong lVar4;
  
  lVar4 = ___i686_get_pc_thunk_bx();
  if (lVar4 != 0xffff00000001) {
    return;
  }
  (&DAT_00c5c6ef)[unaff_EBX] = 0;
  (&DAT_00c5c6f0)[unaff_EBX] = 0;
  (&DAT_00c5c6f1)[unaff_EBX] = 0;
  (&DAT_00c5c6f2)[unaff_EBX] = 0;
  *(undefined4 *)(&DAT_00c5c6f3 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c5c6f7 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c5c6fb + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c5c6ff + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 7) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0xb) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0xf) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x13) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x17) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x1b) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x1f) = 0x7f7fffff;
  *(undefined **)((int)s_pRandomFloats + unaff_EBX + 0x23) = &UNK_00af0e97 + unaff_EBX;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x1347) = unaff_EBX + 0x945599;
  *(undefined **)((int)s_pParticlePtrs + (int)(&UNK_0000133f + unaff_EBX)) =
       &UNK_004a81ef + unaff_EBX;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0x1343) = 0;
  *(RecvTable **)((int)s_pParticlePtrs + unaff_EBX + 0x134b) =
       (RecvTable *)((int)s_pParticlePtrs + (int)(&UNK_00001357 + unaff_EBX));
  piVar1 = *(int **)(&DAT_00aefdf3 + unaff_EBX);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x134f) = *piVar1;
  *piVar1 = unaff_EBX + 0xcf3eff;
  RecvTable::RecvTable((RecvTable *)((int)s_pParticlePtrs + (int)(&UNK_00001357 + unaff_EBX)));
  uVar2 = *(undefined4 *)(&DAT_00aefd7b + unaff_EBX);
  ___cxa_atexit(unaff_EBX + 0x90761f,0,uVar2);
  iVar3 = ClientClassInit<DT_InfoPlacementHelper::ignored>((ignored *)0x0);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x136b) = iVar3;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0x136f) = 0;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0x1373) = 0;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0x1377) = 0;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0x137b) = 0;
  *(undefined4 *)((int)s_pParticlePtrs + (int)(&UNK_0000137f + unaff_EBX)) = 0;
  ___cxa_atexit(unaff_EBX + 0x90763f,0,uVar2);
  return;
}


/* C_InfoPlacementHelper::YouForgotToImplementOrDeclareClientClass at 0051bcd0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_InfoPlacementHelper *
   this) */

int __thiscall
C_InfoPlacementHelper::YouForgotToImplementOrDeclareClientClass(C_InfoPlacementHelper *this)

{
  return 0;
}


/* C_InfoPlacementHelper::GetClientClass at 0051bcf0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: ClientClass * GetClientClass(C_InfoPlacementHelper * this) */

ClientClass * __thiscall C_InfoPlacementHelper::GetClientClass(C_InfoPlacementHelper *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x84c1f8);
}


/* _C_InfoPlacementHelper_CreateObject at 0051c1e0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_InfoPlacementHelper_CreateObject(int entnum,int serialNum)

{
  C_BaseEntity *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_InfoPlacementHelper * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xa78);
  C_BaseEntity::C_BaseEntity(this);
  (this->super_IClientEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x6ac7f4);
  (this->super_IClientEntity).super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(&UNK_006acb90 + unaff_EBX);
  (this->super_IClientEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6acc48);
  (this->super_IClientEntity).super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6acc84);
  (this->super_IClientModelRenderable)._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6acca0);
  *(undefined1 *)&this[1].super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable = 0;
  *(undefined1 *)
   ((int)&this[1].super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable + 1) = 0;
  this[1].super_IClientModelRenderable._vptr_IClientModelRenderable = (_func_int_varargs **)0x0;
  *(undefined1 *)&this[1].m_iClassname = 0;
  (**(code **)(unaff_EBX + 0x6ac8b0))(this,entnum,serialNum);
  return &(this->super_IClientEntity).super_IClientNetworkable;
}


/* __tcf_0 at 0097b610 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3ec8f0),in_stack_00000008);
  return;
}


/* __tcf_1 at 0097b630 */

void __tcf_1(void *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  *(undefined4 *)(unaff_EBX + 0x3ec8e8) = 0;
  if (*(int *)(unaff_EBX + 0x3ec8e4) < 0) {
    iVar1 = *(int *)(unaff_EBX + 0x3ec8dc);
  }
  else {
    if (*(int *)(unaff_EBX + 0x3ec8dc) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1e872c) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1e872c),*(int *)(unaff_EBX + 0x3ec8dc));
      *(undefined4 *)(unaff_EBX + 0x3ec8dc) = 0;
    }
    iVar1 = 0;
    *(undefined4 *)(unaff_EBX + 0x3ec8e0) = 0;
  }
  *(int *)(unaff_EBX + 0x3ec8ec) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x3ec8e4)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1e872c) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1e872c),iVar1);
      *(undefined4 *)(unaff_EBX + 0x3ec8dc) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x3ec8e0) = 0;
  }
  return;
}


/* UTIL_FindPlacementHelper at 0051c010 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

C_InfoPlacementHelper * UTIL_FindPlacementHelper(Vector *vecEndPoint,C_BasePlayer *pPlayer)

{
  C_InfoPlacementHelper *pCVar1;
  float *pfVar2;
  int unaff_EBX;
  int iVar3;
  longdouble lVar4;
  longdouble lVar5;
  float fVar6;
  C_InfoPlacementHelper *local_58;
  float local_54;
  Vector local_40;
  Vector local_34;
  Vector local_28 [2];
  
                    /* Unresolved local var: Vector vecEyeDir@[???]
                       Unresolved local var: Vector vecEyePos@[???]
                       Unresolved local var: C_InfoPlacementHelper * pBestHelper@[???]
                       Unresolved local var: float flBestDist@[???] */
  ___i686_get_pc_thunk_bx();
  if (pPlayer != (C_BasePlayer *)0x0) {
    C_BasePlayer::EyePositionAndVectors(pPlayer,&local_34,local_28,(Vector *)0x0,(Vector *)0x0);
    if (0 < *(int *)(unaff_EBX + 0x84bf0b)) {
      local_58 = (C_InfoPlacementHelper *)0x0;
      local_54 = *(float *)(unaff_EBX + 0x4caec3);
      iVar3 = 0;
                    /* Unresolved local var: C_InfoPlacementHelper * pHelper@[???]
                       Unresolved local var: Vector vecTargetDir@[???]
                       Unresolved local var: float flTargetDist@[???]
                       Unresolved local var: float flToleranceAngle@[???]
                       Unresolved local var: float flTargetAngle@[???] */
      do {
        pCVar1 = *(C_InfoPlacementHelper **)(*(int *)(unaff_EBX + 0x84beff) + iVar3 * 4);
        if ((((pCVar1 != (C_InfoPlacementHelper *)0x0) && (pCVar1->m_bDisabled == false)) &&
            (pCVar1->m_bDeferringToPortal == false)) &&
           ((pCVar1->m_bForcePlacement != false ||
            (fVar6 = *(float *)(**(int **)(unaff_EBX + 0x647d7b) + 0xc),
            pCVar1->m_flDisableTime <= fVar6 && fVar6 != pCVar1->m_flDisableTime)))) {
          pfVar2 = (float *)(*(pCVar1->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.
                              super_IHandleEntity._vptr_IHandleEntity[0xd])(pCVar1);
                    /* Unresolved local var: Vector res@[???] */
          local_40.x = *pfVar2 - local_34.x;
          local_40.y = pfVar2[1] - local_34.y;
          local_40.z = pfVar2[2] - local_34.z;
          fVar6 = VectorNormalize(&local_40);
          lVar4 = (longdouble)_atan2((double)pCVar1->m_flRadius,(double)fVar6);
          lVar5 = (longdouble)
                  _acosf(local_28[0].x * local_40.x + local_28[0].y * local_40.y +
                         local_28[0].z * local_40.z);
          if (((float)lVar5 <= (float)lVar4) && (fVar6 <= local_54)) {
            local_58 = pCVar1;
            local_54 = fVar6;
          }
        }
        iVar3 = iVar3 + 1;
      } while (iVar3 < *(int *)(unaff_EBX + 0x84bf0b));
      return local_58;
    }
  }
  return (C_InfoPlacementHelper *)0x0;
}


/* C_InfoPlacementHelper::C_InfoPlacementHelper at 0051be30 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void C_InfoPlacementHelper(C_InfoPlacementHelper * this) */

void __thiscall C_InfoPlacementHelper::C_InfoPlacementHelper(C_InfoPlacementHelper *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::C_BaseEntity(&this->super_C_BaseEntity);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(&UNK_006acbab + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6acf47);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(&UNK_006acfff + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(&UNK_006ad03b + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(&UNK_006ad057 + unaff_EBX);
  this->m_bSnapToHelperAngles = false;
  this->m_bForcePlacement = false;
  this->m_flDisableTime = 0.0;
  this->m_bDeferringToPortal = false;
  return;
}


/* C_InfoPlacementHelper::C_InfoPlacementHelper at 0051bec0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void C_InfoPlacementHelper(C_InfoPlacementHelper * this) */

void __thiscall C_InfoPlacementHelper::C_InfoPlacementHelper(C_InfoPlacementHelper *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::C_BaseEntity(&this->super_C_BaseEntity);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6acb1b);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6aceb7);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6acf6f);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6acfab);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6acfc7);
  this->m_bSnapToHelperAngles = false;
  this->m_bForcePlacement = false;
  this->m_flDisableTime = 0.0;
  this->m_bDeferringToPortal = false;
  return;
}


/* C_InfoPlacementHelper::Spawn at 0051bf50 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void Spawn(C_InfoPlacementHelper * this) */

void __thiscall C_InfoPlacementHelper::Spawn(C_InfoPlacementHelper *this)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined4 *puVar4;
  int iVar5;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::Spawn(&this->super_C_BaseEntity);
  iVar2 = *(int *)(unaff_EBX + 0x84bfc8);
  iVar1 = iVar2 + 1;
  iVar3 = iVar1;
  if (*(int *)(unaff_EBX + 0x84bfc0) < iVar1) {
    CUtlMemory<C_InfoPlacementHelper*,int>::Grow
              ((CUtlMemory<C_InfoPlacementHelper*,int> *)(unaff_EBX + 0x84bfbc),
               iVar1 - *(int *)(unaff_EBX + 0x84bfc0));
    iVar3 = *(int *)(unaff_EBX + 0x84bfc8) + 1;
  }
  *(int *)(unaff_EBX + 0x84bfc8) = iVar3;
  iVar5 = *(int *)(unaff_EBX + 0x84bfbc);
  *(int *)(unaff_EBX + 0x84bfcc) = iVar5;
  iVar3 = (iVar3 - iVar2) + -1;
  if (0 < iVar3) {
    _V_memmove((void *)(iVar5 + iVar1 * 4),(void *)(iVar5 + iVar2 * 4),iVar3 * 4);
    iVar5 = *(int *)(unaff_EBX + 0x84bfbc);
  }
  puVar4 = (undefined4 *)(iVar2 * 4 + iVar5);
  if (puVar4 != (undefined4 *)0x0) {
    *puVar4 = this;
  }
  return;
}


/* C_InfoPlacementHelper::UpdateOnRemove at 0051bd90 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void UpdateOnRemove(C_InfoPlacementHelper * this) */

void __thiscall C_InfoPlacementHelper::UpdateOnRemove(C_InfoPlacementHelper *this)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::UpdateOnRemove(&this->super_C_BaseEntity);
  iVar1 = *(int *)(unaff_EBX + 0x84c18e);
  if (iVar1 < 1) {
    return;
  }
  piVar2 = *(int **)(unaff_EBX + 0x84c182);
  if (this == (C_InfoPlacementHelper *)*piVar2) {
    iVar4 = 0;
    iVar5 = 0;
  }
  else {
    iVar4 = 0;
    iVar3 = 4;
    do {
      iVar5 = iVar3;
      iVar4 = iVar4 + 1;
      if (iVar4 == iVar1) {
        return;
      }
      iVar3 = iVar5 + 4;
    } while (this != (C_InfoPlacementHelper *)piVar2[iVar4]);
  }
  if (iVar4 != iVar1 + -1) {
    *(int *)(iVar5 + (int)piVar2) = piVar2[iVar1 + -1];
    iVar4 = *(int *)(unaff_EBX + 0x84c18e) + -1;
  }
  *(int *)(unaff_EBX + 0x84c18e) = iVar4;
  return;
}


/* C_InfoPlacementHelper::ShouldUseHelperAngles at 0051bd00 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: bool ShouldUseHelperAngles(C_InfoPlacementHelper * this) */

bool __thiscall C_InfoPlacementHelper::ShouldUseHelperAngles(C_InfoPlacementHelper *this)

{
  return this->m_bSnapToHelperAngles;
}


/* C_InfoPlacementHelper::IsEnabled at 0051bd10 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: bool IsEnabled(C_InfoPlacementHelper * this) */

bool __thiscall C_InfoPlacementHelper::IsEnabled(C_InfoPlacementHelper *this)

{
  float fVar1;
  bool bVar2;
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  if ((this->m_bDisabled == false) && (this->m_bDeferringToPortal == false)) {
    bVar2 = true;
    if (this->m_bForcePlacement == false) {
      fVar1 = *(float *)(**(int **)(extraout_ECX + 0x648084) + 0xc);
      bVar2 = this->m_flDisableTime <= fVar1 && fVar1 != this->m_flDisableTime;
    }
    return bVar2;
  }
  return false;
}


/* C_InfoPlacementHelper::BindToPortal at 0051bd70 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void BindToPortal(C_InfoPlacementHelper * this, C_Portal_Base2D *
   pPortal) */

void __thiscall
C_InfoPlacementHelper::BindToPortal(C_InfoPlacementHelper *this,C_Portal_Base2D *pPortal)

{
  if ((pPortal != (C_Portal_Base2D *)0x0) && (this->m_bForcePlacement == false)) {
    this->m_bDeferringToPortal = true;
  }
  return;
}


/* _GLOBAL__I__ZN21C_InfoPlacementHelper17m_pClassRecvTableE at 00074140 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN21C_InfoPlacementHelper17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

