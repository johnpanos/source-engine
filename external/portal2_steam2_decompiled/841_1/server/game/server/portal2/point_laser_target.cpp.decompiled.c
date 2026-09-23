/* DWARF-guided pseudocode for game/server/portal2/point_laser_target.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* DataMapInit<CPortalLaserTarget> at 000afce0 */

datamap_t * DataMapInit<CPortalLaserTarget>(CPortalLaserTarget *param_1)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  char *pcVar4;
  size_t sVar5;
  int iVar6;
  undefined4 *puVar7;
  int iVar8;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xc5].m_pPrev + unaff_EBX) ==
       '\0') && (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xd9e83c), iVar3 != 0)) {
    *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xc6].m_pPrev + unaff_EBX) =
         unaff_EBX + 0xa02f7d;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[199].m_pEntity + unaff_EBX) =
         0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[199].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[199].m_pPrev + unaff_EBX) = 0
    ;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[199].m_pNext + unaff_EBX) = 0
    ;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[200].m_pEntity + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xc6].m_pNext + unaff_EBX) =
         0x12;
    ___cxa_guard_release(unaff_EBX + 0xd9e83c);
    ___cxa_atexit(unaff_EBX + 0x98e78c,0,*(undefined4 *)(&DAT_00b96e60 + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_CreateCorpse_00cb66b8 + unaff_EBX) =
       *(undefined4 *)(&DAT_00b96f98 + unaff_EBX);
  if ((*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xc6].m_pEntity + unaff_EBX) ==
       '\0') && (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xd9e844), iVar3 != 0)) {
    pcVar4 = operator_new___(*(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xc6].
                                            m_pNext + unaff_EBX) + 0xd);
    _strcpy(pcVar4,*(char **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xc6].m_pPrev +
                             unaff_EBX));
    sVar5 = _strlen(pcVar4);
    builtin_strncpy(pcVar4 + sVar5,"DisableThink",0xd);
    iVar1 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[199].m_pNext + unaff_EBX);
    iVar3 = iVar1 + 1;
    iVar8 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[199].m_SerialNumber +
                    unaff_EBX);
    if (iVar8 < iVar3) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)
                 ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[199].m_pEntity + unaff_EBX),
                 iVar3 - iVar8);
    }
    iVar6 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[199].m_pNext + unaff_EBX) +
            1;
    *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[199].m_pNext + unaff_EBX) = iVar6;
    iVar8 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[199].m_pEntity + unaff_EBX);
    *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[200].m_pEntity + unaff_EBX) = iVar8;
    iVar6 = (iVar6 - iVar1) + -1;
    if (0 < iVar6) {
      _V_memmove((void *)(iVar8 + iVar3 * 4),(void *)(iVar8 + iVar1 * 4),iVar6 * 4);
      iVar8 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[199].m_pEntity + unaff_EBX
                      );
    }
    puVar7 = (undefined4 *)(iVar8 + iVar1 * 4);
    if (puVar7 != (undefined4 *)0x0) {
      *puVar7 = pcVar4;
    }
    *(char **)((int)&PTR_typeinfo_00cb6990 + unaff_EBX) = pcVar4;
    uVar2 = **(undefined4 **)(&DAT_00b96e80 + unaff_EBX);
    *(undefined4 *)((int)&PTR_GetPlayerInfo_00cb6ae0 + unaff_EBX) = uVar2;
    *(undefined4 *)((int)&PTR_typeinfo_name_00cb6b20 + unaff_EBX) = uVar2;
    ___cxa_guard_release(unaff_EBX + 0xd9e844);
  }
  *(undefined4 *)((int)&PTR_GetDelayTicks_00cb66b0 + unaff_EBX) = 7;
  *(int *)((int)&PTR_StopReplayMode_00cb66ac + unaff_EBX) =
       (int)&PTR_typeinfo_name_00cb698c + unaff_EBX;
  return (datamap_t *)((int)&PTR_StopReplayMode_00cb66ac + unaff_EBX);
}


/* __static_initialization_and_destruction_0 at 000aff00 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  IEntityFactoryDictionary *pIVar1;
  datamap_t *pdVar2;
  int unaff_EBX;
  longlong lVar3;
  
  lVar3 = ___i686_get_pc_thunk_bx();
  if (lVar3 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x95].m_pEntity + unaff_EBX) =
       0;
  *(undefined1 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x95].m_pEntity + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x95].m_pEntity + unaff_EBX + 2) = 0;
  *(undefined1 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x95].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x95].m_SerialNumber + unaff_EBX) = 0;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x95].m_pPrev + unaff_EBX) = 0;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x95].m_pNext + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x96].m_pEntity + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x96].m_SerialNumber + unaff_EBX) = 0;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x96].m_pPrev + unaff_EBX) = 0;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x96].m_pNext + unaff_EBX) = 0;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x97].m_pEntity + unaff_EBX) =
       0;
  *(undefined4 *)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x97].m_SerialNumber + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x97].m_pPrev + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x97].m_pNext + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x98].m_pEntity + unaff_EBX) =
       0x7f7fffff;
  *(undefined **)
   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x98].m_SerialNumber + unaff_EBX) =
       &UNK_00b9c3fc + unaff_EBX;
  *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x98].m_pPrev + unaff_EBX) =
       &UNK_00c1fbb4 + unaff_EBX;
  pIVar1 = EntityFactoryDictionary();
  (**pIVar1->_vptr_IEntityFactoryDictionary)(pIVar1,unaff_EBX + 0xd9e56c,unaff_EBX + 0xa02c58);
  pdVar2 = DataMapInit<CPortalLaserTarget>((CPortalLaserTarget *)0x0);
  *(datamap_t **)((int)DataMapInit<AI_LeadArgs_t>::dataDesc[6].flatOffset + unaff_EBX + -0x24) =
       pdVar2;
  *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x98].m_pNext + unaff_EBX) =
       &UNK_00c1fbd4 + unaff_EBX;
  pIVar1 = EntityFactoryDictionary();
  (**pIVar1->_vptr_IEntityFactoryDictionary)(pIVar1,unaff_EBX + 0xd9e570,unaff_EBX + 0xa02d78);
  pdVar2 = DataMapInit<CLaserCatcher>((CLaserCatcher *)0x0);
  *(datamap_t **)((int)DataMapInit<AI_LeadArgs_t>::dataDesc[6].flatOffset + unaff_EBX + -0x20) =
       pdVar2;
  *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x99].m_pEntity + unaff_EBX) =
       &UNK_00c1fbf4 + unaff_EBX;
  pIVar1 = EntityFactoryDictionary();
  (**pIVar1->_vptr_IEntityFactoryDictionary)(pIVar1,unaff_EBX + 0xd9e574,unaff_EBX + 0xa02d8b);
  pdVar2 = DataMapInit<CLaserRelay>((CLaserRelay *)0x0);
  *(datamap_t **)((int)DataMapInit<AI_LeadArgs_t>::dataDesc[6].flatOffset + unaff_EBX + -0x1c) =
       pdVar2;
  return;
}


/* CPortalLaserTarget::GetDataDescMap at 007406e0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPortalLaserTarget * this) */

datamap_t * __thiscall CPortalLaserTarget::GetDataDescMap(CPortalLaserTarget *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x625cb8);
}


/* CPortalLaserTarget::GetBaseMap at 007406f0 */

datamap_t * CPortalLaserTarget::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x506594);
}


/* __tcf_0 at 00a3e480 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x4100c6)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x4100ba) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x4100c6));
  }
  *(undefined4 *)(unaff_EBX + 0x4100c6) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x4100c2)) {
    if (*(int *)(unaff_EBX + 0x4100ba) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2086ce) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2086ce),*(int *)(unaff_EBX + 0x4100ba));
      *(undefined4 *)(unaff_EBX + 0x4100ba) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x4100be) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x4100ba);
  *(int *)(unaff_EBX + 0x4100ca) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x4100c2)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2086ce) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2086ce),iVar1);
      *(undefined4 *)(unaff_EBX + 0x4100ba) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x4100be) = 0;
  }
  return;
}


/* CPortalLaserTarget::Spawn at 00740de0 */

/* DWARF original prototype: void Spawn(CPortalLaserTarget * this) */

void __thiscall CPortalLaserTarget::Spawn(CPortalLaserTarget *this)

{
  Vector local_24;
  Vector local_18;
  
                    /* Unresolved local var: float flWidth@[???] */
  CBaseEntity::AddFlag(&this->super_CBaseEntity,0x2000000);
  this->m_bPowered = false;
  local_24.x = 6.0;
  local_24.y = 6.0;
  local_24.z = 6.0;
  local_18.x = -6.0;
  local_18.y = -6.0;
  local_18.z = -6.0;
  UTIL_SetSize(&this->super_CBaseEntity,&local_18,&local_24);
  CBaseEntity::Spawn(&this->super_CBaseEntity);
  return;
}


/* CPortalLaserTarget::OnTakeDamage at 00740c40 */

/* DWARF original prototype: int OnTakeDamage(CPortalLaserTarget * this, CTakeDamageInfo * info) */

int __thiscall CPortalLaserTarget::OnTakeDamage(CPortalLaserTarget *this,CTakeDamageInfo *info)

{
  uint uVar1;
  CBaseEntity *this_00;
  CCatcher *pCVar2;
  BASEPTR func;
  bool bVar3;
  int iVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_bPowered == false) {
    uVar1 = (info->m_hAttacker).super_CBaseHandle.m_Index;
    if (uVar1 == 0xffffffff) {
      return 0;
    }
    iVar4 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x505f18);
    if (*(uint *)(iVar4 + 8) != uVar1 >> 0x10) {
      return 0;
    }
    this_00 = *(CBaseEntity **)(iVar4 + 4);
    if (this_00 == (CBaseEntity *)0x0) {
      return 0;
    }
    if (((this_00->m_iClassname).pszValue != (char *)(unaff_EBX + 0x36f89e)) &&
       (bVar3 = CBaseEntity::ClassMatchesComplex(this_00,(char *)(unaff_EBX + 0x36f89e)), !bVar3)) {
      return 0;
    }
    this->m_bPowered = true;
    COutputEvent::FireOutput
              (&this->m_OnPowered,&this->super_CBaseEntity,&this->super_CBaseEntity,0.0);
    pCVar2 = this->m_pCatcher;
    if (pCVar2 != (CCatcher *)0x0) {
      (**(pCVar2->super_ICatcher)._vptr_ICatcher)(pCVar2);
    }
    func.__delta = 0;
    func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x1b4);
    CBaseEntity::ThinkSet(&this->super_CBaseEntity,func,0.0,(char *)0x0);
  }
  CBaseEntity::SetNextThink
            (&this->super_CBaseEntity,
             *(float *)(unaff_EBX + 0x38bcc0) + *(float *)(**(int **)(unaff_EBX + 0x505f10) + 0xc),
             (char *)0x0);
  return 0;
}


/* CPortalLaserTarget::DisableThink at 00740aa0 */

/* DWARF original prototype: void DisableThink(CPortalLaserTarget * this) */

void __thiscall CPortalLaserTarget::DisableThink(CPortalLaserTarget *this)

{
  CCatcher *pCVar1;
  
  this->m_bPowered = false;
  COutputEvent::FireOutput
            (&this->m_OnUnpowered,&this->super_CBaseEntity,&this->super_CBaseEntity,0.0);
  pCVar1 = this->m_pCatcher;
  if (pCVar1 != (CCatcher *)0x0) {
    (*(pCVar1->super_ICatcher)._vptr_ICatcher[1])(pCVar1);
  }
  CBaseEntity::ThinkSet(&this->super_CBaseEntity,(BASEPTR)0x0,0.0,(char *)0x0);
  return;
}


/* CPortalLaserTarget::IsPowered at 00740700 */

/* DWARF original prototype: bool IsPowered(CPortalLaserTarget * this) */

bool __thiscall CPortalLaserTarget::IsPowered(CPortalLaserTarget *this)

{
  return this->m_bPowered;
}


/* CLaserCatcher::OnRestore at 00740e60 */

/* DWARF original prototype: void OnRestore(CLaserCatcher * this) */

void __thiscall CLaserCatcher::OnRestore(CLaserCatcher *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::OnRestore(&this->super_CBaseAnimating);
  if ((this->m_pCatcherLaserTarget != (CPortalLaserTarget *)0x0) &&
     (this->m_pCatcherLaserTarget->m_pCatcher = &this->super_CCatcher,
     this->m_pCatcherLaserTarget->m_bPowered != false)) {
    CBaseEntity::EmitSound((CBaseEntity *)this,&UNK_00371d14 + unaff_EBX,0.0,(float *)0x0);
    DispatchParticleEffect
              (&UNK_00371d31 + unaff_EBX,PATTACH_POINT_FOLLOW,(CBaseEntity *)this,
               (char *)(CHintCriteria::~CHintCriteria + unaff_EBX + 2),false,-1,
               (IRecipientFilter *)0x0);
    return;
  }
  return;
}


/* DataMapInit<CLaserCatcher> at 000afab0 */

datamap_t * DataMapInit<CLaserCatcher>(CLaserCatcher *param_1)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  char *pcVar4;
  size_t sVar5;
  int iVar6;
  undefined4 *puVar7;
  int iVar8;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xe5].m_pPrev + unaff_EBX) ==
       '\0') && (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xd9ea3c), iVar3 != 0)) {
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xe6].m_pPrev + unaff_EBX) =
         &UNK_00a0319f + unaff_EBX;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xe7].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xe7].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xe7].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xe7].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xe8].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xe6].m_pNext + unaff_EBX) =
         0xd;
    ___cxa_guard_release(unaff_EBX + 0xd9ea3c);
    ___cxa_atexit(unaff_EBX + 0x98e8dc,0,*(undefined4 *)(&DAT_00b97090 + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_typeinfo_name_00cb6900 + unaff_EBX) =
       *(undefined4 *)(&DAT_00b9709c + unaff_EBX);
  if ((*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xe6].m_pEntity + unaff_EBX) ==
       '\0') && (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xd9ea44), iVar3 != 0)) {
    pcVar4 = operator_new___(*(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xe6].
                                            m_pNext + unaff_EBX) + 0xd);
    _strcpy(pcVar4,*(char **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xe6].m_pPrev +
                             unaff_EBX));
    sVar5 = _strlen(pcVar4);
    builtin_strncpy(pcVar4 + sVar5,"AnimateThink",0xd);
    iVar1 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xe7].m_pNext + unaff_EBX);
    iVar3 = iVar1 + 1;
    iVar8 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xe7].m_SerialNumber +
                    unaff_EBX);
    if (iVar8 < iVar3) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)
                 ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xe7].m_pEntity + unaff_EBX),
                 iVar3 - iVar8);
    }
    iVar6 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xe7].m_pNext + unaff_EBX) +
            1;
    *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xe7].m_pNext + unaff_EBX) = iVar6;
    iVar8 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xe7].m_pEntity + unaff_EBX)
    ;
    *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xe8].m_pEntity + unaff_EBX) = iVar8
    ;
    iVar6 = (iVar6 - iVar1) + -1;
    if (0 < iVar6) {
      _V_memmove((void *)(iVar8 + iVar3 * 4),(void *)(iVar8 + iVar1 * 4),iVar6 * 4);
      iVar8 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xe7].m_pEntity +
                      unaff_EBX);
    }
    puVar7 = (undefined4 *)(iVar8 + iVar1 * 4);
    if (puVar7 != (undefined4 *)0x0) {
      *puVar7 = pcVar4;
    }
    *(char **)(&DAT_00cb69c0 + unaff_EBX) = pcVar4;
    uVar2 = **(undefined4 **)(&DAT_00b970b0 + unaff_EBX);
    *(undefined4 *)(&(anonymous_namespace)::IPlayerInfoManager_V1::vtable + unaff_EBX) = uVar2;
    *(undefined4 *)((int)&PTR_typeinfo_name_00cb6b50 + unaff_EBX) = uVar2;
    ___cxa_guard_release(unaff_EBX + 0xd9ea44);
  }
  *(undefined4 *)((int)&PTR_GetEntitySize_00cb68f8 + unaff_EBX) = 8;
  *(int *)((int)&PTR_Destroy_00cb68f4 + unaff_EBX) =
       (int)&PTR_NetworkStateChanged_00cb697c + unaff_EBX;
  return (datamap_t *)((int)&PTR_Destroy_00cb68f4 + unaff_EBX);
}


/* CLaserCatcher::GetDataDescMap at 00740710 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CLaserCatcher * this) */

datamap_t * __thiscall CLaserCatcher::GetDataDescMap(CLaserCatcher *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x625ca0);
}


/* CLaserCatcher::GetBaseMap at 00740720 */

datamap_t * CLaserCatcher::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x506438);
}


/* __tcf_1 at 00a3e3a0 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x410176)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41016a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x410176));
  }
  *(undefined4 *)(unaff_EBX + 0x410176) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x410172)) {
    if (*(int *)(unaff_EBX + 0x41016a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2087ae) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2087ae),*(int *)(unaff_EBX + 0x41016a));
      *(undefined4 *)(unaff_EBX + 0x41016a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41016e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41016a);
  *(int *)(unaff_EBX + 0x41017a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x410172)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2087ae) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2087ae),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41016a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41016e) = 0;
  }
  return;
}


/* CLaserCatcher::CLaserCatcher at 00740bb0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CLaserCatcher(CLaserCatcher * this) */

void __thiscall CLaserCatcher::CLaserCatcher(CLaserCatcher *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::CBaseAnimating(&this->super_CBaseAnimating);
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x58e767);
  (this->super_CCatcher).super_ICatcher._vptr_ICatcher =
       (_func_int_varargs **)(unaff_EBX + 0x58eb3b);
  (this->m_OnPowered).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnPowered).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_OnPowered).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_OnUnpowered).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnUnpowered).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_OnUnpowered).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  this->m_iPowerState = 0;
  return;
}


/* CLaserCatcher::CLaserCatcher at 00740c30 */

/* DWARF original prototype: void CLaserCatcher(CLaserCatcher * this, CLaserCatcher * this) */

void __thiscall CLaserCatcher::CLaserCatcher(CLaserCatcher *this,CLaserCatcher *this_1)

{
  CLaserCatcher(this);
  return;
}


/* CEntityFactory<CLaserCatcher>::Create at 00741da0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CLaserCatcher> * this, char
   * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CLaserCatcher>::Create(CEntityFactory<CLaserCatcher> *this,char *pClassName)

{
  CLaserCatcher *this_00;
  
                    /* Unresolved local var: CLaserCatcher * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x55c);
  CLaserCatcher::CLaserCatcher(this_00);
  (*(this_00->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1d])(this_00,pClassName);
  return &(this_00->super_CBaseAnimating).super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CLaserCatcher::Precache at 00740b40 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Precache(CLaserCatcher * this) */

void __thiscall CLaserCatcher::Precache(CLaserCatcher *this)

{
  char *name;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  name = (char *)(*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.
                   super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xef])(this);
  CBaseEntity::PrecacheModel(name);
  PrecacheParticleSystem(&UNK_00372055 + unaff_EBX);
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x372069));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x372084));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x372038));
  CBaseAnimating::Precache(&this->super_CBaseAnimating);
  return;
}


/* CLaserCatcher::Spawn at 00740f20 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "MoveCollide_t": Some values do not have unique names */
/* WARNING: Enum "MoveType_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(CLaserCatcher * this) */

void __thiscall CLaserCatcher::Spawn(CLaserCatcher *this)

{
  undefined1 *puVar1;
  ushort uVar2;
  _func_int_varargs **pp_Var3;
  _func_int_varargs *p_Var4;
  CBaseEdict *this_00;
  ushort *puVar5;
  int *piVar6;
  int iVar7;
  IChangeInfoAccessor *pIVar8;
  uint uVar9;
  uint uVar10;
  int unaff_EBX;
  uint local_60;
  
  ___i686_get_pc_thunk_bx();
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1a])(this);
  CBaseAnimating::Spawn(&this->super_CBaseAnimating);
  pp_Var3 = (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
            super_IHandleEntity._vptr_IHandleEntity;
  p_Var4 = pp_Var3[0x1b];
  iVar7 = (*pp_Var3[0xef])(this);
  (*p_Var4)(this,iVar7);
  CCollisionProperty::SetSolid
            (&(this->super_CBaseAnimating).super_CBaseEntity.m_Collision.super_CCollisionProperty,
             SOLID_VPHYSICS);
  CBaseEntity::SetMoveType((CBaseEntity *)this,MOVETYPE_VPHYSICS,MOVECOLLIDE_DEFAULT);
  iVar7 = CBaseAnimating::LookupSequence(&this->super_CBaseAnimating,(char *)(unaff_EBX + 0x36b126))
  ;
  this->m_IdleSequence = iVar7;
  iVar7 = CBaseAnimating::LookupSequence(&this->super_CBaseAnimating,&UNK_00371d17 + unaff_EBX);
  this->m_PowerOnSequence = iVar7;
  iVar7 = CBaseAnimating::LookupAttachment(&this->super_CBaseAnimating,&UNK_00371d1c + unaff_EBX);
  this->m_iTargetAttachment = iVar7;
  CBaseAnimating::ResetSequence(&this->super_CBaseAnimating,this->m_PowerOnSequence);
  (this->super_CBaseAnimating).m_bSequenceFinished = false;
  if ((this->super_CBaseAnimating).m_flPlaybackRate.m_Value != 0.0) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
      ;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar8 = CBaseEdict::GetChangeAccessor(this_00);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x505c3a);
        if (pIVar8->m_iChangeInfoSerialNumber == *puVar5) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar9 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar5[uVar9 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_00741154:
            puVar5[(uint)uVar2 + uVar9 * 0x14 + 1] = 0x3f0;
            puVar5[uVar9 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar5[uVar9 * 0x14 + 1] != 0x3f0) {
            local_60 = 0;
            do {
              uVar10 = local_60 + 1;
              local_60 = uVar10 & 0xffff;
              if ((ushort)uVar10 == uVar2) {
                if (uVar2 != 0x13) goto LAB_00741154;
                pIVar8->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar9 * 0x14 + local_60 + 1] != 0x3f0);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar8->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar8->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x505c3a) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x505c3a) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x505c3a);
          pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar7 = *piVar6 + (uint)pIVar8->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar7 + 2) = 0x3f0;
          *(undefined2 *)(iVar7 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CBaseAnimating).m_flPlaybackRate.m_Value = 0.0;
  }
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0xee])(this);
                    /* WARNING: Could not recover jumptable at 0x0074102d. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x9f])();
  return;
}


/* CLaserCatcher::CreateVPhysics at 00740b20 */

/* DWARF original prototype: bool CreateVPhysics(CLaserCatcher * this) */

bool __thiscall CLaserCatcher::CreateVPhysics(CLaserCatcher *this)

{
  CBaseEntity::VPhysicsInitStatic((CBaseEntity *)this);
  return true;
}


/* CLaserCatcher::Activate at 00740a10 */

/* DWARF original prototype: void Activate(CLaserCatcher * this) */

void __thiscall CLaserCatcher::Activate(CLaserCatcher *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::Activate(&this->super_CBaseAnimating);
  CBaseEntity::ThinkSet((CBaseEntity *)this,(BASEPTR)0x3a5,0.0,(char *)0x0);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(unaff_EBX + 0x38bef3) + *(float *)(**(int **)(unaff_EBX + 0x506143) + 0xc),
             (char *)0x0);
  return;
}


/* CLaserCatcher::AnimateThink at 00741630 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void AnimateThink(CLaserCatcher * this) */

void __thiscall CLaserCatcher::AnimateThink(CLaserCatcher *this)

{
  undefined1 *puVar1;
  float fVar2;
  ushort uVar3;
  _func_int_varargs *p_Var4;
  CBaseEdict *pCVar5;
  ushort *puVar6;
  int *piVar7;
  int iVar8;
  ModelScaleType_t MVar9;
  IChangeInfoAccessor *pIVar10;
  uint uVar11;
  uint uVar12;
  int unaff_EBX;
  longdouble extraout_ST0;
  longdouble extraout_ST0_00;
  double dVar13;
  double dVar14;
  float fVar15;
  uint local_84;
  uint local_80;
  
  ___i686_get_pc_thunk_bx();
  iVar8 = (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
            super_IHandleEntity._vptr_IHandleEntity[0xec])(this);
  if ((char)iVar8 != '\0') {
    p_Var4 = (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown
             .super_IHandleEntity._vptr_IHandleEntity[0xf2];
    fVar2 = (this->super_CBaseAnimating).m_flPlaybackRate.m_Value;
    MVar9 = CBaseAnimating::GetModelScaleType(&this->super_CBaseAnimating);
    if (MVar9 == HIERARCHICAL_MODEL_SCALE) {
      fVar15 = CBaseAnimating::GetModelScale(&this->super_CBaseAnimating);
      dVar13 = (double)fVar15;
      dVar14 = *(double *)(unaff_EBX + 0x38b56f);
    }
    else {
      dVar13 = *(double *)(unaff_EBX + 0x38b56f);
      dVar14 = dVar13;
    }
    (*p_Var4)(this,(float)((double)fVar2 * (dVar14 / SQRT(dVar13))));
    (this->super_CBaseAnimating).m_bSequenceFinished = false;
    if ((float)extraout_ST0_00 != (this->super_CBaseAnimating).m_flPlaybackRate.m_Value) {
      if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
          false) {
        pCVar5 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
          pIVar10 = CBaseEdict::GetChangeAccessor(pCVar5);
          puVar6 = (ushort *)**(undefined4 **)(&DAT_00505527 + unaff_EBX);
          if (pIVar10->m_iChangeInfoSerialNumber == *puVar6) {
            uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar3 = puVar6[uVar11 * 0x14 + 0x14];
            if (uVar3 == 0) {
LAB_00741aab:
              puVar6[(uint)uVar3 + uVar11 * 0x14 + 1] = 0x3f0;
              puVar6[uVar11 * 0x14 + 0x14] = uVar3 + 1;
            }
            else if (puVar6[uVar11 * 0x14 + 1] != 0x3f0) {
              local_84 = 0;
              do {
                uVar12 = local_84 + 1;
                local_84 = uVar12 & 0xffff;
                if ((ushort)uVar12 == uVar3) {
                  if (uVar3 != 0x13) goto LAB_00741aab;
                  pIVar10->m_iChangeInfoSerialNumber = 0;
                  pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar6[uVar11 * 0x14 + local_84 + 1] != 0x3f0);
            }
          }
          else if (puVar6[0x7d1] == 100) {
            pIVar10->m_iChangeInfoSerialNumber = 0;
            pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar10->m_iChangeInfo = puVar6[0x7d1];
            *(short *)(**(int **)(&DAT_00505527 + unaff_EBX) + 0xfa2) =
                 *(short *)(**(int **)(&DAT_00505527 + unaff_EBX) + 0xfa2) + 1;
            piVar7 = *(int **)(&DAT_00505527 + unaff_EBX);
            pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar7;
            iVar8 = *piVar7 + (uint)pIVar10->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar8 + 2) = 0x3f0;
            *(undefined2 *)(iVar8 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->super_CBaseAnimating).m_flPlaybackRate.m_Value = (float)extraout_ST0_00;
    }
  }
  iVar8 = (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
            super_IHandleEntity._vptr_IHandleEntity[0xed])(this);
  if ((char)iVar8 != '\0') {
    p_Var4 = (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown
             .super_IHandleEntity._vptr_IHandleEntity[0xf1];
    fVar2 = (this->super_CBaseAnimating).m_flPlaybackRate.m_Value;
    MVar9 = CBaseAnimating::GetModelScaleType(&this->super_CBaseAnimating);
    if (MVar9 == HIERARCHICAL_MODEL_SCALE) {
      fVar15 = CBaseAnimating::GetModelScale(&this->super_CBaseAnimating);
      dVar13 = (double)fVar15;
      dVar14 = *(double *)(unaff_EBX + 0x38b56f);
    }
    else {
      dVar13 = *(double *)(unaff_EBX + 0x38b56f);
      dVar14 = dVar13;
    }
    (*p_Var4)(this,(float)((double)fVar2 * (dVar14 / SQRT(dVar13))));
    (this->super_CBaseAnimating).m_bSequenceFinished = false;
    if ((float)extraout_ST0 != (this->super_CBaseAnimating).m_flPlaybackRate.m_Value) {
      if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
          false) {
        pCVar5 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
          pIVar10 = CBaseEdict::GetChangeAccessor(pCVar5);
          puVar6 = (ushort *)**(undefined4 **)(&DAT_00505527 + unaff_EBX);
          if (pIVar10->m_iChangeInfoSerialNumber == *puVar6) {
            uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar3 = puVar6[uVar11 * 0x14 + 0x14];
            if (uVar3 == 0) {
LAB_00741a05:
              puVar6[(uint)uVar3 + uVar11 * 0x14 + 1] = 0x3f0;
              puVar6[uVar11 * 0x14 + 0x14] = uVar3 + 1;
            }
            else if (puVar6[uVar11 * 0x14 + 1] != 0x3f0) {
              local_80 = 0;
              do {
                uVar12 = local_80 + 1;
                local_80 = uVar12 & 0xffff;
                if ((ushort)uVar12 == uVar3) {
                  if (uVar3 != 0x13) goto LAB_00741a05;
                  pIVar10->m_iChangeInfoSerialNumber = 0;
                  pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar6[uVar11 * 0x14 + local_80 + 1] != 0x3f0);
            }
          }
          else if (puVar6[0x7d1] == 100) {
            pIVar10->m_iChangeInfoSerialNumber = 0;
            pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar10->m_iChangeInfo = puVar6[0x7d1];
            *(short *)(**(int **)(&DAT_00505527 + unaff_EBX) + 0xfa2) =
                 *(short *)(**(int **)(&DAT_00505527 + unaff_EBX) + 0xfa2) + 1;
            piVar7 = *(int **)(&DAT_00505527 + unaff_EBX);
            pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar7;
            iVar8 = *piVar7 + (uint)pIVar10->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar8 + 2) = 0x3f0;
            *(undefined2 *)(iVar8 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->super_CBaseAnimating).m_flPlaybackRate.m_Value = (float)extraout_ST0;
    }
  }
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[199])(this);
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0xd4])(this,this);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(unaff_EBX + 0x38b2d3) +
             *(float *)(**(int **)(&DAT_00505523 + unaff_EBX) + 0xc),(char *)0x0);
  return;
}


/* CLaserCatcher::UpdateOnRemove at 007409b0 */

/* DWARF original prototype: void UpdateOnRemove(CLaserCatcher * this) */

void __thiscall CLaserCatcher::UpdateOnRemove(CLaserCatcher *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_pCatcherLaserTarget != (CPortalLaserTarget *)0x0) {
    UTIL_Remove(&this->m_pCatcherLaserTarget->super_CBaseEntity);
  }
  CBaseEntity::StopSound((CBaseEntity *)this,&UNK_003721c4 + unaff_EBX);
  StopParticleEffect((CBaseEntity *)this,&UNK_003721e1 + unaff_EBX);
  CBaseEntity::UpdateOnRemove((CBaseEntity *)this);
  return;
}


/* CLaserCatcher::OnPowered at 007411a0 */

/* DWARF original prototype: void OnPowered(CLaserCatcher * this) */

void __thiscall CLaserCatcher::OnPowered(CLaserCatcher *this)

{
  undefined1 *puVar1;
  int iVar2;
  ushort uVar3;
  CBaseEdict *this_00;
  ushort *puVar4;
  int *piVar5;
  IChangeInfoAccessor *pIVar6;
  uint uVar7;
  uint uVar8;
  int unaff_EBX;
  uint local_60;
  
  ___i686_get_pc_thunk_bx();
  COutputEvent::FireOutput(&this->m_OnPowered,(CBaseEntity *)this,(CBaseEntity *)this,0.0);
  DispatchParticleEffect
            ((char *)(unaff_EBX + 0x3719f4),PATTACH_POINT_FOLLOW,(CBaseEntity *)this,
             (char *)(unaff_EBX + 0x371a85),false,-1,(IRecipientFilter *)0x0);
  CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x371a08),0.0,(float *)0x0);
  CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x3719d7),0.0,(float *)0x0);
  CBaseEntity::StopSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x371a23));
  this->m_iPowerState = 2;
  if ((this->super_CBaseAnimating).m_nSkin.m_Value != 1) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
      ;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x5059ba);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_007413b4:
            puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x3e0;
            puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != 0x3e0) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_007413b4;
                pIVar6->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar7 * 0x14 + local_60 + 1] != 0x3e0);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x5059ba) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x5059ba) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x5059ba);
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar2 = *piVar5 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3e0;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CBaseAnimating).m_nSkin.m_Value = 1;
  }
  return;
}


/* CLaserCatcher::OnUnPowered at 00741420 */

/* DWARF original prototype: void OnUnPowered(CLaserCatcher * this) */

void __thiscall CLaserCatcher::OnUnPowered(CLaserCatcher *this)

{
  undefined1 *puVar1;
  int iVar2;
  ushort uVar3;
  CBaseEdict *this_00;
  ushort *puVar4;
  int *piVar5;
  IChangeInfoAccessor *pIVar6;
  uint uVar7;
  uint uVar8;
  int unaff_EBX;
  uint local_60;
  
  ___i686_get_pc_thunk_bx();
  COutputEvent::FireOutput(&this->m_OnUnpowered,(CBaseEntity *)this,(CBaseEntity *)this,0.0);
  StopParticleEffect((CBaseEntity *)this,(char *)(unaff_EBX + 0x37176e));
  CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x37179d),0.0,(float *)0x0);
  CBaseEntity::StopSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x371751));
  this->m_iPowerState = 1;
  if ((this->super_CBaseAnimating).m_nSkin.m_Value != 0) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
      ;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x505734);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_007415f4:
            puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x3e0;
            puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != 0x3e0) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_007415f4;
                pIVar6->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar7 * 0x14 + local_60 + 1] != 0x3e0);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x505734) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x505734) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x505734);
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar2 = *piVar5 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3e0;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CBaseAnimating).m_nSkin.m_Value = 0;
  }
  return;
}


/* CLaserCatcher::CreateHelperEntities at 00740870 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CreateHelperEntities(CLaserCatcher * this) */

void __thiscall CLaserCatcher::CreateHelperEntities(CLaserCatcher *this)

{
  _func_int_varargs *p_Var1;
  CPortalLaserTarget *this_00;
  uint uVar2;
  int unaff_EBX;
  Vector local_28 [2];
  
  ___i686_get_pc_thunk_bx();
  if (this->m_pCatcherLaserTarget == (CPortalLaserTarget *)0x0) {
                    /* Unresolved local var: Vector vecOrigin@[???] */
    CBaseAnimating::GetAttachment
              (&this->super_CBaseAnimating,this->m_iTargetAttachment,local_28,(Vector *)0x0,
               (Vector *)0x0,(Vector *)0x0);
    this_00 = (CPortalLaserTarget *)CreateEntityByName((char *)(unaff_EBX + 0x3722e0),-1,true);
    this->m_pCatcherLaserTarget = this_00;
    CBaseEntity::SetAbsOrigin((CBaseEntity *)this_00,local_28);
    if (((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
    CBaseEntity::SetAbsAngles
              (&this->m_pCatcherLaserTarget->super_CBaseEntity,
               &(this->super_CBaseAnimating).super_CBaseEntity.m_angAbsRotation);
    p_Var1 = (this->m_pCatcherLaserTarget->super_CBaseEntity).super_IServerEntity.
             super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x22];
    uVar2 = (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.
              super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xf0])(this);
    (*p_Var1)(this->m_pCatcherLaserTarget,unaff_EBX + 0x3722f3,uVar2 & 0xff);
    DispatchSpawn(&this->m_pCatcherLaserTarget->super_CBaseEntity,true);
    (*(this->m_pCatcherLaserTarget->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.
      super_IHandleEntity._vptr_IHandleEntity[0x26])(this->m_pCatcherLaserTarget,this,0xffffffff);
    this->m_pCatcherLaserTarget->m_pCatcher = &this->super_CCatcher;
  }
  return;
}


/* CLaserCatcher::GetCatcherModelName at 00740da0 */

/* DWARF original prototype: char * GetCatcherModelName(CLaserCatcher * this) */

char * __thiscall CLaserCatcher::GetCatcherModelName(CLaserCatcher *this)

{
  char *pcVar1;
  int extraout_ECX;
  char *pcVar2;
  
  ___i686_get_pc_thunk_cx();
  pcVar2 = (this->super_CBaseAnimating).super_CBaseEntity.m_ModelName.pszValue;
  pcVar1 = (char *)(extraout_ECX + 0x30df7c);
  if (pcVar2 != (char *)0x0) {
    pcVar1 = pcVar2;
  }
  if (*pcVar1 != '\0') {
    if (pcVar2 == (char *)0x0) {
      pcVar2 = (char *)(extraout_ECX + 0x30df7c);
    }
    return pcVar2;
  }
  return (char *)(extraout_ECX + 0x371e6c);
}


/* CLaserCatcher::IsTerminalPoint at 00740730 */

/* DWARF original prototype: bool IsTerminalPoint(CLaserCatcher * this) */

bool __thiscall CLaserCatcher::IsTerminalPoint(CLaserCatcher *this)

{
  return true;
}


/* CLaserCatcher::SpinDown at 00740740 */

/* DWARF original prototype: float SpinDown(CLaserCatcher * this, float flSpinRate) */

float __thiscall CLaserCatcher::SpinDown(CLaserCatcher *this,float flSpinRate)

{
  float *pfVar1;
  int extraout_ECX;
  float fVar2;
  float flNewSpinRate;
  
  ___i686_get_pc_thunk_cx();
  fVar2 = (float)((double)(float)((uint)flSpinRate & *(uint *)(extraout_ECX + 0x395145)) -
                 *(double *)(extraout_ECX + 0x38c905));
  pfVar1 = (float *)(extraout_ECX + 0x38c425);
  if ((fVar2 < *pfVar1 || fVar2 == *pfVar1) && (!NAN(fVar2) && !NAN(*pfVar1))) {
    this->m_iPowerState = 0;
    return 0.0;
  }
  return fVar2;
}


/* CLaserCatcher::SpinUp at 007407a0 */

/* DWARF original prototype: float SpinUp(CLaserCatcher * this, float flSpinRate) */

float __thiscall CLaserCatcher::SpinUp(CLaserCatcher *this,float flSpinRate)

{
  float fVar1;
  int extraout_ECX;
  float flNewSpinRate;
  
  ___i686_get_pc_thunk_cx();
  flNewSpinRate =
       (float)((double)(float)((uint)flSpinRate & *(uint *)(&DAT_003950e5 + extraout_ECX)) +
              *(double *)(extraout_ECX + 0x38c8a5));
  fVar1 = *(float *)(extraout_ECX + 0x38c165);
  if (fVar1 <= flNewSpinRate) {
    this->m_iPowerState = 2;
    flNewSpinRate = fVar1;
  }
  return flNewSpinRate;
}


/* CLaserCatcher::PoweringDown at 00740800 */

/* DWARF original prototype: bool PoweringDown(CLaserCatcher * this) */

bool __thiscall CLaserCatcher::PoweringDown(CLaserCatcher *this)

{
  return this->m_iPowerState == 1;
}


/* CLaserCatcher::PoweringUp at 00740820 */

/* DWARF original prototype: bool PoweringUp(CLaserCatcher * this) */

bool __thiscall CLaserCatcher::PoweringUp(CLaserCatcher *this)

{
  return this->m_iPowerState == 2;
}


/* DataMapInit<CLaserRelay> at 000af9e0 */

datamap_t * DataMapInit<CLaserRelay>(CLaserRelay *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xeb].m_pEntity + unaff_EBX + 3)
      == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xd9ea97);
    if (iVar1 != 0) {
      *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xf0].m_pPrev + unaff_EBX + 3) =
           unaff_EBX + 0xa03266;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xf1].m_pEntity + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xf1].m_SerialNumber + unaff_EBX + 3) = 0
      ;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xf1].m_pPrev + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xf1].m_pNext + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xf2].m_pEntity + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xf0].m_pNext + unaff_EBX + 3) = 0xb;
      ___cxa_guard_release(unaff_EBX + 0xd9ea97);
      ___cxa_atexit(unaff_EBX + 0x98eb6f,0,*(undefined4 *)(&DAT_00b97163 + unaff_EBX));
    }
  }
  *(undefined **)((int)&PTR_typeinfo_name_00cb69e8 + unaff_EBX + 3) = &UNK_00cb69c7 + unaff_EBX;
  *(undefined4 *)((int)&PTR_typeinfo_00cb69e0 + unaff_EBX + 3) = 1;
  *(int *)((int)&PTR_typeinfo_name_00cb69dc + unaff_EBX + 3) = unaff_EBX + 0xd9eaaf;
  return (datamap_t *)((int)&PTR_typeinfo_name_00cb69dc + unaff_EBX + 3);
}


/* CLaserRelay::GetDataDescMap at 00740840 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CLaserRelay * this) */

datamap_t * __thiscall CLaserRelay::GetDataDescMap(CLaserRelay *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x625b88);
}


/* CLaserRelay::GetBaseMap at 00740850 */

datamap_t * CLaserRelay::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x625b60);
}


/* __tcf_2 at 00a3e560 */

void __tcf_2(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x40ff86)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x40ff7a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x40ff86));
  }
  *(undefined4 *)(unaff_EBX + 0x40ff86) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40ff82)) {
    if (*(int *)(unaff_EBX + 0x40ff7a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2085ee) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2085ee),*(int *)(unaff_EBX + 0x40ff7a));
      *(undefined4 *)(unaff_EBX + 0x40ff7a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40ff7e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40ff7a);
  *(int *)(unaff_EBX + 0x40ff8a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40ff82)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2085ee) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2085ee),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40ff7a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40ff7e) = 0;
  }
  return;
}


/* CLaserRelay::GetCatcherModelName at 00740d60 */

/* DWARF original prototype: char * GetCatcherModelName(CLaserRelay * this) */

char * __thiscall CLaserRelay::GetCatcherModelName(CLaserRelay *this)

{
  char *pcVar1;
  int extraout_ECX;
  char *pcVar2;
  
  ___i686_get_pc_thunk_cx();
  pcVar2 = (this->super_CLaserCatcher).super_CBaseAnimating.super_CBaseEntity.m_ModelName.pszValue;
  pcVar1 = (char *)(extraout_ECX + 0x30dfbc);
  if (pcVar2 != (char *)0x0) {
    pcVar1 = pcVar2;
  }
  if (*pcVar1 != '\0') {
    if (pcVar2 == (char *)0x0) {
      pcVar2 = (char *)(extraout_ECX + 0x30dfbc);
    }
    return pcVar2;
  }
  return (char *)(extraout_ECX + 0x371e88);
}


/* CLaserRelay::IsTerminalPoint at 00740860 */

/* DWARF original prototype: bool IsTerminalPoint(CLaserRelay * this) */

bool __thiscall CLaserRelay::IsTerminalPoint(CLaserRelay *this)

{
  return false;
}


/* _GLOBAL__I__ZN18CPortalLaserTarget9m_DataMapE at 000b0070 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN18CPortalLaserTarget9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

