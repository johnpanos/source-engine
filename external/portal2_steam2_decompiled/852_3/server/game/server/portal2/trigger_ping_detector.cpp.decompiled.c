/* DWARF-guided pseudocode for game/server/portal2/trigger_ping_detector.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* CEntityFactory<CTriggerPingDetector>::Create at 006fa5a0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CTriggerPingDetector> *
   this, char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CTriggerPingDetector>::Create
          (CEntityFactory<CTriggerPingDetector> *this,char *pClassName)

{
  CBaseTrigger *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CTriggerPingDetector * pEnt@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CBaseEntity::operator_new(0x524);
  CBaseTrigger::CBaseTrigger(this_00);
  *(undefined1 *)&this_00[1].super_CBaseToggle.super_CBaseEntity.m_pfnMoveDone.__pfn = 0;
  this_00[1].super_CBaseToggle.super_CBaseEntity.m_iObjectCapsCache.m_Value = 0x2a;
  (this_00->super_CBaseToggle).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5463f4);
  this_00[1].super_CBaseToggle.super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x546750);
  this_00[1].super_CBaseToggle.super_CBaseEntity.m_pfnMoveDone.__delta = 0;
  this_00[1].super_CBaseToggle.super_CBaseEntity.m_Network.super_IServerNetworkable.
  _vptr_IServerNetworkable = (_func_int_varargs **)0xffffffff;
  this_00[1].super_CBaseToggle.super_CBaseEntity.m_Network.super_IEventRegisterCallback.
  _vptr_IEventRegisterCallback = (_func_int_varargs **)0x0;
  this_00[1].super_CBaseToggle.super_CBaseEntity.m_Network.m_pPev = (edict_t *)0x0;
  this_00[1].super_CBaseToggle.super_CBaseEntity.m_Network.m_PVSInfo.m_nAreaNum = -1;
  this_00[1].super_CBaseToggle.super_CBaseEntity.m_Network.m_PVSInfo.m_nAreaNum2 = -1;
  this_00[1].super_CBaseToggle.super_CBaseEntity.m_Network.m_PVSInfo.m_vCenter[0] = 0.0;
  (**(code **)(unaff_EBX + 0x546468))(this_00,pClassName);
  return &(this_00->super_CBaseToggle).super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CTriggerPingDetector::~CTriggerPingDetector at 006fa8f0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CTriggerPingDetector(CTriggerPingDetector * this, int __in_chrg)
    */

void __thiscall
CTriggerPingDetector::~CTriggerPingDetector(CTriggerPingDetector *this,int __in_chrg)

{
  int *piVar1;
  int unaff_EBX;
  CGameEventListener *in_stack_ffffffc8;
  CGameEventListener *pCVar2;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x5460a4);
  (this->super_CGameEventListener).super_IGameEventListener2._vptr_IGameEventListener2 =
       (_func_int_varargs **)(unaff_EBX + 0x546400);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnBluePlayerPinged).super_CBaseEntityOutput,(int)in_stack_ffffffc8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnOrangePlayerPinged).super_CBaseEntityOutput,(int)in_stack_ffffffc8);
  pCVar2 = &this->super_CGameEventListener;
  (this->super_CGameEventListener).super_IGameEventListener2._vptr_IGameEventListener2 =
       (_func_int_varargs **)(&UNK_004b2fa4 + unaff_EBX);
  (this->super_CGameEventListener).m_nDebugID = 0xd;
  if ((this->super_CGameEventListener).m_bRegisteredForEvents != false) {
    piVar1 = (int *)**(int **)(unaff_EBX + 0x4b12bc);
    if (piVar1 != (int *)0x0) {
      (**(code **)(*piVar1 + 0x18))(piVar1);
      in_stack_ffffffc8 = pCVar2;
    }
    (this->super_CGameEventListener).m_bRegisteredForEvents = false;
  }
  CBaseTrigger::~CBaseTrigger(&this->super_CBaseTrigger,(int)in_stack_ffffffc8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CTriggerPingDetector::~CTriggerPingDetector at 006faa20 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CTriggerPingDetector(CTriggerPingDetector * this, int __in_chrg,
   CTriggerPingDetector * this) */

void __thiscall
CTriggerPingDetector::~CTriggerPingDetector
          (CTriggerPingDetector *this,int __in_chrg,CTriggerPingDetector *this_1)

{
  int *piVar1;
  int unaff_EBX;
  int in_stack_ffffffc8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x545f74);
  (this->super_CGameEventListener).super_IGameEventListener2._vptr_IGameEventListener2 =
       (_func_int_varargs **)(unaff_EBX + 0x5462d0);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnBluePlayerPinged).super_CBaseEntityOutput,in_stack_ffffffc8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnOrangePlayerPinged).super_CBaseEntityOutput,in_stack_ffffffc8);
  (this->super_CGameEventListener).super_IGameEventListener2._vptr_IGameEventListener2 =
       (_func_int_varargs **)(unaff_EBX + 0x4b2e74);
  (this->super_CGameEventListener).m_nDebugID = 0xd;
  if ((this->super_CGameEventListener).m_bRegisteredForEvents != false) {
    piVar1 = (int *)**(int **)(unaff_EBX + 0x4b118c);
    if (piVar1 != (int *)0x0) {
      (**(code **)(*piVar1 + 0x18))(piVar1,&this->super_CGameEventListener);
    }
    (this->super_CGameEventListener).m_bRegisteredForEvents = false;
  }
  CBaseTrigger::~CBaseTrigger(&this->super_CBaseTrigger,__in_chrg);
  return;
}


/* CTriggerPingDetector::Spawn at 006fa4f0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CTriggerPingDetector * this) */

void __thiscall CTriggerPingDetector::Spawn(CTriggerPingDetector *this)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.m_spawnflags.m_Value != 0) {
    if ((this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.m_Network.m_TimerEvent.
        m_bRegistered == false) {
      this_00 = &((this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.m_Network.m_pPev)->
                 super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar2->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.m_spawnflags.m_Value = 0;
  }
  CBaseTrigger::Spawn(&this->super_CBaseTrigger);
  CBaseTrigger::InitTrigger(&this->super_CBaseTrigger);
                    /* Unresolved local var: bool bServerSide@[???] */
  (this->super_CGameEventListener).m_bRegisteredForEvents = true;
  (**(code **)(*(int *)**(undefined4 **)(&DAT_004b16c3 + unaff_EBX) + 0x10))
            ((int *)**(undefined4 **)(&DAT_004b16c3 + unaff_EBX),&this->super_CGameEventListener,
             &UNK_00311a0d + unaff_EBX,1);
  return;
}


/* CTriggerPingDetector::FireGameEvent at 006fa670 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void FireGameEvent(CTriggerPingDetector * this, IGameEvent * event) */

void __thiscall CTriggerPingDetector::FireGameEvent(CTriggerPingDetector *this,IGameEvent *event)

{
  float fVar1;
  float fVar2;
  float fVar3;
  bool bVar4;
  char *s1;
  int iVar5;
  CBasePlayer *this_00;
  int iVar6;
  int unaff_EBX;
  int playerIndex;
  longdouble extraout_ST0;
  longdouble extraout_ST0_00;
  longdouble extraout_ST0_01;
  CGameTrace local_c0;
  float local_6c;
  float local_68;
  float local_64;
  float local_5c;
  float local_58;
  float local_54;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_2c;
  undefined1 local_28;
  undefined1 local_27;
  
                    /* Unresolved local var: char * name@[???] */
  ___i686_get_pc_thunk_bx();
  s1 = (char *)(*event->_vptr_IGameEvent[2])(event);
  iVar5 = _V_strcmp(s1,(char *)(unaff_EBX + 0x311889));
  if (iVar5 == 0) {
                    /* Unresolved local var: int nPlayerTeam@[???]
                       Unresolved local var: int playerId@[???]
                       Unresolved local var: Vector vecPingPos@[???]
                       Unresolved local var: Ray_t shotRay@[???]
                       Unresolved local var: trace_t tr@[???] */
    iVar5 = (*event->_vptr_IGameEvent[7])(event,unaff_EBX + 0x2bbb0b,0);
    (*event->_vptr_IGameEvent[9])(event,unaff_EBX + 0x310330,0);
    fVar1 = (float)extraout_ST0;
    (*event->_vptr_IGameEvent[9])(event,unaff_EBX + 0x310337,0);
    fVar2 = (float)extraout_ST0_00;
    (*event->_vptr_IGameEvent[9])(event,unaff_EBX + 0x31033e,0);
    fVar3 = (float)extraout_ST0_01;
    playerIndex = 1;
    do {
                    /* Unresolved local var: CBasePlayer * pPlayer@[???] */
      this_00 = UTIL_PlayerByIndex(playerIndex);
      if (this_00 != (CBasePlayer *)0x0) {
        iVar6 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4b1537) + 0x40))
                          ((int *)**(undefined4 **)(unaff_EBX + 0x4b1537),
                           (this_00->super_CBaseCombatCharacter).super_CBaseFlex.
                           super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                           m_Network.m_pPev);
        if (iVar5 == iVar6) {
          iVar5 = CBaseEntity::GetTeamNumber((CBaseEntity *)this_00);
          goto LAB_006fa781;
        }
      }
      playerIndex = playerIndex + 1;
    } while (playerIndex != 0x22);
    iVar5 = -1;
LAB_006fa781:
    local_5c = fVar1 - fVar1;
    local_58 = fVar2 - fVar2;
    local_54 = fVar3 - fVar3;
    local_27 = local_5c * local_5c + local_58 * local_58 + local_54 * local_54 != 0.0;
    local_34 = 0;
    local_38 = 0;
    local_3c = 0;
    local_2c = 0;
    local_28 = 1;
    local_44 = 0;
    local_48 = 0;
    local_4c = 0;
    local_6c = fVar1;
    local_68 = fVar2;
    local_64 = fVar3;
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4b158b) + 0xc))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4b158b),&local_6c,0xffffffff,this,&local_c0);
    bVar4 = CGameTrace::DidHitNonWorldEntity(&local_c0);
    if ((bVar4) && ((CTriggerPingDetector *)local_c0.m_pEnt == this)) {
      if (iVar5 == 2) {
        COutputEvent::FireOutput
                  (&this->m_OnOrangePlayerPinged,(CBaseEntity *)0x0,(CBaseEntity *)0x0,0.0);
      }
      else {
        COutputEvent::FireOutput
                  (&this->m_OnBluePlayerPinged,(CBaseEntity *)0x0,(CBaseEntity *)0x0,0.0);
      }
    }
  }
  return;
}


/* DataMapInit<CTriggerPingDetector> at 000cf640 */

datamap_t * DataMapInit<CTriggerPingDetector>(CTriggerPingDetector *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)predef_flexcontroller_values[2] + unaff_EBX + 100) == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcec894);
    if (iVar2 != 0) {
      *(int *)((int)predef_flexcontroller_values[2] + unaff_EBX + 0x74) = unaff_EBX + 0x944653;
      *(undefined4 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 4) = 0;
      *(undefined4 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 8) = 0;
      *(undefined4 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0xc) = 0;
      *(undefined4 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x10) = 0;
      *(undefined4 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x14) = 0;
      *(undefined4 *)((int)predef_flexcontroller_values[3] + unaff_EBX) = 0x14;
      ___cxa_guard_release(unaff_EBX + 0xcec894);
      ___cxa_atexit(unaff_EBX + 0x8ce32c,0,*(undefined4 *)(&DAT_00adc53c + unaff_EBX));
    }
  }
  *(undefined4 *)(&DAT_00c01c38 + unaff_EBX) = *(undefined4 *)(&DAT_00adcd40 + unaff_EBX);
  if (*(char *)((int)predef_flexcontroller_values[2] + unaff_EBX + 0x6c) == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcec89c);
    if (iVar2 != 0) {
      uVar1 = **(undefined4 **)(&DAT_00adc558 + unaff_EBX);
      *(undefined4 *)(&CGenericCycler::vtable + unaff_EBX) = uVar1;
      *(undefined4 *)((int)&PTR_GetAIAddOn_00c01ce0 + unaff_EBX) = uVar1;
      ___cxa_guard_release(unaff_EBX + 0xcec89c);
    }
  }
  *(undefined4 *)((int)&PTR_BehaviorBridge_GetJumpGravity_00c01c30 + unaff_EBX) = 2;
  *(int *)((int)&PTR_BehaviorBridge_TranslateSchedule_00c01c2c + unaff_EBX) =
       (int)&PTR_typeinfo_00c01c8c + unaff_EBX;
  return (datamap_t *)((int)&PTR_BehaviorBridge_TranslateSchedule_00c01c2c + unaff_EBX);
}


/* __static_initialization_and_destruction_0 at 000cf760 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  bool *pbVar1;
  datamap_t *pdVar2;
  IEntityFactoryDictionary *pIVar3;
  int unaff_EBX;
  longlong lVar4;
  
  lVar4 = ___i686_get_pc_thunk_bx();
  if (lVar4 != 0xffff00000001) {
    return;
  }
  (&CBaseEntity::sm_bDisableTouchFuncs)[unaff_EBX] = false;
  (&DAT_00cec715)[unaff_EBX] = 0;
  (&DAT_00cec716)[unaff_EBX] = 0;
  (&DAT_00cec717)[unaff_EBX] = 0;
  *(undefined4 *)((int)&CBaseEntity::m_nPredictionRandomSeed + unaff_EBX) = 0;
  *(undefined4 *)((int)&CBaseEntity::m_pPredictionPlayer + unaff_EBX) = 0;
  *(undefined4 *)((int)&g_nInsideDispatchUpdateTransmitState + unaff_EBX) = 0x7f7fffff;
  pbVar1 = &CBaseEntity::s_bAbsQueriesValid + unaff_EBX;
  pbVar1[0] = true;
  pbVar1[1] = true;
  pbVar1[2] = true;
  pbVar1[3] = true;
  *(undefined4 *)
   ((int)&CUtlRBTree<CBaseFlex::FS_LocalToGlobal_t,unsigned_short,bool(*)(CBaseFlex::FS_LocalToGlobal_t_const&,CBaseFlex::FS_LocalToGlobal_t_const&),CUtlMemory<UtlRBTreeNode_t<CBaseFlex::FS_LocalToGlobal_t,unsigned_short>,unsigned_short>>
          ::Links(unsigned_short)::s_Sentinel + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00cec72c + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00cec730 + unaff_EBX) = 0;
  *(undefined4 *)
   ((int)&CUtlRBTree<CBaseFlex::FS_LocalToGlobal_t,unsigned_short,bool(*)(CBaseFlex::FS_LocalToGlobal_t_const&,CBaseFlex::FS_LocalToGlobal_t_const&),CUtlMemory<UtlRBTreeNode_t<CBaseFlex::FS_LocalToGlobal_t,unsigned_short>,unsigned_short>>
          ::Links(unsigned_short)::s_Sentinel + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00cec738 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00cec73c + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)predef_flexcontroller_values[0] + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)predef_flexcontroller_values[0] + unaff_EBX + 4) = 0x7f7fffff;
  *(undefined4 *)((int)predef_flexcontroller_values[0] + unaff_EBX + 8) = 0;
  *(undefined1 *)((int)predef_flexcontroller_values[0] + unaff_EBX + 0xf) = 0;
  *(undefined1 *)((int)predef_flexcontroller_values[0] + unaff_EBX + 0xc) = 0;
  *(undefined1 *)((int)predef_flexcontroller_values[0] + unaff_EBX + 0xd) = 0;
  *(undefined1 *)((int)predef_flexcontroller_values[0] + unaff_EBX + 0xe) = 0;
  *(undefined4 *)((int)predef_flexcontroller_values[0] + unaff_EBX + 0x10) = 0;
  *(undefined1 *)((int)predef_flexcontroller_values[0] + unaff_EBX + 0x17) = 0;
  *(undefined1 *)((int)predef_flexcontroller_values[0] + unaff_EBX + 0x14) = 0;
  *(undefined1 *)((int)predef_flexcontroller_values[0] + unaff_EBX + 0x15) = 0;
  *(undefined1 *)((int)predef_flexcontroller_values[0] + unaff_EBX + 0x16) = 0;
  *(undefined4 *)((int)predef_flexcontroller_values[0] + unaff_EBX + 0x18) = 0;
  *(undefined1 *)((int)predef_flexcontroller_values[0] + unaff_EBX + 0x1f) = 1;
  *(undefined1 *)((int)predef_flexcontroller_values[0] + unaff_EBX + 0x1c) = 0;
  *(undefined1 *)((int)predef_flexcontroller_values[0] + unaff_EBX + 0x1d) = 0;
  *(undefined1 *)((int)predef_flexcontroller_values[0] + unaff_EBX + 0x1e) = 0;
  *(undefined4 *)((int)predef_flexcontroller_values[0] + unaff_EBX + 0x20) = 1;
  *(undefined1 *)((int)predef_flexcontroller_values[0] + unaff_EBX + 0x27) = 0;
  *(undefined1 *)((int)predef_flexcontroller_values[0] + unaff_EBX + 0x24) = 0;
  *(undefined1 *)((int)predef_flexcontroller_values[0] + unaff_EBX + 0x25) = 0;
  *(undefined1 *)((int)predef_flexcontroller_values[0] + unaff_EBX + 0x26) = 0;
  *(undefined4 *)((int)predef_flexcontroller_values[0] + unaff_EBX + 0x28) = 2;
  *(undefined1 *)((int)predef_flexcontroller_values[0] + unaff_EBX + 0x2f) = 0;
  *(undefined1 *)((int)predef_flexcontroller_values[0] + unaff_EBX + 0x2c) = 0;
  *(undefined1 *)((int)predef_flexcontroller_values[0] + unaff_EBX + 0x2d) = 0;
  *(undefined1 *)((int)predef_flexcontroller_values[0] + unaff_EBX + 0x2e) = 0;
  *(undefined **)((int)predef_flexcontroller_values[0] + unaff_EBX + 0x30) =
       &UNK_00ae1bbc + unaff_EBX;
  pdVar2 = DataMapInit<CTriggerPingDetector>((CTriggerPingDetector *)0x0);
  *(datamap_t **)((int)&PTR_HasHotkey_00c60fd4 + unaff_EBX) = pdVar2;
  *(undefined **)((int)predef_flexcontroller_values[0] + unaff_EBX + 0x34) =
       &UNK_00b715ec + unaff_EBX;
  pIVar3 = EntityFactoryDictionary();
  (**pIVar3->_vptr_IEntityFactoryDictionary)(pIVar3,unaff_EBX + 0xcec774,unaff_EBX + 0x944550);
  return;
}


/* CTriggerPingDetector::GetDataDescMap at 006fa490 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CTriggerPingDetector * this) */

datamap_t * __thiscall CTriggerPingDetector::GetDataDescMap(CTriggerPingDetector *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5d6de8);
}


/* CTriggerPingDetector::GetBaseMap at 006fa4a0 */

datamap_t * CTriggerPingDetector::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4b1eec);
}


/* __tcf_0 at 0099d980 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41e57e)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41e572) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41e57e));
  }
  *(undefined4 *)(unaff_EBX + 0x41e57e) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41e57a)) {
    if (*(int *)(unaff_EBX + 0x41e572) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20e20a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20e20a),*(int *)(unaff_EBX + 0x41e572));
      *(undefined4 *)(unaff_EBX + 0x41e572) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e576) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41e572);
  *(int *)(unaff_EBX + 0x41e582) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41e57a)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20e20a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20e20a),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41e572) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e576) = 0;
  }
  return;
}


/* _GLOBAL__I__ZN20CTriggerPingDetector9m_DataMapE at 000cf8f0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN20CTriggerPingDetector9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

