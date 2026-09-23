/* DWARF-guided pseudocode for game/server/portal2/func_placement_clip.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* CFuncPlacementClip::~CFuncPlacementClip at 006b68c0 */

/* DWARF original prototype: void ~CFuncPlacementClip(CFuncPlacementClip * this, int __in_chrg) */

void __thiscall CFuncPlacementClip::~CFuncPlacementClip(CFuncPlacementClip *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x579517);
  CBaseEntity::~CBaseEntity(&this->super_CBaseEntity,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CFuncPlacementClip::~CFuncPlacementClip at 006b6900 */

/* DWARF original prototype: void ~CFuncPlacementClip(CFuncPlacementClip * this, int __in_chrg) */

void __thiscall CFuncPlacementClip::~CFuncPlacementClip(CFuncPlacementClip *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x5794e0);
  CBaseEntity::~CBaseEntity(&this->super_CBaseEntity,__in_chrg);
  return;
}


/* CEntityFactory<CFuncPlacementClip>::Create at 006b6920 */

/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CFuncPlacementClip> * this,
   char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CFuncPlacementClip>::Create
          (CEntityFactory<CFuncPlacementClip> *this,char *pClassName)

{
  CBaseEntity *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CFuncPlacementClip * pEnt@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CBaseEntity::operator_new(0x3c0);
  CBaseEntity::CBaseEntity(this_00,false);
  (this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x5794b4);
  CBaseEntity::PostConstructor(this_00,pClassName);
  return &(this_00->m_Network).super_IServerNetworkable;
}


/* DataMapInit<CFuncPlacementClip> at 000be6e0 */

datamap_t * DataMapInit<CFuncPlacementClip>(CFuncPlacementClip *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&paintblob_tbeam_speed.super_ConCommandBase.m_pszName + unaff_EBX + 3) == '\0')
  {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf81cf);
    if (iVar1 != 0) {
      *(int *)((int)&paintblob_tbeam_speed.super_ConCommandBase.m_nFlags + unaff_EBX + 3) =
           unaff_EBX + 0x9502e9;
      *(undefined4 *)((int)&paintblob_tbeam_speed.m_pParent + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&paintblob_tbeam_speed.m_pszDefaultValue + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&paintblob_tbeam_speed.m_Value.m_pszString + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&paintblob_tbeam_speed.m_Value.m_StringLength + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&paintblob_tbeam_speed.m_Value.m_fValue + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&paintblob_tbeam_speed.super_IConVar._vptr_IConVar + unaff_EBX + 3) =
           0x12;
      ___cxa_guard_release(unaff_EBX + 0xcf81cf);
      ___cxa_atexit(&UNK_008daabf + unaff_EBX,0,*(undefined4 *)(&DAT_00aed49f + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_Save_00c0acb8 + unaff_EBX + 3) =
       *(undefined4 *)(&DAT_00aed5db + unaff_EBX);
  *(undefined4 *)((int)&PTR_DrawDebugGeometryOverlays_00c0acb0 + unaff_EBX + 3) = 2;
  *(int *)((int)&PTR_GetPlayerName_00c0acac + unaff_EBX + 3) = unaff_EBX + 0xc0ad0f;
  return (datamap_t *)((int)&PTR_GetPlayerName_00c0acac + unaff_EBX + 3);
}


/* __static_initialization_and_destruction_0 at 000be7b0 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  datamap_t *pdVar1;
  IEntityFactoryDictionary *pIVar2;
  int unaff_EBX;
  longlong lVar3;
  
  lVar3 = ___i686_get_pc_thunk_bx();
  if (lVar3 != 0xffff00000001) {
    return;
  }
  (&DAT_00cf809c)[unaff_EBX] = 0;
  (&DAT_00cf809d)[unaff_EBX] = 0;
  (&DAT_00cf809e)[unaff_EBX] = 0;
  (&DAT_00cf809f)[unaff_EBX] = 0;
  *(undefined4 *)
   ((int)&paintblob_radius_while_streaking.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) =
       0;
  *(undefined4 *)((int)&paintblob_radius_while_streaking.super_ConCommandBase.m_pNext + unaff_EBX) =
       0;
  *(undefined4 *)(&paintblob_radius_while_streaking.super_ConCommandBase.m_bRegistered + unaff_EBX)
       = 0x7f7fffff;
  *(undefined4 *)((int)&paintblob_radius_while_streaking.super_ConCommandBase.m_pszName + unaff_EBX)
       = 0x7f7fffff;
  *(undefined4 *)
   ((int)&paintblob_radius_while_streaking.super_ConCommandBase.m_pszHelpString + unaff_EBX) = 0;
  *(undefined4 *)((int)&paintblob_radius_while_streaking.super_ConCommandBase.m_nFlags + unaff_EBX)
       = 0;
  *(undefined4 *)((int)&paintblob_radius_while_streaking.super_IConVar._vptr_IConVar + unaff_EBX) =
       0;
  *(undefined4 *)((int)&paintblob_radius_while_streaking.m_pParent + unaff_EBX) = 0;
  *(undefined4 *)((int)&paintblob_radius_while_streaking.m_pszDefaultValue + unaff_EBX) = 0x7f7fffff
  ;
  *(undefined4 *)((int)&paintblob_radius_while_streaking.m_Value.m_pszString + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&paintblob_radius_while_streaking.m_Value.m_StringLength + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&paintblob_radius_while_streaking.m_Value.m_fValue + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&paintblob_radius_while_streaking.m_Value.m_nValue + unaff_EBX) = 0;
  (&paintblob_radius_while_streaking.field_0x37)[unaff_EBX] = 0;
  (&paintblob_radius_while_streaking.m_bHasMin)[unaff_EBX] = false;
  (&paintblob_radius_while_streaking.field_0x35)[unaff_EBX] = 0;
  (&paintblob_radius_while_streaking.field_0x36)[unaff_EBX] = 0;
  *(undefined4 *)((int)&paintblob_radius_while_streaking.m_fMinVal + unaff_EBX) = 0;
  (&paintblob_radius_while_streaking.field_0x3f)[unaff_EBX] = 0;
  (&paintblob_radius_while_streaking.m_bHasMax)[unaff_EBX] = false;
  (&paintblob_radius_while_streaking.field_0x3d)[unaff_EBX] = 0;
  (&paintblob_radius_while_streaking.field_0x3e)[unaff_EBX] = 0;
  *(undefined4 *)((int)&paintblob_radius_while_streaking.m_fMaxVal + unaff_EBX) = 0;
  *(undefined1 *)
   ((int)&paintblob_radius_while_streaking.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 3) =
       1;
  *(undefined1 *)
   ((int)&paintblob_radius_while_streaking.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) = 0;
  *(undefined1 *)
   ((int)&paintblob_radius_while_streaking.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 1) =
       0;
  *(undefined1 *)
   ((int)&paintblob_radius_while_streaking.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 2) =
       0;
  *(undefined4 *)
   ((int)&paintblob_radius_while_streaking.m_fnChangeCallbacks.m_Memory.m_nAllocationCount +
   unaff_EBX) = 1;
  *(undefined1 *)
   ((int)&paintblob_radius_while_streaking.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 3)
       = 0;
  *(undefined1 *)
   ((int)&paintblob_radius_while_streaking.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) = 0
  ;
  *(undefined1 *)
   ((int)&paintblob_radius_while_streaking.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 1)
       = 0;
  *(undefined1 *)
   ((int)&paintblob_radius_while_streaking.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 2)
       = 0;
  *(undefined4 *)((int)&paintblob_radius_while_streaking.m_fnChangeCallbacks.m_Size + unaff_EBX) = 2
  ;
  *(undefined1 *)
   ((int)&paintblob_radius_while_streaking.m_fnChangeCallbacks.m_pElements + unaff_EBX + 3) = 0;
  *(undefined1 *)
   ((int)&paintblob_radius_while_streaking.m_fnChangeCallbacks.m_pElements + unaff_EBX) = 0;
  *(undefined1 *)
   ((int)&paintblob_radius_while_streaking.m_fnChangeCallbacks.m_pElements + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&paintblob_radius_while_streaking.m_fnChangeCallbacks.m_pElements + unaff_EBX + 2) = 0;
  *(undefined **)(&DAT_00cf80f8 + unaff_EBX) = &UNK_00af2b6c + unaff_EBX;
  pdVar1 = DataMapInit<CFuncPlacementClip>((CFuncPlacementClip *)0x0);
  *(datamap_t **)((int)&CFuncLadder::m_DataMap.m_pOptimizedDataMap + unaff_EBX) = pdVar1;
  *(undefined **)(&DAT_00cf80fc + unaff_EBX) = &UNK_00b71954 + unaff_EBX;
  pIVar2 = EntityFactoryDictionary();
  (**pIVar2->_vptr_IEntityFactoryDictionary)(pIVar2,&DAT_00cf80fc + unaff_EBX,unaff_EBX + 0x950231);
  return;
}


/* CFuncPlacementClip::GetDataDescMap at 006b66d0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CFuncPlacementClip * this) */

datamap_t * __thiscall CFuncPlacementClip::GetDataDescMap(CFuncPlacementClip *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x612cc8);
}


/* CFuncPlacementClip::GetBaseMap at 006b66e0 */

datamap_t * CFuncPlacementClip::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4f55e4);
}


/* __tcf_0 at 009991b0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41d71e)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41d712) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41d71e));
  }
  *(undefined4 *)(unaff_EBX + 0x41d71e) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41d71a)) {
    if (*(int *)(unaff_EBX + 0x41d712) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2129da) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2129da),*(int *)(unaff_EBX + 0x41d712));
      *(undefined4 *)(unaff_EBX + 0x41d712) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d716) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41d712);
  *(int *)(unaff_EBX + 0x41d722) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41d71a)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2129da) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2129da),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41d712) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d716) = 0;
  }
  return;
}


/* CFuncPlacementClip::Spawn at 006b6780 */

/* WARNING: Enum "MoveCollide_t": Some values do not have unique names */
/* WARNING: Enum "MoveType_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(CFuncPlacementClip * this) */

void __thiscall CFuncPlacementClip::Spawn(CFuncPlacementClip *this)

{
  _func_int_varargs **pp_Var1;
  _func_int_varargs *p_Var2;
  int iVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::SetLocalAngles(&this->super_CBaseEntity,*(QAngle **)(unaff_EBX + 0x4f54b0));
  CBaseEntity::SetMoveType(&this->super_CBaseEntity,MOVETYPE_PUSH,MOVECOLLIDE_DEFAULT);
  pp_Var1 = (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
            _vptr_IHandleEntity;
  p_Var2 = pp_Var1[0x1b];
  iVar3 = (*pp_Var1[8])(this);
  if (iVar3 == 0) {
    iVar3 = unaff_EBX + 0x2f53c8;
  }
  (*p_Var2)(this,iVar3);
  CBaseEntity::AddFlag(&this->super_CBaseEntity,0x1000000);
  (*(this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x9f])(this);
  CBaseEntity::AddEffects(&this->super_CBaseEntity,0x20);
  CBaseEntity::SetCollisionGroup(&this->super_CBaseEntity,0x16);
  return;
}


/* CFuncPlacementClip::CreateVPhysics at 006b6740 */

/* DWARF original prototype: bool CreateVPhysics(CFuncPlacementClip * this) */

bool __thiscall CFuncPlacementClip::CreateVPhysics(CFuncPlacementClip *this)

{
  CCollisionProperty::SetSolid
            (&(this->super_CBaseEntity).m_Collision.super_CCollisionProperty,SOLID_BSP);
  CBaseEntity::VPhysicsInitStatic(&this->super_CBaseEntity);
  return true;
}


/* CFuncPlacementClip::InputEnable at 006b66f0 */

/* DWARF original prototype: void InputEnable(CFuncPlacementClip * this, inputdata_t * data) */

void __thiscall CFuncPlacementClip::InputEnable(CFuncPlacementClip *this,inputdata_t *data)

{
  IPhysicsObject *pIVar1;
  
                    /* Unresolved local var: IPhysicsObject * pPhys@[???] */
  pIVar1 = (this->super_CBaseEntity).m_pPhysicsObject;
  if (pIVar1 != (IPhysicsObject *)0x0) {
    (*pIVar1->_vptr_IPhysicsObject[0xd])(pIVar1,1);
  }
  CCollisionProperty::SetSolidFlags
            (&(this->super_CBaseEntity).m_Collision.super_CCollisionProperty,
             (this->super_CBaseEntity).m_Collision.super_CCollisionProperty.m_usSolidFlags.m_Value &
             0xfffffffb);
  return;
}


/* CFuncPlacementClip::InputDisable at 006b6830 */

/* DWARF original prototype: void InputDisable(CFuncPlacementClip * this, inputdata_t * data) */

void __thiscall CFuncPlacementClip::InputDisable(CFuncPlacementClip *this,inputdata_t *data)

{
  IPhysicsObject *pIVar1;
  
                    /* Unresolved local var: IPhysicsObject * pPhys@[???] */
  pIVar1 = (this->super_CBaseEntity).m_pPhysicsObject;
  if (pIVar1 != (IPhysicsObject *)0x0) {
    (*pIVar1->_vptr_IPhysicsObject[0xd])(pIVar1,0);
  }
  CCollisionProperty::SetSolidFlags
            (&(this->super_CBaseEntity).m_Collision.super_CCollisionProperty,
             (this->super_CBaseEntity).m_Collision.super_CCollisionProperty.m_usSolidFlags.m_Value |
             4);
  return;
}


/* _GLOBAL__I__ZN18CFuncPlacementClip9m_DataMapE at 000be940 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN18CFuncPlacementClip9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

