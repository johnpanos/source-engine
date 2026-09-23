/* DWARF-guided pseudocode for game/client/portal2/vgui/vgui_mp_lobby_screen.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* CObjectControlPanel::~CObjectControlPanel at 0062c360 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CObjectControlPanel(CObjectControlPanel * this, int __in_chrg) */

void __thiscall CObjectControlPanel::~CObjectControlPanel(CObjectControlPanel *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cab77);
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x5380ef) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CObjectControlPanel::~CObjectControlPanel at 0062c510 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CObjectControlPanel(CObjectControlPanel * this, int __in_chrg) */

void __thiscall CObjectControlPanel::~CObjectControlPanel(CObjectControlPanel *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(extraout_ECX + 0x5ca9d0);
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(*(int *)(&DAT_00537f48 + extraout_ECX) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,__in_chrg);
  return;
}


/* CVGUI_MP_LobbyScreen::~CVGUI_MP_LobbyScreen at 0062c540 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CVGUI_MP_LobbyScreen(CVGUI_MP_LobbyScreen * this, int __in_chrg)
    */

void __thiscall
CVGUI_MP_LobbyScreen::~CVGUI_MP_LobbyScreen(CVGUI_MP_LobbyScreen *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CObjectControlPanel).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(extraout_ECX + 0x5ca9a0);
  (this->super_CObjectControlPanel).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(*(int *)(&DAT_00537f18 + extraout_ECX) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,__in_chrg);
  return;
}


/* CVGUI_MP_LobbyScreen::~CVGUI_MP_LobbyScreen at 0062c570 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CVGUI_MP_LobbyScreen(CVGUI_MP_LobbyScreen * this, int __in_chrg)
    */

void __thiscall
CVGUI_MP_LobbyScreen::~CVGUI_MP_LobbyScreen(CVGUI_MP_LobbyScreen *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CObjectControlPanel).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5ca967);
  (this->super_CObjectControlPanel).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(*(int *)(&DAT_00537edf + unaff_EBX) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CObjectControlPanel::TickCurrentPanel at 0062b340 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: Panel * TickCurrentPanel(CObjectControlPanel * this) */

Panel * __thiscall CObjectControlPanel::TickCurrentPanel(CObjectControlPanel *this)

{
  uint uVar1;
  EditablePanel *pEVar2;
  Panel *pPVar3;
  bool bOldState;
  C_BasePlayer *pCVar4;
  int iVar5;
  int unaff_EBX;
  CHackForGetLocalPlayerAccessAllowedGuard local_28 [2];
  
                    /* Unresolved local var: CHackForGetLocalPlayerAccessAllowedGuard
                       g_HackGLPGuard@[???]
                       Unresolved local var: C_Portal_Player * pLocalPlayer@[???]
                       Unresolved local var: C_BaseEntity * pObj@[???] */
  ___i686_get_pc_thunk_bx();
  bOldState = (bool)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x538a56) + 0x204))
                              ((int *)**(undefined4 **)(unaff_EBX + 0x538a56));
  CHackForGetLocalPlayerAccessAllowedGuard::CHackForGetLocalPlayerAccessAllowedGuard
            (local_28,(char *)(unaff_EBX + 0x3a0526),bOldState);
  pCVar4 = C_BasePlayer::GetLocalPlayer(-1);
  uVar1 = (this->super_CVGuiScreenPanel).m_hEntity.super_CBaseHandle.m_Index;
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???]
                       Unresolved local var: CEntInfo * pInfo@[???] */
  if ((((uVar1 == 0xffffffff) ||
       (iVar5 = (uVar1 & 0xffff) * 0x10 + **(int **)(&DAT_00538a8e + unaff_EBX),
       *(uint *)(iVar5 + 8) != uVar1 >> 0x10)) || (iVar5 = *(int *)(iVar5 + 4), iVar5 == 0)) ||
     (((uVar1 = *(uint *)(iVar5 + 300), uVar1 == 0xffffffff ||
       (iVar5 = (uVar1 & 0xffff) * 0x10 + **(int **)(&DAT_00538a8e + unaff_EBX),
       *(uint *)(iVar5 + 8) != uVar1 >> 0x10)) || (iVar5 = *(int *)(iVar5 + 4), iVar5 == 0)))) {
    iVar5 = 0;
  }
  pEVar2 = this->m_pActivePanel;
  this->m_pCurrentPanel = &pEVar2->super_Panel;
  (*(pEVar2->super_Panel).super_IClientPanel._vptr_IClientPanel[0x40])(pEVar2,2);
  (*(this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
    _vptr_IClientPanel[0xee])(this,iVar5,pCVar4);
  pPVar3 = this->m_pCurrentPanel;
  CHackForGetLocalPlayerAccessAllowedGuard::~CHackForGetLocalPlayerAccessAllowedGuard
            (local_28,iVar5);
  return pPVar3;
}


/* CCommandChainingPanel::~CCommandChainingPanel at 0062be30 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CCommandChainingPanel(CCommandChainingPanel * this, int
   __in_chrg) */

void __thiscall
CCommandChainingPanel::~CCommandChainingPanel(CCommandChainingPanel *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5cc107);
  vgui::EditablePanel::~EditablePanel(&this->super_EditablePanel,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CCommandChainingPanel::~CCommandChainingPanel at 0062be70 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CCommandChainingPanel(CCommandChainingPanel * this, int
   __in_chrg) */

void __thiscall
CCommandChainingPanel::~CCommandChainingPanel(CCommandChainingPanel *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(extraout_ECX + 0x5cc0d0);
  vgui::EditablePanel::~EditablePanel(&this->super_EditablePanel,__in_chrg);
  return;
}


/* CObjectControlPanel::CObjectControlPanel at 0062bc10 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void CObjectControlPanel(CObjectControlPanel * this, Panel * parent,
   char * panelName) */

void __thiscall
CObjectControlPanel::CObjectControlPanel(CObjectControlPanel *this,Panel *parent,char *panelName)

{
  EditablePanel *this_00;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CVGuiScreenPanel::CVGuiScreenPanel(&this->super_CVGuiScreenPanel,parent,panelName,0);
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(&UNK_005cb2ca + unaff_EBX);
  vgui::Panel::SetKeyBoardInputEnabled((Panel *)this,false);
  vgui::Panel::SetMouseInputEnabled((Panel *)this,true);
  this_00 = operator_new(0x17c);
  vgui::EditablePanel::EditablePanel(this_00,(Panel *)this,(char *)(unaff_EBX + 0x39fda5));
  (this_00->super_Panel).super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5cc32a);
  vgui::Panel::SetPaintBackgroundEnabled(&this_00->super_Panel,false);
  this->m_pActivePanel = this_00;
  vgui::Panel::SetCursor((Panel *)this,2);
  vgui::Panel::SetZPos(&this->m_pActivePanel->super_Panel,-1);
  return;
}


/* CCommandChainingPanel::OnCommand at 0062be90 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void OnCommand(CCommandChainingPanel * this, char * command) */

void __thiscall CCommandChainingPanel::OnCommand(CCommandChainingPanel *this,char *command)

{
  int iVar1;
  int *piVar2;
  
  vgui::Panel::OnCommand((Panel *)this,command);
  iVar1 = (*(this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel[0x27])
                    (this);
  if (iVar1 != 0) {
    piVar2 = (int *)(*(this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel
                      [0x27])(this);
                    /* WARNING: Could not recover jumptable at 0x0062bedf. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (**(code **)(*piVar2 + 0x184))();
    return;
  }
  return;
}


/* CObjectControlPanel::CObjectControlPanel at 0062be20 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void CObjectControlPanel(CObjectControlPanel * this, Panel * parent,
   char * panelName, CObjectControlPanel * this, Panel * parent, char * panelName) */

void __thiscall
CObjectControlPanel::CObjectControlPanel
          (CObjectControlPanel *this,Panel *parent,char *panelName,CObjectControlPanel *this_1,
          Panel *parent_1,char *panelName_1)

{
  CObjectControlPanel(this,parent,panelName);
  return;
}


/* CPanelFactory<CObjectControlPanel,VGuiScreenInitData_t>::Create at 0062c3b0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: Panel * Create(CPanelFactory<CObjectControlPanel,VGuiScreenInitData_t>
   * this, char * pMetaClassName, KeyValues * pKeyValues, void * pVoidInitData, Panel * pParent) */

Panel * __thiscall
CPanelFactory<CObjectControlPanel,VGuiScreenInitData_t>::Create
          (CPanelFactory<CObjectControlPanel,VGuiScreenInitData_t> *this,char *pMetaClassName,
          KeyValues *pKeyValues,void *pVoidInitData,Panel *pParent)

{
  CObjectControlPanel *this_00;
  int iVar1;
  CObjectControlPanel *pCVar2;
  
                    /* Unresolved local var: CObjectControlPanel * pPanel@[???] */
  this_00 = operator_new(0x188);
  CObjectControlPanel::CObjectControlPanel(this_00,pParent,pMetaClassName);
  pCVar2 = (CObjectControlPanel *)0x0;
  if (this_00 != (CObjectControlPanel *)0x0) {
    iVar1 = (*(this_00->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
              _vptr_IClientPanel[0xed])(this_00,pKeyValues,pVoidInitData);
    pCVar2 = this_00;
    if ((char)iVar1 == '\0') {
      (*(this_00->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
        _vptr_IClientPanel[0x21])(this_00);
      return (Panel *)0x0;
    }
  }
  return (Panel *)pCVar2;
}


/* CObjectControlPanel::Init at 0062a020 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: bool Init(CObjectControlPanel * this, KeyValues * pKeyValues,
   VGuiScreenInitData_t * pInitData) */

bool __thiscall
CObjectControlPanel::Init
          (CObjectControlPanel *this,KeyValues *pKeyValues,VGuiScreenInitData_t *pInitData)

{
  int *piVar1;
  code *pcVar2;
  bool bVar3;
  int iVar4;
  int unaff_EBX;
  int local_2c;
  int local_28;
  int local_24;
  int local_20 [4];
  
                    /* Unresolved local var: int x@[???]
                       Unresolved local var: int y@[???]
                       Unresolved local var: int w@[???]
                       Unresolved local var: int h@[???] */
  ___i686_get_pc_thunk_bx();
  piVar1 = (int *)**(undefined4 **)(&DAT_0053a162 + unaff_EBX);
  pcVar2 = *(code **)(*piVar1 + 0x54);
  iVar4 = (**(this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
             _vptr_IClientPanel)(this);
  (*pcVar2)(piVar1,iVar4,0);
  bVar3 = CVGuiScreenPanel::Init(&this->super_CVGuiScreenPanel,pKeyValues,pInitData);
  if (bVar3) {
    (*(this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x40])(this,2);
    vgui::Panel::GetBounds((Panel *)this,local_20,&local_24,&local_28,&local_2c);
    vgui::Panel::SetBounds
              (&this->m_pActivePanel->super_Panel,local_20[0],local_24,local_28,local_2c);
    (*(this->m_pActivePanel->super_Panel).super_IClientPanel._vptr_IClientPanel[0x22])
              (this->m_pActivePanel,0);
    this->m_pCurrentPanel = &this->m_pActivePanel->super_Panel;
  }
  return bVar3;
}


/* CObjectControlPanel::GetOwningObject at 00629a60 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: C_BaseEntity * GetOwningObject(CObjectControlPanel * this) */

C_BaseEntity * __thiscall CObjectControlPanel::GetOwningObject(CObjectControlPanel *this)

{
  uint uVar1;
  C_BaseEntity *pCVar2;
  int iVar3;
  int unaff_EBX;
  
                    /* Unresolved local var: C_BaseEntity * pScreenEnt@[???]
                       Unresolved local var: C_BaseEntity * pObj@[???] */
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->super_CVGuiScreenPanel).m_hEntity.super_CBaseHandle.m_Index;
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???]
                       Unresolved local var: CEntInfo * pInfo@[???] */
  if ((((uVar1 == 0xffffffff) ||
       (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x53a373),
       *(uint *)(iVar3 + 8) != uVar1 >> 0x10)) || (iVar3 = *(int *)(iVar3 + 4), iVar3 == 0)) ||
     (((uVar1 = *(uint *)(iVar3 + 300), uVar1 == 0xffffffff ||
       (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x53a373),
       *(uint *)(iVar3 + 8) != uVar1 >> 0x10)) ||
      (pCVar2 = *(C_BaseEntity **)(iVar3 + 4), pCVar2 == (C_BaseEntity *)0x0)))) {
    pCVar2 = (C_BaseEntity *)0x0;
  }
  return pCVar2;
}


/* CObjectControlPanel::OnTick at 0062b460 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void OnTick(CObjectControlPanel * this) */

void __thiscall CObjectControlPanel::OnTick(CObjectControlPanel *this)

{
  uint uVar1;
  int iVar2;
  Panel *pPVar3;
  int unaff_EBX;
  
                    /* Unresolved local var: C_BaseEntity * pObj@[???] */
  ___i686_get_pc_thunk_bx();
  vgui::Panel::OnTick((Panel *)this);
  uVar1 = (this->super_CVGuiScreenPanel).m_hEntity.super_CBaseHandle.m_Index;
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???]
                       Unresolved local var: CEntInfo * pInfo@[???] */
  if ((((uVar1 != 0xffffffff) &&
       (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(&DAT_0053896f + unaff_EBX),
       *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) && (iVar2 = *(int *)(iVar2 + 4), iVar2 != 0)) &&
     (((uVar1 = *(uint *)(iVar2 + 300), uVar1 != 0xffffffff &&
       (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(&DAT_0053896f + unaff_EBX),
       *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) && (*(int *)(iVar2 + 4) != 0)))) {
    (*(this->m_pCurrentPanel->super_IClientPanel)._vptr_IClientPanel[0x22])(this->m_pCurrentPanel,0)
    ;
    pPVar3 = (Panel *)(*(this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.
                        super_IClientPanel._vptr_IClientPanel[0xef])(this);
    this->m_pCurrentPanel = pPVar3;
    (*(pPVar3->super_IClientPanel)._vptr_IClientPanel[0x22])(pPVar3,1);
    return;
  }
  return;
}


/* CObjectControlPanel::SendToServerObject at 0062b570 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void SendToServerObject(CObjectControlPanel * this, char * pMsg) */

void __thiscall CObjectControlPanel::SendToServerObject(CObjectControlPanel *this,char *pMsg)

{
  uint uVar1;
  int *piVar2;
  code *pcVar3;
  undefined4 *puVar4;
  int iVar5;
  int nOldSlot;
  int unaff_EBX;
  CSetActiveSplitScreenPlayerGuard *in_stack_ffffffac;
  char *in_stack_ffffffb0;
  int in_stack_ffffffb4;
  int in_stack_ffffffb8;
  int in_stack_ffffffbc;
  CSetActiveSplitScreenPlayerGuard *this_00;
  CSetActiveSplitScreenPlayerGuard local_3c;
  
                    /* Unresolved local var: C_BaseEntity * pObj@[???] */
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->super_CVGuiScreenPanel).m_hEntity.super_CBaseHandle.m_Index;
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???]
                       Unresolved local var: CEntInfo * pInfo@[???] */
  if ((((uVar1 != 0xffffffff) &&
       (iVar5 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x53885e),
       *(uint *)(iVar5 + 8) == uVar1 >> 0x10)) && (iVar5 = *(int *)(iVar5 + 4), iVar5 != 0)) &&
     (((uVar1 = *(uint *)(iVar5 + 300), uVar1 != 0xffffffff &&
       (iVar5 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x53885e),
       *(uint *)(iVar5 + 8) == uVar1 >> 0x10)) && (*(int *)(iVar5 + 4) != 0)))) {
    piVar2 = (int *)**(undefined4 **)(unaff_EBX + 0x538ca6);
    pcVar3 = *(code **)(*piVar2 + 0xf8);
    iVar5 = (**(this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
               _vptr_IClientPanel)(this);
    iVar5 = (*pcVar3)(piVar2,iVar5);
    puVar4 = *(undefined4 **)(unaff_EBX + 0x538826);
    piVar2 = (int *)*puVar4;
    nOldSlot = (**(code **)(*piVar2 + 0x1f8))(piVar2);
    this_00 = &local_3c;
    CSetActiveSplitScreenPlayerGuard::CSetActiveSplitScreenPlayerGuard
              (this_00,(char *)(unaff_EBX + 0x3a031e),0xb5,iVar5,nOldSlot,false,in_stack_ffffffac,
               in_stack_ffffffb0,in_stack_ffffffb4,in_stack_ffffffb8,in_stack_ffffffbc,
               SUB41(this_00,0));
    piVar2 = (int *)*puVar4;
    (**(code **)(*piVar2 + 0x18))(piVar2,pMsg,1);
    CSetActiveSplitScreenPlayerGuard::~CSetActiveSplitScreenPlayerGuard(this_00,(int)pMsg);
    return;
  }
  return;
}


/* CObjectControlPanel::OnTickActive at 00629ad0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void OnTickActive(CObjectControlPanel * this, C_BaseEntity * pObj,
   C_Portal_Player * pLocalPlayer) */

void __thiscall
CObjectControlPanel::OnTickActive
          (CObjectControlPanel *this,C_BaseEntity *pObj,C_Portal_Player *pLocalPlayer)

{
  return;
}


/* CObjectControlPanel::OnCommand at 00629af0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void OnCommand(CObjectControlPanel * this, char * command) */

void __thiscall CObjectControlPanel::OnCommand(CObjectControlPanel *this,char *command)

{
  CVGuiScreenPanel::OnCommand(&this->super_CVGuiScreenPanel,command);
  return;
}


/* CVGUI_MP_LobbyScreen::OnCommand at 0062b700 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void OnCommand(CVGUI_MP_LobbyScreen * this, char * pCommand) */

void __thiscall CVGUI_MP_LobbyScreen::OnCommand(CVGUI_MP_LobbyScreen *this,char *pCommand)

{
  int iVar1;
  int unaff_EBX;
  char local_40 [32];
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(unaff_EBX + 0x53864c);
  iVar1 = _V_stricmp(pCommand,(char *)(unaff_EBX + 0x3a0208));
  if (iVar1 == 0) {
    iVar1 = this->m_nCurrentDay + 1;
    this->m_nCurrentDay = iVar1;
    if (this->m_nMaxDays < iVar1) {
      this->m_nCurrentDay = 1;
    }
  }
  else {
    iVar1 = _V_stricmp(pCommand,(char *)(unaff_EBX + 0x3a0210));
    if (iVar1 != 0) goto LAB_0062b798;
    iVar1 = this->m_nCurrentDay + -1;
    this->m_nCurrentDay = iVar1;
    if (iVar1 < 1) {
      this->m_nCurrentDay = this->m_nMaxDays;
    }
  }
  UpdateDayLabel(this);
                    /* Unresolved local var: char[32] szServerMsg@[???] */
  V_snprintf(local_40,0x20,(char *)(unaff_EBX + 0x3a01f4),this->m_nCurrentDay);
  CObjectControlPanel::SendToServerObject(&this->super_CObjectControlPanel,local_40);
LAB_0062b798:
  CVGuiScreenPanel::OnCommand((CVGuiScreenPanel *)this,pCommand);
  if (local_20 == **(int **)(unaff_EBX + 0x53864c)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}


/* CVGUI_MP_LobbyScreen::CVGUI_MP_LobbyScreen at 0062bd20 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void CVGUI_MP_LobbyScreen(CVGUI_MP_LobbyScreen * this, Panel * parent,
   char * panelName) */

void __thiscall
CVGUI_MP_LobbyScreen::CVGUI_MP_LobbyScreen(CVGUI_MP_LobbyScreen *this,Panel *parent,char *panelName)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CObjectControlPanel::CObjectControlPanel
            (&this->super_CObjectControlPanel,parent,(char *)(unaff_EBX + 0x39fc9e));
  (this->super_CObjectControlPanel).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(&UNK_005cadd7 + unaff_EBX);
  *(undefined4 *)(this->m_cDefault)._color = 0;
  *(undefined4 *)(this->m_cInvisible)._color = 0;
  this->m_nStartDay = 4;
  this->m_nCurrentDay = 4;
  this->m_nMaxDays = 4;
  return;
}


/* CVGUI_MP_LobbyScreen::CVGUI_MP_LobbyScreen at 0062bda0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void CVGUI_MP_LobbyScreen(CVGUI_MP_LobbyScreen * this, Panel * parent,
   char * panelName) */

void __thiscall
CVGUI_MP_LobbyScreen::CVGUI_MP_LobbyScreen(CVGUI_MP_LobbyScreen *this,Panel *parent,char *panelName)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CObjectControlPanel::CObjectControlPanel
            (&this->super_CObjectControlPanel,parent,(char *)(unaff_EBX + 0x39fc1e));
  (this->super_CObjectControlPanel).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cad57);
  *(undefined4 *)(this->m_cDefault)._color = 0;
  *(undefined4 *)(this->m_cInvisible)._color = 0;
  this->m_nStartDay = 4;
  this->m_nCurrentDay = 4;
  this->m_nMaxDays = 4;
  return;
}


/* CPanelFactory<CVGUI_MP_LobbyScreen,VGuiScreenInitData_t>::Create at 0062c440 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: Panel * Create(CPanelFactory<CVGUI_MP_LobbyScreen,VGuiScreenInitData_t>
   * this, char * pMetaClassName, KeyValues * pKeyValues, void * pVoidInitData, Panel * pParent) */

Panel * __thiscall
CPanelFactory<CVGUI_MP_LobbyScreen,VGuiScreenInitData_t>::Create
          (CPanelFactory<CVGUI_MP_LobbyScreen,VGuiScreenInitData_t> *this,char *pMetaClassName,
          KeyValues *pKeyValues,void *pVoidInitData,Panel *pParent)

{
  CObjectControlPanel *pCVar1;
  char cVar2;
  CObjectControlPanel *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CVGUI_MP_LobbyScreen * pPanel@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = operator_new(0x1a8);
  CObjectControlPanel::CObjectControlPanel(this_00,pParent,(char *)(unaff_EBX + 0x39f57b));
  (this_00->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5ca6b4);
  pCVar1 = this_00 + 1;
  (pCVar1->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.m_OnCommand_register =
       (PanelMessageFunc_OnCommand)0x0;
  (pCVar1->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.m_OnMouseCaptureLost_register =
       (PanelMessageFunc_OnMouseCaptureLost)0x0;
  (pCVar1->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.m_OnSetFocus_register =
       (PanelMessageFunc_OnSetFocus)0x0;
  (pCVar1->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.m_OnKillFocus_register =
       (PanelMessageFunc_OnKillFocus)0x0;
  pCVar1 = this_00 + 1;
  (pCVar1->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.m_OnDelete_register =
       (PanelMessageFunc_OnDelete)0x0;
  (pCVar1->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.m_OnCursorMoved_register =
       (PanelMessageFunc_OnCursorMoved)0x0;
  (pCVar1->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.m_OnMouseFocusTicked_register =
       (PanelMessageFunc_OnMouseFocusTicked)0x0;
  (pCVar1->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.m_OnNavigateTo_register =
       (PanelMessageFunc_OnNavigateTo)0x0;
  this_00[1].super_CVGuiScreenPanel.super_EditablePanel.super_Panel.m_LastNavDirection = ND_BACK;
  pCVar1 = this_00 + 1;
  (pCVar1->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.m_InternalCursorMoved_register =
       (PanelMessageFunc_InternalCursorMoved)0x4;
  (pCVar1->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.m_InternalCursorEntered_register
       = (PanelMessageFunc_InternalCursorEntered)0x0;
  (pCVar1->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.m_InternalCursorExited_register =
       (PanelMessageFunc_InternalCursorExited)0x0;
  (pCVar1->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.m_InternalMousePressed_register =
       (PanelMessageFunc_InternalMousePressed)0x0;
  pCVar1 = this_00 + 1;
  (pCVar1->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.
  m_InternalMouseDoublePressed_register = (PanelMessageFunc_InternalMouseDoublePressed)0x4;
  (pCVar1->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.
  m_InternalMouseTriplePressed_register = (PanelMessageFunc_InternalMouseTriplePressed)0x0;
  (pCVar1->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.m_InternalMouseReleased_register
       = (PanelMessageFunc_InternalMouseReleased)0x0;
  (pCVar1->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.m_InternalMouseWheeled_register =
       (PanelMessageFunc_InternalMouseWheeled)0x0;
                    /* Unresolved local var: VGuiScreenInitData_t * pInitData@[???] */
  cVar2 = (**(code **)(unaff_EBX + 0x5caa68))(this_00,pKeyValues,pVoidInitData);
  if (cVar2 == '\0') {
    (*(this_00->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(this_00);
    this_00 = (CObjectControlPanel *)0x0;
  }
  return (Panel *)this_00;
}


/* CVGUI_MP_LobbyScreen::UpdateDayLabel at 00629ec0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void UpdateDayLabel(CVGUI_MP_LobbyScreen * this) */

void __thiscall CVGUI_MP_LobbyScreen::UpdateDayLabel(CVGUI_MP_LobbyScreen *this)

{
  int iVar1;
  Label *pLVar2;
  int *piVar3;
  int *piVar4;
  char cVar5;
  char *pFormat;
  int unaff_EBX;
  char local_a0 [128];
  int local_20;
  
                    /* Unresolved local var: char[128] szDayName@[???] */
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(unaff_EBX + 0x539e89);
  cVar5 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x539ee1) + 0x88))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x539ee1));
  if (cVar5 != '\0') {
                    /* Unresolved local var: C_PortalMPGameRules * g_pPortalMPGameRules@[???] */
    piVar3 = *(int **)(unaff_EBX + 0x53a131);
    piVar4 = (int *)*piVar3;
    cVar5 = (**(code **)(*piVar4 + 0x10c))(piVar4);
    if (cVar5 != '\0') {
      this->m_nCurrentDay = *(int *)(*piVar3 + 0x48);
      iVar1 = this->m_nCurrentDay;
      goto joined_r0x00629f9f;
    }
  }
  iVar1 = this->m_nCurrentDay;
joined_r0x00629f9f:
  if (iVar1 == 4) {
    pFormat = (char *)(unaff_EBX + 0x3a1787);
  }
  else {
    pFormat = (char *)(unaff_EBX + 0x3a1794);
  }
  V_snprintf(local_a0,0x80,pFormat);
  pLVar2 = this->m_pDayTextLabel;
  if (pLVar2 != (Label *)0x0) {
    (*(pLVar2->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd4])(pLVar2,local_a0);
  }
  if (local_20 == **(int **)(unaff_EBX + 0x539e89)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}


/* CVGUI_MP_LobbyScreen::ApplySchemeSettings at 0062b2b0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ApplySchemeSettings(CVGUI_MP_LobbyScreen * this, IScheme *
   pScheme) */

void __thiscall
CVGUI_MP_LobbyScreen::ApplySchemeSettings(CVGUI_MP_LobbyScreen *this,IScheme *pScheme)

{
  _func_int_varargs *p_Var1;
  int iVar2;
  Label *pLVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  p_Var1 = (pScheme->super_IBaseInterface)._vptr_IBaseInterface[6];
  iVar2 = (*(this->super_CObjectControlPanel).super_CVGuiScreenPanel.super_EditablePanel.super_Panel
            .super_IClientPanel._vptr_IClientPanel[0x3f])(this);
  iVar2 = (*p_Var1)(pScheme,unaff_EBX + 0x3a0588,iVar2);
  *(int *)(this->m_cDefault)._color = iVar2;
  *(undefined4 *)(this->m_cInvisible)._color = 0;
  pLVar3 = (Label *)vgui::Panel::FindChildByName((Panel *)this,(char *)(unaff_EBX + 0x3a05a5),false)
  ;
  this->m_pDayTextLabel = pLVar3;
  return;
}


/* CVGUI_MP_LobbyScreen::Init at 0062a120 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: bool Init(CVGUI_MP_LobbyScreen * this, KeyValues * pKeyValues,
   VGuiScreenInitData_t * pInitData) */

bool __thiscall
CVGUI_MP_LobbyScreen::Init
          (CVGUI_MP_LobbyScreen *this,KeyValues *pKeyValues,VGuiScreenInitData_t *pInitData)

{
  int *piVar1;
  code *pcVar2;
  bool bVar3;
  int iVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x53a05c);
  pcVar2 = *(code **)(*piVar1 + 0x54);
  iVar4 = (**(this->super_CObjectControlPanel).super_CVGuiScreenPanel.super_EditablePanel.
             super_Panel.super_IClientPanel._vptr_IClientPanel)(this);
  (*pcVar2)(piVar1,iVar4,0);
  bVar3 = CObjectControlPanel::Init(&this->super_CObjectControlPanel,pKeyValues,pInitData);
  return bVar3;
}


/* CVGUI_MP_LobbyScreen::NextDay at 00629ff0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void NextDay(CVGUI_MP_LobbyScreen * this) */

void __thiscall CVGUI_MP_LobbyScreen::NextDay(CVGUI_MP_LobbyScreen *this)

{
  int iVar1;
  
  iVar1 = this->m_nCurrentDay + 1;
  this->m_nCurrentDay = iVar1;
  if (this->m_nMaxDays < iVar1) {
    this->m_nCurrentDay = 1;
  }
  UpdateDayLabel(this);
  return;
}


/* CVGUI_MP_LobbyScreen::PreviousDay at 00629fc0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void PreviousDay(CVGUI_MP_LobbyScreen * this) */

void __thiscall CVGUI_MP_LobbyScreen::PreviousDay(CVGUI_MP_LobbyScreen *this)

{
  int iVar1;
  
  iVar1 = this->m_nCurrentDay + -1;
  this->m_nCurrentDay = iVar1;
  if (iVar1 < 1) {
    this->m_nCurrentDay = this->m_nMaxDays;
  }
  UpdateDayLabel(this);
  return;
}


/* CVGUI_MP_LobbyScreen::SelectDay at 0062b680 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void SelectDay(CVGUI_MP_LobbyScreen * this) */

void __thiscall CVGUI_MP_LobbyScreen::SelectDay(CVGUI_MP_LobbyScreen *this)

{
  int unaff_EBX;
  char local_40 [32];
  int local_20;
  
                    /* Unresolved local var: char[32] szServerMsg@[???] */
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(unaff_EBX + 0x5386cc);
  V_snprintf(local_40,0x20,(char *)(unaff_EBX + 0x3a0274),this->m_nCurrentDay);
  CObjectControlPanel::SendToServerObject(&this->super_CObjectControlPanel,local_40);
  if (local_20 == **(int **)(unaff_EBX + 0x5386cc)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}


/* CVGUI_MP_LobbyScreen::OnTick at 0062b520 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void OnTick(CVGUI_MP_LobbyScreen * this) */

void __thiscall CVGUI_MP_LobbyScreen::OnTick(CVGUI_MP_LobbyScreen *this)

{
  CObjectControlPanel::OnTick(&this->super_CObjectControlPanel);
  if (this->bIsAlreadyVisible == false) {
    (*(this->super_CObjectControlPanel).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
      super_IClientPanel._vptr_IClientPanel[0x22])(this,1);
    this->bIsAlreadyVisible = true;
  }
  UpdateDayLabel(this);
  return;
}


/* CVGUI_MP_LobbyScreen::Update at 00629ae0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_MP_LobbyScreen": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: void Update(CVGUI_MP_LobbyScreen * this, C_MP_LobbyScreen *
   pLobbyScreen) */

void __thiscall
CVGUI_MP_LobbyScreen::Update(CVGUI_MP_LobbyScreen *this,C_MP_LobbyScreen *pLobbyScreen)

{
  return;
}


/* CVGUI_MP_ProgressSignScreen::~CVGUI_MP_ProgressSignScreen at 0062bf30 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CVGUI_MP_ProgressSignScreen(CVGUI_MP_ProgressSignScreen * this,
   int __in_chrg) */

void __thiscall
CVGUI_MP_ProgressSignScreen::~CVGUI_MP_ProgressSignScreen
          (CVGUI_MP_ProgressSignScreen *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5ca7e7);
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(*(int *)(&DAT_0053851f + unaff_EBX) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CVGUI_MP_ProgressSignScreen::~CVGUI_MP_ProgressSignScreen at 0062bf80 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CVGUI_MP_ProgressSignScreen(CVGUI_MP_ProgressSignScreen * this,
   int __in_chrg) */

void __thiscall
CVGUI_MP_ProgressSignScreen::~CVGUI_MP_ProgressSignScreen
          (CVGUI_MP_ProgressSignScreen *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(extraout_ECX + 0x5ca7a0);
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(*(int *)(&DAT_005384d8 + extraout_ECX) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,__in_chrg);
  return;
}


/* CVGUI_MP_ProgressSignScreen::CVGUI_MP_ProgressSignScreen at 0062bb10 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void CVGUI_MP_ProgressSignScreen(CVGUI_MP_ProgressSignScreen * this,
   Panel * parent, char * panelName) */

void __thiscall
CVGUI_MP_ProgressSignScreen::CVGUI_MP_ProgressSignScreen
          (CVGUI_MP_ProgressSignScreen *this,Panel *parent,char *panelName)

{
  char *name;
  ImagePanel *this_00;
  int unaff_EBX;
  ImagePanel *in_stack_ffffffd0;
  Panel *in_stack_ffffffd4;
  char *in_stack_ffffffd8;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  CVGuiScreenPanel::CVGuiScreenPanel
            (&this->super_CVGuiScreenPanel,parent,(char *)(unaff_EBX + 0x39fe89));
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cac0a);
  this->m_nDay = 1;
  this->m_nCurrentLevel = 0;
  this->m_bInitializedLevelLabel = false;
  local_20 = 0;
  do {
    name = VarArgs((char *)(unaff_EBX + 0x39fe29),local_20);
    this_00 = operator_new(0x17c);
    vgui::ImagePanel::ImagePanel
              (this_00,(Panel *)this,name,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
    this->m_pLevelIconPanels[local_20] = this_00;
    local_20 = local_20 + 1;
  } while (local_20 != 0x10);
  return;
}


/* CVGUI_MP_ProgressSignScreen::CVGUI_MP_ProgressSignScreen at 0062bc00 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void CVGUI_MP_ProgressSignScreen(CVGUI_MP_ProgressSignScreen * this,
   Panel * parent, char * panelName, CVGUI_MP_ProgressSignScreen * this, Panel * parent, char *
   panelName) */

void __thiscall
CVGUI_MP_ProgressSignScreen::CVGUI_MP_ProgressSignScreen
          (CVGUI_MP_ProgressSignScreen *this,Panel *parent,char *panelName,
          CVGUI_MP_ProgressSignScreen *this_1,Panel *parent_1,char *panelName_1)

{
  CVGUI_MP_ProgressSignScreen(this,parent,panelName);
  return;
}


/* CPanelFactory<CVGUI_MP_ProgressSignScreen,VGuiScreenInitData_t>::Create at 0062c2d0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: Panel *
   Create(CPanelFactory<CVGUI_MP_ProgressSignScreen,VGuiScreenInitData_t> * this, char *
   pMetaClassName, KeyValues * pKeyValues, void * pVoidInitData, Panel * pParent) */

Panel * __thiscall
CPanelFactory<CVGUI_MP_ProgressSignScreen,VGuiScreenInitData_t>::Create
          (CPanelFactory<CVGUI_MP_ProgressSignScreen,VGuiScreenInitData_t> *this,
          char *pMetaClassName,KeyValues *pKeyValues,void *pVoidInitData,Panel *pParent)

{
  CVGUI_MP_ProgressSignScreen *this_00;
  int iVar1;
  CVGUI_MP_ProgressSignScreen *pCVar2;
  
                    /* Unresolved local var: CVGUI_MP_ProgressSignScreen * pPanel@[???] */
  this_00 = operator_new(0x1dc);
  CVGUI_MP_ProgressSignScreen::CVGUI_MP_ProgressSignScreen(this_00,pParent,pMetaClassName);
  pCVar2 = (CVGUI_MP_ProgressSignScreen *)0x0;
  if (this_00 != (CVGUI_MP_ProgressSignScreen *)0x0) {
    iVar1 = (*(this_00->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
              _vptr_IClientPanel[0xed])(this_00,pKeyValues,pVoidInitData);
    pCVar2 = this_00;
    if ((char)iVar1 == '\0') {
      (*(this_00->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
        _vptr_IClientPanel[0x21])(this_00);
      return (Panel *)0x0;
    }
  }
  return (Panel *)pCVar2;
}


/* CVGUI_MP_ProgressSignScreen::Paint at 0062a3f0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void Paint(CVGUI_MP_ProgressSignScreen * this) */

void __thiscall CVGUI_MP_ProgressSignScreen::Paint(CVGUI_MP_ProgressSignScreen *this)

{
  undefined4 *puVar1;
  int iVar2;
  int unaff_EBX;
  int iVar3;
  int iVar4;
  float fVar5;
  int nNumWidth;
  undefined4 local_34;
  undefined4 local_30;
  int local_24;
  int local_20 [4];
  
                    /* Unresolved local var: int w@[???]
                       Unresolved local var: int h@[???]
                       Unresolved local var: int nNumHeight@[???]
                       Unresolved local var: int nNumPerLine@[???]
                       Unresolved local var: float flNumWidthFrac@[???]
                       Unresolved local var: float flNumX@[???]
                       Unresolved local var: float flNumY@[???]
                       Unresolved local var: int nMod@[???] */
  ___i686_get_pc_thunk_bx();
  vgui::Panel::GetSize((Panel *)this,local_20,&local_24);
  puVar1 = *(undefined4 **)(&DAT_00539df6 + unaff_EBX);
  (**(code **)(*(int *)*puVar1 + 0x38))((int *)*puVar1,0xff8c8c8c);
  (**(code **)(*(int *)*puVar1 + 0x94))((int *)*puVar1,this->m_nBackgroundTextureID);
  (**(code **)(*(int *)*puVar1 + 0x9c))((int *)*puVar1,0,0,local_20[0],local_24);
  iVar3 = (int)((float)local_24 * *(float *)(unaff_EBX + 0x3b7f8a));
  (**(code **)(*(int *)**(undefined4 **)(&DAT_00539df6 + unaff_EBX) + 0x94))
            ((int *)**(undefined4 **)(&DAT_00539df6 + unaff_EBX),this->m_nNumberTextureID);
  iVar4 = iVar3;
  if (iVar3 < 0) {
    iVar4 = iVar3 + 3;
  }
  iVar4 = iVar4 >> 2;
  (**(code **)(*(int *)**(undefined4 **)(&DAT_00539df6 + unaff_EBX) + 0x19c))
            ((int *)**(undefined4 **)(&DAT_00539df6 + unaff_EBX),local_20[0] / 2 - iVar3 / 3,iVar4,
             local_20[0] / 2,iVar4 + iVar3,0,0,0x3e2aaaab,0x3f000000);
  if (this->m_nCurrentLevel < 5) {
    local_34 = 0;
    iVar2 = 0;
    local_30 = 0x3f000000;
  }
  else {
    local_34 = 0x3f000000;
    iVar2 = 6;
    local_30 = *(undefined4 *)(CDetailModel::ComputeAngles + unaff_EBX + 6);
  }
  fVar5 = (float)(this->m_nCurrentLevel - iVar2) * *(float *)(unaff_EBX + 0x3bc8fe);
  (**(code **)(*(int *)**(undefined4 **)(&DAT_00539df6 + unaff_EBX) + 0x94))
            ((int *)**(undefined4 **)(&DAT_00539df6 + unaff_EBX),this->m_nNumberTextureID);
  (**(code **)(*(int *)**(undefined4 **)(&DAT_00539df6 + unaff_EBX) + 0x19c))
            ((int *)**(undefined4 **)(&DAT_00539df6 + unaff_EBX),local_20[0] / 2,iVar4,
             iVar3 / 3 + local_20[0] / 2,iVar4 + iVar3,fVar5,local_34,
             *(float *)(unaff_EBX + 0x3bc8fe) + fVar5,local_30);
  vgui::Panel::Paint((Panel *)this);
  return;
}


/* CVGUI_MP_ProgressSignScreen::UpdateLevelLabel at 0062ad30 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void UpdateLevelLabel(CVGUI_MP_ProgressSignScreen * this) */

void __thiscall CVGUI_MP_ProgressSignScreen::UpdateLevelLabel(CVGUI_MP_ProgressSignScreen *this)

{
  int iVar1;
  int iVar2;
  int iVar3;
  Label *pLVar4;
  ImagePanel *panel;
  char cVar5;
  AnimationController *this_00;
  int unaff_EBX;
  int local_144;
  int local_140;
  int local_130;
  int local_12c;
  int local_128;
  int local_124;
  char local_120 [128];
  char local_a0 [128];
  int local_20;
  
                    /* Unresolved local var: int nDay@[???]
                       Unresolved local var: int nTotalLevels@[???]
                       Unresolved local var: int nLevel@[???]
                       Unresolved local var: char[128] szBranchNumber@[???]
                       Unresolved local var: char[128] szProgressNumbers@[???]
                       Unresolved local var: int w@[???]
                       Unresolved local var: int h@[???] */
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(&DAT_0053901f + unaff_EBX);
  cVar5 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x539077) + 0x88))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x539077));
  if (cVar5 != '\0') {
                    /* Unresolved local var: C_PortalMPGameRules * g_pPortalMPGameRules@[???] */
    cVar5 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5392c7) + 0x10c))
                      ((int *)**(undefined4 **)(unaff_EBX + 0x5392c7));
    if ((cVar5 != '\0') && (this->m_bInitializedLevelLabel != false)) {
                    /* Unresolved local var: C_PortalMPGameRules * g_pPortalMPGameRules@[???] */
      iVar1 = **(int **)(unaff_EBX + 0x5392c7);
      iVar2 = *(int *)(iVar1 + 0x48);
      if (0 < iVar2) {
        iVar3 = *(int *)(iVar1 + 0x186c + (iVar2 + -1) * 4);
        iVar1 = *(int *)(iVar1 + 0x4c + (iVar2 + -1) * 4);
        if (iVar1 == this->m_nCurrentLevel) {
          this->m_nDay = 1;
        }
        else {
          this->m_nDay = iVar2;
          this->m_nCurrentLevel = iVar1;
          V_snprintf(local_a0,0x80,(char *)(unaff_EBX + 0x3a0a52),iVar2);
          V_snprintf(local_120,0x80,(char *)(unaff_EBX + 0x3a0a67),this->m_nCurrentLevel,iVar3);
          pLVar4 = this->m_pBranchNumLabel;
          if ((pLVar4 != (Label *)0x0) && (this->m_bInitializedLevelLabel != false)) {
            (*(pLVar4->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd4])(pLVar4,local_a0);
          }
          pLVar4 = this->m_pLevelProgressNumLabel;
          if ((pLVar4 != (Label *)0x0) && (this->m_bInitializedLevelLabel != false)) {
            (*(pLVar4->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd4])(pLVar4,local_120);
          }
          vgui::Panel::GetSize((Panel *)this,&local_124,&local_128);
                    /* Unresolved local var: int i@[???] */
          if (0 < iVar3) {
            local_144 = 0;
            do {
              if (this->m_nCurrentLevel + -1 == local_144) {
                (*(this->m_pLevelIconPanels[local_144]->super_Panel).super_IClientPanel.
                  _vptr_IClientPanel[0xd5])
                          (this->m_pLevelIconPanels[local_144],unaff_EBX + 0x3a09f7);
                (*(this->m_pLevelIconPanels[local_144]->super_Panel).super_IClientPanel.
                  _vptr_IClientPanel[0xd8])
                          (this->m_pLevelIconPanels[local_144],
                           (uint)(byte)~-((&DAT_00001874)
                                          [this->m_nDay * 0x10 + **(int **)(unaff_EBX + 0x5392c7) +
                                           local_144] == '\0') |
                           CONCAT22((byte)-((&DAT_00001874)
                                            [this->m_nDay * 0x10 + **(int **)(unaff_EBX + 0x5392c7)
                                             + local_144] == '\0') | 0xff00,0xc800));
                vgui::Panel::SetSize(&this->m_pLevelIconPanels[local_144]->super_Panel,0x20,0x20);
                vgui::Panel::GetPos(&this->m_pLevelIconPanels[local_144]->super_Panel,&local_12c,
                                    &local_130);
                vgui::Panel::SetPos(&this->m_pLevelIconPanels[local_144]->super_Panel,local_12c,
                                    0x1cb);
                local_140 = local_124 / 2 + -0x10 +
                            (local_144 - (this->m_nCurrentLevel + -1)) * 0x20;
              }
              else {
                if ((&DAT_00001874)
                    [this->m_nDay * 0x10 + **(int **)(unaff_EBX + 0x5392c7) + local_144] == '\0') {
                  (*(this->m_pLevelIconPanels[local_144]->super_Panel).super_IClientPanel.
                    _vptr_IClientPanel[0xd8])(this->m_pLevelIconPanels[local_144],0xffffffff);
                  (*(this->m_pLevelIconPanels[local_144]->super_Panel).super_IClientPanel.
                    _vptr_IClientPanel[0xd5])
                            (this->m_pLevelIconPanels[local_144],unaff_EBX + 0x3a0a39);
                }
                else {
                  (*(this->m_pLevelIconPanels[local_144]->super_Panel).super_IClientPanel.
                    _vptr_IClientPanel[0xd8])(this->m_pLevelIconPanels[local_144],0xffffffff);
                  (*(this->m_pLevelIconPanels[local_144]->super_Panel).super_IClientPanel.
                    _vptr_IClientPanel[0xd5])
                            (this->m_pLevelIconPanels[local_144],unaff_EBX + 0x3a0a17);
                }
                vgui::Panel::SetSize(&this->m_pLevelIconPanels[local_144]->super_Panel,0x1a,0x1a);
                vgui::Panel::GetPos(&this->m_pLevelIconPanels[local_144]->super_Panel,&local_12c,
                                    &local_130);
                vgui::Panel::SetPos(&this->m_pLevelIconPanels[local_144]->super_Panel,local_12c,
                                    0x1ce);
                local_140 = local_124 / 2 + -0xd + (local_144 - (this->m_nCurrentLevel + -1)) * 0x20
                ;
              }
              panel = this->m_pLevelIconPanels[local_144];
              this_00 = vgui::GetAnimationController();
              vgui::AnimationController::RunAnimationCommand
                        (this_00,&panel->super_Panel,(char *)(unaff_EBX + 0x39ca53),(float)local_140
                         ,0.0,0.25,INTERPOLATOR_DEACCEL,0.0);
              (*(this->m_pLevelIconPanels[local_144]->super_Panel).super_IClientPanel.
                _vptr_IClientPanel[0x48])(this->m_pLevelIconPanels[local_144],1);
              (*(this->m_pLevelIconPanels[local_144]->super_Panel).super_IClientPanel.
                _vptr_IClientPanel[0x22])(this->m_pLevelIconPanels[local_144],1);
              local_144 = local_144 + 1;
            } while (local_144 != iVar3);
          }
        }
      }
    }
  }
  if (local_20 == **(int **)(&DAT_0053901f + unaff_EBX)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}


/* CVGUI_MP_ProgressSignScreen::ApplySchemeSettings at 0062a300 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ApplySchemeSettings(CVGUI_MP_ProgressSignScreen * this, IScheme *
   pScheme) */

void __thiscall
CVGUI_MP_ProgressSignScreen::ApplySchemeSettings(CVGUI_MP_ProgressSignScreen *this,IScheme *pScheme)

{
  Label *pLVar1;
  int unaff_EBX;
  int iVar2;
  
  ___i686_get_pc_thunk_bx();
  vgui::Panel::ApplySchemeSettings((Panel *)this,pScheme);
  pLVar1 = (Label *)vgui::Panel::FindChildByName((Panel *)this,(char *)(unaff_EBX + 0x3a13a2),false)
  ;
  this->m_pBranchNumLabel = pLVar1;
  pLVar1 = (Label *)vgui::Panel::FindChildByName((Panel *)this,(char *)(unaff_EBX + 0x3a13b1),false)
  ;
  this->m_pLevelProgressNumLabel = pLVar1;
  pLVar1 = this->m_pBranchNumLabel;
  if (pLVar1 != (Label *)0x0) {
    (*(pLVar1->super_Panel).super_IClientPanel._vptr_IClientPanel[0x3d])(pLVar1,0xff000000);
  }
  pLVar1 = this->m_pLevelProgressNumLabel;
  if (pLVar1 != (Label *)0x0) {
    (*(pLVar1->super_Panel).super_IClientPanel._vptr_IClientPanel[0x3d])(pLVar1,0xff000000);
  }
  iVar2 = 0;
  do {
                    /* Unresolved local var: int i@[???] */
    (*(this->m_pLevelIconPanels[iVar2]->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd8])
              (this->m_pLevelIconPanels[iVar2],0xffffffff);
    (*(this->m_pLevelIconPanels[iVar2]->super_Panel).super_IClientPanel._vptr_IClientPanel[0x22])
              (this->m_pLevelIconPanels[iVar2],0);
    iVar2 = iVar2 + 1;
  } while (iVar2 != 0x10);
  this->m_bInitializedLevelLabel = true;
  return;
}


/* CVGUI_MP_ProgressSignScreen::PerformLayout at 00629d30 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void PerformLayout(CVGUI_MP_ProgressSignScreen * this) */

void __thiscall CVGUI_MP_ProgressSignScreen::PerformLayout(CVGUI_MP_ProgressSignScreen *this)

{
  vgui::EditablePanel::PerformLayout((EditablePanel *)this);
                    /* Unresolved local var: int i@[???] */
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[0],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[1],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[2],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[3],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[4],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[5],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[6],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[7],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[8],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[9],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[10],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[0xb],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[0xc],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[0xd],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[0xe],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[0xf],true);
  return;
}


/* CVGUI_MP_ProgressSignScreen::Init at 0062a690 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: bool Init(CVGUI_MP_ProgressSignScreen * this, KeyValues * pKeyValues,
   VGuiScreenInitData_t * pInitData) */

bool __thiscall
CVGUI_MP_ProgressSignScreen::Init
          (CVGUI_MP_ProgressSignScreen *this,KeyValues *pKeyValues,VGuiScreenInitData_t *pInitData)

{
  undefined4 *puVar1;
  int *piVar2;
  code *pcVar3;
  bool bVar4;
  int iVar5;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  puVar1 = *(undefined4 **)(&DAT_00539b56 + unaff_EBX);
  iVar5 = (**(code **)(*(int *)*puVar1 + 0xa4))((int *)*puVar1,0);
  this->m_nBackgroundTextureID = iVar5;
  (**(code **)(*(int *)*puVar1 + 0x8c))((int *)*puVar1,iVar5,unaff_EBX + 0x3a1036,1,0);
  iVar5 = (**(code **)(*(int *)*puVar1 + 0xa4))((int *)*puVar1,0);
  this->m_nNumberTextureID = iVar5;
  (**(code **)(*(int *)*puVar1 + 0x8c))((int *)*puVar1,iVar5,unaff_EBX + 0x3a105e,1,0);
  piVar2 = (int *)**(undefined4 **)(&DAT_00539af2 + unaff_EBX);
  pcVar3 = *(code **)(*piVar2 + 0x54);
  iVar5 = (**(this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
             _vptr_IClientPanel)(this);
  (*pcVar3)(piVar2,iVar5,0);
  bVar4 = CVGuiScreenPanel::Init(&this->super_CVGuiScreenPanel,pKeyValues,pInitData);
  return bVar4;
}


/* CVGUI_MP_ProgressSignScreen::OnTick at 00629b50 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void OnTick(CVGUI_MP_ProgressSignScreen * this) */

void __thiscall CVGUI_MP_ProgressSignScreen::OnTick(CVGUI_MP_ProgressSignScreen *this)

{
  vgui::Panel::OnTick((Panel *)this);
  if (this->bIsAlreadyVisible == false) {
    (*(this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x22])(this,1);
    this->bIsAlreadyVisible = true;
  }
                    /* WARNING: Could not recover jumptable at 0x00629b96. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
    _vptr_IClientPanel[0xee])();
  return;
}


/* CVGUI_MP_LevelSelectScreen3::~CVGUI_MP_LevelSelectScreen3 at 0062bf00 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CVGUI_MP_LevelSelectScreen3(CVGUI_MP_LevelSelectScreen3 * this,
   int __in_chrg) */

void __thiscall
CVGUI_MP_LevelSelectScreen3::~CVGUI_MP_LevelSelectScreen3
          (CVGUI_MP_LevelSelectScreen3 *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(extraout_ECX + 0x5ca440);
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(*(int *)(extraout_ECX + 0x538558) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,__in_chrg);
  return;
}


/* CVGUI_MP_LevelSelectScreen::~CVGUI_MP_LevelSelectScreen at 0062bfb0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CVGUI_MP_LevelSelectScreen(CVGUI_MP_LevelSelectScreen * this, int
   __in_chrg) */

void __thiscall
CVGUI_MP_LevelSelectScreen::~CVGUI_MP_LevelSelectScreen
          (CVGUI_MP_LevelSelectScreen *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5ca387);
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(*(int *)(&DAT_0053849f + unaff_EBX) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CVGUI_MP_LevelSelectScreen::~CVGUI_MP_LevelSelectScreen at 0062c000 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CVGUI_MP_LevelSelectScreen(CVGUI_MP_LevelSelectScreen * this, int
   __in_chrg) */

void __thiscall
CVGUI_MP_LevelSelectScreen::~CVGUI_MP_LevelSelectScreen
          (CVGUI_MP_LevelSelectScreen *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(extraout_ECX + 0x5ca340);
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(*(int *)(&DAT_00538458 + extraout_ECX) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,__in_chrg);
  return;
}


/* CVGUI_MP_LevelSelectScreen4::~CVGUI_MP_LevelSelectScreen4 at 0062c030 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CVGUI_MP_LevelSelectScreen4(CVGUI_MP_LevelSelectScreen4 * this,
   int __in_chrg) */

void __thiscall
CVGUI_MP_LevelSelectScreen4::~CVGUI_MP_LevelSelectScreen4
          (CVGUI_MP_LevelSelectScreen4 *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(extraout_ECX + 0x5ca310);
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(*(int *)(&DAT_00538428 + extraout_ECX) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,__in_chrg);
  return;
}


/* CVGUI_MP_LevelSelectScreen2::~CVGUI_MP_LevelSelectScreen2 at 0062c5c0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CVGUI_MP_LevelSelectScreen2(CVGUI_MP_LevelSelectScreen2 * this,
   int __in_chrg) */

void __thiscall
CVGUI_MP_LevelSelectScreen2::~CVGUI_MP_LevelSelectScreen2
          (CVGUI_MP_LevelSelectScreen2 *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5c9d77);
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(*(int *)(&DAT_00537e8f + unaff_EBX) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CVGUI_MP_LevelSelectScreen2::~CVGUI_MP_LevelSelectScreen2 at 0062c610 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CVGUI_MP_LevelSelectScreen2(CVGUI_MP_LevelSelectScreen2 * this,
   int __in_chrg) */

void __thiscall
CVGUI_MP_LevelSelectScreen2::~CVGUI_MP_LevelSelectScreen2
          (CVGUI_MP_LevelSelectScreen2 *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(extraout_ECX + 0x5c9d30);
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(*(int *)(&DAT_00537e48 + extraout_ECX) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,__in_chrg);
  return;
}


/* CVGUI_MP_LevelSelectScreen3::~CVGUI_MP_LevelSelectScreen3 at 0062c640 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CVGUI_MP_LevelSelectScreen3(CVGUI_MP_LevelSelectScreen3 * this,
   int __in_chrg) */

void __thiscall
CVGUI_MP_LevelSelectScreen3::~CVGUI_MP_LevelSelectScreen3
          (CVGUI_MP_LevelSelectScreen3 *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5c9cf7);
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(*(int *)(&DAT_00537e0f + unaff_EBX) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CVGUI_MP_LevelSelectScreen4::~CVGUI_MP_LevelSelectScreen4 at 0062c690 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CVGUI_MP_LevelSelectScreen4(CVGUI_MP_LevelSelectScreen4 * this,
   int __in_chrg) */

void __thiscall
CVGUI_MP_LevelSelectScreen4::~CVGUI_MP_LevelSelectScreen4
          (CVGUI_MP_LevelSelectScreen4 *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5c9ca7);
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(*(int *)(&DAT_00537dbf + unaff_EBX) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CVGUI_MP_LevelSelectScreen::CVGUI_MP_LevelSelectScreen at 0062b830 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void CVGUI_MP_LevelSelectScreen(CVGUI_MP_LevelSelectScreen * this,
   Panel * parent, char * panelName) */

void __thiscall
CVGUI_MP_LevelSelectScreen::CVGUI_MP_LevelSelectScreen
          (CVGUI_MP_LevelSelectScreen *this,Panel *parent,char *panelName)

{
  char *name;
  ImagePanel *this_00;
  int unaff_EBX;
  ImagePanel *in_stack_ffffffd0;
  Panel *in_stack_ffffffd4;
  char *in_stack_ffffffd8;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  CVGuiScreenPanel::CVGuiScreenPanel
            (&this->super_CVGuiScreenPanel,parent,(char *)(unaff_EBX + 0x3a00ee));
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(&UNK_005cab0a + unaff_EBX);
  this->m_nDay = 1;
  this->m_nCurrentLevel = 0;
  this->m_bInitializedLevelLabel = false;
  local_20 = 0;
  do {
    name = VarArgs((char *)(unaff_EBX + 0x3a0109),local_20);
    this_00 = operator_new(0x17c);
    vgui::ImagePanel::ImagePanel
              (this_00,(Panel *)this,name,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
    this->m_pLevelIconPanels[local_20] = this_00;
    local_20 = local_20 + 1;
  } while (local_20 != 0x10);
  return;
}


/* CVGUI_MP_LevelSelectScreen::CVGUI_MP_LevelSelectScreen at 0062bb00 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void CVGUI_MP_LevelSelectScreen(CVGUI_MP_LevelSelectScreen * this,
   Panel * parent, char * panelName, CVGUI_MP_LevelSelectScreen * this, Panel * parent, char *
   panelName) */

void __thiscall
CVGUI_MP_LevelSelectScreen::CVGUI_MP_LevelSelectScreen
          (CVGUI_MP_LevelSelectScreen *this,Panel *parent,char *panelName,
          CVGUI_MP_LevelSelectScreen *this_1,Panel *parent_1,char *panelName_1)

{
  CVGUI_MP_LevelSelectScreen(this,parent,panelName);
  return;
}


/* CPanelFactory<CVGUI_MP_LevelSelectScreen,VGuiScreenInitData_t>::Create at 0062c060 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: Panel *
   Create(CPanelFactory<CVGUI_MP_LevelSelectScreen,VGuiScreenInitData_t> * this, char *
   pMetaClassName, KeyValues * pKeyValues, void * pVoidInitData, Panel * pParent) */

Panel * __thiscall
CPanelFactory<CVGUI_MP_LevelSelectScreen,VGuiScreenInitData_t>::Create
          (CPanelFactory<CVGUI_MP_LevelSelectScreen,VGuiScreenInitData_t> *this,char *pMetaClassName
          ,KeyValues *pKeyValues,void *pVoidInitData,Panel *pParent)

{
  CVGUI_MP_LevelSelectScreen *this_00;
  int iVar1;
  CVGUI_MP_LevelSelectScreen *pCVar2;
  
                    /* Unresolved local var: CVGUI_MP_LevelSelectScreen * pPanel@[???] */
  this_00 = operator_new(0x1dc);
  CVGUI_MP_LevelSelectScreen::CVGUI_MP_LevelSelectScreen(this_00,pParent,pMetaClassName);
  pCVar2 = (CVGUI_MP_LevelSelectScreen *)0x0;
  if (this_00 != (CVGUI_MP_LevelSelectScreen *)0x0) {
    iVar1 = (*(this_00->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
              _vptr_IClientPanel[0xed])(this_00,pKeyValues,pVoidInitData);
    pCVar2 = this_00;
    if ((char)iVar1 == '\0') {
      (*(this_00->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
        _vptr_IClientPanel[0x21])(this_00);
      return (Panel *)0x0;
    }
  }
  return (Panel *)pCVar2;
}


/* CVGUI_MP_LevelSelectScreen::UpdateLevelLabel at 0062a780 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void UpdateLevelLabel(CVGUI_MP_LevelSelectScreen * this) */

void __thiscall CVGUI_MP_LevelSelectScreen::UpdateLevelLabel(CVGUI_MP_LevelSelectScreen *this)

{
  int iVar1;
  Label *pLVar2;
  ImagePanel *panel;
  char cVar3;
  AnimationController *this_00;
  int iVar4;
  int unaff_EBX;
  int iVar5;
  int local_144;
  int local_140;
  int local_130;
  int local_12c;
  int local_128;
  int local_124;
  char local_120 [128];
  char local_a0 [128];
  int local_20;
  
                    /* Unresolved local var: int nThisDay@[???]
                       Unresolved local var: int nTotalLevels@[???]
                       Unresolved local var: int nLevel@[???]
                       Unresolved local var: char[128] szLevelNumber@[???]
                       Unresolved local var: char[128] szLevelName@[???]
                       Unresolved local var: int w@[???]
                       Unresolved local var: int h@[???] */
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(unaff_EBX + 0x5395cf);
  cVar3 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x539627) + 0x88))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x539627));
  if (cVar3 != '\0') {
                    /* Unresolved local var: C_PortalMPGameRules * g_pPortalMPGameRules@[???] */
    cVar3 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x539877) + 0x10c))
                      ((int *)**(undefined4 **)(unaff_EBX + 0x539877));
    if ((cVar3 != '\0') && (this->m_bInitializedLevelLabel != false)) {
      iVar5 = this->m_nDay + -1;
                    /* Unresolved local var: C_PortalMPGameRules * g_pPortalMPGameRules@[???] */
      iVar1 = *(int *)(**(int **)(unaff_EBX + 0x539877) + 0x186c + iVar5 * 4);
      iVar4 = *(int *)(**(int **)(unaff_EBX + 0x539877) + 0x4c + iVar5 * 4);
      if (iVar4 != this->m_nCurrentLevel) {
        this->m_nCurrentLevel = iVar4;
        V_snprintf(local_a0,0x80,(char *)(unaff_EBX + 0x3a0f99),iVar4);
        V_snprintf(local_120,0x80,(char *)(unaff_EBX + 0x3a0fa2),
                   (iVar5 * 0x10 + this->m_nCurrentLevel) * 0x40 + **(int **)(unaff_EBX + 0x539877)
                   + 0x29);
        pLVar2 = this->m_pLevelNumLabel;
        if ((pLVar2 != (Label *)0x0) && (this->m_bInitializedLevelLabel != false)) {
          (*(pLVar2->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd4])(pLVar2,local_a0);
        }
        pLVar2 = this->m_pLevelTextLabel;
        if ((pLVar2 != (Label *)0x0) && (this->m_bInitializedLevelLabel != false)) {
          (*(pLVar2->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd4])(pLVar2,local_120);
        }
        vgui::Panel::GetSize((Panel *)this,&local_124,&local_128);
                    /* Unresolved local var: int i@[???] */
        if (0 < iVar1) {
          local_144 = 0;
          do {
            if (this->m_nCurrentLevel + -1 == local_144) {
              (*(this->m_pLevelIconPanels[local_144]->super_Panel).super_IClientPanel.
                _vptr_IClientPanel[0xd5])(this->m_pLevelIconPanels[local_144],unaff_EBX + 0x3a0fa7);
              (*(this->m_pLevelIconPanels[local_144]->super_Panel).super_IClientPanel.
                _vptr_IClientPanel[0xd8])
                        (this->m_pLevelIconPanels[local_144],
                         (uint)(byte)~-((&DAT_00001874)
                                        [this->m_nDay * 0x10 + **(int **)(unaff_EBX + 0x539877) +
                                         local_144] == '\0') |
                         CONCAT22((byte)-((&DAT_00001874)
                                          [this->m_nDay * 0x10 + **(int **)(unaff_EBX + 0x539877) +
                                           local_144] == '\0') | 0xff00,0xc800));
              vgui::Panel::SetSize(&this->m_pLevelIconPanels[local_144]->super_Panel,0x96,0x96);
              vgui::Panel::GetPos(&this->m_pLevelIconPanels[local_144]->super_Panel,&local_12c,
                                  &local_130);
              vgui::Panel::SetPos(&this->m_pLevelIconPanels[local_144]->super_Panel,local_12c,0x26c)
              ;
              local_140 = local_124 / 2 + -0x4b + (local_144 - (this->m_nCurrentLevel + -1)) * 0x96;
            }
            else {
              if ((&DAT_00001874)
                  [this->m_nDay * 0x10 + **(int **)(unaff_EBX + 0x539877) + local_144] == '\0') {
                (*(this->m_pLevelIconPanels[local_144]->super_Panel).super_IClientPanel.
                  _vptr_IClientPanel[0xd8])(this->m_pLevelIconPanels[local_144],0xffffffff);
                (*(this->m_pLevelIconPanels[local_144]->super_Panel).super_IClientPanel.
                  _vptr_IClientPanel[0xd5])
                          (this->m_pLevelIconPanels[local_144],unaff_EBX + 0x3a0fe9);
              }
              else {
                (*(this->m_pLevelIconPanels[local_144]->super_Panel).super_IClientPanel.
                  _vptr_IClientPanel[0xd8])(this->m_pLevelIconPanels[local_144],0xffffffff);
                (*(this->m_pLevelIconPanels[local_144]->super_Panel).super_IClientPanel.
                  _vptr_IClientPanel[0xd5])
                          (this->m_pLevelIconPanels[local_144],unaff_EBX + 0x3a0fc7);
              }
              vgui::Panel::SetSize(&this->m_pLevelIconPanels[local_144]->super_Panel,0x78,0x78);
              vgui::Panel::GetPos(&this->m_pLevelIconPanels[local_144]->super_Panel,&local_12c,
                                  &local_130);
              vgui::Panel::SetPos(&this->m_pLevelIconPanels[local_144]->super_Panel,local_12c,0x27b)
              ;
              local_140 = local_124 / 2 + -0x3c + (local_144 - (this->m_nCurrentLevel + -1)) * 0x96;
            }
            panel = this->m_pLevelIconPanels[local_144];
            this_00 = vgui::GetAnimationController();
            vgui::AnimationController::RunAnimationCommand
                      (this_00,&panel->super_Panel,(char *)(unaff_EBX + 0x39d003),(float)local_140,
                       0.0,0.25,INTERPOLATOR_DEACCEL,0.0);
            (*(this->m_pLevelIconPanels[local_144]->super_Panel).super_IClientPanel.
              _vptr_IClientPanel[0x48])(this->m_pLevelIconPanels[local_144],1);
            iVar4 = vgui::Panel::GetZPos(&this->m_pBranchTitleLabel->super_Panel);
            vgui::Panel::SetZPos(&this->m_pLevelIconPanels[local_144]->super_Panel,iVar4);
            (*(this->m_pLevelIconPanels[local_144]->super_Panel).super_IClientPanel.
              _vptr_IClientPanel[0x22])(this->m_pLevelIconPanels[local_144],1);
            local_144 = local_144 + 1;
          } while (local_144 != iVar1);
        }
      }
    }
  }
  if (local_20 == **(int **)(unaff_EBX + 0x5395cf)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}


/* CVGUI_MP_LevelSelectScreen::ApplySchemeSettings at 0062a190 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ApplySchemeSettings(CVGUI_MP_LevelSelectScreen * this, IScheme *
   pScheme) */

void __thiscall
CVGUI_MP_LevelSelectScreen::ApplySchemeSettings(CVGUI_MP_LevelSelectScreen *this,IScheme *pScheme)

{
  Label *pLVar1;
  int unaff_EBX;
  int iVar2;
  
  ___i686_get_pc_thunk_bx();
  vgui::Panel::ApplySchemeSettings((Panel *)this,pScheme);
  pLVar1 = (Label *)vgui::Panel::FindChildByName((Panel *)this,(char *)(unaff_EBX + 0x3a14d5),false)
  ;
  this->m_pBranchTitleLabel = pLVar1;
  pLVar1 = (Label *)vgui::Panel::FindChildByName((Panel *)this,(char *)(unaff_EBX + 0x3a14e1),false)
  ;
  this->m_pSelectLevelLabel = pLVar1;
  pLVar1 = (Label *)vgui::Panel::FindChildByName((Panel *)this,(char *)(unaff_EBX + 0x3a14f2),false)
  ;
  this->m_pLevelNumLabel = pLVar1;
  pLVar1 = (Label *)vgui::Panel::FindChildByName((Panel *)this,(char *)(unaff_EBX + 0x3a1503),false)
  ;
  this->m_pLevelTextLabel = pLVar1;
  pLVar1 = this->m_pBranchTitleLabel;
  if (pLVar1 != (Label *)0x0) {
    (*(pLVar1->super_Panel).super_IClientPanel._vptr_IClientPanel[0x3d])(pLVar1,0xff000000);
  }
  pLVar1 = this->m_pSelectLevelLabel;
  if (pLVar1 != (Label *)0x0) {
    (*(pLVar1->super_Panel).super_IClientPanel._vptr_IClientPanel[0x3d])(pLVar1,0xff000000);
  }
  pLVar1 = this->m_pLevelNumLabel;
  if (pLVar1 != (Label *)0x0) {
    (*(pLVar1->super_Panel).super_IClientPanel._vptr_IClientPanel[0x3d])(pLVar1,0xff000000);
  }
  pLVar1 = this->m_pLevelTextLabel;
  if (pLVar1 != (Label *)0x0) {
    (*(pLVar1->super_Panel).super_IClientPanel._vptr_IClientPanel[0x3d])(pLVar1,0xff000000);
  }
  iVar2 = 0;
  do {
                    /* Unresolved local var: int i@[???] */
    (*(this->m_pLevelIconPanels[iVar2]->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd8])
              (this->m_pLevelIconPanels[iVar2],0xffffffff);
    (*(this->m_pLevelIconPanels[iVar2]->super_Panel).super_IClientPanel._vptr_IClientPanel[0x22])
              (this->m_pLevelIconPanels[iVar2],0);
    iVar2 = iVar2 + 1;
  } while (iVar2 != 0x10);
  this->m_bInitializedLevelLabel = true;
  return;
}


/* CVGUI_MP_LevelSelectScreen::PerformLayout at 00629ba0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void PerformLayout(CVGUI_MP_LevelSelectScreen * this) */

void __thiscall CVGUI_MP_LevelSelectScreen::PerformLayout(CVGUI_MP_LevelSelectScreen *this)

{
  vgui::EditablePanel::PerformLayout((EditablePanel *)this);
                    /* Unresolved local var: int i@[???] */
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[0],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[1],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[2],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[3],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[4],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[5],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[6],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[7],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[8],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[9],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[10],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[0xb],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[0xc],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[0xd],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[0xe],true);
  vgui::ImagePanel::SetShouldScaleImage(this->m_pLevelIconPanels[0xf],true);
  return;
}


/* CVGUI_MP_LevelSelectScreen::Init at 0062a620 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: bool Init(CVGUI_MP_LevelSelectScreen * this, KeyValues * pKeyValues,
   VGuiScreenInitData_t * pInitData) */

bool __thiscall
CVGUI_MP_LevelSelectScreen::Init
          (CVGUI_MP_LevelSelectScreen *this,KeyValues *pKeyValues,VGuiScreenInitData_t *pInitData)

{
  int *piVar1;
  code *pcVar2;
  bool bVar3;
  int iVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  piVar1 = (int *)**(undefined4 **)(&DAT_00539b5c + unaff_EBX);
  pcVar2 = *(code **)(*piVar1 + 0x54);
  iVar4 = (**(this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
             _vptr_IClientPanel)(this);
  (*pcVar2)(piVar1,iVar4,0);
  bVar3 = CVGuiScreenPanel::Init(&this->super_CVGuiScreenPanel,pKeyValues,pInitData);
  return bVar3;
}


/* CVGUI_MP_LevelSelectScreen::OnTick at 00629b00 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void OnTick(CVGUI_MP_LevelSelectScreen * this) */

void __thiscall CVGUI_MP_LevelSelectScreen::OnTick(CVGUI_MP_LevelSelectScreen *this)

{
  vgui::Panel::OnTick((Panel *)this);
  if (this->bIsAlreadyVisible == false) {
    (*(this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x22])(this,1);
    this->bIsAlreadyVisible = true;
  }
                    /* WARNING: Could not recover jumptable at 0x00629b46. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
    _vptr_IClientPanel[0xee])();
  return;
}


/* CVGUI_MP_LevelSelectScreen2::CVGUI_MP_LevelSelectScreen2 at 0062ba60 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void CVGUI_MP_LevelSelectScreen2(CVGUI_MP_LevelSelectScreen2 * this,
   Panel * parent, char * panelName) */

void __thiscall
CVGUI_MP_LevelSelectScreen2::CVGUI_MP_LevelSelectScreen2
          (CVGUI_MP_LevelSelectScreen2 *this,Panel *parent,char *panelName)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CVGUI_MP_LevelSelectScreen::CVGUI_MP_LevelSelectScreen
            (&this->super_CVGUI_MP_LevelSelectScreen,parent,(char *)(unaff_EBX + 0x39ff1a));
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cc0f7);
  (this->super_CVGUI_MP_LevelSelectScreen).m_nDay = 2;
  return;
}


/* CVGUI_MP_LevelSelectScreen2::CVGUI_MP_LevelSelectScreen2 at 0062bab0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void CVGUI_MP_LevelSelectScreen2(CVGUI_MP_LevelSelectScreen2 * this,
   Panel * parent, char * panelName) */

void __thiscall
CVGUI_MP_LevelSelectScreen2::CVGUI_MP_LevelSelectScreen2
          (CVGUI_MP_LevelSelectScreen2 *this,Panel *parent,char *panelName)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CVGUI_MP_LevelSelectScreen::CVGUI_MP_LevelSelectScreen
            (&this->super_CVGUI_MP_LevelSelectScreen,parent,(char *)(unaff_EBX + 0x39feca));
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cc0a7);
  (this->super_CVGUI_MP_LevelSelectScreen).m_nDay = 2;
  return;
}


/* CPanelFactory<CVGUI_MP_LevelSelectScreen2,VGuiScreenInitData_t>::Create at 0062c190 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: Panel *
   Create(CPanelFactory<CVGUI_MP_LevelSelectScreen2,VGuiScreenInitData_t> * this, char *
   pMetaClassName, KeyValues * pKeyValues, void * pVoidInitData, Panel * pParent) */

Panel * __thiscall
CPanelFactory<CVGUI_MP_LevelSelectScreen2,VGuiScreenInitData_t>::Create
          (CPanelFactory<CVGUI_MP_LevelSelectScreen2,VGuiScreenInitData_t> *this,
          char *pMetaClassName,KeyValues *pKeyValues,void *pVoidInitData,Panel *pParent)

{
  char cVar1;
  CVGUI_MP_LevelSelectScreen *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CVGUI_MP_LevelSelectScreen2 * pPanel@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = operator_new(0x1dc);
  CVGUI_MP_LevelSelectScreen::CVGUI_MP_LevelSelectScreen
            (this_00,pParent,(char *)(unaff_EBX + 0x39f7e7));
  (this_00->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cb9c4);
  this_00->m_nDay = 2;
                    /* Unresolved local var: VGuiScreenInitData_t * pInitData@[???] */
  cVar1 = (**(code **)(unaff_EBX + 0x5cbd78))(this_00,pKeyValues,pVoidInitData);
  if (cVar1 == '\0') {
    (*(this_00->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(this_00);
    this_00 = (CVGUI_MP_LevelSelectScreen *)0x0;
  }
  return (Panel *)this_00;
}


/* CVGUI_MP_LevelSelectScreen3::CVGUI_MP_LevelSelectScreen3 at 0062b9c0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void CVGUI_MP_LevelSelectScreen3(CVGUI_MP_LevelSelectScreen3 * this,
   Panel * parent, char * panelName) */

void __thiscall
CVGUI_MP_LevelSelectScreen3::CVGUI_MP_LevelSelectScreen3
          (CVGUI_MP_LevelSelectScreen3 *this,Panel *parent,char *panelName)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CVGUI_MP_LevelSelectScreen::CVGUI_MP_LevelSelectScreen
            (&this->super_CVGUI_MP_LevelSelectScreen,parent,(char *)(unaff_EBX + 0x39ff9e));
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cbdb7);
  (this->super_CVGUI_MP_LevelSelectScreen).m_nDay = 3;
  return;
}


/* CVGUI_MP_LevelSelectScreen3::CVGUI_MP_LevelSelectScreen3 at 0062ba10 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void CVGUI_MP_LevelSelectScreen3(CVGUI_MP_LevelSelectScreen3 * this,
   Panel * parent, char * panelName) */

void __thiscall
CVGUI_MP_LevelSelectScreen3::CVGUI_MP_LevelSelectScreen3
          (CVGUI_MP_LevelSelectScreen3 *this,Panel *parent,char *panelName)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CVGUI_MP_LevelSelectScreen::CVGUI_MP_LevelSelectScreen
            (&this->super_CVGUI_MP_LevelSelectScreen,parent,(char *)(unaff_EBX + 0x39ff4e));
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cbd67);
  (this->super_CVGUI_MP_LevelSelectScreen).m_nDay = 3;
  return;
}


/* CPanelFactory<CVGUI_MP_LevelSelectScreen3,VGuiScreenInitData_t>::Create at 0062c230 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: Panel *
   Create(CPanelFactory<CVGUI_MP_LevelSelectScreen3,VGuiScreenInitData_t> * this, char *
   pMetaClassName, KeyValues * pKeyValues, void * pVoidInitData, Panel * pParent) */

Panel * __thiscall
CPanelFactory<CVGUI_MP_LevelSelectScreen3,VGuiScreenInitData_t>::Create
          (CPanelFactory<CVGUI_MP_LevelSelectScreen3,VGuiScreenInitData_t> *this,
          char *pMetaClassName,KeyValues *pKeyValues,void *pVoidInitData,Panel *pParent)

{
  char cVar1;
  CVGUI_MP_LevelSelectScreen *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CVGUI_MP_LevelSelectScreen3 * pPanel@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = operator_new(0x1dc);
  CVGUI_MP_LevelSelectScreen::CVGUI_MP_LevelSelectScreen
            (this_00,pParent,(char *)(unaff_EBX + 0x39f72b));
  (this_00->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cb544);
  this_00->m_nDay = 3;
                    /* Unresolved local var: VGuiScreenInitData_t * pInitData@[???] */
  cVar1 = (**(code **)(unaff_EBX + 0x5cb8f8))(this_00,pKeyValues,pVoidInitData);
  if (cVar1 == '\0') {
    (*(this_00->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(this_00);
    this_00 = (CVGUI_MP_LevelSelectScreen *)0x0;
  }
  return (Panel *)this_00;
}


/* CVGUI_MP_LevelSelectScreen4::CVGUI_MP_LevelSelectScreen4 at 0062b920 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void CVGUI_MP_LevelSelectScreen4(CVGUI_MP_LevelSelectScreen4 * this,
   Panel * parent, char * panelName) */

void __thiscall
CVGUI_MP_LevelSelectScreen4::CVGUI_MP_LevelSelectScreen4
          (CVGUI_MP_LevelSelectScreen4 *this,Panel *parent,char *panelName)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CVGUI_MP_LevelSelectScreen::CVGUI_MP_LevelSelectScreen
            (&this->super_CVGUI_MP_LevelSelectScreen,parent,(char *)(unaff_EBX + 0x3a0022));
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cba77);
  (this->super_CVGUI_MP_LevelSelectScreen).m_nDay = 4;
  return;
}


/* CVGUI_MP_LevelSelectScreen4::CVGUI_MP_LevelSelectScreen4 at 0062b970 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void CVGUI_MP_LevelSelectScreen4(CVGUI_MP_LevelSelectScreen4 * this,
   Panel * parent, char * panelName) */

void __thiscall
CVGUI_MP_LevelSelectScreen4::CVGUI_MP_LevelSelectScreen4
          (CVGUI_MP_LevelSelectScreen4 *this,Panel *parent,char *panelName)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CVGUI_MP_LevelSelectScreen::CVGUI_MP_LevelSelectScreen
            (&this->super_CVGUI_MP_LevelSelectScreen,parent,(char *)(unaff_EBX + 0x39ffd2));
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(&UNK_005cba27 + unaff_EBX);
  (this->super_CVGUI_MP_LevelSelectScreen).m_nDay = 4;
  return;
}


/* CPanelFactory<CVGUI_MP_LevelSelectScreen4,VGuiScreenInitData_t>::Create at 0062c0f0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: Panel *
   Create(CPanelFactory<CVGUI_MP_LevelSelectScreen4,VGuiScreenInitData_t> * this, char *
   pMetaClassName, KeyValues * pKeyValues, void * pVoidInitData, Panel * pParent) */

Panel * __thiscall
CPanelFactory<CVGUI_MP_LevelSelectScreen4,VGuiScreenInitData_t>::Create
          (CPanelFactory<CVGUI_MP_LevelSelectScreen4,VGuiScreenInitData_t> *this,
          char *pMetaClassName,KeyValues *pKeyValues,void *pVoidInitData,Panel *pParent)

{
  char cVar1;
  CVGUI_MP_LevelSelectScreen *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CVGUI_MP_LevelSelectScreen4 * pPanel@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = operator_new(0x1dc);
  CVGUI_MP_LevelSelectScreen::CVGUI_MP_LevelSelectScreen
            (this_00,pParent,(char *)(unaff_EBX + 0x39f84f));
  (this_00->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cb2a4);
  this_00->m_nDay = 4;
                    /* Unresolved local var: VGuiScreenInitData_t * pInitData@[???] */
  cVar1 = (**(code **)(unaff_EBX + 0x5cb658))(this_00,pKeyValues,pVoidInitData);
  if (cVar1 == '\0') {
    (*(this_00->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(this_00);
    this_00 = (CVGUI_MP_LevelSelectScreen *)0x0;
  }
  return (Panel *)this_00;
}


/* __static_initialization_and_destruction_0 at 00081860 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  bool *pbVar1;
  IPanelMetaClassMgr *pIVar2;
  int unaff_EBX;
  longlong lVar3;
  
  lVar3 = ___i686_get_pc_thunk_bx();
  if (lVar3 != 0xffff00000001) {
    return;
  }
  (&DAT_00c57e68)[unaff_EBX] = 0;
  (&DAT_00c57e69)[unaff_EBX] = 0;
  (&DAT_00c57e6a)[unaff_EBX] = 0;
  (&DAT_00c57e6b)[unaff_EBX] = 0;
  *(undefined4 *)(&DAT_00c57e6c + unaff_EBX) = 0;
  *(undefined4 *)((int)&CFmtStrN<256>::sprintf::scAsserted + unaff_EBX) = 0;
  *(undefined4 *)((int)&PS_SD_Static_World_StaticProps_ClippedProp_t::pTraceEntity + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&C_WeaponPortalgun::GetBulletSpread()::cone + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c57e7c + unaff_EBX) = 0;
  *(undefined4 *)((int)&vgui::Frame::GetKBMap()::s_pMap + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c57e84 + unaff_EBX) = 0;
  *(undefined4 *)((int)&g_pPortalMPGameRules + unaff_EBX) = 0;
  pbVar1 = &g_bAllowForcePortalTrace + unaff_EBX;
  pbVar1[0] = true;
  pbVar1[1] = true;
  pbVar1[2] = true;
  pbVar1[3] = true;
  *(undefined4 *)((int)&C_Prop_Portal::s_DefaultPortalHalfWidth + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c57e94 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&CUtlRBTree<CUtlMap<vcollide_t*,CStaticCollisionPolyhedronCache::StaticPropPolyhedronCacheInfo_t,unsigned_short>::Node_t,unsigned_short,CUtlMap<vcollide_t*,CStaticCollisionPolyhedronCache::StaticPropPolyhedronCacheInfo_t,unsigned_short>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<vcollide_t*,CStaticCollisionPolyhedronCache::StaticPropPolyhedronCacheInfo_t,unsigned_short>::Node_t,unsigned_short>,unsigned_short>>
          ::Links(unsigned_short)::s_Sentinel + unaff_EBX) = 0x7f7fffff;
  *(undefined **)(&DAT_00c57e9c + unaff_EBX) = &UNK_00ae361c + unaff_EBX;
  *(int *)((int)ice_sbox[3] + unaff_EBX + 0x954) =
       (int)&PTR_PhysicsSolidMaskForEntity_00b75a7c + unaff_EBX;
  pIVar2 = PanelMetaClassMgr();
  (*pIVar2->_vptr_IPanelMetaClassMgr[1])(pIVar2,unaff_EBX + 0x949f42,unaff_EBX + 0xce9574);
  *(int *)((int)ice_sbox[3] + unaff_EBX + 0x958) =
       (int)&PTR_MyCombatWeaponPointer_00b75aac + unaff_EBX;
  pIVar2 = PanelMetaClassMgr();
  (*pIVar2->_vptr_IPanelMetaClassMgr[1])(pIVar2,unaff_EBX + 0x949f57,unaff_EBX + 0xce9578);
  *(int *)((int)ice_sbox[3] + unaff_EBX + 0x95c) = (int)&PTR_EarPosition_00b75ac4 + unaff_EBX;
  pIVar2 = PanelMetaClassMgr();
  (*pIVar2->_vptr_IPanelMetaClassMgr[1])(pIVar2,unaff_EBX + 0x949f67,unaff_EBX + 0xce957c);
  *(int *)((int)ice_sbox[3] + unaff_EBX + 0x960) = (int)&PTR_GetSkin_00b75adc + unaff_EBX;
  pIVar2 = PanelMetaClassMgr();
  (*pIVar2->_vptr_IPanelMetaClassMgr[1])(pIVar2,&UNK_00949f78 + unaff_EBX,unaff_EBX + 0xce9580);
  *(int *)((int)ice_sbox[3] + unaff_EBX + 0x964) =
       (int)&PTR_BoneMergeFastCullBloat_00b75af4 + unaff_EBX;
  pIVar2 = PanelMetaClassMgr();
  (*pIVar2->_vptr_IPanelMetaClassMgr[1])(pIVar2,unaff_EBX + 0x949f92,unaff_EBX + 0xce9584);
  *(int *)((int)ice_sbox[3] + unaff_EBX + 0x968) =
       (int)&PTR_AddRagdollToFadeQueue_00b75b0c + unaff_EBX;
  pIVar2 = PanelMetaClassMgr();
  (*pIVar2->_vptr_IPanelMetaClassMgr[1])(pIVar2,unaff_EBX + 0x949fac,unaff_EBX + 0xce9588);
  *(int *)((int)ice_sbox[3] + unaff_EBX + 0x96c) = (int)&PTR_GetPoseParameters_00b75b24 + unaff_EBX;
  pIVar2 = PanelMetaClassMgr();
  (*pIVar2->_vptr_IPanelMetaClassMgr[1])(pIVar2,unaff_EBX + 0x949fc6,unaff_EBX + 0xce958c);
  return;
}


/* _GLOBAL__I_g_CObjectControlPanelFactory at 00081a60 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_g_CObjectControlPanelFactory(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

