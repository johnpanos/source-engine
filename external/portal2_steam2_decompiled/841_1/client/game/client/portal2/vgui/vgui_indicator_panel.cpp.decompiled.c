/* DWARF-guided pseudocode for game/client/portal2/vgui/vgui_indicator_panel.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* CIndicatorScreen::~CIndicatorScreen at 00629320 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CIndicatorScreen(CIndicatorScreen * this, int __in_chrg) */

void __thiscall CIndicatorScreen::~CIndicatorScreen(CIndicatorScreen *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(extraout_ECX + 0x5cc820);
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(*(int *)(extraout_ECX + 0x53b138) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,__in_chrg);
  return;
}


/* CIndicatorScreen::~CIndicatorScreen at 00629350 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CIndicatorScreen(CIndicatorScreen * this, int __in_chrg) */

void __thiscall CIndicatorScreen::~CIndicatorScreen(CIndicatorScreen *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cc7e7);
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x53b0ff) + 8);
  vgui::EditablePanel::~EditablePanel((EditablePanel *)this,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* CIndicatorScreen::CIndicatorScreen at 00627db0 */

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
  piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x53c42c);
  pcVar2 = *(code **)(*piVar1 + 0x34);
  uVar3 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53c6b4) + 8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x53c6b4),2);
  hScheme = (*pcVar2)(piVar1,uVar3,unaff_EBX + 0x3a3703,unaff_EBX + 0x3a36ee);
  CVGuiScreenPanel::CVGuiScreenPanel
            (&this->super_CVGuiScreenPanel,parent,(char *)(unaff_EBX + 0x3a3720),hScheme);
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cdd84);
  (this->m_hVGUIScreen).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hScreenEntity).super_CBaseHandle.m_Index = 0xffffffff;
  this->m_bWasCounting = false;
  this->m_flFadeUpTime = 0.0;
  return;
}


/* CIndicatorScreen::CIndicatorScreen at 00629260 */

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
  piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x53af7c);
  pcVar2 = *(code **)(*piVar1 + 0x34);
  uVar3 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53b204) + 8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x53b204),2);
  hScheme = (*pcVar2)(piVar1,uVar3,unaff_EBX + 0x3a2253,unaff_EBX + 0x3a223e);
  CVGuiScreenPanel::CVGuiScreenPanel
            (&this->super_CVGuiScreenPanel,parent,(char *)(unaff_EBX + 0x3a2270),hScheme);
  (this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
  _vptr_IClientPanel = (_func_int_varargs **)(unaff_EBX + 0x5cc8d4);
  (this->m_hVGUIScreen).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hScreenEntity).super_CBaseHandle.m_Index = 0xffffffff;
  this->m_bWasCounting = false;
  this->m_flFadeUpTime = 0.0;
  return;
}


/* CPanelFactory<CIndicatorScreen,VGuiScreenInitData_t>::Create at 006293a0 */

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
  piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x53ae42);
  pcVar2 = *(code **)(*piVar1 + 0x34);
  uVar4 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53b0ca) + 8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x53b0ca),2);
  hScheme = (*pcVar2)(piVar1,uVar4,unaff_EBX + 0x3a2119,unaff_EBX + 0x3a2104);
  CVGuiScreenPanel::CVGuiScreenPanel(this_00,pParent,(char *)(unaff_EBX + 0x3a2136),hScheme);
  (this_00->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5cc79a);
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
  cVar3 = (**(code **)(&UNK_005ccb4e + unaff_EBX))(this_00,pKeyValues,pVoidInitData);
  if (cVar3 == '\0') {
    (*(this_00->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel[0x21])
              (this_00);
    this_00 = (CVGuiScreenPanel *)0x0;
  }
  return (Panel *)this_00;
}


/* CIndicatorScreen::ApplySchemeSettings at 00627bc0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ApplySchemeSettings(CIndicatorScreen * this, IScheme * pScheme) */

void __thiscall CIndicatorScreen::ApplySchemeSettings(CIndicatorScreen *this,IScheme *pScheme)

{
  return;
}


/* CIndicatorScreen::Init at 00628e90 */

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
  piVar10 = (int *)**(undefined4 **)(unaff_EBX + 0x53b2f2);
  pcVar1 = *(code **)(*piVar10 + 0x54);
  iVar6 = (**(this->super_CVGuiScreenPanel).super_EditablePanel.super_Panel.super_IClientPanel.
             _vptr_IClientPanel)(this);
  (*pcVar1)(piVar10,iVar6,0);
  bVar5 = CVGuiScreenPanel::Init(&this->super_CVGuiScreenPanel,pKeyValues,pInitData);
  if (!bVar5) {
    return false;
  }
  piVar10 = *(int **)(unaff_EBX + 0x53af3e);
  uVar2 = (this->super_CVGuiScreenPanel).m_hEntity.super_CBaseHandle.m_Index;
  if (((uVar2 != 0xffffffff) &&
      (iVar6 = (uVar2 & 0xffff) * 0x10 + *piVar10, *(uint *)(iVar6 + 8) == uVar2 >> 0x10)) &&
     (iVar6 = *(int *)(iVar6 + 4), iVar6 != 0)) {
    uVar4 = *(undefined4 *)(unaff_EBX + 0x53aeda);
    piVar7 = (int *)___dynamic_cast(iVar6,uVar4,*(undefined4 *)(unaff_EBX + 0x53b81a),0);
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
        uVar2 = *(uint *)(iVar9 + 300);
        if (((uVar2 == 0xffffffff) ||
            (iVar6 = (uVar2 & 0xffff) * 0x10 + iVar6, *(uint *)(iVar6 + 8) != uVar2 >> 0x10)) ||
           ((iVar6 = *(int *)(iVar6 + 4), iVar6 == 0 ||
            (piVar10 = (int *)___dynamic_cast(iVar6,uVar4,*(undefined4 *)(unaff_EBX + 0x53b81e),0),
            piVar10 == (int *)0x0)))) {
          (this->m_hScreenEntity).super_CBaseHandle.m_Index = 0xffffffff;
        }
        else {
          puVar11 = (ulong *)(**(code **)(*piVar10 + 0xc))(piVar10);
          (this->m_hScreenEntity).super_CBaseHandle.m_Index = *puVar11;
        }
      }
      goto LAB_00628f1f;
    }
  }
  (this->m_hVGUIScreen).super_CBaseHandle.m_Index = 0xffffffff;
LAB_00628f1f:
  puVar3 = *(undefined4 **)(unaff_EBX + 0x53b356);
  iVar6 = (**(code **)(*(int *)*puVar3 + 0xa4))((int *)*puVar3,0);
  this->m_nTimerBackgroundTextureID = iVar6;
  (**(code **)(*(int *)*puVar3 + 0x8c))((int *)*puVar3,iVar6,unaff_EBX + 0x3a2667,1,0);
  iVar6 = (**(code **)(*(int *)*puVar3 + 0xa4))((int *)*puVar3,0);
  this->m_nTimerBackgroundActiveTextureID = iVar6;
  (**(code **)(*(int *)*puVar3 + 0x8c))((int *)*puVar3,iVar6,unaff_EBX + 0x3a2686,1,0);
  iVar6 = (**(code **)(*(int *)*puVar3 + 0xa4))((int *)*puVar3,0);
  this->m_nTimerUpperSliceTextureID = iVar6;
  (**(code **)(*(int *)*puVar3 + 0x8c))((int *)*puVar3,iVar6,unaff_EBX + 0x3a26a6,1,0);
  iVar6 = (**(code **)(*(int *)*puVar3 + 0xa4))((int *)*puVar3,0);
  this->m_nTimerLowerSliceTextureID = iVar6;
  (**(code **)(*(int *)*puVar3 + 0x8c))((int *)*puVar3,iVar6,unaff_EBX + 0x3a26bf,1,0);
  iVar6 = (**(code **)(*(int *)*puVar3 + 0xa4))((int *)*puVar3,0);
  this->m_nCheckTextureID = iVar6;
  (**(code **)(*(int *)*puVar3 + 0x8c))((int *)*puVar3,iVar6,unaff_EBX + 0x3a26d8,1,0);
  iVar6 = (**(code **)(*(int *)*puVar3 + 0xa4))((int *)*puVar3,0);
  this->m_nUncheckedTextureID = iVar6;
  (**(code **)(*(int *)*puVar3 + 0x8c))((int *)*puVar3,iVar6,unaff_EBX + 0x3a26f2,1,0);
  iVar6 = (**(code **)(*(int *)*puVar3 + 0xa4))((int *)*puVar3,0);
  this->m_nXTextureID = iVar6;
  (**(code **)(*(int *)*puVar3 + 0x8c))((int *)*puVar3,iVar6,unaff_EBX + 0x3a270e,1,0);
  iVar6 = (**(code **)(*(int *)*puVar3 + 0xa4))((int *)*puVar3,0);
  this->m_nOTextureID = iVar6;
  (**(code **)(*(int *)*puVar3 + 0x8c))((int *)*puVar3,iVar6,unaff_EBX + 0x3a271e,1,0);
  return true;
}


/* CIndicatorScreen::OnTick at 00627da0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void OnTick(CIndicatorScreen * this) */

void __thiscall CIndicatorScreen::OnTick(CIndicatorScreen *this)

{
  vgui::Panel::OnTick((Panel *)this);
  return;
}


/* CIndicatorScreen::PaintCountdownTimer at 00627e70 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_LabIndicatorPanel": ignoring multiple overlapping fields */
/* DWARF original prototype: void PaintCountdownTimer(CIndicatorScreen * this) */

void __thiscall CIndicatorScreen::PaintCountdownTimer(CIndicatorScreen *this)

{
  float fVar1;
  uint uVar2;
  int iVar3;
  C_LabIndicatorPanel *this_00;
  int iVar4;
  undefined4 *puVar5;
  undefined2 uVar6;
  code *pcVar7;
  undefined4 *puVar8;
  undefined4 *puVar9;
  int *piVar10;
  int unaff_EBX;
  float *pfVar11;
  longdouble lVar12;
  float fVar13;
  float fVar14;
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
  piVar10 = *(int **)(unaff_EBX + 0x53bf5b);
  uVar2 = (this->m_hScreenEntity).super_CBaseHandle.m_Index;
  if ((uVar2 == 0xffffffff) ||
     (iVar3 = (uVar2 & 0xffff) * 0x10 + *piVar10, *(uint *)(iVar3 + 8) != uVar2 >> 0x10)) {
    this_00 = (C_LabIndicatorPanel *)0x0;
  }
  else {
    this_00 = *(C_LabIndicatorPanel **)(iVar3 + 4);
  }
  local_34 = C_LabIndicatorPanel::GetCountdownPercentage(this_00);
  local_34 = *(float *)(unaff_EBX + 0x3ba78f) * local_34;
  lVar12 = (longdouble)_ceilf(local_34);
  fVar13 = (float)(int)lVar12 - local_34;
  if ((*(float *)(&DAT_003ba3ff + unaff_EBX) <= fVar13) ||
     (NAN(fVar13) || NAN(*(float *)(&DAT_003ba3ff + unaff_EBX)))) {
    local_28 = 0.0;
    if (fVar13 < *(float *)(unaff_EBX + 0x3ba303) || fVar13 == *(float *)(unaff_EBX + 0x3ba303)) {
      local_28 = fVar13 * *(float *)(unaff_EBX + 0x3bd057) + *(float *)(unaff_EBX + 0x3ba6bf);
    }
  }
  else {
    local_28 = *(float *)(unaff_EBX + 0x3ba6bf);
  }
  iVar3 = vgui::Panel::GetWide((Panel *)this);
  puVar8 = (undefined4 *)(iVar3 / 2);
  iVar3 = vgui::Panel::GetTall((Panel *)this);
  puVar9 = (undefined4 *)(iVar3 / 2);
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53c373) + 0x38))
            ((int *)**(undefined4 **)(unaff_EBX + 0x53c373),0xffffffff);
  fVar13 = *(float *)(&DAT_003bb45b + unaff_EBX);
  uVar2 = (this->m_hScreenEntity).super_CBaseHandle.m_Index;
  if ((uVar2 == 0xffffffff) ||
     (iVar3 = (uVar2 & 0xffff) * 0x10 + *piVar10, *(uint *)(iVar3 + 8) != uVar2 >> 0x10)) {
    if (DAT_00000a5f != '\0') goto LAB_00627f98;
LAB_00628380:
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53c373) + 0x94))
              ((int *)**(undefined4 **)(unaff_EBX + 0x53c373),this->m_nTimerBackgroundTextureID);
    piVar10 = (int *)**(undefined4 **)(unaff_EBX + 0x53c373);
    pcVar7 = *(code **)(*piVar10 + 0x9c);
    puVar5 = (undefined4 *)vgui::Panel::GetTall((Panel *)this);
    iVar3 = vgui::Panel::GetWide((Panel *)this);
    (*pcVar7)(piVar10,0,0,iVar3,puVar5);
    if (local_34 > fVar13) goto LAB_00628102;
LAB_00628004:
    if (this->m_bWasCounting == false) {
      fVar13 = this->m_flFadeUpTime;
      fVar14 = fVar13 + *(float *)(unaff_EBX + 0x3ba4ff);
      fVar1 = *(float *)(**(int **)(unaff_EBX + 0x53bf1b) + 0xc);
      if ((fVar13 != fVar14) || (NAN(fVar13) || NAN(fVar14))) {
        fVar13 = (fVar1 - fVar13) / (fVar14 - fVar13);
        local_2c = 0.0;
        if (0.0 <= fVar13) {
          if (fVar13 < *(float *)(unaff_EBX + 0x3ba303) ||
              fVar13 == *(float *)(unaff_EBX + 0x3ba303)) {
            local_2c = fVar13 * *(float *)(unaff_EBX + 0x3ba6bf) +
                       *(float *)(&DAT_003ba3ff + unaff_EBX);
          }
          else {
            local_2c = *(float *)(unaff_EBX + 0x3ba6bf);
          }
        }
      }
      else {
        local_2c = 0.0;
        if (0.0 <= fVar1 - fVar14) {
          local_2c = *(float *)(unaff_EBX + 0x3ba6bf);
        }
      }
      iVar3 = (int)local_2c;
    }
    else {
      this->m_flFadeUpTime = *(float *)(**(int **)(unaff_EBX + 0x53bf1b) + 0xc);
      this->m_bWasCounting = false;
      local_2c = 0.0;
      iVar3 = 0;
    }
    puVar5 = *(undefined4 **)(unaff_EBX + 0x53c373);
    piVar10 = (int *)*puVar5;
    pcVar7 = *(code **)(*piVar10 + 0x38);
    local_34 = 8.0;
LAB_0062841f:
    (*pcVar7)(piVar10,CONCAT22((short)((uint)puVar5 >> 0x10),0xffff) & 0xffffff | 0xff0000 |
                      iVar3 << 0x18);
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53c373) + 0x94))
              ((int *)**(undefined4 **)(unaff_EBX + 0x53c373),this->m_nTimerUpperSliceTextureID);
    piVar10 = (int *)**(undefined4 **)(unaff_EBX + 0x53c373);
    pcVar7 = *(code **)(*piVar10 + 0x9c);
    iVar3 = vgui::Panel::GetTall((Panel *)this);
    iVar4 = vgui::Panel::GetWide((Panel *)this);
    (*pcVar7)(piVar10,puVar8,0,iVar4 / 2 + (int)puVar8,iVar3 / 2);
    puVar5 = puVar8;
  }
  else {
    if (*(char *)(*(int *)(iVar3 + 4) + 0xa5f) == '\0') goto LAB_00628380;
LAB_00627f98:
    if (local_34 <= fVar13) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53c373) + 0x94))
                ((int *)**(undefined4 **)(unaff_EBX + 0x53c373),this->m_nTimerBackgroundTextureID);
      piVar10 = (int *)**(undefined4 **)(unaff_EBX + 0x53c373);
      pcVar7 = *(code **)(*piVar10 + 0x9c);
      iVar3 = vgui::Panel::GetTall((Panel *)this);
      iVar4 = vgui::Panel::GetWide((Panel *)this);
      (*pcVar7)(piVar10,0,0,iVar4,iVar3);
      goto LAB_00628004;
    }
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53c373) + 0x94))
              ((int *)**(undefined4 **)(unaff_EBX + 0x53c373),
               this->m_nTimerBackgroundActiveTextureID);
    piVar10 = (int *)**(undefined4 **)(unaff_EBX + 0x53c373);
    pcVar7 = *(code **)(*piVar10 + 0x9c);
    puVar5 = (undefined4 *)vgui::Panel::GetTall((Panel *)this);
    iVar3 = vgui::Panel::GetWide((Panel *)this);
    (*pcVar7)(piVar10,0,0,iVar3,puVar5);
LAB_00628102:
    this->m_bWasCounting = true;
    if (*(float *)(unaff_EBX + 0x3bb287) <= local_34 && local_34 != *(float *)(unaff_EBX + 0x3bb287)
       ) {
      piVar10 = (int *)**(undefined4 **)(unaff_EBX + 0x53c373);
      pcVar7 = *(code **)(*piVar10 + 0x38);
      iVar3 = (int)local_28;
      local_2c = *(float *)(unaff_EBX + 0x3ba6bf);
      goto LAB_0062841f;
    }
    local_2c = *(float *)(unaff_EBX + 0x3ba6bf);
  }
  uVar6 = (undefined2)((uint)puVar5 >> 0x10);
  if ((local_34 < *(float *)(unaff_EBX + 0x3ba69b) || local_34 == *(float *)(unaff_EBX + 0x3ba69b))
     || (*(float *)(unaff_EBX + 0x3bb287) <= local_34)) {
    if (*(float *)(unaff_EBX + 0x3bb287) <= local_34 && local_34 != *(float *)(unaff_EBX + 0x3bb287)
       ) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53c373) + 0x38))
                ((int *)**(undefined4 **)(unaff_EBX + 0x53c373),
                 CONCAT22(uVar6,0xffff) & 0xffffff | 0xff0000 | (int)local_2c << 0x18);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53c373) + 0x94))
                ((int *)**(undefined4 **)(unaff_EBX + 0x53c373),this->m_nTimerLowerSliceTextureID);
      piVar10 = (int *)**(undefined4 **)(unaff_EBX + 0x53c373);
      pcVar7 = *(code **)(*piVar10 + 0x9c);
      iVar3 = vgui::Panel::GetTall((Panel *)this);
      iVar4 = vgui::Panel::GetWide((Panel *)this);
      (*pcVar7)(piVar10,puVar8,0,iVar4 / 2 + (int)puVar8,iVar3 / 2);
      puVar5 = puVar8;
    }
  }
  else {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53c373) + 0x38))
              ((int *)**(undefined4 **)(unaff_EBX + 0x53c373),
               CONCAT22(uVar6,0xffff) & 0xffffff | 0xff0000 | (int)local_28 << 0x18);
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53c373) + 0x94))
              ((int *)**(undefined4 **)(unaff_EBX + 0x53c373),this->m_nTimerLowerSliceTextureID);
    piVar10 = (int *)**(undefined4 **)(unaff_EBX + 0x53c373);
    pcVar7 = *(code **)(*piVar10 + 0x9c);
    iVar3 = vgui::Panel::GetTall((Panel *)this);
    iVar4 = vgui::Panel::GetWide((Panel *)this);
    (*pcVar7)(piVar10,puVar8,0,iVar4 / 2 + (int)puVar8,iVar3 / 2);
    puVar5 = puVar8;
  }
  uVar6 = (undefined2)((uint)puVar5 >> 0x10);
  if ((local_34 < *(float *)(&DAT_003ba3eb + unaff_EBX) ||
       local_34 == *(float *)(&DAT_003ba3eb + unaff_EBX)) ||
     (*(float *)(unaff_EBX + 0x3ba69b) <= local_34)) {
    if (*(float *)(unaff_EBX + 0x3ba69b) <= local_34 && local_34 != *(float *)(unaff_EBX + 0x3ba69b)
       ) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53c373) + 0x38))
                ((int *)**(undefined4 **)(unaff_EBX + 0x53c373),
                 CONCAT22(uVar6,0xffff) & 0xffffff | 0xff0000 | (int)local_2c << 0x18);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53c373) + 0x94))
                ((int *)**(undefined4 **)(unaff_EBX + 0x53c373),this->m_nTimerLowerSliceTextureID);
      piVar10 = (int *)**(undefined4 **)(unaff_EBX + 0x53c373);
      pcVar7 = *(code **)(*piVar10 + 0x19c);
      iVar3 = vgui::Panel::GetTall((Panel *)this);
      iVar4 = vgui::Panel::GetWide((Panel *)this);
      (*pcVar7)(piVar10,puVar8,puVar9,iVar4 / 2 + (int)puVar8,iVar3 / 2 + (int)puVar9,0,0x3f800000,
                0x3f800000,0);
      puVar5 = puVar9;
    }
  }
  else {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53c373) + 0x38))
              ((int *)**(undefined4 **)(unaff_EBX + 0x53c373),
               CONCAT22(uVar6,0xffff) & 0xffffff | 0xff0000 | (int)local_28 << 0x18);
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53c373) + 0x94))
              ((int *)**(undefined4 **)(unaff_EBX + 0x53c373),this->m_nTimerLowerSliceTextureID);
    piVar10 = (int *)**(undefined4 **)(unaff_EBX + 0x53c373);
    pcVar7 = *(code **)(*piVar10 + 0x19c);
    iVar3 = vgui::Panel::GetTall((Panel *)this);
    iVar4 = vgui::Panel::GetWide((Panel *)this);
    (*pcVar7)(piVar10,puVar8,puVar9,iVar4 / 2 + (int)puVar8,iVar3 / 2 + (int)puVar9,0,0x3f800000,
              0x3f800000,0);
    puVar5 = puVar9;
  }
  uVar6 = (undefined2)((uint)puVar5 >> 0x10);
  if ((local_34 < *(float *)(&DAT_003ba41b + unaff_EBX) ||
       local_34 == *(float *)(&DAT_003ba41b + unaff_EBX)) ||
     (*(float *)(&DAT_003ba3eb + unaff_EBX) <= local_34)) {
    if (*(float *)(&DAT_003ba3eb + unaff_EBX) <= local_34 &&
        local_34 != *(float *)(&DAT_003ba3eb + unaff_EBX)) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53c373) + 0x38))
                ((int *)**(undefined4 **)(unaff_EBX + 0x53c373),
                 CONCAT22(uVar6,0xffff) & 0xffffff | 0xff0000 | (int)local_2c << 0x18);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53c373) + 0x94))
                ((int *)**(undefined4 **)(unaff_EBX + 0x53c373),this->m_nTimerUpperSliceTextureID);
      piVar10 = (int *)**(undefined4 **)(unaff_EBX + 0x53c373);
      pcVar7 = *(code **)(*piVar10 + 0x19c);
      iVar3 = vgui::Panel::GetTall((Panel *)this);
      iVar4 = vgui::Panel::GetWide((Panel *)this);
      (*pcVar7)(piVar10,puVar8,puVar9,iVar4 / 2 + (int)puVar8,iVar3 / 2 + (int)puVar9,0,0x3f800000,
                0x3f800000,0);
      puVar5 = puVar9;
    }
  }
  else {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53c373) + 0x38))
              ((int *)**(undefined4 **)(unaff_EBX + 0x53c373),
               CONCAT22(uVar6,0xffff) & 0xffffff | 0xff0000 | (int)local_28 << 0x18);
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53c373) + 0x94))
              ((int *)**(undefined4 **)(unaff_EBX + 0x53c373),this->m_nTimerUpperSliceTextureID);
    piVar10 = (int *)**(undefined4 **)(unaff_EBX + 0x53c373);
    pcVar7 = *(code **)(*piVar10 + 0x19c);
    iVar3 = vgui::Panel::GetTall((Panel *)this);
    iVar4 = vgui::Panel::GetWide((Panel *)this);
    (*pcVar7)(piVar10,puVar8,puVar9,iVar4 / 2 + (int)puVar8,iVar3 / 2 + (int)puVar9,0,0x3f800000,
              0x3f800000,0);
    puVar5 = puVar9;
  }
  if ((local_34 < *(float *)(unaff_EBX + 0x3ba6cf) || local_34 == *(float *)(unaff_EBX + 0x3ba6cf))
     || (*(float *)(&DAT_003ba41b + unaff_EBX) <= local_34)) {
    pfVar11 = (float *)(&DAT_003ba41b + unaff_EBX);
    if (*pfVar11 <= local_34 && local_34 != *pfVar11) {
      piVar10 = (int *)**(undefined4 **)(unaff_EBX + 0x53c373);
      pcVar7 = *(code **)(*piVar10 + 0x38);
      uVar6 = (undefined2)((uint)pfVar11 >> 0x10);
      fVar13 = local_2c;
      goto LAB_00628864;
    }
  }
  else {
    piVar10 = (int *)**(undefined4 **)(unaff_EBX + 0x53c373);
    pcVar7 = *(code **)(*piVar10 + 0x38);
    uVar6 = (undefined2)((uint)puVar5 >> 0x10);
    fVar13 = local_28;
LAB_00628864:
    (*pcVar7)(piVar10,CONCAT22(uVar6,0xffff) & 0xffffff | 0xff0000 | (int)fVar13 << 0x18);
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53c373) + 0x94))
              ((int *)**(undefined4 **)(unaff_EBX + 0x53c373),this->m_nTimerUpperSliceTextureID);
    piVar10 = (int *)**(undefined4 **)(unaff_EBX + 0x53c373);
    pcVar7 = *(code **)(*piVar10 + 0x19c);
    iVar3 = vgui::Panel::GetTall((Panel *)this);
    (*pcVar7)(piVar10,0,puVar9,puVar8,iVar3 / 2 + (int)puVar9,0x3f800000,0x3f800000,0,0);
  }
  pfVar11 = (float *)(unaff_EBX + 0x3ba72b);
  if ((local_34 < *pfVar11 || local_34 == *pfVar11) ||
     (*(float *)(unaff_EBX + 0x3ba6cf) <= local_34)) {
    pfVar11 = (float *)(unaff_EBX + 0x3ba6cf);
    if (*pfVar11 <= local_34 && local_34 != *pfVar11) {
      piVar10 = (int *)**(undefined4 **)(unaff_EBX + 0x53c373);
      pcVar7 = *(code **)(*piVar10 + 0x38);
      uVar6 = (undefined2)((uint)pfVar11 >> 0x10);
      fVar13 = local_2c;
      goto LAB_00628794;
    }
  }
  else {
    piVar10 = (int *)**(undefined4 **)(unaff_EBX + 0x53c373);
    pcVar7 = *(code **)(*piVar10 + 0x38);
    uVar6 = (undefined2)((uint)pfVar11 >> 0x10);
    fVar13 = local_28;
LAB_00628794:
    (*pcVar7)(piVar10,CONCAT22(uVar6,0xffff) & 0xffffff | 0xff0000 | (int)fVar13 << 0x18);
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53c373) + 0x94))
              ((int *)**(undefined4 **)(unaff_EBX + 0x53c373),this->m_nTimerLowerSliceTextureID);
    piVar10 = (int *)**(undefined4 **)(unaff_EBX + 0x53c373);
    pcVar7 = *(code **)(*piVar10 + 0x19c);
    iVar3 = vgui::Panel::GetTall((Panel *)this);
    (*pcVar7)(piVar10,0,puVar9,puVar8,iVar3 / 2 + (int)puVar9,0x3f800000,0x3f800000,0,0);
  }
  pfVar11 = (float *)(unaff_EBX + 0x3ba303);
  if ((local_34 < *pfVar11 || local_34 == *pfVar11) ||
     (*(float *)(unaff_EBX + 0x3ba72b) <= local_34)) {
    if (local_34 < *(float *)(unaff_EBX + 0x3ba72b) || local_34 == *(float *)(unaff_EBX + 0x3ba72b))
    goto LAB_00628261;
    piVar10 = (int *)**(undefined4 **)(unaff_EBX + 0x53c373);
    pcVar7 = *(code **)(*piVar10 + 0x38);
    fVar13 = local_2c;
  }
  else {
    piVar10 = (int *)**(undefined4 **)(unaff_EBX + 0x53c373);
    pcVar7 = *(code **)(*piVar10 + 0x38);
    fVar13 = local_28;
  }
  (*pcVar7)(piVar10,CONCAT22((short)((uint)pfVar11 >> 0x10),0xffff) & 0xffffff | 0xff0000 |
                    (int)fVar13 << 0x18);
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53c373) + 0x94))
            ((int *)**(undefined4 **)(unaff_EBX + 0x53c373),this->m_nTimerLowerSliceTextureID);
  pfVar11 = (float *)**(int **)(unaff_EBX + 0x53c373);
  pcVar7 = *(code **)((int)*pfVar11 + 0x19c);
  iVar3 = vgui::Panel::GetTall((Panel *)this);
  (*pcVar7)(pfVar11,0,0,puVar8,iVar3 / 2,0x3f800000,0,0,0x3f800000);
LAB_00628261:
  if ((local_34 < *(float *)(&DAT_003ba3ff + unaff_EBX) ||
       local_34 == *(float *)(&DAT_003ba3ff + unaff_EBX)) ||
     (*(float *)(unaff_EBX + 0x3ba303) <= local_34)) {
    if (local_34 < *(float *)(unaff_EBX + 0x3ba303) || local_34 == *(float *)(unaff_EBX + 0x3ba303))
    {
      return;
    }
    piVar10 = (int *)**(undefined4 **)(unaff_EBX + 0x53c373);
    pcVar7 = *(code **)(*piVar10 + 0x38);
  }
  else {
    piVar10 = (int *)**(undefined4 **)(unaff_EBX + 0x53c373);
    pcVar7 = *(code **)(*piVar10 + 0x38);
    local_2c = local_28;
  }
  (*pcVar7)(piVar10,CONCAT22((short)((uint)pfVar11 >> 0x10),0xffff) & 0xffffff | 0xff0000 |
                    (int)local_2c << 0x18);
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x53c373) + 0x94))
            ((int *)**(undefined4 **)(unaff_EBX + 0x53c373),this->m_nTimerUpperSliceTextureID);
  piVar10 = (int *)**(undefined4 **)(unaff_EBX + 0x53c373);
  pcVar7 = *(code **)(*piVar10 + 0x19c);
  iVar3 = vgui::Panel::GetTall((Panel *)this);
  (*pcVar7)(piVar10,0,0,puVar8,iVar3 / 2,0x3f800000,0,0,0x3f800000);
  return;
}


/* CIndicatorScreen::PaintIndicator at 00627bd0 */

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
  iVar5 = DAT_00000a64;
  if ((uVar1 != 0xffffffff) &&
     (iVar6 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x53c1fe),
     *(uint *)(iVar6 + 8) == uVar1 >> 0x10)) {
    iVar5 = *(int *)(*(int *)(iVar6 + 4) + 0xa64);
  }
  if (iVar5 == 1) {
    iVar5 = this->m_nXTextureID;
  }
  else if (iVar5 == 2) {
    iVar5 = this->m_nOTextureID;
  }
  else {
    if (iVar5 == 0) {
      puVar2 = *(undefined4 **)(unaff_EBX + 0x53c616);
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
       (iVar5 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x53c1fe),
       *(uint *)(iVar5 + 8) != uVar1 >> 0x10)) {
      iVar5 = 0;
    }
    else {
      iVar5 = *(int *)(iVar5 + 4);
    }
    if (*(char *)(iVar5 + 0xa61) == '\0') {
      iVar5 = this->m_nUncheckedTextureID;
    }
    else {
      iVar5 = this->m_nCheckTextureID;
    }
  }
  puVar2 = *(undefined4 **)(unaff_EBX + 0x53c616);
  (**(code **)(*(int *)*puVar2 + 0x38))((int *)*puVar2,0xffffffff);
  (**(code **)(*(int *)*puVar2 + 0x94))((int *)*puVar2,iVar5);
  piVar3 = (int *)*puVar2;
  pcVar4 = *(code **)(*piVar3 + 0x9c);
  iVar5 = vgui::Panel::GetTall((Panel *)this);
  iVar6 = vgui::Panel::GetWide((Panel *)this);
  (*pcVar4)(piVar3,0,0,iVar6,iVar5);
  return;
}


/* CIndicatorScreen::Paint at 00628df0 */

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
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x53afdb),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) && (iVar2 = *(int *)(iVar2 + 4), iVar2 != 0)) {
    if (*(char *)(iVar2 + 0xa60) != '\0') {
      PaintCountdownTimer(this);
      return;
    }
    PaintIndicator(this);
    return;
  }
  return;
}


/* __static_initialization_and_destruction_0 at 00081660 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  IPanelMetaClassMgr *pIVar1;
  int unaff_EBX;
  longlong lVar2;
  
  lVar2 = ___i686_get_pc_thunk_bx();
  if (lVar2 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)&g_pClientClassHead + unaff_EBX) = 0;
  *(undefined1 *)((int)&g_pClientClassHead + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&g_pClientClassHead + unaff_EBX + 2) = 0;
  *(undefined1 *)((int)&g_pClientClassHead + unaff_EBX + 3) = 0;
  *(undefined4 *)(&DAT_00c57ffc + unaff_EBX) = 0;
  *(undefined4 *)((int)&g_CurveNameMap[0].type + unaff_EBX) = 0;
  *(undefined4 *)((int)&g_CurveNameMap[0].hotkey + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&g_CurveNameMap[1].type + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&g_CurveNameMap[1].hotkey + unaff_EBX) = 0;
  *(undefined4 *)((int)&g_CurveNameMap[2].type + unaff_EBX) = 0;
  *(undefined4 *)((int)&g_CurveNameMap[2].hotkey + unaff_EBX) = 0;
  *(undefined4 *)((int)&g_CurveNameMap[3].type + unaff_EBX) = 0;
  *(undefined4 *)((int)&g_CurveNameMap[3].hotkey + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&g_CurveNameMap[4].type + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&g_CurveNameMap[4].hotkey + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&g_CurveNameMap[5].type + unaff_EBX) = 0x7f7fffff;
  *(undefined **)((int)&g_CurveNameMap[5].hotkey + unaff_EBX) = &UNK_00ae381c + unaff_EBX;
  *(int *)((int)ice_sbox[3] + unaff_EBX + 0xb4c) =
       (int)&PTR_CreateModelInstance_00b748bc + unaff_EBX;
  pIVar1 = PanelMetaClassMgr();
  (*pIVar1->_vptr_IPanelMetaClassMgr[1])(pIVar1,unaff_EBX + 0x949e89,unaff_EBX + 0xce976c);
  return;
}


/* _GLOBAL__I_g_CIndicatorScreenFactory at 00081740 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_g_CIndicatorScreenFactory(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

