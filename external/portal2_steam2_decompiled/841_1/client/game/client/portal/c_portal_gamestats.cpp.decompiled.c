/* DWARF-guided pseudocode for game/client/portal/c_portal_gamestats.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 0006aa30 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  uint8 *puVar1;
  CSteamWorksGameStatsUploader *this;
  time_t tVar2;
  undefined4 *puVar3;
  int unaff_EBX;
  longlong lVar4;
  
  lVar4 = ___i686_get_pc_thunk_bx();
  if (lVar4 == 0xffff00000001) {
    *(undefined1 *)((int)&g_aFadeData[0].m_flPercentMax + unaff_EBX) = 0;
    *(undefined1 *)((int)&g_aFadeData[0].m_flPercentMax + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&g_aFadeData[0].m_flPercentMax + unaff_EBX + 2) = 0;
    *(undefined1 *)((int)&g_aFadeData[0].m_flPercentMax + unaff_EBX + 3) = 0;
    *(undefined4 *)((int)&g_aFadeData[0].m_flPixelMin + unaff_EBX) = 0;
    *(undefined4 *)((int)&g_aFadeData[0].m_flPixelMax + unaff_EBX) = 0;
    *(undefined4 *)((int)&g_aFadeData[0].m_flWidth + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&g_aFadeData[1].m_flPercentMin + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&g_aFadeData[1].m_flPercentMax + unaff_EBX) = 0;
    *(undefined4 *)((int)&g_aFadeData[1].m_flPixelMin + unaff_EBX) = 0;
    *(undefined4 *)((int)&g_aFadeData[1].m_flPixelMax + unaff_EBX) = 0;
    *(undefined4 *)((int)&g_aFadeData[1].m_flWidth + unaff_EBX) = 0;
    *(undefined4 *)((int)&g_aFadeData[2].m_flPercentMin + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&g_aFadeData[2].m_flPercentMax + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&g_aFadeData[2].m_flPixelMin + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&g_aFadeData[2].m_flPixelMax + unaff_EBX) = 0x7f7fffff;
    *(undefined **)((int)&g_aFadeData[2].m_flWidth + unaff_EBX) = &UNK_00afa444 + unaff_EBX;
    CBaseGameStats::CBaseGameStats((CBaseGameStats *)(s_SortBuffer + unaff_EBX + 0x44bc));
    *(undefined **)(s_SortBuffer + unaff_EBX + 0x44c4) = &UNK_00b3fe64 + unaff_EBX;
    s_SortBuffer[unaff_EBX + 0x44cc] = '\0';
    puVar1 = s_SortBuffer + unaff_EBX + 0x44c8;
    puVar1[0] = '*';
    puVar1[1] = '\0';
    puVar1[2] = '\0';
    puVar1[3] = '\0';
    CAutoGameSystem::CAutoGameSystem
              ((CAutoGameSystem *)(s_SortBuffer + unaff_EBX + 0x44d0),(char *)0x0);
    *(undefined **)(s_SortBuffer + unaff_EBX + 0x44bc) = &UNK_00b4a804 + unaff_EBX;
    *(undefined **)(s_SortBuffer + unaff_EBX + 0x44c4) = &UNK_00b4a884 + unaff_EBX;
    *(undefined **)(s_SortBuffer + unaff_EBX + 0x44d0) = &UNK_00b4a89c + unaff_EBX;
    *(undefined **)(s_SortBuffer + unaff_EBX + 0x44dc) = &UNK_00b4a8e8 + unaff_EBX;
    *(undefined **)(s_SortBuffer + unaff_EBX + 0x44e0) = &UNK_00b4a8f8 + unaff_EBX;
    puVar1 = s_SortBuffer + unaff_EBX + 0x44e4;
    puVar1[0] = '\0';
    puVar1[1] = '\0';
    puVar1[2] = '\0';
    puVar1[3] = '\0';
    puVar1 = s_SortBuffer + unaff_EBX + 0x44e8;
    puVar1[0] = '\0';
    puVar1[1] = '\0';
    puVar1[2] = '\0';
    puVar1[3] = '\0';
    puVar1 = s_SortBuffer + unaff_EBX + 0x44ec;
    puVar1[0] = '\0';
    puVar1[1] = '\0';
    puVar1[2] = '\0';
    puVar1[3] = '\0';
    puVar1 = s_SortBuffer + unaff_EBX + 0x44f0;
    puVar1[0] = '\0';
    puVar1[1] = '\0';
    puVar1[2] = '\0';
    puVar1[3] = '\0';
    puVar1 = s_SortBuffer + unaff_EBX + 0x44f4;
    puVar1[0] = '\0';
    puVar1[1] = '\0';
    puVar1[2] = '\0';
    puVar1[3] = '\0';
    this = GetSteamWorksSGameStatsUploader();
    tVar2 = CSteamWorksGameStatsUploader::GetTimeSinceEpoch(this);
    *(time_t *)(s_SortBuffer + unaff_EBX + 0x44fc) = tVar2;
    *(time_t *)(s_SortBuffer + unaff_EBX + 0x4500) = tVar2 >> 0x1f;
    s_SortBuffer[unaff_EBX + 0x44f8] = '\0';
    puVar1 = s_SortBuffer + unaff_EBX + 0x45cc;
    puVar1[0] = '\0';
    puVar1[1] = '\0';
    puVar1[2] = '\0';
    puVar1[3] = '\0';
    puVar1 = s_SortBuffer + unaff_EBX + 0x45d0;
    puVar1[0] = '\0';
    puVar1[1] = '\0';
    puVar1[2] = '\0';
    puVar1[3] = '\0';
    puVar1 = s_SortBuffer + unaff_EBX + 0x45d4;
    puVar1[0] = '\0';
    puVar1[1] = '\0';
    puVar1[2] = '\0';
    puVar1[3] = '\0';
    puVar1 = s_SortBuffer + unaff_EBX + 0x45d8;
    puVar1[0] = '\0';
    puVar1[1] = '\0';
    puVar1[2] = '\0';
    puVar1[3] = '\0';
    puVar1 = s_SortBuffer + unaff_EBX + 0x45dc;
    puVar1[0] = '\0';
    puVar1[1] = '\0';
    puVar1[2] = '\0';
    puVar1[3] = '\0';
    s_SortBuffer[unaff_EBX + 0x45e0] = '\0';
    ___cxa_atexit(unaff_EBX + 0x90f88c,0,*(undefined4 *)(&DAT_00af9328 + unaff_EBX));
    puVar3 = operator_new(0x14);
    *puVar3 = 0;
    puVar3[1] = 0;
    puVar3[2] = 0;
    puVar3[3] = 0;
    puVar3[4] = 0;
    *(undefined4 **)(s_SortBuffer + unaff_EBX + 0x45e4) = puVar3;
  }
  return;
}


/* __tcf_0 at 0097a2d0 */

void __tcf_0(void *param_1)

{
  int *piVar1;
  int unaff_EBX;
  int in_stack_ffffffc8;
  int iVar2;
  
  ___i686_get_pc_thunk_bx();
  *(int *)(unaff_EBX + 0x3ec602) = unaff_EBX + 0x23af6a;
  *(int *)(unaff_EBX + 0x3ec60a) = unaff_EBX + 0x23afea;
  *(int *)(unaff_EBX + 0x3ec616) = unaff_EBX + 0x23b002;
  *(undefined **)(unaff_EBX + 0x3ec622) = &UNK_0023b04e + unaff_EBX;
  *(int *)(unaff_EBX + 0x3ec626) = unaff_EBX + 0x23b05e;
  *(undefined4 *)(unaff_EBX + 0x3ec71e) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x3ec71a)) {
    iVar2 = *(int *)(unaff_EBX + 0x3ec712);
    if (iVar2 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1e9a92) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1e9a92));
      *(undefined4 *)(unaff_EBX + 0x3ec712) = 0;
      in_stack_ffffffc8 = iVar2;
    }
    *(undefined4 *)(unaff_EBX + 0x3ec716) = 0;
  }
  iVar2 = *(int *)(unaff_EBX + 0x3ec712);
  *(int *)(unaff_EBX + 0x3ec722) = iVar2;
  if (-1 < *(int *)(unaff_EBX + 0x3ec71a)) {
    if (iVar2 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1e9a92) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1e9a92));
      *(undefined4 *)(unaff_EBX + 0x3ec712) = 0;
      in_stack_ffffffc8 = iVar2;
    }
    *(undefined4 *)(unaff_EBX + 0x3ec716) = 0;
  }
  *(undefined4 *)(unaff_EBX + 0x3ec636) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x3ec632)) {
    iVar2 = *(int *)(unaff_EBX + 0x3ec62a);
    if (iVar2 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1e9a92) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1e9a92));
      *(undefined4 *)(unaff_EBX + 0x3ec62a) = 0;
      in_stack_ffffffc8 = iVar2;
    }
    *(undefined4 *)(unaff_EBX + 0x3ec62e) = 0;
  }
  iVar2 = *(int *)(unaff_EBX + 0x3ec62a);
  *(int *)(unaff_EBX + 0x3ec63a) = iVar2;
  if (-1 < *(int *)(unaff_EBX + 0x3ec632)) {
    if (iVar2 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1e9a92) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1e9a92));
      *(undefined4 *)(unaff_EBX + 0x3ec62a) = 0;
      in_stack_ffffffc8 = iVar2;
    }
    *(undefined4 *)(unaff_EBX + 0x3ec62e) = 0;
  }
  *(int *)(unaff_EBX + 0x3ec616) = unaff_EBX + 0x1ebdca;
  IGameSystem::~IGameSystem((IGameSystem *)(unaff_EBX + 0x3ec616),in_stack_ffffffc8);
  *(undefined **)(unaff_EBX + 0x3ec60a) = &UNK_002305ca + unaff_EBX;
  *(undefined4 *)(unaff_EBX + 0x3ec60e) = 0xd;
  if (*(char *)(unaff_EBX + 0x3ec612) != '\0') {
    piVar1 = (int *)**(int **)(unaff_EBX + 0x1e9a9e);
    if (piVar1 != (int *)0x0) {
      (**(code **)(*piVar1 + 0x18))(piVar1,unaff_EBX + 0x3ec60a);
    }
    *(undefined1 *)(unaff_EBX + 0x3ec612) = 0;
  }
  return;
}


/* C_CPortalGameStats::Init at 004d9a70 */

/* DWARF original prototype: bool Init(C_CPortalGameStats * this) */

bool __thiscall C_CPortalGameStats::Init(C_CPortalGameStats *this)

{
  int *piVar1;
  int *piVar2;
  int iVar3;
  undefined4 *puVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  piVar1 = *(int **)(&DAT_0068a31c + unaff_EBX);
  piVar2 = (int *)*piVar1;
  if (piVar2 != (int *)0x0) {
    iVar3 = (**(code **)(*piVar2 + 0x28))(piVar2);
    if (iVar3 != 0) {
      piVar1 = (int *)*piVar1;
      puVar4 = (undefined4 *)(**(code **)(*piVar1 + 0x28))(piVar1);
      (**(code **)*puVar4)(puVar4,&this->super_IMatchEventsSink);
    }
  }
  **(undefined4 **)(&DAT_0068aab0 + unaff_EBX) = this;
  return true;
}


/* C_CPortalGameStats::Shutdown at 004d9af0 */

/* DWARF original prototype: void Shutdown(C_CPortalGameStats * this) */

void __thiscall C_CPortalGameStats::Shutdown(C_CPortalGameStats *this)

{
  int *piVar1;
  int iVar2;
  int *piVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  piVar1 = *(int **)(&DAT_0068a29f + unaff_EBX);
  piVar3 = (int *)*piVar1;
  if (piVar3 != (int *)0x0) {
    iVar2 = (**(code **)(*piVar3 + 0x28))(piVar3);
    if (iVar2 != 0) {
      piVar1 = (int *)*piVar1;
      piVar3 = (int *)(**(code **)(*piVar1 + 0x28))(piVar1);
      (**(code **)(*piVar3 + 4))(piVar3,&this->super_IMatchEventsSink);
    }
  }
  return;
}


/* C_CPortalGameStats::Event_AddSurveyData at 004d9d10 */

/* DWARF original prototype: void Event_AddSurveyData(C_CPortalGameStats * this, uint32 difficulty,
   uint32 enjoyment, char * essay) */

void __thiscall
C_CPortalGameStats::Event_AddSurveyData
          (C_CPortalGameStats *this,uint32 difficulty,uint32 enjoyment,char *essay)

{
  int *piVar1;
  uint uVar2;
  undefined1 *puVar3;
  CSteamWorksGameStatsUploader *this_00;
  time_t tVar4;
  int iVar5;
  undefined4 *puVar6;
  SSurveyData **ppSVar7;
  int unaff_EBX;
  int iVar8;
  int iVar9;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  puVar3 = operator_new(0x1018);
  this_00 = GetSteamWorksSGameStatsUploader();
  tVar4 = CSteamWorksGameStatsUploader::GetTimeSinceEpoch(this_00);
  *(time_t *)(puVar3 + 4) = tVar4;
  *(time_t *)(puVar3 + 8) = tVar4 >> 0x1f;
  *puVar3 = 1;
  *(uint32 *)(puVar3 + 0xc) = difficulty;
  *(uint32 *)(puVar3 + 0x10) = enjoyment;
  _memset(puVar3 + 0x14,0,0x1001);
  if (essay != (char *)0x0) {
    V_strncpy(puVar3 + 0x14,essay,0x1000);
  }
  uVar2 = (this->m_SurveyData).m_Size;
  iVar9 = uVar2 + 1;
  iVar5 = (this->m_SurveyData).m_Memory.m_nAllocationCount;
  if (iVar9 <= iVar5) goto LAB_004d9db0;
  iVar8 = (this->m_SurveyData).m_Memory.m_nGrowSize;
  if (iVar8 < 0) goto LAB_004d9db0;
  if (iVar8 == 0) {
    if ((iVar5 == 0) && (iVar5 = 8, iVar9 < 9)) {
      local_20 = 0x20;
    }
    else {
      do {
        local_20 = iVar5;
        iVar5 = local_20 * 2;
      } while (iVar5 < iVar9);
      local_20 = local_20 * 8;
    }
    goto LAB_004d9e30;
  }
  iVar5 = ((int)uVar2 / iVar8 + 1) * iVar8;
  if (iVar5 < iVar9) {
    if (iVar5 != 0) {
LAB_004d9ed0:
      do {
        iVar5 = (iVar5 + iVar9) / 2;
      } while (iVar5 < iVar9);
      goto LAB_004d9ee0;
    }
    if (iVar9 < 0) {
      iVar5 = -1;
      local_20 = -4;
    }
    else {
      local_20 = 0;
      if (iVar9 != 0) goto LAB_004d9ed0;
    }
  }
  else {
LAB_004d9ee0:
    local_20 = iVar5 * 4;
  }
LAB_004d9e30:
  (this->m_SurveyData).m_Memory.m_nAllocationCount = iVar5;
  ppSVar7 = (this->m_SurveyData).m_Memory.m_pMemory;
  if (ppSVar7 == (SSurveyData **)0x0) {
    ppSVar7 = (SSurveyData **)
              (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x68a052))
                        ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x68a052),local_20);
    (this->m_SurveyData).m_Memory.m_pMemory = ppSVar7;
  }
  else {
    ppSVar7 = (SSurveyData **)
              (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x68a052) + 4))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x68a052),ppSVar7,local_20);
    (this->m_SurveyData).m_Memory.m_pMemory = ppSVar7;
  }
LAB_004d9db0:
  piVar1 = &(this->m_SurveyData).m_Size;
  *piVar1 = *piVar1 + 1;
  ppSVar7 = (this->m_SurveyData).m_Memory.m_pMemory;
  (this->m_SurveyData).m_pElements = ppSVar7;
  iVar5 = ~uVar2 + (this->m_SurveyData).m_Size;
  if (iVar5 < 1) {
    iVar8 = uVar2 << 2;
  }
  else {
    iVar8 = uVar2 * 4;
    _V_memmove(ppSVar7 + iVar9,ppSVar7 + uVar2,iVar5 * 4);
  }
  puVar6 = (undefined4 *)(iVar8 + (int)(this->m_SurveyData).m_Memory.m_pMemory);
  if (puVar6 != (undefined4 *)0x0) {
    *puVar6 = puVar3;
  }
  return;
}


/* C_CPortalGameStats::SubmitGameStats at 004d9c90 */

/* DWARF original prototype: void SubmitGameStats(C_CPortalGameStats * this, KeyValues * pKV) */

void __thiscall C_CPortalGameStats::SubmitGameStats(C_CPortalGameStats *this,KeyValues *pKV)

{
  int iVar1;
  undefined4 *puVar2;
  int *piVar3;
  int unaff_EBX;
  int iVar4;
  int listCount;
  
  ___i686_get_pc_thunk_bx();
  piVar3 = *(int **)(unaff_EBX + 0x88cd6a);
  iVar1 = piVar3[3];
                    /* Unresolved local var: int i@[???] */
  iVar4 = 0;
  if (0 < iVar1) {
    while( true ) {
      puVar2 = *(undefined4 **)(*piVar3 + iVar4 * 4);
      (**(code **)*puVar2)(puVar2,pKV);
      piVar3 = *(int **)(**(int **)(unaff_EBX + 0x88cd6a) + iVar4 * 4);
      (**(code **)(*piVar3 + 4))(piVar3);
      iVar4 = iVar4 + 1;
      if (iVar4 == iVar1) break;
      piVar3 = *(int **)(unaff_EBX + 0x88cd6a);
    }
  }
  return;
}


/* C_CPortalGameStats::AddDataForSend at 004dabd0 */

/* DWARF original prototype: bool AddDataForSend(C_CPortalGameStats * this, KeyValues * pKV,
   StatSendType_t sendType) */

bool __thiscall
C_CPortalGameStats::AddDataForSend(C_CPortalGameStats *this,KeyValues *pKV,StatSendType_t sendType)

{
  AddData(this);
  if (sendType != STATSEND_APPSHUTDOWN) {
    return false;
  }
  (*(this->super_CBaseGameStats)._vptr_CBaseGameStats[0x19])(this);
  return false;
}


/* C_CPortalGameStats::AddData at 004da6d0 */

/* DWARF original prototype: void AddData(C_CPortalGameStats * this) */

void __thiscall C_CPortalGameStats::AddData(C_CPortalGameStats *this)

{
  int iVar1;
  undefined4 *puVar2;
  SSurveyData *pSVar3;
  uint uVar4;
  int *piVar5;
  KeyValues *this_00;
  char *value;
  int iVar6;
  undefined4 uVar7;
  int iVar8;
  undefined4 *puVar9;
  int *piVar10;
  int *piVar11;
  int iVar12;
  CPortal_StuckPlayer **ppCVar13;
  SSurveyData **ppSVar14;
  int unaff_EBX;
  KeyValues *pKV;
  int local_38;
  int local_24;
  int local_20;
  
                    /* Unresolved local var: char * pzMapName@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = KeyValues::operator_new(0x24);
  KeyValues::KeyValues(this_00,(char *)(unaff_EBX + 0x4dbcfc));
  if (this_00 == (KeyValues *)0x0) {
    return;
  }
  value = IGameSystem::MapName();
  if (value == (char *)0x0) {
    value = (char *)(unaff_EBX + 0x4ae586);
  }
  KeyValues::SetString(this_00,(char *)(unaff_EBX + 0x4bc89b),value);
  if (0 < (this->m_SurveyData).m_Size) {
    local_38 = 0;
    puVar2 = *(undefined4 **)(unaff_EBX + 0x689692);
    do {
      pSVar3 = (this->m_SurveyData).m_Memory.m_pMemory[local_38];
      if (*(int *)(unaff_EBX + 0x77eebe) == 0) {
        piVar10 = operator_new(0x18);
        piVar10[1] = 0;
        piVar10[2] = 0;
        piVar10[3] = 0;
        piVar10[4] = 0;
        piVar10[5] = 0;
        *piVar10 = unaff_EBX + 0x6dad6a;
        *(int **)(unaff_EBX + 0x77eebe) = piVar10;
        piVar11 = (int *)(*(this->super_IGameStatTracker)._vptr_IGameStatTracker[1])
                                   (&this->super_IGameStatTracker);
        uVar4 = piVar11[3];
        iVar1 = uVar4 + 1;
        iVar12 = piVar11[1];
        if (iVar12 < iVar1) {
          iVar6 = piVar11[2];
          if (-1 < iVar6) {
            if (iVar6 == 0) {
              if ((iVar12 == 0) && (iVar12 = 8, iVar1 < 9)) {
                local_20 = 0x20;
              }
              else {
                do {
                  local_20 = iVar12;
                  iVar12 = local_20 * 2;
                } while (iVar12 < iVar1);
                local_20 = local_20 * 8;
              }
              goto LAB_004daa9c;
            }
            iVar12 = ((int)uVar4 / iVar6 + 1) * iVar6;
            if (iVar12 < iVar1) {
              if (iVar12 != 0) {
LAB_004dab34:
                do {
                  iVar12 = (iVar12 + iVar1) / 2;
                } while (iVar12 < iVar1);
                goto LAB_004dab47;
              }
              if (iVar1 < 0) {
                iVar12 = -1;
                local_20 = -4;
              }
              else {
                local_20 = 0;
                if (iVar1 != 0) goto LAB_004dab34;
              }
            }
            else {
LAB_004dab47:
              local_20 = iVar12 * 4;
            }
LAB_004daa9c:
            piVar11[1] = iVar12;
            if (*piVar11 == 0) {
              iVar12 = (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x689692))
                                 ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x689692),local_20);
              *piVar11 = iVar12;
            }
            else {
              piVar5 = (int *)*puVar2;
              iVar12 = (**(code **)(*piVar5 + 4))(piVar5,*piVar11,local_20);
              *piVar11 = iVar12;
            }
          }
        }
        piVar11[3] = piVar11[3] + 1;
        iVar12 = *piVar11;
        piVar11[4] = iVar12;
        if (0 < (int)(~uVar4 + piVar11[3])) {
          _V_memmove((void *)(iVar12 + iVar1 * 4),(void *)(iVar12 + uVar4 * 4),
                     (~uVar4 + piVar11[3]) * 4);
        }
        puVar9 = (undefined4 *)(uVar4 * 4 + *piVar11);
        if (puVar9 != (undefined4 *)0x0) {
          *puVar9 = piVar10;
        }
      }
      iVar12 = *(int *)(unaff_EBX + 0x77eebe);
      uVar4 = *(uint *)(iVar12 + 0x10);
      iVar1 = uVar4 + 1;
      iVar6 = *(int *)(iVar12 + 8);
      if (iVar6 < iVar1) {
        iVar8 = *(int *)(iVar12 + 0xc);
        if (-1 < iVar8) {
          if (iVar8 == 0) {
            if ((iVar6 == 0) && (iVar6 = 8, iVar1 < 9)) {
              local_24 = 0x20;
              goto LAB_004da80d;
            }
            do {
              local_24 = iVar6;
              iVar6 = local_24 * 2;
            } while (iVar6 < iVar1);
            local_24 = local_24 * 8;
            *(int *)(iVar12 + 8) = iVar6;
            iVar6 = *(int *)(iVar12 + 4);
          }
          else {
            iVar6 = ((int)uVar4 / iVar8 + 1) * iVar8;
            if (iVar6 < iVar1) {
              if (iVar6 != 0) {
LAB_004da7f0:
                do {
                  iVar6 = (iVar6 + iVar1) / 2;
                } while (iVar6 < iVar1);
                goto LAB_004da803;
              }
              if (iVar1 < 0) {
                iVar6 = -1;
                local_24 = -4;
              }
              else {
                local_24 = 0;
                if (iVar1 != 0) goto LAB_004da7f0;
              }
            }
            else {
LAB_004da803:
              local_24 = iVar6 * 4;
            }
LAB_004da80d:
            *(int *)(iVar12 + 8) = iVar6;
            iVar6 = *(int *)(iVar12 + 4);
          }
          if (iVar6 == 0) {
            puVar9 = (undefined4 *)*puVar2;
            uVar7 = (**(code **)*puVar9)(puVar9,local_24);
            *(undefined4 *)(iVar12 + 4) = uVar7;
          }
          else {
            uVar7 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x689692) + 4))
                              ((int *)**(undefined4 **)(unaff_EBX + 0x689692),iVar6,local_24);
            *(undefined4 *)(iVar12 + 4) = uVar7;
          }
        }
      }
      *(int *)(iVar12 + 0x10) = *(int *)(iVar12 + 0x10) + 1;
      iVar6 = *(int *)(iVar12 + 4);
      *(int *)(iVar12 + 0x14) = iVar6;
      iVar8 = ~uVar4 + *(int *)(iVar12 + 0x10);
      if (0 < iVar8) {
        _V_memmove((void *)(iVar6 + iVar1 * 4),(void *)(iVar6 + uVar4 * 4),iVar8 * 4);
      }
      puVar9 = (undefined4 *)(uVar4 * 4 + *(int *)(iVar12 + 4));
      if (puVar9 != (undefined4 *)0x0) {
        *puVar9 = pSVar3;
      }
      local_38 = local_38 + 1;
    } while (local_38 < (this->m_SurveyData).m_Size);
  }
  (*(this->super_CBaseGameStats)._vptr_CBaseGameStats[0x17])(this,this_00);
  (this->m_SurveyData).m_Size = 0;
  if ((this->m_SurveyData).m_Memory.m_nGrowSize < 0) {
    ppSVar14 = (this->m_SurveyData).m_Memory.m_pMemory;
  }
  else {
    ppSVar14 = (this->m_SurveyData).m_Memory.m_pMemory;
    if (ppSVar14 != (SSurveyData **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x689692) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x689692),ppSVar14);
      (this->m_SurveyData).m_Memory.m_pMemory = (SSurveyData **)0x0;
    }
    ppSVar14 = (SSurveyData **)0x0;
    (this->m_SurveyData).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_SurveyData).m_pElements = ppSVar14;
  (this->m_VectorStuckPlayer).m_Size = 0;
  if ((this->m_VectorStuckPlayer).m_Memory.m_nGrowSize < 0) {
    ppCVar13 = (this->m_VectorStuckPlayer).m_Memory.m_pMemory;
  }
  else {
    ppCVar13 = (this->m_VectorStuckPlayer).m_Memory.m_pMemory;
    if (ppCVar13 != (CPortal_StuckPlayer **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x689692) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x689692),ppCVar13);
      (this->m_VectorStuckPlayer).m_Memory.m_pMemory = (CPortal_StuckPlayer **)0x0;
    }
    ppCVar13 = (CPortal_StuckPlayer **)0x0;
    (this->m_VectorStuckPlayer).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_VectorStuckPlayer).m_pElements = ppCVar13;
  KeyValues::deleteThis(this_00);
  return;
}


/* C_CPortalGameStats::OnSessionStart at 004da6a0 */

/* DWARF original prototype: void OnSessionStart(C_CPortalGameStats * this) */

void __thiscall C_CPortalGameStats::OnSessionStart(C_CPortalGameStats *this)

{
  CSteamWorksGameStatsUploader *this_00;
  
  this->m_bSessionActive = true;
  GetConfigData(this);
  this_00 = GetSteamWorksSGameStatsUploader();
  CSteamWorksGameStatsUploader::InitializeSystem(this_00);
  return;
}


/* C_CPortalGameStats::OnSessionEnd at 004d9c40 */

/* DWARF original prototype: void OnSessionEnd(C_CPortalGameStats * this) */

void __thiscall C_CPortalGameStats::OnSessionEnd(C_CPortalGameStats *this)

{
  CSteamWorksGameStatsUploader *pCVar1;
  
  this->m_bSessionActive = false;
  pCVar1 = GetSteamWorksSGameStatsUploader();
  CSteamWorksGameStatsUploader::SetExitStatus(pCVar1,k_EGameStatsExitDefaultValue);
  pCVar1 = GetSteamWorksSGameStatsUploader();
  CSteamWorksGameStatsUploader::SendSessionStats(pCVar1);
  return;
}


/* C_CPortalGameStats::Event_LevelInit at 004d9bc0 */

/* DWARF original prototype: void Event_LevelInit(C_CPortalGameStats * this) */

void __thiscall C_CPortalGameStats::Event_LevelInit(C_CPortalGameStats *this)

{
  char *pcVar1;
  _func_int_varargs *UNRECOVERED_JUMPTABLE;
  int unaff_EBX;
  
                    /* Unresolved local var: char * name1@[???] */
  ___i686_get_pc_thunk_bx();
  pcVar1 = IGameSystem::MapName();
  pcVar1 = V_stristr(pcVar1,(char *)(unaff_EBX + 0x4e6b75));
  if (pcVar1 == (char *)0x0) {
    if (this->m_bSessionActive == false) {
      UNRECOVERED_JUMPTABLE = (this->super_CBaseGameStats)._vptr_CBaseGameStats[0x18];
      goto LAB_004d9c00;
    }
  }
  else if (this->m_bSessionActive == false) {
    return;
  }
  UNRECOVERED_JUMPTABLE = (this->super_CBaseGameStats)._vptr_CBaseGameStats[0x19];
LAB_004d9c00:
                    /* WARNING: Could not recover jumptable at 0x004d9c09. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}


/* C_CPortalGameStats::GetConfigData at 004d9f50 */

/* DWARF original prototype: void GetConfigData(C_CPortalGameStats * this) */

void __thiscall C_CPortalGameStats::GetConfigData(C_CPortalGameStats *this)

{
  float *pfVar1;
  char cVar2;
  uint uVar3;
  bool bVar4;
  char *pcVar5;
  int iVar6;
  undefined1 *puVar7;
  CSteamWorksGameStatsUploader *this_00;
  time_t tVar8;
  int iVar9;
  undefined4 *puVar10;
  int iVar11;
  undefined4 uVar12;
  int *piVar13;
  int *piVar14;
  int iVar15;
  int unaff_EBX;
  ConVarRef *local_38;
  int local_24;
  int local_20;
  
                    /* Unresolved local var: ConVar in_joystick@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x68a536) + 0x1c) + 0x30) != 0) {
    (this->m_ConfigData).m_AllowGamepad = '\x01';
  }
  piVar13 = *(int **)(unaff_EBX + 0x689e46);
  if ((((*piVar13 == 0) || (iVar6 = **(int **)(unaff_EBX + 0x689e3a), iVar6 == 0)) ||
      (*(int *)(iVar6 + 0x14) == 0)) || (piVar14 = *(int **)(iVar6 + 8), piVar14 == (int *)0x0)) {
    V_strncpy((this->m_ConfigData).m_AudioLanguage,&UNK_0050067a + unaff_EBX,0x40);
    V_strncpy((this->m_ConfigData).m_TextLanguage,&UNK_0050067a + unaff_EBX,0x40);
LAB_004da0a9:
    cVar2 = *(char *)(unaff_EBX + 0x7f14a2);
  }
  else {
    pcVar5 = (char *)(**(code **)(*piVar14 + 0x10))(piVar14);
    if (pcVar5 != (char *)0x0) {
      V_strncpy((this->m_ConfigData).m_CountryCode,pcVar5,0x40);
    }
    piVar13 = (int *)*piVar13;
    (**(code **)(*piVar13 + 0x170))(piVar13,(this->m_ConfigData).m_TextLanguage,0x40);
    piVar13 = *(int **)(**(int **)(unaff_EBX + 0x689e3a) + 0x14);
    pcVar5 = (char *)(**(code **)(*piVar13 + 0x10))(piVar13);
    if (pcVar5 == (char *)0x0) {
      V_strncpy((this->m_ConfigData).m_AudioLanguage,&UNK_0050067a + unaff_EBX,0x40);
      goto LAB_004da0a9;
    }
    V_strncpy((this->m_ConfigData).m_AudioLanguage,pcVar5,0x40);
    cVar2 = *(char *)(unaff_EBX + 0x7f14a2);
  }
  if (cVar2 == '\0') {
    iVar6 = ___cxa_guard_acquire(unaff_EBX + 0x7f14a2);
    if (iVar6 != 0) {
      ConVarRef::ConVarRef((ConVarRef *)(unaff_EBX + 0x7f14ea),(char *)(unaff_EBX + 0x4d9bee));
      ___cxa_guard_release(unaff_EBX + 0x7f14a2);
    }
  }
  if ((*(char *)(unaff_EBX + 0x7f14aa) == '\0') &&
     (iVar6 = ___cxa_guard_acquire(unaff_EBX + 0x7f14aa), iVar6 != 0)) {
    ConVarRef::ConVarRef((ConVarRef *)(unaff_EBX + 0x7f14e2),&UNK_004ed68a + unaff_EBX);
    ___cxa_guard_release(unaff_EBX + 0x7f14aa);
  }
  if ((*(char *)(unaff_EBX + 0x7f14b2) == '\0') &&
     (iVar6 = ___cxa_guard_acquire(unaff_EBX + 0x7f14b2), iVar6 != 0)) {
    ConVarRef::ConVarRef((ConVarRef *)(unaff_EBX + 0x7f14da),&UNK_004bba8e + unaff_EBX);
    ___cxa_guard_release(unaff_EBX + 0x7f14b2);
  }
  if ((*(char *)(unaff_EBX + 0x7f14ba) == '\0') &&
     (iVar6 = ___cxa_guard_acquire(unaff_EBX + 0x7f14ba), iVar6 != 0)) {
    ConVarRef::ConVarRef((ConVarRef *)(unaff_EBX + 0x7f14d2),(char *)(unaff_EBX + 0x4dc46e));
    ___cxa_guard_release(unaff_EBX + 0x7f14ba);
  }
  if (*(char *)(unaff_EBX + 0x7f14c2) == '\0') {
    iVar6 = ___cxa_guard_acquire(unaff_EBX + 0x7f14c2);
    if (iVar6 != 0) {
      ConVarRef::ConVarRef
                ((ConVarRef *)(unaff_EBX + 0x7f14ca),
                 (char *)(_C_TEPlayerAnimEvent_CreateObject + unaff_EBX + 2));
      ___cxa_guard_release(unaff_EBX + 0x7f14c2);
    }
  }
  local_38 = (ConVarRef *)(unaff_EBX + 0x7f14ca);
  bVar4 = ConVarRef::IsValid((ConVarRef *)(unaff_EBX + 0x7f14ea));
  if ((bVar4) &&
     (pfVar1 = (float *)(*(int *)(unaff_EBX + 0x7f14ee) + 0x2c), *pfVar1 <= 0.0 && *pfVar1 != 0.0))
  {
    (this->m_ConfigData).m_ReverseMouse = '\x01';
  }
  bVar4 = ConVarRef::IsValid((ConVarRef *)(unaff_EBX + 0x7f14e2));
  if ((bVar4) && (*(int *)(*(int *)(unaff_EBX + 0x7f14e6) + 0x30) != 0)) {
    (this->m_ConfigData).m_AllowConsole = '\x01';
  }
  bVar4 = ConVarRef::IsValid((ConVarRef *)(unaff_EBX + 0x7f14d2));
  if (bVar4) {
    (this->m_ConfigData).m_ColorBlind =
         (uint8)*(undefined4 *)(*(int *)(unaff_EBX + 0x7f14d6) + 0x30);
  }
  bVar4 = ConVarRef::IsValid(local_38);
  if (bVar4) {
    (this->m_ConfigData).m_OpenMic = (uint8)*(undefined4 *)(*(int *)(unaff_EBX + 0x7f14ce) + 0x30);
  }
  bVar4 = UTIL_IsLowViolence();
  (this->m_ConfigData).m_LowViolence = bVar4;
  puVar7 = operator_new(0xd4);
  this_00 = GetSteamWorksSGameStatsUploader();
  tVar8 = CSteamWorksGameStatsUploader::GetTimeSinceEpoch(this_00);
  *(time_t *)(puVar7 + 4) = tVar8;
  *(time_t *)(puVar7 + 8) = tVar8 >> 0x1f;
  *puVar7 = 0;
  _memcpy(puVar7,&this->m_ConfigData,0xd3);
  if (*(int *)(unaff_EBX + 0x77f632) == 0) {
    piVar13 = operator_new(0x18);
    piVar13[1] = 0;
    piVar13[2] = 0;
    piVar13[3] = 0;
    piVar13[4] = 0;
    piVar13[5] = 0;
    *piVar13 = unaff_EBX + 0x6db46a;
    *(int **)(unaff_EBX + 0x77f632) = piVar13;
    piVar14 = (int *)(*(this->super_IGameStatTracker)._vptr_IGameStatTracker[1])
                               (&this->super_IGameStatTracker);
    uVar3 = piVar14[3];
    iVar6 = uVar3 + 1;
    iVar15 = piVar14[1];
    if (iVar15 < iVar6) {
      iVar11 = piVar14[2];
      if (-1 < iVar11) {
        if (iVar11 == 0) {
          if ((iVar15 == 0) && (iVar15 = 8, iVar6 < 9)) {
            local_20 = 0x20;
          }
          else {
            do {
              local_20 = iVar15;
              iVar15 = local_20 * 2;
            } while (iVar15 < iVar6);
            local_20 = local_20 * 8;
          }
          goto LAB_004da526;
        }
        iVar15 = ((int)uVar3 / iVar11 + 1) * iVar11;
        if (iVar15 < iVar6) {
          if (iVar15 != 0) {
LAB_004da509:
            do {
              iVar15 = (iVar15 + iVar6) / 2;
            } while (iVar15 < iVar6);
            goto LAB_004da51c;
          }
          if (iVar6 < 0) {
            iVar15 = -1;
            local_20 = -4;
          }
          else {
            local_20 = 0;
            if (iVar6 != 0) goto LAB_004da509;
          }
        }
        else {
LAB_004da51c:
          local_20 = iVar15 * 4;
        }
LAB_004da526:
        piVar14[1] = iVar15;
        if (*piVar14 == 0) {
          iVar15 = (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x689e12))
                             ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x689e12),local_20);
          *piVar14 = iVar15;
        }
        else {
          iVar15 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x689e12) + 4))
                             ((int *)**(undefined4 **)(unaff_EBX + 0x689e12),*piVar14,local_20);
          *piVar14 = iVar15;
        }
      }
    }
    piVar14[3] = piVar14[3] + 1;
    iVar15 = *piVar14;
    piVar14[4] = iVar15;
    if (0 < (int)(~uVar3 + piVar14[3])) {
      _V_memmove((void *)(iVar15 + iVar6 * 4),(void *)(iVar15 + uVar3 * 4),(~uVar3 + piVar14[3]) * 4
                );
    }
    puVar10 = (undefined4 *)(uVar3 * 4 + *piVar14);
    if (puVar10 != (undefined4 *)0x0) {
      *puVar10 = piVar13;
    }
  }
  iVar15 = *(int *)(unaff_EBX + 0x77f632);
  uVar3 = *(uint *)(iVar15 + 0x10);
  iVar6 = uVar3 + 1;
  iVar11 = *(int *)(iVar15 + 8);
  if (iVar6 <= iVar11) goto LAB_004da230;
  iVar9 = *(int *)(iVar15 + 0xc);
  if (iVar9 < 0) goto LAB_004da230;
  if (iVar9 == 0) {
    if ((iVar11 == 0) && (iVar11 = 8, iVar6 < 9)) {
      local_24 = 0x20;
    }
    else {
      do {
        local_24 = iVar11;
        iVar11 = local_24 * 2;
      } while (iVar11 < iVar6);
      local_24 = local_24 * 8;
    }
    goto LAB_004da2a1;
  }
  iVar11 = ((int)uVar3 / iVar9 + 1) * iVar9;
  if (iVar11 < iVar6) {
    if (iVar11 != 0) {
LAB_004da440:
      do {
        iVar11 = (iVar11 + iVar6) / 2;
      } while (iVar11 < iVar6);
      goto LAB_004da453;
    }
    if (iVar6 < 0) {
      iVar11 = -1;
      local_24 = -4;
    }
    else {
      local_24 = 0;
      if (iVar6 != 0) goto LAB_004da440;
    }
  }
  else {
LAB_004da453:
    local_24 = iVar11 * 4;
  }
LAB_004da2a1:
  *(int *)(iVar15 + 8) = iVar11;
  if (*(int *)(iVar15 + 4) == 0) {
    uVar12 = (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x689e12))
                       ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x689e12),local_24);
    *(undefined4 *)(iVar15 + 4) = uVar12;
  }
  else {
    uVar12 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x689e12) + 4))
                       ((int *)**(undefined4 **)(unaff_EBX + 0x689e12),*(int *)(iVar15 + 4),local_24
                       );
    *(undefined4 *)(iVar15 + 4) = uVar12;
  }
LAB_004da230:
  *(int *)(iVar15 + 0x10) = *(int *)(iVar15 + 0x10) + 1;
  iVar11 = *(int *)(iVar15 + 4);
  *(int *)(iVar15 + 0x14) = iVar11;
  iVar9 = ~uVar3 + *(int *)(iVar15 + 0x10);
  if (0 < iVar9) {
    _V_memmove((void *)(iVar11 + iVar6 * 4),(void *)(iVar11 + uVar3 * 4),iVar9 * 4);
  }
  puVar10 = (undefined4 *)(uVar3 * 4 + *(int *)(iVar15 + 4));
  if (puVar10 != (undefined4 *)0x0) {
    *puVar10 = puVar7;
  }
  return;
}


/* C_CPortalGameStats::OnEvent at 004d9b50 */

/* DWARF original prototype: void OnEvent(C_CPortalGameStats * this, KeyValues * pEvent) */

void __thiscall C_CPortalGameStats::OnEvent(C_CPortalGameStats *this,KeyValues *pEvent)

{
  char *s2;
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * szEvent@[???] */
  ___i686_get_pc_thunk_bx();
  s2 = KeyValues::GetName(pEvent);
  iVar1 = _V_stricmp((char *)(unaff_EBX + 0x4e7c2f),s2);
  if (iVar1 != 0) {
    iVar1 = _V_stricmp((char *)(unaff_EBX + 0x4e7c57),s2);
    if (iVar1 != 0) {
      _V_stricmp((char *)(unaff_EBX + 0x4dc76e),s2);
      return;
    }
  }
  return;
}


/* _GLOBAL__I_g_PortalGameStats at 0006ad10 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_g_PortalGameStats(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

