/* DWARF-guided pseudocode for game/server/portal2/trigger_paint_cleanser.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* DataMapInit<CTriggerPaintCleanser> at 000cf2c0 */

datamap_t * DataMapInit<CTriggerPaintCleanser>(CTriggerPaintCleanser *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((&CCommentarySystem::InitCommentary::commentarySpawnedSemaphoreName.field_0xf)[unaff_EBX] ==
      '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcecb2f);
    if (iVar1 != 0) {
      *(int *)((int)&CUtlLinkedList<CFoundryEntitySpawnRecord*,int,false,int,CUtlMemory<UtlLinkedListElem_t<CFoundryEntitySpawnRecord*,_int>,_int>_>
                     ::AllocInternal::__executeCount + unaff_EBX + 3) = unaff_EBX + 0x944966;
      *(undefined4 *)(&DAT_00cecb97 + unaff_EBX) = 0;
      *(undefined4 *)(&DAT_00cecb9b + unaff_EBX) = 0;
      *(undefined4 *)(&DAT_00cecb9f + unaff_EBX) = 0;
      *(undefined4 *)(&DAT_00cecba3 + unaff_EBX) = 0;
      *(undefined4 *)(&DAT_00cecba7 + unaff_EBX) = 0;
      *(undefined4 *)(&DAT_00cecb93 + unaff_EBX) = 0x15;
      ___cxa_guard_release(unaff_EBX + 0xcecb2f);
      ___cxa_atexit(&UNK_008ce5cf + unaff_EBX,0,*(undefined4 *)(&DAT_00adc8bf + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_NetworkStateChanged_m_flFriction_00c01f98 + unaff_EBX + 3) =
       *(undefined4 *)(&DAT_00add0c3 + unaff_EBX);
  *(undefined4 *)((int)&PTR_NetworkStateChanged_m_vecBaseVelocity_00c01f90 + unaff_EBX + 3) = 1;
  *(int *)((int)&PTR_NetworkStateChanged_m_vecBaseVelocity_00c01f8c + unaff_EBX + 3) =
       unaff_EBX + 0xcecb4f;
  return (datamap_t *)((int)&PTR_NetworkStateChanged_m_vecBaseVelocity_00c01f8c + unaff_EBX + 3);
}


/* __static_initialization_and_destruction_0 at 000cf390 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *s2;
  datamap_t *pdVar1;
  int iVar2;
  int iVar3;
  IEntityFactoryDictionary *pIVar4;
  int unaff_EBX;
  undefined4 *puVar5;
  undefined4 *puVar6;
  undefined4 *puVar7;
  longlong lVar8;
  
  lVar8 = ___i686_get_pc_thunk_bx();
  if (lVar8 == 0xffff00000001) {
    *(undefined1 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x34) = 0;
    *(undefined1 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x35) = 0;
    *(undefined1 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x36) = 0;
    *(undefined1 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x37) = 0;
    *(undefined4 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x38) = 0;
    *(undefined4 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x3c) = 0;
    *(undefined4 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x40) = 0x7f7fffff;
    *(undefined4 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x44) = 0x7f7fffff;
    *(undefined4 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x48) = 0;
    *(undefined4 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x4c) = 0;
    *(undefined4 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x50) = 0;
    *(undefined4 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x54) = 0;
    *(undefined4 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x58) = 0x7f7fffff;
    *(undefined4 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x5c) = 0x7f7fffff;
    *(undefined4 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x60) = 0x7f7fffff;
    *(undefined4 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 100) = 0x7f7fffff;
    *(undefined4 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x68) = 0;
    *(undefined1 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x6f) = 0;
    *(undefined1 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x6c) = 0;
    *(undefined1 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x6d) = 0;
    *(undefined1 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x6e) = 0;
    *(undefined4 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x70) = 0;
    *(undefined1 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x77) = 0;
    *(undefined1 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x74) = 0;
    *(undefined1 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x75) = 0;
    *(undefined1 *)((int)predef_flexcontroller_values[3] + unaff_EBX + 0x76) = 0;
    *(undefined4 *)((int)predef_flexcontroller_values[4] + unaff_EBX) = 0;
    *(undefined1 *)((int)predef_flexcontroller_values[4] + unaff_EBX + 7) = 1;
    *(undefined1 *)((int)predef_flexcontroller_values[4] + unaff_EBX + 4) = 0;
    *(undefined1 *)((int)predef_flexcontroller_values[4] + unaff_EBX + 5) = 0;
    *(undefined1 *)((int)predef_flexcontroller_values[4] + unaff_EBX + 6) = 0;
    *(undefined4 *)((int)predef_flexcontroller_values[4] + unaff_EBX + 8) = 1;
    *(undefined1 *)((int)predef_flexcontroller_values[4] + unaff_EBX + 0xf) = 0;
    *(undefined1 *)((int)predef_flexcontroller_values[4] + unaff_EBX + 0xc) = 0;
    *(undefined1 *)((int)predef_flexcontroller_values[4] + unaff_EBX + 0xd) = 0;
    *(undefined1 *)((int)predef_flexcontroller_values[4] + unaff_EBX + 0xe) = 0;
    *(undefined4 *)((int)predef_flexcontroller_values[4] + unaff_EBX + 0x10) = 2;
    *(undefined1 *)((int)predef_flexcontroller_values[4] + unaff_EBX + 0x17) = 0;
    *(undefined1 *)((int)predef_flexcontroller_values[4] + unaff_EBX + 0x14) = 0;
    *(undefined1 *)((int)predef_flexcontroller_values[4] + unaff_EBX + 0x15) = 0;
    *(undefined1 *)((int)predef_flexcontroller_values[4] + unaff_EBX + 0x16) = 0;
    *(undefined **)((int)predef_flexcontroller_values[4] + unaff_EBX + 0x18) =
         &UNK_00ae1f84 + unaff_EBX;
    pdVar1 = DataMapInit<CTriggerPaintCleanser>((CTriggerPaintCleanser *)0x0);
    *(datamap_t **)(&vgui::ComboBoxButton::vtable + unaff_EBX) = pdVar1;
    iVar3 = unaff_EBX + 0xcec93c;
    s2 = (char *)(unaff_EBX + 0x944893);
    *(char **)((int)predef_flexcontroller_values[4] + unaff_EBX + 0x1c) = s2;
    *(SendTable **)((int)predef_flexcontroller_values[4] + unaff_EBX + 0x20) =
         (SendTable *)((int)&PTR_typeinfo_00c61384 + unaff_EBX);
    *(undefined4 *)((int)predef_flexcontroller_values[4] + unaff_EBX + 0x2c) = 0xffff;
    puVar7 = (undefined4 *)**(int **)(&DAT_00adc88c + unaff_EBX);
    if (puVar7 == (undefined4 *)0x0) {
      **(int **)(&DAT_00adc88c + unaff_EBX) = iVar3;
      *(undefined4 *)((int)predef_flexcontroller_values[4] + unaff_EBX + 0x24) = 0;
    }
    else {
      puVar6 = (undefined4 *)puVar7[2];
      iVar2 = _V_stricmp((char *)*puVar7,s2);
      if (iVar2 < 1) {
        if (puVar6 != (undefined4 *)0x0) {
          do {
            puVar5 = puVar6;
            iVar2 = _V_stricmp((char *)*puVar5,s2);
            puVar6 = puVar5;
            if (0 < iVar2) goto LAB_000cf600;
            puVar6 = (undefined4 *)puVar5[2];
            puVar7 = puVar5;
          } while ((undefined4 *)puVar5[2] != (undefined4 *)0x0);
          puVar6 = (undefined4 *)0x0;
        }
LAB_000cf600:
        *(undefined4 **)((int)predef_flexcontroller_values[4] + unaff_EBX + 0x24) = puVar6;
        puVar7[2] = iVar3;
      }
      else {
        *(undefined4 *)((int)predef_flexcontroller_values[4] + unaff_EBX + 0x24) =
             **(undefined4 **)(&DAT_00adc88c + unaff_EBX);
        **(int **)(&DAT_00adc88c + unaff_EBX) = iVar3;
      }
    }
    SendTable::SendTable((SendTable *)((int)&PTR_typeinfo_00c61384 + unaff_EBX));
    ___cxa_atexit(unaff_EBX + 0x8ce4dc,0,*(undefined4 *)(&DAT_00adc7ec + unaff_EBX));
    iVar3 = ServerClassInit<DT_TriggerPaintCleanser::ignored>((ignored *)0x0);
    *(int *)((int)&PTR_Repaint_00c61398 + unaff_EBX) = iVar3;
    *(undefined **)((int)predef_flexcontroller_values[4] + unaff_EBX + 0x30) =
         &UNK_00b715d4 + unaff_EBX;
    pIVar4 = EntityFactoryDictionary();
    (**pIVar4->_vptr_IEntityFactoryDictionary)(pIVar4,unaff_EBX + 0xcec950,unaff_EBX + 0x943cec);
  }
  return;
}


/* CTriggerPaintCleanser::GetDataDescMap at 006f9e70 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CTriggerPaintCleanser * this) */

datamap_t * __thiscall CTriggerPaintCleanser::GetDataDescMap(CTriggerPaintCleanser *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5d73e8);
}


/* CTriggerPaintCleanser::GetBaseMap at 006f9e80 */

datamap_t * CTriggerPaintCleanser::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4b250c);
}


/* __tcf_0 at 0099d8a0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41e5c6)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41e5ba) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41e5c6));
  }
  *(undefined4 *)(unaff_EBX + 0x41e5c6) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41e5c2)) {
    if (*(int *)(unaff_EBX + 0x41e5ba) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20e2ea) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20e2ea),*(int *)(unaff_EBX + 0x41e5ba));
      *(undefined4 *)(unaff_EBX + 0x41e5ba) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e5be) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41e5ba);
  *(int *)(unaff_EBX + 0x41e5ca) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41e5c2)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20e2ea) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20e2ea),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41e5ba) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e5be) = 0;
  }
  return;
}


/* ServerClassInit<DT_TriggerPaintCleanser::ignored> at 000cf100 */

int ServerClassInit<DT_TriggerPaintCleanser::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&random_valve + unaff_EBX) == '\0') {
    iVar1 = ___cxa_guard_acquire((int)&random_valve + unaff_EBX);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)((int)&partition + unaff_EBX),(char *)(unaff_EBX + 0x8e3910),
                  0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&g_pStringTableClientSideChoreoScenes + unaff_EBX),
                        (char *)(unaff_EBX + 0x8e5c80),0,
                        (SendTable *)**(undefined4 **)(&DAT_00add5bc + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00adcb18 + unaff_EBX),0x80);
      SendPropBool((SendProp_conflict *)(unaff_EBX + 0xcecc94),(char *)(unaff_EBX + 0x91d8e0),0x434,
                   1);
      ___cxa_guard_release((int)&random_valve + unaff_EBX);
      ___cxa_atexit(unaff_EBX + 0x8ce71c,0,*(undefined4 *)(&DAT_00adca7c + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)((int)&PTR_OnDroppablePanelPaint_00c61614 + unaff_EBX),
             (SendProp_conflict *)((int)&g_pStringTableClientSideChoreoScenes + unaff_EBX),2,
             *(char **)(&DAT_00b71850 + unaff_EBX));
  return 1;
}


/* CTriggerPaintCleanser::GetServerClass at 006f9e90 */

/* DWARF original prototype: ServerClass * GetServerClass(CTriggerPaintCleanser * this) */

ServerClass * __thiscall CTriggerPaintCleanser::GetServerClass(CTriggerPaintCleanser *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x6c1e48);
}


/* CTriggerPaintCleanser::YouForgotToImplementOrDeclareServerClass at 006f9ea0 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CTriggerPaintCleanser *
   this) */

int __thiscall
CTriggerPaintCleanser::YouForgotToImplementOrDeclareServerClass(CTriggerPaintCleanser *this)

{
  return 0;
}


/* __tcf_2 at 0099d880 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x392ea0),in_stack_00000008);
  return;
}


/* __tcf_1 at 0099d830 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41e567))(unaff_EBX + 0x41e567);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41e513))(unaff_EBX + 0x41e513);
                    /* WARNING: Could not recover jumptable at 0x0099d877. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41e4bf))();
  return;
}


/* CTriggerPaintCleanser::CTriggerPaintCleanser at 006f9f80 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CTriggerPaintCleanser(CTriggerPaintCleanser * this) */

void __thiscall CTriggerPaintCleanser::CTriggerPaintCleanser(CTriggerPaintCleanser *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseTrigger::CBaseTrigger(&this->super_CBaseTrigger);
  (this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x546677);
  return;
}


/* CTriggerPaintCleanser::CTriggerPaintCleanser at 006f9fc0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CTriggerPaintCleanser(CTriggerPaintCleanser * this) */

void __thiscall CTriggerPaintCleanser::CTriggerPaintCleanser(CTriggerPaintCleanser *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseTrigger::CBaseTrigger(&this->super_CBaseTrigger);
  (this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x546637);
  return;
}


/* CEntityFactory<CTriggerPaintCleanser>::Create at 006fa3c0 */

/* WARNING: Type propagation algorithm not settling */
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
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x546234);
  CBaseEntity::PostConstructor((CBaseEntity *)this_00,pClassName);
  return &(this_00->super_CBaseToggle).super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CTriggerPaintCleanser::Spawn at 006f9f60 */

/* DWARF original prototype: void Spawn(CTriggerPaintCleanser * this) */

void __thiscall CTriggerPaintCleanser::Spawn(CTriggerPaintCleanser *this)

{
  CBaseTrigger::Spawn(&this->super_CBaseTrigger);
  CBaseTrigger::InitTrigger(&this->super_CBaseTrigger);
  return;
}


/* CTriggerPaintCleanser::Activate at 006f9f20 */

/* DWARF original prototype: void Activate(CTriggerPaintCleanser * this) */

void __thiscall CTriggerPaintCleanser::Activate(CTriggerPaintCleanser *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseTrigger::Activate(&this->super_CBaseTrigger);
  CPaintCleanserManager::AddPaintCleanser(*(CPaintCleanserManager **)(unaff_EBX + 0x4b1ea7),this);
  return;
}


/* CTriggerPaintCleanser::UpdateOnRemove at 006f9ee0 */

/* DWARF original prototype: void UpdateOnRemove(CTriggerPaintCleanser * this) */

void __thiscall CTriggerPaintCleanser::UpdateOnRemove(CTriggerPaintCleanser *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CPaintCleanserManager::RemovePaintCleanser(*(CPaintCleanserManager **)(unaff_EBX + 0x4b1ee7),this)
  ;
  CBaseTrigger::UpdateOnRemove(&this->super_CBaseTrigger);
  return;
}


/* CTriggerPaintCleanser::Touch at 006fa000 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
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
  code *in_stack_ffffff78;
  int in_stack_ffffff7c;
  CWeaponPaintGun *this_00;
  int *in_stack_ffffff80;
  CTakeDamageInfo local_70;
  char *local_20;
  
                    /* Unresolved local var: CWeaponPaintGun * pPaintGun@[???] */
  ___i686_get_pc_thunk_bx();
  iVar3 = (*(pOther->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
            _vptr_IHandleEntity[0x55])(pOther);
  if ((char)iVar3 == '\0') {
    this_00 = (CWeaponPaintGun *)
              ___dynamic_cast(pOther,*(undefined4 *)(unaff_EBX + 0x4b1bb1),
                              *(undefined4 *)(unaff_EBX + 0x4b26d5),0);
  }
  else {
    iVar3 = (*(pOther->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[0x55])(pOther);
    if (((char)iVar3 == '\0') ||
       (piVar4 = (int *)___dynamic_cast(pOther,*(undefined4 *)(unaff_EBX + 0x4b1bb1),
                                        *(undefined4 *)(unaff_EBX + 0x4b1db1),0),
       piVar4 == (int *)0x0)) goto LAB_006fa040;
                    /* Unresolved local var: IGameEvent * event@[???] */
    iVar3 = (**(code **)(*piVar4 + 0x464))(piVar4,unaff_EBX + 0x2bf629,0);
    this_00 = (CWeaponPaintGun *)0x0;
    if (iVar3 != 0) {
      this_00 = (CWeaponPaintGun *)
                ___dynamic_cast(iVar3,*(undefined4 *)(unaff_EBX + 0x4b1d39),
                                *(undefined4 *)(unaff_EBX + 0x4b26d5),0);
    }
    (**(code **)(*piVar4 + 0x7e8))(piVar4);
    in_stack_ffffff80 =
         (int *)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4b1ba9) + 0x1c))
                          ((int *)**(undefined4 **)(unaff_EBX + 0x4b1ba9),unaff_EBX + 0x319c36,0,0);
    if (in_stack_ffffff80 != (int *)0x0) {
      in_stack_ffffff78 = *(code **)(*in_stack_ffffff80 + 0x30);
      uVar5 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4b1ba1) + 0x40))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x4b1ba1),piVar4[9]);
      (*in_stack_ffffff78)(in_stack_ffffff80,unaff_EBX + 0x2bc175,uVar5);
      peVar1 = (this->super_CBaseTrigger).super_CBaseToggle.super_CBaseEntity.m_Network.m_pPev;
                    /* Unresolved local var: int edictIndex@[???] */
      iVar3 = 0;
      if (peVar1 != (edict_t *)0x0) {
        iVar3 = (int)peVar1 - *(int *)(**(int **)(unaff_EBX + 0x4b1b89) + 0x58) >> 4;
      }
      (**(code **)(*in_stack_ffffff80 + 0x30))(in_stack_ffffff80,unaff_EBX + 0x2e23f1,iVar3);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4b1ba9) + 0x20))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4b1ba9),in_stack_ffffff80,0);
    }
  }
  in_stack_ffffff7c = 0;
  if (this_00 != (CWeaponPaintGun *)0x0) {
    CWeaponPaintGun::CleansePaint(this_00);
    return;
  }
LAB_006fa040:
  local_20 = (char *)(unaff_EBX + 0x319c52);
  if (((pOther->m_iClassname).pszValue == local_20) ||
     (bVar2 = CBaseEntity::ClassMatchesComplex(pOther,local_20), bVar2)) {
    UTIL_Remove(pOther);
    return;
  }
  local_20 = (char *)(unaff_EBX + 0x2c2579);
  if (((pOther->m_iClassname).pszValue != local_20) &&
     (bVar2 = CBaseEntity::ClassMatchesComplex(pOther,local_20), !bVar2)) {
    local_20 = (char *)(unaff_EBX + 0x2c2ab7);
    if (((pOther->m_iClassname).pszValue != local_20) &&
       (bVar2 = CBaseEntity::ClassMatchesComplex(pOther,local_20), !bVar2)) {
      return;
    }
    CTakeDamageInfo::CTakeDamageInfo
              (&local_70,(CBaseEntity *)this,(CBaseEntity *)this,(float)(pOther->m_iHealth).m_Value,
               0,0,in_stack_ffffff6c,in_stack_ffffff70,in_stack_ffffff74,(float)in_stack_ffffff78,
               in_stack_ffffff7c,(int)in_stack_ffffff80);
    if ((pOther->m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition(pOther);
    }
    local_70.m_vecDamagePosition.x = (pOther->m_vecAbsOrigin).x;
    local_70.m_vecDamagePosition.y = (pOther->m_vecAbsOrigin).y;
    local_70.m_vecDamagePosition.z = (pOther->m_vecAbsOrigin).z;
    CBaseEntity::TakeDamage(pOther,&local_70);
    return;
  }
  CPaintDatabase::RemovePaintedEntity(*(CPaintDatabase **)(unaff_EBX + 0x4b1ef9),pOther);
  return;
}


/* CTriggerPaintCleanser::Enable at 006f9ed0 */

/* DWARF original prototype: void Enable(CTriggerPaintCleanser * this) */

void __thiscall CTriggerPaintCleanser::Enable(CTriggerPaintCleanser *this)

{
  CBaseTrigger::Enable(&this->super_CBaseTrigger);
  return;
}


/* CTriggerPaintCleanser::UpdateTransmitState at 006f9eb0 */

/* DWARF original prototype: int UpdateTransmitState(CTriggerPaintCleanser * this) */

int __thiscall CTriggerPaintCleanser::UpdateTransmitState(CTriggerPaintCleanser *this)

{
  int iVar1;
  
  iVar1 = CBaseEntity::SetTransmitState((CBaseEntity *)this,8);
  return iVar1;
}


/* _GLOBAL__I__ZN21CTriggerPaintCleanser9m_DataMapE at 000cf620 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN21CTriggerPaintCleanser9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

