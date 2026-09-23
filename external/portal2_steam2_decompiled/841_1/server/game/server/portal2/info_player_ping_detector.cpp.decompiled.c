/* DWARF-guided pseudocode for game/server/portal2/info_player_ping_detector.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* DataMapInit<CInfo_Player_Ping_Detector> at 000aae80 */

datamap_t * DataMapInit<CInfo_Player_Ping_Detector>(CInfo_Player_Ping_Detector *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3a9].m_pEntity + unaff_EBX) ==
      '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xda1674);
    if (iVar1 != 0) {
      *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3aa].m_pEntity + unaff_EBX) =
           unaff_EBX + 0xa05f7f;
      *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3aa].m_pPrev + unaff_EBX)
           = 0;
      *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3aa].m_pNext + unaff_EBX)
           = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3ab].m_pEntity + unaff_EBX) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3ab].m_SerialNumber + unaff_EBX) = 0;
      *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3ab].m_pPrev + unaff_EBX)
           = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3aa].m_SerialNumber + unaff_EBX) = 0x1a
      ;
      ___cxa_guard_release(unaff_EBX + 0xda1674);
      ___cxa_atexit(unaff_EBX + 0x991e9c,0,*(undefined4 *)(&DAT_00b9bcc0 + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_TakeHealth_00cb91f8 + unaff_EBX) =
       *(undefined4 *)(&DAT_00b9bdf8 + unaff_EBX);
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3a9].m_pPrev + unaff_EBX) ==
      '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xda167c);
    if (iVar1 != 0) {
      *(undefined4 *)((int)&PTR_GetBaseEntity_00cb9460 + unaff_EBX) =
           **(undefined4 **)(&DAT_00b9bce0 + unaff_EBX);
      ___cxa_guard_release(unaff_EBX + 0xda167c);
    }
  }
  *(undefined4 *)((int)&PTR_CanBeHitByMeleeAttack_00cb91f0 + unaff_EBX) = 9;
  *(int *)((int)&PTR_TraceAttack_00cb91ec + unaff_EBX) = (int)&PTR_IsBaseTrain_00cb924c + unaff_EBX;
  return (datamap_t *)((int)&PTR_TraceAttack_00cb91ec + unaff_EBX);
}


/* __static_initialization_and_destruction_0 at 000aafa0 */

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
  *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x393].m_pPrev + unaff_EBX) = 0
  ;
  *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x393].m_pPrev + unaff_EBX + 1)
       = 0;
  *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x393].m_pPrev + unaff_EBX + 2)
       = 0;
  *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x393].m_pPrev + unaff_EBX + 3)
       = 0;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x393].m_pNext + unaff_EBX) = 0
  ;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x394].m_pEntity + unaff_EBX) =
       0;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x394].m_SerialNumber + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x394].m_pPrev + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x394].m_pNext + unaff_EBX) = 0
  ;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x395].m_pEntity + unaff_EBX) =
       0;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x395].m_SerialNumber + unaff_EBX) = 0;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x395].m_pPrev + unaff_EBX) = 0
  ;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x395].m_pNext + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x396].m_pEntity + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x396].m_SerialNumber + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x396].m_pPrev + unaff_EBX) =
       0x7f7fffff;
  *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x396].m_pNext + unaff_EBX) =
       &UNK_00ba135c + unaff_EBX;
  pdVar1 = DataMapInit<CInfo_Player_Ping_Detector>((CInfo_Player_Ping_Detector *)0x0);
  *(datamap_t **)((int)DataMapInit<CAI_PlayerAlly>::dataDesc[0xe].flatOffset + unaff_EBX + -0xc) =
       pdVar1;
  *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x397].m_pEntity + unaff_EBX) =
       &UNK_00c1f1d4 + unaff_EBX;
  pIVar2 = EntityFactoryDictionary();
  (**pIVar2->_vptr_IEntityFactoryDictionary)(pIVar2,unaff_EBX + 0xda1554,unaff_EBX + 0xa05e82);
  return;
}


/* CInfo_Player_Ping_Detector::GetDataDescMap at 0071a040 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CInfo_Player_Ping_Detector * this) */

datamap_t * __thiscall CInfo_Player_Ping_Detector::GetDataDescMap(CInfo_Player_Ping_Detector *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x64a038);
}


/* CInfo_Player_Ping_Detector::GetBaseMap at 0071a050 */

datamap_t * CInfo_Player_Ping_Detector::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x52cc34);
}


/* __tcf_0 at 00a3cd30 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x40f7ee)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x40f7e2) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x40f7ee));
  }
  *(undefined4 *)(unaff_EBX + 0x40f7ee) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40f7ea)) {
    if (*(int *)(unaff_EBX + 0x40f7e2) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x209e1e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x209e1e),*(int *)(unaff_EBX + 0x40f7e2));
      *(undefined4 *)(unaff_EBX + 0x40f7e2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f7e6) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40f7e2);
  *(int *)(unaff_EBX + 0x40f7f2) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40f7ea)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x209e1e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x209e1e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40f7e2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f7e6) = 0;
  }
  return;
}


/* CInfo_Player_Ping_Detector::CInfo_Player_Ping_Detector at 0071a0e0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CInfo_Player_Ping_Detector(CInfo_Player_Ping_Detector * this) */

void __thiscall
CInfo_Player_Ping_Detector::CInfo_Player_Ping_Detector(CInfo_Player_Ping_Detector *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::CBaseEntity((CBaseEntity *)this,false);
  (this->super_CGameEventListener).m_bRegisteredForEvents = false;
  (this->super_CGameEventListener).m_nDebugID = 0x2a;
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5afd17);
  (this->super_CGameEventListener).super_IGameEventListener2._vptr_IGameEventListener2 =
       (_func_int_varargs **)(unaff_EBX + 0x5b0037);
  (this->m_OnPingDetected).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnPingDetected).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->m_OnPingDetected).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_vecPingLocation).x = 0.0;
  (this->m_vecPingLocation).y = 0.0;
  (this->m_vecPingLocation).z = 0.0;
  this->m_bDetectedNewPing = false;
  return;
}


/* CInfo_Player_Ping_Detector::CInfo_Player_Ping_Detector at 0071a170 */

/* DWARF original prototype: void CInfo_Player_Ping_Detector(CInfo_Player_Ping_Detector * this,
   CInfo_Player_Ping_Detector * this) */

void __thiscall
CInfo_Player_Ping_Detector::CInfo_Player_Ping_Detector
          (CInfo_Player_Ping_Detector *this,CInfo_Player_Ping_Detector *this_1)

{
  CInfo_Player_Ping_Detector(this);
  return;
}


/* CEntityFactory<CInfo_Player_Ping_Detector>::Create at 0071abc0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CInfo_Player_Ping_Detector>
   * this, char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CInfo_Player_Ping_Detector>::Create
          (CEntityFactory<CInfo_Player_Ping_Detector> *this,char *pClassName)

{
  CInfo_Player_Ping_Detector *this_00;
  
                    /* Unresolved local var: CInfo_Player_Ping_Detector * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x3fc);
  CInfo_Player_Ping_Detector::CInfo_Player_Ping_Detector(this_00);
  (*(this_00->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1d])(this_00,pClassName);
  return &(this_00->super_CPointEntity).super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CInfo_Player_Ping_Detector::~CInfo_Player_Ping_Detector at 0071a830 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CInfo_Player_Ping_Detector(CInfo_Player_Ping_Detector * this, int
   __in_chrg) */

void __thiscall
CInfo_Player_Ping_Detector::~CInfo_Player_Ping_Detector
          (CInfo_Player_Ping_Detector *this,int __in_chrg)

{
  int *piVar1;
  int unaff_EBX;
  int in_stack_ffffffc8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5af5c4);
  (this->super_CGameEventListener).super_IGameEventListener2._vptr_IGameEventListener2 =
       (_func_int_varargs **)(unaff_EBX + 0x5af8e4);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnPingDetected).super_CBaseEntityOutput,in_stack_ffffffc8);
  (this->super_CGameEventListener).super_IGameEventListener2._vptr_IGameEventListener2 =
       (_func_int_varargs **)(unaff_EBX + 0x52e044);
  (this->super_CGameEventListener).m_nDebugID = 0xd;
  if ((this->super_CGameEventListener).m_bRegisteredForEvents != false) {
    piVar1 = (int *)**(int **)(unaff_EBX + 0x52c344);
    if (piVar1 != (int *)0x0) {
      (**(code **)(*piVar1 + 0x18))(piVar1,&this->super_CGameEventListener);
    }
    (this->super_CGameEventListener).m_bRegisteredForEvents = false;
  }
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x52c44c) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,__in_chrg);
  return;
}


/* CInfo_Player_Ping_Detector::~CInfo_Player_Ping_Detector at 0071a950 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CInfo_Player_Ping_Detector(CInfo_Player_Ping_Detector * this, int
   __in_chrg) */

void __thiscall
CInfo_Player_Ping_Detector::~CInfo_Player_Ping_Detector
          (CInfo_Player_Ping_Detector *this,int __in_chrg)

{
  int *piVar1;
  int unaff_EBX;
  CGameEventListener *in_stack_ffffffc8;
  CGameEventListener *pCVar2;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5af4a4);
  (this->super_CGameEventListener).super_IGameEventListener2._vptr_IGameEventListener2 =
       (_func_int_varargs **)(unaff_EBX + 0x5af7c4);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnPingDetected).super_CBaseEntityOutput,(int)in_stack_ffffffc8);
  pCVar2 = &this->super_CGameEventListener;
  (this->super_CGameEventListener).super_IGameEventListener2._vptr_IGameEventListener2 =
       (_func_int_varargs **)(unaff_EBX + 0x52df24);
  (this->super_CGameEventListener).m_nDebugID = 0xd;
  if ((this->super_CGameEventListener).m_bRegisteredForEvents != false) {
    piVar1 = (int *)**(int **)(unaff_EBX + 0x52c224);
    if (piVar1 != (int *)0x0) {
      (**(code **)(*piVar1 + 0x18))(piVar1);
      in_stack_ffffffc8 = pCVar2;
    }
    (this->super_CGameEventListener).m_bRegisteredForEvents = false;
  }
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(&DAT_0052c32c + unaff_EBX) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,(int)in_stack_ffffffc8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CInfo_Player_Ping_Detector::~CInfo_Player_Ping_Detector at 0071aa70 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CInfo_Player_Ping_Detector(CInfo_Player_Ping_Detector * this, int
   __in_chrg) */

void __thiscall
CInfo_Player_Ping_Detector::~CInfo_Player_Ping_Detector
          (CInfo_Player_Ping_Detector *this,int __in_chrg)

{
  int *piVar1;
  int unaff_EBX;
  int in_stack_ffffffc8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5af384);
  (this->super_CGameEventListener).super_IGameEventListener2._vptr_IGameEventListener2 =
       (_func_int_varargs **)(unaff_EBX + 0x5af6a4);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnPingDetected).super_CBaseEntityOutput,in_stack_ffffffc8);
  (this->super_CGameEventListener).super_IGameEventListener2._vptr_IGameEventListener2 =
       (_func_int_varargs **)(unaff_EBX + 0x52de04);
  (this->super_CGameEventListener).m_nDebugID = 0xd;
  if ((this->super_CGameEventListener).m_bRegisteredForEvents != false) {
    piVar1 = (int *)**(int **)(unaff_EBX + 0x52c104);
    if (piVar1 != (int *)0x0) {
      (**(code **)(*piVar1 + 0x18))(piVar1,&this->super_CGameEventListener);
    }
    (this->super_CGameEventListener).m_bRegisteredForEvents = false;
  }
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x52c20c) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,__in_chrg);
  return;
}


/* CInfo_Player_Ping_Detector::Spawn at 0071a070 */

/* DWARF original prototype: void Spawn(CInfo_Player_Ping_Detector * this) */

void __thiscall CInfo_Player_Ping_Detector::Spawn(CInfo_Player_Ping_Detector *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CPointEntity::Spawn(&this->super_CPointEntity);
  if (this->m_bEnabled != false) {
                    /* Unresolved local var: bool bServerSide@[???] */
    (this->super_CGameEventListener).m_bRegisteredForEvents = true;
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52cb07) + 0x10))
              ((int *)**(undefined4 **)(unaff_EBX + 0x52cb07),&this->super_CGameEventListener,
               unaff_EBX + 0x3924ba,1);
  }
  return;
}


/* CInfo_Player_Ping_Detector::UpdateOnRemove at 0071a060 */

/* DWARF original prototype: void UpdateOnRemove(CInfo_Player_Ping_Detector * this) */

void __thiscall CInfo_Player_Ping_Detector::UpdateOnRemove(CInfo_Player_Ping_Detector *this)

{
  CBaseEntity::UpdateOnRemove((CBaseEntity *)this);
  return;
}


/* CInfo_Player_Ping_Detector::FireGameEvent at 0071a1f0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void FireGameEvent(CInfo_Player_Ping_Detector * this, IGameEvent *
   event) */

void __thiscall
CInfo_Player_Ping_Detector::FireGameEvent(CInfo_Player_Ping_Detector *this,IGameEvent *event)

{
  ushort uVar1;
  CBaseEdict *this_00;
  ushort *puVar2;
  int *piVar3;
  char *pcVar4;
  int iVar5;
  CBasePlayer *this_01;
  int iVar6;
  CBaseEntity *pStartEntity;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  uint uVar9;
  uint uVar10;
  int unaff_EBX;
  int iVar11;
  longdouble extraout_ST0;
  longdouble extraout_ST0_00;
  longdouble extraout_ST0_01;
  uint local_70;
  
                    /* Unresolved local var: char * name@[???] */
  ___i686_get_pc_thunk_bx();
  if (this->m_bEnabled != false) {
    pcVar4 = (char *)(*event->_vptr_IGameEvent[2])(event);
    iVar5 = _V_strcmp(pcVar4,(char *)(unaff_EBX + 0x39233a));
    if (iVar5 == 0) {
      iVar5 = (*event->_vptr_IGameEvent[7])(event,unaff_EBX + 0x33f1c3,0);
      (*event->_vptr_IGameEvent[9])(event,unaff_EBX + 0x3938c1,0);
      (*event->_vptr_IGameEvent[9])(event,unaff_EBX + 0x39234f,0);
      (*event->_vptr_IGameEvent[9])(event,unaff_EBX + 0x3938cb,0);
      iVar11 = 1;
      do {
                    /* Unresolved local var: CBasePlayer * pPlayer@[???] */
        this_01 = UTIL_PlayerByIndex(iVar11);
        if (this_01 != (CBasePlayer *)0x0) {
          iVar6 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52c97f) + 0x40))
                            ((int *)**(undefined4 **)(unaff_EBX + 0x52c97f),
                             (this_01->super_CBaseCombatCharacter).super_CBaseFlex.
                             super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                             m_Network.m_pPev);
          if (iVar5 == iVar6) {
            iVar5 = CBaseEntity::GetTeamNumber((CBaseEntity *)this_01);
            goto LAB_0071a2ec;
          }
        }
        iVar11 = iVar11 + 1;
      } while (iVar11 != 0x22);
      iVar5 = -1;
LAB_0071a2ec:
      if ((this->m_nTeamToLookAt == 1) || (iVar5 == this->m_nTeamToLookAt)) {
        (this->m_vecPingLocation).x = (float)extraout_ST0;
        (this->m_vecPingLocation).y = (float)extraout_ST0_00;
        (this->m_vecPingLocation).z = (float)extraout_ST0_01;
        this->m_bDetectedNewPing = true;
        pStartEntity = (CBaseEntity *)0x0;
        do {
          do {
            pcVar4 = (this->m_iszFuncTankName).pszValue;
            if (pcVar4 == (char *)0x0) {
              pcVar4 = (char *)(unaff_EBX + 0x334b23);
            }
            pStartEntity = CGlobalEntityList::FindEntityByName
                                     (*(CGlobalEntityList **)(unaff_EBX + 0x52ca23),pStartEntity,
                                      pcVar4,(CBaseEntity *)0x0,(CBaseEntity *)0x0,
                                      (CBaseEntity *)0x0,(IEntityFindFilter *)0x0);
            if (pStartEntity == (CBaseEntity *)0x0) {
              COutputEvent::FireOutput
                        (&this->m_OnPingDetected,(CBaseEntity *)0x0,(CBaseEntity *)0x0,0.0);
              return;
            }
            iVar5 = ___dynamic_cast(pStartEntity,*(undefined4 *)(unaff_EBX + 0x52c98f),
                                    *(undefined4 *)(unaff_EBX + 0x52d497),0);
          } while (iVar5 == 0);
          uVar10 = *(uint *)(iVar5 + 0x128) | 0x80;
          if (*(uint *)(iVar5 + 0x128) != uVar10) {
            if (*(char *)(iVar5 + 0x60) == '\0') {
              this_00 = *(CBaseEdict **)(iVar5 + 0x24);
              if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
                this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
                pIVar7 = CBaseEdict::GetChangeAccessor(this_00);
                puVar2 = (ushort *)**(undefined4 **)(unaff_EBX + 0x52c967);
                if (pIVar7->m_iChangeInfoSerialNumber == *puVar2) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                  uVar9 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
                  uVar1 = puVar2[uVar9 * 0x14 + 0x14];
                  if (uVar1 == 0) {
LAB_0071a585:
                    puVar2[(uint)uVar1 + uVar9 * 0x14 + 1] = 0x128;
                    puVar2[uVar9 * 0x14 + 0x14] = uVar1 + 1;
                  }
                  else if (puVar2[uVar9 * 0x14 + 1] != 0x128) {
                    local_70 = 0;
                    do {
                      uVar8 = local_70 + 1;
                      local_70 = uVar8 & 0xffff;
                      if ((ushort)uVar8 == uVar1) {
                        if (uVar1 != 0x13) goto LAB_0071a585;
                        goto LAB_0071a4fd;
                      }
                    } while (puVar2[uVar9 * 0x14 + local_70 + 1] != 0x128);
                  }
                }
                else if (puVar2[0x7d1] == 100) {
LAB_0071a4fd:
                  pIVar7->m_iChangeInfoSerialNumber = 0;
                  this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                }
                else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                  pIVar7->m_iChangeInfo = puVar2[0x7d1];
                  *(short *)(**(int **)(unaff_EBX + 0x52c967) + 0xfa2) =
                       *(short *)(**(int **)(unaff_EBX + 0x52c967) + 0xfa2) + 1;
                  piVar3 = *(int **)(unaff_EBX + 0x52c967);
                  pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar3;
                  iVar11 = *piVar3 + (uint)pIVar7->m_iChangeInfo * 0x28;
                  *(undefined2 *)(iVar11 + 2) = 0x128;
                  *(undefined2 *)(iVar11 + 0x28) = 1;
                }
              }
            }
            else {
              *(uint *)(iVar5 + 100) = *(uint *)(iVar5 + 100) | 1;
            }
            *(uint *)(iVar5 + 0x128) = uVar10;
          }
          *(undefined4 *)(iVar5 + 0x3c8) = 0xffffffff;
          *(vec_t *)(iVar5 + 0x4dc) = (this->m_vecPingLocation).x;
          *(vec_t *)(iVar5 + 0x4e0) = (this->m_vecPingLocation).y;
          *(vec_t *)(iVar5 + 0x4e4) = (this->m_vecPingLocation).z;
        } while( true );
      }
    }
  }
  return;
}


/* CInfo_Player_Ping_Detector::InputToggle at 0071a780 */

/* DWARF original prototype: void InputToggle(CInfo_Player_Ping_Detector * this, inputdata_t *
   inputdata) */

void __thiscall
CInfo_Player_Ping_Detector::InputToggle(CInfo_Player_Ping_Detector *this,inputdata_t *inputdata)

{
  int *piVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_bEnabled != false) {
    this->m_bEnabled = false;
    if ((this->super_CGameEventListener).m_bRegisteredForEvents != false) {
      piVar1 = (int *)**(int **)(unaff_EBX + 0x52c3f7);
      if (piVar1 != (int *)0x0) {
        (**(code **)(*piVar1 + 0x18))(piVar1,&this->super_CGameEventListener);
      }
      (this->super_CGameEventListener).m_bRegisteredForEvents = false;
    }
    return;
  }
  this->m_bEnabled = true;
                    /* Unresolved local var: bool bServerSide@[???] */
  (this->super_CGameEventListener).m_bRegisteredForEvents = true;
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52c3f7) + 0x10))
            ((int *)**(undefined4 **)(unaff_EBX + 0x52c3f7),&this->super_CGameEventListener,
             unaff_EBX + 0x391daa,1);
  return;
}


/* CInfo_Player_Ping_Detector::InputEnable at 0071a670 */

/* DWARF original prototype: void InputEnable(CInfo_Player_Ping_Detector * this, inputdata_t *
   inputdata) */

void __thiscall
CInfo_Player_Ping_Detector::InputEnable(CInfo_Player_Ping_Detector *this,inputdata_t *inputdata)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  this->m_bEnabled = true;
                    /* Unresolved local var: bool bServerSide@[???] */
  (this->super_CGameEventListener).m_bRegisteredForEvents = true;
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52c507) + 0x10))
            ((int *)**(undefined4 **)(unaff_EBX + 0x52c507),&this->super_CGameEventListener,
             unaff_EBX + 0x391eba,1);
  return;
}


/* CInfo_Player_Ping_Detector::InputDisable at 0071a610 */

/* DWARF original prototype: void InputDisable(CInfo_Player_Ping_Detector * this, inputdata_t *
   inputdata) */

void __thiscall
CInfo_Player_Ping_Detector::InputDisable(CInfo_Player_Ping_Detector *this,inputdata_t *inputdata)

{
  int *piVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  this->m_bEnabled = false;
  if ((this->super_CGameEventListener).m_bRegisteredForEvents != false) {
    piVar1 = (int *)**(int **)(unaff_EBX + 0x52c567);
    if (piVar1 != (int *)0x0) {
      (**(code **)(*piVar1 + 0x18))(piVar1,&this->super_CGameEventListener);
    }
    (this->super_CGameEventListener).m_bRegisteredForEvents = false;
  }
  return;
}


/* CInfo_Player_Ping_Detector::Toggle at 0071a6d0 */

/* DWARF original prototype: void Toggle(CInfo_Player_Ping_Detector * this) */

void __thiscall CInfo_Player_Ping_Detector::Toggle(CInfo_Player_Ping_Detector *this)

{
  int *piVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_bEnabled != false) {
    this->m_bEnabled = false;
    if ((this->super_CGameEventListener).m_bRegisteredForEvents != false) {
      piVar1 = (int *)**(int **)(unaff_EBX + 0x52c4a7);
      if (piVar1 != (int *)0x0) {
        (**(code **)(*piVar1 + 0x18))(piVar1,&this->super_CGameEventListener);
      }
      (this->super_CGameEventListener).m_bRegisteredForEvents = false;
    }
    return;
  }
  this->m_bEnabled = true;
                    /* Unresolved local var: bool bServerSide@[???] */
  (this->super_CGameEventListener).m_bRegisteredForEvents = true;
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52c4a7) + 0x10))
            ((int *)**(undefined4 **)(unaff_EBX + 0x52c4a7),&this->super_CGameEventListener,
             unaff_EBX + 0x391e5a,1);
  return;
}


/* CInfo_Player_Ping_Detector::Enable at 0071a180 */

/* DWARF original prototype: void Enable(CInfo_Player_Ping_Detector * this) */

void __thiscall CInfo_Player_Ping_Detector::Enable(CInfo_Player_Ping_Detector *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  this->m_bEnabled = true;
                    /* Unresolved local var: bool bServerSide@[???] */
  (this->super_CGameEventListener).m_bRegisteredForEvents = true;
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52c9f7) + 0x10))
            ((int *)**(undefined4 **)(unaff_EBX + 0x52c9f7),&this->super_CGameEventListener,
             unaff_EBX + 0x3923aa,1);
  return;
}


/* CInfo_Player_Ping_Detector::Disable at 0071a5b0 */

/* DWARF original prototype: void Disable(CInfo_Player_Ping_Detector * this) */

void __thiscall CInfo_Player_Ping_Detector::Disable(CInfo_Player_Ping_Detector *this)

{
  int *piVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  this->m_bEnabled = false;
  if ((this->super_CGameEventListener).m_bRegisteredForEvents != false) {
    piVar1 = (int *)**(int **)(unaff_EBX + 0x52c5c7);
    if (piVar1 != (int *)0x0) {
      (**(code **)(*piVar1 + 0x18))(piVar1,&this->super_CGameEventListener);
    }
    (this->super_CGameEventListener).m_bRegisteredForEvents = false;
  }
  return;
}


/* _GLOBAL__I__ZN26CInfo_Player_Ping_Detector9m_DataMapE at 000ab090 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN26CInfo_Player_Ping_Detector9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

