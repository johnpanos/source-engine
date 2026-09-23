/* DWARF-guided pseudocode for game/server/portal2/prop_tractorbeam.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* DataMapInit<CPropTractorBeamProjector> at 000ce290 */

datamap_t * DataMapInit<CPropTractorBeamProjector>(CPropTractorBeamProjector *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&PropPaintPowerUser<CPhysicsProp>::m_DataMap.dataNumFields + unaff_EBX + 3) ==
      '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xced727);
    if (iVar1 != 0) {
      *(int *)((int)&PropPaintPowerUser<CPhysicsProp>::m_DataMap.baseMap + unaff_EBX + 3) =
           unaff_EBX + 0x9453a1;
      *(undefined4 *)
       ((int)&PropPaintPowerUser<CPhysicsProp>::m_DataMap.m_pOptimizedDataMap + unaff_EBX + 3) = 0;
      *(undefined4 *)(&DAT_00ced73b + unaff_EBX) = 0;
      *(undefined4 *)(&DAT_00ced73f + unaff_EBX) = 0;
      *(undefined4 *)
       ((int)&PropPaintPowerUser<CPhysicsProp>::DataMapInit::nameHolder.m_pszBase + unaff_EBX + 3) =
           0;
      *(undefined4 *)
       ((int)&PropPaintPowerUser<CPhysicsProp>::DataMapInit::nameHolder.m_nLenBase + unaff_EBX + 3)
           = 0;
      *(undefined4 *)
       ((int)&PropPaintPowerUser<CPhysicsProp>::m_DataMap.m_nPackedSize + unaff_EBX + 3) = 0x19;
      ___cxa_guard_release(unaff_EBX + 0xced727);
      ___cxa_atexit(unaff_EBX + 0x8cf0ef,0,*(undefined4 *)(&DAT_00add8ef + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_NetworkStateChanged_m_iHealth_00c029c8 + unaff_EBX + 3) =
       *(undefined4 *)(&DAT_00ade48b + unaff_EBX);
  *(undefined4 *)((int)&PTR_NetworkStateChanged_m_iMaxHealth_00c029c0 + unaff_EBX + 3) = 2;
  *(int *)((int)&PTR_NetworkStateChanged_m_iMaxHealth_00c029bc + unaff_EBX + 3) =
       unaff_EBX + 0xc02a1f;
  return (datamap_t *)((int)&PTR_NetworkStateChanged_m_iMaxHealth_00c029bc + unaff_EBX + 3);
}


/* __static_initialization_and_destruction_0 at 000ce360 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *s2;
  undefined4 *puVar1;
  datamap_t *pdVar2;
  IEntityFactoryDictionary *pIVar3;
  int iVar4;
  int iVar5;
  int unaff_EBX;
  undefined4 *puVar6;
  undefined4 *puVar7;
  longlong lVar8;
  
  lVar8 = ___i686_get_pc_thunk_bx();
  if (lVar8 == 0xffff00000001) {
    (&DAT_00ced42c)[unaff_EBX] = 0;
    (&DAT_00ced42d)[unaff_EBX] = 0;
    (&DAT_00ced42e)[unaff_EBX] = 0;
    (&DAT_00ced42f)[unaff_EBX] = 0;
    *(undefined4 *)
     ((int)&CUtlRBTree<CHandle<CBaseEntity>,unsigned_short,bool(*)(CHandle<CBaseEntity>const&,CHandle<CBaseEntity>const&),CUtlMemory<UtlRBTreeNode_t<CHandle<CBaseEntity>,unsigned_short>,unsigned_short>>
            ::Links(unsigned_short)::s_Sentinel + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00ced434 + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&CUtlRBTree<CUtlMap<CHandle<CBaseEntity>,CLagCompensationManager::EntityLagData*,unsigned_short>::Node_t,unsigned_short,CUtlMap<CHandle<CBaseEntity>,CLagCompensationManager::EntityLagData*,unsigned_short>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<CHandle<CBaseEntity>,CLagCompensationManager::EntityLagData*,unsigned_short>::Node_t,unsigned_short>,unsigned_short>>
            ::Links(unsigned_short)::s_Sentinel + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00ced43c + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)
     ((int)&CUtlLinkedList<LagRecord,int,true,int,CUtlFixedMemory<UtlLinkedListElem_t<LagRecord,_int>_>_>
            ::AllocInternal::__executeCount_1 + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&CUtlLinkedList<LagRecord,int,true,int,CUtlFixedMemory<UtlLinkedListElem_t<LagRecord,_int>_>_>
            ::AllocInternal::__executeCount + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00ced448 + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00ced44c + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00ced450 + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00ced454 + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00ced458 + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00ced45c + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -0x34) =
         0;
    *(undefined1 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -0x2d) =
         0;
    *(undefined1 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -0x30) =
         0;
    *(undefined1 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -0x2f) =
         0;
    *(undefined1 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -0x2e) =
         0;
    *(undefined4 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -0x2c) =
         0;
    *(undefined1 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -0x25) =
         0;
    *(undefined1 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -0x28) =
         0;
    *(undefined1 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -0x27) =
         0;
    *(undefined1 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -0x26) =
         0;
    *(undefined4 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -0x24) =
         0;
    *(undefined1 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -0x1d) =
         1;
    *(undefined1 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -0x20) =
         0;
    *(undefined1 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -0x1f) =
         0;
    *(undefined1 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -0x1e) =
         0;
    *(undefined4 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -0x1c) =
         1;
    *(undefined1 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -0x15) =
         0;
    *(undefined1 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -0x18) =
         0;
    *(undefined1 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -0x17) =
         0;
    *(undefined1 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -0x16) =
         0;
    *(undefined4 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -0x14) =
         2;
    *(undefined1 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -0xd) =
         0;
    *(undefined1 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -0x10) =
         0;
    *(undefined1 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -0xf) =
         0;
    *(undefined1 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -0xe) =
         0;
    *(undefined **)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -0xc) =
         &UNK_00ae2fb4 + unaff_EBX;
    pdVar2 = DataMapInit<CPropTractorBeamProjector>((CPropTractorBeamProjector *)0x0);
    *(datamap_t **)((int)&PTR_GetSchemeColor_00c621c4 + unaff_EBX) = pdVar2;
    *(undefined **)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -8) =
         &UNK_00b712dc + unaff_EBX;
    pIVar3 = EntityFactoryDictionary();
    (**pIVar3->_vptr_IEntityFactoryDictionary)(pIVar3,unaff_EBX + 0xced48c,unaff_EBX + 0x9452e8);
    iVar5 = unaff_EBX + 0xced490;
    s2 = (char *)(unaff_EBX + 0x9452ce);
    *(char **)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + -4) = s2;
    *(SendTable **)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX) =
         (SendTable *)((int)&PTR_GetSchemeColor_00c621c8 + unaff_EBX);
    *(undefined4 *)((int)DataMapInit<CPlayerResource>::dataDesc[1].flatOffset + unaff_EBX + -0x34) =
         0xffff;
    puVar7 = (undefined4 *)**(int **)(&DAT_00add8bc + unaff_EBX);
    if (puVar7 == (undefined4 *)0x0) {
      **(int **)(&DAT_00add8bc + unaff_EBX) = iVar5;
      *(undefined4 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + 4) = 0
      ;
    }
    else {
      puVar1 = (undefined4 *)puVar7[2];
      iVar4 = _V_stricmp((char *)*puVar7,s2);
      if (iVar4 < 1) {
        while ((puVar6 = puVar1, puVar6 != (undefined4 *)0x0 &&
               (iVar4 = _V_stricmp((char *)*puVar6,s2), iVar4 < 1))) {
          puVar1 = (undefined4 *)puVar6[2];
          puVar7 = puVar6;
        }
        *(undefined4 **)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + 4)
             = puVar6;
        puVar7[2] = iVar5;
      }
      else {
        *(undefined4 *)((int)DataMapInit<CPlayerResource>::dataDesc[0].flatOffset + unaff_EBX + 4) =
             **(undefined4 **)(&DAT_00add8bc + unaff_EBX);
        **(int **)(&DAT_00add8bc + unaff_EBX) = iVar5;
      }
    }
    SendTable::SendTable((SendTable *)((int)&PTR_GetSchemeColor_00c621c8 + unaff_EBX));
    ___cxa_atexit(unaff_EBX + 0x8ceffc,0,*(undefined4 *)(&DAT_00add81c + unaff_EBX));
    iVar5 = ServerClassInit<DT_PropTractorBeamProjector::ignored>((ignored *)0x0);
    *(int *)((int)&PTR_GetDescription_00c621dc + unaff_EBX) = iVar5;
  }
  return;
}


/* CPropTractorBeamProjector::GetDataDescMap at 006f5360 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPropTractorBeamProjector * this) */

datamap_t * __thiscall CPropTractorBeamProjector::GetDataDescMap(CPropTractorBeamProjector *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5db8f8);
}


/* CPropTractorBeamProjector::GetBaseMap at 006f5370 */

datamap_t * CPropTractorBeamProjector::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4b73b4);
}


/* __tcf_0 at 0099d390 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41e646)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41e63a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41e646));
  }
  *(undefined4 *)(unaff_EBX + 0x41e646) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41e642)) {
    if (*(int *)(unaff_EBX + 0x41e63a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20e7fa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20e7fa),*(int *)(unaff_EBX + 0x41e63a));
      *(undefined4 *)(unaff_EBX + 0x41e63a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e63e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41e63a);
  *(int *)(unaff_EBX + 0x41e64a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41e642)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20e7fa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20e7fa),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41e63a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e63e) = 0;
  }
  return;
}


/* ServerClassInit<DT_PropTractorBeamProjector::ignored> at 000ce010 */

int ServerClassInit<DT_PropTractorBeamProjector::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&DAT_00ced7fa)[unaff_EBX] == '\0') {
    iVar1 = ___cxa_guard_acquire(&DAT_00ced7fa + unaff_EBX);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)(unaff_EBX + 0xced802),(char *)(unaff_EBX + 0x8e4a06),0,4,-1,
                  0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)(unaff_EBX + 0xced856),&UNK_008e6d76 + unaff_EBX,0,
                        (SendTable *)**(undefined4 **)(&DAT_00addc02 + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00addc0e + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)(unaff_EBX + 0xced8aa),(char *)(unaff_EBX + 0x945607),0x51c
                    ,4,0,4,0.0,-121121.125,*(SendVarProxyFn_conflict *)(&DAT_00addc66 + unaff_EBX),
                    0x80);
      SendPropVector((SendProp_conflict *)(unaff_EBX + 0xced8fe),(char *)(unaff_EBX + 0x945617),
                     0x520,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00addc5a + unaff_EBX),0x80);
      SendPropBool((SendProp_conflict *)(unaff_EBX + 0xced952),(char *)(unaff_EBX + 0x93f88e),0x514,
                   1);
      ___cxa_guard_release(&DAT_00ced7fa + unaff_EBX);
      ___cxa_atexit(unaff_EBX + 0x8cf2e2,0,*(undefined4 *)(&DAT_00addb72 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)((int)&PTR_SetInfo_00c6251c + unaff_EBX + 2),
             (SendProp_conflict *)(unaff_EBX + 0xced856),4,*(char **)(&DAT_00b71622 + unaff_EBX));
  return 1;
}


/* CPropTractorBeamProjector::GetServerClass at 006f5380 */

/* DWARF original prototype: ServerClass * GetServerClass(CPropTractorBeamProjector * this) */

ServerClass * __thiscall CPropTractorBeamProjector::GetServerClass(CPropTractorBeamProjector *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x6c647c);
}


/* CPropTractorBeamProjector::YouForgotToImplementOrDeclareServerClass at 006f5390 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CPropTractorBeamProjector
   * this) */

int __thiscall
CPropTractorBeamProjector::YouForgotToImplementOrDeclareServerClass(CPropTractorBeamProjector *this)

{
  return 0;
}


/* __tcf_2 at 0099d370 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x3931c4),in_stack_00000008);
  return;
}


/* __tcf_1 at 0099d300 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41e663))(unaff_EBX + 0x41e663);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41e60f))(unaff_EBX + 0x41e60f);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41e5bb))(unaff_EBX + 0x41e5bb);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41e567))(unaff_EBX + 0x41e567);
                    /* WARNING: Could not recover jumptable at 0x0099d362. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41e513))();
  return;
}


/* CPropTractorBeamProjector::Spawn at 006f5400 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CPropTractorBeamProjector * this) */

void __thiscall CPropTractorBeamProjector::Spawn(CPropTractorBeamProjector *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseProjector::Spawn(&this->super_CBaseProjector);
  (*(this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1a])(this);
  (*(this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1b])(this,unaff_EBX + 0x31e1cb);
  CCollisionProperty::SetSolid
            (&(this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.m_Collision.
              super_CCollisionProperty,SOLID_VPHYSICS);
  CBaseAnimating::ResetSequence((CBaseAnimating *)this,2);
  CBaseAnimating::UseClientSideAnimation((CBaseAnimating *)this);
  return;
}


/* CPropTractorBeamProjector::Precache at 006f53c0 */

/* DWARF original prototype: void Precache(CPropTractorBeamProjector * this) */

void __thiscall CPropTractorBeamProjector::Precache(CPropTractorBeamProjector *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x31e210));
  PrecacheParticleSystem((char *)(CAI_FightFromCoverGoal::CAI_FightFromCoverGoal + unaff_EBX + 6));
  PrecacheParticleSystem((char *)(CAI_FightFromCoverBehavior::FValidateHintType + unaff_EBX + 7));
  return;
}


/* CPropTractorBeamProjector::Activate at 006f53b0 */

/* DWARF original prototype: void Activate(CPropTractorBeamProjector * this) */

void __thiscall CPropTractorBeamProjector::Activate(CPropTractorBeamProjector *this)

{
  CBaseProjector::Activate(&this->super_CBaseProjector);
  return;
}


/* CPropTractorBeamProjector::CreateNewProjectedEntity at 006f53a0 */

/* DWARF original prototype: CBaseProjectedEntity *
   CreateNewProjectedEntity(CPropTractorBeamProjector * this) */

CBaseProjectedEntity * __thiscall
CPropTractorBeamProjector::CreateNewProjectedEntity(CPropTractorBeamProjector *this)

{
  CProjectedTractorBeamEntity *pCVar1;
  
  pCVar1 = CProjectedTractorBeamEntity::CreateNewInstance();
  return &pCVar1->super_CBaseProjectedEntity;
}


/* CPropTractorBeamProjector::InputSetLinearForce at 006f5580 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void InputSetLinearForce(CPropTractorBeamProjector * this, inputdata_t
   * indata) */

void __thiscall
CPropTractorBeamProjector::InputSetLinearForce(CPropTractorBeamProjector *this,inputdata_t *indata)

{
  undefined1 *puVar1;
  vec_t *pvVar2;
  float *pfVar3;
  float fVar4;
  CBaseEdict *pCVar5;
  IChangeInfoAccessor *pIVar6;
  int unaff_EBX;
  float fVar7;
  float fVar8;
  
  ___i686_get_pc_thunk_bx();
  fVar8 = 0.0;
  if ((indata->value).fieldType == FIELD_FLOAT) {
    fVar8 = (indata->value).field_0.flVal;
  }
  fVar7 = (this->m_flLinearForce).m_Value;
  if (fVar8 != fVar7) {
    if ((this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent.
        m_bRegistered == false) {
      pCVar5 = &((this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.m_Network.
                m_pPev)->super_CBaseEdict;
      if (pCVar5 != (CBaseEdict *)0x0) {
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
        pIVar6 = CBaseEdict::GetChangeAccessor(pCVar5);
        pIVar6->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.m_Network.
                field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flLinearForce).m_Value = fVar8;
    fVar7 = fVar8;
  }
  if ((fVar7 != 0.0) || (NAN(fVar7))) {
    (*(this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
      super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xe9])(this);
  }
  else {
    CBaseProjector::EnableProjection(&this->super_CBaseProjector,false);
    pfVar3 = *(float **)(unaff_EBX + 0x4b6623);
    fVar8 = *pfVar3;
    fVar7 = pfVar3[1];
    fVar4 = pfVar3[2];
    if ((((fVar8 != (this->m_vEndPos).m_Value.x) || (NAN(fVar8) || NAN((this->m_vEndPos).m_Value.x))
         ) || (pvVar2 = &(this->m_vEndPos).m_Value.y, fVar7 != *pvVar2)) ||
       ((NAN(fVar7) || NAN(*pvVar2) || (fVar4 != (this->m_vEndPos).m_Value.z)))) {
      if ((this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent
          .m_bRegistered == false) {
        pCVar5 = &((this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.m_Network.
                  m_pPev)->super_CBaseEdict;
        if (pCVar5 != (CBaseEdict *)0x0) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
          pIVar6 = CBaseEdict::GetChangeAccessor(pCVar5);
          pIVar6->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.m_Network.
                  field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->m_vEndPos).m_Value.x = fVar8;
      (this->m_vEndPos).m_Value.y = fVar7;
      (this->m_vEndPos).m_Value.z = fVar4;
      return;
    }
  }
  return;
}


/* CPropTractorBeamProjector::Project at 006f5470 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Project(CPropTractorBeamProjector * this) */

void __thiscall CPropTractorBeamProjector::Project(CPropTractorBeamProjector *this)

{
  undefined1 *puVar1;
  vec_t *pvVar2;
  uint uVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  CBaseEdict *this_00;
  int iVar7;
  float *pfVar8;
  IChangeInfoAccessor *pIVar9;
  int *piVar10;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseProjector::Project(&this->super_CBaseProjector);
  uVar3 = (this->super_CBaseProjector).m_hFirstChild.super_CBaseHandle.m_Index;
  if ((uVar3 == 0xffffffff) ||
     (iVar7 = (uVar3 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4b6727),
     *(uint *)(iVar7 + 8) != uVar3 >> 0x10)) {
    piVar10 = (int *)0x0;
  }
  else {
    piVar10 = *(int **)(iVar7 + 4);
  }
  pfVar8 = (float *)(**(code **)(*piVar10 + 0x330))(piVar10);
  fVar4 = *pfVar8;
  fVar5 = pfVar8[1];
  fVar6 = pfVar8[2];
  if ((((fVar4 != (this->m_vEndPos).m_Value.x) || (NAN(fVar4) || NAN((this->m_vEndPos).m_Value.x)))
      || (pvVar2 = &(this->m_vEndPos).m_Value.y, fVar5 != *pvVar2)) ||
     ((NAN(fVar5) || NAN(*pvVar2) || (fVar6 != (this->m_vEndPos).m_Value.z)))) {
    if ((this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent.
        m_bRegistered == false) {
      this_00 = &((this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.m_Network.
                 m_pPev)->super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar9 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar9->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.m_Network.
                field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_vEndPos).m_Value.x = fVar4;
    (this->m_vEndPos).m_Value.y = fVar5;
    (this->m_vEndPos).m_Value.z = fVar6;
  }
  return;
}


/* _GLOBAL__I__ZN25CPropTractorBeamProjector9m_DataMapE at 000ce5e0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN25CPropTractorBeamProjector9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

