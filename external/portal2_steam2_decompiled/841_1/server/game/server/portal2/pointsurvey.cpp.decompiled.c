/* DWARF-guided pseudocode for game/server/portal2/pointsurvey.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000af230 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  int *piVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  IEntityFactoryDictionary *pIVar4;
  datamap_t *pdVar5;
  int iVar6;
  int iVar7;
  int unaff_EBX;
  undefined4 *puVar8;
  longlong lVar9;
  ConVar *in_stack_ffffffd4;
  char *s2;
  char *in_stack_ffffffd8;
  SendTable *this;
  char *in_stack_ffffffdc;
  int in_stack_ffffffe0;
  
  lVar9 = ___i686_get_pc_thunk_bx();
  if (lVar9 == 0xffff00000001) {
    (&DAT_0000139c)[(int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
    (&UNK_0000139d)[(int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
    (&UNK_0000139e)[(int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
    (&UNK_0000139f)[(int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
    *(undefined4 *)
     (&DAT_000013a0 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
    *(undefined4 *)
     (&DAT_000013a4 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
    *(undefined4 *)
     (&DAT_000013a8 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
         0x7f7fffff;
    *(undefined4 *)
     (&DAT_000013ac + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
         0x7f7fffff;
    *(undefined4 *)
     (&DAT_000013b0 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
    *(undefined4 *)
     (&DAT_000013b4 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
    *(undefined4 *)
     (&UNK_000013b8 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
    *(undefined4 *)
     (&UNK_000013bc + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
    *(undefined4 *)
     (&UNK_000013c0 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
         0x7f7fffff;
    *(undefined4 *)
     (&UNK_000013c4 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
         0x7f7fffff;
    *(undefined4 *)
     (&UNK_000013c8 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
         0x7f7fffff;
    *(undefined4 *)
     (&UNK_000013cc + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
         0x7f7fffff;
    *(undefined **)
     (&UNK_000013d0 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
         &UNK_00b9d0c4 + unaff_EBX;
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<CAI_StandoffBehavior>::dataDesc[0x12].flatOffset +
                   unaff_EBX + 8),(char *)(unaff_EBX + 0xa036cf),(char *)(unaff_EBX + 0x99fbbc),
                   0x2080,in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0);
    ___cxa_atexit(unaff_EBX + 0x98ee5c,0,*(undefined4 *)(&DAT_00b97910 + unaff_EBX));
    *(undefined **)
     (&UNK_000013d4 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
         &UNK_00c1f28c + unaff_EBX;
    pIVar4 = EntityFactoryDictionary();
    (**pIVar4->_vptr_IEntityFactoryDictionary)(pIVar4,unaff_EBX + 0xd9efb4,unaff_EBX + 0xa036e7);
    pdVar5 = DataMapInit<CPointSurvey>((CPointSurvey *)0x0);
    *(datamap_t **)
     ((int)DataMapInit<AI_StandoffParams_t>::dataDesc[1].flatOffset + unaff_EBX + -0x20) = pdVar5;
    iVar7 = unaff_EBX + 0xd9efb8;
    s2 = (char *)(unaff_EBX + 0xa036c2);
    *(char **)(&UNK_000013d8 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
         s2;
    this = (SendTable *)
           ((int)DataMapInit<AI_StandoffParams_t>::dataDesc[1].flatOffset + unaff_EBX + -0x1c);
    *(SendTable **)
     (&UNK_000013dc + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = this;
    *(undefined4 *)
     (&UNK_000013e8 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0xffff;
    piVar1 = *(int **)(&DAT_00b979b0 + unaff_EBX);
    puVar8 = (undefined4 *)*piVar1;
    if (puVar8 == (undefined4 *)0x0) {
      *piVar1 = iVar7;
      *(undefined4 *)
       (&UNK_000013e0 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
    }
    else {
      puVar2 = (undefined4 *)puVar8[2];
      iVar6 = _V_stricmp((char *)*puVar8,s2);
      if (iVar6 < 1) {
        while ((puVar3 = puVar2, puVar3 != (undefined4 *)0x0 &&
               (iVar6 = _V_stricmp((char *)*puVar3,s2), iVar6 < 1))) {
          puVar2 = (undefined4 *)puVar3[2];
          puVar8 = puVar3;
        }
        *(undefined4 **)(iVar7 + 8) = puVar3;
        puVar8[2] = iVar7;
      }
      else {
        *(int *)(iVar7 + 8) = *piVar1;
        *piVar1 = iVar7;
      }
    }
    SendTable::SendTable(this);
    ___cxa_atexit(unaff_EBX + 0x98ee3c,0,*(undefined4 *)(&DAT_00b97910 + unaff_EBX));
    iVar7 = ServerClassInit<DT_PointSurvey::ignored>((ignored *)0x0);
    *(int *)((int)DataMapInit<AI_StandoffParams_t>::dataDesc[1].flatOffset + unaff_EBX + -8) = iVar7
    ;
  }
  return;
}


/* __tcf_2 at 00a3e0a0 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38b2d8),in_stack_00000008);
  return;
}


/* DataMapInit<CPointSurvey> at 000af110 */

datamap_t * DataMapInit<CPointSurvey>(CPointSurvey *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x161].m_pPrev + unaff_EBX) ==
      '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xd9f1fc);
    if (iVar1 != 0) {
      *(int *)(&DAT_0000162c + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
           unaff_EBX + 0xa037e2;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x163].m_pEntity + unaff_EBX) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x163].m_SerialNumber + unaff_EBX) = 0;
      *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x163].m_pPrev + unaff_EBX)
           = 0;
      *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x163].m_pNext + unaff_EBX)
           = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x164].m_pEntity + unaff_EBX) = 0;
      *(undefined4 *)
       (&DAT_0000162c + (int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX)
           = 0xc;
      ___cxa_guard_release(unaff_EBX + 0xd9f1fc);
      ___cxa_atexit(unaff_EBX + 0x98ef9c,0,*(undefined4 *)(&DAT_00b97a30 + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_BloodColor_00cb6ee8 + unaff_EBX) =
       *(undefined4 *)(&DAT_00b97b68 + unaff_EBX);
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x162].m_pEntity + unaff_EBX) ==
      '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xd9f204);
    if (iVar1 != 0) {
      *(undefined4 *)((int)&PTR_NetworkStateChanged_m_lifeState_00cb6fd0 + unaff_EBX) =
           **(undefined4 **)(&DAT_00b97a50 + unaff_EBX);
      ___cxa_guard_release(unaff_EBX + 0xd9f204);
    }
  }
  *(undefined4 *)((int)&PTR_Event_Killed_00cb6ee0 + unaff_EBX) = 3;
  *(int *)((int)&PTR_IsAlive_00cb6edc + unaff_EBX) = (int)&PTR_IsViewable_00cb6f3c + unaff_EBX;
  return (datamap_t *)((int)&PTR_IsAlive_00cb6edc + unaff_EBX);
}


/* CPointSurvey::GetDataDescMap at 0073f480 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPointSurvey * this) */

datamap_t * __thiscall CPointSurvey::GetDataDescMap(CPointSurvey *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x626b78);
}


/* CPointSurvey::GetBaseMap at 0073f490 */

datamap_t * CPointSurvey::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x5077f4);
}


/* __tcf_0 at 00a3e0c0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x410276)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41026a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x410276));
  }
  *(undefined4 *)(unaff_EBX + 0x410276) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x410272)) {
    if (*(int *)(unaff_EBX + 0x41026a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x208a8e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x208a8e),*(int *)(unaff_EBX + 0x41026a));
      *(undefined4 *)(unaff_EBX + 0x41026a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41026e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41026a);
  *(int *)(unaff_EBX + 0x41027a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x410272)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x208a8e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x208a8e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41026a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41026e) = 0;
  }
  return;
}


/* ServerClassInit<DT_PointSurvey::ignored> at 000aef50 */

int ServerClassInit<DT_PointSurvey::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x16c].m_pPrev + unaff_EBX) ==
      '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xd9f2ac);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x16d].m_pPrev + unaff_EBX),
                  (char *)(vgui::Panel::OnDelete + unaff_EBX),0,4,-1,0,(SendVarProxyFn_conflict)0x0,
                  0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x172].m_pNext +
                        unaff_EBX),(char *)(unaff_EBX + 0x9a9080),0,
                        (SendTable *)**(undefined4 **)(&DAT_00b97ce8 + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00b97c8c + unaff_EBX),0x80);
      SendPropStringT((SendProp_conflict *)
                      ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x178].m_pEntity +
                      unaff_EBX),(char *)(unaff_EBX + 0xa03992),0x3c0,4);
      ___cxa_guard_release(unaff_EBX + 0xd9f2ac);
      ___cxa_atexit(unaff_EBX + 0x98f0cc,0,*(undefined4 *)(&DAT_00b97bf0 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)((int)DataMapInit<CAI_BattleLine>::dataDesc[1].flatOffset + unaff_EBX + 4)
             ,(SendProp_conflict *)
              ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x172].m_pNext + unaff_EBX),2,
             *(char **)(&DAT_00c1f544 + unaff_EBX));
  return 1;
}


/* CPointSurvey::GetServerClass at 0073f4a0 */

/* DWARF original prototype: ServerClass * GetServerClass(CPointSurvey * this) */

ServerClass * __thiscall CPointSurvey::GetServerClass(CPointSurvey *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x70ed54);
}


/* CPointSurvey::YouForgotToImplementOrDeclareServerClass at 0073f4b0 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CPointSurvey * this) */

int __thiscall CPointSurvey::YouForgotToImplementOrDeclareServerClass(CPointSurvey *this)

{
  return 0;
}


/* __tcf_3 at 00a3e080 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(&UNK_0038b354 + extraout_ECX),in_stack_00000008);
  return;
}


/* __tcf_1 at 00a3e030 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410287))(unaff_EBX + 0x410287);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410233))(unaff_EBX + 0x410233);
                    /* WARNING: Could not recover jumptable at 0x00a3e077. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x4101df))();
  return;
}


/* CPointSurvey::InputShowSurvey at 0073f520 */

/* DWARF original prototype: void InputShowSurvey(CPointSurvey * this, inputdata_t * inputdata) */

void __thiscall CPointSurvey::InputShowSurvey(CPointSurvey *this,inputdata_t *inputdata)

{
  int unaff_EBX;
  char *messagename;
  CRecipientFilter local_3c;
  
                    /* Unresolved local var: CBroadcastRecipientFilter filter@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(int *)(*(int *)(unaff_EBX + 0x689e68) + 0x30) == 0) {
    CRecipientFilter::CRecipientFilter(&local_3c);
    local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
         (_func_int_varargs **)(CAI_ActBusyQueueGoal::InputPlayerStoppedBlocking + unaff_EBX + 4);
    CRecipientFilter::AddAllPlayers(&local_3c);
    CRecipientFilter::MakeReliable(&local_3c);
    messagename = (char *)(unaff_EBX + 0x320742);
    UserMessageBegin(&local_3c.super_IRecipientFilter,messagename);
    MessageWriteEHandle(&this->super_CBaseEntity);
    MessageEnd();
    local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
         (_func_int_varargs **)(CAI_ActBusyQueueGoal::InputPlayerStoppedBlocking + unaff_EBX + 4);
    CRecipientFilter::~CRecipientFilter(&local_3c,(int)messagename);
  }
  else {
    COutputEvent::FireOutput
              (&this->m_OnSurveyComplete,&this->super_CBaseEntity,&this->super_CBaseEntity,0.0);
  }
  return;
}


/* CPointSurvey::OnSurveyCompleted at 0073f4f0 */

/* DWARF original prototype: void OnSurveyCompleted(CPointSurvey * this) */

void __thiscall CPointSurvey::OnSurveyCompleted(CPointSurvey *this)

{
  COutputEvent::FireOutput
            (&this->m_OnSurveyComplete,&this->super_CBaseEntity,&this->super_CBaseEntity,0.0);
  return;
}


/* CPointSurvey::Activate at 0073f4c0 */

/* DWARF original prototype: void Activate(CPointSurvey * this) */

void __thiscall CPointSurvey::Activate(CPointSurvey *this)

{
  CBaseEntity::Activate(&this->super_CBaseEntity);
  CBaseEntity::SetTransmitState(&this->super_CBaseEntity,8);
  return;
}


/* _GLOBAL__I_cl_disable_survey_panel at 000af460 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_cl_disable_survey_panel(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

