/* DWARF-guided pseudocode for game/server/portal2/trigger_tractorbeam.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* DataMapInit<CProjectedTractorBeamEntity> at 000bb3a0 */

datamap_t * DataMapInit<CProjectedTractorBeamEntity>(CProjectedTractorBeamEntity *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&paintgun_blobs_min_streak_time.super_ConCommandBase.m_pszName + unaff_EBX + 3)
      == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xd966af);
    if (iVar1 != 0) {
      *(int *)((int)&paintgun_blobs_min_streak_time.super_ConCommandBase.m_nFlags + unaff_EBX + 3) =
           unaff_EBX + 0x9fb0b7;
      *(undefined4 *)((int)&paintgun_blobs_min_streak_time.m_pParent + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&paintgun_blobs_min_streak_time.m_pszDefaultValue + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&paintgun_blobs_min_streak_time.m_Value.m_pszString + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&paintgun_blobs_min_streak_time.m_Value.m_StringLength + unaff_EBX + 3) =
           0;
      *(undefined4 *)((int)&paintgun_blobs_min_streak_time.m_Value.m_fValue + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&paintgun_blobs_min_streak_time.super_IConVar._vptr_IConVar + unaff_EBX + 3) = 0x1b;
      ___cxa_guard_release(unaff_EBX + 0xd966af);
      ___cxa_atexit(unaff_EBX + 0x985fef,0,*(undefined4 *)(&DAT_00b8b7a3 + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_MyCombatCharacterPointer_00cb05d8 + unaff_EBX + 3) =
       *(undefined4 *)(&DAT_00b8c30b + unaff_EBX);
  *(undefined4 *)((int)&PTR_IsNPC_00cb05d0 + unaff_EBX + 3) = 1;
  *(int *)((int)&PTR_IsTriggered_00cb05cc + unaff_EBX + 3) = unaff_EBX + 0xcb0e8f;
  return (datamap_t *)((int)&PTR_IsTriggered_00cb05cc + unaff_EBX + 3);
}


/* __static_initialization_and_destruction_0 at 000bb790 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *pcVar1;
  SendTable *pSVar2;
  datamap_t *pdVar3;
  int iVar4;
  int iVar5;
  IEntityFactoryDictionary *pIVar6;
  int unaff_EBX;
  undefined4 *puVar7;
  undefined4 *puVar8;
  undefined4 *puVar9;
  longlong lVar10;
  
  lVar10 = ___i686_get_pc_thunk_bx();
  if (lVar10 == 0xffff00000001) {
    *(undefined1 *)((int)&sv_auto_taunt_pip.m_pParent + unaff_EBX) = 0;
    *(undefined1 *)((int)&sv_auto_taunt_pip.m_pParent + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&sv_auto_taunt_pip.m_pParent + unaff_EBX + 2) = 0;
    *(undefined1 *)((int)&sv_auto_taunt_pip.m_pParent + unaff_EBX + 3) = 0;
    *(undefined4 *)((int)&sv_auto_taunt_pip.m_pszDefaultValue + unaff_EBX) = 0;
    *(undefined4 *)((int)&sv_auto_taunt_pip.m_Value.m_pszString + unaff_EBX) = 0;
    *(undefined4 *)((int)&sv_auto_taunt_pip.m_Value.m_StringLength + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&sv_auto_taunt_pip.m_Value.m_fValue + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&sv_auto_taunt_pip.m_Value.m_nValue + unaff_EBX) = 0;
    *(undefined4 *)(&sv_auto_taunt_pip.m_bHasMin + unaff_EBX) = 0;
    *(undefined4 *)((int)&sv_auto_taunt_pip.m_fMinVal + unaff_EBX) = 0;
    *(undefined4 *)(&sv_auto_taunt_pip.m_bHasMax + unaff_EBX) = 0;
    *(undefined4 *)((int)&sv_auto_taunt_pip.m_fMaxVal + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&sv_auto_taunt_pip.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)
     ((int)&sv_auto_taunt_pip.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&sv_auto_taunt_pip.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) =
         0x7f7fffff;
    *(undefined **)((int)&sv_auto_taunt_pip.m_fnChangeCallbacks.m_Size + unaff_EBX) =
         &UNK_00b90b64 + unaff_EBX;
    pdVar3 = DataMapInit<CProjectedTractorBeamEntity>((CProjectedTractorBeamEntity *)0x0);
    *(datamap_t **)((int)s_ElementNames + unaff_EBX + 0x8a4) = pdVar3;
    iVar5 = unaff_EBX + 0xd95994;
    pcVar1 = &UNK_009facc4 + unaff_EBX;
    *(char **)((int)&sv_auto_taunt_pip.m_fnChangeCallbacks.m_pElements + unaff_EBX) = pcVar1;
    pSVar2 = (SendTable *)((int)s_ElementNames + unaff_EBX + 0x8a8);
    *(SendTable **)(&DAT_00d95998 + unaff_EBX) = pSVar2;
    *(undefined4 *)((int)&sv_paint_trigger_sound_delay.super_ConCommandBase.m_pNext + unaff_EBX) =
         0xffff;
    puVar9 = (undefined4 *)**(int **)(&DAT_00b8b450 + unaff_EBX);
    if (puVar9 == (undefined4 *)0x0) {
      **(int **)(&DAT_00b8b450 + unaff_EBX) = iVar5;
      *(undefined4 *)(&DAT_00d9599c + unaff_EBX) = 0;
    }
    else {
      puVar8 = (undefined4 *)puVar9[2];
      iVar4 = _V_stricmp((char *)*puVar9,pcVar1);
      if (iVar4 < 1) {
        if (puVar8 != (undefined4 *)0x0) {
          do {
            puVar7 = puVar8;
            iVar4 = _V_stricmp((char *)*puVar7,pcVar1);
            puVar8 = puVar7;
            if (0 < iVar4) goto LAB_000bbaa0;
            puVar8 = (undefined4 *)puVar7[2];
            puVar9 = puVar7;
          } while ((undefined4 *)puVar7[2] != (undefined4 *)0x0);
          puVar8 = (undefined4 *)0x0;
        }
LAB_000bbaa0:
        *(undefined4 **)(&DAT_00d9599c + unaff_EBX) = puVar8;
        puVar9[2] = iVar5;
      }
      else {
        *(undefined4 *)(&DAT_00d9599c + unaff_EBX) = **(undefined4 **)(&DAT_00b8b450 + unaff_EBX);
        **(int **)(&DAT_00b8b450 + unaff_EBX) = iVar5;
      }
    }
    SendTable::SendTable(pSVar2);
    ___cxa_atexit(unaff_EBX + 0x985bdc,0,*(undefined4 *)(&DAT_00b8b3b0 + unaff_EBX));
    iVar5 = ServerClassInit<DT_ProjectedTractorBeamEntity::ignored>((ignored *)0x0);
    *(int *)((int)s_ElementNames + unaff_EBX + 0x8bc) = iVar5;
    *(undefined **)(&sv_paint_trigger_sound_delay.super_ConCommandBase.m_bRegistered + unaff_EBX) =
         &UNK_00c1fb1c + unaff_EBX;
    pIVar6 = EntityFactoryDictionary();
    (**pIVar6->_vptr_IEntityFactoryDictionary)(pIVar6,unaff_EBX + 0xd959a8,unaff_EBX + 0x99f7a2);
    pdVar3 = DataMapInit<CTrigger_TractorBeam>((CTrigger_TractorBeam *)0x0);
    *(datamap_t **)((int)s_ElementNames + unaff_EBX + 0x8c0) = pdVar3;
    *(undefined **)((int)&sv_paint_trigger_sound_delay.super_ConCommandBase.m_pszName + unaff_EBX) =
         &UNK_00c1fb3c + unaff_EBX;
    pIVar6 = EntityFactoryDictionary();
    (**pIVar6->_vptr_IEntityFactoryDictionary)(pIVar6,unaff_EBX + 0xd959ac,unaff_EBX + 0x99f7c0);
    iVar5 = unaff_EBX + 0xd959b0;
    pcVar1 = &UNK_009face0 + unaff_EBX;
    *(char **)((int)&sv_paint_trigger_sound_delay.super_ConCommandBase.m_pszHelpString + unaff_EBX)
         = pcVar1;
    pSVar2 = (SendTable *)((int)s_ElementNames + unaff_EBX + 0x8c4);
    *(SendTable **)((int)&sv_paint_trigger_sound_delay.super_ConCommandBase.m_nFlags + unaff_EBX) =
         pSVar2;
    *(undefined4 *)((int)&sv_paint_trigger_sound_delay.m_pszDefaultValue + unaff_EBX) = 0xffff;
    puVar9 = (undefined4 *)**(int **)(&DAT_00b8b450 + unaff_EBX);
    if (puVar9 == (undefined4 *)0x0) {
      **(int **)(&DAT_00b8b450 + unaff_EBX) = iVar5;
      *(undefined4 *)((int)&sv_paint_trigger_sound_delay.super_IConVar._vptr_IConVar + unaff_EBX) =
           0;
    }
    else {
      puVar8 = (undefined4 *)puVar9[2];
      iVar4 = _V_stricmp((char *)*puVar9,pcVar1);
      if (iVar4 < 1) {
        while ((puVar8 != (undefined4 *)0x0 &&
               (iVar4 = _V_stricmp((char *)*puVar8,pcVar1), iVar4 < 1))) {
          puVar9 = puVar8;
          puVar8 = (undefined4 *)puVar8[2];
        }
        *(undefined4 **)((int)&sv_paint_trigger_sound_delay.super_IConVar._vptr_IConVar + unaff_EBX)
             = puVar8;
        puVar9[2] = iVar5;
      }
      else {
        *(undefined4 *)((int)&sv_paint_trigger_sound_delay.super_IConVar._vptr_IConVar + unaff_EBX)
             = **(undefined4 **)(&DAT_00b8b450 + unaff_EBX);
        **(int **)(&DAT_00b8b450 + unaff_EBX) = iVar5;
      }
    }
    SendTable::SendTable(pSVar2);
    ___cxa_atexit(unaff_EBX + 0x985bbc,0,*(undefined4 *)(&DAT_00b8b3b0 + unaff_EBX));
    iVar5 = ServerClassInit<DT_Trigger_TractorBeam::ignored>((ignored *)0x0);
    *(int *)((int)s_ElementNames + unaff_EBX + 0x8d8) = iVar5;
  }
  return;
}


/* CProjectedTractorBeamEntity::GetDataDescMap at 007763d0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CProjectedTractorBeamEntity * this) */

datamap_t * __thiscall
CProjectedTractorBeamEntity::GetDataDescMap(CProjectedTractorBeamEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5f55a8);
}


/* CProjectedTractorBeamEntity::GetBaseMap at 007763e0 */

datamap_t * CProjectedTractorBeamEntity::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4d12d4);
}


/* __tcf_0 at 00a413a0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x4106ce)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x4106c2) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x4106ce));
  }
  *(undefined4 *)(unaff_EBX + 0x4106ce) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x4106ca)) {
    if (*(int *)(unaff_EBX + 0x4106c2) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2057ae) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2057ae),*(int *)(unaff_EBX + 0x4106c2));
      *(undefined4 *)(unaff_EBX + 0x4106c2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x4106c6) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x4106c2);
  *(int *)(&DAT_004106d2 + unaff_EBX) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x4106ca)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2057ae) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2057ae),iVar1);
      *(undefined4 *)(unaff_EBX + 0x4106c2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x4106c6) = 0;
  }
  return;
}


/* ServerClassInit<DT_ProjectedTractorBeamEntity::ignored> at 000bb1d0 */

int ServerClassInit<DT_ProjectedTractorBeamEntity::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&DAT_00d9675c)[unaff_EBX] == '\0') {
    iVar1 = ___cxa_guard_acquire(&DAT_00d9675c + unaff_EBX);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  ((int)&paintgun_blobs_min_streak_speed_dampen.m_pParent + unaff_EBX),
                  &UNK_0099aa50 + unaff_EBX,0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&paintgun_blobs_max_streak_speed_dampen.super_ConCommandBase.
                               m_pszHelpString + unaff_EBX),&UNK_0099ce00 + unaff_EBX,0,
                        (SendTable *)**(undefined4 **)(&DAT_00b8c4e0 + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00b8ba0c + unaff_EBX),0x80);
      SendPropEHandle((SendProp_conflict *)
                      ((int)&DT_WeaponPortalBase::g_SendTable.m_pPrecalc + unaff_EBX),
                      (char *)(unaff_EBX + 0x9fb26e),0x420,4,-1,
                      *(SendVarProxyFn_conflict *)(&DAT_00b8b9f8 + unaff_EBX));
      ___cxa_guard_release(&DAT_00d9675c + unaff_EBX);
      ___cxa_atexit(unaff_EBX + 0x9860ec,0,*(undefined4 *)(&DAT_00b8b970 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)((int)s_ElementNames + unaff_EBX + 0xe68),
             (SendProp_conflict *)
             ((int)&paintgun_blobs_max_streak_speed_dampen.super_ConCommandBase.m_pszHelpString +
             unaff_EBX),2,*(char **)(&DAT_00c200c0 + unaff_EBX));
  return 1;
}


/* CProjectedTractorBeamEntity::GetServerClass at 007763f0 */

/* DWARF original prototype: ServerClass * GetServerClass(CProjectedTractorBeamEntity * this) */

ServerClass * __thiscall
CProjectedTractorBeamEntity::GetServerClass(CProjectedTractorBeamEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x6dad40);
}


/* CProjectedTractorBeamEntity::YouForgotToImplementOrDeclareServerClass at 00776400 */

/* DWARF original prototype: int
   YouForgotToImplementOrDeclareServerClass(CProjectedTractorBeamEntity * this) */

int __thiscall
CProjectedTractorBeamEntity::YouForgotToImplementOrDeclareServerClass
          (CProjectedTractorBeamEntity *this)

{
  return 0;
}


/* __tcf_4 at 00a41380 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x3893c4),in_stack_00000008);
  return;
}


/* __tcf_1 at 00a412d0 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410727))(unaff_EBX + 0x410727);
  (*(code *)**(undefined4 **)(&DAT_004106d3 + unaff_EBX))(&DAT_004106d3 + unaff_EBX);
                    /* WARNING: Could not recover jumptable at 0x00a41317. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41067f))();
  return;
}


/* DataMapInit<CTrigger_TractorBeam> at 000bb470 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

datamap_t * DataMapInit<CTrigger_TractorBeam>(CTrigger_TractorBeam *param_1)

{
  int iVar1;
  int iVar2;
  ISaveRestoreOps *pIVar3;
  char *pcVar4;
  size_t sVar5;
  int iVar6;
  undefined4 uVar7;
  int iVar8;
  undefined4 *puVar9;
  int unaff_EBX;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  if ((*(char *)((int)&paintgun_blobs_spread_angle.m_Value.m_fValue + unaff_EBX) == '\0') &&
     (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd9648c), iVar2 != 0)) {
    *(int *)(&paintgun_blobs_spread_angle.m_bHasMax + unaff_EBX) = unaff_EBX + 0x9fb000;
    *(undefined4 *)
     ((int)&paintgun_blobs_spread_angle.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&paintgun_blobs_spread_angle.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&paintgun_blobs_spread_angle.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) = 0;
    *(undefined4 *)((int)&paintgun_blobs_spread_angle.m_fnChangeCallbacks.m_Size + unaff_EBX) = 0;
    *(undefined4 *)((int)&paintgun_blobs_spread_angle.m_fnChangeCallbacks.m_pElements + unaff_EBX) =
         0;
    *(undefined4 *)((int)&paintgun_blobs_spread_angle.m_fMaxVal + unaff_EBX) = 0x14;
    ___cxa_guard_release(unaff_EBX + 0xd9648c);
    ___cxa_atexit(unaff_EBX + 0x985ffc,0,*(undefined4 *)(&DAT_00b8b6d0 + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_OnParseMapDataFinished_00cb0524 + unaff_EBX) =
       *(undefined4 *)(&DAT_00b8c260 + unaff_EBX);
  if (((&paintgun_blobs_spread_angle.m_bHasMin)[unaff_EBX] != false) ||
     (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd96494), iVar2 == 0)) goto LAB_000bb4c7;
  pIVar3 = GetPhysObjSaveRestoreOps(PIID_IPHYSICSMOTIONCONTROLLER);
  *(ISaveRestoreOps **)((int)&PTR_Classify_00cb0590 + unaff_EBX) = pIVar3;
  pIVar3 = GetSoundSaveRestoreOps();
  *(ISaveRestoreOps **)((int)&PTR_KeyValue_00cb0850 + unaff_EBX) = pIVar3;
  pcVar4 = operator_new___(*(int *)((int)&paintgun_blobs_spread_angle.m_fMaxVal + unaff_EBX) + 0xd);
  _strcpy(pcVar4,*(char **)(&paintgun_blobs_spread_angle.m_bHasMax + unaff_EBX));
  sVar5 = _strlen(pcVar4);
  builtin_strncpy(pcVar4 + sVar5,"TractorThink",0xd);
  iVar1 = *(int *)((int)&paintgun_blobs_spread_angle.m_fnChangeCallbacks.m_Size + unaff_EBX);
  iVar2 = iVar1 + 1;
  iVar6 = *(int *)((int)&paintgun_blobs_spread_angle.m_fnChangeCallbacks.m_Memory.m_nAllocationCount
                  + unaff_EBX);
  if (iVar6 < iVar2) {
    iVar8 = *(int *)((int)&paintgun_blobs_spread_angle.m_fnChangeCallbacks.m_Memory.m_nGrowSize +
                    unaff_EBX);
    if (-1 < iVar8) {
      if (iVar8 == 0) {
        if ((iVar6 == 0) && (iVar6 = 8, iVar2 < 9)) {
          local_20 = 0x20;
        }
        else {
          do {
            local_20 = iVar6;
            iVar6 = local_20 * 2;
          } while (iVar6 < iVar2);
          local_20 = local_20 * 8;
        }
        goto LAB_000bb610;
      }
      iVar6 = (iVar1 / iVar8 + 1) * iVar8;
      if (iVar6 < iVar2) {
        if (iVar6 != 0) {
LAB_000bb5d0:
          do {
            iVar6 = (iVar6 + iVar2) / 2;
          } while (iVar6 < iVar2);
          goto LAB_000bb5e0;
        }
        if (iVar2 < 0) {
          iVar6 = -1;
          local_20 = -4;
        }
        else {
          local_20 = 0;
          if (iVar2 != 0) goto LAB_000bb5d0;
        }
      }
      else {
LAB_000bb5e0:
        local_20 = iVar6 * 4;
      }
LAB_000bb610:
      *(int *)((int)&paintgun_blobs_spread_angle.m_fnChangeCallbacks.m_Memory.m_nAllocationCount +
              unaff_EBX) = iVar6;
      iVar6 = *(int *)((int)&paintgun_blobs_spread_angle.m_fnChangeCallbacks.m_Memory.m_pMemory +
                      unaff_EBX);
      if (iVar6 == 0) {
        uVar7 = (*(code *)**(undefined4 **)**(undefined4 **)(&DAT_00b8b6d8 + unaff_EBX))
                          ((undefined4 *)**(undefined4 **)(&DAT_00b8b6d8 + unaff_EBX),local_20);
      }
      else {
        uVar7 = (**(code **)(*(int *)**(undefined4 **)(&DAT_00b8b6d8 + unaff_EBX) + 4))
                          ((int *)**(undefined4 **)(&DAT_00b8b6d8 + unaff_EBX),iVar6,local_20);
      }
      *(undefined4 *)
       ((int)&paintgun_blobs_spread_angle.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) =
           uVar7;
    }
  }
  iVar8 = *(int *)((int)&paintgun_blobs_spread_angle.m_fnChangeCallbacks.m_Size + unaff_EBX) + 1;
  *(int *)((int)&paintgun_blobs_spread_angle.m_fnChangeCallbacks.m_Size + unaff_EBX) = iVar8;
  iVar6 = *(int *)((int)&paintgun_blobs_spread_angle.m_fnChangeCallbacks.m_Memory.m_pMemory +
                  unaff_EBX);
  *(int *)((int)&paintgun_blobs_spread_angle.m_fnChangeCallbacks.m_pElements + unaff_EBX) = iVar6;
  iVar8 = (iVar8 - iVar1) + -1;
  if (0 < iVar8) {
    _V_memmove((void *)(iVar6 + iVar2 * 4),(void *)(iVar6 + iVar1 * 4),iVar8 * 4);
    iVar6 = *(int *)((int)&paintgun_blobs_spread_angle.m_fnChangeCallbacks.m_Memory.m_pMemory +
                    unaff_EBX);
  }
  puVar9 = (undefined4 *)(iVar1 * 4 + iVar6);
  if (puVar9 != (undefined4 *)0x0) {
    *puVar9 = pcVar4;
  }
  *(char **)((int)&PTR_StartTouch_00cb0d40 + unaff_EBX) = pcVar4;
  ___cxa_guard_release(unaff_EBX + 0xd96494);
LAB_000bb4c7:
  *(undefined4 *)((int)&PTR_PostConstructor_00cb051c + unaff_EBX) = 0x20;
  *(int *)((int)&PTR_InitSharedVars_00cb0518 + unaff_EBX) =
       (int)&PTR_NetworkStateChanged_m_nNextThinkTick_00cb057c + unaff_EBX;
  return (datamap_t *)((int)&PTR_InitSharedVars_00cb0518 + unaff_EBX);
}


/* CTrigger_TractorBeam::GetDataDescMap at 00776410 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: datamap_t * GetDataDescMap(CTrigger_TractorBeam * this) */

datamap_t * __thiscall CTrigger_TractorBeam::GetDataDescMap(CTrigger_TractorBeam *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5f5584);
}


/* CTrigger_TractorBeam::GetBaseMap at 00776420 */

datamap_t * CTrigger_TractorBeam::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4d12bc);
}


/* __tcf_2 at 00a41480 */

void __tcf_2(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x4104a6)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(&DAT_0041049a + unaff_EBX) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x4104a6));
  }
  *(undefined4 *)(unaff_EBX + 0x4104a6) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x4104a2)) {
    if (*(int *)(&DAT_0041049a + unaff_EBX) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2056ce) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2056ce),*(int *)(&DAT_0041049a + unaff_EBX))
      ;
      *(undefined4 *)(&DAT_0041049a + unaff_EBX) = 0;
    }
    *(undefined4 *)(&DAT_0041049e + unaff_EBX) = 0;
  }
  iVar1 = *(int *)(&DAT_0041049a + unaff_EBX);
  *(int *)(unaff_EBX + 0x4104aa) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x4104a2)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2056ce) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2056ce),iVar1);
      *(undefined4 *)(&DAT_0041049a + unaff_EBX) = 0;
    }
    *(undefined4 *)(&DAT_0041049e + unaff_EBX) = 0;
  }
  return;
}


/* ServerClassInit<DT_Trigger_TractorBeam::ignored> at 000ba940 */

int ServerClassInit<DT_Trigger_TractorBeam::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&DT_WeaponPortalBase::g_SendTable.m_pProps + unaff_EBX + 2) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xd9681a);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  ((int)&CWeaponPortalBase_DataDescInit::g_DataMapHolder + unaff_EBX + 2),
                  (char *)(unaff_EBX + 0x99b2e6),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&sv_weapon_pickup_time_delay.m_Value.m_pszString + unaff_EBX + 2),
                        (char *)(unaff_EBX + 0x99d696),0,
                        (SendTable *)**(undefined4 **)(&DAT_00b8cd92 + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00b8c2a2 + unaff_EBX),0x80);
      SendPropBool((SendProp_conflict *)
                   ((int)&portalgun_fire_delay.super_IConVar._vptr_IConVar + unaff_EBX + 2),
                   (char *)(unaff_EBX + 0x99d1ae),0x3c0,1);
      SendPropFloat((SendProp_conflict *)
                    ((int)&portal2_square_portals.super_ConCommandBase.m_pszName + unaff_EBX + 2),
                    (char *)(unaff_EBX + 0xa052f4),0x3d4,4,0,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00b8c2fa + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&portal2_portal_width.super_ConCommandBase._vptr_ConCommandBase +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x9fba42),0x3d8,4,0,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00b8c2fa + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&portal2_portal_width.m_fnChangeCallbacks.m_pElements + unaff_EBX + 2),
                    (char *)(unaff_EBX + 0x9fba52),0x3dc,4,0,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00b8c2fa + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&use_server_portal_particles.m_fnChangeCallbacks.m_Memory.
                           m_nAllocationCount + unaff_EBX + 2),(char *)(unaff_EBX + 0xa05306),0x3e0,
                    4,0,4,0.0,-121121.125,*(SendVarProxyFn_conflict *)(&DAT_00b8c2fa + unaff_EBX),
                    0x80);
      SendPropFloat((SendProp_conflict *)
                    (&max_hitbox_damage_effects_per_entity.field_0x3e + unaff_EBX),
                    (char *)(unaff_EBX + 0x9fba60),0x3e4,4,0,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00b8c2fa + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&hitbox_damage_enabled.m_Value.m_nValue + unaff_EBX + 2),
                    (char *)(unaff_EBX + 0x9fba72),1000,4,0,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00b8c2fa + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&save_paintblob.super_ConCommandBase.m_pNext + unaff_EBX + 2),
                    (char *)(unaff_EBX + 0xa0531c),0x3ec,4,0,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00b8c2fa + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&g_PaintSaveRestoreBlockHandler.super_CDefSaveRestoreBlockHandler.
                           super_ISaveRestoreBlockHandler._vptr_ISaveRestoreBlockHandler +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x9fba85),0x3f0,4,0,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00b8c2fa + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&g_debug_physcannon.m_fnChangeCallbacks.m_Memory.m_nGrowSize +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x9dd412),0x3f4,4,0,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00b8c2fa + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&debug_viewmodel_grabcontroller.m_fMaxVal + unaff_EBX + 2),
                    (char *)(unaff_EBX + 0x9fba93),0x3f8,4,0,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00b8c2fa + unaff_EBX),0x80);
      SendPropQAngles((SendProp_conflict *)(&physcannon_maxmass.field_0x36 + unaff_EBX),
                      (char *)(unaff_EBX + 0x9fbaa2),0x3fc,0xc,0,4,
                      *(SendVarProxyFn_conflict *)(&DAT_00b8c366 + unaff_EBX),0x80);
      SendPropEHandle((SendProp_conflict *)
                      ((int)&hide_gun_when_holding.m_Value.m_StringLength + unaff_EBX + 2),
                      (char *)(unaff_EBX + 0x9fbab6),0x408,4,-1,
                      *(SendVarProxyFn_conflict *)(&DAT_00b8c28e + unaff_EBX));
      SendPropVector((SendProp_conflict *)
                     ((int)&player_held_object_offset_up_cube.m_pParent + unaff_EBX + 2),
                     (char *)(unaff_EBX + 0x9fbac5),0x40c,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00b8c2ee + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&player_held_object_offset_up_cube_vm.super_ConCommandBase.
                            m_pszHelpString + unaff_EBX + 2),(char *)(unaff_EBX + 0x9fbace),0x418,
                     0xc,0,4,0.0,-121121.125,*(SendVarProxyFn_conflict *)(&DAT_00b8c2ee + unaff_EBX)
                     ,0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&player_held_object_offset_up_sphere.super_ConCommandBase.m_pNext +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0xa05348),0x424,4,0,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00b8c2fa + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)(unaff_EBX + 0xd96e1a),(char *)(unaff_EBX + 0x9bf896),0x428
                    ,4,0,4,0.0,-121121.125,*(SendVarProxyFn_conflict *)(&DAT_00b8c2fa + unaff_EBX),
                    0x80);
      SendPropBool((SendProp_conflict *)
                   ((int)&player_held_object_look_down_adjustment.m_fnChangeCallbacks.m_Memory.
                          m_nGrowSize + unaff_EBX + 2),(char *)(unaff_EBX + 0x9ccb56),0x42c,1);
      SendPropBool((SendProp_conflict *)
                   ((int)&player_held_object_distance.m_fMaxVal + unaff_EBX + 2),
                   (char *)(unaff_EBX + 0x9fbad5),0x42d,1);
      SendPropBool((SendProp_conflict *)(&player_held_object_distance_vm.field_0x36 + unaff_EBX),
                   (char *)(unaff_EBX + 0x9fbae3),0x42e,1);
      SendPropBool((SendProp_conflict *)
                   ((int)&player_held_object_min_distance.m_Value.m_StringLength + unaff_EBX + 2),
                   (char *)(unaff_EBX + 0x9fbaef),0x42f,1);
      ___cxa_guard_release(unaff_EBX + 0xd9681a);
      ___cxa_atexit(&UNK_009869d2 + unaff_EBX,0,*(undefined4 *)(&DAT_00b8c206 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)((int)s_ElementNames + (int)(&UNK_0000171a + unaff_EBX)),
             (SendProp_conflict *)
             ((int)&sv_weapon_pickup_time_delay.m_Value.m_pszString + unaff_EBX + 2),0x16,
             *(char **)(&DAT_00c20952 + unaff_EBX));
  return 1;
}


/* CTrigger_TractorBeam::GetServerClass at 00776430 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: ServerClass * GetServerClass(CTrigger_TractorBeam * this) */

ServerClass * __thiscall CTrigger_TractorBeam::GetServerClass(CTrigger_TractorBeam *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x6dad1c);
}


/* CTrigger_TractorBeam::YouForgotToImplementOrDeclareServerClass at 00776440 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CTrigger_TractorBeam *
   this) */

int __thiscall
CTrigger_TractorBeam::YouForgotToImplementOrDeclareServerClass(CTrigger_TractorBeam *this)

{
  return 0;
}


/* __tcf_5 at 00a41360 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x389400),in_stack_00000008);
  return;
}


/* __tcf_3 at 00a41320 */

void __tcf_3(void *param_1)

{
  int unaff_EBX;
  undefined4 *puVar1;
  
  ___i686_get_pc_thunk_bx();
  puVar1 = (undefined4 *)(unaff_EBX + 0x41058a);
  do {
    (**(code **)*puVar1)(puVar1);
    puVar1 = puVar1 + -0x15;
  } while (puVar1 != (undefined4 *)(unaff_EBX + 0x40fdfe));
  return;
}


/* CTrigger_TractorBeam::CTrigger_TractorBeam at 00777fa0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void CTrigger_TractorBeam(CTrigger_TractorBeam * this) */

void __thiscall CTrigger_TractorBeam::CTrigger_TractorBeam(CTrigger_TractorBeam *this)

{
  undefined1 *puVar1;
  uint uVar2;
  ushort uVar3;
  CBaseEdict *this_00;
  int *piVar4;
  ushort *puVar5;
  int iVar6;
  int iVar7;
  undefined4 *puVar8;
  IChangeInfoAccessor *pIVar9;
  uint uVar10;
  int unaff_EBX;
  uint uVar11;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::CBaseEntity((CBaseEntity *)this,false);
  (this->super_CBaseVPhysicsTrigger).m_hFilter.super_CBaseHandle.m_Index = 0xffffffff;
  (this->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x562c14);
  (this->super_IMotionEvent)._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x562f40);
  (this->m_hProxyEntity).
  super_CNetworkVarBase<CBaseHandle,CTrigger_TractorBeam::NetworkVar_m_hProxyEntity>.m_Value.m_Index
       = 0xffffffff;
  uVar11 = (*(CUtlVector<CTrigger_TractorBeam*,CUtlMemory<CTrigger_TractorBeam*,_int>_> **)
             (unaff_EBX + 0x4cf738))->m_Size;
  CUtlVector<CTrigger_TractorBeam*,CUtlMemory<CTrigger_TractorBeam*,_int>_>::GrowVector
            (*(CUtlVector<CTrigger_TractorBeam*,CUtlMemory<CTrigger_TractorBeam*,_int>_> **)
              (unaff_EBX + 0x4cf738),1);
  iVar7 = ~uVar11 + (*(int **)(unaff_EBX + 0x4cf738))[3];
  if (0 < iVar7) {
    iVar6 = **(int **)(unaff_EBX + 0x4cf738);
    _V_memmove((void *)(iVar6 + 4 + uVar11 * 4),(void *)(iVar6 + uVar11 * 4),iVar7 * 4);
  }
  puVar8 = (undefined4 *)(uVar11 * 4 + **(int **)(unaff_EBX + 0x4cf738));
  if (puVar8 != (undefined4 *)0x0) {
    *puVar8 = this;
  }
  if (*(float *)(unaff_EBX + 0x35b2f4) != (this->m_flRadius).m_Value) {
    if ((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
        false) {
      this_00 = &((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_pPev)->
                 super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar9 = CBaseEdict::GetChangeAccessor(this_00);
        piVar4 = *(int **)(unaff_EBX + 0x4cebb4);
        puVar5 = (ushort *)*piVar4;
        if (pIVar9->m_iChangeInfoSerialNumber == *puVar5) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar11 = (uint)pIVar9->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar11 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_007781b6:
            puVar5[(uint)uVar3 + uVar11 * 0x14 + 1] = 0x428;
            puVar5[uVar11 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar11 * 0x14 + 1] != 0x428) {
            uVar10 = 0;
            do {
              uVar2 = uVar10 + 1;
              uVar10 = uVar2 & 0xffff;
              if ((ushort)uVar2 == uVar3) {
                if (uVar3 != 0x13) goto LAB_007781b6;
                goto LAB_00778148;
              }
            } while (puVar5[uVar11 * 0x14 + uVar10 + 1] != 0x428);
          }
        }
        else if (puVar5[0x7d1] == 100) {
LAB_00778148:
          pIVar9->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar9->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(*piVar4 + 0xfa2) = *(short *)(*piVar4 + 0xfa2) + 1;
          pIVar9->m_iChangeInfoSerialNumber = *(ushort *)*piVar4;
          iVar7 = *piVar4 + (uint)pIVar9->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar7 + 2) = 0x428;
          *(undefined2 *)(iVar7 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flRadius).m_Value = 55.0;
  }
  return;
}


/* CTrigger_TractorBeam::CTrigger_TractorBeam at 00778200 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void CTrigger_TractorBeam(CTrigger_TractorBeam * this,
   CTrigger_TractorBeam * this) */

void __thiscall
CTrigger_TractorBeam::CTrigger_TractorBeam(CTrigger_TractorBeam *this,CTrigger_TractorBeam *this_1)

{
  CTrigger_TractorBeam(this);
  return;
}


/* CEntityFactory<CTrigger_TractorBeam>::Create at 00778cc0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CTrigger_TractorBeam> *
   this, char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CTrigger_TractorBeam>::Create
          (CEntityFactory<CTrigger_TractorBeam> *this,char *pClassName)

{
  CTrigger_TractorBeam *this_00;
  
                    /* Unresolved local var: CTrigger_TractorBeam * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x438);
  CTrigger_TractorBeam::CTrigger_TractorBeam(this_00);
  (*(this_00->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.super_IServerUnknown
    .super_IHandleEntity._vptr_IHandleEntity[0x1d])(this_00,pClassName);
  return &(this_00->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.super_IServerNetworkable
  ;
}


/* CTrigger_TractorBeam::~CTrigger_TractorBeam at 00778210 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ~CTrigger_TractorBeam(CTrigger_TractorBeam * this, int __in_chrg)
    */

void __thiscall
CTrigger_TractorBeam::~CTrigger_TractorBeam(CTrigger_TractorBeam *this,int __in_chrg)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int unaff_EBX;
  int *in_stack_ffffffb8;
  int local_30;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5629aa);
  (this->super_IMotionEvent)._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x562cd6);
  local_30 = *(int *)(*(int *)(unaff_EBX + 0x4cf4ce) + 0xc);
  if (0 < local_30) {
    piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x4cf4ce);
    if (this == (CTrigger_TractorBeam *)*piVar1) {
      iVar4 = 0;
      iVar3 = 0;
    }
    else {
      iVar4 = 0;
      iVar2 = 4;
      do {
        iVar3 = iVar2;
        iVar4 = iVar4 + 1;
        if (iVar4 == local_30) goto LAB_00778283;
        iVar2 = iVar3 + 4;
      } while (this != (CTrigger_TractorBeam *)piVar1[iVar4]);
    }
    iVar2 = (local_30 - iVar4) + -1;
    if (0 < iVar2) {
      in_stack_ffffffb8 = piVar1 + iVar4 + 1;
      _V_memmove((void *)(iVar3 + (int)piVar1),in_stack_ffffffb8,iVar2 * 4);
      local_30 = *(int *)(*(int *)(unaff_EBX + 0x4cf4ce) + 0xc);
    }
    *(int *)(*(int *)(unaff_EBX + 0x4cf4ce) + 0xc) = local_30 + -1;
  }
LAB_00778283:
  (this->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4cf4ca) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,(int)in_stack_ffffffb8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CTrigger_TractorBeam::~CTrigger_TractorBeam at 00778310 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ~CTrigger_TractorBeam(CTrigger_TractorBeam * this, int __in_chrg)
    */

void __thiscall
CTrigger_TractorBeam::~CTrigger_TractorBeam(CTrigger_TractorBeam *this,int __in_chrg)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int unaff_EBX;
  int local_30;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5628aa);
  (this->super_IMotionEvent)._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x562bd6);
  local_30 = *(int *)(*(int *)(unaff_EBX + 0x4cf3ce) + 0xc);
  if (0 < local_30) {
    piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x4cf3ce);
    if (this == (CTrigger_TractorBeam *)*piVar1) {
      iVar4 = 0;
      iVar3 = 0;
    }
    else {
      iVar4 = 0;
      iVar2 = 4;
      do {
        iVar3 = iVar2;
        iVar4 = iVar4 + 1;
        if (iVar4 == local_30) goto LAB_00778383;
        iVar2 = iVar3 + 4;
      } while (this != (CTrigger_TractorBeam *)piVar1[iVar4]);
    }
    iVar2 = (local_30 - iVar4) + -1;
    if (0 < iVar2) {
      _V_memmove((void *)((int)piVar1 + iVar3),piVar1 + iVar4 + 1,iVar2 * 4);
      local_30 = *(int *)(*(int *)(unaff_EBX + 0x4cf3ce) + 0xc);
    }
    *(int *)(*(int *)(unaff_EBX + 0x4cf3ce) + 0xc) = local_30 + -1;
  }
LAB_00778383:
  (this->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4cf3ca) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,__in_chrg);
  return;
}


/* CTrigger_TractorBeam::~CTrigger_TractorBeam at 00778410 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ~CTrigger_TractorBeam(CTrigger_TractorBeam * this, int __in_chrg)
    */

void __thiscall
CTrigger_TractorBeam::~CTrigger_TractorBeam(CTrigger_TractorBeam *this,int __in_chrg)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int unaff_EBX;
  int local_30;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5627aa);
  (this->super_IMotionEvent)._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x562ad6);
  local_30 = *(int *)(*(int *)(unaff_EBX + 0x4cf2ce) + 0xc);
  if (0 < local_30) {
    piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x4cf2ce);
    if (this == (CTrigger_TractorBeam *)*piVar1) {
      iVar4 = 0;
      iVar3 = 0;
    }
    else {
      iVar4 = 0;
      iVar2 = 4;
      do {
        iVar3 = iVar2;
        iVar4 = iVar4 + 1;
        if (iVar4 == local_30) goto LAB_00778483;
        iVar2 = iVar3 + 4;
      } while (this != (CTrigger_TractorBeam *)piVar1[iVar4]);
    }
    iVar2 = (local_30 - iVar4) + -1;
    if (0 < iVar2) {
      _V_memmove((void *)((int)piVar1 + iVar3),piVar1 + iVar4 + 1,iVar2 * 4);
      local_30 = *(int *)(*(int *)(unaff_EBX + 0x4cf2ce) + 0xc);
    }
    *(int *)(*(int *)(unaff_EBX + 0x4cf2ce) + 0xc) = local_30 + -1;
  }
LAB_00778483:
  (this->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4cf2ca) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,__in_chrg);
  return;
}


/* CTrigger_TractorBeam::Spawn at 00776bf0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "MoveCollide_t": Some values do not have unique names */
/* WARNING: Enum "MoveType_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(CTrigger_TractorBeam * this) */

void __thiscall CTrigger_TractorBeam::Spawn(CTrigger_TractorBeam *this)

{
  undefined1 *puVar1;
  uint uVar2;
  int iVar3;
  ushort uVar4;
  CBaseEdict *this_00;
  int *piVar5;
  ushort *puVar6;
  BASEPTR func;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  int unaff_EBX;
  NetworkVar_m_Collision *this_01;
  uint uVar9;
  
  ___i686_get_pc_thunk_bx();
  (*(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1a])(this);
  this_01 = &(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Collision;
  CCollisionProperty::SetSolid(&this_01->super_CCollisionProperty,SOLID_VPHYSICS);
  CCollisionProperty::SetSolidFlags
            (&this_01->super_CCollisionProperty,
             (this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Collision.
             super_CCollisionProperty.m_usSolidFlags.m_Value | 0xc);
  CBaseEntity::SetMoveType((CBaseEntity *)this,MOVETYPE_NONE,MOVECOLLIDE_DEFAULT);
  (*(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x9f])(this);
  CBaseEntity::SetTransmitState((CBaseEntity *)this,0x20);
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0x222);
  CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,*(float *)(**(int **)(unaff_EBX + 0x4cff66) + 0xc),(char *)0x0);
  if ((this->m_bDisablePlayerMove).m_Value != false) {
    if ((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
        false) {
      this_00 = &((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_pPev)->
                 super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(this_00);
        piVar5 = *(int **)(&DAT_004cff6a + unaff_EBX);
        puVar6 = (ushort *)*piVar5;
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar6) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar9 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar4 = puVar6[uVar9 * 0x14 + 0x14];
          if (uVar4 == 0) {
LAB_00776df6:
            puVar6[(uint)uVar4 + uVar9 * 0x14 + 1] = 0x42f;
            puVar6[uVar9 * 0x14 + 0x14] = uVar4 + 1;
          }
          else if (puVar6[uVar9 * 0x14 + 1] != 0x42f) {
            uVar8 = 0;
            do {
              uVar2 = uVar8 + 1;
              uVar8 = uVar2 & 0xffff;
              if ((ushort)uVar2 == uVar4) {
                if (uVar4 != 0x13) goto LAB_00776df6;
                goto LAB_00776d79;
              }
            } while (puVar6[uVar9 * 0x14 + uVar8 + 1] != 0x42f);
          }
        }
        else if (puVar6[0x7d1] == 100) {
LAB_00776d79:
          pIVar7->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar6[0x7d1];
          *(short *)(*piVar5 + 0xfa2) = *(short *)(*piVar5 + 0xfa2) + 1;
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar3 = *piVar5 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar3 + 2) = 0x42f;
          *(undefined2 *)(iVar3 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bDisablePlayerMove).m_Value = false;
  }
  return;
}


/* CTrigger_TractorBeam::Precache at 007767c0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void Precache(CTrigger_TractorBeam * this) */

void __thiscall CTrigger_TractorBeam::Precache(CTrigger_TractorBeam *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  PrecacheParticleSystem((char *)(unaff_EBX + 0x33fb89));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x33fb9a));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x33fbaf));
  return;
}


/* CTrigger_TractorBeam::Activate at 007779c0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void Activate(CTrigger_TractorBeam * this) */

void __thiscall CTrigger_TractorBeam::Activate(CTrigger_TractorBeam *this)

{
  edict_t *peVar1;
  CSoundEnvelopeController *pCVar2;
  CSoundPatch *__in_chrg;
  int iVar3;
  int unaff_EBX;
  CRecipientFilter local_3c;
  
                    /* Unresolved local var: CSoundEnvelopeController * controller@[???]
                       Unresolved local var: CPASFilter filter@[???] */
  ___i686_get_pc_thunk_bx();
  CBaseVPhysicsTrigger::Activate(&this->super_CBaseVPhysicsTrigger);
  pCVar2 = CSoundEnvelopeController::GetController();
  if (((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
  CRecipientFilter::CRecipientFilter(&local_3c);
  local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(unaff_EBX + 0x4d179a);
  CRecipientFilter::AddRecipientsByPAS
            (&local_3c,&(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_vecAbsOrigin);
  peVar1 = (this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_pPev;
                    /* Unresolved local var: int edictIndex@[???] */
  iVar3 = 0;
  if (peVar1 != (edict_t *)0x0) {
    iVar3 = (int)peVar1 - *(int *)(**(int **)(unaff_EBX + 0x4cf196) + 0x58) >> 4;
  }
  __in_chrg = (CSoundPatch *)
              (*pCVar2->_vptr_CSoundEnvelopeController[6])
                        (pCVar2,&local_3c,iVar3,unaff_EBX + 0x33e9ad);
  this->m_sndAmbient = __in_chrg;
  (*pCVar2->_vptr_CSoundEnvelopeController[2])(pCVar2,__in_chrg,0x3f800000,0x42c80000,0);
  local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(unaff_EBX + 0x4d179a);
  CRecipientFilter::~CRecipientFilter(&local_3c,(int)__in_chrg);
  return;
}


/* CTrigger_TractorBeam::TractorThink at 00776e20 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void TractorThink(CTrigger_TractorBeam * this) */

void __thiscall CTrigger_TractorBeam::TractorThink(CTrigger_TractorBeam *this)

{
  undefined1 *puVar1;
  int iVar2;
  ushort uVar3;
  CBaseEdict *pCVar4;
  ushort *puVar5;
  int iVar6;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  uint uVar9;
  int *piVar10;
  int unaff_EBX;
  uint local_74;
  uint local_70;
  
  ___i686_get_pc_thunk_bx();
  uVar8 = (this->m_hProxyEntity).
          super_CNetworkVarBase<CBaseHandle,CTrigger_TractorBeam::NetworkVar_m_hProxyEntity>.m_Value
          .m_Index;
  if (((uVar8 != 0xffffffff) &&
      (iVar6 = (uVar8 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4cfd35),
      *(uint *)(iVar6 + 8) == uVar8 >> 0x10)) &&
     (piVar10 = *(int **)(iVar6 + 4), piVar10 != (int *)0x0)) {
    iVar6 = (**(code **)(*piVar10 + 0x324))(piVar10);
    if ((iVar6 != 0) != (this->m_bFromPortal).m_Value) {
      if ((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered
          == false) {
        pCVar4 = &((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
          pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
          puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4cfd31);
          if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar3 = puVar5[uVar8 * 0x14 + 0x14];
            if (uVar3 == 0) {
LAB_007771a0:
              puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x42d;
              puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
            }
            else if (puVar5[uVar8 * 0x14 + 1] != 0x42d) {
              local_74 = 0;
              do {
                uVar9 = local_74 + 1;
                local_74 = uVar9 & 0xffff;
                if ((ushort)uVar9 == uVar3) {
                  if (uVar3 == 0x13) goto LAB_007771c5;
                  goto LAB_007771a0;
                }
              } while (puVar5[uVar8 * 0x14 + local_74 + 1] != 0x42d);
            }
          }
          else if (puVar5[0x7d1] == 100) {
LAB_007771c5:
            pIVar7->m_iChangeInfoSerialNumber = 0;
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar7->m_iChangeInfo = puVar5[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x4cfd31) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x4cfd31) + 0xfa2) + 1;
            piVar10 = *(int **)(unaff_EBX + 0x4cfd31);
            pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar10;
            iVar2 = *piVar10 + (uint)pIVar7->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar2 + 2) = 0x42d;
            *(undefined2 *)(iVar2 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->m_bFromPortal).m_Value = iVar6 != 0;
    }
    uVar8 = (this->m_hProxyEntity).
            super_CNetworkVarBase<CBaseHandle,CTrigger_TractorBeam::NetworkVar_m_hProxyEntity>.
            m_Value.m_Index;
    if ((uVar8 == 0xffffffff) ||
       (iVar6 = (uVar8 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4cfd35),
       *(uint *)(iVar6 + 8) != uVar8 >> 0x10)) {
      piVar10 = (int *)0x0;
    }
    else {
      piVar10 = *(int **)(iVar6 + 4);
    }
    iVar6 = (**(code **)(*piVar10 + 0x31c))(piVar10);
    if ((iVar6 != 0) != (this->m_bToPortal).m_Value) {
      if ((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered
          == false) {
        pCVar4 = &((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
          pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
          puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4cfd31);
          if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
            uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar3 = puVar5[uVar8 * 0x14 + 0x14];
            if (uVar3 == 0) {
LAB_0077710b:
              puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x42e;
              puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
            }
            else if (puVar5[uVar8 * 0x14 + 1] != 0x42e) {
              local_70 = 0;
              do {
                uVar9 = local_70 + 1;
                local_70 = uVar9 & 0xffff;
                if ((ushort)uVar9 == uVar3) {
                  if (uVar3 != 0x13) goto LAB_0077710b;
                  pIVar7->m_iChangeInfoSerialNumber = 0;
                  pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar5[uVar8 * 0x14 + local_70 + 1] != 0x42e);
            }
          }
          else if (puVar5[0x7d1] == 100) {
            pIVar7->m_iChangeInfoSerialNumber = 0;
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar7->m_iChangeInfo = puVar5[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x4cfd31) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x4cfd31) + 0xfa2) + 1;
            piVar10 = *(int **)(unaff_EBX + 0x4cfd31);
            pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar10;
            iVar2 = *piVar10 + (uint)pIVar7->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar2 + 2) = 0x42e;
            *(undefined2 *)(iVar2 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->m_bToPortal).m_Value = iVar6 != 0;
    }
  }
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,*(float *)(**(int **)(unaff_EBX + 0x4cfd2d) + 0xc),(char *)0x0);
  return;
}


/* CTrigger_TractorBeam::CreateVPhysics at 00776450 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: bool CreateVPhysics(CTrigger_TractorBeam * this) */

bool __thiscall CTrigger_TractorBeam::CreateVPhysics(CTrigger_TractorBeam *this)

{
  IPhysicsMotionController *pIVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  pIVar1 = (IPhysicsMotionController *)
           (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d079b) + 0x78))
                     ((int *)**(undefined4 **)(unaff_EBX + 0x4d079b),&this->super_IMotionEvent);
  this->m_pController = pIVar1;
  return true;
}


/* CTrigger_TractorBeam::WakeTouchingObjects at 00776800 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void WakeTouchingObjects(CTrigger_TractorBeam * this) */

void __thiscall CTrigger_TractorBeam::WakeTouchingObjects(CTrigger_TractorBeam *this)

{
  float fVar1;
  IPhysicsObject *pIVar2;
  CPropWeightedCube *this_00;
  bool bVar3;
  int iVar4;
  int unaff_EBX;
  int iVar5;
  CBaseEntity *local_49c [256];
  Ray_t local_9c;
  CFlaggedEntitiesEnum local_48;
  float local_34;
  float local_30;
  float local_2c;
  float local_28;
  float local_24;
  float local_20;
  
                    /* Unresolved local var: CBaseEntity *[256] list@[???]
                       Unresolved local var: CBaseEntity * pEntity@[???]
                       Unresolved local var: Ray_t ray@[???]
                       Unresolved local var: Vector vExtents@[???]
                       Unresolved local var: int nNumFound@[???] */
  ___i686_get_pc_thunk_bx();
  fVar1 = (this->m_flRadius).m_Value;
  local_34 = (float)((uint)fVar1 ^ *(uint *)(unaff_EBX + 0x35f98f));
  local_9c.m_Delta.super_Vector.x = (this->m_vEnd).m_Value.x - (this->m_vStart).m_Value.x;
  local_9c.m_Delta.super_Vector.y = (this->m_vEnd).m_Value.y - (this->m_vStart).m_Value.y;
  local_9c.m_Delta.super_Vector.z = (this->m_vEnd).m_Value.z - (this->m_vStart).m_Value.z;
  local_9c.m_pWorldAxisTransform = (matrix3x4_t *)0x0;
  local_9c.m_IsSwept =
       local_9c.m_Delta.super_Vector.x * local_9c.m_Delta.super_Vector.x +
       local_9c.m_Delta.super_Vector.y * local_9c.m_Delta.super_Vector.y +
       local_9c.m_Delta.super_Vector.z * local_9c.m_Delta.super_Vector.z !=
       *(float *)(unaff_EBX + 0x35635f);
  local_9c.m_StartOffset.super_Vector.z = *(float *)(unaff_EBX + 0x3562ef);
  local_9c.m_Extents.super_Vector.x = (fVar1 - local_34) * local_9c.m_StartOffset.super_Vector.z;
  local_9c.m_Extents.super_Vector.y = (fVar1 - local_34) * local_9c.m_StartOffset.super_Vector.z;
  local_9c.m_Extents.super_Vector.z = (fVar1 - local_34) * local_9c.m_StartOffset.super_Vector.z;
  local_9c.m_IsRay =
       (double)(local_9c.m_Extents.super_Vector.x * local_9c.m_Extents.super_Vector.x +
                local_9c.m_Extents.super_Vector.y * local_9c.m_Extents.super_Vector.y +
               local_9c.m_Extents.super_Vector.z * local_9c.m_Extents.super_Vector.z) <
       *(double *)(unaff_EBX + 0x3564df);
  local_9c.m_StartOffset.super_Vector.x = (fVar1 + local_34) * local_9c.m_StartOffset.super_Vector.z
  ;
  local_9c.m_StartOffset.super_Vector.y = (fVar1 + local_34) * local_9c.m_StartOffset.super_Vector.z
  ;
  local_9c.m_StartOffset.super_Vector.z = (fVar1 + local_34) * local_9c.m_StartOffset.super_Vector.z
  ;
  local_9c.m_Start.super_Vector.x =
       (this->m_vStart).m_Value.x + local_9c.m_StartOffset.super_Vector.x;
  local_9c.m_Start.super_Vector.y =
       (this->m_vStart).m_Value.y + local_9c.m_StartOffset.super_Vector.y;
  local_9c.m_Start.super_Vector.z =
       (this->m_vStart).m_Value.z + local_9c.m_StartOffset.super_Vector.z;
  local_9c.m_StartOffset.super_Vector.x = -local_9c.m_StartOffset.super_Vector.x;
  local_9c.m_StartOffset.super_Vector.y = -local_9c.m_StartOffset.super_Vector.y;
  local_9c.m_StartOffset.super_Vector.z = -local_9c.m_StartOffset.super_Vector.z;
                    /* Unresolved local var: CFlaggedEntitiesEnum rayEnum@[???] */
  local_30 = local_34;
  local_2c = local_34;
  local_28 = local_34;
  local_24 = local_34;
  local_20 = local_34;
  CFlaggedEntitiesEnum::CFlaggedEntitiesEnum(&local_48,local_49c,0x100,0);
  iVar4 = UTIL_EntitiesAlongRay(&local_9c,&local_48);
                    /* Unresolved local var: int i@[???] */
  if (0 < iVar4) {
    iVar5 = 0;
    do {
      this_00 = (CPropWeightedCube *)local_49c[iVar5];
      if (this_00 != (CPropWeightedCube *)0x0) {
        bVar3 = UTIL_IsReflectiveCube((CBaseEntity *)this_00);
        if (bVar3) {
                    /* Unresolved local var: CPropWeightedCube * pReflectiveCube@[???] */
          CPropWeightedCube::ExitDisabledState(this_00);
        }
        pIVar2 = (this_00->super_PlayerPickupPaintPowerUser<CPhysicsProp>).
                 super_PropPaintPowerUser<CPhysicsProp>.
                 super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.
                 super_CPaintableEntity<CPhysicsProp>.super_CPhysicsProp.super_CBreakableProp.
                 super_CBaseProp.super_CBaseAnimating.super_CBaseEntity.m_pPhysicsObject;
        if (pIVar2 != (IPhysicsObject *)0x0) {
          (*pIVar2->_vptr_IPhysicsObject[0x19])(pIVar2);
        }
      }
      iVar5 = iVar5 + 1;
    } while (iVar5 != iVar4);
  }
  return;
}


/* CTrigger_TractorBeam::UpdateOnRemove at 007766e0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void UpdateOnRemove(CTrigger_TractorBeam * this) */

void __thiscall CTrigger_TractorBeam::UpdateOnRemove(CTrigger_TractorBeam *this)

{
  IPhysicsMotionController *pIVar1;
  int *piVar2;
  int iVar3;
  void *pMem;
  int unaff_EBX;
  int iVar4;
  IPhysicsObject **pObjects;
  
  ___i686_get_pc_thunk_bx();
  pIVar1 = this->m_pController;
  if (pIVar1 != (IPhysicsMotionController *)0x0) {
                    /* Unresolved local var: int nNumObjects@[???] */
    iVar3 = (*pIVar1->_vptr_IPhysicsMotionController[5])(pIVar1);
    pMem = operator_new___(iVar3 * 4);
    (*this->m_pController->_vptr_IPhysicsMotionController[6])(this->m_pController,pMem);
                    /* Unresolved local var: int i@[???] */
    if (0 < iVar3) {
      iVar4 = 0;
      do {
        piVar2 = *(int **)((int)pMem + iVar4 * 4);
        if (piVar2 != (int *)0x0) {
          (**(code **)(*piVar2 + 100))(piVar2);
        }
        iVar4 = iVar4 + 1;
      } while (iVar4 != iVar3);
    }
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d050e) + 0x7c))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4d050e),this->m_pController);
    this->m_pController = (IPhysicsMotionController *)0x0;
    if (pMem != (void *)0x0) {
      operator_delete___(pMem);
    }
  }
  StopParticleEffects((CBaseEntity *)this);
  (*(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x6f])(this);
  CBaseVPhysicsTrigger::UpdateOnRemove(&this->super_CBaseVPhysicsTrigger);
  return;
}


/* CTrigger_TractorBeam::OnRestore at 00776690 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void OnRestore(CTrigger_TractorBeam * this) */

void __thiscall CTrigger_TractorBeam::OnRestore(CTrigger_TractorBeam *this)

{
  IPhysicsMotionController *pIVar1;
  
  CBaseEntity::OnRestore((CBaseEntity *)this);
  pIVar1 = this->m_pController;
  if (pIVar1 != (IPhysicsMotionController *)0x0) {
    (*pIVar1->_vptr_IPhysicsMotionController[2])(pIVar1,&this->super_IMotionEvent);
  }
  CBaseEntity::SetTransmitState((CBaseEntity *)this,0x20);
  return;
}


/* CTrigger_TractorBeam::StopLoopingSounds at 00776600 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void StopLoopingSounds(CTrigger_TractorBeam * this) */

void __thiscall CTrigger_TractorBeam::StopLoopingSounds(CTrigger_TractorBeam *this)

{
  CSoundEnvelopeController *pCVar1;
  
  if (this->m_sndAmbient != (CSoundPatch *)0x0) {
                    /* Unresolved local var: CSoundEnvelopeController * controller@[???] */
    pCVar1 = CSoundEnvelopeController::GetController();
    (*pCVar1->_vptr_CSoundEnvelopeController[5])(pCVar1,this->m_sndAmbient);
    (*pCVar1->_vptr_CSoundEnvelopeController[0xb])(pCVar1,this->m_sndAmbient);
    this->m_sndAmbient = (CSoundPatch *)0x0;
  }
  if (this->m_sndPlayerInBeam != (CSoundPatch *)0x0) {
                    /* Unresolved local var: CSoundEnvelopeController * controller@[???] */
    pCVar1 = CSoundEnvelopeController::GetController();
    (*pCVar1->_vptr_CSoundEnvelopeController[5])(pCVar1,this->m_sndPlayerInBeam);
    (*pCVar1->_vptr_CSoundEnvelopeController[0xb])(pCVar1,this->m_sndPlayerInBeam);
    this->m_sndPlayerInBeam = (CSoundPatch *)0x0;
  }
  return;
}


/* CTrigger_TractorBeam::StartTouch at 00777af0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void StartTouch(CTrigger_TractorBeam * this, CBaseEntity * pOther) */

void __thiscall CTrigger_TractorBeam::StartTouch(CTrigger_TractorBeam *this,CBaseEntity *pOther)

{
  Vector *origin;
  edict_t *peVar1;
  bool bVar2;
  int iVar3;
  CSoundEnvelopeController *pCVar4;
  CSoundPatch *__in_chrg;
  int unaff_EBX;
  undefined4 local_9c;
  int local_98;
  undefined4 local_94;
  undefined4 local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  undefined4 local_80;
  undefined4 local_7c;
  undefined1 local_78;
  undefined1 local_77;
  undefined1 local_76;
  undefined4 local_74;
  int local_70;
  undefined4 local_6c;
  int local_68;
  undefined4 local_64;
  int local_60;
  undefined2 local_5c;
  undefined4 local_58;
  CRecipientFilter local_54;
  triggerevent_t local_34;
  char *local_20;
  
                    /* Unresolved local var: CBasePlayer * pPlayer@[???]
                       Unresolved local var: triggerevent_t event@[???] */
  ___i686_get_pc_thunk_bx();
  CBaseVPhysicsTrigger::StartTouch(&this->super_CBaseVPhysicsTrigger,pOther);
  iVar3 = (*(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.
            super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xc5])(this,pOther);
  if ((char)iVar3 != '\0') {
    if (pOther != (CBaseEntity *)0x0) {
      local_20 = (char *)(unaff_EBX + 0x2e9603);
      if (((pOther->m_iClassname).pszValue == local_20) ||
         (bVar2 = CBaseEntity::ClassMatchesComplex(pOther,local_20), bVar2)) {
                    /* Unresolved local var: CNPC_Portal_FloorTurret * pTurret@[???] */
        CNPC_Portal_FloorTurret::OnEnteredTractorBeam((CNPC_Portal_FloorTurret *)pOther);
      }
      iVar3 = (*(pOther->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                _vptr_IHandleEntity[0x55])(pOther);
      if ((char)iVar3 != '\0') {
        CBasePlayer::SetPhysicsFlag((CBasePlayer *)pOther,0x10,true);
        if (*(char *)&pOther[2].m_vecOrigin.
                      super_CNetworkVectorCommonBase<Vector,CBaseEntity::NetworkVar_m_vecOrigin>.
                      super_CNetworkVarBase<Vector,CBaseEntity::NetworkVar_m_vecOrigin>.m_Value.x ==
            '\0') {
                    /* Unresolved local var: CPlayerLocalData * pThis@[???] */
          (*(code *)(pOther[2].m_DamageModifiers.m_pElements)->m_Previous)
                    (&pOther[2].m_DamageModifiers.m_pElements,&pOther[2].m_vecOrigin);
          *(undefined1 *)
           &pOther[2].m_vecOrigin.
            super_CNetworkVectorCommonBase<Vector,CBaseEntity::NetworkVar_m_vecOrigin>.
            super_CNetworkVarBase<Vector,CBaseEntity::NetworkVar_m_vecOrigin>.m_Value.x = 1;
        }
        pOther->m_flGravity = 1.1754944e-38;
        pCVar4 = CSoundEnvelopeController::GetController();
        if (this->m_sndPlayerInBeam != (CSoundPatch *)0x0) {
          (*pCVar4->_vptr_CSoundEnvelopeController[5])(pCVar4,this->m_sndPlayerInBeam);
          (*pCVar4->_vptr_CSoundEnvelopeController[0xb])(pCVar4,this->m_sndPlayerInBeam);
          this->m_sndPlayerInBeam = (CSoundPatch *)0x0;
        }
        local_8c = 0;
        local_88 = 100;
        local_84 = 0;
        local_80 = 0;
        local_7c = 0;
        local_78 = 1;
        local_77 = 0;
        local_76 = 0;
        local_74 = 0xffffffff;
        local_70 = 0;
        local_6c = 0;
        local_68 = 0;
        local_64 = 0;
        local_60 = 0;
        local_5c = 0xffff;
        local_58 = 1;
        local_9c = 6;
        local_98 = unaff_EBX + 0x33e865;
        local_94 = 0x3f800000;
        local_90 = 0x4b;
        if (((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
          CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
        }
        origin = &(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_vecAbsOrigin;
        CRecipientFilter::CRecipientFilter(&local_54);
        local_54.super_IRecipientFilter._vptr_IRecipientFilter =
             (_func_int_varargs **)(unaff_EBX + 0x4d1667);
        CRecipientFilter::AddRecipientsByPAS(&local_54,origin);
        local_54.super_IRecipientFilter._vptr_IRecipientFilter =
             (_func_int_varargs **)(unaff_EBX + 0x4d1627);
        CPASAttenuationFilter::Filter((CPASAttenuationFilter *)&local_54,origin,0.8);
        CRecipientFilter::MakeReliable(&local_54);
        CRecipientFilter::RemoveAllRecipients(&local_54);
        CRecipientFilter::AddRecipient(&local_54,(CBasePlayer *)pOther);
        peVar1 = (this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_pPev;
                    /* Unresolved local var: int edictIndex@[???] */
        iVar3 = 0;
        if (peVar1 != (edict_t *)0x0) {
          iVar3 = (int)peVar1 - *(int *)(**(int **)(unaff_EBX + 0x4cf063) + 0x58) >> 4;
        }
        __in_chrg = (CSoundPatch *)
                    (*pCVar4->_vptr_CSoundEnvelopeController[10])(pCVar4,&local_54,iVar3,&local_9c);
        this->m_sndPlayerInBeam = __in_chrg;
        (*pCVar4->_vptr_CSoundEnvelopeController[2])(pCVar4,__in_chrg,0x3f800000,0x42c80000,0);
        local_54.super_IRecipientFilter._vptr_IRecipientFilter =
             (_func_int_varargs **)(unaff_EBX + 0x4d1667);
        CRecipientFilter::~CRecipientFilter(&local_54,(int)__in_chrg);
        local_64 = 0;
        if (-1 < local_68) {
          if (local_70 != 0) {
            (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4cf05b) + 8))
                      ((int *)**(undefined4 **)(unaff_EBX + 0x4cf05b),local_70);
            local_70 = 0;
          }
          local_6c = 0;
        }
        local_60 = local_70;
        if (-1 < local_68) {
          if (local_70 != 0) {
            (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4cf05b) + 8))
                      ((int *)**(undefined4 **)(unaff_EBX + 0x4cf05b),local_70);
            local_70 = 0;
          }
          local_6c = 0;
        }
      }
    }
    if (((this->m_pController != (IPhysicsMotionController *)0x0) &&
        (bVar2 = PhysGetTriggerEvent(&local_34,(CBaseEntity *)this), bVar2)) &&
       (local_34.pObject != (IPhysicsObject *)0x0)) {
      (*(local_34.pObject)->_vptr_IPhysicsObject[0x19])(local_34.pObject);
      (*this->m_pController->_vptr_IPhysicsMotionController[3])
                (this->m_pController,local_34.pObject,1);
    }
  }
  return;
}


/* CTrigger_TractorBeam::EndTouch at 00776a40 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void EndTouch(CTrigger_TractorBeam * this, CBaseEntity * pOther) */

void __thiscall CTrigger_TractorBeam::EndTouch(CTrigger_TractorBeam *this,CBaseEntity *pOther)

{
  bool bVar1;
  int iVar2;
  CSoundEnvelopeController *pCVar3;
  int unaff_EBX;
  triggerevent_t local_34;
  char *local_20;
  
                    /* Unresolved local var: CBasePlayer * pPlayer@[???]
                       Unresolved local var: triggerevent_t event@[???] */
  ___i686_get_pc_thunk_bx();
  CBaseVPhysicsTrigger::EndTouch(&this->super_CBaseVPhysicsTrigger,pOther);
  iVar2 = (*(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.
            super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xc5])(this,pOther);
  if ((char)iVar2 != '\0') {
    if (pOther != (CBaseEntity *)0x0) {
      local_20 = (char *)(unaff_EBX + 0x2ea6b0);
      if (((pOther->m_iClassname).pszValue == local_20) ||
         (bVar1 = CBaseEntity::ClassMatchesComplex(pOther,local_20), bVar1)) {
                    /* Unresolved local var: CNPC_Portal_FloorTurret * pTurret@[???] */
        CNPC_Portal_FloorTurret::OnExitedTractorBeam((CNPC_Portal_FloorTurret *)pOther);
      }
      iVar2 = (*(pOther->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                _vptr_IHandleEntity[0x55])(pOther);
      if ((char)iVar2 != '\0') {
        CBasePlayer::SetPhysicsFlag((CBasePlayer *)pOther,0x10,false);
        if (*(char *)&pOther[2].m_vecOrigin.
                      super_CNetworkVectorCommonBase<Vector,CBaseEntity::NetworkVar_m_vecOrigin>.
                      super_CNetworkVarBase<Vector,CBaseEntity::NetworkVar_m_vecOrigin>.m_Value.x !=
            '\0') {
                    /* Unresolved local var: CPlayerLocalData * pThis@[???] */
          (*(code *)(pOther[2].m_DamageModifiers.m_pElements)->m_Previous)
                    (&pOther[2].m_DamageModifiers.m_pElements,&pOther[2].m_vecOrigin);
          *(undefined1 *)
           &pOther[2].m_vecOrigin.
            super_CNetworkVectorCommonBase<Vector,CBaseEntity::NetworkVar_m_vecOrigin>.
            super_CNetworkVarBase<Vector,CBaseEntity::NetworkVar_m_vecOrigin>.m_Value.x = 0;
        }
        pOther->m_flGravity = 1.0;
        if (this->m_sndPlayerInBeam != (CSoundPatch *)0x0) {
          pCVar3 = CSoundEnvelopeController::GetController();
          (*pCVar3->_vptr_CSoundEnvelopeController[0xe])
                    (pCVar3,this->m_sndPlayerInBeam,0x3f000000,0);
        }
      }
    }
    bVar1 = PhysGetTriggerEvent(&local_34,(CBaseEntity *)this);
    if (((bVar1) && (local_34.pObject != (IPhysicsObject *)0x0)) &&
       (this->m_pController != (IPhysicsMotionController *)0x0)) {
      (*(local_34.pObject)->_vptr_IPhysicsObject[0x19])(local_34.pObject);
      (*this->m_pController->_vptr_IPhysicsMotionController[4])
                (this->m_pController,local_34.pObject);
    }
    return;
  }
  return;
}


/* CTrigger_TractorBeam::InputSetVelocityLimitTime at 00777210 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void InputSetVelocityLimitTime(CTrigger_TractorBeam * this, inputdata_t
   * inputdata) */

void __thiscall
CTrigger_TractorBeam::InputSetVelocityLimitTime(CTrigger_TractorBeam *this,inputdata_t *inputdata)

{
  undefined1 *puVar1;
  int iVar2;
  ushort uVar3;
  CBaseEdict *pCVar4;
  ushort *puVar5;
  int *piVar6;
  char *pString;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  uint uVar9;
  int unaff_EBX;
  float fVar10;
  uint local_bc;
  uint local_b8;
  uint local_b4;
  uint local_b0;
  CNetworkVarBase<float,CTrigger_TractorBeam::NetworkVar_m_linearLimit> local_24;
  float local_20;
  
                    /* Unresolved local var: float[2] args@[???] */
  ___i686_get_pc_thunk_bx();
  fVar10 = LinearLimit(this);
  if (fVar10 != (this->m_linearLimitStart).m_Value) {
    if ((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
        false) {
      pCVar4 = &((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_pPev)->
                super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4cf947);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_007776ba:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 1000;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 1000) {
            local_bc = 0;
            do {
              uVar9 = local_bc + 1;
              local_bc = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 != 0x13) goto LAB_007776ba;
                pIVar7->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar8 * 0x14 + local_bc + 1] != 1000);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4cf947) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4cf947) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x4cf947);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 1000;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_linearLimitStart).m_Value = fVar10;
  }
  fVar10 = *(float *)(**(int **)(unaff_EBX + 0x4cf943) + 0xc);
  if (fVar10 != (this->m_linearLimitStartTime).m_Value) {
    if ((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
        false) {
      pCVar4 = &((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_pPev)->
                super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4cf947);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_0077783a:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x3ec;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 0x3ec) {
            local_b8 = 0;
            do {
              uVar9 = local_b8 + 1;
              local_b8 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 != 0x13) goto LAB_0077783a;
                pIVar7->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar8 * 0x14 + local_b8 + 1] != 0x3ec);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4cf947) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4cf947) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x4cf947);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3ec;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_linearLimitStartTime).m_Value = fVar10;
  }
  if ((inputdata->value).fieldType == FIELD_STRING) {
    pString = (char *)(inputdata->value).field_0.iVal;
    if (pString == (char *)0x0) {
      pString = (char *)(unaff_EBX + 0x2d7b03);
    }
  }
  else {
    pString = variant_t::ToString(&inputdata->value);
  }
  UTIL_StringToFloatArray(&local_24.m_Value,2,pString);
  if (local_24.m_Value != (this->m_linearLimit).m_Value) {
    if ((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
        false) {
      pCVar4 = &((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_pPev)->
                super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4cf947);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00777777:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x3dc;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 0x3dc) {
            local_b4 = 0;
            do {
              uVar9 = local_b4 + 1;
              local_b4 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 != 0x13) goto LAB_00777777;
                pIVar7->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar8 * 0x14 + local_b4 + 1] != 0x3dc);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4cf947) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4cf947) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x4cf947);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3dc;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_linearLimit).m_Value = local_24.m_Value;
  }
  if (local_20 != (this->m_linearLimitTime).m_Value) {
    if ((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
        false) {
      pCVar4 = &((this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.m_pPev)->
                super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4cf947);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00777903:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x3e4;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 0x3e4) {
            local_b0 = 0;
            do {
              uVar9 = local_b0 + 1;
              local_b0 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 != 0x13) goto LAB_00777903;
                pIVar7->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar8 * 0x14 + local_b0 + 1] != 0x3e4);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4cf947) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4cf947) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x4cf947);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3e4;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_linearLimitTime).m_Value = local_20;
  }
  return;
}


/* CTrigger_TractorBeam::CreateTractorBeam at 00776590 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

CTrigger_TractorBeam *
CTrigger_TractorBeam::CreateTractorBeam(Vector *vStart,Vector *vEnd,CBaseEntity *pOwner)

{
  CTrigger_TractorBeam *this;
  int unaff_EBX;
  CTrigger_TractorBeam *pCVar1;
  
                    /* Unresolved local var: CTrigger_TractorBeam * pBeam@[???] */
  ___i686_get_pc_thunk_bx();
  this = (CTrigger_TractorBeam *)CreateEntityByName((char *)(unaff_EBX + 0x2e49c8),-1,true);
  pCVar1 = (CTrigger_TractorBeam *)0x0;
  if (this != (CTrigger_TractorBeam *)0x0) {
    (*(this->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
      super_IHandleEntity._vptr_IHandleEntity[0x13])(this,pOwner);
    UpdateBeam(this);
    DispatchSpawn((CBaseEntity *)this,true);
    pCVar1 = this;
  }
  return pCVar1;
}


/* CProjectedTractorBeamEntity::Spawn at 00778a30 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(CProjectedTractorBeamEntity * this) */

void __thiscall CProjectedTractorBeamEntity::Spawn(CProjectedTractorBeamEntity *this)

{
  undefined1 *puVar1;
  ushort uVar2;
  CBaseEdict *this_00;
  ushort *puVar3;
  int *piVar4;
  int iVar5;
  CTrigger_TractorBeam *pCVar6;
  ulong *puVar7;
  IChangeInfoAccessor *pIVar8;
  uint uVar9;
  uint uVar10;
  int unaff_EBX;
  uint local_60;
  CTrigger_TractorBeam *local_20;
  
  ___i686_get_pc_thunk_bx();
  CBaseProjectedEntity::Spawn(&this->super_CBaseProjectedEntity);
  local_20 = (CTrigger_TractorBeam *)
             CreateEntityByName((char *)(CAI_BaseNPC::SoundIsVisible + unaff_EBX + 6),-1,true);
  if (local_20 == (CTrigger_TractorBeam *)0x0) {
    local_20 = (CTrigger_TractorBeam *)0x0;
  }
  else {
    (*(local_20->super_CBaseVPhysicsTrigger).super_CBaseEntity.super_IServerEntity.
      super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x13])(local_20,this);
    CTrigger_TractorBeam::UpdateBeam(local_20);
    DispatchSpawn((CBaseEntity *)local_20,true);
  }
  uVar9 = (this->m_hTractorBeamTrigger).
          super_CNetworkVarBase<CBaseHandle,CProjectedTractorBeamEntity::NetworkVar_m_hTractorBeamTrigger>
          .m_Value.m_Index;
  if ((uVar9 == 0xffffffff) ||
     (iVar5 = (uVar9 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4ce12e),
     *(uint *)(iVar5 + 8) != uVar9 >> 0x10)) {
    pCVar6 = (CTrigger_TractorBeam *)0x0;
  }
  else {
    pCVar6 = *(CTrigger_TractorBeam **)(iVar5 + 4);
  }
  if (pCVar6 != local_20) {
    if ((this->super_CBaseProjectedEntity).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
        false) {
      this_00 = &((this->super_CBaseProjectedEntity).super_CBaseEntity.m_Network.m_pPev)->
                 super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar8 = CBaseEdict::GetChangeAccessor(this_00);
        puVar3 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4ce12a);
        if (pIVar8->m_iChangeInfoSerialNumber == *puVar3) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar9 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar3[uVar9 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_00778c84:
            puVar3[(uint)uVar2 + uVar9 * 0x14 + 1] = 0x420;
            puVar3[uVar9 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar3[uVar9 * 0x14 + 1] != 0x420) {
            local_60 = 0;
            do {
              uVar10 = local_60 + 1;
              local_60 = uVar10 & 0xffff;
              if ((ushort)uVar10 == uVar2) {
                if (uVar2 != 0x13) goto LAB_00778c84;
                pIVar8->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar3[uVar9 * 0x14 + local_60 + 1] != 0x420);
          }
        }
        else if (puVar3[0x7d1] == 100) {
          pIVar8->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar8->m_iChangeInfo = puVar3[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4ce12a) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4ce12a) + 0xfa2) + 1;
          piVar4 = *(int **)(unaff_EBX + 0x4ce12a);
          pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar4;
          iVar5 = *piVar4 + (uint)pIVar8->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar5 + 2) = 0x420;
          *(undefined2 *)(iVar5 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseProjectedEntity).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    if (local_20 == (CTrigger_TractorBeam *)0x0) {
      (this->m_hTractorBeamTrigger).
      super_CNetworkVarBase<CBaseHandle,CProjectedTractorBeamEntity::NetworkVar_m_hTractorBeamTrigger>
      .m_Value.m_Index = 0xffffffff;
    }
    else {
      puVar7 = (ulong *)(*(local_20->super_CBaseVPhysicsTrigger).super_CBaseEntity.
                          super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
                          _vptr_IHandleEntity[3])(local_20);
      (this->m_hTractorBeamTrigger).
      super_CNetworkVarBase<CBaseHandle,CProjectedTractorBeamEntity::NetworkVar_m_hTractorBeamTrigger>
      .m_Value.m_Index = *puVar7;
    }
  }
  CBaseEntity::SetTransmitState((CBaseEntity *)this,8);
  return;
}


/* CProjectedTractorBeamEntity::CreateNewInstance at 00776550 */

CProjectedTractorBeamEntity * CProjectedTractorBeamEntity::CreateNewInstance(void)

{
  CProjectedTractorBeamEntity *pCVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  pCVar1 = (CProjectedTractorBeamEntity *)CreateEntityByName(&UNK_002e49ea + unaff_EBX,-1,true);
  return pCVar1;
}


/* CProjectedTractorBeamEntity::CreateNewProjectedEntity at 00776bb0 */

/* DWARF original prototype: CBaseProjectedEntity *
   CreateNewProjectedEntity(CProjectedTractorBeamEntity * this) */

CBaseProjectedEntity * __thiscall
CProjectedTractorBeamEntity::CreateNewProjectedEntity(CProjectedTractorBeamEntity *this)

{
  CBaseProjectedEntity *pCVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  pCVar1 = (CBaseProjectedEntity *)CreateEntityByName((char *)(unaff_EBX + 0x2e438a),-1,true);
  return pCVar1;
}


/* CProjectedTractorBeamEntity::OnProjected at 007764a0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void OnProjected(CProjectedTractorBeamEntity * this) */

void __thiscall CProjectedTractorBeamEntity::OnProjected(CProjectedTractorBeamEntity *this)

{
  uint uVar1;
  CTrigger_TractorBeam *this_00;
  int iVar2;
  CBasePlayer *pOwner;
  int unaff_EBX;
  
                    /* Unresolved local var: CTrigger_TractorBeam * pBeam@[???] */
  ___i686_get_pc_thunk_bx();
  CBaseProjectedEntity::OnProjected(&this->super_CBaseProjectedEntity);
  uVar1 = (this->m_hTractorBeamTrigger).
          super_CNetworkVarBase<CBaseHandle,CProjectedTractorBeamEntity::NetworkVar_m_hTractorBeamTrigger>
          .m_Value.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d06b8),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (this_00 = *(CTrigger_TractorBeam **)(iVar2 + 4), this_00 != (CTrigger_TractorBeam *)0x0)) {
    if ((this->super_CBaseProjectedEntity).super_CBaseEntity.m_bIsPlayerSimulated.m_Value == false)
    {
      CBaseEntity::UnsetPlayerSimulated((CBaseEntity *)this_00);
    }
    else {
      uVar1 = (this->super_CBaseProjectedEntity).super_CBaseEntity.m_hPlayerSimulationOwner.
              super_CNetworkVarBase<CBaseHandle,CBaseEntity::NetworkVar_m_hPlayerSimulationOwner>.
              m_Value.m_Index;
      if ((uVar1 == 0xffffffff) ||
         (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d06b8),
         *(uint *)(iVar2 + 8) != uVar1 >> 0x10)) {
        pOwner = (CBasePlayer *)0x0;
      }
      else {
        pOwner = *(CBasePlayer **)(iVar2 + 4);
      }
      CBaseEntity::SetPlayerSimulated((CBaseEntity *)this_00,pOwner);
    }
    CTrigger_TractorBeam::UpdateBeam(this_00);
    return;
  }
  return;
}


/* _GLOBAL__I__ZN27CProjectedTractorBeamEntity9m_DataMapE at 000bbac0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN27CProjectedTractorBeamEntity9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

