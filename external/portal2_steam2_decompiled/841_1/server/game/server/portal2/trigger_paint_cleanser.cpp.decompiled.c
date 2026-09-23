/* DWARF-guided pseudocode for game/server/portal2/trigger_paint_cleanser.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* DataMapInit<CTriggerPaintCleanser> at 000ba1f0 */

datamap_t * DataMapInit<CTriggerPaintCleanser>(CTriggerPaintCleanser *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((&player_held_object_offset_up_cube_vm.field_0x3f)[unaff_EBX] == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xd96d9f);
    if (iVar1 != 0) {
      *(int *)(&player_held_object_offset_up_sphere.field_0x3f + unaff_EBX) = unaff_EBX + 0x9fbf2a;
      *(undefined4 *)
       ((int)&player_held_object_offset_up_sphere.m_fnChangeCallbacks.m_Memory.m_pMemory +
       unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&player_held_object_offset_up_sphere.m_fnChangeCallbacks.m_Memory.m_nAllocationCount +
       unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&player_held_object_offset_up_sphere.m_fnChangeCallbacks.m_Memory.m_nGrowSize +
       unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&player_held_object_offset_up_sphere.m_fnChangeCallbacks.m_Size + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&player_held_object_offset_up_sphere.m_fnChangeCallbacks.m_pElements + unaff_EBX + 3) =
           0;
      *(undefined4 *)((int)&player_held_object_offset_up_sphere.m_fMaxVal + unaff_EBX + 3) = 0x15;
      ___cxa_guard_release(unaff_EBX + 0xd96d9f);
      ___cxa_atexit(unaff_EBX + 0x986e2f,0,*(undefined4 *)(&DAT_00b8c953 + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_KeyValue_00cb14a8 + unaff_EBX + 3) =
       *(undefined4 *)(&DAT_00b8d167 + unaff_EBX);
  *(undefined4 *)((int)&PTR_PostClientActive_00cb14a0 + unaff_EBX + 3) = 1;
  *(int *)((int)&PTR_PostConstructor_00cb149c + unaff_EBX + 3) = unaff_EBX + 0xd96dbf;
  return (datamap_t *)((int)&PTR_PostConstructor_00cb149c + unaff_EBX + 3);
}


/* __static_initialization_and_destruction_0 at 000ba2c0 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  code *s2;
  SendTable *this;
  int *piVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  datamap_t *pdVar4;
  int iVar5;
  int iVar6;
  IEntityFactoryDictionary *pIVar7;
  int unaff_EBX;
  longlong lVar8;
  
  lVar8 = ___i686_get_pc_thunk_bx();
  if (lVar8 == 0xffff00000001) {
    *(undefined1 *)((int)&save_paintblob.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) = 0;
    *(undefined1 *)((int)&save_paintblob.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 1) =
         0;
    *(undefined1 *)((int)&save_paintblob.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 2) =
         0;
    *(undefined1 *)((int)&save_paintblob.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 3) =
         0;
    *(undefined4 *)((int)&save_paintblob.m_fnChangeCallbacks.m_Size + unaff_EBX) = 0;
    *(undefined4 *)((int)&save_paintblob.m_fnChangeCallbacks.m_pElements + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&g_PaintSaveRestoreBlockHandler.super_CDefSaveRestoreBlockHandler.
            super_ISaveRestoreBlockHandler._vptr_ISaveRestoreBlockHandler + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00d96b7c + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&g_debug_physcannon.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX)
         = 0;
    *(undefined4 *)((int)&g_debug_physcannon.super_ConCommandBase.m_pNext + unaff_EBX) = 0;
    *(undefined4 *)(&g_debug_physcannon.super_ConCommandBase.m_bRegistered + unaff_EBX) = 0;
    *(undefined4 *)((int)&g_debug_physcannon.super_ConCommandBase.m_pszName + unaff_EBX) = 0;
    *(undefined4 *)((int)&g_debug_physcannon.super_ConCommandBase.m_pszHelpString + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&g_debug_physcannon.super_ConCommandBase.m_nFlags + unaff_EBX) = 0x7f7fffff
    ;
    *(undefined4 *)((int)&g_debug_physcannon.super_IConVar._vptr_IConVar + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&g_debug_physcannon.m_pParent + unaff_EBX) = 0x7f7fffff;
    *(undefined **)((int)&g_debug_physcannon.m_pszDefaultValue + unaff_EBX) =
         &UNK_00b92034 + unaff_EBX;
    pdVar4 = DataMapInit<CTriggerPaintCleanser>((CTriggerPaintCleanser *)0x0);
    *(datamap_t **)((int)s_ElementNames + unaff_EBX + 0x1d50) = pdVar4;
    iVar6 = unaff_EBX + 0xd96ba4;
    s2 = vgui::Tooltip::ShowTooltip + unaff_EBX + 7;
    *(code **)((int)&g_debug_physcannon.m_Value.m_pszString + unaff_EBX) = s2;
    this = (SendTable *)((int)s_ElementNames + unaff_EBX + 0x1d54);
    *(SendTable **)((int)&g_debug_physcannon.m_Value.m_StringLength + unaff_EBX) = this;
    *(undefined4 *)(&g_debug_physcannon.m_bHasMin + unaff_EBX) = 0xffff;
    piVar1 = *(int **)(&DAT_00b8c920 + unaff_EBX);
    puVar2 = (undefined4 *)*piVar1;
    if (puVar2 == (undefined4 *)0x0) {
      *piVar1 = iVar6;
      *(undefined4 *)((int)&g_debug_physcannon.m_Value.m_fValue + unaff_EBX) = 0;
    }
    else {
      puVar3 = (undefined4 *)puVar2[2];
      iVar5 = _V_stricmp((char *)*puVar2,(char *)s2);
      if (iVar5 < 1) {
        while ((puVar3 != (undefined4 *)0x0 &&
               (iVar5 = _V_stricmp((char *)*puVar3,(char *)s2), iVar5 < 1))) {
          puVar2 = puVar3;
          puVar3 = (undefined4 *)puVar3[2];
        }
        *(undefined4 **)((int)&g_debug_physcannon.m_Value.m_fValue + unaff_EBX) = puVar3;
        puVar2[2] = iVar6;
      }
      else {
        *(int *)((int)&g_debug_physcannon.m_Value.m_fValue + unaff_EBX) = *piVar1;
        *piVar1 = iVar6;
      }
    }
    SendTable::SendTable(this);
    ___cxa_atexit(unaff_EBX + 0x986d3c,0,*(undefined4 *)(&DAT_00b8c880 + unaff_EBX));
    iVar6 = ServerClassInit<DT_TriggerPaintCleanser::ignored>((ignored *)0x0);
    *(int *)((int)s_ElementNames + unaff_EBX + 0x1d68) = iVar6;
    *(undefined **)((int)&g_debug_physcannon.m_fMinVal + unaff_EBX) = &UNK_00c20144 + unaff_EBX;
    pIVar7 = EntityFactoryDictionary();
    (**pIVar7->_vptr_IEntityFactoryDictionary)
              (pIVar7,unaff_EBX + 0xd96bb8,&UNK_009fb0cc + unaff_EBX);
  }
  return;
}


/* CTriggerPaintCleanser::GetDataDescMap at 00774d80 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CTriggerPaintCleanser * this) */

datamap_t * __thiscall CTriggerPaintCleanser::GetDataDescMap(CTriggerPaintCleanser *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5f6918);
}


/* CTriggerPaintCleanser::GetBaseMap at 00774d90 */

datamap_t * CTriggerPaintCleanser::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4d25d0);
}


/* __tcf_0 at 00a41030 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x40ffd6)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x40ffca) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x40ffd6));
  }
  *(undefined4 *)(unaff_EBX + 0x40ffd6) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40ffd2)) {
    if (*(int *)(unaff_EBX + 0x40ffca) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x205b1e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x205b1e),*(int *)(unaff_EBX + 0x40ffca));
      *(undefined4 *)(unaff_EBX + 0x40ffca) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40ffce) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40ffca);
  *(int *)(unaff_EBX + 0x40ffda) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40ffd2)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x205b1e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x205b1e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40ffca) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40ffce) = 0;
  }
  return;
}


/* ServerClassInit<DT_TriggerPaintCleanser::ignored> at 000ba030 */

int ServerClassInit<DT_TriggerPaintCleanser::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&player_held_object_look_down_adjustment.m_Value.m_fValue + unaff_EBX) == '\0')
  {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xd96e4c);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  (&player_held_object_look_down_adjustment.m_bHasMax + unaff_EBX),
                  (char *)(unaff_EBX + 0x99bbf0),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&player_held_object_distance.m_Value.m_nValue + unaff_EBX),
                        (char *)(unaff_EBX + 0x99dfa0),0,
                        (SendTable *)**(undefined4 **)(&DAT_00b8d638 + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00b8cbac + unaff_EBX),0x80);
      SendPropBool((SendProp_conflict *)
                   ((int)&player_held_object_distance_vm.m_Value.m_pszString + unaff_EBX),
                   (char *)(unaff_EBX + 0x99dab8),0x434,1);
      ___cxa_guard_release(unaff_EBX + 0xd96e4c);
      ___cxa_atexit(&UNK_00986f7c + unaff_EBX,0,*(undefined4 *)(&DAT_00b8cb10 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)((int)s_ElementNames + unaff_EBX + 0x1fe4),
             (SendProp_conflict *)((int)&player_held_object_distance.m_Value.m_nValue + unaff_EBX),2
             ,*(char **)(&DAT_00c203c0 + unaff_EBX));
  return 1;
}


/* CTriggerPaintCleanser::GetServerClass at 00774da0 */

/* DWARF original prototype: ServerClass * GetServerClass(CTriggerPaintCleanser * this) */

ServerClass * __thiscall CTriggerPaintCleanser::GetServerClass(CTriggerPaintCleanser *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x6dc0d0);
}


/* CTriggerPaintCleanser::YouForgotToImplementOrDeclareServerClass at 00774db0 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CTriggerPaintCleanser *
   this) */

int __thiscall
CTriggerPaintCleanser::YouForgotToImplementOrDeclareServerClass(CTriggerPaintCleanser *this)

{
  return 0;
}


/* __tcf_2 at 00a41010 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x389710),in_stack_00000008);
  return;
}


/* __tcf_1 at 00a40fc0 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40ff77))(unaff_EBX + 0x40ff77);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40ff23))(unaff_EBX + 0x40ff23);
                    /* WARNING: Could not recover jumptable at 0x00a41007. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(&DAT_0040fecf + unaff_EBX))();
  return;
}


/* CTriggerPaintCleanser::CTriggerPaintCleanser at 00774e90 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CTriggerPaintCleanser(CTriggerPaintCleanser * this) */

void __thiscall CTriggerPaintCleanser::CTriggerPaintCleanser(CTriggerPaintCleanser *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseTrigger::CBaseTrigger(&this->super_CBaseTrigger);
  (this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x565207);
  return;
}


/* CTriggerPaintCleanser::CTriggerPaintCleanser at 00774ed0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CTriggerPaintCleanser(CTriggerPaintCleanser * this) */

void __thiscall CTriggerPaintCleanser::CTriggerPaintCleanser(CTriggerPaintCleanser *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseTrigger::CBaseTrigger(&this->super_CBaseTrigger);
  (this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x5651c7);
  return;
}


/* CEntityFactory<CTriggerPaintCleanser>::Create at 00775390 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CTriggerPaintCleanser> *
   this, char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CTriggerPaintCleanser>::Create
          (CEntityFactory<CTriggerPaintCleanser> *this,char *pClassName)

{
  CBaseTrigger *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CTriggerPaintCleanser * pEnt@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CBaseEntity::operator_new(0x4e8);
  CBaseTrigger::CBaseTrigger(this_00);
  (this_00->super_CBaseToggle).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x564d04);
  CBaseEntity::PostConstructor((CBaseEntity *)this_00,pClassName);
  return &(this_00->super_CBaseToggle).super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CTriggerPaintCleanser::Spawn at 00774e70 */

/* DWARF original prototype: void Spawn(CTriggerPaintCleanser * this) */

void __thiscall CTriggerPaintCleanser::Spawn(CTriggerPaintCleanser *this)

{
  CBaseTrigger::Spawn(&this->super_CBaseTrigger);
  CBaseTrigger::InitTrigger(&this->super_CBaseTrigger);
  return;
}


/* CTriggerPaintCleanser::Activate at 00774e30 */

/* DWARF original prototype: void Activate(CTriggerPaintCleanser * this) */

void __thiscall CTriggerPaintCleanser::Activate(CTriggerPaintCleanser *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseTrigger::Activate(&this->super_CBaseTrigger);
  CPaintCleanserManager::AddPaintCleanser(*(CPaintCleanserManager **)(unaff_EBX + 0x4d1f5f),this);
  return;
}


/* CTriggerPaintCleanser::UpdateOnRemove at 00774df0 */

/* DWARF original prototype: void UpdateOnRemove(CTriggerPaintCleanser * this) */

void __thiscall CTriggerPaintCleanser::UpdateOnRemove(CTriggerPaintCleanser *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CPaintCleanserManager::RemovePaintCleanser
            (*(CPaintCleanserManager **)(&DAT_004d1f9f + unaff_EBX),this);
  CBaseTrigger::UpdateOnRemove(&this->super_CBaseTrigger);
  return;
}


/* CTriggerPaintCleanser::Touch at 00774f10 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void Touch(CTriggerPaintCleanser * this, CBaseEntity * pOther) */

void __thiscall CTriggerPaintCleanser::Touch(CTriggerPaintCleanser *this,CBaseEntity *pOther)

{
  edict_t *peVar1;
  bool bVar2;
  int iVar3;
  int *piVar4;
  undefined4 uVar5;
  int unaff_EBX;
  CTakeDamageInfo *in_stack_ffffff6c;
  CBaseEntity *in_stack_ffffff70;
  CBaseEntity *in_stack_ffffff74;
  float in_stack_ffffff78;
  code *in_stack_ffffff7c;
  int in_stack_ffffff80;
  CWeaponPaintGun *this_00;
  CTakeDamageInfo local_70;
  char *local_20;
  
                    /* Unresolved local var: CWeaponPaintGun * pPaintGun@[???] */
  ___i686_get_pc_thunk_bx();
  iVar3 = (*(pOther->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
            _vptr_IHandleEntity[0x55])(pOther);
  if ((char)iVar3 == '\0') {
    this_00 = (CWeaponPaintGun *)
              ___dynamic_cast(pOther,*(undefined4 *)(unaff_EBX + 0x4d1c69),
                              *(undefined4 *)(unaff_EBX + 0x4d2775),0);
  }
  else {
                    /* Unresolved local var: CPortal_Player * pPlayer@[???] */
    iVar3 = (*(pOther->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[0x55])(pOther);
    if ((char)iVar3 == '\0') goto LAB_007750b0;
                    /* Unresolved local var: IGameEvent * event@[???] */
    iVar3 = (*(pOther->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[0x119])(pOther,unaff_EBX + 0x2e7961,0);
    this_00 = (CWeaponPaintGun *)0x0;
    if (iVar3 != 0) {
      this_00 = (CWeaponPaintGun *)
                ___dynamic_cast(iVar3,*(undefined4 *)(unaff_EBX + 0x4d1de9),
                                *(undefined4 *)(unaff_EBX + 0x4d2775),0);
    }
    (*(pOther->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
      [0x1fa])(pOther);
    piVar4 = (int *)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d1c61) + 0x1c))
                              ((int *)**(undefined4 **)(unaff_EBX + 0x4d1c61),unaff_EBX + 0x34121a,0
                               ,0);
    if (piVar4 != (int *)0x0) {
      in_stack_ffffff7c = *(code **)(*piVar4 + 0x30);
      uVar5 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d1c59) + 0x40))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x4d1c59),(pOther->m_Network).m_pPev);
      (*in_stack_ffffff7c)(piVar4,unaff_EBX + 0x2e449d,uVar5);
      peVar1 = (this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.m_Network.m_pPev;
                    /* Unresolved local var: int edictIndex@[???] */
      iVar3 = 0;
      if (peVar1 != (edict_t *)0x0) {
        iVar3 = (int)peVar1 - *(int *)(**(int **)(unaff_EBX + 0x4d1c3d) + 0x58) >> 4;
      }
      (**(code **)(*piVar4 + 0x30))(piVar4,&UNK_0030a949 + unaff_EBX,iVar3);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d1c61) + 0x20))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4d1c61),piVar4,0);
    }
  }
  in_stack_ffffff80 = 0;
  if (this_00 != (CWeaponPaintGun *)0x0) {
    CWeaponPaintGun::CleansePaint(this_00);
    return;
  }
LAB_007750b0:
  local_20 = (char *)(unaff_EBX + 0x341236);
  if (((pOther->m_iClassname).pszValue == local_20) ||
     (bVar2 = CBaseEntity::ClassMatchesComplex(pOther,local_20), bVar2)) {
    UTIL_Remove(pOther);
    return;
  }
  local_20 = &UNK_0033c39d + unaff_EBX;
  if (((pOther->m_iClassname).pszValue != local_20) &&
     (bVar2 = CBaseEntity::ClassMatchesComplex(pOther,local_20), !bVar2)) {
    local_20 = (char *)(unaff_EBX + 0x2eadcf);
    if (((pOther->m_iClassname).pszValue != local_20) &&
       (bVar2 = CBaseEntity::ClassMatchesComplex(pOther,local_20), !bVar2)) {
      return;
    }
    CTakeDamageInfo::CTakeDamageInfo
              (&local_70,(CBaseEntity *)this,(CBaseEntity *)this,(float)(pOther->m_iHealth).m_Value,
               0,0,in_stack_ffffff6c,in_stack_ffffff70,in_stack_ffffff74,in_stack_ffffff78,
               (int)in_stack_ffffff7c,in_stack_ffffff80);
    if ((pOther->m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition(pOther);
    }
    local_70.m_vecDamagePosition.x = (pOther->m_vecAbsOrigin).x;
    local_70.m_vecDamagePosition.y = (pOther->m_vecAbsOrigin).y;
    local_70.m_vecDamagePosition.z = (pOther->m_vecAbsOrigin).z;
    CBaseEntity::TakeDamage(pOther,&local_70);
    return;
  }
  CPaintDatabase::RemovePaintedEntity(*(CPaintDatabase **)(unaff_EBX + 0x4d1fbd),pOther);
  return;
}


/* CTriggerPaintCleanser::Enable at 00774de0 */

/* DWARF original prototype: void Enable(CTriggerPaintCleanser * this) */

void __thiscall CTriggerPaintCleanser::Enable(CTriggerPaintCleanser *this)

{
  CBaseTrigger::Enable(&this->super_CBaseTrigger);
  return;
}


/* CTriggerPaintCleanser::UpdateTransmitState at 00774dc0 */

/* DWARF original prototype: int UpdateTransmitState(CTriggerPaintCleanser * this) */

int __thiscall CTriggerPaintCleanser::UpdateTransmitState(CTriggerPaintCleanser *this)

{
  int iVar1;
  
  iVar1 = CBaseEntity::SetTransmitState((CBaseEntity *)this,8);
  return iVar1;
}


/* _GLOBAL__I__ZN21CTriggerPaintCleanser9m_DataMapE at 000ba4b0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN21CTriggerPaintCleanser9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

