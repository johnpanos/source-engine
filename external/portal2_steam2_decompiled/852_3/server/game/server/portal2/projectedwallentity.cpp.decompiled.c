/* DWARF-guided pseudocode for game/server/portal2/projectedwallentity.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* DataMapInit<CProjectedWallEntity> at 000c7a10 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

datamap_t * DataMapInit<CProjectedWallEntity>(CProjectedWallEntity *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)s_pRandomFloats + unaff_EBX + 0x3d6f) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf2eaf);
    if (iVar1 != 0) {
      *(int *)((int)s_pRandomFloats + unaff_EBX + 0x3d77) = unaff_EBX + 0x9497f2;
      *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3d7f) = 0;
      *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3d83) = 0;
      *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3d87) = 0;
      *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3d8b) = 0;
      *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3d8f) = 0;
      *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3d7b) = 0x14;
      ___cxa_guard_release(unaff_EBX + 0xcf2eaf);
      ___cxa_atexit(&UNK_008d3e4f + unaff_EBX,0,*(undefined4 *)(&DAT_00ae416f + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_NetworkStateChanged_m_hGroundEntity_00c05b08 + unaff_EBX + 3) =
       *(undefined4 *)(&DAT_00ae4ceb + unaff_EBX);
  *(undefined4 *)((int)&PTR_NetworkStateChanged_m_nWaterLevel_00c05b00 + unaff_EBX + 3) = 8;
  *(int *)((int)&PTR_NetworkStateChanged_m_nWaterLevel_00c05afc + unaff_EBX + 3) =
       unaff_EBX + 0xc05b5f;
  return (datamap_t *)((int)&PTR_NetworkStateChanged_m_nWaterLevel_00c05afc + unaff_EBX + 3);
}


/* __static_initialization_and_destruction_0 at 000c7ae0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *pcVar1;
  SendTable *pSVar2;
  IEntityFactoryDictionary *pIVar3;
  datamap_t *pdVar4;
  int iVar5;
  int iVar6;
  int unaff_EBX;
  undefined4 *puVar7;
  undefined4 *puVar8;
  undefined4 *puVar9;
  longlong lVar10;
  
  lVar10 = ___i686_get_pc_thunk_bx();
  if (lVar10 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3692) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3693) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3694) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3695) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3696) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x369a) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x369e) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x36a2) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x36a6) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x36aa) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x36ae) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x36b2) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x36b6) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x36ba) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x36be) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x36c2) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x36c6) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x36cd) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x36ca) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x36cb) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x36cc) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x36ce) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x36d5) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x36d2) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x36d3) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x36d4) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x36d6) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x36dd) = 1;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x36da) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x36db) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x36dc) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x36de) = 1;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x36e5) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x36e2) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x36e3) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x36e4) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x36e6) = 2;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x36ed) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x36ea) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x36eb) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x36ec) = 0;
  *(undefined **)((int)s_pRandomFloats + unaff_EBX + 0x36ee) = &UNK_00ae983a + unaff_EBX;
  *(undefined **)((int)s_pRandomFloats + unaff_EBX + 0x36f2) = &UNK_00b6feaa + unaff_EBX;
  pIVar3 = EntityFactoryDictionary();
  (**pIVar3->_vptr_IEntityFactoryDictionary)(pIVar3,unaff_EBX + 0xcf2832,&UNK_008f16f6 + unaff_EBX);
  pdVar4 = DataMapInit<CProjectedWallEntity>((CProjectedWallEntity *)0x0);
  *(datamap_t **)((int)&PTR_PostActionSignal_00c67d0c + unaff_EBX + 2) = pdVar4;
  iVar6 = unaff_EBX + 0xcf2836;
  pcVar1 = (char *)(unaff_EBX + 0x949725);
  *(char **)((int)s_pRandomFloats + unaff_EBX + 0x36f6) = pcVar1;
  pSVar2 = (SendTable *)((int)&PTR_RequestInfoFromChild_00c67d10 + unaff_EBX + 2);
  *(SendTable **)((int)s_pRandomFloats + unaff_EBX + 0x36fa) = pSVar2;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3706) = 0xffff;
  puVar9 = (undefined4 *)**(int **)(&DAT_00ae4142 + unaff_EBX);
  if (puVar9 == (undefined4 *)0x0) {
    **(int **)(&DAT_00ae4142 + unaff_EBX) = iVar6;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x36fe) = 0;
  }
  else {
    puVar8 = (undefined4 *)puVar9[2];
    iVar5 = _V_stricmp((char *)*puVar9,pcVar1);
    if (iVar5 < 1) {
      if (puVar8 != (undefined4 *)0x0) {
        do {
          puVar7 = puVar8;
          iVar5 = _V_stricmp((char *)*puVar7,pcVar1);
          puVar8 = puVar7;
          if (0 < iVar5) goto LAB_000c7e90;
          puVar8 = (undefined4 *)puVar7[2];
          puVar9 = puVar7;
        } while ((undefined4 *)puVar7[2] != (undefined4 *)0x0);
        puVar8 = (undefined4 *)0x0;
      }
LAB_000c7e90:
      *(undefined4 **)((int)s_pRandomFloats + unaff_EBX + 0x36fe) = puVar8;
      puVar9[2] = iVar6;
    }
    else {
      *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x36fe) =
           **(undefined4 **)(&DAT_00ae4142 + unaff_EBX);
      **(int **)(&DAT_00ae4142 + unaff_EBX) = iVar6;
    }
  }
  SendTable::SendTable(pSVar2);
  ___cxa_atexit(unaff_EBX + 0x8d3d62,0,*(undefined4 *)(&DAT_00ae40a2 + unaff_EBX));
  iVar6 = ServerClassInit<DT_ProjectedWallEntity::ignored>((ignored *)0x0);
  *(int *)((int)&PTR_InstallMouseHandler_00c67d24 + unaff_EBX + 2) = iVar6;
  iVar6 = unaff_EBX + 0xcf284a;
  pcVar1 = (char *)(unaff_EBX + 0x94974e);
  *(char **)((int)s_pRandomFloats + unaff_EBX + 0x370a) = pcVar1;
  pSVar2 = (SendTable *)((int)&PTR_SetEnabled_00c67d28 + unaff_EBX + 2);
  *(SendTable **)((int)s_pRandomFloats + unaff_EBX + 0x370e) = pSVar2;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x371a) = 0xffff;
  puVar9 = (undefined4 *)**(int **)(&DAT_00ae4142 + unaff_EBX);
  if (puVar9 == (undefined4 *)0x0) {
    **(int **)(&DAT_00ae4142 + unaff_EBX) = iVar6;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3712) = 0;
  }
  else {
    puVar8 = (undefined4 *)puVar9[2];
    iVar5 = _V_stricmp((char *)*puVar9,pcVar1);
    if (iVar5 < 1) {
      while ((puVar8 != (undefined4 *)0x0 && (iVar5 = _V_stricmp((char *)*puVar8,pcVar1), iVar5 < 1)
             )) {
        puVar9 = puVar8;
        puVar8 = (undefined4 *)puVar8[2];
      }
      *(undefined4 **)((int)s_pRandomFloats + unaff_EBX + 0x3712) = puVar8;
      puVar9[2] = iVar6;
    }
    else {
      *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3712) =
           **(undefined4 **)(&DAT_00ae4142 + unaff_EBX);
      **(int **)(&DAT_00ae4142 + unaff_EBX) = iVar6;
    }
  }
  SendTable::SendTable(pSVar2);
  ___cxa_atexit(unaff_EBX + 0x8d3d42,0,*(undefined4 *)(&DAT_00ae40a2 + unaff_EBX));
  iVar6 = ServerClassInit<DT_TEWallPaintedEvent::ignored>((ignored *)0x0);
  *(int *)((int)&PTR_SetFgColor_00c67d3c + unaff_EBX + 2) = iVar6;
  CBaseTempEntity::CBaseTempEntity
            ((CBaseTempEntity *)((int)s_pRandomFloats + unaff_EBX + 0x371e),
             (char *)(unaff_EBX + 0x949762));
  *(undefined **)((int)s_pRandomFloats + unaff_EBX + 0x371e) = &UNK_00b6fa9a + unaff_EBX;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x372a) = 0xffffffff;
  ___cxa_atexit(&UNK_008d3e62 + unaff_EBX,0,*(undefined4 *)(&DAT_00ae40a2 + unaff_EBX));
  return;
}


/* CProjectedWallEntity::GetDataDescMap at 006d8e30 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: datamap_t * GetDataDescMap(CProjectedWallEntity * this) */

datamap_t * __thiscall CProjectedWallEntity::GetDataDescMap(CProjectedWallEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5f46e8);
}


/* CProjectedWallEntity::GetBaseMap at 006d8e40 */

datamap_t * CProjectedWallEntity::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4d38c4);
}


/* __tcf_0 at 0099b870 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41f06e)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41f062) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41f06e));
  }
  *(undefined4 *)(unaff_EBX + 0x41f06e) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41f06a)) {
    if (*(int *)(unaff_EBX + 0x41f062) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x21031a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x21031a),*(int *)(unaff_EBX + 0x41f062));
      *(undefined4 *)(unaff_EBX + 0x41f062) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41f066) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41f062);
  *(int *)(unaff_EBX + 0x41f072) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41f06a)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x21031a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x21031a),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41f062) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41f066) = 0;
  }
  return;
}


/* ServerClassInit<DT_ProjectedWallEntity::ignored> at 000c74f0 */

/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Enum "PrecacheSystem_t": Some values do not have unique names */

int ServerClassInit<DT_ProjectedWallEntity::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)s_pRandomFloats + unaff_EBX + 0x3e92) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf2fd2);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)((int)s_pRandomFloats + unaff_EBX + 0x3ea2),
                  (char *)(unaff_EBX + 0x8eb526),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)((int)s_pRandomFloats + unaff_EBX + 0x3ef6),
                        (char *)(unaff_EBX + 0x8ed896),0,
                        (SendTable *)**(undefined4 **)(&DAT_00ae5216 + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00ae472e + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)((int)s_pRandomFloats + unaff_EBX + 0x3f4a),
                     (char *)(unaff_EBX + 0x949c78),0x424,0xc,-1,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00ae477a + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)((int)s_pRandomFloats + unaff_EBX + 0x3f9e),
                     (char *)(unaff_EBX + 0x949c8f),0x430,0xc,-1,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00ae477a + unaff_EBX),0x80);
      SendPropEHandle((SendProp_conflict *)((int)s_pRandomFloats + unaff_EBX + 0x3ff2),
                      (char *)(vgui::TextEntry::GetText + unaff_EBX + 6),0x43c,4,-1,
                      *(SendVarProxyFn_conflict *)(&DAT_00ae471a + unaff_EBX));
      SendPropFloat((SendProp_conflict *)(unaff_EBX + 0xcf3186),(char *)(unaff_EBX + 0x949cb5),0x440
                    ,4,-1,4,0.0,-121121.125,*(SendVarProxyFn_conflict *)(&DAT_00ae4786 + unaff_EBX),
                    0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&CUtlRBTree<CUtlMap<unsigned_int,void(ResponseRules::CResponseSystem::*)(),unsigned_short>::Node_t,unsigned_short,CUtlMap<unsigned_int,void(ResponseRules::CResponseSystem::*)(),unsigned_short>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<unsigned_int,void(ResponseRules::CResponseSystem::*)(),unsigned_short>::Node_t,unsigned_short>,unsigned_short>>
                           ::Links(unsigned_short)::s_Sentinel + unaff_EBX + 2),
                    (char *)(unaff_EBX + 0x9580aa),0x448,4,-1,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00ae4786 + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)(unaff_EBX + 0xcf322e),(char *)(unaff_EBX + 0x949cc2),0x444
                    ,4,-1,4,0.0,-121121.125,*(SendVarProxyFn_conflict *)(&DAT_00ae4786 + unaff_EBX),
                    0x80);
      SendPropFloat((SendProp_conflict *)(unaff_EBX + 0xcf3282),(char *)(unaff_EBX + 0x949ccd),0x454
                    ,4,-1,4,0.0,-121121.125,*(SendVarProxyFn_conflict *)(&DAT_00ae4786 + unaff_EBX),
                    0x80);
      SendPropFloat((SendProp_conflict *)(unaff_EBX + 0xcf32d6),&UNK_00949cdf + unaff_EBX,0x458,4,-1
                    ,4,0.0,-121121.125,*(SendVarProxyFn_conflict *)(&DAT_00ae4786 + unaff_EBX),0x80)
      ;
      SendPropInt((SendProp_conflict *)((int)CBaseResourcePrecacher::sm_pFirst + unaff_EBX + 2),
                  (char *)(unaff_EBX + 0x949cf6),0x450,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropBool((SendProp_conflict *)((int)&g_aFadeData[3].m_flPercentMin + unaff_EBX + 2),
                   (char *)(unaff_EBX + 0x949d05),0x44c,1);
      ___cxa_guard_release(unaff_EBX + 0xcf2fd2);
      ___cxa_atexit(unaff_EBX + 0x8d41f2,0,*(undefined4 *)(&DAT_00ae4692 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)((int)&g_KeyNames[0x42].displaystring + unaff_EBX + 2),
             (SendProp_conflict *)((int)s_pRandomFloats + unaff_EBX + 0x3ef6),0xb,
             *(char **)(&DAT_00b7045e + unaff_EBX));
  return 1;
}


/* CProjectedWallEntity::GetServerClass at 006d8e50 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: ServerClass * GetServerClass(CProjectedWallEntity * this) */

ServerClass * __thiscall CProjectedWallEntity::GetServerClass(CProjectedWallEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x6e14cc);
}


/* CProjectedWallEntity::YouForgotToImplementOrDeclareServerClass at 006d8e60 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CProjectedWallEntity *
   this) */

int __thiscall
CProjectedWallEntity::YouForgotToImplementOrDeclareServerClass(CProjectedWallEntity *this)

{
  return 0;
}


/* __tcf_3 at 0099b850 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x393fa8),in_stack_00000008);
  return;
}


/* __tcf_1 at 0099b6f0 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41f17f))(unaff_EBX + 0x41f17f);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41f12b))(unaff_EBX + 0x41f12b);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41f0d7))(unaff_EBX + 0x41f0d7);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41f083))(unaff_EBX + 0x41f083);
  (*(code *)**(undefined4 **)(&DAT_0041f02f + unaff_EBX))(&DAT_0041f02f + unaff_EBX);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41efdb))(unaff_EBX + 0x41efdb);
  (*(code *)**(undefined4 **)(&DAT_0041ef87 + unaff_EBX))(&DAT_0041ef87 + unaff_EBX);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41ef33))(unaff_EBX + 0x41ef33);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41eedf))(unaff_EBX + 0x41eedf);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41ee8b))(unaff_EBX + 0x41ee8b);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41ee37))(unaff_EBX + 0x41ee37);
                    /* WARNING: Could not recover jumptable at 0x0099b7c9. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41ede3))();
  return;
}


/* CProjectedWallEntity::CProjectedWallEntity at 006db130 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void CProjectedWallEntity(CProjectedWallEntity * this) */

void __thiscall CProjectedWallEntity::CProjectedWallEntity(CProjectedWallEntity *this)

{
  undefined1 *puVar1;
  char cVar2;
  CBaseEdict *pCVar3;
  IChangeInfoAccessor *pIVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseProjectedEntity::CBaseProjectedEntity((CBaseProjectedEntity *)this);
  (this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.super_CBaseEntity.
  super_IServerEntity.super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x55c48a);
  (this->super_CPaintableEntity<CBaseProjectedEntity>).super_IPaintableEntity._vptr_IPaintableEntity
       = (_func_int_varargs **)(&UNK_0055c7ee + unaff_EBX);
  (this->m_hColorPortal).m_Value.super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_flLength).m_Value = 0.0;
  if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
      super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
    pCVar3 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
               super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if (pCVar3 == (CBaseEdict *)0x0) goto LAB_006db18e;
    pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
    pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
    pIVar4->m_iChangeInfoSerialNumber = 0;
    (this->m_flHeight).m_Value = 0.015625;
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) goto LAB_006db290;
LAB_006db1a2:
    puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
              super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
LAB_006db1a6:
    (this->m_flWidth).m_Value = 64.0;
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) goto LAB_006db2d0;
LAB_006db1ba:
    puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
              super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
LAB_006db1be:
    (this->m_bIsHorizontal).m_Value = false;
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) goto LAB_006db300;
LAB_006db1cf:
    puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
              super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
LAB_006db1d3:
    (this->m_nNumSegments).m_Value = 0;
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered != false) goto LAB_006db1e5;
LAB_006db340:
    pCVar3 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
               super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if (pCVar3 == (CBaseEdict *)0x0) goto LAB_006db1e9;
    pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
    pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
    pIVar4->m_iChangeInfoSerialNumber = 0;
    (this->m_flSegmentLength).m_Value = 64.0;
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) goto LAB_006db380;
LAB_006db1fd:
    puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
              super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  else {
    puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
              super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
LAB_006db18e:
    (this->m_flHeight).m_Value = 0.015625;
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered != false) goto LAB_006db1a2;
LAB_006db290:
    pCVar3 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
               super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if (pCVar3 == (CBaseEdict *)0x0) goto LAB_006db1a6;
    pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
    pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
    pIVar4->m_iChangeInfoSerialNumber = 0;
    (this->m_flWidth).m_Value = 64.0;
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered != false) goto LAB_006db1ba;
LAB_006db2d0:
    pCVar3 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
               super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if (pCVar3 == (CBaseEdict *)0x0) goto LAB_006db1be;
    pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
    pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
    pIVar4->m_iChangeInfoSerialNumber = 0;
    (this->m_bIsHorizontal).m_Value = false;
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered != false) goto LAB_006db1cf;
LAB_006db300:
    pCVar3 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
               super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if (pCVar3 == (CBaseEdict *)0x0) goto LAB_006db1d3;
    pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
    pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
    pIVar4->m_iChangeInfoSerialNumber = 0;
    (this->m_nNumSegments).m_Value = 0;
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) goto LAB_006db340;
LAB_006db1e5:
    puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
              super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
LAB_006db1e9:
    (this->m_flSegmentLength).m_Value = 64.0;
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered != false) goto LAB_006db1fd;
LAB_006db380:
    pCVar3 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
               super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if (pCVar3 != (CBaseEdict *)0x0) {
      pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
      pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
      pIVar4->m_iChangeInfoSerialNumber = 0;
      (this->m_flParticleUpdateTime).m_Value = 0.0;
      cVar2 = (this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
              super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered;
      goto joined_r0x006db20f;
    }
  }
  (this->m_flParticleUpdateTime).m_Value = 0.0;
  cVar2 = (this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
          super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered;
joined_r0x006db20f:
  if (cVar2 == '\0') {
    pCVar3 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
               super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if (pCVar3 != (CBaseEdict *)0x0) {
      pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
      pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
      pIVar4->m_iChangeInfoSerialNumber = 0;
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


/* CProjectedWallEntity::CProjectedWallEntity at 006db420 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void CProjectedWallEntity(CProjectedWallEntity * this,
   CProjectedWallEntity * this) */

void __thiscall
CProjectedWallEntity::CProjectedWallEntity(CProjectedWallEntity *this,CProjectedWallEntity *this_1)

{
  CProjectedWallEntity(this);
  return;
}


/* CEntityFactory<CProjectedWallEntity>::Create at 006db840 */

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
  this_00 = CBaseEntity::operator_new(0x470);
  CProjectedWallEntity::CProjectedWallEntity(this_00);
  (*(this_00->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
    super_CBaseEntity.super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x1d])(this_00,pClassName);
  return &(this_00->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
          super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CProjectedWallEntity::~CProjectedWallEntity at 006d9a10 */

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
       (_func_int_varargs **)(unaff_EBX + 0x55dbaa);
  (this->super_CPaintableEntity<CBaseProjectedEntity>).super_IPaintableEntity._vptr_IPaintableEntity
       = (_func_int_varargs **)(unaff_EBX + 0x55df0e);
  CleanupWall(this);
  __in_chrg_00 = this;
  CPaintDatabase::RemovePaintedWall(*(CPaintDatabase **)(unaff_EBX + 0x4d24f2),this,true);
  (this->m_PaintPowers).m_Size = 0;
  if ((this->m_PaintPowers).m_Memory.m_nGrowSize < 0) {
    pCVar1 = (CProjectedWallEntity *)(this->m_PaintPowers).m_Memory.m_pMemory;
  }
  else {
    pCVar1 = (CProjectedWallEntity *)(this->m_PaintPowers).m_Memory.m_pMemory;
    if (pCVar1 != (CProjectedWallEntity *)0x0) {
      __in_chrg_00 = pCVar1;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d217a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4d217a));
      pCVar1 = (CProjectedWallEntity *)0x0;
      (this->m_PaintPowers).m_Memory.m_pMemory = (PaintPowerType *)0x0;
    }
    (this->m_PaintPowers).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintPowers).m_pElements = (PaintPowerType *)pCVar1;
  if (-1 < (this->m_PaintPowers).m_Memory.m_nGrowSize) {
    if (pCVar1 != (CProjectedWallEntity *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d217a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4d217a));
      (this->m_PaintPowers).m_Memory.m_pMemory = (PaintPowerType *)0x0;
      __in_chrg_00 = pCVar1;
    }
    (this->m_PaintPowers).m_Memory.m_nAllocationCount = 0;
  }
  (this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.super_CBaseEntity.
  super_IServerEntity.super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x55dfaa);
  (this->super_CPaintableEntity<CBaseProjectedEntity>).super_IPaintableEntity._vptr_IPaintableEntity
       = (_func_int_varargs **)(unaff_EBX + 0x55e30e);
  (this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.super_CBaseEntity.
  super_IServerEntity.super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4d2cf2) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,(int)__in_chrg_00);
  CBaseEntity::operator_delete(this);
  return;
}


/* CProjectedWallEntity::~CProjectedWallEntity at 006d9c10 */

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
       (_func_int_varargs **)(unaff_EBX + 0x55d9aa);
  (this->super_CPaintableEntity<CBaseProjectedEntity>).super_IPaintableEntity._vptr_IPaintableEntity
       = (_func_int_varargs **)(unaff_EBX + 0x55dd0e);
  CleanupWall(this);
  CPaintDatabase::RemovePaintedWall(*(CPaintDatabase **)(unaff_EBX + 0x4d22f2),this,true);
  (this->m_PaintPowers).m_Size = 0;
  if ((this->m_PaintPowers).m_Memory.m_nGrowSize < 0) {
    pPVar1 = (this->m_PaintPowers).m_Memory.m_pMemory;
  }
  else {
    pPVar1 = (this->m_PaintPowers).m_Memory.m_pMemory;
    if (pPVar1 != (PaintPowerType *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d1f7a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4d1f7a),pPVar1);
      (this->m_PaintPowers).m_Memory.m_pMemory = (PaintPowerType *)0x0;
    }
    pPVar1 = (PaintPowerType *)0x0;
    (this->m_PaintPowers).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintPowers).m_pElements = pPVar1;
  if (-1 < (this->m_PaintPowers).m_Memory.m_nGrowSize) {
    if (pPVar1 != (PaintPowerType *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d1f7a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4d1f7a),pPVar1);
      (this->m_PaintPowers).m_Memory.m_pMemory = (PaintPowerType *)0x0;
    }
    (this->m_PaintPowers).m_Memory.m_nAllocationCount = 0;
  }
  (this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.super_CBaseEntity.
  super_IServerEntity.super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x55ddaa);
  (this->super_CPaintableEntity<CBaseProjectedEntity>).super_IPaintableEntity._vptr_IPaintableEntity
       = (_func_int_varargs **)(unaff_EBX + 0x55e10e);
  (this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.super_CBaseEntity.
  super_IServerEntity.super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4d2af2) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,__in_chrg);
  return;
}


/* CProjectedWallEntity::~CProjectedWallEntity at 006d9e20 */

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


/* CProjectedWallEntity::UpdateOnRemove at 006d99e0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void UpdateOnRemove(CProjectedWallEntity * this) */

void __thiscall CProjectedWallEntity::UpdateOnRemove(CProjectedWallEntity *this)

{
  CleanupWall(this);
  CBaseProjectedEntity::UpdateOnRemove((CBaseProjectedEntity *)this);
  return;
}


/* CProjectedWallEntity::Precache at 006d9220 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void Precache(CProjectedWallEntity * this) */

void __thiscall CProjectedWallEntity::Precache(CProjectedWallEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  PrecacheParticleSystem((char *)(extraout_ECX + 0x337f36));
  return;
}


/* CProjectedWallEntity::Spawn at 006d9180 */

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


/* CProjectedWallEntity::OnRestore at 006d9150 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void OnRestore(CProjectedWallEntity * this) */

void __thiscall CProjectedWallEntity::OnRestore(CProjectedWallEntity *this)

{
  CBaseProjectedEntity::OnRestore((CBaseProjectedEntity *)this);
  CBaseEntity::SetTransmitState((CBaseEntity *)this,8);
  return;
}


/* CProjectedWallEntity::ProjectWall at 006d9e30 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ProjectWall(CProjectedWallEntity * this) */

void __thiscall CProjectedWallEntity::ProjectWall(CProjectedWallEntity *this)

{
  undefined1 *puVar1;
  vec_t *pvVar2;
  int *piVar3;
  uint *puVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  float fVar10;
  code *pcVar11;
  CBaseEdict *pCVar12;
  PaintPowerType *src;
  CBasePlayer *this_00;
  CProjectedWallEntity *pCVar13;
  float *pfVar14;
  CPhysCollide *pCVar15;
  IPhysicsObject *pPhysics;
  IChangeInfoAccessor *pIVar16;
  int iVar17;
  CPolyhedron *pCVar18;
  int unaff_EBX;
  undefined4 *puVar19;
  NetworkVar_m_Collision *this_01;
  int iVar20;
  objectparams_t *poVar21;
  longdouble lVar22;
  float fVar23;
  Vector *local_8a8;
  bool local_885;
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
  local_20 = **(int **)(unaff_EBX + 0x4d1dab);
  CleanupWall(this);
  CBaseEntity::AddEffects((CBaseEntity *)this,8);
  if (0 < *(int *)(**(int **)(&DAT_004d1d5f + unaff_EBX) + 0x14)) {
    iVar20 = 1;
    do {
      this_00 = UTIL_PlayerByIndex(iVar20);
      if ((this_00 != (CBasePlayer *)0x0) &&
         (pCVar13 = (CProjectedWallEntity *)CBaseEntity::GetGroundEntity((CBaseEntity *)this_00),
         this == pCVar13)) {
        CBaseEntity::SetGroundEntity((CBaseEntity *)this_00,(CBaseEntity *)0x0);
      }
      iVar20 = iVar20 + 1;
    } while (iVar20 <= *(int *)(**(int **)(&DAT_004d1d5f + unaff_EBX) + 0x14));
  }
  pfVar14 = (float *)(*(this->super_CPaintableEntity<CBaseProjectedEntity>).
                       super_CBaseProjectedEntity.super_CBaseEntity.super_IServerEntity.
                       super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xcb])(this);
  fVar23 = pfVar14[2];
  fVar5 = pfVar14[1];
  fVar6 = *pfVar14;
  pfVar14 = (float *)(*(this->super_CPaintableEntity<CBaseProjectedEntity>).
                       super_CBaseProjectedEntity.super_CBaseEntity.super_IServerEntity.
                       super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xcc])(this);
  fVar7 = pfVar14[2];
  fVar8 = pfVar14[1];
  fVar9 = *pfVar14;
  (*(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
    super_CBaseEntity.super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x8e])(this,&local_66c,&local_678,&local_684);
  if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x4d28c7) + 0x1c) + 0x30) == 0) {
    puVar4 = (uint *)(unaff_EBX + 0x35973f);
    local_86c.z = (vec_t)((uint)local_664 ^ *puVar4);
    local_86c.y = (vec_t)((uint)local_668 ^ *puVar4);
    local_86c.x = (vec_t)((uint)local_66c ^ *puVar4);
    local_860 = fVar6 * local_86c.x + fVar5 * local_86c.y + fVar23 * local_86c.z;
    local_85c = local_66c;
    local_858 = local_668;
    local_854 = local_664;
    local_850 = fVar9 * local_66c + fVar8 * local_668 + fVar7 * local_664;
                    /* Unresolved local var: Vector res@[???] */
    fVar10 = *(float *)(unaff_EBX + 0x3507db);
                    /* Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
    pfVar14 = (float *)(unaff_EBX + 0x3506bf);
    local_6f0 = local_678 * fVar10 * *pfVar14;
    local_6ec = local_674 * fVar10 * *pfVar14;
    local_6e8 = *pfVar14 * local_670 * fVar10;
                    /* Unresolved local var: Vector res@[???] */
    local_6e4 = fVar6 + local_6f0;
    local_6e0 = fVar5 + local_6ec;
    local_6dc = fVar23 + local_6e8;
    local_84c = local_678;
    local_848 = local_674;
    local_844 = local_670;
    local_840 = local_6e4 * local_678 + local_6e0 * local_674 + local_6dc * local_670;
                    /* Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
                    /* Unresolved local var: Vector res@[???] */
    local_6fc = fVar6 - local_6f0;
    local_6f8 = fVar5 - local_6ec;
    local_6f4 = fVar23 - local_6e8;
    local_834 = (float)((uint)local_670 ^ *puVar4);
    local_838 = (float)((uint)local_674 ^ *puVar4);
    local_83c = (float)((uint)local_678 ^ *puVar4);
    local_830 = local_6fc * local_83c + local_6f8 * local_838 + local_6f4 * local_834;
                    /* Unresolved local var: Vector res@[???] */
    fVar10 = *(float *)(unaff_EBX + 0x350b57);
                    /* Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
    pfVar14 = (float *)(unaff_EBX + 0x3506bf);
    local_72c = local_684 * fVar10 * *pfVar14;
    local_728 = local_680 * fVar10 * *pfVar14;
    local_724 = *pfVar14 * local_67c * fVar10;
                    /* Unresolved local var: Vector res@[???] */
    local_720 = fVar6 + local_72c;
    local_71c = fVar5 + local_728;
    local_718 = fVar23 + local_724;
    local_82c = local_684;
    local_828 = local_680;
    local_824 = local_67c;
    local_820 = local_720 * local_684 + local_71c * local_680 + local_718 * local_67c;
                    /* Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
                    /* Unresolved local var: Vector res@[???] */
    local_738 = fVar6 - local_72c;
    local_734 = fVar5 - local_728;
    local_730 = fVar23 - local_724;
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
    pCVar18 = GeneratePolyhedronFromPlanes(&local_86c.x,6,0.0,false);
    if (pCVar18 == (CPolyhedron *)0x0) {
      _Warning(unaff_EBX + 0x337423);
      goto LAB_006da859;
    }
    local_80c[0] = (float)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d1e9f) + 0x24))
                                    ((int *)**(undefined4 **)(unaff_EBX + 0x4d1e9f),pCVar18);
    (*pCVar18->_vptr_CPolyhedron[2])(pCVar18);
    if (local_80c[0] == 0.0) goto LAB_006da859;
    pCVar15 = (CPhysCollide *)
              (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d1e9f) + 0x3c))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x4d1e9f),local_80c,1);
    this->m_pWallCollideable = pCVar15;
  }
  else {
                    /* Unresolved local var: Vector vScaledRight@[???]
                       Unresolved local var: Vector vStart@[???]
                       Unresolved local var: Vector vEnd@[???]
                       Unresolved local var: Vector[4] vVerts@[???]
                       Unresolved local var: Vector *[4] pVerts@[???]
                       Unresolved local var: CPhysConvex * pTempConvex@[???]
                       Unresolved local var: Vector res@[???] */
    local_688 = *(float *)(unaff_EBX + 0x3507df);
    local_690 = local_678 * local_688;
    local_68c = local_674 * local_688;
    local_688 = local_670 * local_688;
                    /* Unresolved local var: Vector res@[???] */
    local_86c.x = fVar6 + local_690;
    local_86c.y = fVar5 + local_68c;
    local_86c.z = fVar23 + local_688;
                    /* Unresolved local var: Vector res@[???] */
    local_860 = fVar6 - local_690;
    local_85c = fVar5 - local_68c;
    local_858 = fVar23 - local_688;
                    /* Unresolved local var: Vector res@[???] */
    local_854 = fVar9 - local_690;
    local_850 = fVar8 - local_68c;
    local_84c = fVar7 - local_688;
                    /* Unresolved local var: Vector res@[???] */
    local_848 = local_690 + fVar9;
    local_844 = local_68c + fVar8;
    local_840 = local_688 + fVar7;
    local_7fc.x = (vec_t)&local_86c;
    local_7fc.y = (vec_t)&local_860;
    local_7fc.z = (vec_t)&local_854;
    local_7f0 = &local_848;
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
    local_80c[0] = (float)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d1e9f) + 8))
                                    ((int *)**(undefined4 **)(unaff_EBX + 0x4d1e9f),&local_7fc,4);
    pCVar15 = (CPhysCollide *)
              (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d1e9f) + 0x3c))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x4d1e9f),local_80c,1);
    this->m_pWallCollideable = pCVar15;
  }
  local_8a8 = &local_86c;
  if (this->m_pWallCollideable == (CPhysCollide *)0x0) goto LAB_006da859;
  V_strncpy(local_660.surfaceprop,(char *)(unaff_EBX + 0x2dde8d),0x200);
  puVar19 = *(undefined4 **)(unaff_EBX + 0x4d1fc7);
  poVar21 = &local_660.params;
  for (iVar20 = 0xb; iVar20 != 0; iVar20 = iVar20 + -1) {
    poVar21->massCenterOverride = (Vector *)*puVar19;
    puVar19 = puVar19 + 1;
    poVar21 = (objectparams_t *)&poVar21->mass;
  }
  local_660.params.pGameData = this;
  pPhysics = PhysModelCreateCustom
                       ((CBaseEntity *)this,this->m_pWallCollideable,
                        *(Vector **)(unaff_EBX + 0x4d1d6f),*(QAngle **)(unaff_EBX + 0x4d1e03),
                        (char *)(unaff_EBX + 0x2dde8d),true,&local_660);
  if (pPhysics == (IPhysicsObject *)0x0) goto LAB_006da859;
  if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
      super_CBaseEntity.m_pPhysicsObject != (IPhysicsObject *)0x0) {
    (*(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
      super_CBaseEntity.super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
      _vptr_IHandleEntity[0xa1])(this);
  }
  CBaseEntity::VPhysicsSetObject((CBaseEntity *)this,pPhysics);
  iVar20 = (*pPhysics->_vptr_IPhysicsObject[0x4b])(pPhysics);
  if (iVar20 == 0) goto LAB_006da859;
  pfVar14 = *(float **)(unaff_EBX + 0x4d1d6f);
  local_75c = *pfVar14;
  local_758 = pfVar14[1];
  local_754 = pfVar14[2];
  pcVar11 = *(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d1e9f) + 0x60);
  local_750 = local_75c;
  local_74c = local_758;
  local_748 = local_754;
  iVar20 = (*pPhysics->_vptr_IPhysicsObject[0x4b])(pPhysics);
  (*pcVar11)(**(undefined4 **)(unaff_EBX + 0x4d1e9f),&local_750,&local_75c,iVar20,
             *(undefined4 *)(unaff_EBX + 0x4d1d6f),*(undefined4 *)(unaff_EBX + 0x4d1e03));
  local_7e0 = local_750;
  local_7dc = local_74c;
  local_7d8 = local_748;
  if ((((local_750 != (this->m_vWorldSpace_WallMins).m_Value.x) ||
       (NAN(local_750) || NAN((this->m_vWorldSpace_WallMins).m_Value.x))) ||
      (pvVar2 = &(this->m_vWorldSpace_WallMins).m_Value.y, local_74c != *pvVar2)) ||
     ((NAN(local_74c) || NAN(*pvVar2) || (local_748 != (this->m_vWorldSpace_WallMins).m_Value.z))))
  {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar12 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                  super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar12 != (CBaseEdict *)0x0) {
        pCVar12->m_fStateFlags = pCVar12->m_fStateFlags | 0x101;
        pIVar16 = CBaseEdict::GetChangeAccessor(pCVar12);
        pIVar16->m_iChangeInfoSerialNumber = 0;
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
  if (((local_75c != (this->m_vWorldSpace_WallMaxs).m_Value.x) ||
      (NAN(local_75c) || NAN((this->m_vWorldSpace_WallMaxs).m_Value.x))) ||
     ((pvVar2 = &(this->m_vWorldSpace_WallMaxs).m_Value.y, local_758 != *pvVar2 ||
      ((NAN(local_758) || NAN(*pvVar2) || (local_754 != (this->m_vWorldSpace_WallMaxs).m_Value.z))))
     )) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar12 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                  super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar12 != (CBaseEdict *)0x0) {
        pCVar12->m_fStateFlags = pCVar12->m_fStateFlags | 0x101;
        pIVar16 = CBaseEdict::GetChangeAccessor(pCVar12);
        pIVar16->m_iChangeInfoSerialNumber = 0;
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
  local_86c.z = local_748 - fVar23;
                    /* Unresolved local var: Vector res@[???] */
  local_7fc.x = local_75c - fVar6;
  local_7fc.y = local_758 - fVar5;
  local_7fc.z = local_754 - fVar23;
  local_774 = (Vector *)local_7fc.x;
  local_770 = (float *)local_7fc.y;
  local_76c = (float *)local_7fc.z;
  local_768 = local_86c.x;
  local_764 = local_86c.y;
  local_760 = local_86c.z;
  CBaseEntity::SetSize((CBaseEntity *)this,local_8a8,&local_7fc);
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: __m128 root@[???] */
  fVar23 = SQRT((fVar6 - fVar9) * (fVar6 - fVar9) + (fVar5 - fVar8) * (fVar5 - fVar8) +
                (fVar23 - fVar7) * (fVar23 - fVar7));
  local_80c[1] = 0.0;
  local_80c[2] = 0.0;
  local_80c[3] = 0.0;
  local_80c[0] = fVar23;
  if (fVar23 != (this->m_flLength).m_Value) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar12 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                  super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar12 != (CBaseEdict *)0x0) {
        pCVar12->m_fStateFlags = pCVar12->m_fStateFlags | 0x101;
        pIVar16 = CBaseEdict::GetChangeAccessor(pCVar12);
        pIVar16->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flLength).m_Value = fVar23;
  }
  if (*(float *)(unaff_EBX + 0x3507db) != (this->m_flWidth).m_Value) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar12 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                  super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar12 != (CBaseEdict *)0x0) {
        pCVar12->m_fStateFlags = pCVar12->m_fStateFlags | 0x101;
        pIVar16 = CBaseEdict::GetChangeAccessor(pCVar12);
        pIVar16->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flWidth).m_Value = 64.0;
  }
  if (*(float *)(unaff_EBX + 0x350b57) != (this->m_flHeight).m_Value) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar12 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                  super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar12 != (CBaseEdict *)0x0) {
        pCVar12->m_fStateFlags = pCVar12->m_fStateFlags | 0x101;
        pIVar16 = CBaseEdict::GetChangeAccessor(pCVar12);
        pIVar16->m_iChangeInfoSerialNumber = 0;
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
  if (*(float *)(unaff_EBX + 0x35088b) < local_780.z) {
    fVar23 = *(float *)(unaff_EBX + 0x35192b);
LAB_006da890:
    if (local_78c.z <= fVar23) goto LAB_006da71e;
    local_885 = local_78c.z < *(float *)(unaff_EBX + 0x35088b);
  }
  else {
    fVar23 = *(float *)(unaff_EBX + 0x35192b);
    if (local_780.z < fVar23) goto LAB_006da890;
LAB_006da71e:
    local_885 = false;
  }
  if ((this->m_bIsHorizontal).m_Value != local_885) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar12 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                  super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar12 != (CBaseEdict *)0x0) {
        pCVar12->m_fStateFlags = pCVar12->m_fStateFlags | 0x101;
        pIVar16 = CBaseEdict::GetChangeAccessor(pCVar12);
        pIVar16->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bIsHorizontal).m_Value = local_885;
  }
  DisplaceObstructingEntities(this);
  lVar22 = (longdouble)_ceilf((this->m_flLength).m_Value / (this->m_flSegmentLength).m_Value);
  iVar20 = (int)lVar22;
  if (iVar20 != (this->m_nNumSegments).m_Value) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar12 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                  super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar12 != (CBaseEdict *)0x0) {
        pCVar12->m_fStateFlags = pCVar12->m_fStateFlags | 0x101;
        pIVar16 = CBaseEdict::GetChangeAccessor(pCVar12);
        pIVar16->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_nNumSegments).m_Value = iVar20;
  }
  (this->m_PaintPowers).m_Size = 0;
  if (iVar20 != 0) {
    iVar17 = (this->m_PaintPowers).m_Memory.m_nAllocationCount;
    if (iVar17 < iVar20) {
      CUtlMemory<PaintPowerType,int>::Grow(&(this->m_PaintPowers).m_Memory,iVar20 - iVar17);
    }
    piVar3 = &(this->m_PaintPowers).m_Size;
    *piVar3 = *piVar3 + iVar20;
    src = (this->m_PaintPowers).m_Memory.m_pMemory;
    (this->m_PaintPowers).m_pElements = src;
                    /* Unresolved local var: int numToMove@[???] */
    iVar17 = (this->m_PaintPowers).m_Size - iVar20;
    if ((0 < iVar17) && (0 < iVar20)) {
      _V_memmove(src + iVar20,src,iVar17 * 4);
    }
  }
  (*(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
    super_CBaseEntity.super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0xd6])(this);
LAB_006da859:
  if (local_20 != **(int **)(unaff_EBX + 0x4d1dab)) {
                    /* WARNING: Subroutine does not return */
    ___stack_chk_fail();
  }
  return;
}


/* CProjectedWallEntity::CleanupWall at 006d9690 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void CleanupWall(CProjectedWallEntity * this) */

void __thiscall CProjectedWallEntity::CleanupWall(CProjectedWallEntity *this)

{
  undefined1 *puVar1;
  vec_t *pvVar2;
  float *pfVar3;
  float fVar4;
  CBaseEdict *pCVar5;
  uint uVar6;
  IChangeInfoAccessor *pIVar7;
  int iVar8;
  int unaff_EBX;
  float fVar9;
  float fVar10;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_pWallCollideable != (CPhysCollide *)0x0) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d2642) + 0x44))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4d2642),this->m_pWallCollideable);
    this->m_pWallCollideable = (CPhysCollide *)0x0;
  }
  if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
      super_CBaseEntity.m_pPhysicsObject != (IPhysicsObject *)0x0) {
    (*(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
      super_CBaseEntity.super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
      _vptr_IHandleEntity[0xa1])(this);
  }
  pfVar3 = *(float **)(unaff_EBX + 0x4d2512);
  fVar10 = *pfVar3;
  fVar9 = pfVar3[1];
  fVar4 = pfVar3[2];
  if ((fVar10 == (this->m_vWorldSpace_WallMins).m_Value.x) &&
     (!NAN(fVar10) && !NAN((this->m_vWorldSpace_WallMins).m_Value.x))) {
    pvVar2 = &(this->m_vWorldSpace_WallMins).m_Value.y;
    if ((fVar9 == *pvVar2) &&
       ((!NAN(fVar9) && !NAN(*pvVar2) && (fVar4 == (this->m_vWorldSpace_WallMins).m_Value.z))))
    goto LAB_006d977a;
  }
  if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
      super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
    pCVar5 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
               super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if (pCVar5 != (CBaseEdict *)0x0) {
      pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
      pIVar7 = CBaseEdict::GetChangeAccessor(pCVar5);
      pIVar7->m_iChangeInfoSerialNumber = 0;
    }
  }
  else {
    puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
              super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  (this->m_vWorldSpace_WallMins).m_Value.x = fVar10;
  (this->m_vWorldSpace_WallMins).m_Value.y = fVar9;
  (this->m_vWorldSpace_WallMins).m_Value.z = fVar4;
LAB_006d977a:
  fVar10 = (this->m_vWorldSpace_WallMins).m_Value.x;
  fVar9 = (this->m_vWorldSpace_WallMins).m_Value.y;
  fVar4 = (this->m_vWorldSpace_WallMins).m_Value.z;
  if ((((fVar10 != (this->m_vWorldSpace_WallMaxs).m_Value.x) ||
       (NAN(fVar10) || NAN((this->m_vWorldSpace_WallMaxs).m_Value.x))) ||
      (pvVar2 = &(this->m_vWorldSpace_WallMaxs).m_Value.y, fVar9 != *pvVar2)) ||
     ((NAN(fVar9) || NAN(*pvVar2) || (fVar4 != (this->m_vWorldSpace_WallMaxs).m_Value.z)))) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar5 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                 super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar5 != (CBaseEdict *)0x0) {
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar5);
        pIVar7->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_vWorldSpace_WallMaxs).m_Value.x = fVar10;
    (this->m_vWorldSpace_WallMaxs).m_Value.y = fVar9;
    (this->m_vWorldSpace_WallMaxs).m_Value.z = fVar4;
  }
  fVar10 = (this->m_flHeight).m_Value;
  if (fVar10 != 0.0) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar5 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                 super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar5 != (CBaseEdict *)0x0) {
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar5);
        pIVar7->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flHeight).m_Value = 0.0;
    fVar10 = 0.0;
  }
  fVar9 = (this->m_flWidth).m_Value;
  if (fVar9 != fVar10) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar5 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                 super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar5 != (CBaseEdict *)0x0) {
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar5);
        pIVar7->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flWidth).m_Value = fVar10;
    fVar9 = fVar10;
  }
  if (fVar9 != (this->m_flLength).m_Value) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar5 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                 super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar5 != (CBaseEdict *)0x0) {
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar5);
        pIVar7->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flLength).m_Value = fVar9;
  }
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
  uVar6 = (this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
          m_hHitPortal.
          super_CNetworkVarBase<CBaseHandle,CBaseProjectedEntity::NetworkVar_m_hHitPortal>.m_Value.
          m_Index;
  if (((uVar6 != 0xffffffff) &&
      (iVar8 = (uVar6 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d2506),
      *(uint *)(iVar8 + 8) == uVar6 >> 0x10)) && (*(int *)(iVar8 + 4) != 0)) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar5 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                 super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar5 != (CBaseEdict *)0x0) {
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar5);
        pIVar7->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.m_hHitPortal.
    super_CNetworkVarBase<CBaseHandle,CBaseProjectedEntity::NetworkVar_m_hHitPortal>.m_Value.m_Index
         = 0xffffffff;
    return;
  }
  return;
}


/* CProjectedWallEntity::GetProjectionExtents at 006d9120 */

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


/* CProjectedWallEntity::CreateVPhysics at 006db040 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: bool CreateVPhysics(CProjectedWallEntity * this) */

bool __thiscall CProjectedWallEntity::CreateVPhysics(CProjectedWallEntity *this)

{
  ProjectWall(this);
  return true;
}


/* CProjectedWallEntity::TestCollision at 006d8e70 */

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
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d2e5c) + 0x88))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4d2e5c),ray,mask,0,this->m_pWallCollideable,
               *(undefined4 *)(&DAT_004d2d2c + unaff_EBX),*(undefined4 *)(unaff_EBX + 0x4d2dc0),
               trace);
    pfVar1 = &(trace->super_CBaseTrace).fraction;
    if ((*pfVar1 <= *(float *)(&DAT_0035148c + unaff_EBX) &&
         *(float *)(&DAT_0035148c + unaff_EBX) != *pfVar1) ||
       ((trace->super_CBaseTrace).allsolid != false)) {
      uVar2 = 1;
    }
    else {
      uVar2 = (trace->super_CBaseTrace).startsolid;
    }
  }
  return (bool)uVar2;
}


/* CProjectedWallEntity::ComputeWorldSpaceSurroundingBox at 006d9330 */

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


/* CProjectedWallEntity::DrawWallBoundsHighlight at 006d9030 */

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
    iVar1 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d2ca2) + 0xa8))
                      ((int *)**(undefined4 **)(unaff_EBX + 0x4d2ca2),this->m_pWallCollideable,
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
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d2ca2) + 0xac))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4d2ca2),iVar1,local_20[0]);
  }
  return;
}


/* CProjectedWallEntity::NotifyPortalEvent at 006d8fe0 */

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


/* CProjectedWallEntity::SetSourcePortal at 006d94c0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void SetSourcePortal(CProjectedWallEntity * this, CPortal_Base2D *
   pPortal) */

void __thiscall
CProjectedWallEntity::SetSourcePortal(CProjectedWallEntity *this,CPortal_Base2D *pPortal)

{
  undefined1 *puVar1;
  uint uVar2;
  CBaseEdict *pCVar3;
  int iVar4;
  uint *puVar5;
  int iVar6;
  IChangeInfoAccessor *pIVar7;
  int unaff_EBX;
  float fVar8;
  uint local_20;
  
  ___i686_get_pc_thunk_bx();
  if (pPortal == (CPortal_Base2D *)0x0) {
    iVar4 = (*(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
              super_CBaseEntity.super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[0xc9])(this);
    if (iVar4 == 0) goto joined_r0x006d9502;
  }
  else {
    iVar4 = (*(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
              super_CBaseEntity.super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[0xca])(this,pPortal);
    if ((char)iVar4 == '\0') goto joined_r0x006d9502;
  }
  fVar8 = *(float *)(unaff_EBX + 0x351032) + *(float *)(**(int **)(unaff_EBX + 0x4d26d2) + 0xc);
  if (fVar8 != (this->m_flParticleUpdateTime).m_Value) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar3 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                 super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar3 != (CBaseEdict *)0x0) {
        pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar3);
        pIVar7->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flParticleUpdateTime).m_Value = fVar8;
  }
joined_r0x006d9502:
  if (pPortal == (CPortal_Base2D *)0x0) {
    local_20 = 0xffffffff;
  }
  else {
    puVar5 = (uint *)(*(pPortal->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.
                       super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[3])(pPortal);
    local_20 = *puVar5;
  }
  if ((local_20 == 0xffffffff) ||
     (iVar4 = (local_20 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d26d6),
     *(uint *)(iVar4 + 8) != local_20 >> 0x10)) {
    iVar4 = 0;
  }
  else {
    iVar4 = *(int *)(iVar4 + 4);
  }
  uVar2 = (this->m_hColorPortal).m_Value.super_CBaseHandle.m_Index;
  if ((uVar2 == 0xffffffff) ||
     (iVar6 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d26d6),
     *(uint *)(iVar6 + 8) != uVar2 >> 0x10)) {
    iVar6 = 0;
  }
  else {
    iVar6 = *(int *)(iVar6 + 4);
  }
  if (iVar4 != iVar6) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar3 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                 super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar3 != (CBaseEdict *)0x0) {
        pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar3);
        pIVar7->m_iChangeInfoSerialNumber = 0;
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


/* CProjectedWallEntity::SetHitPortal at 006db060 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void SetHitPortal(CProjectedWallEntity * this, CPortal_Base2D *
   pPortal) */

void __thiscall
CProjectedWallEntity::SetHitPortal(CProjectedWallEntity *this,CPortal_Base2D *pPortal)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  int iVar2;
  IChangeInfoAccessor *pIVar3;
  int unaff_EBX;
  float fVar4;
  
  ___i686_get_pc_thunk_bx();
  if (pPortal == (CPortal_Base2D *)0x0) {
    iVar2 = (*(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
              super_CBaseEntity.super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[199])(this);
    if (iVar2 == 0) goto LAB_006db0e8;
  }
  else {
    iVar2 = (*(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
              super_CBaseEntity.super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[0xca])(this,pPortal);
    if ((char)iVar2 == '\0') goto LAB_006db0e8;
  }
  fVar4 = *(float *)(unaff_EBX + 0x34f48c) + *(float *)(**(int **)(unaff_EBX + 0x4d0b2c) + 0xc);
  if (fVar4 != (this->m_flParticleUpdateTime).m_Value) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                  super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar3->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flParticleUpdateTime).m_Value = fVar4;
  }
LAB_006db0e8:
  CBaseProjectedEntity::SetHitPortal((CBaseProjectedEntity *)this,pPortal);
  return;
}


/* CTEWallPaintedEvent::~CTEWallPaintedEvent at 006db480 */

/* DWARF original prototype: void ~CTEWallPaintedEvent(CTEWallPaintedEvent * this, int __in_chrg) */

void __thiscall CTEWallPaintedEvent::~CTEWallPaintedEvent(CTEWallPaintedEvent *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseTempEntity)._vptr_CBaseTempEntity = (_func_int_varargs **)(unaff_EBX + 0x55c0f7)
  ;
  CBaseTempEntity::~CBaseTempEntity(&this->super_CBaseTempEntity,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CTEWallPaintedEvent::~CTEWallPaintedEvent at 006db4c0 */

/* DWARF original prototype: void ~CTEWallPaintedEvent(CTEWallPaintedEvent * this, int __in_chrg) */

void __thiscall CTEWallPaintedEvent::~CTEWallPaintedEvent(CTEWallPaintedEvent *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CBaseTempEntity)._vptr_CBaseTempEntity =
       (_func_int_varargs **)(extraout_ECX + 0x55c0c0);
  CBaseTempEntity::~CBaseTempEntity(&this->super_CBaseTempEntity,__in_chrg);
  return;
}


/* __tcf_5 at 0099b950 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  *(int *)(extraout_ECX + 0x41e9f4) = extraout_ECX + 0x29bc30;
  CBaseTempEntity::~CBaseTempEntity((CBaseTempEntity *)(extraout_ECX + 0x41e9f4),in_stack_00000008);
  return;
}


/* ServerClassInit<DT_TEWallPaintedEvent::ignored> at 000c72c0 */

int ServerClassInit<DT_TEWallPaintedEvent::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)s_pRandomFloats + unaff_EBX + 0x3f54) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf3094);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)((int)s_pRandomFloats + unaff_EBX + 0x3f6c),
                  (char *)(unaff_EBX + 0x8eb750),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropEHandle((SendProp_conflict *)((int)s_pRandomFloats + unaff_EBX + 0x3fc0),
                      (char *)(unaff_EBX + 0x949e68),0xc,4,-1,
                      *(SendVarProxyFn_conflict *)(&DAT_00ae4944 + unaff_EBX));
      SendPropInt((SendProp_conflict *)(unaff_EBX + 0xcf3154),(char *)(unaff_EBX + 0x949e72),0x10,4,
                  -1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropInt((SendProp_conflict *)
                  ((int)&CUtlRBTree<CUtlMap<unsigned_int,char_const*,unsigned_short>::Node_t,unsigned_short,CUtlMap<unsigned_int,char_const*,unsigned_short>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<unsigned_int,char_const*,unsigned_short>::Node_t,unsigned_short>,unsigned_short>>
                         ::Links(unsigned_short)::s_Sentinel + unaff_EBX),
                  (char *)(unaff_EBX + 0x949e7f),0x14,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      ___cxa_guard_release(unaff_EBX + 0xcf3094);
      ___cxa_atexit(unaff_EBX + 0x8d44fc,0,*(undefined4 *)(&DAT_00ae48bc + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)((int)&CProportionalFloatProperty::typeinfo + unaff_EBX),
             (SendProp_conflict *)((int)s_pRandomFloats + unaff_EBX + 0x3fc0),3,
             *(char **)(&DAT_00b70684 + unaff_EBX));
  return 1;
}


/* CTEWallPaintedEvent::GetServerClass at 006d8f10 */

/* DWARF original prototype: ServerClass * GetServerClass(CTEWallPaintedEvent * this) */

ServerClass * __thiscall CTEWallPaintedEvent::GetServerClass(CTEWallPaintedEvent *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x6e1420);
}


/* CTEWallPaintedEvent::YouForgotToImplementOrDeclareServerClass at 006d8f20 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CTEWallPaintedEvent *
   this) */

int __thiscall
CTEWallPaintedEvent::YouForgotToImplementOrDeclareServerClass(CTEWallPaintedEvent *this)

{
  return 0;
}


/* __tcf_4 at 0099b830 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x393fe0),in_stack_00000008);
  return;
}


/* __tcf_2 at 0099b7d0 */

void __tcf_2(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41ec9b))(unaff_EBX + 0x41ec9b);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41ec47))(unaff_EBX + 0x41ec47);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41ebf3))(unaff_EBX + 0x41ebf3);
                    /* WARNING: Could not recover jumptable at 0x0099b828. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41eb9f))();
  return;
}


/* WallPainted at 006d93a0 */

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
  uVar1 = *(uint *)(unaff_EBX + 0x6e0fa4);
  if ((uVar1 == 0xffffffff) ||
     (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d27f0),
     *(uint *)(iVar2 + 8) != uVar1 >> 0x10)) {
    pCVar3 = (CBaseEntity *)0x0;
  }
  else {
    pCVar3 = *(CBaseEntity **)(iVar2 + 4);
  }
  if (pCVar3 != pWall) {
    if (pWall == (CBaseEntity *)0x0) {
      *(undefined4 *)(unaff_EBX + 0x6e0fa4) = 0xffffffff;
    }
    else {
      puVar4 = (undefined4 *)
               (*(pWall->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                 _vptr_IHandleEntity[3])(pWall);
      *(undefined4 *)(unaff_EBX + 0x6e0fa4) = *puVar4;
    }
  }
  *(int *)(unaff_EBX + 0x6e0fa8) = colorIndex;
  *(int *)(unaff_EBX + 0x6e0fac) = nSegment;
  __in_chrg = &local_3c;
  CBaseTempEntity::Create
            ((CBaseTempEntity *)(unaff_EBX + 0x6e0f98),&local_3c.super_IRecipientFilter,0.0);
  CRecipientFilter::~CRecipientFilter(&local_3c,(int)__in_chrg);
  return;
}


/* CProjectedWallEntity::IsWallPainted at 006d8f70 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: bool IsWallPainted(CProjectedWallEntity * this, Vector * vecPosition)
    */

bool __thiscall CProjectedWallEntity::IsWallPainted(CProjectedWallEntity *this,Vector *vecPosition)

{
  PaintPowerType PVar1;
  
  PVar1 = GetPaintPowerAtPoint(this,vecPosition);
  return PVar1 != NO_POWER;
}


/* CProjectedWallEntity::GetNumSegments at 006d8f40 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: int GetNumSegments(CProjectedWallEntity * this) */

int __thiscall CProjectedWallEntity::GetNumSegments(CProjectedWallEntity *this)

{
  return (this->m_nNumSegments).m_Value;
}


/* CProjectedWallEntity::GetSegmentLength at 006d8f30 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: float GetSegmentLength(CProjectedWallEntity * this) */

float __thiscall CProjectedWallEntity::GetSegmentLength(CProjectedWallEntity *this)

{
  return (this->m_flSegmentLength).m_Value;
}


/* CProjectedWallEntity::GetPaintPowerAtSegment at 006d8f50 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: PaintPowerType GetPaintPowerAtSegment(CProjectedWallEntity * this, int
   i) */

PaintPowerType __thiscall
CProjectedWallEntity::GetPaintPowerAtSegment(CProjectedWallEntity *this,int i)

{
  return (this->m_PaintPowers).m_Memory.m_pMemory[i];
}


/* CProjectedWallEntity::CreateNewInstance at 006d8fa0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

CProjectedWallEntity * CProjectedWallEntity::CreateNewInstance(void)

{
  CProjectedWallEntity *pCVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  pCVar1 = (CProjectedWallEntity *)CreateEntityByName((char *)(unaff_EBX + 0x2e0238),-1,true);
  return pCVar1;
}


/* CProjectedWallEntity::CreateNewProjectedEntity at 006d9480 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: CBaseProjectedEntity * CreateNewProjectedEntity(CProjectedWallEntity *
   this) */

CBaseProjectedEntity * __thiscall
CProjectedWallEntity::CreateNewProjectedEntity(CProjectedWallEntity *this)

{
  CBaseProjectedEntity *pCVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  pCVar1 = (CBaseProjectedEntity *)CreateEntityByName((char *)(unaff_EBX + 0x2dfd58),-1,true);
  return pCVar1;
}


/* CProjectedWallEntity::PlacePlacementHelper at 006d9240 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
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
              (pHelper,unaff_EBX + 0x2fdb46,unaff_EBX + 0x337fda);
    (*(pHelper->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
      super_IHandleEntity._vptr_IHandleEntity[0x20])
              (pHelper,unaff_EBX + 0x337fde,unaff_EBX + 0x2da7ee);
    (*(pHelper->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
      super_IHandleEntity._vptr_IHandleEntity[0x13])(pHelper,this);
  }
  return;
}


/* CProjectedWallEntity::OnProjected at 006dafa0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void OnProjected(CProjectedWallEntity * this) */

void __thiscall CProjectedWallEntity::OnProjected(CProjectedWallEntity *this)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  int unaff_EBX;
  float fVar3;
  
  ___i686_get_pc_thunk_bx();
  CBaseProjectedEntity::OnProjected((CBaseProjectedEntity *)this);
  ProjectWall(this);
  fVar3 = *(float *)(unaff_EBX + 0x34f54f) + *(float *)(**(int **)(unaff_EBX + 0x4d0bef) + 0xc);
  if (fVar3 != (this->m_flParticleUpdateTime).m_Value) {
    if ((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
        super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                  super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar2->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
                super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flParticleUpdateTime).m_Value = fVar3;
  }
  return;
}


/* _GLOBAL__I__ZN20CProjectedWallEntity9m_DataMapE at 000c7eb0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN20CProjectedWallEntity9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

