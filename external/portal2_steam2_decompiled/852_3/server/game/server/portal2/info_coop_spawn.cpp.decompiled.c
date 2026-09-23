/* DWARF-guided pseudocode for game/server/portal2/info_coop_spawn.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* CInfo_Coop_Spawn::~CInfo_Coop_Spawn at 006b9860 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CInfo_Coop_Spawn(CInfo_Coop_Spawn * this, int __in_chrg) */

void __thiscall CInfo_Coop_Spawn::~CInfo_Coop_Spawn(CInfo_Coop_Spawn *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5777f7);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CInfo_Coop_Spawn::~CInfo_Coop_Spawn at 006b98a0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CInfo_Coop_Spawn(CInfo_Coop_Spawn * this, int __in_chrg) */

void __thiscall CInfo_Coop_Spawn::~CInfo_Coop_Spawn(CInfo_Coop_Spawn *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x5777c0);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
  return;
}


/* CEntityFactory<CInfo_Coop_Spawn>::Create at 006b98c0 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x577794);
  CBaseEntity::PostConstructor(&this_00->super_CBaseEntity,pClassName);
  return &(this_00->super_CBaseEntity).m_Network.super_IServerNetworkable;
}


/* DataMapInit<CInfo_Coop_Spawn> at 000bf710 */

datamap_t * DataMapInit<CInfo_Coop_Spawn>(CInfo_Coop_Spawn *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&sv_pushaway_max_player_force.super_ConCommandBase.m_pNext + unaff_EBX + 3) ==
      '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf7747);
    if (iVar1 != 0) {
      *(int *)((int)&sv_pushaway_max_player_force.super_ConCommandBase.m_pszName + unaff_EBX + 3) =
           unaff_EBX + 0x94f613;
      *(undefined4 *)
       ((int)&sv_pushaway_max_player_force.super_ConCommandBase.m_nFlags + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&sv_pushaway_max_player_force.super_IConVar._vptr_IConVar + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&sv_pushaway_max_player_force.m_pParent + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&sv_pushaway_max_player_force.m_pszDefaultValue + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&sv_pushaway_max_player_force.m_Value.m_pszString + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&sv_pushaway_max_player_force.super_ConCommandBase.m_pszHelpString + unaff_EBX + 3) =
           0x10;
      ___cxa_guard_release(unaff_EBX + 0xcf7747);
      ___cxa_atexit(unaff_EBX + 0x8da0af,0,*(undefined4 *)(&DAT_00aec46f + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_EndTouch_00c0a768 + unaff_EBX + 3) =
       *(undefined4 *)(&DAT_00aec47b + unaff_EBX);
  *(undefined4 *)((int)&PTR_StartTouch_00c0a760 + unaff_EBX + 3) = 6;
  *(int *)((int)&PTR_Use_00c0a75c + unaff_EBX + 3) = unaff_EBX + 0xc0a7bf;
  return (datamap_t *)((int)&PTR_Use_00c0a75c + unaff_EBX + 3);
}


/* __static_initialization_and_destruction_0 at 000bf7e0 */

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
  *(undefined1 *)
   ((int)&sv_pushaway_min_player_speed.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) = 0;
  *(undefined1 *)
   ((int)&sv_pushaway_min_player_speed.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&sv_pushaway_min_player_speed.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 2) = 0;
  *(undefined1 *)
   ((int)&sv_pushaway_min_player_speed.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&sv_pushaway_min_player_speed.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX)
       = 0;
  *(undefined4 *)
   ((int)&sv_pushaway_min_player_speed.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) = 0;
  *(undefined4 *)((int)&sv_pushaway_min_player_speed.m_fnChangeCallbacks.m_Size + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&sv_pushaway_min_player_speed.m_fnChangeCallbacks.m_pElements + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)(&DAT_00cf7618 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00cf761c + unaff_EBX) = 0;
  *(undefined4 *)((int)&sv_pushaway_max_force.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX)
       = 0;
  *(undefined4 *)((int)&sv_pushaway_max_force.super_ConCommandBase.m_pNext + unaff_EBX) = 0;
  *(undefined4 *)(&sv_pushaway_max_force.super_ConCommandBase.m_bRegistered + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&sv_pushaway_max_force.super_ConCommandBase.m_pszName + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&sv_pushaway_max_force.super_ConCommandBase.m_pszHelpString + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&sv_pushaway_max_force.super_ConCommandBase.m_nFlags + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&sv_pushaway_max_force.super_IConVar._vptr_IConVar + unaff_EBX) = 0;
  *(undefined1 *)((int)&sv_pushaway_max_force.m_pParent + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&sv_pushaway_max_force.m_pParent + unaff_EBX) = 0;
  *(undefined1 *)((int)&sv_pushaway_max_force.m_pParent + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&sv_pushaway_max_force.m_pParent + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&sv_pushaway_max_force.m_pszDefaultValue + unaff_EBX) = 0;
  *(undefined1 *)((int)&sv_pushaway_max_force.m_Value.m_pszString + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&sv_pushaway_max_force.m_Value.m_pszString + unaff_EBX) = 0;
  *(undefined1 *)((int)&sv_pushaway_max_force.m_Value.m_pszString + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&sv_pushaway_max_force.m_Value.m_pszString + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&sv_pushaway_max_force.m_Value.m_StringLength + unaff_EBX) = 0;
  *(undefined1 *)((int)&sv_pushaway_max_force.m_Value.m_fValue + unaff_EBX + 3) = 1;
  *(undefined1 *)((int)&sv_pushaway_max_force.m_Value.m_fValue + unaff_EBX) = 0;
  *(undefined1 *)((int)&sv_pushaway_max_force.m_Value.m_fValue + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&sv_pushaway_max_force.m_Value.m_fValue + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&sv_pushaway_max_force.m_Value.m_nValue + unaff_EBX) = 1;
  (&sv_pushaway_max_force.field_0x37)[unaff_EBX] = 0;
  (&sv_pushaway_max_force.m_bHasMin)[unaff_EBX] = false;
  (&sv_pushaway_max_force.field_0x35)[unaff_EBX] = 0;
  (&sv_pushaway_max_force.field_0x36)[unaff_EBX] = 0;
  *(undefined4 *)((int)&sv_pushaway_max_force.m_fMinVal + unaff_EBX) = 2;
  (&sv_pushaway_max_force.field_0x3f)[unaff_EBX] = 0;
  (&sv_pushaway_max_force.m_bHasMax)[unaff_EBX] = false;
  (&sv_pushaway_max_force.field_0x3d)[unaff_EBX] = 0;
  (&sv_pushaway_max_force.field_0x3e)[unaff_EBX] = 0;
  *(undefined **)((int)&sv_pushaway_max_force.m_fMaxVal + unaff_EBX) = &UNK_00af1b3c + unaff_EBX;
  pdVar1 = DataMapInit<CInfo_Coop_Spawn>((CInfo_Coop_Spawn *)0x0);
  *(datamap_t **)((int)DataMapInit<CBaseGrenade>::dataDesc[4].flatOffset + unaff_EBX) = pdVar1;
  *(undefined **)((int)&sv_pushaway_max_force.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) =
       &UNK_00b71c44 + unaff_EBX;
  pIVar2 = EntityFactoryDictionary();
  (**pIVar2->_vptr_IEntityFactoryDictionary)(pIVar2,unaff_EBX + 0xcf7664,unaff_EBX + 0x94f559);
  *(undefined4 *)
   ((int)&sv_pushaway_max_force.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) = 0;
  *(undefined4 *)((int)&sv_pushaway_max_force.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX)
       = 0;
  *(undefined4 *)((int)&sv_pushaway_max_force.m_fnChangeCallbacks.m_Size + unaff_EBX) = 0;
  *(undefined4 *)((int)&sv_pushaway_max_force.m_fnChangeCallbacks.m_pElements + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00cf7678 + unaff_EBX) = 0;
  ___cxa_atexit(unaff_EBX + 0x8d9f04,0,*(undefined4 *)(&DAT_00aec3a4 + unaff_EBX));
  return;
}


/* CInfo_Coop_Spawn::GetDataDescMap at 006b9310 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CInfo_Coop_Spawn * this) */

datamap_t * __thiscall CInfo_Coop_Spawn::GetDataDescMap(CInfo_Coop_Spawn *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(&UNK_00610b68 + extraout_ECX);
}


/* CInfo_Coop_Spawn::GetBaseMap at 006b9320 */

datamap_t * CInfo_Coop_Spawn::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4f2874);
}


/* __tcf_0 at 009997d0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41d6a6)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41d69a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41d6a6));
  }
  *(undefined4 *)(unaff_EBX + 0x41d6a6) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41d6a2)) {
    if (*(int *)(unaff_EBX + 0x41d69a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2123ba) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2123ba),*(int *)(unaff_EBX + 0x41d69a));
      *(undefined4 *)(unaff_EBX + 0x41d69a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d69e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41d69a);
  *(int *)(unaff_EBX + 0x41d6aa) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41d6a2)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2123ba) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2123ba),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41d69a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d69e) = 0;
  }
  return;
}


/* __tcf_1 at 009996f0 */

void __tcf_1(void *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  *(undefined4 *)(&DAT_0041d75c + unaff_EBX) = 0;
  if (*(int *)(unaff_EBX + 0x41d758) < 0) {
    iVar1 = *(int *)(unaff_EBX + 0x41d750);
  }
  else {
    if (*(int *)(unaff_EBX + 0x41d750) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x212494) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x212494),*(int *)(unaff_EBX + 0x41d750));
      *(undefined4 *)(unaff_EBX + 0x41d750) = 0;
    }
    iVar1 = 0;
    *(undefined4 *)(unaff_EBX + 0x41d754) = 0;
  }
  *(int *)(unaff_EBX + 0x41d760) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41d758)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x212494) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x212494),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41d750) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d754) = 0;
  }
  return;
}


/* CInfo_Coop_Spawn::Activate at 006b95e0 */

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
  iVar2 = *(int *)(unaff_EBX + 0x6fd86c);
  iVar1 = iVar2 + 1;
  iVar3 = iVar1;
  if (*(int *)(unaff_EBX + 0x6fd864) < iVar1) {
    CUtlMemory<CInfo_Coop_Spawn*,int>::Grow
              ((CUtlMemory<CInfo_Coop_Spawn*,int> *)(unaff_EBX + 0x6fd860),
               iVar1 - *(int *)(unaff_EBX + 0x6fd864));
    iVar3 = *(int *)(unaff_EBX + 0x6fd86c) + 1;
  }
  *(int *)(unaff_EBX + 0x6fd86c) = iVar3;
  iVar5 = *(int *)(unaff_EBX + 0x6fd860);
  *(int *)(unaff_EBX + 0x6fd870) = iVar5;
  iVar3 = (iVar3 - iVar2) + -1;
  if (0 < iVar3) {
    _V_memmove((void *)(iVar5 + iVar1 * 4),(void *)(iVar5 + iVar2 * 4),iVar3 * 4);
    iVar5 = *(int *)(unaff_EBX + 0x6fd860);
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


/* CInfo_Coop_Spawn::InputEnable at 006b9330 */

/* DWARF original prototype: void InputEnable(CInfo_Coop_Spawn * this, inputdata_t * inputdata) */

void __thiscall CInfo_Coop_Spawn::InputEnable(CInfo_Coop_Spawn *this,inputdata_t *inputdata)

{
  this->m_bEnabled = true;
  return;
}


/* CInfo_Coop_Spawn::InputDisable at 006b9340 */

/* DWARF original prototype: void InputDisable(CInfo_Coop_Spawn * this, inputdata_t * inputdata) */

void __thiscall CInfo_Coop_Spawn::InputDisable(CInfo_Coop_Spawn *this,inputdata_t *inputdata)

{
  this->m_bEnabled = false;
  return;
}


/* CInfo_Coop_Spawn::InputSetAsActiveSpawn at 006b96c0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
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
  iVar1 = *(int *)(CWeaponPaintGun::ResetPaint + unaff_EBX + 2);
  iVar2 = *(int *)(&DAT_006fd786 + unaff_EBX);
  if (iVar1 != 0) {
    iVar4 = 0;
LAB_006b970a:
    do {
      pCVar3 = *(CInfo_Coop_Spawn **)(iVar2 + iVar4 * 4);
      if (pCVar3 == (CInfo_Coop_Spawn *)0x0) {
        _Warning(unaff_EBX + 0x35568a);
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
        goto LAB_006b970a;
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


/* CInfo_Coop_Spawn::UpdateOnRemove at 006b9430 */

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
  iVar1 = *(int *)(unaff_EBX + 0x6fda22);
  if (0 < iVar1) {
    piVar2 = *(int **)(unaff_EBX + 0x6fda16);
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
        if (iVar4 == iVar1) goto LAB_006b9486;
        iVar3 = iVar5 + 4;
      } while (this != (CInfo_Coop_Spawn *)piVar2[iVar4]);
    }
    if (iVar4 != iVar1 + -1) {
      *(int *)(iVar5 + (int)piVar2) = piVar2[iVar1 + -1];
      iVar4 = *(int *)(unaff_EBX + 0x6fda22) + -1;
    }
    *(int *)(unaff_EBX + 0x6fda22) = iVar4;
  }
LAB_006b9486:
  CBaseEntity::UpdateOnRemove((CBaseEntity *)this);
  return;
}


/* GetCoopSpawnLocation at 006b94c0 */

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
  aiStack_50[0] = 0x6b94ce;
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(unaff_EBX + 0x4f271e);
  iKnownLocations = *(int *)(unaff_EBX + 0x6fd992);
  iVar4 = *(int *)(unaff_EBX + 0x6fd986);
  iVar1 = -((iKnownLocations * 4 + 0xfU & 0xfffffff0) + 0x10);
  local_38 = auStack_3c + iVar1;
                    /* Unresolved local var: int i@[???] */
  if (iKnownLocations != 0) {
    iValidLocations = 0;
    iVar6 = 0;
    do {
      while (iVar3 = *(int *)(iVar4 + iVar6 * 4), *(char *)(iVar3 + 0x514) == '\0') {
LAB_006b9520:
        iVar6 = iVar6 + 1;
        if (iVar6 == iKnownLocations) goto LAB_006b9566;
      }
      *(int *)((int)aiStack_50 + iVar1 + 4) = iVar3;
      *(undefined4 *)((int)aiStack_50 + iVar1) = 0x6b953a;
      iVar3 = CBaseEntity::GetTeamNumber(*(CBaseEntity **)((int)aiStack_50 + iVar1 + 4));
      if (iVar3 != iTeam) {
        *(undefined4 *)((int)aiStack_50 + iVar1 + 4) = *(undefined4 *)(iVar4 + iVar6 * 4);
        *(undefined4 *)((int)aiStack_50 + iVar1) = 0x6b954a;
        iVar3 = CBaseEntity::GetTeamNumber(*(CBaseEntity **)((int)aiStack_50 + iVar1 + 4));
        if (1 < iVar3) goto LAB_006b9520;
      }
      *(undefined4 *)(local_38 + iValidLocations * 4) = *(undefined4 *)(iVar4 + iVar6 * 4);
      iValidLocations = iValidLocations + 1;
      iVar6 = iVar6 + 1;
    } while (iVar6 != iKnownLocations);
LAB_006b9566:
    if (iValidLocations != 0) {
      *(int *)((int)aiStack_50 + iVar1 + 8) = iValidLocations + -1;
      *(undefined4 *)((int)aiStack_50 + iVar1 + 4) = 0;
      *(undefined4 *)((int)aiStack_50 + iVar1) = 0x6b9581;
      iVar4 = _RandomInt();
      pCVar5 = *(CBaseEntity **)(local_38 + iVar4 * 4);
      if ((char)pCVar5[1].m_DamageModifiers.m_LastAlloc.index != '\0') {
        *(undefined4 *)((int)aiStack_50 + iVar1 + 4) = **(undefined4 **)(unaff_EBX + 0x4f270e);
        *(undefined4 *)((int)aiStack_50 + iVar1) = 0x6b95bb;
        bVar2 = CPortalMPGameRules::Is2GunsCoOp
                          (*(CPortalMPGameRules **)((int)aiStack_50 + iVar1 + 4));
        if (bVar2) {
          **(undefined4 **)(unaff_EBX + 0x4f321a) = 3;
        }
      }
      goto LAB_006b9596;
    }
  }
  pCVar5 = (CBaseEntity *)0x0;
LAB_006b9596:
  if (local_20 != **(int **)(unaff_EBX + 0x4f271e)) {
                    /* WARNING: Subroutine does not return */
    *(undefined **)((int)aiStack_50 + iVar1) = &UNK_006b95d4;
    ___stack_chk_fail();
  }
  return pCVar5;
}


/* CInfo_Coop_Spawn::DrawDebugTextOverlays at 006b9350 */

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
  local_20 = **(int **)(unaff_EBX + 0x4f2885);
  text_offset_00 = CBaseAnimating::DrawDebugTextOverlays(&this->super_CBaseAnimating);
  iVar1 = unaff_EBX + 0x3559b9;
  if (this->m_bEnabled == false) {
    iVar1 = unaff_EBX + 0x3559c1;
  }
  V_snprintf(local_11f,0xff,(char *)(unaff_EBX + 0x2fa6bd),iVar1);
  CBaseEntity::EntityText((CBaseEntity *)this,text_offset_00,local_11f,0.0,0xff,0xff,0xff,0xff);
  if (local_20 == **(int **)(unaff_EBX + 0x4f2885)) {
    return text_offset_00 + 1;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}


/* _GLOBAL__I__ZN16CInfo_Coop_Spawn9m_DataMapE at 000bf9b0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN16CInfo_Coop_Spawn9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

