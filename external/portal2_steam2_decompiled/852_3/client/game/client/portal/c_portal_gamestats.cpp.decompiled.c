/* DWARF-guided pseudocode for game/client/portal/c_portal_gamestats.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 0006aea0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

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
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x969].m_pPrev + unaff_EBX) = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x969].m_pPrev + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x969].m_pPrev + unaff_EBX + 2) = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x969].m_pPrev + unaff_EBX + 3) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x969].m_pNext + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96a].m_pEntity + unaff_EBX) = 0;
    *(undefined4 *)
     (&UNK_000096a8 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
         0x7f7fffff;
    *(undefined4 *)
     (&UNK_000096ac + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
         0x7f7fffff;
    *(undefined4 *)
     (&UNK_000096ac +
     (int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96b].m_pEntity + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96b].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96b].m_pPrev + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96b].m_pNext + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96c].m_pEntity + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96c].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96c].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined **)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96c].m_pNext + unaff_EBX) =
         &UNK_00ac6f54 + unaff_EBX;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96d].m_pEntity + unaff_EBX) = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96d].m_SerialNumber + unaff_EBX + 3)
         = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96d].m_SerialNumber + unaff_EBX) = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96d].m_SerialNumber + unaff_EBX + 1)
         = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96d].m_SerialNumber + unaff_EBX + 2)
         = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96d].m_pPrev + unaff_EBX) = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96d].m_pNext + unaff_EBX + 3) = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96d].m_pNext + unaff_EBX) = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96d].m_pNext + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96d].m_pNext + unaff_EBX + 2) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96e].m_pEntity + unaff_EBX) = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96e].m_SerialNumber + unaff_EBX + 3)
         = 1;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96e].m_SerialNumber + unaff_EBX) = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96e].m_SerialNumber + unaff_EBX + 1)
         = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96e].m_SerialNumber + unaff_EBX + 2)
         = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96e].m_pPrev + unaff_EBX) = 1;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96e].m_pNext + unaff_EBX + 3) = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96e].m_pNext + unaff_EBX) = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96e].m_pNext + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96e].m_pNext + unaff_EBX + 2) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96f].m_pEntity + unaff_EBX) = 2;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96f].m_SerialNumber + unaff_EBX + 3)
         = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96f].m_SerialNumber + unaff_EBX) = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96f].m_SerialNumber + unaff_EBX + 1)
         = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x96f].m_SerialNumber + unaff_EBX + 2)
         = 0;
    CBaseGameStats::CBaseGameStats((CBaseGameStats *)(s_SortBuffer + unaff_EBX + 0x3f8c));
    *(undefined **)(s_SortBuffer + unaff_EBX + 0x3f94) = &UNK_00b0bed4 + unaff_EBX;
    s_SortBuffer[unaff_EBX + 0x3f9c] = '\0';
    puVar1 = s_SortBuffer + unaff_EBX + 0x3f98;
    puVar1[0] = '*';
    puVar1[1] = '\0';
    puVar1[2] = '\0';
    puVar1[3] = '\0';
    CAutoGameSystem::CAutoGameSystem
              ((CAutoGameSystem *)(s_SortBuffer + unaff_EBX + 0x3fa0),(char *)0x0);
    *(undefined **)(s_SortBuffer + unaff_EBX + 0x3f8c) = &UNK_00b16d94 + unaff_EBX;
    *(undefined **)(s_SortBuffer + unaff_EBX + 0x3f94) = &UNK_00b16e14 + unaff_EBX;
    *(undefined **)(s_SortBuffer + unaff_EBX + 0x3fa0) = &UNK_00b16e2c + unaff_EBX;
    *(undefined **)(s_SortBuffer + unaff_EBX + 0x3fac) = &UNK_00b16e78 + unaff_EBX;
    *(undefined **)(s_SortBuffer + unaff_EBX + 0x3fb0) = &UNK_00b16e88 + unaff_EBX;
    puVar1 = s_SortBuffer + unaff_EBX + 0x3fb4;
    puVar1[0] = '\0';
    puVar1[1] = '\0';
    puVar1[2] = '\0';
    puVar1[3] = '\0';
    puVar1 = s_SortBuffer + unaff_EBX + 0x3fb8;
    puVar1[0] = '\0';
    puVar1[1] = '\0';
    puVar1[2] = '\0';
    puVar1[3] = '\0';
    puVar1 = s_SortBuffer + unaff_EBX + 0x3fbc;
    puVar1[0] = '\0';
    puVar1[1] = '\0';
    puVar1[2] = '\0';
    puVar1[3] = '\0';
    puVar1 = s_SortBuffer + unaff_EBX + 0x3fc0;
    puVar1[0] = '\0';
    puVar1[1] = '\0';
    puVar1[2] = '\0';
    puVar1[3] = '\0';
    puVar1 = s_SortBuffer + unaff_EBX + 0x3fc4;
    puVar1[0] = '\0';
    puVar1[1] = '\0';
    puVar1[2] = '\0';
    puVar1[3] = '\0';
    this = GetSteamWorksSGameStatsUploader();
    tVar2 = CSteamWorksGameStatsUploader::GetTimeSinceEpoch(this);
    *(time_t *)(s_SortBuffer + unaff_EBX + 0x3fcc) = tVar2;
    *(time_t *)(s_SortBuffer + unaff_EBX + 0x3fd0) = tVar2 >> 0x1f;
    s_SortBuffer[unaff_EBX + 0x3fc8] = '\0';
    puVar1 = s_SortBuffer + unaff_EBX + 0x409c;
    puVar1[0] = '\0';
    puVar1[1] = '\0';
    puVar1[2] = '\0';
    puVar1[3] = '\0';
    puVar1 = s_SortBuffer + unaff_EBX + 0x40a0;
    puVar1[0] = '\0';
    puVar1[1] = '\0';
    puVar1[2] = '\0';
    puVar1[3] = '\0';
    puVar1 = s_SortBuffer + unaff_EBX + 0x40a4;
    puVar1[0] = '\0';
    puVar1[1] = '\0';
    puVar1[2] = '\0';
    puVar1[3] = '\0';
    puVar1 = s_SortBuffer + unaff_EBX + 0x40a8;
    puVar1[0] = '\0';
    puVar1[1] = '\0';
    puVar1[2] = '\0';
    puVar1[3] = '\0';
    puVar1 = s_SortBuffer + unaff_EBX + 0x40ac;
    puVar1[0] = '\0';
    puVar1[1] = '\0';
    puVar1[2] = '\0';
    puVar1[3] = '\0';
    s_SortBuffer[unaff_EBX + 0x40b0] = '\0';
    ___cxa_atexit(unaff_EBX + 0x8e4f7c,0,*(undefined4 *)(&DAT_00ac5e6c + unaff_EBX));
    puVar3 = operator_new(0x14);
    *puVar3 = 0;
    puVar3[1] = 0;
    puVar3[2] = 0;
    puVar3[3] = 0;
    puVar3[4] = 0;
    *(undefined4 **)(s_SortBuffer + unaff_EBX + 0x40b4) = puVar3;
  }
  return;
}


/* __tcf_0 at 0094fe30 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* WARNING: Struct "CBaseViewport": ignoring multiple overlapping fields */

void __tcf_0(void *param_1)

{
  int *piVar1;
  int unaff_EBX;
  int in_stack_ffffffc8;
  int iVar2;
  
  ___i686_get_pc_thunk_bx();
  *(undefined **)(unaff_EBX + 0x3fd682) = &UNK_00231e0a + unaff_EBX;
  *(undefined **)(unaff_EBX + 0x3fd68a) = &UNK_00231e8a + unaff_EBX;
  *(undefined **)(unaff_EBX + 0x3fd696) = &UNK_00231ea2 + unaff_EBX;
  *(undefined **)(unaff_EBX + 0x3fd6a2) = &UNK_00231eee + unaff_EBX;
  *(undefined **)(unaff_EBX + 0x3fd6a6) = &UNK_00231efe + unaff_EBX;
  *(undefined4 *)(unaff_EBX + 0x3fd79e) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x3fd79a)) {
    iVar2 = *(int *)(CBaseViewport::SetParent + unaff_EBX + 2);
    if (iVar2 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1e0ee6) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1e0ee6));
      *(undefined4 *)(CBaseViewport::SetParent + unaff_EBX + 2) = 0;
      in_stack_ffffffc8 = iVar2;
    }
    *(undefined4 *)(unaff_EBX + 0x3fd796) = 0;
  }
  iVar2 = *(int *)(CBaseViewport::SetParent + unaff_EBX + 2);
  *(int *)(unaff_EBX + 0x3fd7a2) = iVar2;
  if (-1 < *(int *)(unaff_EBX + 0x3fd79a)) {
    if (iVar2 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1e0ee6) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1e0ee6));
      *(undefined4 *)(CBaseViewport::SetParent + unaff_EBX + 2) = 0;
      in_stack_ffffffc8 = iVar2;
    }
    *(undefined4 *)(unaff_EBX + 0x3fd796) = 0;
  }
  *(undefined4 *)(unaff_EBX + 0x3fd6b6) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x3fd6b2)) {
    iVar2 = *(int *)(unaff_EBX + 0x3fd6aa);
    if (iVar2 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1e0ee6) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1e0ee6));
      *(undefined4 *)(unaff_EBX + 0x3fd6aa) = 0;
      in_stack_ffffffc8 = iVar2;
    }
    *(undefined4 *)(unaff_EBX + 0x3fd6ae) = 0;
  }
  iVar2 = *(int *)(unaff_EBX + 0x3fd6aa);
  *(int *)(unaff_EBX + 0x3fd6ba) = iVar2;
  if (-1 < *(int *)(unaff_EBX + 0x3fd6b2)) {
    if (iVar2 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1e0ee6) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1e0ee6));
      *(undefined4 *)(unaff_EBX + 0x3fd6aa) = 0;
      in_stack_ffffffc8 = iVar2;
    }
    *(undefined4 *)(unaff_EBX + 0x3fd6ae) = 0;
  }
  *(int *)(unaff_EBX + 0x3fd696) = unaff_EBX + 0x1e31ea;
  IGameSystem::~IGameSystem((IGameSystem *)(unaff_EBX + 0x3fd696),in_stack_ffffffc8);
  *(int *)(unaff_EBX + 0x3fd68a) = unaff_EBX + 0x226f4a;
  *(undefined4 *)(unaff_EBX + 0x3fd68e) = 0xd;
  if (*(char *)(unaff_EBX + 0x3fd692) != '\0') {
    piVar1 = (int *)**(int **)(unaff_EBX + 0x1e0ef2);
    if (piVar1 != (int *)0x0) {
      (**(code **)(*piVar1 + 0x18))(piVar1,unaff_EBX + 0x3fd68a);
    }
    *(undefined1 *)(unaff_EBX + 0x3fd692) = 0;
  }
  return;
}


/* C_CPortalGameStats::Init at 004de950 */

/* DWARF original prototype: bool Init(C_CPortalGameStats * this) */

bool __thiscall C_CPortalGameStats::Init(C_CPortalGameStats *this)

{
  int *piVar1;
  int *piVar2;
  int iVar3;
  undefined4 *puVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  piVar1 = *(int **)(unaff_EBX + 0x6523f0);
  piVar2 = (int *)*piVar1;
  if (piVar2 != (int *)0x0) {
    iVar3 = (**(code **)(*piVar2 + 0x28))(piVar2);
    if (iVar3 != 0) {
      piVar1 = (int *)*piVar1;
      puVar4 = (undefined4 *)(**(code **)(*piVar1 + 0x28))(piVar1);
      (**(code **)*puVar4)(puVar4,&this->super_IMatchEventsSink);
    }
  }
  **(undefined4 **)(unaff_EBX + 0x652b80) = this;
  return true;
}


/* C_CPortalGameStats::Shutdown at 004de9d0 */

/* DWARF original prototype: void Shutdown(C_CPortalGameStats * this) */

void __thiscall C_CPortalGameStats::Shutdown(C_CPortalGameStats *this)

{
  int *piVar1;
  int iVar2;
  int *piVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  piVar1 = *(int **)(unaff_EBX + 0x652373);
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


/* C_CPortalGameStats::Event_AddSurveyData at 004debf0 */

/* DWARF original prototype: void Event_AddSurveyData(C_CPortalGameStats * this, uint32 difficulty,
   uint32 enjoyment, char * essay) */

void __thiscall
C_CPortalGameStats::Event_AddSurveyData
          (C_CPortalGameStats *this,uint32 difficulty,uint32 enjoyment,char *essay)

{
  uint uVar1;
  SSurveyData *pSVar2;
  CSteamWorksGameStatsUploader *this_00;
  time_t tVar3;
  int iVar4;
  SSurveyData **ppSVar5;
  
  pSVar2 = operator_new(0x1018);
  this_00 = GetSteamWorksSGameStatsUploader();
  tVar3 = CSteamWorksGameStatsUploader::GetTimeSinceEpoch(this_00);
  *(time_t *)&(pSVar2->super_BaseStatData).TimeSubmitted = tVar3;
  *(time_t *)((int)&(pSVar2->super_BaseStatData).TimeSubmitted + 4) = tVar3 >> 0x1f;
  (pSVar2->super_BaseStatData).m_bUseGlobalData = true;
  pSVar2->m_Difficulty = difficulty;
  pSVar2->m_Enjoyment = enjoyment;
  _memset(pSVar2->m_Essay,0,0x1001);
  if (essay != (char *)0x0) {
    V_strncpy(pSVar2->m_Essay,essay,0x1000);
  }
  uVar1 = (this->m_SurveyData).m_Size;
  CUtlVector<SSurveyData*,CUtlMemory<SSurveyData*,_int>_>::GrowVector(&this->m_SurveyData,1);
  iVar4 = ~uVar1 + (this->m_SurveyData).m_Size;
  if (0 < iVar4) {
    ppSVar5 = (this->m_SurveyData).m_Memory.m_pMemory;
    _V_memmove(ppSVar5 + uVar1 + 1,ppSVar5 + uVar1,iVar4 * 4);
  }
  ppSVar5 = (this->m_SurveyData).m_Memory.m_pMemory + uVar1;
  if (ppSVar5 != (SSurveyData **)0x0) {
    *ppSVar5 = pSVar2;
  }
  return;
}


/* C_CPortalGameStats::SubmitGameStats at 004deb70 */

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
  piVar3 = *(int **)(unaff_EBX + 0x86ea6a);
  iVar1 = piVar3[3];
                    /* Unresolved local var: int i@[???] */
  iVar4 = 0;
  if (0 < iVar1) {
    while( true ) {
      puVar2 = *(undefined4 **)(*piVar3 + iVar4 * 4);
      (**(code **)*puVar2)(puVar2,pKV);
      piVar3 = *(int **)(**(int **)(unaff_EBX + 0x86ea6a) + iVar4 * 4);
      (**(code **)(*piVar3 + 4))(piVar3);
      iVar4 = iVar4 + 1;
      if (iVar4 == iVar1) break;
      piVar3 = *(int **)(unaff_EBX + 0x86ea6a);
    }
  }
  return;
}


/* C_CPortalGameStats::AddDataForSend at 004df760 */

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


/* C_CPortalGameStats::AddData at 004df370 */

/* DWARF original prototype: void AddData(C_CPortalGameStats * this) */

void __thiscall C_CPortalGameStats::AddData(C_CPortalGameStats *this)

{
  undefined4 *puVar1;
  SSurveyData *pSVar2;
  int iVar3;
  uint uVar4;
  KeyValues *this_00;
  char *value;
  undefined4 *puVar5;
  int iVar6;
  int *piVar7;
  int *piVar8;
  CPortal_StuckPlayer **ppCVar9;
  SSurveyData **ppSVar10;
  int unaff_EBX;
  int iVar11;
  KeyValues *pKV;
  int local_34;
  int local_20;
  
                    /* Unresolved local var: char * pzMapName@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = KeyValues::operator_new(0x24);
  KeyValues::KeyValues(this_00,(char *)(unaff_EBX + 0x4ac747));
  if (this_00 == (KeyValues *)0x0) {
    return;
  }
  value = IGameSystem::MapName();
  if (value == (char *)0x0) {
    value = (char *)(unaff_EBX + 0x47eeee);
  }
  KeyValues::SetString(this_00,(char *)(unaff_EBX + 0x48d1c3),value);
  if (0 < (this->m_SurveyData).m_Size) {
    local_34 = 0;
    puVar1 = *(undefined4 **)(unaff_EBX + 0x6519a6);
    do {
      pSVar2 = (this->m_SurveyData).m_Memory.m_pMemory[local_34];
      if (*(int *)(unaff_EBX + 0x7442de) == 0) {
        piVar7 = operator_new(0x18);
        piVar7[1] = 0;
        piVar7[2] = 0;
        piVar7[3] = 0;
        piVar7[4] = 0;
        piVar7[5] = 0;
        *piVar7 = unaff_EBX + 0x6a2aca;
        *(int **)(unaff_EBX + 0x7442de) = piVar7;
        piVar8 = (int *)(*(this->super_IGameStatTracker)._vptr_IGameStatTracker[1])
                                  (&this->super_IGameStatTracker);
        uVar4 = piVar8[3];
        iVar3 = uVar4 + 1;
        iVar6 = piVar8[1];
        if (iVar6 < iVar3) {
          iVar11 = piVar8[2];
          if (-1 < iVar11) {
            if (iVar11 == 0) {
              if ((iVar6 == 0) && (iVar6 = 8, iVar3 < 9)) {
                local_20 = 0x20;
              }
              else {
                do {
                  local_20 = iVar6;
                  iVar6 = local_20 * 2;
                } while (iVar6 < iVar3);
                local_20 = local_20 * 8;
              }
              goto LAB_004df6e1;
            }
            iVar11 = ((int)uVar4 / iVar11 + 1) * iVar11;
            if (iVar11 < iVar3) {
              if (iVar11 != 0) {
LAB_004df559:
                do {
                  iVar11 = (iVar11 + iVar3) / 2;
                } while (iVar11 < iVar3);
                goto LAB_004df56c;
              }
              if (iVar3 < 0) {
                iVar6 = -1;
                local_20 = -4;
              }
              else {
                local_20 = 0;
                iVar6 = 0;
                if (iVar3 != 0) goto LAB_004df559;
              }
LAB_004df6e1:
              piVar8[1] = iVar6;
              iVar6 = *piVar8;
            }
            else {
LAB_004df56c:
              local_20 = iVar11 * 4;
              piVar8[1] = iVar11;
              iVar6 = *piVar8;
            }
            if (iVar6 == 0) {
              iVar6 = (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x6519a6))
                                ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x6519a6),local_20);
              *piVar8 = iVar6;
            }
            else {
              iVar6 = (**(code **)(*(int *)*puVar1 + 4))((int *)*puVar1,iVar6,local_20);
              *piVar8 = iVar6;
            }
          }
        }
        piVar8[3] = piVar8[3] + 1;
        iVar6 = *piVar8;
        piVar8[4] = iVar6;
        if (0 < (int)(~uVar4 + piVar8[3])) {
          _V_memmove((void *)(iVar6 + iVar3 * 4),(void *)(iVar6 + uVar4 * 4),
                     (~uVar4 + piVar8[3]) * 4);
        }
        puVar5 = (undefined4 *)(uVar4 * 4 + *piVar8);
        if (puVar5 != (undefined4 *)0x0) {
          *puVar5 = piVar7;
        }
      }
      iVar3 = *(int *)(unaff_EBX + 0x7442de);
      uVar4 = *(uint *)(iVar3 + 0x10);
      CUtlVector<SSurveyData*,CUtlMemory<SSurveyData*,_int>_>::GrowVector
                ((CUtlVector<SSurveyData*,CUtlMemory<SSurveyData*,_int>_> *)(iVar3 + 4),1);
      iVar6 = ~uVar4 + *(int *)(iVar3 + 0x10);
      if (iVar6 < 1) {
        iVar11 = uVar4 << 2;
      }
      else {
        iVar11 = uVar4 * 4;
        _V_memmove((void *)(*(int *)(iVar3 + 4) + 4 + uVar4 * 4),
                   (void *)(*(int *)(iVar3 + 4) + iVar11),iVar6 * 4);
      }
      puVar5 = (undefined4 *)(iVar11 + *(int *)(iVar3 + 4));
      if (puVar5 != (undefined4 *)0x0) {
        *puVar5 = pSVar2;
      }
      local_34 = local_34 + 1;
    } while (local_34 < (this->m_SurveyData).m_Size);
  }
  (*(this->super_CBaseGameStats)._vptr_CBaseGameStats[0x17])(this,this_00);
  (this->m_SurveyData).m_Size = 0;
  if ((this->m_SurveyData).m_Memory.m_nGrowSize < 0) {
    ppSVar10 = (this->m_SurveyData).m_Memory.m_pMemory;
  }
  else {
    ppSVar10 = (this->m_SurveyData).m_Memory.m_pMemory;
    if (ppSVar10 != (SSurveyData **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x6519a6) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x6519a6),ppSVar10);
      (this->m_SurveyData).m_Memory.m_pMemory = (SSurveyData **)0x0;
    }
    ppSVar10 = (SSurveyData **)0x0;
    (this->m_SurveyData).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_SurveyData).m_pElements = ppSVar10;
  (this->m_VectorStuckPlayer).m_Size = 0;
  if ((this->m_VectorStuckPlayer).m_Memory.m_nGrowSize < 0) {
    ppCVar9 = (this->m_VectorStuckPlayer).m_Memory.m_pMemory;
  }
  else {
    ppCVar9 = (this->m_VectorStuckPlayer).m_Memory.m_pMemory;
    if (ppCVar9 != (CPortal_StuckPlayer **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x6519a6) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x6519a6),ppCVar9);
      (this->m_VectorStuckPlayer).m_Memory.m_pMemory = (CPortal_StuckPlayer **)0x0;
    }
    ppCVar9 = (CPortal_StuckPlayer **)0x0;
    (this->m_VectorStuckPlayer).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_VectorStuckPlayer).m_pElements = ppCVar9;
  KeyValues::deleteThis(this_00);
  return;
}


/* C_CPortalGameStats::OnSessionStart at 004df340 */

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


/* C_CPortalGameStats::OnSessionEnd at 004deb20 */

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


/* C_CPortalGameStats::Event_LevelInit at 004deaa0 */

/* DWARF original prototype: void Event_LevelInit(C_CPortalGameStats * this) */

void __thiscall C_CPortalGameStats::Event_LevelInit(C_CPortalGameStats *this)

{
  char *pcVar1;
  _func_int_varargs *UNRECOVERED_JUMPTABLE;
  int unaff_EBX;
  
                    /* Unresolved local var: char * name1@[???] */
  ___i686_get_pc_thunk_bx();
  pcVar1 = IGameSystem::MapName();
  pcVar1 = V_stristr(pcVar1,(char *)(unaff_EBX + 0x4b6dd9));
  if (pcVar1 == (char *)0x0) {
    if (this->m_bSessionActive == false) {
      UNRECOVERED_JUMPTABLE = (this->super_CBaseGameStats)._vptr_CBaseGameStats[0x18];
      goto LAB_004deae0;
    }
  }
  else if (this->m_bSessionActive == false) {
    return;
  }
  UNRECOVERED_JUMPTABLE = (this->super_CBaseGameStats)._vptr_CBaseGameStats[0x19];
LAB_004deae0:
                    /* WARNING: Could not recover jumptable at 0x004deae9. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}


/* C_CPortalGameStats::GetConfigData at 004ded00 */

/* DWARF original prototype: void GetConfigData(C_CPortalGameStats * this) */

void __thiscall C_CPortalGameStats::GetConfigData(C_CPortalGameStats *this)

{
  float *pfVar1;
  code cVar2;
  uint uVar3;
  bool bVar4;
  char *pcVar5;
  int iVar6;
  undefined1 *puVar7;
  CSteamWorksGameStatsUploader *this_00;
  time_t tVar8;
  int iVar9;
  undefined4 *puVar10;
  int *piVar11;
  int *piVar12;
  int unaff_EBX;
  int iVar13;
  ConVarRef *local_34;
  int local_20;
  
                    /* Unresolved local var: ConVar in_joystick@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x652736) + 0x1c) + 0x30) != 0) {
    (this->m_ConfigData).m_AllowGamepad = '\x01';
  }
  piVar11 = *(int **)(&DAT_0065204a + unaff_EBX);
  if ((((*piVar11 == 0) || (iVar6 = **(int **)(unaff_EBX + 0x65203e), iVar6 == 0)) ||
      (*(int *)(iVar6 + 0x14) == 0)) || (piVar12 = *(int **)(iVar6 + 8), piVar12 == (int *)0x0)) {
    V_strncpy((this->m_ConfigData).m_AudioLanguage,(char *)(unaff_EBX + 0x4cff56),0x40);
    V_strncpy((this->m_ConfigData).m_TextLanguage,(char *)(unaff_EBX + 0x4cff56),0x40);
LAB_004dee59:
    cVar2 = CParticleCollection::CheckIfOperatorShouldRun[unaff_EBX + 2];
  }
  else {
    pcVar5 = (char *)(**(code **)(*piVar12 + 0x10))(piVar12);
    if (pcVar5 != (char *)0x0) {
      V_strncpy((this->m_ConfigData).m_CountryCode,pcVar5,0x40);
    }
    piVar11 = (int *)*piVar11;
    (**(code **)(*piVar11 + 0x170))(piVar11,(this->m_ConfigData).m_TextLanguage,0x40);
    piVar11 = *(int **)(**(int **)(unaff_EBX + 0x65203e) + 0x14);
    pcVar5 = (char *)(**(code **)(*piVar11 + 0x10))(piVar11);
    if (pcVar5 == (char *)0x0) {
      V_strncpy((this->m_ConfigData).m_AudioLanguage,(char *)(unaff_EBX + 0x4cff56),0x40);
      goto LAB_004dee59;
    }
    V_strncpy((this->m_ConfigData).m_AudioLanguage,pcVar5,0x40);
    cVar2 = CParticleCollection::CheckIfOperatorShouldRun[unaff_EBX + 2];
  }
  if (cVar2 == (code)0x0) {
    iVar6 = ___cxa_guard_acquire(unaff_EBX + 0x7d3a02);
    if (iVar6 != 0) {
      ConVarRef::ConVarRef((ConVarRef *)(unaff_EBX + 0x7d3a4a),(char *)(unaff_EBX + 0x4aa44a));
      ___cxa_guard_release(unaff_EBX + 0x7d3a02);
    }
  }
  if ((*(char *)(unaff_EBX + 0x7d3a0a) == '\0') &&
     (iVar6 = ___cxa_guard_acquire(unaff_EBX + 0x7d3a0a), iVar6 != 0)) {
    ConVarRef::ConVarRef((ConVarRef *)(unaff_EBX + 0x7d3a42),&UNK_004bdaee + unaff_EBX);
    ___cxa_guard_release(unaff_EBX + 0x7d3a0a);
  }
  if ((*(char *)(unaff_EBX + 0x7d3a12) == '\0') &&
     (iVar6 = ___cxa_guard_acquire(unaff_EBX + 0x7d3a12), iVar6 != 0)) {
    ConVarRef::ConVarRef((ConVarRef *)(unaff_EBX + 0x7d3a3a),&UNK_0048c28e + unaff_EBX);
    ___cxa_guard_release(unaff_EBX + 0x7d3a12);
  }
  if ((*(char *)(unaff_EBX + 0x7d3a1a) == '\0') &&
     (iVar6 = ___cxa_guard_acquire(unaff_EBX + 0x7d3a1a), iVar6 != 0)) {
    ConVarRef::ConVarRef((ConVarRef *)(unaff_EBX + 0x7d3a32),(char *)(unaff_EBX + 0x4acda9));
    ___cxa_guard_release(unaff_EBX + 0x7d3a1a);
  }
  if (*(char *)(unaff_EBX + 0x7d3a22) == '\0') {
    iVar6 = ___cxa_guard_acquire(unaff_EBX + 0x7d3a22);
    if (iVar6 != 0) {
      ConVarRef::ConVarRef((ConVarRef *)(unaff_EBX + 0x7d3a2a),&UNK_004ac4a6 + unaff_EBX);
      ___cxa_guard_release(unaff_EBX + 0x7d3a22);
    }
  }
  local_34 = (ConVarRef *)(unaff_EBX + 0x7d3a2a);
  bVar4 = ConVarRef::IsValid((ConVarRef *)(unaff_EBX + 0x7d3a4a));
  if ((bVar4) &&
     (pfVar1 = (float *)(*(int *)(unaff_EBX + 0x7d3a4e) + 0x2c), *pfVar1 <= 0.0 && *pfVar1 != 0.0))
  {
    (this->m_ConfigData).m_ReverseMouse = '\x01';
  }
  bVar4 = ConVarRef::IsValid((ConVarRef *)(unaff_EBX + 0x7d3a42));
  if ((bVar4) && (*(int *)(*(int *)(unaff_EBX + 0x7d3a46) + 0x30) != 0)) {
    (this->m_ConfigData).m_AllowConsole = '\x01';
  }
  bVar4 = ConVarRef::IsValid((ConVarRef *)(unaff_EBX + 0x7d3a32));
  if (bVar4) {
    (this->m_ConfigData).m_ColorBlind =
         (uint8)*(undefined4 *)(*(int *)(unaff_EBX + 0x7d3a36) + 0x30);
  }
  bVar4 = ConVarRef::IsValid(local_34);
  if (bVar4) {
    (this->m_ConfigData).m_OpenMic = (uint8)*(undefined4 *)(*(int *)(unaff_EBX + 0x7d3a2e) + 0x30);
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
  if (*(int *)(unaff_EBX + 0x744942) != 0) goto LAB_004defb6;
  piVar11 = operator_new(0x18);
  piVar11[1] = 0;
  piVar11[2] = 0;
  piVar11[3] = 0;
  piVar11[4] = 0;
  piVar11[5] = 0;
  *piVar11 = unaff_EBX + 0x6a30ba;
  *(int **)(unaff_EBX + 0x744942) = piVar11;
  piVar12 = (int *)(*(this->super_IGameStatTracker)._vptr_IGameStatTracker[1])
                             (&this->super_IGameStatTracker);
  uVar3 = piVar12[3];
  iVar6 = uVar3 + 1;
  iVar9 = piVar12[1];
  if (iVar9 < iVar6) {
    iVar13 = piVar12[2];
    if (-1 < iVar13) {
      if (iVar13 == 0) {
        if ((iVar9 == 0) && (iVar9 = 8, iVar6 < 9)) {
          local_20 = 0x20;
        }
        else {
          do {
            local_20 = iVar9;
            iVar9 = local_20 * 2;
          } while (iVar9 < iVar6);
          local_20 = local_20 * 8;
        }
        goto LAB_004df1f4;
      }
      iVar9 = ((int)uVar3 / iVar13 + 1) * iVar13;
      if (iVar9 < iVar6) {
        if (iVar9 != 0) {
LAB_004df1d7:
          do {
            iVar9 = (iVar9 + iVar6) / 2;
          } while (iVar9 < iVar6);
          goto LAB_004df1ea;
        }
        if (iVar6 < 0) {
          iVar9 = -1;
          local_20 = -4;
        }
        else {
          local_20 = 0;
          if (iVar6 != 0) goto LAB_004df1d7;
        }
      }
      else {
LAB_004df1ea:
        local_20 = iVar9 * 4;
      }
LAB_004df1f4:
      piVar12[1] = iVar9;
      if (*piVar12 == 0) {
        iVar9 = (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x652016))
                          ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x652016),local_20);
        *piVar12 = iVar9;
      }
      else {
        iVar9 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x652016) + 4))
                          ((int *)**(undefined4 **)(unaff_EBX + 0x652016),*piVar12,local_20);
        *piVar12 = iVar9;
      }
    }
  }
  piVar12[3] = piVar12[3] + 1;
  iVar9 = *piVar12;
  piVar12[4] = iVar9;
  if (0 < (int)(~uVar3 + piVar12[3])) {
    _V_memmove((void *)(iVar9 + iVar6 * 4),(void *)(iVar9 + uVar3 * 4),(~uVar3 + piVar12[3]) * 4);
  }
  puVar10 = (undefined4 *)(uVar3 * 4 + *piVar12);
  if (puVar10 != (undefined4 *)0x0) {
    *puVar10 = piVar11;
  }
LAB_004defb6:
  iVar6 = *(int *)(unaff_EBX + 0x744942);
  uVar3 = *(uint *)(iVar6 + 0x10);
  CUtlVector<SConfigStats*,CUtlMemory<SConfigStats*,_int>_>::GrowVector
            ((CUtlVector<SConfigStats*,CUtlMemory<SConfigStats*,_int>_> *)(iVar6 + 4),1);
  iVar9 = ~uVar3 + *(int *)(iVar6 + 0x10);
  if (iVar9 < 1) {
    iVar13 = uVar3 << 2;
  }
  else {
    iVar13 = uVar3 * 4;
    _V_memmove((void *)(*(int *)(iVar6 + 4) + 4 + uVar3 * 4),(void *)(*(int *)(iVar6 + 4) + iVar13),
               iVar9 * 4);
  }
  puVar10 = (undefined4 *)(iVar13 + *(int *)(iVar6 + 4));
  if (puVar10 != (undefined4 *)0x0) {
    *puVar10 = puVar7;
  }
  return;
}


/* C_CPortalGameStats::OnEvent at 004dea30 */

/* DWARF original prototype: void OnEvent(C_CPortalGameStats * this, KeyValues * pEvent) */

void __thiscall C_CPortalGameStats::OnEvent(C_CPortalGameStats *this,KeyValues *pEvent)

{
  char *s2;
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * szEvent@[???] */
  ___i686_get_pc_thunk_bx();
  s2 = KeyValues::GetName(pEvent);
  iVar1 = _V_stricmp((char *)(unaff_EBX + 0x4b7eab),s2);
  if (iVar1 != 0) {
    iVar1 = _V_stricmp((char *)(unaff_EBX + 0x4b7ed3),s2);
    if (iVar1 != 0) {
      _V_stricmp((char *)(unaff_EBX + 0x4acf79),s2);
      return;
    }
  }
  return;
}


/* _GLOBAL__I_g_PortalGameStats at 0006b220 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_g_PortalGameStats(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

