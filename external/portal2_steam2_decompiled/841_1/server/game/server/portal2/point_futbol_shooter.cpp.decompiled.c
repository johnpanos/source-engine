/* DWARF-guided pseudocode for game/server/portal2/point_futbol_shooter.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000af840 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  undefined4 uVar1;
  datamap_t *pdVar2;
  IEntityFactoryDictionary *pIVar3;
  ConVar *unaff_EBX;
  char *unaff_EBP;
  char *unaff_ESI;
  longlong lVar4;
  int unaff_retaddr;
  ConVar *this_1;
  
  this_1 = unaff_EBX;
  lVar4 = ___i686_get_pc_thunk_bx();
  if (lVar4 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)&unaff_EBX[0x279f2].m_fnChangeCallbacks.m_Memory.m_pMemory + 3) = 0;
  *(undefined1 *)&unaff_EBX[0x279f2].m_fnChangeCallbacks.m_Memory.m_nAllocationCount = 0;
  *(undefined1 *)((int)&unaff_EBX[0x279f2].m_fnChangeCallbacks.m_Memory.m_nAllocationCount + 1) = 0;
  *(undefined1 *)((int)&unaff_EBX[0x279f2].m_fnChangeCallbacks.m_Memory.m_nAllocationCount + 2) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x279f2].m_fnChangeCallbacks.m_Memory.m_nAllocationCount + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x279f2].m_fnChangeCallbacks.m_Memory.m_nGrowSize + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x279f2].m_fnChangeCallbacks.m_Size + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x279f2].m_fnChangeCallbacks.m_pElements + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x279f3].super_ConCommandBase._vptr_ConCommandBase + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x279f3].super_ConCommandBase.m_pNext + 3) = 0;
  *(undefined4 *)&unaff_EBX[0x279f3].super_ConCommandBase.field_0xb = 0;
  *(undefined4 *)((int)&unaff_EBX[0x279f3].super_ConCommandBase.m_pszName + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x279f3].super_ConCommandBase.m_pszHelpString + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x279f3].super_ConCommandBase.m_nFlags + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x279f3].super_IConVar._vptr_IConVar + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x279f3].m_pParent + 3) = 0x7f7fffff;
  *(undefined1 **)((int)&unaff_EBX[0x279f3].m_pszDefaultValue + 3) =
       (undefined1 *)((int)&unaff_EBX[0x21c7c].super_ConCommandBase.m_nFlags + 3);
  ConVar::ConVar((ConVar *)((int)&unaff_EBX[0x261c4].m_fnChangeCallbacks.m_Memory.m_nGrowSize + 3),
                 (char *)((int)&unaff_EBX[0x1d205].m_Value.m_StringLength + 3),
                 (char *)((int)&unaff_EBX[0x1cea1].m_Value.m_pszString + 3),0x4000,this_1,unaff_ESI,
                 unaff_EBP,unaff_retaddr);
  uVar1 = *(undefined4 *)&unaff_EBX[0x21b7d].super_ConCommandBase.field_0xb;
  ___cxa_atexit((undefined1 *)((int)&unaff_EBX[0x1bcd8].super_ConCommandBase.m_pszName + 3),0,uVar1)
  ;
  ConVar::ConVar((ConVar *)((int)&unaff_EBX[0x261c5].m_fnChangeCallbacks.m_pElements + 3),
                 (char *)((int)&unaff_EBX[0x1d205].m_fnChangeCallbacks.m_Memory.m_nGrowSize + 3),
                 (char *)((int)&unaff_EBX[0x1bfe1].m_fnChangeCallbacks.m_pElements + 3),0x4000,
                 this_1,unaff_ESI,unaff_EBP,unaff_retaddr);
  ___cxa_atexit((undefined1 *)((int)&unaff_EBX[0x1bcd7].m_fnChangeCallbacks.m_Memory.m_pMemory + 3),
                0,uVar1);
  pdVar2 = DataMapInit<CPointFutbolShooter>((CPointFutbolShooter *)0x0);
  *(datamap_t **)((int)&unaff_EBX[0x261c6].m_fnChangeCallbacks.m_pElements + 3) = pdVar2;
  *(undefined1 **)((int)&unaff_EBX[0x279f3].m_Value.m_pszString + 3) =
       (undefined1 *)((int)&unaff_EBX[0x23441].m_fnChangeCallbacks.m_Memory.m_nGrowSize + 3);
  pIVar3 = EntityFactoryDictionary();
  (**pIVar3->_vptr_IEntityFactoryDictionary)
            (pIVar3,(undefined1 *)((int)&unaff_EBX[0x279f3].m_Value.m_pszString + 3),
             (undefined1 *)((int)&unaff_EBX[0x1d206].super_IConVar._vptr_IConVar + 3));
  return;
}


/* __tcf_1 at 00a3e2a0 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38b158),in_stack_00000008);
  return;
}


/* __tcf_2 at 00a3e280 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38b1d8),in_stack_00000008);
  return;
}


/* CEntityFactory<CPointFutbolShooter>::Create at 00740510 */

/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CPointFutbolShooter> * this,
   char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CPointFutbolShooter>::Create
          (CEntityFactory<CPointFutbolShooter> *this,char *pClassName)

{
  CBaseEntity *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CPointFutbolShooter * pEnt@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CBaseEntity::operator_new(0x400);
  CBaseEntity::CBaseEntity(this_00,false);
  (this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x58e6a4);
  this_00[1].super_IServerEntity.super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)0x0;
  this_00[1].m_pfnMoveDone.__delta = -1;
  this_00[1].m_pfnThink.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)0x0;
  this_00[1].m_Network.super_IServerNetworkable._vptr_IServerNetworkable = (_func_int_varargs **)0x0
  ;
  this_00[1].m_Network.m_pPev = (edict_t *)0xffffffff;
  this_00[1].m_Network.m_PVSInfo.m_nHeadNode = 0;
  this_00[1].m_Network.m_PVSInfo.m_nClusterCount = 0;
  (**(code **)(unaff_EBX + 0x58e718))(this_00,pClassName);
  return &(this_00->m_Network).super_IServerNetworkable;
}


/* CPointFutbolShooter::~CPointFutbolShooter at 007405b0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPointFutbolShooter(CPointFutbolShooter * this, int __in_chrg) */

void __thiscall CPointFutbolShooter::~CPointFutbolShooter(CPointFutbolShooter *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x58e604);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OutputShootFail).super_CBaseEntityOutput,in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OutputShootSuccess).super_CBaseEntityOutput,in_stack_ffffffd8);
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x5066cc) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,__in_chrg);
  return;
}


/* CPointFutbolShooter::~CPointFutbolShooter at 00740640 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPointFutbolShooter(CPointFutbolShooter * this, int __in_chrg) */

void __thiscall CPointFutbolShooter::~CPointFutbolShooter(CPointFutbolShooter *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x58e574);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OutputShootFail).super_CBaseEntityOutput,in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OutputShootSuccess).super_CBaseEntityOutput,in_stack_ffffffd8);
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x50663c) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,in_stack_ffffffd8);
  CBaseEntity::operator_delete(this);
  return;
}


/* DataMapInit<CPointFutbolShooter> at 000af710 */

datamap_t * DataMapInit<CPointFutbolShooter>(CPointFutbolShooter *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((&DAT_00001104)[(int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd9ece4);
    if (iVar2 != 0) {
      *(int *)(&DAT_00001114 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
           unaff_EBX + 0xa032fa;
      *(undefined4 *)
       (&DAT_0000111c + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
      *(undefined4 *)
       (&DAT_00001120 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
      *(undefined4 *)
       (&DAT_00001124 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
      *(undefined4 *)
       (&DAT_00001128 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
      *(undefined4 *)
       (&DAT_0000112c + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
      *(undefined4 *)
       (&DAT_00001118 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0x13;
      ___cxa_guard_release(unaff_EBX + 0xd9ece4);
      ___cxa_atexit(unaff_EBX + 0x98eb9c,0,*(undefined4 *)(&DAT_00b97430 + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&CPluginBotManager::typeinfo + unaff_EBX) =
       *(undefined4 *)(&DAT_00b97568 + unaff_EBX);
  if ((&DAT_0000110c)[(int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd9ecec);
    if (iVar2 != 0) {
      uVar1 = **(undefined4 **)(&DAT_00b97450 + unaff_EBX);
      *(undefined4 *)(&DAT_00cb6c10 + unaff_EBX) = uVar1;
      *(undefined4 *)(&audioparams_t::vtable + unaff_EBX) = uVar1;
      ___cxa_guard_release(unaff_EBX + 0xd9ecec);
    }
  }
  *(undefined4 *)((int)&PTR_GetPlayerInfo_00cb6ae0 + unaff_EBX) = 5;
  *(undefined **)((int)&PTR_typeinfo_00cb6adc + unaff_EBX) = &UNK_00cb6b3c + unaff_EBX;
  return (datamap_t *)((int)&PTR_typeinfo_00cb6adc + unaff_EBX);
}


/* CPointFutbolShooter::GetDataDescMap at 0073fd90 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPointFutbolShooter * this) */

datamap_t * __thiscall CPointFutbolShooter::GetDataDescMap(CPointFutbolShooter *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x626468);
}


/* CPointFutbolShooter::GetBaseMap at 0073fda0 */

datamap_t * CPointFutbolShooter::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x506ee4);
}


/* __tcf_0 at 00a3e2c0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41015e)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x410152) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41015e));
  }
  *(undefined4 *)(unaff_EBX + 0x41015e) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41015a)) {
    if (*(int *)(unaff_EBX + 0x410152) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20888e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20888e),*(int *)(unaff_EBX + 0x410152));
      *(undefined4 *)(unaff_EBX + 0x410152) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x410156) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x410152);
  *(int *)(unaff_EBX + 0x410162) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41015a)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20888e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20888e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x410152) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x410156) = 0;
  }
  return;
}


/* CPointFutbolShooter::Spawn at 0073fdf0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CPointFutbolShooter * this) */

void __thiscall CPointFutbolShooter::Spawn(CPointFutbolShooter *this)

{
  CPointEntity::Spawn(&this->super_CPointEntity);
                    /* WARNING: Could not recover jumptable at 0x0073fe0f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1a])();
  return;
}


/* CPointFutbolShooter::Precache at 0073fdb0 */

/* DWARF original prototype: void Precache(CPointFutbolShooter * this) */

void __thiscall CPointFutbolShooter::Precache(CPointFutbolShooter *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  UTIL_PrecacheOther(&UNK_0031c640 + unaff_EBX,(char *)0x0);
  PrecacheParticleSystem((char *)(unaff_EBX + 0x36dbf8));
  return;
}


/* CPointFutbolShooter::InputShootFutbol at 0073fe20 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void InputShootFutbol(CPointFutbolShooter * this, inputdata_t * data)
    */

void __thiscall CPointFutbolShooter::InputShootFutbol(CPointFutbolShooter *this,inputdata_t *data)

{
  IPhysicsObject *pIVar1;
  code *pcVar2;
  Vector vecOrigin;
  CBaseEntity *this_00;
  int iVar3;
  int *piVar4;
  CBasePlayer *pCVar5;
  int unaff_EBX;
  float fVar6;
  float local_c8;
  float local_c4;
  float local_c0;
  float local_bc [7];
  float local_a0;
  float local_9c;
  float local_98;
  undefined4 local_94;
  undefined4 local_90;
  undefined4 local_8c;
  float local_88;
  float local_84;
  float local_80;
  Vector local_7c;
  float local_70;
  float local_6c;
  float local_68;
  Vector local_64;
  float local_58;
  float local_54;
  float local_50;
  float local_4c;
  float local_48;
  float local_44;
  float local_40;
  float local_3c;
  float local_38;
  float local_34;
  float local_30;
  float local_2c;
  Vector local_28 [2];
  
                    /* Unresolved local var: Vector vecSourcePos@[???]
                       Unresolved local var: Vector vecTargetPos@[???]
                       Unresolved local var: Vector vecVelocity@[???]
                       Unresolved local var: Vector vecForward@[???]
                       Unresolved local var: CBaseEntity * pFutbol@[???]
                       Unresolved local var: float flSpeed@[???]
                       Unresolved local var: float flGravity@[???]
                       Unresolved local var: float time@[???]
                       Unresolved local var: IPhysicsObject * pPhysObject@[???]
                       Unresolved local var: Vector color@[???]
                       Unresolved local var: Vector vecReticulePos@[???] */
  ___i686_get_pc_thunk_bx();
  if (((this->super_CPointEntity).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
  local_28[0].x = (this->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.x;
  local_28[0].y = (this->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.y;
  local_28[0].z = (this->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.z;
  local_c8 = (this->m_vTarget).z;
  local_c4 = (this->m_vTarget).y;
  local_c0 = (this->m_vTarget).x;
                    /* Unresolved local var: Vector res@[???] */
  local_4c = local_c0 - local_28[0].x;
  local_48 = local_c4 - local_28[0].y;
  local_44 = local_c8 - local_28[0].z;
  local_40 = local_4c;
  local_3c = local_48;
  local_38 = local_44;
  if (0.0 < *(float *)(*(int *)(unaff_EBX + 0x6895eb) + 0x2c)) {
                    /* Unresolved local var: Vector vecToTarget@[???] */
    local_7c.z = 0.0;
    local_7c.x = local_4c;
    local_7c.y = local_48;
    VectorNormalize(&local_7c);
    local_50 = *(float *)(*(int *)(unaff_EBX + 0x6895eb) + 0x2c);
                    /* Unresolved local var: Vector res@[???] */
    local_58 = local_50 * local_7c.x;
    local_54 = local_50 * local_7c.y;
    local_50 = local_50 * local_7c.z;
    local_c0 = local_c0 - local_58;
    local_c4 = local_c4 - local_54;
    local_c8 = local_c8 - local_50;
  }
  if (((this->super_CPointEntity).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
  AngleVectors(&(this->super_CPointEntity).super_CBaseEntity.m_angAbsRotation,&local_64,
               (Vector *)0x0,(Vector *)0x0);
  if (0.0 <= local_40 * local_64.x + local_3c * local_64.y) {
    this_00 = CreateEntityByName(&UNK_0031c5cb + unaff_EBX,-1,true);
    if (this_00 != (CBaseEntity *)0x0) {
      CBaseEntity::SetAbsOrigin(this_00,local_28);
      (*(this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
        [0x19])(this_00);
                    /* Unresolved local var: __m128 root@[???] */
      local_bc[0] = SQRT(local_40 * local_40 + local_3c * local_3c + local_38 * local_38);
      local_bc[1] = 0.0;
      local_bc[2] = 0.0;
      local_bc[3] = 0.0;
      fVar6 = local_bc[0] / this->m_flPhysicsSpeed;
      local_68 = *(float *)(unaff_EBX + 0x38cadf) / fVar6;
                    /* Unresolved local var: Vector res@[???] */
      local_70 = local_68 * local_40;
      local_6c = local_68 * local_3c;
      local_68 = local_68 * local_38;
      local_38 = (float)((double)local_68 +
                        (double)(*(float *)(*(int *)(*(int *)(unaff_EBX + 0x506e97) + 0x1c) + 0x2c)
                                * fVar6) * *(double *)(unaff_EBX + 0x38d1ff));
      pIVar1 = this_00->m_pPhysicsObject;
      local_40 = local_70;
      local_3c = local_6c;
      if (pIVar1 != (IPhysicsObject *)0x0) {
                    /* Unresolved local var: AngularImpulse angImpulse@[???]
                       Unresolved local var: float flNull@[???]
                       Unresolved local var: CPhysicsProp * pProp@[???]
                       Unresolved local var: AngularImpulse angImp@[???] */
        iVar3 = rand();
        local_88 = ((float)iVar3 / *(float *)(unaff_EBX + 0x38cdd7)) *
                   *(float *)(unaff_EBX + 0x38e04f) + *(float *)(unaff_EBX + 0x38df33);
        iVar3 = rand();
        local_84 = ((float)iVar3 / *(float *)(unaff_EBX + 0x38cdd7)) *
                   *(float *)(unaff_EBX + 0x38e04f) + *(float *)(unaff_EBX + 0x38df33);
        iVar3 = rand();
        local_80 = ((float)iVar3 / *(float *)(unaff_EBX + 0x38cdd7)) *
                   *(float *)(unaff_EBX + 0x38e04f) + *(float *)(unaff_EBX + 0x38df33);
        local_7c.x = local_88;
        local_7c.y = local_84;
        local_7c.z = local_80;
        (*pIVar1->_vptr_IPhysicsObject[0x34])(pIVar1,&local_40,&local_7c);
        local_bc[0] = 0.0;
        (*pIVar1->_vptr_IPhysicsObject[0x25])(pIVar1,local_bc,local_bc);
        (*pIVar1->_vptr_IPhysicsObject[0x23])(pIVar1,local_bc,local_bc);
        piVar4 = (int *)___dynamic_cast(this_00,*(undefined4 *)(unaff_EBX + 0x506d5f),
                                        *(undefined4 *)(unaff_EBX + 0x506e53),0);
        if (piVar4 != (int *)0x0) {
          pcVar2 = *(code **)(*piVar4 + 0x440);
          pCVar5 = UTIL_GetLocalPlayer();
          (*pcVar2)(piVar4,pCVar5,3);
        }
      }
      COutputEvent::FireOutput(&this->m_OutputShootSuccess,data->pActivator,data->pCaller,0.0);
      local_94 = 0x437f0000;
      local_90 = 0;
      local_8c = 0;
      local_2c = local_c8;
      local_30 = local_c4;
      local_34 = local_c0;
      local_a0 = local_c0;
      local_9c = local_c4;
      local_98 = local_c8 - *(float *)(*(int *)(unaff_EBX + 0x68964b) + 0x2c);
      vecOrigin.y = local_c4;
      vecOrigin.x = local_c0;
      vecOrigin.z = local_98;
      DispatchParticleEffect
                ((char *)(CAI_HintManager::IsInFoundHintList + unaff_EBX + 3),vecOrigin,
                 (Vector)ZEXT812(0x437f0000),**(QAngle **)(unaff_EBX + 0x506dd7),(CBaseEntity *)0x0,
                 -1);
      return;
    }
    DevMsg((char *)(unaff_EBX + 0x372c63));
    return;
  }
  COutputEvent::FireOutput(&this->m_OutputShootFail,data->pActivator,data->pCaller,0.0);
  return;
}


/* CPointFutbolShooter::InputSetTarget at 007403b0 */

/* DWARF original prototype: void InputSetTarget(CPointFutbolShooter * this, inputdata_t * data) */

void __thiscall CPointFutbolShooter::InputSetTarget(CPointFutbolShooter *this,inputdata_t *data)

{
  uint uVar1;
  char *szName;
  CBaseEntity *pCVar2;
  uint *puVar3;
  int iVar4;
  int unaff_EBX;
  
                    /* Unresolved local var: EHANDLE hTarget@[???]
                       Unresolved local var: CBaseEntity * pTargetEnt@[???] */
  ___i686_get_pc_thunk_bx();
  szName = (char *)0x0;
  if ((data->value).fieldType == FIELD_STRING) {
    szName = (char *)(data->value).field_0.iVal;
  }
  if (szName == (char *)0x0) {
    szName = (char *)(unaff_EBX + 0x30e960);
  }
  pCVar2 = CGlobalEntityList::FindEntityByName
                     (*(CGlobalEntityList **)(unaff_EBX + 0x506860),(CBaseEntity *)0x0,szName,
                      (CBaseEntity *)0x0,(CBaseEntity *)0x0,(CBaseEntity *)0x0,
                      (IEntityFindFilter *)0x0);
  if (pCVar2 != (CBaseEntity *)0x0) {
    puVar3 = (uint *)(*(pCVar2->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                       _vptr_IHandleEntity[3])(pCVar2);
    uVar1 = *puVar3;
    if (((uVar1 != 0xffffffff) &&
        (iVar4 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5067a8),
        *(uint *)(iVar4 + 8) == uVar1 >> 0x10)) &&
       (pCVar2 = *(CBaseEntity **)(iVar4 + 4), pCVar2 != (CBaseEntity *)0x0)) {
      if ((pCVar2->m_iEFlags & 0x800) != 0) {
        CBaseEntity::CalcAbsolutePosition(pCVar2);
      }
      (this->m_vTarget).x = (pCVar2->m_vecAbsOrigin).x;
      (this->m_vTarget).y = (pCVar2->m_vecAbsOrigin).y;
      (this->m_vTarget).z = (pCVar2->m_vecAbsOrigin).z;
      return;
    }
  }
  DevMsg((char *)(unaff_EBX + 0x3726f5));
  return;
}


/* _GLOBAL__I_futbol_shooter_distance_from_target at 000af9c0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_futbol_shooter_distance_from_target(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

