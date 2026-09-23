/* DWARF-guided pseudocode for game/server/portal2/npc_personality_core.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000c2090 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  undefined4 *puVar1;
  datamap_t *pdVar2;
  IEntityFactoryDictionary *pIVar3;
  int iVar4;
  int iVar5;
  int unaff_EBX;
  undefined4 *puVar6;
  undefined4 *puVar7;
  longlong lVar8;
  ConVar *in_stack_ffffffc8;
  char *in_stack_ffffffcc;
  char *in_stack_ffffffd0;
  int in_stack_ffffffd4;
  char *s2;
  char *in_stack_ffffffd8;
  SendTable *this;
  
  lVar8 = ___i686_get_pc_thunk_bx();
  if (lVar8 == 0xffff00000001) {
    (&DAT_00cf5a1c)[unaff_EBX] = 0;
    (&DAT_00cf5a1d)[unaff_EBX] = 0;
    (&DAT_00cf5a1e)[unaff_EBX] = 0;
    (&DAT_00cf5a1f)[unaff_EBX] = 0;
    *(undefined4 *)((int)&sk_npc_dmg_buckshot.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX)
         = 0;
    *(undefined4 *)((int)&sk_npc_dmg_buckshot.super_ConCommandBase.m_pNext + unaff_EBX) = 0;
    *(undefined4 *)(&sk_npc_dmg_buckshot.super_ConCommandBase.m_bRegistered + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&sk_npc_dmg_buckshot.super_ConCommandBase.m_pszName + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&sk_npc_dmg_buckshot.super_ConCommandBase.m_pszHelpString + unaff_EBX) = 0;
    *(undefined4 *)((int)&sk_npc_dmg_buckshot.super_ConCommandBase.m_nFlags + unaff_EBX) = 0;
    *(undefined4 *)((int)&sk_npc_dmg_buckshot.super_IConVar._vptr_IConVar + unaff_EBX) = 0;
    *(undefined4 *)((int)&sk_npc_dmg_buckshot.m_pParent + unaff_EBX) = 0;
    *(undefined4 *)((int)&sk_npc_dmg_buckshot.m_pszDefaultValue + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&sk_npc_dmg_buckshot.m_Value.m_pszString + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&sk_npc_dmg_buckshot.m_Value.m_StringLength + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&sk_npc_dmg_buckshot.m_Value.m_fValue + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&sk_npc_dmg_buckshot.m_Value.m_nValue + unaff_EBX) = 0;
    (&sk_npc_dmg_buckshot.field_0x37)[unaff_EBX] = 0;
    (&sk_npc_dmg_buckshot.m_bHasMin)[unaff_EBX] = false;
    (&sk_npc_dmg_buckshot.field_0x35)[unaff_EBX] = 0;
    (&sk_npc_dmg_buckshot.field_0x36)[unaff_EBX] = 0;
    *(undefined4 *)((int)&sk_npc_dmg_buckshot.m_fMinVal + unaff_EBX) = 0;
    (&sk_npc_dmg_buckshot.field_0x3f)[unaff_EBX] = 0;
    (&sk_npc_dmg_buckshot.m_bHasMax)[unaff_EBX] = false;
    (&sk_npc_dmg_buckshot.field_0x3d)[unaff_EBX] = 0;
    (&sk_npc_dmg_buckshot.field_0x3e)[unaff_EBX] = 0;
    *(undefined4 *)((int)&sk_npc_dmg_buckshot.m_fMaxVal + unaff_EBX) = 0;
    *(undefined1 *)
     ((int)&sk_npc_dmg_buckshot.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 3) = 1;
    *(undefined1 *)((int)&sk_npc_dmg_buckshot.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) =
         0;
    *(undefined1 *)
     ((int)&sk_npc_dmg_buckshot.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&sk_npc_dmg_buckshot.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 2) = 0;
    *(undefined4 *)
     ((int)&sk_npc_dmg_buckshot.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) = 1;
    *(undefined1 *)
     ((int)&sk_npc_dmg_buckshot.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 3) = 0;
    *(undefined1 *)((int)&sk_npc_dmg_buckshot.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX)
         = 0;
    *(undefined1 *)
     ((int)&sk_npc_dmg_buckshot.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&sk_npc_dmg_buckshot.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&sk_npc_dmg_buckshot.m_fnChangeCallbacks.m_Size + unaff_EBX) = 2;
    *(undefined1 *)((int)&sk_npc_dmg_buckshot.m_fnChangeCallbacks.m_pElements + unaff_EBX + 3) = 0;
    *(undefined1 *)((int)&sk_npc_dmg_buckshot.m_fnChangeCallbacks.m_pElements + unaff_EBX) = 0;
    *(undefined1 *)((int)&sk_npc_dmg_buckshot.m_fnChangeCallbacks.m_pElements + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&sk_npc_dmg_buckshot.m_fnChangeCallbacks.m_pElements + unaff_EBX + 2) = 0;
    *(undefined **)(&DAT_00cf5a78 + unaff_EBX) = &UNK_00aef284 + unaff_EBX;
    ConVar::ConVar((ConVar *)((int)&PTR_SetScheme_00c6cb5c + unaff_EBX),
                   (char *)(unaff_EBX + 0x94d845),(char *)(unaff_EBX + 0x94dfe4),0,
                   (char *)(unaff_EBX + 0x94d80c),in_stack_ffffffc8,in_stack_ffffffcc,
                   in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
    ___cxa_atexit(unaff_EBX + 0x8d84fc,0,*(undefined4 *)(&DAT_00ae9aec + unaff_EBX));
    ConVar::ConVar((ConVar *)((int)&PTR_OnMouseWheeled_00c6cbbc + unaff_EBX),
                   (char *)(unaff_EBX + 0x94d89f),(char *)(unaff_EBX + 0x94d89b),0,
                   (char *)(unaff_EBX + 0x94d864),in_stack_ffffffc8,in_stack_ffffffcc,
                   in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
    ___cxa_atexit(unaff_EBX + 0x8d84dc,0,*(undefined4 *)(&DAT_00ae9aec + unaff_EBX));
    ConVar::ConVar((ConVar *)((int)&PTR_PerformLayout_00c6cc1c + unaff_EBX),
                   (char *)(unaff_EBX + 0x94d8f8),(char *)(unaff_EBX + 0x94d8f4),0,
                   (char *)(unaff_EBX + 0x94d8bc),in_stack_ffffffc8,in_stack_ffffffcc,
                   in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
    ___cxa_atexit(unaff_EBX + 0x8d84bc,0,*(undefined4 *)(&DAT_00ae9aec + unaff_EBX));
    pdVar2 = DataMapInit<CNPC_PersonalityCore>((CNPC_PersonalityCore *)0x0);
    *(datamap_t **)((int)&PTR_GetDropContextMenu_00c6cc74 + unaff_EBX) = pdVar2;
    *(undefined **)(&DAT_00cf5a7c + unaff_EBX) = &UNK_00b72e84 + unaff_EBX;
    pIVar3 = EntityFactoryDictionary();
    (**pIVar3->_vptr_IEntityFactoryDictionary)
              (pIVar3,&DAT_00cf5a7c + unaff_EBX,unaff_EBX + 0x8fc376);
    iVar5 = (int)&sk_max_buckshot.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX;
    s2 = (char *)(unaff_EBX + 0x94d7f6);
    *(char **)((int)&sk_max_buckshot.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) = s2;
    this = (SendTable *)((int)&PTR_OnDropContextHoverShow_00c6cc78 + unaff_EBX);
    *(SendTable **)((int)&sk_max_buckshot.super_ConCommandBase.m_pNext + unaff_EBX) = this;
    *(undefined4 *)((int)&sk_max_buckshot.super_ConCommandBase.m_pszHelpString + unaff_EBX) = 0xffff
    ;
    puVar7 = (undefined4 *)**(int **)(&DAT_00ae9b8c + unaff_EBX);
    if (puVar7 == (undefined4 *)0x0) {
      **(int **)(&DAT_00ae9b8c + unaff_EBX) = iVar5;
      *(undefined4 *)(&sk_max_buckshot.super_ConCommandBase.m_bRegistered + unaff_EBX) = 0;
    }
    else {
      puVar1 = (undefined4 *)puVar7[2];
      iVar4 = _V_stricmp((char *)*puVar7,s2);
      if (iVar4 < 1) {
        while ((puVar6 = puVar1, puVar6 != (undefined4 *)0x0 &&
               (iVar4 = _V_stricmp((char *)*puVar6,s2), iVar4 < 1))) {
          puVar1 = (undefined4 *)puVar6[2];
          puVar7 = puVar6;
        }
        *(undefined4 **)(&sk_max_buckshot.super_ConCommandBase.m_bRegistered + unaff_EBX) = puVar6;
        puVar7[2] = iVar5;
      }
      else {
        *(undefined4 *)(&sk_max_buckshot.super_ConCommandBase.m_bRegistered + unaff_EBX) =
             **(undefined4 **)(&DAT_00ae9b8c + unaff_EBX);
        **(int **)(&DAT_00ae9b8c + unaff_EBX) = iVar5;
      }
    }
    SendTable::SendTable(this);
    ___cxa_atexit(unaff_EBX + 0x8d849c,0,*(undefined4 *)(&DAT_00ae9aec + unaff_EBX));
    iVar5 = ServerClassInit<DT_NPC_Personality_Core::ignored>((ignored *)0x0);
    *(int *)((int)&PTR_IsDroppable_00c6cc8c + unaff_EBX) = iVar5;
    CAI_LocalIdSpace::CAI_LocalIdSpace
              ((CAI_LocalIdSpace *)((int)&PTR_OnPanelExitedDroppablePanel_00c6cca0 + unaff_EBX),
               false);
    CAI_LocalIdSpace::CAI_LocalIdSpace
              ((CAI_LocalIdSpace *)((int)&PTR_CanStartDragging_00c6ccb8 + unaff_EBX),false);
    CAI_LocalIdSpace::CAI_LocalIdSpace
              ((CAI_LocalIdSpace *)((int)&PTR_NavigateDown_00c6ccd0 + unaff_EBX),false);
    CAI_LocalIdSpace::CAI_LocalIdSpace
              ((CAI_LocalIdSpace *)((int)&PTR_OnNavigateTo_00c6cce8 + unaff_EBX),false);
  }
  return;
}


/* __tcf_2 at 0099a5a0 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x394658),in_stack_00000008);
  return;
}


/* __tcf_3 at 0099a580 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3946d8),in_stack_00000008);
  return;
}


/* __tcf_4 at 0099a560 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x394758),in_stack_00000008);
  return;
}


/* CEntityFactory<CNPC_PersonalityCore>::Create at 006c8d60 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CNPC_PersonalityCore> *
   this, char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CNPC_PersonalityCore>::Create
          (CEntityFactory<CNPC_PersonalityCore> *this,char *pClassName)

{
  int *piVar1;
  int iVar2;
  CBaseAnimating *pCVar3;
  CAI_BaseNPC *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CNPC_PersonalityCore * pEnt@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CBaseEntity::operator_new(0x11cc);
  CAI_BaseNPC::CAI_BaseNPC(this_00);
  (this_00->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x4f211a);
  (this_00->super_CAI_DefMovementSink).super_IAI_MovementSink._vptr_IAI_MovementSink =
       (_func_int_varargs **)(&UNK_004f2b9e + unaff_EBX);
  (this_00->super_IAI_BehaviorBridge)._vptr_IAI_BehaviorBridge =
       (_func_int_varargs **)(unaff_EBX + 0x4f2bca);
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_iObjectCapsCache.m_Value = unaff_EBX + 0x4f2c76;
  (this_00->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(*(int *)(unaff_EBX + 0x4e3536) + 8);
  (this_00->super_CAI_DefMovementSink).super_IAI_MovementSink._vptr_IAI_MovementSink =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4e3536) + 0xad4);
  (this_00->super_IAI_BehaviorBridge)._vptr_IAI_BehaviorBridge =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4e3536) + 0xb00);
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_iObjectCapsCache.m_Value =
       *(int *)(unaff_EBX + 0x4e3536) + 0xbac;
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_pfnThink.__pfn =
       (_func___thiscall_void_CBaseEntity_ptr *)0x0;
  piVar1 = *(int **)(unaff_EBX + 0x4e2e42);
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_pfnThink.__delta = *piVar1;
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_Network.super_IServerNetworkable._vptr_IServerNetworkable
       = (_func_int_varargs **)piVar1[1];
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_Network.super_IEventRegisterCallback.
  _vptr_IEventRegisterCallback = (_func_int_varargs **)piVar1[2];
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_Network.m_pOuter = (CBaseEntity *)*piVar1;
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev = (edict_t *)piVar1[1];
  iVar2 = piVar1[2];
  pCVar3 = &this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
            super_CBaseAnimating;
  (pCVar3->super_CBaseEntity).m_Network.m_PVSInfo.m_nHeadNode = (short)iVar2;
  (pCVar3->super_CBaseEntity).m_Network.m_PVSInfo.m_nClusterCount = (short)((uint)iVar2 >> 0x10);
  *(ushort **)
   ((int)&this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
          super_CBaseAnimating.super_CBaseEntity.m_Network.m_PVSInfo + 4) = (ushort *)*piVar1;
  *(int *)((int)&this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                 super_CBaseAnimating.super_CBaseEntity.m_Network.m_PVSInfo + 8) = piVar1[1];
  *(int *)((int)&this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                 super_CBaseAnimating.super_CBaseEntity.m_Network.m_PVSInfo + 0xc) = piVar1[2];
  *(IEventRegisterCallback **)
   ((int)&this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
          super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0xc) =
       (IEventRegisterCallback *)0x0;
  *(undefined4 *)
   ((int)&this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
          super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) = 0xffffffff;
  *(undefined4 *)
   &this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
    super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c = 0;
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_iClassname.pszValue = (char *)0x0;
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_iGlobalname.pszValue = (char *)0x0;
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_iParent.pszValue = (char *)0x0;
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_iHammerID =
       *(int *)&this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_flPrevAnimTime = 0.0;
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_flAnimTime.m_Value = 0.0;
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_flSimulationTime.m_Value = 0.0;
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_flCreateTime.m_Value = 0.0;
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_nLastThinkTick =
       (int)this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
            super_CBaseAnimating.super_CBaseEntity.m_flPrevAnimTime;
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.touchStamp = 0;
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_aThinkFunctions.m_Memory.m_pMemory = (thinkfunc_t *)0x0;
  *(int *)((int)&this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                 super_CBaseAnimating.super_CBaseEntity.m_aThinkFunctions.m_Memory + 4) = 0;
  *(int *)((int)&this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                 super_CBaseAnimating.super_CBaseEntity.m_aThinkFunctions.m_Memory + 8) = 0;
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_aThinkFunctions.m_Size =
       this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.touchStamp;
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_ResponseContexts.m_Memory.m_pMemory =
       (ResponseContext_t *)0x0;
  *(int *)((int)&this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                 super_CBaseAnimating.super_CBaseEntity.m_ResponseContexts.m_Memory + 4) = -1;
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_ResponseContexts.m_Size = 0;
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_ResponseContexts.m_pElements = (ResponseContext_t *)0x0;
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_iszResponseContext.pszValue = (char *)0x0;
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_nNextThinkTick.m_Value = 0;
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_fEffects.m_Value = 0;
  *(undefined1 *)
   &this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
    super_CBaseAnimating.super_CBaseEntity.m_pfnTouch.__pfn = 0;
  _memset(&this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
           super_CBaseAnimating.super_CBaseEntity.m_vecEyeOffset,0,0x100);
  (this_00->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(*(int *)(unaff_EBX + 0x4e3806) + 8);
  (this_00->super_CAI_DefMovementSink).super_IAI_MovementSink._vptr_IAI_MovementSink =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4e3806) + 0xb0c);
  (this_00->super_IAI_BehaviorBridge)._vptr_IAI_BehaviorBridge =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4e3806) + 0xb38);
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_iObjectCapsCache.m_Value =
       *(int *)(unaff_EBX + 0x4e3806) + 0xbe4;
  ResponseRules::CRR_Response::CRR_Response
            ((CRR_Response *)
             &this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
              super_CBaseAnimating.super_CBaseEntity.m_AIAddOn);
  *(int *)((int)&this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                 super_CBaseAnimating.super_CBaseEntity.m_OnUser1.super_CBaseEntityOutput + 8) =
       *(int *)(unaff_EBX + 0x4e3532) + 0xc;
  *(fieldtype_t *)
   ((int)&this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
          super_CBaseAnimating.super_CBaseEntity.m_OnUser1.super_CBaseEntityOutput + 0x10) =
       ~FIELD_VOID;
  *(undefined4 *)
   ((int)&this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
          super_CBaseAnimating.super_CBaseEntity.m_OnUser2.super_CBaseEntityOutput + 4) = 0xffffffff
  ;
  *(fieldtype_t *)
   ((int)&this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
          super_CBaseAnimating.super_CBaseEntity.m_OnUser2.super_CBaseEntityOutput + 0x10) =
       0xbf800000;
  *(CEventAction **)
   ((int)&this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
          super_CBaseAnimating.super_CBaseEntity.m_OnUser2.super_CBaseEntityOutput + 0x14) =
       (CEventAction *)0xbf800000;
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_OnUser3.super_CBaseEntityOutput.m_Value.field_0.iVal =
       -0x40800000;
  *(undefined4 *)
   ((int)&this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
          super_CBaseAnimating.super_CBaseEntity.m_OnUser3.super_CBaseEntityOutput + 4) = 0xffffffff
  ;
  CAI_PlayerAlly::CScheduleLoader::CScheduleLoader
            ((CScheduleLoader *)
             ((int)&this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.
                    super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.m_OnUser3.
                    super_CBaseEntityOutput + 0x14));
  (this_00->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x56b53a);
  (this_00->super_CAI_DefMovementSink).super_IAI_MovementSink._vptr_IAI_MovementSink =
       (_func_int_varargs **)(unaff_EBX + 0x56c052);
  (this_00->super_IAI_BehaviorBridge)._vptr_IAI_BehaviorBridge =
       (_func_int_varargs **)(unaff_EBX + 0x56c07e);
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_iObjectCapsCache.m_Value = unaff_EBX + 0x56c12a;
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_OnUser4.super_CBaseEntityOutput.m_Value.field_0.iVal =
       unaff_EBX + 0x56c13e;
  *(CEventAction **)
   ((int)&this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
          super_CBaseAnimating.super_CBaseEntity.m_OnUser4.super_CBaseEntityOutput + 0x14) =
       (CEventAction *)0x0;
  *(undefined4 *)
   ((int)&this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
          super_CBaseAnimating.super_CBaseEntity.m_OnKilled.super_CBaseEntityOutput + 8) =
       0xffffffff;
  (((CHandle<CBaseEntity> *)
   ((int)&this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
          super_CBaseAnimating.super_CBaseEntity.m_OnKilled.super_CBaseEntityOutput + 0xc))->
  super_CBaseHandle).m_Index = 0;
  *(CEventAction **)
   ((int)&this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
          super_CBaseAnimating.super_CBaseEntity.m_OnKilled.super_CBaseEntityOutput + 0x14) =
       (CEventAction *)0x0;
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_angAbsRotation.z = -NAN;
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_cellwidth = 0;
  this_00[1].super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_vecOrigin.
  super_CNetworkVarBase<Vector,CBaseEntity::NetworkVar_m_vecOrigin>.m_Value.x = -NAN;
  CAI_PlayerAlly::LoadSchedules();
  piVar1 = *(int **)(unaff_EBX + 0x4e3332);
  if (*(int *)(unaff_EBX + 0x62490a) != *piVar1) {
    CNPC_PersonalityCore::InitCustomSchedules();
    *(undefined1 *)(unaff_EBX + 0x624906) = 1;
    *(int *)(unaff_EBX + 0x62490a) = *piVar1;
  }
  (**(code **)((int)(this_00->super_CBaseCombatCharacter).super_CBaseFlex.
                    super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                    super_IServerEntity.super_IServerUnknown + 0x74))(this_00,pClassName);
  return &(this_00->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
          super_CBaseAnimating.super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CNPC_PersonalityCore::ObjectCaps at 006c8d10 */

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


/* CNPC_PersonalityCore::HasPreferredCarryAnglesForPlayer at 006c8c20 */

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


/* CNPC_PersonalityCore::CreateExpresser at 006c8c90 */

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
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4e36c0) + 8);
  this_00[1].super_IResponseFilter._vptr_IResponseFilter = (_func_int_varargs **)0x0;
  this_00[1].m_pSink = (CAI_ExpresserSink *)((int)&vgui::Panel::GetMessageMap::s_pMap.magic + 3);
  this_00[1].m_ConceptHistories.m_Elements.m_Tree.m_LessFunc.m_LessFunc =
       (_func_bool_char_ptr_ptr_char_ptr_ptr *)0xffffffff;
  this_00->m_pSink =
       &(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
        super_CAI_ExpresserSink;
  return this_00;
}


/* CNPC_PersonalityCore::GetClassScheduleIdSpace at 006c8c30 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: CAI_ClassScheduleIdSpace * GetClassScheduleIdSpace(CNPC_PersonalityCore
   * this) */

CAI_ClassScheduleIdSpace * __thiscall
CNPC_PersonalityCore::GetClassScheduleIdSpace(CNPC_PersonalityCore *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (CAI_ClassScheduleIdSpace *)(&UNK_00666108 + extraout_ECX);
}


/* CNPC_PersonalityCore::GetSchedulingErrorName at 006c8c40 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: char * GetSchedulingErrorName(CNPC_PersonalityCore * this) */

char * __thiscall CNPC_PersonalityCore::GetSchedulingErrorName(CNPC_PersonalityCore *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(char **)(extraout_ECX + 0x602774);
}


/* DataMapInit<CNPC_PersonalityCore> at 000c1f60 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */

datamap_t * DataMapInit<CNPC_PersonalityCore>(CNPC_PersonalityCore *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&sk_npc_dmg_sniper_round.super_ConCommandBase.m_pszName + unaff_EBX) == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcf5ccc);
    if (iVar2 != 0) {
      *(int *)((int)&sk_npc_dmg_sniper_round.m_pParent + unaff_EBX) = unaff_EBX + 0x94d926;
      *(undefined4 *)((int)&sk_npc_dmg_sniper_round.m_Value.m_pszString + unaff_EBX) = 0;
      *(undefined4 *)((int)&sk_npc_dmg_sniper_round.m_Value.m_StringLength + unaff_EBX) = 0;
      *(undefined4 *)((int)&sk_npc_dmg_sniper_round.m_Value.m_fValue + unaff_EBX) = 0;
      *(undefined4 *)((int)&sk_npc_dmg_sniper_round.m_Value.m_nValue + unaff_EBX) = 0;
      *(undefined4 *)(&sk_npc_dmg_sniper_round.m_bHasMin + unaff_EBX) = 0;
      *(undefined4 *)((int)&sk_npc_dmg_sniper_round.m_pszDefaultValue + unaff_EBX) = 0x14;
      ___cxa_guard_release(unaff_EBX + 0xcf5ccc);
      ___cxa_atexit(unaff_EBX + 0x8d864c,0,*(undefined4 *)(&DAT_00ae9c1c + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_VPhysicsIsFlesh_00c09430 + unaff_EBX) =
       *(undefined4 *)(&DAT_00aea5f8 + unaff_EBX);
  if (*(char *)((int)&sk_npc_dmg_sniper_round.super_ConCommandBase.m_nFlags + unaff_EBX) == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcf5cd4);
    if (iVar2 != 0) {
      uVar1 = **(undefined4 **)(&DAT_00ae9c38 + unaff_EBX);
      *(undefined4 *)((int)&PTR_GetVectors_00c09720 + unaff_EBX) = uVar1;
      *(undefined4 *)((int)&PTR_GetSoundEmissionOrigin_00c09760 + unaff_EBX) = uVar1;
      ___cxa_guard_release(unaff_EBX + 0xcf5cd4);
    }
  }
  *(undefined4 *)((int)&PTR_UpdatePhysicsShadowToCurrentPosition_00c09428 + unaff_EBX) = 0x18;
  *(int *)((int)&PTR_VPhysicsFriction_00c09424 + unaff_EBX) =
       (int)&PTR_NetworkStateChanged_m_vecViewOffset_00c0948c + unaff_EBX;
  return (datamap_t *)((int)&PTR_VPhysicsFriction_00c09424 + unaff_EBX);
}


/* CNPC_PersonalityCore::GetDataDescMap at 006c5f00 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: datamap_t * GetDataDescMap(CNPC_PersonalityCore * this) */

datamap_t * __thiscall CNPC_PersonalityCore::GetDataDescMap(CNPC_PersonalityCore *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x605490);
}


/* CNPC_PersonalityCore::GetBaseMap at 006c5f10 */

datamap_t * CNPC_PersonalityCore::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4e6654);
}


/* __tcf_0 at 0099a5c0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41d696)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41d68a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41d696));
  }
  *(undefined4 *)(unaff_EBX + 0x41d696) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41d692)) {
    if (*(int *)(unaff_EBX + 0x41d68a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2115ca) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2115ca),*(int *)(unaff_EBX + 0x41d68a));
      *(undefined4 *)(unaff_EBX + 0x41d68a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d68e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41d68a);
  *(int *)(unaff_EBX + 0x41d69a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41d692)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2115ca) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2115ca),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41d68a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d68e) = 0;
  }
  return;
}


/* ServerClassInit<DT_NPC_Personality_Core::ignored> at 000c1da0 */

int ServerClassInit<DT_NPC_Personality_Core::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&sk_plr_dmg_grenade.super_ConCommandBase.m_pNext + unaff_EBX) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf5d84);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  ((int)&sk_plr_dmg_grenade.super_ConCommandBase.m_pszName + unaff_EBX),
                  &UNK_008f0c70 + unaff_EBX,0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&sk_npc_dmg_grenade.super_ConCommandBase._vptr_ConCommandBase +
                        unaff_EBX),(char *)(unaff_EBX + 0x8f2fe0),0,
                        (SendTable *)**(undefined4 **)(&DAT_00aea88c + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00ae9e78 + unaff_EBX),0x80);
      SendPropBool((SendProp_conflict *)
                   ((int)&sk_npc_dmg_grenade.m_fnChangeCallbacks.m_pElements + unaff_EBX),
                   (char *)(unaff_EBX + 0x94dad1),0x11c0,1);
      ___cxa_guard_release(unaff_EBX + 0xcf5d84);
      ___cxa_atexit(&UNK_008d873c + unaff_EBX,0,*(undefined4 *)(&DAT_00ae9ddc + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)((int)&PTR_GetKeyBindingsFile_00c6cf68 + unaff_EBX),
             (SendProp_conflict *)
             ((int)&sk_npc_dmg_grenade.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX),2,
             *(char **)(&DAT_00b73150 + unaff_EBX));
  return 1;
}


/* CNPC_PersonalityCore::GetServerClass at 006c5f20 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: ServerClass * GetServerClass(CNPC_PersonalityCore * this) */

ServerClass * __thiscall CNPC_PersonalityCore::GetServerClass(CNPC_PersonalityCore *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x6f1bfc);
}


/* CNPC_PersonalityCore::YouForgotToImplementOrDeclareServerClass at 006c5f30 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CNPC_PersonalityCore *
   this) */

int __thiscall
CNPC_PersonalityCore::YouForgotToImplementOrDeclareServerClass(CNPC_PersonalityCore *this)

{
  return 0;
}


/* __tcf_5 at 0099a540 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x3947d4),in_stack_00000008);
  return;
}


/* __tcf_1 at 0099a4f0 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d6e7))(unaff_EBX + 0x41d6e7);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d693))(unaff_EBX + 0x41d693);
                    /* WARNING: Could not recover jumptable at 0x0099a537. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(&DAT_0041d63f + unaff_EBX))();
  return;
}


/* CNPC_PersonalityCore::~CNPC_PersonalityCore at 006c8780 */

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
       unaff_EBX + 0x56bb1a;
  (((CAI_DefMovementSink *)
   ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>
          .super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
          super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x8dc))->
  super_IAI_MovementSink)._vptr_IAI_MovementSink = (_func_int_varargs **)(unaff_EBX + 0x56c632);
  ((IAI_BehaviorBridge *)
  ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
         super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
         super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x8e0))->
  _vptr_IAI_BehaviorBridge = (_func_int_varargs **)(unaff_EBX + 0x56c65e);
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
  super_CAI_ExpresserSink._vptr_CAI_ExpresserSink = (_func_int_varargs **)(unaff_EBX + 0x56c70a);
  (this->super_CDefaultPlayerPickupVPhysics).super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics
       = (_func_int_varargs **)(&UNK_0056c71e + unaff_EBX);
  uVar2 = (this->m_hProjectedTexture).super_CBaseHandle.m_Index;
  if (((uVar2 != 0xffffffff) &&
      (iVar4 = **(int **)(unaff_EBX + 0x4e3416) + (uVar2 & 0xffff) * 0x10,
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
       *(int *)(unaff_EBX + 0x4e3de6) + 8;
  (((CAI_DefMovementSink *)
   ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>
          .super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
          super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x8dc))->
  super_IAI_MovementSink)._vptr_IAI_MovementSink =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4e3de6) + 0xb0c);
  ((IAI_BehaviorBridge *)
  ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
         super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
         super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x8e0))->
  _vptr_IAI_BehaviorBridge = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4e3de6) + 0xb38);
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
  super_CAI_ExpresserSink._vptr_CAI_ExpresserSink =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4e3de6) + 0xbe4);
  pcVar3 = (this->super_CAI_PlayerAlly).m_PendingConcept._M_dataplus._M_p;
  paVar1 = (allocator *)(pcVar3 + -0xc);
  if (paVar1 != *(allocator **)(unaff_EBX + 0x4e3b12)) {
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


/* CNPC_PersonalityCore::~CNPC_PersonalityCore at 006c89c0 */

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
       unaff_EBX + 0x56b8da;
  (((CAI_DefMovementSink *)
   ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>
          .super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
          super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x8dc))->
  super_IAI_MovementSink)._vptr_IAI_MovementSink = (_func_int_varargs **)(unaff_EBX + 0x56c3f2);
  ((IAI_BehaviorBridge *)
  ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
         super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
         super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x8e0))->
  _vptr_IAI_BehaviorBridge = (_func_int_varargs **)(unaff_EBX + 0x56c41e);
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
  super_CAI_ExpresserSink._vptr_CAI_ExpresserSink = (_func_int_varargs **)(unaff_EBX + 0x56c4ca);
  (this->super_CDefaultPlayerPickupVPhysics).super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics
       = (_func_int_varargs **)(unaff_EBX + 0x56c4de);
  uVar2 = (this->m_hProjectedTexture).super_CBaseHandle.m_Index;
  if (((uVar2 != 0xffffffff) &&
      (iVar4 = **(int **)(unaff_EBX + 0x4e31d6) + (uVar2 & 0xffff) * 0x10,
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
       *(int *)(unaff_EBX + 0x4e3ba6) + 8;
  (((CAI_DefMovementSink *)
   ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>
          .super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
          super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x8dc))->
  super_IAI_MovementSink)._vptr_IAI_MovementSink =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4e3ba6) + 0xb0c);
  ((IAI_BehaviorBridge *)
  ((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
         super_CAI_BaseHumanoid.super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
         super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x8e0))->
  _vptr_IAI_BehaviorBridge = (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4e3ba6) + 0xb38);
  (this->super_CAI_PlayerAlly).super_CAI_BaseActor.super_CAI_ExpresserHost<CAI_BaseHumanoid>.
  super_CAI_ExpresserSink._vptr_CAI_ExpresserSink =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x4e3ba6) + 0xbe4);
  pcVar3 = (this->super_CAI_PlayerAlly).m_PendingConcept._M_dataplus._M_p;
  paVar1 = (allocator *)(pcVar3 + -0xc);
  if (paVar1 != *(allocator **)(unaff_EBX + 0x4e38d2)) {
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


/* CNPC_PersonalityCore::~CNPC_PersonalityCore at 006c8c00 */

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


/* CNPC_PersonalityCore::Precache at 006c64c0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void Precache(CNPC_PersonalityCore * this) */

void __thiscall CNPC_PersonalityCore::Precache(CNPC_PersonalityCore *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x346c2c));
  PrecacheInstancedScene((char *)(unaff_EBX + 0x34933c));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x3493a2));
  return;
}


/* CNPC_PersonalityCore::Spawn at 006c6c20 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "MoveCollide_t": Some values do not have unique names */
/* WARNING: Enum "MoveType_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(CNPC_PersonalityCore * this) */

void __thiscall CNPC_PersonalityCore::Spawn(CNPC_PersonalityCore *this)

{
  undefined1 *puVar1;
  NetworkVar_m_Collision *this_00;
  CBaseEdict *pCVar2;
  int iVar3;
  CBaseEntity *pCVar4;
  uint *puVar5;
  int iVar6;
  IChangeInfoAccessor *pIVar7;
  int *piVar8;
  uint uVar9;
  uint uVar10;
  int unaff_EBX;
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
              + 0x6c))(this,unaff_EBX + 0x3464ca);
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
      pCVar2 = &((this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                 super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                 super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                 super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>.
                 super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.
                 super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev
                )->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar7->m_iChangeInfoSerialNumber = 0;
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
  iVar3 = CBaseAnimating::LookupAttachment
                    ((CBaseAnimating *)this,
                     (char *)(CUtlVector<AIRebalanceInfo_t,CUtlMemory<AIRebalanceInfo_t,_int>_>::
                              GrowVector + unaff_EBX + 2));
  CBaseAnimating::GetAttachment((CBaseAnimating *)this,iVar3,local_28,&local_34);
  pCVar4 = CreateEntityByName((char *)(unaff_EBX + 0x32388e),-1,true);
  if (pCVar4 == (CBaseEntity *)0x0) {
    (this->m_hProjectedTexture).super_CBaseHandle.m_Index = 0xffffffff;
    piVar8 = (int *)0x0;
  }
  else {
    puVar5 = (uint *)(*(pCVar4->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                       _vptr_IHandleEntity[3])(pCVar4);
    uVar10 = *puVar5;
    (this->m_hProjectedTexture).super_CBaseHandle.m_Index = uVar10;
    if ((uVar10 == 0xffffffff) ||
       (iVar6 = (uVar10 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e4f76),
       *(uint *)(iVar6 + 8) != uVar10 >> 0x10)) {
      piVar8 = (int *)0x0;
    }
    else {
      piVar8 = *(int **)(iVar6 + 4);
    }
  }
  (**(code **)(*piVar8 + 0x80))(piVar8,unaff_EBX + 0x323950,unaff_EBX + 0x348de5);
  uVar10 = (this->m_hProjectedTexture).super_CBaseHandle.m_Index;
  if ((uVar10 == 0xffffffff) ||
     (iVar6 = (uVar10 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e4f76),
     *(uint *)(iVar6 + 8) != uVar10 >> 0x10)) {
    piVar8 = (int *)0x0;
  }
  else {
    piVar8 = *(int **)(iVar6 + 4);
  }
  (**(code **)(*piVar8 + 0x80))(piVar8,unaff_EBX + 0x323959,unaff_EBX + 0x2ece0e);
  uVar10 = (this->m_hProjectedTexture).super_CBaseHandle.m_Index;
  if ((uVar10 == 0xffffffff) ||
     (iVar6 = (uVar10 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e4f76),
     *(uint *)(iVar6 + 8) != uVar10 >> 0x10)) {
    iVar6 = 0;
  }
  else {
    iVar6 = *(int *)(iVar6 + 4);
  }
  uVar9 = *(uint *)(iVar6 + 0x128) | 2;
  if (*(uint *)(iVar6 + 0x128) != uVar9) {
    if (*(char *)(iVar6 + 0x60) == '\0') {
      pCVar2 = *(CBaseEdict **)(iVar6 + 0x24);
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar7->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
    }
    *(uint *)(iVar6 + 0x128) = uVar9;
    uVar10 = (this->m_hProjectedTexture).super_CBaseHandle.m_Index;
  }
  if ((uVar10 == 0xffffffff) ||
     (iVar6 = (uVar10 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e4f76),
     *(uint *)(iVar6 + 8) != uVar10 >> 0x10)) {
    pCVar4 = (CBaseEntity *)0x0;
  }
  else {
    pCVar4 = *(CBaseEntity **)(iVar6 + 4);
  }
  DispatchSpawn(pCVar4,true);
  uVar10 = (this->m_hProjectedTexture).super_CBaseHandle.m_Index;
  if ((uVar10 == 0xffffffff) ||
     (iVar6 = (uVar10 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e4f76),
     *(uint *)(iVar6 + 8) != uVar10 >> 0x10)) {
    pCVar4 = (CBaseEntity *)0x0;
  }
  else {
    pCVar4 = *(CBaseEntity **)(iVar6 + 4);
  }
  CBaseEntity::SetAbsAngles(pCVar4,&local_34);
  uVar10 = (this->m_hProjectedTexture).super_CBaseHandle.m_Index;
  if ((uVar10 == 0xffffffff) ||
     (iVar6 = (uVar10 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e4f76),
     *(uint *)(iVar6 + 8) != uVar10 >> 0x10)) {
    pCVar4 = (CBaseEntity *)0x0;
  }
  else {
    pCVar4 = *(CBaseEntity **)(iVar6 + 4);
  }
  CBaseEntity::SetAbsOrigin(pCVar4,local_28);
  uVar10 = (this->m_hProjectedTexture).super_CBaseHandle.m_Index;
  if ((uVar10 == 0xffffffff) ||
     (iVar6 = (uVar10 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e4f76),
     *(uint *)(iVar6 + 8) != uVar10 >> 0x10)) {
    piVar8 = (int *)0x0;
  }
  else {
    piVar8 = *(int **)(iVar6 + 4);
  }
  (**(code **)(*piVar8 + 0x98))(piVar8,this,iVar3);
  uVar10 = (this->m_hProjectedTexture).super_CBaseHandle.m_Index;
  if ((uVar10 == 0xffffffff) ||
     (iVar3 = (uVar10 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e4f76),
     *(uint *)(iVar3 + 8) != uVar10 >> 0x10)) {
    piVar8 = (int *)0x0;
  }
  else {
    piVar8 = *(int **)(iVar3 + 4);
  }
  local_48 = 0;
  local_5c = 0;
  local_58 = local_44;
  local_54 = local_40;
  local_50 = 0xffffffff;
  local_4c = 0;
  (**(code **)(*piVar8 + 0xa0))(piVar8,unaff_EBX + 0x32046a,this,this,&local_5c,0);
  return;
}


/* CNPC_PersonalityCore::CreateVPhysics at 006c6420 */

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


/* CNPC_PersonalityCore::PreferredCarryAngles at 006c6510 */

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
  vVar1 = *(vec_t *)(*(int *)(unaff_EBX + 0x6687bf) + 0x2c);
  vVar2 = *(vec_t *)(*(int *)(unaff_EBX + 0x66875f) + 0x2c);
  __return_storage_ptr__->x = *(vec_t *)(*(int *)(unaff_EBX + 0x6686ff) + 0x2c);
  __return_storage_ptr__->y = vVar2;
  __return_storage_ptr__->z = vVar1;
  return __return_storage_ptr__;
}


/* CNPC_PersonalityCore::StartSceneEvent at 006c7170 */

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
      fVar1 = *(float *)(**(int **)(unaff_EBX + 0x4e4a1c) + 0xc);
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


/* CNPC_PersonalityCore::PrescheduleThink at 006c7250 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void PrescheduleThink(CNPC_PersonalityCore * this) */

void __thiscall CNPC_PersonalityCore::PrescheduleThink(CNPC_PersonalityCore *this)

{
  uint *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  int iVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((*(float *)(**(int **)(unaff_EBX + 0x4e493c) + 0xc) <= this->m_flAnimResetTime) ||
     (this->m_flAnimResetTime == 0.0)) goto LAB_006c7304;
  iVar3 = this->m_iIdleOverrideSequence;
  if (iVar3 < 0) {
    if (this->m_bAttached != false) {
      iVar3 = CBaseAnimating::LookupSequence((CBaseAnimating *)this,(char *)(unaff_EBX + 0x3487b7));
      goto LAB_006c729f;
    }
    CAI_BaseNPC::SetIdealActivity((CAI_BaseNPC *)this,ACT_IDLE);
  }
  else {
LAB_006c729f:
    CAI_BaseNPC::SetIdealActivity((CAI_BaseNPC *)this,ACT_SPECIFIC_SEQUENCE);
    *(int *)((int)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                   super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                   super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                   super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC> +
            0xa00) = iVar3;
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
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar2->m_iChangeInfoSerialNumber = 0;
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
LAB_006c7304:
  CAI_PlayerAlly::PrescheduleThink(&this->super_CAI_PlayerAlly);
  return;
}


/* CNPC_PersonalityCore::IsOkToSpeakInResponseToPlayer at 006c63f0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: bool IsOkToSpeakInResponseToPlayer(CNPC_PersonalityCore * this) */

bool __thiscall CNPC_PersonalityCore::IsOkToSpeakInResponseToPlayer(CNPC_PersonalityCore *this)

{
  bool bVar1;
  
  bVar1 = CAI_PlayerAlly::IsOkToSpeak(&this->super_CAI_PlayerAlly,SPEECH_PRIORITY,true);
  return bVar1;
}


/* CNPC_PersonalityCore::StartTask at 006c5f40 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void StartTask(CNPC_PersonalityCore * this, Task_t * pTask) */

void __thiscall CNPC_PersonalityCore::StartTask(CNPC_PersonalityCore *this,Task_t *pTask)

{
  return;
}


/* CNPC_PersonalityCore::RunTask at 006c5f50 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void RunTask(CNPC_PersonalityCore * this, Task_t * pTask) */

void __thiscall CNPC_PersonalityCore::RunTask(CNPC_PersonalityCore *this,Task_t *pTask)

{
  return;
}


/* CNPC_PersonalityCore::InputEnablePickup at 006c5f60 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void InputEnablePickup(CNPC_PersonalityCore * this, inputdata_t *
   inputdata) */

void __thiscall
CNPC_PersonalityCore::InputEnablePickup(CNPC_PersonalityCore *this,inputdata_t *inputdata)

{
  this->m_bPickupEnabled = true;
  return;
}


/* CNPC_PersonalityCore::InputDisablePickup at 006c5f70 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void InputDisablePickup(CNPC_PersonalityCore * this, inputdata_t *
   inputdata) */

void __thiscall
CNPC_PersonalityCore::InputDisablePickup(CNPC_PersonalityCore *this,inputdata_t *inputdata)

{
  this->m_bPickupEnabled = false;
  return;
}


/* CNPC_PersonalityCore::InputPlayLock at 006c63a0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void InputPlayLock(CNPC_PersonalityCore * this, inputdata_t *
   inputdata) */

void __thiscall
CNPC_PersonalityCore::InputPlayLock(CNPC_PersonalityCore *this,inputdata_t *inputdata)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  InstancedScriptedScene
            ((CBaseFlex *)0x0,(char *)(unaff_EBX + 0x349490),(EHANDLE *)0x0,0.0,false,
             (AI_Response *)0x0,false,(IRecipientFilter *)0x0);
  return;
}


/* CNPC_PersonalityCore::InputPlayAttach at 006c6340 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void InputPlayAttach(CNPC_PersonalityCore * this, inputdata_t *
   inputdata) */

void __thiscall
CNPC_PersonalityCore::InputPlayAttach(CNPC_PersonalityCore *this,inputdata_t *inputdata)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  InstancedScriptedScene
            ((CBaseFlex *)0x0,(char *)(unaff_EBX + 0x3494bc),(EHANDLE *)0x0,0.0,false,
             (AI_Response *)0x0,false,(IRecipientFilter *)0x0);
  this->m_bAttached = true;
  return;
}


/* CNPC_PersonalityCore::InputPlayDetach at 006c5f80 */

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


/* CNPC_PersonalityCore::InputSetIdleSequence at 006c62e0 */

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
      label = (char *)(unaff_EBX + 0x2e586b);
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


/* CNPC_PersonalityCore::InputClearIdleSequence at 006c5fa0 */

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


/* CNPC_PersonalityCore::InputExplode at 006c7400 */

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
  ExplosionCreate(pVVar3,*(QAngle **)(unaff_EBX + 0x4e4833),(CBaseEntity *)this,100,500,0x529,0.0,
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
       (_func_int_varargs **)(unaff_EBX + 0x4e74f7);
  CRecipientFilter::AddRecipientsByPVS(&local_60,pVVar3);
                    /* Unresolved local var: Vector random@[???] */
  iVar4 = rand();
  local_34 = ((float)iVar4 / *(float *)(unaff_EBX + 0x3631f7)) * *(float *)(unaff_EBX + 0x36311b) +
             *(float *)(unaff_EBX + 0x36350b);
  iVar4 = rand();
  local_30 = ((float)iVar4 / *(float *)(unaff_EBX + 0x3631f7)) * *(float *)(unaff_EBX + 0x36311b) +
             *(float *)(unaff_EBX + 0x36350b);
  iVar4 = rand();
  local_2c = ((float)iVar4 / *(float *)(unaff_EBX + 0x3631f7)) * *(float *)(unaff_EBX + 0x36311b) +
             *(float *)(unaff_EBX + 0x36350b);
  local_28 = local_34;
  local_24 = local_30;
  puVar1 = *(undefined4 **)(unaff_EBX + 0x4e4797);
  pcVar2 = *(code **)(*(int *)*puVar1 + 0xc);
  local_20 = local_2c;
  pcVar5 = CPropData::GetRandomChunkModel
                     (*(CPropData **)(unaff_EBX + 0x4e4aa7),(char *)(unaff_EBX + 0x2f5243),-1);
  uVar6 = (*pcVar2)(*puVar1,pcVar5);
  iVar4 = 0;
                    /* Unresolved local var: int i@[???] */
  do {
    pcVar2 = *(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e483b) + 0x38);
    local_40 = 0x41800000;
    local_3c = 0x41800000;
    local_38 = 0x42900000;
    uVar7 = (**(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                                  super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                                  super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                                  super_CAI_BehaviorHost<CAI_BaseNPC>.
                                  super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0x274))(this);
    (*pcVar2)(**(undefined4 **)(unaff_EBX + 0x4e483b),&local_60,0,uVar7,
              *(undefined4 *)(unaff_EBX + 0x4e4833),&local_40,&local_28,uVar6,400,1,0x40200000,2);
    iVar4 = iVar4 + 1;
  } while (iVar4 != 0xc);
  thinkTime = *(float *)(unaff_EBX + 0x362f03) + *(float *)(**(int **)(unaff_EBX + 0x4e478f) + 0xc);
  CBaseEntity::SetNextThink((CBaseEntity *)this,thinkTime,(char *)0x0);
  local_60.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(unaff_EBX + 0x4e74f7);
  CRecipientFilter::~CRecipientFilter(&local_60,(int)thinkTime);
  return;
}


/* CNPC_PersonalityCore::InputEnableMotion at 006c73b0 */

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
                    /* WARNING: Could not recover jumptable at 0x006c73f6. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                        super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                        super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                        super_CAI_BehaviorHost<CAI_BaseNPC>.super_CAI_BehaviorHostBase<CAI_BaseNPC>
              + 0x27c))();
  return;
}


/* CNPC_PersonalityCore::EnableMotion at 006c5fc0 */

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
                    /* WARNING: Could not recover jumptable at 0x006c5ff2. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*pIVar1->_vptr_IPhysicsObject[0x19])();
    return;
  }
  return;
}


/* CNPC_PersonalityCore::InputDisableMotion at 006c6570 */

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
                    /* WARNING: Could not recover jumptable at 0x006c6590. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*pIVar1->_vptr_IPhysicsObject[0x10])();
    return;
  }
  return;
}


/* CNPC_PersonalityCore::InputEnableFlashlight at 006c6970 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void InputEnableFlashlight(CNPC_PersonalityCore * this, inputdata_t *
   inputdata) */

void __thiscall
CNPC_PersonalityCore::InputEnableFlashlight(CNPC_PersonalityCore *this,inputdata_t *inputdata)

{
  uint *puVar1;
  uint uVar2;
  int *piVar3;
  CBaseEdict *this_00;
  int iVar4;
  IChangeInfoAccessor *pIVar5;
  int unaff_EBX;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  
                    /* Unresolved local var: variant_t emptyVariant@[???] */
  ___i686_get_pc_thunk_bx();
  uVar2 = (this->m_hProjectedTexture).super_CBaseHandle.m_Index;
  if (((uVar2 != 0xffffffff) &&
      (iVar4 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e5227),
      *(uint *)(iVar4 + 8) == uVar2 >> 0x10)) &&
     (piVar3 = *(int **)(iVar4 + 4), piVar3 != (int *)0x0)) {
    local_20 = 0;
    local_34 = 0;
    local_30 = local_1c;
    local_2c = local_18;
    local_28 = 0xffffffff;
    local_24 = 0;
    (**(code **)(*piVar3 + 0xa0))(piVar3,unaff_EBX + 0x320713,this,this,&local_34,0);
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
        if (this_00 != (CBaseEdict *)0x0) {
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
          pIVar5 = CBaseEdict::GetChangeAccessor(this_00);
          pIVar5->m_iChangeInfoSerialNumber = 0;
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
      return;
    }
  }
  return;
}


/* CNPC_PersonalityCore::InputDisableFlashlight at 006c7b40 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void InputDisableFlashlight(CNPC_PersonalityCore * this, inputdata_t *
   inputdata) */

void __thiscall
CNPC_PersonalityCore::InputDisableFlashlight(CNPC_PersonalityCore *this,inputdata_t *inputdata)

{
  uint *puVar1;
  uint uVar2;
  CBaseEdict *this_00;
  int *piVar3;
  int iVar4;
  IChangeInfoAccessor *pIVar5;
  int unaff_EBX;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  
                    /* Unresolved local var: variant_t emptyVariant@[???] */
  ___i686_get_pc_thunk_bx();
  uVar2 = (this->m_hProjectedTexture).super_CBaseHandle.m_Index;
  if (((uVar2 != 0xffffffff) &&
      (iVar4 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4e4057),
      *(uint *)(iVar4 + 8) == uVar2 >> 0x10)) &&
     (piVar3 = *(int **)(iVar4 + 4), piVar3 != (int *)0x0)) {
    local_20 = 0;
    local_34 = 0;
    local_30 = local_1c;
    local_2c = local_18;
    local_28 = 0xffffffff;
    local_24 = 0;
    (**(code **)(*piVar3 + 0xa0))(piVar3,unaff_EBX + 0x31f54b,this,this,&local_34,0);
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
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar5 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar5->m_iChangeInfoSerialNumber = 0;
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


/* CNPC_PersonalityCore::OnTakeDamage_Alive at 006c67f0 */

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
    ResponseRules::CRR_Concept::CRR_Concept(local_24,(char *)(unaff_EBX + 0x3491ea));
    local_20 = 0xffffffff;
    (*pcVar1)(this,local_24,0,0,0,0);
  }
  return 1;
}


/* CNPC_PersonalityCore::SelectSchedule at 006c62d0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: int SelectSchedule(CNPC_PersonalityCore * this) */

int __thiscall CNPC_PersonalityCore::SelectSchedule(CNPC_PersonalityCore *this)

{
  int iVar1;
  
  iVar1 = CAI_PlayerAlly::SelectSchedule(&this->super_CAI_PlayerAlly);
  return iVar1;
}


/* CNPC_PersonalityCore::ShouldIgnite at 006c6010 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: bool ShouldIgnite(CNPC_PersonalityCore * this, CTakeDamageInfo * info)
    */

bool __thiscall CNPC_PersonalityCore::ShouldIgnite(CNPC_PersonalityCore *this,CTakeDamageInfo *info)

{
  return false;
}


/* CNPC_PersonalityCore::ShouldPlayIdleSound at 006c6020 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: bool ShouldPlayIdleSound(CNPC_PersonalityCore * this) */

bool __thiscall CNPC_PersonalityCore::ShouldPlayIdleSound(CNPC_PersonalityCore *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return this->m_flNextIdleSoundTime <= *(float *)(**(int **)(&DAT_004e5b78 + extraout_ECX) + 0xc)
         && *(float *)(**(int **)(&DAT_004e5b78 + extraout_ECX) + 0xc) !=
            this->m_flNextIdleSoundTime;
}


/* CNPC_PersonalityCore::IdleSound at 006c6760 */

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
  ResponseRules::CRR_Concept::CRR_Concept(local_24,(char *)(unaff_EBX + 0x339e98));
  local_20 = 0xffffffff;
  (*pcVar1)(this,local_24,0,0,0,0);
  this->m_flNextIdleSoundTime =
       *(float *)(**(int **)(unaff_EBX + 0x4e542c) + 0xc) +
       *(float *)(CAI_InterestTarget_t::IsThis + unaff_EBX);
  return;
}


/* CNPC_PersonalityCore::HandleAnimEvent at 006c7370 */

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
                    /* WARNING: Could not recover jumptable at 0x006c738f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*(pCVar1->super_CAI_Component)._vptr_CAI_Component[0x43])();
    return;
  }
  CAI_BaseNPC::HandleAnimEvent((CAI_BaseNPC *)this,pEvent);
  return;
}


/* CNPC_PersonalityCore::TranslateSchedule at 006c62c0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: int TranslateSchedule(CNPC_PersonalityCore * this, int scheduleType) */

int __thiscall CNPC_PersonalityCore::TranslateSchedule(CNPC_PersonalityCore *this,int scheduleType)

{
  int iVar1;
  
  iVar1 = CAI_PlayerAlly::TranslateSchedule(&this->super_CAI_PlayerAlly,scheduleType);
  return iVar1;
}


/* CNPC_PersonalityCore::GatherConditions at 006c62b0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void GatherConditions(CNPC_PersonalityCore * this) */

void __thiscall CNPC_PersonalityCore::GatherConditions(CNPC_PersonalityCore *this)

{
  CAI_PlayerAlly::GatherConditions(&this->super_CAI_PlayerAlly);
  return;
}


/* CNPC_PersonalityCore::Use at 006c6050 */

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
                    /* WARNING: Could not recover jumptable at 0x006c60c1. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (*(pActivator->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
        _vptr_IHandleEntity[0x1b7])();
      return;
    }
  }
  return;
}


/* CNPC_PersonalityCore::OnPhysGunPickup at 006c66b0 */

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
    ResponseRules::CRR_Concept::CRR_Concept(local_24,(char *)(unaff_EBX + 0x349317));
    local_20 = 0xffffffff;
    (*pcVar1)(this,local_24,0,0,0,0);
    this->m_bHasBeenPickedUp = true;
    COutputEvent::FireOutput
              (&this->m_OnPlayerPickup,(CBaseEntity *)pPhysGunUser,(CBaseEntity *)this,0.0);
  }
  return;
}


/* CNPC_PersonalityCore::OnPhysGunDrop at 006c65c0 */

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
    ResponseRules::CRR_Concept::CRR_Concept(this_00,(char *)(unaff_EBX + 0x3493e6));
    local_20 = 0xffffffff;
  }
  else {
    if (reason != THROWN_BY_PLAYER) goto LAB_006c65e2;
    pcVar1 = *(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                                 super_CAI_ExpresserHost<CAI_BaseHumanoid>.super_CAI_BaseHumanoid.
                                 super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                                 super_CAI_BehaviorHost<CAI_BaseNPC>.
                                 super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0xa70);
    this_00 = local_2c;
    ResponseRules::CRR_Concept::CRR_Concept(this_00,(char *)(unaff_EBX + 0x3493f6));
    local_28 = 0xffffffff;
  }
  (*pcVar1)(this,this_00,0,0,0,0);
LAB_006c65e2:
  COutputEvent::FireOutput
            (&this->m_OnPlayerDrop,(CBaseEntity *)pPhysGunUser,(CBaseEntity *)this,0.0);
  return;
}


/* CNPC_PersonalityCore::OnFizzled at 006c68f0 */

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
  ResponseRules::CRR_Concept::CRR_Concept(local_24,(char *)(unaff_EBX + 0x349102));
  local_20 = 0xffffffff;
  (*pcVar1)(this,local_24,0,0,0,0);
  CBaseAnimating::OnFizzled((CBaseAnimating *)this);
  return;
}


/* CNPC_PersonalityCore::IsBeingHeldByPlayer at 006c6240 */

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
  piVar1 = *(int **)(&DAT_004e5952 + unaff_EBX);
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


/* CNPC_PersonalityCore::ModifyOrAppendCriteria at 006c7700 */

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
  CBaseEntity *pCVar5;
  char *pcVar6;
  int unaff_EBX;
  int iVar7;
  float fVar8;
  
  ___i686_get_pc_thunk_bx();
  CAI_PlayerAlly::ModifyOrAppendCriteria(&this->super_CAI_PlayerAlly,set);
  ResponseRules::CriteriaSet::AppendCriteria
            (set,(char *)(unaff_EBX + 0x348326),
             *(char **)(unaff_EBX + 0x603c72 + this->m_CoreType * 4),1.0);
  if (0 < *(int *)(**(int **)(unaff_EBX + 0x4e4492) + 0x14)) {
    iVar7 = 1;
    do {
      pCVar3 = UTIL_PlayerByIndex(iVar7);
      if (pCVar3 != (CBasePlayer *)0x0) {
        pCVar4 = (CNPC_PersonalityCore *)GetPlayerHeldEntity(pCVar3);
        if (pCVar4 == this) {
          pcVar6 = (char *)(unaff_EBX + 0x31d1bc);
          goto LAB_006c77a0;
        }
      }
      iVar7 = iVar7 + 1;
    } while (iVar7 <= *(int *)(**(int **)(unaff_EBX + 0x4e4492) + 0x14));
  }
  pcVar6 = (char *)(unaff_EBX + 0x2ee8ea);
LAB_006c77a0:
  ResponseRules::CriteriaSet::AppendCriteria(set,(char *)(unaff_EBX + 0x348333),pcVar6,1.0);
  pcVar6 = (char *)(unaff_EBX + 0x31d1bc);
  if (this->m_bHasBeenPickedUp == false) {
    pcVar6 = (char *)(unaff_EBX + 0x2ee8ea);
  }
  ResponseRules::CriteriaSet::AppendCriteria(set,(char *)(unaff_EBX + 0x34833d),pcVar6,1.0);
  if (0 < *(int *)(**(int **)(unaff_EBX + 0x4e4492) + 0x14)) {
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
        goto LAB_006c787b;
      }
      iVar7 = iVar7 + 1;
    } while (iVar7 <= *(int *)(**(int **)(unaff_EBX + 0x4e4492) + 0x14));
  }
  fVar8 = 0.0;
LAB_006c787b:
  ResponseRules::CriteriaSet::AppendCriteria(set,(char *)(unaff_EBX + 0x34834c),fVar8,1.0);
                    /* Unresolved local var: CBasePlayer * pPlayer@[???] */
  if (0 < *(int *)(**(int **)(unaff_EBX + 0x4e4492) + 0x14)) {
    iVar7 = 1;
    do {
      pCVar3 = UTIL_PlayerByIndex(iVar7);
                    /* Unresolved local var: CBasePlayer * pPlayer@[???] */
      if (pCVar3 != (CBasePlayer *)0x0) {
        pCVar5 = GetPlayerHeldEntity(pCVar3);
                    /* Unresolved local var: CBaseEntity * heldItem@[???] */
        if (pCVar5 != (CBaseEntity *)0x0) {
          pcVar6 = (pCVar5->m_iClassname).pszValue;
          if (pcVar6 == (char *)0x0) {
            pcVar6 = (char *)(unaff_EBX + 0x2e444e);
          }
          goto LAB_006c78fb;
        }
        break;
      }
      iVar7 = iVar7 + 1;
    } while (iVar7 <= *(int *)(**(int **)(unaff_EBX + 0x4e4492) + 0x14));
  }
  pcVar6 = *(char **)(unaff_EBX + 0x56d7f2);
LAB_006c78fb:
  ResponseRules::CriteriaSet::AppendCriteria(set,(char *)(unaff_EBX + 0x348359),pcVar6,1.0);
  return;
}


/* CNPC_PersonalityCore::VPhysicsCollision at 006c7940 */

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
  if (*(float *)(unaff_EBX + 0x3629c6) <=
      *(float *)(**(int **)(unaff_EBX + 0x4e4252) + 0xc) - this->m_flLastPhysicsImpactTime) {
    piVar3 = *(int **)((int)pEvent->pEntities + (-(uint)(index == 0) & 4));
    if (piVar3 == (int *)**(int **)(&DAT_004e430e + unaff_EBX)) {
      if (0 < *(int *)(**(int **)(unaff_EBX + 0x4e4252) + 0x14)) {
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
            ResponseRules::CRR_Concept::CRR_Concept(this_00,(char *)(unaff_EBX + 0x34812a));
            local_20 = 0xffffffff;
            goto LAB_006c7a6e;
          }
          playerIndex = playerIndex + 1;
        } while (playerIndex <= *(int *)(**(int **)(unaff_EBX + 0x4e4252) + 0x14));
      }
      fVar7 = pEvent->preVelocity[index].z;
      fVar1 = pEvent->preVelocity[index].y;
      fVar2 = pEvent->preVelocity[index].x;
      fVar7 = fVar2 * fVar2 + fVar1 * fVar1 + fVar7 * fVar7;
      if (*(float *)(unaff_EBX + 0x363046) <= fVar7 && fVar7 != *(float *)(unaff_EBX + 0x363046)) {
        pcVar6 = *(code **)(*(int *)&(this->super_CAI_PlayerAlly).super_CAI_BaseActor.
                                     super_CAI_ExpresserHost<CAI_BaseHumanoid>.
                                     super_CAI_BaseHumanoid.
                                     super_CAI_BlendingHost<CAI_BehaviorHost<CAI_BaseNPC>_>.
                                     super_CAI_BehaviorHost<CAI_BaseNPC>.
                                     super_CAI_BehaviorHostBase<CAI_BaseNPC> + 0xa70);
        ResponseRules::CRR_Concept::CRR_Concept(local_2c,(char *)(unaff_EBX + 0x348142));
        local_28 = 0xffffffff;
        (*pcVar6)(this,local_2c,0,0,0,0);
        this->m_flLastPhysicsImpactTime = *(float *)(**(int **)(unaff_EBX + 0x4e4252) + 0xc);
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
        ResponseRules::CRR_Concept::CRR_Concept(this_00,(char *)(unaff_EBX + 0x348142));
        local_30 = 0xffffffff;
LAB_006c7a6e:
        (*pcVar6)(this,this_00,0,0,0,0);
        this->m_flLastPhysicsImpactTime = *(float *)(**(int **)(unaff_EBX + 0x4e4252) + 0xc);
        return;
      }
    }
  }
  return;
}


/* CNPC_PersonalityCore::GetFirstPlayer at 006c61f0 */

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
  piVar1 = *(int **)(CFuncTank::FuncTankPreThink + unaff_EBX + 2);
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


/* CNPC_PersonalityCore::GetPlayerHeldEntityName at 006c6a60 */

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
  piVar1 = *(int **)(unaff_EBX + 0x4e5132);
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
          return (char *)(unaff_EBX + 0x2e50ee);
        }
        break;
      }
      playerIndex = playerIndex + 1;
    } while (playerIndex <= *(int *)(*piVar1 + 0x14));
  }
  return *(char **)(unaff_EBX + 0x56e492);
}


/* CNPC_PersonalityCore::GetPlayerSpeed at 006c6ad0 */

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
  if (0 < *(int *)(**(int **)(unaff_EBX + 0x4e50c2) + 0x14)) {
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
    } while (playerIndex <= *(int *)(**(int **)(unaff_EBX + 0x4e50c2) + 0x14));
  }
  return 0.0;
}


/* CNPC_PersonalityCore::GetCoreTypeName at 006c60e0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: char * GetCoreTypeName(CNPC_PersonalityCore * this) */

char * __thiscall CNPC_PersonalityCore::GetCoreTypeName(CNPC_PersonalityCore *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(char **)(extraout_ECX + 0x605298 + this->m_CoreType * 4);
}


/* CNPC_PersonalityCore::TestRemarkingUpon at 006c6100 */

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


/* CNPC_PersonalityCore::NotifySystemEvent at 006c6870 */

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
    ResponseRules::CRR_Concept::CRR_Concept(local_24,&UNK_00349175 + unaff_EBX);
    local_20 = 0xffffffff;
    (*pcVar1)(this,local_24,0,0,0,0);
  }
  return;
}


/* CNPC_PersonalityCore::LoadSchedules at 006c6130 */

bool CNPC_PersonalityCore::LoadSchedules(void)

{
  int *piVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CAI_PlayerAlly::LoadSchedules();
  piVar1 = *(int **)(unaff_EBX + 0x4e5f5c);
  if (*(int *)(unaff_EBX + 0x627534) != *piVar1) {
    InitCustomSchedules();
    mp_disable_autokick[unaff_EBX] = (code)0x1;
    *(int *)(unaff_EBX + 0x627534) = *piVar1;
  }
  return (bool)mp_disable_autokick[unaff_EBX];
}


/* CNPC_PersonalityCore::LoadedSchedules at 006c6190 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: bool LoadedSchedules(CNPC_PersonalityCore * this) */

bool __thiscall CNPC_PersonalityCore::LoadedSchedules(CNPC_PersonalityCore *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (bool)(&DAT_006274dc)[extraout_ECX];
}


/* CNPC_PersonalityCore::SquadSlotName at 006c61b0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: char * SquadSlotName(CNPC_PersonalityCore * this, int slotEN) */

char * __thiscall CNPC_PersonalityCore::SquadSlotName(CNPC_PersonalityCore *this,int slotEN)

{
  int symbolID;
  char *pcVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  symbolID = CAI_LocalIdSpace::LocalToGlobal((CAI_LocalIdSpace *)(unaff_EBX + 0x668bd0),slotEN);
  pcVar1 = CAI_GlobalNamespace::IdToSymbol(*(CAI_GlobalNamespace **)(unaff_EBX + 0x4e5e94),symbolID)
  ;
  return pcVar1;
}


/* CNPC_PersonalityCore::CScheduleLoader::CScheduleLoader at 006c6b80 */

/* DWARF original prototype: void CScheduleLoader(CScheduleLoader * this) */

void __thiscall CNPC_PersonalityCore::CScheduleLoader::CScheduleLoader(CScheduleLoader *this)

{
  int *piVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CAI_PlayerAlly::LoadSchedules();
  piVar1 = *(int **)(unaff_EBX + 0x4e550c);
  if (*(int *)(unaff_EBX + 0x626ae4) != *piVar1) {
    CNPC_PersonalityCore::InitCustomSchedules();
    *(undefined1 *)(unaff_EBX + 0x626ae0) = 1;
    *(int *)(unaff_EBX + 0x626ae4) = *piVar1;
  }
  return;
}


/* CNPC_PersonalityCore::CScheduleLoader::CScheduleLoader at 006c6bd0 */

/* DWARF original prototype: void CScheduleLoader(CScheduleLoader * this) */

void __thiscall CNPC_PersonalityCore::CScheduleLoader::CScheduleLoader(CScheduleLoader *this)

{
  int *piVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CAI_PlayerAlly::LoadSchedules();
  piVar1 = *(int **)(unaff_EBX + 0x4e54bc);
  if (*(int *)(unaff_EBX + 0x626a94) != *piVar1) {
    CNPC_PersonalityCore::InitCustomSchedules();
    *(undefined1 *)(unaff_EBX + 0x626a90) = 1;
    *(int *)(unaff_EBX + 0x626a94) = *piVar1;
  }
  return;
}


/* CNPC_PersonalityCore::InitCustomSchedules at 006c7c20 */

/* WARNING: Removing unreachable block (ram,0x006c7eab) */
/* WARNING: Removing unreachable block (ram,0x006c8454) */
/* WARNING: Removing unreachable block (ram,0x006c8422) */
/* WARNING: Removing unreachable block (ram,0x006c80e0) */
/* WARNING: Removing unreachable block (ram,0x006c80fa) */
/* WARNING: Removing unreachable block (ram,0x006c8133) */
/* WARNING: Removing unreachable block (ram,0x006c80f0) */
/* WARNING: Removing unreachable block (ram,0x006c7dff) */
/* WARNING: Removing unreachable block (ram,0x006c7df4) */
/* WARNING: Removing unreachable block (ram,0x006c81e0) */
/* WARNING: Removing unreachable block (ram,0x006c81f0) */
/* WARNING: Removing unreachable block (ram,0x006c81f7) */
/* WARNING: Removing unreachable block (ram,0x006c8225) */
/* WARNING: Removing unreachable block (ram,0x006c8253) */
/* WARNING: Removing unreachable block (ram,0x006c8210) */
/* WARNING: Removing unreachable block (ram,0x006c8275) */
/* WARNING: Removing unreachable block (ram,0x006c8282) */
/* WARNING: Removing unreachable block (ram,0x006c7edd) */

void CNPC_PersonalityCore::InitCustomSchedules(void)

{
  CAI_GlobalNamespace *pCVar1;
  AI_NamespaceAddInfo_t *pAVar2;
  CStringRegistry *pCVar3;
  CStringRegistry *pCVar4;
  bool bVar5;
  int iVar6;
  int unaff_EBX;
  int iVar7;
  CAI_GlobalNamespace *pGlobalNamespace;
  int local_b0;
  undefined1 *local_a8;
  int local_a4;
  undefined1 *local_9c;
  int local_98;
  undefined1 *local_90;
  int local_7c;
  int local_78;
  int local_74;
  int local_70;
  CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t,_int>_> local_58;
  CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t,_int>_> local_44;
  CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t,_int>_> local_30;
  
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
  local_30.m_Memory.m_pMemory = (AI_NamespaceAddInfo_t *)0x0;
  local_30.m_Memory.m_nAllocationCount = 0;
  local_30.m_Memory.m_nGrowSize = 0;
  local_30.m_Size = 0;
  local_30.m_pElements = (AI_NamespaceAddInfo_t *)0x0;
  local_44.m_Memory.m_pMemory = (AI_NamespaceAddInfo_t *)0x0;
  local_44.m_Memory.m_nAllocationCount = 0;
  local_44.m_Memory.m_nGrowSize = 0;
  local_44.m_Size = 0;
  local_44.m_pElements = (AI_NamespaceAddInfo_t *)0x0;
  local_58.m_Memory.m_pMemory = (AI_NamespaceAddInfo_t *)0x0;
  local_58.m_Memory.m_nAllocationCount = 0;
  local_58.m_Memory.m_nGrowSize = 0;
  local_58.m_Size = 0;
  local_58.m_pElements = (AI_NamespaceAddInfo_t *)0x0;
  *(int *)(unaff_EBX + 0x66710f) = unaff_EBX + 0x347c69;
  iVar7 = *(int *)(unaff_EBX + 0x4e493f);
  bVar5 = CAI_LocalIdSpace::Init
                    ((CAI_LocalIdSpace *)(unaff_EBX + 0x667113),
                     *(CAI_GlobalNamespace **)(&DAT_004e441b + unaff_EBX),
                     (CAI_LocalIdSpace *)(iVar7 + 4));
  if ((bVar5) &&
     (bVar5 = CAI_LocalIdSpace::Init
                        ((CAI_LocalIdSpace *)(unaff_EBX + 0x66712b),
                         (CAI_GlobalNamespace *)(*(int *)(&DAT_004e441b + unaff_EBX) + 8),
                         (CAI_LocalIdSpace *)(iVar7 + 0x1c)), bVar5)) {
    CAI_LocalIdSpace::Init
              ((CAI_LocalIdSpace *)(unaff_EBX + 0x667143),
               (CAI_GlobalNamespace *)(*(int *)(&DAT_004e441b + unaff_EBX) + 0x10),
               (CAI_LocalIdSpace *)(iVar7 + 0x34));
  }
  pGlobalNamespace = *(CAI_GlobalNamespace **)(&DAT_004e441f + unaff_EBX);
  CAI_LocalIdSpace::Init
            ((CAI_LocalIdSpace *)(unaff_EBX + 0x66715b),pGlobalNamespace,
             *(CAI_LocalIdSpace **)(unaff_EBX + 0x4e494b));
  if (1 < local_30.m_Size) {
    if (local_30.m_Memory.m_pMemory == (AI_NamespaceAddInfo_t *)0x0) {
      local_a8 = (undefined1 *)
                 ((int)&((CAI_GlobalNamespace *)(local_30.m_Size + -8))->m_NextGlobalBase + 3);
      do {
        if (0 < (int)local_a8) {
          local_b0 = 1;
          local_74 = 0;
          do {
            pCVar1 = (CAI_GlobalNamespace *)(local_30.m_Memory.m_pMemory + local_b0);
            pAVar2 = (AI_NamespaceAddInfo_t *)
                     ((int)&(local_30.m_Memory.m_pMemory)->pszName + local_74);
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
      pGlobalNamespace = (CAI_GlobalNamespace *)local_30.m_Size;
      _qsort(local_30.m_Memory.m_pMemory,local_30.m_Size,8,*(int **)(unaff_EBX + 0x603dcf));
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
      _qsort(local_44.m_Memory.m_pMemory,local_44.m_Size,8,*(int **)(unaff_EBX + 0x603dcf));
    }
  }
  if (1 < local_58.m_Size) {
    if ((CAI_GlobalNamespace *)local_58.m_Memory.m_pMemory == (CAI_GlobalNamespace *)0x0) {
      local_90 = (undefined1 *)
                 ((int)&((CAI_GlobalNamespace *)(local_58.m_Size + -8))->m_NextGlobalBase + 3);
      do {
        if (0 < (int)local_90) {
          local_98 = 1;
          local_7c = 0;
          do {
            pCVar1 = (CAI_GlobalNamespace *)(local_58.m_Memory.m_pMemory + local_98);
            pAVar2 = (AI_NamespaceAddInfo_t *)
                     ((int)&(local_58.m_Memory.m_pMemory)->pszName + local_7c);
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
            local_98 = local_98 + 1;
            local_7c = local_7c + 8;
          } while (local_98 <= (int)local_90);
        }
        local_90 = local_90 + -1;
      } while (local_90 != (undefined1 *)0xffffffff);
    }
    else {
      pGlobalNamespace = (CAI_GlobalNamespace *)local_58.m_Size;
      _qsort(local_58.m_Memory.m_pMemory,local_58.m_Size,8,*(int **)(unaff_EBX + 0x603dcf));
    }
  }
  if (0 < local_30.m_Size) {
    iVar7 = 0;
    do {
      pGlobalNamespace = (CAI_GlobalNamespace *)local_30.m_Memory.m_pMemory[iVar7].pszName;
      bVar5 = CAI_LocalIdSpace::AddSymbol
                        ((CAI_LocalIdSpace *)(unaff_EBX + 0x667113),(char *)pGlobalNamespace,
                         local_30.m_Memory.m_pMemory[iVar7].localId,(char *)(unaff_EBX + 0x32cc1b),
                         *(char **)(unaff_EBX + 0x60378b));
      if (!bVar5) goto LAB_006c7e93;
      iVar7 = iVar7 + 1;
    } while (iVar7 < local_30.m_Size);
  }
  if (0 < local_44.m_Size) {
    iVar7 = 0;
    do {
      pGlobalNamespace = (CAI_GlobalNamespace *)local_44.m_Memory.m_pMemory[iVar7].pszName;
      bVar5 = CAI_LocalIdSpace::AddSymbol
                        ((CAI_LocalIdSpace *)(unaff_EBX + 0x66712b),(char *)pGlobalNamespace,
                         local_44.m_Memory.m_pMemory[iVar7].localId,(char *)(unaff_EBX + 0x3016fb),
                         *(char **)(unaff_EBX + 0x60378b));
      if (!bVar5) goto LAB_006c7e93;
      iVar7 = iVar7 + 1;
    } while (iVar7 < local_44.m_Size);
  }
  if (0 < local_58.m_Size) {
    local_70 = 0;
    iVar7 = *(int *)(&DAT_004e441b + unaff_EBX);
    do {
      while( true ) {
        pCVar3 = ((CAI_GlobalNamespace *)(local_58.m_Memory.m_pMemory + local_70))->m_pSymbols;
        iVar6 = CAI_GlobalNamespace::NextGlobalBase((CAI_GlobalNamespace *)(iVar7 + 0x10));
        if (0xff < iVar6 + -1000000000) break;
        pGlobalNamespace =
             (CAI_GlobalNamespace *)
             ((CAI_GlobalNamespace *)(local_58.m_Memory.m_pMemory + local_70))->m_pSymbols;
        bVar5 = CAI_LocalIdSpace::AddSymbol
                          ((CAI_LocalIdSpace *)(unaff_EBX + 0x667143),(char *)pGlobalNamespace,
                           ((CAI_GlobalNamespace *)(local_58.m_Memory.m_pMemory + local_70))->
                           m_NextGlobalBase,&UNK_00308793 + unaff_EBX,
                           *(char **)(unaff_EBX + 0x60378b));
        if (!bVar5) {
LAB_006c7e93:
          local_58.m_Size = 0;
          if (-1 < local_58.m_Memory.m_nGrowSize) {
            if ((CAI_GlobalNamespace *)local_58.m_Memory.m_pMemory != (CAI_GlobalNamespace *)0x0) {
              pGlobalNamespace = (CAI_GlobalNamespace *)local_58.m_Memory.m_pMemory;
              (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e3f67) + 8))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x4e3f67));
              local_58.m_Memory.m_pMemory = (AI_NamespaceAddInfo_t *)0x0;
            }
            local_58.m_Memory.m_nAllocationCount = 0;
          }
          local_58.m_pElements = local_58.m_Memory.m_pMemory;
          if (-1 < local_58.m_Memory.m_nGrowSize) {
            if ((CAI_GlobalNamespace *)local_58.m_Memory.m_pMemory != (CAI_GlobalNamespace *)0x0) {
              pGlobalNamespace = (CAI_GlobalNamespace *)local_58.m_Memory.m_pMemory;
              (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e3f67) + 8))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x4e3f67));
              local_58.m_Memory.m_pMemory = (AI_NamespaceAddInfo_t *)0x0;
            }
            local_58.m_Memory.m_nAllocationCount = 0;
          }
          CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t,_int>_>::~CUtlVector
                    (&local_44,(int)pGlobalNamespace);
          CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t,_int>_>::~CUtlVector
                    (&local_30,(int)pGlobalNamespace);
          return;
        }
        local_70 = local_70 + 1;
        if (local_58.m_Size <= local_70) goto LAB_006c80d5;
      }
      pGlobalNamespace = (CAI_GlobalNamespace *)&section_000000dc.size;
      DevWarning((char *)(unaff_EBX + 0x2fc1b3),0x100,pCVar3);
      local_70 = local_70 + 1;
    } while (local_70 < local_58.m_Size);
  }
LAB_006c80d5:
  CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t,_int>_>::~CUtlVector
            (&local_58,(int)pGlobalNamespace);
  local_44.m_Size = 0;
  if (-1 < local_44.m_Memory.m_nGrowSize) {
    if (local_44.m_Memory.m_pMemory != (AI_NamespaceAddInfo_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e3f67) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4e3f67),local_44.m_Memory.m_pMemory);
      local_44.m_Memory.m_pMemory = (AI_NamespaceAddInfo_t *)0x0;
    }
    local_44.m_Memory.m_nAllocationCount = 0;
  }
  local_44.m_pElements = local_44.m_Memory.m_pMemory;
  if (-1 < local_44.m_Memory.m_nGrowSize) {
    if (local_44.m_Memory.m_pMemory != (AI_NamespaceAddInfo_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e3f67) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4e3f67),local_44.m_Memory.m_pMemory);
      local_44.m_Memory.m_pMemory = (AI_NamespaceAddInfo_t *)0x0;
    }
    local_44.m_Memory.m_nAllocationCount = 0;
  }
  local_30.m_Size = 0;
  if (-1 < local_30.m_Memory.m_nGrowSize) {
    if (local_30.m_Memory.m_pMemory != (AI_NamespaceAddInfo_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e3f67) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4e3f67),local_30.m_Memory.m_pMemory);
      local_30.m_Memory.m_pMemory = (AI_NamespaceAddInfo_t *)0x0;
    }
    local_30.m_Memory.m_nAllocationCount = 0;
  }
  local_30.m_pElements = local_30.m_Memory.m_pMemory;
  if ((-1 < local_30.m_Memory.m_nGrowSize) &&
     (local_30.m_Memory.m_pMemory != (AI_NamespaceAddInfo_t *)0x0)) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4e3f67) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x4e3f67),local_30.m_Memory.m_pMemory);
  }
  return;
}


/* _GLOBAL__I_sv_personality_core_pca_pitch at 000c2460 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_sv_personality_core_pca_pitch(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

