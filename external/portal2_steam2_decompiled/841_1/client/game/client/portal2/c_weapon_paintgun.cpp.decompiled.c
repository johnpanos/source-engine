/* DWARF-guided pseudocode for game/client/portal2/c_weapon_paintgun.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 0007a5c0 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  RecvTable *this;
  undefined4 uVar1;
  int *piVar2;
  int iVar3;
  IClassMap *pIVar4;
  int unaff_EBX;
  longlong lVar5;
  ConVar *this_1;
  ConVar *in_stack_ffffffd8;
  char *in_stack_ffffffdc;
  char *in_stack_ffffffe0;
  int in_stack_ffffffe4;
  char *in_stack_ffffffe8;
  
  lVar5 = ___i686_get_pc_thunk_bx();
  if (lVar5 == 0xffff00000001) {
    (&DAT_00c5924c)[unaff_EBX] = 0;
    (&DAT_00c5924d)[unaff_EBX] = 0;
    (&DAT_00c5924e)[unaff_EBX] = 0;
    (&DAT_00c5924f)[unaff_EBX] = 0;
    *(undefined4 *)((int)&CHudCloseCaption::GetKBMap::s_pMap + unaff_EBX) = 0;
    *(undefined4 *)((int)&CHudCloseCaption::GetMessageMap::s_pMap + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&CUtlRBTree<AsyncCaption_t::BlockInfo_t,unsigned_short,bool(*)(AsyncCaption_t::BlockInfo_t_const&,AsyncCaption_t::BlockInfo_t_const&),CUtlMemory<UtlRBTreeNode_t<AsyncCaption_t::BlockInfo_t,unsigned_short>,unsigned_short>>
            ::Links(unsigned_short)::s_Sentinel + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00c5925c + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)
     ((int)&CUtlRBTree<CHudCloseCaption::CaptionRepeat,int,bool(*)(CHudCloseCaption::CaptionRepeat_const&,CHudCloseCaption::CaptionRepeat_const&),CUtlMemory<UtlRBTreeNode_t<CHudCloseCaption::CaptionRepeat,int>,int>>
            ::Links(int)::s_Sentinel + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00c59264 + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00c59268 + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00c5926c + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&CUtlLinkedList<CAsyncCaption*,short_unsigned_int,false,short_unsigned_int,CUtlMemory<UtlLinkedListElem_t<CAsyncCaption*,_short_unsigned_int>,_short_unsigned_int>_>
            ::AllocInternal::__executeCount_1 + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)
     ((int)&CUtlLinkedList<CAsyncCaption*,short_unsigned_int,false,short_unsigned_int,CUtlMemory<UtlLinkedListElem_t<CAsyncCaption*,_short_unsigned_int>,_short_unsigned_int>_>
            ::AllocInternal::__executeCount + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&CHudCloseCaption::ChainToMap()::chained + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)
     (&CHudCloseCaption::PanelAnimationVar_m_flGrowTime::InitVar()::bAdded + unaff_EBX) = 0x7f7fffff
    ;
    *(undefined **)
     ((int)&CHudCloseCaption::PanelAnimationVar_m_nTopOffset::InitVar()::bAdded + unaff_EBX) =
         &UNK_00aea8b4 + unaff_EBX;
    this_1 = (ConVar *)(unaff_EBX + 0x94135c);
    ConVar::ConVar((ConVar *)((int)s_SIMDRandContexts[0x18].m_RandY[0] + unaff_EBX + 0xc),
                   (char *)(unaff_EBX + 0x94139e),(char *)(unaff_EBX + 0x941390),0x4000,
                   (char *)this_1,in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0,
                   in_stack_ffffffe4,in_stack_ffffffe8);
    uVar1 = *(undefined4 *)(&DAT_00ae9798 + unaff_EBX);
    ___cxa_atexit(unaff_EBX + 0x90202c,0,uVar1);
    ConVar::ConVar((ConVar *)((int)s_SIMDRandContexts[0x18].m_RandY[6] + unaff_EBX + 0xc),
                   (char *)(unaff_EBX + 0x9413b6),(char *)(unaff_EBX + 0x9088e8),0,this_1,
                   (char *)in_stack_ffffffd8,in_stack_ffffffdc,(int)in_stack_ffffffe0);
    ___cxa_atexit(unaff_EBX + 0x90200c,0,uVar1);
    *(int *)((int)s_SIMDRandContexts[0x18].m_RandY[0xc] + unaff_EBX + 0xc) = unaff_EBX + 0x9413d1;
    *(int *)((int)s_SIMDRandContexts[0x18].m_RandY[0xc] + unaff_EBX + 4) = unaff_EBX + 0x4c6b8c;
    *(undefined4 *)((int)s_SIMDRandContexts[0x18].m_RandY[0xc] + unaff_EBX + 8) = 0;
    this = (RecvTable *)((int)s_SIMDRandContexts[0x18].m_RandY[0xd] + unaff_EBX + 0xc);
    *(RecvTable **)((int)s_SIMDRandContexts[0x18].m_RandY[0xd] + unaff_EBX) = this;
    piVar2 = *(int **)(&DAT_00ae9810 + unaff_EBX);
    *(int *)((int)s_SIMDRandContexts[0x18].m_RandY[0xd] + unaff_EBX + 4) = *piVar2;
    *piVar2 = unaff_EBX + 0xcef1c4;
    RecvTable::RecvTable(this);
    ___cxa_atexit(unaff_EBX + 0x901fec,0,uVar1);
    iVar3 = ClientClassInit<DT_WeaponPaintGun::ignored>((ignored *)0x0);
    *(int *)((int)s_SIMDRandContexts[0x18].m_RandY[0xf] + unaff_EBX) = iVar3;
    *(undefined4 *)((int)&PTR_Think_00bb26d4 + unaff_EBX) = 4;
    *(int *)((int)&PTR_ShouldPredict_00bb26d0 + unaff_EBX) =
         (int)&PTR_MyCombatWeaponPointer_00bb272c + unaff_EBX;
    *(int *)((int)s_SIMDRandContexts[0x18].m_RandY[0xf] + unaff_EBX + 4) =
         (int)&PTR_ShouldPredict_00bb26d0 + unaff_EBX;
    pIVar4 = GetClassMap();
    (*pIVar4->_vptr_IClassMap[2])
              (pIVar4,unaff_EBX + 0x9413f2,unaff_EBX + 0x9413e1,0xeac,unaff_EBX + 0x4c6b4c);
    ConCommand::ConCommand
              ((ConCommand *)(unaff_EBX + 0xc5928c),(char *)(unaff_EBX + 0x941402),
               (FnCommandCallbackV1_t)(unaff_EBX + 0x4c6e6c),(char *)0x0,0,
               (FnCommandCompletionCallback)0x0);
    ___cxa_atexit(unaff_EBX + 0x901fcc,0,uVar1);
    ConCommand::ConCommand
              ((ConCommand *)(&DAT_00c592cc + unaff_EBX),(char *)(unaff_EBX + 0x94140c),
               (FnCommandCallbackV1_t)(unaff_EBX + 0x4c6dfc),(char *)0x0,0,
               (FnCommandCompletionCallback)0x0);
    ___cxa_atexit(unaff_EBX + 0x901fac,0,uVar1);
    ConCommand::ConCommand
              ((ConCommand *)(unaff_EBX + 0xc5930c),(char *)(unaff_EBX + 0x941416),
               (FnCommandCallback_t)(unaff_EBX + 0x4c859c),(char *)0x0,0,
               (FnCommandCompletionCallback)0x0);
    ___cxa_atexit(unaff_EBX + 0x901f8c,0,uVar1);
  }
  return;
}


/* __tcf_0 at 0097c600 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ed0d8),in_stack_00000008);
  return;
}


/* __tcf_1 at 0097c5e0 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ed158),in_stack_00000008);
  return;
}


/* ChangePaintTo at 00542b70 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void ChangePaintTo(CCommand *args)

{
  int iVar1;
  C_BasePlayer *pCVar2;
  C_WeaponPaintGun *pCVar3;
  char *pcVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (args->m_nArgc != 2) {
    DevMsg((char *)(unaff_EBX + 0x478f0b));
    return;
  }
  iVar1 = _V_stricmp(args->m_ppArgv[1],(char *)(unaff_EBX + 0x478f2c));
  if (iVar1 == 0) {
                    /* Unresolved local var: C_BasePlayer * pPlayer@[???]
                       Unresolved local var: C_WeaponPaintGun * pPaintGun@[???] */
    pCVar2 = C_BasePlayer::GetLocalPlayer(-1);
    if (pCVar2 != (C_BasePlayer *)0x0) {
      iVar1 = (**(code **)(*(int *)&(pCVar2->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                    super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                    super_C_BaseEntity.super_IClientEntity.super_IClientUnknown +
                          0x490))(pCVar2);
      if (iVar1 != 0) {
        pCVar3 = (C_WeaponPaintGun *)
                 ___dynamic_cast(iVar1,*(undefined4 *)(unaff_EBX + 0x621397),unaff_EBX + 0x68dc33,0)
        ;
        if (pCVar3 != (C_WeaponPaintGun *)0x0) {
          C_WeaponPaintGun::SetPaintPower(pCVar3,BOUNCE_POWER);
        }
      }
    }
  }
  else {
    pcVar4 = (char *)(unaff_EBX + 0x4460e3);
    if (1 < args->m_nArgc) {
      pcVar4 = args->m_ppArgv[1];
    }
    iVar1 = _V_stricmp(pcVar4,(char *)(unaff_EBX + 0x468b57));
    if (iVar1 == 0) {
                    /* Unresolved local var: C_BasePlayer * pPlayer@[???]
                       Unresolved local var: C_WeaponPaintGun * pPaintGun@[???] */
      pCVar2 = C_BasePlayer::GetLocalPlayer(-1);
      if (pCVar2 != (C_BasePlayer *)0x0) {
        iVar1 = (**(code **)(*(int *)&(pCVar2->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                      super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                      super_C_BaseEntity.super_IClientEntity.super_IClientUnknown +
                            0x490))(pCVar2);
        if (iVar1 != 0) {
          pCVar3 = (C_WeaponPaintGun *)
                   ___dynamic_cast(iVar1,*(undefined4 *)(unaff_EBX + 0x621397),unaff_EBX + 0x68dc33,
                                   0);
          if (pCVar3 != (C_WeaponPaintGun *)0x0) {
            C_WeaponPaintGun::SetPaintPower(pCVar3,SPEED_POWER);
          }
        }
      }
    }
    else {
      pcVar4 = (char *)(unaff_EBX + 0x4460e3);
      if (1 < args->m_nArgc) {
        pcVar4 = args->m_ppArgv[1];
      }
      iVar1 = _V_stricmp(pcVar4,(char *)(unaff_EBX + 0x452497));
      if (iVar1 == 0) {
                    /* Unresolved local var: C_BasePlayer * pPlayer@[???]
                       Unresolved local var: C_WeaponPaintGun * pPaintGun@[???] */
        pCVar2 = C_BasePlayer::GetLocalPlayer(-1);
        if (pCVar2 != (C_BasePlayer *)0x0) {
          iVar1 = (**(code **)(*(int *)&(pCVar2->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                        super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                        super_C_BaseEntity.super_IClientEntity.super_IClientUnknown
                              + 0x490))(pCVar2);
          if (iVar1 != 0) {
            pCVar3 = (C_WeaponPaintGun *)
                     ___dynamic_cast(iVar1,*(undefined4 *)(unaff_EBX + 0x621397),
                                     unaff_EBX + 0x68dc33,0);
            if (pCVar3 != (C_WeaponPaintGun *)0x0) {
              C_WeaponPaintGun::SetPaintPower(pCVar3,STICK_POWER);
            }
          }
        }
      }
      else {
        pcVar4 = (char *)(unaff_EBX + 0x4460e3);
        if (1 < args->m_nArgc) {
          pcVar4 = args->m_ppArgv[1];
        }
        iVar1 = _V_stricmp(pcVar4,(char *)(unaff_EBX + 0x478f33));
        if (iVar1 == 0) {
                    /* Unresolved local var: C_BasePlayer * pPlayer@[???]
                       Unresolved local var: C_WeaponPaintGun * pPaintGun@[???] */
          pCVar2 = C_BasePlayer::GetLocalPlayer(-1);
          if (pCVar2 != (C_BasePlayer *)0x0) {
            iVar1 = (**(code **)(*(int *)&(pCVar2->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                          super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                          super_C_BaseEntity.super_IClientEntity.
                                          super_IClientUnknown + 0x490))(pCVar2);
            if (iVar1 != 0) {
              pCVar3 = (C_WeaponPaintGun *)
                       ___dynamic_cast(iVar1,*(undefined4 *)(unaff_EBX + 0x621397),
                                       unaff_EBX + 0x68dc33,0);
              if (pCVar3 != (C_WeaponPaintGun *)0x0) {
                C_WeaponPaintGun::SetPaintPower(pCVar3,PORTAL_POWER);
              }
            }
          }
        }
      }
    }
  }
  return;
}


/* C_WeaponPaintGun::YouForgotToImplementOrDeclareClientClass at 00540c30 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_WeaponPaintGun * this)
    */

int __thiscall C_WeaponPaintGun::YouForgotToImplementOrDeclareClientClass(C_WeaponPaintGun *this)

{
  return 0;
}


/* C_WeaponPaintGun::GetClientClass at 00540c50 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: ClientClass * GetClientClass(C_WeaponPaintGun * this) */

ClientClass * __thiscall C_WeaponPaintGun::GetClientClass(C_WeaponPaintGun *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x828b40);
}


/* _C_WeaponPaintGun_CreateObject at 00541160 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_WeaponPaintGun_CreateObject(int entnum,int serialNum)

{
  C_WeaponPaintGun *this;
  IClientNetworkable *pIVar1;
  
                    /* Unresolved local var: C_WeaponPaintGun * pRet@[???] */
  this = C_BaseEntity::operator_new(0xeac);
  C_WeaponPaintGun::C_WeaponPaintGun(this);
  pIVar1 = (IClientNetworkable *)0x0;
  if (this != (C_WeaponPaintGun *)0x0) {
    (**(code **)(*(int *)&(this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.
                          super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity.
                          super_IClientEntity.super_IClientUnknown + 0xbc))(this,entnum,serialNum);
    pIVar1 = &(this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.
              super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.
              super_IClientNetworkable;
  }
  return pIVar1;
}


/* ClientClassInit<DT_WeaponPaintGun::ignored> at 0007a2a0 */

int ClientClassInit<DT_WeaponPaintGun::ignored>(ignored *param_1)

{
  RecvProp_conflict1 pArrayProp;
  RecvProp_conflict1 pArrayProp_00;
  int iVar1;
  int unaff_EBX;
  RecvProp *pRVar2;
  undefined4 *puVar3;
  DataTableRecvVarProxyFn_conflict1 varProxy;
  undefined1 in_stack_ffffff0c [56];
  DataTableRecvVarProxyFn_conflict1 p_Var4;
  RecvProp local_94;
  RecvProp local_58;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&DAT_00c5965f)[unaff_EBX] == '\0') {
    iVar1 = ___cxa_guard_acquire(&DAT_00c5965f + unaff_EBX);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)(&DAT_00c5966f + unaff_EBX),(char *)(unaff_EBX + 0x9104bf),0
                  ,4,0,(RecvVarProxyFn)0x0);
      varProxy = *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00ae9b3b + unaff_EBX);
      RecvPropDataTable((RecvProp_conflict1 *)(&DAT_00c596ab + unaff_EBX),
                        (char *)(unaff_EBX + 0x915003),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00aea2cf + unaff_EBX),varProxy);
      RecvPropBool(&local_58,(char *)(unaff_EBX + 0x94160d),0xe7e,1);
      p_Var4 = *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00ae9b3b + unaff_EBX);
      pRVar2 = &local_58;
      puVar3 = (undefined4 *)&stack0xffffff08;
      for (iVar1 = 0xf; iVar1 != 0; iVar1 = iVar1 + -1) {
        *puVar3 = pRVar2->m_pVarName;
        pRVar2 = (RecvProp *)&pRVar2->m_RecvType;
        puVar3 = puVar3 + 1;
      }
      pArrayProp.m_RecvType = in_stack_ffffff0c._0_4_;
      pArrayProp.m_Flags = in_stack_ffffff0c._4_4_;
      pArrayProp.m_StringBufferSize = in_stack_ffffff0c._8_4_;
      pArrayProp.m_bInsideArray = (bool)in_stack_ffffff0c[0xc];
      pArrayProp._17_3_ = in_stack_ffffff0c._13_3_;
      pArrayProp.m_pExtraData = (void *)in_stack_ffffff0c._16_4_;
      pArrayProp.m_pArrayProp = (RecvProp_conflict1 *)in_stack_ffffff0c._20_4_;
      pArrayProp.m_ArrayLengthProxy = (ArrayLengthRecvProxyFn)in_stack_ffffff0c._24_4_;
      pArrayProp.m_ProxyFn = (RecvVarProxyFn)in_stack_ffffff0c._28_4_;
      pArrayProp.m_DataTableProxyFn = (DataTableRecvVarProxyFn_conflict1)in_stack_ffffff0c._32_4_;
      pArrayProp.m_pDataTable = (RecvTable *)in_stack_ffffff0c._36_4_;
      pArrayProp.m_Offset = in_stack_ffffff0c._40_4_;
      pArrayProp.m_ElementStride = in_stack_ffffff0c._44_4_;
      pArrayProp.m_nElements = in_stack_ffffff0c._48_4_;
      pArrayProp.m_pParentArrayPropName = (char *)in_stack_ffffff0c._52_4_;
      pArrayProp.m_pVarName = (char *)varProxy;
      RecvPropArray3((RecvProp_conflict1 *)(&DAT_00c596e7 + unaff_EBX),
                     (char *)(unaff_EBX + 0x94161c),0xe7e,1,5,pArrayProp,p_Var4);
      RecvPropEHandle(&local_94,(char *)(unaff_EBX + 0x941628),0xe84,4,
                      *(RecvVarProxyFn *)(&DAT_00ae9b37 + unaff_EBX));
      p_Var4 = *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00ae9b3b + unaff_EBX);
      pRVar2 = &local_94;
      puVar3 = (undefined4 *)&stack0xffffff08;
      for (iVar1 = 0xf; iVar1 != 0; iVar1 = iVar1 + -1) {
        *puVar3 = pRVar2->m_pVarName;
        pRVar2 = (RecvProp *)&pRVar2->m_RecvType;
        puVar3 = puVar3 + 1;
      }
      pArrayProp_00.m_RecvType = in_stack_ffffff0c._0_4_;
      pArrayProp_00.m_Flags = in_stack_ffffff0c._4_4_;
      pArrayProp_00.m_StringBufferSize = in_stack_ffffff0c._8_4_;
      pArrayProp_00.m_bInsideArray = (bool)in_stack_ffffff0c[0xc];
      pArrayProp_00._17_3_ = in_stack_ffffff0c._13_3_;
      pArrayProp_00.m_pExtraData = (void *)in_stack_ffffff0c._16_4_;
      pArrayProp_00.m_pArrayProp = (RecvProp_conflict1 *)in_stack_ffffff0c._20_4_;
      pArrayProp_00.m_ArrayLengthProxy = (ArrayLengthRecvProxyFn)in_stack_ffffff0c._24_4_;
      pArrayProp_00.m_ProxyFn = (RecvVarProxyFn)in_stack_ffffff0c._28_4_;
      pArrayProp_00.m_DataTableProxyFn = (DataTableRecvVarProxyFn_conflict1)in_stack_ffffff0c._32_4_
      ;
      pArrayProp_00.m_pDataTable = (RecvTable *)in_stack_ffffff0c._36_4_;
      pArrayProp_00.m_Offset = in_stack_ffffff0c._40_4_;
      pArrayProp_00.m_ElementStride = in_stack_ffffff0c._44_4_;
      pArrayProp_00.m_nElements = in_stack_ffffff0c._48_4_;
      pArrayProp_00.m_pParentArrayPropName = (char *)in_stack_ffffff0c._52_4_;
      pArrayProp_00.m_pVarName = (char *)varProxy;
      RecvPropArray3((RecvProp_conflict1 *)
                     ((int)NPaintRenderer::g_FastSpherePosData[0] + unaff_EBX + 3),
                     (char *)(unaff_EBX + 0x940214),0xe84,4,5,pArrayProp_00,p_Var4);
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)NPaintRenderer::g_FastSpherePosData[1] + unaff_EBX + 0x1f),
                  (char *)(unaff_EBX + 0x94163d),0xe78,4,0,(RecvVarProxyFn)0x0);
      RecvPropBool((RecvProp *)((int)NPaintRenderer::g_FastSpherePosData[3] + unaff_EBX + 0x1b),
                   (char *)(unaff_EBX + 0x94164d),0xe7c,1);
      RecvPropBool((RecvProp *)((int)NPaintRenderer::g_FastSpherePosData[5] + unaff_EBX + 0x17),
                   (char *)(unaff_EBX + 0x94165c),0xe7d,1);
      RecvPropEHandle((RecvProp *)((int)NPaintRenderer::g_FastSpherePosData[7] + unaff_EBX + 0x13),
                      (char *)(unaff_EBX + 0x940214),0xe84,0x14,
                      *(RecvVarProxyFn *)(&DAT_00ae9b37 + unaff_EBX));
      ___cxa_guard_release(&DAT_00c5965f + unaff_EBX);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_SIMDRandContexts[0x19].m_RandY[7] + unaff_EBX + 0xf),
             (RecvProp_conflict1 *)(&DAT_00c596ab + unaff_EBX),7,(char *)(unaff_EBX + 0x94166b));
  return 1;
}


/* __tcf_2 at 0097c5c0 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3ed1e8),in_stack_00000008);
  return;
}


/* C_WeaponPaintGun::GetPredDescMap at 00540c60 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: datamap_t * GetPredDescMap(C_WeaponPaintGun * this) */

datamap_t * __thiscall C_WeaponPaintGun::GetPredDescMap(C_WeaponPaintGun *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x6ec03c);
}


/* PredMapInit<C_WeaponPaintGun> at 00540c70 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

datamap_t * PredMapInit<C_WeaponPaintGun>(C_WeaponPaintGun *param_1)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *(undefined4 *)(extraout_ECX + 0x6ec02d) = 4;
  *(int *)(extraout_ECX + 0x6ec029) = extraout_ECX + 0x6ec085;
  return (datamap_t *)(extraout_ECX + 0x6ec029);
}


/* CC_WeaponPaintGunFactory at 00541120 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

C_BaseEntity * CC_WeaponPaintGunFactory(void)

{
  C_WeaponPaintGun *this;
  
  this = C_BaseEntity::operator_new(0xeac);
  C_WeaponPaintGun::C_WeaponPaintGun(this);
  return (C_BaseEntity *)this;
}


/* C_WeaponPaintGun::C_WeaponPaintGun at 00540d00 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void C_WeaponPaintGun(C_WeaponPaintGun * this) */

void __thiscall C_WeaponPaintGun::C_WeaponPaintGun(C_WeaponPaintGun *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BasePortalCombatWeapon::C_BasePortalCombatWeapon(&this->super_C_BasePortalCombatWeapon);
  (this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.super_C_BaseCombatWeapon.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientUnknown =
       (IClientUnknown)(unaff_EBX + 0x68f2fb);
  (this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.super_C_BaseCombatWeapon.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x68f993);
  (this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.super_C_BaseCombatWeapon.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x68fa4b);
  (this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.super_C_BaseCombatWeapon.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x68fa87);
  (this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.super_C_BaseCombatWeapon.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientModelRenderable._vptr_IClientModelRenderable
       = (_func_int_varargs **)(unaff_EBX + 0x68faa3);
  (this->m_pStreamEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
  (this->m_pStreamEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
  (this->m_pStreamEffect).m_pObject = (CNewParticleEffect *)0x0;
  this->m_flBobScale = 0.0;
  this->m_flAccumulatedTime = 0.0;
  this->m_flLastThinkTime = 0.0;
  this->m_nCurrentColor = 4;
  this->m_bFiringPaint = false;
  this->m_bFiringErase = false;
  (this->m_hPaintStream).m_Value[0].super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hPaintStream).m_Value[1].super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hPaintStream).m_Value[2].super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hPaintStream).m_Value[3].super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hPaintStream).m_Value[4].super_CBaseHandle.m_Index = 0xffffffff;
  this->m_pLastOwner = (C_BaseCombatCharacter *)0x0;
  (this->m_pLastEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
  (this->m_pLastEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
  (this->m_pLastEffect).m_pObject = (CNewParticleEffect *)0x0;
  return;
}


/* C_WeaponPaintGun::C_WeaponPaintGun at 00540e00 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void C_WeaponPaintGun(C_WeaponPaintGun * this, C_WeaponPaintGun * this)
    */

void __thiscall C_WeaponPaintGun::C_WeaponPaintGun(C_WeaponPaintGun *this,C_WeaponPaintGun *this_1)

{
  C_WeaponPaintGun(this);
  return;
}


/* C_WeaponPaintGun::Precache at 00540ca0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void Precache(C_WeaponPaintGun * this) */

void __thiscall C_WeaponPaintGun::Precache(C_WeaponPaintGun *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  PrecacheParticleSystem((char *)(unaff_EBX + 0x44e4a4));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x44e4ba));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x44e4cf));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x44e4e4));
  C_BaseCombatWeapon::Precache((C_BaseCombatWeapon *)this);
  return;
}


/* C_WeaponPaintGun::Spawn at 005414b0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void Spawn(C_WeaponPaintGun * this) */

void __thiscall C_WeaponPaintGun::Spawn(C_WeaponPaintGun *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseCombatWeapon::Spawn((C_BaseCombatWeapon *)this);
  this->m_flLastThinkTime = *(float *)(**(int **)(unaff_EBX + 0x6228db) + 0xc);
  this->m_flAccumulatedTime =
       *(float *)(unaff_EBX + 0x4a0cc3) /
       *(float *)(*(int *)(*(int *)(unaff_EBX + 0x623133) + 0x1c) + 0x2c);
  this->m_nBlobRandomSeed = 0;
  C_BaseEntity::SetNextThink((C_BaseEntity *)this,-1293.0,(char *)0x0);
  SetPaintPower(this,NO_POWER);
  return;
}


/* C_WeaponPaintGun::ClientThink at 00540f20 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void ClientThink(C_WeaponPaintGun * this) */

void __thiscall C_WeaponPaintGun::ClientThink(C_WeaponPaintGun *this)

{
  float fVar1;
  float fVar2;
  char cVar3;
  bool bVar4;
  C_BaseCombatCharacter *pCVar5;
  PaintPowerType PVar6;
  int paintType;
  int unaff_EBX;
  C_Portal_Player *this_00;
  C_Portal_Player *flDeltaTime;
  C_Portal_Player *in_stack_ffffffa8;
  Vector local_30;
  undefined4 local_24;
  Color local_20 [4];
  
                    /* Unresolved local var: float flDeltaTime@[???]
                       Unresolved local var: C_Portal_Player * pPlayer@[???]
                       Unresolved local var: Color color@[???] */
  ___i686_get_pc_thunk_bx();
  fVar1 = *(float *)(**(int **)(unaff_EBX + 0x622e6e) + 0xc);
  fVar2 = this->m_flLastThinkTime;
  this->m_flLastThinkTime = fVar1;
  pCVar5 = C_BaseCombatWeapon::GetOwner((C_BaseCombatWeapon *)this);
  if (pCVar5 == (C_BaseCombatCharacter *)0x0) {
LAB_00540f7e:
    this_00 = (C_Portal_Player *)0x0;
  }
  else {
    cVar3 = (**(code **)((int)(pCVar5->super_C_BaseFlex).super_C_BaseAnimatingOverlay.
                              super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.
                              super_IClientUnknown.super_IHandleEntity + 0x304))(pCVar5);
    if (cVar3 == '\0') goto LAB_00540f7e;
    in_stack_ffffffa8 = *(C_Portal_Player **)(&DAT_00622e4a + unaff_EBX);
    this_00 = (C_Portal_Player *)
              ___dynamic_cast(pCVar5,in_stack_ffffffa8,*(undefined4 *)(unaff_EBX + 0x623082),0);
  }
  bVar4 = C_BaseCombatWeapon::IsCarrierAlive((C_BaseCombatWeapon *)this);
  if (((bVar4) &&
      ((this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.super_C_BaseCombatWeapon.
       m_iState.m_Value == 2)) &&
     (flDeltaTime = (C_Portal_Player *)(fVar1 - fVar2),
     *(float *)(unaff_EBX + 0x4a1352) <= (float)flDeltaTime &&
     (float)flDeltaTime != *(float *)(unaff_EBX + 0x4a1352))) {
    if (this->m_bFiringPaint == false) {
      paintType = 4;
      if (this->m_bFiringErase == false) goto LAB_00540f99;
    }
    else {
      paintType = this->m_nCurrentColor;
    }
                    /* Unresolved local var: PaintPowerType blobType@[???] */
    SprayPaint(this,(float)flDeltaTime,paintType);
    in_stack_ffffffa8 = flDeltaTime;
  }
LAB_00540f99:
  if (this_00 != (C_Portal_Player *)0x0) {
    C_Portal_Player::GetPaintGunShootPosition(&local_30,this_00);
    (this->m_vecOldBlobFirePos).x = local_30.x;
    (this->m_vecOldBlobFirePos).y = local_30.y;
    (this->m_vecOldBlobFirePos).z = local_30.z;
    in_stack_ffffffa8 = this_00;
  }
  pCVar5 = C_BaseCombatWeapon::GetOwner((C_BaseCombatWeapon *)this);
  bVar4 = C_BasePlayer::IsLocalPlayer((C_BaseEntity *)pCVar5);
  if (bVar4) {
    local_20[0]._color =
         (uchar  [4])
         MapPowerToColor((Color *)(this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.
                                  super_C_BaseCombatWeapon.m_iSubType,(int)in_stack_ffffffa8);
  }
  else {
    local_20[0]._color =
         (uchar  [4])MapPowerToColor((Color *)this->m_nCurrentColor,(int)in_stack_ffffffa8);
  }
  PVar6 = MapColorToPower(local_20);
  if (PVar6 != NO_POWER) {
    bVar4 = HasAnyPaintPower(this);
    if (bVar4) goto LAB_00541010;
  }
  local_24 = 0xffffff;
  local_20[0]._color[0] = 0xff;
  local_20[0]._color[1] = 0xff;
  local_20[0]._color[2] = 0xff;
  local_20[0]._color[3] = '\0';
LAB_00541010:
  if (local_20[0]._color[0] !=
      (this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.super_C_BaseCombatWeapon.
      super_C_BaseAnimating.super_C_BaseEntity.m_clrRender.
      super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.r) {
    (this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.super_C_BaseCombatWeapon.
    super_C_BaseAnimating.super_C_BaseEntity.m_clrRender.
    super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.r =
         local_20[0]._color[0];
  }
  if (local_20[0]._color[1] !=
      *(uint8 *)((int)&(this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.
                       super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity.m_clrRender
                       .super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender> + 1))
  {
    *(uchar *)((int)&(this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.
                     super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity.m_clrRender.
                     super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender> + 1) =
         local_20[0]._color[1];
  }
  if (local_20[0]._color[2] !=
      *(uint8 *)((int)&(this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.
                       super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity.m_clrRender
                       .super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender> + 2))
  {
    *(uchar *)((int)&(this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.
                     super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity.m_clrRender.
                     super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender> + 2) =
         local_20[0]._color[2];
  }
  (**(code **)(*(int *)&(this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.
                        super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity.
                        super_IClientEntity.super_IClientUnknown + 0x274))(this,0xc4a1a000);
  return;
}


/* C_WeaponPaintGun::Simulate at 00541ce0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: bool Simulate(C_WeaponPaintGun * this) */

bool __thiscall C_WeaponPaintGun::Simulate(C_WeaponPaintGun *this)

{
                    /* Unresolved local var: bool result@[???] */
  if (((this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.super_C_BaseCombatWeapon.
       super_C_BaseAnimating.super_C_BaseEntity.m_fEffects & 0x20) == 0) {
    return true;
  }
  StopHoseEffect(this);
  return false;
}


/* C_WeaponPaintGun::OnPreDataChanged at 00540e20 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void OnPreDataChanged(C_WeaponPaintGun * this, DataUpdateType_t type)
    */

void __thiscall C_WeaponPaintGun::OnPreDataChanged(C_WeaponPaintGun *this,DataUpdateType_t type)

{
  CNewParticleEffect *pCVar1;
  CNewParticleEffect *pCVar2;
  CUtlReference<CNewParticleEffect> *pCVar3;
  CUtlReference<CNewParticleEffect> *pCVar4;
  C_BaseCombatCharacter *pCVar5;
  
  C_BaseAnimating::OnPreDataChanged((C_BaseAnimating *)this,type);
  pCVar5 = C_BaseCombatWeapon::GetOwner((C_BaseCombatWeapon *)this);
  this->m_pLastOwner = pCVar5;
  pCVar1 = (this->m_pStreamEffect).m_pObject;
  pCVar2 = (this->m_pLastEffect).m_pObject;
  if (pCVar1 != pCVar2) {
    if (pCVar2 != (CNewParticleEffect *)0x0) {
      pCVar3 = (this->m_pLastEffect).m_pPrev;
      if (pCVar3 == (CUtlReference<CNewParticleEffect> *)0x0) {
        if ((pCVar2->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
            &this->m_pLastEffect) {
          pCVar3 = (this->m_pLastEffect).m_pNext;
          (pCVar2->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar3;
          if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
            pCVar3->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
          }
        }
      }
      else {
        pCVar3->m_pNext = (this->m_pLastEffect).m_pNext;
        pCVar4 = (this->m_pLastEffect).m_pNext;
        if (pCVar4 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar4->m_pPrev = pCVar3;
        }
      }
      (this->m_pLastEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
      (this->m_pLastEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
      (this->m_pLastEffect).m_pObject = (CNewParticleEffect *)0x0;
    }
    (this->m_pLastEffect).m_pObject = pCVar1;
    if (pCVar1 != (CNewParticleEffect *)0x0) {
      (this->m_pLastEffect).m_pNext =
           (pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
           super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead;
      pCVar3 = (pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
               super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead;
      if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
        pCVar3->m_pPrev = &this->m_pLastEffect;
      }
      (this->m_pLastEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
      (pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
      super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = &this->m_pLastEffect;
    }
  }
  return;
}


/* C_WeaponPaintGun::OnDataChanged at 00542900 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void OnDataChanged(C_WeaponPaintGun * this, DataUpdateType_t
   updateType) */

void __thiscall C_WeaponPaintGun::OnDataChanged(C_WeaponPaintGun *this,DataUpdateType_t updateType)

{
  CNewParticleEffect *pCVar1;
  CNewParticleEffect *pCVar2;
  CUtlReference<CNewParticleEffect> *pCVar3;
  CUtlReference<CNewParticleEffect> *pCVar4;
  bool bVar5;
  char cVar6;
  C_BaseCombatCharacter *pCVar7;
  C_BaseCombatCharacter *pCVar8;
  C_Portal_Player *this_00;
  int unaff_EBX;
  Vector local_28 [2];
  
  ___i686_get_pc_thunk_bx();
  C_WeaponPortalBase::OnDataChanged((C_WeaponPortalBase *)this,updateType);
  (**(code **)(*(int *)&(this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.
                        super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity.
                        super_IClientEntity.super_IClientUnknown + 0x274))(this,0xc4a1a000);
  if (updateType == DATA_UPDATE_CREATED) {
                    /* Unresolved local var: C_Portal_Player * pOwner@[???] */
    (this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.super_C_BaseCombatWeapon.
    m_iSubType = this->m_nCurrentColor;
    pCVar8 = C_BaseCombatWeapon::GetOwner((C_BaseCombatWeapon *)this);
    if (((pCVar8 != (C_BaseCombatCharacter *)0x0) &&
        (cVar6 = (**(code **)((int)(pCVar8->super_C_BaseFlex).super_C_BaseAnimatingOverlay.
                                   super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.
                                   super_IClientUnknown.super_IHandleEntity + 0x304))(pCVar8),
        cVar6 != '\0')) &&
       (this_00 = (C_Portal_Player *)
                  ___dynamic_cast(pCVar8,*(undefined4 *)(unaff_EBX + 0x62146a),
                                  *(undefined4 *)(unaff_EBX + 0x6216a2),0),
       this_00 != (C_Portal_Player *)0x0)) {
      C_Portal_Player::GetPaintGunShootPosition(local_28,this_00);
      (this->m_vecOldBlobFirePos).x = local_28[0].x;
      (this->m_vecOldBlobFirePos).y = local_28[0].y;
      (this->m_vecOldBlobFirePos).z = local_28[0].z;
    }
  }
  bVar5 = C_BaseCombatWeapon::IsCarrierAlive((C_BaseCombatWeapon *)this);
  if ((bVar5) &&
     ((this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.super_C_BaseCombatWeapon.
      m_iState.m_Value == 2)) {
    if (((this->m_bFiringPaint != false) && (bVar5 = HasAnyPaintPower(this), bVar5)) ||
       (this->m_bFiringErase != false)) {
      StartHoseEffect(this);
    }
    else {
      StopHoseEffect(this);
    }
    ChangeRenderColor(this,false);
  }
  else {
    StopHoseEffect(this);
  }
  pCVar8 = this->m_pLastOwner;
  pCVar7 = C_BaseCombatWeapon::GetOwner((C_BaseCombatWeapon *)this);
  if (pCVar8 != pCVar7) {
    StopHoseEffect(this);
    pCVar8 = C_BaseCombatWeapon::GetOwner((C_BaseCombatWeapon *)this);
    C_BaseCombatWeapon::SetOwner((C_BaseCombatWeapon *)this,this->m_pLastOwner);
    pCVar1 = (this->m_pLastEffect).m_pObject;
    pCVar2 = (this->m_pStreamEffect).m_pObject;
    if (pCVar1 != pCVar2) {
      if (pCVar2 != (CNewParticleEffect *)0x0) {
        pCVar3 = (this->m_pStreamEffect).m_pPrev;
        if (pCVar3 == (CUtlReference<CNewParticleEffect> *)0x0) {
          if ((pCVar2->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
              super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
              &this->m_pStreamEffect) {
            pCVar3 = (this->m_pStreamEffect).m_pNext;
            (pCVar2->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar3;
            if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
              pCVar3->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
            }
          }
        }
        else {
          pCVar3->m_pNext = (this->m_pStreamEffect).m_pNext;
          pCVar4 = (this->m_pStreamEffect).m_pNext;
          if (pCVar4 != (CUtlReference<CNewParticleEffect> *)0x0) {
            pCVar4->m_pPrev = pCVar3;
          }
        }
        (this->m_pStreamEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
        (this->m_pStreamEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
        (this->m_pStreamEffect).m_pObject = (CNewParticleEffect *)0x0;
      }
      (this->m_pStreamEffect).m_pObject = pCVar1;
      if (pCVar1 != (CNewParticleEffect *)0x0) {
        (this->m_pStreamEffect).m_pNext =
             (pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
             super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead;
        pCVar3 = (pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>
                 .super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead;
        if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar3->m_pPrev = &this->m_pStreamEffect;
        }
        (this->m_pStreamEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
        (pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
        super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = &this->m_pStreamEffect
        ;
      }
    }
    StopHoseEffect(this);
    C_BaseCombatWeapon::SetOwner((C_BaseCombatWeapon *)this,pCVar8);
  }
  return;
}


/* C_WeaponPaintGun::ChangeRenderColor at 00541940 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ChangeRenderColor(C_WeaponPaintGun * this, bool bForce) */

void __thiscall C_WeaponPaintGun::ChangeRenderColor(C_WeaponPaintGun *this,bool bForce)

{
  char cVar1;
  bool bVar2;
  C_BaseCombatCharacter *pEntity;
  C_BaseViewModel *pCVar3;
  int in_stack_ffffffb8;
  undefined4 local_20;
  
                    /* Unresolved local var: C_BasePlayer * pPlayer@[???]
                       Unresolved local var: Color color@[???] */
  pEntity = C_BaseCombatWeapon::GetOwner((C_BaseCombatWeapon *)this);
  if ((pEntity == (C_BaseCombatCharacter *)0x0) ||
     (cVar1 = (**(code **)((int)(pEntity->super_C_BaseFlex).super_C_BaseAnimatingOverlay.
                                super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.
                                super_IClientUnknown.super_IHandleEntity + 0x304))(pEntity),
     cVar1 == '\0')) {
    pEntity = (C_BaseCombatCharacter *)0x0;
  }
  bVar2 = C_BasePlayer::IsLocalPlayer((C_BaseEntity *)pEntity);
  if (bVar2) {
    local_20 = MapPowerToColor((Color *)(this->super_C_BasePortalCombatWeapon).
                                        super_C_WeaponPortalBase.super_C_BaseCombatWeapon.m_iSubType
                               ,in_stack_ffffffb8);
  }
  else {
    local_20 = MapPowerToColor((Color *)this->m_nCurrentColor,in_stack_ffffffb8);
  }
  bVar2 = HasAnyPaintPower(this);
  if ((!bVar2) && (!bForce)) {
    local_20 = (Color *)0xffffffff;
  }
  if (((pEntity != (C_BaseCombatCharacter *)0x0) &&
      (*(char *)(pEntity[1].super_C_BaseFlex.super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                 super_C_BaseEntity.m_pIntermediateData + 0x28) != '\0')) &&
     (pCVar3 = GetEffectViewModel(this), pCVar3 != (C_BaseViewModel *)0x0)) {
    if ((pCVar3->super_C_BaseAnimating).super_C_BaseEntity.m_clrRender.
        super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.r !=
        (uint8)local_20) {
      (pCVar3->super_C_BaseAnimating).super_C_BaseEntity.m_clrRender.
      super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.r =
           (uint8)local_20;
    }
    if (local_20._1_1_ !=
        (pCVar3->super_C_BaseAnimating).super_C_BaseEntity.m_clrRender.
        super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.g) {
      (pCVar3->super_C_BaseAnimating).super_C_BaseEntity.m_clrRender.
      super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.g =
           local_20._1_1_;
    }
    if (local_20._2_1_ !=
        (pCVar3->super_C_BaseAnimating).super_C_BaseEntity.m_clrRender.
        super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.b) {
      (pCVar3->super_C_BaseAnimating).super_C_BaseEntity.m_clrRender.
      super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.b =
           local_20._2_1_;
    }
  }
  if ((uint)(this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.super_C_BaseCombatWeapon
            .super_C_BaseAnimating.super_C_BaseEntity.m_clrRender.
            super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.r !=
      ((uint)local_20 & 0xff)) {
    (this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.super_C_BaseCombatWeapon.
    super_C_BaseAnimating.super_C_BaseEntity.m_clrRender.
    super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.r =
         (uint8)local_20;
  }
  if (local_20._1_1_ !=
      *(uint8 *)((int)&(this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.
                       super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity.m_clrRender
                       .super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender> + 1))
  {
    *(uint8 *)((int)&(this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.
                     super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity.m_clrRender.
                     super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender> + 1) =
         local_20._1_1_;
  }
  if (local_20._2_1_ !=
      *(uint8 *)((int)&(this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.
                       super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity.m_clrRender
                       .super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender> + 2))
  {
    *(uint8 *)((int)&(this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.
                     super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity.m_clrRender.
                     super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender> + 2) =
         local_20._2_1_;
  }
  return;
}


/* C_WeaponPaintGun::StartHoseEffect at 00541d10 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void StartHoseEffect(C_WeaponPaintGun * this) */

void __thiscall C_WeaponPaintGun::StartHoseEffect(C_WeaponPaintGun *this)

{
  CNewParticleEffect *pCVar1;
  CUtlReference<CNewParticleEffect> *pCVar2;
  CUtlReference<CNewParticleEffect> *pCVar3;
  char cVar4;
  bool bVar5;
  C_BasePlayer *this_00;
  int nOldSlot;
  CNewParticleEffect *pCVar6;
  C_BaseViewModel *pCVar7;
  int unaff_EBX;
  char *in_stack_ffffff78;
  char *pchContext;
  CSetActiveSplitScreenPlayerGuard *in_stack_ffffff8c;
  char *in_stack_ffffff90;
  int in_stack_ffffff94;
  C_BaseEntity *in_stack_ffffff98;
  CSetActiveSplitScreenPlayerGuard *this_01;
  uint *puVar8;
  CSetActiveSplitScreenPlayerGuard local_54;
  Vector local_34;
  Color *local_28;
  Color *local_24;
  uint local_20 [4];
  
                    /* Unresolved local var: char * particleEffectName@[???]
                       Unresolved local var: Color paintColor@[???]
                       Unresolved local var: C_BasePlayer * pOwner@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(int *)(*(int *)(unaff_EBX + 0x827a35) + 0x30) == 0) {
    return;
  }
  puVar8 = local_20;
  local_20[0] = 0;
  this_00 = (C_BasePlayer *)C_BaseCombatWeapon::GetOwner((C_BaseCombatWeapon *)this);
  if ((this_00 == (C_BasePlayer *)0x0) ||
     (cVar4 = (**(code **)((int)(this_00->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
                                super_IHandleEntity + 0x304))(this_00), cVar4 == '\0')) {
    this_00 = (C_BasePlayer *)0x0;
    bVar5 = C_BasePlayer::IsLocalPlayer((C_BaseEntity *)0x0);
    if (bVar5) goto LAB_00541e50;
  }
  else {
    bVar5 = C_BasePlayer::IsLocalPlayer((C_BaseEntity *)this_00);
    if (bVar5) {
LAB_00541e50:
      nOldSlot = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x62207d) + 0x1f8))
                           ((int *)**(undefined4 **)(unaff_EBX + 0x62207d));
      pchContext = (char *)(unaff_EBX + 0x479ce9);
      this_01 = &local_54;
      CSetActiveSplitScreenPlayerGuard::CSetActiveSplitScreenPlayerGuard
                (this_01,pchContext,0x102,(C_BaseEntity *)this_00,nOldSlot,false,in_stack_ffffff8c,
                 in_stack_ffffff90,in_stack_ffffff94,in_stack_ffffff98,(int)this_01,SUB41(puVar8,0))
      ;
      if ((this->m_pStreamEffect).m_pObject == (CNewParticleEffect *)0x0) {
        pCVar7 = C_BasePlayer::GetViewModel(this_00,0);
        pchContext = &UNK_00479d5a + unaff_EBX;
        pCVar6 = CParticleProperty::Create
                           (&(pCVar7->super_C_BaseAnimating).super_C_BaseEntity.m_Particles.
                             super_CParticleProperty,pchContext,PATTACH_POINT_FOLLOW,
                            (char *)(unaff_EBX + 0x479d4d));
        pCVar1 = (this->m_pStreamEffect).m_pObject;
        if (pCVar6 != pCVar1) {
          if (pCVar1 != (CNewParticleEffect *)0x0) {
            pCVar2 = (this->m_pStreamEffect).m_pPrev;
            if (pCVar2 == (CUtlReference<CNewParticleEffect> *)0x0) {
              if ((pCVar1->m_References).
                  super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
                  super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
                  &this->m_pStreamEffect) {
                pCVar2 = (this->m_pStreamEffect).m_pNext;
                (pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
                super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar2;
                if (pCVar2 != (CUtlReference<CNewParticleEffect> *)0x0) {
                  pCVar2->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
                }
              }
            }
            else {
              pCVar2->m_pNext = (this->m_pStreamEffect).m_pNext;
              pCVar3 = (this->m_pStreamEffect).m_pNext;
              if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
                pCVar3->m_pPrev = pCVar2;
              }
            }
            (this->m_pStreamEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
            (this->m_pStreamEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
            (this->m_pStreamEffect).m_pObject = (CNewParticleEffect *)0x0;
          }
          (this->m_pStreamEffect).m_pObject = pCVar6;
          if (pCVar6 != (CNewParticleEffect *)0x0) {
            (this->m_pStreamEffect).m_pNext =
                 (pCVar6->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>
                 .super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead;
            pCVar2 = (pCVar6->m_References).
                     super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
                     super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead;
            if (pCVar2 != (CUtlReference<CNewParticleEffect> *)0x0) {
              pCVar2->m_pPrev = &this->m_pStreamEffect;
            }
            (this->m_pStreamEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
            (pCVar6->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead =
                 &this->m_pStreamEffect;
          }
        }
      }
      if (this->m_bFiringPaint == false) {
        local_24 = MapPowerToVisualColor
                             ((Color *)&vgui::CvarToggleCheckButton<CGameUIConVarRef>::GetKBMap::
                                        s_pMap.cputype,(int)pchContext);
      }
      else {
        local_24 = MapPowerToVisualColor
                             ((Color *)(this->super_C_BasePortalCombatWeapon).
                                       super_C_WeaponPortalBase.super_C_BaseCombatWeapon.m_iSubType,
                              (int)pchContext);
      }
      *puVar8 = (uint)local_24;
      CSetActiveSplitScreenPlayerGuard::~CSetActiveSplitScreenPlayerGuard(this_01,(int)pchContext);
      goto LAB_00541dc0;
    }
  }
  if ((this->m_pStreamEffect).m_pObject == (CNewParticleEffect *)0x0) {
    in_stack_ffffff78 = &UNK_00479d5a + unaff_EBX;
    pCVar6 = CParticleProperty::Create
                       (&(this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.
                         super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity.
                         m_Particles.super_CParticleProperty,in_stack_ffffff78,PATTACH_POINT_FOLLOW,
                        (char *)(unaff_EBX + 0x44766d));
    pCVar1 = (this->m_pStreamEffect).m_pObject;
    if (pCVar6 != pCVar1) {
      if (pCVar1 != (CNewParticleEffect *)0x0) {
        pCVar2 = (this->m_pStreamEffect).m_pPrev;
        if (pCVar2 == (CUtlReference<CNewParticleEffect> *)0x0) {
          if ((pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
              super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
              &this->m_pStreamEffect) {
            pCVar2 = (this->m_pStreamEffect).m_pNext;
            (pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar2;
            if (pCVar2 != (CUtlReference<CNewParticleEffect> *)0x0) {
              pCVar2->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
            }
          }
        }
        else {
          pCVar2->m_pNext = (this->m_pStreamEffect).m_pNext;
          pCVar3 = (this->m_pStreamEffect).m_pNext;
          if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
            pCVar3->m_pPrev = pCVar2;
          }
        }
        (this->m_pStreamEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
        (this->m_pStreamEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
        (this->m_pStreamEffect).m_pObject = (CNewParticleEffect *)0x0;
      }
      (this->m_pStreamEffect).m_pObject = pCVar6;
      if (pCVar6 != (CNewParticleEffect *)0x0) {
        (this->m_pStreamEffect).m_pNext =
             (pCVar6->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
             super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead;
        pCVar2 = (pCVar6->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>
                 .super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead;
        if (pCVar2 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar2->m_pPrev = &this->m_pStreamEffect;
        }
        (this->m_pStreamEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
        (pCVar6->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
        super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = &this->m_pStreamEffect
        ;
      }
    }
  }
  if (this->m_bFiringPaint == false) {
    local_28 = MapPowerToVisualColor
                         ((Color *)&vgui::CvarToggleCheckButton<CGameUIConVarRef>::GetKBMap::s_pMap.
                                    cputype,(int)in_stack_ffffff78);
  }
  else {
    local_28 = MapPowerToVisualColor((Color *)this->m_nCurrentColor,(int)in_stack_ffffff78);
  }
  *puVar8 = (uint)local_28;
LAB_00541dc0:
  local_34.z = (vec_t)(local_20[0] >> 0x10 & 0xff);
  local_34.y = (vec_t)(local_20[0] >> 8 & 0xff);
  local_34.x = (vec_t)(local_20[0] & 0xff);
  CNewParticleEffect::SetControlPoint((this->m_pStreamEffect).m_pObject,1,&local_34);
  return;
}


/* C_WeaponPaintGun::StopHoseEffect at 00541a90 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void StopHoseEffect(C_WeaponPaintGun * this) */

void __thiscall C_WeaponPaintGun::StopHoseEffect(C_WeaponPaintGun *this)

{
  CNewParticleEffect *pCVar1;
  CUtlReference<CNewParticleEffect> *pCVar2;
  CUtlReference<CNewParticleEffect> *pCVar3;
  char cVar4;
  bool bVar5;
  C_BasePlayer *this_00;
  int nOldSlot;
  C_BaseViewModel *pCVar6;
  int unaff_EBX;
  CNewParticleEffect *pCVar7;
  CSetActiveSplitScreenPlayerGuard *in_stack_ffffffac;
  char *in_stack_ffffffb0;
  int in_stack_ffffffb4;
  C_BaseEntity *in_stack_ffffffb8;
  int in_stack_ffffffbc;
  CSetActiveSplitScreenPlayerGuard *this_01;
  CSetActiveSplitScreenPlayerGuard local_3c;
  
  ___i686_get_pc_thunk_bx();
  if ((this->m_pStreamEffect).m_pObject != (CNewParticleEffect *)0x0) {
                    /* Unresolved local var: C_BasePlayer * pOwner@[???] */
    this_00 = (C_BasePlayer *)C_BaseCombatWeapon::GetOwner((C_BaseCombatWeapon *)this);
    if ((this_00 == (C_BasePlayer *)0x0) ||
       (cVar4 = (**(code **)((int)(this_00->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                  super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                  super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
                                  super_IHandleEntity + 0x304))(this_00), cVar4 == '\0')) {
      this_00 = (C_BasePlayer *)0x0;
    }
    bVar5 = C_BasePlayer::IsLocalPlayer((C_BaseEntity *)this_00);
    if (bVar5) {
      nOldSlot = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x622300) + 0x1f8))
                           ((int *)**(undefined4 **)(unaff_EBX + 0x622300));
      this_01 = &local_3c;
      CSetActiveSplitScreenPlayerGuard::CSetActiveSplitScreenPlayerGuard
                (this_01,&UNK_00479f6c + unaff_EBX,0x122,(C_BaseEntity *)this_00,nOldSlot,false,
                 in_stack_ffffffac,in_stack_ffffffb0,in_stack_ffffffb4,in_stack_ffffffb8,
                 in_stack_ffffffbc,SUB41(this_01,0));
      pCVar7 = (CNewParticleEffect *)0x0;
      pCVar6 = C_BasePlayer::GetViewModel(this_00,0);
      if (pCVar6 != (C_BaseViewModel *)0x0) {
        pCVar7 = (this->m_pStreamEffect).m_pObject;
        pCVar6 = C_BasePlayer::GetViewModel(this_00,0);
        CParticleProperty::StopEmission
                  (&(pCVar6->super_C_BaseAnimating).super_C_BaseEntity.m_Particles.
                    super_CParticleProperty,pCVar7,false,false,false,false);
        pCVar1 = (this->m_pStreamEffect).m_pObject;
        if (pCVar1 != (CNewParticleEffect *)0x0) {
          pCVar2 = (this->m_pStreamEffect).m_pPrev;
          if (pCVar2 == (CUtlReference<CNewParticleEffect> *)0x0) {
            if ((pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
                super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
                &this->m_pStreamEffect) {
              pCVar2 = (this->m_pStreamEffect).m_pNext;
              (pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
              super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar2;
              if (pCVar2 != (CUtlReference<CNewParticleEffect> *)0x0) {
                pCVar2->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
              }
            }
          }
          else {
            pCVar2->m_pNext = (this->m_pStreamEffect).m_pNext;
            pCVar3 = (this->m_pStreamEffect).m_pNext;
            if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
              pCVar3->m_pPrev = pCVar2;
            }
          }
          (this->m_pStreamEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
          (this->m_pStreamEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
          (this->m_pStreamEffect).m_pObject = (CNewParticleEffect *)0x0;
        }
      }
      CSetActiveSplitScreenPlayerGuard::~CSetActiveSplitScreenPlayerGuard(this_01,(int)pCVar7);
    }
    else {
      CParticleProperty::StopEmission
                (&(this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.
                  super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity.m_Particles.
                  super_CParticleProperty,(this->m_pStreamEffect).m_pObject,false,false,false,false)
      ;
      pCVar7 = (this->m_pStreamEffect).m_pObject;
      if (pCVar7 != (CNewParticleEffect *)0x0) {
        pCVar2 = (this->m_pStreamEffect).m_pPrev;
        if (pCVar2 == (CUtlReference<CNewParticleEffect> *)0x0) {
          if ((pCVar7->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
              super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
              &this->m_pStreamEffect) {
            pCVar2 = (this->m_pStreamEffect).m_pNext;
            (pCVar7->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar2;
            if (pCVar2 != (CUtlReference<CNewParticleEffect> *)0x0) {
              pCVar2->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
            }
          }
        }
        else {
          pCVar2->m_pNext = (this->m_pStreamEffect).m_pNext;
          pCVar3 = (this->m_pStreamEffect).m_pNext;
          if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
            pCVar3->m_pPrev = pCVar2;
          }
        }
        (this->m_pStreamEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
        (this->m_pStreamEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
        (this->m_pStreamEffect).m_pObject = (CNewParticleEffect *)0x0;
      }
    }
  }
  return;
}


/* C_WeaponPaintGun::GetEffectViewModel at 00541850 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: C_BaseViewModel * GetEffectViewModel(C_WeaponPaintGun * this) */

C_BaseViewModel * __thiscall C_WeaponPaintGun::GetEffectViewModel(C_WeaponPaintGun *this)

{
  char cVar1;
  bool bVar2;
  C_BasePlayer *this_00;
  int iVar3;
  C_BaseViewModel *pCVar4;
  int unaff_EBX;
  CSetActiveSplitScreenPlayerGuard *in_stack_ffffffbc;
  char *in_stack_ffffffc0;
  int in_stack_ffffffc4;
  C_BaseEntity *in_stack_ffffffc8;
  int in_stack_ffffffcc;
  undefined1 in_stack_ffffffd0;
  
                    /* Unresolved local var: C_BaseViewModel * pViewModel@[???]
                       Unresolved local var: C_BasePlayer * pOwner@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = (C_BasePlayer *)C_BaseCombatWeapon::GetOwner((C_BaseCombatWeapon *)this);
  if ((this_00 == (C_BasePlayer *)0x0) ||
     (cVar1 = (**(code **)(*(int *)&(this_00->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                    super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                    super_C_BaseEntity.super_IClientEntity.super_IClientUnknown +
                          0x304))(this_00), cVar1 == '\0')) {
    this_00 = (C_BasePlayer *)0x0;
    bVar2 = C_BasePlayer::IsLocalPlayer((C_BaseEntity *)0x0);
  }
  else {
    bVar2 = C_BasePlayer::IsLocalPlayer((C_BaseEntity *)this_00);
  }
  pCVar4 = (C_BaseViewModel *)0x0;
  if (bVar2) {
    iVar3 = (**(code **)(*(int *)**(undefined4 **)(&DAT_00622540 + unaff_EBX) + 0x1f8))
                      ((int *)**(undefined4 **)(&DAT_00622540 + unaff_EBX));
    CSetActiveSplitScreenPlayerGuard::CSetActiveSplitScreenPlayerGuard
              ((CSetActiveSplitScreenPlayerGuard *)&stack0xffffffc4,&UNK_0047a1ac + unaff_EBX,0x13a,
               (C_BaseEntity *)this_00,iVar3,false,in_stack_ffffffbc,in_stack_ffffffc0,
               in_stack_ffffffc4,in_stack_ffffffc8,in_stack_ffffffcc,(bool)in_stack_ffffffd0);
    iVar3 = 0;
    pCVar4 = C_BasePlayer::GetViewModel(this_00,0);
    CSetActiveSplitScreenPlayerGuard::~CSetActiveSplitScreenPlayerGuard
              ((CSetActiveSplitScreenPlayerGuard *)&stack0xffffffc4,iVar3);
  }
  return pCVar4;
}


/* C_WeaponPaintGun::AddViewmodelBob at 005420f0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void AddViewmodelBob(C_WeaponPaintGun * this, C_BaseViewModel *
   viewmodel, Vector * origin, QAngle * angles) */

void __thiscall
C_WeaponPaintGun::AddViewmodelBob
          (C_WeaponPaintGun *this,C_BaseViewModel *viewmodel,Vector *origin,QAngle *angles)

{
  float *pfVar1;
  char cVar2;
  C_BaseCombatCharacter *pCVar3;
  C_Portal_Player *this_00;
  C_PortalPlayerLocalData *pCVar4;
  int unaff_EBX;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float rollAngle;
  VMatrix local_104;
  float local_c4;
  float local_c0;
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
  float local_8c;
  float local_88;
  float local_84;
  float local_80;
  float local_7c;
  float local_78;
  float local_74;
  float local_70;
  float local_6c;
  float local_68;
  Vector local_64;
  Vector local_58;
  float local_4c;
  float local_48;
  float local_44;
  Vector local_40;
  Vector local_34;
  Vector local_28 [2];
  
                    /* Unresolved local var: Vector forward@[???]
                       Unresolved local var: Vector right@[???]
                       Unresolved local var: Vector up@[???]
                       Unresolved local var: C_Portal_Player * pPortalPlayer@[???]
                       Unresolved local var: VMatrix rotMatrix@[???]
                       Unresolved local var: Vector rotAxis@[???]
                       Unresolved local var: float pitchAngle@[???]
                       Unresolved local var: float yawAngle@[???] */
  ___i686_get_pc_thunk_bx();
  AngleVectors(angles,local_28,&local_34,&local_40);
  (**(code **)(*(int *)&(this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.
                        super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity.
                        super_IClientEntity.super_IClientUnknown + 0x594))(this);
  pCVar3 = C_BaseCombatWeapon::GetOwner((C_BaseCombatWeapon *)this);
  if (pCVar3 != (C_BaseCombatCharacter *)0x0) {
    cVar2 = (**(code **)((int)(pCVar3->super_C_BaseFlex).super_C_BaseAnimatingOverlay.
                              super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.
                              super_IClientUnknown.super_IHandleEntity + 0x304))(pCVar3);
    if (cVar2 != '\0') {
      this_00 = (C_Portal_Player *)
                ___dynamic_cast(pCVar3,*(undefined4 *)(unaff_EBX + 0x621c77),
                                *(undefined4 *)(unaff_EBX + 0x621eaf),0);
      if (this_00 != (C_Portal_Player *)0x0) {
        pfVar1 = *(float **)(unaff_EBX + 0x621f6b);
        fVar5 = *(float *)(unaff_EBX + 0x4a0187) * *pfVar1;
        origin->x = fVar5 * local_28[0].x + origin->x;
        origin->y = fVar5 * local_28[0].y + origin->y;
        origin->z = fVar5 * local_28[0].z + origin->z;
        pCVar4 = C_Portal_Player::GetPortalPlayerLocalData(this_00);
        local_b0 = *(float *)(unaff_EBX + 0x4a0187) * *pfVar1;
                    /* Unresolved local var: Vector res@[???] */
        local_b8 = local_b0 * (pCVar4->m_Up).x;
        local_b4 = local_b0 * (pCVar4->m_Up).y;
        local_b0 = local_b0 * (pCVar4->m_Up).z;
        fVar6 = origin->x + local_b8;
        origin->x = fVar6;
        fVar7 = origin->y + local_b4;
        origin->y = fVar7;
        fVar8 = origin->z + local_b0;
        origin->z = fVar8;
        fVar5 = *(float *)(&DAT_004a07b7 + unaff_EBX) * **(float **)(unaff_EBX + 0x621f5f);
        origin->x = fVar6 + fVar5 * local_34.x;
        origin->y = fVar7 + fVar5 * local_34.y;
        origin->z = fVar8 + fVar5 * local_34.z;
        fVar5 = *(float *)(unaff_EBX + 0x4a010f);
        fVar6 = *pfVar1;
        local_c4 = local_34.y * local_40.z - local_40.y * local_34.z;
        local_c0 = local_40.x * local_34.z - local_40.z * local_34.x;
        local_bc = local_34.x * local_40.y - local_34.y * local_40.x;
                    /* Unresolved local var: Vector norm@[???] */
        local_64.x = local_c4;
        local_64.y = local_c0;
        local_64.z = local_bc;
        local_4c = local_b8;
        local_48 = local_b4;
        local_44 = local_b0;
        VectorNormalize(&local_64);
        local_58.x = local_64.x;
        local_58.y = local_64.y;
        local_58.z = local_64.z;
        MatrixBuildRotationAboutAxis(&local_104,&local_58,fVar5 * fVar6);
                    /* Unresolved local var: Vector vRet@[???] */
        local_70 = local_104.m[0][0] * local_40.x + local_104.m[0][1] * local_40.y +
                   local_104.m[0][2] * local_40.z + local_104.m[0][3];
        local_6c = local_104.m[1][0] * local_40.x + local_104.m[1][1] * local_40.y +
                   local_104.m[1][2] * local_40.z + local_104.m[1][3];
        local_68 = local_104.m[2][0] * local_40.x + local_104.m[2][1] * local_40.y +
                   local_104.m[2][2] * local_40.z + local_104.m[2][3];
                    /* Unresolved local var: Vector vRet@[???] */
        local_7c = local_104.m[0][0] * local_28[0].x + local_104.m[0][1] * local_28[0].y +
                   local_104.m[0][2] * local_28[0].z + local_104.m[0][3];
        local_78 = local_104.m[1][0] * local_28[0].x + local_104.m[1][1] * local_28[0].y +
                   local_104.m[1][2] * local_28[0].z + local_104.m[1][3];
        local_74 = local_104.m[2][0] * local_28[0].x + local_104.m[2][1] * local_28[0].y +
                   local_104.m[2][2] * local_28[0].z + local_104.m[2][3];
                    /* Unresolved local var: Vector vRet@[???] */
        local_88 = local_104.m[0][0] * local_34.x + local_104.m[0][1] * local_34.y +
                   local_104.m[0][2] * local_34.z + local_104.m[0][3];
        local_84 = local_104.m[1][0] * local_34.x + local_104.m[1][1] * local_34.y +
                   local_104.m[1][2] * local_34.z + local_104.m[1][3];
        local_80 = local_104.m[2][0] * local_34.x + local_104.m[2][1] * local_34.y +
                   local_104.m[2][2] * local_34.z + local_104.m[2][3];
        local_58.x = local_88;
        local_58.y = local_84;
        local_58.z = local_80;
        local_40.x = local_70;
        local_40.y = local_6c;
        local_40.z = local_68;
        local_34.x = local_88;
        local_34.y = local_84;
        local_34.z = local_80;
        local_28[0].x = local_7c;
        local_28[0].y = local_78;
        local_28[0].z = local_74;
        MatrixBuildRotationAboutAxis
                  (&local_104,&local_58,*(float *)(unaff_EBX + 0x4a07af) * *pfVar1);
                    /* Unresolved local var: Vector vRet@[???] */
        local_94 = local_104.m[0][0] * local_40.x + local_104.m[0][1] * local_40.y +
                   local_104.m[0][2] * local_40.z + local_104.m[0][3];
        local_90 = local_104.m[1][0] * local_40.x + local_104.m[1][1] * local_40.y +
                   local_104.m[1][2] * local_40.z + local_104.m[1][3];
        local_8c = local_104.m[2][0] * local_40.x + local_104.m[2][1] * local_40.y +
                   local_104.m[2][2] * local_40.z + local_104.m[2][3];
                    /* Unresolved local var: Vector vRet@[???] */
        local_a0 = local_104.m[0][0] * local_28[0].x + local_104.m[0][1] * local_28[0].y +
                   local_104.m[0][2] * local_28[0].z + local_104.m[0][3];
        local_9c = local_104.m[1][0] * local_28[0].x + local_104.m[1][1] * local_28[0].y +
                   local_104.m[1][2] * local_28[0].z + local_104.m[1][3];
        local_98 = local_104.m[2][0] * local_28[0].x + local_104.m[2][1] * local_28[0].y +
                   local_104.m[2][2] * local_28[0].z + local_104.m[2][3];
        local_58.x = local_94;
        local_58.y = local_90;
        local_58.z = local_8c;
        local_40.x = local_94;
        local_40.y = local_90;
        local_40.z = local_8c;
        local_28[0].x = local_a0;
        local_28[0].y = local_9c;
        local_28[0].z = local_98;
        MatrixBuildRotationAboutAxis
                  (&local_104,&local_58,
                   *(float *)(unaff_EBX + 0x4a07b3) * **(float **)(unaff_EBX + 0x621f5f));
                    /* Unresolved local var: Vector vRet@[???] */
        local_ac = local_104.m[0][0] * local_28[0].x + local_104.m[0][1] * local_28[0].y +
                   local_104.m[0][2] * local_28[0].z + local_104.m[0][3];
        local_a8 = local_104.m[1][0] * local_28[0].x + local_104.m[1][1] * local_28[0].y +
                   local_104.m[1][2] * local_28[0].z + local_104.m[1][3];
        local_a4 = local_104.m[2][0] * local_28[0].x + local_104.m[2][1] * local_28[0].y +
                   local_104.m[2][2] * local_28[0].z + local_104.m[2][3];
        local_28[0].x = local_ac;
        local_28[0].y = local_a8;
        local_28[0].z = local_a4;
        VectorAngles(local_28,&local_40,angles);
        return;
      }
    }
  }
  return;
}


/* C_WeaponPaintGun::CalcViewmodelBob at 00541540 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: float CalcViewmodelBob(C_WeaponPaintGun * this) */

float __thiscall C_WeaponPaintGun::CalcViewmodelBob(C_WeaponPaintGun *this)

{
  int iVar1;
  float *pfVar2;
  char cVar3;
  C_BaseCombatCharacter *pCVar4;
  int unaff_EBX;
  longdouble lVar5;
  float fVar6;
  float fVar7;
  double dVar8;
  double dVar9;
  double dVar10;
  float fVar11;
  float fVar12;
  float fVar13;
  float fVar14;
  double dVar15;
  __m128 root;
  
                    /* Unresolved local var: float cycle@[???]
                       Unresolved local var: C_BasePlayer * player@[???]
                       Unresolved local var: float speed@[???]
                       Unresolved local var: float bob_offset@[???] */
  ___i686_get_pc_thunk_bx();
  pCVar4 = C_BaseCombatWeapon::GetOwner((C_BaseCombatWeapon *)this);
  if (((pCVar4 != (C_BaseCombatCharacter *)0x0) &&
      (cVar3 = (**(code **)((int)(pCVar4->super_C_BaseFlex).super_C_BaseAnimatingOverlay.
                                 super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.
                                 super_IClientUnknown.super_IHandleEntity + 0x304))(pCVar4),
      cVar3 != '\0')) && (*(float *)(**(int **)(unaff_EBX + 0x62284c) + 0x10) != 0.0)) {
    fVar7 = (pCVar4->super_C_BaseFlex).super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
            super_C_BaseEntity.m_vecVelocity.x;
    fVar14 = (pCVar4->super_C_BaseFlex).super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
             super_C_BaseEntity.m_vecVelocity.y;
    fVar6 = (pCVar4->super_C_BaseFlex).super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
            super_C_BaseEntity.m_vecVelocity.z;
    fVar6 = SQRT(fVar7 * fVar7 + fVar14 * fVar14 + fVar6 * fVar6);
    fVar7 = *(float *)(pCVar4[1].super_C_BaseFlex.super_C_BaseAnimatingOverlay.super_C_BaseAnimating
                       .super_C_BaseEntity.m_iSignifierName + 0x98);
    fVar14 = fVar7;
    if ((fVar6 <= fVar7) &&
       (fVar14 = (float)((uint)fVar7 ^ *(uint *)(unaff_EBX + 0x4a5db0)), fVar14 <= fVar6)) {
      fVar14 = fVar6;
    }
    fVar6 = 0.0;
    if ((fVar7 != 0.0) || (NAN(fVar7))) {
      fVar6 = fVar14 / fVar7 + 0.0;
    }
    else if (0.0 <= fVar14 - fVar7) {
      fVar6 = *(float *)(unaff_EBX + 0x4a0c34);
    }
    iVar1 = **(int **)(unaff_EBX + 0x62284c);
    fVar13 = fVar6 * *(float *)(iVar1 + 0x10) + *(float *)(&DAT_007923b8 + unaff_EBX);
    *(float *)(&DAT_007923b8 + unaff_EBX) = fVar13;
    *(undefined4 *)(&DAT_007923b4 + unaff_EBX) = *(undefined4 *)(iVar1 + 0xc);
    fVar6 = fVar13 / *(float *)(unaff_EBX + 0x4a1354);
    fVar7 = ((float)(int)fVar6 * *(float *)(unaff_EBX + 0x4a1358) + fVar13) /
            *(float *)(unaff_EBX + 0x4a1354);
    if ((*(float *)(unaff_EBX + 0x4a0cc0) <= fVar7) ||
       (NAN(fVar7) || NAN(*(float *)(unaff_EBX + 0x4a0cc0)))) {
      dVar10 = *(double *)(unaff_EBX + 0x4a1020);
      dVar8 = (double)(fVar7 - *(float *)(unaff_EBX + 0x4a0cc0)) * dVar10;
      dVar8 = dVar8 + dVar8 + dVar10;
    }
    else {
      dVar10 = *(double *)(unaff_EBX + 0x4a1020);
      dVar8 = (double)fVar7 * dVar10 + (double)fVar7 * dVar10;
    }
    fVar14 = fVar14 * *(float *)(unaff_EBX + 0x4a1350);
    pfVar2 = *(float **)(unaff_EBX + 0x622b1c);
    *pfVar2 = fVar14;
    dVar15 = (double)fVar14 * *(double *)(unaff_EBX + 0x4a0d20);
    dVar9 = (double)fVar14 * *(double *)(unaff_EBX + 0x4a1150);
    lVar5 = (longdouble)_sin((double)(float)dVar8);
    fVar11 = (float)((double)lVar5 * dVar9 + dVar15);
    fVar7 = *(float *)(unaff_EBX + 0x4a0d4c);
    fVar12 = fVar7;
    if ((fVar11 <= fVar7) &&
       (fVar12 = *(float *)(&DAT_004a135c + unaff_EBX),
       *(float *)(&DAT_004a135c + unaff_EBX) <= fVar11)) {
      fVar12 = fVar11;
    }
    *pfVar2 = fVar12;
    fVar6 = ((float)(int)(fVar6 + fVar6) * *(float *)(unaff_EBX + 0x4a1354) *
             *(float *)(unaff_EBX + 0x4a109c) + fVar13) / *(float *)(unaff_EBX + 0x4a0cbc);
    if (*(float *)(unaff_EBX + 0x4a0cc0) <= fVar6) {
      dVar8 = (double)(fVar6 - *(float *)(unaff_EBX + 0x4a0cc0)) * dVar10;
      dVar10 = dVar8 + dVar8 + dVar10;
    }
    else {
      dVar10 = (double)fVar6 * dVar10 + (double)fVar6 * dVar10;
    }
    pfVar2 = *(float **)(unaff_EBX + 0x622b10);
    *pfVar2 = fVar14;
    lVar5 = (longdouble)_sin((double)(float)dVar10);
    fVar14 = (float)(dVar15 + dVar9 * (double)lVar5);
    if ((fVar14 <= fVar7) &&
       (fVar7 = *(float *)(&DAT_004a135c + unaff_EBX),
       *(float *)(&DAT_004a135c + unaff_EBX) <= fVar14)) {
      fVar7 = fVar14;
    }
    *pfVar2 = fVar7;
    return 0.0;
  }
  return 0.0;
}


/* C_WeaponPaintGun::SetPaintPower at 005411c0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void SetPaintPower(C_WeaponPaintGun * this, PaintPowerType type) */

void __thiscall C_WeaponPaintGun::SetPaintPower(C_WeaponPaintGun *this,PaintPowerType type)

{
  code *pcVar1;
  bool bVar2;
  char cVar3;
  int *piVar4;
  C_BasePlayer *this_00;
  int iVar5;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  bVar2 = HasPaintPower(this,type);
  if (bVar2) {
    (**(code **)(*(int *)&(this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.
                          super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity.
                          super_IClientEntity.super_IClientUnknown + 0x460))(this,type);
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x622cd0) + 0x24))
              ((int *)**(undefined4 **)(unaff_EBX + 0x622cd0),this);
    piVar4 = (int *)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x622ba8) + 0x1c))
                              ((int *)**(undefined4 **)(unaff_EBX + 0x622ba8),unaff_EBX + 0x47a824,0
                               ,0);
    if (piVar4 != (int *)0x0) {
      this_00 = (C_BasePlayer *)C_BaseCombatWeapon::GetOwner((C_BaseCombatWeapon *)this);
      if (this_00 != (C_BasePlayer *)0x0) {
        cVar3 = (**(code **)(*(int *)&(this_00->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                      super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                      super_C_BaseEntity.super_IClientEntity.super_IClientUnknown +
                            0x304))(this_00);
        if (cVar3 != '\0') {
          pcVar1 = *(code **)(*piVar4 + 0x30);
          iVar5 = C_BasePlayer::GetUserID(this_00);
          (*pcVar1)(piVar4,&UNK_0044d73c + unaff_EBX,iVar5);
                    /* WARNING: Could not recover jumptable at 0x005412b9. Too many branches */
                    /* WARNING: Treating indirect jump as call */
          (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x622ba8) + 0x24))();
          return;
        }
      }
    }
  }
  return;
}


/* C_WeaponPaintGun::CyclePaintPower at 005412c0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void CyclePaintPower(C_WeaponPaintGun * this, bool bForward) */

void __thiscall C_WeaponPaintGun::CyclePaintPower(C_WeaponPaintGun *this,bool bForward)

{
  PaintPowerType nIndex;
  bool bVar1;
  C_BaseCombatCharacter *pEntity;
  int iVar2;
  int iVar3;
  int unaff_EBX;
  
                    /* Unresolved local var: int nCurrentPowerIndex@[???]
                       Unresolved local var: int nCurrentColor@[???]
                       Unresolved local var: int nNextPowerIndex@[???]
                       Unresolved local var: int nCounter@[???] */
  ___i686_get_pc_thunk_bx();
  bVar1 = HasAnyPaintPower(this);
  if (!bVar1) {
    return;
  }
  pEntity = C_BaseCombatWeapon::GetOwner((C_BaseCombatWeapon *)this);
  bVar1 = C_BasePlayer::IsLocalPlayer((C_BaseEntity *)pEntity);
  if (bVar1) {
    iVar2 = (this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.super_C_BaseCombatWeapon
            .m_iSubType;
  }
  else {
    iVar2 = this->m_nCurrentColor;
  }
  iVar3 = 0;
  if ((((iVar2 != 0) && (iVar3 = 1, iVar2 != 2)) && (iVar3 = 2, iVar2 != 1)) &&
     (iVar3 = -1, iVar2 == 3)) {
    iVar3 = 3;
  }
  do {
    iVar3 = iVar3 + (-(uint)!bForward | 1);
    if (iVar3 == 4) {
      iVar3 = 0;
    }
    else if (iVar3 < 0) {
      iVar3 = 3;
    }
    nIndex = *(PaintPowerType *)(unaff_EBX + 0x4a6022 + iVar3 * 4);
    bVar1 = HasPaintPower(this,nIndex);
  } while (!bVar1);
  SetPaintPower(this,nIndex);
  return;
}


/* NextPaint at 00541440 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

void NextPaint(void)

{
  C_BasePlayer *pCVar1;
  int iVar2;
  C_WeaponPaintGun *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_BasePlayer * pPlayer@[???]
                       Unresolved local var: C_WeaponPaintGun * pPaintGun@[???] */
  ___i686_get_pc_thunk_bx();
  pCVar1 = C_BasePlayer::GetLocalPlayer(-1);
  if (pCVar1 != (C_BasePlayer *)0x0) {
    iVar2 = (**(code **)(*(int *)&(pCVar1->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                  super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                  super_C_BaseEntity.super_IClientEntity.super_IClientUnknown +
                        0x490))(pCVar1);
    if (iVar2 != 0) {
      this = (C_WeaponPaintGun *)
             ___dynamic_cast(iVar2,*(undefined4 *)(unaff_EBX + 0x622acc),unaff_EBX + 0x68f368,0);
      if (this != (C_WeaponPaintGun *)0x0) {
        C_WeaponPaintGun::CyclePaintPower(this,true);
      }
    }
  }
  return;
}


/* __tcf_3 at 0097c5a0 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x3572b8),in_stack_00000008);
  return;
}


/* PrevPaint at 005413d0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

void PrevPaint(void)

{
  C_BasePlayer *pCVar1;
  int iVar2;
  C_WeaponPaintGun *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_BasePlayer * pPlayer@[???]
                       Unresolved local var: C_WeaponPaintGun * pPaintGun@[???] */
  ___i686_get_pc_thunk_bx();
  pCVar1 = C_BasePlayer::GetLocalPlayer(-1);
  if (pCVar1 != (C_BasePlayer *)0x0) {
    iVar2 = (**(code **)(*(int *)&(pCVar1->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                  super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                  super_C_BaseEntity.super_IClientEntity.super_IClientUnknown +
                        0x490))(pCVar1);
    if (iVar2 != 0) {
      this = (C_WeaponPaintGun *)
             ___dynamic_cast(iVar2,*(undefined4 *)(unaff_EBX + 0x622b3c),unaff_EBX + 0x68f3d8,0);
      if (this != (C_WeaponPaintGun *)0x0) {
        C_WeaponPaintGun::CyclePaintPower(this,false);
      }
    }
  }
  return;
}


/* __tcf_4 at 0097c580 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x357318),in_stack_00000008);
  return;
}


/* __tcf_5 at 0097c560 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(&UNK_00357378 + extraout_ECX),in_stack_00000008);
  return;
}


/* _GLOBAL__I_cl_erase_stream_color at 0007a8b0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_cl_erase_stream_color(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

