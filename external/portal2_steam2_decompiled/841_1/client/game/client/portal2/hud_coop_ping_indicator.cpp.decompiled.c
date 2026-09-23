/* DWARF-guided pseudocode for game/client/portal2/hud_coop_ping_indicator.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 00080590 */

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
  unaff_EBX[0xc58b3c] = '\0';
  unaff_EBX[0xc58b3d] = '\0';
  unaff_EBX[0xc58b3e] = '\0';
  unaff_EBX[0xc58b3f] = '\0';
  unaff_EBX[0xc58b40] = '\0';
  unaff_EBX[0xc58b41] = '\0';
  unaff_EBX[0xc58b42] = '\0';
  unaff_EBX[0xc58b43] = '\0';
  unaff_EBX[0xc58b44] = '\0';
  unaff_EBX[0xc58b45] = '\0';
  unaff_EBX[0xc58b46] = '\0';
  unaff_EBX[0xc58b47] = '\0';
  unaff_EBX[0xc58b48] = -1;
  unaff_EBX[0xc58b49] = -1;
  unaff_EBX[0xc58b4a] = '\x7f';
  unaff_EBX[0xc58b4b] = '\x7f';
  unaff_EBX[0xc58b4c] = -1;
  unaff_EBX[0xc58b4d] = -1;
  unaff_EBX[0xc58b4e] = '\x7f';
  unaff_EBX[0xc58b4f] = '\x7f';
  unaff_EBX[0xc58b50] = '\0';
  unaff_EBX[0xc58b51] = '\0';
  unaff_EBX[0xc58b52] = '\0';
  unaff_EBX[0xc58b53] = '\0';
  unaff_EBX[0xc58b54] = '\0';
  unaff_EBX[0xc58b55] = '\0';
  unaff_EBX[0xc58b56] = '\0';
  unaff_EBX[0xc58b57] = '\0';
  unaff_EBX[0xc58b58] = '\0';
  unaff_EBX[0xc58b59] = '\0';
  unaff_EBX[0xc58b5a] = '\0';
  unaff_EBX[0xc58b5b] = '\0';
  unaff_EBX[0xc58b5c] = '\0';
  unaff_EBX[0xc58b5d] = '\0';
  unaff_EBX[0xc58b5e] = '\0';
  unaff_EBX[0xc58b5f] = '\0';
  unaff_EBX[0xc58b60] = -1;
  unaff_EBX[0xc58b61] = -1;
  unaff_EBX[0xc58b62] = '\x7f';
  unaff_EBX[0xc58b63] = '\x7f';
  unaff_EBX[0xc58b64] = -1;
  unaff_EBX[0xc58b65] = -1;
  unaff_EBX[0xc58b66] = '\x7f';
  unaff_EBX[0xc58b67] = '\x7f';
  unaff_EBX[0xc58b68] = -1;
  unaff_EBX[0xc58b69] = -1;
  unaff_EBX[0xc58b6a] = '\x7f';
  unaff_EBX[0xc58b6b] = '\x7f';
  unaff_EBX[0xc58b6c] = -1;
  unaff_EBX[0xc58b6d] = -1;
  unaff_EBX[0xc58b6e] = '\x7f';
  unaff_EBX[0xc58b6f] = '\x7f';
  *(char **)(unaff_EBX + 0xc58b70) = unaff_EBX + 0xae48ec;
  ConVar::ConVar((ConVar *)(unaff_EBX + 0xcea364),unaff_EBX + 0x949939,unaff_EBX + 0x94e3dc,0,
                 in_stack_fffffff4,pName_1,unaff_EBP,unaff_retaddr);
  ___cxa_atexit(unaff_EBX + 0x8fca04,0,*(undefined4 *)(unaff_EBX + 0xae37d0));
  CHudElementHelper::CHudElementHelper
            ((CHudElementHelper *)(unaff_EBX + 0xc58b74),
             (_func_CHudElement_ptr *)(unaff_EBX + 0x58ba94),0x32,0);
  return;
}


/* __tcf_0 at 0097cfa0 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ed958),in_stack_00000008);
  return;
}


/* CHudCoopPingIndicator::~CHudCoopPingIndicator at 0060c350 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void ~CHudCoopPingIndicator(CHudCoopPingIndicator * this, int
   __in_chrg) */

void __thiscall
CHudCoopPingIndicator::~CHudCoopPingIndicator(CHudCoopPingIndicator *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CHudElement).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(unaff_EBX + 0x5e5584);
  (this->super_Panel).super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5e55ec);
  vgui::Panel::~Panel(&this->super_Panel,in_stack_ffffffd8);
  CHudElement::~CHudElement(&this->super_CHudElement,in_stack_ffffffd8);
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x557a0c) + 8))
            ((int *)**(undefined4 **)(unaff_EBX + 0x557a0c),this);
  return;
}


/* CHudCoopPingIndicator::~CHudCoopPingIndicator at 0060c3e0 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void ~CHudCoopPingIndicator(CHudCoopPingIndicator * this, int
   __in_chrg) */

void __thiscall
CHudCoopPingIndicator::~CHudCoopPingIndicator(CHudCoopPingIndicator *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CHudElement).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(unaff_EBX + 0x5e54f4);
  (this->super_Panel).super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5e555c);
  vgui::Panel::~Panel(&this->super_Panel,in_stack_ffffffd8);
  CHudElement::~CHudElement(&this->super_CHudElement,__in_chrg);
  return;
}


/* CHudCoopPingIndicator::CHudCoopPingIndicator at 0060beb0 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void CHudCoopPingIndicator(CHudCoopPingIndicator * this, char *
   pElementName) */

void __thiscall
CHudCoopPingIndicator::CHudCoopPingIndicator(CHudCoopPingIndicator *this,char *pElementName)

{
  char *panelName;
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
  CHudElement::CHudElement(&this->super_CHudElement,pElementName);
  panelName = (char *)(unaff_EBX + 0x3be001);
  vgui::Panel::Panel(&this->super_Panel,(Panel *)0x0,panelName);
  (this->super_CHudElement).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(unaff_EBX + 0x5e5a2a);
  (this->super_Panel).super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5e5a92);
  if (*(char *)(unaff_EBX + 0x64ee5a) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64ee5a) = 1;
    pPVar5 = vgui::FindOrAddPanelMessageMap(panelName);
    pPVar5->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x621b26);
    pPVar6 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x39aeba));
    pPVar5->baseMap = pPVar6;
    cVar1 = *(char *)(unaff_EBX + 0x64ee5b);
  }
  else {
    cVar1 = *(char *)(unaff_EBX + 0x64ee5b);
  }
  if (cVar1 == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64ee5b) = 1;
    pPVar7 = FindOrAddPanelAnimationMap(panelName);
    pPVar7->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x621b26);
    pPVar8 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x39aeba));
    pPVar7->baseMap = pPVar8;
  }
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
  if (*(char *)(unaff_EBX + 0x64ee5c) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64ee5c) = 1;
    pPVar2 = vgui::FindOrAddPanelKeyBindingMap(panelName);
    pPVar2->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x621b26);
    pPVar3 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x39aeba));
    pPVar2->baseMap = pPVar3;
  }
                    /* Unresolved local var: Panel * pParent@[???] */
  pIVar4 = GetClientMode();
  newParent = (Panel *)(*pIVar4->_vptr_IClientMode[10])(pIVar4);
  vgui::Panel::SetParent(&this->super_Panel,newParent);
  return;
}


/* CHudCoopPingIndicator::GetPanelClassName at 0060c0c0 */

char * CHudCoopPingIndicator::GetPanelClassName(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return &UNK_003bddf7 + extraout_ECX;
}


/* CHudCoopPingIndicator::KB_ChainToMap at 0060c0d0 */

void CHudCoopPingIndicator::KB_ChainToMap(void)

{
  PanelKeyBindingMap *pPVar1;
  PanelKeyBindingMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x64ec39) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64ec39) = 1;
    pPVar1 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3bddde));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x621903);
    pPVar2 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x39ac97));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CHudCoopPingIndicator::ChainToAnimationMap at 0060c130 */

void CHudCoopPingIndicator::ChainToAnimationMap(void)

{
  PanelAnimationMap *pPVar1;
  PanelAnimationMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelAnimationMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x64ebd8) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64ebd8) = 1;
    pPVar1 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x3bdd7e));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x6218a3);
    pPVar2 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x39ac37));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CHudCoopPingIndicator::ChainToMap at 0060c190 */

void CHudCoopPingIndicator::ChainToMap(void)

{
  PanelMessageMap *pPVar1;
  PanelMessageMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelMessageMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x64eb77) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64eb77) = 1;
    pPVar1 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3bdd1e));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x621843);
    pPVar2 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x39abd7));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CHudCoopPingIndicator::GetMessageMap at 0060c200 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: PanelMessageMap * GetMessageMap(CHudCoopPingIndicator * this) */

PanelMessageMap * __thiscall CHudCoopPingIndicator::GetMessageMap(CHudCoopPingIndicator *this)

{
  int iVar1;
  PanelMessageMap *pPVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x64ead4) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0x64ead4);
    if (iVar1 != 0) {
      pPVar2 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3bdcab));
      *(PanelMessageMap **)(unaff_EBX + 0x64eb00) = pPVar2;
      ___cxa_guard_release(unaff_EBX + 0x64ead4);
    }
  }
  return *(PanelMessageMap **)(unaff_EBX + 0x64eb00);
}


/* CHudCoopPingIndicator::GetKBMap at 0060c290 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: PanelKeyBindingMap * GetKBMap(CHudCoopPingIndicator * this) */

PanelKeyBindingMap * __thiscall CHudCoopPingIndicator::GetKBMap(CHudCoopPingIndicator *this)

{
  int iVar1;
  PanelKeyBindingMap *pPVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x64ea4c) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0x64ea4c);
    if (iVar1 != 0) {
      pPVar2 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3bdc1b));
      *(PanelKeyBindingMap **)(unaff_EBX + 0x64ea6c) = pPVar2;
      ___cxa_guard_release(unaff_EBX + 0x64ea4c);
    }
  }
  return *(PanelKeyBindingMap **)(unaff_EBX + 0x64ea6c);
}


/* CHudCoopPingIndicator::GetAnimMap at 0060c320 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: PanelAnimationMap * GetAnimMap(CHudCoopPingIndicator * this) */

PanelAnimationMap * __thiscall CHudCoopPingIndicator::GetAnimMap(CHudCoopPingIndicator *this)

{
  PanelAnimationMap *pPVar1;
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  pPVar1 = FindOrAddPanelAnimationMap((char *)(extraout_ECX + 0x3bdb97));
  return pPVar1;
}


/* Create_CHudCoopPingIndicator at 0060c030 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */

CHudElement * Create_CHudCoopPingIndicator(void)

{
  CHudCoopPingIndicator *this;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: void * pMem@[???] */
  this = (CHudCoopPingIndicator *)
         (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x557d2c))
                   ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x557d2c),0x198);
  _memset(this,0,0x198);
  CHudCoopPingIndicator::CHudCoopPingIndicator(this,(char *)(unaff_EBX + 0x3bde7b));
  return &this->super_CHudElement;
}


/* __MsgFunc_CHudCoopPingIndicator_HudPingIndicator at 0060bd30 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */

void __MsgFunc_CHudCoopPingIndicator_HudPingIndicator(bf_read *msg)

{
  CHud *this;
  CHudCoopPingIndicator *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CHudElement * pElement@[???] */
  ___i686_get_pc_thunk_bx();
  this = GetHud(-1);
  this_00 = (CHudCoopPingIndicator *)CHud::FindElement(this,(char *)(unaff_EBX + 0x3be183));
  if (this_00 != (CHudCoopPingIndicator *)0x0) {
    CHudCoopPingIndicator::MsgFunc_HudPingIndicator(this_00,msg);
  }
  return;
}


/* CHudCoopPingIndicator::CHudCoopPingIndicator at 0060c020 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void CHudCoopPingIndicator(CHudCoopPingIndicator * this, char *
   pElementName, CHudCoopPingIndicator * this, char * pElementName) */

void __thiscall
CHudCoopPingIndicator::CHudCoopPingIndicator
          (CHudCoopPingIndicator *this,char *pElementName,CHudCoopPingIndicator *this_1,
          char *pElementName_1)

{
  CHudCoopPingIndicator(this,pElementName);
  return;
}


/* CHudCoopPingIndicator::ApplySchemeSettings at 0060bcf0 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void ApplySchemeSettings(CHudCoopPingIndicator * this, IScheme *
   scheme) */

void __thiscall
CHudCoopPingIndicator::ApplySchemeSettings(CHudCoopPingIndicator *this,IScheme *scheme)

{
  vgui::Panel::ApplySchemeSettings(&this->super_Panel,scheme);
                    /* WARNING: Could not recover jumptable at 0x0060bd2d. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_Panel).super_IClientPanel._vptr_IClientPanel[0x48])();
  return;
}


/* CHudCoopPingIndicator::Init at 0060be20 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void Init(CHudCoopPingIndicator * this) */

void __thiscall CHudCoopPingIndicator::Init(CHudCoopPingIndicator *this)

{
  undefined4 *puVar1;
  int *piVar2;
  int iVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  puVar1 = *(undefined4 **)(unaff_EBX + 0x5583c7);
  piVar2 = (int *)*puVar1;
  iVar3 = (**(code **)(*piVar2 + 0xa4))(piVar2,0);
  this->m_nArrowTexture = iVar3;
  piVar2 = (int *)*puVar1;
  (**(code **)(*piVar2 + 0x8c))(piVar2,iVar3,unaff_EBX + 0x3ac7d4,1,0);
  CUserMessages::HookMessage
            ((CUserMessages *)**(undefined4 **)(unaff_EBX + 0x557f7f),
             (char *)(C_Team::C_Team + unaff_EBX + 5),(pfnUserMsgHook)(unaff_EBX + -0xfd));
  return;
}


/* CHudCoopPingIndicator::LevelInit at 0060b2a0 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void LevelInit(CHudCoopPingIndicator * this) */

void __thiscall CHudCoopPingIndicator::LevelInit(CHudCoopPingIndicator *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  this->m_flStartDisplayTime =
       *(float *)(extraout_ECX + 0x3d6fa4) -
       *(float *)(*(int *)(*(int *)(extraout_ECX + 0x5593f8) + 0x1c) + 0x2c);
  return;
}


/* CHudCoopPingIndicator::ShouldDraw at 0060bd80 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: bool ShouldDraw(CHudCoopPingIndicator * this) */

bool __thiscall CHudCoopPingIndicator::ShouldDraw(CHudCoopPingIndicator *this)

{
  char cVar1;
  bool bVar2;
  C_BasePlayer *pCVar3;
  int unaff_EBX;
  
                    /* Unresolved local var: C_BasePlayer * player@[???] */
  ___i686_get_pc_thunk_bx();
  pCVar3 = C_BasePlayer::GetLocalPlayer(-1);
  if (((pCVar3 != (C_BasePlayer *)0x0) &&
      (cVar1 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x55802c) + 0x88))
                         ((int *)**(undefined4 **)(unaff_EBX + 0x55802c)), cVar1 != '\0')) &&
     (*(float *)(**(int **)(unaff_EBX + 0x558010) + 0xc) <=
      this->m_flStartDisplayTime +
      *(float *)(*(int *)(*(int *)(unaff_EBX + 0x558914) + 0x1c) + 0x2c))) {
    bVar2 = CHudElement::ShouldDraw(&this->super_CHudElement);
    if (!bVar2) {
      return false;
    }
    cVar1 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x558018) + 0x6c))
                      ((int *)**(undefined4 **)(unaff_EBX + 0x558018));
    return cVar1 == '\0';
  }
  return false;
}


/* CHudCoopPingIndicator::DrawIndicatorHint at 0060b760 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void DrawIndicatorHint(CHudCoopPingIndicator * this) */

void __thiscall CHudCoopPingIndicator::DrawIndicatorHint(CHudCoopPingIndicator *this)

{
  vec_t *pvVar1;
  double dVar2;
  double dVar3;
  float *pfVar4;
  undefined4 *puVar5;
  longdouble lVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  Color *pCVar11;
  int unaff_EBX;
  longdouble lVar12;
  float fVar13;
  float fVar14;
  float fVar15;
  float fVar16;
  float fVar17;
  float fVar18;
  float local_b4;
  float local_a8;
  byte local_8c;
  float local_70;
  float local_68;
  float local_64;
  undefined4 local_60;
  undefined4 local_5c;
  float local_58;
  float local_54;
  undefined4 local_50;
  undefined4 local_4c;
  float local_48;
  float local_44;
  undefined4 local_40;
  undefined4 local_3c;
  float local_38;
  float local_34;
  undefined4 local_30;
  undefined4 local_2c;
  Vector local_28 [2];
  
                    /* Unresolved local var: Vector vecScreen@[???]
                       Unresolved local var: float xCenter@[???]
                       Unresolved local var: float yCenter@[???]
                       Unresolved local var: float x@[???]
                       Unresolved local var: float y@[???]
                       Unresolved local var: float flDist@[???]
                       Unresolved local var: float flInnerCircle@[???]
                       Unresolved local var: float flAng@[???]
                       Unresolved local var: float sa@[???]
                       Unresolved local var: float ca@[???]
                       Unresolved local var: float flScale@[???]
                       Unresolved local var: Vertex_t[4] verts@[???]
                       Unresolved local var: Vector2D center@[???]
                       Unresolved local var: Vector2D[2] axis@[???]
                       Unresolved local var: Color color@[???] */
  ___i686_get_pc_thunk_bx();
  pfVar4 = *(float **)(CInfoDescription::CInfoDescription + unaff_EBX + 7);
  if ((*pfVar4 == (this->m_vecPingLocation).x) &&
     (!NAN(*pfVar4) && !NAN((this->m_vecPingLocation).x))) {
    pvVar1 = &(this->m_vecPingLocation).y;
    if ((pfVar4[1] == *pvVar1) && (!NAN(pfVar4[1]) && !NAN(*pvVar1))) {
      pvVar1 = &(this->m_vecPingLocation).z;
      if ((pfVar4[2] == *pvVar1) && (!NAN(pfVar4[2]) && !NAN(*pvVar1))) {
        return;
      }
    }
  }
  ScreenTransform(&this->m_vecPingLocation,local_28);
  iVar7 = ScreenWidth();
  iVar8 = ScreenHeight();
  dVar2 = *(double *)(&DAT_003d6f1f + unaff_EBX);
  iVar9 = ScreenWidth();
  dVar3 = *(double *)(&DAT_003d6f1f + unaff_EBX);
  iVar10 = ScreenHeight();
  fVar16 = (float)((double)(iVar7 / 2) +
                  *(double *)(&DAT_003d6f1f + unaff_EBX) +
                  (double)iVar9 * dVar2 * (double)local_28[0].x) - (float)(iVar7 / 2);
  fVar17 = (float)((double)(iVar8 / 2) -
                  ((double)local_28[0].y * dVar3 * (double)iVar10 +
                  *(double *)(&DAT_003d6f1f + unaff_EBX))) - (float)(iVar8 / 2);
  fVar13 = SQRT(fVar16 * fVar16 + fVar17 * fVar17);
  iVar7 = ScreenWidth();
  iVar8 = ScreenHeight();
  if (iVar7 < iVar8) {
    iVar7 = ScreenWidth();
    local_a8 = *(float *)(&DAT_003d6a9f + unaff_EBX) *
               (float)iVar7 * *(float *)(&DAT_003d6a9b + unaff_EBX);
  }
  else {
    iVar7 = ScreenHeight();
    local_a8 = *(float *)(&DAT_003d6a9f + unaff_EBX) *
               (float)iVar7 * *(float *)(&DAT_003d6a9b + unaff_EBX);
  }
                    /* Unresolved local var: double __cosr@[???]
                       Unresolved local var: double __sinr@[???] */
  iVar8 = SUB84((double)fVar16,0);
  lVar12 = (longdouble)_atan2((double)fVar17);
  lVar6 = (longdouble)fcos((longdouble)(float)lVar12);
  lVar12 = (longdouble)fsin((longdouble)(float)lVar12);
  fVar17 = (float)lVar12;
  fVar14 = (float)lVar6;
  iVar7 = ScreenWidth();
  fVar16 = *(float *)(&DAT_003d6a9f + unaff_EBX);
  if (local_a8 <= fVar16 * fVar13) {
    iVar9 = ScreenHeight();
    fVar16 = local_a8;
  }
  else {
    iVar9 = ScreenHeight();
    fVar16 = fVar16 * fVar13;
  }
  local_70 = (float)(iVar7 / 2) + fVar14 * fVar16;
  local_b4 = (float)(iVar9 / 2);
  local_38 = *(float *)(*(int *)(unaff_EBX + 0x75f1ab) + 0x2c);
  fVar18 = (float)((uint)fVar17 ^ *(uint *)(unaff_EBX + 0x3dcd7f));
  local_60 = 0;
  local_5c = 0;
  fVar15 = local_38 * *(float *)(unaff_EBX + 0x3d6e5f);
  local_68 = local_70 + fVar14 * fVar15 + fVar18 * fVar15;
  local_64 = fVar16 * fVar17 + local_b4 + fVar15 * fVar17 + fVar14 * fVar15;
  local_50 = 0x3f800000;
  local_4c = 0;
  fVar14 = fVar14 * local_38;
  local_58 = local_68 + fVar14;
  local_54 = fVar17 * local_38 + local_64;
  local_40 = 0x3f800000;
  local_3c = 0x3f800000;
  local_38 = local_38 * fVar18;
  local_48 = local_58 + local_38;
  local_44 = local_54 + fVar14;
  local_30 = 0;
  local_2c = 0x3f800000;
  local_38 = local_68 + local_38;
  local_34 = local_64 + fVar14;
  pCVar11 = UTIL_Portal_Color((Color *)0x0,0,iVar8);
  fVar16 = *(float *)(&DAT_003d6a9f + unaff_EBX) * local_a8;
  if ((local_a8 != fVar16) || (NAN(local_a8) || NAN(fVar16))) {
    fVar13 = (fVar13 - fVar16) / (local_a8 - fVar16);
    local_8c = 0;
    if ((0.0 <= fVar13) &&
       (local_8c = 0x80,
       fVar13 < *(float *)(unaff_EBX + 0x3d6a13) || fVar13 == *(float *)(unaff_EBX + 0x3d6a13))) {
      local_8c = (byte)(int)(fVar13 * *(float *)(unaff_EBX + 0x3d79c7) + 0.0);
    }
  }
  else {
    local_8c = ~-(fVar13 - local_a8 < 0.0) & 0x80;
  }
  puVar5 = *(undefined4 **)(&DAT_00558a83 + unaff_EBX);
  (**(code **)(*(int *)*puVar5 + 0x38))
            ((int *)*puVar5,(uint)local_8c << 0x18 | (uint)pCVar11 & 0xffffff);
  (**(code **)(*(int *)*puVar5 + 0x94))((int *)*puVar5,this->m_nArrowTexture);
  (**(code **)(*(int *)*puVar5 + 0x1a0))((int *)*puVar5,4,&local_68,1);
  return;
}


/* CHudCoopPingIndicator::Paint at 0060bcd0 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void Paint(CHudCoopPingIndicator * this) */

void __thiscall CHudCoopPingIndicator::Paint(CHudCoopPingIndicator *this)

{
  DrawIndicatorHint(this);
  return;
}


/* CHudCoopPingIndicator::MsgFunc_HudPingIndicator at 0060b2d0 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void MsgFunc_HudPingIndicator(CHudCoopPingIndicator * this, bf_read *
   msg) */

void __thiscall
CHudCoopPingIndicator::MsgFunc_HudPingIndicator(CHudCoopPingIndicator *this,bf_read *msg)

{
  uint32 **ppuVar1;
  uint32 *puVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  uint32 *puVar6;
  C_BasePlayer *pCVar7;
  int iVar8;
  int unaff_EBX;
  vec_t vVar9;
  byte local_40;
  
                    /* Unresolved local var: C_Portal_Player * pPlayer@[???] */
  ___i686_get_pc_thunk_bx();
  pCVar7 = C_BasePlayer::GetLocalPlayer(-1);
  if (pCVar7 != (C_BasePlayer *)0x0) {
    this->m_flStartDisplayTime = *(float *)(**(int **)(unaff_EBX + 0x558abe) + 0xc);
    iVar3 = (msg->super_CBitRead).m_nBitsAvail;
    if (iVar3 < 0x20) {
      uVar4 = (msg->super_CBitRead).m_nInBufWord;
      puVar6 = (msg->super_CBitRead).m_pDataIn;
      puVar2 = (msg->super_CBitRead).m_pBufferEnd;
      if (puVar6 == puVar2) {
        (msg->super_CBitRead).m_nBitsAvail = 1;
        (msg->super_CBitRead).m_nInBufWord = 0;
        ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
        *ppuVar1 = *ppuVar1 + 1;
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        vVar9 = 0.0;
      }
      else {
        if (puVar6 < puVar2) {
          (msg->super_CBitRead).m_nInBufWord = *puVar6;
          (msg->super_CBitRead).m_pDataIn = puVar6 + 1;
        }
        else {
          (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
          (msg->super_CBitRead).m_nInBufWord = 0;
        }
        vVar9 = 0.0;
        if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
          iVar8 = 0x20 - iVar3;
          vVar9 = (vec_t)(((msg->super_CBitRead).m_nInBufWord &
                          *(uint *)(*(int *)(unaff_EBX + 0x558aca) + iVar8 * 4)) <<
                          ((byte)iVar3 & 0x1f) | uVar4);
          (msg->super_CBitRead).m_nBitsAvail = 0x20 - iVar8;
          local_40 = (byte)iVar8;
          puVar2 = &(msg->super_CBitRead).m_nInBufWord;
          *puVar2 = *puVar2 >> (local_40 & 0x1f);
        }
      }
    }
    else {
      uVar4 = (msg->super_CBitRead).m_nInBufWord;
      uVar5 = *(uint *)(*(int *)(unaff_EBX + 0x558aca) + 0x80);
      (msg->super_CBitRead).m_nBitsAvail = iVar3 + -0x20;
      if (iVar3 + -0x20 == 0) {
        (msg->super_CBitRead).m_nBitsAvail = 0x20;
        puVar6 = (msg->super_CBitRead).m_pDataIn;
        puVar2 = (msg->super_CBitRead).m_pBufferEnd;
        if (puVar6 == puVar2) {
          (msg->super_CBitRead).m_nBitsAvail = 1;
          (msg->super_CBitRead).m_nInBufWord = 0;
          ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
          *ppuVar1 = *ppuVar1 + 1;
        }
        else if (puVar2 < puVar6) {
          (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
          (msg->super_CBitRead).m_nInBufWord = 0;
        }
        else {
          (msg->super_CBitRead).m_nInBufWord = *puVar6;
          (msg->super_CBitRead).m_pDataIn = puVar6 + 1;
        }
      }
      else {
        puVar2 = &(msg->super_CBitRead).m_nInBufWord;
        *puVar2 = *puVar2;
      }
      vVar9 = (vec_t)(uVar4 & uVar5);
    }
    (this->m_vecPingLocation).x = vVar9;
    iVar3 = (msg->super_CBitRead).m_nBitsAvail;
    if (iVar3 < 0x20) {
      uVar4 = (msg->super_CBitRead).m_nInBufWord;
      puVar6 = (msg->super_CBitRead).m_pDataIn;
      puVar2 = (msg->super_CBitRead).m_pBufferEnd;
      if (puVar6 == puVar2) {
        (msg->super_CBitRead).m_nBitsAvail = 1;
        (msg->super_CBitRead).m_nInBufWord = 0;
        ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
        *ppuVar1 = *ppuVar1 + 1;
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        vVar9 = 0.0;
      }
      else {
        if (puVar6 < puVar2) {
          (msg->super_CBitRead).m_nInBufWord = *puVar6;
          (msg->super_CBitRead).m_pDataIn = puVar6 + 1;
        }
        else {
          (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
          (msg->super_CBitRead).m_nInBufWord = 0;
        }
        vVar9 = 0.0;
        if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
          iVar8 = 0x20 - iVar3;
          vVar9 = (vec_t)(((msg->super_CBitRead).m_nInBufWord &
                          *(uint *)(*(int *)(unaff_EBX + 0x558aca) + iVar8 * 4)) <<
                          ((byte)iVar3 & 0x1f) | uVar4);
          (msg->super_CBitRead).m_nBitsAvail = 0x20 - iVar8;
          local_40 = (byte)iVar8;
          puVar2 = &(msg->super_CBitRead).m_nInBufWord;
          *puVar2 = *puVar2 >> (local_40 & 0x1f);
        }
      }
    }
    else {
      uVar4 = (msg->super_CBitRead).m_nInBufWord;
      uVar5 = *(uint *)(*(int *)(unaff_EBX + 0x558aca) + 0x80);
      (msg->super_CBitRead).m_nBitsAvail = iVar3 + -0x20;
      if (iVar3 + -0x20 == 0) {
        (msg->super_CBitRead).m_nBitsAvail = 0x20;
        puVar6 = (msg->super_CBitRead).m_pDataIn;
        puVar2 = (msg->super_CBitRead).m_pBufferEnd;
        if (puVar6 == puVar2) {
          (msg->super_CBitRead).m_nBitsAvail = 1;
          (msg->super_CBitRead).m_nInBufWord = 0;
          ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
          *ppuVar1 = *ppuVar1 + 1;
        }
        else if (puVar2 < puVar6) {
          (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
          (msg->super_CBitRead).m_nInBufWord = 0;
        }
        else {
          (msg->super_CBitRead).m_nInBufWord = *puVar6;
          (msg->super_CBitRead).m_pDataIn = puVar6 + 1;
        }
      }
      else {
        puVar2 = &(msg->super_CBitRead).m_nInBufWord;
        *puVar2 = *puVar2;
      }
      vVar9 = (vec_t)(uVar4 & uVar5);
    }
    (this->m_vecPingLocation).y = vVar9;
    iVar3 = (msg->super_CBitRead).m_nBitsAvail;
    if (iVar3 < 0x20) {
      uVar4 = (msg->super_CBitRead).m_nInBufWord;
      puVar6 = (msg->super_CBitRead).m_pDataIn;
      puVar2 = (msg->super_CBitRead).m_pBufferEnd;
      if (puVar6 == puVar2) {
        (msg->super_CBitRead).m_nBitsAvail = 1;
        (msg->super_CBitRead).m_nInBufWord = 0;
        ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
        *ppuVar1 = *ppuVar1 + 1;
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        vVar9 = 0.0;
      }
      else {
        if (puVar6 < puVar2) {
          (msg->super_CBitRead).m_nInBufWord = *puVar6;
          (msg->super_CBitRead).m_pDataIn = puVar6 + 1;
        }
        else {
          (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
          (msg->super_CBitRead).m_nInBufWord = 0;
        }
        vVar9 = 0.0;
        if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
          iVar8 = 0x20 - iVar3;
          vVar9 = (vec_t)(((msg->super_CBitRead).m_nInBufWord &
                          *(uint *)(*(int *)(unaff_EBX + 0x558aca) + iVar8 * 4)) <<
                          ((byte)iVar3 & 0x1f) | uVar4);
          (msg->super_CBitRead).m_nBitsAvail = 0x20 - iVar8;
          local_40 = (byte)iVar8;
          puVar2 = &(msg->super_CBitRead).m_nInBufWord;
          *puVar2 = *puVar2 >> (local_40 & 0x1f);
        }
      }
    }
    else {
      uVar4 = (msg->super_CBitRead).m_nInBufWord;
      uVar5 = *(uint *)(*(int *)(unaff_EBX + 0x558aca) + 0x80);
      (msg->super_CBitRead).m_nBitsAvail = iVar3 + -0x20;
      if (iVar3 + -0x20 == 0) {
        (msg->super_CBitRead).m_nBitsAvail = 0x20;
        puVar6 = (msg->super_CBitRead).m_pDataIn;
        puVar2 = (msg->super_CBitRead).m_pBufferEnd;
        if (puVar6 == puVar2) {
          (msg->super_CBitRead).m_nBitsAvail = 1;
          (msg->super_CBitRead).m_nInBufWord = 0;
          ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
          *ppuVar1 = *ppuVar1 + 1;
        }
        else if (puVar2 < puVar6) {
          (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
          (msg->super_CBitRead).m_nInBufWord = 0;
        }
        else {
          (msg->super_CBitRead).m_nInBufWord = *puVar6;
          (msg->super_CBitRead).m_pDataIn = puVar6 + 1;
        }
      }
      else {
        puVar2 = &(msg->super_CBitRead).m_nInBufWord;
        *puVar2 = *puVar2;
      }
      vVar9 = (vec_t)(uVar4 & uVar5);
    }
    (this->m_vecPingLocation).z = vVar9;
  }
  return;
}


/* _GLOBAL__I_cl_coop_ping_indicator_scale at 000806b0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_cl_coop_ping_indicator_scale(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

