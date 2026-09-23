/* DWARF-guided pseudocode for game/server/portal2/env_portal_laser.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000a8aa0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "TreeNodeText": ignoring multiple overlapping fields */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  int *piVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  int iVar4;
  IEntityFactoryDictionary *pIVar5;
  datamap_t *pdVar6;
  int unaff_EBX;
  undefined4 *puVar7;
  longlong lVar8;
  ConVar *in_stack_ffffffd4;
  ConVar *this_1;
  char *in_stack_ffffffd8;
  SendTable *this;
  char *in_stack_ffffffdc;
  char *s2;
  int in_stack_ffffffe0;
  undefined4 *flags_1;
  
  lVar8 = ___i686_get_pc_thunk_bx();
  if (lVar8 == 0xffff00000001) {
    *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4f8].m_pPrev + unaff_EBX) =
         0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4f8].m_pPrev + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4f8].m_pPrev + unaff_EBX + 2) = 0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4f8].m_pPrev + unaff_EBX + 3) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4f8].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4f9].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4f9].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4f9].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4f9].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4fa].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4fa].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4fa].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4fa].m_pNext + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4fb].m_pEntity + unaff_EBX)
         = 0x7f7fffff;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4fb].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4fb].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4fb].m_pNext + unaff_EBX) =
         &UNK_00ba3854 + unaff_EBX;
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<CAI_Expresser>::dataDesc[3].flatOffset + unaff_EBX + -8),
                   (char *)(CUtlLinkedList<vgui::TreeNode*,int,false,int,CUtlMemory<UtlLinkedListElem_t<vgui::TreeNode*,_int>,_int>_>
                            ::~CUtlLinkedList + unaff_EBX + 2),(char *)(unaff_EBX + 0x9ae1c8),0x4000
                   ,in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0);
    ___cxa_atexit(unaff_EBX + 0x99358c,0,*(undefined4 *)(&DAT_00b9e0a0 + unaff_EBX));
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<CAI_Expresser>::dataDesc[5].flatOffset + unaff_EBX + -0x28),
                   (char *)(unaff_EBX + 0xa079bf),(char *)(unaff_EBX + 0x9ae1c8),0x4002,
                   in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0);
    ___cxa_atexit(unaff_EBX + 0x99356c,0,*(undefined4 *)(&DAT_00b9e0a0 + unaff_EBX));
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<CAI_Expresser>::dataDesc[6].flatOffset + unaff_EBX + -8),
                   (char *)(unaff_EBX + 0xa079d2),(char *)(unaff_EBX + 0x9b9038),0x4002,
                   in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0);
    ___cxa_atexit(unaff_EBX + 0x99354c,0,*(undefined4 *)(&DAT_00b9e0a0 + unaff_EBX));
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<ConceptHistory_t>::dataDesc[1].flatOffset + unaff_EBX + -0x28),
                   (char *)(unaff_EBX + 0xa079f4),(char *)(unaff_EBX + 0xa079ed),0x4002,
                   in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0);
    ___cxa_atexit(unaff_EBX + 0x99352c,0,*(undefined4 *)(&DAT_00b9e0a0 + unaff_EBX));
    ConVar::ConVar((ConVar *)((int)&CAI_SpeechFilter::m_DataMap.baseMap + unaff_EBX),
                   (char *)(unaff_EBX + 0xa07a19),&UNK_009a634c + unaff_EBX,0x4002,in_stack_ffffffd4
                   ,in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0);
    ___cxa_atexit(unaff_EBX + 0x99350c,0,*(undefined4 *)(&DAT_00b9e0a0 + unaff_EBX));
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<CAI_SpeechFilter>::dataDesc[1].flatOffset + unaff_EBX + -0x28),
                   (char *)(unaff_EBX + 0xa07a28),&UNK_009a634c + unaff_EBX,0x4002,in_stack_ffffffd4
                   ,in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0);
    ___cxa_atexit(unaff_EBX + 0x9934ec,0,*(undefined4 *)(&DAT_00b9e0a0 + unaff_EBX));
    this_1 = (ConVar *)
             (&UNK_00004fc4 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4);
    s2 = &UNK_00a07993 + unaff_EBX;
    *(char **)(&UNK_00004fc4 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
         s2;
    this = (SendTable *)
           ((int)DataMapInit<CAI_SpeechFilter>::dataDesc[2].flatOffset + unaff_EBX + -0x10);
    *(SendTable **)
     (&UNK_00004fc8 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = this;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4fd].m_pEntity + unaff_EBX)
         = 0xffff;
    piVar1 = *(int **)(&DAT_00b9e140 + unaff_EBX);
    flags_1 = (undefined4 *)*piVar1;
    if (flags_1 == (undefined4 *)0x0) {
      *piVar1 = (int)this_1;
      *(undefined4 *)
       (&UNK_00004fcc + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
    }
    else {
      puVar2 = (undefined4 *)flags_1[2];
      iVar4 = _V_stricmp((char *)*flags_1,s2);
      puVar3 = flags_1;
      if (iVar4 < 1) {
        while ((puVar7 = puVar3, puVar3 = puVar2, puVar3 != (undefined4 *)0x0 &&
               (iVar4 = _V_stricmp((char *)*puVar3,s2), flags_1 = puVar7, iVar4 < 1))) {
          puVar2 = (undefined4 *)puVar3[2];
        }
        *(undefined4 **)&(this_1->super_ConCommandBase).m_bRegistered = puVar3;
        puVar7[2] = this_1;
      }
      else {
        *(int *)&(this_1->super_ConCommandBase).m_bRegistered = *piVar1;
        *piVar1 = (int)this_1;
      }
    }
    SendTable::SendTable(this);
    ___cxa_atexit(&UNK_009935ac + unaff_EBX,0,*(undefined4 *)(&DAT_00b9e0a0 + unaff_EBX));
    iVar4 = ServerClassInit<DT_PortalLaser::ignored>((ignored *)0x0);
    *(int *)((int)DataMapInit<CAI_SpeechFilter>::dataDesc[2].flatOffset + unaff_EBX + 4) = iVar4;
    *(undefined **)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4fd].m_SerialNumber + unaff_EBX) =
         &DAT_00c1f544 + unaff_EBX;
    pIVar5 = EntityFactoryDictionary();
    (**pIVar5->_vptr_IEntityFactoryDictionary)
              (pIVar5,unaff_EBX + 0xda2bb8,&UNK_00a07a3e + unaff_EBX);
    pdVar6 = DataMapInit<CPortalLaser>((CPortalLaser *)0x0);
    *(datamap_t **)((int)DataMapInit<CAI_SpeechFilter>::dataDesc[2].flatOffset + unaff_EBX + 8) =
         pdVar6;
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<CAI_SpeechFilter>::dataDesc[3].flatOffset + unaff_EBX + -0x28),
                   (char *)(unaff_EBX + 0xa07a53),(char *)(unaff_EBX + 0xa07a4f),2,this_1,
                   (char *)this,s2,(int)flags_1);
    ___cxa_atexit(unaff_EBX + 0x9934cc,0,*(undefined4 *)(&DAT_00b9e0a0 + unaff_EBX));
  }
  return;
}


/* __tcf_2 at 00a3c040 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38c0d8),in_stack_00000008);
  return;
}


/* __tcf_3 at 00a3c020 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38c158),in_stack_00000008);
  return;
}


/* __tcf_4 at 00a3c000 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38c1d8),in_stack_00000008);
  return;
}


/* __tcf_5 at 00a3bfe0 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38c258),in_stack_00000008);
  return;
}


/* __tcf_6 at 00a3bfc0 */

void __tcf_6(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38c2d8),in_stack_00000008);
  return;
}


/* __tcf_7 at 00a3bfa0 */

void __tcf_7(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38c358),in_stack_00000008);
  return;
}


/* ServerClassInit<DT_PortalLaser::ignored> at 000a8600 */

int ServerClassInit<DT_PortalLaser::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x54a].m_pNext + unaff_EBX + 2)
      == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xda3092);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x54c].m_pNext + unaff_EBX + 2
                  ),(char *)(unaff_EBX + 0x9ad626),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x552].m_pEntity +
                        unaff_EBX + 2),(char *)(unaff_EBX + 0x9af9d6),0,
                        (SendTable *)**(undefined4 **)(&DAT_00b9e5d6 + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00b9e5e2 + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x557].m_SerialNumber +
                     unaff_EBX + 2),
                     (char *)(vgui::TreeView::PanelMessageFunc_OnSliderMoved::InitVar +
                             unaff_EBX + 2),0x7c4,0xc,2,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00b9e62e + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x55c].m_pPrev +
                     unaff_EBX + 2),(char *)(unaff_EBX + 0xa07df3),2000,0xc,0x20,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00b9e62e + unaff_EBX),0x80);
      SendPropBool((SendProp_conflict *)
                   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x561].m_pNext +
                   unaff_EBX + 2),&UNK_00a07e0a + unaff_EBX,0x7dc,1);
      ___cxa_guard_release(unaff_EBX + 0xda3092);
      ___cxa_atexit(unaff_EBX + 0x993902,0,*(undefined4 *)(&DAT_00b9e546 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)
             ((int)DataMapInit<CAI_TacticalServices>::dataDesc[1].flatOffset + unaff_EBX + -0x2a),
             (SendProp_conflict *)
             ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x552].m_pEntity + unaff_EBX + 2),4
             ,*(char **)(&DAT_00c1f9ca + unaff_EBX));
  return 1;
}


/* CPortalLaser::GetServerClass at 00709020 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: ServerClass * GetServerClass(CPortalLaser * this) */

ServerClass * __thiscall CPortalLaser::GetServerClass(CPortalLaser *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x742630);
}


/* CPortalLaser::YouForgotToImplementOrDeclareServerClass at 00709030 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CPortalLaser * this) */

int __thiscall CPortalLaser::YouForgotToImplementOrDeclareServerClass(CPortalLaser *this)

{
  return 0;
}


/* __tcf_8 at 00a3c060 */

void __tcf_8(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x38c2f0),in_stack_00000008);
  return;
}


/* __tcf_0 at 00a3bf10 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f8f3))(unaff_EBX + 0x40f8f3);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f89f))(unaff_EBX + 0x40f89f);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f84b))(unaff_EBX + 0x40f84b);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f7f7))(unaff_EBX + 0x40f7f7);
                    /* WARNING: Could not recover jumptable at 0x00a3bf72. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f7a3))();
  return;
}


/* DataMapInit<CPortalLaser> at 000a8890 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */

datamap_t * DataMapInit<CPortalLaser>(CPortalLaser *param_1)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  char *pcVar4;
  size_t sVar5;
  undefined4 *puVar6;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x51e].m_pPrev + unaff_EBX) ==
       '\0') && (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xda2dcc), iVar3 != 0)) {
    *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x51f].m_pPrev + unaff_EBX) =
         unaff_EBX + 0xa07ba3;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x520].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x520].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x520].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x520].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x521].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x51f].m_pNext + unaff_EBX) =
         0xc;
    ___cxa_guard_release(unaff_EBX + 0xda2dcc);
    ___cxa_atexit(unaff_EBX + 0x9937dc,0,*(undefined4 *)(&DAT_00b9e2b0 + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_NetworkStateChanged_m_flFriction_00cba2d4 + unaff_EBX) =
       *(undefined4 *)(&DAT_00b9e2bc + unaff_EBX);
  if ((*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x51f].m_pEntity + unaff_EBX) ==
       '\0') && (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xda2dd4), iVar3 != 0)) {
    pcVar4 = operator_new___(*(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x51f].
                                            m_pNext + unaff_EBX) + 0xc);
    _strcpy(pcVar4,*(char **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x51f].m_pPrev +
                             unaff_EBX));
    sVar5 = _strlen(pcVar4);
    builtin_strncpy(pcVar4 + sVar5,"StrikeThink",0xc);
    uVar1 = *(uint *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x520].m_pNext + unaff_EBX)
    ;
    CUtlVector<char*,CUtlMemory<char*,_int>_>::GrowVector
              ((CUtlVector<char*,CUtlMemory<char*,_int>_> *)
               ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x520].m_pEntity + unaff_EBX),1);
    iVar3 = ~uVar1 + *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x520].m_pNext +
                             unaff_EBX);
    if (0 < iVar3) {
      iVar2 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x520].m_pEntity +
                      unaff_EBX);
      _V_memmove((void *)(iVar2 + 4 + uVar1 * 4),(void *)(iVar2 + uVar1 * 4),iVar3 * 4);
    }
    puVar6 = (undefined4 *)
             (uVar1 * 4 +
             *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x520].m_pEntity +
                     unaff_EBX));
    if (puVar6 != (undefined4 *)0x0) {
      *puVar6 = pcVar4;
    }
    *(char **)((int)&PTR_typeinfo_name_00cba340 + unaff_EBX) = pcVar4;
    ___cxa_guard_release(unaff_EBX + 0xda2dd4);
  }
  *(undefined4 *)((int)&PTR_NetworkStateChanged_m_vecBaseVelocity_00cba2cc + unaff_EBX) = 0xf;
  *(int *)((int)&PTR_NetworkStateChanged_m_hGroundEntity_00cba2c8 + unaff_EBX) =
       (int)&CEntityFactory<CEnv_Lightrail_Endpoint>::typeinfo + unaff_EBX;
  return (datamap_t *)((int)&PTR_NetworkStateChanged_m_hGroundEntity_00cba2c8 + unaff_EBX);
}


/* CPortalLaser::GetDataDescMap at 00709040 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: datamap_t * GetDataDescMap(CPortalLaser * this) */

datamap_t * __thiscall CPortalLaser::GetDataDescMap(CPortalLaser *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x659b24);
}


/* CPortalLaser::GetBaseMap at 00709050 */

datamap_t * CPortalLaser::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x53db08);
}


/* __tcf_1 at 00a3c080 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x40f606)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x40f5fa) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x40f606));
  }
  *(undefined4 *)(unaff_EBX + 0x40f606) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40f602)) {
    if (*(int *)(unaff_EBX + 0x40f5fa) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20aace) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20aace),*(int *)(unaff_EBX + 0x40f5fa));
      *(undefined4 *)(unaff_EBX + 0x40f5fa) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f5fe) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40f5fa);
  *(int *)(unaff_EBX + 0x40f60a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40f602)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20aace) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20aace),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40f5fa) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f5fe) = 0;
  }
  return;
}


/* CPortalLaser::CPortalLaser at 007098d0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void CPortalLaser(CPortalLaser * this) */

void __thiscall CPortalLaser::CPortalLaser(CPortalLaser *this)

{
  undefined1 *puVar1;
  uint uVar2;
  int iVar3;
  ushort uVar4;
  vec_t *pvVar5;
  vec_t vVar6;
  vec_t vVar7;
  CBaseEdict *pCVar8;
  int *piVar9;
  ushort *puVar10;
  IChangeInfoAccessor *pIVar11;
  uint uVar12;
  int unaff_EBX;
  uint uVar13;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::CBaseAnimating(&this->super_CBaseAnimating);
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5be34a);
  this->m_pChildLaser = (CPortalLaser *)0x0;
  this->m_bFromReflectedCube = false;
  (this->m_hReflector).super_CBaseHandle.m_Index = 0xffffffff;
  pvVar5 = *(vec_t **)(unaff_EBX + 0x53d29a);
  vVar6 = pvVar5[1];
  vVar7 = *pvVar5;
  (this->m_vecLaserEndPos).m_Value.z = pvVar5[2];
  (this->m_vecLaserEndPos).m_Value.y = vVar6;
  (this->m_vecLaserEndPos).m_Value.x = vVar7;
  if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false)
  {
    pCVar8 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if ((pCVar8 != (CBaseEdict *)0x0) && ((pCVar8->m_fStateFlags & 0x100U) == 0)) {
      pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 1;
      pIVar11 = CBaseEdict::GetChangeAccessor(pCVar8);
      piVar9 = *(int **)(unaff_EBX + 0x53d28a);
      puVar10 = (ushort *)*piVar9;
      if (pIVar11->m_iChangeInfoSerialNumber == *puVar10) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        uVar13 = (uint)pIVar11->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar4 = puVar10[uVar13 * 0x14 + 0x14];
        if (uVar4 == 0) {
LAB_00709c5d:
          puVar10[(uint)uVar4 + uVar13 * 0x14 + 1] = 0x7c4;
          puVar10[uVar13 * 0x14 + 0x14] = uVar4 + 1;
        }
        else if (puVar10[uVar13 * 0x14 + 1] != 0x7c4) {
          uVar12 = 0;
          do {
            uVar2 = uVar12 + 1;
            uVar12 = uVar2 & 0xffff;
            if ((ushort)uVar2 == uVar4) {
              if (uVar4 != 0x13) goto LAB_00709c5d;
              goto LAB_00709cfb;
            }
          } while (puVar10[uVar13 * 0x14 + uVar12 + 1] != 0x7c4);
        }
      }
      else if (puVar10[0x7d1] == 100) {
LAB_00709cfb:
        pIVar11->m_iChangeInfoSerialNumber = 0;
        pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 0x100;
      }
      else {
        pIVar11->m_iChangeInfo = puVar10[0x7d1];
        *(short *)(*piVar9 + 0xfa2) = *(short *)(*piVar9 + 0xfa2) + 1;
        pIVar11->m_iChangeInfoSerialNumber = *(ushort *)*piVar9;
        iVar3 = *piVar9 + (uint)pIVar11->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar3 + 2) = 0x7c4;
        *(undefined2 *)(iVar3 + 0x28) = 1;
      }
    }
  }
  else {
    puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  pvVar5 = *(vec_t **)(unaff_EBX + 0x53d29a);
  vVar6 = pvVar5[1];
  vVar7 = *pvVar5;
  (this->m_vecLaserImpactNormal).m_Value.z = pvVar5[2];
  (this->m_vecLaserImpactNormal).m_Value.y = vVar6;
  (this->m_vecLaserImpactNormal).m_Value.x = vVar7;
  if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false)
  {
    pCVar8 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if ((pCVar8 != (CBaseEdict *)0x0) && ((pCVar8->m_fStateFlags & 0x100U) == 0)) {
      pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 1;
      pIVar11 = CBaseEdict::GetChangeAccessor(pCVar8);
      piVar9 = *(int **)(unaff_EBX + 0x53d28a);
      puVar10 = (ushort *)*piVar9;
      if (pIVar11->m_iChangeInfoSerialNumber == *puVar10) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        uVar13 = (uint)pIVar11->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar4 = puVar10[uVar13 * 0x14 + 0x14];
        if (uVar4 == 0) {
LAB_00709cd9:
          puVar10[(uint)uVar4 + uVar13 * 0x14 + 1] = 2000;
          puVar10[uVar13 * 0x14 + 0x14] = uVar4 + 1;
        }
        else if (puVar10[uVar13 * 0x14 + 1] != 2000) {
          uVar12 = 0;
          do {
            uVar2 = uVar12 + 1;
            uVar12 = uVar2 & 0xffff;
            if ((ushort)uVar2 == uVar4) {
              if (uVar4 != 0x13) goto LAB_00709cd9;
              goto LAB_00709d12;
            }
          } while (puVar10[uVar13 * 0x14 + uVar12 + 1] != 2000);
        }
      }
      else if (puVar10[0x7d1] == 100) {
LAB_00709d12:
        pIVar11->m_iChangeInfoSerialNumber = 0;
        pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar11->m_iChangeInfo = puVar10[0x7d1];
        *(short *)(*piVar9 + 0xfa2) = *(short *)(*piVar9 + 0xfa2) + 1;
        pIVar11->m_iChangeInfoSerialNumber = *(ushort *)*piVar9;
        iVar3 = *piVar9 + (uint)pIVar11->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar3 + 2) = 2000;
        *(undefined2 *)(iVar3 + 0x28) = 1;
      }
    }
  }
  else {
    puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  (this->m_bShouldSpark).m_Value = false;
  if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false)
  {
    pCVar8 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if ((pCVar8 != (CBaseEdict *)0x0) && ((pCVar8->m_fStateFlags & 0x100U) == 0)) {
      pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 1;
      pIVar11 = CBaseEdict::GetChangeAccessor(pCVar8);
      piVar9 = *(int **)(unaff_EBX + 0x53d28a);
      puVar10 = (ushort *)*piVar9;
      if (pIVar11->m_iChangeInfoSerialNumber == *puVar10) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        uVar13 = (uint)pIVar11->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar4 = puVar10[uVar13 * 0x14 + 0x14];
        if (uVar4 == 0) {
LAB_00709bc6:
          puVar10[(uint)uVar4 + uVar13 * 0x14 + 1] = 0x7dc;
          puVar10[uVar13 * 0x14 + 0x14] = uVar4 + 1;
        }
        else if (puVar10[uVar13 * 0x14 + 1] != 0x7dc) {
          uVar12 = 0;
          do {
            uVar2 = uVar12 + 1;
            uVar12 = uVar2 & 0xffff;
            if ((ushort)uVar2 == uVar4) {
              if (uVar4 != 0x13) goto LAB_00709bc6;
              goto LAB_00709be8;
            }
          } while (puVar10[uVar13 * 0x14 + uVar12 + 1] != 0x7dc);
        }
      }
      else if (puVar10[0x7d1] == 100) {
LAB_00709be8:
        pIVar11->m_iChangeInfoSerialNumber = 0;
        pCVar8->m_fStateFlags = pCVar8->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar11->m_iChangeInfo = puVar10[0x7d1];
        *(short *)(*piVar9 + 0xfa2) = *(short *)(*piVar9 + 0xfa2) + 1;
        pIVar11->m_iChangeInfoSerialNumber = *(ushort *)*piVar9;
        iVar3 = *piVar9 + (uint)pIVar11->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar3 + 2) = 0x7dc;
        *(undefined2 *)(iVar3 + 0x28) = 1;
      }
    }
  }
  else {
    puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  this->m_pBeam = (CBeam *)0x0;
  *(int *)(unaff_EBX + 0x67e0c6) = *(int *)(unaff_EBX + 0x67e0c6) + 1;
  return;
}


/* CPortalLaser::CPortalLaser at 00709d40 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void CPortalLaser(CPortalLaser * this, CPortalLaser * this) */

void __thiscall CPortalLaser::CPortalLaser(CPortalLaser *this,CPortalLaser *this_1)

{
  CPortalLaser(this);
  return;
}


/* CEntityFactory<CPortalLaser>::Create at 00712100 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CPortalLaser> * this, char *
   pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CPortalLaser>::Create(CEntityFactory<CPortalLaser> *this,char *pClassName)

{
  CPortalLaser *this_00;
  
                    /* Unresolved local var: CPortalLaser * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x7e4);
  CPortalLaser::CPortalLaser(this_00);
  (*(this_00->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1d])(this_00,pClassName);
  return &(this_00->super_CBaseAnimating).super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CPortalLaser::~CPortalLaser at 00709320 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void ~CPortalLaser(CPortalLaser * this, int __in_chrg) */

void __thiscall CPortalLaser::~CPortalLaser(CPortalLaser *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5be8f7);
  *(int *)(unaff_EBX + 0x67e673) = *(int *)(unaff_EBX + 0x67e673) + -1;
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CPortalLaser::~CPortalLaser at 00709360 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void ~CPortalLaser(CPortalLaser * this, int __in_chrg) */

void __thiscall CPortalLaser::~CPortalLaser(CPortalLaser *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x5be8c0);
  *(int *)(extraout_ECX + 0x67e63c) = *(int *)(extraout_ECX + 0x67e63c) + -1;
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
  return;
}


/* CPortalLaser::~CPortalLaser at 00709390 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void ~CPortalLaser(CPortalLaser * this, int __in_chrg) */

void __thiscall CPortalLaser::~CPortalLaser(CPortalLaser *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x5be890);
  *(int *)(extraout_ECX + 0x67e60c) = *(int *)(extraout_ECX + 0x67e60c) + -1;
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
  return;
}


/* CPortalLaser::Spawn at 0070df60 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(CPortalLaser * this) */

void __thiscall CPortalLaser::Spawn(CPortalLaser *this)

{
  BASEPTR func;
  int iVar1;
  char *pcVar2;
  char *pcVar3;
  int unaff_EBX;
  float fVar4;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::Spawn(&this->super_CBaseAnimating);
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1a])(this);
  if (this->m_bFromReflectedCube == false) {
    pcVar3 = (this->m_ModelName).pszValue;
    pcVar2 = (char *)(unaff_EBX + 0x340db7);
    if (pcVar3 != (char *)0x0) {
      pcVar2 = pcVar3;
    }
    if (*pcVar2 == '\0') {
      (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
        super_IHandleEntity._vptr_IHandleEntity[0x1b])(this,unaff_EBX + 0x3a24bb);
    }
    else {
      if (pcVar3 == (char *)0x0) {
        pcVar3 = (char *)(unaff_EBX + 0x340db7);
      }
      (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
        super_IHandleEntity._vptr_IHandleEntity[0x1b])(this,pcVar3);
    }
    CCollisionProperty::SetSolid
              (&(this->super_CBaseAnimating).super_CBaseEntity.m_Collision.super_CCollisionProperty,
               SOLID_VPHYSICS);
    iVar1 = CBaseAnimating::LookupAttachment
                      (&this->super_CBaseAnimating,(char *)(unaff_EBX + 0x3a25d3));
    this->m_iLaserAttachment = iVar1;
    if (iVar1 == 0) {
      pcVar3 = (this->m_ModelName).pszValue;
      if (pcVar3 == (char *)0x0) {
        pcVar3 = (char *)(unaff_EBX + 0x340db7);
      }
      pcVar2 = (this->super_CBaseAnimating).super_CBaseEntity.m_iName.m_Value.pszValue;
      if (pcVar2 == (char *)0x0) {
        pcVar2 = (char *)(unaff_EBX + 0x340db7);
      }
      _Warning(unaff_EBX + 0x3a25e7,pcVar2,pcVar3);
    }
    (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
      super_IHandleEntity._vptr_IHandleEntity[0x9f])(this);
  }
                    /* Unresolved local var: int i@[???] */
  this->m_pAmbientSound[0] = (CSoundPatch *)0x0;
  iVar1 = 1;
  do {
    this->m_pAmbientSound[iVar1] = (CSoundPatch *)0x0;
    iVar1 = iVar1 + 1;
  } while (iVar1 != 0x21);
  CreateHelperEntities(this);
  if ((this->m_bStartOff == false) &&
     ((this->super_CBaseAnimating).super_CBaseEntity.m_pfnThink.__pfn ==
      (_func___thiscall_void_CBaseEntity_ptr *)0x0)) {
    func.__delta = 0;
    func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0x2fa3);
    CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
    if (this->m_bFromReflectedCube == false) {
      fVar4 = *(float *)(*(int *)(unaff_EBX + 0x6ba2ef) + 0x2c);
    }
    else {
      fVar4 = *(float *)(*(int *)(unaff_EBX + 0x6ba28f) + 0x2c);
    }
    CBaseEntity::SetNextThink
              ((CBaseEntity *)this,fVar4 + *(float *)(**(int **)(unaff_EBX + 0x538bf7) + 0xc),
               (char *)0x0);
    TurnOnGlow(this);
    return;
  }
  return;
}


/* CPortalLaser::UpdateTransmitState at 00709300 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: int UpdateTransmitState(CPortalLaser * this) */

int __thiscall CPortalLaser::UpdateTransmitState(CPortalLaser *this)

{
  int iVar1;
  
  iVar1 = CBaseEntity::SetTransmitState((CBaseEntity *)this,0x20);
  return iVar1;
}


/* CPortalLaser::CreateHelperEntities at 0070d030 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void CreateHelperEntities(CPortalLaser * this) */

void __thiscall CPortalLaser::CreateHelperEntities(CPortalLaser *this)

{
  undefined1 *puVar1;
  int iVar2;
  ushort uVar3;
  CBaseEdict *pCVar4;
  ushort *puVar5;
  int *piVar6;
  char *pSpriteName;
  CBeam *pCVar7;
  CInfoPlacementHelper *pCVar8;
  IChangeInfoAccessor *pIVar9;
  uint uVar10;
  uint uVar11;
  int unaff_EBX;
  uint local_108;
  uint local_104;
  uint local_100;
  uint local_fc;
  uint local_f8;
  uint local_f4;
  uint local_f0;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_pBeam == (CBeam *)0x0) {
    pSpriteName = &UNK_00389285 + unaff_EBX;
    if (this->m_bIsLethal == false) {
      pSpriteName = (char *)(unaff_EBX + 0x37b0c9);
    }
    pCVar7 = CBeam::BeamCreate(pSpriteName,2.0);
    this->m_pBeam = pCVar7;
    if (this->m_bIsLethal == false) {
      if (*(float *)(unaff_EBX + 0x3bfbd9) != (pCVar7->m_fWidth).m_Value) {
        if ((pCVar7->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
          pCVar4 = &((pCVar7->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
          if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
            pIVar9 = CBaseEdict::GetChangeAccessor(pCVar4);
            puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x539b21);
            if (pIVar9->m_iChangeInfoSerialNumber == *puVar5) {
              uVar10 = (uint)pIVar9->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
              uVar3 = puVar5[uVar10 * 0x14 + 0x14];
              if (uVar3 == 0) {
LAB_0070dd24:
                puVar5[(uint)uVar3 + uVar10 * 0x14 + 1] = 0x430;
                puVar5[uVar10 * 0x14 + 0x14] = uVar3 + 1;
              }
              else if (puVar5[uVar10 * 0x14 + 1] != 0x430) {
                local_f4 = 0;
                do {
                  uVar11 = local_f4 + 1;
                  local_f4 = uVar11 & 0xffff;
                  if ((ushort)uVar11 == uVar3) {
                    if (uVar3 != 0x13) goto LAB_0070dd24;
                    pIVar9->m_iChangeInfoSerialNumber = 0;
                    pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                    break;
                  }
                } while (puVar5[uVar10 * 0x14 + local_f4 + 1] != 0x430);
              }
            }
            else if (puVar5[0x7d1] == 100) {
              pIVar9->m_iChangeInfoSerialNumber = 0;
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
            }
            else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
              pIVar9->m_iChangeInfo = puVar5[0x7d1];
              piVar6 = *(int **)(unaff_EBX + 0x539b21);
              *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
              pIVar9->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
              iVar2 = *piVar6 + (uint)pIVar9->m_iChangeInfo * 0x28;
              *(undefined2 *)(iVar2 + 2) = 0x430;
              *(undefined2 *)(iVar2 + 0x28) = 1;
            }
          }
        }
        else {
          puVar1 = &(pCVar7->super_CBaseEntity).m_Network.field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (pCVar7->m_fWidth).m_Value = 32.0;
      }
      pCVar7 = this->m_pBeam;
      if (*(float *)(unaff_EBX + 0x3bfbd9) != (pCVar7->m_fEndWidth).m_Value) {
        if ((pCVar7->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
          pCVar4 = &((pCVar7->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
          if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
            pIVar9 = CBaseEdict::GetChangeAccessor(pCVar4);
            puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x539b21);
            if (pIVar9->m_iChangeInfoSerialNumber == *puVar5) {
              uVar10 = (uint)pIVar9->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
              uVar3 = puVar5[uVar10 * 0x14 + 0x14];
              if (uVar3 == 0) {
LAB_0070db73:
                puVar5[(uint)uVar3 + uVar10 * 0x14 + 1] = 0x434;
                puVar5[uVar10 * 0x14 + 0x14] = uVar3 + 1;
              }
              else if (puVar5[uVar10 * 0x14 + 1] != 0x434) {
                local_f0 = 0;
                do {
                  uVar11 = local_f0 + 1;
                  local_f0 = uVar11 & 0xffff;
                  if ((ushort)uVar11 == uVar3) {
                    if (uVar3 != 0x13) goto LAB_0070db73;
                    pIVar9->m_iChangeInfoSerialNumber = 0;
                    pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                    break;
                  }
                } while (puVar5[uVar10 * 0x14 + local_f0 + 1] != 0x434);
              }
            }
            else if (puVar5[0x7d1] == 100) {
              pIVar9->m_iChangeInfoSerialNumber = 0;
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
            }
            else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
              pIVar9->m_iChangeInfo = puVar5[0x7d1];
              piVar6 = *(int **)(unaff_EBX + 0x539b21);
              *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
              pIVar9->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
              iVar2 = *piVar6 + (uint)pIVar9->m_iChangeInfo * 0x28;
              *(undefined2 *)(iVar2 + 2) = 0x434;
              *(undefined2 *)(iVar2 + 0x28) = 1;
            }
          }
        }
        else {
          puVar1 = &(pCVar7->super_CBaseEntity).m_Network.field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (pCVar7->m_fEndWidth).m_Value = 32.0;
        pCVar7 = this->m_pBeam;
      }
    }
    else {
      if ((pCVar7->super_CBaseEntity).m_clrRender.
          super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r != 'd') {
        if ((pCVar7->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
          pCVar4 = &((pCVar7->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
          if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
            pIVar9 = CBaseEdict::GetChangeAccessor(pCVar4);
            puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x539b21);
            if (pIVar9->m_iChangeInfoSerialNumber == *puVar5) {
              uVar10 = (uint)pIVar9->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
              uVar3 = puVar5[uVar10 * 0x14 + 0x14];
              if (uVar3 == 0) {
LAB_0070d8b7:
                puVar5[(uint)uVar3 + uVar10 * 0x14 + 1] = 0x108;
                puVar5[uVar10 * 0x14 + 0x14] = uVar3 + 1;
              }
              else if (puVar5[uVar10 * 0x14 + 1] != 0x108) {
                local_108 = 0;
                do {
                  uVar11 = local_108 + 1;
                  local_108 = uVar11 & 0xffff;
                  if ((ushort)uVar11 == uVar3) {
                    if (uVar3 != 0x13) goto LAB_0070d8b7;
                    pIVar9->m_iChangeInfoSerialNumber = 0;
                    pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                    break;
                  }
                } while (puVar5[uVar10 * 0x14 + local_108 + 1] != 0x108);
              }
            }
            else if (puVar5[0x7d1] == 100) {
              pIVar9->m_iChangeInfoSerialNumber = 0;
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
            }
            else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
              pIVar9->m_iChangeInfo = puVar5[0x7d1];
              piVar6 = *(int **)(unaff_EBX + 0x539b21);
              *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
              pIVar9->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
              iVar2 = *piVar6 + (uint)pIVar9->m_iChangeInfo * 0x28;
              *(undefined2 *)(iVar2 + 2) = 0x108;
              *(undefined2 *)(iVar2 + 0x28) = 1;
            }
          }
        }
        else {
          puVar1 = &(pCVar7->super_CBaseEntity).m_Network.field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (pCVar7->super_CBaseEntity).m_clrRender.
        super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r = 'd';
      }
      if ((pCVar7->super_CBaseEntity).m_clrRender.
          super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g != 0xff) {
        if ((pCVar7->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
          pCVar4 = &((pCVar7->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
          if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
            pIVar9 = CBaseEdict::GetChangeAccessor(pCVar4);
            puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x539b21);
            if (pIVar9->m_iChangeInfoSerialNumber == *puVar5) {
              uVar10 = (uint)pIVar9->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
              uVar3 = puVar5[uVar10 * 0x14 + 0x14];
              if (uVar3 == 0) {
LAB_0070da66:
                puVar5[(uint)uVar3 + uVar10 * 0x14 + 1] = 0x108;
                puVar5[uVar10 * 0x14 + 0x14] = uVar3 + 1;
              }
              else if (puVar5[uVar10 * 0x14 + 1] != 0x108) {
                local_104 = 0;
                do {
                  uVar11 = local_104 + 1;
                  local_104 = uVar11 & 0xffff;
                  if ((ushort)uVar11 == uVar3) {
                    if (uVar3 != 0x13) goto LAB_0070da66;
                    pIVar9->m_iChangeInfoSerialNumber = 0;
                    pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                    break;
                  }
                } while (puVar5[uVar10 * 0x14 + local_104 + 1] != 0x108);
              }
            }
            else if (puVar5[0x7d1] == 100) {
              pIVar9->m_iChangeInfoSerialNumber = 0;
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
            }
            else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
              pIVar9->m_iChangeInfo = puVar5[0x7d1];
              piVar6 = *(int **)(unaff_EBX + 0x539b21);
              *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
              pIVar9->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
              iVar2 = *piVar6 + (uint)pIVar9->m_iChangeInfo * 0x28;
              *(undefined2 *)(iVar2 + 2) = 0x108;
              *(undefined2 *)(iVar2 + 0x28) = 1;
            }
          }
        }
        else {
          puVar1 = &(pCVar7->super_CBaseEntity).m_Network.field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (pCVar7->super_CBaseEntity).m_clrRender.
        super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g = 0xff;
      }
      if ((pCVar7->super_CBaseEntity).m_clrRender.
          super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b != 'd') {
        if ((pCVar7->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
          pCVar4 = &((pCVar7->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
          if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
            pIVar9 = CBaseEdict::GetChangeAccessor(pCVar4);
            puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x539b21);
            if (pIVar9->m_iChangeInfoSerialNumber == *puVar5) {
              uVar10 = (uint)pIVar9->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
              uVar3 = puVar5[uVar10 * 0x14 + 0x14];
              if (uVar3 == 0) {
LAB_0070d990:
                puVar5[(uint)uVar3 + uVar10 * 0x14 + 1] = 0x108;
                puVar5[uVar10 * 0x14 + 0x14] = uVar3 + 1;
              }
              else if (puVar5[uVar10 * 0x14 + 1] != 0x108) {
                local_100 = 0;
                do {
                  uVar11 = local_100 + 1;
                  local_100 = uVar11 & 0xffff;
                  if ((ushort)uVar11 == uVar3) {
                    if (uVar3 != 0x13) goto LAB_0070d990;
                    pIVar9->m_iChangeInfoSerialNumber = 0;
                    pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                    break;
                  }
                } while (puVar5[uVar10 * 0x14 + local_100 + 1] != 0x108);
              }
            }
            else if (puVar5[0x7d1] == 100) {
              pIVar9->m_iChangeInfoSerialNumber = 0;
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
            }
            else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
              pIVar9->m_iChangeInfo = puVar5[0x7d1];
              piVar6 = *(int **)(unaff_EBX + 0x539b21);
              *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
              pIVar9->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
              iVar2 = *piVar6 + (uint)pIVar9->m_iChangeInfo * 0x28;
              *(undefined2 *)(iVar2 + 2) = 0x108;
              *(undefined2 *)(iVar2 + 0x28) = 1;
            }
          }
        }
        else {
          puVar1 = &(pCVar7->super_CBaseEntity).m_Network.field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (pCVar7->super_CBaseEntity).m_clrRender.
        super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b = 'd';
      }
      pCVar7 = this->m_pBeam;
      if (*(float *)(unaff_EBX + 0x3bfe89) != (pCVar7->m_fWidth).m_Value) {
        if ((pCVar7->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
          pCVar4 = &((pCVar7->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
          if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
            pIVar9 = CBaseEdict::GetChangeAccessor(pCVar4);
            puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x539b21);
            if (pIVar9->m_iChangeInfoSerialNumber == *puVar5) {
              uVar10 = (uint)pIVar9->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
              uVar3 = puVar5[uVar10 * 0x14 + 0x14];
              if (uVar3 == 0) {
LAB_0070dc57:
                puVar5[(uint)uVar3 + uVar10 * 0x14 + 1] = 0x430;
                puVar5[uVar10 * 0x14 + 0x14] = uVar3 + 1;
              }
              else if (puVar5[uVar10 * 0x14 + 1] != 0x430) {
                local_fc = 0;
                do {
                  uVar11 = local_fc + 1;
                  local_fc = uVar11 & 0xffff;
                  if ((ushort)uVar11 == uVar3) {
                    if (uVar3 != 0x13) goto LAB_0070dc57;
                    pIVar9->m_iChangeInfoSerialNumber = 0;
                    pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                    break;
                  }
                } while (puVar5[uVar10 * 0x14 + local_fc + 1] != 0x430);
              }
            }
            else if (puVar5[0x7d1] == 100) {
              pIVar9->m_iChangeInfoSerialNumber = 0;
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
            }
            else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
              pIVar9->m_iChangeInfo = puVar5[0x7d1];
              piVar6 = *(int **)(unaff_EBX + 0x539b21);
              *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
              pIVar9->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
              iVar2 = *piVar6 + (uint)pIVar9->m_iChangeInfo * 0x28;
              *(undefined2 *)(iVar2 + 2) = 0x430;
              *(undefined2 *)(iVar2 + 0x28) = 1;
            }
          }
        }
        else {
          puVar1 = &(pCVar7->super_CBaseEntity).m_Network.field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (pCVar7->m_fWidth).m_Value = 2.0;
        pCVar7 = this->m_pBeam;
      }
      if (*(float *)(unaff_EBX + 0x3bfe89) != (pCVar7->m_fEndWidth).m_Value) {
        if ((pCVar7->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
          pCVar4 = &((pCVar7->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
          if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
            pIVar9 = CBaseEdict::GetChangeAccessor(pCVar4);
            puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x539b21);
            if (pIVar9->m_iChangeInfoSerialNumber == *puVar5) {
              uVar10 = (uint)pIVar9->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
              uVar3 = puVar5[uVar10 * 0x14 + 0x14];
              if (uVar3 == 0) {
LAB_0070ddf1:
                puVar5[(uint)uVar3 + uVar10 * 0x14 + 1] = 0x434;
                puVar5[uVar10 * 0x14 + 0x14] = uVar3 + 1;
              }
              else if (puVar5[uVar10 * 0x14 + 1] != 0x434) {
                local_f8 = 0;
                do {
                  uVar11 = local_f8 + 1;
                  local_f8 = uVar11 & 0xffff;
                  if ((ushort)uVar11 == uVar3) {
                    if (uVar3 != 0x13) goto LAB_0070ddf1;
                    pIVar9->m_iChangeInfoSerialNumber = 0;
                    pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                    break;
                  }
                } while (puVar5[uVar10 * 0x14 + local_f8 + 1] != 0x434);
              }
            }
            else if (puVar5[0x7d1] == 100) {
              pIVar9->m_iChangeInfoSerialNumber = 0;
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
            }
            else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
              pIVar9->m_iChangeInfo = puVar5[0x7d1];
              piVar6 = *(int **)(unaff_EBX + 0x539b21);
              *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
              pIVar9->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
              iVar2 = *piVar6 + (uint)pIVar9->m_iChangeInfo * 0x28;
              *(undefined2 *)(iVar2 + 2) = 0x434;
              *(undefined2 *)(iVar2 + 0x28) = 1;
            }
          }
        }
        else {
          puVar1 = &(pCVar7->super_CBaseEntity).m_Network.field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (pCVar7->m_fEndWidth).m_Value = 2.0;
        pCVar7 = this->m_pBeam;
      }
    }
    DispatchSpawn(&pCVar7->super_CBaseEntity,true);
    pCVar8 = this->m_pPlacementHelper;
  }
  else {
    pCVar8 = this->m_pPlacementHelper;
  }
  if (pCVar8 == (CInfoPlacementHelper *)0x0) {
    pCVar8 = (CInfoPlacementHelper *)CreateEntityByName((char *)(unaff_EBX + 0x3a8769),-1,true);
    this->m_pPlacementHelper = pCVar8;
    if (((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
    CBaseEntity::SetAbsOrigin
              ((CBaseEntity *)this->m_pPlacementHelper,
               &(this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin);
    if (((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
    CBaseEntity::SetAbsAngles
              ((CBaseEntity *)this->m_pPlacementHelper,
               &(this->super_CBaseAnimating).super_CBaseEntity.m_angAbsRotation);
    (*(this->m_pPlacementHelper->super_CPointEntity).super_CBaseEntity.super_IServerEntity.
      super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x20])
              (this->m_pPlacementHelper,&UNK_0037f44d + unaff_EBX,unaff_EBX + 0x3a34e1);
    (*(this->m_pPlacementHelper->super_CPointEntity).super_CBaseEntity.super_IServerEntity.
      super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x20])
              (this->m_pPlacementHelper,unaff_EBX + 0x3a34e7,&UNK_00341db9 + unaff_EBX);
    DispatchSpawn((CBaseEntity *)this->m_pPlacementHelper,true);
  }
  CreateSoundProxies(this);
  return;
}


/* CPortalLaser::CreateSoundProxies at 00709670 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void CreateSoundProxies(CPortalLaser * this) */

void __thiscall CPortalLaser::CreateSoundProxies(CPortalLaser *this)

{
  int *piVar1;
  int *piVar2;
  edict_t *peVar3;
  CSoundEnvelopeController *pCVar4;
  CBasePlayer *player;
  CBaseEntity *pCVar5;
  int iVar6;
  CSoundPatch *pCVar7;
  int iVar8;
  int unaff_EBX;
  _func_int_varargs *p_Var9;
  int playerIndex;
  CSoundEnvelopeController *controller;
  CRecipientFilter local_3c;
  int iVar10;
  
  ___i686_get_pc_thunk_bx();
  pCVar4 = CSoundEnvelopeController::GetController();
  piVar2 = *(int **)(unaff_EBX + 0x53d4e6);
  iVar10 = 0;
  do {
    playerIndex = iVar10 + 1;
    player = UTIL_PlayerByIndex(playerIndex);
    if ((player != (CBasePlayer *)0x0) && (player->m_iConnected != PlayerDisconnected)) {
      if (this->m_pSoundProxy[iVar10] == (CBaseEntity *)0x0) {
        pCVar5 = CreateEntityByName((char *)(unaff_EBX + 0x3543ee),-1,true);
        this->m_pSoundProxy[iVar10] = pCVar5;
        if (((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
          CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
        }
        CBaseEntity::SetAbsOrigin
                  (this->m_pSoundProxy[iVar10],
                   &(this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin);
        pCVar5 = this->m_pSoundProxy[iVar10];
        piVar1 = &pCVar5->m_iEFlags;
        *piVar1 = *piVar1 | 0x80;
        CBaseEntity::DispatchUpdateTransmitState(pCVar5);
        pCVar7 = this->m_pAmbientSound[iVar10];
      }
      else {
        pCVar7 = this->m_pAmbientSound[iVar10];
      }
      if (pCVar7 == (CSoundPatch *)0x0) {
        CRecipientFilter::CRecipientFilter(&local_3c);
        local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
             (_func_int_varargs **)(unaff_EBX + 0x53fa6a);
        CRecipientFilter::AddRecipient(&local_3c,player);
        if (this->m_bIsLethal == false) {
          p_Var9 = pCVar4->_vptr_CSoundEnvelopeController[6];
          peVar3 = (this->m_pSoundProxy[iVar10]->m_Network).m_pPev;
          iVar8 = 0;
          if (peVar3 != (edict_t *)0x0) {
            iVar8 = (int)peVar3 - *(int *)(*piVar2 + 0x58) >> 4;
          }
          iVar6 = unaff_EBX + 0x3a6faf;
        }
        else {
          p_Var9 = pCVar4->_vptr_CSoundEnvelopeController[6];
          peVar3 = (this->m_pSoundProxy[iVar10]->m_Network).m_pPev;
                    /* Unresolved local var: int edictIndex@[???] */
          iVar8 = 0;
          if (peVar3 != (edict_t *)0x0) {
            iVar8 = (int)peVar3 - *(int *)(**(int **)(unaff_EBX + 0x53d4e6) + 0x58) >> 4;
          }
          iVar6 = unaff_EBX + 0x3a6fbe;
        }
        pCVar7 = (CSoundPatch *)(*p_Var9)(pCVar4,&local_3c,iVar8,iVar6);
        this->m_pAmbientSound[iVar10] = pCVar7;
        pCVar7 = this->m_pAmbientSound[iVar10];
        (*pCVar4->_vptr_CSoundEnvelopeController[2])(pCVar4,pCVar7,0x3f800000,0x42c80000,0);
        local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
             (_func_int_varargs **)(unaff_EBX + 0x53fa6a);
        CRecipientFilter::~CRecipientFilter(&local_3c,(int)pCVar7);
      }
    }
    iVar10 = playerIndex;
  } while (playerIndex != 0x21);
  return;
}


/* CPortalLaser::Activate at 0070df40 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void Activate(CPortalLaser * this) */

void __thiscall CPortalLaser::Activate(CPortalLaser *this)

{
  CreateHelperEntities(this);
  CBaseAnimating::Activate(&this->super_CBaseAnimating);
  return;
}


/* CPortalLaser::UpdateOnRemove at 00711d70 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void UpdateOnRemove(CPortalLaser * this) */

void __thiscall CPortalLaser::UpdateOnRemove(CPortalLaser *this)

{
  if (this->m_pPlacementHelper != (CInfoPlacementHelper *)0x0) {
    UTIL_Remove((CBaseEntity *)this->m_pPlacementHelper);
  }
  UTIL_Remove(&this->m_pBeam->super_CBaseEntity);
  this->m_pBeam = (CBeam *)0x0;
  TurnOff(this);
  CBaseEntity::UpdateOnRemove((CBaseEntity *)this);
  return;
}


/* CPortalLaser::Precache at 007093c0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void Precache(CPortalLaser * this) */

void __thiscall CPortalLaser::Precache(CPortalLaser *this)

{
  char *pcVar1;
  char *name;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_bIsLethal == false) {
    CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x3a7260));
  }
  else {
    CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x3a726f));
  }
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x3a724d));
  CBaseEntity::PrecacheModel(*(char **)(unaff_EBX + 0x659793));
  if (this->m_bFromReflectedCube != false) {
    return;
  }
  name = (this->m_ModelName).pszValue;
  pcVar1 = (char *)(unaff_EBX + 0x345957);
  if (name != (char *)0x0) {
    pcVar1 = name;
  }
  if (*pcVar1 != '\0') {
    if (name == (char *)0x0) {
      name = (char *)(unaff_EBX + 0x345957);
    }
    CBaseEntity::PrecacheModel(name);
    return;
  }
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x3a705b));
  return;
}


/* CPortalLaser::CreateVPhysics at 007092e0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: bool CreateVPhysics(CPortalLaser * this) */

bool __thiscall CPortalLaser::CreateVPhysics(CPortalLaser *this)

{
  CBaseEntity::VPhysicsInitStatic((CBaseEntity *)this);
  return true;
}


/* CPortalLaser::IsOn at 00709060 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: bool IsOn(CPortalLaser * this) */

bool __thiscall CPortalLaser::IsOn(CPortalLaser *this)

{
  return (this->super_CBaseAnimating).super_CBaseEntity.m_pfnThink.__pfn !=
         (_func___thiscall_void_CBaseEntity_ptr *)0x0;
}


/* CPortalLaser::InputTurnOn at 0070a5e0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void InputTurnOn(CPortalLaser * this, inputdata_t * inputdata) */

void __thiscall CPortalLaser::InputTurnOn(CPortalLaser *this,inputdata_t *inputdata)

{
  BASEPTR func;
  int unaff_EBX;
  float fVar1;
  
  ___i686_get_pc_thunk_bx();
  if ((this->super_CBaseAnimating).super_CBaseEntity.m_pfnThink.__pfn !=
      (_func___thiscall_void_CBaseEntity_ptr *)0x0) {
    return;
  }
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0x691f);
  CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
  if (this->m_bFromReflectedCube == false) {
    fVar1 = *(float *)(*(int *)(unaff_EBX + 0x6bdc6b) + 0x2c);
  }
  else {
    fVar1 = *(float *)(*(int *)(unaff_EBX + 0x6bdc0b) + 0x2c);
  }
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,fVar1 + *(float *)(**(int **)(unaff_EBX + 0x53c573) + 0xc),
             (char *)0x0);
  TurnOnGlow(this);
  return;
}


/* CPortalLaser::InputTurnOff at 00711d50 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void InputTurnOff(CPortalLaser * this, inputdata_t * inputdata) */

void __thiscall CPortalLaser::InputTurnOff(CPortalLaser *this,inputdata_t *inputdata)

{
  if ((this->super_CBaseAnimating).super_CBaseEntity.m_pfnThink.__pfn ==
      (_func___thiscall_void_CBaseEntity_ptr *)0x0) {
    return;
  }
  TurnOff(this);
  return;
}


/* CPortalLaser::InputToggle at 00711ca0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void InputToggle(CPortalLaser * this, inputdata_t * inputdata) */

void __thiscall CPortalLaser::InputToggle(CPortalLaser *this,inputdata_t *inputdata)

{
  BASEPTR func;
  int unaff_EBX;
  float fVar1;
  
  ___i686_get_pc_thunk_bx();
  if ((this->super_CBaseAnimating).super_CBaseEntity.m_pfnThink.__pfn !=
      (_func___thiscall_void_CBaseEntity_ptr *)0x0) {
    TurnOff(this);
    return;
  }
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0xda1);
  CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
  if (this->m_bFromReflectedCube == false) {
    fVar1 = *(float *)(*(int *)(unaff_EBX + 0x6b65ab) + 0x2c);
  }
  else {
    fVar1 = *(float *)(*(int *)(unaff_EBX + 0x6b654b) + 0x2c);
  }
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             fVar1 + *(float *)(**(int **)(CFuncTankMortar::~CFuncTankMortar + unaff_EBX + 3) + 0xc)
             ,(char *)0x0);
  TurnOnGlow(this);
  return;
}


/* CPortalLaser::TurnOff at 007119c0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void TurnOff(CPortalLaser * this) */

void __thiscall CPortalLaser::TurnOff(CPortalLaser *this)

{
  undefined1 *puVar1;
  float time;
  ushort uVar2;
  CSprite *this_00;
  int *piVar3;
  CBaseEdict *this_01;
  ushort *puVar4;
  BASEPTR func;
  CSoundEnvelopeController *pCVar5;
  IChangeInfoAccessor *pIVar6;
  uint uVar7;
  uint uVar8;
  int unaff_EBX;
  int iVar9;
  uint local_60;
  
  ___i686_get_pc_thunk_bx();
  if ((this->m_bShouldSpark).m_Value != false) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      this_01 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
      ;
      if ((this_01 != (CBaseEdict *)0x0) && ((this_01->m_fStateFlags & 0x100U) == 0)) {
        this_01->m_fStateFlags = this_01->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_01);
        piVar3 = *(int **)(unaff_EBX + 0x53519a);
        puVar4 = (ushort *)*piVar3;
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
          uVar8 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar4[uVar8 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_00711c48:
            puVar4[(uint)uVar2 + uVar8 * 0x14 + 1] = 0x7dc;
            puVar4[uVar8 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar4[uVar8 * 0x14 + 1] != 0x7dc) {
            local_60 = 0;
            do {
              uVar7 = local_60 + 1;
              local_60 = uVar7 & 0xffff;
              if ((ushort)uVar7 == uVar2) {
                if (uVar2 != 0x13) goto LAB_00711c48;
                pIVar6->m_iChangeInfoSerialNumber = 0;
                this_01->m_fStateFlags = this_01->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar8 * 0x14 + local_60 + 1] != 0x7dc);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_01->m_fStateFlags = this_01->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(*piVar3 + 0xfa2) = *(short *)(*piVar3 + 0xfa2) + 1;
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar3;
          iVar9 = *piVar3 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar9 + 2) = 0x7dc;
          *(undefined2 *)(iVar9 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bShouldSpark).m_Value = false;
  }
  if (this->m_pChildLaser != (CPortalLaser *)0x0) {
    UTIL_Remove((CBaseEntity *)this->m_pChildLaser);
    this->m_pChildLaser = (CPortalLaser *)0x0;
  }
  if (this->m_pBeam != (CBeam *)0x0) {
    CBeam::TurnOn(this->m_pBeam);
  }
  this_00 = this->m_pLaserGlow;
  if (this_00 != (CSprite *)0x0) {
    time = *(float *)(unaff_EBX + 0x3baf42);
    CSprite::SetBrightness(this_00,0,time);
    func.__delta = 0;
    func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)*(uint *)(&DAT_005359aa + unaff_EBX);
    CBaseEntity::ThinkSet(&this_00->super_CBaseEntity,func,0.0,(char *)0x0);
    piVar3 = *(int **)(unaff_EBX + 0x535196);
    this_00->m_flDieTime = time + *(float *)(*piVar3 + 0xc);
    CBaseEntity::SetNextThink(&this_00->super_CBaseEntity,*(float *)(*piVar3 + 0xc),(char *)0x0);
    this->m_pLaserGlow = (CSprite *)0x0;
  }
  iVar9 = 0;
  do {
    if (this->m_pAmbientSound[iVar9] != (CSoundPatch *)0x0) {
      pCVar5 = CSoundEnvelopeController::GetController();
                    /* Unresolved local var: CSoundEnvelopeController * controller@[???] */
      (*pCVar5->_vptr_CSoundEnvelopeController[0xb])(pCVar5,this->m_pAmbientSound[iVar9]);
      this->m_pAmbientSound[iVar9] = (CSoundPatch *)0x0;
    }
    iVar9 = iVar9 + 1;
  } while (iVar9 != 0x21);
  CBaseEntity::SetNextThink((CBaseEntity *)this,-1.0,(char *)0x0);
  CBaseEntity::ThinkSet((CBaseEntity *)this,(BASEPTR)0x0,0.0,(char *)0x0);
  return;
}


/* CPortalLaser::TurnOn at 0070a180 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void TurnOn(CPortalLaser * this) */

void __thiscall CPortalLaser::TurnOn(CPortalLaser *this)

{
  BASEPTR func;
  int unaff_EBX;
  float fVar1;
  
  ___i686_get_pc_thunk_bx();
  if ((this->super_CBaseAnimating).super_CBaseEntity.m_pfnThink.__pfn !=
      (_func___thiscall_void_CBaseEntity_ptr *)0x0) {
    return;
  }
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0x6d7f);
  CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
  if (this->m_bFromReflectedCube == false) {
    fVar1 = *(float *)(*(int *)(unaff_EBX + 0x6be0cb) + 0x2c);
  }
  else {
    fVar1 = *(float *)(*(int *)(unaff_EBX + 0x6be06b) + 0x2c);
  }
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,fVar1 + *(float *)(**(int **)(unaff_EBX + 0x53c9d3) + 0xc),
             (char *)0x0);
  TurnOnGlow(this);
  return;
}


/* CPortalLaser::BeamDamage at 0070a230 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void BeamDamage(CPortalLaser * this, trace_t * ptr) */

void __thiscall CPortalLaser::BeamDamage(CPortalLaser *this,trace_t *ptr)

{
  bool bVar1;
  CBaseEntity *this_00;
  int *piVar2;
  int unaff_EBX;
  CTakeDamageInfo *this_1;
  CTakeDamageInfo *pCVar3;
  CTakeDamageInfo *in_stack_ffffff0c;
  CBaseEntity *in_stack_ffffff10;
  CBaseEntity *in_stack_ffffff14;
  float in_stack_ffffff18;
  int in_stack_ffffff1c;
  CBaseEntity *this_01;
  CTakeDamageInfo local_d8;
  CTakeDamageInfo local_88;
  float local_38;
  float local_34;
  float local_30;
  Vector local_2c;
  CTakeDamageInfo *local_20;
  
  ___i686_get_pc_thunk_bx();
  if ((*(float *)(unaff_EBX + 0x3c26cf) != (ptr->super_CBaseTrace).fraction) &&
     (this_01 = ptr->m_pEnt, this_01 != (CBaseEntity *)0x0)) {
    ClearMultiDamage();
    if (((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
                    /* Unresolved local var: Vector res@[???] */
    local_38 = (ptr->super_CBaseTrace).endpos.x -
               (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.x;
    local_34 = (ptr->super_CBaseTrace).endpos.y -
               (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.y;
    local_30 = (ptr->super_CBaseTrace).endpos.z -
               (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.z;
    local_2c.x = local_38;
    local_2c.y = local_34;
    local_2c.z = local_30;
    VectorNormalize(&local_2c);
    CTakeDamageInfo::CTakeDamageInfo
              (&local_88,(CBaseEntity *)this,(CBaseEntity *)this,
               *(float *)(unaff_EBX + 0x3c2a5f) *
               *(float *)(**(int **)(unaff_EBX + 0x53c923) + 0x10),8,0,in_stack_ffffff0c,
               in_stack_ffffff10,in_stack_ffffff14,in_stack_ffffff18,in_stack_ffffff1c,(int)this_01)
    ;
    CalculateMeleeDamageForce(&local_88,&local_2c,&(ptr->super_CBaseTrace).endpos,1.0);
    CBaseEntity::DispatchTraceAttack(this_01,&local_88,&local_2c,ptr);
    ApplyMultiDamage();
    this_1 = (CTakeDamageInfo *)(unaff_EBX + 0x3a62d9);
    UTIL_DecalTrace(ptr,(char *)this_1);
    this_00 = (CBaseEntity *)
              (*(this_01->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                _vptr_IHandleEntity[0x36])(this_01);
    if ((this_00 != (CBaseEntity *)0x0) &&
       ((((pCVar3 = (CTakeDamageInfo *)(unaff_EBX + 0x356ec3), local_20 = pCVar3,
          (CTakeDamageInfo *)(this_00->m_iClassname).pszValue == pCVar3 ||
          (bVar1 = CBaseEntity::ClassMatchesComplex(this_00,(char *)pCVar3), this_1 = pCVar3, bVar1)
          ) || (pCVar3 = (CTakeDamageInfo *)(unaff_EBX + 0x3a396f), local_20 = pCVar3,
               (CTakeDamageInfo *)(this_00->m_iClassname).pszValue == pCVar3)) ||
        (bVar1 = CBaseEntity::ClassMatchesComplex(this_00,(char *)pCVar3), this_1 = pCVar3, bVar1)))
       ) {
      if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x53d39f) + 0x1c) + 0x30) != 0) {
                    /* Unresolved local var: CTakeDamageInfo info@[???]
                       Unresolved local var: CNPC_Portal_FloorTurret * pTurret@[???] */
        CTakeDamageInfo::CTakeDamageInfo(&local_d8,this_1);
        local_d8.m_flDamage = 1.0;
        local_d8.m_bitsDamageType = 1;
        (*(this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
          _vptr_IHandleEntity[0x46])(this_00,&local_d8);
        (*(this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
          _vptr_IHandleEntity[0x282])(this_00);
        CBaseEntity::ThinkSet(this_00,(BASEPTR)0x0,0.0,(char *)0x0);
        return;
      }
      (*(this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
        [0xde])(this_00,0x41f00000,1,0,0);
      return;
    }
    if (((this_01->m_MoveType).m_Value == '\x06') &&
       (piVar2 = (int *)___dynamic_cast(this_01,*(undefined4 *)(unaff_EBX + 0x53c94f),
                                        *(undefined4 *)(unaff_EBX + 0x53ca43),0),
       piVar2 != (int *)0x0)) {
      (**(code **)(*piVar2 + 0x378))(piVar2,0x41f00000,0,0,0);
    }
  }
  return;
}


/* CPortalLaser::TurnOnGlow at 00709d50 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void TurnOnGlow(CPortalLaser * this) */

void __thiscall CPortalLaser::TurnOnGlow(CPortalLaser *this)

{
  undefined1 *puVar1;
  ushort uVar2;
  int iVar3;
  CBaseEdict *pCVar4;
  ushort *puVar5;
  int *piVar6;
  CSprite *this_00;
  int iVar7;
  ulong *puVar8;
  IChangeInfoAccessor *pIVar9;
  uint uVar10;
  uint uVar11;
  int unaff_EBX;
  uint local_84;
  uint local_80;
  
  ___i686_get_pc_thunk_bx();
  if ((this->m_bFromReflectedCube == false) && (this->m_pLaserGlow == (CSprite *)0x0)) {
    this_00 = CSprite::SpriteCreate
                        (*(char **)(unaff_EBX + 0x658df9),
                         (Vector *)&(this->super_CBaseAnimating).super_CBaseEntity.m_vecOrigin,false
                        );
    this->m_pLaserGlow = this_00;
    iVar3 = this->m_iLaserAttachment;
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
    uVar10 = (this_00->m_hAttachedToEntity).
             super_CNetworkVarBase<CBaseHandle,CSprite::NetworkVar_m_hAttachedToEntity>.m_Value.
             m_Index;
    if ((uVar10 == 0xffffffff) ||
       ((iVar7 = (uVar10 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x53ce05),
        *(uint *)(iVar7 + 8) != uVar10 >> 0x10 || (this != *(CPortalLaser **)(iVar7 + 4))))) {
      if ((this_00->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
        pCVar4 = &((this_00->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
        if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
          pIVar9 = CBaseEdict::GetChangeAccessor(pCVar4);
          puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x53ce01);
          if (pIVar9->m_iChangeInfoSerialNumber == *puVar5) {
            uVar10 = (uint)pIVar9->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar2 = puVar5[uVar10 * 0x14 + 0x14];
            if (uVar2 == 0) {
LAB_0070a052:
              puVar5[(uint)uVar2 + uVar10 * 0x14 + 1] = 0x3c0;
              puVar5[uVar10 * 0x14 + 0x14] = uVar2 + 1;
            }
            else if (puVar5[uVar10 * 0x14 + 1] != 0x3c0) {
              local_84 = 0;
              do {
                uVar11 = local_84 + 1;
                local_84 = uVar11 & 0xffff;
                if ((ushort)uVar11 == uVar2) {
                  if (uVar2 != 0x13) goto LAB_0070a052;
                  pIVar9->m_iChangeInfoSerialNumber = 0;
                  pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar5[uVar10 * 0x14 + local_84 + 1] != 0x3c0);
            }
          }
          else if (puVar5[0x7d1] == 100) {
            pIVar9->m_iChangeInfoSerialNumber = 0;
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar9->m_iChangeInfo = puVar5[0x7d1];
            piVar6 = *(int **)(unaff_EBX + 0x53ce01);
            *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
            pIVar9->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
            iVar7 = *piVar6 + (uint)pIVar9->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar7 + 2) = 0x3c0;
            *(undefined2 *)(iVar7 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this_00->super_CBaseEntity).m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      puVar8 = (ulong *)(*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.
                          super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[3])(this);
      (this_00->m_hAttachedToEntity).
      super_CNetworkVarBase<CBaseHandle,CSprite::NetworkVar_m_hAttachedToEntity>.m_Value.m_Index =
           *puVar8;
    }
    if (iVar3 != (this_00->m_nAttachment).m_Value) {
      if ((this_00->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
        pCVar4 = &((this_00->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
        if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
          pIVar9 = CBaseEdict::GetChangeAccessor(pCVar4);
          puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x53ce01);
          if (pIVar9->m_iChangeInfoSerialNumber == *puVar5) {
            uVar10 = (uint)pIVar9->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar2 = puVar5[uVar10 * 0x14 + 0x14];
            if (uVar2 == 0) {
LAB_0070a0f0:
              puVar5[(uint)uVar2 + uVar10 * 0x14 + 1] = 0x3c4;
              puVar5[uVar10 * 0x14 + 0x14] = uVar2 + 1;
            }
            else if (puVar5[uVar10 * 0x14 + 1] != 0x3c4) {
              local_80 = 0;
              do {
                uVar11 = local_80 + 1;
                local_80 = uVar11 & 0xffff;
                if ((ushort)uVar11 == uVar2) {
                  if (uVar2 != 0x13) goto LAB_0070a0f0;
                  pIVar9->m_iChangeInfoSerialNumber = 0;
                  pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar5[uVar10 * 0x14 + local_80 + 1] != 0x3c4);
            }
          }
          else if (puVar5[0x7d1] == 100) {
            pIVar9->m_iChangeInfoSerialNumber = 0;
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar9->m_iChangeInfo = puVar5[0x7d1];
            piVar6 = *(int **)(unaff_EBX + 0x53ce01);
            *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
            pIVar9->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
            iVar7 = *piVar6 + (uint)pIVar9->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar7 + 2) = 0x3c4;
            *(undefined2 *)(iVar7 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this_00->super_CBaseEntity).m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this_00->m_nAttachment).m_Value = iVar3;
    }
    CBaseEntity::FollowEntity(&this_00->super_CBaseEntity,(CBaseEntity *)this,true);
    CSprite::SetBrightness(this->m_pLaserGlow,0x80,0.1);
    CSprite::SetScale(this->m_pLaserGlow,2.0,0.1);
  }
  return;
}


/* CPortalLaser::TurnOffGlow at 00709210 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void TurnOffGlow(CPortalLaser * this) */

void __thiscall CPortalLaser::TurnOffGlow(CPortalLaser *this)

{
  float time;
  CSprite *this_00;
  int *piVar1;
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  this_00 = this->m_pLaserGlow;
  if (this_00 != (CSprite *)0x0) {
    time = *(float *)(unaff_EBX + 0x3c36ec);
    CSprite::SetBrightness(this_00,0,time);
    func.__delta = 0;
    func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)*(uint *)(unaff_EBX + 0x53e154);
    CBaseEntity::ThinkSet(&this_00->super_CBaseEntity,func,0.0,(char *)0x0);
    piVar1 = *(int **)(unaff_EBX + 0x53d940);
    this_00->m_flDieTime = time + *(float *)(*piVar1 + 0xc);
    CBaseEntity::SetNextThink(&this_00->super_CBaseEntity,*(float *)(*piVar1 + 0xc),(char *)0x0);
    this->m_pLaserGlow = (CSprite *)0x0;
  }
  return;
}


/* CPortalLaser::TurnOffLaserSound at 007091c0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void TurnOffLaserSound(CPortalLaser * this) */

void __thiscall CPortalLaser::TurnOffLaserSound(CPortalLaser *this)

{
  CSoundEnvelopeController *pCVar1;
  int iVar2;
  
  iVar2 = 0;
  do {
                    /* Unresolved local var: int i@[???] */
    if (this->m_pAmbientSound[iVar2] != (CSoundPatch *)0x0) {
                    /* Unresolved local var: CSoundEnvelopeController * controller@[???] */
      pCVar1 = CSoundEnvelopeController::GetController();
      (*pCVar1->_vptr_CSoundEnvelopeController[0xb])(pCVar1,this->m_pAmbientSound[iVar2]);
      this->m_pAmbientSound[iVar2] = (CSoundPatch *)0x0;
    }
    iVar2 = iVar2 + 1;
  } while (iVar2 != 0x21);
  return;
}


/* CPortalLaser::FireAtPoint at 0070eb30 */

/* WARNING: Restarted to delay deadcode elimination for space: stack */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void FireAtPoint(CPortalLaser * this, trace_t * tr, bool bImpact) */

void __thiscall CPortalLaser::FireAtPoint(CPortalLaser *this,trace_t *tr,bool bImpact)

{
  undefined1 *puVar1;
  vec_t *pvVar2;
  int iVar3;
  ushort uVar4;
  CBaseEdict *pCVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  ushort *puVar9;
  int *piVar10;
  vec_t vVar11;
  vec_t vVar12;
  vec_t vVar13;
  IChangeInfoAccessor *pIVar14;
  uint uVar15;
  uint uVar16;
  int unaff_EBX;
  uint local_dc;
  uint local_d8;
  uint local_d4;
  uint local_d0;
  Vector local_28 [2];
  
  ___i686_get_pc_thunk_bx();
  BeamDamage(this,tr);
  if (bImpact) {
    if ((this->m_bShouldSpark).m_Value == false) {
      if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
          false) {
        pCVar5 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
          pIVar14 = CBaseEdict::GetChangeAccessor(pCVar5);
          puVar9 = (ushort *)**(undefined4 **)(Fraggrenade_WasCreatedByCombine + unaff_EBX + 7);
          if (pIVar14->m_iChangeInfoSerialNumber == *puVar9) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            uVar15 = (uint)pIVar14->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar4 = puVar9[uVar15 * 0x14 + 0x14];
            if (uVar4 == 0) {
LAB_0070f24d:
              puVar9[(uint)uVar4 + uVar15 * 0x14 + 1] = 0x7dc;
              puVar9[uVar15 * 0x14 + 0x14] = uVar4 + 1;
            }
            else if (puVar9[uVar15 * 0x14 + 1] != 0x7dc) {
              local_dc = 0;
              do {
                uVar16 = local_dc + 1;
                local_dc = uVar16 & 0xffff;
                if ((ushort)uVar16 == uVar4) {
                  if (uVar4 != 0x13) goto LAB_0070f24d;
                  goto LAB_0070f342;
                }
              } while (puVar9[uVar15 * 0x14 + local_dc + 1] != 0x7dc);
            }
          }
          else if (puVar9[0x7d1] == 100) {
LAB_0070f342:
            pIVar14->m_iChangeInfoSerialNumber = 0;
            pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar14->m_iChangeInfo = puVar9[0x7d1];
            *(short *)(**(int **)(Fraggrenade_WasCreatedByCombine + unaff_EBX + 7) + 0xfa2) =
                 *(short *)(**(int **)(Fraggrenade_WasCreatedByCombine + unaff_EBX + 7) + 0xfa2) + 1
            ;
            piVar10 = *(int **)(Fraggrenade_WasCreatedByCombine + unaff_EBX + 7);
            pIVar14->m_iChangeInfoSerialNumber = *(ushort *)*piVar10;
            iVar3 = *piVar10 + (uint)pIVar14->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar3 + 2) = 0x7dc;
            *(undefined2 *)(iVar3 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->m_bShouldSpark).m_Value = true;
    }
    fVar6 = (tr->super_CBaseTrace).endpos.x;
    fVar7 = (tr->super_CBaseTrace).endpos.y;
    fVar8 = (tr->super_CBaseTrace).endpos.z;
    if ((((fVar6 != (this->m_vecLaserEndPos).m_Value.x) ||
         (NAN(fVar6) || NAN((this->m_vecLaserEndPos).m_Value.x))) ||
        (pvVar2 = &(this->m_vecLaserEndPos).m_Value.y, fVar7 != *pvVar2)) ||
       ((NAN(fVar7) || NAN(*pvVar2) || (fVar8 != (this->m_vecLaserEndPos).m_Value.z)))) {
      if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
          false) {
        pCVar5 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
          pIVar14 = CBaseEdict::GetChangeAccessor(pCVar5);
          puVar9 = (ushort *)**(undefined4 **)(Fraggrenade_WasCreatedByCombine + unaff_EBX + 7);
          if (pIVar14->m_iChangeInfoSerialNumber == *puVar9) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            uVar15 = (uint)pIVar14->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar4 = puVar9[uVar15 * 0x14 + 0x14];
            if (uVar4 == 0) {
LAB_0070f03a:
              puVar9[(uint)uVar4 + uVar15 * 0x14 + 1] = 0x7c4;
              puVar9[uVar15 * 0x14 + 0x14] = uVar4 + 1;
            }
            else if (puVar9[uVar15 * 0x14 + 1] != 0x7c4) {
              local_d8 = 0;
              do {
                uVar16 = local_d8 + 1;
                local_d8 = uVar16 & 0xffff;
                if ((ushort)uVar16 == uVar4) {
                  if (uVar4 != 0x13) goto LAB_0070f03a;
                  goto LAB_0070f193;
                }
              } while (puVar9[uVar15 * 0x14 + local_d8 + 1] != 0x7c4);
            }
          }
          else if (puVar9[0x7d1] == 100) {
LAB_0070f193:
            pIVar14->m_iChangeInfoSerialNumber = 0;
            pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar14->m_iChangeInfo = puVar9[0x7d1];
            *(short *)(**(int **)(Fraggrenade_WasCreatedByCombine + unaff_EBX + 7) + 0xfa2) =
                 *(short *)(**(int **)(Fraggrenade_WasCreatedByCombine + unaff_EBX + 7) + 0xfa2) + 1
            ;
            piVar10 = *(int **)(Fraggrenade_WasCreatedByCombine + unaff_EBX + 7);
            pIVar14->m_iChangeInfoSerialNumber = *(ushort *)*piVar10;
            iVar3 = *piVar10 + (uint)pIVar14->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar3 + 2) = 0x7c4;
            *(undefined2 *)(iVar3 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->m_vecLaserEndPos).m_Value.x = fVar6;
      (this->m_vecLaserEndPos).m_Value.y = fVar7;
      (this->m_vecLaserEndPos).m_Value.z = fVar8;
    }
                    /* Unresolved local var: Vector norm@[???] */
    local_28[0].x = (tr->super_CBaseTrace).plane.normal.x;
    local_28[0].y = (tr->super_CBaseTrace).plane.normal.y;
    local_28[0].z = (tr->super_CBaseTrace).plane.normal.z;
    VectorNormalize(local_28);
    vVar13 = local_28[0].z;
    vVar12 = local_28[0].y;
    vVar11 = local_28[0].x;
    if ((local_28[0].x == (this->m_vecLaserImpactNormal).m_Value.x) &&
       (!NAN(local_28[0].x) && !NAN((this->m_vecLaserImpactNormal).m_Value.x))) {
      pvVar2 = &(this->m_vecLaserImpactNormal).m_Value.y;
      if ((local_28[0].y == *pvVar2) &&
         ((!NAN(local_28[0].y) && !NAN(*pvVar2) &&
          (local_28[0].z == (this->m_vecLaserImpactNormal).m_Value.z)))) {
        return;
      }
    }
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      pCVar5 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
        pIVar14 = CBaseEdict::GetChangeAccessor(pCVar5);
        puVar9 = (ushort *)**(undefined4 **)(Fraggrenade_WasCreatedByCombine + unaff_EBX + 7);
        if (pIVar14->m_iChangeInfoSerialNumber == *puVar9) {
          uVar15 = (uint)pIVar14->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar4 = puVar9[uVar15 * 0x14 + 0x14];
          if (uVar4 == 0) {
LAB_0070ef7a:
            puVar9[(uint)uVar4 + uVar15 * 0x14 + 1] = 2000;
            puVar9[uVar15 * 0x14 + 0x14] = uVar4 + 1;
          }
          else if (puVar9[uVar15 * 0x14 + 1] != 2000) {
            local_d4 = 0;
            do {
              uVar16 = local_d4 + 1;
              local_d4 = uVar16 & 0xffff;
              if ((ushort)uVar16 == uVar4) {
                if (uVar4 != 0x13) goto LAB_0070ef7a;
                pIVar14->m_iChangeInfoSerialNumber = 0;
                pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar9[uVar15 * 0x14 + local_d4 + 1] != 2000);
          }
        }
        else if (puVar9[0x7d1] == 100) {
          pIVar14->m_iChangeInfoSerialNumber = 0;
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar14->m_iChangeInfo = puVar9[0x7d1];
          *(short *)(**(int **)(Fraggrenade_WasCreatedByCombine + unaff_EBX + 7) + 0xfa2) =
               *(short *)(**(int **)(Fraggrenade_WasCreatedByCombine + unaff_EBX + 7) + 0xfa2) + 1;
          piVar10 = *(int **)(Fraggrenade_WasCreatedByCombine + unaff_EBX + 7);
          pIVar14->m_iChangeInfoSerialNumber = *(ushort *)*piVar10;
          iVar3 = *piVar10 + (uint)pIVar14->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar3 + 2) = 2000;
          *(undefined2 *)(iVar3 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_vecLaserImpactNormal).m_Value.x = vVar11;
    (this->m_vecLaserImpactNormal).m_Value.y = vVar12;
    (this->m_vecLaserImpactNormal).m_Value.z = vVar13;
  }
  else if ((this->m_bShouldSpark).m_Value != false) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      pCVar5 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
        pIVar14 = CBaseEdict::GetChangeAccessor(pCVar5);
        puVar9 = (ushort *)**(undefined4 **)(Fraggrenade_WasCreatedByCombine + unaff_EBX + 7);
        if (pIVar14->m_iChangeInfoSerialNumber == *puVar9) {
          uVar15 = (uint)pIVar14->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar4 = puVar9[uVar15 * 0x14 + 0x14];
          if (uVar4 == 0) {
LAB_0070f306:
            puVar9[(uint)uVar4 + uVar15 * 0x14 + 1] = 0x7dc;
            puVar9[uVar15 * 0x14 + 0x14] = uVar4 + 1;
          }
          else if (puVar9[uVar15 * 0x14 + 1] != 0x7dc) {
            local_d0 = 0;
            do {
              uVar16 = local_d0 + 1;
              local_d0 = uVar16 & 0xffff;
              if ((ushort)uVar16 == uVar4) {
                if (uVar4 != 0x13) goto LAB_0070f306;
                pIVar14->m_iChangeInfoSerialNumber = 0;
                pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar9[uVar15 * 0x14 + local_d0 + 1] != 0x7dc);
          }
        }
        else if (puVar9[0x7d1] == 100) {
          pIVar14->m_iChangeInfoSerialNumber = 0;
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar14->m_iChangeInfo = puVar9[0x7d1];
          *(short *)(**(int **)(Fraggrenade_WasCreatedByCombine + unaff_EBX + 7) + 0xfa2) =
               *(short *)(**(int **)(Fraggrenade_WasCreatedByCombine + unaff_EBX + 7) + 0xfa2) + 1;
          piVar10 = *(int **)(Fraggrenade_WasCreatedByCombine + unaff_EBX + 7);
          pIVar14->m_iChangeInfoSerialNumber = *(ushort *)*piVar10;
          iVar3 = *piVar10 + (uint)pIVar14->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar3 + 2) = 0x7dc;
          *(undefined2 *)(iVar3 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bShouldSpark).m_Value = false;
    return;
  }
  return;
}


/* CPortalLaser::HideBeam at 00709190 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void HideBeam(CPortalLaser * this) */

void __thiscall CPortalLaser::HideBeam(CPortalLaser *this)

{
  if (this->m_pBeam != (CBeam *)0x0) {
    CBeam::TurnOn(this->m_pBeam);
    return;
  }
  return;
}


/* CPortalLaser::ShowBeam at 00709160 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void ShowBeam(CPortalLaser * this) */

void __thiscall CPortalLaser::ShowBeam(CPortalLaser *this)

{
  if (this->m_pBeam != (CBeam *)0x0) {
    CBeam::TurnOff(this->m_pBeam);
    return;
  }
  return;
}


/* CPortalLaser::StrikeThink at 00710f10 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void StrikeThink(CPortalLaser * this) */

void __thiscall CPortalLaser::StrikeThink(CPortalLaser *this)

{
  undefined1 *puVar1;
  uint *puVar2;
  uint uVar3;
  ushort uVar4;
  CBaseEdict *this_00;
  ushort *puVar5;
  char cVar6;
  undefined *puVar7;
  int iVar8;
  int *piVar9;
  IChangeInfoAccessor *pIVar10;
  vec_t *pvVar11;
  uint uVar12;
  int iVar13;
  uint uVar14;
  int unaff_EBX;
  CBaseEntity *this_01;
  Vector *local_50;
  Vector local_34;
  Vector local_28 [2];
  
                    /* Unresolved local var: CVProfScope VProf_@[???]
                       Unresolved local var: Vector vecDir@[???]
                       Unresolved local var: Vector vecOrigin@[???]
                       Unresolved local var: CBaseEntity * pParent@[???] */
  ___i686_get_pc_thunk_bx();
  if ((*(int *)(&DAT_0000100c + *(int *)(unaff_EBX + 0x535c63)) != 0) ||
     ((&DAT_00001010)[*(int *)(unaff_EBX + 0x535c63)] == '\0')) {
    iVar13 = *(int *)(*(int *)(unaff_EBX + 0x535c63) + 0x19b8);
    iVar8 = _ThreadGetCurrentId();
    if (iVar13 == iVar8) {
      piVar9 = *(int **)(&DAT_00001014 + *(int *)(unaff_EBX + 0x535c63));
      if (*piVar9 != unaff_EBX + 0x39f6a8) {
        piVar9 = (int *)CVProfNode::GetSubNode
                                  ((char *)piVar9,unaff_EBX + 0x39f6a8,(char *)0x0,
                                   unaff_EBX + 0x3519d4);
        *(int **)(&DAT_00001014 + *(int *)(unaff_EBX + 0x535c63)) = piVar9;
      }
      puVar2 = (uint *)(piVar9[0x1c] * 8 + *(int *)(&DAT_000010a0 + *(int *)(unaff_EBX + 0x535c63))
                       + 4);
      *puVar2 = *puVar2 | 4;
      CVProfNode::EnterScope();
      (&DAT_00001010)[*(int *)(unaff_EBX + 0x535c63)] = 0;
    }
  }
  if ((this->m_bShouldSpark).m_Value != false) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
      ;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(this_00);
        piVar9 = *(int **)(unaff_EBX + 0x535c47);
        puVar5 = (ushort *)*piVar9;
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar5) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar12 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar4 = puVar5[uVar12 * 0x14 + 0x14];
          if (uVar4 == 0) {
LAB_0071138d:
            puVar5[(uint)uVar4 + uVar12 * 0x14 + 1] = 0x7dc;
            puVar5[uVar12 * 0x14 + 0x14] = uVar4 + 1;
          }
          else if (puVar5[uVar12 * 0x14 + 1] != 0x7dc) {
            uVar14 = 0;
            do {
              uVar3 = uVar14 + 1;
              uVar14 = uVar3 & 0xffff;
              if ((ushort)uVar3 == uVar4) {
                if (uVar4 != 0x13) goto LAB_0071138d;
                goto LAB_007113b2;
              }
            } while (puVar5[uVar12 * 0x14 + uVar14 + 1] != 0x7dc);
          }
        }
        else if (puVar5[0x7d1] == 100) {
LAB_007113b2:
          pIVar10->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(*piVar9 + 0xfa2) = *(short *)(*piVar9 + 0xfa2) + 1;
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar9;
          iVar13 = *piVar9 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar13 + 2) = 0x7dc;
          *(undefined2 *)(iVar13 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bShouldSpark).m_Value = false;
  }
                    /* Unresolved local var: int i@[???] */
  pvVar11 = *(vec_t **)(unaff_EBX + 0x535fa7);
  this->m_vecNearestSoundSource[0].x = *pvVar11;
  this->m_vecNearestSoundSource[0].y = pvVar11[1];
  this->m_vecNearestSoundSource[0].z = pvVar11[2];
  iVar13 = 0xc;
  do {
    *(vec_t *)((int)&this->m_vecNearestSoundSource[0].x + iVar13) = *pvVar11;
    *(vec_t *)((int)&this->m_vecNearestSoundSource[0].y + iVar13) = pvVar11[1];
    *(vec_t *)((int)&this->m_vecNearestSoundSource[0].z + iVar13) = pvVar11[2];
    iVar13 = iVar13 + 0xc;
  } while (iVar13 != 0x18c);
  uVar12 = (this->super_CBaseAnimating).super_CBaseEntity.m_pParent.super_CBaseHandle.m_Index;
  if ((uVar12 == 0xffffffff) ||
     (iVar13 = (uVar12 & 0xffff) * 0x10 + **(int **)(&DAT_00535c4b + unaff_EBX),
     *(uint *)(iVar13 + 8) != uVar12 >> 0x10)) {
    this_01 = (CBaseEntity *)0x0;
  }
  else {
    this_01 = *(CBaseEntity **)(iVar13 + 4);
    if ((this_01 != (CBaseEntity *)0x0) && (this->m_bFromReflectedCube != false)) {
      pvVar11 = (vec_t *)(*(this_01->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                           _vptr_IHandleEntity[0x9d])(this_01);
      local_34.x = *pvVar11;
      local_34.y = pvVar11[1];
      local_34.z = pvVar11[2];
      if ((this_01->m_iEFlags & 0x800) != 0) {
        CBaseEntity::CalcAbsolutePosition(this_01);
      }
      AngleVectors(&this_01->m_angAbsRotation,local_28);
      iVar13 = **(int **)(unaff_EBX + 0x535c43);
      puVar7 = (undefined *)(unaff_EBX + 0x6b731f);
      goto LAB_00711024;
    }
  }
  CBaseAnimating::GetAttachment
            (&this->super_CBaseAnimating,this->m_iLaserAttachment,&local_34,local_28,(Vector *)0x0,
             (Vector *)0x0);
  iVar13 = **(int **)(unaff_EBX + 0x535c43);
  puVar7 = &UNK_006b72bf + unaff_EBX;
LAB_00711024:
  local_50 = local_28;
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(iVar13 + 0xc) + *(float *)(*(int *)(puVar7 + 0x1c) + 0x2c),(char *)0x0);
  UTIL_Portal_Laser_Prevent_Tilting(local_50);
  FireLaser(this,&local_34,local_50,this_01);
  CreateSoundProxies(this);
  iVar13 = 0;
  do {
                    /* Unresolved local var: int i@[???] */
    if (this->m_pSoundProxy[iVar13] != (CBaseEntity *)0x0) {
      UTIL_SetOrigin(this->m_pSoundProxy[iVar13],this->m_vecNearestSoundSource + iVar13,false);
    }
    iVar13 = iVar13 + 1;
  } while (iVar13 != 0x21);
  if (this->m_pBeam != (CBeam *)0x0) {
    CBeam::TurnOff(this->m_pBeam);
  }
  if (*(int *)(*(int *)(unaff_EBX + 0x6b739b) + 0x30) != 0) {
    (**(code **)(*(int *)**(undefined4 **)(&DAT_00535c5f + unaff_EBX) + 0xbc))
              ((int *)**(undefined4 **)(&DAT_00535c5f + unaff_EBX),0,unaff_EBX + 0x39f6c3,
               *(undefined4 *)(unaff_EBX + 0x676a83));
  }
  if (((&DAT_00001010)[*(int *)(unaff_EBX + 0x535c63)] == '\0') ||
     (*(int *)(&DAT_0000100c + *(int *)(unaff_EBX + 0x535c63)) != 0)) {
    iVar13 = *(int *)(*(int *)(unaff_EBX + 0x535c63) + 0x19b8);
    iVar8 = _ThreadGetCurrentId();
    if (iVar13 == iVar8) {
      cVar6 = CVProfNode::ExitScope();
      if (cVar6 != '\0') {
        *(undefined4 *)(&DAT_00001014 + *(int *)(unaff_EBX + 0x535c63)) =
             *(undefined4 *)(*(int *)(&DAT_00001014 + *(int *)(unaff_EBX + 0x535c63)) + 100);
      }
      iVar13 = *(int *)(unaff_EBX + 0x535c63);
      (&DAT_00001010)[iVar13] = *(undefined **)(&DAT_00001014 + iVar13) == &DAT_00001018 + iVar13;
      return;
    }
  }
  return;
}


/* CPortalLaser::UpdateSoundPosition at 00709480 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void UpdateSoundPosition(CPortalLaser * this, Vector * vecStart, Vector
   * vecEnd) */

void __thiscall
CPortalLaser::UpdateSoundPosition(CPortalLaser *this,Vector *vecStart,Vector *vecEnd)

{
  float fVar1;
  float fVar2;
  IServerEntity IVar3;
  code *pcVar4;
  float *pfVar5;
  CBasePlayer *pCVar6;
  int unaff_EBX;
  int *piVar7;
  int *piVar8;
  CPortalLaser *pCVar14;
  int aiStack_94 [12];
  float *local_64;
  float *local_60;
  float *local_5c;
  int local_58;
  float local_54;
  float local_50;
  float local_40;
  float local_3c;
  float local_38;
  float local_34;
  float local_30;
  float local_2c;
  float local_28;
  float local_24;
  float local_20;
  int *piVar9;
  int *piVar10;
  int *piVar11;
  int *piVar12;
  int *piVar13;
  
  ___i686_get_pc_thunk_bx();
  local_58 = 0;
  local_5c = &local_34;
  local_60 = &local_40;
  local_64 = &local_28;
  piVar7 = aiStack_94 + 2;
  pCVar14 = this;
  do {
    while( true ) {
      local_58 = local_58 + 1;
      *piVar7 = local_58;
      piVar7[-1] = 0x7094fc;
      pCVar6 = UTIL_PlayerByIndex(*piVar7);
      if ((pCVar6 == (CBasePlayer *)0x0) || (pCVar6->m_iConnected == PlayerDisconnected)) break;
      IVar3.super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
           (pCVar6->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
           super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown;
      piVar7[1] = (int)pCVar6;
      *piVar7 = (int)local_5c;
      pcVar4 = *(code **)((int)IVar3.super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity +
                         0x220);
      piVar7[-1] = 0x70951d;
      (*pcVar4)();
      piVar8 = piVar7 + -1;
      piVar9 = piVar7 + -1;
      piVar10 = piVar7 + -1;
      piVar11 = piVar7 + -1;
      piVar12 = piVar7 + -1;
      piVar13 = piVar7 + -1;
      local_28 = local_34;
      local_24 = local_30;
      local_20 = local_2c;
      piVar7[3] = 0;
      piVar7[2] = (int)local_60;
      piVar7[1] = (int)vecEnd;
      *piVar7 = (int)vecStart;
      piVar7[-1] = (int)local_64;
      piVar7[-2] = 0x709558;
      CalcClosestPointOnLineSegment
                ((Vector *)piVar7[-1],(Vector *)*piVar7,(Vector *)piVar7[1],(Vector *)piVar7[2],
                 (float *)piVar7[3]);
                    /* Unresolved local var: Vector res@[???] */
      local_54 = local_40;
                    /* Unresolved local var: Vector res@[???] */
      fVar1 = pCVar14->m_vecNearestSoundSource[0].x;
      local_50 = pCVar14->m_vecNearestSoundSource[0].y;
      fVar2 = pCVar14->m_vecNearestSoundSource[0].z;
      if ((local_28 - fVar1) * (local_28 - fVar1) + (local_24 - local_50) * (local_24 - local_50) +
          (local_20 - fVar2) * (local_20 - fVar2) <=
          (local_28 - local_40) * (local_28 - local_40) +
          (local_24 - local_3c) * (local_24 - local_3c) +
          (local_20 - local_38) * (local_20 - local_38)) {
        pfVar5 = *(float **)(unaff_EBX + 0x53da3a);
        piVar7 = piVar7 + -1;
        if ((((fVar1 != *pfVar5) || (piVar7 = piVar9, NAN(fVar1) || NAN(*pfVar5))) ||
            (piVar7 = piVar10, local_50 != pfVar5[1])) ||
           (((piVar7 = piVar11, NAN(local_50) || NAN(pfVar5[1]) ||
             (piVar7 = piVar12, fVar2 != pfVar5[2])) ||
            (piVar7 = piVar13, NAN(fVar2) || NAN(pfVar5[2]))))) goto LAB_007094e1;
      }
      pCVar14->m_vecNearestSoundSource[0].x = local_40;
      pCVar14->m_vecNearestSoundSource[0].y = local_3c;
      pCVar14->m_vecNearestSoundSource[0].z = local_38;
      pCVar14 = (CPortalLaser *)
                &(pCVar14->super_CBaseAnimating).super_CBaseEntity.m_pfnMoveDone.__delta;
      piVar7 = piVar8;
      if (local_58 == 0x21) {
        return;
      }
    }
    if (((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      *piVar7 = (int)this;
      piVar7[-1] = 0x709668;
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)*piVar7);
    }
    pCVar14->m_vecNearestSoundSource[0].x =
         (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.x;
    pCVar14->m_vecNearestSoundSource[0].y =
         (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.y;
    pCVar14->m_vecNearestSoundSource[0].z =
         (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.z;
LAB_007094e1:
    pCVar14 = (CPortalLaser *)
              &(pCVar14->super_CBaseAnimating).super_CBaseEntity.m_pfnMoveDone.__delta;
    if (local_58 == 0x21) {
      return;
    }
  } while( true );
}


/* CPortalLaser::DamageEntity at 007090b0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void DamageEntity(CPortalLaser * this, CBaseEntity * pVictim, float
   flAmount) */

void __thiscall CPortalLaser::DamageEntity(CPortalLaser *this,CBaseEntity *pVictim,float flAmount)

{
  Vector *vecForceOrigin;
  int unaff_EBX;
  CTakeDamageInfo *in_stack_ffffff7c;
  CBaseEntity *in_stack_ffffff80;
  CBaseEntity *in_stack_ffffff84;
  float in_stack_ffffff88;
  vec_t in_stack_ffffff8c;
  vec_t in_stack_ffffff90;
  Vector local_28 [2];
  
                    /* Unresolved local var: CTakeDamageInfo info@[???] */
  ___i686_get_pc_thunk_bx();
  CTakeDamageInfo::CTakeDamageInfo
            ((CTakeDamageInfo *)&stack0xffffff88,(CBaseEntity *)this,(CBaseEntity *)this,
             flAmount * *(float *)(**(int **)(unaff_EBX + 0x53da9d) + 0x10),8,0,in_stack_ffffff7c,
             in_stack_ffffff80,in_stack_ffffff84,in_stack_ffffff88,(int)in_stack_ffffff8c,
             (int)in_stack_ffffff90);
  vecForceOrigin =
       (Vector *)
       (*(pVictim->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
         [0x9d])(pVictim);
  local_28[0].x = 1.0;
  local_28[0].y = 0.0;
  local_28[0].z = 0.0;
  CalculateMeleeDamageForce((CTakeDamageInfo *)&stack0xffffff88,local_28,vecForceOrigin,1.0);
  CBaseEntity::TakeDamage(pVictim,(CTakeDamageInfo *)&stack0xffffff88);
  return;
}


/* CPortalLaser::StrikeEntitiesAlongLaser at 0070c1c0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Restarted to delay deadcode elimination for space: stack */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: bool StrikeEntitiesAlongLaser(CPortalLaser * this, Vector * vecStart,
   Vector * vecEnd, Vector * pVecOut) */

bool __thiscall
CPortalLaser::StrikeEntitiesAlongLaser
          (CPortalLaser *this,Vector *vecStart,Vector *vecEnd,Vector *pVecOut)

{
  char *pszClassOrWildcard;
  char *pszClassOrWildcard_00;
  float *pfVar1;
  CPortal_Player *this_00;
  bool bVar2;
  bool bVar3;
  int iVar4;
  int iVar5;
  undefined4 *puVar6;
  CBaseEntity *pCVar7;
  Vector *pVVar8;
  int *piVar9;
  vec_t *pvVar10;
  int unaff_EBX;
  int iVar11;
  CPortal_Player *this_01;
  float fVar12;
  float fVar13;
  CBaseEntity *pAttacker_1;
  CTakeDamageInfo *in_stack_fffff59c;
  CBaseEntity *in_stack_fffff5a0;
  float in_stack_fffff5a8;
  int in_stack_fffff5ac;
  int in_stack_fffff5b0;
  int local_a1c;
  int local_a14;
  CBaseEntity *local_a0c [512];
  CTakeDamageInfo local_20c;
  CTakeDamageInfo local_1bc;
  Ray_t local_16c;
  CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_> local_118;
  float local_104;
  undefined1 local_100;
  __m128 root;
  Vector local_e0;
  Vector local_d4;
  float local_c8;
  float local_c4;
  undefined4 local_c0;
  float local_bc;
  float local_b8;
  float local_b4;
  float local_b0;
  float local_ac;
  float local_a8;
  float local_a4;
  float local_a0;
  float local_9c;
  float local_98;
  float local_94;
  float local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  Vector local_80;
  float local_74;
  float local_70;
  float local_6c;
  Vector local_68;
  float local_5c;
  float local_58;
  float local_54;
  Vector local_50;
  Vector local_44;
  float local_38;
  float local_34;
  float local_30;
  Vector local_2c;
  float local_20 [4];
  
                    /* Unresolved local var: Vector vecDirection@[???]
                       Unresolved local var: float flLaserWidth@[???]
                       Unresolved local var: Vector vecMins@[???]
                       Unresolved local var: Vector vecMaxs@[???]
                       Unresolved local var: CBaseEntity *[512] list@[???]
                       Unresolved local var: Ray_t ray@[???]
                       Unresolved local var: int nCount@[???]
                       Unresolved local var: CBaseEntity * pEntity@[???]
                       Unresolved local var: CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>
                       vsrtVictims@[???]
                       Unresolved local var: bool bBestIsTurret@[???] */
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: Vector res@[???] */
  local_38 = vecEnd->x - vecStart->x;
  local_34 = vecEnd->y - vecStart->y;
  local_30 = vecEnd->z - vecStart->z;
  root[0] = SQRT(local_38 * local_38 + local_34 * local_34 + local_30 * local_30);
  root[1] = 0.0;
  root[2] = 0.0;
  root[3] = 0.0;
  fVar12 = *(float *)(unaff_EBX + 0x3c53f7) * root[0];
  pAttacker_1 = (CBaseEntity *)(fVar12 + *(float *)(unaff_EBX + 0x3c09b3));
  if (((float)pAttacker_1 < *(float *)(unaff_EBX + 0x3c1b77) ||
       (float)pAttacker_1 == *(float *)(unaff_EBX + 0x3c1b77)) &&
     (pAttacker_1 = (CBaseEntity *)((float)pAttacker_1 + fVar12),
     *(float *)(unaff_EBX + 0x3c09b3) <= (float)pAttacker_1)) {
    pAttacker_1 = (CBaseEntity *)((float)pAttacker_1 + fVar12);
  }
  local_2c.x = local_38;
  local_2c.y = local_34;
  local_2c.z = local_30;
  VectorNormalize(&local_2c);
  fVar12 = (float)((uint)pAttacker_1 ^ *(uint *)(unaff_EBX + 0x3c90ef));
  local_16c.m_Delta.super_Vector.x = vecEnd->x - vecStart->x;
  local_16c.m_Delta.super_Vector.y = vecEnd->y - vecStart->y;
  local_16c.m_Delta.super_Vector.z = vecEnd->z - vecStart->z;
  local_16c.m_pWorldAxisTransform = (matrix3x4_t *)0x0;
  local_16c.m_IsSwept =
       local_16c.m_Delta.super_Vector.x * local_16c.m_Delta.super_Vector.x +
       local_16c.m_Delta.super_Vector.y * local_16c.m_Delta.super_Vector.y +
       local_16c.m_Delta.super_Vector.z * local_16c.m_Delta.super_Vector.z != 0.0;
  local_16c.m_Extents.super_Vector.x =
       ((float)pAttacker_1 - fVar12) * *(float *)(unaff_EBX + 0x3c092f);
  local_16c.m_IsRay =
       (double)(local_16c.m_Extents.super_Vector.x * local_16c.m_Extents.super_Vector.x +
                local_16c.m_Extents.super_Vector.x * local_16c.m_Extents.super_Vector.x +
               local_16c.m_Extents.super_Vector.x * local_16c.m_Extents.super_Vector.x) <
       *(double *)(unaff_EBX + 0x3c0b1f);
  local_16c.m_StartOffset.super_Vector.z =
       ((float)pAttacker_1 + fVar12) * *(float *)(unaff_EBX + 0x3c092f);
  local_16c.m_Start.super_Vector.x = vecStart->x + local_16c.m_StartOffset.super_Vector.z;
  local_16c.m_Start.super_Vector.y = vecStart->y + local_16c.m_StartOffset.super_Vector.z;
  local_16c.m_Start.super_Vector.z = vecStart->z + local_16c.m_StartOffset.super_Vector.z;
  local_16c.m_StartOffset.super_Vector.x = -local_16c.m_StartOffset.super_Vector.z;
  local_16c.m_StartOffset.super_Vector.y = -local_16c.m_StartOffset.super_Vector.z;
  local_16c.m_StartOffset.super_Vector.z = -local_16c.m_StartOffset.super_Vector.z;
                    /* Unresolved local var: CFlaggedEntitiesEnum rayEnum@[???] */
  local_16c.m_Extents.super_Vector.y = local_16c.m_Extents.super_Vector.x;
  local_16c.m_Extents.super_Vector.z = local_16c.m_Extents.super_Vector.x;
  CFlaggedEntitiesEnum::CFlaggedEntitiesEnum
            ((CFlaggedEntitiesEnum *)&local_1bc,local_a0c,0x200,0x2002080);
  iVar4 = UTIL_EntitiesAlongRay(&local_16c,(CFlaggedEntitiesEnum *)&local_1bc);
  local_118.m_Memory.m_pMemory = (__375 *)0x0;
  local_118.m_Memory.m_nAllocationCount = 0;
  local_118.m_Memory.m_nGrowSize = 0;
  local_118.m_Size = 0;
  local_118.m_pElements = (__375 *)0x0;
  local_104 = 0.0;
  local_100 = 0;
  if (0 < iVar4) {
    local_a1c = 0;
    pszClassOrWildcard = (char *)(unaff_EBX + 0x3a6993);
    pszClassOrWildcard_00 = (char *)(unaff_EBX + 0x354f33);
    do {
      this_00 = (CPortal_Player *)local_a0c[local_a1c];
      if ((((this_00 != (CPortal_Player *)0x0) &&
           ((((root[0] = (float)pszClassOrWildcard,
              (this_00->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
              super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
              super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
              super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
              m_iClassname.pszValue == pszClassOrWildcard ||
              (bVar2 = CBaseEntity::ClassMatchesComplex((CBaseEntity *)this_00,pszClassOrWildcard),
              bVar2)) ||
             (root[0] = (float)pszClassOrWildcard_00,
             (this_00->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
             super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
             super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
             m_iClassname.pszValue == pszClassOrWildcard_00)) ||
            (bVar2 = CBaseEntity::ClassMatchesComplex((CBaseEntity *)this_00,pszClassOrWildcard_00),
            bVar2)))) ||
          (iVar5 = (*(*(_func_int_varargs ***)
                       &(this_00->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                        super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
                        super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.
                        super_CBaseCombatCharacter.super_CBaseFlex)[0x55])(this_00),
          (char)iVar5 != '\0')) &&
         (iVar5 = (*(*(_func_int_varargs ***)
                      &(this_00->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                       super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
                       super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.
                       super_CBaseCombatCharacter.super_CBaseFlex)[0x45])(this_00),
         (char)iVar5 != '\0')) {
        if (((this_00->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
             super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
             super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
             m_iEFlags & 0x800) != 0) {
          CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this_00);
        }
        CalcClosestPointOnLineSegment
                  (&(this_00->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
                    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.
                    super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                    super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin,vecStart,vecEnd,&local_44,
                   local_20);
        if (((this_00->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
             super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
             super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
             m_iEFlags & 0x800) != 0) {
          CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this_00);
        }
                    /* Unresolved local var: Vector res@[???] */
        fVar12 = local_44.x -
                 (this_00->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                 super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
                 super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
                 super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                 m_vecAbsOrigin.x;
        fVar13 = local_44.y -
                 (this_00->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                 super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
                 super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
                 super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                 m_vecAbsOrigin.y;
        fVar12 = fVar13 * fVar13 + fVar12 * fVar12;
        if ((fVar12 < *(float *)(unaff_EBX + 0x3c1bab) || fVar12 == *(float *)(unaff_EBX + 0x3c1bab)
            ) || (this->m_bFromReflectedCube != false)) {
          iVar5 = (*(*(_func_int_varargs ***)
                      &(this_00->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                       super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
                       super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.
                       super_CBaseCombatCharacter.super_CBaseFlex)[0x55])(this_00);
          if ((char)iVar5 == '\0') {
LAB_0070c665:
            fVar12 = local_20[0];
            iVar5 = local_118.m_Size;
            if (0.0 < local_20[0]) {
              local_100 = 1;
              CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>::GrowVector
                        (&local_118,1);
              if (~iVar5 + local_118.m_Size < 1) {
                iVar11 = iVar5 << 3;
              }
              else {
                iVar11 = iVar5 * 8;
                _V_memmove(local_118.m_Memory.m_pMemory + iVar5 + 1,
                           local_118.m_Memory.m_pMemory + iVar5,(~iVar5 + local_118.m_Size) * 8);
              }
              puVar6 = (undefined4 *)((int)&(local_118.m_Memory.m_pMemory)->pVictim + iVar11);
              if (puVar6 != (undefined4 *)0x0) {
                puVar6[1] = fVar12;
                *puVar6 = this_00;
              }
            }
          }
          else {
                    /* Unresolved local var: float flZDist@[???] */
            if ((*(int *)(*(int *)(unaff_EBX + 0x6bbf6b) + 0x30) != 0) &&
               ((pCVar7 = CBaseEntity::GetGroundEntity((CBaseEntity *)this_00),
                pCVar7 != (CBaseEntity *)0x0 ||
                (iVar5 = (*(*(_func_int_varargs ***)
                             &(this_00->
                              super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                              super_CPaintableEntity<CBaseMultiplayerPlayer>.
                              super_CBaseMultiplayerPlayer.super_CAI_ExpresserHost<CBasePlayer>.
                              super_CBasePlayer.super_CBaseCombatCharacter.super_CBaseFlex)[0x9d])
                                   (this_00), *(float *)(iVar5 + 8) - local_44.z < 0.0)))) {
              iVar5 = (*(*(_func_int_varargs ***)
                          &(this_00->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>
                           ).super_CPaintableEntity<CBaseMultiplayerPlayer>.
                           super_CBaseMultiplayerPlayer.super_CAI_ExpresserHost<CBasePlayer>.
                           super_CBasePlayer.super_CBaseCombatCharacter.super_CBaseFlex)[0x55])
                                (this_00);
              this_01 = (CPortal_Player *)0x0;
              if ((char)iVar5 != '\0') {
                this_01 = this_00;
              }
              if (((this_01->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                   super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
                   super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter
                   .super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                   super_CBaseEntity.m_iEFlags & 0x800) != 0) {
                CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this_01);
              }
              fVar13 = (this_01->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                       super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
                       super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.
                       super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                       super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z;
              pVVar8 = CPortal_Player::GetHullMaxs(this_01);
              if ((local_44.z <= fVar13 + pVVar8->z) &&
                 (fVar12 < *(float *)(unaff_EBX + 0x3c1bab) ||
                  fVar12 == *(float *)(unaff_EBX + 0x3c1bab))) goto LAB_0070c665;
            }
          }
        }
      }
      local_a1c = local_a1c + 1;
    } while (local_a1c != iVar4);
  }
  local_100 = 0;
  if (1 < local_118.m_Size) {
    local_44.x = local_104;
    local_44.y = (vec_t)&local_50;
    **(int **)(unaff_EBX + 0x53ad73) = (int)&local_44;
    _qsort(local_118.m_Memory.m_pMemory,local_118.m_Size,8,*(int **)(unaff_EBX + 0x656dcf));
  }
  if (0 < local_118.m_Size) {
    bVar2 = false;
    local_a14 = 0;
    do {
      pCVar7 = local_118.m_Memory.m_pMemory[local_a14].pVictim;
      if (pCVar7 == (CBaseEntity *)0x0) {
LAB_0070c854:
        iVar4 = (*(pCVar7->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                  _vptr_IHandleEntity[0x55])(pCVar7);
        if (((char)iVar4 != '\0') && ((pCVar7->m_MoveType).m_Value != '\b')) {
          if ((pCVar7->m_iEFlags & 0x1000) != 0) {
            CBaseEntity::CalcAbsoluteVelocity(pCVar7);
          }
          local_44.x = (pCVar7->m_vecAbsVelocity).x;
          local_44.y = (pCVar7->m_vecAbsVelocity).y;
          local_44.z = (pCVar7->m_vecAbsVelocity).z;
          VectorNormalize(&local_44);
          if ((pCVar7->m_iEFlags & 0x800) != 0) {
            CBaseEntity::CalcAbsolutePosition(pCVar7);
          }
          CalcClosestPointOnLineSegment
                    (&pCVar7->m_vecAbsOrigin,vecStart,vecEnd,&local_50,(float *)0x0);
          if ((pCVar7->m_iEFlags & 0x800) != 0) {
            CBaseEntity::CalcAbsolutePosition(pCVar7);
          }
          local_5c = (pCVar7->m_vecAbsOrigin).x;
          local_58 = (pCVar7->m_vecAbsOrigin).y;
          local_54 = local_50.z;
                    /* Unresolved local var: Vector res@[???] */
          local_74 = local_5c - local_50.x;
          local_70 = local_58 - local_50.y;
          local_6c = local_50.z - local_50.z;
          local_68.x = local_74;
          local_68.y = local_70;
          local_68.z = local_6c;
          VectorNormalize(&local_68);
          local_68.z = 0.0;
          if (*(float *)(unaff_EBX + 0x3c5877) <=
              local_44.x * local_44.x + local_44.y * local_44.y + local_44.z * local_44.z) {
                    /* Unresolved local var: float flDot@[???] */
            fVar12 = local_44.x * local_68.x + local_44.y * local_68.y + local_44.z * 0.0;
                    /* Unresolved local var: Vector res@[???] */
            local_c8 = local_68.x * *(float *)(unaff_EBX + 0x3c0db7);
            local_c4 = *(float *)(unaff_EBX + 0x3c0db7) * local_68.y;
            local_c0 = 0x80000000;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
            local_a4 = fVar12 * local_c8 + local_44.x;
            local_a0 = fVar12 * local_c4 + local_44.y;
            local_9c = fVar12 * -0.0 + local_44.z;
            local_80.x = local_a4;
            local_80.y = local_a0;
            local_80.z = local_9c;
            VectorNormalize(&local_80);
            local_80.z = 0.0;
            if (local_80.x * local_68.x + local_80.y * local_68.y + local_68.z * 0.0 < 0.0)
            goto LAB_0070cbf0;
          }
          else {
            local_8c = 0;
            local_88 = 0;
            local_84 = 0x3f800000;
            local_bc = local_2c.y - local_2c.z * 0.0;
            local_b8 = local_2c.z * 0.0 - local_2c.x;
            local_b4 = local_2c.x * 0.0 - local_2c.y * 0.0;
            local_98 = local_bc;
            local_94 = local_b8;
            local_90 = local_b4;
            local_80.x = local_bc;
            local_80.y = local_b8;
            local_80.z = local_b4;
          }
                    /* Unresolved local var: Vector res@[???] */
          pfVar1 = (float *)(&DAT_003c0a47 + unaff_EBX);
          local_1bc.m_vecDamageForce.x = local_80.x * *pfVar1;
          local_1bc.m_vecDamageForce.y = local_80.y * *pfVar1;
          local_1bc.m_vecDamageForce.z = *pfVar1 * local_80.z;
          local_b0 = local_1bc.m_vecDamageForce.x;
          local_ac = local_1bc.m_vecDamageForce.y;
          local_a8 = local_1bc.m_vecDamageForce.z;
          if (((pCVar7->m_fFlags).m_Value & 1) != 0) {
            CBaseEntity::SetGroundEntity(pCVar7,(CBaseEntity *)0x0);
            CBaseEntity::SetGroundChangeTime
                      (pCVar7,*(float *)(unaff_EBX + 0x3c092f) +
                              *(float *)(**(int **)(unaff_EBX + 0x53a993) + 0xc));
            local_1bc.m_vecDamageForce.x =
                 local_1bc.m_vecDamageForce.x + local_1bc.m_vecDamageForce.x;
            local_1bc.m_vecDamageForce.y =
                 local_1bc.m_vecDamageForce.y + local_1bc.m_vecDamageForce.y;
            local_1bc.m_vecDamageForce.z =
                 local_1bc.m_vecDamageForce.z + local_1bc.m_vecDamageForce.z;
          }
          CBaseEntity::SetAbsVelocity(pCVar7,&local_1bc.m_vecDamageForce);
          if (this->m_bIsLethal == false) {
            fVar12 = *(float *)(unaff_EBX + 0x3c0acf);
          }
          else {
            fVar12 = *(float *)(unaff_EBX + 0x3c3b83);
          }
                    /* Unresolved local var: CTakeDamageInfo info@[???] */
          CTakeDamageInfo::CTakeDamageInfo
                    (&local_20c,(CBaseEntity *)this,(CBaseEntity *)this,
                     fVar12 * *(float *)(**(int **)(unaff_EBX + 0x53a993) + 0x10),8,0,
                     in_stack_fffff59c,in_stack_fffff5a0,pAttacker_1,in_stack_fffff5a8,
                     in_stack_fffff5ac,in_stack_fffff5b0);
          pVVar8 = (Vector *)
                   (*(pCVar7->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                     _vptr_IHandleEntity[0x9d])(pCVar7);
          local_e0.x = 1.0;
          local_e0.y = 0.0;
          local_e0.z = 0.0;
          CalculateMeleeDamageForce(&local_20c,&local_e0,pVVar8,1.0);
          CBaseEntity::TakeDamage(pCVar7,&local_20c);
          CBaseEntity::EmitSound(pCVar7,(char *)(unaff_EBX + 0x3a4449),0.0,(float *)0x0);
        }
      }
      else {
        root[0] = (float)(unaff_EBX + 0x3a6993);
        if ((((pCVar7->m_iClassname).pszValue != (char *)root[0]) &&
            (bVar3 = CBaseEntity::ClassMatchesComplex(pCVar7,(char *)root[0]), !bVar3)) || (bVar2))
        {
          root[0] = (float)(unaff_EBX + 0x354f33);
          if (((pCVar7->m_iClassname).pszValue != (char *)root[0]) &&
             (bVar3 = CBaseEntity::ClassMatchesComplex(pCVar7,(char *)root[0]), !bVar3))
          goto LAB_0070c854;
          bVar2 = true;
        }
        else {
                    /* Unresolved local var: CTakeDamageInfo info@[???] */
          CTakeDamageInfo::CTakeDamageInfo
                    (&local_1bc,(CBaseEntity *)this,(CBaseEntity *)this,
                     *(float *)(**(int **)(unaff_EBX + 0x53a993) + 0x10),8,0,in_stack_fffff59c,
                     in_stack_fffff5a0,pAttacker_1,in_stack_fffff5a8,in_stack_fffff5ac,
                     in_stack_fffff5b0);
          pVVar8 = (Vector *)
                   (*(pCVar7->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                     _vptr_IHandleEntity[0x9d])(pCVar7);
          local_d4.x = 1.0;
          local_d4.y = 0.0;
          local_d4.z = 0.0;
          CalculateMeleeDamageForce(&local_1bc,&local_d4,pVVar8,1.0);
          CBaseEntity::TakeDamage(pCVar7,&local_1bc);
          piVar9 = (int *)___dynamic_cast(pCVar7,*(undefined4 *)(unaff_EBX + 0x53a9bf),
                                          *(undefined4 *)(&DAT_0053b4bb + unaff_EBX),0);
          if ((piVar9 != (int *)0x0) && (*(char *)((int)piVar9 + 0x3f1) != '\0')) {
            bVar2 = true;
            if (pVecOut != (Vector *)0x0) {
              pvVar10 = (vec_t *)(**(code **)(*piVar9 + 0x274))(piVar9);
              pVecOut->x = *pvVar10;
              pVecOut->y = pvVar10[1];
              pVecOut->z = pvVar10[2];
            }
            goto LAB_0070ce8c;
          }
        }
      }
LAB_0070cbf0:
      local_a14 = local_a14 + 1;
    } while (local_a14 < local_118.m_Size);
  }
  bVar2 = false;
  if (pVecOut != (Vector *)0x0) {
    pVecOut->x = vecEnd->x;
    pVecOut->y = vecEnd->y;
    pVecOut->z = vecEnd->z;
  }
LAB_0070ce8c:
  local_118.m_Size = 0;
  if (-1 < local_118.m_Memory.m_nGrowSize) {
    if (local_118.m_Memory.m_pMemory != (__375 *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53a98b) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x53a98b),local_118.m_Memory.m_pMemory);
      local_118.m_Memory.m_pMemory = (__375 *)0x0;
    }
    local_118.m_Memory.m_nAllocationCount = 0;
  }
  local_118.m_pElements = local_118.m_Memory.m_pMemory;
  if ((-1 < local_118.m_Memory.m_nGrowSize) && (local_118.m_Memory.m_pMemory != (__375 *)0x0)) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53a98b) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x53a98b),local_118.m_Memory.m_pMemory);
  }
  return bVar2;
}


/* CPortalLaser::DamageEntitiesAlongLaser at 0070e110 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void DamageEntitiesAlongLaser(CPortalLaser * this,
   CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t,_int>_> *
   infoList, bool bAutoAim) */

void __thiscall
CPortalLaser::DamageEntitiesAlongLaser
          (CPortalLaser *this,
          CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t,_int>_>
          *infoList,bool bAutoAim)

{
  undefined1 *puVar1;
  uint uVar2;
  ushort uVar3;
  int iVar4;
  CBaseEntity *this_00;
  CBaseEdict *this_01;
  ushort *puVar5;
  bool bVar6;
  bool bVar7;
  Vector *vLineA;
  int *piVar8;
  QAngle *angles;
  Vector *pVVar9;
  Vector *pVVar10;
  Vector *origin;
  int iVar11;
  IChangeInfoAccessor *pIVar12;
  uint uVar13;
  uint uVar14;
  int unaff_EBX;
  float fVar15;
  CTakeDamageInfo *in_stack_fffffe1c;
  CBaseEntity *in_stack_fffffe20;
  CBaseEntity *in_stack_fffffe24;
  float in_stack_fffffe28;
  int in_stack_fffffe2c;
  int in_stack_fffffe30;
  bool bBlockTarget;
  int local_1b0;
  int local_1a8;
  int local_190;
  CTakeDamageInfo local_180;
  CTakeDamageInfo local_130;
  Vector local_e0;
  Vector local_d4;
  Vector local_c8;
  float local_bc;
  float local_b8;
  undefined4 local_b4;
  float local_b0;
  float local_ac;
  float local_a8;
  float local_a4;
  float local_a0;
  float local_9c;
  float local_98;
  float local_94;
  float local_90;
  float local_8c;
  float local_88;
  float local_84;
  undefined4 local_80;
  undefined4 local_7c;
  undefined4 local_78;
  float local_74;
  float local_70;
  float local_6c;
  Vector local_68;
  float local_5c;
  float local_58;
  float local_54;
  Vector local_50;
  float local_44;
  float local_40;
  float local_3c;
  Vector local_38;
  Vector local_2c;
  char *local_20;
  
                    /* Unresolved local var: bool bDebug@[???] */
  ___i686_get_pc_thunk_bx();
  iVar4 = *(int *)(*(int *)(unaff_EBX + 0x6ba19b) + 0x30);
  if (0 < infoList->m_Size) {
    bVar6 = false;
    local_1b0 = 0;
    local_190 = 0;
    do {
      vLineA = (Vector *)((int)&(((infoList->m_Memory).m_pMemory)->vecStart).x + local_190);
      if (0 < (int)vLineA[3].x) {
        local_1a8 = 0;
        do {
          this_00 = *(CBaseEntity **)((int)vLineA[2].x + local_1a8 * 4);
                    /* Unresolved local var: ICollideable * pCollision@[???] */
          if ((iVar4 != 0) &&
             (piVar8 = (int *)(*(this_00->super_IServerEntity).super_IServerUnknown.
                                super_IHandleEntity._vptr_IHandleEntity[4])(this_00),
             piVar8 != (int *)0x0)) {
            angles = (QAngle *)(**(code **)(*piVar8 + 0x24))(piVar8);
            pVVar9 = (Vector *)(**(code **)(*piVar8 + 8))(piVar8);
            pVVar10 = (Vector *)(**(code **)(*piVar8 + 4))(piVar8);
            origin = (Vector *)(**(code **)(*piVar8 + 0x20))(piVar8);
            in_stack_fffffe24 = (CBaseEntity *)0x0;
            in_stack_fffffe20 = (CBaseEntity *)0x0;
            in_stack_fffffe1c = (CTakeDamageInfo *)0x0;
            NDebugOverlay::BoxAngles(origin,pVVar10,pVVar9,angles,0xff,0xff,0,0,0.0);
          }
          if (this_00 == (CBaseEntity *)0x0) {
LAB_0070e3cd:
            iVar11 = (*(this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                       _vptr_IHandleEntity[0x55])(this_00);
            if (((char)iVar11 != '\0') && ((this_00->m_MoveType).m_Value != '\b')) {
              if ((this_00->m_iEFlags & 0x1000) != 0) {
                CBaseEntity::CalcAbsoluteVelocity(this_00);
              }
              local_2c.x = (this_00->m_vecAbsVelocity).x;
              local_2c.y = (this_00->m_vecAbsVelocity).y;
              local_2c.z = (this_00->m_vecAbsVelocity).z;
              VectorNormalize(&local_2c);
              if ((this_00->m_iEFlags & 0x800) != 0) {
                CBaseEntity::CalcAbsolutePosition(this_00);
              }
              CalcClosestPointOnLineSegment
                        (&this_00->m_vecAbsOrigin,vLineA,vLineA + 1,&local_38,(float *)0x0);
              if ((this_00->m_iEFlags & 0x800) != 0) {
                CBaseEntity::CalcAbsolutePosition(this_00);
              }
              local_44 = (this_00->m_vecAbsOrigin).x;
              local_40 = (this_00->m_vecAbsOrigin).y;
              local_3c = local_38.z;
                    /* Unresolved local var: Vector res@[???] */
              local_5c = local_44 - local_38.x;
              local_58 = local_40 - local_38.y;
              local_54 = local_38.z - local_38.z;
              local_50.x = local_5c;
              local_50.y = local_58;
              local_50.z = local_54;
              VectorNormalize(&local_50);
              local_50.z = 0.0;
              fVar15 = local_2c.x * local_2c.x + local_2c.y * local_2c.y + local_2c.z * local_2c.z;
              if ((*(float *)(&DAT_003c3927 + unaff_EBX) <= fVar15) ||
                 (NAN(fVar15) || NAN(*(float *)(&DAT_003c3927 + unaff_EBX)))) {
                    /* Unresolved local var: float flDot@[???] */
                fVar15 = local_2c.x * local_50.x + local_2c.y * local_50.y + local_2c.z * 0.0;
                    /* Unresolved local var: Vector res@[???] */
                local_bc = local_50.x * *(float *)(unaff_EBX + 0x3bee67);
                local_b8 = *(float *)(unaff_EBX + 0x3bee67) * local_50.y;
                local_b4 = 0x80000000;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
                local_98 = fVar15 * local_bc + local_2c.x;
                local_94 = fVar15 * local_b8 + local_2c.y;
                local_90 = fVar15 * -0.0 + local_2c.z;
                local_68.x = local_98;
                local_68.y = local_94;
                local_68.z = local_90;
                VectorNormalize(&local_68);
                local_68.z = 0.0;
                if (local_68.x * local_50.x + local_68.y * local_50.y + local_50.z * 0.0 < 0.0)
                goto LAB_0070e2d0;
              }
              else {
                    /* Unresolved local var: Vector vecDirection@[???]
                       Unresolved local var: Vector res@[???] */
                local_180.m_vecDamageForce.x = vLineA[1].x - vLineA->x;
                local_180.m_vecDamageForce.y = vLineA[1].y - vLineA->y;
                local_180.m_vecDamageForce.z = vLineA[1].z - vLineA->z;
                local_74 = local_180.m_vecDamageForce.x;
                local_70 = local_180.m_vecDamageForce.y;
                local_6c = local_180.m_vecDamageForce.z;
                VectorNormalize(&local_180.m_vecDamageForce);
                local_80 = 0;
                local_7c = 0;
                local_78 = 0x3f800000;
                local_b0 = local_180.m_vecDamageForce.y - local_180.m_vecDamageForce.z * 0.0;
                local_ac = local_180.m_vecDamageForce.z * 0.0 - local_180.m_vecDamageForce.x;
                local_a8 = local_180.m_vecDamageForce.x * 0.0 - local_180.m_vecDamageForce.y * 0.0;
                local_8c = local_b0;
                local_88 = local_ac;
                local_84 = local_a8;
                local_68.x = local_b0;
                local_68.y = local_ac;
                local_68.z = local_a8;
              }
                    /* Unresolved local var: Vector res@[???] */
              local_130.m_vecDamageForce.z = *(float *)(unaff_EBX + 0x3beaf7);
              local_130.m_vecDamageForce.x = local_68.x * local_130.m_vecDamageForce.z;
              local_130.m_vecDamageForce.y = local_68.y * local_130.m_vecDamageForce.z;
              local_130.m_vecDamageForce.z = local_130.m_vecDamageForce.z * local_68.z;
              local_a4 = local_130.m_vecDamageForce.x;
              local_a0 = local_130.m_vecDamageForce.y;
              local_9c = local_130.m_vecDamageForce.z;
              if (((this_00->m_fFlags).m_Value & 1) == 0) {
                piVar8 = *(int **)(unaff_EBX + 0x538a43);
              }
              else {
                CBaseEntity::SetGroundEntity(this_00,(CBaseEntity *)0x0);
                piVar8 = *(int **)(unaff_EBX + 0x538a43);
                CBaseEntity::SetGroundChangeTime
                          (this_00,*(float *)(unaff_EBX + 0x3be9df) + *(float *)(*piVar8 + 0xc));
                local_130.m_vecDamageForce.x =
                     local_130.m_vecDamageForce.x + local_130.m_vecDamageForce.x;
                local_130.m_vecDamageForce.y =
                     local_130.m_vecDamageForce.y + local_130.m_vecDamageForce.y;
                local_130.m_vecDamageForce.z =
                     local_130.m_vecDamageForce.z + local_130.m_vecDamageForce.z;
              }
              CBaseEntity::SetAbsVelocity(this_00,&local_130.m_vecDamageForce);
              if (this->m_bIsLethal == false) {
                fVar15 = *(float *)(unaff_EBX + 0x3beb7f);
              }
              else {
                fVar15 = *(float *)(unaff_EBX + 0x3c1c33);
              }
                    /* Unresolved local var: CTakeDamageInfo info@[???] */
              CTakeDamageInfo::CTakeDamageInfo
                        (&local_180,(CBaseEntity *)this,(CBaseEntity *)this,
                         fVar15 * *(float *)(*piVar8 + 0x10),8,0,in_stack_fffffe1c,in_stack_fffffe20
                         ,in_stack_fffffe24,in_stack_fffffe28,in_stack_fffffe2c,in_stack_fffffe30);
              pVVar9 = (Vector *)
                       (*(this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                         _vptr_IHandleEntity[0x9d])(this_00);
              local_e0.x = 1.0;
              local_e0.y = 0.0;
              local_e0.z = 0.0;
              CalculateMeleeDamageForce(&local_180,&local_e0,pVVar9,1.0);
              CBaseEntity::TakeDamage(this_00,&local_180);
              CBaseEntity::EmitSound(this_00,(char *)(unaff_EBX + 0x3a24f9),0.0,(float *)0x0);
            }
          }
          else {
            local_20 = (char *)(unaff_EBX + 0x3a4a43);
            if ((((this_00->m_iClassname).pszValue != local_20) &&
                (bVar7 = CBaseEntity::ClassMatchesComplex(this_00,local_20), !bVar7)) || (bVar6)) {
              local_20 = (char *)(unaff_EBX + 0x352fe3);
              if (((this_00->m_iClassname).pszValue != local_20) &&
                 (bVar7 = CBaseEntity::ClassMatchesComplex(this_00,local_20), !bVar7))
              goto LAB_0070e3cd;
              bVar6 = true;
            }
            else {
              if (*(char *)((int)&this_00[1].m_Network.m_PVSInfo.m_nAreaNum + 1) == '\0') {
                CTakeDamageInfo::CTakeDamageInfo
                          (&local_130,(CBaseEntity *)this,(CBaseEntity *)this,
                           *(float *)(**(int **)(unaff_EBX + 0x538a43) + 0x10),8,0,in_stack_fffffe1c
                           ,in_stack_fffffe20,in_stack_fffffe24,in_stack_fffffe28,in_stack_fffffe2c,
                           in_stack_fffffe30);
                pVVar10 = (Vector *)
                          (*(this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                            _vptr_IHandleEntity[0x9d])(this_00);
                local_d4.x = 1.0;
                local_d4.y = 0.0;
                local_d4.z = 0.0;
                pVVar9 = &local_d4;
              }
              else {
                if ((this->m_bShouldSpark).m_Value != false) {
                  if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.
                      m_bRegistered == false) {
                    this_01 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                               super_CBaseEdict;
                    if ((this_01 != (CBaseEdict *)0x0) && ((this_01->m_fStateFlags & 0x100U) == 0))
                    {
                      this_01->m_fStateFlags = this_01->m_fStateFlags | 1;
                      pIVar12 = CBaseEdict::GetChangeAccessor(this_01);
                      piVar8 = *(int **)(unaff_EBX + 0x538a47);
                      puVar5 = (ushort *)*piVar8;
                      if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                        uVar13 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
                        uVar3 = puVar5[uVar13 * 0x14 + 0x14];
                        if (uVar3 == 0) {
LAB_0070eaea:
                          puVar5[(uint)uVar3 + uVar13 * 0x14 + 1] = 0x7dc;
                          puVar5[uVar13 * 0x14 + 0x14] = uVar3 + 1;
                        }
                        else if (puVar5[uVar13 * 0x14 + 1] != 0x7dc) {
                          uVar14 = 0;
                          do {
                            uVar2 = uVar14 + 1;
                            uVar14 = uVar2 & 0xffff;
                            if ((ushort)uVar2 == uVar3) {
                              if (uVar3 != 0x13) goto LAB_0070eaea;
                              pIVar12->m_iChangeInfoSerialNumber = 0;
                              this_01->m_fStateFlags = this_01->m_fStateFlags | 0x100;
                              break;
                            }
                          } while (puVar5[uVar13 * 0x14 + uVar14 + 1] != 0x7dc);
                        }
                      }
                      else if (puVar5[0x7d1] == 100) {
                        pIVar12->m_iChangeInfoSerialNumber = 0;
                        this_01->m_fStateFlags = this_01->m_fStateFlags | 0x100;
                      }
                      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                        pIVar12->m_iChangeInfo = puVar5[0x7d1];
                        *(short *)(*piVar8 + 0xfa2) = *(short *)(*piVar8 + 0xfa2) + 1;
                        pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar8;
                        iVar11 = *piVar8 + (uint)pIVar12->m_iChangeInfo * 0x28;
                        *(undefined2 *)(iVar11 + 2) = 0x7dc;
                        *(undefined2 *)(iVar11 + 0x28) = 1;
                      }
                    }
                  }
                  else {
                    puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
                    *(uint *)puVar1 = *(uint *)puVar1 | 1;
                  }
                  (this->m_bShouldSpark).m_Value = false;
                }
                if ((!bAutoAim) && (this->m_bFromReflectedCube == false)) goto LAB_0070e2d0;
                CTakeDamageInfo::CTakeDamageInfo
                          (&local_130,(CBaseEntity *)this,(CBaseEntity *)this,
                           *(float *)(**(int **)(unaff_EBX + 0x538a43) + 0x10),8,0,in_stack_fffffe1c
                           ,in_stack_fffffe20,in_stack_fffffe24,in_stack_fffffe28,in_stack_fffffe2c,
                           in_stack_fffffe30);
                pVVar10 = (Vector *)
                          (*(this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                            _vptr_IHandleEntity[0x9d])(this_00);
                local_c8.x = 1.0;
                local_c8.y = 0.0;
                local_c8.z = 0.0;
                pVVar9 = &local_c8;
              }
              CalculateMeleeDamageForce(&local_130,pVVar9,pVVar10,1.0);
              CBaseEntity::TakeDamage(this_00,&local_130);
            }
          }
LAB_0070e2d0:
          local_1a8 = local_1a8 + 1;
        } while (local_1a8 < (int)vLineA[3].x);
      }
      local_1b0 = local_1b0 + 1;
      local_190 = local_190 + 0x2c;
    } while (local_1b0 < infoList->m_Size);
  }
  return;
}


/* CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::CompareHelper at 00711e00 */

int CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::CompareHelper(__375 *lhs,__375 *rhs)

{
  uint uVar1;
  
                    /* Unresolved local var: QSortContext_t * ctx@[???] */
  uVar1 = 0xffffffff;
  if (rhs->flFraction <= lhs->flFraction) {
    uVar1 = (uint)(rhs->flFraction < lhs->flFraction);
  }
  return uVar1;
}


/* __tcf_9 at 00a3bf80 */

void __tcf_9(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38c3f8),in_stack_00000008);
  return;
}


/* CPortalLaser::GetEntitiesAlongLaser at 0070a690 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Restarted to delay deadcode elimination for space: stack */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: CBaseEntity * GetEntitiesAlongLaser(CPortalLaser * this, Vector *
   vecStart, Vector * vecEnd, Vector * pVecOut,
   CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t,_int>_> *
   infoList, bool bIsFirstTrace) */

CBaseEntity * __thiscall
CPortalLaser::GetEntitiesAlongLaser
          (CPortalLaser *this,Vector *vecStart,Vector *vecEnd,Vector *pVecOut,
          CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t,_int>_>
          *infoList,bool bIsFirstTrace)

{
  char *pcVar1;
  char *pcVar2;
  uint uVar3;
  CPortal_Player *this_00;
  bool bVar4;
  bool bVar5;
  int iVar6;
  undefined4 *puVar7;
  CBaseEntity *pCVar8;
  Vector *pVVar9;
  int *piVar10;
  PortalLaserInfo_t *pPVar11;
  vec_t *pvVar12;
  int iVar13;
  int unaff_EBX;
  int iVar14;
  CPortal_Player *this_01;
  float fVar15;
  float fVar16;
  CBaseEntity **ppCVar17;
  CFlaggedEntitiesEnum *local_93c;
  CBaseEntity *local_91c;
  int local_914;
  int local_90c;
  int local_900;
  CBaseEntity *local_8fc [512];
  Ray_t local_fc;
  CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_> local_ac;
  _func_int_varargs **local_98;
  undefined1 local_94;
  CFlaggedEntitiesEnum local_90;
  char *local_7c;
  undefined4 uStack_78;
  undefined4 uStack_74;
  undefined4 uStack_70;
  Vector local_60;
  Vector local_54;
  Vector local_48;
  float local_3c;
  float local_38;
  float local_34;
  Vector local_30;
  float local_24;
  CBaseEntity *local_1d [3];
  
                    /* Unresolved local var: PortalLaserInfo_t * info@[???]
                       Unresolved local var: Vector vecDirection@[???]
                       Unresolved local var: float flLaserWidth@[???]
                       Unresolved local var: Vector vecMins@[???]
                       Unresolved local var: Vector vecMaxs@[???]
                       Unresolved local var: CBaseEntity *[512] list@[???]
                       Unresolved local var: Ray_t ray@[???]
                       Unresolved local var: int nCount@[???]
                       Unresolved local var: CBaseEntity * pEntity@[???]
                       Unresolved local var: CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>
                       vsrtVictims@[???]
                       Unresolved local var: bool bBlockTarget@[???] */
  ___i686_get_pc_thunk_bx();
  uVar3 = infoList->m_Size;
  iVar6 = uVar3 + 1;
  iVar13 = (infoList->m_Memory).m_nAllocationCount;
  if (iVar6 <= iVar13) goto LAB_0070a6d1;
  iVar14 = (infoList->m_Memory).m_nGrowSize;
  if (iVar14 < 0) goto LAB_0070a6d1;
  if (iVar14 == 0) {
    if ((iVar13 == 0) && (iVar13 = 1, iVar6 < 2)) {
      local_900 = 0x2c;
      goto LAB_0070b196;
    }
    do {
      iVar13 = iVar13 * 2;
    } while (iVar13 < iVar6);
LAB_0070b187:
    local_900 = iVar13 * 0x2c;
  }
  else {
    iVar13 = ((int)uVar3 / iVar14 + 1) * iVar14;
    if (iVar6 <= iVar13) goto LAB_0070b187;
    if (iVar13 != 0) {
LAB_0070b309:
      do {
        iVar13 = (iVar13 + iVar6) / 2;
      } while (iVar13 < iVar6);
      goto LAB_0070b187;
    }
    if (iVar6 < 0) {
      iVar13 = -1;
      local_900 = -0x2c;
    }
    else {
      local_900 = 0;
      if (iVar6 != 0) goto LAB_0070b309;
    }
  }
LAB_0070b196:
  (infoList->m_Memory).m_nAllocationCount = iVar13;
  pPVar11 = (infoList->m_Memory).m_pMemory;
  if (pPVar11 == (PortalLaserInfo_t *)0x0) {
    pPVar11 = (PortalLaserInfo_t *)
              (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x53c4bb))
                        ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x53c4bb),local_900);
    (infoList->m_Memory).m_pMemory = pPVar11;
  }
  else {
    pPVar11 = (PortalLaserInfo_t *)
              (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53c4bb) + 4))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x53c4bb),pPVar11,local_900);
    (infoList->m_Memory).m_pMemory = pPVar11;
  }
LAB_0070a6d1:
  infoList->m_Size = infoList->m_Size + 1;
  pPVar11 = (infoList->m_Memory).m_pMemory;
  infoList->m_pElements = pPVar11;
  iVar13 = ~uVar3 + infoList->m_Size;
  if (0 < iVar13) {
    _V_memmove(pPVar11 + iVar6,pPVar11 + uVar3,iVar13 * 0x2c);
  }
  pPVar11 = (infoList->m_Memory).m_pMemory + uVar3;
  if (pPVar11 != (PortalLaserInfo_t *)0x0) {
    (pPVar11->sortedEntList).m_Memory.m_pMemory = (CBaseEntity **)0x0;
    (pPVar11->sortedEntList).m_Memory.m_nAllocationCount = 0;
    (pPVar11->sortedEntList).m_Memory.m_nGrowSize = 0;
    (pPVar11->sortedEntList).m_Size = 0;
    (pPVar11->sortedEntList).m_pElements = (CBaseEntity **)0x0;
  }
  pPVar11 = (infoList->m_Memory).m_pMemory + infoList->m_Size + -1;
  (pPVar11->vecStart).x = vecStart->x;
  (pPVar11->vecStart).y = vecStart->y;
  (pPVar11->vecStart).z = vecStart->z;
  (pPVar11->vecEnd).x = vecEnd->x;
  (pPVar11->vecEnd).y = vecEnd->y;
  (pPVar11->vecEnd).z = vecEnd->z;
                    /* Unresolved local var: Vector res@[???] */
  local_3c = vecEnd->x - vecStart->x;
  local_38 = vecEnd->y - vecStart->y;
  local_34 = vecEnd->z - vecStart->z;
  if (bIsFirstTrace) {
                    /* Unresolved local var: __m128 root@[???] */
    local_7c = (char *)SQRT(local_3c * local_3c + local_38 * local_38 + local_34 * local_34);
    uStack_78 = 0;
    uStack_74 = 0;
    uStack_70 = 0;
    fVar15 = *(float *)(unaff_EBX + 0x3c6f27) * (float)local_7c;
    ppCVar17 = (CBaseEntity **)(fVar15 + *(float *)(unaff_EBX + 0x3c24e3));
    if (((float)ppCVar17 < *(float *)(unaff_EBX + 0x3c36a7) ||
         (float)ppCVar17 == *(float *)(unaff_EBX + 0x3c36a7)) &&
       (ppCVar17 = (CBaseEntity **)((float)ppCVar17 + fVar15),
       *(float *)(unaff_EBX + 0x3c24e3) <= (float)ppCVar17)) {
      ppCVar17 = (CBaseEntity **)((float)ppCVar17 + fVar15);
    }
  }
  else {
    ppCVar17 = *(CBaseEntity ***)(*(int *)(unaff_EBX + 0x6bdcfb) + 0x2c);
  }
  local_30.x = local_3c;
  local_30.y = local_38;
  local_30.z = local_34;
  VectorNormalize(&local_30);
  local_48.x = (vec_t)((uint)ppCVar17 ^ *(uint *)(unaff_EBX + 0x3cac1f));
  local_48.y = local_48.x;
  local_48.z = local_48.x;
  if (*(int *)(*(int *)(unaff_EBX + 0x6bdc1b) + 0x30) != 0) {
    local_90.super_IPartitionEnumerator._vptr_IPartitionEnumerator =
         (_func_int_varargs **)((float)ppCVar17 + *(float *)(&DAT_003c7707 + unaff_EBX));
    local_90.m_pList = ppCVar17;
    local_90.m_listMax = (int)ppCVar17;
    if (bIsFirstTrace) {
                    /* Unresolved local var: Vector norm@[???] */
      local_54.x = local_30.x;
      local_54.y = local_30.y;
      local_54.z = local_30.z;
      VectorNormalize(&local_54);
      NDebugOverlay::BoxDirection(vecStart,&local_48,(Vector *)&local_90,&local_54,0xff,0,0,0,0.1);
    }
    else {
                    /* Unresolved local var: Vector norm@[???] */
      local_60.x = local_30.x;
      local_60.y = local_30.y;
      local_60.z = local_30.z;
      VectorNormalize(&local_60);
      NDebugOverlay::BoxDirection(vecStart,&local_48,(Vector *)&local_90,&local_60,0,0xff,0,0,0.1);
    }
  }
  local_93c = &local_90;
  local_fc.m_Delta.super_Vector.x = vecEnd->x - vecStart->x;
  local_fc.m_Delta.super_Vector.y = vecEnd->y - vecStart->y;
  local_fc.m_Delta.super_Vector.z = vecEnd->z - vecStart->z;
  local_fc.m_pWorldAxisTransform = (matrix3x4_t *)0x0;
  local_fc.m_IsSwept =
       local_fc.m_Delta.super_Vector.x * local_fc.m_Delta.super_Vector.x +
       local_fc.m_Delta.super_Vector.y * local_fc.m_Delta.super_Vector.y +
       local_fc.m_Delta.super_Vector.z * local_fc.m_Delta.super_Vector.z != 0.0;
  local_fc.m_StartOffset.super_Vector.z = *(float *)(unaff_EBX + 0x3c245f);
  local_fc.m_Extents.super_Vector.x =
       ((float)ppCVar17 - local_48.x) * local_fc.m_StartOffset.super_Vector.z;
  local_fc.m_Extents.super_Vector.y =
       ((float)ppCVar17 - local_48.y) * local_fc.m_StartOffset.super_Vector.z;
  local_fc.m_Extents.super_Vector.z =
       ((float)ppCVar17 - local_48.z) * local_fc.m_StartOffset.super_Vector.z;
  local_fc.m_IsRay =
       (double)(local_fc.m_Extents.super_Vector.x * local_fc.m_Extents.super_Vector.x +
                local_fc.m_Extents.super_Vector.y * local_fc.m_Extents.super_Vector.y +
               local_fc.m_Extents.super_Vector.z * local_fc.m_Extents.super_Vector.z) <
       *(double *)(unaff_EBX + 0x3c264f);
  local_fc.m_StartOffset.super_Vector.x =
       ((float)ppCVar17 + local_48.x) * local_fc.m_StartOffset.super_Vector.z;
  local_fc.m_StartOffset.super_Vector.y =
       ((float)ppCVar17 + local_48.y) * local_fc.m_StartOffset.super_Vector.z;
  local_fc.m_StartOffset.super_Vector.z =
       ((float)ppCVar17 + local_48.z) * local_fc.m_StartOffset.super_Vector.z;
  local_fc.m_Start.super_Vector.x = vecStart->x + local_fc.m_StartOffset.super_Vector.x;
  local_fc.m_Start.super_Vector.y = vecStart->y + local_fc.m_StartOffset.super_Vector.y;
  local_fc.m_Start.super_Vector.z = vecStart->z + local_fc.m_StartOffset.super_Vector.z;
  local_fc.m_StartOffset.super_Vector.x = -local_fc.m_StartOffset.super_Vector.x;
  local_fc.m_StartOffset.super_Vector.y = -local_fc.m_StartOffset.super_Vector.y;
  local_fc.m_StartOffset.super_Vector.z = -local_fc.m_StartOffset.super_Vector.z;
                    /* Unresolved local var: CFlaggedEntitiesEnum rayEnum@[???] */
  CFlaggedEntitiesEnum::CFlaggedEntitiesEnum(local_93c,local_8fc,0x200,0x2002080);
  iVar6 = UTIL_EntitiesAlongRay(&local_fc,local_93c);
  local_ac.m_Memory.m_pMemory = (__375 *)0x0;
  local_ac.m_Memory.m_nAllocationCount = 0;
  local_ac.m_Memory.m_nGrowSize = 0;
  local_ac.m_Size = 0;
  local_ac.m_pElements = (__375 *)0x0;
  local_98 = (_func_int_varargs **)0x0;
  local_94 = 0;
  if (0 < iVar6) {
    local_914 = 0;
    pcVar1 = (char *)(unaff_EBX + 0x3a84c3);
    pcVar2 = (char *)(unaff_EBX + 0x356a63);
    do {
      this_00 = (CPortal_Player *)local_8fc[local_914];
      if ((((this_00 != (CPortal_Player *)0x0) &&
           ((((local_7c = pcVar1,
              (this_00->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
              super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
              super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
              super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
              m_iClassname.pszValue == pcVar1 ||
              (bVar4 = CBaseEntity::ClassMatchesComplex((CBaseEntity *)this_00,pcVar1), bVar4)) ||
             (local_7c = pcVar2,
             (this_00->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
             super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
             super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
             m_iClassname.pszValue == pcVar2)) ||
            (bVar4 = CBaseEntity::ClassMatchesComplex((CBaseEntity *)this_00,pcVar2), bVar4)))) ||
          (iVar13 = (*(*(_func_int_varargs ***)
                        &(this_00->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                         super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer
                         .super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.
                         super_CBaseCombatCharacter.super_CBaseFlex)[0x55])(this_00),
          (char)iVar13 != '\0')) &&
         (iVar13 = (*(*(_func_int_varargs ***)
                       &(this_00->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                        super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
                        super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.
                        super_CBaseCombatCharacter.super_CBaseFlex)[0x45])(this_00),
         (char)iVar13 != '\0')) {
        if (((this_00->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
             super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
             super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
             m_iEFlags & 0x800) != 0) {
          CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this_00);
        }
        CalcClosestPointOnLineSegment
                  (&(this_00->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                    super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
                    super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.
                    super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                    super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin,vecStart,vecEnd,
                   (Vector *)local_93c,&local_24);
        if (((this_00->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
             super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
             super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
             m_iEFlags & 0x800) != 0) {
          CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this_00);
        }
                    /* Unresolved local var: Vector res@[???] */
        fVar15 = (float)local_90.super_IPartitionEnumerator._vptr_IPartitionEnumerator -
                 (this_00->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                 super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
                 super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
                 super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                 m_vecAbsOrigin.x;
        fVar16 = (float)local_90.m_pList -
                 (this_00->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                 super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
                 super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
                 super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                 m_vecAbsOrigin.y;
        fVar15 = fVar16 * fVar16 + fVar15 * fVar15;
        if ((fVar15 < *(float *)(unaff_EBX + 0x3c36db) || fVar15 == *(float *)(unaff_EBX + 0x3c36db)
            ) || (this->m_bFromReflectedCube != false)) {
          iVar13 = (*(*(_func_int_varargs ***)
                       &(this_00->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                        super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
                        super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.
                        super_CBaseCombatCharacter.super_CBaseFlex)[0x55])(this_00);
          if ((char)iVar13 == '\0') {
LAB_0070ac9b:
            fVar15 = local_24;
            iVar13 = local_ac.m_Size;
            if (0.0 < local_24) {
              local_94 = 1;
              CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>::GrowVector
                        (&local_ac,1);
              if (~iVar13 + local_ac.m_Size < 1) {
                iVar14 = iVar13 << 3;
              }
              else {
                iVar14 = iVar13 * 8;
                _V_memmove(local_ac.m_Memory.m_pMemory + iVar13 + 1,
                           local_ac.m_Memory.m_pMemory + iVar13,(~iVar13 + local_ac.m_Size) * 8);
              }
              puVar7 = (undefined4 *)((int)&(local_ac.m_Memory.m_pMemory)->pVictim + iVar14);
              if (puVar7 != (undefined4 *)0x0) {
                puVar7[1] = fVar15;
                *puVar7 = this_00;
              }
            }
          }
          else {
                    /* Unresolved local var: float flZDist@[???] */
            if ((*(int *)(*(int *)(unaff_EBX + 0x6bda9b) + 0x30) != 0) &&
               ((pCVar8 = CBaseEntity::GetGroundEntity((CBaseEntity *)this_00),
                pCVar8 != (CBaseEntity *)0x0 ||
                (iVar13 = (*(*(_func_int_varargs ***)
                              &(this_00->
                               super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                               super_CPaintableEntity<CBaseMultiplayerPlayer>.
                               super_CBaseMultiplayerPlayer.super_CAI_ExpresserHost<CBasePlayer>.
                               super_CBasePlayer.super_CBaseCombatCharacter.super_CBaseFlex)[0x9d])
                                    (this_00),
                *(float *)(iVar13 + 8) - (float)local_90.m_listMax < 0.0)))) {
              iVar13 = (*(*(_func_int_varargs ***)
                           &(this_00->
                            super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                            super_CPaintableEntity<CBaseMultiplayerPlayer>.
                            super_CBaseMultiplayerPlayer.super_CAI_ExpresserHost<CBasePlayer>.
                            super_CBasePlayer.super_CBaseCombatCharacter.super_CBaseFlex)[0x55])
                                 (this_00);
              this_01 = (CPortal_Player *)0x0;
              if ((char)iVar13 != '\0') {
                this_01 = this_00;
              }
              if (((this_01->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                   super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
                   super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter
                   .super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                   super_CBaseEntity.m_iEFlags & 0x800) != 0) {
                CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this_01);
              }
              fVar16 = (this_01->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                       super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
                       super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.
                       super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                       super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z;
              pVVar9 = CPortal_Player::GetHullMaxs(this_01);
              if ((float)local_90.m_listMax <= fVar16 + pVVar9->z) {
                if (((this_01->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                     super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
                     super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.
                     super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                     super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
                  CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this_01);
                }
                if (((this_01->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                     super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
                     super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.
                     super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                     super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z <=
                     (float)local_90.m_listMax) &&
                   (fVar15 < *(float *)(unaff_EBX + 0x3c36db) ||
                    fVar15 == *(float *)(unaff_EBX + 0x3c36db))) goto LAB_0070ac9b;
              }
            }
          }
        }
      }
      local_914 = local_914 + 1;
    } while (local_914 != iVar6);
  }
  local_94 = 0;
  if (1 < local_ac.m_Size) {
    local_90.super_IPartitionEnumerator._vptr_IPartitionEnumerator = local_98;
    local_90.m_pList = local_1d;
    **(undefined4 **)(unaff_EBX + 0x53c8a3) = local_93c;
    _qsort(local_ac.m_Memory.m_pMemory,local_ac.m_Size,8,*(int **)(unaff_EBX + 0x6588ff));
  }
  if (0 < local_ac.m_Size) {
    bVar4 = false;
    local_90c = 0;
    pcVar1 = (char *)(unaff_EBX + 0x3a84c3);
    pcVar2 = (char *)(unaff_EBX + 0x356a63);
    do {
      local_91c = local_ac.m_Memory.m_pMemory[local_90c].pVictim;
      uVar3 = (pPVar11->sortedEntList).m_Size;
      CUtlVector<CBaseEntity*,CUtlMemory<CBaseEntity*,_int>_>::GrowVector(&pPVar11->sortedEntList,1)
      ;
      iVar6 = ~uVar3 + (pPVar11->sortedEntList).m_Size;
      if (0 < iVar6) {
        ppCVar17 = (pPVar11->sortedEntList).m_Memory.m_pMemory;
        _V_memmove(ppCVar17 + uVar3 + 1,ppCVar17 + uVar3,iVar6 * 4);
      }
      ppCVar17 = (pPVar11->sortedEntList).m_Memory.m_pMemory + uVar3;
      if (ppCVar17 != (CBaseEntity **)0x0) {
        *ppCVar17 = local_91c;
      }
      if (local_91c != (CBaseEntity *)0x0) {
        local_7c = pcVar1;
        if ((((local_91c->m_iClassname).pszValue != pcVar1) &&
            (bVar5 = CBaseEntity::ClassMatchesComplex(local_91c,pcVar1), !bVar5)) || (bVar4)) {
          local_7c = pcVar2;
          if (((local_91c->m_iClassname).pszValue == pcVar2) ||
             (bVar5 = CBaseEntity::ClassMatchesComplex(local_91c,pcVar2), bVar5)) {
            bVar4 = true;
          }
        }
        else {
          piVar10 = (int *)___dynamic_cast(local_91c,*(undefined4 *)(unaff_EBX + 0x53c4ef),
                                           *(undefined4 *)(unaff_EBX + 0x53cfeb),0);
          if ((piVar10 != (int *)0x0) && (*(char *)((int)piVar10 + 0x3f1) != '\0')) {
            if (pVecOut != (Vector *)0x0) {
              pvVar12 = (vec_t *)(**(code **)(*piVar10 + 0x274))(piVar10);
              pVecOut->x = *pvVar12;
              pVecOut->y = pvVar12[1];
              pVecOut->z = pvVar12[2];
            }
            goto LAB_0070b050;
          }
        }
      }
      local_90c = local_90c + 1;
    } while (local_90c < local_ac.m_Size);
  }
  local_91c = (CBaseEntity *)0x0;
  if (pVecOut != (Vector *)0x0) {
    pVecOut->x = vecEnd->x;
    pVecOut->y = vecEnd->y;
    pVecOut->z = vecEnd->z;
    local_91c = (CBaseEntity *)0x0;
  }
LAB_0070b050:
  local_ac.m_Size = 0;
  if (-1 < local_ac.m_Memory.m_nGrowSize) {
    if (local_ac.m_Memory.m_pMemory != (__375 *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53c4bb) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x53c4bb),local_ac.m_Memory.m_pMemory);
      local_ac.m_Memory.m_pMemory = (__375 *)0x0;
    }
    local_ac.m_Memory.m_nAllocationCount = 0;
  }
  local_ac.m_pElements = local_ac.m_Memory.m_pMemory;
  if ((-1 < local_ac.m_Memory.m_nGrowSize) && (local_ac.m_Memory.m_pMemory != (__375 *)0x0)) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53c4bb) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x53c4bb),local_ac.m_Memory.m_pMemory);
  }
  return local_91c;
}


/* CPortalLaser::ShouldAutoAim at 0070a530 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: bool ShouldAutoAim(CPortalLaser * this, CBaseEntity * pEntity) */

bool __thiscall CPortalLaser::ShouldAutoAim(CPortalLaser *this,CBaseEntity *pEntity)

{
  bool bVar1;
  int iVar2;
  bool bVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (pEntity == (CBaseEntity *)0x0) {
    return false;
  }
  if (((pEntity->m_iClassname).pszValue != (char *)(unaff_EBX + 0x3a8627)) &&
     (bVar1 = CBaseEntity::ClassMatchesComplex(pEntity,(char *)(unaff_EBX + 0x3a8627)), !bVar1)) {
    return false;
  }
  iVar2 = ___dynamic_cast(pEntity,*(undefined4 *)(unaff_EBX + 0x53c653),
                          *(undefined4 *)(unaff_EBX + 0x53d14f),0);
  bVar1 = false;
  if (iVar2 != 0) {
    bVar1 = *(char *)(iVar2 + 0x3f1) != '\0';
  }
  bVar3 = bVar1;
  if ((this->m_bFromReflectedCube != false) &&
     (bVar3 = false, *(int *)(*(int *)(unaff_EBX + 0x6bdddf) + 0x30) != 0)) {
    bVar3 = bVar1;
  }
  return bVar3;
}


/* CPortalLaser::TraceLaser at 0070b4a0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: CBaseEntity * TraceLaser(CPortalLaser * this, bool bIsFirstTrace,
   Vector * vecStart, Vector * vecDirection, float * flTotalBeamLength, trace_t * tr,
   CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t,_int>_> *
   infoList, Vector * pVecAutoAimOffset) */

CBaseEntity * __thiscall
CPortalLaser::TraceLaser
          (CPortalLaser *this,bool bIsFirstTrace,Vector *vecStart,Vector *vecDirection,
          float *flTotalBeamLength,trace_t *tr,
          CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t,_int>_>
          *infoList,Vector *pVecAutoAimOffset)

{
  CPortal_Base2D *pchClassname;
  VMatrix matThisToLinked;
  VMatrix matThisToLinked_00;
  VMatrix matThisToLinked_01;
  int iVar1;
  bool bVar2;
  int iVar3;
  IHandleEntity *passedict;
  uint uVar4;
  Vector *pVVar5;
  float *pfVar6;
  int iVar7;
  int unaff_EBX;
  VMatrix *pVVar8;
  undefined4 *puVar9;
  vec_t *pvVar10;
  undefined4 uVar11;
  undefined4 *puVar12;
  float fVar13;
  float fVar14;
  float fVar15;
  Vector *pVVar16;
  CBaseEntity *pCVar17;
  trace_t *ptVar18;
  undefined4 uVar19;
  Vector *pVVar20;
  undefined4 uVar21;
  CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t,_int>_>
  *pCVar22;
  undefined4 in_stack_fffffe0c;
  undefined1 in_stack_fffffe10 [36];
  Ray_t *ray;
  Vector *pVVar23;
  Ray_t *rayTransformed;
  Vector *ptTransformed;
  CBaseEntity *local_1a4;
  int local_1a0;
  int local_198;
  int local_194;
  undefined4 *local_190;
  Ray_t local_18c;
  Ray_t local_13c;
  CTraceFilterSimpleClassnameList local_e0;
  float local_bc;
  undefined4 uStack_b8;
  undefined4 uStack_b4;
  undefined4 uStack_b0;
  CTraceFilterSimple local_ac;
  Vector local_9c;
  float local_90;
  float local_8c;
  float local_88;
  float local_84;
  float local_80;
  float local_7c;
  Vector local_78;
  float local_6c;
  float local_68;
  float local_64;
  float local_60;
  float local_5c;
  float local_58;
  float local_54;
  float local_50;
  float local_4c;
  CTraceFilterChain local_48;
  Vector local_3c;
  Vector local_30;
  CPortal_Base2D *local_24;
  IServerUnknown local_20;
  
                    /* Unresolved local var: Vector vStart@[???]
                       Unresolved local var: Vector vDir@[???]
                       Unresolved local var: CTraceFilterSimpleClassnameList traceFilter@[???]
                       Unresolved local var: CTraceFilterSimple parentFilter@[???]
                       Unresolved local var: CTraceFilterChain traceChain@[???]
                       Unresolved local var:
                       CUtlVector<CPortal_Base2D*,CUtlMemory<CPortal_Base2D*,_int>_>
                       portalList@[???]
                       Unresolved local var: CBaseEntity * pHitEntity@[???]
                       Unresolved local var: bool bGoingThroughPortal@[???]
                       Unresolved local var: bool bDebug@[???] */
  ___i686_get_pc_thunk_bx();
  *flTotalBeamLength = 0.0;
  local_30.x = vecStart->x;
  local_30.y = vecStart->y;
  local_30.z = vecStart->z;
  local_3c.x = vecDirection->x;
  local_3c.y = vecDirection->y;
  local_3c.z = vecDirection->z;
  CTraceFilterSimpleClassnameList::CTraceFilterSimpleClassnameList
            (&local_e0,(IHandleEntity *)this,0);
  CTraceFilterSimpleClassnameList::AddClassnameToIgnore(&local_e0,(char *)(unaff_EBX + 0x350f73));
  CTraceFilterSimpleClassnameList::AddClassnameToIgnore(&local_e0,(char *)(unaff_EBX + 0x34deff));
  pchClassname = (CPortal_Base2D *)(unaff_EBX + 0x3a76b3);
  CTraceFilterSimpleClassnameList::AddClassnameToIgnore(&local_e0,(char *)pchClassname);
  uVar4 = (this->super_CBaseAnimating).super_CBaseEntity.m_pParent.super_CBaseHandle.m_Index;
  if ((uVar4 == 0xffffffff) ||
     (iVar3 = (uVar4 & 0xffff) * 0x10 + **(int **)(&DAT_0053b6bb + unaff_EBX),
     *(uint *)(iVar3 + 8) != uVar4 >> 0x10)) {
    passedict = (IHandleEntity *)0x0;
  }
  else {
    passedict = *(IHandleEntity **)(iVar3 + 4);
  }
  CTraceFilterSimple::CTraceFilterSimple(&local_ac,passedict,0,(ShouldHitFunc_t)0x0);
  CTraceFilterChain::CTraceFilterChain
            (&local_48,(ITraceFilter *)&local_e0,(ITraceFilter *)&local_ac);
  iVar3 = *(int *)(*(int *)(unaff_EBX + 0x6bce0b) + 0x30);
  local_198 = 0;
  local_194 = 0;
  local_190 = (undefined4 *)0x0;
  while( true ) {
    UTIL_ClearTrace(tr);
                    /* Unresolved local var: Vector res@[???] */
    pfVar6 = (float *)(unaff_EBX + 0x3c23d7);
    local_54 = *pfVar6 * local_3c.x;
    local_50 = *pfVar6 * local_3c.y;
    local_4c = *pfVar6 * local_3c.z;
                    /* Unresolved local var: Vector res@[???] */
    local_60 = local_30.x + local_54;
    local_5c = local_30.y + local_50;
    local_58 = local_30.z + local_4c;
    local_13c.m_Delta.super_Vector.x = local_60 - local_30.x;
    local_13c.m_Delta.super_Vector.y = local_5c - local_30.y;
    local_13c.m_Delta.super_Vector.z = local_58 - local_30.z;
    local_13c.m_IsSwept =
         local_13c.m_Delta.super_Vector.x * local_13c.m_Delta.super_Vector.x +
         local_13c.m_Delta.super_Vector.y * local_13c.m_Delta.super_Vector.y +
         local_13c.m_Delta.super_Vector.z * local_13c.m_Delta.super_Vector.z != 0.0;
    local_13c.m_Extents.super_Vector.z = 0.0;
    local_13c.m_Extents.super_Vector.y = 0.0;
    local_13c.m_Extents.super_Vector.x = 0.0;
    local_13c.m_pWorldAxisTransform = (matrix3x4_t *)0x0;
    local_13c.m_IsRay = true;
    local_13c.m_StartOffset.super_Vector.z = 0.0;
    local_13c.m_StartOffset.super_Vector.y = 0.0;
    local_13c.m_StartOffset.super_Vector.x = 0.0;
    local_13c.m_Start.super_Vector.x = local_30.x;
    local_13c.m_Start.super_Vector.y = local_30.y;
    local_13c.m_Start.super_Vector.z = local_30.z;
    ray = &local_13c;
    (**(code **)(*(int *)**(undefined4 **)(&DAT_0053b71f + unaff_EBX) + 0x14))();
    pVVar23 = &(tr->super_CBaseTrace).endpos;
    if (*(int *)(*(int *)(*(int *)(&DAT_0053b71b + unaff_EBX) + 0x1c) + 0x30) != 0) {
      in_stack_fffffe0c = 0xbf800000;
      DebugDrawLine((Vector *)tr,pVVar23,0xff,0,0,true,-1.0);
    }
    if (iVar3 != 0) {
      if (pVecAutoAimOffset == (Vector *)0x0) {
        in_stack_fffffe0c = 0x3dcccccd;
        NDebugOverlay::Line((Vector *)tr,pVVar23,0,0,0xff,true,0.1);
      }
      else {
        in_stack_fffffe0c = 0x3dcccccd;
        NDebugOverlay::Line((Vector *)tr,pVVar23,0xff,0xff,0,true,0.1);
      }
    }
    UpdateSoundPosition(this,(Vector *)tr,pVVar23);
    local_24 = (CPortal_Base2D *)0x0;
    bVar2 = UTIL_DidTraceTouchPortals(ray,tr,&local_24,(CPortal_Base2D **)0x0);
    if (((!bVar2) || (local_24 == (CPortal_Base2D *)0x0)) ||
       (bVar2 = CPortal_Base2D::IsActivedAndLinked(local_24), !bVar2)) break;
    uVar4 = (uint)bIsFirstTrace;
    pVVar16 = &local_9c;
    local_1a4 = GetEntitiesAlongLaser(this,(Vector *)tr,pVVar23,pVVar16,infoList,bIsFirstTrace);
    pCVar22 = (CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t,_int>_>
               *)0x0;
    pVVar5 = &local_78;
    ptVar18 = tr;
    pVVar20 = pVVar23;
    CalcClosestPointOnLineSegment(pVVar16,(Vector *)tr,pVVar23,pVVar5,(float *)0x0);
                    /* Unresolved local var: Vector res@[???] */
    fVar15 = local_78.x - (tr->super_CBaseTrace).startpos.x;
    fVar13 = local_78.y - (tr->super_CBaseTrace).startpos.y;
    fVar14 = local_78.z - (tr->super_CBaseTrace).startpos.z;
                    /* Unresolved local var: __m128 root@[???] */
    local_bc = SQRT(fVar15 * fVar15 + fVar13 * fVar13 + fVar14 * fVar14);
    uStack_b8 = 0;
    uStack_b4 = 0;
    uStack_b0 = 0;
    *flTotalBeamLength = *flTotalBeamLength + local_bc;
    if (local_1a4 != (CBaseEntity *)0x0) {
      if (pVecAutoAimOffset != (Vector *)0x0) {
        pfVar6 = (float *)(*(local_1a4->super_IServerEntity).super_IServerUnknown.
                            super_IHandleEntity._vptr_IHandleEntity[0x9d])();
                    /* Unresolved local var: Vector res@[???] */
        local_6c = *pfVar6 - local_78.x;
        local_68 = pfVar6[1] - local_78.y;
        local_64 = pfVar6[2] - local_78.z;
        pVecAutoAimOffset->x = local_6c;
        pVecAutoAimOffset->y = local_68;
        pVecAutoAimOffset->z = local_64;
      }
LAB_0070bd6d:
      if ((local_1a4 != (CBaseEntity *)0x0) &&
         ((local_24 = pchClassname,
          (CPortal_Base2D *)(local_1a4->m_iClassname).pszValue == pchClassname ||
          (bVar2 = CBaseEntity::ClassMatchesComplex(local_1a4,(char *)pchClassname), bVar2)))) {
        uVar21 = 0;
        uVar11 = *(undefined4 *)(unaff_EBX + 0x53c1db);
        uVar19 = *(undefined4 *)(unaff_EBX + 0x53b6df);
        pCVar17 = local_1a4;
        iVar3 = ___dynamic_cast();
                    /* Unresolved local var: CPortalLaserTarget * pLaserTarget@[???]
                       Unresolved local var: bool bShouldAutoAimTarget@[???] */
        bVar2 = false;
        if (iVar3 != 0) {
          bVar2 = *(char *)(iVar3 + 0x3f1) != '\0';
        }
        if ((((this->m_bFromReflectedCube == false) ||
             (*(int *)(*(int *)(unaff_EBX + 0x6bce6b) + 0x30) != 0)) && (bVar2)) &&
           (pVecAutoAimOffset != (Vector *)0x0)) {
                    /* Unresolved local var: Vector vAutoAimOffset@[???] */
          local_9c.x = pVecAutoAimOffset->x;
          local_9c.y = pVecAutoAimOffset->y;
          local_9c.z = pVecAutoAimOffset->z;
                    /* Unresolved local var: int i@[???] */
          if (0 < local_198) {
            local_1a0 = 0;
            do {
                    /* Unresolved local var: CPortal_Base2D * pPortal@[???] */
              puVar9 = (undefined4 *)(local_190[local_1a0] + 0x528);
              puVar12 = (undefined4 *)&stack0xfffffdf4;
              pVVar23 = &local_9c;
              pVVar5 = &local_9c;
              for (iVar3 = 0x10; iVar3 != 0; iVar3 = iVar3 + -1) {
                *puVar12 = *puVar9;
                puVar9 = puVar9 + 1;
                puVar12 = puVar12 + 1;
              }
              matThisToLinked_01.m[0][1] = (vec_t)uVar19;
              matThisToLinked_01.m[0][0] = (vec_t)pCVar17;
              matThisToLinked_01.m[0][2] = (vec_t)uVar11;
              matThisToLinked_01.m[0][3] = (vec_t)uVar21;
              matThisToLinked_01.m[1][0] = (vec_t)pCVar22;
              matThisToLinked_01.m[1][1] = (vec_t)uVar4;
              matThisToLinked_01.m[1][2] = (vec_t)in_stack_fffffe0c;
              matThisToLinked_01.m[1][3] = (vec_t)in_stack_fffffe10._0_4_;
              matThisToLinked_01.m[2][0] = (vec_t)in_stack_fffffe10._4_4_;
              matThisToLinked_01.m[2][1] = (vec_t)in_stack_fffffe10._8_4_;
              matThisToLinked_01.m[2][2] = (vec_t)in_stack_fffffe10._12_4_;
              matThisToLinked_01.m[2][3] = (vec_t)in_stack_fffffe10._16_4_;
              matThisToLinked_01.m[3][0] = (vec_t)in_stack_fffffe10._20_4_;
              matThisToLinked_01.m[3][1] = (vec_t)in_stack_fffffe10._24_4_;
              matThisToLinked_01.m[3][2] = (vec_t)in_stack_fffffe10._28_4_;
              matThisToLinked_01.m[3][3] = (vec_t)in_stack_fffffe10._32_4_;
              UTIL_Portal_VectorTransform(matThisToLinked_01,pVVar23,pVVar5);
              local_1a0 = local_1a0 + 1;
            } while (local_1a0 < local_198);
          }
          pVecAutoAimOffset->x = local_9c.x;
          pVecAutoAimOffset->y = local_9c.y;
          pVecAutoAimOffset->z = local_9c.z;
        }
      }
LAB_0070b910:
      if (local_190 != (undefined4 *)0x0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53b6ab) + 8))();
      }
      local_e0.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter =
           (ITraceFilter)(*(int *)(&DAT_0053b8d7 + unaff_EBX) + 8);
      local_e0.m_PassClassnames.m_Size = 0;
      if (-1 < local_e0.m_PassClassnames.m_Memory.m_nGrowSize) {
        if (local_e0.m_PassClassnames.m_Memory.m_pMemory != (char **)0x0) {
          (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53b6ab) + 8))();
          local_e0.m_PassClassnames.m_Memory.m_pMemory = (char **)0x0;
        }
        local_e0.m_PassClassnames.m_Memory.m_nAllocationCount = 0;
      }
      if ((-1 < local_e0.m_PassClassnames.m_Memory.m_nGrowSize) &&
         (local_e0.m_PassClassnames.m_Memory.m_pMemory != (char **)0x0)) {
        local_e0.m_PassClassnames.m_pElements = local_e0.m_PassClassnames.m_Memory.m_pMemory;
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53b6ab) + 8))();
      }
      return local_1a4;
    }
    local_18c.m_pWorldAxisTransform = (matrix3x4_t *)0x0;
    rayTransformed = &local_18c;
    pVVar8 = &local_24->m_matrixThisToLinked;
    pvVar10 = (vec_t *)&stack0xfffffdf4;
    for (iVar7 = 0x10; iVar7 != 0; iVar7 = iVar7 + -1) {
      *pvVar10 = pVVar8->m[0][0];
      pVVar8 = (VMatrix *)(pVVar8->m[0] + 1);
      pvVar10 = pvVar10 + 1;
    }
    matThisToLinked.m[0][1] = (vec_t)ptVar18;
    matThisToLinked.m[0][0] = (vec_t)pVVar16;
    matThisToLinked.m[0][2] = (vec_t)pVVar20;
    matThisToLinked.m[0][3] = (vec_t)pVVar5;
    matThisToLinked.m[1][0] = (vec_t)pCVar22;
    matThisToLinked.m[1][1] = (vec_t)uVar4;
    matThisToLinked.m[1][2] = (vec_t)in_stack_fffffe0c;
    matThisToLinked.m[1][3] = (vec_t)in_stack_fffffe10._0_4_;
    matThisToLinked.m[2][0] = (vec_t)in_stack_fffffe10._4_4_;
    matThisToLinked.m[2][1] = (vec_t)in_stack_fffffe10._8_4_;
    matThisToLinked.m[2][2] = (vec_t)in_stack_fffffe10._12_4_;
    matThisToLinked.m[2][3] = (vec_t)in_stack_fffffe10._16_4_;
    matThisToLinked.m[3][0] = (vec_t)in_stack_fffffe10._20_4_;
    matThisToLinked.m[3][1] = (vec_t)in_stack_fffffe10._24_4_;
    matThisToLinked.m[3][2] = (vec_t)in_stack_fffffe10._28_4_;
    matThisToLinked.m[3][3] = (vec_t)in_stack_fffffe10._32_4_;
    UTIL_Portal_RayTransform(matThisToLinked,ray,rayTransformed);
    local_3c.x = local_18c.m_Delta.super_Vector.x;
    local_3c.y = local_18c.m_Delta.super_Vector.y;
    local_3c.z = local_18c.m_Delta.super_Vector.z;
    pVVar16 = &local_3c;
    VectorNormalize(pVVar16);
    ptTransformed = &local_30;
    pVVar8 = &local_24->m_matrixThisToLinked;
    pvVar10 = (vec_t *)&stack0xfffffdf4;
    for (iVar7 = 0x10; iVar7 != 0; iVar7 = iVar7 + -1) {
      *pvVar10 = pVVar8->m[0][0];
      pVVar8 = (VMatrix *)(pVVar8->m[0] + 1);
      pvVar10 = pvVar10 + 1;
    }
    matThisToLinked_00.m[0][1] = (vec_t)ptVar18;
    matThisToLinked_00.m[0][0] = (vec_t)pVVar16;
    matThisToLinked_00.m[0][2] = (vec_t)pVVar20;
    matThisToLinked_00.m[0][3] = (vec_t)pVVar5;
    matThisToLinked_00.m[1][0] = (vec_t)pCVar22;
    matThisToLinked_00.m[1][1] = (vec_t)uVar4;
    matThisToLinked_00.m[1][2] = (vec_t)in_stack_fffffe0c;
    matThisToLinked_00.m[1][3] = (vec_t)in_stack_fffffe10._0_4_;
    matThisToLinked_00.m[2][0] = (vec_t)in_stack_fffffe10._4_4_;
    matThisToLinked_00.m[2][1] = (vec_t)in_stack_fffffe10._8_4_;
    matThisToLinked_00.m[2][2] = (vec_t)in_stack_fffffe10._12_4_;
    matThisToLinked_00.m[2][3] = (vec_t)in_stack_fffffe10._16_4_;
    matThisToLinked_00.m[3][0] = (vec_t)in_stack_fffffe10._20_4_;
    matThisToLinked_00.m[3][1] = (vec_t)in_stack_fffffe10._24_4_;
    matThisToLinked_00.m[3][2] = (vec_t)in_stack_fffffe10._28_4_;
    matThisToLinked_00.m[3][3] = (vec_t)in_stack_fffffe10._32_4_;
    UTIL_Portal_PointTransform(matThisToLinked_00,pVVar23,ptTransformed);
    uVar4 = (local_24->m_hLinkedPortal).
            super_CNetworkVarBase<CBaseHandle,CPortal_Base2D::NetworkVar_m_hLinkedPortal>.m_Value.
            m_Index;
    if ((uVar4 == 0xffffffff) ||
       (iVar7 = (uVar4 & 0xffff) * 0x10 + **(int **)(&DAT_0053b6bb + unaff_EBX),
       *(uint *)(iVar7 + 8) != uVar4 >> 0x10)) {
      uVar11 = 0;
    }
    else {
      uVar11 = *(undefined4 *)(iVar7 + 4);
    }
    iVar7 = local_198 + 1;
    if (local_194 < iVar7) {
      if ((local_194 != 0) || (local_194 = 8, iVar1 = local_194, 8 < iVar7)) {
        do {
          iVar1 = local_194 << 1;
          if (iVar7 <= local_194 << 1) break;
          local_194 = local_194 << 2;
          iVar1 = local_194;
        } while (local_194 < iVar7);
      }
      local_194 = iVar1;
      if (local_190 == (undefined4 *)0x0) {
        local_190 = (undefined4 *)
                    (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x53b6ab))();
      }
      else {
        local_190 = (undefined4 *)
                    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53b6ab) + 4))();
      }
    }
    if (local_198 != 0) {
      _V_memmove(local_190 + 1,local_190,local_198 * 4);
    }
    local_198 = iVar7;
    if (local_190 != (undefined4 *)0x0) {
      *local_190 = uVar11;
    }
  }
  uVar4 = (uint)bIsFirstTrace;
  local_1a4 = GetEntitiesAlongLaser(this,(Vector *)tr,pVVar23,&local_78,infoList,bIsFirstTrace);
  if (local_1a4 != (CBaseEntity *)0x0) {
                    /* Unresolved local var: Vector vPointOnLine@[???] */
    pCVar22 = (CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t,_int>_>
               *)0x0;
    CalcClosestPointOnLineSegment(&local_78,(Vector *)tr,pVVar23,&local_9c,(float *)0x0);
                    /* Unresolved local var: Vector res@[???] */
    fVar15 = local_9c.x - (tr->super_CBaseTrace).startpos.x;
    fVar13 = local_9c.y - (tr->super_CBaseTrace).startpos.y;
    fVar14 = local_9c.z - (tr->super_CBaseTrace).startpos.z;
                    /* Unresolved local var: __m128 root@[???] */
    local_bc = SQRT(fVar15 * fVar15 + fVar13 * fVar13 + fVar14 * fVar14);
    uStack_b8 = 0;
    uStack_b4 = 0;
    uStack_b0 = 0;
    *flTotalBeamLength = *flTotalBeamLength + local_bc;
    if (pVecAutoAimOffset != (Vector *)0x0) {
      pfVar6 = (float *)(*(local_1a4->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                          _vptr_IHandleEntity[0x9d])();
                    /* Unresolved local var: Vector res@[???] */
      local_84 = *pfVar6 - local_9c.x;
      local_80 = pfVar6[1] - local_9c.y;
      local_7c = pfVar6[2] - local_9c.z;
      pVecAutoAimOffset->x = local_84;
      pVecAutoAimOffset->y = local_80;
      pVecAutoAimOffset->z = local_7c;
    }
    goto LAB_0070bd6d;
  }
                    /* Unresolved local var: Vector res@[???] */
  fVar15 = (tr->super_CBaseTrace).endpos.x - (tr->super_CBaseTrace).startpos.x;
  fVar13 = (tr->super_CBaseTrace).endpos.y - (tr->super_CBaseTrace).startpos.y;
  fVar14 = (tr->super_CBaseTrace).endpos.z - (tr->super_CBaseTrace).startpos.z;
                    /* Unresolved local var: __m128 root@[???] */
  local_bc = SQRT(fVar15 * fVar15 + fVar13 * fVar13 + fVar14 * fVar14);
  uStack_b8 = 0;
  uStack_b4 = 0;
  uStack_b0 = 0;
  *flTotalBeamLength = *flTotalBeamLength + local_bc;
  pCVar17 = tr->m_pEnt;
  if ((pCVar17 == (CBaseEntity *)0x0) ||
     ((pCVar22 = infoList, local_24 = pchClassname,
      (CPortal_Base2D *)(pCVar17->m_iClassname).pszValue != pchClassname &&
      (bVar2 = CBaseEntity::ClassMatchesComplex(pCVar17,(char *)pchClassname), pCVar22 = infoList,
      !bVar2)))) goto LAB_0070b910;
  iVar3 = ___dynamic_cast();
  bVar2 = false;
  if (iVar3 != 0) {
    bVar2 = *(char *)(iVar3 + 0x3f1) != '\0';
  }
  if (((this->m_bFromReflectedCube != false) &&
      (*(int *)(*(int *)(unaff_EBX + 0x6bce6b) + 0x30) == 0)) || (!bVar2)) goto LAB_0070b910;
  bVar2 = CPhysicsShadowClone::IsShadowClone(tr->m_pEnt);
  if (bVar2) {
    CPhysicsShadowClone::GetClonedEntity((CPhysicsShadowClone *)&local_20);
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
    if ((local_20.super_IHandleEntity._vptr_IHandleEntity != (_func_int_varargs **)0xffffffff) &&
       (iVar3 = ((uint)local_20.super_IHandleEntity._vptr_IHandleEntity & 0xffff) * 0x10 +
                **(int **)(&DAT_0053b6bb + unaff_EBX),
       *(uint *)(iVar3 + 8) == (uint)local_20.super_IHandleEntity._vptr_IHandleEntity >> 0x10)) {
      local_1a4 = *(CBaseEntity **)(iVar3 + 4);
    }
  }
  else {
    local_1a4 = tr->m_pEnt;
  }
  if (pVecAutoAimOffset != (Vector *)0x0) {
                    /* Unresolved local var: Vector vPointOnLine@[???] */
    pVVar5 = (Vector *)
             (*(local_1a4->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
               _vptr_IHandleEntity[0x9d])();
    pCVar22 = (CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t,_int>_>
               *)0x0;
    CalcClosestPointOnLine(pVVar5,(Vector *)tr,pVVar23,&local_9c,(float *)0x0);
    pfVar6 = (float *)(*(local_1a4->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                        _vptr_IHandleEntity[0x9d])();
                    /* Unresolved local var: Vector res@[???] */
    local_90 = *pfVar6 - local_9c.x;
    local_8c = pfVar6[1] - local_9c.y;
    local_88 = pfVar6[2] - local_9c.z;
    pVecAutoAimOffset->x = local_90;
    pVecAutoAimOffset->y = local_8c;
    pVecAutoAimOffset->z = local_88;
  }
  goto LAB_0070bd6d;
}


/* CPortalLaser::FireLaser at 0070f3a0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void FireLaser(CPortalLaser * this, Vector * vecStart, Vector *
   vecDirection, CBaseEntity * pParent) */

void __thiscall
CPortalLaser::FireLaser
          (CPortalLaser *this,Vector *vecStart,Vector *vecDirection,CBaseEntity *pParent)

{
  undefined1 *puVar1;
  uint uVar2;
  EHANDLE *pEVar3;
  ushort uVar4;
  undefined4 *puVar5;
  CBaseEdict *pCVar6;
  int *piVar7;
  ushort *puVar8;
  CBeam *pCVar9;
  VMatrix matThisToLinked;
  VMatrix matThisToLinked_00;
  PortalLaserInfo_t *pPVar10;
  vec_t vVar11;
  CTraceFilter CVar12;
  PortalLaserInfo_t *pPVar13;
  bool bVar14;
  CBaseEntity *this_00;
  int iVar15;
  IChangeInfoAccessor *pIVar16;
  CBaseEntity *pCVar17;
  ushort uVar18;
  int iVar19;
  int iVar20;
  uint uVar21;
  uint uVar22;
  int unaff_EBX;
  EHANDLE *pEVar23;
  EHANDLE *pEVar24;
  Vector *pVVar25;
  CBaseHandle *pCVar26;
  CBaseHandle *pCVar27;
  vec_t *pvVar28;
  CPortalLaser *pCVar29;
  Vector *pVVar30;
  CTraceFilterSimpleClassnameList *__in_chrg;
  CPortal_Base2D *pszClassOrWildcard;
  trace_t *trace;
  undefined4 uVar31;
  Vector *pVVar32;
  trace_t *ptVar33;
  undefined4 in_stack_fffffcf8;
  undefined4 in_stack_fffffcfc;
  undefined4 in_stack_fffffd00;
  undefined1 in_stack_fffffd04 [32];
  Ray_t *ray;
  Vector *vecAbsEnd;
  Ray_t *rayTransformed;
  Vector *ptTransformed;
  uint local_2b4;
  uint local_2b0;
  CBeam *local_27c;
  bool local_24d;
  ShouldHitFunc_t local_24c;
  int local_22c;
  trace_t local_200;
  undefined1 local_1ac [40];
  undefined4 local_184;
  float local_180;
  vec_t local_17c;
  undefined4 local_178;
  float local_174;
  char *local_170;
  matrix3x4_t *local_16c;
  int local_168;
  undefined4 local_164;
  ushort uStack_162;
  CPortal_Base2D *local_160;
  int local_15c;
  undefined1 local_14c [64];
  matrix3x4_t *local_10c;
  int local_108;
  short local_104;
  ushort local_102;
  CPortal_Base2D *local_100;
  int local_fc;
  CTraceFilterSimpleClassnameList local_f4;
  CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t,_int>_>
  local_d0;
  Vector local_bc;
  Vector local_b0;
  Vector local_a4;
  float local_98;
  float local_94;
  float local_90;
  float local_8c;
  float local_88;
  float local_84;
  Vector local_80;
  float local_74;
  float local_70;
  float local_6c;
  IHandleEntity local_68;
  CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iObjectCapsCache> local_64;
  _func___thiscall_void_CBaseEntity_ptr *local_60;
  float local_5c;
  _func___thiscall_void_CBaseEntity_ptr *local_58;
  float local_54;
  Vector local_50;
  edict_t *local_44;
  float local_40;
  ushort *local_3c;
  float local_38;
  float local_34;
  float local_30;
  CBaseHandle local_2c;
  CBaseHandle local_28;
  IHandleEntity local_24;
  IHandleEntity local_20;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_pBeam == (CBeam *)0x0) {
    return;
  }
  if (*(int *)(*(int *)(unaff_EBX + 0x6b8deb) + 0x30) == 0) {
    CTraceFilterSimpleClassnameList::CTraceFilterSimpleClassnameList
              (&local_f4,(IHandleEntity *)this,0);
    CTraceFilterSimpleClassnameList::AddClassnameToIgnore(&local_f4,(char *)(unaff_EBX + 0x34d073));
    CTraceFilterSimpleClassnameList::AddClassnameToIgnore(&local_f4,(char *)(unaff_EBX + 0x349fff));
    CTraceFilterSimpleClassnameList::AddClassnameToIgnore(&local_f4,(char *)(unaff_EBX + 0x3a37b3));
                    /* Unresolved local var: Vector res@[???] */
    local_84 = *(float *)(unaff_EBX + 0x3be4d7);
    local_8c = vecDirection->x * local_84;
    local_88 = vecDirection->y * local_84;
    local_84 = local_84 * vecDirection->z;
                    /* Unresolved local var: Vector res@[???] */
    local_1ac._0_4_ = vecStart->x;
    local_98 = (float)local_1ac._0_4_ + local_8c;
    local_1ac._4_4_ = vecStart->y;
    local_94 = (float)local_1ac._4_4_ + local_88;
    local_1ac._8_4_ = vecStart->z;
    local_90 = (float)local_1ac._8_4_ + local_84;
    local_1ac._16_4_ = local_98 - vecStart->x;
    local_1ac._20_4_ = local_94 - vecStart->y;
    local_1ac._24_4_ = local_90 - vecStart->z;
    local_174 = 0.0;
    local_178 = 0.0;
    local_17c = 0.0;
    local_16c = (matrix3x4_t *)0x0;
    local_168 = CONCAT31(CONCAT21(local_168._2_2_,
                                  (float)local_1ac._16_4_ * (float)local_1ac._16_4_ +
                                  (float)local_1ac._20_4_ * (float)local_1ac._20_4_ +
                                  (float)local_1ac._24_4_ * (float)local_1ac._24_4_ != 0.0),1);
    local_184 = 0.0;
    local_1ac._36_4_ = 0.0;
    local_1ac._32_4_ = 0.0;
    trace = &local_200;
    ray = (Ray_t *)local_1ac;
    ptVar33 = trace;
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53781f) + 0x14))();
    vecAbsEnd = &local_200.super_CBaseTrace.endpos;
    if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x53781b) + 0x1c) + 0x30) != 0) {
      in_stack_fffffcfc = 0xbf800000;
      in_stack_fffffcf8 = 1;
      ptVar33 = (trace_t *)0x0;
      DebugDrawLine((Vector *)trace,vecAbsEnd,0xff,0,0,true,-1.0);
    }
    UpdateSoundPosition(this,(Vector *)trace,vecAbsEnd);
    local_28.m_Index = 0;
    bVar14 = UTIL_DidTraceTouchPortals
                       (ray,trace,(CPortal_Base2D **)&local_28,(CPortal_Base2D **)0x0);
    if (((bVar14) && ((CPortal_Base2D *)local_28.m_Index != (CPortal_Base2D *)0x0)) &&
       (bVar14 = CPortal_Base2D::IsActivedAndLinked((CPortal_Base2D *)local_28.m_Index), bVar14)) {
      pVVar32 = &local_a4;
      bVar14 = StrikeEntitiesAlongLaser(this,(Vector *)trace,vecAbsEnd,&local_a4);
      if (bVar14) {
        CBeam::PointsInit(this->m_pBeam,vecStart,&local_a4);
        CBaseEntity::SetAbsOrigin(&this->m_pBeam->super_CBaseEntity,vecStart);
      }
      else {
        CBeam::PointsInit(this->m_pBeam,vecStart,vecAbsEnd);
        CBaseEntity::SetAbsOrigin(&this->m_pBeam->super_CBaseEntity,vecStart);
        uVar31 = 0;
        pCVar29 = this;
        FireAtPoint(this,trace,false);
        local_10c = (matrix3x4_t *)0x0;
        rayTransformed = (Ray_t *)local_14c;
        pVVar25 = (Vector *)(local_28.m_Index + 0x528);
        pvVar28 = (vec_t *)&stack0xfffffce4;
        for (iVar15 = 0x10; iVar15 != 0; iVar15 = iVar15 + -1) {
          *pvVar28 = pVVar25->x;
          pVVar25 = (Vector *)&pVVar25->y;
          pvVar28 = pvVar28 + 1;
        }
        matThisToLinked.m[0][1] = (vec_t)trace;
        matThisToLinked.m[0][0] = (vec_t)pCVar29;
        matThisToLinked.m[0][2] = (vec_t)uVar31;
        matThisToLinked.m[0][3] = (vec_t)pVVar32;
        matThisToLinked.m[1][0] = (vec_t)ptVar33;
        matThisToLinked.m[1][1] = (vec_t)in_stack_fffffcf8;
        matThisToLinked.m[1][2] = (vec_t)in_stack_fffffcfc;
        matThisToLinked.m[1][3] = (vec_t)in_stack_fffffd00;
        matThisToLinked.m[2][0] = (vec_t)in_stack_fffffd04._0_4_;
        matThisToLinked.m[2][1] = (vec_t)in_stack_fffffd04._4_4_;
        matThisToLinked.m[2][2] = (vec_t)in_stack_fffffd04._8_4_;
        matThisToLinked.m[2][3] = (vec_t)in_stack_fffffd04._12_4_;
        matThisToLinked.m[3][0] = (vec_t)in_stack_fffffd04._16_4_;
        matThisToLinked.m[3][1] = (vec_t)in_stack_fffffd04._20_4_;
        matThisToLinked.m[3][2] = (vec_t)in_stack_fffffd04._24_4_;
        matThisToLinked.m[3][3] = (vec_t)in_stack_fffffd04._28_4_;
        UTIL_Portal_RayTransform(matThisToLinked,ray,rayTransformed);
        local_b0.x = (vec_t)local_14c._16_4_;
        local_b0.y = (vec_t)local_14c._20_4_;
        local_b0.z = (vec_t)local_14c._24_4_;
        pVVar30 = &local_b0;
        VectorNormalize(&local_b0);
        pVVar25 = (Vector *)(local_28.m_Index + 0x528);
        pvVar28 = (vec_t *)&stack0xfffffce4;
        ptTransformed = &local_bc;
        for (iVar15 = 0x10; iVar15 != 0; iVar15 = iVar15 + -1) {
          *pvVar28 = pVVar25->x;
          pVVar25 = (Vector *)&pVVar25->y;
          pvVar28 = pvVar28 + 1;
        }
        matThisToLinked_00.m[0][1] = (vec_t)trace;
        matThisToLinked_00.m[0][0] = (vec_t)pVVar30;
        matThisToLinked_00.m[0][2] = (vec_t)uVar31;
        matThisToLinked_00.m[0][3] = (vec_t)pVVar32;
        matThisToLinked_00.m[1][0] = (vec_t)ptVar33;
        matThisToLinked_00.m[1][1] = (vec_t)in_stack_fffffcf8;
        matThisToLinked_00.m[1][2] = (vec_t)in_stack_fffffcfc;
        matThisToLinked_00.m[1][3] = (vec_t)in_stack_fffffd00;
        matThisToLinked_00.m[2][0] = (vec_t)in_stack_fffffd04._0_4_;
        matThisToLinked_00.m[2][1] = (vec_t)in_stack_fffffd04._4_4_;
        matThisToLinked_00.m[2][2] = (vec_t)in_stack_fffffd04._8_4_;
        matThisToLinked_00.m[2][3] = (vec_t)in_stack_fffffd04._12_4_;
        matThisToLinked_00.m[3][0] = (vec_t)in_stack_fffffd04._16_4_;
        matThisToLinked_00.m[3][1] = (vec_t)in_stack_fffffd04._20_4_;
        matThisToLinked_00.m[3][2] = (vec_t)in_stack_fffffd04._24_4_;
        matThisToLinked_00.m[3][3] = (vec_t)in_stack_fffffd04._28_4_;
        UTIL_Portal_PointTransform(matThisToLinked_00,vecAbsEnd,ptTransformed);
        UpdateNextLaser(this,&local_bc,&local_b0,(CBaseEntity *)0x0);
      }
LAB_0070f992:
      local_f4.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter =
           (ITraceFilter)(*(int *)(unaff_EBX + 0x5379d7) + 8);
      local_f4.m_PassClassnames.m_Size = 0;
      if (local_f4.m_PassClassnames.m_Memory.m_nGrowSize < 0) {
LAB_00710187:
        local_f4.m_PassClassnames.m_Size = 0;
        local_f4.m_PassClassnames.m_pElements = local_f4.m_PassClassnames.m_Memory.m_pMemory;
        pPVar10 = (PortalLaserInfo_t *)local_f4.m_PassClassnames.m_Memory.m_pMemory;
        pPVar13 = local_d0.m_pElements;
        goto joined_r0x0070f9fb;
      }
      if ((PortalLaserInfo_t *)local_f4.m_PassClassnames.m_Memory.m_pMemory !=
          (PortalLaserInfo_t *)0x0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5377ab) + 8))();
LAB_0070f9db:
        local_f4.m_PassClassnames.m_Memory.m_pMemory = (char **)0x0;
      }
    }
    else {
      bVar14 = StrikeEntitiesAlongLaser(this,(Vector *)trace,vecAbsEnd,(Vector *)&local_d0);
      if (bVar14) {
        CBeam::PointsInit(this->m_pBeam,vecStart,(Vector *)&local_d0);
        CBaseEntity::SetAbsOrigin(&this->m_pBeam->super_CBaseEntity,vecStart);
        goto LAB_0070f992;
      }
      if (pParent == (CBaseEntity *)0x0) {
        CBeam::PointsInit(this->m_pBeam,vecStart,vecAbsEnd);
        CBaseEntity::SetAbsOrigin(&this->m_pBeam->super_CBaseEntity,vecStart);
      }
      else {
        CBeam::PointEntInit(this->m_pBeam,vecAbsEnd,pParent);
        pCVar9 = this->m_pBeam;
        local_2c.m_Index = (int)&vgui::Panel::GetMessageMap::s_pMap.magic + 1;
        iVar15 = (pCVar9->m_nNumBeamEnts).m_Value;
        bVar14 = iVar15 == 1;
        pEVar3 = (pCVar9->m_hAttachEntity).m_Value + iVar15 + 9;
        iVar19 = 4;
        pEVar23 = pEVar3;
        pCVar26 = &local_2c;
        do {
          pEVar24 = pEVar23;
          pCVar27 = pCVar26;
          if (iVar19 == 0) break;
          iVar19 = iVar19 + -1;
          pCVar27 = (CBaseHandle *)((int)&pCVar26->m_Index + 1);
          pEVar24 = (EHANDLE *)((int)&(pEVar23->super_CBaseHandle).m_Index + 1);
          bVar14 = (char)(pEVar23->super_CBaseHandle).m_Index == (char)pCVar26->m_Index;
          pEVar23 = pEVar24;
          pCVar26 = pCVar27;
        } while (bVar14);
        iVar19 = 0;
        if (!bVar14) {
          iVar19 = (uint)*(byte *)((int)(pEVar24 + -1) + 3) -
                   (uint)*(byte *)((int)&pCVar27[-1].m_Index + 3);
        }
        if (iVar19 != 0) {
          if ((pCVar9->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
            pCVar6 = &((pCVar9->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
            if ((pCVar6 != (CBaseEdict *)0x0) && ((pCVar6->m_fStateFlags & 0x100U) == 0)) {
              pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 1;
              pIVar16 = CBaseEdict::GetChangeAccessor(pCVar6);
              uVar18 = (short)pEVar3 - (short)pCVar9;
              piVar7 = *(int **)(unaff_EBX + 0x5377b7);
              puVar8 = (ushort *)*piVar7;
              if (pIVar16->m_iChangeInfoSerialNumber == *puVar8) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                uVar21 = (uint)pIVar16->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
                uVar4 = puVar8[uVar21 * 0x14 + 0x14];
                if (uVar4 == 0) {
LAB_00710db0:
                  puVar8[(uint)uVar4 + uVar21 * 0x14 + 1] = uVar18;
                  puVar8[uVar21 * 0x14 + 0x14] = uVar4 + 1;
                }
                else if (puVar8[uVar21 * 0x14 + 1] != uVar18) {
                  local_2b0 = 0;
                  do {
                    uVar22 = local_2b0 + 1;
                    local_2b0 = uVar22 & 0xffff;
                    if ((ushort)uVar22 == uVar4) {
                      if (uVar4 != 0x13) goto LAB_00710db0;
                      goto LAB_00710d03;
                    }
                  } while (puVar8[uVar21 * 0x14 + local_2b0 + 1] != uVar18);
                }
              }
              else if (puVar8[0x7d1] == 100) {
LAB_00710d03:
                pIVar16->m_iChangeInfoSerialNumber = 0;
                pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 0x100;
              }
              else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                pIVar16->m_iChangeInfo = puVar8[0x7d1];
                *(short *)(*piVar7 + 0xfa2) = *(short *)(*piVar7 + 0xfa2) + 1;
                pIVar16->m_iChangeInfoSerialNumber = *(ushort *)*piVar7;
                iVar19 = *piVar7 + (uint)pIVar16->m_iChangeInfo * 0x28;
                *(ushort *)(iVar19 + 2) = uVar18;
                *(undefined2 *)(iVar19 + 0x28) = 1;
              }
            }
          }
          else {
            puVar1 = &(pCVar9->super_CBaseEntity).m_Network.field_0x4c;
            *(uint *)puVar1 = *(uint *)puVar1 | 1;
          }
          (pCVar9->m_hAttachEntity).m_Value[iVar15 + 9].super_CBaseHandle.m_Index = local_2c.m_Index
          ;
        }
      }
      if (local_200.m_pEnt != (CBaseEntity *)0x0) {
        bVar14 = CPhysicsShadowClone::IsShadowClone(local_200.m_pEnt);
        if (bVar14) {
          CPhysicsShadowClone::GetClonedEntity((CPhysicsShadowClone *)&local_24);
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
          if (((IHandleEntity)local_24._vptr_IHandleEntity == (IHandleEntity)0xffffffff) ||
             (iVar15 = ((uint)local_24._vptr_IHandleEntity & 0xffff) * 0x10 +
                       **(int **)(unaff_EBX + 0x5377bb),
             *(uint *)(iVar15 + 8) != (uint)local_24._vptr_IHandleEntity >> 0x10)) {
            local_200.m_pEnt = (CBaseEntity *)0x0;
          }
          else {
            local_200.m_pEnt = *(CBaseEntity **)(iVar15 + 4);
          }
        }
        bVar14 = ReflectLaserFromEntity(this,local_200.m_pEnt);
        if (bVar14) goto LAB_0070f992;
      }
      if (this->m_pChildLaser != (CPortalLaser *)0x0) {
        UTIL_Remove((CBaseEntity *)this->m_pChildLaser);
        this->m_pChildLaser = (CPortalLaser *)0x0;
      }
      UTIL_SetOrigin((CBaseEntity *)this->m_pPlacementHelper,vecAbsEnd,false);
      FireAtPoint(this,trace,true);
      local_f4.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter =
           (ITraceFilter)(*(int *)(unaff_EBX + 0x5379d7) + 8);
      local_f4.m_PassClassnames.m_Size = 0;
      if (local_f4.m_PassClassnames.m_Memory.m_nGrowSize < 0) goto LAB_00710187;
      if ((PortalLaserInfo_t *)local_f4.m_PassClassnames.m_Memory.m_pMemory !=
          (PortalLaserInfo_t *)0x0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5377ab) + 8))();
        goto LAB_0070f9db;
      }
    }
    local_f4.m_PassClassnames.m_Memory.m_nAllocationCount = 0;
    local_f4.m_PassClassnames.m_pElements = local_f4.m_PassClassnames.m_Memory.m_pMemory;
    pPVar10 = (PortalLaserInfo_t *)local_f4.m_PassClassnames.m_Memory.m_pMemory;
    pPVar13 = local_d0.m_pElements;
    goto joined_r0x0070f9fb;
  }
  local_b0.x = vecDirection->x;
  local_b0.y = vecDirection->y;
  local_b0.z = vecDirection->z;
  local_d0.m_Memory.m_pMemory = (PortalLaserInfo_t *)0x0;
  local_d0.m_Memory.m_nAllocationCount = 0;
  local_d0.m_Memory.m_nGrowSize = 0;
  local_d0.m_Size = 0;
  local_d0.m_pElements = (PortalLaserInfo_t *)0x0;
  this_00 = TraceLaser(this,true,vecStart,&local_b0,(float *)&local_2c,(trace_t *)local_14c,
                       &local_d0,&local_bc);
  if ((this_00 == (CBaseEntity *)0x0) ||
     ((local_28.m_Index = unaff_EBX + 0x3a37b3,
      (CPortal_Base2D *)(this_00->m_iClassname).pszValue != (CPortal_Base2D *)local_28.m_Index &&
      (bVar14 = CBaseEntity::ClassMatchesComplex(this_00,(char *)local_28.m_Index), !bVar14)))) {
LAB_0070f509:
    local_f4.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter =
         (ITraceFilter)(_func_int_varargs **)0x0;
    local_f4.super_CTraceFilterSimple.m_pPassEnt = (IHandleEntity *)0x0;
    local_f4.super_CTraceFilterSimple.m_collisionGroup = 0;
    local_f4.super_CTraceFilterSimple.m_pExtraShouldHitCheckFunction = (ShouldHitFunc_t)0x0;
    local_f4.m_PassClassnames.m_Memory.m_pMemory = (char **)0x0;
    UTIL_ClearTrace((trace_t *)local_14c);
    TraceLaser(this,false,vecStart,&local_b0,(float *)&local_2c,(trace_t *)local_14c,
               (CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t,_int>_>
                *)&local_f4,(Vector *)0x0);
    __in_chrg = &local_f4;
    DamageEntitiesAlongLaser
              (this,(CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t,_int>_>
                     *)&local_f4,false);
    CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t,_int>_>::
    ~CUtlVector((CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t,_int>_>
                 *)&local_f4,(int)__in_chrg);
    local_24d = false;
  }
  else {
    iVar15 = ___dynamic_cast();
                    /* Unresolved local var: CPortalLaserTarget * pLaserTarget@[???]
                       Unresolved local var: bool bShouldAutoAimTarget@[???] */
    bVar14 = false;
    if (iVar15 != 0) {
      bVar14 = *(char *)(iVar15 + 0x3f1) != '\0';
    }
    if (((this->m_bFromReflectedCube != false) &&
        (*(int *)(*(int *)(unaff_EBX + 0x6b8f6b) + 0x30) == 0)) || (!bVar14)) goto LAB_0070f509;
                    /* Unresolved local var: Vector res@[???] */
    local_38 = (float)local_2c.m_Index * vecDirection->x;
    local_34 = (float)local_2c.m_Index * vecDirection->y;
    local_30 = (float)local_2c.m_Index * vecDirection->z;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
    local_44 = (edict_t *)(vecStart->x + local_38 + local_bc.x);
    local_40 = vecStart->y + local_34 + local_bc.y;
    local_3c = (ushort *)(vecStart->z + local_30 + local_bc.z);
                    /* Unresolved local var: Vector res@[???] */
    local_50.x = (float)local_44 - vecStart->x;
    local_50.y = local_40 - vecStart->y;
    local_50.z = (float)local_3c - vecStart->z;
                    /* Unresolved local var: Vector norm@[???] */
    VectorNormalize(&local_50);
    local_a4.x = local_50.x;
    local_a4.y = local_50.y;
    local_a4.z = local_50.z;
    local_f4.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter =
         (ITraceFilter)(_func_int_varargs **)0x0;
    local_f4.super_CTraceFilterSimple.m_pPassEnt = (IHandleEntity *)0x0;
    local_f4.super_CTraceFilterSimple.m_collisionGroup = 0;
    local_f4.super_CTraceFilterSimple.m_pExtraShouldHitCheckFunction = (ShouldHitFunc_t)0x0;
    local_f4.m_PassClassnames.m_Memory.m_pMemory = (char **)0x0;
    pCVar17 = TraceLaser(this,false,vecStart,&local_a4,(float *)&local_28,(trace_t *)local_1ac,
                         (CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t,_int>_>
                          *)&local_f4,(Vector *)0x0);
    if (*(int *)(*(int *)(unaff_EBX + 0x6b8f0b) + 0x30) != 0) {
      puVar5 = *(undefined4 **)(unaff_EBX + 0x5377cf);
      (**(code **)(*(int *)*puVar5 + 0xbc))();
      (**(code **)(*(int *)*puVar5 + 0xbc))();
    }
    vVar11 = local_184;
    local_24d = this_00 == pCVar17;
    if (local_24d) {
      local_14c._0_4_ = local_1ac._0_4_;
      local_14c._4_4_ = local_1ac._4_4_;
      local_14c._8_4_ = local_1ac._8_4_;
      local_14c._12_4_ = local_1ac._12_4_;
      local_14c._16_4_ = local_1ac._16_4_;
      local_14c._20_4_ = local_1ac._20_4_;
      local_14c._24_4_ = local_1ac._24_4_;
      local_14c._28_4_ = local_1ac._28_4_;
      local_14c._32_4_ = local_1ac._32_4_;
      local_14c._36_4_ = local_1ac._36_4_;
      local_14c[0x28] = (uint8)local_184;
      local_14c[0x29] = local_184._1_1_;
      local_14c[0x2a] = local_184._2_1_;
      local_14c[0x2b] = local_184._3_1_;
      local_14c._44_4_ = local_180;
      local_14c._48_4_ = local_17c;
      local_14c._52_2_ = SUB42(local_178,0);
      local_14c[0x36] = local_178._2_1_;
      local_14c[0x37] = local_178._3_1_;
      local_14c._56_4_ = local_174;
      local_14c._60_4_ = local_170;
      local_10c = local_16c;
      local_108 = local_168;
      local_104 = (short)local_164;
      local_102 = uStack_162;
      local_100 = local_160;
      local_fc = local_15c;
      local_2c.m_Index = local_28.m_Index;
      local_b0.x = local_a4.x;
      local_b0.y = local_a4.y;
      local_b0.z = local_a4.z;
      local_184 = vVar11;
      DamageEntitiesAlongLaser
                (this,(CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t,_int>_>
                       *)&local_f4,true);
    }
    else {
      DamageEntitiesAlongLaser(this,&local_d0,false);
    }
    local_24c = local_f4.super_CTraceFilterSimple.m_pExtraShouldHitCheckFunction;
    puVar5 = *(undefined4 **)(unaff_EBX + 0x5377ab);
    iVar15 = (int)local_f4.super_CTraceFilterSimple.m_pExtraShouldHitCheckFunction * 0x2c;
    while( true ) {
      iVar19 = iVar15;
      CVar12.super_ITraceFilter._vptr_ITraceFilter =
           (ITraceFilter)(ITraceFilter)local_f4.super_CTraceFilterSimple.super_CTraceFilter;
      local_24c = local_24c + -1;
      if ((int)local_24c < 0) break;
      *(undefined4 *)
       ((int)local_f4.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter.
             _vptr_ITraceFilter + iVar19 + -8) = 0;
      if (*(int *)((int)local_f4.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter.
                        _vptr_ITraceFilter + iVar19 + -0xc) < 0) {
        iVar20 = *(int *)((int)local_f4.super_CTraceFilterSimple.super_CTraceFilter.
                               super_ITraceFilter._vptr_ITraceFilter + iVar19 + -0x14);
      }
      else {
        if (*(int *)((int)local_f4.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter.
                          _vptr_ITraceFilter + iVar19 + -0x14) != 0) {
          (**(code **)(*(int *)*puVar5 + 8))();
          *(undefined4 *)((int)CVar12.super_ITraceFilter._vptr_ITraceFilter + iVar19 + -0x14) = 0;
        }
        iVar20 = 0;
        *(undefined4 *)((int)CVar12.super_ITraceFilter._vptr_ITraceFilter + iVar19 + -0x10) = 0;
      }
      *(int *)((int)CVar12.super_ITraceFilter._vptr_ITraceFilter + iVar19 + -4) = iVar20;
      iVar15 = iVar19 + -0x2c;
      if (-1 < *(int *)((int)CVar12.super_ITraceFilter._vptr_ITraceFilter + iVar19 + -0xc)) {
        if (iVar20 != 0) {
          (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5377ab) + 8))();
          *(undefined4 *)((int)CVar12.super_ITraceFilter._vptr_ITraceFilter + iVar19 + -0x14) = 0;
        }
        *(undefined4 *)((int)CVar12.super_ITraceFilter._vptr_ITraceFilter + iVar19 + -0x10) = 0;
      }
    }
    local_f4.super_CTraceFilterSimple.m_pExtraShouldHitCheckFunction = (ShouldHitFunc_t)0x0;
    if (-1 < local_f4.super_CTraceFilterSimple.m_collisionGroup) {
      if (local_f4.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter
          != (_func_int_varargs **)0x0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5377ab) + 8))();
        local_f4.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter =
             (ITraceFilter)(_func_int_varargs **)0x0;
      }
      local_f4.super_CTraceFilterSimple.m_pPassEnt = (IHandleEntity *)0x0;
    }
    local_f4.m_PassClassnames.m_Memory.m_pMemory =
         (char **)local_f4.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter.
                  _vptr_ITraceFilter;
    if (-1 < local_f4.super_CTraceFilterSimple.m_collisionGroup) {
      if (local_f4.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter
          != (_func_int_varargs **)0x0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5377ab) + 8))();
        local_f4.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter =
             (ITraceFilter)(_func_int_varargs **)0x0;
      }
      local_f4.super_CTraceFilterSimple.m_pPassEnt = (IHandleEntity *)0x0;
    }
  }
  if ((this->m_bFromReflectedCube == false) || (pParent == (CBaseEntity *)0x0)) {
                    /* Unresolved local var: Vector res@[???] */
    local_74 = (float)local_2c.m_Index * local_b0.x;
    local_70 = (float)local_2c.m_Index * local_b0.y;
    local_6c = (float)local_2c.m_Index * local_b0.z;
                    /* Unresolved local var: Vector res@[???] */
    local_80.x = vecStart->x + local_74;
    local_80.y = vecStart->y + local_70;
    local_80.z = vecStart->z + local_6c;
    CBeam::PointsInit(this->m_pBeam,vecStart,&local_80);
    CBaseEntity::SetAbsOrigin(&this->m_pBeam->super_CBaseEntity,vecStart);
    local_27c = this->m_pBeam;
  }
  else {
                    /* Unresolved local var: Vector res@[???] */
    local_5c = (float)local_2c.m_Index * local_b0.x;
    local_58 = (_func___thiscall_void_CBaseEntity_ptr *)((float)local_2c.m_Index * local_b0.y);
    local_54 = (float)local_2c.m_Index * local_b0.z;
                    /* Unresolved local var: Vector res@[???] */
    local_68._vptr_IHandleEntity = (_func_int_varargs **)(vecStart->x + local_5c);
    local_64.m_Value = (int)(vecStart->y + (float)local_58);
    local_60 = (_func___thiscall_void_CBaseEntity_ptr *)(vecStart->z + local_54);
    vecStart = (Vector *)&local_68;
    CBeam::PointEntInit(this->m_pBeam,vecStart,pParent);
    local_27c = this->m_pBeam;
    local_28.m_Index = (int)&vgui::Panel::GetMessageMap::s_pMap.magic + 1;
    iVar15 = (local_27c->m_nNumBeamEnts).m_Value;
    bVar14 = iVar15 == 1;
    pEVar3 = (local_27c->m_hAttachEntity).m_Value + iVar15 + 9;
    iVar19 = 4;
    pEVar23 = pEVar3;
    pCVar26 = &local_28;
    do {
      pEVar24 = pEVar23;
      pCVar27 = pCVar26;
      if (iVar19 == 0) break;
      iVar19 = iVar19 + -1;
      pCVar27 = (CBaseHandle *)((int)&pCVar26->m_Index + 1);
      pEVar24 = (EHANDLE *)((int)&(pEVar23->super_CBaseHandle).m_Index + 1);
      bVar14 = (char)(pEVar23->super_CBaseHandle).m_Index == (char)pCVar26->m_Index;
      pEVar23 = pEVar24;
      pCVar26 = pCVar27;
    } while (bVar14);
    iVar19 = 0;
    if (!bVar14) {
      iVar19 = (uint)*(byte *)((int)(pEVar24 + -1) + 3) -
               (uint)*(byte *)((int)&pCVar27[-1].m_Index + 3);
    }
    if (iVar19 != 0) {
      if ((local_27c->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
        pCVar6 = &((local_27c->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
        if ((pCVar6 != (CBaseEdict *)0x0) && ((pCVar6->m_fStateFlags & 0x100U) == 0)) {
          pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 1;
          pIVar16 = CBaseEdict::GetChangeAccessor(pCVar6);
          uVar18 = (short)pEVar3 - (short)local_27c;
          piVar7 = *(int **)(unaff_EBX + 0x5377b7);
          puVar8 = (ushort *)*piVar7;
          if (pIVar16->m_iChangeInfoSerialNumber == *puVar8) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            uVar21 = (uint)pIVar16->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar4 = puVar8[uVar21 * 0x14 + 0x14];
            if (uVar4 == 0) {
LAB_00710937:
              puVar8[(uint)uVar4 + uVar21 * 0x14 + 1] = uVar18;
              puVar8[uVar21 * 0x14 + 0x14] = uVar4 + 1;
            }
            else if (puVar8[uVar21 * 0x14 + 1] != uVar18) {
              local_2b4 = 0;
              do {
                uVar22 = local_2b4 + 1;
                local_2b4 = uVar22 & 0xffff;
                if ((ushort)uVar22 == uVar4) {
                  if (uVar4 != 0x13) goto LAB_00710937;
                  goto LAB_0071088a;
                }
              } while (puVar8[uVar21 * 0x14 + local_2b4 + 1] != uVar18);
            }
          }
          else if (puVar8[0x7d1] == 100) {
LAB_0071088a:
            pIVar16->m_iChangeInfoSerialNumber = 0;
            pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar16->m_iChangeInfo = puVar8[0x7d1];
            *(short *)(*piVar7 + 0xfa2) = *(short *)(*piVar7 + 0xfa2) + 1;
            pIVar16->m_iChangeInfoSerialNumber = *(ushort *)*piVar7;
            iVar19 = *piVar7 + (uint)pIVar16->m_iChangeInfo * 0x28;
            *(ushort *)(iVar19 + 2) = uVar18;
            *(undefined2 *)(iVar19 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(local_27c->super_CBaseEntity).m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (local_27c->m_hAttachEntity).m_Value[iVar15 + 9].super_CBaseHandle.m_Index = local_28.m_Index;
      local_27c = this->m_pBeam;
    }
  }
  if ((local_27c->m_bShouldAutoAim).m_Value != local_24d) {
    if ((local_27c->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar6 = &((local_27c->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar6 != (CBaseEdict *)0x0) && ((pCVar6->m_fStateFlags & 0x100U) == 0)) {
        pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 1;
        pIVar16 = CBaseEdict::GetChangeAccessor(pCVar6);
        piVar7 = *(int **)(unaff_EBX + 0x5377b7);
        puVar8 = (ushort *)*piVar7;
        if (pIVar16->m_iChangeInfoSerialNumber == *puVar8) {
          uVar21 = (uint)pIVar16->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar18 = puVar8[uVar21 * 0x14 + 0x14];
          if (uVar18 == 0) {
LAB_00710421:
            puVar8[(uint)uVar18 + uVar21 * 0x14 + 1] = 0x46a;
            puVar8[uVar21 * 0x14 + 0x14] = uVar18 + 1;
          }
          else if (puVar8[uVar21 * 0x14 + 1] != 0x46a) {
            uVar22 = 0;
            do {
              uVar2 = uVar22 + 1;
              uVar22 = uVar2 & 0xffff;
              if ((ushort)uVar2 == uVar18) {
                if (uVar18 != 0x13) goto LAB_00710421;
                pIVar16->m_iChangeInfoSerialNumber = 0;
                pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar8[uVar21 * 0x14 + uVar22 + 1] != 0x46a);
          }
        }
        else if (puVar8[0x7d1] == 100) {
          pIVar16->m_iChangeInfoSerialNumber = 0;
          pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar16->m_iChangeInfo = puVar8[0x7d1];
          *(short *)(*piVar7 + 0xfa2) = *(short *)(*piVar7 + 0xfa2) + 1;
          pIVar16->m_iChangeInfoSerialNumber = *(ushort *)*piVar7;
          iVar15 = *piVar7 + (uint)pIVar16->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar15 + 2) = 0x46a;
          *(undefined2 *)(iVar15 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(local_27c->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (local_27c->m_bShouldAutoAim).m_Value = local_24d;
  }
  if ((this_00 != (CBaseEntity *)0x0) &&
     ((pszClassOrWildcard = (CPortal_Base2D *)(unaff_EBX + 0x3a37b3),
      local_28.m_Index = (ulong)pszClassOrWildcard,
      (CPortal_Base2D *)(this_00->m_iClassname).pszValue == pszClassOrWildcard ||
      (bVar14 = CBaseEntity::ClassMatchesComplex(this_00,(char *)pszClassOrWildcard),
      vecStart = (Vector *)pszClassOrWildcard, bVar14)))) {
LAB_0070fc86:
    CUtlVector<CPortalLaser::PortalLaserInfo_t,CUtlMemory<CPortalLaser::PortalLaserInfo_t,_int>_>::
    ~CUtlVector(&local_d0,(int)vecStart);
    return;
  }
  if (local_100 != (CPortal_Base2D *)0x0) {
    bVar14 = CPhysicsShadowClone::IsShadowClone((CBaseEntity *)local_100);
    vecStart = (Vector *)local_100;
    if (bVar14) {
      CPhysicsShadowClone::GetClonedEntity((CPhysicsShadowClone *)&local_20);
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
      if (((IHandleEntity)local_20._vptr_IHandleEntity == (IHandleEntity)0xffffffff) ||
         (iVar15 = ((uint)local_20._vptr_IHandleEntity & 0xffff) * 0x10 +
                   **(int **)(unaff_EBX + 0x5377bb),
         *(uint *)(iVar15 + 8) != (uint)local_20._vptr_IHandleEntity >> 0x10)) {
        vecStart = (Vector *)(CPortal_Base2D *)0x0;
      }
      else {
        vecStart = (Vector *)*(CPortal_Base2D **)(iVar15 + 4);
      }
    }
    bVar14 = ReflectLaserFromEntity(this,(CBaseEntity *)vecStart);
    if (bVar14) goto LAB_0070fc86;
  }
  if (this->m_pChildLaser != (CPortalLaser *)0x0) {
    UTIL_Remove((CBaseEntity *)this->m_pChildLaser);
    this->m_pChildLaser = (CPortalLaser *)0x0;
  }
  UTIL_SetOrigin((CBaseEntity *)this->m_pPlacementHelper,(Vector *)(local_14c + 0xc),false);
  FireAtPoint(this,(trace_t *)local_14c,true);
  local_22c = local_d0.m_Size;
  puVar5 = *(undefined4 **)(unaff_EBX + 0x5377ab);
  iVar15 = local_d0.m_Size * 0x2c;
  while( true ) {
    iVar19 = iVar15;
    pPVar10 = local_d0.m_Memory.m_pMemory;
    local_22c = local_22c + -1;
    if (local_22c < 0) break;
    *(undefined4 *)((int)&local_d0.m_Memory.m_pMemory[-1].sortedEntList.m_Size + iVar19) = 0;
    if (*(int *)((int)&local_d0.m_Memory.m_pMemory[-1].sortedEntList.m_Memory.m_nGrowSize + iVar19)
        < 0) {
      iVar20 = *(int *)((int)&local_d0.m_Memory.m_pMemory[-1].sortedEntList.m_Memory.m_pMemory +
                       iVar19);
    }
    else {
      if (*(int *)((int)&local_d0.m_Memory.m_pMemory[-1].sortedEntList.m_Memory.m_pMemory + iVar19)
          != 0) {
        (**(code **)(*(int *)*puVar5 + 8))();
        *(undefined4 *)((int)&pPVar10[-1].sortedEntList.m_Memory.m_pMemory + iVar19) = 0;
      }
      iVar20 = 0;
      *(undefined4 *)((int)&pPVar10[-1].sortedEntList.m_Memory.m_nAllocationCount + iVar19) = 0;
    }
    *(int *)((int)&pPVar10[-1].sortedEntList.m_pElements + iVar19) = iVar20;
    iVar15 = iVar19 + -0x2c;
    if (-1 < *(int *)((int)&pPVar10[-1].sortedEntList.m_Memory.m_nGrowSize + iVar19)) {
      if (iVar20 != 0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5377ab) + 8))();
        *(undefined4 *)((int)&pPVar10[-1].sortedEntList.m_Memory.m_pMemory + iVar19) = 0;
      }
      *(undefined4 *)((int)&pPVar10[-1].sortedEntList.m_Memory.m_nAllocationCount + iVar19) = 0;
    }
  }
  local_d0.m_Size = 0;
  pPVar13 = local_d0.m_Memory.m_pMemory;
  local_f4.m_PassClassnames.m_Memory.m_nGrowSize = local_d0.m_Memory.m_nGrowSize;
  if (-1 < local_d0.m_Memory.m_nGrowSize) {
    if (local_d0.m_Memory.m_pMemory != (PortalLaserInfo_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5377ab) + 8))();
      local_d0.m_Memory.m_pMemory = (PortalLaserInfo_t *)0x0;
    }
    local_d0.m_Memory.m_nAllocationCount = 0;
    pPVar10 = local_d0.m_Memory.m_pMemory;
    pPVar13 = local_d0.m_Memory.m_pMemory;
    local_f4.m_PassClassnames.m_Memory.m_nGrowSize = local_d0.m_Memory.m_nGrowSize;
  }
joined_r0x0070f9fb:
  local_d0.m_pElements = pPVar13;
  if ((-1 < local_f4.m_PassClassnames.m_Memory.m_nGrowSize) && (pPVar10 != (PortalLaserInfo_t *)0x0)
     ) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5377ab) + 8))();
  }
  return;
}


/* CPortalLaser::RemoveChildLaser at 00709080 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void RemoveChildLaser(CPortalLaser * this) */

void __thiscall CPortalLaser::RemoveChildLaser(CPortalLaser *this)

{
  if (this->m_pChildLaser != (CPortalLaser *)0x0) {
    UTIL_Remove((CBaseEntity *)this->m_pChildLaser);
    this->m_pChildLaser = (CPortalLaser *)0x0;
  }
  return;
}


/* CPortalLaser::UpdateNextLaser at 00711450 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void UpdateNextLaser(CPortalLaser * this, Vector * vecStart, Vector *
   vecDirection, CBaseEntity * pParent) */

void __thiscall
CPortalLaser::UpdateNextLaser
          (CPortalLaser *this,Vector *vecStart,Vector *vecDirection,CBaseEntity *pParent)

{
  uint uVar1;
  bool bVar2;
  int iVar3;
  CPropWeightedCube *this_00;
  ulong *puVar4;
  CPortalLaser *pCVar5;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_pChildLaser == (CPortalLaser *)0x0) {
    pCVar5 = (CPortalLaser *)CreateEntityByName((char *)(unaff_EBX + 0x39f08e),-1,true);
    this->m_pChildLaser = pCVar5;
    if (pParent != (CBaseEntity *)0x0) {
      bVar2 = UTIL_IsReflectiveCube(pParent);
      if (bVar2) {
        pCVar5 = this->m_pChildLaser;
        puVar4 = (ulong *)(*(pParent->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                            _vptr_IHandleEntity[3])(pParent);
        (pCVar5->m_hReflector).super_CBaseHandle.m_Index = *puVar4;
        (*(this->m_pChildLaser->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.
          super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x26])
                  (this->m_pChildLaser,pParent,0xffffffff);
        pCVar5 = this->m_pChildLaser;
        pCVar5->m_bFromReflectedCube = true;
      }
      else {
        pCVar5 = this->m_pChildLaser;
      }
    }
    DispatchSpawn((CBaseEntity *)pCVar5,true);
    pCVar5 = this->m_pChildLaser;
  }
  else if (pParent == (CBaseEntity *)0x0) {
    pCVar5 = this->m_pChildLaser;
  }
  else {
    bVar2 = UTIL_IsReflectiveCube(pParent);
    if (bVar2) {
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
      uVar1 = (this->m_pChildLaser->m_hReflector).super_CBaseHandle.m_Index;
      if ((uVar1 == 0xffffffff) ||
         (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x535708),
         *(uint *)(iVar3 + 8) != uVar1 >> 0x10)) {
        this_00 = (CPropWeightedCube *)0x0;
      }
      else {
        pCVar5 = this->m_pChildLaser;
        if (pParent == *(CBaseEntity **)(iVar3 + 4)) goto LAB_007114c2;
        this_00 = *(CPropWeightedCube **)(iVar3 + 4);
      }
                    /* Unresolved local var: CPropWeightedCube * pOldCube@[???] */
      CPropWeightedCube::SetLaser(this_00,(CBaseEntity *)0x0);
      pCVar5 = this->m_pChildLaser;
      puVar4 = (ulong *)(*(pParent->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                          _vptr_IHandleEntity[3])(pParent);
      (pCVar5->m_hReflector).super_CBaseHandle.m_Index = *puVar4;
      (*(this->m_pChildLaser->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.
        super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x26])
                (this->m_pChildLaser,pParent,0xffffffff);
      pCVar5 = this->m_pChildLaser;
      pCVar5->m_bFromReflectedCube = true;
    }
    else {
      pCVar5 = this->m_pChildLaser;
    }
  }
LAB_007114c2:
  this = pCVar5;
  FireLaser(this,vecStart,vecDirection,pParent);
  return;
}


/* CPortalLaser::ReflectLaserFromEntity at 00711650 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: bool ReflectLaserFromEntity(CPortalLaser * this, CBaseEntity * pEntity)
    */

bool __thiscall CPortalLaser::ReflectLaserFromEntity(CPortalLaser *this,CBaseEntity *pEntity)

{
  uint uVar1;
  bool bVar2;
  int iVar3;
  float *pfVar4;
  CPortalSimulator *this_00;
  bool bVar5;
  int unaff_EBX;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  float fVar10;
  float fVar11;
  Vector local_34;
  Vector local_28 [2];
  
  ___i686_get_pc_thunk_bx();
  bVar2 = UTIL_IsReflectiveCube(pEntity);
  bVar5 = false;
  if (bVar2) {
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
    uVar1 = pEntity[2].m_iEFlags;
    if (((uVar1 == 0xffffffff) ||
        (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x53550b),
        *(uint *)(iVar3 + 8) != uVar1 >> 0x10)) || (bVar5 = true, *(int *)(iVar3 + 4) == 0)) {
      if ((pEntity->m_iEFlags & 0x800) != 0) {
        CBaseEntity::CalcAbsolutePosition(pEntity);
      }
      AngleVectors(&pEntity->m_angAbsRotation,local_28);
                    /* Unresolved local var: Vector res@[???] */
      fVar6 = *(float *)(unaff_EBX + 0x3c1f37);
      fVar7 = local_28[0].x * fVar6;
      fVar8 = local_28[0].y * fVar6;
      fVar6 = fVar6 * local_28[0].z;
      pfVar4 = (float *)(*(pEntity->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                          _vptr_IHandleEntity[0x9d])(pEntity);
                    /* Unresolved local var: Vector res@[???] */
      local_34.x = *pfVar4 + fVar7;
      local_34.y = pfVar4[1] + fVar8;
      local_34.z = pfVar4[2] + fVar6;
      this_00 = CPortalSimulator::GetSimulatorThatOwnsEntity(pEntity);
      if ((this_00 != (CPortalSimulator *)0x0) &&
         (bVar2 = CPortalSimulator::EntityIsInPortalHole(this_00,pEntity), bVar2)) {
        fVar6 = (local_34.x *
                 (this_00->m_InternalData).super_PS_InternalData_t.Placement.PortalPlane.m_Normal.x
                 + local_34.y *
                   (this_00->m_InternalData).super_PS_InternalData_t.Placement.PortalPlane.m_Normal.
                   y + local_34.z *
                       (this_00->m_InternalData).super_PS_InternalData_t.Placement.PortalPlane.
                       m_Normal.z) -
                (this_00->m_InternalData).super_PS_InternalData_t.Placement.PortalPlane.m_Dist;
        if ((fVar6 < *(float *)(unaff_EBX + 0x3bb50f)) &&
           ((!NAN(fVar6) && !NAN(*(float *)(unaff_EBX + 0x3bb50f)) &&
            (pfVar4 = (float *)(*(pEntity->super_IServerEntity).super_IServerUnknown.
                                 super_IHandleEntity._vptr_IHandleEntity[0x9d])(pEntity),
            fVar6 = (*pfVar4 * (this_00->m_InternalData).super_PS_InternalData_t.Placement.
                               PortalPlane.m_Normal.x +
                     pfVar4[1] *
                     (this_00->m_InternalData).super_PS_InternalData_t.Placement.PortalPlane.
                     m_Normal.y +
                    pfVar4[2] *
                    (this_00->m_InternalData).super_PS_InternalData_t.Placement.PortalPlane.m_Normal
                    .z) - (this_00->m_InternalData).super_PS_InternalData_t.Placement.PortalPlane.
                          m_Dist,
            *(float *)(unaff_EBX + 0x3bb50f) <= fVar6 && fVar6 != *(float *)(unaff_EBX + 0x3bb50f)))
           )) {
                    /* Unresolved local var: Vector vRet@[???] */
          fVar6 = (this_00->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m[0]
                  [2] * local_34.z;
          fVar9 = (this_00->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m[1]
                  [0] * local_34.x;
          fVar7 = (this_00->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m[1]
                  [2] * local_34.z;
          local_34.z = (this_00->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                       [2][0] * local_34.x +
                       (this_00->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                       [2][1] * local_34.y +
                       (this_00->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                       [2][2] * local_34.z +
                       (this_00->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                       [2][3];
          fVar11 = local_28[0].x *
                   (this_00->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m[2]
                   [0];
          fVar8 = local_28[0].y *
                  (this_00->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m[2]
                  [1];
          fVar10 = local_28[0].x *
                   (this_00->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m[1]
                   [0];
          local_28[0].x =
               local_28[0].x *
               (this_00->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m[0][0] +
               local_28[0].y *
               (this_00->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m[0][1] +
               local_28[0].z *
               (this_00->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m[0][2];
          local_34.x = (this_00->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                       [0][0] * local_34.x +
                       (this_00->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                       [0][1] * local_34.y + fVar6 +
                       (this_00->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                       [0][3];
          local_34.y = fVar9 + (this_00->m_InternalData).super_PS_InternalData_t.Placement.
                               matThisToLinked.m[1][1] * local_34.y + fVar7 +
                       (this_00->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                       [1][3];
          local_28[0].y =
               fVar10 + local_28[0].y *
                        (this_00->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.
                        m[1][1] +
               local_28[0].z *
               (this_00->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m[1][2];
          local_28[0].z =
               fVar11 + fVar8 +
               local_28[0].z *
               (this_00->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m[2][2];
        }
      }
      UTIL_Portal_Laser_Prevent_Tilting(local_28);
      UpdateNextLaser(this,&local_34,local_28,pEntity);
      CPropWeightedCube::SetLaser((CPropWeightedCube *)pEntity,(CBaseEntity *)this->m_pChildLaser);
      bVar5 = true;
    }
  }
  return bVar5;
}


/* _GLOBAL__I_sv_player_collide_with_laser at 000a8e90 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_sv_player_collide_with_laser(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

