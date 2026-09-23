/* DWARF-guided pseudocode for game/server/portal2/func_placement_clip.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* CFuncPlacementClip::~CFuncPlacementClip at 00712910 */

/* DWARF original prototype: void ~CFuncPlacementClip(CFuncPlacementClip * this, int __in_chrg) */

void __thiscall CFuncPlacementClip::~CFuncPlacementClip(CFuncPlacementClip *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5b5a87);
  CBaseEntity::~CBaseEntity(&this->super_CBaseEntity,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CFuncPlacementClip::~CFuncPlacementClip at 00712950 */

/* DWARF original prototype: void ~CFuncPlacementClip(CFuncPlacementClip * this, int __in_chrg) */

void __thiscall CFuncPlacementClip::~CFuncPlacementClip(CFuncPlacementClip *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x5b5a50);
  CBaseEntity::~CBaseEntity(&this->super_CBaseEntity,__in_chrg);
  return;
}


/* CEntityFactory<CFuncPlacementClip>::Create at 00712970 */

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
       (_func_int_varargs **)(unaff_EBX + 0x5b5a24);
  CBaseEntity::PostConstructor(this_00,pClassName);
  return &(this_00->m_Network).super_IServerNetworkable;
}


/* DataMapInit<CFuncPlacementClip> at 000a9530 */

datamap_t * DataMapInit<CFuncPlacementClip>(CFuncPlacementClip *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x48d].m_pEntity + unaff_EBX + 3)
      == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xda24b7);
    if (iVar1 != 0) {
      *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x48d].m_pPrev + unaff_EBX + 3) =
           unaff_EBX + 0xa07295;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x48e].m_pEntity + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x48e].m_SerialNumber + unaff_EBX + 3) =
           0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x48e].m_pPrev + unaff_EBX + 3) = 0;
      *(undefined4 *)
       (&UNK_000048f3 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
      *(undefined4 *)
       (&UNK_000048f7 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x48d].m_pNext + unaff_EBX + 3) = 0x12;
      ___cxa_guard_release(unaff_EBX + 0xda24b7);
      ___cxa_atexit(unaff_EBX + 0x992dbf,0,*(undefined4 *)(&DAT_00b9d613 + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_Name_00cb9b28 + unaff_EBX + 3) =
       *(undefined4 *)(&DAT_00b9d74b + unaff_EBX);
  *(undefined4 *)(&DAT_00cb9b23 + unaff_EBX) = 2;
  *(undefined **)(&DAT_00cb9b1f + unaff_EBX) = &UNK_00cb9b7f + unaff_EBX;
  return (datamap_t *)(&DAT_00cb9b1f + unaff_EBX);
}


/* __static_initialization_and_destruction_0 at 000a9600 */

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
  *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x47c].m_pPrev + unaff_EBX) = 0
  ;
  *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x47c].m_pPrev + unaff_EBX + 1)
       = 0;
  *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x47c].m_pPrev + unaff_EBX + 2)
       = 0;
  *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x47c].m_pPrev + unaff_EBX + 3)
       = 0;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x47c].m_pNext + unaff_EBX) = 0
  ;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x47d].m_pEntity + unaff_EBX) =
       0;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x47d].m_SerialNumber + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x47d].m_pPrev + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x47d].m_pNext + unaff_EBX) = 0
  ;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x47e].m_pEntity + unaff_EBX) =
       0;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x47e].m_SerialNumber + unaff_EBX) = 0;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x47e].m_pPrev + unaff_EBX) = 0
  ;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x47e].m_pNext + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x47f].m_pEntity + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x47f].m_SerialNumber + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x47f].m_pPrev + unaff_EBX) =
       0x7f7fffff;
  *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x47f].m_pNext + unaff_EBX) =
       &UNK_00ba2cfc + unaff_EBX;
  pdVar1 = DataMapInit<CFuncPlacementClip>((CFuncPlacementClip *)0x0);
  *(datamap_t **)
   ((int)DataMapInit<CAI_ScriptConditions>::dataDesc[0x1f].flatOffset + unaff_EBX + -0x2c) = pdVar1;
  *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x480].m_pEntity + unaff_EBX) =
       &UNK_00c1f0c4 + unaff_EBX;
  pIVar2 = EntityFactoryDictionary();
  (**pIVar2->_vptr_IEntityFactoryDictionary)(pIVar2,unaff_EBX + 0xda23e4,unaff_EBX + 0xa071dd);
  return;
}


/* CFuncPlacementClip::GetDataDescMap at 00712720 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CFuncPlacementClip * this) */

datamap_t * __thiscall CFuncPlacementClip::GetDataDescMap(CFuncPlacementClip *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(&UNK_00650938 + extraout_ECX);
}


/* CFuncPlacementClip::GetBaseMap at 00712730 */

datamap_t * CFuncPlacementClip::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x534554);
}


/* __tcf_0 at 00a3c300 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x40f706)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x40f6fa) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x40f706));
  }
  *(undefined4 *)(unaff_EBX + 0x40f706) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40f702)) {
    if (*(int *)(unaff_EBX + 0x40f6fa) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20a84e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20a84e),*(int *)(unaff_EBX + 0x40f6fa));
      *(undefined4 *)(unaff_EBX + 0x40f6fa) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f6fe) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40f6fa);
  *(int *)(unaff_EBX + 0x40f70a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40f702)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20a84e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20a84e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40f6fa) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f6fe) = 0;
  }
  return;
}


/* CFuncPlacementClip::Spawn at 007127d0 */

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
  CBaseEntity::SetLocalAngles(&this->super_CBaseEntity,*(QAngle **)(unaff_EBX + 0x534424));
  CBaseEntity::SetMoveType(&this->super_CBaseEntity,MOVETYPE_PUSH,MOVECOLLIDE_DEFAULT);
  pp_Var1 = (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
            _vptr_IHandleEntity;
  p_Var2 = pp_Var1[0x1b];
  iVar3 = (*pp_Var1[8])(this);
  if (iVar3 == 0) {
    iVar3 = unaff_EBX + 0x33c540;
  }
  (*p_Var2)(this,iVar3);
  CBaseEntity::AddFlag(&this->super_CBaseEntity,0x1000000);
  (*(this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x9f])(this);
  CBaseEntity::AddEffects(&this->super_CBaseEntity,0x20);
  CBaseEntity::SetCollisionGroup(&this->super_CBaseEntity,0x16);
  return;
}


/* CFuncPlacementClip::CreateVPhysics at 00712790 */

/* DWARF original prototype: bool CreateVPhysics(CFuncPlacementClip * this) */

bool __thiscall CFuncPlacementClip::CreateVPhysics(CFuncPlacementClip *this)

{
  CCollisionProperty::SetSolid
            (&(this->super_CBaseEntity).m_Collision.super_CCollisionProperty,SOLID_BSP);
  CBaseEntity::VPhysicsInitStatic(&this->super_CBaseEntity);
  return true;
}


/* CFuncPlacementClip::InputEnable at 00712740 */

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


/* CFuncPlacementClip::InputDisable at 00712880 */

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


/* _GLOBAL__I__ZN18CFuncPlacementClip9m_DataMapE at 000a96f0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN18CFuncPlacementClip9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

