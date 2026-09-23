/* DWARF-guided pseudocode for game/server/portal2/prop_personality_sphere.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* CInfoTargetPersonalitySphere::~CInfoTargetPersonalitySphere at 006efc20 */

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
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x54d160);
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(&DAT_004bc0a8 + extraout_ECX) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,__in_chrg);
  return;
}


/* CEntityFactory<CInfoTargetPersonalitySphere>::Create at 006efdf0 */

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
       (_func_int_varargs **)(unaff_EBX + 0x54cf84);
  CBaseEntity::PostConstructor(this_00,pClassName);
  return &(this_00->m_Network).super_IServerNetworkable;
}


/* CInfoTargetPersonalitySphere::~CInfoTargetPersonalitySphere at 006f0030 */

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
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x54cd47);
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(&DAT_004bbc8f + unaff_EBX) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CInfoTargetPersonalitySphere::Spawn at 006efc00 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CInfoTargetPersonalitySphere * this) */

void __thiscall CInfoTargetPersonalitySphere::Spawn(CInfoTargetPersonalitySphere *this)

{
  (*(this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1a])(this);
  CPointEntity::Spawn(&this->super_CPointEntity);
  return;
}


/* CInfoTargetPersonalitySphere::Precache at 006efbd0 */

/* DWARF original prototype: void Precache(CInfoTargetPersonalitySphere * this) */

void __thiscall CInfoTargetPersonalitySphere::Precache(CInfoTargetPersonalitySphere *this)

{
  char *soundname;
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  soundname = (this->m_strSphereLine).pszValue;
  if (soundname == (char *)0x0) {
    soundname = (char *)(extraout_ECX + 0x2bbf84);
  }
  CBaseEntity::PrecacheScriptSound(soundname);
  return;
}


/* DataMapInit<CInfoTargetPersonalitySphere> at 000ccef0 */

datamap_t * DataMapInit<CInfoTargetPersonalitySphere>(CInfoTargetPersonalitySphere *param_1)

{
  char *pcVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (speechListSounds[3].name[unaff_EBX + 0xf7] == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcee547);
    if (iVar2 != 0) {
      *(int *)(speechListSounds[3].name + unaff_EBX + 0xff) = unaff_EBX + 0x946246;
      pcVar1 = speechListSounds[3].name + unaff_EBX + 0x107;
      pcVar1[0] = '\0';
      pcVar1[1] = '\0';
      pcVar1[2] = '\0';
      pcVar1[3] = '\0';
      pcVar1 = speechListSounds[3].name + unaff_EBX + 0x10b;
      pcVar1[0] = '\0';
      pcVar1[1] = '\0';
      pcVar1[2] = '\0';
      pcVar1[3] = '\0';
      pcVar1 = speechListSounds[3].name + unaff_EBX + 0x10f;
      pcVar1[0] = '\0';
      pcVar1[1] = '\0';
      pcVar1[2] = '\0';
      pcVar1[3] = '\0';
      pcVar1 = speechListSounds[3].name + unaff_EBX + 0x113;
      pcVar1[0] = '\0';
      pcVar1[1] = '\0';
      pcVar1[2] = '\0';
      pcVar1[3] = '\0';
      pcVar1 = speechListSounds[3].name + unaff_EBX + 0x117;
      pcVar1[0] = '\0';
      pcVar1[1] = '\0';
      pcVar1[2] = '\0';
      pcVar1[3] = '\0';
      pcVar1 = speechListSounds[3].name + unaff_EBX + 0x103;
      pcVar1[0] = '\x1c';
      pcVar1[1] = '\0';
      pcVar1[2] = '\0';
      pcVar1[3] = '\0';
      ___cxa_guard_release(unaff_EBX + 0xcee547);
      ___cxa_atexit(unaff_EBX + 0x8cffdf,0,*(undefined4 *)(&DAT_00adec8f + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_Touch_00c030a4 + unaff_EBX + 3) =
       *(undefined4 *)(&DAT_00adedcb + unaff_EBX);
  *(undefined4 *)((int)&PTR_Use_00c0309c + unaff_EBX + 3) = 2;
  *(int *)((int)&PTR_UpdatePaintPowersFromContacts_00c03098 + unaff_EBX + 3) = unaff_EBX + 0xc0371f;
  return (datamap_t *)((int)&PTR_UpdatePaintPowersFromContacts_00c03098 + unaff_EBX + 3);
}


/* __static_initialization_and_destruction_0 at 000cd300 */

/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *pcVar1;
  IEntityFactoryDictionary *pIVar2;
  datamap_t *pdVar3;
  int unaff_EBX;
  longlong lVar4;
  
  lVar4 = ___i686_get_pc_thunk_bx();
  if (lVar4 != 0xffff00000001) {
    return;
  }
  speechListSounds[1].name[unaff_EBX + 0x15c] = '\0';
  speechListSounds[1].name[unaff_EBX + 0x15d] = '\0';
  speechListSounds[1].name[unaff_EBX + 0x15e] = '\0';
  speechListSounds[1].name[unaff_EBX + 0x15f] = '\0';
  pcVar1 = speechListSounds[1].name + unaff_EBX + 0x160;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = speechListSounds[1].name + unaff_EBX + 0x164;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = speechListSounds[1].name + unaff_EBX + 0x168;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = speechListSounds[1].name + unaff_EBX + 0x16c;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = speechListSounds[1].name + unaff_EBX + 0x170;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = speechListSounds[1].name + unaff_EBX + 0x174;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = speechListSounds[1].name + unaff_EBX + 0x178;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = speechListSounds[1].name + unaff_EBX + 0x17c;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = speechListSounds[1].sceneName + unaff_EBX + -0x80;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = speechListSounds[1].sceneName + unaff_EBX + -0x7c;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = speechListSounds[1].sceneName + unaff_EBX + -0x78;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = speechListSounds[1].sceneName + unaff_EBX + -0x74;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = speechListSounds[1].sceneName + unaff_EBX + -0x70;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  speechListSounds[1].sceneName[unaff_EBX + -0x69] = '\0';
  speechListSounds[1].sceneName[unaff_EBX + -0x6c] = '\0';
  speechListSounds[1].sceneName[unaff_EBX + -0x6b] = '\0';
  speechListSounds[1].sceneName[unaff_EBX + -0x6a] = '\0';
  pcVar1 = speechListSounds[1].sceneName + unaff_EBX + -0x68;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  speechListSounds[1].sceneName[unaff_EBX + -0x61] = '\0';
  speechListSounds[1].sceneName[unaff_EBX + -100] = '\0';
  speechListSounds[1].sceneName[unaff_EBX + -99] = '\0';
  speechListSounds[1].sceneName[unaff_EBX + -0x62] = '\0';
  pcVar1 = speechListSounds[1].sceneName + unaff_EBX + -0x60;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  speechListSounds[1].sceneName[unaff_EBX + -0x59] = '\x01';
  speechListSounds[1].sceneName[unaff_EBX + -0x5c] = '\0';
  speechListSounds[1].sceneName[unaff_EBX + -0x5b] = '\0';
  speechListSounds[1].sceneName[unaff_EBX + -0x5a] = '\0';
  pcVar1 = speechListSounds[1].sceneName + unaff_EBX + -0x58;
  pcVar1[0] = '\x01';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  speechListSounds[1].sceneName[unaff_EBX + -0x51] = '\0';
  speechListSounds[1].sceneName[unaff_EBX + -0x54] = '\0';
  speechListSounds[1].sceneName[unaff_EBX + -0x53] = '\0';
  speechListSounds[1].sceneName[unaff_EBX + -0x52] = '\0';
  pcVar1 = speechListSounds[1].sceneName + unaff_EBX + -0x50;
  pcVar1[0] = '\x02';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  speechListSounds[1].sceneName[unaff_EBX + -0x49] = '\0';
  speechListSounds[1].sceneName[unaff_EBX + -0x4c] = '\0';
  speechListSounds[1].sceneName[unaff_EBX + -0x4b] = '\0';
  speechListSounds[1].sceneName[unaff_EBX + -0x4a] = '\0';
  *(undefined **)(speechListSounds[1].sceneName + unaff_EBX + -0x48) = &UNK_00ae401c + unaff_EBX;
  *(undefined **)(speechListSounds[1].sceneName + unaff_EBX + -0x44) = &UNK_00b6fdcc + unaff_EBX;
  pIVar2 = EntityFactoryDictionary();
  (**pIVar2->_vptr_IEntityFactoryDictionary)(pIVar2,unaff_EBX + 0xcee104,unaff_EBX + 0x945e1c);
  pdVar3 = DataMapInit<CInfoTargetPersonalitySphere>((CInfoTargetPersonalitySphere *)0x0);
  *(datamap_t **)((int)&PTR_SetCursor_00c63208 + unaff_EBX) = pdVar3;
  *(undefined **)(speechListSounds[1].sceneName + unaff_EBX + -0x40) = &UNK_00b6fdec + unaff_EBX;
  pIVar2 = EntityFactoryDictionary();
  (**pIVar2->_vptr_IEntityFactoryDictionary)(pIVar2,unaff_EBX + 0xcee108,unaff_EBX + 0x945e6f);
  pdVar3 = DataMapInit<CPropPersonalitySphere>((CPropPersonalitySphere *)0x0);
  *(datamap_t **)((int)&PTR_GetCursor_00c6320c + unaff_EBX) = pdVar3;
  return;
}


/* CInfoTargetPersonalitySphere::GetDataDescMap at 006ecc10 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CInfoTargetPersonalitySphere * this) */

datamap_t * __thiscall
CInfoTargetPersonalitySphere::GetDataDescMap(CInfoTargetPersonalitySphere *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5e3384);
}


/* CInfoTargetPersonalitySphere::GetBaseMap at 006ecc20 */

datamap_t * CInfoTargetPersonalitySphere::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(CAI_ActBusyBehavior::GetDataDescMap + extraout_ECX + 4);
}


/* __tcf_0 at 0099cee0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41e576)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41e56a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41e576));
  }
  *(undefined4 *)(unaff_EBX + 0x41e576) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41e572)) {
    if (*(int *)(unaff_EBX + 0x41e56a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20ecaa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20ecaa),*(int *)(unaff_EBX + 0x41e56a));
      *(undefined4 *)(unaff_EBX + 0x41e56a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e56e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41e56a);
  *(int *)(unaff_EBX + 0x41e57a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41e572)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20ecaa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20ecaa),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41e56a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e56e) = 0;
  }
  return;
}


/* CInfoTargetEntityEnumerator::EnumEntity at 006efe60 */

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
       (iVar6 = (uVar1 & 0xffff) * 0x10 + *(int *)(&DAT_004bbdf2 + unaff_EBX),
       *(uint *)(iVar6 + 8) == uVar1 >> 0x10)) &&
      (piVar2 = *(int **)(iVar6 + 4), piVar2 != (int *)0x0)) &&
     (this_00 = (CBaseEntity *)(**(code **)(*piVar2 + 0x18))(piVar2), this_00 != (CBaseEntity *)0x0)
     ) {
    local_20 = &UNK_003232ba + unaff_EBX;
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


/* CPropPersonalitySphere::CheckForInfoTargets at 006ee710 */

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
  if (((*(int *)(**(int **)(unaff_EBX + 0x4bd479) + 0x14) < 2) &&
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
    local_6c = *(float *)(unaff_EBX + 0x33c2a9);
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
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4bd4e5) + 0x14))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4bd4e5),&local_fc,0x400b,&local_a0,&local_150)
    ;
    if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x4bd4e1) + 0x1c) + 0x30) != 0) {
      DebugDrawLine(&local_150,local_144,0xff,0,0,true,-1.0);
    }
    fVar12 = *(float *)(unaff_EBX + 0x33c25d);
    fVar13 = *(float *)(unaff_EBX + 0x33bef1);
                    /* Unresolved local var: CFlaggedEntitiesEnum sphereEnum@[???] */
    CFlaggedEntitiesEnum::CFlaggedEntitiesEnum(&local_a0,local_550,0x100,0);
    iVar4 = UTIL_EntitiesInSphere(local_144,fVar12 * local_124 + fVar13,&local_a0);
    if (0 < iVar4) {
      iVar8 = 0;
      local_564 = *(float *)(unaff_EBX + 0x33bbb9);
      local_560 = (CBaseEntity *)0x0;
      pszClassOrWildcard = (char *)(unaff_EBX + 0x324a01);
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
               (iVar6 = (uVar11 & 0xffff) * 0x10 + *(int *)(unaff_EBX + 0x4bd539),
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


/* CPropPersonalitySphere::PreferredCarryAngles at 006efb00 */

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


/* CPropPersonalitySphere::HasPreferredCarryAnglesForPlayer at 006efb40 */

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


/* CPropPersonalitySphere::ObjectCaps at 006efc50 */

/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: int ObjectCaps(CPropPersonalitySphere * this) */

int __thiscall CPropPersonalitySphere::ObjectCaps(CPropPersonalitySphere *this)

{
  uint uVar1;
  
  uVar1 = CPhysicsProp::ObjectCaps(&this->super_CPhysicsProp);
  return uVar1 | 1;
}


/* DataMapInit<CPropPersonalitySphere> at 000ccfc0 */

/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */

datamap_t * DataMapInit<CPropPersonalitySphere>(CPropPersonalitySphere *param_1)

{
  uint uVar1;
  int iVar2;
  char *pcVar3;
  size_t sVar4;
  undefined4 *puVar5;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((speechListSounds[2].sceneName[unaff_EBX + 0x7e] == '\0') &&
     (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcee44a), iVar2 != 0)) {
    *(int *)(speechListSounds[3].name + unaff_EBX + 10) = unaff_EBX + 0x946196;
    pcVar3 = speechListSounds[3].name + unaff_EBX + 0x12;
    pcVar3[0] = '\0';
    pcVar3[1] = '\0';
    pcVar3[2] = '\0';
    pcVar3[3] = '\0';
    pcVar3 = speechListSounds[3].name + unaff_EBX + 0x16;
    pcVar3[0] = '\0';
    pcVar3[1] = '\0';
    pcVar3[2] = '\0';
    pcVar3[3] = '\0';
    pcVar3 = speechListSounds[3].name + unaff_EBX + 0x1a;
    pcVar3[0] = '\0';
    pcVar3[1] = '\0';
    pcVar3[2] = '\0';
    pcVar3[3] = '\0';
    pcVar3 = speechListSounds[3].name + unaff_EBX + 0x1e;
    pcVar3[0] = '\0';
    pcVar3[1] = '\0';
    pcVar3[2] = '\0';
    pcVar3[3] = '\0';
    pcVar3 = speechListSounds[3].name + unaff_EBX + 0x22;
    pcVar3[0] = '\0';
    pcVar3[1] = '\0';
    pcVar3[2] = '\0';
    pcVar3[3] = '\0';
    pcVar3 = speechListSounds[3].name + unaff_EBX + 0xe;
    pcVar3[0] = '\x16';
    pcVar3[1] = '\0';
    pcVar3[2] = '\0';
    pcVar3[3] = '\0';
    ___cxa_guard_release(unaff_EBX + 0xcee44a);
    ___cxa_atexit(unaff_EBX + 0x8cfff2,0,*(undefined4 *)(&DAT_00adebc2 + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_Classify_00c02ff0 + unaff_EBX + 2) =
       *(undefined4 *)(&DAT_00adf5e2 + unaff_EBX);
  if ((speechListSounds[3].name[unaff_EBX + 2] == '\0') &&
     (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcee452), iVar2 != 0)) {
    if ((*(char *)((int)&PTR_GetStepOrigin_00c20728 + unaff_EBX + 2) == '\0') &&
       (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xc2072a), iVar2 != 0)) {
      *(undefined **)((int)&PTR_NetworkStateChanged_m_fFlags_00c20734 + unaff_EBX + 2) =
           &UNK_00b70192 + unaff_EBX;
      ___cxa_guard_release(unaff_EBX + 0xc2072a);
    }
    *(int *)((int)&PTR_Event_KilledOther_00c033a4 + unaff_EBX + 2) = unaff_EBX + 0xc20736;
    pcVar3 = operator_new___(*(int *)(speechListSounds[3].name + unaff_EBX + 0xe) + 0xd);
    _strcpy(pcVar3,*(char **)(speechListSounds[3].name + unaff_EBX + 10));
    sVar4 = _strlen(pcVar3);
    builtin_strncpy(pcVar3 + sVar4,"TalkingThink",0xd);
    uVar1 = *(uint *)(speechListSounds[3].name + unaff_EBX + 0x1e);
    CUtlVector<char*,CUtlMemory<char*,_int>_>::GrowVector
              ((CUtlVector<char*,CUtlMemory<char*,_int>_> *)
               (speechListSounds[3].name + unaff_EBX + 0x12),1);
    iVar2 = ~uVar1 + *(int *)(speechListSounds[3].name + unaff_EBX + 0x1e);
    if (0 < iVar2) {
      _V_memmove((void *)(*(int *)(speechListSounds[3].name + unaff_EBX + 0x12) + 4 + uVar1 * 4),
                 (void *)(*(int *)(speechListSounds[3].name + unaff_EBX + 0x12) + uVar1 * 4),
                 iVar2 * 4);
    }
    puVar5 = (undefined4 *)(uVar1 * 4 + *(int *)(speechListSounds[3].name + unaff_EBX + 0x12));
    if (puVar5 != (undefined4 *)0x0) {
      *puVar5 = pcVar3;
    }
    *(char **)((int)&PTR_RunVScripts_00c03594 + unaff_EBX + 2) = pcVar3;
    pcVar3 = operator_new___(*(int *)(speechListSounds[3].name + unaff_EBX + 0xe) + 0xd);
    _strcpy(pcVar3,*(char **)(speechListSounds[3].name + unaff_EBX + 10));
    sVar4 = _strlen(pcVar3);
    builtin_strncpy(pcVar3 + sVar4,"AnimateThink",0xd);
    uVar1 = *(uint *)(speechListSounds[3].name + unaff_EBX + 0x1e);
    CUtlVector<char*,CUtlMemory<char*,_int>_>::GrowVector
              ((CUtlVector<char*,CUtlMemory<char*,_int>_> *)
               (speechListSounds[3].name + unaff_EBX + 0x12),1);
    iVar2 = ~uVar1 + *(int *)(speechListSounds[3].name + unaff_EBX + 0x1e);
    if (0 < iVar2) {
      _V_memmove((void *)(*(int *)(speechListSounds[3].name + unaff_EBX + 0x12) + 4 + uVar1 * 4),
                 (void *)(*(int *)(speechListSounds[3].name + unaff_EBX + 0x12) + uVar1 * 4),
                 iVar2 * 4);
    }
    puVar5 = (undefined4 *)(uVar1 * 4 + *(int *)(speechListSounds[3].name + unaff_EBX + 0x12));
    if (puVar5 != (undefined4 *)0x0) {
      *puVar5 = pcVar3;
    }
    *(char **)(&DAT_00c035d6 + unaff_EBX) = pcVar3;
    ___cxa_guard_release(unaff_EBX + 0xcee452);
  }
  *(undefined4 *)((int)&PTR_GetResponseSystem_00c02fe8 + unaff_EBX + 2) = 0x17;
  *(int *)((int)&PTR_GetBaseAnimatingOverlay_00c02fe4 + unaff_EBX + 2) = unaff_EBX + 0xc03052;
  return (datamap_t *)((int)&PTR_GetBaseAnimatingOverlay_00c02fe4 + unaff_EBX + 2);
}


/* CPropPersonalitySphere::GetDataDescMap at 006ecc30 */

/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: datamap_t * GetDataDescMap(CPropPersonalitySphere * this) */

datamap_t * __thiscall CPropPersonalitySphere::GetDataDescMap(CPropPersonalitySphere *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5e337c);
}


/* CPropPersonalitySphere::GetBaseMap at 006ecc40 */

datamap_t * CPropPersonalitySphere::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4bf968);
}


/* __tcf_1 at 0099cfc0 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41e46e)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41e462) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41e46e));
  }
  *(undefined4 *)(unaff_EBX + 0x41e46e) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41e46a)) {
    if (*(int *)(unaff_EBX + 0x41e462) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20ebca) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20ebca),*(int *)(unaff_EBX + 0x41e462));
      *(undefined4 *)(unaff_EBX + 0x41e462) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e466) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41e462);
  *(int *)(unaff_EBX + 0x41e472) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41e46a)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20ebca) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20ebca),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41e462) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e466) = 0;
  }
  return;
}


/* CPropPersonalitySphere::CPropPersonalitySphere at 006eee80 */

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
       (IServerUnknown)(unaff_EBX + 0x54d9bb);
  (this->super_CPhysicsProp).super_CBreakableProp.super_IBreakableWithPropData.
  _vptr_IBreakableWithPropData = (_func_int_varargs **)(unaff_EBX + 0x54de1f);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x54dea3);
  (this->super_CPhysicsProp).super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(unaff_EBX + 0x54ded3);
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


/* CPropPersonalitySphere::CPropPersonalitySphere at 006eefa0 */

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


/* CEntityFactory<CPropPersonalitySphere>::Create at 006f01d0 */

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


/* CPropPersonalitySphere::~CPropPersonalitySphere at 006eefb0 */

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
       (IServerUnknown)(unaff_EBX + 0x54d88a);
  (this->super_CPhysicsProp).super_CBreakableProp.super_IBreakableWithPropData.
  _vptr_IBreakableWithPropData = (_func_int_varargs **)(unaff_EBX + 0x54dcee);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x54dd72);
  (this->super_CPhysicsProp).super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(unaff_EBX + 0x54dda2);
  local_20 = (this->m_iLineOrder).m_Size;
  puVar1 = *(undefined4 **)(unaff_EBX + 0x4bcbda);
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
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4bcbda) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x4bcbda));
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
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4bcbda) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4bcbda));
      (this->m_iLineOrder).m_Memory.m_pMemory = (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0;
      pCVar3 = (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0;
    }
    (this->m_iLineOrder).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_iLineOrder).m_pElements = pCVar3;
  if (-1 < (this->m_iLineOrder).m_Memory.m_nGrowSize) {
    if (pCVar3 != (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4bcbda) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4bcbda));
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
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4bcbda) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4bcbda));
      pCVar3 = (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0;
      (this->m_pastPositions).m_Memory.m_pMemory = (Vector *)0x0;
    }
    (this->m_pastPositions).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_pastPositions).m_pElements = (Vector *)pCVar3;
  if (-1 < (this->m_pastPositions).m_Memory.m_nGrowSize) {
    if (pCVar3 != (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4bcbda) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4bcbda));
      (this->m_pastPositions).m_Memory.m_pMemory = (Vector *)0x0;
      in_stack_ffffffc8 = pCVar3;
    }
    (this->m_pastPositions).m_Memory.m_nAllocationCount = 0;
  }
  CPhysicsProp::~CPhysicsProp(&this->super_CPhysicsProp,(int)in_stack_ffffffc8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CPropPersonalitySphere::~CPropPersonalitySphere at 006ef370 */

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
       (IServerUnknown)(unaff_EBX + 0x54d4ca);
  (this->super_CPhysicsProp).super_CBreakableProp.super_IBreakableWithPropData.
  _vptr_IBreakableWithPropData = (_func_int_varargs **)(unaff_EBX + 0x54d92e);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x54d9b2);
  (this->super_CPhysicsProp).super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(&UNK_0054d9e2 + unaff_EBX);
  local_20 = (this->m_iLineOrder).m_Size;
  puVar1 = *(undefined4 **)(unaff_EBX + 0x4bc81a);
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
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4bc81a) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x4bc81a),iVar3);
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
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4bc81a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4bc81a),pCVar4);
      (this->m_iLineOrder).m_Memory.m_pMemory = (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0;
    }
    pCVar4 = (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0;
    (this->m_iLineOrder).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_iLineOrder).m_pElements = pCVar4;
  if (-1 < (this->m_iLineOrder).m_Memory.m_nGrowSize) {
    if (pCVar4 != (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4bc81a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4bc81a),pCVar4);
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
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4bc81a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4bc81a),pVVar5);
      (this->m_pastPositions).m_Memory.m_pMemory = (Vector *)0x0;
    }
    pVVar5 = (Vector *)0x0;
    (this->m_pastPositions).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_pastPositions).m_pElements = pVVar5;
  if (-1 < (this->m_pastPositions).m_Memory.m_nGrowSize) {
    if (pVVar5 != (Vector *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4bc81a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4bc81a),pVVar5);
      (this->m_pastPositions).m_Memory.m_pMemory = (Vector *)0x0;
    }
    (this->m_pastPositions).m_Memory.m_nAllocationCount = 0;
  }
  CPhysicsProp::~CPhysicsProp(&this->super_CPhysicsProp,__in_chrg);
  return;
}


/* CPropPersonalitySphere::~CPropPersonalitySphere at 006ef730 */

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
       (IServerUnknown)(unaff_EBX + 0x54d10a);
  (this->super_CPhysicsProp).super_CBreakableProp.super_IBreakableWithPropData.
  _vptr_IBreakableWithPropData = (_func_int_varargs **)(unaff_EBX + 0x54d56e);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x54d5f2);
  (this->super_CPhysicsProp).super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(unaff_EBX + 0x54d622);
  local_20 = (this->m_iLineOrder).m_Size;
  puVar1 = *(undefined4 **)(unaff_EBX + 0x4bc45a);
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
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4bc45a) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x4bc45a),iVar3);
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
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4bc45a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4bc45a),pCVar4);
      (this->m_iLineOrder).m_Memory.m_pMemory = (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0;
    }
    pCVar4 = (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0;
    (this->m_iLineOrder).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_iLineOrder).m_pElements = pCVar4;
  if (-1 < (this->m_iLineOrder).m_Memory.m_nGrowSize) {
    if (pCVar4 != (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4bc45a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4bc45a),pCVar4);
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
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4bc45a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4bc45a),pVVar5);
      (this->m_pastPositions).m_Memory.m_pMemory = (Vector *)0x0;
    }
    pVVar5 = (Vector *)0x0;
    (this->m_pastPositions).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_pastPositions).m_pElements = pVVar5;
  if (-1 < (this->m_pastPositions).m_Memory.m_nGrowSize) {
    if (pVVar5 != (Vector *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4bc45a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4bc45a),pVVar5);
      (this->m_pastPositions).m_Memory.m_pMemory = (Vector *)0x0;
    }
    (this->m_pastPositions).m_Memory.m_nAllocationCount = 0;
  }
  CPhysicsProp::~CPhysicsProp(&this->super_CPhysicsProp,__in_chrg);
  return;
}


/* CPropPersonalitySphere::Spawn at 006edd20 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: void Spawn(CPropPersonalitySphere * this) */

void __thiscall CPropPersonalitySphere::Spawn(CPropPersonalitySphere *this)

{
  uint *puVar1;
  int *piVar2;
  CBaseEdict *this_00;
  BASEPTR func;
  BASEPTR func_00;
  int iVar3;
  IChangeInfoAccessor *pIVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x80))(this,&UNK_0031757a + unaff_EBX,unaff_EBX + 0x31f3ca);
  CPhysicsProp::Spawn(&this->super_CPhysicsProp);
  iVar3 = CBaseAnimating::LookupSequence((CBaseAnimating *)this,(char *)(unaff_EBX + 0x31f400));
  CBaseAnimating::ResetSequence((CBaseAnimating *)this,iVar3);
  if (*(float *)(unaff_EBX + 0x33c5e2) !=
      (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.m_flCycle
      .m_Value) {
    if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
        false) {
      this_00 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                  super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar4 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar4->m_iChangeInfoSerialNumber = 0;
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
  iVar3 = CBaseAnimating::LookupAttachment((CBaseAnimating *)this,(char *)(unaff_EBX + 0x325468));
  this->m_iEyeballAttachment = iVar3;
  piVar2 = *(int **)(unaff_EBX + 0x4bde72);
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x10ae);
  CBaseEntity::ThinkSet
            ((CBaseEntity *)this,func,*(float *)(unaff_EBX + 0x33c5e6) + *(float *)(*piVar2 + 0xc),
             *(char **)(unaff_EBX + 0x54f386));
  if (this->m_iCoreType == CORETYPE_AQUARIUM) {
    this->m_flLastHeldTime = *(float *)(*piVar2 + 0xc);
    func_00.__delta = 0;
    func_00.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0xfa2);
    CBaseEntity::ThinkSet((CBaseEntity *)this,func_00,0.0,(char *)0x0);
    CBaseEntity::SetNextThink
              ((CBaseEntity *)this,*(float *)(*piVar2 + 0xc) + this->m_flBetweenVOPadding + 0.0,
               (char *)0x0);
    return;
  }
  return;
}


/* CPropPersonalitySphere::RegisterSoundEvent at 006ed4c0 */

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
  local_20 = **(int **)(unaff_EBX + 0x4be71b);
  (&(this->super_CPhysicsProp).field_0x766)[eventName] = 1;
  if (eventName != SPHERE_EVENT_INFO_TARGET) {
    if (this->m_iCoreType == CORETYPE_RICHARD) {
                    /* Unresolved local var: int i@[???] */
      if (0 < count) {
        local_240 = 1;
        local_244 = 0;
        puVar1 = *(undefined4 **)(unaff_EBX + 0x4be6c7);
        do {
          pcVar4 = (this->m_iszVoiceName).pszValue;
          if (pcVar4 == (char *)0x0) {
            pcVar4 = &UNK_002be68b + unaff_EBX;
          }
          CFmtStrN<256>::CFmtStrN
                    (&local_12c,(char *)(unaff_EBX + 0x325c45),pcVar4,
                     *(undefined4 *)(unaff_EBX + 0x5e2a8f + eventName * 4),local_240);
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
                goto LAB_006ed6d3;
              }
              iVar5 = ((int)uVar2 / iVar3 + 1) * iVar3;
              if (iVar5 < iVar7) {
                if (iVar5 != 0) {
LAB_006ed6b0:
                  do {
                    iVar5 = (iVar5 + iVar7) / 2;
                  } while (iVar5 < iVar7);
                  goto LAB_006ed6c6;
                }
                if (iVar7 < 0) {
                  iVar5 = -1;
                  local_248 = -4;
                }
                else {
                  local_248 = 0;
                  if (iVar7 != 0) goto LAB_006ed6b0;
                }
              }
              else {
LAB_006ed6c6:
                local_248 = iVar5 * 4;
              }
LAB_006ed6d3:
              (pCVar8->m_Memory).m_nAllocationCount = iVar5;
              piVar6 = (pCVar8->m_Memory).m_pMemory;
              if (piVar6 == (int *)0x0) {
                piVar6 = (int *)(*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x4be6c7))
                                          ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x4be6c7),
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
          iVar7 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4be753) + 8))
                            ((int *)**(undefined4 **)(unaff_EBX + 0x4be753),0,local_244);
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
        pcVar4 = &UNK_002be68b + unaff_EBX;
      }
      CFmtStrN<256>::CFmtStrN
                (&local_238,(char *)(unaff_EBX + 0x325c4f),pcVar4,
                 *(undefined4 *)(unaff_EBX + 0x5e2a8f + eventName * 4));
      CBaseEntity::PrecacheScriptSound(local_238.m_szBuf);
    }
  }
  if (local_20 != **(int **)(unaff_EBX + 0x4be71b)) {
                    /* WARNING: Subroutine does not return */
    ___stack_chk_fail();
  }
  return;
}


/* CPropPersonalitySphere::Precache at 006ee1c0 */

/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: void Precache(CPropPersonalitySphere * this) */

void __thiscall CPropPersonalitySphere::Precache(CPropPersonalitySphere *this)

{
  int iVar1;
  int *piVar2;
  undefined4 *puVar3;
  uint uVar4;
  int iVar5;
  CUtlVector<int,CUtlMemory<int,_int>_> *pCVar6;
  int iVar7;
  int unaff_EBX;
  int local_28;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  CPhysicsProp::Precache(&this->super_CPhysicsProp);
  SetupVOList(this);
  local_28 = 0;
  puVar3 = *(undefined4 **)(unaff_EBX + 0x4bd9ca);
  do {
    (&(this->super_CPhysicsProp).field_0x766)[local_28] = 0;
    uVar4 = (this->m_iLineOrder).m_Size;
    iVar1 = uVar4 + 1;
    iVar7 = (this->m_iLineOrder).m_Memory.m_nAllocationCount;
    if (iVar7 < iVar1) {
      iVar5 = (this->m_iLineOrder).m_Memory.m_nGrowSize;
      if (-1 < iVar5) {
        if (iVar5 == 0) {
          if ((iVar7 == 0) && (iVar7 = 2, iVar1 < 3)) {
            local_20 = 0x28;
            goto LAB_006ee299;
          }
          do {
            iVar7 = iVar7 * 2;
          } while (iVar7 < iVar1);
LAB_006ee290:
          local_20 = iVar7 * 0x14;
        }
        else {
          iVar7 = ((int)uVar4 / iVar5 + 1) * iVar5;
          if (iVar1 <= iVar7) goto LAB_006ee290;
          if (iVar7 != 0) {
LAB_006ee270:
            do {
              iVar7 = (iVar7 + iVar1) / 2;
            } while (iVar7 < iVar1);
            goto LAB_006ee290;
          }
          if (iVar1 < 0) {
            iVar7 = -1;
            local_20 = -0x14;
          }
          else {
            local_20 = 0;
            if (iVar1 != 0) goto LAB_006ee270;
          }
        }
LAB_006ee299:
        (this->m_iLineOrder).m_Memory.m_nAllocationCount = iVar7;
        pCVar6 = (this->m_iLineOrder).m_Memory.m_pMemory;
        if (pCVar6 == (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0) {
          pCVar6 = (CUtlVector<int,CUtlMemory<int,_int>_> *)
                   (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x4bd9ca))
                             ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x4bd9ca),local_20);
          (this->m_iLineOrder).m_Memory.m_pMemory = pCVar6;
        }
        else {
          pCVar6 = (CUtlVector<int,CUtlMemory<int,_int>_> *)
                   (**(code **)(*(int *)*puVar3 + 4))((int *)*puVar3,pCVar6,local_20);
          (this->m_iLineOrder).m_Memory.m_pMemory = pCVar6;
        }
      }
    }
    piVar2 = &(this->m_iLineOrder).m_Size;
    *piVar2 = *piVar2 + 1;
    pCVar6 = (this->m_iLineOrder).m_Memory.m_pMemory;
    (this->m_iLineOrder).m_pElements = pCVar6;
    iVar7 = ~uVar4 + (this->m_iLineOrder).m_Size;
    if (0 < iVar7) {
      _V_memmove(pCVar6 + iVar1,pCVar6 + uVar4,iVar7 * 0x14);
    }
    pCVar6 = (this->m_iLineOrder).m_Memory.m_pMemory + uVar4;
    if (pCVar6 != (CUtlVector<int,CUtlMemory<int,_int>_> *)0x0) {
      (pCVar6->m_Memory).m_pMemory = (int *)0x0;
      (pCVar6->m_Memory).m_nAllocationCount = 0;
      (pCVar6->m_Memory).m_nGrowSize = 0;
      pCVar6->m_Size = 0;
      pCVar6->m_pElements = (int *)0x0;
    }
    local_28 = local_28 + 1;
    if (local_28 == 0xf) {
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
      CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x31ef2a));
      return;
    }
  } while( true );
}


/* CPropPersonalitySphere::InputPanic at 006ecc50 */

/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: void InputPanic(CPropPersonalitySphere * this, inputdata_t * inputdata)
    */

void __thiscall
CPropPersonalitySphere::InputPanic(CPropPersonalitySphere *this,inputdata_t *inputdata)

{
  this->m_bPanicked = true;
  return;
}


/* CPropPersonalitySphere::InputStartTalking at 006edca0 */

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
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0x101f);
  CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(**(int **)(&DAT_004bdeef + unaff_EBX) + 0xc) + this->m_flBetweenVOPadding +
             0.0,(char *)0x0);
  return;
}


/* CPropPersonalitySphere::StartTalking at 006ecd00 */

/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: void StartTalking(CPropPersonalitySphere * this, float flDelay) */

void __thiscall CPropPersonalitySphere::StartTalking(CPropPersonalitySphere *this,float flDelay)

{
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0x1fbf);
  CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(**(int **)(unaff_EBX + 0x4bee8f) + 0xc) + this->m_flBetweenVOPadding +
             flDelay,(char *)0x0);
  return;
}


/* CPropPersonalitySphere::OnPhysGunPickup at 006ee600 */

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
               *(float *)(**(int **)(unaff_EBX + 0x4bd58c) + 0xc) + this->m_flBetweenVOPadding + 0.0
               ,(char *)0x0);
    TrySpeakLine(this,7,10);
  }
  this->m_bFirstPickup = false;
  this->m_bHeld = true;
  nSequence = CBaseAnimating::LookupSequence((CBaseAnimating *)this,(char *)(unaff_EBX + 0x31eb1f));
  CBaseAnimating::ResetSequence((CBaseAnimating *)this,nSequence);
  CPhysicsProp::EnableMotion(&this->super_CPhysicsProp);
  CPhysicsProp::OnPhysGunPickup(&this->super_CPhysicsProp,pPhysGunUser,reason);
  return;
}


/* CPropPersonalitySphere::TalkingThink at 006eecd0 */

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
             *(float *)(unaff_EBX + 0x33cb66) + *(float *)(**(int **)(unaff_EBX + 0x4bcec2) + 0xc),
             (char *)0x0);
  if ((this->m_bFirstPickup == false) || (this->m_iCoreType == CORETYPE_AQUARIUM)) {
    iVar2 = (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
            m_nSequence.m_Value;
    label = (this->m_iszLookAnimationName).pszValue;
    if (label == (char *)0x0) {
      label = (char *)(unaff_EBX + 0x2bce7e);
    }
    nSequence = CBaseAnimating::LookupSequence((CBaseAnimating *)this,label);
    if (iVar2 != nSequence) {
      CBaseAnimating::ResetSequence((CBaseAnimating *)this,nSequence);
    }
    fVar1 = *(float *)(**(int **)(unaff_EBX + 0x4bcec2) + 0xc);
    fVar3 = CBaseEntity::GetLastThink((CBaseEntity *)this,(char *)0x0);
    this->m_flAbuseLevel =
         (float)((double)this->m_flAbuseLevel *
                ((double)(fVar1 - fVar3) * *(double *)(unaff_EBX + 0x33bd92) +
                *(double *)(unaff_EBX + 0x33b8d2)));
    CheckForInfoTargets(this);
    UpdatePositions(this);
    if (-1 < this->m_iPendingPriority) {
      TrySpeakLine(this,this->m_iPendingLine,this->m_iPendingPriority);
    }
    if ((this->m_flLineCompletionTime + this->m_flIdleWaitTime <
         *(float *)(**(int **)(unaff_EBX + 0x4bcec2) + 0xc)) && (this->m_bHeld != false)) {
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


/* CPropPersonalitySphere::UpdatePositions at 006ed880 */

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
  if ((*(int *)(**(int **)(unaff_EBX + 0x4be312) + 0x14) < 2) &&
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
       (*(float *)(unaff_EBX + 0x33ccaa) + this->m_flLastHeldTime <
        *(float *)(**(int **)(unaff_EBX + 0x4be312) + 0xc))) {
      if (this->m_bPanicked == false) {
        TrySpeakLine(this,5,1);
      }
      else {
        TrySpeakLine(this,6,1);
      }
    }
    pfVar3 = *(float **)(&DAT_004be322 + unaff_EBX);
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
        fVar8 = *(float *)(unaff_EBX + 0x33ca82) / (float)(iVar4 + -1);
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
      if (*(float *)(unaff_EBX + 0x346126) <= vVar10) {
        if (((this_00->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
             super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x1000) != 0) {
          CBaseEntity::CalcAbsoluteVelocity((CBaseEntity *)this_00);
          vVar10 = (this_00->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay
                   .super_CBaseAnimating.super_CBaseEntity.m_vecAbsVelocity.z;
        }
        iVar4 = 0xb;
        if (vVar10 < *(float *)(unaff_EBX + 0x33d142) || vVar10 == *(float *)(unaff_EBX + 0x33d142))
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
          if (fVar12 < *(float *)(unaff_EBX + 0x33d142) ||
              fVar12 == *(float *)(unaff_EBX + 0x33d142)) {
            return;
          }
          iVar4 = 0xe;
        }
      }
      TrySpeakLine(this,iVar4,3);
      return;
    }
                    /* Unresolved local var: __m128 root@[???] */
    fVar8 = *(float *)(**(int **)(unaff_EBX + 0x4be312) + 0xc);
    fVar13 = CBaseEntity::GetLastThink((CBaseEntity *)this,(char *)0x0);
    if ((fVar8 - fVar13) * *(float *)(unaff_EBX + 0x33d142) <
        SQRT(fVar11 * fVar11 + fVar9 * fVar9 + fVar12 * fVar12)) {
      TrySpeakLine(this,0xc,5);
    }
  }
  return;
}


/* CPropPersonalitySphere::TrySpeakLine at 006ecd80 */

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
  local_20 = **(int **)(unaff_EBX + 0x4bee5b);
  if ((&(this->super_CPhysicsProp).field_0x766)[line] == '\0') goto LAB_006ece50;
  fVar12 = this->m_flFirstNoSkipTime - *(float *)(**(int **)(unaff_EBX + 0x4bee0f) + 0xc);
  if (0.0 < fVar12) {
    if ((((fVar12 < *(float *)(unaff_EBX + 0x33d76f)) &&
         (!NAN(fVar12) && !NAN(*(float *)(unaff_EBX + 0x33d76f)))) && (line != 9)) &&
       (this->m_iPendingPriority <= priority)) {
      this->m_iPendingPriority = priority;
      this->m_iPendingLine = line;
    }
  }
  if ((this->m_iCurrentPriority < priority) || (line == 0)) {
    this->m_flLineCompletionTime = *(float *)(**(int **)(unaff_EBX + 0x4bee0f) + 0xc);
  }
  pfVar1 = (float *)(**(int **)(unaff_EBX + 0x4bee0f) + 0xc);
  if (*pfVar1 <= this->m_flLineCompletionTime && this->m_flLineCompletionTime != *pfVar1)
  goto LAB_006ece50;
  if ((*(float *)(unaff_EBX + 0x33d57f) < this->m_flAbuseLevel) && (priority < 4))
  goto LAB_006ece50;
  if (line == 9) {
    if (this->field_0x770 != '\0') {
      fVar12 = *(float *)(unaff_EBX + 0x33d57f) + this->m_flAbuseLevel;
      this->m_flAbuseLevel = fVar12;
      if (*(float *)(&DAT_0033db3f + unaff_EBX) <= fVar12 &&
          fVar12 != *(float *)(&DAT_0033db3f + unaff_EBX)) {
        local_258 = *(float *)(unaff_EBX + 0x33d883);
        line = 10;
        local_240 = 10;
        goto LAB_006ecead;
      }
    }
    local_258 = 0.0;
    local_240 = 9;
LAB_006ecead:
    if (this->m_iCoreType == CORETYPE_AQUARIUM) goto LAB_006ed020;
  }
  else {
    if (line != 2) {
      local_240 = line;
      local_258 = *(float *)(unaff_EBX + 0x33d883);
      goto LAB_006ecead;
    }
LAB_006ed020:
    local_240 = line;
    local_258 = *(float *)(unaff_EBX + 0x33d76f);
  }
  soundname = (string_t *)(this->m_iszLastLinePlayed).pszValue;
  psVar6 = (string_t *)(unaff_EBX + 0x2bedcb);
  if (soundname == (string_t *)0x0) {
    soundname = psVar6;
  }
  CBaseEntity::StopSound((CBaseEntity *)this,(char *)soundname);
  if (this->m_iCoreType == CORETYPE_RICHARD) {
    if (line == 0) goto LAB_006ed050;
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
          goto LAB_006ed199;
        }
        iVar8 = ((int)uVar3 / iVar4 + 1) * iVar4;
        if (iVar8 < iVar7) {
          if (iVar8 != 0) {
LAB_006ed176:
            do {
              iVar8 = (iVar8 + iVar7) / 2;
            } while (iVar8 < iVar7);
            goto LAB_006ed18c;
          }
          if (iVar7 < 0) {
            iVar8 = -1;
            local_244 = -4;
          }
          else {
            local_244 = 0;
            if (iVar7 != 0) goto LAB_006ed176;
          }
        }
        else {
LAB_006ed18c:
          local_244 = iVar8 * 4;
        }
LAB_006ed199:
        (pCVar11->m_Memory).m_nAllocationCount = iVar8;
        piVar9 = (pCVar11->m_Memory).m_pMemory;
        if (piVar9 == (int *)0x0) {
          piVar9 = (int *)(*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x4bee07))
                                    ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x4bee07),local_244
                                    );
          (pCVar11->m_Memory).m_pMemory = piVar9;
        }
        else {
          piVar9 = (int *)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4bee07) + 4))
                                    ((int *)**(undefined4 **)(unaff_EBX + 0x4bee07),piVar9,local_244
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
      pcVar10 = (char *)(unaff_EBX + 0x2bedcb);
    }
    soundname = (string_t *)(unaff_EBX + 0x326385);
    CFmtStrN<256>::CFmtStrN
              ((CFmtStrN<256> *)local_12c,(char *)soundname,pcVar10,
               *(undefined4 *)(unaff_EBX + 0x5e31cf + line * 4),iVar2);
    psVar5 = (string_t *)(local_12c + 5);
  }
  else if (line == 0) {
LAB_006ed050:
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
    soundname = (string_t *)(unaff_EBX + 0x32638f);
    CFmtStrN<256>::CFmtStrN
              ((CFmtStrN<256> *)local_238,(char *)soundname,psVar5,
               *(undefined4 *)(unaff_EBX + 0x5e31cf + line * 4));
    psVar5 = (string_t *)(local_238 + 5);
  }
  psVar6 = AllocPooledString(psVar5,(char *)soundname);
  (this->m_iszLastLinePlayed).pszValue = (char *)psVar6;
  pcVar10 = (this->m_iszLastLinePlayed).pszValue;
  if (pcVar10 == (char *)0x0) {
    pcVar10 = (char *)(unaff_EBX + 0x2bedcb);
  }
  CBaseEntity::EmitSound((CBaseEntity *)this,pcVar10,0.0,(float *)0x0);
  this->m_iCurrentLine = line;
  this->m_iCurrentPriority = priority;
  this->m_iPendingPriority = -1;
  this->m_iPendingLine = -1;
  pcVar10 = (this->m_iszLastLinePlayed).pszValue;
  if (pcVar10 == (char *)0x0) {
    pcVar10 = (char *)(unaff_EBX + 0x2bedcb);
  }
  fVar13 = CBaseEntity::GetSoundDuration(pcVar10,(char *)(unaff_EBX + 0x320367));
  piVar9 = *(int **)(unaff_EBX + 0x4bee0f);
  fVar12 = fVar13 + *(float *)(*piVar9 + 0xc);
  this->m_flFirstNoSkipTime = fVar12;
  this->m_flLineCompletionTime =
       *(float *)(*piVar9 + 0xc) + this->m_flBetweenVOPadding + fVar13 + local_258;
  if (local_240 - 9U < 2) {
    this->m_flLineCompletionTime = fVar12;
  }
LAB_006ece50:
  if (local_20 != **(int **)(unaff_EBX + 0x4bee5b)) {
                    /* WARNING: Subroutine does not return */
    ___stack_chk_fail();
  }
  return;
}


/* CPropPersonalitySphere::AnimateThink at 006ecc80 */

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
             *(float *)(unaff_EBX + 0x33d683) + *(float *)(**(int **)(unaff_EBX + 0x4bef0f) + 0xc),
             *(char **)(unaff_EBX + 0x550423));
  return;
}


/* CPropPersonalitySphere::SetupVOList at 006eded0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* DWARF original prototype: void SetupVOList(CPropPersonalitySphere * this) */

void __thiscall CPropPersonalitySphere::SetupVOList(CPropPersonalitySphere *this)

{
  uint *puVar1;
  CBaseEdict *this_00;
  string_t *psVar2;
  IChangeInfoAccessor *pIVar3;
  int unaff_EBX;
  char *in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  if (CORETYPE_AQUARIUM < this->m_iCoreType) {
    psVar2 = AllocPooledString((string_t *)(unaff_EBX + 0x3252c1),in_stack_ffffffe8);
    (this->m_iszLookAnimationName).pszValue = (char *)psVar2;
    if ((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.m_nSkin
        .m_Value != 0) {
      if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                          super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
          false) {
        this_00 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
        if (this_00 != (CBaseEdict *)0x0) {
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
          pIVar3 = CBaseEdict::GetChangeAccessor(this_00);
          pIVar3->m_iChangeInfoSerialNumber = 0;
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
    psVar2 = AllocPooledString((string_t *)(unaff_EBX + 0x3252e7),in_stack_ffffffe8);
    (this->m_iszDeathSoundScriptName).pszValue = (char *)psVar2;
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x006edf4f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)(*(int *)(unaff_EBX + 0x77 + this->m_iCoreType * 4) + unaff_EBX))();
  return;
}


/* CPropPersonalitySphere::OnPhysGunDrop at 006ed450 */

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
  this->m_flLastHeldTime = *(float *)(**(int **)(unaff_EBX + 0x4be73c) + 0xc);
  CPhysicsProp::OnPhysGunDrop(&this->super_CPhysicsProp,pPhysGunUser,reason);
  return;
}


/* CPropPersonalitySphere::VPhysicsCollision at 006ed330 */

/* WARNING: Struct "CPropPersonalitySphere": ignoring overlapping field "m_bHasEvent" */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
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
  if (piVar3 == (int *)**(int **)(unaff_EBX + 0x4be918)) {
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
    if (fVar5 < *(float *)(CAI_StandoffBehavior::SelectScheduleUpdateWeapon + unaff_EBX + 4) ||
        fVar5 == *(float *)(CAI_StandoffBehavior::SelectScheduleUpdateWeapon + unaff_EBX + 4)) {
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


/* CPropPersonalitySphere::NotifySystemEvent at 006ecc60 */

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


/* _GLOBAL__I_g_LineNames at 000cd4d0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_g_LineNames(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

