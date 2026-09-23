/* DWARF-guided pseudocode for game/server/portal2/item_paint_power_pickup.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* PaintPowerPickup at 0071ceb0 */

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
           (_func_int_varargs **)(unaff_EBX + 0x52c324);
      CRecipientFilter::AddAllPlayers(&local_3c);
      CRecipientFilter::MakeReliable(&local_3c);
      messagename = (char *)(unaff_EBX + 0x342d6e);
      UserMessageBegin(&local_3c.super_IRecipientFilter,messagename);
      MessageWriteEHandle((CBaseEntity *)pCVar1);
      MessageWriteByte(colorIndex);
      MessageEnd();
      local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
           (_func_int_varargs **)(unaff_EBX + 0x52c324);
      CRecipientFilter::~CRecipientFilter(&local_3c,(int)messagename);
    }
  }
  return;
}


/* CPaintPowerPickup::~CPaintPowerPickup at 0071d660 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPaintPowerPickup(CPaintPowerPickup * this, int __in_chrg) */

void __thiscall CPaintPowerPickup::~CPaintPowerPickup(CPaintPowerPickup *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CItem).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x5acf97);
  (this->super_CItem).super_CDefaultPlayerPickupVPhysics.super_IPlayerPickupVPhysics.
  _vptr_IPlayerPickupVPhysics = (_func_int_varargs **)(unaff_EBX + 0x5ad357);
  CItem::~CItem(&this->super_CItem,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CPaintPowerPickup::~CPaintPowerPickup at 0071d6b0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPaintPowerPickup(CPaintPowerPickup * this, int __in_chrg) */

void __thiscall CPaintPowerPickup::~CPaintPowerPickup(CPaintPowerPickup *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CItem).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(extraout_ECX + 0x5acf50);
  (this->super_CItem).super_CDefaultPlayerPickupVPhysics.super_IPlayerPickupVPhysics.
  _vptr_IPlayerPickupVPhysics = (_func_int_varargs **)(extraout_ECX + 0x5ad310);
  CItem::~CItem(&this->super_CItem,__in_chrg);
  return;
}


/* DataMapInit<CPaintPowerPickup> at 000ab630 */

datamap_t * DataMapInit<CPaintPowerPickup>(CPaintPowerPickup *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x340].m_pPrev + unaff_EBX + 3)
      == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xda0fef);
    if (iVar1 != 0) {
      *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x341].m_pEntity + unaff_EBX + 3)
           = unaff_EBX + 0xa05ab6;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x341].m_pPrev + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x341].m_pNext + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x342].m_pEntity + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x342].m_SerialNumber + unaff_EBX + 3) =
           0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x342].m_pPrev + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x341].m_SerialNumber + unaff_EBX + 3) =
           0x11;
      ___cxa_guard_release(unaff_EBX + 0xda0fef);
      ___cxa_atexit(unaff_EBX + 0x991b1f,0,*(undefined4 *)(&DAT_00b9b513 + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_OnRestore_00cb8e48 + unaff_EBX + 3) =
       *(undefined4 *)(&DAT_00b9c05f + unaff_EBX);
  *(undefined4 *)((int)&PTR_ShouldSavePhysics_00cb8e40 + unaff_EBX + 3) = 1;
  *(int *)((int)&PTR_Restore_00cb8e3c + unaff_EBX + 3) = unaff_EBX + 0xcb8e9f;
  return (datamap_t *)((int)&PTR_Restore_00cb8e3c + unaff_EBX + 3);
}


/* __static_initialization_and_destruction_0 at 000ab700 */

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
  ServerClassInit<DT_LocalActiveWeaponData::ignored>
  [(int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX] = (code)0x0;
  *(undefined1 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x330].m_pEntity + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x330].m_pEntity + unaff_EBX + 2) = 0;
  *(undefined1 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x330].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x330].m_SerialNumber + unaff_EBX) = 0;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x330].m_pPrev + unaff_EBX) = 0
  ;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x330].m_pNext + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x331].m_pEntity + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x331].m_SerialNumber + unaff_EBX) = 0;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x331].m_pPrev + unaff_EBX) = 0
  ;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x331].m_pNext + unaff_EBX) = 0
  ;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x332].m_pEntity + unaff_EBX) =
       0;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x332].m_SerialNumber + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x332].m_pPrev + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x332].m_pNext + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x333].m_pEntity + unaff_EBX) =
       0x7f7fffff;
  *(undefined **)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x333].m_SerialNumber + unaff_EBX) =
       &UNK_00ba0bfc + unaff_EBX;
  pdVar1 = DataMapInit<CPaintPowerPickup>((CPaintPowerPickup *)0x0);
  *(datamap_t **)((int)DataMapInit<CAI_NetworkManager>::dataDesc[0].flatOffset + unaff_EBX + -0x28)
       = pdVar1;
  *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x333].m_pPrev + unaff_EBX) =
       &UNK_00c1f30c + unaff_EBX;
  pIVar2 = EntityFactoryDictionary();
  (**pIVar2->_vptr_IEntityFactoryDictionary)(pIVar2,unaff_EBX + 0xda0f1c,&UNK_00a059fd + unaff_EBX);
  return;
}


/* CPaintPowerPickup::GetDataDescMap at 0071cdd0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPaintPowerPickup * this) */

datamap_t * __thiscall CPaintPowerPickup::GetDataDescMap(CPaintPowerPickup *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x6476a8);
}


/* CPaintPowerPickup::GetBaseMap at 0071cde0 */

datamap_t * CPaintPowerPickup::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(&DAT_0052a8b8 + extraout_ECX);
}


/* __tcf_0 at 00a3d160 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x40f4de)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x40f4d2) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x40f4de));
  }
  *(undefined4 *)(unaff_EBX + 0x40f4de) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40f4da)) {
    if (*(int *)(unaff_EBX + 0x40f4d2) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2099ee) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2099ee),*(int *)(unaff_EBX + 0x40f4d2));
      *(undefined4 *)(unaff_EBX + 0x40f4d2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f4d6) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40f4d2);
  *(int *)(unaff_EBX + 0x40f4e2) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40f4da)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2099ee) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2099ee),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40f4d2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f4d6) = 0;
  }
  return;
}


/* CPaintPowerPickup::CPaintPowerPickup at 0071ce10 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CPaintPowerPickup(CPaintPowerPickup * this) */

void __thiscall CPaintPowerPickup::CPaintPowerPickup(CPaintPowerPickup *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CItem::CItem(&this->super_CItem);
  (this->super_CItem).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x5ad7e7);
  (this->super_CItem).super_CDefaultPlayerPickupVPhysics.super_IPlayerPickupVPhysics.
  _vptr_IPlayerPickupVPhysics = (_func_int_varargs **)(unaff_EBX + 0x5adba7);
  this->m_PaintPowerType = 4;
  return;
}


/* CPaintPowerPickup::CPaintPowerPickup at 0071ce60 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CPaintPowerPickup(CPaintPowerPickup * this) */

void __thiscall CPaintPowerPickup::CPaintPowerPickup(CPaintPowerPickup *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CItem::CItem(&this->super_CItem);
  (this->super_CItem).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(&UNK_005ad797 + unaff_EBX);
  (this->super_CItem).super_CDefaultPlayerPickupVPhysics.super_IPlayerPickupVPhysics.
  _vptr_IPlayerPickupVPhysics = (_func_int_varargs **)(unaff_EBX + 0x5adb57);
  this->m_PaintPowerType = 4;
  return;
}


/* CEntityFactory<CPaintPowerPickup>::Create at 0071d6f0 */

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
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5acf04);
  (this_00->super_CDefaultPlayerPickupVPhysics).super_IPlayerPickupVPhysics.
  _vptr_IPlayerPickupVPhysics = (_func_int_varargs **)(unaff_EBX + 0x5ad2c4);
  this_00[1].super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)0x4;
  (**(code **)(unaff_EBX + 0x5acf78))(this_00,pClassName);
  return &(this_00->super_CBaseAnimating).super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CPaintPowerPickup::Spawn at 0071d0b0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CPaintPowerPickup * this) */

void __thiscall CPaintPowerPickup::Spawn(CPaintPowerPickup *this)

{
  undefined1 *puVar1;
  ushort uVar2;
  CBaseEdict *pCVar3;
  ushort *puVar4;
  int *piVar5;
  uint8 uVar6;
  uint8 uVar7;
  Color *pCVar8;
  IChangeInfoAccessor *pIVar9;
  uint uVar10;
  uint uVar11;
  int unaff_EBX;
  int iVar12;
  uint local_a8;
  uint local_a4;
  uint local_a0;
  
                    /* Unresolved local var: Color renderColor@[???] */
  ___i686_get_pc_thunk_bx();
  (*(this->super_CItem).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1a])(this);
  iVar12 = unaff_EBX + 0x39401b;
  (*(this->super_CItem).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1b])(this);
  pCVar8 = MapPowerToVisualColor((Color *)this->m_PaintPowerType,iVar12);
  uVar6 = (uint8)((uint)pCVar8 >> 8);
  uVar7 = (uint8)((uint)pCVar8 >> 0x10);
  if ((uint8)pCVar8 !=
      (this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_clrRender.
      super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r) {
    if ((this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent.
        m_bRegistered == false) {
      pCVar3 = &((this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->
                super_CBaseEdict;
      if ((pCVar3 != (CBaseEdict *)0x0) && ((pCVar3->m_fStateFlags & 0x100U) == 0)) {
        pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 1;
        pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x529aa7);
        if (pIVar9->m_iChangeInfoSerialNumber == *puVar4) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar11 = (uint)pIVar9->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar4[uVar11 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_0071d5e0:
            puVar4[(uint)uVar2 + uVar11 * 0x14 + 1] = 0x108;
            puVar4[uVar11 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar4[uVar11 * 0x14 + 1] != 0x108) {
            local_a8 = 0;
            do {
              uVar10 = local_a8 + 1;
              local_a8 = uVar10 & 0xffff;
              if ((ushort)uVar10 == uVar2) {
                if (uVar2 != 0x13) goto LAB_0071d5e0;
                pIVar9->m_iChangeInfoSerialNumber = 0;
                pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar11 * 0x14 + local_a8 + 1] != 0x108);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar9->m_iChangeInfoSerialNumber = 0;
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar9->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x529aa7) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x529aa7) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x529aa7);
          pIVar9->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar12 = *piVar5 + (uint)pIVar9->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar12 + 2) = 0x108;
          *(undefined2 *)(iVar12 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_clrRender.
    super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r = (uint8)pCVar8;
  }
  if (uVar6 != (this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_clrRender.
               super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g) {
    if ((this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent.
        m_bRegistered == false) {
      pCVar3 = &((this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->
                super_CBaseEdict;
      if ((pCVar3 != (CBaseEdict *)0x0) && ((pCVar3->m_fStateFlags & 0x100U) == 0)) {
        pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 1;
        pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x529aa7);
        if (pIVar9->m_iChangeInfoSerialNumber == *puVar4) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar11 = (uint)pIVar9->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar4[uVar11 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_0071d534:
            puVar4[(uint)uVar2 + uVar11 * 0x14 + 1] = 0x108;
            puVar4[uVar11 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar4[uVar11 * 0x14 + 1] != 0x108) {
            local_a4 = 0;
            do {
              uVar10 = local_a4 + 1;
              local_a4 = uVar10 & 0xffff;
              if ((ushort)uVar10 == uVar2) {
                if (uVar2 == 0x13) goto LAB_0071d3c0;
                goto LAB_0071d534;
              }
            } while (puVar4[uVar11 * 0x14 + local_a4 + 1] != 0x108);
          }
        }
        else if (puVar4[0x7d1] == 100) {
LAB_0071d3c0:
          pIVar9->m_iChangeInfoSerialNumber = 0;
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar9->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x529aa7) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x529aa7) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x529aa7);
          pIVar9->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar12 = *piVar5 + (uint)pIVar9->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar12 + 2) = 0x108;
          *(undefined2 *)(iVar12 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_clrRender.
    super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g = uVar6;
  }
  if (uVar7 != (this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_clrRender.
               super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b) {
    if ((this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent.
        m_bRegistered == false) {
      pCVar3 = &((this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->
                super_CBaseEdict;
      if ((pCVar3 != (CBaseEdict *)0x0) && ((pCVar3->m_fStateFlags & 0x100U) == 0)) {
        pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 1;
        pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x529aa7);
        if (pIVar9->m_iChangeInfoSerialNumber == *puVar4) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar11 = (uint)pIVar9->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar4[uVar11 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_0071d470:
            puVar4[(uint)uVar2 + uVar11 * 0x14 + 1] = 0x108;
            puVar4[uVar11 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar4[uVar11 * 0x14 + 1] != 0x108) {
            local_a0 = 0;
            do {
              uVar10 = local_a0 + 1;
              local_a0 = uVar10 & 0xffff;
              if ((ushort)uVar10 == uVar2) {
                if (uVar2 == 0x13) goto LAB_0071d498;
                goto LAB_0071d470;
              }
            } while (puVar4[uVar11 * 0x14 + local_a0 + 1] != 0x108);
          }
        }
        else if (puVar4[0x7d1] == 100) {
LAB_0071d498:
          pIVar9->m_iChangeInfoSerialNumber = 0;
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar9->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x529aa7) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x529aa7) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x529aa7);
          pIVar9->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar12 = *piVar5 + (uint)pIVar9->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar12 + 2) = 0x108;
          *(undefined2 *)(iVar12 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CItem).super_CBaseAnimating.super_CBaseEntity.m_clrRender.
    super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b = uVar7;
  }
  CItem::Spawn(&this->super_CItem);
  CBaseEntity::AddFlag((CBaseEntity *)this,-0x80000000);
                    /* WARNING: Could not recover jumptable at 0x0071d1d8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CItem).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xa1])();
  return;
}


/* CPaintPowerPickup::Precache at 0071cdf0 */

/* DWARF original prototype: void Precache(CPaintPowerPickup * this) */

void __thiscall CPaintPowerPickup::Precache(CPaintPowerPickup *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  CBaseEntity::PrecacheModel((char *)(extraout_ECX + 0x3942e4));
  return;
}


/* CPaintPowerPickup::Touch at 0071cf80 */

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
              ___dynamic_cast(pOther,*(undefined4 *)(unaff_EBX + 0x529bfc),
                              *(undefined4 *)(unaff_EBX + 0x529c84),0);
    this_00 = CBaseCombatCharacter::GetActiveWeapon((CBaseCombatCharacter *)pPlayer);
    if (this_00 != (CBaseCombatWeapon *)0x0) {
      if (((this_00->super_CBaseAnimating).super_CBaseEntity.m_iClassname.pszValue !=
           (char *)(unaff_EBX + 0x33f8f4)) &&
         (bVar1 = CBaseEntity::ClassMatchesComplex
                            ((CBaseEntity *)this_00,(char *)(unaff_EBX + 0x33f8f4)), !bVar1)) {
        return;
      }
                    /* Unresolved local var: CWeaponPaintGun * pPaintGun@[???] */
      this_01 = (CWeaponPaintGun *)
                ___dynamic_cast(this_00,*(undefined4 *)(unaff_EBX + 0x529d7c),
                                *(undefined4 *)(unaff_EBX + 0x52a708),0);
      if ((this_01 != (CWeaponPaintGun *)0x0) &&
         (bVar1 = CWeaponPaintGun::HasPaintPower(this_01,this->m_PaintPowerType), !bVar1)) {
        CWeaponPaintGun::ActivatePaint(this_01,this->m_PaintPowerType);
        CBaseEntity::EmitSound
                  ((CBaseEntity *)this_01,(char *)(unaff_EBX + 0x36583c),0.0,(float *)0x0);
        PaintPowerPickup(this->m_PaintPowerType,pPlayer);
      }
    }
  }
  return;
}


/* _GLOBAL__I__Z16PaintPowerPickupiP11CBasePlayer at 000ab7f0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__Z16PaintPowerPickupiP11CBasePlayer(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

