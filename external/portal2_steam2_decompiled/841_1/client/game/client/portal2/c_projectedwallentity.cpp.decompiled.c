/* DWARF-guided pseudocode for game/client/portal2/c_projectedwallentity.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 000783c0 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  RecvTable *pRVar1;
  undefined4 uVar2;
  int *piVar3;
  int iVar4;
  IClassMap *pIVar5;
  int unaff_EBX;
  longlong lVar6;
  ConVar *in_stack_ffffffc4;
  ConVar *in_stack_ffffffc8;
  char *in_stack_ffffffcc;
  char *in_stack_ffffffd0;
  int in_stack_ffffffd4;
  char *in_stack_ffffffd8;
  
  lVar6 = ___i686_get_pc_thunk_bx();
  if (lVar6 == 0xffff00000001) {
    *(undefined1 *)
     ((int)&COptionsSubAudio::PanelMessageFunc_OpenThirdPartySoundCreditsDialog::InitVar()::bAdded +
     unaff_EBX + 3) = 0;
    (&DAT_00c5a14d)[unaff_EBX] = 0;
    (&DAT_00c5a14e)[unaff_EBX] = 0;
    (&DAT_00c5a14f)[unaff_EBX] = 0;
    *(undefined4 *)((int)&COptionsSubDifficulty::GetKBMap::s_pMap + unaff_EBX) = 0;
    *(undefined4 *)((int)&COptionsSubDifficulty::GetMessageMap::s_pMap + unaff_EBX) = 0;
    *(undefined4 *)(&COptionsSubDifficulty::ChainToMap()::chained + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)
     ((int)&COptionsSubDifficulty::PanelMessageFunc_OnRadioButtonChecked::InitVar()::bAdded +
     unaff_EBX + 1) = 0x7f7fffff;
    *(undefined4 *)((int)&COptionsSubGame::GetMessageMap()::s_pMap + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00c5a164 + unaff_EBX) = 0;
    *(undefined4 *)((int)&COptionsSubGame::GetKBMap()::s_pMap + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00c5a16c + unaff_EBX) = 0;
    *(undefined4 *)((int)&COptionsSubGame::GetKBMap::s_pMap + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&COptionsSubGame::GetMessageMap::s_pMap + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&COptionsSubGame::ChainToMap()::chained + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&COptionsSubGame::KB_ChainToMap()::chained + unaff_EBX + 2) = 0x7f7fffff;
    *(undefined **)((int)&COptionsSubKeyboardAdvancedDlg::GetMessageMap()::s_pMap + unaff_EBX) =
         &UNK_00aecab4 + unaff_EBX;
    ConVar::ConVar((ConVar *)((int)&vec2_invalid.x + unaff_EBX),&UNK_00942cd0 + unaff_EBX,
                   &UNK_00942ccc + unaff_EBX,0,in_stack_ffffffc4,(char *)in_stack_ffffffc8,
                   in_stack_ffffffcc,(int)in_stack_ffffffd0);
    uVar2 = *(undefined4 *)(&DAT_00aeb998 + unaff_EBX);
    ___cxa_atexit(unaff_EBX + 0x903c8c,0,uVar2);
    ConVar::ConVar((ConVar *)((int)&r_showenvcubemap.super_ConCommandBase.m_pszName + unaff_EBX),
                   &UNK_00942cfc + unaff_EBX,&UNK_00942cf3 + unaff_EBX,0,in_stack_ffffffc4,
                   (char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
    ___cxa_atexit(unaff_EBX + 0x903c6c,0,uVar2);
    ConVar::ConVar((ConVar *)((int)&r_eyegloss.super_ConCommandBase.m_pszName + unaff_EBX),
                   &UNK_00942d2c + unaff_EBX,(char *)(unaff_EBX + 0x913758),0,in_stack_ffffffc4,
                   (char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
    ___cxa_atexit(unaff_EBX + 0x903c4c,0,uVar2);
    ConVar::ConVar((ConVar *)((int)&r_eyemove.super_ConCommandBase.m_pszName + unaff_EBX),
                   &UNK_00942d50 + unaff_EBX,&UNK_0090aae8 + unaff_EBX,0,in_stack_ffffffc4,
                   (char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
    ___cxa_atexit(unaff_EBX + 0x903c2c,0,uVar2);
    *(undefined4 *)(&r_eyeshift_x.super_ConCommandBase.m_bRegistered + unaff_EBX) = 0;
    *(undefined **)((int)&r_eyeshift_x.super_ConCommandBase.m_pszName + unaff_EBX) =
         &UNK_00942d70 + unaff_EBX;
    piVar3 = *(int **)(&DAT_00aebb88 + unaff_EBX);
    *(int *)((int)&r_eyeshift_x.super_ConCommandBase.m_pszHelpString + unaff_EBX) = *piVar3;
    *piVar3 = unaff_EBX + 0xcf0fe4;
    *(undefined **)((int)&r_eyeshift_x.super_ConCommandBase.m_pNext + unaff_EBX) =
         &UNK_00b549fc + unaff_EBX;
    *(undefined4 *)((int)&r_eyeshift_x.super_ConCommandBase.m_nFlags + unaff_EBX) = 0;
    *(undefined4 *)((int)&r_eyeshift_x.super_IConVar._vptr_IConVar + unaff_EBX) = 0;
    *(undefined4 *)((int)&r_eyeshift_x.m_pParent + unaff_EBX) = 0;
    *(undefined4 *)((int)&r_eyeshift_x.m_pszDefaultValue + unaff_EBX) = 0;
    *(undefined4 *)((int)&r_eyeshift_x.m_Value.m_pszString + unaff_EBX) = 0;
    ___cxa_atexit(unaff_EBX + 0x903cac,0,uVar2);
    *(undefined **)((int)&r_eyeshift_x.m_Value.m_nValue + unaff_EBX) = &UNK_00942d92 + unaff_EBX;
    *(int *)((int)&r_eyeshift_x.m_Value.m_StringLength + unaff_EBX) = unaff_EBX + 0x4ba32c;
    *(undefined4 *)((int)&r_eyeshift_x.m_Value.m_fValue + unaff_EBX) = 0;
    pRVar1 = (RecvTable *)((int)&r_eyeshift_x.m_fMaxVal + unaff_EBX);
    *(RecvTable **)(&r_eyeshift_x.m_bHasMin + unaff_EBX) = pRVar1;
    piVar3 = *(int **)(&DAT_00aeba10 + unaff_EBX);
    *(int *)((int)&r_eyeshift_x.m_fMinVal + unaff_EBX) = *piVar3;
    *piVar3 = unaff_EBX + 0xcf1008;
    RecvTable::RecvTable(pRVar1);
    ___cxa_atexit(unaff_EBX + 0x903bec,0,uVar2);
    iVar4 = ClientClassInit<DT_ProjectedWallEntity::ignored>((ignored *)0x0);
    *(int *)((int)&r_eyeshift_x.m_fnChangeCallbacks.m_pElements + unaff_EBX) = iVar4;
    *(undefined4 *)((int)&PTR_GetBeamTraceFilter_00bb44d4 + unaff_EBX) = 5;
    *(int *)((int)&PTR_HandleShotImpactingWater_00bb44d0 + unaff_EBX) =
         (int)&PTR_GetBaseAnimating_00bb452c + unaff_EBX;
    *(int *)(&DAT_00cf1038 + unaff_EBX) = (int)&PTR_HandleShotImpactingWater_00bb44d0 + unaff_EBX;
    pIVar5 = GetClassMap();
    (*pIVar5->_vptr_IClassMap[2])
              (pIVar5,unaff_EBX + 0x9171c4,&UNK_00942cb5 + unaff_EBX,0xb5c,unaff_EBX + 0x4ba2ec);
    ConVar::ConVar((ConVar *)((int)&r_eyeshift_y.super_ConCommandBase.m_pszName + unaff_EBX),
                   &UNK_00942e1a + unaff_EBX,&UNK_0090aae8 + unaff_EBX,0,&UNK_00942da8 + unaff_EBX,
                   in_stack_ffffffc8,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,
                   in_stack_ffffffd8);
    ___cxa_atexit(unaff_EBX + 0x903c0c,0,uVar2);
    CIFM_EntityKeyValuesHandler_RecreateEntities::CIFM_EntityKeyValuesHandler_RecreateEntities
              ((CIFM_EntityKeyValuesHandler_RecreateEntities *)(&DAT_00c5a18c + unaff_EBX),
               &UNK_00942cb5 + unaff_EBX);
    *(undefined **)(&DAT_00c5a18c + unaff_EBX) = &UNK_00b54fd4 + unaff_EBX;
    ___cxa_atexit(unaff_EBX + 0x903d8c,0,uVar2);
    C_BaseTempEntity::C_BaseTempEntity
              ((C_BaseTempEntity *)((int)&COptionsSubMouse::GetMessageMap::s_pMap + unaff_EBX));
    *(undefined **)((int)&COptionsSubMouse::GetMessageMap::s_pMap + unaff_EBX) =
         &UNK_00b543f4 + unaff_EBX;
    *(undefined **)(&COptionsSubMouse::ChainToMap()::chained + unaff_EBX) =
         &UNK_00b54478 + unaff_EBX;
    *(undefined4 *)((int)&vgui::ImagePanel::GetMessageMap::s_pMap + unaff_EBX) = 0xffffffff;
    ___cxa_atexit(unaff_EBX + 0x903e6c,0,uVar2);
    *(undefined **)((int)&r_eyeshift_z.super_ConCommandBase.m_pszName + unaff_EBX) =
         &UNK_00942e36 + unaff_EBX;
    *(undefined4 *)((int)&r_eyeshift_z.super_ConCommandBase.m_pNext + unaff_EBX) = 0;
    *(undefined **)(&r_eyeshift_z.super_ConCommandBase.m_bRegistered + unaff_EBX) =
         &UNK_004b6f3c + unaff_EBX;
    pRVar1 = (RecvTable *)((int)&r_eyeshift_z.m_pParent + unaff_EBX);
    *(RecvTable **)((int)&r_eyeshift_z.super_ConCommandBase.m_pszHelpString + unaff_EBX) = pRVar1;
    piVar3 = *(int **)(&DAT_00aeba10 + unaff_EBX);
    *(int *)((int)&r_eyeshift_z.super_ConCommandBase.m_nFlags + unaff_EBX) = *piVar3;
    *piVar3 = unaff_EBX + 0xcf10a4;
    RecvTable::RecvTable(pRVar1);
    ___cxa_atexit(unaff_EBX + 0x903bcc,0,uVar2);
    iVar4 = ClientClassInit<DT_TEWallPaintedEvent::ignored>((ignored *)0x0);
    *(int *)((int)&r_eyeshift_z.m_Value.m_nValue + unaff_EBX) = iVar4;
  }
  return;
}


/* __tcf_0 at 0097c060 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(&UNK_003ed1d8 + extraout_ECX),in_stack_00000008);
  return;
}


/* __tcf_1 at 0097c040 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ed258),in_stack_00000008);
  return;
}


/* __tcf_2 at 0097c020 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ed2d8),in_stack_00000008);
  return;
}


/* __tcf_3 at 0097c000 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ed358),in_stack_00000008);
  return;
}


/* PrecacheEffectProjectedWallEntityPrecache::CResourcePrecacher::Cache at 0052ed50 */

/* WARNING: Enum "PrecacheSystem_t": Some values do not have unique names */
/* DWARF original prototype: void Cache(CResourcePrecacher * this, IPrecacheHandler *
   pPrecacheHandler, bool bPrecache, ResourceList_t hResourceList, bool bIgnoreConditionals) */

void __thiscall
PrecacheEffectProjectedWallEntityPrecache::CResourcePrecacher::Cache
          (CResourcePrecacher *this,IPrecacheHandler *pPrecacheHandler,bool bPrecache,
          ResourceList_t hResourceList,bool bIgnoreConditionals)

{
  uint uVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  uVar1 = (uint)bPrecache;
  (**pPrecacheHandler->_vptr_IPrecacheHandler)
            (pPrecacheHandler,1,unaff_EBX + 0x48c0c7,uVar1,hResourceList,0);
  (**pPrecacheHandler->_vptr_IPrecacheHandler)
            (pPrecacheHandler,1,unaff_EBX + 0x48c162,uVar1,hResourceList,0);
  (**pPrecacheHandler->_vptr_IPrecacheHandler)
            (pPrecacheHandler,1,unaff_EBX + 0x48c17e,uVar1,hResourceList,0);
  (**pPrecacheHandler->_vptr_IPrecacheHandler)
            (pPrecacheHandler,1,unaff_EBX + 0x48c19b,uVar1,hResourceList,0);
  (**pPrecacheHandler->_vptr_IPrecacheHandler)
            (pPrecacheHandler,1,unaff_EBX + 0x48c1b7,uVar1,hResourceList,0);
  (**pPrecacheHandler->_vptr_IPrecacheHandler)
            (pPrecacheHandler,1,unaff_EBX + 0x48c1cf,uVar1,hResourceList,0);
  return;
}


/* IProjectedWallEntityAutoList::~IProjectedWallEntityAutoList at 00530160 */

/* DWARF original prototype: void ~IProjectedWallEntityAutoList(IProjectedWallEntityAutoList * this,
   int __in_chrg) */

void __thiscall
IProjectedWallEntityAutoList::~IProjectedWallEntityAutoList
          (IProjectedWallEntityAutoList *this,int __in_chrg)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  this->_vptr_IProjectedWallEntityAutoList = (_func_int_varargs **)(unaff_EBX + 0x69cc4a);
  iVar1 = *(int *)(unaff_EBX + 0x839266);
  if (iVar1 < 1) {
    return;
  }
  piVar2 = *(int **)(unaff_EBX + 0x83925a);
  if (this == (IProjectedWallEntityAutoList *)*piVar2) {
    iVar4 = 0;
    iVar5 = 0;
  }
  else {
    iVar4 = 0;
    iVar3 = 4;
    do {
      iVar5 = iVar3;
      iVar4 = iVar4 + 1;
      if (iVar4 == iVar1) {
        return;
      }
      iVar3 = iVar5 + 4;
    } while (this != (IProjectedWallEntityAutoList *)piVar2[iVar4]);
  }
  if (iVar4 != iVar1 + -1) {
    *(int *)(iVar5 + (int)piVar2) = piVar2[iVar1 + -1];
    iVar4 = *(int *)(unaff_EBX + 0x839266) + -1;
  }
  *(int *)(unaff_EBX + 0x839266) = iVar4;
  return;
}


/* IProjectedWallEntityAutoList::IProjectedWallEntityAutoList at 00530ad0 */

/* DWARF original prototype: void IProjectedWallEntityAutoList(IProjectedWallEntityAutoList * this,
   bool bAutoAdd) */

void __thiscall
IProjectedWallEntityAutoList::IProjectedWallEntityAutoList
          (IProjectedWallEntityAutoList *this,bool bAutoAdd)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  int unaff_EBX;
  int iVar5;
  int iVar6;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  this->_vptr_IProjectedWallEntityAutoList = (_func_int_varargs **)(unaff_EBX + 0x69c2d4);
  if (!bAutoAdd) {
    return;
  }
  iVar5 = *(int *)(unaff_EBX + 0x8388f0);
  iVar6 = iVar5 + 1;
  iVar3 = *(int *)(unaff_EBX + 0x8388e8);
  iVar2 = iVar6;
  if (iVar6 <= iVar3) goto LAB_00530b10;
  iVar1 = *(int *)(unaff_EBX + 0x8388ec);
  if (iVar1 < 0) goto LAB_00530b10;
  if (iVar1 == 0) {
    if ((iVar3 == 0) && (iVar3 = 8, iVar6 < 9)) {
      local_20 = 0x20;
    }
    else {
      do {
        local_20 = iVar3;
        iVar3 = local_20 * 2;
      } while (iVar3 < iVar6);
      local_20 = local_20 * 8;
    }
    goto LAB_00530baa;
  }
  iVar3 = (iVar5 / iVar1 + 1) * iVar1;
  if (iVar3 < iVar6) {
    if (iVar3 != 0) {
LAB_00530b90:
      do {
        iVar3 = (iVar3 + iVar6) / 2;
      } while (iVar3 < iVar6);
      goto LAB_00530ba0;
    }
    if (iVar6 < 0) {
      iVar3 = -1;
      local_20 = -4;
    }
    else {
      local_20 = 0;
      if (iVar6 != 0) goto LAB_00530b90;
    }
  }
  else {
LAB_00530ba0:
    local_20 = iVar3 * 4;
  }
LAB_00530baa:
  *(int *)(unaff_EBX + 0x8388e8) = iVar3;
  if (*(int *)(unaff_EBX + 0x8388e4) == 0) {
    uVar4 = (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x63328c))
                      ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x63328c),local_20);
    *(undefined4 *)(unaff_EBX + 0x8388e4) = uVar4;
    iVar2 = *(int *)(unaff_EBX + 0x8388f0) + 1;
  }
  else {
    uVar4 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x63328c) + 4))
                      ((int *)**(undefined4 **)(unaff_EBX + 0x63328c),*(int *)(unaff_EBX + 0x8388e4)
                       ,local_20);
    *(undefined4 *)(unaff_EBX + 0x8388e4) = uVar4;
    iVar2 = *(int *)(unaff_EBX + 0x8388f0) + 1;
  }
LAB_00530b10:
  *(int *)(unaff_EBX + 0x8388f0) = iVar2;
  iVar3 = *(int *)(unaff_EBX + 0x8388e4);
  *(int *)(unaff_EBX + 0x8388f4) = iVar3;
  iVar2 = (iVar2 - iVar5) + -1;
  if (iVar2 < 1) {
    iVar5 = iVar5 << 2;
  }
  else {
    iVar5 = iVar5 * 4;
    _V_memmove((void *)(iVar3 + iVar6 * 4),(void *)(iVar3 + iVar5),iVar2 * 4);
    iVar3 = *(int *)(unaff_EBX + 0x8388e4);
  }
  if ((undefined4 *)(iVar5 + iVar3) != (undefined4 *)0x0) {
    *(undefined4 *)(iVar5 + iVar3) = this;
  }
  return;
}


/* IProjectedWallEntityAutoList::~IProjectedWallEntityAutoList at 00530c90 */

/* DWARF original prototype: void ~IProjectedWallEntityAutoList(IProjectedWallEntityAutoList * this,
   int __in_chrg) */

void __thiscall
IProjectedWallEntityAutoList::~IProjectedWallEntityAutoList
          (IProjectedWallEntityAutoList *this,int __in_chrg)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  this->_vptr_IProjectedWallEntityAutoList = (_func_int_varargs **)(unaff_EBX + 0x69c11a);
  iVar1 = *(int *)(unaff_EBX + 0x838736);
  if (iVar1 < 1) {
    return;
  }
  piVar2 = *(int **)(unaff_EBX + 0x83872a);
  if (this == (IProjectedWallEntityAutoList *)*piVar2) {
    iVar4 = 0;
    iVar5 = 0;
  }
  else {
    iVar4 = 0;
    iVar3 = 4;
    do {
      iVar5 = iVar3;
      iVar4 = iVar4 + 1;
      if (iVar4 == iVar1) {
        return;
      }
      iVar3 = iVar5 + 4;
    } while (this != (IProjectedWallEntityAutoList *)piVar2[iVar4]);
  }
  if (iVar4 != iVar1 + -1) {
    *(int *)(iVar5 + (int)piVar2) = piVar2[iVar1 + -1];
    iVar4 = *(int *)(unaff_EBX + 0x838736) + -1;
  }
  *(int *)(unaff_EBX + 0x838736) = iVar4;
  return;
}


/* IProjectedWallEntityAutoList::~IProjectedWallEntityAutoList at 00530d30 */

/* DWARF original prototype: void ~IProjectedWallEntityAutoList(IProjectedWallEntityAutoList * this,
   int __in_chrg) */

void __thiscall
IProjectedWallEntityAutoList::~IProjectedWallEntityAutoList
          (IProjectedWallEntityAutoList *this,int __in_chrg)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  this->_vptr_IProjectedWallEntityAutoList = (_func_int_varargs **)(unaff_EBX + 0x69c07a);
  iVar1 = *(int *)(unaff_EBX + 0x838696);
  if (0 < iVar1) {
    piVar2 = *(int **)(unaff_EBX + 0x83868a);
    if (this == (IProjectedWallEntityAutoList *)*piVar2) {
      iVar4 = 0;
      iVar5 = 0;
    }
    else {
      iVar4 = 0;
      iVar3 = 4;
      do {
        iVar5 = iVar3;
        iVar4 = iVar4 + 1;
        if (iVar4 == iVar1) goto LAB_00530d96;
        iVar3 = iVar5 + 4;
      } while (this != (IProjectedWallEntityAutoList *)piVar2[iVar4]);
    }
    if (iVar4 != iVar1 + -1) {
      *(int *)(iVar5 + (int)piVar2) = piVar2[iVar1 + -1];
      iVar4 = *(int *)(unaff_EBX + 0x838696) + -1;
    }
    *(int *)(unaff_EBX + 0x838696) = iVar4;
  }
LAB_00530d96:
  operator_delete(this);
  return;
}


/* IProjectedWallEntityAutoList::IProjectedWallEntityAutoList at 00532300 */

/* DWARF original prototype: void IProjectedWallEntityAutoList(IProjectedWallEntityAutoList * this,
   bool bAutoAdd) */

void __thiscall
IProjectedWallEntityAutoList::IProjectedWallEntityAutoList
          (IProjectedWallEntityAutoList *this,bool bAutoAdd)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  int unaff_EBX;
  int iVar5;
  int iVar6;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  this->_vptr_IProjectedWallEntityAutoList = (_func_int_varargs **)(unaff_EBX + 0x69aaa4);
  if (!bAutoAdd) {
    return;
  }
  iVar5 = *(int *)(unaff_EBX + 0x8370c0);
  iVar6 = iVar5 + 1;
  iVar3 = *(int *)(unaff_EBX + 0x8370b8);
  iVar2 = iVar6;
  if (iVar6 <= iVar3) goto LAB_00532340;
  iVar1 = *(int *)(unaff_EBX + 0x8370bc);
  if (iVar1 < 0) goto LAB_00532340;
  if (iVar1 == 0) {
    if ((iVar3 == 0) && (iVar3 = 8, iVar6 < 9)) {
      local_20 = 0x20;
    }
    else {
      do {
        local_20 = iVar3;
        iVar3 = local_20 * 2;
      } while (iVar3 < iVar6);
      local_20 = local_20 * 8;
    }
    goto LAB_005323da;
  }
  iVar3 = (iVar5 / iVar1 + 1) * iVar1;
  if (iVar3 < iVar6) {
    if (iVar3 != 0) {
LAB_005323c0:
      do {
        iVar3 = (iVar3 + iVar6) / 2;
      } while (iVar3 < iVar6);
      goto LAB_005323d0;
    }
    if (iVar6 < 0) {
      iVar3 = -1;
      local_20 = -4;
    }
    else {
      local_20 = 0;
      if (iVar6 != 0) goto LAB_005323c0;
    }
  }
  else {
LAB_005323d0:
    local_20 = iVar3 * 4;
  }
LAB_005323da:
  *(int *)(unaff_EBX + 0x8370b8) = iVar3;
  if (*(int *)(unaff_EBX + 0x8370b4) == 0) {
    uVar4 = (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x631a5c))
                      ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x631a5c),local_20);
    *(undefined4 *)(unaff_EBX + 0x8370b4) = uVar4;
    iVar2 = *(int *)(unaff_EBX + 0x8370c0) + 1;
  }
  else {
    uVar4 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x631a5c) + 4))
                      ((int *)**(undefined4 **)(unaff_EBX + 0x631a5c),*(int *)(unaff_EBX + 0x8370b4)
                       ,local_20);
    *(undefined4 *)(unaff_EBX + 0x8370b4) = uVar4;
    iVar2 = *(int *)(unaff_EBX + 0x8370c0) + 1;
  }
LAB_00532340:
  *(int *)(unaff_EBX + 0x8370c0) = iVar2;
  iVar3 = *(int *)(unaff_EBX + 0x8370b4);
  *(int *)(unaff_EBX + 0x8370c4) = iVar3;
  iVar2 = (iVar2 - iVar5) + -1;
  if (iVar2 < 1) {
    iVar5 = iVar5 << 2;
  }
  else {
    iVar5 = iVar5 * 4;
    _V_memmove((void *)(iVar3 + iVar6 * 4),(void *)(iVar3 + iVar5),iVar2 * 4);
    iVar3 = *(int *)(unaff_EBX + 0x8370b4);
  }
  if ((undefined4 *)(iVar5 + iVar3) != (undefined4 *)0x0) {
    *(undefined4 *)(iVar5 + iVar3) = this;
  }
  return;
}


/* C_ProjectedWallEntity::~C_ProjectedWallEntity at 00537b00 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void ~C_ProjectedWallEntity(C_ProjectedWallEntity * this, int
   __in_chrg) */

void __thiscall
C_ProjectedWallEntity::~C_ProjectedWallEntity(C_ProjectedWallEntity *this,int __in_chrg)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  WallCollideableAtTime_t *pWVar5;
  int iVar6;
  int unaff_EBX;
  WallCollideableAtTime_t *in_stack_ffffff98;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(&UNK_00694d7a + unaff_EBX);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x69515e);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x695216);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(CBaseWorldView::~CBaseWorldView + unaff_EBX + 2);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x69526e);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_IPaintableEntity.
  _vptr_IPaintableEntity = (_func_int_varargs **)(unaff_EBX + 0x69527a);
  (this->super_IProjectedWallEntityAutoList)._vptr_IProjectedWallEntityAutoList =
       (_func_int_varargs **)(unaff_EBX + 0x695296);
  (this->m_PaintPowers).m_Size = 0;
  if ((this->m_PaintPowers).m_Memory.m_nGrowSize < 0) {
    pWVar5 = (WallCollideableAtTime_t *)(this->m_PaintPowers).m_Memory.m_pMemory;
  }
  else {
    pWVar5 = (WallCollideableAtTime_t *)(this->m_PaintPowers).m_Memory.m_pMemory;
    if (pWVar5 != (WallCollideableAtTime_t *)0x0) {
      in_stack_ffffff98 = pWVar5;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x62c262) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x62c262));
      pWVar5 = (WallCollideableAtTime_t *)0x0;
      (this->m_PaintPowers).m_Memory.m_pMemory = (PaintPowerType *)0x0;
    }
    (this->m_PaintPowers).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintPowers).m_pElements = (PaintPowerType *)pWVar5;
  if (-1 < (this->m_PaintPowers).m_Memory.m_nGrowSize) {
    if (pWVar5 != (WallCollideableAtTime_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x62c262) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x62c262));
      (this->m_PaintPowers).m_Memory.m_pMemory = (PaintPowerType *)0x0;
      in_stack_ffffff98 = pWVar5;
    }
    (this->m_PaintPowers).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_WallCollideables).m_Size = 0;
  if ((this->m_WallCollideables).m_Memory.m_nGrowSize < 0) {
    pWVar5 = (this->m_WallCollideables).m_Memory.m_pMemory;
  }
  else {
    pWVar5 = (this->m_WallCollideables).m_Memory.m_pMemory;
    if (pWVar5 != (WallCollideableAtTime_t *)0x0) {
      in_stack_ffffff98 = pWVar5;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x62c262) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x62c262));
      pWVar5 = (WallCollideableAtTime_t *)0x0;
      (this->m_WallCollideables).m_Memory.m_pMemory = (WallCollideableAtTime_t *)0x0;
    }
    (this->m_WallCollideables).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_WallCollideables).m_pElements = pWVar5;
  if (-1 < (this->m_WallCollideables).m_Memory.m_nGrowSize) {
    if (pWVar5 != (WallCollideableAtTime_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x62c262) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x62c262));
      (this->m_WallCollideables).m_Memory.m_pMemory = (WallCollideableAtTime_t *)0x0;
      in_stack_ffffff98 = pWVar5;
    }
    (this->m_WallCollideables).m_Memory.m_nAllocationCount = 0;
  }
  (this->super_IProjectedWallEntityAutoList)._vptr_IProjectedWallEntityAutoList =
       (_func_int_varargs **)(unaff_EBX + 0x6952aa);
  iVar1 = *(int *)(&DAT_008318c6 + unaff_EBX);
  if (0 < iVar1) {
    piVar2 = *(int **)(&DAT_008318ba + unaff_EBX);
    if (&this->super_IProjectedWallEntityAutoList == (IProjectedWallEntityAutoList *)*piVar2) {
      iVar4 = 0;
      iVar6 = 0;
    }
    else {
      iVar4 = 0;
      iVar3 = 4;
      do {
        iVar6 = iVar3;
        iVar4 = iVar4 + 1;
        if (iVar4 == iVar1) goto LAB_00537d0a;
        iVar3 = iVar6 + 4;
      } while (&this->super_IProjectedWallEntityAutoList !=
               (IProjectedWallEntityAutoList *)piVar2[iVar4]);
    }
    if (iVar4 != iVar1 + -1) {
      *(int *)((int)piVar2 + iVar6) = piVar2[iVar1 + -1];
      iVar4 = *(int *)(&DAT_008318c6 + unaff_EBX) + -1;
    }
    *(int *)(&DAT_008318c6 + unaff_EBX) = iVar4;
  }
LAB_00537d0a:
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x69537a);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(&UNK_00695756 + unaff_EBX);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x69580e);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x69584a);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x695866);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_IPaintableEntity.
  _vptr_IPaintableEntity = (_func_int_varargs **)(unaff_EBX + 0x695872);
  C_BaseProjectedEntity::~C_BaseProjectedEntity
            ((C_BaseProjectedEntity *)this,(int)in_stack_ffffff98);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_ProjectedWallEntity::~C_ProjectedWallEntity at 00538040 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* DWARF original prototype: void ~C_ProjectedWallEntity(C_ProjectedWallEntity * this, int
   __in_chrg, C_ProjectedWallEntity * this) */

void __thiscall
C_ProjectedWallEntity::~C_ProjectedWallEntity
          (C_ProjectedWallEntity *this,int __in_chrg,C_ProjectedWallEntity *this_1)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  PaintPowerType *pPVar5;
  WallCollideableAtTime_t *pWVar6;
  int iVar7;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x69483a);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(&UNK_00694c1e + unaff_EBX);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x694cd6);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(&UNK_00694d12 + unaff_EBX);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(&UNK_00694d2e + unaff_EBX);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_IPaintableEntity.
  _vptr_IPaintableEntity = (_func_int_varargs **)(&UNK_00694d3a + unaff_EBX);
  (this->super_IProjectedWallEntityAutoList)._vptr_IProjectedWallEntityAutoList =
       (_func_int_varargs **)(&UNK_00694d56 + unaff_EBX);
  (this->m_PaintPowers).m_Size = 0;
  if ((this->m_PaintPowers).m_Memory.m_nGrowSize < 0) {
    pPVar5 = (this->m_PaintPowers).m_Memory.m_pMemory;
  }
  else {
    pPVar5 = (this->m_PaintPowers).m_Memory.m_pMemory;
    if (pPVar5 != (PaintPowerType *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)
                             (CVGUI_MP_LobbyScreen::CVGUI_MP_LobbyScreen + unaff_EBX + 2) + 8))
                ((int *)**(undefined4 **)
                          (CVGUI_MP_LobbyScreen::CVGUI_MP_LobbyScreen + unaff_EBX + 2),pPVar5);
      (this->m_PaintPowers).m_Memory.m_pMemory = (PaintPowerType *)0x0;
    }
    pPVar5 = (PaintPowerType *)0x0;
    (this->m_PaintPowers).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintPowers).m_pElements = pPVar5;
  if (-1 < (this->m_PaintPowers).m_Memory.m_nGrowSize) {
    if (pPVar5 != (PaintPowerType *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)
                             (CVGUI_MP_LobbyScreen::CVGUI_MP_LobbyScreen + unaff_EBX + 2) + 8))
                ((int *)**(undefined4 **)
                          (CVGUI_MP_LobbyScreen::CVGUI_MP_LobbyScreen + unaff_EBX + 2),pPVar5);
      (this->m_PaintPowers).m_Memory.m_pMemory = (PaintPowerType *)0x0;
    }
    (this->m_PaintPowers).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_WallCollideables).m_Size = 0;
  if ((this->m_WallCollideables).m_Memory.m_nGrowSize < 0) {
    pWVar6 = (this->m_WallCollideables).m_Memory.m_pMemory;
  }
  else {
    pWVar6 = (this->m_WallCollideables).m_Memory.m_pMemory;
    if (pWVar6 != (WallCollideableAtTime_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)
                             (CVGUI_MP_LobbyScreen::CVGUI_MP_LobbyScreen + unaff_EBX + 2) + 8))
                ((int *)**(undefined4 **)
                          (CVGUI_MP_LobbyScreen::CVGUI_MP_LobbyScreen + unaff_EBX + 2),pWVar6);
      (this->m_WallCollideables).m_Memory.m_pMemory = (WallCollideableAtTime_t *)0x0;
    }
    pWVar6 = (WallCollideableAtTime_t *)0x0;
    (this->m_WallCollideables).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_WallCollideables).m_pElements = pWVar6;
  if (-1 < (this->m_WallCollideables).m_Memory.m_nGrowSize) {
    if (pWVar6 != (WallCollideableAtTime_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)
                             (CVGUI_MP_LobbyScreen::CVGUI_MP_LobbyScreen + unaff_EBX + 2) + 8))
                ((int *)**(undefined4 **)
                          (CVGUI_MP_LobbyScreen::CVGUI_MP_LobbyScreen + unaff_EBX + 2),pWVar6);
      (this->m_WallCollideables).m_Memory.m_pMemory = (WallCollideableAtTime_t *)0x0;
    }
    (this->m_WallCollideables).m_Memory.m_nAllocationCount = 0;
  }
  (this->super_IProjectedWallEntityAutoList)._vptr_IProjectedWallEntityAutoList =
       (_func_int_varargs **)(&UNK_00694d6a + unaff_EBX);
  iVar1 = *(int *)(unaff_EBX + 0x831386);
  if (0 < iVar1) {
    piVar2 = *(int **)(unaff_EBX + 0x83137a);
    if (&this->super_IProjectedWallEntityAutoList == (IProjectedWallEntityAutoList *)*piVar2) {
      iVar4 = 0;
      iVar7 = 0;
    }
    else {
      iVar4 = 0;
      iVar3 = 4;
      do {
        iVar7 = iVar3;
        iVar4 = iVar4 + 1;
        if (iVar4 == iVar1) goto LAB_0053824a;
        iVar3 = iVar7 + 4;
      } while (&this->super_IProjectedWallEntityAutoList !=
               (IProjectedWallEntityAutoList *)piVar2[iVar4]);
    }
    if (iVar4 != iVar1 + -1) {
      *(int *)((int)piVar2 + iVar7) = piVar2[iVar1 + -1];
      iVar4 = *(int *)(unaff_EBX + 0x831386) + -1;
    }
    *(int *)(unaff_EBX + 0x831386) = iVar4;
  }
LAB_0053824a:
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x694e3a);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x695216);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6952ce);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(&UNK_0069530a + unaff_EBX);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x695326);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_IPaintableEntity.
  _vptr_IPaintableEntity = (_func_int_varargs **)(unaff_EBX + 0x695332);
  C_BaseProjectedEntity::~C_BaseProjectedEntity((C_BaseProjectedEntity *)this,__in_chrg);
  return;
}


/* __tcf_4 at 0097c080 */

void __tcf_4(void *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  *(undefined4 *)(unaff_EBX + 0x3ed340) = 0;
  if (*(int *)(unaff_EBX + 0x3ed33c) < 0) {
    iVar1 = *(int *)(unaff_EBX + 0x3ed334);
  }
  else {
    if (*(int *)(unaff_EBX + 0x3ed334) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1e7cdc) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1e7cdc),*(int *)(unaff_EBX + 0x3ed334));
      *(undefined4 *)(unaff_EBX + 0x3ed334) = 0;
    }
    iVar1 = 0;
    *(undefined4 *)(unaff_EBX + 0x3ed338) = 0;
  }
  *(int *)(unaff_EBX + 0x3ed344) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x3ed33c)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1e7cdc) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1e7cdc),iVar1);
      *(undefined4 *)(unaff_EBX + 0x3ed334) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x3ed338) = 0;
  }
  return;
}


/* ClientClassInit<DT_ProjectedWallEntity::ignored> at 00078020 */

int ClientClassInit<DT_ProjectedWallEntity::ignored>(ignored *param_1)

{
  RecvVarProxyFn p_Var1;
  int iVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&BaseModUI::LeaderboardListItem::GetKBMap()::s_pMap + unaff_EBX + 2) == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xc5a682);
    if (iVar2 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&BaseModUI::Leaderboard::GetKBMap()::s_pMap + unaff_EBX + 2),
                  (char *)(unaff_EBX + 0x912742),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)(unaff_EBX + 0xc5a6ce),(char *)(unaff_EBX + 0x917286),
                        0,0,(RecvTable *)**(undefined4 **)(&DAT_00aec5a2 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00aebdbe + unaff_EBX));
      p_Var1 = *(RecvVarProxyFn *)(&DAT_00aebe62 + unaff_EBX);
      RecvPropVector((RecvProp_conflict1 *)(unaff_EBX + 0xc5a70a),(char *)(unaff_EBX + 0x931f2a),
                     0x118,0xc,0,p_Var1);
      RecvPropVector((RecvProp_conflict1 *)
                     ((int)&BaseModUI::Achievements::GetMessageMap::s_pMap + unaff_EBX + 2),
                     (char *)(unaff_EBX + 0x92e57a),0x10c,0xc,0,p_Var1);
      RecvPropVector((RecvProp_conflict1 *)
                     ((int)&BaseModUI::AddonAssociation::KB_ChainToMap()::chained + unaff_EBX),
                     (char *)(unaff_EBX + 0x942fb3),0xaf0,0xc,0,p_Var1);
      RecvPropVector((RecvProp_conflict1 *)
                     ((int)&BaseModUI::CAttractScreen::GetMessageMap::s_pMap + unaff_EBX + 2),
                     (char *)(unaff_EBX + 0x942fca),0xafc,0xc,0,p_Var1);
      RecvPropEHandle((RecvProp *)((int)&BaseModUI::Audio::KB_ChainToMap()::chained + unaff_EBX),
                      (char *)(unaff_EBX + 0x942fe1),0xb2c,4,
                      *(RecvVarProxyFn *)(&DAT_00aebdba + unaff_EBX));
      p_Var1 = *(RecvVarProxyFn *)(&DAT_00aebdfe + unaff_EBX);
      RecvPropFloat((RecvProp_conflict1 *)(unaff_EBX + 0xc5a836),(char *)(unaff_EBX + 0x942ff0),
                    0xb1c,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&BaseModUI::ControllerOptions::GetKBMap::s_pMap + unaff_EBX + 2),
                    (char *)(unaff_EBX + 0x933ad6),0xb20,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&BaseModUI::CustomCampaigns::GetMessageMap::s_pMap + unaff_EBX + 2),
                    (char *)(unaff_EBX + 0x933ae0),0xb24,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&BaseModUI::DropDownMenu::KB_ChainToMap()::chained + unaff_EBX),
                    (char *)(unaff_EBX + 0x942ffc),0xb3c,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&BaseModUI::FoundGames::GetMessageMap::s_pMap + unaff_EBX + 2),
                    (char *)(unaff_EBX + 0x94300e),0xb54,4,0,p_Var1);
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&BaseModUI::CPlayerItem::GetKBMap()::s_pMap + unaff_EBX + 2),
                  (char *)(unaff_EBX + 0x943025),0xb38,4,0,(RecvVarProxyFn)0x0);
      RecvPropBool((RecvProp *)(&BaseModUI::CPlayersList::KB_ChainToMap()::chained + unaff_EBX),
                   (char *)(unaff_EBX + 0x943034),0xb28,1);
      ___cxa_guard_release(unaff_EBX + 0xc5a682);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)&r_eyes.m_Value.m_pszString + unaff_EBX + 2),
             (RecvProp_conflict1 *)(unaff_EBX + 0xc5a6ce),0xd,(char *)(unaff_EBX + 0x943044));
  return 1;
}


/* C_ProjectedWallEntity::YouForgotToImplementOrDeclareClientClass at 0052ee80 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_ProjectedWallEntity *
   this) */

int __thiscall
C_ProjectedWallEntity::YouForgotToImplementOrDeclareClientClass(C_ProjectedWallEntity *this)

{
  return 0;
}


/* C_ProjectedWallEntity::GetClientClass at 0052eea0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: ClientClass * GetClientClass(C_ProjectedWallEntity * this) */

ClientClass * __thiscall C_ProjectedWallEntity::GetClientClass(C_ProjectedWallEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x83a534);
}


/* _C_ProjectedWallEntity_CreateObject at 00532700 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

IClientNetworkable * _C_ProjectedWallEntity_CreateObject(int entnum,int serialNum)

{
  C_ProjectedWallEntity *this;
  IClientNetworkable *pIVar1;
  
                    /* Unresolved local var: C_ProjectedWallEntity * pRet@[???] */
  this = C_BaseEntity::operator_new(0xb5c);
  C_ProjectedWallEntity::C_ProjectedWallEntity(this);
  pIVar1 = (IClientNetworkable *)0x0;
  if (this != (C_ProjectedWallEntity *)0x0) {
    (*(this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
      super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
      _vptr_IHandleEntity[0x2f])(this,entnum,serialNum);
    pIVar1 = &(this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
              super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable;
  }
  return pIVar1;
}


/* __tcf_5 at 0097bfc0 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3ed42c),in_stack_00000008);
  return;
}


/* C_ProjectedWallEntity::GetPredDescMap at 0052eeb0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: datamap_t * GetPredDescMap(C_ProjectedWallEntity * this) */

datamap_t * __thiscall C_ProjectedWallEntity::GetPredDescMap(C_ProjectedWallEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x6fd9ec);
}


/* PredMapInit<C_ProjectedWallEntity> at 0052eec0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

datamap_t * PredMapInit<C_ProjectedWallEntity>(C_ProjectedWallEntity *param_1)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *(undefined4 *)(extraout_ECX + 0x6fd9dd) = 5;
  *(int *)(extraout_ECX + 0x6fd9d9) = extraout_ECX + 0x6fda35;
  return (datamap_t *)(extraout_ECX + 0x6fd9d9);
}


/* CC_ProjectedWallEntityFactory at 005326c0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

C_BaseEntity * CC_ProjectedWallEntityFactory(void)

{
  C_ProjectedWallEntity *this;
  
  this = C_BaseEntity::operator_new(0xb5c);
  C_ProjectedWallEntity::C_ProjectedWallEntity(this);
  return (C_BaseEntity *)this;
}


/* C_ProjectedWallEntity::C_ProjectedWallEntity at 005324c0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void C_ProjectedWallEntity(C_ProjectedWallEntity * this) */

void __thiscall C_ProjectedWallEntity::C_ProjectedWallEntity(C_ProjectedWallEntity *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseProjectedEntity::C_BaseProjectedEntity((C_BaseProjectedEntity *)this);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x69a9ba);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(&UNK_0069ad96 + unaff_EBX);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(&UNK_0069ae4e + unaff_EBX);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(&UNK_0069ae8a + unaff_EBX);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(&UNK_0069aea6 + unaff_EBX);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_IPaintableEntity.
  _vptr_IPaintableEntity = (_func_int_varargs **)(&UNK_0069aeb2 + unaff_EBX);
  IProjectedWallEntityAutoList::IProjectedWallEntityAutoList
            (&this->super_IProjectedWallEntityAutoList,true);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x69a3ba);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x69a79e);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x69a856);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x69a892);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x69a8ae);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_IPaintableEntity.
  _vptr_IPaintableEntity = (_func_int_varargs **)(unaff_EBX + 0x69a8ba);
  (this->super_IProjectedWallEntityAutoList)._vptr_IProjectedWallEntityAutoList =
       (_func_int_varargs **)(unaff_EBX + 0x69a8d6);
  (this->m_WallCollideables).m_Memory.m_pMemory = (WallCollideableAtTime_t *)0x0;
  (this->m_WallCollideables).m_Memory.m_nAllocationCount = 0;
  (this->m_WallCollideables).m_Memory.m_nGrowSize = 0;
  (this->m_WallCollideables).m_Size = 0;
  (this->m_WallCollideables).m_pElements = (WallCollideableAtTime_t *)0x0;
  (this->m_hColorPortal).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_PaintPowers).m_Memory.m_pMemory = (PaintPowerType *)0x0;
  (this->m_PaintPowers).m_Memory.m_nAllocationCount = 0;
  (this->m_PaintPowers).m_Memory.m_nGrowSize = 0;
  (this->m_PaintPowers).m_Size = 0;
  (this->m_PaintPowers).m_pElements = (PaintPowerType *)0x0;
  this->m_pBodyMaterial = (IMaterial *)0x0;
  this->m_pPaintMaterialMid = (IMaterial *)0x0;
  this->m_pPaintMaterialEnd1 = (IMaterial *)0x0;
  this->m_pPaintMaterialEnd2 = (IMaterial *)0x0;
  this->m_pPaintMaterialSing = (IMaterial *)0x0;
  this->m_pSideRailMaterial = (IMaterial *)0x0;
  this->m_flCurDisplayLength = 0.0;
  this->m_nNumSegments = 0;
  this->m_flSegmentLength = 0.0;
  this->m_flParticleUpdateTime = 0.0;
  this->m_flPrevParticleUpdateTime = 0.0;
  return;
}


/* C_ProjectedWallEntity::C_ProjectedWallEntity at 005326b0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void C_ProjectedWallEntity(C_ProjectedWallEntity * this,
   C_ProjectedWallEntity * this) */

void __thiscall
C_ProjectedWallEntity::C_ProjectedWallEntity
          (C_ProjectedWallEntity *this,C_ProjectedWallEntity *this_1)

{
  C_ProjectedWallEntity(this);
  return;
}


/* CProjectedWallEntity_NonConformantDataHandler::CreateInstance at 00537aa0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void * CreateInstance(CProjectedWallEntity_NonConformantDataHandler *
   this) */

void * __thiscall
CProjectedWallEntity_NonConformantDataHandler::CreateInstance
          (CProjectedWallEntity_NonConformantDataHandler *this)

{
  C_ProjectedWallEntity *this_00;
  
  this_00 = C_BaseEntity::operator_new(0xb5c);
  C_ProjectedWallEntity::C_ProjectedWallEntity(this_00);
  return this_00;
}


/* C_ProjectedWallEntity::UpdateOnRemove at 00530200 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void UpdateOnRemove(C_ProjectedWallEntity * this) */

void __thiscall C_ProjectedWallEntity::UpdateOnRemove(C_ProjectedWallEntity *this)

{
  int unaff_EBX;
  int iVar1;
  int i;
  
  ___i686_get_pc_thunk_bx();
  StopParticleEffects((C_BaseEntity *)this);
  if ((this->m_WallCollideables).m_Size != 0) {
    i = 0;
    iVar1 = 0;
    do {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x633ca6) + 0x44))
                ((int *)**(undefined4 **)(unaff_EBX + 0x633ca6),
                 *(undefined4 *)
                  ((int)((this->m_WallCollideables).m_Memory.m_pMemory)->flTime + iVar1 + 8));
      i = i + 1;
      iVar1 = iVar1 + 0x4c;
    } while ((this->m_WallCollideables).m_Size != i);
  }
  (this->m_WallCollideables).m_Size = 0;
  C_BaseEntity::UpdateOnRemove((C_BaseEntity *)this);
  return;
}


/* C_ProjectedWallEntity::GetCollideType at 0052eef0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: CollideType_t GetCollideType(C_ProjectedWallEntity * this) */

CollideType_t __thiscall C_ProjectedWallEntity::GetCollideType(C_ProjectedWallEntity *this)

{
  return ENTITY_SHOULD_COLLIDE;
}


/* C_ProjectedWallEntity::DrawModel at 00534cf0 */

/* WARNING: Removing unreachable block (ram,0x005352df) */
/* WARNING: Removing unreachable block (ram,0x0053543a) */
/* WARNING: Removing unreachable block (ram,0x0053559f) */
/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: int DrawModel(C_ProjectedWallEntity * this, int flags,
   RenderableInstance_t * instance) */

int __thiscall
C_ProjectedWallEntity::DrawModel
          (C_ProjectedWallEntity *this,int flags,RenderableInstance_t *instance)

{
  undefined4 *puVar1;
  uint *puVar2;
  undefined4 uVar3;
  float fVar4;
  char cVar5;
  int iVar6;
  QAngle *pQVar7;
  float *pfVar8;
  IMesh *pIVar9;
  int *piVar10;
  int iVar11;
  int unaff_EBX;
  float fVar12;
  float fVar13;
  float fVar14;
  float fVar15;
  undefined1 local_1558 [156];
  int local_14bc;
  int local_14b8;
  int local_14b4;
  int local_14a8;
  int *local_14a4;
  uint local_14a0;
  char local_149c;
  int local_1498;
  int local_148c;
  int *local_1488;
  int local_1484;
  int local_1480;
  int local_147c;
  int local_1478;
  undefined4 local_1474;
  undefined4 local_1470;
  undefined4 local_146c;
  undefined1 local_1468;
  int local_1464 [3];
  int local_1458;
  int local_1454;
  int local_144c;
  int local_1448;
  int local_1444;
  int local_1440;
  int local_143c;
  int local_1438;
  int local_1434;
  int local_1430;
  undefined4 local_1418;
  uint local_1414;
  float *local_1410;
  int local_1404;
  undefined4 *local_1400;
  undefined4 *local_13f8;
  int local_13f4;
  int local_13f0;
  int local_13ec;
  int local_13e8;
  int local_13e4;
  int local_13e0;
  int local_13dc;
  int *local_13c0;
  undefined1 local_13bc;
  undefined4 local_13b8;
  int local_13b4;
  int local_13b0;
  float *local_13ac;
  int local_13a8;
  undefined4 *local_13a4;
  undefined4 *local_13a0;
  int local_139c;
  int local_1398;
  int local_1394;
  int local_1390;
  int local_138c;
  int local_1388;
  int local_1384;
  undefined4 local_1380;
  int local_137c;
  int local_1378;
  CMeshBuilder local_1370;
  CMeshBuilder local_1188;
  CMeshBuilder local_fa0;
  CMeshBuilder local_db8;
  CMeshBuilder local_bd0;
  CMeshBuilder local_9e8;
  CMeshBuilder local_800;
  CMeshBuilder local_618;
  CMeshBuilder local_430;
  CMeshBuilder local_248;
  float local_60;
  float local_5c;
  float local_58;
  float local_54;
  float local_50;
  float local_4c;
  float local_48;
  float local_44;
  float local_40;
  float local_3c;
  float local_38;
  float local_34;
  float local_30;
  float local_2c;
  float local_28;
  int local_24;
  int *local_20;
  
  ___i686_get_pc_thunk_bx();
  iVar6 = (*(this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
            super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
            _vptr_IHandleEntity[0xf6])(this);
  iVar11 = 0;
  if ((char)iVar6 != '\0') {
    if ((*(int *)(*(int *)(unaff_EBX + 0x83461b) + 0x30) == 0) ||
       ((this->m_PaintPowers).m_Size != this->m_nNumSegments)) {
      pQVar7 = C_BaseEntity::GetNetworkAngles((C_BaseEntity *)this);
      AngleVectors(pQVar7,(Vector *)&local_db8,(Vector *)&local_9e8,(Vector *)&local_618);
      fVar12 = this->m_flCurDisplayLength;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
      pfVar8 = (float *)(unaff_EBX + 0x4ad50f);
      local_30 = fVar12 * (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position
                 * *pfVar8;
      local_2c = fVar12 * (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.
                                 m_VertexSize_BoneWeight * *pfVar8;
      local_28 = fVar12 * (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.
                                 m_VertexSize_BoneMatrixIndex * *pfVar8;
      pfVar8 = (float *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                          super_C_BaseProjectedEntity.super_C_BaseEntity.super_IClientEntity.
                          super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0xe5])(this);
                    /* Unresolved local var: Vector res@[???] */
      fVar12 = *pfVar8 + local_30;
      fVar13 = pfVar8[1] + local_2c;
      fVar14 = pfVar8[2] + local_28;
      local_3c = fVar12;
      local_38 = fVar13;
      local_34 = fVar14;
      local_20 = (int *)(**(code **)(*(int *)**(undefined4 **)(&DAT_0062f20b + unaff_EBX) + 0x1a0))
                                  ((int *)**(undefined4 **)(&DAT_0062f20b + unaff_EBX));
      if (local_20 != (int *)0x0) {
        (**(code **)(*local_20 + 8))(local_20);
      }
      pIVar9 = (IMesh *)(**(code **)(*local_20 + 0xe4))(local_20,1,0,0,this->m_pBodyMaterial);
      local_1370.m_pMesh = (IMesh *)0x0;
      local_1370.m_bGenerateIndices = false;
      local_1370.m_IndexBuilder.m_pIndexBuffer = (IIndexBuffer *)0x0;
      local_1370.m_IndexBuilder.m_nMaxIndexCount = 0;
      local_1370.m_IndexBuilder.m_nIndexCount = 0;
      local_1370.m_IndexBuilder.m_nCurrentIndex = 0;
      local_1370.m_IndexBuilder.m_nTotalIndexCount = 0;
      local_1370.m_IndexBuilder.m_nBufferOffset = 0xffffffff;
      local_1370.m_IndexBuilder.m_nBufferFirstIndex = 0;
      local_1370.m_VertexBuilder.m_pVertexBuffer = (IVertexBuffer *)0x0;
      local_1370.m_VertexBuilder.m_nBufferOffset = 0xffffffff;
      local_1370.m_VertexBuilder.m_nBufferFirstVertex = 0;
      local_1370.m_VertexBuilder.m_nVertexCount = 0;
      local_1370.m_VertexBuilder.m_nCurrentVertex = 0;
      local_1370.m_VertexBuilder.m_nMaxVertexCount = 0;
      local_1370.m_VertexBuilder.m_nTotalVertexCount = 0;
      local_1370.m_VertexBuilder.super_VertexDesc_t.m_CompressionType = VERTEX_COMPRESSION_INVALID;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x62f0b7) + 0x88))
                ((int *)**(undefined4 **)(unaff_EBX + 0x62f0b7));
      pfVar8 = (float *)(unaff_EBX + 0x4ad50f);
      fVar15 = *pfVar8 * this->m_flCurDisplayLength;
      local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position =
           (int)((float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position * fVar15
                );
      local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight =
           (int)((float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight *
                fVar15);
      local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex =
           (int)(fVar15 * (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.
                                 m_VertexSize_BoneMatrixIndex);
      fVar15 = *pfVar8 * this->m_flWidth;
      local_9e8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position =
           (int)((float)local_9e8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position * fVar15
                );
      local_9e8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight =
           (int)((float)local_9e8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight *
                fVar15);
      local_9e8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex =
           (int)(fVar15 * (float)local_9e8.super_MeshDesc_t.super_VertexDesc_t.
                                 m_VertexSize_BoneMatrixIndex);
      fVar15 = *pfVar8 * this->m_flHeight;
      local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position =
           (int)((float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position * fVar15
                );
      local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight =
           (int)((float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight *
                fVar15);
      local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex =
           (int)(fVar15 * (float)local_618.super_MeshDesc_t.super_VertexDesc_t.
                                 m_VertexSize_BoneMatrixIndex);
      fVar15 = *(float *)(unaff_EBX + 0x4b00c3) * this->m_flCurDisplayLength;
      local_1370.m_bGenerateIndices = true;
      local_1370.m_Type = MATERIAL_QUADS;
      local_1370.m_pMesh = pIVar9;
      (*(pIVar9->super_IVertexBuffer)._vptr_IVertexBuffer[10])(pIVar9,2);
      (*((local_1370.m_pMesh)->super_IVertexBuffer)._vptr_IVertexBuffer[0x11])
                (local_1370.m_pMesh,0x14,0x1e,&local_1370,0);
      local_1370.m_IndexBuilder.m_pIndexBuffer = (IIndexBuffer *)0x0;
      if (pIVar9 != (IMesh *)0x0) {
        local_1370.m_IndexBuilder.m_pIndexBuffer = &pIVar9->super_IIndexBuffer;
      }
      local_1370.m_IndexBuilder.m_nIndexCount = 0;
      local_1370.m_IndexBuilder.m_nMaxIndexCount = 0x1e;
      local_1370.m_IndexBuilder.m_bModify = false;
      local_1370.m_IndexBuilder.m_nIndexOffset =
           local_1370.super_MeshDesc_t.super_VertexDesc_t.m_nFirstVertex;
      local_1370.m_IndexBuilder.super_IndexDesc_t.m_pIndices =
           local_1370.super_MeshDesc_t.super_IndexDesc_t.m_pIndices;
      local_1370.m_IndexBuilder.super_IndexDesc_t.m_nIndexSize =
           local_1370.super_MeshDesc_t.super_IndexDesc_t.m_nIndexSize;
      local_1370.m_IndexBuilder.m_nCurrentIndex = 0;
      local_1370.m_VertexBuilder.m_pVertexBuffer = (IVertexBuffer *)pIVar9;
      _memcpy(&local_1370.m_VertexBuilder,&local_1370,0xa4);
      local_1370.m_VertexBuilder.m_nMaxVertexCount = 0x14;
      local_1370.m_VertexBuilder.super_VertexDesc_t.m_NumBoneWeights =
           ~-(uint)(local_1370.m_VertexBuilder.super_VertexDesc_t.m_NumBoneWeights == 0) & 2;
      local_1370.m_VertexBuilder.m_bModify = false;
      if (local_1370.m_VertexBuilder.m_nBufferOffset == 0xffffffff) {
        local_1370.m_VertexBuilder.m_nTotalVertexCount = 0;
        local_1370.m_VertexBuilder.m_nBufferOffset =
             local_1370.super_MeshDesc_t.super_VertexDesc_t.m_nOffset;
        local_1370.m_VertexBuilder.m_nBufferFirstVertex =
             local_1370.super_MeshDesc_t.super_VertexDesc_t.m_nFirstVertex;
      }
      local_1370.m_IndexBuilder.m_nCurrentIndex = 0;
                    /* Unresolved local var: int i@[???] */
                    /* Unresolved local var: Vector res@[???] */
      local_48 = fVar12 + (float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position
      ;
      local_44 = fVar13 + (float)local_618.super_MeshDesc_t.super_VertexDesc_t.
                                 m_VertexSize_BoneWeight;
      local_40 = fVar14 + (float)local_618.super_MeshDesc_t.super_VertexDesc_t.
                                 m_VertexSize_BoneMatrixIndex;
                    /* Unresolved local var: Vector vert@[???]
                       Unresolved local var: float[4] flColors@[???] */
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
      local_24 = *(int *)(unaff_EBX + 0x4b2557);
      *(uint *)local_1370.m_VertexBuilder.super_VertexDesc_t.m_pColor = local_24 << 0x18 | 0xffffff;
                    /* Unresolved local var: float * pDst@[???] */
      *local_1370.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[0] = 0.0;
      local_1370.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[0][1] = fVar15;
                    /* Unresolved local var: float * pDst@[???] */
      *local_1370.m_VertexBuilder.super_VertexDesc_t.m_pPosition =
           (local_48 + (float)local_9e8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position) -
           (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position;
      local_1370.m_VertexBuilder.super_VertexDesc_t.m_pPosition[1] =
           (local_44 + (float)local_9e8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight)
           - (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight;
      local_1370.m_VertexBuilder.super_VertexDesc_t.m_pPosition[2] =
           (local_40 +
           (float)local_9e8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex) -
           (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex;
      local_1370.m_VertexBuilder.super_VertexDesc_t.m_pPosition =
           (float *)((int)local_1370.m_VertexBuilder.super_VertexDesc_t.m_pPosition +
                    local_1370.m_VertexBuilder.super_VertexDesc_t.m_VertexSize_Position);
      local_1370.m_VertexBuilder.m_pCurrNormal =
           local_1370.m_VertexBuilder.super_VertexDesc_t.m_pNormal +
           local_1370.m_VertexBuilder.super_VertexDesc_t.m_VertexSize_Normal;
      puVar1 = (undefined4 *)
               ((int)local_1370.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[0] +
               local_1370.m_VertexBuilder.super_VertexDesc_t.m_VertexSize_TexCoord[0]);
      local_1370.m_VertexBuilder.m_pCurrTexCoord[1] =
           local_1370.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[1] +
           local_1370.m_VertexBuilder.super_VertexDesc_t.m_VertexSize_TexCoord[1];
      local_1370.m_VertexBuilder.m_pCurrTexCoord[2] =
           local_1370.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[2] +
           local_1370.m_VertexBuilder.super_VertexDesc_t.m_VertexSize_TexCoord[2];
      local_1370.m_VertexBuilder.m_pCurrTexCoord[3] =
           local_1370.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[3] +
           local_1370.m_VertexBuilder.super_VertexDesc_t.m_VertexSize_TexCoord[3];
      local_1370.m_VertexBuilder.m_pCurrTexCoord[4] =
           local_1370.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[4] +
           local_1370.m_VertexBuilder.super_VertexDesc_t.m_VertexSize_TexCoord[4];
      local_1370.m_VertexBuilder.m_pCurrTexCoord[5] =
           local_1370.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[5] +
           local_1370.m_VertexBuilder.super_VertexDesc_t.m_VertexSize_TexCoord[5];
      local_1370.m_VertexBuilder.m_pCurrTexCoord[6] =
           local_1370.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[6] +
           local_1370.m_VertexBuilder.super_VertexDesc_t.m_VertexSize_TexCoord[6];
      local_1370.m_VertexBuilder.m_pCurrTexCoord[7] =
           local_1370.m_VertexBuilder.super_VertexDesc_t.m_pTexCoord[7] +
           local_1370.m_VertexBuilder.super_VertexDesc_t.m_VertexSize_TexCoord[7];
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
      *(uint *)((int)local_1370.m_VertexBuilder.super_VertexDesc_t.m_pColor +
               local_1370.m_VertexBuilder.super_VertexDesc_t.m_VertexSize_Color) =
           local_24 << 0x18 | 0xffffff;
                    /* Unresolved local var: float * pDst@[???] */
      *puVar1 = 0;
      puVar1[1] = 0;
                    /* Unresolved local var: float * pDst@[???] */
      *local_1370.m_VertexBuilder.super_VertexDesc_t.m_pPosition =
           local_48 + (float)local_9e8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position +
           (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position;
      local_1370.m_VertexBuilder.super_VertexDesc_t.m_pPosition[1] =
           local_44 + (float)local_9e8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight +
           (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight;
      local_1370.m_VertexBuilder.super_VertexDesc_t.m_pPosition[2] =
           local_40 +
           (float)local_9e8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex +
           (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex;
      local_1370.m_VertexBuilder.super_VertexDesc_t.m_pPosition =
           (float *)((int)local_1370.m_VertexBuilder.super_VertexDesc_t.m_pPosition +
                    local_1370.m_VertexBuilder.super_VertexDesc_t.m_VertexSize_Position);
      puVar2 = (uint *)((int)local_1370.m_VertexBuilder.super_VertexDesc_t.m_pColor +
                        local_1370.m_VertexBuilder.super_VertexDesc_t.m_VertexSize_Color +
                       local_1370.m_VertexBuilder.super_VertexDesc_t.m_VertexSize_Color);
      puVar1 = (undefined4 *)
               ((int)puVar1 + local_1370.m_VertexBuilder.super_VertexDesc_t.m_VertexSize_TexCoord[0]
               );
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
      *puVar2 = local_24 << 0x18 | 0xffffff;
                    /* Unresolved local var: float * pDst@[???] */
      uVar3 = *(undefined4 *)(unaff_EBX + 0x4ad483);
      *puVar1 = uVar3;
      puVar1[1] = 0;
                    /* Unresolved local var: float * pDst@[???] */
      *local_1370.m_VertexBuilder.super_VertexDesc_t.m_pPosition =
           (local_48 - (float)local_9e8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position) +
           (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position;
      local_1370.m_VertexBuilder.super_VertexDesc_t.m_pPosition[1] =
           (local_44 - (float)local_9e8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight)
           + (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight;
      local_1370.m_VertexBuilder.super_VertexDesc_t.m_pPosition[2] =
           (local_40 -
           (float)local_9e8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex) +
           (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex;
      local_1370.m_VertexBuilder.super_VertexDesc_t.m_pPosition =
           (float *)((int)local_1370.m_VertexBuilder.super_VertexDesc_t.m_pPosition +
                    local_1370.m_VertexBuilder.super_VertexDesc_t.m_VertexSize_Position);
      puVar2 = (uint *)((int)puVar2 +
                       local_1370.m_VertexBuilder.super_VertexDesc_t.m_VertexSize_Color);
      puVar1 = (undefined4 *)
               ((int)puVar1 + local_1370.m_VertexBuilder.super_VertexDesc_t.m_VertexSize_TexCoord[0]
               );
      local_248.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position =
           (int)((local_48 -
                 (float)local_9e8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position) -
                (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position);
      local_248.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight =
           (int)((local_44 -
                 (float)local_9e8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight) -
                (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight);
      local_248.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex =
           (int)((local_40 -
                 (float)local_9e8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex)
                - (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex)
      ;
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
      *puVar2 = local_24 << 0x18 | 0xffffff;
                    /* Unresolved local var: float * pDst@[???] */
      *puVar1 = uVar3;
      puVar1[1] = fVar15;
                    /* Unresolved local var: float * pDst@[???] */
      *local_1370.m_VertexBuilder.super_VertexDesc_t.m_pPosition =
           (float)local_248.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position;
      local_1370.m_VertexBuilder.super_VertexDesc_t.m_pPosition[1] =
           (float)local_248.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight;
      local_1370.m_VertexBuilder.super_VertexDesc_t.m_pPosition[2] =
           (float)local_248.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex;
      local_1370.m_VertexBuilder.m_nCurrentVertex = 4;
      local_1370.m_VertexBuilder.m_nVertexCount = 4;
      local_1370.m_VertexBuilder.m_pCurrPosition =
           (float *)((int)local_1370.m_VertexBuilder.super_VertexDesc_t.m_pPosition +
                    local_1370.m_VertexBuilder.super_VertexDesc_t.m_VertexSize_Position);
      local_1370.m_VertexBuilder.m_pCurrColor =
           (uchar *)((int)puVar2 + local_1370.m_VertexBuilder.super_VertexDesc_t.m_VertexSize_Color)
      ;
      local_1370.m_VertexBuilder.m_pCurrTexCoord[0] =
           (float *)((int)puVar1 +
                    local_1370.m_VertexBuilder.super_VertexDesc_t.m_VertexSize_TexCoord[0]);
      if (local_1370.m_bGenerateIndices != false) {
        if (local_1370.m_Type - MATERIAL_LINE_STRIP < 7) {
                    /* WARNING: Could not recover jumptable at 0x005357c3. Too many branches */
                    /* WARNING: Treating indirect jump as call */
          iVar6 = (*(code *)(*(int *)(unaff_EBX + 0xac7 +
                                     (local_1370.m_Type - MATERIAL_LINE_STRIP) * 4) + unaff_EBX))();
          return iVar6;
        }
        if (local_1370.m_IndexBuilder.super_IndexDesc_t.m_nIndexSize != 0) {
          iVar6 = local_1370.m_IndexBuilder.m_nMaxIndexCount;
          if (3 < local_1370.m_IndexBuilder.m_nMaxIndexCount) {
            iVar6 = local_1370.m_VertexBuilder.m_nVertexCount;
          }
          if (iVar6 != 0) {
            if (local_1370.m_Type < MATERIAL_HETEROGENOUS) {
                    /* WARNING: Could not recover jumptable at 0x0053707f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
              iVar6 = (*(code *)(*(int *)(unaff_EBX + 0x2383 + local_1370.m_Type * 4) + unaff_EBX))
                                ();
              return iVar6;
            }
            GenerateSequentialIndexBuffer
                      (local_1370.m_IndexBuilder.super_IndexDesc_t.m_pIndices,iVar6,
                       local_1370.m_IndexBuilder.m_nIndexOffset);
            local_1370.m_IndexBuilder.m_nCurrentIndex =
                 iVar6 * local_1370.m_IndexBuilder.super_IndexDesc_t.m_nIndexSize +
                 local_1370.m_IndexBuilder.m_nCurrentIndex;
            if (local_1370.m_IndexBuilder.m_nIndexCount < local_1370.m_IndexBuilder.m_nCurrentIndex)
            {
              local_1370.m_IndexBuilder.m_nIndexCount = local_1370.m_IndexBuilder.m_nCurrentIndex;
            }
          }
        }
      }
      (*((local_1370.m_pMesh)->super_IVertexBuffer)._vptr_IVertexBuffer[0x14])
                (local_1370.m_pMesh,local_1370.m_VertexBuilder.m_nVertexCount,
                 local_1370.m_IndexBuilder.m_nIndexCount,&local_1370);
      local_1370.m_IndexBuilder.m_pIndexBuffer = (IIndexBuffer *)0x0;
      local_1370.m_IndexBuilder.m_nMaxIndexCount = 0;
      local_1370.m_VertexBuilder.m_nMaxVertexCount = 0;
      local_1370.m_VertexBuilder.m_pVertexBuffer = (IVertexBuffer *)0x0;
      local_1370.m_VertexBuilder.super_VertexDesc_t.m_CompressionType = VERTEX_COMPRESSION_INVALID;
      local_1370.m_pMesh = (IMesh *)0x0;
      (*(pIVar9->super_IVertexBuffer)._vptr_IVertexBuffer[0xb])(pIVar9,0xffffffff,0);
      piVar10 = (int *)(**(code **)(*local_20 + 0xe4))(local_20,1,0,0,this->m_pSideRailMaterial);
      local_1488 = (int *)0x0;
      local_1484 = 0;
      local_1480 = 0;
      local_1478 = 0;
      local_1474 = 0;
      local_1470 = 0xffffffff;
      local_146c = 0;
      local_13c0 = (int *)0x0;
      local_137c = -1;
      local_1378 = 0;
      local_13b4 = 0;
      local_13b0 = 0;
      local_13b8 = 0;
      local_1380 = 0;
      local_1418 = 0xffffffff;
      local_149c = '\x01';
      local_14a0 = 7;
      local_14a4 = piVar10;
      (**(code **)(*piVar10 + 0x28))(piVar10,2);
      (**(code **)(*local_14a4 + 0x44))(local_14a4,0x14,0x1e,local_1558,0);
      local_1488 = (int *)0x0;
      if (piVar10 != (int *)0x0) {
        local_1488 = piVar10 + 1;
      }
      local_1480 = 0;
      local_1484 = 0x1e;
      local_1468 = 0;
      local_147c = local_14bc;
      local_1498 = local_14b4;
      local_148c = local_14a8;
      local_1478 = 0;
      local_13c0 = piVar10;
      _memcpy(local_1464,local_1558,0xa4);
      local_13b8 = 0x14;
      local_1414 = ~-(uint)(local_1414 == 0) & 2;
      local_13b4 = 0;
      local_13bc = 0;
      if (local_137c == -1) {
        local_1380 = 0;
        local_137c = local_14b8;
        local_1378 = local_14bc;
      }
      local_1478 = 0;
      local_13b0 = 0;
      local_13ac = local_1410;
      local_13a8 = local_1404;
                    /* Unresolved local var: int i@[???] */
      local_13a0 = local_13f8;
      local_139c = local_13f4;
      local_1398 = local_13f0;
      local_1394 = local_13ec;
      local_1390 = local_13e8;
      local_138c = local_13e4;
      local_1388 = local_13e0;
      local_1384 = local_13dc;
      local_13a4 = local_1400;
      pQVar7 = C_BaseEntity::GetNetworkAngles((C_BaseEntity *)this);
      AngleVectors(pQVar7,(Vector *)0x0,(Vector *)0x0,(Vector *)&local_618);
      fVar4 = *(float *)(unaff_EBX + 0x4ad90f);
      local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position =
           (int)((float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position * fVar4)
      ;
      local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight =
           (int)((float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight *
                fVar4);
      local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex =
           (int)(fVar4 * (float)local_618.super_MeshDesc_t.super_VertexDesc_t.
                                m_VertexSize_BoneMatrixIndex);
                    /* Unresolved local var: Vector res@[???] */
      local_54 = fVar12 + (float)local_9e8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position
      ;
      local_50 = fVar13 + (float)local_9e8.super_MeshDesc_t.super_VertexDesc_t.
                                 m_VertexSize_BoneWeight;
      local_4c = fVar14 + (float)local_9e8.super_MeshDesc_t.super_VertexDesc_t.
                                 m_VertexSize_BoneMatrixIndex;
                    /* Unresolved local var: Vector vert@[???]
                       Unresolved local var: float[4] flColors@[???] */
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
      *local_13a4 = 0xffffffff;
                    /* Unresolved local var: float * pDst@[???] */
      *local_13a0 = 0;
      local_13a0[1] = fVar15;
                    /* Unresolved local var: float * pDst@[???] */
      *local_13ac = (local_54 +
                    (float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position) -
                    (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position;
      local_13ac[1] =
           (local_50 + (float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight)
           - (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight;
      local_13ac[2] =
           (local_4c +
           (float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex) -
           (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex;
      iVar6 = local_13b0 + 1;
      if (local_13b0 + 1 <= local_13b4) {
        iVar6 = local_13b4;
      }
      local_13ac = (float *)((int)local_13ac + local_1464[0]);
      local_13a8 = local_13a8 + local_1458 * 8;
      local_13a0 = (undefined4 *)((int)local_13a0 + local_144c);
      local_139c = local_139c + local_1448 * 8;
      local_1398 = local_1398 + local_1444 * 8;
      local_1394 = local_1394 + local_1440 * 8;
      local_1390 = local_1390 + local_143c * 8;
      local_138c = local_138c + local_1438 * 8;
      local_1388 = local_1388 + local_1434 * 8;
      local_1384 = local_1384 + local_1430 * 8;
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
      *(undefined4 *)((int)local_13a4 + local_1454) = 0xffffffff;
                    /* Unresolved local var: float * pDst@[???] */
      *local_13a0 = 0;
      local_13a0[1] = 0;
                    /* Unresolved local var: float * pDst@[???] */
      *local_13ac = local_54 +
                    (float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position +
                    (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position;
      local_13ac[1] =
           local_50 + (float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight +
           (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight;
      local_13ac[2] =
           local_4c +
           (float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex +
           (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex;
      iVar11 = local_13b0 + 2;
      if (local_13b0 + 2 <= iVar6) {
        iVar11 = iVar6;
      }
      local_13ac = (float *)((int)local_13ac + local_1464[0]);
      local_13a4 = (undefined4 *)((int)local_13a4 + local_1454 + local_1454);
      local_13a0 = (undefined4 *)((int)local_13a0 + local_144c);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
      *local_13a4 = 0xffffffff;
                    /* Unresolved local var: float * pDst@[???] */
      *local_13a0 = 0x3f800000;
      local_13a0[1] = 0;
                    /* Unresolved local var: float * pDst@[???] */
      *local_13ac = (local_54 -
                    (float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position) +
                    (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position;
      local_13ac[1] =
           (local_50 - (float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight)
           + (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight;
      local_13ac[2] =
           (local_4c -
           (float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex) +
           (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex;
      iVar6 = local_13b0 + 3;
      if (local_13b0 + 3 <= iVar11) {
        iVar6 = iVar11;
      }
      local_13ac = (float *)((int)local_13ac + local_1464[0]);
      local_13a4 = (undefined4 *)((int)local_13a4 + local_1454);
      local_13a0 = (undefined4 *)((int)local_13a0 + local_144c);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
      *local_13a4 = 0xffffffff;
                    /* Unresolved local var: float * pDst@[???] */
      *local_13a0 = 0x3f800000;
      local_13a0[1] = fVar15;
                    /* Unresolved local var: float * pDst@[???] */
      *local_13ac = (local_54 -
                    (float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position) -
                    (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position;
      local_13ac[1] =
           (local_50 - (float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight)
           - (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight;
      local_13ac[2] =
           (local_4c -
           (float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex) -
           (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex;
      iVar11 = local_13b0 + 4;
      if (local_13b0 + 4 <= iVar6) {
        iVar11 = iVar6;
      }
      local_13ac = (float *)((int)local_13ac + local_1464[0]);
      local_13a4 = (undefined4 *)((int)local_13a4 + local_1454);
      local_13a0 = (undefined4 *)((int)local_13a0 + local_144c);
                    /* Unresolved local var: Vector res@[???] */
      local_60 = fVar12 - (float)local_9e8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position
      ;
      local_5c = fVar13 - (float)local_9e8.super_MeshDesc_t.super_VertexDesc_t.
                                 m_VertexSize_BoneWeight;
      local_58 = fVar14 - (float)local_9e8.super_MeshDesc_t.super_VertexDesc_t.
                                 m_VertexSize_BoneMatrixIndex;
                    /* Unresolved local var: Vector vert@[???]
                       Unresolved local var: float[4] flColors@[???] */
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
      *local_13a4 = 0xffffffff;
                    /* Unresolved local var: float * pDst@[???] */
      *local_13a0 = 0;
      local_13a0[1] = fVar15;
                    /* Unresolved local var: float * pDst@[???] */
      *local_13ac = (local_60 +
                    (float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position) -
                    (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position;
      local_13ac[1] =
           (local_5c + (float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight)
           - (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight;
      local_13ac[2] =
           (local_58 +
           (float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex) -
           (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex;
      iVar6 = local_13b0 + 5;
      if (local_13b0 + 5 <= iVar11) {
        iVar6 = iVar11;
      }
      local_13ac = (float *)((int)local_13ac + local_1464[0]);
      local_13a4 = (undefined4 *)((int)local_13a4 + local_1454);
      local_13a0 = (undefined4 *)((int)local_13a0 + local_144c);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
      *local_13a4 = 0xffffffff;
                    /* Unresolved local var: float * pDst@[???] */
      *local_13a0 = 0;
      local_13a0[1] = 0;
                    /* Unresolved local var: float * pDst@[???] */
      *local_13ac = local_60 +
                    (float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position +
                    (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position;
      local_13ac[1] =
           local_5c + (float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight +
           (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight;
      local_13ac[2] =
           local_58 +
           (float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex +
           (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex;
      iVar11 = local_13b0 + 6;
      if (local_13b0 + 6 <= iVar6) {
        iVar11 = iVar6;
      }
      local_13ac = (float *)((int)local_13ac + local_1464[0]);
      local_13a4 = (undefined4 *)((int)local_13a4 + local_1454);
      local_13a0 = (undefined4 *)((int)local_13a0 + local_144c);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
      *local_13a4 = 0xffffffff;
                    /* Unresolved local var: float * pDst@[???] */
      *local_13a0 = 0x3f800000;
      local_13a0[1] = 0;
                    /* Unresolved local var: float * pDst@[???] */
      *local_13ac = (local_60 -
                    (float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position) +
                    (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position;
      local_13ac[1] =
           (local_5c - (float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight)
           + (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight;
      local_13ac[2] =
           (local_58 -
           (float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex) +
           (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex;
      local_13b4 = local_13b0 + 7;
      if (local_13b0 + 7 <= iVar11) {
        local_13b4 = iVar11;
      }
      local_13ac = (float *)((int)local_13ac + local_1464[0]);
      local_13a4 = (undefined4 *)((int)local_13a4 + local_1454);
      local_13a0 = (undefined4 *)((int)local_13a0 + local_144c);
      local_248.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position =
           (int)((local_60 -
                 (float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position) -
                (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position);
      local_248.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight =
           (int)((local_5c -
                 (float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight) -
                (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight);
      local_248.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex =
           (int)((local_58 -
                 (float)local_618.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex)
                - (float)local_db8.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex)
      ;
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
      local_24 = 0x4b0000ff;
      *local_13a4 = 0xffffffff;
                    /* Unresolved local var: float * pDst@[???] */
      *local_13a0 = 0x3f800000;
      local_13a0[1] = fVar15;
                    /* Unresolved local var: float * pDst@[???] */
      *local_13ac = (float)local_248.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_Position;
      local_13ac[1] = (float)local_248.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneWeight;
      local_13ac[2] =
           (float)local_248.super_MeshDesc_t.super_VertexDesc_t.m_VertexSize_BoneMatrixIndex;
      local_13b0 = local_13b0 + 8;
      if (local_13b4 < local_13b0) {
        local_13b4 = local_13b0;
      }
      local_13ac = (float *)((int)local_13ac + local_1464[0]);
      local_13a4 = (undefined4 *)((int)local_13a4 + local_1454);
      local_13a0 = (undefined4 *)((int)local_13a0 + local_144c);
      if (local_149c != '\0') {
        if (local_14a0 - 4 < 7) {
                    /* WARNING: Could not recover jumptable at 0x0053693b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
          iVar6 = (*(code *)(*(int *)(unaff_EBX + 0x1c3f + (local_14a0 - 4) * 4) + unaff_EBX))();
          return iVar6;
        }
        if (local_148c != 0) {
          iVar6 = local_1484 - local_1478;
          if (local_13b4 <= local_1484 - local_1478) {
            iVar6 = local_13b4;
          }
          if (iVar6 != 0) {
            if (local_14a0 < 0xb) {
                    /* WARNING: Could not recover jumptable at 0x005370b9. Too many branches */
                    /* WARNING: Treating indirect jump as call */
              iVar6 = (*(code *)(*(int *)(unaff_EBX + 0x23bb + local_14a0 * 4) + unaff_EBX))();
              return iVar6;
            }
            GenerateSequentialIndexBuffer((ushort *)(local_1478 * 2 + local_1498),iVar6,local_147c);
            local_1478 = iVar6 * local_148c + local_1478;
            if (local_1480 < local_1478) {
              local_1480 = local_1478;
            }
          }
        }
      }
      (**(code **)(*local_14a4 + 0x50))(local_14a4,local_13b4,local_1480,local_1558);
      local_1488 = (int *)0x0;
      local_1484 = 0;
      local_13b8 = 0;
      local_13c0 = (int *)0x0;
      local_1418 = 0xffffffff;
      local_14a4 = (int *)0x0;
      iVar6 = -1;
      (**(code **)(*piVar10 + 0x2c))(piVar10,0xffffffff,0);
      if ((local_13c0 != (int *)0x0) &&
         (cVar5 = (**(code **)(*local_13c0 + 8))(local_13c0), cVar5 != '\0')) {
        (**(code **)(*local_13c0 + 0x10))(local_13c0);
      }
      if ((local_1488 != (int *)0x0) &&
         (cVar5 = (**(code **)(*local_1488 + 8))(local_1488), cVar5 != '\0')) {
        (**(code **)(*local_1488 + 0x10))(local_1488);
      }
      CMeshBuilder::~CMeshBuilder(&local_1370,iVar6);
      if (local_20 != (int *)0x0) {
        (**(code **)(*local_20 + 0xc))(local_20);
      }
    }
    else {
      local_20 = (int *)(**(code **)(*(int *)**(undefined4 **)(&DAT_0062f20b + unaff_EBX) + 0x1a0))
                                  ((int *)**(undefined4 **)(&DAT_0062f20b + unaff_EBX));
      if (local_20 != (int *)0x0) {
        (**(code **)(*local_20 + 8))(local_20);
      }
      pIVar9 = (IMesh *)(**(code **)(*local_20 + 0xe4))(local_20,1,0,0,this->m_pBodyMaterial);
      local_248.m_pMesh = (IMesh *)0x0;
      local_248.m_bGenerateIndices = false;
      local_248.m_IndexBuilder.m_pIndexBuffer = (IIndexBuffer *)0x0;
      local_248.m_IndexBuilder.m_nMaxIndexCount = 0;
      local_248.m_IndexBuilder.m_nIndexCount = 0;
      local_248.m_IndexBuilder.m_nCurrentIndex = 0;
      local_248.m_IndexBuilder.m_nTotalIndexCount = 0;
      local_248.m_IndexBuilder.m_nBufferOffset = 0xffffffff;
      local_248.m_IndexBuilder.m_nBufferFirstIndex = 0;
      local_248.m_VertexBuilder.m_pVertexBuffer = (IVertexBuffer *)0x0;
      local_248.m_VertexBuilder.m_nBufferOffset = 0xffffffff;
      local_248.m_VertexBuilder.m_nBufferFirstVertex = 0;
      local_248.m_VertexBuilder.m_nVertexCount = 0;
      local_248.m_VertexBuilder.m_nCurrentVertex = 0;
      local_248.m_VertexBuilder.m_nMaxVertexCount = 0;
      local_248.m_VertexBuilder.m_nTotalVertexCount = 0;
      local_248.m_VertexBuilder.super_VertexDesc_t.m_CompressionType = VERTEX_COMPRESSION_INVALID;
      _memcpy(&local_430,&local_248,0x1e8);
      PaintWallWithPaint(this,pIVar9,&local_430,BASE_SEGMENT);
      CMeshBuilder::~CMeshBuilder(&local_430,(int)pIVar9);
      pIVar9 = (IMesh *)(**(code **)(*local_20 + 0xe4))(local_20,1,0,0,this->m_pPaintMaterialMid);
      local_618.m_pMesh = (IMesh *)0x0;
      local_618.m_bGenerateIndices = false;
      local_618.m_IndexBuilder.m_pIndexBuffer = (IIndexBuffer *)0x0;
      local_618.m_IndexBuilder.m_nMaxIndexCount = 0;
      local_618.m_IndexBuilder.m_nIndexCount = 0;
      local_618.m_IndexBuilder.m_nCurrentIndex = 0;
      local_618.m_IndexBuilder.m_nTotalIndexCount = 0;
      local_618.m_IndexBuilder.m_nBufferOffset = 0xffffffff;
      local_618.m_IndexBuilder.m_nBufferFirstIndex = 0;
      local_618.m_VertexBuilder.m_pVertexBuffer = (IVertexBuffer *)0x0;
      local_618.m_VertexBuilder.m_nBufferOffset = 0xffffffff;
      local_618.m_VertexBuilder.m_nBufferFirstVertex = 0;
      local_618.m_VertexBuilder.m_nVertexCount = 0;
      local_618.m_VertexBuilder.m_nCurrentVertex = 0;
      local_618.m_VertexBuilder.m_nMaxVertexCount = 0;
      local_618.m_VertexBuilder.m_nTotalVertexCount = 0;
      local_618.m_VertexBuilder.super_VertexDesc_t.m_CompressionType = VERTEX_COMPRESSION_INVALID;
      _memcpy(&local_800,&local_618,0x1e8);
      PaintWallWithPaint(this,pIVar9,&local_800,PAINT_MIDDLE_SEGMENT);
      CMeshBuilder::~CMeshBuilder(&local_800,(int)pIVar9);
      pIVar9 = (IMesh *)(**(code **)(*local_20 + 0xe4))(local_20,1,0,0,this->m_pPaintMaterialEnd1);
      local_9e8.m_pMesh = (IMesh *)0x0;
      local_9e8.m_bGenerateIndices = false;
      local_9e8.m_IndexBuilder.m_pIndexBuffer = (IIndexBuffer *)0x0;
      local_9e8.m_IndexBuilder.m_nMaxIndexCount = 0;
      local_9e8.m_IndexBuilder.m_nIndexCount = 0;
      local_9e8.m_IndexBuilder.m_nCurrentIndex = 0;
      local_9e8.m_IndexBuilder.m_nTotalIndexCount = 0;
      local_9e8.m_IndexBuilder.m_nBufferOffset = 0xffffffff;
      local_9e8.m_IndexBuilder.m_nBufferFirstIndex = 0;
      local_9e8.m_VertexBuilder.m_pVertexBuffer = (IVertexBuffer *)0x0;
      local_9e8.m_VertexBuilder.m_nBufferOffset = 0xffffffff;
      local_9e8.m_VertexBuilder.m_nBufferFirstVertex = 0;
      local_9e8.m_VertexBuilder.m_nVertexCount = 0;
      local_9e8.m_VertexBuilder.m_nCurrentVertex = 0;
      local_9e8.m_VertexBuilder.m_nMaxVertexCount = 0;
      local_9e8.m_VertexBuilder.m_nTotalVertexCount = 0;
      local_9e8.m_VertexBuilder.super_VertexDesc_t.m_CompressionType = VERTEX_COMPRESSION_INVALID;
      _memcpy(&local_bd0,&local_9e8,0x1e8);
      PaintWallWithPaint(this,pIVar9,&local_bd0,PAINT_END_1_SEGMENT);
      CMeshBuilder::~CMeshBuilder(&local_bd0,(int)pIVar9);
      pIVar9 = (IMesh *)(**(code **)(*local_20 + 0xe4))(local_20,1,0,0,this->m_pPaintMaterialEnd2);
      local_db8.m_pMesh = (IMesh *)0x0;
      local_db8.m_bGenerateIndices = false;
      local_db8.m_IndexBuilder.m_pIndexBuffer = (IIndexBuffer *)0x0;
      local_db8.m_IndexBuilder.m_nMaxIndexCount = 0;
      local_db8.m_IndexBuilder.m_nIndexCount = 0;
      local_db8.m_IndexBuilder.m_nCurrentIndex = 0;
      local_db8.m_IndexBuilder.m_nTotalIndexCount = 0;
      local_db8.m_IndexBuilder.m_nBufferOffset = 0xffffffff;
      local_db8.m_IndexBuilder.m_nBufferFirstIndex = 0;
      local_db8.m_VertexBuilder.m_pVertexBuffer = (IVertexBuffer *)0x0;
      local_db8.m_VertexBuilder.m_nBufferOffset = 0xffffffff;
      local_db8.m_VertexBuilder.m_nBufferFirstVertex = 0;
      local_db8.m_VertexBuilder.m_nVertexCount = 0;
      local_db8.m_VertexBuilder.m_nCurrentVertex = 0;
      local_db8.m_VertexBuilder.m_nMaxVertexCount = 0;
      local_db8.m_VertexBuilder.m_nTotalVertexCount = 0;
      local_db8.m_VertexBuilder.super_VertexDesc_t.m_CompressionType = VERTEX_COMPRESSION_INVALID;
      _memcpy(&local_fa0,&local_db8,0x1e8);
      PaintWallWithPaint(this,pIVar9,&local_fa0,PAINT_END_2_SEGMENT);
      CMeshBuilder::~CMeshBuilder(&local_fa0,(int)pIVar9);
      pIVar9 = (IMesh *)(**(code **)(*local_20 + 0xe4))(local_20,1,0,0,this->m_pPaintMaterialSing);
      local_1370.m_pMesh = (IMesh *)0x0;
      local_1370.m_bGenerateIndices = false;
      local_1370.m_IndexBuilder.m_pIndexBuffer = (IIndexBuffer *)0x0;
      local_1370.m_IndexBuilder.m_nMaxIndexCount = 0;
      local_1370.m_IndexBuilder.m_nIndexCount = 0;
      local_1370.m_IndexBuilder.m_nCurrentIndex = 0;
      local_1370.m_IndexBuilder.m_nTotalIndexCount = 0;
      local_1370.m_IndexBuilder.m_nBufferOffset = 0xffffffff;
      local_1370.m_IndexBuilder.m_nBufferFirstIndex = 0;
      local_1370.m_VertexBuilder.m_pVertexBuffer = (IVertexBuffer *)0x0;
      local_1370.m_VertexBuilder.m_nBufferOffset = 0xffffffff;
      local_1370.m_VertexBuilder.m_nBufferFirstVertex = 0;
      local_1370.m_VertexBuilder.m_nVertexCount = 0;
      local_1370.m_VertexBuilder.m_nCurrentVertex = 0;
      local_1370.m_VertexBuilder.m_nMaxVertexCount = 0;
      local_1370.m_VertexBuilder.m_nTotalVertexCount = 0;
      local_1370.m_VertexBuilder.super_VertexDesc_t.m_CompressionType = VERTEX_COMPRESSION_INVALID;
      _memcpy(&local_1188,&local_1370,0x1e8);
      PaintWallWithPaint(this,pIVar9,&local_1188,PAINT_SINGLE_SEGMENT);
      CMeshBuilder::~CMeshBuilder(&local_1188,(int)pIVar9);
      CMeshBuilder::~CMeshBuilder(&local_1370,(int)pIVar9);
      CMeshBuilder::~CMeshBuilder(&local_db8,(int)pIVar9);
      CMeshBuilder::~CMeshBuilder(&local_9e8,(int)pIVar9);
      CMeshBuilder::~CMeshBuilder(&local_618,(int)pIVar9);
      CMeshBuilder::~CMeshBuilder(&local_248,(int)pIVar9);
      if (local_20 != (int *)0x0) {
        (**(code **)(*local_20 + 0xc))(local_20);
      }
    }
    if (local_20 == (int *)0x0) {
      return 1;
    }
    (**(code **)(*local_20 + 4))(local_20);
    iVar11 = 1;
  }
  return iVar11;
}


/* C_ProjectedWallEntity::PaintWallWithPaint at 00533c00 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void PaintWallWithPaint(C_ProjectedWallEntity * this, IMesh * pMesh,
   CMeshBuilder * meshBuilder, ProjectedWallSegmentType type) */

void __thiscall
C_ProjectedWallEntity::PaintWallWithPaint
          (C_ProjectedWallEntity *this,IMesh *pMesh,CMeshBuilder *meshBuilder,
          ProjectedWallSegmentType type)

{
  uint *puVar1;
  Vector vecOrigin;
  Vector vecOrigin_00;
  Vector vecOrigin_01;
  Vector vecOrigin_02;
  Vector vecOrigin_03;
  Vector vecUp;
  Vector vecUp_00;
  Vector vecUp_01;
  Vector vecUp_02;
  Vector vecUp_03;
  Vector vecForward;
  Vector vecForward_00;
  Vector vecForward_01;
  Vector vecForward_02;
  Vector vecForward_03;
  Vector vecRight;
  Vector vecRight_00;
  Vector vecRight_01;
  Vector vecRight_02;
  Vector vecRight_03;
  bool bVar2;
  QAngle *angles;
  float *pfVar3;
  int iVar4;
  PaintPowerType *pPVar5;
  int unaff_EBX;
  bool bVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  bool bDebugging;
  float local_b08;
  uint local_b00;
  int local_afc;
  int local_af8;
  int local_af4;
  float local_af0;
  CMeshBuilder local_aec;
  CMeshBuilder local_904;
  CMeshBuilder local_71c;
  CMeshBuilder local_534;
  CMeshBuilder local_34c;
  float local_164;
  float local_160;
  float local_15c;
  float local_158;
  float local_154;
  float local_150;
  float local_14c;
  float local_148;
  float local_144;
  float local_140;
  float local_13c;
  float local_138;
  float local_134;
  float local_130;
  float local_12c;
  float local_128;
  float local_124;
  float local_120;
  float local_11c;
  float local_118;
  float local_114;
  float local_110;
  float local_10c;
  float local_108;
  float local_104;
  float local_100;
  float local_fc;
  Vector local_f8;
  Vector local_ec;
  float local_e0;
  float local_dc;
  float local_d8;
  float local_d4;
  float local_d0;
  float local_cc;
  float local_c8;
  float local_c4;
  float local_c0;
  float local_bc;
  float local_b8;
  float local_b4;
  char *local_b0;
  float local_ac;
  float local_a8;
  float local_a4;
  float local_a0;
  float local_9c;
  Vector local_98;
  Vector local_8c;
  float local_80;
  float local_7c;
  float local_78;
  float local_74;
  float local_70;
  float local_6c;
  Vector local_68;
  Vector local_5c;
  Vector local_50;
  float local_44;
  float local_40;
  vec_t local_3c;
  float local_38;
  float local_34;
  vec_t local_30;
  float local_2c;
  float local_28;
  vec_t local_24;
  undefined4 local_20;
  
                    /* Unresolved local var: Vector vecForward@[???]
                       Unresolved local var: Vector vecRight@[???]
                       Unresolved local var: Vector vecUp@[???]
                       Unresolved local var: Vector vecForwardDir@[???]
                       Unresolved local var: Vector vecRightDir@[???]
                       Unresolved local var: Vector vecUpDir@[???]
                       Unresolved local var: Vector vecOrigin@[???]
                       Unresolved local var: float flDrawnLength@[???]
                       Unresolved local var: float flPrevSegmentLength@[???]
                       Unresolved local var: float flCurrentSegmentLength@[???]
                       Unresolved local var: Vector vecOriginalOrigin@[???]
                       Unresolved local var: Vector vecOriginalForward@[???] */
  ___i686_get_pc_thunk_bx();
  angles = C_BaseEntity::GetNetworkAngles((C_BaseEntity *)this);
  AngleVectors(angles,&local_50,&local_5c,&local_68);
  fVar9 = this->m_flWidth;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
  pfVar3 = (float *)(&DAT_004ae5ff + unaff_EBX);
  local_74 = fVar9 * local_5c.x * *pfVar3;
  local_70 = fVar9 * local_5c.y * *pfVar3;
  local_6c = fVar9 * local_5c.z * *pfVar3;
  fVar9 = this->m_flHeight;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
  local_80 = fVar9 * local_68.x * *pfVar3;
  local_7c = fVar9 * local_68.y * *pfVar3;
  local_78 = fVar9 * local_68.z * *pfVar3;
  local_44 = local_80;
  local_40 = local_7c;
  local_3c = local_78;
  local_38 = local_74;
  local_34 = local_70;
  local_30 = local_6c;
  pfVar3 = (float *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                      super_C_BaseProjectedEntity.super_C_BaseEntity.super_IClientEntity.
                      super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0xe5])(this);
  local_8c.x = *pfVar3;
  local_8c.y = pfVar3[1];
  local_8c.z = pfVar3[2];
  bVar6 = *(int *)(*(int *)(unaff_EBX + 0x83576b) + 0x30) != 0;
  if (bVar6) {
                    /* Unresolved local var: Vector vecWallEnd@[???] */
    local_9c = this->m_flCurDisplayLength;
                    /* Unresolved local var: Vector res@[???] */
    local_a4 = local_9c * local_50.x;
    local_a0 = local_9c * local_50.y;
    local_9c = local_9c * local_50.z;
                    /* Unresolved local var: Vector res@[???] */
    local_b0 = (char *)(local_8c.x + local_a4);
    local_ac = local_8c.y + local_a0;
    local_a8 = local_8c.z + local_9c;
    local_98.x = (vec_t)local_b0;
    local_98.y = local_ac;
    local_98.z = local_a8;
    NDebugOverlay::Sphere(&local_98,5.0,0,0xff,0,true,0.1);
  }
  fVar9 = this->m_flSegmentLength;
  local_af0 = this->m_flCurDisplayLength;
  if (local_af0 <= fVar9) {
    local_cc = *(float *)(&DAT_004ae5ff + unaff_EBX) * local_af0;
                    /* Unresolved local var: Vector res@[???] */
    local_d4 = local_cc * local_50.x;
    local_d0 = local_cc * local_50.y;
    local_cc = local_cc * local_50.z;
    local_d8 = *(float *)(&DAT_004ae5ff + unaff_EBX) * this->m_flCurDisplayLength;
                    /* Unresolved local var: Vector res@[???] */
    local_e0 = local_d8 * local_50.x;
    local_dc = local_d8 * local_50.y;
    local_d8 = local_d8 * local_50.z;
    local_2c = local_d4;
    local_28 = local_d0;
    local_24 = local_cc;
    fVar9 = local_e0;
    fVar8 = local_dc;
    fVar7 = local_d8;
  }
  else {
    local_b4 = *(float *)(&DAT_004ae5ff + unaff_EBX) * fVar9;
                    /* Unresolved local var: Vector res@[???] */
    local_bc = local_b4 * local_50.x;
    local_b8 = local_b4 * local_50.y;
    local_b4 = local_b4 * local_50.z;
    local_c0 = *(float *)(&DAT_004ae5ff + unaff_EBX) * this->m_flSegmentLength;
                    /* Unresolved local var: Vector res@[???] */
    local_c8 = local_c0 * local_50.x;
    local_c4 = local_c0 * local_50.y;
    local_c0 = local_c0 * local_50.z;
    local_af0 = fVar9;
    local_2c = local_bc;
    local_28 = local_b8;
    local_24 = local_b4;
    fVar9 = local_c8;
    fVar8 = local_c4;
    fVar7 = local_c0;
  }
  local_8c.z = local_8c.z + fVar7;
  local_8c.y = local_8c.y + fVar8;
  local_8c.x = local_8c.x + fVar9;
  if (0 < this->m_nNumSegments) {
    local_b08 = 0.0;
    local_b00 = 1;
    local_afc = -1;
    local_af8 = 4;
    local_af4 = -4;
    do {
      if (type == BASE_SEGMENT) {
        _memcpy(&local_34c,meshBuilder,0x1e8);
        vecOrigin.y = local_8c.y;
        vecOrigin.x = local_8c.x;
        vecOrigin.z = local_8c.z;
        vecUp.y = local_40;
        vecUp.x = local_44;
        vecUp.z = local_3c;
        vecForward.y = local_28;
        vecForward.x = local_2c;
        vecForward.z = local_24;
        vecRight.y = local_34;
        vecRight.x = local_38;
        vecRight.z = local_30;
        PaintWallWithPaintSegment
                  (this,pMesh,&local_34c,BASE_SEGMENT,vecOrigin,vecUp,vecForward,vecRight);
        if ((local_34c.m_VertexBuilder.m_pVertexBuffer != (IVertexBuffer *)0x0) &&
           (iVar4 = (*(local_34c.m_VertexBuilder.m_pVertexBuffer)->_vptr_IVertexBuffer[2])
                              (local_34c.m_VertexBuilder.m_pVertexBuffer), (char)iVar4 != '\0')) {
          (*(local_34c.m_VertexBuilder.m_pVertexBuffer)->_vptr_IVertexBuffer[4])
                    (local_34c.m_VertexBuilder.m_pVertexBuffer);
        }
        if ((local_34c.m_IndexBuilder.m_pIndexBuffer != (IIndexBuffer *)0x0) &&
           (iVar4 = (*(local_34c.m_IndexBuilder.m_pIndexBuffer)->_vptr_IIndexBuffer[2])
                              (local_34c.m_IndexBuilder.m_pIndexBuffer), (char)iVar4 != '\0')) {
          (*(local_34c.m_IndexBuilder.m_pIndexBuffer)->_vptr_IIndexBuffer[4])
                    (local_34c.m_IndexBuilder.m_pIndexBuffer);
        }
      }
      else {
        bVar2 = SetPaintSurfaceColor
                          (this,(PaintPowerType *)
                                (local_af8 + -4 + (int)(this->m_PaintPowers).m_Memory.m_pMemory));
        if (bVar2) {
          if (type == PAINT_SINGLE_SEGMENT) {
            if (local_afc == -1) {
              pPVar5 = (this->m_PaintPowers).m_Memory.m_pMemory;
            }
            else {
              pPVar5 = (this->m_PaintPowers).m_Memory.m_pMemory;
              if (*(int *)((int)pPVar5 + local_af4) != 4) goto LAB_00533f31;
            }
            if ((*(int *)(local_af8 + (int)pPVar5) == 4) || (this->m_nNumSegments <= (int)local_b00)
               ) {
              _memcpy(&local_534,meshBuilder,0x1e8);
              vecOrigin_01.y = local_8c.y;
              vecOrigin_01.x = local_8c.x;
              vecOrigin_01.z = local_8c.z;
              vecUp_01.y = local_40;
              vecUp_01.x = local_44;
              vecUp_01.z = local_3c;
              vecForward_01.y = local_28;
              vecForward_01.x = local_2c;
              vecForward_01.z = local_24;
              vecRight_01.y = local_34;
              vecRight_01.x = local_38;
              vecRight_01.z = local_30;
              PaintWallWithPaintSegment
                        (this,pMesh,&local_534,PAINT_SINGLE_SEGMENT,vecOrigin_01,vecUp_01,
                         vecForward_01,vecRight_01);
              if ((local_534.m_VertexBuilder.m_pVertexBuffer != (IVertexBuffer *)0x0) &&
                 (iVar4 = (*(local_534.m_VertexBuilder.m_pVertexBuffer)->_vptr_IVertexBuffer[2])
                                    (local_534.m_VertexBuilder.m_pVertexBuffer), (char)iVar4 != '\0'
                 )) {
                (*(local_534.m_VertexBuilder.m_pVertexBuffer)->_vptr_IVertexBuffer[4])
                          (local_534.m_VertexBuilder.m_pVertexBuffer);
              }
              if ((local_534.m_IndexBuilder.m_pIndexBuffer != (IIndexBuffer *)0x0) &&
                 (iVar4 = (*(local_534.m_IndexBuilder.m_pIndexBuffer)->_vptr_IIndexBuffer[2])
                                    (local_534.m_IndexBuilder.m_pIndexBuffer), (char)iVar4 != '\0'))
              {
                (*(local_534.m_IndexBuilder.m_pIndexBuffer)->_vptr_IIndexBuffer[4])
                          (local_534.m_IndexBuilder.m_pIndexBuffer);
              }
            }
          }
          else if (type == PAINT_END_1_SEGMENT) {
            if ((((local_afc == -1) ||
                 (*(int *)((int)(this->m_PaintPowers).m_Memory.m_pMemory + local_af4) == 4)) &&
                ((int)local_b00 < this->m_nNumSegments)) &&
               (*(int *)((int)(this->m_PaintPowers).m_Memory.m_pMemory + local_af8) != 4)) {
              _memcpy(&local_71c,meshBuilder,0x1e8);
              vecOrigin_02.y = local_8c.y;
              vecOrigin_02.x = local_8c.x;
              vecOrigin_02.z = local_8c.z;
              vecUp_02.y = local_40;
              vecUp_02.x = local_44;
              vecUp_02.z = local_3c;
              vecForward_02.y = local_28;
              vecForward_02.x = local_2c;
              vecForward_02.z = local_24;
              vecRight_02.y = local_34;
              vecRight_02.x = local_38;
              vecRight_02.z = local_30;
              PaintWallWithPaintSegment
                        (this,pMesh,&local_71c,PAINT_END_1_SEGMENT,vecOrigin_02,vecUp_02,
                         vecForward_02,vecRight_02);
              if ((local_71c.m_VertexBuilder.m_pVertexBuffer != (IVertexBuffer *)0x0) &&
                 (iVar4 = (*(local_71c.m_VertexBuilder.m_pVertexBuffer)->_vptr_IVertexBuffer[2])
                                    (local_71c.m_VertexBuilder.m_pVertexBuffer), (char)iVar4 != '\0'
                 )) {
                (*(local_71c.m_VertexBuilder.m_pVertexBuffer)->_vptr_IVertexBuffer[4])
                          (local_71c.m_VertexBuilder.m_pVertexBuffer);
              }
              if ((local_71c.m_IndexBuilder.m_pIndexBuffer != (IIndexBuffer *)0x0) &&
                 (iVar4 = (*(local_71c.m_IndexBuilder.m_pIndexBuffer)->_vptr_IIndexBuffer[2])
                                    (local_71c.m_IndexBuilder.m_pIndexBuffer), (char)iVar4 != '\0'))
              {
                (*(local_71c.m_IndexBuilder.m_pIndexBuffer)->_vptr_IIndexBuffer[4])
                          (local_71c.m_IndexBuilder.m_pIndexBuffer);
              }
            }
          }
          else if (type == PAINT_END_2_SEGMENT) {
            if (((local_afc != 0) || (*(this->m_PaintPowers).m_Memory.m_pMemory != NO_POWER)) &&
               ((this->m_nNumSegments <= (int)local_b00 ||
                (*(int *)((int)(this->m_PaintPowers).m_Memory.m_pMemory + local_af8) == 4)))) {
              _memcpy(&local_904,meshBuilder,0x1e8);
              vecOrigin_03.y = local_8c.y;
              vecOrigin_03.x = local_8c.x;
              vecOrigin_03.z = local_8c.z;
              vecUp_03.y = local_40;
              vecUp_03.x = local_44;
              vecUp_03.z = local_3c;
              vecForward_03.y = local_28;
              vecForward_03.x = local_2c;
              vecForward_03.z = local_24;
              vecRight_03.y = local_34;
              vecRight_03.x = local_38;
              vecRight_03.z = local_30;
              PaintWallWithPaintSegment
                        (this,pMesh,&local_904,PAINT_END_2_SEGMENT,vecOrigin_03,vecUp_03,
                         vecForward_03,vecRight_03);
              if ((local_904.m_VertexBuilder.m_pVertexBuffer != (IVertexBuffer *)0x0) &&
                 (iVar4 = (*(local_904.m_VertexBuilder.m_pVertexBuffer)->_vptr_IVertexBuffer[2])
                                    (local_904.m_VertexBuilder.m_pVertexBuffer), (char)iVar4 != '\0'
                 )) {
                (*(local_904.m_VertexBuilder.m_pVertexBuffer)->_vptr_IVertexBuffer[4])
                          (local_904.m_VertexBuilder.m_pVertexBuffer);
              }
              if ((local_904.m_IndexBuilder.m_pIndexBuffer != (IIndexBuffer *)0x0) &&
                 (iVar4 = (*(local_904.m_IndexBuilder.m_pIndexBuffer)->_vptr_IIndexBuffer[2])
                                    (local_904.m_IndexBuilder.m_pIndexBuffer), (char)iVar4 != '\0'))
              {
                (*(local_904.m_IndexBuilder.m_pIndexBuffer)->_vptr_IIndexBuffer[4])
                          (local_904.m_IndexBuilder.m_pIndexBuffer);
              }
            }
          }
          else if ((((type == PAINT_MIDDLE_SEGMENT) && (-1 < local_afc)) &&
                   ((int)local_b00 < this->m_nNumSegments)) &&
                  ((pPVar5 = (this->m_PaintPowers).m_Memory.m_pMemory,
                   *(int *)((int)pPVar5 + local_af4) != 4 &&
                   (*(int *)((int)pPVar5 + local_af8) != 4)))) {
            _memcpy(&local_aec,meshBuilder,0x1e8);
            vecOrigin_00.y = local_8c.y;
            vecOrigin_00.x = local_8c.x;
            vecOrigin_00.z = local_8c.z;
            vecUp_00.y = local_40;
            vecUp_00.x = local_44;
            vecUp_00.z = local_3c;
            vecForward_00.y = local_28;
            vecForward_00.x = local_2c;
            vecForward_00.z = local_24;
            vecRight_00.y = local_34;
            vecRight_00.x = local_38;
            vecRight_00.z = local_30;
            PaintWallWithPaintSegment
                      (this,pMesh,&local_aec,PAINT_MIDDLE_SEGMENT,vecOrigin_00,vecUp_00,
                       vecForward_00,vecRight_00);
            if ((local_aec.m_VertexBuilder.m_pVertexBuffer != (IVertexBuffer *)0x0) &&
               (iVar4 = (*(local_aec.m_VertexBuilder.m_pVertexBuffer)->_vptr_IVertexBuffer[2])
                                  (local_aec.m_VertexBuilder.m_pVertexBuffer), (char)iVar4 != '\0'))
            {
              (*(local_aec.m_VertexBuilder.m_pVertexBuffer)->_vptr_IVertexBuffer[4])
                        (local_aec.m_VertexBuilder.m_pVertexBuffer);
            }
            if ((local_aec.m_IndexBuilder.m_pIndexBuffer != (IIndexBuffer *)0x0) &&
               (iVar4 = (*(local_aec.m_IndexBuilder.m_pIndexBuffer)->_vptr_IIndexBuffer[2])
                                  (local_aec.m_IndexBuilder.m_pIndexBuffer), (char)iVar4 != '\0')) {
              (*(local_aec.m_IndexBuilder.m_pIndexBuffer)->_vptr_IIndexBuffer[4])
                        (local_aec.m_IndexBuilder.m_pIndexBuffer);
            }
          }
        }
      }
LAB_00533f31:
      if (bVar6) {
                    /* Unresolved local var: Color color@[???]
                       Unresolved local var: Vector vecMins@[???]
                       Unresolved local var: Vector vecMaxs@[???] */
        local_98.x = 2.3418409e-38;
        if ((local_b00 & 1) != 0) {
          local_20 = 0xffff00;
          local_98.x = 2.3509528e-38;
        }
        puVar1 = (uint *)(unaff_EBX + 0x4b365f);
        local_15c = (float)((uint)local_50.z ^ *puVar1);
        local_160 = (float)((uint)local_50.y ^ *puVar1);
        local_164 = (float)((uint)local_50.x ^ *puVar1);
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
        pfVar3 = (float *)(&DAT_004ae5ff + unaff_EBX);
        local_104 = local_af0 * local_164 * *pfVar3;
        local_100 = local_af0 * local_160 * *pfVar3;
        local_fc = local_af0 * local_15c * *pfVar3;
        fVar9 = this->m_flWidth;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
        local_134 = fVar9 * local_5c.x * *pfVar3;
        local_130 = fVar9 * local_5c.y * *pfVar3;
        local_12c = fVar9 * local_5c.z * *pfVar3;
                    /* Unresolved local var: Vector res@[???] */
        pfVar3 = (float *)(unaff_EBX + 0x4ae65b);
        local_140 = *pfVar3 * local_68.x;
        local_13c = *pfVar3 * local_68.y;
        local_138 = *pfVar3 * local_68.z;
        local_ec.x = (local_104 - local_134) - local_140;
        local_ec.y = (local_100 - local_130) - local_13c;
        local_ec.z = (local_fc - local_12c) - local_138;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
        pfVar3 = (float *)(&DAT_004ae5ff + unaff_EBX);
        local_128 = local_af0 * local_50.x * *pfVar3;
        local_124 = local_af0 * local_50.y * *pfVar3;
        local_120 = local_af0 * local_50.z * *pfVar3;
                    /* Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
                    /* Unresolved local var: Vector res@[???] */
        local_f8.x = local_128 + local_134 + local_140;
        local_f8.y = local_124 + local_130 + local_13c;
        local_f8.z = local_120 + local_12c + local_138;
        local_11c = local_140;
        local_118 = local_13c;
        local_114 = local_138;
        local_110 = local_134;
        local_10c = local_130;
        local_108 = local_12c;
        NDebugOverlay::Box(&local_8c,&local_ec,&local_f8,(uint)local_98.x & 0xff,
                           (uint)local_98.x >> 8 & 0xff,(uint)local_98.x >> 0x10,0x40,0.1);
        NDebugOverlay::Sphere(&local_8c,4.0,0xff,0,0,true,0.1);
      }
      local_b08 = local_b08 + local_af0;
      fVar9 = this->m_flSegmentLength;
      fVar8 = (local_b08 + fVar9) - this->m_flCurDisplayLength;
      if (0.0 < fVar8) {
        fVar9 = fVar9 - fVar8;
      }
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
      pfVar3 = (float *)(&DAT_004ae5ff + unaff_EBX);
      local_14c = fVar9 * local_50.x * *pfVar3;
      local_148 = fVar9 * local_50.y * *pfVar3;
      local_144 = fVar9 * local_50.z * *pfVar3;
      local_150 = *pfVar3 * local_af0 + *pfVar3 * fVar9;
                    /* Unresolved local var: Vector res@[???] */
      local_158 = local_150 * local_50.x;
      local_154 = local_150 * local_50.y;
      local_150 = local_150 * local_50.z;
      local_8c.x = local_8c.x + local_158;
      local_8c.y = local_8c.y + local_154;
      local_8c.z = local_8c.z + local_150;
      local_afc = local_afc + 1;
      local_af8 = local_af8 + 4;
      local_af4 = local_af4 + 4;
      bVar2 = (int)local_b00 < this->m_nNumSegments;
      local_b00 = local_b00 + 1;
      local_af0 = fVar9;
      local_2c = local_14c;
      local_28 = local_148;
      local_24 = local_144;
    } while (bVar2);
  }
  return;
}


/* C_ProjectedWallEntity::PaintWallWithPaintSegment at 00532760 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void PaintWallWithPaintSegment(C_ProjectedWallEntity * this, IMesh *
   pMesh, CMeshBuilder * meshBuilder, ProjectedWallSegmentType type, Vector vecOrigin, Vector vecUp,
   Vector vecForward, Vector vecRight) */

void __thiscall
C_ProjectedWallEntity::PaintWallWithPaintSegment
          (C_ProjectedWallEntity *this,IMesh *pMesh,CMeshBuilder *meshBuilder,
          ProjectedWallSegmentType type,Vector vecOrigin,Vector vecUp,Vector vecForward,
          Vector vecRight)

{
  float **ppfVar1;
  uchar **ppuVar2;
  uint uVar3;
  uint *puVar4;
  float *pfVar5;
  uchar *puVar6;
  MaterialPrimitiveType_t MVar7;
  IIndexBuffer *pIVar8;
  int iVar9;
  int nIndexCount;
  int iVar10;
  int unaff_EBX;
  float fVar11;
  float fVar12;
  float fVar13;
  float fVar14;
  float flLengthTexRate;
  vec_t local_64;
  
                    /* Unresolved local var: float flWidthTextRate@[???]
                       Unresolved local var: Vector vecNewOrigin@[???] */
  ___i686_get_pc_thunk_bx();
  fVar12 = this->m_flSegmentLength / *(float *)(*(int *)(unaff_EBX + 0x836b4b) + 0x2c);
  local_64 = vecOrigin.z;
  if (type != BASE_SEGMENT) {
    local_64 = vecOrigin.z + *(float *)(unaff_EBX + 0x4b0763);
  }
  meshBuilder->m_pMesh = pMesh;
  meshBuilder->m_bGenerateIndices = true;
  meshBuilder->m_Type = MATERIAL_QUADS;
  (*(pMesh->super_IVertexBuffer)._vptr_IVertexBuffer[10])(pMesh,2);
  (*(meshBuilder->m_pMesh->super_IVertexBuffer)._vptr_IVertexBuffer[0x11])
            (meshBuilder->m_pMesh,0x14,0x1e,meshBuilder,0);
  pIVar8 = (IIndexBuffer *)0x0;
  if (pMesh != (IMesh *)0x0) {
    pIVar8 = &pMesh->super_IIndexBuffer;
  }
  (meshBuilder->m_IndexBuilder).m_pIndexBuffer = pIVar8;
  (meshBuilder->m_IndexBuilder).m_nIndexCount = 0;
  (meshBuilder->m_IndexBuilder).m_nMaxIndexCount = 0x1e;
  (meshBuilder->m_IndexBuilder).m_bModify = false;
  (meshBuilder->m_IndexBuilder).m_nIndexOffset =
       (meshBuilder->super_MeshDesc_t).super_VertexDesc_t.m_nFirstVertex;
  (meshBuilder->m_IndexBuilder).super_IndexDesc_t.m_pIndices =
       (meshBuilder->super_MeshDesc_t).super_IndexDesc_t.m_pIndices;
  (meshBuilder->m_IndexBuilder).super_IndexDesc_t.m_nIndexSize =
       (meshBuilder->super_MeshDesc_t).super_IndexDesc_t.m_nIndexSize;
  (meshBuilder->m_IndexBuilder).m_nCurrentIndex = 0;
  (meshBuilder->m_VertexBuilder).m_pVertexBuffer = &pMesh->super_IVertexBuffer;
  _memcpy(&meshBuilder->m_VertexBuilder,meshBuilder,0xa4);
  (meshBuilder->m_VertexBuilder).m_nMaxVertexCount = 0x14;
  (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_NumBoneWeights =
       ~-(uint)((meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_NumBoneWeights == 0) & 2;
  (meshBuilder->m_VertexBuilder).m_nVertexCount = 0;
  (meshBuilder->m_VertexBuilder).m_bModify = false;
  if ((meshBuilder->m_VertexBuilder).m_nBufferOffset == 0xffffffff) {
    (meshBuilder->m_VertexBuilder).m_nTotalVertexCount = 0;
    (meshBuilder->m_VertexBuilder).m_nBufferOffset =
         (meshBuilder->super_MeshDesc_t).super_VertexDesc_t.m_nOffset;
    (meshBuilder->m_VertexBuilder).m_nBufferFirstVertex =
         (meshBuilder->super_MeshDesc_t).super_VertexDesc_t.m_nFirstVertex;
  }
  (meshBuilder->m_IndexBuilder).m_nCurrentIndex = 0;
  (meshBuilder->m_VertexBuilder).m_nCurrentVertex = 0;
  (meshBuilder->m_VertexBuilder).m_pCurrPosition =
       (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_pPosition;
  (meshBuilder->m_VertexBuilder).m_pCurrNormal =
       (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_pNormal;
                    /* Unresolved local var: int i@[???] */
  (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[0] =
       (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_pTexCoord[0];
  (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[1] =
       (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_pTexCoord[1];
  (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[2] =
       (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_pTexCoord[2];
  (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[3] =
       (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_pTexCoord[3];
  (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[4] =
       (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_pTexCoord[4];
  (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[5] =
       (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_pTexCoord[5];
  (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[6] =
       (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_pTexCoord[6];
  (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[7] =
       (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_pTexCoord[7];
  puVar4 = (uint *)(meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_pColor;
  (meshBuilder->m_VertexBuilder).m_pCurrColor = (uchar *)puVar4;
                    /* Unresolved local var: Vector res@[???] */
  fVar11 = vecUp.x + vecOrigin.x;
  fVar13 = vecUp.y + vecOrigin.y;
  fVar14 = vecUp.z + local_64;
                    /* Unresolved local var: Vector vert@[???]
                       Unresolved local var: float[4] flColors@[???] */
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
  uVar3 = *(uint *)(&DAT_004b458f + unaff_EBX);
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
  *puVar4 = (uVar3 & 0xff) << 8 | (uVar3 & 0xff) << 0x10 | uVar3 & 0xff | uVar3 << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
  pfVar5 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[0];
  *pfVar5 = 0.0;
  pfVar5[1] = fVar12;
                    /* Unresolved local var: float * pDst@[???] */
  pfVar5 = (meshBuilder->m_VertexBuilder).m_pCurrPosition;
  *pfVar5 = (fVar11 + vecRight.x) - vecForward.x;
  pfVar5[1] = (fVar13 + vecRight.y) - vecForward.y;
  pfVar5[2] = (fVar14 + vecRight.z) - vecForward.z;
  iVar9 = (meshBuilder->m_VertexBuilder).m_nCurrentVertex + 1;
  (meshBuilder->m_VertexBuilder).m_nCurrentVertex = iVar9;
  if ((meshBuilder->m_VertexBuilder).m_nVertexCount < iVar9) {
    (meshBuilder->m_VertexBuilder).m_nVertexCount = iVar9;
  }
  ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrPosition;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Position);
  ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrNormal;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Normal);
  ppuVar2 = &(meshBuilder->m_VertexBuilder).m_pCurrColor;
  *ppuVar2 = *ppuVar2 + (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Color;
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[0]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 1;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[1]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 2;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[2]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 3;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[3]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 4;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[4]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 5;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[5]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 6;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[6]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 7;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[7]);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
  *(uint *)(meshBuilder->m_VertexBuilder).m_pCurrColor =
       (uVar3 & 0xff) << 8 | (uVar3 & 0xff) << 0x10 | uVar3 & 0xff | uVar3 << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
  pfVar5 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[0];
  *pfVar5 = 0.0;
  pfVar5[1] = 0.0;
                    /* Unresolved local var: float * pDst@[???] */
  pfVar5 = (meshBuilder->m_VertexBuilder).m_pCurrPosition;
  *pfVar5 = vecRight.x + fVar11 + vecForward.x;
  pfVar5[1] = vecRight.y + fVar13 + vecForward.y;
  pfVar5[2] = vecRight.z + fVar14 + vecForward.z;
  iVar9 = (meshBuilder->m_VertexBuilder).m_nCurrentVertex + 1;
  (meshBuilder->m_VertexBuilder).m_nCurrentVertex = iVar9;
  if ((meshBuilder->m_VertexBuilder).m_nVertexCount < iVar9) {
    (meshBuilder->m_VertexBuilder).m_nVertexCount = iVar9;
  }
  ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrPosition;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Position);
  ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrNormal;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Normal);
  ppuVar2 = &(meshBuilder->m_VertexBuilder).m_pCurrColor;
  *ppuVar2 = *ppuVar2 + (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Color;
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[0]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 1;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[1]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 2;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[2]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 3;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[3]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 4;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[4]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 5;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[5]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 6;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[6]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 7;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[7]);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
  *(uint *)(meshBuilder->m_VertexBuilder).m_pCurrColor =
       (uVar3 & 0xff) << 8 | (uVar3 & 0xff) << 0x10 | uVar3 & 0xff | uVar3 << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
  pfVar5 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[0];
  *pfVar5 = 1.0;
  pfVar5[1] = 0.0;
                    /* Unresolved local var: float * pDst@[???] */
  pfVar5 = (meshBuilder->m_VertexBuilder).m_pCurrPosition;
  *pfVar5 = (fVar11 - vecRight.x) + vecForward.x;
  pfVar5[1] = (fVar13 - vecRight.y) + vecForward.y;
  pfVar5[2] = (fVar14 - vecRight.z) + vecForward.z;
  iVar9 = (meshBuilder->m_VertexBuilder).m_nCurrentVertex + 1;
  (meshBuilder->m_VertexBuilder).m_nCurrentVertex = iVar9;
  if ((meshBuilder->m_VertexBuilder).m_nVertexCount < iVar9) {
    (meshBuilder->m_VertexBuilder).m_nVertexCount = iVar9;
  }
  ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrPosition;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Position);
  ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrNormal;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Normal);
  ppuVar2 = &(meshBuilder->m_VertexBuilder).m_pCurrColor;
  *ppuVar2 = *ppuVar2 + (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Color;
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[0]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 1;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[1]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 2;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[2]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 3;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[3]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 4;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[4]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 5;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[5]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 6;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[6]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 7;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[7]);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
  *(uint *)(meshBuilder->m_VertexBuilder).m_pCurrColor =
       (uVar3 & 0xff) << 8 | (uVar3 & 0xff) << 0x10 | uVar3 & 0xff | uVar3 << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
  pfVar5 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[0];
  *pfVar5 = 1.0;
  pfVar5[1] = fVar12;
                    /* Unresolved local var: float * pDst@[???] */
  pfVar5 = (meshBuilder->m_VertexBuilder).m_pCurrPosition;
  *pfVar5 = (fVar11 - vecRight.x) - vecForward.x;
  pfVar5[1] = (fVar13 - vecRight.y) - vecForward.y;
  pfVar5[2] = (fVar14 - vecRight.z) - vecForward.z;
  iVar9 = (meshBuilder->m_VertexBuilder).m_nCurrentVertex + 1;
  (meshBuilder->m_VertexBuilder).m_nCurrentVertex = iVar9;
  if ((meshBuilder->m_VertexBuilder).m_nVertexCount < iVar9) {
    (meshBuilder->m_VertexBuilder).m_nVertexCount = iVar9;
  }
  ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrPosition;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Position);
  ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrNormal;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Normal);
  ppuVar2 = &(meshBuilder->m_VertexBuilder).m_pCurrColor;
  *ppuVar2 = *ppuVar2 + (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Color;
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[0]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 1;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[1]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 2;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[2]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 3;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[3]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 4;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[4]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 5;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[5]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 6;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[6]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 7;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[7]);
                    /* Unresolved local var: Vector res@[???] */
  fVar11 = vecRight.x + vecOrigin.x;
  fVar13 = vecRight.y + vecOrigin.y;
  fVar14 = vecRight.z + local_64;
                    /* Unresolved local var: Vector vert@[???]
                       Unresolved local var: float[4] flColors@[???] */
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
  *(uint *)(meshBuilder->m_VertexBuilder).m_pCurrColor =
       (uVar3 & 0xff) << 8 | (uVar3 & 0xff) << 0x10 | uVar3 & 0xff | uVar3 << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
  pfVar5 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[0];
  *pfVar5 = 0.0;
  pfVar5[1] = fVar12;
                    /* Unresolved local var: float * pDst@[???] */
  pfVar5 = (meshBuilder->m_VertexBuilder).m_pCurrPosition;
  *pfVar5 = (fVar11 + vecUp.x) - vecForward.x;
  pfVar5[1] = (fVar13 + vecUp.y) - vecForward.y;
  pfVar5[2] = (fVar14 + vecUp.z) - vecForward.z;
  iVar9 = (meshBuilder->m_VertexBuilder).m_nCurrentVertex + 1;
  (meshBuilder->m_VertexBuilder).m_nCurrentVertex = iVar9;
  if ((meshBuilder->m_VertexBuilder).m_nVertexCount < iVar9) {
    (meshBuilder->m_VertexBuilder).m_nVertexCount = iVar9;
  }
  ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrPosition;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Position);
  ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrNormal;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Normal);
  ppuVar2 = &(meshBuilder->m_VertexBuilder).m_pCurrColor;
  *ppuVar2 = *ppuVar2 + (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Color;
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[0]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 1;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[1]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 2;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[2]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 3;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[3]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 4;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[4]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 5;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[5]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 6;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[6]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 7;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[7]);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
  *(uint *)(meshBuilder->m_VertexBuilder).m_pCurrColor =
       (uVar3 & 0xff) << 8 | (uVar3 & 0xff) << 0x10 | uVar3 & 0xff | uVar3 << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
  pfVar5 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[0];
  *pfVar5 = 0.0;
  pfVar5[1] = 0.0;
                    /* Unresolved local var: float * pDst@[???] */
  pfVar5 = (meshBuilder->m_VertexBuilder).m_pCurrPosition;
  *pfVar5 = vecUp.x + fVar11 + vecForward.x;
  pfVar5[1] = vecUp.y + fVar13 + vecForward.y;
  pfVar5[2] = vecUp.z + fVar14 + vecForward.z;
  iVar9 = (meshBuilder->m_VertexBuilder).m_nCurrentVertex + 1;
  (meshBuilder->m_VertexBuilder).m_nCurrentVertex = iVar9;
  if ((meshBuilder->m_VertexBuilder).m_nVertexCount < iVar9) {
    (meshBuilder->m_VertexBuilder).m_nVertexCount = iVar9;
  }
  ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrPosition;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Position);
  ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrNormal;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Normal);
  ppuVar2 = &(meshBuilder->m_VertexBuilder).m_pCurrColor;
  *ppuVar2 = *ppuVar2 + (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Color;
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[0]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 1;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[1]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 2;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[2]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 3;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[3]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 4;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[4]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 5;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[5]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 6;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[6]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 7;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[7]);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
  *(uint *)(meshBuilder->m_VertexBuilder).m_pCurrColor =
       (uVar3 & 0xff) << 8 | (uVar3 & 0xff) << 0x10 | uVar3 & 0xff | uVar3 << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
  pfVar5 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[0];
  *pfVar5 = 1.0;
  pfVar5[1] = 0.0;
                    /* Unresolved local var: float * pDst@[???] */
  pfVar5 = (meshBuilder->m_VertexBuilder).m_pCurrPosition;
  *pfVar5 = (fVar11 - vecUp.x) + vecForward.x;
  pfVar5[1] = (fVar13 - vecUp.y) + vecForward.y;
  pfVar5[2] = (fVar14 - vecUp.z) + vecForward.z;
  iVar9 = (meshBuilder->m_VertexBuilder).m_nCurrentVertex + 1;
  (meshBuilder->m_VertexBuilder).m_nCurrentVertex = iVar9;
  if ((meshBuilder->m_VertexBuilder).m_nVertexCount < iVar9) {
    (meshBuilder->m_VertexBuilder).m_nVertexCount = iVar9;
  }
  ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrPosition;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Position);
  ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrNormal;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Normal);
  ppuVar2 = &(meshBuilder->m_VertexBuilder).m_pCurrColor;
  *ppuVar2 = *ppuVar2 + (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Color;
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[0]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 1;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[1]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 2;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[2]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 3;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[3]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 4;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[4]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 5;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[5]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 6;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[6]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 7;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[7]);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
  *(uint *)(meshBuilder->m_VertexBuilder).m_pCurrColor =
       (uVar3 & 0xff) << 8 | (uVar3 & 0xff) << 0x10 | uVar3 & 0xff | uVar3 << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
  pfVar5 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[0];
  *pfVar5 = 1.0;
  pfVar5[1] = fVar12;
                    /* Unresolved local var: float * pDst@[???] */
  pfVar5 = (meshBuilder->m_VertexBuilder).m_pCurrPosition;
  *pfVar5 = (fVar11 - vecUp.x) - vecForward.x;
  pfVar5[1] = (fVar13 - vecUp.y) - vecForward.y;
  pfVar5[2] = (fVar14 - vecUp.z) - vecForward.z;
  iVar9 = (meshBuilder->m_VertexBuilder).m_nCurrentVertex + 1;
  (meshBuilder->m_VertexBuilder).m_nCurrentVertex = iVar9;
  if ((meshBuilder->m_VertexBuilder).m_nVertexCount < iVar9) {
    (meshBuilder->m_VertexBuilder).m_nVertexCount = iVar9;
  }
  ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrPosition;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Position);
  ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrNormal;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Normal);
  ppuVar2 = &(meshBuilder->m_VertexBuilder).m_pCurrColor;
  *ppuVar2 = *ppuVar2 + (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Color;
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[0]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 1;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[1]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 2;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[2]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 3;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[3]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 4;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[4]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 5;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[5]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 6;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[6]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 7;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[7]);
                    /* Unresolved local var: Vector res@[???] */
  fVar11 = vecOrigin.x - vecRight.x;
  fVar13 = vecOrigin.y - vecRight.y;
  local_64 = local_64 - vecRight.z;
                    /* Unresolved local var: Vector vert@[???]
                       Unresolved local var: float[4] flColors@[???] */
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
  *(uint *)(meshBuilder->m_VertexBuilder).m_pCurrColor =
       (uVar3 & 0xff) << 8 | (uVar3 & 0xff) << 0x10 | uVar3 & 0xff | uVar3 << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
  pfVar5 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[0];
  *pfVar5 = 0.0;
  pfVar5[1] = fVar12;
                    /* Unresolved local var: float * pDst@[???] */
  pfVar5 = (meshBuilder->m_VertexBuilder).m_pCurrPosition;
  *pfVar5 = (fVar11 + vecUp.x) - vecForward.x;
  pfVar5[1] = (fVar13 + vecUp.y) - vecForward.y;
  pfVar5[2] = (local_64 + vecUp.z) - vecForward.z;
  iVar9 = (meshBuilder->m_VertexBuilder).m_nCurrentVertex + 1;
  (meshBuilder->m_VertexBuilder).m_nCurrentVertex = iVar9;
  if ((meshBuilder->m_VertexBuilder).m_nVertexCount < iVar9) {
    (meshBuilder->m_VertexBuilder).m_nVertexCount = iVar9;
  }
  ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrPosition;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Position);
  ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrNormal;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Normal);
  ppuVar2 = &(meshBuilder->m_VertexBuilder).m_pCurrColor;
  *ppuVar2 = *ppuVar2 + (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Color;
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[0]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 1;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[1]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 2;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[2]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 3;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[3]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 4;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[4]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 5;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[5]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 6;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[6]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 7;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[7]);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
  *(uint *)(meshBuilder->m_VertexBuilder).m_pCurrColor =
       (uVar3 & 0xff) << 8 | (uVar3 & 0xff) << 0x10 | uVar3 & 0xff | uVar3 << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
  pfVar5 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[0];
  *pfVar5 = 0.0;
  pfVar5[1] = 0.0;
                    /* Unresolved local var: float * pDst@[???] */
  pfVar5 = (meshBuilder->m_VertexBuilder).m_pCurrPosition;
  *pfVar5 = vecUp.x + fVar11 + vecForward.x;
  pfVar5[1] = vecUp.y + fVar13 + vecForward.y;
  pfVar5[2] = vecUp.z + local_64 + vecForward.z;
  iVar9 = (meshBuilder->m_VertexBuilder).m_nCurrentVertex + 1;
  (meshBuilder->m_VertexBuilder).m_nCurrentVertex = iVar9;
  if ((meshBuilder->m_VertexBuilder).m_nVertexCount < iVar9) {
    (meshBuilder->m_VertexBuilder).m_nVertexCount = iVar9;
  }
  ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrPosition;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Position);
  ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrNormal;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Normal);
  ppuVar2 = &(meshBuilder->m_VertexBuilder).m_pCurrColor;
  *ppuVar2 = *ppuVar2 + (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Color;
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[0]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 1;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[1]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 2;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[2]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 3;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[3]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 4;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[4]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 5;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[5]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 6;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[6]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 7;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[7]);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
  puVar6 = (meshBuilder->m_VertexBuilder).m_pCurrColor;
  puVar6[0] = 0xff;
  puVar6[1] = 0xff;
  puVar6[2] = 0xff;
  puVar6[3] = 0xff;
                    /* Unresolved local var: float * pDst@[???] */
  pfVar5 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[0];
  *pfVar5 = 1.0;
  pfVar5[1] = 0.0;
                    /* Unresolved local var: float * pDst@[???] */
  pfVar5 = (meshBuilder->m_VertexBuilder).m_pCurrPosition;
  *pfVar5 = (fVar11 - vecUp.x) + vecForward.x;
  pfVar5[1] = (fVar13 - vecUp.y) + vecForward.y;
  pfVar5[2] = (local_64 - vecUp.z) + vecForward.z;
  iVar9 = (meshBuilder->m_VertexBuilder).m_nCurrentVertex + 1;
  (meshBuilder->m_VertexBuilder).m_nCurrentVertex = iVar9;
  if ((meshBuilder->m_VertexBuilder).m_nVertexCount < iVar9) {
    (meshBuilder->m_VertexBuilder).m_nVertexCount = iVar9;
  }
  ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrPosition;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Position);
  ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrNormal;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Normal);
  ppuVar2 = &(meshBuilder->m_VertexBuilder).m_pCurrColor;
  *ppuVar2 = *ppuVar2 + (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Color;
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[0]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 1;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[1]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 2;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[2]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 3;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[3]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 4;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[4]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 5;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[5]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 6;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[6]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 7;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[7]);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
  puVar6 = (meshBuilder->m_VertexBuilder).m_pCurrColor;
  puVar6[0] = 0xff;
  puVar6[1] = 0xff;
  puVar6[2] = 0xff;
  puVar6[3] = 0xff;
                    /* Unresolved local var: float * pDst@[???] */
  pfVar5 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord[0];
  *pfVar5 = 1.0;
  pfVar5[1] = fVar12;
                    /* Unresolved local var: float * pDst@[???] */
  pfVar5 = (meshBuilder->m_VertexBuilder).m_pCurrPosition;
  *pfVar5 = (fVar11 - vecUp.x) - vecForward.x;
  pfVar5[1] = (fVar13 - vecUp.y) - vecForward.y;
  pfVar5[2] = (local_64 - vecUp.z) - vecForward.z;
  iVar9 = (meshBuilder->m_VertexBuilder).m_nCurrentVertex + 1;
  (meshBuilder->m_VertexBuilder).m_nCurrentVertex = iVar9;
  if ((meshBuilder->m_VertexBuilder).m_nVertexCount < iVar9) {
    (meshBuilder->m_VertexBuilder).m_nVertexCount = iVar9;
  }
  ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrPosition;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Position);
  ppfVar1 = &(meshBuilder->m_VertexBuilder).m_pCurrNormal;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Normal);
  ppuVar2 = &(meshBuilder->m_VertexBuilder).m_pCurrColor;
  *ppuVar2 = *ppuVar2 + (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_Color;
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[0]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 1;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[1]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 2;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[2]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 3;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[3]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 4;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[4]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 5;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[5]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 6;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[6]);
  ppfVar1 = (meshBuilder->m_VertexBuilder).m_pCurrTexCoord + 7;
  *ppfVar1 = (float *)((int)*ppfVar1 +
                      (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_VertexSize_TexCoord[7]);
  if (meshBuilder->m_bGenerateIndices == false) {
    iVar9 = (meshBuilder->m_VertexBuilder).m_nVertexCount;
  }
  else {
    iVar9 = (meshBuilder->m_VertexBuilder).m_nVertexCount;
    MVar7 = meshBuilder->m_Type;
    if (MVar7 - MATERIAL_LINE_STRIP < 7) {
                    /* WARNING: Could not recover jumptable at 0x00533a75. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (*(code *)(*(int *)(unaff_EBX + 0x1307 + (MVar7 - MATERIAL_LINE_STRIP) * 4) + unaff_EBX))();
      return;
    }
    if ((meshBuilder->m_IndexBuilder).super_IndexDesc_t.m_nIndexSize != 0) {
      iVar10 = (meshBuilder->m_IndexBuilder).m_nCurrentIndex;
      nIndexCount = (meshBuilder->m_IndexBuilder).m_nMaxIndexCount - iVar10;
      if (iVar9 <= nIndexCount) {
        nIndexCount = iVar9;
      }
      if (nIndexCount != 0) {
        if (MVar7 < MATERIAL_HETEROGENOUS) {
                    /* WARNING: Could not recover jumptable at 0x00533ad3. Too many branches */
                    /* WARNING: Treating indirect jump as call */
          (*(code *)(*(int *)(unaff_EBX + 0x1367 + MVar7 * 4) + unaff_EBX))();
          return;
        }
        GenerateSequentialIndexBuffer
                  ((meshBuilder->m_IndexBuilder).super_IndexDesc_t.m_pIndices + iVar10,nIndexCount,
                   (meshBuilder->m_IndexBuilder).m_nIndexOffset);
        iVar9 = (meshBuilder->m_VertexBuilder).m_nVertexCount;
        iVar10 = nIndexCount * (meshBuilder->m_IndexBuilder).super_IndexDesc_t.m_nIndexSize +
                 (meshBuilder->m_IndexBuilder).m_nCurrentIndex;
        (meshBuilder->m_IndexBuilder).m_nCurrentIndex = iVar10;
        if ((meshBuilder->m_IndexBuilder).m_nIndexCount < iVar10) {
          (meshBuilder->m_IndexBuilder).m_nIndexCount = iVar10;
        }
      }
    }
  }
  (*(meshBuilder->m_pMesh->super_IVertexBuffer)._vptr_IVertexBuffer[0x14])
            (meshBuilder->m_pMesh,iVar9,(meshBuilder->m_IndexBuilder).m_nIndexCount,meshBuilder);
  (meshBuilder->m_IndexBuilder).m_pIndexBuffer = (IIndexBuffer *)0x0;
  (meshBuilder->m_IndexBuilder).m_nMaxIndexCount = 0;
  (meshBuilder->m_VertexBuilder).m_nMaxVertexCount = 0;
  (meshBuilder->m_VertexBuilder).m_pVertexBuffer = (IVertexBuffer *)0x0;
  (meshBuilder->m_VertexBuilder).super_VertexDesc_t.m_CompressionType = VERTEX_COMPRESSION_INVALID;
  meshBuilder->m_pMesh = (IMesh *)0x0;
  (*(pMesh->super_IVertexBuffer)._vptr_IVertexBuffer[0xb])(pMesh,0xffffffff,0);
  return;
}


/* C_ProjectedWallEntity::ColorWallByPortal at 00530830 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void ColorWallByPortal(C_ProjectedWallEntity * this, IMaterial * pBody)
    */

void __thiscall
C_ProjectedWallEntity::ColorWallByPortal(C_ProjectedWallEntity *this,IMaterial *pBody)

{
  uint uVar1;
  int *piVar2;
  int iVar3;
  Color *pCVar4;
  int unaff_EBX;
  int in_stack_ffffffcc;
  float local_28;
  float local_24;
  float local_20;
  
                    /* Unresolved local var: float[3] flColors@[???] */
  ___i686_get_pc_thunk_bx();
  local_28 = 1.0;
  local_24 = 1.0;
  local_20 = 1.0;
  uVar1 = (this->m_hColorPortal).super_CBaseHandle.m_Index;
                    /* Unresolved local var: C_Prop_Portal * pPortal@[???]
                       Unresolved local var: CBaseEntityList * g_pEntityList@[???]
                       Unresolved local var: CEntInfo * pInfo@[???] */
  if ((((uVar1 != 0xffffffff) &&
       (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x63359e),
       *(uint *)(iVar3 + 8) == uVar1 >> 0x10)) &&
      (piVar2 = *(int **)(iVar3 + 4), piVar2 != (int *)0x0)) &&
     (((uVar1 = piVar2[0xcf7], uVar1 != 0xffffffff &&
       (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x63359e),
       *(uint *)(iVar3 + 8) == uVar1 >> 0x10)) && (*(int *)(iVar3 + 4) != 0)))) {
                    /* Unresolved local var: Color clrPortal@[???] */
    iVar3 = (**(code **)(*piVar2 + 0x1e4))(piVar2);
    pCVar4 = UTIL_Portal_Color((Color *)(2 - (uint)((char)piVar2[0x3f7] == '\0')),iVar3,
                               in_stack_ffffffcc);
    local_20 = *(float *)(unaff_EBX + 0x4b1d02);
    local_28 = (float)((uint)pCVar4 & 0xff) / local_20;
    local_24 = (float)((uint)pCVar4 >> 8 & 0xff) / local_20;
    local_20 = (float)((uint)pCVar4 >> 0x10 & 0xff) / local_20;
  }
  (*this->m_pPaintColorMid->_vptr_IMaterialVar[9])(this->m_pPaintColorMid,&local_28,3);
  (*this->m_pPaintColorEnd1->_vptr_IMaterialVar[9])(this->m_pPaintColorEnd1,&local_28,3);
  (*this->m_pPaintColorEnd2->_vptr_IMaterialVar[9])(this->m_pPaintColorEnd2,&local_28,3);
  (*this->m_pPaintColorSing->_vptr_IMaterialVar[9])(this->m_pPaintColorSing,&local_28,3);
  return;
}


/* C_ProjectedWallEntity::InitMaterials at 0052ef00 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: bool InitMaterials(C_ProjectedWallEntity * this) */

bool __thiscall C_ProjectedWallEntity::InitMaterials(C_ProjectedWallEntity *this)

{
  IMaterialVar *pIVar1;
  IMaterial *pIVar2;
  int unaff_EBX;
  char local_d;
  
                    /* Unresolved local var: bool bFound@[???] */
  ___i686_get_pc_thunk_bx();
  if (this->m_pBodyMaterial == (IMaterial *)0x0) {
    pIVar2 = (IMaterial *)
             (**(code **)(*(int *)**(undefined4 **)(&DAT_00634fff + unaff_EBX) + 0x130))
                       ((int *)**(undefined4 **)(&DAT_00634fff + unaff_EBX),unaff_EBX + 0x48bf18,0,0
                        ,0);
    this->m_pBodyMaterial = pIVar2;
    if (pIVar2 == (IMaterial *)0x0) {
      return false;
    }
  }
  if (this->m_pPaintMaterialMid == (IMaterial *)0x0) {
    pIVar2 = (IMaterial *)
             (**(code **)(*(int *)**(undefined4 **)(&DAT_00634fff + unaff_EBX) + 0x130))
                       ((int *)**(undefined4 **)(&DAT_00634fff + unaff_EBX),unaff_EBX + 0x48c008,0,0
                        ,0);
    this->m_pPaintMaterialMid = pIVar2;
    if (pIVar2 == (IMaterial *)0x0) {
      return false;
    }
  }
  if (this->m_pPaintMaterialEnd1 == (IMaterial *)0x0) {
    pIVar2 = (IMaterial *)
             (**(code **)(*(int *)**(undefined4 **)(&DAT_00634fff + unaff_EBX) + 0x130))
                       ((int *)**(undefined4 **)(&DAT_00634fff + unaff_EBX),unaff_EBX + 0x48bfcf,0,0
                        ,0);
    this->m_pPaintMaterialEnd1 = pIVar2;
    if (pIVar2 == (IMaterial *)0x0) {
      return false;
    }
  }
  if (this->m_pPaintMaterialEnd2 == (IMaterial *)0x0) {
    pIVar2 = (IMaterial *)
             (**(code **)(*(int *)**(undefined4 **)(&DAT_00634fff + unaff_EBX) + 0x130))
                       ((int *)**(undefined4 **)(&DAT_00634fff + unaff_EBX),unaff_EBX + 0x48bfec,0,0
                        ,0);
    this->m_pPaintMaterialEnd2 = pIVar2;
    if (pIVar2 == (IMaterial *)0x0) {
      return false;
    }
  }
  if (this->m_pPaintMaterialSing == (IMaterial *)0x0) {
    pIVar2 = (IMaterial *)
             (**(code **)(*(int *)**(undefined4 **)(&DAT_00634fff + unaff_EBX) + 0x130))
                       ((int *)**(undefined4 **)(&DAT_00634fff + unaff_EBX),unaff_EBX + 0x48c020,0,0
                        ,0);
    this->m_pPaintMaterialSing = pIVar2;
    if (pIVar2 == (IMaterial *)0x0) {
      return false;
    }
  }
  if (this->m_pSideRailMaterial == (IMaterial *)0x0) {
    pIVar2 = (IMaterial *)
             (**(code **)(*(int *)**(undefined4 **)(&DAT_00634fff + unaff_EBX) + 0x130))
                       ((int *)**(undefined4 **)(&DAT_00634fff + unaff_EBX),unaff_EBX + 0x48bfb3,0,0
                        ,0);
    this->m_pSideRailMaterial = pIVar2;
    if (pIVar2 == (IMaterial *)0x0) {
      return false;
    }
  }
  local_d = '\0';
  if (this->m_pPaintColorMid == (IMaterialVar *)0x0) {
    pIVar1 = (IMaterialVar *)
             (*this->m_pPaintMaterialMid->_vptr_IMaterial[0xb])
                       (this->m_pPaintMaterialMid,&UNK_0048750b + unaff_EBX,&local_d,1);
    this->m_pPaintColorMid = pIVar1;
    if (local_d == '\0') {
      return false;
    }
  }
  local_d = '\0';
  if (this->m_pPaintColorEnd1 == (IMaterialVar *)0x0) {
    pIVar1 = (IMaterialVar *)
             (*this->m_pPaintMaterialEnd1->_vptr_IMaterial[0xb])
                       (this->m_pPaintMaterialEnd1,&UNK_0048750b + unaff_EBX,&local_d,1);
    this->m_pPaintColorEnd1 = pIVar1;
    if (local_d == '\0') {
      return false;
    }
  }
  local_d = '\0';
  if (this->m_pPaintColorEnd2 == (IMaterialVar *)0x0) {
    pIVar1 = (IMaterialVar *)
             (*this->m_pPaintMaterialEnd2->_vptr_IMaterial[0xb])
                       (this->m_pPaintMaterialEnd2,&UNK_0048750b + unaff_EBX,&local_d,1);
    this->m_pPaintColorEnd2 = pIVar1;
    if (local_d == '\0') {
      return false;
    }
  }
  local_d = '\0';
  if (this->m_pPaintColorSing != (IMaterialVar *)0x0) {
    return true;
  }
  pIVar1 = (IMaterialVar *)
           (*this->m_pPaintMaterialSing->_vptr_IMaterial[0xb])
                     (this->m_pPaintMaterialSing,&UNK_0048750b + unaff_EBX,&local_d,1);
  this->m_pPaintColorSing = pIVar1;
  return local_d != '\0';
}


/* C_ProjectedWallEntity::GetRenderBounds at 00530290 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void GetRenderBounds(C_ProjectedWallEntity * this, Vector * vecMins,
   Vector * vecMaxs) */

void __thiscall
C_ProjectedWallEntity::GetRenderBounds(C_ProjectedWallEntity *this,Vector *vecMins,Vector *vecMaxs)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float *pfVar5;
  
  pfVar5 = (float *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                      super_C_BaseProjectedEntity.super_C_BaseEntity.super_IClientEntity.
                      super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0x37])(this);
                    /* Unresolved local var: Vector res@[???] */
  fVar1 = (this->m_vWorldSpace_WallMins).y;
  fVar2 = pfVar5[1];
  fVar3 = (this->m_vWorldSpace_WallMins).z;
  fVar4 = pfVar5[2];
  vecMins->x = (this->m_vWorldSpace_WallMins).x - *pfVar5;
  vecMins->y = fVar1 - fVar2;
  vecMins->z = fVar3 - fVar4;
  pfVar5 = (float *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                      super_C_BaseProjectedEntity.super_C_BaseEntity.super_IClientEntity.
                      super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0x37])(this);
                    /* Unresolved local var: Vector res@[???] */
  fVar1 = (this->m_vWorldSpace_WallMaxs).y;
  fVar2 = pfVar5[1];
  fVar3 = (this->m_vWorldSpace_WallMaxs).z;
  fVar4 = pfVar5[2];
  vecMaxs->x = (this->m_vWorldSpace_WallMaxs).x - *pfVar5;
  vecMaxs->y = fVar1 - fVar2;
  vecMaxs->z = fVar3 - fVar4;
  return;
}


/* C_ProjectedWallEntity::GetRenderAngles at 0052f270 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: QAngle * GetRenderAngles(C_ProjectedWallEntity * this) */

QAngle * __thiscall C_ProjectedWallEntity::GetRenderAngles(C_ProjectedWallEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(QAngle **)(extraout_ECX + 0x634ae4);
}


/* C_ProjectedWallEntity::ComputeTranslucencyType at 0052f280 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: RenderableTranslucencyType_t
   ComputeTranslucencyType(C_ProjectedWallEntity * this) */

RenderableTranslucencyType_t __thiscall
C_ProjectedWallEntity::ComputeTranslucencyType(C_ProjectedWallEntity *this)

{
  return RENDERABLE_IS_TRANSLUCENT;
}


/* C_ProjectedWallEntity::OnPreDataChanged at 0052f340 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void OnPreDataChanged(C_ProjectedWallEntity * this, DataUpdateType_t
   type) */

void __thiscall
C_ProjectedWallEntity::OnPreDataChanged(C_ProjectedWallEntity *this,DataUpdateType_t type)

{
  C_BaseEntity::OnPreDataChanged((C_BaseEntity *)this,type);
  this->m_flPrevParticleUpdateTime = this->m_flParticleUpdateTime;
  return;
}


/* C_ProjectedWallEntity::OnDataChanged at 00530530 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void OnDataChanged(C_ProjectedWallEntity * this, DataUpdateType_t
   updateType) */

void __thiscall
C_ProjectedWallEntity::OnDataChanged(C_ProjectedWallEntity *this,DataUpdateType_t updateType)

{
  NetworkVar_m_Collision *this_00;
  
  C_BaseProjectedEntity::OnDataChanged((C_BaseProjectedEntity *)this,updateType);
  if (updateType == DATA_UPDATE_CREATED) {
    (*(this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
      super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
      _vptr_IHandleEntity[0xf6])(this);
    SetupWallParticles(this);
  }
  if (this->m_flParticleUpdateTime != this->m_flPrevParticleUpdateTime) {
    this->m_flPrevParticleUpdateTime = this->m_flParticleUpdateTime;
    SetupWallParticles(this);
  }
  this_00 = &(this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
             super_C_BaseEntity.m_Collision;
  CCollisionProperty::MarkSurroundingBoundsDirty(&this_00->super_CCollisionProperty);
  CCollisionProperty::MarkPartitionHandleDirty(&this_00->super_CCollisionProperty);
  return;
}


/* C_ProjectedWallEntity::OnProjected at 005322d0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void OnProjected(C_ProjectedWallEntity * this) */

void __thiscall C_ProjectedWallEntity::OnProjected(C_ProjectedWallEntity *this)

{
  C_BaseProjectedEntity::OnProjected((C_BaseProjectedEntity *)this);
  ProjectWall(this);
  SetupWallParticles(this);
  return;
}


/* C_ProjectedWallEntity::Spawn at 00530110 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void Spawn(C_ProjectedWallEntity * this) */

void __thiscall C_ProjectedWallEntity::Spawn(C_ProjectedWallEntity *this)

{
  C_BaseEntity::ThinkSet((C_BaseEntity *)this,(BASEPTR)0x155,0.0,(char *)0x0);
  C_BaseEntity::Spawn((C_BaseEntity *)this);
  return;
}


/* C_ProjectedWallEntity::ClientThink at 0052fd80 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void ClientThink(C_ProjectedWallEntity * this) */

void __thiscall C_ProjectedWallEntity::ClientThink(C_ProjectedWallEntity *this)

{
  float fVar1;
  int unaff_EBX;
  float fVar2;
  float fVar3;
  
                    /* Unresolved local var: float flDisplaySpeed@[???] */
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::ClientThink((C_BaseEntity *)this);
  fVar1 = *(float *)(*(int *)(&DAT_008394cf + unaff_EBX) + 0x2c);
  fVar2 = this->m_flLength;
  fVar3 = fVar2;
  if ((fVar1 <= fVar2) && (fVar3 = 0.0, 0.0 <= fVar1)) {
    fVar3 = fVar1;
  }
  fVar3 = fVar3 + this->m_flCurDisplayLength;
  this->m_flCurDisplayLength = fVar3;
  if ((fVar3 <= fVar2) && (fVar2 = 0.0, 0.0 <= fVar3)) {
    fVar2 = fVar3;
  }
  this->m_flCurDisplayLength = fVar2;
  (*(this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
    super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0xe9])(this);
  if (this->m_flParticleUpdateTime != this->m_flPrevParticleUpdateTime) {
    this->m_flPrevParticleUpdateTime = this->m_flParticleUpdateTime;
    SetupWallParticles(this);
  }
  (*(this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
    super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x9d])
            (this,*(float *)(unaff_EBX + 0x4b74c7) +
                  *(float *)(**(int **)(&DAT_0063400f + unaff_EBX) + 0xc));
  return;
}


/* C_ProjectedWallEntity::ProjectWall at 00530dd0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ProjectWall(C_ProjectedWallEntity * this) */

void __thiscall C_ProjectedWallEntity::ProjectWall(C_ProjectedWallEntity *this)

{
  uint *puVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  vec_t vVar9;
  vec_t vVar10;
  vec_t vVar11;
  vec_t vVar12;
  vec_t vVar13;
  vec_t vVar14;
  vec_t vVar15;
  vec_t vVar16;
  vec_t vVar17;
  PaintPowerType *src;
  uint uVar18;
  int iVar19;
  bool bVar20;
  int iVar21;
  char cVar22;
  bool bVar23;
  float *pfVar24;
  QAngle *pQVar25;
  int iVar26;
  C_BasePlayer *this_00;
  C_ProjectedWallEntity *pCVar27;
  CPhysCollide *pCVar28;
  vec_t *pvVar29;
  CPolyhedron *pCVar30;
  int iVar31;
  int unaff_EBX;
  NetworkVar_m_Collision *this_01;
  WallCollideableAtTime_t *pWVar32;
  longdouble lVar33;
  float fVar34;
  Vector *local_288;
  Vector *local_284;
  uint local_278;
  uint local_274;
  int local_270;
  float *local_208;
  int local_204;
  float local_1fc;
  float local_1f8;
  float local_1f4;
  float local_1f0;
  float local_1ec;
  float local_1e8;
  float local_1e4;
  float local_1e0;
  float local_1dc;
  float local_1d8;
  float local_1d4;
  float local_1d0;
  float local_1cc;
  float local_1c8;
  float local_1c4;
  float local_1c0;
  float local_1bc;
  float local_1b8;
  float local_1b4;
  float local_1b0;
  float local_1ac;
  float local_1a8;
  float local_1a4;
  float local_1a0;
  float local_19c [5];
  float *local_188;
  float *local_184;
  float *local_180;
  float *local_17c;
  float local_178;
  float local_174;
  float local_170;
  float local_16c;
  float local_168;
  float local_164;
  float local_160;
  float local_15c;
  float local_158;
  float local_154;
  float local_150;
  float local_14c;
  float local_148;
  float local_144;
  float local_140;
  float local_13c;
  float local_138;
  float local_134;
  float local_130;
  float local_12c;
  float local_128;
  Vector local_124;
  float local_118;
  float local_114;
  float local_110;
  Vector local_10c;
  float local_100;
  float local_fc;
  float local_f8;
  float local_f4;
  float local_f0;
  float local_ec;
  float local_e8;
  float local_e4;
  float local_e0;
  float local_dc;
  float local_d8;
  float local_d4;
  float local_d0;
  float local_cc;
  float local_c8;
  float local_c4;
  float local_c0;
  float local_bc;
  float local_b8;
  float local_b4;
  float local_b0;
  float local_ac;
  float local_a8;
  float local_a4;
  float local_a0;
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
  float local_5c;
  float local_58;
  float local_54;
  float local_50;
  float local_4c;
  float local_48;
  float local_44;
  Vector local_40;
  Vector local_34;
  Vector local_28 [2];
  
                    /* Unresolved local var: Vector vWallStartPoint@[???]
                       Unresolved local var: Vector vWallEndPoint@[???]
                       Unresolved local var: bool bCreate@[???]
                       Unresolved local var: int iReplaceIndex@[???]
                       Unresolved local var: int iValidIndex@[???]
                       Unresolved local var: Vector vecForward@[???]
                       Unresolved local var: Vector vecRight@[???]
                       Unresolved local var: Vector vecUp@[???] */
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::AddEffects((C_BaseEntity *)this,8);
  pfVar24 = (float *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                       super_C_BaseProjectedEntity.super_C_BaseEntity.super_IClientEntity.
                       super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0xe5])(this);
  fVar2 = pfVar24[2];
  fVar3 = pfVar24[1];
  fVar4 = *pfVar24;
  pfVar24 = (float *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                       super_C_BaseProjectedEntity.super_C_BaseEntity.super_IClientEntity.
                       super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0xe6])(this);
  fVar5 = pfVar24[2];
  fVar6 = pfVar24[1];
  fVar7 = *pfVar24;
  cVar22 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x63306b) + 0x3c))
                     ((int *)**(undefined4 **)(unaff_EBX + 0x63306b));
  if (cVar22 == '\0') {
    iVar26 = (this->m_WallCollideables).m_Size;
    iVar31 = iVar26;
    iVar21 = iVar26 * 0x4c;
    do {
      iVar19 = iVar21;
      iVar31 = iVar31 + -1;
      if (iVar31 < 1) goto LAB_005317fe;
      pWVar32 = (this->m_WallCollideables).m_Memory.m_pMemory;
      iVar21 = iVar19 + -0x4c;
    } while (*(float *)(**(int **)(unaff_EBX + 0x632fbb) + 0xc) <
             *(float *)((int)pWVar32->flTime + iVar19 + -0x4c));
    local_270 = 0;
    local_204 = 0;
    while( true ) {
                    /* Unresolved local var: int j@[???] */
      (**(code **)(*(int *)**(undefined4 **)(&DAT_006330d3 + unaff_EBX) + 0x44))
                ((int *)**(undefined4 **)(&DAT_006330d3 + unaff_EBX),
                 *(undefined4 *)((int)pWVar32->flTime + local_204 + 8));
      local_270 = local_270 + 1;
      local_204 = local_204 + 0x4c;
      if (local_270 == iVar31) break;
      pWVar32 = (this->m_WallCollideables).m_Memory.m_pMemory;
    }
    iVar26 = (this->m_WallCollideables).m_Size;
    if (0 < iVar26 - iVar31) {
      pWVar32 = (this->m_WallCollideables).m_Memory.m_pMemory;
      _V_memmove(pWVar32,(void *)((int)pWVar32[-1].flTime + iVar19 + -0x3c),(iVar26 - iVar31) * 0x4c
                );
      iVar26 = (this->m_WallCollideables).m_Size;
    }
    iVar26 = iVar26 - iVar31;
    (this->m_WallCollideables).m_Size = iVar26;
LAB_005317fe:
    if (iVar26 != 0) {
      pQVar25 = C_BaseEntity::GetNetworkAngles((C_BaseEntity *)this);
      pWVar32 = (this->m_WallCollideables).m_Memory.m_pMemory;
      if ((pQVar25->x == (pWVar32->qAngles).x) && (!NAN(pQVar25->x) && !NAN((pWVar32->qAngles).x)))
      {
        pvVar29 = &(pWVar32->qAngles).y;
        if ((pQVar25->y == *pvVar29) && (!NAN(pQVar25->y) && !NAN(*pvVar29))) {
          pvVar29 = &(pWVar32->qAngles).z;
          if ((pQVar25->z == *pvVar29) && (!NAN(pQVar25->z) && !NAN(*pvVar29))) {
            pfVar24 = (float *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                                 super_C_BaseProjectedEntity.super_C_BaseEntity.super_IClientEntity.
                                 super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0xe5])
                                         (this);
            pWVar32 = (this->m_WallCollideables).m_Memory.m_pMemory;
            if ((*pfVar24 == (pWVar32->vStart).x) && (!NAN(*pfVar24) && !NAN((pWVar32->vStart).x)))
            {
              pvVar29 = &(pWVar32->vStart).y;
              if ((pfVar24[1] == *pvVar29) && (!NAN(pfVar24[1]) && !NAN(*pvVar29))) {
                pvVar29 = &(pWVar32->vStart).z;
                if ((pfVar24[2] == *pvVar29) && (!NAN(pfVar24[2]) && !NAN(*pvVar29))) {
                  pfVar24 = (float *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                                       super_C_BaseProjectedEntity.super_C_BaseEntity.
                                       super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
                                       _vptr_IHandleEntity[0xe6])(this);
                  pWVar32 = (this->m_WallCollideables).m_Memory.m_pMemory;
                  if (*pfVar24 == (pWVar32->vEnd).x) {
                    pvVar29 = &(pWVar32->vEnd).y;
                    if ((pfVar24[1] == *pvVar29) && (!NAN(pfVar24[1]) && !NAN(*pvVar29))) {
                      pvVar29 = &(pWVar32->vEnd).z;
                      if ((pfVar24[2] == *pvVar29) && (!NAN(pfVar24[2]) && !NAN(*pvVar29))) {
                        fVar8 = *(float *)(**(int **)(unaff_EBX + 0x632fbb) + 0xc);
                        pWVar32->flTime[1] = fVar8;
                        pWVar32->flTime[0] = fVar8;
                        ((this->m_WallCollideables).m_Memory.m_pMemory)->bIsPredicted = false;
                        bVar20 = false;
                        local_278 = 0xffffffff;
                        local_274 = 0;
                        goto LAB_00530f5d;
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      (**(code **)(*(int *)**(undefined4 **)(&DAT_006330d3 + unaff_EBX) + 0x44))
                ((int *)**(undefined4 **)(&DAT_006330d3 + unaff_EBX),pWVar32->pCollideable);
      bVar20 = true;
      local_278 = 0;
      local_274 = 0xffffffff;
      goto LAB_00530f5d;
    }
LAB_00530f40:
    bVar20 = true;
    local_278 = 0xffffffff;
    local_274 = 0xffffffff;
  }
  else {
    local_274 = (this->m_WallCollideables).m_Size;
    iVar26 = local_274 * 0x4c;
    do {
      iVar31 = iVar26;
      local_278 = local_274;
      local_274 = local_278 - 1;
      if ((int)local_274 < 0) goto LAB_00530f40;
      iVar26 = iVar31 + -0x4c;
    } while (*(float *)(**(int **)(unaff_EBX + 0x632fbb) + 0xc) <
             *(float *)((int)((this->m_WallCollideables).m_Memory.m_pMemory)->flTime +
                       iVar31 + -0x48));
    pQVar25 = C_BaseEntity::GetNetworkAngles((C_BaseEntity *)this);
    local_208 = (float *)((int)(this->m_WallCollideables).m_Memory.m_pMemory[-1].flTime +
                         iVar31 + -0x3c);
    if ((pQVar25->x == local_208[0xc]) && (!NAN(pQVar25->x) && !NAN(local_208[0xc]))) {
      if ((pQVar25->y == local_208[0xd]) && (!NAN(pQVar25->y) && !NAN(local_208[0xd]))) {
        if ((pQVar25->z == local_208[0xe]) && (!NAN(pQVar25->z) && !NAN(local_208[0xe]))) {
          pfVar24 = (float *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                               super_C_BaseProjectedEntity.super_C_BaseEntity.super_IClientEntity.
                               super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0xe5])
                                       (this);
          local_208 = (float *)((int)(this->m_WallCollideables).m_Memory.m_pMemory[-1].flTime +
                               iVar31 + -0x3c);
          if ((*pfVar24 == *local_208) && (!NAN(*pfVar24) && !NAN(*local_208))) {
            if ((pfVar24[1] == local_208[1]) && (!NAN(pfVar24[1]) && !NAN(local_208[1]))) {
              if ((pfVar24[2] == local_208[2]) && (!NAN(pfVar24[2]) && !NAN(local_208[2]))) {
                pfVar24 = (float *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                                     super_C_BaseProjectedEntity.super_C_BaseEntity.
                                     super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
                                     _vptr_IHandleEntity[0xe6])(this);
                local_208 = (float *)((int)(this->m_WallCollideables).m_Memory.m_pMemory[-1].flTime
                                     + iVar31 + -0x3c);
                if (*pfVar24 == local_208[3]) {
                  if ((pfVar24[1] == local_208[4]) && (!NAN(pfVar24[1]) && !NAN(local_208[4]))) {
                    if ((pfVar24[2] == local_208[5]) && (!NAN(pfVar24[2]) && !NAN(local_208[5]))) {
                      bVar20 = false;
                      local_278 = 0xffffffff;
                      goto LAB_00530f5d;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    fVar8 = *(float *)(**(int **)(unaff_EBX + 0x632fbb) + 0xc);
    if (fVar8 < local_208[0x10] || fVar8 == local_208[0x10]) {
      (**(code **)(*(int *)**(undefined4 **)(&DAT_006330d3 + unaff_EBX) + 0x44))
                ((int *)**(undefined4 **)(&DAT_006330d3 + unaff_EBX),local_208[0x11]);
      *(undefined4 *)((int)(this->m_WallCollideables).m_Memory.m_pMemory[-1].flTime + iVar31 + 8) =
           0;
      local_278 = local_274;
    }
    else {
      if ((this->m_WallCollideables).m_Size <= (int)local_278) goto LAB_00530f40;
      CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t,_int>_>
      ::GrowVector(&this->m_WallCollideables,1);
      iVar26 = ~local_278 + (this->m_WallCollideables).m_Size;
      if (0 < iVar26) {
        pWVar32 = (this->m_WallCollideables).m_Memory.m_pMemory;
        _V_memmove(pWVar32 + local_278 + 1,pWVar32 + local_278,iVar26 * 0x4c);
      }
    }
    bVar20 = true;
    local_274 = 0xffffffff;
  }
LAB_00530f5d:
  for (iVar26 = FirstValidSplitScreenSlot(); iVar26 != -1; iVar26 = NextValidSplitScreenSlot(iVar26)
      ) {
    this_00 = C_BasePlayer::GetLocalPlayer(iVar26);
    if ((this_00 != (C_BasePlayer *)0x0) &&
       (pCVar27 = (C_ProjectedWallEntity *)C_BaseEntity::GetGroundEntity((C_BaseEntity *)this_00),
       this == pCVar27)) {
      C_BaseEntity::SetGroundEntity((C_BaseEntity *)this_00,(C_BaseEntity *)0x0);
    }
  }
  pQVar25 = C_BaseEntity::GetNetworkAngles((C_BaseEntity *)this);
  AngleVectors(pQVar25,local_28,&local_34,&local_40);
  if (bVar20) {
    if (*(int *)(*(int *)(*(int *)(&DAT_006337eb + unaff_EBX) + 0x1c) + 0x30) == 0) {
      puVar1 = (uint *)(unaff_EBX + 0x4b648f);
      local_1f4 = (float)((uint)local_28[0].z ^ *puVar1);
      local_1f8 = (float)((uint)local_28[0].y ^ *puVar1);
      local_1fc = (float)((uint)local_28[0].x ^ *puVar1);
      local_1f0 = fVar4 * local_1fc + fVar3 * local_1f8 + fVar2 * local_1f4;
      local_1ec = local_28[0].x;
      local_1e8 = local_28[0].y;
      local_1e4 = local_28[0].z;
      local_1e0 = fVar7 * local_28[0].x + fVar6 * local_28[0].y + fVar5 * local_28[0].z;
      local_a4 = *(float *)(unaff_EBX + 0x4b142f) * this->m_flWidth;
                    /* Unresolved local var: Vector res@[???] */
      local_ac = local_a4 * local_34.x;
      local_a8 = local_a4 * local_34.y;
      local_a4 = local_a4 * local_34.z;
                    /* Unresolved local var: Vector res@[???] */
      local_a0 = fVar4 + local_ac;
      local_9c = fVar3 + local_a8;
      local_98 = fVar2 + local_a4;
      local_1dc = local_34.x;
      local_1d8 = local_34.y;
      local_1d4 = local_34.z;
      local_1d0 = local_a0 * local_34.x + local_9c * local_34.y + local_98 * local_34.z;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
      local_b8 = fVar4 - local_ac;
      local_b4 = fVar3 - local_a8;
      local_b0 = fVar2 - local_a4;
      local_1c4 = (float)((uint)local_34.z ^ *puVar1);
      local_1c8 = (float)((uint)local_34.y ^ *puVar1);
      local_1cc = (float)((uint)local_34.x ^ *puVar1);
      local_1c0 = local_b8 * local_1cc + local_b4 * local_1c8 + local_b0 * local_1c4;
      local_e0 = *(float *)(unaff_EBX + 0x4b142f) * this->m_flHeight;
                    /* Unresolved local var: Vector res@[???] */
      local_e8 = local_e0 * local_40.x;
      local_e4 = local_e0 * local_40.y;
      local_e0 = local_e0 * local_40.z;
                    /* Unresolved local var: Vector res@[???] */
      local_dc = fVar4 + local_e8;
      local_d8 = fVar3 + local_e4;
      local_d4 = fVar2 + local_e0;
      local_1bc = local_40.x;
      local_1b8 = local_40.y;
      local_1b4 = local_40.z;
      local_1b0 = local_dc * local_40.x + local_d8 * local_40.y + local_d4 * local_40.z;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
      local_f4 = fVar4 - local_e8;
      local_f0 = fVar3 - local_e4;
      local_ec = fVar2 - local_e0;
      local_1a4 = (float)((uint)local_40.z ^ *puVar1);
      local_1a8 = (float)((uint)local_40.y ^ *puVar1);
      local_1ac = (float)((uint)local_40.x ^ *puVar1);
      local_1a0 = local_f4 * local_1ac + local_f0 * local_1a8 + local_ec * local_1a4;
      local_178 = local_1ac;
      local_174 = local_1a8;
      local_170 = local_1a4;
      local_16c = local_1ac;
      local_168 = local_1a8;
      local_164 = local_1a4;
      local_160 = local_1cc;
      local_15c = local_1c8;
      local_158 = local_1c4;
      local_154 = local_1cc;
      local_150 = local_1c8;
      local_14c = local_1c4;
      local_148 = local_1fc;
      local_144 = local_1f8;
      local_140 = local_1f4;
      local_13c = local_1fc;
      local_138 = local_1f8;
      local_134 = local_1f4;
      local_100 = local_1ac;
      local_fc = local_1a8;
      local_f8 = local_1a4;
      local_d0 = local_e8;
      local_cc = local_e4;
      local_c8 = local_e0;
      local_c4 = local_1cc;
      local_c0 = local_1c8;
      local_bc = local_1c4;
      local_94 = local_ac;
      local_90 = local_a8;
      local_8c = local_a4;
      local_88 = local_1fc;
      local_84 = local_1f8;
      local_80 = local_1f4;
      pCVar30 = GeneratePolyhedronFromPlanes(&local_1fc,6,1.1920929e-07,false);
      if (pCVar30 == (CPolyhedron *)0x0) {
        _Warning(unaff_EBX + 0x48a43f);
        return;
      }
      local_19c[0] = (float)(**(code **)(*(int *)**(undefined4 **)(&DAT_006330d3 + unaff_EBX) + 0x24
                                        ))((int *)**(undefined4 **)(&DAT_006330d3 + unaff_EBX),
                                           pCVar30);
      (*pCVar30->_vptr_CPolyhedron[2])(pCVar30);
      if (local_19c[0] == 0.0) {
        return;
      }
      pCVar28 = (CPhysCollide *)
                (**(code **)(*(int *)**(undefined4 **)(&DAT_006330d3 + unaff_EBX) + 0x3c))
                          ((int *)**(undefined4 **)(&DAT_006330d3 + unaff_EBX),local_19c,1);
    }
    else {
                    /* Unresolved local var: Vector vScaledRight@[???]
                       Unresolved local var: Vector vStart@[???]
                       Unresolved local var: Vector vEnd@[???]
                       Unresolved local var: Vector[4] vVerts@[???]
                       Unresolved local var: Vector *[4] pVerts@[???]
                       Unresolved local var: CPhysConvex * pTempConvex@[???] */
      local_44 = *(float *)(unaff_EBX + 0x4b142f) * this->m_flWidth;
                    /* Unresolved local var: Vector res@[???] */
      local_4c = local_44 * local_34.x;
      local_48 = local_44 * local_34.y;
      local_44 = local_44 * local_34.z;
                    /* Unresolved local var: Vector res@[???] */
      local_1fc = fVar4 + local_4c;
      local_1f8 = fVar3 + local_48;
      local_1f4 = fVar2 + local_44;
                    /* Unresolved local var: Vector res@[???] */
      local_1f0 = fVar4 - local_4c;
      local_1ec = fVar3 - local_48;
      local_1e8 = fVar2 - local_44;
                    /* Unresolved local var: Vector res@[???] */
      local_1e4 = fVar7 - local_4c;
      local_1e0 = fVar6 - local_48;
      local_1dc = fVar5 - local_44;
                    /* Unresolved local var: Vector res@[???] */
      local_1d8 = fVar7 + local_4c;
      local_1d4 = fVar6 + local_48;
      local_1d0 = local_44 + fVar5;
      local_188 = &local_1fc;
      local_184 = &local_1f0;
      local_180 = &local_1e4;
      local_17c = &local_1d8;
      local_7c = local_1d8;
      local_78 = local_1d4;
      local_74 = local_1d0;
      local_70 = local_1e4;
      local_6c = local_1e0;
      local_68 = local_1dc;
      local_64 = local_1f0;
      local_60 = local_1ec;
      local_5c = local_1e8;
      local_58 = local_1fc;
      local_54 = local_1f8;
      local_50 = local_1f4;
      local_19c[0] = (float)(**(code **)(*(int *)**(undefined4 **)(&DAT_006330d3 + unaff_EBX) + 8))
                                      ((int *)**(undefined4 **)(&DAT_006330d3 + unaff_EBX),
                                       &local_188,4);
      pCVar28 = (CPhysCollide *)
                (**(code **)(*(int *)**(undefined4 **)(&DAT_006330d3 + unaff_EBX) + 0x3c))
                          ((int *)**(undefined4 **)(&DAT_006330d3 + unaff_EBX),local_19c,1);
    }
    if (pCVar28 == (CPhysCollide *)0x0) {
      return;
    }
    (**(code **)(*(int *)**(undefined4 **)(&DAT_006330d3 + unaff_EBX) + 0x60))
              ((int *)**(undefined4 **)(&DAT_006330d3 + unaff_EBX),&local_124,&local_10c,pCVar28,
               *(undefined4 *)(&DAT_0063301f + unaff_EBX),*(undefined4 *)(unaff_EBX + 0x632f7b));
    pvVar29 = (vec_t *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                         super_C_BaseProjectedEntity.super_C_BaseEntity.super_IClientEntity.
                         super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0xe5])(this);
    vVar9 = *pvVar29;
    vVar10 = pvVar29[1];
    vVar11 = pvVar29[2];
    pvVar29 = (vec_t *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                         super_C_BaseProjectedEntity.super_C_BaseEntity.super_IClientEntity.
                         super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0xe6])(this);
    vVar12 = *pvVar29;
    vVar13 = pvVar29[1];
    vVar14 = pvVar29[2];
    pQVar25 = C_BaseEntity::GetNetworkAngles((C_BaseEntity *)this);
    vVar15 = pQVar25->x;
    vVar16 = pQVar25->y;
    vVar17 = pQVar25->z;
    bVar23 = (bool)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x63306b) + 0x3c))
                             ((int *)**(undefined4 **)(unaff_EBX + 0x63306b));
    iVar26 = **(int **)(unaff_EBX + 0x632fbb);
    fVar34 = (float)*(int *)(iVar26 + 0x18) * *(float *)(iVar26 + 0x1c);
    fVar8 = *(float *)(iVar26 + 0xc);
    if ((int)local_278 < 0) {
      uVar18 = (this->m_WallCollideables).m_Size;
      CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t,_int>_>
      ::GrowVector(&this->m_WallCollideables,1);
      iVar26 = ~uVar18 + (this->m_WallCollideables).m_Size;
      if (0 < iVar26) {
        pWVar32 = (this->m_WallCollideables).m_Memory.m_pMemory;
        _V_memmove(pWVar32 + uVar18 + 1,pWVar32 + uVar18,iVar26 * 0x4c);
      }
      iVar26 = uVar18 * 0x4c;
      pWVar32 = (this->m_WallCollideables).m_Memory.m_pMemory + uVar18;
      if (pWVar32 != (WallCollideableAtTime_t *)0x0) {
        pWVar32->pCollideable = pCVar28;
        pWVar32->flTime[1] = fVar8;
        pWVar32->flTime[0] = fVar34;
        pWVar32->bIsPredicted = bVar23;
        (pWVar32->qAngles).z = vVar17;
        (pWVar32->qAngles).y = vVar16;
        (pWVar32->qAngles).x = vVar15;
        (pWVar32->vWorldMaxs).z = local_10c.z;
        (pWVar32->vWorldMaxs).y = local_10c.y;
        (pWVar32->vWorldMaxs).x = local_10c.x;
        (pWVar32->vWorldMins).z = local_124.z;
        (pWVar32->vWorldMins).y = local_124.y;
        (pWVar32->vWorldMins).x = local_124.x;
        (pWVar32->vEnd).z = vVar14;
        (pWVar32->vEnd).y = vVar13;
        (pWVar32->vEnd).x = vVar12;
        (pWVar32->vStart).z = vVar11;
        (pWVar32->vStart).y = vVar10;
        (pWVar32->vStart).x = vVar9;
      }
    }
    else {
      iVar26 = local_278 * 0x4c;
      pWVar32 = (this->m_WallCollideables).m_Memory.m_pMemory + local_278;
      (pWVar32->vStart).x = vVar9;
      (pWVar32->vStart).y = vVar10;
      (pWVar32->vStart).z = vVar11;
      (pWVar32->vEnd).x = vVar12;
      (pWVar32->vEnd).y = vVar13;
      (pWVar32->vEnd).z = vVar14;
      (pWVar32->vWorldMins).x = local_124.x;
      (pWVar32->vWorldMins).y = local_124.y;
      (pWVar32->vWorldMins).z = local_124.z;
      (pWVar32->vWorldMaxs).x = local_10c.x;
      (pWVar32->vWorldMaxs).y = local_10c.y;
      (pWVar32->vWorldMaxs).z = local_10c.z;
      (pWVar32->qAngles).x = vVar15;
      (pWVar32->qAngles).y = vVar16;
      (pWVar32->qAngles).z = vVar17;
      pWVar32->flTime[1] = fVar8;
      pWVar32->flTime[0] = fVar34;
      pWVar32->pCollideable = pCVar28;
      pWVar32->bIsPredicted = bVar23;
    }
  }
  else {
    iVar26 = local_274 * 0x4c;
  }
                    /* Unresolved local var: Vector vSetMins@[???]
                       Unresolved local var: Vector vSetMaxs@[???] */
  local_284 = &local_10c;
  local_288 = &local_124;
  pWVar32 = (this->m_WallCollideables).m_Memory.m_pMemory;
  local_118 = *(float *)((int)pWVar32->flTime + iVar26 + -0x24);
  (this->m_vWorldSpace_WallMins).x = local_118;
  local_114 = *(float *)((int)pWVar32->flTime + iVar26 + -0x20);
  (this->m_vWorldSpace_WallMins).y = local_114;
  local_110 = *(float *)((int)pWVar32->flTime + iVar26 + -0x1c);
  (this->m_vWorldSpace_WallMins).z = local_110;
  local_130 = *(float *)((int)pWVar32->flTime + iVar26 + -0x18);
  (this->m_vWorldSpace_WallMaxs).x = local_130;
  local_12c = *(float *)((int)pWVar32->flTime + iVar26 + -0x14);
  (this->m_vWorldSpace_WallMaxs).y = local_12c;
  local_128 = *(float *)((int)pWVar32->flTime + iVar26 + -0x10);
  (this->m_vWorldSpace_WallMaxs).z = local_128;
                    /* Unresolved local var: Vector res@[???] */
  local_118 = local_118 - fVar4;
  local_114 = local_114 - fVar3;
  local_110 = local_110 - fVar2;
                    /* Unresolved local var: Vector res@[???] */
  local_130 = local_130 - fVar4;
  local_12c = local_12c - fVar3;
  local_128 = local_128 - fVar2;
  local_124.x = local_130;
  local_124.y = local_12c;
  local_124.z = local_128;
  local_10c.x = local_118;
  local_10c.y = local_114;
  local_10c.z = local_110;
  C_BaseEntity::SetSize((C_BaseEntity *)this,local_284,local_288);
  if (bVar20) {
    this->m_flCurDisplayLength = 0.0;
    (*(this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
      super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
      _vptr_IHandleEntity[0x9d])(this,0xc4a1a000);
  }
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: __m128 root@[???] */
  local_19c[0] = SQRT((fVar4 - fVar7) * (fVar4 - fVar7) + (fVar3 - fVar6) * (fVar3 - fVar6) +
                      (fVar2 - fVar5) * (fVar2 - fVar5));
  local_19c[1] = 0.0;
  local_19c[2] = 0.0;
  local_19c[3] = 0.0;
  this->m_flLength = local_19c[0];
  this_01 = &(this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
             super_C_BaseEntity.m_Collision;
  CCollisionProperty::MarkSurroundingBoundsDirty(&this_01->super_CCollisionProperty);
  CCollisionProperty::MarkPartitionHandleDirty(&this_01->super_CCollisionProperty);
  cVar22 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x63306b) + 0x3c))
                     ((int *)**(undefined4 **)(unaff_EBX + 0x63306b));
  if (cVar22 != '\0') {
    DisplaceObstructingEntities(this);
  }
  lVar33 = (longdouble)_ceilf(this->m_flLength / this->m_flSegmentLength);
  iVar26 = (int)lVar33;
  this->m_nNumSegments = iVar26;
  (this->m_PaintPowers).m_Size = 0;
  if (iVar26 != 0) {
    CUtlVector<PaintPowerType,CUtlMemory<PaintPowerType,_int>_>::GrowVector
              (&this->m_PaintPowers,iVar26);
                    /* Unresolved local var: int numToMove@[???] */
    iVar31 = (this->m_PaintPowers).m_Size - iVar26;
    if ((0 < iVar31) && (0 < iVar26)) {
      src = (this->m_PaintPowers).m_Memory.m_pMemory;
      _V_memmove(src + iVar26,src,iVar31 * 4);
    }
  }
  (*(this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
    super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0xf4])(this);
  this->m_fLastProjectedTime = *(float *)(**(int **)(unaff_EBX + 0x632fbb) + 0xc);
  return;
}


/* C_ProjectedWallEntity::GetProjectionExtents at 005300e0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void GetProjectionExtents(C_ProjectedWallEntity * this, Vector *
   outMins, Vector * outMaxs) */

void __thiscall
C_ProjectedWallEntity::GetProjectionExtents
          (C_ProjectedWallEntity *this,Vector *outMins,Vector *outMaxs)

{
  GetExtents(this,outMins,outMaxs,0.5);
  return;
}


/* __tcf_6 at 0097bfe0 */

void __tcf_6(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(&UNK_003ed438 + extraout_ECX),in_stack_00000008);
  return;
}


/* C_ProjectedWallEntity::ComputeWorldSpaceSurroundingBox at 005305d0 */

/* WARNING: Restarted to delay deadcode elimination for space: stack */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void ComputeWorldSpaceSurroundingBox(C_ProjectedWallEntity * this,
   Vector * pWorldMins, Vector * pWorldMaxs) */

void __thiscall
C_ProjectedWallEntity::ComputeWorldSpaceSurroundingBox
          (C_ProjectedWallEntity *this,Vector *pWorldMins,Vector *pWorldMaxs)

{
  float fVar1;
  WallCollideableAtTime_t *pWVar2;
  int iVar3;
  char cVar4;
  int iVar5;
  int unaff_EBX;
  int iVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  float fVar10;
  float fVar11;
  float fVar12;
  
  ___i686_get_pc_thunk_bx();
  if (*(int *)(*(int *)(unaff_EBX + 0x838e5e) + 0x30) == 0) {
    if ((this->m_WallCollideables).m_Size == 0) {
      pWorldMins->x = (this->m_vWorldSpace_WallMins).x;
      pWorldMins->y = (this->m_vWorldSpace_WallMins).y;
      pWorldMins->z = (this->m_vWorldSpace_WallMins).z;
      pWorldMaxs->x = (this->m_vWorldSpace_WallMaxs).x;
      pWorldMaxs->y = (this->m_vWorldSpace_WallMaxs).y;
      pWorldMaxs->z = (this->m_vWorldSpace_WallMaxs).z;
      return;
    }
    cVar4 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x63386e) + 0x3c))
                      ((int *)**(undefined4 **)(unaff_EBX + 0x63386e));
    if (cVar4 == '\0') {
LAB_005307f1:
      pWVar2 = (this->m_WallCollideables).m_Memory.m_pMemory;
      pWorldMins->x = (pWVar2->vWorldMins).x;
      pWorldMins->y = (pWVar2->vWorldMins).y;
      pWorldMins->z = (pWVar2->vWorldMins).z;
      pWVar2 = (this->m_WallCollideables).m_Memory.m_pMemory;
      pWorldMaxs->x = (pWVar2->vWorldMaxs).x;
      pWorldMaxs->y = (pWVar2->vWorldMaxs).y;
      pWorldMaxs->z = (pWVar2->vWorldMaxs).z;
    }
    else {
                    /* Unresolved local var: int i@[???] */
      iVar5 = (this->m_WallCollideables).m_Size;
      iVar6 = iVar5 * 0x4c;
      do {
        iVar3 = iVar6;
        iVar5 = iVar5 + -1;
        if (iVar5 < 0) goto LAB_005307f1;
        pWVar2 = (this->m_WallCollideables).m_Memory.m_pMemory;
        iVar6 = iVar3 + -0x4c;
      } while (this->m_fLastProjectedTime < *(float *)((int)pWVar2[-1].flTime + iVar3 + 4));
      pWorldMins->x = *(vec_t *)((int)pWVar2[-1].flTime + iVar3 + -0x24);
      pWorldMins->y = *(vec_t *)((int)pWVar2[-1].flTime + iVar3 + -0x20);
      pWorldMins->z = *(vec_t *)((int)pWVar2[-1].flTime + iVar3 + -0x1c);
      pWVar2 = (this->m_WallCollideables).m_Memory.m_pMemory;
      pWorldMaxs->x = *(vec_t *)((int)pWVar2[-1].flTime + iVar3 + -0x18);
      pWorldMaxs->y = *(vec_t *)((int)pWVar2[-1].flTime + iVar3 + -0x14);
      pWorldMaxs->z = *(vec_t *)((int)pWVar2[-1].flTime + iVar3 + -0x10);
    }
  }
  else {
    fVar12 = (this->m_vWorldSpace_WallMins).x;
    fVar7 = (this->m_vWorldSpace_WallMins).y;
    fVar8 = (this->m_vWorldSpace_WallMins).z;
    fVar9 = (this->m_vWorldSpace_WallMaxs).x;
    fVar11 = (this->m_vWorldSpace_WallMaxs).y;
    fVar10 = (this->m_vWorldSpace_WallMaxs).z;
                    /* Unresolved local var: Vector vMins@[???]
                       Unresolved local var: Vector vMaxs@[???]
                       Unresolved local var: int i@[???] */
    if ((this->m_WallCollideables).m_Size != 0) {
      iVar6 = 0;
      iVar5 = 0;
      do {
        pWVar2 = (this->m_WallCollideables).m_Memory.m_pMemory;
        fVar1 = *(float *)((int)pWVar2->flTime + iVar5 + -0x1c);
        if (fVar1 <= fVar8) {
          fVar8 = fVar1;
        }
        fVar1 = *(float *)((int)pWVar2->flTime + iVar5 + -0x20);
        if (fVar1 <= fVar7) {
          fVar7 = fVar1;
        }
        fVar1 = *(float *)((int)pWVar2->flTime + iVar5 + -0x24);
        if (fVar1 <= fVar12) {
          fVar12 = fVar1;
        }
        fVar1 = *(float *)((int)pWVar2->flTime + iVar5 + -0x10);
        if (fVar10 <= fVar1) {
          fVar10 = fVar1;
        }
        fVar1 = *(float *)((int)pWVar2->flTime + iVar5 + -0x14);
        if (fVar11 <= fVar1) {
          fVar11 = fVar1;
        }
        fVar1 = *(float *)((int)pWVar2->flTime + iVar5 + -0x18);
        if (fVar9 <= fVar1) {
          fVar9 = fVar1;
        }
        iVar6 = iVar6 + 1;
        iVar5 = iVar5 + 0x4c;
      } while ((this->m_WallCollideables).m_Size != iVar6);
    }
    pWorldMins->x = fVar12;
    pWorldMins->y = fVar7;
    pWorldMins->z = fVar8;
    pWorldMaxs->x = fVar9;
    pWorldMaxs->y = fVar11;
    pWorldMaxs->z = fVar10;
  }
  return;
}


/* C_ProjectedWallEntity::TestCollision at 005309a0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: bool TestCollision(C_ProjectedWallEntity * this, Ray_t * ray, uint
   mask, trace_t * trace) */

bool __thiscall
C_ProjectedWallEntity::TestCollision
          (C_ProjectedWallEntity *this,Ray_t *ray,uint mask,trace_t *trace)

{
  float *pfVar1;
  WallCollideableAtTime_t *pWVar2;
  int iVar3;
  int iVar4;
  undefined1 uVar5;
  char cVar6;
  CPhysCollide *pCVar7;
  int iVar8;
  int *piVar9;
  int unaff_EBX;
  undefined4 uVar10;
  undefined4 uVar11;
  
  ___i686_get_pc_thunk_bx();
  uVar5 = 0;
  if ((this->m_WallCollideables).m_Size != 0) {
    cVar6 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x633498) + 0x3c))
                      ((int *)**(undefined4 **)(unaff_EBX + 0x633498));
    if (cVar6 == '\0') {
LAB_00530a90:
      piVar9 = (int *)**(undefined4 **)(CProxyHealth::~CProxyHealth + unaff_EBX);
      iVar8 = *piVar9;
      uVar11 = *(undefined4 *)(&DAT_006333a8 + unaff_EBX);
      uVar10 = *(undefined4 *)(unaff_EBX + 0x63344c);
      pCVar7 = ((this->m_WallCollideables).m_Memory.m_pMemory)->pCollideable;
    }
    else {
                    /* Unresolved local var: int i@[???] */
      iVar8 = (this->m_WallCollideables).m_Size;
      iVar4 = iVar8 * 0x4c;
      do {
        iVar3 = iVar4;
        iVar8 = iVar8 + -1;
        if (iVar8 < 0) goto LAB_00530a90;
        pWVar2 = (this->m_WallCollideables).m_Memory.m_pMemory;
        iVar4 = iVar3 + -0x4c;
      } while (this->m_fLastProjectedTime < *(float *)((int)pWVar2[-1].flTime + iVar3 + 4));
      piVar9 = (int *)**(undefined4 **)(CProxyHealth::~CProxyHealth + unaff_EBX);
      iVar8 = *piVar9;
      uVar11 = *(undefined4 *)(&DAT_006333a8 + unaff_EBX);
      uVar10 = *(undefined4 *)(unaff_EBX + 0x63344c);
      pCVar7 = *(CPhysCollide **)((int)pWVar2[-1].flTime + iVar3 + 8);
    }
    (**(code **)(iVar8 + 0x88))(piVar9,ray,mask,0,pCVar7,uVar10,uVar11,trace);
    pfVar1 = &(trace->super_CBaseTrace).fraction;
    if ((*pfVar1 <= *(float *)(unaff_EBX + 0x4b17d0) && *(float *)(unaff_EBX + 0x4b17d0) != *pfVar1)
       || ((trace->super_CBaseTrace).allsolid != false)) {
      uVar5 = 1;
    }
    else {
      uVar5 = (trace->super_CBaseTrace).startsolid;
    }
  }
  return (bool)uVar5;
}


/* C_ProjectedWallEntity::TestHitboxes at 0052f290 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: bool TestHitboxes(C_ProjectedWallEntity * this, Ray_t * ray, uint
   fContentsMask, trace_t * tr) */

bool __thiscall
C_ProjectedWallEntity::TestHitboxes
          (C_ProjectedWallEntity *this,Ray_t *ray,uint fContentsMask,trace_t *tr)

{
  int iVar1;
  
  iVar1 = (*(this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
            super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
            _vptr_IHandleEntity[0x45])(this,ray,fContentsMask,tr);
  return (bool)(char)iVar1;
}


/* C_ProjectedWallEntity::SetPaintSurfaceColor at 0052f370 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: bool SetPaintSurfaceColor(C_ProjectedWallEntity * this, PaintPowerType
   * power) */

bool __thiscall
C_ProjectedWallEntity::SetPaintSurfaceColor(C_ProjectedWallEntity *this,PaintPowerType *power)

{
  Color *pCVar1;
  int unaff_EBX;
  bool bVar2;
  int in_stack_ffffffb8;
  float local_28;
  float local_24;
  float local_20;
  
                    /* Unresolved local var: float[3] flColors@[???]
                       Unresolved local var: bool bIsPainted@[???] */
  ___i686_get_pc_thunk_bx();
  local_28 = 1.0;
  local_24 = 1.0;
  local_20 = 1.0;
  pCVar1 = (Color *)*power;
  bVar2 = pCVar1 != (Color *)&vgui::CvarToggleCheckButton<CGameUIConVarRef>::GetKBMap::s_pMap.
                              cputype;
  if (bVar2) {
                    /* Unresolved local var: Color color@[???] */
    pCVar1 = MapPowerToVisualColor(pCVar1,in_stack_ffffffb8);
    local_20 = *(float *)(&DAT_004b31c2 + unaff_EBX);
    local_28 = (float)((uint)pCVar1 & 0xff) / local_20;
    local_24 = (float)((uint)pCVar1 >> 8 & 0xff) / local_20;
    local_20 = (float)((uint)pCVar1 >> 0x10 & 0xff) / local_20;
  }
  (*this->m_pPaintColorMid->_vptr_IMaterialVar[9])(this->m_pPaintColorMid,&local_28,3);
  (*this->m_pPaintColorEnd1->_vptr_IMaterialVar[9])(this->m_pPaintColorEnd1,&local_28,3);
  (*this->m_pPaintColorEnd2->_vptr_IMaterialVar[9])(this->m_pPaintColorEnd2,&local_28,3);
  (*this->m_pPaintColorSing->_vptr_IMaterialVar[9])(this->m_pPaintColorSing,&local_28,3);
  return bVar2;
}


/* C_ProjectedWallEntity::SetPaintPower at 0052f2c0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void SetPaintPower(C_ProjectedWallEntity * this, int nSegment,
   PaintPowerType power) */

void __thiscall
C_ProjectedWallEntity::SetPaintPower(C_ProjectedWallEntity *this,int nSegment,PaintPowerType power)

{
  if (nSegment < this->m_nNumSegments) {
    (this->m_PaintPowers).m_Memory.m_pMemory[nSegment] = power;
  }
  return;
}


/* C_TEWallPaintedEvent::PostDataUpdate at 00537770 */

/* DWARF original prototype: void PostDataUpdate(C_TEWallPaintedEvent * this, DataUpdateType_t
   updateType) */

void __thiscall
C_TEWallPaintedEvent::PostDataUpdate(C_TEWallPaintedEvent *this,DataUpdateType_t updateType)

{
  uint uVar1;
  int iVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: C_ProjectedWallEntity * pWall@[???] */
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->m_hEntity).m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x62c65b),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) && (iVar2 = *(int *)(iVar2 + 4), iVar2 != 0)) {
    iVar2 = ___dynamic_cast(iVar2,unaff_EBX + 0x62e03f,&UNK_0069567f + unaff_EBX,0);
    if (iVar2 != 0) {
      if (this->m_nSegment < *(int *)(iVar2 + 0xb38)) {
        *(int *)(*(int *)(iVar2 + 0xb40) + this->m_nSegment * 4) = this->m_colorIndex;
        return;
      }
    }
  }
  return;
}


/* C_ProjectedWallEntity::GetToolRecordingState at 00530350 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void GetToolRecordingState(C_ProjectedWallEntity * this, KeyValues *
   msg) */

void __thiscall
C_ProjectedWallEntity::GetToolRecordingState(C_ProjectedWallEntity *this,KeyValues *msg)

{
  float fVar1;
  float fVar2;
  KeyValues *this_00;
  char *keyName;
  float *pfVar3;
  QAngle *pQVar4;
  int unaff_EBX;
  
                    /* Unresolved local var: KeyValues * pKV@[???]
                       Unresolved local var: Vector vStart@[???]
                       Unresolved local var: QAngle qAngles@[???] */
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::GetToolRecordingState((C_BaseEntity *)this,msg);
  this_00 = CIFM_EntityKeyValuesHandler_AutoRegister::FindOrCreateNonConformantKeyValues(msg);
  keyName = CIFM_EntityKeyValuesHandler_AutoRegister::GetHandlerIDKeyString();
  KeyValues::SetString(this_00,keyName,(char *)(unaff_EBX + 0x48ad2b));
  KeyValues::SetInt(this_00,(char *)(unaff_EBX + 0x45a3d2),
                    (this->super_CPaintableEntity<C_BaseProjectedEntity>).
                    super_C_BaseProjectedEntity.super_C_BaseEntity.index);
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x47a14a),this->m_flWidth);
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x47a150),this->m_flHeight);
  KeyValues::SetInt(this_00,(char *)(unaff_EBX + 0x48ac44),this->m_nNumSegments);
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x48ac50),this->m_flSegmentLength);
  KeyValues::SetFloat(this_00,&UNK_0048ac5a + unaff_EBX,this->m_flCurDisplayLength);
  pfVar3 = (float *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                      super_C_BaseProjectedEntity.super_C_BaseEntity.super_IClientEntity.
                      super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0xe5])(this);
  fVar1 = pfVar3[2];
  fVar2 = pfVar3[1];
  KeyValues::SetFloat(this_00,&UNK_0048b2be + unaff_EBX,*pfVar3);
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x48ac39),fVar2);
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x48ac3e),fVar1);
  pQVar4 = C_BaseEntity::GetNetworkAngles((C_BaseEntity *)this);
  fVar1 = pQVar4->x;
  fVar2 = pQVar4->z;
  KeyValues::SetFloat(this_00,(char *)(CLampHaloProxy::GetMaterial + unaff_EBX + 5),pQVar4->y);
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x48ac6a),fVar1);
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x48ac6f),fVar2);
  return;
}


/* C_ProjectedWallEntity::RestoreToToolRecordedState at 0052fe60 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void RestoreToToolRecordedState(C_ProjectedWallEntity * this, KeyValues
   * pKV) */

void __thiscall
C_ProjectedWallEntity::RestoreToToolRecordedState(C_ProjectedWallEntity *this,KeyValues *pKV)

{
  int iVar1;
  matrix3x4_t *matrix;
  int unaff_EBX;
  float fVar2;
  QAngle local_58;
  Vector local_4c;
  float local_40;
  float local_3c;
  float local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  
                    /* Unresolved local var: QAngle qKVAngles@[???]
                       Unresolved local var: matrix3x4_t * pMatrix@[???] */
  ___i686_get_pc_thunk_bx();
  fVar2 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x48b7ae),0.0);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.m_vecStartPoint.
  x = fVar2;
  fVar2 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x48b129),0.0);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.m_vecStartPoint.
  y = fVar2;
  fVar2 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x48b12e),0.0);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.m_vecStartPoint.
  z = fVar2;
  fVar2 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x47a63a),0.0);
  this->m_flWidth = fVar2;
  fVar2 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x47a640),0.0);
  this->m_flHeight = fVar2;
  iVar1 = KeyValues::GetInt(pKV,&DAT_0048b134 + unaff_EBX,0);
  this->m_nNumSegments = iVar1;
  fVar2 = KeyValues::GetFloat(pKV,&UNK_0048b140 + unaff_EBX,0.0);
  this->m_flSegmentLength = fVar2;
  fVar2 = KeyValues::GetFloat(pKV,&UNK_0048b14a + unaff_EBX,0.0);
  this->m_flCurDisplayLength = fVar2;
  local_28 = 0xc6800000;
  local_24 = 0xc6800000;
  local_20 = 0xc6800000;
  (this->m_vWorldSpace_WallMins).x = -16384.0;
  (this->m_vWorldSpace_WallMins).y = -16384.0;
  (this->m_vWorldSpace_WallMins).z = -16384.0;
  local_34 = 0x46800000;
  local_30 = 0x46800000;
  local_2c = 0x46800000;
  (this->m_vWorldSpace_WallMaxs).x = 16384.0;
  (this->m_vWorldSpace_WallMaxs).y = 16384.0;
  (this->m_vWorldSpace_WallMaxs).z = 16384.0;
  C_BaseEntity::SetSize
            ((C_BaseEntity *)this,&this->m_vWorldSpace_WallMins,&this->m_vWorldSpace_WallMaxs);
  local_3c = KeyValues::GetFloat(pKV,&UNK_0048b155 + unaff_EBX,0.0);
  local_40 = KeyValues::GetFloat(pKV,&UNK_0048b15a + unaff_EBX,0.0);
  local_38 = KeyValues::GetFloat(pKV,&UNK_0048b15f + unaff_EBX,0.0);
  matrix = KeyValues::GetPtr(pKV,&UNK_0048b164 + unaff_EBX,(void *)0x0);
  if (matrix != (matrix3x4_t *)0x0) {
                    /* Unresolved local var: Vector vPos@[???]
                       Unresolved local var: QAngle qAngles@[???] */
    MatrixAngles(matrix,&local_58.x);
    local_4c.x = matrix->m_flMatVal[0][3];
    local_4c.y = matrix->m_flMatVal[1][3];
    local_4c.z = matrix->m_flMatVal[2][3];
    local_58.x = local_40;
    local_58.y = local_3c;
    local_58.z = local_38;
    C_BaseEntity::SetNetworkOrigin((C_BaseEntity *)this,&local_4c);
    C_BaseEntity::SetAbsOrigin((C_BaseEntity *)this,&local_4c);
    C_BaseEntity::SetLocalOrigin((C_BaseEntity *)this,&local_4c);
    C_BaseEntity::SetNetworkAngles((C_BaseEntity *)this,&local_58);
    C_BaseEntity::SetAbsAngles((C_BaseEntity *)this,&local_58);
    C_BaseEntity::SetLocalAngles((C_BaseEntity *)this,&local_58);
  }
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.m_VisibilityBits.super_CBitVecT<CFixedBitVecBase<2>_>.super_CFixedBitVecBase<2>
  .m_Ints[0] = 0xffffffff;
  return;
}


/* C_ProjectedWallEntity::ShouldSpawnParticles at 0052f470 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: bool ShouldSpawnParticles(C_ProjectedWallEntity * this, C_Portal_Base2D
   * pPortal) */

bool __thiscall
C_ProjectedWallEntity::ShouldSpawnParticles(C_ProjectedWallEntity *this,C_Portal_Base2D *pPortal)

{
  bool bVar1;
  int iVar2;
  QAngle *pQVar3;
  int unaff_EBX;
  float fVar4;
  float fVar5;
  Vector local_24;
  Vector local_18;
  
                    /* Unresolved local var: C_Prop_Portal * pPropPortal@[???]
                       Unresolved local var: Vector vPortalUp@[???]
                       Unresolved local var: Vector vWallUp@[???]
                       Unresolved local var: float flDot@[???] */
  ___i686_get_pc_thunk_bx();
  bVar1 = C_Portal_Base2D::IsActivedAndLinked(pPortal);
  if (!bVar1) {
    return true;
  }
  if ((pPortal != (C_Portal_Base2D *)0x0) &&
     (iVar2 = ___dynamic_cast(pPortal,*(undefined4 *)(unaff_EBX + 0x634bff),
                              *(undefined4 *)(unaff_EBX + 0x634c07),0), iVar2 != 0)) {
    pQVar3 = (QAngle *)
             (*(pPortal->super_CPortalRenderable_FlatBasic).super_C_BaseAnimating.super_C_BaseEntity
               .super_IClientEntity.super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity
               [0xe])(pPortal);
    AngleVectors(pQVar3,(Vector *)0x0,(Vector *)0x0,&local_18);
    pQVar3 = (QAngle *)
             (*(this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
               super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
               _vptr_IHandleEntity[0xe])(this);
    AngleVectors(pQVar3,(Vector *)0x0,(Vector *)0x0,&local_24);
    fVar5 = local_18.x * local_24.x + local_18.y * local_24.y + local_18.z * local_24.z;
    if (*(float *)(unaff_EBX + 0x4b2d07) <= fVar5 && fVar5 != *(float *)(unaff_EBX + 0x4b2d07)) {
      return true;
    }
    fVar4 = *(float *)(unaff_EBX + 0x4b2dcf);
    if (*(float *)(unaff_EBX + 0x4b2dcf) <= fVar5) {
      fVar4 = fVar5;
    }
    return *(double *)(unaff_EBX + 0x4b3223) <=
           (double)(float)((uint)fVar4 & *(uint *)(&DAT_004b7de3 + unaff_EBX)) &&
           (double)(float)((uint)fVar4 & *(uint *)(&DAT_004b7de3 + unaff_EBX)) !=
           *(double *)(unaff_EBX + 0x4b3223);
  }
  return false;
}


/* C_ProjectedWallEntity::SetupWallParticles at 0052f590 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void SetupWallParticles(C_ProjectedWallEntity * this) */

void __thiscall C_ProjectedWallEntity::SetupWallParticles(C_ProjectedWallEntity *this)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  uint uVar7;
  Vector vecOrigin;
  Vector vecOrigin_00;
  Vector vecOrigin_01;
  Vector vecOrigin_02;
  Vector vecOrigin_03;
  Vector vecOrigin_04;
  QAngle vecAngles;
  QAngle vecAngles_00;
  QAngle vecAngles_01;
  QAngle vecAngles_02;
  QAngle vecAngles_03;
  QAngle vecAngles_04;
  bool bVar8;
  int iVar9;
  float *pfVar10;
  QAngle *pQVar11;
  int unaff_EBX;
  char *pcVar12;
  float fVar13;
  C_Portal_Base2D *local_140;
  C_Portal_Base2D *local_120;
  Vector local_58;
  Vector local_4c;
  Vector local_40;
  QAngle local_34;
  float local_28;
  float local_24;
  float local_20;
  
                    /* Unresolved local var: float flImpactPointOffset@[???]
                       Unresolved local var: C_Portal_Base2D * pSourcePortal@[???]
                       Unresolved local var: C_Portal_Base2D * pHitPortal@[???]
                       Unresolved local var: Vector vecParticlePos@[???]
                       Unresolved local var: Vector vWallEndPoint@[???]
                       Unresolved local var: Vector vWallStartPoint@[???]
                       Unresolved local var: QAngle qAngles@[???]
                       Unresolved local var: Vector vecForward@[???]
                       Unresolved local var: Vector vecRight@[???]
                       Unresolved local var: Vector vecUp@[???] */
  ___i686_get_pc_thunk_bx();
  StopParticleEffects((C_BaseEntity *)this);
  uVar7 = (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
          m_hSourcePortal.super_CBaseHandle.m_Index;
  if ((uVar7 == 0xffffffff) ||
     (iVar9 = (uVar7 & 0xffff) * 0x10 + **(int **)(&DAT_0063483b + unaff_EBX),
     *(uint *)(iVar9 + 8) != uVar7 >> 0x10)) {
    local_140 = (C_Portal_Base2D *)0x0;
  }
  else {
    local_140 = *(C_Portal_Base2D **)(iVar9 + 4);
  }
  uVar7 = (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
          m_hHitPortal.super_CBaseHandle.m_Index;
  if ((uVar7 == 0xffffffff) ||
     (iVar9 = (uVar7 & 0xffff) * 0x10 + **(int **)(&DAT_0063483b + unaff_EBX),
     *(uint *)(iVar9 + 8) != uVar7 >> 0x10)) {
    local_120 = (C_Portal_Base2D *)0x0;
  }
  else {
    local_120 = *(C_Portal_Base2D **)(iVar9 + 4);
  }
  pfVar10 = (float *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                       super_C_BaseProjectedEntity.super_C_BaseEntity.super_IClientEntity.
                       super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0xe6])(this);
  fVar1 = pfVar10[2];
  fVar2 = pfVar10[1];
  fVar3 = *pfVar10;
  pfVar10 = (float *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                       super_C_BaseProjectedEntity.super_C_BaseEntity.super_IClientEntity.
                       super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0xe5])(this);
  fVar4 = pfVar10[2];
  fVar5 = pfVar10[1];
  fVar6 = *pfVar10;
  pQVar11 = C_BaseEntity::GetNetworkAngles((C_BaseEntity *)this);
  local_34.x = pQVar11->x;
  local_34.y = pQVar11->y;
  local_34.z = pQVar11->z;
  AngleVectors(&local_34,&local_40,&local_4c,&local_58);
  if ((local_140 == (C_Portal_Base2D *)0x0) ||
     (bVar8 = ShouldSpawnParticles(this,local_140), !bVar8)) {
    if (local_120 != (C_Portal_Base2D *)0x0) goto LAB_0052f6cf;
  }
  else {
                    /* Unresolved local var: C_Prop_Portal * pPortal@[???]
                       Unresolved local var: Vector res@[???] */
    fVar13 = *(float *)(unaff_EBX + 0x4b3073);
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
    local_28 = fVar6 + local_40.x + local_40.x + local_4c.x * fVar13;
    local_24 = fVar5 + local_40.y + local_40.y + local_4c.y * fVar13;
    local_20 = fVar4 + local_40.z + local_40.z + local_4c.z * fVar13;
    pcVar12 = &UNK_0048b9df + unaff_EBX;
    vecOrigin_01.y = local_24;
    vecOrigin_01.x = local_28;
    vecOrigin_01.z = local_20;
    vecAngles_01.y = local_34.y;
    vecAngles_01.x = local_34.x;
    vecAngles_01.z = local_34.z;
    DispatchParticleEffect(pcVar12,vecOrigin_01,vecAngles_01,(C_BaseEntity *)this,-1);
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
    local_28 = (fVar6 + local_40.x + local_40.x) - local_4c.x * fVar13;
    local_24 = (fVar5 + local_40.y + local_40.y) - local_4c.y * fVar13;
    local_20 = (fVar4 + local_40.z + local_40.z) - local_4c.z * fVar13;
    vecOrigin_02.y = local_24;
    vecOrigin_02.x = local_28;
    vecOrigin_02.z = local_20;
    vecAngles_02.y = local_34.y;
    vecAngles_02.x = local_34.x;
    vecAngles_02.z = local_34.z;
    DispatchParticleEffect(pcVar12,vecOrigin_02,vecAngles_02,(C_BaseEntity *)this,-1);
    if (local_120 == (C_Portal_Base2D *)0x0) goto LAB_0052f6f7;
    iVar9 = ___dynamic_cast(local_120,*(undefined4 *)(unaff_EBX + 0x634adb),
                            *(undefined4 *)(unaff_EBX + 0x634ae3),0);
    if (iVar9 != 0) {
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
      local_28 = fVar6 + local_40.x + local_40.x;
      local_24 = fVar5 + local_40.y + local_40.y;
      local_20 = fVar4 + local_40.z + local_40.z;
      vecOrigin_03.y = local_24;
      vecOrigin_03.x = local_28;
      vecOrigin_03.z = local_20;
      vecAngles_03.y = local_34.y;
      vecAngles_03.x = local_34.x;
      vecAngles_03.z = local_34.z;
      DispatchParticleEffect(pcVar12,vecOrigin_03,vecAngles_03,(C_BaseEntity *)this,-1);
    }
LAB_0052f6cf:
    bVar8 = ShouldSpawnParticles(this,local_120);
    if (!bVar8) {
      return;
    }
  }
  fVar13 = *(float *)(unaff_EBX + 0x4b3073);
LAB_0052f6f7:
                    /* Unresolved local var: Vector res@[???] */
  pcVar12 = &UNK_0048b9df + unaff_EBX;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
  local_28 = (fVar3 - (local_40.x + local_40.x)) + local_4c.x * fVar13;
  local_24 = (fVar2 - (local_40.y + local_40.y)) + local_4c.y * fVar13;
  local_20 = (fVar1 - (local_40.z + local_40.z)) + local_4c.z * fVar13;
  vecOrigin.y = local_24;
  vecOrigin.x = local_28;
  vecOrigin.z = local_20;
  vecAngles.y = local_34.y;
  vecAngles.x = local_34.x;
  vecAngles.z = local_34.z;
  DispatchParticleEffect(pcVar12,vecOrigin,vecAngles,(C_BaseEntity *)this,-1);
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
  local_28 = (fVar3 - (local_40.x + local_40.x)) - local_4c.x * fVar13;
  local_24 = (fVar2 - (local_40.y + local_40.y)) - local_4c.y * fVar13;
  local_20 = (fVar1 - (local_40.z + local_40.z)) - fVar13 * local_4c.z;
  vecOrigin_00.y = local_24;
  vecOrigin_00.x = local_28;
  vecOrigin_00.z = local_20;
  vecAngles_00.y = local_34.y;
  vecAngles_00.x = local_34.x;
  vecAngles_00.z = local_34.z;
  DispatchParticleEffect(pcVar12,vecOrigin_00,vecAngles_00,(C_BaseEntity *)this,-1);
  if (local_120 != (C_Portal_Base2D *)0x0) {
    return;
  }
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
  local_28 = fVar3 - (local_40.x + local_40.x);
  local_24 = fVar2 - (local_40.y + local_40.y);
  local_20 = fVar1 - (local_40.z + local_40.z);
  vecOrigin_04.y = local_24;
  vecOrigin_04.x = local_28;
  vecOrigin_04.z = local_20;
  vecAngles_04.y = local_34.y;
  vecAngles_04.x = local_34.x;
  vecAngles_04.z = local_34.z;
  DispatchParticleEffect(pcVar12,vecOrigin_04,vecAngles_04,(C_BaseEntity *)this,-1);
  return;
}


/* __tcf_7 at 0097c160 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_DynamicProp": ignoring overlapping field "m_bUseHitboxesForRenderBox" */

void __tcf_7(void *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  *(int *)(unaff_EBX + 0x3563ec) = *(int *)(unaff_EBX + 0x1e7d10) + 8;
  *(undefined4 *)(unaff_EBX + 0x356404) = 0;
  if (-1 < *(int *)(C_BasePropDoor::GetClientClass + unaff_EBX)) {
    if (*(int *)(&DAT_003563f8 + unaff_EBX) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1e7bfc) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1e7bfc),*(int *)(&DAT_003563f8 + unaff_EBX))
      ;
      *(undefined4 *)(&DAT_003563f8 + unaff_EBX) = 0;
    }
    *(undefined4 *)(&DAT_003563fc + unaff_EBX) = 0;
  }
  iVar1 = *(int *)(&DAT_003563f8 + unaff_EBX);
  *(int *)(unaff_EBX + 0x356408) = iVar1;
  if (-1 < *(int *)(C_BasePropDoor::GetClientClass + unaff_EBX)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1e7bfc) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1e7bfc),iVar1);
      *(undefined4 *)(&DAT_003563f8 + unaff_EBX) = 0;
    }
    *(undefined4 *)(&DAT_003563fc + unaff_EBX) = 0;
  }
  return;
}


/* CProjectedWallEntity_NonConformantDataHandler::DestroyInstance at 00537410 */

/* DWARF original prototype: void DestroyInstance(CProjectedWallEntity_NonConformantDataHandler *
   this, void * pEntity) */

void __thiscall
CProjectedWallEntity_NonConformantDataHandler::DestroyInstance
          (CProjectedWallEntity_NonConformantDataHandler *this,void *pEntity)

{
  code *UNRECOVERED_JUMPTABLE;
  int unaff_EBX;
  
                    /* Unresolved local var: C_ProjectedWallEntity * pCastEntity@[???] */
  ___i686_get_pc_thunk_bx();
  UNRECOVERED_JUMPTABLE = *(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x62ca13) + 0x44);
  if (pEntity != (void *)0x0) {
    (*UNRECOVERED_JUMPTABLE)((int *)**(undefined4 **)(unaff_EBX + 0x62ca13),(int)pEntity + 4);
                    /* WARNING: Could not recover jumptable at 0x00537452. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (**(code **)(*(int *)pEntity + 4))();
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x00537473. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}


/* CProjectedWallEntity_NonConformantDataHandler::HandleInstance at 005376c0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void HandleInstance(CProjectedWallEntity_NonConformantDataHandler *
   this, void * pEntity, KeyValues * pKeyValues) */

void __thiscall
CProjectedWallEntity_NonConformantDataHandler::HandleInstance
          (CProjectedWallEntity_NonConformantDataHandler *this,void *pEntity,KeyValues *pKeyValues)

{
  short *psVar1;
  int unaff_EBX;
  undefined4 *puVar2;
  
                    /* Unresolved local var: C_ProjectedWallEntity * pCastEntity@[???] */
  ___i686_get_pc_thunk_bx();
  C_ProjectedWallEntity::RestoreToToolRecordedState(pEntity,pKeyValues);
  psVar1 = (short *)(**(code **)(*(int *)pEntity + 0x360))(pEntity);
  if (*psVar1 == -1) {
    puVar2 = *(undefined4 **)(CPrediction::GetIncomingPacketNumber + unaff_EBX);
    (**(code **)(*(int *)*puVar2 + 0x40))((int *)*puVar2,(int)pEntity + 4,0,1,0xffffffff);
  }
  else {
    puVar2 = *(undefined4 **)(CPrediction::GetIncomingPacketNumber + unaff_EBX);
  }
                    /* WARNING: Could not recover jumptable at 0x00537719. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(int *)*puVar2 + 0x4c))();
  return;
}


/* C_TEWallPaintedEvent::~C_TEWallPaintedEvent at 00537640 */

/* DWARF original prototype: void ~C_TEWallPaintedEvent(C_TEWallPaintedEvent * this, int __in_chrg)
    */

void __thiscall
C_TEWallPaintedEvent::~C_TEWallPaintedEvent(C_TEWallPaintedEvent *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseTempEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x695177);
  (this->super_C_BaseTempEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6951fb);
  C_BaseTempEntity::~C_BaseTempEntity(&this->super_C_BaseTempEntity,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* C_TEWallPaintedEvent::~C_TEWallPaintedEvent at 00537690 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_TEWallPaintedEvent(C_TEWallPaintedEvent * this, int __in_chrg)
    */

void __thiscall
C_TEWallPaintedEvent::~C_TEWallPaintedEvent(C_TEWallPaintedEvent *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_C_BaseTempEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(CBaseWorldView::~CBaseWorldView + extraout_ECX);
  (this->super_C_BaseTempEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(extraout_ECX + 0x6951b4);
  C_BaseTempEntity::~C_BaseTempEntity(&this->super_C_BaseTempEntity,__in_chrg);
  return;
}


/* __tcf_8 at 0097c240 */

void __tcf_8(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  *(int *)(extraout_ECX + 0x356338) = extraout_ECX + 0x250580;
  *(int *)(extraout_ECX + 0x35633c) = extraout_ECX + 0x250604;
  C_BaseTempEntity::~C_BaseTempEntity
            ((C_BaseTempEntity *)(extraout_ECX + 0x356338),in_stack_00000008);
  return;
}


/* C_TEWallPaintedEvent::YouForgotToImplementOrDeclareClientClass at 0052f2e0 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_TEWallPaintedEvent *
   this) */

int __thiscall
C_TEWallPaintedEvent::YouForgotToImplementOrDeclareClientClass(C_TEWallPaintedEvent *this)

{
  return 0;
}


/* C_TEWallPaintedEvent::GetClientClass at 0052f300 */

/* DWARF original prototype: ClientClass * GetClientClass(C_TEWallPaintedEvent * this) */

ClientClass * __thiscall C_TEWallPaintedEvent::GetClientClass(C_TEWallPaintedEvent *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x83a170);
}


/* _C_TEWallPaintedEvent_CreateObject at 0052f310 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

IClientNetworkable * _C_TEWallPaintedEvent_CreateObject(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (IClientNetworkable *)(extraout_ECX + 0x7a326c);
}


/* ClientClassInit<DT_TEWallPaintedEvent::ignored> at 00077ea0 */

int ClientClassInit<DT_TEWallPaintedEvent::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&DAT_00c5a6ec)[unaff_EBX] == '\0') {
    iVar1 = ___cxa_guard_acquire(&DAT_00c5a6ec + unaff_EBX);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)((int)&BaseModUI::CUIGameData::m_Instance + unaff_EBX),
                  (char *)(unaff_EBX + 0x9128bc),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropEHandle((RecvProp *)(&BaseModUI::Achievements::ChainToMap()::chained + unaff_EBX),
                      (char *)(unaff_EBX + 0x943094),0x10,4,
                      *(RecvVarProxyFn *)(&DAT_00aebf34 + unaff_EBX));
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&BaseModUI::AddonAssociation::KB_ChainToMap()::chained + unaff_EBX + 2),
                  (char *)(unaff_EBX + 0x94309e),0x14,4,0,(RecvVarProxyFn)0x0);
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&BaseModUI::CShadowLabel::GetKBMap::s_pMap + unaff_EBX),
                  (char *)(unaff_EBX + 0x9430ab),0x18,4,0,(RecvVarProxyFn)0x0);
      ___cxa_guard_release(&DAT_00c5a6ec + unaff_EBX);
    }
  }
  RecvTable::Construct
            ((RecvTable *)(unaff_EBX + 0xcf15dc),
             (RecvProp_conflict1 *)(&BaseModUI::Achievements::ChainToMap()::chained + unaff_EBX),3,
             (char *)(unaff_EBX + 0x9430b6));
  return 1;
}


/* __tcf_9 at 0097bfa0 */

void __tcf_9(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(&UNK_003ed4e8 + extraout_ECX),in_stack_00000008);
  return;
}


/* _GLOBAL__I_cl_projected_wall_projection_speed at 000787f0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_cl_projected_wall_projection_speed(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

