/* DWARF-guided pseudocode for game/server/portal2/item_nugget.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* CItem_Nugget::~CItem_Nugget at 0071c360 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CItem_Nugget(CItem_Nugget * this, int __in_chrg) */

void __thiscall CItem_Nugget::~CItem_Nugget(CItem_Nugget *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5ade97);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CItem_Nugget::~CItem_Nugget at 0071c3a0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CItem_Nugget(CItem_Nugget * this, int __in_chrg) */

void __thiscall CItem_Nugget::~CItem_Nugget(CItem_Nugget *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x5ade60);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
  return;
}


/* CEntityFactory<CItem_Nugget>::Create at 0071c3c0 */

/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CItem_Nugget> * this, char *
   pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CItem_Nugget>::Create(CEntityFactory<CItem_Nugget> *this,char *pClassName)

{
  CBaseAnimating *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CItem_Nugget * pEnt@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CBaseEntity::operator_new(0x520);
  CBaseAnimating::CBaseAnimating(this_00);
  (this_00->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5ade34);
  CBaseEntity::PostConstructor(&this_00->super_CBaseEntity,pClassName);
  return &(this_00->super_CBaseEntity).m_Network.super_IServerNetworkable;
}


/* DataMapInit<CItem_Nugget> at 000ab0b0 */

datamap_t * DataMapInit<CItem_Nugget>(CItem_Nugget *param_1)

{
  int iVar1;
  int iVar2;
  char *pcVar3;
  size_t sVar4;
  int iVar5;
  undefined4 uVar6;
  int iVar7;
  undefined4 *puVar8;
  int unaff_EBX;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  if ((*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x391].m_pPrev + unaff_EBX) ==
       '\0') && (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xda14fc), iVar2 != 0)) {
    *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x392].m_pPrev + unaff_EBX) =
         unaff_EBX + 0xa05e1f;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x393].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x393].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x393].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x393].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x394].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x392].m_pNext + unaff_EBX) =
         0xc;
    ___cxa_guard_release(unaff_EBX + 0xda14fc);
    ___cxa_atexit(unaff_EBX + 0x991dac,0,*(undefined4 *)(&DAT_00b9ba90 + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_CanStandOn_00cb9268 + unaff_EBX) =
       *(undefined4 *)(&DAT_00b9ba9c + unaff_EBX);
  if ((*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x392].m_pEntity + unaff_EBX) !=
       '\0') || (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xda1504), iVar2 == 0)) goto LAB_000ab104;
  pcVar3 = operator_new___(*(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x392].
                                          m_pNext + unaff_EBX) + 0x11);
  _strcpy(pcVar3,*(char **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x392].m_pPrev +
                           unaff_EBX));
  sVar4 = _strlen(pcVar3);
  builtin_strncpy(pcVar3 + sVar4,"MaterializeThink",0x11);
  iVar1 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x393].m_pNext + unaff_EBX);
  iVar2 = iVar1 + 1;
  iVar5 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x393].m_SerialNumber +
                  unaff_EBX);
  if (iVar5 < iVar2) {
    iVar7 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x393].m_pPrev + unaff_EBX);
    if (-1 < iVar7) {
      if (iVar7 == 0) {
        if ((iVar5 == 0) && (iVar5 = 8, iVar2 < 9)) {
          local_20 = 0x20;
        }
        else {
          do {
            local_20 = iVar5;
            iVar5 = local_20 * 2;
          } while (iVar5 < iVar2);
          local_20 = local_20 * 8;
        }
        goto LAB_000ab230;
      }
      iVar5 = (iVar1 / iVar7 + 1) * iVar7;
      if (iVar5 < iVar2) {
        if (iVar5 != 0) {
LAB_000ab1f0:
          do {
            iVar5 = (iVar5 + iVar2) / 2;
          } while (iVar5 < iVar2);
          goto LAB_000ab200;
        }
        if (iVar2 < 0) {
          iVar5 = -1;
          local_20 = -4;
        }
        else {
          local_20 = 0;
          if (iVar2 != 0) goto LAB_000ab1f0;
        }
      }
      else {
LAB_000ab200:
        local_20 = iVar5 * 4;
      }
LAB_000ab230:
      *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x393].m_SerialNumber + unaff_EBX)
           = iVar5;
      iVar5 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x393].m_pEntity +
                      unaff_EBX);
      if (iVar5 == 0) {
        uVar6 = (*(code *)**(undefined4 **)**(undefined4 **)(&DAT_00b9ba98 + unaff_EBX))
                          ((undefined4 *)**(undefined4 **)(&DAT_00b9ba98 + unaff_EBX),local_20);
      }
      else {
        uVar6 = (**(code **)(*(int *)**(undefined4 **)(&DAT_00b9ba98 + unaff_EBX) + 4))
                          ((int *)**(undefined4 **)(&DAT_00b9ba98 + unaff_EBX),iVar5,local_20);
      }
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x393].m_pEntity + unaff_EBX) = uVar6;
    }
  }
  iVar7 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x393].m_pNext + unaff_EBX) +
          1;
  *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x393].m_pNext + unaff_EBX) = iVar7;
  iVar5 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x393].m_pEntity + unaff_EBX);
  *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x394].m_pEntity + unaff_EBX) = iVar5;
  iVar7 = (iVar7 - iVar1) + -1;
  if (0 < iVar7) {
    _V_memmove((void *)(iVar5 + iVar2 * 4),(void *)(iVar5 + iVar1 * 4),iVar7 * 4);
    iVar5 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x393].m_pEntity + unaff_EBX
                    );
  }
  puVar8 = (undefined4 *)(iVar5 + iVar1 * 4);
  if (puVar8 != (undefined4 *)0x0) {
    *puVar8 = pcVar3;
  }
  *(char **)((int)&PTR_VPhysicsCollision_00cb9380 + unaff_EBX) = pcVar3;
  ___cxa_guard_release(unaff_EBX + 0xda1504);
LAB_000ab104:
  *(undefined4 *)((int)&PTR_ChangeTeam_00cb9260 + unaff_EBX) = 4;
  *(int *)((int)&PTR_IsViewable_00cb925c + unaff_EBX) = (int)&PTR_FireBullets_00cb92bc + unaff_EBX;
  return (datamap_t *)((int)&PTR_IsViewable_00cb925c + unaff_EBX);
}


/* __static_initialization_and_destruction_0 at 000ab3b0 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  undefined4 uVar1;
  datamap_t *pdVar2;
  IEntityFactoryDictionary *pIVar3;
  int unaff_EBX;
  longlong lVar4;
  ConVar *in_stack_ffffffd8;
  char *in_stack_ffffffdc;
  char *in_stack_ffffffe0;
  int in_stack_ffffffe4;
  char *in_stack_ffffffe8;
  
  lVar4 = ___i686_get_pc_thunk_bx();
  if (lVar4 == 0xffff00000001) {
    *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x359].m_pPrev + unaff_EBX) =
         0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x359].m_pPrev + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x359].m_pPrev + unaff_EBX + 2) = 0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x359].m_pPrev + unaff_EBX + 3) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x359].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x35a].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x35a].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)
     (&UNK_000035ac + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
         0x7f7fffff;
    *(undefined4 *)
     (&UNK_000035ac + (int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x35b].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x35b].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x35b].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x35b].m_pNext + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x35c].m_pEntity + unaff_EBX)
         = 0x7f7fffff;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x35c].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x35c].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x35c].m_pNext + unaff_EBX) =
         &UNK_00ba0f44 + unaff_EBX;
    pdVar2 = DataMapInit<CItem_Nugget>((CItem_Nugget *)0x0);
    *(datamap_t **)((int)DataMapInit<CAI_Pathfinder>::dataDesc[1].flatOffset + unaff_EBX + 8) =
         pdVar2;
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x35d].m_pEntity + unaff_EBX)
         = &DAT_00c1f214 + unaff_EBX;
    pIVar3 = EntityFactoryDictionary();
    (**pIVar3->_vptr_IEntityFactoryDictionary)(pIVar3,unaff_EBX + 0xda11b4,unaff_EBX + 0xa05b2c);
    CAutoGameSystem::CAutoGameSystem
              ((CAutoGameSystem *)
               ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x35d].m_pPrev + unaff_EBX),
               (char *)0x0);
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x35d].m_pPrev + unaff_EBX) =
         &UNK_00c1ede4 + unaff_EBX;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x35e].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x35e].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x35e].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x35f].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x35f].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x35f].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x35f].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x360].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x360].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x360].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x360].m_pNext + unaff_EBX) =
         0;
    *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x361].m_pEntity + unaff_EBX)
         = 0;
    uVar1 = *(undefined4 *)(&DAT_00b9b790 + unaff_EBX);
    ___cxa_atexit(unaff_EBX + 0x991b8c,0,uVar1);
    ConVar::ConVar((ConVar *)((int)&CAI_AllySpeechManager::m_DataMap.baseMap + unaff_EBX),
                   &UNK_00a05b78 + unaff_EBX,(char *)(unaff_EBX + 0x9a3a3c),0,
                   (char *)(unaff_EBX + 0xa05b38),in_stack_ffffffd8,in_stack_ffffffdc,
                   in_stack_ffffffe0,in_stack_ffffffe4,in_stack_ffffffe8);
    ___cxa_atexit(unaff_EBX + 0x991a8c,0,uVar1);
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<CAI_PlayerAlly>::dataDesc[0].flatOffset + unaff_EBX + -0x18),
                   &UNK_00a05bfc + unaff_EBX,(char *)(unaff_EBX + 0x9b5710),0,
                   &UNK_00a05b9c + unaff_EBX,in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0,
                   in_stack_ffffffe4,in_stack_ffffffe8);
    ___cxa_atexit(unaff_EBX + 0x991a6c,0,uVar1);
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<CAI_PlayerAlly>::dataDesc[1].flatOffset + unaff_EBX + 8),
                   (char *)(unaff_EBX + 0xa05c88),(char *)(unaff_EBX + 0x9b5710),0,
                   &UNK_00a05c34 + unaff_EBX,in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0,
                   in_stack_ffffffe4,in_stack_ffffffe8);
    ___cxa_atexit(unaff_EBX + 0x991a4c,0,uVar1);
  }
  return;
}


/* CItem_Nugget::GetDataDescMap at 0071ac10 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CItem_Nugget * this) */

datamap_t * __thiscall CItem_Nugget::GetDataDescMap(CItem_Nugget *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x649708);
}


/* CItem_Nugget::GetBaseMap at 0071ac20 */

datamap_t * CItem_Nugget::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x52bf38);
}


/* __tcf_0 at 00a3ce70 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x40f766)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(&DAT_0040f75a + unaff_EBX) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x40f766));
  }
  *(undefined4 *)(unaff_EBX + 0x40f766) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40f762)) {
    if (*(int *)(&DAT_0040f75a + unaff_EBX) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x209cde) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x209cde),*(int *)(&DAT_0040f75a + unaff_EBX))
      ;
      *(undefined4 *)(&DAT_0040f75a + unaff_EBX) = 0;
    }
    *(undefined4 *)(&DAT_0040f75e + unaff_EBX) = 0;
  }
  iVar1 = *(int *)(&DAT_0040f75a + unaff_EBX);
  *(int *)(unaff_EBX + 0x40f76a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40f762)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x209cde) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x209cde),iVar1);
      *(undefined4 *)(&DAT_0040f75a + unaff_EBX) = 0;
    }
    *(undefined4 *)(&DAT_0040f75e + unaff_EBX) = 0;
  }
  return;
}


/* CNuggetDirector::~CNuggetDirector at 0071c9b0 */

/* DWARF original prototype: void ~CNuggetDirector(CNuggetDirector * this, int __in_chrg) */

void __thiscall CNuggetDirector::~CNuggetDirector(CNuggetDirector *this,int __in_chrg)

{
  CItem_Nugget **ppCVar1;
  int unaff_EBX;
  CItem_Nugget **in_stack_ffffffc8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(unaff_EBX + 0x5ad7ea);
  (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.m_Size =
       0;
  if (-1 < (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
           m_Memory.m_nGrowSize) {
    ppCVar1 = (this->m_GroupedNuggets).
              super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.m_Memory.m_pMemory;
    if (ppCVar1 != (CItem_Nugget **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52a19e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x52a19e));
      (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
      m_Memory.m_pMemory = (CItem_Nugget **)0x0;
      in_stack_ffffffc8 = ppCVar1;
    }
    (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
    m_Memory.m_nAllocationCount = 0;
  }
  ppCVar1 = (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>
            .m_Memory.m_pMemory;
  (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
  m_pElements = ppCVar1;
  if (-1 < (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
           m_Memory.m_nGrowSize) {
    if (ppCVar1 != (CItem_Nugget **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52a19e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x52a19e));
      (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
      m_Memory.m_pMemory = (CItem_Nugget **)0x0;
      in_stack_ffffffc8 = ppCVar1;
    }
    (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
    m_Memory.m_nAllocationCount = 0;
  }
  (this->m_UngroupedNuggets).m_Size = 0;
  if ((this->m_UngroupedNuggets).m_Memory.m_nGrowSize < 0) {
    ppCVar1 = (this->m_UngroupedNuggets).m_Memory.m_pMemory;
  }
  else {
    ppCVar1 = (this->m_UngroupedNuggets).m_Memory.m_pMemory;
    if (ppCVar1 != (CItem_Nugget **)0x0) {
      in_stack_ffffffc8 = ppCVar1;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52a19e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x52a19e));
      ppCVar1 = (CItem_Nugget **)0x0;
      (this->m_UngroupedNuggets).m_Memory.m_pMemory = (CItem_Nugget **)0x0;
    }
    (this->m_UngroupedNuggets).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_UngroupedNuggets).m_pElements = ppCVar1;
  if (-1 < (this->m_UngroupedNuggets).m_Memory.m_nGrowSize) {
    if (ppCVar1 != (CItem_Nugget **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52a19e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x52a19e));
      (this->m_UngroupedNuggets).m_Memory.m_pMemory = (CItem_Nugget **)0x0;
      in_stack_ffffffc8 = ppCVar1;
    }
    (this->m_UngroupedNuggets).m_Memory.m_nAllocationCount = 0;
  }
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(unaff_EBX + 0x52c6ca);
  IGameSystem::~IGameSystem((IGameSystem *)this,(int)in_stack_ffffffc8);
  operator_delete(this);
  return;
}


/* CNuggetDirector::~CNuggetDirector at 0071cbc0 */

/* DWARF original prototype: void ~CNuggetDirector(CNuggetDirector * this, int __in_chrg) */

void __thiscall CNuggetDirector::~CNuggetDirector(CNuggetDirector *this,int __in_chrg)

{
  CItem_Nugget **ppCVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(unaff_EBX + 0x5ad5da);
  (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.m_Size =
       0;
  if (-1 < (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
           m_Memory.m_nGrowSize) {
    ppCVar1 = (this->m_GroupedNuggets).
              super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.m_Memory.m_pMemory;
    if (ppCVar1 != (CItem_Nugget **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x529f8e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x529f8e),ppCVar1);
      (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
      m_Memory.m_pMemory = (CItem_Nugget **)0x0;
    }
    (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
    m_Memory.m_nAllocationCount = 0;
  }
  ppCVar1 = (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>
            .m_Memory.m_pMemory;
  (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
  m_pElements = ppCVar1;
  if (-1 < (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
           m_Memory.m_nGrowSize) {
    if (ppCVar1 != (CItem_Nugget **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x529f8e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x529f8e),ppCVar1);
      (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
      m_Memory.m_pMemory = (CItem_Nugget **)0x0;
    }
    (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
    m_Memory.m_nAllocationCount = 0;
  }
  (this->m_UngroupedNuggets).m_Size = 0;
  if ((this->m_UngroupedNuggets).m_Memory.m_nGrowSize < 0) {
    ppCVar1 = (this->m_UngroupedNuggets).m_Memory.m_pMemory;
  }
  else {
    ppCVar1 = (this->m_UngroupedNuggets).m_Memory.m_pMemory;
    if (ppCVar1 != (CItem_Nugget **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x529f8e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x529f8e),ppCVar1);
      (this->m_UngroupedNuggets).m_Memory.m_pMemory = (CItem_Nugget **)0x0;
    }
    ppCVar1 = (CItem_Nugget **)0x0;
    (this->m_UngroupedNuggets).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_UngroupedNuggets).m_pElements = ppCVar1;
  if (-1 < (this->m_UngroupedNuggets).m_Memory.m_nGrowSize) {
    if (ppCVar1 != (CItem_Nugget **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x529f8e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x529f8e),ppCVar1);
      (this->m_UngroupedNuggets).m_Memory.m_pMemory = (CItem_Nugget **)0x0;
    }
    (this->m_UngroupedNuggets).m_Memory.m_nAllocationCount = 0;
  }
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(unaff_EBX + 0x52c4ba);
  IGameSystem::~IGameSystem((IGameSystem *)this,__in_chrg);
  return;
}


/* __tcf_1 at 00a3cf50 */

void __tcf_1(void *param_1)

{
  int iVar1;
  int unaff_EBX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_bx();
  *(int *)(unaff_EBX + 0x40f622) = unaff_EBX + 0x28d24a;
  *(undefined4 *)(unaff_EBX + 0x40f64e) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40f64a)) {
    if (*(int *)(unaff_EBX + 0x40f642) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x209bfe) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x209bfe),*(int *)(unaff_EBX + 0x40f642));
      *(undefined4 *)(unaff_EBX + 0x40f642) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f646) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40f642);
  *(int *)(unaff_EBX + 0x40f652) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40f64a)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x209bfe) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x209bfe),iVar1);
      *(int *)(unaff_EBX + 0x40f642) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f646) = 0;
  }
  *(undefined4 *)(unaff_EBX + 0x40f63a) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40f636)) {
    if (*(int *)(unaff_EBX + 0x40f62e) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x209bfe) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x209bfe),*(int *)(unaff_EBX + 0x40f62e));
      *(undefined4 *)(unaff_EBX + 0x40f62e) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f632) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40f62e);
  *(int *)(unaff_EBX + 0x40f63e) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40f636)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x209bfe) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x209bfe),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40f62e) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f632) = 0;
  }
  *(int *)(unaff_EBX + 0x40f622) = unaff_EBX + 0x20c12a;
  IGameSystem::~IGameSystem((IGameSystem *)(unaff_EBX + 0x40f622),in_stack_00000008);
  return;
}


/* CItem_Nugget::Spawn at 0071b930 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "MoveCollide_t": Some values do not have unique names */
/* WARNING: Enum "MoveType_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(CItem_Nugget * this) */

void __thiscall CItem_Nugget::Spawn(CItem_Nugget *this)

{
  undefined1 *puVar1;
  NetworkVar_m_Collision *this_00;
  ushort uVar2;
  int iVar3;
  CBaseEdict *pCVar4;
  ushort *puVar5;
  int *piVar6;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  uint uVar9;
  int unaff_EBX;
  uint local_dc;
  uint local_d8;
  uint local_d4;
  uint local_d0;
  float local_9c;
  vec_t local_58;
  float local_54;
  vec_t local_50;
  Vector local_4c;
  Vector local_40;
  Vector local_34;
  AngularImpulse local_28 [2];
  
                    /* Unresolved local var: float fScale@[???]
                       Unresolved local var: float fScaledExtent@[???]
                       Unresolved local var: Vector vModelOffset@[???] */
  ___i686_get_pc_thunk_bx();
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1a])(this);
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1b])(this,unaff_EBX + 0x395563);
  iVar3 = this->m_nValue;
  if (iVar3 < 5) {
    this->m_nValue = 1;
    if ((this->super_CBaseAnimating).m_nSkin.m_Value != 1) {
      if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
          false) {
        pCVar4 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
          pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
          puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x52b227);
          if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
            uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar2 = puVar5[uVar8 * 0x14 + 0x14];
            if (uVar2 == 0) {
LAB_0071bef0:
              puVar5[(uint)uVar2 + uVar8 * 0x14 + 1] = 0x3e0;
              puVar5[uVar8 * 0x14 + 0x14] = uVar2 + 1;
            }
            else if (puVar5[uVar8 * 0x14 + 1] != 0x3e0) {
              local_dc = 0;
              do {
                uVar9 = local_dc + 1;
                local_dc = uVar9 & 0xffff;
                if ((ushort)uVar9 == uVar2) {
                  if (uVar2 != 0x13) goto LAB_0071bef0;
                  pIVar7->m_iChangeInfoSerialNumber = 0;
                  pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar5[uVar8 * 0x14 + local_dc + 1] != 0x3e0);
            }
          }
          else if (puVar5[0x7d1] == 100) {
            pIVar7->m_iChangeInfoSerialNumber = 0;
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar7->m_iChangeInfo = puVar5[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x52b227) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x52b227) + 0xfa2) + 1;
            piVar6 = *(int **)(unaff_EBX + 0x52b227);
            pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
            iVar3 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar3 + 2) = 0x3e0;
            *(undefined2 *)(iVar3 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->super_CBaseAnimating).m_nSkin.m_Value = 1;
    }
    local_9c = *(float *)(unaff_EBX + 0x3b11bf);
    local_50 = *(vec_t *)(unaff_EBX + 0x3b11fb);
    local_54 = *(float *)(unaff_EBX + 0x3b3b0b);
    local_58 = *(vec_t *)(unaff_EBX + 0x3b165f);
  }
  else if (iVar3 < 10) {
    this->m_nValue = 5;
    if ((this->super_CBaseAnimating).m_nSkin.m_Value != 2) {
      if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
          false) {
        pCVar4 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
          pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
          puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x52b227);
          if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
            uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar2 = puVar5[uVar8 * 0x14 + 0x14];
            if (uVar2 == 0) {
LAB_0071c0fa:
              puVar5[(uint)uVar2 + uVar8 * 0x14 + 1] = 0x3e0;
              puVar5[uVar8 * 0x14 + 0x14] = uVar2 + 1;
            }
            else if (puVar5[uVar8 * 0x14 + 1] != 0x3e0) {
              local_d8 = 0;
              do {
                uVar9 = local_d8 + 1;
                local_d8 = uVar9 & 0xffff;
                if ((ushort)uVar9 == uVar2) {
                  if (uVar2 != 0x13) goto LAB_0071c0fa;
                  pIVar7->m_iChangeInfoSerialNumber = 0;
                  pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar5[uVar8 * 0x14 + local_d8 + 1] != 0x3e0);
            }
          }
          else if (puVar5[0x7d1] == 100) {
            pIVar7->m_iChangeInfoSerialNumber = 0;
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar7->m_iChangeInfo = puVar5[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x52b227) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x52b227) + 0xfa2) + 1;
            piVar6 = *(int **)(unaff_EBX + 0x52b227);
            pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
            iVar3 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar3 + 2) = 0x3e0;
            *(undefined2 *)(iVar3 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->super_CBaseAnimating).m_nSkin.m_Value = 2;
    }
    local_9c = *(float *)(unaff_EBX + 0x3b11e3);
    local_50 = *(vec_t *)(unaff_EBX + 0x3b9b3b);
    local_54 = *(float *)(unaff_EBX + 0x3b9b3f);
    local_58 = *(vec_t *)(unaff_EBX + 0x3b9b43);
  }
  else if (iVar3 < 0x19) {
    this->m_nValue = 10;
    if ((this->super_CBaseAnimating).m_nSkin.m_Value != 3) {
      if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
          false) {
        pCVar4 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
          pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
          puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x52b227);
          if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
            uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar2 = puVar5[uVar8 * 0x14 + 0x14];
            if (uVar2 == 0) {
LAB_0071c277:
              puVar5[(uint)uVar2 + uVar8 * 0x14 + 1] = 0x3e0;
              puVar5[uVar8 * 0x14 + 0x14] = uVar2 + 1;
            }
            else if (puVar5[uVar8 * 0x14 + 1] != 0x3e0) {
              local_d4 = 0;
              do {
                uVar9 = local_d4 + 1;
                local_d4 = uVar9 & 0xffff;
                if ((ushort)uVar9 == uVar2) {
                  if (uVar2 != 0x13) goto LAB_0071c277;
                  pIVar7->m_iChangeInfoSerialNumber = 0;
                  pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar5[uVar8 * 0x14 + local_d4 + 1] != 0x3e0);
            }
          }
          else if (puVar5[0x7d1] == 100) {
            pIVar7->m_iChangeInfoSerialNumber = 0;
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar7->m_iChangeInfo = puVar5[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x52b227) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x52b227) + 0xfa2) + 1;
            piVar6 = *(int **)(unaff_EBX + 0x52b227);
            pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
            iVar3 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar3 + 2) = 0x3e0;
            *(undefined2 *)(iVar3 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->super_CBaseAnimating).m_nSkin.m_Value = 3;
    }
    local_9c = *(float *)(unaff_EBX + 0x3b2437);
    local_50 = *(vec_t *)(unaff_EBX + 0x3b9b47);
    local_54 = *(float *)(unaff_EBX + 0x3b9b4b);
    local_58 = *(vec_t *)(unaff_EBX + 0x3b9b4f);
  }
  else {
    this->m_nValue = 0x19;
    if ((this->super_CBaseAnimating).m_nSkin.m_Value != 4) {
      if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
          false) {
        pCVar4 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
          pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
          puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x52b227);
          if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
            uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar2 = puVar5[uVar8 * 0x14 + 0x14];
            if (uVar2 == 0) {
LAB_0071c1c6:
              puVar5[(uint)uVar2 + uVar8 * 0x14 + 1] = 0x3e0;
              puVar5[uVar8 * 0x14 + 0x14] = uVar2 + 1;
            }
            else if (puVar5[uVar8 * 0x14 + 1] != 0x3e0) {
              local_d0 = 0;
              do {
                uVar9 = local_d0 + 1;
                local_d0 = uVar9 & 0xffff;
                if ((ushort)uVar9 == uVar2) {
                  if (uVar2 != 0x13) goto LAB_0071c1c6;
                  pIVar7->m_iChangeInfoSerialNumber = 0;
                  pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar5[uVar8 * 0x14 + local_d0 + 1] != 0x3e0);
            }
          }
          else if (puVar5[0x7d1] == 100) {
            pIVar7->m_iChangeInfoSerialNumber = 0;
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar7->m_iChangeInfo = puVar5[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x52b227) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x52b227) + 0xfa2) + 1;
            piVar6 = *(int **)(unaff_EBX + 0x52b227);
            pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
            iVar3 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar3 + 2) = 0x3e0;
            *(undefined2 *)(iVar3 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->super_CBaseAnimating).m_nSkin.m_Value = 4;
    }
    local_9c = *(float *)(unaff_EBX + 0x3b0fcf);
    local_50 = *(vec_t *)(unaff_EBX + 0x3b165f);
    local_54 = *(float *)(unaff_EBX + 0x3b9b53);
    local_58 = *(vec_t *)(unaff_EBX + 0x3b34bf);
  }
  CBaseEntity::SetMoveType((CBaseEntity *)this,MOVETYPE_NOCLIP,MOVECOLLIDE_DEFAULT);
  this_00 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Collision;
  CCollisionProperty::SetSolid(&this_00->super_CCollisionProperty,SOLID_BBOX);
  CCollisionProperty::SetSolidFlags(&this_00->super_CCollisionProperty,0xc);
  CBaseEntity::SetCollisionGroup((CBaseEntity *)this,5);
  CBaseAnimating::SetModelScale(&this->super_CBaseAnimating,local_9c,0.0,HIERARCHICAL_MODEL_SCALE);
  local_28[0].x = 0.0;
  local_28[0].y = 0.0;
  local_28[0].z = 180.0;
  CBaseEntity::ApplyLocalAngularVelocityImpulse((CBaseEntity *)this,local_28);
  if (((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
                    /* Unresolved local var: Vector res@[???] */
  local_34.x = (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.x + 0.0;
  local_34.y = (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.y + 0.0;
  local_34.z = local_54 + (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.z;
  CBaseEntity::SetAbsOrigin((CBaseEntity *)this,&local_34);
                    /* Unresolved local var: Vector res@[???] */
  local_4c.x = local_50;
  local_4c.y = local_50;
  local_4c.z = local_58;
                    /* Unresolved local var: Vector res@[???] */
  local_40.x = local_54;
  local_40.y = local_54;
  local_40.z = 0.0;
  CBaseEntity::SetSize((CBaseEntity *)this,&local_40,&local_4c);
  CBaseAnimating::Spawn(&this->super_CBaseAnimating);
  CNuggetDirector::RegisterNugget((CNuggetDirector *)(unaff_EBX + 0x730c3f),this);
  return;
}


/* CItem_Nugget::UpdateOnRemove at 0071adb0 */

/* DWARF original prototype: void UpdateOnRemove(CItem_Nugget * this) */

void __thiscall CItem_Nugget::UpdateOnRemove(CItem_Nugget *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CNuggetDirector::UnregisterNugget((CNuggetDirector *)(&UNK_007317bf + unaff_EBX),this);
  CBaseEntity::UpdateOnRemove((CBaseEntity *)this);
  return;
}


/* CItem_Nugget::Precache at 0071acc0 */

/* DWARF original prototype: void Precache(CItem_Nugget * this) */

void __thiscall CItem_Nugget::Precache(CItem_Nugget *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x3961d8));
  CBaseEntity::PrecacheScriptSound(&UNK_003961fd + unaff_EBX);
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x39620a));
  return;
}


/* CItem_Nugget::StartTouch at 0071aed0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void StartTouch(CItem_Nugget * this, CBaseEntity * pOther) */

void __thiscall CItem_Nugget::StartTouch(CItem_Nugget *this,CBaseEntity *pOther)

{
  edict_t *peVar1;
  int iVar2;
  char *sz;
  soundlevel_t sVar3;
  CTeam *pCVar4;
  int unaff_EBX;
  float local_80;
  CRecipientFilter local_74;
  CRecipientFilter local_54;
  Vector local_34;
  Vector local_28 [2];
  
                    /* Unresolved local var: CBasePlayer * pPlayer@[???]
                       Unresolved local var: CSingleUserRecipientFilter user@[???]
                       Unresolved local var: CPASAttenuationFilter filter@[???] */
  ___i686_get_pc_thunk_bx();
  if (((this->super_CBaseAnimating).super_CBaseEntity.m_fEffects.m_Value & 0x20) == 0) {
    iVar2 = (*(pOther->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[0x55])(pOther);
    if ((char)iVar2 != '\0') {
      CRecipientFilter::CRecipientFilter(&local_54);
      local_54.super_IRecipientFilter._vptr_IRecipientFilter =
           (_func_int_varargs **)(unaff_EBX + 0x52e201);
      CRecipientFilter::AddRecipient(&local_54,(CBasePlayer *)pOther);
      CRecipientFilter::MakeReliable(&local_54);
      UserMessageBegin(&local_54.super_IRecipientFilter,(char *)(unaff_EBX + 0x344c2b));
      sz = (this->super_CBaseAnimating).super_CBaseEntity.m_iClassname.pszValue;
      if (sz == (char *)0x0) {
        sz = (char *)(unaff_EBX + 0x333e3d);
      }
      MessageWriteString(sz);
      MessageEnd();
      (*(pOther->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
        [0x9e])(local_28,pOther);
      CRecipientFilter::CRecipientFilter(&local_74);
      local_74.super_IRecipientFilter._vptr_IRecipientFilter =
           (_func_int_varargs **)(unaff_EBX + 0x52e281);
      CRecipientFilter::AddRecipientsByPAS(&local_74,local_28);
      local_74.super_IRecipientFilter._vptr_IRecipientFilter =
           (_func_int_varargs **)(unaff_EBX + 0x52e241);
      sVar3 = CBaseEntity::LookupSoundLevel((char *)(unaff_EBX + 0x395fe2));
      if ((int)sVar3 < 0x33) {
        if (sVar3 == SNDLVL_NONE) {
          local_80 = 0.0;
        }
        else {
          local_80 = *(float *)(unaff_EBX + 0x3b1c51);
        }
      }
      else {
        local_80 = *(float *)(unaff_EBX + 0x3b1c4d) / (float)(int)(sVar3 - SNDLVL_50dB);
      }
      (*(pOther->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
        [0x9e])(&local_34,pOther);
      CPASAttenuationFilter::Filter((CPASAttenuationFilter *)&local_74,&local_34,local_80);
      peVar1 = (pOther->m_Network).m_pPev;
                    /* Unresolved local var: int edictIndex@[???] */
      iVar2 = 0;
      if (peVar1 != (edict_t *)0x0) {
        iVar2 = (int)peVar1 - *(int *)(**(int **)(unaff_EBX + 0x52bc7d) + 0x58) >> 4;
      }
      CBaseEntity::EmitSound
                (&local_74.super_IRecipientFilter,iVar2,(char *)(unaff_EBX + 0x395fe2),(Vector *)0x0
                 ,0.0,(float *)0x0);
      iVar2 = this->m_nValue;
      CBasePlayer::IncrementFragCount((CBasePlayer *)pOther,iVar2);
      pCVar4 = CBaseEntity::GetTeam(pOther);
      if (pCVar4 != (CTeam *)0x0) {
        pCVar4 = CBaseEntity::GetTeam(pOther);
        iVar2 = this->m_nValue;
        (*(pCVar4->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
          _vptr_IHandleEntity[0xd3])(pCVar4);
      }
      if (this->m_fRespawnTime <= 0.0) {
        UTIL_Remove((CBaseEntity *)this);
      }
      else {
        (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
          super_IHandleEntity._vptr_IHandleEntity[0x77])(this);
      }
      local_74.super_IRecipientFilter._vptr_IRecipientFilter =
           (_func_int_varargs **)(unaff_EBX + 0x52e281);
      CRecipientFilter::~CRecipientFilter(&local_74,iVar2);
      local_54.super_IRecipientFilter._vptr_IRecipientFilter =
           (_func_int_varargs **)(unaff_EBX + 0x52e201);
      CRecipientFilter::~CRecipientFilter(&local_54,iVar2);
    }
  }
  return;
}


/* CItem_Nugget::Respawn at 0071ac30 */

/* DWARF original prototype: CBaseEntity * Respawn(CItem_Nugget * this) */

CBaseEntity * __thiscall CItem_Nugget::Respawn(CItem_Nugget *this)

{
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::AddEffects((CBaseEntity *)this,0x20);
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0x583);
  CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(**(int **)(unaff_EBX + 0x52bf27) + 0xc) + this->m_fRespawnTime,(char *)0x0);
  return (CBaseEntity *)this;
}


/* CItem_Nugget::MaterializeThink at 0071b1c0 */

/* DWARF original prototype: void MaterializeThink(CItem_Nugget * this) */

void __thiscall CItem_Nugget::MaterializeThink(CItem_Nugget *this)

{
  undefined1 *puVar1;
  int iVar2;
  ushort uVar3;
  CBaseEdict *this_00;
  edict_t *peVar4;
  ushort *puVar5;
  int *piVar6;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  uint uVar9;
  uint uVar10;
  int unaff_EBX;
  uint local_60;
  
  ___i686_get_pc_thunk_bx();
  if (((this->super_CBaseAnimating).super_CBaseEntity.m_fEffects.m_Value & 0x20) == 0) {
    return;
  }
  CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x395d02),0.0,(float *)0x0);
  uVar9 = (this->super_CBaseAnimating).super_CBaseEntity.m_fEffects.m_Value;
  uVar10 = uVar9 & 0xffffffdf;
  if (uVar9 != uVar10) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
      ;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(this_00);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x52b994);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar9 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar9 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_0071b374:
            puVar5[(uint)uVar3 + uVar9 * 0x14 + 1] = 0xc0;
            puVar5[uVar9 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar9 * 0x14 + 1] != 0xc0) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_0071b374;
                goto LAB_0071b399;
              }
            } while (puVar5[uVar9 * 0x14 + local_60 + 1] != 0xc0);
          }
        }
        else if (puVar5[0x7d1] == 100) {
LAB_0071b399:
          pIVar7->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x52b994) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x52b994) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x52b994);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0xc0;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CBaseAnimating).super_CBaseEntity.m_fEffects.m_Value = uVar10;
  }
  peVar4 = (this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev;
  if (peVar4 != (edict_t *)0x0) {
    (peVar4->super_CBaseEdict).m_fStateFlags = (peVar4->super_CBaseEdict).m_fStateFlags | 0x80;
  }
  CBaseEntity::DispatchUpdateTransmitState((CBaseEntity *)this);
  return;
}


/* CNuggetDirector::RegisterNugget at 0071adf0 */

/* DWARF original prototype: void RegisterNugget(CNuggetDirector * this, CItem_Nugget * pNugget) */

void __thiscall CNuggetDirector::RegisterNugget(CNuggetDirector *this,CItem_Nugget *pNugget)

{
  char *s1;
  uint uVar1;
  int iVar2;
  CItem_Nugget **ppCVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  s1 = (pNugget->m_iGroupName).pszValue;
  if (((s1 == (char *)0x0) || (s1 == (char *)(unaff_EBX + 0x333f20))) ||
     (iVar2 = _V_stricmp(s1,(char *)(unaff_EBX + 0x333f20)), iVar2 == 0)) {
    uVar1 = (this->m_UngroupedNuggets).m_Size;
    CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>::GrowVector
              (&this->m_UngroupedNuggets,1);
    iVar2 = ~uVar1 + (this->m_UngroupedNuggets).m_Size;
    if (0 < iVar2) {
      ppCVar3 = (this->m_UngroupedNuggets).m_Memory.m_pMemory;
      _V_memmove(ppCVar3 + uVar1 + 1,ppCVar3 + uVar1,iVar2 * 4);
    }
    ppCVar3 = (this->m_UngroupedNuggets).m_Memory.m_pMemory + uVar1;
    if (ppCVar3 != (CItem_Nugget **)0x0) {
      *ppCVar3 = pNugget;
    }
  }
  else {
    CUtlSortVector<CItem_Nugget*,CNuggetDirector::CNuggetGroupSort>::Insert
              (&this->m_GroupedNuggets,&pNugget);
  }
  return;
}


/* CNuggetDirector::UnregisterNugget at 0071ad00 */

/* DWARF original prototype: void UnregisterNugget(CNuggetDirector * this, CItem_Nugget * pNugget)
    */

void __thiscall CNuggetDirector::UnregisterNugget(CNuggetDirector *this,CItem_Nugget *pNugget)

{
  int *piVar1;
  char *s1;
  CItem_Nugget **ppCVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int unaff_EBX;
  int iVar6;
  
  ___i686_get_pc_thunk_bx();
  s1 = (pNugget->m_iGroupName).pszValue;
  if (((s1 != (char *)0x0) && (s1 != (char *)(unaff_EBX + 0x334016))) &&
     (iVar4 = _V_stricmp(s1,(char *)(unaff_EBX + 0x334016)), iVar4 != 0)) {
    CUtlSortVector<CItem_Nugget*,CNuggetDirector::CNuggetGroupSort>::Remove
              (&this->m_GroupedNuggets,&pNugget);
    return;
  }
  iVar4 = (this->m_UngroupedNuggets).m_Size;
  if (iVar4 < 1) {
    return;
  }
  ppCVar2 = (this->m_UngroupedNuggets).m_Memory.m_pMemory;
  if (*ppCVar2 == pNugget) {
    iVar5 = 0;
    iVar6 = 0;
  }
  else {
    iVar5 = 0;
    iVar3 = 4;
    do {
      iVar6 = iVar3;
      iVar5 = iVar5 + 1;
      if (iVar5 == iVar4) {
        return;
      }
      iVar3 = iVar6 + 4;
    } while (ppCVar2[iVar5] != pNugget);
  }
  if (iVar5 != iVar4 + -1) {
    *(CItem_Nugget **)(iVar6 + (int)ppCVar2) = ppCVar2[iVar4 + -1];
  }
  piVar1 = &(this->m_UngroupedNuggets).m_Size;
  *piVar1 = *piVar1 + -1;
  return;
}


/* CNuggetDirector::RandomizeAvailableNuggets at 0071b3b0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void RandomizeAvailableNuggets(CNuggetDirector * this, float
   fUngroupedAvailabilityChance, float fGroupAvailabilityChance) */

void __thiscall
CNuggetDirector::RandomizeAvailableNuggets
          (CNuggetDirector *this,float fUngroupedAvailabilityChance,float fGroupAvailabilityChance)

{
  undefined1 *puVar1;
  int iVar2;
  ushort uVar3;
  int iVar4;
  CBaseEntity *this_00;
  CItem_Nugget *this_01;
  edict_t *peVar5;
  CBaseEdict *pCVar6;
  ushort *puVar7;
  int *piVar8;
  bool bVar9;
  CItem_Nugget **ppCVar10;
  IChangeInfoAccessor *pIVar11;
  uint uVar12;
  uint uVar13;
  uint uVar14;
  int unaff_EBX;
  longdouble lVar15;
  ushort i;
  uint local_a0;
  int local_58;
  int local_54;
  char *local_28;
  int local_24;
  
                    /* Unresolved local var: int iCount@[???]
                       Unresolved local var: string_t iLastGroup@[???]
                       Unresolved local var: bool bExists@[???] */
  ___i686_get_pc_thunk_bx();
  iVar4 = (this->m_UngroupedNuggets).m_Size;
  if (iVar4 != 0) {
    local_58 = 0;
    local_24 = 0;
    do {
      while (lVar15 = (longdouble)_RandomFloat(0,0x3f800000),
            (float)lVar15 <= fUngroupedAvailabilityChance) {
        this_00 = *(CBaseEntity **)((int)(this->m_UngroupedNuggets).m_Memory.m_pMemory + local_24);
        uVar12 = (this_00->m_fEffects).m_Value;
        uVar14 = uVar12 & 0xffffffdf;
        if (uVar12 != uVar14) {
          if ((this_00->m_Network).m_TimerEvent.m_bRegistered == false) {
            pCVar6 = &((this_00->m_Network).m_pPev)->super_CBaseEdict;
            if ((pCVar6 != (CBaseEdict *)0x0) && ((pCVar6->m_fStateFlags & 0x100U) == 0)) {
              pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 1;
              pIVar11 = CBaseEdict::GetChangeAccessor(pCVar6);
              puVar7 = (ushort *)**(undefined4 **)(unaff_EBX + 0x52b7a7);
              if (pIVar11->m_iChangeInfoSerialNumber == *puVar7) {
                uVar12 = (uint)pIVar11->m_iChangeInfo;
                uVar3 = puVar7[uVar12 * 0x14 + 0x14];
                if (uVar3 == 0) {
LAB_0071b860:
                  puVar7[(uint)uVar3 + uVar12 * 0x14 + 1] = 0xc0;
                  puVar7[uVar12 * 0x14 + 0x14] = uVar3 + 1;
                }
                else if (puVar7[uVar12 * 0x14 + 1] != 0xc0) {
                  _i = 0;
                  do {
                    uVar13 = _i + 1;
                    _i = uVar13 & 0xffff;
                    if ((ushort)uVar13 == uVar3) {
                      if (uVar3 != 0x13) goto LAB_0071b860;
                      pIVar11->m_iChangeInfoSerialNumber = 0;
                      pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 0x100;
                      break;
                    }
                  } while (puVar7[uVar12 * 0x14 + _i + 1] != 0xc0);
                }
              }
              else if (puVar7[0x7d1] == 100) {
                pIVar11->m_iChangeInfoSerialNumber = 0;
                pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 0x100;
              }
              else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                pIVar11->m_iChangeInfo = puVar7[0x7d1];
                *(short *)(**(int **)(unaff_EBX + 0x52b7a7) + 0xfa2) =
                     *(short *)(**(int **)(unaff_EBX + 0x52b7a7) + 0xfa2) + 1;
                piVar8 = *(int **)(unaff_EBX + 0x52b7a7);
                pIVar11->m_iChangeInfoSerialNumber = *(ushort *)*piVar8;
                iVar2 = *piVar8 + (uint)pIVar11->m_iChangeInfo * 0x28;
                *(undefined2 *)(iVar2 + 2) = 0xc0;
                *(undefined2 *)(iVar2 + 0x28) = 1;
              }
            }
          }
          else {
            puVar1 = &(this_00->m_Network).field_0x4c;
            *(uint *)puVar1 = *(uint *)puVar1 | 1;
          }
          (this_00->m_fEffects).m_Value = uVar14;
        }
        peVar5 = (this_00->m_Network).m_pPev;
        if (peVar5 != (edict_t *)0x0) {
          (peVar5->super_CBaseEdict).m_fStateFlags = (peVar5->super_CBaseEdict).m_fStateFlags | 0x80
          ;
        }
        CBaseEntity::DispatchUpdateTransmitState(this_00);
        local_58 = local_58 + 1;
        local_24 = local_24 + 4;
        if (local_58 == iVar4) goto LAB_0071b4a1;
      }
      CBaseEntity::AddEffects
                (*(CBaseEntity **)((int)(this->m_UngroupedNuggets).m_Memory.m_pMemory + local_24),
                 0x20);
      local_58 = local_58 + 1;
      local_24 = local_24 + 4;
    } while (local_58 != iVar4);
  }
LAB_0071b4a1:
  iVar4 = (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
          m_Size;
  local_28 = (char *)0x0;
  if (iVar4 != 0) {
    bVar9 = false;
    local_54 = 0;
    do {
      while( true ) {
        ppCVar10 = (this->m_GroupedNuggets).
                   super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.m_Memory.
                   m_pMemory;
        if (local_28 != (ppCVar10[local_54]->m_iGroupName).pszValue) {
          local_28 = (ppCVar10[local_54]->m_iGroupName).pszValue;
          lVar15 = (longdouble)_RandomFloat(0,0x3f800000);
          bVar9 = (float)lVar15 <= fGroupAvailabilityChance;
          ppCVar10 = (this->m_GroupedNuggets).
                     super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.m_Memory.
                     m_pMemory;
        }
        if (!bVar9) break;
        this_01 = ppCVar10[local_54];
        uVar12 = (this_01->super_CBaseAnimating).super_CBaseEntity.m_fEffects.m_Value;
        uVar14 = uVar12 & 0xffffffdf;
        if (uVar12 != uVar14) {
          if (*(bool *)((int)&(this_01->super_CBaseAnimating).super_CBaseEntity.m_Network.
                              m_TimerEvent + 0x10) == false) {
            pCVar6 = &((this_01->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                      super_CBaseEdict;
            if ((pCVar6 != (CBaseEdict *)0x0) && ((pCVar6->m_fStateFlags & 0x100U) == 0)) {
              pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 1;
              pIVar11 = CBaseEdict::GetChangeAccessor(pCVar6);
              puVar7 = (ushort *)**(undefined4 **)(unaff_EBX + 0x52b7a7);
              if (pIVar11->m_iChangeInfoSerialNumber == *puVar7) {
                uVar12 = (uint)pIVar11->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
                uVar3 = puVar7[uVar12 * 0x14 + 0x14];
                if (uVar3 == 0) {
LAB_0071b7b4:
                  puVar7[(uint)uVar3 + uVar12 * 0x14 + 1] = 0xc0;
                  puVar7[uVar12 * 0x14 + 0x14] = uVar3 + 1;
                }
                else if (puVar7[uVar12 * 0x14 + 1] != 0xc0) {
                  local_a0 = 0;
                  do {
                    uVar13 = local_a0 + 1;
                    local_a0 = uVar13 & 0xffff;
                    if ((ushort)uVar13 == uVar3) {
                      if (uVar3 != 0x13) goto LAB_0071b7b4;
                      pIVar11->m_iChangeInfoSerialNumber = 0;
                      pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 0x100;
                      break;
                    }
                  } while (puVar7[uVar12 * 0x14 + local_a0 + 1] != 0xc0);
                }
              }
              else if (puVar7[0x7d1] == 100) {
                pIVar11->m_iChangeInfoSerialNumber = 0;
                pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 0x100;
              }
              else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                pIVar11->m_iChangeInfo = puVar7[0x7d1];
                *(short *)(**(int **)(unaff_EBX + 0x52b7a7) + 0xfa2) =
                     *(short *)(**(int **)(unaff_EBX + 0x52b7a7) + 0xfa2) + 1;
                piVar8 = *(int **)(unaff_EBX + 0x52b7a7);
                pIVar11->m_iChangeInfoSerialNumber = *(ushort *)*piVar8;
                iVar2 = *piVar8 + (uint)pIVar11->m_iChangeInfo * 0x28;
                *(undefined2 *)(iVar2 + 2) = 0xc0;
                *(undefined2 *)(iVar2 + 0x28) = 1;
              }
            }
          }
          else {
            puVar1 = &(this_01->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
            *(uint *)puVar1 = *(uint *)puVar1 | 1;
          }
          (this_01->super_CBaseAnimating).super_CBaseEntity.m_fEffects.m_Value = uVar14;
        }
        peVar5 = (this_01->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev;
        if (peVar5 != (edict_t *)0x0) {
          (peVar5->super_CBaseEdict).m_fStateFlags = (peVar5->super_CBaseEdict).m_fStateFlags | 0x80
          ;
        }
        CBaseEntity::DispatchUpdateTransmitState((CBaseEntity *)this_01);
        local_54 = local_54 + 1;
        if (local_54 == iVar4) {
          return;
        }
      }
      CBaseEntity::AddEffects((CBaseEntity *)ppCVar10[local_54],0x20);
      local_54 = local_54 + 1;
    } while (local_54 != iVar4);
  }
  return;
}


/* __tcf_2 at 00a3ce50 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38b7a8),in_stack_00000008);
  return;
}


/* __tcf_3 at 00a3ce30 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38b828),in_stack_00000008);
  return;
}


/* __tcf_4 at 00a3ce10 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38b8a8),in_stack_00000008);
  return;
}


/* CNuggetDirector::LevelInitPostEntity at 0071b8e0 */

/* DWARF original prototype: void LevelInitPostEntity(CNuggetDirector * this) */

void __thiscall CNuggetDirector::LevelInitPostEntity(CNuggetDirector *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(int *)(*(int *)(unaff_EBX + 0x6acd30) + 0x30) != 0) {
    RandomizeAvailableNuggets
              (this,*(float *)(*(int *)(unaff_EBX + 0x6acd90) + 0x2c),
               *(float *)(*(int *)(unaff_EBX + 0x6acdf0) + 0x2c));
  }
  return;
}


/* _GLOBAL__I__ZN12CItem_Nugget9m_DataMapE at 000ab610 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN12CItem_Nugget9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

