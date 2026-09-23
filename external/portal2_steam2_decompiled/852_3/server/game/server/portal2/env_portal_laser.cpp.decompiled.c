/* DWARF-guided pseudocode for game/server/portal2/env_portal_laser.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000bdd50 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  undefined4 uVar1;
  IEntityFactoryDictionary *pIVar2;
  datamap_t *pdVar3;
  int unaff_EBX;
  longlong lVar4;
  ConVar *in_stack_ffffffe4;
  char *in_stack_ffffffe8;
  char *in_stack_ffffffec;
  int flags_1;
  
  flags_1 = unaff_EBX;
  lVar4 = ___i686_get_pc_thunk_bx();
  if (lVar4 == 0xffff00000001) {
    *(undefined1 *)((int)&paint_color_max_diff.super_ConCommandBase.m_pNext + unaff_EBX) = 0;
    *(undefined1 *)((int)&paint_color_max_diff.super_ConCommandBase.m_pNext + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&paint_color_max_diff.super_ConCommandBase.m_pNext + unaff_EBX + 2) = 0;
    *(undefined1 *)((int)&paint_color_max_diff.super_ConCommandBase.m_pNext + unaff_EBX + 3) = 0;
    *(undefined4 *)(&paint_color_max_diff.super_ConCommandBase.m_bRegistered + unaff_EBX) = 0;
    *(undefined4 *)((int)&paint_color_max_diff.super_ConCommandBase.m_pszName + unaff_EBX) = 0;
    *(undefined4 *)((int)&paint_color_max_diff.super_ConCommandBase.m_pszHelpString + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&paint_color_max_diff.super_ConCommandBase.m_nFlags + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&paint_color_max_diff.super_IConVar._vptr_IConVar + unaff_EBX) = 0;
    *(undefined4 *)((int)&paint_color_max_diff.m_pParent + unaff_EBX) = 0;
    *(undefined4 *)((int)&paint_color_max_diff.m_pszDefaultValue + unaff_EBX) = 0;
    *(undefined4 *)((int)&paint_color_max_diff.m_Value.m_pszString + unaff_EBX) = 0;
    *(undefined4 *)((int)&paint_color_max_diff.m_Value.m_StringLength + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&paint_color_max_diff.m_Value.m_fValue + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&paint_color_max_diff.m_Value.m_nValue + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&paint_color_max_diff.m_bHasMin + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&paint_color_max_diff.m_fMinVal + unaff_EBX) = 0;
    (&paint_color_max_diff.field_0x3f)[unaff_EBX] = 0;
    (&paint_color_max_diff.m_bHasMax)[unaff_EBX] = false;
    (&paint_color_max_diff.field_0x3d)[unaff_EBX] = 0;
    (&paint_color_max_diff.field_0x3e)[unaff_EBX] = 0;
    *(undefined4 *)((int)&paint_color_max_diff.m_fMaxVal + unaff_EBX) = 0;
    *(undefined1 *)
     ((int)&paint_color_max_diff.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 3) = 0;
    *(undefined1 *)((int)&paint_color_max_diff.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) =
         0;
    *(undefined1 *)
     ((int)&paint_color_max_diff.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&paint_color_max_diff.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 2) = 0;
    *(undefined4 *)
     ((int)&paint_color_max_diff.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) = 0;
    *(undefined1 *)
     ((int)&paint_color_max_diff.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 3) = 1;
    *(undefined1 *)((int)&paint_color_max_diff.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX)
         = 0;
    *(undefined1 *)
     ((int)&paint_color_max_diff.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&paint_color_max_diff.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&paint_color_max_diff.m_fnChangeCallbacks.m_Size + unaff_EBX) = 1;
    *(undefined1 *)((int)&paint_color_max_diff.m_fnChangeCallbacks.m_pElements + unaff_EBX + 3) = 0;
    *(undefined1 *)((int)&paint_color_max_diff.m_fnChangeCallbacks.m_pElements + unaff_EBX) = 0;
    *(undefined1 *)((int)&paint_color_max_diff.m_fnChangeCallbacks.m_pElements + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&paint_color_max_diff.m_fnChangeCallbacks.m_pElements + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&PaintPowerInfo_t_DataDescInit::g_DataMapHolder + unaff_EBX) = 2;
    (&DAT_00cf895f)[unaff_EBX] = 0;
    (&DAT_00cf895c)[unaff_EBX] = 0;
    (&DAT_00cf895d)[unaff_EBX] = 0;
    (&DAT_00cf895e)[unaff_EBX] = 0;
    *(undefined **)
     ((int)&sv_enable_paint_power_user_debug.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX)
         = &UNK_00af35c4 + unaff_EBX;
    ConVar::ConVar((ConVar *)((int)g_pszMPConcepts + unaff_EBX + 0x9c),
                   (char *)(unaff_EBX + 0x9509f6),(char *)(unaff_EBX + 0x8f5cd8),0x4000,
                   in_stack_ffffffe4,in_stack_ffffffe8,in_stack_ffffffec,flags_1);
    uVar1 = *(undefined4 *)(&DAT_00aede2c + unaff_EBX);
    ___cxa_atexit(unaff_EBX + 0x8db1ec,0,uVar1);
    ConVar::ConVar((ConVar *)((int)s_aGestureSlotNames + unaff_EBX),(char *)(unaff_EBX + 0x950a13),
                   (char *)(unaff_EBX + 0x8f5cd8),0x4002,in_stack_ffffffe4,in_stack_ffffffe8,
                   in_stack_ffffffec,flags_1);
    ___cxa_atexit(unaff_EBX + 0x8db1cc,0,uVar1);
    ConVar::ConVar((ConVar *)(unaff_EBX + 0xc709dc),(char *)(unaff_EBX + 0x950a26),
                   (char *)(unaff_EBX + 0x900a34),0x4002,in_stack_ffffffe4,in_stack_ffffffe8,
                   in_stack_ffffffec,flags_1);
    ___cxa_atexit(unaff_EBX + 0x8db1ac,0,uVar1);
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<PhysObjectHeader_t>::dataDesc[0].flatOffset + unaff_EBX + -0x18
                   ),(char *)(unaff_EBX + 0x950a34),(char *)(unaff_EBX + 0x8eded4),0x4002,
                   in_stack_ffffffe4,in_stack_ffffffe8,in_stack_ffffffec,flags_1);
    ___cxa_atexit(unaff_EBX + 0x8db18c,0,uVar1);
    CBaseEntityClassList::CBaseEntityClassList
              ((CBaseEntityClassList *)
               ((int)DataMapInit<PhysObjectHeader_t>::dataDesc[1].flatOffset + unaff_EBX));
    *(undefined **)((int)DataMapInit<PhysObjectHeader_t>::dataDesc[1].flatOffset + unaff_EBX) =
         &UNK_00b71cb4 + unaff_EBX;
    ___cxa_atexit(unaff_EBX + 0x8db15c,0,uVar1);
    *(undefined **)((int)&sv_enable_paint_power_user_debug.super_ConCommandBase.m_pNext + unaff_EBX)
         = &UNK_00b71cd4 + unaff_EBX;
    pIVar2 = EntityFactoryDictionary();
    (**pIVar2->_vptr_IEntityFactoryDictionary)(pIVar2,unaff_EBX + 0xcf8964,unaff_EBX + 0x950a43);
    pdVar3 = DataMapInit<CPortalLaser>((CPortalLaser *)0x0);
    *(datamap_t **)((int)DataMapInit<PhysObjectHeader_t>::dataDesc[1].flatOffset + unaff_EBX + 8) =
         pdVar3;
  }
  return;
}


/* __tcf_1 at 00998f50 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x395728),in_stack_00000008);
  return;
}


/* __tcf_2 at 00998f30 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3957a8),in_stack_00000008);
  return;
}


/* __tcf_3 at 00998f10 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x395828),in_stack_00000008);
  return;
}


/* __tcf_4 at 00998ef0 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3958a8),in_stack_00000008);
  return;
}


/* __tcf_5 at 00998ec0 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  *(int *)(extraout_ECX + 0x395930) = extraout_ECX + 0x296b50;
  CBaseEntityClassList::~CBaseEntityClassList
            ((CBaseEntityClassList *)(extraout_ECX + 0x395930),in_stack_00000008);
  return;
}


/* GetEnvPortalLaserList at 006afe10 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */

CPortalLaser * GetEnvPortalLaserList(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(CPortalLaser **)(extraout_ECX + 0x63d828);
}


/* FindLasersInSphere at 006b05d0 */

/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

bool FindLasersInSphere(Vector *vecOrigin,float flRadius,
                       CUtlVector<CBeam*,CUtlMemory<CBeam*,_int>_> *vecBeamsOut)

{
  int iVar1;
  bool bVar2;
  int unaff_EBX;
  CPortalLaser *this;
  int iStartCount;
  
  ___i686_get_pc_thunk_bx();
  iVar1 = vecBeamsOut->m_Size;
                    /* Unresolved local var: CPortalLaser * pLaser@[???] */
  this = *(CPortalLaser **)(unaff_EBX + 0x63d062);
  bVar2 = false;
  if (this != (CPortalLaser *)0x0) {
    do {
      CPortalLaser::FindChildBeamsInSphere(this,vecOrigin,flRadius,vecBeamsOut);
      this = this->m_pNext;
    } while (this != (CPortalLaser *)0x0);
    bVar2 = iVar1 < vecBeamsOut->m_Size;
  }
  return bVar2;
}


/* DataMapInit<CPortalLaser> at 000bda60 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */

datamap_t * DataMapInit<CPortalLaser>(CPortalLaser *param_1)

{
  int iVar1;
  int iVar2;
  char *pcVar3;
  size_t sVar4;
  char *pcVar5;
  int iVar6;
  undefined4 uVar7;
  int iVar8;
  undefined4 *puVar9;
  int unaff_EBX;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  if (((&DAT_00cf8c5c)[unaff_EBX] == '\0') &&
     (iVar2 = ___cxa_guard_acquire(&DAT_00cf8c5c + unaff_EBX), iVar2 != 0)) {
    *(int *)((int)&paint_max_impact_particles.super_ConCommandBase.m_pszName + unaff_EBX) =
         unaff_EBX + 0x950cd7;
    *(undefined4 *)((int)&paint_max_impact_particles.super_ConCommandBase.m_nFlags + unaff_EBX) = 0;
    *(undefined4 *)((int)&paint_max_impact_particles.super_IConVar._vptr_IConVar + unaff_EBX) = 0;
    *(undefined4 *)((int)&paint_max_impact_particles.m_pParent + unaff_EBX) = 0;
    *(undefined4 *)((int)&paint_max_impact_particles.m_pszDefaultValue + unaff_EBX) = 0;
    *(undefined4 *)((int)&paint_max_impact_particles.m_Value.m_pszString + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&paint_max_impact_particles.super_ConCommandBase.m_pszHelpString + unaff_EBX) = 0xc;
    ___cxa_guard_release(&DAT_00cf8c5c + unaff_EBX);
    ___cxa_atexit(unaff_EBX + 0x8db4fc,0,*(undefined4 *)(&DAT_00aee11c + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_NetworkStateChanged_m_nWaterLevel_00c0b51c + unaff_EBX) =
       *(undefined4 *)(&DAT_00aee128 + unaff_EBX);
  if ((*(char *)((int)&paint_max_impact_particles.super_ConCommandBase.m_pNext + unaff_EBX) != '\0')
     || (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcf8c64), iVar2 == 0)) goto LAB_000bdab4;
  pcVar3 = operator_new___(*(int *)((int)&paint_max_impact_particles.super_ConCommandBase.
                                          m_pszHelpString + unaff_EBX) + 0xc);
  _strcpy(pcVar3,*(char **)((int)&paint_max_impact_particles.super_ConCommandBase.m_pszName +
                           unaff_EBX));
  sVar4 = _strlen(pcVar3);
  pcVar5 = pcVar3 + sVar4;
  pcVar5[0] = 'S';
  pcVar5[1] = 't';
  pcVar5[2] = 'r';
  pcVar5[3] = 'i';
  pcVar5[4] = 'k';
  pcVar5[5] = 'e';
  pcVar5[6] = 'T';
  pcVar5[7] = 'h';
  *(undefined **)(pcVar5 + 8) = &DAT_006b6e69;
  iVar1 = *(int *)((int)&paint_max_impact_particles.m_pszDefaultValue + unaff_EBX);
  iVar2 = iVar1 + 1;
  iVar6 = *(int *)((int)&paint_max_impact_particles.super_IConVar._vptr_IConVar + unaff_EBX);
  if (iVar6 < iVar2) {
    iVar8 = *(int *)((int)&paint_max_impact_particles.m_pParent + unaff_EBX);
    if (-1 < iVar8) {
      if (iVar8 == 0) {
        if ((iVar6 == 0) && (iVar6 = 8, iVar2 < 9)) {
          local_20 = 0x20;
        }
        else {
          do {
            local_20 = iVar6;
            iVar6 = local_20 * 2;
          } while (iVar6 < iVar2);
          local_20 = local_20 * 8;
        }
        goto LAB_000bdbd0;
      }
      iVar6 = (iVar1 / iVar8 + 1) * iVar8;
      if (iVar6 < iVar2) {
        if (iVar6 != 0) {
LAB_000bdb90:
          do {
            iVar6 = (iVar6 + iVar2) / 2;
          } while (iVar6 < iVar2);
          goto LAB_000bdba0;
        }
        if (iVar2 < 0) {
          iVar6 = -1;
          local_20 = -4;
        }
        else {
          local_20 = 0;
          if (iVar2 != 0) goto LAB_000bdb90;
        }
      }
      else {
LAB_000bdba0:
        local_20 = iVar6 * 4;
      }
LAB_000bdbd0:
      *(int *)((int)&paint_max_impact_particles.super_IConVar._vptr_IConVar + unaff_EBX) = iVar6;
      iVar6 = *(int *)((int)&paint_max_impact_particles.super_ConCommandBase.m_nFlags + unaff_EBX);
      if (iVar6 == 0) {
        uVar7 = (*(code *)**(undefined4 **)**(undefined4 **)(&DAT_00aee124 + unaff_EBX))
                          ((undefined4 *)**(undefined4 **)(&DAT_00aee124 + unaff_EBX),local_20);
      }
      else {
        uVar7 = (**(code **)(*(int *)**(undefined4 **)(&DAT_00aee124 + unaff_EBX) + 4))
                          ((int *)**(undefined4 **)(&DAT_00aee124 + unaff_EBX),iVar6,local_20);
      }
      *(undefined4 *)((int)&paint_max_impact_particles.super_ConCommandBase.m_nFlags + unaff_EBX) =
           uVar7;
    }
  }
  iVar8 = *(int *)((int)&paint_max_impact_particles.m_pszDefaultValue + unaff_EBX) + 1;
  *(int *)((int)&paint_max_impact_particles.m_pszDefaultValue + unaff_EBX) = iVar8;
  iVar6 = *(int *)((int)&paint_max_impact_particles.super_ConCommandBase.m_nFlags + unaff_EBX);
  *(int *)((int)&paint_max_impact_particles.m_Value.m_pszString + unaff_EBX) = iVar6;
  iVar8 = (iVar8 - iVar1) + -1;
  if (0 < iVar8) {
    _V_memmove((void *)(iVar6 + iVar2 * 4),(void *)(iVar6 + iVar1 * 4),iVar8 * 4);
    iVar6 = *(int *)((int)&paint_max_impact_particles.super_ConCommandBase.m_nFlags + unaff_EBX);
  }
  puVar9 = (undefined4 *)(iVar6 + iVar1 * 4);
  if (puVar9 != (undefined4 *)0x0) {
    *puVar9 = pcVar3;
  }
  *(char **)((int)&PTR_SetRefEHandle_00c0b570 + unaff_EBX) = pcVar3;
  ___cxa_guard_release(unaff_EBX + 0xcf8c64);
LAB_000bdab4:
  *(undefined4 *)((int)&PTR_NetworkStateChanged_m_fFlags_00c0b514 + unaff_EBX) = 0xc;
  *(int *)((int)&PTR_ShouldDrawWaterImpacts_00c0b510 + unaff_EBX) =
       (int)&PTR__CRuleBrushEntity_00c0b56c + unaff_EBX;
  return (datamap_t *)((int)&PTR_ShouldDrawWaterImpacts_00c0b510 + unaff_EBX);
}


/* CPortalLaser::GetDataDescMap at 006afe20 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: datamap_t * GetDataDescMap(CPortalLaser * this) */

datamap_t * __thiscall CPortalLaser::GetDataDescMap(CPortalLaser *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x61915c);
}


/* CPortalLaser::GetBaseMap at 006afe30 */

datamap_t * CPortalLaser::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4fbd64);
}


/* __tcf_0 at 00998f70 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41d776)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41d76a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41d776));
  }
  *(undefined4 *)(unaff_EBX + 0x41d776) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41d772)) {
    if (*(int *)(unaff_EBX + 0x41d76a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x212c1a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x212c1a),*(int *)(unaff_EBX + 0x41d76a));
      *(undefined4 *)(unaff_EBX + 0x41d76a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d76e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41d76a);
  *(int *)(unaff_EBX + 0x41d77a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41d772)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x212c1a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x212c1a),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41d76a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d76e) = 0;
  }
  return;
}


/* CPortalLaser::CPortalLaser at 006b0150 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void CPortalLaser(CPortalLaser * this) */

void __thiscall CPortalLaser::CPortalLaser(CPortalLaser *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::CBaseAnimating(&this->super_CBaseAnimating);
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x57f4e7);
  this->m_pNext = *(CPortalLaser **)(unaff_EBX + 0x63d4df);
  *(CPortalLaser **)(unaff_EBX + 0x63d4df) = this;
  return;
}


/* CPortalLaser::CPortalLaser at 006b01a0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void CPortalLaser(CPortalLaser * this) */

void __thiscall CPortalLaser::CPortalLaser(CPortalLaser *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::CBaseAnimating(&this->super_CBaseAnimating);
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x57f497);
  this->m_pNext = *(CPortalLaser **)(unaff_EBX + 0x63d48f);
  *(CPortalLaser **)(unaff_EBX + 0x63d48f) = this;
  return;
}


/* CEntityFactory<CPortalLaser>::Create at 006b6020 */

/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CPortalLaser> * this, char *
   pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CPortalLaser>::Create(CEntityFactory<CPortalLaser> *this,char *pClassName)

{
  CBaseAnimating *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CPortalLaser * pEnt@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CBaseEntity::operator_new(0x560);
  CBaseAnimating::CBaseAnimating(this_00);
  (this_00->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x579614);
  this_00[1].super_CBaseEntity.super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = *(_func_int_varargs ***)(unaff_EBX + 0x63760c);
  *(CBaseAnimating **)(unaff_EBX + 0x63760c) = this_00;
  (*(this_00->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x1d])(this_00,pClassName);
  return &(this_00->super_CBaseEntity).m_Network.super_IServerNetworkable;
}


/* CPortalLaser::~CPortalLaser at 006b00f0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void ~CPortalLaser(CPortalLaser * this, int __in_chrg) */

void __thiscall CPortalLaser::~CPortalLaser(CPortalLaser *this,int __in_chrg)

{
  CPortalLaser **ppCVar1;
  CPortalLaser *pCVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x57f54f);
  pCVar2 = *(CPortalLaser **)(unaff_EBX + 0x63d547);
  if (pCVar2 != (CPortalLaser *)0x0) {
    ppCVar1 = (CPortalLaser **)(unaff_EBX + 0x63d547);
    do {
      if (this == pCVar2) {
        *ppCVar1 = this->m_pNext;
        CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
        return;
      }
      ppCVar1 = &pCVar2->m_pNext;
      pCVar2 = pCVar2->m_pNext;
    } while (pCVar2 != (CPortalLaser *)0x0);
  }
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
  return;
}


/* CPortalLaser::~CPortalLaser at 006b0270 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void ~CPortalLaser(CPortalLaser * this, int __in_chrg) */

void __thiscall CPortalLaser::~CPortalLaser(CPortalLaser *this,int __in_chrg)

{
  CPortalLaser **ppCVar1;
  CPortalLaser *pCVar2;
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x57f3cb);
  pCVar2 = *(CPortalLaser **)(unaff_EBX + 0x63d3c3);
  if (pCVar2 != (CPortalLaser *)0x0) {
    ppCVar1 = (CPortalLaser **)(unaff_EBX + 0x63d3c3);
    do {
      if (this == pCVar2) {
        *ppCVar1 = this->m_pNext;
        break;
      }
      ppCVar1 = &pCVar2->m_pNext;
      pCVar2 = pCVar2->m_pNext;
    } while (pCVar2 != (CPortalLaser *)0x0);
  }
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CPortalLaser::~CPortalLaser at 006b02f0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void ~CPortalLaser(CPortalLaser * this, int __in_chrg) */

void __thiscall CPortalLaser::~CPortalLaser(CPortalLaser *this,int __in_chrg)

{
  CPortalLaser **ppCVar1;
  CPortalLaser *pCVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x57f34f);
  pCVar2 = *(CPortalLaser **)(unaff_EBX + 0x63d347);
  if (pCVar2 != (CPortalLaser *)0x0) {
    ppCVar1 = (CPortalLaser **)(unaff_EBX + 0x63d347);
    do {
      if (this == pCVar2) {
        *ppCVar1 = this->m_pNext;
        CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
        return;
      }
      ppCVar1 = &pCVar2->m_pNext;
      pCVar2 = pCVar2->m_pNext;
    } while (pCVar2 != (CPortalLaser *)0x0);
  }
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
  return;
}


/* CPortalLaser::Spawn at 006b1810 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(CPortalLaser * this) */

void __thiscall CPortalLaser::Spawn(CPortalLaser *this)

{
  BASEPTR func;
  int iVar1;
  char *pcVar2;
  char *pcVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::Spawn(&this->super_CBaseAnimating);
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1a])(this);
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[8])(this);
  pcVar3 = (this->m_ModelName).pszValue;
  pcVar2 = (char *)(unaff_EBX + 0x2fa33f);
  if (pcVar3 != (char *)0x0) {
    pcVar2 = pcVar3;
  }
  if (*pcVar2 == '\0') {
    (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
      super_IHandleEntity._vptr_IHandleEntity[0x1b])(this,unaff_EBX + 0x35cf0f);
  }
  else {
    if (pcVar3 == (char *)0x0) {
      pcVar3 = (char *)(unaff_EBX + 0x2fa33f);
    }
    (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
      super_IHandleEntity._vptr_IHandleEntity[0x1b])(this,pcVar3);
  }
  CCollisionProperty::SetSolid
            (&(this->super_CBaseAnimating).super_CBaseEntity.m_Collision.super_CCollisionProperty,
             SOLID_VPHYSICS);
  iVar1 = CBaseAnimating::LookupAttachment
                    (&this->super_CBaseAnimating,(char *)(unaff_EBX + 0x35cff7));
  this->m_iLaserAttachment = iVar1;
  if (iVar1 == 0) {
    pcVar3 = (this->m_ModelName).pszValue;
    if (pcVar3 == (char *)0x0) {
      pcVar3 = (char *)(unaff_EBX + 0x2fa33f);
    }
    pcVar2 = (this->super_CBaseAnimating).super_CBaseEntity.m_iName.m_Value.pszValue;
    if (pcVar2 == (char *)0x0) {
      pcVar2 = (char *)(unaff_EBX + 0x2fa33f);
    }
    _Warning(unaff_EBX + 0x35d00b,pcVar2,pcVar3);
  }
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x9f])(this);
                    /* Unresolved local var: int i@[???] */
  this->m_pChildBeams[0] = (CBeam *)0x0;
  this->m_pChildBeams[1] = (CBeam *)0x0;
  this->m_pChildBeams[2] = (CBeam *)0x0;
  this->m_pChildBeams[3] = (CBeam *)0x0;
  this->m_pChildBeams[4] = (CBeam *)0x0;
  this->m_pChildBeams[5] = (CBeam *)0x0;
  this->m_pChildBeams[6] = (CBeam *)0x0;
  this->m_pChildBeams[7] = (CBeam *)0x0;
  if ((this->m_bStartOff == false) &&
     ((this->super_CBaseAnimating).super_CBaseEntity.m_pfnThink.__pfn ==
      (_func___thiscall_void_CBaseEntity_ptr *)0x0)) {
    func.__delta = 0;
    func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0x4443);
    CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
    CBaseEntity::SetNextThink
              ((CBaseEntity *)this,
               *(float *)(**(int **)(unaff_EBX + 0x4fa383) + 0xc) +
               *(float *)(*(int *)(unaff_EBX + 0x67cf3f) + 0x2c),(char *)0x0);
    TurnOnGlow(this);
    return;
  }
  return;
}


/* CPortalLaser::CreateHelperEntities at 006b10b0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void CreateHelperEntities(CPortalLaser * this) */

void __thiscall CPortalLaser::CreateHelperEntities(CPortalLaser *this)

{
  undefined1 *puVar1;
  int *piVar2;
  CBaseEdict *pCVar3;
  edict_t *peVar4;
  char *pSpriteName;
  CBeam *pCVar5;
  IChangeInfoAccessor *pIVar6;
  CSoundEnvelopeController *pCVar7;
  CSoundPatch *__in_chrg;
  CInfoPlacementHelper *pCVar8;
  CBaseEntity *pCVar9;
  int unaff_EBX;
  int iVar10;
  CRecipientFilter local_54;
  Vector local_34;
  Vector local_28 [2];
  
  ___i686_get_pc_thunk_bx();
  iVar10 = 0;
  do {
    if (this->m_pChildBeams[iVar10] == (CBeam *)0x0) {
      pSpriteName = (char *)(unaff_EBX + 0x341d6b);
      if (this->m_bIsLethal == false) {
        pSpriteName = &UNK_00333c2f + unaff_EBX;
      }
      pCVar5 = CBeam::BeamCreate(pSpriteName,2.0);
      this->m_pChildBeams[iVar10] = pCVar5;
      DispatchSpawn(&pCVar5->super_CBaseEntity,true);
      if (this->m_bIsLethal == false) {
        pCVar5 = this->m_pChildBeams[iVar10];
        if (*(float *)(unaff_EBX + 0x37955f) != (pCVar5->m_fWidth).m_Value) {
          if ((pCVar5->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
            pCVar3 = &((pCVar5->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
            if (pCVar3 != (CBaseEdict *)0x0) {
              pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
              pIVar6 = CBaseEdict::GetChangeAccessor(pCVar3);
              pIVar6->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            puVar1 = &(pCVar5->super_CBaseEntity).m_Network.field_0x4c;
            *(uint *)puVar1 = *(uint *)puVar1 | 1;
          }
          (pCVar5->m_fWidth).m_Value = 32.0;
          pCVar5 = this->m_pChildBeams[iVar10];
        }
        if (*(float *)(unaff_EBX + 0x37955f) != (pCVar5->m_fEndWidth).m_Value) {
          if ((pCVar5->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
            pCVar3 = &((pCVar5->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
            if (pCVar3 != (CBaseEdict *)0x0) {
              pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
              pIVar6 = CBaseEdict::GetChangeAccessor(pCVar3);
              pIVar6->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            puVar1 = &(pCVar5->super_CBaseEntity).m_Network.field_0x4c;
            *(uint *)puVar1 = *(uint *)puVar1 | 1;
          }
          (pCVar5->m_fEndWidth).m_Value = 32.0;
        }
      }
      else {
        pCVar5 = this->m_pChildBeams[iVar10];
        if ((pCVar5->super_CBaseEntity).m_clrRender.
            super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r != 'd') {
          if ((pCVar5->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
            pCVar3 = &((pCVar5->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
            if (pCVar3 != (CBaseEdict *)0x0) {
              pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
              pIVar6 = CBaseEdict::GetChangeAccessor(pCVar3);
              pIVar6->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            puVar1 = &(pCVar5->super_CBaseEntity).m_Network.field_0x4c;
            *(uint *)puVar1 = *(uint *)puVar1 | 1;
          }
          (pCVar5->super_CBaseEntity).m_clrRender.
          super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r = 'd';
        }
        if ((pCVar5->super_CBaseEntity).m_clrRender.
            super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g != 0xff)
        {
          if ((pCVar5->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
            pCVar3 = &((pCVar5->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
            if (pCVar3 != (CBaseEdict *)0x0) {
              pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
              pIVar6 = CBaseEdict::GetChangeAccessor(pCVar3);
              pIVar6->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            puVar1 = &(pCVar5->super_CBaseEntity).m_Network.field_0x4c;
            *(uint *)puVar1 = *(uint *)puVar1 | 1;
          }
          (pCVar5->super_CBaseEntity).m_clrRender.
          super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g = 0xff;
        }
        if ((pCVar5->super_CBaseEntity).m_clrRender.
            super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b != 'd') {
          if ((pCVar5->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
            pCVar3 = &((pCVar5->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
            if (pCVar3 != (CBaseEdict *)0x0) {
              pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
              pIVar6 = CBaseEdict::GetChangeAccessor(pCVar3);
              pIVar6->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            puVar1 = &(pCVar5->super_CBaseEntity).m_Network.field_0x4c;
            *(uint *)puVar1 = *(uint *)puVar1 | 1;
          }
          (pCVar5->super_CBaseEntity).m_clrRender.
          super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b = 'd';
        }
        pCVar5 = this->m_pChildBeams[iVar10];
        if (*(float *)(unaff_EBX + 0x37980f) != (pCVar5->m_fWidth).m_Value) {
          if ((pCVar5->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
            pCVar3 = &((pCVar5->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
            if (pCVar3 != (CBaseEdict *)0x0) {
              pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
              pIVar6 = CBaseEdict::GetChangeAccessor(pCVar3);
              pIVar6->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            puVar1 = &(pCVar5->super_CBaseEntity).m_Network.field_0x4c;
            *(uint *)puVar1 = *(uint *)puVar1 | 1;
          }
          (pCVar5->m_fWidth).m_Value = 2.0;
          pCVar5 = this->m_pChildBeams[iVar10];
        }
        if (*(float *)(unaff_EBX + 0x37980f) != (pCVar5->m_fEndWidth).m_Value) {
          if ((pCVar5->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
            pCVar3 = &((pCVar5->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
            if (pCVar3 != (CBaseEdict *)0x0) {
              pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
              pIVar6 = CBaseEdict::GetChangeAccessor(pCVar3);
              pIVar6->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            puVar1 = &(pCVar5->super_CBaseEntity).m_Network.field_0x4c;
            *(uint *)puVar1 = *(uint *)puVar1 | 1;
          }
          (pCVar5->m_fEndWidth).m_Value = 2.0;
        }
      }
    }
    iVar10 = iVar10 + 1;
  } while (iVar10 != 8);
  if (this->m_pPlacementHelper == (CInfoPlacementHelper *)0x0) {
    pCVar8 = (CInfoPlacementHelper *)CreateEntityByName((char *)(unaff_EBX + 0x35d725),-1,true);
    this->m_pPlacementHelper = pCVar8;
    if (((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
    CBaseEntity::SetAbsOrigin
              ((CBaseEntity *)this->m_pPlacementHelper,
               &(this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin);
    if (((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
    CBaseEntity::SetAbsAngles
              ((CBaseEntity *)this->m_pPlacementHelper,
               &(this->super_CBaseAnimating).super_CBaseEntity.m_angAbsRotation);
    (*(this->m_pPlacementHelper->super_CPointEntity).super_CBaseEntity.super_IServerEntity.
      super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x20])
              (this->m_pPlacementHelper,unaff_EBX + 0x325cd3,unaff_EBX + 0x35d73b);
    (*(this->m_pPlacementHelper->super_CPointEntity).super_CBaseEntity.super_IServerEntity.
      super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x20])
              (this->m_pPlacementHelper,unaff_EBX + 0x35d741,unaff_EBX + 0x2fab77);
    DispatchSpawn((CBaseEntity *)this->m_pPlacementHelper,true);
  }
  if (this->m_pSoundProxy == (CBaseEntity *)0x0) {
    pCVar9 = CreateEntityByName(&UNK_003350fb + unaff_EBX,-1,true);
    this->m_pSoundProxy = pCVar9;
    if (((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
    CBaseEntity::SetAbsOrigin
              (this->m_pSoundProxy,&(this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin);
    pCVar9 = this->m_pSoundProxy;
    piVar2 = &pCVar9->m_iEFlags;
    *piVar2 = *piVar2 | 0x80;
    CBaseEntity::DispatchUpdateTransmitState(pCVar9);
  }
  if (this->m_pAmbientSound != (CSoundPatch *)0x0) {
    return;
  }
  pCVar7 = CSoundEnvelopeController::GetController();
  pCVar9 = this->m_pSoundProxy;
  (*(pCVar9->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x9e]
  )(local_28,pCVar9);
  CRecipientFilter::CRecipientFilter(&local_54);
  local_54.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(&UNK_004fd0c7 + unaff_EBX);
  CRecipientFilter::AddRecipientsByPAS(&local_54,local_28);
  local_54.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(unaff_EBX + 0x4fd087);
  (*(pCVar9->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x9e]
  )(&local_34,pCVar9);
  CPASAttenuationFilter::Filter((CPASAttenuationFilter *)&local_54,&local_34,0.8);
  peVar4 = (this->m_pSoundProxy->m_Network).m_pPev;
                    /* Unresolved local var: int edictIndex@[???] */
  iVar10 = 0;
  if (peVar4 != (edict_t *)0x0) {
    iVar10 = (int)peVar4 - *(int *)(**(int **)(unaff_EBX + 0x4faadf) + 0x58) >> 4;
  }
  __in_chrg = (CSoundPatch *)
              (*pCVar7->_vptr_CSoundEnvelopeController[6])
                        (pCVar7,&local_54,iVar10,unaff_EBX + 0x35d81e);
  this->m_pAmbientSound = __in_chrg;
  (*pCVar7->_vptr_CSoundEnvelopeController[2])(pCVar7,__in_chrg,0x3f800000,0x42c80000,0);
  local_54.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(&UNK_004fd0c7 + unaff_EBX);
  CRecipientFilter::~CRecipientFilter(&local_54,(int)__in_chrg);
  return;
}


/* CPortalLaser::Activate at 006b16b0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void Activate(CPortalLaser * this) */

void __thiscall CPortalLaser::Activate(CPortalLaser *this)

{
  CreateHelperEntities(this);
  CBaseAnimating::Activate(&this->super_CBaseAnimating);
  return;
}


/* CPortalLaser::UpdateOnRemove at 006b0810 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void UpdateOnRemove(CPortalLaser * this) */

void __thiscall CPortalLaser::UpdateOnRemove(CPortalLaser *this)

{
  if (this->m_pPlacementHelper != (CInfoPlacementHelper *)0x0) {
    UTIL_Remove((CBaseEntity *)this->m_pPlacementHelper);
  }
                    /* Unresolved local var: int i@[???] */
  if (this->m_pChildBeams[0] != (CBeam *)0x0) {
    UTIL_Remove(&this->m_pChildBeams[0]->super_CBaseEntity);
    this->m_pChildBeams[0] = (CBeam *)0x0;
  }
  if (this->m_pChildBeams[1] != (CBeam *)0x0) {
    UTIL_Remove(&this->m_pChildBeams[1]->super_CBaseEntity);
    this->m_pChildBeams[1] = (CBeam *)0x0;
  }
  if (this->m_pChildBeams[2] != (CBeam *)0x0) {
    UTIL_Remove(&this->m_pChildBeams[2]->super_CBaseEntity);
    this->m_pChildBeams[2] = (CBeam *)0x0;
  }
  if (this->m_pChildBeams[3] != (CBeam *)0x0) {
    UTIL_Remove(&this->m_pChildBeams[3]->super_CBaseEntity);
    this->m_pChildBeams[3] = (CBeam *)0x0;
  }
  if (this->m_pChildBeams[4] != (CBeam *)0x0) {
    UTIL_Remove(&this->m_pChildBeams[4]->super_CBaseEntity);
    this->m_pChildBeams[4] = (CBeam *)0x0;
  }
  if (this->m_pChildBeams[5] != (CBeam *)0x0) {
    UTIL_Remove(&this->m_pChildBeams[5]->super_CBaseEntity);
    this->m_pChildBeams[5] = (CBeam *)0x0;
  }
  if (this->m_pChildBeams[6] != (CBeam *)0x0) {
    UTIL_Remove(&this->m_pChildBeams[6]->super_CBaseEntity);
    this->m_pChildBeams[6] = (CBeam *)0x0;
  }
  if (this->m_pChildBeams[7] != (CBeam *)0x0) {
    UTIL_Remove(&this->m_pChildBeams[7]->super_CBaseEntity);
    this->m_pChildBeams[7] = (CBeam *)0x0;
  }
  TurnOff(this);
  CBaseEntity::UpdateOnRemove((CBaseEntity *)this);
  return;
}


/* CPortalLaser::Precache at 006b01f0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void Precache(CPortalLaser * this) */

void __thiscall CPortalLaser::Precache(CPortalLaser *this)

{
  CPortalLaser *pCVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::PrecacheScriptSound(&UNK_0035e6de + unaff_EBX);
  CBaseEntity::PrecacheScriptSound(&UNK_0035e6cb + unaff_EBX);
  this = (CPortalLaser *)(this->m_ModelName).pszValue;
  pCVar1 = (CPortalLaser *)(unaff_EBX + 0x2fb95b);
  if (this != (CPortalLaser *)0x0) {
    pCVar1 = this;
  }
  if (*(char *)&(pCVar1->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.
                super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity == '\0') {
    this = (CPortalLaser *)(unaff_EBX + 0x35e52b);
  }
  else if (this == (CPortalLaser *)0x0) {
    this = (CPortalLaser *)(unaff_EBX + 0x2fb95b);
  }
  CBaseEntity::PrecacheModel((char *)this);
  return;
}


/* CPortalLaser::CreateVPhysics at 006b00d0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: bool CreateVPhysics(CPortalLaser * this) */

bool __thiscall CPortalLaser::CreateVPhysics(CPortalLaser *this)

{
  CBaseEntity::VPhysicsInitStatic((CBaseEntity *)this);
  return true;
}


/* CPortalLaser::FindChildBeamsInSphere at 006b0350 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: bool FindChildBeamsInSphere(CPortalLaser * this, Vector * vecOrigin,
   float flRadius, CUtlVector<CBeam*,CUtlMemory<CBeam*,_int>_> * vecBeamsOut) */

bool __thiscall
CPortalLaser::FindChildBeamsInSphere
          (CPortalLaser *this,Vector *vecOrigin,float flRadius,
          CUtlVector<CBeam*,CUtlMemory<CBeam*,_int>_> *vecBeamsOut)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  uint uVar4;
  bool bVar5;
  Vector *pVVar6;
  Vector *pVVar7;
  int iVar8;
  CBeam **ppCVar9;
  int iVar10;
  int unaff_EBX;
  int iVar11;
  int iStartCount;
  int local_30;
  Vector local_28 [2];
  
  ___i686_get_pc_thunk_bx();
  iVar2 = vecBeamsOut->m_Size;
  iVar11 = 0;
  puVar3 = *(undefined4 **)(&DAT_004fb83a + unaff_EBX);
  do {
    if (this->m_pChildBeams[iVar11] != (CBeam *)0x0) {
      pVVar6 = CBeam::GetAbsStartPos(this->m_pChildBeams[iVar11]);
      pVVar7 = CBeam::GetAbsEndPos(this->m_pChildBeams[iVar11]);
                    /* Unresolved local var: Vector res@[???] */
      local_28[0].x = pVVar7->x - pVVar6->x;
      local_28[0].y = pVVar7->y - pVVar6->y;
      local_28[0].z = pVVar7->z - pVVar6->z;
      pVVar6 = CBeam::GetAbsStartPos(this->m_pChildBeams[iVar11]);
      bVar5 = IsRayIntersectingSphere(pVVar6,local_28,vecOrigin,flRadius,0.0);
      if (bVar5) {
        uVar4 = vecBeamsOut->m_Size;
        iVar1 = uVar4 + 1;
        iVar10 = (vecBeamsOut->m_Memory).m_nAllocationCount;
        if (iVar10 < iVar1) {
          iVar8 = (vecBeamsOut->m_Memory).m_nGrowSize;
          if (-1 < iVar8) {
            if (iVar8 == 0) {
              if ((iVar10 == 0) && (iVar10 = 8, iVar1 < 9)) {
                local_30 = 0x20;
              }
              else {
                do {
                  local_30 = iVar10;
                  iVar10 = local_30 * 2;
                } while (iVar10 < iVar1);
                local_30 = local_30 * 8;
              }
              goto LAB_006b0571;
            }
            iVar8 = ((int)uVar4 / iVar8 + 1) * iVar8;
            if (iVar8 < iVar1) {
              if (iVar8 != 0) {
LAB_006b0480:
                do {
                  iVar8 = (iVar8 + iVar1) / 2;
                } while (iVar8 < iVar1);
                goto LAB_006b0493;
              }
              if (iVar1 < 0) {
                iVar10 = -1;
                local_30 = -4;
              }
              else {
                local_30 = 0;
                iVar10 = 0;
                if (iVar1 != 0) goto LAB_006b0480;
              }
LAB_006b0571:
              (vecBeamsOut->m_Memory).m_nAllocationCount = iVar10;
              ppCVar9 = (vecBeamsOut->m_Memory).m_pMemory;
            }
            else {
LAB_006b0493:
              local_30 = iVar8 * 4;
              (vecBeamsOut->m_Memory).m_nAllocationCount = iVar8;
              ppCVar9 = (vecBeamsOut->m_Memory).m_pMemory;
            }
            if (ppCVar9 == (CBeam **)0x0) {
              ppCVar9 = (CBeam **)
                        (*(code *)**(undefined4 **)**(undefined4 **)(&DAT_004fb83a + unaff_EBX))
                                  ((undefined4 *)**(undefined4 **)(&DAT_004fb83a + unaff_EBX),
                                   local_30);
              (vecBeamsOut->m_Memory).m_pMemory = ppCVar9;
            }
            else {
              ppCVar9 = (CBeam **)
                        (**(code **)(*(int *)*puVar3 + 4))((int *)*puVar3,ppCVar9,local_30);
              (vecBeamsOut->m_Memory).m_pMemory = ppCVar9;
            }
          }
        }
        vecBeamsOut->m_Size = vecBeamsOut->m_Size + 1;
        ppCVar9 = (vecBeamsOut->m_Memory).m_pMemory;
        vecBeamsOut->m_pElements = ppCVar9;
        iVar10 = ~uVar4 + vecBeamsOut->m_Size;
        if (0 < iVar10) {
          _V_memmove(ppCVar9 + iVar1,ppCVar9 + uVar4,iVar10 * 4);
        }
        ppCVar9 = (vecBeamsOut->m_Memory).m_pMemory + uVar4;
        if (ppCVar9 != (CBeam **)0x0) {
          *ppCVar9 = this->m_pChildBeams[iVar11];
        }
      }
    }
    iVar11 = iVar11 + 1;
    if (iVar11 == 8) {
      return iVar2 < vecBeamsOut->m_Size;
    }
  } while( true );
}


/* CPortalLaser::IsOn at 006afe40 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: bool IsOn(CPortalLaser * this) */

bool __thiscall CPortalLaser::IsOn(CPortalLaser *this)

{
  return (this->super_CBaseAnimating).super_CBaseEntity.m_pfnThink.__pfn !=
         (_func___thiscall_void_CBaseEntity_ptr *)0x0;
}


/* CPortalLaser::InputTurnOn at 006b1770 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void InputTurnOn(CPortalLaser * this, inputdata_t * inputdata) */

void __thiscall CPortalLaser::InputTurnOn(CPortalLaser *this,inputdata_t *inputdata)

{
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((this->super_CBaseAnimating).super_CBaseEntity.m_pfnThink.__pfn !=
      (_func___thiscall_void_CBaseEntity_ptr *)0x0) {
    return;
  }
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0x44df);
  CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(**(int **)(unaff_EBX + 0x4fa41f) + 0xc) +
             *(float *)(*(int *)(unaff_EBX + 0x67cfdb) + 0x2c),(char *)0x0);
  TurnOnGlow(this);
  return;
}


/* CPortalLaser::InputTurnOff at 006b07f0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void InputTurnOff(CPortalLaser * this, inputdata_t * inputdata) */

void __thiscall CPortalLaser::InputTurnOff(CPortalLaser *this,inputdata_t *inputdata)

{
  if ((this->super_CBaseAnimating).super_CBaseEntity.m_pfnThink.__pfn ==
      (_func___thiscall_void_CBaseEntity_ptr *)0x0) {
    return;
  }
  TurnOff(this);
  return;
}


/* CPortalLaser::InputToggle at 006b16d0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void InputToggle(CPortalLaser * this, inputdata_t * inputdata) */

void __thiscall CPortalLaser::InputToggle(CPortalLaser *this,inputdata_t *inputdata)

{
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((this->super_CBaseAnimating).super_CBaseEntity.m_pfnThink.__pfn !=
      (_func___thiscall_void_CBaseEntity_ptr *)0x0) {
    TurnOff(this);
    return;
  }
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0x457f);
  CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(**(int **)(unaff_EBX + 0x4fa4bf) + 0xc) +
             *(float *)(*(int *)(unaff_EBX + 0x67d07b) + 0x2c),(char *)0x0);
  TurnOnGlow(this);
  return;
}


/* CPortalLaser::TurnOff at 006b0640 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void TurnOff(CPortalLaser * this) */

void __thiscall CPortalLaser::TurnOff(CPortalLaser *this)

{
  float time;
  CSprite *this_00;
  int *piVar1;
  BASEPTR func;
  CSoundEnvelopeController *pCVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (this->m_pChildBeams[0] != (CBeam *)0x0) {
    CBeam::TurnOn(this->m_pChildBeams[0]);
  }
  if (this->m_pChildBeams[1] != (CBeam *)0x0) {
    CBeam::TurnOn(this->m_pChildBeams[1]);
  }
  if (this->m_pChildBeams[2] != (CBeam *)0x0) {
    CBeam::TurnOn(this->m_pChildBeams[2]);
  }
  if (this->m_pChildBeams[3] != (CBeam *)0x0) {
    CBeam::TurnOn(this->m_pChildBeams[3]);
  }
  if (this->m_pChildBeams[4] != (CBeam *)0x0) {
    CBeam::TurnOn(this->m_pChildBeams[4]);
  }
  if (this->m_pChildBeams[5] != (CBeam *)0x0) {
    CBeam::TurnOn(this->m_pChildBeams[5]);
  }
  if (this->m_pChildBeams[6] != (CBeam *)0x0) {
    CBeam::TurnOn(this->m_pChildBeams[6]);
  }
  if (this->m_pChildBeams[7] != (CBeam *)0x0) {
    CBeam::TurnOn(this->m_pChildBeams[7]);
  }
  this_00 = this->m_pLaserGlow;
  if (this_00 != (CSprite *)0x0) {
    time = *(float *)(unaff_EBX + 0x379cc2);
    CSprite::SetBrightness(this_00,0,time);
    func.__delta = 0;
    func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)*(uint *)(unaff_EBX + 0x4fbd56);
    CBaseEntity::ThinkSet(&this_00->super_CBaseEntity,func,0.0,(char *)0x0);
    piVar1 = *(int **)(&DAT_004fb552 + unaff_EBX);
    this_00->m_flDieTime = time + *(float *)(*piVar1 + 0xc);
    CBaseEntity::SetNextThink(&this_00->super_CBaseEntity,*(float *)(*piVar1 + 0xc),(char *)0x0);
    this->m_pLaserGlow = (CSprite *)0x0;
  }
  if (this->m_pAmbientSound != (CSoundPatch *)0x0) {
    pCVar2 = CSoundEnvelopeController::GetController();
                    /* Unresolved local var: CSoundEnvelopeController * controller@[???] */
    (*pCVar2->_vptr_CSoundEnvelopeController[0xb])(pCVar2,this->m_pAmbientSound);
    this->m_pAmbientSound = (CSoundPatch *)0x0;
  }
  CBaseEntity::SetNextThink((CBaseEntity *)this,-1.0,(char *)0x0);
  CBaseEntity::ThinkSet((CBaseEntity *)this,(BASEPTR)0x0,0.0,(char *)0x0);
  return;
}


/* CPortalLaser::TurnOn at 006b0a90 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void TurnOn(CPortalLaser * this) */

void __thiscall CPortalLaser::TurnOn(CPortalLaser *this)

{
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((this->super_CBaseAnimating).super_CBaseEntity.m_pfnThink.__pfn !=
      (_func___thiscall_void_CBaseEntity_ptr *)0x0) {
    return;
  }
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0x51bf);
  CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(**(int **)(unaff_EBX + 0x4fb0ff) + 0xc) +
             *(float *)(*(int *)(unaff_EBX + 0x67dcbb) + 0x2c),(char *)0x0);
  TurnOnGlow(this);
  return;
}


/* CPortalLaser::BeamDamage at 006b0cf0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void BeamDamage(CPortalLaser * this, trace_t * ptr) */

void __thiscall CPortalLaser::BeamDamage(CPortalLaser *this,trace_t *ptr)

{
  bool bVar1;
  CBaseEntity *this_00;
  int *piVar2;
  int unaff_EBX;
  CTakeDamageInfo *this_1;
  CTakeDamageInfo *pCVar3;
  CTakeDamageInfo *in_stack_ffffff0c;
  CBaseEntity *in_stack_ffffff10;
  CBaseEntity *in_stack_ffffff14;
  float in_stack_ffffff18;
  int in_stack_ffffff1c;
  CBaseEntity *this_01;
  CTakeDamageInfo local_d8;
  CTakeDamageInfo local_88;
  float local_38;
  float local_34;
  float local_30;
  Vector local_2c;
  CTakeDamageInfo *local_20;
  
  ___i686_get_pc_thunk_bx();
  if ((*(float *)(unaff_EBX + 0x37960f) != (ptr->super_CBaseTrace).fraction) &&
     (this_01 = ptr->m_pEnt, this_01 != (CBaseEntity *)0x0)) {
    ClearMultiDamage();
    if (((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
                    /* Unresolved local var: Vector res@[???] */
    local_38 = (ptr->super_CBaseTrace).endpos.x -
               (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.x;
    local_34 = (ptr->super_CBaseTrace).endpos.y -
               (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.y;
    local_30 = (ptr->super_CBaseTrace).endpos.z -
               (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.z;
    local_2c.x = local_38;
    local_2c.y = local_34;
    local_2c.z = local_30;
    VectorNormalize(&local_2c);
    CTakeDamageInfo::CTakeDamageInfo
              (&local_88,(CBaseEntity *)this,(CBaseEntity *)this,
               *(float *)(unaff_EBX + 0x37999f) *
               *(float *)(**(int **)(unaff_EBX + 0x4fae9f) + 0x10),8,0,in_stack_ffffff0c,
               in_stack_ffffff10,in_stack_ffffff14,in_stack_ffffff18,in_stack_ffffff1c,(int)this_01)
    ;
    CalculateMeleeDamageForce(&local_88,&local_2c,&(ptr->super_CBaseTrace).endpos,1.0);
    CBaseEntity::DispatchTraceAttack(this_01,&local_88,&local_2c,ptr);
    ApplyMultiDamage();
    this_1 = (CTakeDamageInfo *)(unaff_EBX + 0x35e745);
    UTIL_DecalTrace(ptr,(char *)this_1);
    this_00 = (CBaseEntity *)
              (*(this_01->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                _vptr_IHandleEntity[0x36])(this_01);
    if ((this_00 != (CBaseEntity *)0x0) &&
       ((((pCVar3 = (CTakeDamageInfo *)(&UNK_0030d0a7 + unaff_EBX), local_20 = pCVar3,
          (CTakeDamageInfo *)(this_00->m_iClassname).pszValue == pCVar3 ||
          (bVar1 = CBaseEntity::ClassMatchesComplex(this_00,(char *)pCVar3), this_1 = pCVar3, bVar1)
          ) || (pCVar3 = (CTakeDamageInfo *)(unaff_EBX + 0x35b29f), local_20 = pCVar3,
               (CTakeDamageInfo *)(this_00->m_iClassname).pszValue == pCVar3)) ||
        (bVar1 = CBaseEntity::ClassMatchesComplex(this_00,(char *)pCVar3), this_1 = pCVar3, bVar1)))
       ) {
      if (*(int *)(*(int *)(*(int *)(&DAT_004fb92f + unaff_EBX) + 0x1c) + 0x30) != 0) {
                    /* Unresolved local var: CTakeDamageInfo info@[???]
                       Unresolved local var: CNPC_Portal_FloorTurret * pTurret@[???] */
        CTakeDamageInfo::CTakeDamageInfo(&local_d8,this_1);
        local_d8.m_flDamage = 1.0;
        local_d8.m_bitsDamageType = 1;
        (*(this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
          _vptr_IHandleEntity[0x46])(this_00,&local_d8);
        (*(this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
          _vptr_IHandleEntity[0x282])(this_00);
        CBaseEntity::ThinkSet(this_00,(BASEPTR)0x0,0.0,(char *)0x0);
        return;
      }
      (*(this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
        [0xde])(this_00,0x41f00000,1,0,0);
      return;
    }
    if (((this_01->m_MoveType).m_Value == '\x06') &&
       (piVar2 = (int *)___dynamic_cast(this_01,*(undefined4 *)(unaff_EBX + 0x4faec7),
                                        *(undefined4 *)(unaff_EBX + 0x4fafbf),0),
       piVar2 != (int *)0x0)) {
      (**(code **)(*piVar2 + 0x378))(piVar2,0x41f00000,0,0,0);
    }
  }
  return;
}


/* CPortalLaser::TurnOnGlow at 006b0920 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void TurnOnGlow(CPortalLaser * this) */

void __thiscall CPortalLaser::TurnOnGlow(CPortalLaser *this)

{
  undefined1 *puVar1;
  int iVar2;
  uint uVar3;
  CBaseEdict *pCVar4;
  CSprite *this_00;
  int iVar5;
  ulong *puVar6;
  IChangeInfoAccessor *pIVar7;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_pLaserGlow == (CSprite *)0x0) {
    this_00 = CSprite::SpriteCreate
                        ((char *)(unaff_EBX + 0x35de84),
                         (Vector *)&(this->super_CBaseAnimating).super_CBaseEntity.m_vecOrigin,false
                        );
    this->m_pLaserGlow = this_00;
    iVar2 = this->m_iLaserAttachment;
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
    uVar3 = (this_00->m_hAttachedToEntity).
            super_CNetworkVarBase<CBaseHandle,CSprite::NetworkVar_m_hAttachedToEntity>.m_Value.
            m_Index;
    if (((uVar3 == 0xffffffff) ||
        (iVar5 = (uVar3 & 0xffff) * 0x10 + **(int **)(&DAT_004fb270 + unaff_EBX),
        *(uint *)(iVar5 + 8) != uVar3 >> 0x10)) || (this != *(CPortalLaser **)(iVar5 + 4))) {
      if ((this_00->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
        pCVar4 = &((this_00->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
        if (pCVar4 != (CBaseEdict *)0x0) {
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x101;
          pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
          pIVar7->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this_00->super_CBaseEntity).m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      puVar6 = (ulong *)(*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.
                          super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[3])(this);
      (this_00->m_hAttachedToEntity).
      super_CNetworkVarBase<CBaseHandle,CSprite::NetworkVar_m_hAttachedToEntity>.m_Value.m_Index =
           *puVar6;
    }
    if (iVar2 != (this_00->m_nAttachment).m_Value) {
      if ((this_00->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
        pCVar4 = &((this_00->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
        if (pCVar4 != (CBaseEdict *)0x0) {
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x101;
          pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
          pIVar7->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this_00->super_CBaseEntity).m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this_00->m_nAttachment).m_Value = iVar2;
    }
    CBaseEntity::FollowEntity(&this_00->super_CBaseEntity,(CBaseEntity *)this,true);
    CSprite::SetBrightness(this->m_pLaserGlow,0x80,0.1);
    CSprite::SetScale(this->m_pLaserGlow,2.0,0.1);
  }
  return;
}


/* CPortalLaser::TurnOffGlow at 006b0000 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void TurnOffGlow(CPortalLaser * this) */

void __thiscall CPortalLaser::TurnOffGlow(CPortalLaser *this)

{
  float time;
  CSprite *this_00;
  int *piVar1;
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  this_00 = this->m_pLaserGlow;
  if (this_00 != (CSprite *)0x0) {
    time = *(float *)(unaff_EBX + 0x37a2fc);
    CSprite::SetBrightness(this_00,0,time);
    func.__delta = 0;
    func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)*(uint *)(unaff_EBX + 0x4fc390);
    CBaseEntity::ThinkSet(&this_00->super_CBaseEntity,func,0.0,(char *)0x0);
    piVar1 = *(int **)(unaff_EBX + 0x4fbb8c);
    this_00->m_flDieTime = time + *(float *)(*piVar1 + 0xc);
    CBaseEntity::SetNextThink(&this_00->super_CBaseEntity,*(float *)(*piVar1 + 0xc),(char *)0x0);
    this->m_pLaserGlow = (CSprite *)0x0;
  }
  return;
}


/* CPortalLaser::TurnOffLaserSound at 006affc0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void TurnOffLaserSound(CPortalLaser * this) */

void __thiscall CPortalLaser::TurnOffLaserSound(CPortalLaser *this)

{
  CSoundEnvelopeController *pCVar1;
  
  if (this->m_pAmbientSound != (CSoundPatch *)0x0) {
                    /* Unresolved local var: CSoundEnvelopeController * controller@[???] */
    pCVar1 = CSoundEnvelopeController::GetController();
    (*pCVar1->_vptr_CSoundEnvelopeController[0xb])(pCVar1,this->m_pAmbientSound);
    this->m_pAmbientSound = (CSoundPatch *)0x0;
  }
  return;
}


/* CPortalLaser::FireAtPoint at 006b0ff0 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void FireAtPoint(CPortalLaser * this, trace_t * tr, bool bImpact) */

void __thiscall CPortalLaser::FireAtPoint(CPortalLaser *this,trace_t *tr,bool bImpact)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  BeamDamage(this,tr);
  if (bImpact) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4facd3) + 0x10))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4facd3),&(tr->super_CBaseTrace).endpos,1,1,0);
  }
  return;
}


/* CPortalLaser::FireLaser at 006b42b0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void FireLaser(CPortalLaser * this, int nNumBeams, Vector * vecStart,
   Vector * vecDirection, CBaseEntity * pParent) */

void __thiscall
CPortalLaser::FireLaser
          (CPortalLaser *this,int nNumBeams,Vector *vecStart,Vector *vecDirection,
          CBaseEntity *pParent)

{
  undefined1 *puVar1;
  CBeam *pCVar2;
  CBaseEdict *this_00;
  undefined4 *puVar3;
  VMatrix matThisToLinked;
  VMatrix matThisToLinked_00;
  bool bVar4;
  IChangeInfoAccessor *pIVar5;
  float *pfVar6;
  CPortalSimulator *pCVar7;
  CBaseEntity *pCVar8;
  CBaseEntity *pCVar9;
  Vector *pVVar10;
  int *piVar11;
  int iVar12;
  int iVar13;
  int unaff_EBX;
  EHANDLE *pEVar14;
  EHANDLE *pEVar15;
  VMatrix *pVVar16;
  CBaseHandle *pCVar17;
  CBaseHandle *pCVar18;
  vec_t *pvVar19;
  double dVar20;
  float fVar21;
  CPortalLaser *pCVar22;
  Vector *pVVar23;
  trace_t *trace;
  Vector *pVVar24;
  Vector *pVVar25;
  trace_t *ptVar26;
  undefined4 in_stack_fffffc88;
  undefined4 in_stack_fffffc8c;
  CTakeDamageInfo *pCVar27;
  CBaseEntity *in_stack_fffffc90;
  CBaseEntity *in_stack_fffffc94;
  float in_stack_fffffc98;
  int in_stack_fffffc9c;
  int in_stack_fffffca0;
  vec_t in_stack_fffffca4 [4];
  Ray_t *ray;
  Vector *ptTransformed;
  trace_t local_2f4;
  trace_t local_2a0;
  undefined1 local_24c [48];
  vec_t local_21c;
  vec_t local_218;
  vec_t local_214;
  matrix3x4_t *local_20c;
  bool local_208;
  undefined3 uStack_207;
  short local_204;
  ushort local_202;
  CBaseEntity *local_200;
  CTakeDamageInfo local_1ec;
  float local_194;
  float local_190;
  float local_18c;
  float local_188;
  float local_184;
  float local_180;
  float local_17c;
  float local_178;
  float local_174;
  float local_170;
  float local_16c;
  float local_168;
  Vector local_164;
  float local_158;
  float local_154;
  float local_150;
  float local_14c;
  float local_148;
  float local_144;
  float local_140;
  float local_13c;
  float local_138;
  Vector local_134;
  float local_128;
  float local_124;
  float local_120;
  Vector local_11c;
  float local_110;
  float local_10c;
  float local_108;
  float local_104;
  float local_100;
  float local_fc;
  float local_f8;
  float local_f4;
  float local_f0;
  float local_ec;
  float local_e8;
  float local_e4;
  Vector local_e0;
  float local_d4;
  float local_d0;
  float local_cc;
  float local_c8;
  float local_c4;
  float local_c0;
  float local_bc;
  float local_b8;
  float local_b4;
  EHANDLE local_b0;
  EHANDLE local_ac;
  EHANDLE local_a8;
  float local_a4;
  float local_a0;
  float local_9c;
  float local_98;
  float local_94;
  float local_90;
  float local_8c;
  float local_88;
  float local_84;
  float local_80;
  float local_7c;
  float local_78;
  float local_74;
  float local_70;
  float local_6c;
  Vector local_68;
  float local_5c;
  float local_58;
  float local_54;
  Vector local_50;
  float local_44;
  float local_40;
  float local_3c;
  float local_38;
  float local_34;
  float local_30;
  CBaseHandle local_2c;
  CPortal_Base2D *local_28;
  IHandleEntity local_24;
  IHandleEntity local_20;
  
  ___i686_get_pc_thunk_bx();
  if (7 < nNumBeams) {
    return;
  }
  if (this->m_pChildBeams[nNumBeams] == (CBeam *)0x0) {
    return;
  }
  if (*(int *)(*(int *)(&DAT_0067a43b + unaff_EBX) + 0x30) != 0) {
    CBeam::TurnOff(this->m_pChildBeams[nNumBeams]);
    local_134.x = vecDirection->x;
    local_134.y = vecDirection->y;
    local_134.z = vecDirection->z;
    pCVar8 = TraceLaser(this,true,vecStart,&local_134,(float *)&local_2c,(trace_t *)local_24c,
                        &local_e0);
    if ((pCVar8 != (CBaseEntity *)0x0) &&
       ((local_28 = (CPortal_Base2D *)(unaff_EBX + 0x35a50f),
        (CPortal_Base2D *)(pCVar8->m_iClassname).pszValue == local_28 ||
        (bVar4 = CBaseEntity::ClassMatchesComplex(pCVar8,(char *)local_28), bVar4)))) {
                    /* Unresolved local var: Vector res@[???] */
      local_38 = (float)local_2c.m_Index * vecDirection->x;
      local_34 = (float)local_2c.m_Index * vecDirection->y;
      local_30 = (float)local_2c.m_Index * vecDirection->z;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
      local_44 = vecStart->x + local_38 + local_e0.x;
      local_40 = vecStart->y + local_34 + local_e0.y;
      local_3c = vecStart->z + local_30 + local_e0.z;
                    /* Unresolved local var: Vector res@[???] */
      local_50.x = local_44 - vecStart->x;
      local_50.y = local_40 - vecStart->y;
      local_50.z = local_3c - vecStart->z;
                    /* Unresolved local var: Vector norm@[???] */
      VectorNormalize(&local_50);
      local_11c.x = local_50.x;
      local_11c.y = local_50.y;
      local_11c.z = local_50.z;
      pCVar27 = (CTakeDamageInfo *)&local_d4;
      pCVar9 = TraceLaser(this,false,vecStart,&local_11c,(float *)&local_28,&local_2a0,
                          (Vector *)pCVar27);
      if (*(int *)(*(int *)(unaff_EBX + 0x67a4fb) + 0x30) != 0) {
        puVar3 = *(undefined4 **)(unaff_EBX + 0x4f78f7);
        (**(code **)(*(int *)*puVar3 + 0xbc))();
        (**(code **)(*(int *)*puVar3 + 0xbc))();
      }
      if (pCVar8 == pCVar9) {
        local_24c._0_4_ = local_2a0.super_CBaseTrace.startpos.x;
        local_24c._4_4_ = local_2a0.super_CBaseTrace.startpos.y;
        local_24c._8_4_ = local_2a0.super_CBaseTrace.startpos.z;
        local_24c._12_4_ = local_2a0.super_CBaseTrace.endpos.x;
        local_24c._16_4_ = local_2a0.super_CBaseTrace.endpos.y;
        local_24c._20_4_ = local_2a0.super_CBaseTrace.endpos.z;
        local_24c._24_4_ = local_2a0.super_CBaseTrace.plane.normal.x;
        local_24c._28_4_ = local_2a0.super_CBaseTrace.plane.normal.y;
        local_24c._32_4_ = local_2a0.super_CBaseTrace.plane.normal.z;
        local_204 = local_2a0.physicsbone;
        local_202 = local_2a0.worldSurfaceIndex;
        local_200 = local_2a0.m_pEnt;
        local_2c.m_Index = (ulong)local_28;
        local_134.x = local_11c.x;
        local_134.y = local_11c.y;
        local_134.z = local_11c.z;
      }
                    /* Unresolved local var: CTakeDamageInfo info@[???] */
      CTakeDamageInfo::CTakeDamageInfo
                (&local_1ec,(CBaseEntity *)this,(CBaseEntity *)this,
                 *(float *)(**(int **)(unaff_EBX + 0x4f78df) + 0x10),8,0,pCVar27,in_stack_fffffc90,
                 in_stack_fffffc94,in_stack_fffffc98,in_stack_fffffc9c,in_stack_fffffca0);
      pVVar10 = (Vector *)
                (*(pCVar8->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                  _vptr_IHandleEntity[0x9d])();
      local_164.x = 1.0;
      local_164.y = 0.0;
      local_164.z = 0.0;
      CalculateMeleeDamageForce(&local_1ec,&local_164,pVVar10,1.0);
      CBaseEntity::TakeDamage(pCVar8,&local_1ec);
    }
                    /* Unresolved local var: Vector res@[???] */
    local_5c = (float)local_2c.m_Index * local_134.x;
    local_58 = (float)local_2c.m_Index * local_134.y;
    local_54 = (float)local_2c.m_Index * local_134.z;
                    /* Unresolved local var: Vector res@[???] */
    local_68.x = vecStart->x + local_5c;
    local_68.y = vecStart->y + local_58;
    local_68.z = vecStart->z + local_54;
    CBeam::PointsInit(this->m_pChildBeams[nNumBeams],vecStart,&local_68);
    CBaseEntity::SetAbsOrigin(&this->m_pChildBeams[nNumBeams]->super_CBaseEntity,vecStart);
    if (pCVar8 != (CBaseEntity *)0x0) {
      local_28 = (CPortal_Base2D *)(unaff_EBX + 0x35a50f);
      if ((CPortal_Base2D *)(pCVar8->m_iClassname).pszValue == local_28) {
        return;
      }
      bVar4 = CBaseEntity::ClassMatchesComplex(pCVar8,(char *)local_28);
      if (bVar4) {
        return;
      }
    }
    if (local_200 != (CBaseEntity *)0x0) {
      bVar4 = CPhysicsShadowClone::IsShadowClone(local_200);
      pCVar8 = local_200;
      if (bVar4) {
        CPhysicsShadowClone::GetClonedEntity((CPhysicsShadowClone *)&local_20);
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
        if (((IHandleEntity)local_20._vptr_IHandleEntity == (IHandleEntity)0xffffffff) ||
           (iVar12 = ((uint)local_20._vptr_IHandleEntity & 0xffff) * 0x10 +
                     **(int **)(unaff_EBX + 0x4f78e3),
           *(uint *)(iVar12 + 8) != (uint)local_20._vptr_IHandleEntity >> 0x10)) {
          pCVar8 = (CBaseEntity *)0x0;
        }
        else {
          pCVar8 = *(CBaseEntity **)(iVar12 + 4);
        }
      }
      bVar4 = UTIL_IsReflectiveCube(pCVar8);
      if (bVar4) {
        if ((pCVar8->m_iEFlags & 0x800) != 0) {
          CBaseEntity::CalcAbsolutePosition(pCVar8);
        }
        AngleVectors(&pCVar8->m_angAbsRotation,(Vector *)&local_d4);
                    /* Unresolved local var: Vector res@[???] */
        local_6c = *(float *)(unaff_EBX + 0x37cfa7);
        local_74 = local_d4 * local_6c;
        local_70 = local_d0 * local_6c;
        local_6c = local_6c * local_cc;
        pfVar6 = (float *)(*(pCVar8->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                            _vptr_IHandleEntity[0x9d])();
                    /* Unresolved local var: Vector res@[???] */
        local_11c.x = *pfVar6 + local_74;
        local_11c.y = pfVar6[1] + local_70;
        local_11c.z = pfVar6[2] + local_6c;
        local_80 = local_11c.x;
        local_7c = local_11c.y;
        local_78 = local_11c.z;
                    /* Unresolved local var: CPortalSimulator * pSimulator@[???] */
        pCVar7 = CPortalSimulator::GetSimulatorThatOwnsEntity(pCVar8);
        if ((pCVar7 != (CPortalSimulator *)0x0) &&
           (bVar4 = CPortalSimulator::EntityIsInPortalHole(pCVar7,pCVar8), bVar4)) {
          fVar21 = (local_11c.x *
                    (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.PortalPlane.m_Normal.
                    x + local_11c.y *
                        (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.PortalPlane.
                        m_Normal.y +
                   local_11c.z *
                   (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.PortalPlane.m_Normal.z
                   ) - (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.PortalPlane.m_Dist
          ;
          if ((fVar21 < *(float *)(unaff_EBX + 0x3762af)) &&
             ((!NAN(fVar21) && !NAN(*(float *)(unaff_EBX + 0x3762af)) &&
              (pfVar6 = (float *)(*(pCVar8->super_IServerEntity).super_IServerUnknown.
                                   super_IHandleEntity._vptr_IHandleEntity[0x9d])(),
              fVar21 = (*pfVar6 * (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.
                                  PortalPlane.m_Normal.x +
                        pfVar6[1] *
                        (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.PortalPlane.
                        m_Normal.y +
                       pfVar6[2] *
                       (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.PortalPlane.
                       m_Normal.z) -
                       (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.PortalPlane.m_Dist
              , *(float *)(unaff_EBX + 0x3762af) <= fVar21 &&
                fVar21 != *(float *)(unaff_EBX + 0x3762af))))) {
                    /* Unresolved local var: Vector vRet@[???] */
            local_8c = (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                       [0][0] * local_11c.x +
                       (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                       [0][1] * local_11c.y +
                       (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                       [0][2] * local_11c.z +
                       (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                       [0][3];
            local_88 = (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                       [1][0] * local_11c.x +
                       (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                       [1][1] * local_11c.y +
                       (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                       [1][2] * local_11c.z +
                       (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                       [1][3];
            local_11c.z = (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked
                          .m[2][0] * local_11c.x +
                          (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked
                          .m[2][1] * local_11c.y +
                          (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked
                          .m[2][2] * local_11c.z +
                          (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked
                          .m[2][3];
            local_174 = local_d4 *
                        (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                        [2][0] + local_d0 *
                                 (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.
                                 matThisToLinked.m[2][1] +
                        local_cc *
                        (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                        [2][2];
            local_178 = local_d4 *
                        (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                        [1][0] + local_d0 *
                                 (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.
                                 matThisToLinked.m[1][1] +
                        local_cc *
                        (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                        [1][2];
            local_17c = local_d4 *
                        (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                        [0][0] + local_d0 *
                                 (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.
                                 matThisToLinked.m[0][1] +
                        local_cc *
                        (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                        [0][2];
            local_170 = local_17c;
            local_16c = local_178;
            local_168 = local_174;
            local_11c.x = local_8c;
            local_11c.y = local_88;
            local_d4 = local_17c;
            local_d0 = local_178;
            local_cc = local_174;
            local_98 = local_17c;
            local_94 = local_178;
            local_90 = local_174;
            local_84 = local_11c.z;
          }
        }
        this->m_bHasHitCube = true;
        piVar11 = (int *)(*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.
                           super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[3])();
        pCVar8[2].m_iEFlags = *piVar11;
        pCVar8[2].m_fFlags.m_Value =
             (int)(*(float *)(**(int **)(unaff_EBX + 0x4f78df) + 0xc) +
                  *(float *)(unaff_EBX + 0x3771a3));
        FireLaser(this,nNumBeams + 1,&local_11c,(Vector *)&local_d4,pCVar8);
        return;
      }
    }
    iVar12 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4f798f) + 0x18))();
    if ((iVar12 != 0) && (*(short *)(iVar12 + 0x4c) == 0x52)) {
                    /* Unresolved local var: Vector vecDir@[???]
                       Unresolved local var: Vector vecBounce@[???]
                       Unresolved local var: Vector res@[???] */
      local_11c.x = (float)local_24c._12_4_ - (float)local_24c._0_4_;
      local_11c.y = (float)local_24c._16_4_ - (float)local_24c._4_4_;
      local_11c.z = (float)local_24c._20_4_ - (float)local_24c._8_4_;
      local_a4 = local_11c.x;
      local_a0 = local_11c.y;
      local_9c = local_11c.z;
      VectorNormalize(&local_11c);
      fVar21 = (float)local_24c._24_4_ * local_11c.x + local_11c.y * (float)local_24c._28_4_ +
               local_11c.z * (float)local_24c._32_4_;
                    /* Unresolved local var: Vector res@[???] */
      local_144 = *(float *)(unaff_EBX + 0x3766cb);
      local_14c = (float)local_24c._24_4_ * local_144;
      local_148 = (float)local_24c._28_4_ * local_144;
      local_144 = local_144 * (float)local_24c._32_4_;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
      local_d4 = fVar21 * local_14c + local_11c.x;
      local_d0 = fVar21 * local_148 + local_11c.y;
      local_cc = fVar21 * local_144 + local_11c.z;
      local_b0.super_CBaseHandle.m_Index = (CBaseHandle)(CBaseHandle)local_d4;
      local_ac.super_CBaseHandle.m_Index = (CBaseHandle)(CBaseHandle)local_d0;
      local_a8.super_CBaseHandle.m_Index = (CBaseHandle)(CBaseHandle)local_cc;
      VectorNormalize((Vector *)&local_d4);
      dVar20 = (double)(local_d4 * local_11c.x + local_d0 * local_11c.y + local_cc * local_11c.z);
      if (*(double *)(unaff_EBX + 0x37ecef) <= dVar20 && dVar20 != *(double *)(unaff_EBX + 0x37ecef)
         ) {
        FireLaser(this,nNumBeams + 1,(Vector *)(local_24c + 0xc),(Vector *)&local_d4,
                  (CBaseEntity *)0x0);
        BeamDamage(this,(trace_t *)local_24c);
        return;
      }
    }
    UTIL_SetOrigin((CBaseEntity *)this->m_pPlacementHelper,(Vector *)(local_24c + 0xc),false);
    BeamDamage(this,(trace_t *)local_24c);
    (**(code **)(*(int *)**(undefined4 **)(CNPC_FloorTurret::InputRestoreAmmo + unaff_EBX + 3) +
                0x10))();
    return;
  }
  CTraceFilterSimpleClassnameList::CTraceFilterSimpleClassnameList
            ((CTraceFilterSimpleClassnameList *)&local_2a0,(IHandleEntity *)this,0);
  CTraceFilterSimpleClassnameList::AddClassnameToIgnore
            ((CTraceFilterSimpleClassnameList *)&local_2a0,&UNK_00304f23 + unaff_EBX);
  CTraceFilterSimpleClassnameList::AddClassnameToIgnore
            ((CTraceFilterSimpleClassnameList *)&local_2a0,(char *)(unaff_EBX + 0x301eab));
  CTraceFilterSimpleClassnameList::AddClassnameToIgnore
            ((CTraceFilterSimpleClassnameList *)&local_2a0,(char *)(unaff_EBX + 0x35a50f));
                    /* Unresolved local var: Vector res@[???] */
  local_b4 = *(float *)(unaff_EBX + 0x376fc3);
  local_bc = vecDirection->x * local_b4;
  local_b8 = vecDirection->y * local_b4;
  local_b4 = local_b4 * vecDirection->z;
                    /* Unresolved local var: Vector res@[???] */
  local_24c._0_4_ = vecStart->x;
  local_c8 = (float)local_24c._0_4_ + local_bc;
  local_24c._4_4_ = vecStart->y;
  local_c4 = (float)local_24c._4_4_ + local_b8;
  local_24c._8_4_ = vecStart->z;
  local_c0 = (float)local_24c._8_4_ + local_b4;
  local_24c._16_4_ = local_c8 - vecStart->x;
  local_24c._20_4_ = local_c4 - vecStart->y;
  local_24c._24_4_ = local_c0 - vecStart->z;
  local_214 = 0.0;
  local_218 = 0.0;
  local_21c = 0.0;
  local_20c = (matrix3x4_t *)0x0;
  _local_208 = CONCAT31(CONCAT21(uStack_207._1_2_,
                                 (float)local_24c._16_4_ * (float)local_24c._16_4_ +
                                 (float)local_24c._20_4_ * (float)local_24c._20_4_ +
                                 (float)local_24c._24_4_ * (float)local_24c._24_4_ != 0.0),true);
  local_24c._40_4_ = 0.0;
  local_24c._36_4_ = 0.0;
  local_24c._32_4_ = 0.0;
  trace = &local_2f4;
  ray = (Ray_t *)local_24c;
  ptVar26 = trace;
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4f794b) + 0x14))();
  pVVar10 = &local_2f4.super_CBaseTrace.endpos;
  if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x4f7947) + 0x1c) + 0x30) != 0) {
    in_stack_fffffc8c = 0xbf800000;
    in_stack_fffffc88 = 1;
    ptVar26 = (trace_t *)0x0;
    DebugDrawLine((Vector *)trace,pVVar10,0xff,0,0,true,-1.0);
  }
  UpdateSoundPosition(this,(Vector *)trace,pVVar10);
  local_28 = (CPortal_Base2D *)0x0;
  bVar4 = UTIL_DidTraceTouchPortals(ray,trace,&local_28,(CPortal_Base2D **)0x0);
  if (((bVar4) && (local_28 != (CPortal_Base2D *)0x0)) &&
     (bVar4 = CPortal_Base2D::IsActivedAndLinked(local_28), bVar4)) {
    CBeam::TurnOff(this->m_pChildBeams[nNumBeams]);
    pVVar24 = (Vector *)&local_d4;
    pVVar25 = pVVar24;
    bVar4 = StrikeEntitiesAlongLaser(this,(Vector *)trace,pVVar10,pVVar24);
    if (bVar4) {
LAB_006b49fd:
      CBeam::PointsInit(this->m_pChildBeams[nNumBeams],vecStart,pVVar24);
      CBaseEntity::SetAbsOrigin(&this->m_pChildBeams[nNumBeams]->super_CBaseEntity,vecStart);
    }
    else {
      CBeam::TurnOff(this->m_pChildBeams[nNumBeams]);
      pVVar24 = pVVar10;
      CBeam::PointsInit(this->m_pChildBeams[nNumBeams],vecStart,pVVar10);
      CBaseEntity::SetAbsOrigin(&this->m_pChildBeams[nNumBeams]->super_CBaseEntity,vecStart);
      pCVar22 = this;
      BeamDamage(this,trace);
      local_1ec.m_iDamageCustom = 0;
      pCVar27 = &local_1ec;
      pVVar16 = &local_28->m_matrixThisToLinked;
      pvVar19 = (vec_t *)&stack0xfffffc74;
      for (iVar12 = 0x10; iVar12 != 0; iVar12 = iVar12 + -1) {
        *pvVar19 = pVVar16->m[0][0];
        pVVar16 = (VMatrix *)(pVVar16->m[0] + 1);
        pvVar19 = pvVar19 + 1;
      }
      matThisToLinked.m[0][1] = (vec_t)trace;
      matThisToLinked.m[0][0] = (vec_t)pCVar22;
      matThisToLinked.m[0][2] = (vec_t)pVVar24;
      matThisToLinked.m[0][3] = (vec_t)pVVar25;
      matThisToLinked.m[1][0] = (vec_t)ptVar26;
      matThisToLinked.m[1][1] = (vec_t)in_stack_fffffc88;
      matThisToLinked.m[1][2] = (vec_t)in_stack_fffffc8c;
      matThisToLinked.m[1][3] = (vec_t)in_stack_fffffc90;
      matThisToLinked.m[2][0] = (vec_t)in_stack_fffffc94;
      matThisToLinked.m[2][1] = in_stack_fffffc98;
      matThisToLinked.m[2][2] = (vec_t)in_stack_fffffc9c;
      matThisToLinked.m[2][3] = (vec_t)in_stack_fffffca0;
      matThisToLinked.m[3][0] = in_stack_fffffca4[0];
      matThisToLinked.m[3][1] = in_stack_fffffca4[1];
      matThisToLinked.m[3][2] = in_stack_fffffca4[2];
      matThisToLinked.m[3][3] = in_stack_fffffca4[3];
      UTIL_Portal_RayTransform(matThisToLinked,ray,(Ray_t *)pCVar27);
      local_11c.x = local_1ec.m_vecDamagePosition.y;
      local_11c.y = local_1ec.m_vecDamagePosition.z;
      local_11c.z = local_1ec.m_vecReportedPosition.x;
      pVVar23 = &local_11c;
      VectorNormalize(&local_11c);
      pVVar16 = &local_28->m_matrixThisToLinked;
      pvVar19 = (vec_t *)&stack0xfffffc74;
      ptTransformed = &local_134;
      for (iVar12 = 0x10; iVar12 != 0; iVar12 = iVar12 + -1) {
        *pvVar19 = pVVar16->m[0][0];
        pVVar16 = (VMatrix *)(pVVar16->m[0] + 1);
        pvVar19 = pvVar19 + 1;
      }
      matThisToLinked_00.m[0][1] = (vec_t)trace;
      matThisToLinked_00.m[0][0] = (vec_t)pVVar23;
      matThisToLinked_00.m[0][2] = (vec_t)pVVar24;
      matThisToLinked_00.m[0][3] = (vec_t)pVVar25;
      matThisToLinked_00.m[1][0] = (vec_t)ptVar26;
      matThisToLinked_00.m[1][1] = (vec_t)in_stack_fffffc88;
      matThisToLinked_00.m[1][2] = (vec_t)in_stack_fffffc8c;
      matThisToLinked_00.m[1][3] = (vec_t)in_stack_fffffc90;
      matThisToLinked_00.m[2][0] = (vec_t)in_stack_fffffc94;
      matThisToLinked_00.m[2][1] = in_stack_fffffc98;
      matThisToLinked_00.m[2][2] = (vec_t)in_stack_fffffc9c;
      matThisToLinked_00.m[2][3] = (vec_t)in_stack_fffffca0;
      matThisToLinked_00.m[3][0] = in_stack_fffffca4[0];
      matThisToLinked_00.m[3][1] = in_stack_fffffca4[1];
      matThisToLinked_00.m[3][2] = in_stack_fffffca4[2];
      matThisToLinked_00.m[3][3] = in_stack_fffffca4[3];
      UTIL_Portal_PointTransform(matThisToLinked_00,pVVar10,ptTransformed);
      FireLaser(this,nNumBeams + 1,&local_134,&local_11c,(CBaseEntity *)0x0);
    }
LAB_006b4a35:
    local_2a0.super_CBaseTrace.startpos.x = (vec_t)(*(int *)(&DAT_004f7a07 + unaff_EBX) + 8);
    local_2a0.super_CBaseTrace.plane.normal.y = 0.0;
    if ((int)local_2a0.super_CBaseTrace.plane.normal.x < 0) {
LAB_006b51dd:
      local_2a0.super_CBaseTrace.plane.normal.y = 0.0;
      goto LAB_006b4a8a;
    }
    if (local_2a0.super_CBaseTrace.endpos.y != 0.0) {
      (**(code **)(*(int *)**(undefined4 **)
                             (CTurretTipController::~CTurretTipController + unaff_EBX + 7) + 8))();
LAB_006b4a78:
      local_2a0.super_CBaseTrace.endpos.y = 0.0;
    }
  }
  else {
    CBeam::TurnOff(this->m_pChildBeams[nNumBeams]);
    pVVar24 = &local_e0;
    bVar4 = StrikeEntitiesAlongLaser(this,(Vector *)trace,pVVar10,pVVar24);
    if (bVar4) goto LAB_006b49fd;
    if (pParent == (CBaseEntity *)0x0) {
      CBeam::PointsInit(this->m_pChildBeams[nNumBeams],vecStart,pVVar10);
      CBaseEntity::SetAbsOrigin(&this->m_pChildBeams[nNumBeams]->super_CBaseEntity,vecStart);
    }
    else {
      CBeam::PointEntInit(this->m_pChildBeams[nNumBeams],pVVar10,pParent);
      pCVar2 = this->m_pChildBeams[nNumBeams];
      local_2c.m_Index = (int)&vgui::Panel::GetMessageMap::s_pMap.magic + 1;
      iVar12 = (pCVar2->m_nNumBeamEnts).m_Value;
      bVar4 = iVar12 == 1;
      iVar13 = 4;
      pEVar14 = (pCVar2->m_hAttachEntity).m_Value + iVar12 + 9;
      pCVar17 = &local_2c;
      do {
        pEVar15 = pEVar14;
        pCVar18 = pCVar17;
        if (iVar13 == 0) break;
        iVar13 = iVar13 + -1;
        pCVar18 = (CBaseHandle *)((int)&pCVar17->m_Index + 1);
        pEVar15 = (EHANDLE *)((int)&(pEVar14->super_CBaseHandle).m_Index + 1);
        bVar4 = (char)(pEVar14->super_CBaseHandle).m_Index == (char)pCVar17->m_Index;
        pEVar14 = pEVar15;
        pCVar17 = pCVar18;
      } while (bVar4);
      iVar13 = 0;
      if (!bVar4) {
        iVar13 = (uint)*(byte *)((int)(pEVar15 + -1) + 3) -
                 (uint)*(byte *)((int)&pCVar18[-1].m_Index + 3);
      }
      if (iVar13 != 0) {
        if ((pCVar2->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
          this_00 = &((pCVar2->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
          if (this_00 != (CBaseEdict *)0x0) {
            this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
            pIVar5 = CBaseEdict::GetChangeAccessor(this_00);
            pIVar5->m_iChangeInfoSerialNumber = 0;
          }
        }
        else {
          puVar1 = &(pCVar2->super_CBaseEntity).m_Network.field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (pCVar2->m_hAttachEntity).m_Value[iVar12 + 9].super_CBaseHandle.m_Index = local_2c.m_Index;
      }
    }
    if (local_2f4.m_pEnt != (CBaseEntity *)0x0) {
      bVar4 = CPhysicsShadowClone::IsShadowClone(local_2f4.m_pEnt);
      pCVar8 = local_2f4.m_pEnt;
      if (bVar4) {
        CPhysicsShadowClone::GetClonedEntity((CPhysicsShadowClone *)&local_24);
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
        if (((IHandleEntity)local_24._vptr_IHandleEntity == (IHandleEntity)0xffffffff) ||
           (iVar12 = ((uint)local_24._vptr_IHandleEntity & 0xffff) * 0x10 +
                     **(int **)(unaff_EBX + 0x4f78e3),
           *(uint *)(iVar12 + 8) != (uint)local_24._vptr_IHandleEntity >> 0x10)) {
          pCVar8 = (CBaseEntity *)0x0;
        }
        else {
          pCVar8 = *(CBaseEntity **)(iVar12 + 4);
        }
      }
      bVar4 = UTIL_IsReflectiveCube(pCVar8);
      if (bVar4) {
        if ((pCVar8->m_iEFlags & 0x800) != 0) {
          CBaseEntity::CalcAbsolutePosition(pCVar8);
        }
        AngleVectors(&pCVar8->m_angAbsRotation,&local_134);
                    /* Unresolved local var: Vector res@[???] */
        local_e4 = *(float *)(unaff_EBX + 0x37cfa7);
        local_ec = local_134.x * local_e4;
        local_e8 = local_134.y * local_e4;
        local_e4 = local_e4 * local_134.z;
        pfVar6 = (float *)(*(pCVar8->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                            _vptr_IHandleEntity[0x9d])();
                    /* Unresolved local var: Vector res@[???] */
        local_11c.x = *pfVar6 + local_ec;
        local_11c.y = pfVar6[1] + local_e8;
        local_11c.z = pfVar6[2] + local_e4;
        local_f8 = local_11c.x;
        local_f4 = local_11c.y;
        local_f0 = local_11c.z;
                    /* Unresolved local var: CPortalSimulator * pSimulator@[???] */
        pCVar7 = CPortalSimulator::GetSimulatorThatOwnsEntity(pCVar8);
        if (((pCVar7 != (CPortalSimulator *)0x0) &&
            (bVar4 = CPortalSimulator::EntityIsInPortalHole(pCVar7,pCVar8), bVar4)) &&
           (((local_11c.x *
              (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.PortalPlane.m_Normal.x +
              local_11c.y *
              (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.PortalPlane.m_Normal.y +
             local_11c.z *
             (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.PortalPlane.m_Normal.z) -
             (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.PortalPlane.m_Dist < 0.0 &&
            (pfVar6 = (float *)(*(pCVar8->super_IServerEntity).super_IServerUnknown.
                                 super_IHandleEntity._vptr_IHandleEntity[0x9d])(),
            0.0 < (*pfVar6 * (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.PortalPlane.
                             m_Normal.x +
                   pfVar6[1] *
                   (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.PortalPlane.m_Normal.y
                  + pfVar6[2] *
                    (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.PortalPlane.m_Normal.
                    z) - (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.PortalPlane.
                         m_Dist)))) {
                    /* Unresolved local var: Vector vRet@[???] */
          local_104 = (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                      [0][0] * local_11c.x +
                      (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                      [0][1] * local_11c.y +
                      (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                      [0][2] * local_11c.z +
                      (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                      [0][3];
          local_100 = (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                      [1][0] * local_11c.x +
                      (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                      [1][1] * local_11c.y +
                      (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                      [1][2] * local_11c.z +
                      (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                      [1][3];
          local_11c.z = (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                        [2][0] * local_11c.x +
                        (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                        [2][1] * local_11c.y +
                        (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                        [2][2] * local_11c.z +
                        (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                        [2][3];
          local_18c = local_134.x *
                      (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                      [2][0] + local_134.y *
                               (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.
                               matThisToLinked.m[2][1] +
                      local_134.z *
                      (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                      [2][2];
          local_190 = local_134.x *
                      (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                      [1][0] + local_134.y *
                               (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.
                               matThisToLinked.m[1][1] +
                      local_134.z *
                      (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                      [1][2];
          local_194 = local_134.x *
                      (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                      [0][0] + local_134.y *
                               (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.
                               matThisToLinked.m[0][1] +
                      local_134.z *
                      (pCVar7->m_InternalData).super_PS_InternalData_t.Placement.matThisToLinked.m
                      [0][2];
          local_188 = local_194;
          local_184 = local_190;
          local_180 = local_18c;
          local_134.x = local_194;
          local_134.y = local_190;
          local_134.z = local_18c;
          local_11c.x = local_104;
          local_11c.y = local_100;
          local_110 = local_194;
          local_10c = local_190;
          local_108 = local_18c;
          local_fc = local_11c.z;
        }
        this->m_bHasHitCube = true;
        FireLaser(this,nNumBeams + 1,&local_11c,&local_134,pCVar8);
        goto LAB_006b4a35;
      }
    }
    iVar12 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4f798f) + 0x18))();
    if ((iVar12 == 0) || (*(short *)(iVar12 + 0x4c) != 0x52)) {
LAB_006b52a6:
      UTIL_SetOrigin((CBaseEntity *)this->m_pPlacementHelper,pVVar10,false);
      bVar4 = false;
    }
    else {
                    /* Unresolved local var: Vector vecDir@[???]
                       Unresolved local var: Vector vecBounce@[???]
                       Unresolved local var: Vector res@[???] */
      local_128 = local_2f4.super_CBaseTrace.endpos.x - local_2f4.super_CBaseTrace.startpos.x;
      local_124 = local_2f4.super_CBaseTrace.endpos.y - local_2f4.super_CBaseTrace.startpos.y;
      local_120 = local_2f4.super_CBaseTrace.endpos.z - local_2f4.super_CBaseTrace.startpos.z;
      local_11c.x = local_128;
      local_11c.y = local_124;
      local_11c.z = local_120;
      VectorNormalize(&local_11c);
      fVar21 = local_2f4.super_CBaseTrace.plane.normal.x * local_11c.x +
               local_11c.y * local_2f4.super_CBaseTrace.plane.normal.y +
               local_11c.z * local_2f4.super_CBaseTrace.plane.normal.z;
                    /* Unresolved local var: Vector res@[???] */
      local_150 = *(float *)(unaff_EBX + 0x3766cb);
      local_158 = local_2f4.super_CBaseTrace.plane.normal.x * local_150;
      local_154 = local_2f4.super_CBaseTrace.plane.normal.y * local_150;
      local_150 = local_150 * local_2f4.super_CBaseTrace.plane.normal.z;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
      local_140 = fVar21 * local_158 + local_11c.x;
      local_13c = fVar21 * local_154 + local_11c.y;
      local_138 = fVar21 * local_150 + local_11c.z;
      local_134.x = local_140;
      local_134.y = local_13c;
      local_134.z = local_138;
      VectorNormalize(&local_134);
      dVar20 = (double)(local_134.x * local_11c.x + local_134.y * local_11c.y +
                       local_134.z * local_11c.z);
      if (dVar20 < *(double *)(unaff_EBX + 0x37ecef) || dVar20 == *(double *)(unaff_EBX + 0x37ecef))
      goto LAB_006b52a6;
      FireLaser(this,nNumBeams + 1,pVVar10,&local_134,(CBaseEntity *)0x0);
      bVar4 = true;
    }
    BeamDamage(this,trace);
    if (!bVar4) {
      (**(code **)(*(int *)**(undefined4 **)(CNPC_FloorTurret::InputRestoreAmmo + unaff_EBX + 3) +
                  0x10))();
    }
    local_2a0.super_CBaseTrace.startpos.x = (vec_t)(*(int *)(&DAT_004f7a07 + unaff_EBX) + 8);
    local_2a0.super_CBaseTrace.plane.normal.y = 0.0;
    if ((int)local_2a0.super_CBaseTrace.plane.normal.x < 0) goto LAB_006b51dd;
    if (local_2a0.super_CBaseTrace.endpos.y != 0.0) {
      (**(code **)(*(int *)**(undefined4 **)
                             (CTurretTipController::~CTurretTipController + unaff_EBX + 7) + 8))();
      goto LAB_006b4a78;
    }
  }
  local_2a0.super_CBaseTrace.endpos.z = 0.0;
LAB_006b4a8a:
  if ((-1 < (int)local_2a0.super_CBaseTrace.plane.normal.x) &&
     (local_2a0.super_CBaseTrace.endpos.y != 0.0)) {
    local_2a0.super_CBaseTrace.plane.normal.z = local_2a0.super_CBaseTrace.endpos.y;
    (**(code **)(*(int *)**(undefined4 **)
                           (CTurretTipController::~CTurretTipController + unaff_EBX + 7) + 8))();
  }
  return;
}


/* CPortalLaser::HideChildBeams at 006aff10 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void HideChildBeams(CPortalLaser * this) */

void __thiscall CPortalLaser::HideChildBeams(CPortalLaser *this)

{
  if (this->m_pChildBeams[0] != (CBeam *)0x0) {
    CBeam::TurnOn(this->m_pChildBeams[0]);
  }
  if (this->m_pChildBeams[1] != (CBeam *)0x0) {
    CBeam::TurnOn(this->m_pChildBeams[1]);
  }
  if (this->m_pChildBeams[2] != (CBeam *)0x0) {
    CBeam::TurnOn(this->m_pChildBeams[2]);
  }
  if (this->m_pChildBeams[3] != (CBeam *)0x0) {
    CBeam::TurnOn(this->m_pChildBeams[3]);
  }
  if (this->m_pChildBeams[4] != (CBeam *)0x0) {
    CBeam::TurnOn(this->m_pChildBeams[4]);
  }
  if (this->m_pChildBeams[5] != (CBeam *)0x0) {
    CBeam::TurnOn(this->m_pChildBeams[5]);
  }
  if (this->m_pChildBeams[6] != (CBeam *)0x0) {
    CBeam::TurnOn(this->m_pChildBeams[6]);
  }
  if (this->m_pChildBeams[7] != (CBeam *)0x0) {
    CBeam::TurnOn(this->m_pChildBeams[7]);
    return;
  }
  return;
}


/* CPortalLaser::StrikeThink at 006b5c60 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void StrikeThink(CPortalLaser * this) */

void __thiscall CPortalLaser::StrikeThink(CPortalLaser *this)

{
  uint *puVar1;
  vec_t *pvVar2;
  int iVar3;
  char cVar4;
  int iVar5;
  int *piVar6;
  int unaff_EBX;
  Vector local_34;
  Vector local_28 [2];
  
                    /* Unresolved local var: CVProfScope VProf_@[???]
                       Unresolved local var: Vector vecDir@[???]
                       Unresolved local var: Vector vecOrigin@[???] */
  ___i686_get_pc_thunk_bx();
  if ((*(int *)(*(int *)(&DAT_004f5f4e + unaff_EBX) + 0x100c) != 0) ||
     (*(char *)(*(int *)(&DAT_004f5f4e + unaff_EBX) + 0x1010) == '\0')) {
    iVar3 = *(int *)(*(int *)(&DAT_004f5f4e + unaff_EBX) + 0x19b8);
    iVar5 = _ThreadGetCurrentId();
    if (iVar3 == iVar5) {
      piVar6 = *(int **)(*(int *)(&DAT_004f5f4e + unaff_EBX) + 0x1014);
      if (*piVar6 != unaff_EBX + 0x358c30) {
        piVar6 = (int *)CVProfNode::GetSubNode
                                  ((char *)piVar6,unaff_EBX + 0x358c30,(char *)0x0,
                                   unaff_EBX + 0x358c4a);
        *(int **)(*(int *)(&DAT_004f5f4e + unaff_EBX) + 0x1014) = piVar6;
      }
      puVar1 = (uint *)(piVar6[0x1c] * 8 + *(int *)(*(int *)(&DAT_004f5f4e + unaff_EBX) + 0x10a0) +
                       4);
      *puVar1 = *puVar1 | 4;
      CVProfNode::EnterScope();
      *(undefined1 *)(*(int *)(&DAT_004f5f4e + unaff_EBX) + 0x1010) = 0;
    }
  }
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(**(int **)(unaff_EBX + 0x4f5f32) + 0xc) +
             *(float *)(*(int *)(unaff_EBX + 0x678aee) + 0x2c),(char *)0x0);
  CreateHelperEntities(this);
                    /* Unresolved local var: int i@[???] */
  if (this->m_pChildBeams[0] != (CBeam *)0x0) {
    CBeam::TurnOn(this->m_pChildBeams[0]);
  }
  if (this->m_pChildBeams[1] != (CBeam *)0x0) {
    CBeam::TurnOn(this->m_pChildBeams[1]);
  }
  if (this->m_pChildBeams[2] != (CBeam *)0x0) {
    CBeam::TurnOn(this->m_pChildBeams[2]);
  }
  if (this->m_pChildBeams[3] != (CBeam *)0x0) {
    CBeam::TurnOn(this->m_pChildBeams[3]);
  }
  if (this->m_pChildBeams[4] != (CBeam *)0x0) {
    CBeam::TurnOn(this->m_pChildBeams[4]);
  }
  if (this->m_pChildBeams[5] != (CBeam *)0x0) {
    CBeam::TurnOn(this->m_pChildBeams[5]);
  }
  if (this->m_pChildBeams[6] != (CBeam *)0x0) {
    CBeam::TurnOn(this->m_pChildBeams[6]);
  }
  if (this->m_pChildBeams[7] != (CBeam *)0x0) {
    CBeam::TurnOn(this->m_pChildBeams[7]);
  }
  pvVar2 = *(vec_t **)(unaff_EBX + 0x4f6282);
  (this->m_vecNearestSoundSource).x = *pvVar2;
  (this->m_vecNearestSoundSource).y = pvVar2[1];
  (this->m_vecNearestSoundSource).z = pvVar2[2];
  CBaseAnimating::GetAttachment
            (&this->super_CBaseAnimating,this->m_iLaserAttachment,&local_34,local_28,(Vector *)0x0,
             (Vector *)0x0);
  this->m_bHasHitCube = false;
  FireLaser(this,0,&local_34,local_28,(CBaseEntity *)0x0);
  UTIL_SetOrigin(this->m_pSoundProxy,&this->m_vecNearestSoundSource,false);
  if ((*(char *)(*(int *)(&DAT_004f5f4e + unaff_EBX) + 0x1010) == '\0') ||
     (*(int *)(*(int *)(&DAT_004f5f4e + unaff_EBX) + 0x100c) != 0)) {
    iVar3 = *(int *)(*(int *)(&DAT_004f5f4e + unaff_EBX) + 0x19b8);
    iVar5 = _ThreadGetCurrentId();
    if (iVar3 == iVar5) {
      cVar4 = CVProfNode::ExitScope();
      if (cVar4 != '\0') {
        *(undefined4 *)(*(int *)(&DAT_004f5f4e + unaff_EBX) + 0x1014) =
             *(undefined4 *)(*(int *)(*(int *)(&DAT_004f5f4e + unaff_EBX) + 0x1014) + 100);
      }
      iVar3 = *(int *)(&DAT_004f5f4e + unaff_EBX);
      *(bool *)(iVar3 + 0x1010) = *(int *)(iVar3 + 0x1014) == iVar3 + 0x1018;
      return;
    }
  }
  return;
}


/* CPortalLaser::UpdateSoundPosition at 006b0b30 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void UpdateSoundPosition(CPortalLaser * this, Vector * vecStart, Vector
   * vecEnd) */

void __thiscall
CPortalLaser::UpdateSoundPosition(CPortalLaser *this,Vector *vecStart,Vector *vecEnd)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float *pfVar4;
  char cVar5;
  CBasePlayer *pCVar6;
  int unaff_EBX;
  Vector local_30;
  float local_24;
  float local_20;
  float local_1c;
  Vector local_18;
  
                    /* Unresolved local var: CBasePlayer * pPlayer@[???] */
  ___i686_get_pc_thunk_bx();
  cVar5 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4fb09f) + 0x88))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x4fb09f));
  if ((cVar5 == '\0') && (pCVar6 = UTIL_GetLocalPlayer(), pCVar6 != (CBasePlayer *)0x0)) {
                    /* Unresolved local var: Vector vecPlayer@[???]
                       Unresolved local var: Vector vecNearestPoint@[???]
                       Unresolved local var: float flDistToPlayerSqr@[???]
                       Unresolved local var: float flNearestToPlayerSqr@[???] */
    (**(code **)((int)(pCVar6->super_CBaseCombatCharacter).super_CBaseFlex.
                      super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                      super_IServerEntity.super_IServerUnknown + 0x220))(&local_24,pCVar6);
    local_18.x = local_24;
    local_18.y = local_20;
    local_18.z = local_1c;
    CalcClosestPointOnLineSegment(&local_18,vecStart,vecEnd,&local_30,(float *)0x0);
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
    fVar1 = (this->m_vecNearestSoundSource).x;
    fVar2 = (this->m_vecNearestSoundSource).y;
    fVar3 = (this->m_vecNearestSoundSource).z;
    if ((local_18.x - local_30.x) * (local_18.x - local_30.x) +
        (local_18.y - local_30.y) * (local_18.y - local_30.y) +
        (local_18.z - local_30.z) * (local_18.z - local_30.z) <
        (local_18.x - fVar1) * (local_18.x - fVar1) + (local_18.y - fVar2) * (local_18.y - fVar2) +
        (local_18.z - fVar3) * (local_18.z - fVar3)) {
LAB_006b0cb0:
      (this->m_vecNearestSoundSource).x = local_30.x;
      (this->m_vecNearestSoundSource).y = local_30.y;
      (this->m_vecNearestSoundSource).z = local_30.z;
      return;
    }
    pfVar4 = *(float **)(&DAT_004fb3b3 + unaff_EBX);
    if ((fVar1 == *pfVar4) && (!NAN(fVar1) && !NAN(*pfVar4))) {
      if ((fVar2 == pfVar4[1]) && (!NAN(fVar2) && !NAN(pfVar4[1]))) {
        if ((fVar3 == pfVar4[2]) && (!NAN(fVar3) && !NAN(pfVar4[2]))) goto LAB_006b0cb0;
      }
    }
  }
  else {
    if (((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
    (this->m_vecNearestSoundSource).x =
         (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.x;
    (this->m_vecNearestSoundSource).y =
         (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.y;
    (this->m_vecNearestSoundSource).z =
         (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.z;
  }
  return;
}


/* CPortalLaser::DamageEntity at 006afe60 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void DamageEntity(CPortalLaser * this, CBaseEntity * pVictim, float
   flAmount) */

void __thiscall CPortalLaser::DamageEntity(CPortalLaser *this,CBaseEntity *pVictim,float flAmount)

{
  Vector *vecForceOrigin;
  int unaff_EBX;
  CTakeDamageInfo *in_stack_ffffff7c;
  CBaseEntity *in_stack_ffffff80;
  CBaseEntity *in_stack_ffffff84;
  float in_stack_ffffff88;
  vec_t in_stack_ffffff8c;
  vec_t in_stack_ffffff90;
  Vector local_28 [2];
  
                    /* Unresolved local var: CTakeDamageInfo info@[???] */
  ___i686_get_pc_thunk_bx();
  CTakeDamageInfo::CTakeDamageInfo
            ((CTakeDamageInfo *)&stack0xffffff88,(CBaseEntity *)this,(CBaseEntity *)this,
             flAmount * *(float *)(**(int **)(&DAT_004fbd29 + unaff_EBX) + 0x10),8,0,
             in_stack_ffffff7c,in_stack_ffffff80,in_stack_ffffff84,in_stack_ffffff88,
             (int)in_stack_ffffff8c,(int)in_stack_ffffff90);
  vecForceOrigin =
       (Vector *)
       (*(pVictim->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
         [0x9d])(pVictim);
  local_28[0].x = 1.0;
  local_28[0].y = 0.0;
  local_28[0].z = 0.0;
  CalculateMeleeDamageForce((CTakeDamageInfo *)&stack0xffffff88,local_28,vecForceOrigin,1.0);
  CBaseEntity::TakeDamage(pVictim,(CTakeDamageInfo *)&stack0xffffff88);
  return;
}


/* CPortalLaser::StrikeEntitiesAlongLaser at 006b19c0 */

/* WARNING: Restarted to delay deadcode elimination for space: stack */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: bool StrikeEntitiesAlongLaser(CPortalLaser * this, Vector * vecStart,
   Vector * vecEnd, Vector * pVecOut) */

bool __thiscall
CPortalLaser::StrikeEntitiesAlongLaser
          (CPortalLaser *this,Vector *vecStart,Vector *vecEnd,Vector *pVecOut)

{
  char *pszClassOrWildcard;
  char *pszClassOrWildcard_00;
  float *pfVar1;
  CBaseEntity *pCVar2;
  bool bVar3;
  bool bVar4;
  int iVar5;
  int iVar6;
  CBaseEntity *pCVar7;
  Vector *pVVar8;
  int *piVar9;
  vec_t *pvVar10;
  int unaff_EBX;
  int iVar11;
  float fVar12;
  float fVar13;
  CBaseEntity *pAttacker_1;
  CTakeDamageInfo *in_stack_fffff59c;
  CBaseEntity *in_stack_fffff5a0;
  float in_stack_fffff5a8;
  int in_stack_fffff5ac;
  int in_stack_fffff5b0;
  CPortal_Player *local_a14;
  int local_a10;
  CBaseEntity *local_a0c [512];
  CTakeDamageInfo local_20c;
  CTakeDamageInfo local_1bc;
  Ray_t local_16c;
  CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess> local_118;
  __m128 root;
  Vector local_e0;
  Vector local_d4;
  float local_c8;
  float local_c4;
  undefined4 local_c0;
  float local_bc;
  float local_b8;
  float local_b4;
  float local_b0;
  float local_ac;
  float local_a8;
  float local_a4;
  float local_a0;
  float local_9c;
  float local_98;
  float local_94;
  float local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  Vector local_80;
  float local_74;
  float local_70;
  float local_6c;
  Vector local_68;
  float local_5c;
  float local_58;
  float local_54;
  Vector local_50;
  Vector local_44;
  float local_38;
  float local_34;
  float local_30;
  Vector local_2c;
  float local_20 [4];
  
                    /* Unresolved local var: Vector vecDirection@[???]
                       Unresolved local var: float flLaserWidth@[???]
                       Unresolved local var: Vector vecMins@[???]
                       Unresolved local var: Vector vecMaxs@[???]
                       Unresolved local var: CBaseEntity *[512] list@[???]
                       Unresolved local var: Ray_t ray@[???]
                       Unresolved local var: int nCount@[???]
                       Unresolved local var: CBaseEntity * pEntity@[???]
                       Unresolved local var: CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>
                       vsrtVictims@[???]
                       Unresolved local var: bool bBestIsTurret@[???] */
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: Vector res@[???] */
  local_38 = vecEnd->x - vecStart->x;
  local_34 = vecEnd->y - vecStart->y;
  local_30 = vecEnd->z - vecStart->z;
  root[0] = SQRT(local_38 * local_38 + local_34 * local_34 + local_30 * local_30);
  root[1] = 0.0;
  root[2] = 0.0;
  root[3] = 0.0;
  fVar12 = *(float *)(unaff_EBX + 0x37d5d7) * root[0];
  pAttacker_1 = (CBaseEntity *)(fVar12 + *(float *)(unaff_EBX + 0x378ba3));
  if (((float)pAttacker_1 < *(float *)(unaff_EBX + 0x379d77) ||
       (float)pAttacker_1 == *(float *)(unaff_EBX + 0x379d77)) &&
     (pAttacker_1 = (CBaseEntity *)((float)pAttacker_1 + fVar12),
     *(float *)(unaff_EBX + 0x378ba3) <= (float)pAttacker_1)) {
    pAttacker_1 = (CBaseEntity *)((float)pAttacker_1 + fVar12);
  }
  local_2c.x = local_38;
  local_2c.y = local_34;
  local_2c.z = local_30;
  VectorNormalize(&local_2c);
  fVar12 = (float)((uint)pAttacker_1 ^ *(uint *)(unaff_EBX + 0x3815cf));
  local_16c.m_Delta.super_Vector.x = vecEnd->x - vecStart->x;
  local_16c.m_Delta.super_Vector.y = vecEnd->y - vecStart->y;
  local_16c.m_Delta.super_Vector.z = vecEnd->z - vecStart->z;
  local_16c.m_pWorldAxisTransform = (matrix3x4_t *)0x0;
  local_16c.m_IsSwept =
       local_16c.m_Delta.super_Vector.x * local_16c.m_Delta.super_Vector.x +
       local_16c.m_Delta.super_Vector.y * local_16c.m_Delta.super_Vector.y +
       local_16c.m_Delta.super_Vector.z * local_16c.m_Delta.super_Vector.z != 0.0;
  local_16c.m_Extents.super_Vector.x =
       ((float)pAttacker_1 - fVar12) * *(float *)(unaff_EBX + 0x378b2f);
  local_16c.m_IsRay =
       (double)(local_16c.m_Extents.super_Vector.x * local_16c.m_Extents.super_Vector.x +
                local_16c.m_Extents.super_Vector.x * local_16c.m_Extents.super_Vector.x +
               local_16c.m_Extents.super_Vector.x * local_16c.m_Extents.super_Vector.x) <
       *(double *)(unaff_EBX + 0x378d1f);
  local_16c.m_StartOffset.super_Vector.z =
       ((float)pAttacker_1 + fVar12) * *(float *)(unaff_EBX + 0x378b2f);
  local_16c.m_Start.super_Vector.x = vecStart->x + local_16c.m_StartOffset.super_Vector.z;
  local_16c.m_Start.super_Vector.y = vecStart->y + local_16c.m_StartOffset.super_Vector.z;
  local_16c.m_Start.super_Vector.z = vecStart->z + local_16c.m_StartOffset.super_Vector.z;
  local_16c.m_StartOffset.super_Vector.x = -local_16c.m_StartOffset.super_Vector.z;
  local_16c.m_StartOffset.super_Vector.y = -local_16c.m_StartOffset.super_Vector.z;
  local_16c.m_StartOffset.super_Vector.z = -local_16c.m_StartOffset.super_Vector.z;
                    /* Unresolved local var: CFlaggedEntitiesEnum rayEnum@[???] */
  local_16c.m_Extents.super_Vector.y = local_16c.m_Extents.super_Vector.x;
  local_16c.m_Extents.super_Vector.z = local_16c.m_Extents.super_Vector.x;
  CFlaggedEntitiesEnum::CFlaggedEntitiesEnum
            ((CFlaggedEntitiesEnum *)&local_1bc,local_a0c,0x200,0x2002080);
  iVar5 = UTIL_EntitiesAlongRay(&local_16c,(CFlaggedEntitiesEnum *)&local_1bc);
  local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_Memory.
  m_pMemory = (__376_conflict *)0x0;
  local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_Memory.
  m_nAllocationCount = 0;
  local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_Memory.
  m_nGrowSize = 0;
  local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_Size = 0;
  local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_pElements =
       (__376_conflict *)0x0;
  local_118.m_pLessContext = (void *)0x0;
  local_118.m_bNeedsSort = false;
  if (0 < iVar5) {
    iVar11 = 0;
    pszClassOrWildcard = (char *)(unaff_EBX + 0x35cdff);
    pszClassOrWildcard_00 = &UNK_0030c3d7 + unaff_EBX;
    do {
      while ((((pCVar2 = local_a0c[iVar11], pCVar2 == (CBaseEntity *)0x0 ||
               ((((root[0] = (float)pszClassOrWildcard,
                  (pCVar2->m_iClassname).pszValue != pszClassOrWildcard &&
                  (bVar3 = CBaseEntity::ClassMatchesComplex(pCVar2,pszClassOrWildcard), !bVar3)) &&
                 (root[0] = (float)pszClassOrWildcard_00,
                 (pCVar2->m_iClassname).pszValue != pszClassOrWildcard_00)) &&
                (bVar3 = CBaseEntity::ClassMatchesComplex(pCVar2,pszClassOrWildcard_00), !bVar3))))
              && (iVar6 = (*(pCVar2->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                            _vptr_IHandleEntity[0x55])(pCVar2), (char)iVar6 == '\0')) ||
             (iVar6 = (*(pCVar2->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                        _vptr_IHandleEntity[0x45])(pCVar2), (char)iVar6 == '\0'))) {
LAB_006b1cf0:
        iVar11 = iVar11 + 1;
        if (iVar11 == iVar5) goto LAB_006b1ee1;
      }
      if ((pCVar2->m_iEFlags & 0x800) != 0) {
        CBaseEntity::CalcAbsolutePosition(pCVar2);
      }
      CalcClosestPointOnLineSegment(&pCVar2->m_vecAbsOrigin,vecStart,vecEnd,&local_44,local_20);
      if ((pCVar2->m_iEFlags & 0x800) != 0) {
        CBaseEntity::CalcAbsolutePosition(pCVar2);
      }
                    /* Unresolved local var: Vector res@[???] */
      fVar12 = local_44.x - (pCVar2->m_vecAbsOrigin).x;
      fVar13 = local_44.y - (pCVar2->m_vecAbsOrigin).y;
      fVar12 = fVar13 * fVar13 + fVar12 * fVar12;
      if ((*(float *)(unaff_EBX + 0x379dab) <= fVar12 && fVar12 != *(float *)(unaff_EBX + 0x379dab))
         && (this->m_bHasHitCube == false)) goto LAB_006b1cf0;
      iVar6 = (*(pCVar2->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                _vptr_IHandleEntity[0x55])(pCVar2);
      if ((char)iVar6 == '\0') {
LAB_006b1cc0:
        if (0.0 < local_20[0]) {
                    /* Unresolved local var: LaserVictimInfo_t victim@[???] */
          local_50.y = local_20[0];
          local_50.x = (vec_t)pCVar2;
          CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::InsertNoSort
                    (&local_118,(__376_conflict *)&local_50);
        }
        goto LAB_006b1cf0;
      }
                    /* Unresolved local var: float flZDist@[???] */
      if ((*(int *)(*(int *)(unaff_EBX + 0x67cccb) + 0x30) == 0) ||
         ((pCVar7 = CBaseEntity::GetGroundEntity(pCVar2), pCVar7 == (CBaseEntity *)0x0 &&
          (iVar6 = (*(pCVar2->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                     _vptr_IHandleEntity[0x9d])(pCVar2), 0.0 <= *(float *)(iVar6 + 8) - local_44.z))
         )) goto LAB_006b1cf0;
      iVar6 = (*(pCVar2->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                _vptr_IHandleEntity[0x55])(pCVar2);
      if ((char)iVar6 == '\0') {
        local_a14 = (CPortal_Player *)0x0;
      }
      else {
        local_a14 = (CPortal_Player *)
                    ___dynamic_cast(pCVar2,*(undefined4 *)(unaff_EBX + 0x4fa1f7),
                                    *(undefined4 *)(unaff_EBX + 0x4fa3f7),0);
      }
      if (((local_a14->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
           super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
           super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
           m_iEFlags & 0x800) != 0) {
        CBaseEntity::CalcAbsolutePosition((CBaseEntity *)local_a14);
      }
      fVar13 = (local_a14->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
               super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
               super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
               super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
               m_vecAbsOrigin.z;
      pVVar8 = CPortal_Player::GetHullMaxs(local_a14);
      if (fVar13 + pVVar8->z < local_44.z) goto LAB_006b1cf0;
      if (fVar12 < *(float *)(unaff_EBX + 0x379dab) || fVar12 == *(float *)(unaff_EBX + 0x379dab))
      goto LAB_006b1cc0;
      iVar11 = iVar11 + 1;
    } while (iVar11 != iVar5);
  }
LAB_006b1ee1:
  local_118.m_bNeedsSort = false;
  if (1 < local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_Size)
  {
    local_44.x = (vec_t)local_118.m_pLessContext;
    local_44.y = (vec_t)&local_50;
    **(int **)(unaff_EBX + 0x4fa59f) = (int)&local_44;
    _qsort(local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.
           m_Memory.m_pMemory,
           local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_Size,
           8,*(int **)(unaff_EBX + 0x61790f));
  }
  if (0 < local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_Size)
  {
    bVar3 = false;
    local_a10 = 0;
    do {
      pCVar2 = local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.
               m_Memory.m_pMemory[local_a10].pVictim;
      if (pCVar2 == (CBaseEntity *)0x0) {
LAB_006b1f74:
        iVar5 = (*(pCVar2->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                  _vptr_IHandleEntity[0x55])(pCVar2);
        if (((char)iVar5 != '\0') && ((pCVar2->m_MoveType).m_Value != '\b')) {
          if ((pCVar2->m_iEFlags & 0x1000) != 0) {
            CBaseEntity::CalcAbsoluteVelocity(pCVar2);
          }
          local_44.x = (pCVar2->m_vecAbsVelocity).x;
          local_44.y = (pCVar2->m_vecAbsVelocity).y;
          local_44.z = (pCVar2->m_vecAbsVelocity).z;
          VectorNormalize(&local_44);
          if ((pCVar2->m_iEFlags & 0x800) != 0) {
            CBaseEntity::CalcAbsolutePosition(pCVar2);
          }
          CalcClosestPointOnLineSegment
                    (&pCVar2->m_vecAbsOrigin,vecStart,vecEnd,&local_50,(float *)0x0);
          if ((pCVar2->m_iEFlags & 0x800) != 0) {
            CBaseEntity::CalcAbsolutePosition(pCVar2);
          }
          local_5c = (pCVar2->m_vecAbsOrigin).x;
          local_58 = (pCVar2->m_vecAbsOrigin).y;
          local_54 = local_50.z;
                    /* Unresolved local var: Vector res@[???] */
          local_74 = local_5c - local_50.x;
          local_70 = local_58 - local_50.y;
          local_6c = local_50.z - local_50.z;
          local_68.x = local_74;
          local_68.y = local_70;
          local_68.z = local_6c;
          VectorNormalize(&local_68);
          local_68.z = 0.0;
          if (*(float *)(unaff_EBX + 0x37da47) <=
              local_44.x * local_44.x + local_44.y * local_44.y + local_44.z * local_44.z) {
                    /* Unresolved local var: float flDot@[???] */
            fVar12 = local_44.x * local_68.x + local_44.y * local_68.y + local_44.z * 0.0;
                    /* Unresolved local var: Vector res@[???] */
            local_c8 = local_68.x * *(float *)(unaff_EBX + 0x378fbb);
            local_c4 = *(float *)(unaff_EBX + 0x378fbb) * local_68.y;
            local_c0 = 0x80000000;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
            local_a4 = fVar12 * local_c8 + local_44.x;
            local_a0 = fVar12 * local_c4 + local_44.y;
            local_9c = fVar12 * -0.0 + local_44.z;
            local_80.x = local_a4;
            local_80.y = local_a0;
            local_80.z = local_9c;
            VectorNormalize(&local_80);
            local_80.z = 0.0;
            if (local_80.x * local_68.x + local_80.y * local_68.y + local_68.z * 0.0 < 0.0)
            goto LAB_006b2310;
          }
          else {
            local_8c = 0;
            local_88 = 0;
            local_84 = 0x3f800000;
            local_bc = local_2c.y - local_2c.z * 0.0;
            local_b8 = local_2c.z * 0.0 - local_2c.x;
            local_b4 = local_2c.x * 0.0 - local_2c.y * 0.0;
            local_98 = local_bc;
            local_94 = local_b8;
            local_90 = local_b4;
            local_80.x = local_bc;
            local_80.y = local_b8;
            local_80.z = local_b4;
          }
                    /* Unresolved local var: Vector res@[???] */
          pfVar1 = (float *)(unaff_EBX + 0x378c47);
          local_1bc.m_vecDamageForce.x = local_80.x * *pfVar1;
          local_1bc.m_vecDamageForce.y = local_80.y * *pfVar1;
          local_1bc.m_vecDamageForce.z = *pfVar1 * local_80.z;
          local_b0 = local_1bc.m_vecDamageForce.x;
          local_ac = local_1bc.m_vecDamageForce.y;
          local_a8 = local_1bc.m_vecDamageForce.z;
          if (((pCVar2->m_fFlags).m_Value & 1) != 0) {
            CBaseEntity::SetGroundEntity(pCVar2,(CBaseEntity *)0x0);
            CBaseEntity::SetGroundChangeTime
                      (pCVar2,*(float *)(unaff_EBX + 0x378b2f) +
                              *(float *)(**(int **)(unaff_EBX + 0x4fa1cf) + 0xc));
            local_1bc.m_vecDamageForce.x =
                 local_1bc.m_vecDamageForce.x + local_1bc.m_vecDamageForce.x;
            local_1bc.m_vecDamageForce.y =
                 local_1bc.m_vecDamageForce.y + local_1bc.m_vecDamageForce.y;
            local_1bc.m_vecDamageForce.z =
                 local_1bc.m_vecDamageForce.z + local_1bc.m_vecDamageForce.z;
          }
          CBaseEntity::SetAbsVelocity(pCVar2,&local_1bc.m_vecDamageForce);
          if (this->m_bIsLethal == false) {
            fVar12 = *(float *)(unaff_EBX + 0x378ccf);
          }
          else {
            fVar12 = *(float *)(unaff_EBX + 0x37bd63);
          }
                    /* Unresolved local var: CTakeDamageInfo info@[???] */
          CTakeDamageInfo::CTakeDamageInfo
                    (&local_20c,(CBaseEntity *)this,(CBaseEntity *)this,
                     fVar12 * *(float *)(**(int **)(unaff_EBX + 0x4fa1cf) + 0x10),8,0,
                     in_stack_fffff59c,in_stack_fffff5a0,pAttacker_1,in_stack_fffff5a8,
                     in_stack_fffff5ac,in_stack_fffff5b0);
          pVVar8 = (Vector *)
                   (*(pCVar2->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                     _vptr_IHandleEntity[0x9d])(pCVar2);
          local_e0.x = 1.0;
          local_e0.y = 0.0;
          local_e0.z = 0.0;
          CalculateMeleeDamageForce(&local_20c,&local_e0,pVVar8,1.0);
          CBaseEntity::TakeDamage(pCVar2,&local_20c);
          CBaseEntity::EmitSound(pCVar2,(char *)(unaff_EBX + 0x35cefb),0.0,(float *)0x0);
        }
      }
      else {
        root[0] = (float)(unaff_EBX + 0x35cdff);
        if ((((pCVar2->m_iClassname).pszValue != (char *)root[0]) &&
            (bVar4 = CBaseEntity::ClassMatchesComplex(pCVar2,(char *)root[0]), !bVar4)) || (bVar3))
        {
          root[0] = (float)(&UNK_0030c3d7 + unaff_EBX);
          if (((pCVar2->m_iClassname).pszValue != (char *)root[0]) &&
             (bVar4 = CBaseEntity::ClassMatchesComplex(pCVar2,(char *)root[0]), !bVar4))
          goto LAB_006b1f74;
          bVar3 = true;
        }
        else {
                    /* Unresolved local var: CTakeDamageInfo info@[???] */
          CTakeDamageInfo::CTakeDamageInfo
                    (&local_1bc,(CBaseEntity *)this,(CBaseEntity *)this,
                     *(float *)(**(int **)(unaff_EBX + 0x4fa1cf) + 0x10),8,0,in_stack_fffff59c,
                     in_stack_fffff5a0,pAttacker_1,in_stack_fffff5a8,in_stack_fffff5ac,
                     in_stack_fffff5b0);
          pVVar8 = (Vector *)
                   (*(pCVar2->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                     _vptr_IHandleEntity[0x9d])(pCVar2);
          local_d4.x = 1.0;
          local_d4.y = 0.0;
          local_d4.z = 0.0;
          CalculateMeleeDamageForce(&local_1bc,&local_d4,pVVar8,1.0);
          CBaseEntity::TakeDamage(pCVar2,&local_1bc);
          piVar9 = (int *)___dynamic_cast(pCVar2,*(undefined4 *)(unaff_EBX + 0x4fa1f7),
                                          *(undefined4 *)(unaff_EBX + 0x4fad0f),0);
          if ((piVar9 != (int *)0x0) && (*(char *)((int)piVar9 + 0x3f1) != '\0')) {
            bVar3 = true;
            if (pVecOut != (Vector *)0x0) {
              pvVar10 = (vec_t *)(**(code **)(*piVar9 + 0x274))(piVar9);
              pVecOut->x = *pvVar10;
              pVecOut->y = pvVar10[1];
              pVecOut->z = pvVar10[2];
            }
            goto LAB_006b25bc;
          }
        }
      }
LAB_006b2310:
      local_a10 = local_a10 + 1;
    } while (local_a10 <
             local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.
             m_Size);
  }
  bVar3 = false;
  if (pVecOut != (Vector *)0x0) {
    pVecOut->x = vecEnd->x;
    pVecOut->y = vecEnd->y;
    pVecOut->z = vecEnd->z;
  }
LAB_006b25bc:
  local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_Size = 0;
  if (-1 < local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.
           m_Memory.m_nGrowSize) {
    if (local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_Memory.
        m_pMemory != (__376_conflict *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4fa1c7) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4fa1c7),
                 local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.
                 m_Memory.m_pMemory);
      local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_Memory.
      m_pMemory = (__376_conflict *)0x0;
    }
    local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_Memory.
    m_nAllocationCount = 0;
  }
  local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_pElements =
       local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_Memory.
       m_pMemory;
  if ((-1 < local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.
            m_Memory.m_nGrowSize) &&
     (local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_Memory.
      m_pMemory != (__376_conflict *)0x0)) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4fa1c7) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4fa1c7),
               local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.
               m_Memory.m_pMemory);
  }
  return bVar3;
}


/* CPortalLaser::NewStrikeEntitiesAlongLaser at 006b27a0 */

/* WARNING: Restarted to delay deadcode elimination for space: stack */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: CBaseEntity * NewStrikeEntitiesAlongLaser(CPortalLaser * this, Vector *
   vecStart, Vector * vecEnd, Vector * pVecOut, bool bIsFirstTrace) */

CBaseEntity * __thiscall
CPortalLaser::NewStrikeEntitiesAlongLaser
          (CPortalLaser *this,Vector *vecStart,Vector *vecEnd,Vector *pVecOut,bool bIsFirstTrace)

{
  char *pszClassOrWildcard;
  char *pszClassOrWildcard_00;
  float *pfVar1;
  bool bVar2;
  bool bVar3;
  int iVar4;
  int iVar5;
  CBaseEntity *pCVar6;
  Vector *pVVar7;
  vec_t *pvVar8;
  int unaff_EBX;
  CBaseEntity *pCVar9;
  int iVar10;
  float fVar11;
  float fVar12;
  CBaseEntity *pAttacker_1;
  CTakeDamageInfo *in_stack_fffff59c;
  CBaseEntity *in_stack_fffff5a0;
  float in_stack_fffff5a8;
  int in_stack_fffff5ac;
  int in_stack_fffff5b0;
  CPortal_Player *local_a18;
  int local_a14;
  int local_a10;
  CBaseEntity *local_a0c [512];
  CTakeDamageInfo local_20c;
  CTakeDamageInfo local_1bc;
  Ray_t local_16c;
  CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess> local_118;
  __m128 root;
  Vector local_e0;
  Vector local_d4;
  float local_c8;
  float local_c4;
  undefined4 local_c0;
  float local_bc;
  float local_b8;
  float local_b4;
  float local_b0;
  float local_ac;
  float local_a8;
  float local_a4;
  float local_a0;
  float local_9c;
  float local_98;
  float local_94;
  float local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  Vector local_80;
  float local_74;
  float local_70;
  float local_6c;
  Vector local_68;
  float local_5c;
  float local_58;
  float local_54;
  Vector local_50;
  Vector local_44;
  float local_38;
  float local_34;
  float local_30;
  Vector local_2c;
  float local_20 [4];
  
                    /* Unresolved local var: Vector vecDirection@[???]
                       Unresolved local var: float flLaserWidth@[???]
                       Unresolved local var: Vector vecMins@[???]
                       Unresolved local var: Vector vecMaxs@[???]
                       Unresolved local var: CBaseEntity *[512] list@[???]
                       Unresolved local var: Ray_t ray@[???]
                       Unresolved local var: int nCount@[???]
                       Unresolved local var: CBaseEntity * pEntity@[???]
                       Unresolved local var: CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>
                       vsrtVictims@[???]
                       Unresolved local var: bool bBlockTarget@[???] */
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: Vector res@[???] */
  local_38 = vecEnd->x - vecStart->x;
  local_34 = vecEnd->y - vecStart->y;
  local_30 = vecEnd->z - vecStart->z;
  root[0] = SQRT(local_38 * local_38 + local_34 * local_34 + local_30 * local_30);
  root[1] = 0.0;
  root[2] = 0.0;
  root[3] = 0.0;
  fVar11 = *(float *)(unaff_EBX + 0x37c7f7) * root[0];
  pAttacker_1 = (CBaseEntity *)(fVar11 + *(float *)(unaff_EBX + 0x377dc3));
  if (((float)pAttacker_1 < *(float *)(unaff_EBX + 0x378f97) ||
       (float)pAttacker_1 == *(float *)(unaff_EBX + 0x378f97)) &&
     (pAttacker_1 = (CBaseEntity *)((float)pAttacker_1 + fVar11),
     *(float *)(unaff_EBX + 0x377dc3) <= (float)pAttacker_1)) {
    pAttacker_1 = (CBaseEntity *)((float)pAttacker_1 + fVar11);
  }
  local_2c.x = local_38;
  local_2c.y = local_34;
  local_2c.z = local_30;
  VectorNormalize(&local_2c);
  fVar11 = (float)((uint)pAttacker_1 ^ *(uint *)(unaff_EBX + 0x3807ef));
  local_16c.m_Delta.super_Vector.x = vecEnd->x - vecStart->x;
  local_16c.m_Delta.super_Vector.y = vecEnd->y - vecStart->y;
  local_16c.m_Delta.super_Vector.z = vecEnd->z - vecStart->z;
  local_16c.m_pWorldAxisTransform = (matrix3x4_t *)0x0;
  local_16c.m_IsSwept =
       local_16c.m_Delta.super_Vector.x * local_16c.m_Delta.super_Vector.x +
       local_16c.m_Delta.super_Vector.y * local_16c.m_Delta.super_Vector.y +
       local_16c.m_Delta.super_Vector.z * local_16c.m_Delta.super_Vector.z != 0.0;
  local_16c.m_Extents.super_Vector.x =
       ((float)pAttacker_1 - fVar11) * *(float *)(&DAT_00377d4f + unaff_EBX);
  local_16c.m_IsRay =
       (double)(local_16c.m_Extents.super_Vector.x * local_16c.m_Extents.super_Vector.x +
                local_16c.m_Extents.super_Vector.x * local_16c.m_Extents.super_Vector.x +
               local_16c.m_Extents.super_Vector.x * local_16c.m_Extents.super_Vector.x) <
       *(double *)(&DAT_00377f3f + unaff_EBX);
  local_16c.m_StartOffset.super_Vector.z =
       ((float)pAttacker_1 + fVar11) * *(float *)(&DAT_00377d4f + unaff_EBX);
  local_16c.m_Start.super_Vector.x = vecStart->x + local_16c.m_StartOffset.super_Vector.z;
  local_16c.m_Start.super_Vector.y = vecStart->y + local_16c.m_StartOffset.super_Vector.z;
  local_16c.m_Start.super_Vector.z = vecStart->z + local_16c.m_StartOffset.super_Vector.z;
  local_16c.m_StartOffset.super_Vector.x = -local_16c.m_StartOffset.super_Vector.z;
  local_16c.m_StartOffset.super_Vector.y = -local_16c.m_StartOffset.super_Vector.z;
  local_16c.m_StartOffset.super_Vector.z = -local_16c.m_StartOffset.super_Vector.z;
                    /* Unresolved local var: CFlaggedEntitiesEnum rayEnum@[???] */
  local_16c.m_Extents.super_Vector.y = local_16c.m_Extents.super_Vector.x;
  local_16c.m_Extents.super_Vector.z = local_16c.m_Extents.super_Vector.x;
  CFlaggedEntitiesEnum::CFlaggedEntitiesEnum
            ((CFlaggedEntitiesEnum *)&local_1bc,local_a0c,0x200,0x2002080);
  iVar4 = UTIL_EntitiesAlongRay(&local_16c,(CFlaggedEntitiesEnum *)&local_1bc);
  local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_Memory.
  m_pMemory = (__376_conflict *)0x0;
  local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_Memory.
  m_nAllocationCount = 0;
  local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_Memory.
  m_nGrowSize = 0;
  local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_Size = 0;
  local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_pElements =
       (__376_conflict *)0x0;
  local_118.m_pLessContext = (void *)0x0;
  local_118.m_bNeedsSort = false;
  if (0 < iVar4) {
    iVar10 = 0;
    pszClassOrWildcard = (char *)(unaff_EBX + 0x35c01f);
    pszClassOrWildcard_00 = &UNK_0030b5f7 + unaff_EBX;
    do {
      while ((((pCVar9 = local_a0c[iVar10], pCVar9 == (CBaseEntity *)0x0 ||
               ((((root[0] = (float)pszClassOrWildcard,
                  (pCVar9->m_iClassname).pszValue != pszClassOrWildcard &&
                  (bVar2 = CBaseEntity::ClassMatchesComplex(pCVar9,pszClassOrWildcard), !bVar2)) &&
                 (root[0] = (float)pszClassOrWildcard_00,
                 (pCVar9->m_iClassname).pszValue != pszClassOrWildcard_00)) &&
                (bVar2 = CBaseEntity::ClassMatchesComplex(pCVar9,pszClassOrWildcard_00), !bVar2))))
              && (iVar5 = (*(pCVar9->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                            _vptr_IHandleEntity[0x55])(pCVar9), (char)iVar5 == '\0')) ||
             (iVar5 = (*(pCVar9->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                        _vptr_IHandleEntity[0x45])(pCVar9), (char)iVar5 == '\0'))) {
LAB_006b2ae0:
        iVar10 = iVar10 + 1;
        if (iVar10 == iVar4) goto LAB_006b2cfb;
      }
      if ((pCVar9->m_iEFlags & 0x800) != 0) {
        CBaseEntity::CalcAbsolutePosition(pCVar9);
      }
      CalcClosestPointOnLineSegment(&pCVar9->m_vecAbsOrigin,vecStart,vecEnd,&local_44,local_20);
      if ((pCVar9->m_iEFlags & 0x800) != 0) {
        CBaseEntity::CalcAbsolutePosition(pCVar9);
      }
                    /* Unresolved local var: Vector res@[???] */
      fVar11 = local_44.x - (pCVar9->m_vecAbsOrigin).x;
      fVar12 = local_44.y - (pCVar9->m_vecAbsOrigin).y;
      fVar11 = fVar12 * fVar12 + fVar11 * fVar11;
      if ((*(float *)(unaff_EBX + 0x378fcb) <= fVar11 && fVar11 != *(float *)(unaff_EBX + 0x378fcb))
         && (this->m_bHasHitCube == false)) goto LAB_006b2ae0;
      iVar5 = (*(pCVar9->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                _vptr_IHandleEntity[0x55])(pCVar9);
      if ((char)iVar5 == '\0') {
LAB_006b2ab0:
        if (0.0 < local_20[0]) {
                    /* Unresolved local var: LaserVictimInfo_t victim@[???] */
          local_50.y = local_20[0];
          local_50.x = (vec_t)pCVar9;
          CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::InsertNoSort
                    (&local_118,(__376_conflict *)&local_50);
        }
        goto LAB_006b2ae0;
      }
                    /* Unresolved local var: float flZDist@[???] */
      if ((*(int *)(*(int *)(unaff_EBX + 0x67beeb) + 0x30) == 0) ||
         ((pCVar6 = CBaseEntity::GetGroundEntity(pCVar9), pCVar6 == (CBaseEntity *)0x0 &&
          (iVar5 = (*(pCVar9->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                     _vptr_IHandleEntity[0x9d])(pCVar9), 0.0 <= *(float *)(iVar5 + 8) - local_44.z))
         )) goto LAB_006b2ae0;
      iVar5 = (*(pCVar9->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                _vptr_IHandleEntity[0x55])(pCVar9);
      if ((char)iVar5 == '\0') {
        local_a18 = (CPortal_Player *)0x0;
      }
      else {
        local_a18 = (CPortal_Player *)
                    ___dynamic_cast(pCVar9,*(undefined4 *)(unaff_EBX + 0x4f9417),
                                    *(undefined4 *)(unaff_EBX + 0x4f9617),0);
      }
      if (((local_a18->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
           super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
           super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
           m_iEFlags & 0x800) != 0) {
        CBaseEntity::CalcAbsolutePosition((CBaseEntity *)local_a18);
      }
      fVar12 = (local_a18->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
               super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
               super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
               super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
               m_vecAbsOrigin.z;
      pVVar7 = CPortal_Player::GetHullMaxs(local_a18);
      if (fVar12 + pVVar7->z < local_44.z) goto LAB_006b2ae0;
      if (((local_a18->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
           super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
           super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
           m_iEFlags & 0x800) != 0) {
        CBaseEntity::CalcAbsolutePosition((CBaseEntity *)local_a18);
      }
      if (local_44.z <
          (local_a18->super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
          super_CPaintableEntity<CBaseMultiplayerPlayer>.super_CBaseMultiplayerPlayer.
          super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer.super_CBaseCombatCharacter.
          super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
          m_vecAbsOrigin.z) goto LAB_006b2ae0;
      if (fVar11 < *(float *)(unaff_EBX + 0x378fcb) || fVar11 == *(float *)(unaff_EBX + 0x378fcb))
      goto LAB_006b2ab0;
      iVar10 = iVar10 + 1;
    } while (iVar10 != iVar4);
  }
LAB_006b2cfb:
  local_118.m_bNeedsSort = false;
  if (1 < local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_Size)
  {
    local_44.x = (vec_t)local_118.m_pLessContext;
    local_44.y = (vec_t)&local_50;
    **(int **)(unaff_EBX + 0x4f97bf) = (int)&local_44;
    _qsort(local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.
           m_Memory.m_pMemory,
           local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_Size,
           8,*(int **)(unaff_EBX + 0x616b2f));
  }
  if (0 < local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_Size)
  {
    bVar2 = false;
    local_a14 = 0;
    local_a10 = 1;
    do {
      pCVar9 = *(CBaseEntity **)
                ((int)&(local_118.
                        super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.
                        m_Memory.m_pMemory)->pVictim + local_a14);
      if (pCVar9 == (CBaseEntity *)0x0) {
LAB_006b2d94:
        iVar4 = (*(pCVar9->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                  _vptr_IHandleEntity[0x55])(pCVar9);
        if ((((char)iVar4 != '\0') && ((pCVar9->m_MoveType).m_Value != '\b')) &&
           ((((local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.
               m_Size <= local_a10 ||
              (pCVar6 = *(CBaseEntity **)
                         ((int)&local_118.
                                super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>
                                .m_Memory.m_pMemory[1].pVictim + local_a14),
              pCVar6 == (CBaseEntity *)0x0)) ||
             ((root[0] = (float)(unaff_EBX + 0x35c01f),
              (pCVar6->m_iClassname).pszValue != (char *)root[0] &&
              (bVar3 = CBaseEntity::ClassMatchesComplex(pCVar6,(char *)root[0]), !bVar3)))) ||
            (!bIsFirstTrace)))) {
          if ((pCVar9->m_iEFlags & 0x1000) != 0) {
            CBaseEntity::CalcAbsoluteVelocity(pCVar9);
          }
          local_44.x = (pCVar9->m_vecAbsVelocity).x;
          local_44.y = (pCVar9->m_vecAbsVelocity).y;
          local_44.z = (pCVar9->m_vecAbsVelocity).z;
          VectorNormalize(&local_44);
          if ((pCVar9->m_iEFlags & 0x800) != 0) {
            CBaseEntity::CalcAbsolutePosition(pCVar9);
          }
          CalcClosestPointOnLineSegment
                    (&pCVar9->m_vecAbsOrigin,vecStart,vecEnd,&local_50,(float *)0x0);
          if ((pCVar9->m_iEFlags & 0x800) != 0) {
            CBaseEntity::CalcAbsolutePosition(pCVar9);
          }
          local_5c = (pCVar9->m_vecAbsOrigin).x;
          local_58 = (pCVar9->m_vecAbsOrigin).y;
          local_54 = local_50.z;
                    /* Unresolved local var: Vector res@[???] */
          local_74 = local_5c - local_50.x;
          local_70 = local_58 - local_50.y;
          local_6c = local_50.z - local_50.z;
          local_68.x = local_74;
          local_68.y = local_70;
          local_68.z = local_6c;
          VectorNormalize(&local_68);
          local_68.z = 0.0;
          if (*(float *)(unaff_EBX + 0x37cc67) <=
              local_44.x * local_44.x + local_44.y * local_44.y + local_44.z * local_44.z) {
                    /* Unresolved local var: float flDot@[???] */
            fVar11 = local_44.x * local_68.x + local_44.y * local_68.y + local_44.z * 0.0;
                    /* Unresolved local var: Vector res@[???] */
            local_c8 = local_68.x * *(float *)(unaff_EBX + 0x3781db);
            local_c4 = *(float *)(unaff_EBX + 0x3781db) * local_68.y;
            local_c0 = 0x80000000;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
            local_a4 = fVar11 * local_c8 + local_44.x;
            local_a0 = fVar11 * local_c4 + local_44.y;
            local_9c = fVar11 * -0.0 + local_44.z;
            local_80.x = local_a4;
            local_80.y = local_a0;
            local_80.z = local_9c;
            VectorNormalize(&local_80);
            local_80.z = 0.0;
            if (local_80.x * local_68.x + local_80.y * local_68.y + local_68.z * 0.0 < 0.0)
            goto LAB_006b31a0;
          }
          else {
            local_8c = 0;
            local_88 = 0;
            local_84 = 0x3f800000;
            local_bc = local_2c.y - local_2c.z * 0.0;
            local_b8 = local_2c.z * 0.0 - local_2c.x;
            local_b4 = local_2c.x * 0.0 - local_2c.y * 0.0;
            local_98 = local_bc;
            local_94 = local_b8;
            local_90 = local_b4;
            local_80.x = local_bc;
            local_80.y = local_b8;
            local_80.z = local_b4;
          }
                    /* Unresolved local var: Vector res@[???] */
          pfVar1 = (float *)(&DAT_00377e67 + unaff_EBX);
          local_1bc.m_vecDamageForce.x = local_80.x * *pfVar1;
          local_1bc.m_vecDamageForce.y = local_80.y * *pfVar1;
          local_1bc.m_vecDamageForce.z = *pfVar1 * local_80.z;
          local_b0 = local_1bc.m_vecDamageForce.x;
          local_ac = local_1bc.m_vecDamageForce.y;
          local_a8 = local_1bc.m_vecDamageForce.z;
          if (((pCVar9->m_fFlags).m_Value & 1) != 0) {
            CBaseEntity::SetGroundEntity(pCVar9,(CBaseEntity *)0x0);
            CBaseEntity::SetGroundChangeTime
                      (pCVar9,*(float *)(&DAT_00377d4f + unaff_EBX) +
                              *(float *)(**(int **)(unaff_EBX + 0x4f93ef) + 0xc));
            local_1bc.m_vecDamageForce.x =
                 local_1bc.m_vecDamageForce.x + local_1bc.m_vecDamageForce.x;
            local_1bc.m_vecDamageForce.y =
                 local_1bc.m_vecDamageForce.y + local_1bc.m_vecDamageForce.y;
            local_1bc.m_vecDamageForce.z =
                 local_1bc.m_vecDamageForce.z + local_1bc.m_vecDamageForce.z;
          }
          CBaseEntity::SetAbsVelocity(pCVar9,&local_1bc.m_vecDamageForce);
          if (this->m_bIsLethal == false) {
            fVar11 = *(float *)(&DAT_00377eef + unaff_EBX);
          }
          else {
            fVar11 = *(float *)(unaff_EBX + 0x37af83);
          }
                    /* Unresolved local var: CTakeDamageInfo info@[???] */
          CTakeDamageInfo::CTakeDamageInfo
                    (&local_20c,(CBaseEntity *)this,(CBaseEntity *)this,
                     fVar11 * *(float *)(**(int **)(unaff_EBX + 0x4f93ef) + 0x10),8,0,
                     in_stack_fffff59c,in_stack_fffff5a0,pAttacker_1,in_stack_fffff5a8,
                     in_stack_fffff5ac,in_stack_fffff5b0);
          pVVar7 = (Vector *)
                   (*(pCVar9->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                     _vptr_IHandleEntity[0x9d])(pCVar9);
          local_e0.x = 1.0;
          local_e0.y = 0.0;
          local_e0.z = 0.0;
          CalculateMeleeDamageForce(&local_20c,&local_e0,pVVar7,1.0);
          CBaseEntity::TakeDamage(pCVar9,&local_20c);
          CBaseEntity::EmitSound(pCVar9,(char *)(unaff_EBX + 0x35c11b),0.0,(float *)0x0);
        }
      }
      else {
        root[0] = (float)(unaff_EBX + 0x35c01f);
        if ((((pCVar9->m_iClassname).pszValue != (char *)root[0]) &&
            (bVar3 = CBaseEntity::ClassMatchesComplex(pCVar9,(char *)root[0]), !bVar3)) || (bVar2))
        {
          root[0] = (float)(&UNK_0030b5f7 + unaff_EBX);
          if (((pCVar9->m_iClassname).pszValue != (char *)root[0]) &&
             (bVar3 = CBaseEntity::ClassMatchesComplex(pCVar9,(char *)root[0]), !bVar3))
          goto LAB_006b2d94;
          bVar2 = true;
        }
        else {
          pCVar6 = (CBaseEntity *)
                   ___dynamic_cast(pCVar9,*(undefined4 *)(unaff_EBX + 0x4f9417),
                                   *(undefined4 *)(unaff_EBX + 0x4f9f2f),0);
          if ((pCVar6 != (CBaseEntity *)0x0) &&
             (*(char *)((int)&pCVar6[1].m_Network.m_PVSInfo.m_nAreaNum + 1) != '\0')) {
            if (pVecOut != (Vector *)0x0) {
              pvVar8 = (vec_t *)(*(pCVar6->super_IServerEntity).super_IServerUnknown.
                                  super_IHandleEntity._vptr_IHandleEntity[0x9d])(pCVar6);
              pVecOut->x = *pvVar8;
              pVecOut->y = pvVar8[1];
              pVecOut->z = pvVar8[2];
            }
            goto LAB_006b341d;
          }
          CTakeDamageInfo::CTakeDamageInfo
                    (&local_1bc,(CBaseEntity *)this,(CBaseEntity *)this,
                     *(float *)(**(int **)(unaff_EBX + 0x4f93ef) + 0x10),8,0,in_stack_fffff59c,
                     in_stack_fffff5a0,pAttacker_1,in_stack_fffff5a8,in_stack_fffff5ac,
                     in_stack_fffff5b0);
          pVVar7 = (Vector *)
                   (*(pCVar6->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                     _vptr_IHandleEntity[0x9d])(pCVar6);
          local_d4.x = 1.0;
          local_d4.y = 0.0;
          local_d4.z = 0.0;
          CalculateMeleeDamageForce(&local_1bc,&local_d4,pVVar7,1.0);
          CBaseEntity::TakeDamage(pCVar6,&local_1bc);
        }
      }
LAB_006b31a0:
      local_a14 = local_a14 + 8;
      bVar3 = local_a10 <
              local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.
              m_Size;
      local_a10 = local_a10 + 1;
    } while (bVar3);
  }
  pCVar9 = (CBaseEntity *)0x0;
  if (pVecOut != (Vector *)0x0) {
    pVecOut->x = vecEnd->x;
    pVecOut->y = vecEnd->y;
    pVecOut->z = vecEnd->z;
  }
LAB_006b341d:
  local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_Size = 0;
  if (-1 < local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.
           m_Memory.m_nGrowSize) {
    if (local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_Memory.
        m_pMemory != (__376_conflict *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4f93e7) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4f93e7),
                 local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.
                 m_Memory.m_pMemory);
      local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_Memory.
      m_pMemory = (__376_conflict *)0x0;
    }
    local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_Memory.
    m_nAllocationCount = 0;
  }
  local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_pElements =
       local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_Memory.
       m_pMemory;
  if ((-1 < local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.
            m_Memory.m_nGrowSize) &&
     (local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.m_Memory.
      m_pMemory != (__376_conflict *)0x0)) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4f93e7) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4f93e7),
               local_118.super_CUtlVector<LaserVictimInfo_t,CUtlMemory<LaserVictimInfo_t,_int>_>.
               m_Memory.m_pMemory);
  }
  return pCVar9;
}


/* CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::CompareHelper at 006b5ff0 */

int CUtlSortVector<LaserVictimInfo_t,CLaserVictimLess>::CompareHelper
              (__376_conflict *lhs,__376_conflict *rhs)

{
  uint uVar1;
  
                    /* Unresolved local var: QSortContext_t * ctx@[???] */
  uVar1 = 0xffffffff;
  if (rhs->flFraction <= lhs->flFraction) {
    uVar1 = (uint)(rhs->flFraction < lhs->flFraction);
  }
  return uVar1;
}


/* CPortalLaser::ShouldAutoAim at 006b1060 */

/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: bool ShouldAutoAim(CPortalLaser * this, CBaseEntity * pEntity) */

bool __thiscall CPortalLaser::ShouldAutoAim(CPortalLaser *this,CBaseEntity *pEntity)

{
  bool bVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  bVar1 = false;
  if (pEntity != (CBaseEntity *)0x0) {
    bVar1 = true;
    if ((pEntity->m_iClassname).pszValue != (char *)(unaff_EBX + 0x35d75f)) {
      bVar1 = CBaseEntity::ClassMatchesComplex(pEntity,(char *)(unaff_EBX + 0x35d75f));
    }
  }
  return bVar1;
}


/* CPortalLaser::TraceLaser at 006b3650 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: CBaseEntity * TraceLaser(CPortalLaser * this, bool bIsFirstTrace,
   Vector * vecStart, Vector * vecDirection, float * flTotalBeamLength, trace_t * tr, Vector *
   pVecAutoAimOffset) */

CBaseEntity * __thiscall
CPortalLaser::TraceLaser
          (CPortalLaser *this,bool bIsFirstTrace,Vector *vecStart,Vector *vecDirection,
          float *flTotalBeamLength,trace_t *tr,Vector *pVecAutoAimOffset)

{
  VMatrix matThisToLinked;
  VMatrix matThisToLinked_00;
  VMatrix matThisToLinked_01;
  int iVar1;
  bool bVar2;
  int iVar3;
  Vector *pVVar4;
  float *pfVar5;
  CPhysicsShadowClone *this_00;
  int unaff_EBX;
  undefined4 *puVar6;
  VMatrix *pVVar7;
  undefined4 uVar8;
  undefined4 *puVar9;
  vec_t *pvVar10;
  float fVar11;
  float fVar12;
  float fVar13;
  CPortalLaser *pCVar14;
  Vector *pVVar15;
  CPortal_Base2D *pchClassname;
  trace_t *ptVar16;
  CPortal_Base2D *pCVar17;
  Vector *pVVar18;
  CPhysicsShadowClone *pCVar19;
  uint uVar20;
  undefined4 in_stack_fffffe08;
  float in_stack_fffffe0c;
  undefined1 in_stack_fffffe10 [36];
  Ray_t *ray;
  Vector *ptSource;
  Ray_t *rayTransformed;
  Vector *ptTransformed;
  CPhysicsShadowClone *local_198;
  int local_190;
  int local_18c;
  int local_188;
  undefined4 *local_184;
  Ray_t local_17c;
  Ray_t local_12c;
  CTraceFilterSimpleClassnameList local_d0;
  float local_ac;
  undefined4 uStack_a8;
  undefined4 uStack_a4;
  undefined4 uStack_a0;
  Vector local_9c;
  float local_90;
  _func___thiscall_void_CBaseEntity_ptr *local_8c;
  float local_88;
  IServerNetworkable local_84;
  IEventRegisterCallback local_80;
  CBaseEntity *local_7c;
  float local_6c;
  _func___thiscall_void_CBaseEntity_ptr *local_68;
  float local_64;
  ServerClass *local_54;
  EHANDLE local_50;
  CTimedEventMgr *local_4c;
  IServerNetworkable local_48;
  IEventRegisterCallback local_44;
  CBaseEntity *local_40;
  Vector local_3c;
  Vector local_30;
  CPortal_Base2D *local_24;
  CNetworkVarBase<float,CBaseEntity::NetworkVar_m_flAnimTime> local_20;
  
                    /* Unresolved local var: Vector vStart@[???]
                       Unresolved local var: Vector vDir@[???]
                       Unresolved local var: CTraceFilterSimpleClassnameList traceFilter@[???]
                       Unresolved local var:
                       CUtlVector<CPortal_Base2D*,CUtlMemory<CPortal_Base2D*,_int>_>
                       portalList@[???]
                       Unresolved local var: CBaseEntity * pHitEntity@[???]
                       Unresolved local var: bool bGoingThroughPortal@[???] */
  ___i686_get_pc_thunk_bx();
  *flTotalBeamLength = 0.0;
  local_30.x = vecStart->x;
  local_30.y = vecStart->y;
  local_30.z = vecStart->z;
  local_3c.x = vecDirection->x;
  local_3c.y = vecDirection->y;
  local_3c.z = vecDirection->z;
  CTraceFilterSimpleClassnameList::CTraceFilterSimpleClassnameList
            (&local_d0,(IHandleEntity *)this,0);
  CTraceFilterSimpleClassnameList::AddClassnameToIgnore(&local_d0,&UNK_00305b83 + unaff_EBX);
  CTraceFilterSimpleClassnameList::AddClassnameToIgnore(&local_d0,(char *)(unaff_EBX + 0x302b0b));
  pchClassname = (CPortal_Base2D *)(unaff_EBX + 0x35b16f);
  CTraceFilterSimpleClassnameList::AddClassnameToIgnore(&local_d0,(char *)pchClassname);
  local_18c = 0;
  local_188 = 0;
  local_184 = (undefined4 *)0x0;
  puVar6 = *(undefined4 **)(unaff_EBX + 0x4f8537);
  while( true ) {
    UTIL_ClearTrace(tr);
                    /* Unresolved local var: Vector res@[???] */
    pfVar5 = (float *)(unaff_EBX + 0x377c23);
    local_48._vptr_IServerNetworkable = (_func_int_varargs **)(*pfVar5 * local_3c.x);
    local_44._vptr_IEventRegisterCallback = (_func_int_varargs **)(*pfVar5 * local_3c.y);
    local_40 = (CBaseEntity *)(*pfVar5 * local_3c.z);
                    /* Unresolved local var: Vector res@[???] */
    local_54 = (ServerClass *)(local_30.x + (float)local_48._vptr_IServerNetworkable);
    local_50.super_CBaseHandle.m_Index =
         (CBaseHandle)(local_30.y + (float)local_44._vptr_IEventRegisterCallback);
    local_4c = (CTimedEventMgr *)(local_30.z + (float)local_40);
    local_12c.m_Delta.super_Vector.x = (float)local_54 - local_30.x;
    local_12c.m_Delta.super_Vector.y = (float)local_50.super_CBaseHandle.m_Index - local_30.y;
    local_12c.m_Delta.super_Vector.z = (float)local_4c - local_30.z;
    local_12c.m_IsSwept =
         local_12c.m_Delta.super_Vector.x * local_12c.m_Delta.super_Vector.x +
         local_12c.m_Delta.super_Vector.y * local_12c.m_Delta.super_Vector.y +
         local_12c.m_Delta.super_Vector.z * local_12c.m_Delta.super_Vector.z != 0.0;
    local_12c.m_Extents.super_Vector.z = 0.0;
    local_12c.m_Extents.super_Vector.y = 0.0;
    local_12c.m_Extents.super_Vector.x = 0.0;
    local_12c.m_pWorldAxisTransform = (matrix3x4_t *)0x0;
    local_12c.m_IsRay = true;
    local_12c.m_StartOffset.super_Vector.z = 0.0;
    local_12c.m_StartOffset.super_Vector.y = 0.0;
    local_12c.m_StartOffset.super_Vector.x = 0.0;
    local_12c.m_Start.super_Vector.x = local_30.x;
    local_12c.m_Start.super_Vector.y = local_30.y;
    local_12c.m_Start.super_Vector.z = local_30.z;
    ray = &local_12c;
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4f85ab) + 0x14))();
    pVVar18 = &(tr->super_CBaseTrace).endpos;
    if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x4f85a7) + 0x1c) + 0x30) != 0) {
      in_stack_fffffe0c = -1.0;
      in_stack_fffffe08 = 1;
      DebugDrawLine((Vector *)tr,pVVar18,0xff,0,0,true,-1.0);
    }
    if (*(int *)(*(int *)(unaff_EBX + 0x67b15b) + 0x30) != 0) {
      if (pVecAutoAimOffset == (Vector *)0x0) {
        in_stack_fffffe0c = *(float *)(*(int *)(unaff_EBX + 0x67b0fb) + 0x2c);
        in_stack_fffffe08 = 1;
        NDebugOverlay::Line((Vector *)tr,pVVar18,0,0,0xff,true,in_stack_fffffe0c);
      }
      else {
        in_stack_fffffe0c = *(float *)(*(int *)(unaff_EBX + 0x67b0fb) + 0x2c);
        in_stack_fffffe08 = 1;
        NDebugOverlay::Line((Vector *)tr,pVVar18,0xff,0xff,0,true,in_stack_fffffe0c);
      }
    }
    UpdateSoundPosition(this,(Vector *)tr,pVVar18);
    local_24 = (CPortal_Base2D *)0x0;
    bVar2 = UTIL_DidTraceTouchPortals(ray,tr,&local_24,(CPortal_Base2D **)0x0);
    pCVar17 = (CPortal_Base2D *)tr;
    if (((bVar2) && (local_24 != (CPortal_Base2D *)0x0)) &&
       (bVar2 = CPortal_Base2D::IsActivedAndLinked(local_24), bVar2)) {
      uVar20 = (uint)bIsFirstTrace;
      this_00 = (CPhysicsShadowClone *)&stack0xffffffa0;
      pCVar14 = this;
      ptVar16 = tr;
      pVVar4 = pVVar18;
      pCVar19 = this_00;
      local_198 = (CPhysicsShadowClone *)
                  NewStrikeEntitiesAlongLaser
                            (this,(Vector *)tr,pVVar18,(Vector *)this_00,bIsFirstTrace);
      if (local_198 != (CPhysicsShadowClone *)0x0) {
                    /* Unresolved local var: Vector vPointOnLine@[???] */
        uVar20 = 0;
        pCVar19 = (CPhysicsShadowClone *)&local_9c;
        CalcClosestPointOnLineSegment
                  ((Vector *)this_00,(Vector *)tr,pVVar18,(Vector *)pCVar19,(float *)0x0);
                    /* Unresolved local var: Vector res@[???] */
        fVar13 = local_9c.x - (tr->super_CBaseTrace).startpos.x;
        fVar11 = local_9c.y - (tr->super_CBaseTrace).startpos.y;
        fVar12 = local_9c.z - (tr->super_CBaseTrace).startpos.z;
                    /* Unresolved local var: __m128 root@[???] */
        local_ac = SQRT(fVar13 * fVar13 + fVar11 * fVar11 + fVar12 * fVar12);
        uStack_a8 = 0;
        uStack_a4 = 0;
        uStack_a0 = 0;
        *flTotalBeamLength = *flTotalBeamLength + local_ac;
        pVVar4 = pVVar18;
        if (pVecAutoAimOffset != (Vector *)0x0) {
          this_00 = local_198;
          pfVar5 = (float *)(*(local_198->super_CBaseAnimating).super_CBaseEntity.
                              super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
                              _vptr_IHandleEntity[0x9d])();
                    /* Unresolved local var: Vector res@[???] */
          local_6c = *pfVar5 - local_9c.x;
          local_68 = (_func___thiscall_void_CBaseEntity_ptr *)(pfVar5[1] - local_9c.y);
          local_64 = pfVar5[2] - local_9c.z;
          pVecAutoAimOffset->x = local_6c;
          pVecAutoAimOffset->y = (vec_t)local_68;
          pVecAutoAimOffset->z = local_64;
          pVVar4 = pVVar18;
        }
        goto LAB_006b3c09;
      }
      local_17c.m_pWorldAxisTransform = (matrix3x4_t *)0x0;
      rayTransformed = &local_17c;
      pVVar7 = &local_24->m_matrixThisToLinked;
      pvVar10 = (vec_t *)&stack0xfffffdf4;
      for (iVar3 = 0x10; iVar3 != 0; iVar3 = iVar3 + -1) {
        *pvVar10 = pVVar7->m[0][0];
        pVVar7 = (VMatrix *)(pVVar7->m[0] + 1);
        pvVar10 = pvVar10 + 1;
      }
      matThisToLinked_00.m[0][1] = (vec_t)ptVar16;
      matThisToLinked_00.m[0][0] = (vec_t)pCVar14;
      matThisToLinked_00.m[0][2] = (vec_t)pVVar4;
      matThisToLinked_00.m[0][3] = (vec_t)pCVar19;
      matThisToLinked_00.m[1][0] = (vec_t)uVar20;
      matThisToLinked_00.m[1][1] = (vec_t)in_stack_fffffe08;
      matThisToLinked_00.m[1][2] = in_stack_fffffe0c;
      matThisToLinked_00.m[1][3] = (vec_t)in_stack_fffffe10._0_4_;
      matThisToLinked_00.m[2][0] = (vec_t)in_stack_fffffe10._4_4_;
      matThisToLinked_00.m[2][1] = (vec_t)in_stack_fffffe10._8_4_;
      matThisToLinked_00.m[2][2] = (vec_t)in_stack_fffffe10._12_4_;
      matThisToLinked_00.m[2][3] = (vec_t)in_stack_fffffe10._16_4_;
      matThisToLinked_00.m[3][0] = (vec_t)in_stack_fffffe10._20_4_;
      matThisToLinked_00.m[3][1] = (vec_t)in_stack_fffffe10._24_4_;
      matThisToLinked_00.m[3][2] = (vec_t)in_stack_fffffe10._28_4_;
      matThisToLinked_00.m[3][3] = (vec_t)in_stack_fffffe10._32_4_;
      UTIL_Portal_RayTransform(matThisToLinked_00,ray,rayTransformed);
      local_3c.x = local_17c.m_Delta.super_Vector.x;
      local_3c.y = local_17c.m_Delta.super_Vector.y;
      local_3c.z = local_17c.m_Delta.super_Vector.z;
      pVVar15 = &local_3c;
      VectorNormalize(pVVar15);
      ptTransformed = &local_30;
      pVVar7 = &local_24->m_matrixThisToLinked;
      pvVar10 = (vec_t *)&stack0xfffffdf4;
      ptSource = pVVar18;
      for (iVar3 = 0x10; iVar3 != 0; iVar3 = iVar3 + -1) {
        *pvVar10 = pVVar7->m[0][0];
        pVVar7 = (VMatrix *)(pVVar7->m[0] + 1);
        pvVar10 = pvVar10 + 1;
      }
      matThisToLinked_01.m[0][1] = (vec_t)ptVar16;
      matThisToLinked_01.m[0][0] = (vec_t)pVVar15;
      matThisToLinked_01.m[0][2] = (vec_t)pVVar4;
      matThisToLinked_01.m[0][3] = (vec_t)pCVar19;
      matThisToLinked_01.m[1][0] = (vec_t)uVar20;
      matThisToLinked_01.m[1][1] = (vec_t)in_stack_fffffe08;
      matThisToLinked_01.m[1][2] = in_stack_fffffe0c;
      matThisToLinked_01.m[1][3] = (vec_t)in_stack_fffffe10._0_4_;
      matThisToLinked_01.m[2][0] = (vec_t)in_stack_fffffe10._4_4_;
      matThisToLinked_01.m[2][1] = (vec_t)in_stack_fffffe10._8_4_;
      matThisToLinked_01.m[2][2] = (vec_t)in_stack_fffffe10._12_4_;
      matThisToLinked_01.m[2][3] = (vec_t)in_stack_fffffe10._16_4_;
      matThisToLinked_01.m[3][0] = (vec_t)in_stack_fffffe10._20_4_;
      matThisToLinked_01.m[3][1] = (vec_t)in_stack_fffffe10._24_4_;
      matThisToLinked_01.m[3][2] = (vec_t)in_stack_fffffe10._28_4_;
      matThisToLinked_01.m[3][3] = (vec_t)in_stack_fffffe10._32_4_;
      UTIL_Portal_PointTransform(matThisToLinked_01,ptSource,ptTransformed);
      bVar2 = true;
    }
    else {
      bVar2 = false;
    }
    uVar20 = (uint)bIsFirstTrace;
    this_00 = (CPhysicsShadowClone *)&stack0xffffff88;
    pVVar4 = pVVar18;
    pCVar19 = this_00;
    local_198 = (CPhysicsShadowClone *)
                NewStrikeEntitiesAlongLaser
                          (this,(Vector *)tr,pVVar18,(Vector *)this_00,bIsFirstTrace);
    if (local_198 != (CPhysicsShadowClone *)0x0) {
                    /* Unresolved local var: Vector vPointOnLine@[???] */
      uVar20 = 0;
      pCVar19 = (CPhysicsShadowClone *)&local_9c;
      pCVar17 = (CPortal_Base2D *)tr;
      CalcClosestPointOnLineSegment
                ((Vector *)this_00,(Vector *)tr,pVVar18,(Vector *)pCVar19,(float *)0x0);
                    /* Unresolved local var: Vector res@[???] */
      fVar13 = local_9c.x - (tr->super_CBaseTrace).startpos.x;
      fVar11 = local_9c.y - (tr->super_CBaseTrace).startpos.y;
      fVar12 = local_9c.z - (tr->super_CBaseTrace).startpos.z;
                    /* Unresolved local var: __m128 root@[???] */
      local_ac = SQRT(fVar13 * fVar13 + fVar11 * fVar11 + fVar12 * fVar12);
      uStack_a8 = 0;
      uStack_a4 = 0;
      uStack_a0 = 0;
      *flTotalBeamLength = *flTotalBeamLength + local_ac;
      pVVar4 = pVVar18;
      if (pVecAutoAimOffset != (Vector *)0x0) {
        this_00 = local_198;
        pfVar5 = (float *)(*(local_198->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.
                            super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x9d])();
                    /* Unresolved local var: Vector res@[???] */
        local_84._vptr_IServerNetworkable = (_func_int_varargs **)(*pfVar5 - local_9c.x);
        local_80._vptr_IEventRegisterCallback = (_func_int_varargs **)(pfVar5[1] - local_9c.y);
        local_7c = (CBaseEntity *)(pfVar5[2] - local_9c.z);
        pVecAutoAimOffset->x = (vec_t)local_84._vptr_IServerNetworkable;
        pVecAutoAimOffset->y = (vec_t)local_80._vptr_IEventRegisterCallback;
        pVecAutoAimOffset->z = (vec_t)local_7c;
        pVVar4 = pVVar18;
      }
      goto LAB_006b3c09;
    }
                    /* Unresolved local var: Vector res@[???] */
    fVar13 = (tr->super_CBaseTrace).endpos.x - (tr->super_CBaseTrace).startpos.x;
    fVar11 = (tr->super_CBaseTrace).endpos.y - (tr->super_CBaseTrace).startpos.y;
    fVar12 = (tr->super_CBaseTrace).endpos.z - (tr->super_CBaseTrace).startpos.z;
                    /* Unresolved local var: __m128 root@[???] */
    local_ac = SQRT(fVar13 * fVar13 + fVar11 * fVar11 + fVar12 * fVar12);
    uStack_a8 = 0;
    uStack_a4 = 0;
    uStack_a0 = 0;
    *flTotalBeamLength = *flTotalBeamLength + local_ac;
    if (!bVar2) break;
    uVar20 = (local_24->m_hLinkedPortal).
             super_CNetworkVarBase<CBaseHandle,CPortal_Base2D::NetworkVar_m_hLinkedPortal>.m_Value.
             m_Index;
    if ((uVar20 == 0xffffffff) ||
       (iVar3 = (uVar20 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f8543),
       *(uint *)(iVar3 + 8) != uVar20 >> 0x10)) {
      uVar8 = 0;
    }
    else {
      uVar8 = *(undefined4 *)(iVar3 + 4);
    }
    iVar3 = local_18c + 1;
    if (local_188 < iVar3) {
      if ((local_188 != 0) || (local_188 = 8, iVar1 = local_188, 8 < iVar3)) {
        do {
          iVar1 = local_188 << 1;
          if (iVar3 <= local_188 << 1) break;
          local_188 = local_188 << 2;
          iVar1 = local_188;
        } while (local_188 < iVar3);
      }
      local_188 = iVar1;
      if (local_184 == (undefined4 *)0x0) {
        local_184 = (undefined4 *)
                    (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x4f8537))();
      }
      else {
        local_184 = (undefined4 *)(**(code **)(*(int *)*puVar6 + 4))();
      }
    }
    if (local_18c != 0) {
      _V_memmove(local_184 + 1,local_184,local_18c * 4);
    }
    local_18c = iVar3;
    if (local_184 != (undefined4 *)0x0) {
      *local_184 = uVar8;
    }
  }
  this_00 = (CPhysicsShadowClone *)tr->m_pEnt;
  if (this_00 != (CPhysicsShadowClone *)0x0) {
    local_24 = pchClassname;
    if ((CPortal_Base2D *)(this_00->super_CBaseAnimating).super_CBaseEntity.m_iClassname.pszValue !=
        pchClassname) {
      pCVar17 = pchClassname;
      bVar2 = CBaseEntity::ClassMatchesComplex((CBaseEntity *)this_00,(char *)pchClassname);
      if (!bVar2) goto LAB_006b3d00;
      this_00 = (CPhysicsShadowClone *)tr->m_pEnt;
    }
    bVar2 = CPhysicsShadowClone::IsShadowClone((CBaseEntity *)this_00);
    if (bVar2) {
      this_00 = (CPhysicsShadowClone *)&local_20;
      pCVar17 = (CPortal_Base2D *)tr->m_pEnt;
      CPhysicsShadowClone::GetClonedEntity(this_00);
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
      if ((local_20.m_Value != -NAN) &&
         (iVar3 = ((uint)local_20.m_Value & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f8543),
         *(uint *)(iVar3 + 8) == (uint)local_20.m_Value >> 0x10)) {
        local_198 = *(CPhysicsShadowClone **)(iVar3 + 4);
      }
    }
    else {
      local_198 = (CPhysicsShadowClone *)tr->m_pEnt;
    }
    if (pVecAutoAimOffset != (Vector *)0x0) {
                    /* Unresolved local var: Vector vPointOnLine@[???] */
      pVVar4 = (Vector *)
               (*(local_198->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.
                 super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x9d])();
      uVar20 = 0;
      pCVar19 = (CPhysicsShadowClone *)&local_9c;
      CalcClosestPointOnLine(pVVar4,(Vector *)tr,pVVar18,(Vector *)pCVar19,(float *)0x0);
      this_00 = local_198;
      pfVar5 = (float *)(*(local_198->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.
                          super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x9d])();
                    /* Unresolved local var: Vector res@[???] */
      local_90 = *pfVar5 - local_9c.x;
      local_8c = (_func___thiscall_void_CBaseEntity_ptr *)(pfVar5[1] - local_9c.y);
      local_88 = pfVar5[2] - local_9c.z;
      pVecAutoAimOffset->x = local_90;
      pVecAutoAimOffset->y = (vec_t)local_8c;
      pVecAutoAimOffset->z = local_88;
      pCVar17 = (CPortal_Base2D *)tr;
      pVVar4 = pVVar18;
    }
LAB_006b3c09:
    if ((local_198 != (CPhysicsShadowClone *)0x0) &&
       (((local_24 = pchClassname,
         (CPortal_Base2D *)(local_198->super_CBaseAnimating).super_CBaseEntity.m_iClassname.pszValue
         == pchClassname ||
         (this_00 = local_198,
         bVar2 = CBaseEntity::ClassMatchesComplex((CBaseEntity *)local_198,(char *)pchClassname),
         pCVar17 = pchClassname, bVar2)) && (pVecAutoAimOffset != (Vector *)0x0)))) {
                    /* Unresolved local var: Vector vAutoAimOffset@[???] */
      local_9c.x = pVecAutoAimOffset->x;
      local_9c.y = pVecAutoAimOffset->y;
      local_9c.z = pVecAutoAimOffset->z;
                    /* Unresolved local var: int i@[???] */
      if (0 < local_18c) {
        local_190 = 0;
        do {
                    /* Unresolved local var: CPortal_Base2D * pPortal@[???] */
          puVar6 = (undefined4 *)(local_184[local_190] + 0x528);
          puVar9 = (undefined4 *)&stack0xfffffdf4;
          pVVar18 = &local_9c;
          pVVar15 = &local_9c;
          for (iVar3 = 0x10; iVar3 != 0; iVar3 = iVar3 + -1) {
            *puVar9 = *puVar6;
            puVar6 = puVar6 + 1;
            puVar9 = puVar9 + 1;
          }
          matThisToLinked.m[0][1] = (vec_t)pCVar17;
          matThisToLinked.m[0][0] = (vec_t)this_00;
          matThisToLinked.m[0][2] = (vec_t)pVVar4;
          matThisToLinked.m[0][3] = (vec_t)pCVar19;
          matThisToLinked.m[1][0] = (vec_t)uVar20;
          matThisToLinked.m[1][1] = (vec_t)in_stack_fffffe08;
          matThisToLinked.m[1][2] = in_stack_fffffe0c;
          matThisToLinked.m[1][3] = (vec_t)in_stack_fffffe10._0_4_;
          matThisToLinked.m[2][0] = (vec_t)in_stack_fffffe10._4_4_;
          matThisToLinked.m[2][1] = (vec_t)in_stack_fffffe10._8_4_;
          matThisToLinked.m[2][2] = (vec_t)in_stack_fffffe10._12_4_;
          matThisToLinked.m[2][3] = (vec_t)in_stack_fffffe10._16_4_;
          matThisToLinked.m[3][0] = (vec_t)in_stack_fffffe10._20_4_;
          matThisToLinked.m[3][1] = (vec_t)in_stack_fffffe10._24_4_;
          matThisToLinked.m[3][2] = (vec_t)in_stack_fffffe10._28_4_;
          matThisToLinked.m[3][3] = (vec_t)in_stack_fffffe10._32_4_;
          UTIL_Portal_VectorTransform(matThisToLinked,pVVar18,pVVar15);
          local_190 = local_190 + 1;
        } while (local_190 < local_18c);
      }
      pVecAutoAimOffset->x = local_9c.x;
      pVecAutoAimOffset->y = local_9c.y;
      pVecAutoAimOffset->z = local_9c.z;
    }
  }
LAB_006b3d00:
  if (local_184 != (undefined4 *)0x0) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4f8537) + 8))();
  }
  local_d0.super_CTraceFilterSimple.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter =
       (ITraceFilter)(*(int *)(unaff_EBX + 0x4f8667) + 8);
  local_d0.m_PassClassnames.m_Size = 0;
  if (-1 < local_d0.m_PassClassnames.m_Memory.m_nGrowSize) {
    if (local_d0.m_PassClassnames.m_Memory.m_pMemory != (char **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4f8537) + 8))();
      local_d0.m_PassClassnames.m_Memory.m_pMemory = (char **)0x0;
    }
    local_d0.m_PassClassnames.m_Memory.m_nAllocationCount = 0;
  }
  if ((-1 < local_d0.m_PassClassnames.m_Memory.m_nGrowSize) &&
     (local_d0.m_PassClassnames.m_Memory.m_pMemory != (char **)0x0)) {
    local_d0.m_PassClassnames.m_pElements = local_d0.m_PassClassnames.m_Memory.m_pMemory;
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4f8537) + 8))();
  }
  return (CBaseEntity *)local_198;
}


/* _GLOBAL__I_sv_player_collide_with_laser at 000be030 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_sv_player_collide_with_laser(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

