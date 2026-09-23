/* DWARF-guided pseudocode for game/client/portal2/hud_multiplayerbasicinfo.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* CHudMultiplayerBasicInfo::~CHudMultiplayerBasicInfo at 00604e40 */

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
       (_func_int_varargs **)(unaff_EBX + 0x5b8334);
  (this->super_CHudElement).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(unaff_EBX + 0x5b86fc);
  CHudElement::~CHudElement(&this->super_CHudElement,in_stack_ffffffd8);
  vgui::EditablePanel::~EditablePanel(&this->super_EditablePanel,__in_chrg);
  return;
}


/* CHudMultiplayerBasicInfo::~CHudMultiplayerBasicInfo at 00605220 */

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
       (_func_int_varargs **)(unaff_EBX + 0x5b7f54);
  (this->super_CHudElement).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(unaff_EBX + 0x5b831c);
  CHudElement::~CHudElement(&this->super_CHudElement,in_stack_ffffffd8);
  vgui::EditablePanel::~EditablePanel(&this->super_EditablePanel,in_stack_ffffffd8);
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52baf0) + 8))
            ((int *)**(undefined4 **)(unaff_EBX + 0x52baf0),this);
  return;
}


/* CHudMultiplayerBasicInfo::CHudMultiplayerBasicInfo at 00604ab0 */

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
            (&this->super_EditablePanel,(Panel *)0x0,(char *)(unaff_EBX + 0x39a4d1));
  CHudElement::CHudElement(&this->super_CHudElement,pElementName);
  (this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5b86ca);
  (this->super_CHudElement).super_CGameEventListener.super_IGameEventListener2.
  _vptr_IGameEventListener2 = (_func_int_varargs **)(unaff_EBX + 0x5b8a92);
  if (*(char *)(unaff_EBX + 0x6202ba) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x6202ba) = 1;
    pPVar9 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x39a47e));
    pPVar9->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x5f2fd2);
    pPVar10 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x36bb0e));
    pPVar9->baseMap = pPVar10;
  }
  if (*(char *)(unaff_EBX + 0x6202bb) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x6202bb) = 1;
    pPVar4 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x39a47e));
    pPVar4->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x5f2fd2);
    pPVar8 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x36bb0e));
    pPVar4->baseMap = pPVar8;
  }
  if (*(char *)(unaff_EBX + 0x6202bc) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x6202bc) = 1;
    pPVar6 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x39a47e));
    pPVar6->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x5f2fd2);
    pPVar7 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x36bb0e));
    pPVar6->baseMap = pPVar7;
  }
  if (*(char *)(unaff_EBX + 0x6202bd) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x6202bd) = 1;
    pPVar4 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x39a47e));
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
      pPVar12->m_pfnLookup = *(PANELLOOKUPFUNC *)(unaff_EBX + 0x5f2fc6);
      pPVar12->m_pszDefaultValue = (char *)(unaff_EBX + 0x390ec6);
      pPVar12->m_pszType = (char *)(unaff_EBX + 0x39aa52);
      pPVar12->m_pszVariable = (char *)(unaff_EBX + 0x39a4bc);
      pPVar12->m_pszScriptName = (char *)(unaff_EBX + 0x39a4c8);
    }
  }
                    /* Unresolved local var: Panel * pParent@[???] */
  pIVar5 = GetClientMode();
  newParent = (Panel *)(*pIVar5->_vptr_IClientMode[10])(pIVar5);
  vgui::Panel::SetParent((Panel *)this,newParent);
  CHudElement::SetHiddenBits(&this->super_CHudElement,0x10);
  vgui::EditablePanel::LoadControlSettings
            (&this->super_EditablePanel,(char *)(unaff_EBX + 0x39a49a),(char *)0x0,(KeyValues *)0x0,
             (KeyValues *)0x0);
  return;
}


/* CHudMultiplayerBasicInfo::GetPanelClassName at 00604e10 */

char * CHudMultiplayerBasicInfo::GetPanelClassName(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (char *)(extraout_ECX + 0x39a124);
}


/* CHudMultiplayerBasicInfo::KB_ChainToMap at 00604eb0 */

void CHudMultiplayerBasicInfo::KB_ChainToMap(void)

{
  PanelKeyBindingMap *pPVar1;
  PanelKeyBindingMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelKeyBindingMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&DAT_0061feb9)[unaff_EBX] == '\0') {
    (&DAT_0061feb9)[unaff_EBX] = 1;
    pPVar1 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x39a07b));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x5f2bcf);
    pPVar2 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x36b70b));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CHudMultiplayerBasicInfo::ChainToAnimationMap at 00604f10 */

void CHudMultiplayerBasicInfo::ChainToAnimationMap(void)

{
  PanelAnimationMap *pPVar1;
  PanelAnimationMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelAnimationMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x61fe58) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x61fe58) = 1;
    pPVar1 = FindOrAddPanelAnimationMap(&UNK_0039a01b + unaff_EBX);
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x5f2b6f);
    pPVar2 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x36b6ab));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CHudMultiplayerBasicInfo::ChainToMap at 00604f70 */

void CHudMultiplayerBasicInfo::ChainToMap(void)

{
  PanelMessageMap *pPVar1;
  PanelMessageMap *pPVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: PanelMessageMap * map@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)(unaff_EBX + 0x61fdf7) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x61fdf7) = 1;
    pPVar1 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x399fbb));
    pPVar1->pfnClassName = *(_func_char_ptr **)(unaff_EBX + 0x5f2b0f);
    pPVar2 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x36b64b));
    pPVar1->baseMap = pPVar2;
  }
  return;
}


/* CHudMultiplayerBasicInfo::GetKBMap at 00604fd0 */

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
  pcVar1 = *(char **)(unaff_EBX + 0x5f2aa4);
  if (*pcVar1 == '\0') {
    iVar2 = ___cxa_guard_acquire(pcVar1);
    if (iVar2 != 0) {
      pPVar3 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x399f58));
      *(PanelKeyBindingMap **)(unaff_EBX + 0x61fd8c) = pPVar3;
      ___cxa_guard_release(pcVar1);
    }
  }
  return *(PanelKeyBindingMap **)(unaff_EBX + 0x61fd8c);
}


/* CHudMultiplayerBasicInfo::GetAnimMap at 00605050 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: PanelAnimationMap * GetAnimMap(CHudMultiplayerBasicInfo * this) */

PanelAnimationMap * __thiscall CHudMultiplayerBasicInfo::GetAnimMap(CHudMultiplayerBasicInfo *this)

{
  PanelAnimationMap *pPVar1;
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  pPVar1 = FindOrAddPanelAnimationMap((char *)(extraout_ECX + 0x399ee4));
  return pPVar1;
}


/* CHudMultiplayerBasicInfo::GetMessageMap at 00605070 */

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
  pcVar1 = *(char **)(unaff_EBX + 0x5f2a08);
  if (*pcVar1 == '\0') {
    iVar2 = ___cxa_guard_acquire(pcVar1);
    if (iVar2 != 0) {
      pPVar3 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x399eb8));
      *(PanelMessageMap **)(unaff_EBX + 0x61fcf0) = pPVar3;
      ___cxa_guard_release(pcVar1);
    }
  }
  return *(PanelMessageMap **)(unaff_EBX + 0x61fcf0);
}


/* CHudMultiplayerBasicInfo::PanelAnimationVar_m_hTextFont::InitVar at 006050f0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */

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
  if (CPrediction::ShowPredictionListEntry[unaff_EBX + 7] == (code)0x0) {
    CPrediction::ShowPredictionListEntry[unaff_EBX + 7] = (code)0x1;
                    /* Unresolved local var: PanelAnimationMap * map@[???]
                       Unresolved local var: PanelAnimationMapEntry entry@[???] */
    this = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x399e38));
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
      pPVar5->m_pfnLookup = *(PANELLOOKUPFUNC *)(unaff_EBX + 0x5f2980);
      pPVar5->m_pszDefaultValue = (char *)(unaff_EBX + 0x390880);
      pPVar5->m_pszType = (char *)(unaff_EBX + 0x39a40c);
      pPVar5->m_pszVariable = (char *)(unaff_EBX + 0x399e76);
      pPVar5->m_pszScriptName = (char *)(unaff_EBX + 0x399e82);
    }
  }
  return;
}


/* CHudMultiplayerBasicInfo::GetVar_m_hTextFont at 00604e20 */

void * CHudMultiplayerBasicInfo::GetVar_m_hTextFont(Panel *panel)

{
  return &panel[1]._actionSignalTargetDar.
          super_CUtlVector<long_unsigned_int,CUtlMemory<long_unsigned_int,_int>_>.m_Memory.
          m_nAllocationCount;
}


/* __static_initialization_and_destruction_0 at 000820a0 */

/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  int unaff_EBX;
  longlong lVar1;
  
  lVar1 = ___i686_get_pc_thunk_bx();
  if (lVar1 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4c].m_pPrev + unaff_EBX)
       = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4c].m_pPrev + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4c].m_pPrev + unaff_EBX + 2) = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4c].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4c].m_pNext + unaff_EBX)
       = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4d].m_pEntity + unaff_EBX) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4d].m_SerialNumber + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4d].m_pPrev + unaff_EBX)
       = 0x7f7fffff;
  *(undefined4 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4d].m_pNext + unaff_EBX)
       = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4e].m_pEntity + unaff_EBX) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4e].m_SerialNumber + unaff_EBX) = 0;
  *(undefined4 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4e].m_pPrev + unaff_EBX)
       = 0;
  *(undefined4 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4e].m_pNext + unaff_EBX)
       = 0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4f].m_pEntity + unaff_EBX) = 0x7f7fffff
  ;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4f].m_SerialNumber + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4f].m_pPrev + unaff_EBX)
       = 0x7f7fffff;
  *(undefined **)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x4f].m_pNext + unaff_EBX)
       = &UNK_00aafd5c + unaff_EBX;
  CHudElementHelper::CHudElementHelper
            ((CHudElementHelper *)
             ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x50].m_pEntity + unaff_EBX),
             (_func_CHudElement_ptr *)(unaff_EBX + 0x582cd4),100,0);
  return;
}


/* Create_CHudMultiplayerBasicInfo at 00604d80 */

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
         (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x52bf90))
                   ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x52bf90),0x1b8);
  _memset(this,0,0x1b8);
  CHudMultiplayerBasicInfo::CHudMultiplayerBasicInfo(this,(char *)(unaff_EBX + 0x39a1a8));
  pCVar1 = (CHudElement *)0x0;
  if (this != (CHudMultiplayerBasicInfo *)0x0) {
    pCVar1 = &this->super_CHudElement;
  }
  return pCVar1;
}


/* CHudMultiplayerBasicInfo::CHudMultiplayerBasicInfo at 00604d70 */

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


/* CHudMultiplayerBasicInfo::Init at 006045b0 */

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


/* CHudMultiplayerBasicInfo::Reset at 00604450 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void Reset(CHudMultiplayerBasicInfo * this) */

void __thiscall CHudMultiplayerBasicInfo::Reset(CHudMultiplayerBasicInfo *this)

{
                    /* WARNING: Could not recover jumptable at 0x0060445f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel[0xee])();
  return;
}


/* CHudMultiplayerBasicInfo::ApplySchemeSettings at 00604500 */

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
            (this,unaff_EBX + 0x39aa44,0,0,0);
  vgui::Panel::ApplySchemeSettings((Panel *)this,scheme);
  (*(this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel[0x48])(this,1);
  (*(this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel[0x47])(this,0);
                    /* WARNING: Could not recover jumptable at 0x00604590. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel[0x4b])();
  return;
}


/* CHudMultiplayerBasicInfo::ShouldDraw at 00604480 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: bool ShouldDraw(CHudMultiplayerBasicInfo * this) */

bool __thiscall CHudMultiplayerBasicInfo::ShouldDraw(CHudMultiplayerBasicInfo *this)

{
  int *piVar1;
  int iVar2;
  char cVar3;
  int *piVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  piVar4 = *(int **)(&DAT_0052c8df + unaff_EBX);
  piVar1 = (int *)*piVar4;
  if (piVar1 != (int *)0x0) {
    cVar3 = (**(code **)(*piVar1 + 0x88))(piVar1);
    if (cVar3 != '\0') {
      iVar2 = *piVar4;
      piVar4 = (int *)0x0;
      if (iVar2 != 0) {
        piVar4 = (int *)___dynamic_cast(iVar2,*(undefined4 *)(unaff_EBX + 0x52cb0f),
                                        *(undefined4 *)(unaff_EBX + 0x52d05b),0);
      }
      cVar3 = (**(code **)(*piVar4 + 0x10c))(piVar4);
      return cVar3 == '\0';
    }
  }
  return false;
}


/* CHudMultiplayerBasicInfo::Paint at 00604610 */

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
                        0x1dc))(pCVar3);
    if (iVar4 != 0) {
      piVar5 = (int *)(**(code **)(*(int *)&(pCVar3->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                            super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                            super_C_BaseEntity.super_IClientEntity.
                                            super_IClientUnknown + 0x1dc))(pCVar3);
      iVar4 = (**(code **)(*piVar5 + 0x1e0))(piVar5);
      if (iVar4 == 2) {
        local_bd = '\x14';
        local_bc = 0x4b;
        bVar9 = 0xff;
      }
      else {
        piVar5 = (int *)(**(code **)(*(int *)&(pCVar3->super_C_BaseCombatCharacter).super_C_BaseFlex
                                              .super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                              super_C_BaseEntity.super_IClientEntity.
                                              super_IClientUnknown + 0x1dc))(pCVar3);
        iVar4 = (**(code **)(*piVar5 + 0x1e0))(piVar5);
        local_bd = (iVar4 != 3) + -1;
        local_bc = (iVar4 != 3) - 1U & 0xaf;
        bVar9 = (iVar4 != 3) - 1U & 100;
      }
      (*(this->super_EditablePanel).super_Panel.super_IClientPanel._vptr_IClientPanel[0x3c])
                (this,CONCAT21(CONCAT11(local_bd,local_bc),bVar9) | 0x80000000);
                    /* Unresolved local var: wchar_t[32] text@[???] */
      pcVar1 = *(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52c743) + 0x3b8);
      uVar6 = (**(code **)(*(int *)&(pCVar3->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                    super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                    super_C_BaseEntity.super_IClientEntity.super_IClientUnknown +
                          0x144))(pCVar3);
      uVar6 = (*pcVar1)(**(undefined4 **)(unaff_EBX + 0x52c743),uVar6);
      V_snwprintf(local_9c,0x20,(wchar_t *)(unaff_EBX + 0x3b83f7),uVar6);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52cb7f) + 0x50))
                ((int *)**(undefined4 **)(unaff_EBX + 0x52cb7f),this->m_hTextFont);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52cb7f) + 0x58))
                ((int *)**(undefined4 **)(unaff_EBX + 0x52cb7f),0xffffffff);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52cb7f) + 0x5c))
                ((int *)**(undefined4 **)(unaff_EBX + 0x52cb7f),10,10);
      puVar2 = *(undefined4 **)(unaff_EBX + 0x52cb7f);
      piVar5 = (int *)*puVar2;
      (**(code **)(*piVar5 + 0x200))(piVar5,local_9c,0);
      piVar5 = (int *)*puVar2;
      iVar4 = (**(code **)(*piVar5 + 0x118))(piVar5,this->m_hTextFont);
                    /* Unresolved local var: wchar_t[32] text@[???] */
      pcVar1 = *(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52c743) + 0x390);
      uVar6 = (**(code **)(*(int *)&(pCVar3->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                    super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                    super_C_BaseEntity.super_IClientEntity.super_IClientUnknown +
                          0x1e0))(pCVar3);
      uVar6 = (*pcVar1)(**(undefined4 **)(unaff_EBX + 0x52c743),uVar6);
      V_snwprintf(local_9c,0x20,(wchar_t *)(unaff_EBX + 0x3b8433),uVar6);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52cb7f) + 0x50))
                ((int *)**(undefined4 **)(unaff_EBX + 0x52cb7f),this->m_hTextFont);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52cb7f) + 0x58))
                ((int *)**(undefined4 **)(unaff_EBX + 0x52cb7f),0xffffffff);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52cb7f) + 0x5c))
                ((int *)**(undefined4 **)(unaff_EBX + 0x52cb7f),10,iVar4 + 0x14);
      puVar2 = *(undefined4 **)(unaff_EBX + 0x52cb7f);
      piVar5 = (int *)*puVar2;
      (**(code **)(*piVar5 + 0x200))(piVar5,local_9c,0);
      piVar5 = (int *)*puVar2;
      iVar7 = (**(code **)(*piVar5 + 0x118))(piVar5,this->m_hTextFont);
      iVar8 = (**(code **)(*(int *)&(pCVar3->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                    super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                    super_C_BaseEntity.super_IClientEntity.super_IClientUnknown +
                          0x1e0))(pCVar3);
      if (iVar8 == 3) {
        uVar6 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52c743) + 0x390))
                          ((int *)**(undefined4 **)(unaff_EBX + 0x52c743),2);
        V_snwprintf(local_9c,0x20,(wchar_t *)(unaff_EBX + 0x3b846f),uVar6);
        local_9e = 0;
        local_9d = 0xff;
      }
      else {
        uVar6 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52c743) + 0x390))
                          ((int *)**(undefined4 **)(unaff_EBX + 0x52c743),3);
        V_snwprintf(local_9c,0x20,(wchar_t *)(unaff_EBX + 0x3b846f),uVar6);
        local_9e = 0xff;
        local_9d = 0;
      }
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52cb7f) + 0x50))
                ((int *)**(undefined4 **)(unaff_EBX + 0x52cb7f),this->m_hTextFont);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52cb7f) + 0x58))
                ((int *)**(undefined4 **)(unaff_EBX + 0x52cb7f),
                 (uint)local_9e << 0x10 | 0xff000000 | (uint)local_9d);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52cb7f) + 0x5c))
                ((int *)**(undefined4 **)(unaff_EBX + 0x52cb7f),10,iVar4 + 0x1e + iVar7);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52cb7f) + 0x200))
                ((int *)**(undefined4 **)(unaff_EBX + 0x52cb7f),local_9c,0);
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52cb7f) + 0x118))
                ((int *)**(undefined4 **)(unaff_EBX + 0x52cb7f),this->m_hTextFont);
      vgui::Panel::Paint((Panel *)this);
    }
  }
  return;
}


/* _GLOBAL__I__ZN24CHudMultiplayerBasicInfoC2EPKc at 00082170 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN24CHudMultiplayerBasicInfoC2EPKc(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

