/* DWARF-guided pseudocode for game/server/portal2/info_coop_spawn.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* CInfo_Coop_Spawn::~CInfo_Coop_Spawn at 00717da0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CInfo_Coop_Spawn(CInfo_Coop_Spawn * this, int __in_chrg) */

void __thiscall CInfo_Coop_Spawn::~CInfo_Coop_Spawn(CInfo_Coop_Spawn *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5b1877);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CInfo_Coop_Spawn::~CInfo_Coop_Spawn at 00717de0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CInfo_Coop_Spawn(CInfo_Coop_Spawn * this, int __in_chrg) */

void __thiscall CInfo_Coop_Spawn::~CInfo_Coop_Spawn(CInfo_Coop_Spawn *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x5b1840);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
  return;
}


/* CEntityFactory<CInfo_Coop_Spawn>::Create at 00717e00 */

/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CInfo_Coop_Spawn> * this,
   char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CInfo_Coop_Spawn>::Create(CEntityFactory<CInfo_Coop_Spawn> *this,char *pClassName)

{
  CBaseAnimating *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CInfo_Coop_Spawn * pEnt@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CBaseEntity::operator_new(0x520);
  CBaseAnimating::CBaseAnimating(this_00);
  (this_00->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5b1814);
  CBaseEntity::PostConstructor(&this_00->super_CBaseEntity,pClassName);
  return &(this_00->super_CBaseEntity).m_Network.super_IServerNetworkable;
}


/* DataMapInit<CInfo_Coop_Spawn> at 000aa390 */

datamap_t * DataMapInit<CInfo_Coop_Spawn>(CInfo_Coop_Spawn *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3f9].m_pPrev + unaff_EBX + 3)
      == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xda1b7f);
    if (iVar1 != 0) {
      *(undefined **)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3fa].m_pEntity + unaff_EBX + 3) =
           &UNK_00a0678f + unaff_EBX;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3fa].m_pPrev + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3fa].m_pNext + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3fb].m_pEntity + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3fb].m_SerialNumber + unaff_EBX + 3) =
           0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3fb].m_pPrev + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3fa].m_SerialNumber + unaff_EBX + 3) =
           0x10;
      ___cxa_guard_release(unaff_EBX + 0xda1b7f);
      ___cxa_atexit(unaff_EBX + 0x99249f,0,*(undefined4 *)(&DAT_00b9c7b3 + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR__CPointTemplate_00cb97a8 + unaff_EBX + 3) =
       *(undefined4 *)(&DAT_00b9c7bf + unaff_EBX);
  *(undefined4 *)(&DAT_00cb97a3 + unaff_EBX) = 6;
  *(int *)(&DAT_00cb979f + unaff_EBX) = unaff_EBX + 0xcb97ff;
  return (datamap_t *)(&DAT_00cb979f + unaff_EBX);
}


/* __static_initialization_and_destruction_0 at 000aa460 */

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
  *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[1000].m_pEntity + unaff_EBX) =
       0;
  *(undefined1 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[1000].m_pEntity + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[1000].m_pEntity + unaff_EBX + 2) = 0;
  *(undefined1 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[1000].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[1000].m_SerialNumber + unaff_EBX) = 0;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[1000].m_pPrev + unaff_EBX) = 0;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[1000].m_pNext + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3e9].m_pEntity + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3e9].m_SerialNumber + unaff_EBX) = 0;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3e9].m_pPrev + unaff_EBX) = 0
  ;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3e9].m_pNext + unaff_EBX) = 0
  ;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3ea].m_pEntity + unaff_EBX) =
       0;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3ea].m_SerialNumber + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3ea].m_pPrev + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3ea].m_pNext + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3eb].m_pEntity + unaff_EBX) =
       0x7f7fffff;
  *(undefined **)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3eb].m_SerialNumber + unaff_EBX) =
       &UNK_00ba1e9c + unaff_EBX;
  pdVar1 = DataMapInit<CInfo_Coop_Spawn>((CInfo_Coop_Spawn *)0x0);
  *(datamap_t **)((int)DataMapInit<CAI_Path>::dataDesc[5].flatOffset + unaff_EBX + -0x20) = pdVar1;
  *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3eb].m_pPrev + unaff_EBX) =
       &UNK_00c1f584 + unaff_EBX;
  pIVar2 = EntityFactoryDictionary();
  (**pIVar2->_vptr_IEntityFactoryDictionary)(pIVar2,unaff_EBX + 0xda1a9c,unaff_EBX + 0xa066d5);
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3eb].m_pNext + unaff_EBX) = 0
  ;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3ec].m_pEntity + unaff_EBX) =
       0;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3ec].m_SerialNumber + unaff_EBX) = 0;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3ec].m_pPrev + unaff_EBX) = 0
  ;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3ec].m_pNext + unaff_EBX) = 0
  ;
  ___cxa_atexit(unaff_EBX + 0x9924b4,0,*(undefined4 *)(&DAT_00b9c6e8 + unaff_EBX));
  return;
}


/* CInfo_Coop_Spawn::GetDataDescMap at 00717850 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CInfo_Coop_Spawn * this) */

datamap_t * __thiscall CInfo_Coop_Spawn::GetDataDescMap(CInfo_Coop_Spawn *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x64c2e8);
}


/* CInfo_Coop_Spawn::GetBaseMap at 00717860 */

datamap_t * CInfo_Coop_Spawn::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(&DAT_0052f2f8 + extraout_ECX);
}


/* __tcf_0 at 00a3c840 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x40f6ee)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x40f6e2) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x40f6ee));
  }
  *(undefined4 *)(unaff_EBX + 0x40f6ee) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40f6ea)) {
    if (*(int *)(unaff_EBX + 0x40f6e2) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20a30e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20a30e),*(int *)(unaff_EBX + 0x40f6e2));
      *(undefined4 *)(unaff_EBX + 0x40f6e2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f6e6) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40f6e2);
  *(int *)(unaff_EBX + 0x40f6f2) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40f6ea)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20a30e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20a30e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40f6e2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f6e6) = 0;
  }
  return;
}


/* __tcf_1 at 00a3c920 */

void __tcf_1(void *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  *(undefined4 *)(unaff_EBX + 0x40f5e4) = 0;
  if (*(int *)(unaff_EBX + 0x40f5e0) < 0) {
    iVar1 = *(int *)(unaff_EBX + 0x40f5d8);
  }
  else {
    if (*(int *)(unaff_EBX + 0x40f5d8) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20a228) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20a228),*(int *)(unaff_EBX + 0x40f5d8));
      *(undefined4 *)(unaff_EBX + 0x40f5d8) = 0;
    }
    iVar1 = 0;
    *(undefined4 *)(unaff_EBX + 0x40f5dc) = 0;
  }
  *(int *)(unaff_EBX + 0x40f5e8) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40f5e0)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20a228) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20a228),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40f5d8) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f5dc) = 0;
  }
  return;
}


/* CInfo_Coop_Spawn::Activate at 00717b20 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Activate(CInfo_Coop_Spawn * this) */

void __thiscall CInfo_Coop_Spawn::Activate(CInfo_Coop_Spawn *this)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined4 *puVar4;
  int iVar5;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  iVar2 = *(int *)(&DAT_007343e4 + unaff_EBX);
  iVar1 = iVar2 + 1;
  iVar3 = iVar1;
  if (*(int *)(&DAT_007343dc + unaff_EBX) < iVar1) {
    CUtlMemory<CInfo_Coop_Spawn*,int>::Grow
              ((CUtlMemory<CInfo_Coop_Spawn*,int> *)(&DAT_007343d8 + unaff_EBX),
               iVar1 - *(int *)(&DAT_007343dc + unaff_EBX));
    iVar3 = *(int *)(&DAT_007343e4 + unaff_EBX) + 1;
  }
  *(int *)(&DAT_007343e4 + unaff_EBX) = iVar3;
  iVar5 = *(int *)(&DAT_007343d8 + unaff_EBX);
  *(int *)(&DAT_007343e8 + unaff_EBX) = iVar5;
  iVar3 = (iVar3 - iVar2) + -1;
  if (0 < iVar3) {
    _V_memmove((void *)(iVar5 + iVar1 * 4),(void *)(iVar5 + iVar2 * 4),iVar3 * 4);
    iVar5 = *(int *)(&DAT_007343d8 + unaff_EBX);
  }
  puVar4 = (undefined4 *)(iVar2 * 4 + iVar5);
  if (puVar4 != (undefined4 *)0x0) {
    *puVar4 = this;
  }
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x5e])(this,this->m_iStartingTeam);
  CBaseAnimating::Activate(&this->super_CBaseAnimating);
  return;
}


/* CInfo_Coop_Spawn::InputEnable at 00717870 */

/* DWARF original prototype: void InputEnable(CInfo_Coop_Spawn * this, inputdata_t * inputdata) */

void __thiscall CInfo_Coop_Spawn::InputEnable(CInfo_Coop_Spawn *this,inputdata_t *inputdata)

{
  this->m_bEnabled = true;
  return;
}


/* CInfo_Coop_Spawn::InputDisable at 00717880 */

/* DWARF original prototype: void InputDisable(CInfo_Coop_Spawn * this, inputdata_t * inputdata) */

void __thiscall CInfo_Coop_Spawn::InputDisable(CInfo_Coop_Spawn *this,inputdata_t *inputdata)

{
  this->m_bEnabled = false;
  return;
}


/* CInfo_Coop_Spawn::InputSetAsActiveSpawn at 00717c00 */

/* DWARF original prototype: void InputSetAsActiveSpawn(CInfo_Coop_Spawn * this, inputdata_t *
   inputdata) */

void __thiscall
CInfo_Coop_Spawn::InputSetAsActiveSpawn(CInfo_Coop_Spawn *this,inputdata_t *inputdata)

{
  int iVar1;
  int iVar2;
  CInfo_Coop_Spawn *pCVar3;
  CBaseEntity *this_00;
  int unaff_EBX;
  int iVar4;
  int iKnownLocations;
  CInfo_Coop_Spawn **pKnownLocations;
  
  ___i686_get_pc_thunk_bx();
  iVar1 = *(int *)(unaff_EBX + 0x73430a);
  iVar2 = *(int *)(unaff_EBX + 0x7342fe);
  if (iVar1 != 0) {
    iVar4 = 0;
LAB_00717c4a:
    do {
      pCVar3 = *(CInfo_Coop_Spawn **)(iVar2 + iVar4 * 4);
      if (pCVar3 == (CInfo_Coop_Spawn *)0x0) {
        _Warning(unaff_EBX + 0x398f46);
      }
      else if (((pCVar3 != this) && (pCVar3->m_iStartingTeam == this->m_iStartingTeam)) &&
              (pCVar3->m_bEnabled = false,
              (this->super_CBaseAnimating).super_CBaseEntity.m_debugOverlays != 0)) {
        this_00 = *(CBaseEntity **)(iVar2 + iVar4 * 4);
        if ((this_00->m_iEFlags & 0x800) != 0) {
          CBaseEntity::CalcAbsolutePosition(this_00);
        }
        NDebugOverlay::Cross3D(&this_00->m_vecAbsOrigin,4.0,0xff,0,0,true,2.0);
        iVar4 = iVar4 + 1;
        if (iVar4 == iVar1) break;
        goto LAB_00717c4a;
      }
      iVar4 = iVar4 + 1;
    } while (iVar4 != iVar1);
  }
  if ((this->super_CBaseAnimating).super_CBaseEntity.m_debugOverlays != 0) {
    if (((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
    NDebugOverlay::Cross3D
              (&(this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin,4.0,0,0xff,0,true,2.0)
    ;
  }
  this->m_bEnabled = true;
  return;
}


/* CInfo_Coop_Spawn::UpdateOnRemove at 00717970 */

/* DWARF original prototype: void UpdateOnRemove(CInfo_Coop_Spawn * this) */

void __thiscall CInfo_Coop_Spawn::UpdateOnRemove(CInfo_Coop_Spawn *this)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  iVar1 = *(int *)(&DAT_0073459a + unaff_EBX);
  if (0 < iVar1) {
    piVar2 = *(int **)(&DAT_0073458e + unaff_EBX);
    if (this == (CInfo_Coop_Spawn *)*piVar2) {
      iVar4 = 0;
      iVar5 = 0;
    }
    else {
      iVar4 = 0;
      iVar3 = 4;
      do {
        iVar5 = iVar3;
        iVar4 = iVar4 + 1;
        if (iVar4 == iVar1) goto LAB_007179c6;
        iVar3 = iVar5 + 4;
      } while (this != (CInfo_Coop_Spawn *)piVar2[iVar4]);
    }
    if (iVar4 != iVar1 + -1) {
      *(int *)(iVar5 + (int)piVar2) = piVar2[iVar1 + -1];
      iVar4 = *(int *)(&DAT_0073459a + unaff_EBX) + -1;
    }
    *(int *)(&DAT_0073459a + unaff_EBX) = iVar4;
  }
LAB_007179c6:
  CBaseEntity::UpdateOnRemove((CBaseEntity *)this);
  return;
}


/* GetCoopSpawnLocation at 00717a00 */

/* WARNING: Struct "CPortalMPGameRules": ignoring overlapping field "m_bTeamPlayEnabled" */

CBaseEntity * GetCoopSpawnLocation(int iTeam)

{
  int iVar1;
  bool bVar2;
  int iVar3;
  int iVar4;
  CBaseEntity *pCVar5;
  int unaff_EBX;
  int iVar6;
  int aiStack_50 [5];
  undefined1 auStack_3c [4];
  undefined1 *local_38;
  int iKnownLocations;
  int iValidLocations;
  int local_20;
  
                    /* Unresolved local var: CInfo_Coop_Spawn * * pKnownLocations@[???]
                       Unresolved local var: CInfo_Coop_Spawn * * pValidLocations@[???]
                       Unresolved local var: CInfo_Coop_Spawn * pValidSpawn@[???] */
  aiStack_50[0] = 0x717a0e;
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(unaff_EBX + 0x52f1a2);
  iKnownLocations = *(int *)(&DAT_0073450a + unaff_EBX);
  iVar4 = *(int *)(&DAT_007344fe + unaff_EBX);
  iVar1 = -((iKnownLocations * 4 + 0xfU & 0xfffffff0) + 0x10);
  local_38 = auStack_3c + iVar1;
                    /* Unresolved local var: int i@[???] */
  if (iKnownLocations != 0) {
    iValidLocations = 0;
    iVar6 = 0;
    do {
      while (iVar3 = *(int *)(iVar4 + iVar6 * 4), *(char *)(iVar3 + 0x514) == '\0') {
LAB_00717a60:
        iVar6 = iVar6 + 1;
        if (iVar6 == iKnownLocations) goto LAB_00717aa6;
      }
      *(int *)((int)aiStack_50 + iVar1 + 4) = iVar3;
      *(undefined4 *)((int)aiStack_50 + iVar1) = 0x717a7a;
      iVar3 = CBaseEntity::GetTeamNumber(*(CBaseEntity **)((int)aiStack_50 + iVar1 + 4));
      if (iVar3 != iTeam) {
        *(undefined4 *)((int)aiStack_50 + iVar1 + 4) = *(undefined4 *)(iVar4 + iVar6 * 4);
        *(undefined4 *)((int)aiStack_50 + iVar1) = 0x717a8a;
        iVar3 = CBaseEntity::GetTeamNumber(*(CBaseEntity **)((int)aiStack_50 + iVar1 + 4));
        if (1 < iVar3) goto LAB_00717a60;
      }
      *(undefined4 *)(local_38 + iValidLocations * 4) = *(undefined4 *)(iVar4 + iVar6 * 4);
      iValidLocations = iValidLocations + 1;
      iVar6 = iVar6 + 1;
    } while (iVar6 != iKnownLocations);
LAB_00717aa6:
    if (iValidLocations != 0) {
      *(int *)((int)aiStack_50 + iVar1 + 8) = iValidLocations + -1;
      *(undefined4 *)((int)aiStack_50 + iVar1 + 4) = 0;
      *(undefined4 *)((int)aiStack_50 + iVar1) = 0x717ac1;
      iVar4 = _RandomInt();
      pCVar5 = *(CBaseEntity **)(local_38 + iVar4 * 4);
      if ((char)pCVar5[1].m_DamageModifiers.m_LastAlloc.index != '\0') {
        *(undefined4 *)((int)aiStack_50 + iVar1 + 4) = **(undefined4 **)(unaff_EBX + 0x52f192);
        *(undefined4 *)((int)aiStack_50 + iVar1) = 0x717afb;
        bVar2 = CPortalMPGameRules::Is2GunsCoOp
                          (*(CPortalMPGameRules **)((int)aiStack_50 + iVar1 + 4));
        if (bVar2) {
          **(undefined4 **)(unaff_EBX + 0x52fc86) = 3;
        }
      }
      goto LAB_00717ad6;
    }
  }
  pCVar5 = (CBaseEntity *)0x0;
LAB_00717ad6:
  if (local_20 != **(int **)(unaff_EBX + 0x52f1a2)) {
                    /* WARNING: Subroutine does not return */
    *(undefined **)((int)aiStack_50 + iVar1) = &UNK_00717b14;
    ___stack_chk_fail();
  }
  return pCVar5;
}


/* CInfo_Coop_Spawn::DrawDebugTextOverlays at 00717890 */

/* DWARF original prototype: int DrawDebugTextOverlays(CInfo_Coop_Spawn * this) */

int __thiscall CInfo_Coop_Spawn::DrawDebugTextOverlays(CInfo_Coop_Spawn *this)

{
  int text_offset_00;
  int iVar1;
  int unaff_EBX;
  int text_offset;
  char local_11f [255];
  int local_20;
  
                    /* Unresolved local var: char[255] tempstr@[???] */
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(&DAT_0052f309 + unaff_EBX);
  text_offset_00 = CBaseAnimating::DrawDebugTextOverlays(&this->super_CBaseAnimating);
  iVar1 = unaff_EBX + 0x399275;
  if (this->m_bEnabled == false) {
    iVar1 = unaff_EBX + 0x39927d;
  }
  V_snprintf(local_11f,0xff,(char *)(unaff_EBX + 0x33f3bd),iVar1);
  CBaseEntity::EntityText((CBaseEntity *)this,text_offset_00,local_11f,0.0,0xff,0xff,0xff,0xff);
  if (local_20 == **(int **)(&DAT_0052f309 + unaff_EBX)) {
    return text_offset_00 + 1;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}


/* _GLOBAL__I__ZN16CInfo_Coop_Spawn9m_DataMapE at 000aa590 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN16CInfo_Coop_Spawn9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

