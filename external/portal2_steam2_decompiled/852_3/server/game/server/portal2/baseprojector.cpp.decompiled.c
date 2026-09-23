/* DWARF-guided pseudocode for game/server/portal2/baseprojector.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000bd660 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  undefined4 uVar1;
  datamap_t *pdVar2;
  ConVar *unaff_EBX;
  char *unaff_EBP;
  char *unaff_ESI;
  longlong lVar3;
  int unaff_retaddr;
  ConVar *this_1;
  
  this_1 = unaff_EBX;
  lVar3 = ___i686_get_pc_thunk_bx();
  if (lVar3 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)&unaff_EBX[0x25bce].m_pParent + 3) = 0;
  *(undefined1 *)&unaff_EBX[0x25bce].m_pszDefaultValue = 0;
  *(undefined1 *)((int)&unaff_EBX[0x25bce].m_pszDefaultValue + 1) = 0;
  *(undefined1 *)((int)&unaff_EBX[0x25bce].m_pszDefaultValue + 2) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x25bce].m_pszDefaultValue + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x25bce].m_Value.m_pszString + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x25bce].m_Value.m_StringLength + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x25bce].m_Value.m_fValue + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x25bce].m_Value.m_nValue + 3) = 0;
  *(undefined4 *)&unaff_EBX[0x25bce].field_0x37 = 0;
  *(undefined4 *)((int)&unaff_EBX[0x25bce].m_fMinVal + 3) = 0;
  *(undefined4 *)&unaff_EBX[0x25bce].field_0x3f = 0;
  *(undefined4 *)((int)&unaff_EBX[0x25bce].m_fMaxVal + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x25bce].m_fnChangeCallbacks.m_Memory.m_pMemory + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x25bce].m_fnChangeCallbacks.m_Memory.m_nAllocationCount + 3) =
       0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x25bce].m_fnChangeCallbacks.m_Memory.m_nGrowSize + 3) =
       0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x25bce].m_fnChangeCallbacks.m_Size + 3) = 0;
  *(undefined1 *)((int)&unaff_EBX[0x25bcf].super_ConCommandBase._vptr_ConCommandBase + 2) = 0;
  *(undefined1 *)((int)&unaff_EBX[0x25bce].m_fnChangeCallbacks.m_pElements + 3) = 0;
  *(undefined1 *)&unaff_EBX[0x25bcf].super_ConCommandBase._vptr_ConCommandBase = 0;
  *(undefined1 *)((int)&unaff_EBX[0x25bcf].super_ConCommandBase._vptr_ConCommandBase + 1) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x25bcf].super_ConCommandBase._vptr_ConCommandBase + 3) = 0;
  unaff_EBX[0x25bcf].super_ConCommandBase.field_0xa = 0;
  *(undefined1 *)((int)&unaff_EBX[0x25bcf].super_ConCommandBase.m_pNext + 3) = 0;
  unaff_EBX[0x25bcf].super_ConCommandBase.m_bRegistered = false;
  unaff_EBX[0x25bcf].super_ConCommandBase.field_0x9 = 0;
  *(undefined4 *)&unaff_EBX[0x25bcf].super_ConCommandBase.field_0xb = 0;
  *(undefined1 *)((int)&unaff_EBX[0x25bcf].super_ConCommandBase.m_pszHelpString + 2) = 1;
  *(undefined1 *)((int)&unaff_EBX[0x25bcf].super_ConCommandBase.m_pszName + 3) = 0;
  *(undefined1 *)&unaff_EBX[0x25bcf].super_ConCommandBase.m_pszHelpString = 0;
  *(undefined1 *)((int)&unaff_EBX[0x25bcf].super_ConCommandBase.m_pszHelpString + 1) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x25bcf].super_ConCommandBase.m_pszHelpString + 3) = 1;
  *(undefined1 *)((int)&unaff_EBX[0x25bcf].super_IConVar._vptr_IConVar + 2) = 0;
  *(undefined1 *)((int)&unaff_EBX[0x25bcf].super_ConCommandBase.m_nFlags + 3) = 0;
  *(undefined1 *)&unaff_EBX[0x25bcf].super_IConVar._vptr_IConVar = 0;
  *(undefined1 *)((int)&unaff_EBX[0x25bcf].super_IConVar._vptr_IConVar + 1) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x25bcf].super_IConVar._vptr_IConVar + 3) = 2;
  *(undefined1 *)((int)&unaff_EBX[0x25bcf].m_pszDefaultValue + 2) = 0;
  *(undefined1 *)((int)&unaff_EBX[0x25bcf].m_pParent + 3) = 0;
  *(undefined1 *)&unaff_EBX[0x25bcf].m_pszDefaultValue = 0;
  *(undefined1 *)((int)&unaff_EBX[0x25bcf].m_pszDefaultValue + 1) = 0;
  *(undefined1 **)((int)&unaff_EBX[0x25bcf].m_pszDefaultValue + 3) =
       (undefined1 *)((int)&unaff_EBX[0x1fdc7].m_fnChangeCallbacks.m_Memory.m_nGrowSize + 3);
  ConVar::ConVar((ConVar *)((int)&unaff_EBX[0x24314].super_ConCommandBase.m_pszName + 3),
                 (char *)((int)&unaff_EBX[0x1b1a2].m_Value.m_nValue + 3),
                 (char *)((int)&unaff_EBX[0x1a122].super_IConVar._vptr_IConVar + 3),0,this_1,
                 unaff_ESI,unaff_EBP,unaff_retaddr);
  uVar1 = *(undefined4 *)((int)&unaff_EBX[0x1fcc9].super_ConCommandBase.m_pNext + 3);
  ___cxa_atexit((undefined1 *)((int)&unaff_EBX[0x19c40].m_Value.m_fValue + 3),0,uVar1);
  pdVar2 = DataMapInit<CBaseProjector>((CBaseProjector *)0x0);
  *(datamap_t **)((int)&unaff_EBX[0x24315].super_ConCommandBase.m_pszName + 3) = pdVar2;
  *(undefined4 *)((int)&unaff_EBX[0x25bcf].m_Value.m_pszString + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x25bcf].m_Value.m_StringLength + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x25bcf].m_Value.m_fValue + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x25bcf].m_Value.m_nValue + 3) = 0;
  *(undefined4 *)&unaff_EBX[0x25bcf].field_0x37 = 0;
  ___cxa_atexit((undefined1 *)((int)&unaff_EBX[0x19c43].m_Value.m_pszString + 3),0,uVar1);
  return;
}


/* __tcf_1 at 00998ca0 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3958b8),in_stack_00000008);
  return;
}


/* DataMapInit<CBaseProjector> at 000bd360 */

datamap_t * DataMapInit<CBaseProjector>(CBaseProjector *param_1)

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
  char *pcVar9;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  if ((*(char *)((int)&sv_futbol_floor_exit_angle.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX
                ) == '\0') && (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcf9264), iVar2 != 0)) {
    *(int *)((int)&sv_futbol_floor_exit_angle.m_fnChangeCallbacks.m_pElements + unaff_EBX) =
         unaff_EBX + 0x9512b6;
    *(undefined4 *)(&DAT_00cf927c + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&sv_player_trace_through_portals.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) =
         0;
    *(undefined4 *)((int)&sv_player_trace_through_portals.super_ConCommandBase.m_pNext + unaff_EBX)
         = 0;
    *(undefined4 *)(&sv_player_trace_through_portals.super_ConCommandBase.m_bRegistered + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&sv_player_trace_through_portals.super_ConCommandBase.m_pszName + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00cf9278 + unaff_EBX) = 0xe;
    ___cxa_guard_release(unaff_EBX + 0xcf9264);
    ___cxa_atexit(unaff_EBX + 0x8db94c,0,*(undefined4 *)(&DAT_00aee81c + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_ModifyOrAppendDerivedCriteria_00c0ba78 + unaff_EBX) =
       *(undefined4 *)(&DAT_00aee828 + unaff_EBX);
  if ((*(char *)((int)&sv_futbol_floor_exit_angle.m_fnChangeCallbacks.m_Memory.m_nGrowSize +
                unaff_EBX) != '\0') ||
     (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcf926c), iVar2 == 0)) goto LAB_000bd3b4;
  pcVar3 = operator_new___(*(int *)(&DAT_00cf9278 + unaff_EBX) + 0x1b);
  _strcpy(pcVar3,*(char **)((int)&sv_futbol_floor_exit_angle.m_fnChangeCallbacks.m_pElements +
                           unaff_EBX));
  sVar4 = _strlen(pcVar3);
  puVar8 = (undefined4 *)(unaff_EBX + 0x9512c5);
  pcVar9 = pcVar3 + sVar4;
  for (iVar2 = 6; iVar2 != 0; iVar2 = iVar2 + -1) {
    *(undefined4 *)pcVar9 = *puVar8;
    puVar8 = puVar8 + 1;
    pcVar9 = pcVar9 + 4;
  }
  *(undefined2 *)pcVar9 = *(undefined2 *)puVar8;
  pcVar9[2] = *(char *)((int)puVar8 + 2);
  iVar1 = *(int *)(&sv_player_trace_through_portals.super_ConCommandBase.m_bRegistered + unaff_EBX);
  iVar2 = iVar1 + 1;
  iVar5 = *(int *)((int)&sv_player_trace_through_portals.super_ConCommandBase._vptr_ConCommandBase +
                  unaff_EBX);
  if (iVar5 < iVar2) {
    iVar7 = *(int *)((int)&sv_player_trace_through_portals.super_ConCommandBase.m_pNext + unaff_EBX)
    ;
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
        goto LAB_000bd4e0;
      }
      iVar5 = (iVar1 / iVar7 + 1) * iVar7;
      if (iVar5 < iVar2) {
        if (iVar5 != 0) {
LAB_000bd4a0:
          do {
            iVar5 = (iVar5 + iVar2) / 2;
          } while (iVar5 < iVar2);
          goto LAB_000bd4b0;
        }
        if (iVar2 < 0) {
          iVar5 = -1;
          local_20 = -4;
        }
        else {
          local_20 = 0;
          if (iVar2 != 0) goto LAB_000bd4a0;
        }
      }
      else {
LAB_000bd4b0:
        local_20 = iVar5 * 4;
      }
LAB_000bd4e0:
      *(int *)((int)&sv_player_trace_through_portals.super_ConCommandBase._vptr_ConCommandBase +
              unaff_EBX) = iVar5;
      if (*(int *)(&DAT_00cf927c + unaff_EBX) == 0) {
        uVar6 = (*(code *)**(undefined4 **)**(undefined4 **)(&DAT_00aee824 + unaff_EBX))
                          ((undefined4 *)**(undefined4 **)(&DAT_00aee824 + unaff_EBX),local_20);
      }
      else {
        uVar6 = (**(code **)(*(int *)**(undefined4 **)(&DAT_00aee824 + unaff_EBX) + 4))
                          ((int *)**(undefined4 **)(&DAT_00aee824 + unaff_EBX),
                           *(int *)(&DAT_00cf927c + unaff_EBX),local_20);
      }
      *(undefined4 *)(&DAT_00cf927c + unaff_EBX) = uVar6;
    }
  }
  iVar7 = *(int *)(&sv_player_trace_through_portals.super_ConCommandBase.m_bRegistered + unaff_EBX)
          + 1;
  *(int *)(&sv_player_trace_through_portals.super_ConCommandBase.m_bRegistered + unaff_EBX) = iVar7;
  iVar5 = *(int *)(&DAT_00cf927c + unaff_EBX);
  *(int *)((int)&sv_player_trace_through_portals.super_ConCommandBase.m_pszName + unaff_EBX) = iVar5
  ;
  iVar7 = (iVar7 - iVar1) + -1;
  if (0 < iVar7) {
    _V_memmove((void *)(iVar5 + iVar2 * 4),(void *)(iVar5 + iVar1 * 4),iVar7 * 4);
    iVar5 = *(int *)(&DAT_00cf927c + unaff_EBX);
  }
  puVar8 = (undefined4 *)(iVar5 + iVar1 * 4);
  if (puVar8 != (undefined4 *)0x0) {
    *puVar8 = pcVar3;
  }
  *(char **)((int)&PTR_GetServerClass_00c0bbd0 + unaff_EBX) = pcVar3;
  ___cxa_guard_release(unaff_EBX + 0xcf926c);
LAB_000bd3b4:
  *(undefined4 *)((int)&PTR_SetHealth_00c0ba70 + unaff_EBX) = 5;
  *(int *)((int)&PTR_GetMaxHealth_00c0ba6c + unaff_EBX) =
       (int)&PTR_GetSmoothedVelocity_00c0bacc + unaff_EBX;
  return (datamap_t *)((int)&PTR_GetMaxHealth_00c0ba6c + unaff_EBX);
}


/* CBaseProjector::GetDataDescMap at 006aedc0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CBaseProjector * this) */

datamap_t * __thiscall CBaseProjector::GetDataDescMap(CBaseProjector *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x61a018);
}


/* CBaseProjector::GetBaseMap at 006aedd0 */

datamap_t * CBaseProjector::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4fcdc4);
}


/* __tcf_0 at 00998cc0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41d92e)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41d922) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41d92e));
  }
  *(undefined4 *)(unaff_EBX + 0x41d92e) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41d92a)) {
    if (*(int *)(unaff_EBX + 0x41d922) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x212eca) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x212eca),*(int *)(unaff_EBX + 0x41d922));
      *(undefined4 *)(unaff_EBX + 0x41d922) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d926) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41d922);
  *(int *)(unaff_EBX + 0x41d932) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41d92a)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x212eca) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x212eca),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41d922) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d926) = 0;
  }
  return;
}


/* __tcf_2 at 00998da0 */

void __tcf_2(void *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  *(undefined4 *)(unaff_EBX + 0x41d818) = 0;
  if (*(int *)(unaff_EBX + 0x41d814) < 0) {
    iVar1 = *(int *)(unaff_EBX + 0x41d80c);
  }
  else {
    if (*(int *)(unaff_EBX + 0x41d80c) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x212de4) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x212de4),*(int *)(unaff_EBX + 0x41d80c));
      *(undefined4 *)(unaff_EBX + 0x41d80c) = 0;
    }
    iVar1 = 0;
    *(undefined4 *)(unaff_EBX + 0x41d810) = 0;
  }
  *(int *)(unaff_EBX + 0x41d81c) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41d814)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x212de4) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x212de4),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41d80c) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d810) = 0;
  }
  return;
}


/* CBaseProjector::CBaseProjector at 006af180 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CBaseProjector(CBaseProjector * this) */

void __thiscall CBaseProjector::CBaseProjector(CBaseProjector *this)

{
  undefined1 *puVar1;
  int iVar2;
  CBaseEdict *this_00;
  int iVar3;
  IChangeInfoAccessor *pIVar4;
  int iVar5;
  undefined4 uVar6;
  int unaff_EBX;
  int iVar7;
  int iVar8;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::CBaseAnimating(&this->super_CBaseAnimating);
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5800fa);
  (this->m_bEnabled).m_Value = true;
  if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false)
  {
    this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if (this_00 != (CBaseEdict *)0x0) {
      this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
      pIVar4 = CBaseEdict::GetChangeAccessor(this_00);
      pIVar4->m_iChangeInfoSerialNumber = 0;
    }
  }
  else {
    puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  (this->m_hFirstChild).super_CBaseHandle.m_Index = 0xffffffff;
  iVar7 = *(int *)(unaff_EBX + 0x70743e);
  iVar8 = iVar7 + 1;
  iVar5 = *(int *)(unaff_EBX + 0x707436);
  iVar3 = iVar8;
  if (iVar8 <= iVar5) goto LAB_006af1e6;
  iVar2 = *(int *)(unaff_EBX + 0x70743a);
  if (iVar2 < 0) goto LAB_006af1e6;
  if (iVar2 == 0) {
    if ((iVar5 == 0) && (iVar5 = 8, iVar8 < 9)) {
      local_20 = 0x20;
    }
    else {
      do {
        local_20 = iVar5;
        iVar5 = local_20 * 2;
      } while (iVar5 < iVar8);
      local_20 = local_20 * 8;
    }
    goto LAB_006af280;
  }
  iVar5 = (iVar7 / iVar2 + 1) * iVar2;
  if (iVar5 < iVar8) {
    if (iVar5 != 0) {
LAB_006af330:
      do {
        iVar5 = (iVar5 + iVar8) / 2;
      } while (iVar5 < iVar8);
      goto LAB_006af340;
    }
    if (iVar8 < 0) {
      iVar5 = -1;
      local_20 = -4;
    }
    else {
      local_20 = 0;
      if (iVar8 != 0) goto LAB_006af330;
    }
  }
  else {
LAB_006af340:
    local_20 = iVar5 * 4;
  }
LAB_006af280:
  *(int *)(unaff_EBX + 0x707436) = iVar5;
  if (*(int *)(unaff_EBX + 0x707432) == 0) {
    uVar6 = (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x4fca0a))
                      ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x4fca0a),local_20);
  }
  else {
    uVar6 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4fca0a) + 4))
                      ((int *)**(undefined4 **)(unaff_EBX + 0x4fca0a),*(int *)(unaff_EBX + 0x707432)
                       ,local_20);
  }
  *(undefined4 *)(unaff_EBX + 0x707432) = uVar6;
  iVar3 = *(int *)(unaff_EBX + 0x70743e) + 1;
LAB_006af1e6:
  *(int *)(unaff_EBX + 0x70743e) = iVar3;
  iVar5 = *(int *)(unaff_EBX + 0x707432);
  *(int *)(unaff_EBX + 0x707442) = iVar5;
  iVar3 = (iVar3 - iVar7) + -1;
  if (iVar3 < 1) {
    iVar7 = iVar7 << 2;
  }
  else {
    iVar7 = iVar7 * 4;
    _V_memmove((void *)(iVar5 + iVar8 * 4),(void *)(iVar5 + iVar7),iVar3 * 4);
    iVar5 = *(int *)(unaff_EBX + 0x707432);
  }
  if ((undefined4 *)(iVar7 + iVar5) != (undefined4 *)0x0) {
    *(undefined4 *)(iVar7 + iVar5) = this;
  }
  return;
}


/* CBaseProjector::CBaseProjector at 006af3a0 */

/* DWARF original prototype: void CBaseProjector(CBaseProjector * this, CBaseProjector * this) */

void __thiscall CBaseProjector::CBaseProjector(CBaseProjector *this,CBaseProjector *this_1)

{
  CBaseProjector(this);
  return;
}


/* CBaseProjector::~CBaseProjector at 006af480 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CBaseProjector(CBaseProjector * this, int __in_chrg) */

void __thiscall CBaseProjector::~CBaseProjector(CBaseProjector *this,int __in_chrg)

{
  uint uVar1;
  CBaseEntity *oldObj;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x57fdf4);
  uVar1 = (this->m_hFirstChild).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4fc710),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (oldObj = *(CBaseEntity **)(iVar2 + 4), oldObj != (CBaseEntity *)0x0)) {
    UTIL_Remove(oldObj);
  }
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
  return;
}


/* CBaseProjector::~CBaseProjector at 006af6c0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CBaseProjector(CBaseProjector * this, int __in_chrg) */

void __thiscall CBaseProjector::~CBaseProjector(CBaseProjector *this,int __in_chrg)

{
  uint uVar1;
  CBaseEntity *oldObj;
  int iVar2;
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x57fbb4);
  uVar1 = (this->m_hFirstChild).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4fc4d0),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (oldObj = *(CBaseEntity **)(iVar2 + 4), oldObj != (CBaseEntity *)0x0)) {
    UTIL_Remove(oldObj);
  }
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,in_stack_ffffffd8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CBaseProjector::~CBaseProjector at 006af760 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CBaseProjector(CBaseProjector * this, int __in_chrg) */

void __thiscall CBaseProjector::~CBaseProjector(CBaseProjector *this,int __in_chrg)

{
  uint uVar1;
  CBaseEntity *oldObj;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x57fb14);
  uVar1 = (this->m_hFirstChild).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4fc430),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (oldObj = *(CBaseEntity **)(iVar2 + 4), oldObj != (CBaseEntity *)0x0)) {
    UTIL_Remove(oldObj);
  }
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
  return;
}


/* CBaseProjector::UpdateOnRemove at 006af5f0 */

/* DWARF original prototype: void UpdateOnRemove(CBaseProjector * this) */

void __thiscall CBaseProjector::UpdateOnRemove(CBaseProjector *this)

{
  int *piVar1;
  uint uVar2;
  CBaseEntity *oldObj;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  iVar5 = *(int *)(unaff_EBX + 0x706fce);
  if (0 < iVar5) {
    piVar1 = *(int **)(unaff_EBX + 0x706fc2);
    if (this == (CBaseProjector *)*piVar1) {
      iVar4 = 0;
      iVar6 = 0;
    }
    else {
      iVar4 = 0;
      iVar3 = 4;
      do {
        iVar6 = iVar3;
        iVar4 = iVar4 + 1;
        if (iVar4 == iVar5) goto LAB_006af646;
        iVar3 = iVar6 + 4;
      } while (this != (CBaseProjector *)piVar1[iVar4]);
    }
    if (iVar4 != iVar5 + -1) {
      *(int *)(iVar6 + (int)piVar1) = piVar1[iVar5 + -1];
      iVar4 = *(int *)(unaff_EBX + 0x706fce) + -1;
    }
    *(int *)(unaff_EBX + 0x706fce) = iVar4;
  }
LAB_006af646:
  uVar2 = (this->m_hFirstChild).super_CBaseHandle.m_Index;
  if (((uVar2 != 0xffffffff) &&
      (iVar5 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4fc5a6),
      *(uint *)(iVar5 + 8) == uVar2 >> 0x10)) &&
     (oldObj = *(CBaseEntity **)(iVar5 + 4), oldObj != (CBaseEntity *)0x0)) {
    UTIL_Remove(oldObj);
  }
  CBaseEntity::UpdateOnRemove((CBaseEntity *)this);
  return;
}


/* CBaseProjector::TestProjectionChangesThink at 006af3b0 */

/* DWARF original prototype: void TestProjectionChangesThink(CBaseProjector * this) */

void __thiscall CBaseProjector::TestProjectionChangesThink(CBaseProjector *this)

{
  int *piVar1;
  uint uVar2;
  int *piVar3;
  BASEPTR func;
  int iVar4;
  int unaff_EBX;
  int iVar5;
  
  ___i686_get_pc_thunk_bx();
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0xe);
  CBaseEntity::ThinkSet
            ((CBaseEntity *)this,func,
             *(float *)(&DAT_0037c486 + unaff_EBX) +
             *(float *)(**(int **)(unaff_EBX + 0x4fc7e2) + 0xc),&UNK_0035f256 + unaff_EBX);
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x70720e)) {
    iVar5 = 0;
    piVar1 = *(int **)(unaff_EBX + 0x4fc7e6);
    do {
      while( true ) {
                    /* Unresolved local var: CBaseProjectedEntity * pFirstChild@[???]
                       Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
        uVar2 = *(uint *)(*(int *)(*(int *)(unaff_EBX + 0x707202) + iVar5 * 4) + 0x518);
        if (((uVar2 == 0xffffffff) ||
            (iVar4 = (uVar2 & 0xffff) * 0x10 + *piVar1, *(uint *)(iVar4 + 8) != uVar2 >> 0x10)) ||
           (piVar3 = *(int **)(iVar4 + 4), piVar3 == (int *)0x0)) break;
        (**(code **)(*piVar3 + 0x340))(piVar3);
        iVar5 = iVar5 + 1;
        if (*(int *)(unaff_EBX + 0x70720e) <= iVar5) {
          return;
        }
      }
      iVar5 = iVar5 + 1;
    } while (iVar5 < *(int *)(unaff_EBX + 0x70720e));
  }
  return;
}


/* CBaseProjector::Spawn at 006aef30 */

/* DWARF original prototype: void Spawn(CBaseProjector * this) */

void __thiscall CBaseProjector::Spawn(CBaseProjector *this)

{
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::Spawn(&this->super_CBaseAnimating);
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0x46f);
  CBaseEntity::ThinkSet
            ((CBaseEntity *)this,func,
             *(float *)(unaff_EBX + 0x37c903) + *(float *)(**(int **)(unaff_EBX + 0x4fcc5f) + 0xc),
             (char *)(unaff_EBX + 0x35f6d3));
  return;
}


/* CBaseProjector::Activate at 006aefb0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Activate(CBaseProjector * this) */

void __thiscall CBaseProjector::Activate(CBaseProjector *this)

{
  CBaseAnimating::Activate(&this->super_CBaseAnimating);
  if ((this->m_bEnabled).m_Value == false) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x006aeff0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0xe9])();
  return;
}


/* CBaseProjector::OnRestore at 006aeee0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void OnRestore(CBaseProjector * this) */

void __thiscall CBaseProjector::OnRestore(CBaseProjector *this)

{
  CBaseAnimating::OnRestore(&this->super_CBaseAnimating);
  if ((this->m_bEnabled).m_Value == false) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x006aef20. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0xe9])();
  return;
}


/* CBaseProjector::Project at 006af000 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void Project(CBaseProjector * this) */

void __thiscall CBaseProjector::Project(CBaseProjector *this)

{
  byte bVar1;
  int *piVar2;
  uint uVar3;
  int iVar4;
  int *piVar5;
  uint *puVar6;
  CBaseProjectedEntity *this_00;
  int unaff_EBX;
  QAngle *local_24;
  bool local_1d;
  
                    /* Unresolved local var: bool bCreateNew@[???] */
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
  piVar2 = *(int **)(CNPCBaseInteractive<CAI_BaseNPC>::NotifyInteraction + unaff_EBX + 6);
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
  uVar3 = (this->m_hFirstChild).super_CBaseHandle.m_Index;
  if ((uVar3 == 0xffffffff) ||
     (iVar4 = (uVar3 & 0xffff) * 0x10 + *piVar2, *(uint *)(iVar4 + 8) != uVar3 >> 0x10)) {
    local_1d = true;
  }
  else {
    local_1d = *(int *)(iVar4 + 4) == 0;
    if (!local_1d) {
      bVar1 = *(byte *)((int)&(this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags + 1);
      goto joined_r0x006af08a;
    }
  }
  piVar5 = (int *)(*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.
                    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xea])(this);
  if (piVar5 == (int *)0x0) {
    (this->m_hFirstChild).super_CBaseHandle.m_Index = 0xffffffff;
    return;
  }
  puVar6 = (uint *)(**(code **)(*piVar5 + 0xc))(piVar5);
  uVar3 = *puVar6;
  (this->m_hFirstChild).super_CBaseHandle.m_Index = uVar3;
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
  if (uVar3 == 0xffffffff) {
    return;
  }
  iVar4 = (uVar3 & 0xffff) * 0x10 + *piVar2;
  if (*(uint *)(iVar4 + 8) != uVar3 >> 0x10) {
    return;
  }
  if (*(int *)(iVar4 + 4) == 0) {
    return;
  }
  bVar1 = *(byte *)((int)&(this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags + 1);
joined_r0x006af08a:
  if (((bVar1 & 8) != 0) &&
     (CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this),
     ((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0)) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
  local_24 = &(this->super_CBaseAnimating).super_CBaseEntity.m_angAbsRotation;
  uVar3 = (this->m_hFirstChild).super_CBaseHandle.m_Index;
  if ((uVar3 == 0xffffffff) ||
     (iVar4 = (uVar3 & 0xffff) * 0x10 + *piVar2, *(uint *)(iVar4 + 8) != uVar3 >> 0x10)) {
    this_00 = (CBaseProjectedEntity *)0x0;
  }
  else {
    this_00 = *(CBaseProjectedEntity **)(iVar4 + 4);
  }
  CBaseProjectedEntity::RecursiveProjection
            (this_00,local_1d,this,(CPortal_Base2D *)0x0,
             &(this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin,local_24,0x10);
  return;
}


/* CBaseProjector::Cleanup at 006aee80 */

/* DWARF original prototype: void Cleanup(CBaseProjector * this) */

void __thiscall CBaseProjector::Cleanup(CBaseProjector *this)

{
  uint uVar1;
  CBaseEntity *oldObj;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->m_hFirstChild).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4fcd1b),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (oldObj = *(CBaseEntity **)(iVar2 + 4), oldObj != (CBaseEntity *)0x0)) {
    UTIL_Remove(oldObj);
    return;
  }
  return;
}


/* CBaseProjector::EnableProjection at 006af510 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void EnableProjection(CBaseProjector * this, bool bEnable) */

void __thiscall CBaseProjector::EnableProjection(CBaseProjector *this,bool bEnable)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  uint uVar2;
  CBaseEntity *oldObj;
  char cVar3;
  IChangeInfoAccessor *pIVar4;
  int iVar5;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  cVar3 = (this->m_bEnabled).m_Value;
  if (bEnable != (bool)cVar3) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
      ;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar4 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar4->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bEnabled).m_Value = bEnable;
    cVar3 = bEnable;
  }
  if (cVar3 == '\0') {
    uVar2 = (this->m_hFirstChild).super_CBaseHandle.m_Index;
    if (((uVar2 != 0xffffffff) &&
        (iVar5 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4fc687),
        *(uint *)(iVar5 + 8) == uVar2 >> 0x10)) &&
       (oldObj = *(CBaseEntity **)(iVar5 + 4), oldObj != (CBaseEntity *)0x0)) {
      UTIL_Remove(oldObj);
      return;
    }
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x006af5e1. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0xe9])();
  return;
}


/* CBaseProjector::InputDisable at 006af7f0 */

/* DWARF original prototype: void InputDisable(CBaseProjector * this, inputdata_t * data) */

void __thiscall CBaseProjector::InputDisable(CBaseProjector *this,inputdata_t *data)

{
  undefined1 *puVar1;
  uint uVar2;
  CBaseEntity *oldObj;
  CBaseEdict *this_00;
  int iVar3;
  IChangeInfoAccessor *pIVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((this->m_bEnabled).m_Value != false) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
      ;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar4 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar4->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bEnabled).m_Value = false;
  }
  uVar2 = (this->m_hFirstChild).super_CBaseHandle.m_Index;
  if (((uVar2 != 0xffffffff) &&
      (iVar3 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4fc3a3),
      *(uint *)(iVar3 + 8) == uVar2 >> 0x10)) &&
     (oldObj = *(CBaseEntity **)(iVar3 + 4), oldObj != (CBaseEntity *)0x0)) {
    UTIL_Remove(oldObj);
    return;
  }
  return;
}


/* CBaseProjector::DrawDebugTextOverlays at 006aee70 */

/* DWARF original prototype: int DrawDebugTextOverlays(CBaseProjector * this) */

int __thiscall CBaseProjector::DrawDebugTextOverlays(CBaseProjector *this)

{
  int iVar1;
  
                    /* Unresolved local var: int text_offset@[???] */
  iVar1 = CBaseAnimating::DrawDebugTextOverlays(&this->super_CBaseAnimating);
  return iVar1;
}


/* CBaseProjector::TestAllForProjectionChanges at 006aede0 */

void CBaseProjector::TestAllForProjectionChanges(void)

{
  int *piVar1;
  uint uVar2;
  int *piVar3;
  int iVar4;
  int unaff_EBX;
  int iVar5;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x7077de)) {
    iVar5 = 0;
    piVar1 = *(int **)(unaff_EBX + 0x4fcdb6);
    do {
      while( true ) {
                    /* Unresolved local var: CBaseProjectedEntity * pFirstChild@[???]
                       Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
        uVar2 = *(uint *)(*(int *)(*(int *)(unaff_EBX + 0x7077d2) + iVar5 * 4) + 0x518);
        if (((uVar2 == 0xffffffff) ||
            (iVar4 = (uVar2 & 0xffff) * 0x10 + *piVar1, *(uint *)(iVar4 + 8) != uVar2 >> 0x10)) ||
           (piVar3 = *(int **)(iVar4 + 4), piVar3 == (int *)0x0)) break;
        (**(code **)(*piVar3 + 0x340))(piVar3);
        iVar5 = iVar5 + 1;
        if (*(int *)(unaff_EBX + 0x7077de) <= iVar5) {
          return;
        }
      }
      iVar5 = iVar5 + 1;
    } while (iVar5 < *(int *)(unaff_EBX + 0x7077de));
  }
  return;
}


/* CBaseProjector::InputEnable at 006af890 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void InputEnable(CBaseProjector * this, inputdata_t * data) */

void __thiscall CBaseProjector::InputEnable(CBaseProjector *this,inputdata_t *data)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  
  if ((this->m_bEnabled).m_Value == false) {
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
    (this->m_bEnabled).m_Value = true;
  }
                    /* WARNING: Could not recover jumptable at 0x006af8e7. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0xe9])();
  return;
}


/* _GLOBAL__I_sv_projected_entities_use_placement_helper at 000bd840 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_sv_projected_entities_use_placement_helper(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

