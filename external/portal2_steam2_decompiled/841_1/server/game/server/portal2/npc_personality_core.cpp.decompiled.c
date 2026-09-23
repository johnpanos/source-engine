/* DWARF-guided pseudocode for game/server/portal2/npc_personality_core.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000acd10 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *s2;
  int *piVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  datamap_t *pdVar4;
  IEntityFactoryDictionary *pIVar5;
  int iVar6;
  int iVar7;
  int unaff_EBX;
  longlong lVar8;
  ConVar *in_stack_ffffffc8;
  char *in_stack_ffffffcc;
  char *in_stack_ffffffd0;
  int in_stack_ffffffd4;
  SendTable *this;
  char *in_stack_ffffffd8;
  
  lVar8 = ___i686_get_pc_thunk_bx();
  if (lVar8 == 0xffff00000001) {
    *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x233].m_pPrev + unaff_EBX) =
         0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x233].m_pPrev + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x233].m_pPrev + unaff_EBX + 2) = 0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x233].m_pPrev + unaff_EBX + 3) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x233].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x234].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x234].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x234].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x234].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x235].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x235].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x235].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x235].m_pNext + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x236].m_pEntity + unaff_EBX)
         = 0x7f7fffff;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x236].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x236].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x236].m_pNext + unaff_EBX) =
         &UNK_00b9f5e4 + unaff_EBX;
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<HintNodeData>::dataDesc[1].flatOffset + unaff_EBX + 8),
                   (char *)(unaff_EBX + 0xa04a89),&UNK_00a05294 + unaff_EBX,0,
                   (char *)(unaff_EBX + 0xa04a50),in_stack_ffffffc8,in_stack_ffffffcc,
                   in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
    ___cxa_atexit(unaff_EBX + 0x990aec,0,*(undefined4 *)(&DAT_00b99e30 + unaff_EBX));
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<HintNodeData>::dataDesc[3].flatOffset + unaff_EBX + -0x18),
                   (char *)(unaff_EBX + 0xa04ae3),(char *)(unaff_EBX + 0xa04adf),0,
                   (char *)(unaff_EBX + 0xa04aa8),in_stack_ffffffc8,in_stack_ffffffcc,
                   in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
    ___cxa_atexit(unaff_EBX + 0x990acc,0,*(undefined4 *)(&DAT_00b99e30 + unaff_EBX));
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<HintNodeData>::dataDesc[4].flatOffset + unaff_EBX + 8),
                   (char *)(unaff_EBX + 0xa04b3c),(char *)(unaff_EBX + 0xa04b38),0,
                   (char *)(unaff_EBX + 0xa04b00),in_stack_ffffffc8,in_stack_ffffffcc,
                   in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
    ___cxa_atexit(unaff_EBX + 0x990aac,0,*(undefined4 *)(&DAT_00b99e30 + unaff_EBX));
    pdVar4 = DataMapInit<CNPC_PersonalityCore>((CNPC_PersonalityCore *)0x0);
    *(datamap_t **)((int)DataMapInit<HintNodeData>::dataDesc[6].flatOffset + unaff_EBX + -0x20) =
         pdVar4;
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x237].m_pEntity + unaff_EBX)
         = &UNK_00c20b64 + unaff_EBX;
    pIVar5 = EntityFactoryDictionary();
    (**pIVar5->_vptr_IEntityFactoryDictionary)(pIVar5,unaff_EBX + 0xd9ff54,unaff_EBX + 0x9b4a49);
    iVar7 = unaff_EBX + 0xd9ff58;
    s2 = (char *)(unaff_EBX + 0xa04a39);
    *(char **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x237].m_SerialNumber + unaff_EBX)
         = s2;
    this = (SendTable *)((int)DataMapInit<HintNodeData>::dataDesc[6].flatOffset + unaff_EBX + -0x1c)
    ;
    *(SendTable **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x237].m_pPrev + unaff_EBX) =
         this;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x238].m_SerialNumber + unaff_EBX) = 0xffff
    ;
    piVar1 = *(int **)(&DAT_00b99ed0 + unaff_EBX);
    puVar2 = (undefined4 *)*piVar1;
    if (puVar2 == (undefined4 *)0x0) {
      *piVar1 = iVar7;
      *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x237].m_pNext + unaff_EBX)
           = 0;
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
        *(undefined4 **)(iVar7 + 8) = puVar3;
        puVar2[2] = iVar7;
      }
      else {
        *(int *)(iVar7 + 8) = *piVar1;
        *piVar1 = iVar7;
      }
    }
    SendTable::SendTable(this);
    ___cxa_atexit(unaff_EBX + 0x990a8c,0,*(undefined4 *)(&DAT_00b99e30 + unaff_EBX));
    iVar7 = ServerClassInit<DT_NPC_Personality_Core::ignored>((ignored *)0x0);
    *(int *)((int)DataMapInit<HintNodeData>::dataDesc[6].flatOffset + unaff_EBX + -8) = iVar7;
    CAI_LocalIdSpace::CAI_LocalIdSpace
              ((CAI_LocalIdSpace *)
               ((int)DataMapInit<HintNodeData>::dataDesc[7].flatOffset + unaff_EBX + -0x34),false);
    CAI_LocalIdSpace::CAI_LocalIdSpace
              ((CAI_LocalIdSpace *)
               ((int)DataMapInit<HintNodeData>::dataDesc[7].flatOffset + unaff_EBX + -0x1c),false);
    CAI_LocalIdSpace::CAI_LocalIdSpace
              ((CAI_LocalIdSpace *)
               ((int)DataMapInit<HintNodeData>::dataDesc[7].flatOffset + unaff_EBX + -4),false);
    CAI_LocalIdSpace::CAI_LocalIdSpace
              ((CAI_LocalIdSpace *)
               ((int)DataMapInit<HintNodeData>::dataDesc[8].flatOffset + unaff_EBX + -0x2c),false);
  }
  return;
}


/* __tcf_2 at 00a3d810 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38b068),in_stack_00000008);
  return;
}


/* __tcf_3 at 00a3d7f0 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38b0e8),in_stack_00000008);
  return;
}


/* __tcf_4 at 00a3d7d0 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38b168),in_stack_00000008);
  return;
}


/* CNPC_PersonalityCore::CNPC_PersonalityCore at 0072f020 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void CNPC_PersonalityCore(CNPC_PersonalityCore * this) */

void __thiscall CNPC_PersonalityCore::CNPC_PersonalityCore(CNPC_PersonalityCore *this)

{
  vec_t *pvVar1;
  int *piVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CAI_BaseNPC::CAI_BaseNPC((CAI_BaseNPC *)this);
  *(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
           super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
           super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
           super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> =
       unaff_EBX + 0x526e5a;
  (((CAI_DefMovementSink *)
   ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>
          .super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
          super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x8dc))->
  super_IAI_MovementSink)._vptr_IAI_MovementSink = (_func_int_varargs **)(unaff_EBX + 0x5278de);
  ((IAI_BehaviorBridge *)
  ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
         super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
         super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x8e0))->
  _vptr_IAI_BehaviorBridge = (_func_int_varargs **)(unaff_EBX + 0x52790a);
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
  super_CAI_ExpresserSink._vptr_CAI_ExpresserSink = (_func_int_varargs **)(unaff_EBX + 0x5279b6);
  *(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
           super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
           super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
           super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> =
       *(int *)(&DAT_0051824a + unaff_EBX) + 8;
  (((CAI_DefMovementSink *)
   ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>
          .super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
          super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x8dc))->
  super_IAI_MovementSink)._vptr_IAI_MovementSink =
       (_func_int_varargs **)(*(int *)(&DAT_0051824a + unaff_EBX) + 0xad4);
  ((IAI_BehaviorBridge *)
  ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
         super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
         super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x8e0))->
  _vptr_IAI_BehaviorBridge = (_func_int_varargs **)(*(int *)(&DAT_0051824a + unaff_EBX) + 0xb00);
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
  super_CAI_ExpresserSink._vptr_CAI_ExpresserSink =
       (_func_int_varargs **)(*(int *)(&DAT_0051824a + unaff_EBX) + 0xbac);
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_fLatchedPositions = 0;
  pvVar1 = *(vec_t **)(unaff_EBX + 0x517b4a);
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_latchedEyeOrigin.x = *pvVar1;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_latchedEyeOrigin.y = pvVar1[1];
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_latchedEyeOrigin.z = pvVar1[2];
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_latchedEyeDirection.x = *pvVar1;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_latchedEyeDirection.y = pvVar1[1];
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_latchedEyeDirection.z = pvVar1[2];
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_latchedHeadDirection.x = *pvVar1;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_latchedHeadDirection.y = pvVar1[1];
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_latchedHeadDirection.z = pvVar1[2];
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_flBlinktime = 0.0;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_hLookTarget.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_lookQueue.
  super_CUtlVector<CAI_InterestTarget_t,CUtlMemory<CAI_InterestTarget_t,_int>_>.m_Memory.m_pMemory =
       (CAI_InterestTarget_t *)0x0;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_lookQueue.
  super_CUtlVector<CAI_InterestTarget_t,CUtlMemory<CAI_InterestTarget_t,_int>_>.m_Memory.
  m_nAllocationCount = 0;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_lookQueue.
  super_CUtlVector<CAI_InterestTarget_t,CUtlMemory<CAI_InterestTarget_t,_int>_>.m_Memory.m_nGrowSize
       = 0;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_lookQueue.
  super_CUtlVector<CAI_InterestTarget_t,CUtlMemory<CAI_InterestTarget_t,_int>_>.m_Size = 0;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_lookQueue.
  super_CUtlVector<CAI_InterestTarget_t,CUtlMemory<CAI_InterestTarget_t,_int>_>.m_pElements =
       (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_lookQueue.
       super_CUtlVector<CAI_InterestTarget_t,CUtlMemory<CAI_InterestTarget_t,_int>_>.m_Memory.
       m_pMemory;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_syntheticLookQueue.
  super_CUtlVector<CAI_InterestTarget_t,CUtlMemory<CAI_InterestTarget_t,_int>_>.m_Memory.m_pMemory =
       (CAI_InterestTarget_t *)0x0;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_syntheticLookQueue.
  super_CUtlVector<CAI_InterestTarget_t,CUtlMemory<CAI_InterestTarget_t,_int>_>.m_Memory.
  m_nAllocationCount = 0;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_syntheticLookQueue.
  super_CUtlVector<CAI_InterestTarget_t,CUtlMemory<CAI_InterestTarget_t,_int>_>.m_Memory.m_nGrowSize
       = 0;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_syntheticLookQueue.
  super_CUtlVector<CAI_InterestTarget_t,CUtlMemory<CAI_InterestTarget_t,_int>_>.m_Size = 0;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_syntheticLookQueue.
  super_CUtlVector<CAI_InterestTarget_t,CUtlMemory<CAI_InterestTarget_t,_int>_>.m_pElements =
       (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_syntheticLookQueue.
       super_CUtlVector<CAI_InterestTarget_t,CUtlMemory<CAI_InterestTarget_t,_int>_>.m_Memory.
       m_pMemory;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_randomLookQueue.
  super_CUtlVector<CAI_InterestTarget_t,CUtlMemory<CAI_InterestTarget_t,_int>_>.m_Memory.m_pMemory =
       (CAI_InterestTarget_t *)0x0;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_randomLookQueue.
  super_CUtlVector<CAI_InterestTarget_t,CUtlMemory<CAI_InterestTarget_t,_int>_>.m_Memory.
  m_nAllocationCount = 0;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_randomLookQueue.
  super_CUtlVector<CAI_InterestTarget_t,CUtlMemory<CAI_InterestTarget_t,_int>_>.m_Memory.m_nGrowSize
       = 0;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_randomLookQueue.
  super_CUtlVector<CAI_InterestTarget_t,CUtlMemory<CAI_InterestTarget_t,_int>_>.m_Size = 0;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_randomLookQueue.
  super_CUtlVector<CAI_InterestTarget_t,CUtlMemory<CAI_InterestTarget_t,_int>_>.m_pElements =
       (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_randomLookQueue.
       super_CUtlVector<CAI_InterestTarget_t,CUtlMemory<CAI_InterestTarget_t,_int>_>.m_Memory.
       m_pMemory;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_iszExpressionScene.pszValue = (char *)0x0;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_hExpressionSceneEnt.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_iszExpressionOverride.pszValue = (char *)0x0;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_iszIdleExpression.pszValue = (char *)0x0;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_iszAlertExpression.pszValue = (char *)0x0;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_iszCombatExpression.pszValue = (char *)0x0;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_iszDeathExpression.pszValue = (char *)0x0;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_bRemarkablePolling = false;
  _memset((this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_flextarget,0,0x100);
  *(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
           super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
           super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
           super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> =
       *(int *)(unaff_EBX + 0x5184fa) + 8;
  (((CAI_DefMovementSink *)
   ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>
          .super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
          super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x8dc))->
  super_IAI_MovementSink)._vptr_IAI_MovementSink =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x5184fa) + 0xb0c);
  ((IAI_BehaviorBridge *)
  ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
         super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
         super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x8e0))->
  _vptr_IAI_BehaviorBridge = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x5184fa) + 0xb38);
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
  super_CAI_ExpresserSink._vptr_CAI_ExpresserSink =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x5184fa) + 0xbe4);
  ResponseRules::CRR_Response::CRR_Response(&(this->super_CAI_PlayerAlly).m_PendingResponse);
  (this->super_CAI_PlayerAlly).m_PendingConcept._M_dataplus._M_p =
       (char *)(*(int *)(unaff_EBX + 0x518246) + 0xc);
  (this->super_CAI_PlayerAlly).m_hTalkTarget.super_CBaseHandle.m_Index = 0xffffffff;
  (this->super_CAI_PlayerAlly).m_hPotentialSpeechTarget.super_CBaseHandle.m_Index = 0xffffffff;
  (this->super_CAI_PlayerAlly).m_ConceptCategoryTimers[0].m_next = -1.0;
  (this->super_CAI_PlayerAlly).m_ConceptCategoryTimers[1].m_next = -1.0;
  (this->super_CAI_PlayerAlly).m_ConceptCategoryTimers[2].m_next = -1.0;
  (this->super_CAI_PlayerAlly).m_hSpeechFilter.super_CBaseHandle.m_Index = 0xffffffff;
  CAI_PlayerAlly::CScheduleLoader::CScheduleLoader(&(this->super_CAI_PlayerAlly).m_ScheduleLoader);
  *(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
           super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
           super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
           super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> =
       unaff_EBX + 0x59dbda;
  (((CAI_DefMovementSink *)
   ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>
          .super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
          super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x8dc))->
  super_IAI_MovementSink)._vptr_IAI_MovementSink = (_func_int_varargs **)(unaff_EBX + 0x59e6f2);
  ((IAI_BehaviorBridge *)
  ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
         super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
         super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x8e0))->
  _vptr_IAI_BehaviorBridge = (_func_int_varargs **)(unaff_EBX + 0x59e71e);
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
  super_CAI_ExpresserSink._vptr_CAI_ExpresserSink = (_func_int_varargs **)(unaff_EBX + 0x59e7ca);
  (this->super_CDefaultPlayerPickupVPhysics).super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics
       = (_func_int_varargs **)(unaff_EBX + 0x59e7de);
  (this->m_OnPlayerPickup).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnPlayerPickup).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->m_OnPlayerPickup).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_OnPlayerDrop).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnPlayerDrop).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index = 0xffffffff
  ;
  (this->m_OnPlayerDrop).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_hProjectedTexture).super_CBaseHandle.m_Index = 0xffffffff;
  CAI_PlayerAlly::LoadSchedules();
  piVar2 = *(int **)(unaff_EBX + 0x518046);
  if (*(int *)(unaff_EBX + 0x6589aa) != *piVar2) {
    InitCustomSchedules();
    *(undefined1 *)(unaff_EBX + 0x6589a6) = 1;
    *(int *)(unaff_EBX + 0x6589aa) = *piVar2;
  }
  return;
}


/* CEntityFactory<CNPC_PersonalityCore>::Create at 0072f490 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CNPC_PersonalityCore> *
   this, char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CNPC_PersonalityCore>::Create
          (CEntityFactory<CNPC_PersonalityCore> *this,char *pClassName)

{
  CNPC_PersonalityCore *this_00;
  
                    /* Unresolved local var: CNPC_PersonalityCore * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x11cc);
  CNPC_PersonalityCore::CNPC_PersonalityCore(this_00);
  (**(code **)(*(int *)&(this_00->super_CAI_PlayerAlly).super_CAI_BaseActor.
                        super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                        super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                        super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>
              + 0x74))(this_00,pClassName);
  return &((CServerNetworkProperty *)
          ((int)&(this_00->super_CAI_PlayerAlly).super_CAI_BaseActor.
                 super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                 super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                 super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x18)
          )->super_IServerNetworkable;
}


/* CNPC_PersonalityCore::ObjectCaps at 0072efd0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: int ObjectCaps(CNPC_PersonalityCore * this) */

int __thiscall CNPC_PersonalityCore::ObjectCaps(CNPC_PersonalityCore *this)

{
  char cVar1;
  uint uVar2;
  uint uVar3;
  
  uVar2 = CBaseEntity::ObjectCaps((CBaseEntity *)this);
  cVar1 = (**(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                                super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                                super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                                super_CAI_BehaviorHost<CAI_BaseNPC>.
                                super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x114))(this);
  uVar3 = uVar2 | 0x200;
  if (cVar1 != '\0') {
    uVar3 = uVar2 | 0x201;
  }
  return uVar3 | 0x21;
}


/* CNPC_PersonalityCore::HasPreferredCarryAnglesForPlayer at 0072eee0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: bool HasPreferredCarryAnglesForPlayer(CNPC_PersonalityCore * this,
   CBasePlayer * pPlayer) */

bool __thiscall
CNPC_PersonalityCore::HasPreferredCarryAnglesForPlayer
          (CNPC_PersonalityCore *this,CBasePlayer *pPlayer)

{
  return true;
}


/* CNPC_PersonalityCore::CreateExpresser at 0072ef50 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: CAI_Expresser * CreateExpresser(CNPC_PersonalityCore * this) */

CAI_Expresser * __thiscall CNPC_PersonalityCore::CreateExpresser(CNPC_PersonalityCore *this)

{
  CAI_Expresser *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CAI_Expresser * expresser@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = ::operator_new(0x50);
  CAI_Expresser::CAI_Expresser(this_00,(CBaseFlex *)this);
  (this_00->super_IResponseFilter)._vptr_IResponseFilter =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x5183d4) + 8);
  this_00[1].super_IResponseFilter._vptr_IResponseFilter = (_func_int_varargs **)0x0;
  this_00[1].m_pSink = (CAI_ExpresserSink *)((int)&vgui::Panel::GetMessageMap::s_pMap.magic + 3);
  this_00[1].m_ConceptHistories.m_Elements.m_Tree.m_LessFunc.m_LessFunc =
       (_func_bool_char_ptr_ptr_char_ptr_ptr *)0xffffffff;
  this_00->m_pSink =
       &(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
        super_CAI_ExpresserSink;
  return this_00;
}


/* CNPC_PersonalityCore::GetClassScheduleIdSpace at 0072eef0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: CAI_ClassScheduleIdSpace * GetClassScheduleIdSpace(CNPC_PersonalityCore
   * this) */

CAI_ClassScheduleIdSpace * __thiscall
CNPC_PersonalityCore::GetClassScheduleIdSpace(CNPC_PersonalityCore *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (CAI_ClassScheduleIdSpace *)(extraout_ECX + 0x699ac8);
}


/* CNPC_PersonalityCore::GetSchedulingErrorName at 0072ef00 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: char * GetSchedulingErrorName(CNPC_PersonalityCore * this) */

char * __thiscall CNPC_PersonalityCore::GetSchedulingErrorName(CNPC_PersonalityCore *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(char **)(extraout_ECX + 0x636414);
}


/* DataMapInit<CNPC_PersonalityCore> at 000acbe0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */

datamap_t * DataMapInit<CNPC_PersonalityCore>(CNPC_PersonalityCore *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((&UNK_000025cc)[(int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xda01ac);
    if (iVar2 != 0) {
      *(int *)(&UNK_000025dc + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
           unaff_EBX + 0xa04b69;
      *(undefined4 *)
       (&UNK_000025e4 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
      *(undefined4 *)
       (&UNK_000025e8 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
      *(undefined4 *)
       (&UNK_000025ec + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
      *(undefined4 *)
       (&UNK_000025f0 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
      *(undefined4 *)
       (&UNK_000025f4 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
      *(undefined4 *)
       (&UNK_000025e0 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0x14;
      ___cxa_guard_release(unaff_EBX + 0xda01ac);
      ___cxa_atexit(unaff_EBX + 0x990c3c,0,*(undefined4 *)(&DAT_00b99f60 + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_Classify_00cb8710 + unaff_EBX) =
       *(undefined4 *)(&DAT_00b9a92c + unaff_EBX);
  if ((&UNK_000025d4)[(int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xda01b4);
    if (iVar2 != 0) {
      uVar1 = **(undefined4 **)(&DAT_00b99f80 + unaff_EBX);
      *(undefined4 *)((int)&PTR_GetAIAddOn_00cb8a00 + unaff_EBX) = uVar1;
      *(undefined4 *)((int)&PTR_PostClientActive_00cb8a40 + unaff_EBX) = uVar1;
      ___cxa_guard_release(unaff_EBX + 0xda01b4);
    }
  }
  *(undefined4 *)((int)&PTR_GetResponseSystem_00cb8708 + unaff_EBX) = 0x19;
  *(int *)((int)&PTR_GetBaseAnimatingOverlay_00cb8704 + unaff_EBX) =
       (int)&PTR_DecalTrace_00cb876c + unaff_EBX;
  return (datamap_t *)((int)&PTR_GetBaseAnimatingOverlay_00cb8704 + unaff_EBX);
}


/* CNPC_PersonalityCore::GetDataDescMap at 0072baa0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: datamap_t * GetDataDescMap(CNPC_PersonalityCore * this) */

datamap_t * __thiscall CNPC_PersonalityCore::GetDataDescMap(CNPC_PersonalityCore *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x639850);
}


/* CNPC_PersonalityCore::GetBaseMap at 0072bab0 */

datamap_t * CNPC_PersonalityCore::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x51ba68);
}


/* __tcf_0 at 00a3d830 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x40f586)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x40f57a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x40f586));
  }
  *(undefined4 *)(unaff_EBX + 0x40f586) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40f582)) {
    if (*(int *)(unaff_EBX + 0x40f57a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20931e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20931e),*(int *)(unaff_EBX + 0x40f57a));
      *(undefined4 *)(unaff_EBX + 0x40f57a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f57e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40f57a);
  *(int *)(unaff_EBX + 0x40f58a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40f582)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20931e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20931e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40f57a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f57e) = 0;
  }
  return;
}


/* ServerClassInit<DT_NPC_Personality_Core::ignored> at 000aca20 */

int ServerClassInit<DT_NPC_Personality_Core::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x267].m_pPrev + unaff_EBX) ==
      '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xda025c);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x268].m_pPrev + unaff_EBX),
                  (char *)(vgui::Panel::RequestFocusPrev + unaff_EBX),0,4,-1,0,
                  (SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x26d].m_pNext +
                        unaff_EBX),(char *)(unaff_EBX + 0x9ab5b0),0,
                        (SendTable *)**(undefined4 **)(&DAT_00b9abbc + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00b9a1bc + unaff_EBX),0x80);
      SendPropBool((SendProp_conflict *)
                   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x273].m_pEntity + unaff_EBX)
                   ,(char *)(unaff_EBX + 0xa04d11),0x11c0,1);
      ___cxa_guard_release(unaff_EBX + 0xda025c);
      ___cxa_atexit(unaff_EBX + 0x990d2c,0,*(undefined4 *)(&DAT_00b9a120 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)
             ((int)DataMapInit<CAI_LookTarget>::dataDesc[3].flatOffset + unaff_EBX + -0x2c),
             (SendProp_conflict *)
             ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x26d].m_pNext + unaff_EBX),2,
             *(char **)(&DAT_00c20e30 + unaff_EBX));
  return 1;
}


/* CNPC_PersonalityCore::GetServerClass at 0072bac0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: ServerClass * GetServerClass(CNPC_PersonalityCore * this) */

ServerClass * __thiscall CNPC_PersonalityCore::GetServerClass(CNPC_PersonalityCore *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x7211b4);
}


/* CNPC_PersonalityCore::YouForgotToImplementOrDeclareServerClass at 0072bad0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CNPC_PersonalityCore *
   this) */

int __thiscall
CNPC_PersonalityCore::YouForgotToImplementOrDeclareServerClass(CNPC_PersonalityCore *this)

{
  return 0;
}


/* __tcf_5 at 00a3d7b0 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x38b1e4),in_stack_00000008);
  return;
}


/* __tcf_1 at 00a3d760 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f5d7))(unaff_EBX + 0x40f5d7);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f583))(unaff_EBX + 0x40f583);
                    /* WARNING: Could not recover jumptable at 0x00a3d7a7. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f52f))();
  return;
}


/* CNPC_PersonalityCore::~CNPC_PersonalityCore at 0072ea40 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void ~CNPC_PersonalityCore(CNPC_PersonalityCore * this, int __in_chrg)
    */

void __thiscall
CNPC_PersonalityCore::~CNPC_PersonalityCore(CNPC_PersonalityCore *this,int __in_chrg)

{
  allocator *paVar1;
  uint uVar2;
  char *pcVar3;
  CBaseEntity *oldObj;
  int iVar4;
  int unaff_EBX;
  undefined1 *in_stack_ffffffb8;
  undefined1 local_1e [14];
  
  ___i686_get_pc_thunk_bx();
  *(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
           super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
           super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
           super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> =
       unaff_EBX + 0x59e1ba;
  (((CAI_DefMovementSink *)
   ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>
          .super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
          super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x8dc))->
  super_IAI_MovementSink)._vptr_IAI_MovementSink = (_func_int_varargs **)(unaff_EBX + 0x59ecd2);
  ((IAI_BehaviorBridge *)
  ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
         super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
         super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x8e0))->
  _vptr_IAI_BehaviorBridge = (_func_int_varargs **)(unaff_EBX + 0x59ecfe);
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
  super_CAI_ExpresserSink._vptr_CAI_ExpresserSink = (_func_int_varargs **)(unaff_EBX + 0x59edaa);
  (this->super_CDefaultPlayerPickupVPhysics).super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics
       = (_func_int_varargs **)(unaff_EBX + 0x59edbe);
  uVar2 = (this->m_hProjectedTexture).super_CBaseHandle.m_Index;
  if (((uVar2 != 0xffffffff) &&
      (iVar4 = **(int **)(unaff_EBX + 0x51811e) + (uVar2 & 0xffff) * 0x10,
      *(uint *)(iVar4 + 8) == uVar2 >> 0x10)) &&
     (oldObj = *(CBaseEntity **)(iVar4 + 4), oldObj != (CBaseEntity *)0x0)) {
    UTIL_Remove(oldObj);
  }
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnPlayerDrop).super_CBaseEntityOutput,(int)in_stack_ffffffb8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnPlayerPickup).super_CBaseEntityOutput,(int)in_stack_ffffffb8);
  *(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
           super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
           super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
           super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> =
       *(int *)(unaff_EBX + 0x518ada) + 8;
  (((CAI_DefMovementSink *)
   ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>
          .super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
          super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x8dc))->
  super_IAI_MovementSink)._vptr_IAI_MovementSink =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x518ada) + 0xb0c);
  ((IAI_BehaviorBridge *)
  ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
         super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
         super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x8e0))->
  _vptr_IAI_BehaviorBridge = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x518ada) + 0xb38);
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
  super_CAI_ExpresserSink._vptr_CAI_ExpresserSink =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x518ada) + 0xbe4);
  pcVar3 = (this->super_CAI_PlayerAlly).m_PendingConcept._M_dataplus._M_p;
  paVar1 = (allocator *)(pcVar3 + -0xc);
  if (paVar1 != *(allocator **)(unaff_EBX + 0x518826)) {
    in_stack_ffffffb8 = (undefined1 *)0xffffffff;
    iVar4 = __gnu_cxx::__exchange_and_add((int *)(pcVar3 + -4),-1);
    if (iVar4 < 1) {
      in_stack_ffffffb8 = local_1e;
      std::string::_Rep::_M_destroy(paVar1);
    }
  }
  ResponseRules::CRR_Response::~CRR_Response
            (&(this->super_CAI_PlayerAlly).m_PendingResponse,(int)in_stack_ffffffb8);
  CAI_BaseActor::~CAI_BaseActor((CAI_BaseActor *)this,(int)in_stack_ffffffb8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CNPC_PersonalityCore::~CNPC_PersonalityCore at 0072ec80 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void ~CNPC_PersonalityCore(CNPC_PersonalityCore * this, int __in_chrg)
    */

void __thiscall
CNPC_PersonalityCore::~CNPC_PersonalityCore(CNPC_PersonalityCore *this,int __in_chrg)

{
  allocator *paVar1;
  uint uVar2;
  char *pcVar3;
  CBaseEntity *oldObj;
  int iVar4;
  int unaff_EBX;
  undefined1 *in_stack_ffffffb8;
  undefined1 local_1e [14];
  
  ___i686_get_pc_thunk_bx();
  *(undefined **)
   &(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
    super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
    super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> =
       &UNK_0059df7a + unaff_EBX;
  (((CAI_DefMovementSink *)
   ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>
          .super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
          super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x8dc))->
  super_IAI_MovementSink)._vptr_IAI_MovementSink = (_func_int_varargs **)(unaff_EBX + 0x59ea92);
  ((IAI_BehaviorBridge *)
  ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
         super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
         super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x8e0))->
  _vptr_IAI_BehaviorBridge = (_func_int_varargs **)(unaff_EBX + 0x59eabe);
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
  super_CAI_ExpresserSink._vptr_CAI_ExpresserSink = (_func_int_varargs **)(unaff_EBX + 0x59eb6a);
  (this->super_CDefaultPlayerPickupVPhysics).super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics
       = (_func_int_varargs **)(unaff_EBX + 0x59eb7e);
  uVar2 = (this->m_hProjectedTexture).super_CBaseHandle.m_Index;
  if (((uVar2 != 0xffffffff) &&
      (iVar4 = **(int **)(unaff_EBX + 0x517ede) + (uVar2 & 0xffff) * 0x10,
      *(uint *)(iVar4 + 8) == uVar2 >> 0x10)) &&
     (oldObj = *(CBaseEntity **)(iVar4 + 4), oldObj != (CBaseEntity *)0x0)) {
    UTIL_Remove(oldObj);
  }
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnPlayerDrop).super_CBaseEntityOutput,(int)in_stack_ffffffb8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnPlayerPickup).super_CBaseEntityOutput,(int)in_stack_ffffffb8);
  *(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
           super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
           super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
           super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> =
       *(int *)(unaff_EBX + 0x51889a) + 8;
  (((CAI_DefMovementSink *)
   ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>
          .super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
          super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x8dc))->
  super_IAI_MovementSink)._vptr_IAI_MovementSink =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x51889a) + 0xb0c);
  ((IAI_BehaviorBridge *)
  ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
         super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
         super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x8e0))->
  _vptr_IAI_BehaviorBridge = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x51889a) + 0xb38);
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
  super_CAI_ExpresserSink._vptr_CAI_ExpresserSink =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x51889a) + 0xbe4);
  pcVar3 = (this->super_CAI_PlayerAlly).m_PendingConcept._M_dataplus._M_p;
  paVar1 = (allocator *)(pcVar3 + -0xc);
  if (paVar1 != *(allocator **)(unaff_EBX + 0x5185e6)) {
    in_stack_ffffffb8 = (undefined1 *)0xffffffff;
    iVar4 = __gnu_cxx::__exchange_and_add((int *)(pcVar3 + -4),-1);
    if (iVar4 < 1) {
      in_stack_ffffffb8 = local_1e;
      std::string::_Rep::_M_destroy(paVar1);
    }
  }
  ResponseRules::CRR_Response::~CRR_Response
            (&(this->super_CAI_PlayerAlly).m_PendingResponse,(int)in_stack_ffffffb8);
  CAI_BaseActor::~CAI_BaseActor((CAI_BaseActor *)this,(int)in_stack_ffffffb8);
  return;
}


/* CNPC_PersonalityCore::~CNPC_PersonalityCore at 0072eec0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void ~CNPC_PersonalityCore(CNPC_PersonalityCore * this, int __in_chrg,
   CNPC_PersonalityCore * this) */

void __thiscall
CNPC_PersonalityCore::~CNPC_PersonalityCore
          (CNPC_PersonalityCore *this,int __in_chrg,CNPC_PersonalityCore *this_1)

{
  ~CNPC_PersonalityCore(this,__in_chrg);
  return;
}


/* CNPC_PersonalityCore::Precache at 0072c250 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void Precache(CNPC_PersonalityCore * this) */

void __thiscall CNPC_PersonalityCore::Precache(CNPC_PersonalityCore *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x382b08));
  PrecacheInstancedScene((char *)(unaff_EBX + 0x38546c));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x3854d2));
  return;
}


/* CNPC_PersonalityCore::Spawn at 0072d6f0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "MoveCollide_t": Some values do not have unique names */
/* WARNING: Enum "MoveType_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(CNPC_PersonalityCore * this) */

void __thiscall CNPC_PersonalityCore::Spawn(CNPC_PersonalityCore *this)

{
  undefined1 *puVar1;
  NetworkVar_m_Collision *this_00;
  int iVar2;
  ushort uVar3;
  CBaseEdict *pCVar4;
  ushort *puVar5;
  int iVar6;
  CBaseEntity *pCVar7;
  uint *puVar8;
  int iVar9;
  IChangeInfoAccessor *pIVar10;
  uint uVar11;
  uint uVar12;
  int *piVar13;
  uint uVar14;
  int unaff_EBX;
  uint local_c4;
  uint local_c0;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  QAngle local_34;
  Vector local_28 [2];
  
                    /* Unresolved local var: int iAttachmentIndex@[???]
                       Unresolved local var: Vector vecAttachOrigin@[???]
                       Unresolved local var: QAngle vecAttachAngles@[???]
                       Unresolved local var: variant_t emptyVariant@[???] */
  ___i686_get_pc_thunk_bx();
  (**(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                        super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                        super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                        super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>
              + 0x68))(this);
  CAI_BaseNPC::CapabilitiesClear((CAI_BaseNPC *)this);
  (**(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                        super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                        super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                        super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>
              + 0x6c))(this,unaff_EBX + 0x381663);
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
  super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
  super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>.super_CAI_BaseNPC.
  super_CBaseCombatCharacter.m_eHull = HULL_SMALL_CENTERED;
  CAI_BaseNPC::SetHullSizeNormal((CAI_BaseNPC *)this,false);
  CAI_BaseNPC::SetDefaultEyeOffset((CAI_BaseNPC *)this);
  this_00 = &(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
             super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
             super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
             super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>.
             super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.
             super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.m_Collision;
  CCollisionProperty::SetSolid(&this_00->super_CCollisionProperty,SOLID_BBOX);
  CCollisionProperty::SetSolidFlags
            (&this_00->super_CCollisionProperty,
             (this->super_CAI_PlayerAlly).super_CAI_BaseActor.
             super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
             super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
             super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>.
             super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.
             super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.m_Collision.
             super_CCollisionProperty.m_usSolidFlags.m_Value | 0x10);
  CBaseEntity::SetCollisionGroup((CBaseEntity *)this,0);
  CBaseEntity::SetMoveType((CBaseEntity *)this,MOVETYPE_VPHYSICS,MOVECOLLIDE_DEFAULT);
  CAI_BaseNPC::CapabilitiesAdd((CAI_BaseNPC *)this,0x800000);
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
  super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
  super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>.super_CAI_BaseNPC.
  m_NPCState = NPC_STATE_NONE;
  if ((this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
      super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
      super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>.super_CAI_BaseNPC.
      super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
      super_CBaseEntity.m_takedamage.m_Value != '\x01') {
                    /* Unresolved local var: CBaseEntity * pThis@[???] */
    (**(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                          super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                          super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                          super_CAI_BehaviorHost<CAI_BaseNPC>.
                          super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x210))
              (this,&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                     super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                     super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                     super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>.
                     super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.
                     super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.m_takedamage
              );
    (this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
    super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
    super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>.super_CAI_BaseNPC.
    super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
    super_CBaseEntity.m_takedamage.m_Value = '\x01';
  }
  (**(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                        super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                        super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                        super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>
              + 0x1e8))(this,0xfa);
  CBaseCombatCharacter::SetBloodColor((CBaseCombatCharacter *)this,-1);
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
  super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
  super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>.super_CAI_BaseNPC.
  super_CBaseCombatCharacter.m_flFieldOfView = -1.0;
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.m_bRemarkablePolling = true;
  (**(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                        super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                        super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                        super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>
              + 0x55c))(this);
  CBaseAnimating::Spawn((CBaseAnimating *)this);
  CBaseEntity::SetBlocksLOS((CBaseEntity *)this,false);
  this->m_flLastPhysicsImpactTime = 0.0;
  this->m_flAnimResetTime = 0.0;
  this->m_bHasBeenPickedUp = false;
  this->m_bPickupEnabled = true;
  this->m_bAttached = false;
  this->m_iIdleOverrideSequence = -1;
  if ((this->m_bFlashlightEnabled).m_Value != false) {
    if ((this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
        super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
        super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>.
        super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
        super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar4 = &((this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                 super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                 super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                 super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>.
                 super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.
                 super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev
                )->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x519467);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar5) {
          uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar11 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_0072de0a:
            puVar5[(uint)uVar3 + uVar11 * 0x14 + 1] = 0x11c0;
            puVar5[uVar11 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar11 * 0x14 + 1] != 0x11c0) {
            local_c4 = 0;
            do {
              uVar14 = local_c4 + 1;
              local_c4 = uVar14 & 0xffff;
              if ((ushort)uVar14 == uVar3) {
                if (uVar3 != 0x13) goto LAB_0072de0a;
                pIVar10->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar11 * 0x14 + local_c4 + 1] != 0x11c0);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x519467) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x519467) + 0xfa2) + 1;
          piVar13 = *(int **)(unaff_EBX + 0x519467);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar13;
          iVar6 = *piVar13 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar6 + 2) = 0x11c0;
          *(undefined2 *)(iVar6 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>.
                super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.
                super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.m_Network.
                field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bFlashlightEnabled).m_Value = false;
  }
  iVar6 = CBaseAnimating::LookupAttachment((CBaseAnimating *)this,(char *)(unaff_EBX + 0x33a02b));
  CBaseAnimating::GetAttachment((CBaseAnimating *)this,iVar6,local_28,&local_34);
  pCVar7 = CreateEntityByName((char *)(unaff_EBX + 0x360137),-1,true);
  if (pCVar7 == (CBaseEntity *)0x0) {
    (this->m_hProjectedTexture).super_CBaseHandle.m_Index = 0xffffffff;
    piVar13 = (int *)0x0;
  }
  else {
    puVar8 = (uint *)(*(pCVar7->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                       _vptr_IHandleEntity[3])(pCVar7);
    uVar11 = *puVar8;
    (this->m_hProjectedTexture).super_CBaseHandle.m_Index = uVar11;
    if ((uVar11 == 0xffffffff) ||
       (iVar9 = (uVar11 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51946b),
       *(uint *)(iVar9 + 8) != uVar11 >> 0x10)) {
      piVar13 = (int *)0x0;
    }
    else {
      piVar13 = *(int **)(iVar9 + 4);
    }
  }
  (**(code **)(*piVar13 + 0x80))(piVar13,unaff_EBX + 0x384267,unaff_EBX + 0x384260);
  uVar11 = (this->m_hProjectedTexture).super_CBaseHandle.m_Index;
  if ((uVar11 == 0xffffffff) ||
     (iVar9 = (uVar11 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51946b),
     *(uint *)(iVar9 + 8) != uVar11 >> 0x10)) {
    piVar13 = (int *)0x0;
  }
  else {
    piVar13 = *(int **)(iVar9 + 4);
  }
  (**(code **)(*piVar13 + 0x80))(piVar13,unaff_EBX + 0x360329,&UNK_0032957b + unaff_EBX);
  uVar11 = (this->m_hProjectedTexture).super_CBaseHandle.m_Index;
  if ((uVar11 == 0xffffffff) ||
     (iVar9 = (uVar11 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51946b),
     *(uint *)(iVar9 + 8) != uVar11 >> 0x10)) {
    iVar9 = 0;
  }
  else {
    iVar9 = *(int *)(iVar9 + 4);
  }
  uVar14 = *(uint *)(iVar9 + 0x128) | 2;
  if (*(uint *)(iVar9 + 0x128) != uVar14) {
    if (*(char *)(iVar9 + 0x60) == '\0') {
      pCVar4 = *(CBaseEdict **)(iVar9 + 0x24);
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x519467);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar5) {
          uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar11 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_0072dec6:
            puVar5[(uint)uVar3 + uVar11 * 0x14 + 1] = 0x128;
            puVar5[uVar11 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar11 * 0x14 + 1] != 0x128) {
            local_c0 = 0;
            do {
              uVar12 = local_c0 + 1;
              local_c0 = uVar12 & 0xffff;
              if ((ushort)uVar12 == uVar3) {
                if (uVar3 != 0x13) goto LAB_0072dec6;
                pIVar10->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar11 * 0x14 + local_c0 + 1] != 0x128);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar5[0x7d1];
          piVar13 = *(int **)(unaff_EBX + 0x519467);
          *(short *)(*piVar13 + 0xfa2) = *(short *)(*piVar13 + 0xfa2) + 1;
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar13;
          iVar2 = *piVar13 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x128;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      *(uint *)(iVar9 + 100) = *(uint *)(iVar9 + 100) | 1;
    }
    *(uint *)(iVar9 + 0x128) = uVar14;
    uVar11 = (this->m_hProjectedTexture).super_CBaseHandle.m_Index;
  }
  if ((uVar11 == 0xffffffff) ||
     (iVar9 = (uVar11 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51946b),
     *(uint *)(iVar9 + 8) != uVar11 >> 0x10)) {
    pCVar7 = (CBaseEntity *)0x0;
  }
  else {
    pCVar7 = *(CBaseEntity **)(iVar9 + 4);
  }
  DispatchSpawn(pCVar7,true);
  uVar11 = (this->m_hProjectedTexture).super_CBaseHandle.m_Index;
  if ((uVar11 == 0xffffffff) ||
     (iVar9 = (uVar11 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51946b),
     *(uint *)(iVar9 + 8) != uVar11 >> 0x10)) {
    pCVar7 = (CBaseEntity *)0x0;
  }
  else {
    pCVar7 = *(CBaseEntity **)(iVar9 + 4);
  }
  CBaseEntity::SetAbsAngles(pCVar7,&local_34);
  uVar11 = (this->m_hProjectedTexture).super_CBaseHandle.m_Index;
  if ((uVar11 == 0xffffffff) ||
     (iVar9 = (uVar11 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51946b),
     *(uint *)(iVar9 + 8) != uVar11 >> 0x10)) {
    pCVar7 = (CBaseEntity *)0x0;
  }
  else {
    pCVar7 = *(CBaseEntity **)(iVar9 + 4);
  }
  CBaseEntity::SetAbsOrigin(pCVar7,local_28);
  uVar11 = (this->m_hProjectedTexture).super_CBaseHandle.m_Index;
  if ((uVar11 == 0xffffffff) ||
     (iVar9 = (uVar11 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51946b),
     *(uint *)(iVar9 + 8) != uVar11 >> 0x10)) {
    piVar13 = (int *)0x0;
  }
  else {
    piVar13 = *(int **)(iVar9 + 4);
  }
  (**(code **)(*piVar13 + 0x98))(piVar13,this,iVar6);
  uVar11 = (this->m_hProjectedTexture).super_CBaseHandle.m_Index;
  if ((uVar11 == 0xffffffff) ||
     (iVar6 = (uVar11 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51946b),
     *(uint *)(iVar6 + 8) != uVar11 >> 0x10)) {
    piVar13 = (int *)0x0;
  }
  else {
    piVar13 = *(int **)(iVar6 + 4);
  }
  local_48 = 0;
  local_5c = 0;
  local_58 = local_44;
  local_54 = local_40;
  local_50 = 0xffffffff;
  local_4c = 0;
  (**(code **)(*piVar13 + 0xa0))(piVar13,unaff_EBX + 0x34cbc3,this,this,&local_5c,0);
  return;
}


/* CNPC_PersonalityCore::CreateVPhysics at 0072c1b0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "MoveCollide_t": Some values do not have unique names */
/* WARNING: Enum "MoveType_t": Some values do not have unique names */
/* DWARF original prototype: bool CreateVPhysics(CNPC_PersonalityCore * this) */

bool __thiscall CNPC_PersonalityCore::CreateVPhysics(CNPC_PersonalityCore *this)

{
  NetworkVar_m_Collision *this_00;
  int nSolidFlags;
  IPhysicsObject *pIVar1;
  
                    /* Unresolved local var: IPhysicsObject * pPhysicsObject@[???] */
  (**(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                        super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                        super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                        super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>
              + 0x284))(this);
  this_00 = &(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
             super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
             super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
             super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>.
             super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.
             super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.m_Collision;
  CCollisionProperty::SetSolidFlags
            (&this_00->super_CCollisionProperty,
             (this->super_CAI_PlayerAlly).super_CAI_BaseActor.
             super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
             super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
             super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>.
             super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.
             super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.m_Collision.
             super_CCollisionProperty.m_usSolidFlags.m_Value & 0xfffffffb);
  nSolidFlags = (*(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                  super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                  super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                  super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>.
                  super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.
                  super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.m_Collision.
                  super_CCollisionProperty.super_ICollideable._vptr_ICollideable[0xc])(this_00);
  pIVar1 = CBaseEntity::VPhysicsInitNormal
                     ((CBaseEntity *)this,SOLID_VPHYSICS,nSolidFlags,false,(solid_t *)0x0);
  if (pIVar1 != (IPhysicsObject *)0x0) {
    (*pIVar1->_vptr_IPhysicsObject[0x1d])(pIVar1,0x42960000);
    CBaseEntity::SetMoveType((CBaseEntity *)this,MOVETYPE_VPHYSICS,MOVECOLLIDE_DEFAULT);
  }
  return pIVar1 != (IPhysicsObject *)0x0;
}


/* CNPC_PersonalityCore::PreferredCarryAngles at 0072c2a0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: QAngle PreferredCarryAngles(CNPC_PersonalityCore * this) */

QAngle * __thiscall
CNPC_PersonalityCore::PreferredCarryAngles
          (QAngle *__return_storage_ptr__,CNPC_PersonalityCore *this)

{
  vec_t vVar1;
  vec_t vVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  vVar1 = *(vec_t *)(*(int *)(unaff_EBX + 0x69c6af) + 0x2c);
  vVar2 = *(vec_t *)(*(int *)(&DAT_0069c64f + unaff_EBX) + 0x2c);
  __return_storage_ptr__->x = *(vec_t *)(*(int *)(unaff_EBX + 0x69c5ef) + 0x2c);
  __return_storage_ptr__->y = vVar2;
  __return_storage_ptr__->z = vVar1;
  return __return_storage_ptr__;
}


/* CNPC_PersonalityCore::StartSceneEvent at 0072c0d0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: bool StartSceneEvent(CNPC_PersonalityCore * this, CSceneEventInfo *
   info, CChoreoScene * scene, CChoreoEvent * event, CChoreoActor * actor, CBaseEntity * pTarget) */

bool __thiscall
CNPC_PersonalityCore::StartSceneEvent
          (CNPC_PersonalityCore *this,CSceneEventInfo *info,CChoreoScene *scene,CChoreoEvent *event,
          CChoreoActor *actor,CBaseEntity *pTarget)

{
  float fVar1;
  bool bVar2;
  __59 _Var3;
  char *label;
  int iVar4;
  int unaff_EBX;
  float fVar5;
  
  ___i686_get_pc_thunk_bx();
  _Var3 = CChoreoEvent::GetType(event);
  if (_Var3 == SEQUENCE) {
    label = CChoreoEvent::GetParameters(event);
    iVar4 = CBaseAnimating::LookupSequence((CBaseAnimating *)this,label);
    info->m_nSequence = iVar4;
    bVar2 = false;
    if (-1 < iVar4) {
      CAI_BaseNPC::ResetIdealActivity((CAI_BaseNPC *)this,ACT_SPECIFIC_SEQUENCE);
      *(int *)((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                     super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                     super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                     super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> +
              0xa00) = iVar4;
      fVar1 = *(float *)(**(int **)(unaff_EBX + 0x51aa80) + 0xc);
      fVar5 = CChoreoEvent::GetEndTime(event);
      this->m_flAnimResetTime = fVar1 + fVar5;
      bVar2 = true;
    }
  }
  else {
    bVar2 = CAI_BaseActor::StartSceneEvent((CAI_BaseActor *)this,info,scene,event,actor,pTarget);
  }
  return bVar2;
}


/* CNPC_PersonalityCore::PrescheduleThink at 0072ccd0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void PrescheduleThink(CNPC_PersonalityCore * this) */

void __thiscall CNPC_PersonalityCore::PrescheduleThink(CNPC_PersonalityCore *this)

{
  uint *puVar1;
  ushort uVar2;
  CBaseEdict *this_00;
  ushort *puVar3;
  int *piVar4;
  int iVar5;
  IChangeInfoAccessor *pIVar6;
  uint uVar7;
  uint uVar8;
  int unaff_EBX;
  uint local_60;
  
  ___i686_get_pc_thunk_bx();
  if ((*(float *)(**(int **)(unaff_EBX + 0x519e80) + 0xc) <= this->m_flAnimResetTime) ||
     (this->m_flAnimResetTime == 0.0)) goto LAB_0072cd74;
  iVar5 = this->m_iIdleOverrideSequence;
  if (iVar5 < 0) {
    if (this->m_bAttached != false) {
      iVar5 = CBaseAnimating::LookupSequence((CBaseAnimating *)this,(char *)(unaff_EBX + 0x384be7));
      goto LAB_0072cd1b;
    }
    CAI_BaseNPC::SetIdealActivity((CAI_BaseNPC *)this,ACT_IDLE);
  }
  else {
LAB_0072cd1b:
    CAI_BaseNPC::SetIdealActivity((CAI_BaseNPC *)this,ACT_SPECIFIC_SEQUENCE);
    *(int *)((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                   super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                   super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                   super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> +
            0xa00) = iVar5;
  }
  if (((CNetworkVarBase<float,CBaseAnimating::NetworkVar_m_flCycle> *)
      ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
             super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
             super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
             super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x41c))->
      m_Value != 0.0) {
    if (*(bool *)((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                        super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                        super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                        super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>
                 + 0x60) == false) {
      this_00 = *(CBaseEdict **)
                 ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                        super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                        super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                        super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>
                 + 0x24);
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        puVar3 = (ushort *)**(undefined4 **)(unaff_EBX + 0x519e84);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar3) {
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar3[uVar7 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_0072ced1:
            puVar3[(uint)uVar2 + uVar7 * 0x14 + 1] = 0x41c;
            puVar3[uVar7 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar3[uVar7 * 0x14 + 1] != 0x41c) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar2) {
                if (uVar2 != 0x13) goto LAB_0072ced1;
                pIVar6->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar3[uVar7 * 0x14 + local_60 + 1] != 0x41c);
          }
        }
        else if (puVar3[0x7d1] == 100) {
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar3[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x519e84) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x519e84) + 0xfa2) + 1;
          piVar4 = *(int **)(unaff_EBX + 0x519e84);
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar4;
          iVar5 = *piVar4 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar5 + 2) = 0x41c;
          *(undefined2 *)(iVar5 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = (uint *)((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                              super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                              super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                              super_CAI_BehaviorHost<CAI_BaseNPC>.
                              super_CAI_BehaviorHostBase<CAI_BaseNPC> + 100);
      *puVar1 = *puVar1 | 1;
    }
    ((CNetworkVarBase<float,CBaseAnimating::NetworkVar_m_flCycle> *)
    ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
           super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
           super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
           super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x41c))->
    m_Value = 0.0;
  }
  this->m_flAnimResetTime = 0.0;
LAB_0072cd74:
  CAI_PlayerAlly::PrescheduleThink(&this->super_CAI_PlayerAlly);
  return;
}


/* CNPC_PersonalityCore::IsOkToSpeakInResponseToPlayer at 0072c0a0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: bool IsOkToSpeakInResponseToPlayer(CNPC_PersonalityCore * this) */

bool __thiscall CNPC_PersonalityCore::IsOkToSpeakInResponseToPlayer(CNPC_PersonalityCore *this)

{
  bool bVar1;
  
  bVar1 = CAI_PlayerAlly::IsOkToSpeak(&this->super_CAI_PlayerAlly,SPEECH_PRIORITY,true);
  return bVar1;
}


/* CNPC_PersonalityCore::StartTask at 0072bae0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void StartTask(CNPC_PersonalityCore * this, Task_t * pTask) */

void __thiscall CNPC_PersonalityCore::StartTask(CNPC_PersonalityCore *this,Task_t *pTask)

{
  return;
}


/* CNPC_PersonalityCore::RunTask at 0072baf0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void RunTask(CNPC_PersonalityCore * this, Task_t * pTask) */

void __thiscall CNPC_PersonalityCore::RunTask(CNPC_PersonalityCore *this,Task_t *pTask)

{
  return;
}


/* CNPC_PersonalityCore::InputForcePickup at 0072c030 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void InputForcePickup(CNPC_PersonalityCore * this, inputdata_t *
   inputdata) */

void __thiscall
CNPC_PersonalityCore::InputForcePickup(CNPC_PersonalityCore *this,inputdata_t *inputdata)

{
  CBasePlayer *pCVar1;
  
                    /* Unresolved local var: CBasePlayer * pPlayer@[???] */
  pCVar1 = UTIL_GetLocalPlayer();
  if (pCVar1 != (CBasePlayer *)0x0) {
    (**(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                          super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                          super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                          super_CAI_BehaviorHost<CAI_BaseNPC>.
                          super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x98))(this,0,0xffffffff);
    (**(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                          super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                          super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                          super_CAI_BehaviorHost<CAI_BaseNPC>.
                          super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x27c))(this);
    (**(code **)((int)(pCVar1->super_CBaseCombatCharacter).super_CBaseFlex.
                      super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                      super_IServerEntity.super_IServerUnknown + 0x6dc))(pCVar1,this,1);
  }
  return;
}


/* CNPC_PersonalityCore::InputEnablePickup at 0072bb00 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void InputEnablePickup(CNPC_PersonalityCore * this, inputdata_t *
   inputdata) */

void __thiscall
CNPC_PersonalityCore::InputEnablePickup(CNPC_PersonalityCore *this,inputdata_t *inputdata)

{
  this->m_bPickupEnabled = true;
  return;
}


/* CNPC_PersonalityCore::InputDisablePickup at 0072bb10 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void InputDisablePickup(CNPC_PersonalityCore * this, inputdata_t *
   inputdata) */

void __thiscall
CNPC_PersonalityCore::InputDisablePickup(CNPC_PersonalityCore *this,inputdata_t *inputdata)

{
  this->m_bPickupEnabled = false;
  return;
}


/* CNPC_PersonalityCore::InputPlayLock at 0072bfe0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void InputPlayLock(CNPC_PersonalityCore * this, inputdata_t *
   inputdata) */

void __thiscall
CNPC_PersonalityCore::InputPlayLock(CNPC_PersonalityCore *this,inputdata_t *inputdata)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  InstancedScriptedScene
            ((CBaseFlex *)0x0,(char *)(unaff_EBX + 0x385710),(EHANDLE *)0x0,0.0,false,
             (AI_Response *)0x0,false,(IRecipientFilter *)0x0);
  return;
}


/* CNPC_PersonalityCore::InputPlayAttach at 0072bf80 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void InputPlayAttach(CNPC_PersonalityCore * this, inputdata_t *
   inputdata) */

void __thiscall
CNPC_PersonalityCore::InputPlayAttach(CNPC_PersonalityCore *this,inputdata_t *inputdata)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  InstancedScriptedScene
            ((CBaseFlex *)0x0,(char *)(unaff_EBX + 0x38573c),(EHANDLE *)0x0,0.0,false,
             (AI_Response *)0x0,false,(IRecipientFilter *)0x0);
  this->m_bAttached = true;
  return;
}


/* CNPC_PersonalityCore::InputPlayDetach at 0072bb20 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void InputPlayDetach(CNPC_PersonalityCore * this, inputdata_t *
   inputdata) */

void __thiscall
CNPC_PersonalityCore::InputPlayDetach(CNPC_PersonalityCore *this,inputdata_t *inputdata)

{
  this->m_bAttached = false;
  this->m_flAnimResetTime = -1.0;
  return;
}


/* CNPC_PersonalityCore::InputSetIdleSequence at 0072bf20 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void InputSetIdleSequence(CNPC_PersonalityCore * this, inputdata_t *
   inputdata) */

void __thiscall
CNPC_PersonalityCore::InputSetIdleSequence(CNPC_PersonalityCore *this,inputdata_t *inputdata)

{
  char *label;
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((inputdata->value).fieldType == FIELD_STRING) {
    label = (char *)(inputdata->value).field_0.iVal;
    if (label == (char *)0x0) {
      label = (char *)(unaff_EBX + 0x322df3);
    }
  }
  else {
    label = variant_t::ToString(&inputdata->value);
  }
  iVar1 = CBaseAnimating::LookupSequence((CBaseAnimating *)this,label);
  this->m_iIdleOverrideSequence = iVar1;
  this->m_flAnimResetTime = -1.0;
  return;
}


/* CNPC_PersonalityCore::InputClearIdleSequence at 0072bb40 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void InputClearIdleSequence(CNPC_PersonalityCore * this, inputdata_t *
   inputdata) */

void __thiscall
CNPC_PersonalityCore::InputClearIdleSequence(CNPC_PersonalityCore *this,inputdata_t *inputdata)

{
  this->m_iIdleOverrideSequence = -1;
  this->m_flAnimResetTime = -1.0;
  return;
}


/* CNPC_PersonalityCore::InputExplode at 0072cfb0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void InputExplode(CNPC_PersonalityCore * this, inputdata_t * inputdata)
    */

void __thiscall
CNPC_PersonalityCore::InputExplode(CNPC_PersonalityCore *this,inputdata_t *inputdata)

{
  undefined4 *puVar1;
  code *pcVar2;
  Vector *pVVar3;
  int iVar4;
  char *pcVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  int unaff_EBX;
  float thinkTime;
  CRecipientFilter local_60;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  float local_34;
  float local_30;
  float local_2c;
  float local_28;
  float local_24;
  float local_20;
  
                    /* Unresolved local var: CPVSFilter filter@[???]
                       Unresolved local var: Vector gibVelocity@[???]
                       Unresolved local var: int iModelIndex@[???] */
  ___i686_get_pc_thunk_bx();
  pVVar3 = (Vector *)
           (**(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                                 super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                                 super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                                 super_CAI_BehaviorHost<CAI_BaseNPC>.
                                 super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x274))(this);
  ExplosionCreate(pVVar3,*(QAngle **)(unaff_EBX + 0x519c47),(CBaseEntity *)this,100,500,0x529,0.0,
                  (CBaseEntity *)0x0,-1,(EHANDLE *)0x0,CLASS_NONE);
  pVVar3 = (Vector *)
           (**(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                                 super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                                 super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                                 super_CAI_BehaviorHost<CAI_BaseNPC>.
                                 super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x274))(this);
  UTIL_ScreenShake(pVVar3,10.0,150.0,1.0,750.0,SHAKE_START,false,
                   (CUtlVector<CBasePlayer*,CUtlMemory<CBasePlayer*,_int>_> *)0x0);
  pVVar3 = (Vector *)
           (**(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                                 super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                                 super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                                 super_CAI_BehaviorHost<CAI_BaseNPC>.
                                 super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x274))(this);
  CRecipientFilter::CRecipientFilter(&local_60);
  local_60.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(CFuncTank::WorldBarrelPosition + unaff_EBX + 7);
  CRecipientFilter::AddRecipientsByPVS(&local_60,pVVar3);
                    /* Unresolved local var: Vector random@[???] */
  iVar4 = rand();
  local_34 = ((float)iVar4 / *(float *)(unaff_EBX + 0x39fc47)) * *(float *)(unaff_EBX + 0x39fb6b) +
             *(float *)(unaff_EBX + 0x39ff5b);
  iVar4 = rand();
  local_30 = ((float)iVar4 / *(float *)(unaff_EBX + 0x39fc47)) * *(float *)(unaff_EBX + 0x39fb6b) +
             *(float *)(unaff_EBX + 0x39ff5b);
  iVar4 = rand();
  local_2c = ((float)iVar4 / *(float *)(unaff_EBX + 0x39fc47)) * *(float *)(unaff_EBX + 0x39fb6b) +
             *(float *)(unaff_EBX + 0x39ff5b);
  local_28 = local_34;
  local_24 = local_30;
  puVar1 = *(undefined4 **)(unaff_EBX + 0x519baf);
  pcVar2 = *(code **)(*(int *)*puVar1 + 0xc);
  local_20 = local_2c;
  pcVar5 = CPropData::GetRandomChunkModel
                     (*(CPropData **)(unaff_EBX + 0x519ec7),(char *)(unaff_EBX + 0x384923),-1);
  uVar6 = (*pcVar2)(*puVar1,pcVar5);
  iVar4 = 0;
                    /* Unresolved local var: int i@[???] */
  do {
    pcVar2 = *(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x519c4b) + 0x38);
    local_40 = 0x41800000;
    local_3c = 0x41800000;
    local_38 = 0x42900000;
    uVar7 = (**(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                                  super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                                  super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                                  super_CAI_BehaviorHost<CAI_BaseNPC>.
                                  super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x274))(this);
    (*pcVar2)(**(undefined4 **)(unaff_EBX + 0x519c4b),&local_60,0,uVar7,
              *(undefined4 *)(unaff_EBX + 0x519c47),&local_40,&local_28,uVar6,400,1,0x40200000,2);
    iVar4 = iVar4 + 1;
  } while (iVar4 != 0xc);
  thinkTime = *(float *)(unaff_EBX + 0x39f953) + *(float *)(**(int **)(unaff_EBX + 0x519ba3) + 0xc);
  CBaseEntity::SetNextThink((CBaseEntity *)this,thinkTime,(char *)0x0);
  local_60.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(CFuncTank::WorldBarrelPosition + unaff_EBX + 7);
  CRecipientFilter::~CRecipientFilter(&local_60,(int)thinkTime);
  return;
}


/* CNPC_PersonalityCore::InputEnableMotion at 0072cf60 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void InputEnableMotion(CNPC_PersonalityCore * this, inputdata_t *
   inputdata) */

void __thiscall
CNPC_PersonalityCore::InputEnableMotion(CNPC_PersonalityCore *this,inputdata_t *inputdata)

{
  IPhysicsObject *pIVar1;
  
                    /* Unresolved local var: IPhysicsObject * pPhysicsObject@[???] */
  pIVar1 = (this->super_CAI_PlayerAlly).super_CAI_BaseActor.
           super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
           super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
           super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>.
           super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
           super_CBaseAnimating.super_CBaseEntity.m_pPhysicsObject;
  if (pIVar1 != (IPhysicsObject *)0x0) {
    (*pIVar1->_vptr_IPhysicsObject[0x10])(pIVar1,1);
    (*pIVar1->_vptr_IPhysicsObject[0x19])(pIVar1);
  }
                    /* WARNING: Could not recover jumptable at 0x0072cfa6. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                        super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                        super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                        super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>
              + 0x27c))();
  return;
}


/* CNPC_PersonalityCore::EnableMotion at 0072bb60 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void EnableMotion(CNPC_PersonalityCore * this) */

void __thiscall CNPC_PersonalityCore::EnableMotion(CNPC_PersonalityCore *this)

{
  IPhysicsObject *pIVar1;
  
                    /* Unresolved local var: IPhysicsObject * pPhysicsObject@[???] */
  pIVar1 = (this->super_CAI_PlayerAlly).super_CAI_BaseActor.
           super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
           super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
           super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>.
           super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
           super_CBaseAnimating.super_CBaseEntity.m_pPhysicsObject;
  if (pIVar1 != (IPhysicsObject *)0x0) {
    (*pIVar1->_vptr_IPhysicsObject[0x10])(pIVar1,1);
                    /* WARNING: Could not recover jumptable at 0x0072bb92. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*pIVar1->_vptr_IPhysicsObject[0x19])();
    return;
  }
  return;
}


/* CNPC_PersonalityCore::InputDisableMotion at 0072c300 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void InputDisableMotion(CNPC_PersonalityCore * this, inputdata_t *
   inputdata) */

void __thiscall
CNPC_PersonalityCore::InputDisableMotion(CNPC_PersonalityCore *this,inputdata_t *inputdata)

{
  IPhysicsObject *pIVar1;
  
                    /* Unresolved local var: IPhysicsObject * pPhysicsObject@[???] */
  pIVar1 = (this->super_CAI_PlayerAlly).super_CAI_BaseActor.
           super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
           super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
           super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>.
           super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
           super_CBaseAnimating.super_CBaseEntity.m_pPhysicsObject;
  if (pIVar1 != (IPhysicsObject *)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0072c320. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*pIVar1->_vptr_IPhysicsObject[0x10])();
    return;
  }
  return;
}


/* CNPC_PersonalityCore::InputEnableFlashlight at 0072c340 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void InputEnableFlashlight(CNPC_PersonalityCore * this, inputdata_t *
   inputdata) */

void __thiscall
CNPC_PersonalityCore::InputEnableFlashlight(CNPC_PersonalityCore *this,inputdata_t *inputdata)

{
  uint *puVar1;
  ushort uVar2;
  int *piVar3;
  CBaseEdict *this_00;
  ushort *puVar4;
  int iVar5;
  IChangeInfoAccessor *pIVar6;
  uint uVar7;
  uint uVar8;
  int unaff_EBX;
  uint local_90;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  
                    /* Unresolved local var: variant_t emptyVariant@[???] */
  ___i686_get_pc_thunk_bx();
  uVar7 = (this->m_hProjectedTexture).super_CBaseHandle.m_Index;
  if (((uVar7 != 0xffffffff) &&
      (iVar5 = (uVar7 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51a815),
      *(uint *)(iVar5 + 8) == uVar7 >> 0x10)) &&
     (piVar3 = *(int **)(iVar5 + 4), piVar3 != (int *)0x0)) {
    local_30 = 0;
    local_44 = 0;
    local_40 = local_2c;
    local_3c = local_28;
    local_38 = 0xffffffff;
    local_34 = 0;
    (**(code **)(*piVar3 + 0xa0))(piVar3,unaff_EBX + 0x34df65,this,this,&local_44,0);
    if ((this->m_bFlashlightEnabled).m_Value == false) {
      if (*(bool *)((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                          super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                          super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                          super_CAI_BehaviorHost<CAI_BaseNPC>.
                          super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x60) == false) {
        this_00 = *(CBaseEdict **)
                   ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                          super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                          super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                          super_CAI_BehaviorHost<CAI_BaseNPC>.
                          super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x24);
        if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
          this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
          pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
          puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x51a811);
          if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
            uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar2 = puVar4[uVar7 * 0x14 + 0x14];
            if (uVar2 == 0) {
LAB_0072c529:
              puVar4[(uint)uVar2 + uVar7 * 0x14 + 1] = 0x11c0;
              puVar4[uVar7 * 0x14 + 0x14] = uVar2 + 1;
            }
            else if (puVar4[uVar7 * 0x14 + 1] != 0x11c0) {
              local_90 = 0;
              do {
                uVar8 = local_90 + 1;
                local_90 = uVar8 & 0xffff;
                if ((ushort)uVar8 == uVar2) {
                  if (uVar2 != 0x13) goto LAB_0072c529;
                  pIVar6->m_iChangeInfoSerialNumber = 0;
                  this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar4[uVar7 * 0x14 + local_90 + 1] != 0x11c0);
            }
          }
          else if (puVar4[0x7d1] == 100) {
            pIVar6->m_iChangeInfoSerialNumber = 0;
            this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar6->m_iChangeInfo = puVar4[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x51a811) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x51a811) + 0xfa2) + 1;
            piVar3 = *(int **)(unaff_EBX + 0x51a811);
            pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar3;
            iVar5 = *piVar3 + (uint)pIVar6->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar5 + 2) = 0x11c0;
            *(undefined2 *)(iVar5 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = (uint *)((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                                super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                                super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                                super_CAI_BehaviorHost<CAI_BaseNPC>.
                                super_CAI_BehaviorHostBase<CAI_BaseNPC> + 100);
        *puVar1 = *puVar1 | 1;
      }
      (this->m_bFlashlightEnabled).m_Value = true;
    }
  }
  return;
}


/* CNPC_PersonalityCore::InputDisableFlashlight at 0072c8c0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void InputDisableFlashlight(CNPC_PersonalityCore * this, inputdata_t *
   inputdata) */

void __thiscall
CNPC_PersonalityCore::InputDisableFlashlight(CNPC_PersonalityCore *this,inputdata_t *inputdata)

{
  uint *puVar1;
  ushort uVar2;
  CBaseEdict *this_00;
  ushort *puVar3;
  int *piVar4;
  int iVar5;
  IChangeInfoAccessor *pIVar6;
  uint uVar7;
  uint uVar8;
  int unaff_EBX;
  uint local_90;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  
                    /* Unresolved local var: variant_t emptyVariant@[???] */
  ___i686_get_pc_thunk_bx();
  uVar7 = (this->m_hProjectedTexture).super_CBaseHandle.m_Index;
  if (((uVar7 != 0xffffffff) &&
      (iVar5 = (uVar7 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51a295),
      *(uint *)(iVar5 + 8) == uVar7 >> 0x10)) &&
     (piVar4 = *(int **)(iVar5 + 4), piVar4 != (int *)0x0)) {
    local_30 = 0;
    local_44 = 0;
    local_40 = local_2c;
    local_3c = local_28;
    local_38 = 0xffffffff;
    local_34 = 0;
    (**(code **)(*piVar4 + 0xa0))(piVar4,unaff_EBX + 0x34d9ed,this,this,&local_44,0);
  }
  if ((this->m_bFlashlightEnabled).m_Value != false) {
    if (*(bool *)((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                        super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                        super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                        super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>
                 + 0x60) == false) {
      this_00 = *(CBaseEdict **)
                 ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                        super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                        super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                        super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>
                 + 0x24);
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        puVar3 = (ushort *)**(undefined4 **)(unaff_EBX + 0x51a291);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar3) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar3[uVar7 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_0072cacd:
            puVar3[(uint)uVar2 + uVar7 * 0x14 + 1] = 0x11c0;
            puVar3[uVar7 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar3[uVar7 * 0x14 + 1] != 0x11c0) {
            local_90 = 0;
            do {
              uVar8 = local_90 + 1;
              local_90 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar2) {
                if (uVar2 != 0x13) goto LAB_0072cacd;
                pIVar6->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar3[uVar7 * 0x14 + local_90 + 1] != 0x11c0);
          }
        }
        else if (puVar3[0x7d1] == 100) {
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar3[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x51a291) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x51a291) + 0xfa2) + 1;
          piVar4 = *(int **)(unaff_EBX + 0x51a291);
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar4;
          iVar5 = *piVar4 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar5 + 2) = 0x11c0;
          *(undefined2 *)(iVar5 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = (uint *)((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                              super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                              super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                              super_CAI_BehaviorHost<CAI_BaseNPC>.
                              super_CAI_BehaviorHostBase<CAI_BaseNPC> + 100);
      *puVar1 = *puVar1 | 1;
    }
    (this->m_bFlashlightEnabled).m_Value = false;
  }
  return;
}


/* CNPC_PersonalityCore::OnTakeDamage_Alive at 0072bdb0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: int OnTakeDamage_Alive(CNPC_PersonalityCore * this, CTakeDamageInfo *
   inputInfo) */

int __thiscall
CNPC_PersonalityCore::OnTakeDamage_Alive(CNPC_PersonalityCore *this,CTakeDamageInfo *inputInfo)

{
  code *pcVar1;
  int unaff_EBX;
  CRR_Concept local_24 [2];
  undefined4 local_20;
  
                    /* Unresolved local var: CTakeDamageInfo info@[???]
                       Unresolved local var: bool bTookDamage@[???] */
  ___i686_get_pc_thunk_bx();
  if ((inputInfo->m_bitsDamageType & 8) != 0) {
    pcVar1 = *(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                                 super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                                 super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                                 super_CAI_BehaviorHost<CAI_BaseNPC>.
                                 super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0xa70);
    ResponseRules::CRR_Concept::CRR_Concept(local_24,(char *)(unaff_EBX + 0x3858f6));
    local_20 = 0xffffffff;
    (*pcVar1)(this,local_24,0,0,0,0);
  }
  return 1;
}


/* CNPC_PersonalityCore::SelectSchedule at 0072bf10 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: int SelectSchedule(CNPC_PersonalityCore * this) */

int __thiscall CNPC_PersonalityCore::SelectSchedule(CNPC_PersonalityCore *this)

{
  int iVar1;
  
  iVar1 = CAI_PlayerAlly::SelectSchedule(&this->super_CAI_PlayerAlly);
  return iVar1;
}


/* CNPC_PersonalityCore::ShouldIgnite at 0072bbb0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: bool ShouldIgnite(CNPC_PersonalityCore * this, CTakeDamageInfo * info)
    */

bool __thiscall CNPC_PersonalityCore::ShouldIgnite(CNPC_PersonalityCore *this,CTakeDamageInfo *info)

{
  return false;
}


/* CNPC_PersonalityCore::ShouldPlayIdleSound at 0072bbc0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: bool ShouldPlayIdleSound(CNPC_PersonalityCore * this) */

bool __thiscall CNPC_PersonalityCore::ShouldPlayIdleSound(CNPC_PersonalityCore *this)

{
  float fVar1;
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  if (((uint)(*(NPC_STATE *)
               ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                      super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                      super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                      super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> +
               0x9c8) + NPC_STATE_INVALID) < 2) &&
     (fVar1 = *(float *)(**(int **)(extraout_ECX + 0x51af9c) + 0xc),
     this->m_flNextIdleSoundTime <= fVar1 && fVar1 != this->m_flNextIdleSoundTime)) {
    return (((CNetworkVarBase<int,CBaseEntity::NetworkVar_m_spawnflags> *)
            ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                   super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                   super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                   super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> +
            0x128))->m_Value & 2) == 0;
  }
  return false;
}


/* CNPC_PersonalityCore::IdleSound at 0072c640 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void IdleSound(CNPC_PersonalityCore * this) */

void __thiscall CNPC_PersonalityCore::IdleSound(CNPC_PersonalityCore *this)

{
  code *pcVar1;
  int unaff_EBX;
  CRR_Concept local_24 [2];
  undefined4 local_20;
  
  ___i686_get_pc_thunk_bx();
  pcVar1 = *(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                               super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                               super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                               super_CAI_BehaviorHost<CAI_BaseNPC>.
                               super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0xa70);
  ResponseRules::CRR_Concept::CRR_Concept(local_24,(char *)(unaff_EBX + 0x375c58));
  local_20 = 0xffffffff;
  (*pcVar1)(this,local_24,0,0,0,0);
  this->m_flNextIdleSoundTime =
       *(float *)(**(int **)(unaff_EBX + 0x51a510) + 0xc) + *(float *)(unaff_EBX + 0x3a02c0);
  return;
}


/* CNPC_PersonalityCore::HandleAnimEvent at 0072cf20 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void HandleAnimEvent(CNPC_PersonalityCore * this, animevent_t * pEvent)
    */

void __thiscall
CNPC_PersonalityCore::HandleAnimEvent(CNPC_PersonalityCore *this,animevent_t *pEvent)

{
  CAI_BehaviorBase *pCVar1;
  
  pCVar1 = (this->super_CAI_PlayerAlly).super_CAI_BaseActor.
           super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
           super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
           super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>.
           super_CAI_BaseNPC.m_pPrimaryBehavior;
  if (pCVar1 != (CAI_BehaviorBase *)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0072cf3f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*(pCVar1->super_CAI_Component)._vptr_CAI_Component[0x43])();
    return;
  }
  CAI_BaseNPC::HandleAnimEvent((CAI_BaseNPC *)this,pEvent);
  return;
}


/* CNPC_PersonalityCore::TranslateSchedule at 0072bf00 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: int TranslateSchedule(CNPC_PersonalityCore * this, int scheduleType) */

int __thiscall CNPC_PersonalityCore::TranslateSchedule(CNPC_PersonalityCore *this,int scheduleType)

{
  int iVar1;
  
  iVar1 = CAI_PlayerAlly::TranslateSchedule(&this->super_CAI_PlayerAlly,scheduleType);
  return iVar1;
}


/* CNPC_PersonalityCore::GatherConditions at 0072bef0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void GatherConditions(CNPC_PersonalityCore * this) */

void __thiscall CNPC_PersonalityCore::GatherConditions(CNPC_PersonalityCore *this)

{
  CAI_PlayerAlly::GatherConditions(&this->super_CAI_PlayerAlly);
  return;
}


/* CNPC_PersonalityCore::Use at 0072bc10 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void Use(CNPC_PersonalityCore * this, CBaseEntity * pActivator,
   CBaseEntity * pCaller, $_170 useType, float value) */

void __thiscall
CNPC_PersonalityCore::Use
          (CNPC_PersonalityCore *this,CBaseEntity *pActivator,CBaseEntity *pCaller,__170 useType,
          float value)

{
  int iVar1;
  
  if ((this->m_bPickupEnabled != false) && (pActivator != (CBaseEntity *)0x0)) {
    iVar1 = (*(pActivator->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[0x55])(pActivator);
    if ((char)iVar1 != '\0') {
      (**(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                            super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                            super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                            super_CAI_BehaviorHost<CAI_BaseNPC>.
                            super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x98))(this,0,0xffffffff);
      (**(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                            super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                            super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                            super_CAI_BehaviorHost<CAI_BaseNPC>.
                            super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x27c))(this);
                    /* WARNING: Could not recover jumptable at 0x0072bc81. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (*(pActivator->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
        _vptr_IHandleEntity[0x1b7])();
      return;
    }
  }
  return;
}


/* CNPC_PersonalityCore::OnPhysGunPickup at 0072c590 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void OnPhysGunPickup(CNPC_PersonalityCore * this, CBasePlayer *
   pPhysGunUser, PhysGunPickup_t reason) */

void __thiscall
CNPC_PersonalityCore::OnPhysGunPickup
          (CNPC_PersonalityCore *this,CBasePlayer *pPhysGunUser,PhysGunPickup_t reason)

{
  code *pcVar1;
  int unaff_EBX;
  CRR_Concept local_24 [2];
  undefined4 local_20;
  
  ___i686_get_pc_thunk_bx();
  if (reason == PICKED_UP_BY_PLAYER) {
    pcVar1 = *(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                                 super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                                 super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                                 super_CAI_BehaviorHost<CAI_BaseNPC>.
                                 super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0xa70);
    ResponseRules::CRR_Concept::CRR_Concept(local_24,(char *)(unaff_EBX + 0x3852da));
    local_20 = 0xffffffff;
    (*pcVar1)(this,local_24,0,0,0,0);
    this->m_bHasBeenPickedUp = true;
    COutputEvent::FireOutput
              (&this->m_OnPlayerPickup,(CBaseEntity *)pPhysGunUser,(CBaseEntity *)this,0.0);
  }
  return;
}


/* CNPC_PersonalityCore::OnPhysGunDrop at 0072c7e0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void OnPhysGunDrop(CNPC_PersonalityCore * this, CBasePlayer *
   pPhysGunUser, PhysGunDrop_t reason) */

void __thiscall
CNPC_PersonalityCore::OnPhysGunDrop
          (CNPC_PersonalityCore *this,CBasePlayer *pPhysGunUser,PhysGunDrop_t reason)

{
  int unaff_EBX;
  CRR_Concept *this_00;
  code *pcVar1;
  CRR_Concept local_2c [2];
  undefined4 local_28;
  CRR_Concept local_24 [2];
  undefined4 local_20;
  
  ___i686_get_pc_thunk_bx();
  if (reason == DROPPED_BY_PLAYER) {
    pcVar1 = *(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                                 super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                                 super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                                 super_CAI_BehaviorHost<CAI_BaseNPC>.
                                 super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0xa70);
    this_00 = local_24;
    ResponseRules::CRR_Concept::CRR_Concept(this_00,(char *)(unaff_EBX + 0x3850b6));
    local_20 = 0xffffffff;
  }
  else {
    if (reason != THROWN_BY_PLAYER) goto LAB_0072c802;
    pcVar1 = *(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                                 super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                                 super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                                 super_CAI_BehaviorHost<CAI_BaseNPC>.
                                 super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0xa70);
    this_00 = local_2c;
    ResponseRules::CRR_Concept::CRR_Concept(this_00,(char *)(unaff_EBX + 0x3850c6));
    local_28 = 0xffffffff;
  }
  (*pcVar1)(this,this_00,0,0,0,0);
LAB_0072c802:
  COutputEvent::FireOutput
            (&this->m_OnPlayerDrop,(CBaseEntity *)pPhysGunUser,(CBaseEntity *)this,0.0);
  return;
}


/* CNPC_PersonalityCore::OnFizzled at 0072c750 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void OnFizzled(CNPC_PersonalityCore * this) */

void __thiscall CNPC_PersonalityCore::OnFizzled(CNPC_PersonalityCore *this)

{
  code *pcVar1;
  int unaff_EBX;
  CRR_Concept local_24 [2];
  undefined4 local_20;
  
  ___i686_get_pc_thunk_bx();
  pcVar1 = *(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                               super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                               super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                               super_CAI_BehaviorHost<CAI_BaseNPC>.
                               super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0xa70);
  ResponseRules::CRR_Concept::CRR_Concept(local_24,(char *)(unaff_EBX + 0x38513a));
  local_20 = 0xffffffff;
  (*pcVar1)(this,local_24,0,0,0,0);
  CBaseAnimating::OnFizzled((CBaseAnimating *)this);
  return;
}


/* CNPC_PersonalityCore::IsBeingHeldByPlayer at 0072be80 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: bool IsBeingHeldByPlayer(CNPC_PersonalityCore * this) */

bool __thiscall CNPC_PersonalityCore::IsBeingHeldByPlayer(CNPC_PersonalityCore *this)

{
  int *piVar1;
  CBasePlayer *pPlayer;
  CNPC_PersonalityCore *pCVar2;
  int unaff_EBX;
  int playerIndex;
  
  ___i686_get_pc_thunk_bx();
  piVar1 = *(int **)(unaff_EBX + 0x51acd6);
  if (0 < *(int *)(*piVar1 + 0x14)) {
    playerIndex = 1;
    do {
      pPlayer = UTIL_PlayerByIndex(playerIndex);
      if (pPlayer != (CBasePlayer *)0x0) {
        pCVar2 = (CNPC_PersonalityCore *)GetPlayerHeldEntity(pPlayer);
        if (pCVar2 == this) {
          return true;
        }
      }
      playerIndex = playerIndex + 1;
    } while (playerIndex <= *(int *)(*piVar1 + 0x14));
  }
  return false;
}


/* CNPC_PersonalityCore::ModifyOrAppendCriteria at 0072d2b0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ModifyOrAppendCriteria(CNPC_PersonalityCore * this, CriteriaSet *
   set) */

void __thiscall
CNPC_PersonalityCore::ModifyOrAppendCriteria(CNPC_PersonalityCore *this,CriteriaSet *set)

{
  float fVar1;
  float fVar2;
  CBasePlayer *pCVar3;
  CNPC_PersonalityCore *pCVar4;
  char *pcVar5;
  CBaseEntity *pCVar6;
  code *value;
  int unaff_EBX;
  int iVar7;
  float fVar8;
  
  ___i686_get_pc_thunk_bx();
  CAI_PlayerAlly::ModifyOrAppendCriteria(&this->super_CAI_PlayerAlly,set);
  ResponseRules::CriteriaSet::AppendCriteria
            (set,(char *)(unaff_EBX + 0x384632),
             *(char **)(unaff_EBX + 0x638022 + this->m_CoreType * 4),1.0);
  if (0 < *(int *)(**(int **)(unaff_EBX + 0x5198a6) + 0x14)) {
    iVar7 = 1;
    do {
      pCVar3 = UTIL_PlayerByIndex(iVar7);
      if (pCVar3 != (CBasePlayer *)0x0) {
        pCVar4 = (CNPC_PersonalityCore *)GetPlayerHeldEntity(pCVar3);
        if (pCVar4 == this) {
          pcVar5 = (char *)(unaff_EBX + 0x32be60);
          goto LAB_0072d350;
        }
      }
      iVar7 = iVar7 + 1;
    } while (iVar7 <= *(int *)(**(int **)(unaff_EBX + 0x5198a6) + 0x14));
  }
  pcVar5 = (char *)(unaff_EBX + 0x33618a);
LAB_0072d350:
  ResponseRules::CriteriaSet::AppendCriteria(set,(char *)(unaff_EBX + 0x38463f),pcVar5,1.0);
  pcVar5 = (char *)(unaff_EBX + 0x32be60);
  if (this->m_bHasBeenPickedUp == false) {
    pcVar5 = (char *)(unaff_EBX + 0x33618a);
  }
  ResponseRules::CriteriaSet::AppendCriteria(set,(char *)(unaff_EBX + 0x384649),pcVar5,1.0);
  if (0 < *(int *)(**(int **)(unaff_EBX + 0x5198a6) + 0x14)) {
    iVar7 = 1;
    do {
      pCVar3 = UTIL_PlayerByIndex(iVar7);
                    /* Unresolved local var: CBasePlayer * pPlayer@[???] */
      if (pCVar3 != (CBasePlayer *)0x0) {
        if (((pCVar3->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
             super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x1000) != 0) {
          CBaseEntity::CalcAbsoluteVelocity((CBaseEntity *)pCVar3);
        }
        fVar8 = (pCVar3->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                super_CBaseAnimating.super_CBaseEntity.m_vecAbsVelocity.x;
        fVar1 = (pCVar3->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                super_CBaseAnimating.super_CBaseEntity.m_vecAbsVelocity.y;
        fVar2 = (pCVar3->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                super_CBaseAnimating.super_CBaseEntity.m_vecAbsVelocity.z;
                    /* Unresolved local var: __m128 root@[???] */
        fVar8 = SQRT(fVar8 * fVar8 + fVar1 * fVar1 + fVar2 * fVar2);
        goto LAB_0072d42b;
      }
      iVar7 = iVar7 + 1;
    } while (iVar7 <= *(int *)(**(int **)(unaff_EBX + 0x5198a6) + 0x14));
  }
  fVar8 = 0.0;
LAB_0072d42b:
  ResponseRules::CriteriaSet::AppendCriteria(set,(char *)(unaff_EBX + 0x384658),fVar8,1.0);
                    /* Unresolved local var: CBasePlayer * pPlayer@[???] */
  if (0 < *(int *)(**(int **)(unaff_EBX + 0x5198a6) + 0x14)) {
    iVar7 = 1;
    do {
      pCVar3 = UTIL_PlayerByIndex(iVar7);
                    /* Unresolved local var: CBasePlayer * pPlayer@[???] */
      if (pCVar3 != (CBasePlayer *)0x0) {
        pCVar6 = GetPlayerHeldEntity(pCVar3);
                    /* Unresolved local var: CBaseEntity * heldItem@[???] */
        if (pCVar6 != (CBaseEntity *)0x0) {
          value = (code *)(pCVar6->m_iClassname).pszValue;
          if (value == (code *)0x0) {
            value = CAI_BehaviorBase::GetNewSchedule + unaff_EBX + 6;
          }
          goto LAB_0072d4ab;
        }
        break;
      }
      iVar7 = iVar7 + 1;
    } while (iVar7 <= *(int *)(**(int **)(unaff_EBX + 0x5198a6) + 0x14));
  }
  value = *(code **)(unaff_EBX + 0x5a05a2);
LAB_0072d4ab:
  ResponseRules::CriteriaSet::AppendCriteria(set,(char *)(unaff_EBX + 0x384665),(char *)value,1.0);
  return;
}


/* CNPC_PersonalityCore::VPhysicsCollision at 0072d4f0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void VPhysicsCollision(CNPC_PersonalityCore * this, int index,
   gamevcollisionevent_t * pEvent) */

void __thiscall
CNPC_PersonalityCore::VPhysicsCollision
          (CNPC_PersonalityCore *this,int index,gamevcollisionevent_t *pEvent)

{
  float fVar1;
  float fVar2;
  int *piVar3;
  char cVar4;
  CBasePlayer *pPlayer;
  CNPC_PersonalityCore *pCVar5;
  int unaff_EBX;
  int playerIndex;
  CRR_Concept *this_00;
  code *pcVar6;
  float fVar7;
  CRR_Concept local_34 [2];
  undefined4 local_30;
  CRR_Concept local_2c [2];
  undefined4 local_28;
  CRR_Concept local_24 [2];
  undefined4 local_20;
  
                    /* Unresolved local var: CBaseEntity * pHitEntity@[???] */
  ___i686_get_pc_thunk_bx();
  CBaseEntity::VPhysicsCollision((CBaseEntity *)this,index,pEvent);
  if (*(float *)(unaff_EBX + 0x39f416) <=
      *(float *)(**(int **)(unaff_EBX + 0x519666) + 0xc) - this->m_flLastPhysicsImpactTime) {
    piVar3 = *(int **)((int)pEvent->pEntities + (-(uint)(index == 0) & 4));
    if (piVar3 == (int *)**(int **)(&DAT_00519722 + unaff_EBX)) {
      if (0 < *(int *)(**(int **)(unaff_EBX + 0x519666) + 0x14)) {
        playerIndex = 1;
        do {
          pPlayer = UTIL_PlayerByIndex(playerIndex);
          if ((pPlayer != (CBasePlayer *)0x0) &&
             (pCVar5 = (CNPC_PersonalityCore *)GetPlayerHeldEntity(pPlayer), this == pCVar5)) {
            pcVar6 = *(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                                         super_CAI_ExpresserHost<CAI_BaseHumanoid>.
                                         super_CAI_BaseHumanoid.
                                         super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                                         super_CAI_BehaviorHost<CAI_BaseNPC>.
                                         super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0xa70);
            this_00 = local_24;
            ResponseRules::CRR_Concept::CRR_Concept(this_00,(char *)(unaff_EBX + 0x384436));
            local_20 = 0xffffffff;
            goto LAB_0072d61e;
          }
          playerIndex = playerIndex + 1;
        } while (playerIndex <= *(int *)(**(int **)(unaff_EBX + 0x519666) + 0x14));
      }
      fVar7 = pEvent->preVelocity[index].z;
      fVar1 = pEvent->preVelocity[index].y;
      fVar2 = pEvent->preVelocity[index].x;
      fVar7 = fVar2 * fVar2 + fVar1 * fVar1 + fVar7 * fVar7;
      if (*(float *)(unaff_EBX + 0x39fa92) <= fVar7 && fVar7 != *(float *)(unaff_EBX + 0x39fa92)) {
        pcVar6 = *(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                                     super_CAI_ExpresserHost<CAI_BaseHumanoid>.
                                     super_CAI_BaseHumanoid.
                                     super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                                     super_CAI_BehaviorHost<CAI_BaseNPC>.
                                     super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0xa70);
        ResponseRules::CRR_Concept::CRR_Concept(local_2c,(char *)(unaff_EBX + 0x38444e));
        local_28 = 0xffffffff;
        (*pcVar6)(this,local_2c,0,0,0,0);
        this->m_flLastPhysicsImpactTime = *(float *)(**(int **)(unaff_EBX + 0x519666) + 0xc);
      }
    }
    else {
      cVar4 = (**(code **)(*piVar3 + 0x154))(piVar3);
      if (cVar4 == '\0') {
        pcVar6 = *(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                                     super_CAI_ExpresserHost<CAI_BaseHumanoid>.
                                     super_CAI_BaseHumanoid.
                                     super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                                     super_CAI_BehaviorHost<CAI_BaseNPC>.
                                     super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0xa70);
        this_00 = local_34;
        ResponseRules::CRR_Concept::CRR_Concept(this_00,(char *)(unaff_EBX + 0x38444e));
        local_30 = 0xffffffff;
LAB_0072d61e:
        (*pcVar6)(this,this_00,0,0,0,0);
        this->m_flLastPhysicsImpactTime = *(float *)(**(int **)(unaff_EBX + 0x519666) + 0xc);
        return;
      }
    }
  }
  return;
}


/* CNPC_PersonalityCore::GetFirstPlayer at 0072be30 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: CBasePlayer * GetFirstPlayer(CNPC_PersonalityCore * this) */

CBasePlayer * __thiscall CNPC_PersonalityCore::GetFirstPlayer(CNPC_PersonalityCore *this)

{
  int *piVar1;
  CBasePlayer *pCVar2;
  int unaff_EBX;
  int playerIndex;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  piVar1 = *(int **)(unaff_EBX + 0x51ad26);
  if (0 < *(int *)(*piVar1 + 0x14)) {
    playerIndex = 1;
    do {
                    /* Unresolved local var: CBasePlayer * pPlayer@[???] */
      pCVar2 = UTIL_PlayerByIndex(playerIndex);
      if (pCVar2 != (CBasePlayer *)0x0) {
        return pCVar2;
      }
      playerIndex = playerIndex + 1;
    } while (playerIndex <= *(int *)(*piVar1 + 0x14));
  }
  return (CBasePlayer *)0x0;
}


/* CNPC_PersonalityCore::GetPlayerHeldEntityName at 0072cb10 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: char * GetPlayerHeldEntityName(CNPC_PersonalityCore * this) */

char * __thiscall CNPC_PersonalityCore::GetPlayerHeldEntityName(CNPC_PersonalityCore *this)

{
  int *piVar1;
  char *pcVar2;
  CBasePlayer *pPlayer;
  CBaseEntity *pCVar3;
  int unaff_EBX;
  int playerIndex;
  
                    /* Unresolved local var: CBasePlayer * pPlayer@[???] */
  ___i686_get_pc_thunk_bx();
  piVar1 = *(int **)(unaff_EBX + 0x51a046);
  if (0 < *(int *)(*piVar1 + 0x14)) {
    playerIndex = 1;
    do {
      pPlayer = UTIL_PlayerByIndex(playerIndex);
                    /* Unresolved local var: CBasePlayer * pPlayer@[???] */
      if (pPlayer != (CBasePlayer *)0x0) {
                    /* Unresolved local var: CBaseEntity * heldItem@[???] */
        pCVar3 = GetPlayerHeldEntity(pPlayer);
        if (pCVar3 != (CBaseEntity *)0x0) {
          pcVar2 = (pCVar3->m_iClassname).pszValue;
          if (pcVar2 != (char *)0x0) {
            return pcVar2;
          }
          return (char *)(unaff_EBX + 0x322206);
        }
        break;
      }
      playerIndex = playerIndex + 1;
    } while (playerIndex <= *(int *)(*piVar1 + 0x14));
  }
  return *(char **)(unaff_EBX + 0x5a0d42);
}


/* CNPC_PersonalityCore::GetPlayerSpeed at 0072cb80 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: float GetPlayerSpeed(CNPC_PersonalityCore * this) */

float __thiscall CNPC_PersonalityCore::GetPlayerSpeed(CNPC_PersonalityCore *this)

{
  float fVar1;
  float fVar2;
  float fVar3;
  CBasePlayer *this_00;
  int unaff_EBX;
  int playerIndex;
  
                    /* Unresolved local var: CBasePlayer * pPlayer@[???] */
  ___i686_get_pc_thunk_bx();
  if (0 < *(int *)(**(int **)(unaff_EBX + 0x519fd6) + 0x14)) {
    playerIndex = 1;
    do {
      this_00 = UTIL_PlayerByIndex(playerIndex);
                    /* Unresolved local var: CBasePlayer * pPlayer@[???] */
      if (this_00 != (CBasePlayer *)0x0) {
        if (((this_00->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
             super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x1000) != 0) {
          CBaseEntity::CalcAbsoluteVelocity((CBaseEntity *)this_00);
        }
        fVar1 = (this_00->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                super_CBaseAnimating.super_CBaseEntity.m_vecAbsVelocity.x;
        fVar2 = (this_00->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                super_CBaseAnimating.super_CBaseEntity.m_vecAbsVelocity.y;
        fVar3 = (this_00->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                super_CBaseAnimating.super_CBaseEntity.m_vecAbsVelocity.z;
        return SQRT(fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3);
      }
      playerIndex = playerIndex + 1;
    } while (playerIndex <= *(int *)(**(int **)(unaff_EBX + 0x519fd6) + 0x14));
  }
  return 0.0;
}


/* CNPC_PersonalityCore::GetCoreTypeName at 0072bca0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: char * GetCoreTypeName(CNPC_PersonalityCore * this) */

char * __thiscall CNPC_PersonalityCore::GetCoreTypeName(CNPC_PersonalityCore *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(char **)(extraout_ECX + 0x639638 + this->m_CoreType * 4);
}


/* CNPC_PersonalityCore::TestRemarkingUpon at 0072bcc0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: bool TestRemarkingUpon(CNPC_PersonalityCore * this, CInfoRemarkable *
   pRemarkable) */

bool __thiscall
CNPC_PersonalityCore::TestRemarkingUpon(CNPC_PersonalityCore *this,CInfoRemarkable *pRemarkable)

{
  undefined1 uVar1;
  
  uVar1 = (**(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                                super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                                super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                                super_CAI_BehaviorHost<CAI_BaseNPC>.
                                super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x444))
                    (this,pRemarkable,1);
  return (bool)uVar1;
}


/* CNPC_PersonalityCore::NotifySystemEvent at 0072c6d0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void NotifySystemEvent(CNPC_PersonalityCore * this, CBaseEntity *
   pNotify, notify_system_event_t eventType, notify_system_event_params_t * params) */

void __thiscall
CNPC_PersonalityCore::NotifySystemEvent
          (CNPC_PersonalityCore *this,CBaseEntity *pNotify,notify_system_event_t eventType,
          notify_system_event_params_t *params)

{
  code *pcVar1;
  int unaff_EBX;
  CRR_Concept local_24 [2];
  undefined4 local_20;
  
  ___i686_get_pc_thunk_bx();
  if (eventType == NOTIFY_EVENT_TELEPORT) {
    pcVar1 = *(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                                 super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                                 super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                                 super_CAI_BehaviorHost<CAI_BaseNPC>.
                                 super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0xa70);
    ResponseRules::CRR_Concept::CRR_Concept(local_24,(char *)(unaff_EBX + 0x3851ad));
    local_20 = 0xffffffff;
    (*pcVar1)(this,local_24,0,0,0,0);
  }
  return;
}


/* CNPC_PersonalityCore::LoadSchedules at 0072bcf0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

bool CNPC_PersonalityCore::LoadSchedules(void)

{
  int *piVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CAI_PlayerAlly::LoadSchedules();
  piVar1 = *(int **)(unaff_EBX + 0x51b370);
  if (*(int *)(unaff_EBX + 0x65bcd4) != *piVar1) {
    InitCustomSchedules();
    CPlayerInfo::GetNetworkIDString[unaff_EBX] = (code)0x1;
    *(int *)(unaff_EBX + 0x65bcd4) = *piVar1;
  }
  return (bool)CPlayerInfo::GetNetworkIDString[unaff_EBX];
}


/* CNPC_PersonalityCore::LoadedSchedules at 0072bd50 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: bool LoadedSchedules(CNPC_PersonalityCore * this) */

bool __thiscall CNPC_PersonalityCore::LoadedSchedules(CNPC_PersonalityCore *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (bool)*(undefined1 *)(extraout_ECX + 0x65bc7c);
}


/* CNPC_PersonalityCore::SquadSlotName at 0072bd70 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: char * SquadSlotName(CNPC_PersonalityCore * this, int slotEN) */

char * __thiscall CNPC_PersonalityCore::SquadSlotName(CNPC_PersonalityCore *this,int slotEN)

{
  int symbolID;
  char *pcVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  symbolID = CAI_LocalIdSpace::LocalToGlobal((CAI_LocalIdSpace *)(unaff_EBX + 0x69cc90),slotEN);
  pcVar1 = CAI_GlobalNamespace::IdToSymbol
                     (*(CAI_GlobalNamespace **)(&DAT_0051b2a8 + unaff_EBX),symbolID);
  return pcVar1;
}


/* CNPC_PersonalityCore::CScheduleLoader::CScheduleLoader at 0072cc30 */

/* DWARF original prototype: void CScheduleLoader(CScheduleLoader * this) */

void __thiscall CNPC_PersonalityCore::CScheduleLoader::CScheduleLoader(CScheduleLoader *this)

{
  int *piVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CAI_PlayerAlly::LoadSchedules();
  piVar1 = *(int **)(unaff_EBX + 0x51a430);
  if (*(int *)(unaff_EBX + 0x65ad94) != *piVar1) {
    CNPC_PersonalityCore::InitCustomSchedules();
    *(undefined1 *)(unaff_EBX + 0x65ad90) = 1;
    *(int *)(unaff_EBX + 0x65ad94) = *piVar1;
  }
  return;
}


/* CNPC_PersonalityCore::CScheduleLoader::CScheduleLoader at 0072cc80 */

/* DWARF original prototype: void CScheduleLoader(CScheduleLoader * this) */

void __thiscall CNPC_PersonalityCore::CScheduleLoader::CScheduleLoader(CScheduleLoader *this)

{
  int *piVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CAI_PlayerAlly::LoadSchedules();
  piVar1 = *(int **)(unaff_EBX + 0x51a3e0);
  if (*(int *)(unaff_EBX + 0x65ad44) != *piVar1) {
    CNPC_PersonalityCore::InitCustomSchedules();
    *(undefined1 *)(unaff_EBX + 0x65ad40) = 1;
    *(int *)(unaff_EBX + 0x65ad44) = *piVar1;
  }
  return;
}


/* CNPC_PersonalityCore::InitCustomSchedules at 0072df50 */

/* WARNING: Removing unreachable block (ram,0x0072e81d) */
/* WARNING: Removing unreachable block (ram,0x0072e7e4) */
/* WARNING: Removing unreachable block (ram,0x0072e7b2) */
/* WARNING: Removing unreachable block (ram,0x0072e246) */
/* WARNING: Removing unreachable block (ram,0x0072e20d) */
/* WARNING: Removing unreachable block (ram,0x0072e470) */
/* WARNING: Removing unreachable block (ram,0x0072e48a) */
/* WARNING: Removing unreachable block (ram,0x0072e480) */
/* WARNING: Removing unreachable block (ram,0x0072e4c3) */
/* WARNING: Removing unreachable block (ram,0x0072e38f) */
/* WARNING: Removing unreachable block (ram,0x0072e3f6) */
/* WARNING: Removing unreachable block (ram,0x0072e3d0) */
/* WARNING: Removing unreachable block (ram,0x0072e41c) */
/* WARNING: Removing unreachable block (ram,0x0072e45a) */
/* WARNING: Removing unreachable block (ram,0x0072e124) */
/* WARNING: Removing unreachable block (ram,0x0072e570) */
/* WARNING: Removing unreachable block (ram,0x0072e580) */
/* WARNING: Removing unreachable block (ram,0x0072e587) */
/* WARNING: Removing unreachable block (ram,0x0072e5b5) */
/* WARNING: Removing unreachable block (ram,0x0072e5e3) */
/* WARNING: Removing unreachable block (ram,0x0072e5a0) */
/* WARNING: Removing unreachable block (ram,0x0072e605) */
/* WARNING: Removing unreachable block (ram,0x0072e612) */
/* WARNING: Removing unreachable block (ram,0x0072e0f2) */
/* WARNING: Removing unreachable block (ram,0x0072e617) */
/* WARNING: Removing unreachable block (ram,0x0072e620) */
/* WARNING: Removing unreachable block (ram,0x0072e62a) */
/* WARNING: Removing unreachable block (ram,0x0072e658) */
/* WARNING: Removing unreachable block (ram,0x0072e686) */
/* WARNING: Removing unreachable block (ram,0x0072e640) */
/* WARNING: Removing unreachable block (ram,0x0072e6a8) */
/* WARNING: Removing unreachable block (ram,0x0072e6bb) */
/* WARNING: Removing unreachable block (ram,0x0072e0fd) */
/* WARNING: Removing unreachable block (ram,0x0072e12f) */
/* WARNING: Removing unreachable block (ram,0x0072e84f) */
/* WARNING: Removing unreachable block (ram,0x0072e1db) */
/* WARNING: Removing unreachable block (ram,0x0072e278) */

void CNPC_PersonalityCore::InitCustomSchedules(void)

{
  CAI_GlobalNamespace *pCVar1;
  AI_NamespaceAddInfo_t *pAVar2;
  CStringRegistry *pCVar3;
  CStringRegistry *pCVar4;
  bool bVar5;
  undefined4 *puVar6;
  int unaff_EBX;
  int iVar7;
  CAI_GlobalNamespace *pGlobalNamespace;
  int local_b0;
  undefined1 *local_a8;
  int local_a4;
  undefined1 *local_9c;
  int local_78;
  int local_74;
  CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t,_int>_> local_44;
  void *local_30;
  undefined4 local_2c;
  int local_28;
  CAI_GlobalNamespace *local_24;
  void *local_20;
  
                    /* Unresolved local var: char * pszClassName@[???]
                       Unresolved local var: CUtlVector<char*,CUtlMemory<char*,_int>_>
                       schedulesToLoad@[???]
                       Unresolved local var: CUtlVector<bool_(*)(),CUtlMemory<bool_(*)(),_int>_>
                       reqiredOthers@[???]
                       Unresolved local var: CAI_NamespaceInfos scheduleIds@[???]
                       Unresolved local var: CAI_NamespaceInfos taskIds@[???]
                       Unresolved local var: CAI_NamespaceInfos conditionIds@[???]
                       Unresolved local var: CAI_NamespaceInfos squadSlotIds@[???]
                       Unresolved local var: int i@[???] */
  ___i686_get_pc_thunk_bx();
  local_30 = (void *)0x0;
  local_2c = 0;
  local_28 = 0;
  local_24 = (CAI_GlobalNamespace *)0x0;
  local_20 = (void *)0x0;
  local_44.m_Memory.m_pMemory = (AI_NamespaceAddInfo_t *)0x0;
  local_44.m_Memory.m_nAllocationCount = 0;
  local_44.m_Memory.m_nGrowSize = 0;
  local_44.m_Size = 0;
  local_44.m_pElements = (AI_NamespaceAddInfo_t *)0x0;
  *(int *)(unaff_EBX + 0x69aa5f) = unaff_EBX + 0x3837fc;
  iVar7 = *(int *)(unaff_EBX + 0x5195c3);
  bVar5 = CAI_LocalIdSpace::Init
                    ((CAI_LocalIdSpace *)(unaff_EBX + 0x69aa63),
                     *(CAI_GlobalNamespace **)(unaff_EBX + 0x5190bf),(CAI_LocalIdSpace *)(iVar7 + 4)
                    );
  if ((bVar5) &&
     (bVar5 = CAI_LocalIdSpace::Init
                        ((CAI_LocalIdSpace *)(unaff_EBX + 0x69aa7b),
                         (CAI_GlobalNamespace *)(*(int *)(unaff_EBX + 0x5190bf) + 8),
                         (CAI_LocalIdSpace *)(iVar7 + 0x1c)), bVar5)) {
    CAI_LocalIdSpace::Init
              ((CAI_LocalIdSpace *)(unaff_EBX + 0x69aa93),
               (CAI_GlobalNamespace *)(*(int *)(unaff_EBX + 0x5190bf) + 0x10),
               (CAI_LocalIdSpace *)(iVar7 + 0x34));
  }
  pGlobalNamespace = *(CAI_GlobalNamespace **)(unaff_EBX + 0x5190c3);
  CAI_LocalIdSpace::Init
            ((CAI_LocalIdSpace *)(unaff_EBX + 0x69aaab),pGlobalNamespace,
             *(CAI_LocalIdSpace **)(unaff_EBX + 0x5195cf));
  if (1 < (int)local_24) {
    if (local_30 == (void *)0x0) {
      local_a8 = (undefined1 *)((int)&local_24[-1].m_NextGlobalBase + 3);
      do {
        if (0 < (int)local_a8) {
          local_b0 = 1;
          local_74 = 0;
          do {
            pCVar1 = (CAI_GlobalNamespace *)((int)local_30 + local_b0 * 8);
            pAVar2 = (AI_NamespaceAddInfo_t *)((int)local_30 + local_74);
            pGlobalNamespace = pCVar1;
            iVar7 = CAI_NamespaceInfos::Compare(pAVar2,(AI_NamespaceAddInfo_t *)pCVar1);
            if (iVar7 < 0) {
              iVar7 = pAVar2->localId;
              pCVar3 = (CStringRegistry *)pAVar2->pszName;
              pCVar4 = pCVar1->m_pSymbols;
              pAVar2->localId = pCVar1->m_NextGlobalBase;
              pAVar2->pszName = (char *)pCVar4;
              pCVar1->m_NextGlobalBase = iVar7;
              pCVar1->m_pSymbols = pCVar3;
            }
            local_b0 = local_b0 + 1;
            local_74 = local_74 + 8;
          } while (local_b0 <= (int)local_a8);
        }
        local_a8 = local_a8 + -1;
      } while (local_a8 != (undefined1 *)0xffffffff);
    }
    else {
      pGlobalNamespace = local_24;
      _qsort(local_30,(size_t)local_24,8,*(int **)(unaff_EBX + 0x637a3f));
    }
  }
  if (1 < local_44.m_Size) {
    if (local_44.m_Memory.m_pMemory == (AI_NamespaceAddInfo_t *)0x0) {
      local_9c = (undefined1 *)
                 ((int)&((CAI_GlobalNamespace *)(local_44.m_Size + -8))->m_NextGlobalBase + 3);
      do {
        if (0 < (int)local_9c) {
          local_a4 = 1;
          local_78 = 0;
          do {
            pCVar1 = (CAI_GlobalNamespace *)(local_44.m_Memory.m_pMemory + local_a4);
            pAVar2 = (AI_NamespaceAddInfo_t *)
                     ((int)&(local_44.m_Memory.m_pMemory)->pszName + local_78);
            pGlobalNamespace = pCVar1;
            iVar7 = CAI_NamespaceInfos::Compare(pAVar2,(AI_NamespaceAddInfo_t *)pCVar1);
            if (iVar7 < 0) {
              iVar7 = pAVar2->localId;
              pCVar3 = (CStringRegistry *)pAVar2->pszName;
              pCVar4 = pCVar1->m_pSymbols;
              pAVar2->localId = pCVar1->m_NextGlobalBase;
              pAVar2->pszName = (char *)pCVar4;
              pCVar1->m_NextGlobalBase = iVar7;
              pCVar1->m_pSymbols = pCVar3;
            }
            local_a4 = local_a4 + 1;
            local_78 = local_78 + 8;
          } while (local_a4 <= (int)local_9c);
        }
        local_9c = local_9c + -1;
      } while (local_9c != (undefined1 *)0xffffffff);
    }
    else {
      pGlobalNamespace = (CAI_GlobalNamespace *)local_44.m_Size;
      _qsort(local_44.m_Memory.m_pMemory,local_44.m_Size,8,*(int **)(unaff_EBX + 0x637a3f));
    }
  }
  if (0 < (int)local_24) {
    iVar7 = 0;
    do {
      puVar6 = (undefined4 *)(iVar7 * 8 + (int)local_30);
      pGlobalNamespace = (CAI_GlobalNamespace *)*puVar6;
      bVar5 = CAI_LocalIdSpace::AddSymbol
                        ((CAI_LocalIdSpace *)(unaff_EBX + 0x69aa63),(char *)pGlobalNamespace,
                         puVar6[1],(char *)(unaff_EBX + 0x34258b),*(char **)(unaff_EBX + 0x6373bb));
      if (!bVar5) goto LAB_0072e1c3;
      iVar7 = iVar7 + 1;
    } while (iVar7 < (int)local_24);
  }
  if (0 < local_44.m_Size) {
    iVar7 = 0;
    do {
      pGlobalNamespace = (CAI_GlobalNamespace *)local_44.m_Memory.m_pMemory[iVar7].pszName;
      bVar5 = CAI_LocalIdSpace::AddSymbol
                        ((CAI_LocalIdSpace *)(unaff_EBX + 0x69aa7b),(char *)pGlobalNamespace,
                         local_44.m_Memory.m_pMemory[iVar7].localId,(char *)(unaff_EBX + 0x33e85f),
                         *(char **)(unaff_EBX + 0x6373bb));
      if (!bVar5) goto LAB_0072e1c3;
      iVar7 = iVar7 + 1;
    } while (iVar7 < local_44.m_Size);
  }
  CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t,_int>_>::~CUtlVector
            (&local_44,(int)pGlobalNamespace);
  local_24 = (CAI_GlobalNamespace *)0x0;
  if (local_28 < 0) goto LAB_0072e2de;
  if (local_30 != (void *)0x0) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x518bfb) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x518bfb),local_30);
    goto LAB_0072e2d0;
  }
LAB_0072e2d7:
  local_2c = 0;
LAB_0072e2de:
  local_20 = local_30;
  if ((-1 < local_28) && (local_30 != (void *)0x0)) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x518bfb) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x518bfb),local_30);
  }
  return;
LAB_0072e1c3:
  CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t,_int>_>::~CUtlVector
            (&local_44,(int)pGlobalNamespace);
  local_24 = (CAI_GlobalNamespace *)0x0;
  if (local_28 < 0) goto LAB_0072e2de;
  if (local_30 != (void *)0x0) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x518bfb) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x518bfb),local_30);
LAB_0072e2d0:
    local_30 = (void *)0x0;
  }
  goto LAB_0072e2d7;
}


/* _GLOBAL__I_sv_personality_core_pca_pitch at 000ad050 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_sv_personality_core_pca_pitch(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

