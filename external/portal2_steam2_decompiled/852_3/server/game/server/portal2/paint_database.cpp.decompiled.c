/* DWARF-guided pseudocode for game/server/portal2/paint_database.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000c2de0 */

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
  *(undefined1 *)((int)&sk_npc_dmg_pistol.super_ConCommandBase.m_pszName + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&sk_npc_dmg_pistol.super_ConCommandBase.m_pszHelpString + unaff_EBX) = 0;
  *(undefined1 *)((int)&sk_npc_dmg_pistol.super_ConCommandBase.m_pszHelpString + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&sk_npc_dmg_pistol.super_ConCommandBase.m_pszHelpString + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&sk_npc_dmg_pistol.super_ConCommandBase.m_pszHelpString + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&sk_npc_dmg_pistol.super_ConCommandBase.m_nFlags + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&sk_npc_dmg_pistol.super_IConVar._vptr_IConVar + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&sk_npc_dmg_pistol.m_pParent + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&sk_npc_dmg_pistol.m_pszDefaultValue + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&sk_npc_dmg_pistol.m_Value.m_pszString + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&sk_npc_dmg_pistol.m_Value.m_StringLength + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&sk_npc_dmg_pistol.m_Value.m_fValue + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&sk_npc_dmg_pistol.m_Value.m_nValue + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)(&sk_npc_dmg_pistol.field_0x37 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&sk_npc_dmg_pistol.m_fMinVal + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)(&sk_npc_dmg_pistol.field_0x3f + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&sk_npc_dmg_pistol.m_fMaxVal + unaff_EBX + 3) = 0;
  *(undefined1 *)
   ((int)&sk_npc_dmg_pistol.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX + 2) = 0;
  *(undefined1 *)((int)&sk_npc_dmg_pistol.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 3) =
       0;
  *(undefined1 *)
   ((int)&sk_npc_dmg_pistol.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) = 0;
  *(undefined1 *)
   ((int)&sk_npc_dmg_pistol.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX + 1) = 0;
  *(undefined4 *)
   ((int)&sk_npc_dmg_pistol.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&sk_npc_dmg_pistol.m_fnChangeCallbacks.m_Size + unaff_EBX + 2) = 0;
  *(undefined1 *)((int)&sk_npc_dmg_pistol.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 3)
       = 0;
  *(undefined1 *)((int)&sk_npc_dmg_pistol.m_fnChangeCallbacks.m_Size + unaff_EBX) = 0;
  *(undefined1 *)((int)&sk_npc_dmg_pistol.m_fnChangeCallbacks.m_Size + unaff_EBX + 1) = 0;
  *(undefined4 *)((int)&sk_npc_dmg_pistol.m_fnChangeCallbacks.m_Size + unaff_EBX + 3) = 0;
  (&DAT_00cf583a)[unaff_EBX] = 1;
  *(undefined1 *)((int)&sk_npc_dmg_pistol.m_fnChangeCallbacks.m_pElements + unaff_EBX + 3) = 0;
  (&DAT_00cf5838)[unaff_EBX] = 0;
  (&DAT_00cf5839)[unaff_EBX] = 0;
  *(undefined4 *)(&DAT_00cf583b + unaff_EBX) = 1;
  *(undefined1 *)((int)&sk_max_pistol.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX + 2) = 0
  ;
  (&DAT_00cf583f)[unaff_EBX] = 0;
  *(undefined1 *)((int)&sk_max_pistol.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) = 0;
  *(undefined1 *)((int)&sk_max_pistol.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX + 1) = 0
  ;
  *(undefined4 *)((int)&sk_max_pistol.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX + 3) = 2
  ;
  (&sk_max_pistol.super_ConCommandBase.field_0xa)[unaff_EBX] = 0;
  *(undefined1 *)((int)&sk_max_pistol.super_ConCommandBase.m_pNext + unaff_EBX + 3) = 0;
  (&sk_max_pistol.super_ConCommandBase.m_bRegistered)[unaff_EBX] = false;
  (&sk_max_pistol.super_ConCommandBase.field_0x9)[unaff_EBX] = 0;
  *(undefined **)(&sk_max_pistol.super_ConCommandBase.field_0xb + unaff_EBX) =
       &UNK_00aee537 + unaff_EBX;
  CPaintDatabase::CPaintDatabase
            ((CPaintDatabase *)((int)&PTR_IsBeingDragged_00c6c5ac + unaff_EBX + 3),
             (char *)(unaff_EBX + 0x94d6ea));
  uVar1 = *(undefined4 *)(&DAT_00ae8d9f + unaff_EBX);
  ___cxa_atexit(unaff_EBX + 0x8d7c2f,0,uVar1);
  ConVar::ConVar((ConVar *)((int)&PTR_InternalMouseWheeled_00c6c62c + unaff_EBX + 3),
                 &UNK_0094d6ff + unaff_EBX,(char *)(unaff_EBX + 0x94d6f9),0,in_stack_ffffffe4,
                 in_stack_ffffffe8,in_stack_ffffffec,in_stack_fffffff0);
  ___cxa_atexit(unaff_EBX + 0x8d7c0f,0,uVar1);
  ConCommand::ConCommand
            ((ConCommand *)((int)&sk_max_pistol.super_ConCommandBase.m_pszName + unaff_EBX + 3),
             (char *)(unaff_EBX + 0x94d728),(FnCommandCallbackV1_t)(unaff_EBX + 0x6094cf),
             (char *)0x0,0,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x8d7bef,0,uVar1);
  ConCommand::ConCommand
            ((ConCommand *)
             ((int)&sk_max_pistol.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 3),
             (char *)(unaff_EBX + 0x94d737),(FnCommandCallback_t)(unaff_EBX + 0x609ccf),(char *)0x0,
             0,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x8d7bcf,0,uVar1);
  ConCommand::ConCommand
            ((ConCommand *)((int)&sk_plr_dmg_smg1.m_Value.m_fValue + unaff_EBX + 3),
             (char *)(unaff_EBX + 0x94d73f),(FnCommandCallback_t)(unaff_EBX + 0x60927f),(char *)0x0,
             0,(FnCommandCompletionCallback)0x0);
  ___cxa_atexit(unaff_EBX + 0x8d7baf,0,uVar1);
  return;
}


/* __tcf_0 at 0099aa20 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  CPaintDatabase::~CPaintDatabase((CPaintDatabase *)(extraout_ECX + 0x394978),in_stack_00000008);
  return;
}


/* CPaintDatabase::CPaintDatabase at 000c2d00 */

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
  super_IGameSystem._vptr_IGameSystem = (_func_int_varargs **)(&UNK_00b7225b + unaff_EBX);
  (this->super_CGameEventListener).super_IGameEventListener2._vptr_IGameEventListener2 =
       (_func_int_varargs **)(&UNK_00b722ab + unaff_EBX);
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


/* CPaintDatabase::CPaintDatabase at 006cd030 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void CPaintDatabase(CPaintDatabase * this, char * name, CPaintDatabase
   * this, char * name) */

void __thiscall
CPaintDatabase::CPaintDatabase(CPaintDatabase *this,char *name,CPaintDatabase *this_1,char *name_1)

{
  CPaintDatabase(this,name);
  return;
}


/* CPaintDatabase::~CPaintDatabase at 006cd050 */

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
  super_IGameSystem._vptr_IGameSystem = (_func_int_varargs **)(&UNK_00567f0a + unaff_EBX);
  (this->super_CGameEventListener).super_IGameEventListener2._vptr_IGameEventListener2 =
       (_func_int_varargs **)(unaff_EBX + 0x567f5a);
  (this->m_PaintThisFrame).m_Size = 0;
  (this->m_PaintedEntities).m_Size = 0;
  (this->m_PaintedProjectedWalls).m_Size = 0;
  local_20 = (this->m_Paintmaps).m_Size;
  puVar1 = *(undefined4 **)(unaff_EBX + 0x4deb3a);
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
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4deb3a) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x4deb3a));
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
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4deb3a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4deb3a));
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
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4deb3a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4deb3a));
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
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4deb3a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4deb3a));
      pPVar3 = (PaintLocationData_t *)0x0;
      (this->m_PaintedProjectedWalls).m_Memory.m_pMemory = (CProjectedWallEntity **)0x0;
    }
    (this->m_PaintedProjectedWalls).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintedProjectedWalls).m_pElements = (CProjectedWallEntity **)pPVar3;
  if (-1 < (this->m_PaintedProjectedWalls).m_Memory.m_nGrowSize) {
    if (pPVar3 != (PaintLocationData_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4deb3a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4deb3a));
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
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4deb3a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4deb3a));
      pPVar3 = (PaintLocationData_t *)0x0;
      (this->m_PaintedEntities).m_Memory.m_pMemory = (PaintEntityData_t *)0x0;
    }
    (this->m_PaintedEntities).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintedEntities).m_pElements = (PaintEntityData_t *)pPVar3;
  if (-1 < (this->m_PaintedEntities).m_Memory.m_nGrowSize) {
    if (pPVar3 != (PaintLocationData_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4deb3a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4deb3a));
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
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4deb3a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4deb3a));
      pPVar3 = (PaintLocationData_t *)0x0;
      (this->m_PaintThisFrame).m_Memory.m_pMemory = (PaintLocationData_t *)0x0;
    }
    (this->m_PaintThisFrame).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintThisFrame).m_pElements = pPVar3;
  if (-1 < (this->m_PaintThisFrame).m_Memory.m_nGrowSize) {
    if (pPVar3 != (PaintLocationData_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4deb3a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4deb3a));
      (this->m_PaintThisFrame).m_Memory.m_pMemory = (PaintLocationData_t *)0x0;
      in_stack_ffffffc8 = pPVar3;
    }
    (this->m_PaintThisFrame).m_Memory.m_nAllocationCount = 0;
  }
  pPVar3 = (PaintLocationData_t *)&this->super_CGameEventListener;
  (this->super_CGameEventListener).super_IGameEventListener2._vptr_IGameEventListener2 =
       (_func_int_varargs **)(unaff_EBX + 0x4e084a);
  (this->super_CGameEventListener).m_nDebugID = 0xd;
  if ((this->super_CGameEventListener).m_bRegisteredForEvents != false) {
    piVar4 = (int *)**(int **)(unaff_EBX + 0x4deb62);
    if (piVar4 != (int *)0x0) {
      (**(code **)(*piVar4 + 0x18))(piVar4);
      in_stack_ffffffc8 = pPVar3;
    }
    (this->super_CGameEventListener).m_bRegisteredForEvents = false;
  }
  (this->super_CAutoGameSystemPerFrame).super_CBaseGameSystemPerFrame.super_IGameSystemPerFrame.
  super_IGameSystem._vptr_IGameSystem = (_func_int_varargs **)(&UNK_004e032a + unaff_EBX);
  IGameSystemPerFrame::~IGameSystemPerFrame((IGameSystemPerFrame *)this,(int)in_stack_ffffffc8);
  operator_delete(this);
  return;
}


/* CPaintDatabase::~CPaintDatabase at 006cd6e0 */

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
  super_IGameSystem._vptr_IGameSystem = (_func_int_varargs **)(unaff_EBX + 0x56787a);
  (this->super_CGameEventListener).super_IGameEventListener2._vptr_IGameEventListener2 =
       (_func_int_varargs **)(unaff_EBX + 0x5678ca);
  (this->m_PaintThisFrame).m_Size = 0;
  (this->m_PaintedEntities).m_Size = 0;
  (this->m_PaintedProjectedWalls).m_Size = 0;
  local_20 = (this->m_Paintmaps).m_Size;
  puVar1 = *(undefined4 **)(unaff_EBX + 0x4de4aa);
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
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4de4aa) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x4de4aa),iVar3);
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
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4de4aa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4de4aa),pCVar4);
      (this->m_Paintmaps).m_Memory.m_pMemory =
           (CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *)0x0;
    }
    pCVar4 = (CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *)0x0;
    (this->m_Paintmaps).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_Paintmaps).m_pElements = pCVar4;
  if (-1 < (this->m_Paintmaps).m_Memory.m_nGrowSize) {
    if (pCVar4 != (CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4de4aa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4de4aa),pCVar4);
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
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4de4aa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4de4aa),ppCVar5);
      (this->m_PaintedProjectedWalls).m_Memory.m_pMemory = (CProjectedWallEntity **)0x0;
    }
    ppCVar5 = (CProjectedWallEntity **)0x0;
    (this->m_PaintedProjectedWalls).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintedProjectedWalls).m_pElements = ppCVar5;
  if (-1 < (this->m_PaintedProjectedWalls).m_Memory.m_nGrowSize) {
    if (ppCVar5 != (CProjectedWallEntity **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4de4aa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4de4aa),ppCVar5);
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
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4de4aa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4de4aa),pPVar6);
      (this->m_PaintedEntities).m_Memory.m_pMemory = (PaintEntityData_t *)0x0;
    }
    pPVar6 = (PaintEntityData_t *)0x0;
    (this->m_PaintedEntities).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintedEntities).m_pElements = pPVar6;
  if (-1 < (this->m_PaintedEntities).m_Memory.m_nGrowSize) {
    if (pPVar6 != (PaintEntityData_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4de4aa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4de4aa),pPVar6);
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
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4de4aa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4de4aa),pPVar7);
      (this->m_PaintThisFrame).m_Memory.m_pMemory = (PaintLocationData_t *)0x0;
    }
    pPVar7 = (PaintLocationData_t *)0x0;
    (this->m_PaintThisFrame).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintThisFrame).m_pElements = pPVar7;
  if (-1 < (this->m_PaintThisFrame).m_Memory.m_nGrowSize) {
    if (pPVar7 != (PaintLocationData_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4de4aa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4de4aa),pPVar7);
      (this->m_PaintThisFrame).m_Memory.m_pMemory = (PaintLocationData_t *)0x0;
    }
    (this->m_PaintThisFrame).m_Memory.m_nAllocationCount = 0;
  }
  (this->super_CGameEventListener).super_IGameEventListener2._vptr_IGameEventListener2 =
       (_func_int_varargs **)(unaff_EBX + 0x4e01ba);
  (this->super_CGameEventListener).m_nDebugID = 0xd;
  if ((this->super_CGameEventListener).m_bRegisteredForEvents != false) {
    piVar8 = (int *)**(int **)(unaff_EBX + 0x4de4d2);
    if (piVar8 != (int *)0x0) {
      (**(code **)(*piVar8 + 0x18))(piVar8,&this->super_CGameEventListener);
    }
    (this->super_CGameEventListener).m_bRegisteredForEvents = false;
  }
  (this->super_CAutoGameSystemPerFrame).super_CBaseGameSystemPerFrame.super_IGameSystemPerFrame.
  super_IGameSystem._vptr_IGameSystem = (_func_int_varargs **)(unaff_EBX + 0x4dfc9a);
  IGameSystemPerFrame::~IGameSystemPerFrame((IGameSystemPerFrame *)this,__in_chrg);
  return;
}


/* CPaintDatabase::~CPaintDatabase at 006cdd70 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void ~CPaintDatabase(CPaintDatabase * this, int __in_chrg,
   CPaintDatabase * this) */

void __thiscall
CPaintDatabase::~CPaintDatabase(CPaintDatabase *this,int __in_chrg,CPaintDatabase *this_1)

{
  ~CPaintDatabase(this,__in_chrg);
  return;
}


/* CPaintDatabase::LevelInitPreEntity at 006cb940 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void LevelInitPreEntity(CPaintDatabase * this) */

void __thiscall CPaintDatabase::LevelInitPreEntity(CPaintDatabase *this)

{
  (this->m_PaintThisFrame).m_Size = 0;
  (this->m_PaintedEntities).m_Size = 0;
  (this->m_PaintedProjectedWalls).m_Size = 0;
  return;
}


/* CPaintDatabase::LevelInitPostEntity at 006cbbd0 */

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
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dffda) + 0x224))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4dffda),
                 (int)&(((this->m_Paintmaps).m_Memory.m_pMemory)->m_Memory).m_pMemory + local_20,
                 iVar4);
      iVar4 = iVar4 + 1;
      iVar6 = (this->m_Paintmaps).m_Size;
      local_20 = local_20 + 0x14;
    } while (iVar4 < iVar6);
  }
  puVar1 = *(undefined4 **)(unaff_EBX + 0x4dffba);
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
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dffba) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x4dffba),iVar3);
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
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dffba) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4dffba),pCVar2);
    (this->m_Paintmaps).m_Memory.m_pMemory =
         (CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *)0x0;
  }
  (this->m_Paintmaps).m_Memory.m_nAllocationCount = 0;
  (this->m_Paintmaps).m_pElements = (CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *)0x0
  ;
  return;
}


/* CPaintDatabase::LevelShutdownPostEntity at 006cb6a0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void LevelShutdownPostEntity(CPaintDatabase * this) */

void __thiscall CPaintDatabase::LevelShutdownPostEntity(CPaintDatabase *this)

{
  this->m_bCanPaint = false;
  return;
}


/* CPaintDatabase::AddPaint at 006cc890 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void AddPaint(CPaintDatabase * this, trace_t * tr, PaintPowerType
   powerType) */

void __thiscall CPaintDatabase::AddPaint(CPaintDatabase *this,trace_t *tr,PaintPowerType powerType)

{
  float fVar1;
  float fVar2;
  float fVar3;
  CBaseEntity *this_00;
  uint uVar4;
  bool bVar5;
  CProjectedWallEntity *pWall;
  PaintLocationData_t *pPVar6;
  int iVar7;
  int unaff_EBX;
  float fVar8;
  float fVar9;
  float fVar10;
  Vector local_28 [2];
  
                    /* Unresolved local var: CBaseEntity * pPaintedEntity@[???]
                       Unresolved local var: Vector vecPaintLocation@[???]
                       Unresolved local var: Vector vecNormal@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = tr->m_pEnt;
  local_28[0].x = (tr->super_CBaseTrace).endpos.x;
  local_28[0].y = (tr->super_CBaseTrace).endpos.y;
  local_28[0].z = (tr->super_CBaseTrace).endpos.z;
  fVar1 = (tr->super_CBaseTrace).plane.normal.z;
  fVar2 = (tr->super_CBaseTrace).plane.normal.y;
  fVar3 = (tr->super_CBaseTrace).plane.normal.x;
  bVar5 = CBaseEntity::IsBSPModel(this_00);
  if ((bVar5) && (bVar5 = UTIL_IsPaintableSurface(&tr->surface), bVar5)) {
                    /* Unresolved local var: PaintLocationData_t data@[???]
                       Unresolved local var: Vector res@[???] */
    fVar10 = *(float *)(unaff_EBX + 0x35da70);
                    /* Unresolved local var: Vector res@[???] */
    fVar8 = local_28[0].x + fVar3 * fVar10;
    fVar9 = local_28[0].y + fVar2 * fVar10;
    fVar10 = local_28[0].z + fVar10 * fVar1;
    uVar4 = (this->m_PaintThisFrame).m_Size;
    CUtlVector<PaintLocationData_t,CUtlMemory<PaintLocationData_t,_int>_>::GrowVector
              (&this->m_PaintThisFrame,1);
    iVar7 = ~uVar4 + (this->m_PaintThisFrame).m_Size;
    if (0 < iVar7) {
      pPVar6 = (this->m_PaintThisFrame).m_Memory.m_pMemory;
      _V_memmove(pPVar6 + uVar4 + 1,pPVar6 + uVar4,iVar7 * 0x20);
    }
    pPVar6 = (this->m_PaintThisFrame).m_Memory.m_pMemory + uVar4;
    if (pPVar6 != (PaintLocationData_t *)0x0) {
      (pPVar6->normal).z = fVar1;
      (pPVar6->normal).y = fVar2;
      (pPVar6->normal).x = fVar3;
      pPVar6->type = powerType;
      (pPVar6->location).z = fVar10;
      (pPVar6->location).y = fVar9;
      (pPVar6->location).x = fVar8;
      pPVar6->pBrushEntity = this_00;
    }
  }
  else if (-1 < (this_00->m_fFlags).m_Value) {
    if ((this_00 == (CBaseEntity *)0x0) ||
       (pWall = (CProjectedWallEntity *)
                ___dynamic_cast(this_00,*(undefined4 *)(CFuncTank::Spawn + unaff_EBX + 4),
                                *(undefined4 *)(unaff_EBX + 0x4df670),0),
       pWall == (CProjectedWallEntity *)0x0)) {
      PaintEntity((CPaintDatabase *)(unaff_EBX + 0x662afc),this_00,powerType,local_28);
    }
    else {
      PaintProjectedWall((CPaintDatabase *)(unaff_EBX + 0x662afc),pWall,powerType,local_28);
    }
  }
  return;
}


/* CPaintDatabase::PaintEntity at 006cc2e0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void PaintEntity(CPaintDatabase * this, CBaseEntity * pPaintedEntity,
   PaintPowerType newPowerType, Vector * vecPosition) */

void __thiscall
CPaintDatabase::PaintEntity
          (CPaintDatabase *this,CBaseEntity *pPaintedEntity,PaintPowerType newPowerType,
          Vector *vecPosition)

{
  int *piVar1;
  undefined1 *puVar2;
  IPhysicsObject *pIVar3;
  uint uVar4;
  CBaseEdict *pCVar5;
  uint8 uVar6;
  undefined1 uVar7;
  undefined1 uVar8;
  undefined1 uVar9;
  char cVar10;
  uint8 uVar11;
  PaintPowerType PVar12;
  int iVar13;
  int iVar14;
  ulong *puVar15;
  PaintEntityData_t *pPVar16;
  Color *pCVar17;
  IChangeInfoAccessor *pIVar18;
  CBaseEntity *pCVar19;
  int unaff_EBX;
  Vector *pVVar20;
  Vector *pVVar21;
  int iVar22;
  Vector *index;
  Vector *in_stack_ffffff58;
  int *local_7c;
  ulong local_70;
  Vector *local_40;
  CRecipientFilter local_3c;
  
                    /* Unresolved local var: IPaintableEntity * pPaintableEntity@[???]
                       Unresolved local var: PaintPowerType paintedPowerType@[???]
                       Unresolved local var: bool bPainted@[???] */
  ___i686_get_pc_thunk_bx();
  if (pPaintedEntity == (CBaseEntity *)0x0) {
    local_7c = (int *)0x0;
    local_40 = in_stack_ffffff58;
LAB_006cc593:
                    /* Unresolved local var: color24 otherColor@[???]
                       Unresolved local var: Color paintedColor@[???]
                       Unresolved local var: color24 c@[???] */
    uVar7 = (pPaintedEntity->m_clrRender).
            super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r;
    uVar8 = (pPaintedEntity->m_clrRender).
            super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g;
    uVar9 = (pPaintedEntity->m_clrRender).
            super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b;
    local_3c.super_IRecipientFilter._vptr_IRecipientFilter._2_1_ = uVar9;
    local_3c.super_IRecipientFilter._vptr_IRecipientFilter._1_1_ = uVar8;
    local_3c.super_IRecipientFilter._vptr_IRecipientFilter._0_1_ = uVar7;
    local_3c.super_IRecipientFilter._vptr_IRecipientFilter._3_1_ = 0;
    PVar12 = MapColorToPower((Color *)&local_3c);
  }
  else {
    local_40 = *(Vector **)(unaff_EBX + 0x4df8d7);
    local_7c = (int *)___dynamic_cast(pPaintedEntity,local_40,unaff_EBX + 0x557fef,0xfffffffe);
    if (local_7c == (int *)0x0) goto LAB_006cc593;
    local_40 = vecPosition;
    PVar12 = (**(code **)(*local_7c + 8))(local_7c);
    iVar13 = (*(pPaintedEntity->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
               _vptr_IHandleEntity[0x55])(pPaintedEntity);
    if ((char)iVar13 != '\0') {
      CRecipientFilter::CRecipientFilter(&local_3c);
      local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
           (_func_int_varargs **)(unaff_EBX + 0x4e1f17);
      CRecipientFilter::AddAllPlayers(&local_3c);
      CRecipientFilter::MakeReliable(&local_3c);
      local_40 = (Vector *)(unaff_EBX + 0x344189);
      UserMessageBegin(&local_3c.super_IRecipientFilter,(char *)local_40);
      MessageWriteEHandle(pPaintedEntity);
      MessageWriteByte(newPowerType);
      MessageWriteFloat(vecPosition->x);
      MessageWriteFloat(vecPosition->y);
      MessageWriteFloat(vecPosition->z);
      MessageEnd();
      local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
           (_func_int_varargs **)(unaff_EBX + 0x4e1f17);
      CRecipientFilter::~CRecipientFilter(&local_3c,(int)local_40);
    }
  }
  if (PVar12 != NO_POWER) {
    if ((newPowerType == NO_POWER) &&
       (pVVar20 = (Vector *)(this->m_PaintedEntities).m_Size, 0 < (int)pVVar20)) {
      index = (Vector *)0x0;
      do {
        uVar4 = (this->m_PaintedEntities).m_Memory.m_pMemory[(int)index].hEnt.m_Index;
        if ((uVar4 == 0xffffffff) ||
           (iVar13 = (uVar4 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4df8b3),
           *(uint *)(iVar13 + 8) != uVar4 >> 0x10)) {
          pVVar21 = (Vector *)0x0;
        }
        else {
          pVVar21 = *(Vector **)(iVar13 + 4);
        }
        local_40 = pVVar21;
        cVar10 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4df91f) + 8))
                           ((int *)**(undefined4 **)(unaff_EBX + 0x4df91f));
        pCVar19 = (CBaseEntity *)0x0;
        if (cVar10 == '\0') {
          pCVar19 = (CBaseEntity *)(**(code **)((int)pVVar21->x + 0x18))(pVVar21);
        }
        if (pCVar19 == pPaintedEntity) {
          RemovePaintedEntity(this,(int)index,true);
          local_40 = index;
          break;
        }
        index = (Vector *)((int)&index->x + 1);
      } while (index != pVVar20);
    }
    goto LAB_006cc370;
  }
  if (newPowerType == NO_POWER) goto LAB_006cc370;
  iVar13 = (this->m_PaintedEntities).m_Size;
  if (0 < iVar13) {
    iVar22 = 0;
    do {
      uVar4 = (this->m_PaintedEntities).m_Memory.m_pMemory[iVar22].hEnt.m_Index;
      if ((uVar4 == 0xffffffff) ||
         (iVar14 = (uVar4 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4df8b3),
         *(uint *)(iVar14 + 8) != uVar4 >> 0x10)) {
        pVVar20 = (Vector *)0x0;
      }
      else {
        pVVar20 = *(Vector **)(iVar14 + 4);
      }
      local_40 = pVVar20;
      cVar10 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4df91f) + 8))
                         ((int *)**(undefined4 **)(unaff_EBX + 0x4df91f));
      pCVar19 = (CBaseEntity *)0x0;
      if (cVar10 == '\0') {
        pCVar19 = (CBaseEntity *)(**(code **)((int)pVVar20->x + 0x18))(pVVar20);
      }
      if (pCVar19 == pPaintedEntity) goto LAB_006cc370;
      iVar22 = iVar22 + 1;
    } while (iVar22 != iVar13);
  }
  local_70 = 0xffffffff;
  if (pPaintedEntity != (CBaseEntity *)0x0) {
    puVar15 = (ulong *)(*(pPaintedEntity->super_IServerEntity).super_IServerUnknown.
                         super_IHandleEntity._vptr_IHandleEntity[3])(pPaintedEntity);
    local_70 = *puVar15;
  }
  uVar4 = (this->m_PaintedEntities).m_Size;
  iVar13 = uVar4 + 1;
  iVar22 = (this->m_PaintedEntities).m_Memory.m_nAllocationCount;
  if (iVar22 < iVar13) {
    iVar14 = (this->m_PaintedEntities).m_Memory.m_nGrowSize;
    if (-1 < iVar14) {
      if (iVar14 == 0) {
        if ((iVar22 == 0) && (iVar22 = 8, iVar13 < 9)) {
          local_40 = (Vector *)&segment_command_0000001c.cmdsize;
        }
        else {
          do {
            iVar14 = iVar22;
            iVar22 = iVar14 * 2;
          } while (iVar22 < iVar13);
          local_40 = (Vector *)(iVar14 * 8);
        }
        goto LAB_006cc511;
      }
      iVar22 = ((int)uVar4 / iVar14 + 1) * iVar14;
      if (iVar22 < iVar13) {
        if (iVar22 != 0) {
LAB_006cc4a6:
          do {
            iVar22 = (iVar22 + iVar13) / 2;
          } while (iVar22 < iVar13);
          goto LAB_006cc4b6;
        }
        if (iVar13 < 0) {
          iVar22 = -1;
          local_40 = (Vector *)0xfffffffc;
        }
        else {
          local_40 = (Vector *)0x0;
          if (iVar13 != 0) goto LAB_006cc4a6;
        }
      }
      else {
LAB_006cc4b6:
        local_40 = (Vector *)(iVar22 * 4);
      }
LAB_006cc511:
      (this->m_PaintedEntities).m_Memory.m_nAllocationCount = iVar22;
      pVVar20 = (Vector *)(this->m_PaintedEntities).m_Memory.m_pMemory;
      if (pVVar20 == (Vector *)0x0) {
        pPVar16 = (PaintEntityData_t *)
                  (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x4df8a7))
                            ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x4df8a7));
        (this->m_PaintedEntities).m_Memory.m_pMemory = pPVar16;
      }
      else {
        pPVar16 = (PaintEntityData_t *)
                  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4df8a7) + 4))
                            ((int *)**(undefined4 **)(unaff_EBX + 0x4df8a7),pVVar20,local_40);
        (this->m_PaintedEntities).m_Memory.m_pMemory = pPVar16;
        local_40 = pVVar20;
      }
    }
  }
  piVar1 = &(this->m_PaintedEntities).m_Size;
  *piVar1 = *piVar1 + 1;
  pPVar16 = (this->m_PaintedEntities).m_Memory.m_pMemory;
  (this->m_PaintedEntities).m_pElements = pPVar16;
  iVar22 = ~uVar4 + (this->m_PaintedEntities).m_Size;
  if (0 < iVar22) {
    local_40 = (Vector *)(pPVar16 + uVar4);
    _V_memmove(pPVar16 + iVar13,local_40,iVar22 * 4);
  }
  pPVar16 = (this->m_PaintedEntities).m_Memory.m_pMemory + uVar4;
  if (pPVar16 != (PaintEntityData_t *)0x0) {
    (pPVar16->hEnt).m_Index = local_70;
  }
LAB_006cc370:
  if (local_7c == (int *)0x0) {
    pCVar17 = MapPowerToVisualColor((Color *)newPowerType,(int)local_40);
    uVar6 = (uint8)((uint)pCVar17 >> 8);
    uVar11 = (uint8)((uint)pCVar17 >> 0x10);
    if ((uint8)pCVar17 !=
        (pPaintedEntity->m_clrRender).
        super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r) {
      if ((pPaintedEntity->m_Network).m_TimerEvent.m_bRegistered == false) {
        pCVar5 = &((pPaintedEntity->m_Network).m_pPev)->super_CBaseEdict;
        if (pCVar5 != (CBaseEdict *)0x0) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
          pIVar18 = CBaseEdict::GetChangeAccessor(pCVar5);
          pIVar18->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar2 = &(pPaintedEntity->m_Network).field_0x4c;
        *(uint *)puVar2 = *(uint *)puVar2 | 1;
      }
      (pPaintedEntity->m_clrRender).
      super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r =
           (uint8)pCVar17;
    }
    if (uVar6 != (pPaintedEntity->m_clrRender).
                 super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g) {
      if ((pPaintedEntity->m_Network).m_TimerEvent.m_bRegistered == false) {
        pCVar5 = &((pPaintedEntity->m_Network).m_pPev)->super_CBaseEdict;
        if (pCVar5 != (CBaseEdict *)0x0) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
          pIVar18 = CBaseEdict::GetChangeAccessor(pCVar5);
          pIVar18->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar2 = &(pPaintedEntity->m_Network).field_0x4c;
        *(uint *)puVar2 = *(uint *)puVar2 | 1;
      }
      (pPaintedEntity->m_clrRender).
      super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g = uVar6;
    }
    if (uVar11 != (pPaintedEntity->m_clrRender).
                  super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b) {
      if ((pPaintedEntity->m_Network).m_TimerEvent.m_bRegistered == false) {
        pCVar5 = &((pPaintedEntity->m_Network).m_pPev)->super_CBaseEdict;
        if (pCVar5 != (CBaseEdict *)0x0) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
          pIVar18 = CBaseEdict::GetChangeAccessor(pCVar5);
          pIVar18->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar2 = &(pPaintedEntity->m_Network).field_0x4c;
        *(uint *)puVar2 = *(uint *)puVar2 | 1;
      }
      (pPaintedEntity->m_clrRender).
      super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b = uVar11;
    }
  }
  else {
    pIVar3 = pPaintedEntity->m_pPhysicsObject;
    if ((pIVar3 != (IPhysicsObject *)0x0) &&
       (iVar13 = (*pIVar3->_vptr_IPhysicsObject[3])(pIVar3), (char)iVar13 != '\0')) {
      (*pIVar3->_vptr_IPhysicsObject[0x19])(pIVar3);
    }
    (**(code **)(*local_7c + 0xc))(local_7c,newPowerType,vecPosition);
  }
  return;
}


/* CPaintDatabase::PaintProjectedWall at 006cb770 */

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
  if (iVar1 <= iVar7) goto LAB_006cb7fe;
  iVar9 = (this->m_PaintedProjectedWalls).m_Memory.m_nGrowSize;
  if (iVar9 < 0) goto LAB_006cb7fe;
  if (iVar9 == 0) {
    if (iVar7 == 0) {
      iVar7 = 8;
      iVar9 = 0x20;
      if (iVar1 < 9) goto LAB_006cb888;
    }
    do {
      iVar7 = iVar7 * 2;
    } while (iVar7 < iVar1);
LAB_006cb881:
    iVar9 = iVar7 * 4;
  }
  else {
    iVar7 = ((int)uVar4 / iVar9 + 1) * iVar9;
    if (iVar1 <= iVar7) goto LAB_006cb881;
    if (iVar7 != 0) {
LAB_006cb86e:
      do {
        iVar7 = (iVar7 + iVar1) / 2;
      } while (iVar7 < iVar1);
      goto LAB_006cb881;
    }
    if (iVar1 < 0) {
      iVar7 = -1;
      iVar9 = -4;
    }
    else {
      iVar9 = 0;
      if (iVar1 != 0) goto LAB_006cb86e;
    }
  }
LAB_006cb888:
  (this->m_PaintedProjectedWalls).m_Memory.m_nAllocationCount = iVar7;
  if (ppCVar8 == (CProjectedWallEntity **)0x0) {
    ppCVar8 = (CProjectedWallEntity **)
              (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x4e041a))
                        ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x4e041a),iVar9);
    (this->m_PaintedProjectedWalls).m_Memory.m_pMemory = ppCVar8;
  }
  else {
    ppCVar8 = (CProjectedWallEntity **)
              (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e041a) + 4))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x4e041a),ppCVar8,iVar9);
    (this->m_PaintedProjectedWalls).m_Memory.m_pMemory = ppCVar8;
  }
LAB_006cb7fe:
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


/* CPaintDatabase::RemoveAllPaint at 006cc160 */

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
         (iVar4 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4dfa36),
         *(uint *)(iVar4 + 8) != uVar2 >> 0x10)) {
        piVar5 = (int *)0x0;
      }
      else {
        piVar5 = *(int **)(iVar4 + 4);
      }
      cVar3 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dfaa2) + 8))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x4dfaa2),piVar5);
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
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dfa4a) + 0x218))
            ((int *)**(undefined4 **)(unaff_EBX + 0x4dfa4a));
  CRecipientFilter::CRecipientFilter(&local_3c);
  local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(unaff_EBX + 0x4e209a);
  CRecipientFilter::AddAllPlayers(&local_3c);
  CRecipientFilter::MakeReliable(&local_3c);
  messagename = (char *)(unaff_EBX + 0x2f08ac);
  UserMessageBegin(&local_3c.super_IRecipientFilter,messagename);
  MessageEnd();
  local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(unaff_EBX + 0x4e209a);
  CRecipientFilter::~CRecipientFilter(&local_3c,(int)messagename);
  return;
}


/* CPaintDatabase::RemovePaintedEntity at 006cbe30 */

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
         (iVar4 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4dfd66),
         *(uint *)(iVar4 + 8) != uVar2 >> 0x10)) {
        piVar6 = (int *)0x0;
      }
      else {
        piVar6 = *(int **)(iVar4 + 4);
      }
      cVar3 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dfdd2) + 8))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x4dfdd2),piVar6);
      if (cVar3 == '\0') {
        pCVar5 = (CBaseEntity *)(**(code **)(*piVar6 + 0x18))(piVar6);
        if (pCVar5 == pPaintedEntity) goto LAB_006cbecf;
      }
      else if (pPaintedEntity == (CBaseEntity *)0x0) {
LAB_006cbecf:
        RemovePaintedEntity(this,index,true);
        return;
      }
      index = index + 1;
    } while (index != iVar1);
  }
  return;
}


/* CPaintDatabase::RemovePaintedEntity at 006cbd60 */

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
         (iVar4 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4dfe36),
         *(uint *)(iVar4 + 8) != uVar2 >> 0x10)) {
        piVar6 = (int *)0x0;
      }
      else {
        piVar6 = *(int **)(iVar4 + 4);
      }
      cVar3 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dfea2) + 8))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x4dfea2),piVar6);
      if (cVar3 == '\0') {
        pCVar5 = (CBaseEntity *)(**(code **)(*piVar6 + 0x18))(piVar6);
        if (pCVar5 == pPaintedEntity) goto LAB_006cbe0f;
      }
      else if (pPaintedEntity == (CBaseEntity *)0x0) {
LAB_006cbe0f:
        RemovePaintedEntity(this,index,bDeleteData);
        return;
      }
      index = index + 1;
    } while (index != iVar1);
  }
  return;
}


/* CPaintDatabase::RemovePaintedEntity at 006cba00 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void RemovePaintedEntity(CPaintDatabase * this, int index, bool
   bDeleteData) */

void __thiscall CPaintDatabase::RemovePaintedEntity(CPaintDatabase *this,int index,bool bDeleteData)

{
  uint uVar1;
  PaintEntityData_t *pPVar2;
  CBaseEdict *pCVar3;
  char cVar4;
  int iVar5;
  IChangeInfoAccessor *pIVar6;
  int unaff_EBX;
  int *piVar7;
  
                    /* Unresolved local var: CBaseEntity * pEnt@[???] */
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->m_PaintedEntities).m_Memory.m_pMemory[index].hEnt.m_Index;
  if ((uVar1 == 0xffffffff) ||
     (iVar5 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e0190),
     *(uint *)(iVar5 + 8) != uVar1 >> 0x10)) {
    piVar7 = (int *)0x0;
  }
  else {
    piVar7 = *(int **)(iVar5 + 4);
  }
  cVar4 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e01fc) + 8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x4e01fc),piVar7);
  if (cVar4 == '\0') {
    iVar5 = (**(code **)(*piVar7 + 0x18))(piVar7);
    if (iVar5 != 0) {
                    /* Unresolved local var: IPaintableEntity * pPaintableEntity@[???] */
      piVar7 = (int *)___dynamic_cast(iVar5,*(undefined4 *)(unaff_EBX + 0x4e01b4),
                                      unaff_EBX + 0x5588cc,0xfffffffe);
      if (piVar7 == (int *)0x0) {
        if (*(char *)(iVar5 + 0x108) != -1) {
          if (*(char *)(iVar5 + 0x60) == '\0') {
            pCVar3 = *(CBaseEdict **)(iVar5 + 0x24);
            if (pCVar3 != (CBaseEdict *)0x0) {
              pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
              pIVar6 = CBaseEdict::GetChangeAccessor(pCVar3);
              pIVar6->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            *(uint *)(iVar5 + 100) = *(uint *)(iVar5 + 100) | 1;
          }
          *(undefined1 *)(iVar5 + 0x108) = 0xff;
        }
        if (*(char *)(iVar5 + 0x109) != -1) {
          if (*(char *)(iVar5 + 0x60) == '\0') {
            pCVar3 = *(CBaseEdict **)(iVar5 + 0x24);
            if (pCVar3 != (CBaseEdict *)0x0) {
              pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
              pIVar6 = CBaseEdict::GetChangeAccessor(pCVar3);
              pIVar6->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            *(uint *)(iVar5 + 100) = *(uint *)(iVar5 + 100) | 1;
          }
          *(undefined1 *)(iVar5 + 0x109) = 0xff;
        }
        if (*(char *)(iVar5 + 0x10a) != -1) {
          if (*(char *)(iVar5 + 0x60) == '\0') {
            pCVar3 = *(CBaseEdict **)(iVar5 + 0x24);
            if (pCVar3 != (CBaseEdict *)0x0) {
              pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
              pIVar6 = CBaseEdict::GetChangeAccessor(pCVar3);
              pIVar6->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            *(uint *)(iVar5 + 100) = *(uint *)(iVar5 + 100) | 1;
          }
          *(undefined1 *)(iVar5 + 0x10a) = 0xff;
        }
      }
      else {
        (**(code **)(*piVar7 + 0x10))(piVar7);
      }
    }
  }
  if (bDeleteData) {
    iVar5 = ~index + (this->m_PaintedEntities).m_Size;
    if (0 < iVar5) {
      pPVar2 = (this->m_PaintedEntities).m_Memory.m_pMemory;
      _V_memmove(pPVar2 + index,pPVar2 + index + 1,iVar5 * 4);
    }
    piVar7 = &(this->m_PaintedEntities).m_Size;
    *piVar7 = *piVar7 + -1;
  }
  return;
}


/* CPaintDatabase::RemovePaintedWall at 006cb6f0 */

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


/* CPaintDatabase::FindPaintedEntity at 006cb960 */

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
         (iVar4 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e0236),
         *(uint *)(iVar4 + 8) != uVar2 >> 0x10)) {
        piVar6 = (int *)0x0;
      }
      else {
        piVar6 = *(int **)(iVar4 + 4);
      }
      cVar3 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e02a2) + 8))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x4e02a2),piVar6);
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


/* CPaintDatabase::FireGameEvent at 006cb6c0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void FireGameEvent(CPaintDatabase * this, IGameEvent * event) */

void __thiscall CPaintDatabase::FireGameEvent(CPaintDatabase *this,IGameEvent *event)

{
  return;
}


/* CPaintDatabase::ClearPaintData at 006cb6d0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void ClearPaintData(CPaintDatabase * this) */

void __thiscall CPaintDatabase::ClearPaintData(CPaintDatabase *this)

{
  (this->m_PaintThisFrame).m_Size = 0;
  (this->m_PaintedEntities).m_Size = 0;
  (this->m_PaintedProjectedWalls).m_Size = 0;
  return;
}


/* AscendingPaintLocationCompare at 006cff90 */

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


/* __tcf_1 at 0099aa00 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x394a18),in_stack_00000008);
  return;
}


/* CPaintDatabase::PreClientUpdate at 006ce720 */

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
  size_t sVar9;
  PaintPowerType PVar10;
  CBaseEntity *pCVar11;
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
  int local_114;
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
  if (((*(int *)(*(int *)(unaff_EBX + 0x4dd48b) + 0x100c) == 0) &&
      (*(char *)(*(int *)(unaff_EBX + 0x4dd48b) + 0x1010) != '\0')) ||
     (iVar25 = *(int *)(*(int *)(unaff_EBX + 0x4dd48b) + 0x19b8), iVar26 = _ThreadGetCurrentId(),
     iVar25 != iVar26)) {
    cVar18 = this->m_bCanPaint;
  }
  else {
    piVar19 = *(int **)(*(int *)(unaff_EBX + 0x4dd48b) + 0x1014);
    if (*piVar19 != unaff_EBX + 0x341e13) {
      piVar19 = (int *)CVProfNode::GetSubNode
                                 ((char *)piVar19,unaff_EBX + 0x341e13,(char *)0x0,
                                  unaff_EBX + 0x341e33);
      *(int **)(*(int *)(unaff_EBX + 0x4dd48b) + 0x1014) = piVar19;
    }
    iVar25 = *(int *)(unaff_EBX + 0x4dd48b);
    puVar1 = (uint *)(piVar19[0x1c] * 8 + *(int *)(iVar25 + 0x10a0) + 4);
    *puVar1 = *puVar1 | 4;
    CVProfNode::EnterScope();
    *(undefined1 *)(iVar25 + 0x1010) = 0;
    cVar18 = this->m_bCanPaint;
  }
  if (cVar18 == '\0') goto LAB_006ce8ad;
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
           *(float *)(*(int *)(unaff_EBX + 0x660d0b) + 0x2c) <=
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
    puVar8 = *(undefined4 **)(unaff_EBX + 0x4dd467);
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
                (fVar35 - fVar36) * (fVar35 - fVar36) < *(float *)(unaff_EBX + 0x364b93)) {
              pfVar27[3] = local_58;
              pfVar27[4] = fVar3;
              pfVar27[5] = fVar36;
              *pfVar27 = fVar16;
              pfVar27[1] = fVar2;
              pfVar27[2] = fVar35;
              goto LAB_006ceba4;
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
LAB_006cf146:
            iVar26 = local_2a8 * 0x18;
          }
          else {
            local_2a8 = ((int)uVar17 / local_2a4 + 1) * local_2a4;
            if (iVar25 <= local_2a8) goto LAB_006cf146;
            if (local_2a8 != 0) {
LAB_006cf136:
              do {
                local_2a8 = (local_2a8 + iVar25) / 2;
              } while (local_2a8 < iVar25);
              goto LAB_006cf146;
            }
            if (iVar25 < 0) {
              local_2a8 = -1;
              iVar26 = -0x18;
            }
            else {
              iVar26 = 0;
              if (iVar25 != 0) goto LAB_006cf136;
            }
          }
          if (local_2ac == (undefined4 *)0x0) {
            local_2ac = (undefined4 *)
                        (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x4dd467))
                                  ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x4dd467),iVar26);
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
LAB_006ceba4:
      local_32c = local_32c + 1;
    } while (local_32c != local_338);
  }
  local_114 = unaff_EBX + 0x5668e7;
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
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dd4db) + 0x2c))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4dd4db),iVar25 + (int)local_2ac,
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
      (**(code **)(**(int **)(local_110 + iVar25 * 4) + 400))(*(int **)(local_110 + iVar25 * 4));
      iVar25 = iVar25 + 1;
    } while (iVar25 < local_80);
  }
  if (*(int *)(**(int **)(unaff_EBX + 0x4dd46f) + 0x14) < 2) {
LAB_006cf355:
    (this->m_PaintThisFrame).m_Size = 0;
    local_114 = unaff_EBX + 0x5668e7;
    local_80 = 0;
    if (-1 < local_108) {
      if (local_110 != (undefined1 *)0x0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dd467) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x4dd467),local_110);
        local_110 = (undefined1 *)0x0;
      }
      local_10c = 0;
    }
    local_7c = local_110;
    if (-1 < local_108) {
      if (local_110 != (undefined1 *)0x0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dd467) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x4dd467),local_110);
        local_110 = (undefined1 *)0x0;
      }
      local_10c = 0;
    }
    local_11c = 0;
    if ((-1 < local_2a4) && (local_2ac != (undefined4 *)0x0)) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dd467) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4dd467),local_2ac);
LAB_006cf41d:
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
      sVar9 = (this->m_PaintThisFrame).m_Size;
      if (1 < (int)sVar9) {
        pPVar28 = (this->m_PaintThisFrame).m_Memory.m_pMemory;
        if (pPVar28 == (PaintLocationData_t *)0x0) {
          local_2f8 = sVar9 - 1;
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
                  PVar10 = pPVar28->type;
                  pCVar11 = pPVar28->pBrushEntity;
                    /* Unresolved local var: PaintLocationData_t temp@[???] */
                  pPVar28->pBrushEntity = pPVar30->pBrushEntity;
                  pPVar28->type = pPVar30->type;
                  (pPVar28->location).x = (pPVar30->location).x;
                  (pPVar28->location).y = (pPVar30->location).y;
                  (pPVar28->location).z = (pPVar30->location).z;
                  (pPVar28->normal).x = (pPVar30->normal).x;
                  (pPVar28->normal).y = (pPVar30->normal).y;
                  (pPVar28->normal).z = (pPVar30->normal).z;
                  pPVar30->pBrushEntity = pCVar11;
                  pPVar30->type = PVar10;
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
          _qsort(pPVar28,sVar9,0x20,*(int **)(unaff_EBX + 0x5fd2d3));
        }
      }
      CRecipientFilter::CRecipientFilter(&local_78);
      local_78.super_IRecipientFilter._vptr_IRecipientFilter =
           (_func_int_varargs **)(unaff_EBX + 0x4df9d7);
      CRecipientFilter::AddRecipient(&local_78,pPlayer);
      CRecipientFilter::MakeReliable(&local_78);
      local_2dc = (this->m_PaintThisFrame).m_Memory.m_pMemory;
      pPVar28 = local_2dc + (this->m_PaintThisFrame).m_Size;
      if (local_2dc != pPVar28) {
        do {
          pPlayer = (CBasePlayer *)(unaff_EBX + 0x341e39);
          UserMessageBegin(&local_78.super_IRecipientFilter,(char *)(unaff_EBX + 0x341e39));
          PVar10 = local_2dc->type;
          MessageWriteByte(PVar10);
          pCVar11 = local_2dc->pBrushEntity;
          MessageWriteEHandle(pCVar11);
          local_2c8 = (int)pPVar28 - (int)local_2dc;
          iVar26 = local_2c8 >> 7;
          pPVar30 = local_2dc;
          iVar25 = local_2c8;
          if (iVar26 < 1) {
LAB_006cf48f:
            iVar25 = iVar25 >> 5;
            if (iVar25 == 2) {
LAB_006cf519:
              if ((pCVar11 == pPVar30->pBrushEntity) && (PVar10 == pPVar30->type)) {
                pPVar30 = pPVar30 + 1;
LAB_006cf4a4:
                if ((pCVar11 == pPVar30->pBrushEntity) && (PVar10 == pPVar30->type))
                goto LAB_006ceee0;
              }
LAB_006cf4b2:
              local_2c8 = (int)pPVar30 - (int)local_2dc;
            }
            else if (iVar25 == 3) {
              if ((pCVar11 == pPVar30->pBrushEntity) && (PVar10 == pPVar30->type)) {
                pPVar30 = pPVar30 + 1;
                goto LAB_006cf519;
              }
              local_2c8 = (int)pPVar30 - (int)local_2dc;
            }
            else if (iVar25 == 1) goto LAB_006cf4a4;
LAB_006ceee0:
            local_318 = 0x28;
            if (local_2c8 < 0x500) goto LAB_006ceef7;
          }
          else {
            if ((pCVar11 == local_2dc->pBrushEntity) && (PVar10 == local_2dc->type)) {
              pPVar20 = local_2dc + 1;
              if (local_2dc[1].pBrushEntity == pCVar11) {
                if (((local_2dc[1].type == PVar10) &&
                    (pPVar20 = local_2dc + 2, local_2dc[2].pBrushEntity == pCVar11)) &&
                   (PVar12 = local_2dc[2].type, PVar12 == PVar10)) {
                  pPVar20 = local_2dc + 3;
                  pCVar13 = local_2dc[3].pBrushEntity;
                  if (pCVar13 != pCVar11) goto LAB_006cf4e1;
                  do {
                    if (PVar12 != pPVar20->type) break;
                    pPVar30 = pPVar20 + 1;
                    iVar26 = iVar26 + -1;
                    if (iVar26 == 0) {
                      iVar25 = (int)pPVar28 - (int)pPVar30;
                      goto LAB_006cf48f;
                    }
                    if ((pCVar13 != pPVar20[1].pBrushEntity) ||
                       ((((PVar12 != pPVar20[1].type ||
                          (pPVar30 = pPVar20 + 2, pCVar13 != pPVar20[2].pBrushEntity)) ||
                         (PVar12 != pPVar20[2].type)) ||
                        ((pPVar30 = pPVar20 + 3, pCVar13 != pPVar20[3].pBrushEntity ||
                         (PVar12 != pPVar20[3].type)))))) goto LAB_006cf4b2;
                    pPVar20 = pPVar20 + 4;
                  } while (pCVar13 == pPVar20->pBrushEntity);
                }
                local_2c8 = (int)pPVar20 - (int)local_2dc;
              }
              else {
LAB_006cf4e1:
                local_2c8 = (int)pPVar20 - (int)local_2dc;
              }
              goto LAB_006ceee0;
            }
            local_2c8 = 0;
LAB_006ceef7:
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
          fVar34 = *(float *)(unaff_EBX + 0x35bbdf) / (float)local_318;
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
           (_func_int_varargs **)(unaff_EBX + 0x4df9d7);
      CRecipientFilter::~CRecipientFilter(&local_78,(int)pPlayer);
      goto LAB_006cf355;
    }
    this->m_bSentClientPaintData = false;
    (this->m_PaintThisFrame).m_Size = 0;
    local_114 = unaff_EBX + 0x5668e7;
    local_80 = 0;
    if (-1 < local_108) {
      if (local_110 != (undefined1 *)0x0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dd467) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x4dd467),local_110);
        local_110 = (undefined1 *)0x0;
      }
      local_10c = 0;
    }
    local_7c = local_110;
    if (-1 < local_108) {
      if (local_110 != (undefined1 *)0x0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dd467) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x4dd467),local_110);
        local_110 = (undefined1 *)0x0;
      }
      local_10c = 0;
    }
    local_11c = 0;
    if ((-1 < local_2a4) && (local_2ac != (undefined4 *)0x0)) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dd467) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4dd467),local_2ac);
      goto LAB_006cf41d;
    }
  }
  local_118 = local_2ac;
  if ((-1 < local_2a4) && (local_2ac != (undefined4 *)0x0)) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dd467) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4dd467),local_2ac);
  }
LAB_006ce8ad:
  if (((*(char *)(*(int *)(unaff_EBX + 0x4dd48b) + 0x1010) == '\0') ||
      (*(int *)(*(int *)(unaff_EBX + 0x4dd48b) + 0x100c) != 0)) &&
     (iVar25 = *(int *)(*(int *)(unaff_EBX + 0x4dd48b) + 0x19b8), iVar26 = _ThreadGetCurrentId(),
     iVar25 == iVar26)) {
    cVar18 = CVProfNode::ExitScope();
    if (cVar18 != '\0') {
      *(undefined4 *)(*(int *)(unaff_EBX + 0x4dd48b) + 0x1014) =
           *(undefined4 *)(*(int *)(*(int *)(unaff_EBX + 0x4dd48b) + 0x1014) + 100);
    }
    iVar25 = *(int *)(unaff_EBX + 0x4dd48b);
    *(bool *)(iVar25 + 0x1010) = *(int *)(iVar25 + 0x1014) == iVar25 + 0x1018;
    return;
  }
  return;
}


/* CPaintAffectedEntityList::EnumEntity at 006d00e0 */

/* DWARF original prototype: bool EnumEntity(CPaintAffectedEntityList * this, IHandleEntity *
   pHandleEntity) */

bool __thiscall
CPaintAffectedEntityList::EnumEntity(CPaintAffectedEntityList *this,IHandleEntity *pHandleEntity)

{
  uint uVar1;
  uint *puVar2;
  int iVar3;
  CBaseEntity **ppCVar4;
  int iVar5;
  int *piVar6;
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
  puVar2 = (uint *)(*pHandleEntity->_vptr_IHandleEntity[3])(pHandleEntity);
  uVar1 = *puVar2;
  if (uVar1 == 0xffffffff) {
    return true;
  }
  iVar9 = (uVar1 & 0xffff) * 0x10 + *(int *)(unaff_EBX + 0x4dbb6c);
  if (*(uint *)(iVar9 + 8) != uVar1 >> 0x10) {
    return true;
  }
  piVar6 = *(int **)(iVar9 + 4);
  if (piVar6 == (int *)0x0) {
    return true;
  }
  iVar9 = (**(code **)(*piVar6 + 0x18))(piVar6);
  if (iVar9 == 0) {
    return true;
  }
  if ((*(byte *)(iVar9 + 0x132) & 8) == 0) {
    return true;
  }
  uVar1 = (this->m_EntitiesToUpdate).
          super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.m_Size;
  iVar3 = uVar1 + 1;
  iVar5 = (this->m_EntitiesToUpdate).
          super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.m_Memory.
          super_CUtlMemory<CBaseEntity*,int>.m_nAllocationCount;
  if (iVar3 <= iVar5) goto LAB_006d0270;
  iVar11 = (this->m_EntitiesToUpdate).
           super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.m_Memory
           .super_CUtlMemory<CBaseEntity*,int>.m_nGrowSize;
  iVar10 = iVar5;
  if (iVar11 < 0) {
    (this->m_EntitiesToUpdate).
    super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.m_Memory.
    super_CUtlMemory<CBaseEntity*,int>.m_nGrowSize =
         (this->m_EntitiesToUpdate).
         super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.m_Memory.
         m_nMallocGrowSize;
    if (iVar5 == 0) {
      (this->m_EntitiesToUpdate).
      super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.m_Memory.
      super_CUtlMemory<CBaseEntity*,int>.m_pMemory = (CBaseEntity **)0x0;
    }
    else {
                    /* Unresolved local var: int nNumBytes@[???]
                       Unresolved local var: CBaseEntity * * pMemory@[???] */
      uVar12 = iVar5 * 4;
      ppCVar7 = (CBaseEntity **)
                (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x4dbaa4))
                          ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x4dbaa4),uVar12);
      ppCVar4 = (this->m_EntitiesToUpdate).
                super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.
                m_Memory.super_CUtlMemory<CBaseEntity*,int>.m_pMemory;
      ppCVar8 = ppCVar7;
      if ((7 < uVar12) && (((uint)ppCVar7 & 4) != 0)) {
        *ppCVar7 = *ppCVar4;
        ppCVar8 = ppCVar7 + 1;
        ppCVar4 = ppCVar4 + 1;
        uVar12 = uVar12 - 4;
      }
      for (uVar12 = uVar12 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
        *ppCVar8 = *ppCVar4;
        ppCVar4 = ppCVar4 + 1;
        ppCVar8 = ppCVar8 + 1;
      }
      (this->m_EntitiesToUpdate).
      super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.m_Memory.
      super_CUtlMemory<CBaseEntity*,int>.m_pMemory = ppCVar7;
    }
    iVar11 = (this->m_EntitiesToUpdate).
             super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.
             m_Memory.super_CUtlMemory<CBaseEntity*,int>.m_nGrowSize;
    if (iVar11 < 0) goto LAB_006d0270;
    iVar10 = (this->m_EntitiesToUpdate).
             super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.
             m_Memory.super_CUtlMemory<CBaseEntity*,int>.m_nAllocationCount;
  }
  iVar5 = (iVar3 - iVar5) + iVar10;
  if (iVar11 == 0) {
    if (iVar10 == 0) {
      iVar10 = 8;
    }
    if (iVar10 < iVar5) {
      do {
        local_20 = iVar10;
        iVar10 = local_20 * 2;
      } while (iVar10 < iVar5);
      local_20 = local_20 * 8;
    }
    else {
      local_20 = iVar10 * 4;
    }
  }
  else {
    iVar10 = ((iVar5 + -1) / iVar11 + 1) * iVar11;
    if (iVar10 < iVar5) {
      if (iVar10 == 0) {
        if (iVar5 < 0) {
          iVar10 = -1;
          local_20 = -4;
        }
        else {
          local_20 = 0;
          if (iVar5 != 0) goto LAB_006d0200;
        }
      }
      else {
LAB_006d0200:
        do {
          iVar10 = (iVar10 + iVar5) / 2;
        } while (iVar10 < iVar5);
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
  ppCVar4 = (this->m_EntitiesToUpdate).
            super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.
            m_Memory.super_CUtlMemory<CBaseEntity*,int>.m_pMemory;
  if (ppCVar4 == (CBaseEntity **)0x0) {
    ppCVar4 = (CBaseEntity **)
              (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x4dbaa4))
                        ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x4dbaa4),local_20);
    (this->m_EntitiesToUpdate).
    super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.m_Memory.
    super_CUtlMemory<CBaseEntity*,int>.m_pMemory = ppCVar4;
  }
  else {
    ppCVar4 = (CBaseEntity **)
              (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dbaa4) + 4))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x4dbaa4),ppCVar4,local_20);
    (this->m_EntitiesToUpdate).
    super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.m_Memory.
    super_CUtlMemory<CBaseEntity*,int>.m_pMemory = ppCVar4;
  }
LAB_006d0270:
  piVar6 = &(this->m_EntitiesToUpdate).
            super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.m_Size;
  *piVar6 = *piVar6 + 1;
  ppCVar4 = (this->m_EntitiesToUpdate).
            super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.
            m_Memory.super_CUtlMemory<CBaseEntity*,int>.m_pMemory;
  (this->m_EntitiesToUpdate).
  super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.m_pElements =
       ppCVar4;
  iVar5 = ~uVar1 + (this->m_EntitiesToUpdate).
                   super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>.
                   m_Size;
  if (iVar5 < 1) {
    iVar11 = uVar1 << 2;
  }
  else {
    iVar11 = uVar1 * 4;
    _V_memmove(ppCVar4 + iVar3,ppCVar4 + uVar1,iVar5 * 4);
  }
  piVar6 = (int *)(iVar11 + (int)(this->m_EntitiesToUpdate).
                                 super_CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*,_32ul,_int>_>
                                 .m_Memory.super_CUtlMemory<CBaseEntity*,int>.m_pMemory);
  if (piVar6 != (int *)0x0) {
    *piVar6 = iVar9;
  }
  return true;
}


/* CC_RemoveAllPaint at 006cc2c0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void CC_RemoveAllPaint(void)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CPaintDatabase::RemoveAllPaint((CPaintDatabase *)(unaff_EBX + 0x6630d4));
  return;
}


/* __tcf_2 at 0099a9e0 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x41dc58),in_stack_00000008);
  return;
}


/* CC_PaintAt at 006ccac0 */

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
    DevMsg((char *)(unaff_EBX + 0x3439b1));
  }
  else {
    powerType = _atoi(args->m_ppArgv[1]);
    pcVar1 = (char *)(unaff_EBX + 0x2df085);
    if (4 < args->m_nArgc) {
      pcVar1 = args->m_ppArgv[4];
    }
    dVar2 = _atof(pcVar1);
    pcVar1 = (char *)(unaff_EBX + 0x2df085);
    if (3 < args->m_nArgc) {
      pcVar1 = args->m_ppArgv[3];
    }
    dVar3 = _atof(pcVar1);
    pcVar1 = (char *)(unaff_EBX + 0x2df085);
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
    CPaintDatabase::AddPaint((CPaintDatabase *)(unaff_EBX + 0x6628c9),&local_88,powerType);
    if (args->m_nArgc == 6) {
      dVar2 = _atof(args->m_ppArgv[5]);
      NDebugOverlay::Sphere(local_28,2.0,0xff,0xff,0,true,(float)dVar2);
    }
  }
  return;
}


/* __tcf_3 at 0099a9c0 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x41dcb8),in_stack_00000008);
  return;
}


/* CPaintDatabase::SavePaintmapData at 006cfa60 */

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
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dc147) + 0x220))
            ((int *)**(undefined4 **)(unaff_EBX + 0x4dc147),local_50);
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
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dc147) + 0x234))
            ((int *)**(undefined4 **)(unaff_EBX + 0x4dc147),&stack0xffffffc4);
  numBits = local_3c & 0xffff;
  (*pSave->_vptr_ISave[8])(pSave);
  (*pSave->_vptr_ISave[0xb])(pSave,&numBits,1);
  (*pSave->_vptr_ISave[0xb])(pSave,local_34,local_3c >> 0x10);
  (*pSave->_vptr_ISave[9])(pSave);
  if (1 < uStack_3a) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dc127) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4dc127),local_34);
  }
  local_34 = 0;
  local_6c = local_50[3];
  local_64 = local_50[3] * 0x14;
  puVar2 = *(undefined4 **)(unaff_EBX + 0x4dc127);
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
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dc127) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x4dc127),iVar4);
        *piVar7 = 0;
      }
      piVar7[1] = 0;
    }
  }
  local_50[3] = 0;
  if (-1 < local_50[2]) {
    if (local_50[0] != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dc127) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4dc127),local_50[0]);
      local_50[0] = 0;
    }
    local_50[1] = 0;
  }
  if ((-1 < local_50[2]) && (local_50[0] != 0)) {
    local_50[4] = local_50[0];
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dc127) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4dc127),local_50[0]);
  }
  return;
}


/* CPaintDatabase::RestorePaintmapData at 006ccc50 */

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
  DevMsg((char *)(unaff_EBX + 0x343865),iVar1);
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
                     (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x4def3a))
                               ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x4def3a),uVar3 << 2);
        }
      }
    }
    else if (uVar3 == 0) {
      if (1 < local_2c._2_2_) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4def3a) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x4def3a),local_24);
      }
      local_24 = (undefined4 *)0x0;
    }
    else if (local_24 == &local_28) {
      if (uVar3 != 1) {
        local_24 = (undefined4 *)
                   (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x4def3a))
                             ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x4def3a),uVar3 << 2);
        *local_24 = local_28;
      }
    }
    else if (uVar3 == 1) {
      local_28 = *local_24;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4def3a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4def3a),local_24);
      local_24 = &local_28;
    }
    else {
      local_24 = (undefined4 *)
                 (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4def3a) + 4))
                           ((int *)**(undefined4 **)(unaff_EBX + 0x4def3a),local_24,uVar3 << 2);
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
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4def5a) + 0x238))
            ((int *)**(undefined4 **)(unaff_EBX + 0x4def5a),&local_2c);
  if (1 < local_2c._2_2_) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4def3a) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4def3a),local_24);
  }
  return;
}


/* CPaintDatabase::SendPaintDataTo at 006cdd80 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void SendPaintDataTo(CPaintDatabase * this, CBasePlayer * pPlayer) */

void __thiscall CPaintDatabase::SendPaintDataTo(CPaintDatabase *this,CBasePlayer *pPlayer)

{
  char cVar1;
  undefined4 *puVar2;
  int iVar3;
  float *pfVar4;
  char *pcVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int unaff_EBX;
  int *piVar10;
  int iVar11;
  int iVar12;
  int local_ac;
  int local_a4;
  int local_a0;
  int local_9c;
  int local_8c;
  float local_88;
  int local_84;
  int local_80;
  CRecipientFilter local_78;
  CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> local_58;
  CUtlVector<float,CUtlMemory<float,_int>_> local_44;
  int local_30;
  undefined4 local_2c;
  int local_28;
  int local_24;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  if (pPlayer->m_iConnected != PlayerDisconnected) {
    local_30 = 0;
    local_2c = 0;
    local_28 = 0;
    local_24 = 0;
    local_20 = 0;
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dde27) + 0x220))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4dde27),&local_30);
    CRecipientFilter::CRecipientFilter(&local_78);
    local_78.super_IRecipientFilter._vptr_IRecipientFilter =
         (_func_int_varargs **)(unaff_EBX + 0x4e0377);
    CRecipientFilter::AddRecipient(&local_78,pPlayer);
    CRecipientFilter::MakeReliable(&local_78);
    iVar12 = local_24;
    local_44.m_Memory.m_pMemory = (float *)0x0;
    local_44.m_Memory.m_nAllocationCount = 0;
    local_44.m_Memory.m_nGrowSize = 0;
    local_44.m_Size = 0;
    local_44.m_pElements = (float *)0x0;
    local_58.m_Memory.m_pMemory = (uchar *)0x0;
    local_58.m_Memory.m_nAllocationCount = 0;
    local_58.m_Memory.m_nGrowSize = 0;
    local_58.m_Size = 0;
    local_58.m_pElements = (uchar *)0x0;
                    /* Unresolved local var: int n@[???] */
    if (0 < local_24) {
      local_ac = 0;
      local_84 = 0;
                    /* Unresolved local var: int count@[???]
                       Unresolved local var: int total@[???]
                       Unresolved local var: int nRLE@[???]
                       Unresolved local var: int nOffset@[???] */
      do {
        iVar7 = *(int *)(local_30 + 0xc + local_84);
        if (0 < iVar7) {
          local_a4 = 0;
          local_80 = 0;
          local_9c = 0;
          iVar8 = 0;
          do {
            iVar3 = local_44.m_Size;
            iVar9 = *(int *)(local_30 + local_84);
            cVar1 = *(char *)(iVar9 + iVar8);
            iVar11 = iVar8 + 1;
            if ((iVar11 < iVar7) && (*(char *)(iVar9 + iVar11) == cVar1)) {
              iVar9 = iVar9 + iVar8;
              iVar6 = 1;
              do {
                iVar6 = iVar6 + 1;
                iVar11 = iVar11 + 1;
                if (iVar7 <= iVar11) break;
                pcVar5 = (char *)(iVar9 + 2);
                iVar9 = iVar9 + 1;
              } while (*pcVar5 == cVar1);
              local_88 = (float)iVar6;
              iVar8 = iVar11;
            }
            else {
              iVar6 = 1;
              local_88 = 1.0;
              iVar8 = iVar11;
            }
            local_a4 = local_a4 + iVar6;
            local_80 = local_80 + 1;
            CUtlVector<float,CUtlMemory<float,_int>_>::GrowVector(&local_44,1);
            if (~iVar3 + local_44.m_Size < 1) {
              iVar11 = iVar3 << 2;
            }
            else {
              iVar11 = iVar3 * 4;
              _V_memmove((void *)((int)&(((CBaseCombatCharacter *)local_44.m_Memory.m_pMemory)->
                                        super_CBaseFlex).super_CBaseAnimatingOverlay.
                                        super_CBaseAnimating.super_CBaseEntity.m_Network.m_PVSInfo +
                                 iVar3 * 4 + 0xffffffdcU),
                         (void *)((int)&(((CBaseCombatCharacter *)local_44.m_Memory.m_pMemory)->
                                        super_CBaseFlex).super_CBaseAnimatingOverlay.
                                        super_CBaseAnimating.super_CBaseEntity.m_Network.m_PVSInfo +
                                 iVar11 + 0xffffffd8U),(~iVar3 + local_44.m_Size) * 4);
            }
            iVar9 = local_58.m_Size;
            pfVar4 = (float *)((int)&(((CBaseCombatCharacter *)local_44.m_Memory.m_pMemory)->
                                     super_CBaseFlex).super_CBaseAnimatingOverlay.
                                     super_CBaseAnimating.super_CBaseEntity.m_Network.m_PVSInfo +
                              iVar11 + 0xffffffd8U);
            if (pfVar4 != (float *)0x0) {
              *pfVar4 = local_88;
            }
            pPlayer = (CBasePlayer *)((int)&vgui::Panel::GetMessageMap::s_pMap.magic + 1);
            CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_>::GrowVector(&local_58,1);
            if (0 < ~iVar9 + local_58.m_Size) {
              pPlayer = (CBasePlayer *)
                        ((int)&(((CBaseCombatCharacter *)local_58.m_Memory.m_pMemory)->
                               super_CBaseFlex).super_CBaseAnimatingOverlay.super_CBaseAnimating.
                               super_CBaseEntity.m_Network.m_PVSInfo + iVar9 + 0xffffffd8U);
              _V_memmove((void *)((int)&(((CBaseCombatCharacter *)local_58.m_Memory.m_pMemory)->
                                        super_CBaseFlex).super_CBaseAnimatingOverlay.
                                        super_CBaseAnimating.super_CBaseEntity.m_Network.m_PVSInfo +
                                 iVar9 + 0xffffffd9U),pPlayer,~iVar9 + local_58.m_Size);
            }
            pcVar5 = (char *)((int)&(((CBaseCombatCharacter *)local_58.m_Memory.m_pMemory)->
                                    super_CBaseFlex).super_CBaseAnimatingOverlay.
                                    super_CBaseAnimating.super_CBaseEntity.m_Network.m_PVSInfo +
                             iVar9 + 0xffffffd8U);
            if (pcVar5 != (char *)0x0) {
              *pcVar5 = cVar1;
            }
            if ((local_80 == 0x32) || (iVar7 == local_a4)) {
              pPlayer = (CBasePlayer *)(unaff_EBX + 0x2eec98);
              UserMessageBegin(&local_78.super_IRecipientFilter,(char *)(unaff_EBX + 0x2eec98));
              MessageWriteByte(local_ac);
              MessageWriteFloat((float)local_9c);
              MessageWriteFloat((float)local_80);
                    /* Unresolved local var: int m@[???] */
              if (0 < local_80) {
                iVar11 = 0;
                do {
                  MessageWriteFloat(*(float *)((int)&(((CBaseCombatCharacter *)
                                                      local_44.m_Memory.m_pMemory)->super_CBaseFlex)
                                                     .super_CBaseAnimatingOverlay.
                                                     super_CBaseAnimating.super_CBaseEntity.
                                                     m_Network.m_PVSInfo + iVar11 * 4 + 0xffffffd8U)
                                   );
                  MessageWriteByte((uint)*(byte *)((int)&(((CBaseCombatCharacter *)
                                                          local_58.m_Memory.m_pMemory)->
                                                         super_CBaseFlex).
                                                         super_CBaseAnimatingOverlay.
                                                         super_CBaseAnimating.super_CBaseEntity.
                                                         m_Network.m_PVSInfo + iVar11 + 0xffffffd8U)
                                  );
                  iVar11 = iVar11 + 1;
                } while (iVar11 != local_80);
              }
              MessageEnd();
              local_44.m_Size = 0;
              local_58.m_Size = 0;
              local_a0 = 0;
              local_9c = local_a4;
              local_80 = local_a0;
            }
          } while (iVar8 < iVar7);
        }
        local_ac = local_ac + 1;
        local_84 = local_84 + 0x14;
      } while (local_ac != iVar12);
    }
    local_58.m_Size = 0;
    if (-1 < local_58.m_Memory.m_nGrowSize) {
      if ((CBasePlayer *)local_58.m_Memory.m_pMemory != (CBasePlayer *)0x0) {
        pPlayer = (CBasePlayer *)local_58.m_Memory.m_pMemory;
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dde07) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x4dde07));
        local_58.m_Memory.m_pMemory = (uchar *)0x0;
      }
      local_58.m_Memory.m_nAllocationCount = 0;
    }
    local_58.m_pElements = local_58.m_Memory.m_pMemory;
    if (-1 < local_58.m_Memory.m_nGrowSize) {
      if ((CBasePlayer *)local_58.m_Memory.m_pMemory != (CBasePlayer *)0x0) {
        pPlayer = (CBasePlayer *)local_58.m_Memory.m_pMemory;
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dde07) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x4dde07));
        local_58.m_Memory.m_pMemory = (uchar *)0x0;
      }
      local_58.m_Memory.m_nAllocationCount = 0;
    }
    local_44.m_Size = 0;
    if (-1 < local_44.m_Memory.m_nGrowSize) {
      if ((CBasePlayer *)local_44.m_Memory.m_pMemory != (CBasePlayer *)0x0) {
        pPlayer = (CBasePlayer *)local_44.m_Memory.m_pMemory;
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dde07) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x4dde07));
        local_44.m_Memory.m_pMemory = (float *)0x0;
      }
      local_44.m_Memory.m_nAllocationCount = 0;
    }
    local_44.m_pElements = local_44.m_Memory.m_pMemory;
    if (-1 < local_44.m_Memory.m_nGrowSize) {
      if ((CBasePlayer *)local_44.m_Memory.m_pMemory != (CBasePlayer *)0x0) {
        pPlayer = (CBasePlayer *)local_44.m_Memory.m_pMemory;
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dde07) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x4dde07));
        local_44.m_Memory.m_pMemory = (float *)0x0;
      }
      local_44.m_Memory.m_nAllocationCount = 0;
    }
    local_78.super_IRecipientFilter._vptr_IRecipientFilter =
         (_func_int_varargs **)(unaff_EBX + 0x4e0377);
    CRecipientFilter::~CRecipientFilter(&local_78,(int)pPlayer);
    local_8c = local_24;
    iVar12 = local_24 * 0x14;
    puVar2 = *(undefined4 **)(unaff_EBX + 0x4dde07);
    while( true ) {
      iVar12 = iVar12 + -0x14;
      local_8c = local_8c + -1;
      if (local_8c < 0) break;
      piVar10 = (int *)(iVar12 + local_30);
      piVar10[3] = 0;
      if (piVar10[2] < 0) {
        iVar7 = *piVar10;
      }
      else {
        if (*piVar10 != 0) {
          (**(code **)(*(int *)*puVar2 + 8))((int *)*puVar2,*piVar10);
          *piVar10 = 0;
        }
        iVar7 = 0;
        piVar10[1] = 0;
      }
      piVar10[4] = iVar7;
      if (-1 < piVar10[2]) {
        if (iVar7 != 0) {
          (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dde07) + 8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x4dde07),iVar7);
          *piVar10 = 0;
        }
        piVar10[1] = 0;
      }
    }
    local_24 = 0;
    if (-1 < local_28) {
      if (local_30 != 0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dde07) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x4dde07),local_30);
        local_30 = 0;
      }
      local_2c = 0;
    }
    if ((-1 < local_28) && (local_30 != 0)) {
      local_20 = local_30;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dde07) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4dde07),local_30);
    }
  }
  return;
}


/* CPaintDatabase::SendPaintDataToEngine at 006cbef0 */

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
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dfcba) + 0x224))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4dfcba),
                 (int)&(((this->m_Paintmaps).m_Memory.m_pMemory)->m_Memory).m_pMemory + local_20,
                 iVar4);
      iVar4 = iVar4 + 1;
      iVar6 = (this->m_Paintmaps).m_Size;
      local_20 = local_20 + 0x14;
    } while (iVar4 < iVar6);
  }
  puVar1 = *(undefined4 **)(unaff_EBX + 0x4dfc9a);
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
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dfc9a) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x4dfc9a),iVar3);
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
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4dfc9a) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4dfc9a),pCVar2);
    (this->m_Paintmaps).m_Memory.m_pMemory =
         (CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *)0x0;
  }
  (this->m_Paintmaps).m_Memory.m_nAllocationCount = 0;
  (this->m_Paintmaps).m_pElements = (CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> *)0x0
  ;
  return;
}


/* CC_PaintAllSurfaces at 006cc070 */

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
  (**(code **)(*(int *)**(undefined4 **)(CFuncTank::TankActivate + unaff_EBX + 4) + 0x21c))
            ((int *)**(undefined4 **)(CFuncTank::TankActivate + unaff_EBX + 4),uVar1);
  CRecipientFilter::CRecipientFilter(&local_3c);
  local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(unaff_EBX + 0x4e2184);
  CRecipientFilter::AddAllPlayers(&local_3c);
  CRecipientFilter::MakeReliable(&local_3c);
  messagename = (char *)(unaff_EBX + 0x3443e2);
  UserMessageBegin(&local_3c.super_IRecipientFilter,messagename);
  MessageWriteByte(local_40);
  MessageEnd();
  local_3c.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(unaff_EBX + 0x4e2184);
  CRecipientFilter::~CRecipientFilter(&local_3c,(int)messagename);
  return;
}


/* __tcf_4 at 0099a9a0 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x41dd18),in_stack_00000008);
  return;
}


/* _GLOBAL__I_PaintDatabase at 000c3080 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_PaintDatabase(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

