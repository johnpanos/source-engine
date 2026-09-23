/* DWARF-guided pseudocode for game/client/portal2/c_weapon_paintgun.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 00079e40 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

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
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x23a].m_pPrev + unaff_EBX) = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x23a].m_pPrev + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x23a].m_pPrev + unaff_EBX + 2) = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x23a].m_pPrev + unaff_EBX + 3) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x23a].m_pNext + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x23b].m_pEntity + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x23b].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x23b].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x23b].m_pNext + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x23c].m_pEntity + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x23c].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x23c].m_pPrev + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x23c].m_pNext + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x23d].m_pEntity + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x23d].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x23d].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined **)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x23d].m_pNext + unaff_EBX) =
         &UNK_00ab7fb4 + unaff_EBX;
    this_1 = (ConVar *)(unaff_EBX + 0x916cc4);
    ConVar::ConVar((ConVar *)((int)s_SIMDRandContexts[0x19].m_RandY[0xc] + unaff_EBX + 0xc),
                   (char *)(unaff_EBX + 0x916d06),(char *)(unaff_EBX + 0x916cf8),0x4000,
                   (char *)this_1,in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0,
                   in_stack_ffffffe4,in_stack_ffffffe8);
    uVar1 = *(undefined4 *)(&DAT_00ab6ecc + unaff_EBX);
    ___cxa_atexit(&UNK_008d7fcc + unaff_EBX,0,uVar1);
    ConVar::ConVar((ConVar *)((int)s_SIMDRandContexts[0x19].m_RandY[0x12] + unaff_EBX + 0xc),
                   (char *)(unaff_EBX + 0x916d1e),(char *)(unaff_EBX + 0x8de7b8),0,this_1,
                   (char *)in_stack_ffffffd8,in_stack_ffffffdc,(int)in_stack_ffffffe0);
    ___cxa_atexit(&UNK_008d7fac + unaff_EBX,0,uVar1);
    *(int *)((int)s_SIMDRandContexts[0x19].m_RandY[0x18] + unaff_EBX + 0xc) = unaff_EBX + 0x916d39;
    *(int *)((int)s_SIMDRandContexts[0x19].m_RandY[0x18] + unaff_EBX + 4) = unaff_EBX + 0x4c681c;
    *(undefined4 *)((int)s_SIMDRandContexts[0x19].m_RandY[0x18] + unaff_EBX + 8) = 0;
    this = (RecvTable *)((int)s_SIMDRandContexts[0x19].m_RandY[0x19] + unaff_EBX + 0xc);
    *(RecvTable **)((int)s_SIMDRandContexts[0x19].m_RandY[0x19] + unaff_EBX) = this;
    piVar2 = *(int **)(&DAT_00ab6f44 + unaff_EBX);
    *(int *)((int)s_SIMDRandContexts[0x19].m_RandY[0x19] + unaff_EBX + 4) = *piVar2;
    *piVar2 = unaff_EBX + 0xcd62a4;
    RecvTable::RecvTable(this);
    ___cxa_atexit(&UNK_008d7f8c + unaff_EBX,0,uVar1);
    iVar3 = ClientClassInit<DT_WeaponPaintGun::ignored>((ignored *)0x0);
    *(int *)((int)s_SIMDRandContexts[0x19].m_RandY[0x1b] + unaff_EBX) = iVar3;
    *(undefined4 *)((int)&PTR_GetRenderClipPlane_00b7cef4 + unaff_EBX) = 4;
    *(int *)((int)&PTR_PerformCustomPhysics_00b7cef0 + unaff_EBX) =
         (int)&PTR_CreateClientRagdoll_00b7cf4c + unaff_EBX;
    *(int *)((int)s_SIMDRandContexts[0x19].m_RandY[0x1b] + unaff_EBX + 4) =
         (int)&PTR_PerformCustomPhysics_00b7cef0 + unaff_EBX;
    pIVar4 = GetClassMap();
    (*pIVar4->_vptr_IClassMap[2])
              (pIVar4,unaff_EBX + 0x916d5a,unaff_EBX + 0x916d49,0xe94,unaff_EBX + 0x4c64ac);
    ConCommand::ConCommand
              ((ConCommand *)
               ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x23e].m_pPrev + unaff_EBX),
               (char *)(unaff_EBX + 0x916d6a),(FnCommandCallbackV1_t)(unaff_EBX + 0x4c554c),
               (char *)0x0,0,(FnCommandCompletionCallback)0x0);
    ___cxa_atexit(&UNK_008d7f6c + unaff_EBX,0,uVar1);
    ConCommand::ConCommand
              ((ConCommand *)
               ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x242].m_pPrev + unaff_EBX),
               (char *)(unaff_EBX + 0x916d74),(FnCommandCallbackV1_t)(unaff_EBX + 0x4c54dc),
               (char *)0x0,0,(FnCommandCompletionCallback)0x0);
    ___cxa_atexit(&UNK_008d7f4c + unaff_EBX,0,uVar1);
    ConCommand::ConCommand
              ((ConCommand *)
               (&UNK_0000246c +
               (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
               (char *)(unaff_EBX + 0x916d7e),(FnCommandCallback_t)(unaff_EBX + 0x4c65ac),
               (char *)0x0,0,(FnCommandCompletionCallback)0x0);
    ___cxa_atexit(unaff_EBX + 0x8d7f2c,0,uVar1);
  }
  return;
}


/* __tcf_0 at 00951e20 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fe218),in_stack_00000008);
  return;
}


/* __tcf_1 at 00951e00 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fe298),in_stack_00000008);
  return;
}


/* ChangePaintTo at 00540400 */

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
    DevMsg((char *)(unaff_EBX + 0x450803));
    return;
  }
  iVar1 = _V_stricmp(args->m_ppArgv[1],(char *)(unaff_EBX + 0x450824));
  if (iVar1 == 0) {
                    /* Unresolved local var: C_BasePlayer * pPlayer@[???]
                       Unresolved local var: C_WeaponPaintGun * pPaintGun@[???] */
    pCVar2 = C_BasePlayer::GetLocalPlayer(-1);
    if (pCVar2 != (C_BasePlayer *)0x0) {
      iVar1 = (**(code **)(*(int *)&(pCVar2->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                    super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                    super_C_BaseEntity.super_IClientEntity.super_IClientUnknown +
                          0x48c))(pCVar2);
      if (iVar1 != 0) {
        pCVar3 = (C_WeaponPaintGun *)
                 ___dynamic_cast(iVar1,*(undefined4 *)(unaff_EBX + 0x5f0abf),unaff_EBX + 0x65be7f,0)
        ;
        if (pCVar3 != (C_WeaponPaintGun *)0x0) {
          C_WeaponPaintGun::SetPaintPower(pCVar3,BOUNCE_POWER);
        }
      }
    }
  }
  else {
    pcVar4 = (char *)(unaff_EBX + 0x41de5b);
    if (1 < args->m_nArgc) {
      pcVar4 = args->m_ppArgv[1];
    }
    iVar1 = _V_stricmp(pcVar4,(char *)(unaff_EBX + 0x4408f3));
    if (iVar1 == 0) {
                    /* Unresolved local var: C_BasePlayer * pPlayer@[???]
                       Unresolved local var: C_WeaponPaintGun * pPaintGun@[???] */
      pCVar2 = C_BasePlayer::GetLocalPlayer(-1);
      if (pCVar2 != (C_BasePlayer *)0x0) {
        iVar1 = (**(code **)(*(int *)&(pCVar2->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                      super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                      super_C_BaseEntity.super_IClientEntity.super_IClientUnknown +
                            0x48c))(pCVar2);
        if (iVar1 != 0) {
          pCVar3 = (C_WeaponPaintGun *)
                   ___dynamic_cast(iVar1,*(undefined4 *)(unaff_EBX + 0x5f0abf),unaff_EBX + 0x65be7f,
                                   0);
          if (pCVar3 != (C_WeaponPaintGun *)0x0) {
            C_WeaponPaintGun::SetPaintPower(pCVar3,SPEED_POWER);
          }
        }
      }
    }
    else {
      pcVar4 = (char *)(unaff_EBX + 0x41de5b);
      if (1 < args->m_nArgc) {
        pcVar4 = args->m_ppArgv[1];
      }
      iVar1 = _V_stricmp(pcVar4,(char *)(unaff_EBX + 0x42a1e3));
      if (iVar1 == 0) {
                    /* Unresolved local var: C_BasePlayer * pPlayer@[???]
                       Unresolved local var: C_WeaponPaintGun * pPaintGun@[???] */
        pCVar2 = C_BasePlayer::GetLocalPlayer(-1);
        if (pCVar2 != (C_BasePlayer *)0x0) {
          iVar1 = (**(code **)(*(int *)&(pCVar2->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                        super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                        super_C_BaseEntity.super_IClientEntity.super_IClientUnknown
                              + 0x48c))(pCVar2);
          if (iVar1 != 0) {
            pCVar3 = (C_WeaponPaintGun *)
                     ___dynamic_cast(iVar1,*(undefined4 *)(unaff_EBX + 0x5f0abf),
                                     unaff_EBX + 0x65be7f,0);
            if (pCVar3 != (C_WeaponPaintGun *)0x0) {
              C_WeaponPaintGun::SetPaintPower(pCVar3,STICK_POWER);
            }
          }
        }
      }
      else {
        pcVar4 = (char *)(unaff_EBX + 0x41de5b);
        if (1 < args->m_nArgc) {
          pcVar4 = args->m_ppArgv[1];
        }
        iVar1 = _V_stricmp(pcVar4,(char *)(unaff_EBX + 0x45082b));
        if (iVar1 == 0) {
                    /* Unresolved local var: C_BasePlayer * pPlayer@[???]
                       Unresolved local var: C_WeaponPaintGun * pPaintGun@[???] */
          pCVar2 = C_BasePlayer::GetLocalPlayer(-1);
          if (pCVar2 != (C_BasePlayer *)0x0) {
            iVar1 = (**(code **)(*(int *)&(pCVar2->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                          super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                          super_C_BaseEntity.super_IClientEntity.
                                          super_IClientUnknown + 0x48c))(pCVar2);
            if (iVar1 != 0) {
              pCVar3 = (C_WeaponPaintGun *)
                       ___dynamic_cast(iVar1,*(undefined4 *)(unaff_EBX + 0x5f0abf),
                                       unaff_EBX + 0x65be7f,0);
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


/* C_WeaponPaintGun::YouForgotToImplementOrDeclareClientClass at 0053e3d0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_WeaponPaintGun * this)
    */

int __thiscall C_WeaponPaintGun::YouForgotToImplementOrDeclareClientClass(C_WeaponPaintGun *this)

{
  return 0;
}


/* C_WeaponPaintGun::GetClientClass at 0053e3f0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: ClientClass * GetClientClass(C_WeaponPaintGun * this) */

ClientClass * __thiscall C_WeaponPaintGun::GetClientClass(C_WeaponPaintGun *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x811d00);
}


/* _C_WeaponPaintGun_CreateObject at 00540670 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_WeaponPaintGun_CreateObject(int entnum,int serialNum)

{
  C_BasePortalCombatWeapon *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_WeaponPaintGun * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xe94);
  C_BasePortalCombatWeapon::C_BasePortalCombatWeapon(this);
  (this->super_C_WeaponPortalBase).super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity
  .super_IClientEntity.super_IClientUnknown.super_IHandleEntity =
       (IHandleEntity)(unaff_EBX + 0x65b46a);
  (this->super_C_WeaponPortalBase).super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity
  .super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x65bafe);
  (this->super_C_WeaponPortalBase).super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity
  .super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x65bbb6);
  (this->super_C_WeaponPortalBase).super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity
  .super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x65bbf2);
  (this->super_C_WeaponPortalBase).super_C_BaseCombatWeapon.super_C_BaseAnimating.
  super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x65bc0e);
  this[1].super_C_WeaponPortalBase.super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity
  .super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable = (_func_int_varargs **)0x0;
  this[1].super_C_WeaponPortalBase.super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity
  .super_IClientEntity.super_IClientUnknown.super_IHandleEntity = (IHandleEntity)0x0;
  this[1].super_C_WeaponPortalBase.super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity
  .super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable = (_func_int_varargs **)0x0
  ;
  this[1].super_C_WeaponPortalBase.super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity
  .super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable = (_func_int_varargs **)0x0;
  this[1].super_C_WeaponPortalBase.super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity
  .m_iClassname = (string_t)0x0;
  *(int *)((int)&this[1].super_C_WeaponPortalBase.super_C_BaseCombatWeapon.super_C_BaseAnimating.
                 super_C_BaseEntity.m_VarMap.m_Entries.m_Memory + 4) = 0;
  *(int *)((int)&this[1].super_C_WeaponPortalBase.super_C_BaseCombatWeapon.super_C_BaseAnimating.
                 super_C_BaseEntity.m_VarMap.m_Entries.m_Memory + 8) = 4;
  *(undefined1 *)
   &this[1].super_C_WeaponPortalBase.super_C_BaseCombatWeapon.super_C_BaseAnimating.
    super_C_BaseEntity.m_VarMap.m_Entries.m_Size = 0;
  *(undefined1 *)
   ((int)&this[1].super_C_WeaponPortalBase.super_C_BaseCombatWeapon.super_C_BaseAnimating.
          super_C_BaseEntity.m_VarMap.m_Entries.m_Size + 1) = 0;
  this[1].super_C_WeaponPortalBase.super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity
  .m_VarMap.m_nInterpolatedEntries = 0;
  this[1].super_C_WeaponPortalBase.super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity
  .m_pfnThink.__pfn = (_func___thiscall_void_C_BaseEntity_ptr *)0x0;
  this[1].super_C_WeaponPortalBase.super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity
  .m_VarMap.m_lastInterpolationTime = 0.0;
  this[1].super_C_WeaponPortalBase.super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity
  .m_pfnThink.__delta = 0;
  (**(code **)(unaff_EBX + 0x65b526))(this,entnum,serialNum);
  return &(this->super_C_WeaponPortalBase).super_C_BaseCombatWeapon.super_C_BaseAnimating.
          super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable;
}


/* ClientClassInit<DT_WeaponPaintGun::ignored> at 00079c00 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

int ClientClassInit<DT_WeaponPaintGun::ignored>(ignored *param_1)

{
  RecvProp_conflict1 pArrayProp;
  int iVar1;
  int unaff_EBX;
  RecvProp *pRVar2;
  undefined4 *puVar3;
  DataTableRecvVarProxyFn_conflict1 p_Var4;
  undefined1 in_stack_ffffff4c [56];
  DataTableRecvVarProxyFn_conflict1 varProxy;
  RecvProp local_58;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x26d].m_pPrev +
               unaff_EBX + 3) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc4083f);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  (&UNK_000026ef +
                  (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                  (char *)(unaff_EBX + 0x8e614f),0,4,0,(RecvVarProxyFn)0x0);
      varProxy = *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00ab718f + unaff_EBX);
      p_Var4 = varProxy;
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x272].
                               m_SerialNumber + unaff_EBX + 3),(char *)(unaff_EBX + 0x8ead2f),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00ab791f + unaff_EBX),varProxy);
      RecvPropBool(&local_58,(char *)(unaff_EBX + 0x916eac),0xe7a,1);
      pRVar2 = &local_58;
      puVar3 = (undefined4 *)&stack0xffffff48;
      for (iVar1 = 0xf; iVar1 != 0; iVar1 = iVar1 + -1) {
        *puVar3 = pRVar2->m_pVarName;
        pRVar2 = (RecvProp *)&pRVar2->m_RecvType;
        puVar3 = puVar3 + 1;
      }
      pArrayProp.m_RecvType = in_stack_ffffff4c._0_4_;
      pArrayProp.m_Flags = in_stack_ffffff4c._4_4_;
      pArrayProp.m_StringBufferSize = in_stack_ffffff4c._8_4_;
      pArrayProp.m_bInsideArray = (bool)in_stack_ffffff4c[0xc];
      pArrayProp._17_3_ = in_stack_ffffff4c._13_3_;
      pArrayProp.m_pExtraData = (void *)in_stack_ffffff4c._16_4_;
      pArrayProp.m_pArrayProp = (RecvProp_conflict1 *)in_stack_ffffff4c._20_4_;
      pArrayProp.m_ArrayLengthProxy = (ArrayLengthRecvProxyFn)in_stack_ffffff4c._24_4_;
      pArrayProp.m_ProxyFn = (RecvVarProxyFn)in_stack_ffffff4c._28_4_;
      pArrayProp.m_DataTableProxyFn = (DataTableRecvVarProxyFn_conflict1)in_stack_ffffff4c._32_4_;
      pArrayProp.m_pDataTable = (RecvTable *)in_stack_ffffff4c._36_4_;
      pArrayProp.m_Offset = in_stack_ffffff4c._40_4_;
      pArrayProp.m_ElementStride = in_stack_ffffff4c._44_4_;
      pArrayProp.m_nElements = in_stack_ffffff4c._48_4_;
      pArrayProp.m_pParentArrayPropName = (char *)in_stack_ffffff4c._52_4_;
      pArrayProp.m_pVarName = (char *)p_Var4;
      RecvPropArray3((RecvProp_conflict1 *)
                     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x276].m_pEntity +
                     unaff_EBX + 3),(char *)(unaff_EBX + 0x916ebb),0xe7a,1,5,pArrayProp,varProxy);
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x279].m_pNext +
                  unaff_EBX + 3),(char *)(unaff_EBX + 0x916ec7),0xe74,4,0,(RecvVarProxyFn)0x0);
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x27d].m_pPrev +
                   unaff_EBX + 3),(char *)(unaff_EBX + 0x916ed7),0xe78,1);
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x281].m_SerialNumber +
                   unaff_EBX + 3),(char *)(unaff_EBX + 0x916ee6),0xe79,1);
      ___cxa_guard_release(unaff_EBX + 0xc4083f);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_SIMDRandContexts[0x1a].m_RandY[5] + unaff_EBX + 0xf),
             (RecvProp_conflict1 *)
             ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x272].m_SerialNumber +
             unaff_EBX + 3),5,(char *)(unaff_EBX + 0x916ef5));
  return 1;
}


/* __tcf_2 at 00951de0 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3fe328),in_stack_00000008);
  return;
}


/* C_WeaponPaintGun::GetPredDescMap at 0053e400 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: datamap_t * GetPredDescMap(C_WeaponPaintGun * this) */

datamap_t * __thiscall C_WeaponPaintGun::GetPredDescMap(C_WeaponPaintGun *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x6b893c);
}


/* PredMapInit<C_WeaponPaintGun> at 0053e410 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

datamap_t * PredMapInit<C_WeaponPaintGun>(C_WeaponPaintGun *param_1)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *(undefined4 *)(extraout_ECX + 0x6b892d) = 4;
  *(int *)(extraout_ECX + 0x6b8929) = extraout_ECX + 0x6b8985;
  return (datamap_t *)(extraout_ECX + 0x6b8929);
}


/* CC_WeaponPaintGunFactory at 00540300 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

C_BaseEntity * CC_WeaponPaintGunFactory(void)

{
  C_BasePortalCombatWeapon *this;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xe94);
  C_BasePortalCombatWeapon::C_BasePortalCombatWeapon(this);
  (this->super_C_WeaponPortalBase).super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity
  .super_IClientEntity.super_IClientUnknown.super_IHandleEntity =
       (IHandleEntity)(unaff_EBX + 0x65b7d4);
  (this->super_C_WeaponPortalBase).super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity
  .super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x65be68);
  (this->super_C_WeaponPortalBase).super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity
  .super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x65bf20);
  (this->super_C_WeaponPortalBase).super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity
  .super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x65bf5c);
  (this->super_C_WeaponPortalBase).super_C_BaseCombatWeapon.super_C_BaseAnimating.
  super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x65bf78);
  this[1].super_C_WeaponPortalBase.super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity
  .super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable = (_func_int_varargs **)0x0;
  this[1].super_C_WeaponPortalBase.super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity
  .super_IClientEntity.super_IClientUnknown.super_IHandleEntity = (IHandleEntity)0x0;
  this[1].super_C_WeaponPortalBase.super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity
  .super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable = (_func_int_varargs **)0x0
  ;
  this[1].super_C_WeaponPortalBase.super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity
  .super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable = (_func_int_varargs **)0x0;
  this[1].super_C_WeaponPortalBase.super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity
  .m_iClassname = (string_t)0x0;
  *(int *)((int)&this[1].super_C_WeaponPortalBase.super_C_BaseCombatWeapon.super_C_BaseAnimating.
                 super_C_BaseEntity.m_VarMap.m_Entries.m_Memory + 4) = 0;
  *(int *)((int)&this[1].super_C_WeaponPortalBase.super_C_BaseCombatWeapon.super_C_BaseAnimating.
                 super_C_BaseEntity.m_VarMap.m_Entries.m_Memory + 8) = 4;
  *(undefined1 *)
   &this[1].super_C_WeaponPortalBase.super_C_BaseCombatWeapon.super_C_BaseAnimating.
    super_C_BaseEntity.m_VarMap.m_Entries.m_Size = 0;
  *(undefined1 *)
   ((int)&this[1].super_C_WeaponPortalBase.super_C_BaseCombatWeapon.super_C_BaseAnimating.
          super_C_BaseEntity.m_VarMap.m_Entries.m_Size + 1) = 0;
  this[1].super_C_WeaponPortalBase.super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity
  .m_VarMap.m_nInterpolatedEntries = 0;
  this[1].super_C_WeaponPortalBase.super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity
  .m_pfnThink.__pfn = (_func___thiscall_void_C_BaseEntity_ptr *)0x0;
  this[1].super_C_WeaponPortalBase.super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity
  .m_VarMap.m_lastInterpolationTime = 0.0;
  this[1].super_C_WeaponPortalBase.super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity
  .m_pfnThink.__delta = 0;
  return (C_BaseEntity *)this;
}


/* C_WeaponPaintGun::C_WeaponPaintGun at 0053e590 */

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
       (IClientUnknown)(unaff_EBX + 0x65d54b);
  (this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.super_C_BaseCombatWeapon.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x65dbdf);
  (this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.super_C_BaseCombatWeapon.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x65dc97);
  (this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.super_C_BaseCombatWeapon.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x65dcd3);
  (this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.super_C_BaseCombatWeapon.
  super_C_BaseAnimating.super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x65dcef);
  (this->m_pStreamEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
  (this->m_pStreamEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
  (this->m_pStreamEffect).m_pObject = (CNewParticleEffect *)0x0;
  this->m_flBobScale = 0.0;
  this->m_flAccumulatedTime = 0.0;
  this->m_flLastThinkTime = 0.0;
  this->m_nCurrentColor = 4;
  this->m_bFiringPaint = false;
  this->m_bFiringErase = false;
  this->m_pLastOwner = (C_BaseCombatCharacter *)0x0;
  (this->m_pLastEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
  (this->m_pLastEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
  (this->m_pLastEffect).m_pObject = (CNewParticleEffect *)0x0;
  return;
}


/* C_WeaponPaintGun::C_WeaponPaintGun at 00540230 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void C_WeaponPaintGun(C_WeaponPaintGun * this, C_WeaponPaintGun * this)
    */

void __thiscall C_WeaponPaintGun::C_WeaponPaintGun(C_WeaponPaintGun *this,C_WeaponPaintGun *this_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BasePortalCombatWeapon::C_BasePortalCombatWeapon(&this->super_C_BasePortalCombatWeapon);
  (this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.super_C_BaseCombatWeapon.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientUnknown =
       (IClientUnknown)(&UNK_0065b8ab + unaff_EBX);
  (this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.super_C_BaseCombatWeapon.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x65bf3f);
  (this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.super_C_BaseCombatWeapon.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x65bff7);
  (this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.super_C_BaseCombatWeapon.
  super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x65c033);
  (this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.super_C_BaseCombatWeapon.
  super_C_BaseAnimating.super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x65c04f);
  (this->m_pStreamEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
  (this->m_pStreamEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
  (this->m_pStreamEffect).m_pObject = (CNewParticleEffect *)0x0;
  this->m_flBobScale = 0.0;
  this->m_flAccumulatedTime = 0.0;
  this->m_flLastThinkTime = 0.0;
  this->m_nCurrentColor = 4;
  this->m_bFiringPaint = false;
  this->m_bFiringErase = false;
  this->m_pLastOwner = (C_BaseCombatCharacter *)0x0;
  (this->m_pLastEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
  (this->m_pLastEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
  (this->m_pLastEffect).m_pObject = (CNewParticleEffect *)0x0;
  return;
}


/* C_WeaponPaintGun::Precache at 0053e530 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void Precache(C_WeaponPaintGun * this) */

void __thiscall C_WeaponPaintGun::Precache(C_WeaponPaintGun *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  PrecacheParticleSystem(&UNK_0045253b + unaff_EBX);
  PrecacheParticleSystem(&UNK_00452551 + unaff_EBX);
  PrecacheParticleSystem((char *)(unaff_EBX + 0x42632b));
  PrecacheParticleSystem(&UNK_00452567 + unaff_EBX);
  C_BaseCombatWeapon::Precache((C_BaseCombatWeapon *)this);
  return;
}


/* C_WeaponPaintGun::Spawn at 0053f410 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void Spawn(C_WeaponPaintGun * this) */

void __thiscall C_WeaponPaintGun::Spawn(C_WeaponPaintGun *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseCombatWeapon::Spawn((C_BaseCombatWeapon *)this);
  this->m_flLastThinkTime = *(float *)(**(int **)(unaff_EBX + 0x5f192f) + 0xc);
  this->m_flAccumulatedTime =
       *(float *)(unaff_EBX + 0x477423) /
       *(float *)(*(int *)(*(int *)(unaff_EBX + 0x5f217b) + 0x1c) + 0x2c);
  this->m_nBlobRandomSeed = 0;
  C_BaseEntity::SetNextThink((C_BaseEntity *)this,-1293.0,(char *)0x0);
  SetPaintPower(this,NO_POWER);
  return;
}


/* C_WeaponPaintGun::ClientThink at 0053e770 */

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
  fVar1 = *(float *)(**(int **)(unaff_EBX + 0x5f25d2) + 0xc);
  fVar2 = this->m_flLastThinkTime;
  this->m_flLastThinkTime = fVar1;
  pCVar5 = C_BaseCombatWeapon::GetOwner((C_BaseCombatWeapon *)this);
  if (pCVar5 == (C_BaseCombatCharacter *)0x0) {
LAB_0053e7ce:
    this_00 = (C_Portal_Player *)0x0;
  }
  else {
    cVar3 = (**(code **)((int)(pCVar5->super_C_BaseFlex).super_C_BaseAnimatingOverlay.
                              super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.
                              super_IClientUnknown.super_IHandleEntity + 0x300))(pCVar5);
    if (cVar3 == '\0') goto LAB_0053e7ce;
    in_stack_ffffffa8 = *(C_Portal_Player **)(unaff_EBX + 0x5f25ae);
    this_00 = (C_Portal_Player *)
              ___dynamic_cast(pCVar5,in_stack_ffffffa8,*(undefined4 *)(&DAT_005f27ea + unaff_EBX),0)
    ;
  }
  bVar4 = C_BaseCombatWeapon::IsCarrierAlive((C_BaseCombatWeapon *)this);
  if (((bVar4) &&
      ((this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.super_C_BaseCombatWeapon.
       m_iState.m_Value == 2)) &&
     (flDeltaTime = (C_Portal_Player *)(fVar1 - fVar2),
     *(float *)(unaff_EBX + 0x4781d6) <= (float)flDeltaTime &&
     (float)flDeltaTime != *(float *)(unaff_EBX + 0x4781d6))) {
    if (this->m_bFiringPaint == false) {
      paintType = 4;
      if (this->m_bFiringErase == false) goto LAB_0053e7e9;
    }
    else {
      paintType = this->m_nCurrentColor;
    }
                    /* Unresolved local var: PaintPowerType blobType@[???] */
    SprayPaint(this,(float)flDeltaTime,paintType);
    in_stack_ffffffa8 = flDeltaTime;
  }
LAB_0053e7e9:
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
    if (bVar4) goto LAB_0053e860;
  }
  local_24 = 0xffffff;
  local_20[0]._color[0] = 0xff;
  local_20[0]._color[1] = 0xff;
  local_20[0]._color[2] = 0xff;
  local_20[0]._color[3] = '\0';
LAB_0053e860:
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
                        super_IClientEntity.super_IClientUnknown + 0x26c))(this,0xc4a1a000);
  return;
}


/* C_WeaponPaintGun::Simulate at 0053ed10 */

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


/* C_WeaponPaintGun::OnPreDataChanged at 0053e670 */

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


/* C_WeaponPaintGun::OnDataChanged at 0053ffc0 */

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
                        super_IClientEntity.super_IClientUnknown + 0x26c))(this,0xc4a1a000);
  if (updateType == DATA_UPDATE_CREATED) {
                    /* Unresolved local var: C_Portal_Player * pOwner@[???] */
    (this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.super_C_BaseCombatWeapon.
    m_iSubType = this->m_nCurrentColor;
    pCVar8 = C_BaseCombatWeapon::GetOwner((C_BaseCombatWeapon *)this);
    if (((pCVar8 != (C_BaseCombatCharacter *)0x0) &&
        (cVar6 = (**(code **)((int)(pCVar8->super_C_BaseFlex).super_C_BaseAnimatingOverlay.
                                   super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.
                                   super_IClientUnknown.super_IHandleEntity + 0x300))(pCVar8),
        cVar6 != '\0')) &&
       (this_00 = (C_Portal_Player *)
                  ___dynamic_cast(pCVar8,*(undefined4 *)(unaff_EBX + 0x5f0d5e),
                                  *(undefined4 *)(unaff_EBX + 0x5f0f9a),0),
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


/* C_WeaponPaintGun::ChangeRenderColor at 0053e970 */

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
                                super_IClientUnknown.super_IHandleEntity + 0x300))(pEntity),
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
                 super_C_BaseEntity.m_pIntermediateData + 0x29) != '\0')) &&
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


/* C_WeaponPaintGun::StartHoseEffect at 0053ed40 */

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
  if (*(int *)(*(int *)(unaff_EBX + 0x811365) + 0x30) == 0) {
    return;
  }
  puVar8 = local_20;
  local_20[0] = 0;
  this_00 = (C_BasePlayer *)C_BaseCombatWeapon::GetOwner((C_BaseCombatWeapon *)this);
  if ((this_00 == (C_BasePlayer *)0x0) ||
     (cVar4 = (**(code **)((int)(this_00->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
                                super_IHandleEntity + 0x300))(this_00), cVar4 == '\0')) {
    this_00 = (C_BasePlayer *)0x0;
    bVar5 = C_BasePlayer::IsLocalPlayer((C_BaseEntity *)0x0);
    if (bVar5) goto LAB_0053ee80;
  }
  else {
    bVar5 = C_BasePlayer::IsLocalPlayer((C_BaseEntity *)this_00);
    if (bVar5) {
LAB_0053ee80:
      nOldSlot = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5f2001) + 0x1f8))
                           ((int *)**(undefined4 **)(unaff_EBX + 0x5f2001));
      pchContext = (char *)(unaff_EBX + 0x451cc1);
      this_01 = &local_54;
      CSetActiveSplitScreenPlayerGuard::CSetActiveSplitScreenPlayerGuard
                (this_01,pchContext,0x101,(C_BaseEntity *)this_00,nOldSlot,false,in_stack_ffffff8c,
                 in_stack_ffffff90,in_stack_ffffff94,in_stack_ffffff98,(int)this_01,SUB41(puVar8,0))
      ;
      if ((this->m_pStreamEffect).m_pObject == (CNewParticleEffect *)0x0) {
        pCVar7 = C_BasePlayer::GetViewModel(this_00,0);
        pchContext = (char *)(unaff_EBX + 0x451e96);
        pCVar6 = CParticleProperty::Create
                           (&(pCVar7->super_C_BaseAnimating).super_C_BaseEntity.m_Particles.
                             super_CParticleProperty,pchContext,PATTACH_POINT_FOLLOW,
                            (char *)(unaff_EBX + 0x451e89));
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
      goto LAB_0053edf0;
    }
  }
  if ((this->m_pStreamEffect).m_pObject == (CNewParticleEffect *)0x0) {
    in_stack_ffffff78 = (char *)(unaff_EBX + 0x451e96);
    pCVar6 = CParticleProperty::Create
                       (&(this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.
                         super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity.
                         m_Particles.super_CParticleProperty,in_stack_ffffff78,PATTACH_POINT_FOLLOW,
                        (char *)(unaff_EBX + 0x41fc3d));
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
LAB_0053edf0:
  local_34.z = (vec_t)(local_20[0] >> 0x10 & 0xff);
  local_34.y = (vec_t)(local_20[0] >> 8 & 0xff);
  local_34.x = (vec_t)(local_20[0] & 0xff);
  CNewParticleEffect::SetControlPoint((this->m_pStreamEffect).m_pObject,1,&local_34);
  return;
}


/* C_WeaponPaintGun::StopHoseEffect at 0053eac0 */

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
                                  super_IHandleEntity + 0x300))(this_00), cVar4 == '\0')) {
      this_00 = (C_BasePlayer *)0x0;
    }
    bVar5 = C_BasePlayer::IsLocalPlayer((C_BaseEntity *)this_00);
    if (bVar5) {
      nOldSlot = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5f2284) + 0x1f8))
                           ((int *)**(undefined4 **)(unaff_EBX + 0x5f2284));
      this_01 = &local_3c;
      CSetActiveSplitScreenPlayerGuard::CSetActiveSplitScreenPlayerGuard
                (this_01,(char *)(unaff_EBX + 0x451f44),0x121,(C_BaseEntity *)this_00,nOldSlot,false
                 ,in_stack_ffffffac,in_stack_ffffffb0,in_stack_ffffffb4,in_stack_ffffffb8,
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


/* C_WeaponPaintGun::GetEffectViewModel at 0053e440 */

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
                          0x300))(this_00), cVar1 == '\0')) {
    this_00 = (C_BasePlayer *)0x0;
    bVar2 = C_BasePlayer::IsLocalPlayer((C_BaseEntity *)0x0);
  }
  else {
    bVar2 = C_BasePlayer::IsLocalPlayer((C_BaseEntity *)this_00);
  }
  pCVar4 = (C_BaseViewModel *)0x0;
  if (bVar2) {
    iVar3 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5f2904) + 0x1f8))
                      ((int *)**(undefined4 **)(unaff_EBX + 0x5f2904));
    CSetActiveSplitScreenPlayerGuard::CSetActiveSplitScreenPlayerGuard
              ((CSetActiveSplitScreenPlayerGuard *)&stack0xffffffc4,&UNK_004525c4 + unaff_EBX,0x139,
               (C_BaseEntity *)this_00,iVar3,false,in_stack_ffffffbc,in_stack_ffffffc0,
               in_stack_ffffffc4,in_stack_ffffffc8,in_stack_ffffffcc,(bool)in_stack_ffffffd0);
    iVar3 = 0;
    pCVar4 = C_BasePlayer::GetViewModel(this_00,0);
    CSetActiveSplitScreenPlayerGuard::~CSetActiveSplitScreenPlayerGuard
              ((CSetActiveSplitScreenPlayerGuard *)&stack0xffffffc4,iVar3);
  }
  return pCVar4;
}


/* C_WeaponPaintGun::AddViewmodelBob at 0053f7b0 */

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
                        super_IClientEntity.super_IClientUnknown + 0x590))(this);
  pCVar3 = C_BaseCombatWeapon::GetOwner((C_BaseCombatWeapon *)this);
  if (pCVar3 != (C_BaseCombatCharacter *)0x0) {
    cVar2 = (**(code **)((int)(pCVar3->super_C_BaseFlex).super_C_BaseAnimatingOverlay.
                              super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.
                              super_IClientUnknown.super_IHandleEntity + 0x300))(pCVar3);
    if (cVar2 != '\0') {
      this_00 = (C_Portal_Player *)
                ___dynamic_cast(pCVar3,*(undefined4 *)(unaff_EBX + 0x5f156b),
                                *(undefined4 *)(unaff_EBX + 0x5f17a7),0);
      if (this_00 != (C_Portal_Player *)0x0) {
        pfVar1 = *(float **)(&DAT_005f185b + unaff_EBX);
        fVar5 = *(float *)(unaff_EBX + 0x47717f) * *pfVar1;
        origin->x = fVar5 * local_28[0].x + origin->x;
        origin->y = fVar5 * local_28[0].y + origin->y;
        origin->z = fVar5 * local_28[0].z + origin->z;
        pCVar4 = C_Portal_Player::GetPortalPlayerLocalData(this_00);
        local_b0 = *(float *)(unaff_EBX + 0x47717f) * *pfVar1;
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
        fVar5 = *(float *)(unaff_EBX + 0x4777c7) * **(float **)(unaff_EBX + 0x5f184f);
        origin->x = fVar6 + fVar5 * local_34.x;
        origin->y = fVar7 + fVar5 * local_34.y;
        origin->z = fVar8 + fVar5 * local_34.z;
        fVar5 = *(float *)(unaff_EBX + 0x47710f);
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
                  (&local_104,&local_58,*(float *)(&DAT_004777bf + unaff_EBX) * *pfVar1);
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
                   *(float *)(unaff_EBX + 0x4777c3) * **(float **)(unaff_EBX + 0x5f184f));
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


/* C_WeaponPaintGun::CalcViewmodelBob at 0053f4a0 */

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
                                 super_IClientUnknown.super_IHandleEntity + 0x300))(pCVar4),
      cVar3 != '\0')) && (*(float *)(**(int **)(unaff_EBX + 0x5f18a0) + 0x10) != 0.0)) {
    fVar7 = (pCVar4->super_C_BaseFlex).super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
            super_C_BaseEntity.m_vecVelocity.x;
    fVar14 = (pCVar4->super_C_BaseFlex).super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
             super_C_BaseEntity.m_vecVelocity.y;
    fVar6 = (pCVar4->super_C_BaseFlex).super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
            super_C_BaseEntity.m_vecVelocity.z;
    fVar6 = SQRT(fVar7 * fVar7 + fVar14 * fVar14 + fVar6 * fVar6);
    fVar7 = *(float *)(pCVar4[1].super_C_BaseFlex.super_C_BaseAnimatingOverlay.super_C_BaseAnimating
                       .super_C_BaseEntity.m_iSignifierName + 0x9c);
    fVar14 = fVar7;
    if ((fVar6 <= fVar7) &&
       (fVar14 = (float)((uint)fVar7 ^ *(uint *)(&DAT_0047c380 + unaff_EBX)),
       (float)((uint)fVar7 ^ *(uint *)(&DAT_0047c380 + unaff_EBX)) <= fVar6)) {
      fVar14 = fVar6;
    }
    fVar6 = 0.0;
    if ((fVar7 != 0.0) || (NAN(fVar7))) {
      fVar6 = fVar14 / fVar7 + 0.0;
    }
    else if (0.0 <= fVar14 - fVar7) {
      fVar6 = *(float *)(unaff_EBX + 0x477394);
    }
    iVar1 = **(int **)(unaff_EBX + 0x5f18a0);
    fVar13 = fVar6 * *(float *)(iVar1 + 0x10) + *(float *)(unaff_EBX + 0x77af98);
    *(float *)(unaff_EBX + 0x77af98) = fVar13;
    *(undefined4 *)(unaff_EBX + 0x77af94) = *(undefined4 *)(iVar1 + 0xc);
    fVar6 = fVar13 / *(float *)(&DAT_00477ac4 + unaff_EBX);
    fVar7 = ((float)(int)fVar6 * *(float *)(&DAT_00477ac8 + unaff_EBX) + fVar13) /
            *(float *)(&DAT_00477ac4 + unaff_EBX);
    if ((*(float *)(unaff_EBX + 0x477420) <= fVar7) ||
       (NAN(fVar7) || NAN(*(float *)(unaff_EBX + 0x477420)))) {
      dVar10 = *(double *)(unaff_EBX + 0x477780);
      dVar8 = (double)(fVar7 - *(float *)(unaff_EBX + 0x477420)) * dVar10;
      dVar8 = dVar8 + dVar8 + dVar10;
    }
    else {
      dVar10 = *(double *)(unaff_EBX + 0x477780);
      dVar8 = (double)fVar7 * dVar10 + (double)fVar7 * dVar10;
    }
    fVar14 = fVar14 * *(float *)(&DAT_00477ac0 + unaff_EBX);
    pfVar2 = *(float **)(unaff_EBX + 0x5f1b6c);
    *pfVar2 = fVar14;
    dVar15 = (double)fVar14 * *(double *)(unaff_EBX + 0x477480);
    dVar9 = (double)fVar14 * *(double *)(unaff_EBX + 0x4778b0);
    lVar5 = (longdouble)_sin((double)(float)dVar8);
    fVar11 = (float)((double)lVar5 * dVar9 + dVar15);
    fVar7 = *(float *)(unaff_EBX + 0x4774ac);
    fVar12 = fVar7;
    if ((fVar11 <= fVar7) &&
       (fVar12 = *(float *)(&DAT_00477acc + unaff_EBX),
       *(float *)(&DAT_00477acc + unaff_EBX) <= fVar11)) {
      fVar12 = fVar11;
    }
    *pfVar2 = fVar12;
    fVar6 = ((float)(int)(fVar6 + fVar6) * *(float *)(&DAT_00477ac4 + unaff_EBX) *
             *(float *)(unaff_EBX + 0x477800) + fVar13) / *(float *)(unaff_EBX + 0x47741c);
    if (*(float *)(unaff_EBX + 0x477420) <= fVar6) {
      dVar8 = (double)(fVar6 - *(float *)(unaff_EBX + 0x477420)) * dVar10;
      dVar10 = dVar8 + dVar8 + dVar10;
    }
    else {
      dVar10 = (double)fVar6 * dVar10 + (double)fVar6 * dVar10;
    }
    pfVar2 = *(float **)(unaff_EBX + 0x5f1b60);
    *pfVar2 = fVar14;
    lVar5 = (longdouble)_sin((double)(float)dVar10);
    fVar14 = (float)(dVar15 + dVar9 * (double)lVar5);
    if ((fVar14 <= fVar7) &&
       (fVar7 = *(float *)(&DAT_00477acc + unaff_EBX),
       *(float *)(&DAT_00477acc + unaff_EBX) <= fVar14)) {
      fVar7 = fVar14;
    }
    *pfVar2 = fVar7;
    return 0.0;
  }
  return 0.0;
}


/* C_WeaponPaintGun::SetPaintPower at 0053f120 */

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
                          super_IClientEntity.super_IClientUnknown + 0x45c))(this,type);
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5f1d28) + 0x24))
              ((int *)**(undefined4 **)(unaff_EBX + 0x5f1d28),this);
    piVar4 = (int *)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5f1bfc) + 0x1c))
                              ((int *)**(undefined4 **)(unaff_EBX + 0x5f1bfc),unaff_EBX + 0x451ac7,0
                               ,0);
    if (piVar4 != (int *)0x0) {
      this_00 = (C_BasePlayer *)C_BaseCombatWeapon::GetOwner((C_BaseCombatWeapon *)this);
      if (this_00 != (C_BasePlayer *)0x0) {
        cVar3 = (**(code **)(*(int *)&(this_00->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                      super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                      super_C_BaseEntity.super_IClientEntity.super_IClientUnknown +
                            0x300))(this_00);
        if (cVar3 != '\0') {
          pcVar1 = *(code **)(*piVar4 + 0x30);
          iVar5 = C_BasePlayer::GetUserID(this_00);
          (*pcVar1)(piVar4,unaff_EBX + 0x424d4c,iVar5);
                    /* WARNING: Could not recover jumptable at 0x0053f219. Too many branches */
                    /* WARNING: Treating indirect jump as call */
          (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5f1bfc) + 0x24))();
          return;
        }
      }
    }
  }
  return;
}


/* C_WeaponPaintGun::CyclePaintPower at 0053f220 */

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
    nIndex = *(PaintPowerType *)(&DAT_0047c5f2 + iVar3 * 4 + unaff_EBX);
    bVar1 = HasPaintPower(this,nIndex);
  } while (!bVar1);
  SetPaintPower(this,nIndex);
  return;
}


/* NextPaint at 0053f3a0 */

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
                        0x48c))(pCVar1);
    if (iVar2 != 0) {
      this = (C_WeaponPaintGun *)
             ___dynamic_cast(iVar2,*(undefined4 *)
                                    (BaseModUI::SpinnerControl::OnSetCurrentItem + unaff_EBX + 4),
                             unaff_EBX + 0x65cee4,0);
      if (this != (C_WeaponPaintGun *)0x0) {
        C_WeaponPaintGun::CyclePaintPower(this,true);
      }
    }
  }
  return;
}


/* __tcf_3 at 00951dc0 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x3685d8),in_stack_00000008);
  return;
}


/* PrevPaint at 0053f330 */

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
                        0x48c))(pCVar1);
    if (iVar2 != 0) {
      this = (C_WeaponPaintGun *)
             ___dynamic_cast(iVar2,*(undefined4 *)(unaff_EBX + 0x5f1b94),unaff_EBX + 0x65cf54,0);
      if (this != (C_WeaponPaintGun *)0x0) {
        C_WeaponPaintGun::CyclePaintPower(this,false);
      }
    }
  }
  return;
}


/* __tcf_4 at 00951da0 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x368638),in_stack_00000008);
  return;
}


/* __tcf_5 at 00951d80 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x368698),in_stack_00000008);
  return;
}


/* _GLOBAL__I_cl_erase_stream_color at 0007a130 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_cl_erase_stream_color(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

