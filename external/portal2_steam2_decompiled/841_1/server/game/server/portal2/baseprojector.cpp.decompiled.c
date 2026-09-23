/* DWARF-guided pseudocode for game/server/portal2/baseprojector.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000a8330 */

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
  *(undefined1 *)((int)&unaff_EBX[0x27ac0].m_pParent + 3) = 0;
  *(undefined1 *)&unaff_EBX[0x27ac0].m_pszDefaultValue = 0;
  *(undefined1 *)((int)&unaff_EBX[0x27ac0].m_pszDefaultValue + 1) = 0;
  *(undefined1 *)((int)&unaff_EBX[0x27ac0].m_pszDefaultValue + 2) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x27ac0].m_pszDefaultValue + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x27ac0].m_Value.m_pszString + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x27ac0].m_Value.m_StringLength + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x27ac0].m_Value.m_fValue + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x27ac0].m_Value.m_nValue + 3) = 0;
  *(undefined4 *)&unaff_EBX[0x27ac0].field_0x37 = 0;
  *(undefined4 *)((int)&unaff_EBX[0x27ac0].m_fMinVal + 3) = 0;
  *(undefined4 *)&unaff_EBX[0x27ac0].field_0x3f = 0;
  *(undefined4 *)((int)&unaff_EBX[0x27ac0].m_fMaxVal + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x27ac0].m_fnChangeCallbacks.m_Memory.m_pMemory + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x27ac0].m_fnChangeCallbacks.m_Memory.m_nAllocationCount + 3) =
       0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x27ac0].m_fnChangeCallbacks.m_Memory.m_nGrowSize + 3) =
       0x7f7fffff;
  *(undefined1 **)((int)&unaff_EBX[0x27ac0].m_fnChangeCallbacks.m_Size + 3) =
       (undefined1 *)((int)&unaff_EBX[0x21dd0].m_fnChangeCallbacks.m_Memory.m_pMemory + 3);
  ConVar::ConVar((ConVar *)((int)&unaff_EBX[0x262df].super_ConCommandBase.m_nFlags + 3),
                 (char *)((int)&unaff_EBX[0x1d2e7].m_fnChangeCallbacks.m_Memory.m_pMemory + 3),
                 (char *)((int)&unaff_EBX[0x1c2a6].m_Value.m_StringLength + 3),0,this_1,unaff_ESI,
                 unaff_EBP,unaff_retaddr);
  uVar1 = *(undefined4 *)((int)&unaff_EBX[0x21cd1].m_fMinVal + 3);
  ___cxa_atexit((undefined1 *)((int)&unaff_EBX[0x1bdbf].super_ConCommandBase.m_pNext + 3),0,uVar1);
  pdVar2 = DataMapInit<CBaseProjector>((CBaseProjector *)0x0);
  *(datamap_t **)((int)&unaff_EBX[0x262e0].super_ConCommandBase.m_nFlags + 3) = pdVar2;
  *(undefined4 *)((int)&unaff_EBX[0x27ac0].m_fnChangeCallbacks.m_pElements + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x27ac1].super_ConCommandBase._vptr_ConCommandBase + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x27ac1].super_ConCommandBase.m_pNext + 3) = 0;
  *(undefined4 *)&unaff_EBX[0x27ac1].super_ConCommandBase.field_0xb = 0;
  *(undefined4 *)((int)&unaff_EBX[0x27ac1].super_ConCommandBase.m_pszName + 3) = 0;
  ___cxa_atexit((undefined1 *)((int)&unaff_EBX[0x1bdbf].m_Value.m_pszString + 3),0,uVar1);
  return;
}


/* __tcf_1 at 00a3bcf0 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38c308),in_stack_00000008);
  return;
}


/* DataMapInit<CBaseProjector> at 000a8030 */

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
  if ((*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x598].m_pPrev + unaff_EBX) ==
       '\0') && (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xda356c), iVar2 != 0)) {
    *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x599].m_pPrev + unaff_EBX) =
         unaff_EBX + 0xa08282;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x59a].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x59a].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x59a].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x59a].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x59b].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x599].m_pNext + unaff_EBX) =
         0xe;
    ___cxa_guard_release(unaff_EBX + 0xda356c);
    ___cxa_atexit(unaff_EBX + 0x993dac,0,*(undefined4 *)(&DAT_00b9eb10 + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_ForceVPhysicsCollide_00cba948 + unaff_EBX) =
       *(undefined4 *)(&DAT_00b9eb1c + unaff_EBX);
  if ((*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x599].m_pEntity + unaff_EBX) !=
       '\0') || (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xda3574), iVar2 == 0)) goto LAB_000a8084;
  pcVar3 = operator_new___(*(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x599].
                                          m_pNext + unaff_EBX) + 0x1b);
  _strcpy(pcVar3,*(char **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x599].m_pPrev +
                           unaff_EBX));
  sVar4 = _strlen(pcVar3);
  puVar8 = (undefined4 *)(unaff_EBX + 0xa08291);
  pcVar9 = pcVar3 + sVar4;
  for (iVar2 = 6; iVar2 != 0; iVar2 = iVar2 + -1) {
    *(undefined4 *)pcVar9 = *puVar8;
    puVar8 = puVar8 + 1;
    pcVar9 = pcVar9 + 4;
  }
  *(undefined2 *)pcVar9 = *(undefined2 *)puVar8;
  pcVar9[2] = *(char *)((int)puVar8 + 2);
  iVar1 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x59a].m_pNext + unaff_EBX);
  iVar2 = iVar1 + 1;
  iVar5 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x59a].m_SerialNumber +
                  unaff_EBX);
  if (iVar5 < iVar2) {
    iVar7 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x59a].m_pPrev + unaff_EBX);
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
        goto LAB_000a81b0;
      }
      iVar5 = (iVar1 / iVar7 + 1) * iVar7;
      if (iVar5 < iVar2) {
        if (iVar5 != 0) {
LAB_000a8170:
          do {
            iVar5 = (iVar5 + iVar2) / 2;
          } while (iVar5 < iVar2);
          goto LAB_000a8180;
        }
        if (iVar2 < 0) {
          iVar5 = -1;
          local_20 = -4;
        }
        else {
          local_20 = 0;
          if (iVar2 != 0) goto LAB_000a8170;
        }
      }
      else {
LAB_000a8180:
        local_20 = iVar5 * 4;
      }
LAB_000a81b0:
      *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x59a].m_SerialNumber + unaff_EBX)
           = iVar5;
      iVar5 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x59a].m_pEntity +
                      unaff_EBX);
      if (iVar5 == 0) {
        uVar6 = (*(code *)**(undefined4 **)**(undefined4 **)(&DAT_00b9eb18 + unaff_EBX))
                          ((undefined4 *)**(undefined4 **)(&DAT_00b9eb18 + unaff_EBX),local_20);
      }
      else {
        uVar6 = (**(code **)(*(int *)**(undefined4 **)(&DAT_00b9eb18 + unaff_EBX) + 4))
                          ((int *)**(undefined4 **)(&DAT_00b9eb18 + unaff_EBX),iVar5,local_20);
      }
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x59a].m_pEntity + unaff_EBX) = uVar6;
    }
  }
  iVar7 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x59a].m_pNext + unaff_EBX) +
          1;
  *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x59a].m_pNext + unaff_EBX) = iVar7;
  iVar5 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x59a].m_pEntity + unaff_EBX);
  *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x59b].m_pEntity + unaff_EBX) = iVar5;
  iVar7 = (iVar7 - iVar1) + -1;
  if (0 < iVar7) {
    _V_memmove((void *)(iVar5 + iVar2 * 4),(void *)(iVar5 + iVar1 * 4),iVar7 * 4);
    iVar5 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x59a].m_pEntity + unaff_EBX
                    );
  }
  puVar8 = (undefined4 *)(iVar5 + iVar1 * 4);
  if (puVar8 != (undefined4 *)0x0) {
    *puVar8 = pcVar3;
  }
  *(char **)((int)&PTR_SetParent_00cbaae0 + unaff_EBX) = pcVar3;
  ___cxa_guard_release(unaff_EBX + 0xda3574);
LAB_000a8084:
  *(undefined4 *)((int)&PTR_GetSoundEmissionOrigin_00cba940 + unaff_EBX) = 6;
  *(int *)((int)&PTR_WorldSpaceCenter_00cba93c + unaff_EBX) =
       (int)&PTR_NetworkStateChanged_m_nWaterLevel_00cba99c + unaff_EBX;
  return (datamap_t *)((int)&PTR_WorldSpaceCenter_00cba93c + unaff_EBX);
}


/* CBaseProjector::GetDataDescMap at 007076f0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CBaseProjector * this) */

datamap_t * __thiscall CBaseProjector::GetDataDescMap(CBaseProjector *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x65b288);
}


/* CBaseProjector::GetBaseMap at 00707700 */

datamap_t * CBaseProjector::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(&DAT_0053f458 + extraout_ECX);
}


/* __tcf_0 at 00a3bdf0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x40f7d6)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x40f7ca) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x40f7d6));
  }
  *(undefined4 *)(unaff_EBX + 0x40f7d6) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40f7d2)) {
    if (*(int *)(unaff_EBX + 0x40f7ca) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20ad5e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20ad5e),*(int *)(unaff_EBX + 0x40f7ca));
      *(undefined4 *)(unaff_EBX + 0x40f7ca) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f7ce) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40f7ca);
  *(int *)(unaff_EBX + 0x40f7da) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40f7d2)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20ad5e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20ad5e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40f7ca) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f7ce) = 0;
  }
  return;
}


/* __tcf_2 at 00a3bd10 */

void __tcf_2(void *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  *(undefined4 *)(unaff_EBX + 0x40f880) = 0;
  if (*(int *)(&DAT_0040f87c + unaff_EBX) < 0) {
    iVar1 = *(int *)(unaff_EBX + 0x40f874);
  }
  else {
    if (*(int *)(unaff_EBX + 0x40f874) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20ae38) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20ae38),*(int *)(unaff_EBX + 0x40f874));
      *(undefined4 *)(unaff_EBX + 0x40f874) = 0;
    }
    iVar1 = 0;
    *(undefined4 *)(unaff_EBX + 0x40f878) = 0;
  }
  *(int *)(unaff_EBX + 0x40f884) = iVar1;
  if (-1 < *(int *)(&DAT_0040f87c + unaff_EBX)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20ae38) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20ae38),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40f874) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f878) = 0;
  }
  return;
}


/* CBaseProjector::CBaseProjector at 00707ae0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CBaseProjector(CBaseProjector * this) */

void __thiscall CBaseProjector::CBaseProjector(CBaseProjector *this)

{
  undefined1 *puVar1;
  uint uVar2;
  ushort uVar3;
  int iVar4;
  CBaseEdict *this_00;
  int *piVar5;
  ushort *puVar6;
  int iVar7;
  IChangeInfoAccessor *pIVar8;
  int iVar9;
  undefined4 uVar10;
  uint uVar11;
  int unaff_EBX;
  int iVar12;
  uint uVar13;
  int iVar14;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::CBaseAnimating(&this->super_CBaseAnimating);
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5bfd7a);
  (this->m_bEnabled).m_Value = true;
  if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false)
  {
    this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
      this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
      pIVar8 = CBaseEdict::GetChangeAccessor(this_00);
      piVar5 = *(int **)(&DAT_0053f07a + unaff_EBX);
      puVar6 = (ushort *)*piVar5;
      if (pIVar8->m_iChangeInfoSerialNumber == *puVar6) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        uVar13 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar3 = puVar6[uVar13 * 0x14 + 0x14];
        if (uVar3 == 0) {
LAB_00707da6:
          puVar6[(uint)uVar3 + uVar13 * 0x14 + 1] = 0x514;
          puVar6[uVar13 * 0x14 + 0x14] = uVar3 + 1;
        }
        else if (puVar6[uVar13 * 0x14 + 1] != 0x514) {
          uVar11 = 0;
          do {
            uVar2 = uVar11 + 1;
            uVar11 = uVar2 & 0xffff;
            if ((ushort)uVar2 == uVar3) {
              if (uVar3 == 0x13) goto LAB_00707dc8;
              goto LAB_00707da6;
            }
          } while (puVar6[uVar13 * 0x14 + uVar11 + 1] != 0x514);
        }
      }
      else if (puVar6[0x7d1] == 100) {
LAB_00707dc8:
        pIVar8->m_iChangeInfoSerialNumber = 0;
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar8->m_iChangeInfo = puVar6[0x7d1];
        *(short *)(*piVar5 + 0xfa2) = *(short *)(*piVar5 + 0xfa2) + 1;
        pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
        iVar12 = *piVar5 + (uint)pIVar8->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar12 + 2) = 0x514;
        *(undefined2 *)(iVar12 + 0x28) = 1;
      }
    }
  }
  else {
    puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  (this->m_hFirstChild).super_CBaseHandle.m_Index = 0xffffffff;
  iVar12 = *(int *)(unaff_EBX + 0x743ab6);
  iVar14 = iVar12 + 1;
  iVar9 = *(int *)(unaff_EBX + 0x743aae);
  iVar7 = iVar14;
  if (iVar14 <= iVar9) goto LAB_00707b52;
  iVar4 = *(int *)(unaff_EBX + 0x743ab2);
  if (iVar4 < 0) goto LAB_00707b52;
  if (iVar4 == 0) {
    if ((iVar9 == 0) && (iVar9 = 8, iVar14 < 9)) {
      local_20 = 0x20;
    }
    else {
      do {
        local_20 = iVar9;
        iVar9 = local_20 * 2;
      } while (iVar9 < iVar14);
      local_20 = local_20 * 8;
    }
    goto LAB_00707c60;
  }
  iVar9 = (iVar12 / iVar4 + 1) * iVar4;
  if (iVar9 < iVar14) {
    if (iVar9 != 0) {
LAB_00707d10:
      do {
        iVar9 = (iVar9 + iVar14) / 2;
      } while (iVar9 < iVar14);
      goto LAB_00707d20;
    }
    if (iVar14 < 0) {
      iVar9 = -1;
      local_20 = -4;
    }
    else {
      local_20 = 0;
      if (iVar14 != 0) goto LAB_00707d10;
    }
  }
  else {
LAB_00707d20:
    local_20 = iVar9 * 4;
  }
LAB_00707c60:
  *(int *)(unaff_EBX + 0x743aae) = iVar9;
  if (*(int *)(unaff_EBX + 0x743aaa) == 0) {
    uVar10 = (*(code *)**(undefined4 **)**(undefined4 **)(&DAT_0053f06e + unaff_EBX))
                       ((undefined4 *)**(undefined4 **)(&DAT_0053f06e + unaff_EBX),local_20);
  }
  else {
    uVar10 = (**(code **)(*(int *)**(undefined4 **)(&DAT_0053f06e + unaff_EBX) + 4))
                       ((int *)**(undefined4 **)(&DAT_0053f06e + unaff_EBX),
                        *(int *)(unaff_EBX + 0x743aaa),local_20);
  }
  *(undefined4 *)(unaff_EBX + 0x743aaa) = uVar10;
  iVar7 = *(int *)(unaff_EBX + 0x743ab6) + 1;
LAB_00707b52:
  *(int *)(unaff_EBX + 0x743ab6) = iVar7;
  iVar9 = *(int *)(unaff_EBX + 0x743aaa);
  *(int *)(unaff_EBX + 0x743aba) = iVar9;
  iVar7 = (iVar7 - iVar12) + -1;
  if (iVar7 < 1) {
    iVar12 = iVar12 << 2;
  }
  else {
    iVar12 = iVar12 * 4;
    _V_memmove((void *)(iVar9 + iVar14 * 4),(void *)(iVar9 + iVar12),iVar7 * 4);
    iVar9 = *(int *)(unaff_EBX + 0x743aaa);
  }
  if ((undefined4 *)(iVar9 + iVar12) != (undefined4 *)0x0) {
    *(undefined4 *)(iVar9 + iVar12) = this;
  }
  return;
}


/* CBaseProjector::CBaseProjector at 00707e10 */

/* DWARF original prototype: void CBaseProjector(CBaseProjector * this, CBaseProjector * this) */

void __thiscall CBaseProjector::CBaseProjector(CBaseProjector *this,CBaseProjector *this_1)

{
  CBaseProjector(this);
  return;
}


/* CBaseProjector::~CBaseProjector at 007081f0 */

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
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5bf664);
  uVar1 = (this->m_hFirstChild).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x53e968),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (oldObj = *(CBaseEntity **)(iVar2 + 4), oldObj != (CBaseEntity *)0x0)) {
    UTIL_Remove(oldObj);
    (this->m_hFirstChild).super_CBaseHandle.m_Index = 0xffffffff;
  }
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,in_stack_ffffffd8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CBaseProjector::~CBaseProjector at 00708290 */

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
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5bf5c4);
  uVar1 = (this->m_hFirstChild).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(&DAT_0053e8c8 + unaff_EBX),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (oldObj = *(CBaseEntity **)(iVar2 + 4), oldObj != (CBaseEntity *)0x0)) {
    UTIL_Remove(oldObj);
    (this->m_hFirstChild).super_CBaseHandle.m_Index = 0xffffffff;
  }
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
  return;
}


/* CBaseProjector::~CBaseProjector at 00708320 */

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
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5bf534);
  uVar1 = (this->m_hFirstChild).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x53e838),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (oldObj = *(CBaseEntity **)(iVar2 + 4), oldObj != (CBaseEntity *)0x0)) {
    UTIL_Remove(oldObj);
    (this->m_hFirstChild).super_CBaseHandle.m_Index = 0xffffffff;
  }
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
  return;
}


/* CBaseProjector::UpdateOnRemove at 00708120 */

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
  iVar5 = *(int *)(unaff_EBX + 0x743476);
  if (0 < iVar5) {
    piVar1 = *(int **)(unaff_EBX + 0x74346a);
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
        if (iVar4 == iVar5) goto LAB_00708176;
        iVar3 = iVar6 + 4;
      } while (this != (CBaseProjector *)piVar1[iVar4]);
    }
    if (iVar4 != iVar5 + -1) {
      *(int *)(iVar6 + (int)piVar1) = piVar1[iVar5 + -1];
      iVar4 = *(int *)(unaff_EBX + 0x743476) + -1;
    }
    *(int *)(unaff_EBX + 0x743476) = iVar4;
  }
LAB_00708176:
  uVar2 = (this->m_hFirstChild).super_CBaseHandle.m_Index;
  if (((uVar2 != 0xffffffff) &&
      (iVar5 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x53ea3e),
      *(uint *)(iVar5 + 8) == uVar2 >> 0x10)) &&
     (oldObj = *(CBaseEntity **)(iVar5 + 4), oldObj != (CBaseEntity *)0x0)) {
    UTIL_Remove(oldObj);
    (this->m_hFirstChild).super_CBaseHandle.m_Index = 0xffffffff;
  }
  CBaseEntity::UpdateOnRemove((CBaseEntity *)this);
  return;
}


/* CBaseProjector::TestProjectionChangesThink at 00707e20 */

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
             *(float *)(unaff_EBX + 0x3c6016) + *(float *)(**(int **)(unaff_EBX + 0x53ed36) + 0xc),
             (char *)(unaff_EBX + 0x3a8482));
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x743776)) {
    iVar5 = 0;
    piVar1 = *(int **)(unaff_EBX + 0x53ed3e);
    do {
      while( true ) {
                    /* Unresolved local var: CBaseProjectedEntity * pFirstChild@[???]
                       Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
        uVar2 = *(uint *)(*(int *)(*(int *)(unaff_EBX + 0x74376a) + iVar5 * 4) + 0x518);
        if (((uVar2 == 0xffffffff) ||
            (iVar4 = (uVar2 & 0xffff) * 0x10 + *piVar1, *(uint *)(iVar4 + 8) != uVar2 >> 0x10)) ||
           (piVar3 = *(int **)(iVar4 + 4), piVar3 == (int *)0x0)) break;
        (**(code **)(*piVar3 + 0x340))(piVar3);
        iVar5 = iVar5 + 1;
        if (*(int *)(unaff_EBX + 0x743776) <= iVar5) {
          return;
        }
      }
      iVar5 = iVar5 + 1;
    } while (iVar5 < *(int *)(unaff_EBX + 0x743776));
  }
  return;
}


/* CBaseProjector::Spawn at 00707880 */

/* DWARF original prototype: void Spawn(CBaseProjector * this) */

void __thiscall CBaseProjector::Spawn(CBaseProjector *this)

{
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::Spawn(&this->super_CBaseAnimating);
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0x58f);
  CBaseEntity::ThinkSet
            ((CBaseEntity *)this,func,
             *(float *)(unaff_EBX + 0x3c65b3) + *(float *)(**(int **)(unaff_EBX + 0x53f2d3) + 0xc),
             (char *)(unaff_EBX + 0x3a8a1f));
  return;
}


/* CBaseProjector::Activate at 00707900 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Activate(CBaseProjector * this) */

void __thiscall CBaseProjector::Activate(CBaseProjector *this)

{
  CBaseAnimating::Activate(&this->super_CBaseAnimating);
  if ((this->m_bEnabled).m_Value == false) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x00707940. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0xe9])();
  return;
}


/* CBaseProjector::OnRestore at 00707830 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void OnRestore(CBaseProjector * this) */

void __thiscall CBaseProjector::OnRestore(CBaseProjector *this)

{
  CBaseAnimating::OnRestore(&this->super_CBaseAnimating);
  if ((this->m_bEnabled).m_Value == false) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x00707870. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0xe9])();
  return;
}


/* CBaseProjector::Project at 00707950 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Project(CBaseProjector * this) */

void __thiscall CBaseProjector::Project(CBaseProjector *this)

{
  bool bDisablePlacementHelper;
  int *piVar1;
  uint uVar2;
  int iVar3;
  int *piVar4;
  uint *puVar5;
  CBaseProjectedEntity *this_00;
  int unaff_EBX;
  QAngle *local_24;
  bool local_1d;
  
                    /* Unresolved local var: bool bCreateNew@[???] */
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
  piVar1 = *(int **)(unaff_EBX + 0x53f20e);
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
  uVar2 = (this->m_hFirstChild).super_CBaseHandle.m_Index;
  if ((uVar2 == 0xffffffff) ||
     (iVar3 = (uVar2 & 0xffff) * 0x10 + *piVar1, *(uint *)(iVar3 + 8) != uVar2 >> 0x10)) {
    local_1d = true;
  }
  else {
    local_1d = *(int *)(iVar3 + 4) == 0;
    if (!local_1d) goto LAB_007079d7;
  }
  piVar4 = (int *)(*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.
                    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xea])(this);
  if (piVar4 == (int *)0x0) {
    (this->m_hFirstChild).super_CBaseHandle.m_Index = 0xffffffff;
    return;
  }
  puVar5 = (uint *)(**(code **)(*piVar4 + 0xc))(piVar4);
  uVar2 = *puVar5;
  (this->m_hFirstChild).super_CBaseHandle.m_Index = uVar2;
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
  if (uVar2 == 0xffffffff) {
    return;
  }
  iVar3 = (uVar2 & 0xffff) * 0x10 + *piVar1;
  if (*(uint *)(iVar3 + 8) != uVar2 >> 0x10) {
    return;
  }
  if (*(int *)(iVar3 + 4) == 0) {
    return;
  }
LAB_007079d7:
  bDisablePlacementHelper = this->m_bDisablePlacementHelper;
  if ((((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) &&
     (CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this),
     ((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0)) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
  local_24 = &(this->super_CBaseAnimating).super_CBaseEntity.m_angAbsRotation;
  uVar2 = (this->m_hFirstChild).super_CBaseHandle.m_Index;
  if ((uVar2 == 0xffffffff) ||
     (iVar3 = (uVar2 & 0xffff) * 0x10 + *piVar1, *(uint *)(iVar3 + 8) != uVar2 >> 0x10)) {
    this_00 = (CBaseProjectedEntity *)0x0;
  }
  else {
    this_00 = *(CBaseProjectedEntity **)(iVar3 + 4);
  }
  CBaseProjectedEntity::RecursiveProjection
            (this_00,local_1d,this,(CPortal_Base2D *)0x0,
             &(this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin,local_24,0x10,
             bDisablePlacementHelper);
  return;
}


/* CBaseProjector::Cleanup at 007077b0 */

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
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(&DAT_0053f3ab + unaff_EBX),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (oldObj = *(CBaseEntity **)(iVar2 + 4), oldObj != (CBaseEntity *)0x0)) {
    UTIL_Remove(oldObj);
    (this->m_hFirstChild).super_CBaseHandle.m_Index = 0xffffffff;
    return;
  }
  return;
}


/* CBaseProjector::EnableProjection at 00707ef0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void EnableProjection(CBaseProjector * this, bool bEnable) */

void __thiscall CBaseProjector::EnableProjection(CBaseProjector *this,bool bEnable)

{
  undefined1 *puVar1;
  ushort uVar2;
  CBaseEdict *this_00;
  CBaseEntity *oldObj;
  ushort *puVar3;
  int *piVar4;
  char cVar5;
  int iVar6;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  uint uVar9;
  int unaff_EBX;
  uint local_60;
  
  ___i686_get_pc_thunk_bx();
  cVar5 = (this->m_bEnabled).m_Value;
  if (bEnable != (bool)cVar5) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
      ;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(this_00);
        puVar3 = (ushort *)**(undefined4 **)(unaff_EBX + 0x53ec64);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar3) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar3[uVar8 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_007080e8:
            puVar3[(uint)uVar2 + uVar8 * 0x14 + 1] = 0x514;
            puVar3[uVar8 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar3[uVar8 * 0x14 + 1] != 0x514) {
            local_60 = 0;
            do {
              uVar9 = local_60 + 1;
              local_60 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar2) {
                if (uVar2 == 0x13) goto LAB_0070805d;
                goto LAB_007080e8;
              }
            } while (puVar3[uVar8 * 0x14 + local_60 + 1] != 0x514);
          }
        }
        else if (puVar3[0x7d1] == 100) {
LAB_0070805d:
          pIVar7->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar3[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x53ec64) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x53ec64) + 0xfa2) + 1;
          piVar4 = *(int **)(unaff_EBX + 0x53ec64);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar4;
          iVar6 = *piVar4 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar6 + 2) = 0x514;
          *(undefined2 *)(iVar6 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bEnabled).m_Value = bEnable;
    cVar5 = bEnable;
  }
  if (cVar5 != '\0') {
                    /* WARNING: Could not recover jumptable at 0x00707fd7. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
      super_IHandleEntity._vptr_IHandleEntity[0xe9])();
    return;
  }
  uVar8 = (this->m_hFirstChild).super_CBaseHandle.m_Index;
  if (((uVar8 != 0xffffffff) &&
      (iVar6 = (uVar8 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x53ec68),
      *(uint *)(iVar6 + 8) == uVar8 >> 0x10)) &&
     (oldObj = *(CBaseEntity **)(iVar6 + 4), oldObj != (CBaseEntity *)0x0)) {
    UTIL_Remove(oldObj);
    (this->m_hFirstChild).super_CBaseHandle.m_Index = 0xffffffff;
  }
  return;
}


/* CBaseProjector::DrawDebugTextOverlays at 007077a0 */

/* DWARF original prototype: int DrawDebugTextOverlays(CBaseProjector * this) */

int __thiscall CBaseProjector::DrawDebugTextOverlays(CBaseProjector *this)

{
  int iVar1;
  
                    /* Unresolved local var: int text_offset@[???] */
  iVar1 = CBaseAnimating::DrawDebugTextOverlays(&this->super_CBaseAnimating);
  return iVar1;
}


/* CBaseProjector::TestAllForProjectionChanges at 00707710 */

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
  if (0 < *(int *)(unaff_EBX + 0x743e86)) {
    iVar5 = 0;
    piVar1 = *(int **)(&DAT_0053f44e + unaff_EBX);
    do {
      while( true ) {
                    /* Unresolved local var: CBaseProjectedEntity * pFirstChild@[???]
                       Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
        uVar2 = *(uint *)(*(int *)(*(int *)(unaff_EBX + 0x743e7a) + iVar5 * 4) + 0x518);
        if (((uVar2 == 0xffffffff) ||
            (iVar4 = (uVar2 & 0xffff) * 0x10 + *piVar1, *(uint *)(iVar4 + 8) != uVar2 >> 0x10)) ||
           (piVar3 = *(int **)(iVar4 + 4), piVar3 == (int *)0x0)) break;
        (**(code **)(*piVar3 + 0x340))(piVar3);
        iVar5 = iVar5 + 1;
        if (*(int *)(unaff_EBX + 0x743e86) <= iVar5) {
          return;
        }
      }
      iVar5 = iVar5 + 1;
    } while (iVar5 < *(int *)(unaff_EBX + 0x743e86));
  }
  return;
}


/* CBaseProjector::InputEnable at 007083b0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void InputEnable(CBaseProjector * this, inputdata_t * data) */

void __thiscall CBaseProjector::InputEnable(CBaseProjector *this,inputdata_t *data)

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
  if ((this->m_bEnabled).m_Value == false) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
      ;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x53e7a4);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00708524:
            puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x514;
            puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != 0x514) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_00708524;
                pIVar6->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar7 * 0x14 + local_60 + 1] != 0x514);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x53e7a4) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x53e7a4) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x53e7a4);
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar2 = *piVar5 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x514;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bEnabled).m_Value = true;
  }
                    /* WARNING: Could not recover jumptable at 0x0070840e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0xe9])();
  return;
}


/* CBaseProjector::InputDisable at 00708110 */

/* DWARF original prototype: void InputDisable(CBaseProjector * this, inputdata_t * data) */

void __thiscall CBaseProjector::InputDisable(CBaseProjector *this,inputdata_t *data)

{
  EnableProjection(this,false);
  return;
}


/* _GLOBAL__I_sv_projected_entities_use_placement_helper at 000a8480 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_sv_projected_entities_use_placement_helper(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

