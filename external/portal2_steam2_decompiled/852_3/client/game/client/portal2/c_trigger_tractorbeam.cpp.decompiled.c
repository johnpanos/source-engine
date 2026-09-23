/* DWARF-guided pseudocode for game/client/portal2/c_trigger_tractorbeam.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 00079950 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  RecvTable *pRVar1;
  int *piVar2;
  undefined4 uVar3;
  int iVar4;
  IClassMap *pIVar5;
  IMaterialProxyDict *pIVar6;
  int unaff_EBX;
  longlong lVar7;
  
  lVar7 = ___i686_get_pc_thunk_bx();
  if (lVar7 == 0xffff00000001) {
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x219].m_pPrev + unaff_EBX) = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x219].m_pPrev + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x219].m_pPrev + unaff_EBX + 2) = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x219].m_pPrev + unaff_EBX + 3) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x219].m_pNext + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x21a].m_pEntity + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x21a].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x21a].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x21a].m_pNext + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x21b].m_pEntity + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x21b].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x21b].m_pPrev + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x21b].m_pNext + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x21c].m_pEntity + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x21c].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x21c].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined **)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x21c].m_pNext + unaff_EBX) =
         &UNK_00ab84a4 + unaff_EBX;
    *(undefined4 *)((int)&PTR____cxa_pure_virtual_00b7d2c0 + unaff_EBX) = 1;
    *(int *)((int)&PTR____cxa_pure_virtual_00b7d2bc + unaff_EBX) = unaff_EBX + 0xc409bc;
    *(int *)((int)s_SIMDRandContexts[0x1a].m_RandY[0x1c] + unaff_EBX + 8) =
         (int)&PTR____cxa_pure_virtual_00b7d2bc + unaff_EBX;
    *(int *)((int)s_SIMDRandContexts[0x1a].m_RandY[0x1d] + unaff_EBX + 4) = unaff_EBX + 0x916fed;
    *(int *)((int)s_SIMDRandContexts[0x1a].m_RandY[0x1c] + unaff_EBX + 0xc) = unaff_EBX + 0x4c151c;
    *(undefined4 *)((int)s_SIMDRandContexts[0x1a].m_RandY[0x1d] + unaff_EBX) = 0;
    pRVar1 = (RecvTable *)((int)s_SIMDRandContexts[0x1a].m_RandY[0x1e] + unaff_EBX + 4);
    *(RecvTable **)((int)s_SIMDRandContexts[0x1a].m_RandY[0x1d] + unaff_EBX + 8) = pRVar1;
    piVar2 = *(int **)(&DAT_00ab7434 + unaff_EBX);
    *(int *)((int)s_SIMDRandContexts[0x1a].m_RandY[0x1d] + unaff_EBX + 0xc) = *piVar2;
    *piVar2 = unaff_EBX + 0xcd666c;
    RecvTable::RecvTable(pRVar1);
    uVar3 = *(undefined4 *)(&DAT_00ab73bc + unaff_EBX);
    ___cxa_atexit(unaff_EBX + 0x8d831c,0,uVar3);
    iVar4 = ClientClassInit<DT_ProjectedTractorBeamEntity::ignored>((ignored *)0x0);
    *(int *)((int)s_SIMDRandContexts[0x1a].m_RandY[0x1f] + unaff_EBX + 8) = iVar4;
    pIVar5 = GetClassMap();
    (*pIVar5->_vptr_IClassMap[2])
              (pIVar5,unaff_EBX + 0x917026,unaff_EBX + 0x917009,0xabc,unaff_EBX + 0x4c265c);
    *(int *)((int)s_SIMDRandContexts[0x1a].m_RandY[0x20] + unaff_EBX + 4) = unaff_EBX + 0x917044;
    *(int *)((int)s_SIMDRandContexts[0x1a].m_RandY[0x1f] + unaff_EBX + 0xc) = unaff_EBX + 0x4c402c;
    *(undefined4 *)((int)s_SIMDRandContexts[0x1a].m_RandY[0x20] + unaff_EBX) = 0;
    pRVar1 = (RecvTable *)((int)s_SIMDRandContexts[0x1a].m_RandY[0x21] + unaff_EBX + 4);
    *(RecvTable **)((int)s_SIMDRandContexts[0x1a].m_RandY[0x20] + unaff_EBX + 8) = pRVar1;
    *(int *)((int)s_SIMDRandContexts[0x1a].m_RandY[0x20] + unaff_EBX + 0xc) = *piVar2;
    *piVar2 = unaff_EBX + 0xcd669c;
    RecvTable::RecvTable(pRVar1);
    ___cxa_atexit(unaff_EBX + 0x8d82fc,0,uVar3);
    iVar4 = ClientClassInit<DT_Trigger_TractorBeam::ignored>((ignored *)0x0);
    *(int *)((int)s_SIMDRandContexts[0x1a].m_RandY[0x22] + unaff_EBX + 8) = iVar4;
    *(undefined4 *)((int)&PTR____cxa_pure_virtual_00b7d2e0 + unaff_EBX) = 2;
    *(int *)((int)&PTR____cxa_pure_virtual_00b7d2dc + unaff_EBX) =
         (int)&PTR_ProcessInput_00b7d33c + unaff_EBX;
    *(int *)((int)s_SIMDRandContexts[0x1a].m_RandY[0x22] + unaff_EBX + 0xc) =
         (int)&PTR____cxa_pure_virtual_00b7d2dc + unaff_EBX;
    pIVar5 = GetClassMap();
    (*pIVar5->_vptr_IClassMap[2])
              (pIVar5,unaff_EBX + 0x8e9e60,(char *)(unaff_EBX + 0x916fd6),0xae4,unaff_EBX + 0x4c408c
              );
    CIFM_EntityKeyValuesHandler_RecreateEntities::CIFM_EntityKeyValuesHandler_RecreateEntities
              ((CIFM_EntityKeyValuesHandler_RecreateEntities *)
               ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x21d].m_pPrev + unaff_EBX),
               (char *)(unaff_EBX + 0x916fd6));
    *(undefined **)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x21d].m_pPrev + unaff_EBX) =
         &UNK_00b22144 + unaff_EBX;
    ___cxa_atexit(&UNK_008d833c + unaff_EBX,0,uVar3);
    pIVar6 = GetMaterialProxyDict();
    (*pIVar6->_vptr_IMaterialProxyDict[3])(pIVar6,unaff_EBX + 0x917059,unaff_EBX + 0x4c15cc);
  }
  return;
}


/* C_ProjectedTractorBeamEntity::GetPredDescMap at 0053ac10 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: datamap_t * GetPredDescMap(C_ProjectedTractorBeamEntity * this) */

datamap_t * __thiscall
C_ProjectedTractorBeamEntity::GetPredDescMap(C_ProjectedTractorBeamEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x6bc008);
}


/* PredMapInit<C_ProjectedTractorBeamEntity> at 0053ac20 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

datamap_t * PredMapInit<C_ProjectedTractorBeamEntity>(C_ProjectedTractorBeamEntity *param_1)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *(undefined4 *)(extraout_ECX + 0x6bbff9) = 1;
  *(int *)(extraout_ECX + 0x6bbff5) = extraout_ECX + 0x77f6f5;
  return (datamap_t *)(extraout_ECX + 0x6bbff5);
}


/* ClientClassInit<DT_ProjectedTractorBeamEntity::ignored> at 00079800 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

int ClientClassInit<DT_ProjectedTractorBeamEntity::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x28d].m_pEntity + unaff_EBX)
      == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc40a34);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x28e].m_pPrev + unaff_EBX
                  ),(char *)(unaff_EBX + 0x8e654c),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x292].
                               m_SerialNumber + unaff_EBX),&UNK_008eb12c + unaff_EBX,0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00ab7d64 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00ab758c + unaff_EBX));
      RecvPropEHandle((RecvProp *)
                      (&UNK_00002964 +
                      (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                      (char *)(unaff_EBX + 0x9170f2),0xab8,4,
                      *(RecvVarProxyFn *)(&DAT_00ab7588 + unaff_EBX));
      ___cxa_guard_release(unaff_EBX + 0xc40a34);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_SIMDRandContexts[0x1a].m_RandY[0x33] + unaff_EBX + 4),
             (RecvProp_conflict1 *)
             ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x292].m_SerialNumber +
             unaff_EBX),2,(char *)(unaff_EBX + 0x917108));
  return 1;
}


/* C_ProjectedTractorBeamEntity::YouForgotToImplementOrDeclareClientClass at 0053ac50 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: int
   YouForgotToImplementOrDeclareClientClass(C_ProjectedTractorBeamEntity * this) */

int __thiscall
C_ProjectedTractorBeamEntity::YouForgotToImplementOrDeclareClientClass
          (C_ProjectedTractorBeamEntity *this)

{
  return 0;
}


/* C_ProjectedTractorBeamEntity::GetClientClass at 0053ac70 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: ClientClass * GetClientClass(C_ProjectedTractorBeamEntity * this) */

ClientClass * __thiscall
C_ProjectedTractorBeamEntity::GetClientClass(C_ProjectedTractorBeamEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(&UNK_00815358 + extraout_ECX);
}


/* _C_ProjectedTractorBeamEntity_CreateObject at 0053ae80 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_ProjectedTractorBeamEntity_CreateObject(int entnum,int serialNum)

{
  C_BaseProjectedEntity *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_ProjectedTractorBeamEntity * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xabc);
  C_BaseProjectedEntity::C_BaseProjectedEntity(this);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6606f4);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x660ac0);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x660b78);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x660bb4);
  this[1].super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)0xffffffff;
  (**(code **)(unaff_EBX + 0x6607b0))(this,entnum,serialNum);
  return &(this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable;
}


/* __tcf_0 at 00951c80 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3fe360),in_stack_00000008);
  return;
}


/* CC_ProjectedTractorBeamEntityFactory at 0053bfc0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

C_BaseEntity * CC_ProjectedTractorBeamEntityFactory(void)

{
  C_BaseProjectedEntity *this;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xabc);
  C_BaseProjectedEntity::C_BaseProjectedEntity(this);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(&UNK_0065f5b4 + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x65f980);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x65fa38);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x65fa74);
  this[1].super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)0xffffffff;
  return &this->super_C_BaseEntity;
}


/* C_Trigger_TractorBeam::RecvProxy_Start at 0053bef0 */

void C_Trigger_TractorBeam::RecvProxy_Start(CRecvProxyData *pData,void *pStruct,void *pOut)

{
  float fVar1;
  float fVar2;
  float fVar3;
  int extraout_ECX;
  
                    /* Unresolved local var: C_Trigger_TractorBeam * pBeam@[???]
                       Unresolved local var: Vector vStart@[???] */
  ___i686_get_pc_thunk_cx();
  fVar1 = (pData->m_Value).field_0.m_Vector[2];
  fVar2 = (pData->m_Value).field_0.m_Vector[1];
  fVar3 = (pData->m_Value).field_0.m_Float;
  if ((((fVar3 != *(float *)((int)pStruct + 0xa64)) ||
       (NAN(fVar3) || NAN(*(float *)((int)pStruct + 0xa64)))) ||
      (fVar2 != *(float *)((int)pStruct + 0xa68))) ||
     ((NAN(fVar2) || NAN(*(float *)((int)pStruct + 0xa68)) ||
      (fVar1 != *(float *)((int)pStruct + 0xa6c))))) {
    *(float *)((int)pStruct + 0xa64) = fVar3;
    *(float *)((int)pStruct + 0xa68) = fVar2;
    *(float *)((int)pStruct + 0xa6c) = fVar1;
    *(undefined4 *)((int)pStruct + 0xae0) =
         *(undefined4 *)(**(int **)(&DAT_005f4e58 + extraout_ECX) + 0xc);
    *(undefined1 *)((int)pStruct + 0xa94) = 1;
  }
  return;
}


/* C_Trigger_TractorBeam::RecvProxy_End at 0053bb00 */

void C_Trigger_TractorBeam::RecvProxy_End(CRecvProxyData *pData,void *pStruct,void *pOut)

{
  float fVar1;
  float fVar2;
  float fVar3;
  int extraout_ECX;
  
                    /* Unresolved local var: C_Trigger_TractorBeam * pBeam@[???]
                       Unresolved local var: Vector vEnd@[???] */
  ___i686_get_pc_thunk_cx();
  fVar1 = (pData->m_Value).field_0.m_Vector[2];
  fVar2 = (pData->m_Value).field_0.m_Vector[1];
  fVar3 = (pData->m_Value).field_0.m_Float;
  if ((((fVar3 != *(float *)((int)pStruct + 0xa70)) ||
       (NAN(fVar3) || NAN(*(float *)((int)pStruct + 0xa70)))) ||
      (fVar2 != *(float *)((int)pStruct + 0xa74))) ||
     ((NAN(fVar2) || NAN(*(float *)((int)pStruct + 0xa74)) ||
      (fVar1 != *(float *)((int)pStruct + 0xa78))))) {
    *(float *)((int)pStruct + 0xa70) = fVar3;
    *(float *)((int)pStruct + 0xa74) = fVar2;
    *(float *)((int)pStruct + 0xa78) = fVar1;
    *(undefined4 *)((int)pStruct + 0xae0) =
         *(undefined4 *)(**(int **)(&DAT_005f5248 + extraout_ECX) + 0xc);
    *(undefined1 *)((int)pStruct + 0xa94) = 1;
  }
  return;
}


/* ClientClassInit<DT_Trigger_TractorBeam::ignored> at 00079290 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

int ClientClassInit<DT_Trigger_TractorBeam::ignored>(ignored *param_1)

{
  RecvVarProxyFn varProxy;
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x28b].m_pNext +
               unaff_EBX + 2) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc40a22);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x28d].m_pNext +
                  unaff_EBX + 2),(char *)(unaff_EBX + 0x8e6ac2),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x291].m_pPrev +
                        unaff_EBX + 2),(char *)(unaff_EBX + 0x8eb6a2),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00ab82f2 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00ab7b02 + unaff_EBX));
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x295].m_SerialNumber +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0x91751e),0xa59,1);
      varProxy = *(RecvVarProxyFn *)(&DAT_00ab7b42 + unaff_EBX);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x299].m_pEntity +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x91752a),0xaac,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x29c].m_pNext +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x917539),0xab0,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2a0].m_pPrev +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x917549),0xab4,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2a4].m_SerialNumber +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x917557),0xab8,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2a8].m_pEntity +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x91756a),0xabc,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2ab].m_pNext +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x91757c),0xac0,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)
                    (&UNK_00002afe +
                    (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                    (char *)(unaff_EBX + 0x91758f),0xac4,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2b3].m_SerialNumber +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x9175a6),0xac8,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2b7].m_pEntity +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x9175b4),0xacc,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2ba].m_pNext +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x9175c3),0xad0,4,0,varProxy);
      RecvPropVector((RecvProp_conflict1 *)
                     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2be].m_pPrev +
                     unaff_EBX + 2),(char *)(unaff_EBX + 0x9175d2),0xad4,0xc,0,
                     *(RecvVarProxyFn *)(&DAT_00ab7baa + unaff_EBX));
      RecvPropEHandle((RecvProp *)
                      ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2c2].m_SerialNumber
                      + unaff_EBX + 2),(char *)(unaff_EBX + 0x9175e6),0xa98,4,
                      *(RecvVarProxyFn *)(&DAT_00ab7afe + unaff_EBX));
      RecvPropVector((RecvProp_conflict1 *)
                     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2c6].m_pEntity +
                     unaff_EBX + 2),(char *)(unaff_EBX + 0x9175f5),0xa64,0xc,0,
                     (RecvVarProxyFn)(unaff_EBX + 0x4c2c52));
      RecvPropVector((RecvProp_conflict1 *)
                     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2c9].m_pNext +
                     unaff_EBX + 2),(char *)(unaff_EBX + 0x9175fe),0xa70,0xc,0,
                     (RecvVarProxyFn)(unaff_EBX + 0x4c2862));
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2cd].m_pPrev +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x917605),0xa7c,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2d1].m_SerialNumber +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x8e6ecd),0xa80,4,0,varProxy);
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2d5].m_pEntity +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0x917616),0xa84,1);
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2d8].m_pNext +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0x917622),0xa85,1);
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2dc].m_pPrev +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0x917630),0xa86,1);
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2e0].m_SerialNumber +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0x91763c),0xa87,1);
      ___cxa_guard_release(unaff_EBX + 0xc40a22);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_SIMDRandContexts[0x1c].m_RandY[0x1d] + unaff_EBX + 10),
             (RecvProp_conflict1 *)
             ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x291].m_pPrev + unaff_EBX + 2)
             ,0x16,(char *)(unaff_EBX + 0x917651));
  return 1;
}


/* C_Trigger_TractorBeam::YouForgotToImplementOrDeclareClientClass at 0053ac80 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_Trigger_TractorBeam *
   this) */

int __thiscall
C_Trigger_TractorBeam::YouForgotToImplementOrDeclareClientClass(C_Trigger_TractorBeam *this)

{
  return 0;
}


/* C_Trigger_TractorBeam::GetClientClass at 0053aca0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: ClientClass * GetClientClass(C_Trigger_TractorBeam * this) */

ClientClass * __thiscall C_Trigger_TractorBeam::GetClientClass(C_Trigger_TractorBeam *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(&UNK_00815358 + extraout_ECX);
}


/* _C_Trigger_TractorBeam_CreateObject at 0053d990 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */

IClientNetworkable * _C_Trigger_TractorBeam_CreateObject(int entnum,int serialNum)

{
  C_Trigger_TractorBeam *this;
  IClientNetworkable *pIVar1;
  
                    /* Unresolved local var: C_Trigger_TractorBeam * pRet@[???] */
  this = C_BaseEntity::operator_new(0xae4);
  C_Trigger_TractorBeam::C_Trigger_TractorBeam(this);
  pIVar1 = (IClientNetworkable *)0x0;
  if (this != (C_Trigger_TractorBeam *)0x0) {
    (*(this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.
      super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0x2f])(this,entnum,serialNum);
    pIVar1 = &(this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.
              super_IClientNetworkable;
  }
  return pIVar1;
}


/* __tcf_1 at 00951c60 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3fe3b0),in_stack_00000008);
  return;
}


/* C_Trigger_TractorBeam::GetPredDescMap at 0053acb0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: datamap_t * GetPredDescMap(C_Trigger_TractorBeam * this) */

datamap_t * __thiscall C_Trigger_TractorBeam::GetPredDescMap(C_Trigger_TractorBeam *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x6bbf88);
}


/* PredMapInit<C_Trigger_TractorBeam> at 0053acc0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */

datamap_t * PredMapInit<C_Trigger_TractorBeam>(C_Trigger_TractorBeam *param_1)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *(undefined4 *)(extraout_ECX + 0x6bbf79) = 2;
  *(int *)(extraout_ECX + 0x6bbf75) = extraout_ECX + 0x6bbfd5;
  return (datamap_t *)(extraout_ECX + 0x6bbf75);
}


/* CC_Trigger_TractorBeamFactory at 0053d9f0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */

C_BaseEntity * CC_Trigger_TractorBeamFactory(void)

{
  C_Trigger_TractorBeam *this;
  
  this = C_BaseEntity::operator_new(0xae4);
  C_Trigger_TractorBeam::C_Trigger_TractorBeam(this);
  return (C_BaseEntity *)this;
}


/* C_Trigger_TractorBeam::C_Trigger_TractorBeam at 0053d7a0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void C_Trigger_TractorBeam(C_Trigger_TractorBeam * this) */

void __thiscall C_Trigger_TractorBeam::C_Trigger_TractorBeam(C_Trigger_TractorBeam *this)

{
  C_Trigger_TractorBeam **ppCVar1;
  CUtlMemory<C_Trigger_TractorBeam*,int> *this_00;
  C_Trigger_TractorBeam **ppCVar2;
  int iVar3;
  C_Trigger_TractorBeam **ppCVar4;
  C_Trigger_TractorBeam **ppCVar5;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::C_BaseEntity((C_BaseEntity *)this);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x65d91a);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable
  ._vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x65dcb6);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.
  super_IClientNetworkable._vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x65dd6e);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x65ddaa);
  (this->super_IMotionEvent)._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x65ddc6);
  (this->m_hProxyEntity).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hCoreEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
  (this->m_hCoreEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
  (this->m_hCoreEffect).m_pObject = (CNewParticleEffect *)0x0;
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.field_0xa59 = 0;
  this_00 = *(CUtlMemory<C_Trigger_TractorBeam*,int> **)(unaff_EBX + 0x5f3de6);
  ppCVar2 = this_00[1].m_pMemory;
  ppCVar1 = (C_Trigger_TractorBeam **)((int)ppCVar2 + 1);
  if (this_00->m_nAllocationCount < (int)ppCVar1) {
    CUtlMemory<C_Trigger_TractorBeam*,int>::Grow(this_00,(int)ppCVar1 - this_00->m_nAllocationCount)
    ;
    ppCVar4 = (C_Trigger_TractorBeam **)((int)this_00[1].m_pMemory + 1);
    this_00[1].m_pMemory = ppCVar4;
    ppCVar5 = this_00->m_pMemory;
    this_00[1].m_nAllocationCount = (int)ppCVar5;
    iVar3 = (int)ppCVar4 + (-1 - (int)ppCVar2);
  }
  else {
    this_00[1].m_pMemory = ppCVar1;
    ppCVar5 = this_00->m_pMemory;
    this_00[1].m_nAllocationCount = (int)ppCVar5;
    iVar3 = (int)ppCVar1 + (-1 - (int)ppCVar2);
  }
  if (0 < iVar3) {
    _V_memmove(ppCVar5 + (int)ppCVar1,ppCVar5 + (int)ppCVar2,iVar3 << 2);
    ppCVar5 = this_00->m_pMemory;
  }
  if (ppCVar5 + (int)ppCVar2 != (C_Trigger_TractorBeam **)0x0) {
    ppCVar5[(int)ppCVar2] = this;
  }
  return;
}


/* C_Trigger_TractorBeam::C_Trigger_TractorBeam at 0053d980 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void C_Trigger_TractorBeam(C_Trigger_TractorBeam * this,
   C_Trigger_TractorBeam * this) */

void __thiscall
C_Trigger_TractorBeam::C_Trigger_TractorBeam
          (C_Trigger_TractorBeam *this,C_Trigger_TractorBeam *this_1)

{
  C_Trigger_TractorBeam(this);
  return;
}


/* C_Trigger_TractorBeam_NonConformantDataHandler::CreateInstance at 0053e390 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void * CreateInstance(C_Trigger_TractorBeam_NonConformantDataHandler *
   this) */

void * __thiscall
C_Trigger_TractorBeam_NonConformantDataHandler::CreateInstance
          (C_Trigger_TractorBeam_NonConformantDataHandler *this)

{
  C_Trigger_TractorBeam *this_00;
  
  this_00 = C_BaseEntity::operator_new(0xae4);
  C_Trigger_TractorBeam::C_Trigger_TractorBeam(this_00);
  return this_00;
}


/* C_Trigger_TractorBeam::~C_Trigger_TractorBeam at 0053d540 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* WARNING: Struct "BaseModHybridButton": ignoring multiple overlapping fields */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void ~C_Trigger_TractorBeam(C_Trigger_TractorBeam * this, int
   __in_chrg) */

void __thiscall
C_Trigger_TractorBeam::~C_Trigger_TractorBeam(C_Trigger_TractorBeam *this,int __in_chrg)

{
  int *piVar1;
  CNewParticleEffect *pCVar2;
  CUtlReference<CNewParticleEffect> *pCVar3;
  CUtlReference<CNewParticleEffect> *pCVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int unaff_EBX;
  int local_30;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x65db7a);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable
  ._vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x65df16);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.
  super_IClientNetworkable._vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x65dfce);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x65e00a);
  (this->super_IMotionEvent)._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x65e026);
  local_30 = *(int *)(*(int *)(BaseModUI::SteamCloudConfirmation::OnCommand + unaff_EBX + 6) + 0xc);
  if (0 < local_30) {
    piVar1 = (int *)**(undefined4 **)(BaseModUI::SteamCloudConfirmation::OnCommand + unaff_EBX + 6);
    if (this == (C_Trigger_TractorBeam *)*piVar1) {
      iVar7 = 0;
      iVar6 = 0;
    }
    else {
      iVar7 = 0;
      iVar5 = 4;
      do {
        iVar6 = iVar5;
        iVar7 = iVar7 + 1;
        if (iVar7 == local_30) goto LAB_0053d5e7;
        iVar5 = iVar6 + 4;
      } while (this != (C_Trigger_TractorBeam *)piVar1[iVar7]);
    }
    iVar5 = (local_30 - iVar7) + -1;
    if (0 < iVar5) {
      _V_memmove((void *)((int)piVar1 + iVar6),piVar1 + iVar7 + 1,iVar5 * 4);
      local_30 = *(int *)(*(int *)(BaseModUI::SteamCloudConfirmation::OnCommand + unaff_EBX + 6) +
                         0xc);
    }
    *(int *)(*(int *)(BaseModUI::SteamCloudConfirmation::OnCommand + unaff_EBX + 6) + 0xc) =
         local_30 + -1;
  }
LAB_0053d5e7:
  pCVar2 = (this->m_hCoreEffect).m_pObject;
  if (pCVar2 != (CNewParticleEffect *)0x0) {
    pCVar3 = (this->m_hCoreEffect).m_pPrev;
    if (pCVar3 == (CUtlReference<CNewParticleEffect> *)0x0) {
      if ((pCVar2->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
          &this->m_hCoreEffect) {
        pCVar3 = (this->m_hCoreEffect).m_pNext;
        (pCVar2->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
        super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar3;
        if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar3->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
        }
      }
    }
    else {
      pCVar3->m_pNext = (this->m_hCoreEffect).m_pNext;
      pCVar4 = (this->m_hCoreEffect).m_pNext;
      if (pCVar4 != (CUtlReference<CNewParticleEffect> *)0x0) {
        pCVar4->m_pPrev = pCVar3;
      }
    }
    (this->m_hCoreEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
    (this->m_hCoreEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
    (this->m_hCoreEffect).m_pObject = (CNewParticleEffect *)0x0;
  }
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x5f404a) + 8);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable
  ._vptr_IClientRenderable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x5f404a) + 0x3a0);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.
  super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x5f404a) + 0x458);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x5f404a) + 0x494);
  C_BaseEntity::~C_BaseEntity((C_BaseEntity *)this,__in_chrg);
  return;
}


/* C_Trigger_TractorBeam::~C_Trigger_TractorBeam at 0053da30 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void ~C_Trigger_TractorBeam(C_Trigger_TractorBeam * this, int
   __in_chrg) */

void __thiscall
C_Trigger_TractorBeam::~C_Trigger_TractorBeam(C_Trigger_TractorBeam *this,int __in_chrg)

{
  int *piVar1;
  CNewParticleEffect *pCVar2;
  CUtlReference<CNewParticleEffect> *pCVar3;
  CUtlReference<CNewParticleEffect> *pCVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int unaff_EBX;
  int *in_stack_ffffffb8;
  int local_30;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(&UNK_0065d68a + unaff_EBX);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable
  ._vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x65da26);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.
  super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(&UNK_0065dade + unaff_EBX);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x65db1a);
  (this->super_IMotionEvent)._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x65db36);
  local_30 = *(int *)(*(int *)(&DAT_005f3b56 + unaff_EBX) + 0xc);
  if (0 < local_30) {
    piVar1 = (int *)**(undefined4 **)(&DAT_005f3b56 + unaff_EBX);
    if (this == (C_Trigger_TractorBeam *)*piVar1) {
      iVar7 = 0;
      iVar6 = 0;
    }
    else {
      iVar7 = 0;
      iVar5 = 4;
      do {
        iVar6 = iVar5;
        iVar7 = iVar7 + 1;
        if (iVar7 == local_30) goto LAB_0053dad7;
        iVar5 = iVar6 + 4;
      } while (this != (C_Trigger_TractorBeam *)piVar1[iVar7]);
    }
    iVar5 = (local_30 - iVar7) + -1;
    if (0 < iVar5) {
      in_stack_ffffffb8 = piVar1 + iVar7 + 1;
      _V_memmove((void *)((int)piVar1 + iVar6),in_stack_ffffffb8,iVar5 * 4);
      local_30 = *(int *)(*(int *)(&DAT_005f3b56 + unaff_EBX) + 0xc);
    }
    *(int *)(*(int *)(&DAT_005f3b56 + unaff_EBX) + 0xc) = local_30 + -1;
  }
LAB_0053dad7:
  pCVar2 = (this->m_hCoreEffect).m_pObject;
  if (pCVar2 != (CNewParticleEffect *)0x0) {
    pCVar3 = (this->m_hCoreEffect).m_pPrev;
    if (pCVar3 == (CUtlReference<CNewParticleEffect> *)0x0) {
      if ((pCVar2->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
          &this->m_hCoreEffect) {
        pCVar3 = (this->m_hCoreEffect).m_pNext;
        (pCVar2->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
        super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar3;
        if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar3->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
        }
      }
    }
    else {
      pCVar3->m_pNext = (this->m_hCoreEffect).m_pNext;
      pCVar4 = (this->m_hCoreEffect).m_pNext;
      if (pCVar4 != (CUtlReference<CNewParticleEffect> *)0x0) {
        pCVar4->m_pPrev = pCVar3;
      }
    }
    (this->m_hCoreEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
    (this->m_hCoreEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
    (this->m_hCoreEffect).m_pObject = (CNewParticleEffect *)0x0;
  }
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(&DAT_005f3b5a + unaff_EBX) + 8);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable
  ._vptr_IClientRenderable = (_func_int_varargs **)(*(int *)(&DAT_005f3b5a + unaff_EBX) + 0x3a0);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.
  super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(*(int *)(&DAT_005f3b5a + unaff_EBX) + 0x458);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(*(int *)(&DAT_005f3b5a + unaff_EBX) + 0x494);
  C_BaseEntity::~C_BaseEntity((C_BaseEntity *)this,(int)in_stack_ffffffb8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_Trigger_TractorBeam::~C_Trigger_TractorBeam at 0053dca0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void ~C_Trigger_TractorBeam(C_Trigger_TractorBeam * this, int
   __in_chrg) */

void __thiscall
C_Trigger_TractorBeam::~C_Trigger_TractorBeam(C_Trigger_TractorBeam *this,int __in_chrg)

{
  int *piVar1;
  CNewParticleEffect *pCVar2;
  CUtlReference<CNewParticleEffect> *pCVar3;
  CUtlReference<CNewParticleEffect> *pCVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int unaff_EBX;
  int local_30;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x65d41a);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable
  ._vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x65d7b6);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.
  super_IClientNetworkable._vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x65d86e);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x65d8aa);
  (this->super_IMotionEvent)._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x65d8c6);
  local_30 = *(int *)(*(int *)(unaff_EBX + 0x5f38e6) + 0xc);
  if (0 < local_30) {
    piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x5f38e6);
    if (this == (C_Trigger_TractorBeam *)*piVar1) {
      iVar7 = 0;
      iVar6 = 0;
    }
    else {
      iVar7 = 0;
      iVar5 = 4;
      do {
        iVar6 = iVar5;
        iVar7 = iVar7 + 1;
        if (iVar7 == local_30) goto LAB_0053dd47;
        iVar5 = iVar6 + 4;
      } while (this != (C_Trigger_TractorBeam *)piVar1[iVar7]);
    }
    iVar5 = (local_30 - iVar7) + -1;
    if (0 < iVar5) {
      _V_memmove((void *)((int)piVar1 + iVar6),piVar1 + iVar7 + 1,iVar5 * 4);
      local_30 = *(int *)(*(int *)(unaff_EBX + 0x5f38e6) + 0xc);
    }
    *(int *)(*(int *)(unaff_EBX + 0x5f38e6) + 0xc) = local_30 + -1;
  }
LAB_0053dd47:
  pCVar2 = (this->m_hCoreEffect).m_pObject;
  if (pCVar2 != (CNewParticleEffect *)0x0) {
    pCVar3 = (this->m_hCoreEffect).m_pPrev;
    if (pCVar3 == (CUtlReference<CNewParticleEffect> *)0x0) {
      if ((pCVar2->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
          &this->m_hCoreEffect) {
        pCVar3 = (this->m_hCoreEffect).m_pNext;
        (pCVar2->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
        super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar3;
        if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar3->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
        }
      }
    }
    else {
      pCVar3->m_pNext = (this->m_hCoreEffect).m_pNext;
      pCVar4 = (this->m_hCoreEffect).m_pNext;
      if (pCVar4 != (CUtlReference<CNewParticleEffect> *)0x0) {
        pCVar4->m_pPrev = pCVar3;
      }
    }
    (this->m_hCoreEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
    (this->m_hCoreEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
    (this->m_hCoreEffect).m_pObject = (CNewParticleEffect *)0x0;
  }
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x5f38ea) + 8);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable
  ._vptr_IClientRenderable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x5f38ea) + 0x3a0);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.
  super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x5f38ea) + 0x458);
  (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x5f38ea) + 0x494);
  C_BaseEntity::~C_BaseEntity((C_BaseEntity *)this,__in_chrg);
  return;
}


/* C_Trigger_TractorBeam::Spawn at 0053ad60 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void Spawn(C_Trigger_TractorBeam * this) */

void __thiscall C_Trigger_TractorBeam::Spawn(C_Trigger_TractorBeam *this)

{
  IMaterial *pIVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::Spawn((C_BaseEntity *)this);
  if (this->m_pMaterial1 == (IMaterial *)0x0) {
    pIVar1 = (IMaterial *)
             (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5f6157) + 300))
                       ((int *)**(undefined4 **)(unaff_EBX + 0x5f6157),unaff_EBX + 0x4559b4,0,0,0);
    this->m_pMaterial1 = pIVar1;
  }
  if (this->m_pMaterial2 == (IMaterial *)0x0) {
    pIVar1 = (IMaterial *)
             (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5f6157) + 300))
                       ((int *)**(undefined4 **)(unaff_EBX + 0x5f6157),unaff_EBX + 0x4559c9,0,0,0);
    this->m_pMaterial2 = pIVar1;
  }
  if (this->m_pMaterial3 == (IMaterial *)0x0) {
    pIVar1 = (IMaterial *)
             (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5f6157) + 300))
                       ((int *)**(undefined4 **)(unaff_EBX + 0x5f6157),unaff_EBX + 0x4559df,0,0,0);
    this->m_pMaterial3 = pIVar1;
  }
  (*(this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x9b])(this,0xc4a1a000);
  return;
}


/* C_Trigger_TractorBeam::CreateParticles at 0053b2e0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void CreateParticles(C_Trigger_TractorBeam * this) */

void __thiscall C_Trigger_TractorBeam::CreateParticles(C_Trigger_TractorBeam *this)

{
  NetworkVar_m_Particles *this_00;
  uint uVar1;
  float fVar2;
  CNewParticleEffect *pCVar3;
  CUtlReference<CNewParticleEffect> *pCVar4;
  CUtlReference<CNewParticleEffect> *pCVar5;
  Vector *pVVar6;
  CNewParticleEffect *pEffect;
  int unaff_EBX;
  Vector local_b8;
  undefined4 local_ac;
  undefined4 local_a8;
  undefined4 local_a4;
  float local_a0;
  float local_9c;
  float local_98;
  Vector local_94;
  Vector local_88;
  Vector local_7c;
  Vector local_70;
  float local_64;
  float local_60;
  float local_5c;
  Vector local_58;
  vec_t local_4c;
  vec_t local_48;
  vec_t local_44;
  vec_t local_40;
  vec_t local_3c;
  vec_t local_38;
  vec_t local_34;
  vec_t local_30;
  vec_t local_2c;
  vec_t local_28;
  vec_t local_24;
  vec_t local_20;
  
  ___i686_get_pc_thunk_bx();
  pCVar3 = (this->m_hCoreEffect).m_pObject;
  this_00 = &(this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.m_Particles;
  if (pCVar3 != (CNewParticleEffect *)0x0) {
    CParticleProperty::StopEmission(&this_00->super_CParticleProperty,pCVar3,false,false,false,true)
    ;
    pCVar3 = (this->m_hCoreEffect).m_pObject;
    if (pCVar3 != (CNewParticleEffect *)0x0) {
      pCVar4 = (this->m_hCoreEffect).m_pPrev;
      if (pCVar4 == (CUtlReference<CNewParticleEffect> *)0x0) {
        if ((pCVar3->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
            &this->m_hCoreEffect) {
          pCVar4 = (this->m_hCoreEffect).m_pNext;
          (pCVar3->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar4;
          if (pCVar4 != (CUtlReference<CNewParticleEffect> *)0x0) {
            pCVar4->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
          }
        }
      }
      else {
        pCVar4->m_pNext = (this->m_hCoreEffect).m_pNext;
        pCVar5 = (this->m_hCoreEffect).m_pNext;
        if (pCVar5 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar5->m_pPrev = pCVar4;
        }
      }
      (this->m_hCoreEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
      (this->m_hCoreEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
      (this->m_hCoreEffect).m_pObject = (CNewParticleEffect *)0x0;
    }
  }
  pVVar6 = *(Vector **)(SetFlyoutButtonText + unaff_EBX + 3);
  local_28 = pVVar6->x;
  local_24 = pVVar6->y;
  local_20 = pVVar6->z;
  pEffect = CParticleProperty::Create
                      (&this_00->super_CParticleProperty,&UNK_004554b9 + unaff_EBX,
                       PATTACH_CUSTOMORIGIN,-1,*pVVar6,(matrix3x4_t *)0x0);
  pCVar3 = (this->m_hCoreEffect).m_pObject;
  if (pEffect != pCVar3) {
    if (pCVar3 != (CNewParticleEffect *)0x0) {
      pCVar4 = (this->m_hCoreEffect).m_pPrev;
      if (pCVar4 == (CUtlReference<CNewParticleEffect> *)0x0) {
        if ((pCVar3->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
            &this->m_hCoreEffect) {
          pCVar4 = (this->m_hCoreEffect).m_pNext;
          (pCVar3->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar4;
          if (pCVar4 != (CUtlReference<CNewParticleEffect> *)0x0) {
            pCVar4->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
          }
        }
      }
      else {
        pCVar4->m_pNext = (this->m_hCoreEffect).m_pNext;
        pCVar5 = (this->m_hCoreEffect).m_pNext;
        if (pCVar5 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar5->m_pPrev = pCVar4;
        }
      }
      (this->m_hCoreEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
      (this->m_hCoreEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
      (this->m_hCoreEffect).m_pObject = (CNewParticleEffect *)0x0;
    }
    (this->m_hCoreEffect).m_pObject = pEffect;
    if (pEffect == (CNewParticleEffect *)0x0) {
      return;
    }
    (this->m_hCoreEffect).m_pNext =
         (pEffect->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
         super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead;
    pCVar4 = (pEffect->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
             super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead;
    if (pCVar4 != (CUtlReference<CNewParticleEffect> *)0x0) {
      pCVar4->m_pPrev = &this->m_hCoreEffect;
    }
    (this->m_hCoreEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
    (pEffect->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
    super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = &this->m_hCoreEffect;
    pEffect = (this->m_hCoreEffect).m_pObject;
  }
  if (pEffect != (CNewParticleEffect *)0x0) {
    pVVar6 = *(Vector **)(SetFlyoutButtonText + unaff_EBX + 3);
    local_34 = pVVar6->x;
    local_30 = pVVar6->y;
    local_2c = pVVar6->z;
    CParticleProperty::AddControlPoint
              (&this_00->super_CParticleProperty,pEffect,1,(C_BaseEntity *)this,PATTACH_CUSTOMORIGIN
               ,(char *)0x0,*pVVar6,(matrix3x4_t *)0x0);
    pVVar6 = *(Vector **)(SetFlyoutButtonText + unaff_EBX + 3);
    local_40 = pVVar6->x;
    local_3c = pVVar6->y;
    local_38 = pVVar6->z;
    CParticleProperty::AddControlPoint
              (&this_00->super_CParticleProperty,(this->m_hCoreEffect).m_pObject,2,
               (C_BaseEntity *)this,PATTACH_CUSTOMORIGIN,(char *)0x0,*pVVar6,(matrix3x4_t *)0x0);
    local_4c = pVVar6->x;
    local_48 = pVVar6->y;
    local_44 = pVVar6->z;
    CParticleProperty::AddControlPoint
              (&this_00->super_CParticleProperty,(this->m_hCoreEffect).m_pObject,3,
               (C_BaseEntity *)this,PATTACH_CUSTOMORIGIN,(char *)0x0,*pVVar6,(matrix3x4_t *)0x0);
                    /* Unresolved local var: Vector res@[???] */
    local_64 = (this->m_vEnd).x - (this->m_vStart).x;
    local_60 = (this->m_vEnd).y - (this->m_vStart).y;
    local_5c = (this->m_vEnd).z - (this->m_vStart).z;
    local_58.x = local_64;
    local_58.y = local_60;
    local_58.z = local_5c;
    VectorNormalize(&local_58);
    VectorVectors(&local_58,&local_70,&local_7c);
    CNewParticleEffect::SetControlPoint((this->m_hCoreEffect).m_pObject,0,&this->m_vStart);
    CNewParticleEffect::SetControlPointOrientation
              ((this->m_hCoreEffect).m_pObject,0,&local_58,&local_70,&local_7c);
    CNewParticleEffect::SetControlPoint((this->m_hCoreEffect).m_pObject,1,&this->m_vEnd);
    uVar1 = *(uint *)(unaff_EBX + 0x48051f);
    local_88.z = (vec_t)((uint)local_58.z ^ uVar1);
    local_88.y = (vec_t)((uint)local_58.y ^ uVar1);
    local_88.x = (vec_t)((uint)local_58.x ^ uVar1);
    CNewParticleEffect::SetControlPointOrientation
              ((this->m_hCoreEffect).m_pObject,1,&local_88,&local_70,&local_7c);
    fVar2 = this->m_linearForce;
                    /* Unresolved local var: matrix3x4_t * mat@[???] */
    C_BaseEntity::CalcAbsolutePosition((C_BaseEntity *)this);
                    /* Unresolved local var: Vector res@[???] */
    local_a0 = fVar2 * (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.m_rgflCoordinateFrame.
                       m_flMatVal[0][0];
    local_9c = (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.m_rgflCoordinateFrame.
               m_flMatVal[1][0] * fVar2;
    local_98 = fVar2 * (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.m_rgflCoordinateFrame.
                       m_flMatVal[2][0];
    local_94.x = local_a0;
    local_94.y = local_9c;
    local_94.z = local_98;
    CNewParticleEffect::SetControlPoint((this->m_hCoreEffect).m_pObject,2,&local_94);
    if (this->m_bReversed == false) {
      local_ac = 0x42800000;
      local_a4 = 0x437f0000;
    }
    else {
      local_ac = 0x437f0000;
      local_a4 = 0x42000000;
    }
    local_a8 = 0x43200000;
    local_b8.x = 64.0;
    local_b8.y = 160.0;
    local_b8.z = 255.0;
    CNewParticleEffect::SetControlPoint((this->m_hCoreEffect).m_pObject,3,&local_b8);
  }
  return;
}


/* C_Trigger_TractorBeam::UpdateOnRemove at 0053b980 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void UpdateOnRemove(C_Trigger_TractorBeam * this) */

void __thiscall C_Trigger_TractorBeam::UpdateOnRemove(C_Trigger_TractorBeam *this)

{
  IPhysicsMotionController *pIVar1;
  _func_int_varargs **pp_Var2;
  _func_int_varargs *p_Var3;
  int *piVar4;
  int iVar5;
  code *pcVar6;
  CNewParticleEffect *pCVar7;
  CUtlReference<CNewParticleEffect> *pCVar8;
  CUtlReference<CNewParticleEffect> *pCVar9;
  int iVar10;
  C_Trigger_TractorBeam *pCVar11;
  int iVar12;
  int unaff_EBX;
  IPhysicsMotionController **ppIVar13;
  int iVar14;
  undefined4 uStack_60;
  IPhysicsMotionController *local_5c;
  int local_58 [7];
  undefined1 auStack_3c [8];
  undefined1 *local_34;
  C_Trigger_TractorBeam *local_30;
  int local_20;
  
  ppIVar13 = &local_5c;
  uStack_60 = 0x53b98e;
  ___i686_get_pc_thunk_bx();
  local_30 = this;
  local_20 = **(int **)(unaff_EBX + 0x5f5386);
  pIVar1 = this->m_pController;
  if (pIVar1 != (IPhysicsMotionController *)0x0) {
                    /* Unresolved local var: int nNumObjects@[???]
                       Unresolved local var: IPhysicsObject * * pObjects@[???] */
    uStack_60 = 0x53b9b7;
    local_5c = pIVar1;
    iVar12 = (*pIVar1->_vptr_IPhysicsMotionController[5])();
    iVar10 = -((iVar12 * 4 + 0xfU & 0xfffffff0) + 0x10);
    ppIVar13 = (IPhysicsMotionController **)((int)local_58 + iVar10 + -4);
    local_34 = auStack_3c + iVar10;
    pIVar1 = local_30->m_pController;
    pp_Var2 = pIVar1->_vptr_IPhysicsMotionController;
    *(undefined1 **)((int)local_58 + iVar10) = local_34;
    *(IPhysicsMotionController **)((int)local_58 + iVar10 + -4) = pIVar1;
    p_Var3 = pp_Var2[6];
    *(undefined4 *)((int)&uStack_60 + iVar10) = 0x53b9e4;
    (*p_Var3)();
                    /* Unresolved local var: int i@[???] */
    if (0 < iVar12) {
      iVar14 = 0;
      do {
        piVar4 = *(int **)(local_34 + iVar14 * 4);
        if (piVar4 != (int *)0x0) {
          iVar5 = *piVar4;
          *(int **)((int)local_58 + iVar10 + -4) = piVar4;
          pcVar6 = *(code **)(iVar5 + 100);
          *(undefined4 *)((int)&uStack_60 + iVar10) = 0x53ba02;
          (*pcVar6)();
        }
        iVar14 = iVar14 + 1;
      } while (iVar14 != iVar12);
    }
    pCVar11 = local_30;
    piVar4 = (int *)**(undefined4 **)(unaff_EBX + 0x5f5582);
    iVar12 = *piVar4;
    *(IPhysicsMotionController **)((int)local_58 + iVar10) = local_30->m_pController;
    *(int **)((int)local_58 + iVar10 + -4) = piVar4;
    pcVar6 = *(code **)(iVar12 + 0x7c);
    *(undefined4 *)((int)&uStack_60 + iVar10) = 0x53ba24;
    (*pcVar6)();
    pCVar11->m_pController = (IPhysicsMotionController *)0x0;
  }
  pCVar11 = local_30;
  pCVar7 = (local_30->m_hCoreEffect).m_pObject;
  if (pCVar7 != (CNewParticleEffect *)0x0) {
    ppIVar13[5] = (IPhysicsMotionController *)0x1;
    ppIVar13[4] = (IPhysicsMotionController *)0x0;
    ppIVar13[3] = (IPhysicsMotionController *)0x0;
    ppIVar13[2] = (IPhysicsMotionController *)0x0;
    ppIVar13[1] = (IPhysicsMotionController *)pCVar7;
    *ppIVar13 = (IPhysicsMotionController *)
                &(pCVar11->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.m_Particles;
    ppIVar13[-1] = (IPhysicsMotionController *)0x53ba67;
    CParticleProperty::StopEmission
              ((CParticleProperty *)*ppIVar13,(CNewParticleEffect *)ppIVar13[1],SUB41(ppIVar13[2],0)
               ,SUB41(ppIVar13[3],0),SUB41(ppIVar13[4],0),SUB41(ppIVar13[5],0));
    pCVar7 = (local_30->m_hCoreEffect).m_pObject;
    if (pCVar7 != (CNewParticleEffect *)0x0) {
      pCVar8 = (local_30->m_hCoreEffect).m_pPrev;
      if (pCVar8 == (CUtlReference<CNewParticleEffect> *)0x0) {
        if ((pCVar7->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
            &local_30->m_hCoreEffect) {
          pCVar8 = (local_30->m_hCoreEffect).m_pNext;
          (pCVar7->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar8;
          if (pCVar8 != (CUtlReference<CNewParticleEffect> *)0x0) {
            pCVar8->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
          }
        }
      }
      else {
        pCVar8->m_pNext = (local_30->m_hCoreEffect).m_pNext;
        pCVar9 = (local_30->m_hCoreEffect).m_pNext;
        if (pCVar9 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar9->m_pPrev = pCVar8;
        }
      }
      (local_30->m_hCoreEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
      (local_30->m_hCoreEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
      (local_30->m_hCoreEffect).m_pObject = (CNewParticleEffect *)0x0;
    }
  }
  *ppIVar13 = (IPhysicsMotionController *)local_30;
  ppIVar13[-1] = (IPhysicsMotionController *)0x53bab9;
  C_BaseEntity::UpdateOnRemove((C_BaseEntity *)*ppIVar13);
  if (local_20 == **(int **)(unaff_EBX + 0x5f5386)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  ppIVar13[-1] = (IPhysicsMotionController *)&UNK_0053baff;
  ___stack_chk_fail();
}


/* C_Trigger_TractorBeam::OnDataChanged at 0053b8d0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void OnDataChanged(C_Trigger_TractorBeam * this, DataUpdateType_t
   updateType) */

void __thiscall
C_Trigger_TractorBeam::OnDataChanged(C_Trigger_TractorBeam *this,DataUpdateType_t updateType)

{
  int *piVar1;
  IPhysicsMotionController *pIVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::OnDataChanged((C_BaseEntity *)this,updateType);
  if (updateType == DATA_UPDATE_CREATED) {
    C_BaseEntity::AddEffects((C_BaseEntity *)this,0x400);
    piVar1 = (int *)**(int **)(unaff_EBX + 0x5f562c);
    if ((piVar1 != (int *)0x0) && (this->m_pController == (IPhysicsMotionController *)0x0)) {
      pIVar2 = (IPhysicsMotionController *)
               (**(code **)(*piVar1 + 0x78))(piVar1,&this->super_IMotionEvent);
      this->m_pController = pIVar2;
      UpdateBeam(this);
    }
  }
  else {
    if (this->m_bRecreateParticles == false) {
      return;
    }
    this->m_bRecreateParticles = false;
  }
  CreateParticles(this);
  return;
}


/* C_Trigger_TractorBeam::GetPredictionOwner at 0053bf70 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: C_BasePlayer * GetPredictionOwner(C_Trigger_TractorBeam * this) */

C_BasePlayer * __thiscall C_Trigger_TractorBeam::GetPredictionOwner(C_Trigger_TractorBeam *this)

{
  uint uVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.m_hPlayerSimulationOwner.
          super_CBaseHandle.m_Index;
  if ((uVar1 != 0xffffffff) &&
     (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5f4e17),
     *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) {
    return *(C_BasePlayer **)(iVar2 + 4);
  }
  return (C_BasePlayer *)0x0;
}


/* C_Trigger_TractorBeam::UpdatePartitionListEntry at 0053acf0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void UpdatePartitionListEntry(C_Trigger_TractorBeam * this) */

void __thiscall C_Trigger_TractorBeam::UpdatePartitionListEntry(C_Trigger_TractorBeam *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5f6168) + 0x14))
            ((int *)**(undefined4 **)(unaff_EBX + 0x5f6168),0x8c,0x100,
             (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.m_Collision.
             super_CCollisionProperty.m_Partition);
  return;
}


/* C_Trigger_TractorBeam::GetSoundSpatialization at 0053bdd0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: bool GetSoundSpatialization(C_Trigger_TractorBeam * this,
   SpatializationInfo_t * info) */

bool __thiscall
C_Trigger_TractorBeam::GetSoundSpatialization
          (C_Trigger_TractorBeam *this,SpatializationInfo_t *info)

{
  Vector *pVVar1;
  QAngle *pQVar2;
  int iVar3;
  C_BasePlayer *pCVar4;
  vec_t *pvVar5;
  bool bVar6;
  int unaff_EBX;
  float local_20 [4];
  
  ___i686_get_pc_thunk_bx();
  iVar3 = (*(this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.
            super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0x4c])(this);
  bVar6 = false;
  if ((char)iVar3 == '\0') {
    if (info->pOrigin != (Vector *)0x0) {
      pCVar4 = C_BasePlayer::GetLocalPlayer(-1);
      if (pCVar4 != (C_BasePlayer *)0x0) {
        CalcClosestPointOnLine
                  (&(info->info).vListenerOrigin,&this->m_vStart,&this->m_vEnd,info->pOrigin,
                   local_20);
        if ((*(float *)(unaff_EBX + 0x47ab70) <= local_20[0]) ||
           (NAN(local_20[0]) || NAN(*(float *)(unaff_EBX + 0x47ab70)))) {
          if (*(float *)(unaff_EBX + 0x47aa60) <= local_20[0] &&
              local_20[0] != *(float *)(unaff_EBX + 0x47aa60)) {
            pVVar1 = info->pOrigin;
            pVVar1->x = (this->m_vEnd).x;
            pVVar1->y = (this->m_vEnd).y;
            pVVar1->z = (this->m_vEnd).z;
          }
        }
        else {
          pVVar1 = info->pOrigin;
          pVVar1->x = (this->m_vStart).x;
          pVVar1->y = (this->m_vStart).y;
          pVVar1->z = (this->m_vStart).z;
        }
      }
    }
    pQVar2 = info->pAngles;
    bVar6 = true;
    if (pQVar2 != (QAngle *)0x0) {
      pvVar5 = (vec_t *)(*(this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.m_Collision.
                          super_CCollisionProperty.super_ICollideable._vptr_ICollideable[9])
                                  (&(this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.
                                    m_Collision);
      pQVar2->x = *pvVar5;
      pQVar2->y = pvVar5[1];
      pQVar2->z = pvVar5[2];
      bVar6 = true;
    }
  }
  return bVar6;
}


/* C_Trigger_TractorBeam::StartTouch at 0053c050 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void StartTouch(C_Trigger_TractorBeam * this, C_BaseEntity * pOther) */

void __thiscall C_Trigger_TractorBeam::StartTouch(C_Trigger_TractorBeam *this,C_BaseEntity *pOther)

{
  IPhysicsMotionController *pIVar1;
  int iVar2;
  
  pIVar1 = this->m_pController;
                    /* Unresolved local var: IPhysicsObject * pPhysObject@[???] */
  if ((pIVar1 != (IPhysicsMotionController *)0x0) &&
     (pOther->m_pPhysicsObject != (IPhysicsObject *)0x0)) {
    (*pIVar1->_vptr_IPhysicsMotionController[3])(pIVar1,pOther->m_pPhysicsObject,1);
  }
  iVar2 = (*(pOther->super_IClientEntity).super_IClientUnknown.super_IHandleEntity.
            _vptr_IHandleEntity[0xc0])(pOther);
  if ((char)iVar2 != '\0') {
                    /* Unresolved local var: C_BasePlayer * pPlayer@[???] */
    iVar2 = (*(pOther->super_IClientEntity).super_IClientUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[0xc0])(pOther);
    if ((char)iVar2 != '\0') {
      C_BasePlayer::SetPhysicsFlag((C_BasePlayer *)pOther,0x10,true);
      *(undefined1 *)&pOther[2].m_pfnThink.__delta = 1;
      pOther->m_flGravity = 1.1754944e-38;
    }
  }
  C_BaseEntity::StartTouch((C_BaseEntity *)this,pOther);
  return;
}


/* C_Trigger_TractorBeam::EndTouch at 0053bd40 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void EndTouch(C_Trigger_TractorBeam * this, C_BaseEntity * pOther) */

void __thiscall C_Trigger_TractorBeam::EndTouch(C_Trigger_TractorBeam *this,C_BaseEntity *pOther)

{
  IPhysicsMotionController *pIVar1;
  int iVar2;
  
  pIVar1 = this->m_pController;
                    /* Unresolved local var: IPhysicsObject * pPhysObject@[???] */
  if ((pIVar1 != (IPhysicsMotionController *)0x0) &&
     (pOther->m_pPhysicsObject != (IPhysicsObject *)0x0)) {
    (*pIVar1->_vptr_IPhysicsMotionController[4])(pIVar1,pOther->m_pPhysicsObject);
  }
  iVar2 = (*(pOther->super_IClientEntity).super_IClientUnknown.super_IHandleEntity.
            _vptr_IHandleEntity[0xc0])(pOther);
  if ((char)iVar2 != '\0') {
                    /* Unresolved local var: C_BasePlayer * pPlayer@[???] */
    iVar2 = (*(pOther->super_IClientEntity).super_IClientUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[0xc0])(pOther);
    if ((char)iVar2 != '\0') {
      C_BasePlayer::SetPhysicsFlag((C_BasePlayer *)pOther,0x10,false);
      *(undefined1 *)&pOther[2].m_pfnThink.__delta = 0;
      pOther->m_flGravity = 1.0;
    }
  }
  C_BaseEntity::EndTouch((C_BaseEntity *)this,pOther);
  return;
}


/* C_Trigger_TractorBeam::PhysicsSimulate at 0053ae70 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void PhysicsSimulate(C_Trigger_TractorBeam * this) */

void __thiscall C_Trigger_TractorBeam::PhysicsSimulate(C_Trigger_TractorBeam *this)

{
  C_BaseEntity::PhysicsSimulate((C_BaseEntity *)this);
  return;
}


/* C_Trigger_TractorBeam::DrawColumnSegment at 0053c0f0 */

/* WARNING: Restarted to delay deadcode elimination for space: stack */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void DrawColumnSegment(C_Trigger_TractorBeam * this, CMeshBuilder *
   meshBuilder, Vector * vecStart, Vector * vDir, float flLength, Vector * vecXAxis, Vector *
   vecYAxis, float flRadius, float flAlpha, float flTextureOffset) */

void __thiscall
C_Trigger_TractorBeam::DrawColumnSegment
          (C_Trigger_TractorBeam *this,CMeshBuilder *meshBuilder,Vector *vecStart,Vector *vDir,
          float flLength,Vector *vecXAxis,Vector *vecYAxis,float flRadius,float flAlpha,
          float flTextureOffset)

{
  float **ppfVar1;
  uchar **ppuVar2;
  int iVar3;
  float *pfVar4;
  longdouble lVar5;
  int iVar6;
  int unaff_EBX;
  longdouble lVar7;
  float fVar8;
  float fVar9;
  float fVar10;
  float fVar11;
  float fVar12;
  float fVar13;
  float fVar14;
  float fVar15;
  float fVar16;
  float fVar17;
  float fVar18;
  int local_118;
  float local_104;
  float local_dc;
  float local_d8;
  float local_d4;
  float local_d0;
  
                    /* Unresolved local var: Vector vStart@[???]
                       Unresolved local var: Vector vecPosition@[???]
                       Unresolved local var: Vector vecLastPosition@[???]
                       Unresolved local var: float flRadStep@[???]
                       Unresolved local var: float flSin@[???]
                       Unresolved local var: float flCos@[???]
                       Unresolved local var: float flV@[???]
                       Unresolved local var: int r@[???]
                       Unresolved local var: int g@[???]
                       Unresolved local var: int b@[???]
                       Unresolved local var: float flLastV@[???] */
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
  fVar17 = vecStart->x;
  fVar15 = vecStart->y;
  fVar14 = vecStart->z;
  if (this->m_bReversed == false) {
    fVar12 = *(float *)(unaff_EBX + 0x47f6f7);
    local_104 = *(float *)(unaff_EBX + 0x47a743);
  }
  else {
    fVar12 = *(float *)(unaff_EBX + 0x47a743);
    local_104 = *(float *)(unaff_EBX + 0x47f6f3);
  }
  fVar10 = flLength * *(float *)(unaff_EBX + 0x47dd3b);
  fVar9 = *(float *)(unaff_EBX + 0x47aaff);
  fVar8 = *(float *)(&DAT_0047c5f3 + unaff_EBX);
  fVar12 = fVar12 * flAlpha * fVar9 + fVar8;
  fVar13 = flAlpha * *(float *)(unaff_EBX + 0x47f6fb) * fVar9 + fVar8;
  fVar8 = fVar8 + fVar9 * flAlpha * local_104;
  local_dc = 0.0;
  local_118 = 1;
  iVar3 = *(int *)(unaff_EBX + 0x47f2df);
  local_d8 = fVar14 + flRadius * vecXAxis->z;
  local_d4 = fVar15 + flRadius * vecXAxis->y;
  local_d0 = fVar17 + flRadius * vecXAxis->x;
  while( true ) {
                    /* Unresolved local var: Vector vert@[???]
                       Unresolved local var: float flU@[???]
                       Unresolved local var: float[4] pColors@[???] */
                    /* Unresolved local var: double __cosr@[???]
                       Unresolved local var: double __sinr@[???] */
    lVar7 = (longdouble)(*(float *)(unaff_EBX + 0x47f6ff) * (float)local_118);
    lVar5 = (longdouble)fcos(lVar7);
    lVar7 = (longdouble)fsin(lVar7);
    fVar9 = (float)lVar7;
    fVar11 = (float)lVar5;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
    fVar18 = fVar17 + fVar11 * vecXAxis->x * flRadius + fVar9 * vecYAxis->x * flRadius;
    fVar16 = fVar15 + fVar11 * vecXAxis->y * flRadius + fVar9 * vecYAxis->y * flRadius;
    fVar11 = fVar14 + fVar11 * vecXAxis->z * flRadius + fVar9 * vecYAxis->z * flRadius;
    fVar9 = (float)local_118 * *(float *)(unaff_EBX + 0x47abd7);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
    *(uint *)(meshBuilder->m_VertexBuilder).m_pCurrColor =
         ((uint)fVar13 & 0xff) << 8 | ((uint)fVar8 & 0xff) << 0x10 | (uint)fVar12 & 0xff |
         iVar3 << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
    pfVar4 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[0];
    *pfVar4 = 0.0;
    pfVar4[1] = fVar9;
                    /* Unresolved local var: float * pDst@[???] */
    pfVar4 = (meshBuilder->m_VertexBuilder).m_pCurrPosition;
    *pfVar4 = fVar18;
    pfVar4[1] = fVar16;
    pfVar4[2] = fVar11;
    iVar6 = (meshBuilder->m_VertexBuilder).m_nCurrentVertex + 1;
    (meshBuilder->m_VertexBuilder).m_nCurrentVertex = iVar6;
    if ((meshBuilder->m_VertexBuilder).m_nVertexCount < iVar6) {
      (meshBuilder->m_VertexBuilder).m_nVertexCount = iVar6;
    }
    ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrPosition;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Position);
    ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrNormal;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Normal);
    ppuVar2 = &(meshBuilder->m_VertexBuilder).m_pCurrColor;
    *ppuVar2 = *ppuVar2 + (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Color;
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[0]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 1;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[1]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 2;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[2]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 3;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[3]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 4;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[4]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 5;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[5]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 6;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[6]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 7;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[7]);
                    /* Unresolved local var: Vector res@[???] */
    fVar17 = vDir->x;
    fVar15 = vDir->y;
    fVar14 = vDir->z;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
    *(uint *)(meshBuilder->m_VertexBuilder).m_pCurrColor =
         ((uint)fVar13 & 0xff) << 8 | ((uint)fVar8 & 0xff) << 0x10 | (uint)fVar12 & 0xff |
         iVar3 << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
    pfVar4 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[0];
    *pfVar4 = fVar10;
    pfVar4[1] = fVar9;
                    /* Unresolved local var: float * pDst@[???] */
    pfVar4 = (meshBuilder->m_VertexBuilder).m_pCurrPosition;
    *pfVar4 = fVar18 + flLength * fVar17;
    pfVar4[1] = fVar16 + flLength * fVar15;
    pfVar4[2] = fVar11 + flLength * fVar14;
    iVar6 = (meshBuilder->m_VertexBuilder).m_nCurrentVertex + 1;
    (meshBuilder->m_VertexBuilder).m_nCurrentVertex = iVar6;
    if ((meshBuilder->m_VertexBuilder).m_nVertexCount < iVar6) {
      (meshBuilder->m_VertexBuilder).m_nVertexCount = iVar6;
    }
    ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrPosition;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Position);
    ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrNormal;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Normal);
    ppuVar2 = &(meshBuilder->m_VertexBuilder).m_pCurrColor;
    *ppuVar2 = *ppuVar2 + (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Color;
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[0]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 1;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[1]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 2;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[2]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 3;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[3]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 4;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[4]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 5;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[5]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 6;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[6]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 7;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[7]);
                    /* Unresolved local var: Vector res@[???] */
    fVar17 = vDir->x;
    fVar15 = vDir->y;
    fVar14 = vDir->z;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
    *(uint *)(meshBuilder->m_VertexBuilder).m_pCurrColor =
         ((uint)fVar13 & 0xff) << 8 | ((uint)fVar8 & 0xff) << 0x10 | (uint)fVar12 & 0xff |
         iVar3 << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
    pfVar4 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[0];
    *pfVar4 = fVar10;
    pfVar4[1] = local_dc;
                    /* Unresolved local var: float * pDst@[???] */
    pfVar4 = (meshBuilder->m_VertexBuilder).m_pCurrPosition;
    *pfVar4 = local_d0 + flLength * fVar17;
    pfVar4[1] = local_d4 + flLength * fVar15;
    pfVar4[2] = local_d8 + flLength * fVar14;
    iVar6 = (meshBuilder->m_VertexBuilder).m_nCurrentVertex + 1;
    (meshBuilder->m_VertexBuilder).m_nCurrentVertex = iVar6;
    if ((meshBuilder->m_VertexBuilder).m_nVertexCount < iVar6) {
      (meshBuilder->m_VertexBuilder).m_nVertexCount = iVar6;
    }
    ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrPosition;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Position);
    ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrNormal;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Normal);
    ppuVar2 = &(meshBuilder->m_VertexBuilder).m_pCurrColor;
    *ppuVar2 = *ppuVar2 + (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Color;
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[0]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 1;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[1]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 2;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[2]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 3;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[3]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 4;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[4]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 5;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[5]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 6;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[6]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 7;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[7]);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
    *(uint *)(meshBuilder->m_VertexBuilder).m_pCurrColor =
         ((uint)fVar13 & 0xff) << 8 | ((uint)fVar8 & 0xff) << 0x10 | (uint)fVar12 & 0xff |
         iVar3 << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
    pfVar4 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[0];
    *pfVar4 = 0.0;
    pfVar4[1] = local_dc;
                    /* Unresolved local var: float * pDst@[???] */
    pfVar4 = (meshBuilder->m_VertexBuilder).m_pCurrPosition;
    *pfVar4 = local_d0;
    pfVar4[1] = local_d4;
    pfVar4[2] = local_d8;
    iVar6 = (meshBuilder->m_VertexBuilder).m_nCurrentVertex + 1;
    (meshBuilder->m_VertexBuilder).m_nCurrentVertex = iVar6;
    if ((meshBuilder->m_VertexBuilder).m_nVertexCount < iVar6) {
      (meshBuilder->m_VertexBuilder).m_nVertexCount = iVar6;
    }
    ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrPosition;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Position);
    ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrNormal;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Normal);
    ppuVar2 = &(meshBuilder->m_VertexBuilder).m_pCurrColor;
    *ppuVar2 = *ppuVar2 + (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Color;
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[0]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 1;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[1]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 2;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[2]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 3;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[3]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 4;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[4]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 5;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[5]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 6;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[6]);
    ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 7;
    *ppfVar1 = (float *)((int)*ppfVar1 +
                        (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[7]);
    local_118 = local_118 + 1;
    if (local_118 == 0x41) break;
                    /* Unresolved local var: int i@[???] */
    fVar17 = vecStart->x;
    fVar15 = vecStart->y;
    fVar14 = vecStart->z;
    local_dc = fVar9;
    local_d8 = fVar11;
    local_d4 = fVar16;
    local_d0 = fVar18;
  }
                    /* Unresolved local var: Vector res@[???] */
  fVar17 = vDir->y;
  fVar15 = vDir->z;
                    /* Unresolved local var: Vector res@[???] */
  vecStart->x = vecStart->x + flLength * vDir->x;
  vecStart->y = vecStart->y + flLength * fVar17;
  vecStart->z = vecStart->z + flLength * fVar15;
  return;
}


/* C_Trigger_TractorBeam::DrawColumn at 0053cac0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void DrawColumn(C_Trigger_TractorBeam * this, IMaterial * pMaterial,
   Vector * vecStart, Vector * vDir, float flLength, Vector * vecXAxis, Vector * vecYAxis, float
   flRadius, float flAlpha, bool bPinchIn, bool bPinchOut, float flTextureOffset) */

void __thiscall
C_Trigger_TractorBeam::DrawColumn
          (C_Trigger_TractorBeam *this,IMaterial *pMaterial,Vector *vecStart,Vector *vDir,
          float flLength,Vector *vecXAxis,Vector *vecYAxis,float flRadius,float flAlpha,
          bool bPinchIn,bool bPinchOut,float flTextureOffset)

{
  code *pcVar1;
  int *piVar2;
  int iVar3;
  IMesh *pIVar4;
  int unaff_EBX;
  CMeshBuilder local_214;
  Vector local_2c;
  int *local_20;
  
                    /* Unresolved local var: CMatRenderContextPtr pRenderContext@[???]
                       Unresolved local var: IMesh * pMesh@[???]
                       Unresolved local var: CMeshBuilder meshBuilder@[???]
                       Unresolved local var: int nTotalSegments@[???]
                       Unresolved local var: float flMainLength@[???]
                       Unresolved local var: Vector vStart@[???] */
  ___i686_get_pc_thunk_bx();
  local_20 = (int *)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5f43f7) + 400))
                              ((int *)**(undefined4 **)(unaff_EBX + 0x5f43f7));
  if (local_20 != (int *)0x0) {
    (**(code **)(*local_20 + 8))(local_20);
  }
  piVar2 = local_20;
  pcVar1 = *(code **)(*local_20 + 0x24);
  iVar3 = (*(this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.super_IClientEntity.
            super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[6])(this);
  (*pcVar1)(piVar2,pMaterial,iVar3);
  pIVar4 = (IMesh *)(**(code **)(*local_20 + 0xe4))(local_20,1,0,0,pMaterial);
  local_214.m_IndexBuilder.m_pIndexBuffer = (IIndexBuffer *)0x0;
  local_214.m_IndexBuilder.m_nMaxIndexCount = 0;
  local_214.m_IndexBuilder.m_nIndexCount = 0;
  local_214.m_IndexBuilder.m_nCurrentIndex = 0;
  local_214.m_IndexBuilder.m_nTotalIndexCount = 0;
  local_214.m_IndexBuilder.m_nBufferOffset = 0xffffffff;
  local_214.m_IndexBuilder.m_nBufferFirstIndex = 0;
  local_214.m_VertexBuilder.m_pVertexBuffer = (IVertexBuffer *)0x0;
  local_214.m_VertexBuilder.m_nBufferOffset = 0xffffffff;
  local_214.m_VertexBuilder.m_nBufferFirstVertex = 0;
  local_214.m_VertexBuilder.m_nVertexCount = 0;
  local_214.m_VertexBuilder.m_nCurrentVertex = 0;
  local_214.m_VertexBuilder.m_nMaxVertexCount = 0;
  local_214.m_VertexBuilder.m_nTotalVertexCount = 0;
  local_214.m_VertexBuilder.super_VertexDesc_t.m_CompressionType = VERTEX_COMPRESSION_INVALID;
  iVar3 = (-(uint)!bPinchOut & 0xfffffe00) + 0x240;
  if (bPinchIn) {
    iVar3 = (-(uint)!bPinchOut & 0xfffffe00) + 0x440;
  }
  local_214.m_bGenerateIndices = true;
  local_214.m_Type = MATERIAL_QUADS;
  local_214.m_pMesh = pIVar4;
  (*(pIVar4->super_IVertexBuffer)._vptr_IVertexBuffer[10])(pIVar4,2);
  (*((local_214.m_pMesh)->super_IVertexBuffer)._vptr_IVertexBuffer[0x11])
            (local_214.m_pMesh,iVar3 << 4,iVar3 * 0x18,&local_214);
  local_214.m_IndexBuilder.m_pIndexBuffer = (IIndexBuffer *)0x0;
  if (pIVar4 != (IMesh *)0x0) {
    local_214.m_IndexBuilder.m_pIndexBuffer = &pIVar4->super_IIndexBuffer;
  }
  local_214.m_IndexBuilder.m_nIndexCount = 0;
  local_214.m_IndexBuilder.m_bModify = false;
  local_214.m_IndexBuilder.m_nIndexOffset =
       local_214.super_MeshDesc_t.super_VertexDesc_t.m_nFirstVertex;
  local_214.m_IndexBuilder.super_IndexDesc_t.m_pIndices =
       local_214.super_MeshDesc_t.super_IndexDesc_t.m_pIndices;
  local_214.m_IndexBuilder.super_IndexDesc_t.m_nIndexSize =
       local_214.super_MeshDesc_t.super_IndexDesc_t.m_nIndexSize;
  local_214.m_IndexBuilder.m_nCurrentIndex = 0;
  local_214.m_IndexBuilder.m_nMaxIndexCount = iVar3 * 0x18;
  local_214.m_VertexBuilder.m_pVertexBuffer = (IVertexBuffer *)pIVar4;
  _memcpy(&local_214.m_VertexBuilder,&local_214,0xa4);
  local_214.m_VertexBuilder.super_VertexDesc_t.m_NumBoneWeights =
       ~-(uint)(local_214.m_VertexBuilder.super_VertexDesc_t.m_NumBoneWeights == 0) & 2;
  local_214.m_VertexBuilder.m_nVertexCount = 0;
  local_214.m_VertexBuilder.m_bModify = false;
  if (local_214.m_VertexBuilder.m_nBufferOffset == 0xffffffff) {
    local_214.m_VertexBuilder.m_nTotalVertexCount = 0;
    local_214.m_VertexBuilder.m_nBufferOffset =
         local_214.super_MeshDesc_t.super_VertexDesc_t.m_nOffset;
    local_214.m_VertexBuilder.m_nBufferFirstVertex =
         local_214.super_MeshDesc_t.super_VertexDesc_t.m_nFirstVertex;
  }
  local_214.m_IndexBuilder.m_nCurrentIndex = 0;
  local_214.m_VertexBuilder.m_nCurrentVertex = 0;
  local_214.m_VertexBuilder.m_pCurrPosition =
       local_214.m_VertexBuilder.super_VertexDesc_t.m_pPosition;
  local_214.m_VertexBuilder.m_pCurrNormal = local_214.m_VertexBuilder.super_VertexDesc_t.m_pNormal;
                    /* Unresolved local var: int i@[???] */
  local_214.m_VertexBuilder.m_pCurrTexCoord[0] =
       local_214.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[0];
  local_214.m_VertexBuilder.m_pCurrTexCoord[1] =
       local_214.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[1];
  local_214.m_VertexBuilder.m_pCurrTexCoord[2] =
       local_214.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[2];
  local_214.m_VertexBuilder.m_pCurrTexCoord[3] =
       local_214.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[3];
  local_214.m_VertexBuilder.m_pCurrTexCoord[4] =
       local_214.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[4];
  local_214.m_VertexBuilder.m_pCurrTexCoord[5] =
       local_214.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[5];
  local_214.m_VertexBuilder.m_pCurrTexCoord[6] =
       local_214.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[6];
  local_214.m_VertexBuilder.m_pCurrTexCoord[7] =
       local_214.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[7];
  local_214.m_VertexBuilder.m_pCurrColor = local_214.m_VertexBuilder.super_VertexDesc_t.m_pColor;
  local_2c.x = vecStart->x;
  local_2c.y = vecStart->y;
  local_2c.z = vecStart->z;
  local_214.m_VertexBuilder.m_nMaxVertexCount = iVar3 << 4;
  DrawColumnSegment(this,&local_214,&local_2c,vDir,flLength,vecXAxis,vecYAxis,flRadius,flAlpha,
                    flTextureOffset);
  if (local_214.m_bGenerateIndices != false) {
    if (local_214.m_Type - MATERIAL_LINE_STRIP < 7) {
                    /* WARNING: Could not recover jumptable at 0x0053cf9f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (*(code *)(*(int *)(unaff_EBX + 0x4d3 + (local_214.m_Type - MATERIAL_LINE_STRIP) * 4) +
                unaff_EBX))();
      return;
    }
    if (local_214.m_IndexBuilder.super_IndexDesc_t.m_nIndexSize != 0) {
      iVar3 = local_214.m_IndexBuilder.m_nMaxIndexCount - local_214.m_IndexBuilder.m_nCurrentIndex;
      if (local_214.m_VertexBuilder.m_nVertexCount <=
          local_214.m_IndexBuilder.m_nMaxIndexCount - local_214.m_IndexBuilder.m_nCurrentIndex) {
        iVar3 = local_214.m_VertexBuilder.m_nVertexCount;
      }
      if (iVar3 != 0) {
        if (local_214.m_Type < MATERIAL_HETEROGENOUS) {
                    /* WARNING: Could not recover jumptable at 0x0053d08e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
          (*(code *)(*(int *)(unaff_EBX + 0x5bf + local_214.m_Type * 4) + unaff_EBX))();
          return;
        }
        GenerateSequentialIndexBuffer
                  (local_214.m_IndexBuilder.super_IndexDesc_t.m_pIndices +
                   local_214.m_IndexBuilder.m_nCurrentIndex,iVar3,
                   local_214.m_IndexBuilder.m_nIndexOffset);
        local_214.m_IndexBuilder.m_nCurrentIndex =
             iVar3 * local_214.m_IndexBuilder.super_IndexDesc_t.m_nIndexSize +
             local_214.m_IndexBuilder.m_nCurrentIndex;
        if (local_214.m_IndexBuilder.m_nIndexCount < local_214.m_IndexBuilder.m_nCurrentIndex) {
          local_214.m_IndexBuilder.m_nIndexCount = local_214.m_IndexBuilder.m_nCurrentIndex;
        }
      }
    }
  }
  (*((local_214.m_pMesh)->super_IVertexBuffer)._vptr_IVertexBuffer[0x14])
            (local_214.m_pMesh,local_214.m_VertexBuilder.m_nVertexCount,
             local_214.m_IndexBuilder.m_nIndexCount,&local_214);
  local_214.m_IndexBuilder.m_pIndexBuffer = (IIndexBuffer *)0x0;
  local_214.m_IndexBuilder.m_nMaxIndexCount = 0;
  local_214.m_VertexBuilder.m_nMaxVertexCount = 0;
  local_214.m_VertexBuilder.m_pVertexBuffer = (IVertexBuffer *)0x0;
  local_214.m_VertexBuilder.super_VertexDesc_t.m_CompressionType = VERTEX_COMPRESSION_INVALID;
  local_214.m_pMesh = (IMesh *)0x0;
  (*(pIVar4->super_IVertexBuffer)._vptr_IVertexBuffer[0xb])(pIVar4,0xffffffff,0);
  if ((local_214.m_VertexBuilder.m_pVertexBuffer != (IVertexBuffer *)0x0) &&
     (iVar3 = (*(local_214.m_VertexBuilder.m_pVertexBuffer)->_vptr_IVertexBuffer[2])
                        (local_214.m_VertexBuilder.m_pVertexBuffer), (char)iVar3 != '\0')) {
    (*(local_214.m_VertexBuilder.m_pVertexBuffer)->_vptr_IVertexBuffer[4])
              (local_214.m_VertexBuilder.m_pVertexBuffer);
  }
  if ((local_214.m_IndexBuilder.m_pIndexBuffer != (IIndexBuffer *)0x0) &&
     (iVar3 = (*(local_214.m_IndexBuilder.m_pIndexBuffer)->_vptr_IIndexBuffer[2])
                        (local_214.m_IndexBuilder.m_pIndexBuffer), (char)iVar3 != '\0')) {
    (*(local_214.m_IndexBuilder.m_pIndexBuffer)->_vptr_IIndexBuffer[4])
              (local_214.m_IndexBuilder.m_pIndexBuffer);
  }
  if (local_20 != (int *)0x0) {
    (**(code **)(*local_20 + 0xc))(local_20);
  }
  if (local_20 != (int *)0x0) {
    (**(code **)(*local_20 + 4))(local_20);
  }
  return;
}


/* C_Trigger_TractorBeam::ShouldDraw at 0053ad50 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: bool ShouldDraw(C_Trigger_TractorBeam * this) */

bool __thiscall C_Trigger_TractorBeam::ShouldDraw(C_Trigger_TractorBeam *this)

{
  return true;
}


/* C_Trigger_TractorBeam::DrawModel at 0053d1f0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: int DrawModel(C_Trigger_TractorBeam * this, int flags,
   RenderableInstance_t * instance) */

int __thiscall
C_Trigger_TractorBeam::DrawModel
          (C_Trigger_TractorBeam *this,int flags,RenderableInstance_t *instance)

{
  float fVar1;
  Vector *position;
  int unaff_EBX;
  float fVar2;
  float fVar3;
  float fVar4;
  float flLength;
  matrix3x4_t local_88;
  Vector local_58;
  Vector local_4c;
  QAngle local_40;
  float local_34;
  float local_30;
  float local_2c;
  Vector local_28 [2];
  
                    /* Unresolved local var: Vector vDir@[???]
                       Unresolved local var: float flStartDelta@[???]
                       Unresolved local var: QAngle angles@[???]
                       Unresolved local var: float flRadius@[???]
                       Unresolved local var: matrix3x4_t xform@[???]
                       Unresolved local var: Vector xAxis@[???]
                       Unresolved local var: Vector yAxis@[???] */
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: Vector res@[???] */
  local_34 = (this->m_vEnd).x - (this->m_vStart).x;
  local_30 = (this->m_vEnd).y - (this->m_vStart).y;
  local_2c = (this->m_vEnd).z - (this->m_vStart).z;
  local_28[0].x = local_34;
  local_28[0].y = local_30;
  local_28[0].z = local_2c;
  flLength = VectorNormalize(local_28);
  fVar3 = *(float *)(**(int **)(unaff_EBX + 0x5f3b4f) + 0xc);
  fVar1 = this->m_flStartTime;
  fVar4 = fVar3 - fVar1;
  if ((fVar4 < *(float *)(unaff_EBX + 0x4796cf)) &&
     (!NAN(fVar4) && !NAN(*(float *)(unaff_EBX + 0x4796cf)))) {
    fVar2 = fVar1 + *(float *)(unaff_EBX + 0x4796cf);
    if ((fVar1 != fVar2) || (NAN(fVar1) || NAN(fVar2))) {
      fVar4 = fVar4 / (fVar2 - fVar1);
      fVar3 = *(float *)(unaff_EBX + 0x479643);
      if (fVar4 <= fVar3) {
        fVar3 = 0.0;
        if (0.0 <= fVar4) {
          fVar3 = fVar4;
        }
        flLength = (fVar3 * fVar3 * *(float *)(unaff_EBX + 0x479a0f) +
                    fVar3 * fVar3 * fVar3 * *(float *)(unaff_EBX + 0x479aaf) + 0.0) * flLength;
        goto LAB_0053d2d1;
      }
    }
    else {
      if (fVar3 < fVar2) {
        flLength = flLength * 0.0;
        goto LAB_0053d2d1;
      }
      fVar3 = *(float *)(unaff_EBX + 0x479643);
    }
    flLength = fVar3 * flLength;
  }
LAB_0053d2d1:
  VectorAngles(local_28,&local_40);
  position = &this->m_vStart;
  AngleMatrix(&local_40,position,&local_88);
  MatrixGetColumn(&local_88,2,&local_4c);
  MatrixGetColumn(&local_88,1,&local_58);
  DrawColumn(this,this->m_pMaterial1,position,local_28,flLength,&local_4c,&local_58,58.0,1.0,
             this->m_bFromPortal,this->m_bToPortal,0.0);
  DrawColumn(this,this->m_pMaterial2,position,local_28,flLength,&local_4c,&local_58,56.0,0.75,
             this->m_bFromPortal,this->m_bToPortal,0.0);
  DrawColumn(this,this->m_pMaterial3,position,local_28,flLength,&local_4c,&local_58,54.0,0.5,
             this->m_bFromPortal,this->m_bToPortal,0.0);
  return 1;
}


/* C_Trigger_TractorBeam::GetToolRecordingState at 0053bb80 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void GetToolRecordingState(C_Trigger_TractorBeam * this, KeyValues *
   msg) */

void __thiscall
C_Trigger_TractorBeam::GetToolRecordingState(C_Trigger_TractorBeam *this,KeyValues *msg)

{
  float fVar1;
  float fVar2;
  KeyValues *this_00;
  char *keyName;
  int unaff_EBX;
  
                    /* Unresolved local var: KeyValues * pKV@[???]
                       Unresolved local var: Vector vStart@[???]
                       Unresolved local var: Vector vEnd@[???] */
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::GetToolRecordingState((C_BaseEntity *)this,msg);
  this_00 = CIFM_EntityKeyValuesHandler_AutoRegister::FindOrCreateNonConformantKeyValues(msg);
  keyName = CIFM_EntityKeyValuesHandler_AutoRegister::GetHandlerIDKeyString();
  KeyValues::SetString(this_00,keyName,(char *)(unaff_EBX + 0x454dac));
  KeyValues::SetInt(this_00,(char *)(unaff_EBX + 0x4510f6),
                    (this->super_C_BaseVPhysicsTrigger).super_C_BaseEntity.index);
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x454be9),this->m_flStartTime);
  KeyValues::SetInt(this_00,(char *)(unaff_EBX + 0x454bf3),(uint)this->m_bReversed);
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x454bfc),this->m_linearForce);
  fVar1 = (this->m_vStart).y;
  fVar2 = (this->m_vStart).z;
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x454c02),(this->m_vStart).x);
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x454592),fVar1);
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x454c08),fVar2);
  fVar1 = (this->m_vEnd).y;
  fVar2 = (this->m_vEnd).z;
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x454c0d),(this->m_vEnd).x);
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x454c12),fVar1);
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x454c17),fVar2);
  return;
}


/* C_Trigger_TractorBeam::RestoreToToolRecordedState at 0053b0c0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void RestoreToToolRecordedState(C_Trigger_TractorBeam * this, KeyValues
   * pKV) */

void __thiscall
C_Trigger_TractorBeam::RestoreToToolRecordedState(C_Trigger_TractorBeam *this,KeyValues *pKV)

{
  undefined4 *puVar1;
  int iVar2;
  IMaterial *pIVar3;
  int unaff_EBX;
  float fVar4;
  Vector local_34;
  Vector local_28 [2];
  
  ___i686_get_pc_thunk_bx();
  fVar4 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x4556a9),0.0);
  this->m_flStartTime = fVar4;
  iVar2 = KeyValues::GetInt(pKV,(char *)(unaff_EBX + 0x4556b3),0);
  this->m_bReversed = iVar2 == 1;
  fVar4 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x4556bc),0.0);
  this->m_linearForce = fVar4;
  fVar4 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x4556c2),0.0);
  (this->m_vStart).x = fVar4;
  fVar4 = KeyValues::GetFloat(pKV,&UNK_00455052 + unaff_EBX,0.0);
  (this->m_vStart).y = fVar4;
  fVar4 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x4556c8),0.0);
  (this->m_vStart).z = fVar4;
  fVar4 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x4556cd),0.0);
  (this->m_vEnd).x = fVar4;
  fVar4 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x4556d2),0.0);
  (this->m_vEnd).y = fVar4;
  fVar4 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x4556d7),0.0);
  (this->m_vEnd).z = fVar4;
  local_34.x = 16384.0;
  local_34.y = 16384.0;
  local_34.z = 16384.0;
  local_28[0].x = -16384.0;
  local_28[0].y = -16384.0;
  local_28[0].z = -16384.0;
  C_BaseEntity::SetSize((C_BaseEntity *)this,local_28,&local_34);
  puVar1 = *(undefined4 **)(unaff_EBX + 0x5f5dfa);
  pIVar3 = (IMaterial *)
           (**(code **)(*(int *)*puVar1 + 300))((int *)*puVar1,unaff_EBX + 0x455657,0,0,0);
  this->m_pMaterial1 = pIVar3;
  pIVar3 = (IMaterial *)
           (**(code **)(*(int *)*puVar1 + 300))((int *)*puVar1,unaff_EBX + 0x45566c,0,0,0);
  this->m_pMaterial2 = pIVar3;
  pIVar3 = (IMaterial *)
           (**(code **)(*(int *)*puVar1 + 300))((int *)*puVar1,unaff_EBX + 0x455682,0,0,0);
  this->m_pMaterial3 = pIVar3;
  return;
}


/* __tcf_2 at 00951ca0 */

void __tcf_2(void *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  *(int *)(unaff_EBX + 0x367fec) = *(int *)(unaff_EBX + 0x1df188) + 8;
  *(undefined4 *)(unaff_EBX + 0x368004) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x368000)) {
    if (*(int *)(unaff_EBX + 0x367ff8) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1df070) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1df070),*(int *)(unaff_EBX + 0x367ff8));
      *(undefined4 *)(unaff_EBX + 0x367ff8) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x367ffc) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x367ff8);
  *(int *)(unaff_EBX + 0x368008) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x368000)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1df070) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1df070),iVar1);
      *(undefined4 *)(unaff_EBX + 0x367ff8) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x367ffc) = 0;
  }
  return;
}


/* C_Trigger_TractorBeam_NonConformantDataHandler::DestroyInstance at 0053df10 */

/* DWARF original prototype: void DestroyInstance(C_Trigger_TractorBeam_NonConformantDataHandler *
   this, void * pEntity) */

void __thiscall
C_Trigger_TractorBeam_NonConformantDataHandler::DestroyInstance
          (C_Trigger_TractorBeam_NonConformantDataHandler *this,void *pEntity)

{
  code *UNRECOVERED_JUMPTABLE;
  int unaff_EBX;
  
                    /* Unresolved local var: C_Trigger_TractorBeam * pCastEntity@[???] */
  ___i686_get_pc_thunk_bx();
  UNRECOVERED_JUMPTABLE = *(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5f2ec7) + 0x44);
  if (pEntity != (void *)0x0) {
    (*UNRECOVERED_JUMPTABLE)((int *)**(undefined4 **)(unaff_EBX + 0x5f2ec7),(int)pEntity + 4);
                    /* WARNING: Could not recover jumptable at 0x0053df52. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (**(code **)(*(int *)pEntity + 4))();
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x0053df73. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}


/* C_Trigger_TractorBeam_NonConformantDataHandler::HandleInstance at 0053e250 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void HandleInstance(C_Trigger_TractorBeam_NonConformantDataHandler *
   this, void * pEntity, KeyValues * pKeyValues) */

void __thiscall
C_Trigger_TractorBeam_NonConformantDataHandler::HandleInstance
          (C_Trigger_TractorBeam_NonConformantDataHandler *this,void *pEntity,KeyValues *pKeyValues)

{
  short *psVar1;
  int unaff_EBX;
  undefined4 *puVar2;
  
                    /* Unresolved local var: C_Trigger_TractorBeam * pCastEntity@[???] */
  ___i686_get_pc_thunk_bx();
  C_Trigger_TractorBeam::RestoreToToolRecordedState(pEntity,pKeyValues);
  psVar1 = (short *)(**(code **)(*(int *)pEntity + 0x35c))(pEntity);
  if (*psVar1 == -1) {
    puVar2 = *(undefined4 **)(unaff_EBX + 0x5f2b84);
    (**(code **)(*(int *)*puVar2 + 0x40))((int *)*puVar2,(int)pEntity + 4,0,1,0xffffffff);
  }
  else {
    puVar2 = *(undefined4 **)(unaff_EBX + 0x5f2b84);
  }
                    /* WARNING: Could not recover jumptable at 0x0053e2a9. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(int *)*puVar2 + 0x4c))();
  return;
}


/* C_Trigger_TractorBeam::OnNewParticleEffect at 0053b050 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void OnNewParticleEffect(C_Trigger_TractorBeam * this, char *
   pszParticleName, CNewParticleEffect * pNewParticleEffect) */

void __thiscall
C_Trigger_TractorBeam::OnNewParticleEffect
          (C_Trigger_TractorBeam *this,char *pszParticleName,CNewParticleEffect *pNewParticleEffect)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  iVar1 = _V_stricmp(pszParticleName,(char *)(unaff_EBX + 0x455702));
  if (iVar1 != 0) {
    return;
  }
  CNewParticleEffect::SetControlPoint(pNewParticleEffect,2,&this->m_vEnd);
  return;
}


/* CCTractorBeamProxyFactory at 0053af30 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

IMaterialProxy * CCTractorBeamProxyFactory(void)

{
  CResultProxy *this;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  this = operator_new(0xc);
  CResultProxy::CResultProxy(this);
  (this->super_IMaterialProxy)._vptr_IMaterialProxy = (_func_int_varargs **)(unaff_EBX + 0x660164);
  return &this->super_IMaterialProxy;
}


/* CTractorBeamProxy::~CTractorBeamProxy at 0053e1f0 */

/* DWARF original prototype: void ~CTractorBeamProxy(CTractorBeamProxy * this, int __in_chrg) */

void __thiscall CTractorBeamProxy::~CTractorBeamProxy(CTractorBeamProxy *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CResultProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(unaff_EBX + 0x65cea7);
  CResultProxy::~CResultProxy(&this->super_CResultProxy,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CTractorBeamProxy::~CTractorBeamProxy at 0053e230 */

/* DWARF original prototype: void ~CTractorBeamProxy(CTractorBeamProxy * this, int __in_chrg) */

void __thiscall CTractorBeamProxy::~CTractorBeamProxy(CTractorBeamProxy *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CResultProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(extraout_ECX + 0x65ce70);
  CResultProxy::~CResultProxy(&this->super_CResultProxy,__in_chrg);
  return;
}


/* CTractorBeamProxy::Init at 0053b020 */

/* DWARF original prototype: bool Init(CTractorBeamProxy * this, IMaterial * pMaterial, KeyValues *
   pKeyValues) */

bool __thiscall
CTractorBeamProxy::Init(CTractorBeamProxy *this,IMaterial *pMaterial,KeyValues *pKeyValues)

{
  bool bVar1;
  
  bVar1 = CResultProxy::Init(&this->super_CResultProxy,pMaterial,pKeyValues);
  return bVar1;
}


/* CTractorBeamProxy::OnBind at 0053af90 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void OnBind(CTractorBeamProxy * this, void * pC_BaseEntity) */

void __thiscall CTractorBeamProxy::OnBind(CTractorBeamProxy *this,void *pC_BaseEntity)

{
  C_BaseEntity *pCVar1;
  int iVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: C_BaseEntity * pEntity@[???]
                       Unresolved local var: C_Trigger_TractorBeam * pBeam@[???] */
  ___i686_get_pc_thunk_bx();
  if (pC_BaseEntity != (void *)0x0) {
    pCVar1 = CResultProxy::BindArgToEntity(&this->super_CResultProxy,pC_BaseEntity);
    if (pCVar1 != (C_BaseEntity *)0x0) {
      iVar2 = ___dynamic_cast(pCVar1,*(undefined4 *)(unaff_EBX + 0x5f5d8b),unaff_EBX + 0x660adf,0);
      if (iVar2 != 0) {
        CResultProxy::SetFloatResult
                  (&this->super_CResultProxy,
                   *(float *)(unaff_EBX + 0x47caeb) * *(float *)(iVar2 + 0xa7c));
        return;
      }
    }
  }
  return;
}


/* _GLOBAL__I__ZN28C_ProjectedTractorBeamEntity9m_PredMapE at 00079be0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN28C_ProjectedTractorBeamEntity9m_PredMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

