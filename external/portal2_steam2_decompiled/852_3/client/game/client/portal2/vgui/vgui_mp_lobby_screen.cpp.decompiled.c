/* DWARF-guided pseudocode for game/client/portal2/vgui/vgui_mp_lobby_screen.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* CObjectControlPanel::~CObjectControlPanel at 0061f540 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CObjectControlPanel(CObjectControlPanel * this, int __in_chrg) */

void __thiscall CObjectControlPanel::~CObjectControlPanel(CObjectControlPanel *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a1d97);
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x511ebf) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CObjectControlPanel::~CObjectControlPanel at 0061f590 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CObjectControlPanel(CObjectControlPanel * this, int __in_chrg) */

void __thiscall CObjectControlPanel::~CObjectControlPanel(CObjectControlPanel *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(extraout_ECX + 0x5a1d50);
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(*(int *)(extraout_ECX + 0x511e78) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,__in_chrg);
  return;
}


/* CVGUI_MP_LobbyScreen::~CVGUI_MP_LobbyScreen at 0061f5c0 */

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
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(extraout_ECX + 0x5a1d20);
  (this->super_CObjectControlPanel).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(*(int *)(extraout_ECX + 0x511e48) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,__in_chrg);
  return;
}


/* CVGUI_MP_LobbyScreen::~CVGUI_MP_LobbyScreen at 0061f750 */

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
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a1b87);
  (this->super_CObjectControlPanel).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x511caf) + 8)
  ;
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CObjectControlPanel::TickCurrentPanel at 0061e710 */

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
  bOldState = (bool)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51263a) + 0x204))
                              ((int *)**(undefined4 **)(unaff_EBX + 0x51263a));
  CHackForGetLocalPlayerAccessAllowedGuard::CHackForGetLocalPlayerAccessAllowedGuard
            (local_28,(char *)(unaff_EBX + 0x381f02),bOldState);
  pCVar4 = C_BasePlayer::GetLocalPlayer(-1);
  uVar1 = (this->super_CVGuiScreenPanel).m_hEntity.super_CBaseHandle.m_Index;
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???]
                       Unresolved local var: CEntInfo * pInfo@[???] */
  if ((((uVar1 == 0xffffffff) ||
       (iVar5 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x512672),
       *(uint *)(iVar5 + 8) != uVar1 >> 0x10)) || (iVar5 = *(int *)(iVar5 + 4), iVar5 == 0)) ||
     (((uVar1 = *(uint *)(iVar5 + 0x128), uVar1 == 0xffffffff ||
       (iVar5 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x512672),
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


/* CCommandChainingPanel::~CCommandChainingPanel at 0061f100 */

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
       (_func_int_varargs **)(unaff_EBX + 0x5a3237);
  vgui::EditablePanel::~EditablePanel(&this->super_EditablePanel,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CCommandChainingPanel::~CCommandChainingPanel at 0061f140 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CCommandChainingPanel(CCommandChainingPanel * this, int
   __in_chrg) */

void __thiscall
CCommandChainingPanel::~CCommandChainingPanel(CCommandChainingPanel *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(extraout_ECX + 0x5a3200);
  vgui::EditablePanel::~EditablePanel(&this->super_EditablePanel,__in_chrg);
  return;
}


/* CObjectControlPanel::CObjectControlPanel at 0061eee0 */

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
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a23fa);
  vgui::Panel::SetKeyBoardInputEnabled((Panel *)this,false);
  vgui::Panel::SetMouseInputEnabled((Panel *)this,true);
  this_00 = operator_new(0x17c);
  vgui::EditablePanel::EditablePanel(this_00,(Panel *)this,(char *)(unaff_EBX + 0x381865));
  (this_00->super_Panel).super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5a345a);
  vgui::Panel::SetPaintBackgroundEnabled(&this_00->super_Panel,false);
  this->m_pActivePanel = this_00;
  vgui::Panel::SetCursor((Panel *)this,2);
  vgui::Panel::SetZPos(&this->m_pActivePanel->super_Panel,-1);
  return;
}


/* CCommandChainingPanel::OnCommand at 0061f160 */

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
                    /* WARNING: Could not recover jumptable at 0x0061f1af. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (**(code **)(*piVar2 + 0x184))();
    return;
  }
  return;
}


/* CObjectControlPanel::CObjectControlPanel at 0061f0f0 */

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


/* CPanelFactory<CObjectControlPanel,VGuiScreenInitData_t>::Create at 0061f5f0 */

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


/* CObjectControlPanel::Init at 0061dad0 */

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
  piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x513662);
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


/* CObjectControlPanel::GetOwningObject at 0061d850 */

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
       (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x513537),
       *(uint *)(iVar3 + 8) != uVar1 >> 0x10)) || (iVar3 = *(int *)(iVar3 + 4), iVar3 == 0)) ||
     (((uVar1 = *(uint *)(iVar3 + 0x128), uVar1 == 0xffffffff ||
       (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x513537),
       *(uint *)(iVar3 + 8) != uVar1 >> 0x10)) ||
      (pCVar2 = *(C_BaseEntity **)(iVar3 + 4), pCVar2 == (C_BaseEntity *)0x0)))) {
    pCVar2 = (C_BaseEntity *)0x0;
  }
  return pCVar2;
}


/* CObjectControlPanel::OnTick at 0061e830 */

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
       (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x512553),
       *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) && (iVar2 = *(int *)(iVar2 + 4), iVar2 != 0)) &&
     (((uVar1 = *(uint *)(iVar2 + 0x128), uVar1 != 0xffffffff &&
       (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x512553),
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


/* CObjectControlPanel::SendToServerObject at 0061e940 */

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
       (iVar5 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x512442),
       *(uint *)(iVar5 + 8) == uVar1 >> 0x10)) && (iVar5 = *(int *)(iVar5 + 4), iVar5 != 0)) &&
     (((uVar1 = *(uint *)(iVar5 + 0x128), uVar1 != 0xffffffff &&
       (iVar5 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x512442),
       *(uint *)(iVar5 + 8) == uVar1 >> 0x10)) && (*(int *)(iVar5 + 4) != 0)))) {
    piVar2 = (int *)**(undefined4 **)(unaff_EBX + 0x512882);
    pcVar3 = *(code **)(*piVar2 + 0xf8);
    iVar5 = (**(this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
               _vptr_IClientPanel)(this);
    iVar5 = (*pcVar3)(piVar2,iVar5);
    puVar4 = *(undefined4 **)(unaff_EBX + 0x51240a);
    piVar2 = (int *)*puVar4;
    nOldSlot = (**(code **)(*piVar2 + 0x1f8))(piVar2);
    this_00 = &local_3c;
    CSetActiveSplitScreenPlayerGuard::CSetActiveSplitScreenPlayerGuard
              (this_00,(char *)(unaff_EBX + 0x381cfa),0xb4,iVar5,nOldSlot,false,in_stack_ffffffac,
               in_stack_ffffffb0,in_stack_ffffffb4,in_stack_ffffffb8,in_stack_ffffffbc,
               SUB41(this_00,0));
    piVar2 = (int *)*puVar4;
    (**(code **)(*piVar2 + 0x18))(piVar2,pMsg,1);
    CSetActiveSplitScreenPlayerGuard::~CSetActiveSplitScreenPlayerGuard(this_00,(int)pMsg);
    return;
  }
  return;
}


/* CObjectControlPanel::OnTickActive at 0061d8c0 */

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


/* CObjectControlPanel::OnCommand at 0061d8e0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void OnCommand(CObjectControlPanel * this, char * command) */

void __thiscall CObjectControlPanel::OnCommand(CObjectControlPanel *this,char *command)

{
  CVGuiScreenPanel::OnCommand(&this->super_CVGuiScreenPanel,command);
  return;
}


/* CVGUI_MP_LobbyScreen::OnCommand at 0061ead0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void OnCommand(CVGUI_MP_LobbyScreen * this, char * pCommand) */

void __thiscall CVGUI_MP_LobbyScreen::OnCommand(CVGUI_MP_LobbyScreen *this,char *pCommand)

{
  int iVar1;
  int unaff_EBX;
  char local_40 [32];
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(unaff_EBX + 0x512230);
  iVar1 = _V_stricmp(pCommand,(char *)(unaff_EBX + 0x381be4));
  if (iVar1 == 0) {
    iVar1 = this->m_nCurrentDay + 1;
    this->m_nCurrentDay = iVar1;
    if (this->m_nMaxDays < iVar1) {
      this->m_nCurrentDay = 1;
    }
  }
  else {
    iVar1 = _V_stricmp(pCommand,(char *)(unaff_EBX + 0x381bec));
    if (iVar1 != 0) goto LAB_0061eb68;
    iVar1 = this->m_nCurrentDay + -1;
    this->m_nCurrentDay = iVar1;
    if (iVar1 < 1) {
      this->m_nCurrentDay = this->m_nMaxDays;
    }
  }
  UpdateDayLabel(this);
                    /* Unresolved local var: char[32] szServerMsg@[???] */
  V_snprintf(local_40,0x20,(char *)(unaff_EBX + 0x381bd0),this->m_nCurrentDay);
  CObjectControlPanel::SendToServerObject(&this->super_CObjectControlPanel,local_40);
LAB_0061eb68:
  CVGuiScreenPanel::OnCommand((CVGuiScreenPanel *)this,pCommand);
  if (local_20 == **(int **)(unaff_EBX + 0x512230)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}


/* CVGUI_MP_LobbyScreen::CVGUI_MP_LobbyScreen at 0061eff0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* WARNING: Struct "BaseModHybridButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void CVGUI_MP_LobbyScreen(CVGUI_MP_LobbyScreen * this, Panel * parent,
   char * panelName) */

void __thiscall
CVGUI_MP_LobbyScreen::CVGUI_MP_LobbyScreen(CVGUI_MP_LobbyScreen *this,Panel *parent,char *panelName)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CObjectControlPanel::CObjectControlPanel
            (&this->super_CObjectControlPanel,parent,(char *)(unaff_EBX + 0x38175e));
  (this->super_CObjectControlPanel).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a1f07);
  *(undefined4 *)(this->m_cDefault)._color = 0;
  *(undefined4 *)(this->m_cInvisible)._color = 0;
  this->m_nStartDay = 4;
  this->m_nCurrentDay = 4;
  this->m_nMaxDays = 4;
  return;
}


/* CVGUI_MP_LobbyScreen::CVGUI_MP_LobbyScreen at 0061f070 */

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
            (&this->super_CObjectControlPanel,parent,(char *)(unaff_EBX + 0x3816de));
  (this->super_CObjectControlPanel).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a1e87);
  *(undefined4 *)(this->m_cDefault)._color = 0;
  *(undefined4 *)(this->m_cInvisible)._color = 0;
  this->m_nStartDay = 4;
  this->m_nCurrentDay = 4;
  this->m_nMaxDays = 4;
  return;
}


/* CPanelFactory<CVGUI_MP_LobbyScreen,VGuiScreenInitData_t>::Create at 0061f680 */

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
  CObjectControlPanel::CObjectControlPanel(this_00,pParent,(char *)(unaff_EBX + 0x3810cb));
  (this_00->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a1874);
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
  cVar2 = (**(code **)(unaff_EBX + 0x5a1c28))(this_00,pKeyValues,pVoidInitData);
  if (cVar2 == '\0') {
    (*(this_00->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(this_00);
    this_00 = (CObjectControlPanel *)0x0;
  }
  return (Panel *)this_00;
}


/* CVGUI_MP_LobbyScreen::UpdateDayLabel at 0061deb0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void UpdateDayLabel(CVGUI_MP_LobbyScreen * this) */

void __thiscall CVGUI_MP_LobbyScreen::UpdateDayLabel(CVGUI_MP_LobbyScreen *this)

{
  int *piVar1;
  Label *pLVar2;
  char cVar3;
  char *pFormat;
  int iVar4;
  int *piVar5;
  int unaff_EBX;
  char local_a0 [128];
  int local_20;
  
                    /* Unresolved local var: char[128] szDayName@[???] */
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(unaff_EBX + 0x512e4d);
  piVar1 = *(int **)(unaff_EBX + 0x512ea5);
  cVar3 = (**(code **)(*(int *)*piVar1 + 0x88))((int *)*piVar1);
  if (cVar3 != '\0') {
    piVar5 = (int *)0x0;
    if (*piVar1 != 0) {
      piVar5 = (int *)___dynamic_cast(*piVar1,*(undefined4 *)(unaff_EBX + 0x5130d5),
                                      *(undefined4 *)(unaff_EBX + 0x513621),0);
    }
    cVar3 = (**(code **)(*piVar5 + 0x10c))(piVar5);
    if (cVar3 != '\0') {
      iVar4 = 0;
      if (*piVar1 != 0) {
        iVar4 = ___dynamic_cast(*piVar1,*(undefined4 *)(unaff_EBX + 0x5130d5),
                                *(undefined4 *)(unaff_EBX + 0x513621),0);
      }
      this->m_nCurrentDay = *(int *)(iVar4 + 0x48);
      iVar4 = this->m_nCurrentDay;
      goto joined_r0x0061dfcc;
    }
  }
  iVar4 = this->m_nCurrentDay;
joined_r0x0061dfcc:
  if (iVar4 == 4) {
    pFormat = (char *)(unaff_EBX + 0x3826db);
  }
  else {
    pFormat = (char *)(unaff_EBX + 0x3826e8);
  }
  V_snprintf(local_a0,0x80,pFormat);
  pLVar2 = this->m_pDayTextLabel;
  if (pLVar2 != (Label *)0x0) {
    (*(pLVar2->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd4])(pLVar2,local_a0);
  }
  if (local_20 != **(int **)(unaff_EBX + 0x512e4d)) {
                    /* WARNING: Subroutine does not return */
    ___stack_chk_fail();
  }
  return;
}


/* CVGUI_MP_LobbyScreen::ApplySchemeSettings at 0061ddb0 */

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
  iVar2 = (*p_Var1)(pScheme,&UNK_003827b8 + unaff_EBX,iVar2);
  *(int *)(this->m_cDefault)._color = iVar2;
  *(undefined4 *)(this->m_cInvisible)._color = 0;
  pLVar3 = (Label *)vgui::Panel::FindChildByName((Panel *)this,(char *)(unaff_EBX + 0x3827d5),false)
  ;
  this->m_pDayTextLabel = pLVar3;
  return;
}


/* CVGUI_MP_LobbyScreen::Init at 0061dbd0 */

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
  piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x51355c);
  pcVar2 = *(code **)(*piVar1 + 0x54);
  iVar4 = (**(this->super_CObjectControlPanel).super_CVGuiScreenPanel.super_EditablePanel.
             super_Panel.super_IClientPanel._vptr_IClientPanel)(this);
  (*pcVar2)(piVar1,iVar4,0);
  bVar3 = CObjectControlPanel::Init(&this->super_CObjectControlPanel,pKeyValues,pInitData);
  return bVar3;
}


/* CVGUI_MP_LobbyScreen::NextDay at 0061e020 */

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


/* CVGUI_MP_LobbyScreen::PreviousDay at 0061dff0 */

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


/* CVGUI_MP_LobbyScreen::SelectDay at 0061ea50 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void SelectDay(CVGUI_MP_LobbyScreen * this) */

void __thiscall CVGUI_MP_LobbyScreen::SelectDay(CVGUI_MP_LobbyScreen *this)

{
  int unaff_EBX;
  char local_40 [32];
  int local_20;
  
                    /* Unresolved local var: char[32] szServerMsg@[???] */
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(unaff_EBX + 0x5122b0);
  V_snprintf(local_40,0x20,(char *)(unaff_EBX + 0x381c50),this->m_nCurrentDay);
  CObjectControlPanel::SendToServerObject(&this->super_CObjectControlPanel,local_40);
  if (local_20 == **(int **)(unaff_EBX + 0x5122b0)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}


/* CVGUI_MP_LobbyScreen::OnTick at 0061e8f0 */

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


/* CVGUI_MP_LobbyScreen::Update at 0061d8d0 */

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


/* CVGUI_MP_LevelSelectScreen4::~CVGUI_MP_LevelSelectScreen4 at 0061f1d0 */

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
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(extraout_ECX + 0x5a1950);
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(*(int *)(extraout_ECX + 0x512238) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,__in_chrg);
  return;
}


/* CVGUI_MP_LevelSelectScreen::~CVGUI_MP_LevelSelectScreen at 0061f200 */

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
  _vptr_IClientPanel = (_func_int_varargs **)(extraout_ECX + 0x5a1920);
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(*(int *)(extraout_ECX + 0x512208) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,__in_chrg);
  return;
}


/* CVGUI_MP_LevelSelectScreen2::~CVGUI_MP_LevelSelectScreen2 at 0061f230 */

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
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a18e7);
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x5121cf) + 8)
  ;
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CVGUI_MP_LevelSelectScreen::~CVGUI_MP_LevelSelectScreen at 0061f280 */

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
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a1897);
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x51217f) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CVGUI_MP_LevelSelectScreen2::~CVGUI_MP_LevelSelectScreen2 at 0061f7a0 */

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
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(extraout_ECX + 0x5a1380);
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(*(int *)(extraout_ECX + 0x511c68) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,__in_chrg);
  return;
}


/* CVGUI_MP_LevelSelectScreen3::~CVGUI_MP_LevelSelectScreen3 at 0061f7d0 */

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
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a1347);
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x511c2f) + 8)
  ;
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CVGUI_MP_LevelSelectScreen3::~CVGUI_MP_LevelSelectScreen3 at 0061f820 */

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
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(extraout_ECX + 0x5a1300);
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(*(int *)(extraout_ECX + 0x511be8) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,__in_chrg);
  return;
}


/* CVGUI_MP_LevelSelectScreen4::~CVGUI_MP_LevelSelectScreen4 at 0061f850 */

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
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a12c7);
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x511baf) + 8)
  ;
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CVGUI_MP_LevelSelectScreen::CVGUI_MP_LevelSelectScreen at 0061ec00 */

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
  CVGuiScreenPanel::CVGuiScreenPanel(&this->super_CVGuiScreenPanel,parent,&UNK_00381aca + unaff_EBX)
  ;
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a1f1a);
  this->m_nDay = 1;
  this->m_nCurrentLevel = 0;
  this->m_bInitializedLevelLabel = false;
  local_20 = 0;
  do {
    name = VarArgs((char *)(unaff_EBX + 0x381ae5),local_20);
    this_00 = operator_new(0x17c);
    vgui::ImagePanel::ImagePanel
              (this_00,(Panel *)this,name,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
    this->m_pLevelIconPanels[local_20] = this_00;
    local_20 = local_20 + 1;
  } while (local_20 != 0x10);
  return;
}


/* CVGUI_MP_LevelSelectScreen::CVGUI_MP_LevelSelectScreen at 0061eed0 */

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


/* CPanelFactory<CVGUI_MP_LevelSelectScreen,VGuiScreenInitData_t>::Create at 0061f2d0 */

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


/* CVGUI_MP_LevelSelectScreen::UpdateLevelLabel at 0061e050 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void UpdateLevelLabel(CVGUI_MP_LevelSelectScreen * this) */

void __thiscall CVGUI_MP_LevelSelectScreen::UpdateLevelLabel(CVGUI_MP_LevelSelectScreen *this)

{
  Label *pLVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  ImagePanel *pIVar4;
  _func_int_varargs *p_Var5;
  char cVar6;
  int iVar7;
  AnimationController *this_00;
  int iVar8;
  int *piVar9;
  int unaff_EBX;
  int iVar10;
  CVGUI_MP_LevelSelectScreen *pCVar11;
  int local_14c;
  int local_148;
  int local_144;
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
  local_20 = **(int **)(unaff_EBX + 0x512cb3);
  cVar6 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x512d0b) + 0x88))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x512d0b));
  if (cVar6 != '\0') {
    piVar9 = (int *)0x0;
    if (**(int **)(unaff_EBX + 0x512d0b) != 0) {
      piVar9 = (int *)___dynamic_cast(**(int **)(unaff_EBX + 0x512d0b),
                                      *(undefined4 *)(unaff_EBX + 0x512f3b),
                                      *(undefined4 *)(unaff_EBX + 0x513487),0);
    }
    cVar6 = (**(code **)(*piVar9 + 0x10c))(piVar9);
    if ((cVar6 != '\0') && (this->m_bInitializedLevelLabel != false)) {
      iVar10 = this->m_nDay + -1;
      if (**(int **)(unaff_EBX + 0x512d0b) == 0) {
        local_14c = *(int *)(::__static_initialization_and_destruction_0 + iVar10 * 4 + 4);
        iVar7 = 0;
      }
      else {
        iVar7 = ___dynamic_cast(**(int **)(unaff_EBX + 0x512d0b),
                                *(undefined4 *)(unaff_EBX + 0x512f3b),
                                *(undefined4 *)(unaff_EBX + 0x513487),0);
        local_14c = *(int *)(iVar7 + 0x1064 + iVar10 * 4);
      }
      iVar7 = *(int *)(iVar7 + 0x4c + iVar10 * 4);
      if (iVar7 != this->m_nCurrentLevel) {
        this->m_nCurrentLevel = iVar7;
        V_snprintf(local_a0,0x80,(char *)(unaff_EBX + 0x382555),iVar7);
        iVar7 = this->m_nCurrentLevel;
        iVar8 = 0;
        if (**(int **)(unaff_EBX + 0x512d0b) != 0) {
          iVar8 = ___dynamic_cast(**(int **)(unaff_EBX + 0x512d0b),
                                  *(undefined4 *)(unaff_EBX + 0x512f3b),
                                  *(undefined4 *)(unaff_EBX + 0x513487),0);
        }
        V_snprintf(local_120,0x80,(char *)(unaff_EBX + 0x38255e),
                   iVar8 + 0x21 + (iVar10 * 0x10 + iVar7) * 0x40);
        pLVar1 = this->m_pLevelNumLabel;
        if ((pLVar1 != (Label *)0x0) && (this->m_bInitializedLevelLabel != false)) {
          (*(pLVar1->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd4])(pLVar1,local_a0);
        }
        pLVar1 = this->m_pLevelTextLabel;
        if ((pLVar1 != (Label *)0x0) && (this->m_bInitializedLevelLabel != false)) {
          (*(pLVar1->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd4])(pLVar1,local_120);
        }
        vgui::Panel::GetSize((Panel *)this,&local_124,&local_128);
        if (0 < local_14c) {
          local_148 = 0;
          uVar2 = *(undefined4 *)(unaff_EBX + 0x513487);
          uVar3 = *(undefined4 *)(unaff_EBX + 0x512f3b);
          pCVar11 = this;
          do {
            iVar10 = this->m_nCurrentLevel + -1;
            if (iVar10 == local_148) {
              (*(this->m_pLevelIconPanels[local_148]->super_Panel).super_IClientPanel.
                _vptr_IClientPanel[0xd5])(this->m_pLevelIconPanels[local_148],unaff_EBX + 0x382563);
              pIVar4 = this->m_pLevelIconPanels[local_148];
              p_Var5 = (pIVar4->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd8];
              iVar7 = this->m_nDay;
              iVar8 = 0;
              if (**(int **)(unaff_EBX + 0x512d0b) != 0) {
                iVar8 = ___dynamic_cast(**(int **)(unaff_EBX + 0x512d0b),uVar3,uVar2,0);
              }
              (*p_Var5)(pIVar4,(uint)(byte)~-(::__static_initialization_and_destruction_0
                                              [local_148 + iVar7 * 0x10 + iVar8 + 4] == (code)0x0) |
                               CONCAT22((byte)-(::__static_initialization_and_destruction_0
                                                [local_148 + iVar7 * 0x10 + iVar8 + 4] == (code)0x0)
                                        | 0xff00,0xc800));
              vgui::Panel::SetSize(&pCVar11->m_pLevelIconPanels[0]->super_Panel,0x96,0x96);
              vgui::Panel::GetPos(&pCVar11->m_pLevelIconPanels[0]->super_Panel,&local_12c,&local_130
                                 );
              vgui::Panel::SetPos(&pCVar11->m_pLevelIconPanels[0]->super_Panel,local_12c,0x26c);
              local_144 = local_124 / 2 + -0x4b + (iVar10 - (this->m_nCurrentLevel + -1)) * 0x96;
            }
            else {
              iVar10 = this->m_nDay;
              iVar7 = 0;
              if (**(int **)(unaff_EBX + 0x512d0b) != 0) {
                iVar7 = ___dynamic_cast(**(int **)(unaff_EBX + 0x512d0b),
                                        *(undefined4 *)(unaff_EBX + 0x512f3b),
                                        *(undefined4 *)(unaff_EBX + 0x513487),0);
              }
              if (*(char *)(local_148 + 0x1064 + iVar10 * 0x10 + iVar7) == '\0') {
                (*(pCVar11->m_pLevelIconPanels[0]->super_Panel).super_IClientPanel.
                  _vptr_IClientPanel[0xd8])(pCVar11->m_pLevelIconPanels[0],0xffffffff);
                (*(pCVar11->m_pLevelIconPanels[0]->super_Panel).super_IClientPanel.
                  _vptr_IClientPanel[0xd5])(pCVar11->m_pLevelIconPanels[0],unaff_EBX + 0x3825a5);
              }
              else {
                (*(pCVar11->m_pLevelIconPanels[0]->super_Panel).super_IClientPanel.
                  _vptr_IClientPanel[0xd8])(pCVar11->m_pLevelIconPanels[0],0xffffffff);
                (*(pCVar11->m_pLevelIconPanels[0]->super_Panel).super_IClientPanel.
                  _vptr_IClientPanel[0xd5])(pCVar11->m_pLevelIconPanels[0],unaff_EBX + 0x382583);
              }
              vgui::Panel::SetSize(&pCVar11->m_pLevelIconPanels[0]->super_Panel,0x78,0x78);
              vgui::Panel::GetPos(&pCVar11->m_pLevelIconPanels[0]->super_Panel,&local_12c,&local_130
                                 );
              vgui::Panel::SetPos(&pCVar11->m_pLevelIconPanels[0]->super_Panel,local_12c,0x27b);
              local_144 = local_124 / 2 + -0x3c + (local_148 - (this->m_nCurrentLevel + -1)) * 0x96;
            }
            pIVar4 = pCVar11->m_pLevelIconPanels[0];
            this_00 = vgui::GetAnimationController();
            vgui::AnimationController::RunAnimationCommand
                      (this_00,&pIVar4->super_Panel,&UNK_0037e947 + unaff_EBX,(float)local_144,0.0,
                       0.25,INTERPOLATOR_DEACCEL,0.0);
            (*(pCVar11->m_pLevelIconPanels[0]->super_Panel).super_IClientPanel._vptr_IClientPanel
              [0x48])(pCVar11->m_pLevelIconPanels[0],1);
            iVar10 = vgui::Panel::GetZPos(&this->m_pBranchTitleLabel->super_Panel);
            vgui::Panel::SetZPos(&pCVar11->m_pLevelIconPanels[0]->super_Panel,iVar10);
            (*(pCVar11->m_pLevelIconPanels[0]->super_Panel).super_IClientPanel._vptr_IClientPanel
              [0x22])(pCVar11->m_pLevelIconPanels[0],1);
            local_148 = local_148 + 1;
            pCVar11 = (CVGUI_MP_LevelSelectScreen *)
                      &(pCVar11->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.
                       m_RegisterClass;
          } while (local_148 != local_14c);
        }
      }
    }
  }
  if (local_20 != **(int **)(unaff_EBX + 0x512cb3)) {
                    /* WARNING: Subroutine does not return */
    ___stack_chk_fail();
  }
  return;
}


/* CVGUI_MP_LevelSelectScreen::ApplySchemeSettings at 0061dc40 */

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
  pLVar1 = (Label *)vgui::Panel::FindChildByName((Panel *)this,(char *)(unaff_EBX + 0x382865),false)
  ;
  this->m_pBranchTitleLabel = pLVar1;
  pLVar1 = (Label *)vgui::Panel::FindChildByName((Panel *)this,(char *)(unaff_EBX + 0x382871),false)
  ;
  this->m_pSelectLevelLabel = pLVar1;
  pLVar1 = (Label *)vgui::Panel::FindChildByName((Panel *)this,(char *)(unaff_EBX + 0x382882),false)
  ;
  this->m_pLevelNumLabel = pLVar1;
  pLVar1 = (Label *)vgui::Panel::FindChildByName((Panel *)this,(char *)(unaff_EBX + 0x382893),false)
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


/* CVGUI_MP_LevelSelectScreen::PerformLayout at 0061d940 */

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


/* CVGUI_MP_LevelSelectScreen::Init at 0061de40 */

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
  piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x5132ec);
  pcVar2 = *(code **)(*piVar1 + 0x54);
  iVar4 = (**(this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
             _vptr_IClientPanel)(this);
  (*pcVar2)(piVar1,iVar4,0);
  bVar3 = CVGuiScreenPanel::Init(&this->super_CVGuiScreenPanel,pKeyValues,pInitData);
  return bVar3;
}


/* CVGUI_MP_LevelSelectScreen::OnTick at 0061d8f0 */

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
                    /* WARNING: Could not recover jumptable at 0x0061d936. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
    _vptr_IClientPanel[0xee])();
  return;
}


/* CVGUI_MP_LevelSelectScreen2::CVGUI_MP_LevelSelectScreen2 at 0061ee30 */

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
            (&this->super_CVGUI_MP_LevelSelectScreen,parent,&UNK_003818f6 + unaff_EBX);
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a3127);
  (this->super_CVGUI_MP_LevelSelectScreen).m_nDay = 2;
  return;
}


/* CVGUI_MP_LevelSelectScreen2::CVGUI_MP_LevelSelectScreen2 at 0061ee80 */

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
            (&this->super_CVGUI_MP_LevelSelectScreen,parent,&UNK_003818a6 + unaff_EBX);
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a30d7);
  (this->super_CVGUI_MP_LevelSelectScreen).m_nDay = 2;
  return;
}


/* CPanelFactory<CVGUI_MP_LevelSelectScreen2,VGuiScreenInitData_t>::Create at 0061f360 */

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
            (this_00,pParent,(char *)(unaff_EBX + 0x3813c3));
  (this_00->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a2bf4);
  this_00->m_nDay = 2;
                    /* Unresolved local var: VGuiScreenInitData_t * pInitData@[???] */
  cVar1 = (**(code **)(unaff_EBX + 0x5a2fa8))(this_00,pKeyValues,pVoidInitData);
  if (cVar1 == '\0') {
    (*(this_00->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(this_00);
    this_00 = (CVGUI_MP_LevelSelectScreen *)0x0;
  }
  return (Panel *)this_00;
}


/* CVGUI_MP_LevelSelectScreen3::CVGUI_MP_LevelSelectScreen3 at 0061ed90 */

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
            (&this->super_CVGUI_MP_LevelSelectScreen,parent,(char *)(unaff_EBX + 0x38197a));
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a2de7);
  (this->super_CVGUI_MP_LevelSelectScreen).m_nDay = 3;
  return;
}


/* CVGUI_MP_LevelSelectScreen3::CVGUI_MP_LevelSelectScreen3 at 0061ede0 */

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
            (&this->super_CVGUI_MP_LevelSelectScreen,parent,&UNK_0038192a + unaff_EBX);
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a2d97);
  (this->super_CVGUI_MP_LevelSelectScreen).m_nDay = 3;
  return;
}


/* CPanelFactory<CVGUI_MP_LevelSelectScreen3,VGuiScreenInitData_t>::Create at 0061f400 */

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
            (this_00,pParent,(char *)(unaff_EBX + 0x381307));
  (this_00->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a2774);
  this_00->m_nDay = 3;
                    /* Unresolved local var: VGuiScreenInitData_t * pInitData@[???] */
  cVar1 = (**(code **)(unaff_EBX + 0x5a2b28))(this_00,pKeyValues,pVoidInitData);
  if (cVar1 == '\0') {
    (*(this_00->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(this_00);
    this_00 = (CVGUI_MP_LevelSelectScreen *)0x0;
  }
  return (Panel *)this_00;
}


/* CVGUI_MP_LevelSelectScreen4::CVGUI_MP_LevelSelectScreen4 at 0061ecf0 */

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
            (&this->super_CVGUI_MP_LevelSelectScreen,parent,(char *)(unaff_EBX + 0x3819fe));
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a2aa7);
  (this->super_CVGUI_MP_LevelSelectScreen).m_nDay = 4;
  return;
}


/* CVGUI_MP_LevelSelectScreen4::CVGUI_MP_LevelSelectScreen4 at 0061ed40 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* WARNING: Struct "BaseModHybridButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void CVGUI_MP_LevelSelectScreen4(CVGUI_MP_LevelSelectScreen4 * this,
   Panel * parent, char * panelName) */

void __thiscall
CVGUI_MP_LevelSelectScreen4::CVGUI_MP_LevelSelectScreen4
          (CVGUI_MP_LevelSelectScreen4 *this,Panel *parent,char *panelName)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CVGUI_MP_LevelSelectScreen::CVGUI_MP_LevelSelectScreen
            (&this->super_CVGUI_MP_LevelSelectScreen,parent,(char *)(unaff_EBX + 0x3819ae));
  (this->super_CVGUI_MP_LevelSelectScreen).super_CVGuiScreenPanel.super_EditablePanel.super_Panel.
  super_IClientPanel._vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a2a57);
  (this->super_CVGUI_MP_LevelSelectScreen).m_nDay = 4;
  return;
}


/* CPanelFactory<CVGUI_MP_LevelSelectScreen4,VGuiScreenInitData_t>::Create at 0061f4a0 */

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
            (this_00,pParent,(char *)(unaff_EBX + 0x38124b));
  (this_00->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a22f4);
  this_00->m_nDay = 4;
                    /* Unresolved local var: VGuiScreenInitData_t * pInitData@[???] */
  cVar1 = (**(code **)(unaff_EBX + 0x5a26a8))(this_00,pKeyValues,pVoidInitData);
  if (cVar1 == '\0') {
    (*(this_00->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0x21])(this_00);
    this_00 = (CVGUI_MP_LevelSelectScreen *)0x0;
  }
  return (Panel *)this_00;
}


/* __static_initialization_and_destruction_0 at 00083070 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  IPanelMetaClassMgr *pIVar1;
  int unaff_EBX;
  longlong lVar2;
  
  lVar2 = ___i686_get_pc_thunk_bx();
  if (lVar2 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)UTIL_GetLocalizedKeyString::useString[3] + unaff_EBX + 0x3b8) = 0;
  *(undefined1 *)((int)UTIL_GetLocalizedKeyString::useString[3] + unaff_EBX + 0x3b9) = 0;
  *(undefined1 *)((int)UTIL_GetLocalizedKeyString::useString[3] + unaff_EBX + 0x3ba) = 0;
  *(undefined1 *)((int)UTIL_GetLocalizedKeyString::useString[3] + unaff_EBX + 0x3bb) = 0;
  *(undefined4 *)((int)UTIL_GetLocalizedKeyString::useString[3] + unaff_EBX + 0x3bc) = 0;
  *(undefined4 *)((int)UTIL_GetLocalizedKeyString::useString[3] + unaff_EBX + 0x3c0) = 0;
  *(undefined4 *)((int)UTIL_GetLocalizedKeyString::useString[3] + unaff_EBX + 0x3c4) = 0x7f7fffff;
  *(undefined4 *)((int)UTIL_GetLocalizedKeyString::useString[3] + unaff_EBX + 0x3c8) = 0x7f7fffff;
  *(undefined4 *)((int)UTIL_GetLocalizedKeyString::useString[3] + unaff_EBX + 0x3cc) = 0;
  *(undefined4 *)((int)UTIL_GetLocalizedKeyString::useString[3] + unaff_EBX + 0x3d0) = 0;
  *(undefined4 *)((int)UTIL_GetLocalizedKeyString::useString[3] + unaff_EBX + 0x3d4) = 0;
  *(undefined4 *)((int)UTIL_GetLocalizedKeyString::useString[3] + unaff_EBX + 0x3d8) = 0;
  *(undefined4 *)((int)UTIL_GetLocalizedKeyString::useString[3] + unaff_EBX + 0x3dc) = 0x7f7fffff;
  *(undefined4 *)((int)UTIL_GetLocalizedKeyString::useString[3] + unaff_EBX + 0x3e0) = 0x7f7fffff;
  *(undefined4 *)((int)UTIL_GetLocalizedKeyString::useString[3] + unaff_EBX + 0x3e4) = 0x7f7fffff;
  *(undefined4 *)((int)UTIL_GetLocalizedKeyString::useString[3] + unaff_EBX + 1000) = 0x7f7fffff;
  *(undefined **)((int)UTIL_GetLocalizedKeyString::useString[3] + unaff_EBX + 0x3ec) =
       &UNK_00aaed8c + unaff_EBX;
  *(int *)((int)ice_sbox[1] + unaff_EBX + 0xca4) =
       (int)&PTR_ShouldDrawForSplitScreenUser_00b3e66c + unaff_EBX;
  pIVar1 = PanelMetaClassMgr();
  (*pIVar1->_vptr_IPanelMetaClassMgr[1])(pIVar1,unaff_EBX + 0x91d473,unaff_EBX + 0xcce564);
  *(int *)((int)ice_sbox[1] + unaff_EBX + 0xca8) = (int)&PTR_GetModelInstance_00b3e69c + unaff_EBX;
  pIVar1 = PanelMetaClassMgr();
  (*pIVar1->_vptr_IPanelMetaClassMgr[1])(pIVar1,unaff_EBX + 0x91d488,unaff_EBX + 0xcce568);
  *(int *)((int)ice_sbox[1] + unaff_EBX + 0xcac) =
       (int)&PTR_OnPredictedEntityRemove_00b3e6b4 + unaff_EBX;
  pIVar1 = PanelMetaClassMgr();
  (*pIVar1->_vptr_IPanelMetaClassMgr[1])(pIVar1,unaff_EBX + 0x91d498,unaff_EBX + 0xcce56c);
  *(int *)((int)ice_sbox[1] + unaff_EBX + 0xcb0) = (int)&PTR_GetStudioBody_00b3e6cc + unaff_EBX;
  pIVar1 = PanelMetaClassMgr();
  (*pIVar1->_vptr_IPanelMetaClassMgr[1])(pIVar1,unaff_EBX + 0x91d4b2,unaff_EBX + 0xcce570);
  *(int *)((int)ice_sbox[1] + unaff_EBX + 0xcb4) = (int)&PTR_GetIClientUnknown_00b3e6e4 + unaff_EBX;
  pIVar1 = PanelMetaClassMgr();
  (*pIVar1->_vptr_IPanelMetaClassMgr[1])(pIVar1,unaff_EBX + 0x91d4cc,unaff_EBX + 0xcce574);
  *(int *)((int)ice_sbox[1] + unaff_EBX + 0xcb8) = (int)&PTR_GetShadowHandle_00b3e6fc + unaff_EBX;
  pIVar1 = PanelMetaClassMgr();
  (*pIVar1->_vptr_IPanelMetaClassMgr[1])(pIVar1,unaff_EBX + 0x91d4e6,unaff_EBX + 0xcce578);
  return;
}


/* _GLOBAL__I_g_CObjectControlPanelFactory at 00083240 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_g_CObjectControlPanelFactory(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

