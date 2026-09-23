/* DWARF-guided pseudocode for game/server/portal2/cpaintblob.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000bd860 */

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
   ((int)&paint_min_impact_sounds.super_ConCommandBase.m_pszHelpString + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&paint_min_impact_sounds.super_ConCommandBase.m_nFlags + unaff_EBX) = 0;
  *(undefined1 *)((int)&paint_min_impact_sounds.super_ConCommandBase.m_nFlags + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&paint_min_impact_sounds.super_ConCommandBase.m_nFlags + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&paint_min_impact_sounds.super_ConCommandBase.m_nFlags + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&paint_min_impact_sounds.super_IConVar._vptr_IConVar + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&paint_min_impact_sounds.m_pParent + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&paint_min_impact_sounds.m_pszDefaultValue + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&paint_min_impact_sounds.m_Value.m_pszString + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&paint_min_impact_sounds.m_Value.m_StringLength + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&paint_min_impact_sounds.m_Value.m_fValue + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&paint_min_impact_sounds.m_Value.m_nValue + unaff_EBX + 3) = 0;
  *(undefined4 *)(&paint_min_impact_sounds.field_0x37 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&paint_min_impact_sounds.m_fMinVal + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)(&paint_min_impact_sounds.field_0x3f + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&paint_min_impact_sounds.m_fMaxVal + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&paint_min_impact_sounds.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 3) = 0;
  *(undefined1 *)
   ((int)&paint_min_impact_sounds.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 2) = 0;
  *(undefined1 *)
   ((int)&paint_min_impact_sounds.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX + 3) =
       0;
  *(undefined1 *)
   ((int)&paint_min_impact_sounds.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) = 0;
  *(undefined1 *)
   ((int)&paint_min_impact_sounds.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 1) = 0;
  *(undefined4 *)
   ((int)&paint_min_impact_sounds.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&paint_min_impact_sounds.m_fnChangeCallbacks.m_pElements + unaff_EBX + 2) = 0
  ;
  *(undefined1 *)((int)&paint_min_impact_sounds.m_fnChangeCallbacks.m_Size + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&paint_min_impact_sounds.m_fnChangeCallbacks.m_pElements + unaff_EBX) = 0;
  *(undefined1 *)((int)&paint_min_impact_sounds.m_fnChangeCallbacks.m_pElements + unaff_EBX + 1) = 0
  ;
  *(undefined4 *)((int)&paint_min_impact_sounds.m_fnChangeCallbacks.m_pElements + unaff_EBX + 3) = 0
  ;
  (&DAT_00cf8dde)[unaff_EBX] = 1;
  (&DAT_00cf8ddb)[unaff_EBX] = 0;
  (&DAT_00cf8ddc)[unaff_EBX] = 0;
  (&DAT_00cf8ddd)[unaff_EBX] = 0;
  *(undefined4 *)(&DAT_00cf8ddf + unaff_EBX) = 1;
  *(undefined1 *)((int)&paint_max_impact_sounds.super_ConCommandBase.m_pNext + unaff_EBX + 2) = 0;
  *(undefined1 *)
   ((int)&paint_max_impact_sounds.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&paint_max_impact_sounds.super_ConCommandBase.m_pNext + unaff_EBX) = 0;
  *(undefined1 *)((int)&paint_max_impact_sounds.super_ConCommandBase.m_pNext + unaff_EBX + 1) = 0;
  *(undefined4 *)((int)&paint_max_impact_sounds.super_ConCommandBase.m_pNext + unaff_EBX + 3) = 2;
  *(undefined1 *)((int)&paint_max_impact_sounds.super_ConCommandBase.m_pszName + unaff_EBX + 2) = 0;
  (&paint_max_impact_sounds.super_ConCommandBase.field_0xb)[unaff_EBX] = 0;
  *(undefined1 *)((int)&paint_max_impact_sounds.super_ConCommandBase.m_pszName + unaff_EBX) = 0;
  *(undefined1 *)((int)&paint_max_impact_sounds.super_ConCommandBase.m_pszName + unaff_EBX + 1) = 0;
  *(undefined **)((int)&paint_max_impact_sounds.super_ConCommandBase.m_pszName + unaff_EBX + 3) =
       &UNK_00af3ab7 + unaff_EBX;
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<PhysBlockHeader_t>::dataDesc[2].flatOffset + unaff_EBX + -5),
                 (char *)(unaff_EBX + 0x950e43),(char *)(unaff_EBX + 0x950e3f),0x4000,
                 in_stack_ffffffe4,(char *)in_stack_ffffffe8,in_stack_ffffffec,
                 (int)in_stack_fffffff0);
  uVar1 = *(undefined4 *)(&DAT_00aee31f + unaff_EBX);
  ___cxa_atexit(unaff_EBX + 0x8db62f,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<CPointBonusMapsAccessor>::dataDesc[0].flatOffset +
                 unaff_EBX + -0x25),(char *)(unaff_EBX + 0x950e9f),(char *)(unaff_EBX + 0x8ee3c7),
                 0x6000,(char *)(unaff_EBX + 0x950e57),in_stack_ffffffe8,in_stack_ffffffec,
                 in_stack_fffffff0,flags_1,unaff_ESI);
  ___cxa_atexit(&UNK_008db60f + unaff_EBX,0,uVar1);
  return;
}


/* __tcf_0 at 00998ea0 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x395718),in_stack_00000008);
  return;
}


/* __tcf_1 at 00998e80 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x395798),in_stack_00000008);
  return;
}


/* CPaintBlob::CPaintBlob at 006afa70 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void CPaintBlob(CPaintBlob * this) */

void __thiscall CPaintBlob::CPaintBlob(CPaintBlob *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBasePaintBlob::CBasePaintBlob(&this->super_CBasePaintBlob);
  (this->super_CBasePaintBlob)._vptr_CBasePaintBlob =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4fcc5b) + 8);
  this->m_pPaintingEntity = (CBaseEntity *)0x0;
  return;
}


/* CPaintBlob::CPaintBlob at 006afab0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void CPaintBlob(CPaintBlob * this) */

void __thiscall CPaintBlob::CPaintBlob(CPaintBlob *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBasePaintBlob::CBasePaintBlob(&this->super_CBasePaintBlob);
  (this->super_CBasePaintBlob)._vptr_CBasePaintBlob =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4fcc1b) + 8);
  this->m_pPaintingEntity = (CBaseEntity *)0x0;
  return;
}


/* CPaintBlob::~CPaintBlob at 006afa30 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ~CPaintBlob(CPaintBlob * this, int __in_chrg) */

void __thiscall CPaintBlob::~CPaintBlob(CPaintBlob *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CBasePaintBlob)._vptr_CBasePaintBlob =
       (_func_int_varargs **)(*(int *)(&DAT_004fcca4 + extraout_ECX) + 8);
  CBasePaintBlob::~CBasePaintBlob(&this->super_CBasePaintBlob,__in_chrg);
  return;
}


/* CPaintBlob::~CPaintBlob at 006afa50 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ~CPaintBlob(CPaintBlob * this, int __in_chrg) */

void __thiscall CPaintBlob::~CPaintBlob(CPaintBlob *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CBasePaintBlob)._vptr_CBasePaintBlob =
       (_func_int_varargs **)(*(int *)(extraout_ECX + 0x4fcc84) + 8);
  CBasePaintBlob::~CBasePaintBlob(&this->super_CBasePaintBlob,__in_chrg);
  return;
}


/* CPaintBlob::AddToPaintDatabase at 006afaf0 */

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
              (*(CPaintDatabase **)(unaff_EBX + 0x4fc413),&local_60,
               (this->super_CBasePaintBlob).m_paintType);
    this->m_pPaintingEntity = (CBaseEntity *)0x0;
  }
  return;
}


/* CPaintBlob::SetupPaintInfo at 006af9d0 */

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


/* CPaintBlob::PaintBlobPaint at 006afb70 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
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
  if (*(int *)(*(int *)(unaff_EBX + 0x67eabb) + 0x30) != 0) {
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
      fVar6 = *(float *)(*(int *)(unaff_EBX + 0x67ea5b) + 0x2c);
      fVar7 = ImpulseScale(75.0,4.0);
      fVar6 = fVar6 * fVar7;
      local_58.x = local_4c.x;
      local_58.y = local_4c.y;
      local_58.z = local_4c.z;
      VectorNormalize(&local_58);
      fVar7 = *(float *)(*(int *)(*(int *)(unaff_EBX + 0x4fca37) + 0x1c) + 0x2c);
      local_58.x = fVar6 * local_58.x * fVar7;
      local_58.y = fVar6 * local_58.y * fVar7;
      local_58.z = fVar6 * local_58.z * fVar7;
                    /* Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
      local_68 = *(float *)(unaff_EBX + 0x37a793);
      local_70 = local_58.x * local_68;
      local_6c = local_58.y * local_68;
      local_68 = local_58.z * local_68;
      local_64 = local_70;
      local_60 = local_6c;
      local_5c = local_68;
      local_40 = local_70;
      local_3c = local_6c;
      local_38 = local_68;
      iVar3 = ___dynamic_cast(pCVar1,*(undefined4 *)
                                      (CNPC_FloorTurret::InactiveThink + unaff_EBX + 7),
                              *(undefined4 *)(&DAT_004fc82b + unaff_EBX),0);
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


/* GetPaintBlobDamage at 006af900 */

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
  fVar1 = *(float *)(*(int *)(unaff_EBX + 0x67eccf) + 0x2c);
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
  fVar4 = *(float *)(*(int *)(*(int *)(&DAT_004fccab + unaff_EBX) + 0x1c) + 0x2c);
  __return_storage_ptr__->x = fVar2 * fVar4;
  __return_storage_ptr__->y = fVar3 * fVar4;
  __return_storage_ptr__->z = fVar1 * fVar4;
  return __return_storage_ptr__;
}


/* _GLOBAL__I_sv_paintblob_damage at 000bda40 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_sv_paintblob_damage(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

