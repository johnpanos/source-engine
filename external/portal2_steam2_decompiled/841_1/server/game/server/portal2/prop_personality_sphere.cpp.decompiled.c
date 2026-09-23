/* DWARF-guided pseudocode for game/server/portal2/prop_personality_sphere.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* CInfoTargetPersonalitySphere::~CInfoTargetPersonalitySphere at 00766b70 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CInfoTargetPersonalitySphere(CInfoTargetPersonalitySphere * this,
   int __in_chrg) */

void __thiscall
CInfoTargetPersonalitySphere::~CInfoTargetPersonalitySphere
          (CInfoTargetPersonalitySphere *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x56fcb0);
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(extraout_ECX + 0x4e0118) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,__in_chrg);
  return;
}


/* CEntityFactory<CInfoTargetPersonalitySphere>::Create at 00766d40 */

/* DWARF original prototype: IServerNetworkable *
   Create(CEntityFactory<CInfoTargetPersonalitySphere> * this, char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CInfoTargetPersonalitySphere>::Create
          (CEntityFactory<CInfoTargetPersonalitySphere> *this,char *pClassName)

{
  CBaseEntity *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CInfoTargetPersonalitySphere * pEnt@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CBaseEntity::operator_new(0x3c8);
  CBaseEntity::CBaseEntity(this_00,false);
  (this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x56fad4);
  CBaseEntity::PostConstructor(this_00,pClassName);
  return &(this_00->m_Network).super_IServerNetworkable;
}


/* CInfoTargetPersonalitySphere::~CInfoTargetPersonalitySphere at 00766f80 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CInfoTargetPersonalitySphere(CInfoTargetPersonalitySphere * this,
   int __in_chrg) */

void __thiscall
CInfoTargetPersonalitySphere::~CInfoTargetPersonalitySphere
          (CInfoTargetPersonalitySphere *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x56f897);
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4dfcff) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CInfoTargetPersonalitySphere::Spawn at 00766b50 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CInfoTargetPersonalitySphere * this) */

void __thiscall CInfoTargetPersonalitySphere::Spawn(CInfoTargetPersonalitySphere *this)

{
  (*(this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1a])(this);
  CPointEntity::Spawn(&this->super_CPointEntity);
  return;
}


/* CInfoTargetPersonalitySphere::Precache at 00766b20 */

/* DWARF original prototype: void Precache(CInfoTargetPersonalitySphere * this) */

void __thiscall CInfoTargetPersonalitySphere::Precache(CInfoTargetPersonalitySphere *this)

{
  char *soundname;
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  soundname = (this->m_strSphereLine).pszValue;
  if (soundname == (char *)0x0) {
    soundname = (char *)(extraout_ECX + 0x2e81fc);
  }
  CBaseEntity::PrecacheScriptSound(soundname);
  return;
}


/* DataMapInit<CInfoTargetPersonalitySphere> at 000b7c40 */

/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "TreeNodeText": ignoring multiple overlapping fields */

datamap_t * DataMapInit<CInfoTargetPersonalitySphere>(CInfoTargetPersonalitySphere *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&flex_minplayertime.super_ConCommandBase.m_pszName + unaff_EBX + 3) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xd98a8f);
    if (iVar1 != 0) {
      *(int *)((int)&flex_minplayertime.super_ConCommandBase.m_nFlags + unaff_EBX + 3) =
           unaff_EBX + 0x9fd816;
      *(undefined4 *)((int)&flex_minplayertime.m_pParent + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&flex_minplayertime.m_pszDefaultValue + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&flex_minplayertime.m_Value.m_pszString + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&flex_minplayertime.m_Value.m_StringLength + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&flex_minplayertime.m_Value.m_fValue + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&flex_minplayertime.super_IConVar._vptr_IConVar + unaff_EBX + 3) = 0x1c;
      ___cxa_guard_release(unaff_EBX + 0xd98a8f);
      ___cxa_atexit(unaff_EBX + 0x988a7f,0,*(undefined4 *)(&DAT_00b8ef03 + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_RunVScripts_00cb26b4 + unaff_EBX + 3) =
       *(undefined4 *)(&DAT_00b8f03b + unaff_EBX);
  *(undefined4 *)((int)&PTR_NetworkStateChanged_m_vecViewOffset_00cb26ac + unaff_EBX + 3) = 2;
  *(int *)((int)&PTR_NetworkStateChanged_m_bClientSideRagdoll_00cb26a8 + unaff_EBX + 3) =
       unaff_EBX + 0xcb2d2f;
  return (datamap_t *)((int)&PTR_NetworkStateChanged_m_bClientSideRagdoll_00cb26a8 + unaff_EBX + 3);
}


/* __static_initialization_and_destruction_0 at 000b8250 */

/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  IEntityFactoryDictionary *pIVar1;
  datamap_t *pdVar2;
  int unaff_EBX;
  longlong lVar3;
  
  lVar3 = ___i686_get_pc_thunk_bx();
  if (lVar3 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)
   ((int)g_SentGameRulesMasks[0x1a].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>
         .m_Ints + unaff_EBX + 4) = 0;
  *(undefined1 *)
   ((int)g_SentGameRulesMasks[0x1a].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>
         .m_Ints + unaff_EBX + 5) = 0;
  *(undefined1 *)
   ((int)g_SentGameRulesMasks[0x1a].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>
         .m_Ints + unaff_EBX + 6) = 0;
  *(undefined1 *)
   ((int)g_SentGameRulesMasks[0x1a].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>
         .m_Ints + unaff_EBX + 7) = 0;
  *(undefined4 *)
   ((int)g_SentGameRulesMasks[0x1b].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>
         .m_Ints + unaff_EBX) = 0;
  *(undefined4 *)
   ((int)g_SentGameRulesMasks[0x1b].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>
         .m_Ints + unaff_EBX + 4) = 0;
  *(undefined4 *)
   ((int)g_SentGameRulesMasks[0x1c].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>
         .m_Ints + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)
   ((int)g_SentGameRulesMasks[0x1c].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>
         .m_Ints + unaff_EBX + 4) = 0x7f7fffff;
  *(undefined4 *)
   ((int)g_SentGameRulesMasks[0x1d].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>
         .m_Ints + unaff_EBX) = 0;
  *(undefined4 *)
   ((int)g_SentGameRulesMasks[0x1d].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>
         .m_Ints + unaff_EBX + 4) = 0;
  *(undefined4 *)
   ((int)g_SentGameRulesMasks[0x1e].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>
         .m_Ints + unaff_EBX) = 0;
  *(undefined4 *)
   ((int)g_SentGameRulesMasks[0x1e].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>
         .m_Ints + unaff_EBX + 4) = 0;
  *(undefined4 *)
   ((int)g_SentGameRulesMasks[0x1f].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>
         .m_Ints + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)
   ((int)g_SentGameRulesMasks[0x1f].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>
         .m_Ints + unaff_EBX + 4) = 0x7f7fffff;
  *(undefined4 *)
   ((int)g_SentGameRulesMasks[0x20].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>
         .m_Ints + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)
   ((int)g_SentGameRulesMasks[0x20].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>
         .m_Ints + unaff_EBX + 4) = 0x7f7fffff;
  *(undefined **)(&DAT_00d98448 + unaff_EBX) = &UNK_00b940ac + unaff_EBX;
  *(undefined **)(&DAT_00d9844c + unaff_EBX) = &UNK_00c1e91c + unaff_EBX;
  pIVar1 = EntityFactoryDictionary();
  (**pIVar1->_vptr_IEntityFactoryDictionary)(pIVar1,&DAT_00d9844c + unaff_EBX,unaff_EBX + 0x9fd1ec);
  pdVar2 = DataMapInit<CInfoTargetPersonalitySphere>((CInfoTargetPersonalitySphere *)0x0);
  *(datamap_t **)((int)&ScriptedNPCInteraction_t::m_DataMap.dataNumFields + unaff_EBX) = pdVar2;
  *(undefined **)(&DAT_00d98450 + unaff_EBX) = &UNK_00c1e93c + unaff_EBX;
  pIVar1 = EntityFactoryDictionary();
  (**pIVar1->_vptr_IEntityFactoryDictionary)
            (pIVar1,&DAT_00d98450 + unaff_EBX,&UNK_009fd23f + unaff_EBX);
  pdVar2 = DataMapInit<CPropPersonalitySphere>((CPropPersonalitySphere *)0x0);
  *(datamap_t **)((int)&ScriptedNPCInteraction_t::m_DataMap.dataClassName + unaff_EBX) = pdVar2;
  return;
}


/* CInfoTargetPersonalitySphere::GetDataDescMap at 00763210 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CInfoTargetPersonalitySphere * this) */

datamap_t * __thiscall
CInfoTargetPersonalitySphere::GetDataDescMap(CInfoTargetPersonalitySphere *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x6070e4);
}


/* CInfoTargetPersonalitySphere::GetBaseMap at 00763220 */

datamap_t * CInfoTargetPersonalitySphere::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4e3a64);
}


/* __tcf_0 at 00a406d0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41001e)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x410012) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41001e));
  }
  *(undefined4 *)(unaff_EBX + 0x41001e) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41001a)) {
    if (*(int *)(unaff_EBX + 0x410012) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20647e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20647e),*(int *)(unaff_EBX + 0x410012));
      *(undefined4 *)(unaff_EBX + 0x410012) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x410016) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x410012);
  *(int *)(unaff_EBX + 0x410022) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41001a)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20647e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20647e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x410012) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x410016) = 0;
  }
  return;
}


/* CInfoTargetEntityEnumerator::EnumEntity at 00766db0 */

/* DWARF original prototype: bool EnumEntity(CInfoTargetEntityEnumerator * this, IHandleEntity *
   pHandleEntity) */

bool __thiscall
CInfoTargetEntityEnumerator::EnumEntity
          (CInfoTargetEntityEnumerator *this,IHandleEntity *pHandleEntity)

{
  uint uVar1;
  int *piVar2;
  Ray_t *pRVar3;
  bool bVar4;
  uint *puVar5;
  CBaseEntity *this_00;
  int iVar6;
  int unaff_EBX;
  longdouble lVar7;
  longdouble lVar8;
  float fVar9;
  float fVar10;
  Vector local_44;
  float local_38;
  float local_34;
  float local_30;
  Vector local_2c;
  char *local_20;
  
                    /* Unresolved local var: CBaseEntity * pEnt@[???] */
  ___i686_get_pc_thunk_bx();
  puVar5 = (uint *)(*pHandleEntity->_vptr_IHandleEntity[3])(pHandleEntity);
  uVar1 = *puVar5;
  if ((((uVar1 != 0xffffffff) &&
       (iVar6 = (uVar1 & 0xffff) * 0x10 + *(int *)(unaff_EBX + 0x4dfe66),
       *(uint *)(iVar6 + 8) == uVar1 >> 0x10)) &&
      (piVar2 = *(int **)(iVar6 + 4), piVar2 != (int *)0x0)) &&
     (this_00 = (CBaseEntity *)(**(code **)(*piVar2 + 0x18))(piVar2), this_00 != (CBaseEntity *)0x0)
     ) {
    local_20 = (char *)(unaff_EBX + 0x34e68a);
    if (((this_00->m_iClassname).pszValue != local_20) &&
       (bVar4 = CBaseEntity::ClassMatchesComplex(this_00,local_20), !bVar4)) {
      return true;
    }
    pRVar3 = this->m_ray;
    if ((this_00->m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition(this_00);
    }
                    /* Unresolved local var: Vector res@[???] */
    local_38 = (this_00->m_vecAbsOrigin).x - (pRVar3->m_Start).super_Vector.x;
    local_34 = (this_00->m_vecAbsOrigin).y - (pRVar3->m_Start).super_Vector.y;
    local_30 = (this_00->m_vecAbsOrigin).z - (pRVar3->m_Start).super_Vector.z;
    local_2c.x = local_38;
    local_2c.y = local_34;
    local_2c.z = local_30;
    fVar10 = VectorNormalize(&local_2c);
    pRVar3 = this->m_ray;
    local_44.x = (pRVar3->m_Delta).super_Vector.x;
    local_44.y = (pRVar3->m_Delta).super_Vector.y;
    local_44.z = (pRVar3->m_Delta).super_Vector.z;
    VectorNormalize(&local_44);
    fVar9 = local_2c.x * local_44.x + local_2c.y * local_44.y + local_2c.z * local_44.z;
    lVar7 = (longdouble)_atan2((double)(float)this_00[1].m_iObjectCapsCache.m_Value,(double)fVar10);
    if ((this->m_flLargestDot <= fVar9 && fVar9 != this->m_flLargestDot) &&
       (lVar8 = (longdouble)_acosf(fVar9), (float)lVar8 < (float)lVar7)) {
      this->m_flClosestDist = fVar10;
      this->m_flLargestDot = fVar9;
      this->m_pBestEnt = this_00;
      return true;
    }
  }
  return true;
}


/* CPropPersonalitySphere::CheckForInfoTargets at 00764740 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void CheckForInfoTargets(CPropPersonalitySphere * this) */

void __thiscall CPropPersonalitySphere::CheckForInfoTargets(CPropPersonalitySphere *this)

{
  char *pszClassOrWildcard;
  int *piVar1;
  CBaseEntity *pCVar2;
  bool bVar3;
  CBasePlayer *this_00;
  int iVar4;
  uint *puVar5;
  int iVar6;
  CBaseEntity *pCVar7;
  int unaff_EBX;
  int iVar8;
  longdouble lVar9;
  longdouble lVar10;
  uint uVar11;
  float fVar12;
  float fVar13;
  float local_564;
  CBaseEntity *local_560;
  CBaseEntity *local_550 [256];
  Vector local_150;
  Vector local_144 [2];
  float local_124;
  float local_fc;
  float local_f8;
  float local_f4;
  _func_int_varargs **local_ec;
  CBaseEntity **local_e8;
  float local_e4;
  undefined4 local_dc;
  undefined4 local_d8;
  undefined4 local_d4;
  undefined4 local_cc;
  undefined4 local_c8;
  undefined4 local_c4;
  undefined4 local_bc;
  undefined1 local_b8;
  undefined1 local_b7;
  CFlaggedEntitiesEnum local_a0;
  float local_8c;
  float local_88;
  float local_84;
  Vector local_80;
  float local_74;
  float local_70;
  float local_6c;
  float local_68;
  float local_64;
  float local_60;
  float local_5c;
  float local_58;
  float local_54;
  Vector local_50;
  Vector local_44;
  Vector local_38;
  float local_2c;
  float local_28;
  float local_24;
  char *local_20;
  
                    /* Unresolved local var: CPortal_Player * pPlayer@[???]
                       Unresolved local var: Vector vecStart@[???]
                       Unresolved local var: Vector vecForward@[???]
                       Unresolved local var: Vector vecRight@[???]
                       Unresolved local var: Vector vecUp@[???] */
  ___i686_get_pc_thunk_bx();
  if (((*(int *)(**(int **)(unaff_EBX + 0x4e240d) + 0x14) < 2) &&
      (this_00 = UTIL_GetLocalPlayer(), this_00 != (CBasePlayer *)0x0)) && (this->m_bHeld != false))
  {
    (**(code **)((int)(this_00->super_CBaseCombatCharacter).super_CBaseFlex.
                      super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                      super_IServerEntity.super_IServerUnknown + 0x220))();
    CBasePlayer::EyeVectors(this_00,&local_38,&local_44,&local_50);
                    /* Unresolved local var: Ray_t ray@[???]
                       Unresolved local var: CInfoTargetEntityEnumerator InfoTargetEnum@[???]
                       Unresolved local var: trace_t tr@[???]
                       Unresolved local var: CBaseEntity *[256] ppEnts@[???]
                       Unresolved local var: int nEntCount@[???]
                       Unresolved local var: int i@[???]
                       Unresolved local var: CInfoTargetPersonalitySphere * pEnt@[???]
                       Unresolved local var: Vector res@[???] */
    local_6c = *(float *)(unaff_EBX + 0x368859);
    local_74 = local_38.x * local_6c;
    local_70 = local_38.y * local_6c;
    local_6c = local_6c * local_38.z;
                    /* Unresolved local var: Vector res@[???] */
    local_68 = local_2c + local_74;
    local_64 = local_28 + local_70;
    local_60 = local_24 + local_6c;
    local_ec = (_func_int_varargs **)(local_68 - local_2c);
    local_e8 = (CBaseEntity **)(local_64 - local_28);
    local_e4 = local_60 - local_24;
    local_b7 = (float)local_ec * (float)local_ec + (float)local_e8 * (float)local_e8 +
               local_e4 * local_e4 != 0.0;
    local_c4 = 0;
    local_c8 = 0;
    local_cc = 0;
    local_bc = 0;
    local_b8 = 1;
    local_d4 = 0;
    local_d8 = 0;
    local_dc = 0;
    local_fc = local_2c;
    local_f8 = local_28;
    local_f4 = local_24;
    local_5c = local_74;
    local_58 = local_70;
    local_54 = local_6c;
    CTraceFilterSimple::CTraceFilterSimple
              ((CTraceFilterSimple *)&local_a0,(IHandleEntity *)0x0,0,(ShouldHitFunc_t)0x0);
    (**(code **)(*(int *)**(undefined4 **)(&DAT_004e2479 + unaff_EBX) + 0x14))
              ((int *)**(undefined4 **)(&DAT_004e2479 + unaff_EBX),&local_fc,0x400b,&local_a0,
               &local_150);
    if (*(int *)(*(int *)(*(int *)(&DAT_004e2475 + unaff_EBX) + 0x1c) + 0x30) != 0) {
      DebugDrawLine(&local_150,local_144,0xff,0,0,true,-1.0);
    }
    fVar12 = *(float *)(unaff_EBX + 0x36882d);
    fVar13 = *(float *)(unaff_EBX + 0x3684c1);
                    /* Unresolved local var: CFlaggedEntitiesEnum sphereEnum@[???] */
    CFlaggedEntitiesEnum::CFlaggedEntitiesEnum(&local_a0,local_550,0x100,0);
    iVar4 = UTIL_EntitiesInSphere(local_144,fVar12 * local_124 + fVar13,&local_a0);
    if (0 < iVar4) {
      iVar8 = 0;
      local_564 = *(float *)(unaff_EBX + 0x368189);
      local_560 = (CBaseEntity *)0x0;
      pszClassOrWildcard = (char *)(unaff_EBX + 0x350cf1);
      do {
        pCVar7 = local_550[iVar8];
        pCVar2 = local_560;
        if (pCVar7 != (CBaseEntity *)0x0) {
          puVar5 = (uint *)(*(pCVar7->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                             _vptr_IHandleEntity[3])(pCVar7);
          uVar11 = *puVar5;
                    /* Unresolved local var: IServerUnknown * pUnk@[???]
                       Unresolved local var: CEntInfo * pInfo@[???] */
          if ((((uVar11 != 0xffffffff) &&
               (iVar6 = (uVar11 & 0xffff) * 0x10 + *(int *)(unaff_EBX + 0x4e24cd),
               *(uint *)(iVar6 + 8) == uVar11 >> 0x10)) &&
              ((piVar1 = *(int **)(iVar6 + 4), piVar1 != (int *)0x0 &&
               (pCVar7 = (CBaseEntity *)(**(code **)(*piVar1 + 0x18))(piVar1),
               pCVar7 != (CBaseEntity *)0x0)))) &&
             ((local_20 = pszClassOrWildcard, (pCVar7->m_iClassname).pszValue == pszClassOrWildcard
              || (bVar3 = CBaseEntity::ClassMatchesComplex(pCVar7,pszClassOrWildcard), bVar3)))) {
            if ((pCVar7->m_iEFlags & 0x800) != 0) {
              CBaseEntity::CalcAbsolutePosition(pCVar7);
            }
                    /* Unresolved local var: Vector res@[???] */
            local_8c = (pCVar7->m_vecAbsOrigin).x - local_fc;
            local_88 = (pCVar7->m_vecAbsOrigin).y - local_f8;
            local_84 = (pCVar7->m_vecAbsOrigin).z - local_f4;
            local_80.x = local_8c;
            local_80.y = local_88;
            local_80.z = local_84;
            fVar13 = VectorNormalize(&local_80);
            local_a0.super_IPartitionEnumerator._vptr_IPartitionEnumerator = local_ec;
            local_a0.m_pList = local_e8;
            local_a0.m_listMax = (int)local_e4;
            VectorNormalize((Vector *)&local_a0);
            fVar12 = local_80.x *
                     (float)local_a0.super_IPartitionEnumerator._vptr_IPartitionEnumerator +
                     local_80.y * (float)local_a0.m_pList + local_80.z * (float)local_a0.m_listMax;
            lVar9 = (longdouble)
                    _atan2((double)(float)pCVar7[1].m_iObjectCapsCache.m_Value,(double)fVar13);
            if (local_564 < fVar12) {
              lVar10 = (longdouble)_acosf(fVar12);
              uVar11 = -(uint)((float)lVar9 <= (float)lVar10);
              local_564 = (float)(~uVar11 & (uint)fVar12 | (uint)local_564 & uVar11);
              pCVar2 = pCVar7;
              if ((float)lVar9 <= (float)lVar10) {
                pCVar2 = local_560;
              }
            }
          }
        }
        local_560 = pCVar2;
        iVar8 = iVar8 + 1;
      } while (iVar8 != iVar4);
      if (local_560 != (CBaseEntity *)0x0) {
        (this->m_iszInfoTargetScriptName).pszValue =
             (char *)local_560[1].super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
                     _vptr_IHandleEntity;
        TrySpeakLine(this,0,8);
        UTIL_Remove(local_560);
      }
    }
  }
  return;
}


/* CPropPersonalitySphere::PreferredCarryAngles at 00766a50 */

/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: QAngle PreferredCarryAngles(CPropPersonalitySphere * this) */

QAngle * __thiscall
CPropPersonalitySphere::PreferredCarryAngles
          (QAngle *__return_storage_ptr__,CPropPersonalitySphere *this)

{
  __return_storage_ptr__->x = 180.0;
  __return_storage_ptr__->y = -90.0;
  __return_storage_ptr__->z = 180.0;
  return __return_storage_ptr__;
}


/* CPropPersonalitySphere::HasPreferredCarryAnglesForPlayer at 00766a90 */

/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: bool HasPreferredCarryAnglesForPlayer(CPropPersonalitySphere * this,
   CBasePlayer * pPlayer) */

bool __thiscall
CPropPersonalitySphere::HasPreferredCarryAnglesForPlayer
          (CPropPersonalitySphere *this,CBasePlayer *pPlayer)

{
  return true;
}


/* CPropPersonalitySphere::ObjectCaps at 00766ba0 */

/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: int ObjectCaps(CPropPersonalitySphere * this) */

int __thiscall CPropPersonalitySphere::ObjectCaps(CPropPersonalitySphere *this)

{
  uint uVar1;
  
  uVar1 = CPhysicsProp::ObjectCaps(&this->super_CPhysicsProp);
  return uVar1 | 1;
}


/* DataMapInit<CPropPersonalitySphere> at 000b7d10 */

/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */

datamap_t * DataMapInit<CPropPersonalitySphere>(CPropPersonalitySphere *param_1)

{
  int iVar1;
  int iVar2;
  char *pcVar3;
  size_t sVar4;
  int iVar5;
  undefined4 *puVar6;
  int iVar7;
  undefined4 uVar8;
  int unaff_EBX;
  int local_24;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  if ((*(char *)((int)&g_AIAgentTaskTimings[1].runTimer.m_Duration.m_Int64 + unaff_EBX + 2) == '\0')
     && (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd98992), iVar2 != 0)) {
    *(undefined **)((int)&g_AIAgentTaskTimings[2].selectSchedule.m_Duration.m_Int64 + unaff_EBX + 6)
         = &UNK_009fd766 + unaff_EBX;
    *(undefined4 *)((int)&g_AIAgentTaskTimings[2].startTimer.m_Duration.m_Int64 + unaff_EBX + 6) = 0
    ;
    *(undefined4 *)((int)&g_AIAgentTaskTimings[2].runTimer.m_Duration.m_Int64 + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&g_AIAgentTaskTimings[2].runTimer.m_Duration.m_Int64 + unaff_EBX + 6) = 0;
    *(undefined4 *)((int)&g_AIAgentTaskTimings[3].pszTask + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&g_AIAgentTaskTimings[3].selectSchedule.m_Duration.m_Int64 + unaff_EBX + 2)
         = 0;
    *(undefined4 *)((int)&g_AIAgentTaskTimings[2].startTimer.m_Duration.m_Int64 + unaff_EBX + 2) =
         0x16;
    ___cxa_guard_release(unaff_EBX + 0xd98992);
    ___cxa_atexit(unaff_EBX + 0x9888d2,0,*(undefined4 *)(&DAT_00b8ee36 + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_CanBeSeenBy_00cb2600 + unaff_EBX + 2) =
       *(undefined4 *)(&DAT_00b8f846 + unaff_EBX);
  if ((*(char *)((int)&g_AIAgentTaskTimings[2].pszTask + unaff_EBX + 2) != '\0') ||
     (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd9899a), iVar2 == 0)) goto LAB_000b7d61;
  if ((*(char *)((int)&PTR_ModifyOrAppendDerivedCriteria_00ccfd38 + unaff_EBX + 2) == '\0') &&
     (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xccfd3a), iVar2 != 0)) {
    *(undefined **)((int)&PTR_NetworkStateChanged_m_iHealth_00ccfd44 + unaff_EBX + 2) =
         &UNK_00c1eee2 + unaff_EBX;
    ___cxa_guard_release(unaff_EBX + 0xccfd3a);
  }
  *(int *)((int)&PTR_NetworkStateChanged_m_flFriction_00cb29b4 + unaff_EBX + 2) =
       unaff_EBX + 0xccfd46;
  pcVar3 = operator_new___(*(int *)((int)&g_AIAgentTaskTimings[2].startTimer.m_Duration.m_Int64 +
                                   unaff_EBX + 2) + 0xd);
  _strcpy(pcVar3,*(char **)((int)&g_AIAgentTaskTimings[2].selectSchedule.m_Duration.m_Int64 +
                           unaff_EBX + 6));
  sVar4 = _strlen(pcVar3);
  builtin_strncpy(pcVar3 + sVar4,"TalkingThink",0xd);
  iVar1 = *(int *)((int)&g_AIAgentTaskTimings[3].pszTask + unaff_EBX + 2);
  iVar2 = iVar1 + 1;
  iVar7 = *(int *)((int)&g_AIAgentTaskTimings[2].runTimer.m_Duration.m_Int64 + unaff_EBX + 2);
  if (iVar7 < iVar2) {
    iVar5 = *(int *)((int)&g_AIAgentTaskTimings[2].runTimer.m_Duration.m_Int64 + unaff_EBX + 6);
    if (-1 < iVar5) {
      if (iVar5 == 0) {
        if ((iVar7 == 0) && (iVar7 = 8, iVar2 < 9)) {
          local_20 = 0x20;
        }
        else {
          do {
            local_20 = iVar7;
            iVar7 = local_20 * 2;
          } while (iVar7 < iVar2);
          local_20 = local_20 * 8;
        }
        goto LAB_000b8141;
      }
      iVar7 = (iVar1 / iVar5 + 1) * iVar5;
      if (iVar7 < iVar2) {
        if (iVar7 != 0) {
LAB_000b8060:
          do {
            iVar7 = (iVar7 + iVar2) / 2;
          } while (iVar7 < iVar2);
          goto LAB_000b8073;
        }
        if (iVar2 < 0) {
          iVar7 = -1;
          local_20 = -4;
        }
        else {
          local_20 = 0;
          if (iVar2 != 0) goto LAB_000b8060;
        }
      }
      else {
LAB_000b8073:
        local_20 = iVar7 * 4;
      }
LAB_000b8141:
      *(int *)((int)&g_AIAgentTaskTimings[2].runTimer.m_Duration.m_Int64 + unaff_EBX + 2) = iVar7;
      iVar7 = *(int *)((int)&g_AIAgentTaskTimings[2].startTimer.m_Duration.m_Int64 + unaff_EBX + 6);
      if (iVar7 == 0) {
        uVar8 = (*(code *)**(undefined4 **)**(undefined4 **)(&DAT_00b8ee3e + unaff_EBX))
                          ((undefined4 *)**(undefined4 **)(&DAT_00b8ee3e + unaff_EBX),local_20);
      }
      else {
        uVar8 = (**(code **)(*(int *)**(undefined4 **)(&DAT_00b8ee3e + unaff_EBX) + 4))
                          ((int *)**(undefined4 **)(&DAT_00b8ee3e + unaff_EBX),iVar7,local_20);
      }
      *(undefined4 *)((int)&g_AIAgentTaskTimings[2].startTimer.m_Duration.m_Int64 + unaff_EBX + 6) =
           uVar8;
    }
  }
  iVar5 = *(int *)((int)&g_AIAgentTaskTimings[3].pszTask + unaff_EBX + 2) + 1;
  *(int *)((int)&g_AIAgentTaskTimings[3].pszTask + unaff_EBX + 2) = iVar5;
  iVar7 = *(int *)((int)&g_AIAgentTaskTimings[2].startTimer.m_Duration.m_Int64 + unaff_EBX + 6);
  *(int *)((int)&g_AIAgentTaskTimings[3].selectSchedule.m_Duration.m_Int64 + unaff_EBX + 2) = iVar7;
  iVar5 = (iVar5 - iVar1) + -1;
  if (0 < iVar5) {
    _V_memmove((void *)(iVar7 + iVar2 * 4),(void *)(iVar7 + iVar1 * 4),iVar5 * 4);
    iVar7 = *(int *)((int)&g_AIAgentTaskTimings[2].startTimer.m_Duration.m_Int64 + unaff_EBX + 6);
  }
  puVar6 = (undefined4 *)(iVar1 * 4 + iVar7);
  if (puVar6 != (undefined4 *)0x0) {
    *puVar6 = pcVar3;
  }
  *(char **)((int)&PTR_OnParseMapDataFinished_00cb2ba4 + unaff_EBX + 2) = pcVar3;
  pcVar3 = operator_new___(*(int *)((int)&g_AIAgentTaskTimings[2].startTimer.m_Duration.m_Int64 +
                                   unaff_EBX + 2) + 0xd);
  _strcpy(pcVar3,*(char **)((int)&g_AIAgentTaskTimings[2].selectSchedule.m_Duration.m_Int64 +
                           unaff_EBX + 6));
  sVar4 = _strlen(pcVar3);
  builtin_strncpy(pcVar3 + sVar4,"AnimateThink",0xd);
  iVar1 = *(int *)((int)&g_AIAgentTaskTimings[3].pszTask + unaff_EBX + 2);
  iVar2 = iVar1 + 1;
  iVar7 = *(int *)((int)&g_AIAgentTaskTimings[2].runTimer.m_Duration.m_Int64 + unaff_EBX + 2);
  if (iVar7 < iVar2) {
    iVar5 = *(int *)((int)&g_AIAgentTaskTimings[2].runTimer.m_Duration.m_Int64 + unaff_EBX + 6);
    if (-1 < iVar5) {
      if (iVar5 == 0) {
        if ((iVar7 == 0) && (iVar7 = 8, iVar2 < 9)) {
          local_24 = 0x20;
        }
        else {
          do {
            local_24 = iVar7;
            iVar7 = local_24 * 2;
          } while (iVar7 < iVar2);
          local_24 = local_24 * 8;
        }
        goto LAB_000b81a1;
      }
      iVar7 = (iVar1 / iVar5 + 1) * iVar5;
      if (iVar7 < iVar2) {
        if (iVar7 != 0) {
LAB_000b7fe0:
          do {
            iVar7 = (iVar7 + iVar2) / 2;
          } while (iVar7 < iVar2);
          goto LAB_000b7ff3;
        }
        if (iVar2 < 0) {
          iVar7 = -1;
          local_24 = -4;
        }
        else {
          local_24 = 0;
          if (iVar2 != 0) goto LAB_000b7fe0;
        }
      }
      else {
LAB_000b7ff3:
        local_24 = iVar7 * 4;
      }
LAB_000b81a1:
      *(int *)((int)&g_AIAgentTaskTimings[2].runTimer.m_Duration.m_Int64 + unaff_EBX + 2) = iVar7;
      iVar7 = *(int *)((int)&g_AIAgentTaskTimings[2].startTimer.m_Duration.m_Int64 + unaff_EBX + 6);
      if (iVar7 == 0) {
        uVar8 = (*(code *)**(undefined4 **)**(undefined4 **)(&DAT_00b8ee3e + unaff_EBX))
                          ((undefined4 *)**(undefined4 **)(&DAT_00b8ee3e + unaff_EBX),local_24);
      }
      else {
        uVar8 = (**(code **)(*(int *)**(undefined4 **)(&DAT_00b8ee3e + unaff_EBX) + 4))
                          ((int *)**(undefined4 **)(&DAT_00b8ee3e + unaff_EBX),iVar7,local_24);
      }
      *(undefined4 *)((int)&g_AIAgentTaskTimings[2].startTimer.m_Duration.m_Int64 + unaff_EBX + 6) =
           uVar8;
    }
  }
  iVar5 = *(int *)((int)&g_AIAgentTaskTimings[3].pszTask + unaff_EBX + 2) + 1;
  *(int *)((int)&g_AIAgentTaskTimings[3].pszTask + unaff_EBX + 2) = iVar5;
  iVar7 = *(int *)((int)&g_AIAgentTaskTimings[2].startTimer.m_Duration.m_Int64 + unaff_EBX + 6);
  *(int *)((int)&g_AIAgentTaskTimings[3].selectSchedule.m_Duration.m_Int64 + unaff_EBX + 2) = iVar7;
  iVar5 = (iVar5 - iVar1) + -1;
  if (0 < iVar5) {
    _V_memmove((void *)(iVar7 + iVar2 * 4),(void *)(iVar7 + iVar1 * 4),iVar5 * 4);
    iVar7 = *(int *)((int)&g_AIAgentTaskTimings[2].startTimer.m_Duration.m_Int64 + unaff_EBX + 6);
  }
  puVar6 = (undefined4 *)(iVar1 * 4 + iVar7);
  if (puVar6 != (undefined4 *)0x0) {
    *puVar6 = pcVar3;
  }
  *(char **)((int)&PTR_OnSave_00cb2be4 + unaff_EBX + 2) = pcVar3;
  ___cxa_guard_release(unaff_EBX + 0xd9899a);
LAB_000b7d61:
  *(undefined4 *)((int)&PTR_FVisible_00cb25f8 + unaff_EBX + 2) = 0x17;
  *(int *)((int)&PTR_GetFriction_00cb25f4 + unaff_EBX + 2) = unaff_EBX + 0xcb2662;
  return (datamap_t *)((int)&PTR_GetFriction_00cb25f4 + unaff_EBX + 2);
}


/* CPropPersonalitySphere::GetDataDescMap at 00763230 */

/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: datamap_t * GetDataDescMap(CPropPersonalitySphere * this) */

datamap_t * __thiscall CPropPersonalitySphere::GetDataDescMap(CPropPersonalitySphere *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x6070dc);
}


/* CPropPersonalitySphere::GetBaseMap at 00763240 */

datamap_t * CPropPersonalitySphere::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4e431c);
}


/* __tcf_1 at 00a405f0 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x4100d6)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x4100ca) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x4100d6));
  }
  *(undefined4 *)(unaff_EBX + 0x4100d6) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x4100d2)) {
    if (*(int *)(unaff_EBX + 0x4100ca) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20655e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20655e),*(int *)(unaff_EBX + 0x4100ca));
      *(undefined4 *)(unaff_EBX + 0x4100ca) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x4100ce) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x4100ca);
  *(int *)(unaff_EBX + 0x4100da) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x4100d2)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20655e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20655e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x4100ca) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x4100ce) = 0;
  }
  return;
}


/* CPropPersonalitySphere::CPropPersonalitySphere at 00765270 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: void CPropPersonalitySphere(CPropPersonalitySphere * this) */

void __thiscall CPropPersonalitySphere::CPropPersonalitySphere(CPropPersonalitySphere *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CPhysicsProp::CPhysicsProp(&this->super_CPhysicsProp);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
  super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x57106b);
  (this->super_CPhysicsProp).super_CBreakableProp.super_IBreakableWithPropData.
  _vptr_IBreakableWithPropData = (_func_int_varargs **)(unaff_EBX + 0x5714cf);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x571553);
  (this->super_CPhysicsProp).super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(unaff_EBX + 0x571583);
  (this->m_pastPositions).m_Memory.m_pMemory = (Vector *)0x0;
  (this->m_pastPositions).m_Memory.m_nAllocationCount = 0;
  (this->m_pastPositions).m_Memory.m_nGrowSize = 0;
  (this->m_pastPositions).m_Size = 0;
  (this->m_pastPositions).m_pElements = (Vector *)0x0;
  (this->m_iLineOrder).m_Memory.m_pMemory = (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0;
  (this->m_iLineOrder).m_Memory.m_nAllocationCount = 0;
  (this->m_iLineOrder).m_Memory.m_nGrowSize = 0;
  (this->m_iLineOrder).m_Size = 0;
  (this->m_iLineOrder).m_pElements = (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0;
  this->m_iCurrentPriority = 0;
  this->m_iCurrentLine = 0;
  (this->m_iszLastLinePlayed).pszValue = (char *)0x0;
  (this->m_iszInfoTargetScriptName).pszValue = (char *)0x0;
  (this->m_iszVoiceName).pszValue = (char *)0x0;
  (this->m_iszDeathSoundScriptName).pszValue = (char *)0x0;
  (this->m_iszLookAnimationName).pszValue = (char *)0x0;
  this->m_flBetweenVOPadding = 2.5;
  this->m_flIdleWaitTime = 4.0;
  this->m_flAbuseLevel = 0.0;
  this->m_bFirstPickup = true;
  this->m_bPanicked = false;
  this->m_bHeld = false;
  return;
}


/* CPropPersonalitySphere::CPropPersonalitySphere at 00765390 */

/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: void CPropPersonalitySphere(CPropPersonalitySphere * this,
   CPropPersonalitySphere * this) */

void __thiscall
CPropPersonalitySphere::CPropPersonalitySphere
          (CPropPersonalitySphere *this,CPropPersonalitySphere *this_1)

{
  CPropPersonalitySphere(this);
  return;
}


/* CEntityFactory<CPropPersonalitySphere>::Create at 007672d0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CPropPersonalitySphere> *
   this, char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CPropPersonalitySphere>::Create
          (CEntityFactory<CPropPersonalitySphere> *this,char *pClassName)

{
  CPropPersonalitySphere *this_00;
  
                    /* Unresolved local var: CPropPersonalitySphere * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x7ec);
  CPropPersonalitySphere::CPropPersonalitySphere(this_00);
  (**(code **)((int)(this_00->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x74))(this_00,pClassName);
  return &(this_00->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
          super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CPropPersonalitySphere::~CPropPersonalitySphere at 00764eb0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: void ~CPropPersonalitySphere(CPropPersonalitySphere * this, int
   __in_chrg) */

void __thiscall
CPropPersonalitySphere::~CPropPersonalitySphere(CPropPersonalitySphere *this,int __in_chrg)

{
  undefined4 *puVar1;
  int iVar2;
  CUtlVector<int,CUtlMemory<int,_int>_> *pCVar3;
  int unaff_EBX;
  int *piVar4;
  int iVar5;
  CUtlVector<int,CUtlMemory<int,_int>_> *in_stack_ffffffc8;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
  super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x57142a);
  (this->super_CPhysicsProp).super_CBreakableProp.super_IBreakableWithPropData.
  _vptr_IBreakableWithPropData = (_func_int_varargs **)(unaff_EBX + 0x57188e);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x571912);
  (this->super_CPhysicsProp).super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(unaff_EBX + 0x571942);
  local_20 = (this->m_iLineOrder).m_Size;
  puVar1 = *(undefined4 **)(unaff_EBX + 0x4e1c9e);
  iVar2 = local_20 * 0x14;
  while( true ) {
    iVar5 = iVar2 + -0x14;
    local_20 = local_20 + -1;
    if (local_20 < 0) break;
    piVar4 = (int *)((int)&(this->m_iLineOrder).m_Memory.m_pMemory[-1].m_Memory.m_pMemory + iVar2);
    piVar4[3] = 0;
    if (piVar4[2] < 0) {
      pCVar3 = (CUtlVector<int,CUtlMemory<int,_int>_> *)*piVar4;
    }
    else {
      pCVar3 = (CUtlVector<int,CUtlMemory<int,_int>_> *)*piVar4;
      if (pCVar3 != (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0) {
        in_stack_ffffffc8 = pCVar3;
        (**(code **)(*(int *)*puVar1 + 8))((int *)*puVar1);
        pCVar3 = (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0;
        *piVar4 = 0;
      }
      piVar4[1] = 0;
    }
    piVar4[4] = (int)pCVar3;
    iVar2 = iVar5;
    if (-1 < piVar4[2]) {
      if (pCVar3 != (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e1c9e) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x4e1c9e));
        *piVar4 = 0;
        in_stack_ffffffc8 = pCVar3;
      }
      piVar4[1] = 0;
    }
  }
  (this->m_iLineOrder).m_Size = 0;
  if ((this->m_iLineOrder).m_Memory.m_nGrowSize < 0) {
    pCVar3 = (this->m_iLineOrder).m_Memory.m_pMemory;
  }
  else {
    pCVar3 = (this->m_iLineOrder).m_Memory.m_pMemory;
    if (pCVar3 != (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0) {
      in_stack_ffffffc8 = pCVar3;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e1c9e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4e1c9e));
      (this->m_iLineOrder).m_Memory.m_pMemory = (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0;
      pCVar3 = (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0;
    }
    (this->m_iLineOrder).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_iLineOrder).m_pElements = pCVar3;
  if (-1 < (this->m_iLineOrder).m_Memory.m_nGrowSize) {
    if (pCVar3 != (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e1c9e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4e1c9e));
      (this->m_iLineOrder).m_Memory.m_pMemory = (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0;
      in_stack_ffffffc8 = pCVar3;
    }
    (this->m_iLineOrder).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_pastPositions).m_Size = 0;
  if ((this->m_pastPositions).m_Memory.m_nGrowSize < 0) {
    pCVar3 = (CUtlVector<int,CUtlMemory<int,_int>_> *)(this->m_pastPositions).m_Memory.m_pMemory;
  }
  else {
    pCVar3 = (CUtlVector<int,CUtlMemory<int,_int>_> *)(this->m_pastPositions).m_Memory.m_pMemory;
    if (pCVar3 != (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0) {
      in_stack_ffffffc8 = pCVar3;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e1c9e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4e1c9e));
      pCVar3 = (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0;
      (this->m_pastPositions).m_Memory.m_pMemory = (Vector *)0x0;
    }
    (this->m_pastPositions).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_pastPositions).m_pElements = (Vector *)pCVar3;
  if (-1 < (this->m_pastPositions).m_Memory.m_nGrowSize) {
    if (pCVar3 != (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e1c9e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4e1c9e));
      (this->m_pastPositions).m_Memory.m_pMemory = (Vector *)0x0;
      in_stack_ffffffc8 = pCVar3;
    }
    (this->m_pastPositions).m_Memory.m_nAllocationCount = 0;
  }
  CPhysicsProp::~CPhysicsProp(&this->super_CPhysicsProp,(int)in_stack_ffffffc8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CPropPersonalitySphere::~CPropPersonalitySphere at 007653a0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: void ~CPropPersonalitySphere(CPropPersonalitySphere * this, int
   __in_chrg) */

void __thiscall
CPropPersonalitySphere::~CPropPersonalitySphere(CPropPersonalitySphere *this,int __in_chrg)

{
  undefined4 *puVar1;
  int iVar2;
  int iVar3;
  CUtlVector<int,CUtlMemory<int,_int>_> *pCVar4;
  Vector *pVVar5;
  int unaff_EBX;
  int *piVar6;
  int iVar7;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
  super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x570f3a);
  (this->super_CPhysicsProp).super_CBreakableProp.super_IBreakableWithPropData.
  _vptr_IBreakableWithPropData = (_func_int_varargs **)(unaff_EBX + 0x57139e);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x571422);
  (this->super_CPhysicsProp).super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(unaff_EBX + 0x571452);
  local_20 = (this->m_iLineOrder).m_Size;
  puVar1 = *(undefined4 **)(unaff_EBX + 0x4e17ae);
  iVar2 = local_20 * 0x14;
  while( true ) {
    iVar7 = iVar2 + -0x14;
    local_20 = local_20 + -1;
    if (local_20 < 0) break;
    piVar6 = (int *)((int)&(this->m_iLineOrder).m_Memory.m_pMemory[-1].m_Memory.m_pMemory + iVar2);
    piVar6[3] = 0;
    if (piVar6[2] < 0) {
      iVar3 = *piVar6;
    }
    else {
      if (*piVar6 != 0) {
        (**(code **)(*(int *)*puVar1 + 8))((int *)*puVar1,*piVar6);
        *piVar6 = 0;
      }
      iVar3 = 0;
      piVar6[1] = 0;
    }
    piVar6[4] = iVar3;
    iVar2 = iVar7;
    if (-1 < piVar6[2]) {
      if (iVar3 != 0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e17ae) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x4e17ae),iVar3);
        *piVar6 = 0;
      }
      piVar6[1] = 0;
    }
  }
  (this->m_iLineOrder).m_Size = 0;
  if ((this->m_iLineOrder).m_Memory.m_nGrowSize < 0) {
    pCVar4 = (this->m_iLineOrder).m_Memory.m_pMemory;
  }
  else {
    pCVar4 = (this->m_iLineOrder).m_Memory.m_pMemory;
    if (pCVar4 != (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e17ae) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4e17ae),pCVar4);
      (this->m_iLineOrder).m_Memory.m_pMemory = (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0;
    }
    pCVar4 = (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0;
    (this->m_iLineOrder).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_iLineOrder).m_pElements = pCVar4;
  if (-1 < (this->m_iLineOrder).m_Memory.m_nGrowSize) {
    if (pCVar4 != (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e17ae) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4e17ae),pCVar4);
      (this->m_iLineOrder).m_Memory.m_pMemory = (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0;
    }
    (this->m_iLineOrder).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_pastPositions).m_Size = 0;
  if ((this->m_pastPositions).m_Memory.m_nGrowSize < 0) {
    pVVar5 = (this->m_pastPositions).m_Memory.m_pMemory;
  }
  else {
    pVVar5 = (this->m_pastPositions).m_Memory.m_pMemory;
    if (pVVar5 != (Vector *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e17ae) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4e17ae),pVVar5);
      (this->m_pastPositions).m_Memory.m_pMemory = (Vector *)0x0;
    }
    pVVar5 = (Vector *)0x0;
    (this->m_pastPositions).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_pastPositions).m_pElements = pVVar5;
  if (-1 < (this->m_pastPositions).m_Memory.m_nGrowSize) {
    if (pVVar5 != (Vector *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e17ae) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4e17ae),pVVar5);
      (this->m_pastPositions).m_Memory.m_pMemory = (Vector *)0x0;
    }
    (this->m_pastPositions).m_Memory.m_nAllocationCount = 0;
  }
  CPhysicsProp::~CPhysicsProp(&this->super_CPhysicsProp,__in_chrg);
  return;
}


/* CPropPersonalitySphere::~CPropPersonalitySphere at 00765760 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: void ~CPropPersonalitySphere(CPropPersonalitySphere * this, int
   __in_chrg) */

void __thiscall
CPropPersonalitySphere::~CPropPersonalitySphere(CPropPersonalitySphere *this,int __in_chrg)

{
  undefined4 *puVar1;
  int iVar2;
  int iVar3;
  CUtlVector<int,CUtlMemory<int,_int>_> *pCVar4;
  Vector *pVVar5;
  int unaff_EBX;
  int *piVar6;
  int iVar7;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
  super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x570b7a);
  (this->super_CPhysicsProp).super_CBreakableProp.super_IBreakableWithPropData.
  _vptr_IBreakableWithPropData = (_func_int_varargs **)(unaff_EBX + 0x570fde);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x571062);
  (this->super_CPhysicsProp).super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(unaff_EBX + 0x571092);
  local_20 = (this->m_iLineOrder).m_Size;
  puVar1 = *(undefined4 **)(unaff_EBX + 0x4e13ee);
  iVar2 = local_20 * 0x14;
  while( true ) {
    iVar7 = iVar2 + -0x14;
    local_20 = local_20 + -1;
    if (local_20 < 0) break;
    piVar6 = (int *)((int)&(this->m_iLineOrder).m_Memory.m_pMemory[-1].m_Memory.m_pMemory + iVar2);
    piVar6[3] = 0;
    if (piVar6[2] < 0) {
      iVar3 = *piVar6;
    }
    else {
      if (*piVar6 != 0) {
        (**(code **)(*(int *)*puVar1 + 8))((int *)*puVar1,*piVar6);
        *piVar6 = 0;
      }
      iVar3 = 0;
      piVar6[1] = 0;
    }
    piVar6[4] = iVar3;
    iVar2 = iVar7;
    if (-1 < piVar6[2]) {
      if (iVar3 != 0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e13ee) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x4e13ee),iVar3);
        *piVar6 = 0;
      }
      piVar6[1] = 0;
    }
  }
  (this->m_iLineOrder).m_Size = 0;
  if ((this->m_iLineOrder).m_Memory.m_nGrowSize < 0) {
    pCVar4 = (this->m_iLineOrder).m_Memory.m_pMemory;
  }
  else {
    pCVar4 = (this->m_iLineOrder).m_Memory.m_pMemory;
    if (pCVar4 != (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e13ee) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4e13ee),pCVar4);
      (this->m_iLineOrder).m_Memory.m_pMemory = (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0;
    }
    pCVar4 = (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0;
    (this->m_iLineOrder).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_iLineOrder).m_pElements = pCVar4;
  if (-1 < (this->m_iLineOrder).m_Memory.m_nGrowSize) {
    if (pCVar4 != (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e13ee) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4e13ee),pCVar4);
      (this->m_iLineOrder).m_Memory.m_pMemory = (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0;
    }
    (this->m_iLineOrder).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_pastPositions).m_Size = 0;
  if ((this->m_pastPositions).m_Memory.m_nGrowSize < 0) {
    pVVar5 = (this->m_pastPositions).m_Memory.m_pMemory;
  }
  else {
    pVVar5 = (this->m_pastPositions).m_Memory.m_pMemory;
    if (pVVar5 != (Vector *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e13ee) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4e13ee),pVVar5);
      (this->m_pastPositions).m_Memory.m_pMemory = (Vector *)0x0;
    }
    pVVar5 = (Vector *)0x0;
    (this->m_pastPositions).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_pastPositions).m_pElements = pVVar5;
  if (-1 < (this->m_pastPositions).m_Memory.m_nGrowSize) {
    if (pVVar5 != (Vector *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e13ee) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4e13ee),pVVar5);
      (this->m_pastPositions).m_Memory.m_pMemory = (Vector *)0x0;
    }
    (this->m_pastPositions).m_Memory.m_nAllocationCount = 0;
  }
  CPhysicsProp::~CPhysicsProp(&this->super_CPhysicsProp,__in_chrg);
  return;
}


/* CPropPersonalitySphere::Spawn at 00764320 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: void Spawn(CPropPersonalitySphere * this) */

void __thiscall CPropPersonalitySphere::Spawn(CPropPersonalitySphere *this)

{
  uint *puVar1;
  ushort uVar2;
  int *piVar3;
  CBaseEdict *this_00;
  ushort *puVar4;
  BASEPTR func;
  BASEPTR func_00;
  int iVar5;
  IChangeInfoAccessor *pIVar6;
  uint uVar7;
  uint uVar8;
  int unaff_EBX;
  uint local_70;
  
  ___i686_get_pc_thunk_bx();
  (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x80))(this,unaff_EBX + 0x342c3f,unaff_EBX + 0x34aa33);
  CPhysicsProp::Spawn(&this->super_CPhysicsProp);
  iVar5 = CBaseAnimating::LookupSequence((CBaseAnimating *)this,(char *)(unaff_EBX + 0x34aaa1));
  CBaseAnimating::ResetSequence((CBaseAnimating *)this,iVar5);
  if (*(float *)(&DAT_003685df + unaff_EBX) !=
      (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.m_flCycle
      .m_Value) {
    if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
        false) {
      this_00 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                  super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)**(undefined4 **)(&DAT_004e2837 + unaff_EBX);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_007645c4:
            puVar4[(uint)uVar2 + uVar7 * 0x14 + 1] = 0x41c;
            puVar4[uVar7 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != 0x41c) {
            local_70 = 0;
            do {
              uVar8 = local_70 + 1;
              local_70 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar2) {
                if (uVar2 != 0x13) goto LAB_007645c4;
                pIVar6->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar7 * 0x14 + local_70 + 1] != 0x41c);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(&DAT_004e2837 + unaff_EBX) + 0xfa2) =
               *(short *)(**(int **)(&DAT_004e2837 + unaff_EBX) + 0xfa2) + 1;
          piVar3 = *(int **)(&DAT_004e2837 + unaff_EBX);
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar3;
          iVar5 = *piVar3 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar5 + 2) = 0x41c;
          *(undefined2 *)(iVar5 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = (uint *)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
      *puVar1 = *puVar1 | 1;
    }
    (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.m_flCycle.
    m_Value = 1.0;
  }
  CBreakableProp::DisableAutoFade((CBreakableProp *)this);
  iVar5 = CBaseAnimating::LookupAttachment((CBaseAnimating *)this,(char *)(unaff_EBX + 0x351185));
  this->m_iEyeballAttachment = iVar5;
  piVar3 = *(int **)(&DAT_004e2833 + unaff_EBX);
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x10b1);
  CBaseEntity::ThinkSet
            ((CBaseEntity *)this,func,
             *(float *)(CAI_GoalEntity::OnEntityCreated + unaff_EBX + 3) + *(float *)(*piVar3 + 0xc)
             ,*(char **)(unaff_EBX + 0x572823));
  if (this->m_iCoreType == CORETYPE_AQUARIUM) {
    this->m_flLastHeldTime = *(float *)(*piVar3 + 0xc);
    func_00.__delta = 0;
    func_00.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0x9cf);
    CBaseEntity::ThinkSet((CBaseEntity *)this,func_00,0.0,(char *)0x0);
    CBaseEntity::SetNextThink
              ((CBaseEntity *)this,*(float *)(*piVar3 + 0xc) + this->m_flBetweenVOPadding + 0.0,
               (char *)0x0);
    return;
  }
  return;
}


/* CPropPersonalitySphere::RegisterSoundEvent at 007639b0 */

/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: void RegisterSoundEvent(CPropPersonalitySphere * this, SphereEvent_e
   eventName, int count) */

void __thiscall
CPropPersonalitySphere::RegisterSoundEvent
          (CPropPersonalitySphere *this,SphereEvent_e eventName,int count)

{
  undefined4 *puVar1;
  uint uVar2;
  int iVar3;
  char *pcVar4;
  int iVar5;
  int *piVar6;
  int iVar7;
  int unaff_EBX;
  CUtlVector<int,CUtlMemory<int,_int>_> *pCVar8;
  int local_248;
  int local_244;
  int local_240;
  CFmtStrN<256> local_238;
  CFmtStrN<256> local_12c;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(&DAT_004e31ef + unaff_EBX);
  (&(this->super_CPhysicsProp).field_0x766)[eventName] = 1;
  if (eventName != SPHERE_EVENT_INFO_TARGET) {
    if (this->m_iCoreType == CORETYPE_RICHARD) {
                    /* Unresolved local var: int i@[???] */
      if (0 < count) {
        local_240 = 1;
        local_244 = 0;
        puVar1 = *(undefined4 **)(unaff_EBX + 0x4e319b);
        do {
          pcVar4 = (this->m_iszVoiceName).pszValue;
          if (pcVar4 == (char *)0x0) {
            pcVar4 = (char *)(unaff_EBX + 0x2eb363);
          }
          CFmtStrN<256>::CFmtStrN
                    (&local_12c,(char *)(unaff_EBX + 0x351a75),pcVar4,
                     *(undefined4 *)(unaff_EBX + 0x6068ff + eventName * 4),local_240);
          CBaseEntity::PrecacheScriptSound(local_12c.m_szBuf);
          pCVar8 = (this->m_iLineOrder).m_Memory.m_pMemory + eventName;
          uVar2 = pCVar8->m_Size;
          iVar7 = uVar2 + 1;
          iVar5 = (pCVar8->m_Memory).m_nAllocationCount;
          if (iVar5 < iVar7) {
            iVar3 = (pCVar8->m_Memory).m_nGrowSize;
            if (-1 < iVar3) {
              if (iVar3 == 0) {
                if ((iVar5 == 0) && (iVar5 = 8, iVar7 < 9)) {
                  local_248 = 0x20;
                }
                else {
                  do {
                    local_248 = iVar5;
                    iVar5 = local_248 * 2;
                  } while (iVar5 < iVar7);
                  local_248 = local_248 * 8;
                }
                goto LAB_00763bc3;
              }
              iVar5 = ((int)uVar2 / iVar3 + 1) * iVar3;
              if (iVar5 < iVar7) {
                if (iVar5 != 0) {
LAB_00763ba0:
                  do {
                    iVar5 = (iVar5 + iVar7) / 2;
                  } while (iVar5 < iVar7);
                  goto LAB_00763bb6;
                }
                if (iVar7 < 0) {
                  iVar5 = -1;
                  local_248 = -4;
                }
                else {
                  local_248 = 0;
                  if (iVar7 != 0) goto LAB_00763ba0;
                }
              }
              else {
LAB_00763bb6:
                local_248 = iVar5 * 4;
              }
LAB_00763bc3:
              (pCVar8->m_Memory).m_nAllocationCount = iVar5;
              piVar6 = (pCVar8->m_Memory).m_pMemory;
              if (piVar6 == (int *)0x0) {
                piVar6 = (int *)(*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x4e319b))
                                          ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x4e319b),
                                           local_248);
                (pCVar8->m_Memory).m_pMemory = piVar6;
              }
              else {
                piVar6 = (int *)(**(code **)(*(int *)*puVar1 + 4))((int *)*puVar1,piVar6,local_248);
                (pCVar8->m_Memory).m_pMemory = piVar6;
              }
            }
          }
          pCVar8->m_Size = pCVar8->m_Size + 1;
          piVar6 = (pCVar8->m_Memory).m_pMemory;
          pCVar8->m_pElements = piVar6;
          iVar5 = ~uVar2 + pCVar8->m_Size;
          if (0 < iVar5) {
            _V_memmove(piVar6 + iVar7,piVar6 + uVar2,iVar5 * 4);
          }
          piVar6 = (pCVar8->m_Memory).m_pMemory + uVar2;
          if (piVar6 != (int *)0x0) {
            *piVar6 = local_240;
          }
          iVar7 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e3227) + 8))
                            ((int *)**(undefined4 **)(unaff_EBX + 0x4e3227),0,local_244);
          piVar6 = (this->m_iLineOrder).m_Memory.m_pMemory[eventName].m_Memory.m_pMemory;
          piVar6[local_244] = piVar6[iVar7];
          (this->m_iLineOrder).m_Memory.m_pMemory[eventName].m_Memory.m_pMemory[iVar7] = local_240;
          local_240 = local_240 + 1;
          local_244 = local_244 + 1;
        } while (local_244 != count);
      }
    }
    else {
      pcVar4 = (this->m_iszVoiceName).pszValue;
      if (pcVar4 == (char *)0x0) {
        pcVar4 = (char *)(unaff_EBX + 0x2eb363);
      }
      CFmtStrN<256>::CFmtStrN
                (&local_238,(char *)(unaff_EBX + 0x351a7f),pcVar4,
                 *(undefined4 *)(unaff_EBX + 0x6068ff + eventName * 4));
      CBaseEntity::PrecacheScriptSound(local_238.m_szBuf);
    }
  }
  if (local_20 != **(int **)(&DAT_004e31ef + unaff_EBX)) {
                    /* WARNING: Subroutine does not return */
    ___stack_chk_fail();
  }
  return;
}


/* CPropPersonalitySphere::Precache at 007666a0 */

/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: void Precache(CPropPersonalitySphere * this) */

void __thiscall CPropPersonalitySphere::Precache(CPropPersonalitySphere *this)

{
  CUtlVector<CUtlVector<int,_CUtlMemory<int,_int>_>,CUtlMemory<CUtlVector<int,_CUtlMemory<int,_int>_>,_int>_>
  *this_00;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CPhysicsProp::Precache(&this->super_CPhysicsProp);
  SetupVOList(this);
                    /* Unresolved local var: uint i@[???] */
  (this->super_CPhysicsProp).field_0x766 = 0;
  this_00 = &this->m_iLineOrder;
  CUtlVector<CUtlVector<int,_CUtlMemory<int,_int>_>,CUtlMemory<CUtlVector<int,_CUtlMemory<int,_int>_>,_int>_>
  ::InsertBefore(this_00,(this->m_iLineOrder).m_Size);
  (this->super_CPhysicsProp).field_0x767 = 0;
  CUtlVector<CUtlVector<int,_CUtlMemory<int,_int>_>,CUtlMemory<CUtlVector<int,_CUtlMemory<int,_int>_>,_int>_>
  ::InsertBefore(this_00,(this->m_iLineOrder).m_Size);
  this->field_0x768 = 0;
  CUtlVector<CUtlVector<int,_CUtlMemory<int,_int>_>,CUtlMemory<CUtlVector<int,_CUtlMemory<int,_int>_>,_int>_>
  ::InsertBefore(this_00,(this->m_iLineOrder).m_Size);
  this->field_0x769 = 0;
  CUtlVector<CUtlVector<int,_CUtlMemory<int,_int>_>,CUtlMemory<CUtlVector<int,_CUtlMemory<int,_int>_>,_int>_>
  ::InsertBefore(this_00,(this->m_iLineOrder).m_Size);
  this->field_0x76a = 0;
  CUtlVector<CUtlVector<int,_CUtlMemory<int,_int>_>,CUtlMemory<CUtlVector<int,_CUtlMemory<int,_int>_>,_int>_>
  ::InsertBefore(this_00,(this->m_iLineOrder).m_Size);
  this->field_0x76b = 0;
  CUtlVector<CUtlVector<int,_CUtlMemory<int,_int>_>,CUtlMemory<CUtlVector<int,_CUtlMemory<int,_int>_>,_int>_>
  ::InsertBefore(this_00,(this->m_iLineOrder).m_Size);
  this->field_0x76c = 0;
  CUtlVector<CUtlVector<int,_CUtlMemory<int,_int>_>,CUtlMemory<CUtlVector<int,_CUtlMemory<int,_int>_>,_int>_>
  ::InsertBefore(this_00,(this->m_iLineOrder).m_Size);
  this->field_0x76d = 0;
  CUtlVector<CUtlVector<int,_CUtlMemory<int,_int>_>,CUtlMemory<CUtlVector<int,_CUtlMemory<int,_int>_>,_int>_>
  ::InsertBefore(this_00,(this->m_iLineOrder).m_Size);
  this->field_0x76e = 0;
  CUtlVector<CUtlVector<int,_CUtlMemory<int,_int>_>,CUtlMemory<CUtlVector<int,_CUtlMemory<int,_int>_>,_int>_>
  ::InsertBefore(this_00,(this->m_iLineOrder).m_Size);
  this->field_0x76f = 0;
  CUtlVector<CUtlVector<int,_CUtlMemory<int,_int>_>,CUtlMemory<CUtlVector<int,_CUtlMemory<int,_int>_>,_int>_>
  ::InsertBefore(this_00,(this->m_iLineOrder).m_Size);
  this->field_0x770 = 0;
  CUtlVector<CUtlVector<int,_CUtlMemory<int,_int>_>,CUtlMemory<CUtlVector<int,_CUtlMemory<int,_int>_>,_int>_>
  ::InsertBefore(this_00,(this->m_iLineOrder).m_Size);
  this->field_0x771 = 0;
  CUtlVector<CUtlVector<int,_CUtlMemory<int,_int>_>,CUtlMemory<CUtlVector<int,_CUtlMemory<int,_int>_>,_int>_>
  ::InsertBefore(this_00,(this->m_iLineOrder).m_Size);
  this->field_0x772 = 0;
  CUtlVector<CUtlVector<int,_CUtlMemory<int,_int>_>,CUtlMemory<CUtlVector<int,_CUtlMemory<int,_int>_>,_int>_>
  ::InsertBefore(this_00,(this->m_iLineOrder).m_Size);
  this->field_0x773 = 0;
  CUtlVector<CUtlVector<int,_CUtlMemory<int,_int>_>,CUtlMemory<CUtlVector<int,_CUtlMemory<int,_int>_>,_int>_>
  ::InsertBefore(this_00,(this->m_iLineOrder).m_Size);
  this->field_0x774 = 0;
  CUtlVector<CUtlVector<int,_CUtlMemory<int,_int>_>,CUtlMemory<CUtlVector<int,_CUtlMemory<int,_int>_>,_int>_>
  ::InsertBefore(this_00,(this->m_iLineOrder).m_Size);
  if (this->m_iCoreType == CORETYPE_RICHARD) {
    RegisterSoundEvent(this,SPHERE_EVENT_INFO_TARGET,1);
    RegisterSoundEvent(this,SPHERE_EVENT_WAITING,7);
    RegisterSoundEvent(this,SPHERE_EVENT_HELD,0x24);
    RegisterSoundEvent(this,SPHERE_EVENT_HELD_PANIC,0x1e);
    RegisterSoundEvent(this,SPHERE_EVENT_DROP,3);
    RegisterSoundEvent(this,SPHERE_EVENT_NOTHELD,10);
    RegisterSoundEvent(this,SPHERE_EVENT_NOTHELD_RELIEVED,7);
    RegisterSoundEvent(this,SPHERE_EVENT_FIRST_PICKUP,1);
    RegisterSoundEvent(this,SPHERE_EVENT_FRUSTRATION,0xd);
    RegisterSoundEvent(this,SPHERE_EVENT_HURT,8);
    RegisterSoundEvent(this,SPHERE_EVENT_HURT_REPEATEDLY,8);
    RegisterSoundEvent(this,SPHERE_EVENT_VELOCITY_UP_HIGH,1);
    RegisterSoundEvent(this,SPHERE_EVENT_VELOCITY_DOWN_HIGH,1);
    RegisterSoundEvent(this,SPHERE_EVENT_VELOCITY_LATERAL_LOW,1);
    RegisterSoundEvent(this,SPHERE_EVENT_VELOCITY_LATERAL_HIGH,1);
  }
  else if (this->m_iCoreType == CORETYPE_AQUARIUM) {
    RegisterSoundEvent(this,SPHERE_EVENT_NOTHELD,1);
    RegisterSoundEvent(this,SPHERE_EVENT_HELD,1);
    RegisterSoundEvent(this,SPHERE_EVENT_HURT,1);
  }
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x3486b6));
  return;
}


/* CPropPersonalitySphere::InputPanic at 00763250 */

/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: void InputPanic(CPropPersonalitySphere * this, inputdata_t * inputdata)
    */

void __thiscall
CPropPersonalitySphere::InputPanic(CPropPersonalitySphere *this,inputdata_t *inputdata)

{
  this->m_bPanicked = true;
  return;
}


/* CPropPersonalitySphere::InputStartTalking at 007642a0 */

/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: void InputStartTalking(CPropPersonalitySphere * this, inputdata_t *
   inputdata) */

void __thiscall
CPropPersonalitySphere::InputStartTalking(CPropPersonalitySphere *this,inputdata_t *inputdata)

{
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0xa4f);
  CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(**(int **)(&DAT_004e28b3 + unaff_EBX) + 0xc) + this->m_flBetweenVOPadding +
             0.0,(char *)0x0);
  return;
}


/* CPropPersonalitySphere::StartTalking at 00763300 */

/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: void StartTalking(CPropPersonalitySphere * this, float flDelay) */

void __thiscall CPropPersonalitySphere::StartTalking(CPropPersonalitySphere *this,float flDelay)

{
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0x19ef);
  CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(**(int **)(CEntityFactory<CFunc_LOD>::Destroy + unaff_EBX + 3) + 0xc) +
             this->m_flBetweenVOPadding + flDelay,(char *)0x0);
  return;
}


/* CPropPersonalitySphere::OnPhysGunPickup at 00764630 */

/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void OnPhysGunPickup(CPropPersonalitySphere * this, CBasePlayer *
   pPhysGunUser, PhysGunPickup_t reason) */

void __thiscall
CPropPersonalitySphere::OnPhysGunPickup
          (CPropPersonalitySphere *this,CBasePlayer *pPhysGunUser,PhysGunPickup_t reason)

{
  BASEPTR func;
  int nSequence;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_bFirstPickup == false) {
    TrySpeakLine(this,8,3);
  }
  else {
    func.__delta = 0;
    func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0x6bc);
    CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
    CBaseEntity::SetNextThink
              ((CBaseEntity *)this,
               *(float *)(**(int **)(unaff_EBX + 0x4e2520) + 0xc) + this->m_flBetweenVOPadding + 0.0
               ,(char *)0x0);
    TrySpeakLine(this,7,10);
  }
  this->m_bFirstPickup = false;
  this->m_bHeld = true;
  nSequence = CBaseAnimating::LookupSequence((CBaseAnimating *)this,(char *)(unaff_EBX + 0x350e7a));
  CBaseAnimating::ResetSequence((CBaseAnimating *)this,nSequence);
  CPhysicsProp::EnableMotion(&this->super_CPhysicsProp);
  CPhysicsProp::OnPhysGunPickup(&this->super_CPhysicsProp,pPhysGunUser,reason);
  return;
}


/* CPropPersonalitySphere::TalkingThink at 00764d00 */

/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: void TalkingThink(CPropPersonalitySphere * this) */

void __thiscall CPropPersonalitySphere::TalkingThink(CPropPersonalitySphere *this)

{
  float fVar1;
  int iVar2;
  char *label;
  int nSequence;
  int unaff_EBX;
  float fVar3;
  
                    /* Unresolved local var: int iCurSequence@[???]
                       Unresolved local var: int iLookSequence@[???]
                       Unresolved local var: float flInterval@[???] */
  ___i686_get_pc_thunk_bx();
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(unaff_EBX + 0x369136) + *(float *)(**(int **)(unaff_EBX + 0x4e1e56) + 0xc),
             (char *)0x0);
  if ((this->m_bFirstPickup == false) || (this->m_iCoreType == CORETYPE_AQUARIUM)) {
    iVar2 = (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
            m_nSequence.m_Value;
    label = (this->m_iszLookAnimationName).pszValue;
    if (label == (char *)0x0) {
      label = (char *)(unaff_EBX + 0x2ea016);
    }
    nSequence = CBaseAnimating::LookupSequence((CBaseAnimating *)this,label);
    if (iVar2 != nSequence) {
      CBaseAnimating::ResetSequence((CBaseAnimating *)this,nSequence);
    }
    fVar1 = *(float *)(**(int **)(unaff_EBX + 0x4e1e56) + 0xc);
    fVar3 = CBaseEntity::GetLastThink((CBaseEntity *)this,(char *)0x0);
    this->m_flAbuseLevel =
         (float)((double)this->m_flAbuseLevel *
                ((double)(fVar1 - fVar3) * *(double *)(unaff_EBX + 0x368362) +
                *(double *)(unaff_EBX + 0x367ea2)));
    CheckForInfoTargets(this);
    UpdatePositions(this);
    if (-1 < this->m_iPendingPriority) {
      TrySpeakLine(this,this->m_iPendingLine,this->m_iPendingPriority);
    }
    if ((this->m_flLineCompletionTime + this->m_flIdleWaitTime <
         *(float *)(**(int **)(unaff_EBX + 0x4e1e56) + 0xc)) && (this->m_bHeld != false)) {
      if (this->m_bPanicked != false) {
        TrySpeakLine(this,3,0);
        return;
      }
      TrySpeakLine(this,2,0);
      return;
    }
  }
  else {
    TrySpeakLine(this,1,0);
  }
  return;
}


/* CPropPersonalitySphere::UpdatePositions at 00763e80 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void UpdatePositions(CPropPersonalitySphere * this) */

void __thiscall CPropPersonalitySphere::UpdatePositions(CPropPersonalitySphere *this)

{
  int *piVar1;
  uint uVar2;
  CBasePlayer *this_00;
  float *pfVar3;
  int iVar4;
  Vector *pVVar5;
  int iVar6;
  float *pfVar7;
  int unaff_EBX;
  float fVar8;
  float fVar9;
  vec_t vVar10;
  float fVar11;
  float fVar12;
  float fVar13;
  int local_44;
  
                    /* Unresolved local var: CPortal_Player * pPlayer@[???]
                       Unresolved local var: float flBestFit@[???]
                       Unresolved local var: int iBestLine@[???]
                       Unresolved local var: Vector vecVelocity@[???] */
  ___i686_get_pc_thunk_bx();
  if ((*(int *)(**(int **)(&DAT_004e2cd6 + unaff_EBX) + 0x14) < 2) &&
     (this_00 = UTIL_GetLocalPlayer(), this_00 != (CBasePlayer *)0x0)) {
    if (((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
         super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
    uVar2 = (this->m_pastPositions).m_Size;
    CUtlVector<Vector,CUtlMemory<Vector,_int>_>::GrowVector(&this->m_pastPositions,1);
    iVar4 = ~uVar2 + (this->m_pastPositions).m_Size;
    if (0 < iVar4) {
      pVVar5 = (this->m_pastPositions).m_Memory.m_pMemory;
      _V_memmove(pVVar5 + uVar2 + 1,pVVar5 + uVar2,iVar4 * 0xc);
    }
    pVVar5 = (this->m_pastPositions).m_Memory.m_pMemory + uVar2;
    if (pVVar5 != (Vector *)0x0) {
      pVVar5->x = (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                  super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.x;
      pVVar5->y = (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                  super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.y;
      pVVar5->z = (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                  super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z;
    }
    iVar4 = (this->m_pastPositions).m_Size;
    if (0x28 < iVar4) {
                    /* Unresolved local var: int numToMove@[???] */
      pVVar5 = (this->m_pastPositions).m_Memory.m_pMemory;
      _V_memmove(pVVar5,pVVar5 + 1,(iVar4 * 3 + -3) * 4);
      piVar1 = &(this->m_pastPositions).m_Size;
      *piVar1 = *piVar1 + -1;
    }
    if ((this->m_bHeld == false) &&
       (*(float *)(unaff_EBX + 0x368caa) + this->m_flLastHeldTime <
        *(float *)(**(int **)(&DAT_004e2cd6 + unaff_EBX) + 0xc))) {
      if (this->m_bPanicked == false) {
        TrySpeakLine(this,5,1);
      }
      else {
        TrySpeakLine(this,6,1);
      }
    }
    pfVar3 = *(float **)(&DAT_004e2cea + unaff_EBX);
    fVar12 = pfVar3[2];
    fVar9 = pfVar3[1];
    fVar11 = *pfVar3;
                    /* Unresolved local var: int i@[???] */
    if (1 < (this->m_pastPositions).m_Size) {
      local_44 = 1;
      iVar4 = (this->m_pastPositions).m_Size;
      pVVar5 = (this->m_pastPositions).m_Memory.m_pMemory;
      iVar6 = 0;
      do {
        pfVar3 = (float *)((int)&pVVar5[1].x + iVar6);
        pfVar7 = (float *)((int)&pVVar5->x + iVar6);
                    /* Unresolved local var: Vector res@[???] */
        fVar11 = fVar11 + (*pfVar3 - *pfVar7);
        fVar9 = fVar9 + (pfVar3[1] - pfVar7[1]);
        fVar12 = fVar12 + (pfVar3[2] - pfVar7[2]);
        local_44 = local_44 + 1;
        iVar6 = iVar6 + 0xc;
      } while (local_44 < iVar4);
      if (1 < iVar4) {
                    /* Unresolved local var: float oofl@[???] */
        fVar8 = *(float *)(unaff_EBX + 0x368a82) / (float)(iVar4 + -1);
        fVar11 = fVar11 * fVar8;
        fVar9 = fVar9 * fVar8;
        fVar12 = fVar12 * fVar8;
      }
    }
    if (this->m_bHeld != false) {
      if (((this_00->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
           super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x1000) != 0) {
        CBaseEntity::CalcAbsoluteVelocity((CBaseEntity *)this_00);
      }
      vVar10 = (this_00->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
               super_CBaseAnimating.super_CBaseEntity.m_vecAbsVelocity.z;
      iVar4 = 0xc;
      if (*(float *)(unaff_EBX + 0x372026) <= vVar10) {
        if (((this_00->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
             super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x1000) != 0) {
          CBaseEntity::CalcAbsoluteVelocity((CBaseEntity *)this_00);
          vVar10 = (this_00->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay
                   .super_CBaseAnimating.super_CBaseEntity.m_vecAbsVelocity.z;
        }
        iVar4 = 0xb;
        if (vVar10 < *(float *)(unaff_EBX + 0x369122) || vVar10 == *(float *)(unaff_EBX + 0x369122))
        {
          if (((this_00->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
               super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x1000) != 0) {
            CBaseEntity::CalcAbsoluteVelocity((CBaseEntity *)this_00);
            vVar10 = (this_00->super_CBaseCombatCharacter).super_CBaseFlex.
                     super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                     m_vecAbsVelocity.z;
          }
          fVar12 = (this_00->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay
                   .super_CBaseAnimating.super_CBaseEntity.m_vecAbsVelocity.x;
          fVar9 = (this_00->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                  super_CBaseAnimating.super_CBaseEntity.m_vecAbsVelocity.y;
                    /* Unresolved local var: __m128 root@[???] */
          fVar12 = SQRT(fVar12 * fVar12 + fVar9 * fVar9 + vVar10 * vVar10);
          if (fVar12 < *(float *)(unaff_EBX + 0x369122) ||
              fVar12 == *(float *)(unaff_EBX + 0x369122)) {
            return;
          }
          iVar4 = 0xe;
        }
      }
      TrySpeakLine(this,iVar4,3);
      return;
    }
                    /* Unresolved local var: __m128 root@[???] */
    fVar8 = *(float *)(**(int **)(&DAT_004e2cd6 + unaff_EBX) + 0xc);
    fVar13 = CBaseEntity::GetLastThink((CBaseEntity *)this,(char *)0x0);
    if ((fVar8 - fVar13) * *(float *)(unaff_EBX + 0x369122) <
        SQRT(fVar11 * fVar11 + fVar9 * fVar9 + fVar12 * fVar12)) {
      TrySpeakLine(this,0xc,5);
    }
  }
  return;
}


/* CPropPersonalitySphere::TrySpeakLine at 00763380 */

/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: void TrySpeakLine(CPropPersonalitySphere * this, int line, int
   priority) */

void __thiscall
CPropPersonalitySphere::TrySpeakLine(CPropPersonalitySphere *this,int line,int priority)

{
  float *pfVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  string_t *soundname;
  string_t *psVar5;
  string_t *psVar6;
  int iVar7;
  int iVar8;
  int *piVar9;
  char *pcVar10;
  int unaff_EBX;
  CUtlVector<int,CUtlMemory<int,_int>_> *pCVar11;
  float fVar12;
  float fVar13;
  float local_258;
  int local_244;
  int local_240;
  undefined1 local_238 [268];
  undefined1 local_12c [268];
  int local_20;
  
                    /* Unresolved local var: float flFinishTime@[???]
                       Unresolved local var: float flAdditionalPadding@[???]
                       Unresolved local var: float flCurDuration@[???] */
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(unaff_EBX + 0x4e381f);
  if ((&(this->super_CPhysicsProp).field_0x766)[line] == '\0') goto LAB_00763450;
  fVar12 = this->m_flFirstNoSkipTime - *(float *)(**(int **)(unaff_EBX + 0x4e37d3) + 0xc);
  if (0.0 < fVar12) {
    if ((((fVar12 < *(float *)(unaff_EBX + 0x36976f)) &&
         (!NAN(fVar12) && !NAN(*(float *)(unaff_EBX + 0x36976f)))) && (line != 9)) &&
       (this->m_iPendingPriority <= priority)) {
      this->m_iPendingPriority = priority;
      this->m_iPendingLine = line;
    }
  }
  if ((this->m_iCurrentPriority < priority) || (line == 0)) {
    this->m_flLineCompletionTime = *(float *)(**(int **)(unaff_EBX + 0x4e37d3) + 0xc);
  }
  pfVar1 = (float *)(**(int **)(unaff_EBX + 0x4e37d3) + 0xc);
  if (*pfVar1 <= this->m_flLineCompletionTime && this->m_flLineCompletionTime != *pfVar1)
  goto LAB_00763450;
  if ((*(float *)(unaff_EBX + 0x36957f) < this->m_flAbuseLevel) && (priority < 4))
  goto LAB_00763450;
  if (line == 9) {
    if (this->field_0x770 != '\0') {
      fVar12 = *(float *)(unaff_EBX + 0x36957f) + this->m_flAbuseLevel;
      this->m_flAbuseLevel = fVar12;
      if (*(float *)(unaff_EBX + 0x369b3f) <= fVar12 && fVar12 != *(float *)(unaff_EBX + 0x369b3f))
      {
        local_258 = *(float *)(unaff_EBX + 0x369883);
        line = 10;
        local_240 = 10;
        goto LAB_007634ad;
      }
    }
    local_258 = 0.0;
    local_240 = 9;
LAB_007634ad:
    if (this->m_iCoreType == CORETYPE_AQUARIUM) goto LAB_00763620;
  }
  else {
    if (line != 2) {
      local_240 = line;
      local_258 = *(float *)(unaff_EBX + 0x369883);
      goto LAB_007634ad;
    }
LAB_00763620:
    local_240 = line;
    local_258 = *(float *)(unaff_EBX + 0x36976f);
  }
  soundname = (string_t *)(this->m_iszLastLinePlayed).pszValue;
  psVar6 = (string_t *)(unaff_EBX + 0x2eb993);
  if (soundname == (string_t *)0x0) {
    soundname = psVar6;
  }
  CBaseEntity::StopSound((CBaseEntity *)this,(char *)soundname);
  if (this->m_iCoreType == CORETYPE_RICHARD) {
    if (line == 0) goto LAB_00763650;
    pCVar11 = (this->m_iLineOrder).m_Memory.m_pMemory + local_240;
    piVar9 = (pCVar11->m_Memory).m_pMemory;
    iVar2 = *piVar9;
    iVar7 = pCVar11->m_Size + -1;
    if (0 < iVar7) {
      _V_memmove(piVar9,piVar9 + 1,iVar7 * 4);
    }
    pCVar11->m_Size = pCVar11->m_Size + -1;
    pCVar11 = (this->m_iLineOrder).m_Memory.m_pMemory + local_240;
    uVar3 = pCVar11->m_Size;
    iVar7 = uVar3 + 1;
    iVar8 = (pCVar11->m_Memory).m_nAllocationCount;
    if (iVar8 < iVar7) {
      iVar4 = (pCVar11->m_Memory).m_nGrowSize;
      if (-1 < iVar4) {
        if (iVar4 == 0) {
          if ((iVar8 == 0) && (iVar8 = 8, iVar7 < 9)) {
            local_244 = 0x20;
          }
          else {
            do {
              local_244 = iVar8;
              iVar8 = local_244 * 2;
            } while (iVar8 < iVar7);
            local_244 = local_244 * 8;
          }
          goto LAB_00763799;
        }
        iVar8 = ((int)uVar3 / iVar4 + 1) * iVar4;
        if (iVar8 < iVar7) {
          if (iVar8 != 0) {
LAB_00763776:
            do {
              iVar8 = (iVar8 + iVar7) / 2;
            } while (iVar8 < iVar7);
            goto LAB_0076378c;
          }
          if (iVar7 < 0) {
            iVar8 = -1;
            local_244 = -4;
          }
          else {
            local_244 = 0;
            if (iVar7 != 0) goto LAB_00763776;
          }
        }
        else {
LAB_0076378c:
          local_244 = iVar8 * 4;
        }
LAB_00763799:
        (pCVar11->m_Memory).m_nAllocationCount = iVar8;
        piVar9 = (pCVar11->m_Memory).m_pMemory;
        if (piVar9 == (int *)0x0) {
          piVar9 = (int *)(*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x4e37cb))
                                    ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x4e37cb),local_244
                                    );
          (pCVar11->m_Memory).m_pMemory = piVar9;
        }
        else {
          piVar9 = (int *)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e37cb) + 4))
                                    ((int *)**(undefined4 **)(unaff_EBX + 0x4e37cb),piVar9,local_244
                                    );
          (pCVar11->m_Memory).m_pMemory = piVar9;
        }
      }
    }
    pCVar11->m_Size = pCVar11->m_Size + 1;
    piVar9 = (pCVar11->m_Memory).m_pMemory;
    pCVar11->m_pElements = piVar9;
    iVar8 = ~uVar3 + pCVar11->m_Size;
    if (0 < iVar8) {
      _V_memmove(piVar9 + iVar7,piVar9 + uVar3,iVar8 * 4);
    }
    piVar9 = (pCVar11->m_Memory).m_pMemory + uVar3;
    if (piVar9 != (int *)0x0) {
      *piVar9 = iVar2;
    }
    pcVar10 = (this->m_iszVoiceName).pszValue;
    if (pcVar10 == (char *)0x0) {
      pcVar10 = (char *)(unaff_EBX + 0x2eb993);
    }
    soundname = (string_t *)(unaff_EBX + 0x3520a5);
    CFmtStrN<256>::CFmtStrN
              ((CFmtStrN<256> *)local_12c,(char *)soundname,pcVar10,
               *(undefined4 *)(unaff_EBX + 0x606f2f + line * 4),iVar2);
    psVar5 = (string_t *)(local_12c + 5);
  }
  else if (line == 0) {
LAB_00763650:
    psVar5 = (string_t *)(this->m_iszInfoTargetScriptName).pszValue;
    if (psVar5 == (string_t *)0x0) {
      psVar5 = psVar6;
    }
  }
  else {
    psVar5 = (string_t *)(this->m_iszVoiceName).pszValue;
    if (psVar5 == (string_t *)0x0) {
      psVar5 = psVar6;
    }
    soundname = (string_t *)(unaff_EBX + 0x3520af);
    CFmtStrN<256>::CFmtStrN
              ((CFmtStrN<256> *)local_238,(char *)soundname,psVar5,
               *(undefined4 *)(unaff_EBX + 0x606f2f + line * 4));
    psVar5 = (string_t *)(local_238 + 5);
  }
  psVar6 = AllocPooledString(psVar5,(char *)soundname);
  (this->m_iszLastLinePlayed).pszValue = (char *)psVar6;
  pcVar10 = (this->m_iszLastLinePlayed).pszValue;
  if (pcVar10 == (char *)0x0) {
    pcVar10 = (char *)(unaff_EBX + 0x2eb993);
  }
  CBaseEntity::EmitSound((CBaseEntity *)this,pcVar10,0.0,(float *)0x0);
  this->m_iCurrentLine = line;
  this->m_iCurrentPriority = priority;
  this->m_iPendingPriority = -1;
  this->m_iPendingLine = -1;
  pcVar10 = (this->m_iszLastLinePlayed).pszValue;
  if (pcVar10 == (char *)0x0) {
    pcVar10 = (char *)(unaff_EBX + 0x2eb993);
  }
  fVar13 = CBaseEntity::GetSoundDuration(pcVar10,&UNK_0034b9d3 + unaff_EBX);
  piVar9 = *(int **)(unaff_EBX + 0x4e37d3);
  fVar12 = fVar13 + *(float *)(*piVar9 + 0xc);
  this->m_flFirstNoSkipTime = fVar12;
  this->m_flLineCompletionTime =
       *(float *)(*piVar9 + 0xc) + this->m_flBetweenVOPadding + fVar13 + local_258;
  if (local_240 - 9U < 2) {
    this->m_flLineCompletionTime = fVar12;
  }
LAB_00763450:
  if (local_20 != **(int **)(unaff_EBX + 0x4e381f)) {
                    /* WARNING: Subroutine does not return */
    ___stack_chk_fail();
  }
  return;
}


/* CPropPersonalitySphere::AnimateThink at 00763280 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: void AnimateThink(CPropPersonalitySphere * this) */

void __thiscall CPropPersonalitySphere::AnimateThink(CPropPersonalitySphere *this)

{
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x31c))(this);
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x11);
  CBaseEntity::ThinkSet
            ((CBaseEntity *)this,func,
             *(float *)(unaff_EBX + 0x369683) + *(float *)(**(int **)(unaff_EBX + 0x4e38d3) + 0xc),
             *(char **)(unaff_EBX + 0x5738c3));
  return;
}


/* CPropPersonalitySphere::SetupVOList at 00765b20 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: void SetupVOList(CPropPersonalitySphere * this) */

void __thiscall CPropPersonalitySphere::SetupVOList(CPropPersonalitySphere *this)

{
  uint *puVar1;
  int iVar2;
  ushort uVar3;
  CBaseEdict *this_00;
  ushort *puVar4;
  int *piVar5;
  string_t *psVar6;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  uint uVar9;
  int unaff_EBX;
  char *in_stack_ffffff18;
  uint local_c0;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_iCoreType < CORETYPE_NONE) {
                    /* WARNING: Could not recover jumptable at 0x00765bbf. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*(code *)(*(int *)(unaff_EBX + 0x8d + this->m_iCoreType * 4) + unaff_EBX))();
    return;
  }
  psVar6 = AllocPooledString((string_t *)(&UNK_00349264 + unaff_EBX),in_stack_ffffff18);
  (this->m_iszLookAnimationName).pszValue = (char *)psVar6;
  if ((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.m_nSkin.
      m_Value != 0) {
    if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
        false) {
      this_00 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                  super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4e1031);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar4) {
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00766587:
            puVar4[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x3e0;
            puVar4[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar8 * 0x14 + 1] != 0x3e0) {
            local_c0 = 0;
            do {
              uVar9 = local_c0 + 1;
              local_c0 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 != 0x13) goto LAB_00766587;
                pIVar7->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar8 * 0x14 + local_c0 + 1] != 0x3e0);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar7->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4e1031) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4e1031) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x4e1031);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar2 = *piVar5 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3e0;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = (uint *)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
      *puVar1 = *puVar1 | 1;
    }
    (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.m_nSkin.
    m_Value = 0;
  }
  psVar6 = AllocPooledString((string_t *)(CAI_LeadGoal::~CAI_LeadGoal + unaff_EBX),in_stack_ffffff18
                            );
  (this->m_iszDeathSoundScriptName).pszValue = (char *)psVar6;
  return;
}


/* CPropPersonalitySphere::OnPhysGunDrop at 00763940 */

/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void OnPhysGunDrop(CPropPersonalitySphere * this, CBasePlayer *
   pPhysGunUser, PhysGunDrop_t reason) */

void __thiscall
CPropPersonalitySphere::OnPhysGunDrop
          (CPropPersonalitySphere *this,CBasePlayer *pPhysGunUser,PhysGunDrop_t reason)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  TrySpeakLine(this,4,4);
  this->m_bHeld = false;
  this->m_flLastHeldTime = *(float *)(**(int **)(unaff_EBX + 0x4e3210) + 0xc);
  CPhysicsProp::OnPhysGunDrop(&this->super_CPhysicsProp,pPhysGunUser,reason);
  return;
}


/* CPropPersonalitySphere::VPhysicsCollision at 00763d70 */

/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: void VPhysicsCollision(CPropPersonalitySphere * this, int index,
   gamevcollisionevent_t * pEvent) */

void __thiscall
CPropPersonalitySphere::VPhysicsCollision
          (CPropPersonalitySphere *this,int index,gamevcollisionevent_t *pEvent)

{
  float fVar1;
  float fVar2;
  int *piVar3;
  char cVar4;
  int unaff_EBX;
  float fVar5;
  
                    /* Unresolved local var: CBaseEntity * pHitEntity@[???] */
  ___i686_get_pc_thunk_bx();
  CPhysicsProp::VPhysicsCollision(&this->super_CPhysicsProp,index,pEvent);
  piVar3 = *(int **)((int)pEvent->pEntities + (-(uint)(index == 0) & 4));
  if (piVar3 == (int *)**(int **)(unaff_EBX + 0x4e2e9c)) {
    if (this->m_bHeld != false) {
      TrySpeakLine(this,9,6);
      return;
    }
    fVar5 = pEvent->preVelocity[index].z;
    fVar1 = pEvent->preVelocity[index].y;
    fVar2 = pEvent->preVelocity[index].x;
                    /* Unresolved local var: Vector vecVelocity@[???]
                       Unresolved local var: __m128 root@[???] */
    fVar5 = SQRT(fVar2 * fVar2 + fVar1 * fVar1 + fVar5 * fVar5);
    if (fVar5 < *(float *)(unaff_EBX + 0x368e94) || fVar5 == *(float *)(unaff_EBX + 0x368e94)) {
      return;
    }
  }
  else {
    cVar4 = (**(code **)(*piVar3 + 0x154))(piVar3);
    if (cVar4 != '\0') {
      return;
    }
  }
  TrySpeakLine(this,9,6);
  return;
}


/* CPropPersonalitySphere::NotifySystemEvent at 00763260 */

/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: void NotifySystemEvent(CPropPersonalitySphere * this, CBaseEntity *
   pNotify, notify_system_event_t eventType, notify_system_event_params_t * params) */

void __thiscall
CPropPersonalitySphere::NotifySystemEvent
          (CPropPersonalitySphere *this,CBaseEntity *pNotify,notify_system_event_t eventType,
          notify_system_event_params_t *params)

{
  if (eventType == NOTIFY_EVENT_TELEPORT) {
    (this->m_pastPositions).m_Size = 0;
  }
  return;
}


/* _GLOBAL__I_g_LineNames at 000b8380 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_g_LineNames(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

