/* DWARF-guided pseudocode for game/client/portal2/hud_multiplayerbasicinfo.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* CHudMultiplayerBasicInfo::~CHudMultiplayerBasicInfo at 0060f1a0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void ~CHudMultiplayerBasicInfo(CHudMultiplayerBasicInfo * this, int
   __in_chrg) */

void __thiscall
CHudMultiplayerBasicInfo::~CHudMultiplayerBasicInfo(CHudMultiplayerBasicInfo *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5e3394);
  (this->super_CHudElement).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(unaff_EBX + 0x5e375c);
  CHudElement::~CHudElement(&this->super_CHudElement,in_stack_ffffffd8);
  vgui::EditablePanel::~EditablePanel(&this->super_EditablePanel,__in_chrg);
  return;
}


/* CHudMultiplayerBasicInfo::~CHudMultiplayerBasicInfo at 0060f580 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void ~CHudMultiplayerBasicInfo(CHudMultiplayerBasicInfo * this, int
   __in_chrg) */

void __thiscall
CHudMultiplayerBasicInfo::~CHudMultiplayerBasicInfo(CHudMultiplayerBasicInfo *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5e2fb4);
  (this->super_CHudElement).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(unaff_EBX + 0x5e337c);
  CHudElement::~CHudElement(&this->super_CHudElement,in_stack_ffffffd8);
  vgui::EditablePanel::~EditablePanel(&this->super_EditablePanel,in_stack_ffffffd8);
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5547dc) + 8))
            ((int *)**(undefined4 **)(unaff_EBX + 0x5547dc),this);
  return;
}


/* CHudMultiplayerBasicInfo::CHudMultiplayerBasicInfo at 0060ee10 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void CHudMultiplayerBasicInfo(CHudMultiplayerBasicInfo * this, char *
   pElementName) */

void __thiscall
CHudMultiplayerBasicInfo::CHudMultiplayerBasicInfo
          (CHudMultiplayerBasicInfo *this,char *pElementName)

{
  int iVar1;
  int *piVar2;
  uint uVar3;
  PanelAnimationMap *pPVar4;
  IClientMode *pIVar5;
  Panel *newParent;
  PanelKeyBindingMap *pPVar6;
  PanelKeyBindingMap *pPVar7;
  PanelAnimationMap *pPVar8;
  PanelMessageMap *pPVar9;
  PanelMessageMap *pPVar10;
  int iVar11;
  PanelAnimationMapEntry *pPVar12;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  vgui::EditablePanel::EditablePanel
            (&this->super_EditablePanel,(Panel *)0x0,(char *)(unaff_EBX + 0x3bb251));
  CHudElement::CHudElement(&this->super_CHudElement,pElementName);
  (this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5e372a);
  (this->super_CHudElement).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(unaff_EBX + 0x5e3af2);
  if (*(char *)(unaff_EBX + 0x64bf7a) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64bf7a) = 1;
    pPVar9 = vgui::FindOrAddPanelMessageMap(&DAT_003bb1f2 + unaff_EBX);
    pPVar9->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61ebf2);
    pPVar10 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x38c1f6));
    pPVar9->baseMap = pPVar10;
  }
  if (*(char *)(unaff_EBX + 0x64bf7b) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64bf7b) = 1;
    pPVar4 = FindOrAddPanelAnimationMap(&DAT_003bb1f2 + unaff_EBX);
    pPVar4->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61ebf2);
    pPVar8 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x38c1f6));
    pPVar4->baseMap = pPVar8;
  }
  if (*(char *)(unaff_EBX + 0x64bf7c) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64bf7c) = 1;
    pPVar6 = vgui::FindOrAddPanelKeyBindingMap(&DAT_003bb1f2 + unaff_EBX);
    pPVar6->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61ebf2);
    pPVar7 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x38c1f6));
    pPVar6->baseMap = pPVar7;
  }
  if (*(char *)(unaff_EBX + 0x64bf7d) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64bf7d) = 1;
    pPVar4 = FindOrAddPanelAnimationMap(&DAT_003bb1f2 + unaff_EBX);
    uVar3 = (pPVar4->entries).m_Size;
    iVar1 = uVar3 + 1;
    iVar11 = (pPVar4->entries).m_Memory.m_nAllocationCount;
    if (iVar11 < iVar1) {
      CUtlMemory<PanelAnimationMapEntry,int>::Grow
                ((CUtlMemory<PanelAnimationMapEntry,int> *)pPVar4,iVar1 - iVar11);
    }
    piVar2 = &(pPVar4->entries).m_Size;
    *piVar2 = *piVar2 + 1;
    pPVar12 = (pPVar4->entries).m_Memory.m_pMemory;
    (pPVar4->entries).m_pElements = pPVar12;
    iVar11 = ~uVar3 + (pPVar4->entries).m_Size;
    if (0 < iVar11) {
      _V_memmove(pPVar12 + iVar1,pPVar12 + uVar3,iVar11 * 0x18);
    }
    pPVar12 = (pPVar4->entries).m_Memory.m_pMemory + uVar3;
    if (pPVar12 != (PanelAnimationMapEntry *)0x0) {
      pPVar12->m_bArray = false;
      pPVar12->m_pfnLookup = *(PANELLOOKUPFUNC *)(unaff_EBX + 0x61ebe6);
      pPVar12->m_pszDefaultValue = (char *)(unaff_EBX + 0x3b1a22);
      pPVar12->m_pszType = (char *)(CDetailObjectSystem::CountFastSpritesInLeafList + unaff_EBX);
      pPVar12->m_pszVariable = (char *)(unaff_EBX + 0x3bb23c);
      pPVar12->m_pszScriptName = (char *)(unaff_EBX + 0x3bb248);
    }
  }
                    /* Unresolved local var: Panel * pParent@[???] */
  pIVar5 = GetClientMode();
  newParent = (Panel *)(*pIVar5->_vptr_IClientMode[10])(pIVar5);
  vgui::Panel::SetParent((Panel *)this,newParent);
  CHudElement::SetHiddenBits(&this->super_CHudElement,0x10);
  vgui::EditablePanel::LoadControlSettings
            (&this->super_EditablePanel,&UNK_003bb20e + unaff_EBX,(char *)0x0,(KeyValues *)0x0,
             (KeyValues *)0x0);
  return;
}


/* CHudMultiplayerBasicInfo::GetPanelClassName at 0060f170 */

char * CHudMultiplayerBasicInfo::GetPanelClassName(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (char *)(extraout_ECX + 0x3bae98);
}


/* CHudMultiplayerBasicInfo::KB_ChainToMap at 0060f210 */

void CHudMultiplayerBasicInfo::KB_ChainToMap(void)

{
  PanelKeyBindingMap *pPVar1;
  PanelKeyBindingMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x64bb79) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64bb79) = 1;
    pPVar1 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3badef));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61e7ef);
    pPVar2 = vgui::FindOrAddPanelKeyBindingMap(&UNK_0038bdf3 + unaff_EBX);
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CHudMultiplayerBasicInfo::ChainToAnimationMap at 0060f270 */

void CHudMultiplayerBasicInfo::ChainToAnimationMap(void)

{
  PanelAnimationMap *pPVar1;
  PanelAnimationMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelAnimationMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x64bb18) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64bb18) = 1;
    pPVar1 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x3bad8f));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61e78f);
    pPVar2 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x38bd93));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CHudMultiplayerBasicInfo::ChainToMap at 0060f2d0 */

void CHudMultiplayerBasicInfo::ChainToMap(void)

{
  PanelMessageMap *pPVar1;
  PanelMessageMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelMessageMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x64bab7) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64bab7) = 1;
    pPVar1 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3bad2f));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x61e72f);
    pPVar2 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x38bd33));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CHudMultiplayerBasicInfo::GetKBMap at 0060f330 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: PanelKeyBindingMap * GetKBMap(CHudMultiplayerBasicInfo * this) */

PanelKeyBindingMap * __thiscall CHudMultiplayerBasicInfo::GetKBMap(CHudMultiplayerBasicInfo *this)

{
  char *pcVar1;
  int iVar2;
  PanelKeyBindingMap *pPVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  pcVar1 = *(char **)(unaff_EBX + 0x61e6c4);
  if (*pcVar1 == '\0') {
    iVar2 = ___cxa_guard_acquire(pcVar1);
    if (iVar2 != 0) {
      pPVar3 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3baccc));
      *(PanelKeyBindingMap **)(&DAT_0064ba4c + unaff_EBX) = pPVar3;
      ___cxa_guard_release(pcVar1);
    }
  }
  return *(PanelKeyBindingMap **)(&DAT_0064ba4c + unaff_EBX);
}


/* CHudMultiplayerBasicInfo::GetAnimMap at 0060f3b0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: PanelAnimationMap * GetAnimMap(CHudMultiplayerBasicInfo * this) */

PanelAnimationMap * __thiscall CHudMultiplayerBasicInfo::GetAnimMap(CHudMultiplayerBasicInfo *this)

{
  PanelAnimationMap *pPVar1;
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  pPVar1 = FindOrAddPanelAnimationMap((char *)(extraout_ECX + 0x3bac58));
  return pPVar1;
}


/* CHudMultiplayerBasicInfo::GetMessageMap at 0060f3d0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: PanelMessageMap * GetMessageMap(CHudMultiplayerBasicInfo * this) */

PanelMessageMap * __thiscall CHudMultiplayerBasicInfo::GetMessageMap(CHudMultiplayerBasicInfo *this)

{
  char *pcVar1;
  int iVar2;
  PanelMessageMap *pPVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  pcVar1 = *(char **)(unaff_EBX + 0x61e628);
  if (*pcVar1 == '\0') {
    iVar2 = ___cxa_guard_acquire(pcVar1);
    if (iVar2 != 0) {
      pPVar3 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3bac2c));
      *(PanelMessageMap **)(unaff_EBX + 0x64b9b0) = pPVar3;
      ___cxa_guard_release(pcVar1);
    }
  }
  return *(PanelMessageMap **)(unaff_EBX + 0x64b9b0);
}


/* CHudMultiplayerBasicInfo::PanelAnimationVar_m_hTextFont::InitVar at 0060f450 */

void CHudMultiplayerBasicInfo::PanelAnimationVar_m_hTextFont::InitVar(void)

{
  int iVar1;
  int *piVar2;
  uint uVar3;
  PanelAnimationMap *this;
  int iVar4;
  PanelAnimationMapEntry *pPVar5;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x64b937) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x64b937) = 1;
                    /* Unresolved local var: PanelAnimationMap * map@[???]
                       Unresolved local var: PanelAnimationMapEntry entry@[???] */
    this = FindOrAddPanelAnimationMap(&UNK_003babac + unaff_EBX);
    uVar3 = (this->entries).m_Size;
    iVar1 = uVar3 + 1;
    iVar4 = (this->entries).m_Memory.m_nAllocationCount;
    if (iVar4 < iVar1) {
      CUtlMemory<PanelAnimationMapEntry,int>::Grow
                ((CUtlMemory<PanelAnimationMapEntry,int> *)this,iVar1 - iVar4);
    }
    piVar2 = &(this->entries).m_Size;
    *piVar2 = *piVar2 + 1;
    pPVar5 = (this->entries).m_Memory.m_pMemory;
    (this->entries).m_pElements = pPVar5;
    iVar4 = ~uVar3 + (this->entries).m_Size;
    if (0 < iVar4) {
      _V_memmove(pPVar5 + iVar1,pPVar5 + uVar3,iVar4 * 0x18);
    }
    pPVar5 = (this->entries).m_Memory.m_pMemory + uVar3;
    if (pPVar5 != (PanelAnimationMapEntry *)0x0) {
      pPVar5->m_bArray = false;
      pPVar5->m_pfnLookup = *(PANELLOOKUPFUNC *)(unaff_EBX + 0x61e5a0);
      pPVar5->m_pszDefaultValue = &UNK_003b13dc + unaff_EBX;
      pPVar5->m_pszType = (char *)(unaff_EBX + 0x3babea);
      pPVar5->m_pszVariable = (char *)(unaff_EBX + 0x3babf6);
      pPVar5->m_pszScriptName = (char *)(unaff_EBX + 0x3bac02);
    }
  }
  return;
}


/* CHudMultiplayerBasicInfo::GetVar_m_hTextFont at 0060f180 */

void * CHudMultiplayerBasicInfo::GetVar_m_hTextFont(Panel *panel)

{
  return &panel[1]._actionSignalTargetDar.
          super_CUtlVector<long_unsigned_int,CUtlMemory<long_unsigned_int,_int>_>.m_Memory.
          m_nAllocationCount;
}


/* __static_initialization_and_destruction_0 at 000807e0 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  int unaff_EBX;
  longlong lVar1;
  
  lVar1 = ___i686_get_pc_thunk_bx();
  if (lVar1 != 0xffff00000001) {
    return;
  }
  (&DAT_00c5896c)[unaff_EBX] = 0;
  (&DAT_00c5896d)[unaff_EBX] = 0;
  (&DAT_00c5896e)[unaff_EBX] = 0;
  (&DAT_00c5896f)[unaff_EBX] = 0;
  *(undefined4 *)((int)&CHudCommentary::GetKBMap()::s_pMap + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c58974 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c58978 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c5897c + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c58980 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c58984 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c58988 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c5898c + unaff_EBX) = 0;
  *(undefined4 *)((int)&CHudCommentary::GetKBMap::s_pMap + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&CHudCommentary::GetMessageMap::s_pMap + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&CHudCommentary::ChainToMap()::chained + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&CHudCommentary::PanelAnimationVar_m_iBarY::InitVar()::bAdded + unaff_EBX) =
       0x7f7fffff;
  *(undefined **)(&CHudCommentary::PanelAnimationVar_m_iSpeakersY::InitVar()::bAdded + unaff_EBX) =
       &UNK_00ae469c + unaff_EBX;
  CHudElementHelper::CHudElementHelper
            ((CHudElementHelper *)
             (&CHudCommentary::PanelAnimationVar_m_iIconY::InitVar()::bAdded + unaff_EBX),
             (_func_CHudElement_ptr *)(unaff_EBX + 0x58e8f4),100,0);
  return;
}


/* Create_CHudMultiplayerBasicInfo at 0060f0e0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */

CHudElement * Create_CHudMultiplayerBasicInfo(void)

{
  CHudMultiplayerBasicInfo *this;
  CHudElement *pCVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: void * pMem@[???] */
  this = (CHudMultiplayerBasicInfo *)
         (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x554c7c))
                   ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x554c7c),0x1b8);
  _memset(this,0,0x1b8);
  CHudMultiplayerBasicInfo::CHudMultiplayerBasicInfo(this,(char *)(unaff_EBX + 0x3baf1c));
  pCVar1 = (CHudElement *)0x0;
  if (this != (CHudMultiplayerBasicInfo *)0x0) {
    pCVar1 = &this->super_CHudElement;
  }
  return pCVar1;
}


/* CHudMultiplayerBasicInfo::CHudMultiplayerBasicInfo at 0060f0d0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void CHudMultiplayerBasicInfo(CHudMultiplayerBasicInfo * this, char *
   pElementName, CHudMultiplayerBasicInfo * this, char * pElementName) */

void __thiscall
CHudMultiplayerBasicInfo::CHudMultiplayerBasicInfo
          (CHudMultiplayerBasicInfo *this,char *pElementName,CHudMultiplayerBasicInfo *this_1,
          char *pElementName_1)

{
  CHudMultiplayerBasicInfo(this,pElementName);
  return;
}


/* CHudMultiplayerBasicInfo::Init at 0060e910 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void Init(CHudMultiplayerBasicInfo * this) */

void __thiscall CHudMultiplayerBasicInfo::Init(CHudMultiplayerBasicInfo *this)

{
  int local_14;
  int local_10 [2];
  
                    /* Unresolved local var: int screenWide@[???]
                       Unresolved local var: int screenTall@[???] */
  GetHudSize(local_10,&local_14);
  vgui::Panel::SetSize((Panel *)this,0x96,0x50);
  vgui::Panel::SetPos((Panel *)this,local_10[0] + -200,local_14 + -0x82);
  return;
}


/* CHudMultiplayerBasicInfo::Reset at 0060e7d0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void Reset(CHudMultiplayerBasicInfo * this) */

void __thiscall CHudMultiplayerBasicInfo::Reset(CHudMultiplayerBasicInfo *this)

{
                    /* WARNING: Could not recover jumptable at 0x0060e7df. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel[0xee])();
  return;
}


/* CHudMultiplayerBasicInfo::ApplySchemeSettings at 0060e860 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void ApplySchemeSettings(CHudMultiplayerBasicInfo * this, IScheme *
   scheme) */

void __thiscall
CHudMultiplayerBasicInfo::ApplySchemeSettings(CHudMultiplayerBasicInfo *this,IScheme *scheme)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel[0xd4])
            (this,unaff_EBX + 0x3bb7b8,0,0,0);
  vgui::Panel::ApplySchemeSettings((Panel *)this,scheme);
  (*(this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel[0x48])(this,1);
  (*(this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel[0x47])(this,0);
                    /* WARNING: Could not recover jumptable at 0x0060e8f0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel[0x4b])();
  return;
}


/* CHudMultiplayerBasicInfo::ShouldDraw at 0060e800 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: bool ShouldDraw(CHudMultiplayerBasicInfo * this) */

bool __thiscall CHudMultiplayerBasicInfo::ShouldDraw(CHudMultiplayerBasicInfo *this)

{
  int *piVar1;
  char cVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  piVar1 = (int *)**(int **)(unaff_EBX + 0x5555ac);
  if (piVar1 != (int *)0x0) {
    cVar2 = (**(code **)(*piVar1 + 0x88))(piVar1);
    if (cVar2 != '\0') {
                    /* Unresolved local var: C_PortalMPGameRules * g_pPortalMPGameRules@[???] */
      cVar2 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5557fc) + 0x10c))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x5557fc));
      return cVar2 == '\0';
    }
  }
  return false;
}


/* CHudMultiplayerBasicInfo::Paint at 0060e970 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void Paint(CHudMultiplayerBasicInfo * this) */

void __thiscall CHudMultiplayerBasicInfo::Paint(CHudMultiplayerBasicInfo *this)

{
  code *pcVar1;
  undefined4 *puVar2;
  C_BasePlayer *pCVar3;
  int iVar4;
  int *piVar5;
  undefined4 uVar6;
  int iVar7;
  int iVar8;
  byte bVar9;
  int unaff_EBX;
  char local_bd;
  byte local_bc;
  byte local_9e;
  byte local_9d;
  wchar_t local_9c [35];
  
                    /* Unresolved local var: C_BaseHLPlayer * pPlayer@[???]
                       Unresolved local var: Color clrPanelBg@[???]
                       Unresolved local var: Color clrText@[???]
                       Unresolved local var: int iTextY@[???] */
  ___i686_get_pc_thunk_bx();
  pCVar3 = C_BasePlayer::GetLocalPlayer(-1);
  if (pCVar3 != (C_BasePlayer *)0x0) {
    iVar4 = (**(code **)(*(int *)&(pCVar3->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                  super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                  super_C_BaseEntity.super_IClientEntity.super_IClientUnknown +
                        0x1e0))(pCVar3);
    if (iVar4 != 0) {
      piVar5 = (int *)(**(code **)(*(int *)&(pCVar3->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                            super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                            super_C_BaseEntity.super_IClientEntity.
                                            super_IClientUnknown + 0x1e0))(pCVar3);
      iVar4 = (**(code **)(*piVar5 + 0x1e4))(piVar5);
      if (iVar4 == 2) {
        local_bd = '\x14';
        local_bc = 0x4b;
        bVar9 = 0xff;
      }
      else {
        piVar5 = (int *)(**(code **)(*(int *)&(pCVar3->super_C_BaseCombatCharacter).super_C_BaseFlex
                                              .super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                              super_C_BaseEntity.super_IClientEntity.
                                              super_IClientUnknown + 0x1e0))(pCVar3);
        iVar4 = (**(code **)(*piVar5 + 0x1e4))(piVar5);
        local_bd = (iVar4 != 3) + -1;
        local_bc = (iVar4 != 3) - 1U & 0xaf;
        bVar9 = (iVar4 != 3) - 1U & 100;
      }
      (*(this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel[0x3c])
                (this,CONCAT21(CONCAT11(local_bd,local_bc),bVar9) | 0x80000000);
                    /* Unresolved local var: wchar_t[32] text@[???] */
      pcVar1 = *(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x55542f) + 0x3bc);
      uVar6 = (**(code **)(*(int *)&(pCVar3->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                    super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                    super_C_BaseEntity.super_IClientEntity.super_IClientUnknown +
                          0x148))(pCVar3);
      uVar6 = (*pcVar1)(**(undefined4 **)(unaff_EBX + 0x55542f),uVar6);
      V_snwprintf(local_9c,0x20,(wchar_t *)(&UNK_003d9b87 + unaff_EBX),uVar6);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x555873) + 0x58))
                ((int *)**(undefined4 **)(unaff_EBX + 0x555873),this->m_hTextFont);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x555873) + 0x60))
                ((int *)**(undefined4 **)(unaff_EBX + 0x555873),0xffffffff);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x555873) + 100))
                ((int *)**(undefined4 **)(unaff_EBX + 0x555873),10,10);
      puVar2 = *(undefined4 **)(unaff_EBX + 0x555873);
      piVar5 = (int *)*puVar2;
      (**(code **)(*piVar5 + 0x208))(piVar5,local_9c,0);
      piVar5 = (int *)*puVar2;
      iVar4 = (**(code **)(*piVar5 + 0x120))(piVar5,this->m_hTextFont);
                    /* Unresolved local var: wchar_t[32] text@[???] */
      pcVar1 = *(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x55542f) + 0x394);
      uVar6 = (**(code **)(*(int *)&(pCVar3->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                    super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                    super_C_BaseEntity.super_IClientEntity.super_IClientUnknown +
                          0x1e4))(pCVar3);
      uVar6 = (*pcVar1)(**(undefined4 **)(unaff_EBX + 0x55542f),uVar6);
      V_snwprintf(local_9c,0x20,(wchar_t *)(&UNK_003d9bc3 + unaff_EBX),uVar6);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x555873) + 0x58))
                ((int *)**(undefined4 **)(unaff_EBX + 0x555873),this->m_hTextFont);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x555873) + 0x60))
                ((int *)**(undefined4 **)(unaff_EBX + 0x555873),0xffffffff);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x555873) + 100))
                ((int *)**(undefined4 **)(unaff_EBX + 0x555873),10,iVar4 + 0x14);
      puVar2 = *(undefined4 **)(unaff_EBX + 0x555873);
      piVar5 = (int *)*puVar2;
      (**(code **)(*piVar5 + 0x208))(piVar5,local_9c,0);
      piVar5 = (int *)*puVar2;
      iVar7 = (**(code **)(*piVar5 + 0x120))(piVar5,this->m_hTextFont);
      iVar8 = (**(code **)(*(int *)&(pCVar3->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                    super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                    super_C_BaseEntity.super_IClientEntity.super_IClientUnknown +
                          0x1e4))(pCVar3);
      if (iVar8 == 3) {
        uVar6 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x55542f) + 0x394))
                          ((int *)**(undefined4 **)(unaff_EBX + 0x55542f),2);
        V_snwprintf(local_9c,0x20,(wchar_t *)(&UNK_003d9bff + unaff_EBX),uVar6);
        local_9e = 0;
        local_9d = 0xff;
      }
      else {
        uVar6 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x55542f) + 0x394))
                          ((int *)**(undefined4 **)(unaff_EBX + 0x55542f),3);
        V_snwprintf(local_9c,0x20,(wchar_t *)(&UNK_003d9bff + unaff_EBX),uVar6);
        local_9e = 0xff;
        local_9d = 0;
      }
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x555873) + 0x58))
                ((int *)**(undefined4 **)(unaff_EBX + 0x555873),this->m_hTextFont);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x555873) + 0x60))
                ((int *)**(undefined4 **)(unaff_EBX + 0x555873),
                 (uint)local_9e << 0x10 | 0xff000000 | (uint)local_9d);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x555873) + 100))
                ((int *)**(undefined4 **)(unaff_EBX + 0x555873),10,iVar4 + 0x1e + iVar7);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x555873) + 0x208))
                ((int *)**(undefined4 **)(unaff_EBX + 0x555873),local_9c,0);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x555873) + 0x120))
                ((int *)**(undefined4 **)(unaff_EBX + 0x555873),this->m_hTextFont);
      vgui::Panel::Paint((Panel *)this);
    }
  }
  return;
}


/* _GLOBAL__I__ZN24CHudMultiplayerBasicInfoC2EPKc at 000808b0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN24CHudMultiplayerBasicInfoC2EPKc(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

