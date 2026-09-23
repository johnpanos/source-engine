/* DWARF-guided pseudocode for game/server/portal2/paint_database.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000ad940 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  undefined4 uVar1;
  int unaff_EBX;
  longlong lVar2;
  ConVar *in_stack_ffffffe4;
  char *in_stack_ffffffe8;
  char *in_stack_ffffffec;
  int in_stack_fffffff0;
  
  lVar2 = ___i686_get_pc_thunk_bx();
  if (lVar2 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x21e].m_pPrev + unaff_EBX + 3)
       = 0;
  *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x21e].m_pNext + unaff_EBX) = 0
  ;
  *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x21e].m_pNext + unaff_EBX + 1)
       = 0;
  *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x21e].m_pNext + unaff_EBX + 2)
       = 0;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x21e].m_pNext + unaff_EBX + 3)
       = 0;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x21f].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x21f].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x21f].m_pPrev + unaff_EBX + 3)
       = 0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x21f].m_pNext + unaff_EBX + 3)
       = 0;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x220].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x220].m_SerialNumber + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x220].m_pPrev + unaff_EBX + 3)
       = 0;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x220].m_pNext + unaff_EBX + 3)
       = 0x7f7fffff;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x221].m_pEntity + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x221].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x221].m_pPrev + unaff_EBX + 3)
       = 0x7f7fffff;
  *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x221].m_pNext + unaff_EBX + 3)
       = &UNK_00b9e9b7 + unaff_EBX;
  CPaintDatabase::CPaintDatabase
            ((CPaintDatabase *)((int)&g_pszHintDescriptions[0x11].pszDesc + unaff_EBX + 3),
             (char *)(unaff_EBX + 0xa04ab6));
  uVar1 = *(undefined4 *)(&DAT_00b99203 + unaff_EBX);
  ___cxa_atexit(unaff_EBX + 0x99033f,0,uVar1);
  ConVar::ConVar((ConVar *)((int)DataMapInit<CAI_Hint>::dataDesc[1].flatOffset + unaff_EBX + -0x25),
                 (char *)(unaff_EBX + 0xa04acb),(char *)(unaff_EBX + 0xa04ac5),0,in_stack_ffffffe4,
                 in_stack_ffffffe8,in_stack_ffffffec,in_stack_fffffff0);
  ___cxa_atexit(unaff_EBX + 0x99031f,0,uVar1);
  ConCommand::ConCommand
            ((ConCommand *)
             ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x222].m_pPrev + unaff_EBX + 3),
             (char *)(unaff_EBX + 0xa04af4),(FnCommandCallbackV1_t)(unaff_EBX + 0x68605f),
             (char *)0x0,0,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x9902ff,0,uVar1);
  ConCommand::ConCommand
            ((ConCommand *)
             ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x226].m_pPrev + unaff_EBX + 3),
             (char *)(unaff_EBX + 0xa04b03),(FnCommandCallback_t)(unaff_EBX + 0x685c7f),(char *)0x0,
             0,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x9902df,0,uVar1);
  ConCommand::ConCommand
            ((ConCommand *)
             ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x22a].m_pPrev + unaff_EBX + 3),
             (char *)(unaff_EBX + 0xa04b0b),(FnCommandCallback_t)(unaff_EBX + 0x685e0f),(char *)0x0,
             0,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x9902bf,0,uVar1);
  return;
}


/* __tcf_0 at 00a3dc90 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  CPaintDatabase::~CPaintDatabase
            ((CPaintDatabase *)(&UNK_0038b388 + extraout_ECX),in_stack_00000008);
  return;
}


/* CPaintDatabase::CPaintDatabase at 000ad860 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void CPaintDatabase(CPaintDatabase * this, char * name) */

void __thiscall CPaintDatabase::CPaintDatabase(CPaintDatabase *this,char *name)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CAutoGameSystemPerFrame::CAutoGameSystemPerFrame(&this->super_CAutoGameSystemPerFrame,name);
  (this->super_CGameEventListener).m_bRegisteredForEvents = false;
  (this->super_CGameEventListener).m_nDebugID = 0x2a;
  (this->super_CAutoGameSystemPerFrame).super_CBaseGameSystemPerFrame.super_IGameSystemPerFrame.
  super_IGameSystem._vptr_IGameSystem = (_func_int_varargs **)(&UNK_00c2005b + unaff_EBX);
  (this->super_CGameEventListener).super_IGameEventListener2._vptr_IGameEventListener2 =
       (_func_int_varargs **)(&UNK_00c200ab + unaff_EBX);
  (this->m_PaintThisFrame).m_Memory.m_pMemory = (PaintLocationData_t *)0x0;
  (this->m_PaintThisFrame).m_Memory.m_nAllocationCount = 0;
  (this->m_PaintThisFrame).m_Memory.m_nGrowSize = 0;
  (this->m_PaintThisFrame).m_pElements = (PaintLocationData_t *)0x0;
  (this->m_PaintedEntities).m_Memory.m_pMemory = (PaintEntityData_t *)0x0;
  (this->m_PaintedEntities).m_Memory.m_nAllocationCount = 0;
  (this->m_PaintedEntities).m_Memory.m_nGrowSize = 0;
  (this->m_PaintedEntities).m_pElements = (PaintEntityData_t *)0x0;
  (this->m_PaintedProjectedWalls).m_Memory.m_pMemory = (CProjectedWallEntity **)0x0;
  (this->m_PaintedProjectedWalls).m_Memory.m_nAllocationCount = 0;
  (this->m_PaintedProjectedWalls).m_Memory.m_nGrowSize = 0;
  (this->m_PaintedProjectedWalls).m_pElements = (CProjectedWallEntity **)0x0;
  (this->m_Paintmaps).m_Memory.m_pMemory =
       (CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *)0x0;
  (this->m_Paintmaps).m_Memory.m_nAllocationCount = 0;
  (this->m_Paintmaps).m_Memory.m_nGrowSize = 0;
  (this->m_Paintmaps).m_Size = 0;
  (this->m_Paintmaps).m_pElements = (CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *)0x0
  ;
  (this->m_PaintThisFrame).m_Size = 0;
  (this->m_PaintedEntities).m_Size = 0;
  (this->m_PaintedProjectedWalls).m_Size = 0;
  return;
}


/* CPaintDatabase::CPaintDatabase at 00733db0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void CPaintDatabase(CPaintDatabase * this, char * name, CPaintDatabase
   * this, char * name) */

void __thiscall
CPaintDatabase::CPaintDatabase(CPaintDatabase *this,char *name,CPaintDatabase *this_1,char *name_1)

{
  CPaintDatabase(this,name);
  return;
}


/* CPaintDatabase::~CPaintDatabase at 007360c0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void ~CPaintDatabase(CPaintDatabase * this, int __in_chrg) */

void __thiscall CPaintDatabase::~CPaintDatabase(CPaintDatabase *this,int __in_chrg)

{
  undefined4 *puVar1;
  int iVar2;
  PaintLocationData_t *pPVar3;
  int unaff_EBX;
  int *piVar4;
  int iVar5;
  PaintLocationData_t *in_stack_ffffffc8;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CAutoGameSystemPerFrame).super_CBaseGameSystemPerFrame.super_IGameSystemPerFrame.
  super_IGameSystem._vptr_IGameSystem = (_func_int_varargs **)(unaff_EBX + 0x5977fa);
  (this->super_CGameEventListener).super_IGameEventListener2._vptr_IGameEventListener2 =
       (_func_int_varargs **)(unaff_EBX + 0x59784a);
  (this->m_PaintThisFrame).m_Size = 0;
  (this->m_PaintedEntities).m_Size = 0;
  (this->m_PaintedProjectedWalls).m_Size = 0;
  local_20 = (this->m_Paintmaps).m_Size;
  puVar1 = *(undefined4 **)(unaff_EBX + 0x510a8e);
  iVar2 = local_20 * 0x14;
  while( true ) {
    iVar5 = iVar2 + -0x14;
    local_20 = local_20 + -1;
    if (local_20 < 0) break;
    piVar4 = (int *)((int)&(this->m_Paintmaps).m_Memory.m_pMemory[-1].m_Memory.m_pMemory + iVar2);
    piVar4[3] = 0;
    if (piVar4[2] < 0) {
      pPVar3 = (PaintLocationData_t *)*piVar4;
    }
    else {
      pPVar3 = (PaintLocationData_t *)*piVar4;
      if (pPVar3 != (PaintLocationData_t *)0x0) {
        in_stack_ffffffc8 = pPVar3;
        (**(code **)(*(int *)*puVar1 + 8))((int *)*puVar1);
        pPVar3 = (PaintLocationData_t *)0x0;
        *piVar4 = 0;
      }
      piVar4[1] = 0;
    }
    piVar4[4] = (int)pPVar3;
    iVar2 = iVar5;
    if (-1 < piVar4[2]) {
      if (pPVar3 != (PaintLocationData_t *)0x0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x510a8e) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x510a8e));
        *piVar4 = 0;
        in_stack_ffffffc8 = pPVar3;
      }
      piVar4[1] = 0;
    }
  }
  (this->m_Paintmaps).m_Size = 0;
  if ((this->m_Paintmaps).m_Memory.m_nGrowSize < 0) {
    pPVar3 = (PaintLocationData_t *)(this->m_Paintmaps).m_Memory.m_pMemory;
  }
  else {
    pPVar3 = (PaintLocationData_t *)(this->m_Paintmaps).m_Memory.m_pMemory;
    if (pPVar3 != (PaintLocationData_t *)0x0) {
      in_stack_ffffffc8 = pPVar3;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x510a8e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x510a8e));
      (this->m_Paintmaps).m_Memory.m_pMemory =
           (CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *)0x0;
      pPVar3 = (PaintLocationData_t *)0x0;
    }
    (this->m_Paintmaps).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_Paintmaps).m_pElements =
       (CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *)pPVar3;
  if (-1 < (this->m_Paintmaps).m_Memory.m_nGrowSize) {
    if (pPVar3 != (PaintLocationData_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x510a8e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x510a8e));
      (this->m_Paintmaps).m_Memory.m_pMemory =
           (CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *)0x0;
      in_stack_ffffffc8 = pPVar3;
    }
    (this->m_Paintmaps).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintedProjectedWalls).m_Size = 0;
  if ((this->m_PaintedProjectedWalls).m_Memory.m_nGrowSize < 0) {
    pPVar3 = (PaintLocationData_t *)(this->m_PaintedProjectedWalls).m_Memory.m_pMemory;
  }
  else {
    pPVar3 = (PaintLocationData_t *)(this->m_PaintedProjectedWalls).m_Memory.m_pMemory;
    if (pPVar3 != (PaintLocationData_t *)0x0) {
      in_stack_ffffffc8 = pPVar3;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x510a8e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x510a8e));
      pPVar3 = (PaintLocationData_t *)0x0;
      (this->m_PaintedProjectedWalls).m_Memory.m_pMemory = (CProjectedWallEntity **)0x0;
    }
    (this->m_PaintedProjectedWalls).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintedProjectedWalls).m_pElements = (CProjectedWallEntity **)pPVar3;
  if (-1 < (this->m_PaintedProjectedWalls).m_Memory.m_nGrowSize) {
    if (pPVar3 != (PaintLocationData_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x510a8e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x510a8e));
      (this->m_PaintedProjectedWalls).m_Memory.m_pMemory = (CProjectedWallEntity **)0x0;
      in_stack_ffffffc8 = pPVar3;
    }
    (this->m_PaintedProjectedWalls).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintedEntities).m_Size = 0;
  if ((this->m_PaintedEntities).m_Memory.m_nGrowSize < 0) {
    pPVar3 = (PaintLocationData_t *)(this->m_PaintedEntities).m_Memory.m_pMemory;
  }
  else {
    pPVar3 = (PaintLocationData_t *)(this->m_PaintedEntities).m_Memory.m_pMemory;
    if (pPVar3 != (PaintLocationData_t *)0x0) {
      in_stack_ffffffc8 = pPVar3;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x510a8e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x510a8e));
      pPVar3 = (PaintLocationData_t *)0x0;
      (this->m_PaintedEntities).m_Memory.m_pMemory = (PaintEntityData_t *)0x0;
    }
    (this->m_PaintedEntities).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintedEntities).m_pElements = (PaintEntityData_t *)pPVar3;
  if (-1 < (this->m_PaintedEntities).m_Memory.m_nGrowSize) {
    if (pPVar3 != (PaintLocationData_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x510a8e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x510a8e));
      (this->m_PaintedEntities).m_Memory.m_pMemory = (PaintEntityData_t *)0x0;
      in_stack_ffffffc8 = pPVar3;
    }
    (this->m_PaintedEntities).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintThisFrame).m_Size = 0;
  if ((this->m_PaintThisFrame).m_Memory.m_nGrowSize < 0) {
    pPVar3 = (this->m_PaintThisFrame).m_Memory.m_pMemory;
  }
  else {
    pPVar3 = (this->m_PaintThisFrame).m_Memory.m_pMemory;
    if (pPVar3 != (PaintLocationData_t *)0x0) {
      in_stack_ffffffc8 = pPVar3;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x510a8e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x510a8e));
      pPVar3 = (PaintLocationData_t *)0x0;
      (this->m_PaintThisFrame).m_Memory.m_pMemory = (PaintLocationData_t *)0x0;
    }
    (this->m_PaintThisFrame).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintThisFrame).m_pElements = pPVar3;
  if (-1 < (this->m_PaintThisFrame).m_Memory.m_nGrowSize) {
    if (pPVar3 != (PaintLocationData_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x510a8e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x510a8e));
      (this->m_PaintThisFrame).m_Memory.m_pMemory = (PaintLocationData_t *)0x0;
      in_stack_ffffffc8 = pPVar3;
    }
    (this->m_PaintThisFrame).m_Memory.m_nAllocationCount = 0;
  }
  pPVar3 = (PaintLocationData_t *)&this->super_CGameEventListener;
  (this->super_CGameEventListener).super_IGameEventListener2._vptr_IGameEventListener2 =
       (_func_int_varargs **)(unaff_EBX + 0x5127ba);
  (this->super_CGameEventListener).m_nDebugID = 0xd;
  if ((this->super_CGameEventListener).m_bRegisteredForEvents != false) {
    piVar4 = (int *)**(int **)(unaff_EBX + 0x510aba);
    if (piVar4 != (int *)0x0) {
      (**(code **)(*piVar4 + 0x18))(piVar4);
      in_stack_ffffffc8 = pPVar3;
    }
    (this->super_CGameEventListener).m_bRegisteredForEvents = false;
  }
  (this->super_CAutoGameSystemPerFrame).super_CBaseGameSystemPerFrame.super_IGameSystemPerFrame.
  super_IGameSystem._vptr_IGameSystem = (_func_int_varargs **)(&UNK_0051229a + unaff_EBX);
  IGameSystemPerFrame::~IGameSystemPerFrame((IGameSystemPerFrame *)this,(int)in_stack_ffffffc8);
  operator_delete(this);
  return;
}


/* CPaintDatabase::~CPaintDatabase at 00736750 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void ~CPaintDatabase(CPaintDatabase * this, int __in_chrg) */

void __thiscall CPaintDatabase::~CPaintDatabase(CPaintDatabase *this,int __in_chrg)

{
  undefined4 *puVar1;
  int iVar2;
  int iVar3;
  CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *pCVar4;
  CProjectedWallEntity **ppCVar5;
  PaintEntityData_t *pPVar6;
  PaintLocationData_t *pPVar7;
  int unaff_EBX;
  int *piVar8;
  int iVar9;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CAutoGameSystemPerFrame).super_CBaseGameSystemPerFrame.super_IGameSystemPerFrame.
  super_IGameSystem._vptr_IGameSystem = (_func_int_varargs **)(unaff_EBX + 0x59716a);
  (this->super_CGameEventListener).super_IGameEventListener2._vptr_IGameEventListener2 =
       (_func_int_varargs **)(unaff_EBX + 0x5971ba);
  (this->m_PaintThisFrame).m_Size = 0;
  (this->m_PaintedEntities).m_Size = 0;
  (this->m_PaintedProjectedWalls).m_Size = 0;
  local_20 = (this->m_Paintmaps).m_Size;
  puVar1 = *(undefined4 **)(unaff_EBX + 0x5103fe);
  iVar2 = local_20 * 0x14;
  while( true ) {
    iVar9 = iVar2 + -0x14;
    local_20 = local_20 + -1;
    if (local_20 < 0) break;
    piVar8 = (int *)((int)&(this->m_Paintmaps).m_Memory.m_pMemory[-1].m_Memory.m_pMemory + iVar2);
    piVar8[3] = 0;
    if (piVar8[2] < 0) {
      iVar3 = *piVar8;
    }
    else {
      if (*piVar8 != 0) {
        (**(code **)(*(int *)*puVar1 + 8))((int *)*puVar1,*piVar8);
        *piVar8 = 0;
      }
      iVar3 = 0;
      piVar8[1] = 0;
    }
    piVar8[4] = iVar3;
    iVar2 = iVar9;
    if (-1 < piVar8[2]) {
      if (iVar3 != 0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5103fe) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x5103fe),iVar3);
        *piVar8 = 0;
      }
      piVar8[1] = 0;
    }
  }
  (this->m_Paintmaps).m_Size = 0;
  if ((this->m_Paintmaps).m_Memory.m_nGrowSize < 0) {
    pCVar4 = (this->m_Paintmaps).m_Memory.m_pMemory;
  }
  else {
    pCVar4 = (this->m_Paintmaps).m_Memory.m_pMemory;
    if (pCVar4 != (CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5103fe) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x5103fe),pCVar4);
      (this->m_Paintmaps).m_Memory.m_pMemory =
           (CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *)0x0;
    }
    pCVar4 = (CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *)0x0;
    (this->m_Paintmaps).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_Paintmaps).m_pElements = pCVar4;
  if (-1 < (this->m_Paintmaps).m_Memory.m_nGrowSize) {
    if (pCVar4 != (CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5103fe) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x5103fe),pCVar4);
      (this->m_Paintmaps).m_Memory.m_pMemory =
           (CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *)0x0;
    }
    (this->m_Paintmaps).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintedProjectedWalls).m_Size = 0;
  if ((this->m_PaintedProjectedWalls).m_Memory.m_nGrowSize < 0) {
    ppCVar5 = (this->m_PaintedProjectedWalls).m_Memory.m_pMemory;
  }
  else {
    ppCVar5 = (this->m_PaintedProjectedWalls).m_Memory.m_pMemory;
    if (ppCVar5 != (CProjectedWallEntity **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5103fe) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x5103fe),ppCVar5);
      (this->m_PaintedProjectedWalls).m_Memory.m_pMemory = (CProjectedWallEntity **)0x0;
    }
    ppCVar5 = (CProjectedWallEntity **)0x0;
    (this->m_PaintedProjectedWalls).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintedProjectedWalls).m_pElements = ppCVar5;
  if (-1 < (this->m_PaintedProjectedWalls).m_Memory.m_nGrowSize) {
    if (ppCVar5 != (CProjectedWallEntity **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5103fe) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x5103fe),ppCVar5);
      (this->m_PaintedProjectedWalls).m_Memory.m_pMemory = (CProjectedWallEntity **)0x0;
    }
    (this->m_PaintedProjectedWalls).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintedEntities).m_Size = 0;
  if ((this->m_PaintedEntities).m_Memory.m_nGrowSize < 0) {
    pPVar6 = (this->m_PaintedEntities).m_Memory.m_pMemory;
  }
  else {
    pPVar6 = (this->m_PaintedEntities).m_Memory.m_pMemory;
    if (pPVar6 != (PaintEntityData_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5103fe) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x5103fe),pPVar6);
      (this->m_PaintedEntities).m_Memory.m_pMemory = (PaintEntityData_t *)0x0;
    }
    pPVar6 = (PaintEntityData_t *)0x0;
    (this->m_PaintedEntities).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintedEntities).m_pElements = pPVar6;
  if (-1 < (this->m_PaintedEntities).m_Memory.m_nGrowSize) {
    if (pPVar6 != (PaintEntityData_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5103fe) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x5103fe),pPVar6);
      (this->m_PaintedEntities).m_Memory.m_pMemory = (PaintEntityData_t *)0x0;
    }
    (this->m_PaintedEntities).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintThisFrame).m_Size = 0;
  if ((this->m_PaintThisFrame).m_Memory.m_nGrowSize < 0) {
    pPVar7 = (this->m_PaintThisFrame).m_Memory.m_pMemory;
  }
  else {
    pPVar7 = (this->m_PaintThisFrame).m_Memory.m_pMemory;
    if (pPVar7 != (PaintLocationData_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5103fe) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x5103fe),pPVar7);
      (this->m_PaintThisFrame).m_Memory.m_pMemory = (PaintLocationData_t *)0x0;
    }
    pPVar7 = (PaintLocationData_t *)0x0;
    (this->m_PaintThisFrame).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintThisFrame).m_pElements = pPVar7;
  if (-1 < (this->m_PaintThisFrame).m_Memory.m_nGrowSize) {
    if (pPVar7 != (PaintLocationData_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5103fe) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x5103fe),pPVar7);
      (this->m_PaintThisFrame).m_Memory.m_pMemory = (PaintLocationData_t *)0x0;
    }
    (this->m_PaintThisFrame).m_Memory.m_nAllocationCount = 0;
  }
  (this->super_CGameEventListener).super_IGameEventListener2._vptr_IGameEventListener2 =
       (_func_int_varargs **)(&UNK_0051212a + unaff_EBX);
  (this->super_CGameEventListener).m_nDebugID = 0xd;
  if ((this->super_CGameEventListener).m_bRegisteredForEvents != false) {
    piVar8 = (int *)**(int **)(unaff_EBX + 0x51042a);
    if (piVar8 != (int *)0x0) {
      (**(code **)(*piVar8 + 0x18))(piVar8,&this->super_CGameEventListener);
    }
    (this->super_CGameEventListener).m_bRegisteredForEvents = false;
  }
  (this->super_CAutoGameSystemPerFrame).super_CBaseGameSystemPerFrame.super_IGameSystemPerFrame.
  super_IGameSystem._vptr_IGameSystem = (_func_int_varargs **)(&UNK_00511c0a + unaff_EBX);
  IGameSystemPerFrame::~IGameSystemPerFrame((IGameSystemPerFrame *)this,__in_chrg);
  return;
}


/* CPaintDatabase::~CPaintDatabase at 00736de0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void ~CPaintDatabase(CPaintDatabase * this, int __in_chrg,
   CPaintDatabase * this) */

void __thiscall
CPaintDatabase::~CPaintDatabase(CPaintDatabase *this,int __in_chrg,CPaintDatabase *this_1)

{
  ~CPaintDatabase(this,__in_chrg);
  return;
}


/* CPaintDatabase::LevelInitPreEntity at 00731d80 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void LevelInitPreEntity(CPaintDatabase * this) */

void __thiscall CPaintDatabase::LevelInitPreEntity(CPaintDatabase *this)

{
  (this->m_PaintThisFrame).m_Size = 0;
  (this->m_PaintedEntities).m_Size = 0;
  (this->m_PaintedProjectedWalls).m_Size = 0;
  return;
}


/* CPaintDatabase::LevelInitPostEntity at 00732460 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void LevelInitPostEntity(CPaintDatabase * this) */

void __thiscall CPaintDatabase::LevelInitPostEntity(CPaintDatabase *this)

{
  undefined4 *puVar1;
  CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *pCVar2;
  int iVar3;
  int unaff_EBX;
  int iVar4;
  int *piVar5;
  int iVar6;
  int local_24;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  this->m_bCanPaint = true;
                    /* Unresolved local var: int i@[???] */
  iVar6 = (this->m_Paintmaps).m_Size;
  if (0 < iVar6) {
    iVar4 = 0;
    local_20 = 0;
    do {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x514712) + 0x224))
                ((int *)**(undefined4 **)(unaff_EBX + 0x514712),
                 (int)&(((this->m_Paintmaps).m_Memory.m_pMemory)->m_Memory).m_pMemory + local_20,
                 iVar4);
      iVar4 = iVar4 + 1;
      iVar6 = (this->m_Paintmaps).m_Size;
      local_20 = local_20 + 0x14;
    } while (iVar4 < iVar6);
  }
  puVar1 = *(undefined4 **)(unaff_EBX + 0x5146ee);
  iVar4 = iVar6 * 0x14;
  while( true ) {
    local_24 = iVar4 + -0x14;
    iVar6 = iVar6 + -1;
    if (iVar6 < 0) break;
    piVar5 = (int *)((int)&(this->m_Paintmaps).m_Memory.m_pMemory[-1].m_Memory.m_pMemory + iVar4);
    piVar5[3] = 0;
    if (piVar5[2] < 0) {
      iVar3 = *piVar5;
    }
    else {
      if (*piVar5 != 0) {
        (**(code **)(*(int *)*puVar1 + 8))((int *)*puVar1,*piVar5);
        *piVar5 = 0;
      }
      iVar3 = 0;
      piVar5[1] = 0;
    }
    piVar5[4] = iVar3;
    iVar4 = local_24;
    if (-1 < piVar5[2]) {
      if (iVar3 != 0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5146ee) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x5146ee),iVar3);
        *piVar5 = 0;
      }
      piVar5[1] = 0;
    }
  }
  (this->m_Paintmaps).m_Size = 0;
  if ((this->m_Paintmaps).m_Memory.m_nGrowSize < 0) {
    (this->m_Paintmaps).m_pElements = (this->m_Paintmaps).m_Memory.m_pMemory;
    return;
  }
  pCVar2 = (this->m_Paintmaps).m_Memory.m_pMemory;
  if (pCVar2 != (CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *)0x0) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5146ee) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x5146ee),pCVar2);
    (this->m_Paintmaps).m_Memory.m_pMemory =
         (CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *)0x0;
  }
  (this->m_Paintmaps).m_Memory.m_nAllocationCount = 0;
  (this->m_Paintmaps).m_pElements = (CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *)0x0
  ;
  return;
}


/* CPaintDatabase::LevelShutdownPostEntity at 00731ae0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void LevelShutdownPostEntity(CPaintDatabase * this) */

void __thiscall CPaintDatabase::LevelShutdownPostEntity(CPaintDatabase *this)

{
  this->m_bCanPaint = false;
  return;
}


/* CPaintDatabase::AddPaint at 00733290 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void AddPaint(CPaintDatabase * this, trace_t * tr, PaintPowerType
   powerType) */

void __thiscall CPaintDatabase::AddPaint(CPaintDatabase *this,trace_t *tr,PaintPowerType powerType)

{
  int *piVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  CBaseEntity *this_00;
  uint uVar5;
  bool bVar6;
  CProjectedWallEntity *pWall;
  int iVar7;
  PaintLocationData_t *pPVar8;
  undefined4 *puVar9;
  int unaff_EBX;
  int iVar10;
  int iVar11;
  float fVar12;
  float fVar13;
  float fVar14;
  int local_50;
  Vector local_28 [2];
  
                    /* Unresolved local var: CBaseEntity * pPaintedEntity@[???]
                       Unresolved local var: Vector vecPaintLocation@[???]
                       Unresolved local var: Vector vecNormal@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = tr->m_pEnt;
  local_28[0].x = (tr->super_CBaseTrace).endpos.x;
  local_28[0].y = (tr->super_CBaseTrace).endpos.y;
  local_28[0].z = (tr->super_CBaseTrace).endpos.z;
  fVar2 = (tr->super_CBaseTrace).plane.normal.z;
  fVar3 = (tr->super_CBaseTrace).plane.normal.y;
  fVar4 = (tr->super_CBaseTrace).plane.normal.x;
  bVar6 = CBaseEntity::IsBSPModel(this_00);
  if ((!bVar6) || (bVar6 = UTIL_IsPaintableSurface(&tr->surface), !bVar6)) {
    if ((this_00->m_fFlags).m_Value < 0) {
      return;
    }
    if ((this_00 != (CBaseEntity *)0x0) &&
       (pWall = (CProjectedWallEntity *)
                ___dynamic_cast(this_00,*(undefined4 *)(unaff_EBX + 0x5138f2),
                                *(undefined4 *)(unaff_EBX + 0x513c4a),0),
       pWall != (CProjectedWallEntity *)0x0)) {
      PaintProjectedWall((CPaintDatabase *)(unaff_EBX + 0x695d82),pWall,powerType,local_28);
      return;
    }
    PaintEntity((CPaintDatabase *)(unaff_EBX + 0x695d82),this_00,powerType,local_28);
    return;
  }
                    /* Unresolved local var: Vector res@[???] */
  fVar14 = *(float *)(CAI_NetworkManager::GetBaseMap + unaff_EBX + 6);
                    /* Unresolved local var: Vector res@[???] */
  fVar12 = local_28[0].x + fVar4 * fVar14;
  fVar13 = local_28[0].y + fVar3 * fVar14;
  fVar14 = local_28[0].z + fVar14 * fVar2;
  uVar5 = (this->m_PaintThisFrame).m_Size;
  iVar11 = uVar5 + 1;
  iVar7 = (this->m_PaintThisFrame).m_Memory.m_nAllocationCount;
  if (iVar11 <= iVar7) goto LAB_007334c0;
  iVar10 = (this->m_PaintThisFrame).m_Memory.m_nGrowSize;
  if (iVar10 < 0) goto LAB_007334c0;
  if (iVar10 == 0) {
    if ((iVar7 == 0) && (iVar7 = 1, iVar11 < 2)) {
      local_50 = 0x20;
    }
    else {
      do {
        local_50 = iVar7;
        iVar7 = local_50 * 2;
      } while (iVar7 < iVar11);
      local_50 = local_50 << 6;
    }
    goto LAB_0073347c;
  }
  iVar7 = ((int)uVar5 / iVar10 + 1) * iVar10;
  if (iVar7 < iVar11) {
    if (iVar7 != 0) {
LAB_00733464:
      do {
        iVar7 = (iVar7 + iVar11) / 2;
      } while (iVar7 < iVar11);
      goto LAB_00733474;
    }
    if (iVar11 < 0) {
      iVar7 = -1;
      local_50 = -0x20;
    }
    else {
      local_50 = 0;
      if (iVar11 != 0) goto LAB_00733464;
    }
  }
  else {
LAB_00733474:
    local_50 = iVar7 << 5;
  }
LAB_0073347c:
  (this->m_PaintThisFrame).m_Memory.m_nAllocationCount = iVar7;
  pPVar8 = (this->m_PaintThisFrame).m_Memory.m_pMemory;
  if (pPVar8 == (PaintLocationData_t *)0x0) {
    pPVar8 = (PaintLocationData_t *)
             (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x5138be))
                       ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x5138be),local_50);
    (this->m_PaintThisFrame).m_Memory.m_pMemory = pPVar8;
  }
  else {
    pPVar8 = (PaintLocationData_t *)
             (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5138be) + 4))
                       ((int *)**(undefined4 **)(unaff_EBX + 0x5138be),pPVar8,local_50);
    (this->m_PaintThisFrame).m_Memory.m_pMemory = pPVar8;
  }
LAB_007334c0:
  piVar1 = &(this->m_PaintThisFrame).m_Size;
  *piVar1 = *piVar1 + 1;
  pPVar8 = (this->m_PaintThisFrame).m_Memory.m_pMemory;
  (this->m_PaintThisFrame).m_pElements = pPVar8;
  iVar7 = ~uVar5 + (this->m_PaintThisFrame).m_Size;
  if (iVar7 < 1) {
    iVar10 = uVar5 << 5;
  }
  else {
    iVar10 = uVar5 * 0x20;
    _V_memmove(pPVar8 + iVar11,pPVar8 + uVar5,iVar7 * 0x20);
  }
  puVar9 = (undefined4 *)
           ((int)&((this->m_PaintThisFrame).m_Memory.m_pMemory)->pBrushEntity + iVar10);
  if (puVar9 == (undefined4 *)0x0) {
    return;
  }
  puVar9[7] = fVar2;
  puVar9[6] = fVar3;
  puVar9[5] = fVar4;
  puVar9[1] = powerType;
  puVar9[4] = fVar14;
  puVar9[3] = fVar13;
  puVar9[2] = fVar12;
  *puVar9 = this_00;
  return;
}


/* CPaintDatabase::PaintEntity at 00732900 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void PaintEntity(CPaintDatabase * this, CBaseEntity * pPaintedEntity,
   PaintPowerType newPowerType, Vector * vecPosition) */

void __thiscall
CPaintDatabase::PaintEntity
          (CPaintDatabase *this,CBaseEntity *pPaintedEntity,PaintPowerType newPowerType,
          Vector *vecPosition)

{
  undefined1 *puVar1;
  uint uVar2;
  ushort uVar3;
  IPhysicsObject *pIVar4;
  CBaseEdict *pCVar5;
  int *piVar6;
  ushort *puVar7;
  uint8 uVar8;
  undefined1 uVar9;
  undefined1 uVar10;
  undefined1 uVar11;
  char cVar12;
  uint8 uVar13;
  PaintPowerType PVar14;
  int iVar15;
  int iVar16;
  ulong *puVar17;
  PaintEntityData_t *pPVar18;
  Color *pCVar19;
  IChangeInfoAccessor *pIVar20;
  uint uVar21;
  CBaseEntity *pCVar22;
  uint uVar23;
  int unaff_EBX;
  Vector *pVVar24;
  Vector *pVVar25;
  int iVar26;
  Vector *index;
  Vector *in_stack_ffffff18;
  int *local_a4;
  ulong local_74;
  Vector *local_40;
  CRecipientFilter local_3c;
  
                    /* Unresolved local var: IPaintableEntity * pPaintableEntity@[???]
                       Unresolved local var: PaintPowerType paintedPowerType@[???]
                       Unresolved local var: bool bPainted@[???] */
  ___i686_get_pc_thunk_bx();
  if (pPaintedEntity == (CBaseEntity *)0x0) {
    local_a4 = (int *)0x0;
    local_40 = in_stack_ffffff18;
LAB_00732bb0:
                    /* Unresolved local var: color24 otherColor@[???]
                       Unresolved local var: Color paintedColor@[???]
                       Unresolved local var: color24 c@[???] */
    uVar9 = (pPaintedEntity->m_clrRender).
            super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r;
    uVar10 = (pPaintedEntity->m_clrRender).
             super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g;
    uVar11 = (pPaintedEntity->m_clrRender).
             super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b;
    local_3c.super_IRecipientFilter._vptr_IRecipientFilter._2_1_ = uVar11;
    local_3c.super_IRecipientFilter._vptr_IRecipientFilter._1_1_ = uVar10;
    local_3c.super_IRecipientFilter._vptr_IRecipientFilter._0_1_ = uVar9;
    local_3c.super_IRecipientFilter._vptr_IRecipientFilter._3_1_ = 0;
    PVar14 = MapColorToPower((Color *)&local_3c);
  }
  else {
    local_40 = *(Vector **)(&DAT_0051427f + unaff_EBX);
    local_a4 = (int *)___dynamic_cast(pPaintedEntity,local_40,unaff_EBX + 0x589f8f,0xfffffffe);
    if (local_a4 == (int *)0x0) goto LAB_00732bb0;
    local_40 = vecPosition;
    PVar14 = (**(code **)(*local_a4 + 8))(local_a4);
    iVar15 = (*(pPaintedEntity->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
               _vptr_IHandleEntity[0x55])(pPaintedEntity);
    if ((char)iVar15 != '\0') {
      CRecipientFilter::CRecipientFilter(&local_3c);
      local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
           (_func_int_varargs **)(unaff_EBX + 0x5168d7);
      CRecipientFilter::AddAllPlayers(&local_3c);
      CRecipientFilter::MakeReliable(&local_3c);
      local_40 = (Vector *)(unaff_EBX + 0x37fa85);
      UserMessageBegin(&local_3c.super_IRecipientFilter,(char *)local_40);
      MessageWriteEHandle(pPaintedEntity);
      MessageWriteByte(newPowerType);
      MessageWriteFloat(vecPosition->x);
      MessageWriteFloat(vecPosition->y);
      MessageWriteFloat(vecPosition->z);
      MessageEnd();
      local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
           (_func_int_varargs **)(unaff_EBX + 0x5168d7);
      CRecipientFilter::~CRecipientFilter(&local_3c,(int)local_40);
    }
  }
  if (PVar14 != NO_POWER) {
    if ((newPowerType == NO_POWER) &&
       (pVVar24 = (Vector *)(this->m_PaintedEntities).m_Size, 0 < (int)pVVar24)) {
      index = (Vector *)0x0;
      do {
        uVar21 = (this->m_PaintedEntities).m_Memory.m_pMemory[(int)index].hEnt.m_Index;
        if ((uVar21 == 0xffffffff) ||
           (iVar15 = (uVar21 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51425b),
           *(uint *)(iVar15 + 8) != uVar21 >> 0x10)) {
          pVVar25 = (Vector *)0x0;
        }
        else {
          pVVar25 = *(Vector **)(iVar15 + 4);
        }
        local_40 = pVVar25;
        cVar12 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5142c3) + 8))
                           ((int *)**(undefined4 **)(unaff_EBX + 0x5142c3));
        pCVar22 = (CBaseEntity *)0x0;
        if (cVar12 == '\0') {
          pCVar22 = (CBaseEntity *)(**(code **)((int)pVVar25->x + 0x18))(pVVar25);
        }
        if (pCVar22 == pPaintedEntity) {
          RemovePaintedEntity(this,(int)index,true);
          local_40 = index;
          break;
        }
        index = (Vector *)((int)&index->x + 1);
      } while (index != pVVar24);
    }
    goto LAB_00732990;
  }
  if (newPowerType == NO_POWER) goto LAB_00732990;
  iVar15 = (this->m_PaintedEntities).m_Size;
  if (0 < iVar15) {
    iVar26 = 0;
    do {
      uVar21 = (this->m_PaintedEntities).m_Memory.m_pMemory[iVar26].hEnt.m_Index;
      if ((uVar21 == 0xffffffff) ||
         (iVar16 = (uVar21 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51425b),
         *(uint *)(iVar16 + 8) != uVar21 >> 0x10)) {
        pVVar24 = (Vector *)0x0;
      }
      else {
        pVVar24 = *(Vector **)(iVar16 + 4);
      }
      local_40 = pVVar24;
      cVar12 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5142c3) + 8))
                         ((int *)**(undefined4 **)(unaff_EBX + 0x5142c3));
      pCVar22 = (CBaseEntity *)0x0;
      if (cVar12 == '\0') {
        pCVar22 = (CBaseEntity *)(**(code **)((int)pVVar24->x + 0x18))(pVVar24);
      }
      if (pCVar22 == pPaintedEntity) goto LAB_00732990;
      iVar26 = iVar26 + 1;
    } while (iVar26 != iVar15);
  }
  local_74 = 0xffffffff;
  if (pPaintedEntity != (CBaseEntity *)0x0) {
    puVar17 = (ulong *)(*(pPaintedEntity->super_IServerEntity).super_IServerUnknown.
                         super_IHandleEntity._vptr_IHandleEntity[3])(pPaintedEntity);
    local_74 = *puVar17;
  }
  uVar21 = (this->m_PaintedEntities).m_Size;
  iVar15 = uVar21 + 1;
  iVar26 = (this->m_PaintedEntities).m_Memory.m_nAllocationCount;
  if (iVar26 < iVar15) {
    iVar16 = (this->m_PaintedEntities).m_Memory.m_nGrowSize;
    if (-1 < iVar16) {
      if (iVar16 == 0) {
        if ((iVar26 == 0) && (iVar26 = 8, iVar15 < 9)) {
          local_40 = (Vector *)&segment_command_0000001c.cmdsize;
        }
        else {
          do {
            iVar16 = iVar26;
            iVar26 = iVar16 * 2;
          } while (iVar26 < iVar15);
          local_40 = (Vector *)(iVar16 * 8);
        }
        goto LAB_00732b31;
      }
      iVar26 = ((int)uVar21 / iVar16 + 1) * iVar16;
      if (iVar26 < iVar15) {
        if (iVar26 != 0) {
LAB_00732ac9:
          do {
            iVar26 = (iVar26 + iVar15) / 2;
          } while (iVar26 < iVar15);
          goto LAB_00732ad9;
        }
        if (iVar15 < 0) {
          iVar26 = -1;
          local_40 = (Vector *)0xfffffffc;
        }
        else {
          local_40 = (Vector *)0x0;
          if (iVar15 != 0) goto LAB_00732ac9;
        }
      }
      else {
LAB_00732ad9:
        local_40 = (Vector *)(iVar26 * 4);
      }
LAB_00732b31:
      (this->m_PaintedEntities).m_Memory.m_nAllocationCount = iVar26;
      pVVar24 = (Vector *)(this->m_PaintedEntities).m_Memory.m_pMemory;
      if (pVVar24 == (Vector *)0x0) {
        pPVar18 = (PaintEntityData_t *)
                  (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x51424b))
                            ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x51424b));
        (this->m_PaintedEntities).m_Memory.m_pMemory = pPVar18;
      }
      else {
        pPVar18 = (PaintEntityData_t *)
                  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51424b) + 4))
                            ((int *)**(undefined4 **)(unaff_EBX + 0x51424b),pVVar24,local_40);
        (this->m_PaintedEntities).m_Memory.m_pMemory = pPVar18;
        local_40 = pVVar24;
      }
    }
  }
  piVar6 = &(this->m_PaintedEntities).m_Size;
  *piVar6 = *piVar6 + 1;
  pPVar18 = (this->m_PaintedEntities).m_Memory.m_pMemory;
  (this->m_PaintedEntities).m_pElements = pPVar18;
  iVar26 = ~uVar21 + (this->m_PaintedEntities).m_Size;
  if (0 < iVar26) {
    local_40 = (Vector *)(pPVar18 + uVar21);
    _V_memmove(pPVar18 + iVar15,local_40,iVar26 * 4);
  }
  pPVar18 = (this->m_PaintedEntities).m_Memory.m_pMemory + uVar21;
  if (pPVar18 != (PaintEntityData_t *)0x0) {
    (pPVar18->hEnt).m_Index = local_74;
  }
LAB_00732990:
  if (local_a4 == (int *)0x0) {
    pCVar19 = MapPowerToVisualColor((Color *)newPowerType,(int)local_40);
    uVar8 = (uint8)((uint)pCVar19 >> 8);
    uVar13 = (uint8)((uint)pCVar19 >> 0x10);
    if ((uint8)pCVar19 !=
        (pPaintedEntity->m_clrRender).
        super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r) {
      if ((pPaintedEntity->m_Network).m_TimerEvent.m_bRegistered == false) {
        pCVar5 = &((pPaintedEntity->m_Network).m_pPev)->super_CBaseEdict;
        if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
          pIVar20 = CBaseEdict::GetChangeAccessor(pCVar5);
          piVar6 = *(int **)(unaff_EBX + 0x514257);
          puVar7 = (ushort *)*piVar6;
          if (pIVar20->m_iChangeInfoSerialNumber == *puVar7) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            uVar21 = (uint)pIVar20->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar3 = puVar7[uVar21 * 0x14 + 0x14];
            if (uVar3 == 0) {
LAB_00733165:
              puVar7[(uint)uVar3 + uVar21 * 0x14 + 1] = 0x108;
              puVar7[uVar21 * 0x14 + 0x14] = uVar3 + 1;
            }
            else if (puVar7[uVar21 * 0x14 + 1] != 0x108) {
              uVar23 = 0;
              do {
                uVar2 = uVar23 + 1;
                uVar23 = uVar2 & 0xffff;
                if ((ushort)uVar2 == uVar3) {
                  if (uVar3 == 0x13) goto LAB_007330cc;
                  goto LAB_00733165;
                }
              } while (puVar7[uVar21 * 0x14 + uVar23 + 1] != 0x108);
            }
          }
          else if (puVar7[0x7d1] == 100) {
LAB_007330cc:
            pIVar20->m_iChangeInfoSerialNumber = 0;
            pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar20->m_iChangeInfo = puVar7[0x7d1];
            *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
            pIVar20->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
            iVar15 = *piVar6 + (uint)pIVar20->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar15 + 2) = 0x108;
            *(undefined2 *)(iVar15 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(pPaintedEntity->m_Network).field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (pPaintedEntity->m_clrRender).
      super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r =
           (uint8)pCVar19;
    }
    if (uVar8 != (pPaintedEntity->m_clrRender).
                 super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g) {
      if ((pPaintedEntity->m_Network).m_TimerEvent.m_bRegistered == false) {
        pCVar5 = &((pPaintedEntity->m_Network).m_pPev)->super_CBaseEdict;
        if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
          pIVar20 = CBaseEdict::GetChangeAccessor(pCVar5);
          piVar6 = *(int **)(unaff_EBX + 0x514257);
          puVar7 = (ushort *)*piVar6;
          if (pIVar20->m_iChangeInfoSerialNumber == *puVar7) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            uVar21 = (uint)pIVar20->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar3 = puVar7[uVar21 * 0x14 + 0x14];
            if (uVar3 == 0) {
LAB_00733225:
              puVar7[(uint)uVar3 + uVar21 * 0x14 + 1] = 0x108;
              puVar7[uVar21 * 0x14 + 0x14] = uVar3 + 1;
            }
            else if (puVar7[uVar21 * 0x14 + 1] != 0x108) {
              uVar23 = 0;
              do {
                uVar2 = uVar23 + 1;
                uVar23 = uVar2 & 0xffff;
                if ((ushort)uVar2 == uVar3) {
                  if (uVar3 == 0x13) goto LAB_0073318f;
                  goto LAB_00733225;
                }
              } while (puVar7[uVar21 * 0x14 + uVar23 + 1] != 0x108);
            }
          }
          else if (puVar7[0x7d1] == 100) {
LAB_0073318f:
            pIVar20->m_iChangeInfoSerialNumber = 0;
            pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar20->m_iChangeInfo = puVar7[0x7d1];
            *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
            pIVar20->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
            iVar15 = *piVar6 + (uint)pIVar20->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar15 + 2) = 0x108;
            *(undefined2 *)(iVar15 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(pPaintedEntity->m_Network).field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (pPaintedEntity->m_clrRender).
      super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g = uVar8;
    }
    if (uVar13 != (pPaintedEntity->m_clrRender).
                  super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b) {
      if ((pPaintedEntity->m_Network).m_TimerEvent.m_bRegistered == false) {
        pCVar5 = &((pPaintedEntity->m_Network).m_pPev)->super_CBaseEdict;
        if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
          pIVar20 = CBaseEdict::GetChangeAccessor(pCVar5);
          piVar6 = *(int **)(unaff_EBX + 0x514257);
          puVar7 = (ushort *)*piVar6;
          if (pIVar20->m_iChangeInfoSerialNumber == *puVar7) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            uVar21 = (uint)pIVar20->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar3 = puVar7[uVar21 * 0x14 + 0x14];
            if (uVar3 == 0) {
LAB_00733094:
              puVar7[(uint)uVar3 + uVar21 * 0x14 + 1] = 0x108;
              puVar7[uVar21 * 0x14 + 0x14] = uVar3 + 1;
            }
            else if (puVar7[uVar21 * 0x14 + 1] != 0x108) {
              uVar23 = 0;
              do {
                uVar2 = uVar23 + 1;
                uVar23 = uVar2 & 0xffff;
                if ((ushort)uVar2 == uVar3) {
                  if (uVar3 == 0x13) goto LAB_007330b8;
                  goto LAB_00733094;
                }
              } while (puVar7[uVar21 * 0x14 + uVar23 + 1] != 0x108);
            }
          }
          else if (puVar7[0x7d1] == 100) {
LAB_007330b8:
            pIVar20->m_iChangeInfoSerialNumber = 0;
            pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar20->m_iChangeInfo = puVar7[0x7d1];
            *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
            pIVar20->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
            iVar15 = *piVar6 + (uint)pIVar20->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar15 + 2) = 0x108;
            *(undefined2 *)(iVar15 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(pPaintedEntity->m_Network).field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (pPaintedEntity->m_clrRender).
      super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b = uVar13;
    }
  }
  else {
    pIVar4 = pPaintedEntity->m_pPhysicsObject;
    if ((pIVar4 != (IPhysicsObject *)0x0) &&
       (iVar15 = (*pIVar4->_vptr_IPhysicsObject[3])(pIVar4), (char)iVar15 != '\0')) {
      (*pIVar4->_vptr_IPhysicsObject[0x19])(pIVar4);
    }
    (**(code **)(*local_a4 + 0xc))(local_a4,newPowerType,vecPosition);
  }
  return;
}


/* CPaintDatabase::PaintProjectedWall at 00731bb0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void PaintProjectedWall(CPaintDatabase * this, CProjectedWallEntity *
   pWall, PaintPowerType powerType, Vector * vecPosition) */

void __thiscall
CPaintDatabase::PaintProjectedWall
          (CPaintDatabase *this,CProjectedWallEntity *pWall,PaintPowerType powerType,
          Vector *vecPosition)

{
  int iVar1;
  int *piVar2;
  CProjectedWallEntity *pCVar3;
  uint uVar4;
  bool bVar5;
  uint uVar6;
  int iVar7;
  CProjectedWallEntity **ppCVar8;
  int unaff_EBX;
  int iVar9;
  bool bWallPainted;
  
  ___i686_get_pc_thunk_bx();
  bVar5 = CProjectedWallEntity::IsWallPainted(pWall,vecPosition);
  (*(pWall->super_CPaintableEntity<CBaseProjectedEntity>).super_CBaseProjectedEntity.
    super_CBaseEntity.super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0xd5])(pWall,powerType,vecPosition);
  if (bVar5) {
    return;
  }
  if (powerType == NO_POWER) {
    return;
  }
                    /* Unresolved local var: int i@[???] */
  uVar4 = (this->m_PaintedProjectedWalls).m_Size;
  if ((int)uVar4 < 1) {
    ppCVar8 = (this->m_PaintedProjectedWalls).m_Memory.m_pMemory;
  }
  else {
    ppCVar8 = (this->m_PaintedProjectedWalls).m_Memory.m_pMemory;
    uVar6 = 0;
    pCVar3 = *ppCVar8;
    while( true ) {
      if (pWall == pCVar3) {
        return;
      }
      uVar6 = uVar6 + 1;
      if (uVar6 == uVar4) break;
      pCVar3 = ppCVar8[uVar6];
    }
  }
  iVar1 = uVar4 + 1;
  iVar7 = (this->m_PaintedProjectedWalls).m_Memory.m_nAllocationCount;
  if (iVar1 <= iVar7) goto LAB_00731c3e;
  iVar9 = (this->m_PaintedProjectedWalls).m_Memory.m_nGrowSize;
  if (iVar9 < 0) goto LAB_00731c3e;
  if (iVar9 == 0) {
    if (iVar7 == 0) {
      iVar7 = 8;
      iVar9 = 0x20;
      if (iVar1 < 9) goto LAB_00731cc8;
    }
    do {
      iVar7 = iVar7 * 2;
    } while (iVar7 < iVar1);
LAB_00731cc1:
    iVar9 = iVar7 * 4;
  }
  else {
    iVar7 = ((int)uVar4 / iVar9 + 1) * iVar9;
    if (iVar1 <= iVar7) goto LAB_00731cc1;
    if (iVar7 != 0) {
LAB_00731cae:
      do {
        iVar7 = (iVar7 + iVar1) / 2;
      } while (iVar7 < iVar1);
      goto LAB_00731cc1;
    }
    if (iVar1 < 0) {
      iVar7 = -1;
      iVar9 = -4;
    }
    else {
      iVar9 = 0;
      if (iVar1 != 0) goto LAB_00731cae;
    }
  }
LAB_00731cc8:
  (this->m_PaintedProjectedWalls).m_Memory.m_nAllocationCount = iVar7;
  if (ppCVar8 == (CProjectedWallEntity **)0x0) {
    ppCVar8 = (CProjectedWallEntity **)
              (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x514f9e))
                        ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x514f9e),iVar9);
    (this->m_PaintedProjectedWalls).m_Memory.m_pMemory = ppCVar8;
  }
  else {
    ppCVar8 = (CProjectedWallEntity **)
              (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x514f9e) + 4))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x514f9e),ppCVar8,iVar9);
    (this->m_PaintedProjectedWalls).m_Memory.m_pMemory = ppCVar8;
  }
LAB_00731c3e:
  piVar2 = &(this->m_PaintedProjectedWalls).m_Size;
  *piVar2 = *piVar2 + 1;
  (this->m_PaintedProjectedWalls).m_pElements = ppCVar8;
  iVar7 = ~uVar4 + (this->m_PaintedProjectedWalls).m_Size;
  if (0 < iVar7) {
    _V_memmove(ppCVar8 + iVar1,ppCVar8 + uVar4,iVar7 * 4);
  }
  ppCVar8 = (this->m_PaintedProjectedWalls).m_Memory.m_pMemory + uVar4;
  if (ppCVar8 != (CProjectedWallEntity **)0x0) {
    *ppCVar8 = pWall;
  }
  return;
}


/* CPaintDatabase::RemoveAllPaint at 00733850 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void RemoveAllPaint(CPaintDatabase * this) */

void __thiscall CPaintDatabase::RemoveAllPaint(CPaintDatabase *this)

{
  int iVar1;
  uint uVar2;
  char cVar3;
  int iVar4;
  int unaff_EBX;
  int *piVar5;
  int index;
  char *messagename;
  int nEntityCount;
  CRecipientFilter local_3c;
  
                    /* Unresolved local var: int nPaintedProjectedWallCount@[???]
                       Unresolved local var: CBroadcastRecipientFilter filter@[???] */
  ___i686_get_pc_thunk_bx();
  iVar1 = (this->m_PaintedEntities).m_Size;
  if (0 < iVar1) {
    index = 0;
    do {
      uVar2 = (this->m_PaintedEntities).m_Memory.m_pMemory[index].hEnt.m_Index;
      if ((uVar2 == 0xffffffff) ||
         (iVar4 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51330e),
         *(uint *)(iVar4 + 8) != uVar2 >> 0x10)) {
        piVar5 = (int *)0x0;
      }
      else {
        piVar5 = *(int **)(iVar4 + 4);
      }
      cVar3 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x513376) + 8))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x513376),piVar5);
      if (cVar3 == '\0') {
        iVar4 = (**(code **)(*piVar5 + 0x18))(piVar5);
        if (iVar4 != 0) {
          RemovePaintedEntity(this,index,false);
        }
      }
      index = index + 1;
    } while (index != iVar1);
  }
  (this->m_PaintThisFrame).m_Size = 0;
  (this->m_PaintedEntities).m_Size = 0;
  (this->m_PaintedProjectedWalls).m_Size = 0;
  (**(code **)(*(int *)**(undefined4 **)(CAI_FuncTankBehavior::EndScheduleSelection + unaff_EBX + 2)
              + 0x218))
            ((int *)**(undefined4 **)(CAI_FuncTankBehavior::EndScheduleSelection + unaff_EBX + 2));
  CRecipientFilter::CRecipientFilter(&local_3c);
  local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(unaff_EBX + 0x51598a);
  CRecipientFilter::AddAllPlayers(&local_3c);
  CRecipientFilter::MakeReliable(&local_3c);
  messagename = (char *)(unaff_EBX + 0x37eb82);
  UserMessageBegin(&local_3c.super_IRecipientFilter,messagename);
  MessageEnd();
  local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(unaff_EBX + 0x51598a);
  CRecipientFilter::~CRecipientFilter(&local_3c,(int)messagename);
  return;
}


/* CPaintDatabase::RemovePaintedEntity at 007326c0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void RemovePaintedEntity(CPaintDatabase * this, CBaseEntity *
   pPaintedEntity) */

void __thiscall
CPaintDatabase::RemovePaintedEntity(CPaintDatabase *this,CBaseEntity *pPaintedEntity)

{
  int iVar1;
  uint uVar2;
  char cVar3;
  int iVar4;
  CBaseEntity *pCVar5;
  int unaff_EBX;
  int *piVar6;
  int index;
  
                    /* Unresolved local var: int index@[???] */
  ___i686_get_pc_thunk_bx();
  iVar1 = (this->m_PaintedEntities).m_Size;
  if (0 < iVar1) {
    index = 0;
    do {
      uVar2 = (this->m_PaintedEntities).m_Memory.m_pMemory[index].hEnt.m_Index;
      if ((uVar2 == 0xffffffff) ||
         (iVar4 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51449e),
         *(uint *)(iVar4 + 8) != uVar2 >> 0x10)) {
        piVar6 = (int *)0x0;
      }
      else {
        piVar6 = *(int **)(iVar4 + 4);
      }
      cVar3 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x514506) + 8))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x514506),piVar6);
      if (cVar3 == '\0') {
        pCVar5 = (CBaseEntity *)(**(code **)(*piVar6 + 0x18))(piVar6);
        if (pCVar5 == pPaintedEntity) goto LAB_0073275f;
      }
      else if (pPaintedEntity == (CBaseEntity *)0x0) {
LAB_0073275f:
        RemovePaintedEntity(this,index,true);
        return;
      }
      index = index + 1;
    } while (index != iVar1);
  }
  return;
}


/* CPaintDatabase::RemovePaintedEntity at 007325f0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void RemovePaintedEntity(CPaintDatabase * this, CBaseEntity *
   pPaintedEntity, bool bDeleteData) */

void __thiscall
CPaintDatabase::RemovePaintedEntity
          (CPaintDatabase *this,CBaseEntity *pPaintedEntity,bool bDeleteData)

{
  int iVar1;
  uint uVar2;
  char cVar3;
  int iVar4;
  CBaseEntity *pCVar5;
  int unaff_EBX;
  int *piVar6;
  int index;
  
                    /* Unresolved local var: int index@[???] */
  ___i686_get_pc_thunk_bx();
  iVar1 = (this->m_PaintedEntities).m_Size;
  if (0 < iVar1) {
    index = 0;
    do {
      uVar2 = (this->m_PaintedEntities).m_Memory.m_pMemory[index].hEnt.m_Index;
      if ((uVar2 == 0xffffffff) ||
         (iVar4 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51456e),
         *(uint *)(iVar4 + 8) != uVar2 >> 0x10)) {
        piVar6 = (int *)0x0;
      }
      else {
        piVar6 = *(int **)(iVar4 + 4);
      }
      cVar3 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5145d6) + 8))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x5145d6),piVar6);
      if (cVar3 == '\0') {
        pCVar5 = (CBaseEntity *)(**(code **)(*piVar6 + 0x18))(piVar6);
        if (pCVar5 == pPaintedEntity) goto LAB_0073269f;
      }
      else if (pPaintedEntity == (CBaseEntity *)0x0) {
LAB_0073269f:
        RemovePaintedEntity(this,index,bDeleteData);
        return;
      }
      index = index + 1;
    } while (index != iVar1);
  }
  return;
}


/* CPaintDatabase::RemovePaintedEntity at 00731da0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void RemovePaintedEntity(CPaintDatabase * this, int index, bool
   bDeleteData) */

void __thiscall CPaintDatabase::RemovePaintedEntity(CPaintDatabase *this,int index,bool bDeleteData)

{
  int iVar1;
  ushort uVar2;
  PaintEntityData_t *pPVar3;
  CBaseEdict *pCVar4;
  ushort *puVar5;
  char cVar6;
  int iVar7;
  IChangeInfoAccessor *pIVar8;
  uint uVar9;
  uint uVar10;
  int unaff_EBX;
  int *piVar11;
  uint local_98;
  uint local_94;
  uint local_90;
  
                    /* Unresolved local var: CBaseEntity * pEnt@[???] */
  ___i686_get_pc_thunk_bx();
  uVar9 = (this->m_PaintedEntities).m_Memory.m_pMemory[index].hEnt.m_Index;
  if ((uVar9 == 0xffffffff) ||
     (iVar7 = (uVar9 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x514db5),
     *(uint *)(iVar7 + 8) != uVar9 >> 0x10)) {
    piVar11 = (int *)0x0;
  }
  else {
    piVar11 = *(int **)(iVar7 + 4);
  }
  cVar6 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x514e1d) + 8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x514e1d),piVar11);
  if (cVar6 == '\0') {
    iVar7 = (**(code **)(*piVar11 + 0x18))(piVar11);
    if (iVar7 != 0) {
                    /* Unresolved local var: IPaintableEntity * pPaintableEntity@[???] */
      piVar11 = (int *)___dynamic_cast(iVar7,*(undefined4 *)(unaff_EBX + 0x514dd9),
                                       unaff_EBX + 0x58aae9,0xfffffffe);
      if (piVar11 == (int *)0x0) {
        if (*(char *)(iVar7 + 0x108) != -1) {
          if (*(char *)(iVar7 + 0x60) == '\0') {
            pCVar4 = *(CBaseEdict **)(iVar7 + 0x24);
            if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
              pIVar8 = CBaseEdict::GetChangeAccessor(pCVar4);
              puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x514db1);
              if (pIVar8->m_iChangeInfoSerialNumber == *puVar5) {
                uVar9 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
                uVar2 = puVar5[uVar9 * 0x14 + 0x14];
                if (uVar2 == 0) {
LAB_00732301:
                  puVar5[(uint)uVar2 + uVar9 * 0x14 + 1] = 0x108;
                  puVar5[uVar9 * 0x14 + 0x14] = uVar2 + 1;
                }
                else if (puVar5[uVar9 * 0x14 + 1] != 0x108) {
                  local_98 = 0;
                  do {
                    uVar10 = local_98 + 1;
                    local_98 = uVar10 & 0xffff;
                    if ((ushort)uVar10 == uVar2) {
                      if (uVar2 != 0x13) goto LAB_00732301;
                      pIVar8->m_iChangeInfoSerialNumber = 0;
                      pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                      break;
                    }
                  } while (puVar5[uVar9 * 0x14 + local_98 + 1] != 0x108);
                }
              }
              else if (puVar5[0x7d1] == 100) {
                pIVar8->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
              }
              else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                pIVar8->m_iChangeInfo = puVar5[0x7d1];
                piVar11 = *(int **)(unaff_EBX + 0x514db1);
                *(short *)(*piVar11 + 0xfa2) = *(short *)(*piVar11 + 0xfa2) + 1;
                pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar11;
                iVar1 = *piVar11 + (uint)pIVar8->m_iChangeInfo * 0x28;
                *(undefined2 *)(iVar1 + 2) = 0x108;
                *(undefined2 *)(iVar1 + 0x28) = 1;
              }
            }
          }
          else {
            *(uint *)(iVar7 + 100) = *(uint *)(iVar7 + 100) | 1;
          }
          *(undefined1 *)(iVar7 + 0x108) = 0xff;
        }
        if (*(char *)(iVar7 + 0x109) != -1) {
          if (*(char *)(iVar7 + 0x60) == '\0') {
            pCVar4 = *(CBaseEdict **)(iVar7 + 0x24);
            if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
              pIVar8 = CBaseEdict::GetChangeAccessor(pCVar4);
              puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x514db1);
              if (pIVar8->m_iChangeInfoSerialNumber == *puVar5) {
                uVar9 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
                uVar2 = puVar5[uVar9 * 0x14 + 0x14];
                if (uVar2 == 0) {
LAB_007321a1:
                  puVar5[(uint)uVar2 + uVar9 * 0x14 + 1] = 0x108;
                  puVar5[uVar9 * 0x14 + 0x14] = uVar2 + 1;
                }
                else if (puVar5[uVar9 * 0x14 + 1] != 0x108) {
                  local_94 = 0;
                  do {
                    uVar10 = local_94 + 1;
                    local_94 = uVar10 & 0xffff;
                    if ((ushort)uVar10 == uVar2) {
                      if (uVar2 != 0x13) goto LAB_007321a1;
                      pIVar8->m_iChangeInfoSerialNumber = 0;
                      pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                      break;
                    }
                  } while (puVar5[uVar9 * 0x14 + local_94 + 1] != 0x108);
                }
              }
              else if (puVar5[0x7d1] == 100) {
                pIVar8->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
              }
              else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                pIVar8->m_iChangeInfo = puVar5[0x7d1];
                piVar11 = *(int **)(unaff_EBX + 0x514db1);
                *(short *)(*piVar11 + 0xfa2) = *(short *)(*piVar11 + 0xfa2) + 1;
                pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar11;
                iVar1 = *piVar11 + (uint)pIVar8->m_iChangeInfo * 0x28;
                *(undefined2 *)(iVar1 + 2) = 0x108;
                *(undefined2 *)(iVar1 + 0x28) = 1;
              }
            }
          }
          else {
            *(uint *)(iVar7 + 100) = *(uint *)(iVar7 + 100) | 1;
          }
          *(undefined1 *)(iVar7 + 0x109) = 0xff;
        }
        if (*(char *)(iVar7 + 0x10a) != -1) {
          if (*(char *)(iVar7 + 0x60) == '\0') {
            pCVar4 = *(CBaseEdict **)(iVar7 + 0x24);
            if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
              pIVar8 = CBaseEdict::GetChangeAccessor(pCVar4);
              puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x514db1);
              if (pIVar8->m_iChangeInfoSerialNumber == *puVar5) {
                uVar9 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
                uVar2 = puVar5[uVar9 * 0x14 + 0x14];
                if (uVar2 == 0) {
LAB_00732251:
                  puVar5[(uint)uVar2 + uVar9 * 0x14 + 1] = 0x108;
                  puVar5[uVar9 * 0x14 + 0x14] = uVar2 + 1;
                }
                else if (puVar5[uVar9 * 0x14 + 1] != 0x108) {
                  local_90 = 0;
                  do {
                    uVar10 = local_90 + 1;
                    local_90 = uVar10 & 0xffff;
                    if ((ushort)uVar10 == uVar2) {
                      if (uVar2 != 0x13) goto LAB_00732251;
                      pIVar8->m_iChangeInfoSerialNumber = 0;
                      pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                      break;
                    }
                  } while (puVar5[uVar9 * 0x14 + local_90 + 1] != 0x108);
                }
              }
              else if (puVar5[0x7d1] == 100) {
                pIVar8->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
              }
              else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                pIVar8->m_iChangeInfo = puVar5[0x7d1];
                piVar11 = *(int **)(unaff_EBX + 0x514db1);
                *(short *)(*piVar11 + 0xfa2) = *(short *)(*piVar11 + 0xfa2) + 1;
                pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar11;
                iVar1 = *piVar11 + (uint)pIVar8->m_iChangeInfo * 0x28;
                *(undefined2 *)(iVar1 + 2) = 0x108;
                *(undefined2 *)(iVar1 + 0x28) = 1;
              }
            }
          }
          else {
            *(uint *)(iVar7 + 100) = *(uint *)(iVar7 + 100) | 1;
          }
          *(undefined1 *)(iVar7 + 0x10a) = 0xff;
        }
      }
      else {
        (**(code **)(*piVar11 + 0x10))(piVar11);
      }
    }
  }
  if (bDeleteData) {
    iVar7 = ~index + (this->m_PaintedEntities).m_Size;
    if (0 < iVar7) {
      pPVar3 = (this->m_PaintedEntities).m_Memory.m_pMemory;
      _V_memmove(pPVar3 + index,pPVar3 + index + 1,iVar7 * 4);
    }
    piVar11 = &(this->m_PaintedEntities).m_Size;
    *piVar11 = *piVar11 + -1;
  }
  return;
}


/* CPaintDatabase::RemovePaintedWall at 00731b30 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void RemovePaintedWall(CPaintDatabase * this, CProjectedWallEntity *
   pWall, bool bDeleteData) */

void __thiscall
CPaintDatabase::RemovePaintedWall(CPaintDatabase *this,CProjectedWallEntity *pWall,bool bDeleteData)

{
  int *piVar1;
  CProjectedWallEntity **ppCVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  
  if ((bDeleteData) && (iVar4 = (this->m_PaintedProjectedWalls).m_Size, 0 < iVar4)) {
    ppCVar2 = (this->m_PaintedProjectedWalls).m_Memory.m_pMemory;
    if (*ppCVar2 == pWall) {
      iVar6 = 0;
      iVar5 = 0;
    }
    else {
      iVar6 = 0;
      iVar3 = 4;
      do {
        iVar5 = iVar3;
        iVar6 = iVar6 + 1;
        if (iVar6 == iVar4) {
          return;
        }
        iVar3 = iVar5 + 4;
      } while (ppCVar2[iVar6] != pWall);
    }
    iVar4 = (iVar4 - iVar6) + -1;
    if (0 < iVar4) {
      _V_memmove((void *)(iVar5 + (int)ppCVar2),ppCVar2 + iVar6 + 1,iVar4 * 4);
    }
    piVar1 = &(this->m_PaintedProjectedWalls).m_Size;
    *piVar1 = *piVar1 + -1;
    return;
  }
  return;
}


/* CPaintDatabase::FindPaintedEntity at 007323c0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: int FindPaintedEntity(CPaintDatabase * this, CBaseEntity *
   pPaintedEntity) */

int __thiscall CPaintDatabase::FindPaintedEntity(CPaintDatabase *this,CBaseEntity *pPaintedEntity)

{
  int iVar1;
  uint uVar2;
  char cVar3;
  int iVar4;
  CBaseEntity *pCVar5;
  int unaff_EBX;
  int *piVar6;
  int iVar7;
  int nPaintCount;
  
  ___i686_get_pc_thunk_bx();
  iVar1 = (this->m_PaintedEntities).m_Size;
  if (0 < iVar1) {
    iVar7 = 0;
    do {
      uVar2 = (this->m_PaintedEntities).m_Memory.m_pMemory[iVar7].hEnt.m_Index;
      if ((uVar2 == 0xffffffff) ||
         (iVar4 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51479e),
         *(uint *)(iVar4 + 8) != uVar2 >> 0x10)) {
        piVar6 = (int *)0x0;
      }
      else {
        piVar6 = *(int **)(iVar4 + 4);
      }
      cVar3 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x514806) + 8))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x514806),piVar6);
      pCVar5 = (CBaseEntity *)0x0;
      if (cVar3 == '\0') {
        pCVar5 = (CBaseEntity *)(**(code **)(*piVar6 + 0x18))(piVar6);
      }
      if (pCVar5 == pPaintedEntity) {
        return iVar7;
      }
      iVar7 = iVar7 + 1;
    } while (iVar7 != iVar1);
  }
  return -1;
}


/* CPaintDatabase::FireGameEvent at 00731b00 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void FireGameEvent(CPaintDatabase * this, IGameEvent * event) */

void __thiscall CPaintDatabase::FireGameEvent(CPaintDatabase *this,IGameEvent *event)

{
  return;
}


/* CPaintDatabase::ClearPaintData at 00731b10 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void ClearPaintData(CPaintDatabase * this) */

void __thiscall CPaintDatabase::ClearPaintData(CPaintDatabase *this)

{
  (this->m_PaintThisFrame).m_Size = 0;
  (this->m_PaintedEntities).m_Size = 0;
  (this->m_PaintedProjectedWalls).m_Size = 0;
  return;
}


/* AscendingPaintLocationCompare at 00736e00 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

int AscendingPaintLocationCompare(PaintLocationData_t *a,PaintLocationData_t *b)

{
  uint uVar1;
  
  uVar1 = 0;
  if (b->pBrushEntity <= a->pBrushEntity) {
    uVar1 = (uint)((int)b->type <= (int)a->type);
  }
  return uVar1;
}


/* __tcf_1 at 00a3dc70 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(&UNK_0038b428 + extraout_ECX),in_stack_00000008);
  return;
}


/* CPaintDatabase::PreClientUpdate at 00734840 */

/* WARNING: Restarted to delay deadcode elimination for space: stack */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void PreClientUpdate(CPaintDatabase * this) */

void __thiscall CPaintDatabase::PreClientUpdate(CPaintDatabase *this)

{
  uint *puVar1;
  float fVar2;
  float fVar3;
  vec_t vVar4;
  vec_t vVar5;
  vec_t vVar6;
  vec_t vVar7;
  undefined4 *puVar8;
  CBaseEntity *pCVar9;
  size_t sVar10;
  PaintPowerType PVar11;
  PaintPowerType PVar12;
  CBaseEntity *pCVar13;
  vec_t vVar14;
  vec_t vVar15;
  float fVar16;
  uint uVar17;
  char cVar18;
  int *piVar19;
  CBasePlayer *pPlayer;
  PaintLocationData_t *pPVar20;
  Vector *pVVar21;
  uint uVar22;
  undefined4 *puVar23;
  undefined4 *puVar24;
  int iVar25;
  int iVar26;
  float *pfVar27;
  int unaff_EBX;
  PaintLocationData_t *pPVar28;
  undefined4 *puVar29;
  PaintLocationData_t *pPVar30;
  CRecipientFilter *pCVar31;
  float fVar32;
  float fVar33;
  float fVar34;
  float fVar35;
  float fVar36;
  float fVar37;
  float fVar38;
  float fVar39;
  int local_338;
  int local_32c;
  int local_328;
  int local_318;
  int local_310;
  uint local_30c;
  int local_2fc;
  int local_2f8;
  int local_2e4;
  PaintLocationData_t *local_2dc;
  int local_2c8;
  undefined4 *local_2ac;
  int local_2a8;
  int local_2a4;
  undefined4 local_29c [96];
  uint local_11c;
  undefined4 *local_118;
  undefined *local_114;
  undefined1 *local_110;
  undefined4 local_10c;
  int local_108;
  undefined4 local_104;
  undefined1 local_100 [128];
  int local_80;
  undefined1 *local_7c;
  CRecipientFilter local_78;
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
  float local_24;
  float local_20;
  
                    /* Unresolved local var: CVProfScope VProf_@[???]
                       Unresolved local var: int count@[???]
                       Unresolved local var: CChangedPaintBoundsCache boundsCache@[???]
                       Unresolved local var: CPaintAffectedEntityList paintEnum@[???] */
  ___i686_get_pc_thunk_bx();
  if (((*(int *)(&DAT_0000100c + *(int *)(&DAT_00512333 + unaff_EBX)) == 0) &&
      ((&DAT_00001010)[*(int *)(&DAT_00512333 + unaff_EBX)] != '\0')) ||
     (iVar25 = *(int *)(*(int *)(&DAT_00512333 + unaff_EBX) + 0x19b8),
     iVar26 = _ThreadGetCurrentId(), iVar25 != iVar26)) {
    cVar18 = this->m_bCanPaint;
  }
  else {
    piVar19 = *(int **)(&DAT_00001014 + *(int *)(&DAT_00512333 + unaff_EBX));
    if (*piVar19 != unaff_EBX + 0x37dc2f) {
      piVar19 = (int *)CVProfNode::GetSubNode
                                 ((char *)piVar19,unaff_EBX + 0x37dc2f,(char *)0x0,
                                  unaff_EBX + 0x37dc4f);
      *(int **)(&DAT_00001014 + *(int *)(&DAT_00512333 + unaff_EBX)) = piVar19;
    }
    iVar25 = *(int *)(&DAT_00512333 + unaff_EBX);
    puVar1 = (uint *)(piVar19[0x1c] * 8 + *(int *)(&DAT_000010a0 + iVar25) + 4);
    *puVar1 = *puVar1 | 4;
    CVProfNode::EnterScope();
    (&DAT_00001010)[iVar25] = 0;
    cVar18 = this->m_bCanPaint;
  }
  if (cVar18 == '\0') goto LAB_007349cd;
  local_338 = (this->m_PaintThisFrame).m_Size;
  if (1 < local_338) {
    local_310 = 0;
    do {
      pPVar28 = (this->m_PaintThisFrame).m_Memory.m_pMemory + local_310;
      pCVar31 = &local_78;
      for (iVar25 = 8; iVar25 != 0; iVar25 = iVar25 + -1) {
        (pCVar31->super_IRecipientFilter)._vptr_IRecipientFilter =
             (_func_int_varargs **)pPVar28->pBrushEntity;
        pPVar28 = (PaintLocationData_t *)&pPVar28->type;
        pCVar31 = (CRecipientFilter *)&pCVar31->m_bReliable;
      }
      local_310 = local_310 + 1;
      iVar25 = local_310;
      while (iVar25 < local_338) {
        pPVar30 = (this->m_PaintThisFrame).m_Memory.m_pMemory;
        pPVar28 = pPVar30 + iVar25;
                    /* Unresolved local var: Vector delta@[???] */
        if ((pPVar28->type != local_78._4_4_) ||
           (fVar32 = (float)local_78.m_Recipients.m_Memory.m_pMemory - (pPVar28->location).x,
           fVar33 = (float)local_78.m_Recipients.m_Memory.m_nAllocationCount - (pPVar28->location).y
           , fVar37 = (float)local_78.m_Recipients.m_Memory.m_nGrowSize - (pPVar28->location).z,
           *(float *)(*(int *)(unaff_EBX + 0x69486b) + 0x2c) <=
           fVar32 * fVar32 + fVar33 * fVar33 + fVar37 * fVar37)) {
          iVar25 = iVar25 + 1;
        }
        else if (0 < local_338) {
          if (iVar25 != local_338 + -1) {
            pPVar30 = pPVar30 + local_338 + -1;
            for (iVar26 = 8; iVar26 != 0; iVar26 = iVar26 + -1) {
              pPVar28->pBrushEntity = pPVar30->pBrushEntity;
              pPVar30 = (PaintLocationData_t *)&pPVar30->type;
              pPVar28 = (PaintLocationData_t *)&pPVar28->type;
            }
          }
          piVar19 = &(this->m_PaintThisFrame).m_Size;
          *piVar19 = *piVar19 + -1;
          local_338 = (this->m_PaintThisFrame).m_Size;
        }
      }
    } while (local_310 < local_338);
  }
  local_2ac = local_29c;
  local_2a8 = 0x10;
  local_2a4 = -1;
  local_11c = 0;
  local_118 = local_2ac;
  if (0 < local_338) {
    local_32c = 0;
    puVar8 = *(undefined4 **)(&DAT_0051230b + unaff_EBX);
    do {
      pPVar28 = (this->m_PaintThisFrame).m_Memory.m_pMemory + local_32c;
      pCVar31 = &local_78;
      for (iVar25 = 8; iVar25 != 0; iVar25 = iVar25 + -1) {
        (pCVar31->super_IRecipientFilter)._vptr_IRecipientFilter =
             (_func_int_varargs **)pPVar28->pBrushEntity;
        pPVar28 = (PaintLocationData_t *)&pPVar28->type;
        pCVar31 = (CRecipientFilter *)&pCVar31->m_bReliable;
      }
      fVar32 = UTIL_PaintBrushEntity
                         ((CBaseEntity *)local_78.super_IRecipientFilter._vptr_IRecipientFilter,
                          (Vector *)&local_78.m_Recipients,local_78._4_4_);
      uVar17 = local_11c;
      if (fVar32 != 0.0) {
        fVar33 = (float)local_78.m_Recipients.m_Memory.m_pMemory - fVar32;
        fVar38 = (float)local_78.m_Recipients.m_Memory.m_nAllocationCount - fVar32;
        fVar39 = (float)local_78.m_Recipients.m_Memory.m_nGrowSize - fVar32;
        fVar34 = (float)local_78.m_Recipients.m_Memory.m_pMemory + fVar32;
        fVar37 = (float)local_78.m_Recipients.m_Memory.m_nAllocationCount + fVar32;
        fVar32 = fVar32 + (float)local_78.m_Recipients.m_Memory.m_nGrowSize;
        if (0 < (int)local_11c) {
          local_30c = 0;
          iVar25 = 0;
          do {
                    /* Unresolved local var: Bounds_t * b@[???] */
            pfVar27 = (float *)(iVar25 + (int)local_2ac);
            fVar35 = pfVar27[2];
            if (fVar39 <= pfVar27[2]) {
              fVar35 = fVar39;
            }
            fVar2 = pfVar27[1];
            if (fVar38 <= pfVar27[1]) {
              fVar2 = fVar38;
            }
            fVar16 = *pfVar27;
            if (fVar33 <= *pfVar27) {
              fVar16 = fVar33;
            }
            fVar36 = pfVar27[5];
            if (pfVar27[5] <= fVar32) {
              fVar36 = fVar32;
            }
            fVar3 = pfVar27[4];
            if (pfVar27[4] <= fVar37) {
              fVar3 = fVar37;
            }
            local_58 = pfVar27[3];
            if (pfVar27[3] <= fVar34) {
              local_58 = fVar34;
            }
                    /* Unresolved local var: Vector delta@[???] */
            local_54 = fVar3;
            local_50 = fVar36;
            local_4c = local_58;
            local_48 = fVar3;
            local_44 = fVar36;
            local_40 = fVar16;
            local_3c = fVar2;
            local_38 = fVar35;
            local_34 = fVar16;
            local_30 = fVar2;
            local_2c = fVar35;
            if ((fVar16 - local_58) * (fVar16 - local_58) + (fVar2 - fVar3) * (fVar2 - fVar3) +
                (fVar35 - fVar36) * (fVar35 - fVar36) < *(float *)(unaff_EBX + 0x3a0dc3)) {
              pfVar27[3] = local_58;
              pfVar27[4] = fVar3;
              pfVar27[5] = fVar36;
              *pfVar27 = fVar16;
              pfVar27[1] = fVar2;
              pfVar27[2] = fVar35;
              goto LAB_00734cc4;
            }
            local_30c = local_30c + 1;
            iVar25 = iVar25 + 0x18;
          } while (local_30c != local_11c);
        }
        iVar25 = local_11c + 1;
        if (local_2a8 < iVar25) {
          if (local_2a4 < 0) {
            local_2a4 = 0;
            if (local_2a8 == 0) {
              local_2ac = (undefined4 *)0x0;
            }
            else {
              uVar22 = local_2a8 * 0x18;
              puVar24 = (undefined4 *)*puVar8;
              puVar23 = (undefined4 *)(**(code **)*puVar24)(puVar24,uVar22);
              puVar24 = puVar23;
              if ((7 < uVar22) && (((uint)puVar23 & 4) != 0)) {
                *puVar23 = *local_2ac;
                puVar24 = puVar23 + 1;
                local_2ac = local_2ac + 1;
                uVar22 = uVar22 - 4;
              }
              puVar29 = local_2ac;
              for (uVar22 = uVar22 >> 2; local_2ac = puVar23, uVar22 != 0; uVar22 = uVar22 - 1) {
                *puVar24 = *puVar29;
                puVar29 = puVar29 + 1;
                puVar24 = puVar24 + 1;
              }
            }
          }
          if (local_2a4 == 0) {
            if (local_2a8 == 0) {
              local_2a8 = 2;
            }
            for (; local_2a8 < iVar25; local_2a8 = local_2a8 * 2) {
            }
LAB_00735276:
            iVar26 = local_2a8 * 0x18;
          }
          else {
            local_2a8 = ((int)uVar17 / local_2a4 + 1) * local_2a4;
            if (iVar25 <= local_2a8) goto LAB_00735276;
            if (local_2a8 != 0) {
LAB_00735266:
              do {
                local_2a8 = (local_2a8 + iVar25) / 2;
              } while (local_2a8 < iVar25);
              goto LAB_00735276;
            }
            if (iVar25 < 0) {
              local_2a8 = -1;
              iVar26 = -0x18;
            }
            else {
              iVar26 = 0;
              if (iVar25 != 0) goto LAB_00735266;
            }
          }
          if (local_2ac == (undefined4 *)0x0) {
            local_2ac = (undefined4 *)
                        (*(code *)**(undefined4 **)**(undefined4 **)(&DAT_0051230b + unaff_EBX))
                                  ((undefined4 *)**(undefined4 **)(&DAT_0051230b + unaff_EBX),iVar26
                                  );
          }
          else {
            piVar19 = (int *)*puVar8;
            local_2ac = (undefined4 *)(**(code **)(*piVar19 + 4))(piVar19,local_2ac,iVar26);
          }
        }
        local_11c = local_11c + 1;
        local_118 = local_2ac;
        if (0 < (int)(~uVar17 + local_11c)) {
          _V_memmove(local_2ac + iVar25 * 6,local_2ac + uVar17 * 6,(~uVar17 + local_11c) * 0x18);
        }
        pfVar27 = (float *)(local_2ac + uVar17 * 6);
        *pfVar27 = fVar33;
        pfVar27[1] = fVar38;
        pfVar27[2] = fVar39;
        local_2ac[uVar17 * 6 + 3] = fVar34;
        local_2ac[uVar17 * 6 + 4] = fVar37;
        local_2ac[uVar17 * 6 + 5] = fVar32;
      }
LAB_00734cc4:
      local_32c = local_32c + 1;
    } while (local_32c != local_338);
  }
  local_114 = &UNK_00599127 + unaff_EBX;
  local_110 = local_100;
  local_10c = 0x20;
  local_108 = -1;
  local_104 = 0;
  local_80 = 0;
                    /* Unresolved local var: int i@[???] */
  local_7c = local_110;
  if (0 < (int)local_11c) {
    local_328 = 0;
    iVar25 = 0;
    do {
      (**(code **)(*(int *)**(undefined4 **)(&DAT_0051237f + unaff_EBX) + 0x2c))
                ((int *)**(undefined4 **)(&DAT_0051237f + unaff_EBX),iVar25 + (int)local_2ac,
                 iVar25 + (int)local_2ac + 0xc,&local_114);
      local_328 = local_328 + 1;
      iVar25 = iVar25 + 0x18;
    } while (local_328 < (int)local_11c);
  }
                    /* Unresolved local var: int i@[???] */
  if (0 < local_80) {
    iVar25 = 0;
    do {
                    /* Unresolved local var: CBaseEntity * pEntity@[???] */
      pCVar9 = *(CBaseEntity **)(local_110 + iVar25 * 4);
      CBaseEntity::AddFlag(pCVar9,0x80000);
      (*(pCVar9->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
        [100])(pCVar9);
      iVar25 = iVar25 + 1;
    } while (iVar25 < local_80);
  }
  if (*(int *)(**(int **)(&DAT_00512313 + unaff_EBX) + 0x14) < 2) {
LAB_00735485:
    (this->m_PaintThisFrame).m_Size = 0;
    local_114 = &UNK_00599127 + unaff_EBX;
    local_80 = 0;
    if (-1 < local_108) {
      if (local_110 != (undefined1 *)0x0) {
        (**(code **)(*(int *)**(undefined4 **)(&DAT_0051230b + unaff_EBX) + 8))
                  ((int *)**(undefined4 **)(&DAT_0051230b + unaff_EBX),local_110);
        local_110 = (undefined1 *)0x0;
      }
      local_10c = 0;
    }
    local_7c = local_110;
    if (-1 < local_108) {
      if (local_110 != (undefined1 *)0x0) {
        (**(code **)(*(int *)**(undefined4 **)(&DAT_0051230b + unaff_EBX) + 8))
                  ((int *)**(undefined4 **)(&DAT_0051230b + unaff_EBX),local_110);
        local_110 = (undefined1 *)0x0;
      }
      local_10c = 0;
    }
    local_11c = 0;
    if ((-1 < local_2a4) && (local_2ac != (undefined4 *)0x0)) {
      (**(code **)(*(int *)**(undefined4 **)(&DAT_0051230b + unaff_EBX) + 8))
                ((int *)**(undefined4 **)(&DAT_0051230b + unaff_EBX),local_2ac);
LAB_0073554d:
      local_2ac = (undefined4 *)0x0;
    }
  }
  else {
    pPlayer = UTIL_PlayerByIndex(2);
    if (pPlayer != (CBasePlayer *)0x0) {
      if ((this->m_bSentClientPaintData == false) && (pPlayer->m_iConnected != PlayerDisconnected))
      {
        SendPaintDataTo(this,pPlayer);
      }
      this->m_bSentClientPaintData = true;
      sVar10 = (this->m_PaintThisFrame).m_Size;
      if (1 < (int)sVar10) {
        pPVar28 = (this->m_PaintThisFrame).m_Memory.m_pMemory;
        if (pPVar28 == (PaintLocationData_t *)0x0) {
          local_2f8 = sVar10 - 1;
          do {
            if (0 < local_2f8) {
              local_2fc = 1;
              local_2e4 = 0;
              do {
                pPVar28 = (this->m_PaintThisFrame).m_Memory.m_pMemory;
                pPVar30 = pPVar28 + local_2fc;
                pPVar28 = (PaintLocationData_t *)((int)&pPVar28->pBrushEntity + local_2e4);
                iVar25 = AscendingPaintLocationCompare(pPVar28,pPVar30);
                if (iVar25 < 0) {
                  vVar14 = (pPVar28->normal).z;
                  vVar15 = (pPVar28->normal).y;
                  vVar4 = (pPVar28->normal).x;
                  vVar5 = (pPVar28->location).z;
                  vVar6 = (pPVar28->location).y;
                  vVar7 = (pPVar28->location).x;
                  PVar11 = pPVar28->type;
                  pCVar9 = pPVar28->pBrushEntity;
                    /* Unresolved local var: PaintLocationData_t temp@[???] */
                  pPVar28->pBrushEntity = pPVar30->pBrushEntity;
                  pPVar28->type = pPVar30->type;
                  (pPVar28->location).x = (pPVar30->location).x;
                  (pPVar28->location).y = (pPVar30->location).y;
                  (pPVar28->location).z = (pPVar30->location).z;
                  (pPVar28->normal).x = (pPVar30->normal).x;
                  (pPVar28->normal).y = (pPVar30->normal).y;
                  (pPVar28->normal).z = (pPVar30->normal).z;
                  pPVar30->pBrushEntity = pCVar9;
                  pPVar30->type = PVar11;
                  (pPVar30->location).x = vVar7;
                  (pPVar30->location).y = vVar6;
                  (pPVar30->location).z = vVar5;
                  (pPVar30->normal).x = vVar4;
                  (pPVar30->normal).y = vVar15;
                  (pPVar30->normal).z = vVar14;
                }
                local_2fc = local_2fc + 1;
                local_2e4 = local_2e4 + 0x20;
              } while (local_2fc <= local_2f8);
            }
            local_2f8 = local_2f8 + -1;
          } while (local_2f8 != -1);
        }
        else {
          _qsort(pPVar28,sVar10,0x20,*(int **)(&DAT_00631153 + unaff_EBX));
        }
      }
      CRecipientFilter::CRecipientFilter(&local_78);
      local_78.super_IRecipientFilter._vptr_IRecipientFilter =
           (_func_int_varargs **)(&UNK_00514897 + unaff_EBX);
      CRecipientFilter::AddRecipient(&local_78,pPlayer);
      CRecipientFilter::MakeReliable(&local_78);
      local_2dc = (this->m_PaintThisFrame).m_Memory.m_pMemory;
      pPVar28 = local_2dc + (this->m_PaintThisFrame).m_Size;
      if (local_2dc != pPVar28) {
        do {
          pPlayer = (CBasePlayer *)(unaff_EBX + 0x37dc55);
          UserMessageBegin(&local_78.super_IRecipientFilter,(char *)(unaff_EBX + 0x37dc55));
          PVar11 = local_2dc->type;
          MessageWriteByte(PVar11);
          pCVar9 = local_2dc->pBrushEntity;
          MessageWriteEHandle(pCVar9);
          local_2c8 = (int)pPVar28 - (int)local_2dc;
          iVar26 = local_2c8 >> 7;
          pPVar30 = local_2dc;
          iVar25 = local_2c8;
          if (iVar26 < 1) {
LAB_007355bf:
            iVar25 = iVar25 >> 5;
            if (iVar25 == 2) {
LAB_00735649:
              if ((pCVar9 == pPVar30->pBrushEntity) && (PVar11 == pPVar30->type)) {
                pPVar30 = pPVar30 + 1;
LAB_007355d4:
                if ((pCVar9 == pPVar30->pBrushEntity) && (PVar11 == pPVar30->type))
                goto LAB_00735010;
              }
LAB_007355e2:
              local_2c8 = (int)pPVar30 - (int)local_2dc;
            }
            else if (iVar25 == 3) {
              if ((pCVar9 == pPVar30->pBrushEntity) && (PVar11 == pPVar30->type)) {
                pPVar30 = pPVar30 + 1;
                goto LAB_00735649;
              }
              local_2c8 = (int)pPVar30 - (int)local_2dc;
            }
            else if (iVar25 == 1) goto LAB_007355d4;
LAB_00735010:
            local_318 = 0x28;
            if (local_2c8 < 0x500) goto LAB_00735027;
          }
          else {
            if ((pCVar9 == local_2dc->pBrushEntity) && (PVar11 == local_2dc->type)) {
              pPVar20 = local_2dc + 1;
              if (local_2dc[1].pBrushEntity == pCVar9) {
                if (((local_2dc[1].type == PVar11) &&
                    (pPVar20 = local_2dc + 2, local_2dc[2].pBrushEntity == pCVar9)) &&
                   (PVar12 = local_2dc[2].type, PVar12 == PVar11)) {
                  pPVar20 = local_2dc + 3;
                  pCVar13 = local_2dc[3].pBrushEntity;
                  if (pCVar13 != pCVar9) goto LAB_00735611;
                  do {
                    if (PVar12 != pPVar20->type) break;
                    pPVar30 = pPVar20 + 1;
                    iVar26 = iVar26 + -1;
                    if (iVar26 == 0) {
                      iVar25 = (int)pPVar28 - (int)pPVar30;
                      goto LAB_007355bf;
                    }
                    if ((pCVar13 != pPVar20[1].pBrushEntity) ||
                       ((((PVar12 != pPVar20[1].type ||
                          (pPVar30 = pPVar20 + 2, pCVar13 != pPVar20[2].pBrushEntity)) ||
                         (PVar12 != pPVar20[2].type)) ||
                        ((pPVar30 = pPVar20 + 3, pCVar13 != pPVar20[3].pBrushEntity ||
                         (PVar12 != pPVar20[3].type)))))) goto LAB_007355e2;
                    pPVar20 = pPVar20 + 4;
                  } while (pCVar13 == pPVar20->pBrushEntity);
                }
                local_2c8 = (int)pPVar20 - (int)local_2dc;
              }
              else {
LAB_00735611:
                local_2c8 = (int)pPVar20 - (int)local_2dc;
              }
              goto LAB_00735010;
            }
            local_2c8 = 0;
LAB_00735027:
            local_318 = local_2c8 >> 5;
          }
          MessageWriteByte(local_318);
          if (local_318 < 1) {
            fVar37 = 0.0;
            fVar33 = 0.0;
            fVar32 = fVar33;
          }
          else {
            iVar25 = 0;
            fVar37 = 0.0;
            fVar33 = 0.0;
            fVar32 = fVar33;
            pPVar30 = local_2dc;
            do {
              pVVar21 = &pPVar30->location;
              fVar33 = fVar33 + pVVar21->x;
              fVar32 = fVar32 + pVVar21->y;
              fVar37 = fVar37 + pVVar21->z;
              iVar25 = iVar25 + 1;
              pPVar30 = (PaintLocationData_t *)(pVVar21 + 2);
            } while (iVar25 != local_318);
          }
          fVar34 = *(float *)(unaff_EBX + 0x3980bf) / (float)local_318;
          MessageWriteFloat(fVar33 * fVar34);
          MessageWriteFloat(fVar32 * fVar34);
          MessageWriteFloat(fVar37 * fVar34);
          if (0 < local_318) {
            iVar25 = 0;
            do {
              pPVar30 = local_2dc + 1;
                    /* Unresolved local var: Vector offset@[???]
                       Unresolved local var: Vector res@[???] */
              local_28 = (local_2dc->location).x - fVar33 * fVar34;
              fVar38 = (local_2dc->location).y - fVar32 * fVar34;
              fVar39 = (local_2dc->location).z - fVar37 * fVar34;
              local_24 = fVar38;
              local_20 = fVar39;
              MessageWriteShort((int)(short)(int)local_28);
              MessageWriteShort((int)(short)(int)fVar38);
              MessageWriteShort((int)(short)(int)fVar39);
              iVar25 = iVar25 + 1;
              local_2dc = pPVar30;
            } while (iVar25 != local_318);
          }
          MessageEnd();
        } while (local_2dc != pPVar28);
      }
      local_78.super_IRecipientFilter._vptr_IRecipientFilter =
           (_func_int_varargs **)(&UNK_00514897 + unaff_EBX);
      CRecipientFilter::~CRecipientFilter(&local_78,(int)pPlayer);
      goto LAB_00735485;
    }
    this->m_bSentClientPaintData = false;
    (this->m_PaintThisFrame).m_Size = 0;
    local_114 = &UNK_00599127 + unaff_EBX;
    local_80 = 0;
    if (-1 < local_108) {
      if (local_110 != (undefined1 *)0x0) {
        (**(code **)(*(int *)**(undefined4 **)(&DAT_0051230b + unaff_EBX) + 8))
                  ((int *)**(undefined4 **)(&DAT_0051230b + unaff_EBX),local_110);
        local_110 = (undefined1 *)0x0;
      }
      local_10c = 0;
    }
    local_7c = local_110;
    if (-1 < local_108) {
      if (local_110 != (undefined1 *)0x0) {
        (**(code **)(*(int *)**(undefined4 **)(&DAT_0051230b + unaff_EBX) + 8))
                  ((int *)**(undefined4 **)(&DAT_0051230b + unaff_EBX),local_110);
        local_110 = (undefined1 *)0x0;
      }
      local_10c = 0;
    }
    local_11c = 0;
    if ((-1 < local_2a4) && (local_2ac != (undefined4 *)0x0)) {
      (**(code **)(*(int *)**(undefined4 **)(&DAT_0051230b + unaff_EBX) + 8))
                ((int *)**(undefined4 **)(&DAT_0051230b + unaff_EBX),local_2ac);
      goto LAB_0073554d;
    }
  }
  local_118 = local_2ac;
  if ((-1 < local_2a4) && (local_2ac != (undefined4 *)0x0)) {
    (**(code **)(*(int *)**(undefined4 **)(&DAT_0051230b + unaff_EBX) + 8))
              ((int *)**(undefined4 **)(&DAT_0051230b + unaff_EBX),local_2ac);
  }
LAB_007349cd:
  if ((((&DAT_00001010)[*(int *)(&DAT_00512333 + unaff_EBX)] == '\0') ||
      (*(int *)(&DAT_0000100c + *(int *)(&DAT_00512333 + unaff_EBX)) != 0)) &&
     (iVar25 = *(int *)(*(int *)(&DAT_00512333 + unaff_EBX) + 0x19b8),
     iVar26 = _ThreadGetCurrentId(), iVar25 == iVar26)) {
    cVar18 = CVProfNode::ExitScope();
    if (cVar18 != '\0') {
      *(undefined4 *)(&DAT_00001014 + *(int *)(&DAT_00512333 + unaff_EBX)) =
           *(undefined4 *)(*(int *)(&DAT_00001014 + *(int *)(&DAT_00512333 + unaff_EBX)) + 100);
    }
    iVar25 = *(int *)(&DAT_00512333 + unaff_EBX);
    (&DAT_00001010)[iVar25] = *(undefined **)(&DAT_00001014 + iVar25) == &DAT_00001018 + iVar25;
    return;
  }
  return;
}


/* CPaintAffectedEntityList::EnumEntity at 00736e30 */

/* DWARF original prototype: bool EnumEntity(CPaintAffectedEntityList * this, IHandleEntity *
   pHandleEntity) */

bool __thiscall
CPaintAffectedEntityList::EnumEntity(CPaintAffectedEntityList *this,IHandleEntity *pHandleEntity)

{
  uint uVar1;
  int *piVar2;
  uint *puVar3;
  CBaseEntity *this_00;
  int iVar4;
  undefined4 *puVar5;
  CBaseEntity **ppCVar6;
  CBaseEntity **ppCVar7;
  CBaseEntity **ppCVar8;
  int iVar9;
  int iVar10;
  int unaff_EBX;
  int iVar11;
  uint uVar12;
  int local_20;
  
                    /* Unresolved local var: CBaseEntity * pEntity@[???] */
  ___i686_get_pc_thunk_bx();
  if (pHandleEntity == (IHandleEntity *)0x0) {
    return true;
  }
  puVar3 = (uint *)(*pHandleEntity->_vptr_IHandleEntity[3])(pHandleEntity);
  uVar1 = *puVar3;
  if (uVar1 == 0xffffffff) {
    return true;
  }
  iVar9 = (uVar1 & 0xffff) * 0x10 + *(int *)(unaff_EBX + 0x50fde0);
  if (*(uint *)(iVar9 + 8) != uVar1 >> 0x10) {
    return true;
  }
  piVar2 = *(int **)(iVar9 + 4);
  if (piVar2 == (int *)0x0) {
    return true;
  }
  this_00 = (CBaseEntity *)(**(code **)(*piVar2 + 0x18))(piVar2);
  if (this_00 == (CBaseEntity *)0x0) {
    return true;
  }
  if (((this_00->m_fFlags).m_Value & 0x80000) == 0) {
    return true;
  }
  CBaseEntity::RemoveFlag(this_00,0x80000);
  uVar1 = (this->m_EntitiesToUpdate).
          super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.m_Size;
  iVar4 = uVar1 + 1;
  iVar9 = (this->m_EntitiesToUpdate).
          super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.m_Memory.
          super_CUtlMemory<CBaseEntity*,int>.m_nAllocationCount;
  if (iVar4 <= iVar9) goto LAB_00736f10;
  iVar11 = (this->m_EntitiesToUpdate).
           super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.m_Memory
           .super_CUtlMemory<CBaseEntity*,int>.m_nGrowSize;
  iVar10 = iVar9;
  if (iVar11 < 0) {
    (this->m_EntitiesToUpdate).
    super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.m_Memory.
    super_CUtlMemory<CBaseEntity*,int>.m_nGrowSize =
         (this->m_EntitiesToUpdate).
         super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.m_Memory.
         m_nMallocGrowSize;
    if (iVar9 == 0) {
      (this->m_EntitiesToUpdate).
      super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.m_Memory.
      super_CUtlMemory<CBaseEntity*,int>.m_pMemory = (CBaseEntity **)0x0;
    }
    else {
                    /* Unresolved local var: int nNumBytes@[???]
                       Unresolved local var: CBaseEntity * * pMemory@[???] */
      uVar12 = iVar9 * 4;
      ppCVar7 = (CBaseEntity **)
                (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x50fd18))
                          ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x50fd18),uVar12);
      ppCVar6 = (this->m_EntitiesToUpdate).
                super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.
                m_Memory.super_CUtlMemory<CBaseEntity*,int>.m_pMemory;
      ppCVar8 = ppCVar7;
      if ((7 < uVar12) && (((uint)ppCVar7 & 4) != 0)) {
        *ppCVar7 = *ppCVar6;
        ppCVar8 = ppCVar7 + 1;
        ppCVar6 = ppCVar6 + 1;
        uVar12 = uVar12 - 4;
      }
      for (uVar12 = uVar12 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
        *ppCVar8 = *ppCVar6;
        ppCVar6 = ppCVar6 + 1;
        ppCVar8 = ppCVar8 + 1;
      }
      (this->m_EntitiesToUpdate).
      super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.m_Memory.
      super_CUtlMemory<CBaseEntity*,int>.m_pMemory = ppCVar7;
    }
    iVar11 = (this->m_EntitiesToUpdate).
             super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.
             m_Memory.super_CUtlMemory<CBaseEntity*,int>.m_nGrowSize;
    if (iVar11 < 0) goto LAB_00736f10;
    iVar10 = (this->m_EntitiesToUpdate).
             super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.
             m_Memory.super_CUtlMemory<CBaseEntity*,int>.m_nAllocationCount;
  }
  iVar9 = (iVar4 - iVar9) + iVar10;
  if (iVar11 == 0) {
    if (iVar10 == 0) {
      iVar10 = 8;
    }
    if (iVar10 < iVar9) {
      do {
        local_20 = iVar10;
        iVar10 = local_20 * 2;
      } while (iVar10 < iVar9);
      local_20 = local_20 * 8;
    }
    else {
      local_20 = iVar10 * 4;
    }
  }
  else {
    iVar10 = ((iVar9 + -1) / iVar11 + 1) * iVar11;
    if (iVar10 < iVar9) {
      if (iVar10 == 0) {
        if (iVar9 < 0) {
          iVar10 = -1;
          local_20 = -4;
        }
        else {
          local_20 = 0;
          if (iVar9 != 0) goto LAB_00736f90;
        }
      }
      else {
LAB_00736f90:
        do {
          iVar10 = (iVar10 + iVar9) / 2;
        } while (iVar10 < iVar9);
        local_20 = iVar10 * 4;
      }
    }
    else {
      local_20 = iVar10 * 4;
    }
  }
  (this->m_EntitiesToUpdate).
  super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.m_Memory.
  super_CUtlMemory<CBaseEntity*,int>.m_nAllocationCount = iVar10;
  ppCVar6 = (this->m_EntitiesToUpdate).
            super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.
            m_Memory.super_CUtlMemory<CBaseEntity*,int>.m_pMemory;
  if (ppCVar6 == (CBaseEntity **)0x0) {
    ppCVar6 = (CBaseEntity **)
              (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x50fd18))
                        ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x50fd18),local_20);
    (this->m_EntitiesToUpdate).
    super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.m_Memory.
    super_CUtlMemory<CBaseEntity*,int>.m_pMemory = ppCVar6;
  }
  else {
    ppCVar6 = (CBaseEntity **)
              (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x50fd18) + 4))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x50fd18),ppCVar6,local_20);
    (this->m_EntitiesToUpdate).
    super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.m_Memory.
    super_CUtlMemory<CBaseEntity*,int>.m_pMemory = ppCVar6;
  }
LAB_00736f10:
  piVar2 = &(this->m_EntitiesToUpdate).
            super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.m_Size;
  *piVar2 = *piVar2 + 1;
  ppCVar6 = (this->m_EntitiesToUpdate).
            super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.
            m_Memory.super_CUtlMemory<CBaseEntity*,int>.m_pMemory;
  (this->m_EntitiesToUpdate).
  super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.m_pElements =
       ppCVar6;
  iVar9 = ~uVar1 + (this->m_EntitiesToUpdate).
                   super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.
                   m_Size;
  if (iVar9 < 1) {
    iVar11 = uVar1 << 2;
  }
  else {
    iVar11 = uVar1 * 4;
    _V_memmove(ppCVar6 + iVar4,ppCVar6 + uVar1,iVar9 * 4);
  }
  puVar5 = (undefined4 *)
           (iVar11 + (int)(this->m_EntitiesToUpdate).
                          super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>
                          .m_Memory.super_CUtlMemory<CBaseEntity*,int>.m_pMemory);
  if (puVar5 != (undefined4 *)0x0) {
    *puVar5 = this_00;
  }
  return true;
}


/* CC_RemoveAllPaint at 007339b0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void CC_RemoveAllPaint(void)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CPaintDatabase::RemoveAllPaint((CPaintDatabase *)(unaff_EBX + 0x695664));
  return;
}


/* __tcf_2 at 00a3dc50 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x40fb08),in_stack_00000008);
  return;
}


/* CC_PaintAt at 007335d0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void CC_PaintAt(CCommand *args)

{
  PaintPowerType powerType;
  char *pcVar1;
  int unaff_EBX;
  double dVar2;
  double dVar3;
  double dVar4;
  trace_t local_88;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  Vector local_28 [2];
  
                    /* Unresolved local var: PaintPowerType power@[???]
                       Unresolved local var: Vector pos@[???]
                       Unresolved local var: trace_t tr@[???] */
  ___i686_get_pc_thunk_bx();
  if (args->m_nArgc < 5) {
    DevMsg((char *)(unaff_EBX + 0x37edbd));
  }
  else {
    powerType = _atoi(args->m_ppArgv[1]);
    pcVar1 = &UNK_0031b73d + unaff_EBX;
    if (4 < args->m_nArgc) {
      pcVar1 = args->m_ppArgv[4];
    }
    dVar2 = _atof(pcVar1);
    pcVar1 = &UNK_0031b73d + unaff_EBX;
    if (3 < args->m_nArgc) {
      pcVar1 = args->m_ppArgv[3];
    }
    dVar3 = _atof(pcVar1);
    pcVar1 = &UNK_0031b73d + unaff_EBX;
    if (2 < args->m_nArgc) {
      pcVar1 = args->m_ppArgv[2];
    }
    dVar4 = _atof(pcVar1);
    local_28[0].x = (vec_t)dVar4;
    local_28[0].y = (vec_t)dVar3;
    local_28[0].z = (vec_t)dVar2;
    local_88.m_pEnt = &GetWorldEntity()->super_CBaseEntity;
    local_88.super_CBaseTrace.endpos.x = local_28[0].x;
    local_88.super_CBaseTrace.endpos.y = local_28[0].y;
    local_88.super_CBaseTrace.endpos.z = local_28[0].z;
    local_34 = 0;
    local_30 = 0;
    local_2c = 0;
    local_88.super_CBaseTrace.plane.normal.x = 0.0;
    local_88.super_CBaseTrace.plane.normal.y = 0.0;
    local_88.super_CBaseTrace.plane.normal.z = 0.0;
    CPaintDatabase::AddPaint((CPaintDatabase *)(unaff_EBX + 0x695a39),&local_88,powerType);
    if (args->m_nArgc == 6) {
      dVar2 = _atof(args->m_ppArgv[5]);
      NDebugOverlay::Sphere(local_28,2.0,0xff,0xff,0,true,(float)dVar2);
    }
  }
  return;
}


/* __tcf_3 at 00a3dc30 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x40fb68),in_stack_00000008);
  return;
}


/* CPaintDatabase::SavePaintmapData at 00735b90 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void SavePaintmapData(CPaintDatabase * this, ISave * pSave) */

void __thiscall CPaintDatabase::SavePaintmapData(CPaintDatabase *this,ISave *pSave)

{
  char *pcVar1;
  undefined4 *puVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int unaff_EBX;
  int iVar6;
  int *piVar7;
  int i;
  int local_6c;
  int local_64;
  int local_60;
  int local_50 [5];
  uint local_3c;
  ushort uStack_3a;
  undefined4 local_38;
  undefined4 local_34;
  int local_30;
  int numBits;
  int local_28;
  int nPaintData;
  char local_1d [13];
  
                    /* Unresolved local var:
                       CUtlVector<CUtlVector<unsigned_char,_CUtlMemory<unsigned_char,_int>_>,CUtlMemory<CUtlVector<unsigned_char,_CUtlMemory<unsigned_char,_int>_>,_int>_>
                       data@[???]
                       Unresolved local var: CVarBitVec paintSurfBits@[???] */
  ___i686_get_pc_thunk_bx();
  local_50[0] = 0;
  local_50[1] = 0;
  local_50[2] = 0;
  local_50[3] = 0;
  local_50[4] = 0;
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x510fdf) + 0x220))
            ((int *)**(undefined4 **)(unaff_EBX + 0x510fdf),local_50);
  nPaintData = local_50[3];
  (*pSave->_vptr_ISave[0xb])(pSave,&nPaintData,1);
  if (0 < nPaintData) {
    i = 0;
    local_60 = 0;
    do {
      (*pSave->_vptr_ISave[8])(pSave);
      local_28 = *(int *)(local_50[0] + 0xc + local_60);
      (*pSave->_vptr_ISave[0xb])(pSave,&local_28,1);
      iVar3 = local_28;
      iVar4 = *(int *)(local_50[0] + local_60);
      if (0 < local_28) {
        iVar5 = 0;
        do {
          local_30 = 1;
          local_1d[0] = *(char *)(iVar4 + iVar5);
          iVar6 = iVar5 + 1;
          if ((iVar6 < iVar3) && (local_1d[0] == *(char *)(iVar5 + 1 + iVar4))) {
            iVar5 = iVar5 + iVar4;
            do {
              local_30 = local_30 + 1;
              iVar6 = iVar6 + 1;
              if (iVar3 <= iVar6) break;
              pcVar1 = (char *)(iVar5 + 2);
              iVar5 = iVar5 + 1;
            } while (*pcVar1 == local_1d[0]);
          }
          iVar5 = iVar6;
          (*pSave->_vptr_ISave[0xb])(pSave,&local_30,1);
          (*pSave->_vptr_ISave[0xe])(pSave,local_1d,1);
        } while (iVar5 < iVar3);
      }
      (*pSave->_vptr_ISave[9])(pSave);
      i = i + 1;
      local_60 = local_60 + 0x14;
    } while (i < nPaintData);
  }
  local_3c = 0;
  local_38 = 0;
  local_34 = 0;
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x510fdf) + 0x228))
            ((int *)**(undefined4 **)(unaff_EBX + 0x510fdf),&stack0xffffffc4);
  numBits = local_3c & 0xffff;
  (*pSave->_vptr_ISave[8])(pSave);
  (*pSave->_vptr_ISave[0xb])(pSave,&numBits,1);
  (*pSave->_vptr_ISave[0xb])(pSave,local_34,local_3c >> 0x10);
  (*pSave->_vptr_ISave[9])(pSave);
  if (1 < uStack_3a) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x510fbb) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x510fbb),local_34);
  }
  local_34 = 0;
  local_6c = local_50[3];
  local_64 = local_50[3] * 0x14;
  puVar2 = *(undefined4 **)(unaff_EBX + 0x510fbb);
  while( true ) {
    local_64 = local_64 + -0x14;
    local_6c = local_6c + -1;
    if (local_6c < 0) break;
    piVar7 = (int *)(local_64 + local_50[0]);
    piVar7[3] = 0;
    if (piVar7[2] < 0) {
      iVar4 = *piVar7;
    }
    else {
      if (*piVar7 != 0) {
        (**(code **)(*(int *)*puVar2 + 8))((int *)*puVar2,*piVar7);
        *piVar7 = 0;
      }
      iVar4 = 0;
      piVar7[1] = 0;
    }
    piVar7[4] = iVar4;
    if (-1 < piVar7[2]) {
      if (iVar4 != 0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x510fbb) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x510fbb),iVar4);
        *piVar7 = 0;
      }
      piVar7[1] = 0;
    }
  }
  local_50[3] = 0;
  if (-1 < local_50[2]) {
    if (local_50[0] != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x510fbb) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x510fbb),local_50[0]);
      local_50[0] = 0;
    }
    local_50[1] = 0;
  }
  if ((-1 < local_50[2]) && (local_50[0] != 0)) {
    local_50[4] = local_50[0];
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x510fbb) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x510fbb),local_50[0]);
  }
  return;
}


/* CPaintDatabase::RestorePaintmapData at 007339d0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void RestorePaintmapData(CPaintDatabase * this, IRestore * pRestore) */

void __thiscall CPaintDatabase::RestorePaintmapData(CPaintDatabase *this,IRestore *pRestore)

{
  uchar *src;
  int iVar1;
  int iVar2;
  int count;
  uint uVar3;
  uint uVar4;
  int iVar5;
  int unaff_EBX;
  CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *this_00;
  int iVar6;
  undefined4 *puVar7;
  int nPaintData;
  int i;
  int local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 *local_24;
  byte local_20 [16];
  
                    /* Unresolved local var: CVarBitVec paintSurfBits@[???] */
  ___i686_get_pc_thunk_bx();
  iVar1 = (*pRestore->_vptr_IRestore[0xf])(pRestore);
  CUtlVector<CUtlVector<unsigned_char,_CUtlMemory<unsigned_char,_int>_>,CUtlMemory<CUtlVector<unsigned_char,_CUtlMemory<unsigned_char,_int>_>,_int>_>
  ::SetCount(&this->m_Paintmaps,iVar1);
  DevMsg((char *)(unaff_EBX + 0x37ea11),iVar1);
  if (0 < iVar1) {
    i = 0;
    local_30 = 0;
    do {
      (*pRestore->_vptr_IRestore[7])(pRestore);
      iVar2 = (*pRestore->_vptr_IRestore[0xf])(pRestore);
      this_00 = (CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *)
                ((int)&(((this->m_Paintmaps).m_Memory.m_pMemory)->m_Memory).m_pMemory + local_30);
      this_00->m_Size = 0;
      if (iVar2 != 0) {
        CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_>::GrowVector(this_00,iVar2);
                    /* Unresolved local var: int numToMove@[???] */
        iVar5 = this_00->m_Size - iVar2;
        if ((0 < iVar5) && (0 < iVar2)) {
          src = (this_00->m_Memory).m_pMemory;
          _V_memmove(src + iVar2,src,iVar5);
        }
        iVar5 = *(int *)((int)&(((this->m_Paintmaps).m_Memory.m_pMemory)->m_Memory).m_pMemory +
                        local_30);
        iVar6 = 0;
        do {
                    /* Unresolved local var: int total@[???] */
          count = (*pRestore->_vptr_IRestore[0xf])(pRestore);
                    /* Unresolved local var: int length@[???]
                       Unresolved local var: uchar val@[???] */
          (*pRestore->_vptr_IRestore[0x12])(pRestore,local_20,1,0);
          _V_memset((void *)(iVar5 + iVar6),(uint)local_20[0],count);
          iVar6 = iVar6 + count;
        } while (iVar2 != iVar6);
      }
      (*pRestore->_vptr_IRestore[8])(pRestore);
      i = i + 1;
      local_30 = local_30 + 0x14;
    } while (i != iVar1);
  }
  local_2c = 0;
  local_28 = 0;
  local_24 = (undefined4 *)0x0;
  (*pRestore->_vptr_IRestore[7])(pRestore);
  iVar2 = (*pRestore->_vptr_IRestore[0xf])(pRestore);
  iVar1 = iVar2 + 0x1f;
  if (iVar2 + 0x1f < 0) {
    iVar1 = iVar2 + 0x3e;
  }
  uVar3 = iVar1 >> 5;
  if (uVar3 != local_2c._2_2_) {
    if (local_24 == (undefined4 *)0x0) {
      if (uVar3 != 0) {
        if (uVar3 == 1) {
          local_24 = &local_28;
        }
        else {
          local_24 = (undefined4 *)
                     (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x51317e))
                               ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x51317e),uVar3 << 2);
        }
      }
    }
    else if (uVar3 == 0) {
      if (1 < local_2c._2_2_) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51317e) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x51317e),local_24);
      }
      local_24 = (undefined4 *)0x0;
    }
    else if (local_24 == &local_28) {
      if (uVar3 != 1) {
        local_24 = (undefined4 *)
                   (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x51317e))
                             ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x51317e),uVar3 << 2);
        *local_24 = local_28;
      }
    }
    else if (uVar3 == 1) {
      local_28 = *local_24;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51317e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x51317e),local_24);
      local_24 = &local_28;
    }
    else {
      local_24 = (undefined4 *)
                 (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51317e) + 4))
                           ((int *)**(undefined4 **)(unaff_EBX + 0x51317e),local_24,uVar3 << 2);
    }
    local_2c = uVar3 << 0x10;
  }
  puVar7 = local_24;
  if (local_24 != (undefined4 *)0x0) {
    uVar4 = uVar3 * 4;
    if ((7 < uVar4) && (((uint)local_24 & 4) != 0)) {
      *local_24 = 0;
      puVar7 = puVar7 + 1;
      uVar4 = uVar4 - 4;
    }
    for (uVar4 = uVar4 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
      *puVar7 = 0;
      puVar7 = puVar7 + 1;
    }
  }
  local_2c = CONCAT22(local_2c._2_2_,(short)iVar2);
  uVar4 = uVar3;
  if ((int)(uint)local_2c._2_2_ < (int)uVar3) {
    uVar4 = (uint)local_2c._2_2_;
  }
  (*pRestore->_vptr_IRestore[0xe])(pRestore,local_24,uVar3,0);
                    /* Unresolved local var: int ignored@[???] */
  for (iVar1 = uVar3 - uVar4; 0 < iVar1; iVar1 = iVar1 + -1) {
    (*pRestore->_vptr_IRestore[0xe])(pRestore,local_20,1,0);
  }
  (*pRestore->_vptr_IRestore[8])(pRestore);
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5131a2) + 0x22c))
            ((int *)**(undefined4 **)(unaff_EBX + 0x5131a2),&local_2c);
  if (1 < local_2c._2_2_) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51317e) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x51317e),local_24);
  }
  return;
}


/* CPaintDatabase::SendPaintDataTo at 00733dc0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void SendPaintDataTo(CPaintDatabase * this, CBasePlayer * pPlayer) */

void __thiscall CPaintDatabase::SendPaintDataTo(CPaintDatabase *this,CBasePlayer *pPlayer)

{
  float *src;
  uchar uVar1;
  undefined4 *puVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int unaff_EBX;
  uint uVar6;
  int iVar7;
  int *piVar8;
  int iVar9;
  uchar *puVar10;
  int local_b8;
  int local_b0;
  int local_ac;
  int local_a8;
  int local_98;
  int local_94;
  int local_90;
  uchar *local_8c;
  float local_88;
  int local_84;
  int local_80;
  CRecipientFilter local_70;
  CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> local_50;
  int local_3c;
  undefined4 local_38;
  int local_34;
  int local_30;
  int local_2c;
  undefined4 local_28;
  undefined4 local_24;
  uchar *local_20;
  
  ___i686_get_pc_thunk_bx();
  if (pPlayer->m_iConnected != PlayerDisconnected) {
    local_3c = 0;
    local_38 = 0;
    local_34 = 0;
    local_30 = 0;
    local_2c = 0;
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x512daf) + 0x220))
              ((int *)**(undefined4 **)(unaff_EBX + 0x512daf),&local_3c);
    CRecipientFilter::CRecipientFilter(&local_70);
    local_70.super_IRecipientFilter._vptr_IRecipientFilter =
         (_func_int_varargs **)(unaff_EBX + 0x515317);
    CRecipientFilter::AddRecipient(&local_70,pPlayer);
    CRecipientFilter::MakeReliable(&local_70);
    iVar7 = local_30;
    local_8c = (uchar *)0x0;
    local_94 = 0;
    local_50.m_Memory.m_pMemory = (uchar *)0x0;
    local_50.m_Memory.m_nAllocationCount = 0;
    local_50.m_Memory.m_nGrowSize = 0;
    local_50.m_Size = 0;
    local_50.m_pElements = (uchar *)0x0;
    if (0 < local_30) {
      local_b8 = 0;
      local_90 = 0;
      local_84 = 0;
      puVar2 = *(undefined4 **)(unaff_EBX + 0x512d8b);
      do {
        iVar4 = *(int *)(local_84 + 0xc + local_3c);
        if (0 < iVar4) {
          local_b0 = 0;
          local_80 = 0;
          local_ac = 0;
          iVar5 = 0;
          do {
            iVar3 = *(int *)(local_84 + local_3c);
            uVar1 = *(uchar *)(iVar3 + iVar5);
            iVar9 = iVar5 + 1;
            if ((iVar9 < iVar4) && (*(uchar *)(iVar3 + iVar9) == uVar1)) {
              iVar3 = iVar3 + iVar5;
              local_a8 = 1;
              do {
                local_a8 = local_a8 + 1;
                iVar9 = iVar9 + 1;
                if (iVar4 <= iVar9) break;
                puVar10 = (uchar *)(iVar3 + 2);
                iVar3 = iVar3 + 1;
              } while (*puVar10 == uVar1);
              local_88 = (float)local_a8;
              iVar5 = iVar9;
            }
            else {
              local_a8 = 1;
              local_88 = 1.0;
              iVar5 = iVar9;
            }
            iVar9 = local_94 + 1;
            if (local_90 < iVar9) {
              if ((local_90 != 0) || (local_90 = 8, 8 < iVar9)) {
                do {
                  local_90 = local_90 << 1;
                } while (local_90 < iVar9);
              }
              if (local_8c == (uchar *)0x0) {
                local_8c = (uchar *)(*(code *)**(undefined4 **)
                                                **(undefined4 **)(unaff_EBX + 0x512d8b))
                                              ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x512d8b)
                                               ,local_90 << 2);
              }
              else {
                local_8c = (uchar *)(**(code **)(*(int *)*puVar2 + 4))
                                              ((int *)*puVar2,local_8c,local_90 << 2);
              }
            }
            iVar3 = (iVar9 - local_94) + -1;
            src = (float *)(local_8c + local_94 * 4);
            if (0 < iVar3) {
              _V_memmove(local_8c + iVar9 * 4,src,iVar3 * 4);
            }
            iVar3 = local_50.m_Size;
            if (src != (float *)0x0) {
              *src = local_88;
            }
            CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_>::GrowVector(&local_50,1);
            if (0 < ~iVar3 + local_50.m_Size) {
              _V_memmove(local_50.m_Memory.m_pMemory + iVar3 + 1,local_50.m_Memory.m_pMemory + iVar3
                         ,~iVar3 + local_50.m_Size);
            }
            if (local_50.m_Memory.m_pMemory + iVar3 != (uchar *)0x0) {
              local_50.m_Memory.m_pMemory[iVar3] = uVar1;
            }
            local_b0 = local_b0 + local_a8;
            local_80 = local_80 + 1;
            if ((local_80 == 0x32) || (local_94 = iVar9, iVar4 == local_b0)) {
              UserMessageBegin(&local_70.super_IRecipientFilter,&UNK_0032be94 + unaff_EBX);
              MessageWriteByte(local_b8);
              MessageWriteFloat((float)local_ac);
              MessageWriteFloat((float)local_80);
                    /* Unresolved local var: int m@[???] */
              if (0 < local_80) {
                iVar9 = 0;
                do {
                  MessageWriteFloat(*(float *)(local_8c + iVar9 * 4));
                  MessageWriteByte((uint)local_50.m_Memory.m_pMemory[iVar9]);
                  iVar9 = iVar9 + 1;
                } while (iVar9 != local_80);
              }
              MessageEnd();
              local_50.m_Size = 0;
              local_80 = 0;
              local_94 = 0;
              local_ac = local_b0;
            }
          } while (iVar5 < iVar4);
        }
        local_b8 = local_b8 + 1;
        local_84 = local_84 + 0x14;
      } while (local_b8 != iVar7);
    }
    local_28 = 0;
    local_24 = 0;
    local_20 = (uchar *)0x0;
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x512daf) + 0x228))
              ((int *)**(undefined4 **)(unaff_EBX + 0x512daf),&local_28);
    uVar6 = local_28 & 0xffff;
    puVar10 = (uchar *)(unaff_EBX + 0x37e69d);
    UserMessageBegin(&local_70.super_IRecipientFilter,(char *)puVar10);
    MessageWriteFloat((float)uVar6);
                    /* Unresolved local var: int i@[???] */
    if (local_28._2_2_ != 0) {
      iVar7 = 0;
      do {
        MessageWriteFloat((float)*(uint *)(local_20 + iVar7 * 4));
        iVar7 = iVar7 + 1;
      } while (iVar7 < (int)(local_28 >> 0x10));
    }
    MessageEnd();
    if (1 < local_28._2_2_) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x512d8b) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x512d8b));
      puVar10 = local_20;
    }
    local_20 = (uchar *)0x0;
    local_50.m_Size = 0;
    if (-1 < local_50.m_Memory.m_nGrowSize) {
      if (local_50.m_Memory.m_pMemory != (uchar *)0x0) {
        puVar10 = local_50.m_Memory.m_pMemory;
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x512d8b) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x512d8b));
        local_50.m_Memory.m_pMemory = (uchar *)0x0;
      }
      local_50.m_Memory.m_nAllocationCount = 0;
    }
    local_50.m_pElements = local_50.m_Memory.m_pMemory;
    if (-1 < local_50.m_Memory.m_nGrowSize) {
      if (local_50.m_Memory.m_pMemory != (uchar *)0x0) {
        puVar10 = local_50.m_Memory.m_pMemory;
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x512d8b) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x512d8b));
        local_50.m_Memory.m_pMemory = (uchar *)0x0;
      }
      local_50.m_Memory.m_nAllocationCount = 0;
    }
    if (local_8c != (uchar *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x512d8b) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x512d8b));
      puVar10 = local_8c;
    }
    local_70.super_IRecipientFilter._vptr_IRecipientFilter =
         (_func_int_varargs **)(unaff_EBX + 0x515317);
    CRecipientFilter::~CRecipientFilter(&local_70,(int)puVar10);
    local_98 = local_30;
    iVar7 = local_30 * 0x14;
    puVar2 = *(undefined4 **)(unaff_EBX + 0x512d8b);
    while( true ) {
      iVar7 = iVar7 + -0x14;
      local_98 = local_98 + -1;
      if (local_98 < 0) break;
      piVar8 = (int *)(iVar7 + local_3c);
      piVar8[3] = 0;
      if (piVar8[2] < 0) {
        iVar4 = *piVar8;
      }
      else {
        if (*piVar8 != 0) {
          (**(code **)(*(int *)*puVar2 + 8))((int *)*puVar2,*piVar8);
          *piVar8 = 0;
        }
        iVar4 = 0;
        piVar8[1] = 0;
      }
      piVar8[4] = iVar4;
      if (-1 < piVar8[2]) {
        if (iVar4 != 0) {
          (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x512d8b) + 8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x512d8b),iVar4);
          *piVar8 = 0;
        }
        piVar8[1] = 0;
      }
    }
    local_30 = 0;
    if (-1 < local_34) {
      if (local_3c != 0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x512d8b) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x512d8b),local_3c);
        local_3c = 0;
      }
      local_38 = 0;
    }
    if ((-1 < local_34) && (local_3c != 0)) {
      local_2c = local_3c;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x512d8b) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x512d8b),local_3c);
    }
  }
  return;
}


/* CPaintDatabase::SendPaintDataToEngine at 00732780 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void SendPaintDataToEngine(CPaintDatabase * this) */

void __thiscall CPaintDatabase::SendPaintDataToEngine(CPaintDatabase *this)

{
  undefined4 *puVar1;
  CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *pCVar2;
  int iVar3;
  int unaff_EBX;
  int iVar4;
  int *piVar5;
  int iVar6;
  int local_24;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  iVar6 = (this->m_Paintmaps).m_Size;
  if (0 < iVar6) {
    iVar4 = 0;
    local_20 = 0;
    do {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5143f2) + 0x224))
                ((int *)**(undefined4 **)(unaff_EBX + 0x5143f2),
                 (int)&(((this->m_Paintmaps).m_Memory.m_pMemory)->m_Memory).m_pMemory + local_20,
                 iVar4);
      iVar4 = iVar4 + 1;
      iVar6 = (this->m_Paintmaps).m_Size;
      local_20 = local_20 + 0x14;
    } while (iVar4 < iVar6);
  }
  puVar1 = *(undefined4 **)(unaff_EBX + 0x5143ce);
  iVar4 = iVar6 * 0x14;
  while( true ) {
    local_24 = iVar4 + -0x14;
    iVar6 = iVar6 + -1;
    if (iVar6 < 0) break;
    piVar5 = (int *)((int)&(this->m_Paintmaps).m_Memory.m_pMemory[-1].m_Memory.m_pMemory + iVar4);
    piVar5[3] = 0;
    if (piVar5[2] < 0) {
      iVar3 = *piVar5;
    }
    else {
      if (*piVar5 != 0) {
        (**(code **)(*(int *)*puVar1 + 8))((int *)*puVar1,*piVar5);
        *piVar5 = 0;
      }
      iVar3 = 0;
      piVar5[1] = 0;
    }
    piVar5[4] = iVar3;
    iVar4 = local_24;
    if (-1 < piVar5[2]) {
      if (iVar3 != 0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5143ce) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x5143ce),iVar3);
        *piVar5 = 0;
      }
      piVar5[1] = 0;
    }
  }
  (this->m_Paintmaps).m_Size = 0;
  if ((this->m_Paintmaps).m_Memory.m_nGrowSize < 0) {
    (this->m_Paintmaps).m_pElements = (this->m_Paintmaps).m_Memory.m_pMemory;
    return;
  }
  pCVar2 = (this->m_Paintmaps).m_Memory.m_pMemory;
  if (pCVar2 != (CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *)0x0) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5143ce) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x5143ce),pCVar2);
    (this->m_Paintmaps).m_Memory.m_pMemory =
         (CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *)0x0;
  }
  (this->m_Paintmaps).m_Memory.m_nAllocationCount = 0;
  (this->m_Paintmaps).m_pElements = (CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *)0x0
  ;
  return;
}


/* CC_PaintAllSurfaces at 00733760 */

void CC_PaintAllSurfaces(CCommand *args)

{
  uint uVar1;
  int unaff_EBX;
  char *messagename;
  uint local_40;
  CRecipientFilter local_3c;
  
                    /* Unresolved local var: PaintPowerType power@[???]
                       Unresolved local var: CBroadcastRecipientFilter filter@[???] */
  ___i686_get_pc_thunk_bx();
  if (args->m_nArgc == 2) {
    local_40 = _atoi(args->m_ppArgv[1]);
    uVar1 = local_40 & 0xff;
  }
  else {
    local_40 = 2;
    uVar1 = 2;
  }
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51340c) + 0x21c))
            ((int *)**(undefined4 **)(unaff_EBX + 0x51340c),uVar1);
  CRecipientFilter::CRecipientFilter(&local_3c);
  local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(unaff_EBX + 0x515a74);
  CRecipientFilter::AddAllPlayers(&local_3c);
  CRecipientFilter::MakeReliable(&local_3c);
  messagename = (char *)(unaff_EBX + 0x32c4e0);
  UserMessageBegin(&local_3c.super_IRecipientFilter,messagename);
  MessageWriteByte(local_40);
  MessageEnd();
  local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(unaff_EBX + 0x515a74);
  CRecipientFilter::~CRecipientFilter(&local_3c,(int)messagename);
  return;
}


/* __tcf_4 at 00a3dc10 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x40fbc8),in_stack_00000008);
  return;
}


/* _GLOBAL__I_PaintDatabase at 000adb50 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_PaintDatabase(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

