/* DWARF-guided pseudocode for game/client/portal2/vgui/portalclientscoreboard.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* CPortalClientScoreBoardDialog::CPortalClientScoreBoardDialog at 006179a0 */

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
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a6fda);
  (this->super_CClientScoreBoardDialog).super_IViewPortPanel._vptr_IViewPortPanel =
       (_func_int_varargs **)(&UNK_005a73e2 + unaff_EBX);
  (this->super_CClientScoreBoardDialog).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(&UNK_005a742a + unaff_EBX);
  if (*(char *)(unaff_EBX + 0x60d4ca) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x60d4ca) = 1;
    pPVar6 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x388129));
    pPVar6->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x5e01ba);
    pPVar7 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x36c106));
    pPVar6->baseMap = pPVar7;
    cVar1 = *(char *)(unaff_EBX + 0x60d4cb);
  }
  else {
    cVar1 = *(char *)(unaff_EBX + 0x60d4cb);
  }
  if (cVar1 == '\0') {
    *(undefined1 *)(unaff_EBX + 0x60d4cb) = 1;
    pPVar8 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x388129));
    pPVar8->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x5e01ba);
    pPVar9 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x36c106));
    pPVar8->baseMap = pPVar9;
  }
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
  if (*(char *)(unaff_EBX + 0x60d4cc) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x60d4cc) = 1;
    pPVar2 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x388129));
    pPVar2->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x5e01ba);
    pPVar3 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x36c106));
    pPVar2->baseMap = pPVar3;
  }
  *(undefined4 *)(this->m_bgColor)._color = 0;
  *(undefined4 *)(this->m_borderColor)._color = 0;
  pSVar4 = operator_new(0x1fc);
  vgui::SectionedListPanel::SectionedListPanel
            (pSVar4,(Panel *)this,(char *)(unaff_EBX + 0x3882b4),in_stack_ffffffd0,
             &in_stack_ffffffd4->super_Panel,(char *)in_stack_ffffffd8);
  this->m_pPlayerListT = pSVar4;
  pSVar4 = operator_new(0x1fc);
  vgui::SectionedListPanel::SectionedListPanel
            (pSVar4,(Panel *)this,(char *)(unaff_EBX + 0x3882c0),in_stack_ffffffd0,
             &in_stack_ffffffd4->super_Panel,(char *)in_stack_ffffffd8);
  this->m_pPlayerListCT = pSVar4;
  pLVar5 = operator_new(0x1a4);
  text = (char *)(unaff_EBX + 0x3468be);
  vgui::Label::Label(pLVar5,(Panel *)this,(char *)(unaff_EBX + 0x3882cd),text,in_stack_ffffffd4,
                     in_stack_ffffffd8,in_stack_ffffffdc,text);
  this->m_pPlayerCountLabel_T = pLVar5;
  pLVar5 = operator_new(0x1a4);
  vgui::Label::Label(pLVar5,(Panel *)this,(char *)(unaff_EBX + 0x3882db),text,in_stack_ffffffd4,
                     in_stack_ffffffd8,in_stack_ffffffdc,text);
  this->m_pScoreLabel_T = pLVar5;
  pLVar5 = operator_new(0x1a4);
  vgui::Label::Label(pLVar5,(Panel *)this,(char *)(unaff_EBX + 0x3882e3),text,in_stack_ffffffd4,
                     in_stack_ffffffd8,in_stack_ffffffdc,text);
  this->m_pPingLabel_T = pLVar5;
  pLVar5 = operator_new(0x1a4);
  vgui::Label::Label(pLVar5,(Panel *)this,(char *)(unaff_EBX + 0x3882ed),text,in_stack_ffffffd4,
                     in_stack_ffffffd8,in_stack_ffffffdc,text);
  this->m_pPlayerCountLabel_CT = pLVar5;
  pLVar5 = operator_new(0x1a4);
  vgui::Label::Label(pLVar5,(Panel *)this,(char *)(unaff_EBX + 0x3882fc),text,in_stack_ffffffd4,
                     in_stack_ffffffd8,in_stack_ffffffdc,text);
  this->m_pScoreLabel_CT = pLVar5;
  pLVar5 = operator_new(0x1a4);
  vgui::Label::Label(pLVar5,(Panel *)this,(char *)(unaff_EBX + 0x388305),text,in_stack_ffffffd4,
                     in_stack_ffffffd8,in_stack_ffffffdc,text);
  this->m_pPingLabel_CT = pLVar5;
                    /* Unresolved local var: bool bServerSide@[???] */
  (this->super_CClientScoreBoardDialog).super_CGameEventListener.m_bRegisteredForEvents = true;
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x519382) + 0x10))
            ((int *)**(undefined4 **)(unaff_EBX + 0x519382),
             &(this->super_CClientScoreBoardDialog).super_CGameEventListener,unaff_EBX + 0x36c1cc,0)
  ;
  vgui::EditablePanel::SetDialogVariable((EditablePanel *)this,(char *)(unaff_EBX + 0x35291c),text);
  vgui::Panel::SetVisible((Panel *)this,false);
  return;
}


/* CPortalClientScoreBoardDialog::CPortalClientScoreBoardDialog at 00617cf0 */

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


/* CPortalClientScoreBoardDialog::~CPortalClientScoreBoardDialog at 00616f30 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CClientScoreBoardDialog": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* WARNING: Struct "BaseModHybridButton": ignoring multiple overlapping fields */
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
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a7a47);
  (this->super_CClientScoreBoardDialog).super_IViewPortPanel._vptr_IViewPortPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5a7e4f);
  (this->super_CClientScoreBoardDialog).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(unaff_EBX + 0x5a7e97);
  CClientScoreBoardDialog::~CClientScoreBoardDialog
            (&this->super_CClientScoreBoardDialog,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CPortalClientScoreBoardDialog::~CPortalClientScoreBoardDialog at 00616fb0 */

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
  _vptr_IClientPanel = (_func_int_varargs **)(extraout_ECX + 0x5a79d0);
  (this->super_CClientScoreBoardDialog).super_IViewPortPanel._vptr_IViewPortPanel =
       (_func_int_varargs **)(extraout_ECX + 0x5a7dd8);
  (this->super_CClientScoreBoardDialog).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(extraout_ECX + 0x5a7e20);
  CClientScoreBoardDialog::~CClientScoreBoardDialog(&this->super_CClientScoreBoardDialog,__in_chrg);
  return;
}


/* CPortalClientScoreBoardDialog::~CPortalClientScoreBoardDialog at 00617000 */

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
  _vptr_IClientPanel =
       (_func_int_varargs **)(BaseModUI::Downloads::ChainToAnimationMap + extraout_ECX);
  (this->super_CClientScoreBoardDialog).super_IViewPortPanel._vptr_IViewPortPanel =
       (_func_int_varargs **)(&UNK_005a7d88 + extraout_ECX);
  (this->super_CClientScoreBoardDialog).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(extraout_ECX + 0x5a7dd0);
  CClientScoreBoardDialog::~CClientScoreBoardDialog(&this->super_CClientScoreBoardDialog,__in_chrg);
  return;
}


/* CPortalClientScoreBoardDialog::PaintBackground at 00616f00 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CClientScoreBoardDialog": ignoring multiple overlapping fields */
/* DWARF original prototype: void PaintBackground(CPortalClientScoreBoardDialog * this) */

void __thiscall CPortalClientScoreBoardDialog::PaintBackground(CPortalClientScoreBoardDialog *this)

{
  vgui::EditablePanel::PaintBackground((EditablePanel *)this);
  return;
}


/* CPortalClientScoreBoardDialog::PaintBorder at 00616ef0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CClientScoreBoardDialog": ignoring multiple overlapping fields */
/* DWARF original prototype: void PaintBorder(CPortalClientScoreBoardDialog * this) */

void __thiscall CPortalClientScoreBoardDialog::PaintBorder(CPortalClientScoreBoardDialog *this)

{
  vgui::Panel::PaintBorder((Panel *)this);
  return;
}


/* CPortalClientScoreBoardDialog::ApplySchemeSettings at 00617050 */

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
    _vptr_IClientPanel[0xd4])(this,unaff_EBX + 0x388b72,0,0,0);
  pp_Var1 = (this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel
            ._vptr_IClientPanel;
  p_Var2 = pp_Var1[0x58];
  iVar5 = (*pp_Var1[0x3e])(this);
  iVar5 = (*p_Var2)(this,unaff_EBX + 0x39e410,iVar5,pScheme);
  *(int *)(this->m_bgColor)._color = iVar5;
  iVar5 = (*(pScheme->super_IBaseInterface)._vptr_IBaseInterface[6])(pScheme,unaff_EBX + 0x388b90,0)
  ;
  *(int *)(this->m_borderColor)._color = iVar5;
  (*(this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel.
    _vptr_IClientPanel[0x3c])(this,0xa0282828);
  p_Var2 = (this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel.
           _vptr_IClientPanel[0x45];
  iVar5 = (*(pScheme->super_IBaseInterface)._vptr_IBaseInterface[3])(pScheme,unaff_EBX + 0x388b98);
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


/* CPortalClientScoreBoardDialog::Reset at 00616d80 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CClientScoreBoardDialog": ignoring multiple overlapping fields */
/* DWARF original prototype: void Reset(CPortalClientScoreBoardDialog * this) */

void __thiscall CPortalClientScoreBoardDialog::Reset(CPortalClientScoreBoardDialog *this)

{
  InitPlayerList(this,this->m_pPlayerListT,2);
  InitPlayerList(this,this->m_pPlayerListCT,3);
  return;
}


/* CPortalClientScoreBoardDialog::PortalPlayerSortFunc at 00616dd0 */

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
  iVar2 = KeyValues::GetInt(this,(char *)(unaff_EBX + 0x388dd2),0);
  iVar3 = KeyValues::GetInt(this_00,(char *)(unaff_EBX + 0x388dd2),0);
  if (iVar3 < iVar2) {
LAB_00616ea0:
    bVar1 = true;
  }
  else {
    if (iVar3 <= iVar2) {
      iVar2 = KeyValues::GetInt(this,(char *)(unaff_EBX + 0x388dd8),0);
      iVar3 = KeyValues::GetInt(this_00,(char *)(unaff_EBX + 0x388dd8),0);
      if (iVar2 <= iVar3) {
        if (iVar3 <= iVar2) {
          iVar2 = KeyValues::GetInt(this,(char *)(unaff_EBX + 0x388de0),0);
          iVar3 = KeyValues::GetInt(this_00,(char *)(unaff_EBX + 0x388de0),0);
          return iVar3 < iVar2;
        }
        goto LAB_00616ea0;
      }
    }
    bVar1 = false;
  }
  return bVar1;
}


/* CPortalClientScoreBoardDialog::InitPlayerList at 00616b00 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CClientScoreBoardDialog": ignoring multiple overlapping fields */
/* DWARF original prototype: void InitPlayerList(CPortalClientScoreBoardDialog * this,
   SectionedListPanel * pPlayerList, int teamNumber) */

void __thiscall
CPortalClientScoreBoardDialog::InitPlayerList
          (CPortalClientScoreBoardDialog *this,SectionedListPanel *pPlayerList,int teamNumber)

{
  int iVar1;
  _func_int_varargs *p_Var2;
  IGameResources *pIVar3;
  undefined4 *puVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(pPlayerList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xfe])(pPlayerList,0);
  (*(pPlayerList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xe0])(pPlayerList);
  (*(pPlayerList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd6])(pPlayerList);
  (*(pPlayerList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd4])
            (pPlayerList,0,&UNK_0038909a + unaff_EBX,unaff_EBX + 0x2c2);
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
  iVar1 = unaff_EBX + 0x34775e;
  (*(pPlayerList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xda])
            (pPlayerList,0,unaff_EBX + 0x384dc2,iVar1,10,
             (this->super_CClientScoreBoardDialog).m_iAvatarWidth,0);
  (*(pPlayerList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xda])
            (pPlayerList,0,unaff_EBX + 0x35b24e,iVar1,0,
             (this->super_CClientScoreBoardDialog).m_iNameWidth,0);
  (*(pPlayerList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xda])
            (pPlayerList,0,unaff_EBX + 0x3890a2,iVar1,0,
             (this->super_CClientScoreBoardDialog).m_iClassWidth,0);
  (*(pPlayerList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xda])
            (pPlayerList,0,unaff_EBX + 0x3890a8,iVar1,0x10,
             (this->super_CClientScoreBoardDialog).m_iScoreWidth,0);
  (*(pPlayerList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xda])
            (pPlayerList,0,unaff_EBX + 0x3890ae,iVar1,0x10,
             (this->super_CClientScoreBoardDialog).m_iDeathWidth,0);
  (*(pPlayerList->super_Panel).super_IClientPanel._vptr_IClientPanel[0xda])
            (pPlayerList,0,unaff_EBX + 0x36d036,iVar1,0x10,
             (this->super_CClientScoreBoardDialog).m_iPingWidth,0);
  return;
}


/* CPortalClientScoreBoardDialog::Update at 00617940 */

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
       *(float *)(unaff_EBX + 0x39eef3) + *(float *)(**(int **)(unaff_EBX + 0x5193ff) + 0xc);
  return;
}


/* CPortalClientScoreBoardDialog::UpdateTeamInfo at 006166e0 */

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
  char cVar4;
  C_Team *pCVar5;
  int iVar6;
  int *piVar7;
  undefined4 uVar8;
  int iVar9;
  undefined *puVar10;
  int unaff_EBX;
  code *pcVar11;
  wchar_t *local_116c;
  undefined4 *local_1164;
  int local_115c;
  undefined1 *local_1158;
  int local_1150;
  int local_114c;
  undefined *local_1148;
  int local_1144;
  int local_1140;
  undefined1 local_1134 [4096];
  undefined1 local_134 [256];
  wchar_t local_34 [9];
  
  ___i686_get_pc_thunk_bx();
  local_115c = 2;
  puVar1 = *(undefined4 **)(unaff_EBX + 0x51a673);
  puVar2 = *(undefined4 **)(unaff_EBX + 0x51aac7);
  do {
    pCVar5 = GetGlobalTeam(local_115c);
    if (pCVar5 != (C_Team *)0x0) {
      if (local_115c == 2) {
        local_1158 = (undefined1 *)
                     (**(code **)(*(int *)*puVar2 + 0x28))((int *)*puVar2,&UNK_0038941a + unaff_EBX)
        ;
        local_1150 = unaff_EBX + 0x389427;
        local_114c = unaff_EBX + 0x389433;
        local_1148 = (undefined *)(unaff_EBX + 0x389445);
LAB_00616a15:
        iVar6 = (*(pCVar5->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.
                  super_IHandleEntity._vptr_IHandleEntity[0xe8])(pCVar5);
        V_snwprintf(local_34,6,(wchar_t *)(C_TEMuzzleFlash::~C_TEMuzzleFlash + unaff_EBX + 7),iVar6)
        ;
        local_1164 = puVar2;
        if (local_1158 == (undefined1 *)0x0) goto LAB_006167cb;
      }
      else {
        if (local_115c == 3) {
          local_1158 = (undefined1 *)
                       (**(code **)(*(int *)*puVar2 + 0x28))((int *)*puVar2,unaff_EBX + 0x389450);
          local_1150 = unaff_EBX + 0x38945e;
          local_114c = unaff_EBX + 0x38946b;
          local_1148 = &UNK_0038947e + unaff_EBX;
          goto LAB_00616a15;
        }
        iVar6 = (*(pCVar5->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.
                  super_IHandleEntity._vptr_IHandleEntity[0xe8])(pCVar5);
        V_snwprintf(local_34,6,(wchar_t *)(C_TEMuzzleFlash::~C_TEMuzzleFlash + unaff_EBX + 7),iVar6)
        ;
        local_1150 = 0;
        local_114c = 0;
        local_1148 = (undefined *)0x0;
        local_1164 = *(undefined4 **)(unaff_EBX + 0x51aac7);
LAB_006167cb:
        pcVar11 = *(code **)(*(int *)*local_1164 + 0x30);
        iVar6 = (*(pCVar5->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.
                  super_IHandleEntity._vptr_IHandleEntity[0xe4])(pCVar5);
        local_1158 = local_134;
        (*pcVar11)(*local_1164,iVar6,local_1158,0x100);
      }
      local_116c = local_34;
      iVar6 = (*(pCVar5->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.
                super_IHandleEntity._vptr_IHandleEntity[0xe8])(pCVar5);
      if (iVar6 == 1) {
        piVar7 = (int *)*local_1164;
        iVar6 = *piVar7;
        pcVar11 = *(code **)(iVar6 + 0x3c);
        puVar10 = &UNK_0038948a + unaff_EBX;
      }
      else {
        piVar7 = (int *)*local_1164;
        iVar6 = *piVar7;
        pcVar11 = *(code **)(iVar6 + 0x3c);
        puVar10 = (undefined *)(unaff_EBX + 0x3894a0);
      }
      uVar8 = (**(code **)(iVar6 + 0x28))(piVar7,puVar10);
      (*pcVar11)(*local_1164,local_1134,0x1000,uVar8,2,local_1158,local_116c);
      (*(this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel.
        _vptr_IClientPanel[0xe4])(this,local_114c,local_1134);
      p_Var3 = (this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.
               super_IClientPanel._vptr_IClientPanel[0xe5];
      iVar6 = (*(pCVar5->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.
                super_IHandleEntity._vptr_IHandleEntity[0xe5])(pCVar5);
      (*p_Var3)(this,local_1150,iVar6);
      local_1144 = 0;
      local_1140 = 0;
      iVar6 = 1;
      do {
                    /* Unresolved local var: int ping@[???] */
        while (((cVar4 = (**(code **)(*(int *)*puVar1 + 0x39c))((int *)*puVar1,iVar6), cVar4 == '\0'
                || (iVar9 = (**(code **)(*(int *)*puVar1 + 0x3c0))((int *)*puVar1,iVar6),
                   iVar9 != local_115c)) ||
               (iVar9 = (**(code **)(*(int *)*puVar1 + 0x3b4))((int *)*puVar1,iVar6), iVar9 < 1))) {
                    /* Unresolved local var: int playerIndex@[???] */
          iVar6 = iVar6 + 1;
          if (iVar6 == 0x22) goto LAB_0061696b;
        }
        local_1144 = local_1144 + iVar9;
        local_1140 = local_1140 + 1;
        iVar6 = iVar6 + 1;
      } while (iVar6 != 0x22);
LAB_0061696b:
      if (local_1140 < 1) {
        (*(this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel.
          _vptr_IClientPanel[0xe3])(this,local_1148,&UNK_00347b7b + unaff_EBX);
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


/* CPortalClientScoreBoardDialog::UpdatePlayerList at 00616530 */

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
  if ((**(int **)(unaff_EBX + 0x51a826) != 0) &&
     (pCVar3 = C_BasePlayer::GetLocalPlayer(-1), pCVar3 != (C_BasePlayer *)0x0)) {
    iVar8 = 1;
    do {
      while (cVar2 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51a826) + 0x39c))
                               ((int *)**(undefined4 **)(unaff_EBX + 0x51a826),iVar8), cVar2 != '\0'
            ) {
        iVar4 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51a826) + 0x3c0))
                          ((int *)**(undefined4 **)(unaff_EBX + 0x51a826),iVar8);
        if (iVar4 == 2) {
          local_28 = this->m_pPlayerListT;
        }
        else {
          if (iVar4 != 3) break;
          local_28 = this->m_pPlayerListCT;
        }
        if (local_28 == (SectionedListPanel *)0x0) break;
        this_00 = KeyValues::operator_new(0x24);
        KeyValues::KeyValues(this_00,(char *)(unaff_EBX + 0x36d60c));
        (*(this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel.
          _vptr_IClientPanel[0xf8])(this,iVar8,this_00);
        iVar5 = (*(local_28->super_Panel).super_IClientPanel._vptr_IClientPanel[0xdd])
                          (local_28,0,this_00);
        iVar4 = *(int *)**(undefined4 **)(unaff_EBX + 0x51a826);
        pcVar1 = *(code **)(iVar4 + 0x398);
        uVar6 = (**(code **)(iVar4 + 0x3c0))((int *)**(undefined4 **)(unaff_EBX + 0x51a826),iVar8);
        puVar7 = (undefined4 *)(*pcVar1)(**(undefined4 **)(unaff_EBX + 0x51a826),uVar6);
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


/* CPortalClientScoreBoardDialog::UpdateSpectatorList at 00617690 */

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
  char cVar5;
  uint uVar6;
  char *pSrc;
  undefined4 uVar7;
  int unaff_EBX;
  int iVar8;
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
  local_20 = **(int **)(&DAT_00519673 + unaff_EBX);
  local_220 = *(char *)(unaff_EBX + 0x3a5533);
  _memset(local_21f,0,0x1ff);
  nSpectators = 0;
  iVar8 = 1;
  piVar1 = *(int **)(unaff_EBX + 0x5196c3);
                    /* Unresolved local var: int playerIndex@[???] */
  do {
    piVar2 = (int *)*piVar1;
    if (((piVar2 != (int *)0x0) &&
        (cVar5 = (**(code **)(*piVar2 + 0x39c))(piVar2,iVar8), cVar5 != '\0')) &&
       (uVar6 = (**(code **)(*(int *)*piVar1 + 0x3c0))((int *)*piVar1,iVar8), uVar6 < 2)) {
      if (0 < nSpectators) {
        V_strncat(&local_220,(char *)(unaff_EBX + 0x388586),0x200,-1);
      }
      pSrc = (char *)(**(code **)(*(int *)*piVar1 + 0x3b0))((int *)*piVar1,iVar8);
      V_strncat(&local_220,pSrc,0x200,-1);
      nSpectators = nSpectators + 1;
    }
    iVar8 = iVar8 + 1;
  } while (iVar8 != 0x22);
  local_a60 = *(undefined4 *)(unaff_EBX + 0x3a5733);
  _memset(local_a5c,0,0x7fc);
  if (0 < nSpectators) {
                    /* Unresolved local var: wchar_t[16] wzSpectatorCount@[???]
                       Unresolved local var: wchar_t[1024] wzSpectatorList@[???] */
    pchFormat = (char *)(unaff_EBX + 0x388589);
    if (nSpectators != 1) {
      pchFormat = (char *)(unaff_EBX + 0x38859f);
    }
    V_snwprintf(local_260,0x10,(wchar_t *)(unaff_EBX + 0x3a5527),nSpectators);
    puVar3 = *(undefined4 **)(unaff_EBX + 0x519b17);
    (**(code **)(*(int *)*puVar3 + 0x30))((int *)*puVar3,&local_220,local_1a60,0x1000);
    iVar8 = *(int *)*puVar3;
    pcVar4 = *(code **)(iVar8 + 0x3c);
    uVar7 = (**(code **)(iVar8 + 0x28))((int *)*puVar3,pchFormat);
    (*pcVar4)(*puVar3,&local_a60,0x800,uVar7,2,local_260,local_1a60);
  }
  (*(this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel.
    _vptr_IClientPanel[0xe4])(this,unaff_EBX + 0x3885b6,&local_a60);
  if (local_20 == **(int **)(&DAT_00519673 + unaff_EBX)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}


/* CPortalClientScoreBoardDialog::ShouldShowAsSpectator at 00616370 */

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
  piVar1 = *(int **)(unaff_EBX + 0x51a9e0);
  piVar2 = (int *)*piVar1;
  if ((piVar2 != (int *)0x0) &&
     (cVar3 = (**(code **)(*piVar2 + 0x39c))(piVar2,iPlayerIndex), cVar3 != '\0')) {
    piVar1 = (int *)*piVar1;
    uVar4 = (**(code **)(*piVar1 + 0x3c0))(piVar1,iPlayerIndex);
    return uVar4 < 2;
  }
  return false;
}


/* CPortalClientScoreBoardDialog::FireGameEvent at 006163f0 */

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
  iVar3 = _V_strcmp(s1,&UNK_0036d773 + unaff_EBX);
  if (iVar3 == 0) {
                    /* Unresolved local var: char * hostname@[???]
                       Unresolved local var: wchar_t[256] wzHostName@[???]
                       Unresolved local var: wchar_t[256] wzServerLabel@[???] */
    iVar3 = (*event->_vptr_IGameEvent[10])(event,unaff_EBX + 0x37b665,unaff_EBX + 0x347e65);
    puVar1 = *(undefined4 **)(unaff_EBX + 0x51adb1);
    (**(code **)(*(int *)*puVar1 + 0x30))((int *)*puVar1,iVar3,local_41c,0x400);
    iVar3 = *(int *)*puVar1;
    pcVar2 = *(code **)(iVar3 + 0x3c);
    uVar4 = (**(code **)(iVar3 + 0x28))((int *)*puVar1,unaff_EBX + 0x3896f1);
    (*pcVar2)(*puVar1,local_81c,0x400,uVar4,1,local_41c);
    (*(this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0xe4])(this,unaff_EBX + 0x353ec3,local_81c);
  }
  iVar3 = (*(this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel
            ._vptr_IClientPanel[0x23])(this);
  if ((char)iVar3 != '\0') {
    (*(this->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.super_IClientPanel.
      _vptr_IClientPanel[0xef])(this);
  }
  return;
}


/* CPortalClientScoreBoardDialog::GetPlayerScoreInfo at 00617360 */

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
  int *piVar1;
  int iVar2;
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
  iStack_70 = 0x61736e;
  ___i686_get_pc_thunk_bx();
  local_50 = this;
  local_54 = kv;
  local_20 = **(int **)(unaff_EBX + 0x5199a6);
  local_68 = playerIndex;
  iStack_70 = 0x6173a1;
  pcVar8 = (char *)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5199f6) + 0x3b0))();
  iStack_70 = 0x6173ab;
  sVar9 = _strlen(pcVar8);
  iVar6 = -(sVar9 * 2 + 0x1f & 0xfffffff0);
  *(size_t *)(&stack0xffffff9c + iVar6) = sVar9 * 2 + 1;
  *(undefined1 **)((int)&stack0xffffff98 + iVar6) = auStack_5c + iVar6;
  *(char **)(&stack0xffffff94 + iVar6) = pcVar8;
  *(undefined4 *)((int)&iStack_70 + iVar6) = 0x6173cc;
  UTIL_MakeSafeName(*(char **)(&stack0xffffff94 + iVar6),*(char **)((int)&stack0xffffff98 + iVar6),
                    *(int *)(&stack0xffffff9c + iVar6));
  *(undefined1 **)(&stack0xffffff9c + iVar6) = auStack_5c + iVar6;
  *(int *)((int)&stack0xffffff98 + iVar6) = unaff_EBX + 0x35a9ee;
  *(KeyValues **)(&stack0xffffff94 + iVar6) = local_54;
  *(undefined4 *)((int)&iStack_70 + iVar6) = 0x6173e5;
  KeyValues::SetString
            (*(KeyValues **)(&stack0xffffff94 + iVar6),*(char **)((int)&stack0xffffff98 + iVar6),
             *(char **)(&stack0xffffff9c + iVar6));
  *(int *)(&stack0xffffff9c + iVar6) = playerIndex;
  *(int *)((int)&stack0xffffff98 + iVar6) = unaff_EBX + 0x388856;
  *(KeyValues **)(&stack0xffffff94 + iVar6) = local_54;
  *(undefined4 *)((int)&iStack_70 + iVar6) = 0x617401;
  KeyValues::SetInt(*(KeyValues **)(&stack0xffffff94 + iVar6),
                    *(char **)((int)&stack0xffffff98 + iVar6),*(int *)(&stack0xffffff9c + iVar6));
  piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x5199f6);
  iVar2 = *piVar1;
  *(int *)((int)&stack0xffffff98 + iVar6) = playerIndex;
  *(int **)(&stack0xffffff94 + iVar6) = piVar1;
  pcVar3 = *(code **)(iVar2 + 0x3b8);
  *(undefined4 *)((int)&iStack_70 + iVar6) = 0x61741b;
  uVar10 = (*pcVar3)();
  *(undefined4 *)(&stack0xffffff9c + iVar6) = uVar10;
  *(int *)((int)&stack0xffffff98 + iVar6) = unaff_EBX + 0x388848;
  *(KeyValues **)(&stack0xffffff94 + iVar6) = local_54;
  *(undefined4 *)((int)&iStack_70 + iVar6) = 0x617434;
  KeyValues::SetInt(*(KeyValues **)(&stack0xffffff94 + iVar6),
                    *(char **)((int)&stack0xffffff98 + iVar6),*(int *)(&stack0xffffff9c + iVar6));
  piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x5199f6);
  iVar2 = *piVar1;
  *(int *)((int)&stack0xffffff98 + iVar6) = playerIndex;
  *(int **)(&stack0xffffff94 + iVar6) = piVar1;
  pcVar3 = *(code **)(iVar2 + 0x3bc);
  *(undefined4 *)((int)&iStack_70 + iVar6) = 0x61744e;
  uVar10 = (*pcVar3)();
  *(undefined4 *)(&stack0xffffff9c + iVar6) = uVar10;
  *(int *)((int)&stack0xffffff98 + iVar6) = unaff_EBX + 0x38884e;
  *(KeyValues **)(&stack0xffffff94 + iVar6) = local_54;
  *(undefined4 *)((int)&iStack_70 + iVar6) = 0x617467;
  KeyValues::SetInt(*(KeyValues **)(&stack0xffffff94 + iVar6),
                    *(char **)((int)&stack0xffffff98 + iVar6),*(int *)(&stack0xffffff9c + iVar6));
  *(int *)(&stack0xffffff9c + iVar6) = unaff_EBX + 0x346efe;
  iVar2 = unaff_EBX + 0x388842;
  *(int *)((int)&stack0xffffff98 + iVar6) = iVar2;
  *(KeyValues **)(&stack0xffffff94 + iVar6) = local_54;
  *(undefined4 *)((int)&iStack_70 + iVar6) = 0x617486;
  KeyValues::SetString
            (*(KeyValues **)(&stack0xffffff94 + iVar6),*(char **)((int)&stack0xffffff98 + iVar6),
             *(char **)(&stack0xffffff9c + iVar6));
  pp_Var4 = (local_50->super_CClientScoreBoardDialog).super_EditablePanel.super_Panel.
            super_IClientPanel._vptr_IClientPanel;
  *(KeyValues **)(&stack0xffffff9c + iVar6) = local_54;
  *(int *)((int)&stack0xffffff98 + iVar6) = playerIndex;
  *(CPortalClientScoreBoardDialog **)(&stack0xffffff94 + iVar6) = local_50;
  p_Var5 = pp_Var4[0xf6];
  *(undefined4 *)((int)&iStack_70 + iVar6) = 0x6174a5;
  (*p_Var5)();
  piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x5199f6);
  iVar11 = *piVar1;
  *(int *)((int)&stack0xffffff98 + iVar6) = playerIndex;
  *(int **)(&stack0xffffff94 + iVar6) = piVar1;
  pcVar3 = *(code **)(iVar11 + 0x3b4);
  *(undefined4 *)((int)&iStack_70 + iVar6) = 0x6174bf;
  iVar11 = (*pcVar3)();
  if (iVar11 < 1) {
    piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x5199f6);
    iVar11 = *piVar1;
    *(int *)((int)&stack0xffffff98 + iVar6) = playerIndex;
    *(int **)(&stack0xffffff94 + iVar6) = piVar1;
    pcVar3 = *(code **)(iVar11 + 0x3a4);
    *(undefined4 *)((int)&iStack_70 + iVar6) = 0x61762a;
    cVar7 = (*pcVar3)();
    if (cVar7 == '\0') {
      *(int *)(&stack0xffffff9c + iVar6) = unaff_EBX + 0x346efe;
      *(int *)((int)&stack0xffffff98 + iVar6) = unaff_EBX + 0x36c7d6;
      *(KeyValues **)(&stack0xffffff94 + iVar6) = local_54;
      *(undefined4 *)((int)&iStack_70 + iVar6) = 0x617679;
      KeyValues::SetString
                (*(KeyValues **)(&stack0xffffff94 + iVar6),*(char **)((int)&stack0xffffff98 + iVar6)
                 ,*(char **)(&stack0xffffff9c + iVar6));
    }
    else {
      *(int *)(&stack0xffffff9c + iVar6) = unaff_EBX + 0x388893;
      *(int *)((int)&stack0xffffff98 + iVar6) = unaff_EBX + 0x36c7d6;
      *(KeyValues **)(&stack0xffffff94 + iVar6) = local_54;
      *(undefined4 *)((int)&iStack_70 + iVar6) = 0x61764d;
      KeyValues::SetString
                (*(KeyValues **)(&stack0xffffff94 + iVar6),*(char **)((int)&stack0xffffff98 + iVar6)
                 ,*(char **)(&stack0xffffff9c + iVar6));
    }
  }
  else {
    piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x5199f6);
    iVar11 = *piVar1;
    *(int *)((int)&stack0xffffff98 + iVar6) = playerIndex;
    *(int **)(&stack0xffffff94 + iVar6) = piVar1;
    pcVar3 = *(code **)(iVar11 + 0x3b4);
    *(undefined4 *)((int)&iStack_70 + iVar6) = 0x6174e1;
    uVar10 = (*pcVar3)();
    *(undefined4 *)(&stack0xffffff9c + iVar6) = uVar10;
    *(int *)((int)&stack0xffffff98 + iVar6) = unaff_EBX + 0x36c7d6;
    *(KeyValues **)(&stack0xffffff94 + iVar6) = local_54;
    *(undefined4 *)((int)&iStack_70 + iVar6) = 0x6174fa;
    KeyValues::SetInt(*(KeyValues **)(&stack0xffffff94 + iVar6),
                      *(char **)((int)&stack0xffffff98 + iVar6),*(int *)(&stack0xffffff9c + iVar6));
  }
  *(undefined4 *)(&stack0xffffff94 + iVar6) = 0xffffffff;
  *(undefined4 *)((int)&iStack_70 + iVar6) = 0x617506;
  pCVar12 = C_BasePlayer::GetLocalPlayer(*(int *)(&stack0xffffff94 + iVar6));
  piVar1 = (int *)**(int **)(unaff_EBX + 0x5199f6);
  if ((piVar1 != (int *)0x0) && (pCVar12 != (C_BasePlayer *)0x0)) {
    iVar11 = *piVar1;
    *(int *)((int)&stack0xffffff98 + iVar6) = playerIndex;
    *(int **)(&stack0xffffff94 + iVar6) = piVar1;
    pcVar3 = *(code **)(iVar11 + 0x3ac);
    *(undefined4 *)((int)&iStack_70 + iVar6) = 0x617552;
    cVar7 = (*pcVar3)();
    if (cVar7 == '\0') {
      piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x5199f6);
      iVar11 = *piVar1;
      *(int *)((int)&stack0xffffff98 + iVar6) = playerIndex;
      *(int **)(&stack0xffffff94 + iVar6) = piVar1;
      pcVar3 = *(code **)(iVar11 + 0x3a0);
      *(undefined4 *)((int)&iStack_70 + iVar6) = 0x6175ba;
      cVar7 = (*pcVar3)();
      if (cVar7 == '\0') {
        piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x5199f6);
        iVar11 = *piVar1;
        *(int *)((int)&stack0xffffff98 + iVar6) = playerIndex;
        *(int **)(&stack0xffffff94 + iVar6) = piVar1;
        pcVar3 = *(code **)(iVar11 + 0x3c0);
        *(undefined4 *)((int)&iStack_70 + iVar6) = 0x6175dc;
        iVar11 = (*pcVar3)();
        if (1 < iVar11) {
          *(int *)(&stack0xffffff9c + iVar6) = unaff_EBX + 0x3888a5;
          *(int *)((int)&stack0xffffff98 + iVar6) = iVar2;
          *(KeyValues **)(&stack0xffffff94 + iVar6) = local_54;
          *(undefined4 *)((int)&iStack_70 + iVar6) = 0x6175fc;
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
      *(int *)(&stack0xffffff9c + iVar6) = unaff_EBX + 0x388897;
      *(undefined4 *)((int)&stack0xffffff98 + iVar6) = 0x20;
      *(undefined1 **)(&stack0xffffff94 + iVar6) = local_40;
      *(undefined4 *)((int)&iStack_70 + iVar6) = 0x617581;
      V_snprintf(*(char **)(&stack0xffffff94 + iVar6),*(int *)((int)&stack0xffffff98 + iVar6),
                 *(char **)(&stack0xffffff9c + iVar6));
      *(undefined1 **)(&stack0xffffff9c + iVar6) = local_40;
      *(int *)((int)&stack0xffffff98 + iVar6) = iVar2;
      *(KeyValues **)(&stack0xffffff94 + iVar6) = local_54;
      *(undefined4 *)((int)&iStack_70 + iVar6) = 0x617594;
      KeyValues::SetString
                (*(KeyValues **)(&stack0xffffff94 + iVar6),*(char **)((int)&stack0xffffff98 + iVar6)
                 ,*(char **)(&stack0xffffff9c + iVar6));
    }
  }
  if (local_20 == **(int **)(unaff_EBX + 0x5199a6)) {
    return true;
  }
                    /* WARNING: Subroutine does not return */
  *(undefined **)((int)&iStack_70 + iVar6) = &DAT_00617683;
  ___stack_chk_fail();
}


/* __static_initialization_and_destruction_0 at 00082aa0 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *pcVar1;
  int unaff_EBX;
  longlong lVar2;
  
  lVar2 = ___i686_get_pc_thunk_bx();
  if (lVar2 != 0xffff00000001) {
    return;
  }
  VarArgs::string[unaff_EBX + 0x2af] = '\0';
  VarArgs::string[unaff_EBX + 0x2b0] = '\0';
  VarArgs::string[unaff_EBX + 0x2b1] = '\0';
  VarArgs::string[unaff_EBX + 0x2b2] = '\0';
  pcVar1 = VarArgs::string + unaff_EBX + 0x2b3;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = VarArgs::string + unaff_EBX + 0x2b7;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = VarArgs::string + unaff_EBX + 699;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = VarArgs::string + unaff_EBX + 0x2bf;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = VarArgs::string + unaff_EBX + 0x2c3;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = VarArgs::string + unaff_EBX + 0x2c7;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = VarArgs::string + unaff_EBX + 0x2cb;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = VarArgs::string + unaff_EBX + 0x2cf;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = VarArgs::string + unaff_EBX + 0x2d3;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = VarArgs::string + unaff_EBX + 0x2d7;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = VarArgs::string + unaff_EBX + 0x2db;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = VarArgs::string + unaff_EBX + 0x2df;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  *(undefined **)(VarArgs::string + unaff_EBX + 0x2e3) = &UNK_00aaf35f + unaff_EBX;
  pcVar1 = VarArgs::string + unaff_EBX + 0x2e7;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  VarArgs::string[unaff_EBX + 0x2ee] = '\0';
  VarArgs::string[unaff_EBX + 0x2eb] = '\0';
  VarArgs::string[unaff_EBX + 0x2ec] = '\0';
  VarArgs::string[unaff_EBX + 0x2ed] = '\0';
  pcVar1 = VarArgs::string + unaff_EBX + 0x2ef;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  VarArgs::string[unaff_EBX + 0x2f6] = '\0';
  VarArgs::string[unaff_EBX + 0x2f3] = '\0';
  VarArgs::string[unaff_EBX + 0x2f4] = '\0';
  VarArgs::string[unaff_EBX + 0x2f5] = '\0';
  pcVar1 = VarArgs::string + unaff_EBX + 0x2f7;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  VarArgs::string[unaff_EBX + 0x2fe] = '\x01';
  VarArgs::string[unaff_EBX + 0x2fb] = '\0';
  VarArgs::string[unaff_EBX + 0x2fc] = '\0';
  VarArgs::string[unaff_EBX + 0x2fd] = '\0';
  pcVar1 = VarArgs::string + unaff_EBX + 0x2ff;
  pcVar1[0] = '\x01';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  VarArgs::string[unaff_EBX + 0x306] = '\0';
  VarArgs::string[unaff_EBX + 0x303] = '\0';
  VarArgs::string[unaff_EBX + 0x304] = '\0';
  VarArgs::string[unaff_EBX + 0x305] = '\0';
  pcVar1 = VarArgs::string + unaff_EBX + 0x307;
  pcVar1[0] = '\x02';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  VarArgs::string[unaff_EBX + 0x30e] = '\0';
  VarArgs::string[unaff_EBX + 0x30b] = '\0';
  VarArgs::string[unaff_EBX + 0x30c] = '\0';
  VarArgs::string[unaff_EBX + 0x30d] = '\0';
  return;
}


/* _GLOBAL__I__ZN29CPortalClientScoreBoardDialogC2EP9IViewPort at 00082bd0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN29CPortalClientScoreBoardDialogC2EP9IViewPort(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

