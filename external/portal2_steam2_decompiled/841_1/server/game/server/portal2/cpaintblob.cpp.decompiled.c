/* DWARF-guided pseudocode for game/server/portal2/cpaintblob.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000a84a0 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  undefined4 uVar1;
  int unaff_EBX;
  char *unaff_ESI;
  longlong lVar2;
  ConVar *in_stack_ffffffe4;
  ConVar *in_stack_ffffffe8;
  char *in_stack_ffffffec;
  char *in_stack_fffffff0;
  int flags_1;
  
  flags_1 = unaff_EBX;
  lVar2 = ___i686_get_pc_thunk_bx();
  if (lVar2 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x554].m_SerialNumber + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x554].m_pPrev + unaff_EBX) = 0
  ;
  *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x554].m_pPrev + unaff_EBX + 1)
       = 0;
  *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x554].m_pPrev + unaff_EBX + 2)
       = 0;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x554].m_pPrev + unaff_EBX + 3)
       = 0;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x554].m_pNext + unaff_EBX + 3)
       = 0;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x555].m_pEntity + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x555].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x555].m_pPrev + unaff_EBX + 3)
       = 0;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x555].m_pNext + unaff_EBX + 3)
       = 0;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x556].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x556].m_SerialNumber + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x556].m_pPrev + unaff_EBX + 3)
       = 0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x556].m_pNext + unaff_EBX + 3)
       = 0x7f7fffff;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x557].m_pEntity + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x557].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x557].m_pPrev + unaff_EBX + 3)
       = &UNK_00ba3e57 + unaff_EBX;
  ConVar::ConVar((ConVar *)((int)DataMapInit<CAI_Squad>::dataDesc[5].flatOffset + unaff_EBX + -5),
                 (char *)(unaff_EBX + 0xa07ec7),(char *)(unaff_EBX + 0xa07ec3),0x4000,
                 in_stack_ffffffe4,(char *)in_stack_ffffffe8,in_stack_ffffffec,
                 (int)in_stack_fffffff0);
  uVar1 = *(undefined4 *)(&DAT_00b9e6a3 + unaff_EBX);
  ___cxa_atexit(&UNK_00993a3f + unaff_EBX,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<AISquadEnemyInfo_t>::dataDesc[0].flatOffset + unaff_EBX + -0x25),
                 (char *)(unaff_EBX + 0xa07f23),(char *)(unaff_EBX + 0x9a694f),0x6000,
                 (char *)(unaff_EBX + 0xa07edb),in_stack_ffffffe8,in_stack_ffffffec,
                 in_stack_fffffff0,flags_1,unaff_ESI);
  ___cxa_atexit(&UNK_00993a1f + unaff_EBX,0,uVar1);
  return;
}


/* __tcf_0 at 00a3bef0 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38c168),in_stack_00000008);
  return;
}


/* __tcf_1 at 00a3bed0 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38c1e8),in_stack_00000008);
  return;
}


/* CPaintBlob::CPaintBlob at 007086d0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void CPaintBlob(CPaintBlob * this) */

void __thiscall CPaintBlob::CPaintBlob(CPaintBlob *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBasePaintBlob::CBasePaintBlob(&this->super_CBasePaintBlob);
  (this->super_CBasePaintBlob)._vptr_CBasePaintBlob =
       (_func_int_varargs **)(*(int *)(&DAT_0053efa7 + unaff_EBX) + 8);
  this->m_pPaintingEntity = (CBaseEntity *)0x0;
  this->m_ID = 0;
  this->m_bTeleportedThisFrame = false;
  (this->m_teleportationHistory).m_Memory.m_pMemory = (BlobTeleportationHistory_t *)0x0;
  (this->m_teleportationHistory).m_Memory.m_nAllocationCount = 0;
  (this->m_teleportationHistory).m_Memory.m_nGrowSize = 0;
  (this->m_teleportationHistory).m_Size = 0;
  (this->m_teleportationHistory).m_pElements = (BlobTeleportationHistory_t *)0x0;
  return;
}


/* CPaintBlob::CPaintBlob at 00708b10 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void CPaintBlob(CPaintBlob * this) */

void __thiscall CPaintBlob::CPaintBlob(CPaintBlob *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBasePaintBlob::CBasePaintBlob(&this->super_CBasePaintBlob);
  (this->super_CBasePaintBlob)._vptr_CBasePaintBlob =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x53eb67) + 8);
  this->m_pPaintingEntity = (CBaseEntity *)0x0;
  this->m_ID = 0;
  this->m_bTeleportedThisFrame = false;
  (this->m_teleportationHistory).m_Memory.m_pMemory = (BlobTeleportationHistory_t *)0x0;
  (this->m_teleportationHistory).m_Memory.m_nAllocationCount = 0;
  (this->m_teleportationHistory).m_Memory.m_nGrowSize = 0;
  (this->m_teleportationHistory).m_Size = 0;
  (this->m_teleportationHistory).m_pElements = (BlobTeleportationHistory_t *)0x0;
  return;
}


/* CPaintBlob::~CPaintBlob at 00708a00 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ~CPaintBlob(CPaintBlob * this, int __in_chrg) */

void __thiscall CPaintBlob::~CPaintBlob(CPaintBlob *this,int __in_chrg)

{
  BlobTeleportationHistory_t *pBVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBasePaintBlob)._vptr_CBasePaintBlob =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x53ec74) + 8);
  (this->m_teleportationHistory).m_Size = 0;
  if ((this->m_teleportationHistory).m_Memory.m_nGrowSize < 0) {
    pBVar1 = (this->m_teleportationHistory).m_Memory.m_pMemory;
  }
  else {
    pBVar1 = (this->m_teleportationHistory).m_Memory.m_pMemory;
    if (pBVar1 != (BlobTeleportationHistory_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53e148) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x53e148),pBVar1);
      (this->m_teleportationHistory).m_Memory.m_pMemory = (BlobTeleportationHistory_t *)0x0;
    }
    pBVar1 = (BlobTeleportationHistory_t *)0x0;
    (this->m_teleportationHistory).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_teleportationHistory).m_pElements = pBVar1;
  if (-1 < (this->m_teleportationHistory).m_Memory.m_nGrowSize) {
    if (pBVar1 != (BlobTeleportationHistory_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53e148) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x53e148),pBVar1);
      (this->m_teleportationHistory).m_Memory.m_pMemory = (BlobTeleportationHistory_t *)0x0;
    }
    (this->m_teleportationHistory).m_Memory.m_nAllocationCount = 0;
  }
  CBasePaintBlob::~CBasePaintBlob(&this->super_CBasePaintBlob,__in_chrg);
  return;
}


/* CPaintBlob::~CPaintBlob at 00708e30 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ~CPaintBlob(CPaintBlob * this, int __in_chrg) */

void __thiscall CPaintBlob::~CPaintBlob(CPaintBlob *this,int __in_chrg)

{
  BlobTeleportationHistory_t *pBVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBasePaintBlob)._vptr_CBasePaintBlob =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x53e844) + 8);
  (this->m_teleportationHistory).m_Size = 0;
  if ((this->m_teleportationHistory).m_Memory.m_nGrowSize < 0) {
    pBVar1 = (this->m_teleportationHistory).m_Memory.m_pMemory;
  }
  else {
    pBVar1 = (this->m_teleportationHistory).m_Memory.m_pMemory;
    if (pBVar1 != (BlobTeleportationHistory_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53dd18) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x53dd18),pBVar1);
      (this->m_teleportationHistory).m_Memory.m_pMemory = (BlobTeleportationHistory_t *)0x0;
    }
    pBVar1 = (BlobTeleportationHistory_t *)0x0;
    (this->m_teleportationHistory).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_teleportationHistory).m_pElements = pBVar1;
  if (-1 < (this->m_teleportationHistory).m_Memory.m_nGrowSize) {
    if (pBVar1 != (BlobTeleportationHistory_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53dd18) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x53dd18),pBVar1);
      (this->m_teleportationHistory).m_Memory.m_pMemory = (BlobTeleportationHistory_t *)0x0;
    }
    (this->m_teleportationHistory).m_Memory.m_nAllocationCount = 0;
  }
  CBasePaintBlob::~CBasePaintBlob(&this->super_CBasePaintBlob,__in_chrg);
  return;
}


/* CPaintBlob::AddToPaintDatabase at 00708750 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void AddToPaintDatabase(CPaintBlob * this) */

void __thiscall CPaintBlob::AddToPaintDatabase(CPaintBlob *this)

{
  int unaff_EBX;
  trace_t local_60;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_pPaintingEntity != (CBaseEntity *)0x0) {
                    /* Unresolved local var: trace_t tr@[???] */
    local_60.super_CBaseTrace.endpos.x = (this->super_CBasePaintBlob).m_vecPosition.x;
    local_60.super_CBaseTrace.endpos.y = (this->super_CBasePaintBlob).m_vecPosition.y;
    local_60.super_CBaseTrace.endpos.z = (this->super_CBasePaintBlob).m_vecPosition.z;
    local_60.super_CBaseTrace.plane.normal.x = (this->super_CBasePaintBlob).m_vContactNormal.x;
    local_60.super_CBaseTrace.plane.normal.y = (this->super_CBasePaintBlob).m_vContactNormal.y;
    local_60.super_CBaseTrace.plane.normal.z = (this->super_CBasePaintBlob).m_vContactNormal.z;
    local_60.m_pEnt = this->m_pPaintingEntity;
    local_60.surface.flags = (ushort)*(undefined4 *)&this->m_nSurfaceFlags;
    CPaintDatabase::AddPaint
              (*(CPaintDatabase **)(unaff_EBX + 0x53e787),&local_60,
               (this->super_CBasePaintBlob).m_paintType);
    this->m_pPaintingEntity = (CBaseEntity *)0x0;
  }
  return;
}


/* CPaintBlob::SetupPaintInfo at 00708670 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void SetupPaintInfo(CPaintBlob * this, trace_t * tr) */

void __thiscall CPaintBlob::SetupPaintInfo(CPaintBlob *this,trace_t *tr)

{
  this->m_pPaintingEntity = tr->m_pEnt;
  CBasePaintBlob::SetPosition(&this->super_CBasePaintBlob,&(tr->super_CBaseTrace).endpos);
  (this->super_CBasePaintBlob).m_vContactNormal.x = (tr->super_CBaseTrace).plane.normal.x;
  (this->super_CBasePaintBlob).m_vContactNormal.y = (tr->super_CBaseTrace).plane.normal.y;
  (this->super_CBasePaintBlob).m_vContactNormal.z = (tr->super_CBaseTrace).plane.normal.z;
  this->m_nSurfaceFlags = (tr->surface).flags;
  return;
}


/* CPaintBlob::PaintBlobPaint at 00708b90 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void PaintBlobPaint(CPaintBlob * this, trace_t * tr) */

void __thiscall CPaintBlob::PaintBlobPaint(CPaintBlob *this,trace_t *tr)

{
  CBaseEntity *pCVar1;
  IPhysicsObject *pIVar2;
  int iVar3;
  int unaff_EBX;
  int iVar4;
  int iVar5;
  float fVar6;
  float fVar7;
  float local_70;
  float local_6c;
  float local_68;
  float local_64;
  float local_60;
  float local_5c;
  Vector local_58;
  Vector local_4c;
  float local_40;
  float local_3c;
  float local_38;
  Vector local_34;
  Vector local_28 [2];
  
                    /* Unresolved local var: Vector vecTouchPos@[???]
                       Unresolved local var: Vector vecNormal@[???] */
  ___i686_get_pc_thunk_bx();
  local_28[0].x = (tr->super_CBaseTrace).endpos.x;
  local_28[0].y = (tr->super_CBaseTrace).endpos.y;
  local_28[0].z = (tr->super_CBaseTrace).endpos.z;
  local_34.x = (tr->super_CBaseTrace).plane.normal.x;
  local_34.y = (tr->super_CBaseTrace).plane.normal.y;
  local_34.z = (tr->super_CBaseTrace).plane.normal.z;
  CBasePaintBlob::PlayEffect(&this->super_CBasePaintBlob,local_28,&local_34);
  this->m_pPaintingEntity = tr->m_pEnt;
  CBasePaintBlob::SetPosition(&this->super_CBasePaintBlob,&(tr->super_CBaseTrace).endpos);
  (this->super_CBasePaintBlob).m_vContactNormal.x = (tr->super_CBaseTrace).plane.normal.x;
  (this->super_CBasePaintBlob).m_vContactNormal.y = (tr->super_CBaseTrace).plane.normal.y;
  (this->super_CBasePaintBlob).m_vContactNormal.z = (tr->super_CBaseTrace).plane.normal.z;
  this->m_nSurfaceFlags = (tr->surface).flags;
  if (*(int *)(*(int *)(unaff_EBX + 0x6bf53b) + 0x30) != 0) {
    pCVar1 = tr->m_pEnt;
    iVar3 = (*(pCVar1->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[0x55])(pCVar1);
    if (((char)iVar3 == '\0') &&
       (pIVar2 = pCVar1->m_pPhysicsObject, pIVar2 != (IPhysicsObject *)0x0)) {
                    /* Unresolved local var: Vector norm@[???] */
      local_4c.x = (this->super_CBasePaintBlob).m_vecVelocity.x;
      local_4c.y = (this->super_CBasePaintBlob).m_vecVelocity.y;
      local_4c.z = (this->super_CBasePaintBlob).m_vecVelocity.z;
      VectorNormalize(&local_4c);
                    /* Unresolved local var: float flForceScale@[???]
                       Unresolved local var: Vector vecForce@[???] */
      fVar6 = *(float *)(*(int *)(&DAT_006bf4db + unaff_EBX) + 0x2c);
      fVar7 = ImpulseScale(75.0,4.0);
      fVar6 = fVar6 * fVar7;
      local_58.x = local_4c.x;
      local_58.y = local_4c.y;
      local_58.z = local_4c.z;
      VectorNormalize(&local_58);
      fVar7 = *(float *)(*(int *)(*(int *)(unaff_EBX + 0x53e9cf) + 0x1c) + 0x2c);
      local_58.x = fVar6 * local_58.x * fVar7;
      local_58.y = fVar6 * local_58.y * fVar7;
      local_58.z = fVar6 * local_58.z * fVar7;
                    /* Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
      local_68 = *(float *)(unaff_EBX + 0x3c3d73);
      local_70 = local_58.x * local_68;
      local_6c = local_58.y * local_68;
      local_68 = local_58.z * local_68;
      local_64 = local_70;
      local_60 = local_6c;
      local_5c = local_68;
      local_40 = local_70;
      local_3c = local_6c;
      local_38 = local_68;
      iVar3 = ___dynamic_cast(pCVar1,*(undefined4 *)(unaff_EBX + 0x53dfef),
                              *(undefined4 *)(unaff_EBX + 0x53e7df),0);
      if (iVar3 == 0) {
        (*pIVar2->_vptr_IPhysicsObject[0x36])(pIVar2,&local_40,&local_70);
      }
      else if (0 < *(int *)(iVar3 + 0x518)) {
        iVar5 = 0;
        iVar4 = iVar3;
        do {
          (**(code **)(**(int **)(iVar4 + 0x530) + 0xd8))
                    (*(int **)(iVar4 + 0x530),&local_40,&local_70);
          iVar5 = iVar5 + 1;
          iVar4 = iVar4 + 0x18;
        } while (iVar5 < *(int *)(iVar3 + 0x518));
        return;
      }
    }
  }
  return;
}


/* GetPaintBlobDamage at 007085a0 */

Vector * GetPaintBlobDamage(Vector *__return_storage_ptr__,Vector *vecDir)

{
  int unaff_EBX;
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float flForceScale;
  
                    /* Unresolved local var: Vector vecForce@[DW_OP_breg6(ESI): 0]
                       Unresolved local var: Vector vecForce@[???] */
  ___i686_get_pc_thunk_bx();
  fVar1 = *(float *)(*(int *)(unaff_EBX + 0x6bfacf) + 0x2c);
  fVar4 = ImpulseScale(75.0,4.0);
  fVar1 = fVar1 * fVar4;
  _memmove(__return_storage_ptr__,vecDir,0xc);
  VectorNormalize(__return_storage_ptr__);
  fVar2 = fVar1 * __return_storage_ptr__->x;
  __return_storage_ptr__->x = fVar2;
  fVar3 = fVar1 * __return_storage_ptr__->y;
  __return_storage_ptr__->y = fVar3;
  fVar1 = fVar1 * __return_storage_ptr__->z;
  __return_storage_ptr__->z = fVar1;
  fVar4 = *(float *)(*(int *)(*(int *)(&DAT_0053efc3 + unaff_EBX) + 0x1c) + 0x2c);
  __return_storage_ptr__->x = fVar2 * fVar4;
  __return_storage_ptr__->y = fVar3 * fVar4;
  __return_storage_ptr__->z = fVar1 * fVar4;
  return __return_storage_ptr__;
}


/* CPaintBlob::AddBlobTeleportationHistory at 007087d0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void AddBlobTeleportationHistory(CPaintBlob * this,
   BlobTeleportationHistory_t * history) */

void __thiscall
CPaintBlob::AddBlobTeleportationHistory(CPaintBlob *this,BlobTeleportationHistory_t *history)

{
  int *piVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  BlobTeleportationHistory_t *pBVar5;
  
  uVar2 = (this->m_teleportationHistory).m_Size;
  iVar3 = uVar2 + 1;
  iVar4 = (this->m_teleportationHistory).m_Memory.m_nAllocationCount;
  if (iVar4 < iVar3) {
    CUtlMemory<BlobTeleportationHistory_t,int>::Grow
              (&(this->m_teleportationHistory).m_Memory,iVar3 - iVar4);
  }
  piVar1 = &(this->m_teleportationHistory).m_Size;
  *piVar1 = *piVar1 + 1;
  pBVar5 = (this->m_teleportationHistory).m_Memory.m_pMemory;
  (this->m_teleportationHistory).m_pElements = pBVar5;
  iVar4 = ~uVar2 + (this->m_teleportationHistory).m_Size;
  if (0 < iVar4) {
    _V_memmove(pBVar5 + iVar3,pBVar5 + uVar2,iVar4 * 0x18);
  }
  pBVar5 = (this->m_teleportationHistory).m_Memory.m_pMemory + uVar2;
  if (pBVar5 != (BlobTeleportationHistory_t *)0x0) {
    for (iVar3 = 6; iVar3 != 0; iVar3 = iVar3 + -1) {
      (pBVar5->m_vEnterPosition).x = (history->m_vEnterPosition).x;
      history = (BlobTeleportationHistory_t *)&(history->m_vEnterPosition).y;
      pBVar5 = (BlobTeleportationHistory_t *)&(pBVar5->m_vEnterPosition).y;
    }
  }
  return;
}


/* CPaintBlob::ClearTeleportationHistory at 00708590 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ClearTeleportationHistory(CPaintBlob * this) */

void __thiscall CPaintBlob::ClearTeleportationHistory(CPaintBlob *this)

{
  (this->m_teleportationHistory).m_Size = 0;
  return;
}


/* CPaintBlob::GetTeleportationHistory at 007088a0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void GetTeleportationHistory(CPaintBlob * this,
   BlobTeleportationHistoryVector_t * historyList) */

void __thiscall
CPaintBlob::GetTeleportationHistory(CPaintBlob *this,BlobTeleportationHistoryVector_t *historyList)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  bool bVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  BlobTeleportationHistory_t *pBVar10;
  int unaff_EBX;
  BlobTeleportationHistory_t *pBVar11;
  undefined4 *puVar12;
  vec_t *pvVar13;
  int nHistoryCount;
  int local_28;
  int local_24;
  
  ___i686_get_pc_thunk_bx();
  iVar2 = (this->m_teleportationHistory).m_Size;
  if (iVar2 != 0) {
    pBVar10 = (this->m_teleportationHistory).m_Memory.m_pMemory;
    iVar8 = historyList->m_Size;
    iVar9 = iVar8 + iVar2;
    iVar7 = (historyList->m_Memory).m_nAllocationCount;
    if (iVar7 < iVar9) {
      CUtlMemory<BlobTeleportationHistory_t,int>::Grow(&historyList->m_Memory,iVar9 - iVar7);
    }
    historyList->m_Size = historyList->m_Size + iVar2;
    pBVar11 = (historyList->m_Memory).m_pMemory;
    historyList->m_pElements = pBVar11;
    iVar7 = (historyList->m_Size - iVar8) - iVar2;
    bVar6 = 0 < iVar2;
    if ((0 < iVar7) && (bVar6)) {
      _V_memmove(pBVar11 + iVar9,pBVar11 + iVar8,iVar7 * 0x18);
    }
    if (pBVar10 == (BlobTeleportationHistory_t *)0x0) {
      if (bVar6) {
        iVar8 = iVar8 * 0x18;
        local_24 = 0;
        do {
          puVar12 = (undefined4 *)
                    ((int)&(((historyList->m_Memory).m_pMemory)->m_vEnterPosition).x + iVar8);
          if (puVar12 != (undefined4 *)0x0) {
            puVar3 = *(undefined4 **)(unaff_EBX + 0x53e2ca);
            uVar4 = *puVar3;
            puVar12[3] = uVar4;
            uVar5 = puVar3[1];
            puVar12[4] = uVar5;
            uVar1 = puVar3[2];
            puVar12[5] = uVar1;
            *puVar12 = uVar4;
            puVar12[1] = uVar5;
            puVar12[2] = uVar1;
          }
          local_24 = local_24 + 1;
          iVar8 = iVar8 + 0x18;
        } while (local_24 < iVar2);
        return;
      }
    }
    else if (bVar6) {
      iVar8 = iVar8 * 0x18;
      local_28 = 0;
      do {
        pvVar13 = (vec_t *)((int)&(((historyList->m_Memory).m_pMemory)->m_vEnterPosition).x + iVar8)
        ;
        if (pvVar13 != (vec_t *)0x0) {
          pBVar11 = pBVar10;
          for (iVar9 = 6; iVar9 != 0; iVar9 = iVar9 + -1) {
            *pvVar13 = (pBVar11->m_vEnterPosition).x;
            pBVar11 = (BlobTeleportationHistory_t *)&(pBVar11->m_vEnterPosition).y;
            pvVar13 = pvVar13 + 1;
          }
        }
        local_28 = local_28 + 1;
        iVar8 = iVar8 + 0x18;
        pBVar10 = pBVar10 + 1;
      } while (local_28 < iVar2);
    }
  }
  return;
}


/* CPaintBlob::SetBlobTeleportedThisFrame at 00708560 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void SetBlobTeleportedThisFrame(CPaintBlob * this, bool bTeleported) */

void __thiscall CPaintBlob::SetBlobTeleportedThisFrame(CPaintBlob *this,bool bTeleported)

{
  this->m_bTeleportedThisFrame = bTeleported;
  return;
}


/* CPaintBlob::HasBlobTeleportedThisFrame at 00708580 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: bool HasBlobTeleportedThisFrame(CPaintBlob * this) */

bool __thiscall CPaintBlob::HasBlobTeleportedThisFrame(CPaintBlob *this)

{
  return this->m_bTeleportedThisFrame;
}


/* _GLOBAL__I_sv_paintblob_damage at 000a85e0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_sv_paintblob_damage(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

