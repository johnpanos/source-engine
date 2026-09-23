/* DWARF-guided pseudocode for game/server/portal2/trigger_playerteam.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* CEntityFactory<CTriggerPlayerTeam>::Create at 006fad20 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CTriggerPlayerTeam> * this,
   char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CTriggerPlayerTeam>::Create
          (CEntityFactory<CTriggerPlayerTeam> *this,char *pClassName)

{
  CServerNetworkProperty *pCVar1;
  CBaseTrigger *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CTriggerPlayerTeam * pEnt@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CBaseEntity::operator_new(0x550);
  CBaseTrigger::CBaseTrigger(this_00);
  (this_00->super_CBaseToggle).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x546054);
  this_00[1].super_CBaseToggle.super_CBaseEntity.m_pfnMoveDone.__pfn =
       (_func___thiscall_void_CBaseEntity_ptr *)0x0;
  this_00[1].super_CBaseToggle.super_CBaseEntity.m_pfnThink.__delta = -1;
  this_00[1].super_CBaseToggle.super_CBaseEntity.m_Network.super_IServerNetworkable.
  _vptr_IServerNetworkable = (_func_int_varargs **)0x0;
  this_00[1].super_CBaseToggle.super_CBaseEntity.m_Network.m_pOuter = (CBaseEntity *)0x0;
  this_00[1].super_CBaseToggle.super_CBaseEntity.m_Network.m_PVSInfo.m_pClusters =
       (ushort *)0xffffffff;
  this_00[1].super_CBaseToggle.super_CBaseEntity.m_Network.m_PVSInfo.m_nAreaNum = 0;
  this_00[1].super_CBaseToggle.super_CBaseEntity.m_Network.m_PVSInfo.m_nAreaNum2 = 0;
  this_00[1].super_CBaseToggle.super_CBaseEntity.m_Network.m_PVSInfo.m_vCenter[1] = 0.0;
  pCVar1 = &this_00[1].super_CBaseToggle.super_CBaseEntity.m_Network;
  (pCVar1->m_PVSInfo).m_pClustersInline[2] = 0xffff;
  (pCVar1->m_PVSInfo).m_pClustersInline[3] = 0xffff;
  this_00[1].super_CBaseToggle.super_CBaseEntity.m_Network.m_pServerClass = (ServerClass *)0x0;
  this_00[1].super_CBaseToggle.super_CBaseEntity.m_Network.m_TimerEvent.m_pEventMgr =
       (CTimedEventMgr *)0x0;
  this_00[1].super_CBaseToggle.super_CBaseEntity.m_Network.m_TimerEvent.m_pCallback =
       (IEventRegisterCallback *)0xffffffff;
  *(undefined4 *)
   &this_00[1].super_CBaseToggle.super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered = 0;
  (**(code **)(unaff_EBX + 0x5460c8))(this_00,pClassName);
  return &(this_00->super_CBaseToggle).super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CTriggerPlayerTeam::~CTriggerPlayerTeam at 006fae90 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CTriggerPlayerTeam(CTriggerPlayerTeam * this, int __in_chrg) */

void __thiscall CTriggerPlayerTeam::~CTriggerPlayerTeam(CTriggerPlayerTeam *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x545eeb);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnEndTouchBluePlayer).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnStartTouchBluePlayer).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnEndTouchOrangePlayer).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnStartTouchOrangePlayer).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseTrigger::~CBaseTrigger(&this->super_CBaseTrigger,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CTriggerPlayerTeam::~CTriggerPlayerTeam at 006faf50 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CTriggerPlayerTeam(CTriggerPlayerTeam * this, int __in_chrg,
   CTriggerPlayerTeam * this) */

void __thiscall
CTriggerPlayerTeam::~CTriggerPlayerTeam
          (CTriggerPlayerTeam *this,int __in_chrg,CTriggerPlayerTeam *this_1)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x545e2b);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnEndTouchBluePlayer).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnStartTouchBluePlayer).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnEndTouchOrangePlayer).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnStartTouchOrangePlayer).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseTrigger::~CBaseTrigger(&this->super_CBaseTrigger,__in_chrg);
  return;
}


/* CTriggerPlayerTeam::Spawn at 006fadf0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CTriggerPlayerTeam * this) */

void __thiscall CTriggerPlayerTeam::Spawn(CTriggerPlayerTeam *this)

{
  undefined1 *puVar1;
  CBaseEdict *pCVar2;
  IChangeInfoAccessor *pIVar3;
  
  if ((this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.m_spawnflags.m_Value != 0) {
    if ((this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.m_Network.m_TimerEvent.
        m_bRegistered == false) {
      pCVar2 = &((this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.m_Network.m_pPev)->
                super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.m_spawnflags.m_Value = 0;
  }
  if ((this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.m_Network.m_TimerEvent.
      m_bRegistered == false) {
    pCVar2 = &((this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.m_Network.m_pPev)->
              super_CBaseEdict;
    if (pCVar2 != (CBaseEdict *)0x0) {
      pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
      pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
      pIVar3->m_iChangeInfoSerialNumber = 0;
    }
  }
  else {
    puVar1 = &(this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  (this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.m_spawnflags.m_Value = 1;
  CBaseTrigger::Spawn(&this->super_CBaseTrigger);
  CBaseTrigger::InitTrigger(&this->super_CBaseTrigger);
  return;
}


/* CTriggerPlayerTeam::StartTouch at 006fac50 */

/* DWARF original prototype: void StartTouch(CTriggerPlayerTeam * this, CBaseEntity * pOther) */

void __thiscall CTriggerPlayerTeam::StartTouch(CTriggerPlayerTeam *this,CBaseEntity *pOther)

{
  int iVar1;
  
  if ((pOther != (CBaseEntity *)0x0) &&
     (iVar1 = (*(pOther->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                _vptr_IHandleEntity[0x55])(pOther), (char)iVar1 != '\0')) {
    if ((0 < this->m_nTargetTeam) &&
       (iVar1 = CBaseEntity::GetTeamNumber(pOther), iVar1 != this->m_nTargetTeam)) {
      return;
    }
    CBaseTrigger::StartTouch(&this->super_CBaseTrigger,pOther);
    iVar1 = CBaseEntity::GetTeamNumber(pOther);
    if (iVar1 == 2) {
      COutputEvent::FireOutput(&this->m_OnStartTouchOrangePlayer,pOther,(CBaseEntity *)this,0.0);
    }
    else {
      iVar1 = CBaseEntity::GetTeamNumber(pOther);
      if (iVar1 == 3) {
        COutputEvent::FireOutput(&this->m_OnStartTouchBluePlayer,pOther,(CBaseEntity *)this,0.0);
      }
    }
    if (this->m_bTriggerOnce != false) {
      UTIL_Remove((CBaseEntity *)this);
      return;
    }
  }
  return;
}


/* CTriggerPlayerTeam::EndTouch at 006fab90 */

/* DWARF original prototype: void EndTouch(CTriggerPlayerTeam * this, CBaseEntity * pOther) */

void __thiscall CTriggerPlayerTeam::EndTouch(CTriggerPlayerTeam *this,CBaseEntity *pOther)

{
  int iVar1;
  
  if ((pOther != (CBaseEntity *)0x0) &&
     (iVar1 = (*(pOther->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                _vptr_IHandleEntity[0x55])(pOther), (char)iVar1 != '\0')) {
    if ((0 < this->m_nTargetTeam) &&
       (iVar1 = CBaseEntity::GetTeamNumber(pOther), iVar1 != this->m_nTargetTeam)) {
      return;
    }
    CBaseTrigger::EndTouch(&this->super_CBaseTrigger,pOther);
    iVar1 = CBaseEntity::GetTeamNumber(pOther);
    if (iVar1 == 2) {
      COutputEvent::FireOutput(&this->m_OnEndTouchOrangePlayer,pOther,(CBaseEntity *)this,0.0);
    }
    else {
      iVar1 = CBaseEntity::GetTeamNumber(pOther);
      if (iVar1 == 3) {
        COutputEvent::FireOutput(&this->m_OnEndTouchBluePlayer,pOther,(CBaseEntity *)this,0.0);
      }
    }
  }
  return;
}


/* DataMapInit<CTriggerPlayerTeam> at 000cf910 */

datamap_t * DataMapInit<CTriggerPlayerTeam>(CTriggerPlayerTeam *param_1)

{
  char *pcVar1;
  undefined4 uVar2;
  int iVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((&DAT_00cec65c)[unaff_EBX] == '\0') {
    iVar3 = ___cxa_guard_acquire(&DAT_00cec65c + unaff_EBX);
    if (iVar3 != 0) {
      *(int *)(CBaseEntity::GetPreTemplateName::szStrippedName + unaff_EBX + 0xc) =
           unaff_EBX + 0x944419;
      pcVar1 = CBaseEntity::GetPreTemplateName::szStrippedName + unaff_EBX + 0x14;
      pcVar1[0] = '\0';
      pcVar1[1] = '\0';
      pcVar1[2] = '\0';
      pcVar1[3] = '\0';
      pcVar1 = CBaseEntity::GetPreTemplateName::szStrippedName + unaff_EBX + 0x18;
      pcVar1[0] = '\0';
      pcVar1[1] = '\0';
      pcVar1[2] = '\0';
      pcVar1[3] = '\0';
      pcVar1 = CBaseEntity::GetPreTemplateName::szStrippedName + unaff_EBX + 0x1c;
      pcVar1[0] = '\0';
      pcVar1[1] = '\0';
      pcVar1[2] = '\0';
      pcVar1[3] = '\0';
      pcVar1 = CBaseEntity::GetPreTemplateName::szStrippedName + unaff_EBX + 0x20;
      pcVar1[0] = '\0';
      pcVar1[1] = '\0';
      pcVar1[2] = '\0';
      pcVar1[3] = '\0';
      pcVar1 = CBaseEntity::GetPreTemplateName::szStrippedName + unaff_EBX + 0x24;
      pcVar1[0] = '\0';
      pcVar1[1] = '\0';
      pcVar1[2] = '\0';
      pcVar1[3] = '\0';
      pcVar1 = CBaseEntity::GetPreTemplateName::szStrippedName + unaff_EBX + 0x10;
      pcVar1[0] = '\x12';
      pcVar1[1] = '\0';
      pcVar1[2] = '\0';
      pcVar1[3] = '\0';
      ___cxa_guard_release(&DAT_00cec65c + unaff_EBX);
      ___cxa_atexit(unaff_EBX + 0x8ce13c,0,*(undefined4 *)(&DAT_00adc26c + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_IsValidCover_00c01a48 + unaff_EBX) =
       *(undefined4 *)(&DAT_00adca70 + unaff_EBX);
  if (CBaseEntity::GetPreTemplateName::szStrippedName[unaff_EBX + 4] == '\0') {
    iVar3 = ___cxa_guard_acquire(CBaseEntity::GetPreTemplateName::szStrippedName + unaff_EBX + 4);
    if (iVar3 != 0) {
      uVar2 = **(undefined4 **)(&DAT_00adc288 + unaff_EBX);
      *(undefined4 *)((int)&PTR_Crouch_00c01b30 + unaff_EBX) = uVar2;
      *(undefined4 *)((int)&PTR_OnCalcBaseMove_00c01b70 + unaff_EBX) = uVar2;
      *(undefined4 *)((int)&PTR_BehaviorBridge_IsValidCover_00c01bb0 + unaff_EBX) = uVar2;
      *(undefined4 *)((int)&PTR_BehaviorBridge_CanRunAScriptedNPCInteraction_00c01bf0 + unaff_EBX) =
           uVar2;
      ___cxa_guard_release(CBaseEntity::GetPreTemplateName::szStrippedName + unaff_EBX + 4);
    }
  }
  *(undefined4 *)((int)&PTR_FindCoverPosInRadius_00c01a40 + unaff_EBX) = 6;
  *(int *)((int)&PTR_FindCoverPos_00c01a3c + unaff_EBX) =
       (int)&PTR_OnGivenWeapon_00c01a9c + unaff_EBX;
  return (datamap_t *)((int)&PTR_FindCoverPos_00c01a3c + unaff_EBX);
}


/* __static_initialization_and_destruction_0 at 000cfa40 */

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
  *(undefined1 *)((int)&rgdpvpreset[0x1a].pitchrun + unaff_EBX) = 0;
  *(undefined1 *)((int)&rgdpvpreset[0x1a].pitchrun + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&rgdpvpreset[0x1a].pitchrun + unaff_EBX + 2) = 0;
  *(undefined1 *)((int)&rgdpvpreset[0x1a].pitchrun + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&rgdpvpreset[0x1a].pitchstart + unaff_EBX) = 0;
  *(undefined4 *)((int)&rgdpvpreset[0x1a].spinup + unaff_EBX) = 0;
  *(undefined4 *)((int)&rgdpvpreset[0x1a].spindown + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&rgdpvpreset[0x1a].volrun + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&rgdpvpreset[0x1a].volstart + unaff_EBX) = 0;
  *(undefined4 *)((int)&rgdpvpreset[0x1a].fadein + unaff_EBX) = 0;
  *(undefined4 *)((int)&rgdpvpreset[0x1a].fadeout + unaff_EBX) = 0;
  *(undefined4 *)((int)&rgdpvpreset[0x1a].lfotype + unaff_EBX) = 0;
  *(undefined4 *)((int)&rgdpvpreset[0x1a].lforate + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&rgdpvpreset[0x1a].lfomodpitch + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&rgdpvpreset[0x1a].lfomodvol + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&rgdpvpreset[0x1a].cspinup + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&rgdpvpreset[0x1a].cspincount + unaff_EBX) = 0;
  *(undefined1 *)((int)&rgdpvpreset[0x1a].pitch + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&rgdpvpreset[0x1a].pitch + unaff_EBX) = 0;
  *(undefined1 *)((int)&rgdpvpreset[0x1a].pitch + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&rgdpvpreset[0x1a].pitch + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&rgdpvpreset[0x1a].spinupsav + unaff_EBX) = 0;
  *(undefined1 *)((int)&rgdpvpreset[0x1a].spindownsav + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&rgdpvpreset[0x1a].spindownsav + unaff_EBX) = 0;
  *(undefined1 *)((int)&rgdpvpreset[0x1a].spindownsav + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&rgdpvpreset[0x1a].spindownsav + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&rgdpvpreset[0x1a].pitchfrac + unaff_EBX) = 0;
  *(undefined1 *)((int)&rgdpvpreset[0x1a].vol + unaff_EBX + 3) = 1;
  *(undefined1 *)((int)&rgdpvpreset[0x1a].vol + unaff_EBX) = 0;
  *(undefined1 *)((int)&rgdpvpreset[0x1a].vol + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&rgdpvpreset[0x1a].vol + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&rgdpvpreset[0x1a].fadeinsav + unaff_EBX) = 1;
  *(undefined1 *)((int)&rgdpvpreset[0x1a].fadeoutsav + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&rgdpvpreset[0x1a].fadeoutsav + unaff_EBX) = 0;
  *(undefined1 *)((int)&rgdpvpreset[0x1a].fadeoutsav + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&rgdpvpreset[0x1a].fadeoutsav + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&rgdpvpreset[0x1a].volfrac + unaff_EBX) = 2;
  *(undefined1 *)((int)&rgdpvpreset[0x1a].lfofrac + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&rgdpvpreset[0x1a].lfofrac + unaff_EBX) = 0;
  *(undefined1 *)((int)&rgdpvpreset[0x1a].lfofrac + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&rgdpvpreset[0x1a].lfofrac + unaff_EBX + 2) = 0;
  *(undefined **)((int)&rgdpvpreset[0x1a].lfomult + unaff_EBX) = &UNK_00ae18dc + unaff_EBX;
  pdVar1 = DataMapInit<CTriggerPlayerTeam>((CTriggerPlayerTeam *)0x0);
  *(datamap_t **)((int)&PTR_OnKeyTyped_00c60cf8 + unaff_EBX) = pdVar1;
  *(undefined **)(&DAT_00cec52c + unaff_EBX) = &UNK_00b716a4 + unaff_EBX;
  pIVar2 = EntityFactoryDictionary();
  (**pIVar2->_vptr_IEntityFactoryDictionary)(pIVar2,&DAT_00cec52c + unaff_EBX,unaff_EBX + 0x944304);
  return;
}


/* CTriggerPlayerTeam::GetDataDescMap at 006fab30 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CTriggerPlayerTeam * this) */

datamap_t * __thiscall CTriggerPlayerTeam::GetDataDescMap(CTriggerPlayerTeam *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5d6828);
}


/* CTriggerPlayerTeam::GetBaseMap at 006fab40 */

datamap_t * CTriggerPlayerTeam::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4b184c);
}


/* __tcf_0 at 0099da60 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41e536)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41e52a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41e536));
  }
  *(undefined4 *)(unaff_EBX + 0x41e536) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41e532)) {
    if (*(int *)(unaff_EBX + 0x41e52a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20e12a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20e12a),*(int *)(unaff_EBX + 0x41e52a));
      *(undefined4 *)(unaff_EBX + 0x41e52a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e52e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41e52a);
  *(int *)(unaff_EBX + 0x41e53a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41e532)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20e12a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20e12a),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41e52a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e52e) = 0;
  }
  return;
}


/* _GLOBAL__I__ZN18CTriggerPlayerTeam9m_DataMapE at 000cfbd0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN18CTriggerPlayerTeam9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

