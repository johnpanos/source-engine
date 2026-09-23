/* DWARF-guided pseudocode for game/server/portal2/point_laser_target.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* DataMapInit<CPortalLaserTarget> at 000c4840 */

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
  if ((*(char *)((int)&rgv3tStuckTable[0x32].y + unaff_EBX) == '\0') &&
     (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xcf497c), iVar3 != 0)) {
    *(undefined **)((int)&rgv3tStuckTable[0x33].z + unaff_EBX) = &UNK_0094c23b + unaff_EBX;
    *(undefined4 *)((int)&rgv3tStuckTable[0x34].y + unaff_EBX) = 0;
    *(undefined4 *)((int)&rgv3tStuckTable[0x34].z + unaff_EBX) = 0;
    *(undefined4 *)((int)&rgv3tStuckTable[0x35].x + unaff_EBX) = 0;
    *(undefined4 *)((int)&rgv3tStuckTable[0x35].y + unaff_EBX) = 0;
    *(undefined4 *)((int)&rgv3tStuckTable[0x35].z + unaff_EBX) = 0;
    *(undefined4 *)((int)&rgv3tStuckTable[0x34].x + unaff_EBX) = 0x12;
    ___cxa_guard_release(unaff_EBX + 0xcf497c);
    ___cxa_atexit(unaff_EBX + 0x8d661c,0,*(undefined4 *)(&DAT_00ae733c + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_HasTarget_00c078b8 + unaff_EBX) =
       *(undefined4 *)(&DAT_00ae7478 + unaff_EBX);
  if ((*(char *)((int)&rgv3tStuckTable[0x33].x + unaff_EBX) == '\0') &&
     (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xcf4984), iVar3 != 0)) {
    pcVar4 = operator_new___(*(int *)((int)&rgv3tStuckTable[0x34].x + unaff_EBX) + 0xd);
    _strcpy(pcVar4,*(char **)((int)&rgv3tStuckTable[0x33].z + unaff_EBX));
    sVar5 = _strlen(pcVar4);
    builtin_strncpy(pcVar4 + sVar5,"DisableThink",0xd);
    iVar1 = *(int *)((int)&rgv3tStuckTable[0x35].y + unaff_EBX);
    iVar3 = iVar1 + 1;
    iVar8 = *(int *)((int)&rgv3tStuckTable[0x34].z + unaff_EBX);
    if (iVar8 < iVar3) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)((int)&rgv3tStuckTable[0x34].y + unaff_EBX),iVar3 - iVar8)
      ;
    }
    iVar6 = *(int *)((int)&rgv3tStuckTable[0x35].y + unaff_EBX) + 1;
    *(int *)((int)&rgv3tStuckTable[0x35].y + unaff_EBX) = iVar6;
    iVar8 = *(int *)((int)&rgv3tStuckTable[0x34].y + unaff_EBX);
    *(int *)((int)&rgv3tStuckTable[0x35].z + unaff_EBX) = iVar8;
    iVar6 = (iVar6 - iVar1) + -1;
    if (0 < iVar6) {
      _V_memmove((void *)(iVar8 + iVar3 * 4),(void *)(iVar8 + iVar1 * 4),iVar6 * 4);
      iVar8 = *(int *)((int)&rgv3tStuckTable[0x34].y + unaff_EBX);
    }
    puVar7 = (undefined4 *)(iVar8 + iVar1 * 4);
    if (puVar7 != (undefined4 *)0x0) {
      *puVar7 = pcVar4;
    }
    *(char **)((int)&PTR_CanBeHitByMeleeAttack_00c07b90 + unaff_EBX) = pcVar4;
    uVar2 = **(undefined4 **)(&DAT_00ae7358 + unaff_EBX);
    *(undefined4 *)((int)&PTR_CanBeSeenBy_00c07ce0 + unaff_EBX) = uVar2;
    *(undefined4 *)((int)&PTR_VPhysicsCollision_00c07d20 + unaff_EBX) = uVar2;
    ___cxa_guard_release(unaff_EBX + 0xcf4984);
  }
  *(undefined4 *)((int)&PTR_ImpactTrace_00c078b0 + unaff_EBX) = 7;
  *(int *)((int)&PTR_DecalTrace_00c078ac + unaff_EBX) = (int)&PTR_TraceAttack_00c07b8c + unaff_EBX;
  return (datamap_t *)((int)&PTR_DecalTrace_00c078ac + unaff_EBX);
}


/* __static_initialization_and_destruction_0 at 000c4d60 */

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
  *(undefined1 *)((int)&sv_showanimstate.super_ConCommandBase.m_nFlags + unaff_EBX) = 0;
  *(undefined1 *)((int)&sv_showanimstate.super_ConCommandBase.m_nFlags + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&sv_showanimstate.super_ConCommandBase.m_nFlags + unaff_EBX + 2) = 0;
  *(undefined1 *)((int)&sv_showanimstate.super_ConCommandBase.m_nFlags + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&sv_showanimstate.super_IConVar._vptr_IConVar + unaff_EBX) = 0;
  *(undefined4 *)((int)&sv_showanimstate.m_pParent + unaff_EBX) = 0;
  *(undefined4 *)((int)&sv_showanimstate.m_pszDefaultValue + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&sv_showanimstate.m_Value.m_pszString + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&sv_showanimstate.m_Value.m_StringLength + unaff_EBX) = 0;
  *(undefined4 *)((int)&sv_showanimstate.m_Value.m_fValue + unaff_EBX) = 0;
  *(undefined4 *)((int)&sv_showanimstate.m_Value.m_nValue + unaff_EBX) = 0;
  *(undefined4 *)(&sv_showanimstate.m_bHasMin + unaff_EBX) = 0;
  *(undefined4 *)((int)&sv_showanimstate.m_fMinVal + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&sv_showanimstate.m_bHasMax + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&sv_showanimstate.m_fMaxVal + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&sv_showanimstate.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&sv_showanimstate.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) = 0;
  *(undefined1 *)((int)&sv_showanimstate.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 3) =
       0;
  *(undefined1 *)((int)&sv_showanimstate.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) = 0;
  *(undefined1 *)((int)&sv_showanimstate.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 1) =
       0;
  *(undefined1 *)((int)&sv_showanimstate.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 2) =
       0;
  *(undefined4 *)((int)&sv_showanimstate.m_fnChangeCallbacks.m_Size + unaff_EBX) = 0;
  *(undefined1 *)((int)&sv_showanimstate.m_fnChangeCallbacks.m_pElements + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&sv_showanimstate.m_fnChangeCallbacks.m_pElements + unaff_EBX) = 0;
  *(undefined1 *)((int)&sv_showanimstate.m_fnChangeCallbacks.m_pElements + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&sv_showanimstate.m_fnChangeCallbacks.m_pElements + unaff_EBX + 2) = 0;
  *(undefined4 *)(&DAT_00cf4398 + unaff_EBX) = 0;
  (&DAT_00cf439f)[unaff_EBX] = 1;
  (&DAT_00cf439c)[unaff_EBX] = 0;
  (&DAT_00cf439d)[unaff_EBX] = 0;
  (&DAT_00cf439e)[unaff_EBX] = 0;
  *(undefined4 *)((int)&showanimstate_log.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) = 1
  ;
  *(undefined1 *)((int)&showanimstate_log.super_ConCommandBase.m_pNext + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&showanimstate_log.super_ConCommandBase.m_pNext + unaff_EBX) = 0;
  *(undefined1 *)((int)&showanimstate_log.super_ConCommandBase.m_pNext + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&showanimstate_log.super_ConCommandBase.m_pNext + unaff_EBX + 2) = 0;
  *(undefined4 *)(&showanimstate_log.super_ConCommandBase.m_bRegistered + unaff_EBX) = 2;
  *(undefined1 *)((int)&showanimstate_log.super_ConCommandBase.m_pszName + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&showanimstate_log.super_ConCommandBase.m_pszName + unaff_EBX) = 0;
  *(undefined1 *)((int)&showanimstate_log.super_ConCommandBase.m_pszName + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&showanimstate_log.super_ConCommandBase.m_pszName + unaff_EBX + 2) = 0;
  *(undefined **)((int)&showanimstate_log.super_ConCommandBase.m_pszHelpString + unaff_EBX) =
       &UNK_00aec5bc + unaff_EBX;
  *(undefined **)((int)&showanimstate_log.super_ConCommandBase.m_nFlags + unaff_EBX) =
       &UNK_00b71c34 + unaff_EBX;
  pIVar1 = EntityFactoryDictionary();
  (**pIVar1->_vptr_IEntityFactoryDictionary)(pIVar1,unaff_EBX + 0xcf43b4,unaff_EBX + 0x949a64);
  pdVar2 = DataMapInit<CPortalLaserTarget>((CPortalLaserTarget *)0x0);
  *(datamap_t **)((int)&PTR__ScrollBarButton_00c6a92c + unaff_EBX) = pdVar2;
  *(undefined **)((int)&showanimstate_log.super_IConVar._vptr_IConVar + unaff_EBX) =
       &UNK_00b71c54 + unaff_EBX;
  pIVar1 = EntityFactoryDictionary();
  (**pIVar1->_vptr_IEntityFactoryDictionary)(pIVar1,unaff_EBX + 0xcf43b8,&UNK_0094bd74 + unaff_EBX);
  pdVar2 = DataMapInit<CLaserCatcher>((CLaserCatcher *)0x0);
  *(datamap_t **)((int)&PTR_SetVisible_00c6a930 + unaff_EBX) = pdVar2;
  *(undefined **)((int)&showanimstate_log.m_pParent + unaff_EBX) = &UNK_00b71c74 + unaff_EBX;
  pIVar1 = EntityFactoryDictionary();
  (**pIVar1->_vptr_IEntityFactoryDictionary)(pIVar1,unaff_EBX + 0xcf43bc,&UNK_0094bd87 + unaff_EBX);
  pdVar2 = DataMapInit<CLaserRelay>((CLaserRelay *)0x0);
  *(datamap_t **)((int)&PTR_IsVisible_00c6a934 + unaff_EBX) = pdVar2;
  return;
}


/* CPortalLaserTarget::GetDataDescMap at 006d31a0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPortalLaserTarget * this) */

datamap_t * __thiscall CPortalLaserTarget::GetDataDescMap(CPortalLaserTarget *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5f8f58);
}


/* CPortalLaserTarget::GetBaseMap at 006d31b0 */

datamap_t * CPortalLaserTarget::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4d8b14);
}


/* __tcf_0 at 0099ae70 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41e376)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41e36a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41e376));
  }
  *(undefined4 *)(unaff_EBX + 0x41e376) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41e372)) {
    if (*(int *)(unaff_EBX + 0x41e36a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x210d1a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x210d1a),*(int *)(unaff_EBX + 0x41e36a));
      *(undefined4 *)(unaff_EBX + 0x41e36a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e36e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41e36a);
  *(int *)(unaff_EBX + 0x41e37a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41e372)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x210d1a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x210d1a),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41e36a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e36e) = 0;
  }
  return;
}


/* CPortalLaserTarget::Spawn at 006d38a0 */

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


/* CPortalLaserTarget::OnTakeDamage at 006d3700 */

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
    iVar4 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d8490);
    if (*(uint *)(iVar4 + 8) != uVar1 >> 0x10) {
      return 0;
    }
    this_00 = *(CBaseEntity **)(iVar4 + 4);
    if (this_00 == (CBaseEntity *)0x0) {
      return 0;
    }
    if (((this_00->m_iClassname).pszValue != (char *)(unaff_EBX + 0x33b093)) &&
       (bVar3 = CBaseEntity::ClassMatchesComplex(this_00,(char *)(unaff_EBX + 0x33b093)), !bVar3)) {
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
             *(float *)(unaff_EBX + 0x356c00) + *(float *)(**(int **)(unaff_EBX + 0x4d848c) + 0xc),
             (char *)0x0);
  return 0;
}


/* CPortalLaserTarget::DisableThink at 006d3560 */

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


/* CPortalLaserTarget::IsPowered at 006d31c0 */

/* DWARF original prototype: bool IsPowered(CPortalLaserTarget * this) */

bool __thiscall CPortalLaserTarget::IsPowered(CPortalLaserTarget *this)

{
  return this->m_bPowered;
}


/* CLaserCatcher::OnRestore at 006d3920 */

/* DWARF original prototype: void OnRestore(CLaserCatcher * this) */

void __thiscall CLaserCatcher::OnRestore(CLaserCatcher *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::OnRestore(&this->super_CBaseAnimating);
  if ((this->m_pCatcherLaserTarget != (CPortalLaserTarget *)0x0) &&
     (this->m_pCatcherLaserTarget->m_pCatcher = &this->super_CCatcher,
     this->m_pCatcherLaserTarget->m_bPowered != false)) {
    CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x33d0b0),0.0,(float *)0x0);
    DispatchParticleEffect
              ((char *)(unaff_EBX + 0x33d0cd),PATTACH_POINT_FOLLOW,(CBaseEntity *)this,
               (char *)(unaff_EBX + 0x33d171),false,-1,(IRecipientFilter *)0x0);
    return;
  }
  return;
}


/* DataMapInit<CLaserCatcher> at 000c4b30 */

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
  if (((&sv_showladders.m_bHasMax)[unaff_EBX] == false) &&
     (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xcf465c), iVar3 != 0)) {
    *(int *)((int)&sv_showladders.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) =
         unaff_EBX + 0x94bf8e;
    *(undefined4 *)((int)&sv_showladders.m_fnChangeCallbacks.m_pElements + unaff_EBX) = 0;
    *(undefined4 *)((int)&CFuncLadder::s_Ladders.m_Memory.m_pMemory + unaff_EBX) = 0;
    *(undefined4 *)((int)&CFuncLadder::s_Ladders.m_Memory.m_nAllocationCount + unaff_EBX) = 0;
    *(undefined4 *)((int)&CFuncLadder::s_Ladders.m_Memory.m_nGrowSize + unaff_EBX) = 0;
    *(undefined4 *)((int)&CFuncLadder::s_Ladders.m_Size + unaff_EBX) = 0;
    *(undefined4 *)((int)&sv_showladders.m_fnChangeCallbacks.m_Size + unaff_EBX) = 0xd;
    ___cxa_guard_release(unaff_EBX + 0xcf465c);
    ___cxa_atexit(unaff_EBX + 0x8d64ec,0,*(undefined4 *)(&DAT_00ae704c + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_StartTouch_00c075e0 + unaff_EBX) =
       *(undefined4 *)(&DAT_00ae7058 + unaff_EBX);
  if ((*(char *)((int)&sv_showladders.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) == '\0')
     && (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xcf4664), iVar3 != 0)) {
    pcVar4 = operator_new___(*(int *)((int)&sv_showladders.m_fnChangeCallbacks.m_Size + unaff_EBX) +
                             0xd);
    _strcpy(pcVar4,*(char **)((int)&sv_showladders.m_fnChangeCallbacks.m_Memory.m_nGrowSize +
                             unaff_EBX));
    sVar5 = _strlen(pcVar4);
    builtin_strncpy(pcVar4 + sVar5,"AnimateThink",0xd);
    iVar1 = *(int *)((int)&CFuncLadder::s_Ladders.m_Memory.m_nGrowSize + unaff_EBX);
    iVar3 = iVar1 + 1;
    iVar8 = *(int *)((int)&CFuncLadder::s_Ladders.m_Memory.m_pMemory + unaff_EBX);
    if (iVar8 < iVar3) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)
                 ((int)&sv_showladders.m_fnChangeCallbacks.m_pElements + unaff_EBX),iVar3 - iVar8);
    }
    iVar6 = *(int *)((int)&CFuncLadder::s_Ladders.m_Memory.m_nGrowSize + unaff_EBX) + 1;
    *(int *)((int)&CFuncLadder::s_Ladders.m_Memory.m_nGrowSize + unaff_EBX) = iVar6;
    iVar8 = *(int *)((int)&sv_showladders.m_fnChangeCallbacks.m_pElements + unaff_EBX);
    *(int *)((int)&CFuncLadder::s_Ladders.m_Size + unaff_EBX) = iVar8;
    iVar6 = (iVar6 - iVar1) + -1;
    if (0 < iVar6) {
      _V_memmove((void *)(iVar8 + iVar3 * 4),(void *)(iVar8 + iVar1 * 4),iVar6 * 4);
      iVar8 = *(int *)((int)&sv_showladders.m_fnChangeCallbacks.m_pElements + unaff_EBX);
    }
    puVar7 = (undefined4 *)(iVar8 + iVar1 * 4);
    if (puVar7 != (undefined4 *)0x0) {
      *puVar7 = pcVar4;
    }
    *(char **)((int)&PTR_CanBeSeenBy_00c076a0 + unaff_EBX) = pcVar4;
    uVar2 = **(undefined4 **)(&DAT_00ae7068 + unaff_EBX);
    *(undefined4 *)((int)&PTR_KeyValue_00c077f0 + unaff_EBX) = uVar2;
    *(undefined4 *)((int)&PTR_MoveDone_00c07830 + unaff_EBX) = uVar2;
    ___cxa_guard_release(unaff_EBX + 0xcf4664);
  }
  *(undefined4 *)((int)&PTR_UpdatePaintPowersFromContacts_00c075d8 + unaff_EBX) = 8;
  *(int *)((int)&PTR_GetEnemy_00c075d4 + unaff_EBX) = (int)&PTR_GetDamageType_00c0765c + unaff_EBX;
  return (datamap_t *)((int)&PTR_GetEnemy_00c075d4 + unaff_EBX);
}


/* CLaserCatcher::GetDataDescMap at 006d31d0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CLaserCatcher * this) */

datamap_t * __thiscall CLaserCatcher::GetDataDescMap(CLaserCatcher *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5f8f40);
}


/* CLaserCatcher::GetBaseMap at 006d31e0 */

datamap_t * CLaserCatcher::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4d89b4);
}


/* __tcf_1 at 0099b030 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41e186)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41e17a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41e186));
  }
  *(undefined4 *)(unaff_EBX + 0x41e186) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41e182)) {
    if (*(int *)(unaff_EBX + 0x41e17a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x210b5a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x210b5a),*(int *)(unaff_EBX + 0x41e17a));
      *(undefined4 *)(unaff_EBX + 0x41e17a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e17e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41e17a);
  *(int *)(unaff_EBX + 0x41e18a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41e182)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x210b5a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x210b5a),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41e17a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e17e) = 0;
  }
  return;
}


/* CLaserCatcher::CLaserCatcher at 006d3670 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CLaserCatcher(CLaserCatcher * this) */

void __thiscall CLaserCatcher::CLaserCatcher(CLaserCatcher *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::CBaseAnimating(&this->super_CBaseAnimating);
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x562b87);
  (this->super_CCatcher).super_ICatcher._vptr_ICatcher =
       (_func_int_varargs **)(unaff_EBX + 0x562f5b);
  (this->m_OnPowered).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnPowered).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_OnPowered).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_OnUnpowered).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnUnpowered).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_OnUnpowered).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  this->m_iPowerState = 0;
  return;
}


/* CLaserCatcher::CLaserCatcher at 006d36f0 */

/* DWARF original prototype: void CLaserCatcher(CLaserCatcher * this, CLaserCatcher * this) */

void __thiscall CLaserCatcher::CLaserCatcher(CLaserCatcher *this,CLaserCatcher *this_1)

{
  CLaserCatcher(this);
  return;
}


/* CEntityFactory<CLaserCatcher>::Create at 006d41f0 */

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


/* CLaserCatcher::Precache at 006d3600 */

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
  PrecacheParticleSystem((char *)(unaff_EBX + 0x33d3f1));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x33d405));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x33d420));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x33d3d4));
  CBaseAnimating::Precache(&this->super_CBaseAnimating);
  return;
}


/* CLaserCatcher::Spawn at 006d39e0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "MoveCollide_t": Some values do not have unique names */
/* WARNING: Enum "MoveType_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(CLaserCatcher * this) */

void __thiscall CLaserCatcher::Spawn(CLaserCatcher *this)

{
  undefined1 *puVar1;
  _func_int_varargs **pp_Var2;
  _func_int_varargs *p_Var3;
  CBaseEdict *this_00;
  int iVar4;
  IChangeInfoAccessor *pIVar5;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1a])(this);
  CBaseAnimating::Spawn(&this->super_CBaseAnimating);
  pp_Var2 = (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
            super_IHandleEntity._vptr_IHandleEntity;
  p_Var3 = pp_Var2[0x1b];
  iVar4 = (*pp_Var2[0xef])(this);
  (*p_Var3)(this,iVar4);
  CCollisionProperty::SetSolid
            (&(this->super_CBaseAnimating).super_CBaseEntity.m_Collision.super_CCollisionProperty,
             SOLID_VPHYSICS);
  CBaseEntity::SetMoveType((CBaseEntity *)this,MOVETYPE_VPHYSICS,MOVECOLLIDE_DEFAULT);
  iVar4 = CBaseAnimating::LookupSequence(&this->super_CBaseAnimating,(char *)(unaff_EBX + 0x31ee80))
  ;
  this->m_IdleSequence = iVar4;
  iVar4 = CBaseAnimating::LookupSequence(&this->super_CBaseAnimating,(char *)(unaff_EBX + 0x33d0c6))
  ;
  this->m_PowerOnSequence = iVar4;
  iVar4 = CBaseAnimating::LookupAttachment
                    (&this->super_CBaseAnimating,(char *)(unaff_EBX + 0x33d0cb));
  this->m_iTargetAttachment = iVar4;
  CBaseAnimating::ResetSequence(&this->super_CBaseAnimating,this->m_PowerOnSequence);
  if ((this->super_CBaseAnimating).m_flPlaybackRate.m_Value != 0.0) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
      ;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar5 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar5->m_iChangeInfoSerialNumber = 0;
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
                    /* WARNING: Could not recover jumptable at 0x006d3af7. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x9f])();
  return;
}


/* CLaserCatcher::CreateVPhysics at 006d35e0 */

/* DWARF original prototype: bool CreateVPhysics(CLaserCatcher * this) */

bool __thiscall CLaserCatcher::CreateVPhysics(CLaserCatcher *this)

{
  CBaseEntity::VPhysicsInitStatic((CBaseEntity *)this);
  return true;
}


/* CLaserCatcher::Activate at 006d34d0 */

/* DWARF original prototype: void Activate(CLaserCatcher * this) */

void __thiscall CLaserCatcher::Activate(CLaserCatcher *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::Activate(&this->super_CBaseAnimating);
  CBaseEntity::ThinkSet((CBaseEntity *)this,(BASEPTR)0x3a5,0.0,(char *)0x0);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(unaff_EBX + 0x356e33) + *(float *)(**(int **)(unaff_EBX + 0x4d86bf) + 0xc),
             (char *)0x0);
  return;
}


/* CLaserCatcher::AnimateThink at 006d3d30 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void AnimateThink(CLaserCatcher * this) */

void __thiscall CLaserCatcher::AnimateThink(CLaserCatcher *this)

{
  undefined1 *puVar1;
  float fVar2;
  _func_int_varargs *p_Var3;
  CBaseEdict *pCVar4;
  int iVar5;
  ModelScaleType_t MVar6;
  IChangeInfoAccessor *pIVar7;
  int unaff_EBX;
  longdouble extraout_ST0;
  longdouble extraout_ST0_00;
  double dVar8;
  double dVar9;
  float fVar10;
  
  ___i686_get_pc_thunk_bx();
  iVar5 = (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
            super_IHandleEntity._vptr_IHandleEntity[0xec])(this);
  if ((char)iVar5 != '\0') {
    p_Var3 = (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown
             .super_IHandleEntity._vptr_IHandleEntity[0xf2];
    fVar2 = (this->super_CBaseAnimating).m_flPlaybackRate.m_Value;
    MVar6 = CBaseAnimating::GetModelScaleType(&this->super_CBaseAnimating);
    if (MVar6 == HIERARCHICAL_MODEL_SCALE) {
      fVar10 = CBaseAnimating::GetModelScale(&this->super_CBaseAnimating);
      dVar8 = (double)fVar10;
      dVar9 = *(double *)(unaff_EBX + 0x356872);
    }
    else {
      dVar8 = *(double *)(unaff_EBX + 0x356872);
      dVar9 = dVar8;
    }
    (*p_Var3)(this,(float)((double)fVar2 * (dVar9 / SQRT(dVar8))));
    if ((float)extraout_ST0_00 != (this->super_CBaseAnimating).m_flPlaybackRate.m_Value) {
      if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
          false) {
        pCVar4 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if (pCVar4 != (CBaseEdict *)0x0) {
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x101;
          pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
          pIVar7->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->super_CBaseAnimating).m_flPlaybackRate.m_Value = (float)extraout_ST0_00;
    }
  }
  iVar5 = (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
            super_IHandleEntity._vptr_IHandleEntity[0xed])(this);
  if ((char)iVar5 != '\0') {
    p_Var3 = (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown
             .super_IHandleEntity._vptr_IHandleEntity[0xf1];
    fVar2 = (this->super_CBaseAnimating).m_flPlaybackRate.m_Value;
    MVar6 = CBaseAnimating::GetModelScaleType(&this->super_CBaseAnimating);
    if (MVar6 == HIERARCHICAL_MODEL_SCALE) {
      fVar10 = CBaseAnimating::GetModelScale(&this->super_CBaseAnimating);
      dVar8 = (double)fVar10;
      dVar9 = *(double *)(unaff_EBX + 0x356872);
    }
    else {
      dVar8 = *(double *)(unaff_EBX + 0x356872);
      dVar9 = dVar8;
    }
    (*p_Var3)(this,(float)((double)fVar2 * (dVar9 / SQRT(dVar8))));
    if ((float)extraout_ST0 != (this->super_CBaseAnimating).m_flPlaybackRate.m_Value) {
      if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
          false) {
        pCVar4 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if (pCVar4 != (CBaseEdict *)0x0) {
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x101;
          pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
          pIVar7->m_iChangeInfoSerialNumber = 0;
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
             *(float *)(unaff_EBX + 0x3565d6) + *(float *)(**(int **)(unaff_EBX + 0x4d7e62) + 0xc),
             (char *)0x0);
  return;
}


/* CLaserCatcher::UpdateOnRemove at 006d3470 */

/* DWARF original prototype: void UpdateOnRemove(CLaserCatcher * this) */

void __thiscall CLaserCatcher::UpdateOnRemove(CLaserCatcher *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_pCatcherLaserTarget != (CPortalLaserTarget *)0x0) {
    UTIL_Remove(&this->m_pCatcherLaserTarget->super_CBaseEntity);
  }
  CBaseEntity::StopSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x33d560));
  StopParticleEffect((CBaseEntity *)this,(char *)(unaff_EBX + 0x33d57d));
  CBaseEntity::UpdateOnRemove((CBaseEntity *)this);
  return;
}


/* CLaserCatcher::OnPowered at 006d3b20 */

/* DWARF original prototype: void OnPowered(CLaserCatcher * this) */

void __thiscall CLaserCatcher::OnPowered(CLaserCatcher *this)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  COutputEvent::FireOutput(&this->m_OnPowered,(CBaseEntity *)this,(CBaseEntity *)this,0.0);
  DispatchParticleEffect
            ((char *)(unaff_EBX + 0x33ced0),PATTACH_POINT_FOLLOW,(CBaseEntity *)this,
             (char *)(unaff_EBX + 0x33cf74),false,-1,(IRecipientFilter *)0x0);
  CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x33cee4),0.0,(float *)0x0);
  CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x33ceb3),0.0,(float *)0x0);
  CBaseEntity::StopSound((CBaseEntity *)this,&UNK_0033ceff + unaff_EBX);
  this->m_iPowerState = 2;
  if ((this->super_CBaseAnimating).m_nSkin.m_Value != 1) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
      ;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar2->m_iChangeInfoSerialNumber = 0;
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


/* CLaserCatcher::OnUnPowered at 006d3c50 */

/* DWARF original prototype: void OnUnPowered(CLaserCatcher * this) */

void __thiscall CLaserCatcher::OnUnPowered(CLaserCatcher *this)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  COutputEvent::FireOutput(&this->m_OnUnpowered,(CBaseEntity *)this,(CBaseEntity *)this,0.0);
  StopParticleEffect((CBaseEntity *)this,(char *)(unaff_EBX + 0x33cd9a));
  CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x33cdc9),0.0,(float *)0x0);
  CBaseEntity::StopSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x33cd7d));
  this->m_iPowerState = 1;
  if ((this->super_CBaseAnimating).m_nSkin.m_Value != 0) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
      ;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar2->m_iChangeInfoSerialNumber = 0;
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


/* CLaserCatcher::CreateHelperEntities at 006d3330 */

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
    this_00 = (CPortalLaserTarget *)CreateEntityByName((char *)(unaff_EBX + 0x33b48c),-1,true);
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
    (*p_Var1)(this->m_pCatcherLaserTarget,unaff_EBX + 0x33d68f,uVar2 & 0xff);
    DispatchSpawn(&this->m_pCatcherLaserTarget->super_CBaseEntity,true);
    (*(this->m_pCatcherLaserTarget->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.
      super_IHandleEntity._vptr_IHandleEntity[0x26])(this->m_pCatcherLaserTarget,this,0xffffffff);
    this->m_pCatcherLaserTarget->m_pCatcher = &this->super_CCatcher;
  }
  return;
}


/* CLaserCatcher::GetCatcherModelName at 006d3860 */

/* DWARF original prototype: char * GetCatcherModelName(CLaserCatcher * this) */

char * __thiscall CLaserCatcher::GetCatcherModelName(CLaserCatcher *this)

{
  char *pcVar1;
  int extraout_ECX;
  char *pcVar2;
  
  ___i686_get_pc_thunk_cx();
  pcVar2 = (this->super_CBaseAnimating).super_CBaseEntity.m_ModelName.pszValue;
  pcVar1 = &DAT_002d82f4 + extraout_ECX;
  if (pcVar2 != (char *)0x0) {
    pcVar1 = pcVar2;
  }
  if (*pcVar1 != '\0') {
    if (pcVar2 == (char *)0x0) {
      pcVar2 = &DAT_002d82f4 + extraout_ECX;
    }
    return pcVar2;
  }
  return &UNK_0033d208 + extraout_ECX;
}


/* CLaserCatcher::IsTerminalPoint at 006d31f0 */

/* DWARF original prototype: bool IsTerminalPoint(CLaserCatcher * this) */

bool __thiscall CLaserCatcher::IsTerminalPoint(CLaserCatcher *this)

{
  return true;
}


/* CLaserCatcher::SpinDown at 006d3200 */

/* DWARF original prototype: float SpinDown(CLaserCatcher * this, float flSpinRate) */

float __thiscall CLaserCatcher::SpinDown(CLaserCatcher *this,float flSpinRate)

{
  float *pfVar1;
  int extraout_ECX;
  float fVar2;
  float flNewSpinRate;
  
  ___i686_get_pc_thunk_cx();
  fVar2 = (float)((double)(float)((uint)flSpinRate & *(uint *)(extraout_ECX + 0x3601f5)) -
                 *(double *)(extraout_ECX + 0x357855));
  pfVar1 = (float *)(extraout_ECX + 0x357365);
  if ((fVar2 < *pfVar1 || fVar2 == *pfVar1) && (!NAN(fVar2) && !NAN(*pfVar1))) {
    this->m_iPowerState = 0;
    return 0.0;
  }
  return fVar2;
}


/* CLaserCatcher::SpinUp at 006d3260 */

/* DWARF original prototype: float SpinUp(CLaserCatcher * this, float flSpinRate) */

float __thiscall CLaserCatcher::SpinUp(CLaserCatcher *this,float flSpinRate)

{
  float fVar1;
  int extraout_ECX;
  float flNewSpinRate;
  
  ___i686_get_pc_thunk_cx();
  flNewSpinRate =
       (float)((double)(float)((uint)flSpinRate & *(uint *)(extraout_ECX + 0x360195)) +
              *(double *)(extraout_ECX + 0x3577f5));
  fVar1 = *(float *)(extraout_ECX + 0x3570a5);
  if (fVar1 <= flNewSpinRate) {
    this->m_iPowerState = 2;
    flNewSpinRate = fVar1;
  }
  return flNewSpinRate;
}


/* CLaserCatcher::PoweringDown at 006d32c0 */

/* DWARF original prototype: bool PoweringDown(CLaserCatcher * this) */

bool __thiscall CLaserCatcher::PoweringDown(CLaserCatcher *this)

{
  return this->m_iPowerState == 1;
}


/* CLaserCatcher::PoweringUp at 006d32e0 */

/* DWARF original prototype: bool PoweringUp(CLaserCatcher * this) */

bool __thiscall CLaserCatcher::PoweringUp(CLaserCatcher *this)

{
  return this->m_iPowerState == 2;
}


/* DataMapInit<CLaserRelay> at 000c4a60 */

datamap_t * DataMapInit<CLaserRelay>(CLaserRelay *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&DT_InfoLadderDismount::g_SendTableInit + unaff_EBX + 3) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf46bf);
    if (iVar1 != 0) {
      *(undefined **)
       ((int)&option_duck_method.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 3) =
           &UNK_0094c055 + unaff_EBX;
      *(undefined4 *)((int)&option_duck_method.m_fnChangeCallbacks.m_pElements + unaff_EBX + 3) = 0;
      *(undefined4 *)(&DAT_00cf471b + unaff_EBX) = 0;
      *(undefined4 *)(&DAT_00cf471f + unaff_EBX) = 0;
      *(undefined4 *)((int)&rgv3tStuckTable[0].x + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&rgv3tStuckTable[0].y + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&option_duck_method.m_fnChangeCallbacks.m_Size + unaff_EBX + 3) = 0xb;
      ___cxa_guard_release(unaff_EBX + 0xcf46bf);
      ___cxa_atexit(&UNK_008d64df + unaff_EBX,0,*(undefined4 *)(&DAT_00ae711f + unaff_EBX));
    }
  }
  *(int *)((int)&PTR_ForceVPhysicsCollide_00c076c8 + unaff_EBX + 3) = unaff_EBX + 0xc076a7;
  *(undefined4 *)((int)&PTR_GetSoundEmissionOrigin_00c076c0 + unaff_EBX + 3) = 1;
  *(int *)((int)&PTR_WorldSpaceCenter_00c076bc + unaff_EBX + 3) = unaff_EBX + 0xcf46cf;
  return (datamap_t *)((int)&PTR_WorldSpaceCenter_00c076bc + unaff_EBX + 3);
}


/* CLaserRelay::GetDataDescMap at 006d3300 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CLaserRelay * this) */

datamap_t * __thiscall CLaserRelay::GetDataDescMap(CLaserRelay *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5f8e28);
}


/* CLaserRelay::GetBaseMap at 006d3310 */

datamap_t * CLaserRelay::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5f8e00);
}


/* __tcf_2 at 0099af50 */

void __tcf_2(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41e236)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41e22a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41e236));
  }
  *(undefined4 *)(unaff_EBX + 0x41e236) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41e232)) {
    if (*(int *)(unaff_EBX + 0x41e22a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x210c3a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x210c3a),*(int *)(unaff_EBX + 0x41e22a));
      *(undefined4 *)(unaff_EBX + 0x41e22a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e22e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41e22a);
  *(int *)(unaff_EBX + 0x41e23a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41e232)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x210c3a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x210c3a),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41e22a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e22e) = 0;
  }
  return;
}


/* CLaserRelay::GetCatcherModelName at 006d3820 */

/* DWARF original prototype: char * GetCatcherModelName(CLaserRelay * this) */

char * __thiscall CLaserRelay::GetCatcherModelName(CLaserRelay *this)

{
  code *pcVar1;
  int extraout_ECX;
  code *pcVar2;
  
  ___i686_get_pc_thunk_cx();
  pcVar2 = (code *)(this->super_CLaserCatcher).super_CBaseAnimating.super_CBaseEntity.m_ModelName.
                   pszValue;
  pcVar1 = CPostFrameNavigationHook::FrameUpdatePreEntityThink + extraout_ECX + 4;
  if (pcVar2 != (code *)0x0) {
    pcVar1 = pcVar2;
  }
  if (*pcVar1 != (code)0x0) {
    if (pcVar2 == (code *)0x0) {
      pcVar2 = CPostFrameNavigationHook::FrameUpdatePreEntityThink + extraout_ECX + 4;
    }
    return (char *)pcVar2;
  }
  return (char *)(extraout_ECX + 0x33d224);
}


/* CLaserRelay::IsTerminalPoint at 006d3320 */

/* DWARF original prototype: bool IsTerminalPoint(CLaserRelay * this) */

bool __thiscall CLaserRelay::IsTerminalPoint(CLaserRelay *this)

{
  return false;
}


/* _GLOBAL__I__ZN18CPortalLaserTarget9m_DataMapE at 000c4f70 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN18CPortalLaserTarget9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

