/* DWARF-guided pseudocode for game/server/portal2/item_paint_power_pickup.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* PaintPowerPickup at 006bc9f0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

void PaintPowerPickup(int colorIndex,CBasePlayer *pPlayer)

{
  CBaseCombatWeapon *pCVar1;
  int unaff_EBX;
  char *messagename;
  CBaseCombatWeapon *pWeapon;
  CRecipientFilter local_3c;
  
                    /* Unresolved local var: CBroadcastRecipientFilter filter@[???] */
  ___i686_get_pc_thunk_bx();
  if (pPlayer != (CBasePlayer *)0x0) {
    pCVar1 = CBaseCombatCharacter::GetActiveWeapon(&pPlayer->super_CBaseCombatCharacter);
    if (pCVar1 != (CBaseCombatWeapon *)0x0) {
      pCVar1 = CBaseCombatCharacter::GetActiveWeapon(&pPlayer->super_CBaseCombatCharacter);
      CRecipientFilter::CRecipientFilter(&local_3c);
      local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
           (_func_int_varargs **)(unaff_EBX + 0x4f1804);
      CRecipientFilter::AddAllPlayers(&local_3c);
      CRecipientFilter::MakeReliable(&local_3c);
      messagename = (char *)(unaff_EBX + 0x2ffff4);
      UserMessageBegin(&local_3c.super_IRecipientFilter,messagename);
      MessageWriteEHandle((CBaseEntity *)pCVar1);
      MessageWriteByte(colorIndex);
      MessageEnd();
      local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
           (_func_int_varargs **)(unaff_EBX + 0x4f1804);
      CRecipientFilter::~CRecipientFilter(&local_3c,(int)messagename);
    }
  }
  return;
}


/* CPaintPowerPickup::~CPaintPowerPickup at 006bcc30 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPaintPowerPickup(CPaintPowerPickup * this, int __in_chrg) */

void __thiscall CPaintPowerPickup::~CPaintPowerPickup(CPaintPowerPickup *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CItem).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x575067);
  (this->super_CItem).super_CDefaultPlayerPickupVPhysics.super_IPlayerPickupVPhysics.
  _vptr_IPlayerPickupVPhysics = (_func_int_varargs **)(unaff_EBX + 0x575427);
  CItem::~CItem(&this->super_CItem,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CPaintPowerPickup::~CPaintPowerPickup at 006bcc80 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPaintPowerPickup(CPaintPowerPickup * this, int __in_chrg) */

void __thiscall CPaintPowerPickup::~CPaintPowerPickup(CPaintPowerPickup *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CItem).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(extraout_ECX + 0x575020);
  (this->super_CItem).super_CDefaultPlayerPickupVPhysics.super_IPlayerPickupVPhysics.
  _vptr_IPlayerPickupVPhysics = (_func_int_varargs **)(extraout_ECX + 0x5753e0);
  CItem::~CItem(&this->super_CItem,__in_chrg);
  return;
}


/* DataMapInit<CPaintPowerPickup> at 000c0740 */

datamap_t * DataMapInit<CPaintPowerPickup>(CPaintPowerPickup *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&sv_backspeed.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 3) == '\0')
  {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf6ce7);
    if (iVar1 != 0) {
      *(int *)((int)&sv_backspeed.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 3) =
           unaff_EBX + 0x94eb1e;
      *(undefined4 *)((int)&sv_backspeed.m_fnChangeCallbacks.m_pElements + unaff_EBX + 3) = 0;
      *(undefined4 *)(&DAT_00cf6cfb + unaff_EBX) = 0;
      *(undefined4 *)(&DAT_00cf6cff + unaff_EBX) = 0;
      *(undefined4 *)((int)&sv_waterdist.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX + 3)
           = 0;
      *(undefined4 *)((int)&sv_waterdist.super_ConCommandBase.m_pNext + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&sv_backspeed.m_fnChangeCallbacks.m_Size + unaff_EBX + 3) = 0x11;
      ___cxa_guard_release(unaff_EBX + 0xcf6ce7);
      ___cxa_atexit(unaff_EBX + 0x8d979f,0,*(undefined4 *)(&DAT_00aeb43f + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_ModifyOrAppendDerivedCriteria_00c09dd8 + unaff_EBX + 3) =
       *(undefined4 *)(&DAT_00aebf9f + unaff_EBX);
  *(undefined4 *)((int)&PTR_SetHealth_00c09dd0 + unaff_EBX + 3) = 1;
  *(int *)((int)&PTR_GetMaxHealth_00c09dcc + unaff_EBX + 3) = unaff_EBX + 0xc09e2f;
  return (datamap_t *)((int)&PTR_GetMaxHealth_00c09dcc + unaff_EBX + 3);
}


/* __static_initialization_and_destruction_0 at 000c0810 */

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
  (&sv_maxvelocity.m_bHasMin)[unaff_EBX] = false;
  (&sv_maxvelocity.field_0x35)[unaff_EBX] = 0;
  (&sv_maxvelocity.field_0x36)[unaff_EBX] = 0;
  (&sv_maxvelocity.field_0x37)[unaff_EBX] = 0;
  *(undefined4 *)((int)&sv_maxvelocity.m_fMinVal + unaff_EBX) = 0;
  *(undefined4 *)(&sv_maxvelocity.m_bHasMax + unaff_EBX) = 0;
  *(undefined4 *)((int)&sv_maxvelocity.m_fMaxVal + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&sv_maxvelocity.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&sv_maxvelocity.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX)
       = 0;
  *(undefined4 *)((int)&sv_maxvelocity.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) = 0;
  *(undefined4 *)((int)&sv_maxvelocity.m_fnChangeCallbacks.m_Size + unaff_EBX) = 0;
  *(undefined4 *)((int)&sv_maxvelocity.m_fnChangeCallbacks.m_pElements + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00cf6bd8 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00cf6bdc + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&sv_stepsize.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&sv_stepsize.super_ConCommandBase.m_pNext + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&sv_stepsize.super_ConCommandBase.m_bRegistered + unaff_EBX) = 0;
  *(undefined1 *)((int)&sv_stepsize.super_ConCommandBase.m_pszName + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&sv_stepsize.super_ConCommandBase.m_pszName + unaff_EBX) = 0;
  *(undefined1 *)((int)&sv_stepsize.super_ConCommandBase.m_pszName + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&sv_stepsize.super_ConCommandBase.m_pszName + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&sv_stepsize.super_ConCommandBase.m_pszHelpString + unaff_EBX) = 0;
  *(undefined1 *)((int)&sv_stepsize.super_ConCommandBase.m_nFlags + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&sv_stepsize.super_ConCommandBase.m_nFlags + unaff_EBX) = 0;
  *(undefined1 *)((int)&sv_stepsize.super_ConCommandBase.m_nFlags + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&sv_stepsize.super_ConCommandBase.m_nFlags + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&sv_stepsize.super_IConVar._vptr_IConVar + unaff_EBX) = 0;
  *(undefined1 *)((int)&sv_stepsize.m_pParent + unaff_EBX + 3) = 1;
  *(undefined1 *)((int)&sv_stepsize.m_pParent + unaff_EBX) = 0;
  *(undefined1 *)((int)&sv_stepsize.m_pParent + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&sv_stepsize.m_pParent + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&sv_stepsize.m_pszDefaultValue + unaff_EBX) = 1;
  *(undefined1 *)((int)&sv_stepsize.m_Value.m_pszString + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&sv_stepsize.m_Value.m_pszString + unaff_EBX) = 0;
  *(undefined1 *)((int)&sv_stepsize.m_Value.m_pszString + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&sv_stepsize.m_Value.m_pszString + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&sv_stepsize.m_Value.m_StringLength + unaff_EBX) = 2;
  *(undefined1 *)((int)&sv_stepsize.m_Value.m_fValue + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&sv_stepsize.m_Value.m_fValue + unaff_EBX) = 0;
  *(undefined1 *)((int)&sv_stepsize.m_Value.m_fValue + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&sv_stepsize.m_Value.m_fValue + unaff_EBX + 2) = 0;
  *(undefined **)((int)&sv_stepsize.m_Value.m_nValue + unaff_EBX) = &UNK_00af0b0c + unaff_EBX;
  pdVar1 = DataMapInit<CPaintPowerPickup>((CPaintPowerPickup *)0x0);
  *(datamap_t **)((int)DataMapInit<CFailableAchievement>::dataDesc[2].flatOffset + unaff_EBX + 8) =
       pdVar1;
  *(undefined **)(&sv_stepsize.m_bHasMin + unaff_EBX) = &UNK_00b7189c + unaff_EBX;
  pIVar2 = EntityFactoryDictionary();
  (**pIVar2->_vptr_IEntityFactoryDictionary)(pIVar2,unaff_EBX + 0xcf6c14,unaff_EBX + 0x94ea65);
  return;
}


/* CPaintPowerPickup::GetDataDescMap at 006bc7a0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPaintPowerPickup * this) */

datamap_t * __thiscall CPaintPowerPickup::GetDataDescMap(CPaintPowerPickup *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x60dd78);
}


/* CPaintPowerPickup::GetBaseMap at 006bc7b0 */

datamap_t * CPaintPowerPickup::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4eff38);
}


/* __tcf_0 at 00999ef0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41d556)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41d54a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41d556));
  }
  *(undefined4 *)(unaff_EBX + 0x41d556) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41d552)) {
    if (*(int *)(unaff_EBX + 0x41d54a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x211c9a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x211c9a),*(int *)(unaff_EBX + 0x41d54a));
      *(undefined4 *)(unaff_EBX + 0x41d54a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d54e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41d54a);
  *(int *)(unaff_EBX + 0x41d55a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41d552)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x211c9a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x211c9a),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41d54a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d54e) = 0;
  }
  return;
}


/* CPaintPowerPickup::CPaintPowerPickup at 006bc7e0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CPaintPowerPickup(CPaintPowerPickup * this) */

void __thiscall CPaintPowerPickup::CPaintPowerPickup(CPaintPowerPickup *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CItem::CItem(&this->super_CItem);
  (this->super_CItem).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x5754b7);
  (this->super_CItem).super_CDefaultPlayerPickupVPhysics.super_IPlayerPickupVPhysics.
  _vptr_IPlayerPickupVPhysics = (_func_int_varargs **)(unaff_EBX + 0x575877);
  this->m_PaintPowerType = 4;
  return;
}


/* CPaintPowerPickup::CPaintPowerPickup at 006bc830 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CPaintPowerPickup(CPaintPowerPickup * this) */

void __thiscall CPaintPowerPickup::CPaintPowerPickup(CPaintPowerPickup *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CItem::CItem(&this->super_CItem);
  (this->super_CItem).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x575467);
  (this->super_CItem).super_CDefaultPlayerPickupVPhysics.super_IPlayerPickupVPhysics.
  _vptr_IPlayerPickupVPhysics = (_func_int_varargs **)(unaff_EBX + 0x575827);
  this->m_PaintPowerType = 4;
  return;
}


/* CEntityFactory<CPaintPowerPickup>::Create at 006bccc0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CPaintPowerPickup> * this,
   char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CPaintPowerPickup>::Create(CEntityFactory<CPaintPowerPickup> *this,char *pClassName)

{
  CItem *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CPaintPowerPickup * pEnt@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CBaseEntity::operator_new(0x56c);
  CItem::CItem(this_00);
  (this_00->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x574fd4);
  (this_00->super_CDefaultPlayerPickupVPhysics).super_IPlayerPickupVPhysics.
  _vptr_IPlayerPickupVPhysics = (_func_int_varargs **)(unaff_EBX + 0x575394);
  this_00[1].super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)0x4;
  (**(code **)(unaff_EBX + 0x575048))(this_00,pClassName);
  return &(this_00->super_CBaseAnimating).super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CPaintPowerPickup::Spawn at 006bc880 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CPaintPowerPickup * this) */

void __thiscall CPaintPowerPickup::Spawn(CPaintPowerPickup *this)

{
  undefined1 *puVar1;
  CBaseEdict *pCVar2;
  uint8 uVar3;
  uint8 uVar4;
  Color *pCVar5;
  IChangeInfoAccessor *pIVar6;
  int unaff_EBX;
  int paintPowerType;
  
                    /* Unresolved local var: Color renderColor@[???] */
  ___i686_get_pc_thunk_bx();
  (*(this->super_CItem).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1a])(this);
  paintPowerType = unaff_EBX + 0x3529c7;
  (*(this->super_CItem).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1b])(this);
  pCVar5 = MapPowerToVisualColor((Color *)this->m_PaintPowerType,paintPowerType);
  uVar3 = (uint8)((uint)pCVar5 >> 8);
  uVar4 = (uint8)((uint)pCVar5 >> 0x10);
  if ((uint8)pCVar5 !=
      (this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_clrRender.
      super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r) {
    if ((this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent.
        m_bRegistered == false) {
      pCVar2 = &((this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->
                super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar6 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar6->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_clrRender.
    super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r = (uint8)pCVar5;
  }
  if (uVar3 != (this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_clrRender.
               super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g) {
    if ((this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent.
        m_bRegistered == false) {
      pCVar2 = &((this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->
                super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar6 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar6->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_clrRender.
    super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g = uVar3;
  }
  if (uVar4 != (this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_clrRender.
               super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b) {
    if ((this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent.
        m_bRegistered == false) {
      pCVar2 = &((this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->
                super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar6 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar6->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_clrRender.
    super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b = uVar4;
  }
  CItem::Spawn(&this->super_CItem);
  CBaseEntity::AddFlag((CBaseEntity *)this,-0x80000000);
                    /* WARNING: Could not recover jumptable at 0x006bc9b4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CItem).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xa1])();
  return;
}


/* CPaintPowerPickup::Precache at 006bc7c0 */

/* DWARF original prototype: void Precache(CPaintPowerPickup * this) */

void __thiscall CPaintPowerPickup::Precache(CPaintPowerPickup *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  CBaseEntity::PrecacheModel((char *)(extraout_ECX + 0x352a8c));
  return;
}


/* CPaintPowerPickup::Touch at 006bcac0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void Touch(CPaintPowerPickup * this, CBaseEntity * pOther) */

void __thiscall CPaintPowerPickup::Touch(CPaintPowerPickup *this,CBaseEntity *pOther)

{
  bool bVar1;
  int iVar2;
  CBasePlayer *pPlayer;
  CBaseCombatWeapon *this_00;
  CWeaponPaintGun *this_01;
  int unaff_EBX;
  
                    /* Unresolved local var: CBasePlayer * pPlayer@[???]
                       Unresolved local var: CBaseCombatWeapon * pWeapon@[???] */
  ___i686_get_pc_thunk_bx();
  iVar2 = (*(pOther->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
            _vptr_IHandleEntity[0x55])(pOther);
  if ((char)iVar2 != '\0') {
    pPlayer = (CBasePlayer *)
              ___dynamic_cast(pOther,*(undefined4 *)(unaff_EBX + 0x4ef0f4),
                              *(undefined4 *)(unaff_EBX + 0x4ef180),0);
    this_00 = CBaseCombatCharacter::GetActiveWeapon((CBaseCombatCharacter *)pPlayer);
    if (this_00 != (CBaseCombatWeapon *)0x0) {
      if (((this_00->super_CBaseAnimating).super_CBaseEntity.m_iClassname.pszValue !=
           (char *)(unaff_EBX + 0x2fcb6c)) &&
         (bVar1 = CBaseEntity::ClassMatchesComplex
                            ((CBaseEntity *)this_00,(char *)(unaff_EBX + 0x2fcb6c)), !bVar1)) {
        return;
      }
                    /* Unresolved local var: CWeaponPaintGun * pPaintGun@[???] */
      this_01 = (CWeaponPaintGun *)
                ___dynamic_cast(this_00,*(undefined4 *)(&DAT_004ef27c + unaff_EBX),
                                *(undefined4 *)(unaff_EBX + 0x4efc18),0);
      if ((this_01 != (CWeaponPaintGun *)0x0) &&
         (bVar1 = CWeaponPaintGun::HasPaintPower(this_01,this->m_PaintPowerType), !bVar1)) {
        CWeaponPaintGun::ActivatePaint(this_01,this->m_PaintPowerType);
        CBaseEntity::EmitSound
                  ((CBaseEntity *)this_01,(char *)(unaff_EBX + 0x3228e8),0.0,(float *)0x0);
        PaintPowerPickup(this->m_PaintPowerType,pPlayer);
      }
    }
  }
  return;
}


/* _GLOBAL__I__Z16PaintPowerPickupiP11CBasePlayer at 000c09a0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__Z16PaintPowerPickupiP11CBasePlayer(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

