/* DWARF-guided pseudocode for game/server/portal2/prop_rockettripwire.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* DataMapInit<CPropRocketTripwire> at 000b8890 */

datamap_t * DataMapInit<CPropRocketTripwire>(CPropRocketTripwire *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  char *pcVar4;
  size_t sVar5;
  int iVar6;
  undefined4 *puVar7;
  int iVar8;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((*(char *)((int)&r_visualizetraces.super_IConVar._vptr_IConVar + unaff_EBX + 2) == '\0') &&
     (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xd980fa), iVar3 != 0)) {
    *(undefined **)((int)&r_visualizetraces.m_Value.m_StringLength + unaff_EBX + 2) =
         &UNK_009fcec9 + unaff_EBX;
    *(undefined4 *)((int)&r_visualizetraces.m_Value.m_nValue + unaff_EBX + 2) = 0;
    *(undefined4 *)(&r_visualizetraces.field_0x36 + unaff_EBX) = 0;
    *(undefined4 *)((int)&r_visualizetraces.m_fMinVal + unaff_EBX + 2) = 0;
    *(undefined4 *)(&r_visualizetraces.field_0x3e + unaff_EBX) = 0;
    *(undefined4 *)((int)&r_visualizetraces.m_fMaxVal + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&r_visualizetraces.m_Value.m_fValue + unaff_EBX + 2) = 0x13;
    ___cxa_guard_release(unaff_EBX + 0xd980fa);
    ___cxa_atexit(unaff_EBX + 0x988092,0,*(undefined4 *)(&DAT_00b8e2b6 + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_DecalTrace_00cb21cc + unaff_EBX + 2) =
       *(undefined4 *)(&DAT_00b8e2c2 + unaff_EBX);
  if ((*(char *)((int)&r_visualizetraces.m_pszDefaultValue + unaff_EBX + 2) == '\0') &&
     (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xd98102), iVar3 != 0)) {
    uVar1 = **(undefined4 **)(&DAT_00b8e2d6 + unaff_EBX);
    *(undefined4 *)((int)&PTR_MakeTracer_00cb2574 + unaff_EBX + 2) = uVar1;
    *(undefined4 *)((int)&PTR_NetworkStateChanged_m_takedamage_00cb25b4 + unaff_EBX + 2) = uVar1;
    pcVar4 = operator_new___(*(int *)((int)&r_visualizetraces.m_Value.m_fValue + unaff_EBX + 2) +
                             0x10);
    _strcpy(pcVar4,*(char **)((int)&r_visualizetraces.m_Value.m_StringLength + unaff_EBX + 2));
    sVar5 = _strlen(pcVar4);
    builtin_strncpy(pcVar4 + sVar5,"FireRocketThink",0x10);
    iVar2 = *(int *)(&r_visualizetraces.field_0x3e + unaff_EBX);
    iVar3 = iVar2 + 1;
    if (*(int *)(&r_visualizetraces.field_0x36 + unaff_EBX) < iVar3) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)((int)&r_visualizetraces.m_Value.m_nValue + unaff_EBX + 2)
                 ,iVar3 - *(int *)(&r_visualizetraces.field_0x36 + unaff_EBX));
    }
    iVar6 = *(int *)(&r_visualizetraces.field_0x3e + unaff_EBX) + 1;
    *(int *)(&r_visualizetraces.field_0x3e + unaff_EBX) = iVar6;
    iVar8 = *(int *)((int)&r_visualizetraces.m_Value.m_nValue + unaff_EBX + 2);
    *(int *)((int)&r_visualizetraces.m_fMaxVal + unaff_EBX + 2) = iVar8;
    iVar6 = (iVar6 - iVar2) + -1;
    if (0 < iVar6) {
      _V_memmove((void *)(iVar8 + iVar3 * 4),(void *)(iVar8 + iVar2 * 4),iVar6 * 4);
      iVar8 = *(int *)((int)&r_visualizetraces.m_Value.m_nValue + unaff_EBX + 2);
    }
    puVar7 = (undefined4 *)(iVar2 * 4 + iVar8);
    if (puVar7 != (undefined4 *)0x0) {
      *puVar7 = pcVar4;
    }
    *(char **)((int)&PTR_NetworkStateChanged_m_bClientSideRagdoll_00cb26a4 + unaff_EBX + 2) = pcVar4
    ;
    pcVar4 = operator_new___(*(int *)((int)&r_visualizetraces.m_Value.m_fValue + unaff_EBX + 2) +
                             0xd);
    _strcpy(pcVar4,*(char **)((int)&r_visualizetraces.m_Value.m_StringLength + unaff_EBX + 2));
    sVar5 = _strlen(pcVar4);
    builtin_strncpy(pcVar4 + sVar5,"WaitingThink",0xd);
    iVar2 = *(int *)(&r_visualizetraces.field_0x3e + unaff_EBX);
    iVar3 = iVar2 + 1;
    if (*(int *)(&r_visualizetraces.field_0x36 + unaff_EBX) < iVar3) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)((int)&r_visualizetraces.m_Value.m_nValue + unaff_EBX + 2)
                 ,iVar3 - *(int *)(&r_visualizetraces.field_0x36 + unaff_EBX));
    }
    iVar6 = *(int *)(&r_visualizetraces.field_0x3e + unaff_EBX) + 1;
    *(int *)(&r_visualizetraces.field_0x3e + unaff_EBX) = iVar6;
    iVar8 = *(int *)((int)&r_visualizetraces.m_Value.m_nValue + unaff_EBX + 2);
    *(int *)((int)&r_visualizetraces.m_fMaxVal + unaff_EBX + 2) = iVar8;
    iVar6 = (iVar6 - iVar2) + -1;
    if (0 < iVar6) {
      _V_memmove((void *)(iVar8 + iVar3 * 4),(void *)(iVar8 + iVar2 * 4),iVar6 * 4);
      iVar8 = *(int *)((int)&r_visualizetraces.m_Value.m_nValue + unaff_EBX + 2);
    }
    puVar7 = (undefined4 *)(iVar2 * 4 + iVar8);
    if (puVar7 != (undefined4 *)0x0) {
      *puVar7 = pcVar4;
    }
    *(char **)((int)&PTR_GetModelIndex_00cb26e4 + unaff_EBX + 2) = pcVar4;
    ___cxa_guard_release(unaff_EBX + 0xd98102);
  }
  *(undefined4 *)((int)&PTR_IsMoving_00cb21c4 + unaff_EBX + 2) = 0x14;
  *(int *)((int)&PTR_GetDelay_00cb21c0 + unaff_EBX + 2) = unaff_EBX + 0xcb2222;
  return (datamap_t *)((int)&PTR_GetDelay_00cb21c0 + unaff_EBX + 2);
}


/* __static_initialization_and_destruction_0 at 000b8b90 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *s2;
  SendTable *this;
  int *piVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  IEntityFactoryDictionary *pIVar4;
  datamap_t *pdVar5;
  int iVar6;
  int iVar7;
  int unaff_EBX;
  longlong lVar8;
  
  lVar8 = ___i686_get_pc_thunk_bx();
  if (lVar8 == 0xffff00000001) {
    (&DAT_00d97b9c)[unaff_EBX] = 0;
    (&DAT_00d97b9d)[unaff_EBX] = 0;
    (&DAT_00d97b9e)[unaff_EBX] = 0;
    (&DAT_00d97b9f)[unaff_EBX] = 0;
    *(undefined4 *)((int)&LOG_SND_EMITTERSYSTEM + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00d97ba4 + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00d97ba8 + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00d97bac + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00d97bb0 + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00d97bb4 + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00d97bb8 + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00d97bbc + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&sv_soundemitter_version.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&sv_soundemitter_version.super_ConCommandBase.m_pNext + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)(&sv_soundemitter_version.super_ConCommandBase.m_bRegistered + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&sv_soundemitter_version.super_ConCommandBase.m_pszName + unaff_EBX) =
         0x7f7fffff;
    *(undefined **)((int)&sv_soundemitter_version.super_ConCommandBase.m_pszHelpString + unaff_EBX)
         = &UNK_00b93764 + unaff_EBX;
    *(undefined **)((int)&sv_soundemitter_version.super_ConCommandBase.m_nFlags + unaff_EBX) =
         &UNK_00c2014c + unaff_EBX;
    pIVar4 = EntityFactoryDictionary();
    (**pIVar4->_vptr_IEntityFactoryDictionary)(pIVar4,unaff_EBX + 0xd97bd4,unaff_EBX + 0x9fcbd7);
    pdVar5 = DataMapInit<CPropRocketTripwire>((CPropRocketTripwire *)0x0);
    *(datamap_t **)((int)DataMapInit<CAI_BaseActor>::dataDesc[10].flatOffset + unaff_EBX + 4) =
         pdVar5;
    iVar7 = unaff_EBX + 0xd97bd8;
    s2 = (char *)(unaff_EBX + 0x9fcbc3);
    *(char **)((int)&sv_soundemitter_version.super_IConVar._vptr_IConVar + unaff_EBX) = s2;
    this = (SendTable *)((int)DataMapInit<CAI_BaseActor>::dataDesc[10].flatOffset + unaff_EBX + 8);
    *(SendTable **)((int)&sv_soundemitter_version.m_pParent + unaff_EBX) = this;
    *(undefined4 *)((int)&sv_soundemitter_version.m_Value.m_StringLength + unaff_EBX) = 0xffff;
    piVar1 = *(int **)(&DAT_00b8e050 + unaff_EBX);
    puVar2 = (undefined4 *)*piVar1;
    if (puVar2 == (undefined4 *)0x0) {
      *piVar1 = iVar7;
      *(undefined4 *)((int)&sv_soundemitter_version.m_pszDefaultValue + unaff_EBX) = 0;
    }
    else {
      puVar3 = (undefined4 *)puVar2[2];
      iVar6 = _V_stricmp((char *)*puVar2,s2);
      if (iVar6 < 1) {
        while ((puVar3 != (undefined4 *)0x0 && (iVar6 = _V_stricmp((char *)*puVar3,s2), iVar6 < 1)))
        {
          puVar2 = puVar3;
          puVar3 = (undefined4 *)puVar3[2];
        }
        *(undefined4 **)((int)&sv_soundemitter_version.m_pszDefaultValue + unaff_EBX) = puVar3;
        puVar2[2] = iVar7;
      }
      else {
        *(int *)((int)&sv_soundemitter_version.m_pszDefaultValue + unaff_EBX) = *piVar1;
        *piVar1 = iVar7;
      }
    }
    SendTable::SendTable(this);
    ___cxa_atexit(unaff_EBX + 0x987d6c,0,*(undefined4 *)(&DAT_00b8dfb0 + unaff_EBX));
    iVar7 = ServerClassInit<DT_PropRocketTripwire::ignored>((ignored *)0x0);
    *(int *)((int)DataMapInit<CAI_BaseActor>::dataDesc[0xb].flatOffset + unaff_EBX + -0x24) = iVar7;
  }
  return;
}


/* CPropRocketTripwire::GetDataDescMap at 0076b700 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPropRocketTripwire * this) */

datamap_t * __thiscall CPropRocketTripwire::GetDataDescMap(CPropRocketTripwire *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5ff358);
}


/* CPropRocketTripwire::GetBaseMap at 0076b710 */

datamap_t * CPropRocketTripwire::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4db448);
}


/* __tcf_0 at 00a40930 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41007e)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x410072) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41007e));
  }
  *(undefined4 *)(unaff_EBX + 0x41007e) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41007a)) {
    if (*(int *)(unaff_EBX + 0x410072) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20621e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20621e),*(int *)(unaff_EBX + 0x410072));
      *(undefined4 *)(unaff_EBX + 0x410072) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x410076) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x410072);
  *(int *)(unaff_EBX + 0x410082) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41007a)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20621e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20621e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x410072) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x410076) = 0;
  }
  return;
}


/* ServerClassInit<DT_PropRocketTripwire::ignored> at 000b85f0 */

int ServerClassInit<DT_PropRocketTripwire::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&developer.m_fnChangeCallbacks.m_Size + unaff_EBX + 2) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xd98192);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  ((int)&DT_IntervalTimer::g_SendTable.m_nProps + unaff_EBX + 2),
                  (char *)(unaff_EBX + 0x99d636),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)(unaff_EBX + 0xd981f6),(char *)(unaff_EBX + 0x99f9e6),0
                        ,(SendTable *)**(undefined4 **)(&DAT_00b8e5e6 + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00b8e5f2 + unaff_EBX),0x80);
      SendPropInt((SendProp_conflict *)
                  ((int)g_BanMasks[5].super_CBitVecT<CFixedBitVecBase<64>_>.
                        super_CFixedBitVecBase<64>.m_Ints + unaff_EBX + 2),&UNK_009df886 + unaff_EBX
                  ,0x568,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropInt((SendProp_conflict *)
                  ((int)g_BanMasks[0xf].super_CBitVecT<CFixedBitVecBase<64>_>.
                        super_CFixedBitVecBase<64>.m_Ints + unaff_EBX + 6),&UNK_009fd0bd + unaff_EBX
                  ,0x56c,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropBool((SendProp_conflict *)
                   ((int)g_BanMasks[0x1a].super_CBitVecT<CFixedBitVecBase<64>_>.
                         super_CFixedBitVecBase<64>.m_Ints + unaff_EBX + 2),
                   (char *)(unaff_EBX + 0x9fd0d4),0x570,1);
      SendPropEHandle((SendProp_conflict *)
                      ((int)g_SentGameRulesMasks[0].super_CBitVecT<CFixedBitVecBase<64>_>.
                            super_CFixedBitVecBase<64>.m_Ints + unaff_EBX + 6),
                      (char *)(unaff_EBX + 0x9fd0e1),0x574,4,-1,
                      *(SendVarProxyFn_conflict *)(&DAT_00b8e5de + unaff_EBX));
      ___cxa_guard_release(unaff_EBX + 0xd98192);
      ___cxa_atexit(unaff_EBX + 0x988292,0,*(undefined4 *)(&DAT_00b8e556 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)
             ((int)DataMapInit<CAI_BaseActor>::dataDesc[0x21].flatOffset + unaff_EBX + -0x12),
             (SendProp_conflict *)(unaff_EBX + 0xd981f6),5,*(char **)(&DAT_00c206de + unaff_EBX));
  return 1;
}


/* CPropRocketTripwire::GetServerClass at 0076b720 */

/* DWARF original prototype: ServerClass * GetServerClass(CPropRocketTripwire * this) */

ServerClass * __thiscall CPropRocketTripwire::GetServerClass(CPropRocketTripwire *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x6e5054);
}


/* CPropRocketTripwire::YouForgotToImplementOrDeclareServerClass at 0076b730 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CPropRocketTripwire *
   this) */

int __thiscall
CPropRocketTripwire::YouForgotToImplementOrDeclareServerClass(CPropRocketTripwire *this)

{
  return 0;
}


/* __tcf_2 at 00a40910 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x389a88),in_stack_00000008);
  return;
}


/* __tcf_1 at 00a40890 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x4100a7))(unaff_EBX + 0x4100a7);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410053))(unaff_EBX + 0x410053);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40ffff))(unaff_EBX + 0x40ffff);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40ffab))(unaff_EBX + 0x40ffab);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40ff57))(unaff_EBX + 0x40ff57);
                    /* WARNING: Could not recover jumptable at 0x00a40903. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40ff03))();
  return;
}


/* CPropRocketTripwire::CPropRocketTripwire at 0076bf70 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CPropRocketTripwire(CPropRocketTripwire * this) */

void __thiscall CPropRocketTripwire::CPropRocketTripwire(CPropRocketTripwire *this)

{
  undefined1 *puVar1;
  uint uVar2;
  int iVar3;
  ushort uVar4;
  CBaseEdict *this_00;
  int *piVar5;
  ushort *puVar6;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  int unaff_EBX;
  uint uVar9;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::CBaseAnimating(&this->super_CBaseAnimating);
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x56c9a4);
  this->m_flRocketSpeed = 400.0;
  this->m_flRocketLifetime = 15.0;
  this->m_flTimeLastFired = 0.0;
  (this->m_OnTripped).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnTripped).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_OnTripped).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_OnRocketExplode).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnRocketExplode).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->m_OnRocketExplode).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_hPlacementHelper).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hCurRocket).super_CBaseHandle.m_Index = 0xffffffff;
  this->m_bHasRocketOut = false;
  this->m_flRocketTimeOfDeath = 0.0;
  (this->m_bDrawBeams).m_Value = true;
  if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false)
  {
    this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
      this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
      pIVar7 = CBaseEdict::GetChangeAccessor(this_00);
      piVar5 = *(int **)(unaff_EBX + 0x4dabe4);
      puVar6 = (ushort *)*piVar5;
      if (pIVar7->m_iChangeInfoSerialNumber == *puVar6) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        uVar9 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar4 = puVar6[uVar9 * 0x14 + 0x14];
        if (uVar4 == 0) {
LAB_0076c136:
          puVar6[(uint)uVar4 + uVar9 * 0x14 + 1] = 0x570;
          puVar6[uVar9 * 0x14 + 0x14] = uVar4 + 1;
        }
        else if (puVar6[uVar9 * 0x14 + 1] != 0x570) {
          uVar8 = 0;
          do {
            uVar2 = uVar8 + 1;
            uVar8 = uVar2 & 0xffff;
            if ((ushort)uVar2 == uVar4) {
              if (uVar4 != 0x13) goto LAB_0076c136;
              goto LAB_0076c158;
            }
          } while (puVar6[uVar9 * 0x14 + uVar8 + 1] != 0x570);
        }
      }
      else if (puVar6[0x7d1] == 100) {
LAB_0076c158:
        pIVar7->m_iChangeInfoSerialNumber = 0;
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar7->m_iChangeInfo = puVar6[0x7d1];
        *(short *)(*piVar5 + 0xfa2) = *(short *)(*piVar5 + 0xfa2) + 1;
        pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
        iVar3 = *piVar5 + (uint)pIVar7->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar3 + 2) = 0x570;
        *(undefined2 *)(iVar3 + 0x28) = 1;
      }
    }
  }
  else {
    puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  (this->m_hRocket).m_Value.super_CBaseHandle.m_Index = 0xffffffff;
  return;
}


/* CPropRocketTripwire::CPropRocketTripwire at 0076c1a0 */

/* DWARF original prototype: void CPropRocketTripwire(CPropRocketTripwire * this,
   CPropRocketTripwire * this) */

void __thiscall
CPropRocketTripwire::CPropRocketTripwire(CPropRocketTripwire *this,CPropRocketTripwire *this_1)

{
  CPropRocketTripwire(this);
  return;
}


/* CEntityFactory<CPropRocketTripwire>::Create at 0076dc70 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CPropRocketTripwire> * this,
   char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CPropRocketTripwire>::Create
          (CEntityFactory<CPropRocketTripwire> *this,char *pClassName)

{
  CPropRocketTripwire *this_00;
  
                    /* Unresolved local var: CPropRocketTripwire * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x578);
  CPropRocketTripwire::CPropRocketTripwire(this_00);
  (*(this_00->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1d])(this_00,pClassName);
  return &(this_00->super_CBaseAnimating).super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CPropRocketTripwire::~CPropRocketTripwire at 0076b890 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropRocketTripwire(CPropRocketTripwire * this, int __in_chrg) */

void __thiscall CPropRocketTripwire::~CPropRocketTripwire(CPropRocketTripwire *this,int __in_chrg)

{
  uint uVar1;
  int iVar2;
  CBaseEntity *oldObj;
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x56d08b);
  uVar1 = (this->m_hPlacementHelper).super_CBaseHandle.m_Index;
  if ((uVar1 == 0xffffffff) ||
     (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4db2cf),
     *(uint *)(iVar2 + 8) != uVar1 >> 0x10)) {
    oldObj = (CBaseEntity *)0x0;
  }
  else {
    oldObj = *(CBaseEntity **)(iVar2 + 4);
  }
  UTIL_Remove(oldObj);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnRocketExplode).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnTripped).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
  return;
}


/* CPropRocketTripwire::~CPropRocketTripwire at 0076b950 */

/* DWARF original prototype: void ~CPropRocketTripwire(CPropRocketTripwire * this, int __in_chrg,
   CPropRocketTripwire * this) */

void __thiscall
CPropRocketTripwire::~CPropRocketTripwire
          (CPropRocketTripwire *this,int __in_chrg,CPropRocketTripwire *this_1)

{
  ~CPropRocketTripwire(this,__in_chrg);
  return;
}


/* CPropRocketTripwire::~CPropRocketTripwire at 0076ba50 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropRocketTripwire(CPropRocketTripwire * this, int __in_chrg) */

void __thiscall CPropRocketTripwire::~CPropRocketTripwire(CPropRocketTripwire *this,int __in_chrg)

{
  uint uVar1;
  int iVar2;
  CBaseEntity *oldObj;
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x56cecb);
  uVar1 = (this->m_hPlacementHelper).super_CBaseHandle.m_Index;
  if ((uVar1 == 0xffffffff) ||
     (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4db10f),
     *(uint *)(iVar2 + 8) != uVar1 >> 0x10)) {
    oldObj = (CBaseEntity *)0x0;
  }
  else {
    oldObj = *(CBaseEntity **)(iVar2 + 4);
  }
  UTIL_Remove(oldObj);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnRocketExplode).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnTripped).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CPropRocketTripwire::Precache at 0076c1b0 */

/* DWARF original prototype: void Precache(CPropRocketTripwire * this) */

void __thiscall CPropRocketTripwire::Precache(CPropRocketTripwire *this)

{
  undefined1 *puVar1;
  int iVar2;
  ushort uVar3;
  CBaseEdict *this_00;
  ushort *puVar4;
  int *piVar5;
  int iVar6;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  uint uVar9;
  int unaff_EBX;
  uint local_60;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::PrecacheModel(&UNK_00349528 + unaff_EBX);
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x33fe33));
  iVar6 = CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x33e884));
  if (iVar6 != (this->m_nBeamEndpointTexture).m_Value) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
      ;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4da9a4);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar4) {
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_0076c394:
            puVar4[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x56c;
            puVar4[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar8 * 0x14 + 1] != 0x56c) {
            local_60 = 0;
            do {
              uVar9 = local_60 + 1;
              local_60 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 != 0x13) goto LAB_0076c394;
                pIVar7->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar8 * 0x14 + local_60 + 1] != 0x56c);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar7->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4da9a4) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4da9a4) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x4da9a4);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar2 = *piVar5 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x56c;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_nBeamEndpointTexture).m_Value = iVar6;
  }
  UTIL_PrecacheOther((char *)(unaff_EBX + 0x33fe13),(char *)0x0);
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x349549));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x349563));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x349584));
  CBaseAnimating::Precache(&this->super_CBaseAnimating);
  return;
}


/* CPropRocketTripwire::Spawn at 0076d2a0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CPropRocketTripwire * this) */

void __thiscall CPropRocketTripwire::Spawn(CPropRocketTripwire *this)

{
  undefined1 *puVar1;
  int iVar2;
  ushort uVar3;
  CBaseEdict *pCVar4;
  ushort *puVar5;
  BASEPTR func;
  int iVar6;
  CBaseEntity *pCVar7;
  uint *puVar8;
  IChangeInfoAccessor *pIVar9;
  uint uVar10;
  uint uVar11;
  int *piVar12;
  int *piVar13;
  int unaff_EBX;
  uint local_c8;
  uint local_c4;
  uint local_c0;
  undefined1 local_40 [12];
  QAngle local_34;
  Vector local_28 [2];
  
                    /* Unresolved local var: Vector vecMuzzle@[???]
                       Unresolved local var: QAngle angMuzzleDir@[???]
                       Unresolved local var: Vector vecAimDir@[???] */
  ___i686_get_pc_thunk_bx();
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1a])(this);
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1b])(this,unaff_EBX + 0x34843b);
  iVar6 = CBaseAnimating::LookupAttachment
                    (&this->super_CBaseAnimating,(char *)(unaff_EBX + 0x328e11));
  if (iVar6 != (this->m_iMuzzleAttachment).m_Value) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      pCVar4 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar9 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4d98b7);
        if (pIVar9->m_iChangeInfoSerialNumber == *puVar5) {
          uVar10 = (uint)pIVar9->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar10 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_0076d86a:
            puVar5[(uint)uVar3 + uVar10 * 0x14 + 1] = 0x568;
            puVar5[uVar10 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar10 * 0x14 + 1] != 0x568) {
            local_c8 = 0;
            do {
              uVar11 = local_c8 + 1;
              local_c8 = uVar11 & 0xffff;
              if ((ushort)uVar11 == uVar3) {
                if (uVar3 != 0x13) goto LAB_0076d86a;
                pIVar9->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar10 * 0x14 + local_c8 + 1] != 0x568);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar9->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar9->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4d98b7) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4d98b7) + 0xfa2) + 1;
          piVar13 = *(int **)(unaff_EBX + 0x4d98b7);
          pIVar9->m_iChangeInfoSerialNumber = *(ushort *)*piVar13;
          iVar2 = *piVar13 + (uint)pIVar9->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x568;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_iMuzzleAttachment).m_Value = iVar6;
  }
  CBaseAnimating::GetAttachment
            (&this->super_CBaseAnimating,(this->m_iMuzzleAttachment).m_Value,local_28,&local_34);
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0xe9])(local_40,this);
  pCVar7 = CreateEntityByName((char *)(unaff_EBX + 0x3484ff),-1,true);
  if (pCVar7 == (CBaseEntity *)0x0) {
    (this->m_hPlacementHelper).super_CBaseHandle.m_Index = 0xffffffff;
  }
  else {
    puVar8 = (uint *)(*(pCVar7->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                       _vptr_IHandleEntity[3])(pCVar7);
    uVar10 = *puVar8;
    (this->m_hPlacementHelper).super_CBaseHandle.m_Index = uVar10;
    piVar13 = *(int **)(unaff_EBX + 0x4d98bb);
    if (((uVar10 != 0xffffffff) &&
        (iVar6 = (uVar10 & 0xffff) * 0x10 + *piVar13, *(uint *)(iVar6 + 8) == uVar10 >> 0x10)) &&
       (piVar12 = *(int **)(iVar6 + 4), piVar12 != (int *)0x0)) {
      (**(code **)(*piVar12 + 0x80))(piVar12,&UNK_0031f1e3 + unaff_EBX,unaff_EBX + 0x34611b);
      uVar10 = (this->m_hPlacementHelper).super_CBaseHandle.m_Index;
      if ((uVar10 == 0xffffffff) ||
         (iVar6 = (uVar10 & 0xffff) * 0x10 + *piVar13, *(uint *)(iVar6 + 8) != uVar10 >> 0x10)) {
        piVar12 = (int *)0x0;
      }
      else {
        piVar12 = *(int **)(iVar6 + 4);
      }
      (**(code **)(*piVar12 + 0x80))(piVar12,unaff_EBX + 0x34611f,unaff_EBX + 0x2e99cb);
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
      uVar10 = (this->m_hPlacementHelper).super_CBaseHandle.m_Index;
      if ((uVar10 == 0xffffffff) ||
         (iVar6 = (uVar10 & 0xffff) * 0x10 + *piVar13, *(uint *)(iVar6 + 8) != uVar10 >> 0x10)) {
        pCVar7 = (CBaseEntity *)0x0;
      }
      else {
        pCVar7 = *(CBaseEntity **)(iVar6 + 4);
      }
      DispatchSpawn(pCVar7,true);
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
      uVar10 = (this->m_hPlacementHelper).super_CBaseHandle.m_Index;
      if ((uVar10 == 0xffffffff) ||
         (iVar6 = (uVar10 & 0xffff) * 0x10 + *piVar13, *(uint *)(iVar6 + 8) != uVar10 >> 0x10)) {
        piVar13 = (int *)0x0;
      }
      else {
        piVar13 = *(int **)(iVar6 + 4);
      }
      (**(code **)(*piVar13 + 0x94))(piVar13);
    }
  }
  if (this->m_bDisabled == false) {
    this->m_State = ACTIVE;
    if ((this->m_bDrawBeams).m_Value == false) {
      if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
          false) {
        pCVar4 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
          pIVar9 = CBaseEdict::GetChangeAccessor(pCVar4);
          puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4d98b7);
          if (pIVar9->m_iChangeInfoSerialNumber == *puVar5) {
            uVar10 = (uint)pIVar9->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar3 = puVar5[uVar10 * 0x14 + 0x14];
            if (uVar3 == 0) {
LAB_0076d92a:
              puVar5[(uint)uVar3 + uVar10 * 0x14 + 1] = 0x570;
              puVar5[uVar10 * 0x14 + 0x14] = uVar3 + 1;
            }
            else if (puVar5[uVar10 * 0x14 + 1] != 0x570) {
              local_c0 = 0;
              do {
                uVar11 = local_c0 + 1;
                local_c0 = uVar11 & 0xffff;
                if ((ushort)uVar11 == uVar3) {
                  if (uVar3 != 0x13) goto LAB_0076d92a;
                  pIVar9->m_iChangeInfoSerialNumber = 0;
                  pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar5[uVar10 * 0x14 + local_c0 + 1] != 0x570);
            }
          }
          else if (puVar5[0x7d1] == 100) {
            pIVar9->m_iChangeInfoSerialNumber = 0;
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar9->m_iChangeInfo = puVar5[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x4d98b7) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x4d98b7) + 0xfa2) + 1;
            piVar13 = *(int **)(unaff_EBX + 0x4d98b7);
            pIVar9->m_iChangeInfoSerialNumber = *(ushort *)*piVar13;
            iVar6 = *piVar13 + (uint)pIVar9->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar6 + 2) = 0x570;
            *(undefined2 *)(iVar6 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->m_bDrawBeams).m_Value = true;
    }
    this->m_bDisabled = false;
  }
  else {
    this->m_State = DISABLED;
    if ((this->m_bDrawBeams).m_Value != false) {
      if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
          false) {
        pCVar4 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
          pIVar9 = CBaseEdict::GetChangeAccessor(pCVar4);
          puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4d98b7);
          if (pIVar9->m_iChangeInfoSerialNumber == *puVar5) {
            uVar10 = (uint)pIVar9->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar3 = puVar5[uVar10 * 0x14 + 0x14];
            if (uVar3 == 0) {
LAB_0076d9ef:
              puVar5[(uint)uVar3 + uVar10 * 0x14 + 1] = 0x570;
              puVar5[uVar10 * 0x14 + 0x14] = uVar3 + 1;
            }
            else if (puVar5[uVar10 * 0x14 + 1] != 0x570) {
              local_c4 = 0;
              do {
                uVar11 = local_c4 + 1;
                local_c4 = uVar11 & 0xffff;
                if ((ushort)uVar11 == uVar3) {
                  if (uVar3 != 0x13) goto LAB_0076d9ef;
                  pIVar9->m_iChangeInfoSerialNumber = 0;
                  pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar5[uVar10 * 0x14 + local_c4 + 1] != 0x570);
            }
          }
          else if (puVar5[0x7d1] == 100) {
            pIVar9->m_iChangeInfoSerialNumber = 0;
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar9->m_iChangeInfo = puVar5[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x4d98b7) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x4d98b7) + 0xfa2) + 1;
            piVar13 = *(int **)(unaff_EBX + 0x4d98b7);
            pIVar9->m_iChangeInfoSerialNumber = *(ushort *)*piVar13;
            iVar6 = *piVar13 + (uint)pIVar9->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar6 + 2) = 0x570;
            *(undefined2 *)(iVar6 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->m_bDrawBeams).m_Value = false;
    }
    this->m_bDisabled = true;
  }
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x291);
  CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             (float)((double)*(float *)(**(int **)(unaff_EBX + 0x4d98b3) + 0xc) +
                    *(double *)(unaff_EBX + 0x35fd9f)),(char *)0x0);
  return;
}


/* CPropRocketTripwire::EyePosition at 0076b9a0 */

/* DWARF original prototype: Vector EyePosition(CPropRocketTripwire * this) */

Vector * __thiscall
CPropRocketTripwire::EyePosition(Vector *__return_storage_ptr__,CPropRocketTripwire *this)

{
                    /* Unresolved local var: Vector vMuzzlePos@[DW_OP_breg6(ESI): 0]
                       Unresolved local var: Vector vMuzzlePos@[???] */
  CBaseAnimating::GetAttachment
            (&this->super_CBaseAnimating,(this->m_iMuzzleAttachment).m_Value,__return_storage_ptr__,
             (Vector *)0x0,(Vector *)0x0,(Vector *)0x0);
  return __return_storage_ptr__;
}


/* CPropRocketTripwire::EyeDirection at 0076b960 */

/* DWARF original prototype: Vector EyeDirection(CPropRocketTripwire * this) */

Vector * __thiscall
CPropRocketTripwire::EyeDirection(Vector *__return_storage_ptr__,CPropRocketTripwire *this)

{
  Vector local_18;
  
                    /* Unresolved local var: Vector vMuzzleDir@[DW_OP_breg6(ESI): 0]
                       Unresolved local var: Vector vMuzzlePos@[???]
                       Unresolved local var: Vector vMuzzleDir@[???] */
  CBaseAnimating::GetAttachment
            (&this->super_CBaseAnimating,(this->m_iMuzzleAttachment).m_Value,&local_18,
             __return_storage_ptr__,(Vector *)0x0,(Vector *)0x0);
  return __return_storage_ptr__;
}


/* CPropRocketTripwire::UpdateOnRemove at 0076b9e0 */

/* DWARF original prototype: void UpdateOnRemove(CPropRocketTripwire * this) */

void __thiscall CPropRocketTripwire::UpdateOnRemove(CPropRocketTripwire *this)

{
  uint uVar1;
  CBaseEntity *oldObj;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->m_hPlacementHelper).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4db17b),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (oldObj = *(CBaseEntity **)(iVar2 + 4), oldObj != (CBaseEntity *)0x0)) {
    UTIL_Remove(oldObj);
  }
  CBaseEntity::UpdateOnRemove((CBaseEntity *)this);
  return;
}


/* CPropRocketTripwire::WaitingThink at 0076d020 */

/* DWARF original prototype: void WaitingThink(CPropRocketTripwire * this) */

void __thiscall CPropRocketTripwire::WaitingThink(CPropRocketTripwire *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             (float)((double)*(float *)(**(int **)(unaff_EBX + 0x4d9b33) + 0xc) +
                    *(double *)(unaff_EBX + 0x36001f)),(char *)0x0);
  if (this->m_State == ACTIVE) {
    ActiveThink(this);
    return;
  }
  if (this->m_State != FIRING) {
    return;
  }
  PreFiringThink(this);
  return;
}


/* CPropRocketTripwire::ActiveThink at 0076c4a0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ActiveThink(CPropRocketTripwire * this) */

void __thiscall CPropRocketTripwire::ActiveThink(CPropRocketTripwire *this)

{
  BASEPTR func;
  CBaseEntity *pCVar1;
  CPortal_Base2D *pPortal;
  int iVar2;
  CBasePlayer *pCVar3;
  int unaff_EBX;
  float fVar4;
  float fVar5;
  float fVar6;
  trace_t local_130;
  Ray_t local_dc;
  CTraceFilterSimple local_80;
  float local_70;
  float local_6c;
  float local_68;
  float local_64;
  float local_60;
  float local_5c;
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
  ITraceFilter local_28;
  CTraceFilterSimple *local_24;
  float flRequiredParameter;
  
                    /* Unresolved local var: Ray_t rayDmg@[???]
                       Unresolved local var: Vector vForward@[???]
                       Unresolved local var: Vector vEndPoint@[???]
                       Unresolved local var: trace_t traceDmg@[???]
                       Unresolved local var: CTraceFilterSimple subfilter@[???]
                       Unresolved local var: CTraceFilterTranslateClones filter@[???]
                       Unresolved local var: CPortal_Base2D * pFirstPortal@[???] */
  ___i686_get_pc_thunk_bx();
  local_dc.m_pWorldAxisTransform = (matrix3x4_t *)0x0;
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0xe9])(&local_34,this);
                    /* Unresolved local var: Vector res@[???] */
  local_44 = *(float *)(CNPCEventResponseSystemEntity::GetBaseMap + unaff_EBX + 3);
  local_4c = local_34 * local_44;
  local_48 = local_30 * local_44;
  local_44 = local_2c * local_44;
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x88])(&local_40,this);
                    /* Unresolved local var: Vector res@[???] */
  fVar5 = local_40 + local_4c;
  fVar6 = local_3c + local_48;
  fVar4 = local_38 + local_44;
  local_58 = fVar5;
  local_54 = fVar6;
  local_50 = fVar4;
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x88])(&local_64,this);
  local_dc.m_Delta.super_Vector.x = fVar5 - local_64;
  local_dc.m_Delta.super_Vector.y = fVar6 - local_60;
  local_dc.m_Delta.super_Vector.z = fVar4 - local_5c;
  local_dc.m_IsSwept =
       local_dc.m_Delta.super_Vector.x * local_dc.m_Delta.super_Vector.x +
       local_dc.m_Delta.super_Vector.y * local_dc.m_Delta.super_Vector.y +
       local_dc.m_Delta.super_Vector.z * local_dc.m_Delta.super_Vector.z != 0.0;
  local_dc.m_Extents.super_Vector.z = 0.0;
  local_dc.m_Extents.super_Vector.y = 0.0;
  local_dc.m_Extents.super_Vector.x = 0.0;
  local_dc.m_pWorldAxisTransform = (matrix3x4_t *)0x0;
  local_dc.m_StartOffset.super_Vector.z = 0.0;
  local_dc.m_StartOffset.super_Vector.y = 0.0;
  local_dc.m_StartOffset.super_Vector.x = 0.0;
  local_dc.m_Start.super_Vector.x = local_64;
  local_dc.m_Start.super_Vector.y = local_60;
  local_dc.m_Start.super_Vector.z = local_5c;
  local_dc.m_IsRay = true;
  CTraceFilterSimple::CTraceFilterSimple(&local_80,(IHandleEntity *)this,0,(ShouldHitFunc_t)0x0);
  local_28._vptr_ITraceFilter = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4da7af) + 8);
  flRequiredParameter = 2.0;
  local_24 = &local_80;
  pPortal = UTIL_Portal_FirstAlongRay(&local_dc,&flRequiredParameter);
  UTIL_Portal_TraceRay_Bullets(pPortal,&local_dc,0x2006081,&local_28,&local_130,false);
                    /* Unresolved local var: Vector vVel@[???] */
  if ((local_130.m_pEnt != (CBaseEntity *)0x0) &&
     ((iVar2 = (*((local_130.m_pEnt)->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                 _vptr_IHandleEntity[0x55])(local_130.m_pEnt), pCVar1 = local_130.m_pEnt,
      (char)iVar2 != '\0' ||
      ((local_130.m_pEnt != (CBaseEntity *)0x0 &&
       ((pCVar3 = GetPlayerHoldingEntity(local_130.m_pEnt), pCVar3 != (CBasePlayer *)0x0 ||
        (((pCVar1->m_MoveType).m_Value == '\x06' &&
         ((*(pCVar1->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
            _vptr_IHandleEntity[0x92])(pCVar1,&local_70,0),
         fVar4 = local_70 * local_70 + local_6c * local_6c + local_68 * local_68,
         *(float *)(unaff_EBX + 0x360767) <= fVar4 && fVar4 != *(float *)(unaff_EBX + 0x360767))))))
       ))))) {
    if (this->m_State != DISABLED) {
      func.__delta = 0;
      func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x551);
      CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
      CBaseEntity::SetNextThink
                ((CBaseEntity *)this,
                 *(float *)(&DAT_0036182f + unaff_EBX) +
                 *(float *)(**(int **)(unaff_EBX + 0x4da6b3) + 0xc),(char *)0x0);
      CBaseEntity::EmitSound((CBaseEntity *)this,&UNK_0033fb5f + unaff_EBX,0.0,(float *)0x0);
      this->m_State = FIRING;
    }
    COutputEvent::FireOutput(&this->m_OnTripped,(CBaseEntity *)this,local_130.m_pEnt,0.0);
  }
  return;
}


/* CPropRocketTripwire::PrepareToFireRocket at 0076b7e0 */

/* DWARF original prototype: void PrepareToFireRocket(CPropRocketTripwire * this) */

void __thiscall CPropRocketTripwire::PrepareToFireRocket(CPropRocketTripwire *this)

{
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_State != DISABLED) {
    func.__delta = 0;
    func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0x76c);
    CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
    CBaseEntity::SetNextThink
              ((CBaseEntity *)this,
               *(float *)(unaff_EBX + 0x3624ec) + *(float *)(**(int **)(unaff_EBX + 0x4db370) + 0xc)
               ,(char *)0x0);
    CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x34081c),0.0,(float *)0x0);
    this->m_State = FIRING;
  }
  return;
}


/* CPropRocketTripwire::InputForceFire at 0076c3f0 */

/* DWARF original prototype: void InputForceFire(CPropRocketTripwire * this, inputdata_t *
   inputdata) */

void __thiscall
CPropRocketTripwire::InputForceFire(CPropRocketTripwire *this,inputdata_t *inputdata)

{
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_State != DISABLED) {
    func.__delta = 0;
    func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x4a4);
    CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
    CBaseEntity::SetNextThink
              ((CBaseEntity *)this,
               *(float *)(unaff_EBX + 0x3618dc) + *(float *)(**(int **)(unaff_EBX + 0x4da760) + 0xc)
               ,(char *)0x0);
    CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x33fc0c),0.0,(float *)0x0);
    this->m_State = FIRING;
  }
  return;
}


/* CPropRocketTripwire::FireRocketThink at 0076bf60 */

/* DWARF original prototype: void FireRocketThink(CPropRocketTripwire * this) */

void __thiscall CPropRocketTripwire::FireRocketThink(CPropRocketTripwire *this)

{
  FireRocket(this);
  return;
}


/* CPropRocketTripwire::FireRocket at 0076bb10 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void FireRocket(CPropRocketTripwire * this) */

void __thiscall CPropRocketTripwire::FireRocket(CPropRocketTripwire *this)

{
  undefined1 *puVar1;
  ushort uVar2;
  _func_int_varargs *p_Var3;
  int *piVar4;
  CBaseEdict *this_00;
  ushort *puVar5;
  BASEPTR func;
  int iVar6;
  CBaseEntity *pCVar7;
  ulong *puVar8;
  uint *puVar9;
  int iVar10;
  undefined4 *puVar11;
  IChangeInfoAccessor *pIVar12;
  uint uVar13;
  uint uVar14;
  int unaff_EBX;
  uint local_b0;
  Vector local_60;
  Vector local_54;
  QAngle local_48;
  float local_3c;
  float local_38;
  float local_34;
  Vector local_30;
  uint local_24;
  undefined4 local_20 [4];
  
                    /* Unresolved local var: Vector vForward@[???]
                       Unresolved local var: QAngle spawnAngles@[???]
                       Unresolved local var: CRocket_Turret_Projectile * pRocket@[???] */
  ___i686_get_pc_thunk_bx();
  uVar13 = (this->m_hCurRocket).super_CBaseHandle.m_Index;
  if (((uVar13 != 0xffffffff) &&
      (iVar6 = (uVar13 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4db04b),
      *(uint *)(iVar6 + 8) == uVar13 >> 0x10)) &&
     (pCVar7 = *(CBaseEntity **)(iVar6 + 4), pCVar7 != (CBaseEntity *)0x0)) {
    UTIL_RemoveImmediate(pCVar7);
  }
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0xe9])(&local_3c,this);
  local_30.x = local_3c;
  local_30.y = local_38;
  local_30.z = local_34;
  VectorAngles(&local_30,&local_48);
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x88])(&local_54,this);
  pCVar7 = CBaseEntity::Create((char *)(unaff_EBX + 0x3404b6),&local_54,&local_48,
                               (CBaseEntity *)this);
  if (pCVar7 == (CBaseEntity *)0x0) {
    (this->m_hCurRocket).super_CBaseHandle.m_Index = 0xffffffff;
  }
  else {
    puVar8 = (ulong *)(*(pCVar7->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                        _vptr_IHandleEntity[3])(pCVar7);
    (this->m_hCurRocket).super_CBaseHandle.m_Index = *puVar8;
  }
  local_24 = 0xffffffff;
  if (pCVar7 == (CBaseEntity *)0x0) {
    local_24 = 0xffffffff;
  }
  else {
    puVar9 = (uint *)(*(pCVar7->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                       _vptr_IHandleEntity[3])(pCVar7);
    local_24 = *puVar9;
  }
  if ((local_24 == 0xffffffff) ||
     (iVar6 = (local_24 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4db04b),
     *(uint *)(iVar6 + 8) != local_24 >> 0x10)) {
    iVar6 = 0;
  }
  else {
    iVar6 = *(int *)(iVar6 + 4);
  }
  uVar13 = (this->m_hRocket).m_Value.super_CBaseHandle.m_Index;
  if ((uVar13 == 0xffffffff) ||
     (iVar10 = (uVar13 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4db04b),
     *(uint *)(iVar10 + 8) != uVar13 >> 0x10)) {
    iVar10 = 0;
  }
  else {
    iVar10 = *(int *)(iVar10 + 4);
  }
  if (iVar10 != iVar6) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
      ;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar12 = CBaseEdict::GetChangeAccessor(this_00);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4db047);
        if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar13 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar5[uVar13 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_0076bf2a:
            puVar5[(uint)uVar2 + uVar13 * 0x14 + 1] = 0x574;
            puVar5[uVar13 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar5[uVar13 * 0x14 + 1] != 0x574) {
            local_b0 = 0;
            do {
              uVar14 = local_b0 + 1;
              local_b0 = uVar14 & 0xffff;
              if ((ushort)uVar14 == uVar2) {
                if (uVar2 != 0x13) goto LAB_0076bf2a;
                goto LAB_0076be78;
              }
            } while (puVar5[uVar13 * 0x14 + local_b0 + 1] != 0x574);
          }
        }
        else if (puVar5[0x7d1] == 100) {
LAB_0076be78:
          pIVar12->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar12->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4db047) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4db047) + 0xfa2) + 1;
          piVar4 = *(int **)(unaff_EBX + 0x4db047);
          pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar4;
          iVar6 = *piVar4 + (uint)pIVar12->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar6 + 2) = 0x574;
          *(undefined2 *)(iVar6 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_hRocket).m_Value.super_CBaseHandle.m_Index = local_24;
  }
  local_60.z = this->m_flRocketSpeed;
                    /* Unresolved local var: Vector res@[???] */
  local_60.x = local_60.z * local_30.x;
  local_60.y = local_60.z * local_30.y;
  local_60.z = local_60.z * local_30.z;
  CBaseEntity::SetAbsVelocity(pCVar7,&local_60);
  p_Var3 = (pCVar7->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
           _vptr_IHandleEntity[0x157];
  local_20[0] = 0xffffffff;
  puVar11 = (undefined4 *)
            (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.
              super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[3])(this);
  local_20[0] = *puVar11;
  (*p_Var3)(pCVar7,local_20);
  piVar4 = *(int **)(unaff_EBX + 0x4db043);
  this->m_flTimeLastFired = *(float *)(*piVar4 + 0xc);
  this->m_bHasRocketOut = true;
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0x14ff);
  CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             (float)((double)*(float *)(*piVar4 + 0xc) + *(double *)(&DAT_0036152f + unaff_EBX)),
             (char *)0x0);
  return;
}


/* CPropRocketTripwire::IsAlternativeTripwireTarget at 0076b740 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: bool IsAlternativeTripwireTarget(CPropRocketTripwire * this,
   CBaseEntity * pEnt) */

bool __thiscall
CPropRocketTripwire::IsAlternativeTripwireTarget(CPropRocketTripwire *this,CBaseEntity *pEnt)

{
  CBasePlayer *pCVar1;
  bool bVar2;
  int unaff_EBX;
  float fVar3;
  float local_18;
  float local_14;
  float local_10;
  
  ___i686_get_pc_thunk_bx();
  if (pEnt == (CBaseEntity *)0x0) {
    return false;
  }
  pCVar1 = GetPlayerHoldingEntity(pEnt);
  bVar2 = true;
  if (pCVar1 == (CBasePlayer *)0x0) {
    if ((pEnt->m_MoveType).m_Value != '\x06') {
      return false;
    }
    (*(pEnt->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x92]
    )(pEnt,&local_18,0);
    fVar3 = local_18 * local_18 + local_14 * local_14 + local_10 * local_10;
    bVar2 = *(float *)(unaff_EBX + 0x3614c7) <= fVar3 && fVar3 != *(float *)(unaff_EBX + 0x3614c7);
  }
  return bVar2;
}


/* CPropRocketTripwire::PreFiringThink at 0076cbb0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void PreFiringThink(CPropRocketTripwire * this) */

void __thiscall CPropRocketTripwire::PreFiringThink(CPropRocketTripwire *this)

{
  undefined1 *puVar1;
  ushort uVar2;
  CRocket_Turret_Projectile *this_00;
  CBaseEdict *pCVar3;
  ushort *puVar4;
  int *piVar5;
  int iVar6;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  uint uVar9;
  int unaff_EBX;
  float fVar10;
  uint local_74;
  uint local_70;
  
                    /* Unresolved local var: CRocket_Turret_Projectile * pRocket@[???] */
  ___i686_get_pc_thunk_bx();
  uVar8 = (this->m_hCurRocket).super_CBaseHandle.m_Index;
  if (((uVar8 == 0xffffffff) ||
      (iVar6 = (uVar8 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d9fa5),
      *(uint *)(iVar6 + 8) != uVar8 >> 0x10)) ||
     (this_00 = *(CRocket_Turret_Projectile **)(iVar6 + 4),
     this_00 == (CRocket_Turret_Projectile *)0x0)) {
    if (this->m_bHasRocketOut == false) {
      if ((*(float *)(&DAT_003611e1 + unaff_EBX) + this->m_flRocketTimeOfDeath <
           *(float *)(**(int **)(unaff_EBX + 0x4d9f9d) + 0xc)) && (this->m_bDisabled == false)) {
        this->m_State = ACTIVE;
        if ((this->m_bDrawBeams).m_Value == false) {
          if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
              false) {
            pCVar3 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                      super_CBaseEdict;
            if ((pCVar3 != (CBaseEdict *)0x0) && ((pCVar3->m_fStateFlags & 0x100U) == 0)) {
              pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 1;
              pIVar7 = CBaseEdict::GetChangeAccessor(pCVar3);
              puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4d9fa1);
              if (pIVar7->m_iChangeInfoSerialNumber == *puVar4) {
                uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
                uVar2 = puVar4[uVar8 * 0x14 + 0x14];
                if (uVar2 == 0) {
LAB_0076cf12:
                  puVar4[(uint)uVar2 + uVar8 * 0x14 + 1] = 0x570;
                  puVar4[uVar8 * 0x14 + 0x14] = uVar2 + 1;
                }
                else if (puVar4[uVar8 * 0x14 + 1] != 0x570) {
                  local_70 = 0;
                  do {
                    uVar9 = local_70 + 1;
                    local_70 = uVar9 & 0xffff;
                    if ((ushort)uVar9 == uVar2) {
                      if (uVar2 != 0x13) goto LAB_0076cf12;
                      pIVar7->m_iChangeInfoSerialNumber = 0;
                      pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
                      break;
                    }
                  } while (puVar4[uVar8 * 0x14 + local_70 + 1] != 0x570);
                }
              }
              else if (puVar4[0x7d1] == 100) {
                pIVar7->m_iChangeInfoSerialNumber = 0;
                pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
              }
              else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                pIVar7->m_iChangeInfo = puVar4[0x7d1];
                *(short *)(**(int **)(unaff_EBX + 0x4d9fa1) + 0xfa2) =
                     *(short *)(**(int **)(unaff_EBX + 0x4d9fa1) + 0xfa2) + 1;
                piVar5 = *(int **)(unaff_EBX + 0x4d9fa1);
                pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
                iVar6 = *piVar5 + (uint)pIVar7->m_iChangeInfo * 0x28;
                *(undefined2 *)(iVar6 + 2) = 0x570;
                *(undefined2 *)(iVar6 + 0x28) = 1;
              }
            }
          }
          else {
            puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
            *(uint *)puVar1 = *(uint *)puVar1 | 1;
          }
          (this->m_bDrawBeams).m_Value = true;
        }
        CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x348bc9),0.0,(float *)0x0);
      }
    }
    else {
      this->m_flRocketTimeOfDeath = *(float *)(**(int **)(unaff_EBX + 0x4d9f9d) + 0xc);
      this->m_bHasRocketOut = false;
      COutputEvent::FireOutput(&this->m_OnRocketExplode,(CBaseEntity *)this,(CBaseEntity *)this,0.0)
      ;
      if ((this->m_bDrawBeams).m_Value != false) {
        if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
            false) {
          pCVar3 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                    super_CBaseEdict;
          if ((pCVar3 != (CBaseEdict *)0x0) && ((pCVar3->m_fStateFlags & 0x100U) == 0)) {
            pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 1;
            pIVar7 = CBaseEdict::GetChangeAccessor(pCVar3);
            puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4d9fa1);
            if (pIVar7->m_iChangeInfoSerialNumber == *puVar4) {
              uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
              uVar2 = puVar4[uVar8 * 0x14 + 0x14];
              if (uVar2 == 0) {
LAB_0076cfb8:
                puVar4[(uint)uVar2 + uVar8 * 0x14 + 1] = 0x570;
                puVar4[uVar8 * 0x14 + 0x14] = uVar2 + 1;
              }
              else if (puVar4[uVar8 * 0x14 + 1] != 0x570) {
                local_74 = 0;
                do {
                  uVar9 = local_74 + 1;
                  local_74 = uVar9 & 0xffff;
                  if ((ushort)uVar9 == uVar2) {
                    if (uVar2 != 0x13) goto LAB_0076cfb8;
                    pIVar7->m_iChangeInfoSerialNumber = 0;
                    pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
                    break;
                  }
                } while (puVar4[uVar8 * 0x14 + local_74 + 1] != 0x570);
              }
            }
            else if (puVar4[0x7d1] == 100) {
              pIVar7->m_iChangeInfoSerialNumber = 0;
              pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
            }
            else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
              pIVar7->m_iChangeInfo = puVar4[0x7d1];
              *(short *)(**(int **)(unaff_EBX + 0x4d9fa1) + 0xfa2) =
                   *(short *)(**(int **)(unaff_EBX + 0x4d9fa1) + 0xfa2) + 1;
              piVar5 = *(int **)(unaff_EBX + 0x4d9fa1);
              pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
              iVar6 = *piVar5 + (uint)pIVar7->m_iChangeInfo * 0x28;
              *(undefined2 *)(iVar6 + 2) = 0x570;
              *(undefined2 *)(iVar6 + 0x28) = 1;
            }
          }
        }
        else {
          puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (this->m_bDrawBeams).m_Value = false;
      }
      CBaseEntity::EmitSound
                ((CBaseEntity *)this,
                 (char *)(CEntityFactory<CAI_LeadGoal>::GetEntitySize + unaff_EBX + 6),0.0,
                 (float *)0x0);
    }
  }
  else {
    piVar5 = *(int **)(unaff_EBX + 0x4d9f9d);
    fVar10 = *(float *)(*piVar5 + 0xc) - this->m_flTimeLastFired;
    if (this->m_flRocketLifetime <= fVar10 && fVar10 != this->m_flRocketLifetime) {
      CRocket_Turret_Projectile::Explode(this_00);
      this->m_flTimeLastFired = *(float *)(*piVar5 + 0xc);
    }
  }
  return;
}


/* CPropRocketTripwire::InputEnable at 0076d0c0 */

/* DWARF original prototype: void InputEnable(CPropRocketTripwire * this, inputdata_t * inputdata)
    */

void __thiscall CPropRocketTripwire::InputEnable(CPropRocketTripwire *this,inputdata_t *inputdata)

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
  this->m_State = ACTIVE;
  if ((this->m_bDrawBeams).m_Value == false) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
      ;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4d9a94);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_0076d244:
            puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x570;
            puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != 0x570) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_0076d244;
                pIVar6->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar7 * 0x14 + local_60 + 1] != 0x570);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4d9a94) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4d9a94) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x4d9a94);
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar2 = *piVar5 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x570;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bDrawBeams).m_Value = true;
  }
  this->m_bDisabled = false;
  return;
}


/* CPropRocketTripwire::InputDisable at 0076da70 */

/* DWARF original prototype: void InputDisable(CPropRocketTripwire * this, inputdata_t * inputdata)
    */

void __thiscall CPropRocketTripwire::InputDisable(CPropRocketTripwire *this,inputdata_t *inputdata)

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
  this->m_State = DISABLED;
  if ((this->m_bDrawBeams).m_Value != false) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
      ;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4d90e4);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_0076dbf4:
            puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x570;
            puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != 0x570) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_0076dbf4;
                pIVar6->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar7 * 0x14 + local_60 + 1] != 0x570);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4d90e4) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4d90e4) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x4d90e4);
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar2 = *piVar5 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x570;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bDrawBeams).m_Value = false;
  }
  this->m_bDisabled = true;
  return;
}


/* CPropRocketTripwire::EnableTripwire at 0076c9d0 */

/* DWARF original prototype: void EnableTripwire(CPropRocketTripwire * this) */

void __thiscall CPropRocketTripwire::EnableTripwire(CPropRocketTripwire *this)

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
  this->m_State = ACTIVE;
  if ((this->m_bDrawBeams).m_Value == false) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
      ;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)**(undefined4 **)(&DAT_004da184 + unaff_EBX);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_0076cb54:
            puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x570;
            puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != 0x570) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_0076cb54;
                pIVar6->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar7 * 0x14 + local_60 + 1] != 0x570);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(&DAT_004da184 + unaff_EBX) + 0xfa2) =
               *(short *)(**(int **)(&DAT_004da184 + unaff_EBX) + 0xfa2) + 1;
          piVar5 = *(int **)(&DAT_004da184 + unaff_EBX);
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar2 = *piVar5 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x570;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bDrawBeams).m_Value = true;
  }
  this->m_bDisabled = false;
  return;
}


/* CPropRocketTripwire::DisableTripwire at 0076c810 */

/* DWARF original prototype: void DisableTripwire(CPropRocketTripwire * this) */

void __thiscall CPropRocketTripwire::DisableTripwire(CPropRocketTripwire *this)

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
  this->m_State = DISABLED;
  if ((this->m_bDrawBeams).m_Value != false) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
      ;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4da344);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_0076c994:
            puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x570;
            puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != 0x570) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_0076c994;
                pIVar6->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar7 * 0x14 + local_60 + 1] != 0x570);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4da344) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4da344) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x4da344);
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar2 = *piVar5 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x570;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bDrawBeams).m_Value = false;
  }
  this->m_bDisabled = true;
  return;
}


/* _GLOBAL__I__ZN19CPropRocketTripwire9m_DataMapE at 000b8d80 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN19CPropRocketTripwire9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

