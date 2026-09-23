/* DWARF-guided pseudocode for game/client/portal2/vgui/surveypanel.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 00081470 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  bool *pbVar1;
  undefined4 uVar2;
  int unaff_EBX;
  longlong lVar3;
  ConVar *in_stack_ffffffd4;
  char *in_stack_ffffffd8;
  char *in_stack_ffffffdc;
  int in_stack_ffffffe0;
  
  lVar3 = ___i686_get_pc_thunk_bx();
  if (lVar3 == 0xffff00000001) {
    (&DAT_00c5805c)[unaff_EBX] = 0;
    (&DAT_00c5805d)[unaff_EBX] = 0;
    (&DAT_00c5805e)[unaff_EBX] = 0;
    (&DAT_00c5805f)[unaff_EBX] = 0;
    *(undefined4 *)
     ((int)&CUtlLinkedList<JiggleData,short_unsigned_int,false,short_unsigned_int,CUtlMemory<UtlLinkedListElem_t<JiggleData,_short_unsigned_int>,_short_unsigned_int>_>
            ::AllocInternal::__executeCount_1 + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&CUtlLinkedList<JiggleData,short_unsigned_int,false,short_unsigned_int,CUtlMemory<UtlLinkedListElem_t<JiggleData,_short_unsigned_int>,_short_unsigned_int>_>
            ::AllocInternal::__executeCount + unaff_EBX) = 0;
    *(undefined4 *)((int)&s_id + unaff_EBX) = 0x7f7fffff;
    pbVar1 = &g_bUseNetworkVars + unaff_EBX;
    pbVar1[0] = true;
    pbVar1[1] = true;
    pbVar1[2] = true;
    pbVar1[3] = true;
    *(undefined4 *)
     ((int)&CUtlRBTree<CUtlMap<CStudioHdr_const*,ModelPoseDebugInfo,unsigned_short>::Node_t,unsigned_short,CUtlMap<CStudioHdr_const*,ModelPoseDebugInfo,unsigned_short>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<CStudioHdr_const*,ModelPoseDebugInfo,unsigned_short>::Node_t,unsigned_short>,unsigned_short>>
            ::Links(unsigned_short)::s_Sentinel + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00c58074 + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&CUtlRBTree<CUtlMap<CStudioHdr_const*,ModelPoseDebugInfo,unsigned_short>::Node_t,unsigned_short,CUtlMap<CStudioHdr_const*,ModelPoseDebugInfo,unsigned_short>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<CStudioHdr_const*,ModelPoseDebugInfo,unsigned_short>::Node_t,unsigned_short>,unsigned_short>>
            ::Links(unsigned_short)::s_Sentinel + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00c5807c + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&CUtlRBTree<CUtlMap<char_const*,int,unsigned_short>::Node_t,unsigned_short,CUtlMap<char_const*,int,unsigned_short>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<char_const*,int,unsigned_short>::Node_t,unsigned_short>,unsigned_short>>
            ::Links(unsigned_short)::s_Sentinel + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00c58084 + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)
     ((int)&CUtlRBTree<CUtlMap<char_const*,int,unsigned_short>::Node_t,unsigned_short,CUtlMap<char_const*,int,unsigned_short>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<char_const*,int,unsigned_short>::Node_t,unsigned_short>,unsigned_short>>
            ::Links(unsigned_short)::s_Sentinel + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00c5808c + unaff_EBX) = 0x7f7fffff;
    *(undefined **)
     ((int)&CUtlRBTree<CUtlMap<studiohdr_t_const*,StudioHdrToActivityMapEntry_t,unsigned_short>::Node_t,unsigned_short,CUtlMap<studiohdr_t_const*,StudioHdrToActivityMapEntry_t,unsigned_short>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<studiohdr_t_const*,StudioHdrToActivityMapEntry_t,unsigned_short>::Node_t,unsigned_short>,unsigned_short>>
            ::Links(unsigned_short)::s_Sentinel + unaff_EBX) = &UNK_00ae3a04 + unaff_EBX;
    ConVar::ConVar((ConVar *)((int)ice_sbox[3] + unaff_EBX + 0xc7c),&UNK_00949d25 + unaff_EBX,
                   (char *)(unaff_EBX + 0x901a38),0x2080,in_stack_ffffffd4,in_stack_ffffffd8,
                   in_stack_ffffffdc,in_stack_ffffffe0);
    uVar2 = *(undefined4 *)(&DAT_00ae28e8 + unaff_EBX);
    ___cxa_atexit(unaff_EBX + 0x8fc0ec,0,uVar2);
    ConVar::ConVar((ConVar *)((int)ice_sbox[3] + unaff_EBX + 0xcdc),&UNK_00949d40 + unaff_EBX,
                   (char *)(unaff_EBX + 0x901a38),0,in_stack_ffffffd4,in_stack_ffffffd8,
                   in_stack_ffffffdc,in_stack_ffffffe0);
    ___cxa_atexit(unaff_EBX + 0x8fc0cc,0,uVar2);
    CAutoGameSystem::CAutoGameSystem((CAutoGameSystem *)(&DAT_00c5809c + unaff_EBX),(char *)0x0);
    *(int *)(&DAT_00c5809c + unaff_EBX) =
         (int)&PTR_ComputeWorldSpaceSurroundingBox_00b74664 + unaff_EBX;
    ___cxa_atexit(unaff_EBX + 0x8fc10c,0,uVar2);
    ConCommand::ConCommand
              ((ConCommand *)(unaff_EBX + 0xc581bc),(char *)(unaff_EBX + 0x949e1d),
               (FnCommandCallback_t)(&UNK_005a328c + unaff_EBX),&UNK_00949d64 + unaff_EBX,0,
               (FnCommandCompletionCallback)0x0);
    ___cxa_atexit(unaff_EBX + 0x8fc0ac,0,uVar2);
  }
  return;
}


/* __tcf_0 at 0097d570 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ed7a8),in_stack_00000008);
  return;
}


/* __tcf_1 at 0097d550 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ed828),in_stack_00000008);
  return;
}


/* CTimeInMapHelper::~CTimeInMapHelper at 00627400 */

/* DWARF original prototype: void ~CTimeInMapHelper(CTimeInMapHelper * this, int __in_chrg) */

void __thiscall CTimeInMapHelper::~CTimeInMapHelper(CTimeInMapHelper *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(extraout_ECX + 0x53eca0);
  IGameSystem::~IGameSystem((IGameSystem *)this,__in_chrg);
  return;
}


/* CTimeInMapHelper::~CTimeInMapHelper at 006274f0 */

/* DWARF original prototype: void ~CTimeInMapHelper(CTimeInMapHelper * this, int __in_chrg) */

void __thiscall CTimeInMapHelper::~CTimeInMapHelper(CTimeInMapHelper *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(unaff_EBX + 0x53eba7);
  IGameSystem::~IGameSystem((IGameSystem *)this,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CTimeInMapHelper::Name at 00626700 */

/* DWARF original prototype: char * Name(CTimeInMapHelper * this) */

char * __thiscall CTimeInMapHelper::Name(CTimeInMapHelper *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (char *)(extraout_ECX + 0x3a47a1);
}


/* CTimeInMapHelper::Init at 00626710 */

/* DWARF original prototype: bool Init(CTimeInMapHelper * this) */

bool __thiscall CTimeInMapHelper::Init(CTimeInMapHelper *this)

{
  this->m_szCurMapName[0] = '\0';
  return true;
}


/* CTimeInMapHelper::LevelInitPreEntity at 00626db0 */

/* DWARF original prototype: void LevelInitPreEntity(CTimeInMapHelper * this) */

void __thiscall CTimeInMapHelper::LevelInitPreEntity(CTimeInMapHelper *this)

{
  undefined4 *puVar1;
  int *piVar2;
  char *pcVar3;
  int iVar4;
  int unaff_EBX;
  longdouble lVar5;
  
  ___i686_get_pc_thunk_bx();
  puVar1 = *(undefined4 **)(unaff_EBX + 0x53cfe0);
  piVar2 = (int *)*puVar1;
  pcVar3 = (char *)(**(code **)(*piVar2 + 0xcc))(piVar2);
  iVar4 = V_strncmp(pcVar3,this->m_szCurMapName,0x104);
  if (iVar4 != 0) {
    lVar5 = (longdouble)_Plat_FloatTime();
    this->m_flTimeMapStart = (float)lVar5;
    piVar2 = (int *)*puVar1;
    pcVar3 = (char *)(**(code **)(*piVar2 + 0xcc))(piVar2);
    V_strncpy(this->m_szCurMapName,pcVar3,0x104);
  }
  return;
}


/* CSurveyPanel::WriteSurveyDataToFile at 00624740 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: bool WriteSurveyDataToFile(CSurveyPanel * this, char * file) */

bool __thiscall CSurveyPanel::WriteSurveyDataToFile(CSurveyPanel *this,char *file)

{
  float fVar1;
  code *pcVar2;
  bool bVar3;
  char cVar4;
  undefined4 uVar5;
  char *pcVar6;
  C_BasePlayer *this_00;
  int nItemID;
  int *piVar7;
  int iVar8;
  int unaff_EBX;
  longdouble lVar9;
  FileHandle_t handle;
  undefined4 local_2190;
  undefined4 local_218c;
  undefined4 local_2188;
  undefined4 local_2184;
  int local_2180;
  int local_217c;
  char local_2164 [8192];
  undefined1 local_164 [260];
  char local_60 [64];
  int local_20;
  
                    /* Unresolved local var: tm t@[???]
                       Unresolved local var: char[64] pszMap@[???]
                       Unresolved local var: C_Portal_Player * pPlayer@[???] */
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(&DAT_0053f60f + unaff_EBX);
  bVar3 = false;
  if (file != (char *)0x0) {
    uVar5 = (**(code **)(*(int *)(**(int **)(&DAT_0053f617 + unaff_EBX) + 4) + 8))
                      (**(int **)(&DAT_0053f617 + unaff_EBX) + 4,file,unaff_EBX + 0x38555f,0);
    cVar4 = (**(code **)(*(int *)**(undefined4 **)(&DAT_0053f617 + unaff_EBX) + 0x60))
                      ((int *)**(undefined4 **)(&DAT_0053f617 + unaff_EBX),uVar5);
    if (cVar4 == '\0') {
      _Warning(unaff_EBX + 0x3a6907,file);
      bVar3 = false;
    }
    else {
      _Plat_GetLocalTime(&local_2190);
      pcVar6 = (char *)(**(code **)(*(int *)**(undefined4 **)(&DAT_0053f653 + unaff_EBX) + 0xcc))
                                 ((int *)**(undefined4 **)(&DAT_0053f653 + unaff_EBX));
      V_FileBase(pcVar6,local_60,0x40);
      if ((this->id == -1) || (this->version == -1)) {
        _Warning(unaff_EBX + 0x3a692f,this->m_szSurveyName);
        bVar3 = false;
      }
      else {
        (**(code **)(*(int *)**(undefined4 **)(&DAT_0053f617 + unaff_EBX) + 0x6c))
                  ((int *)**(undefined4 **)(&DAT_0053f617 + unaff_EBX),uVar5,unaff_EBX + 0x3a6984,
                   this->id,this->version);
        this_00 = C_BasePlayer::GetLocalPlayer(-1);
        pcVar2 = *(code **)(*(int *)**(undefined4 **)(&DAT_0053f617 + unaff_EBX) + 0x6c);
        lVar9 = (longdouble)_Plat_FloatTime();
        fVar1 = *(float *)(unaff_EBX + 0x6b4ddb);
        pcVar6 = C_BasePlayer::GetPlayerName(this_00);
        (*pcVar2)(**(undefined4 **)(&DAT_0053f617 + unaff_EBX),uVar5,unaff_EBX + 0x3a698b,pcVar6,
                  local_60,local_217c + 0x76c,local_2180 + 1,local_2184,local_2188,local_218c,
                  local_2190,(double)((float)lVar9 - fVar1));
                    /* Unresolved local var: int i@[???] */
        nItemID = vgui::PanelListPanel::FirstItem(this->m_pQuestionsList);
                    /* Unresolved local var: CSurveyQuestion * pQuestion@[???]
                       Unresolved local var: char[260] szAnswerBuff@[???]
                       Unresolved local var: char[8192] szOutputBuff@[???] */
        while( true ) {
          iVar8 = vgui::PanelListPanel::InvalidItemID(this->m_pQuestionsList);
          if (iVar8 == nItemID) break;
          piVar7 = (int *)(*(this->m_pQuestionsList->super_Panel).super_IClientPanel.
                            _vptr_IClientPanel[0xd6])(this->m_pQuestionsList,nItemID);
          (**(code **)(*piVar7 + 0x3b8))(piVar7,local_164,0x104);
          V_snprintf(local_2164,0x2000,(char *)(unaff_EBX + 0x3a69b4),local_164);
          _Msg(local_2164);
          (**(code **)(*(int *)**(undefined4 **)(&DAT_0053f617 + unaff_EBX) + 0x6c))
                    ((int *)**(undefined4 **)(&DAT_0053f617 + unaff_EBX),uVar5,local_2164);
          nItemID = vgui::PanelListPanel::NextItem(this->m_pQuestionsList,nItemID);
        }
        (**(code **)(*(int *)**(undefined4 **)(&DAT_0053f617 + unaff_EBX) + 0x6c))
                  ((int *)**(undefined4 **)(&DAT_0053f617 + unaff_EBX),uVar5,unaff_EBX + 0x3a1583);
        (**(code **)(*(int *)(**(int **)(&DAT_0053f617 + unaff_EBX) + 4) + 0xc))
                  (**(int **)(&DAT_0053f617 + unaff_EBX) + 4,uVar5);
        bVar3 = true;
      }
    }
  }
  if (local_20 == **(int **)(&DAT_0053f60f + unaff_EBX)) {
    return bVar3;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}


/* __tcf_2 at 0097d590 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  *(int *)(extraout_ECX + 0x35bf88) = extraout_ECX + 0x1e8b10;
  IGameSystem::~IGameSystem((IGameSystem *)(extraout_ECX + 0x35bf88),in_stack_00000008);
  return;
}


/* CSurveyQuestion::ApplySchemeSettings at 006255b0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ApplySchemeSettings(CSurveyQuestion * this, IScheme * pScheme) */

void __thiscall CSurveyQuestion::ApplySchemeSettings(CSurveyQuestion *this,IScheme *pScheme)

{
  vgui::Panel::ApplySchemeSettings((Panel *)this,pScheme);
                    /* WARNING: Could not recover jumptable at 0x006255e0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel[0x3c])();
  return;
}


/* CEssayQuestion::ApplySchemeSettings at 00627420 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* DWARF original prototype: void ApplySchemeSettings(CEssayQuestion * this, IScheme * pScheme) */

void __thiscall CEssayQuestion::ApplySchemeSettings(CEssayQuestion *this,IScheme *pScheme)

{
  _func_int_varargs *p_Var1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  p_Var1 = (this->m_pAnswer->super_Panel).super_IClientPanel._vptr_IClientPanel[0x3d];
  iVar2 = (*(pScheme->super_IBaseInterface)._vptr_IBaseInterface[6])
                    (pScheme,unaff_EBX + 0x3896e6,0xffffffff);
  (*p_Var1)(this->m_pAnswer,iVar2);
  p_Var1 = (this->m_pAnswer->super_Panel).super_IClientPanel._vptr_IClientPanel[0x3c];
  iVar2 = (*(pScheme->super_IBaseInterface)._vptr_IBaseInterface[6])
                    (pScheme,unaff_EBX + 0x3a3e8b,0xff000000);
  (*p_Var1)(this->m_pAnswer,iVar2);
  vgui::Panel::ApplySchemeSettings((Panel *)this,pScheme);
                    /* WARNING: Could not recover jumptable at 0x006274e0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
    _vptr_IClientPanel[0x3c])();
  return;
}


/* CSurveyPanel::SetData at 00625ab0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void SetData(CSurveyPanel * this, KeyValues * data) */

void __thiscall CSurveyPanel::SetData(CSurveyPanel *this,KeyValues *data)

{
  _func_int_varargs **pp_Var1;
  char cVar2;
  _func_char_ptr *p_Var3;
  PanelListPanel *pPVar4;
  char *pcVar5;
  int iVar6;
  COneToTenQuestion *this_00;
  PanelKeyBindingMap *pPVar7;
  PanelKeyBindingMap *pPVar8;
  Label *pLVar9;
  int iVar10;
  RadioButton *this_01;
  int iVar11;
  CEssayQuestion *this_02;
  TextEntry *this_03;
  CRadioChoicesQuestion *this_04;
  PanelAnimationMap *pPVar12;
  PanelAnimationMap *pPVar13;
  PanelMessageMap *pPVar14;
  PanelMessageMap *pPVar15;
  int unaff_EBX;
  int iVar16;
  RadioButton **ppRVar17;
  TextEntry *this_1;
  RadioButton *in_stack_ffffff84;
  Panel *in_stack_ffffff88;
  char *in_stack_ffffff8c;
  char *in_stack_ffffff90;
  KeyValues *pQuestionEntry;
  int local_34;
  char local_1f [15];
  
  ___i686_get_pc_thunk_bx();
  pcVar5 = KeyValues::GetName(data);
  V_strncpy(this->m_szSurveyName,pcVar5,0x104);
  iVar6 = KeyValues::GetInt(data,(char *)(unaff_EBX + 0x36fd64),-1);
  this->id = iVar6;
  iVar6 = KeyValues::GetInt(data,(char *)(unaff_EBX + 0x37a6da),-1);
  this->version = iVar6;
  pQuestionEntry = KeyValues::GetFirstTrueSubKey(data);
  if (pQuestionEntry != (KeyValues *)0x0) {
    pp_Var1 = (_func_int_varargs **)(unaff_EBX + 0x5cfbea);
    p_Var3 = *(_func_char_ptr **)(unaff_EBX + 0x60805a);
    do {
      pcVar5 = KeyValues::GetName(pQuestionEntry);
      iVar6 = _V_stricmp(&UNK_003a5833 + unaff_EBX,pcVar5);
      if (iVar6 == 0) {
        this_00 = operator_new(0x1bc);
        pPVar4 = this->m_pQuestionsList;
        vgui::EditablePanel::EditablePanel
                  ((EditablePanel *)this_00,&pPVar4->super_Panel,&UNK_003a583c + unaff_EBX);
        (this_00->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
        _vptr_IClientPanel = pp_Var1;
        if (*(char *)(unaff_EBX + 0x63549a) == '\0') {
          *(undefined1 *)(unaff_EBX + 0x63549a) = 1;
          pPVar14 = vgui::FindOrAddPanelMessageMap(&UNK_003a53ce + unaff_EBX);
          pPVar14->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x608046);
          pPVar15 = vgui::FindOrAddPanelMessageMap(&UNK_0039f70a + unaff_EBX);
          pPVar14->baseMap = pPVar15;
        }
        if (*(char *)(unaff_EBX + 0x63549b) == '\0') {
          *(undefined1 *)(unaff_EBX + 0x63549b) = 1;
          pPVar12 = FindOrAddPanelAnimationMap(&UNK_003a53ce + unaff_EBX);
          pPVar12->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x608046);
          pPVar13 = FindOrAddPanelAnimationMap(&UNK_0039f70a + unaff_EBX);
          pPVar12->baseMap = pPVar13;
        }
        if (*(char *)(unaff_EBX + 0x63549c) == '\0') {
          *(undefined1 *)(unaff_EBX + 0x63549c) = 1;
          pPVar7 = vgui::FindOrAddPanelKeyBindingMap(&UNK_003a53ce + unaff_EBX);
          pPVar7->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x608046);
          pPVar8 = vgui::FindOrAddPanelKeyBindingMap(&UNK_0039f70a + unaff_EBX);
          pPVar7->baseMap = pPVar8;
        }
        (this_00->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
        _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cf04a);
        if (*(char *)(unaff_EBX + 0x63549d) == '\0') {
          *(undefined1 *)(unaff_EBX + 0x63549d) = 1;
          pPVar14 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3a5420));
          pPVar14->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x60803a);
          pPVar15 = vgui::FindOrAddPanelMessageMap(&UNK_003a53ce + unaff_EBX);
          pPVar14->baseMap = pPVar15;
        }
        if (*(char *)(unaff_EBX + 0x63549e) == '\0') {
          *(undefined1 *)(unaff_EBX + 0x63549e) = 1;
          pPVar12 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x3a5420));
          pPVar12->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x60803a);
          pPVar13 = FindOrAddPanelAnimationMap(&UNK_003a53ce + unaff_EBX);
          pPVar12->baseMap = pPVar13;
        }
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
        if (*(char *)(unaff_EBX + 0x63549f) == '\0') {
          *(undefined1 *)(unaff_EBX + 0x63549f) = 1;
          pPVar7 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3a5420));
          pPVar7->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x60803a);
          pPVar8 = vgui::FindOrAddPanelKeyBindingMap(&UNK_003a53ce + unaff_EBX);
          pPVar7->baseMap = pPVar8;
        }
        pLVar9 = operator_new(0x1a4);
        vgui::Label::Label(pLVar9,(Panel *)this_00,(char *)(unaff_EBX + 0x3a585b),
                           (char *)(unaff_EBX + 0x3a584d),(Label *)in_stack_ffffff84,
                           in_stack_ffffff88,in_stack_ffffff8c,in_stack_ffffff90);
        this_00->m_pQuestionText = pLVar9;
        pLVar9 = operator_new(0x1a4);
        vgui::Label::Label(pLVar9,(Panel *)this_00,(char *)(unaff_EBX + 0x3a587c),
                           (char *)(unaff_EBX + 0x3a5870),(Label *)in_stack_ffffff84,
                           in_stack_ffffff88,in_stack_ffffff8c,in_stack_ffffff90);
        this_00->m_pLabelAtOne = pLVar9;
        pLVar9 = operator_new(0x1a4);
        vgui::Label::Label(pLVar9,(Panel *)this_00,(char *)(unaff_EBX + 0x3a5892),
                           &UNK_003a5887 + unaff_EBX,(Label *)in_stack_ffffff84,in_stack_ffffff88,
                           in_stack_ffffff8c,in_stack_ffffff90);
        this_00->m_pLabelAtTen = pLVar9;
        (this_00->super_CSurveyQuestion).m_questiontype = SVQ_ONE_TO_TEN;
        in_stack_ffffff84 = (RadioButton *)0x0;
        vgui::EditablePanel::LoadControlSettings
                  ((EditablePanel *)this_00,(char *)(unaff_EBX + 0x3a589e),(char *)0x0,
                   (KeyValues *)0x0,(KeyValues *)0x0);
        iVar10 = vgui::Panel::GetWide(&pPVar4->super_Panel);
        local_34 = 0;
        iVar6 = 0;
        while( true ) {
          iVar16 = iVar6 + 1;
          V_snprintf(local_1f,3,(char *)(unaff_EBX + 0x376f66),iVar16);
          this_01 = operator_new(0x20c);
          vgui::RadioButton::RadioButton
                    (this_01,(Panel *)this_00,(char *)(unaff_EBX + 0x3a56dd),local_1f,
                     in_stack_ffffff84,in_stack_ffffff88,in_stack_ffffff8c,in_stack_ffffff90);
          this_00->m_pRadioButtons[iVar6] = this_01;
          iVar11 = vgui::Panel::GetTall((Panel *)this_00);
          vgui::Panel::SetPos((Panel *)this_00->m_pRadioButtons[iVar6],local_34,iVar11 + -0x30);
          if (iVar16 == 10) break;
          local_34 = local_34 + iVar10 / 10;
          iVar6 = iVar16;
        }
        COneToTenQuestion::SetData(this_00,pQuestionEntry);
        (*(this->m_pQuestionsList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd4])
                  (this->m_pQuestionsList,0,this_00);
      }
      else {
        pcVar5 = KeyValues::GetName(pQuestionEntry);
        iVar6 = _V_stricmp((char *)(unaff_EBX + 0x3a58c2),pcVar5);
        if (iVar6 == 0) {
          this_02 = operator_new(400);
          vgui::EditablePanel::EditablePanel
                    ((EditablePanel *)this_02,&this->m_pQuestionsList->super_Panel,
                     (char *)(unaff_EBX + 0x3a58c8));
          (this_02->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
          _vptr_IClientPanel = pp_Var1;
          if (*(char *)(unaff_EBX + 0x63549a) == '\0') {
            *(undefined1 *)(unaff_EBX + 0x63549a) = 1;
            pPVar14 = vgui::FindOrAddPanelMessageMap(&UNK_003a53ce + unaff_EBX);
            pPVar14->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x608046);
            pPVar15 = vgui::FindOrAddPanelMessageMap(&UNK_0039f70a + unaff_EBX);
            pPVar14->baseMap = pPVar15;
          }
          if (*(char *)(unaff_EBX + 0x63549b) == '\0') {
            *(undefined1 *)(unaff_EBX + 0x63549b) = 1;
            pPVar12 = FindOrAddPanelAnimationMap(&UNK_003a53ce + unaff_EBX);
            pPVar12->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x608046);
            pPVar13 = FindOrAddPanelAnimationMap(&UNK_0039f70a + unaff_EBX);
            pPVar12->baseMap = pPVar13;
          }
          if (*(char *)(unaff_EBX + 0x63549c) == '\0') {
            *(undefined1 *)(unaff_EBX + 0x63549c) = 1;
            pPVar7 = vgui::FindOrAddPanelKeyBindingMap(&UNK_003a53ce + unaff_EBX);
            pPVar7->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x608046);
            pPVar8 = vgui::FindOrAddPanelKeyBindingMap(&UNK_0039f70a + unaff_EBX);
            pPVar7->baseMap = pPVar8;
          }
          (this_02->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
          _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cf80a);
          if (*(char *)(unaff_EBX + 0x6354a0) == '\0') {
            *(undefined1 *)(unaff_EBX + 0x6354a0) = 1;
            pPVar14 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3a53fb));
            pPVar14->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x608056);
            pPVar15 = vgui::FindOrAddPanelMessageMap(&UNK_003a53ce + unaff_EBX);
            pPVar14->baseMap = pPVar15;
            cVar2 = *(char *)(unaff_EBX + 0x6354a1);
          }
          else {
            cVar2 = *(char *)(unaff_EBX + 0x6354a1);
          }
          if (cVar2 == '\0') {
            *(undefined1 *)(unaff_EBX + 0x6354a1) = 1;
            pPVar12 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x3a53fb));
            pPVar12->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x608056);
            pPVar13 = FindOrAddPanelAnimationMap(&UNK_003a53ce + unaff_EBX);
            pPVar12->baseMap = pPVar13;
          }
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
          if (*(char *)(unaff_EBX + 0x6354a2) == '\0') {
            *(undefined1 *)(unaff_EBX + 0x6354a2) = 1;
            pPVar7 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3a53fb));
            pPVar7->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x608056);
            pPVar8 = vgui::FindOrAddPanelKeyBindingMap(&UNK_003a53ce + unaff_EBX);
            pPVar7->baseMap = pPVar8;
          }
          pLVar9 = operator_new(0x1a4);
          this_1 = (TextEntry *)(unaff_EBX + 0x3a58d6);
          vgui::Label::Label(pLVar9,(Panel *)this_02,(char *)(unaff_EBX + 0x3a58ea),(char *)this_1,
                             (Label *)in_stack_ffffff84,in_stack_ffffff88,in_stack_ffffff8c,
                             in_stack_ffffff90);
          this_02->m_pQuestionText = pLVar9;
          this_03 = operator_new(0x63c);
          vgui::TextEntry::TextEntry
                    (this_03,(Panel *)this_02,(char *)(unaff_EBX + 0x3a58fc),this_1,
                     (Panel *)in_stack_ffffff84,(char *)in_stack_ffffff88);
          this_02->m_pAnswer = this_03;
          (*(this_03->super_Panel).super_IClientPanel._vptr_IClientPanel[0x109])(this_03,1);
          (*(this_02->m_pAnswer->super_Panel).super_IClientPanel._vptr_IClientPanel[0xff])
                    (this_02->m_pAnswer,1);
          (*(this_02->m_pAnswer->super_Panel).super_IClientPanel._vptr_IClientPanel[0x102])
                    (this_02->m_pAnswer,1);
          (*(this_02->m_pAnswer->super_Panel).super_IClientPanel._vptr_IClientPanel[0x106])
                    (this_02->m_pAnswer,0x2000);
          (this_02->super_CSurveyQuestion).m_questiontype = SVQ_ESSAY;
          in_stack_ffffff84 = (RadioButton *)0x0;
          vgui::EditablePanel::LoadControlSettings
                    ((EditablePanel *)this_02,(char *)(unaff_EBX + 0x3a5903),(char *)0x0,
                     (KeyValues *)0x0,(KeyValues *)0x0);
          CEssayQuestion::SetData(this_02,pQuestionEntry);
          (*(this->m_pQuestionsList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd4])
                    (this->m_pQuestionsList,0,this_02);
        }
        else {
          pcVar5 = KeyValues::GetName(pQuestionEntry);
          iVar6 = _V_stricmp(&UNK_003a5921 + unaff_EBX,pcVar5);
          if (iVar6 == 0) {
            this_04 = operator_new(0x1b8);
            pPVar4 = this->m_pQuestionsList;
            vgui::EditablePanel::EditablePanel
                      ((EditablePanel *)this_04,&pPVar4->super_Panel,(char *)(unaff_EBX + 0x3a58c8))
            ;
            (this_04->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
            _vptr_IClientPanel = pp_Var1;
            if (*(char *)(unaff_EBX + 0x63549a) == '\0') {
              *(undefined1 *)(unaff_EBX + 0x63549a) = 1;
              pPVar14 = vgui::FindOrAddPanelMessageMap(&UNK_003a53ce + unaff_EBX);
              pPVar14->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x608046);
              pPVar15 = vgui::FindOrAddPanelMessageMap(&UNK_0039f70a + unaff_EBX);
              pPVar14->baseMap = pPVar15;
            }
            if (*(char *)(unaff_EBX + 0x63549b) == '\0') {
              *(undefined1 *)(unaff_EBX + 0x63549b) = 1;
              pPVar12 = FindOrAddPanelAnimationMap(&UNK_003a53ce + unaff_EBX);
              pPVar12->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x608046);
              pPVar13 = FindOrAddPanelAnimationMap(&UNK_0039f70a + unaff_EBX);
              pPVar12->baseMap = pPVar13;
            }
            if (*(char *)(unaff_EBX + 0x63549c) == '\0') {
              *(undefined1 *)(unaff_EBX + 0x63549c) = 1;
              pPVar7 = vgui::FindOrAddPanelKeyBindingMap(&UNK_003a53ce + unaff_EBX);
              pPVar7->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x608046);
              pPVar8 = vgui::FindOrAddPanelKeyBindingMap(&UNK_0039f70a + unaff_EBX);
              pPVar7->baseMap = pPVar8;
            }
            (this_04->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
            _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cf42a);
            if (*(char *)(unaff_EBX + 0x6354a3) == '\0') {
              *(undefined1 *)(unaff_EBX + 0x6354a3) = 1;
              pPVar14 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3a540a));
              pPVar14->pfnClassName = p_Var3;
              pPVar15 = vgui::FindOrAddPanelMessageMap(&UNK_003a53ce + unaff_EBX);
              pPVar14->baseMap = pPVar15;
            }
            if (*(char *)(unaff_EBX + 0x6354a4) == '\0') {
              *(undefined1 *)(unaff_EBX + 0x6354a4) = 1;
              pPVar12 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x3a540a));
              pPVar12->pfnClassName = p_Var3;
              pPVar13 = FindOrAddPanelAnimationMap(&UNK_003a53ce + unaff_EBX);
              pPVar12->baseMap = pPVar13;
            }
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
            if (*(char *)(unaff_EBX + 0x6354a5) == '\0') {
              *(undefined1 *)(unaff_EBX + 0x6354a5) = 1;
              pPVar7 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3a540a));
              pPVar7->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x60805a);
              pPVar8 = vgui::FindOrAddPanelKeyBindingMap(&UNK_003a53ce + unaff_EBX);
              pPVar7->baseMap = pPVar8;
            }
            pLVar9 = operator_new(0x1a4);
            vgui::Label::Label(pLVar9,(Panel *)this_04,&UNK_003a594a + unaff_EBX,
                               &UNK_003a592e + unaff_EBX,(Label *)in_stack_ffffff84,
                               in_stack_ffffff88,in_stack_ffffff8c,in_stack_ffffff90);
            this_04->m_pQuestionText = pLVar9;
            iVar6 = vgui::Panel::GetWide(&pPVar4->super_Panel);
            vgui::Panel::SetSize((Panel *)this_04,iVar6,0x40);
            in_stack_ffffff84 = (RadioButton *)0x0;
            vgui::EditablePanel::LoadControlSettings
                      ((EditablePanel *)this_04,(char *)(unaff_EBX + 0x3a5966),(char *)0x0,
                       (KeyValues *)0x0,(KeyValues *)0x0);
            (this_04->super_CSurveyQuestion).m_questiontype = SVQ_RADIOCHOICES;
            this_04->m_iNumRadioButtons = 0;
            ppRVar17 = this_04->m_pRadioButtons;
            for (iVar6 = 10; iVar6 != 0; iVar6 = iVar6 + -1) {
              *ppRVar17 = (RadioButton *)0x0;
              ppRVar17 = ppRVar17 + 1;
            }
            CRadioChoicesQuestion::SetData(this_04,pQuestionEntry);
            (*(this->m_pQuestionsList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd4])
                      (this->m_pQuestionsList,0,this_04);
          }
        }
      }
      pQuestionEntry = KeyValues::GetNextTrueSubKey(pQuestionEntry);
    } while (pQuestionEntry != (KeyValues *)0x0);
  }
  (*(this->super_Frame).super_EditablePanel.super_Panel.super_IClientPanel._vptr_IClientPanel[0x43])
            (this,1,1);
  return;
}


/* CEssayQuestion::GetPanelClassName at 00626730 */

char * CEssayQuestion::GetPanelClassName(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (char *)(extraout_ECX + 0x3a4781);
}


/* CEssayQuestion::KB_ChainToMap at 006268d0 */

void CEssayQuestion::KB_ChainToMap(void)

{
  PanelKeyBindingMap *pPVar1;
  PanelKeyBindingMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&DAT_0063467f)[unaff_EBX] == '\0') {
    (&DAT_0063467f)[unaff_EBX] = 1;
    pPVar1 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3a45d8));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x607233);
    pPVar2 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3a45ab));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CEssayQuestion::ChainToAnimationMap at 00626ab0 */

void CEssayQuestion::ChainToAnimationMap(void)

{
  PanelAnimationMap *pPVar1;
  PanelAnimationMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelAnimationMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x63449e) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x63449e) = 1;
    pPVar1 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x3a43f8));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x607053);
    pPVar2 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x3a43cb));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CEssayQuestion::ChainToMap at 00626c90 */

void CEssayQuestion::ChainToMap(void)

{
  PanelMessageMap *pPVar1;
  PanelMessageMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelMessageMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x6342bd) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x6342bd) = 1;
    pPVar1 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3a4218));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x606e73);
    pPVar2 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3a41eb));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CEssayQuestion::GetKBMap at 00627100 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* DWARF original prototype: PanelKeyBindingMap * GetKBMap(CEssayQuestion * this) */

PanelKeyBindingMap * __thiscall CEssayQuestion::GetKBMap(CEssayQuestion *this)

{
  int iVar1;
  PanelKeyBindingMap *pPVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x633dec) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0x633dec);
    if (iVar1 != 0) {
      pPVar2 = vgui::FindOrAddPanelKeyBindingMap(&UNK_003a3da5 + unaff_EBX);
      *(PanelKeyBindingMap **)(unaff_EBX + 0x633e24) = pPVar2;
      ___cxa_guard_release(unaff_EBX + 0x633dec);
    }
  }
  return *(PanelKeyBindingMap **)(unaff_EBX + 0x633e24);
}


/* CEssayQuestion::GetAnimMap at 00627180 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* DWARF original prototype: PanelAnimationMap * GetAnimMap(CEssayQuestion * this) */

PanelAnimationMap * __thiscall CEssayQuestion::GetAnimMap(CEssayQuestion *this)

{
  PanelAnimationMap *pPVar1;
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  pPVar1 = FindOrAddPanelAnimationMap((char *)(extraout_ECX + 0x3a3d31));
  return pPVar1;
}


/* CEssayQuestion::GetMessageMap at 006271a0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* DWARF original prototype: PanelMessageMap * GetMessageMap(CEssayQuestion * this) */

PanelMessageMap * __thiscall CEssayQuestion::GetMessageMap(CEssayQuestion *this)

{
  int iVar1;
  PanelMessageMap *pPVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x633d44) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0x633d44);
    if (iVar1 != 0) {
      pPVar2 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3a3d05));
      *(PanelMessageMap **)(unaff_EBX + 0x633d88) = pPVar2;
      ___cxa_guard_release(unaff_EBX + 0x633d44);
    }
  }
  return *(PanelMessageMap **)(unaff_EBX + 0x633d88);
}


/* CEssayQuestion::~CEssayQuestion at 00627aa0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* DWARF original prototype: void ~CEssayQuestion(CEssayQuestion * this, int __in_chrg) */

void __thiscall CEssayQuestion::~CEssayQuestion(CEssayQuestion *this,int __in_chrg)

{
  Label *pLVar1;
  TextEntry *pTVar2;
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cd814);
  pLVar1 = this->m_pQuestionText;
  if (pLVar1 != (Label *)0x0) {
    (*(pLVar1->super_Panel).super_IClientPanel._vptr_IClientPanel[0x21])(pLVar1);
  }
  pTVar2 = this->m_pAnswer;
  if (pTVar2 != (TextEntry *)0x0) {
    (*(pTVar2->super_Panel).super_IClientPanel._vptr_IClientPanel[0x21])(pTVar2);
  }
  (this->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cdbf4);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,in_stack_ffffffd8);
  operator_delete(this);
  return;
}


/* CEssayQuestion::~CEssayQuestion at 00627b30 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* DWARF original prototype: void ~CEssayQuestion(CEssayQuestion * this, int __in_chrg) */

void __thiscall CEssayQuestion::~CEssayQuestion(CEssayQuestion *this,int __in_chrg)

{
  Label *pLVar1;
  TextEntry *pTVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cd784);
  pLVar1 = this->m_pQuestionText;
  if (pLVar1 != (Label *)0x0) {
    (*(pLVar1->super_Panel).super_IClientPanel._vptr_IClientPanel[0x21])(pLVar1);
  }
  pTVar2 = this->m_pAnswer;
  if (pTVar2 != (TextEntry *)0x0) {
    (*(pTVar2->super_Panel).super_IClientPanel._vptr_IClientPanel[0x21])(pTVar2);
  }
  (this->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(&UNK_005cdb64 + unaff_EBX);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,__in_chrg);
  return;
}


/* CEssayQuestion::SetData at 006252a0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* DWARF original prototype: bool SetData(CEssayQuestion * this, KeyValues * pKeys) */

bool __thiscall CEssayQuestion::SetData(CEssayQuestion *this,KeyValues *pKeys)

{
  _func_int_varargs *p_Var1;
  KeyValues *this_00;
  char *pcVar2;
  int iVar3;
  int unaff_EBX;
  
                    /* Unresolved local var: KeyValues * pQuestionData@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = KeyValues::GetFirstSubKey(pKeys);
  if (this_00 != (KeyValues *)0x0) {
    do {
      while( true ) {
        pcVar2 = KeyValues::GetName(this_00);
        iVar3 = _V_stricmp((char *)(unaff_EBX + 0x38c412),pcVar2);
        if (iVar3 != 0) break;
        p_Var1 = (this->m_pQuestionText->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd4];
        pcVar2 = KeyValues::GetString(this_00,(char *)0x0,(char *)(unaff_EBX + 0x3639b6));
        (*p_Var1)(this->m_pQuestionText,pcVar2);
        this_00 = KeyValues::GetNextKey(this_00);
        if (this_00 == (KeyValues *)0x0) {
          return true;
        }
      }
      this_00 = KeyValues::GetNextKey(this_00);
    } while (this_00 != (KeyValues *)0x0);
  }
  return true;
}


/* CEssayQuestion::GetQuestionText at 00624160 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* DWARF original prototype: void GetQuestionText(CEssayQuestion * this, char * pBuffOut, int
   iBuffLen) */

void __thiscall CEssayQuestion::GetQuestionText(CEssayQuestion *this,char *pBuffOut,int iBuffLen)

{
                    /* WARNING: Could not recover jumptable at 0x00624178. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->m_pQuestionText->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd6])();
  return;
}


/* CEssayQuestion::GetAnswerText at 00624180 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* DWARF original prototype: void GetAnswerText(CEssayQuestion * this, char * pBuffOut, int
   iBuffLen) */

void __thiscall CEssayQuestion::GetAnswerText(CEssayQuestion *this,char *pBuffOut,int iBuffLen)

{
                    /* WARNING: Could not recover jumptable at 0x00624198. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->m_pAnswer->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd6])();
  return;
}


/* CRadioChoicesQuestion::GetPanelClassName at 00626740 */

char * CRadioChoicesQuestion::GetPanelClassName(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (char *)(extraout_ECX + 0x3a4780);
}


/* CRadioChoicesQuestion::KB_ChainToMap at 00626870 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* WARNING: Struct "QueryBox": ignoring multiple overlapping fields */
/* WARNING: Struct "CCvarTextEntry": ignoring multiple overlapping fields */

void CRadioChoicesQuestion::KB_ChainToMap(void)

{
  PanelKeyBindingMap *pPVar1;
  PanelKeyBindingMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (CPlayerPositionProxy::~CPlayerPositionProxy[unaff_EBX + 2] == (code)0x0) {
    CPlayerPositionProxy::~CPlayerPositionProxy[unaff_EBX + 2] = (code)0x1;
    pPVar1 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3a4647));
    pPVar1->pfnClassName = *(_func_char_ptr **)(CTextEntryBox::PerformLayout + unaff_EBX + 7);
    pPVar2 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3a460b));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CRadioChoicesQuestion::ChainToAnimationMap at 00626a50 */

void CRadioChoicesQuestion::ChainToAnimationMap(void)

{
  PanelAnimationMap *pPVar1;
  PanelAnimationMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelAnimationMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x634501) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x634501) = 1;
    pPVar1 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x3a4467));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x6070b7);
    pPVar2 = FindOrAddPanelAnimationMap(&UNK_003a442b + unaff_EBX);
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CRadioChoicesQuestion::ChainToMap at 00626c30 */

void CRadioChoicesQuestion::ChainToMap(void)

{
  PanelMessageMap *pPVar1;
  PanelMessageMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelMessageMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x634320) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x634320) = 1;
    pPVar1 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3a4287));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x606ed7);
    pPVar2 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3a424b));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CRadioChoicesQuestion::GetKBMap at 00627220 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: PanelKeyBindingMap * GetKBMap(CRadioChoicesQuestion * this) */

PanelKeyBindingMap * __thiscall CRadioChoicesQuestion::GetKBMap(CRadioChoicesQuestion *this)

{
  int iVar1;
  PanelKeyBindingMap *pPVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x633cdc) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0x633cdc);
    if (iVar1 != 0) {
      pPVar2 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3a3c94));
      *(PanelKeyBindingMap **)(unaff_EBX + 0x633d0c) = pPVar2;
      ___cxa_guard_release(unaff_EBX + 0x633cdc);
    }
  }
  return *(PanelKeyBindingMap **)(unaff_EBX + 0x633d0c);
}


/* CRadioChoicesQuestion::GetAnimMap at 006272a0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: PanelAnimationMap * GetAnimMap(CRadioChoicesQuestion * this) */

PanelAnimationMap * __thiscall CRadioChoicesQuestion::GetAnimMap(CRadioChoicesQuestion *this)

{
  PanelAnimationMap *pPVar1;
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  pPVar1 = FindOrAddPanelAnimationMap((char *)(extraout_ECX + 0x3a3c20));
  return pPVar1;
}


/* CRadioChoicesQuestion::GetMessageMap at 006272c0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: PanelMessageMap * GetMessageMap(CRadioChoicesQuestion * this) */

PanelMessageMap * __thiscall CRadioChoicesQuestion::GetMessageMap(CRadioChoicesQuestion *this)

{
  int iVar1;
  PanelMessageMap *pPVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x633c34) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0x633c34);
    if (iVar1 != 0) {
      pPVar2 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3a3bf4));
      *(PanelMessageMap **)(CPlayerProximityProxy::OnBind + unaff_EBX) = pPVar2;
      ___cxa_guard_release(unaff_EBX + 0x633c34);
    }
  }
  return *(PanelMessageMap **)(CPlayerProximityProxy::OnBind + unaff_EBX);
}


/* CRadioChoicesQuestion::~CRadioChoicesQuestion at 00627810 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void ~CRadioChoicesQuestion(CRadioChoicesQuestion * this, int
   __in_chrg) */

void __thiscall
CRadioChoicesQuestion::~CRadioChoicesQuestion(CRadioChoicesQuestion *this,int __in_chrg)

{
  Label *pLVar1;
  RadioButton *pRVar2;
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cd6ca);
  pLVar1 = this->m_pQuestionText;
  if (pLVar1 != (Label *)0x0) {
    (*(pLVar1->super_Panel).super_IClientPanel._vptr_IClientPanel[0x21])(pLVar1);
  }
                    /* Unresolved local var: int i@[???] */
  pRVar2 = this->m_pRadioButtons[0];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[1];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[2];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[3];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[4];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[5];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[6];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[7];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[8];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[9];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  (this->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cde8a);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,in_stack_ffffffd8);
  operator_delete(this);
  return;
}


/* CRadioChoicesQuestion::~CRadioChoicesQuestion at 00627960 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void ~CRadioChoicesQuestion(CRadioChoicesQuestion * this, int
   __in_chrg) */

void __thiscall
CRadioChoicesQuestion::~CRadioChoicesQuestion(CRadioChoicesQuestion *this,int __in_chrg)

{
  Label *pLVar1;
  RadioButton *pRVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cd57a);
  pLVar1 = this->m_pQuestionText;
  if (pLVar1 != (Label *)0x0) {
    (*(pLVar1->super_Panel).super_IClientPanel._vptr_IClientPanel[0x21])(pLVar1);
  }
                    /* Unresolved local var: int i@[???] */
  pRVar2 = this->m_pRadioButtons[0];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[1];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[2];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[3];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[4];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[5];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[6];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[7];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[8];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[9];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  (this->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cdd3a);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,__in_chrg);
  return;
}


/* CRadioChoicesQuestion::SetData at 00625360 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: bool SetData(CRadioChoicesQuestion * this, KeyValues * pKeys) */

bool __thiscall CRadioChoicesQuestion::SetData(CRadioChoicesQuestion *this,KeyValues *pKeys)

{
  _func_int_varargs *p_Var1;
  KeyValues *this_00;
  char *pcVar2;
  int iVar3;
  RadioButton *this_01;
  Panel *this_02;
  int unaff_EBX;
  char *this_1;
  Panel *parent_1;
  int *x;
  int *y;
  int local_24;
  int local_20 [4];
  
                    /* Unresolved local var: KeyValues * pQuestionData@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = KeyValues::GetFirstSubKey(pKeys);
  if (this_00 != (KeyValues *)0x0) {
    y = local_20;
    x = &local_24;
    parent_1 = (Panel *)(unaff_EBX + 0x3638f6);
    this_1 = &UNK_003a5e2d + unaff_EBX;
LAB_0062540a:
    do {
      pcVar2 = KeyValues::GetName(this_00);
      iVar3 = _V_stricmp((char *)(unaff_EBX + 0x38c352),pcVar2);
      if (iVar3 == 0) {
        p_Var1 = (this->m_pQuestionText->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd4];
        pcVar2 = KeyValues::GetString(this_00,(char *)0x0,(char *)parent_1);
        (*p_Var1)(this->m_pQuestionText,pcVar2);
      }
      else {
        pcVar2 = KeyValues::GetName(this_00);
        iVar3 = _V_stricmp((char *)(unaff_EBX + 0x3a5e26),pcVar2);
        if ((iVar3 == 0) && (this->m_iNumRadioButtons < 10)) {
          local_24 = 0x10;
          vgui::Panel::GetPos(&this->m_pQuestionText->super_Panel,x,y);
          iVar3 = this->m_iNumRadioButtons;
          pcVar2 = KeyValues::GetString(this_00,(char *)0x0,(char *)parent_1);
          this_01 = operator_new(0x20c);
          vgui::RadioButton::RadioButton
                    (this_01,(Panel *)this,this_1,pcVar2,(RadioButton *)this_1,parent_1,(char *)x,
                     (char *)y);
          this->m_pRadioButtons[iVar3] = this_01;
          vgui::Panel::SetPos((Panel *)this->m_pRadioButtons[this->m_iNumRadioButtons],local_24,
                              (this->m_iNumRadioButtons * 3 + 3) * 8);
          this_02 = (Panel *)(*(this->super_CSurveyQuestion).super_EditablePanel.super_Panel.
                               super_IClientPanel._vptr_IClientPanel[0x27])(this);
          iVar3 = vgui::Panel::GetWide(this_02);
          vgui::Panel::SetSize((Panel *)this->m_pRadioButtons[this->m_iNumRadioButtons],iVar3,0x18);
          this->m_iNumRadioButtons = this->m_iNumRadioButtons + 1;
          this_00 = KeyValues::GetNextKey(this_00);
          if (this_00 == (KeyValues *)0x0) break;
          goto LAB_0062540a;
        }
      }
      this_00 = KeyValues::GetNextKey(this_00);
    } while (this_00 != (KeyValues *)0x0);
  }
  vgui::Panel::SetTall((Panel *)this,this->m_iNumRadioButtons * 0x18 + 0x30);
  (*(this->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
    _vptr_IClientPanel[0x43])(this,1,1);
  return true;
}


/* CRadioChoicesQuestion::GetQuestionText at 006241a0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void GetQuestionText(CRadioChoicesQuestion * this, char * pBuffOut, int
   iBuffLen) */

void __thiscall
CRadioChoicesQuestion::GetQuestionText(CRadioChoicesQuestion *this,char *pBuffOut,int iBuffLen)

{
                    /* WARNING: Could not recover jumptable at 0x006241b8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->m_pQuestionText->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd6])();
  return;
}


/* CRadioChoicesQuestion::GetAnswerText at 00624410 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void GetAnswerText(CRadioChoicesQuestion * this, char * pBuffOut, int
   iBuffLen) */

void __thiscall
CRadioChoicesQuestion::GetAnswerText(CRadioChoicesQuestion *this,char *pBuffOut,int iBuffLen)

{
  RadioButton *pRVar1;
  char cVar2;
  int unaff_EBX;
  int iVar3;
  
                    /* Unresolved local var: bool bFound@[???] */
  ___i686_get_pc_thunk_bx();
  if (0 < this->m_iNumRadioButtons) {
    iVar3 = 0;
    do {
      pRVar1 = this->m_pRadioButtons[iVar3];
      if (pRVar1 != (RadioButton *)0x0) {
        cVar2 = (**(code **)(*(int *)&(pRVar1->super_ToggleButton).super_Button.super_Label.
                                      super_Panel.super_IClientPanel + 0x404))(pRVar1);
        if (cVar2 != '\0') {
                    /* WARNING: Could not recover jumptable at 0x006244a9. Too many branches */
                    /* WARNING: Treating indirect jump as call */
          (**(code **)(*(int *)&(this->m_pRadioButtons[iVar3]->super_ToggleButton).super_Button.
                                super_Label.super_Panel.super_IClientPanel + 0x358))();
          return;
        }
      }
      iVar3 = iVar3 + 1;
    } while (iVar3 < this->m_iNumRadioButtons);
  }
  V_strncpy(pBuffOut,*(char **)(unaff_EBX + 0x5d16ae),iBuffLen);
  return;
}


/* COneToTenQuestion::GetPanelClassName at 00626750 */

char * COneToTenQuestion::GetPanelClassName(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (char *)(extraout_ECX + 0x3a4786);
}


/* COneToTenQuestion::KB_ChainToMap at 00626810 */

void COneToTenQuestion::KB_ChainToMap(void)

{
  PanelKeyBindingMap *pPVar1;
  PanelKeyBindingMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&DAT_0063473c)[unaff_EBX] == '\0') {
    (&DAT_0063473c)[unaff_EBX] = 1;
    pPVar1 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3a46bd));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x6072d7);
    pPVar2 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3a466b));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* COneToTenQuestion::ChainToAnimationMap at 006269f0 */

void COneToTenQuestion::ChainToAnimationMap(void)

{
  PanelAnimationMap *pPVar1;
  PanelAnimationMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelAnimationMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&DAT_0063455b)[unaff_EBX] == '\0') {
    (&DAT_0063455b)[unaff_EBX] = 1;
    pPVar1 = FindOrAddPanelAnimationMap(&UNK_003a44dd + unaff_EBX);
    pPVar1->pfnClassName = *(_func_char_ptr **)(&DAT_006070f7 + unaff_EBX);
    pPVar2 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x3a448b));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* COneToTenQuestion::ChainToMap at 00626bd0 */

void COneToTenQuestion::ChainToMap(void)

{
  PanelMessageMap *pPVar1;
  PanelMessageMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelMessageMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x63437a) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x63437a) = 1;
    pPVar1 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3a42fd));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x606f17);
    pPVar2 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3a42ab));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* COneToTenQuestion::GetMessageMap at 00626e40 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: PanelMessageMap * GetMessageMap(COneToTenQuestion * this) */

PanelMessageMap * __thiscall COneToTenQuestion::GetMessageMap(COneToTenQuestion *this)

{
  int iVar1;
  PanelMessageMap *pPVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x6340c4) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0x6340c4);
    if (iVar1 != 0) {
      pPVar2 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3a408a));
      *(PanelMessageMap **)(unaff_EBX + 0x6340f8) = pPVar2;
      ___cxa_guard_release(unaff_EBX + 0x6340c4);
    }
  }
  return *(PanelMessageMap **)(unaff_EBX + 0x6340f8);
}


/* COneToTenQuestion::GetKBMap at 00627340 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: PanelKeyBindingMap * GetKBMap(COneToTenQuestion * this) */

PanelKeyBindingMap * __thiscall COneToTenQuestion::GetKBMap(COneToTenQuestion *this)

{
  int iVar1;
  PanelKeyBindingMap *pPVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((&DAT_00633bcc)[unaff_EBX] == '\0') {
    iVar1 = ___cxa_guard_acquire(&DAT_00633bcc + unaff_EBX);
    if (iVar1 != 0) {
      pPVar2 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3a3b8a));
      *(PanelKeyBindingMap **)(unaff_EBX + 0x633bf4) = pPVar2;
      ___cxa_guard_release(&DAT_00633bcc + unaff_EBX);
    }
  }
  return *(PanelKeyBindingMap **)(unaff_EBX + 0x633bf4);
}


/* COneToTenQuestion::GetAnimMap at 006273c0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: PanelAnimationMap * GetAnimMap(COneToTenQuestion * this) */

PanelAnimationMap * __thiscall COneToTenQuestion::GetAnimMap(COneToTenQuestion *this)

{
  PanelAnimationMap *pPVar1;
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  pPVar1 = FindOrAddPanelAnimationMap((char *)(extraout_ECX + 0x3a3b16));
  return pPVar1;
}


/* COneToTenQuestion::~COneToTenQuestion at 00627530 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void ~COneToTenQuestion(COneToTenQuestion * this, int __in_chrg) */

void __thiscall COneToTenQuestion::~COneToTenQuestion(COneToTenQuestion *this,int __in_chrg)

{
  Label *pLVar1;
  RadioButton *pRVar2;
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cd5ca);
  pLVar1 = this->m_pQuestionText;
  if (pLVar1 != (Label *)0x0) {
    (*(pLVar1->super_Panel).super_IClientPanel._vptr_IClientPanel[0x21])(pLVar1);
  }
  pLVar1 = this->m_pLabelAtOne;
  if (pLVar1 != (Label *)0x0) {
    (*(pLVar1->super_Panel).super_IClientPanel._vptr_IClientPanel[0x21])(pLVar1);
  }
  pLVar1 = this->m_pLabelAtTen;
  if (pLVar1 != (Label *)0x0) {
    (*(pLVar1->super_Panel).super_IClientPanel._vptr_IClientPanel[0x21])(pLVar1);
  }
                    /* Unresolved local var: int i@[???] */
  pRVar2 = this->m_pRadioButtons[0];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[1];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[2];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[3];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[4];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[5];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[6];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[7];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[8];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[9];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  (this->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5ce16a);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,in_stack_ffffffd8);
  operator_delete(this);
  return;
}


/* COneToTenQuestion::~COneToTenQuestion at 006276a0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void ~COneToTenQuestion(COneToTenQuestion * this, int __in_chrg,
   COneToTenQuestion * this) */

void __thiscall
COneToTenQuestion::~COneToTenQuestion
          (COneToTenQuestion *this,int __in_chrg,COneToTenQuestion *this_1)

{
  Label *pLVar1;
  RadioButton *pRVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cd45a);
  pLVar1 = this->m_pQuestionText;
  if (pLVar1 != (Label *)0x0) {
    (*(pLVar1->super_Panel).super_IClientPanel._vptr_IClientPanel[0x21])(pLVar1);
  }
  pLVar1 = this->m_pLabelAtOne;
  if (pLVar1 != (Label *)0x0) {
    (*(pLVar1->super_Panel).super_IClientPanel._vptr_IClientPanel[0x21])(pLVar1);
  }
  pLVar1 = this->m_pLabelAtTen;
  if (pLVar1 != (Label *)0x0) {
    (*(pLVar1->super_Panel).super_IClientPanel._vptr_IClientPanel[0x21])(pLVar1);
  }
                    /* Unresolved local var: int i@[???] */
  pRVar2 = this->m_pRadioButtons[0];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[1];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[2];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[3];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[4];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[5];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[6];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[7];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[8];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  pRVar2 = this->m_pRadioButtons[9];
  if (pRVar2 != (RadioButton *)0x0) {
    (*(pRVar2->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(pRVar2);
  }
  (this->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cdffa);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,__in_chrg);
  return;
}


/* COneToTenQuestion::SetData at 00625120 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: bool SetData(COneToTenQuestion * this, KeyValues * pKeys) */

bool __thiscall COneToTenQuestion::SetData(COneToTenQuestion *this,KeyValues *pKeys)

{
  char *defaultValue;
  _func_int_varargs *p_Var1;
  KeyValues *this_00;
  char *pcVar2;
  int iVar3;
  int unaff_EBX;
  
                    /* Unresolved local var: KeyValues * pQuestionData@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = KeyValues::GetFirstSubKey(pKeys);
  if (this_00 != (KeyValues *)0x0) {
    defaultValue = (char *)(unaff_EBX + 0x363b36);
    do {
      while( true ) {
        pcVar2 = KeyValues::GetName(this_00);
        iVar3 = _V_stricmp((char *)(unaff_EBX + 0x38c592),pcVar2);
        if (iVar3 == 0) break;
        pcVar2 = KeyValues::GetName(this_00);
        iVar3 = _V_stricmp((char *)(unaff_EBX + 0x3a6050),pcVar2);
        if (iVar3 != 0) {
          pcVar2 = KeyValues::GetName(this_00);
          iVar3 = _V_stricmp((char *)(unaff_EBX + 0x3a605b),pcVar2);
          if (iVar3 == 0) {
            p_Var1 = (this->m_pLabelAtTen->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd4];
            pcVar2 = KeyValues::GetString(this_00,(char *)0x0,defaultValue);
            (*p_Var1)(this->m_pLabelAtTen,pcVar2);
          }
          goto LAB_006251a8;
        }
        p_Var1 = (this->m_pLabelAtOne->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd4];
        pcVar2 = KeyValues::GetString(this_00,(char *)0x0,defaultValue);
        (*p_Var1)(this->m_pLabelAtOne,pcVar2);
        this_00 = KeyValues::GetNextKey(this_00);
        if (this_00 == (KeyValues *)0x0) {
          return true;
        }
      }
      p_Var1 = (this->m_pQuestionText->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd4];
      pcVar2 = KeyValues::GetString(this_00,(char *)0x0,defaultValue);
      (*p_Var1)(this->m_pQuestionText,pcVar2);
LAB_006251a8:
      this_00 = KeyValues::GetNextKey(this_00);
    } while (this_00 != (KeyValues *)0x0);
  }
  return true;
}


/* COneToTenQuestion::GetQuestionText at 006241c0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void GetQuestionText(COneToTenQuestion * this, char * pBuffOut, int
   iBuffLen) */

void __thiscall
COneToTenQuestion::GetQuestionText(COneToTenQuestion *this,char *pBuffOut,int iBuffLen)

{
                    /* WARNING: Could not recover jumptable at 0x006241d8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->m_pQuestionText->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd6])();
  return;
}


/* COneToTenQuestion::GetAnswerText at 00624250 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* WARNING: Exceeded maximum restarts with more pending */
/* DWARF original prototype: void GetAnswerText(COneToTenQuestion * this, char * pBuffOut, int
   iBuffLen) */

void __thiscall
COneToTenQuestion::GetAnswerText(COneToTenQuestion *this,char *pBuffOut,int iBuffLen)

{
  RadioButton *pRVar1;
  int iVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: bool bFound@[???] */
  ___i686_get_pc_thunk_bx();
  pRVar1 = this->m_pRadioButtons[0];
  if (pRVar1 != (RadioButton *)0x0) {
    iVar2 = (*(pRVar1->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
              _vptr_IClientPanel[0x101])(pRVar1);
    if ((char)iVar2 != '\0') {
      iVar2 = 0;
      goto LAB_006243a3;
    }
  }
  pRVar1 = this->m_pRadioButtons[1];
  if (pRVar1 != (RadioButton *)0x0) {
    iVar2 = (*(pRVar1->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
              _vptr_IClientPanel[0x101])(pRVar1);
    if ((char)iVar2 != '\0') {
      iVar2 = 1;
      goto LAB_006243a3;
    }
  }
  pRVar1 = this->m_pRadioButtons[2];
  if (pRVar1 != (RadioButton *)0x0) {
    iVar2 = (*(pRVar1->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
              _vptr_IClientPanel[0x101])(pRVar1);
    if ((char)iVar2 != '\0') {
      iVar2 = 2;
      goto LAB_006243a3;
    }
  }
  pRVar1 = this->m_pRadioButtons[3];
  if (pRVar1 != (RadioButton *)0x0) {
    iVar2 = (*(pRVar1->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
              _vptr_IClientPanel[0x101])(pRVar1);
    if ((char)iVar2 != '\0') {
      iVar2 = 3;
      goto LAB_006243a3;
    }
  }
  pRVar1 = this->m_pRadioButtons[4];
  if (pRVar1 != (RadioButton *)0x0) {
    iVar2 = (*(pRVar1->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
              _vptr_IClientPanel[0x101])(pRVar1);
    if ((char)iVar2 != '\0') {
      iVar2 = 4;
      goto LAB_006243a3;
    }
  }
  pRVar1 = this->m_pRadioButtons[5];
  if (pRVar1 != (RadioButton *)0x0) {
    iVar2 = (*(pRVar1->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
              _vptr_IClientPanel[0x101])(pRVar1);
    if ((char)iVar2 != '\0') {
      iVar2 = 5;
      goto LAB_006243a3;
    }
  }
  pRVar1 = this->m_pRadioButtons[6];
  if (pRVar1 != (RadioButton *)0x0) {
    iVar2 = (*(pRVar1->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
              _vptr_IClientPanel[0x101])(pRVar1);
    if ((char)iVar2 != '\0') {
      iVar2 = 6;
      goto LAB_006243a3;
    }
  }
  pRVar1 = this->m_pRadioButtons[7];
  if (pRVar1 != (RadioButton *)0x0) {
    iVar2 = (*(pRVar1->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
              _vptr_IClientPanel[0x101])(pRVar1);
    if ((char)iVar2 != '\0') {
      iVar2 = 7;
      goto LAB_006243a3;
    }
  }
  pRVar1 = this->m_pRadioButtons[8];
  if (pRVar1 != (RadioButton *)0x0) {
    iVar2 = (*(pRVar1->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
              _vptr_IClientPanel[0x101])(pRVar1);
    if ((char)iVar2 != '\0') {
      iVar2 = 8;
      goto LAB_006243a3;
    }
  }
  pRVar1 = this->m_pRadioButtons[9];
  if (pRVar1 != (RadioButton *)0x0) {
    iVar2 = (*(pRVar1->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
              _vptr_IClientPanel[0x101])(pRVar1);
    if ((char)iVar2 != '\0') {
      iVar2 = 9;
LAB_006243a3:
                    /* WARNING: Could not recover jumptable at 0x006243c5. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (*(this->m_pRadioButtons[iVar2]->super_ToggleButton).super_Button.super_Label.super_Panel.
        super_IClientPanel._vptr_IClientPanel[0xd6])();
      return;
    }
  }
  V_strncpy(pBuffOut,*(char **)(unaff_EBX + 0x5d186e),iBuffLen);
  return;
}


/* CSurveyPanel::CSurveyPanel at 00625800 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void CSurveyPanel(CSurveyPanel * this, IViewPort * pViewport) */

void __thiscall CSurveyPanel::CSurveyPanel(CSurveyPanel *this,IViewPort *pViewport)

{
  char cVar1;
  PanelKeyBindingMap *pPVar2;
  PanelKeyBindingMap *pPVar3;
  Button *this_00;
  PanelListPanel *this_01;
  PanelMessageMap *pPVar4;
  PanelMessageMap *pPVar5;
  PanelAnimationMap *pPVar6;
  PanelAnimationMap *pPVar7;
  int unaff_EBX;
  PanelListPanel *this_1;
  Panel *parent_1;
  char *panelName_1;
  Button *in_stack_ffffffcc;
  Panel *in_stack_ffffffd0;
  char *in_stack_ffffffd4;
  char *in_stack_ffffffd8;
  Panel *in_stack_ffffffdc;
  char *in_stack_ffffffe0;
  
  ___i686_get_pc_thunk_bx();
  vgui::Frame::Frame(&this->super_Frame,(Panel *)0x0,&UNK_003902b3 + unaff_EBX,true,true);
  (this->super_Frame).super_EditablePanel.super_Panel.super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5cee1a);
  (this->super_IViewPortPanel)._vptr_IViewPortPanel = (_func_int_varargs **)(unaff_EBX + 0x5cf29e);
  if (*(char *)(unaff_EBX + 0x635756) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x635756) = 1;
    pPVar4 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3a568e));
    pPVar4->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x6082ee);
    pPVar5 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x389662));
    pPVar4->baseMap = pPVar5;
    cVar1 = *(char *)(unaff_EBX + 0x635757);
  }
  else {
    cVar1 = *(char *)(unaff_EBX + 0x635757);
  }
  if (cVar1 == '\0') {
    *(undefined1 *)(unaff_EBX + 0x635757) = 1;
    pPVar6 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x3a568e));
    pPVar6->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x6082ee);
    pPVar7 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x389662));
    pPVar6->baseMap = pPVar7;
  }
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
  if (*(char *)(unaff_EBX + 0x635758) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x635758) = 1;
    pPVar2 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3a568e));
    pPVar2->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x6082ee);
    pPVar3 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x389662));
    pPVar2->baseMap = pPVar3;
  }
  (this->m_hPointSurveyEntity).super_CBaseHandle.m_Index = 0xffffffff;
  this->id = -1;
  this->version = -1;
  this->m_pViewPort = pViewport;
  this_00 = operator_new(500);
  panelName_1 = (char *)0x0;
  parent_1 = (Panel *)0x0;
  this_1 = (PanelListPanel *)(unaff_EBX + 0x363456);
  vgui::Button::Button
            (this_00,(Panel *)this,(char *)(unaff_EBX + 0x3a58ff),(char *)this_1,(Panel *)0x0,
             (char *)0x0,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8,
             in_stack_ffffffdc,in_stack_ffffffe0);
  this->m_pSubmitButton = this_00;
  this_01 = operator_new(0x1ac);
  vgui::PanelListPanel::PanelListPanel
            (this_01,(Panel *)this,(char *)(unaff_EBX + 0x3a5ab7),this_1,parent_1,panelName_1);
  this->m_pQuestionsList = this_01;
  vgui::PanelListPanel::SetShowScrollBar(this_01,false);
  vgui::PanelListPanel::SetFirstColumnWidth(this->m_pQuestionsList,0);
  (*(this->m_pQuestionsList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xc])
            (this->m_pQuestionsList,0);
  vgui::Frame::SetMoveable(&this->super_Frame,false);
  vgui::Frame::SetSizeable(&this->super_Frame,false);
  vgui::Frame::SetTitleBarVisible(&this->super_Frame,false);
  vgui::Panel::SetProportional((Panel *)this,true);
  vgui::Frame::LoadControlSettings
            (&this->super_Frame,
             (char *)(CUtlRBTree<void*,short_unsigned_int,bool_(*)(void*_const&,_void*_const&),CUtlMemory<UtlRBTreeNode_t<void*,_short_unsigned_int>,_short_unsigned_int>_>
                      ::InsertRebalance + unaff_EBX + 4),(char *)0x0,(KeyValues *)0x0,
             (KeyValues *)0x0);
  return;
}


/* CSurveyPanel::CSurveyPanel at 00625a90 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void CSurveyPanel(CSurveyPanel * this, IViewPort * pViewport,
   CSurveyPanel * this, IViewPort * pViewport) */

void __thiscall
CSurveyPanel::CSurveyPanel
          (CSurveyPanel *this,IViewPort *pViewport,CSurveyPanel *this_1,IViewPort *pViewport_1)

{
  CSurveyPanel(this,pViewport);
  return;
}


/* CSurveyPanel::~CSurveyPanel at 00624f80 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void ~CSurveyPanel(CSurveyPanel * this, int __in_chrg) */

void __thiscall CSurveyPanel::~CSurveyPanel(CSurveyPanel *this,int __in_chrg)

{
  Button *pBVar1;
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_Frame).super_EditablePanel.super_Panel.super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5cf694);
  (this->super_IViewPortPanel)._vptr_IViewPortPanel = (_func_int_varargs **)(unaff_EBX + 0x5cfb18);
  (*(this->m_pQuestionsList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd8])
            (this->m_pQuestionsList);
  pBVar1 = this->m_pSubmitButton;
  if (pBVar1 != (Button *)0x0) {
    (*(pBVar1->super_Label).super_Panel.super_IClientPanel._vptr_IClientPanel[0x21])(pBVar1);
  }
  vgui::Frame::~Frame(&this->super_Frame,in_stack_ffffffd8);
  operator_delete(this);
  return;
}


/* CSurveyPanel::~CSurveyPanel at 00625020 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* WARNING: Struct "BaseModHybridButton": ignoring multiple overlapping fields */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void ~CSurveyPanel(CSurveyPanel * this, int __in_chrg) */

void __thiscall CSurveyPanel::~CSurveyPanel(CSurveyPanel *this,int __in_chrg)

{
  Button *pBVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_Frame).super_EditablePanel.super_Panel.super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5cf5f4);
  (this->super_IViewPortPanel)._vptr_IViewPortPanel = (_func_int_varargs **)(unaff_EBX + 0x5cfa78);
  (*(this->m_pQuestionsList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd8])
            (this->m_pQuestionsList);
  pBVar1 = this->m_pSubmitButton;
  if (pBVar1 != (Button *)0x0) {
    (*(pBVar1->super_Label).super_Panel.super_IClientPanel._vptr_IClientPanel[0x21])(pBVar1);
  }
  vgui::Frame::~Frame(&this->super_Frame,__in_chrg);
  return;
}


/* CSurveyPanel::~CSurveyPanel at 006250a0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void ~CSurveyPanel(CSurveyPanel * this, int __in_chrg) */

void __thiscall CSurveyPanel::~CSurveyPanel(CSurveyPanel *this,int __in_chrg)

{
  Button *pBVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_Frame).super_EditablePanel.super_Panel.super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5cf574);
  (this->super_IViewPortPanel)._vptr_IViewPortPanel =
       (_func_int_varargs **)(&UNK_005cf9f8 + unaff_EBX);
  (*(this->m_pQuestionsList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd8])
            (this->m_pQuestionsList);
  pBVar1 = this->m_pSubmitButton;
  if (pBVar1 != (Button *)0x0) {
    (*(pBVar1->super_Label).super_Panel.super_IClientPanel._vptr_IClientPanel[0x21])(pBVar1);
  }
  vgui::Frame::~Frame(&this->super_Frame,__in_chrg);
  return;
}


/* CSurveyPanel::Reset at 006241f0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void Reset(CSurveyPanel * this) */

void __thiscall CSurveyPanel::Reset(CSurveyPanel *this)

{
                    /* WARNING: Could not recover jumptable at 0x00624208. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->m_pQuestionsList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd8])();
  return;
}


/* CSurveyPanel::Update at 00624220 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void Update(CSurveyPanel * this) */

void __thiscall CSurveyPanel::Update(CSurveyPanel *this)

{
  return;
}


/* CSurveyPanel::NeedsUpdate at 00624240 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* DWARF original prototype: bool NeedsUpdate(CSurveyPanel * this) */

bool __thiscall CSurveyPanel::NeedsUpdate(CSurveyPanel *this)

{
  return false;
}


/* CSurveyPanel::ShowPanel at 00625710 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void ShowPanel(CSurveyPanel * this, bool bShow) */

void __thiscall CSurveyPanel::ShowPanel(CSurveyPanel *this,bool bShow)

{
  bool bVar1;
  
  bVar1 = vgui::Panel::IsVisible((Panel *)this);
  if (bVar1 != bShow) {
    if (bShow) {
      (*(this->super_Frame).super_EditablePanel.super_Panel.super_IClientPanel._vptr_IClientPanel
        [0x11a])(this);
      (*(this->super_Frame).super_EditablePanel.super_Panel.super_IClientPanel._vptr_IClientPanel
        [0x22])(this,1);
      (*(this->super_Frame).super_EditablePanel.super_Panel.super_IClientPanel._vptr_IClientPanel
        [0x3b])(this);
      (*(this->super_Frame).super_EditablePanel.super_Panel.super_IClientPanel._vptr_IClientPanel
        [0x88])(this,1);
      (*(this->super_Frame).super_EditablePanel.super_Panel.super_IClientPanel._vptr_IClientPanel
        [0x89])(this,1);
    }
    else {
      vgui::Panel::SetVisible((Panel *)this,false);
      (*(this->super_Frame).super_EditablePanel.super_Panel.super_IClientPanel._vptr_IClientPanel
        [0x119])(this);
      SendSurveyDoneCmd(this);
    }
    (*(this->m_pSubmitButton->super_Label).super_Panel.super_IClientPanel._vptr_IClientPanel[0x22])
              (this->m_pSubmitButton,(uint)bShow);
                    /* WARNING: Could not recover jumptable at 0x0062578d. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*(this->m_pQuestionsList->super_Panel).super_IClientPanel._vptr_IClientPanel[0x22])();
    return;
  }
  return;
}


/* CSurveyPanel::SendSurveyDoneCmd at 006255f0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void SendSurveyDoneCmd(CSurveyPanel * this) */

void __thiscall CSurveyPanel::SendSurveyDoneCmd(CSurveyPanel *this)

{
  uint uVar1;
  undefined4 *puVar2;
  code *pcVar3;
  int *piVar4;
  int iVar5;
  undefined4 uVar6;
  char *pcVar7;
  int unaff_EBX;
  CUtlString local_2c;
  
                    /* Unresolved local var: CUtlString strCmd@[???]
                       Unresolved local var: int nEntityIndex@[???] */
  ___i686_get_pc_thunk_bx();
  CUtlString::CUtlString(&local_2c);
  uVar1 = (this->m_hPointSurveyEntity).super_CBaseHandle.m_Index;
  if (((uVar1 == 0xffffffff) ||
      (iVar5 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x53e7de),
      *(uint *)(iVar5 + 8) != uVar1 >> 0x10)) ||
     (piVar4 = *(int **)(iVar5 + 4), piVar4 == (int *)0x0)) {
    uVar6 = 0xffffffff;
  }
  else {
    uVar6 = (**(code **)(*piVar4 + 0x148))(piVar4);
  }
  CUtlString::Format(&local_2c,(char *)(C_TEFizz::~C_TEFizz + unaff_EBX + 6),uVar6);
  puVar2 = *(undefined4 **)(unaff_EBX + 0x53e7a6);
  pcVar3 = *(code **)(*(int *)*puVar2 + 0x1c);
  pcVar7 = CUtlString::operator_const_char_(&local_2c);
  (*pcVar3)(*puVar2,pcVar7);
  if ((-1 < local_2c.m_Storage.m_Memory.m_nGrowSize) &&
     (local_2c.m_Storage.m_Memory.m_pMemory != (uchar *)0x0)) {
    local_2c.m_Storage.m_nActualLength = 0;
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53e772) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x53e772),local_2c.m_Storage.m_Memory.m_pMemory)
    ;
  }
  return;
}


/* CSurveyPanel::OnCommand at 00624b30 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void OnCommand(CSurveyPanel * this, char * command) */

void __thiscall CSurveyPanel::OnCommand(CSurveyPanel *this,char *command)

{
  int iVar1;
  IViewPort *pIVar2;
  char *pcVar3;
  C_BasePlayer *this_00;
  int *piVar4;
  uint32 enjoyment;
  uint32 difficulty;
  char *pcVar5;
  int unaff_EBX;
  undefined4 local_698;
  undefined4 local_694;
  undefined4 local_690;
  undefined4 local_68c;
  int local_688;
  int local_684;
  char local_66c [512];
  char local_46c [260];
  char local_368 [260];
  char local_264 [260];
  char local_160 [256];
  char local_60 [64];
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(unaff_EBX + 0x53f21f);
  iVar1 = _V_stricmp(command,(char *)(unaff_EBX + 0x3a65cc));
  if (iVar1 == 0) {
    if (*(int *)(*(int *)(unaff_EBX + 0x74625b) + 0x30) == 0) {
      iVar1 = vgui::PanelListPanel::GetItemCount(this->m_pQuestionsList);
      if (2 < iVar1) {
                    /* Unresolved local var: char[260] szDifficulty@[???]
                       Unresolved local var: char[260] szEnjoyment@[???]
                       Unresolved local var: char[260] szEssay@[???] */
        _memset(local_264,0,0x104);
        _memset(local_368,0,0x104);
        _memset(local_46c,0,0x104);
        piVar4 = (int *)(*(this->m_pQuestionsList->super_Panel).super_IClientPanel.
                          _vptr_IClientPanel[0xd6])(this->m_pQuestionsList,0);
        (**(code **)(*piVar4 + 0x3b8))(piVar4,local_264,0x104);
        piVar4 = (int *)(*(this->m_pQuestionsList->super_Panel).super_IClientPanel.
                          _vptr_IClientPanel[0xd6])(this->m_pQuestionsList,1);
        (**(code **)(*piVar4 + 0x3b8))(piVar4,local_368,0x104);
        piVar4 = (int *)(*(this->m_pQuestionsList->super_Panel).super_IClientPanel.
                          _vptr_IClientPanel[0xd6])(this->m_pQuestionsList,2);
        (**(code **)(*piVar4 + 0x3b8))(piVar4,local_46c,0x104);
        enjoyment = _atoi(local_368);
        difficulty = _atoi(local_264);
        C_CPortalGameStats::Event_AddSurveyData
                  (*(C_CPortalGameStats **)(unaff_EBX + 0x53fb73),difficulty,enjoyment,local_46c);
      }
      _Plat_GetLocalTime(&local_698);
      pcVar3 = (char *)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53f263) + 0xcc))
                                 ((int *)**(undefined4 **)(unaff_EBX + 0x53f263));
      V_FileBase(pcVar3,local_60,0x40);
      this_00 = C_BasePlayer::GetLocalPlayer(-1);
      pcVar3 = C_BasePlayer::GetPlayerName(this_00);
      pcVar5 = this->m_szSurveyName;
      V_snprintf(local_160,0x100,(char *)(unaff_EBX + 0x3a65d3),pcVar5,local_60,pcVar3,
                 local_684 + 0x76c,local_688 + 1,local_68c,local_690,local_694,local_698);
      V_snprintf(local_66c,0x200,&UNK_003a65ff + unaff_EBX,pcVar5);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53f227) + 0x50))
                ((int *)**(undefined4 **)(unaff_EBX + 0x53f227),local_66c,0);
      V_strncat(local_66c,local_160,0x200,-1);
      WriteSurveyDataToFile(this,local_66c);
      V_snprintf(local_160,0x100,(char *)(unaff_EBX + 0x3a6625),pcVar5);
      V_snprintf(local_66c,0x200,(char *)(unaff_EBX + 0x3a662c));
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53f227) + 0x50))
                ((int *)**(undefined4 **)(unaff_EBX + 0x53f227),local_66c,0);
      V_strncat(local_66c,local_160,0x200,-1);
      WriteSurveyDataToFile(this,local_66c);
    }
  }
  else {
    iVar1 = _V_stricmp(command,(char *)(unaff_EBX + 0x397ff9));
    if (iVar1 != 0) goto LAB_00624ba1;
  }
  pIVar2 = GetViewPortInterface();
  (*pIVar2->_vptr_IViewPort[2])(pIVar2,unaff_EBX + 0x390f80,0);
LAB_00624ba1:
  if (local_20 != **(int **)(unaff_EBX + 0x53f21f)) {
                    /* WARNING: Subroutine does not return */
    ___stack_chk_fail();
  }
  return;
}


/* cl_show_portal_puzzle_survey at 00624710 */

void cl_show_portal_puzzle_survey(CCommand *args)

{
                    /* Unresolved local var: char * pszSurveyName@[???] */
  ___i686_get_pc_thunk_cx();
  if (args->m_nArgc != 2) {
    _Msg();
    return;
  }
  ShowSurveyPanel(args->m_ppArgv[1]);
  return;
}


/* __tcf_3 at 0097d530 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x35c108),in_stack_00000008);
  return;
}


/* ShowSurveyPanel at 006244b0 */

bool ShowSurveyPanel(char *pszSurveyName)

{
  bool bVar1;
  IViewPort *pIVar2;
  IClientMode *pIVar3;
  int iVar4;
  int *piVar5;
  KeyValues *this;
  IBaseFileSystem *filesystem;
  int unaff_EBX;
  int *local_130;
  char local_124 [260];
  int local_20;
  
                    /* Unresolved local var: CBaseViewport * pViewport@[???]
                       Unresolved local var: IViewPortPanel * pSurveyPanel@[???]
                       Unresolved local var: char[260] pszFilePath@[???]
                       Unresolved local var: KeyValues * pKV@[???] */
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(unaff_EBX + 0x53f899);
  pIVar2 = GetViewPortInterface();
  if ((pIVar2 == (IViewPort *)0x0) || (*(int *)(*(int *)(unaff_EBX + 0x746875) + 0x30) != 0)) {
    bVar1 = false;
    goto LAB_006244f8;
  }
  pIVar3 = GetClientMode();
  iVar4 = (*pIVar3->_vptr_IClientMode[10])(pIVar3);
  if ((iVar4 == 0) ||
     (piVar5 = (int *)___dynamic_cast(iVar4,*(undefined4 *)(&DAT_0053fd75 + unaff_EBX),
                                      *(undefined4 *)(&DAT_0053fd7d + unaff_EBX),0),
     piVar5 == (int *)0x0)) {
LAB_00624680:
    _Warning(unaff_EBX + 0x3a6a39);
    bVar1 = false;
  }
  else {
    iVar4 = unaff_EBX + 0x3915fa;
    local_130 = (int *)(**(code **)(*piVar5 + 0x3b8))(piVar5,iVar4);
    if (local_130 == (int *)0x0) {
      local_130 = (int *)(**(code **)(*piVar5 + 0x3b4))(piVar5,iVar4);
      (**(code **)(*piVar5 + 0x3d4))(piVar5,local_130,unaff_EBX + 0x3a6a29);
      if (local_130 == (int *)0x0) goto LAB_00624680;
    }
    V_snprintf(local_124,0x104,(char *)(unaff_EBX + 0x3a6a77),pszSurveyName);
    this = KeyValues::operator_new(0x24);
    KeyValues::KeyValues(this,pszSurveyName);
    filesystem = (IBaseFileSystem *)0x0;
    if (**(int **)(unaff_EBX + 0x53f8a1) != 0) {
      filesystem = (IBaseFileSystem *)(**(int **)(unaff_EBX + 0x53f8a1) + 4);
    }
    bVar1 = KeyValues::LoadFromFile
                      (this,filesystem,local_124,(char *)(unaff_EBX + 0x374589),(GetSymbolProc_t)0x0
                      );
    if (bVar1) {
      (**(code **)(*local_130 + 0x10))(local_130);
      (**(code **)(*local_130 + 0xc))(local_130,this);
      KeyValues::deleteThis(this);
      pIVar2 = GetViewPortInterface();
      (*pIVar2->_vptr_IViewPort[2])(pIVar2,iVar4,1);
      bVar1 = true;
    }
    else {
      _Warning(unaff_EBX + 0x3a6a91,pszSurveyName);
      bVar1 = false;
    }
  }
LAB_006244f8:
  if (local_20 != **(int **)(unaff_EBX + 0x53f899)) {
                    /* WARNING: Subroutine does not return */
    ___stack_chk_fail();
  }
  return bVar1;
}


/* _GLOBAL__I_cl_disable_survey_panel at 00081640 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_cl_disable_survey_panel(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

