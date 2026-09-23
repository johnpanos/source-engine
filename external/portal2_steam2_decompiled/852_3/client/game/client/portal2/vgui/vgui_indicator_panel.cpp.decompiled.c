/* DWARF-guided pseudocode for game/client/portal2/vgui/vgui_indicator_panel.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* CIndicatorScreen::~CIndicatorScreen at 0061d110 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CIndicatorScreen(CIndicatorScreen * this, int __in_chrg) */

void __thiscall CIndicatorScreen::~CIndicatorScreen(CIndicatorScreen *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(extraout_ECX + 0x5a3210);
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(*(int *)(extraout_ECX + 0x5142f8) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,__in_chrg);
  return;
}


/* CIndicatorScreen::~CIndicatorScreen at 0061d140 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CIndicatorScreen(CIndicatorScreen * this, int __in_chrg) */

void __thiscall CIndicatorScreen::~CIndicatorScreen(CIndicatorScreen *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a31d7);
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x5142bf) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CIndicatorScreen::CIndicatorScreen at 0061bba0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void CIndicatorScreen(CIndicatorScreen * this, Panel * parent, char *
   panelName) */

void __thiscall
CIndicatorScreen::CIndicatorScreen(CIndicatorScreen *this,Panel *parent,char *panelName)

{
  int *piVar1;
  code *pcVar2;
  undefined4 uVar3;
  HScheme hScheme;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x5155e8);
  pcVar2 = *(code **)(*piVar1 + 0x34);
  uVar3 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x515874) + 8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x515874),2);
  hScheme = (*pcVar2)(piVar1,uVar3,unaff_EBX + 0x384767,unaff_EBX + 0x384752);
  CVGuiScreenPanel::CVGuiScreenPanel
            (&this->super_CVGuiScreenPanel,parent,(char *)(unaff_EBX + 0x384784),hScheme);
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a4774);
  (this->m_hVGUIScreen).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hScreenEntity).super_CBaseHandle.m_Index = 0xffffffff;
  this->m_bWasCounting = false;
  this->m_flFadeUpTime = 0.0;
  return;
}


/* CIndicatorScreen::CIndicatorScreen at 0061d050 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void CIndicatorScreen(CIndicatorScreen * this, Panel * parent, char *
   panelName, CIndicatorScreen * this, Panel * parent, char * panelName) */

void __thiscall
CIndicatorScreen::CIndicatorScreen
          (CIndicatorScreen *this,Panel *parent,char *panelName,CIndicatorScreen *this_1,
          Panel *parent_1,char *panelName_1)

{
  int *piVar1;
  code *pcVar2;
  undefined4 uVar3;
  HScheme hScheme;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  piVar1 = (int *)**(undefined4 **)(&DAT_00514138 + unaff_EBX);
  pcVar2 = *(code **)(*piVar1 + 0x34);
  uVar3 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5143c4) + 8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x5143c4),2);
  hScheme = (*pcVar2)(piVar1,uVar3,unaff_EBX + 0x3832b7,unaff_EBX + 0x3832a2);
  CVGuiScreenPanel::CVGuiScreenPanel
            (&this->super_CVGuiScreenPanel,parent,(char *)(unaff_EBX + 0x3832d4),hScheme);
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5a32c4);
  (this->m_hVGUIScreen).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hScreenEntity).super_CBaseHandle.m_Index = 0xffffffff;
  this->m_bWasCounting = false;
  this->m_flFadeUpTime = 0.0;
  return;
}


/* CPanelFactory<CIndicatorScreen,VGuiScreenInitData_t>::Create at 0061d190 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: Panel * Create(CPanelFactory<CIndicatorScreen,VGuiScreenInitData_t> *
   this, char * pMetaClassName, KeyValues * pKeyValues, void * pVoidInitData, Panel * pParent) */

Panel * __thiscall
CPanelFactory<CIndicatorScreen,VGuiScreenInitData_t>::Create
          (CPanelFactory<CIndicatorScreen,VGuiScreenInitData_t> *this,char *pMetaClassName,
          KeyValues *pKeyValues,void *pVoidInitData,Panel *pParent)

{
  int *piVar1;
  code *pcVar2;
  char cVar3;
  CVGuiScreenPanel *this_00;
  undefined4 uVar4;
  HScheme hScheme;
  int unaff_EBX;
  
                    /* Unresolved local var: CIndicatorScreen * pPanel@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = operator_new(0x1b0);
  piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x513ffe);
  pcVar2 = *(code **)(*piVar1 + 0x34);
  uVar4 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51428a) + 8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x51428a),2);
  hScheme = (*pcVar2)(piVar1,uVar4,&UNK_0038317d + unaff_EBX,&UNK_00383168 + unaff_EBX);
  CVGuiScreenPanel::CVGuiScreenPanel(this_00,pParent,(char *)(unaff_EBX + 0x38319a),hScheme);
  (this_00->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5a318a);
  this_00[1].super_EditablePanel.super_Panel.m_pDragDrop = (DragDrop_t *)0xffffffff;
  *(undefined4 *)this_00[1].super_EditablePanel.super_Panel.m_clrDragFrame._color = 0xffffffff;
  this_00[1].super_EditablePanel.super_Panel.m_InternalKeyCodePressed_register =
       (PanelMessageFunc_InternalKeyCodePressed)0x0;
  this_00[1].super_EditablePanel.super_Panel.m_InternalKeyFocusTicked_register =
       (PanelMessageFunc_InternalKeyFocusTicked)0x0;
  this_00[1].super_EditablePanel.super_Panel.m_InternalMouseFocusTicked_register =
       (PanelMessageFunc_InternalMouseFocusTicked)0x0;
  this_00[1].super_EditablePanel.super_Panel.m_InternalInvalidateLayout_register =
       (PanelMessageFunc_InternalInvalidateLayout)0x0;
  this_00[1].super_EditablePanel.super_Panel.m_InternalMove_register =
       (PanelMessageFunc_InternalMove)0x0;
                    /* Unresolved local var: VGuiScreenInitData_t * pInitData@[???] */
  cVar3 = (**(code **)(unaff_EBX + 0x5a353e))(this_00,pKeyValues,pVoidInitData);
  if (cVar3 == '\0') {
    (*(this_00->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel[0x21])
              (this_00);
    this_00 = (CVGuiScreenPanel *)0x0;
  }
  return (Panel *)this_00;
}


/* CIndicatorScreen::ApplySchemeSettings at 0061b9b0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ApplySchemeSettings(CIndicatorScreen * this, IScheme * pScheme) */

void __thiscall CIndicatorScreen::ApplySchemeSettings(CIndicatorScreen *this,IScheme *pScheme)

{
  return;
}


/* CIndicatorScreen::Init at 0061cc80 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: bool Init(CIndicatorScreen * this, KeyValues * pKeyValues,
   VGuiScreenInitData_t * pInitData) */

bool __thiscall
CIndicatorScreen::Init(CIndicatorScreen *this,KeyValues *pKeyValues,VGuiScreenInitData_t *pInitData)

{
  code *pcVar1;
  uint uVar2;
  undefined4 *puVar3;
  undefined4 uVar4;
  bool bVar5;
  int iVar6;
  int *piVar7;
  uint *puVar8;
  int iVar9;
  int *piVar10;
  ulong *puVar11;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  piVar10 = (int *)**(undefined4 **)(unaff_EBX + 0x5144b2);
  pcVar1 = *(code **)(*piVar10 + 0x54);
  iVar6 = (**(this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
             _vptr_IClientPanel)(this);
  (*pcVar1)(piVar10,iVar6,0);
  bVar5 = CVGuiScreenPanel::Init(&this->super_CVGuiScreenPanel,pKeyValues,pInitData);
  if (!bVar5) {
    return false;
  }
  piVar10 = *(int **)(unaff_EBX + 0x514102);
  uVar2 = (this->super_CVGuiScreenPanel).m_hEntity.super_CBaseHandle.m_Index;
  if (((uVar2 != 0xffffffff) &&
      (iVar6 = (uVar2 & 0xffff) * 0x10 + *piVar10, *(uint *)(iVar6 + 8) == uVar2 >> 0x10)) &&
     (iVar6 = *(int *)(iVar6 + 4), iVar6 != 0)) {
    uVar4 = *(undefined4 *)(unaff_EBX + 0x51409e);
    piVar7 = (int *)___dynamic_cast(iVar6,uVar4,*(undefined4 *)(unaff_EBX + 0x5149c2),0);
    if (piVar7 != (int *)0x0) {
      puVar8 = (uint *)(**(code **)(*piVar7 + 0xc))(piVar7);
      uVar2 = *puVar8;
      (this->m_hVGUIScreen).super_CBaseHandle.m_Index = uVar2;
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
      iVar6 = *piVar10;
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
      if (((uVar2 != 0xffffffff) &&
          (iVar9 = (uVar2 & 0xffff) * 0x10 + iVar6, *(uint *)(iVar9 + 8) == uVar2 >> 0x10)) &&
         (iVar9 = *(int *)(iVar9 + 4), iVar9 != 0)) {
        uVar2 = *(uint *)(iVar9 + 0x128);
        if (((uVar2 == 0xffffffff) ||
            (iVar6 = (uVar2 & 0xffff) * 0x10 + iVar6, *(uint *)(iVar6 + 8) != uVar2 >> 0x10)) ||
           ((iVar6 = *(int *)(iVar6 + 4), iVar6 == 0 ||
            (piVar10 = (int *)___dynamic_cast(iVar6,uVar4,*(undefined4 *)(unaff_EBX + 0x5149c6),0),
            piVar10 == (int *)0x0)))) {
          (this->m_hScreenEntity).super_CBaseHandle.m_Index = 0xffffffff;
        }
        else {
          puVar11 = (ulong *)(**(code **)(*piVar10 + 0xc))(piVar10);
          (this->m_hScreenEntity).super_CBaseHandle.m_Index = *puVar11;
        }
      }
      goto LAB_0061cd0f;
    }
  }
  (this->m_hVGUIScreen).super_CBaseHandle.m_Index = 0xffffffff;
LAB_0061cd0f:
  puVar3 = *(undefined4 **)(unaff_EBX + 0x514512);
  iVar6 = (**(code **)(*(int *)*puVar3 + 0x9c))((int *)*puVar3,0);
  this->m_nTimerBackgroundTextureID = iVar6;
  (**(code **)(*(int *)*puVar3 + 0x84))((int *)*puVar3,iVar6,unaff_EBX + 0x3836cb,1,0);
  iVar6 = (**(code **)(*(int *)*puVar3 + 0x9c))((int *)*puVar3,0);
  this->m_nTimerBackgroundActiveTextureID = iVar6;
  (**(code **)(*(int *)*puVar3 + 0x84))((int *)*puVar3,iVar6,unaff_EBX + 0x3836ea,1,0);
  iVar6 = (**(code **)(*(int *)*puVar3 + 0x9c))((int *)*puVar3,0);
  this->m_nTimerUpperSliceTextureID = iVar6;
  (**(code **)(*(int *)*puVar3 + 0x84))((int *)*puVar3,iVar6,unaff_EBX + 0x38370a,1,0);
  iVar6 = (**(code **)(*(int *)*puVar3 + 0x9c))((int *)*puVar3,0);
  this->m_nTimerLowerSliceTextureID = iVar6;
  (**(code **)(*(int *)*puVar3 + 0x84))((int *)*puVar3,iVar6,unaff_EBX + 0x383723,1,0);
  iVar6 = (**(code **)(*(int *)*puVar3 + 0x9c))((int *)*puVar3,0);
  this->m_nCheckTextureID = iVar6;
  (**(code **)(*(int *)*puVar3 + 0x84))((int *)*puVar3,iVar6,unaff_EBX + 0x38373c,1,0);
  iVar6 = (**(code **)(*(int *)*puVar3 + 0x9c))((int *)*puVar3,0);
  this->m_nUncheckedTextureID = iVar6;
  (**(code **)(*(int *)*puVar3 + 0x84))((int *)*puVar3,iVar6,unaff_EBX + 0x383756,1,0);
  iVar6 = (**(code **)(*(int *)*puVar3 + 0x9c))((int *)*puVar3,0);
  this->m_nXTextureID = iVar6;
  (**(code **)(*(int *)*puVar3 + 0x84))((int *)*puVar3,iVar6,unaff_EBX + 0x383772,1,0);
  iVar6 = (**(code **)(*(int *)*puVar3 + 0x9c))((int *)*puVar3,0);
  this->m_nOTextureID = iVar6;
  (**(code **)(*(int *)*puVar3 + 0x84))((int *)*puVar3,iVar6,unaff_EBX + 0x383782,1,0);
  return true;
}


/* CIndicatorScreen::OnTick at 0061bb90 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void OnTick(CIndicatorScreen * this) */

void __thiscall CIndicatorScreen::OnTick(CIndicatorScreen *this)

{
  vgui::Panel::OnTick((Panel *)this);
  return;
}


/* CIndicatorScreen::PaintCountdownTimer at 0061bc60 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_LabIndicatorPanel": ignoring multiple overlapping fields */
/* DWARF original prototype: void PaintCountdownTimer(CIndicatorScreen * this) */

void __thiscall CIndicatorScreen::PaintCountdownTimer(CIndicatorScreen *this)

{
  float fVar1;
  uint uVar2;
  bool bVar3;
  int iVar4;
  C_LabIndicatorPanel *this_00;
  int iVar5;
  undefined4 *puVar6;
  undefined2 uVar7;
  code *pcVar8;
  undefined4 *puVar9;
  undefined4 *puVar10;
  int *piVar11;
  int unaff_EBX;
  float *pfVar12;
  longdouble lVar13;
  float fVar14;
  float fVar15;
  float local_34;
  float local_2c;
  float local_28;
  
                    /* Unresolved local var: int NUM_PIE_SECTIONS@[???]
                       Unresolved local var: float flPerc@[???]
                       Unresolved local var: int nWhole@[???]
                       Unresolved local var: float flFraction@[???]
                       Unresolved local var: float flAlpha@[???]
                       Unresolved local var: int cx@[???]
                       Unresolved local var: int cy@[???]
                       Unresolved local var: bool bTimedOut@[???]
                       Unresolved local var: float flGlobalAlpha@[???] */
  ___i686_get_pc_thunk_bx();
  piVar11 = *(int **)(unaff_EBX + 0x51511f);
  uVar2 = (this->m_hScreenEntity).super_CBaseHandle.m_Index;
  if ((uVar2 == 0xffffffff) ||
     (iVar4 = (uVar2 & 0xffff) * 0x10 + *piVar11, *(uint *)(iVar4 + 8) != uVar2 >> 0x10)) {
    this_00 = (C_LabIndicatorPanel *)0x0;
  }
  else {
    this_00 = *(C_LabIndicatorPanel **)(iVar4 + 4);
  }
  local_34 = C_LabIndicatorPanel::GetCountdownPercentage(this_00);
  local_34 = *(float *)(unaff_EBX + 0x39b05b) * local_34;
  lVar13 = (longdouble)_ceilf(local_34);
  fVar14 = (float)(int)lVar13 - local_34;
  if ((*(float *)(unaff_EBX + 0x39ace3) <= fVar14) ||
     (NAN(fVar14) || NAN(*(float *)(unaff_EBX + 0x39ace3)))) {
    local_28 = 0.0;
    if (fVar14 < *(float *)(unaff_EBX + 0x39abd3) || fVar14 == *(float *)(unaff_EBX + 0x39abd3)) {
      local_28 = fVar14 * *(float *)(unaff_EBX + 0x39d917) + *(float *)(unaff_EBX + 0x39af8f);
    }
  }
  else {
    local_28 = *(float *)(unaff_EBX + 0x39af8f);
  }
  iVar4 = vgui::Panel::GetWide((Panel *)this);
  puVar9 = (undefined4 *)(iVar4 / 2);
  iVar4 = vgui::Panel::GetTall((Panel *)this);
  puVar10 = (undefined4 *)(iVar4 / 2);
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51552f) + 0x38))
            ((int *)**(undefined4 **)(unaff_EBX + 0x51552f),0xffffffff);
  bVar3 = local_34 <= *(float *)(unaff_EBX + 0x39b963);
  uVar2 = (this->m_hScreenEntity).super_CBaseHandle.m_Index;
  if ((uVar2 == 0xffffffff) ||
     (iVar4 = (uVar2 & 0xffff) * 0x10 + *piVar11, *(uint *)(iVar4 + 8) != uVar2 >> 0x10)) {
    if (DAT_00000a5b != '\0') goto LAB_0061bd88;
LAB_0061c170:
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51552f) + 0x8c))
              ((int *)**(undefined4 **)(unaff_EBX + 0x51552f),this->m_nTimerBackgroundTextureID);
    piVar11 = (int *)**(undefined4 **)(unaff_EBX + 0x51552f);
    pcVar8 = *(code **)(*piVar11 + 0x94);
    puVar6 = (undefined4 *)vgui::Panel::GetTall((Panel *)this);
    iVar4 = vgui::Panel::GetWide((Panel *)this);
    (*pcVar8)(piVar11,0,0,iVar4,puVar6);
    if (!bVar3) goto LAB_0061bef2;
LAB_0061bdf4:
    if (this->m_bWasCounting == false) {
      fVar14 = this->m_flFadeUpTime;
      fVar15 = fVar14 + *(float *)(unaff_EBX + 0x39adcf);
      fVar1 = *(float *)(**(int **)(unaff_EBX + 0x5150df) + 0xc);
      if ((fVar14 != fVar15) || (NAN(fVar14) || NAN(fVar15))) {
        fVar14 = (fVar1 - fVar14) / (fVar15 - fVar14);
        local_2c = 0.0;
        if (0.0 <= fVar14) {
          if (fVar14 < *(float *)(unaff_EBX + 0x39abd3) ||
              fVar14 == *(float *)(unaff_EBX + 0x39abd3)) {
            local_2c = fVar14 * *(float *)(unaff_EBX + 0x39af8f) + *(float *)(unaff_EBX + 0x39ace3);
          }
          else {
            local_2c = *(float *)(unaff_EBX + 0x39af8f);
          }
        }
      }
      else {
        local_2c = 0.0;
        if (0.0 <= fVar1 - fVar15) {
          local_2c = *(float *)(unaff_EBX + 0x39af8f);
        }
      }
      iVar4 = (int)local_2c;
    }
    else {
      this->m_flFadeUpTime = *(float *)(**(int **)(unaff_EBX + 0x5150df) + 0xc);
      this->m_bWasCounting = false;
      local_2c = 0.0;
      iVar4 = 0;
    }
    puVar6 = *(undefined4 **)(unaff_EBX + 0x51552f);
    piVar11 = (int *)*puVar6;
    pcVar8 = *(code **)(*piVar11 + 0x38);
    local_34 = 8.0;
LAB_0061c20f:
    (*pcVar8)(piVar11,CONCAT22((short)((uint)puVar6 >> 0x10),0xffff) & 0xffffff | 0xff0000 |
                      iVar4 << 0x18);
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51552f) + 0x8c))
              ((int *)**(undefined4 **)(unaff_EBX + 0x51552f),this->m_nTimerUpperSliceTextureID);
    piVar11 = (int *)**(undefined4 **)(unaff_EBX + 0x51552f);
    pcVar8 = *(code **)(*piVar11 + 0x94);
    iVar4 = vgui::Panel::GetTall((Panel *)this);
    iVar5 = vgui::Panel::GetWide((Panel *)this);
    (*pcVar8)(piVar11,puVar9,0,iVar5 / 2 + (int)puVar9,iVar4 / 2);
    puVar6 = puVar9;
  }
  else {
    if (*(char *)(*(int *)(iVar4 + 4) + 0xa5b) == '\0') goto LAB_0061c170;
LAB_0061bd88:
    if (bVar3) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51552f) + 0x8c))
                ((int *)**(undefined4 **)(unaff_EBX + 0x51552f),this->m_nTimerBackgroundTextureID);
      piVar11 = (int *)**(undefined4 **)(unaff_EBX + 0x51552f);
      pcVar8 = *(code **)(*piVar11 + 0x94);
      iVar4 = vgui::Panel::GetTall((Panel *)this);
      iVar5 = vgui::Panel::GetWide((Panel *)this);
      (*pcVar8)(piVar11,0,0,iVar5,iVar4);
      goto LAB_0061bdf4;
    }
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51552f) + 0x8c))
              ((int *)**(undefined4 **)(unaff_EBX + 0x51552f),
               this->m_nTimerBackgroundActiveTextureID);
    piVar11 = (int *)**(undefined4 **)(unaff_EBX + 0x51552f);
    pcVar8 = *(code **)(*piVar11 + 0x94);
    puVar6 = (undefined4 *)vgui::Panel::GetTall((Panel *)this);
    iVar4 = vgui::Panel::GetWide((Panel *)this);
    (*pcVar8)(piVar11,0,0,iVar4,puVar6);
LAB_0061bef2:
    this->m_bWasCounting = true;
    if (*(float *)(unaff_EBX + 0x39bb77) <= local_34 && local_34 != *(float *)(unaff_EBX + 0x39bb77)
       ) {
      piVar11 = (int *)**(undefined4 **)(unaff_EBX + 0x51552f);
      pcVar8 = *(code **)(*piVar11 + 0x38);
      iVar4 = (int)local_28;
      local_2c = *(float *)(unaff_EBX + 0x39af8f);
      goto LAB_0061c20f;
    }
    local_2c = *(float *)(unaff_EBX + 0x39af8f);
  }
  uVar7 = (undefined2)((uint)puVar6 >> 0x10);
  if ((local_34 < *(float *)(unaff_EBX + 0x39af6b) || local_34 == *(float *)(unaff_EBX + 0x39af6b))
     || (*(float *)(unaff_EBX + 0x39bb77) <= local_34)) {
    if (*(float *)(unaff_EBX + 0x39bb77) <= local_34 && local_34 != *(float *)(unaff_EBX + 0x39bb77)
       ) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51552f) + 0x38))
                ((int *)**(undefined4 **)(unaff_EBX + 0x51552f),
                 CONCAT22(uVar7,0xffff) & 0xffffff | 0xff0000 | (int)local_2c << 0x18);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51552f) + 0x8c))
                ((int *)**(undefined4 **)(unaff_EBX + 0x51552f),this->m_nTimerLowerSliceTextureID);
      piVar11 = (int *)**(undefined4 **)(unaff_EBX + 0x51552f);
      pcVar8 = *(code **)(*piVar11 + 0x94);
      iVar4 = vgui::Panel::GetTall((Panel *)this);
      iVar5 = vgui::Panel::GetWide((Panel *)this);
      (*pcVar8)(piVar11,puVar9,0,iVar5 / 2 + (int)puVar9,iVar4 / 2);
      puVar6 = puVar9;
    }
  }
  else {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51552f) + 0x38))
              ((int *)**(undefined4 **)(unaff_EBX + 0x51552f),
               CONCAT22(uVar7,0xffff) & 0xffffff | 0xff0000 | (int)local_28 << 0x18);
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51552f) + 0x8c))
              ((int *)**(undefined4 **)(unaff_EBX + 0x51552f),this->m_nTimerLowerSliceTextureID);
    piVar11 = (int *)**(undefined4 **)(unaff_EBX + 0x51552f);
    pcVar8 = *(code **)(*piVar11 + 0x94);
    iVar4 = vgui::Panel::GetTall((Panel *)this);
    iVar5 = vgui::Panel::GetWide((Panel *)this);
    (*pcVar8)(piVar11,puVar9,0,iVar5 / 2 + (int)puVar9,iVar4 / 2);
    puVar6 = puVar9;
  }
  uVar7 = (undefined2)((uint)puVar6 >> 0x10);
  if ((local_34 < *(float *)(unaff_EBX + 0x39acbb) || local_34 == *(float *)(unaff_EBX + 0x39acbb))
     || (*(float *)(unaff_EBX + 0x39af6b) <= local_34)) {
    if (*(float *)(unaff_EBX + 0x39af6b) <= local_34 && local_34 != *(float *)(unaff_EBX + 0x39af6b)
       ) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51552f) + 0x38))
                ((int *)**(undefined4 **)(unaff_EBX + 0x51552f),
                 CONCAT22(uVar7,0xffff) & 0xffffff | 0xff0000 | (int)local_2c << 0x18);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51552f) + 0x8c))
                ((int *)**(undefined4 **)(unaff_EBX + 0x51552f),this->m_nTimerLowerSliceTextureID);
      piVar11 = (int *)**(undefined4 **)(unaff_EBX + 0x51552f);
      pcVar8 = *(code **)(*piVar11 + 0x194);
      iVar4 = vgui::Panel::GetTall((Panel *)this);
      iVar5 = vgui::Panel::GetWide((Panel *)this);
      (*pcVar8)(piVar11,puVar9,puVar10,iVar5 / 2 + (int)puVar9,iVar4 / 2 + (int)puVar10,0,0x3f800000
                ,0x3f800000,0);
      puVar6 = puVar10;
    }
  }
  else {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51552f) + 0x38))
              ((int *)**(undefined4 **)(unaff_EBX + 0x51552f),
               CONCAT22(uVar7,0xffff) & 0xffffff | 0xff0000 | (int)local_28 << 0x18);
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51552f) + 0x8c))
              ((int *)**(undefined4 **)(unaff_EBX + 0x51552f),this->m_nTimerLowerSliceTextureID);
    piVar11 = (int *)**(undefined4 **)(unaff_EBX + 0x51552f);
    pcVar8 = *(code **)(*piVar11 + 0x194);
    iVar4 = vgui::Panel::GetTall((Panel *)this);
    iVar5 = vgui::Panel::GetWide((Panel *)this);
    (*pcVar8)(piVar11,puVar9,puVar10,iVar5 / 2 + (int)puVar9,iVar4 / 2 + (int)puVar10,0,0x3f800000,
              0x3f800000,0);
    puVar6 = puVar10;
  }
  uVar7 = (undefined2)((uint)puVar6 >> 0x10);
  if ((local_34 < *(float *)(unaff_EBX + 0x39aceb) || local_34 == *(float *)(unaff_EBX + 0x39aceb))
     || (*(float *)(unaff_EBX + 0x39acbb) <= local_34)) {
    if (*(float *)(unaff_EBX + 0x39acbb) <= local_34 && local_34 != *(float *)(unaff_EBX + 0x39acbb)
       ) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51552f) + 0x38))
                ((int *)**(undefined4 **)(unaff_EBX + 0x51552f),
                 CONCAT22(uVar7,0xffff) & 0xffffff | 0xff0000 | (int)local_2c << 0x18);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51552f) + 0x8c))
                ((int *)**(undefined4 **)(unaff_EBX + 0x51552f),this->m_nTimerUpperSliceTextureID);
      piVar11 = (int *)**(undefined4 **)(unaff_EBX + 0x51552f);
      pcVar8 = *(code **)(*piVar11 + 0x194);
      iVar4 = vgui::Panel::GetTall((Panel *)this);
      iVar5 = vgui::Panel::GetWide((Panel *)this);
      (*pcVar8)(piVar11,puVar9,puVar10,iVar5 / 2 + (int)puVar9,iVar4 / 2 + (int)puVar10,0,0x3f800000
                ,0x3f800000,0);
      puVar6 = puVar10;
    }
  }
  else {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51552f) + 0x38))
              ((int *)**(undefined4 **)(unaff_EBX + 0x51552f),
               CONCAT22(uVar7,0xffff) & 0xffffff | 0xff0000 | (int)local_28 << 0x18);
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51552f) + 0x8c))
              ((int *)**(undefined4 **)(unaff_EBX + 0x51552f),this->m_nTimerUpperSliceTextureID);
    piVar11 = (int *)**(undefined4 **)(unaff_EBX + 0x51552f);
    pcVar8 = *(code **)(*piVar11 + 0x194);
    iVar4 = vgui::Panel::GetTall((Panel *)this);
    iVar5 = vgui::Panel::GetWide((Panel *)this);
    (*pcVar8)(piVar11,puVar9,puVar10,iVar5 / 2 + (int)puVar9,iVar4 / 2 + (int)puVar10,0,0x3f800000,
              0x3f800000,0);
    puVar6 = puVar10;
  }
  if ((local_34 < *(float *)(unaff_EBX + 0x39af9f) || local_34 == *(float *)(unaff_EBX + 0x39af9f))
     || (*(float *)(unaff_EBX + 0x39aceb) <= local_34)) {
    pfVar12 = (float *)(unaff_EBX + 0x39aceb);
    if (*pfVar12 <= local_34 && local_34 != *pfVar12) {
      piVar11 = (int *)**(undefined4 **)(unaff_EBX + 0x51552f);
      pcVar8 = *(code **)(*piVar11 + 0x38);
      uVar7 = (undefined2)((uint)pfVar12 >> 0x10);
      fVar14 = local_2c;
      goto LAB_0061c654;
    }
  }
  else {
    piVar11 = (int *)**(undefined4 **)(unaff_EBX + 0x51552f);
    pcVar8 = *(code **)(*piVar11 + 0x38);
    uVar7 = (undefined2)((uint)puVar6 >> 0x10);
    fVar14 = local_28;
LAB_0061c654:
    (*pcVar8)(piVar11,CONCAT22(uVar7,0xffff) & 0xffffff | 0xff0000 | (int)fVar14 << 0x18);
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51552f) + 0x8c))
              ((int *)**(undefined4 **)(unaff_EBX + 0x51552f),this->m_nTimerUpperSliceTextureID);
    piVar11 = (int *)**(undefined4 **)(unaff_EBX + 0x51552f);
    pcVar8 = *(code **)(*piVar11 + 0x194);
    iVar4 = vgui::Panel::GetTall((Panel *)this);
    (*pcVar8)(piVar11,0,puVar10,puVar9,iVar4 / 2 + (int)puVar10,0x3f800000,0x3f800000,0,0);
  }
  pfVar12 = (float *)(unaff_EBX + 0x39affb);
  if ((local_34 < *pfVar12 || local_34 == *pfVar12) ||
     (*(float *)(unaff_EBX + 0x39af9f) <= local_34)) {
    pfVar12 = (float *)(unaff_EBX + 0x39af9f);
    if (*pfVar12 <= local_34 && local_34 != *pfVar12) {
      piVar11 = (int *)**(undefined4 **)(unaff_EBX + 0x51552f);
      pcVar8 = *(code **)(*piVar11 + 0x38);
      uVar7 = (undefined2)((uint)pfVar12 >> 0x10);
      fVar14 = local_2c;
      goto LAB_0061c584;
    }
  }
  else {
    piVar11 = (int *)**(undefined4 **)(unaff_EBX + 0x51552f);
    pcVar8 = *(code **)(*piVar11 + 0x38);
    uVar7 = (undefined2)((uint)pfVar12 >> 0x10);
    fVar14 = local_28;
LAB_0061c584:
    (*pcVar8)(piVar11,CONCAT22(uVar7,0xffff) & 0xffffff | 0xff0000 | (int)fVar14 << 0x18);
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51552f) + 0x8c))
              ((int *)**(undefined4 **)(unaff_EBX + 0x51552f),this->m_nTimerLowerSliceTextureID);
    piVar11 = (int *)**(undefined4 **)(unaff_EBX + 0x51552f);
    pcVar8 = *(code **)(*piVar11 + 0x194);
    iVar4 = vgui::Panel::GetTall((Panel *)this);
    (*pcVar8)(piVar11,0,puVar10,puVar9,iVar4 / 2 + (int)puVar10,0x3f800000,0x3f800000,0,0);
  }
  pfVar12 = (float *)(unaff_EBX + 0x39abd3);
  if ((local_34 < *pfVar12 || local_34 == *pfVar12) ||
     (*(float *)(unaff_EBX + 0x39affb) <= local_34)) {
    if (local_34 < *(float *)(unaff_EBX + 0x39affb) || local_34 == *(float *)(unaff_EBX + 0x39affb))
    goto LAB_0061c051;
    piVar11 = (int *)**(undefined4 **)(unaff_EBX + 0x51552f);
    pcVar8 = *(code **)(*piVar11 + 0x38);
    fVar14 = local_2c;
  }
  else {
    piVar11 = (int *)**(undefined4 **)(unaff_EBX + 0x51552f);
    pcVar8 = *(code **)(*piVar11 + 0x38);
    fVar14 = local_28;
  }
  (*pcVar8)(piVar11,CONCAT22((short)((uint)pfVar12 >> 0x10),0xffff) & 0xffffff | 0xff0000 |
                    (int)fVar14 << 0x18);
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51552f) + 0x8c))
            ((int *)**(undefined4 **)(unaff_EBX + 0x51552f),this->m_nTimerLowerSliceTextureID);
  pfVar12 = (float *)**(int **)(unaff_EBX + 0x51552f);
  pcVar8 = *(code **)((int)*pfVar12 + 0x194);
  iVar4 = vgui::Panel::GetTall((Panel *)this);
  (*pcVar8)(pfVar12,0,0,puVar9,iVar4 / 2,0x3f800000,0,0,0x3f800000);
LAB_0061c051:
  if ((local_34 < *(float *)(unaff_EBX + 0x39ace3) || local_34 == *(float *)(unaff_EBX + 0x39ace3))
     || (*(float *)(unaff_EBX + 0x39abd3) <= local_34)) {
    if (local_34 < *(float *)(unaff_EBX + 0x39abd3) || local_34 == *(float *)(unaff_EBX + 0x39abd3))
    {
      return;
    }
    piVar11 = (int *)**(undefined4 **)(unaff_EBX + 0x51552f);
    pcVar8 = *(code **)(*piVar11 + 0x38);
  }
  else {
    piVar11 = (int *)**(undefined4 **)(unaff_EBX + 0x51552f);
    pcVar8 = *(code **)(*piVar11 + 0x38);
    local_2c = local_28;
  }
  (*pcVar8)(piVar11,CONCAT22((short)((uint)pfVar12 >> 0x10),0xffff) & 0xffffff | 0xff0000 |
                    (int)local_2c << 0x18);
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51552f) + 0x8c))
            ((int *)**(undefined4 **)(unaff_EBX + 0x51552f),this->m_nTimerUpperSliceTextureID);
  piVar11 = (int *)**(undefined4 **)(unaff_EBX + 0x51552f);
  pcVar8 = *(code **)(*piVar11 + 0x194);
  iVar4 = vgui::Panel::GetTall((Panel *)this);
  (*pcVar8)(piVar11,0,0,puVar9,iVar4 / 2,0x3f800000,0,0,0x3f800000);
  return;
}


/* CIndicatorScreen::PaintIndicator at 0061b9c0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void PaintIndicator(CIndicatorScreen * this) */

void __thiscall CIndicatorScreen::PaintIndicator(CIndicatorScreen *this)

{
  uint uVar1;
  undefined4 *puVar2;
  int *piVar3;
  code *pcVar4;
  int iVar5;
  int iVar6;
  int unaff_EBX;
  
                    /* Unresolved local var: int nTextureID@[???] */
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->m_hScreenEntity).super_CBaseHandle.m_Index;
  iVar5 = DAT_00000a60;
  if ((uVar1 != 0xffffffff) &&
     (iVar6 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5153c2),
     *(uint *)(iVar6 + 8) == uVar1 >> 0x10)) {
    iVar5 = *(int *)(*(int *)(iVar6 + 4) + 0xa60);
  }
  if (iVar5 == 1) {
    iVar5 = this->m_nXTextureID;
  }
  else if (iVar5 == 2) {
    iVar5 = this->m_nOTextureID;
  }
  else {
    if (iVar5 == 0) {
      puVar2 = *(undefined4 **)(unaff_EBX + 0x5157d2);
      piVar3 = (int *)*puVar2;
      (**(code **)(*piVar3 + 0x38))(piVar3,0xff282828);
      piVar3 = (int *)*puVar2;
      pcVar4 = *(code **)(*piVar3 + 0x3c);
      iVar5 = vgui::Panel::GetTall((Panel *)this);
      iVar6 = vgui::Panel::GetWide((Panel *)this);
      (*pcVar4)(piVar3,0,0,iVar6,iVar5);
      return;
    }
    if ((uVar1 == 0xffffffff) ||
       (iVar5 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5153c2),
       *(uint *)(iVar5 + 8) != uVar1 >> 0x10)) {
      iVar5 = 0;
    }
    else {
      iVar5 = *(int *)(iVar5 + 4);
    }
    if (*(char *)(iVar5 + 0xa5d) == '\0') {
      iVar5 = this->m_nUncheckedTextureID;
    }
    else {
      iVar5 = this->m_nCheckTextureID;
    }
  }
  puVar2 = *(undefined4 **)(unaff_EBX + 0x5157d2);
  (**(code **)(*(int *)*puVar2 + 0x38))((int *)*puVar2,0xffffffff);
  (**(code **)(*(int *)*puVar2 + 0x8c))((int *)*puVar2,iVar5);
  piVar3 = (int *)*puVar2;
  pcVar4 = *(code **)(*piVar3 + 0x94);
  iVar5 = vgui::Panel::GetTall((Panel *)this);
  iVar6 = vgui::Panel::GetWide((Panel *)this);
  (*pcVar4)(piVar3,0,0,iVar6,iVar5);
  return;
}


/* CIndicatorScreen::Paint at 0061cbe0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void Paint(CIndicatorScreen * this) */

void __thiscall CIndicatorScreen::Paint(CIndicatorScreen *this)

{
  uint uVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  vgui::Panel::Paint((Panel *)this);
  uVar1 = (this->m_hScreenEntity).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51419f),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) && (iVar2 = *(int *)(iVar2 + 4), iVar2 != 0)) {
    if (*(char *)(iVar2 + 0xa5c) != '\0') {
      PaintCountdownTimer(this);
      return;
    }
    PaintIndicator(this);
    return;
  }
  return;
}


/* __static_initialization_and_destruction_0 at 00082e70 */

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
  VarArgs::string[unaff_EBX + 0xe8] = '\0';
  VarArgs::string[unaff_EBX + 0xe9] = '\0';
  VarArgs::string[unaff_EBX + 0xea] = '\0';
  VarArgs::string[unaff_EBX + 0xeb] = '\0';
  pcVar1 = VarArgs::string + unaff_EBX + 0xec;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = VarArgs::string + unaff_EBX + 0xf0;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = VarArgs::string + unaff_EBX + 0xf4;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = VarArgs::string + unaff_EBX + 0xf8;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = VarArgs::string + unaff_EBX + 0xfc;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = VarArgs::string + unaff_EBX + 0x100;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = VarArgs::string + unaff_EBX + 0x104;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = VarArgs::string + unaff_EBX + 0x108;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = VarArgs::string + unaff_EBX + 0x10c;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = VarArgs::string + unaff_EBX + 0x110;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = VarArgs::string + unaff_EBX + 0x114;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = VarArgs::string + unaff_EBX + 0x118;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  *(undefined **)(VarArgs::string + unaff_EBX + 0x11c) = &UNK_00aaef8c + unaff_EBX;
  *(int *)((int)ice_sbox[1] + unaff_EBX + 0xe9c) = (int)&PTR_ShouldPredict_00b3d88c + unaff_EBX;
  pIVar2 = PanelMetaClassMgr();
  (*pIVar2->_vptr_IPanelMetaClassMgr[1])(pIVar2,unaff_EBX + 0x91d4cd,unaff_EBX + 0xcce75c);
  return;
}


/* _GLOBAL__I_g_CIndicatorScreenFactory at 00082f50 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_g_CIndicatorScreenFactory(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

