/* DWARF-guided pseudocode for game/server/portal2/trigger_ping_detector.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* CEntityFactory<CTriggerPingDetector>::Create at 00775740 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
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
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x564cf4);
  this_00[1].super_CBaseToggle.super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x565050);
  this_00[1].super_CBaseToggle.super_CBaseEntity.m_pfnMoveDone.__delta = 0;
  this_00[1].super_CBaseToggle.super_CBaseEntity.m_Network.super_IServerNetworkable.
  _vptr_IServerNetworkable = (_func_int_varargs **)0xffffffff;
  this_00[1].super_CBaseToggle.super_CBaseEntity.m_Network.super_IEventRegisterCallback.
  _vptr_IEventRegisterCallback = (_func_int_varargs **)0x0;
  this_00[1].super_CBaseToggle.super_CBaseEntity.m_Network.m_pPev = (edict_t *)0x0;
  this_00[1].super_CBaseToggle.super_CBaseEntity.m_Network.m_PVSInfo.m_nAreaNum = -1;
  this_00[1].super_CBaseToggle.super_CBaseEntity.m_Network.m_PVSInfo.m_nAreaNum2 = -1;
  this_00[1].super_CBaseToggle.super_CBaseEntity.m_Network.m_PVSInfo.m_vCenter[0] = 0.0;
  (**(code **)(unaff_EBX + 0x564d68))(this_00,pClassName);
  return &(this_00->super_CBaseToggle).super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CTriggerPingDetector::~CTriggerPingDetector at 00775a00 */

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
       (_func_int_varargs **)(unaff_EBX + 0x564a34);
  (this->super_CGameEventListener).super_IGameEventListener2._vptr_IGameEventListener2 =
       (_func_int_varargs **)(unaff_EBX + 0x564d90);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnBluePlayerPinged).super_CBaseEntityOutput,in_stack_ffffffc8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnOrangePlayerPinged).super_CBaseEntityOutput,in_stack_ffffffc8);
  (this->super_CGameEventListener).super_IGameEventListener2._vptr_IGameEventListener2 =
       (_func_int_varargs **)(unaff_EBX + 0x4d2e74);
  (this->super_CGameEventListener).m_nDebugID = 0xd;
  if ((this->super_CGameEventListener).m_bRegisteredForEvents != false) {
    piVar1 = (int *)**(int **)(unaff_EBX + 0x4d1174);
    if (piVar1 != (int *)0x0) {
      (**(code **)(*piVar1 + 0x18))(piVar1,&this->super_CGameEventListener);
    }
    (this->super_CGameEventListener).m_bRegisteredForEvents = false;
  }
  CBaseTrigger::~CBaseTrigger(&this->super_CBaseTrigger,__in_chrg);
  return;
}


/* CTriggerPingDetector::~CTriggerPingDetector at 00775b20 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
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
       (_func_int_varargs **)(unaff_EBX + 0x564914);
  (this->super_CGameEventListener).super_IGameEventListener2._vptr_IGameEventListener2 =
       (_func_int_varargs **)(CPropVehicleDriveable::ItemPostFrame + unaff_EBX);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnBluePlayerPinged).super_CBaseEntityOutput,(int)in_stack_ffffffc8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnOrangePlayerPinged).super_CBaseEntityOutput,(int)in_stack_ffffffc8);
  pCVar2 = &this->super_CGameEventListener;
  (this->super_CGameEventListener).super_IGameEventListener2._vptr_IGameEventListener2 =
       (_func_int_varargs **)(unaff_EBX + 0x4d2d54);
  (this->super_CGameEventListener).m_nDebugID = 0xd;
  if ((this->super_CGameEventListener).m_bRegisteredForEvents != false) {
    piVar1 = (int *)**(int **)(unaff_EBX + 0x4d1054);
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


/* CTriggerPingDetector::Spawn at 00775800 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CTriggerPingDetector * this) */

void __thiscall CTriggerPingDetector::Spawn(CTriggerPingDetector *this)

{
  undefined1 *puVar1;
  int iVar2;
  ushort uVar3;
  CBaseEdict *this_00;
  ushort *puVar4;
  int *piVar5;
  IChangeInfoAccessor *pIVar6;
  uint uVar7;
  uint uVar8;
  int unaff_EBX;
  uint local_60;
  
  ___i686_get_pc_thunk_bx();
  if ((this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.m_spawnflags.m_Value != 0) {
    if ((this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.m_Network.m_TimerEvent.
        m_bRegistered == false) {
      this_00 = &((this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.m_Network.m_pPev)->
                 super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4d1354);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_007759b4:
            puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x128;
            puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != 0x128) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_007759b4;
                pIVar6->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar7 * 0x14 + local_60 + 1] != 0x128);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4d1354) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4d1354) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x4d1354);
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar2 = *piVar5 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x128;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
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
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d1374) + 0x10))
            ((int *)**(undefined4 **)(unaff_EBX + 0x4d1374),&this->super_CGameEventListener,
             unaff_EBX + 0x336d27,1);
  return;
}


/* CTriggerPingDetector::FireGameEvent at 007754d0 */

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
  iVar5 = _V_strcmp(s1,(char *)(unaff_EBX + 0x33705a));
  if (iVar5 == 0) {
                    /* Unresolved local var: int nPlayerTeam@[???]
                       Unresolved local var: int playerId@[???]
                       Unresolved local var: Vector vecPingPos@[???]
                       Unresolved local var: Ray_t shotRay@[???]
                       Unresolved local var: trace_t tr@[???] */
    iVar5 = (*event->_vptr_IGameEvent[7])(event,unaff_EBX + 0x2e3ee3,0);
    (*event->_vptr_IGameEvent[9])(event,unaff_EBX + 0x3385e1,0);
    fVar1 = (float)extraout_ST0;
    (*event->_vptr_IGameEvent[9])(event,unaff_EBX + 0x33706f,0);
    fVar2 = (float)extraout_ST0_00;
    (*event->_vptr_IGameEvent[9])(event,unaff_EBX + 0x3385eb,0);
    fVar3 = (float)extraout_ST0_01;
    playerIndex = 1;
    do {
                    /* Unresolved local var: CBasePlayer * pPlayer@[???] */
      this_00 = UTIL_PlayerByIndex(playerIndex);
      if (this_00 != (CBasePlayer *)0x0) {
        iVar6 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d169f) + 0x40))
                          ((int *)**(undefined4 **)(unaff_EBX + 0x4d169f),
                           (this_00->super_CBaseCombatCharacter).super_CBaseFlex.
                           super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                           m_Network.m_pPev);
        if (iVar5 == iVar6) {
          iVar5 = CBaseEntity::GetTeamNumber((CBaseEntity *)this_00);
          goto LAB_007755e1;
        }
      }
      playerIndex = playerIndex + 1;
    } while (playerIndex != 0x22);
    iVar5 = -1;
LAB_007755e1:
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
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d16ef) + 0xc))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4d16ef),&local_6c,0xffffffff,this,&local_c0);
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


/* DataMapInit<CTriggerPingDetector> at 000ba4d0 */

datamap_t * DataMapInit<CTriggerPingDetector>(CTriggerPingDetector *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((&DAT_00d96b7c)[unaff_EBX] == '\0') {
    iVar2 = ___cxa_guard_acquire(&DAT_00d96b7c + unaff_EBX);
    if (iVar2 != 0) {
      *(int *)((int)&g_debug_physcannon.super_ConCommandBase.m_pszName + unaff_EBX) =
           unaff_EBX + 0x9fbcb7;
      *(undefined4 *)((int)&g_debug_physcannon.super_ConCommandBase.m_nFlags + unaff_EBX) = 0;
      *(undefined4 *)((int)&g_debug_physcannon.super_IConVar._vptr_IConVar + unaff_EBX) = 0;
      *(undefined4 *)((int)&g_debug_physcannon.m_pParent + unaff_EBX) = 0;
      *(undefined4 *)((int)&g_debug_physcannon.m_pszDefaultValue + unaff_EBX) = 0;
      *(undefined4 *)((int)&g_debug_physcannon.m_Value.m_pszString + unaff_EBX) = 0;
      *(undefined4 *)((int)&g_debug_physcannon.super_ConCommandBase.m_pszHelpString + unaff_EBX) =
           0x14;
      ___cxa_guard_release(&DAT_00d96b7c + unaff_EBX);
      ___cxa_atexit(unaff_EBX + 0x986c2c,0,*(undefined4 *)(&DAT_00b8c670 + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_SetViewOffset_00cb11e8 + unaff_EBX) =
       *(undefined4 *)(&DAT_00b8ce84 + unaff_EBX);
  if (*(char *)((int)&g_debug_physcannon.super_ConCommandBase.m_pNext + unaff_EBX) == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd96b84);
    if (iVar2 != 0) {
      uVar1 = **(undefined4 **)(&DAT_00b8c690 + unaff_EBX);
      *(undefined4 *)((int)&PTR_VPhysicsIsFlesh_00cb1250 + unaff_EBX) = uVar1;
      *(undefined4 *)((int)&PTR_NetworkStateChanged_m_vecBaseVelocity_00cb1290 + unaff_EBX) = uVar1;
      ___cxa_guard_release(unaff_EBX + 0xd96b84);
    }
  }
  *(undefined4 *)((int)&PTR_GetVectors_00cb11e0 + unaff_EBX) = 2;
  *(int *)((int)&PTR_HeadTarget_00cb11dc + unaff_EBX) =
       (int)&PTR_VPhysicsShadowUpdate_00cb123c + unaff_EBX;
  return (datamap_t *)((int)&PTR_HeadTarget_00cb11dc + unaff_EBX);
}


/* __static_initialization_and_destruction_0 at 000ba5f0 */

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
   ((int)&use_server_portal_particles.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) = 0;
  *(undefined1 *)
   ((int)&use_server_portal_particles.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&use_server_portal_particles.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 2) = 0;
  *(undefined1 *)
   ((int)&use_server_portal_particles.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&use_server_portal_particles.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) =
       0;
  *(undefined4 *)
   ((int)&use_server_portal_particles.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) = 0;
  *(undefined4 *)((int)&use_server_portal_particles.m_fnChangeCallbacks.m_Size + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&use_server_portal_particles.m_fnChangeCallbacks.m_pElements + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)(&DAT_00d96a38 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00d96a3c + unaff_EBX) = 0;
  *(undefined4 *)
   ((int)&max_hitbox_damage_effects_per_entity.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX
   ) = 0;
  *(undefined4 *)
   ((int)&max_hitbox_damage_effects_per_entity.super_ConCommandBase.m_pNext + unaff_EBX) = 0;
  *(undefined4 *)
   (&max_hitbox_damage_effects_per_entity.super_ConCommandBase.m_bRegistered + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&max_hitbox_damage_effects_per_entity.super_ConCommandBase.m_pszName + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&max_hitbox_damage_effects_per_entity.super_ConCommandBase.m_pszHelpString + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&max_hitbox_damage_effects_per_entity.super_ConCommandBase.m_nFlags + unaff_EBX) =
       0x7f7fffff;
  *(undefined **)
   ((int)&max_hitbox_damage_effects_per_entity.super_IConVar._vptr_IConVar + unaff_EBX) =
       &UNK_00b91d0c + unaff_EBX;
  pdVar1 = DataMapInit<CTriggerPingDetector>((CTriggerPingDetector *)0x0);
  *(datamap_t **)((int)s_ElementNames + unaff_EBX + 0x1a44) = pdVar1;
  *(undefined **)((int)&max_hitbox_damage_effects_per_entity.m_pParent + unaff_EBX) =
       &UNK_00c201fc + unaff_EBX;
  pIVar2 = EntityFactoryDictionary();
  (**pIVar2->_vptr_IEntityFactoryDictionary)(pIVar2,unaff_EBX + 0xd96a5c,unaff_EBX + 0x9fbbb4);
  return;
}


/* CTriggerPingDetector::GetDataDescMap at 00775460 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CTriggerPingDetector * this) */

datamap_t * __thiscall CTriggerPingDetector::GetDataDescMap(CTriggerPingDetector *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5f6258);
}


/* CTriggerPingDetector::GetBaseMap at 00775470 */

datamap_t * CTriggerPingDetector::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4d1ef0);
}


/* __tcf_0 at 00a41110 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x40ff66)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x40ff5a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x40ff66));
  }
  *(undefined4 *)(unaff_EBX + 0x40ff66) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40ff62)) {
    if (*(int *)(unaff_EBX + 0x40ff5a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x205a3e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x205a3e),*(int *)(unaff_EBX + 0x40ff5a));
      *(undefined4 *)(unaff_EBX + 0x40ff5a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40ff5e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40ff5a);
  *(int *)(unaff_EBX + 0x40ff6a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40ff62)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x205a3e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x205a3e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40ff5a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40ff5e) = 0;
  }
  return;
}


/* _GLOBAL__I__ZN20CTriggerPingDetector9m_DataMapE at 000ba6e0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN20CTriggerPingDetector9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

