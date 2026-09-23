/* DWARF-guided pseudocode for game/server/portal2/pointsurvey.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000c4230 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  undefined4 *puVar1;
  IEntityFactoryDictionary *pIVar2;
  datamap_t *pdVar3;
  int iVar4;
  int iVar5;
  int unaff_EBX;
  undefined4 *puVar6;
  undefined4 *puVar7;
  longlong lVar8;
  ConVar *in_stack_ffffffd4;
  char *s2;
  char *in_stack_ffffffd8;
  SendTable *this;
  char *in_stack_ffffffdc;
  int in_stack_ffffffe0;
  
  lVar8 = ___i686_get_pc_thunk_bx();
  if (lVar8 == 0xffff00000001) {
    (&DAT_00cf4c1c)[unaff_EBX] = 0;
    (&DAT_00cf4c1d)[unaff_EBX] = 0;
    (&DAT_00cf4c1e)[unaff_EBX] = 0;
    (&DAT_00cf4c1f)[unaff_EBX] = 0;
    *(undefined4 *)((int)&mp_forcecamera.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) = 0;
    *(undefined4 *)((int)&mp_forcecamera.super_ConCommandBase.m_pNext + unaff_EBX) = 0;
    *(undefined4 *)(&mp_forcecamera.super_ConCommandBase.m_bRegistered + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&mp_forcecamera.super_ConCommandBase.m_pszName + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&mp_forcecamera.super_ConCommandBase.m_pszHelpString + unaff_EBX) = 0;
    *(undefined4 *)((int)&mp_forcecamera.super_ConCommandBase.m_nFlags + unaff_EBX) = 0;
    *(undefined4 *)((int)&mp_forcecamera.super_IConVar._vptr_IConVar + unaff_EBX) = 0;
    *(undefined4 *)((int)&mp_forcecamera.m_pParent + unaff_EBX) = 0;
    *(undefined4 *)((int)&mp_forcecamera.m_pszDefaultValue + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&mp_forcecamera.m_Value.m_pszString + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&mp_forcecamera.m_Value.m_StringLength + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&mp_forcecamera.m_Value.m_fValue + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&mp_forcecamera.m_Value.m_nValue + unaff_EBX) = 0;
    (&mp_forcecamera.field_0x37)[unaff_EBX] = 0;
    (&mp_forcecamera.m_bHasMin)[unaff_EBX] = false;
    (&mp_forcecamera.field_0x35)[unaff_EBX] = 0;
    (&mp_forcecamera.field_0x36)[unaff_EBX] = 0;
    *(undefined4 *)((int)&mp_forcecamera.m_fMinVal + unaff_EBX) = 0;
    (&mp_forcecamera.field_0x3f)[unaff_EBX] = 0;
    (&mp_forcecamera.m_bHasMax)[unaff_EBX] = false;
    (&mp_forcecamera.field_0x3d)[unaff_EBX] = 0;
    (&mp_forcecamera.field_0x3e)[unaff_EBX] = 0;
    *(undefined4 *)((int)&mp_forcecamera.m_fMaxVal + unaff_EBX) = 0;
    *(undefined1 *)((int)&mp_forcecamera.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 3) = 1
    ;
    *(undefined1 *)((int)&mp_forcecamera.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) = 0;
    *(undefined1 *)((int)&mp_forcecamera.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 1) = 0
    ;
    *(undefined1 *)((int)&mp_forcecamera.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 2) = 0
    ;
    *(undefined4 *)
     ((int)&mp_forcecamera.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) = 1;
    *(undefined1 *)((int)&mp_forcecamera.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 3) =
         0;
    *(undefined1 *)((int)&mp_forcecamera.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) = 0;
    *(undefined1 *)((int)&mp_forcecamera.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 1) =
         0;
    *(undefined1 *)((int)&mp_forcecamera.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 2) =
         0;
    *(undefined4 *)((int)&mp_forcecamera.m_fnChangeCallbacks.m_Size + unaff_EBX) = 2;
    *(undefined1 *)((int)&mp_forcecamera.m_fnChangeCallbacks.m_pElements + unaff_EBX + 3) = 0;
    *(undefined1 *)((int)&mp_forcecamera.m_fnChangeCallbacks.m_pElements + unaff_EBX) = 0;
    *(undefined1 *)((int)&mp_forcecamera.m_fnChangeCallbacks.m_pElements + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&mp_forcecamera.m_fnChangeCallbacks.m_pElements + unaff_EBX + 2) = 0;
    *(undefined **)(&DAT_00cf4c78 + unaff_EBX) = &UNK_00aed0e4 + unaff_EBX;
    ConVar::ConVar((ConVar *)((int)&PTR_NavigateTo_00c6b3dc + unaff_EBX),
                   (char *)(unaff_EBX + 0x94c683),(char *)(unaff_EBX + 0x8e79f4),0x2080,
                   in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0);
    ___cxa_atexit(unaff_EBX + 0x8d6a4c,0,*(undefined4 *)(&DAT_00ae794c + unaff_EBX));
    *(undefined **)(&DAT_00cf4c7c + unaff_EBX) = &UNK_00b714cc + unaff_EBX;
    pIVar2 = EntityFactoryDictionary();
    (**pIVar2->_vptr_IEntityFactoryDictionary)
              (pIVar2,&DAT_00cf4c7c + unaff_EBX,unaff_EBX + 0x94c69b);
    pdVar3 = DataMapInit<CPointSurvey>((CPointSurvey *)0x0);
    *(datamap_t **)((int)&PTR_InternalKeyCodeTyped_00c6b434 + unaff_EBX) = pdVar3;
    iVar5 = (int)&mp_allowspectators.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX;
    s2 = (char *)(unaff_EBX + 0x94c676);
    *(char **)((int)&mp_allowspectators.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) = s2;
    this = (SendTable *)((int)&PTR_InternalKeyTyped_00c6b438 + unaff_EBX);
    *(SendTable **)((int)&mp_allowspectators.super_ConCommandBase.m_pNext + unaff_EBX) = this;
    *(undefined4 *)((int)&mp_allowspectators.super_ConCommandBase.m_pszHelpString + unaff_EBX) =
         0xffff;
    puVar7 = (undefined4 *)**(int **)(&DAT_00ae79ec + unaff_EBX);
    if (puVar7 == (undefined4 *)0x0) {
      **(int **)(&DAT_00ae79ec + unaff_EBX) = iVar5;
      *(undefined4 *)(&mp_allowspectators.super_ConCommandBase.m_bRegistered + unaff_EBX) = 0;
    }
    else {
      puVar1 = (undefined4 *)puVar7[2];
      iVar4 = _V_stricmp((char *)*puVar7,s2);
      if (iVar4 < 1) {
        while ((puVar6 = puVar1, puVar6 != (undefined4 *)0x0 &&
               (iVar4 = _V_stricmp((char *)*puVar6,s2), iVar4 < 1))) {
          puVar1 = (undefined4 *)puVar6[2];
          puVar7 = puVar6;
        }
        *(undefined4 **)(iVar5 + 8) = puVar6;
        puVar7[2] = iVar5;
      }
      else {
        *(undefined4 *)(iVar5 + 8) = **(undefined4 **)(&DAT_00ae79ec + unaff_EBX);
        **(int **)(&DAT_00ae79ec + unaff_EBX) = iVar5;
      }
    }
    SendTable::SendTable(this);
    ___cxa_atexit(unaff_EBX + 0x8d6a2c,0,*(undefined4 *)(&DAT_00ae794c + unaff_EBX));
    iVar5 = ServerClassInit<DT_PointSurvey::ignored>((ignored *)0x0);
    *(int *)((int)&PTR_InternalMove_00c6b44c + unaff_EBX) = iVar5;
  }
  return;
}


/* __tcf_2 at 0099ac90 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x394988),in_stack_00000008);
  return;
}


/* DataMapInit<CPointSurvey> at 000c4110 */

datamap_t * DataMapInit<CPointSurvey>(CPointSurvey *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&DT_HalfLife2Proxy::g_SendTable.m_nProps + unaff_EBX) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf4ebc);
    if (iVar1 != 0) {
      *(int *)((int)&DT_HalfLife2Proxy::g_SendTableInit + unaff_EBX) = unaff_EBX + 0x94c796;
      *(undefined4 *)(&DAT_00cf4ed4 + unaff_EBX) = 0;
      *(undefined4 *)(&DAT_00cf4ed8 + unaff_EBX) = 0;
      *(undefined4 *)(&DAT_00cf4edc + unaff_EBX) = 0;
      *(undefined4 *)
       ((int)&physcannon_mega_enabled.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) = 0;
      *(undefined4 *)((int)&physcannon_mega_enabled.super_ConCommandBase.m_pNext + unaff_EBX) = 0;
      *(undefined4 *)(&DAT_00cf4ed0 + unaff_EBX) = 0xc;
      ___cxa_guard_release(unaff_EBX + 0xcf4ebc);
      ___cxa_atexit(unaff_EBX + 0x8d6b8c,0,*(undefined4 *)(&DAT_00ae7a6c + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_TestHitboxes_00c07de8 + unaff_EBX) =
       *(undefined4 *)(&DAT_00ae7ba8 + unaff_EBX);
  if (*(char *)((int)&DT_HalfLife2Proxy::g_SendTable.m_pPrecalc + unaff_EBX) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf4ec4);
    if (iVar1 != 0) {
      *(undefined4 *)((int)&PTR_IsNPC_00c07ed0 + unaff_EBX) =
           **(undefined4 **)(&DAT_00ae7a88 + unaff_EBX);
      ___cxa_guard_release(unaff_EBX + 0xcf4ec4);
    }
  }
  *(undefined4 *)((int)&PTR_GetAIAddOn_00c07de0 + unaff_EBX) = 3;
  *(int *)((int)&PTR_GetScriptDesc_00c07ddc + unaff_EBX) = (int)&PTR_Activate_00c07e3c + unaff_EBX;
  return (datamap_t *)((int)&PTR_GetScriptDesc_00c07ddc + unaff_EBX);
}


/* CPointSurvey::GetDataDescMap at 006d29d0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPointSurvey * this) */

datamap_t * __thiscall CPointSurvey::GetDataDescMap(CPointSurvey *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(&UNK_005f9528 + extraout_ECX);
}


/* CPointSurvey::GetBaseMap at 006d29e0 */

datamap_t * CPointSurvey::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4d92e4);
}


/* __tcf_0 at 0099acb0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41e346)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41e33a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41e346));
  }
  *(undefined4 *)(unaff_EBX + 0x41e346) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41e342)) {
    if (*(int *)(unaff_EBX + 0x41e33a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x210eda) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x210eda),*(int *)(unaff_EBX + 0x41e33a));
      *(undefined4 *)(unaff_EBX + 0x41e33a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e33e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41e33a);
  *(int *)(unaff_EBX + 0x41e34a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41e342)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x210eda) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x210eda),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41e33a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e33e) = 0;
  }
  return;
}


/* ServerClassInit<DT_PointSurvey::ignored> at 000c3f50 */

int ServerClassInit<DT_PointSurvey::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&sv_robust_explosions.m_bHasMin)[unaff_EBX] == false) {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf4f74);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)(&sv_robust_explosions.m_bHasMax + unaff_EBX),
                  (char *)(unaff_EBX + 0x8eeac0),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&sk_dmg_inflict_scale1.m_Value.m_nValue + unaff_EBX),
                        &UNK_008f0e30 + unaff_EBX,0,
                        (SendTable *)**(undefined4 **)(&DAT_00ae7d24 + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00ae7cc8 + unaff_EBX),0x80);
      SendPropStringT((SendProp_conflict *)
                      ((int)&sk_dmg_inflict_scale2.m_Value.m_pszString + unaff_EBX),
                      (char *)(unaff_EBX + 0x94c946),0x3c0,4);
      ___cxa_guard_release(unaff_EBX + 0xcf4f74);
      ___cxa_atexit(unaff_EBX + 0x8d6cbc,0,*(undefined4 *)(&DAT_00ae7c2c + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)((int)&PTR_LookupElementBounds_00c6b718 + unaff_EBX),
             (SendProp_conflict *)((int)&sk_dmg_inflict_scale1.m_Value.m_nValue + unaff_EBX),2,
             *(char **)(&DAT_00b71784 + unaff_EBX));
  return 1;
}


/* CPointSurvey::GetServerClass at 006d29f0 */

/* DWARF original prototype: ServerClass * GetServerClass(CPointSurvey * this) */

ServerClass * __thiscall CPointSurvey::GetServerClass(CPointSurvey *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x6e64cc);
}


/* CPointSurvey::YouForgotToImplementOrDeclareServerClass at 006d2a00 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CPointSurvey * this) */

int __thiscall CPointSurvey::YouForgotToImplementOrDeclareServerClass(CPointSurvey *this)

{
  return 0;
}


/* __tcf_3 at 0099ac70 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x394a04),in_stack_00000008);
  return;
}


/* __tcf_1 at 0099ac20 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41e357))(unaff_EBX + 0x41e357);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41e303))(unaff_EBX + 0x41e303);
                    /* WARNING: Could not recover jumptable at 0x0099ac67. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41e2af))();
  return;
}


/* CPointSurvey::InputShowSurvey at 006d2a70 */

/* DWARF original prototype: void InputShowSurvey(CPointSurvey * this, inputdata_t * inputdata) */

void __thiscall CPointSurvey::InputShowSurvey(CPointSurvey *this,inputdata_t *inputdata)

{
  int unaff_EBX;
  char *messagename;
  CRecipientFilter local_3c;
  
                    /* Unresolved local var: CBroadcastRecipientFilter filter@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(int *)(*(int *)(unaff_EBX + 0x65cbb8) + 0x30) == 0) {
    CRecipientFilter::CRecipientFilter(&local_3c);
    local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
         (_func_int_varargs **)(unaff_EBX + 0x4db784);
    CRecipientFilter::AddAllPlayers(&local_3c);
    CRecipientFilter::MakeReliable(&local_3c);
    messagename = (char *)(unaff_EBX + 0x2e9fb6);
    UserMessageBegin(&local_3c.super_IRecipientFilter,messagename);
    MessageWriteEHandle(&this->super_CBaseEntity);
    MessageEnd();
    local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
         (_func_int_varargs **)(unaff_EBX + 0x4db784);
    CRecipientFilter::~CRecipientFilter(&local_3c,(int)messagename);
  }
  else {
    COutputEvent::FireOutput
              (&this->m_OnSurveyComplete,&this->super_CBaseEntity,&this->super_CBaseEntity,0.0);
  }
  return;
}


/* CPointSurvey::OnSurveyCompleted at 006d2a40 */

/* DWARF original prototype: void OnSurveyCompleted(CPointSurvey * this) */

void __thiscall CPointSurvey::OnSurveyCompleted(CPointSurvey *this)

{
  COutputEvent::FireOutput
            (&this->m_OnSurveyComplete,&this->super_CBaseEntity,&this->super_CBaseEntity,0.0);
  return;
}


/* CPointSurvey::Activate at 006d2a10 */

/* DWARF original prototype: void Activate(CPointSurvey * this) */

void __thiscall CPointSurvey::Activate(CPointSurvey *this)

{
  CBaseEntity::Activate(&this->super_CBaseEntity);
  CBaseEntity::SetTransmitState(&this->super_CBaseEntity,8);
  return;
}


/* _GLOBAL__I_cl_disable_survey_panel at 000c4500 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_cl_disable_survey_panel(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

