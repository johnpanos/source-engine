/* DWARF-guided pseudocode for game/client/portal2/hud_viewfinder.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* CHudViewfinder::~CHudViewfinder at 006122f0 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void ~CHudViewfinder(CHudViewfinder * this, int __in_chrg) */

void __thiscall CHudViewfinder::~CHudViewfinder(CHudViewfinder *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_Panel).super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5e0ae4);
  (this->super_CHudElement).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(unaff_EBX + 0x5e0e44);
  CHudElement::~CHudElement(&this->super_CHudElement,in_stack_ffffffd8);
  vgui::Panel::~Panel(&this->super_Panel,in_stack_ffffffd8);
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x551a6c) + 8))
            ((int *)**(undefined4 **)(unaff_EBX + 0x551a6c),this);
  return;
}


/* CHudViewfinder::~CHudViewfinder at 006125c0 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void ~CHudViewfinder(CHudViewfinder * this, int __in_chrg) */

void __thiscall CHudViewfinder::~CHudViewfinder(CHudViewfinder *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_Panel).super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5e0814);
  (this->super_CHudElement).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(unaff_EBX + 0x5e0b74);
  CHudElement::~CHudElement(&this->super_CHudElement,in_stack_ffffffd8);
  vgui::Panel::~Panel(&this->super_Panel,__in_chrg);
  return;
}


/* CHudViewfinder::CHudViewfinder at 00610ef0 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void CHudViewfinder(CHudViewfinder * this, char * pElementName) */

void __thiscall CHudViewfinder::CHudViewfinder(CHudViewfinder *this,char *pElementName)

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
  vgui::Panel::Panel(&this->super_Panel,(Panel *)0x0,(char *)(unaff_EBX + 0x3b92a8));
  CHudElement::CHudElement(&this->super_CHudElement,pElementName);
  (this->super_Panel).super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(&UNK_005e1eea + unaff_EBX);
  (this->super_CHudElement).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(unaff_EBX + 0x5e224a);
  if (*(char *)(unaff_EBX + 0x649f1a) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x649f1a) = 1;
    pPVar5 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3b9232));
    pPVar5->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61cb22);
    pPVar6 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x395e7a));
    pPVar5->baseMap = pPVar6;
    cVar1 = *(char *)(unaff_EBX + 0x649f1b);
  }
  else {
    cVar1 = *(char *)(unaff_EBX + 0x649f1b);
  }
  if (cVar1 == '\0') {
    *(undefined1 *)(unaff_EBX + 0x649f1b) = 1;
    pPVar7 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x3b9232));
    pPVar7->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61cb22);
    pPVar8 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x395e7a));
    pPVar7->baseMap = pPVar8;
  }
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
  if (*(char *)(unaff_EBX + 0x649f1c) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x649f1c) = 1;
    pPVar2 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3b9232));
    pPVar2->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61cb22);
    pPVar3 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x395e7a));
    pPVar2->baseMap = pPVar3;
  }
                    /* Unresolved local var: Panel * pParent@[???] */
  pIVar4 = GetClientMode();
  newParent = (Panel *)(*pIVar4->_vptr_IClientMode[10])(pIVar4);
  vgui::Panel::SetParent(&this->super_Panel,newParent);
  CHudElement::SetHiddenBits(&this->super_CHudElement,0x10);
  return;
}


/* CHudViewfinder::GetPanelClassName at 006122d0 */

char * CHudViewfinder::GetPanelClassName(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (char *)(extraout_ECX + 0x3b7e58);
}


/* CHudViewfinder::KB_ChainToMap at 00612370 */

void CHudViewfinder::KB_ChainToMap(void)

{
  PanelKeyBindingMap *pPVar1;
  PanelKeyBindingMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x648a99) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x648a99) = 1;
    pPVar1 = vgui::FindOrAddPanelKeyBindingMap(&UNK_003b7daf + unaff_EBX);
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61b69f);
    pPVar2 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3949f7));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CHudViewfinder::ChainToAnimationMap at 006123d0 */

void CHudViewfinder::ChainToAnimationMap(void)

{
  PanelAnimationMap *pPVar1;
  PanelAnimationMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelAnimationMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&DAT_00648a38)[unaff_EBX] == '\0') {
    (&DAT_00648a38)[unaff_EBX] = 1;
    pPVar1 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x3b7d4f));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61b63f);
    pPVar2 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x394997));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CHudViewfinder::ChainToMap at 00612430 */

void CHudViewfinder::ChainToMap(void)

{
  PanelMessageMap *pPVar1;
  PanelMessageMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelMessageMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x6489d7) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x6489d7) = 1;
    pPVar1 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3b7cef));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61b5df);
    pPVar2 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x394937));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CHudViewfinder::GetKBMap at 00612490 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: PanelKeyBindingMap * GetKBMap(CHudViewfinder * this) */

PanelKeyBindingMap * __thiscall CHudViewfinder::GetKBMap(CHudViewfinder *this)

{
  char *pcVar1;
  int iVar2;
  PanelKeyBindingMap *pPVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  pcVar1 = *(char **)(unaff_EBX + 0x61b584);
  if (*pcVar1 == '\0') {
    iVar2 = ___cxa_guard_acquire(pcVar1);
    if (iVar2 != 0) {
      pPVar3 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3b7c8c));
      *(PanelKeyBindingMap **)(unaff_EBX + 0x64896c) = pPVar3;
      ___cxa_guard_release(pcVar1);
    }
  }
  return *(PanelKeyBindingMap **)(unaff_EBX + 0x64896c);
}


/* CHudViewfinder::GetAnimMap at 00612510 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: PanelAnimationMap * GetAnimMap(CHudViewfinder * this) */

PanelAnimationMap * __thiscall CHudViewfinder::GetAnimMap(CHudViewfinder *this)

{
  PanelAnimationMap *pPVar1;
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  pPVar1 = FindOrAddPanelAnimationMap((char *)(extraout_ECX + 0x3b7c18));
  return pPVar1;
}


/* CHudViewfinder::GetMessageMap at 00612530 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: PanelMessageMap * GetMessageMap(CHudViewfinder * this) */

PanelMessageMap * __thiscall CHudViewfinder::GetMessageMap(CHudViewfinder *this)

{
  char *pcVar1;
  int iVar2;
  PanelMessageMap *pPVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  pcVar1 = *(char **)(unaff_EBX + 0x61b4e0);
  if (*pcVar1 == '\0') {
    iVar2 = ___cxa_guard_acquire(pcVar1);
    if (iVar2 != 0) {
      pPVar3 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3b7bec));
      *(PanelMessageMap **)(unaff_EBX + 0x6488d0) = pPVar3;
      ___cxa_guard_release(pcVar1);
    }
  }
  return *(PanelMessageMap **)(unaff_EBX + 0x6488d0);
}


/* CHudViewfinder::CHudViewfinder at 00611080 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void CHudViewfinder(CHudViewfinder * this, char * pElementName,
   CHudViewfinder * this, char * pElementName) */

void __thiscall
CHudViewfinder::CHudViewfinder
          (CHudViewfinder *this,char *pElementName,CHudViewfinder *this_1,char *pElementName_1)

{
  CHudViewfinder(this,pElementName);
  return;
}


/* CHudViewfinder::Init at 00610d60 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void Init(CHudViewfinder * this) */

void __thiscall CHudViewfinder::Init(CHudViewfinder *this)

{
  undefined4 *puVar1;
  int iVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: int i@[???] */
  ___i686_get_pc_thunk_bx();
  puVar1 = *(undefined4 **)(CKeyToggleCheckButton::CKeyToggleCheckButton + unaff_EBX + 6);
  iVar2 = (**(code **)(*(int *)*puVar1 + 0xa4))((int *)*puVar1,0);
  this->m_iScopeTexture[0] = iVar2;
  iVar2 = (**(code **)(*(int *)*puVar1 + 0xa4))((int *)*puVar1,0);
  this->m_iScopeTexture[1] = iVar2;
  iVar2 = (**(code **)(*(int *)*puVar1 + 0xa4))((int *)*puVar1,0);
  this->m_iScopeTexture[2] = iVar2;
  iVar2 = (**(code **)(*(int *)*puVar1 + 0xa4))((int *)*puVar1,0);
  this->m_iScopeTexture[3] = iVar2;
  (**(code **)(*(int *)*puVar1 + 0x8c))
            ((int *)*puVar1,this->m_iScopeTexture[0],unaff_EBX + 0x3b93d1,1,0);
  (**(code **)(*(int *)*puVar1 + 0x8c))
            ((int *)*puVar1,this->m_iScopeTexture[1],unaff_EBX + 0x3b93ea,1,0);
  iVar2 = (**(code **)(*(int *)*puVar1 + 0xa4))((int *)*puVar1,0);
  this->m_nChickenIcon = iVar2;
  (**(code **)(*(int *)*puVar1 + 0x8c))((int *)*puVar1,iVar2,unaff_EBX + 0x3b940b,1,0);
  iVar2 = (**(code **)(*(int *)*puVar1 + 0xa4))((int *)*puVar1,0);
  this->m_nArrowIcon = iVar2;
  (**(code **)(*(int *)*puVar1 + 0x8c))((int *)*puVar1,iVar2,unaff_EBX + 0x3b941f,1,0);
  CHudElement::UnregisterForRenderGroup(&this->super_CHudElement,(char *)(unaff_EBX + 0x3b9431));
  return;
}


/* CHudViewfinder::ApplySchemeSettings at 00610cd0 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void ApplySchemeSettings(CHudViewfinder * this, IScheme * scheme) */

void __thiscall CHudViewfinder::ApplySchemeSettings(CHudViewfinder *this,IScheme *scheme)

{
  int local_14;
  int local_10 [2];
  
                    /* Unresolved local var: int screenWide@[???]
                       Unresolved local var: int screenTall@[???] */
  vgui::Panel::ApplySchemeSettings(&this->super_Panel,scheme);
  (*(this->super_Panel).super_IClientPanel._vptr_IClientPanel[0x48])(this,0);
  (*(this->super_Panel).super_IClientPanel._vptr_IClientPanel[0x47])(this,0);
  GetHudSize(local_10,&local_14);
  vgui::Panel::SetBounds(&this->super_Panel,0,0,local_10[0],local_14);
  return;
}


/* CHudViewfinder::ShouldDraw at 00610c80 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: bool ShouldDraw(CHudViewfinder * this) */

bool __thiscall CHudViewfinder::ShouldDraw(CHudViewfinder *this)

{
  bool bVar1;
  C_Portal_Player *this_00;
  
                    /* Unresolved local var: C_Portal_Player * pPlayer@[???] */
  this_00 = (C_Portal_Player *)C_BasePlayer::GetLocalPlayer(-1);
  if (this_00 != (C_Portal_Player *)0x0) {
    bVar1 = C_Portal_Player::IsTaunting(this_00);
    if (bVar1) {
      return this_00->m_bTauntRemoteView != false;
    }
  }
  return false;
}


/* UTIL_WorldToScreenCoords at 00610b80 */

void UTIL_WorldToScreenCoords(Vector *vecWorld,int *pScreenX,int *pScreenY)

{
  float fVar1;
  int iVar2;
  int iVar3;
  int unaff_EBX;
  Vector local_28 [2];
  
                    /* Unresolved local var: Vector vecTransform@[???] */
  ___i686_get_pc_thunk_bx();
  *pScreenX = 0;
  *pScreenY = 0;
  iVar2 = ScreenTransform(vecWorld,local_28);
  if (iVar2 == 0) {
    iVar2 = ScreenWidth();
    fVar1 = *(float *)(unaff_EBX + 0x3d167c);
    iVar3 = ScreenWidth();
    *pScreenX = (int)((float)(iVar2 / 2) + (float)iVar3 * fVar1 * local_28[0].x +
                     *(float *)(unaff_EBX + 0x3d167c));
    iVar2 = ScreenHeight();
    fVar1 = *(float *)(unaff_EBX + 0x3d1a3c);
    iVar3 = ScreenHeight();
    *pScreenY = (int)((float)(iVar2 / 2) + fVar1 * local_28[0].y * (float)iVar3 +
                     *(float *)(unaff_EBX + 0x3d167c));
  }
  return;
}


/* UTIL_GenerateBoxVertices at 006103f0 */

void UTIL_GenerateBoxVertices(Vector *vOrigin,Vector *vMins,Vector *vMaxs,Vector *pVerts)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  
                    /* Unresolved local var: Vector vecPos@[???] */
                    /* Unresolved local var: int i@[???] */
  fVar1 = vMins->y;
  fVar2 = vMins->z;
  fVar3 = vOrigin->y;
  fVar4 = vOrigin->z;
  pVerts->x = vMins->x + vOrigin->x;
  pVerts->y = fVar1 + fVar3;
  pVerts->z = fVar2 + fVar4;
  fVar1 = vMins->y;
  fVar2 = vMins->z;
  fVar3 = vOrigin->y;
  fVar4 = vOrigin->z;
  pVerts[1].x = vMaxs->x + vOrigin->x;
  pVerts[1].y = fVar1 + fVar3;
  pVerts[1].z = fVar2 + fVar4;
  fVar1 = vMaxs->y;
  fVar2 = vMins->z;
  fVar3 = vOrigin->y;
  fVar4 = vOrigin->z;
  pVerts[2].x = vMins->x + vOrigin->x;
  pVerts[2].y = fVar1 + fVar3;
  pVerts[2].z = fVar2 + fVar4;
  fVar1 = vMaxs->y;
  fVar2 = vMins->z;
  fVar3 = vOrigin->y;
  fVar4 = vOrigin->z;
  pVerts[3].x = vMaxs->x + vOrigin->x;
  pVerts[3].y = fVar1 + fVar3;
  pVerts[3].z = fVar2 + fVar4;
  fVar1 = vMins->y;
  fVar2 = vMaxs->z;
  fVar3 = vOrigin->y;
  fVar4 = vOrigin->z;
  pVerts[4].x = vMins->x + vOrigin->x;
  pVerts[4].y = fVar1 + fVar3;
  pVerts[4].z = fVar2 + fVar4;
  fVar1 = vMins->y;
  fVar2 = vMaxs->z;
  fVar3 = vOrigin->y;
  fVar4 = vOrigin->z;
  pVerts[5].x = vMaxs->x + vOrigin->x;
  pVerts[5].y = fVar1 + fVar3;
  pVerts[5].z = fVar2 + fVar4;
  fVar1 = vMaxs->y;
  fVar2 = vMaxs->z;
  fVar3 = vOrigin->y;
  fVar4 = vOrigin->z;
  pVerts[6].x = vMins->x + vOrigin->x;
  pVerts[6].y = fVar1 + fVar3;
  pVerts[6].z = fVar2 + fVar4;
  fVar1 = vMaxs->y;
  fVar2 = vMaxs->z;
  fVar3 = vOrigin->y;
  fVar4 = vOrigin->z;
  pVerts[7].x = vMaxs->x + vOrigin->x;
  pVerts[7].y = fVar1 + fVar3;
  pVerts[7].z = fVar2 + fVar4;
  return;
}


/* UTIL_WorldSpaceToScreensSpaceBounds at 00611090 */

bool UTIL_WorldSpaceToScreensSpaceBounds
               (Vector *vecCenter,Vector *mins,Vector *maxs,Vector2D *pMins,Vector2D *pMaxs)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int nMinY;
  Vector local_9c;
  Vector local_90;
  Vector local_84;
  Vector local_78;
  Vector local_6c;
  Vector local_60;
  Vector local_54;
  Vector local_48;
  float local_3c;
  float local_38;
  float local_34;
  float local_30;
  float local_2c;
  float local_28;
  int local_24;
  int local_20 [4];
  
                    /* Unresolved local var: Vector[8] vecBoxVerts@[???]
                       Unresolved local var: int nMaxX@[???]
                       Unresolved local var: int nMinX@[???]
                       Unresolved local var: int nMaxY@[???]
                       Unresolved local var: int nX@[???]
                       Unresolved local var: int nY@[???] */
                    /* Unresolved local var: Vector vecPos@[???]
                       Unresolved local var: int i@[???] */
  local_9c.x = mins->x + vecCenter->x;
  local_9c.y = mins->y + vecCenter->y;
  local_9c.z = mins->z + vecCenter->z;
  local_30 = maxs->x;
  local_90.x = local_30 + vecCenter->x;
  local_2c = maxs->y;
  local_84.y = local_2c + vecCenter->y;
  local_28 = maxs->z;
  local_6c.z = local_28 + vecCenter->z;
  local_90.y = local_9c.y;
  local_90.z = local_9c.z;
  local_84.x = local_9c.x;
  local_84.z = local_9c.z;
  local_78.x = local_90.x;
  local_78.y = local_84.y;
  local_78.z = local_9c.z;
  local_6c.x = local_9c.x;
  local_6c.y = local_9c.y;
  local_60.x = local_90.x;
  local_60.y = local_9c.y;
  local_60.z = local_6c.z;
  local_54.x = local_9c.x;
  local_54.y = local_84.y;
  local_54.z = local_6c.z;
  local_48.x = local_90.x;
  local_48.y = local_84.y;
  local_48.z = local_6c.z;
  local_3c = local_90.x;
  local_38 = local_84.y;
  local_34 = local_6c.z;
  iVar2 = ScreenWidth();
  nMinY = ScreenHeight();
  UTIL_WorldToScreenCoords(&local_9c,local_20,&local_24);
  iVar4 = local_20[0];
  iVar5 = iVar2;
  if ((local_20[0] < 1) && (iVar4 = 0, iVar5 = local_20[0], iVar2 <= local_20[0])) {
    iVar5 = iVar2;
  }
  iVar2 = local_24;
  if (local_24 < 1) {
    if (local_24 < nMinY) {
      nMinY = local_24;
      iVar2 = 0;
    }
    else {
      iVar2 = 0;
    }
  }
  UTIL_WorldToScreenCoords(&local_90,local_20,&local_24);
  iVar3 = local_20[0];
  if ((local_20[0] <= iVar4) && (iVar3 = iVar4, local_20[0] < iVar5)) {
    iVar5 = local_20[0];
  }
  iVar4 = local_24;
  iVar1 = nMinY;
  if ((local_24 <= iVar2) && (iVar4 = iVar2, iVar1 = local_24, nMinY <= local_24)) {
    iVar1 = nMinY;
  }
  nMinY = iVar1;
  UTIL_WorldToScreenCoords(&local_84,local_20,&local_24);
  iVar2 = local_20[0];
  if ((local_20[0] <= iVar3) && (iVar2 = iVar3, local_20[0] < iVar5)) {
    iVar5 = local_20[0];
  }
  iVar3 = local_24;
  iVar1 = nMinY;
  if ((local_24 <= iVar4) && (iVar3 = iVar4, iVar1 = local_24, nMinY <= local_24)) {
    iVar1 = nMinY;
  }
  nMinY = iVar1;
  UTIL_WorldToScreenCoords(&local_78,local_20,&local_24);
  iVar4 = local_20[0];
  if ((local_20[0] <= iVar2) && (iVar4 = iVar2, local_20[0] < iVar5)) {
    iVar5 = local_20[0];
  }
  iVar2 = local_24;
  iVar1 = nMinY;
  if ((local_24 <= iVar3) && (iVar2 = iVar3, iVar1 = local_24, nMinY <= local_24)) {
    iVar1 = nMinY;
  }
  nMinY = iVar1;
  UTIL_WorldToScreenCoords(&local_6c,local_20,&local_24);
  iVar3 = local_20[0];
  if ((local_20[0] <= iVar4) && (iVar3 = iVar4, local_20[0] < iVar5)) {
    iVar5 = local_20[0];
  }
  iVar4 = local_24;
  iVar1 = nMinY;
  if ((local_24 <= iVar2) && (iVar4 = iVar2, iVar1 = local_24, nMinY <= local_24)) {
    iVar1 = nMinY;
  }
  nMinY = iVar1;
  UTIL_WorldToScreenCoords(&local_60,local_20,&local_24);
  iVar2 = local_20[0];
  if ((local_20[0] <= iVar3) && (iVar2 = iVar3, local_20[0] < iVar5)) {
    iVar5 = local_20[0];
  }
  iVar3 = local_24;
  iVar1 = nMinY;
  if ((local_24 <= iVar4) && (iVar3 = iVar4, iVar1 = local_24, nMinY <= local_24)) {
    iVar1 = nMinY;
  }
  nMinY = iVar1;
  UTIL_WorldToScreenCoords(&local_54,local_20,&local_24);
  iVar4 = local_20[0];
  if ((local_20[0] <= iVar2) && (iVar4 = iVar2, local_20[0] < iVar5)) {
    iVar5 = local_20[0];
  }
  iVar2 = local_24;
  iVar1 = nMinY;
  if ((local_24 <= iVar3) && (iVar2 = iVar3, iVar1 = local_24, nMinY <= local_24)) {
    iVar1 = nMinY;
  }
  nMinY = iVar1;
  UTIL_WorldToScreenCoords(&local_48,local_20,&local_24);
  iVar3 = local_20[0];
  if ((local_20[0] <= iVar4) && (iVar3 = iVar4, local_20[0] < iVar5)) {
    iVar5 = local_20[0];
  }
  iVar4 = local_24;
  iVar1 = nMinY;
  if ((local_24 <= iVar2) && (iVar4 = iVar2, iVar1 = local_24, nMinY <= local_24)) {
    iVar1 = nMinY;
  }
  nMinY = iVar1;
  if (pMins != (Vector2D *)0x0) {
    pMins->x = (float)iVar5;
    pMins->y = (float)nMinY;
  }
  if (pMaxs != (Vector2D *)0x0) {
    pMaxs->x = (float)iVar3;
    pMaxs->y = (float)iVar4;
  }
  return true;
}


/* UTIL_EntityBoundsToSizes at 006114d0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

bool UTIL_EntityBoundsToSizes(C_BaseEntity *pTarget,int *pMinX,int *pMinY,int *pMaxX,int *pMaxY)

{
  float *pfVar1;
  Vector *vecCenter;
  Vector local_44;
  Vector local_38;
  Vector2D local_2c;
  Vector2D local_24 [2];
  
                    /* Unresolved local var: Vector vOBBMins@[???]
                       Unresolved local var: Vector vOBBMaxs@[???]
                       Unresolved local var: Vector2D mins@[???]
                       Unresolved local var: Vector2D maxs@[???] */
  (*(pTarget->m_Collision).super_CCollisionProperty.super_ICollideable._vptr_ICollideable[0xf])
            (&pTarget->m_Collision,&local_38,&local_44);
  pfVar1 = (float *)(*(pTarget->super_IClientEntity).super_IClientUnknown.super_IHandleEntity.
                      _vptr_IHandleEntity[0xd])(pTarget);
  local_44.x = local_44.x - *pfVar1;
  local_44.y = local_44.y - pfVar1[1];
  local_44.z = local_44.z - pfVar1[2];
  pfVar1 = (float *)(*(pTarget->super_IClientEntity).super_IClientUnknown.super_IHandleEntity.
                      _vptr_IHandleEntity[0xd])(pTarget);
  local_38.x = local_38.x - *pfVar1;
  local_38.y = local_38.y - pfVar1[1];
  local_38.z = local_38.z - pfVar1[2];
  vecCenter = (Vector *)
              (*(pTarget->super_IClientEntity).super_IClientUnknown.super_IHandleEntity.
                _vptr_IHandleEntity[0xd])(pTarget);
  UTIL_WorldSpaceToScreensSpaceBounds(vecCenter,&local_38,&local_44,local_24,&local_2c);
  if (pMinX != (int *)0x0) {
    *pMinX = (int)local_24[0].x;
  }
  if (pMinY != (int *)0x0) {
    *pMinY = (int)local_24[0].y;
  }
  if (pMaxX != (int *)0x0) {
    *pMaxX = (int)local_2c.x;
  }
  if (pMaxY != (int *)0x0) {
    *pMaxY = (int)local_2c.y;
  }
  return true;
}


/* CHudViewfinder::PaintLocator at 00610580 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void PaintLocator(CHudViewfinder * this, C_BaseEntity * pTarget) */

void __thiscall CHudViewfinder::PaintLocator(CHudViewfinder *this,C_BaseEntity *pTarget)

{
  double dVar1;
  undefined4 *puVar2;
  Vector *point;
  int iVar3;
  int iVar4;
  int iVar5;
  C_BasePlayer *pCVar6;
  float *pfVar7;
  int unaff_EBX;
  float fVar8;
  float fVar9;
  float fVar10;
  float fVar11;
  float fVar12;
  int local_e0;
  float local_a0;
  int local_9c;
  float local_98;
  float local_80;
  float local_7c;
  undefined4 local_78;
  undefined4 local_74;
  float local_70;
  float local_6c;
  undefined4 local_68;
  undefined4 local_64;
  float local_60;
  float local_5c;
  undefined4 local_58;
  undefined4 local_54;
  float local_50;
  float local_4c;
  undefined4 local_48;
  undefined4 local_44;
  float local_40;
  float local_3c;
  float local_38;
  undefined1 local_34 [8];
  float local_2c;
  Vector local_28 [2];
  
                    /* Unresolved local var: Vector vecScreen@[???]
                       Unresolved local var: float centerX@[???]
                       Unresolved local var: float centerY@[???]
                       Unresolved local var: C_BasePlayer * pLocalPlayer@[???]
                       Unresolved local var: float flZDiff@[???]
                       Unresolved local var: float flDistSqr@[???]
                       Unresolved local var: float flDistScale@[???]
                       Unresolved local var: float screenMax@[???]
                       Unresolved local var: int nDropShadowDepth@[???] */
  ___i686_get_pc_thunk_bx();
  point = (Vector *)
          (*(pTarget->super_IClientEntity).super_IClientUnknown.super_IHandleEntity.
            _vptr_IHandleEntity[0x6e])(pTarget);
  iVar3 = ScreenTransform(point,local_28);
  if (iVar3 == 0) {
    iVar3 = ScreenWidth();
    iVar4 = ScreenHeight();
    dVar1 = *(double *)(unaff_EBX + 0x3d20ff);
    iVar5 = ScreenWidth();
    fVar8 = (float)((double)(iVar3 / 2) +
                   (double)local_28[0].x * dVar1 * (double)iVar5 + *(double *)(unaff_EBX + 0x3d20ff)
                   );
    dVar1 = *(double *)(unaff_EBX + 0x3d20ff);
    iVar3 = ScreenHeight();
    fVar10 = (float)((double)(iVar4 / 2) -
                    ((double)local_28[0].y * dVar1 * (double)iVar3 +
                    *(double *)(unaff_EBX + 0x3d20ff)));
    pCVar6 = C_BasePlayer::GetLocalPlayer(-1);
    iVar3 = (*(pTarget->super_IClientEntity).super_IClientUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[0x6e])(pTarget);
    fVar11 = *(float *)(iVar3 + 8);
    (**(code **)(*(int *)&(pCVar6->super_C_BaseCombatCharacter).super_C_BaseFlex.
                          super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.
                          super_IClientEntity.super_IClientUnknown + 0x330))(local_34,pCVar6);
    fVar11 = fVar11 - local_2c;
    (**(code **)(*(int *)&(pCVar6->super_C_BaseCombatCharacter).super_C_BaseFlex.
                          super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.
                          super_IClientEntity.super_IClientUnknown + 0x330))(&local_40,pCVar6);
    pfVar7 = (float *)(*(pTarget->super_IClientEntity).super_IClientUnknown.super_IHandleEntity.
                        _vptr_IHandleEntity[0x6e])(pTarget);
                    /* Unresolved local var: Vector res@[???] */
    fVar12 = (((*pfVar7 - local_40) * (*pfVar7 - local_40) +
               (pfVar7[1] - local_3c) * (pfVar7[1] - local_3c) +
              (pfVar7[2] - local_38) * (pfVar7[2] - local_38)) -
             *(float *)(&DAT_003d2bbf + unaff_EBX)) / *(float *)(unaff_EBX + 0x3d802f);
    if ((*(float *)(unaff_EBX + 0x3d1cef) <= fVar12) ||
       (NAN(fVar12) || NAN(*(float *)(unaff_EBX + 0x3d1cef)))) {
      if (*(float *)(unaff_EBX + 0x3d1bf3) < fVar12) {
        local_98 = *(float *)(unaff_EBX + 0x3d1def);
        local_a0 = *(float *)(unaff_EBX + 0x3d1cd7);
        local_e0 = 0x10;
        local_9c = 0x29;
      }
      else {
        local_98 = *(float *)(unaff_EBX + 0x3d1bf3) + fVar12 * *(float *)(unaff_EBX + 0x3d493f);
        local_a0 = local_98 * *(float *)(unaff_EBX + 0x3d4257);
        local_e0 = (int)(local_98 * *(float *)(unaff_EBX + 0x3d1d5b));
        local_9c = (int)(local_98 * *(float *)(unaff_EBX + 0x3d8033));
      }
    }
    else {
      local_98 = *(float *)(unaff_EBX + 0x3d1bf3);
      local_a0 = *(float *)(unaff_EBX + 0x3d4257);
      local_e0 = 0x40;
      local_9c = 0xa4;
    }
    puVar2 = *(undefined4 **)(unaff_EBX + 0x553c63);
    (**(code **)(*(int *)*puVar2 + 0x94))((int *)*puVar2,this->m_nChickenIcon);
    (**(code **)(*(int *)*puVar2 + 0x34))((int *)*puVar2,0,0,0,local_e0);
    fVar9 = fVar10 + local_a0;
    fVar12 = *(float *)(unaff_EBX + 0x3d1d0b);
    (**(code **)(*(int *)*puVar2 + 0x9c))
              ((int *)*puVar2,(int)(fVar12 + (fVar8 - local_a0)),(int)((fVar10 - local_a0) + fVar12)
               ,(int)(fVar8 + local_a0 + fVar12),(int)(fVar9 + fVar12));
    (**(code **)(*(int *)*puVar2 + 0x34))((int *)*puVar2,0xff,0x80,0,local_9c);
    iVar3 = (int)(fVar10 - local_a0);
    (**(code **)(*(int *)*puVar2 + 0x9c))
              ((int *)*puVar2,(int)(fVar8 - local_a0),iVar3,(int)(fVar8 + local_a0),(int)fVar9);
    fVar12 = (float)((uint)fVar11 & *(uint *)(unaff_EBX + 0x3d803f));
    if (*(float *)(unaff_EBX + 0x3d2033) <= fVar12 && fVar12 != *(float *)(unaff_EBX + 0x3d2033)) {
      (**(code **)(*(int *)*puVar2 + 0x94))((int *)*puVar2,this->m_nArrowIcon);
      iVar4 = (int)(local_98 * *(float *)(unaff_EBX + 0x3d207f));
      if (*(float *)(unaff_EBX + 0x3d1cef) <= fVar11 && fVar11 != *(float *)(unaff_EBX + 0x3d1cef))
      {
        fVar11 = (float)iVar4;
        (**(code **)(*(int *)*puVar2 + 0x9c))
                  ((int *)*puVar2,(int)(fVar8 - fVar11),(int)(fVar10 - (local_a0 + fVar11)),
                   (int)(fVar8 + fVar11),iVar3);
        return;
      }
                    /* Unresolved local var: Vertex_t[4] vert@[???] */
      fVar11 = (float)iVar4;
      local_80 = fVar8 - fVar11;
      local_78 = 0;
      local_74 = 0x3f800000;
      local_70 = fVar8 + fVar11;
      local_68 = 0x3f800000;
      local_64 = 0x3f800000;
      local_5c = fVar11 + fVar10 + local_a0;
      local_58 = 0x3f800000;
      local_54 = 0;
      local_48 = 0;
      local_44 = 0;
      local_7c = fVar9;
      local_6c = fVar9;
      local_60 = local_70;
      local_50 = local_80;
      local_4c = local_5c;
      (**(code **)(*(int *)*puVar2 + 0x1a0))((int *)*puVar2,4,&local_80,1);
    }
  }
  return;
}


/* CHudViewfinder::PaintLocators at 00610b20 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void PaintLocators(CHudViewfinder * this) */

void __thiscall CHudViewfinder::PaintLocators(CHudViewfinder *this)

{
  C_Portal_Player *this_00;
  C_PortalPlayerLocalData *pCVar1;
  C_BaseEntity *pTarget;
  int iVar2;
  
                    /* Unresolved local var: C_Portal_Player * pPlayer@[???] */
  this_00 = (C_Portal_Player *)C_BasePlayer::GetLocalPlayer(-1);
  iVar2 = 0;
  do {
                    /* Unresolved local var: int i@[???]
                       Unresolved local var: int nEntityIndex@[???]
                       Unresolved local var: C_BaseEntity * pEntity@[???] */
    pCVar1 = C_Portal_Player::GetPortalPlayerLocalData(this_00);
    if (-1 < pCVar1->m_nLocatorEntityIndices[iVar2]) {
      pTarget = C_BaseEntity::Instance(pCVar1->m_nLocatorEntityIndices[iVar2]);
      if (pTarget != (C_BaseEntity *)0x0) {
        PaintLocator(this,pTarget);
      }
    }
    iVar2 = iVar2 + 1;
  } while (iVar2 != 0x10);
  return;
}


/* CHudViewfinder::Paint at 006115e0 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void Paint(CHudViewfinder * this) */

void __thiscall CHudViewfinder::Paint(CHudViewfinder *this)

{
  float fVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  C_Portal_Player *this_00;
  C_PortalPlayerLocalData *pCVar4;
  C_BaseEntity *pTarget;
  undefined4 uVar5;
  int *piVar6;
  ITexture *pIVar7;
  int iVar8;
  int iVar9;
  int unaff_EBX;
  int iVar10;
  longdouble lVar11;
  float fVar12;
  float fVar13;
  float fVar14;
  float fVar15;
  undefined1 local_49c [1024];
  float local_9c;
  float local_98;
  float local_94;
  float local_90;
  float local_8c;
  float local_88;
  float local_84;
  float local_80;
  float local_7c;
  float local_78;
  float local_74;
  float local_70;
  float local_6c;
  float local_68;
  float local_64;
  float local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  int *local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  int *local_38;
  int local_34;
  int local_30;
  int local_2c;
  int local_28;
  int local_24;
  int local_20 [4];
  
                    /* Unresolved local var: int screenWide@[???]
                       Unresolved local var: int screenTall@[???]
                       Unresolved local var: int xMid@[???]
                       Unresolved local var: int yMid@[???]
                       Unresolved local var: int wide@[???]
                       Unresolved local var: int xLeft@[???]
                       Unresolved local var: int xRight@[???]
                       Unresolved local var: int yTop@[???]
                       Unresolved local var: int yBottom@[???]
                       Unresolved local var: float uv1@[???]
                       Unresolved local var: float uv2@[???]
                       Unresolved local var: Vertex_t[4] vert@[???]
                       Unresolved local var: Vector2D uv11@[???]
                       Unresolved local var: Vector2D uv12@[???]
                       Unresolved local var: Vector2D uv21@[???]
                       Unresolved local var: Vector2D uv22@[???]
                       Unresolved local var: int tall@[???]
                       Unresolved local var: int nInnerCircle@[???]
                       Unresolved local var: int nOuterCircle@[???]
                       Unresolved local var: float circleScale@[???]
                       Unresolved local var: float flRegisterWeight@[???]
                       Unresolved local var: float flRegisterOffset@[???]
                       Unresolved local var: float flRegisterLength@[???]
                       Unresolved local var: HScheme scheme@[???]
                       Unresolved local var: FontHandle_t hTFont@[???]
                       Unresolved local var: int nAlpha@[???]
                       Unresolved local var: float flAlpha@[???]
                       Unresolved local var: wchar_t[256] unicode@[???]
                       Unresolved local var: int nWide@[???]
                       Unresolved local var: int nTall@[???] */
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: C_Portal_Player * pPlayer@[???] */
  this_00 = (C_Portal_Player *)C_BasePlayer::GetLocalPlayer(-1);
  iVar10 = 0;
  do {
    pCVar4 = C_Portal_Player::GetPortalPlayerLocalData(this_00);
    if (-1 < pCVar4->m_nLocatorEntityIndices[iVar10]) {
      pTarget = C_BaseEntity::Instance(pCVar4->m_nLocatorEntityIndices[iVar10]);
      if (pTarget != (C_BaseEntity *)0x0) {
        PaintLocator(this,pTarget);
      }
    }
    iVar10 = iVar10 + 1;
  } while (iVar10 != 0x10);
  local_38 = (int *)(**(code **)(*(int *)**(undefined4 **)(&DAT_0055291b + unaff_EBX) + 0x1a0))
                              ((int *)**(undefined4 **)(&DAT_0055291b + unaff_EBX));
  if (local_38 != (int *)0x0) {
    (**(code **)(*local_38 + 8))(local_38);
  }
  (**(code **)(*local_38 + 0x9c))(local_38,&local_3c,&local_40,&local_44,&local_48);
  if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x552f03) + 0x1c) + 0x30) != 0) {
    local_4c = (int *)(**(code **)(*(int *)**(undefined4 **)(&DAT_0055291b + unaff_EBX) + 0x1a0))
                                ((int *)**(undefined4 **)(&DAT_0055291b + unaff_EBX));
    if (local_4c != (int *)0x0) {
      (**(code **)(*local_4c + 8))(local_4c);
    }
    pIVar7 = GetPowerOfTwoFrameBufferTexture();
                    /* Unresolved local var: Rect_t rect@[???] */
    local_5c = local_3c;
    local_58 = local_40;
    local_54 = local_44;
    local_50 = local_48;
    (**(code **)(*local_4c + 0x1bc))(local_4c,pIVar7,0,&local_5c,0);
    **(undefined4 **)(unaff_EBX + 0x552f0b) = **(undefined4 **)(unaff_EBX + 0x552f0f);
    **(undefined4 **)(unaff_EBX + 0x552eff) = *(undefined4 *)(**(int **)(unaff_EBX + 0x5527ab) + 4);
    (**(code **)(*local_4c + 0x48))(local_4c,pIVar7,0);
    if (local_4c != (int *)0x0) {
      (**(code **)(*local_4c + 0xc))(local_4c);
    }
    if (local_4c != (int *)0x0) {
      (**(code **)(*local_4c + 4))(local_4c);
    }
  }
  if (local_38 != (int *)0x0) {
    (**(code **)(*local_38 + 0xc))(local_38);
  }
  if (local_38 != (int *)0x0) {
    (**(code **)(*local_38 + 4))(local_38);
  }
  GetHudSize(local_20,&local_24);
  iVar10 = local_24;
  iVar8 = local_20[0] / 2;
  iVar9 = local_24 / 2;
  local_28 = local_20[0];
  puVar2 = *(undefined4 **)
            (CGameUiSetActiveSplitScreenPlayerGuard::~CGameUiSetActiveSplitScreenPlayerGuard +
            unaff_EBX + 3);
  (**(code **)(*(int *)*puVar2 + 0x34))((int *)*puVar2,0,0,0,0xff);
  (**(code **)(*(int *)*puVar2 + 0x94))((int *)*puVar2,this->m_iScopeTexture[0]);
  local_9c = 0.0;
  local_98 = 0.0;
  local_94 = 0.0;
  local_90 = 0.0;
  fVar12 = (float)iVar8;
  local_88 = 0.0;
  fVar1 = *(float *)(unaff_EBX + 0x3d0b93);
  local_80 = 0.0;
  fVar13 = (float)iVar9;
  local_6c = 0.0;
  local_64 = 0.0;
  local_8c = fVar12;
  local_84 = fVar1;
  local_7c = fVar12;
  local_78 = fVar13;
  local_74 = fVar1;
  local_70 = fVar1;
  local_68 = fVar13;
  local_60 = fVar1;
  (**(code **)(*(int *)*puVar2 + 0x1a0))((int *)*puVar2,4,&local_9c,1);
  local_98 = 0.0;
  local_90 = 0.0;
  fVar14 = (float)(iVar8 * 2);
  local_88 = 0.0;
  local_84 = 0.0;
  local_80 = 0.0;
  local_74 = 0.0;
  local_9c = fVar12;
  local_94 = fVar1;
  local_8c = fVar14;
  local_7c = fVar14;
  local_78 = fVar13;
  local_70 = fVar1;
  local_6c = fVar12;
  local_68 = fVar13;
  local_64 = fVar1;
  local_60 = fVar1;
  (**(code **)(*(int *)*puVar2 + 0x1a0))((int *)*puVar2,4,&local_9c,1);
  local_84 = 0.0;
  fVar15 = (float)iVar10;
  local_74 = 0.0;
  local_70 = 0.0;
  local_60 = 0.0;
  local_9c = fVar12;
  local_98 = fVar13;
  local_94 = fVar1;
  local_90 = fVar1;
  local_8c = fVar14;
  local_88 = fVar13;
  local_80 = fVar1;
  local_7c = fVar14;
  local_78 = fVar15;
  local_6c = fVar12;
  local_68 = fVar15;
  local_64 = fVar1;
  (**(code **)(*(int *)*puVar2 + 0x1a0))((int *)*puVar2,4,&local_9c,1);
  local_9c = 0.0;
  local_94 = 0.0;
  local_70 = 0.0;
  local_6c = 0.0;
  local_64 = 0.0;
  local_60 = 0.0;
  local_98 = fVar13;
  local_90 = fVar1;
  local_8c = fVar12;
  local_88 = fVar13;
  local_84 = fVar1;
  local_80 = fVar1;
  local_7c = fVar12;
  local_78 = fVar15;
  local_74 = fVar1;
  local_68 = fVar15;
  (**(code **)(*(int *)*puVar2 + 0x1a0))((int *)*puVar2,4,&local_9c,1);
  vgui::Panel::GetSize(&this->super_Panel,&local_28,&local_2c);
  iVar10 = (int)((float)local_24 * *(float *)(unaff_EBX + 0x3d106b));
  (**(code **)(*(int *)*puVar2 + 0x194))((int *)*puVar2,iVar8,iVar9,iVar10,0x40);
  (**(code **)(*(int *)*puVar2 + 0x194))
            ((int *)*puVar2,iVar8,iVar9,(int)((float)iVar10 * *(float *)(unaff_EBX + 0x3d1013)),0x40
            );
  (**(code **)(*(int *)*puVar2 + 0x34))((int *)*puVar2,0,0,0,0x40);
  (**(code **)(*(int *)*puVar2 + 0x94))((int *)*puVar2,this->m_iScopeTexture[1]);
  fVar14 = (float)local_24 * *(float *)(unaff_EBX + 0x3d139f);
  fVar15 = fVar12 - fVar14;
  local_94 = 0.0;
  fVar12 = fVar12 + fVar14;
  local_78 = fVar13 + fVar14;
  local_70 = 0.0;
  local_64 = 0.0;
  local_60 = 0.0;
  local_9c = fVar15;
  local_98 = fVar13;
  local_90 = fVar1;
  local_8c = fVar12;
  local_88 = fVar13;
  local_84 = fVar1;
  local_80 = fVar1;
  local_7c = fVar12;
  local_74 = fVar1;
  local_6c = fVar15;
  local_68 = local_78;
  (**(code **)(*(int *)*puVar2 + 0x1a0))((int *)*puVar2,4,&local_9c,1);
  (**(code **)(*(int *)*puVar2 + 0x34))((int *)*puVar2,0,0,0,0x20);
  local_98 = fVar13 - fVar14;
  local_90 = 0.0;
  local_84 = 0.0;
  local_80 = 0.0;
  local_74 = 0.0;
  local_9c = fVar15;
  local_94 = fVar1;
  local_8c = fVar12;
  local_88 = local_98;
  local_7c = fVar12;
  local_78 = fVar13;
  local_70 = fVar1;
  local_6c = fVar15;
  local_68 = fVar13;
  local_64 = fVar1;
  local_60 = fVar1;
  (**(code **)(*(int *)*puVar2 + 0x1a0))((int *)*puVar2,4,&local_9c,1);
  fVar13 = (float)local_20[0];
  fVar14 = fVar13 * *(float *)(unaff_EBX + 0x3d2a67);
  (**(code **)(*(int *)*puVar2 + 0x34))((int *)*puVar2,0,0,0,0xff);
  fVar12 = fVar14 + fVar1;
  iVar10 = (int)fVar12;
  fVar13 = fVar14 + fVar13 * *(float *)(unaff_EBX + 0x3d0c97);
  iVar8 = (int)fVar13;
  iVar9 = (int)fVar14;
  (**(code **)(*(int *)*puVar2 + 0x3c))((int *)*puVar2,iVar9,iVar9,iVar8,iVar10);
  (**(code **)(*(int *)*puVar2 + 0x3c))((int *)*puVar2,iVar9,iVar9,iVar10,iVar8);
  (**(code **)(*(int *)*puVar2 + 0x3c))
            ((int *)*puVar2,iVar9,(int)((float)local_24 - fVar14),iVar8,
             (int)(fVar1 + ((float)local_24 - fVar14)));
  (**(code **)(*(int *)*puVar2 + 0x3c))
            ((int *)*puVar2,iVar9,(int)((float)local_24 - fVar13),iVar10,
             (int)((float)local_24 - fVar14));
  (**(code **)(*(int *)*puVar2 + 0x3c))
            ((int *)*puVar2,(int)((float)local_20[0] - fVar13),iVar9,
             (int)((float)local_20[0] - fVar14),iVar10);
  (**(code **)(*(int *)*puVar2 + 0x3c))
            ((int *)*puVar2,(int)((float)local_20[0] - fVar12),iVar9,
             (int)((float)local_20[0] - fVar14),iVar8);
  (**(code **)(*(int *)*puVar2 + 0x3c))
            ((int *)*puVar2,(int)((float)local_20[0] - fVar13),(int)((float)local_24 - fVar12),
             (int)((float)local_20[0] - fVar14),(int)((float)local_24 - fVar14));
  (**(code **)(*(int *)*puVar2 + 0x3c))
            ((int *)*puVar2,(int)((float)local_20[0] - fVar12),(int)((float)local_24 - fVar13),
             (int)((float)local_20[0] - fVar14),(int)((float)local_24 - fVar14));
  puVar3 = *(undefined4 **)(&DAT_00552bff + unaff_EBX);
  piVar6 = (int *)*puVar3;
  uVar5 = (**(code **)(*piVar6 + 0x18))(piVar6,unaff_EBX + 0x38a2e3);
  piVar6 = (int *)*puVar3;
  piVar6 = (int *)(**(code **)(*piVar6 + 0x24))(piVar6,uVar5);
  uVar5 = (**(code **)(*piVar6 + 0x10))(piVar6,unaff_EBX + 0x3a0dc1,1);
  lVar11 = (longdouble)
           _sinf(*(float *)(unaff_EBX + 0x3d101f) *
                 *(float *)(**(int **)(unaff_EBX + 0x5527ab) + 0xc));
  fVar1 = *(float *)(unaff_EBX + 0x3d0d8f);
  fVar13 = *(float *)(unaff_EBX + 0x3d0d8f);
  (**(code **)(*(int *)*puVar2 + 0x58))((int *)*puVar2,uVar5);
  (**(code **)(*(int *)*puVar2 + 0x60))
            ((int *)*puVar2,
             (uint)(byte)(((float)lVar11 < fVar13 || (float)lVar11 == fVar1) - 1) << 0x18 | 0xff);
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x552c17) + 0x30))
            ((int *)**(undefined4 **)(unaff_EBX + 0x552c17),unaff_EBX + 0x3b8bc4,local_49c,0x400);
  (**(code **)(*(int *)*puVar2 + 0x134))((int *)*puVar2,uVar5,local_49c,&local_30,&local_34);
  (**(code **)(*(int *)*puVar2 + 100))
            ((int *)*puVar2,(int)(((float)local_20[0] - fVar14) - (float)(local_30 + 0x20)),
             (int)(((float)local_24 - fVar14) - (float)(local_34 + 0x10)));
  (**(code **)(*(int *)*puVar2 + 0x208))((int *)*puVar2,local_49c,0);
  return;
}


/* __static_initialization_and_destruction_0 at 00080a10 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  int unaff_EBX;
  longlong lVar1;
  
  lVar1 = ___i686_get_pc_thunk_bx();
  if (lVar1 != 0xffff00000001) {
    return;
  }
  (&DAT_00c587cf)[unaff_EBX] = 0;
  (&DAT_00c587d0)[unaff_EBX] = 0;
  (&DAT_00c587d1)[unaff_EBX] = 0;
  (&DAT_00c587d2)[unaff_EBX] = 0;
  *(undefined4 *)(&DAT_00c587d3 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c587d7 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c587db + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c587df + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&g_nKillCamMode + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&g_nKillCamTarget1 + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&g_nKillCamTarget2 + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&C_BaseTempEntity::s_pTempEntities + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&CUtlLinkedList<CPrecipitationParticle,short_unsigned_int,false,short_unsigned_int,CUtlMemory<UtlLinkedListElem_t<CPrecipitationParticle,_short_unsigned_int>,_short_unsigned_int>_>
          ::AllocInternal::__executeCount_1 + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&CUtlLinkedList<CPrecipitationParticle,short_unsigned_int,false,short_unsigned_int,CUtlMemory<UtlLinkedListElem_t<CPrecipitationParticle,_short_unsigned_int>,_short_unsigned_int>_>
          ::AllocInternal::__executeCount + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&g_flSplashScale + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&g_flSplashLifetime + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined **)((int)&g_flSplashAlpha + unaff_EBX + 3) = &UNK_00ae446f + unaff_EBX;
  return;
}


/* _GLOBAL__I__ZN14CHudViewfinderC2EPKc at 00080ab0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN14CHudViewfinderC2EPKc(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

