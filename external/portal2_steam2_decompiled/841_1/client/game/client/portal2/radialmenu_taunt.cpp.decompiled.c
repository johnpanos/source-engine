/* DWARF-guided pseudocode for game/client/portal2/radialmenu_taunt.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 00081190 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  int unaff_EBX;
  longlong lVar1;
  
  lVar1 = ___i686_get_pc_thunk_bx();
  if (lVar1 == 0xffff00000001) {
    (&DAT_00c582cc)[unaff_EBX] = 0;
    (&DAT_00c582cd)[unaff_EBX] = 0;
    (&DAT_00c582ce)[unaff_EBX] = 0;
    (&DAT_00c582cf)[unaff_EBX] = 0;
    *(undefined4 *)(&DAT_00c582d0 + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00c582d4 + unaff_EBX) = 0;
    *(undefined4 *)((int)&vgui::MenuItem::GetMessageMap()::s_pMap + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00c582dc + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&vgui::MenuItem::GetKBMap()::s_pMap + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00c582e4 + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00c582e8 + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00c582ec + unaff_EBX) = 0;
    *(undefined4 *)((int)&vgui::AnimatingImagePanel::GetKBMap()::s_pMap + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00c582f4 + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00c582f8 + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00c582fc + unaff_EBX) = 0x7f7fffff;
    *(undefined **)((int)&vgui::ImagePanel::GetKBMap()::s_pMap + unaff_EBX) =
         &UNK_00ae3ce4 + unaff_EBX;
    *(undefined4 *)((int)ice_sbox[3] + unaff_EBX + 0xee0) = 0;
    *(int *)((int)ice_sbox[3] + unaff_EBX + 0xedc) = (int)&PTR_IsPerFrame_00b72fc4 + unaff_EBX;
    *(undefined4 *)((int)ice_sbox[3] + unaff_EBX + 0xf30) = 0;
    *(undefined4 *)((int)ice_sbox[3] + unaff_EBX + 0xf34) = 0;
    *(undefined4 *)((int)ice_sbox[3] + unaff_EBX + 0xf38) = 0;
    *(undefined4 *)((int)ice_sbox[3] + unaff_EBX + 0xf3c) = 0;
    *(undefined4 *)((int)ice_sbox[3] + unaff_EBX + 0xf40) = 0;
    *(undefined4 *)((int)ice_sbox[3] + unaff_EBX + 0xee4) = 0;
    ClientMenuManagerTaunt::ClearCustomMenuKeys
              ((ClientMenuManagerTaunt *)((int)ice_sbox[3] + unaff_EBX + 0xedc));
    *(undefined1 *)((int)ice_sbox[3] + unaff_EBX + 0xeec) = 0;
    *(undefined1 *)((int)ice_sbox[3] + unaff_EBX + 0xeed) = 1;
    *(undefined1 *)((int)ice_sbox[3] + unaff_EBX + 0xeee) = 0;
    *(undefined1 *)((int)ice_sbox[3] + unaff_EBX + 0xeef) = 0;
    ___cxa_atexit(unaff_EBX + 0x8fc1dc,0,*(undefined4 *)(&DAT_00ae2bc8 + unaff_EBX));
  }
  return;
}


/* __tcf_0 at 0097d380 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  *(int *)(unaff_EBX + 0x3ed90c) = unaff_EBX + 0x276dd4;
  if (*(KeyValues **)(unaff_EBX + 0x3ed914) != (KeyValues *)0x0) {
    KeyValues::deleteThis(*(KeyValues **)(unaff_EBX + 0x3ed914));
  }
  *(undefined4 *)(unaff_EBX + 0x3ed914) = 0;
  *(undefined4 *)(unaff_EBX + 0x3ed96c) = 0;
  if (*(int *)(unaff_EBX + 0x3ed968) < 0) {
    *(undefined4 *)(unaff_EBX + 0x3ed970) = *(undefined4 *)(unaff_EBX + 0x3ed960);
  }
  else {
    if (*(int *)(unaff_EBX + 0x3ed960) == 0) {
      *(undefined4 *)(unaff_EBX + 0x3ed964) = 0;
      *(undefined4 *)(unaff_EBX + 0x3ed970) = 0;
    }
    else {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1e69dc) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1e69dc),*(int *)(unaff_EBX + 0x3ed960));
      *(undefined4 *)(unaff_EBX + 0x3ed960) = 0;
      *(undefined4 *)(unaff_EBX + 0x3ed964) = 0;
      *(undefined4 *)(unaff_EBX + 0x3ed970) = 0;
      if (*(int *)(unaff_EBX + 0x3ed968) < 0) goto LAB_0097d416;
    }
    *(undefined4 *)(unaff_EBX + 0x3ed964) = 0;
  }
LAB_0097d416:
  *(int *)(unaff_EBX + 0x3ed90c) = *(int *)(unaff_EBX + 0x1e731c) + 8;
  if (*(KeyValues **)(unaff_EBX + 0x3ed910) != (KeyValues *)0x0) {
    KeyValues::deleteThis(*(KeyValues **)(unaff_EBX + 0x3ed910));
  }
  *(undefined4 *)(unaff_EBX + 0x3ed910) = 0;
  return;
}


/* ClientMenuManagerTaunt::ClearTauntStatusData at 00620b00 */

/* DWARF original prototype: void ClearTauntStatusData(ClientMenuManagerTaunt * this) */

void __thiscall ClientMenuManagerTaunt::ClearTauntStatusData(ClientMenuManagerTaunt *this)

{
  char *pcVar1;
  TauntStatusData *pTVar2;
  int iVar3;
  int unaff_EBX;
  int iVar4;
  int i;
  
  ___i686_get_pc_thunk_bx();
  if (0 < (this->m_TauntData).m_Size) {
    i = 0;
    iVar4 = 0;
    do {
                    /* Unresolved local var: TauntStatusData * pNew@[???] */
      pTVar2 = (this->m_TauntData).m_Memory.m_pMemory;
      iVar3 = *(int *)(pTVar2->szPosition + iVar4 + -5);
      if (iVar3 != -1) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5436e6) + 0x234))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x5436e6),iVar3);
        pcVar1 = pTVar2->szPosition + iVar4 + -5;
        pcVar1[0] = -1;
        pcVar1[1] = -1;
        pcVar1[2] = -1;
        pcVar1[3] = -1;
      }
      i = i + 1;
      iVar4 = iVar4 + 0x8c;
    } while (i < (this->m_TauntData).m_Size);
  }
  (this->m_TauntData).m_Size = 0;
  return;
}


/* ClientMenuManagerTaunt::Flush at 00620f50 */

/* DWARF original prototype: void Flush(ClientMenuManagerTaunt * this) */

void __thiscall ClientMenuManagerTaunt::Flush(ClientMenuManagerTaunt *this)

{
  KeyValues *this_00;
  int *piVar1;
  code *pcVar2;
  uint uVar3;
  char *pcVar4;
  int iVar5;
  int unaff_EBX;
  TauntStatusData *pTVar6;
  int iVar7;
  KeyValues *pKVData;
  int local_24;
  
  ___i686_get_pc_thunk_bx();
  this_00 = (this->super_ClientMenuManager).m_menuKeys;
  if (this_00 != (KeyValues *)0x0) {
    KeyValues::deleteThis(this_00);
  }
  (this->super_ClientMenuManager).m_menuKeys = (KeyValues *)0x0;
  ClientMenuManager::AddMenuFile(&this->super_ClientMenuManager,(char *)(unaff_EBX + 0x3a9cdb));
                    /* Unresolved local var: int i@[???] */
  if (0 < (this->m_TauntData).m_Size) {
    local_24 = 0;
    iVar7 = 0;
    do {
                    /* Unresolved local var: TauntStatusData * pNew@[???] */
      pTVar6 = (this->m_TauntData).m_Memory.m_pMemory;
      iVar5 = *(int *)(pTVar6->szPosition + iVar7 + -5);
      if (iVar5 != -1) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x543296) + 0x234))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x543296),iVar5);
        pcVar4 = pTVar6->szPosition + iVar7 + -5;
        pcVar4[0] = -1;
        pcVar4[1] = -1;
        pcVar4[2] = -1;
        pcVar4[3] = -1;
      }
      local_24 = local_24 + 1;
      iVar7 = iVar7 + 0x8c;
    } while (local_24 < (this->m_TauntData).m_Size);
  }
  (this->m_TauntData).m_Size = 0;
  pKVData = KeyValues::GetFirstTrueSubKey((this->super_ClientMenuManager).m_menuKeys);
  if (pKVData != (KeyValues *)0x0) {
    do {
      uVar3 = (this->m_TauntData).m_Size;
      iVar7 = uVar3 + 1;
      iVar5 = (this->m_TauntData).m_Memory.m_nAllocationCount;
      if (iVar5 < iVar7) {
        CUtlMemory<TauntStatusData,int>::Grow(&(this->m_TauntData).m_Memory,iVar7 - iVar5);
      }
      piVar1 = &(this->m_TauntData).m_Size;
      *piVar1 = *piVar1 + 1;
      pTVar6 = (this->m_TauntData).m_Memory.m_pMemory;
      (this->m_TauntData).m_pElements = pTVar6;
      iVar5 = ~uVar3 + (this->m_TauntData).m_Size;
      if (0 < iVar5) {
        _V_memmove(pTVar6 + iVar7,pTVar6 + uVar3,iVar5 * 0x8c);
      }
      pTVar6 = (this->m_TauntData).m_Memory.m_pMemory + uVar3;
      pcVar4 = KeyValues::GetName(pKVData);
      V_strncpy(pTVar6->szName,pcVar4,0x10);
      pcVar4 = KeyValues::GetString
                         (pKVData,(char *)(unaff_EBX + 0x38d7e2),(char *)(unaff_EBX + 0x367d06));
      V_strncpy(pTVar6->szCommand,pcVar4,0x20);
      pcVar4 = KeyValues::GetString
                         (pKVData,(char *)(C_TEBeamRing::GetClientClass + unaff_EBX + 2),
                          (char *)(unaff_EBX + 0x367d06));
      V_strncpy(pTVar6->szIcon,pcVar4,0x40);
      iVar7 = KeyValues::GetInt(pKVData,(char *)(unaff_EBX + 0x38440e),0);
      pTVar6->bTeamTaunt = iVar7 != 0;
      iVar7 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x543296) + 0xa4))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x543296),0);
      pTVar6->nTextureID = iVar7;
      piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x543296);
      pcVar2 = *(code **)(*piVar1 + 0x8c);
      pcVar4 = VarArgs((char *)(unaff_EBX + 0x3bfb1a),pTVar6->szIcon);
      (*pcVar2)(piVar1,pTVar6->nTextureID,pcVar4,1,0);
      V_strncpy(pTVar6->szPosition,(char *)(unaff_EBX + 0x374122),0x10);
      pTVar6->bUnlocked = false;
      pTVar6->bOwned = false;
      pTVar6->bUsed = false;
      pTVar6->nCompletionDelay = 0;
      pKVData = KeyValues::GetNextTrueSubKey(pKVData);
    } while (pKVData != (KeyValues *)0x0);
  }
  return;
}


/* ClientMenuManagerTaunt::FindMenu at 00621930 */

/* DWARF original prototype: KeyValues * FindMenu(ClientMenuManagerTaunt * this, char * menuName) */

KeyValues * __thiscall ClientMenuManagerTaunt::FindMenu(ClientMenuManagerTaunt *this,char *menuName)

{
  KeyValues *pKVar1;
  
  UpdateDefaultTaunts(this);
  pKVar1 = KeyValues::FindKey(this->m_customMenuKeys,menuName,false);
  return pKVar1;
}


/* ClientMenuManagerTaunt::KeyValueBuilder at 00620d20 */

/* DWARF original prototype: void KeyValueBuilder(ClientMenuManagerTaunt * this, KeyValues *
   pKeyValues) */

void __thiscall
ClientMenuManagerTaunt::KeyValueBuilder(ClientMenuManagerTaunt *this,KeyValues *pKeyValues)

{
  KeyValues *this_00;
  int unaff_EBX;
  char *setName;
  int local_24;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  if (0 < (this->m_TauntData).m_Size) {
    local_24 = 0;
    local_20 = 0;
    do {
      setName = ((this->m_TauntData).m_Memory.m_pMemory)->szName + local_20;
      this_00 = KeyValues::operator_new(0x24);
      KeyValues::KeyValues(this_00,setName);
      KeyValues::SetInt(this_00,(char *)(unaff_EBX + 0x36f6cb),(uint)(byte)setName[0x85]);
      KeyValues::SetInt(this_00,(char *)(unaff_EBX + 0x36f6d4),(uint)(byte)setName[0x86]);
      KeyValues::SetInt(this_00,(char *)(unaff_EBX + 0x36f6da),(uint)(byte)setName[0x87]);
      KeyValues::SetInt(this_00,(char *)(unaff_EBX + 0x36f6df),*(int *)(setName + 0x88));
      KeyValues::SetString(this_00,(char *)(unaff_EBX + 0x36f6e6),setName + 0x75);
      KeyValues::AddSubKey(pKeyValues,this_00);
      local_24 = local_24 + 1;
      local_20 = local_20 + 0x8c;
    } while (local_24 < (this->m_TauntData).m_Size);
  }
  return;
}


/* ClientMenuManagerTaunt::GetNextTauntToEarn at 00620ca0 */

/* DWARF original prototype: char * GetNextTauntToEarn(ClientMenuManagerTaunt * this) */

char * __thiscall ClientMenuManagerTaunt::GetNextTauntToEarn(ClientMenuManagerTaunt *this)

{
  int iVar1;
  TauntStatusData *pTVar2;
  int iVar3;
  TauntStatusData *pTVar4;
  TauntStatusData *pTVar5;
  
                    /* Unresolved local var: TauntStatusData * pNext@[???] */
  iVar1 = (this->m_TauntData).m_Size;
  if (0 < iVar1) {
    iVar3 = 0;
    pTVar2 = (this->m_TauntData).m_Memory.m_pMemory;
    pTVar4 = (TauntStatusData *)0x0;
    do {
      while ((((pTVar5 = pTVar2, pTVar5->bUnlocked != false && (pTVar5->bOwned == false)) &&
              (pTVar5->nCompletionDelay == 0)) &&
             ((pTVar4 == (TauntStatusData *)0x0 ||
              ((pTVar4->bTeamTaunt == false && (pTVar5->bTeamTaunt != false))))))) {
        iVar3 = iVar3 + 1;
        pTVar2 = pTVar5 + 1;
        pTVar4 = pTVar5;
        if (iVar3 == iVar1) goto LAB_00620cfb;
      }
      iVar3 = iVar3 + 1;
      pTVar2 = pTVar5 + 1;
      pTVar5 = pTVar4;
    } while (iVar3 != iVar1);
LAB_00620cfb:
    if (pTVar5 != (TauntStatusData *)0x0) {
      return pTVar5->szName;
    }
  }
  return (char *)0x0;
}


/* ClientMenuManagerTaunt::IsTauntTeam at 00621c40 */

/* DWARF original prototype: bool IsTauntTeam(ClientMenuManagerTaunt * this, char * pchTaunt) */

bool __thiscall ClientMenuManagerTaunt::IsTauntTeam(ClientMenuManagerTaunt *this,char *pchTaunt)

{
  int iVar1;
  char *pcVar2;
  int iVar3;
  int iVar4;
  
                    /* Unresolved local var: TauntStatusData * pTauntData@[???] */
  if (0 < (this->m_TauntData).m_Size) {
    iVar4 = 0;
    iVar3 = 0;
    do {
      iVar1 = _V_strcmp(((this->m_TauntData).m_Memory.m_pMemory)->szName + iVar3,pchTaunt);
      if (iVar1 == 0) {
        pcVar2 = ((this->m_TauntData).m_Memory.m_pMemory)->szName + iVar3;
        if (pcVar2 == (char *)0x0) {
          return false;
        }
        return (bool)pcVar2[0x74];
      }
      iVar4 = iVar4 + 1;
      iVar3 = iVar3 + 0x8c;
    } while (iVar4 < (this->m_TauntData).m_Size);
  }
  return false;
}


/* ClientMenuManagerTaunt::SetTauntUnlocked at 00621a90 */

/* DWARF original prototype: void SetTauntUnlocked(ClientMenuManagerTaunt * this, char * pchTaunt)
    */

void __thiscall
ClientMenuManagerTaunt::SetTauntUnlocked(ClientMenuManagerTaunt *this,char *pchTaunt)

{
  code *pcVar1;
  int iVar2;
  char *pcVar3;
  C_KeyValueSaver *this_00;
  TauntStatusData *pTVar4;
  int unaff_EBX;
  int iVar5;
  TauntStatusData *pTVar6;
  int iVar7;
  int local_20;
  
                    /* Unresolved local var: TauntStatusData * pTauntData@[???] */
  ___i686_get_pc_thunk_bx();
  if (0 < (this->m_TauntData).m_Size) {
    iVar7 = 0;
    iVar5 = 0;
    do {
      iVar2 = _V_strcmp(((this->m_TauntData).m_Memory.m_pMemory)->szName + iVar5,pchTaunt);
      if (iVar2 == 0) {
        pTVar6 = (TauntStatusData *)(((this->m_TauntData).m_Memory.m_pMemory)->szName + iVar5);
        if (pTVar6 == (TauntStatusData *)0x0) {
          return;
        }
        if (pTVar6->bUnlocked != false) {
          return;
        }
        pTVar6->bUnlocked = true;
        if (pTVar6->bTeamTaunt == false) {
          iVar5 = (this->m_TauntData).m_Size;
          local_20 = 0;
          if (0 < iVar5) {
            pTVar4 = (this->m_TauntData).m_Memory.m_pMemory;
            local_20 = -3;
            iVar7 = 0;
            do {
                    /* Unresolved local var: TauntStatusData * pOtherTauntData@[???] */
              iVar2 = local_20;
              if ((((pTVar6 != pTVar4) && (pTVar4->bUnlocked != false)) && (pTVar4->bOwned == false)
                  ) && (iVar2 = pTVar4->nCompletionDelay, pTVar4->nCompletionDelay <= local_20)) {
                iVar2 = local_20;
              }
              local_20 = iVar2;
              iVar7 = iVar7 + 1;
              pTVar4 = pTVar4 + 1;
            } while (iVar7 != iVar5);
            local_20 = local_20 + 3;
          }
          pTVar6->nCompletionDelay = local_20;
        }
        else {
          pTVar6->nCompletionDelay = 0;
          pcVar1 = *(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x542306) + 0x1c);
                    /* Unresolved local var: int nNumTauntsPending@[???]
                       Unresolved local var: int i@[???] */
          iVar5 = 0;
          if (0 < *(int *)(unaff_EBX + 0x749262)) {
            iVar7 = *(int *)(unaff_EBX + 0x749256);
            iVar2 = 0;
            do {
                    /* Unresolved local var: TauntStatusData * pTauntData@[???] */
              if ((*(char *)(iVar7 + 0x85) != '\0') && (*(char *)(iVar7 + 0x86) == '\0')) {
                iVar5 = iVar5 + (uint)(*(int *)(iVar7 + 0x88) == 0);
              }
              iVar2 = iVar2 + 1;
              iVar7 = iVar7 + 0x8c;
            } while (iVar2 != *(int *)(unaff_EBX + 0x749262));
          }
          pcVar3 = VarArgs((char *)(unaff_EBX + 0x36e97f),iVar5);
          (*pcVar1)(**(undefined4 **)(unaff_EBX + 0x542306),pcVar3);
        }
        this_00 = KeyValueSaver();
        C_KeyValueSaver::MarkKeyValuesDirty(this_00,(char *)(unaff_EBX + 0x36e7f7));
        this->m_bLoadedSaveData = true;
        return;
      }
      iVar7 = iVar7 + 1;
      iVar5 = iVar5 + 0x8c;
    } while (iVar7 < (this->m_TauntData).m_Size);
  }
  return;
}


/* ClientMenuManagerTaunt::SetTauntOwned at 00621a00 */

/* DWARF original prototype: void SetTauntOwned(ClientMenuManagerTaunt * this, char * pchTaunt) */

void __thiscall ClientMenuManagerTaunt::SetTauntOwned(ClientMenuManagerTaunt *this,char *pchTaunt)

{
  int iVar1;
  char *pcVar2;
  C_KeyValueSaver *this_00;
  int unaff_EBX;
  int iVar3;
  int iVar4;
  
                    /* Unresolved local var: TauntStatusData * pTauntData@[???] */
  ___i686_get_pc_thunk_bx();
  if (0 < (this->m_TauntData).m_Size) {
    iVar4 = 0;
    iVar3 = 0;
    do {
      iVar1 = _V_strcmp(((this->m_TauntData).m_Memory.m_pMemory)->szName + iVar3,pchTaunt);
      if (iVar1 == 0) {
        pcVar2 = ((this->m_TauntData).m_Memory.m_pMemory)->szName + iVar3;
        if (pcVar2 == (char *)0x0) {
          return;
        }
        if ((bool)pcVar2[0x86] != false) {
          return;
        }
        pcVar2[0x86] = true;
        this_00 = KeyValueSaver();
        C_KeyValueSaver::MarkKeyValuesDirty(this_00,(char *)(unaff_EBX + 0x36e887));
        this->m_bLoadedSaveData = true;
        return;
      }
      iVar4 = iVar4 + 1;
      iVar3 = iVar3 + 0x8c;
    } while (iVar4 < (this->m_TauntData).m_Size);
  }
  return;
}


/* ClientMenuManagerTaunt::DecreaseOwnershipDelays at 00621f30 */

/* DWARF original prototype: void DecreaseOwnershipDelays(ClientMenuManagerTaunt * this) */

void __thiscall ClientMenuManagerTaunt::DecreaseOwnershipDelays(ClientMenuManagerTaunt *this)

{
  code *pcVar1;
  bool bVar2;
  bool bVar3;
  int iVar4;
  int iVar5;
  char *pcVar6;
  C_KeyValueSaver *pCVar7;
  char *pcVar8;
  int iVar9;
  int unaff_EBX;
  bool bPending;
  int local_28;
  int local_24;
  int local_20;
  
                    /* Unresolved local var: bool bChange@[???] */
  ___i686_get_pc_thunk_bx();
  if (0 < (this->m_TauntData).m_Size) {
    bVar2 = false;
    bVar3 = false;
    local_28 = 0;
    local_20 = 0;
    do {
      pcVar6 = ((this->m_TauntData).m_Memory.m_pMemory)->szName + local_20;
      if (((bool)pcVar6[0x86] == false) && (0 < *(int *)(pcVar6 + 0x88))) {
        iVar4 = *(int *)(pcVar6 + 0x88) + -1;
        *(int *)(pcVar6 + 0x88) = iVar4;
        bVar2 = true;
        if (iVar4 == 0) {
          if ((bool)pcVar6[0x74] == false) {
            if (0 < (this->m_TauntData).m_Size) {
              local_24 = 0;
              iVar4 = 0;
              do {
                iVar5 = _V_strcmp(((this->m_TauntData).m_Memory.m_pMemory)->szName + iVar4,pcVar6);
                if (iVar5 == 0) {
                  pcVar8 = ((this->m_TauntData).m_Memory.m_pMemory)->szName + iVar4;
                  if ((pcVar8 != (char *)0x0) && ((bool)pcVar8[0x86] == false)) {
                    pcVar8[0x86] = true;
                    pCVar7 = KeyValueSaver();
                    C_KeyValueSaver::MarkKeyValuesDirty(pCVar7,(char *)(unaff_EBX + 0x36e357));
                    this->m_bLoadedSaveData = true;
                  }
                  break;
                }
                local_24 = local_24 + 1;
                iVar4 = iVar4 + 0x8c;
              } while (local_24 < (this->m_TauntData).m_Size);
            }
            SetTauntPosition((ClientMenuManagerTaunt *)
                             (CChoreoScene::RemoveEventsExceptTypes + unaff_EBX + 2),pcVar6,
                             (char *)(unaff_EBX + 0x3a8496));
            bVar2 = true;
          }
          else {
            bVar3 = true;
          }
        }
      }
      local_28 = local_28 + 1;
      local_20 = local_20 + 0x8c;
    } while (local_28 < (this->m_TauntData).m_Size);
    if (bVar2) {
      if (bVar3) {
        pcVar1 = *(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x541e66) + 0x1c);
                    /* Unresolved local var: int nNumTauntsPending@[???]
                       Unresolved local var: int i@[???] */
        iVar4 = 0;
        if (0 < *(int *)(unaff_EBX + 0x748dc2)) {
          iVar5 = *(int *)(unaff_EBX + 0x748db6);
          iVar9 = 0;
          do {
                    /* Unresolved local var: TauntStatusData * pTauntData@[???] */
            if ((*(char *)(iVar5 + 0x85) != '\0') && (*(char *)(iVar5 + 0x86) == '\0')) {
              iVar4 = iVar4 + (uint)(*(int *)(iVar5 + 0x88) == 0);
            }
            iVar9 = iVar9 + 1;
            iVar5 = iVar5 + 0x8c;
          } while (iVar9 != *(int *)(unaff_EBX + 0x748dc2));
        }
        pcVar6 = VarArgs((char *)(unaff_EBX + 0x36e4df),iVar4);
        (*pcVar1)(**(undefined4 **)(unaff_EBX + 0x541e66),pcVar6);
      }
      pCVar7 = KeyValueSaver();
      C_KeyValueSaver::MarkKeyValuesDirty(pCVar7,(char *)(unaff_EBX + 0x36e357));
    }
  }
  return;
}


/* ClientMenuManagerTaunt::SetTauntUsed at 00621960 */

/* DWARF original prototype: void SetTauntUsed(ClientMenuManagerTaunt * this, char * pchTaunt) */

void __thiscall ClientMenuManagerTaunt::SetTauntUsed(ClientMenuManagerTaunt *this,char *pchTaunt)

{
  int iVar1;
  char *pcVar2;
  C_KeyValueSaver *this_00;
  int unaff_EBX;
  int iVar3;
  int iVar4;
  
                    /* Unresolved local var: TauntStatusData * pTauntData@[???] */
  ___i686_get_pc_thunk_bx();
  if (0 < (this->m_TauntData).m_Size) {
    iVar4 = 0;
    iVar3 = 0;
    do {
      iVar1 = _V_strcmp(((this->m_TauntData).m_Memory.m_pMemory)->szName + iVar3,pchTaunt);
      if (iVar1 == 0) {
        pcVar2 = ((this->m_TauntData).m_Memory.m_pMemory)->szName + iVar3;
        if (pcVar2 == (char *)0x0) {
          return;
        }
        if ((bool)pcVar2[0x87] != false) {
          return;
        }
        this->m_bMenuStale = true;
        pcVar2[0x87] = true;
        this_00 = KeyValueSaver();
        C_KeyValueSaver::MarkKeyValuesDirty(this_00,(char *)(unaff_EBX + 0x36e927));
        this->m_bLoadedSaveData = true;
        return;
      }
      iVar4 = iVar4 + 1;
      iVar3 = iVar3 + 0x8c;
    } while (iVar4 < (this->m_TauntData).m_Size);
  }
  return;
}


/* ClientMenuManagerTaunt::SetTauntDelay at 00621ea0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void SetTauntDelay(ClientMenuManagerTaunt * this, char * pchTaunt, int
   nDelay) */

void __thiscall
ClientMenuManagerTaunt::SetTauntDelay(ClientMenuManagerTaunt *this,char *pchTaunt,int nDelay)

{
  int iVar1;
  char *pcVar2;
  C_KeyValueSaver *this_00;
  int unaff_EBX;
  int iVar3;
  int iVar4;
  
                    /* Unresolved local var: TauntStatusData * pTauntData@[???] */
  ___i686_get_pc_thunk_bx();
  if (0 < (this->m_TauntData).m_Size) {
    iVar4 = 0;
    iVar3 = 0;
    do {
      iVar1 = _V_strcmp(((this->m_TauntData).m_Memory.m_pMemory)->szName + iVar3,pchTaunt);
      if (iVar1 == 0) {
        pcVar2 = ((this->m_TauntData).m_Memory.m_pMemory)->szName + iVar3;
        if (pcVar2 == (char *)0x0) {
          return;
        }
        if (*(int *)(pcVar2 + 0x88) == nDelay) {
          return;
        }
        *(int *)(pcVar2 + 0x88) = nDelay;
        this_00 = KeyValueSaver();
        C_KeyValueSaver::MarkKeyValuesDirty
                  (this_00,(char *)(SporeEffect::UpdateColor + unaff_EBX + 7));
        this->m_bLoadedSaveData = true;
        return;
      }
      iVar4 = iVar4 + 1;
      iVar3 = iVar3 + 0x8c;
    } while (iVar4 < (this->m_TauntData).m_Size);
  }
  return;
}


/* ClientMenuManagerTaunt::SetTauntPosition at 00621cb0 */

/* DWARF original prototype: void SetTauntPosition(ClientMenuManagerTaunt * this, char * pchTaunt,
   char * pchPosition) */

void __thiscall
ClientMenuManagerTaunt::SetTauntPosition
          (ClientMenuManagerTaunt *this,char *pchTaunt,char *pchPosition)

{
  int iVar1;
  C_KeyValueSaver *this_00;
  int unaff_EBX;
  int iVar2;
  char *pcVar3;
  int iVar4;
  
                    /* Unresolved local var: TauntStatusData * pTauntData@[???]
                       Unresolved local var: bool bMoveToEmpty@[???] */
  ___i686_get_pc_thunk_bx();
  if ((this->m_TauntData).m_Size < 1) {
    return;
  }
  iVar4 = 0;
  iVar2 = 0;
  while (iVar1 = _V_strcmp(((this->m_TauntData).m_Memory.m_pMemory)->szName + iVar2,pchTaunt),
        iVar1 != 0) {
    iVar4 = iVar4 + 1;
    iVar2 = iVar2 + 0x8c;
    if ((this->m_TauntData).m_Size <= iVar4) {
      return;
    }
  }
  pcVar3 = ((this->m_TauntData).m_Memory.m_pMemory)->szName + iVar2;
  if (pcVar3 == (char *)0x0) {
    return;
  }
  iVar2 = _V_strcmp(pchPosition,(char *)(unaff_EBX + 0x3a8716));
  pcVar3 = pcVar3 + 0x75;
  iVar4 = _V_strcmp(pcVar3,pchPosition);
  if (iVar4 == 0) {
    return;
  }
  if (iVar2 == 0) {
    pchPosition = (char *)(unaff_EBX + 0x3733c2);
    iVar2 = _V_strcmp(pcVar3,pchPosition);
    if (iVar2 != 0) {
      return;
    }
    iVar2 = 0;
    do {
                    /* Unresolved local var: bool bNotTaken@[???]
                       Unresolved local var: int j@[???] */
      iVar4 = 0;
      if ((this->m_TauntData).m_Size < 1) {
LAB_00621e75:
        pchPosition = (char *)(iVar2 * 0x10 + unaff_EBX + 0x60bda2);
        break;
      }
      while (iVar1 = _V_strcmp((this->m_TauntData).m_Memory.m_pMemory[iVar4].szPosition,
                               (char *)(iVar2 * 0x10 + unaff_EBX + 0x60bda2)), iVar1 != 0) {
        iVar4 = iVar4 + 1;
        if ((this->m_TauntData).m_Size <= iVar4) goto LAB_00621e75;
      }
      iVar2 = iVar2 + 1;
    } while (iVar2 < 8);
  }
  else if (0 < (this->m_TauntData).m_Size) {
    iVar2 = 0;
    iVar4 = 0;
    do {
      iVar1 = _V_strcmp(((this->m_TauntData).m_Memory.m_pMemory)->szPosition + iVar4,pchPosition);
      if (iVar1 == 0) {
        V_strncpy(((this->m_TauntData).m_Memory.m_pMemory)->szPosition + iVar4,
                  (char *)(unaff_EBX + 0x3733c2),0x10);
        break;
      }
      iVar2 = iVar2 + 1;
      iVar4 = iVar4 + 0x8c;
    } while (iVar2 < (this->m_TauntData).m_Size);
  }
  V_strncpy(pcVar3,pchPosition,0x10);
  this_00 = KeyValueSaver();
  C_KeyValueSaver::MarkKeyValuesDirty(this_00,(char *)(unaff_EBX + 0x36e5d7));
  this->m_bMenuStale = true;
  this->m_bLoadedSaveData = true;
  return;
}


/* ClientMenuManagerTaunt::GetNumTauntsOwned at 00620f10 */

/* DWARF original prototype: int GetNumTauntsOwned(ClientMenuManagerTaunt * this) */

int __thiscall ClientMenuManagerTaunt::GetNumTauntsOwned(ClientMenuManagerTaunt *this)

{
  int iVar1;
  int iVar2;
  TauntStatusData *pTVar3;
  int iVar4;
  
                    /* Unresolved local var: int nNumTauntsOwned@[???] */
                    /* Unresolved local var: int i@[DW_OP_reg1(ECX)] */
  iVar1 = (this->m_TauntData).m_Size;
  iVar4 = 0;
  if (0 < iVar1) {
    pTVar3 = (this->m_TauntData).m_Memory.m_pMemory;
    iVar2 = 0;
    do {
                    /* Unresolved local var: TauntStatusData * pTauntData@[???] */
      iVar4 = (iVar4 + 1) - (uint)(pTVar3->bOwned == false);
      iVar2 = iVar2 + 1;
      pTVar3 = pTVar3 + 1;
    } while (iVar2 != iVar1);
  }
  return iVar4;
}


/* ClientMenuManagerTaunt::GetNumTauntsPending at 00620b90 */

/* DWARF original prototype: int GetNumTauntsPending(ClientMenuManagerTaunt * this) */

int __thiscall ClientMenuManagerTaunt::GetNumTauntsPending(ClientMenuManagerTaunt *this)

{
  int iVar1;
  int iVar2;
  TauntStatusData *pTVar3;
  int iVar4;
  
                    /* Unresolved local var: int nNumTauntsPending@[???] */
                    /* Unresolved local var: int i@[DW_OP_reg1(ECX)] */
  iVar1 = (this->m_TauntData).m_Size;
  iVar4 = 0;
  if (0 < iVar1) {
    pTVar3 = (this->m_TauntData).m_Memory.m_pMemory;
    iVar2 = 0;
    do {
                    /* Unresolved local var: TauntStatusData * pTauntData@[???] */
      if ((pTVar3->bUnlocked != false) && (pTVar3->bOwned == false)) {
        iVar4 = iVar4 + (uint)(pTVar3->nCompletionDelay == 0);
      }
      iVar2 = iVar2 + 1;
      pTVar3 = pTVar3 + 1;
    } while (iVar2 != iVar1);
  }
  return iVar4;
}


/* ClientMenuManagerTaunt::UpdateDefaultTaunts at 00621280 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void UpdateDefaultTaunts(ClientMenuManagerTaunt * this) */

void __thiscall ClientMenuManagerTaunt::UpdateDefaultTaunts(ClientMenuManagerTaunt *this)

{
  char cVar1;
  _func_int_varargs **pp_Var2;
  code *pcVar3;
  TauntStatusData *pTVar4;
  bool bVar5;
  undefined4 uVar6;
  TauntStatusData *pTVar7;
  int iVar8;
  C_BasePlayer *pCVar9;
  KeyValues *pKVar10;
  KeyValues *pKVar11;
  C_KeyValueSaver *this_00;
  char *pcVar12;
  int iVar13;
  int iVar14;
  int unaff_EBX;
  TauntStatusData *pTVar15;
  int local_40;
  bool local_39;
  int local_38;
  int local_30;
  undefined4 local_24;
  undefined4 local_20;
  
                    /* Unresolved local var: C_Portal_Player * pPlayer@[???]
                       Unresolved local var: C_Portal_Player * pPartnerPlayer@[???]
                       Unresolved local var: bool bTeamAccept@[???]
                       Unresolved local var: bool[8] bPositionsFilled@[???] */
  ___i686_get_pc_thunk_bx();
  iVar8 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x542b13) + 0x1f8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x542b13));
  pCVar9 = C_BasePlayer::GetLocalPlayer(iVar8);
  if (pCVar9 == (C_BasePlayer *)0x0) {
    local_40 = 0;
    local_39 = false;
  }
  else {
    pp_Var2 = pCVar9[1].super_C_BaseCombatCharacter.super_C_BaseFlex.super_C_BaseAnimatingOverlay.
              super_C_BaseAnimating.super_C_BaseEntity.m_iv_vecOrigin.
              super_CInterpolatedVarArrayBase<Vector,false>.super_IInterpolatedVar.
              _vptr_IInterpolatedVar;
    if ((pp_Var2 == (_func_int_varargs **)0xffffffff) ||
       (iVar8 = ((uint)pp_Var2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x542b4b),
       *(uint *)(iVar8 + 8) != (uint)pp_Var2 >> 0x10)) {
      local_40 = 0;
    }
    else {
      local_40 = *(int *)(iVar8 + 4);
    }
    local_39 = local_40 != 0;
  }
  if ((this->m_bOldPartnerInRange == local_39) &&
     ((local_39 == false ||
      (iVar8 = _V_strcmp(this->m_bOldPartnerForceTauntName,(char *)(local_40 + 0x1f18)), iVar8 == 0)
      ))) {
    if (this->m_bMenuStale == false) {
      return;
    }
    cVar1 = this->m_bLoadedSaveData;
  }
  else {
    this->m_bOldPartnerInRange = local_39;
    pcVar12 = (char *)(unaff_EBX + 0x3679d3);
    if (local_39 != false) {
      pcVar12 = (char *)(local_40 + 0x1f18);
    }
    V_strncpy(this->m_bOldPartnerForceTauntName,pcVar12,0x40);
    this->m_bMenuStale = true;
    cVar1 = this->m_bLoadedSaveData;
  }
  if (cVar1 == '\0') {
    this->m_bLoadedSaveData = true;
    this_00 = KeyValueSaver();
    C_KeyValueSaver::MarkKeyValuesDirty(this_00,(char *)(unaff_EBX + 0x36f004));
                    /* Unresolved local var: int i@[???] */
    if (0 < (this->m_TauntData).m_Size) {
                    /* Unresolved local var: TauntStatusData * pTauntData@[???] */
      pTVar15 = (this->m_TauntData).m_Memory.m_pMemory;
      pTVar15->bUnlocked = true;
      pTVar15->bOwned = true;
      pTVar15->bUsed = true;
      pTVar15->nCompletionDelay = 0;
      V_strncpy(pTVar15->szPosition,(char *)(unaff_EBX + 0x60c7cf),0x10);
                    /* Unresolved local var: int i@[???] */
      iVar8 = (this->m_TauntData).m_Size;
      if (0 < iVar8) {
        pTVar15 = (this->m_TauntData).m_Memory.m_pMemory;
        if (pTVar15->bTeamTaunt == false) {
          iVar13 = 0;
          pTVar4 = pTVar15;
          do {
            pTVar15 = pTVar4 + 1;
            iVar13 = iVar13 + 1;
            if (iVar13 == iVar8) goto LAB_00621341;
            pTVar7 = pTVar4 + 1;
            pTVar4 = pTVar15;
          } while (pTVar7->bTeamTaunt == false);
        }
        pTVar15->bUnlocked = true;
        pTVar15->bOwned = false;
        pTVar15->bUsed = false;
        pcVar3 = *(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x542b13) + 0x1c);
                    /* Unresolved local var: int nNumTauntsPending@[???]
                       Unresolved local var: int i@[???] */
        iVar8 = 0;
        if (0 < *(int *)(unaff_EBX + 0x749a6f)) {
          iVar13 = *(int *)(unaff_EBX + 0x749a63);
          iVar14 = 0;
          do {
                    /* Unresolved local var: TauntStatusData * pTauntData@[???] */
            if ((*(char *)(iVar13 + 0x85) != '\0') && (*(char *)(iVar13 + 0x86) == '\0')) {
              iVar8 = iVar8 + (uint)(*(int *)(iVar13 + 0x88) == 0);
            }
            iVar14 = iVar14 + 1;
            iVar13 = iVar13 + 0x8c;
          } while (iVar14 != *(int *)(unaff_EBX + 0x749a6f));
        }
        pcVar12 = VarArgs((char *)(unaff_EBX + 0x36f18c),iVar8);
        (*pcVar3)(**(undefined4 **)(unaff_EBX + 0x542b13),pcVar12);
      }
    }
  }
LAB_00621341:
  ClearCustomMenuKeys(this);
  local_24 = 0;
  local_20 = 0;
  if (0 < (this->m_TauntData).m_Size) {
    local_38 = 0;
    local_30 = 0;
    do {
      pTVar15 = (this->m_TauntData).m_Memory.m_pMemory;
      if (local_39 == false) {
LAB_006214c1:
        bVar5 = false;
      }
      else {
        iVar8 = _V_strlen((char *)(unaff_EBX + 0x3a99c4));
        iVar8 = _V_strcmp((char *)(local_40 + 0x1f18),pTVar15->szCommand + iVar8 + local_30);
        bVar5 = true;
        if (iVar8 != 0) goto LAB_006214c1;
      }
      pcVar12 = pTVar15->szPosition + local_30;
      iVar8 = _V_strcmp(pcVar12,(char *)(unaff_EBX + 0x373def));
      if (iVar8 == 0) {
        if (bVar5) goto LAB_006213cd;
      }
      else {
        if (bVar5) {
LAB_006213cd:
          pKVar11 = KeyValues::operator_new(0x24);
          KeyValues::KeyValues(pKVar11,(char *)(unaff_EBX + 0x3a8fd1));
          local_39 = false;
        }
        else {
          pKVar11 = KeyValues::operator_new(0x24);
          KeyValues::KeyValues(pKVar11,pcVar12);
        }
        KeyValues::SetString(pKVar11,&UNK_0038d4af + unaff_EBX,pTVar15->szCommand + local_30);
        KeyValues::SetString(pKVar11,(char *)(unaff_EBX + 0x38d52f),pTVar15->szIcon + local_30);
        pKVar10 = KeyValues::FindKey(this->m_customMenuKeys,(char *)(unaff_EBX + 0x39f5af),true);
        KeyValues::AddSubKey(pKVar10,pKVar11);
        this->m_nNumCustomKeys = this->m_nNumCustomKeys + 1;
        if ((char)local_24 == '\0') {
          iVar8 = _V_strcmp((char *)(unaff_EBX + 0x60c7cf),pcVar12);
          iVar13 = 0;
          if (iVar8 != 0) goto LAB_00621454;
        }
        else {
LAB_00621454:
          if (local_24._1_1_ == '\0') {
            iVar8 = _V_strcmp(&UNK_0060c7df + unaff_EBX,pcVar12);
            iVar13 = 1;
            if (iVar8 == 0) goto LAB_00621557;
            uVar6 = local_24;
          }
          else {
            uVar6 = local_24;
          }
          if (local_24._2_1_ == '\0') {
            local_24 = uVar6;
            iVar8 = _V_strcmp(&UNK_0060c7ef + unaff_EBX,pcVar12);
            iVar13 = 2;
            if (iVar8 == 0) goto LAB_00621557;
            uVar6 = local_24;
            cVar1 = local_24._3_1_;
          }
          else {
            local_24._3_1_ = (char)((uint)uVar6 >> 0x18);
            cVar1 = local_24._3_1_;
          }
          local_24 = uVar6;
          if (cVar1 == '\0') {
            iVar8 = _V_strcmp((char *)(unaff_EBX + 0x60c7ff),pcVar12);
            iVar13 = 3;
            if (iVar8 != 0) {
              if ((char)local_20 == '\0') goto LAB_006215d9;
LAB_0062147c:
              if (local_20._1_1_ == '\0') goto LAB_00621605;
LAB_00621486:
              uVar6 = local_20;
              goto joined_r0x0062162b;
            }
          }
          else {
            if ((char)local_20 != '\0') goto LAB_0062147c;
LAB_006215d9:
            iVar8 = _V_strcmp(&UNK_0060c80f + unaff_EBX,pcVar12);
            iVar13 = 4;
            if (iVar8 == 0) goto LAB_00621557;
            if (local_20._1_1_ != '\0') goto LAB_00621486;
LAB_00621605:
            iVar8 = _V_strcmp(&UNK_0060c81f + unaff_EBX,pcVar12);
            iVar13 = 5;
            if (iVar8 != 0) {
              uVar6 = local_20;
joined_r0x0062162b:
              if (local_20._2_1_ == '\0') {
                local_20 = uVar6;
                iVar8 = _V_strcmp((char *)(unaff_EBX + 0x60c82f),pcVar12);
                iVar13 = 6;
                if (iVar8 == 0) goto LAB_00621557;
                uVar6 = local_20;
                cVar1 = local_20._3_1_;
              }
              else {
                local_20._3_1_ = (char)((uint)uVar6 >> 0x18);
                cVar1 = local_20._3_1_;
              }
              local_20 = uVar6;
              if ((cVar1 == '\0') &&
                 (iVar8 = _V_strcmp((char *)(unaff_EBX + 0x60c83f),pcVar12), iVar8 == 0)) {
                local_20 = CONCAT13(1,(undefined3)local_20);
              }
              goto LAB_0062149a;
            }
          }
        }
LAB_00621557:
        *(undefined1 *)((int)&local_24 + iVar13) = 1;
      }
LAB_0062149a:
      local_38 = local_38 + 1;
      local_30 = local_30 + 0x8c;
    } while (local_38 < (this->m_TauntData).m_Size);
  }
  iVar8 = 0;
                    /* Unresolved local var: int nPosition@[???] */
  do {
    while (*(char *)((int)&local_24 + iVar8) != '\0') {
      iVar8 = iVar8 + 1;
      if (iVar8 == 8) goto LAB_00621781;
    }
    pKVar11 = KeyValues::operator_new(0x24);
    KeyValues::KeyValues(pKVar11,(char *)(iVar8 * 0x10 + unaff_EBX + 0x60c7cf));
    KeyValues::SetString(pKVar11,&UNK_0038d4af + unaff_EBX,(char *)(unaff_EBX + 0x3679d3));
    KeyValues::SetString(pKVar11,(char *)(unaff_EBX + 0x38d52f),(char *)(unaff_EBX + 0x3a99cb));
    KeyValues::SetInt(pKVar11,(char *)(unaff_EBX + 0x3a92cf),0);
    pKVar10 = KeyValues::FindKey(this->m_customMenuKeys,(char *)(unaff_EBX + 0x39f5af),true);
    KeyValues::AddSubKey(pKVar10,pKVar11);
    this->m_nNumCustomKeys = this->m_nNumCustomKeys + 1;
    iVar8 = iVar8 + 1;
  } while (iVar8 != 8);
LAB_00621781:
  this->m_bMenuStale = false;
  return;
}


/* ClientMenuManagerTaunt::GetNumTauntsUnused at 00620ec0 */

/* DWARF original prototype: int GetNumTauntsUnused(ClientMenuManagerTaunt * this) */

int __thiscall ClientMenuManagerTaunt::GetNumTauntsUnused(ClientMenuManagerTaunt *this)

{
  int iVar1;
  int iVar2;
  TauntStatusData *pTVar3;
  int iVar4;
  
                    /* Unresolved local var: int nNumTauntsUnused@[???] */
                    /* Unresolved local var: int i@[DW_OP_reg1(ECX)] */
  iVar1 = (this->m_TauntData).m_Size;
  iVar4 = 0;
  if (0 < iVar1) {
    pTVar3 = (this->m_TauntData).m_Memory.m_pMemory;
    iVar2 = 0;
    do {
                    /* Unresolved local var: TauntStatusData * pTauntData@[???] */
      if ((pTVar3->bUnlocked != false) && (pTVar3->bOwned != false)) {
        iVar4 = iVar4 + (uint)(pTVar3->bUsed == false);
      }
      iVar2 = iVar2 + 1;
      pTVar3 = pTVar3 + 1;
    } while (iVar2 != iVar1);
  }
  return iVar4;
}


/* ClientMenuManagerTaunt::GetUnusedTaunt at 00620e70 */

/* DWARF original prototype: TauntStatusData * GetUnusedTaunt(ClientMenuManagerTaunt * this, int
   nIndex) */

TauntStatusData * __thiscall
ClientMenuManagerTaunt::GetUnusedTaunt(ClientMenuManagerTaunt *this,int nIndex)

{
  int iVar1;
  TauntStatusData *pTVar2;
  int iVar3;
  
                    /* Unresolved local var: int i@[DW_OP_reg2(EDX)] */
  iVar1 = (this->m_TauntData).m_Size;
  if (0 < iVar1) {
    pTVar2 = (this->m_TauntData).m_Memory.m_pMemory;
    iVar3 = 0;
    do {
                    /* Unresolved local var: TauntStatusData * pTauntData@[???] */
      if (((pTVar2->bUnlocked != false) && (pTVar2->bOwned != false)) && (pTVar2->bUsed == false)) {
        if (nIndex == 0) {
          return pTVar2;
        }
        nIndex = nIndex + -1;
      }
      iVar3 = iVar3 + 1;
      pTVar2 = pTVar2 + 1;
    } while (iVar3 != iVar1);
  }
  return (TauntStatusData *)0x0;
}


/* ClientMenuManagerTaunt::ClearCustomMenuKeys at 00081120 */

/* DWARF original prototype: void ClearCustomMenuKeys(ClientMenuManagerTaunt * this) */

void __thiscall ClientMenuManagerTaunt::ClearCustomMenuKeys(ClientMenuManagerTaunt *this)

{
  KeyValues *this_00;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_customMenuKeys != (KeyValues *)0x0) {
    KeyValues::deleteThis(this->m_customMenuKeys);
  }
  this_00 = KeyValues::operator_new(0x24);
  KeyValues::KeyValues(this_00,(char *)(unaff_EBX + 0x9492b1));
  this->m_customMenuKeys = this_00;
  this->m_nNumCustomKeys = 0;
  return;
}


/* ClientMenuManagerTaunt::AddCustomMenuKey at 00620c50 */

/* DWARF original prototype: void AddCustomMenuKey(ClientMenuManagerTaunt * this, KeyValues * pKeys)
    */

void __thiscall
ClientMenuManagerTaunt::AddCustomMenuKey(ClientMenuManagerTaunt *this,KeyValues *pKeys)

{
  KeyValues *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: KeyValues * pDefaultKey@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = KeyValues::FindKey(this->m_customMenuKeys,(char *)(unaff_EBX + 0x39fbdf),true);
  KeyValues::AddSubKey(this_00,pKeys);
  this->m_nNumCustomKeys = this->m_nNumCustomKeys + 1;
  return;
}


/* ClientMenuManagerTaunt::GetAllTauntKeys at 00620be0 */

/* DWARF original prototype: KeyValues * GetAllTauntKeys(ClientMenuManagerTaunt * this) */

KeyValues * __thiscall ClientMenuManagerTaunt::GetAllTauntKeys(ClientMenuManagerTaunt *this)

{
  return (this->super_ClientMenuManager).m_menuKeys;
}


/* ClientMenuManagerTaunt::GetTauntDataByName at 00620bf0 */

/* DWARF original prototype: TauntStatusData * GetTauntDataByName(ClientMenuManagerTaunt * this,
   char * pchTaunt) */

TauntStatusData * __thiscall
ClientMenuManagerTaunt::GetTauntDataByName(ClientMenuManagerTaunt *this,char *pchTaunt)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  if (0 < (this->m_TauntData).m_Size) {
    iVar3 = 0;
    iVar2 = 0;
    do {
      iVar1 = _V_strcmp(((this->m_TauntData).m_Memory.m_pMemory)->szName + iVar2,pchTaunt);
      if (iVar1 == 0) {
        return (TauntStatusData *)(((this->m_TauntData).m_Memory.m_pMemory)->szName + iVar2);
      }
      iVar3 = iVar3 + 1;
      iVar2 = iVar2 + 0x8c;
    } while (iVar3 < (this->m_TauntData).m_Size);
  }
  return (TauntStatusData *)0x0;
}


/* _GLOBAL__I_g_szPositions at 00081390 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_g_szPositions(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

