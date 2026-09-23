/* DWARF-guided pseudocode for game/server/portal2/trigger_playerteam.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* CEntityFactory<CTriggerPlayerTeam>::Create at 00775e30 */

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
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5649e4);
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
  (**(code **)(unaff_EBX + 0x564a58))(this_00,pClassName);
  return &(this_00->super_CBaseToggle).super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CTriggerPlayerTeam::~CTriggerPlayerTeam at 00776260 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CTriggerPlayerTeam(CTriggerPlayerTeam * this, int __in_chrg) */

void __thiscall CTriggerPlayerTeam::~CTriggerPlayerTeam(CTriggerPlayerTeam *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x5645bb);
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


/* CTriggerPlayerTeam::~CTriggerPlayerTeam at 00776320 */

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
       (_func_int_varargs **)(unaff_EBX + 0x5644fb);
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


/* CTriggerPlayerTeam::Spawn at 00775f00 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CTriggerPlayerTeam * this) */

void __thiscall CTriggerPlayerTeam::Spawn(CTriggerPlayerTeam *this)

{
  undefined1 *puVar1;
  int iVar2;
  ushort uVar3;
  CBaseEdict *pCVar4;
  ushort *puVar5;
  int *piVar6;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  uint uVar9;
  int unaff_EBX;
  uint local_74;
  uint local_70;
  
  ___i686_get_pc_thunk_bx();
  if ((this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.m_spawnflags.m_Value != 0) {
    if ((this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.m_Network.m_TimerEvent.
        m_bRegistered == false) {
      pCVar4 = &((this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.m_Network.m_pPev)->
                super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4d0c51);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00776168:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x128;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 0x128) {
            local_74 = 0;
            do {
              uVar9 = local_74 + 1;
              local_74 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 == 0x13) goto LAB_0077623d;
                goto LAB_00776168;
              }
            } while (puVar5[uVar8 * 0x14 + local_74 + 1] != 0x128);
          }
        }
        else if (puVar5[0x7d1] == 100) {
LAB_0077623d:
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4d0c51) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4d0c51) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x4d0c51);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
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
  if ((this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.m_Network.m_TimerEvent.
      m_bRegistered == false) {
    pCVar4 = &((this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.m_Network.m_pPev)->
              super_CBaseEdict;
    if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
      pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
      pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
      puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4d0c51);
      if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar3 = puVar5[uVar8 * 0x14 + 0x14];
        if (uVar3 == 0) {
LAB_00776204:
          puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x128;
          puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
        }
        else if (puVar5[uVar8 * 0x14 + 1] != 0x128) {
          local_70 = 0;
          do {
            uVar9 = local_70 + 1;
            local_70 = uVar9 & 0xffff;
            if ((ushort)uVar9 == uVar3) {
              if (uVar3 == 0x13) goto LAB_00776229;
              goto LAB_00776204;
            }
          } while (puVar5[uVar8 * 0x14 + local_70 + 1] != 0x128);
        }
      }
      else if (puVar5[0x7d1] == 100) {
LAB_00776229:
        pIVar7->m_iChangeInfoSerialNumber = 0;
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar7->m_iChangeInfo = puVar5[0x7d1];
        *(short *)(**(int **)(unaff_EBX + 0x4d0c51) + 0xfa2) =
             *(short *)(**(int **)(unaff_EBX + 0x4d0c51) + 0xfa2) + 1;
        piVar6 = *(int **)(unaff_EBX + 0x4d0c51);
        pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
        iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar2 + 2) = 0x128;
        *(undefined2 *)(iVar2 + 0x28) = 1;
      }
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


/* CTriggerPlayerTeam::StartTouch at 00775d60 */

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


/* CTriggerPlayerTeam::EndTouch at 00775ca0 */

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


/* DataMapInit<CTriggerPlayerTeam> at 000ba700 */

datamap_t * DataMapInit<CTriggerPlayerTeam>(CTriggerPlayerTeam *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((&portal2_portal_width.m_bHasMax)[unaff_EBX] == false) {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd969bc);
    if (iVar2 != 0) {
      *(int *)((int)&portal2_portal_width.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) =
           unaff_EBX + 0x9fbb1d;
      *(undefined4 *)((int)&portal2_portal_width.m_fnChangeCallbacks.m_pElements + unaff_EBX) = 0;
      *(undefined4 *)(&DAT_00d969d8 + unaff_EBX) = 0;
      *(undefined4 *)(&DAT_00d969dc + unaff_EBX) = 0;
      *(undefined4 *)
       ((int)&use_server_portal_particles.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) = 0
      ;
      *(undefined4 *)((int)&use_server_portal_particles.super_ConCommandBase.m_pNext + unaff_EBX) =
           0;
      *(undefined4 *)((int)&portal2_portal_width.m_fnChangeCallbacks.m_Size + unaff_EBX) = 0x12;
      ___cxa_guard_release(unaff_EBX + 0xd969bc);
      ___cxa_atexit(unaff_EBX + 0x986adc,0,*(undefined4 *)(&DAT_00b8c440 + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_ShouldAttractAutoAim_00cb1098 + unaff_EBX) =
       *(undefined4 *)(&DAT_00b8cc54 + unaff_EBX);
  if (*(char *)((int)&portal2_portal_width.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) ==
      '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd969c4);
    if (iVar2 != 0) {
      uVar1 = **(undefined4 **)(&DAT_00b8c460 + unaff_EBX);
      *(undefined4 *)((int)&PTR_DoImpactEffect_00cb1180 + unaff_EBX) = uVar1;
      *(undefined4 *)((int)&PTR_GetDamage_00cb11c0 + unaff_EBX) = uVar1;
      *(undefined4 *)((int)&PTR_CanBeSeenBy_00cb1200 + unaff_EBX) = uVar1;
      *(undefined4 *)((int)&PTR_VPhysicsCollision_00cb1240 + unaff_EBX) = uVar1;
      ___cxa_guard_release(unaff_EBX + 0xd969c4);
    }
  }
  *(undefined4 *)((int)&PTR_Classify_00cb1090 + unaff_EBX) = 6;
  *(int *)((int)&PTR_DispatchResponse_00cb108c + unaff_EBX) =
       (int)&PTR_DecalTrace_00cb10ec + unaff_EBX;
  return (datamap_t *)((int)&PTR_DispatchResponse_00cb108c + unaff_EBX);
}


/* __static_initialization_and_destruction_0 at 000ba830 */

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
  *(undefined1 *)((int)&DT_BasePortalCombatWeapon::g_SendTableInit + unaff_EBX) = 0;
  *(undefined1 *)((int)&DT_BasePortalCombatWeapon::g_SendTableInit + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&DT_BasePortalCombatWeapon::g_SendTableInit + unaff_EBX + 2) = 0;
  *(undefined1 *)((int)&DT_BasePortalCombatWeapon::g_SendTableInit + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&CBasePortalCombatWeapon_DataDescInit::g_DataMapHolder + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00d9685c + unaff_EBX) = 0;
  *(undefined4 *)
   ((int)&sv_weapon_pickup_time_delay.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&sv_weapon_pickup_time_delay.super_ConCommandBase.m_pNext + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)(&sv_weapon_pickup_time_delay.super_ConCommandBase.m_bRegistered + unaff_EBX) = 0;
  *(undefined4 *)((int)&sv_weapon_pickup_time_delay.super_ConCommandBase.m_pszName + unaff_EBX) = 0;
  *(undefined4 *)
   ((int)&sv_weapon_pickup_time_delay.super_ConCommandBase.m_pszHelpString + unaff_EBX) = 0;
  *(undefined4 *)((int)&sv_weapon_pickup_time_delay.super_ConCommandBase.m_nFlags + unaff_EBX) = 0;
  *(undefined4 *)((int)&sv_weapon_pickup_time_delay.super_IConVar._vptr_IConVar + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&sv_weapon_pickup_time_delay.m_pParent + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&sv_weapon_pickup_time_delay.m_pszDefaultValue + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&sv_weapon_pickup_time_delay.m_Value.m_pszString + unaff_EBX) = 0x7f7fffff;
  *(undefined **)((int)&sv_weapon_pickup_time_delay.m_Value.m_StringLength + unaff_EBX) =
       &UNK_00b91acc + unaff_EBX;
  pdVar1 = DataMapInit<CTriggerPlayerTeam>((CTriggerPlayerTeam *)0x0);
  *(datamap_t **)((int)s_ElementNames + (int)(&UNK_00001808 + unaff_EBX)) = pdVar1;
  *(undefined **)((int)&sv_weapon_pickup_time_delay.m_Value.m_fValue + unaff_EBX) =
       &UNK_00c20354 + unaff_EBX;
  pIVar2 = EntityFactoryDictionary();
  (**pIVar2->_vptr_IEntityFactoryDictionary)(pIVar2,unaff_EBX + 0xd9688c,unaff_EBX + 0x9fba08);
  return;
}


/* CTriggerPlayerTeam::GetDataDescMap at 00775c40 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CTriggerPlayerTeam * this) */

datamap_t * __thiscall CTriggerPlayerTeam::GetDataDescMap(CTriggerPlayerTeam *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5f5b58);
}


/* CTriggerPlayerTeam::GetBaseMap at 00775c50 */

datamap_t * CTriggerPlayerTeam::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4d1710);
}


/* __tcf_0 at 00a411f0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x40fef6)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x40feea) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x40fef6));
  }
  *(undefined4 *)(unaff_EBX + 0x40fef6) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40fef2)) {
    if (*(int *)(unaff_EBX + 0x40feea) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20595e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20595e),*(int *)(unaff_EBX + 0x40feea));
      *(undefined4 *)(unaff_EBX + 0x40feea) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40feee) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40feea);
  *(int *)(unaff_EBX + 0x40fefa) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40fef2)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20595e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20595e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40feea) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40feee) = 0;
  }
  return;
}


/* _GLOBAL__I__ZN18CTriggerPlayerTeam9m_DataMapE at 000ba920 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN18CTriggerPlayerTeam9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

