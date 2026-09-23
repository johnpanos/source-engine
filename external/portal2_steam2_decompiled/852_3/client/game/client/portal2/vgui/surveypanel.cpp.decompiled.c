/* DWARF-guided pseudocode for game/client/portal2/vgui/surveypanel.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 00082bf0 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *pcVar1;
  undefined4 uVar2;
  int unaff_EBX;
  longlong lVar3;
  ConVar *in_stack_ffffffd4;
  char *in_stack_ffffffd8;
  char *in_stack_ffffffdc;
  int in_stack_ffffffe0;
  
  lVar3 = ___i686_get_pc_thunk_bx();
  if (lVar3 == 0xffff00000001) {
    VarArgs::string[unaff_EBX + 0x1bc] = '\0';
    VarArgs::string[unaff_EBX + 0x1bd] = '\0';
    VarArgs::string[unaff_EBX + 0x1be] = '\0';
    VarArgs::string[unaff_EBX + 0x1bf] = '\0';
    pcVar1 = VarArgs::string + unaff_EBX + 0x1c0;
    pcVar1[0] = '\0';
    pcVar1[1] = '\0';
    pcVar1[2] = '\0';
    pcVar1[3] = '\0';
    pcVar1 = VarArgs::string + unaff_EBX + 0x1c4;
    pcVar1[0] = '\0';
    pcVar1[1] = '\0';
    pcVar1[2] = '\0';
    pcVar1[3] = '\0';
    pcVar1 = VarArgs::string + unaff_EBX + 0x1c8;
    pcVar1[0] = -1;
    pcVar1[1] = -1;
    pcVar1[2] = '\x7f';
    pcVar1[3] = '\x7f';
    pcVar1 = VarArgs::string + unaff_EBX + 0x1cc;
    pcVar1[0] = -1;
    pcVar1[1] = -1;
    pcVar1[2] = '\x7f';
    pcVar1[3] = '\x7f';
    pcVar1 = VarArgs::string + unaff_EBX + 0x1d0;
    pcVar1[0] = '\0';
    pcVar1[1] = '\0';
    pcVar1[2] = '\0';
    pcVar1[3] = '\0';
    pcVar1 = VarArgs::string + unaff_EBX + 0x1d4;
    pcVar1[0] = '\0';
    pcVar1[1] = '\0';
    pcVar1[2] = '\0';
    pcVar1[3] = '\0';
    pcVar1 = VarArgs::string + unaff_EBX + 0x1d8;
    pcVar1[0] = '\0';
    pcVar1[1] = '\0';
    pcVar1[2] = '\0';
    pcVar1[3] = '\0';
    pcVar1 = VarArgs::string + unaff_EBX + 0x1dc;
    pcVar1[0] = '\0';
    pcVar1[1] = '\0';
    pcVar1[2] = '\0';
    pcVar1[3] = '\0';
    pcVar1 = VarArgs::string + unaff_EBX + 0x1e0;
    pcVar1[0] = -1;
    pcVar1[1] = -1;
    pcVar1[2] = '\x7f';
    pcVar1[3] = '\x7f';
    pcVar1 = VarArgs::string + unaff_EBX + 0x1e4;
    pcVar1[0] = -1;
    pcVar1[1] = -1;
    pcVar1[2] = '\x7f';
    pcVar1[3] = '\x7f';
    pcVar1 = VarArgs::string + unaff_EBX + 0x1e8;
    pcVar1[0] = -1;
    pcVar1[1] = -1;
    pcVar1[2] = '\x7f';
    pcVar1[3] = '\x7f';
    pcVar1 = VarArgs::string + unaff_EBX + 0x1ec;
    pcVar1[0] = -1;
    pcVar1[1] = -1;
    pcVar1[2] = '\x7f';
    pcVar1[3] = '\x7f';
    *(undefined **)(VarArgs::string + unaff_EBX + 0x1f0) = &UNK_00aaf204 + unaff_EBX;
    pcVar1 = VarArgs::string + unaff_EBX + 500;
    pcVar1[0] = '\0';
    pcVar1[1] = '\0';
    pcVar1[2] = '\0';
    pcVar1[3] = '\0';
    VarArgs::string[unaff_EBX + 0x1fb] = '\0';
    VarArgs::string[unaff_EBX + 0x1f8] = '\0';
    VarArgs::string[unaff_EBX + 0x1f9] = '\0';
    VarArgs::string[unaff_EBX + 0x1fa] = '\0';
    pcVar1 = VarArgs::string + unaff_EBX + 0x1fc;
    pcVar1[0] = '\0';
    pcVar1[1] = '\0';
    pcVar1[2] = '\0';
    pcVar1[3] = '\0';
    VarArgs::string[unaff_EBX + 0x203] = '\0';
    VarArgs::string[unaff_EBX + 0x200] = '\0';
    VarArgs::string[unaff_EBX + 0x201] = '\0';
    VarArgs::string[unaff_EBX + 0x202] = '\0';
    pcVar1 = VarArgs::string + unaff_EBX + 0x204;
    pcVar1[0] = '\0';
    pcVar1[1] = '\0';
    pcVar1[2] = '\0';
    pcVar1[3] = '\0';
    VarArgs::string[unaff_EBX + 0x20b] = '\x01';
    VarArgs::string[unaff_EBX + 0x208] = '\0';
    VarArgs::string[unaff_EBX + 0x209] = '\0';
    VarArgs::string[unaff_EBX + 0x20a] = '\0';
    pcVar1 = VarArgs::string + unaff_EBX + 0x20c;
    pcVar1[0] = '\x01';
    pcVar1[1] = '\0';
    pcVar1[2] = '\0';
    pcVar1[3] = '\0';
    VarArgs::string[unaff_EBX + 0x213] = '\0';
    VarArgs::string[unaff_EBX + 0x210] = '\0';
    VarArgs::string[unaff_EBX + 0x211] = '\0';
    VarArgs::string[unaff_EBX + 0x212] = '\0';
    pcVar1 = VarArgs::string + unaff_EBX + 0x214;
    pcVar1[0] = '\x02';
    pcVar1[1] = '\0';
    pcVar1[2] = '\0';
    pcVar1[3] = '\0';
    VarArgs::string[unaff_EBX + 0x21b] = '\0';
    VarArgs::string[unaff_EBX + 0x218] = '\0';
    VarArgs::string[unaff_EBX + 0x219] = '\0';
    VarArgs::string[unaff_EBX + 0x21a] = '\0';
    ConVar::ConVar((ConVar *)((int)ice_sbox[2] + unaff_EBX + 0x5c),(char *)(unaff_EBX + 0x91d3f9),
                   (char *)(unaff_EBX + 0x8d5a08),0x2080,in_stack_ffffffd4,in_stack_ffffffd8,
                   in_stack_ffffffdc,in_stack_ffffffe0);
    uVar2 = *(undefined4 *)(&DAT_00aae11c + unaff_EBX);
    ___cxa_atexit(unaff_EBX + 0x8d012c,0,uVar2);
    ConVar::ConVar((ConVar *)((int)ice_sbox[2] + unaff_EBX + 0xbc),(char *)(unaff_EBX + 0x91d414),
                   (char *)(unaff_EBX + 0x8d5a08),0,in_stack_ffffffd4,in_stack_ffffffd8,
                   in_stack_ffffffdc,in_stack_ffffffe0);
    ___cxa_atexit(unaff_EBX + 0x8d010c,0,uVar2);
    CAutoGameSystem::CAutoGameSystem
              ((CAutoGameSystem *)(VarArgs::string + unaff_EBX + 0x21c),(char *)0x0);
    *(int *)(VarArgs::string + unaff_EBX + 0x21c) =
         (int)&PTR_GetPVSNotifyInterface_00b3d6c4 + unaff_EBX;
    ___cxa_atexit(unaff_EBX + 0x8d014c,0,uVar2);
    ConCommand::ConCommand
              ((ConCommand *)(VarArgs::string + unaff_EBX + 0x33c),(char *)(unaff_EBX + 0x91d4f1),
               (FnCommandCallback_t)(unaff_EBX + 0x5958fc),(char *)(unaff_EBX + 0x91d438),0,
               (FnCommandCompletionCallback)0x0);
    ___cxa_atexit(unaff_EBX + 0x8d00ec,0,uVar2);
  }
  return;
}


/* __tcf_0 at 00952d30 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fe7e8),in_stack_00000008);
  return;
}


/* __tcf_1 at 00952d10 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fe868),in_stack_00000008);
  return;
}


/* CTimeInMapHelper::~CTimeInMapHelper at 0061b1f0 */

/* DWARF original prototype: void ~CTimeInMapHelper(CTimeInMapHelper * this, int __in_chrg) */

void __thiscall CTimeInMapHelper::~CTimeInMapHelper(CTimeInMapHelper *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(extraout_ECX + 0x517e30);
  IGameSystem::~IGameSystem((IGameSystem *)this,__in_chrg);
  return;
}


/* CTimeInMapHelper::~CTimeInMapHelper at 0061b2e0 */

/* DWARF original prototype: void ~CTimeInMapHelper(CTimeInMapHelper * this, int __in_chrg) */

void __thiscall CTimeInMapHelper::~CTimeInMapHelper(CTimeInMapHelper *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(unaff_EBX + 0x517d37);
  IGameSystem::~IGameSystem((IGameSystem *)this,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CTimeInMapHelper::Name at 0061a4f0 */

/* DWARF original prototype: char * Name(CTimeInMapHelper * this) */

char * __thiscall CTimeInMapHelper::Name(CTimeInMapHelper *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (char *)(extraout_ECX + 0x385805);
}


/* CTimeInMapHelper::Init at 0061a500 */

/* DWARF original prototype: bool Init(CTimeInMapHelper * this) */

bool __thiscall CTimeInMapHelper::Init(CTimeInMapHelper *this)

{
  this->m_szCurMapName[0] = '\0';
  return true;
}


/* CTimeInMapHelper::LevelInitPreEntity at 0061aba0 */

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
  puVar1 = *(undefined4 **)(unaff_EBX + 0x5161a4);
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


/* CSurveyPanel::WriteSurveyDataToFile at 00618530 */

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
  local_20 = **(int **)(unaff_EBX + 0x5187d3);
  bVar3 = false;
  if (file != (char *)0x0) {
    uVar5 = (**(code **)(*(int *)(**(int **)(unaff_EBX + 0x5187db) + 4) + 8))
                      (**(int **)(unaff_EBX + 0x5187db) + 4,file,unaff_EBX + 0x366d7b,0);
    cVar4 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5187db) + 0x60))
                      ((int *)**(undefined4 **)(unaff_EBX + 0x5187db),uVar5);
    if (cVar4 == '\0') {
      _Warning(unaff_EBX + 0x38796b,file);
      bVar3 = false;
    }
    else {
      _Plat_GetLocalTime(&local_2190);
      pcVar6 = (char *)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x518817) + 0xcc))
                                 ((int *)**(undefined4 **)(unaff_EBX + 0x518817));
      V_FileBase(pcVar6,local_60,0x40);
      if ((this->id == -1) || (this->version == -1)) {
        _Warning(unaff_EBX + 0x387993,this->m_szSurveyName);
        bVar3 = false;
      }
      else {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5187db) + 0x6c))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x5187db),uVar5,unaff_EBX + 0x3879e8,
                   this->id,this->version);
        this_00 = C_BasePlayer::GetLocalPlayer(-1);
        pcVar2 = *(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5187db) + 0x6c);
        lVar9 = (longdouble)_Plat_FloatTime();
        fVar1 = *(float *)(unaff_EBX + 0x6a856b);
        pcVar6 = C_BasePlayer::GetPlayerName(this_00);
        (*pcVar2)(**(undefined4 **)(unaff_EBX + 0x5187db),uVar5,unaff_EBX + 0x3879ef,pcVar6,local_60
                  ,local_217c + 0x76c,local_2180 + 1,local_2184,local_2188,local_218c,local_2190,
                  (double)((float)lVar9 - fVar1));
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
          V_snprintf(local_2164,0x2000,(char *)(unaff_EBX + 0x387a18),local_164);
          _Msg(local_2164);
          (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5187db) + 0x6c))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x5187db),uVar5,local_2164);
          nItemID = vgui::PanelListPanel::NextItem(this->m_pQuestionsList,nItemID);
        }
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5187db) + 0x6c))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x5187db),uVar5,unaff_EBX + 0x352657);
        (**(code **)(*(int *)(**(int **)(unaff_EBX + 0x5187db) + 4) + 0xc))
                  (**(int **)(unaff_EBX + 0x5187db) + 4,uVar5);
        bVar3 = true;
      }
    }
  }
  if (local_20 == **(int **)(unaff_EBX + 0x5187d3)) {
    return bVar3;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}


/* __tcf_2 at 00952d50 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  *(int *)(extraout_ECX + 0x36dd48) = extraout_ECX + 0x1e02d0;
  IGameSystem::~IGameSystem((IGameSystem *)(extraout_ECX + 0x36dd48),in_stack_00000008);
  return;
}


/* CSurveyQuestion::ApplySchemeSettings at 006193a0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ApplySchemeSettings(CSurveyQuestion * this, IScheme * pScheme) */

void __thiscall CSurveyQuestion::ApplySchemeSettings(CSurveyQuestion *this,IScheme *pScheme)

{
  vgui::Panel::ApplySchemeSettings((Panel *)this,pScheme);
                    /* WARNING: Could not recover jumptable at 0x006193d0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel[0x3c])();
  return;
}


/* CEssayQuestion::ApplySchemeSettings at 0061b210 */

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
                    (pScheme,&UNK_0036af02 + unaff_EBX,0xffffffff);
  (*p_Var1)(this->m_pAnswer,iVar2);
  p_Var1 = (this->m_pAnswer->super_Panel).super_IClientPanel._vptr_IClientPanel[0x3c];
  iVar2 = (*(pScheme->super_IBaseInterface)._vptr_IBaseInterface[6])
                    (pScheme,&UNK_00384eef + unaff_EBX,0xff000000);
  (*p_Var1)(this->m_pAnswer,iVar2);
  vgui::Panel::ApplySchemeSettings((Panel *)this,pScheme);
                    /* WARNING: Could not recover jumptable at 0x0061b2d0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
    _vptr_IClientPanel[0x3c])();
  return;
}


/* CSurveyPanel::SetData at 006198a0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "BaseModHybridButton": ignoring multiple overlapping fields */
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
  iVar6 = KeyValues::GetInt(data,(char *)(C_PlayerResource::~C_PlayerResource + unaff_EBX + 4),-1);
  this->id = iVar6;
  iVar6 = KeyValues::GetInt(data,(char *)(unaff_EBX + 0x3902aa),-1);
  this->version = iVar6;
  pQuestionEntry = KeyValues::GetFirstTrueSubKey(data);
  if (pQuestionEntry != (KeyValues *)0x0) {
    pp_Var1 = (_func_int_varargs **)(unaff_EBX + 0x5a65da);
    p_Var3 = *(_func_char_ptr **)(&DAT_005de2ea + unaff_EBX);
    do {
      pcVar5 = KeyValues::GetName(pQuestionEntry);
      iVar6 = _V_stricmp((char *)(unaff_EBX + 0x386897),pcVar5);
      if (iVar6 == 0) {
        this_00 = operator_new(0x1bc);
        pPVar4 = this->m_pQuestionsList;
        vgui::EditablePanel::EditablePanel
                  ((EditablePanel *)this_00,&pPVar4->super_Panel,(char *)(unaff_EBX + 0x3868a0));
        (this_00->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
        _vptr_IClientPanel = pp_Var1;
        if (*(char *)(unaff_EBX + 0x60b64a) == '\0') {
          *(undefined1 *)(unaff_EBX + 0x60b64a) = 1;
          pPVar14 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x386432));
          pPVar14->pfnClassName = *(_func_char_ptr **)(&DAT_005de2d6 + unaff_EBX);
          pPVar15 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x380baa));
          pPVar14->baseMap = pPVar15;
        }
        if (*(char *)(unaff_EBX + 0x60b64b) == '\0') {
          *(undefined1 *)(unaff_EBX + 0x60b64b) = 1;
          pPVar12 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x386432));
          pPVar12->pfnClassName = *(_func_char_ptr **)(&DAT_005de2d6 + unaff_EBX);
          pPVar13 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x380baa));
          pPVar12->baseMap = pPVar13;
        }
        if (*(char *)(unaff_EBX + 0x60b64c) == '\0') {
          *(undefined1 *)(unaff_EBX + 0x60b64c) = 1;
          pPVar7 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x386432));
          pPVar7->pfnClassName = *(_func_char_ptr **)(&DAT_005de2d6 + unaff_EBX);
          pPVar8 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x380baa));
          pPVar7->baseMap = pPVar8;
        }
        (this_00->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
        _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a5a3a);
        if (*(char *)(unaff_EBX + 0x60b64d) == '\0') {
          *(undefined1 *)(unaff_EBX + 0x60b64d) = 1;
          pPVar14 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x386484));
          pPVar14->pfnClassName = *(_func_char_ptr **)(&DAT_005de2ca + unaff_EBX);
          pPVar15 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x386432));
          pPVar14->baseMap = pPVar15;
        }
        if (*(char *)(unaff_EBX + 0x60b64e) == '\0') {
          *(undefined1 *)(unaff_EBX + 0x60b64e) = 1;
          pPVar12 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x386484));
          pPVar12->pfnClassName = *(_func_char_ptr **)(&DAT_005de2ca + unaff_EBX);
          pPVar13 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x386432));
          pPVar12->baseMap = pPVar13;
        }
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
        if (*(char *)(unaff_EBX + 0x60b64f) == '\0') {
          *(undefined1 *)(unaff_EBX + 0x60b64f) = 1;
          pPVar7 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x386484));
          pPVar7->pfnClassName = *(_func_char_ptr **)(&DAT_005de2ca + unaff_EBX);
          pPVar8 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x386432));
          pPVar7->baseMap = pPVar8;
        }
        pLVar9 = operator_new(0x1a4);
        vgui::Label::Label(pLVar9,(Panel *)this_00,(char *)(unaff_EBX + 0x3868bf),
                           (char *)(unaff_EBX + 0x3868b1),(Label *)in_stack_ffffff84,
                           in_stack_ffffff88,in_stack_ffffff8c,in_stack_ffffff90);
        this_00->m_pQuestionText = pLVar9;
        pLVar9 = operator_new(0x1a4);
        vgui::Label::Label(pLVar9,(Panel *)this_00,&UNK_003868e0 + unaff_EBX,
                           &UNK_003868d4 + unaff_EBX,(Label *)in_stack_ffffff84,in_stack_ffffff88,
                           in_stack_ffffff8c,in_stack_ffffff90);
        this_00->m_pLabelAtOne = pLVar9;
        pLVar9 = operator_new(0x1a4);
        vgui::Label::Label(pLVar9,(Panel *)this_00,&UNK_003868f6 + unaff_EBX,
                           &UNK_003868eb + unaff_EBX,(Label *)in_stack_ffffff84,in_stack_ffffff88,
                           in_stack_ffffff8c,in_stack_ffffff90);
        this_00->m_pLabelAtTen = pLVar9;
        (this_00->super_CSurveyQuestion).m_questiontype = SVQ_ONE_TO_TEN;
        in_stack_ffffff84 = (RadioButton *)0x0;
        vgui::EditablePanel::LoadControlSettings
                  ((EditablePanel *)this_00,&UNK_00386902 + unaff_EBX,(char *)0x0,(KeyValues *)0x0,
                   (KeyValues *)0x0);
        iVar10 = vgui::Panel::GetWide(&pPVar4->super_Panel);
        local_34 = 0;
        iVar6 = 0;
        while( true ) {
          iVar16 = iVar6 + 1;
          V_snprintf(local_1f,3,(char *)(unaff_EBX + 0x35878a),iVar16);
          this_01 = operator_new(0x20c);
          vgui::RadioButton::RadioButton
                    (this_01,(Panel *)this_00,(char *)(unaff_EBX + 0x386741),local_1f,
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
        iVar6 = _V_stricmp(&UNK_00386926 + unaff_EBX,pcVar5);
        if (iVar6 == 0) {
          this_02 = operator_new(400);
          vgui::EditablePanel::EditablePanel
                    ((EditablePanel *)this_02,&this->m_pQuestionsList->super_Panel,
                     &UNK_0038692c + unaff_EBX);
          (this_02->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
          _vptr_IClientPanel = pp_Var1;
          if (*(char *)(unaff_EBX + 0x60b64a) == '\0') {
            *(undefined1 *)(unaff_EBX + 0x60b64a) = 1;
            pPVar14 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x386432));
            pPVar14->pfnClassName = *(_func_char_ptr **)(&DAT_005de2d6 + unaff_EBX);
            pPVar15 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x380baa));
            pPVar14->baseMap = pPVar15;
          }
          if (*(char *)(unaff_EBX + 0x60b64b) == '\0') {
            *(undefined1 *)(unaff_EBX + 0x60b64b) = 1;
            pPVar12 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x386432));
            pPVar12->pfnClassName = *(_func_char_ptr **)(&DAT_005de2d6 + unaff_EBX);
            pPVar13 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x380baa));
            pPVar12->baseMap = pPVar13;
          }
          if (*(char *)(unaff_EBX + 0x60b64c) == '\0') {
            *(undefined1 *)(unaff_EBX + 0x60b64c) = 1;
            pPVar7 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x386432));
            pPVar7->pfnClassName = *(_func_char_ptr **)(&DAT_005de2d6 + unaff_EBX);
            pPVar8 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x380baa));
            pPVar7->baseMap = pPVar8;
          }
          (this_02->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
          _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a61fa);
          if (*(char *)(unaff_EBX + 0x60b650) == '\0') {
            *(undefined1 *)(unaff_EBX + 0x60b650) = 1;
            pPVar14 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x38645f));
            pPVar14->pfnClassName =
                 *(_func_char_ptr **)(BaseModUI::Leaderboard::Leaderboard + unaff_EBX + 6);
            pPVar15 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x386432));
            pPVar14->baseMap = pPVar15;
            cVar2 = *(char *)(unaff_EBX + 0x60b651);
          }
          else {
            cVar2 = *(char *)(unaff_EBX + 0x60b651);
          }
          if (cVar2 == '\0') {
            *(undefined1 *)(unaff_EBX + 0x60b651) = 1;
            pPVar12 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x38645f));
            pPVar12->pfnClassName =
                 *(_func_char_ptr **)(BaseModUI::Leaderboard::Leaderboard + unaff_EBX + 6);
            pPVar13 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x386432));
            pPVar12->baseMap = pPVar13;
          }
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
          if (*(char *)(unaff_EBX + 0x60b652) == '\0') {
            *(undefined1 *)(unaff_EBX + 0x60b652) = 1;
            pPVar7 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x38645f));
            pPVar7->pfnClassName =
                 *(_func_char_ptr **)(BaseModUI::Leaderboard::Leaderboard + unaff_EBX + 6);
            pPVar8 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x386432));
            pPVar7->baseMap = pPVar8;
          }
          pLVar9 = operator_new(0x1a4);
          this_1 = (TextEntry *)(&UNK_0038693a + unaff_EBX);
          vgui::Label::Label(pLVar9,(Panel *)this_02,&UNK_0038694e + unaff_EBX,(char *)this_1,
                             (Label *)in_stack_ffffff84,in_stack_ffffff88,in_stack_ffffff8c,
                             in_stack_ffffff90);
          this_02->m_pQuestionText = pLVar9;
          this_03 = operator_new(0x63c);
          vgui::TextEntry::TextEntry
                    (this_03,(Panel *)this_02,&UNK_00386960 + unaff_EBX,this_1,
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
                    ((EditablePanel *)this_02,&UNK_00386967 + unaff_EBX,(char *)0x0,(KeyValues *)0x0
                     ,(KeyValues *)0x0);
          CEssayQuestion::SetData(this_02,pQuestionEntry);
          (*(this->m_pQuestionsList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd4])
                    (this->m_pQuestionsList,0,this_02);
        }
        else {
          pcVar5 = KeyValues::GetName(pQuestionEntry);
          iVar6 = _V_stricmp(&UNK_00386985 + unaff_EBX,pcVar5);
          if (iVar6 == 0) {
            this_04 = operator_new(0x1b8);
            pPVar4 = this->m_pQuestionsList;
            vgui::EditablePanel::EditablePanel
                      ((EditablePanel *)this_04,&pPVar4->super_Panel,&UNK_0038692c + unaff_EBX);
            (this_04->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
            _vptr_IClientPanel = pp_Var1;
            if (*(char *)(unaff_EBX + 0x60b64a) == '\0') {
              *(undefined1 *)(unaff_EBX + 0x60b64a) = 1;
              pPVar14 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x386432));
              pPVar14->pfnClassName = *(_func_char_ptr **)(&DAT_005de2d6 + unaff_EBX);
              pPVar15 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x380baa));
              pPVar14->baseMap = pPVar15;
            }
            if (*(char *)(unaff_EBX + 0x60b64b) == '\0') {
              *(undefined1 *)(unaff_EBX + 0x60b64b) = 1;
              pPVar12 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x386432));
              pPVar12->pfnClassName = *(_func_char_ptr **)(&DAT_005de2d6 + unaff_EBX);
              pPVar13 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x380baa));
              pPVar12->baseMap = pPVar13;
            }
            if (*(char *)(unaff_EBX + 0x60b64c) == '\0') {
              *(undefined1 *)(unaff_EBX + 0x60b64c) = 1;
              pPVar7 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x386432));
              pPVar7->pfnClassName = *(_func_char_ptr **)(&DAT_005de2d6 + unaff_EBX);
              pPVar8 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x380baa));
              pPVar7->baseMap = pPVar8;
            }
            (this_04->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
            _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a5e1a);
            if (*(char *)(unaff_EBX + 0x60b653) == '\0') {
              *(undefined1 *)(unaff_EBX + 0x60b653) = 1;
              pPVar14 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x38646e));
              pPVar14->pfnClassName = p_Var3;
              pPVar15 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x386432));
              pPVar14->baseMap = pPVar15;
            }
            if (*(char *)(unaff_EBX + 0x60b654) == '\0') {
              *(undefined1 *)(unaff_EBX + 0x60b654) = 1;
              pPVar12 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x38646e));
              pPVar12->pfnClassName = p_Var3;
              pPVar13 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x386432));
              pPVar12->baseMap = pPVar13;
            }
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
            if (*(char *)(unaff_EBX + 0x60b655) == '\0') {
              *(undefined1 *)(unaff_EBX + 0x60b655) = 1;
              pPVar7 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x38646e));
              pPVar7->pfnClassName = *(_func_char_ptr **)(&DAT_005de2ea + unaff_EBX);
              pPVar8 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x386432));
              pPVar7->baseMap = pPVar8;
            }
            pLVar9 = operator_new(0x1a4);
            vgui::Label::Label(pLVar9,(Panel *)this_04,&UNK_003869ae + unaff_EBX,
                               &UNK_00386992 + unaff_EBX,(Label *)in_stack_ffffff84,
                               in_stack_ffffff88,in_stack_ffffff8c,in_stack_ffffff90);
            this_04->m_pQuestionText = pLVar9;
            iVar6 = vgui::Panel::GetWide(&pPVar4->super_Panel);
            vgui::Panel::SetSize((Panel *)this_04,iVar6,0x40);
            in_stack_ffffff84 = (RadioButton *)0x0;
            vgui::EditablePanel::LoadControlSettings
                      ((EditablePanel *)this_04,(char *)(unaff_EBX + 0x3869ca),(char *)0x0,
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


/* CEssayQuestion::GetPanelClassName at 0061a520 */

char * CEssayQuestion::GetPanelClassName(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (char *)(extraout_ECX + 0x3857e5);
}


/* CEssayQuestion::KB_ChainToMap at 0061a6c0 */

void CEssayQuestion::KB_ChainToMap(void)

{
  PanelKeyBindingMap *pPVar1;
  PanelKeyBindingMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x60a82f) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x60a82f) = 1;
    pPVar1 = vgui::FindOrAddPanelKeyBindingMap(&UNK_0038563c + unaff_EBX);
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x5dd4c3);
    pPVar2 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x38560f));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CEssayQuestion::ChainToAnimationMap at 0061a8a0 */

void CEssayQuestion::ChainToAnimationMap(void)

{
  PanelAnimationMap *pPVar1;
  PanelAnimationMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelAnimationMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x60a64e) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x60a64e) = 1;
    pPVar1 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x38545c));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x5dd2e3);
    pPVar2 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x38542f));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CEssayQuestion::ChainToMap at 0061aa80 */

void CEssayQuestion::ChainToMap(void)

{
  PanelMessageMap *pPVar1;
  PanelMessageMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelMessageMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&DAT_0060a46d)[unaff_EBX] == '\0') {
    (&DAT_0060a46d)[unaff_EBX] = 1;
    pPVar1 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x38527c));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x5dd103);
    pPVar2 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x38524f));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CEssayQuestion::GetKBMap at 0061ac30 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* DWARF original prototype: PanelKeyBindingMap * GetKBMap(CEssayQuestion * this) */

PanelKeyBindingMap * __thiscall CEssayQuestion::GetKBMap(CEssayQuestion *this)

{
  int iVar1;
  PanelKeyBindingMap *pPVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((&DAT_0060a25c)[unaff_EBX] == '\0') {
    iVar1 = ___cxa_guard_acquire(&DAT_0060a25c + unaff_EBX);
    if (iVar1 != 0) {
      pPVar2 = vgui::FindOrAddPanelKeyBindingMap(&UNK_003850c9 + unaff_EBX);
      *(PanelKeyBindingMap **)(&DAT_0060a294 + unaff_EBX) = pPVar2;
      ___cxa_guard_release(&DAT_0060a25c + unaff_EBX);
    }
  }
  return *(PanelKeyBindingMap **)(&DAT_0060a294 + unaff_EBX);
}


/* CEssayQuestion::GetAnimMap at 0061acb0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* DWARF original prototype: PanelAnimationMap * GetAnimMap(CEssayQuestion * this) */

PanelAnimationMap * __thiscall CEssayQuestion::GetAnimMap(CEssayQuestion *this)

{
  PanelAnimationMap *pPVar1;
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  pPVar1 = FindOrAddPanelAnimationMap((char *)(extraout_ECX + 0x385055));
  return pPVar1;
}


/* CEssayQuestion::GetMessageMap at 0061acd0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* DWARF original prototype: PanelMessageMap * GetMessageMap(CEssayQuestion * this) */

PanelMessageMap * __thiscall CEssayQuestion::GetMessageMap(CEssayQuestion *this)

{
  int iVar1;
  PanelMessageMap *pPVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((&DAT_0060a1b4)[unaff_EBX] == '\0') {
    iVar1 = ___cxa_guard_acquire(&DAT_0060a1b4 + unaff_EBX);
    if (iVar1 != 0) {
      pPVar2 = vgui::FindOrAddPanelMessageMap(&UNK_00385029 + unaff_EBX);
      *(PanelMessageMap **)(unaff_EBX + 0x60a1f8) = pPVar2;
      ___cxa_guard_release(&DAT_0060a1b4 + unaff_EBX);
    }
  }
  return *(PanelMessageMap **)(unaff_EBX + 0x60a1f8);
}


/* CEssayQuestion::~CEssayQuestion at 0061b490 */

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
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a4604);
  pLVar1 = this->m_pQuestionText;
  if (pLVar1 != (Label *)0x0) {
    (*(pLVar1->super_Panel).super_IClientPanel._vptr_IClientPanel[0x21])(pLVar1);
  }
  pTVar2 = this->m_pAnswer;
  if (pTVar2 != (TextEntry *)0x0) {
    (*(pTVar2->super_Panel).super_IClientPanel._vptr_IClientPanel[0x21])(pTVar2);
  }
  (this->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a49e4);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,in_stack_ffffffd8);
  operator_delete(this);
  return;
}


/* CEssayQuestion::~CEssayQuestion at 0061b520 */

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
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a4574);
  pLVar1 = this->m_pQuestionText;
  if (pLVar1 != (Label *)0x0) {
    (*(pLVar1->super_Panel).super_IClientPanel._vptr_IClientPanel[0x21])(pLVar1);
  }
  pTVar2 = this->m_pAnswer;
  if (pTVar2 != (TextEntry *)0x0) {
    (*(pTVar2->super_Panel).super_IClientPanel._vptr_IClientPanel[0x21])(pTVar2);
  }
  (this->super_CSurveyQuestion).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a4954);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,__in_chrg);
  return;
}


/* CEssayQuestion::SetData at 00619090 */

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
        iVar3 = _V_stricmp((char *)(unaff_EBX + 0x36dc36),pcVar2);
        if (iVar3 != 0) break;
        p_Var1 = (this->m_pQuestionText->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd4];
        pcVar2 = KeyValues::GetString(this_00,(char *)0x0,(char *)(unaff_EBX + 0x3451ce));
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


/* CEssayQuestion::GetQuestionText at 00617f50 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* DWARF original prototype: void GetQuestionText(CEssayQuestion * this, char * pBuffOut, int
   iBuffLen) */

void __thiscall CEssayQuestion::GetQuestionText(CEssayQuestion *this,char *pBuffOut,int iBuffLen)

{
                    /* WARNING: Could not recover jumptable at 0x00617f68. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->m_pQuestionText->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd6])();
  return;
}


/* CEssayQuestion::GetAnswerText at 00617f70 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* DWARF original prototype: void GetAnswerText(CEssayQuestion * this, char * pBuffOut, int
   iBuffLen) */

void __thiscall CEssayQuestion::GetAnswerText(CEssayQuestion *this,char *pBuffOut,int iBuffLen)

{
                    /* WARNING: Could not recover jumptable at 0x00617f88. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->m_pAnswer->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd6])();
  return;
}


/* CRadioChoicesQuestion::GetPanelClassName at 0061a530 */

char * CRadioChoicesQuestion::GetPanelClassName(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (char *)(extraout_ECX + 0x3857e4);
}


/* CRadioChoicesQuestion::KB_ChainToMap at 0061a660 */

void CRadioChoicesQuestion::KB_ChainToMap(void)

{
  PanelKeyBindingMap *pPVar1;
  PanelKeyBindingMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x60a892) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x60a892) = 1;
    pPVar1 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3856ab));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x5dd527);
    pPVar2 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x38566f));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CRadioChoicesQuestion::ChainToAnimationMap at 0061a840 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* WARNING: Struct "CPolygonButton": ignoring multiple overlapping fields */

void CRadioChoicesQuestion::ChainToAnimationMap(void)

{
  PanelAnimationMap *pPVar1;
  PanelAnimationMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelAnimationMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (CRadialMenu::~CRadialMenu[unaff_EBX + 1] == (code)0x0) {
    CRadialMenu::~CRadialMenu[unaff_EBX + 1] = (code)0x1;
    pPVar1 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x3854cb));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x5dd347);
    pPVar2 = FindOrAddPanelAnimationMap(&UNK_0038548f + unaff_EBX);
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CRadioChoicesQuestion::ChainToMap at 0061aa20 */

void CRadioChoicesQuestion::ChainToMap(void)

{
  PanelMessageMap *pPVar1;
  PanelMessageMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelMessageMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x60a4d0) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x60a4d0) = 1;
    pPVar1 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3852eb));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x5dd167);
    pPVar2 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3852af));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CRadioChoicesQuestion::GetKBMap at 0061af90 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: PanelKeyBindingMap * GetKBMap(CRadioChoicesQuestion * this) */

PanelKeyBindingMap * __thiscall CRadioChoicesQuestion::GetKBMap(CRadioChoicesQuestion *this)

{
  int iVar1;
  PanelKeyBindingMap *pPVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x609f0c) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0x609f0c);
    if (iVar1 != 0) {
      pPVar2 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x384d78));
      *(PanelKeyBindingMap **)(unaff_EBX + 0x609f3c) = pPVar2;
      ___cxa_guard_release(unaff_EBX + 0x609f0c);
    }
  }
  return *(PanelKeyBindingMap **)(unaff_EBX + 0x609f3c);
}


/* CRadioChoicesQuestion::GetAnimMap at 0061b010 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: PanelAnimationMap * GetAnimMap(CRadioChoicesQuestion * this) */

PanelAnimationMap * __thiscall CRadioChoicesQuestion::GetAnimMap(CRadioChoicesQuestion *this)

{
  PanelAnimationMap *pPVar1;
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  pPVar1 = FindOrAddPanelAnimationMap((char *)(extraout_ECX + 0x384d04));
  return pPVar1;
}


/* CRadioChoicesQuestion::GetMessageMap at 0061b030 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: PanelMessageMap * GetMessageMap(CRadioChoicesQuestion * this) */

PanelMessageMap * __thiscall CRadioChoicesQuestion::GetMessageMap(CRadioChoicesQuestion *this)

{
  int iVar1;
  PanelMessageMap *pPVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x609e64) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0x609e64);
    if (iVar1 != 0) {
      pPVar2 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x384cd8));
      *(PanelMessageMap **)(unaff_EBX + 0x609ea0) = pPVar2;
      ___cxa_guard_release(unaff_EBX + 0x609e64);
    }
  }
  return *(PanelMessageMap **)(unaff_EBX + 0x609ea0);
}


/* CRadioChoicesQuestion::~CRadioChoicesQuestion at 0061b720 */

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
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a3f9a);
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
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a475a);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,in_stack_ffffffd8);
  operator_delete(this);
  return;
}


/* CRadioChoicesQuestion::~CRadioChoicesQuestion at 0061b870 */

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
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a3e4a);
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
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a460a);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,__in_chrg);
  return;
}


/* CRadioChoicesQuestion::SetData at 00619150 */

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
    parent_1 = (Panel *)(unaff_EBX + 0x34510e);
    this_1 = &UNK_00386e91 + unaff_EBX;
LAB_006191fa:
    do {
      pcVar2 = KeyValues::GetName(this_00);
      iVar3 = _V_stricmp((char *)(unaff_EBX + 0x36db76),pcVar2);
      if (iVar3 == 0) {
        p_Var1 = (this->m_pQuestionText->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd4];
        pcVar2 = KeyValues::GetString(this_00,(char *)0x0,(char *)parent_1);
        (*p_Var1)(this->m_pQuestionText,pcVar2);
      }
      else {
        pcVar2 = KeyValues::GetName(this_00);
        iVar3 = _V_stricmp(&UNK_00386e8a + unaff_EBX,pcVar2);
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
          goto LAB_006191fa;
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


/* CRadioChoicesQuestion::GetQuestionText at 00617f90 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void GetQuestionText(CRadioChoicesQuestion * this, char * pBuffOut, int
   iBuffLen) */

void __thiscall
CRadioChoicesQuestion::GetQuestionText(CRadioChoicesQuestion *this,char *pBuffOut,int iBuffLen)

{
                    /* WARNING: Could not recover jumptable at 0x00617fa8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->m_pQuestionText->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd6])();
  return;
}


/* CRadioChoicesQuestion::GetAnswerText at 00618200 */

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
                    /* WARNING: Could not recover jumptable at 0x00618299. Too many branches */
                    /* WARNING: Treating indirect jump as call */
          (**(code **)(*(int *)&(this->m_pRadioButtons[iVar3]->super_ToggleButton).super_Button.
                                super_Label.super_Panel.super_IClientPanel + 0x358))();
          return;
        }
      }
      iVar3 = iVar3 + 1;
    } while (iVar3 < this->m_iNumRadioButtons);
  }
  V_strncpy(pBuffOut,*(char **)(unaff_EBX + 0x5a809e),iBuffLen);
  return;
}


/* COneToTenQuestion::GetPanelClassName at 0061a540 */

char * COneToTenQuestion::GetPanelClassName(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (char *)(extraout_ECX + 0x3857ea);
}


/* COneToTenQuestion::KB_ChainToMap at 0061a600 */

void COneToTenQuestion::KB_ChainToMap(void)

{
  PanelKeyBindingMap *pPVar1;
  PanelKeyBindingMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x60a8ec) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x60a8ec) = 1;
    pPVar1 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x385721));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x5dd567);
    pPVar2 = vgui::FindOrAddPanelKeyBindingMap(&UNK_003856cf + unaff_EBX);
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* COneToTenQuestion::ChainToAnimationMap at 0061a7e0 */

void COneToTenQuestion::ChainToAnimationMap(void)

{
  PanelAnimationMap *pPVar1;
  PanelAnimationMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelAnimationMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x60a70b) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x60a70b) = 1;
    pPVar1 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x385541));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x5dd387);
    pPVar2 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x3854ef));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* COneToTenQuestion::ChainToMap at 0061a9c0 */

void COneToTenQuestion::ChainToMap(void)

{
  PanelMessageMap *pPVar1;
  PanelMessageMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelMessageMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x60a52a) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x60a52a) = 1;
    pPVar1 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x385361));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x5dd1a7);
    pPVar2 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x38530f));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* COneToTenQuestion::GetKBMap at 0061b0b0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: PanelKeyBindingMap * GetKBMap(COneToTenQuestion * this) */

PanelKeyBindingMap * __thiscall COneToTenQuestion::GetKBMap(COneToTenQuestion *this)

{
  int iVar1;
  PanelKeyBindingMap *pPVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x609dfc) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0x609dfc);
    if (iVar1 != 0) {
      pPVar2 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x384c6e));
      *(PanelKeyBindingMap **)(unaff_EBX + 0x609e24) = pPVar2;
      ___cxa_guard_release(unaff_EBX + 0x609dfc);
    }
  }
  return *(PanelKeyBindingMap **)(unaff_EBX + 0x609e24);
}


/* COneToTenQuestion::GetAnimMap at 0061b130 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: PanelAnimationMap * GetAnimMap(COneToTenQuestion * this) */

PanelAnimationMap * __thiscall COneToTenQuestion::GetAnimMap(COneToTenQuestion *this)

{
  PanelAnimationMap *pPVar1;
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  pPVar1 = FindOrAddPanelAnimationMap((char *)(extraout_ECX + 0x384bfa));
  return pPVar1;
}


/* COneToTenQuestion::GetMessageMap at 0061b150 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: PanelMessageMap * GetMessageMap(COneToTenQuestion * this) */

PanelMessageMap * __thiscall COneToTenQuestion::GetMessageMap(COneToTenQuestion *this)

{
  int iVar1;
  PanelMessageMap *pPVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x609d54) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0x609d54);
    if (iVar1 != 0) {
      pPVar2 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x384bce));
      *(PanelMessageMap **)(unaff_EBX + 0x609d88) = pPVar2;
      ___cxa_guard_release(unaff_EBX + 0x609d54);
    }
  }
  return *(PanelMessageMap **)(unaff_EBX + 0x609d88);
}


/* COneToTenQuestion::~COneToTenQuestion at 0061b320 */

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
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a3fba);
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
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a4b5a);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,in_stack_ffffffd8);
  operator_delete(this);
  return;
}


/* COneToTenQuestion::~COneToTenQuestion at 0061b5b0 */

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
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a3d2a);
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
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a48ca);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,__in_chrg);
  return;
}


/* COneToTenQuestion::SetData at 00618f10 */

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
    defaultValue = (char *)(unaff_EBX + 0x34534e);
    do {
      while( true ) {
        pcVar2 = KeyValues::GetName(this_00);
        iVar3 = _V_stricmp((char *)(unaff_EBX + 0x36ddb6),pcVar2);
        if (iVar3 == 0) break;
        pcVar2 = KeyValues::GetName(this_00);
        iVar3 = _V_stricmp((char *)(unaff_EBX + 0x3870b4),pcVar2);
        if (iVar3 != 0) {
          pcVar2 = KeyValues::GetName(this_00);
          iVar3 = _V_stricmp((char *)(unaff_EBX + 0x3870bf),pcVar2);
          if (iVar3 == 0) {
            p_Var1 = (this->m_pLabelAtTen->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd4];
            pcVar2 = KeyValues::GetString(this_00,(char *)0x0,defaultValue);
            (*p_Var1)(this->m_pLabelAtTen,pcVar2);
          }
          goto LAB_00618f98;
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
LAB_00618f98:
      this_00 = KeyValues::GetNextKey(this_00);
    } while (this_00 != (KeyValues *)0x0);
  }
  return true;
}


/* COneToTenQuestion::GetQuestionText at 00617fb0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void GetQuestionText(COneToTenQuestion * this, char * pBuffOut, int
   iBuffLen) */

void __thiscall
COneToTenQuestion::GetQuestionText(COneToTenQuestion *this,char *pBuffOut,int iBuffLen)

{
                    /* WARNING: Could not recover jumptable at 0x00617fc8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->m_pQuestionText->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd6])();
  return;
}


/* COneToTenQuestion::GetAnswerText at 00618040 */

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
      goto LAB_00618193;
    }
  }
  pRVar1 = this->m_pRadioButtons[1];
  if (pRVar1 != (RadioButton *)0x0) {
    iVar2 = (*(pRVar1->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
              _vptr_IClientPanel[0x101])(pRVar1);
    if ((char)iVar2 != '\0') {
      iVar2 = 1;
      goto LAB_00618193;
    }
  }
  pRVar1 = this->m_pRadioButtons[2];
  if (pRVar1 != (RadioButton *)0x0) {
    iVar2 = (*(pRVar1->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
              _vptr_IClientPanel[0x101])(pRVar1);
    if ((char)iVar2 != '\0') {
      iVar2 = 2;
      goto LAB_00618193;
    }
  }
  pRVar1 = this->m_pRadioButtons[3];
  if (pRVar1 != (RadioButton *)0x0) {
    iVar2 = (*(pRVar1->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
              _vptr_IClientPanel[0x101])(pRVar1);
    if ((char)iVar2 != '\0') {
      iVar2 = 3;
      goto LAB_00618193;
    }
  }
  pRVar1 = this->m_pRadioButtons[4];
  if (pRVar1 != (RadioButton *)0x0) {
    iVar2 = (*(pRVar1->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
              _vptr_IClientPanel[0x101])(pRVar1);
    if ((char)iVar2 != '\0') {
      iVar2 = 4;
      goto LAB_00618193;
    }
  }
  pRVar1 = this->m_pRadioButtons[5];
  if (pRVar1 != (RadioButton *)0x0) {
    iVar2 = (*(pRVar1->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
              _vptr_IClientPanel[0x101])(pRVar1);
    if ((char)iVar2 != '\0') {
      iVar2 = 5;
      goto LAB_00618193;
    }
  }
  pRVar1 = this->m_pRadioButtons[6];
  if (pRVar1 != (RadioButton *)0x0) {
    iVar2 = (*(pRVar1->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
              _vptr_IClientPanel[0x101])(pRVar1);
    if ((char)iVar2 != '\0') {
      iVar2 = 6;
      goto LAB_00618193;
    }
  }
  pRVar1 = this->m_pRadioButtons[7];
  if (pRVar1 != (RadioButton *)0x0) {
    iVar2 = (*(pRVar1->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
              _vptr_IClientPanel[0x101])(pRVar1);
    if ((char)iVar2 != '\0') {
      iVar2 = 7;
      goto LAB_00618193;
    }
  }
  pRVar1 = this->m_pRadioButtons[8];
  if (pRVar1 != (RadioButton *)0x0) {
    iVar2 = (*(pRVar1->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
              _vptr_IClientPanel[0x101])(pRVar1);
    if ((char)iVar2 != '\0') {
      iVar2 = 8;
      goto LAB_00618193;
    }
  }
  pRVar1 = this->m_pRadioButtons[9];
  if (pRVar1 != (RadioButton *)0x0) {
    iVar2 = (*(pRVar1->super_ToggleButton).super_Button.super_Label.super_Panel.super_IClientPanel.
              _vptr_IClientPanel[0x101])(pRVar1);
    if ((char)iVar2 != '\0') {
      iVar2 = 9;
LAB_00618193:
                    /* WARNING: Could not recover jumptable at 0x006181b5. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (*(this->m_pRadioButtons[iVar2]->super_ToggleButton).super_Button.super_Label.super_Panel.
        super_IClientPanel._vptr_IClientPanel[0xd6])();
      return;
    }
  }
  V_strncpy(pBuffOut,*(char **)(unaff_EBX + 0x5a825e),iBuffLen);
  return;
}


/* CSurveyPanel::CSurveyPanel at 006195f0 */

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
  vgui::Frame::Frame(&this->super_Frame,(Panel *)0x0,(char *)(unaff_EBX + 0x371aa3),true,true);
  (this->super_Frame).super_EditablePanel.super_Panel.super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5a580a);
  (this->super_IViewPortPanel)._vptr_IViewPortPanel =
       (_func_int_varargs **)(&UNK_005a5c8e + unaff_EBX);
  if (*(char *)(unaff_EBX + 0x60b906) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x60b906) = 1;
    pPVar4 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3866f2));
    pPVar4->pfnClassName = *(_func_char_ptr **)(&DAT_005de57e + unaff_EBX);
    pPVar5 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x36ae96));
    pPVar4->baseMap = pPVar5;
    cVar1 = *(char *)(unaff_EBX + 0x60b907);
  }
  else {
    cVar1 = *(char *)(unaff_EBX + 0x60b907);
  }
  if (cVar1 == '\0') {
    *(undefined1 *)(unaff_EBX + 0x60b907) = 1;
    pPVar6 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x3866f2));
    pPVar6->pfnClassName = *(_func_char_ptr **)(&DAT_005de57e + unaff_EBX);
    pPVar7 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x36ae96));
    pPVar6->baseMap = pPVar7;
  }
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
  if (*(char *)(unaff_EBX + 0x60b908) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x60b908) = 1;
    pPVar2 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3866f2));
    pPVar2->pfnClassName = *(_func_char_ptr **)(&DAT_005de57e + unaff_EBX);
    pPVar3 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x36ae96));
    pPVar2->baseMap = pPVar3;
  }
  (this->m_hPointSurveyEntity).super_CBaseHandle.m_Index = 0xffffffff;
  this->id = -1;
  this->version = -1;
  this->m_pViewPort = pViewport;
  this_00 = operator_new(500);
  panelName_1 = (char *)0x0;
  parent_1 = (Panel *)0x0;
  this_1 = (PanelListPanel *)(unaff_EBX + 0x344c6e);
  vgui::Button::Button
            (this_00,(Panel *)this,&UNK_00386963 + unaff_EBX,(char *)this_1,(Panel *)0x0,(char *)0x0
             ,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8,
             in_stack_ffffffdc,in_stack_ffffffe0);
  this->m_pSubmitButton = this_00;
  this_01 = operator_new(0x1ac);
  vgui::PanelListPanel::PanelListPanel
            (this_01,(Panel *)this,(char *)(unaff_EBX + 0x386b1b),this_1,parent_1,panelName_1);
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
            (&this->super_Frame,(char *)(unaff_EBX + 0x386b28),(char *)0x0,(KeyValues *)0x0,
             (KeyValues *)0x0);
  return;
}


/* CSurveyPanel::CSurveyPanel at 00619880 */

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


/* CSurveyPanel::~CSurveyPanel at 00618d70 */

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
       (_func_int_varargs **)(unaff_EBX + 0x5a6084);
  (this->super_IViewPortPanel)._vptr_IViewPortPanel = (_func_int_varargs **)(unaff_EBX + 0x5a6508);
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


/* CSurveyPanel::~CSurveyPanel at 00618e10 */

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
       (_func_int_varargs **)(&UNK_005a5fe4 + unaff_EBX);
  (this->super_IViewPortPanel)._vptr_IViewPortPanel = (_func_int_varargs **)(unaff_EBX + 0x5a6468);
  (*(this->m_pQuestionsList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd8])
            (this->m_pQuestionsList);
  pBVar1 = this->m_pSubmitButton;
  if (pBVar1 != (Button *)0x0) {
    (*(pBVar1->super_Label).super_Panel.super_IClientPanel._vptr_IClientPanel[0x21])(pBVar1);
  }
  vgui::Frame::~Frame(&this->super_Frame,__in_chrg);
  return;
}


/* CSurveyPanel::~CSurveyPanel at 00618e90 */

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
       (_func_int_varargs **)(unaff_EBX + 0x5a5f64);
  (this->super_IViewPortPanel)._vptr_IViewPortPanel = (_func_int_varargs **)(unaff_EBX + 0x5a63e8);
  (*(this->m_pQuestionsList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd8])
            (this->m_pQuestionsList);
  pBVar1 = this->m_pSubmitButton;
  if (pBVar1 != (Button *)0x0) {
    (*(pBVar1->super_Label).super_Panel.super_IClientPanel._vptr_IClientPanel[0x21])(pBVar1);
  }
  vgui::Frame::~Frame(&this->super_Frame,__in_chrg);
  return;
}


/* CSurveyPanel::Reset at 00617fe0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void Reset(CSurveyPanel * this) */

void __thiscall CSurveyPanel::Reset(CSurveyPanel *this)

{
                    /* WARNING: Could not recover jumptable at 0x00617ff8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->m_pQuestionsList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd8])();
  return;
}


/* CSurveyPanel::Update at 00618010 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void Update(CSurveyPanel * this) */

void __thiscall CSurveyPanel::Update(CSurveyPanel *this)

{
  return;
}


/* CSurveyPanel::NeedsUpdate at 00618030 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* DWARF original prototype: bool NeedsUpdate(CSurveyPanel * this) */

bool __thiscall CSurveyPanel::NeedsUpdate(CSurveyPanel *this)

{
  return false;
}


/* CSurveyPanel::ShowPanel at 00619500 */

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
                    /* WARNING: Could not recover jumptable at 0x0061957d. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*(this->m_pQuestionsList->super_Panel).super_IClientPanel._vptr_IClientPanel[0x22])();
    return;
  }
  return;
}


/* CSurveyPanel::SendSurveyDoneCmd at 006193e0 */

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
      (iVar5 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5179a2),
      *(uint *)(iVar5 + 8) != uVar1 >> 0x10)) ||
     (piVar4 = *(int **)(iVar5 + 4), piVar4 == (int *)0x0)) {
    uVar6 = 0xffffffff;
  }
  else {
    uVar6 = (**(code **)(*piVar4 + 0x144))(piVar4);
  }
  CUtlString::Format(&local_2c,(char *)(unaff_EBX + 0x37681a),uVar6);
  puVar2 = *(undefined4 **)(unaff_EBX + 0x51796a);
  pcVar3 = *(code **)(*(int *)*puVar2 + 0x1c);
  pcVar7 = CUtlString::operator_const_char_(&local_2c);
  (*pcVar3)(*puVar2,pcVar7);
  if ((-1 < local_2c.m_Storage.m_Memory.m_nGrowSize) &&
     (local_2c.m_Storage.m_Memory.m_pMemory != (uchar *)0x0)) {
    local_2c.m_Storage.m_nActualLength = 0;
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x517936) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x517936),local_2c.m_Storage.m_Memory.m_pMemory)
    ;
  }
  return;
}


/* CSurveyPanel::OnCommand at 00618920 */

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
  local_20 = **(int **)(unaff_EBX + 0x5183e3);
  iVar1 = _V_stricmp(command,(char *)(unaff_EBX + 0x387630));
  if (iVar1 == 0) {
    if (*(int *)(*(int *)(unaff_EBX + 0x738c6b) + 0x30) == 0) {
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
                  (*(C_CPortalGameStats **)(unaff_EBX + 0x518d1b),difficulty,enjoyment,local_46c);
      }
      _Plat_GetLocalTime(&local_698);
      pcVar3 = (char *)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x518427) + 0xcc))
                                 ((int *)**(undefined4 **)(unaff_EBX + 0x518427));
      V_FileBase(pcVar3,local_60,0x40);
      this_00 = C_BasePlayer::GetLocalPlayer(-1);
      pcVar3 = C_BasePlayer::GetPlayerName(this_00);
      pcVar5 = this->m_szSurveyName;
      V_snprintf(local_160,0x100,(char *)(unaff_EBX + 0x387637),pcVar5,local_60,pcVar3,
                 local_684 + 0x76c,local_688 + 1,local_68c,local_690,local_694,local_698);
      V_snprintf(local_66c,0x200,(char *)(unaff_EBX + 0x387663),pcVar5);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5183eb) + 0x50))
                ((int *)**(undefined4 **)(unaff_EBX + 0x5183eb),local_66c,0);
      V_strncat(local_66c,local_160,0x200,-1);
      WriteSurveyDataToFile(this,local_66c);
      V_snprintf(local_160,0x100,(char *)(unaff_EBX + 0x387689),pcVar5);
      V_snprintf(local_66c,0x200,(char *)(unaff_EBX + 0x387690));
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5183eb) + 0x50))
                ((int *)**(undefined4 **)(unaff_EBX + 0x5183eb),local_66c,0);
      V_strncat(local_66c,local_160,0x200,-1);
      WriteSurveyDataToFile(this,local_66c);
    }
  }
  else {
    iVar1 = _V_stricmp(command,(char *)(unaff_EBX + 0x379391));
    if (iVar1 != 0) goto LAB_00618991;
  }
  pIVar2 = GetViewPortInterface();
  (*pIVar2->_vptr_IViewPort[2])(pIVar2,&UNK_00372770 + unaff_EBX,0);
LAB_00618991:
  if (local_20 != **(int **)(unaff_EBX + 0x5183e3)) {
                    /* WARNING: Subroutine does not return */
    ___stack_chk_fail();
  }
  return;
}


/* cl_show_portal_puzzle_survey at 00618500 */

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


/* __tcf_3 at 00952cf0 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x36dec8),in_stack_00000008);
  return;
}


/* ShowSurveyPanel at 006182a0 */

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
  local_20 = **(int **)(unaff_EBX + 0x518a5d);
  pIVar2 = GetViewPortInterface();
  if ((pIVar2 == (IViewPort *)0x0) || (*(int *)(*(int *)(unaff_EBX + 0x739285) + 0x30) != 0)) {
    bVar1 = false;
    goto LAB_006182e8;
  }
  pIVar3 = GetClientMode();
  iVar4 = (*pIVar3->_vptr_IClientMode[10])(pIVar3);
  if ((iVar4 == 0) ||
     (piVar5 = (int *)___dynamic_cast(iVar4,*(undefined4 *)(unaff_EBX + 0x518f35),
                                      *(undefined4 *)(unaff_EBX + 0x518f3d),0), piVar5 == (int *)0x0
     )) {
LAB_00618470:
    _Warning(unaff_EBX + 0x387a9d);
    bVar1 = false;
  }
  else {
    iVar4 = unaff_EBX + 0x372dea;
    local_130 = (int *)(**(code **)(*piVar5 + 0x3b8))(piVar5,iVar4);
    if (local_130 == (int *)0x0) {
      local_130 = (int *)(**(code **)(*piVar5 + 0x3b4))(piVar5,iVar4);
      (**(code **)(*piVar5 + 0x3d4))(piVar5,local_130,&UNK_00387a8d + unaff_EBX);
      if (local_130 == (int *)0x0) goto LAB_00618470;
    }
    V_snprintf(local_124,0x104,(char *)(unaff_EBX + 0x387adb),pszSurveyName);
    this = KeyValues::operator_new(0x24);
    KeyValues::KeyValues(this,pszSurveyName);
    filesystem = (IBaseFileSystem *)0x0;
    if (**(int **)(unaff_EBX + 0x518a65) != 0) {
      filesystem = (IBaseFileSystem *)(**(int **)(unaff_EBX + 0x518a65) + 4);
    }
    bVar1 = KeyValues::LoadFromFile
                      (this,filesystem,local_124,(char *)(unaff_EBX + 0x355d55),(GetSymbolProc_t)0x0
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
      _Warning(unaff_EBX + 0x387af5,pszSurveyName);
      bVar1 = false;
    }
  }
LAB_006182e8:
  if (local_20 != **(int **)(unaff_EBX + 0x518a5d)) {
                    /* WARNING: Subroutine does not return */
    ___stack_chk_fail();
  }
  return bVar1;
}


/* _GLOBAL__I_cl_disable_survey_panel at 00082e50 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_cl_disable_survey_panel(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

