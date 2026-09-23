/* DWARF-guided pseudocode for game/client/portal2/hud_taunt_earned.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 000808d0 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */

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
  unaff_EBX[0xc588c4] = '\0';
  unaff_EBX[0xc588c5] = '\0';
  unaff_EBX[0xc588c6] = '\0';
  unaff_EBX[0xc588c7] = '\0';
  unaff_EBX[0xc588c8] = '\0';
  unaff_EBX[0xc588c9] = '\0';
  unaff_EBX[0xc588ca] = '\0';
  unaff_EBX[0xc588cb] = '\0';
  unaff_EBX[0xc588cc] = '\0';
  unaff_EBX[0xc588cd] = '\0';
  unaff_EBX[0xc588ce] = '\0';
  unaff_EBX[0xc588cf] = '\0';
  unaff_EBX[0xc588d0] = -1;
  unaff_EBX[0xc588d1] = -1;
  unaff_EBX[0xc588d2] = '\x7f';
  unaff_EBX[0xc588d3] = '\x7f';
  unaff_EBX[0xc588d4] = -1;
  unaff_EBX[0xc588d5] = -1;
  unaff_EBX[0xc588d6] = '\x7f';
  unaff_EBX[0xc588d7] = '\x7f';
  unaff_EBX[0xc588d8] = '\0';
  unaff_EBX[0xc588d9] = '\0';
  unaff_EBX[0xc588da] = '\0';
  unaff_EBX[0xc588db] = '\0';
  unaff_EBX[0xc588dc] = '\0';
  unaff_EBX[0xc588dd] = '\0';
  unaff_EBX[0xc588de] = '\0';
  unaff_EBX[0xc588df] = '\0';
  unaff_EBX[0xc588e0] = '\0';
  unaff_EBX[0xc588e1] = '\0';
  unaff_EBX[0xc588e2] = '\0';
  unaff_EBX[0xc588e3] = '\0';
  unaff_EBX[0xc588e4] = '\0';
  unaff_EBX[0xc588e5] = '\0';
  unaff_EBX[0xc588e6] = '\0';
  unaff_EBX[0xc588e7] = '\0';
  unaff_EBX[0xc588e8] = -1;
  unaff_EBX[0xc588e9] = -1;
  unaff_EBX[0xc588ea] = '\x7f';
  unaff_EBX[0xc588eb] = '\x7f';
  unaff_EBX[0xc588ec] = -1;
  unaff_EBX[0xc588ed] = -1;
  unaff_EBX[0xc588ee] = '\x7f';
  unaff_EBX[0xc588ef] = '\x7f';
  unaff_EBX[0xc588f0] = -1;
  unaff_EBX[0xc588f1] = -1;
  unaff_EBX[0xc588f2] = '\x7f';
  unaff_EBX[0xc588f3] = '\x7f';
  unaff_EBX[0xc588f4] = -1;
  unaff_EBX[0xc588f5] = -1;
  unaff_EBX[0xc588f6] = '\x7f';
  unaff_EBX[0xc588f7] = '\x7f';
  *(char **)(unaff_EBX + 0xc588f8) = unaff_EBX + 0xae45ac;
  ConVar::ConVar((ConVar *)(unaff_EBX + 0xcea0e4),unaff_EBX + 0x9497f7,unaff_EBX + 0x9025e0,0,
                 in_stack_fffffff4,pName_1,unaff_EBP,unaff_retaddr);
  ___cxa_atexit(unaff_EBX + 0x8fc704,0,*(undefined4 *)(unaff_EBX + 0xae3490));
  CHudElementHelper::CHudElementHelper
            ((CHudElementHelper *)(unaff_EBX + 0xc588fc),
             (_func_CHudElement_ptr *)(unaff_EBX + 0x58f214),0x32,0);
  return;
}


/* __tcf_0 at 0097cfe0 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ed9d8),in_stack_00000008);
  return;
}


/* CHUDTauntEarned::~CHUDTauntEarned at 00610300 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CHUDTauntEarned(CHUDTauntEarned * this, int __in_chrg) */

void __thiscall CHUDTauntEarned::~CHUDTauntEarned(CHUDTauntEarned *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CHudElement).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(unaff_EBX + 0x5e2674);
  (this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5e26e4);
  vgui::EditablePanel::~EditablePanel(&this->super_EditablePanel,in_stack_ffffffd8);
  CHudElement::~CHudElement(&this->super_CHudElement,in_stack_ffffffd8);
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x553a5c) + 8))
            ((int *)**(undefined4 **)(unaff_EBX + 0x553a5c),this);
  return;
}


/* CHUDTauntEarned::~CHUDTauntEarned at 00610390 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~CHUDTauntEarned(CHUDTauntEarned * this, int __in_chrg) */

void __thiscall CHUDTauntEarned::~CHUDTauntEarned(CHUDTauntEarned *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CHudElement).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(unaff_EBX + 0x5e25e4);
  (this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5e2654);
  vgui::EditablePanel::~EditablePanel(&this->super_EditablePanel,in_stack_ffffffd8);
  CHudElement::~CHudElement(&this->super_CHudElement,__in_chrg);
  return;
}


/* CHUDTauntEarned::CHUDTauntEarned at 0060f8c0 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void CHUDTauntEarned(CHUDTauntEarned * this, char * pElementName) */

void __thiscall CHUDTauntEarned::CHUDTauntEarned(CHUDTauntEarned *this,char *pElementName)

{
  char cVar1;
  PanelKeyBindingMap *pPVar2;
  PanelKeyBindingMap *pPVar3;
  IClientMode *pIVar4;
  Panel *newParent;
  Label *this_00;
  ImagePanel *this_01;
  PanelMessageMap *pPVar5;
  PanelMessageMap *pPVar6;
  PanelAnimationMap *pPVar7;
  PanelAnimationMap *pPVar8;
  int unaff_EBX;
  ImagePanel *this_1;
  Label *in_stack_ffffffd4;
  Panel *in_stack_ffffffd8;
  char *in_stack_ffffffdc;
  Panel *this_02;
  
  ___i686_get_pc_thunk_bx();
  CHudElement::CHudElement(&this->super_CHudElement,pElementName);
  this_02 = &(this->super_EditablePanel).super_Panel;
  vgui::EditablePanel::EditablePanel
            ((EditablePanel *)this_02,(Panel *)0x0,(char *)(unaff_EBX + 0x3ba81e));
  (this->super_CHudElement).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(unaff_EBX + 0x5e30ba);
  (this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5e312a);
  if (*(char *)(unaff_EBX + 0x64b50a) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64b50a) = 1;
    pPVar5 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3ba7d2));
    pPVar5->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61e14a);
    pPVar6 = vgui::FindOrAddPanelMessageMap(&UNK_0038b746 + unaff_EBX);
    pPVar5->baseMap = pPVar6;
    cVar1 = *(char *)(unaff_EBX + 0x64b50b);
  }
  else {
    cVar1 = *(char *)(unaff_EBX + 0x64b50b);
  }
  if (cVar1 == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64b50b) = 1;
    pPVar7 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x3ba7d2));
    pPVar7->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61e14a);
    pPVar8 = FindOrAddPanelAnimationMap(&UNK_0038b746 + unaff_EBX);
    pPVar7->baseMap = pPVar8;
  }
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
  if (*(char *)(unaff_EBX + 0x64b50c) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64b50c) = 1;
    pPVar2 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3ba7d2));
    pPVar2->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61e14a);
    pPVar3 = vgui::FindOrAddPanelKeyBindingMap(&UNK_0038b746 + unaff_EBX);
    pPVar2->baseMap = pPVar3;
  }
  *(undefined4 *)(this->m_rgbaCurrent)._color = 0;
  pIVar4 = GetClientMode();
  newParent = (Panel *)(*pIVar4->_vptr_IClientMode[10])(pIVar4);
  vgui::Panel::SetParent(this_02,newParent);
  CHudElement::SetHiddenBits(&this->super_CHudElement,0x10);
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5548c2) + 0x54))
            ((int *)**(undefined4 **)(unaff_EBX + 0x5548c2),
             (this->super_EditablePanel).super_Panel._vpanel,0);
  this_00 = operator_new(0x1a4);
  this_1 = (ImagePanel *)(unaff_EBX + 0x379396);
  vgui::Label::Label(this_00,this_02,(char *)(unaff_EBX + 0x3ba82b),(char *)this_1,in_stack_ffffffd4
                     ,in_stack_ffffffd8,in_stack_ffffffdc,(char *)this_02);
  this->m_pGestureLabel = this_00;
  this_01 = operator_new(0x17c);
  vgui::ImagePanel::ImagePanel
            (this_01,this_02,(char *)(unaff_EBX + 0x3ba838),this_1,&in_stack_ffffffd4->super_Panel,
             (char *)in_stack_ffffffd8);
  this->m_pGestureImage = this_01;
  this->m_bActive = false;
  return;
}


/* CHUDTauntEarned::GetPanelClassName at 00610070 */

char * CHUDTauntEarned::GetPanelClassName(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (char *)(extraout_ECX + 0x3ba028);
}


/* CHUDTauntEarned::KB_ChainToMap at 00610080 */

void CHUDTauntEarned::KB_ChainToMap(void)

{
  PanelKeyBindingMap *pPVar1;
  PanelKeyBindingMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x64ad49) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64ad49) = 1;
    pPVar1 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3ba00f));
    pPVar1->pfnClassName = *(_func_char_ptr **)(&DAT_0061d987 + unaff_EBX);
    pPVar2 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x38af83));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CHUDTauntEarned::ChainToAnimationMap at 006100e0 */

void CHUDTauntEarned::ChainToAnimationMap(void)

{
  PanelAnimationMap *pPVar1;
  PanelAnimationMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelAnimationMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&DAT_0064ace8)[unaff_EBX] == '\0') {
    (&DAT_0064ace8)[unaff_EBX] = 1;
    pPVar1 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x3b9faf));
    pPVar1->pfnClassName = *(_func_char_ptr **)(&DAT_0061d927 + unaff_EBX);
    pPVar2 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x38af23));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CHUDTauntEarned::ChainToMap at 00610140 */

void CHUDTauntEarned::ChainToMap(void)

{
  PanelMessageMap *pPVar1;
  PanelMessageMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelMessageMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x64ac87) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64ac87) = 1;
    pPVar1 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3b9f4f));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61d8c7);
    pPVar2 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x38aec3));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CHUDTauntEarned::GetKBMap at 006101b0 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: PanelKeyBindingMap * GetKBMap(CHUDTauntEarned * this) */

PanelKeyBindingMap * __thiscall CHUDTauntEarned::GetKBMap(CHUDTauntEarned *this)

{
  int iVar1;
  PanelKeyBindingMap *pPVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x64abec) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0x64abec);
    if (iVar1 != 0) {
      pPVar2 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3b9edc));
      *(PanelKeyBindingMap **)(unaff_EBX + 0x64ac0c) = pPVar2;
      ___cxa_guard_release(unaff_EBX + 0x64abec);
    }
  }
  return *(PanelKeyBindingMap **)(unaff_EBX + 0x64ac0c);
}


/* CHUDTauntEarned::GetAnimMap at 00610240 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: PanelAnimationMap * GetAnimMap(CHUDTauntEarned * this) */

PanelAnimationMap * __thiscall CHUDTauntEarned::GetAnimMap(CHUDTauntEarned *this)

{
  PanelAnimationMap *pPVar1;
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  pPVar1 = FindOrAddPanelAnimationMap((char *)(extraout_ECX + 0x3b9e58));
  return pPVar1;
}


/* CHUDTauntEarned::GetMessageMap at 00610270 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: PanelMessageMap * GetMessageMap(CHUDTauntEarned * this) */

PanelMessageMap * __thiscall CHUDTauntEarned::GetMessageMap(CHUDTauntEarned *this)

{
  int iVar1;
  PanelMessageMap *pPVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x64ab24) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0x64ab24);
    if (iVar1 != 0) {
      pPVar2 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3b9e1c));
      *(PanelMessageMap **)(unaff_EBX + 0x64ab50) = pPVar2;
      ___cxa_guard_release(unaff_EBX + 0x64ab24);
    }
  }
  return *(PanelMessageMap **)(unaff_EBX + 0x64ab50);
}


/* Create_CHUDTauntEarned at 0060faf0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */

CHudElement * Create_CHUDTauntEarned(void)

{
  CHUDTauntEarned *this;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: void * pMem@[???] */
  this = (CHUDTauntEarned *)
         (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x55426c))
                   ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x55426c),0x1c4);
  _memset(this,0,0x1c4);
  CHUDTauntEarned::CHUDTauntEarned(this,(char *)(unaff_EBX + 0x3ba59c));
  return &this->super_CHudElement;
}


/* CHUDTauntEarned::CHUDTauntEarned at 0060fae0 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void CHUDTauntEarned(CHUDTauntEarned * this, char * pElementName,
   CHUDTauntEarned * this, char * pElementName) */

void __thiscall
CHUDTauntEarned::CHUDTauntEarned
          (CHUDTauntEarned *this,char *pElementName,CHUDTauntEarned *this_1,char *pElementName_1)

{
  CHUDTauntEarned(this,pElementName);
  return;
}


/* CHUDTauntEarned::ApplySchemeSettings at 0060f630 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ApplySchemeSettings(CHUDTauntEarned * this, IScheme * pScheme) */

void __thiscall CHUDTauntEarned::ApplySchemeSettings(CHUDTauntEarned *this,IScheme *pScheme)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  vgui::Panel::ApplySchemeSettings(&(this->super_EditablePanel).super_Panel,pScheme);
  (*(this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel[0xd4])
            (&this->super_EditablePanel,unaff_EBX + 0x3baa6c,0,0,0);
  return;
}


/* CHUDTauntEarned::OnTick at 0060fb90 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void OnTick(CHUDTauntEarned * this) */

void __thiscall CHUDTauntEarned::OnTick(CHUDTauntEarned *this)

{
  float fVar1;
  ClientMenuManagerTaunt *this_00;
  EditablePanel *this_01;
  int iVar2;
  TauntStatusData *pTVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  AnimationController *pAVar7;
  int unaff_EBX;
  longdouble lVar8;
  float fVar9;
  CLocalPlayerFilter *this_1;
  EditablePanel *__in_chrg;
  bool local_4d;
  C_RecipientFilter local_40;
  undefined4 local_20;
  
                    /* Unresolved local var: bool bVisible@[???]
                       Unresolved local var: int nUnusedCount@[???] */
  ___i686_get_pc_thunk_bx();
  this_01 = &this->super_EditablePanel;
  vgui::Panel::OnTick(&this_01->super_Panel);
  iVar4 = *(int *)(*(int *)(unaff_EBX + 0x75ae3b) + 0x30);
  this_00 = *(ClientMenuManagerTaunt **)(unaff_EBX + 0x554463);
  iVar2 = ClientMenuManagerTaunt::GetNumTauntsUnused(this_00);
  if (iVar2 == 0) {
    local_4d = iVar4 != 0;
  }
  else {
    pTVar3 = ClientMenuManagerTaunt::GetUnusedTaunt
                       (this_00,(int)(*(float *)(unaff_EBX + 0x3d266f) *
                                     *(float *)(**(int **)(unaff_EBX + 0x5541fb) + 0xc)) % iVar2);
    (*(this->m_pGestureImage->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd5])
              (this->m_pGestureImage,pTVar3->szIcon);
    iVar4 = vgui::ImagePanel::GetNumFrames(this->m_pGestureImage);
    if (iVar4 < 2) {
      vgui::ImagePanel::SetFrame(this->m_pGestureImage,0);
      local_4d = true;
    }
    else {
      lVar8 = (longdouble)
              _sinf(*(float *)(unaff_EBX + 0x3d54fb) *
                    *(float *)(**(int **)(unaff_EBX + 0x5541fb) + 0xc));
      fVar9 = *(float *)(unaff_EBX + 0x3d25e3);
      fVar1 = *(float *)(unaff_EBX + 0x3d266f);
      iVar4 = vgui::ImagePanel::GetNumFrames(this->m_pGestureImage);
      iVar2 = vgui::ImagePanel::GetNumFrames(this->m_pGestureImage);
      if ((float)iVar4 * fVar1 * (fVar9 + (float)lVar8) < (float)(iVar2 + -1)) {
        lVar8 = (longdouble)
                _sinf(*(float *)(unaff_EBX + 0x3d54fb) *
                      *(float *)(**(int **)(unaff_EBX + 0x5541fb) + 0xc));
        fVar9 = *(float *)(unaff_EBX + 0x3d25e3);
        fVar1 = *(float *)(unaff_EBX + 0x3d266f);
        iVar4 = vgui::ImagePanel::GetNumFrames(this->m_pGestureImage);
        fVar9 = ((float)lVar8 + fVar9) * fVar1 * (float)iVar4;
      }
      else {
        iVar4 = vgui::ImagePanel::GetNumFrames(this->m_pGestureImage);
        fVar9 = (float)(iVar4 + -1);
      }
      vgui::ImagePanel::SetFrame(this->m_pGestureImage,(int)fVar9);
      local_4d = true;
    }
  }
  lVar8 = (longdouble)
          _sinf(*(float *)(unaff_EBX + 0x3d26fb) *
                *(float *)(**(int **)(unaff_EBX + 0x5541fb) + 0xc));
  local_20 = CONCAT13(0xff,CONCAT12(0x10,CONCAT11((char)(int)(((float)lVar8 +
                                                              *(float *)(unaff_EBX + 0x3d25e3)) *
                                                              *(float *)(unaff_EBX + 0x3d27df) *
                                                              *(float *)(unaff_EBX + 0x3d6563) +
                                                             *(float *)(unaff_EBX + 0x3d3597)),8)));
  *(undefined4 *)(this->m_rgbaCurrent)._color = local_20;
  (*(this->m_pGestureImage->super_Panel).super_IClientPanel._vptr_IClientPanel[0xd8])
            (this->m_pGestureImage,local_20);
  this_1 = *(CLocalPlayerFilter **)(this->m_rgbaCurrent)._color;
  (*(this->m_pGestureLabel->super_Panel).super_IClientPanel._vptr_IClientPanel[0x3d])
            (this->m_pGestureLabel);
  if (this->m_bActive == false) {
    if (local_4d != false) {
      CLocalPlayerFilter::CLocalPlayerFilter((CLocalPlayerFilter *)&local_40,this_1);
      C_BaseEntity::EmitSound
                (&local_40.super_IRecipientFilter,-1,(char *)(unaff_EBX + 0x38360b),(Vector *)0x0,
                 0.0,(float *)0x0);
      this->m_bActive = true;
      vgui::Panel::SetAlpha(&this_01->super_Panel,0);
      iVar4 = ScreenHeight();
      iVar2 = vgui::Panel::GetTall(&this_01->super_Panel);
      iVar5 = ScreenWidth();
      iVar6 = vgui::Panel::GetWide(&this_01->super_Panel);
      vgui::Panel::SetPos(&this_01->super_Panel,iVar5 / 2 - iVar6 / 2,iVar4 / 2 - iVar2 / 2);
      pAVar7 = vgui::GetAnimationController();
      vgui::AnimationController::RunAnimationCommand
                (pAVar7,&this_01->super_Panel,(char *)(unaff_EBX + 0x3ba573),255.0,0.0,0.5,
                 INTERPOLATOR_DEACCEL,0.0);
      pAVar7 = vgui::GetAnimationController();
      vgui::AnimationController::RunAnimationCommand
                (pAVar7,&this_01->super_Panel,(char *)(unaff_EBX + 0x3b7bf3),0.0,1.5,0.5,
                 INTERPOLATOR_DEACCEL,0.0);
      iVar4 = ScreenHeight();
      iVar2 = vgui::Panel::GetTall(&this_01->super_Panel);
      pAVar7 = vgui::GetAnimationController();
      __in_chrg = this_01;
      vgui::AnimationController::RunAnimationCommand
                (pAVar7,&this_01->super_Panel,(char *)(unaff_EBX + 0x3cf93d),(float)(iVar4 - iVar2),
                 1.5,0.5,INTERPOLATOR_DEACCEL,0.0);
      local_40.super_IRecipientFilter._vptr_IRecipientFilter =
           (_func_int_varargs **)(unaff_EBX + 0x55b9a7);
      C_RecipientFilter::~C_RecipientFilter(&local_40,(int)__in_chrg);
      goto LAB_0060fc97;
    }
  }
  else if (local_4d != false) goto LAB_0060fc97;
  this->m_bActive = false;
LAB_0060fc97:
  (*(this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel[0x22])
            (this_01,(uint)local_4d);
  return;
}


/* CHUDTauntEarned::PaintBackground at 0060f6a0 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void PaintBackground(CHUDTauntEarned * this) */

void __thiscall CHUDTauntEarned::PaintBackground(CHUDTauntEarned *this)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int unaff_EBX;
  EditablePanel *this_00;
  int iVar7;
  int nYBottom;
  int nXRight;
  int nX;
  int local_28;
  int local_24;
  int local_20;
  
                    /* Unresolved local var: int nSpacing@[???]
                       Unresolved local var: int nSize@[???]
                       Unresolved local var: int nOffset@[???]
                       Unresolved local var: int nY@[???] */
  ___i686_get_pc_thunk_bx();
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x554b46) + 0x38))
            ((int *)**(undefined4 **)(unaff_EBX + 0x554b46),
             *(undefined4 *)(this->m_rgbaCurrent)._color);
  this_00 = &this->super_EditablePanel;
  iVar1 = vgui::Panel::GetWide(&this_00->super_Panel);
  iVar2 = vgui::Panel::GetTall(&this_00->super_Panel);
  iVar7 = (int)((float)(iVar2 * iVar1) * *(float *)(unaff_EBX + 0x3d8f0e));
  iVar2 = vgui::Panel::GetTall(&this_00->super_Panel);
  iVar3 = iVar2 + -6;
  iVar4 = vgui::Panel::GetWide(&this_00->super_Panel);
  iVar5 = iVar4 + -6;
  local_24 = (int)(*(float *)(unaff_EBX + 0x3d2d36) *
                  *(float *)(**(int **)(unaff_EBX + 0x5546ee) + 0xc)) % iVar7;
  local_28 = (iVar7 + 3) - local_24;
  local_24 = local_24 + 3;
  iVar1 = local_24;
  local_20 = local_28;
  for (nX = 3; iVar6 = vgui::Panel::GetWide(&this_00->super_Panel), nX < iVar6 + iVar7;
      nX = nX + iVar7) {
    if (iVar1 < iVar5) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x554b46) + 0x3c))
                ((int *)**(undefined4 **)(unaff_EBX + 0x554b46),iVar1,3,iVar1 + 3,6);
    }
    if (local_20 < iVar5) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x554b46) + 0x3c))
                ((int *)**(undefined4 **)(unaff_EBX + 0x554b46),local_20,iVar3,local_20 + 3,
                 iVar2 + -3);
    }
    local_20 = local_20 + iVar7;
    iVar1 = iVar1 + iVar7;
  }
  for (iVar1 = 3; iVar2 = vgui::Panel::GetTall(&this_00->super_Panel), iVar1 < iVar2 + iVar7;
      iVar1 = iVar1 + iVar7) {
    if (local_28 < iVar3) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x554b46) + 0x3c))
                ((int *)**(undefined4 **)(unaff_EBX + 0x554b46),3,local_28,6,local_28 + 3);
    }
    if (local_24 < iVar3) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x554b46) + 0x3c))
                ((int *)**(undefined4 **)(unaff_EBX + 0x554b46),iVar5,local_24,iVar4 + -3,
                 local_24 + 3);
    }
    local_24 = local_24 + iVar7;
    local_28 = local_28 + iVar7;
  }
  return;
}


/* CHUDTauntEarned::Paint at 0060f610 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void Paint(CHUDTauntEarned * this) */

void __thiscall CHUDTauntEarned::Paint(CHUDTauntEarned *this)

{
  vgui::Panel::Paint(&(this->super_EditablePanel).super_Panel);
  return;
}


/* _GLOBAL__I_cl_hud_taunt_earned_show at 000809f0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_cl_hud_taunt_earned_show(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

