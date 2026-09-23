/* DWARF-guided pseudocode for game/server/portal2/projectedwallentity.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* DataMapInit<CProjectedWallEntity> at 000b27c0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

datamap_t * DataMapInit<CProjectedWallEntity>(CProjectedWallEntity *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&tf_escort_score_rate.m_pParent + unaff_EBX + 3) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xd9d3bf);
    if (iVar1 != 0) {
      *(int *)((int)&tf_escort_score_rate.m_Value.m_pszString + unaff_EBX + 3) =
           unaff_EBX + 0xa00be6;
      *(undefined4 *)((int)&tf_escort_score_rate.m_Value.m_fValue + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&tf_escort_score_rate.m_Value.m_nValue + unaff_EBX + 3) = 0;
      *(undefined4 *)(&tf_escort_score_rate.field_0x37 + unaff_EBX) = 0;
      *(undefined4 *)((int)&tf_escort_score_rate.m_fMinVal + unaff_EBX + 3) = 0;
      *(undefined4 *)(&tf_escort_score_rate.field_0x3f + unaff_EBX) = 0;
      *(undefined4 *)((int)&tf_escort_score_rate.m_Value.m_StringLength + unaff_EBX + 3) = 0x14;
      ___cxa_guard_release(unaff_EBX + 0xd9d3bf);
      ___cxa_atexit(unaff_EBX + 0x98c5cf,0,*(undefined4 *)(&DAT_00b94383 + unaff_EBX));
    }
  }
  *(undefined4 *)(&DAT_00cb4ffb + unaff_EBX) = *(undefined4 *)(&DAT_00b94eeb + unaff_EBX);
  *(undefined4 *)((int)&PTR_SetLastUserCommand_00cb4ff0 + unaff_EBX + 3) = 8;
  *(int *)((int)&PTR_GetMaxHealth_00cb4fec + unaff_EBX + 3) = unaff_EBX + 0xcb504f;
  return (datamap_t *)((int)&PTR_GetMaxHealth_00cb4fec + unaff_EBX + 3);
}


/* __static_initialization_and_destruction_0 at 000b2890 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  undefined *puVar1;
  char *pcVar2;
  SendTable *pSVar3;
  IEntityFactoryDictionary *pIVar4;
  datamap_t *pdVar5;
  int iVar6;
  int iVar7;
  int unaff_EBX;
  undefined4 *puVar8;
  undefined4 *puVar9;
  undefined4 *puVar10;
  longlong lVar11;
  
  lVar11 = ___i686_get_pc_thunk_bx();
  if (lVar11 == 0xffff00000001) {
    *(undefined1 *)((int)&phys_upimpactforcescale.m_pParent + unaff_EBX) = 0;
    *(undefined1 *)((int)&phys_upimpactforcescale.m_pParent + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&phys_upimpactforcescale.m_pParent + unaff_EBX + 2) = 0;
    *(undefined1 *)((int)&phys_upimpactforcescale.m_pParent + unaff_EBX + 3) = 0;
    *(undefined4 *)((int)&phys_upimpactforcescale.m_pszDefaultValue + unaff_EBX) = 0;
    *(undefined4 *)((int)&phys_upimpactforcescale.m_Value.m_pszString + unaff_EBX) = 0;
    *(undefined4 *)((int)&phys_upimpactforcescale.m_Value.m_StringLength + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&phys_upimpactforcescale.m_Value.m_fValue + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&phys_upimpactforcescale.m_Value.m_nValue + unaff_EBX) = 0;
    *(undefined4 *)(&phys_upimpactforcescale.m_bHasMin + unaff_EBX) = 0;
    *(undefined4 *)((int)&phys_upimpactforcescale.m_fMinVal + unaff_EBX) = 0;
    *(undefined4 *)(&phys_upimpactforcescale.m_bHasMax + unaff_EBX) = 0;
    *(undefined4 *)((int)&phys_upimpactforcescale.m_fMaxVal + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)
     ((int)&phys_upimpactforcescale.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) = 0x7f7fffff
    ;
    *(undefined4 *)
     ((int)&phys_upimpactforcescale.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)
     ((int)&phys_upimpactforcescale.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) =
         0x7f7fffff;
    *(undefined **)((int)&phys_upimpactforcescale.m_fnChangeCallbacks.m_Size + unaff_EBX) =
         &UNK_00b99a64 + unaff_EBX;
    *(undefined **)((int)&phys_upimpactforcescale.m_fnChangeCallbacks.m_pElements + unaff_EBX) =
         &UNK_00c1e214 + unaff_EBX;
    pIVar4 = EntityFactoryDictionary();
    (**pIVar4->_vptr_IEntityFactoryDictionary)(pIVar4,unaff_EBX + 0xd9cd34,unaff_EBX + 0x9a9b80);
    pdVar5 = DataMapInit<CProjectedWallEntity>((CProjectedWallEntity *)0x0);
    *(datamap_t **)((int)DataMapInit<CRallyPoint>::dataDesc[5].flatOffset + unaff_EBX + 4) = pdVar5;
    puVar1 = &DAT_00d9cd38 + unaff_EBX;
    pcVar2 = (char *)(unaff_EBX + 0xa00b13);
    *(char **)(&DAT_00d9cd38 + unaff_EBX) = pcVar2;
    pSVar3 = (SendTable *)((int)DataMapInit<CRallyPoint>::dataDesc[5].flatOffset + unaff_EBX + 8);
    *(SendTable **)(&DAT_00d9cd3c + unaff_EBX) = pSVar3;
    *(undefined4 *)((int)&g_sModelIndexFireball + unaff_EBX) = 0xffff;
    puVar10 = (undefined4 *)**(int **)(&DAT_00b94350 + unaff_EBX);
    if (puVar10 == (undefined4 *)0x0) {
      **(int **)(&DAT_00b94350 + unaff_EBX) = (int)puVar1;
      *(undefined4 *)((int)&g_sModelIndexLaser + unaff_EBX) = 0;
    }
    else {
      puVar9 = (undefined4 *)puVar10[2];
      iVar6 = _V_stricmp((char *)*puVar10,pcVar2);
      if (iVar6 < 1) {
        if (puVar9 != (undefined4 *)0x0) {
          do {
            puVar8 = puVar9;
            iVar6 = _V_stricmp((char *)*puVar8,pcVar2);
            puVar9 = puVar8;
            if (0 < iVar6) goto LAB_000b2bb0;
            puVar9 = (undefined4 *)puVar8[2];
            puVar10 = puVar8;
          } while ((undefined4 *)puVar8[2] != (undefined4 *)0x0);
          puVar9 = (undefined4 *)0x0;
        }
LAB_000b2bb0:
        *(undefined4 **)((int)&g_sModelIndexLaser + unaff_EBX) = puVar9;
        puVar10[2] = puVar1;
      }
      else {
        *(undefined4 *)((int)&g_sModelIndexLaser + unaff_EBX) =
             **(undefined4 **)(&DAT_00b94350 + unaff_EBX);
        **(undefined4 **)(&DAT_00b94350 + unaff_EBX) = puVar1;
      }
    }
    SendTable::SendTable(pSVar3);
    ___cxa_atexit(unaff_EBX + 0x98c4dc,0,*(undefined4 *)(&DAT_00b942b0 + unaff_EBX));
    iVar6 = ServerClassInit<DT_ProjectedWallEntity::ignored>((ignored *)0x0);
    *(int *)((int)DataMapInit<CRallyPoint>::dataDesc[6].flatOffset + unaff_EBX + -0x24) = iVar6;
    iVar6 = (int)&g_sModelIndexSmoke + unaff_EBX;
    pcVar2 = (char *)(unaff_EBX + 0xa00b3c);
    *(char **)((int)&g_sModelIndexSmoke + unaff_EBX) = pcVar2;
    pSVar3 = (SendTable *)
             ((int)DataMapInit<CRallyPoint>::dataDesc[6].flatOffset + unaff_EBX + -0x20);
    *(SendTable **)((int)&g_sModelIndexWExplosion + unaff_EBX) = pSVar3;
    *(undefined4 *)((int)&g_sModelIndexBloodSpray + unaff_EBX) = 0xffff;
    puVar10 = (undefined4 *)**(int **)(&DAT_00b94350 + unaff_EBX);
    if (puVar10 == (undefined4 *)0x0) {
      **(int **)(&DAT_00b94350 + unaff_EBX) = iVar6;
      *(undefined4 *)((int)&g_sModelIndexBubbles + unaff_EBX) = 0;
    }
    else {
      puVar9 = (undefined4 *)puVar10[2];
      iVar7 = _V_stricmp((char *)*puVar10,pcVar2);
      if (iVar7 < 1) {
        while ((puVar9 != (undefined4 *)0x0 &&
               (iVar7 = _V_stricmp((char *)*puVar9,pcVar2), iVar7 < 1))) {
          puVar10 = puVar9;
          puVar9 = (undefined4 *)puVar9[2];
        }
        *(undefined4 **)((int)&g_sModelIndexBubbles + unaff_EBX) = puVar9;
        puVar10[2] = iVar6;
      }
      else {
        *(undefined4 *)((int)&g_sModelIndexBubbles + unaff_EBX) =
             **(undefined4 **)(&DAT_00b94350 + unaff_EBX);
        **(int **)(&DAT_00b94350 + unaff_EBX) = iVar6;
      }
    }
    SendTable::SendTable(pSVar3);
    ___cxa_atexit(unaff_EBX + 0x98c4bc,0,*(undefined4 *)(&DAT_00b942b0 + unaff_EBX));
    iVar6 = ServerClassInit<DT_TEWallPaintedEvent::ignored>((ignored *)0x0);
    *(int *)((int)DataMapInit<CRallyPoint>::dataDesc[6].flatOffset + unaff_EBX + -0xc) = iVar6;
    CBaseTempEntity::CBaseTempEntity
              ((CBaseTempEntity *)
               ((int)&weapon_showproficiency.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX),
               (char *)(unaff_EBX + 0xa00b50));
    *(undefined **)
     ((int)&weapon_showproficiency.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) =
         &UNK_00c1de04 + unaff_EBX;
    *(undefined4 *)((int)&weapon_showproficiency.super_ConCommandBase.m_pszName + unaff_EBX) =
         0xffffffff;
    ___cxa_atexit(unaff_EBX + 0x98c5dc,0,*(undefined4 *)(&DAT_00b942b0 + unaff_EBX));
  }
  return;
}


/* CProjectedWallEntity::GetDataDescMap at 00748700 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: datamap_t * GetDataDescMap(CProjectedWallEntity * this) */

datamap_t * __thiscall CProjectedWallEntity::GetDataDescMap(CProjectedWallEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x61f0b8);
}


/* CProjectedWallEntity::GetBaseMap at 00748710 */

datamap_t * CProjectedWallEntity::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4fefa4);
}


/* __tcf_0 at 00a3eda0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x410dfe)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x410df2) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x410dfe));
  }
  *(undefined4 *)(unaff_EBX + 0x410dfe) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x410dfa)) {
    if (*(int *)(unaff_EBX + 0x410df2) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x207dae) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x207dae),*(int *)(unaff_EBX + 0x410df2));
      *(undefined4 *)(unaff_EBX + 0x410df2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x410df6) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x410df2);
  *(int *)(unaff_EBX + 0x410e02) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x410dfa)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x207dae) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x207dae),iVar1);
      *(undefined4 *)(unaff_EBX + 0x410df2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x410df6) = 0;
  }
  return;
}


/* ServerClassInit<DT_ProjectedWallEntity::ignored> at 000b22a0 */

/* WARNING: Enum "PrecacheSystem_t": Some values do not have unique names */

int ServerClassInit<DT_ProjectedWallEntity::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (g_EntityListPool.m_BlobHead.m_Data[unaff_EBX + -10] == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xd9d4e2);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  ((int)&CBaseEntityOutput_DataDescInit::g_DataMapHolder + unaff_EBX + 2),
                  (char *)(unaff_EBX + 0x9a3986),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&g_EventQueue.m_Events.m_iTarget.pszValue + unaff_EBX + 2),
                        &UNK_009a5d36 + unaff_EBX,0,
                        (SendTable *)**(undefined4 **)(&DAT_00b95416 + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00b94942 + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)(unaff_EBX + 0xd9d59a),&UNK_00a0106c + unaff_EBX,0x428,0xc
                     ,-1,4,0.0,-121121.125,*(SendVarProxyFn_conflict *)(&DAT_00b9498e + unaff_EBX),
                     0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&ClientPrecachePrecache::s_ResourcePrecacher.super_CBaseResourcePrecacher
                            .m_pNext + unaff_EBX + 2),(char *)(unaff_EBX + 0xa01083),0x434,0xc,-1,4,
                     0.0,-121121.125,*(SendVarProxyFn_conflict *)(&DAT_00b9498e + unaff_EBX),0x80);
      SendPropEHandle((SendProp_conflict *)((int)&noclip_fixup.m_fMaxVal + unaff_EBX + 2),
                      (char *)(unaff_EBX + 0xa0109a),0x440,4,-1,
                      *(SendVarProxyFn_conflict *)(&DAT_00b9492e + unaff_EBX));
      SendPropFloat((SendProp_conflict *)
                    ((int)&DT_ColorCorrection::g_SendTable.m_pPrecalc + unaff_EBX + 2),
                    (char *)(unaff_EBX + 0xa010a9),0x444,4,-1,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00b9499a + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)
                    (&commentary_available.super_ConCommandBase.field_0xa + unaff_EBX),
                    (char *)(unaff_EBX + 0xa0f8ae),0x44c,4,-1,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00b9499a + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&PrecachePointCommentaryNodePrecache::s_ResourcePrecacher.
                           super_CBaseResourcePrecacher.m_nSystem + unaff_EBX + 2),
                    (char *)(unaff_EBX + 0xa010b6),0x448,4,-1,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00b9499a + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&g_CommentarySystem.m_iCommentaryNodeCount + unaff_EBX + 2),
                    (char *)(unaff_EBX + 0xa010c1),0x458,4,-1,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00b9499a + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&modifiedconvars_t_DataDescInit::g_DataMapHolder + unaff_EBX + 2),
                    (char *)(unaff_EBX + 0xa010d3),0x45c,4,-1,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00b9499a + unaff_EBX),0x80);
      SendPropInt((SendProp_conflict *)((int)&commentary.m_fMinVal + unaff_EBX + 2),
                  (char *)(unaff_EBX + 0xa010ea),0x454,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropBool((SendProp_conflict *)
                   ((int)&ai_debug_ragdoll_magnets.super_ConCommandBase.m_pszName + unaff_EBX + 2),
                   (char *)(unaff_EBX + 0xa010f9),0x450,1);
      ___cxa_guard_release(unaff_EBX + 0xd9d4e2);
      ___cxa_atexit(unaff_EBX + 0x98c972,0,*(undefined4 *)(&DAT_00b948a6 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)
             ((int)DataMapInit<CAI_FightFromCoverGoal>::dataDesc[7].flatOffset + unaff_EBX + -2),
             (SendProp_conflict *)((int)&g_EventQueue.m_Events.m_iTarget.pszValue + unaff_EBX + 2),
             0xb,*(char **)(&DAT_00c1e7ce + unaff_EBX));
  return 1;
}


/* CProjectedWallEntity::GetServerClass at 00748720 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: ServerClass * GetServerClass(CProjectedWallEntity * this) */

ServerClass * __thiscall CProjectedWallEntity::GetServerClass(CProjectedWallEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x706eb4);
}


/* CProjectedWallEntity::YouForgotToImplementOrDeclareServerClass at 00748730 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CProjectedWallEntity *
   this) */

int __thiscall
CProjectedWallEntity::YouForgotToImplementOrDeclareServerClass(CProjectedWallEntity *this)

{
  return 0;
}


/* __tcf_3 at 00a3ed80 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x38a898),in_stack_00000008);
  return;
}


/* __tcf_1 at 00a3ec20 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410f0f))(unaff_EBX + 0x410f0f);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410ebb))(unaff_EBX + 0x410ebb);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410e67))(unaff_EBX + 0x410e67);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410e13))(unaff_EBX + 0x410e13);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410dbf))(unaff_EBX + 0x410dbf);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410d6b))(unaff_EBX + 0x410d6b);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410d17))(unaff_EBX + 0x410d17);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410cc3))(unaff_EBX + 0x410cc3);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410c6f))(unaff_EBX + 0x410c6f);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410c1b))(unaff_EBX + 0x410c1b);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410bc7))(unaff_EBX + 0x410bc7);
                    /* WARNING: Could not recover jumptable at 0x00a3ecf9. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410b73))();
  return;
}


/* CProjectedWallEntity::CProjectedWallEntity at 0074a290 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void CProjectedWallEntity(CProjectedWallEntity * this) */

void __thiscall CProjectedWallEntity::CProjectedWallEntity(CProjectedWallEntity *this)

{
  undefined1 *puVar1;
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
  CBaseProjectedEntity::CBaseProjectedEntity((CBaseProjectedEntity *)this);
  (this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.super_CBaseEntity.
  super_IServerEntity.super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x586447);
  (this->super_CPaintableEntity<CBaseProjectedEntity>).super_IPaintableEntity._vptr_IPaintableEntity
       = (_func_int_varargs **)(unaff_EBX + 0x5867ab);
  (this->m_hColorPortal).m_Value.super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_flLength).m_Value = 0.0;
  if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
      super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
    pCVar5 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
               super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
      pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
      pIVar8 = CBaseEdict::GetChangeAccessor(pCVar5);
      piVar6 = *(int **)(unaff_EBX + 0x4fc8c7);
      puVar7 = (ushort *)*piVar6;
      if (pIVar8->m_iChangeInfoSerialNumber == *puVar7) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        uVar10 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar4 = puVar7[uVar10 * 0x14 + 0x14];
        if (uVar4 == 0) {
LAB_0074a8c6:
          puVar7[(uint)uVar4 + uVar10 * 0x14 + 1] = 0x444;
          puVar7[uVar10 * 0x14 + 0x14] = uVar4 + 1;
        }
        else if (puVar7[uVar10 * 0x14 + 1] != 0x444) {
          uVar9 = 0;
          do {
            uVar2 = uVar9 + 1;
            uVar9 = uVar2 & 0xffff;
            if ((ushort)uVar2 == uVar4) {
              if (uVar4 != 0x13) goto LAB_0074a8c6;
              goto LAB_0074acce;
            }
          } while (puVar7[uVar10 * 0x14 + uVar9 + 1] != 0x444);
        }
      }
      else if (puVar7[0x7d1] == 100) {
LAB_0074acce:
        pIVar8->m_iChangeInfoSerialNumber = 0;
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar8->m_iChangeInfo = puVar7[0x7d1];
        *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
        pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
        iVar3 = *piVar6 + (uint)pIVar8->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar3 + 2) = 0x444;
        *(undefined2 *)(iVar3 + 0x28) = 1;
      }
    }
  }
  else {
    puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
              super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  (this->m_flHeight).m_Value = 0.015625;
  if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
      super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
    pCVar5 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
               super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
      pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
      pIVar8 = CBaseEdict::GetChangeAccessor(pCVar5);
      piVar6 = *(int **)(unaff_EBX + 0x4fc8c7);
      puVar7 = (ushort *)*piVar6;
      if (pIVar8->m_iChangeInfoSerialNumber == *puVar7) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        uVar10 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar4 = puVar7[uVar10 * 0x14 + 0x14];
        if (uVar4 == 0) {
LAB_0074a973:
          puVar7[(uint)uVar4 + uVar10 * 0x14 + 1] = 0x448;
          puVar7[uVar10 * 0x14 + 0x14] = uVar4 + 1;
        }
        else if (puVar7[uVar10 * 0x14 + 1] != 0x448) {
          uVar9 = 0;
          do {
            uVar2 = uVar9 + 1;
            uVar9 = uVar2 & 0xffff;
            if ((ushort)uVar2 == uVar4) {
              if (uVar4 != 0x13) goto LAB_0074a973;
              goto LAB_0074ad44;
            }
          } while (puVar7[uVar10 * 0x14 + uVar9 + 1] != 0x448);
        }
      }
      else if (puVar7[0x7d1] == 100) {
LAB_0074ad44:
        pIVar8->m_iChangeInfoSerialNumber = 0;
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar8->m_iChangeInfo = puVar7[0x7d1];
        *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
        pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
        iVar3 = *piVar6 + (uint)pIVar8->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar3 + 2) = 0x448;
        *(undefined2 *)(iVar3 + 0x28) = 1;
      }
    }
  }
  else {
    puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
              super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  (this->m_flWidth).m_Value = 64.0;
  if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
      super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
    pCVar5 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
               super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
      pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
      pIVar8 = CBaseEdict::GetChangeAccessor(pCVar5);
      piVar6 = *(int **)(unaff_EBX + 0x4fc8c7);
      puVar7 = (ushort *)*piVar6;
      if (pIVar8->m_iChangeInfoSerialNumber == *puVar7) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        uVar10 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar4 = puVar7[uVar10 * 0x14 + 0x14];
        if (uVar4 == 0) {
LAB_0074aa13:
          puVar7[(uint)uVar4 + uVar10 * 0x14 + 1] = 0x44c;
          puVar7[uVar10 * 0x14 + 0x14] = uVar4 + 1;
        }
        else if (puVar7[uVar10 * 0x14 + 1] != 0x44c) {
          uVar9 = 0;
          do {
            uVar2 = uVar9 + 1;
            uVar9 = uVar2 & 0xffff;
            if ((ushort)uVar2 == uVar4) {
              if (uVar4 != 0x13) goto LAB_0074aa13;
              goto LAB_0074ad16;
            }
          } while (puVar7[uVar10 * 0x14 + uVar9 + 1] != 0x44c);
        }
      }
      else if (puVar7[0x7d1] == 100) {
LAB_0074ad16:
        pIVar8->m_iChangeInfoSerialNumber = 0;
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar8->m_iChangeInfo = puVar7[0x7d1];
        *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
        pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
        iVar3 = *piVar6 + (uint)pIVar8->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar3 + 2) = 0x44c;
        *(undefined2 *)(iVar3 + 0x28) = 1;
      }
    }
  }
  else {
    puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
              super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  (this->m_bIsHorizontal).m_Value = false;
  if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
      super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
    pCVar5 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
               super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
      pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
      pIVar8 = CBaseEdict::GetChangeAccessor(pCVar5);
      piVar6 = *(int **)(unaff_EBX + 0x4fc8c7);
      puVar7 = (ushort *)*piVar6;
      if (pIVar8->m_iChangeInfoSerialNumber == *puVar7) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        uVar10 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar4 = puVar7[uVar10 * 0x14 + 0x14];
        if (uVar4 == 0) {
LAB_0074aab3:
          puVar7[(uint)uVar4 + uVar10 * 0x14 + 1] = 0x450;
          puVar7[uVar10 * 0x14 + 0x14] = uVar4 + 1;
        }
        else if (puVar7[uVar10 * 0x14 + 1] != 0x450) {
          uVar9 = 0;
          do {
            uVar2 = uVar9 + 1;
            uVar9 = uVar2 & 0xffff;
            if ((ushort)uVar2 == uVar4) {
              if (uVar4 != 0x13) goto LAB_0074aab3;
              goto LAB_0074ad2d;
            }
          } while (puVar7[uVar10 * 0x14 + uVar9 + 1] != 0x450);
        }
      }
      else if (puVar7[0x7d1] == 100) {
LAB_0074ad2d:
        pIVar8->m_iChangeInfoSerialNumber = 0;
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar8->m_iChangeInfo = puVar7[0x7d1];
        *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
        pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
        iVar3 = *piVar6 + (uint)pIVar8->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar3 + 2) = 0x450;
        *(undefined2 *)(iVar3 + 0x28) = 1;
      }
    }
  }
  else {
    puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
              super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  (this->m_nNumSegments).m_Value = 0;
  if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
      super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
    pCVar5 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
               super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
      pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
      pIVar8 = CBaseEdict::GetChangeAccessor(pCVar5);
      piVar6 = *(int **)(unaff_EBX + 0x4fc8c7);
      puVar7 = (ushort *)*piVar6;
      if (pIVar8->m_iChangeInfoSerialNumber == *puVar7) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        uVar10 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar4 = puVar7[uVar10 * 0x14 + 0x14];
        if (uVar4 == 0) {
LAB_0074ab53:
          puVar7[(uint)uVar4 + uVar10 * 0x14 + 1] = 0x454;
          puVar7[uVar10 * 0x14 + 0x14] = uVar4 + 1;
        }
        else if (puVar7[uVar10 * 0x14 + 1] != 0x454) {
          uVar9 = 0;
          do {
            uVar2 = uVar9 + 1;
            uVar9 = uVar2 & 0xffff;
            if ((ushort)uVar2 == uVar4) {
              if (uVar4 != 0x13) goto LAB_0074ab53;
              goto LAB_0074ace8;
            }
          } while (puVar7[uVar10 * 0x14 + uVar9 + 1] != 0x454);
        }
      }
      else if (puVar7[0x7d1] == 100) {
LAB_0074ace8:
        pIVar8->m_iChangeInfoSerialNumber = 0;
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar8->m_iChangeInfo = puVar7[0x7d1];
        *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
        pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
        iVar3 = *piVar6 + (uint)pIVar8->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar3 + 2) = 0x454;
        *(undefined2 *)(iVar3 + 0x28) = 1;
      }
    }
  }
  else {
    puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
              super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  (this->m_flSegmentLength).m_Value = 64.0;
  if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
      super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
    pCVar5 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
               super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
      pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
      pIVar8 = CBaseEdict::GetChangeAccessor(pCVar5);
      piVar6 = *(int **)(unaff_EBX + 0x4fc8c7);
      puVar7 = (ushort *)*piVar6;
      if (pIVar8->m_iChangeInfoSerialNumber == *puVar7) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        uVar10 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar4 = puVar7[uVar10 * 0x14 + 0x14];
        if (uVar4 == 0) {
LAB_0074abf3:
          puVar7[(uint)uVar4 + uVar10 * 0x14 + 1] = 0x458;
          puVar7[uVar10 * 0x14 + 0x14] = uVar4 + 1;
        }
        else if (puVar7[uVar10 * 0x14 + 1] != 0x458) {
          uVar9 = 0;
          do {
            uVar2 = uVar9 + 1;
            uVar9 = uVar2 & 0xffff;
            if ((ushort)uVar2 == uVar4) {
              if (uVar4 != 0x13) goto LAB_0074abf3;
              goto LAB_0074acff;
            }
          } while (puVar7[uVar10 * 0x14 + uVar9 + 1] != 0x458);
        }
      }
      else if (puVar7[0x7d1] == 100) {
LAB_0074acff:
        pIVar8->m_iChangeInfoSerialNumber = 0;
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar8->m_iChangeInfo = puVar7[0x7d1];
        *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
        pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
        iVar3 = *piVar6 + (uint)pIVar8->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar3 + 2) = 0x458;
        *(undefined2 *)(iVar3 + 0x28) = 1;
      }
    }
  }
  else {
    puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
              super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  (this->m_flParticleUpdateTime).m_Value = 0.0;
  if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
      super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
    pCVar5 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
               super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
      pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
      pIVar8 = CBaseEdict::GetChangeAccessor(pCVar5);
      piVar6 = *(int **)(unaff_EBX + 0x4fc8c7);
      puVar7 = (ushort *)*piVar6;
      if (pIVar8->m_iChangeInfoSerialNumber == *puVar7) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        uVar10 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar4 = puVar7[uVar10 * 0x14 + 0x14];
        if (uVar4 == 0) {
LAB_0074ac8f:
          puVar7[(uint)uVar4 + uVar10 * 0x14 + 1] = 0x45c;
          puVar7[uVar10 * 0x14 + 0x14] = uVar4 + 1;
        }
        else if (puVar7[uVar10 * 0x14 + 1] != 0x45c) {
          uVar9 = 0;
          do {
            uVar2 = uVar9 + 1;
            uVar9 = uVar2 & 0xffff;
            if ((ushort)uVar2 == uVar4) {
              if (uVar4 != 0x13) goto LAB_0074ac8f;
              goto LAB_0074acb7;
            }
          } while (puVar7[uVar10 * 0x14 + uVar9 + 1] != 0x45c);
        }
      }
      else if (puVar7[0x7d1] == 100) {
LAB_0074acb7:
        pIVar8->m_iChangeInfoSerialNumber = 0;
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar8->m_iChangeInfo = puVar7[0x7d1];
        *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
        pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
        iVar3 = *piVar6 + (uint)pIVar8->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar3 + 2) = 0x45c;
        *(undefined2 *)(iVar3 + 0x28) = 1;
      }
    }
  }
  else {
    puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
              super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  (this->m_PaintPowers).m_Memory.m_pMemory = (PaintPowerType *)0x0;
  (this->m_PaintPowers).m_Memory.m_nAllocationCount = 0;
  (this->m_PaintPowers).m_Memory.m_nGrowSize = 0;
  (this->m_PaintPowers).m_Size = 0;
  (this->m_PaintPowers).m_pElements = (PaintPowerType *)0x0;
  return;
}


/* CProjectedWallEntity::CProjectedWallEntity at 0074ad90 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void CProjectedWallEntity(CProjectedWallEntity * this,
   CProjectedWallEntity * this) */

void __thiscall
CProjectedWallEntity::CProjectedWallEntity(CProjectedWallEntity *this,CProjectedWallEntity *this_1)

{
  CProjectedWallEntity(this);
  return;
}


/* CEntityFactory<CProjectedWallEntity>::Create at 0074cf90 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CProjectedWallEntity> *
   this, char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CProjectedWallEntity>::Create
          (CEntityFactory<CProjectedWallEntity> *this,char *pClassName)

{
  CProjectedWallEntity *this_00;
  
                    /* Unresolved local var: CProjectedWallEntity * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x474);
  CProjectedWallEntity::CProjectedWallEntity(this_00);
  (*(this_00->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
    super_CBaseEntity.super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x1d])(this_00,pClassName);
  return &(this_00->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
          super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CProjectedWallEntity::~CProjectedWallEntity at 00749e70 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void ~CProjectedWallEntity(CProjectedWallEntity * this, int __in_chrg)
    */

void __thiscall
CProjectedWallEntity::~CProjectedWallEntity(CProjectedWallEntity *this,int __in_chrg)

{
  CProjectedWallEntity *pCVar1;
  int unaff_EBX;
  CProjectedWallEntity *__in_chrg_00;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.super_CBaseEntity.
  super_IServerEntity.super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x58686a);
  (this->super_CPaintableEntity<CBaseProjectedEntity>).super_IPaintableEntity._vptr_IPaintableEntity
       = (_func_int_varargs **)(unaff_EBX + 0x586bce);
  CleanupWall(this);
  __in_chrg_00 = this;
  CPaintDatabase::RemovePaintedWall(*(CPaintDatabase **)(unaff_EBX + 0x4fd066),this,true);
  (this->m_PaintPowers).m_Size = 0;
  if ((this->m_PaintPowers).m_Memory.m_nGrowSize < 0) {
    pCVar1 = (CProjectedWallEntity *)(this->m_PaintPowers).m_Memory.m_pMemory;
  }
  else {
    pCVar1 = (CProjectedWallEntity *)(this->m_PaintPowers).m_Memory.m_pMemory;
    if (pCVar1 != (CProjectedWallEntity *)0x0) {
      __in_chrg_00 = pCVar1;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4fccde) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4fccde));
      pCVar1 = (CProjectedWallEntity *)0x0;
      (this->m_PaintPowers).m_Memory.m_pMemory = (PaintPowerType *)0x0;
    }
    (this->m_PaintPowers).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintPowers).m_pElements = (PaintPowerType *)pCVar1;
  if (-1 < (this->m_PaintPowers).m_Memory.m_nGrowSize) {
    if (pCVar1 != (CProjectedWallEntity *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4fccde) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4fccde));
      (this->m_PaintPowers).m_Memory.m_pMemory = (PaintPowerType *)0x0;
      __in_chrg_00 = pCVar1;
    }
    (this->m_PaintPowers).m_Memory.m_nAllocationCount = 0;
  }
  (this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.super_CBaseEntity.
  super_IServerEntity.super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x586c6a);
  (this->super_CPaintableEntity<CBaseProjectedEntity>).super_IPaintableEntity._vptr_IPaintableEntity
       = (_func_int_varargs **)(unaff_EBX + 0x586fce);
  (this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.super_CBaseEntity.
  super_IServerEntity.super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4fd842) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,(int)__in_chrg_00);
  CBaseEntity::operator_delete(this);
  return;
}


/* CProjectedWallEntity::~CProjectedWallEntity at 0074a070 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void ~CProjectedWallEntity(CProjectedWallEntity * this, int __in_chrg)
    */

void __thiscall
CProjectedWallEntity::~CProjectedWallEntity(CProjectedWallEntity *this,int __in_chrg)

{
  PaintPowerType *pPVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.super_CBaseEntity.
  super_IServerEntity.super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x58666a);
  (this->super_CPaintableEntity<CBaseProjectedEntity>).super_IPaintableEntity._vptr_IPaintableEntity
       = (_func_int_varargs **)(unaff_EBX + 0x5869ce);
  CleanupWall(this);
  CPaintDatabase::RemovePaintedWall(*(CPaintDatabase **)(unaff_EBX + 0x4fce66),this,true);
  (this->m_PaintPowers).m_Size = 0;
  if ((this->m_PaintPowers).m_Memory.m_nGrowSize < 0) {
    pPVar1 = (this->m_PaintPowers).m_Memory.m_pMemory;
  }
  else {
    pPVar1 = (this->m_PaintPowers).m_Memory.m_pMemory;
    if (pPVar1 != (PaintPowerType *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4fcade) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4fcade),pPVar1);
      (this->m_PaintPowers).m_Memory.m_pMemory = (PaintPowerType *)0x0;
    }
    pPVar1 = (PaintPowerType *)0x0;
    (this->m_PaintPowers).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintPowers).m_pElements = pPVar1;
  if (-1 < (this->m_PaintPowers).m_Memory.m_nGrowSize) {
    if (pPVar1 != (PaintPowerType *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4fcade) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4fcade),pPVar1);
      (this->m_PaintPowers).m_Memory.m_pMemory = (PaintPowerType *)0x0;
    }
    (this->m_PaintPowers).m_Memory.m_nAllocationCount = 0;
  }
  (this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.super_CBaseEntity.
  super_IServerEntity.super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x586a6a);
  (this->super_CPaintableEntity<CBaseProjectedEntity>).super_IPaintableEntity._vptr_IPaintableEntity
       = (_func_int_varargs **)(unaff_EBX + 0x586dce);
  (this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.super_CBaseEntity.
  super_IServerEntity.super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4fd642) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,__in_chrg);
  return;
}


/* CProjectedWallEntity::~CProjectedWallEntity at 0074a280 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void ~CProjectedWallEntity(CProjectedWallEntity * this, int __in_chrg,
   CProjectedWallEntity * this) */

void __thiscall
CProjectedWallEntity::~CProjectedWallEntity
          (CProjectedWallEntity *this,int __in_chrg,CProjectedWallEntity *this_1)

{
  ~CProjectedWallEntity(this,__in_chrg);
  return;
}


/* CProjectedWallEntity::UpdateOnRemove at 00749e40 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void UpdateOnRemove(CProjectedWallEntity * this) */

void __thiscall CProjectedWallEntity::UpdateOnRemove(CProjectedWallEntity *this)

{
  CleanupWall(this);
  CBaseProjectedEntity::UpdateOnRemove((CBaseProjectedEntity *)this);
  return;
}


/* CProjectedWallEntity::Precache at 00748af0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void Precache(CProjectedWallEntity * this) */

void __thiscall CProjectedWallEntity::Precache(CProjectedWallEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  PrecacheParticleSystem(&UNK_0036a80a + extraout_ECX);
  return;
}


/* CProjectedWallEntity::Spawn at 00748a50 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "SurroundingBoundsType_t": Some values do not have unique names */
/* WARNING: Enum "MoveCollide_t": Some values do not have unique names */
/* WARNING: Enum "MoveType_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(CProjectedWallEntity * this) */

void __thiscall CProjectedWallEntity::Spawn(CProjectedWallEntity *this)

{
  NetworkVar_m_Collision *this_00;
  
  CBaseProjectedEntity::Spawn((CBaseProjectedEntity *)this);
  (*(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
    super_CBaseEntity.super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x1a])(this);
  this_00 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
             super_CBaseEntity.m_Collision;
  CCollisionProperty::SetSolid(&this_00->super_CCollisionProperty,SOLID_CUSTOM);
  CCollisionProperty::SetSolidFlags(&this_00->super_CCollisionProperty,3);
  CCollisionProperty::SetSurroundingBoundsType
            (&this_00->super_CCollisionProperty,USE_GAME_CODE,(Vector *)0x0,(Vector *)0x0);
  CBaseEntity::SetMoveType((CBaseEntity *)this,MOVETYPE_NONE,MOVECOLLIDE_DEFAULT);
  (*(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
    super_CBaseEntity.super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x9f])(this);
  CBaseEntity::SetTransmitState((CBaseEntity *)this,8);
  return;
}


/* CProjectedWallEntity::OnRestore at 00748a20 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void OnRestore(CProjectedWallEntity * this) */

void __thiscall CProjectedWallEntity::OnRestore(CProjectedWallEntity *this)

{
  CBaseProjectedEntity::OnRestore((CBaseProjectedEntity *)this);
  CBaseEntity::SetTransmitState((CBaseEntity *)this,8);
  return;
}


/* CProjectedWallEntity::ProjectWall at 0074ada0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ProjectWall(CProjectedWallEntity * this) */

void __thiscall CProjectedWallEntity::ProjectWall(CProjectedWallEntity *this)

{
  undefined1 *puVar1;
  uint uVar2;
  vec_t *pvVar3;
  uint *puVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  float fVar10;
  ushort uVar11;
  code *pcVar12;
  CBaseEdict *pCVar13;
  PaintPowerType *src;
  ushort *puVar14;
  int *piVar15;
  CBasePlayer *this_00;
  CProjectedWallEntity *pCVar16;
  float *pfVar17;
  CPhysCollide *pCVar18;
  IPhysicsObject *pPhysics;
  int iVar19;
  CPolyhedron *pCVar20;
  IChangeInfoAccessor *pIVar21;
  uint uVar22;
  uint uVar23;
  int unaff_EBX;
  undefined4 *puVar24;
  NetworkVar_m_Collision *this_01;
  int iVar25;
  objectparams_t *poVar26;
  longdouble lVar27;
  float fVar28;
  uint local_968;
  uint local_964;
  uint local_960;
  Vector *local_908;
  bool local_8e5;
  Vector local_86c;
  float local_860;
  float local_85c;
  float local_858;
  float local_854;
  float local_850;
  float local_84c;
  float local_848;
  float local_844;
  float local_840;
  float local_83c;
  float local_838;
  float local_834;
  float local_830;
  float local_82c;
  float local_828;
  float local_824;
  float local_820;
  float local_81c;
  float local_818;
  float local_814;
  float local_810;
  float local_80c [4];
  Vector local_7fc;
  float *local_7f0;
  vec_t local_7ec;
  vec_t local_7e8;
  vec_t local_7e4;
  vec_t local_7e0;
  vec_t local_7dc;
  vec_t local_7d8;
  float local_7d4;
  float local_7d0;
  float local_7cc;
  float local_7c8;
  float local_7c4;
  float local_7c0;
  float local_7bc;
  float local_7b8;
  float local_7b4;
  float local_7b0;
  float local_7ac;
  float local_7a8;
  float local_7a4;
  float local_7a0;
  float local_79c;
  float local_798;
  float local_794;
  float local_790;
  Vector local_78c;
  Vector local_780;
  Vector *local_774;
  float *local_770;
  float *local_76c;
  float local_768;
  float local_764;
  float local_760;
  float local_75c;
  float local_758;
  float local_754;
  float local_750;
  float local_74c;
  float local_748;
  float local_744;
  float local_740;
  float local_73c;
  float local_738;
  float local_734;
  float local_730;
  float local_72c;
  float local_728;
  float local_724;
  float local_720;
  float local_71c;
  float local_718;
  float local_714;
  float local_710;
  float local_70c;
  float local_708;
  float local_704;
  float local_700;
  float local_6fc;
  float local_6f8;
  float local_6f4;
  float local_6f0;
  float local_6ec;
  float local_6e8;
  float local_6e4;
  float local_6e0;
  float local_6dc;
  float local_6d8;
  float local_6d4;
  float local_6d0;
  float local_6cc;
  float local_6c8;
  float local_6c4;
  float local_6c0;
  float local_6bc;
  float local_6b8;
  float local_6b4;
  float local_6b0;
  float local_6ac;
  float local_6a8;
  float local_6a4;
  float local_6a0;
  float local_69c;
  float local_698;
  float local_694;
  float local_690;
  float local_68c;
  float local_688;
  float local_684;
  float local_680;
  float local_67c;
  float local_678;
  float local_674;
  float local_670;
  float local_66c;
  float local_668;
  float local_664;
  solid_t local_660;
  int local_20;
  
                    /* Unresolved local var: Vector vWallStartPoint@[???]
                       Unresolved local var: Vector vWallEndPoint@[???]
                       Unresolved local var: Vector vecForward@[???]
                       Unresolved local var: Vector vecRight@[???]
                       Unresolved local var: Vector vecUp@[???]
                       Unresolved local var: solid_t solid@[???]
                       Unresolved local var: IPhysicsObject * pWallObj@[???]
                       Unresolved local var: Vector vMins@[???]
                       Unresolved local var: Vector vMaxs@[???]
                       Unresolved local var: Vector vUp@[???]
                       Unresolved local var: Vector vRight@[???] */
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(unaff_EBX + 0x4fbdff);
  CleanupWall(this);
  CBaseEntity::AddEffects((CBaseEntity *)this,8);
  if (0 < *(int *)(**(int **)(unaff_EBX + 0x4fbdb3) + 0x14)) {
    iVar25 = 1;
    do {
      this_00 = UTIL_PlayerByIndex(iVar25);
      if ((this_00 != (CBasePlayer *)0x0) &&
         (pCVar16 = (CProjectedWallEntity *)CBaseEntity::GetGroundEntity((CBaseEntity *)this_00),
         this == pCVar16)) {
        CBaseEntity::SetGroundEntity((CBaseEntity *)this_00,(CBaseEntity *)0x0);
      }
      iVar25 = iVar25 + 1;
    } while (iVar25 <= *(int *)(**(int **)(unaff_EBX + 0x4fbdb3) + 0x14));
  }
  pfVar17 = (float *)(*(this->super_CPaintableEntity<CBaseProjectedEntity>).
                       super_CBaseProjectedEntity.super_CBaseEntity.super_IServerEntity.
                       super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xcb])(this);
  fVar28 = pfVar17[2];
  fVar5 = pfVar17[1];
  fVar6 = *pfVar17;
  pfVar17 = (float *)(*(this->super_CPaintableEntity<CBaseProjectedEntity>).
                       super_CBaseProjectedEntity.super_CBaseEntity.super_IServerEntity.
                       super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xcc])(this);
  fVar7 = pfVar17[2];
  fVar8 = pfVar17[1];
  fVar9 = *pfVar17;
  (*(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
    super_CBaseEntity.super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x8e])(this,&local_66c,&local_678,&local_684);
  if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x4fc907) + 0x1c) + 0x30) == 0) {
    puVar4 = (uint *)(unaff_EBX + 0x38ac5f);
    local_86c.z = (vec_t)((uint)local_664 ^ *puVar4);
    local_86c.y = (vec_t)((uint)local_668 ^ *puVar4);
    local_86c.x = (vec_t)((uint)local_66c ^ *puVar4);
    local_860 = fVar6 * local_86c.x + fVar5 * local_86c.y + fVar28 * local_86c.z;
    local_85c = local_66c;
    local_858 = local_668;
    local_854 = local_664;
    local_850 = fVar9 * local_66c + fVar8 * local_668 + fVar7 * local_664;
                    /* Unresolved local var: Vector res@[???] */
    fVar10 = *(float *)(unaff_EBX + 0x381e6b);
                    /* Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
    pfVar17 = (float *)(unaff_EBX + 0x381d4f);
    local_6f0 = local_678 * fVar10 * *pfVar17;
    local_6ec = local_674 * fVar10 * *pfVar17;
    local_6e8 = *pfVar17 * local_670 * fVar10;
                    /* Unresolved local var: Vector res@[???] */
    local_6e4 = fVar6 + local_6f0;
    local_6e0 = fVar5 + local_6ec;
    local_6dc = fVar28 + local_6e8;
    local_84c = local_678;
    local_848 = local_674;
    local_844 = local_670;
    local_840 = local_6e4 * local_678 + local_6e0 * local_674 + local_6dc * local_670;
                    /* Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
                    /* Unresolved local var: Vector res@[???] */
    local_6fc = fVar6 - local_6f0;
    local_6f8 = fVar5 - local_6ec;
    local_6f4 = fVar28 - local_6e8;
    local_834 = (float)((uint)local_670 ^ *puVar4);
    local_838 = (float)((uint)local_674 ^ *puVar4);
    local_83c = (float)((uint)local_678 ^ *puVar4);
    local_830 = local_6fc * local_83c + local_6f8 * local_838 + local_6f4 * local_834;
                    /* Unresolved local var: Vector res@[???] */
    fVar10 = *(float *)(unaff_EBX + 0x3821e7);
                    /* Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
    pfVar17 = (float *)(unaff_EBX + 0x381d4f);
    local_72c = local_684 * fVar10 * *pfVar17;
    local_728 = local_680 * fVar10 * *pfVar17;
    local_724 = *pfVar17 * local_67c * fVar10;
                    /* Unresolved local var: Vector res@[???] */
    local_720 = fVar6 + local_72c;
    local_71c = fVar5 + local_728;
    local_718 = fVar28 + local_724;
    local_82c = local_684;
    local_828 = local_680;
    local_824 = local_67c;
    local_820 = local_720 * local_684 + local_71c * local_680 + local_718 * local_67c;
                    /* Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
                    /* Unresolved local var: Vector res@[???] */
    local_738 = fVar6 - local_72c;
    local_734 = fVar5 - local_728;
    local_730 = fVar28 - local_724;
    local_814 = (float)((uint)local_67c ^ *puVar4);
    local_818 = (float)((uint)local_680 ^ *puVar4);
    local_81c = (float)((uint)local_684 ^ *puVar4);
    local_810 = local_738 * local_81c + local_734 * local_818 + local_730 * local_814;
    local_7d4 = local_81c;
    local_7d0 = local_818;
    local_7cc = local_814;
    local_7c8 = local_81c;
    local_7c4 = local_818;
    local_7c0 = local_814;
    local_7bc = local_83c;
    local_7b8 = local_838;
    local_7b4 = local_834;
    local_7b0 = local_83c;
    local_7ac = local_838;
    local_7a8 = local_834;
    local_7a4 = local_86c.x;
    local_7a0 = local_86c.y;
    local_79c = local_86c.z;
    local_798 = local_86c.x;
    local_794 = local_86c.y;
    local_790 = local_86c.z;
    local_744 = local_81c;
    local_740 = local_818;
    local_73c = local_814;
    local_714 = local_72c;
    local_710 = local_728;
    local_70c = local_724;
    local_708 = local_83c;
    local_704 = local_838;
    local_700 = local_834;
    local_6d8 = local_6f0;
    local_6d4 = local_6ec;
    local_6d0 = local_6e8;
    local_6cc = local_86c.x;
    local_6c8 = local_86c.y;
    local_6c4 = local_86c.z;
    pCVar20 = GeneratePolyhedronFromPlanes(&local_86c.x,6,0.0,false);
    if (pCVar20 == (CPolyhedron *)0x0) {
      _Warning(unaff_EBX + 0x368657);
      goto LAB_0074b81c;
    }
    puVar24 = *(undefined4 **)(unaff_EBX + 0x4fbef3);
    local_80c[0] = (float)(**(code **)(*(int *)*puVar24 + 0x24))((int *)*puVar24,pCVar20);
    (*pCVar20->_vptr_CPolyhedron[2])(pCVar20);
    if (local_80c[0] == 0.0) goto LAB_0074b81c;
  }
  else {
                    /* Unresolved local var: Vector vScaledRight@[???]
                       Unresolved local var: Vector vStart@[???]
                       Unresolved local var: Vector vEnd@[???]
                       Unresolved local var: Vector[4] vVerts@[???]
                       Unresolved local var: Vector *[4] pVerts@[???]
                       Unresolved local var: CPhysConvex * pTempConvex@[???]
                       Unresolved local var: Vector res@[???] */
    local_688 = *(float *)(unaff_EBX + 0x381e6f);
    local_690 = local_678 * local_688;
    local_68c = local_674 * local_688;
    local_688 = local_670 * local_688;
                    /* Unresolved local var: Vector res@[???] */
    local_86c.x = fVar6 + local_690;
    local_86c.y = fVar5 + local_68c;
    local_86c.z = fVar28 + local_688;
                    /* Unresolved local var: Vector res@[???] */
    local_860 = fVar6 - local_690;
    local_85c = fVar5 - local_68c;
    local_858 = fVar28 - local_688;
                    /* Unresolved local var: Vector res@[???] */
    local_854 = fVar9 - local_690;
    local_850 = fVar8 - local_68c;
    local_84c = fVar7 - local_688;
                    /* Unresolved local var: Vector res@[???] */
    local_848 = local_690 + fVar9;
    local_844 = fVar8 + local_68c;
    local_840 = local_688 + fVar7;
    local_7fc.x = (vec_t)&local_86c;
    local_7fc.y = (vec_t)&local_860;
    local_7fc.z = (vec_t)&local_854;
    local_7f0 = &local_848;
    puVar24 = *(undefined4 **)(unaff_EBX + 0x4fbef3);
    local_6c0 = local_848;
    local_6bc = local_844;
    local_6b8 = local_840;
    local_6b4 = local_854;
    local_6b0 = local_850;
    local_6ac = local_84c;
    local_6a8 = local_860;
    local_6a4 = local_85c;
    local_6a0 = local_858;
    local_69c = local_86c.x;
    local_698 = local_86c.y;
    local_694 = local_86c.z;
    local_80c[0] = (float)(**(code **)(*(int *)*puVar24 + 8))((int *)*puVar24,&local_7fc,4);
  }
  local_908 = &local_86c;
  pCVar18 = (CPhysCollide *)(**(code **)(*(int *)*puVar24 + 0x3c))((int *)*puVar24,local_80c,1);
  this->m_pWallCollideable = pCVar18;
  if (this->m_pWallCollideable == (CPhysCollide *)0x0) goto LAB_0074b81c;
  V_strncpy(local_660.surfaceprop,(char *)(unaff_EBX + 0x310181),0x200);
  puVar24 = *(undefined4 **)(unaff_EBX + 0x4fc01f);
  poVar26 = &local_660.params;
  for (iVar25 = 0xb; iVar25 != 0; iVar25 = iVar25 + -1) {
    poVar26->massCenterOverride = (Vector *)*puVar24;
    puVar24 = puVar24 + 1;
    poVar26 = (objectparams_t *)&poVar26->mass;
  }
  local_660.params.pGameData = this;
  pPhysics = PhysModelCreateCustom
                       ((CBaseEntity *)this,this->m_pWallCollideable,
                        *(Vector **)(unaff_EBX + 0x4fbdc7),*(QAngle **)(unaff_EBX + 0x4fbe57),
                        (char *)(unaff_EBX + 0x310181),true,&local_660);
  if (pPhysics == (IPhysicsObject *)0x0) goto LAB_0074b81c;
  if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
      super_CBaseEntity.m_pPhysicsObject != (IPhysicsObject *)0x0) {
    (*(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
      super_CBaseEntity.super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
      _vptr_IHandleEntity[0xa1])(this);
  }
  CBaseEntity::VPhysicsSetObject((CBaseEntity *)this,pPhysics);
  iVar25 = (*pPhysics->_vptr_IPhysicsObject[0x4b])(pPhysics);
  if (iVar25 == 0) goto LAB_0074b81c;
  pfVar17 = *(float **)(unaff_EBX + 0x4fbdc7);
  local_75c = *pfVar17;
  local_758 = pfVar17[1];
  local_754 = pfVar17[2];
  pcVar12 = *(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4fbef3) + 0x60);
  local_750 = local_75c;
  local_74c = local_758;
  local_748 = local_754;
  iVar25 = (*pPhysics->_vptr_IPhysicsObject[0x4b])(pPhysics);
  (*pcVar12)(**(undefined4 **)(unaff_EBX + 0x4fbef3),&local_750,&local_75c,iVar25,
             *(undefined4 *)(unaff_EBX + 0x4fbdc7),*(undefined4 *)(unaff_EBX + 0x4fbe57));
  local_7e0 = local_750;
  local_7dc = local_74c;
  local_7d8 = local_748;
  if ((((local_750 != (this->m_vWorldSpace_WallMins).m_Value.x) ||
       (NAN(local_750) || NAN((this->m_vWorldSpace_WallMins).m_Value.x))) ||
      (pvVar3 = &(this->m_vWorldSpace_WallMins).m_Value.y, local_74c != *pvVar3)) ||
     ((NAN(local_74c) || NAN(*pvVar3) || (local_748 != (this->m_vWorldSpace_WallMins).m_Value.z))))
  {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar13 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                  super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar13 != (CBaseEdict *)0x0) && ((pCVar13->m_fStateFlags & 0x100U) == 0)) {
        pCVar13->m_fStateFlags = pCVar13->m_fStateFlags | 1;
        pIVar21 = CBaseEdict::GetChangeAccessor(pCVar13);
        puVar14 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4fbdb7);
        if (pIVar21->m_iChangeInfoSerialNumber == *puVar14) {
          uVar22 = (uint)pIVar21->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar11 = puVar14[uVar22 * 0x14 + 0x14];
          if (uVar11 == 0) {
LAB_0074c180:
            puVar14[(uint)uVar11 + uVar22 * 0x14 + 1] = 0x428;
            puVar14[uVar22 * 0x14 + 0x14] = uVar11 + 1;
          }
          else if (puVar14[uVar22 * 0x14 + 1] != 0x428) {
            local_968 = 0;
            do {
              uVar23 = local_968 + 1;
              local_968 = uVar23 & 0xffff;
              if ((ushort)uVar23 == uVar11) {
                if (uVar11 != 0x13) goto LAB_0074c180;
                pIVar21->m_iChangeInfoSerialNumber = 0;
                pCVar13->m_fStateFlags = pCVar13->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar14[uVar22 * 0x14 + local_968 + 1] != 0x428);
          }
        }
        else if (puVar14[0x7d1] == 100) {
          pIVar21->m_iChangeInfoSerialNumber = 0;
          pCVar13->m_fStateFlags = pCVar13->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar21->m_iChangeInfo = puVar14[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4fbdb7) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4fbdb7) + 0xfa2) + 1;
          piVar15 = *(int **)(unaff_EBX + 0x4fbdb7);
          pIVar21->m_iChangeInfoSerialNumber = *(ushort *)*piVar15;
          iVar25 = *piVar15 + (uint)pIVar21->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar25 + 2) = 0x428;
          *(undefined2 *)(iVar25 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_vWorldSpace_WallMins).m_Value.x = local_7e0;
    (this->m_vWorldSpace_WallMins).m_Value.y = local_7dc;
    (this->m_vWorldSpace_WallMins).m_Value.z = local_7d8;
  }
  local_7ec = local_75c;
  local_7e8 = local_758;
  local_7e4 = local_754;
  if ((((local_75c != (this->m_vWorldSpace_WallMaxs).m_Value.x) ||
       (NAN(local_75c) || NAN((this->m_vWorldSpace_WallMaxs).m_Value.x))) ||
      (pvVar3 = &(this->m_vWorldSpace_WallMaxs).m_Value.y, local_758 != *pvVar3)) ||
     ((NAN(local_758) || NAN(*pvVar3) || (local_754 != (this->m_vWorldSpace_WallMaxs).m_Value.z))))
  {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar13 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                  super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar13 != (CBaseEdict *)0x0) && ((pCVar13->m_fStateFlags & 0x100U) == 0)) {
        pCVar13->m_fStateFlags = pCVar13->m_fStateFlags | 1;
        pIVar21 = CBaseEdict::GetChangeAccessor(pCVar13);
        puVar14 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4fbdb7);
        if (pIVar21->m_iChangeInfoSerialNumber == *puVar14) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar22 = (uint)pIVar21->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar11 = puVar14[uVar22 * 0x14 + 0x14];
          if (uVar11 == 0) {
LAB_0074c0b0:
            puVar14[(uint)uVar11 + uVar22 * 0x14 + 1] = 0x434;
            puVar14[uVar22 * 0x14 + 0x14] = uVar11 + 1;
          }
          else if (puVar14[uVar22 * 0x14 + 1] != 0x434) {
            local_964 = 0;
            do {
              uVar23 = local_964 + 1;
              local_964 = uVar23 & 0xffff;
              if ((ushort)uVar23 == uVar11) {
                if (uVar11 == 0x13) goto LAB_0074c491;
                goto LAB_0074c0b0;
              }
            } while (puVar14[uVar22 * 0x14 + local_964 + 1] != 0x434);
          }
        }
        else if (puVar14[0x7d1] == 100) {
LAB_0074c491:
          pIVar21->m_iChangeInfoSerialNumber = 0;
          pCVar13->m_fStateFlags = pCVar13->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar21->m_iChangeInfo = puVar14[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4fbdb7) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4fbdb7) + 0xfa2) + 1;
          piVar15 = *(int **)(unaff_EBX + 0x4fbdb7);
          pIVar21->m_iChangeInfoSerialNumber = *(ushort *)*piVar15;
          iVar25 = *piVar15 + (uint)pIVar21->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar25 + 2) = 0x434;
          *(undefined2 *)(iVar25 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_vWorldSpace_WallMaxs).m_Value.x = local_7ec;
    (this->m_vWorldSpace_WallMaxs).m_Value.y = local_7e8;
    (this->m_vWorldSpace_WallMaxs).m_Value.z = local_7e4;
  }
                    /* Unresolved local var: Vector vSetMins@[???]
                       Unresolved local var: Vector vSetMaxs@[???]
                       Unresolved local var: Vector res@[???] */
  local_86c.x = local_750 - fVar6;
  local_86c.y = local_74c - fVar5;
  local_86c.z = local_748 - fVar28;
                    /* Unresolved local var: Vector res@[???] */
  local_7fc.x = local_75c - fVar6;
  local_7fc.y = local_758 - fVar5;
  local_7fc.z = local_754 - fVar28;
  local_774 = (Vector *)local_7fc.x;
  local_770 = (float *)local_7fc.y;
  local_76c = (float *)local_7fc.z;
  local_768 = local_86c.x;
  local_764 = local_86c.y;
  local_760 = local_86c.z;
  CBaseEntity::SetSize((CBaseEntity *)this,local_908,&local_7fc);
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: __m128 root@[???] */
  fVar28 = SQRT((fVar6 - fVar9) * (fVar6 - fVar9) + (fVar5 - fVar8) * (fVar5 - fVar8) +
                (fVar28 - fVar7) * (fVar28 - fVar7));
  local_80c[1] = 0.0;
  local_80c[2] = 0.0;
  local_80c[3] = 0.0;
  local_80c[0] = fVar28;
  if (fVar28 != (this->m_flLength).m_Value) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar13 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                  super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar13 != (CBaseEdict *)0x0) && ((pCVar13->m_fStateFlags & 0x100U) == 0)) {
        pCVar13->m_fStateFlags = pCVar13->m_fStateFlags | 1;
        pIVar21 = CBaseEdict::GetChangeAccessor(pCVar13);
        piVar15 = *(int **)(unaff_EBX + 0x4fbdb7);
        puVar14 = (ushort *)*piVar15;
        if (pIVar21->m_iChangeInfoSerialNumber == *puVar14) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar22 = (uint)pIVar21->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar11 = puVar14[uVar22 * 0x14 + 0x14];
          if (uVar11 == 0) {
LAB_0074c83a:
            puVar14[(uint)uVar11 + uVar22 * 0x14 + 1] = 0x444;
            puVar14[uVar22 * 0x14 + 0x14] = uVar11 + 1;
          }
          else if (puVar14[uVar22 * 0x14 + 1] != 0x444) {
            uVar23 = 0;
            do {
              uVar2 = uVar23 + 1;
              uVar23 = uVar2 & 0xffff;
              if ((ushort)uVar2 == uVar11) {
                if (uVar11 == 0x13) goto LAB_0074c87b;
                goto LAB_0074c83a;
              }
            } while (puVar14[uVar22 * 0x14 + uVar23 + 1] != 0x444);
          }
        }
        else if (puVar14[0x7d1] == 100) {
LAB_0074c87b:
          pIVar21->m_iChangeInfoSerialNumber = 0;
          pCVar13->m_fStateFlags = pCVar13->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar21->m_iChangeInfo = puVar14[0x7d1];
          *(short *)(*piVar15 + 0xfa2) = *(short *)(*piVar15 + 0xfa2) + 1;
          pIVar21->m_iChangeInfoSerialNumber = *(ushort *)*piVar15;
          iVar25 = *piVar15 + (uint)pIVar21->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar25 + 2) = 0x444;
          *(undefined2 *)(iVar25 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flLength).m_Value = fVar28;
  }
  if (*(float *)(unaff_EBX + 0x381e6b) != (this->m_flWidth).m_Value) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar13 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                  super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar13 != (CBaseEdict *)0x0) && ((pCVar13->m_fStateFlags & 0x100U) == 0)) {
        pCVar13->m_fStateFlags = pCVar13->m_fStateFlags | 1;
        pIVar21 = CBaseEdict::GetChangeAccessor(pCVar13);
        piVar15 = *(int **)(unaff_EBX + 0x4fbdb7);
        puVar14 = (ushort *)*piVar15;
        if (pIVar21->m_iChangeInfoSerialNumber == *puVar14) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar22 = (uint)pIVar21->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar11 = puVar14[uVar22 * 0x14 + 0x14];
          if (uVar11 == 0) {
LAB_0074c6f2:
            puVar14[(uint)uVar11 + uVar22 * 0x14 + 1] = 0x44c;
            puVar14[uVar22 * 0x14 + 0x14] = uVar11 + 1;
          }
          else if (puVar14[uVar22 * 0x14 + 1] != 0x44c) {
            uVar23 = 0;
            do {
              uVar2 = uVar23 + 1;
              uVar23 = uVar2 & 0xffff;
              if ((ushort)uVar2 == uVar11) {
                if (uVar11 == 0x13) goto LAB_0074c892;
                goto LAB_0074c6f2;
              }
            } while (puVar14[uVar22 * 0x14 + uVar23 + 1] != 0x44c);
          }
        }
        else if (puVar14[0x7d1] == 100) {
LAB_0074c892:
          pIVar21->m_iChangeInfoSerialNumber = 0;
          pCVar13->m_fStateFlags = pCVar13->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar21->m_iChangeInfo = puVar14[0x7d1];
          *(short *)(*piVar15 + 0xfa2) = *(short *)(*piVar15 + 0xfa2) + 1;
          pIVar21->m_iChangeInfoSerialNumber = *(ushort *)*piVar15;
          iVar25 = *piVar15 + (uint)pIVar21->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar25 + 2) = 0x44c;
          *(undefined2 *)(iVar25 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flWidth).m_Value = 64.0;
  }
  if (*(float *)(unaff_EBX + 0x3821e7) != (this->m_flHeight).m_Value) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar13 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                  super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar13 != (CBaseEdict *)0x0) && ((pCVar13->m_fStateFlags & 0x100U) == 0)) {
        pCVar13->m_fStateFlags = pCVar13->m_fStateFlags | 1;
        pIVar21 = CBaseEdict::GetChangeAccessor(pCVar13);
        piVar15 = *(int **)(unaff_EBX + 0x4fbdb7);
        puVar14 = (ushort *)*piVar15;
        if (pIVar21->m_iChangeInfoSerialNumber == *puVar14) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar22 = (uint)pIVar21->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar11 = puVar14[uVar22 * 0x14 + 0x14];
          if (uVar11 == 0) {
LAB_0074c798:
            puVar14[(uint)uVar11 + uVar22 * 0x14 + 1] = 0x448;
            puVar14[uVar22 * 0x14 + 0x14] = uVar11 + 1;
          }
          else if (puVar14[uVar22 * 0x14 + 1] != 0x448) {
            uVar23 = 0;
            do {
              uVar2 = uVar23 + 1;
              uVar23 = uVar2 & 0xffff;
              if ((ushort)uVar2 == uVar11) {
                if (uVar11 == 0x13) goto LAB_0074c864;
                goto LAB_0074c798;
              }
            } while (puVar14[uVar22 * 0x14 + uVar23 + 1] != 0x448);
          }
        }
        else if (puVar14[0x7d1] == 100) {
LAB_0074c864:
          pIVar21->m_iChangeInfoSerialNumber = 0;
          pCVar13->m_fStateFlags = pCVar13->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar21->m_iChangeInfo = puVar14[0x7d1];
          *(short *)(*piVar15 + 0xfa2) = *(short *)(*piVar15 + 0xfa2) + 1;
          pIVar21->m_iChangeInfoSerialNumber = *(ushort *)*piVar15;
          iVar25 = *piVar15 + (uint)pIVar21->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar25 + 2) = 0x448;
          *(undefined2 *)(iVar25 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flHeight).m_Value = 0.015625;
  }
  this_01 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
             super_CBaseEntity.m_Collision;
  CCollisionProperty::MarkSurroundingBoundsDirty(&this_01->super_CCollisionProperty);
  CCollisionProperty::MarkPartitionHandleDirty(&this_01->super_CCollisionProperty);
  CCollisionProperty::UpdatePartition(&this_01->super_CCollisionProperty);
  if (((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
       super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
  AngleVectors(&(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_angAbsRotation,(Vector *)0x0,&local_78c,&local_780);
  if (*(float *)(unaff_EBX + 0x381f1b) < local_780.z) {
    fVar28 = *(float *)(unaff_EBX + 0x382fbb);
LAB_0074b853:
    if (local_78c.z <= fVar28) goto LAB_0074b6cc;
    local_8e5 = local_78c.z < *(float *)(unaff_EBX + 0x381f1b);
  }
  else {
    fVar28 = *(float *)(unaff_EBX + 0x382fbb);
    if (local_780.z < fVar28) goto LAB_0074b853;
LAB_0074b6cc:
    local_8e5 = false;
  }
  if ((this->m_bIsHorizontal).m_Value != local_8e5) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar13 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                  super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar13 != (CBaseEdict *)0x0) && ((pCVar13->m_fStateFlags & 0x100U) == 0)) {
        pCVar13->m_fStateFlags = pCVar13->m_fStateFlags | 1;
        pIVar21 = CBaseEdict::GetChangeAccessor(pCVar13);
        piVar15 = *(int **)(unaff_EBX + 0x4fbdb7);
        puVar14 = (ushort *)*piVar15;
        if (pIVar21->m_iChangeInfoSerialNumber == *puVar14) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar22 = (uint)pIVar21->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar11 = puVar14[uVar22 * 0x14 + 0x14];
          if (uVar11 == 0) {
LAB_0074c602:
            puVar14[(uint)uVar11 + uVar22 * 0x14 + 1] = 0x450;
            puVar14[uVar22 * 0x14 + 0x14] = uVar11 + 1;
          }
          else if (puVar14[uVar22 * 0x14 + 1] != 0x450) {
            uVar23 = 0;
            do {
              uVar2 = uVar23 + 1;
              uVar23 = uVar2 & 0xffff;
              if ((ushort)uVar2 == uVar11) {
                if (uVar11 == 0x13) goto LAB_0074c62c;
                goto LAB_0074c602;
              }
            } while (puVar14[uVar22 * 0x14 + uVar23 + 1] != 0x450);
          }
        }
        else if (puVar14[0x7d1] == 100) {
LAB_0074c62c:
          pIVar21->m_iChangeInfoSerialNumber = 0;
          pCVar13->m_fStateFlags = pCVar13->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar21->m_iChangeInfo = puVar14[0x7d1];
          *(short *)(*piVar15 + 0xfa2) = *(short *)(*piVar15 + 0xfa2) + 1;
          pIVar21->m_iChangeInfoSerialNumber = *(ushort *)*piVar15;
          iVar25 = *piVar15 + (uint)pIVar21->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar25 + 2) = 0x450;
          *(undefined2 *)(iVar25 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bIsHorizontal).m_Value = local_8e5;
  }
  DisplaceObstructingEntities(this);
  lVar27 = (longdouble)_ceilf((this->m_flLength).m_Value / (this->m_flSegmentLength).m_Value);
  iVar25 = (int)lVar27;
  if (iVar25 != (this->m_nNumSegments).m_Value) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar13 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                  super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar13 != (CBaseEdict *)0x0) && ((pCVar13->m_fStateFlags & 0x100U) == 0)) {
        pCVar13->m_fStateFlags = pCVar13->m_fStateFlags | 1;
        pIVar21 = CBaseEdict::GetChangeAccessor(pCVar13);
        puVar14 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4fbdb7);
        if (pIVar21->m_iChangeInfoSerialNumber == *puVar14) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar22 = (uint)pIVar21->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar11 = puVar14[uVar22 * 0x14 + 0x14];
          if (uVar11 == 0) {
LAB_0074c550:
            puVar14[(uint)uVar11 + uVar22 * 0x14 + 1] = 0x454;
            puVar14[uVar22 * 0x14 + 0x14] = uVar11 + 1;
          }
          else if (puVar14[uVar22 * 0x14 + 1] != 0x454) {
            local_960 = 0;
            do {
              uVar23 = local_960 + 1;
              local_960 = uVar23 & 0xffff;
              if ((ushort)uVar23 == uVar11) {
                if (uVar11 == 0x13) goto LAB_0074c65a;
                goto LAB_0074c550;
              }
            } while (puVar14[uVar22 * 0x14 + local_960 + 1] != 0x454);
          }
        }
        else if (puVar14[0x7d1] == 100) {
LAB_0074c65a:
          pIVar21->m_iChangeInfoSerialNumber = 0;
          pCVar13->m_fStateFlags = pCVar13->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar21->m_iChangeInfo = puVar14[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4fbdb7) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4fbdb7) + 0xfa2) + 1;
          piVar15 = *(int **)(unaff_EBX + 0x4fbdb7);
          pIVar21->m_iChangeInfoSerialNumber = *(ushort *)*piVar15;
          iVar19 = *piVar15 + (uint)pIVar21->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar19 + 2) = 0x454;
          *(undefined2 *)(iVar19 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_nNumSegments).m_Value = iVar25;
  }
  (this->m_PaintPowers).m_Size = 0;
  if (iVar25 != 0) {
    iVar19 = (this->m_PaintPowers).m_Memory.m_nAllocationCount;
    if (iVar19 < iVar25) {
      CUtlMemory<PaintPowerType,int>::Grow(&(this->m_PaintPowers).m_Memory,iVar25 - iVar19);
    }
    piVar15 = &(this->m_PaintPowers).m_Size;
    *piVar15 = *piVar15 + iVar25;
    src = (this->m_PaintPowers).m_Memory.m_pMemory;
    (this->m_PaintPowers).m_pElements = src;
                    /* Unresolved local var: int numToMove@[???] */
    iVar19 = (this->m_PaintPowers).m_Size - iVar25;
    if ((0 < iVar19) && (0 < iVar25)) {
      _V_memmove(src + iVar25,src,iVar19 * 4);
    }
  }
  (*(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
    super_CBaseEntity.super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0xd6])(this);
LAB_0074b81c:
  if (local_20 != **(int **)(unaff_EBX + 0x4fbdff)) {
                    /* WARNING: Subroutine does not return */
    ___stack_chk_fail();
  }
  return;
}


/* CProjectedWallEntity::CleanupWall at 00749200 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Restarted to delay deadcode elimination for space: stack */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void CleanupWall(CProjectedWallEntity * this) */

void __thiscall CProjectedWallEntity::CleanupWall(CProjectedWallEntity *this)

{
  undefined1 *puVar1;
  vec_t *pvVar2;
  ushort uVar3;
  float *pfVar4;
  float fVar5;
  CBaseEdict *pCVar6;
  ushort *puVar7;
  int *piVar8;
  int iVar9;
  IChangeInfoAccessor *pIVar10;
  uint uVar11;
  uint uVar12;
  int unaff_EBX;
  float fVar13;
  float fVar14;
  uint local_114;
  uint local_110;
  uint local_10c;
  uint local_108;
  uint local_104;
  uint local_100;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_pWallCollideable != (CPhysCollide *)0x0) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4fda93) + 0x44))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4fda93),this->m_pWallCollideable);
    this->m_pWallCollideable = (CPhysCollide *)0x0;
  }
  if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
      super_CBaseEntity.m_pPhysicsObject != (IPhysicsObject *)0x0) {
    (*(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
      super_CBaseEntity.super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
      _vptr_IHandleEntity[0xa1])(this);
  }
  pfVar4 = *(float **)(unaff_EBX + 0x4fd967);
  fVar13 = *pfVar4;
  fVar14 = pfVar4[1];
  fVar5 = pfVar4[2];
  if ((((fVar13 != (this->m_vWorldSpace_WallMins).m_Value.x) ||
       (NAN(fVar13) || NAN((this->m_vWorldSpace_WallMins).m_Value.x))) ||
      (pvVar2 = &(this->m_vWorldSpace_WallMins).m_Value.y, fVar14 != *pvVar2)) ||
     ((NAN(fVar14) || NAN(*pvVar2) || (fVar5 != (this->m_vWorldSpace_WallMins).m_Value.z)))) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar6 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                 super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar6 != (CBaseEdict *)0x0) && ((pCVar6->m_fStateFlags & 0x100U) == 0)) {
        pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar6);
        puVar7 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4fd957);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar7) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar7[uVar11 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00749bf0:
            puVar7[(uint)uVar3 + uVar11 * 0x14 + 1] = 0x428;
            puVar7[uVar11 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar7[uVar11 * 0x14 + 1] != 0x428) {
            local_114 = 0;
            do {
              uVar12 = local_114 + 1;
              local_114 = uVar12 & 0xffff;
              if ((ushort)uVar12 == uVar3) {
                if (uVar3 == 0x13) goto LAB_00749cba;
                goto LAB_00749bf0;
              }
            } while (puVar7[uVar11 * 0x14 + local_114 + 1] != 0x428);
          }
        }
        else if (puVar7[0x7d1] == 100) {
LAB_00749cba:
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar7[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4fd957) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4fd957) + 0xfa2) + 1;
          piVar8 = *(int **)(unaff_EBX + 0x4fd957);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar8;
          iVar9 = *piVar8 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar9 + 2) = 0x428;
          *(undefined2 *)(iVar9 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_vWorldSpace_WallMins).m_Value.x = fVar13;
    (this->m_vWorldSpace_WallMins).m_Value.y = fVar14;
    (this->m_vWorldSpace_WallMins).m_Value.z = fVar5;
  }
  fVar13 = (this->m_vWorldSpace_WallMins).m_Value.x;
  fVar14 = (this->m_vWorldSpace_WallMins).m_Value.y;
  fVar5 = (this->m_vWorldSpace_WallMins).m_Value.z;
  if ((((fVar13 != (this->m_vWorldSpace_WallMaxs).m_Value.x) ||
       (NAN(fVar13) || NAN((this->m_vWorldSpace_WallMaxs).m_Value.x))) ||
      (pvVar2 = &(this->m_vWorldSpace_WallMaxs).m_Value.y, fVar14 != *pvVar2)) ||
     ((NAN(fVar14) || NAN(*pvVar2) || (fVar5 != (this->m_vWorldSpace_WallMaxs).m_Value.z)))) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar6 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                 super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar6 != (CBaseEdict *)0x0) && ((pCVar6->m_fStateFlags & 0x100U) == 0)) {
        pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar6);
        puVar7 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4fd957);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar7) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar7[uVar11 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00749b1d:
            puVar7[(uint)uVar3 + uVar11 * 0x14 + 1] = 0x434;
            puVar7[uVar11 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar7[uVar11 * 0x14 + 1] != 0x434) {
            local_110 = 0;
            do {
              uVar12 = local_110 + 1;
              local_110 = uVar12 & 0xffff;
              if ((ushort)uVar12 == uVar3) {
                if (uVar3 == 0x13) goto LAB_00749cd1;
                goto LAB_00749b1d;
              }
            } while (puVar7[uVar11 * 0x14 + local_110 + 1] != 0x434);
          }
        }
        else if (puVar7[0x7d1] == 100) {
LAB_00749cd1:
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar7[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4fd957) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4fd957) + 0xfa2) + 1;
          piVar8 = *(int **)(unaff_EBX + 0x4fd957);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar8;
          iVar9 = *piVar8 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar9 + 2) = 0x434;
          *(undefined2 *)(iVar9 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_vWorldSpace_WallMaxs).m_Value.x = fVar13;
    (this->m_vWorldSpace_WallMaxs).m_Value.y = fVar14;
    (this->m_vWorldSpace_WallMaxs).m_Value.z = fVar5;
  }
  fVar13 = (this->m_flHeight).m_Value;
  if (fVar13 != 0.0) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar6 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                 super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar6 != (CBaseEdict *)0x0) && ((pCVar6->m_fStateFlags & 0x100U) == 0)) {
        pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar6);
        puVar7 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4fd957);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar7) {
          uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar7[uVar11 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_007498da:
            puVar7[(uint)uVar3 + uVar11 * 0x14 + 1] = 0x448;
            puVar7[uVar11 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar7[uVar11 * 0x14 + 1] != 0x448) {
            local_10c = 0;
            do {
              uVar12 = local_10c + 1;
              local_10c = uVar12 & 0xffff;
              if ((ushort)uVar12 == uVar3) {
                if (uVar3 != 0x13) goto LAB_007498da;
                pIVar10->m_iChangeInfoSerialNumber = 0;
                pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar7[uVar11 * 0x14 + local_10c + 1] != 0x448);
          }
        }
        else if (puVar7[0x7d1] == 100) {
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar7[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4fd957) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4fd957) + 0xfa2) + 1;
          piVar8 = *(int **)(unaff_EBX + 0x4fd957);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar8;
          iVar9 = *piVar8 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar9 + 2) = 0x448;
          *(undefined2 *)(iVar9 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flHeight).m_Value = 0.0;
    fVar13 = 0.0;
  }
  fVar14 = (this->m_flWidth).m_Value;
  if (fVar14 != fVar13) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar6 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                 super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar6 != (CBaseEdict *)0x0) && ((pCVar6->m_fStateFlags & 0x100U) == 0)) {
        pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar6);
        puVar7 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4fd957);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar7) {
          uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar7[uVar11 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_0074999a:
            puVar7[(uint)uVar3 + uVar11 * 0x14 + 1] = 0x44c;
            puVar7[uVar11 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar7[uVar11 * 0x14 + 1] != 0x44c) {
            local_108 = 0;
            do {
              uVar12 = local_108 + 1;
              local_108 = uVar12 & 0xffff;
              if ((ushort)uVar12 == uVar3) {
                if (uVar3 != 0x13) goto LAB_0074999a;
                pIVar10->m_iChangeInfoSerialNumber = 0;
                pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar7[uVar11 * 0x14 + local_108 + 1] != 0x44c);
          }
        }
        else if (puVar7[0x7d1] == 100) {
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar7[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4fd957) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4fd957) + 0xfa2) + 1;
          piVar8 = *(int **)(unaff_EBX + 0x4fd957);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar8;
          iVar9 = *piVar8 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar9 + 2) = 0x44c;
          *(undefined2 *)(iVar9 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flWidth).m_Value = fVar13;
    fVar14 = fVar13;
  }
  if (fVar14 != (this->m_flLength).m_Value) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar6 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                 super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar6 != (CBaseEdict *)0x0) && ((pCVar6->m_fStateFlags & 0x100U) == 0)) {
        pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar6);
        puVar7 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4fd957);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar7) {
          uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar7[uVar11 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00749a5a:
            puVar7[(uint)uVar3 + uVar11 * 0x14 + 1] = 0x444;
            puVar7[uVar11 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar7[uVar11 * 0x14 + 1] != 0x444) {
            local_104 = 0;
            do {
              uVar12 = local_104 + 1;
              local_104 = uVar12 & 0xffff;
              if ((ushort)uVar12 == uVar3) {
                if (uVar3 != 0x13) goto LAB_00749a5a;
                pIVar10->m_iChangeInfoSerialNumber = 0;
                pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar7[uVar11 * 0x14 + local_104 + 1] != 0x444);
          }
        }
        else if (puVar7[0x7d1] == 100) {
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar7[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4fd957) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4fd957) + 0xfa2) + 1;
          piVar8 = *(int **)(unaff_EBX + 0x4fd957);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar8;
          iVar9 = *piVar8 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar9 + 2) = 0x444;
          *(undefined2 *)(iVar9 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flLength).m_Value = fVar14;
  }
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
  uVar11 = (this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
           m_hHitPortal.
           super_CNetworkVarBase<CBaseHandle,CBaseProjectedEntity::NetworkVar_m_hHitPortal>.m_Value.
           m_Index;
  if (((uVar11 == 0xffffffff) ||
      (iVar9 = (uVar11 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4fd95b),
      *(uint *)(iVar9 + 8) != uVar11 >> 0x10)) || (*(int *)(iVar9 + 4) == 0)) {
    return;
  }
  if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
      super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
    pCVar6 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
               super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if ((pCVar6 != (CBaseEdict *)0x0) && ((pCVar6->m_fStateFlags & 0x100U) == 0)) {
      pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 1;
      pIVar10 = CBaseEdict::GetChangeAccessor(pCVar6);
      puVar7 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4fd957);
      if (pIVar10->m_iChangeInfoSerialNumber == *puVar7) {
        uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar3 = puVar7[uVar11 * 0x14 + 0x14];
        if (uVar3 == 0) {
LAB_00749dad:
          puVar7[(uint)uVar3 + uVar11 * 0x14 + 1] = 0x3c0;
          puVar7[uVar11 * 0x14 + 0x14] = uVar3 + 1;
        }
        else if (puVar7[uVar11 * 0x14 + 1] != 0x3c0) {
          local_100 = 0;
          do {
            uVar12 = local_100 + 1;
            local_100 = uVar12 & 0xffff;
            if ((ushort)uVar12 == uVar3) {
              if (uVar3 != 0x13) goto LAB_00749dad;
              pIVar10->m_iChangeInfoSerialNumber = 0;
              pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 0x100;
              break;
            }
          } while (puVar7[uVar11 * 0x14 + local_100 + 1] != 0x3c0);
        }
      }
      else if (puVar7[0x7d1] == 100) {
        pIVar10->m_iChangeInfoSerialNumber = 0;
        pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar10->m_iChangeInfo = puVar7[0x7d1];
        *(short *)(**(int **)(unaff_EBX + 0x4fd957) + 0xfa2) =
             *(short *)(**(int **)(unaff_EBX + 0x4fd957) + 0xfa2) + 1;
        piVar8 = *(int **)(unaff_EBX + 0x4fd957);
        pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar8;
        iVar9 = *piVar8 + (uint)pIVar10->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar9 + 2) = 0x3c0;
        *(undefined2 *)(iVar9 + 0x28) = 1;
      }
    }
  }
  else {
    puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
              super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  (this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.m_hHitPortal.
  super_CNetworkVarBase<CBaseHandle,CBaseProjectedEntity::NetworkVar_m_hHitPortal>.m_Value.m_Index =
       0xffffffff;
  return;
}


/* CProjectedWallEntity::GetProjectionExtents at 007489f0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void GetProjectionExtents(CProjectedWallEntity * this, Vector *
   outMins, Vector * outMaxs) */

void __thiscall
CProjectedWallEntity::GetProjectionExtents
          (CProjectedWallEntity *this,Vector *outMins,Vector *outMaxs)

{
  GetExtents(this,outMins,outMaxs,0.5);
  return;
}


/* CProjectedWallEntity::CreateVPhysics at 0074c8c0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: bool CreateVPhysics(CProjectedWallEntity * this) */

bool __thiscall CProjectedWallEntity::CreateVPhysics(CProjectedWallEntity *this)

{
  ProjectWall(this);
  return true;
}


/* CProjectedWallEntity::TestCollision at 00748740 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: bool TestCollision(CProjectedWallEntity * this, Ray_t * ray, uint mask,
   trace_t * trace) */

bool __thiscall
CProjectedWallEntity::TestCollision(CProjectedWallEntity *this,Ray_t *ray,uint mask,trace_t *trace)

{
  float *pfVar1;
  undefined1 uVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  uVar2 = 0;
  if (this->m_pWallCollideable != (CPhysCollide *)0x0) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4fe550) + 0x88))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4fe550),ray,mask,0,this->m_pWallCollideable,
               *(undefined4 *)(unaff_EBX + 0x4fe424),*(undefined4 *)(&DAT_004fe4b4 + unaff_EBX),
               trace);
    pfVar1 = &(trace->super_CBaseTrace).fraction;
    if ((*pfVar1 <= *(float *)(unaff_EBX + 0x3841bc) && *(float *)(unaff_EBX + 0x3841bc) != *pfVar1)
       || ((trace->super_CBaseTrace).allsolid != false)) {
      uVar2 = 1;
    }
    else {
      uVar2 = (trace->super_CBaseTrace).startsolid;
    }
  }
  return (bool)uVar2;
}


/* CProjectedWallEntity::ComputeWorldSpaceSurroundingBox at 00748c00 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void ComputeWorldSpaceSurroundingBox(CProjectedWallEntity * this,
   Vector * pWorldMins, Vector * pWorldMaxs) */

void __thiscall
CProjectedWallEntity::ComputeWorldSpaceSurroundingBox
          (CProjectedWallEntity *this,Vector *pWorldMins,Vector *pWorldMaxs)

{
  vec_t vVar1;
  vec_t vVar2;
  vec_t vVar3;
  vec_t vVar4;
  vec_t vVar5;
  
                    /* Unresolved local var: Vector vMins@[???]
                       Unresolved local var: Vector vMaxs@[???] */
  vVar1 = (this->m_vWorldSpace_WallMins).m_Value.y;
  vVar2 = (this->m_vWorldSpace_WallMins).m_Value.z;
  vVar3 = (this->m_vWorldSpace_WallMaxs).m_Value.x;
  vVar4 = (this->m_vWorldSpace_WallMaxs).m_Value.y;
  vVar5 = (this->m_vWorldSpace_WallMaxs).m_Value.z;
  pWorldMins->x = (this->m_vWorldSpace_WallMins).m_Value.x;
  pWorldMins->y = vVar1;
  pWorldMins->z = vVar2;
  pWorldMaxs->x = vVar3;
  pWorldMaxs->y = vVar4;
  pWorldMaxs->z = vVar5;
  return;
}


/* CProjectedWallEntity::DrawWallBoundsHighlight at 00748900 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void DrawWallBoundsHighlight(CProjectedWallEntity * this) */

void __thiscall CProjectedWallEntity::DrawWallBoundsHighlight(CProjectedWallEntity *this)

{
  int iVar1;
  int unaff_EBX;
  int iVar2;
  int iVar3;
  int vertCount;
  int triCount;
  int local_20 [4];
  
                    /* Unresolved local var: Vector * outVerts@[???]
                       Unresolved local var: int vert@[???] */
  ___i686_get_pc_thunk_bx();
  if (this->m_pWallCollideable != (CPhysCollide *)0x0) {
    iVar1 = (**(code **)(*(int *)**(undefined4 **)(&DAT_004fe396 + unaff_EBX) + 0xa8))
                      ((int *)**(undefined4 **)(&DAT_004fe396 + unaff_EBX),this->m_pWallCollideable,
                       local_20);
                    /* Unresolved local var: int i@[???] */
    if (0 < iVar1 / 3) {
      iVar3 = 0;
      iVar2 = 0;
      do {
        NDebugOverlay::Triangle
                  ((Vector *)(local_20[0] + iVar2),(Vector *)(iVar2 + 0xc + local_20[0]),
                   (Vector *)(iVar2 + 0x18 + local_20[0]),0x2a,0x52,0xbe,0x80,false,0.5);
        iVar3 = iVar3 + 1;
        iVar2 = iVar2 + 0x24;
      } while (iVar3 != iVar1 / 3);
    }
    (**(code **)(*(int *)**(undefined4 **)(&DAT_004fe396 + unaff_EBX) + 0xac))
              ((int *)**(undefined4 **)(&DAT_004fe396 + unaff_EBX),iVar1,local_20[0]);
  }
  return;
}


/* CProjectedWallEntity::NotifyPortalEvent at 007488b0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void NotifyPortalEvent(CProjectedWallEntity * this, PortalEvent_t
   nEventType, CPortal_Base2D * pNotifier) */

void __thiscall
CProjectedWallEntity::NotifyPortalEvent
          (CProjectedWallEntity *this,PortalEvent_t nEventType,CPortal_Base2D *pNotifier)

{
  ulong *puVar1;
  CBaseHandle local_10 [3];
  
  if (nEventType == PORTALEVENT_LINKED) {
    local_10[0].m_Index = 0xffffffff;
    if (this == (CProjectedWallEntity *)0x0) {
      local_10[0].m_Index = 0xffffffff;
    }
    else {
      puVar1 = (ulong *)(*(this->super_CPaintableEntity<CBaseProjectedEntity>).
                          super_CBaseProjectedEntity.super_CBaseEntity.super_IServerEntity.
                          super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[3])(this);
      local_10[0].m_Index = *puVar1;
    }
    CPortal_Base2D::RemovePortalEventListener(pNotifier,(EHANDLE *)local_10);
  }
  return;
}


/* CProjectedWallEntity::SetSourcePortal at 00748d90 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void SetSourcePortal(CProjectedWallEntity * this, CPortal_Base2D *
   pPortal) */

void __thiscall
CProjectedWallEntity::SetSourcePortal(CProjectedWallEntity *this,CPortal_Base2D *pPortal)

{
  undefined1 *puVar1;
  ushort uVar2;
  CBaseEdict *pCVar3;
  ushort *puVar4;
  int *piVar5;
  int iVar6;
  uint *puVar7;
  int iVar8;
  IChangeInfoAccessor *pIVar9;
  uint uVar10;
  uint uVar11;
  int unaff_EBX;
  float fVar12;
  uint local_94;
  uint local_90;
  uint local_20;
  
  ___i686_get_pc_thunk_bx();
  if (pPortal == (CPortal_Base2D *)0x0) {
    iVar6 = (*(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
              super_CBaseEntity.super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[0xc9])(this);
    if (iVar6 == 0) goto joined_r0x00748dd5;
  }
  else {
    iVar6 = (*(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
              super_CBaseEntity.super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[0xca])(this,pPortal);
    if ((char)iVar6 == '\0') goto joined_r0x00748dd5;
  }
  fVar12 = *(float *)(&DAT_00383d5f + unaff_EBX) +
           *(float *)(**(int **)(unaff_EBX + 0x4fddc3) + 0xc);
  if (fVar12 != (this->m_flParticleUpdateTime).m_Value) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar3 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                 super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar3 != (CBaseEdict *)0x0) && ((pCVar3->m_fStateFlags & 0x100U) == 0)) {
        pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 1;
        pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4fddc7);
        if (pIVar9->m_iChangeInfoSerialNumber == *puVar4) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar10 = (uint)pIVar9->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar4[uVar10 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_007491ad:
            puVar4[(uint)uVar2 + uVar10 * 0x14 + 1] = 0x45c;
            puVar4[uVar10 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar4[uVar10 * 0x14 + 1] != 0x45c) {
            local_94 = 0;
            do {
              uVar11 = local_94 + 1;
              local_94 = uVar11 & 0xffff;
              if ((ushort)uVar11 == uVar2) {
                if (uVar2 != 0x13) goto LAB_007491ad;
                goto LAB_007491d2;
              }
            } while (puVar4[uVar10 * 0x14 + local_94 + 1] != 0x45c);
          }
        }
        else if (puVar4[0x7d1] == 100) {
LAB_007491d2:
          pIVar9->m_iChangeInfoSerialNumber = 0;
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar9->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4fddc7) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4fddc7) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x4fddc7);
          pIVar9->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar6 = *piVar5 + (uint)pIVar9->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar6 + 2) = 0x45c;
          *(undefined2 *)(iVar6 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flParticleUpdateTime).m_Value = fVar12;
  }
joined_r0x00748dd5:
  if (pPortal == (CPortal_Base2D *)0x0) {
    local_20 = 0xffffffff;
  }
  else {
    puVar7 = (uint *)(*(pPortal->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.
                       super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[3])(pPortal);
    local_20 = *puVar7;
  }
  if ((local_20 == 0xffffffff) ||
     (iVar6 = (local_20 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4fddcb),
     *(uint *)(iVar6 + 8) != local_20 >> 0x10)) {
    iVar6 = 0;
  }
  else {
    iVar6 = *(int *)(iVar6 + 4);
  }
  uVar10 = (this->m_hColorPortal).m_Value.super_CBaseHandle.m_Index;
  if ((uVar10 == 0xffffffff) ||
     (iVar8 = (uVar10 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4fddcb),
     *(uint *)(iVar8 + 8) != uVar10 >> 0x10)) {
    iVar8 = 0;
  }
  else {
    iVar8 = *(int *)(iVar8 + 4);
  }
  if (iVar6 != iVar8) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar3 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                 super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar3 != (CBaseEdict *)0x0) && ((pCVar3->m_fStateFlags & 0x100U) == 0)) {
        pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 1;
        pIVar9 = CBaseEdict::GetChangeAccessor(pCVar3);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4fddc7);
        if (pIVar9->m_iChangeInfoSerialNumber == *puVar4) {
          uVar10 = (uint)pIVar9->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar4[uVar10 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_00749101:
            puVar4[(uint)uVar2 + uVar10 * 0x14 + 1] = 0x440;
            puVar4[uVar10 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar4[uVar10 * 0x14 + 1] != 0x440) {
            local_90 = 0;
            do {
              uVar11 = local_90 + 1;
              local_90 = uVar11 & 0xffff;
              if ((ushort)uVar11 == uVar2) {
                if (uVar2 != 0x13) goto LAB_00749101;
                pIVar9->m_iChangeInfoSerialNumber = 0;
                pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar10 * 0x14 + local_90 + 1] != 0x440);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar9->m_iChangeInfoSerialNumber = 0;
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar9->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4fddc7) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4fddc7) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x4fddc7);
          pIVar9->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar6 = *piVar5 + (uint)pIVar9->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar6 + 2) = 0x440;
          *(undefined2 *)(iVar6 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_hColorPortal).m_Value.super_CBaseHandle.m_Index = local_20;
  }
  CBaseProjectedEntity::SetSourcePortal((CBaseProjectedEntity *)this,pPortal);
  return;
}


/* CProjectedWallEntity::SetHitPortal at 0074cab0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void SetHitPortal(CProjectedWallEntity * this, CPortal_Base2D *
   pPortal) */

void __thiscall
CProjectedWallEntity::SetHitPortal(CProjectedWallEntity *this,CPortal_Base2D *pPortal)

{
  undefined1 *puVar1;
  ushort uVar2;
  CBaseEdict *this_00;
  ushort *puVar3;
  int *piVar4;
  int iVar5;
  IChangeInfoAccessor *pIVar6;
  uint uVar7;
  uint uVar8;
  int unaff_EBX;
  float fVar9;
  uint local_70;
  
  ___i686_get_pc_thunk_bx();
  if (pPortal == (CPortal_Base2D *)0x0) {
    iVar5 = (*(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
              super_CBaseEntity.super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[199])(this);
    if (iVar5 == 0) goto LAB_0074cb38;
  }
  else {
    iVar5 = (*(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
              super_CBaseEntity.super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[0xca])(this,pPortal);
    if ((char)iVar5 == '\0') goto LAB_0074cb38;
  }
  fVar9 = *(float *)(unaff_EBX + 0x380039) + *(float *)(**(int **)(unaff_EBX + 0x4fa09d) + 0xc);
  if (fVar9 != (this->m_flParticleUpdateTime).m_Value) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                  super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        puVar3 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4fa0a1);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar3) {
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar3[uVar7 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_0074cc84:
            puVar3[(uint)uVar2 + uVar7 * 0x14 + 1] = 0x45c;
            puVar3[uVar7 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar3[uVar7 * 0x14 + 1] != 0x45c) {
            local_70 = 0;
            do {
              uVar8 = local_70 + 1;
              local_70 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar2) {
                if (uVar2 != 0x13) goto LAB_0074cc84;
                pIVar6->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar3[uVar7 * 0x14 + local_70 + 1] != 0x45c);
          }
        }
        else if (puVar3[0x7d1] == 100) {
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar3[0x7d1];
          piVar4 = *(int **)(unaff_EBX + 0x4fa0a1);
          *(short *)(*piVar4 + 0xfa2) = *(short *)(*piVar4 + 0xfa2) + 1;
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar4;
          iVar5 = *piVar4 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar5 + 2) = 0x45c;
          *(undefined2 *)(iVar5 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flParticleUpdateTime).m_Value = fVar9;
  }
LAB_0074cb38:
  CBaseProjectedEntity::SetHitPortal((CBaseProjectedEntity *)this,pPortal);
  return;
}


/* CTEWallPaintedEvent::~CTEWallPaintedEvent at 0074cd40 */

/* DWARF original prototype: void ~CTEWallPaintedEvent(CTEWallPaintedEvent * this, int __in_chrg) */

void __thiscall CTEWallPaintedEvent::~CTEWallPaintedEvent(CTEWallPaintedEvent *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseTempEntity)._vptr_CBaseTempEntity =
       (_func_int_varargs **)(&UNK_00583957 + unaff_EBX);
  CBaseTempEntity::~CBaseTempEntity(&this->super_CBaseTempEntity,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CTEWallPaintedEvent::~CTEWallPaintedEvent at 0074cd80 */

/* DWARF original prototype: void ~CTEWallPaintedEvent(CTEWallPaintedEvent * this, int __in_chrg) */

void __thiscall CTEWallPaintedEvent::~CTEWallPaintedEvent(CTEWallPaintedEvent *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CBaseTempEntity)._vptr_CBaseTempEntity =
       (_func_int_varargs **)(extraout_ECX + 0x583920);
  CBaseTempEntity::~CBaseTempEntity(&this->super_CBaseTempEntity,__in_chrg);
  return;
}


/* __tcf_5 at 00a3ee80 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  *(int *)(extraout_ECX + 0x41077c) = extraout_ECX + 0x291820;
  CBaseTempEntity::~CBaseTempEntity((CBaseTempEntity *)(extraout_ECX + 0x41077c),in_stack_00000008);
  return;
}


/* ServerClassInit<DT_TEWallPaintedEvent::ignored> at 000b2070 */

int ServerClassInit<DT_TEWallPaintedEvent::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&DAT_00d9d59c)[unaff_EBX] == '\0') {
    iVar1 = ___cxa_guard_acquire(&DAT_00d9d59c + unaff_EBX);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)(g_EntListMemPool.m_BlobHead.m_Data + unaff_EBX + -0x10),
                  (char *)(unaff_EBX + 0x9a3bb0),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropEHandle((SendProp_conflict *)
                      ((int)&noclip_fixup.super_ConCommandBase.m_pszHelpString + unaff_EBX),
                      (char *)(unaff_EBX + 0xa0125c),0xc,4,-1,
                      *(SendVarProxyFn_conflict *)(&DAT_00b94b58 + unaff_EBX));
      SendPropInt((SendProp_conflict *)((int)&notarget.super_ConCommandBase.m_pNext + unaff_EBX),
                  (char *)(unaff_EBX + 0xa01266),0x10,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropInt((SendProp_conflict *)
                  ((int)&s_ColorCorrectionSystem.m_hMasterController.super_CBaseHandle.m_Index +
                  unaff_EBX),(char *)(unaff_EBX + 0xa01273),0x14,4,-1,0,(SendVarProxyFn_conflict)0x0
                  ,0x80);
      ___cxa_guard_release(&DAT_00d9d59c + unaff_EBX);
      ___cxa_atexit(unaff_EBX + 0x98cc7c,0,*(undefined4 *)(&DAT_00b94ad0 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)(unaff_EBX + 0xd175b4),
             (SendProp_conflict *)
             ((int)&noclip_fixup.super_ConCommandBase.m_pszHelpString + unaff_EBX),3,
             *(char **)(&DAT_00c1e9f4 + unaff_EBX));
  return 1;
}


/* CTEWallPaintedEvent::GetServerClass at 007487e0 */

/* DWARF original prototype: ServerClass * GetServerClass(CTEWallPaintedEvent * this) */

ServerClass * __thiscall CTEWallPaintedEvent::GetServerClass(CTEWallPaintedEvent *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x706e08);
}


/* CTEWallPaintedEvent::YouForgotToImplementOrDeclareServerClass at 007487f0 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CTEWallPaintedEvent *
   this) */

int __thiscall
CTEWallPaintedEvent::YouForgotToImplementOrDeclareServerClass(CTEWallPaintedEvent *this)

{
  return 0;
}


/* __tcf_4 at 00a3ed60 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x38a8d0),in_stack_00000008);
  return;
}


/* __tcf_2 at 00a3ed00 */

void __tcf_2(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410a2b))(unaff_EBX + 0x410a2b);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x4109d7))(unaff_EBX + 0x4109d7);
  (*(code *)**(undefined4 **)(&DAT_00410983 + unaff_EBX))(&DAT_00410983 + unaff_EBX);
                    /* WARNING: Could not recover jumptable at 0x00a3ed58. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(&DAT_0041092f + unaff_EBX))();
  return;
}


/* WallPainted at 00748c70 */

void WallPainted(int colorIndex,int nSegment,CBaseEntity *pWall)

{
  uint uVar1;
  int iVar2;
  CBaseEntity *pCVar3;
  undefined4 *puVar4;
  int unaff_EBX;
  CRecipientFilter *__in_chrg;
  CRecipientFilter local_3c;
  
                    /* Unresolved local var: CRecipientFilter filter@[???] */
  ___i686_get_pc_thunk_bx();
  CRecipientFilter::CRecipientFilter(&local_3c);
  CRecipientFilter::AddAllPlayers(&local_3c);
  uVar1 = *(uint *)(unaff_EBX + 0x70698c);
  if ((uVar1 == 0xffffffff) ||
     (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4fdee8),
     *(uint *)(iVar2 + 8) != uVar1 >> 0x10)) {
    pCVar3 = (CBaseEntity *)0x0;
  }
  else {
    pCVar3 = *(CBaseEntity **)(iVar2 + 4);
  }
  if (pCVar3 != pWall) {
    if (pWall == (CBaseEntity *)0x0) {
      *(undefined4 *)(unaff_EBX + 0x70698c) = 0xffffffff;
    }
    else {
      puVar4 = (undefined4 *)
               (*(pWall->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                 _vptr_IHandleEntity[3])(pWall);
      *(undefined4 *)(unaff_EBX + 0x70698c) = *puVar4;
    }
  }
  *(int *)(unaff_EBX + 0x706990) = colorIndex;
  *(int *)(unaff_EBX + 0x706994) = nSegment;
  __in_chrg = &local_3c;
  CBaseTempEntity::Create
            ((CBaseTempEntity *)(unaff_EBX + 0x706980),&local_3c.super_IRecipientFilter,0.0);
  CRecipientFilter::~CRecipientFilter(&local_3c,(int)__in_chrg);
  return;
}


/* CProjectedWallEntity::IsWallPainted at 00748840 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: bool IsWallPainted(CProjectedWallEntity * this, Vector * vecPosition)
    */

bool __thiscall CProjectedWallEntity::IsWallPainted(CProjectedWallEntity *this,Vector *vecPosition)

{
  PaintPowerType PVar1;
  
  PVar1 = GetPaintPowerAtPoint(this,vecPosition);
  return PVar1 != NO_POWER;
}


/* CProjectedWallEntity::GetNumSegments at 00748810 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: int GetNumSegments(CProjectedWallEntity * this) */

int __thiscall CProjectedWallEntity::GetNumSegments(CProjectedWallEntity *this)

{
  return (this->m_nNumSegments).m_Value;
}


/* CProjectedWallEntity::GetSegmentLength at 00748800 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: float GetSegmentLength(CProjectedWallEntity * this) */

float __thiscall CProjectedWallEntity::GetSegmentLength(CProjectedWallEntity *this)

{
  return (this->m_flSegmentLength).m_Value;
}


/* CProjectedWallEntity::GetPaintPowerAtSegment at 00748820 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: PaintPowerType GetPaintPowerAtSegment(CProjectedWallEntity * this, int
   i) */

PaintPowerType __thiscall
CProjectedWallEntity::GetPaintPowerAtSegment(CProjectedWallEntity *this,int i)

{
  return (this->m_PaintPowers).m_Memory.m_pMemory[i];
}


/* CProjectedWallEntity::CreateNewInstance at 00748870 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

CProjectedWallEntity * CProjectedWallEntity::CreateNewInstance(void)

{
  CProjectedWallEntity *pCVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  pCVar1 = (CProjectedWallEntity *)CreateEntityByName((char *)(unaff_EBX + 0x313ba8),-1,true);
  return pCVar1;
}


/* CProjectedWallEntity::CreateNewProjectedEntity at 00748d50 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: CBaseProjectedEntity * CreateNewProjectedEntity(CProjectedWallEntity *
   this) */

CBaseProjectedEntity * __thiscall
CProjectedWallEntity::CreateNewProjectedEntity(CProjectedWallEntity *this)

{
  CBaseProjectedEntity *pCVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  pCVar1 = (CBaseProjectedEntity *)CreateEntityByName((char *)(unaff_EBX + 0x3136c8),-1,true);
  return pCVar1;
}


/* CProjectedWallEntity::PlacePlacementHelper at 00748b10 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void PlacePlacementHelper(CProjectedWallEntity * this,
   CInfoPlacementHelper * pHelper) */

void __thiscall
CProjectedWallEntity::PlacePlacementHelper(CProjectedWallEntity *this,CInfoPlacementHelper *pHelper)

{
  float *pfVar1;
  int unaff_EBX;
  Vector local_28 [2];
  
                    /* Unresolved local var: float flZOffset@[???]
                       Unresolved local var: Vector vecPosition@[???] */
  ___i686_get_pc_thunk_bx();
  local_28[0].z = 32.0;
  if ((this->m_bIsHorizontal).m_Value == false) {
    local_28[0].z = 0.0;
  }
  pfVar1 = (float *)(*(this->super_CPaintableEntity<CBaseProjectedEntity>).
                      super_CBaseProjectedEntity.super_CBaseEntity.super_IServerEntity.
                      super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xcc])(this);
                    /* Unresolved local var: Vector res@[???] */
  local_28[0].x = *pfVar1 + 0.0;
  local_28[0].y = pfVar1[1] + 0.0;
  local_28[0].z = pfVar1[2] + local_28[0].z;
  if (pHelper != (CInfoPlacementHelper *)0x0) {
    CBaseEntity::SetAbsOrigin((CBaseEntity *)pHelper,local_28);
    (*(pHelper->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
      super_IHandleEntity._vptr_IHandleEntity[0x20])
              (pHelper,unaff_EBX + 0x343976,&UNK_0036a8ae + unaff_EBX);
    (*(pHelper->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
      super_IHandleEntity._vptr_IHandleEntity[0x20])
              (pHelper,unaff_EBX + 0x36a8b2,unaff_EBX + 0x30e15e);
    (*(pHelper->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
      super_IHandleEntity._vptr_IHandleEntity[0x13])(pHelper,this);
  }
  return;
}


/* CProjectedWallEntity::OnProjected at 0074c8e0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void OnProjected(CProjectedWallEntity * this) */

void __thiscall CProjectedWallEntity::OnProjected(CProjectedWallEntity *this)

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
  float fVar9;
  uint local_70;
  
  ___i686_get_pc_thunk_bx();
  CBaseProjectedEntity::OnProjected((CBaseProjectedEntity *)this);
  ProjectWall(this);
  fVar9 = *(float *)(unaff_EBX + 0x380209) + *(float *)(**(int **)(unaff_EBX + 0x4fa26d) + 0xc);
  if (fVar9 != (this->m_flParticleUpdateTime).m_Value) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                  super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4fa271);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_0074ca74:
            puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x45c;
            puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != 0x45c) {
            local_70 = 0;
            do {
              uVar8 = local_70 + 1;
              local_70 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_0074ca74;
                pIVar6->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar7 * 0x14 + local_70 + 1] != 0x45c);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4fa271) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4fa271) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x4fa271);
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar2 = *piVar5 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x45c;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flParticleUpdateTime).m_Value = fVar9;
  }
  return;
}


/* _GLOBAL__I__ZN20CProjectedWallEntity9m_DataMapE at 000b2bd0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN20CProjectedWallEntity9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

