/* DWARF-guided pseudocode for game/client/portal2/vgui/vgui_level_placard.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* CLevelPlacardScreen::CLevelPlacardScreen at 0061d590 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void CLevelPlacardScreen(CLevelPlacardScreen * this, Panel * parent,
   char * panelName) */

void __thiscall
CLevelPlacardScreen::CLevelPlacardScreen(CLevelPlacardScreen *this,Panel *parent,char *panelName)

{
  int *piVar1;
  code *pcVar2;
  undefined4 uVar3;
  HScheme hScheme;
  Label *this_00;
  char *unaff_EBX;
  Label *in_stack_ffffffe4;
  Panel *in_stack_ffffffe8;
  char *in_stack_ffffffec;
  char *text_1;
  
  text_1 = unaff_EBX;
  ___i686_get_pc_thunk_bx();
  piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x513bfe);
  pcVar2 = *(code **)(*piVar1 + 0x34);
  uVar3 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x513e8a) + 8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x513e8a),2);
  hScheme = (*pcVar2)(piVar1,uVar3,unaff_EBX + 0x382ebe,unaff_EBX + 0x382eaa);
  CVGuiScreenPanel::CVGuiScreenPanel
            (&this->super_CVGuiScreenPanel,parent,unaff_EBX + 0x382ede,hScheme);
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a318a);
  (this->m_hVGUIScreen).super_CBaseHandle.m_Index = 0xffffffff;
  this_00 = operator_new(0x1a4);
  vgui::Label::Label(this_00,(Panel *)this,unaff_EBX + 0x371176,unaff_EBX + 0x382ef2,
                     in_stack_ffffffe4,in_stack_ffffffe8,in_stack_ffffffec,text_1);
  this->m_pDisplayTextLabel = this_00;
  this->m_nTextureId = -1;
  return;
}


/* CLevelPlacardScreen::CLevelPlacardScreen at 0061d6a0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void CLevelPlacardScreen(CLevelPlacardScreen * this, Panel * parent,
   char * panelName, CLevelPlacardScreen * this, Panel * parent, char * panelName) */

void __thiscall
CLevelPlacardScreen::CLevelPlacardScreen
          (CLevelPlacardScreen *this,Panel *parent,char *panelName,CLevelPlacardScreen *this_1,
          Panel *parent_1,char *panelName_1)

{
  CLevelPlacardScreen(this,parent,panelName);
  return;
}


/* CPanelFactory<CLevelPlacardScreen,VGuiScreenInitData_t>::Create at 0061d7c0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: Panel * Create(CPanelFactory<CLevelPlacardScreen,VGuiScreenInitData_t>
   * this, char * pMetaClassName, KeyValues * pKeyValues, void * pVoidInitData, Panel * pParent) */

Panel * __thiscall
CPanelFactory<CLevelPlacardScreen,VGuiScreenInitData_t>::Create
          (CPanelFactory<CLevelPlacardScreen,VGuiScreenInitData_t> *this,char *pMetaClassName,
          KeyValues *pKeyValues,void *pVoidInitData,Panel *pParent)

{
  CLevelPlacardScreen *this_00;
  int iVar1;
  CLevelPlacardScreen *pCVar2;
  
                    /* Unresolved local var: CLevelPlacardScreen * pPanel@[???] */
  this_00 = operator_new(0x18c);
  CLevelPlacardScreen::CLevelPlacardScreen(this_00,pParent,pMetaClassName);
  pCVar2 = (CLevelPlacardScreen *)0x0;
  if (this_00 != (CLevelPlacardScreen *)0x0) {
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


/* CLevelPlacardScreen::~CLevelPlacardScreen at 0061d510 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CLevelPlacardScreen(CLevelPlacardScreen * this, int __in_chrg) */

void __thiscall CLevelPlacardScreen::~CLevelPlacardScreen(CLevelPlacardScreen *this,int __in_chrg)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a3204);
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x513c64) + 0x230))
            ((int *)**(undefined4 **)(unaff_EBX + 0x513c64),this->m_nTextureId);
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x513eec) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,__in_chrg);
  return;
}


/* CLevelPlacardScreen::~CLevelPlacardScreen at 0061d6b0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CLevelPlacardScreen(CLevelPlacardScreen * this, int __in_chrg) */

void __thiscall CLevelPlacardScreen::~CLevelPlacardScreen(CLevelPlacardScreen *this,int __in_chrg)

{
  int unaff_EBX;
  int __in_chrg_00;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a3064);
  __in_chrg_00 = this->m_nTextureId;
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x513ac4) + 0x230))
            ((int *)**(undefined4 **)(unaff_EBX + 0x513ac4));
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x513d4c) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,__in_chrg_00);
  operator_delete(this);
  return;
}


/* CLevelPlacardScreen::~CLevelPlacardScreen at 0061d740 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CLevelPlacardScreen(CLevelPlacardScreen * this, int __in_chrg) */

void __thiscall CLevelPlacardScreen::~CLevelPlacardScreen(CLevelPlacardScreen *this,int __in_chrg)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a2fd4);
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x513a34) + 0x230))
            ((int *)**(undefined4 **)(unaff_EBX + 0x513a34),this->m_nTextureId);
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x513cbc) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,__in_chrg);
  return;
}


/* CLevelPlacardScreen::ApplySchemeSettings at 0061d290 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ApplySchemeSettings(CLevelPlacardScreen * this, IScheme * pScheme)
    */

void __thiscall CLevelPlacardScreen::ApplySchemeSettings(CLevelPlacardScreen *this,IScheme *pScheme)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(this->m_pDisplayTextLabel->super_Panel).super_IClientPanel._vptr_IClientPanel[0x3d])
            (this->m_pDisplayTextLabel,0xffffffff);
  (*(this->m_pDisplayTextLabel->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd4])
            (this->m_pDisplayTextLabel,unaff_EBX + 0x340fcb);
                    /* WARNING: Could not recover jumptable at 0x0061d2fc. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->m_pDisplayTextLabel->super_Panel).super_IClientPanel._vptr_IClientPanel[0x22])();
  return;
}


/* CLevelPlacardScreen::Init at 0061d310 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: bool Init(CLevelPlacardScreen * this, KeyValues * pKeyValues,
   VGuiScreenInitData_t * pInitData) */

bool __thiscall
CLevelPlacardScreen::Init
          (CLevelPlacardScreen *this,KeyValues *pKeyValues,VGuiScreenInitData_t *pInitData)

{
  code *pcVar1;
  uint uVar2;
  bool bVar3;
  int iVar4;
  int *piVar5;
  ulong *puVar6;
  bool bVar7;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  piVar5 = (int *)**(undefined4 **)(unaff_EBX + 0x513e1c);
  pcVar1 = *(code **)(*piVar5 + 0x54);
  iVar4 = (**(this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
             _vptr_IClientPanel)(this);
  (*pcVar1)(piVar5,iVar4,0);
  bVar3 = CVGuiScreenPanel::Init(&this->super_CVGuiScreenPanel,pKeyValues,pInitData);
  bVar7 = false;
  if (bVar3) {
    uVar2 = (this->super_CVGuiScreenPanel).m_hEntity.super_CBaseHandle.m_Index;
    if ((((uVar2 != 0xffffffff) &&
         (iVar4 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x513a6c),
         *(uint *)(iVar4 + 8) == uVar2 >> 0x10)) && (iVar4 = *(int *)(iVar4 + 4), iVar4 != 0)) &&
       (piVar5 = (int *)___dynamic_cast(iVar4,*(undefined4 *)(unaff_EBX + 0x513a08),
                                        *(undefined4 *)(unaff_EBX + 0x51432c),0),
       piVar5 != (int *)0x0)) {
      puVar6 = (ulong *)(**(code **)(*piVar5 + 0xc))(piVar5);
      (this->m_hVGUIScreen).super_CBaseHandle.m_Index = *puVar6;
      return true;
    }
    (this->m_hVGUIScreen).super_CBaseHandle.m_Index = 0xffffffff;
    bVar7 = true;
  }
  return bVar7;
}


/* CLevelPlacardScreen::IsActive at 0061d4b0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: bool IsActive(CLevelPlacardScreen * this) */

bool __thiscall CLevelPlacardScreen::IsActive(CLevelPlacardScreen *this)

{
  uint uVar1;
  C_VGuiScreen *this_00;
  bool bVar2;
  int iVar3;
  int unaff_EBX;
  
                    /* Unresolved local var: bool bScreenActive@[???] */
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->m_hVGUIScreen).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5138d4),
      *(uint *)(iVar3 + 8) == uVar1 >> 0x10)) &&
     (this_00 = *(C_VGuiScreen **)(iVar3 + 4), this_00 != (C_VGuiScreen *)0x0)) {
    bVar2 = C_VGuiScreen::IsActive(this_00);
    return bVar2;
  }
  return false;
}


/* CLevelPlacardScreen::OnTick at 0061d300 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void OnTick(CLevelPlacardScreen * this) */

void __thiscall CLevelPlacardScreen::OnTick(CLevelPlacardScreen *this)

{
  vgui::Panel::OnTick((Panel *)this);
  return;
}


/* CLevelPlacardScreen::Paint at 0061d410 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void Paint(CLevelPlacardScreen * this) */

void __thiscall CLevelPlacardScreen::Paint(CLevelPlacardScreen *this)

{
  undefined4 *puVar1;
  int *piVar2;
  code *pcVar3;
  int iVar4;
  int iVar5;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  puVar1 = *(undefined4 **)(unaff_EBX + 0x513d7c);
  piVar2 = (int *)*puVar1;
  (**(code **)(*piVar2 + 0x34))(piVar2,0,0,0,0xff);
  piVar2 = (int *)*puVar1;
  pcVar3 = *(code **)(*piVar2 + 0x3c);
  iVar4 = vgui::Panel::GetTall((Panel *)this);
  iVar5 = vgui::Panel::GetWide((Panel *)this);
  (*pcVar3)(piVar2,0,0,iVar5,iVar4);
  vgui::Panel::Paint((Panel *)this);
  return;
}


/* __static_initialization_and_destruction_0 at 00082f70 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *pcVar1;
  IPanelMetaClassMgr *pIVar2;
  int unaff_EBX;
  longlong lVar3;
  
  lVar3 = ___i686_get_pc_thunk_bx();
  if (lVar3 != 0xffff00000001) {
    return;
  }
  VarArgs::string[unaff_EBX + 0x20] = '\0';
  VarArgs::string[unaff_EBX + 0x21] = '\0';
  VarArgs::string[unaff_EBX + 0x22] = '\0';
  VarArgs::string[unaff_EBX + 0x23] = '\0';
  pcVar1 = VarArgs::string + unaff_EBX + 0x24;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = VarArgs::string + unaff_EBX + 0x28;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = VarArgs::string + unaff_EBX + 0x2c;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = VarArgs::string + unaff_EBX + 0x30;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = VarArgs::string + unaff_EBX + 0x34;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = VarArgs::string + unaff_EBX + 0x38;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = VarArgs::string + unaff_EBX + 0x3c;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = VarArgs::string + unaff_EBX + 0x40;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = VarArgs::string + unaff_EBX + 0x44;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = VarArgs::string + unaff_EBX + 0x48;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = VarArgs::string + unaff_EBX + 0x4c;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = VarArgs::string + unaff_EBX + 0x50;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  *(undefined **)(VarArgs::string + unaff_EBX + 0x54) = &UNK_00aaee8c + unaff_EBX;
  *(int *)((int)ice_sbox[1] + unaff_EBX + 0xda0) = (int)&PTR_GetBaseAnimating_00b3db8c + unaff_EBX;
  pIVar2 = PanelMetaClassMgr();
  (*pIVar2->_vptr_IPanelMetaClassMgr[1])(pIVar2,unaff_EBX + 0x91d4b7,unaff_EBX + 0xcce660);
  return;
}


/* _GLOBAL__I_g_CLevelPlacardScreenFactory at 00083050 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_g_CLevelPlacardScreenFactory(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

