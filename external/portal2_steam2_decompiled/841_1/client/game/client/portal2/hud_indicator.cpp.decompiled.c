/* DWARF-guided pseudocode for game/client/portal2/hud_indicator.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* CHudIndicator::~CHudIndicator at 0060e630 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void ~CHudIndicator(CHudIndicator * this, int __in_chrg) */

void __thiscall CHudIndicator::~CHudIndicator(CHudIndicator *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5e3aa4);
  (this->super_CHudElement).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(unaff_EBX + 0x5e3e6c);
  CHudElement::~CHudElement(&this->super_CHudElement,in_stack_ffffffd8);
  vgui::EditablePanel::~EditablePanel(&this->super_EditablePanel,in_stack_ffffffd8);
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x55572c) + 8))
            ((int *)**(undefined4 **)(unaff_EBX + 0x55572c),this);
  return;
}


/* CHudIndicator::~CHudIndicator at 0060e6c0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void ~CHudIndicator(CHudIndicator * this, int __in_chrg) */

void __thiscall CHudIndicator::~CHudIndicator(CHudIndicator *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5e3a14);
  (this->super_CHudElement).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(unaff_EBX + 0x5e3ddc);
  CHudElement::~CHudElement(&this->super_CHudElement,in_stack_ffffffd8);
  vgui::EditablePanel::~EditablePanel(&this->super_EditablePanel,__in_chrg);
  return;
}


/* CHudIndicator::CHudIndicator at 0060dd30 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void CHudIndicator(CHudIndicator * this, char * pElementName) */

void __thiscall CHudIndicator::CHudIndicator(CHudIndicator *this,char *pElementName)

{
  char cVar1;
  PanelKeyBindingMap *pPVar2;
  PanelKeyBindingMap *pPVar3;
  IClientMode *pIVar4;
  Panel *newParent;
  PanelMessageMap *pPVar5;
  PanelMessageMap *pPVar6;
  PanelAnimationMap *pPVar7;
  PanelAnimationMap *pPVar8;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  vgui::EditablePanel::EditablePanel
            (&this->super_EditablePanel,(Panel *)0x0,&UNK_003bc28d + unaff_EBX);
  CHudElement::CHudElement(&this->super_CHudElement,pElementName);
  (this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5e43aa);
  (this->super_CHudElement).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(unaff_EBX + 0x5e4772);
  if (*(char *)(unaff_EBX + 0x64d025) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64d025) = 1;
    pPVar5 = vgui::FindOrAddPanelMessageMap(&UNK_003bc1cc + unaff_EBX);
    pPVar5->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61fcba);
    pPVar6 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x38d2d6));
    pPVar5->baseMap = pPVar6;
    cVar1 = *(char *)(unaff_EBX + 0x64d026);
  }
  else {
    cVar1 = *(char *)(unaff_EBX + 0x64d026);
  }
  if (cVar1 == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64d026) = 1;
    pPVar7 = FindOrAddPanelAnimationMap(&UNK_003bc1cc + unaff_EBX);
    pPVar7->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61fcba);
    pPVar8 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x38d2d6));
    pPVar7->baseMap = pPVar8;
  }
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
  if (*(char *)(unaff_EBX + 0x64d027) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64d027) = 1;
    pPVar2 = vgui::FindOrAddPanelKeyBindingMap(&UNK_003bc1cc + unaff_EBX);
    pPVar2->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61fcba);
    pPVar3 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x38d2d6));
    pPVar2->baseMap = pPVar3;
  }
                    /* Unresolved local var: Panel * pParent@[???] */
  pIVar4 = GetClientMode();
  newParent = (Panel *)(*pIVar4->_vptr_IClientMode[10])(pIVar4);
  vgui::Panel::SetParent((Panel *)this,newParent);
  CHudElement::SetHiddenBits(&this->super_CHudElement,0x10);
  vgui::EditablePanel::LoadControlSettings
            (&this->super_EditablePanel,&UNK_003bc207 + unaff_EBX,(char *)0x0,(KeyValues *)0x0,
             (KeyValues *)0x0);
  this->m_nTexture[0] = -1;
  this->m_nTexture[1] = -1;
  this->m_nIndicatorType = 0;
  return;
}


/* CHudIndicator::GetPanelClassName at 0060e100 */

char * CHudIndicator::GetPanelClassName(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (char *)(extraout_ECX + 0x3bbe02);
}


/* CHudIndicator::KB_ChainToMap at 0060e200 */

void CHudIndicator::KB_ChainToMap(void)

{
  PanelKeyBindingMap *pPVar1;
  PanelKeyBindingMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x64cb54) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64cb54) = 1;
    pPVar1 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3bbcf9));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61f7e7);
    pPVar2 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x38ce03));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CHudIndicator::ChainToAnimationMap at 0060e2c0 */

void CHudIndicator::ChainToAnimationMap(void)

{
  PanelAnimationMap *pPVar1;
  PanelAnimationMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelAnimationMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&DAT_0064ca93)[unaff_EBX] == '\0') {
    (&DAT_0064ca93)[unaff_EBX] = 1;
    pPVar1 = FindOrAddPanelAnimationMap(&UNK_003bbc39 + unaff_EBX);
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61f727);
    pPVar2 = FindOrAddPanelAnimationMap((char *)(C_TEBeamFollow::GetClientClass + unaff_EBX + 3));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CHudIndicator::ChainToMap at 0060e380 */

void CHudIndicator::ChainToMap(void)

{
  PanelMessageMap *pPVar1;
  PanelMessageMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelMessageMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x64c9d2) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64c9d2) = 1;
    pPVar1 = vgui::FindOrAddPanelMessageMap(&UNK_003bbb79 + unaff_EBX);
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61f667);
    pPVar2 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x38cc83));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CHudIndicator::GetKBMap at 0060e500 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: PanelKeyBindingMap * GetKBMap(CHudIndicator * this) */

PanelKeyBindingMap * __thiscall CHudIndicator::GetKBMap(CHudIndicator *this)

{
  char *pcVar1;
  int iVar2;
  PanelKeyBindingMap *pPVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  pcVar1 = *(char **)(unaff_EBX + 0x61f4e0);
  if (*pcVar1 == '\0') {
    iVar2 = ___cxa_guard_acquire(pcVar1);
    if (iVar2 != 0) {
      pPVar3 = vgui::FindOrAddPanelKeyBindingMap(&UNK_003bb9f6 + unaff_EBX);
      *(PanelKeyBindingMap **)(unaff_EBX + 0x64c83c) = pPVar3;
      ___cxa_guard_release(pcVar1);
    }
  }
  return *(PanelKeyBindingMap **)(unaff_EBX + 0x64c83c);
}


/* CHudIndicator::GetAnimMap at 0060e580 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: PanelAnimationMap * GetAnimMap(CHudIndicator * this) */

PanelAnimationMap * __thiscall CHudIndicator::GetAnimMap(CHudIndicator *this)

{
  PanelAnimationMap *pPVar1;
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  pPVar1 = FindOrAddPanelAnimationMap(&UNK_003bb982 + extraout_ECX);
  return pPVar1;
}


/* CHudIndicator::GetMessageMap at 0060e5a0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: PanelMessageMap * GetMessageMap(CHudIndicator * this) */

PanelMessageMap * __thiscall CHudIndicator::GetMessageMap(CHudIndicator *this)

{
  char *pcVar1;
  int iVar2;
  PanelMessageMap *pPVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  pcVar1 = *(char **)(unaff_EBX + 0x61f438);
  if (*pcVar1 == '\0') {
    iVar2 = ___cxa_guard_acquire(pcVar1);
    if (iVar2 != 0) {
      pPVar3 = vgui::FindOrAddPanelMessageMap(&UNK_003bb956 + unaff_EBX);
      *(PanelMessageMap **)(unaff_EBX + 0x64c7a0) = pPVar3;
      ___cxa_guard_release(pcVar1);
    }
  }
  return *(PanelMessageMap **)(unaff_EBX + 0x64c7a0);
}


/* __MsgFunc_CHudIndicator_IndicatorFlash at 0060cf30 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */

void __MsgFunc_CHudIndicator_IndicatorFlash(bf_read *msg)

{
  CHud *this;
  CHudElement *pCVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: CHudElement * pElement@[???] */
  ___i686_get_pc_thunk_bx();
  this = GetHud(-1);
  pCVar1 = CHud::FindElement(this,(char *)(unaff_EBX + 0x3bcfce));
  if (pCVar1 != (CHudElement *)0x0) {
    CHudIndicator::MsgFunc_IndicatorFlash
              ((CHudIndicator *)&pCVar1[-8].m_HudRenderGroups.m_Memory.m_nGrowSize,msg);
  }
  return;
}


/* CHudIndicator::CHudIndicator at 0060df00 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void CHudIndicator(CHudIndicator * this, char * pElementName,
   CHudIndicator * this, char * pElementName) */

void __thiscall
CHudIndicator::CHudIndicator
          (CHudIndicator *this,char *pElementName,CHudIndicator *this_1,char *pElementName_1)

{
  CHudIndicator(this,pElementName);
  return;
}


/* CHudIndicator::Init at 0060d1b0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void Init(CHudIndicator * this) */

void __thiscall CHudIndicator::Init(CHudIndicator *this)

{
  undefined4 *puVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CUserMessages::HookMessage
            ((CUserMessages *)**(undefined4 **)(unaff_EBX + 0x556bee),(char *)(unaff_EBX + 0x385310)
             ,(pfnUserMsgHook)(unaff_EBX + -0x28e));
  puVar1 = *(undefined4 **)(unaff_EBX + 0x557036);
  iVar2 = (**(code **)(*(int *)*puVar1 + 0xa4))((int *)*puVar1,0);
  this->m_nTexture[0] = iVar2;
  (**(code **)(*(int *)*puVar1 + 0x8c))((int *)*puVar1,iVar2,unaff_EBX + 0x3bcddc,1,0);
  iVar2 = (**(code **)(*(int *)*puVar1 + 0xa4))((int *)*puVar1,0);
  this->m_nTexture[1] = iVar2;
  (**(code **)(*(int *)*puVar1 + 0x8c))((int *)*puVar1,iVar2,unaff_EBX + 0x3bcde8,1,0);
  this->m_flDisplayTime = 0.0;
  this->m_flStartTime = 0.0;
  this->m_flFadeInTime = 0.0;
  this->m_flFadeOutTime = 0.0;
  return;
}


/* CHudIndicator::Reset at 0060c450 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void Reset(CHudIndicator * this) */

void __thiscall CHudIndicator::Reset(CHudIndicator *this)

{
                    /* WARNING: Could not recover jumptable at 0x0060c45f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel[0xed])();
  return;
}


/* CHudIndicator::ApplySchemeSettings at 0060cf80 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void ApplySchemeSettings(CHudIndicator * this, IScheme * scheme) */

void __thiscall CHudIndicator::ApplySchemeSettings(CHudIndicator *this,IScheme *scheme)

{
  int unaff_EBX;
  int local_14;
  int local_10;
  
                    /* Unresolved local var: int screenWide@[???]
                       Unresolved local var: int screenTall@[???]
                       Unresolved local var: int nCenterX@[???]
                       Unresolved local var: int nCenterY@[???] */
  ___i686_get_pc_thunk_bx();
  (*(this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel[0xd4])
            (this,unaff_EBX + 0x3bcfb8,0,0,0);
  vgui::Panel::ApplySchemeSettings((Panel *)this,scheme);
  (*(this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel[0x48])(this,0);
  (*(this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel[0x47])(this,0);
  GetHudSize(&local_10,&local_14);
  vgui::Panel::SetBounds((Panel *)this,local_10 / 2 + -0x80,local_14 / 2 + -0x80,0x100,0x100);
  return;
}


/* CHudIndicator::ShouldDraw at 0060c480 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: bool ShouldDraw(CHudIndicator * this) */

bool __thiscall CHudIndicator::ShouldDraw(CHudIndicator *this)

{
  float *pfVar1;
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  pfVar1 = (float *)(**(int **)(extraout_ECX + 0x557914) + 0xc);
  return *pfVar1 <= this->m_flDisplayTime && this->m_flDisplayTime != *pfVar1;
}


/* CHudIndicator::Paint at 0060d2a0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void Paint(CHudIndicator * this) */

void __thiscall CHudIndicator::Paint(CHudIndicator *this)

{
  undefined4 *puVar1;
  int iVar2;
  int unaff_EBX;
  longdouble lVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  int local_a0;
  float local_74;
  float local_5c;
  float local_58;
  undefined4 local_54;
  undefined4 local_50;
  float local_4c;
  float local_48;
  undefined4 local_44;
  undefined4 local_40;
  float local_3c;
  float local_38;
  undefined4 local_34;
  undefined4 local_30;
  float local_2c;
  float local_28;
  undefined4 local_24;
  undefined4 local_20;
  
                    /* Unresolved local var: float flGlobalScale@[???]
                       Unresolved local var: float flGlobalAlpha@[???]
                       Unresolved local var: float flBump@[???]
                       Unresolved local var: float flScale@[???]
                       Unresolved local var: float flOffset@[???]
                       Unresolved local var: float xLeft@[???]
                       Unresolved local var: float xRight@[???]
                       Unresolved local var: float yTop@[???]
                       Unresolved local var: float yBottom@[???]
                       Unresolved local var: int nShadowDepth@[???]
                       Unresolved local var: Vertex_t[4] vert@[???] */
  ___i686_get_pc_thunk_bx();
  fVar4 = *(float *)(**(int **)(unaff_EBX + 0x556aeb) + 0xc);
  fVar5 = this->m_flFadeInTime;
  if (fVar4 < fVar5) {
    fVar6 = this->m_flStartTime;
    if ((fVar5 != fVar6) || (NAN(fVar5) || NAN(fVar6))) {
      fVar5 = (fVar4 - fVar6) / (fVar5 - fVar6);
      local_74 = *(float *)(&DAT_003d4ed3 + unaff_EBX);
      if (fVar5 <= local_74) {
        fVar6 = 0.0;
        if (0.0 <= fVar5) {
          fVar6 = fVar5;
        }
        local_74 = fVar6 * fVar6 * *(float *)(unaff_EBX + 0x3d529f) +
                   fVar6 * fVar6 * fVar6 * *(float *)(unaff_EBX + 0x3d533b) + 0.0;
      }
    }
    else if (fVar4 < fVar5) {
      local_74 = 0.0;
    }
    else {
      local_74 = *(float *)(&DAT_003d4ed3 + unaff_EBX);
    }
    fVar5 = *(float *)(&DAT_003d5e87 + unaff_EBX);
    local_a0 = (int)(local_74 * fVar5);
    goto LAB_0060d370;
  }
  fVar5 = this->m_flFadeOutTime;
  if (fVar4 <= fVar5) {
    local_74 = *(float *)(&DAT_003d4ed3 + unaff_EBX);
    local_a0 = 0x80;
    fVar5 = *(float *)(&DAT_003d5e87 + unaff_EBX);
    goto LAB_0060d370;
  }
  fVar6 = this->m_flDisplayTime;
  if ((fVar5 != fVar6) || (NAN(fVar5) || NAN(fVar6))) {
    fVar5 = (fVar4 - fVar5) / (fVar6 - fVar5);
    if (*(float *)(&DAT_003d4ed3 + unaff_EBX) < fVar5) goto LAB_0060d749;
    fVar6 = 0.0;
    if (0.0 <= fVar5) {
      fVar6 = fVar5;
    }
    local_74 = *(float *)(&DAT_003d4ed3 + unaff_EBX) -
               (fVar6 * fVar6 * *(float *)(unaff_EBX + 0x3d529f) +
               fVar6 * fVar6 * fVar6 * *(float *)(unaff_EBX + 0x3d533b));
  }
  else if (fVar6 <= fVar4) {
LAB_0060d749:
    local_74 = 0.0;
  }
  else {
    local_74 = *(float *)(&DAT_003d4ed3 + unaff_EBX);
  }
  fVar5 = *(float *)(&DAT_003d5e87 + unaff_EBX);
  local_a0 = (int)(local_74 * fVar5);
LAB_0060d370:
  lVar3 = (longdouble)_sinf(fVar4 * *(float *)(&DAT_003d4f67 + unaff_EBX));
  fVar4 = (fVar5 + local_74 * (float)lVar3 * *(float *)(unaff_EBX + 0x3d4fd3) * local_74) *
          *(float *)(&DAT_003d4f5f + unaff_EBX);
  puVar1 = *(undefined4 **)(unaff_EBX + 0x556f43);
  (**(code **)(*(int *)*puVar1 + 0x94))((int *)*puVar1,this->m_nTexture[this->m_nIndicatorType]);
  iVar2 = vgui::Panel::GetWide((Panel *)this);
  fVar5 = (float)(iVar2 / 2) - fVar4;
  iVar2 = vgui::Panel::GetWide((Panel *)this);
  fVar7 = (float)(iVar2 / 2) + fVar4;
  iVar2 = vgui::Panel::GetTall((Panel *)this);
  fVar6 = (float)(iVar2 / 2) - fVar4;
  iVar2 = vgui::Panel::GetTall((Panel *)this);
  fVar4 = (float)(iVar2 / 2) + fVar4;
  (**(code **)(*(int *)*puVar1 + 0x34))((int *)*puVar1,0,0,0,local_a0);
  local_38 = *(float *)(unaff_EBX + 0x3d4feb);
  local_58 = fVar6 + local_38;
  local_5c = fVar5 + local_38;
  local_54 = 0;
  local_50 = 0;
  local_4c = fVar7 + local_38;
  local_44 = 0x3f800000;
  local_40 = 0;
  local_38 = local_38 + fVar4;
  local_34 = 0x3f800000;
  local_30 = 0x3f800000;
  local_24 = 0;
  local_20 = 0x3f800000;
  local_48 = local_58;
  local_3c = local_4c;
  local_2c = local_5c;
  local_28 = local_38;
  (**(code **)(*(int *)*puVar1 + 0x1a0))((int *)*puVar1,4,&local_5c,1);
  (**(code **)(*(int *)*puVar1 + 0x34))
            ((int *)*puVar1,0xff,0xff,0xff,(int)(local_74 * *(float *)(unaff_EBX + 0x3d528f)));
  local_54 = 0;
  local_50 = 0;
  local_44 = 0x3f800000;
  local_40 = 0;
  local_34 = 0x3f800000;
  local_30 = 0x3f800000;
  local_24 = 0;
  local_20 = 0x3f800000;
  local_5c = fVar5;
  local_58 = fVar6;
  local_4c = fVar7;
  local_48 = fVar6;
  local_3c = fVar7;
  local_38 = fVar4;
  local_2c = fVar5;
  local_28 = fVar4;
  (**(code **)(*(int *)*puVar1 + 0x1a0))((int *)*puVar1,4,&local_5c,1);
  vgui::Panel::Paint((Panel *)this);
  return;
}


/* CHudIndicator::MsgFunc_IndicatorFlash at 0060c4b0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void MsgFunc_IndicatorFlash(CHudIndicator * this, bf_read * msg) */

void __thiscall CHudIndicator::MsgFunc_IndicatorFlash(CHudIndicator *this,bf_read *msg)

{
  uint32 **ppuVar1;
  uint32 *puVar2;
  int iVar3;
  uint32 *puVar4;
  uint uVar5;
  int *piVar6;
  uint uVar7;
  uint uVar8;
  int iVar9;
  int unaff_EBX;
  float fVar10;
  float fVar11;
  byte local_28;
  
  ___i686_get_pc_thunk_bx();
  iVar3 = (msg->super_CBitRead).m_nBitsAvail;
  if (iVar3 < 8) {
    uVar7 = (msg->super_CBitRead).m_nInBufWord;
    puVar4 = (msg->super_CBitRead).m_pDataIn;
    puVar2 = (msg->super_CBitRead).m_pBufferEnd;
    if (puVar4 == puVar2) {
      (msg->super_CBitRead).m_nBitsAvail = 1;
      (msg->super_CBitRead).m_nInBufWord = 0;
      ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
      *ppuVar1 = *ppuVar1 + 1;
      (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
      uVar8 = 0;
    }
    else {
      if (puVar2 < puVar4) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar4;
        (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
      }
      uVar8 = 0;
      if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
        iVar9 = 8 - iVar3;
        uVar8 = (msg->super_CBitRead).m_nInBufWord;
        uVar5 = *(uint *)(*(int *)(unaff_EBX + 0x5578ea) + iVar9 * 4);
        (msg->super_CBitRead).m_nBitsAvail = 0x20 - iVar9;
        local_28 = (byte)iVar9;
        puVar2 = &(msg->super_CBitRead).m_nInBufWord;
        *puVar2 = *puVar2 >> (local_28 & 0x1f);
        uVar8 = (uVar8 & uVar5) << ((byte)iVar3 & 0x1f) | uVar7;
      }
    }
  }
  else {
    uVar8 = (msg->super_CBitRead).m_nInBufWord & *(uint *)(*(int *)(unaff_EBX + 0x5578ea) + 0x20);
    (msg->super_CBitRead).m_nBitsAvail = iVar3 + -8;
    if (iVar3 + -8 == 0) {
      (msg->super_CBitRead).m_nBitsAvail = 0x20;
      puVar4 = (msg->super_CBitRead).m_pDataIn;
      puVar2 = (msg->super_CBitRead).m_pBufferEnd;
      if (puVar4 == puVar2) {
        (msg->super_CBitRead).m_nBitsAvail = 1;
        (msg->super_CBitRead).m_nInBufWord = 0;
        ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
        *ppuVar1 = *ppuVar1 + 1;
      }
      else if (puVar2 < puVar4) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar4;
        (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
      }
    }
    else {
      puVar2 = &(msg->super_CBitRead).m_nInBufWord;
      *puVar2 = *puVar2 >> 8;
    }
  }
  this->m_nIndicatorType = uVar8;
  piVar6 = *(int **)(unaff_EBX + 0x5578de);
  this->m_flStartTime = *(float *)(*piVar6 + 0xc);
  fVar11 = *(float *)(*piVar6 + 0xc);
  iVar3 = (msg->super_CBitRead).m_nBitsAvail;
  if (iVar3 < 0x20) {
    uVar7 = (msg->super_CBitRead).m_nInBufWord;
    puVar4 = (msg->super_CBitRead).m_pDataIn;
    puVar2 = (msg->super_CBitRead).m_pBufferEnd;
    if (puVar4 == puVar2) {
      (msg->super_CBitRead).m_nBitsAvail = 1;
      (msg->super_CBitRead).m_nInBufWord = 0;
      ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
      *ppuVar1 = *ppuVar1 + 1;
      (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
      fVar10 = 0.0;
    }
    else {
      if (puVar2 < puVar4) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar4;
        (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
      }
      fVar10 = 0.0;
      if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
        iVar9 = 0x20 - iVar3;
        fVar10 = (float)(((msg->super_CBitRead).m_nInBufWord &
                         *(uint *)(*(int *)(unaff_EBX + 0x5578ea) + iVar9 * 4)) <<
                         ((byte)iVar3 & 0x1f) | uVar7);
        (msg->super_CBitRead).m_nBitsAvail = 0x20 - iVar9;
        local_28 = (byte)iVar9;
        puVar2 = &(msg->super_CBitRead).m_nInBufWord;
        *puVar2 = *puVar2 >> (local_28 & 0x1f);
      }
    }
  }
  else {
    uVar7 = (msg->super_CBitRead).m_nInBufWord;
    uVar8 = *(uint *)(*(int *)(unaff_EBX + 0x5578ea) + 0x80);
    (msg->super_CBitRead).m_nBitsAvail = iVar3 + -0x20;
    if (iVar3 + -0x20 == 0) {
      (msg->super_CBitRead).m_nBitsAvail = 0x20;
      puVar4 = (msg->super_CBitRead).m_pDataIn;
      puVar2 = (msg->super_CBitRead).m_pBufferEnd;
      if (puVar4 == puVar2) {
        (msg->super_CBitRead).m_nBitsAvail = 1;
        (msg->super_CBitRead).m_nInBufWord = 0;
        ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
        *ppuVar1 = *ppuVar1 + 1;
      }
      else if (puVar2 < puVar4) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar4;
        (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
      }
    }
    else {
      puVar2 = &(msg->super_CBitRead).m_nInBufWord;
      *puVar2 = *puVar2;
    }
    fVar10 = (float)(uVar7 & uVar8);
  }
  fVar11 = fVar11 + fVar10;
  this->m_flDisplayTime = fVar11;
  fVar10 = *(float *)(unaff_EBX + 0x3d5dd2);
  this->m_flFadeInTime = *(float *)(**(int **)(unaff_EBX + 0x5578de) + 0xc) + fVar10;
  this->m_flFadeOutTime = fVar11 - fVar10;
  return;
}


/* __static_initialization_and_destruction_0 at 000806d0 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *unaff_EBX;
  char *unaff_EBP;
  longlong lVar1;
  int unaff_retaddr;
  ConVar *in_stack_fffffff4;
  char *pName_1;
  
  pName_1 = unaff_EBX;
  lVar1 = ___i686_get_pc_thunk_bx();
  if (lVar1 != 0xffff00000001) {
    return;
  }
  unaff_EBX[0xc58a44] = '\0';
  unaff_EBX[0xc58a45] = '\0';
  unaff_EBX[0xc58a46] = '\0';
  unaff_EBX[0xc58a47] = '\0';
  unaff_EBX[0xc58a48] = '\0';
  unaff_EBX[0xc58a49] = '\0';
  unaff_EBX[0xc58a4a] = '\0';
  unaff_EBX[0xc58a4b] = '\0';
  unaff_EBX[0xc58a4c] = '\0';
  unaff_EBX[0xc58a4d] = '\0';
  unaff_EBX[0xc58a4e] = '\0';
  unaff_EBX[0xc58a4f] = '\0';
  unaff_EBX[0xc58a50] = -1;
  unaff_EBX[0xc58a51] = -1;
  unaff_EBX[0xc58a52] = '\x7f';
  unaff_EBX[0xc58a53] = '\x7f';
  unaff_EBX[0xc58a54] = -1;
  unaff_EBX[0xc58a55] = -1;
  unaff_EBX[0xc58a56] = '\x7f';
  unaff_EBX[0xc58a57] = '\x7f';
  unaff_EBX[0xc58a58] = '\0';
  unaff_EBX[0xc58a59] = '\0';
  unaff_EBX[0xc58a5a] = '\0';
  unaff_EBX[0xc58a5b] = '\0';
  unaff_EBX[0xc58a5c] = '\0';
  unaff_EBX[0xc58a5d] = '\0';
  unaff_EBX[0xc58a5e] = '\0';
  unaff_EBX[0xc58a5f] = '\0';
  unaff_EBX[0xc58a60] = '\0';
  unaff_EBX[0xc58a61] = '\0';
  unaff_EBX[0xc58a62] = '\0';
  unaff_EBX[0xc58a63] = '\0';
  unaff_EBX[0xc58a64] = '\0';
  unaff_EBX[0xc58a65] = '\0';
  unaff_EBX[0xc58a66] = '\0';
  unaff_EBX[0xc58a67] = '\0';
  unaff_EBX[0xc58a68] = -1;
  unaff_EBX[0xc58a69] = -1;
  unaff_EBX[0xc58a6a] = '\x7f';
  unaff_EBX[0xc58a6b] = '\x7f';
  unaff_EBX[0xc58a6c] = -1;
  unaff_EBX[0xc58a6d] = -1;
  unaff_EBX[0xc58a6e] = '\x7f';
  unaff_EBX[0xc58a6f] = '\x7f';
  unaff_EBX[0xc58a70] = -1;
  unaff_EBX[0xc58a71] = -1;
  unaff_EBX[0xc58a72] = '\x7f';
  unaff_EBX[0xc58a73] = '\x7f';
  unaff_EBX[0xc58a74] = -1;
  unaff_EBX[0xc58a75] = -1;
  unaff_EBX[0xc58a76] = '\x7f';
  unaff_EBX[0xc58a77] = '\x7f';
  *(char **)(unaff_EBX + 0xc58a78) = unaff_EBX + 0xae47ac;
  ConVar::ConVar((ConVar *)(unaff_EBX + 0xcea284),unaff_EBX + 0x9498d7,unaff_EBX + 0x9027e0,0,
                 in_stack_fffffff4,pName_1,unaff_EBP,unaff_retaddr);
  ___cxa_atexit(unaff_EBX + 0x8fc8e4,0,*(undefined4 *)(unaff_EBX + 0xae3690));
  return;
}


/* __tcf_0 at 0097cfc0 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ed998),in_stack_00000008);
  return;
}


/* CHudControlHelper::~CHudControlHelper at 0060e130 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void ~CHudControlHelper(CHudControlHelper * this, int __in_chrg) */

void __thiscall CHudControlHelper::~CHudControlHelper(CHudControlHelper *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5e3b84);
  (this->super_CHudElement).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(unaff_EBX + 0x5e3f4c);
  CHudElement::~CHudElement(&this->super_CHudElement,in_stack_ffffffd8);
  vgui::EditablePanel::~EditablePanel(&this->super_EditablePanel,__in_chrg);
  return;
}


/* CHudControlHelper::~CHudControlHelper at 0060e740 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void ~CHudControlHelper(CHudControlHelper * this, int __in_chrg) */

void __thiscall CHudControlHelper::~CHudControlHelper(CHudControlHelper *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5e3574);
  (this->super_CHudElement).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(unaff_EBX + 0x5e393c);
  CHudElement::~CHudElement(&this->super_CHudElement,in_stack_ffffffd8);
  vgui::EditablePanel::~EditablePanel(&this->super_EditablePanel,in_stack_ffffffd8);
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x55561c) + 8))
            ((int *)**(undefined4 **)(unaff_EBX + 0x55561c),this);
  return;
}


/* CHudControlHelper::CHudControlHelper at 0060df10 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void CHudControlHelper(CHudControlHelper * this, char * pElementName)
    */

void __thiscall CHudControlHelper::CHudControlHelper(CHudControlHelper *this,char *pElementName)

{
  char cVar1;
  PanelKeyBindingMap *pPVar2;
  PanelKeyBindingMap *pPVar3;
  IClientMode *pIVar4;
  Panel *newParent;
  PanelMessageMap *pPVar5;
  PanelMessageMap *pPVar6;
  PanelAnimationMap *pPVar7;
  PanelAnimationMap *pPVar8;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  vgui::EditablePanel::EditablePanel
            (&this->super_EditablePanel,(Panel *)0x0,(char *)(unaff_EBX + 0x3bc0ba));
  CHudElement::CHudElement(&this->super_CHudElement,pElementName);
  (this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5e3daa);
  (this->super_CHudElement).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(unaff_EBX + 0x5e4172);
  if (*(char *)(unaff_EBX + 0x64ce42) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64ce42) = 1;
    pPVar5 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3bbffa));
    pPVar5->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61fade);
    pPVar6 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x38d0f6));
    pPVar5->baseMap = pPVar6;
    cVar1 = *(char *)(unaff_EBX + 0x64ce43);
  }
  else {
    cVar1 = *(char *)(unaff_EBX + 0x64ce43);
  }
  if (cVar1 == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64ce43) = 1;
    pPVar7 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x3bbffa));
    pPVar7->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61fade);
    pPVar8 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x38d0f6));
    pPVar7->baseMap = pPVar8;
  }
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
  if (*(char *)(unaff_EBX + 0x64ce44) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64ce44) = 1;
    pPVar2 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3bbffa));
    pPVar2->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61fade);
    pPVar3 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x38d0f6));
    pPVar2->baseMap = pPVar3;
  }
                    /* Unresolved local var: Panel * pParent@[???] */
  pIVar4 = GetClientMode();
  newParent = (Panel *)(*pIVar4->_vptr_IClientMode[10])(pIVar4);
  vgui::Panel::SetParent((Panel *)this,newParent);
  CHudElement::SetHiddenBits(&this->super_CHudElement,0x10);
  vgui::EditablePanel::LoadControlSettings
            (&this->super_EditablePanel,(char *)(unaff_EBX + 0x3bc00c),(char *)0x0,(KeyValues *)0x0,
             (KeyValues *)0x0);
  this->m_nTexture[0] = -1;
  this->m_nTexture[1] = -1;
  this->m_nTexture[2] = -1;
  this->m_nFrontIcon = 0;
  this->m_bHoldIndefinitely = false;
  return;
}


/* CHudControlHelper::GetPanelClassName at 0060e110 */

char * CHudControlHelper::GetPanelClassName(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (char *)(extraout_ECX + 0x3bbe00);
}


/* CHudControlHelper::KB_ChainToMap at 0060e1a0 */

void CHudControlHelper::KB_ChainToMap(void)

{
  PanelKeyBindingMap *pPVar1;
  PanelKeyBindingMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x64cbb1) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64cbb1) = 1;
    pPVar1 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3bbd67));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61f84b);
    pPVar2 = vgui::FindOrAddPanelKeyBindingMap
                       ((char *)(C_TEBeamFollow::~C_TEBeamFollow + unaff_EBX + 3));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CHudControlHelper::ChainToAnimationMap at 0060e260 */

void CHudControlHelper::ChainToAnimationMap(void)

{
  PanelAnimationMap *pPVar1;
  PanelAnimationMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelAnimationMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x64caf0) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64caf0) = 1;
    pPVar1 = FindOrAddPanelAnimationMap(&UNK_003bbca7 + unaff_EBX);
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61f78b);
    pPVar2 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x38cda3));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CHudControlHelper::ChainToMap at 0060e320 */

void CHudControlHelper::ChainToMap(void)

{
  PanelMessageMap *pPVar1;
  PanelMessageMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelMessageMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x64ca2f) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64ca2f) = 1;
    pPVar1 = vgui::FindOrAddPanelMessageMap(&UNK_003bbbe7 + unaff_EBX);
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61f6cb);
    pPVar2 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x38cce3));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CHudControlHelper::GetKBMap at 0060e3e0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: PanelKeyBindingMap * GetKBMap(CHudControlHelper * this) */

PanelKeyBindingMap * __thiscall CHudControlHelper::GetKBMap(CHudControlHelper *this)

{
  int iVar1;
  PanelKeyBindingMap *pPVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x64c94c) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0x64c94c);
    if (iVar1 != 0) {
      pPVar2 = vgui::FindOrAddPanelKeyBindingMap(&UNK_003bbb24 + unaff_EBX);
      *(PanelKeyBindingMap **)(unaff_EBX + 0x64c964) = pPVar2;
      ___cxa_guard_release(unaff_EBX + 0x64c94c);
    }
  }
  return *(PanelKeyBindingMap **)(unaff_EBX + 0x64c964);
}


/* CHudControlHelper::GetAnimMap at 0060e460 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: PanelAnimationMap * GetAnimMap(CHudControlHelper * this) */

PanelAnimationMap * __thiscall CHudControlHelper::GetAnimMap(CHudControlHelper *this)

{
  PanelAnimationMap *pPVar1;
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  pPVar1 = FindOrAddPanelAnimationMap(&UNK_003bbab0 + extraout_ECX);
  return pPVar1;
}


/* CHudControlHelper::GetMessageMap at 0060e480 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: PanelMessageMap * GetMessageMap(CHudControlHelper * this) */

PanelMessageMap * __thiscall CHudControlHelper::GetMessageMap(CHudControlHelper *this)

{
  int iVar1;
  PanelMessageMap *pPVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x64c8a4) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0x64c8a4);
    if (iVar1 != 0) {
      pPVar2 = vgui::FindOrAddPanelMessageMap(&UNK_003bba84 + unaff_EBX);
      *(PanelMessageMap **)(unaff_EBX + 0x64c8c8) = pPVar2;
      ___cxa_guard_release(unaff_EBX + 0x64c8a4);
    }
  }
  return *(PanelMessageMap **)(unaff_EBX + 0x64c8c8);
}


/* __MsgFunc_CHudControlHelper_ControlHelperAnimate at 0060cee0 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */

void __MsgFunc_CHudControlHelper_ControlHelperAnimate(bf_read *msg)

{
  CHud *this;
  CHudElement *pCVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: CHudElement * pElement@[???] */
  ___i686_get_pc_thunk_bx();
  this = GetHud(-1);
  pCVar1 = CHud::FindElement(this,(char *)(unaff_EBX + 0x3bd02c));
  if (pCVar1 != (CHudElement *)0x0) {
    CHudControlHelper::MsgFunc_ControlHelperAnimate
              ((CHudControlHelper *)&pCVar1[-8].m_HudRenderGroups.m_Memory.m_nGrowSize,msg);
  }
  return;
}


/* CHudControlHelper::CHudControlHelper at 0060e0f0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void CHudControlHelper(CHudControlHelper * this, char * pElementName,
   CHudControlHelper * this, char * pElementName) */

void __thiscall
CHudControlHelper::CHudControlHelper
          (CHudControlHelper *this,char *pElementName,CHudControlHelper *this_1,char *pElementName_1
          )

{
  CHudControlHelper(this,pElementName);
  return;
}


/* CHudControlHelper::Init at 0060d060 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void Init(CHudControlHelper * this) */

void __thiscall CHudControlHelper::Init(CHudControlHelper *this)

{
  undefined4 *puVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CUserMessages::HookMessage
            ((CUserMessages *)**(undefined4 **)(unaff_EBX + 0x556d3e),&UNK_0038546f + unaff_EBX,
             (pfnUserMsgHook)(unaff_EBX + -0x18e));
  puVar1 = *(undefined4 **)(&DAT_00557186 + unaff_EBX);
  iVar2 = (**(code **)(*(int *)*puVar1 + 0xa4))((int *)*puVar1,0);
  this->m_nTexture[0] = iVar2;
  (**(code **)(*(int *)*puVar1 + 0x8c))((int *)*puVar1,iVar2,unaff_EBX + 0x3bceee,1,0);
  iVar2 = (**(code **)(*(int *)*puVar1 + 0xa4))((int *)*puVar1,0);
  this->m_nTexture[1] = iVar2;
  (**(code **)(*(int *)*puVar1 + 0x8c))((int *)*puVar1,iVar2,unaff_EBX + 0x3bcf02,1,0);
  iVar2 = (**(code **)(*(int *)*puVar1 + 0xa4))((int *)*puVar1,0);
  this->m_nTexture[2] = iVar2;
  (**(code **)(*(int *)*puVar1 + 0x8c))((int *)*puVar1,iVar2,unaff_EBX + 0x3bcf17,1,0);
  this->m_flDisplayTime = 0.0;
  this->m_flStartTime = 0.0;
  this->m_flFadeInTime = 0.0;
  this->m_flFadeOutTime = 0.0;
  return;
}


/* CHudControlHelper::Reset at 0060c7f0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void Reset(CHudControlHelper * this) */

void __thiscall CHudControlHelper::Reset(CHudControlHelper *this)

{
                    /* WARNING: Could not recover jumptable at 0x0060c7ff. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel[0xed])();
  return;
}


/* CHudControlHelper::ApplySchemeSettings at 0060ce20 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void ApplySchemeSettings(CHudControlHelper * this, IScheme * scheme) */

void __thiscall CHudControlHelper::ApplySchemeSettings(CHudControlHelper *this,IScheme *scheme)

{
  int iVar1;
  int wide;
  int unaff_EBX;
  
                    /* Unresolved local var: int nCenterY@[???] */
  ___i686_get_pc_thunk_bx();
  (*(this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel[0xd4])
            (this,unaff_EBX + 0x3bd0f6,0,0,0);
  vgui::Panel::ApplySchemeSettings((Panel *)this,scheme);
  (*(this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel[0x48])(this,0);
  (*(this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel[0x47])(this,0);
  iVar1 = ScreenHeight();
  wide = ScreenWidth();
  vgui::Panel::SetBounds
            ((Panel *)this,0,(int)((float)iVar1 * *(float *)(&DAT_003d5a84 + unaff_EBX)) + -0x30,
             wide,0x60);
  return;
}


/* CHudControlHelper::ShouldDraw at 0060c820 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: bool ShouldDraw(CHudControlHelper * this) */

bool __thiscall CHudControlHelper::ShouldDraw(CHudControlHelper *this)

{
  float *pfVar1;
  undefined1 uVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  uVar2 = 0;
  if ((*(int *)(*(int *)(&LAB_0075e153 + unaff_EBX) + 0x30) != 0) &&
     (uVar2 = 1, pfVar1 = (float *)(**(int **)(unaff_EBX + 0x557573) + 0xc),
     this->m_flDisplayTime < *pfVar1 || this->m_flDisplayTime == *pfVar1)) {
    uVar2 = this->m_bHoldIndefinitely;
  }
  return (bool)uVar2;
}


/* CHudControlHelper::DrawIcon at 0060c870 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void DrawIcon(CHudControlHelper * this, int x, int y, float flScale,
   float flAlpha, int nIconID, int nDepth) */

void __thiscall
CHudControlHelper::DrawIcon
          (CHudControlHelper *this,int x,int y,float flScale,float flAlpha,int nIconID,int nDepth)

{
  int iVar1;
  int unaff_EBX;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float xLeft;
  float xRight;
  float yTop;
  float yBottom;
  float local_5c;
  float local_58;
  undefined4 local_54;
  undefined4 local_50;
  float local_4c;
  float local_48;
  undefined4 local_44;
  undefined4 local_40;
  float local_3c;
  float local_38;
  undefined4 local_34;
  undefined4 local_30;
  float local_2c;
  float local_28;
  undefined4 local_24;
  undefined4 local_20;
  
                    /* Unresolved local var: float flOffset@[???]
                       Unresolved local var: int nShadowDepth@[???]
                       Unresolved local var: Vertex_t[4] vert@[???]
                       Unresolved local var: float flColor@[???] */
  ___i686_get_pc_thunk_bx();
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x557973) + 0x94))
            ((int *)**(undefined4 **)(unaff_EBX + 0x557973),this->m_nTexture[nIconID]);
  fVar2 = flScale * *(float *)(unaff_EBX + 0x3d598f);
  fVar3 = (float)x - fVar2;
  fVar4 = (float)(x + 0x30) + fVar2;
  fVar5 = (float)y - fVar2;
  fVar2 = fVar2 + (float)(y + 0x30);
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x557973) + 0x34))
            ((int *)**(undefined4 **)(unaff_EBX + 0x557973),0,0,0,
             (int)(flAlpha * *(float *)(&DAT_003d9883 + unaff_EBX)));
  local_38 = *(float *)(unaff_EBX + 0x3d5c9b);
  local_58 = fVar5 + local_38;
  local_5c = fVar3 + local_38;
  local_54 = 0;
  local_50 = 0;
  local_4c = fVar4 + local_38;
  local_44 = 0x3f800000;
  local_40 = 0;
  local_38 = local_38 + fVar2;
  local_34 = 0x3f800000;
  local_30 = 0x3f800000;
  local_24 = 0;
  local_20 = 0x3f800000;
  local_48 = local_58;
  local_3c = local_4c;
  local_2c = local_5c;
  local_28 = local_38;
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x557973) + 0x1a0))
            ((int *)**(undefined4 **)(unaff_EBX + 0x557973),4,&local_5c,1);
  iVar1 = (int)(float)(int)(0xff / (longlong)nDepth);
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x557973) + 0x34))
            ((int *)**(undefined4 **)(unaff_EBX + 0x557973),iVar1,iVar1,iVar1,
             (int)(flAlpha * *(float *)(unaff_EBX + 0x3d5cbf)));
  local_54 = 0;
  local_50 = 0;
  local_44 = 0x3f800000;
  local_40 = 0;
  local_34 = 0x3f800000;
  local_30 = 0x3f800000;
  local_24 = 0;
  local_20 = 0x3f800000;
  local_5c = fVar3;
  local_58 = fVar5;
  local_4c = fVar4;
  local_48 = fVar5;
  local_3c = fVar4;
  local_38 = fVar2;
  local_2c = fVar3;
  local_28 = fVar2;
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x557973) + 0x1a0))
            ((int *)**(undefined4 **)(unaff_EBX + 0x557973),4,&local_5c,1);
  return;
}


/* CHudControlHelper::Paint at 0060d760 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void Paint(CHudControlHelper * this) */

void __thiscall CHudControlHelper::Paint(CHudControlHelper *this)

{
  float fVar1;
  uint uVar2;
  bool bVar3;
  bool bVar4;
  int iVar5;
  int iVar6;
  int unaff_EBX;
  uint uVar7;
  float fVar8;
  float fVar9;
  float fVar10;
  float local_3c;
  int local_34;
  float local_30;
  int local_28 [6];
  
                    /* Unresolved local var: float flGlobalScale@[???]
                       Unresolved local var: float flGlobalAlpha@[???]
                       Unresolved local var: bool bScalingUp@[???]
                       Unresolved local var: bool bScalingDown@[???]
                       Unresolved local var: int iconCenterX@[???]
                       Unresolved local var: int iconY@[???]
                       Unresolved local var: float flMoveScale@[???]
                       Unresolved local var: int nBaseOffset@[???]
                       Unresolved local var: int[3] xOffset@[???]
                       Unresolved local var: int nCurIcon@[???] */
  ___i686_get_pc_thunk_bx();
  fVar9 = *(float *)(**(int **)(&DAT_0055662e + unaff_EBX) + 0xc);
  fVar10 = this->m_flFadeInTime;
  if (fVar10 <= fVar9) {
    if ((this->m_bHoldIndefinitely != false) || (fVar10 = this->m_flFadeOutTime, fVar9 <= fVar10)) {
      local_3c = *(float *)(unaff_EBX + 0x3d4a16);
      bVar4 = false;
    }
    else {
      fVar8 = this->m_flDisplayTime;
      if ((fVar10 != fVar8) || (NAN(fVar10) || NAN(fVar8))) {
        fVar9 = (fVar9 - fVar10) / (fVar8 - fVar10);
        if (*(float *)(unaff_EBX + 0x3d4a16) < fVar9) {
          local_3c = 0.0;
        }
        else {
          fVar10 = 0.0;
          if (0.0 <= fVar9) {
            fVar10 = fVar9;
          }
          local_3c = *(float *)(unaff_EBX + 0x3d4a16) -
                     (fVar10 * fVar10 * fVar10 * *(float *)(&DAT_003d4e7e + unaff_EBX) +
                     fVar10 * fVar10 * *(float *)(&DAT_003d4de2 + unaff_EBX));
        }
      }
      else if (fVar8 <= fVar9) {
        local_3c = 0.0;
      }
      else {
        local_3c = *(float *)(unaff_EBX + 0x3d4a16);
      }
      bVar4 = true;
    }
    iVar5 = vgui::Panel::GetWide((Panel *)this);
    iVar6 = vgui::Panel::GetTall((Panel *)this);
    fVar8 = (float)iVar6 * *(float *)(&DAT_003d514a + unaff_EBX) - *(float *)(unaff_EBX + 0x3d707a);
    fVar9 = local_3c * *(float *)(unaff_EBX + 0x3dad92);
    fVar10 = local_3c * *(float *)(unaff_EBX + 0x3dad96);
    bVar3 = false;
    local_30 = local_3c;
  }
  else {
    fVar8 = this->m_flStartTime;
    if ((fVar10 != fVar8) || (NAN(fVar10) || NAN(fVar8))) {
      fVar9 = (fVar9 - fVar8) / (fVar10 - fVar8);
      local_30 = *(float *)(unaff_EBX + 0x3d4a16);
      if (fVar9 <= local_30) {
        fVar10 = 0.0;
        if (0.0 <= fVar9) {
          fVar10 = fVar9;
        }
        local_30 = fVar10 * fVar10 * *(float *)(&DAT_003d4de2 + unaff_EBX) +
                   fVar10 * fVar10 * fVar10 * *(float *)(&DAT_003d4e7e + unaff_EBX) + 0.0;
      }
    }
    else {
      local_30 = 0.0;
      if (fVar10 <= fVar9) {
        local_30 = *(float *)(unaff_EBX + 0x3d4a16);
      }
    }
    iVar5 = vgui::Panel::GetWide((Panel *)this);
    iVar6 = vgui::Panel::GetTall((Panel *)this);
    fVar8 = (float)iVar6 * *(float *)(&DAT_003d514a + unaff_EBX) - *(float *)(unaff_EBX + 0x3d707a);
    fVar9 = *(float *)(unaff_EBX + 0x3dad92);
    fVar10 = *(float *)(unaff_EBX + 0x3dad96);
    local_3c = 1.0;
    bVar3 = true;
    bVar4 = false;
  }
  local_34 = (int)fVar8;
  iVar5 = (int)((float)(int)((float)iVar5 * *(float *)(unaff_EBX + 0x3d4aa2)) -
               *(float *)(&DAT_003d4ea6 + unaff_EBX));
  local_28[0] = (int)(fVar9 + (float)iVar5);
  local_28[1] = iVar5;
  local_28[2] = (int)(fVar10 + (float)iVar5);
  uVar2 = this->m_nFrontIcon;
  uVar7 = (uVar2 + 1) % 3;
  fVar9 = local_30 * *(float *)(unaff_EBX + 0x3d4aaa);
  fVar10 = local_30 * *(float *)(&DAT_003d6592 + unaff_EBX);
  local_30 = local_30 * *(float *)(unaff_EBX + 0x3d707a);
  if (uVar2 == uVar7) {
    if (uVar7 == 1) {
      fVar8 = *(float *)(unaff_EBX + 0x3d4a16);
      iVar6 = 1;
    }
    else {
      iVar5 = local_28[uVar7];
      if (uVar7 == 0) {
        iVar5 = (int)((float)iVar5 + fVar10);
        fVar8 = *(float *)(&DAT_003d4e3e + unaff_EBX);
        iVar6 = 1;
      }
      else {
        if (uVar7 == 2) {
          iVar5 = (int)((float)iVar5 + fVar9);
        }
        fVar8 = *(float *)(&DAT_003d4e3e + unaff_EBX);
        iVar6 = 1;
      }
    }
  }
  else {
    iVar5 = local_28[uVar7];
    fVar8 = *(float *)(unaff_EBX + 0x3d4a16);
    iVar6 = 2;
  }
  fVar1 = local_30;
  if ((!bVar4) && ((uVar2 != uVar7 || (!bVar3)))) {
    fVar1 = *(float *)(unaff_EBX + 0x3d707a);
  }
  DrawIcon(this,iVar5,local_34,fVar1 * fVar8,local_3c,uVar7,iVar6);
  uVar7 = (uVar7 + 1) % 3;
  uVar2 = this->m_nFrontIcon;
  if (uVar7 == uVar2) {
    if (uVar7 == 1) {
      fVar8 = *(float *)(unaff_EBX + 0x3d4a16);
      iVar5 = local_28[1];
    }
    else {
      if (uVar7 == 0) {
        iVar5 = (int)((float)local_28[uVar7] + fVar10);
        fVar8 = *(float *)(&DAT_003d4e3e + unaff_EBX);
        iVar6 = 1;
        goto LAB_0060d9b4;
      }
      fVar8 = *(float *)(&DAT_003d4e3e + unaff_EBX);
      iVar5 = local_28[uVar7];
    }
    iVar6 = 1;
    if (uVar2 == 2) {
      iVar5 = (int)((float)iVar5 + fVar9);
    }
  }
  else {
    iVar5 = local_28[uVar7];
    fVar8 = *(float *)(unaff_EBX + 0x3d4a16);
    iVar6 = 2;
  }
LAB_0060d9b4:
  fVar1 = local_30;
  if ((!bVar4) && ((uVar7 != uVar2 || (!bVar3)))) {
    fVar1 = *(float *)(unaff_EBX + 0x3d707a);
  }
  DrawIcon(this,iVar5,local_34,fVar1 * fVar8,local_3c,uVar7,iVar6);
  uVar7 = (uVar7 + 1) % 3;
  uVar2 = this->m_nFrontIcon;
  if (uVar7 == uVar2) {
    if (uVar7 == 1) {
      fVar9 = *(float *)(unaff_EBX + 0x3d4a16);
      iVar6 = 1;
      iVar5 = local_28[1];
    }
    else {
      iVar5 = local_28[uVar7];
      if (uVar7 == 0) {
        fVar9 = *(float *)(&DAT_003d4e3e + unaff_EBX);
        iVar6 = 1;
        iVar5 = (int)((float)iVar5 + fVar10);
      }
      else {
        if (uVar2 == 2) {
          iVar5 = (int)((float)iVar5 + fVar9);
        }
        fVar9 = *(float *)(&DAT_003d4e3e + unaff_EBX);
        iVar6 = 1;
      }
    }
  }
  else {
    fVar9 = *(float *)(unaff_EBX + 0x3d4a16);
    iVar6 = 2;
    iVar5 = local_28[uVar7];
  }
  if ((!bVar4) && ((uVar7 != uVar2 || (!bVar3)))) {
    local_30 = 48.0;
  }
  DrawIcon(this,iVar5,local_34,fVar9 * local_30,local_3c,uVar7,iVar6);
  vgui::Panel::Paint((Panel *)this);
  return;
}


/* CHudControlHelper::MsgFunc_ControlHelperAnimate at 0060cab0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void MsgFunc_ControlHelperAnimate(CHudControlHelper * this, bf_read *
   msg) */

void __thiscall
CHudControlHelper::MsgFunc_ControlHelperAnimate(CHudControlHelper *this,bf_read *msg)

{
  uint32 **ppuVar1;
  uint32 *puVar2;
  char cVar3;
  uint uVar4;
  uint32 *puVar5;
  uint32 uVar6;
  float fVar7;
  int iVar8;
  uint uVar9;
  int *piVar10;
  int unaff_EBX;
  int iVar11;
  float fVar12;
  
                    /* Unresolved local var: bool bClear@[???] */
  ___i686_get_pc_thunk_bx();
  iVar8 = (msg->super_CBitRead).m_nBitsAvail;
  if (iVar8 < 8) {
    uVar6 = (msg->super_CBitRead).m_nInBufWord;
    puVar5 = (msg->super_CBitRead).m_pDataIn;
    puVar2 = (msg->super_CBitRead).m_pBufferEnd;
    if (puVar5 != puVar2) {
      if (puVar2 < puVar5) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar5;
        (msg->super_CBitRead).m_pDataIn = puVar5 + 1;
      }
      if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
        iVar11 = 8 - iVar8;
        uVar9 = (msg->super_CBitRead).m_nInBufWord;
        uVar4 = *(uint *)(*(int *)(unaff_EBX + 0x5572ea) + iVar11 * 4);
        (msg->super_CBitRead).m_nBitsAvail = 0x20 - iVar11;
        puVar2 = &(msg->super_CBitRead).m_nInBufWord;
        *puVar2 = *puVar2 >> ((byte)iVar11 & 0x1f);
        if ((uVar9 & uVar4) << ((byte)iVar8 & 0x1f) != 0 || uVar6 != 0) goto LAB_0060cc20;
        goto LAB_0060cafb;
      }
      goto LAB_0060cafe;
    }
    iVar8 = 1;
    (msg->super_CBitRead).m_nBitsAvail = 1;
    (msg->super_CBitRead).m_nInBufWord = 0;
    ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
    *ppuVar1 = *ppuVar1 + 1;
    (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
LAB_0060ccc0:
    uVar9 = (msg->super_CBitRead).m_nInBufWord;
    puVar5 = (msg->super_CBitRead).m_pDataIn;
    puVar2 = (msg->super_CBitRead).m_pBufferEnd;
    if (puVar5 == puVar2) {
      (msg->super_CBitRead).m_nBitsAvail = 1;
      (msg->super_CBitRead).m_nInBufWord = 0;
      ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
      *ppuVar1 = *ppuVar1 + 1;
      (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
    }
    else {
      if (puVar5 < puVar2) {
        (msg->super_CBitRead).m_nInBufWord = *puVar5;
        (msg->super_CBitRead).m_pDataIn = puVar5 + 1;
        cVar3 = (msg->super_CBitRead).super_CBitBuffer.m_bOverflow;
      }
      else {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
        cVar3 = (msg->super_CBitRead).super_CBitBuffer.m_bOverflow;
      }
      if (cVar3 == '\0') {
        iVar11 = 8 - iVar8;
        uVar9 = ((msg->super_CBitRead).m_nInBufWord &
                *(uint *)(*(int *)(unaff_EBX + 0x5572ea) + iVar11 * 4)) << ((byte)iVar8 & 0x1f) |
                uVar9;
        (msg->super_CBitRead).m_nBitsAvail = 0x20 - iVar11;
        puVar2 = &(msg->super_CBitRead).m_nInBufWord;
        *puVar2 = *puVar2 >> ((byte)iVar11 & 0x1f);
        this->m_nFrontIcon = uVar9;
        goto joined_r0x0060cd20;
      }
    }
    this->m_nFrontIcon = 0;
  }
  else {
    uVar9 = (msg->super_CBitRead).m_nInBufWord;
    uVar4 = *(uint *)(*(int *)(unaff_EBX + 0x5572ea) + 0x20);
    (msg->super_CBitRead).m_nBitsAvail = iVar8 + -8;
    if (iVar8 + -8 == 0) {
      (msg->super_CBitRead).m_nBitsAvail = 0x20;
      puVar5 = (msg->super_CBitRead).m_pDataIn;
      puVar2 = (msg->super_CBitRead).m_pBufferEnd;
      if (puVar5 == puVar2) {
        (msg->super_CBitRead).m_nBitsAvail = 1;
        (msg->super_CBitRead).m_nInBufWord = 0;
        ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
        *ppuVar1 = *ppuVar1 + 1;
      }
      else if (puVar2 < puVar5) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar5;
        (msg->super_CBitRead).m_pDataIn = puVar5 + 1;
      }
    }
    else {
      puVar2 = &(msg->super_CBitRead).m_nInBufWord;
      *puVar2 = *puVar2 >> 8;
    }
    if ((uVar9 & uVar4) != 0) {
LAB_0060cc20:
      piVar10 = *(int **)(unaff_EBX + 0x5572de);
      fVar12 = *(float *)(*piVar10 + 0xc) + *(float *)(unaff_EBX + 0x3d57ca);
      this->m_flDisplayTime = fVar12;
      fVar7 = *(float *)(*piVar10 + 0xc);
      this->m_flFadeInTime = fVar7;
      this->m_flStartTime = fVar7;
      this->m_flFadeOutTime = fVar12;
      this->m_bHoldIndefinitely = false;
      return;
    }
LAB_0060cafb:
    iVar8 = (msg->super_CBitRead).m_nBitsAvail;
LAB_0060cafe:
    if (iVar8 < 8) goto LAB_0060ccc0;
    uVar9 = (msg->super_CBitRead).m_nInBufWord;
    uVar4 = *(uint *)(*(int *)(unaff_EBX + 0x5572ea) + 0x20);
    (msg->super_CBitRead).m_nBitsAvail = iVar8 + -8;
    if (iVar8 + -8 == 0) {
      (msg->super_CBitRead).m_nBitsAvail = 0x20;
      puVar5 = (msg->super_CBitRead).m_pDataIn;
      puVar2 = (msg->super_CBitRead).m_pBufferEnd;
      if (puVar5 == puVar2) {
        (msg->super_CBitRead).m_nBitsAvail = 1;
        (msg->super_CBitRead).m_nInBufWord = 0;
        ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
        *ppuVar1 = *ppuVar1 + 1;
      }
      else if (puVar2 < puVar5) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar5;
        (msg->super_CBitRead).m_pDataIn = puVar5 + 1;
      }
    }
    else {
      puVar2 = &(msg->super_CBitRead).m_nInBufWord;
      *puVar2 = *puVar2 >> 8;
    }
    uVar9 = uVar9 & uVar4;
    this->m_nFrontIcon = uVar9;
joined_r0x0060cd20:
    if (uVar9 == 1) {
      piVar10 = *(int **)(unaff_EBX + 0x5572de);
      this->m_flDisplayTime = *(float *)(*piVar10 + 0xc) + *(float *)(unaff_EBX + 0x3d5752);
      this->m_bHoldIndefinitely = false;
      goto LAB_0060cb75;
    }
  }
  this->m_bHoldIndefinitely = true;
  piVar10 = *(int **)(unaff_EBX + 0x5572de);
LAB_0060cb75:
  this->m_flStartTime = *(float *)(*piVar10 + 0xc);
  this->m_flFadeInTime = *(float *)(*piVar10 + 0xc) + *(float *)(unaff_EBX + 0x3d57d2);
  this->m_flFadeOutTime = this->m_flDisplayTime - *(float *)(unaff_EBX + 0x3d57ca);
  return;
}


/* _GLOBAL__I__Z38__MsgFunc_CHudIndicator_IndicatorFlashR7bf_read at 000807c0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__Z38__MsgFunc_CHudIndicator_IndicatorFlashR7bf_read(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

