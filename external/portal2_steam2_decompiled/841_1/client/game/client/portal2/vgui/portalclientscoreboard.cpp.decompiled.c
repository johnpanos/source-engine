/* DWARF-guided pseudocode for game/client/portal2/vgui/portalclientscoreboard.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* CPortalClientScoreBoardDialog::CPortalClientScoreBoardDialog at 00623bb0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CClientScoreBoardDialog": ignoring multiple overlapping fields */
/* DWARF original prototype: void CPortalClientScoreBoardDialog(CPortalClientScoreBoardDialog *
   this, IViewPort * pViewPort) */

void __thiscall
CPortalClientScoreBoardDialog::CPortalClientScoreBoardDialog
          (CPortalClientScoreBoardDialog *this,IViewPort *pViewPort)

{
  char cVar1;
  PanelKeyBindingMap *pPVar2;
  PanelKeyBindingMap *pPVar3;
  SectionedListPanel *pSVar4;
  Label *pLVar5;
  PanelMessageMap *pPVar6;
  PanelMessageMap *pPVar7;
  PanelAnimationMap *pPVar8;
  PanelAnimationMap *pPVar9;
  int unaff_EBX;
  SectionedListPanel *in_stack_ffffffd0;
  Label *in_stack_ffffffd4;
  Panel *in_stack_ffffffd8;
  char *in_stack_ffffffdc;
  char *text;
  
  ___i686_get_pc_thunk_bx();
  CClientScoreBoardDialog::CClientScoreBoardDialog(&this->super_CClientScoreBoardDialog,pViewPort);
  (this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5d05ea);
  (this->super_CClientScoreBoardDialog).super_IViewPortPanel._vptr_IViewPortPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5d09f2);
  (this->super_CClientScoreBoardDialog).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(unaff_EBX + 0x5d0a3a);
  if (*(char *)(unaff_EBX + 0x63731a) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x63731a) = 1;
    pPVar6 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3a70cd));
    pPVar6->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x609f2a);
    pPVar7 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x38a8ce));
    pPVar6->baseMap = pPVar7;
    cVar1 = *(char *)(unaff_EBX + 0x63731b);
  }
  else {
    cVar1 = *(char *)(unaff_EBX + 0x63731b);
  }
  if (cVar1 == '\0') {
    *(undefined1 *)(unaff_EBX + 0x63731b) = 1;
    pPVar8 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x3a70cd));
    pPVar8->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x609f2a);
    pPVar9 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x38a8ce));
    pPVar8->baseMap = pPVar9;
  }
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
  if (*(char *)(unaff_EBX + 0x63731c) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x63731c) = 1;
    pPVar2 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3a70cd));
    pPVar2->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x609f2a);
    pPVar3 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x38a8ce));
    pPVar2->baseMap = pPVar3;
  }
  *(undefined4 *)(this->m_bgColor)._color = 0;
  *(undefined4 *)(this->m_borderColor)._color = 0;
  pSVar4 = operator_new(0x1fc);
  vgui::SectionedListPanel::SectionedListPanel
            (pSVar4,(Panel *)this,(char *)(unaff_EBX + 0x3a7250),in_stack_ffffffd0,
             &in_stack_ffffffd4->super_Panel,(char *)in_stack_ffffffd8);
  this->m_pPlayerListT = pSVar4;
  pSVar4 = operator_new(0x1fc);
  vgui::SectionedListPanel::SectionedListPanel
            (pSVar4,(Panel *)this,(char *)(unaff_EBX + 0x3a725c),in_stack_ffffffd0,
             &in_stack_ffffffd4->super_Panel,(char *)in_stack_ffffffd8);
  this->m_pPlayerListCT = pSVar4;
  pLVar5 = operator_new(0x1a4);
  text = (char *)(unaff_EBX + 0x3650a6);
  vgui::Label::Label(pLVar5,(Panel *)this,(char *)(unaff_EBX + 0x3a7269),text,in_stack_ffffffd4,
                     in_stack_ffffffd8,in_stack_ffffffdc,text);
  this->m_pPlayerCountLabel_T = pLVar5;
  pLVar5 = operator_new(0x1a4);
  vgui::Label::Label(pLVar5,(Panel *)this,&DAT_003a7277 + unaff_EBX,text,in_stack_ffffffd4,
                     in_stack_ffffffd8,in_stack_ffffffdc,text);
  this->m_pScoreLabel_T = pLVar5;
  pLVar5 = operator_new(0x1a4);
  vgui::Label::Label(pLVar5,(Panel *)this,&UNK_003a727f + unaff_EBX,text,in_stack_ffffffd4,
                     in_stack_ffffffd8,in_stack_ffffffdc,text);
  this->m_pPingLabel_T = pLVar5;
  pLVar5 = operator_new(0x1a4);
  vgui::Label::Label(pLVar5,(Panel *)this,&UNK_003a7289 + unaff_EBX,text,in_stack_ffffffd4,
                     in_stack_ffffffd8,in_stack_ffffffdc,text);
  this->m_pPlayerCountLabel_CT = pLVar5;
  pLVar5 = operator_new(0x1a4);
  vgui::Label::Label(pLVar5,(Panel *)this,&UNK_003a7298 + unaff_EBX,text,in_stack_ffffffd4,
                     in_stack_ffffffd8,in_stack_ffffffdc,text);
  this->m_pScoreLabel_CT = pLVar5;
  pLVar5 = operator_new(0x1a4);
  vgui::Label::Label(pLVar5,(Panel *)this,&UNK_003a72a1 + unaff_EBX,text,in_stack_ffffffd4,
                     in_stack_ffffffd8,in_stack_ffffffdc,text);
  this->m_pPingLabel_CT = pLVar5;
                    /* Unresolved local var: bool bServerSide@[???] */
  (this->super_CClientScoreBoardDialog).super_CGameEventListener.m_bRegisteredForEvents = true;
  (**(code **)(*(int *)**(undefined4 **)(&DAT_005401be + unaff_EBX) + 0x10))
            ((int *)**(undefined4 **)(&DAT_005401be + unaff_EBX),
             &(this->super_CClientScoreBoardDialog).super_CGameEventListener,
             &UNK_0038a998 + unaff_EBX,0);
  vgui::EditablePanel::SetDialogVariable((EditablePanel *)this,(char *)(unaff_EBX + 0x371130),text);
  vgui::Panel::SetVisible((Panel *)this,false);
  return;
}


/* CPortalClientScoreBoardDialog::CPortalClientScoreBoardDialog at 00623f00 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CClientScoreBoardDialog": ignoring multiple overlapping fields */
/* DWARF original prototype: void CPortalClientScoreBoardDialog(CPortalClientScoreBoardDialog *
   this, IViewPort * pViewPort, CPortalClientScoreBoardDialog * this, IViewPort * pViewPort) */

void __thiscall
CPortalClientScoreBoardDialog::CPortalClientScoreBoardDialog
          (CPortalClientScoreBoardDialog *this,IViewPort *pViewPort,
          CPortalClientScoreBoardDialog *this_1,IViewPort *pViewPort_1)

{
  CPortalClientScoreBoardDialog(this,pViewPort);
  return;
}


/* CPortalClientScoreBoardDialog::~CPortalClientScoreBoardDialog at 00623470 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CClientScoreBoardDialog": ignoring multiple overlapping fields */
/* DWARF original prototype: void ~CPortalClientScoreBoardDialog(CPortalClientScoreBoardDialog *
   this, int __in_chrg) */

void __thiscall
CPortalClientScoreBoardDialog::~CPortalClientScoreBoardDialog
          (CPortalClientScoreBoardDialog *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5d0d27);
  (this->super_CClientScoreBoardDialog).super_IViewPortPanel._vptr_IViewPortPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5d112f);
  (this->super_CClientScoreBoardDialog).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(unaff_EBX + 0x5d1177);
  CClientScoreBoardDialog::~CClientScoreBoardDialog
            (&this->super_CClientScoreBoardDialog,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CPortalClientScoreBoardDialog::~CPortalClientScoreBoardDialog at 006234f0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CClientScoreBoardDialog": ignoring multiple overlapping fields */
/* DWARF original prototype: void ~CPortalClientScoreBoardDialog(CPortalClientScoreBoardDialog *
   this, int __in_chrg) */

void __thiscall
CPortalClientScoreBoardDialog::~CPortalClientScoreBoardDialog
          (CPortalClientScoreBoardDialog *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(extraout_ECX + 0x5d0cb0);
  (this->super_CClientScoreBoardDialog).super_IViewPortPanel._vptr_IViewPortPanel =
       (_func_int_varargs **)(extraout_ECX + 0x5d10b8);
  (this->super_CClientScoreBoardDialog).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(extraout_ECX + 0x5d1100);
  CClientScoreBoardDialog::~CClientScoreBoardDialog(&this->super_CClientScoreBoardDialog,__in_chrg);
  return;
}


/* CPortalClientScoreBoardDialog::~CPortalClientScoreBoardDialog at 00623540 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CClientScoreBoardDialog": ignoring multiple overlapping fields */
/* DWARF original prototype: void ~CPortalClientScoreBoardDialog(CPortalClientScoreBoardDialog *
   this, int __in_chrg) */

void __thiscall
CPortalClientScoreBoardDialog::~CPortalClientScoreBoardDialog
          (CPortalClientScoreBoardDialog *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(BaseModUI::GenericPanelList::Paint + extraout_ECX);
  (this->super_CClientScoreBoardDialog).super_IViewPortPanel._vptr_IViewPortPanel =
       (_func_int_varargs **)(extraout_ECX + 0x5d1068);
  (this->super_CClientScoreBoardDialog).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(extraout_ECX + 0x5d10b0);
  CClientScoreBoardDialog::~CClientScoreBoardDialog(&this->super_CClientScoreBoardDialog,__in_chrg);
  return;
}


/* CPortalClientScoreBoardDialog::PaintBackground at 00623440 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CClientScoreBoardDialog": ignoring multiple overlapping fields */
/* DWARF original prototype: void PaintBackground(CPortalClientScoreBoardDialog * this) */

void __thiscall CPortalClientScoreBoardDialog::PaintBackground(CPortalClientScoreBoardDialog *this)

{
  vgui::EditablePanel::PaintBackground((EditablePanel *)this);
  return;
}


/* CPortalClientScoreBoardDialog::PaintBorder at 00623430 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CClientScoreBoardDialog": ignoring multiple overlapping fields */
/* DWARF original prototype: void PaintBorder(CPortalClientScoreBoardDialog * this) */

void __thiscall CPortalClientScoreBoardDialog::PaintBorder(CPortalClientScoreBoardDialog *this)

{
  vgui::Panel::PaintBorder((Panel *)this);
  return;
}


/* CPortalClientScoreBoardDialog::ApplySchemeSettings at 00623590 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CClientScoreBoardDialog": ignoring multiple overlapping fields */
/* DWARF original prototype: void ApplySchemeSettings(CPortalClientScoreBoardDialog * this, IScheme
   * pScheme) */

void __thiscall
CPortalClientScoreBoardDialog::ApplySchemeSettings
          (CPortalClientScoreBoardDialog *this,IScheme *pScheme)

{
  _func_int_varargs **pp_Var1;
  _func_int_varargs *p_Var2;
  SectionedListPanel *pSVar3;
  Label *pLVar4;
  int iVar5;
  int unaff_EBX;
  uint uVar6;
  
  ___i686_get_pc_thunk_bx();
  CClientScoreBoardDialog::ApplySchemeSettings(&this->super_CClientScoreBoardDialog,pScheme);
  (*(this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel.
    _vptr_IClientPanel[0xd4])(this,unaff_EBX + 0x3a7804,0,0,0);
  pp_Var1 = (this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel
            ._vptr_IClientPanel;
  p_Var2 = pp_Var1[0x58];
  iVar5 = (*pp_Var1[0x3e])(this);
  iVar5 = (*p_Var2)(this,unaff_EBX + 0x3bd7f8,iVar5,pScheme);
  *(int *)(this->m_bgColor)._color = iVar5;
  iVar5 = (*(pScheme->super_IBaseInterface)._vptr_IBaseInterface[6])(pScheme,unaff_EBX + 0x3a7822,0)
  ;
  *(int *)(this->m_borderColor)._color = iVar5;
  (*(this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel.
    _vptr_IClientPanel[0x3c])(this,0xa0282828);
  p_Var2 = (this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel.
           _vptr_IClientPanel[0x45];
  iVar5 = (*(pScheme->super_IBaseInterface)._vptr_IBaseInterface[3])(pScheme,unaff_EBX + 0x3a782a);
  (*p_Var2)(this,iVar5);
  pSVar3 = this->m_pPlayerListT;
  if (pSVar3 != (SectionedListPanel *)0x0) {
    (*(pSVar3->super_Panel).super_IClientPanel._vptr_IClientPanel[0x100])
              (pSVar3,(this->super_CClientScoreBoardDialog).m_pImageList,0);
    (*(this->m_pPlayerListT->super_Panel).super_IClientPanel._vptr_IClientPanel[0x3c])
              (this->m_pPlayerListT,0);
    (*(this->m_pPlayerListT->super_Panel).super_IClientPanel._vptr_IClientPanel[0x45])
              (this->m_pPlayerListT,0);
    (*(this->m_pPlayerListT->super_Panel).super_IClientPanel._vptr_IClientPanel[0x22])
              (this->m_pPlayerListT,1);
  }
  pSVar3 = this->m_pPlayerListCT;
  if (pSVar3 != (SectionedListPanel *)0x0) {
    (*(pSVar3->super_Panel).super_IClientPanel._vptr_IClientPanel[0x100])
              (pSVar3,(this->super_CClientScoreBoardDialog).m_pImageList,0);
    (*(this->m_pPlayerListCT->super_Panel).super_IClientPanel._vptr_IClientPanel[0x3c])
              (this->m_pPlayerListCT,0);
    (*(this->m_pPlayerListCT->super_Panel).super_IClientPanel._vptr_IClientPanel[0x45])
              (this->m_pPlayerListCT,0);
    (*(this->m_pPlayerListCT->super_Panel).super_IClientPanel._vptr_IClientPanel[0x22])
              (this->m_pPlayerListCT,1);
  }
  pSVar3 = (this->super_CClientScoreBoardDialog).m_pPlayerList;
  if (pSVar3 != (SectionedListPanel *)0x0) {
    (*(pSVar3->super_Panel).super_IClientPanel._vptr_IClientPanel[0x22])(pSVar3,0);
  }
  pLVar4 = this->m_pPlayerCountLabel_T;
  if (((pLVar4 != (Label *)0x0) && (this->m_pScoreLabel_T != (Label *)0x0)) &&
     (this->m_pPingLabel_T != (Label *)0x0)) {
    uVar6 = CONCAT22((short)((uint)unaff_EBX >> 0x10),0x40ff) & 0xff00ffff;
    (*(pLVar4->super_Panel).super_IClientPanel._vptr_IClientPanel[0x3d])(pLVar4,uVar6 | 0xff400000);
    uVar6 = uVar6 | 0xff4000ff;
    uVar6 = CONCAT22((short)(uVar6 >> 0x10),CONCAT11(0x40,(char)uVar6)) & 0xff00ffff;
    (*(this->m_pScoreLabel_T->super_Panel).super_IClientPanel._vptr_IClientPanel[0x3d])
              (this->m_pScoreLabel_T,uVar6 | 0xff400000);
    uVar6 = uVar6 | 0xff4000ff;
    (*(this->m_pPingLabel_T->super_Panel).super_IClientPanel._vptr_IClientPanel[0x3d])
              (this->m_pPingLabel_T,
               CONCAT22((short)(uVar6 >> 0x10),CONCAT11(0x40,(char)uVar6)) & 0xff00ffff | 0xff400000
              );
  }
  pLVar4 = this->m_pPlayerCountLabel_CT;
  if (((pLVar4 != (Label *)0x0) && (this->m_pScoreLabel_CT != (Label *)0x0)) &&
     (this->m_pPingLabel_CT != (Label *)0x0)) {
    (*(pLVar4->super_Panel).super_IClientPanel._vptr_IClientPanel[0x3d])(pLVar4,0xffffcc99);
    (*(this->m_pScoreLabel_CT->super_Panel).super_IClientPanel._vptr_IClientPanel[0x3d])
              (this->m_pScoreLabel_CT,0xffffcc99);
    (*(this->m_pPingLabel_CT->super_Panel).super_IClientPanel._vptr_IClientPanel[0x3d])
              (this->m_pPingLabel_CT,0xffffcc99);
  }
  (*(this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel.
    _vptr_IClientPanel[0x22])(this,0);
  (*(this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel.
    _vptr_IClientPanel[0xee])(this);
  return;
}


/* CPortalClientScoreBoardDialog::Reset at 006232c0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CClientScoreBoardDialog": ignoring multiple overlapping fields */
/* DWARF original prototype: void Reset(CPortalClientScoreBoardDialog * this) */

void __thiscall CPortalClientScoreBoardDialog::Reset(CPortalClientScoreBoardDialog *this)

{
  InitPlayerList(this,this->m_pPlayerListT,2);
  InitPlayerList(this,this->m_pPlayerListCT,3);
  return;
}


/* CPortalClientScoreBoardDialog::PortalPlayerSortFunc at 00623310 */

bool CPortalClientScoreBoardDialog::PortalPlayerSortFunc
               (SectionedListPanel *list,int itemID1,int itemID2)

{
  bool bVar1;
  KeyValues *this;
  KeyValues *this_00;
  int iVar2;
  int iVar3;
  int unaff_EBX;
  KeyValues *it1;
  KeyValues *it2;
  
                    /* Unresolved local var: int v1@[???]
                       Unresolved local var: int v2@[???]
                       Unresolved local var: int iPlayerIndex1@[???]
                       Unresolved local var: int iPlayerIndex2@[???] */
  ___i686_get_pc_thunk_bx();
  this = (KeyValues *)
         (*(list->super_Panel).super_IClientPanel._vptr_IClientPanel[0xf0])(list,itemID1);
  this_00 = (KeyValues *)
            (*(list->super_Panel).super_IClientPanel._vptr_IClientPanel[0xf0])(list,itemID2);
  iVar2 = KeyValues::GetInt(this,(char *)(unaff_EBX + 0x3a7986),0);
  iVar3 = KeyValues::GetInt(this_00,(char *)(unaff_EBX + 0x3a7986),0);
  if (iVar3 < iVar2) {
LAB_006233e0:
    bVar1 = true;
  }
  else {
    if (iVar3 <= iVar2) {
      iVar2 = KeyValues::GetInt(this,(char *)(unaff_EBX + 0x3a798c),0);
      iVar3 = KeyValues::GetInt(this_00,(char *)(unaff_EBX + 0x3a798c),0);
      if (iVar2 <= iVar3) {
        if (iVar3 <= iVar2) {
          iVar2 = KeyValues::GetInt(this,(char *)(unaff_EBX + 0x38b1f6),0);
          iVar3 = KeyValues::GetInt(this_00,(char *)(unaff_EBX + 0x38b1f6),0);
          return iVar3 < iVar2;
        }
        goto LAB_006233e0;
      }
    }
    bVar1 = false;
  }
  return bVar1;
}


/* CPortalClientScoreBoardDialog::InitPlayerList at 00623040 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CClientScoreBoardDialog": ignoring multiple overlapping fields */
/* DWARF original prototype: void InitPlayerList(CPortalClientScoreBoardDialog * this,
   SectionedListPanel * pPlayerList, int teamNumber) */

void __thiscall
CPortalClientScoreBoardDialog::InitPlayerList
          (CPortalClientScoreBoardDialog *this,SectionedListPanel *pPlayerList,int teamNumber)

{
  undefined *puVar1;
  _func_int_varargs *p_Var2;
  IGameResources *pIVar3;
  undefined4 *puVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(pPlayerList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xfe])(pPlayerList,0);
  (*(pPlayerList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xe0])(pPlayerList);
  (*(pPlayerList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd6])(pPlayerList);
  (*(pPlayerList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd4])
            (pPlayerList,0,unaff_EBX + 0x3a7d49,unaff_EBX + 0x2c2);
  (*(pPlayerList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd9])(pPlayerList,0,1);
  (*(pPlayerList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd7])
            (pPlayerList,0,0xffffffff);
  (*(pPlayerList->super_Panel).super_IClientPanel._vptr_IClientPanel[0x3c])(pPlayerList,0);
  (*(pPlayerList->super_Panel).super_IClientPanel._vptr_IClientPanel[0x45])(pPlayerList,0);
  if (teamNumber != 0) {
    pIVar3 = GameResources();
    if (pIVar3 != (IGameResources *)0x0) {
      p_Var2 = (pPlayerList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd7];
      pIVar3 = GameResources();
      puVar4 = (undefined4 *)(*pIVar3->_vptr_IGameResources[4])(pIVar3,teamNumber);
      (*p_Var2)(pPlayerList,0,*puVar4);
    }
  }
  puVar1 = &UNK_00365c16 + unaff_EBX;
  (*(pPlayerList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xda])
            (pPlayerList,0,unaff_EBX + 0x3a35ba,puVar1,10,
             (this->super_CClientScoreBoardDialog).m_iAvatarWidth,0);
  (*(pPlayerList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xda])
            (pPlayerList,0,unaff_EBX + 0x379706,puVar1,0,
             (this->super_CClientScoreBoardDialog).m_iNameWidth,0);
  (*(pPlayerList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xda])
            (pPlayerList,0,unaff_EBX + 0x3a7c6a,puVar1,0,
             (this->super_CClientScoreBoardDialog).m_iClassWidth,0);
  (*(pPlayerList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xda])
            (pPlayerList,0,unaff_EBX + 0x3a7c5c,puVar1,0x10,
             (this->super_CClientScoreBoardDialog).m_iScoreWidth,0);
  (*(pPlayerList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xda])
            (pPlayerList,0,unaff_EBX + 0x3a7c62,puVar1,0x10,
             (this->super_CClientScoreBoardDialog).m_iDeathWidth,0);
  (*(pPlayerList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xda])
            (pPlayerList,0,&UNK_0038b4c6 + unaff_EBX,puVar1,0x10,
             (this->super_CClientScoreBoardDialog).m_iPingWidth,0);
  return;
}


/* CPortalClientScoreBoardDialog::Update at 00623b50 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CClientScoreBoardDialog": ignoring multiple overlapping fields */
/* DWARF original prototype: void Update(CPortalClientScoreBoardDialog * this) */

void __thiscall CPortalClientScoreBoardDialog::Update(CPortalClientScoreBoardDialog *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel.
    _vptr_IClientPanel[0xfa])(this);
  UpdatePlayerList(this);
  UpdateSpectatorList(this);
  (this->super_CClientScoreBoardDialog).m_fNextUpdateTime =
       *(float *)(unaff_EBX + 0x3be623) + *(float *)(**(int **)(&DAT_0054023b + unaff_EBX) + 0xc);
  return;
}


/* CPortalClientScoreBoardDialog::UpdateTeamInfo at 00622c20 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CClientScoreBoardDialog": ignoring multiple overlapping fields */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void UpdateTeamInfo(CPortalClientScoreBoardDialog * this) */

void __thiscall CPortalClientScoreBoardDialog::UpdateTeamInfo(CPortalClientScoreBoardDialog *this)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  _func_int_varargs *p_Var3;
  undefined *puVar4;
  char cVar5;
  C_Team *pCVar6;
  int iVar7;
  int *piVar8;
  undefined4 uVar9;
  int iVar10;
  int unaff_EBX;
  code *pcVar11;
  wchar_t *local_116c;
  undefined4 *local_1164;
  int local_115c;
  undefined1 *local_1158;
  undefined *local_1150;
  undefined *local_114c;
  undefined *local_1148;
  int local_1144;
  int local_1140;
  undefined1 local_1134 [4096];
  undefined1 local_134 [256];
  wchar_t local_34 [9];
  
  ___i686_get_pc_thunk_bx();
  local_115c = 2;
  puVar1 = *(undefined4 **)(unaff_EBX + 0x54117f);
  puVar2 = *(undefined4 **)(unaff_EBX + 0x5415d7);
  do {
    pCVar6 = GetGlobalTeam(local_115c);
    if (pCVar6 != (C_Team *)0x0) {
      if (local_115c == 2) {
        local_1158 = (undefined1 *)
                     (**(code **)(*(int *)*puVar2 + 0x28))((int *)*puVar2,&UNK_003a80c9 + unaff_EBX)
        ;
        local_1150 = &UNK_003a80d6;
        local_114c = &UNK_003a80e2;
        local_1148 = &UNK_003a80f4;
LAB_00622f55:
        local_1148 = local_1148 + unaff_EBX;
        local_114c = local_114c + unaff_EBX;
        local_1150 = local_1150 + unaff_EBX;
        iVar7 = (*(pCVar6->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.
                  super_IHandleEntity._vptr_IHandleEntity[0xe9])(pCVar6);
        V_snwprintf(local_34,6,(wchar_t *)(unaff_EBX + 0x3c5aaf),iVar7);
        local_1164 = puVar2;
        if (local_1158 == (undefined1 *)0x0) goto LAB_00622d0b;
      }
      else {
        if (local_115c == 3) {
          local_1158 = (undefined1 *)
                       (**(code **)(*(int *)*puVar2 + 0x28))
                                 ((int *)*puVar2,&UNK_003a80ff + unaff_EBX);
          local_1150 = &UNK_003a810d;
          local_114c = &UNK_003a811a;
          local_1148 = &UNK_003a812d;
          goto LAB_00622f55;
        }
        iVar7 = (*(pCVar6->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.
                  super_IHandleEntity._vptr_IHandleEntity[0xe9])(pCVar6);
        V_snwprintf(local_34,6,(wchar_t *)(unaff_EBX + 0x3c5aaf),iVar7);
        local_1150 = (undefined *)0x0;
        local_114c = (undefined *)0x0;
        local_1148 = (undefined *)0x0;
        local_1164 = *(undefined4 **)(unaff_EBX + 0x5415d7);
LAB_00622d0b:
        pcVar11 = *(code **)(*(int *)*local_1164 + 0x30);
        iVar7 = (*(pCVar6->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.
                  super_IHandleEntity._vptr_IHandleEntity[0xe5])(pCVar6);
        local_1158 = local_134;
        (*pcVar11)(*local_1164,iVar7,local_1158,0x100);
      }
      local_116c = local_34;
      iVar7 = (*(pCVar6->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.
                super_IHandleEntity._vptr_IHandleEntity[0xe9])(pCVar6);
      if (iVar7 == 1) {
        piVar8 = (int *)*local_1164;
        iVar7 = *piVar8;
        pcVar11 = *(code **)(iVar7 + 0x3c);
        puVar4 = &UNK_003a8139;
      }
      else {
        piVar8 = (int *)*local_1164;
        iVar7 = *piVar8;
        pcVar11 = *(code **)(iVar7 + 0x3c);
        puVar4 = &UNK_003a814f;
      }
      uVar9 = (**(code **)(iVar7 + 0x28))(piVar8,puVar4 + unaff_EBX);
      (*pcVar11)(*local_1164,local_1134,0x1000,uVar9,2,local_1158,local_116c);
      (*(this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel.
        _vptr_IClientPanel[0xe4])(this,local_114c,local_1134);
      p_Var3 = (this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.
               super_IClientPanel._vptr_IClientPanel[0xe5];
      iVar7 = (*(pCVar6->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.
                super_IHandleEntity._vptr_IHandleEntity[0xe6])(pCVar6);
      (*p_Var3)(this,local_1150,iVar7);
      local_1144 = 0;
      local_1140 = 0;
      iVar7 = 1;
      do {
                    /* Unresolved local var: int ping@[???] */
        while (((cVar5 = (**(code **)(*(int *)*puVar1 + 0x3a0))((int *)*puVar1,iVar7), cVar5 == '\0'
                || (iVar10 = (**(code **)(*(int *)*puVar1 + 0x3c4))((int *)*puVar1,iVar7),
                   iVar10 != local_115c)) ||
               (iVar10 = (**(code **)(*(int *)*puVar1 + 0x3b8))((int *)*puVar1,iVar7), iVar10 < 1)))
        {
                    /* Unresolved local var: int playerIndex@[???] */
          iVar7 = iVar7 + 1;
          if (iVar7 == 0x22) goto LAB_00622eab;
        }
        local_1144 = local_1144 + iVar10;
        local_1140 = local_1140 + 1;
        iVar7 = iVar7 + 1;
      } while (iVar7 != 0x22);
LAB_00622eab:
      if (local_1140 < 1) {
        (*(this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel.
          _vptr_IClientPanel[0xe3])(this,local_1148,unaff_EBX + 0x366033);
      }
      else {
                    /* Unresolved local var: int ping@[???] */
        (*(this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel.
          _vptr_IClientPanel[0xe5])(this,local_1148,(int)((float)local_1144 / (float)local_1140));
      }
    }
    local_115c = local_115c + 1;
    if (local_115c == 4) {
      return;
    }
  } while( true );
}


/* CPortalClientScoreBoardDialog::UpdatePlayerList at 00622a70 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CClientScoreBoardDialog": ignoring multiple overlapping fields */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void UpdatePlayerList(CPortalClientScoreBoardDialog * this) */

void __thiscall CPortalClientScoreBoardDialog::UpdatePlayerList(CPortalClientScoreBoardDialog *this)

{
  code *pcVar1;
  char cVar2;
  C_BasePlayer *pCVar3;
  int iVar4;
  KeyValues *this_00;
  int iVar5;
  undefined4 uVar6;
  undefined4 *puVar7;
  int unaff_EBX;
  int iVar8;
  SectionedListPanel *local_28;
  
                    /* Unresolved local var: C_Portal_Player * pLocalPlayer@[???] */
  ___i686_get_pc_thunk_bx();
  (*(this->m_pPlayerListT->super_Panel).super_IClientPanel._vptr_IClientPanel[0xe0])
            (this->m_pPlayerListT);
  (*(this->m_pPlayerListCT->super_Panel).super_IClientPanel._vptr_IClientPanel[0xe0])
            (this->m_pPlayerListCT);
  if ((**(int **)(unaff_EBX + 0x541332) != 0) &&
     (pCVar3 = C_BasePlayer::GetLocalPlayer(-1), pCVar3 != (C_BasePlayer *)0x0)) {
    iVar8 = 1;
    do {
      while (cVar2 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x541332) + 0x3a0))
                               ((int *)**(undefined4 **)(unaff_EBX + 0x541332),iVar8), cVar2 != '\0'
            ) {
        iVar4 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x541332) + 0x3c4))
                          ((int *)**(undefined4 **)(unaff_EBX + 0x541332),iVar8);
        if (iVar4 == 2) {
          local_28 = this->m_pPlayerListT;
        }
        else {
          if (iVar4 != 3) break;
          local_28 = this->m_pPlayerListCT;
        }
        if (local_28 == (SectionedListPanel *)0x0) break;
        this_00 = KeyValues::operator_new(0x24);
        KeyValues::KeyValues(this_00,(char *)(unaff_EBX + 0x38baa8));
        (*(this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel.
          _vptr_IClientPanel[0xf8])(this,iVar8,this_00);
        iVar5 = (*(local_28->super_Panel).super_IClientPanel._vptr_IClientPanel[0xdd])
                          (local_28,0,this_00);
        iVar4 = *(int *)**(undefined4 **)(unaff_EBX + 0x541332);
        pcVar1 = *(code **)(iVar4 + 0x39c);
        uVar6 = (**(code **)(iVar4 + 0x3c4))((int *)**(undefined4 **)(unaff_EBX + 0x541332),iVar8);
        puVar7 = (undefined4 *)(*pcVar1)(**(undefined4 **)(unaff_EBX + 0x541332),uVar6);
        (*(local_28->super_Panel).super_IClientPanel._vptr_IClientPanel[0xe2])
                  (local_28,iVar5,*puVar7);
        KeyValues::deleteThis(this_00);
        iVar8 = iVar8 + 1;
        if (iVar8 == 0x22) {
          return;
        }
      }
      iVar8 = iVar8 + 1;
    } while (iVar8 != 0x22);
  }
  return;
}


/* CPortalClientScoreBoardDialog::UpdateSpectatorList at 006238a0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CClientScoreBoardDialog": ignoring multiple overlapping fields */
/* DWARF original prototype: void UpdateSpectatorList(CPortalClientScoreBoardDialog * this) */

void __thiscall
CPortalClientScoreBoardDialog::UpdateSpectatorList(CPortalClientScoreBoardDialog *this)

{
  int *piVar1;
  int *piVar2;
  undefined4 *puVar3;
  code *pcVar4;
  undefined *puVar5;
  char cVar6;
  uint uVar7;
  char *pSrc;
  undefined4 uVar8;
  int unaff_EBX;
  int iVar9;
  int nSpectators;
  char *pchFormat;
  undefined1 local_1a60 [4096];
  undefined4 local_a60;
  undefined1 local_a5c [2044];
  wchar_t local_260 [16];
  char local_220;
  undefined1 local_21f [511];
  int local_20;
  
                    /* Unresolved local var: char[512] szSpectatorList@[???]
                       Unresolved local var: wchar_t[512] wzSpectators@[???] */
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(unaff_EBX + 0x5404af);
  local_220 = *(char *)(unaff_EBX + 0x3c4e3b);
  _memset(local_21f,0,0x1ff);
  nSpectators = 0;
  iVar9 = 1;
  piVar1 = *(int **)(unaff_EBX + 0x5404ff);
                    /* Unresolved local var: int playerIndex@[???] */
  do {
    piVar2 = (int *)*piVar1;
    if (((piVar2 != (int *)0x0) &&
        (cVar6 = (**(code **)(*piVar2 + 0x3a0))(piVar2,iVar9), cVar6 != '\0')) &&
       (uVar7 = (**(code **)(*(int *)*piVar1 + 0x3c4))((int *)*piVar1,iVar9), uVar7 < 2)) {
      if (0 < nSpectators) {
        V_strncat(&local_220,&UNK_003a7522 + unaff_EBX,0x200,-1);
      }
      pSrc = (char *)(**(code **)(*(int *)*piVar1 + 0x3b4))((int *)*piVar1,iVar9);
      V_strncat(&local_220,pSrc,0x200,-1);
      nSpectators = nSpectators + 1;
    }
    iVar9 = iVar9 + 1;
  } while (iVar9 != 0x22);
  local_a60 = *(undefined4 *)(unaff_EBX + 0x3c503b);
  _memset(local_a5c,0,0x7fc);
  if (0 < nSpectators) {
    puVar5 = &UNK_003a7525;
    if (nSpectators != 1) {
      puVar5 = &UNK_003a753b;
    }
                    /* Unresolved local var: wchar_t[16] wzSpectatorCount@[???]
                       Unresolved local var: wchar_t[1024] wzSpectatorList@[???] */
    pchFormat = puVar5 + unaff_EBX;
    V_snwprintf(local_260,0x10,(wchar_t *)(unaff_EBX + 0x3c4e2f),nSpectators);
    puVar3 = *(undefined4 **)(unaff_EBX + 0x540957);
    (**(code **)(*(int *)*puVar3 + 0x30))((int *)*puVar3,&local_220,local_1a60,0x1000);
    iVar9 = *(int *)*puVar3;
    pcVar4 = *(code **)(iVar9 + 0x3c);
    uVar8 = (**(code **)(iVar9 + 0x28))((int *)*puVar3,pchFormat);
    (*pcVar4)(*puVar3,&local_a60,0x800,uVar8,2,local_260,local_1a60);
  }
  (*(this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel.
    _vptr_IClientPanel[0xe4])(this,&UNK_003a7552 + unaff_EBX,&local_a60);
  if (local_20 == **(int **)(unaff_EBX + 0x5404af)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}


/* CPortalClientScoreBoardDialog::ShouldShowAsSpectator at 00622580 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CClientScoreBoardDialog": ignoring multiple overlapping fields */
/* DWARF original prototype: bool ShouldShowAsSpectator(CPortalClientScoreBoardDialog * this, int
   iPlayerIndex) */

bool __thiscall
CPortalClientScoreBoardDialog::ShouldShowAsSpectator
          (CPortalClientScoreBoardDialog *this,int iPlayerIndex)

{
  int *piVar1;
  int *piVar2;
  char cVar3;
  uint uVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  piVar1 = *(int **)(unaff_EBX + 0x54181c);
  piVar2 = (int *)*piVar1;
  if ((piVar2 != (int *)0x0) &&
     (cVar3 = (**(code **)(*piVar2 + 0x3a0))(piVar2,iPlayerIndex), cVar3 != '\0')) {
    piVar1 = (int *)*piVar1;
    uVar4 = (**(code **)(*piVar1 + 0x3c4))(piVar1,iPlayerIndex);
    return uVar4 < 2;
  }
  return false;
}


/* CPortalClientScoreBoardDialog::FireGameEvent at 00622930 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CClientScoreBoardDialog": ignoring multiple overlapping fields */
/* DWARF original prototype: void FireGameEvent(CPortalClientScoreBoardDialog * this, IGameEvent *
   event) */

void __thiscall
CPortalClientScoreBoardDialog::FireGameEvent(CPortalClientScoreBoardDialog *this,IGameEvent *event)

{
  undefined4 *puVar1;
  code *pcVar2;
  char *s1;
  int iVar3;
  undefined4 uVar4;
  int unaff_EBX;
  undefined1 local_81c [1024];
  undefined1 local_41c [1036];
  
                    /* Unresolved local var: char * type@[???] */
  ___i686_get_pc_thunk_bx();
  s1 = (char *)(*event->_vptr_IGameEvent[2])(event);
  iVar3 = _V_strcmp(s1,(char *)(unaff_EBX + 0x38bc0f));
  if (iVar3 == 0) {
                    /* Unresolved local var: char * hostname@[???]
                       Unresolved local var: wchar_t[256] wzHostName@[???]
                       Unresolved local var: wchar_t[256] wzServerLabel@[???] */
    iVar3 = (*event->_vptr_IGameEvent[10])(event,unaff_EBX + 0x399f9d,unaff_EBX + 0x36631d);
    puVar1 = *(undefined4 **)(unaff_EBX + 0x5418c1);
    (**(code **)(*(int *)*puVar1 + 0x30))((int *)*puVar1,iVar3,local_41c,0x400);
    iVar3 = *(int *)*puVar1;
    pcVar2 = *(code **)(iVar3 + 0x3c);
    uVar4 = (**(code **)(iVar3 + 0x28))((int *)*puVar1,unaff_EBX + 0x3a83a0);
    (*pcVar2)(*puVar1,local_81c,0x400,uVar4,1,local_41c);
    (*(this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0xe4])(this,unaff_EBX + 0x3723a7,local_81c);
  }
  iVar3 = (*(this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel
            ._vptr_IClientPanel[0x23])(this);
  if ((char)iVar3 != '\0') {
    (*(this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0xef])(this);
  }
  return;
}


/* CPortalClientScoreBoardDialog::GetPlayerScoreInfo at 006225f0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CClientScoreBoardDialog": ignoring multiple overlapping fields */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: bool GetPlayerScoreInfo(CPortalClientScoreBoardDialog * this, int
   playerIndex, KeyValues * kv) */

bool __thiscall
CPortalClientScoreBoardDialog::GetPlayerScoreInfo
          (CPortalClientScoreBoardDialog *this,int playerIndex,KeyValues *kv)

{
  undefined *puVar1;
  int *piVar2;
  code *pcVar3;
  _func_int_varargs **pp_Var4;
  _func_int_varargs *p_Var5;
  int iVar6;
  char cVar7;
  char *pcVar8;
  size_t sVar9;
  undefined4 uVar10;
  int iVar11;
  C_BasePlayer *pCVar12;
  int unaff_EBX;
  int iStack_70;
  int local_68;
  undefined1 auStack_5c [8];
  KeyValues *local_54;
  CPortalClientScoreBoardDialog *local_50;
  undefined1 local_40 [32];
  int local_20;
  
                    /* Unresolved local var: char * oldName@[???]
                       Unresolved local var: int bufsize@[???]
                       Unresolved local var: char * newName@[???]
                       Unresolved local var: C_Portal_Player * me@[???] */
  iStack_70 = 0x6225fe;
  ___i686_get_pc_thunk_bx();
  local_50 = this;
  local_54 = kv;
  local_20 = **(int **)(unaff_EBX + 0x541762);
  local_68 = playerIndex;
  iStack_70 = 0x622631;
  pcVar8 = (char *)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5417b2) + 0x3b4))();
  iStack_70 = 0x62263b;
  sVar9 = _strlen(pcVar8);
  iVar6 = -(sVar9 * 2 + 0x1f & 0xfffffff0);
  *(size_t *)(&stack0xffffff9c + iVar6) = sVar9 * 2 + 1;
  *(undefined1 **)((int)&stack0xffffff98 + iVar6) = auStack_5c + iVar6;
  *(char **)(&stack0xffffff94 + iVar6) = pcVar8;
  *(undefined4 *)((int)&iStack_70 + iVar6) = 0x62265c;
  UTIL_MakeSafeName(*(char **)(&stack0xffffff94 + iVar6),*(char **)((int)&stack0xffffff98 + iVar6),
                    *(int *)(&stack0xffffff9c + iVar6));
  *(undefined1 **)(&stack0xffffff9c + iVar6) = auStack_5c + iVar6;
  *(int *)((int)&stack0xffffff98 + iVar6) = unaff_EBX + 0x37a156;
  *(KeyValues **)(&stack0xffffff94 + iVar6) = local_54;
  *(undefined4 *)((int)&iStack_70 + iVar6) = 0x622675;
  KeyValues::SetString
            (*(KeyValues **)(&stack0xffffff94 + iVar6),*(char **)((int)&stack0xffffff98 + iVar6),
             *(char **)(&stack0xffffff9c + iVar6));
  *(int *)(&stack0xffffff9c + iVar6) = playerIndex;
  *(int *)((int)&stack0xffffff98 + iVar6) = unaff_EBX + 0x38bf1c;
  *(KeyValues **)(&stack0xffffff94 + iVar6) = local_54;
  *(undefined4 *)((int)&iStack_70 + iVar6) = 0x622691;
  KeyValues::SetInt(*(KeyValues **)(&stack0xffffff94 + iVar6),
                    *(char **)((int)&stack0xffffff98 + iVar6),*(int *)(&stack0xffffff9c + iVar6));
  piVar2 = (int *)**(undefined4 **)(unaff_EBX + 0x5417b2);
  iVar11 = *piVar2;
  *(int *)((int)&stack0xffffff98 + iVar6) = playerIndex;
  *(int **)(&stack0xffffff94 + iVar6) = piVar2;
  pcVar3 = *(code **)(iVar11 + 0x3bc);
  *(undefined4 *)((int)&iStack_70 + iVar6) = 0x6226ab;
  uVar10 = (*pcVar3)();
  *(undefined4 *)(&stack0xffffff9c + iVar6) = uVar10;
  *(int *)((int)&stack0xffffff98 + iVar6) = unaff_EBX + 0x3a86ac;
  *(KeyValues **)(&stack0xffffff94 + iVar6) = local_54;
  *(undefined4 *)((int)&iStack_70 + iVar6) = 0x6226c4;
  KeyValues::SetInt(*(KeyValues **)(&stack0xffffff94 + iVar6),
                    *(char **)((int)&stack0xffffff98 + iVar6),*(int *)(&stack0xffffff9c + iVar6));
  piVar2 = (int *)**(undefined4 **)(unaff_EBX + 0x5417b2);
  iVar11 = *piVar2;
  *(int *)((int)&stack0xffffff98 + iVar6) = playerIndex;
  *(int **)(&stack0xffffff94 + iVar6) = piVar2;
  pcVar3 = *(code **)(iVar11 + 0x3c0);
  *(undefined4 *)((int)&iStack_70 + iVar6) = 0x6226de;
  uVar10 = (*pcVar3)();
  *(undefined4 *)(&stack0xffffff9c + iVar6) = uVar10;
  *(undefined **)((int)&stack0xffffff98 + iVar6) = &UNK_003a86b2 + unaff_EBX;
  *(KeyValues **)(&stack0xffffff94 + iVar6) = local_54;
  *(undefined4 *)((int)&iStack_70 + iVar6) = 0x6226f7;
  KeyValues::SetInt(*(KeyValues **)(&stack0xffffff94 + iVar6),
                    *(char **)((int)&stack0xffffff98 + iVar6),*(int *)(&stack0xffffff9c + iVar6));
  *(undefined **)(&stack0xffffff9c + iVar6) = &UNK_00366666 + unaff_EBX;
  puVar1 = &UNK_003a86ba + unaff_EBX;
  *(undefined **)((int)&stack0xffffff98 + iVar6) = puVar1;
  *(KeyValues **)(&stack0xffffff94 + iVar6) = local_54;
  *(undefined4 *)((int)&iStack_70 + iVar6) = 0x622716;
  KeyValues::SetString
            (*(KeyValues **)(&stack0xffffff94 + iVar6),*(char **)((int)&stack0xffffff98 + iVar6),
             *(char **)(&stack0xffffff9c + iVar6));
  pp_Var4 = (local_50->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.
            super_IClientPanel._vptr_IClientPanel;
  *(KeyValues **)(&stack0xffffff9c + iVar6) = local_54;
  *(int *)((int)&stack0xffffff98 + iVar6) = playerIndex;
  *(CPortalClientScoreBoardDialog **)(&stack0xffffff94 + iVar6) = local_50;
  p_Var5 = pp_Var4[0xf6];
  *(undefined4 *)((int)&iStack_70 + iVar6) = 0x622735;
  (*p_Var5)();
  piVar2 = (int *)**(undefined4 **)(unaff_EBX + 0x5417b2);
  iVar11 = *piVar2;
  *(int *)((int)&stack0xffffff98 + iVar6) = playerIndex;
  *(int **)(&stack0xffffff94 + iVar6) = piVar2;
  pcVar3 = *(code **)(iVar11 + 0x3b8);
  *(undefined4 *)((int)&iStack_70 + iVar6) = 0x62274f;
  iVar11 = (*pcVar3)();
  if (iVar11 < 1) {
    piVar2 = (int *)**(undefined4 **)(unaff_EBX + 0x5417b2);
    iVar11 = *piVar2;
    *(int *)((int)&stack0xffffff98 + iVar6) = playerIndex;
    *(int **)(&stack0xffffff94 + iVar6) = piVar2;
    pcVar3 = *(code **)(iVar11 + 0x3a8);
    *(undefined4 *)((int)&iStack_70 + iVar6) = 0x6228ba;
    cVar7 = (*pcVar3)();
    if (cVar7 == '\0') {
      *(undefined **)(&stack0xffffff9c + iVar6) = &UNK_00366666 + unaff_EBX;
      *(int *)((int)&stack0xffffff98 + iVar6) = unaff_EBX + 0x38bf16;
      *(KeyValues **)(&stack0xffffff94 + iVar6) = local_54;
      *(undefined4 *)((int)&iStack_70 + iVar6) = 0x622909;
      KeyValues::SetString
                (*(KeyValues **)(&stack0xffffff94 + iVar6),*(char **)((int)&stack0xffffff98 + iVar6)
                 ,*(char **)(&stack0xffffff9c + iVar6));
    }
    else {
      *(code **)(&stack0xffffff9c + iVar6) = C_TEPhysicsProp::PostDataUpdate + unaff_EBX;
      *(int *)((int)&stack0xffffff98 + iVar6) = unaff_EBX + 0x38bf16;
      *(KeyValues **)(&stack0xffffff94 + iVar6) = local_54;
      *(undefined4 *)((int)&iStack_70 + iVar6) = 0x6228dd;
      KeyValues::SetString
                (*(KeyValues **)(&stack0xffffff94 + iVar6),*(char **)((int)&stack0xffffff98 + iVar6)
                 ,*(char **)(&stack0xffffff9c + iVar6));
    }
  }
  else {
    piVar2 = (int *)**(undefined4 **)(unaff_EBX + 0x5417b2);
    iVar11 = *piVar2;
    *(int *)((int)&stack0xffffff98 + iVar6) = playerIndex;
    *(int **)(&stack0xffffff94 + iVar6) = piVar2;
    pcVar3 = *(code **)(iVar11 + 0x3b8);
    *(undefined4 *)((int)&iStack_70 + iVar6) = 0x622771;
    uVar10 = (*pcVar3)();
    *(undefined4 *)(&stack0xffffff9c + iVar6) = uVar10;
    *(int *)((int)&stack0xffffff98 + iVar6) = unaff_EBX + 0x38bf16;
    *(KeyValues **)(&stack0xffffff94 + iVar6) = local_54;
    *(undefined4 *)((int)&iStack_70 + iVar6) = 0x62278a;
    KeyValues::SetInt(*(KeyValues **)(&stack0xffffff94 + iVar6),
                      *(char **)((int)&stack0xffffff98 + iVar6),*(int *)(&stack0xffffff9c + iVar6));
  }
  *(undefined4 *)(&stack0xffffff94 + iVar6) = 0xffffffff;
  *(undefined4 *)((int)&iStack_70 + iVar6) = 0x622796;
  pCVar12 = C_BasePlayer::GetLocalPlayer(*(int *)(&stack0xffffff94 + iVar6));
  piVar2 = (int *)**(int **)(unaff_EBX + 0x5417b2);
  if ((piVar2 != (int *)0x0) && (pCVar12 != (C_BasePlayer *)0x0)) {
    iVar11 = *piVar2;
    *(int *)((int)&stack0xffffff98 + iVar6) = playerIndex;
    *(int **)(&stack0xffffff94 + iVar6) = piVar2;
    pcVar3 = *(code **)(iVar11 + 0x3b0);
    *(undefined4 *)((int)&iStack_70 + iVar6) = 0x6227e2;
    cVar7 = (*pcVar3)();
    if (cVar7 == '\0') {
      piVar2 = (int *)**(undefined4 **)(unaff_EBX + 0x5417b2);
      iVar11 = *piVar2;
      *(int *)((int)&stack0xffffff98 + iVar6) = playerIndex;
      *(int **)(&stack0xffffff94 + iVar6) = piVar2;
      pcVar3 = *(code **)(iVar11 + 0x3a4);
      *(undefined4 *)((int)&iStack_70 + iVar6) = 0x62284a;
      cVar7 = (*pcVar3)();
      if (cVar7 == '\0') {
        piVar2 = (int *)**(undefined4 **)(unaff_EBX + 0x5417b2);
        iVar11 = *piVar2;
        *(int *)((int)&stack0xffffff98 + iVar6) = playerIndex;
        *(int **)(&stack0xffffff94 + iVar6) = piVar2;
        pcVar3 = *(code **)(iVar11 + 0x3c4);
        *(undefined4 *)((int)&iStack_70 + iVar6) = 0x62286c;
        iVar11 = (*pcVar3)();
        if (1 < iVar11) {
          *(int *)(&stack0xffffff9c + iVar6) = unaff_EBX + 0x3a86d3;
          *(undefined **)((int)&stack0xffffff98 + iVar6) = puVar1;
          *(KeyValues **)(&stack0xffffff94 + iVar6) = local_54;
          *(undefined4 *)((int)&iStack_70 + iVar6) = 0x62288c;
          KeyValues::SetString
                    (*(KeyValues **)(&stack0xffffff94 + iVar6),
                     *(char **)((int)&stack0xffffff98 + iVar6),*(char **)(&stack0xffffff9c + iVar6))
          ;
        }
      }
    }
    else {
                    /* Unresolved local var: char[32] numspecs@[???] */
      *(int *)(&stack0xffffffa0 + iVar6) =
           (local_50->super_CClientScoreBoardDialog).m_HLTVSpectators;
      *(int *)(&stack0xffffff9c + iVar6) = unaff_EBX + 0x3a86c5;
      *(undefined4 *)((int)&stack0xffffff98 + iVar6) = 0x20;
      *(undefined1 **)(&stack0xffffff94 + iVar6) = local_40;
      *(undefined4 *)((int)&iStack_70 + iVar6) = 0x622811;
      V_snprintf(*(char **)(&stack0xffffff94 + iVar6),*(int *)((int)&stack0xffffff98 + iVar6),
                 *(char **)(&stack0xffffff9c + iVar6));
      *(undefined1 **)(&stack0xffffff9c + iVar6) = local_40;
      *(undefined **)((int)&stack0xffffff98 + iVar6) = puVar1;
      *(KeyValues **)(&stack0xffffff94 + iVar6) = local_54;
      *(undefined4 *)((int)&iStack_70 + iVar6) = 0x622824;
      KeyValues::SetString
                (*(KeyValues **)(&stack0xffffff94 + iVar6),*(char **)((int)&stack0xffffff98 + iVar6)
                 ,*(char **)(&stack0xffffff9c + iVar6));
    }
  }
  if (local_20 == **(int **)(unaff_EBX + 0x541762)) {
    return true;
  }
                    /* WARNING: Subroutine does not return */
  *(undefined **)((int)&iStack_70 + iVar6) = &UNK_00622913;
  ___stack_chk_fail();
}


/* __static_initialization_and_destruction_0 at 000813b0 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  int unaff_EBX;
  longlong lVar1;
  
  lVar1 = ___i686_get_pc_thunk_bx();
  if (lVar1 != 0xffff00000001) {
    return;
  }
  (&DAT_00c580ef)[unaff_EBX] = 0;
  (&DAT_00c580f0)[unaff_EBX] = 0;
  (&DAT_00c580f1)[unaff_EBX] = 0;
  (&DAT_00c580f2)[unaff_EBX] = 0;
  *(undefined4 *)(&DAT_00c580f3 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c580f7 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c580fb + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c580ff + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&vgui::Label::GetKBMap()::s_pMap + unaff_EBX + 3) = 0;
  *(undefined4 *)(&DAT_00c58107 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c5810b + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c5810f + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c58113 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c58117 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c5811b + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c5811f + unaff_EBX) = 0x7f7fffff;
  *(undefined **)((int)&CAchievementNotificationPanel::GetKBMap()::s_pMap + unaff_EBX + 3) =
       &UNK_00ae3acf + unaff_EBX;
  return;
}


/* _GLOBAL__I__ZN29CPortalClientScoreBoardDialogC2EP9IViewPort at 00081450 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN29CPortalClientScoreBoardDialogC2EP9IViewPort(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

