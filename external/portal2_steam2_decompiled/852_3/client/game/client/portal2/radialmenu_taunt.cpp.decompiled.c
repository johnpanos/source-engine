/* DWARF-guided pseudocode for game/client/portal2/radialmenu_taunt.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 00082890 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  int unaff_EBX;
  longlong lVar1;
  
  lVar1 = ___i686_get_pc_thunk_bx();
  if (lVar1 == 0xffff00000001) {
    (&DAT_00c3e0fc)[unaff_EBX] = 0;
    (&DAT_00c3e0fd)[unaff_EBX] = 0;
    (&DAT_00c3e0fe)[unaff_EBX] = 0;
    (&DAT_00c3e0ff)[unaff_EBX] = 0;
    *(undefined4 *)((int)&GetClassMap::g_Classmap.super_IClassMap._vptr_IClassMap + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&GetClassMap::g_Classmap.m_ClassDict.m_Elements.m_Tree.m_LessFunc.m_LessFunc + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&GetClassMap::g_Classmap.m_ClassDict.m_Elements.m_Tree.m_Elements.m_pMemory + unaff_EBX)
         = 0x7f7fffff;
    *(undefined4 *)
     ((int)&GetClassMap::g_Classmap.m_ClassDict.m_Elements.m_Tree.m_Elements.m_nAllocationCount +
     unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)
     ((int)&GetClassMap::g_Classmap.m_ClassDict.m_Elements.m_Tree.m_Elements.m_nGrowSize + unaff_EBX
     ) = 0;
    *(undefined4 *)((int)&GetClassMap::g_Classmap.m_ClassDict.m_Elements.m_Tree.m_Root + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&GetClassMap::g_Classmap.m_ClassDict.m_Elements.m_Tree.m_FirstFree + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&GetClassMap::g_Classmap.m_ClassDict.m_Elements.m_Tree.m_pElements + unaff_EBX) = 0;
    *(undefined4 *)(UNSPECIFIED_LOGGING_COLOR._color + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&vec2_origin.x + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&vec2_origin.y + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&vec2_invalid.x + unaff_EBX) = 0x7f7fffff;
    *(undefined **)((int)&vec2_invalid.y + unaff_EBX) = &UNK_00aaf564 + unaff_EBX;
    *(undefined4 *)((int)ice_sbox[2] + unaff_EBX + 0x380) = 0;
    *(int *)((int)ice_sbox[2] + unaff_EBX + 0x37c) =
         (int)&PTR_OnLatchInterpolatedVariables_00b3c0a4 + unaff_EBX;
    *(undefined4 *)((int)ice_sbox[2] + unaff_EBX + 0x390) = 0;
    *(undefined4 *)((int)ice_sbox[2] + unaff_EBX + 0x394) = 0;
    *(undefined4 *)((int)ice_sbox[2] + unaff_EBX + 0x398) = 0;
    *(undefined4 *)((int)ice_sbox[2] + unaff_EBX + 0x39c) = 0;
    *(undefined4 *)((int)ice_sbox[2] + unaff_EBX + 0x3a0) = 0;
    *(undefined4 *)((int)ice_sbox[2] + unaff_EBX + 900) = 0;
    ClientMenuManagerTaunt::ClearCustomMenuKeys
              ((ClientMenuManagerTaunt *)((int)ice_sbox[2] + unaff_EBX + 0x37c));
    *(undefined1 *)((int)ice_sbox[2] + unaff_EBX + 0x38c) = 0;
    *(undefined1 *)((int)ice_sbox[2] + unaff_EBX + 0x38d) = 1;
    ___cxa_atexit(unaff_EBX + 0x8d029c,0,*(undefined4 *)(&DAT_00aae47c + unaff_EBX));
  }
  return;
}


/* __tcf_0 at 00952b40 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  *(int *)(unaff_EBX + 0x3fe98c) = unaff_EBX + 0x26bdf4;
  if (*(KeyValues **)(unaff_EBX + 0x3fe994) != (KeyValues *)0x0) {
    KeyValues::deleteThis(*(KeyValues **)(unaff_EBX + 0x3fe994));
  }
  *(undefined4 *)(unaff_EBX + 0x3fe994) = 0;
  *(undefined4 *)(unaff_EBX + 0x3fe9ac) = 0;
  if (*(int *)(unaff_EBX + 0x3fe9a8) < 0) {
    *(undefined4 *)(unaff_EBX + 0x3fe9b0) = *(undefined4 *)(unaff_EBX + 0x3fe9a0);
  }
  else {
    if (*(int *)(unaff_EBX + 0x3fe9a0) == 0) {
      *(undefined4 *)(unaff_EBX + 0x3fe9a4) = 0;
      *(undefined4 *)(unaff_EBX + 0x3fe9b0) = 0;
    }
    else {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1de1d0) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1de1d0),*(int *)(unaff_EBX + 0x3fe9a0));
      *(undefined4 *)(unaff_EBX + 0x3fe9a0) = 0;
      *(undefined4 *)(unaff_EBX + 0x3fe9a4) = 0;
      *(undefined4 *)(unaff_EBX + 0x3fe9b0) = 0;
      if (*(int *)(unaff_EBX + 0x3fe9a8) < 0) goto LAB_00952bd6;
    }
    *(undefined4 *)(unaff_EBX + 0x3fe9a4) = 0;
  }
LAB_00952bd6:
  *(int *)(unaff_EBX + 0x3fe98c) = *(int *)(unaff_EBX + 0x1deaf4) + 8;
  if (*(KeyValues **)(unaff_EBX + 0x3fe990) != (KeyValues *)0x0) {
    KeyValues::deleteThis(*(KeyValues **)(unaff_EBX + 0x3fe990));
  }
  *(undefined4 *)(unaff_EBX + 0x3fe990) = 0;
  return;
}


/* ClientMenuManagerTaunt::ClearTauntStatusData at 00614ad0 */

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
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51c6c2) + 0x230))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x51c6c2),iVar3);
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


/* ClientMenuManagerTaunt::Flush at 00614e80 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
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
  ClientMenuManager::AddMenuFile(&this->super_ClientMenuManager,(char *)(unaff_EBX + 0x38abff));
                    /* Unresolved local var: int i@[???] */
  if (0 < (this->m_TauntData).m_Size) {
    local_24 = 0;
    iVar7 = 0;
    do {
                    /* Unresolved local var: TauntStatusData * pNew@[???] */
      pTVar6 = (this->m_TauntData).m_Memory.m_pMemory;
      iVar5 = *(int *)(pTVar6->szPosition + iVar7 + -5);
      if (iVar5 != -1) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51c312) + 0x230))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x51c312),iVar5);
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
                         (pKVData,(char *)(unaff_EBX + 0x36eed6),(char *)(unaff_EBX + 0x3493de));
      V_strncpy(pTVar6->szCommand,pcVar4,0x20);
      pcVar4 = KeyValues::GetString
                         (pKVData,(char *)(unaff_EBX + 0x36ef56),(char *)(unaff_EBX + 0x3493de));
      V_strncpy(pTVar6->szIcon,pcVar4,0x40);
      iVar7 = KeyValues::GetInt(pKVData,&UNK_00365af6 + unaff_EBX,0);
      pTVar6->bTeamTaunt = iVar7 != 0;
      iVar7 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51c312) + 0x9c))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x51c312),0);
      pTVar6->nTextureID = iVar7;
      piVar1 = (int *)**(undefined4 **)(unaff_EBX + 0x51c312);
      pcVar2 = *(code **)(*piVar1 + 0x84);
      pcVar4 = VarArgs((char *)(C_LocalTempEntity::DrawStudioModel + unaff_EBX + 2),pTVar6->szIcon);
      (*pcVar2)(piVar1,pTVar6->nTextureID,pcVar4,1,0);
      V_strncpy(pTVar6->szPosition,(char *)(unaff_EBX + 0x3557ce),0x10);
      pTVar6->bUnlocked = false;
      pTVar6->bOwned = false;
      pTVar6->bUsed = (bool)(pTVar6->bTeamTaunt ^ 1);
      pTVar6->nCompletionDelay = 0;
      pKVData = KeyValues::GetNextTrueSubKey(pKVData);
    } while (pKVData != (KeyValues *)0x0);
  }
  return;
}


/* ClientMenuManagerTaunt::FindMenu at 00615720 */

/* DWARF original prototype: KeyValues * FindMenu(ClientMenuManagerTaunt * this, char * menuName) */

KeyValues * __thiscall ClientMenuManagerTaunt::FindMenu(ClientMenuManagerTaunt *this,char *menuName)

{
  KeyValues *pKVar1;
  
  UpdateDefaultTaunts(this);
  pKVar1 = KeyValues::FindKey(this->m_customMenuKeys,menuName,false);
  return pKVar1;
}


/* ClientMenuManagerTaunt::KeyValueBuilder at 00614c70 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
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
      KeyValues::SetInt(this_00,&UNK_00350ded + unaff_EBX,(uint)(byte)setName[0x85]);
      KeyValues::SetInt(this_00,(char *)(C_PlayerResource::GetTeam + unaff_EBX + 6),
                        (uint)(byte)setName[0x86]);
      KeyValues::SetInt(this_00,(char *)(unaff_EBX + 0x350dfc),(uint)(byte)setName[0x87]);
      KeyValues::SetInt(this_00,(char *)(C_PlayerResource::GetTeam + unaff_EBX + 1),
                        *(int *)(setName + 0x88));
      KeyValues::SetString(this_00,(char *)(unaff_EBX + 0x3693d2),setName + 0x75);
      KeyValues::AddSubKey(pKeyValues,this_00);
      local_24 = local_24 + 1;
      local_20 = local_20 + 0x8c;
    } while (local_24 < (this->m_TauntData).m_Size);
  }
  return;
}


/* ClientMenuManagerTaunt::GetNextTauntToEarn at 00614dc0 */

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
        if (iVar3 == iVar1) goto LAB_00614e1b;
      }
      iVar3 = iVar3 + 1;
      pTVar2 = pTVar5 + 1;
      pTVar5 = pTVar4;
    } while (iVar3 != iVar1);
LAB_00614e1b:
    if (pTVar5 != (TauntStatusData *)0x0) {
      return pTVar5->szName;
    }
  }
  return (char *)0x0;
}


/* ClientMenuManagerTaunt::IsTauntTeam at 00615cb0 */

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


/* ClientMenuManagerTaunt::SetTauntUnlocked at 00615b00 */

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
            local_20 = -2;
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
            local_20 = local_20 + 2;
          }
          pTVar6->nCompletionDelay = local_20;
        }
        else {
          pTVar6->nCompletionDelay = 0;
          pcVar1 = *(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51b24a) + 0x1c);
                    /* Unresolved local var: int nNumTauntsPending@[???]
                       Unresolved local var: int i@[???] */
          iVar5 = 0;
          if (0 < *(int *)(unaff_EBX + 0x73b9f2)) {
            iVar7 = *(int *)(unaff_EBX + 0x73b9e6);
            iVar2 = 0;
            do {
                    /* Unresolved local var: TauntStatusData * pTauntData@[???] */
              if ((*(char *)(iVar7 + 0x85) != '\0') && (*(char *)(iVar7 + 0x86) == '\0')) {
                iVar5 = iVar5 + (uint)(*(int *)(iVar7 + 0x88) == 0);
              }
              iVar2 = iVar2 + 1;
              iVar7 = iVar7 + 0x8c;
            } while (iVar2 != *(int *)(unaff_EBX + 0x73b9f2));
          }
          pcVar3 = VarArgs((char *)(unaff_EBX + 0x34ff79),iVar5);
          (*pcVar1)(**(undefined4 **)(unaff_EBX + 0x51b24a),pcVar3);
        }
        this_00 = KeyValueSaver();
        C_KeyValueSaver::MarkKeyValuesDirty(this_00,(char *)(unaff_EBX + 0x34ff2d));
        this->m_bLoadedSaveData = true;
        return;
      }
      iVar7 = iVar7 + 1;
      iVar5 = iVar5 + 0x8c;
    } while (iVar7 < (this->m_TauntData).m_Size);
  }
  return;
}


/* ClientMenuManagerTaunt::SetTauntOwned at 00615a70 */

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
        C_KeyValueSaver::MarkKeyValuesDirty(this_00,&UNK_0034ffbd + unaff_EBX);
        this->m_bLoadedSaveData = true;
        return;
      }
      iVar4 = iVar4 + 1;
      iVar3 = iVar3 + 0x8c;
    } while (iVar4 < (this->m_TauntData).m_Size);
  }
  return;
}


/* ClientMenuManagerTaunt::DecreaseOwnershipDelays at 00615d20 */

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
                    C_KeyValueSaver::MarkKeyValuesDirty(pCVar7,(char *)(unaff_EBX + 0x34fd0d));
                    this->m_bLoadedSaveData = true;
                  }
                  break;
                }
                local_24 = local_24 + 1;
                iVar4 = iVar4 + 0x8c;
              } while (local_24 < (this->m_TauntData).m_Size);
            }
            SetTauntPosition((ClientMenuManagerTaunt *)(unaff_EBX + 0x73b7b2),pcVar6,
                             (char *)(unaff_EBX + 0x389536));
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
        pcVar1 = *(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51b02a) + 0x1c);
                    /* Unresolved local var: int nNumTauntsPending@[???]
                       Unresolved local var: int i@[???] */
        iVar4 = 0;
        if (0 < *(int *)(unaff_EBX + 0x73b7d2)) {
          iVar5 = *(int *)(unaff_EBX + 0x73b7c6);
          iVar9 = 0;
          do {
                    /* Unresolved local var: TauntStatusData * pTauntData@[???] */
            if ((*(char *)(iVar5 + 0x85) != '\0') && (*(char *)(iVar5 + 0x86) == '\0')) {
              iVar4 = iVar4 + (uint)(*(int *)(iVar5 + 0x88) == 0);
            }
            iVar9 = iVar9 + 1;
            iVar5 = iVar5 + 0x8c;
          } while (iVar9 != *(int *)(unaff_EBX + 0x73b7d2));
        }
        pcVar6 = VarArgs((char *)(unaff_EBX + 0x34fd59),iVar4);
        (*pcVar1)(**(undefined4 **)(unaff_EBX + 0x51b02a),pcVar6);
      }
      pCVar7 = KeyValueSaver();
      C_KeyValueSaver::MarkKeyValuesDirty(pCVar7,(char *)(unaff_EBX + 0x34fd0d));
    }
  }
  return;
}


/* ClientMenuManagerTaunt::SetTauntUsed at 006159d0 */

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
        C_KeyValueSaver::MarkKeyValuesDirty(this_00,&UNK_0035005d + unaff_EBX);
        this->m_bLoadedSaveData = true;
        return;
      }
      iVar4 = iVar4 + 1;
      iVar3 = iVar3 + 0x8c;
    } while (iVar4 < (this->m_TauntData).m_Size);
  }
  return;
}


/* ClientMenuManagerTaunt::SetTauntDelay at 00615940 */

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
        C_KeyValueSaver::MarkKeyValuesDirty(this_00,(char *)(unaff_EBX + 0x3500ed));
        this->m_bLoadedSaveData = true;
        return;
      }
      iVar4 = iVar4 + 1;
      iVar3 = iVar3 + 0x8c;
    } while (iVar4 < (this->m_TauntData).m_Size);
  }
  return;
}


/* ClientMenuManagerTaunt::SetTauntPosition at 00615750 */

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
  iVar2 = _V_strcmp(pchPosition,(char *)(unaff_EBX + 0x389b06));
  pcVar3 = pcVar3 + 0x75;
  iVar4 = _V_strcmp(pcVar3,pchPosition);
  if (iVar4 == 0) {
    return;
  }
  if (iVar2 == 0) {
    pchPosition = (char *)(unaff_EBX + 0x354efe);
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
LAB_00615915:
        pchPosition = (char *)(iVar2 * 0x10 + unaff_EBX + 0x5e2382);
        break;
      }
      while (iVar1 = _V_strcmp((this->m_TauntData).m_Memory.m_pMemory[iVar4].szPosition,
                               (char *)(iVar2 * 0x10 + unaff_EBX + 0x5e2382)), iVar1 != 0) {
        iVar4 = iVar4 + 1;
        if ((this->m_TauntData).m_Size <= iVar4) goto LAB_00615915;
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
                  (char *)(unaff_EBX + 0x354efe),0x10);
        break;
      }
      iVar2 = iVar2 + 1;
      iVar4 = iVar4 + 0x8c;
    } while (iVar2 < (this->m_TauntData).m_Size);
  }
  V_strncpy(pcVar3,pchPosition,0x10);
  this_00 = KeyValueSaver();
  C_KeyValueSaver::MarkKeyValuesDirty(this_00,(char *)(unaff_EBX + 0x3502dd));
  this->m_bMenuStale = true;
  this->m_bLoadedSaveData = true;
  return;
}


/* ClientMenuManagerTaunt::GetNumTauntsOwned at 00614e40 */

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


/* ClientMenuManagerTaunt::GetNumTauntsPending at 00614b60 */

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


/* ClientMenuManagerTaunt::UpdateDefaultTaunts at 006151b0 */

/* DWARF original prototype: void UpdateDefaultTaunts(ClientMenuManagerTaunt * this) */

void __thiscall ClientMenuManagerTaunt::UpdateDefaultTaunts(ClientMenuManagerTaunt *this)

{
  code *pcVar1;
  TauntStatusData *pTVar2;
  undefined4 uVar3;
  char cVar4;
  TauntStatusData *pTVar5;
  int iVar6;
  KeyValues *pKVar7;
  KeyValues *pKVar8;
  C_KeyValueSaver *this_00;
  char *pcVar9;
  int iVar10;
  int iVar11;
  int unaff_EBX;
  TauntStatusData *pTVar12;
  bool bVar13;
  int local_34;
  int local_30;
  undefined4 local_24;
  undefined4 local_20;
  
                    /* Unresolved local var: bool[8] bPositionsFilled@[???] */
  ___i686_get_pc_thunk_bx();
  if (this->m_bMenuStale != false) {
    if (this->m_bLoadedSaveData == false) {
      this->m_bLoadedSaveData = true;
      this_00 = KeyValueSaver();
      C_KeyValueSaver::MarkKeyValuesDirty(this_00,(char *)(unaff_EBX + 0x35087d));
                    /* Unresolved local var: int i@[???] */
      if (0 < (this->m_TauntData).m_Size) {
                    /* Unresolved local var: TauntStatusData * pTauntData@[???] */
        pTVar12 = (this->m_TauntData).m_Memory.m_pMemory;
        pTVar12->bUnlocked = true;
        pTVar12->bOwned = true;
        pTVar12->bUsed = true;
        pTVar12->nCompletionDelay = 0;
        V_strncpy(pTVar12->szPosition,(char *)(unaff_EBX + 0x5e2922),0x10);
                    /* Unresolved local var: int i@[???] */
        iVar6 = (this->m_TauntData).m_Size;
        if (0 < iVar6) {
          pTVar12 = (this->m_TauntData).m_Memory.m_pMemory;
          if (pTVar12->bTeamTaunt == false) {
            iVar10 = 0;
            pTVar2 = pTVar12;
            do {
              pTVar12 = pTVar2 + 1;
              iVar10 = iVar10 + 1;
              if (iVar10 == iVar6) goto LAB_006151d5;
              pTVar5 = pTVar2 + 1;
              pTVar2 = pTVar12;
            } while (pTVar5->bTeamTaunt == false);
          }
          pTVar12->bUnlocked = true;
          pTVar12->bOwned = false;
          pTVar12->bUsed = false;
          pcVar1 = *(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51bb9a) + 0x1c);
                    /* Unresolved local var: int nNumTauntsPending@[???]
                       Unresolved local var: int i@[???] */
          iVar6 = 0;
          if (0 < *(int *)(&DAT_0073c342 + unaff_EBX)) {
            iVar10 = *(int *)(unaff_EBX + 0x73c336);
            iVar11 = 0;
            do {
                    /* Unresolved local var: TauntStatusData * pTauntData@[???] */
              if ((*(char *)(iVar10 + 0x85) != '\0') && (*(char *)(iVar10 + 0x86) == '\0')) {
                iVar6 = iVar6 + (uint)(*(int *)(iVar10 + 0x88) == 0);
              }
              iVar11 = iVar11 + 1;
              iVar10 = iVar10 + 0x8c;
            } while (iVar11 != *(int *)(&DAT_0073c342 + unaff_EBX));
          }
          pcVar9 = VarArgs(&UNK_003508c9 + unaff_EBX,iVar6);
          (*pcVar1)(**(undefined4 **)(unaff_EBX + 0x51bb9a),pcVar9);
        }
      }
    }
LAB_006151d5:
    ClearCustomMenuKeys(this);
    local_24 = 0;
    local_20 = 0;
    if (0 < (this->m_TauntData).m_Size) {
      local_34 = 0;
      local_30 = 0;
      do {
        pTVar12 = (this->m_TauntData).m_Memory.m_pMemory;
        pcVar9 = pTVar12->szPosition + local_30;
        iVar6 = _V_strcmp(pcVar9,(char *)(unaff_EBX + 0x35549e));
        if (iVar6 != 0) {
          pKVar7 = KeyValues::operator_new(0x24);
          KeyValues::KeyValues(pKVar7,pcVar9);
          KeyValues::SetString(pKVar7,(char *)(unaff_EBX + 0x36eba6),pTVar12->szCommand + local_30);
          KeyValues::SetString(pKVar7,(char *)(unaff_EBX + 0x36ec26),pTVar12->szIcon + local_30);
          KeyValues::SetInt(pKVar7,&UNK_00350044 + unaff_EBX,
                            (uint)(byte)(pTVar12->szPosition[local_30 + 0x12] ^ 1));
          pKVar8 = KeyValues::FindKey(this->m_customMenuKeys,(char *)(unaff_EBX + 0x3807c6),true);
          KeyValues::AddSubKey(pKVar8,pKVar7);
          this->m_nNumCustomKeys = this->m_nNumCustomKeys + 1;
          if ((char)local_24 == '\0') {
            iVar6 = _V_strcmp((char *)(unaff_EBX + 0x5e2922),pcVar9);
            iVar10 = 0;
            if (iVar6 != 0) {
              uVar3 = local_24;
              goto joined_r0x006153b6;
            }
          }
          else {
            uVar3 = local_24;
joined_r0x006153b6:
            if (local_24._1_1_ == '\0') {
              local_24 = uVar3;
              iVar6 = _V_strcmp((char *)(unaff_EBX + 0x5e2932),pcVar9);
              iVar10 = 1;
              if (iVar6 == 0) goto LAB_00615390;
              uVar3 = local_24;
            }
            else {
              local_24._2_1_ = (char)((uint)uVar3 >> 0x10);
            }
            local_24 = uVar3;
            if (local_24._2_1_ == '\0') {
              iVar6 = _V_strcmp((char *)(unaff_EBX + 0x5e2942),pcVar9);
              iVar10 = 2;
              if (iVar6 == 0) goto LAB_00615390;
              if (local_24._3_1_ == '\0') goto LAB_0061540c;
LAB_00615349:
              if ((char)local_20 != '\0') goto LAB_00615353;
LAB_00615438:
              iVar6 = _V_strcmp((char *)(unaff_EBX + 0x5e2962),pcVar9);
              iVar10 = 4;
              if (iVar6 == 0) goto LAB_00615390;
              uVar3 = local_20;
            }
            else {
              local_24._3_1_ = (char)((uint)uVar3 >> 0x18);
              bVar13 = local_24._3_1_ != '\0';
              if (bVar13) goto LAB_00615349;
LAB_0061540c:
              iVar6 = _V_strcmp((char *)(unaff_EBX + 0x5e2952),pcVar9);
              iVar10 = 3;
              if (iVar6 == 0) goto LAB_00615390;
              if ((char)local_20 == '\0') goto LAB_00615438;
LAB_00615353:
              uVar3 = local_20;
            }
            if (local_20._1_1_ == '\0') {
              local_20 = uVar3;
              iVar6 = _V_strcmp((char *)(unaff_EBX + 0x5e2972),pcVar9);
              iVar10 = 5;
              if (iVar6 == 0) goto LAB_00615390;
              uVar3 = local_20;
              cVar4 = local_20._2_1_;
            }
            else {
              local_20._2_1_ = (char)((uint)uVar3 >> 0x10);
              cVar4 = local_20._2_1_;
            }
            local_20 = uVar3;
            if (cVar4 == '\0') {
              iVar6 = _V_strcmp((char *)(unaff_EBX + 0x5e2982),pcVar9);
              iVar10 = 6;
              if (iVar6 == 0) goto LAB_00615390;
            }
            if ((local_20._3_1_ != '\0') ||
               (iVar6 = _V_strcmp((char *)(unaff_EBX + 0x5e2992),pcVar9), iVar6 != 0))
            goto LAB_00615250;
            iVar10 = 7;
          }
LAB_00615390:
          *(undefined1 *)((int)&local_24 + iVar10) = 1;
        }
LAB_00615250:
        local_34 = local_34 + 1;
        local_30 = local_30 + 0x8c;
      } while (local_34 < (this->m_TauntData).m_Size);
    }
    iVar6 = 0;
    do {
      if (*(char *)((int)&local_24 + iVar6) == '\0') {
        pKVar7 = KeyValues::operator_new(0x24);
        KeyValues::KeyValues(pKVar7,(char *)(iVar6 * 0x10 + unaff_EBX + 0x5e2922));
        KeyValues::SetString(pKVar7,(char *)(unaff_EBX + 0x36eba6),(char *)(unaff_EBX + 0x3490ae));
        KeyValues::SetString(pKVar7,(char *)(unaff_EBX + 0x36ec26),(char *)(unaff_EBX + 0x38a8eb));
        KeyValues::SetInt(pKVar7,&UNK_00350044 + unaff_EBX,0);
        pKVar8 = KeyValues::FindKey(this->m_customMenuKeys,(char *)(unaff_EBX + 0x3807c6),true);
        KeyValues::AddSubKey(pKVar8,pKVar7);
        this->m_nNumCustomKeys = this->m_nNumCustomKeys + 1;
      }
      iVar6 = iVar6 + 1;
    } while (iVar6 != 8);
    this->m_bMenuStale = false;
  }
  return;
}


/* ClientMenuManagerTaunt::ClearCustomMenuKeys at 00082820 */

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
  KeyValues::KeyValues(this_00,(char *)(unaff_EBX + 0x91ca41));
  this->m_customMenuKeys = this_00;
  this->m_nNumCustomKeys = 0;
  return;
}


/* ClientMenuManagerTaunt::AddCustomMenuKey at 00614c20 */

/* DWARF original prototype: void AddCustomMenuKey(ClientMenuManagerTaunt * this, KeyValues * pKeys)
    */

void __thiscall
ClientMenuManagerTaunt::AddCustomMenuKey(ClientMenuManagerTaunt *this,KeyValues *pKeys)

{
  KeyValues *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: KeyValues * pDefaultKey@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = KeyValues::FindKey(this->m_customMenuKeys,(char *)(unaff_EBX + 0x380d53),true);
  KeyValues::AddSubKey(this_00,pKeys);
  this->m_nNumCustomKeys = this->m_nNumCustomKeys + 1;
  return;
}


/* ClientMenuManagerTaunt::GetAllTauntKeys at 00614bb0 */

/* DWARF original prototype: KeyValues * GetAllTauntKeys(ClientMenuManagerTaunt * this) */

KeyValues * __thiscall ClientMenuManagerTaunt::GetAllTauntKeys(ClientMenuManagerTaunt *this)

{
  return (this->super_ClientMenuManager).m_menuKeys;
}


/* ClientMenuManagerTaunt::GetTauntDataByName at 00614bc0 */

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


/* _GLOBAL__I_g_szPositions at 00082a80 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_g_szPositions(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

