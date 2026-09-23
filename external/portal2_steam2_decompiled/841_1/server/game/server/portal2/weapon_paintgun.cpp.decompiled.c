/* DWARF-guided pseudocode for game/server/portal2/weapon_paintgun.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* CWeaponPaintGun::GetServerClass at 00778d10 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: ServerClass * GetServerClass(CWeaponPaintGun * this) */

ServerClass * __thiscall CWeaponPaintGun::GetServerClass(CWeaponPaintGun *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x6d8dc0);
}


/* CWeaponPaintGun::YouForgotToImplementOrDeclareServerClass at 00778d20 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CWeaponPaintGun * this) */

int __thiscall CWeaponPaintGun::YouForgotToImplementOrDeclareServerClass(CWeaponPaintGun *this)

{
  return 0;
}


/* ServerClassInit<DT_WeaponPaintGun::ignored> at 000bbae0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

int ServerClassInit<DT_WeaponPaintGun::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  char *pcVar2;
  SendProp local_c4;
  SendProp local_70;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&debug_paintable_projected_wall.super_ConCommandBase.m_pNext + unaff_EBX + 3)
      == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xd96067);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  ((int)&debug_paintable_projected_wall.super_ConCommandBase.m_pszName +
                  unaff_EBX + 3),(char *)(unaff_EBX + 0x99a143),0,4,-1,0,
                  (SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&sv_thinnerprojectedwalls.super_ConCommandBase._vptr_ConCommandBase +
                        unaff_EBX + 3),(char *)(unaff_EBX + 0x99c4f3),0,
                        (SendTable *)**(undefined4 **)(&DAT_00b8bb93 + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00b8b0ff + unaff_EBX),0x80);
      pcVar2 = &UNK_009faadd + unaff_EBX;
      SendPropBool((SendProp_conflict *)&local_70,pcVar2,0x644,1);
      SendPropArray3((SendProp_conflict *)
                     ((int)&sv_thinnerprojectedwalls.m_fnChangeCallbacks.m_pElements + unaff_EBX + 3
                     ),pcVar2,0x644,1,5,(SendProp_conflict *)&local_70,
                     *(SendTableProxyFn_conflict *)(&DAT_00b8b0ff + unaff_EBX),0x80);
      SendProp::~SendProp(&local_70,(int)pcVar2);
      pcVar2 = &UNK_009faae9 + unaff_EBX;
      SendPropEHandle((SendProp_conflict *)&local_c4,pcVar2,0x64c,4,-1,
                      *(SendVarProxyFn_conflict *)(&DAT_00b8b0eb + unaff_EBX));
      SendPropArray3((SendProp_conflict *)
                     ((int)&g_StaticCollisionPolyhedronCache.m_BrushPolyhedrons.m_Size +
                     unaff_EBX + 3),pcVar2,0x64c,4,5,(SendProp_conflict *)&local_c4,
                     *(SendTableProxyFn_conflict *)(&DAT_00b8b0ff + unaff_EBX),0x80);
      SendProp::~SendProp(&local_c4,(int)pcVar2);
      SendPropInt((SendProp_conflict *)
                  ((int)&sv_portal_staticcollisioncache_cachebrushes.m_pParent + unaff_EBX + 3),
                  &UNK_009faaf8 + unaff_EBX,0x664,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropBool((SendProp_conflict *)
                   ((int)&sv_portal_staticcollisioncache_cachestaticprops.super_ConCommandBase.
                          m_pszHelpString + unaff_EBX + 3),&UNK_009fab08 + unaff_EBX,0x668,1);
      SendPropBool((SendProp_conflict *)
                   ((int)&CTrigger_TractorBeam_Shared::AllTractorBeams.m_Memory.m_nAllocationCount +
                   unaff_EBX + 3),&UNK_009fab17 + unaff_EBX,0x669,1);
      ___cxa_guard_release(unaff_EBX + 0xd96067);
      ___cxa_atexit(&UNK_00985a6f + unaff_EBX,0,*(undefined4 *)(&DAT_00b8b063 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)((int)s_ElementNames + unaff_EBX + 0x58f),
             (SendProp_conflict *)
             ((int)&sv_thinnerprojectedwalls.super_ConCommandBase._vptr_ConCommandBase +
             unaff_EBX + 3),6,*(char **)(&DAT_00c1fe13 + unaff_EBX));
  return 1;
}


/* __static_initialization_and_destruction_0 at 000bc0c0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *s2;
  SendTable *this;
  int *piVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  undefined4 uVar4;
  int iVar5;
  int iVar6;
  datamap_t *pdVar7;
  IEntityFactoryDictionary *pIVar8;
  int unaff_EBX;
  longlong lVar9;
  
  lVar9 = ___i686_get_pc_thunk_bx();
  if (lVar9 == 0xffff00000001) {
    *(undefined1 *)((int)&sv_paint_trigger_sound_delay.m_Value.m_fValue + unaff_EBX) = 0;
    *(undefined1 *)((int)&sv_paint_trigger_sound_delay.m_Value.m_fValue + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&sv_paint_trigger_sound_delay.m_Value.m_fValue + unaff_EBX + 2) = 0;
    *(undefined1 *)((int)&sv_paint_trigger_sound_delay.m_Value.m_fValue + unaff_EBX + 3) = 0;
    *(undefined4 *)((int)&sv_paint_trigger_sound_delay.m_Value.m_nValue + unaff_EBX) = 0;
    *(undefined4 *)(&sv_paint_trigger_sound_delay.m_bHasMin + unaff_EBX) = 0;
    *(undefined4 *)((int)&sv_paint_trigger_sound_delay.m_fMinVal + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&sv_paint_trigger_sound_delay.m_bHasMax + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&sv_paint_trigger_sound_delay.m_fMaxVal + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&sv_paint_trigger_sound_delay.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&sv_paint_trigger_sound_delay.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX
     ) = 0;
    *(undefined4 *)
     ((int)&sv_paint_trigger_sound_delay.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) = 0;
    *(undefined4 *)((int)&sv_paint_trigger_sound_delay.m_fnChangeCallbacks.m_Size + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&sv_paint_trigger_sound_delay.m_fnChangeCallbacks.m_pElements + unaff_EBX)
         = 0x7f7fffff;
    *(undefined4 *)(&DAT_00d959f8 + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00d959fc + unaff_EBX) = 0x7f7fffff;
    *(undefined **)
     ((int)&player_paint_shoot_pos_forward_scale.super_ConCommandBase._vptr_ConCommandBase +
     unaff_EBX) = &UNK_00b90234 + unaff_EBX;
    iVar6 = unaff_EBX + 0xd95a04;
    s2 = (char *)(unaff_EBX + 0x9fa553);
    *(char **)((int)&player_paint_shoot_pos_forward_scale.super_ConCommandBase.m_pNext + unaff_EBX)
         = s2;
    this = (SendTable *)((int)&g_ItemFlags[1].m_iFlagValue + unaff_EBX);
    *(SendTable **)
     (&player_paint_shoot_pos_forward_scale.super_ConCommandBase.m_bRegistered + unaff_EBX) = this;
    *(undefined4 *)
     ((int)&player_paint_shoot_pos_forward_scale.super_ConCommandBase.m_nFlags + unaff_EBX) = 0xffff
    ;
    piVar1 = *(int **)(&DAT_00b8ab20 + unaff_EBX);
    puVar2 = (undefined4 *)*piVar1;
    if (puVar2 == (undefined4 *)0x0) {
      *piVar1 = iVar6;
      *(undefined4 *)
       ((int)&player_paint_shoot_pos_forward_scale.super_ConCommandBase.m_pszName + unaff_EBX) = 0;
    }
    else {
      puVar3 = (undefined4 *)puVar2[2];
      iVar5 = _V_stricmp((char *)*puVar2,s2);
      if (iVar5 < 1) {
        while ((puVar3 != (undefined4 *)0x0 && (iVar5 = _V_stricmp((char *)*puVar3,s2), iVar5 < 1)))
        {
          puVar2 = puVar3;
          puVar3 = (undefined4 *)puVar3[2];
        }
        *(undefined4 **)
         ((int)&player_paint_shoot_pos_forward_scale.super_ConCommandBase.m_pszName + unaff_EBX) =
             puVar3;
        puVar2[2] = iVar6;
      }
      else {
        *(int *)((int)&player_paint_shoot_pos_forward_scale.super_ConCommandBase.m_pszName +
                unaff_EBX) = *piVar1;
        *piVar1 = iVar6;
      }
    }
    SendTable::SendTable(this);
    uVar4 = *(undefined4 *)(&DAT_00b8aa80 + unaff_EBX);
    ___cxa_atexit(unaff_EBX + 0x98553c,0,uVar4);
    iVar6 = ServerClassInit<DT_WeaponPaintGun::ignored>((ignored *)0x0);
    *(int *)((int)&g_ItemFlags[4].m_pFlagName + unaff_EBX) = iVar6;
    pdVar7 = DataMapInit<CWeaponPaintGun>((CWeaponPaintGun *)0x0);
    *(datamap_t **)((int)&g_ItemFlags[4].m_iFlagValue + unaff_EBX) = pdVar7;
    *(undefined **)
     ((int)&player_paint_shoot_pos_forward_scale.super_IConVar._vptr_IConVar + unaff_EBX) =
         &UNK_00c1f844 + unaff_EBX;
    pIVar8 = EntityFactoryDictionary();
    (**pIVar8->_vptr_IEntityFactoryDictionary)(pIVar8,unaff_EBX + 0xd95a18,unaff_EBX + 0x9a07b4);
    ConCommand::ConCommand
              ((ConCommand *)
               ((int)&player_paint_shoot_pos_forward_scale.m_Value.m_fValue + unaff_EBX),
               (char *)(unaff_EBX + 0x9fa563),(FnCommandCallbackV1_t)(unaff_EBX + 0x6bda4c),
               (char *)0x0,0,(FnCommandCompletionCallback)0x0);
    ___cxa_atexit(unaff_EBX + 0x98551c,0,uVar4);
  }
  return;
}


/* __tcf_2 at 00a41610 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x389168),in_stack_00000008);
  return;
}


/* __tcf_0 at 00a41560 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x4107eb))(unaff_EBX + 0x4107eb);
  (*(code *)**(undefined4 **)(&DAT_00410797 + unaff_EBX))(&DAT_00410797 + unaff_EBX);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410743))(unaff_EBX + 0x410743);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x4106ef))(unaff_EBX + 0x4106ef);
  (*(code *)**(undefined4 **)(&DAT_0041069b + unaff_EBX))(&DAT_0041069b + unaff_EBX);
  (*(code *)**(undefined4 **)(&DAT_00410647 + unaff_EBX))(&DAT_00410647 + unaff_EBX);
                    /* WARNING: Could not recover jumptable at 0x00a415e4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x4105f3))();
  return;
}


/* DataMapInit<CWeaponPaintGun> at 000bbea0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

datamap_t * DataMapInit<CWeaponPaintGun>(CWeaponPaintGun *param_1)

{
  int iVar1;
  int iVar2;
  char *pcVar3;
  size_t sVar4;
  int iVar5;
  undefined4 *puVar6;
  int iVar7;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (((&sv_paint_detection_sphere_radius.m_bHasMin)[unaff_EBX] == false) &&
     (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd95c74), iVar2 != 0)) {
    *(undefined **)
     ((int)&sv_paint_detection_sphere_radius.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) =
         &UNK_009fa773 + unaff_EBX;
    *(undefined4 *)
     ((int)&sv_paint_detection_sphere_radius.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) =
         0;
    *(undefined4 *)((int)&sv_paint_detection_sphere_radius.m_fnChangeCallbacks.m_Size + unaff_EBX) =
         0;
    *(undefined4 *)
     ((int)&sv_paint_detection_sphere_radius.m_fnChangeCallbacks.m_pElements + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00d95c98 + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00d95c9c + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&sv_paint_detection_sphere_radius.m_fnChangeCallbacks.m_Memory.m_nAllocationCount +
     unaff_EBX) = 0xf;
    ___cxa_guard_release(unaff_EBX + 0xd95c74);
    ___cxa_atexit(unaff_EBX + 0x98577c,0,*(undefined4 *)(&DAT_00b8aca0 + unaff_EBX));
  }
  *(undefined4 *)(&DAT_00cb03dc + unaff_EBX) = *(undefined4 *)(&DAT_00b8b7cc + unaff_EBX);
  if (((&sv_paint_detection_sphere_radius.m_bHasMax)[unaff_EBX] == false) &&
     (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd95c7c), iVar2 != 0)) {
    pcVar3 = operator_new___(*(int *)((int)&sv_paint_detection_sphere_radius.m_fnChangeCallbacks.
                                            m_Memory.m_nAllocationCount + unaff_EBX) + 0xe);
    _strcpy(pcVar3,*(char **)((int)&sv_paint_detection_sphere_radius.m_fnChangeCallbacks.m_Memory.
                                    m_pMemory + unaff_EBX));
    sVar4 = _strlen(pcVar3);
    builtin_strncpy(pcVar3 + sVar4,"PaintGunThink",0xe);
    iVar1 = *(int *)(&DAT_00d95c98 + unaff_EBX);
    iVar2 = iVar1 + 1;
    iVar5 = *(int *)((int)&sv_paint_detection_sphere_radius.m_fnChangeCallbacks.m_Size + unaff_EBX);
    if (iVar5 < iVar2) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)
                 ((int)&sv_paint_detection_sphere_radius.m_fnChangeCallbacks.m_Memory.m_nGrowSize +
                 unaff_EBX),iVar2 - iVar5);
    }
    iVar5 = *(int *)(&DAT_00d95c98 + unaff_EBX);
    *(int *)(&DAT_00d95c98 + unaff_EBX) = iVar5 + 1;
    iVar7 = *(int *)((int)&sv_paint_detection_sphere_radius.m_fnChangeCallbacks.m_Memory.m_nGrowSize
                    + unaff_EBX);
    *(int *)(&DAT_00d95c9c + unaff_EBX) = iVar7;
    iVar5 = ((iVar5 + 1) - iVar1) + -1;
    if (0 < iVar5) {
      _V_memmove((void *)(iVar7 + iVar2 * 4),(void *)(iVar7 + iVar1 * 4),iVar5 * 4);
      iVar7 = *(int *)((int)&sv_paint_detection_sphere_radius.m_fnChangeCallbacks.m_Memory.
                             m_nGrowSize + unaff_EBX);
    }
    puVar6 = (undefined4 *)(iVar1 * 4 + iVar7);
    if (puVar6 != (undefined4 *)0x0) {
      *puVar6 = pcVar3;
    }
    *(char **)((int)&PTR_MoveDone_00cb0570 + unaff_EBX) = pcVar3;
    ___cxa_guard_release(unaff_EBX + 0xd95c7c);
  }
  *(undefined4 *)(&DAT_00cb03d4 + unaff_EBX) = 6;
  *(undefined **)((int)&PTR_typeinfo_name_00cb03d0 + unaff_EBX) = &UNK_00cb042c + unaff_EBX;
  return (datamap_t *)((int)&PTR_typeinfo_name_00cb03d0 + unaff_EBX);
}


/* CWeaponPaintGun::GetDataDescMap at 00778d30 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: datamap_t * GetDataDescMap(CWeaponPaintGun * this) */

datamap_t * __thiscall CWeaponPaintGun::GetDataDescMap(CWeaponPaintGun *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5f354c);
}


/* CWeaponPaintGun::GetBaseMap at 00778d40 */

datamap_t * CWeaponPaintGun::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(&DAT_004ce938 + extraout_ECX);
}


/* __tcf_1 at 00a41630 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(&DAT_0041050e + unaff_EBX)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x410502) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(&DAT_0041050e + unaff_EBX));
  }
  *(undefined4 *)(&DAT_0041050e + unaff_EBX) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41050a)) {
    if (*(int *)(unaff_EBX + 0x410502) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20551e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20551e),*(int *)(unaff_EBX + 0x410502));
      *(undefined4 *)(unaff_EBX + 0x410502) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x410506) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x410502);
  *(int *)(unaff_EBX + 0x410512) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41050a)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20551e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20551e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x410502) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x410506) = 0;
  }
  return;
}


/* CWeaponPaintGun::CWeaponPaintGun at 0077a1c0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void CWeaponPaintGun(CWeaponPaintGun * this) */

void __thiscall CWeaponPaintGun::CWeaponPaintGun(CWeaponPaintGun *this)

{
  uint *puVar1;
  uint uVar2;
  int iVar3;
  ushort uVar4;
  CBaseEdict *pCVar5;
  int *piVar6;
  ushort *puVar7;
  IChangeInfoAccessor *pIVar8;
  uint uVar9;
  int unaff_EBX;
  uint uVar10;
  
  ___i686_get_pc_thunk_bx();
  CBasePortalCombatWeapon::CBasePortalCombatWeapon(&this->super_CBasePortalCombatWeapon);
  (this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon.
  super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x56115a);
  (this->m_hPaintStream).m_Value[0].super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hPaintStream).m_Value[1].super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hPaintStream).m_Value[2].super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hPaintStream).m_Value[3].super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hPaintStream).m_Value[4].super_CBaseHandle.m_Index = 0xffffffff;
  this->m_flAccumulatedTime = 0.0;
  (this->m_nCurrentColor).m_Value = 4;
  if (*(bool *)((int)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                      super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                      m_TimerEvent + 0x10) == false) {
    pCVar5 = &((this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon
               .super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
      pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
      pIVar8 = CBaseEdict::GetChangeAccessor(pCVar5);
      piVar6 = *(int **)(unaff_EBX + 0x4cc99a);
      puVar7 = (ushort *)*piVar6;
      if (pIVar8->m_iChangeInfoSerialNumber == *puVar7) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        uVar10 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar4 = puVar7[uVar10 * 0x14 + 0x14];
        if (uVar4 == 0) {
LAB_0077a4e6:
          puVar7[(uint)uVar4 + uVar10 * 0x14 + 1] = 0x664;
          puVar7[uVar10 * 0x14 + 0x14] = uVar4 + 1;
        }
        else if (puVar7[uVar10 * 0x14 + 1] != 0x664) {
          uVar9 = 0;
          do {
            uVar2 = uVar9 + 1;
            uVar9 = uVar2 & 0xffff;
            if ((ushort)uVar2 == uVar4) {
              if (uVar4 != 0x13) goto LAB_0077a4e6;
              goto LAB_0077a61b;
            }
          } while (puVar7[uVar10 * 0x14 + uVar9 + 1] != 0x664);
        }
      }
      else if (puVar7[0x7d1] == 100) {
LAB_0077a61b:
        pIVar8->m_iChangeInfoSerialNumber = 0;
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar8->m_iChangeInfo = puVar7[0x7d1];
        *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
        pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
        iVar3 = *piVar6 + (uint)pIVar8->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar3 + 2) = 0x664;
        *(undefined2 *)(iVar3 + 0x28) = 1;
      }
    }
  }
  else {
    puVar1 = (uint *)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                      super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                      field_0x4c;
    *puVar1 = *puVar1 | 1;
  }
  (this->m_bFiringPaint).m_Value = false;
  if (*(bool *)((int)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                      super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                      m_TimerEvent + 0x10) == false) {
    pCVar5 = &((this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon
               .super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
      pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
      pIVar8 = CBaseEdict::GetChangeAccessor(pCVar5);
      piVar6 = *(int **)(unaff_EBX + 0x4cc99a);
      puVar7 = (ushort *)*piVar6;
      if (pIVar8->m_iChangeInfoSerialNumber == *puVar7) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        uVar10 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar4 = puVar7[uVar10 * 0x14 + 0x14];
        if (uVar4 == 0) {
LAB_0077a566:
          puVar7[(uint)uVar4 + uVar10 * 0x14 + 1] = 0x668;
          puVar7[uVar10 * 0x14 + 0x14] = uVar4 + 1;
        }
        else if (puVar7[uVar10 * 0x14 + 1] != 0x668) {
          uVar9 = 0;
          do {
            uVar2 = uVar9 + 1;
            uVar9 = uVar2 & 0xffff;
            if ((ushort)uVar2 == uVar4) {
              if (uVar4 != 0x13) goto LAB_0077a566;
              goto LAB_0077a632;
            }
          } while (puVar7[uVar10 * 0x14 + uVar9 + 1] != 0x668);
        }
      }
      else if (puVar7[0x7d1] == 100) {
LAB_0077a632:
        pIVar8->m_iChangeInfoSerialNumber = 0;
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar8->m_iChangeInfo = puVar7[0x7d1];
        *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
        pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
        iVar3 = *piVar6 + (uint)pIVar8->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar3 + 2) = 0x668;
        *(undefined2 *)(iVar3 + 0x28) = 1;
      }
    }
  }
  else {
    puVar1 = (uint *)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                      super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                      field_0x4c;
    *puVar1 = *puVar1 | 1;
  }
  (this->m_bFiringErase).m_Value = false;
  if (*(bool *)((int)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                      super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                      m_TimerEvent + 0x10) == false) {
    pCVar5 = &((this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon
               .super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
      pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
      pIVar8 = CBaseEdict::GetChangeAccessor(pCVar5);
      piVar6 = *(int **)(unaff_EBX + 0x4cc99a);
      puVar7 = (ushort *)*piVar6;
      if (pIVar8->m_iChangeInfoSerialNumber == *puVar7) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        uVar10 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar4 = puVar7[uVar10 * 0x14 + 0x14];
        if (uVar4 == 0) {
LAB_0077a5e2:
          puVar7[(uint)uVar4 + uVar10 * 0x14 + 1] = 0x669;
          puVar7[uVar10 * 0x14 + 0x14] = uVar4 + 1;
        }
        else if (puVar7[uVar10 * 0x14 + 1] != 0x669) {
          uVar9 = 0;
          do {
            uVar2 = uVar9 + 1;
            uVar9 = uVar2 & 0xffff;
            if ((ushort)uVar2 == uVar4) {
              if (uVar4 != 0x13) goto LAB_0077a5e2;
              goto LAB_0077a604;
            }
          } while (puVar7[uVar10 * 0x14 + uVar9 + 1] != 0x669);
        }
      }
      else if (puVar7[0x7d1] == 100) {
LAB_0077a604:
        pIVar8->m_iChangeInfoSerialNumber = 0;
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar8->m_iChangeInfo = puVar7[0x7d1];
        *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
        pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
        iVar3 = *piVar6 + (uint)pIVar8->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar3 + 2) = 0x669;
        *(undefined2 *)(iVar3 + 0x28) = 1;
      }
    }
  }
  else {
    puVar1 = (uint *)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                      super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                      field_0x4c;
    *puVar1 = *puVar1 | 1;
  }
  (this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon.
  m_bFireOnEmpty = true;
  (this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon.
  m_bReloadsSingly = false;
  ResetPaint(this);
  ActivatePaint(this,NO_POWER);
  SetSubType(this,(this->m_nCurrentColor).m_Value);
  return;
}


/* CWeaponPaintGun::CWeaponPaintGun at 0077a680 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void CWeaponPaintGun(CWeaponPaintGun * this, CWeaponPaintGun * this) */

void __thiscall CWeaponPaintGun::CWeaponPaintGun(CWeaponPaintGun *this,CWeaponPaintGun *this_1)

{
  CWeaponPaintGun(this);
  return;
}


/* CEntityFactory<CWeaponPaintGun>::Create at 0077a6d0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CWeaponPaintGun> * this,
   char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CWeaponPaintGun>::Create(CEntityFactory<CWeaponPaintGun> *this,char *pClassName)

{
  CWeaponPaintGun *this_00;
  
                    /* Unresolved local var: CWeaponPaintGun * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x678);
  CWeaponPaintGun::CWeaponPaintGun(this_00);
  (**(code **)((int)(this_00->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                    super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.
                    super_IServerEntity.super_IServerUnknown + 0x74))(this_00,pClassName);
  return &(this_00->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon.
          super_CBaseAnimating.super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CWeaponPaintGun::~CWeaponPaintGun at 00779700 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ~CWeaponPaintGun(CWeaponPaintGun * this, int __in_chrg) */

void __thiscall CWeaponPaintGun::~CWeaponPaintGun(CWeaponPaintGun *this,int __in_chrg)

{
  CSoundEnvelopeController *pCVar1;
  int unaff_EBX;
  CSoundPatch *in_stack_ffffffd8;
  CSoundPatch *pCVar2;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon.
  super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x561c14);
                    /* Unresolved local var: CSoundEnvelopeController * controller@[???] */
  pCVar1 = CSoundEnvelopeController::GetController();
  pCVar2 = this->m_pLiquidLoop;
  if (pCVar2 != (CSoundPatch *)0x0) {
    (*pCVar1->_vptr_CSoundEnvelopeController[0xb])(pCVar1);
    this->m_pLiquidLoop = (CSoundPatch *)0x0;
    in_stack_ffffffd8 = pCVar2;
  }
  pCVar2 = this->m_pLiquidStart;
  if (pCVar2 != (CSoundPatch *)0x0) {
    (*pCVar1->_vptr_CSoundEnvelopeController[0xb])(pCVar1);
    this->m_pLiquidStart = (CSoundPatch *)0x0;
    in_stack_ffffffd8 = pCVar2;
  }
  (this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon.
  super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(*(int *)(unaff_EBX + 0x4cd7bc) + 8);
  (this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon.
  super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(*(int *)(unaff_EBX + 0x4cd7a0) + 8);
  CBaseCombatWeapon::~CBaseCombatWeapon((CBaseCombatWeapon *)this,(int)in_stack_ffffffd8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CWeaponPaintGun::~CWeaponPaintGun at 007797c0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ~CWeaponPaintGun(CWeaponPaintGun * this, int __in_chrg) */

void __thiscall CWeaponPaintGun::~CWeaponPaintGun(CWeaponPaintGun *this,int __in_chrg)

{
  CSoundEnvelopeController *pCVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon.
  super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x561b54);
                    /* Unresolved local var: CSoundEnvelopeController * controller@[???] */
  pCVar1 = CSoundEnvelopeController::GetController();
  if (this->m_pLiquidLoop != (CSoundPatch *)0x0) {
    (*pCVar1->_vptr_CSoundEnvelopeController[0xb])(pCVar1,this->m_pLiquidLoop);
    this->m_pLiquidLoop = (CSoundPatch *)0x0;
  }
  if (this->m_pLiquidStart != (CSoundPatch *)0x0) {
    (*pCVar1->_vptr_CSoundEnvelopeController[0xb])(pCVar1,this->m_pLiquidStart);
    this->m_pLiquidStart = (CSoundPatch *)0x0;
  }
  (this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon.
  super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(*(int *)(unaff_EBX + 0x4cd6fc) + 8);
  (this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon.
  super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(*(int *)(unaff_EBX + 0x4cd6e0) + 8);
  CBaseCombatWeapon::~CBaseCombatWeapon((CBaseCombatWeapon *)this,__in_chrg);
  return;
}


/* CWeaponPaintGun::~CWeaponPaintGun at 00779880 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ~CWeaponPaintGun(CWeaponPaintGun * this, int __in_chrg,
   CWeaponPaintGun * this) */

void __thiscall
CWeaponPaintGun::~CWeaponPaintGun(CWeaponPaintGun *this,int __in_chrg,CWeaponPaintGun *this_1)

{
  ~CWeaponPaintGun(this,__in_chrg);
  return;
}


/* CWeaponPaintGun::Spawn at 007790c0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(CWeaponPaintGun * this) */

void __thiscall CWeaponPaintGun::Spawn(CWeaponPaintGun *this)

{
  undefined1 *puVar1;
  uint uVar2;
  EHANDLE *pEVar3;
  ushort uVar4;
  CBaseEdict *this_00;
  ushort *puVar5;
  int *piVar6;
  BASEPTR func;
  int iVar7;
  CBaseEntity *pEntity;
  ulong *puVar8;
  IChangeInfoAccessor *pIVar9;
  uint uVar10;
  int unaff_EBX;
  EHANDLE *pEVar11;
  EHANDLE *pEVar12;
  ushort uVar13;
  CBaseHandle *pCVar14;
  CBaseHandle *pCVar15;
  uint uVar16;
  undefined1 uVar17;
  int local_3c;
  CBaseHandle local_20 [4];
  
  ___i686_get_pc_thunk_bx();
  (**(code **)((int)(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                    super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.
                    super_IServerEntity.super_IServerUnknown + 0x68))(this);
  CWeaponPortalBase::Spawn((CWeaponPortalBase *)this);
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x21e);
  CBaseEntity::ThinkSet
            ((CBaseEntity *)this,func,*(float *)(**(int **)(unaff_EBX + 0x4cda96) + 0xc),
             &UNK_0033d549 + unaff_EBX);
  this->m_flAccumulatedTime =
       *(float *)(unaff_EBX + 0x353842) /
       *(float *)(*(int *)(*(int *)(unaff_EBX + 0x4ce622) + 0x1c) + 0x2c);
  this->m_nBlobRandomSeed = 0;
  local_3c = 0;
  do {
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
    uVar16 = (this->m_hPaintStream).m_Value[local_3c].super_CBaseHandle.m_Index;
    if (((uVar16 == 0xffffffff) ||
        (iVar7 = (uVar16 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4cda9e),
        *(uint *)(iVar7 + 8) != uVar16 >> 0x10)) || (*(int *)(iVar7 + 4) == 0)) {
      pEntity = CreateEntityByName(&UNK_0033d57c + unaff_EBX,-1,true);
      uVar17 = pEntity == (CBaseEntity *)0x0;
      if (!(bool)uVar17) {
        DispatchSpawn(pEntity,true);
        local_20[0].m_Index = 0xffffffff;
        puVar8 = (ulong *)(*(pEntity->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                            _vptr_IHandleEntity[3])(pEntity);
        local_20[0].m_Index = *puVar8;
        pEVar3 = (this->m_hPaintStream).m_Value + local_3c;
        iVar7 = 4;
        pEVar11 = pEVar3;
        pCVar14 = local_20;
        do {
          pEVar12 = pEVar11;
          pCVar15 = pCVar14;
          if (iVar7 == 0) break;
          iVar7 = iVar7 + -1;
          pCVar15 = (CBaseHandle *)((int)&pCVar14->m_Index + 1);
          pEVar12 = (EHANDLE *)((int)&(pEVar11->super_CBaseHandle).m_Index + 1);
          uVar17 = (char)(pEVar11->super_CBaseHandle).m_Index == (char)pCVar14->m_Index;
          pEVar11 = pEVar12;
          pCVar14 = pCVar15;
        } while ((bool)uVar17);
        iVar7 = 0;
        if (!(bool)uVar17) {
          iVar7 = (uint)*(byte *)((int)(pEVar12 + -1) + 3) -
                  (uint)*(byte *)((int)&pCVar15[-1].m_Index + 3);
        }
        if (iVar7 != 0) {
          if ((this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon.
              super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false)
          {
            this_00 = &((this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                        super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                       m_pPev)->super_CBaseEdict;
            if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
              uVar13 = (short)pEVar3 - (short)this;
              this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
              pIVar9 = CBaseEdict::GetChangeAccessor(this_00);
              puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4cda9a);
              if (pIVar9->m_iChangeInfoSerialNumber == *puVar5) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                uVar16 = (uint)pIVar9->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
                uVar4 = puVar5[uVar16 * 0x14 + 0x14];
                if (uVar4 == 0) {
LAB_0077934c:
                  puVar5[(uint)uVar4 + uVar16 * 0x14 + 1] = uVar13;
                  puVar5[uVar16 * 0x14 + 0x14] = uVar4 + 1;
                }
                else if (puVar5[uVar16 * 0x14 + 1] != uVar13) {
                  uVar10 = 0;
                  do {
                    uVar2 = uVar10 + 1;
                    uVar10 = uVar2 & 0xffff;
                    if ((ushort)uVar2 == uVar4) {
                      if (uVar4 == 0x13) goto LAB_0077936d;
                      goto LAB_0077934c;
                    }
                  } while (puVar5[uVar16 * 0x14 + uVar10 + 1] != uVar13);
                }
              }
              else if (puVar5[0x7d1] == 100) {
LAB_0077936d:
                pIVar9->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
              }
              else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                pIVar9->m_iChangeInfo = puVar5[0x7d1];
                piVar6 = *(int **)(unaff_EBX + 0x4cda9a);
                *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
                pIVar9->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
                iVar7 = *piVar6 + (uint)pIVar9->m_iChangeInfo * 0x28;
                *(ushort *)(iVar7 + 2) = uVar13;
                *(undefined2 *)(iVar7 + 0x28) = 1;
              }
            }
          }
          else {
            puVar1 = &(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                      super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                      field_0x4c;
            *(uint *)puVar1 = *(uint *)puVar1 | 1;
          }
          (this->m_hPaintStream).m_Value[local_3c].super_CBaseHandle.m_Index = local_20[0].m_Index;
        }
      }
    }
    local_3c = local_3c + 1;
    if (local_3c == 5) {
      return;
    }
  } while( true );
}


/* CWeaponPaintGun::UpdateOnRemove at 00778f70 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void UpdateOnRemove(CWeaponPaintGun * this) */

void __thiscall CWeaponPaintGun::UpdateOnRemove(CWeaponPaintGun *this)

{
  int *piVar1;
  uint uVar2;
  int iVar3;
  CBaseEntity *pCVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  piVar1 = *(int **)(unaff_EBX + 0x4cdbee);
  uVar2 = (this->m_hPaintStream).m_Value[0].super_CBaseHandle.m_Index;
  if ((uVar2 == 0xffffffff) ||
     (iVar3 = (uVar2 & 0xffff) * 0x10 + *piVar1, *(uint *)(iVar3 + 8) != uVar2 >> 0x10)) {
    pCVar4 = (CBaseEntity *)0x0;
  }
  else {
    pCVar4 = *(CBaseEntity **)(iVar3 + 4);
  }
  UTIL_Remove(pCVar4);
  uVar2 = (this->m_hPaintStream).m_Value[1].super_CBaseHandle.m_Index;
  if ((uVar2 == 0xffffffff) ||
     (iVar3 = (uVar2 & 0xffff) * 0x10 + *piVar1, *(uint *)(iVar3 + 8) != uVar2 >> 0x10)) {
    pCVar4 = (CBaseEntity *)0x0;
  }
  else {
    pCVar4 = *(CBaseEntity **)(iVar3 + 4);
  }
  UTIL_Remove(pCVar4);
  uVar2 = (this->m_hPaintStream).m_Value[2].super_CBaseHandle.m_Index;
  if ((uVar2 == 0xffffffff) ||
     (iVar3 = (uVar2 & 0xffff) * 0x10 + *piVar1, *(uint *)(iVar3 + 8) != uVar2 >> 0x10)) {
    pCVar4 = (CBaseEntity *)0x0;
  }
  else {
    pCVar4 = *(CBaseEntity **)(iVar3 + 4);
  }
  UTIL_Remove(pCVar4);
  uVar2 = (this->m_hPaintStream).m_Value[3].super_CBaseHandle.m_Index;
  if ((uVar2 == 0xffffffff) ||
     (iVar3 = (uVar2 & 0xffff) * 0x10 + *piVar1, *(uint *)(iVar3 + 8) != uVar2 >> 0x10)) {
    pCVar4 = (CBaseEntity *)0x0;
  }
  else {
    pCVar4 = *(CBaseEntity **)(iVar3 + 4);
  }
  UTIL_Remove(pCVar4);
  uVar2 = (this->m_hPaintStream).m_Value[4].super_CBaseHandle.m_Index;
  if ((uVar2 == 0xffffffff) ||
     (iVar3 = (uVar2 & 0xffff) * 0x10 + *piVar1, *(uint *)(iVar3 + 8) != uVar2 >> 0x10)) {
    pCVar4 = (CBaseEntity *)0x0;
  }
  else {
    pCVar4 = *(CBaseEntity **)(iVar3 + 4);
  }
  UTIL_Remove(pCVar4);
  CBaseEntity::UpdateOnRemove((CBaseEntity *)this);
  return;
}


/* CWeaponPaintGun::Precache at 00778df0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void Precache(CWeaponPaintGun * this) */

void __thiscall CWeaponPaintGun::Precache(CWeaponPaintGun *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x33d7a3));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x33d7b5));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x2e2cf8));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x3396c7));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x3396e7));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x339713));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x33973f));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x2e2e6d));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x2e2e83));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x33976b));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x2e2e99));
  CBaseCombatWeapon::Precache((CBaseCombatWeapon *)this);
  return;
}


/* CWeaponPaintGun::SendWeaponAnim at 00779080 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: bool SendWeaponAnim(CWeaponPaintGun * this, int iActivity) */

bool __thiscall CWeaponPaintGun::SendWeaponAnim(CWeaponPaintGun *this,int iActivity)

{
  bool bVar1;
  
                    /* Unresolved local var: int newActivity@[???] */
  if (iActivity == 0xbe) {
    iActivity = (-(uint)((this->m_bHasPaint).m_Value[(this->m_nCurrentColor).m_Value] == false) & 3)
                + 0xbe;
  }
  bVar1 = CBaseCombatWeapon::SendWeaponAnim((CBaseCombatWeapon *)this,iActivity);
  return bVar1;
}


/* CWeaponPaintGun::HasCurrentColor at 00778d50 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: bool HasCurrentColor(CWeaponPaintGun * this) */

bool __thiscall CWeaponPaintGun::HasCurrentColor(CWeaponPaintGun *this)

{
  return (this->m_bHasPaint).m_Value[(this->m_nCurrentColor).m_Value];
}


/* CWeaponPaintGun::StartShootingSound at 00779570 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void StartShootingSound(CWeaponPaintGun * this) */

void __thiscall CWeaponPaintGun::StartShootingSound(CWeaponPaintGun *this)

{
  edict_t *peVar1;
  CSoundEnvelopeController *pCVar2;
  CSoundPatch *pCVar3;
  int iVar4;
  int unaff_EBX;
  CRecipientFilter local_54;
  Vector local_34;
  Vector local_28 [2];
  
                    /* Unresolved local var: CSoundEnvelopeController * controller@[???] */
  ___i686_get_pc_thunk_bx();
  pCVar2 = CSoundEnvelopeController::GetController();
  if (this->m_pLiquidLoop != (CSoundPatch *)0x0) {
    (*pCVar2->_vptr_CSoundEnvelopeController[0xb])(pCVar2,this->m_pLiquidLoop);
    this->m_pLiquidLoop = (CSoundPatch *)0x0;
  }
  (**(code **)((int)(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                    super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.
                    super_IServerEntity.super_IServerUnknown + 0x278))(local_28,this);
  CRecipientFilter::CRecipientFilter(&local_54);
  local_54.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(unaff_EBX + 0x4cfbea);
  CRecipientFilter::AddRecipientsByPAS(&local_54,local_28);
  local_54.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(unaff_EBX + 0x4cfbaa);
  (**(code **)((int)(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                    super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.
                    super_IServerEntity.super_IServerUnknown + 0x278))(&local_34,this);
  CPASAttenuationFilter::Filter((CPASAttenuationFilter *)&local_54,&local_34,0.8);
  peVar1 = (this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon.
           super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev;
                    /* Unresolved local var: int edictIndex@[???] */
  iVar4 = 0;
  if (peVar1 != (edict_t *)0x0) {
    iVar4 = (int)peVar1 - *(int *)(**(int **)(unaff_EBX + 0x4cd5e6) + 0x58) >> 4;
  }
  pCVar3 = (CSoundPatch *)
           (*pCVar2->_vptr_CSoundEnvelopeController[6])(pCVar2,&local_54,iVar4,unaff_EBX + 0x33d022)
  ;
  this->m_pLiquidLoop = pCVar3;
  (*pCVar2->_vptr_CSoundEnvelopeController[2])(pCVar2,pCVar3,0,0x42c80000,0);
  pCVar3 = this->m_pLiquidLoop;
  (*pCVar2->_vptr_CSoundEnvelopeController[0xd])(pCVar2,pCVar3,0x3f800000,0x3f800000);
  local_54.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(unaff_EBX + 0x4cfbea);
  CRecipientFilter::~CRecipientFilter(&local_54,(int)pCVar3);
  return;
}


/* CWeaponPaintGun::StopShootingSound at 00778db0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void StopShootingSound(CWeaponPaintGun * this) */

void __thiscall CWeaponPaintGun::StopShootingSound(CWeaponPaintGun *this)

{
  CSoundEnvelopeController *pCVar1;
  
  if (this->m_pLiquidLoop != (CSoundPatch *)0x0) {
                    /* Unresolved local var: CSoundEnvelopeController * controller@[???] */
    pCVar1 = CSoundEnvelopeController::GetController();
    (*pCVar1->_vptr_CSoundEnvelopeController[0xe])(pCVar1,this->m_pLiquidLoop,0x3f000000,0);
  }
  return;
}


/* CWeaponPaintGun::ActivatePaint at 00779890 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void ActivatePaint(CWeaponPaintGun * this, PaintPowerType nIndex) */

void __thiscall CWeaponPaintGun::ActivatePaint(CWeaponPaintGun *this,PaintPowerType nIndex)

{
  uint *puVar1;
  uint uVar2;
  int iVar3;
  ushort uVar4;
  CBaseEdict *this_00;
  code *pcVar5;
  ushort *puVar6;
  char cVar7;
  int *piVar8;
  CBaseCombatCharacter *pCVar9;
  undefined4 uVar10;
  IChangeInfoAccessor *pIVar11;
  uint uVar12;
  int unaff_EBX;
  ushort uVar13;
  uint uVar14;
  
                    /* Unresolved local var: IGameEvent * event@[???] */
  ___i686_get_pc_thunk_bx();
  if ((this->m_bHasPaint).m_Value[nIndex] != true) {
    if (*(bool *)((int)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                        super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                        m_TimerEvent + 0x10) == false) {
      this_00 = &((this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                  super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->
                 super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        uVar13 = (short)nIndex + 0x644;
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar11 = CBaseEdict::GetChangeAccessor(this_00);
        piVar8 = *(int **)(&DAT_004cd2ca + unaff_EBX);
        puVar6 = (ushort *)*piVar8;
        if (pIVar11->m_iChangeInfoSerialNumber == *puVar6) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar14 = (uint)pIVar11->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar4 = puVar6[uVar14 * 0x14 + 0x14];
          if (uVar4 == 0) {
LAB_00779af6:
            puVar6[(uint)uVar4 + uVar14 * 0x14 + 1] = uVar13;
            puVar6[uVar14 * 0x14 + 0x14] = uVar4 + 1;
          }
          else if (puVar6[uVar14 * 0x14 + 1] != uVar13) {
            uVar12 = 0;
            do {
              uVar2 = uVar12 + 1;
              uVar12 = uVar2 & 0xffff;
              if ((ushort)uVar2 == uVar4) {
                if (uVar4 == 0x13) goto LAB_00779a85;
                goto LAB_00779af6;
              }
            } while (puVar6[uVar14 * 0x14 + uVar12 + 1] != uVar13);
          }
        }
        else if (puVar6[0x7d1] == 100) {
LAB_00779a85:
          pIVar11->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar11->m_iChangeInfo = puVar6[0x7d1];
          *(short *)(*piVar8 + 0xfa2) = *(short *)(*piVar8 + 0xfa2) + 1;
          pIVar11->m_iChangeInfoSerialNumber = *(ushort *)*piVar8;
          iVar3 = *piVar8 + (uint)pIVar11->m_iChangeInfo * 0x28;
          *(ushort *)(iVar3 + 2) = uVar13;
          *(undefined2 *)(iVar3 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = (uint *)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                        super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                        field_0x4c;
      *puVar1 = *puVar1 | 1;
    }
    (this->m_bHasPaint).m_Value[nIndex] = true;
  }
  piVar8 = (int *)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4cd2ea) + 0x1c))
                            ((int *)**(undefined4 **)(unaff_EBX + 0x4cd2ea),unaff_EBX + 0x33cdb9,0,0
                            );
  if (((piVar8 != (int *)0x0) &&
      (pCVar9 = CBaseCombatWeapon::GetOwner((CBaseCombatWeapon *)this),
      pCVar9 != (CBaseCombatCharacter *)0x0)) &&
     (cVar7 = (**(code **)((int)(pCVar9->super_CBaseFlex).super_CBaseAnimatingOverlay.
                                super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
                                super_IServerUnknown.super_IHandleEntity + 0x154))(pCVar9),
     cVar7 != '\0')) {
    pcVar5 = *(code **)(*piVar8 + 0x30);
    uVar10 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4cd2e2) + 0x40))
                       ((int *)**(undefined4 **)(unaff_EBX + 0x4cd2e2),
                        (pCVar9->super_CBaseFlex).super_CBaseAnimatingOverlay.super_CBaseAnimating.
                        super_CBaseEntity.m_Network.m_pPev);
    (*pcVar5)(piVar8,unaff_EBX + 0x2dfb26,uVar10);
                    /* Unresolved local var: int nPaintCount@[???]
                       Unresolved local var: int i@[???] */
    (**(code **)(*piVar8 + 0x30))
              (piVar8,unaff_EBX + 0x2e6e9c,
               (((((uint)(this->m_bHasPaint).m_Value[0] -
                  (uint)((this->m_bHasPaint).m_Value[1] == false)) -
                 (uint)((this->m_bHasPaint).m_Value[2] == false)) -
                (uint)((this->m_bHasPaint).m_Value[3] == false)) + 4) -
               (uint)((this->m_bHasPaint).m_Value[4] == false));
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4cd2ea) + 0x20))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4cd2ea),piVar8,0);
  }
  (**(code **)((int)(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                    super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.
                    super_IServerEntity.super_IServerUnknown + 0x3b0))(this,nIndex);
  return;
}


/* CWeaponPaintGun::GetPaintCount at 00778d70 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: int GetPaintCount(CWeaponPaintGun * this) */

int __thiscall CWeaponPaintGun::GetPaintCount(CWeaponPaintGun *this)

{
                    /* Unresolved local var: int nPaintCount@[???] */
                    /* Unresolved local var: int i@[???] */
  return (((((uint)(this->m_bHasPaint).m_Value[0] - (uint)((this->m_bHasPaint).m_Value[1] == false))
           - (uint)((this->m_bHasPaint).m_Value[2] == false)) -
          (uint)((this->m_bHasPaint).m_Value[3] == false)) + 4) -
         (uint)((this->m_bHasPaint).m_Value[4] == false);
}


/* CWeaponPaintGun::ResetPaint at 00779ef0 */

/* WARNING: Restarted to delay deadcode elimination for space: stack */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void ResetPaint(CWeaponPaintGun * this) */

void __thiscall CWeaponPaintGun::ResetPaint(CWeaponPaintGun *this)

{
  undefined1 *puVar1;
  ushort uVar2;
  CBaseEdict *this_00;
  ushort *puVar3;
  int *piVar4;
  IChangeInfoAccessor *pIVar5;
  uint uVar6;
  ushort uVar7;
  uint uVar8;
  int unaff_EBX;
  int iVar9;
  ushort i;
  int local_64;
  CRecipientFilter local_40;
  bool local_1d;
  
                    /* Unresolved local var: CBroadcastRecipientFilter filter@[???] */
  ___i686_get_pc_thunk_bx();
  local_64 = 0;
  do {
    local_1d = false;
    if ((this->m_bHasPaint).m_Value[local_64] != false) {
      if ((this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.super_CBaseCombatWeapon.
          super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
        this_00 = &((this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                    super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)
                   ->super_CBaseEdict;
        if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
          uVar7 = (short)local_64 + 0x644;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
          pIVar5 = CBaseEdict::GetChangeAccessor(this_00);
          puVar3 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4ccc67);
          if (pIVar5->m_iChangeInfoSerialNumber == *puVar3) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            uVar8 = (uint)pIVar5->m_iChangeInfo;
            uVar2 = puVar3[uVar8 * 0x14 + 0x14];
            if (uVar2 == 0) {
LAB_0077a0f1:
              puVar3[(uint)uVar2 + uVar8 * 0x14 + 1] = uVar7;
              puVar3[uVar8 * 0x14 + 0x14] = uVar2 + 1;
            }
            else if (puVar3[uVar8 * 0x14 + 1] != uVar7) {
              _i = 0;
              do {
                uVar6 = _i + 1;
                _i = uVar6 & 0xffff;
                if ((ushort)uVar6 == uVar2) {
                  if (uVar2 != 0x13) goto LAB_0077a0f1;
                  goto LAB_0077a057;
                }
              } while (puVar3[uVar8 * 0x14 + _i + 1] != uVar7);
            }
          }
          else if (puVar3[0x7d1] == 100) {
LAB_0077a057:
            pIVar5->m_iChangeInfoSerialNumber = 0;
            this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar5->m_iChangeInfo = puVar3[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x4ccc67) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x4ccc67) + 0xfa2) + 1;
            piVar4 = *(int **)(unaff_EBX + 0x4ccc67);
            pIVar5->m_iChangeInfoSerialNumber = *(ushort *)*piVar4;
            iVar9 = *piVar4 + (uint)pIVar5->m_iChangeInfo * 0x28;
            *(ushort *)(iVar9 + 2) = uVar7;
            *(undefined2 *)(iVar9 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                  super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                  field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->m_bHasPaint).m_Value[local_64] = local_1d;
    }
    local_64 = local_64 + 1;
    if (local_64 == 4) {
      CRecipientFilter::CRecipientFilter(&local_40);
      local_40.super_IRecipientFilter._vptr_IRecipientFilter =
           (_func_int_varargs **)(unaff_EBX + 0x4cf2e7);
      CRecipientFilter::AddAllPlayers(&local_40);
      CRecipientFilter::MakeReliable(&local_40);
      iVar9 = 4;
      SetCurrentPaint(this,NO_POWER);
      local_40.super_IRecipientFilter._vptr_IRecipientFilter =
           (_func_int_varargs **)(unaff_EBX + 0x4cf2e7);
      CRecipientFilter::~CRecipientFilter(&local_40,iVar9);
      return;
    }
  } while( true );
}


/* CWeaponPaintGun::DeactivatePaint at 00779390 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void DeactivatePaint(CWeaponPaintGun * this, PaintPowerType nIndex) */

void __thiscall CWeaponPaintGun::DeactivatePaint(CWeaponPaintGun *this,PaintPowerType nIndex)

{
  uint *puVar1;
  int iVar2;
  ushort uVar3;
  CBaseEdict *this_00;
  ushort *puVar4;
  int *piVar5;
  IChangeInfoAccessor *pIVar6;
  uint uVar7;
  uint uVar8;
  ushort uVar9;
  int unaff_EBX;
  uint local_70;
  
  ___i686_get_pc_thunk_bx();
  if ((this->m_bHasPaint).m_Value[nIndex] != false) {
    if (*(bool *)((int)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                        super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                        m_TimerEvent + 0x10) == false) {
      this_00 = &((this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                  super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->
                 super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        uVar9 = (short)nIndex + 0x644;
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4cd7c1);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00779514:
            puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = uVar9;
            puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != uVar9) {
            local_70 = 0;
            do {
              uVar8 = local_70 + 1;
              local_70 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_00779514;
                pIVar6->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar7 * 0x14 + local_70 + 1] != uVar9);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4cd7c1) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4cd7c1) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x4cd7c1);
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar2 = *piVar5 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(ushort *)(iVar2 + 2) = uVar9;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = (uint *)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                        super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                        field_0x4c;
      *puVar1 = *puVar1 | 1;
    }
    (this->m_bHasPaint).m_Value[nIndex] = false;
  }
  return;
}


/* CWeaponPaintGun::CleansePaint at 0077a150 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void CleansePaint(CWeaponPaintGun * this) */

void __thiscall CWeaponPaintGun::CleansePaint(CWeaponPaintGun *this)

{
  bool bVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  bVar1 = HasAnyPaintPower(this);
  if (!bVar1) {
    return;
  }
  CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x33c451),0.0,(float *)0x0);
  ResetPaint(this);
  return;
}


/* CWeaponPaintGun::SetCurrentPaint at 00779c90 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void SetCurrentPaint(CWeaponPaintGun * this, PaintPowerType nIndex) */

void __thiscall CWeaponPaintGun::SetCurrentPaint(CWeaponPaintGun *this,PaintPowerType nIndex)

{
  uint *puVar1;
  uint uVar2;
  int iVar3;
  ushort uVar4;
  CBaseEdict *this_00;
  int *piVar5;
  ushort *puVar6;
  bool bVar7;
  char cVar8;
  CBasePlayer *pCVar9;
  IChangeInfoAccessor *pIVar10;
  uint uVar11;
  int unaff_EBX;
  CBasePlayer *player;
  uint uVar12;
  char *messagename;
  CRecipientFilter local_3c;
  
                    /* Unresolved local var: CBaseEntity * pOwner@[???]
                       Unresolved local var: CSingleUserRecipientFilter filter@[???] */
  ___i686_get_pc_thunk_bx();
  bVar7 = HasPaintPower(this,nIndex);
  if ((bVar7) && (nIndex != (this->m_nCurrentColor).m_Value)) {
    if (*(bool *)((int)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                        super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                        m_TimerEvent + 0x10) == false) {
      this_00 = &((this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                  super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->
                 super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(this_00);
        piVar5 = *(int **)(&DAT_004cceca + unaff_EBX);
        puVar6 = (ushort *)*piVar5;
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar6) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar12 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar4 = puVar6[uVar12 * 0x14 + 0x14];
          if (uVar4 == 0) {
LAB_00779e86:
            puVar6[(uint)uVar4 + uVar12 * 0x14 + 1] = 0x664;
            puVar6[uVar12 * 0x14 + 0x14] = uVar4 + 1;
          }
          else if (puVar6[uVar12 * 0x14 + 1] != 0x664) {
            uVar11 = 0;
            do {
              uVar2 = uVar11 + 1;
              uVar11 = uVar2 & 0xffff;
              if ((ushort)uVar2 == uVar4) {
                if (uVar4 != 0x13) goto LAB_00779e86;
                goto LAB_00779ea7;
              }
            } while (puVar6[uVar12 * 0x14 + uVar11 + 1] != 0x664);
          }
        }
        else if (puVar6[0x7d1] == 100) {
LAB_00779ea7:
          pIVar10->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar6[0x7d1];
          *(short *)(*piVar5 + 0xfa2) = *(short *)(*piVar5 + 0xfa2) + 1;
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar3 = *piVar5 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar3 + 2) = 0x664;
          *(undefined2 *)(iVar3 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = (uint *)&(this->super_CBasePortalCombatWeapon).super_CWeaponPortalBase.
                        super_CBaseCombatWeapon.super_CBaseAnimating.super_CBaseEntity.m_Network.
                        field_0x4c;
      *puVar1 = *puVar1 | 1;
    }
    (this->m_nCurrentColor).m_Value = nIndex;
  }
  pCVar9 = (CBasePlayer *)CBaseCombatWeapon::GetOwner((CBaseCombatWeapon *)this);
  if (pCVar9 != (CBasePlayer *)0x0) {
    cVar8 = (**(code **)((int)(pCVar9->super_CBaseCombatCharacter).super_CBaseFlex.
                              super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                              super_IServerEntity.super_IServerUnknown.super_IHandleEntity + 0x154))
                      (pCVar9);
    player = (CBasePlayer *)0x0;
    if (cVar8 != '\0') {
      player = pCVar9;
    }
    CRecipientFilter::CRecipientFilter(&local_3c);
    local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
         (_func_int_varargs **)(&UNK_004cf44a + unaff_EBX);
    CRecipientFilter::AddRecipient(&local_3c,player);
    CRecipientFilter::MakeReliable(&local_3c);
    messagename = (char *)(unaff_EBX + 0x2e5f94);
    UserMessageBegin(&local_3c.super_IRecipientFilter,messagename);
    MessageWriteEHandle((CBaseEntity *)this);
    MessageWriteByte(nIndex);
    MessageEnd();
    local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
         (_func_int_varargs **)(&UNK_004cf44a + unaff_EBX);
    CRecipientFilter::~CRecipientFilter(&local_3c,(int)messagename);
  }
  return;
}


/* CWeaponPaintGun::PaintGunThink at 00778eb0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void PaintGunThink(CWeaponPaintGun * this) */

void __thiscall CWeaponPaintGun::PaintGunThink(CWeaponPaintGun *this)

{
  BASEPTR func;
  char cVar1;
  CPortal_Player *this_00;
  int unaff_EBX;
  Vector local_28 [2];
  
                    /* Unresolved local var: CPortal_Player * pPlayer@[???] */
  ___i686_get_pc_thunk_bx();
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x14);
  CBaseEntity::ThinkSet
            ((CBaseEntity *)this,func,*(float *)(**(int **)(unaff_EBX + 0x4cdca0) + 0xc),
             (char *)(unaff_EBX + 0x33d753));
  this_00 = (CPortal_Player *)CBaseCombatWeapon::GetOwner((CBaseCombatWeapon *)this);
  if (this_00 != (CPortal_Player *)0x0) {
    cVar1 = (**(code **)(*(int *)&(this_00->
                                  super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                                  super_CPaintableEntity<CBaseMultiplayerPlayer>.
                                  super_CBaseMultiplayerPlayer.super_CAI_ExpresserHost<CBasePlayer>.
                                  super_CBasePlayer.super_CBaseCombatCharacter.super_CBaseFlex +
                        0x154))(this_00);
    if (cVar1 != '\0') {
      CPortal_Player::GetPaintGunShootPosition(local_28,this_00);
      (this->m_vecOldBlobFirePos).x = local_28[0].x;
      (this->m_vecOldBlobFirePos).y = local_28[0].y;
      (this->m_vecOldBlobFirePos).z = local_28[0].z;
    }
  }
  return;
}


/* GiveAllPaintPowers at 00779b20 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void GiveAllPaintPowers(void)

{
  char cVar1;
  CWeaponPaintGun *this;
  CBasePlayer *pPlayer;
  int unaff_EBX;
  CBaseEntity *pFoundEnt;
  
  ___i686_get_pc_thunk_bx();
  pFoundEnt = CGlobalEntityList::FindEntityByClassname
                        (*(CGlobalEntityList **)
                          (CTEFoundryHelpers::~CTEFoundryHelpers + unaff_EBX + 6),(CBaseEntity *)0x0
                         ,(char *)(unaff_EBX + 0x2e2d5a));
  if (pFoundEnt != (CBaseEntity *)0x0) {
    do {
      this = (CWeaponPaintGun *)
             ___dynamic_cast(pFoundEnt,*(undefined4 *)(unaff_EBX + 0x4cd062),unaff_EBX + 0x561dca,0)
      ;
      if (this != (CWeaponPaintGun *)0x0) {
        pPlayer = (CBasePlayer *)CBaseCombatWeapon::GetOwner((CBaseCombatWeapon *)this);
        if ((pPlayer == (CBasePlayer *)0x0) ||
           (cVar1 = (**(code **)((int)(pPlayer->super_CBaseCombatCharacter).super_CBaseFlex.
                                      super_CBaseAnimatingOverlay.super_CBaseAnimating.
                                      super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
                                      super_IHandleEntity + 0x154))(pPlayer), cVar1 == '\0')) {
          pPlayer = (CBasePlayer *)0x0;
        }
        CWeaponPaintGun::ActivatePaint(this,BOUNCE_POWER);
        CWeaponPaintGun::ActivatePaint(this,SPEED_POWER);
        CWeaponPaintGun::ActivatePaint(this,STICK_POWER);
        CWeaponPaintGun::ActivatePaint(this,PORTAL_POWER);
        PaintPowerPickup(0,pPlayer);
        PaintPowerPickup(2,pPlayer);
        PaintPowerPickup(1,pPlayer);
        PaintPowerPickup(3,pPlayer);
      }
      pFoundEnt = CGlobalEntityList::FindEntityByClassname
                            (*(CGlobalEntityList **)
                              (CTEFoundryHelpers::~CTEFoundryHelpers + unaff_EBX + 6),pFoundEnt,
                             (char *)(unaff_EBX + 0x2e2d5a));
    } while (pFoundEnt != (CBaseEntity *)0x0);
  }
  return;
}


/* __tcf_3 at 00a415f0 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x410508),in_stack_00000008);
  return;
}


/* _GLOBAL__I__ZN15CWeaponPaintGun14GetServerClassEv at 000bc300 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN15CWeaponPaintGun14GetServerClassEv(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

