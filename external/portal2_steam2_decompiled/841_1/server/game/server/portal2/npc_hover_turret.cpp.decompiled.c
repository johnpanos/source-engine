/* DWARF-guided pseudocode for game/server/portal2/npc_hover_turret.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000ac6e0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  int *piVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  IEntityFactoryDictionary *pIVar4;
  datamap_t *pdVar5;
  int iVar6;
  int iVar7;
  int unaff_EBX;
  undefined4 *puVar8;
  longlong lVar9;
  ConVar *in_stack_ffffffd4;
  SendTable *this;
  char *in_stack_ffffffd8;
  char *in_stack_ffffffdc;
  char *s2;
  int in_stack_ffffffe0;
  
  lVar9 = ___i686_get_pc_thunk_bx();
  if (lVar9 == 0xffff00000001) {
    *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x26c].m_pPrev + unaff_EBX) =
         0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x26c].m_pPrev + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x26c].m_pPrev + unaff_EBX + 2) = 0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x26c].m_pPrev + unaff_EBX + 3) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x26c].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x26d].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x26d].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x26d].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x26d].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x26e].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x26e].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x26e].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x26e].m_pNext + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x26f].m_pEntity + unaff_EBX)
         = 0x7f7fffff;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x26f].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x26f].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x26f].m_pNext + unaff_EBX) =
         &UNK_00b9fc14 + unaff_EBX;
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<AI_EnemyInfo_t>::dataDesc[0].flatOffset + unaff_EBX + -0x28),
                   (char *)(unaff_EBX + 0xa04b54),(char *)(unaff_EBX + 0xa04b50),0,in_stack_ffffffd4
                   ,in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0);
    ___cxa_atexit(unaff_EBX + 0x990e8c,0,*(undefined4 *)(&DAT_00b9a460 + unaff_EBX));
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x270].m_pEntity + unaff_EBX)
         = &UNK_00c1f9c4 + unaff_EBX;
    pIVar4 = EntityFactoryDictionary();
    (**pIVar4->_vptr_IEntityFactoryDictionary)(pIVar4,unaff_EBX + 0xda02e4,unaff_EBX + 0xa04b6b);
    pdVar5 = DataMapInit<CHoverTurretTether>((CHoverTurretTether *)0x0);
    *(datamap_t **)((int)DataMapInit<AI_EnemyInfo_t>::dataDesc[1].flatOffset + unaff_EBX) = pdVar5;
    ConVar::ConVar((ConVar *)
                   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x270].m_pPrev + unaff_EBX),
                   (char *)(unaff_EBX + 0xa04b86),(char *)(unaff_EBX + 0x9adac8),0,in_stack_ffffffd4
                   ,in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0);
    ___cxa_atexit(unaff_EBX + 0x990e6c,0,*(undefined4 *)(&DAT_00b9a460 + unaff_EBX));
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x276].m_pEntity + unaff_EBX)
         = &UNK_00c1f9e4 + unaff_EBX;
    pIVar4 = EntityFactoryDictionary();
    (**pIVar4->_vptr_IEntityFactoryDictionary)(pIVar4,unaff_EBX + 0xda0344,unaff_EBX + 0xa014bc);
    pdVar5 = DataMapInit<CNPC_HoverTurret>((CNPC_HoverTurret *)0x0);
    *(datamap_t **)((int)DataMapInit<AI_EnemyInfo_t>::dataDesc[1].flatOffset + unaff_EBX + 4) =
         pdVar5;
    iVar7 = unaff_EBX + 0xda0348;
    s2 = (char *)(unaff_EBX + 0xa04b3f);
    *(char **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x276].m_SerialNumber + unaff_EBX)
         = s2;
    this = (SendTable *)((int)DataMapInit<AI_EnemyInfo_t>::dataDesc[1].flatOffset + unaff_EBX + 8);
    *(SendTable **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x276].m_pPrev + unaff_EBX) =
         this;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x277].m_SerialNumber + unaff_EBX) = 0xffff
    ;
    piVar1 = *(int **)(&DAT_00b9a500 + unaff_EBX);
    puVar8 = (undefined4 *)*piVar1;
    if (puVar8 == (undefined4 *)0x0) {
      *piVar1 = iVar7;
      *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x276].m_pNext + unaff_EBX)
           = 0;
    }
    else {
      puVar2 = (undefined4 *)puVar8[2];
      iVar6 = _V_stricmp((char *)*puVar8,s2);
      if (iVar6 < 1) {
        while ((puVar3 = puVar2, puVar3 != (undefined4 *)0x0 &&
               (iVar6 = _V_stricmp((char *)*puVar3,s2), iVar6 < 1))) {
          puVar2 = (undefined4 *)puVar3[2];
          puVar8 = puVar3;
        }
        *(undefined4 **)(iVar7 + 8) = puVar3;
        puVar8[2] = iVar7;
      }
      else {
        *(int *)(iVar7 + 8) = *piVar1;
        *piVar1 = iVar7;
      }
    }
    SendTable::SendTable(this);
    ___cxa_atexit(unaff_EBX + 0x990e4c,0,*(undefined4 *)(&DAT_00b9a460 + unaff_EBX));
    iVar7 = ServerClassInit<DT_NPC_HoverTurret::ignored>((ignored *)0x0);
    *(int *)((int)DataMapInit<AI_EnemyInfo_t>::dataDesc[2].flatOffset + unaff_EBX + -0x24) = iVar7;
    CAI_LocalIdSpace::CAI_LocalIdSpace
              ((CAI_LocalIdSpace *)
               ((int)DataMapInit<AI_EnemyInfo_t>::dataDesc[2].flatOffset + unaff_EBX + -4),false);
    CAI_LocalIdSpace::CAI_LocalIdSpace
              ((CAI_LocalIdSpace *)
               ((int)DataMapInit<AI_EnemyInfo_t>::dataDesc[3].flatOffset + unaff_EBX + -0x2c),false)
    ;
    CAI_LocalIdSpace::CAI_LocalIdSpace
              ((CAI_LocalIdSpace *)
               ((int)DataMapInit<AI_EnemyInfo_t>::dataDesc[3].flatOffset + unaff_EBX + -0x14),false)
    ;
    CAI_LocalIdSpace::CAI_LocalIdSpace
              ((CAI_LocalIdSpace *)
               ((int)DataMapInit<AI_EnemyInfo_t>::dataDesc[3].flatOffset + unaff_EBX + 4),false);
  }
  return;
}


/* __tcf_3 at 00a3d580 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38b1d8),in_stack_00000008);
  return;
}


/* GetHoverTurretTalkName at 007202b0 */

char * GetHoverTurretTalkName(int iState)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(char **)(extraout_ECX + 0x6445a8 + iState * 4);
}


/* CNPC_HoverTurret::TalkThink at 007207f0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void TalkThink(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::TalkThink(CNPC_HoverTurret *this)

{
  uint uVar1;
  int iVar2;
  int unaff_EBX;
  int *piVar3;
  
  ___i686_get_pc_thunk_bx();
  uVar1 = ((CNetworkVarBase<int,CBaseEntity::NetworkVar_m_fFlags> *)
          ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                 super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                 super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x130))->m_Value
  ;
  if (((uVar1 & 0x10000000) != 0) || ((uVar1 & 0x8000000) != 0)) {
    this->m_iDesiredState = 7;
  }
  iVar2 = this->m_iDesiredState;
  if ((this->m_iLastState == iVar2) || (((iVar2 != 2 && (iVar2 != 7)) && (iVar2 != 5)))) {
    piVar3 = *(int **)(unaff_EBX + 0x526360);
  }
  else {
    piVar3 = *(int **)(unaff_EBX + 0x526360);
    this->m_fNextTalk = *(float *)(*piVar3 + 0xc) - *(float *)(unaff_EBX + 0x3ac10c);
  }
  if (((this->m_fNextTalk <= *(float *)(*piVar3 + 0xc) &&
        *(float *)(*piVar3 + 0xc) != this->m_fNextTalk) && (this->m_iLastState != iVar2)) &&
     (this->m_iLastState = iVar2, (uint)this->m_iDesiredState < 8)) {
                    /* WARNING: Could not recover jumptable at 0x007208a9. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*(code *)(*(int *)(unaff_EBX + 0xa8 + iVar2 * 4) + unaff_EBX))();
    return;
  }
  return;
}


/* CHoverTurretTether::~CHoverTurretTether at 0072b790 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CHoverTurretTether(CHoverTurretTether * this, int __in_chrg) */

void __thiscall CHoverTurretTether::~CHoverTurretTether(CHoverTurretTether *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5a04e7);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CHoverTurretTether::~CHoverTurretTether at 0072b7d0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CHoverTurretTether(CHoverTurretTether * this, int __in_chrg) */

void __thiscall CHoverTurretTether::~CHoverTurretTether(CHoverTurretTether *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x5a04b0);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
  return;
}


/* DataMapInit<CHoverTurretTether> at 000ac380 */

datamap_t * DataMapInit<CHoverTurretTether>(CHoverTurretTether *param_1)

{
  int iVar1;
  int iVar2;
  ISaveRestoreOps *pIVar3;
  char *pcVar4;
  size_t sVar5;
  int iVar6;
  undefined4 *puVar7;
  int iVar8;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2c8].m_pPrev + unaff_EBX) ==
       '\0') && (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xda086c), iVar2 != 0)) {
    *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2c9].m_pPrev + unaff_EBX) =
         unaff_EBX + 0xa04e8c;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2ca].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2ca].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2ca].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2ca].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2cb].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2c9].m_pNext + unaff_EBX) =
         0x12;
    ___cxa_guard_release(unaff_EBX + 0xda086c);
    ___cxa_atexit(unaff_EBX + 0x9912ec,0,*(undefined4 *)(&DAT_00b9a7c0 + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_BodyTarget_00cb84f8 + unaff_EBX) =
       *(undefined4 *)(&DAT_00b9a7cc + unaff_EBX);
  if ((*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2c9].m_pEntity + unaff_EBX) ==
       '\0') && (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xda0874), iVar2 != 0)) {
    pIVar3 = GetPhysObjSaveRestoreOps(PIID_IPHYSICSSPRING);
    *(ISaveRestoreOps **)((int)&PTR_GetAIAddOn_00cb8dc0 + unaff_EBX) = pIVar3;
    pcVar4 = operator_new___(*(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2c9].
                                            m_pNext + unaff_EBX) + 10);
    _strcpy(pcVar4,*(char **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2c9].m_pPrev +
                             unaff_EBX));
    sVar5 = _strlen(pcVar4);
    builtin_strncpy(pcVar4 + sVar5,"PullThink",10);
    iVar1 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2ca].m_pNext + unaff_EBX);
    iVar2 = iVar1 + 1;
    iVar8 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2ca].m_SerialNumber +
                    unaff_EBX);
    if (iVar8 < iVar2) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)
                 ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2ca].m_pEntity + unaff_EBX),
                 iVar2 - iVar8);
    }
    iVar6 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2ca].m_pNext + unaff_EBX)
            + 1;
    *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2ca].m_pNext + unaff_EBX) = iVar6;
    iVar8 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2ca].m_pEntity + unaff_EBX
                    );
    *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2cb].m_pEntity + unaff_EBX) =
         iVar8;
    iVar6 = (iVar6 - iVar1) + -1;
    if (0 < iVar6) {
      _V_memmove((void *)(iVar8 + iVar2 * 4),(void *)(iVar8 + iVar1 * 4),iVar6 * 4);
      iVar8 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2ca].m_pEntity +
                      unaff_EBX);
    }
    puVar7 = (undefined4 *)(iVar1 * 4 + iVar8);
    if (puVar7 != (undefined4 *)0x0) {
      *puVar7 = pcVar4;
    }
    *(char **)((int)&PTR_IsBaseCombatWeapon_00cb8ef0 + unaff_EBX) = pcVar4;
    ___cxa_guard_release(unaff_EBX + 0xda0874);
  }
  *(undefined4 *)((int)&PTR_LocalEyeAngles_00cb84f0 + unaff_EBX) = 6;
  *(int *)((int)&PTR_EyeAngles_00cb84ec + unaff_EBX) = (int)&PTR_SetModelIndex_00cb8dac + unaff_EBX;
  return (datamap_t *)((int)&PTR_EyeAngles_00cb84ec + unaff_EBX);
}


/* CHoverTurretTether::GetDataDescMap at 007202d0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CHoverTurretTether * this) */

datamap_t * __thiscall CHoverTurretTether::GetDataDescMap(CHoverTurretTether *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x6445a8);
}


/* CHoverTurretTether::GetBaseMap at 007202e0 */

datamap_t * CHoverTurretTether::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x526878);
}


/* __tcf_0 at 00a3d680 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x40f596)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x40f58a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x40f596));
  }
  *(undefined4 *)(unaff_EBX + 0x40f596) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40f592)) {
    if (*(int *)(unaff_EBX + 0x40f58a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2094ce) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2094ce),*(int *)(unaff_EBX + 0x40f58a));
      *(undefined4 *)(unaff_EBX + 0x40f58a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f58e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40f58a);
  *(int *)(unaff_EBX + 0x40f59a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40f592)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2094ce) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2094ce),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40f58a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f58e) = 0;
  }
  return;
}


/* CHoverTurretTether::CHoverTurretTether at 00721580 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CHoverTurretTether(CHoverTurretTether * this) */

void __thiscall CHoverTurretTether::CHoverTurretTether(CHoverTurretTether *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::CBaseAnimating(&this->super_CBaseAnimating);
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5aa6f7);
  (this->m_hTurret).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hAttachPoint).super_CBaseHandle.m_Index = 0xffffffff;
  this->m_pSpring = (IPhysicsSpring *)0x0;
  (this->m_hRope).super_CBaseHandle.m_Index = 0xffffffff;
  return;
}


/* CHoverTurretTether::CHoverTurretTether at 007215e0 */

/* DWARF original prototype: void CHoverTurretTether(CHoverTurretTether * this, CHoverTurretTether *
   this) */

void __thiscall
CHoverTurretTether::CHoverTurretTether(CHoverTurretTether *this,CHoverTurretTether *this_1)

{
  CHoverTurretTether(this);
  return;
}


/* CEntityFactory<CHoverTurretTether>::Create at 0072ba00 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CHoverTurretTether> * this,
   char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CHoverTurretTether>::Create
          (CEntityFactory<CHoverTurretTether> *this,char *pClassName)

{
  CHoverTurretTether *this_00;
  
                    /* Unresolved local var: CHoverTurretTether * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x528);
  CHoverTurretTether::CHoverTurretTether(this_00);
  (*(this_00->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1d])(this_00,pClassName);
  return &(this_00->super_CBaseAnimating).super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CHoverTurretTether::ReleaseTether at 00720ef0 */

/* DWARF original prototype: void ReleaseTether(CHoverTurretTether * this) */

void __thiscall CHoverTurretTether::ReleaseTether(CHoverTurretTether *this)

{
  uint uVar1;
  CRopeKeyframe *this_00;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_pSpring != (IPhysicsSpring *)0x0) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x525cff) + 0x3c))
              ((int *)**(undefined4 **)(unaff_EBX + 0x525cff),this->m_pSpring);
    this->m_pSpring = (IPhysicsSpring *)0x0;
  }
  uVar1 = (this->m_hRope).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x525c6f),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (this_00 = *(CRopeKeyframe **)(iVar2 + 4), this_00 != (CRopeKeyframe *)0x0)) {
    CRopeKeyframe::DetachPoint(this_00,1);
    return;
  }
  return;
}


/* CNPC_HoverTurret::Event_Killed at 0072aa80 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void Event_Killed(CNPC_HoverTurret * this, CTakeDamageInfo * info) */

void __thiscall CNPC_HoverTurret::Event_Killed(CNPC_HoverTurret *this,CTakeDamageInfo *info)

{
  uint uVar1;
  IPhysicsMotionController *pIVar2;
  CRopeKeyframe *this_00;
  int iVar3;
  int iVar4;
  CBaseEntity *oldObj;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
  uVar1 = (this->m_hSmokeTrail).super_CBaseHandle.m_Index;
  if (((uVar1 == 0xffffffff) ||
      (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51c0de),
      *(uint *)(iVar3 + 8) != uVar1 >> 0x10)) || (*(int *)(iVar3 + 4) == 0)) {
    CreateSmokeTrail(this);
    this->m_fSparkTime =
         *(float *)(**(int **)(unaff_EBX + 0x51c0d6) + 0xc) + *(float *)(unaff_EBX + 0x3a1e86);
  }
  pIVar2 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.m_pMotionController;
  if (pIVar2 != (IPhysicsMotionController *)0x0) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51c16e) + 0x7c))
              ((int *)**(undefined4 **)(unaff_EBX + 0x51c16e),pIVar2);
    (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
    m_pMotionController = (IPhysicsMotionController *)0x0;
  }
  iVar3 = **(int **)(unaff_EBX + 0x51c0de);
  uVar1 = (this->m_hTether).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar4 = (uVar1 & 0xffff) * 0x10 + iVar3, *(uint *)(iVar4 + 8) == uVar1 >> 0x10)) &&
     (iVar4 = *(int *)(iVar4 + 4), iVar4 != 0)) {
                    /* Unresolved local var: CHoverTurretTether * pTether@[???] */
    if (*(int *)(iVar4 + 0x520) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51c16e) + 0x3c))
                ((int *)**(undefined4 **)(unaff_EBX + 0x51c16e),*(int *)(iVar4 + 0x520));
      *(undefined4 *)(iVar4 + 0x520) = 0;
      iVar3 = **(int **)(unaff_EBX + 0x51c0de);
    }
    uVar1 = *(uint *)(iVar4 + 0x524);
    if (((uVar1 != 0xffffffff) &&
        (iVar3 = (uVar1 & 0xffff) * 0x10 + iVar3, *(uint *)(iVar3 + 8) == uVar1 >> 0x10)) &&
       (this_00 = *(CRopeKeyframe **)(iVar3 + 4), this_00 != (CRopeKeyframe *)0x0)) {
      CRopeKeyframe::DetachPoint(this_00,1);
    }
    CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x386957),0.0,(float *)0x0);
    uVar1 = (this->m_hTether).super_CBaseHandle.m_Index;
    if ((uVar1 == 0xffffffff) ||
       (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51c0de),
       *(uint *)(iVar3 + 8) != uVar1 >> 0x10)) {
      oldObj = (CBaseEntity *)0x0;
    }
    else {
      oldObj = *(CBaseEntity **)(iVar3 + 4);
    }
    UTIL_Remove(oldObj);
    (this->m_hTether).super_CBaseHandle.m_Index = 0xffffffff;
  }
  if (this->m_bCarriedByPlayer == false) {
    StartDeathSequence(this);
  }
  SetFiringState(this,HOVER_TURRET_SHOT_DISABLED);
  if (((CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth> *)
      ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0xe8))->m_Value != 0
     ) {
                    /* Unresolved local var: CBaseEntity * pThis@[???] */
    (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x200))
              (this,(CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth> *)
                    ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                           super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                           super_CBaseAnimating + 0xe8));
    ((CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth> *)
    ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0xe8))->m_Value = 0;
  }
  COutputEvent::FireOutput
            (&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.m_OnDeath,(CBaseEntity *)this,
             (CBaseEntity *)this,0.0);
  return;
}


/* CHoverTurretTether::UpdateOnRemove at 00720e60 */

/* DWARF original prototype: void UpdateOnRemove(CHoverTurretTether * this) */

void __thiscall CHoverTurretTether::UpdateOnRemove(CHoverTurretTether *this)

{
  uint uVar1;
  CRopeKeyframe *this_00;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_pSpring != (IPhysicsSpring *)0x0) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x525d8f) + 0x3c))
              ((int *)**(undefined4 **)(unaff_EBX + 0x525d8f),this->m_pSpring);
    this->m_pSpring = (IPhysicsSpring *)0x0;
  }
  CBaseEntity::UpdateOnRemove((CBaseEntity *)this);
  uVar1 = (this->m_hRope).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x525cff),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (this_00 = *(CRopeKeyframe **)(iVar2 + 4), this_00 != (CRopeKeyframe *)0x0)) {
    CRopeKeyframe::DetachPoint(this_00,1);
    return;
  }
  return;
}


/* CHoverTurretTether::AttachEntities at 007202f0 */

/* DWARF original prototype: void AttachEntities(CHoverTurretTether * this, EHANDLE * hTurret,
   EHANDLE * hAttachPoint) */

void __thiscall
CHoverTurretTether::AttachEntities(CHoverTurretTether *this,EHANDLE *hTurret,EHANDLE *hAttachPoint)

{
  (this->m_hTurret).super_CBaseHandle.m_Index = (hTurret->super_CBaseHandle).m_Index;
  (this->m_hAttachPoint).super_CBaseHandle.m_Index = (hAttachPoint->super_CBaseHandle).m_Index;
  return;
}


/* CNPC_HoverTurret::Spawn at 00729e30 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "MoveCollide_t": Some values do not have unique names */
/* WARNING: Enum "MoveType_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::Spawn(CNPC_HoverTurret *this)

{
  int *piVar1;
  float fVar2;
  vec_t *pvVar3;
  float *pfVar4;
  uint uVar5;
  ulong uVar6;
  vec_t vVar7;
  char *szName;
  CBaseEntity *pCVar8;
  uint *puVar9;
  ulong *puVar10;
  undefined4 *puVar11;
  model_t *pmVar12;
  vec_t vVar13;
  CStudioHdr *pCVar14;
  vec_t vVar15;
  CBaseEntity *this_00;
  int unaff_EBX;
  NetworkVar_m_Collision *this_01;
  int iVar16;
  longdouble lVar17;
  longdouble lVar18;
  longdouble lVar19;
  float fVar20;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  
                    /* Unresolved local var: Vector bobAmount@[???] */
  ___i686_get_pc_thunk_bx();
  (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x68))
            (this);
  (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x6c))
            (this,unaff_EBX + 0x387457);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_CBaseCombatCharacter.m_eHull = HULL_TINY_CENTERED;
  CAI_BaseNPC::SetHullSizeNormal((CAI_BaseNPC *)this,false);
  this_01 = &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
             m_Collision;
  CCollisionProperty::SetSolid(&this_01->super_CCollisionProperty,SOLID_BBOX);
  CCollisionProperty::SetSolidFlags
            (&this_01->super_CCollisionProperty,
             (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
             m_Collision.super_CCollisionProperty.m_usSolidFlags.m_Value | 0x10);
  CBaseEntity::SetMoveType((CBaseEntity *)this,MOVETYPE_VPHYSICS,MOVECOLLIDE_DEFAULT);
  iVar16 = (int)*(float *)(*(int *)(unaff_EBX + 0x69e93e) + 0x2c);
  if (iVar16 != (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                m_iHealth.m_Value) {
                    /* Unresolved local var: CBaseEntity * pThis@[???] */
    (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x200))
              (this,&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                     super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                     super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                     super_CBaseEntity.m_iHealth);
    (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
    super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
    super_CBaseAnimating.super_CBaseEntity.m_iHealth.m_Value = iVar16;
  }
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.m_NPCState = NPC_STATE_NONE;
  iVar16 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.m_poseAim_Yaw;
  pCVar14 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
            super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.m_pStudioHdr;
  if (pCVar14 == (CStudioHdr *)0x0) {
    pmVar12 = CBaseEntity::GetModel((CBaseEntity *)this);
    if (pmVar12 == (model_t *)0x0) {
      pCVar14 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.m_pStudioHdr;
    }
    else {
      CBaseAnimating::LockStudioHdr((CBaseAnimating *)this);
      pCVar14 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.m_pStudioHdr;
    }
    if (pCVar14 != (CStudioHdr *)0x0) goto LAB_00729f21;
  }
  else {
LAB_00729f21:
    if (pCVar14->m_pStudioHdr == (studiohdr_t *)0x0) {
      pCVar14 = (CStudioHdr *)0x0;
    }
  }
  CBaseAnimating::SetPoseParameter((CBaseAnimating *)this,pCVar14,iVar16,0.0);
  iVar16 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.m_poseAim_Pitch;
  pCVar14 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
            super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.m_pStudioHdr;
  if (pCVar14 == (CStudioHdr *)0x0) {
    pmVar12 = CBaseEntity::GetModel((CBaseEntity *)this);
    if (pmVar12 != (model_t *)0x0) {
      CBaseAnimating::LockStudioHdr((CBaseAnimating *)this);
    }
    pCVar14 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
              super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.m_pStudioHdr;
    if (pCVar14 != (CStudioHdr *)0x0) goto LAB_00729f61;
  }
  else {
LAB_00729f61:
    if (pCVar14->m_pStudioHdr == (studiohdr_t *)0x0) {
      pCVar14 = (CStudioHdr *)0x0;
    }
  }
  CBaseAnimating::SetPoseParameter((CBaseAnimating *)this,pCVar14,iVar16,0.0);
  (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x684))
            (this,*(undefined4 *)(unaff_EBX + 0x69e97a));
  this->m_iDesiredState = 1;
  this->m_iLastState = 1;
  SetFiringState(this,HOVER_TURRET_FIND_TARGET);
  CAI_BaseNPC::SetNavType((CAI_BaseNPC *)this,NAV_FLY);
  piVar1 = &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
            super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
            m_iEFlags;
  *piVar1 = *piVar1 | 0x18000000;
  CBaseEntity::AddEffects((CBaseEntity *)this,0x10);
  CBaseCombatCharacter::SetBloodColor((CBaseCombatCharacter *)this,-1);
  pvVar3 = *(vec_t **)(unaff_EBX + 0x51cd3a);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vCurrentVelocity.x = *pvVar3;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vCurrentVelocity.y = pvVar3[1];
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vCurrentVelocity.z = pvVar3[2];
  (this->m_vForceVelocity).x = 0.0;
  (this->m_vForceVelocity).y = 0.0;
  (this->m_vForceVelocity).z = 0.0;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vCurrentBanking.x = 0.0;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vCurrentBanking.y = 0.0;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vCurrentBanking.z = 0.0;
  (this->m_vTargetBanking).x = 0.0;
  (this->m_vTargetBanking).y = 0.0;
  (this->m_vTargetBanking).z = 0.0;
  CAI_BaseNPC::CapabilitiesAdd((CAI_BaseNPC *)this,0x4080004);
  lVar17 = (longdouble)
           (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51cdaa) + 4))
                     ((int *)**(undefined4 **)(unaff_EBX + 0x51cdaa),0xbf800000,0x3f800000);
  lVar18 = (longdouble)
           (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51cdaa) + 4))
                     ((int *)**(undefined4 **)(unaff_EBX + 0x51cdaa),0xbf800000,0x3f800000);
  lVar19 = (longdouble)
           (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51cdaa) + 4))
                     ((int *)**(undefined4 **)(unaff_EBX + 0x51cdaa),0xbf800000,0x3f800000);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vNoiseMod.x = (float)lVar17;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vNoiseMod.y = (float)lVar18;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vNoiseMod.z = (float)lVar19;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_fHeadYaw = 0.0;
  (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x55c))
            (this);
  local_28 = 0;
  local_24 = 0;
  local_20 = 0x41200000;
  (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x240))
            (this,&local_28);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_CBaseCombatCharacter.m_flFieldOfView = 0.4;
  CBaseEntity::SetCollisionGroup((CBaseEntity *)this,0);
  iVar16 = CBaseAnimating::LookupAttachment((CBaseAnimating *)this,&UNK_0033d8ee + unaff_EBX);
  this->m_iMuzzleAttachment = iVar16;
  if (((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
       super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
  vVar13 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
           m_angAbsRotation.x;
  (this->m_vecGoalAngles).x = vVar13;
  vVar15 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
           m_angAbsRotation.y;
  (this->m_vecGoalAngles).y = vVar15;
  vVar7 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
          super_CBaseAnimating.super_CBaseEntity.m_angAbsRotation.z;
  (this->m_vecGoalAngles).z = vVar7;
  if (((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
       super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    vVar13 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
             m_angAbsRotation.x;
    vVar15 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
             m_angAbsRotation.y;
    vVar7 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
            super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
            m_angAbsRotation.z;
  }
  (this->m_vInitialLookAngles).x = vVar13;
  (this->m_vInitialLookAngles).y = vVar15;
  (this->m_vInitialLookAngles).z = vVar7;
  this->m_bCarriedByPlayer = false;
  this->m_flDeathTime = 0.0;
  if (((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
       super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
  fVar20 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
           m_vecAbsOrigin.x;
  pfVar4 = *(float **)(unaff_EBX + 0x51cd3a);
  if ((fVar20 == *pfVar4) && (!NAN(fVar20) && !NAN(*pfVar4))) {
    fVar2 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
            super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
            m_vecAbsOrigin.y;
    if ((fVar2 == pfVar4[1]) && (!NAN(fVar2) && !NAN(pfVar4[1]))) {
      fVar2 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
              super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
              m_vecAbsOrigin.z;
      if ((fVar2 == pfVar4[2]) && (!NAN(fVar2) && !NAN(pfVar4[2]))) {
        this->m_bInitialPositionSet = false;
        goto LAB_0072a25d;
      }
    }
  }
  this->m_bInitialPositionSet = true;
  if (((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
       super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    fVar20 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
             m_vecAbsOrigin.x;
  }
  (this->m_vForceMoveTarget).x = fVar20;
  (this->m_vForceMoveTarget).y =
       (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
       super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.y;
  (this->m_vForceMoveTarget).z =
       (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
       super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z;
LAB_0072a25d:
  szName = (this->m_strAttachName).pszValue;
  if (szName == (char *)0x0) {
    szName = (char *)(unaff_EBX + 0x324ee6);
  }
  pCVar8 = CGlobalEntityList::FindEntityByName
                     (*(CGlobalEntityList **)(unaff_EBX + 0x51cde6),(CBaseEntity *)0x0,szName,
                      (CBaseEntity *)0x0,(CBaseEntity *)0x0,(CBaseEntity *)0x0,
                      (IEntityFindFilter *)0x0);
  if (pCVar8 == (CBaseEntity *)0x0) {
    (this->m_hAttachTarget).super_CBaseHandle.m_Index = 0xffffffff;
    return;
  }
  puVar9 = (uint *)(*(pCVar8->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                     _vptr_IHandleEntity[3])(pCVar8);
  uVar5 = *puVar9;
  (this->m_hAttachTarget).super_CBaseHandle.m_Index = uVar5;
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???]
                       Unresolved local var: CEntInfo * pInfo@[???] */
  if ((((uVar5 != 0xffffffff) &&
       (iVar16 = (uVar5 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51cd2e),
       *(uint *)(iVar16 + 8) == uVar5 >> 0x10)) && (*(int *)(iVar16 + 4) != 0)) &&
     (((uVar5 = (this->m_hTether).super_CBaseHandle.m_Index, uVar5 == 0xffffffff ||
       (iVar16 = (uVar5 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51cd2e),
       *(uint *)(iVar16 + 8) != uVar5 >> 0x10)) || (*(int *)(iVar16 + 4) == 0)))) {
    pCVar8 = CreateEntityByName((char *)(unaff_EBX + 0x387421),-1,true);
    if (pCVar8 == (CBaseEntity *)0x0) {
      (this->m_hTether).super_CBaseHandle.m_Index = 0xffffffff;
    }
    else {
      puVar10 = (ulong *)(*(pCVar8->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                           _vptr_IHandleEntity[3])(pCVar8);
      (this->m_hTether).super_CBaseHandle.m_Index = *puVar10;
    }
    uVar5 = (this->m_hAttachTarget).super_CBaseHandle.m_Index;
    if ((uVar5 == 0xffffffff) ||
       (iVar16 = (uVar5 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51cd2e),
       *(uint *)(iVar16 + 8) != uVar5 >> 0x10)) {
      pCVar8 = (CBaseEntity *)0x0;
    }
    else {
      pCVar8 = *(CBaseEntity **)(iVar16 + 4);
    }
    if ((pCVar8->m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition(pCVar8);
    }
    uVar5 = (this->m_hTether).super_CBaseHandle.m_Index;
    if ((uVar5 == 0xffffffff) ||
       (iVar16 = (uVar5 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51cd2e),
       *(uint *)(iVar16 + 8) != uVar5 >> 0x10)) {
      this_00 = (CBaseEntity *)0x0;
    }
    else {
      this_00 = *(CBaseEntity **)(iVar16 + 4);
    }
    CBaseEntity::SetAbsOrigin(this_00,&pCVar8->m_vecAbsOrigin);
    uVar5 = (this->m_hTether).super_CBaseHandle.m_Index;
    if ((uVar5 == 0xffffffff) ||
       (iVar16 = (uVar5 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51cd2e),
       *(uint *)(iVar16 + 8) != uVar5 >> 0x10)) {
      iVar16 = 0;
    }
    else {
      iVar16 = *(int *)(iVar16 + 4);
    }
    uVar6 = (this->m_hAttachTarget).super_CBaseHandle.m_Index;
    puVar11 = (undefined4 *)
              (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                    super_CBaseCombatCharacter.super_CBaseFlex.
                                    super_CBaseAnimatingOverlay.super_CBaseAnimating + 0xc))(this);
    *(undefined4 *)(iVar16 + 0x51c) = *puVar11;
    *(ulong *)(iVar16 + 0x518) = uVar6;
    uVar5 = (this->m_hTether).super_CBaseHandle.m_Index;
    if ((uVar5 == 0xffffffff) ||
       (iVar16 = (uVar5 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51cd2e),
       *(uint *)(iVar16 + 8) != uVar5 >> 0x10)) {
      pCVar8 = (CBaseEntity *)0x0;
    }
    else {
      pCVar8 = *(CBaseEntity **)(iVar16 + 4);
    }
    DispatchSpawn(pCVar8,true);
    uVar5 = (this->m_hAttachTarget).super_CBaseHandle.m_Index;
    if ((uVar5 == 0xffffffff) ||
       (iVar16 = (uVar5 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51cd2e),
       *(uint *)(iVar16 + 8) != uVar5 >> 0x10)) {
      pCVar8 = (CBaseEntity *)0x0;
    }
    else {
      pCVar8 = *(CBaseEntity **)(iVar16 + 4);
    }
    if (((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
         super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
         super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
    if ((pCVar8->m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition(pCVar8);
    }
  }
  return;
}


/* CHoverTurretTether::ToggleThroughPortal at 00720610 */

/* DWARF original prototype: void ToggleThroughPortal(CHoverTurretTether * this, CPortal_Base2D *
   pEnteredPortal) */

void __thiscall
CHoverTurretTether::ToggleThroughPortal(CHoverTurretTether *this,CPortal_Base2D *pEnteredPortal)

{
  uint uVar1;
  int *piVar2;
  int iVar3;
  int unaff_EBX;
  CTakeDamageInfo *in_stack_ffffff7c;
  CBaseEntity *in_stack_ffffff80;
  CBaseEntity *in_stack_ffffff84;
  float in_stack_ffffff88;
  int in_stack_ffffff8c;
  code *iKillType_1;
  CTakeDamageInfo local_6c;
  
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->m_hTurret).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x526545),
      *(uint *)(iVar3 + 8) == uVar1 >> 0x10)) &&
     (piVar2 = *(int **)(iVar3 + 4), piVar2 != (int *)0x0)) {
                    /* Unresolved local var: CAI_BaseNPC * pTurret@[???] */
    iKillType_1 = *(code **)(*piVar2 + 0x118);
    CTakeDamageInfo::CTakeDamageInfo
              (&local_6c,(CBaseEntity *)this,(CBaseEntity *)this,200.0,0x40,0,in_stack_ffffff7c,
               in_stack_ffffff80,in_stack_ffffff84,in_stack_ffffff88,in_stack_ffffff8c,
               (int)iKillType_1);
    (*iKillType_1)(piVar2,&local_6c);
  }
  return;
}


/* CNPC_HoverTurret::NotifySystemEvent at 007215f0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void NotifySystemEvent(CNPC_HoverTurret * this, CBaseEntity * pNotify,
   notify_system_event_t eventType, notify_system_event_params_t * params) */

void __thiscall
CNPC_HoverTurret::NotifySystemEvent
          (CNPC_HoverTurret *this,CBaseEntity *pNotify,notify_system_event_t eventType,
          notify_system_event_params_t *params)

{
  uint uVar1;
  CBaseEntity *pInflictor;
  code *pcVar2;
  char cVar3;
  int iVar4;
  int iVar5;
  int unaff_EBX;
  CTakeDamageInfo *in_stack_ffffff7c;
  CBaseEntity *in_stack_ffffff80;
  CBaseEntity *in_stack_ffffff84;
  float in_stack_ffffff88;
  int in_stack_ffffff8c;
  int *iKillType_1;
  CTakeDamageInfo local_6c;
  
  ___i686_get_pc_thunk_bx();
  if (eventType == NOTIFY_EVENT_TELEPORT) {
    cVar3 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                  super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                  super_CBaseCombatCharacter.super_CBaseFlex.
                                  super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x114))(this);
    if (cVar3 != '\0') {
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
      iVar5 = **(int **)(unaff_EBX + 0x525565);
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
      uVar1 = (this->m_hTether).super_CBaseHandle.m_Index;
                    /* Unresolved local var: CHoverTurretTether * pHoverTurretTether@[???]
                       Unresolved local var: CPortal_Base2D * pEnteredPortal@[???] */
      if ((((uVar1 != 0xffffffff) &&
           (iVar4 = (uVar1 & 0xffff) * 0x10 + iVar5, *(uint *)(iVar4 + 8) == uVar1 >> 0x10)) &&
          (pInflictor = *(CBaseEntity **)(iVar4 + 4), pInflictor != (CBaseEntity *)0x0)) &&
         (pNotify != (CBaseEntity *)0x0)) {
        iVar4 = ___dynamic_cast(pNotify,*(undefined4 *)(unaff_EBX + 0x525589),
                                *(undefined4 *)(unaff_EBX + 0x5258bd),0);
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???]
                       Unresolved local var: CEntInfo * pInfo@[???] */
        if (((iVar4 != 0) &&
            (uVar1 = pInflictor[1].m_DamageModifiers.m_LastAlloc.index, uVar1 != 0xffffffff)) &&
           (iVar5 = (uVar1 & 0xffff) * 0x10 + iVar5, *(uint *)(iVar5 + 8) == uVar1 >> 0x10)) {
          iKillType_1 = *(int **)(iVar5 + 4);
          if (iKillType_1 != (int *)0x0) {
                    /* Unresolved local var: CAI_BaseNPC * pTurret@[???] */
            pcVar2 = *(code **)(*iKillType_1 + 0x118);
            CTakeDamageInfo::CTakeDamageInfo
                      (&local_6c,pInflictor,pInflictor,200.0,0x40,0,in_stack_ffffff7c,
                       in_stack_ffffff80,in_stack_ffffff84,in_stack_ffffff88,in_stack_ffffff8c,
                       (int)iKillType_1);
            (*pcVar2)(iKillType_1,&local_6c);
          }
        }
      }
    }
  }
  return;
}


/* CHoverTurretTether::Spawn at 00722250 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "MoveCollide_t": Some values do not have unique names */
/* WARNING: Enum "MoveType_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(CHoverTurretTether * this) */

void __thiscall CHoverTurretTether::Spawn(CHoverTurretTether *this)

{
  NetworkVar_m_Collision *this_00;
  uint uVar1;
  BASEPTR func;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::Precache(&this->super_CBaseAnimating);
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1b])(this,&UNK_0038f037 + unaff_EBX);
  CBaseEntity::AddEffects((CBaseEntity *)this,0x20);
  this_00 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Collision;
  CCollisionProperty::SetSolid(&this_00->super_CCollisionProperty,SOLID_VPHYSICS);
  CCollisionProperty::SetSolidFlags
            (&this_00->super_CCollisionProperty,
             (this->super_CBaseAnimating).super_CBaseEntity.m_Collision.super_CCollisionProperty.
             m_usSolidFlags.m_Value | 4);
  CBaseEntity::VPhysicsInitShadow((CBaseEntity *)this,false,false,(solid_t *)0x0);
  CBaseEntity::SetMoveType((CBaseEntity *)this,MOVETYPE_NONE,MOVECOLLIDE_DEFAULT);
  CBaseAnimating::Spawn(&this->super_CBaseAnimating);
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
  uVar1 = (this->m_hAttachPoint).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x52490e),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) && (*(int *)(iVar2 + 4) != 0)) {
    CBaseEntity::SetNextThink
              ((CBaseEntity *)this,*(float *)(**(int **)(unaff_EBX + 0x524906) + 0xc),(char *)0x0);
    func.__delta = 0;
    func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x73e);
    CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
    PullThink(this);
    return;
  }
  return;
}


/* __tcf_4 at 00a3d560 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x40f478),in_stack_00000008);
  return;
}


/* CHoverTurretTether::PullThink at 00721b20 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void PullThink(CHoverTurretTether * this) */

void __thiscall CHoverTurretTether::PullThink(CHoverTurretTether *this)

{
  float *pfVar1;
  float fVar2;
  float fVar3;
  int *piVar4;
  uint uVar5;
  CBaseEntity *this_00;
  IPhysicsObject *pIVar6;
  IPhysicsObject *pIVar7;
  bool bVar8;
  int iVar9;
  IPhysicsSpring *pIVar10;
  CRopeKeyframe *pCVar11;
  uint *puVar12;
  int unaff_EBX;
  CTakeDamageInfo *this_01;
  _func_int_varargs *p_Var13;
  CTraceFilterSimple *pCVar14;
  float fVar15;
  IHandleEntity *pIVar16;
  CTakeDamageInfo *in_stack_fffffdcc;
  CBaseEntity *in_stack_fffffdd0;
  CBaseEntity *in_stack_fffffdd4;
  float in_stack_fffffdd8;
  int in_stack_fffffddc;
  int in_stack_fffffde0;
  CGameTrace local_1f0;
  CTakeDamageInfo local_19c;
  float local_14c;
  float local_148;
  float local_144;
  float local_13c;
  float local_138;
  float local_134;
  undefined4 local_12c;
  undefined4 local_128;
  undefined4 local_124;
  undefined4 local_11c;
  undefined4 local_118;
  undefined4 local_114;
  undefined4 local_10c;
  undefined1 local_108;
  undefined1 local_107;
  CTakeDamageInfo local_f8;
  CTraceFilterSimple local_a8;
  float local_98;
  float local_94;
  float local_90;
  vec_t local_8c;
  vec_t local_88;
  vec_t local_84;
  undefined1 local_80;
  undefined1 local_7f;
  float local_7c;
  undefined4 uStack_78;
  undefined4 uStack_74;
  undefined4 uStack_70;
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
  float local_28;
  float local_24;
  float local_20;
  
                    /* Unresolved local var: CAI_BaseNPC * pTurret@[???] */
  ___i686_get_pc_thunk_bx();
  piVar4 = *(int **)(unaff_EBX + 0x52503b);
  uVar5 = (this->m_hTurret).super_CBaseHandle.m_Index;
  if (((uVar5 == 0xffffffff) ||
      (iVar9 = (uVar5 & 0xffff) * 0x10 + *piVar4, *(uint *)(iVar9 + 8) != uVar5 >> 0x10)) ||
     (this_00 = *(CBaseEntity **)(iVar9 + 4), this_00 == (CBaseEntity *)0x0)) {
    return;
  }
  if (this->m_pSpring == (IPhysicsSpring *)0x0) {
                    /* Unresolved local var: Vector vecDistance@[???]
                       Unresolved local var: IPhysicsObject * pPhysObject@[???]
                       Unresolved local var: IPhysicsObject * pAttachPhysObject@[???]
                       Unresolved local var: springparams_t spring@[???]
                       Unresolved local var: Vector up@[???] */
    if (((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
    if ((this_00->m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition(this_00);
    }
                    /* Unresolved local var: Vector res@[???] */
    local_28 = (this_00->m_vecAbsOrigin).x -
               (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.x;
    local_24 = (this_00->m_vecAbsOrigin).y -
               (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.y;
    local_20 = (this_00->m_vecAbsOrigin).z -
               (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.z;
                    /* Unresolved local var: __m128 root@[???] */
    local_7c = SQRT(local_28 * local_28 + local_24 * local_24 + local_20 * local_20);
    uStack_78 = 0;
    uStack_74 = 0;
    uStack_70 = 0;
    in_stack_fffffdd4 = *(CBaseEntity **)(unaff_EBX + 0x3ab003);
    pIVar16 = (IHandleEntity *)(local_7c - (float)in_stack_fffffdd4);
    this->m_flNaturalDistance = (float)pIVar16;
    pIVar6 = this_00->m_pPhysicsObject;
    pIVar7 = (this->super_CBaseAnimating).super_CBaseEntity.m_pPhysicsObject;
    pCVar14 = &local_a8;
    for (iVar9 = 0xb; iVar9 != 0; iVar9 = iVar9 + -1) {
      (pCVar14->super_CTraceFilter).super_ITraceFilter._vptr_ITraceFilter =
           (_func_int_varargs **)0x0;
      pCVar14 = (CTraceFilterSimple *)&pCVar14->m_pPassEnt;
    }
    local_a8.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter =
         (ITraceFilter)(_func_int_varargs **)0x461c4000;
    local_a8.m_collisionGroup = 0x41800000;
    local_a8.m_pExtraShouldHitCheckFunction = (ShouldHitFunc_t)0x0;
    local_a8.m_pPassEnt = pIVar16;
    (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
      super_IHandleEntity._vptr_IHandleEntity[0x8e])(this,0,0,&local_4c);
                    /* Unresolved local var: Vector res@[???] */
    local_64 = local_4c * (float)in_stack_fffffdd4;
    local_60 = local_48 * (float)in_stack_fffffdd4;
    local_5c = (float)in_stack_fffffdd4 * local_44;
    local_34 = local_64;
    local_30 = local_60;
    local_2c = local_5c;
    if ((this_00->m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition(this_00);
    }
                    /* Unresolved local var: Vector res@[???] */
    local_98 = (this_00->m_vecAbsOrigin).x + local_34;
    local_94 = (this_00->m_vecAbsOrigin).y + local_30;
    local_90 = (this_00->m_vecAbsOrigin).z + local_2c;
    local_40 = local_98;
    local_3c = local_94;
    local_38 = local_90;
    if (((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
    local_8c = (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.x;
    local_88 = (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.y;
    local_84 = (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.z;
    local_80 = 0;
    local_7f = 1;
    pIVar10 = (IPhysicsSpring *)
              (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5250cb) + 0x38))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x5250cb),pIVar6,pIVar7,&local_a8);
    this->m_pSpring = pIVar10;
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
    uVar5 = (this->m_hRope).super_CBaseHandle.m_Index;
    if (((uVar5 == 0xffffffff) ||
        (iVar9 = (uVar5 & 0xffff) * 0x10 + *piVar4, *(uint *)(iVar9 + 8) != uVar5 >> 0x10)) ||
       (*(int *)(iVar9 + 4) == 0)) {
      in_stack_fffffdd0 = (CBaseEntity *)(&UNK_0033bfa7 + unaff_EBX);
      in_stack_fffffdcc = (CTakeDamageInfo *)((int)&vgui::Panel::GetMessageMap::s_pMap.cputype + 1);
      pCVar11 = CRopeKeyframe::Create
                          ((CBaseEntity *)this,this_00,0,0,1,(char *)(unaff_EBX + 0x34f1a3),5,
                           (char *)in_stack_fffffdd0);
      if (pCVar11 == (CRopeKeyframe *)0x0) {
        (this->m_hRope).super_CBaseHandle.m_Index = 0xffffffff;
        pCVar11 = (CRopeKeyframe *)0x0;
      }
      else {
        puVar12 = (uint *)(*(pCVar11->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.
                            super_IHandleEntity._vptr_IHandleEntity[3])(pCVar11);
        uVar5 = *puVar12;
        (this->m_hRope).super_CBaseHandle.m_Index = uVar5;
        if ((uVar5 == 0xffffffff) ||
           (iVar9 = (uVar5 & 0xffff) * 0x10 + *piVar4, *(uint *)(iVar9 + 8) != uVar5 >> 0x10)) {
          pCVar11 = (CRopeKeyframe *)0x0;
        }
        else {
          pCVar11 = *(CRopeKeyframe **)(iVar9 + 4);
        }
      }
      CRopeKeyframe::EnableWind(pCVar11,true);
    }
  }
  (*this->m_pSpring->_vptr_IPhysicsSpring[2])(this->m_pSpring,&local_4c,&local_58);
  local_4c = local_4c - local_58;
  local_48 = local_48 - local_54;
  local_44 = local_44 - local_50;
                    /* Unresolved local var: __m128 root@[???] */
  local_7c = SQRT(local_4c * local_4c + local_48 * local_48 + local_44 * local_44);
  uStack_78 = 0;
  uStack_74 = 0;
  uStack_70 = 0;
  fVar15 = local_7c - this->m_flNaturalDistance;
  pfVar1 = (float *)(*(int *)(unaff_EBX + 0x72aecb) + 0x2c);
  if (*pfVar1 <= fVar15 && fVar15 != *pfVar1) {
    p_Var13 = (this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[0x46];
    this_01 = &local_f8;
  }
  else {
    if (((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
    fVar15 = (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.z;
    fVar2 = (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.y;
    fVar3 = (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.x;
    local_10c = 0;
    if ((this_00->m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition(this_00);
    }
    local_13c = (this_00->m_vecAbsOrigin).x - fVar3;
    local_138 = (this_00->m_vecAbsOrigin).y - fVar2;
    local_134 = (this_00->m_vecAbsOrigin).z - fVar15;
    local_107 = local_13c * local_13c + local_138 * local_138 + local_134 * local_134 != 0.0;
    local_114 = 0;
    local_118 = 0;
    local_11c = 0;
    local_10c = 0;
    local_108 = 1;
    local_124 = 0;
    local_128 = 0;
    local_12c = 0;
    local_14c = fVar3;
    local_148 = fVar2;
    local_144 = fVar15;
    CTraceFilterSimple::CTraceFilterSimple(&local_a8,(IHandleEntity *)0x0,0,(ShouldHitFunc_t)0x0);
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52509f) + 0x14))
              ((int *)**(undefined4 **)(unaff_EBX + 0x52509f),&local_14c,0x400b,&local_a8,&local_1f0
              );
    if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x52509b) + 0x1c) + 0x30) != 0) {
      in_stack_fffffdcc = (CTakeDamageInfo *)0xbf800000;
      DebugDrawLine((Vector *)&local_1f0,&local_1f0.super_CBaseTrace.endpos,0xff,0,0,true,-1.0);
    }
    bVar8 = CGameTrace::DidHitWorld(&local_1f0);
    if (!bVar8) {
      CBaseEntity::SetNextThink
                ((CBaseEntity *)this,*(float *)(**(int **)(unaff_EBX + 0x525033) + 0xc),(char *)0x0)
      ;
      return;
    }
    p_Var13 = (this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[0x46];
    this_01 = &local_19c;
  }
  CTakeDamageInfo::CTakeDamageInfo
            (this_01,(CBaseEntity *)this,(CBaseEntity *)this,200.0,0x40,0,in_stack_fffffdcc,
             in_stack_fffffdd0,in_stack_fffffdd4,in_stack_fffffdd8,in_stack_fffffddc,
             in_stack_fffffde0);
  (*p_Var13)(this_00,this_01);
  return;
}


/* CNPC_HoverTurret::Classify at 0072b5c0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: Class_T Classify(CNPC_HoverTurret * this) */

Class_T __thiscall CNPC_HoverTurret::Classify(CNPC_HoverTurret *this)

{
  return CLASS_COMBINE;
}


/* CNPC_HoverTurret::BodyTarget at 0072b5d0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: Vector BodyTarget(CNPC_HoverTurret * this, Vector * posSrc, bool
   bNoisy) */

Vector * __thiscall
CNPC_HoverTurret::BodyTarget
          (Vector *__return_storage_ptr__,CNPC_HoverTurret *this,Vector *posSrc,bool bNoisy)

{
  void *pvVar1;
  
  pvVar1 = (void *)(**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>)
                                         .super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                         super_CBaseCombatCharacter.super_CBaseFlex.
                                         super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x274))
                             (this);
  _memmove(__return_storage_ptr__,pvVar1,0xc);
  return __return_storage_ptr__;
}


/* CNPC_HoverTurret::GetHeadTurnRate at 0072b610 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: float GetHeadTurnRate(CNPC_HoverTurret * this) */

float __thiscall CNPC_HoverTurret::GetHeadTurnRate(CNPC_HoverTurret *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(float *)(extraout_ECX + 0x3a1768);
}


/* CNPC_HoverTurret::ObjectCaps at 0072b9e0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: int ObjectCaps(CNPC_HoverTurret * this) */

int __thiscall CNPC_HoverTurret::ObjectCaps(CNPC_HoverTurret *this)

{
  uint uVar1;
  
  uVar1 = CBaseEntity::ObjectCaps((CBaseEntity *)this);
  return uVar1 | 0x201;
}


/* CNPC_HoverTurret::Use at 0072b620 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void Use(CNPC_HoverTurret * this, CBaseEntity * pActivator, CBaseEntity
   * pCaller, $_170 useType, float value) */

void __thiscall
CNPC_HoverTurret::Use
          (CNPC_HoverTurret *this,CBaseEntity *pActivator,CBaseEntity *pCaller,__170 useType,
          float value)

{
  int iVar1;
  
                    /* Unresolved local var: CBasePlayer * pPlayer@[???] */
  if (pActivator != (CBaseEntity *)0x0) {
    iVar1 = (*(pActivator->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[0x55])(pActivator);
    if ((char)iVar1 != '\0') {
                    /* WARNING: Could not recover jumptable at 0x0072b66e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (*(pActivator->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
        _vptr_IHandleEntity[0x1b7])();
      return;
    }
  }
  return;
}


/* CNPC_HoverTurret::GetClassScheduleIdSpace at 0072b670 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: CAI_ClassScheduleIdSpace * GetClassScheduleIdSpace(CNPC_HoverTurret *
   this) */

CAI_ClassScheduleIdSpace * __thiscall
CNPC_HoverTurret::GetClassScheduleIdSpace(CNPC_HoverTurret *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (CAI_ClassScheduleIdSpace *)(extraout_ECX + 0x69d188);
}


/* CNPC_HoverTurret::GetSchedulingErrorName at 0072b680 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: char * GetSchedulingErrorName(CNPC_HoverTurret * this) */

char * __thiscall CNPC_HoverTurret::GetSchedulingErrorName(CNPC_HoverTurret *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(char **)(extraout_ECX + 0x639234);
}


/* CNPC_HoverTurret::AllowedToIgnite at 0072b690 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: bool AllowedToIgnite(CNPC_HoverTurret * this) */

bool __thiscall CNPC_HoverTurret::AllowedToIgnite(CNPC_HoverTurret *this)

{
  return true;
}


/* DataMapInit<CNPC_HoverTurret> at 000ac5b0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */

datamap_t * DataMapInit<CNPC_HoverTurret>(CNPC_HoverTurret *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2a2].m_pPrev + unaff_EBX) ==
      '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xda060c);
    if (iVar2 != 0) {
      *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2a3].m_pPrev + unaff_EBX) =
           unaff_EBX + 0xa04c6f;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2a4].m_pEntity + unaff_EBX) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2a4].m_SerialNumber + unaff_EBX) = 0;
      *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2a4].m_pPrev + unaff_EBX)
           = 0;
      *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2a4].m_pNext + unaff_EBX)
           = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2a5].m_pEntity + unaff_EBX) = 0;
      *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2a3].m_pNext + unaff_EBX)
           = 0x10;
      ___cxa_guard_release(unaff_EBX + 0xda060c);
      ___cxa_atexit(unaff_EBX + 0x990fdc,0,*(undefined4 *)(&DAT_00b9a590 + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_GetBaseEntity_00cb82e0 + unaff_EBX) =
       *(undefined4 *)(&DAT_00b9b0e0 + unaff_EBX);
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2a3].m_pEntity + unaff_EBX) ==
      '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xda0614);
    if (iVar2 != 0) {
      uVar1 = **(undefined4 **)(&DAT_00b9a5b0 + unaff_EBX);
      *(undefined4 *)((int)&PTR_ShouldCollide_00cb8a10 + unaff_EBX) = uVar1;
      *(undefined4 *)((int)&PTR_KeyValue_00cb8a50 + unaff_EBX) = uVar1;
      *(undefined4 *)((int)&PTR_MoveDone_00cb8a90 + unaff_EBX) = uVar1;
      *(undefined4 *)((int)&PTR_CanBeHitByMeleeAttack_00cb8ad0 + unaff_EBX) = uVar1;
      ___cxa_guard_release(unaff_EBX + 0xda0614);
    }
  }
  *(undefined4 *)((int)&PTR_GetCollideable_00cb82d8 + unaff_EBX) = 0x20;
  *(int *)((int)&PTR_GetRefEHandle_00cb82d4 + unaff_EBX) =
       (int)&PTR_PostConstructor_00cb833c + unaff_EBX;
  return (datamap_t *)((int)&PTR_GetRefEHandle_00cb82d4 + unaff_EBX);
}


/* CNPC_HoverTurret::GetDataDescMap at 00720310 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: datamap_t * GetDataDescMap(CNPC_HoverTurret * this) */

datamap_t * __thiscall CNPC_HoverTurret::GetDataDescMap(CNPC_HoverTurret *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x644580);
}


/* CNPC_HoverTurret::GetBaseMap at 00720320 */

datamap_t * CNPC_HoverTurret::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x52737c);
}


/* __tcf_1 at 00a3d5a0 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x40f646)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x40f63a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x40f646));
  }
  *(undefined4 *)(unaff_EBX + 0x40f646) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40f642)) {
    if (*(int *)(unaff_EBX + 0x40f63a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2095ae) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2095ae),*(int *)(unaff_EBX + 0x40f63a));
      *(undefined4 *)(unaff_EBX + 0x40f63a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f63e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40f63a);
  *(int *)(unaff_EBX + 0x40f64a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40f642)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2095ae) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2095ae),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40f63a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f63e) = 0;
  }
  return;
}


/* ServerClassInit<DT_NPC_HoverTurret::ignored> at 000ac150 */

int ServerClassInit<DT_NPC_HoverTurret::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2d2].m_pEntity + unaff_EBX) ==
      '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xda0904);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  (&UNK_00002d3c +
                  (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                  (char *)(unaff_EBX + 0x9a9ad0),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2d8].m_pNext +
                        unaff_EBX),(char *)(unaff_EBX + 0x9abe80),0,
                        (SendTable *)**(undefined4 **)(&DAT_00b9b48c + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00b9aa8c + unaff_EBX),0x80);
      SendPropInt((SendProp_conflict *)
                  ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2de].m_pEntity + unaff_EBX),
                  (char *)(unaff_EBX + 0x9ffe57),0xf0c,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropInt((SendProp_conflict *)
                  ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2e3].m_SerialNumber +
                  unaff_EBX),(char *)(unaff_EBX + 0x9ff7a4),0xf10,4,-1,0,
                  (SendVarProxyFn_conflict)0x0,0x80);
      ___cxa_guard_release(unaff_EBX + 0xda0904);
      ___cxa_atexit(unaff_EBX + 0x99137c,0,*(undefined4 *)(&DAT_00b9a9f0 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)((int)DataMapInit<CAI_Motor>::dataDesc[1].flatOffset + unaff_EBX + -0x28),
             (SendProp_conflict *)
             ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2d8].m_pNext + unaff_EBX),3,
             *(char **)(&DAT_00c1ff08 + unaff_EBX));
  return 1;
}


/* CNPC_HoverTurret::GetServerClass at 00720330 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: ServerClass * GetServerClass(CNPC_HoverTurret * this) */

ServerClass * __thiscall CNPC_HoverTurret::GetServerClass(CNPC_HoverTurret *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x72c704);
}


/* CNPC_HoverTurret::YouForgotToImplementOrDeclareServerClass at 00720340 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CNPC_HoverTurret * this)
    */

int __thiscall CNPC_HoverTurret::YouForgotToImplementOrDeclareServerClass(CNPC_HoverTurret *this)

{
  return 0;
}


/* __tcf_5 at 00a3d540 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x38b288),in_stack_00000008);
  return;
}


/* __tcf_2 at 00a3d4e0 */

void __tcf_2(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f68b))(unaff_EBX + 0x40f68b);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f637))(unaff_EBX + 0x40f637);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f5e3))(unaff_EBX + 0x40f5e3);
                    /* WARNING: Could not recover jumptable at 0x00a3d538. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f58f))();
  return;
}


/* CNPC_HoverTurret::CNPC_HoverTurret at 00726160 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void CNPC_HoverTurret(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::CNPC_HoverTurret(CNPC_HoverTurret *this)

{
  uint *puVar1;
  uint uVar2;
  int iVar3;
  ushort uVar4;
  int *piVar5;
  CBaseEdict *pCVar6;
  ushort *puVar7;
  IChangeInfoAccessor *pIVar8;
  uint uVar9;
  int unaff_EBX;
  uint uVar10;
  
  ___i686_get_pc_thunk_bx();
  CAI_BasePhysicsFlyingBot::CAI_BasePhysicsFlyingBot((CAI_BasePhysicsFlyingBot *)this);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).m_OnAlyxStartedInteraction.
  super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).m_OnAlyxStartedInteraction.
  super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index = 0xffffffff;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).m_OnAlyxStartedInteraction.
  super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).m_OnAlyxFinishedInteraction.
  super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).m_OnAlyxFinishedInteraction.
  super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index = 0xffffffff;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).m_OnAlyxFinishedInteraction.
  super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  *(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating = unaff_EBX + 0x5a4fda;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_CAI_DefMovementSink.super_IAI_MovementSink._vptr_IAI_MovementSink =
       (_func_int_varargs **)(unaff_EBX + 0x5a59de);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_IAI_BehaviorBridge._vptr_IAI_BehaviorBridge =
       (_func_int_varargs **)(unaff_EBX + 0x5a5a0a);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_IMotionEvent._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x5a5ab6);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_INPCInteractive.
  _vptr_INPCInteractive = (_func_int_varargs **)(unaff_EBX + 0x5a5ac2);
  (this->super_CDefaultPlayerPickupVPhysics).super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics
       = (_func_int_varargs **)(unaff_EBX + 0x5a5ade);
  CAI_BaseNPC::LoadSchedules();
  piVar5 = *(int **)(unaff_EBX + 0x520f06);
  if (*(int *)(unaff_EBX + 0x66184a) != *piVar5) {
    InitCustomSchedules();
    *(undefined1 *)(unaff_EBX + 0x661846) = 1;
    *(int *)(unaff_EBX + 0x66184a) = *piVar5;
  }
  (this->m_hAttachTarget).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hTether).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_OnPhysGunPickup).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnPhysGunPickup).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->m_OnPhysGunPickup).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_OnPhysGunDrop).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnPhysGunDrop).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->m_OnPhysGunDrop).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_hSmokeTrail).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hEyeGlow).super_CBaseHandle.m_Index = 0xffffffff;
  this->m_bUseCarryAngles = true;
  if ((this->m_iLaserState).m_Value != 1) {
    if (*(bool *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x60) ==
        false) {
      pCVar6 = *(CBaseEdict **)
                ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                       super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                       super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x24);
      if ((pCVar6 != (CBaseEdict *)0x0) && ((pCVar6->m_fStateFlags & 0x100U) == 0)) {
        pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 1;
        pIVar8 = CBaseEdict::GetChangeAccessor(pCVar6);
        piVar5 = *(int **)(&DAT_005209fa + unaff_EBX);
        puVar7 = (ushort *)*piVar5;
        if (pIVar8->m_iChangeInfoSerialNumber == *puVar7) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar10 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar4 = puVar7[uVar10 * 0x14 + 0x14];
          if (uVar4 == 0) {
LAB_007264da:
            puVar7[(uint)uVar4 + uVar10 * 0x14 + 1] = 0xf0c;
            puVar7[uVar10 * 0x14 + 0x14] = uVar4 + 1;
          }
          else if (puVar7[uVar10 * 0x14 + 1] != 0xf0c) {
            uVar9 = 0;
            do {
              uVar2 = uVar9 + 1;
              uVar9 = uVar2 & 0xffff;
              if ((ushort)uVar2 == uVar4) {
                if (uVar4 != 0x13) goto LAB_007264da;
                goto LAB_0072644b;
              }
            } while (puVar7[uVar10 * 0x14 + uVar9 + 1] != 0xf0c);
          }
        }
        else if (puVar7[0x7d1] == 100) {
LAB_0072644b:
          pIVar8->m_iChangeInfoSerialNumber = 0;
          pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar8->m_iChangeInfo = puVar7[0x7d1];
          *(short *)(*piVar5 + 0xfa2) = *(short *)(*piVar5 + 0xfa2) + 1;
          pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar3 = *piVar5 + (uint)pIVar8->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar3 + 2) = 0xf0c;
          *(undefined2 *)(iVar3 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = (uint *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                              super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay
                              .super_CBaseAnimating + 100);
      *puVar1 = *puVar1 | 1;
    }
    (this->m_iLaserState).m_Value = 1;
  }
  if ((this->m_sLaserHaloSprite).m_Value != 0) {
    if (*(bool *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x60) ==
        false) {
      pCVar6 = *(CBaseEdict **)
                ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                       super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                       super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x24);
      if ((pCVar6 != (CBaseEdict *)0x0) && ((pCVar6->m_fStateFlags & 0x100U) == 0)) {
        pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 1;
        pIVar8 = CBaseEdict::GetChangeAccessor(pCVar6);
        piVar5 = *(int **)(&DAT_005209fa + unaff_EBX);
        puVar7 = (ushort *)*piVar5;
        if (pIVar8->m_iChangeInfoSerialNumber == *puVar7) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar10 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar4 = puVar7[uVar10 * 0x14 + 0x14];
          if (uVar4 == 0) {
LAB_0072655a:
            puVar7[(uint)uVar4 + uVar10 * 0x14 + 1] = 0xf10;
            puVar7[uVar10 * 0x14 + 0x14] = uVar4 + 1;
          }
          else if (puVar7[uVar10 * 0x14 + 1] != 0xf10) {
            uVar9 = 0;
            do {
              uVar2 = uVar9 + 1;
              uVar9 = uVar2 & 0xffff;
              if ((ushort)uVar2 == uVar4) {
                if (uVar4 != 0x13) goto LAB_0072655a;
                goto LAB_00726462;
              }
            } while (puVar7[uVar10 * 0x14 + uVar9 + 1] != 0xf10);
          }
        }
        else if (puVar7[0x7d1] == 100) {
LAB_00726462:
          pIVar8->m_iChangeInfoSerialNumber = 0;
          pCVar6->m_fStateFlags = pCVar6->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar8->m_iChangeInfo = puVar7[0x7d1];
          *(short *)(*piVar5 + 0xfa2) = *(short *)(*piVar5 + 0xfa2) + 1;
          pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar3 = *piVar5 + (uint)pIVar8->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar3 + 2) = 0xf10;
          *(undefined2 *)(iVar3 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = (uint *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                              super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay
                              .super_CBaseAnimating + 100);
      *puVar1 = *puVar1 | 1;
    }
    (this->m_sLaserHaloSprite).m_Value = 0;
  }
  (this->m_hAttachTarget).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hTether).super_CBaseHandle.m_Index = 0xffffffff;
  return;
}


/* CNPC_HoverTurret::CNPC_HoverTurret at 00726620 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void CNPC_HoverTurret(CNPC_HoverTurret * this, CNPC_HoverTurret * this)
    */

void __thiscall CNPC_HoverTurret::CNPC_HoverTurret(CNPC_HoverTurret *this,CNPC_HoverTurret *this_1)

{
  CNPC_HoverTurret(this);
  return;
}


/* CEntityFactory<CNPC_HoverTurret>::Create at 0072ba50 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CNPC_HoverTurret> * this,
   char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CNPC_HoverTurret>::Create(CEntityFactory<CNPC_HoverTurret> *this,char *pClassName)

{
  CNPC_HoverTurret *this_00;
  
                    /* Unresolved local var: CNPC_HoverTurret * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0xfec);
  CNPC_HoverTurret::CNPC_HoverTurret(this_00);
  (**(code **)(*(int *)&(this_00->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x74))
            (this_00,pClassName);
  return &((CServerNetworkProperty *)
          ((int)&(this_00->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                 super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                 super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x18))->
          super_IServerNetworkable;
}


/* CNPC_HoverTurret::~CNPC_HoverTurret at 007258b0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void ~CNPC_HoverTurret(CNPC_HoverTurret * this, int __in_chrg) */

void __thiscall CNPC_HoverTurret::~CNPC_HoverTurret(CNPC_HoverTurret *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  *(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating = unaff_EBX + 0x5a588a;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_CAI_DefMovementSink.super_IAI_MovementSink._vptr_IAI_MovementSink =
       (_func_int_varargs **)(unaff_EBX + 0x5a628e);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_IAI_BehaviorBridge._vptr_IAI_BehaviorBridge =
       (_func_int_varargs **)(unaff_EBX + 0x5a62ba);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_IMotionEvent._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x5a6366);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_INPCInteractive.
  _vptr_INPCInteractive = (_func_int_varargs **)(unaff_EBX + 0x5a6372);
  (this->super_CDefaultPlayerPickupVPhysics).super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics
       = (_func_int_varargs **)(unaff_EBX + 0x5a638e);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnPhysGunDrop).super_CBaseEntityOutput,in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnPhysGunPickup).super_CBaseEntityOutput,in_stack_ffffffd8);
  *(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating = unaff_EBX + 0x5a684a;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_CAI_DefMovementSink.super_IAI_MovementSink._vptr_IAI_MovementSink =
       (_func_int_varargs **)(unaff_EBX + 0x5a723e);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_IAI_BehaviorBridge._vptr_IAI_BehaviorBridge =
       (_func_int_varargs **)(unaff_EBX + 0x5a726a);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_IMotionEvent._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x5a7316);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_INPCInteractive.
  _vptr_INPCInteractive = (_func_int_varargs **)(unaff_EBX + 0x5a7322);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
              m_OnAlyxFinishedInteraction.super_CBaseEntityOutput,in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).m_OnAlyxStartedInteraction
              .super_CBaseEntityOutput,in_stack_ffffffd8);
  CAI_BasePhysicsFlyingBot::~CAI_BasePhysicsFlyingBot((CAI_BasePhysicsFlyingBot *)this,__in_chrg);
  return;
}


/* CNPC_HoverTurret::~CNPC_HoverTurret at 00725a70 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void ~CNPC_HoverTurret(CNPC_HoverTurret * this, int __in_chrg) */

void __thiscall CNPC_HoverTurret::~CNPC_HoverTurret(CNPC_HoverTurret *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  *(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating = unaff_EBX + 0x5a56ca;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_CAI_DefMovementSink.super_IAI_MovementSink._vptr_IAI_MovementSink =
       (_func_int_varargs **)(unaff_EBX + 0x5a60ce);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_IAI_BehaviorBridge._vptr_IAI_BehaviorBridge =
       (_func_int_varargs **)(unaff_EBX + 0x5a60fa);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_IMotionEvent._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x5a61a6);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_INPCInteractive.
  _vptr_INPCInteractive = (_func_int_varargs **)(unaff_EBX + 0x5a61b2);
  (this->super_CDefaultPlayerPickupVPhysics).super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics
       = (_func_int_varargs **)(unaff_EBX + 0x5a61ce);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnPhysGunDrop).super_CBaseEntityOutput,in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnPhysGunPickup).super_CBaseEntityOutput,in_stack_ffffffd8);
  *(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating = unaff_EBX + 0x5a668a;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_CAI_DefMovementSink.super_IAI_MovementSink._vptr_IAI_MovementSink =
       (_func_int_varargs **)(unaff_EBX + 0x5a707e);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_IAI_BehaviorBridge._vptr_IAI_BehaviorBridge =
       (_func_int_varargs **)(unaff_EBX + 0x5a70aa);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_IMotionEvent._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x5a7156);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_INPCInteractive.
  _vptr_INPCInteractive = (_func_int_varargs **)(unaff_EBX + 0x5a7162);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
              m_OnAlyxFinishedInteraction.super_CBaseEntityOutput,in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).m_OnAlyxStartedInteraction
              .super_CBaseEntityOutput,in_stack_ffffffd8);
  CAI_BasePhysicsFlyingBot::~CAI_BasePhysicsFlyingBot
            ((CAI_BasePhysicsFlyingBot *)this,in_stack_ffffffd8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CNPC_HoverTurret::~CNPC_HoverTurret at 00725c30 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void ~CNPC_HoverTurret(CNPC_HoverTurret * this, int __in_chrg) */

void __thiscall CNPC_HoverTurret::~CNPC_HoverTurret(CNPC_HoverTurret *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  *(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating = unaff_EBX + 0x5a550a;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_CAI_DefMovementSink.super_IAI_MovementSink._vptr_IAI_MovementSink =
       (_func_int_varargs **)(unaff_EBX + 0x5a5f0e);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_IAI_BehaviorBridge._vptr_IAI_BehaviorBridge =
       (_func_int_varargs **)(&UNK_005a5f3a + unaff_EBX);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_IMotionEvent._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x5a5fe6);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_INPCInteractive.
  _vptr_INPCInteractive = (_func_int_varargs **)(unaff_EBX + 0x5a5ff2);
  (this->super_CDefaultPlayerPickupVPhysics).super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics
       = (_func_int_varargs **)(unaff_EBX + 0x5a600e);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnPhysGunDrop).super_CBaseEntityOutput,in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnPhysGunPickup).super_CBaseEntityOutput,in_stack_ffffffd8);
  *(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating = unaff_EBX + 0x5a64ca;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_CAI_DefMovementSink.super_IAI_MovementSink._vptr_IAI_MovementSink =
       (_func_int_varargs **)(unaff_EBX + 0x5a6ebe);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_CAI_BaseNPC.super_IAI_BehaviorBridge._vptr_IAI_BehaviorBridge =
       (_func_int_varargs **)(unaff_EBX + 0x5a6eea);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  super_IMotionEvent._vptr_IMotionEvent = (_func_int_varargs **)(unaff_EBX + 0x5a6f96);
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_INPCInteractive.
  _vptr_INPCInteractive = (_func_int_varargs **)(unaff_EBX + 0x5a6fa2);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
              m_OnAlyxFinishedInteraction.super_CBaseEntityOutput,in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).m_OnAlyxStartedInteraction
              .super_CBaseEntityOutput,in_stack_ffffffd8);
  CAI_BasePhysicsFlyingBot::~CAI_BasePhysicsFlyingBot((CAI_BasePhysicsFlyingBot *)this,__in_chrg);
  return;
}


/* CNPC_HoverTurret::GatherConditions at 00720e50 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void GatherConditions(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::GatherConditions(CNPC_HoverTurret *this)

{
  CAI_BaseNPC::GatherConditions((CAI_BaseNPC *)this);
  return;
}


/* CNPC_HoverTurret::PrescheduleThink at 0072b4d0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void PrescheduleThink(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::PrescheduleThink(CNPC_HoverTurret *this)

{
  int iVar1;
  int iVar2;
  hoverTurretAttackState_e hVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((*(byte *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                       super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                       super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x133) & 8
      ) == 0) {
    if (0 < ((CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth> *)
            ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                   super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                   super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0xe8))->
            m_Value) {
      hVar3 = this->m_iFiringState;
      if (hVar3 == HOVER_TURRET_AIM_TARGET) {
        AimThink(this);
      }
      else {
        if (hVar3 == HOVER_TURRET_SHOT_DISABLED) {
          return;
        }
        if (hVar3 == HOVER_TURRET_FIND_TARGET) {
          FindTargetThink(this);
        }
      }
      goto LAB_0072b519;
    }
    if ((this->m_flDeathTime == *(float *)(unaff_EBX + 0x3a168f)) ||
       (*(float *)(**(int **)(unaff_EBX + 0x51b683) + 0xc) <= this->m_flDeathTime))
    goto LAB_0072b519;
  }
  else {
    iVar2 = ((CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth> *)
            ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                   super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                   super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0xe8))->
            m_Value;
    iVar1 = iVar2 + -1;
    if (iVar2 != iVar1) {
                    /* Unresolved local var: CBaseEntity * pThis@[???] */
      (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                            super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                            super_CBaseAnimating + 0x200))
                (this,(CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth> *)
                      ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                             super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                             super_CBaseAnimating + 0xe8));
      ((CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth> *)
      ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0xe8))->m_Value =
           iVar1;
    }
    if (0 < iVar1) goto LAB_0072b519;
  }
  Explode(this);
LAB_0072b519:
  TalkThink(this);
  CAI_BaseNPC::PrescheduleThink((CAI_BaseNPC *)this);
  return;
}


/* CNPC_HoverTurret::SetFiringState at 00726630 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void SetFiringState(CNPC_HoverTurret * this, hoverTurretAttackState_e
   state) */

void __thiscall
CNPC_HoverTurret::SetFiringState(CNPC_HoverTurret *this,hoverTurretAttackState_e state)

{
  undefined1 *puVar1;
  int iVar2;
  ushort uVar3;
  CBaseEdict *pCVar4;
  ushort *puVar5;
  int *piVar6;
  _func_int_varargs **pp_Var7;
  char cVar8;
  int iVar9;
  CSprite *pCVar10;
  uint *puVar11;
  IChangeInfoAccessor *pIVar12;
  undefined4 *puVar13;
  uint uVar14;
  uint uVar15;
  int unaff_EBX;
  CBaseEntity *this_00;
  bool bVar16;
  uint local_32c;
  uint local_328;
  uint local_324;
  uint local_320;
  uint local_31c;
  uint local_318;
  uint local_314;
  uint local_310;
  uint local_30c;
  uint local_308;
  uint local_304;
  uint local_300;
  uint local_2fc;
  uint local_2f8;
  uint local_2f4;
  uint local_2f0;
  uint local_2e4;
  uint local_2e0;
  uint local_2dc;
  uint local_2d8;
  uint local_2d4;
  uint local_2d0;
  uint local_2cc;
  uint local_2c8;
  uint local_2c4;
  uint local_2c0;
  Vector local_28 [2];
  
                    /* Unresolved local var: bool bNewState@[???] */
  ___i686_get_pc_thunk_bx();
  cVar8 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                super_CBaseCombatCharacter.super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x114))(this);
  if ((cVar8 != '\0') || (state == HOVER_TURRET_SHOT_DISABLED)) {
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
    uVar14 = (this->m_hEyeGlow).super_CBaseHandle.m_Index;
    if ((uVar14 == 0xffffffff) ||
       ((iVar9 = (uVar14 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x52052b),
        *(uint *)(iVar9 + 8) != uVar14 >> 0x10 || (*(int *)(iVar9 + 4) == 0)))) {
      pCVar10 = CSprite::SpriteCreate
                          ((char *)(unaff_EBX + 0x384407),
                           (Vector *)
                           &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                            super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                            super_CBaseAnimating.super_CBaseEntity.m_vecOrigin,false);
      if (pCVar10 == (CSprite *)0x0) {
        (this->m_hEyeGlow).super_CBaseHandle.m_Index = 0xffffffff;
        return;
      }
      puVar11 = (uint *)(*(pCVar10->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.
                          super_IHandleEntity._vptr_IHandleEntity[3])(pCVar10);
      uVar14 = *puVar11;
      (this->m_hEyeGlow).super_CBaseHandle.m_Index = uVar14;
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
      if (uVar14 == 0xffffffff) {
        return;
      }
      iVar9 = (uVar14 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x52052b);
      if (*(uint *)(iVar9 + 8) != uVar14 >> 0x10) {
        return;
      }
      pCVar10 = *(CSprite **)(iVar9 + 4);
      if (pCVar10 == (CSprite *)0x0) {
        return;
      }
      if ((pCVar10->super_CBaseEntity).m_nRenderMode.m_Value != '\t') {
        if ((pCVar10->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
          pCVar4 = &((pCVar10->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
          if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
            pIVar12 = CBaseEdict::GetChangeAccessor(pCVar4);
            puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x520527);
            if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
              uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
              uVar3 = puVar5[uVar14 * 0x14 + 0x14];
              if (uVar3 == 0) {
LAB_0072863e:
                puVar5[(uint)uVar3 + uVar14 * 0x14 + 1] = 0x105;
                puVar5[uVar14 * 0x14 + 0x14] = uVar3 + 1;
              }
              else if (puVar5[uVar14 * 0x14 + 1] != 0x105) {
                local_32c = 0;
                do {
                  uVar15 = local_32c + 1;
                  local_32c = uVar15 & 0xffff;
                  if ((ushort)uVar15 == uVar3) {
                    if (uVar3 != 0x13) goto LAB_0072863e;
                    pIVar12->m_iChangeInfoSerialNumber = 0;
                    pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                    break;
                  }
                } while (puVar5[uVar14 * 0x14 + local_32c + 1] != 0x105);
              }
            }
            else if (puVar5[0x7d1] == 100) {
              pIVar12->m_iChangeInfoSerialNumber = 0;
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
            }
            else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
              pIVar12->m_iChangeInfo = puVar5[0x7d1];
              piVar6 = *(int **)(unaff_EBX + 0x520527);
              *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
              pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
              iVar9 = *piVar6 + (uint)pIVar12->m_iChangeInfo * 0x28;
              *(undefined2 *)(iVar9 + 2) = 0x105;
              *(undefined2 *)(iVar9 + 0x28) = 1;
            }
          }
        }
        else {
          puVar1 = &(pCVar10->super_CBaseEntity).m_Network.field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (pCVar10->super_CBaseEntity).m_nRenderMode.m_Value = '\t';
      }
      if ((pCVar10->super_CBaseEntity).m_clrRender.
          super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r != 0xff) {
        if ((pCVar10->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
          pCVar4 = &((pCVar10->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
          if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
            pIVar12 = CBaseEdict::GetChangeAccessor(pCVar4);
            puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x520527);
            if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
              uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
              uVar3 = puVar5[uVar14 * 0x14 + 0x14];
              if (uVar3 == 0) {
LAB_0072846e:
                puVar5[(uint)uVar3 + uVar14 * 0x14 + 1] = 0x108;
                puVar5[uVar14 * 0x14 + 0x14] = uVar3 + 1;
              }
              else if (puVar5[uVar14 * 0x14 + 1] != 0x108) {
                local_328 = 0;
                do {
                  uVar15 = local_328 + 1;
                  local_328 = uVar15 & 0xffff;
                  if ((ushort)uVar15 == uVar3) {
                    if (uVar3 != 0x13) goto LAB_0072846e;
                    pIVar12->m_iChangeInfoSerialNumber = 0;
                    pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                    break;
                  }
                } while (puVar5[uVar14 * 0x14 + local_328 + 1] != 0x108);
              }
            }
            else if (puVar5[0x7d1] == 100) {
              pIVar12->m_iChangeInfoSerialNumber = 0;
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
            }
            else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
              pIVar12->m_iChangeInfo = puVar5[0x7d1];
              piVar6 = *(int **)(unaff_EBX + 0x520527);
              *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
              pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
              iVar9 = *piVar6 + (uint)pIVar12->m_iChangeInfo * 0x28;
              *(undefined2 *)(iVar9 + 2) = 0x108;
              *(undefined2 *)(iVar9 + 0x28) = 1;
            }
          }
        }
        else {
          puVar1 = &(pCVar10->super_CBaseEntity).m_Network.field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (pCVar10->super_CBaseEntity).m_clrRender.
        super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.r = 0xff;
      }
      if ((pCVar10->super_CBaseEntity).m_clrRender.
          super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g != '\0') {
        if ((pCVar10->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
          pCVar4 = &((pCVar10->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
          if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
            pIVar12 = CBaseEdict::GetChangeAccessor(pCVar4);
            puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x520527);
            if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
              uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
              uVar3 = puVar5[uVar14 * 0x14 + 0x14];
              if (uVar3 == 0) {
LAB_00728890:
                puVar5[(uint)uVar3 + uVar14 * 0x14 + 1] = 0x108;
                puVar5[uVar14 * 0x14 + 0x14] = uVar3 + 1;
              }
              else if (puVar5[uVar14 * 0x14 + 1] != 0x108) {
                local_324 = 0;
                do {
                  uVar15 = local_324 + 1;
                  local_324 = uVar15 & 0xffff;
                  if ((ushort)uVar15 == uVar3) {
                    if (uVar3 != 0x13) goto LAB_00728890;
                    pIVar12->m_iChangeInfoSerialNumber = 0;
                    pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                    break;
                  }
                } while (puVar5[uVar14 * 0x14 + local_324 + 1] != 0x108);
              }
            }
            else if (puVar5[0x7d1] == 100) {
              pIVar12->m_iChangeInfoSerialNumber = 0;
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
            }
            else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
              pIVar12->m_iChangeInfo = puVar5[0x7d1];
              piVar6 = *(int **)(unaff_EBX + 0x520527);
              *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
              pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
              iVar9 = *piVar6 + (uint)pIVar12->m_iChangeInfo * 0x28;
              *(undefined2 *)(iVar9 + 2) = 0x108;
              *(undefined2 *)(iVar9 + 0x28) = 1;
            }
          }
        }
        else {
          puVar1 = &(pCVar10->super_CBaseEntity).m_Network.field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (pCVar10->super_CBaseEntity).m_clrRender.
        super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.g = '\0';
      }
      if ((pCVar10->super_CBaseEntity).m_clrRender.
          super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b != '\0') {
        if ((pCVar10->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
          pCVar4 = &((pCVar10->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
          if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
            pIVar12 = CBaseEdict::GetChangeAccessor(pCVar4);
            puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x520527);
            if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
              uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
              uVar3 = puVar5[uVar14 * 0x14 + 0x14];
              if (uVar3 == 0) {
LAB_007287a0:
                puVar5[(uint)uVar3 + uVar14 * 0x14 + 1] = 0x108;
                puVar5[uVar14 * 0x14 + 0x14] = uVar3 + 1;
              }
              else if (puVar5[uVar14 * 0x14 + 1] != 0x108) {
                local_320 = 0;
                do {
                  uVar15 = local_320 + 1;
                  local_320 = uVar15 & 0xffff;
                  if ((ushort)uVar15 == uVar3) {
                    if (uVar3 != 0x13) goto LAB_007287a0;
                    pIVar12->m_iChangeInfoSerialNumber = 0;
                    pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                    break;
                  }
                } while (puVar5[uVar14 * 0x14 + local_320 + 1] != 0x108);
              }
            }
            else if (puVar5[0x7d1] == 100) {
              pIVar12->m_iChangeInfoSerialNumber = 0;
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
            }
            else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
              pIVar12->m_iChangeInfo = puVar5[0x7d1];
              piVar6 = *(int **)(unaff_EBX + 0x520527);
              *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
              pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
              iVar9 = *piVar6 + (uint)pIVar12->m_iChangeInfo * 0x28;
              *(undefined2 *)(iVar9 + 2) = 0x108;
              *(undefined2 *)(iVar9 + 0x28) = 1;
            }
          }
        }
        else {
          puVar1 = &(pCVar10->super_CBaseEntity).m_Network.field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (pCVar10->super_CBaseEntity).m_clrRender.
        super_CNetworkVarBase<color32_s,CBaseEntity::NetworkVar_m_clrRender>.m_Value.b = '\0';
      }
      CSprite::SetBrightness(pCVar10,0x80,0.0);
      if ((pCVar10->super_CBaseEntity).m_nRenderFX.m_Value != '\x0e') {
        if ((pCVar10->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
          pCVar4 = &((pCVar10->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
          if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
            pIVar12 = CBaseEdict::GetChangeAccessor(pCVar4);
            puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x520527);
            if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
              uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
              uVar3 = puVar5[uVar14 * 0x14 + 0x14];
              if (uVar3 == 0) {
LAB_00728386:
                puVar5[(uint)uVar3 + uVar14 * 0x14 + 1] = 0x104;
                puVar5[uVar14 * 0x14 + 0x14] = uVar3 + 1;
              }
              else if (puVar5[uVar14 * 0x14 + 1] != 0x104) {
                local_31c = 0;
                do {
                  uVar15 = local_31c + 1;
                  local_31c = uVar15 & 0xffff;
                  if ((ushort)uVar15 == uVar3) {
                    if (uVar3 != 0x13) goto LAB_00728386;
                    pIVar12->m_iChangeInfoSerialNumber = 0;
                    pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                    break;
                  }
                } while (puVar5[uVar14 * 0x14 + local_31c + 1] != 0x104);
              }
            }
            else if (puVar5[0x7d1] == 100) {
              pIVar12->m_iChangeInfoSerialNumber = 0;
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
            }
            else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
              pIVar12->m_iChangeInfo = puVar5[0x7d1];
              piVar6 = *(int **)(unaff_EBX + 0x520527);
              *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
              pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
              iVar9 = *piVar6 + (uint)pIVar12->m_iChangeInfo * 0x28;
              *(undefined2 *)(iVar9 + 2) = 0x104;
              *(undefined2 *)(iVar9 + 0x28) = 1;
            }
          }
        }
        else {
          puVar1 = &(pCVar10->super_CBaseEntity).m_Network.field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (pCVar10->super_CBaseEntity).m_nRenderFX.m_Value = '\x0e';
      }
      uVar14 = (this->m_hEyeGlow).super_CBaseHandle.m_Index;
      if ((uVar14 == 0xffffffff) ||
         (iVar9 = (uVar14 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x52052b),
         *(uint *)(iVar9 + 8) != uVar14 >> 0x10)) {
        this_00 = (CBaseEntity *)0x0;
      }
      else {
        this_00 = *(CBaseEntity **)(iVar9 + 4);
      }
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???]
                       Unresolved local var: CEntInfo * pInfo@[???] */
      pp_Var7 = this_00[1].super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
                _vptr_IHandleEntity;
      if (((pp_Var7 == (_func_int_varargs **)0xffffffff) ||
          (iVar9 = ((uint)pp_Var7 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x52052b),
          *(uint *)(iVar9 + 8) != (uint)pp_Var7 >> 0x10)) ||
         (this != *(CNPC_HoverTurret **)(iVar9 + 4))) {
        if ((this_00->m_Network).m_TimerEvent.m_bRegistered == false) {
          pCVar4 = &((this_00->m_Network).m_pPev)->super_CBaseEdict;
          if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
            pIVar12 = CBaseEdict::GetChangeAccessor(pCVar4);
            puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x520527);
            if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
              uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
              uVar3 = puVar5[uVar14 * 0x14 + 0x14];
              if (uVar3 == 0) {
LAB_0072829e:
                puVar5[(uint)uVar3 + uVar14 * 0x14 + 1] = 0x3c0;
                puVar5[uVar14 * 0x14 + 0x14] = uVar3 + 1;
              }
              else if (puVar5[uVar14 * 0x14 + 1] != 0x3c0) {
                local_318 = 0;
                do {
                  uVar15 = local_318 + 1;
                  local_318 = uVar15 & 0xffff;
                  if ((ushort)uVar15 == uVar3) {
                    if (uVar3 != 0x13) goto LAB_0072829e;
                    pIVar12->m_iChangeInfoSerialNumber = 0;
                    pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                    break;
                  }
                } while (puVar5[uVar14 * 0x14 + local_318 + 1] != 0x3c0);
              }
            }
            else if (puVar5[0x7d1] == 100) {
              pIVar12->m_iChangeInfoSerialNumber = 0;
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
            }
            else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
              pIVar12->m_iChangeInfo = puVar5[0x7d1];
              piVar6 = *(int **)(unaff_EBX + 0x520527);
              *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
              pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
              iVar9 = *piVar6 + (uint)pIVar12->m_iChangeInfo * 0x28;
              *(undefined2 *)(iVar9 + 2) = 0x3c0;
              *(undefined2 *)(iVar9 + 0x28) = 1;
            }
          }
        }
        else {
          puVar1 = &(this_00->m_Network).field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        puVar13 = (undefined4 *)
                  (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                        super_CBaseCombatCharacter.super_CBaseFlex.
                                        super_CBaseAnimatingOverlay.super_CBaseAnimating + 0xc))
                            (this);
        this_00[1].super_IServerEntity.super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
             = (_func_int_varargs **)*puVar13;
      }
      if (this_00[1].m_iObjectCapsCache.m_Value != 1) {
        if ((this_00->m_Network).m_TimerEvent.m_bRegistered == false) {
          pCVar4 = &((this_00->m_Network).m_pPev)->super_CBaseEdict;
          if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
            pIVar12 = CBaseEdict::GetChangeAccessor(pCVar4);
            puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x520527);
            if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
              uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
              uVar3 = puVar5[uVar14 * 0x14 + 0x14];
              if (uVar3 == 0) {
LAB_00728556:
                puVar5[(uint)uVar3 + uVar14 * 0x14 + 1] = 0x3c4;
                puVar5[uVar14 * 0x14 + 0x14] = uVar3 + 1;
              }
              else if (puVar5[uVar14 * 0x14 + 1] != 0x3c4) {
                local_314 = 0;
                do {
                  uVar15 = local_314 + 1;
                  local_314 = uVar15 & 0xffff;
                  if ((ushort)uVar15 == uVar3) {
                    if (uVar3 != 0x13) goto LAB_00728556;
                    pIVar12->m_iChangeInfoSerialNumber = 0;
                    pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                    break;
                  }
                } while (puVar5[uVar14 * 0x14 + local_314 + 1] != 0x3c4);
              }
            }
            else if (puVar5[0x7d1] == 100) {
              pIVar12->m_iChangeInfoSerialNumber = 0;
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
            }
            else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
              pIVar12->m_iChangeInfo = puVar5[0x7d1];
              piVar6 = *(int **)(unaff_EBX + 0x520527);
              *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
              pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
              iVar9 = *piVar6 + (uint)pIVar12->m_iChangeInfo * 0x28;
              *(undefined2 *)(iVar9 + 2) = 0x3c4;
              *(undefined2 *)(iVar9 + 0x28) = 1;
            }
          }
        }
        else {
          puVar1 = &(this_00->m_Network).field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        this_00[1].m_iObjectCapsCache.m_Value = 1;
      }
      CBaseEntity::FollowEntity(this_00,(CBaseEntity *)this,true);
    }
    bVar16 = this->m_iFiringState != state;
    if (state != HOVER_TURRET_SHOT_DISABLED) {
      if (state == HOVER_TURRET_HIT_WITH_PHYSICS) {
        this->m_iFiringState = HOVER_TURRET_FIND_TARGET;
        this->m_iDesiredState = 7;
        (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                              super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay
                              .super_CBaseAnimating + 0x684))
                  (this,*(undefined4 *)(&DAT_006a217f + unaff_EBX));
        if ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
            super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.m_nSkin.m_Value != 2) {
          if ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
              super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
              m_Network.m_TimerEvent.m_bRegistered == false) {
            pCVar4 = &((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                       super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                       super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                       super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
            if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
              pIVar12 = CBaseEdict::GetChangeAccessor(pCVar4);
              puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x520527);
              if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
                uVar3 = puVar5[uVar14 * 0x14 + 0x14];
                if (uVar3 == 0) {
LAB_007290f9:
                  puVar5[(uint)uVar3 + uVar14 * 0x14 + 1] = 0x3e0;
                  puVar5[uVar14 * 0x14 + 0x14] = uVar3 + 1;
                }
                else if (puVar5[uVar14 * 0x14 + 1] != 0x3e0) {
                  local_2f8 = 0;
                  do {
                    uVar15 = local_2f8 + 1;
                    local_2f8 = uVar15 & 0xffff;
                    if ((ushort)uVar15 == uVar3) {
                      if (uVar3 != 0x13) goto LAB_007290f9;
                      goto LAB_00729124;
                    }
                  } while (puVar5[uVar14 * 0x14 + local_2f8 + 1] != 0x3e0);
                }
              }
              else if (puVar5[0x7d1] == 100) {
LAB_00729124:
                pIVar12->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
              }
              else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                pIVar12->m_iChangeInfo = puVar5[0x7d1];
                *(short *)(**(int **)(unaff_EBX + 0x520527) + 0xfa2) =
                     *(short *)(**(int **)(unaff_EBX + 0x520527) + 0xfa2) + 1;
                piVar6 = *(int **)(unaff_EBX + 0x520527);
                pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
                iVar9 = *piVar6 + (uint)pIVar12->m_iChangeInfo * 0x28;
                *(undefined2 *)(iVar9 + 2) = 0x3e0;
                *(undefined2 *)(iVar9 + 0x28) = 1;
              }
            }
          }
          else {
            puVar1 = &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                      super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                      super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                      super_CBaseEntity.m_Network.field_0x4c;
            *(uint *)puVar1 = *(uint *)puVar1 | 1;
          }
          (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
          super_CBaseAnimating.m_nSkin.m_Value = 2;
        }
        if ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
            super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
            m_Network.m_TimerEvent.m_bRegistered == false) {
          pCVar4 = &((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                     super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                     super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                     super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
          if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
            pIVar12 = CBaseEdict::GetChangeAccessor(pCVar4);
            puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x520527);
            if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
              uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
              uVar3 = puVar5[uVar14 * 0x14 + 0x14];
              if (uVar3 == 0) {
LAB_00728a56:
                puVar5[(uint)uVar3 + uVar14 * 0x14 + 1] = 0xf0c;
                puVar5[uVar14 * 0x14 + 0x14] = uVar3 + 1;
              }
              else if (puVar5[uVar14 * 0x14 + 1] != 0xf0c) {
                local_2f4 = 0;
                do {
                  uVar15 = local_2f4 + 1;
                  local_2f4 = uVar15 & 0xffff;
                  if ((ushort)uVar15 == uVar3) {
                    if (uVar3 != 0x13) goto LAB_00728a56;
                    pIVar12->m_iChangeInfoSerialNumber = 0;
                    pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                    break;
                  }
                } while (puVar5[uVar14 * 0x14 + local_2f4 + 1] != 0xf0c);
              }
            }
            else if (puVar5[0x7d1] == 100) {
              pIVar12->m_iChangeInfoSerialNumber = 0;
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
            }
            else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
              pIVar12->m_iChangeInfo = puVar5[0x7d1];
              *(short *)(**(int **)(unaff_EBX + 0x520527) + 0xfa2) =
                   *(short *)(**(int **)(unaff_EBX + 0x520527) + 0xfa2) + 1;
              piVar6 = *(int **)(unaff_EBX + 0x520527);
              pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
              iVar9 = *piVar6 + (uint)pIVar12->m_iChangeInfo * 0x28;
              *(undefined2 *)(iVar9 + 2) = 0xf0c;
              *(undefined2 *)(iVar9 + 0x28) = 1;
            }
          }
        }
        else {
          puVar1 = &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                    super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                    super_CBaseEntity.m_Network.field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (this->m_iLaserState).m_Value = 0;
        uVar14 = (this->m_hEyeGlow).super_CBaseHandle.m_Index;
        if ((uVar14 == 0xffffffff) ||
           (iVar9 = (uVar14 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x52052b),
           *(uint *)(iVar9 + 8) != uVar14 >> 0x10)) {
          iVar9 = 0;
        }
        else {
          iVar9 = *(int *)(iVar9 + 4);
        }
        if (*(char *)(iVar9 + 0x108) != -1) {
          if (*(char *)(iVar9 + 0x60) == '\0') {
            pCVar4 = *(CBaseEdict **)(iVar9 + 0x24);
            if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
              pIVar12 = CBaseEdict::GetChangeAccessor(pCVar4);
              puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x520527);
              if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
                uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
                uVar3 = puVar5[uVar14 * 0x14 + 0x14];
                if (uVar3 == 0) {
LAB_00728f2a:
                  puVar5[(uint)uVar3 + uVar14 * 0x14 + 1] = 0x108;
                  puVar5[uVar14 * 0x14 + 0x14] = uVar3 + 1;
                }
                else if (puVar5[uVar14 * 0x14 + 1] != 0x108) {
                  local_2f0 = 0;
                  do {
                    uVar15 = local_2f0 + 1;
                    local_2f0 = uVar15 & 0xffff;
                    if ((ushort)uVar15 == uVar3) {
                      if (uVar3 != 0x13) goto LAB_00728f2a;
                      pIVar12->m_iChangeInfoSerialNumber = 0;
                      pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                      break;
                    }
                  } while (puVar5[uVar14 * 0x14 + local_2f0 + 1] != 0x108);
                }
              }
              else if (puVar5[0x7d1] == 100) {
                pIVar12->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
              }
              else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                pIVar12->m_iChangeInfo = puVar5[0x7d1];
                piVar6 = *(int **)(unaff_EBX + 0x520527);
                *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
                pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
                iVar2 = *piVar6 + (uint)pIVar12->m_iChangeInfo * 0x28;
                *(undefined2 *)(iVar2 + 2) = 0x108;
                *(undefined2 *)(iVar2 + 0x28) = 1;
              }
            }
          }
          else {
            *(uint *)(iVar9 + 100) = *(uint *)(iVar9 + 100) | 1;
          }
          *(undefined1 *)(iVar9 + 0x108) = 0xff;
        }
        if (*(char *)(iVar9 + 0x109) != '\0') {
          if (*(char *)(iVar9 + 0x60) == '\0') {
            pCVar4 = *(CBaseEdict **)(iVar9 + 0x24);
            if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
              pIVar12 = CBaseEdict::GetChangeAccessor(pCVar4);
              piVar6 = *(int **)(unaff_EBX + 0x520527);
              puVar5 = (ushort *)*piVar6;
              if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
                uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
                uVar3 = puVar5[uVar14 * 0x14 + 0x14];
                if (uVar3 == 0) {
LAB_00729610:
                  puVar5[(uint)uVar3 + uVar14 * 0x14 + 1] = 0x108;
                  puVar5[uVar14 * 0x14 + 0x14] = uVar3 + 1;
                }
                else if (puVar5[uVar14 * 0x14 + 1] != 0x108) {
                  local_2c4 = 0;
                  do {
                    uVar15 = local_2c4 + 1;
                    local_2c4 = uVar15 & 0xffff;
                    if ((ushort)uVar15 == uVar3) {
                      if (uVar3 != 0x13) goto LAB_00729610;
                      pIVar12->m_iChangeInfoSerialNumber = 0;
                      pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                      break;
                    }
                  } while (puVar5[uVar14 * 0x14 + local_2c4 + 1] != 0x108);
                }
              }
              else if (puVar5[0x7d1] == 100) {
                pIVar12->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
              }
              else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                pIVar12->m_iChangeInfo = puVar5[0x7d1];
                *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
                pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
                iVar2 = *piVar6 + (uint)pIVar12->m_iChangeInfo * 0x28;
                *(undefined2 *)(iVar2 + 2) = 0x108;
                *(undefined2 *)(iVar2 + 0x28) = 1;
              }
            }
          }
          else {
            *(uint *)(iVar9 + 100) = *(uint *)(iVar9 + 100) | 1;
          }
          *(undefined1 *)(iVar9 + 0x109) = 0;
        }
        if (*(char *)(iVar9 + 0x10a) != '\0') {
          if (*(char *)(iVar9 + 0x60) == '\0') {
            pCVar4 = *(CBaseEdict **)(iVar9 + 0x24);
            if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
              pIVar12 = CBaseEdict::GetChangeAccessor(pCVar4);
              piVar6 = *(int **)(unaff_EBX + 0x520527);
              puVar5 = (ushort *)*piVar6;
              if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
                uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
                uVar3 = puVar5[uVar14 * 0x14 + 0x14];
                if (uVar3 == 0) {
LAB_00729222:
                  puVar5[(uint)uVar3 + uVar14 * 0x14 + 1] = 0x108;
                  puVar5[uVar14 * 0x14 + 0x14] = uVar3 + 1;
                }
                else if (puVar5[uVar14 * 0x14 + 1] != 0x108) {
                  local_2c0 = 0;
                  do {
                    uVar15 = local_2c0 + 1;
                    local_2c0 = uVar15 & 0xffff;
                    if ((ushort)uVar15 == uVar3) {
                      if (uVar3 != 0x13) goto LAB_00729222;
                      pIVar12->m_iChangeInfoSerialNumber = 0;
                      pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                      break;
                    }
                  } while (puVar5[uVar14 * 0x14 + local_2c0 + 1] != 0x108);
                }
              }
              else if (puVar5[0x7d1] == 100) {
                pIVar12->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
              }
              else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                pIVar12->m_iChangeInfo = puVar5[0x7d1];
                *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
                pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
                iVar2 = *piVar6 + (uint)pIVar12->m_iChangeInfo * 0x28;
                *(undefined2 *)(iVar2 + 2) = 0x108;
                *(undefined2 *)(iVar2 + 0x28) = 1;
              }
            }
          }
          else {
            *(uint *)(iVar9 + 100) = *(uint *)(iVar9 + 100) | 1;
          }
          *(undefined1 *)(iVar9 + 0x10a) = 0;
        }
        uVar14 = (this->m_hEyeGlow).super_CBaseHandle.m_Index;
        if ((uVar14 == 0xffffffff) ||
           (iVar9 = (uVar14 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x52052b),
           *(uint *)(iVar9 + 8) != uVar14 >> 0x10)) {
          pCVar10 = (CSprite *)0x0;
        }
        else {
          pCVar10 = *(CSprite **)(iVar9 + 4);
        }
        iVar9 = 0x80;
      }
      else if (state == HOVER_TURRET_AIM_TARGET) {
        this->m_iDesiredState = 0;
        this->m_bAimingAtTarget = true;
        GetClosestVisibleEnemyPosition(local_28,this);
        (this->m_vecTargetPos).x = local_28[0].x;
        (this->m_vecTargetPos).y = local_28[0].y;
        (this->m_vecTargetPos).z = local_28[0].z;
        this->m_iFiringState = HOVER_TURRET_AIM_TARGET;
        this->m_flAimStartTime = *(float *)(**(int **)(unaff_EBX + 0x520523) + 0xc);
        if ((bVar16) &&
           ((**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                  super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                  super_CBaseCombatCharacter.super_CBaseFlex.
                                  super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x684))
                      (this,*(undefined4 *)(&DAT_006a217b + unaff_EBX)),
           (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.m_nSkin.m_Value != 1)) {
          if ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
              super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
              m_Network.m_TimerEvent.m_bRegistered == false) {
            pCVar4 = &((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                       super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                       super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                       super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
            if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
              pIVar12 = CBaseEdict::GetChangeAccessor(pCVar4);
              piVar6 = *(int **)(unaff_EBX + 0x520527);
              puVar5 = (ushort *)*piVar6;
              if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
                uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
                uVar3 = puVar5[uVar14 * 0x14 + 0x14];
                if (uVar3 == 0) {
LAB_00729bc3:
                  puVar5[(uint)uVar3 + uVar14 * 0x14 + 1] = 0x3e0;
                  puVar5[uVar14 * 0x14 + 0x14] = uVar3 + 1;
                }
                else if (puVar5[uVar14 * 0x14 + 1] != 0x3e0) {
                  local_2d8 = 0;
                  do {
                    uVar15 = local_2d8 + 1;
                    local_2d8 = uVar15 & 0xffff;
                    if ((ushort)uVar15 == uVar3) {
                      if (uVar3 != 0x13) goto LAB_00729bc3;
                      pIVar12->m_iChangeInfoSerialNumber = 0;
                      pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                      break;
                    }
                  } while (puVar5[uVar14 * 0x14 + local_2d8 + 1] != 0x3e0);
                }
              }
              else if (puVar5[0x7d1] == 100) {
                pIVar12->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
              }
              else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                pIVar12->m_iChangeInfo = puVar5[0x7d1];
                *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
                pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
                iVar9 = *piVar6 + (uint)pIVar12->m_iChangeInfo * 0x28;
                *(undefined2 *)(iVar9 + 2) = 0x3e0;
                *(undefined2 *)(iVar9 + 0x28) = 1;
              }
            }
          }
          else {
            puVar1 = &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                      super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                      super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                      super_CBaseEntity.m_Network.field_0x4c;
            *(uint *)puVar1 = *(uint *)puVar1 | 1;
          }
          (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
          super_CBaseAnimating.m_nSkin.m_Value = 1;
        }
        uVar14 = (this->m_hEyeGlow).super_CBaseHandle.m_Index;
        if ((uVar14 == 0xffffffff) ||
           (iVar9 = (uVar14 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x52052b),
           *(uint *)(iVar9 + 8) != uVar14 >> 0x10)) {
          iVar9 = 0;
        }
        else {
          iVar9 = *(int *)(iVar9 + 4);
        }
        if (*(char *)(iVar9 + 0x108) != -1) {
          if (*(char *)(iVar9 + 0x60) == '\0') {
            pCVar4 = *(CBaseEdict **)(iVar9 + 0x24);
            if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
              pIVar12 = CBaseEdict::GetChangeAccessor(pCVar4);
              puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x520527);
              if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
                uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
                uVar3 = puVar5[uVar14 * 0x14 + 0x14];
                if (uVar3 == 0) {
LAB_00728e50:
                  puVar5[(uint)uVar3 + uVar14 * 0x14 + 1] = 0x108;
                  puVar5[uVar14 * 0x14 + 0x14] = uVar3 + 1;
                }
                else if (puVar5[uVar14 * 0x14 + 1] != 0x108) {
                  local_308 = 0;
                  do {
                    uVar15 = local_308 + 1;
                    local_308 = uVar15 & 0xffff;
                    if ((ushort)uVar15 == uVar3) {
                      if (uVar3 != 0x13) goto LAB_00728e50;
                      pIVar12->m_iChangeInfoSerialNumber = 0;
                      pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                      break;
                    }
                  } while (puVar5[uVar14 * 0x14 + local_308 + 1] != 0x108);
                }
              }
              else if (puVar5[0x7d1] == 100) {
                pIVar12->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
              }
              else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                pIVar12->m_iChangeInfo = puVar5[0x7d1];
                piVar6 = *(int **)(unaff_EBX + 0x520527);
                *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
                pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
                iVar2 = *piVar6 + (uint)pIVar12->m_iChangeInfo * 0x28;
                *(undefined2 *)(iVar2 + 2) = 0x108;
                *(undefined2 *)(iVar2 + 0x28) = 1;
              }
            }
          }
          else {
            *(uint *)(iVar9 + 100) = *(uint *)(iVar9 + 100) | 1;
          }
          *(undefined1 *)(iVar9 + 0x108) = 0xff;
        }
        if (*(char *)(iVar9 + 0x109) != -1) {
          if (*(char *)(iVar9 + 0x60) == '\0') {
            pCVar4 = *(CBaseEdict **)(iVar9 + 0x24);
            if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
              pIVar12 = CBaseEdict::GetChangeAccessor(pCVar4);
              puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x520527);
              if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
                uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
                uVar3 = puVar5[uVar14 * 0x14 + 0x14];
                if (uVar3 == 0) {
LAB_00729010:
                  puVar5[(uint)uVar3 + uVar14 * 0x14 + 1] = 0x108;
                  puVar5[uVar14 * 0x14 + 0x14] = uVar3 + 1;
                }
                else if (puVar5[uVar14 * 0x14 + 1] != 0x108) {
                  local_304 = 0;
                  do {
                    uVar15 = local_304 + 1;
                    local_304 = uVar15 & 0xffff;
                    if ((ushort)uVar15 == uVar3) {
                      if (uVar3 != 0x13) goto LAB_00729010;
                      pIVar12->m_iChangeInfoSerialNumber = 0;
                      pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                      break;
                    }
                  } while (puVar5[uVar14 * 0x14 + local_304 + 1] != 0x108);
                }
              }
              else if (puVar5[0x7d1] == 100) {
                pIVar12->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
              }
              else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                pIVar12->m_iChangeInfo = puVar5[0x7d1];
                piVar6 = *(int **)(unaff_EBX + 0x520527);
                *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
                pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
                iVar2 = *piVar6 + (uint)pIVar12->m_iChangeInfo * 0x28;
                *(undefined2 *)(iVar2 + 2) = 0x108;
                *(undefined2 *)(iVar2 + 0x28) = 1;
              }
            }
          }
          else {
            *(uint *)(iVar9 + 100) = *(uint *)(iVar9 + 100) | 1;
          }
          *(undefined1 *)(iVar9 + 0x109) = 0xff;
        }
        if (*(char *)(iVar9 + 0x10a) != '\0') {
          if (*(char *)(iVar9 + 0x60) == '\0') {
            pCVar4 = *(CBaseEdict **)(iVar9 + 0x24);
            if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
              pIVar12 = CBaseEdict::GetChangeAccessor(pCVar4);
              piVar6 = *(int **)(unaff_EBX + 0x520527);
              puVar5 = (ushort *)*piVar6;
              if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
                uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
                uVar3 = puVar5[uVar14 * 0x14 + 0x14];
                if (uVar3 == 0) {
LAB_00729355:
                  puVar5[(uint)uVar3 + uVar14 * 0x14 + 1] = 0x108;
                  puVar5[uVar14 * 0x14 + 0x14] = uVar3 + 1;
                }
                else if (puVar5[uVar14 * 0x14 + 1] != 0x108) {
                  local_2d4 = 0;
                  do {
                    uVar15 = local_2d4 + 1;
                    local_2d4 = uVar15 & 0xffff;
                    if ((ushort)uVar15 == uVar3) {
                      if (uVar3 != 0x13) goto LAB_00729355;
                      pIVar12->m_iChangeInfoSerialNumber = 0;
                      pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                      break;
                    }
                  } while (puVar5[uVar14 * 0x14 + local_2d4 + 1] != 0x108);
                }
              }
              else if (puVar5[0x7d1] == 100) {
                pIVar12->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
              }
              else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                pIVar12->m_iChangeInfo = puVar5[0x7d1];
                *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
                pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
                iVar2 = *piVar6 + (uint)pIVar12->m_iChangeInfo * 0x28;
                *(undefined2 *)(iVar2 + 2) = 0x108;
                *(undefined2 *)(iVar2 + 0x28) = 1;
              }
            }
          }
          else {
            *(uint *)(iVar9 + 100) = *(uint *)(iVar9 + 100) | 1;
          }
          *(undefined1 *)(iVar9 + 0x10a) = 0;
        }
        uVar14 = (this->m_hEyeGlow).super_CBaseHandle.m_Index;
        if ((uVar14 == 0xffffffff) ||
           (iVar9 = (uVar14 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x52052b),
           *(uint *)(iVar9 + 8) != uVar14 >> 0x10)) {
          pCVar10 = (CSprite *)0x0;
        }
        else {
          pCVar10 = *(CSprite **)(iVar9 + 4);
        }
        iVar9 = 0xc4;
      }
      else {
        this->m_bAimingAtTarget = false;
        this->m_flAimStartTime = 0.0;
        this->m_iDesiredState = 1;
        this->m_iFiringState = HOVER_TURRET_FIND_TARGET;
        if ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
            super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
            m_Network.m_TimerEvent.m_bRegistered == false) {
          pCVar4 = &((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                     super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                     super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                     super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
          if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
            pIVar12 = CBaseEdict::GetChangeAccessor(pCVar4);
            puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x520527);
            if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
              uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
              uVar3 = puVar5[uVar14 * 0x14 + 0x14];
              if (uVar3 == 0) {
LAB_00728995:
                puVar5[(uint)uVar3 + uVar14 * 0x14 + 1] = 0xf0c;
                puVar5[uVar14 * 0x14 + 0x14] = uVar3 + 1;
              }
              else if (puVar5[uVar14 * 0x14 + 1] != 0xf0c) {
                local_310 = 0;
                do {
                  uVar15 = local_310 + 1;
                  local_310 = uVar15 & 0xffff;
                  if ((ushort)uVar15 == uVar3) {
                    if (uVar3 != 0x13) goto LAB_00728995;
                    pIVar12->m_iChangeInfoSerialNumber = 0;
                    pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                    break;
                  }
                } while (puVar5[uVar14 * 0x14 + local_310 + 1] != 0xf0c);
              }
            }
            else if (puVar5[0x7d1] == 100) {
              pIVar12->m_iChangeInfoSerialNumber = 0;
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
            }
            else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
              pIVar12->m_iChangeInfo = puVar5[0x7d1];
              *(short *)(**(int **)(unaff_EBX + 0x520527) + 0xfa2) =
                   *(short *)(**(int **)(unaff_EBX + 0x520527) + 0xfa2) + 1;
              piVar6 = *(int **)(unaff_EBX + 0x520527);
              pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
              iVar9 = *piVar6 + (uint)pIVar12->m_iChangeInfo * 0x28;
              *(undefined2 *)(iVar9 + 2) = 0xf0c;
              *(undefined2 *)(iVar9 + 0x28) = 1;
            }
          }
        }
        else {
          puVar1 = &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                    super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                    super_CBaseEntity.m_Network.field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (this->m_iLaserState).m_Value = 1;
        if (bVar16) {
          if ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
              super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.m_nSkin.m_Value != 3)
          {
            if ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                m_Network.m_TimerEvent.m_bRegistered == false) {
              pCVar4 = &((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                         super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter
                         .super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                         super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
              if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
                pIVar12 = CBaseEdict::GetChangeAccessor(pCVar4);
                piVar6 = *(int **)(unaff_EBX + 0x520527);
                puVar5 = (ushort *)*piVar6;
                if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
                  uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
                  uVar3 = puVar5[uVar14 * 0x14 + 0x14];
                  if (uVar3 == 0) {
LAB_00729831:
                    puVar5[(uint)uVar3 + uVar14 * 0x14 + 1] = 0x3e0;
                    puVar5[uVar14 * 0x14 + 0x14] = uVar3 + 1;
                  }
                  else if (puVar5[uVar14 * 0x14 + 1] != 0x3e0) {
                    local_2e4 = 0;
                    do {
                      uVar15 = local_2e4 + 1;
                      local_2e4 = uVar15 & 0xffff;
                      if ((ushort)uVar15 == uVar3) {
                        if (uVar3 != 0x13) goto LAB_00729831;
                        pIVar12->m_iChangeInfoSerialNumber = 0;
                        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                        break;
                      }
                    } while (puVar5[uVar14 * 0x14 + local_2e4 + 1] != 0x3e0);
                  }
                }
                else if (puVar5[0x7d1] == 100) {
                  pIVar12->m_iChangeInfoSerialNumber = 0;
                  pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                }
                else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                  pIVar12->m_iChangeInfo = puVar5[0x7d1];
                  *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
                  pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
                  iVar9 = *piVar6 + (uint)pIVar12->m_iChangeInfo * 0x28;
                  *(undefined2 *)(iVar9 + 2) = 0x3e0;
                  *(undefined2 *)(iVar9 + 0x28) = 1;
                }
              }
            }
            else {
              puVar1 = &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                        super_CBaseEntity.m_Network.field_0x4c;
              *(uint *)puVar1 = *(uint *)puVar1 | 1;
            }
            (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
            super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.m_nSkin.m_Value = 3;
          }
          (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                super_CBaseCombatCharacter.super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x684))
                    (this,*(undefined4 *)(&DAT_006a2177 + unaff_EBX));
        }
        uVar14 = (this->m_hEyeGlow).super_CBaseHandle.m_Index;
        if ((uVar14 == 0xffffffff) ||
           (iVar9 = (uVar14 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x52052b),
           *(uint *)(iVar9 + 8) != uVar14 >> 0x10)) {
          iVar9 = 0;
        }
        else {
          iVar9 = *(int *)(iVar9 + 4);
        }
        if (*(char *)(iVar9 + 0x108) != '\0') {
          if (*(char *)(iVar9 + 0x60) == '\0') {
            pCVar4 = *(CBaseEdict **)(iVar9 + 0x24);
            if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
              pIVar12 = CBaseEdict::GetChangeAccessor(pCVar4);
              piVar6 = *(int **)(unaff_EBX + 0x520527);
              puVar5 = (ushort *)*piVar6;
              if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
                uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
                uVar3 = puVar5[uVar14 * 0x14 + 0x14];
                if (uVar3 == 0) {
LAB_00729531:
                  puVar5[(uint)uVar3 + uVar14 * 0x14 + 1] = 0x108;
                  puVar5[uVar14 * 0x14 + 0x14] = uVar3 + 1;
                }
                else if (puVar5[uVar14 * 0x14 + 1] != 0x108) {
                  local_2e0 = 0;
                  do {
                    uVar15 = local_2e0 + 1;
                    local_2e0 = uVar15 & 0xffff;
                    if ((ushort)uVar15 == uVar3) {
                      if (uVar3 != 0x13) goto LAB_00729531;
                      pIVar12->m_iChangeInfoSerialNumber = 0;
                      pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                      break;
                    }
                  } while (puVar5[uVar14 * 0x14 + local_2e0 + 1] != 0x108);
                }
              }
              else if (puVar5[0x7d1] == 100) {
                pIVar12->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
              }
              else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                pIVar12->m_iChangeInfo = puVar5[0x7d1];
                *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
                pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
                iVar2 = *piVar6 + (uint)pIVar12->m_iChangeInfo * 0x28;
                *(undefined2 *)(iVar2 + 2) = 0x108;
                *(undefined2 *)(iVar2 + 0x28) = 1;
              }
            }
          }
          else {
            *(uint *)(iVar9 + 100) = *(uint *)(iVar9 + 100) | 1;
          }
          *(undefined1 *)(iVar9 + 0x108) = 0;
        }
        if (*(char *)(iVar9 + 0x109) != '\0') {
          if (*(char *)(iVar9 + 0x60) == '\0') {
            pCVar4 = *(CBaseEdict **)(iVar9 + 0x24);
            if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
              pIVar12 = CBaseEdict::GetChangeAccessor(pCVar4);
              piVar6 = *(int **)(unaff_EBX + 0x520527);
              puVar5 = (ushort *)*piVar6;
              if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
                uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
                uVar3 = puVar5[uVar14 * 0x14 + 0x14];
                if (uVar3 == 0) {
LAB_00729443:
                  puVar5[(uint)uVar3 + uVar14 * 0x14 + 1] = 0x108;
                  puVar5[uVar14 * 0x14 + 0x14] = uVar3 + 1;
                }
                else if (puVar5[uVar14 * 0x14 + 1] != 0x108) {
                  local_2dc = 0;
                  do {
                    uVar15 = local_2dc + 1;
                    local_2dc = uVar15 & 0xffff;
                    if ((ushort)uVar15 == uVar3) {
                      if (uVar3 != 0x13) goto LAB_00729443;
                      pIVar12->m_iChangeInfoSerialNumber = 0;
                      pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                      break;
                    }
                  } while (puVar5[uVar14 * 0x14 + local_2dc + 1] != 0x108);
                }
              }
              else if (puVar5[0x7d1] == 100) {
                pIVar12->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
              }
              else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                pIVar12->m_iChangeInfo = puVar5[0x7d1];
                *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
                pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
                iVar2 = *piVar6 + (uint)pIVar12->m_iChangeInfo * 0x28;
                *(undefined2 *)(iVar2 + 2) = 0x108;
                *(undefined2 *)(iVar2 + 0x28) = 1;
              }
            }
          }
          else {
            *(uint *)(iVar9 + 100) = *(uint *)(iVar9 + 100) | 1;
          }
          *(undefined1 *)(iVar9 + 0x109) = 0;
        }
        if (*(char *)(iVar9 + 0x10a) != -1) {
          if (*(char *)(iVar9 + 0x60) == '\0') {
            pCVar4 = *(CBaseEdict **)(iVar9 + 0x24);
            if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
              pIVar12 = CBaseEdict::GetChangeAccessor(pCVar4);
              puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x520527);
              if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
                uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
                uVar3 = puVar5[uVar14 * 0x14 + 0x14];
                if (uVar3 == 0) {
LAB_00728b70:
                  puVar5[(uint)uVar3 + uVar14 * 0x14 + 1] = 0x108;
                  puVar5[uVar14 * 0x14 + 0x14] = uVar3 + 1;
                }
                else if (puVar5[uVar14 * 0x14 + 1] != 0x108) {
                  local_30c = 0;
                  do {
                    uVar15 = local_30c + 1;
                    local_30c = uVar15 & 0xffff;
                    if ((ushort)uVar15 == uVar3) {
                      if (uVar3 != 0x13) goto LAB_00728b70;
                      pIVar12->m_iChangeInfoSerialNumber = 0;
                      pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                      break;
                    }
                  } while (puVar5[uVar14 * 0x14 + local_30c + 1] != 0x108);
                }
              }
              else if (puVar5[0x7d1] == 100) {
                pIVar12->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
              }
              else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                pIVar12->m_iChangeInfo = puVar5[0x7d1];
                piVar6 = *(int **)(unaff_EBX + 0x520527);
                *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
                pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
                iVar2 = *piVar6 + (uint)pIVar12->m_iChangeInfo * 0x28;
                *(undefined2 *)(iVar2 + 2) = 0x108;
                *(undefined2 *)(iVar2 + 0x28) = 1;
              }
            }
          }
          else {
            *(uint *)(iVar9 + 100) = *(uint *)(iVar9 + 100) | 1;
          }
          *(undefined1 *)(iVar9 + 0x10a) = 0xff;
        }
        uVar14 = (this->m_hEyeGlow).super_CBaseHandle.m_Index;
        if ((uVar14 == 0xffffffff) ||
           (iVar9 = (uVar14 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x52052b),
           *(uint *)(iVar9 + 8) != uVar14 >> 0x10)) {
          pCVar10 = (CSprite *)0x0;
        }
        else {
          pCVar10 = *(CSprite **)(iVar9 + 4);
        }
        iVar9 = 0x60;
      }
      CSprite::SetBrightness(pCVar10,iVar9,0.1);
      uVar14 = (this->m_hEyeGlow).super_CBaseHandle.m_Index;
      if ((uVar14 == 0xffffffff) ||
         (iVar9 = (uVar14 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x52052b),
         *(uint *)(iVar9 + 8) != uVar14 >> 0x10)) {
        pCVar10 = (CSprite *)0x0;
      }
      else {
        pCVar10 = *(CSprite **)(iVar9 + 4);
      }
      CSprite::SetScale(pCVar10,0.4,0.1);
      return;
    }
    this->m_iFiringState = HOVER_TURRET_SHOT_DISABLED;
    this->m_iDesiredState = 5;
    if (bVar16) {
      (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                            super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                            super_CBaseAnimating + 0x684))
                (this,*(undefined4 *)(&DAT_006a217f + unaff_EBX));
      if ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
          super_CBaseAnimating.m_nSkin.m_Value != 2) {
        if ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
            super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
            m_Network.m_TimerEvent.m_bRegistered == false) {
          pCVar4 = &((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                     super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                     super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                     super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
          if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
            pIVar12 = CBaseEdict::GetChangeAccessor(pCVar4);
            piVar6 = *(int **)(unaff_EBX + 0x520527);
            puVar5 = (ushort *)*piVar6;
            if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
              uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
              uVar3 = puVar5[uVar14 * 0x14 + 0x14];
              if (uVar3 == 0) {
LAB_00729ad6:
                puVar5[(uint)uVar3 + uVar14 * 0x14 + 1] = 0x3e0;
                puVar5[uVar14 * 0x14 + 0x14] = uVar3 + 1;
              }
              else if (puVar5[uVar14 * 0x14 + 1] != 0x3e0) {
                local_2d0 = 0;
                do {
                  uVar15 = local_2d0 + 1;
                  local_2d0 = uVar15 & 0xffff;
                  if ((ushort)uVar15 == uVar3) {
                    if (uVar3 != 0x13) goto LAB_00729ad6;
                    pIVar12->m_iChangeInfoSerialNumber = 0;
                    pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                    break;
                  }
                } while (puVar5[uVar14 * 0x14 + local_2d0 + 1] != 0x3e0);
              }
            }
            else if (puVar5[0x7d1] == 100) {
              pIVar12->m_iChangeInfoSerialNumber = 0;
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
            }
            else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
              pIVar12->m_iChangeInfo = puVar5[0x7d1];
              *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
              pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
              iVar9 = *piVar6 + (uint)pIVar12->m_iChangeInfo * 0x28;
              *(undefined2 *)(iVar9 + 2) = 0x3e0;
              *(undefined2 *)(iVar9 + 0x28) = 1;
            }
          }
        }
        else {
          puVar1 = &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                    super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                    super_CBaseEntity.m_Network.field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
        super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
        super_CBaseAnimating.m_nSkin.m_Value = 2;
      }
      if ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
          super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
        pCVar4 = &((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                   super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                   super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                   super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
        if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
          pIVar12 = CBaseEdict::GetChangeAccessor(pCVar4);
          piVar6 = *(int **)(unaff_EBX + 0x520527);
          puVar5 = (ushort *)*piVar6;
          if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
            uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar3 = puVar5[uVar14 * 0x14 + 0x14];
            if (uVar3 == 0) {
LAB_00729958:
              puVar5[(uint)uVar3 + uVar14 * 0x14 + 1] = 0xf0c;
              puVar5[uVar14 * 0x14 + 0x14] = uVar3 + 1;
            }
            else if (puVar5[uVar14 * 0x14 + 1] != 0xf0c) {
              local_2cc = 0;
              do {
                uVar15 = local_2cc + 1;
                local_2cc = uVar15 & 0xffff;
                if ((ushort)uVar15 == uVar3) {
                  if (uVar3 != 0x13) goto LAB_00729958;
                  pIVar12->m_iChangeInfoSerialNumber = 0;
                  pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar5[uVar14 * 0x14 + local_2cc + 1] != 0xf0c);
            }
          }
          else if (puVar5[0x7d1] == 100) {
            pIVar12->m_iChangeInfoSerialNumber = 0;
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar12->m_iChangeInfo = puVar5[0x7d1];
            *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
            pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
            iVar9 = *piVar6 + (uint)pIVar12->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar9 + 2) = 0xf0c;
            *(undefined2 *)(iVar9 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                  super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                  super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity
                  .m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->m_iLaserState).m_Value = 0;
    }
    uVar14 = (this->m_hEyeGlow).super_CBaseHandle.m_Index;
    if ((uVar14 == 0xffffffff) ||
       (iVar9 = (uVar14 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x52052b),
       *(uint *)(iVar9 + 8) != uVar14 >> 0x10)) {
      iVar9 = 0;
    }
    else {
      iVar9 = *(int *)(iVar9 + 4);
    }
    if (*(char *)(iVar9 + 0x108) != -1) {
      if (*(char *)(iVar9 + 0x60) == '\0') {
        pCVar4 = *(CBaseEdict **)(iVar9 + 0x24);
        if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
          pIVar12 = CBaseEdict::GetChangeAccessor(pCVar4);
          puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x520527);
          if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
            uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar3 = puVar5[uVar14 * 0x14 + 0x14];
            if (uVar3 == 0) {
LAB_00728c70:
              puVar5[(uint)uVar3 + uVar14 * 0x14 + 1] = 0x108;
              puVar5[uVar14 * 0x14 + 0x14] = uVar3 + 1;
            }
            else if (puVar5[uVar14 * 0x14 + 1] != 0x108) {
              local_300 = 0;
              do {
                uVar15 = local_300 + 1;
                local_300 = uVar15 & 0xffff;
                if ((ushort)uVar15 == uVar3) {
                  if (uVar3 != 0x13) goto LAB_00728c70;
                  pIVar12->m_iChangeInfoSerialNumber = 0;
                  pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar5[uVar14 * 0x14 + local_300 + 1] != 0x108);
            }
          }
          else if (puVar5[0x7d1] == 100) {
            pIVar12->m_iChangeInfoSerialNumber = 0;
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar12->m_iChangeInfo = puVar5[0x7d1];
            piVar6 = *(int **)(unaff_EBX + 0x520527);
            *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
            pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
            iVar2 = *piVar6 + (uint)pIVar12->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar2 + 2) = 0x108;
            *(undefined2 *)(iVar2 + 0x28) = 1;
          }
        }
      }
      else {
        *(uint *)(iVar9 + 100) = *(uint *)(iVar9 + 100) | 1;
      }
      *(undefined1 *)(iVar9 + 0x108) = 0xff;
    }
    if (*(char *)(iVar9 + 0x109) != '\0') {
      if (*(char *)(iVar9 + 0x60) == '\0') {
        pCVar4 = *(CBaseEdict **)(iVar9 + 0x24);
        if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
          pIVar12 = CBaseEdict::GetChangeAccessor(pCVar4);
          piVar6 = *(int **)(unaff_EBX + 0x520527);
          puVar5 = (ushort *)*piVar6;
          if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
            uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar3 = puVar5[uVar14 * 0x14 + 0x14];
            if (uVar3 == 0) {
LAB_007296f2:
              puVar5[(uint)uVar3 + uVar14 * 0x14 + 1] = 0x108;
              puVar5[uVar14 * 0x14 + 0x14] = uVar3 + 1;
            }
            else if (puVar5[uVar14 * 0x14 + 1] != 0x108) {
              local_2c8 = 0;
              do {
                uVar15 = local_2c8 + 1;
                local_2c8 = uVar15 & 0xffff;
                if ((ushort)uVar15 == uVar3) {
                  if (uVar3 != 0x13) goto LAB_007296f2;
                  pIVar12->m_iChangeInfoSerialNumber = 0;
                  pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar5[uVar14 * 0x14 + local_2c8 + 1] != 0x108);
            }
          }
          else if (puVar5[0x7d1] == 100) {
            pIVar12->m_iChangeInfoSerialNumber = 0;
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar12->m_iChangeInfo = puVar5[0x7d1];
            *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
            pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
            iVar2 = *piVar6 + (uint)pIVar12->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar2 + 2) = 0x108;
            *(undefined2 *)(iVar2 + 0x28) = 1;
          }
        }
      }
      else {
        *(uint *)(iVar9 + 100) = *(uint *)(iVar9 + 100) | 1;
      }
      *(undefined1 *)(iVar9 + 0x109) = 0;
    }
    if (*(char *)(iVar9 + 0x10a) != -1) {
      if (*(char *)(iVar9 + 0x60) == '\0') {
        pCVar4 = *(CBaseEdict **)(iVar9 + 0x24);
        if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
          pIVar12 = CBaseEdict::GetChangeAccessor(pCVar4);
          puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x520527);
          if (pIVar12->m_iChangeInfoSerialNumber == *puVar5) {
            uVar14 = (uint)pIVar12->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar3 = puVar5[uVar14 * 0x14 + 0x14];
            if (uVar3 == 0) {
LAB_00728d60:
              puVar5[(uint)uVar3 + uVar14 * 0x14 + 1] = 0x108;
              puVar5[uVar14 * 0x14 + 0x14] = uVar3 + 1;
            }
            else if (puVar5[uVar14 * 0x14 + 1] != 0x108) {
              local_2fc = 0;
              do {
                uVar15 = local_2fc + 1;
                local_2fc = uVar15 & 0xffff;
                if ((ushort)uVar15 == uVar3) {
                  if (uVar3 != 0x13) goto LAB_00728d60;
                  pIVar12->m_iChangeInfoSerialNumber = 0;
                  pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar5[uVar14 * 0x14 + local_2fc + 1] != 0x108);
            }
          }
          else if (puVar5[0x7d1] == 100) {
            pIVar12->m_iChangeInfoSerialNumber = 0;
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar12->m_iChangeInfo = puVar5[0x7d1];
            piVar6 = *(int **)(unaff_EBX + 0x520527);
            *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
            pIVar12->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
            iVar2 = *piVar6 + (uint)pIVar12->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar2 + 2) = 0x108;
            *(undefined2 *)(iVar2 + 0x28) = 1;
          }
        }
      }
      else {
        *(uint *)(iVar9 + 100) = *(uint *)(iVar9 + 100) | 1;
      }
      *(undefined1 *)(iVar9 + 0x10a) = 0xff;
    }
    uVar14 = (this->m_hEyeGlow).super_CBaseHandle.m_Index;
    if ((uVar14 == 0xffffffff) ||
       (iVar9 = (uVar14 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x52052b),
       *(uint *)(iVar9 + 8) != uVar14 >> 0x10)) {
      pCVar10 = (CSprite *)0x0;
    }
    else {
      pCVar10 = *(CSprite **)(iVar9 + 4);
    }
    CSprite::SetScale(pCVar10,0.1,3.0);
    uVar14 = (this->m_hEyeGlow).super_CBaseHandle.m_Index;
    if ((uVar14 == 0xffffffff) ||
       (iVar9 = (uVar14 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x52052b),
       *(uint *)(iVar9 + 8) != uVar14 >> 0x10)) {
      pCVar10 = (CSprite *)0x0;
    }
    else {
      pCVar10 = *(CSprite **)(iVar9 + 4);
    }
    CSprite::SetBrightness(pCVar10,0,3.0);
  }
  return;
}


/* CNPC_HoverTurret::FindTargetThink at 0072b150 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void FindTargetThink(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::FindTargetThink(CNPC_HoverTurret *this)

{
  float *pfVar1;
  uint *puVar2;
  ushort uVar3;
  CBaseEdict *this_00;
  ushort *puVar4;
  bool bVar5;
  char cVar6;
  char cVar7;
  char cVar8;
  CAI_Enemies *this_01;
  CBaseEntity *pEnemy;
  int *piVar9;
  int iVar10;
  IChangeInfoAccessor *pIVar11;
  uint uVar12;
  uint uVar13;
  int unaff_EBX;
  uint local_60;
  
  ___i686_get_pc_thunk_bx();
  pfVar1 = (float *)(**(int **)(unaff_EBX + 0x51b9fd) + 0xc);
  if ((*pfVar1 <= this->m_flEngineStallTime && this->m_flEngineStallTime != *pfVar1) ||
     (this->m_iFiringState == HOVER_TURRET_SHOT_DISABLED)) {
    return;
  }
  this_01 = (CAI_Enemies *)
            (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                  super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                  super_CBaseCombatCharacter.super_CBaseFlex.
                                  super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x860))(this);
  CAI_Enemies::RefreshMemories(this_01);
  CAI_Senses::Look((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                   super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.m_pSenses,0x2000);
  pEnemy = (CBaseEntity *)
           (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                 super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                 super_CBaseCombatCharacter.super_CBaseFlex.
                                 super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x6cc))(this);
  CAI_BaseNPC::SetEnemy((CAI_BaseNPC *)this,pEnemy,true);
  if ((this->m_iLaserState).m_Value == 0) {
    if (*(bool *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x60) ==
        false) {
      this_00 = *(CBaseEdict **)
                 ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x24);
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar11 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)
                 **(undefined4 **)(CFuncTankCombineCannon::InputDisableHarrass + unaff_EBX + 1);
        if (pIVar11->m_iChangeInfoSerialNumber == *puVar4) {
          uVar12 = (uint)pIVar11->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar12 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_0072b47c:
            puVar4[(uint)uVar3 + uVar12 * 0x14 + 1] = 0xf0c;
            puVar4[uVar12 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar12 * 0x14 + 1] != 0xf0c) {
            local_60 = 0;
            do {
              uVar13 = local_60 + 1;
              local_60 = uVar13 & 0xffff;
              if ((ushort)uVar13 == uVar3) {
                if (uVar3 != 0x13) goto LAB_0072b47c;
                pIVar11->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar12 * 0x14 + local_60 + 1] != 0xf0c);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar11->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar11->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(CFuncTankCombineCannon::InputDisableHarrass + unaff_EBX + 1) + 0xfa2
                    ) = *(short *)(**(int **)(CFuncTankCombineCannon::InputDisableHarrass +
                                             unaff_EBX + 1) + 0xfa2) + 1;
          piVar9 = *(int **)(CFuncTankCombineCannon::InputDisableHarrass + unaff_EBX + 1);
          pIVar11->m_iChangeInfoSerialNumber = *(ushort *)*piVar9;
          iVar10 = *piVar9 + (uint)pIVar11->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar10 + 2) = 0xf0c;
          *(undefined2 *)(iVar10 + 0x28) = 1;
        }
      }
    }
    else {
      puVar2 = (uint *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                              super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay
                              .super_CBaseAnimating + 100);
      *puVar2 = *puVar2 | 1;
    }
    (this->m_iLaserState).m_Value = 1;
  }
  bVar5 = CAI_BaseNPC::HasCondition((CAI_BaseNPC *)this,10);
  if (bVar5) {
    piVar9 = (int *)(**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>
                                          ).super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                          super_CBaseCombatCharacter.super_CBaseFlex.
                                          super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x188))
                              (this);
    if ((piVar9 != (int *)0x0) && (cVar6 = (**(code **)(*piVar9 + 0x114))(piVar9), cVar6 != '\0')) {
      cVar6 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                    super_CBaseCombatCharacter.super_CBaseFlex.
                                    super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x3d8))
                        (this,piVar9);
      cVar7 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                    super_CBaseCombatCharacter.super_CBaseFlex.
                                    super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x250))
                        (this,piVar9,0x4041,0);
      iVar10 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                     super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                     super_CBaseCombatCharacter.super_CBaseFlex.
                                     super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x3e0))
                         (this,piVar9);
      if ((iVar10 == 0) ||
         (cVar8 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                        super_CBaseCombatCharacter.super_CBaseFlex.
                                        super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x3d4))
                            (this,iVar10,piVar9,0x4041,0), cVar8 == '\0')) {
        bVar5 = false;
      }
      else {
        bVar5 = true;
      }
      if (((cVar6 != '\0') && (cVar7 != '\0')) || (bVar5)) {
        SetFiringState(this,HOVER_TURRET_AIM_TARGET);
      }
    }
  }
  else {
    if ((*(byte *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                         super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter
                         .super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x12d)
        & 8) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
    (this->m_vecGoalAngles).x =
         ((QAngle *)
         ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x330))->x;
    (this->m_vecGoalAngles).y =
         *(vec_t *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x334);
    (this->m_vecGoalAngles).z =
         *(vec_t *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x338);
    (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
    super_CAI_BaseNPC.super_CBaseCombatCharacter.m_flFieldOfView = 0.4;
  }
  UpdateFacing(this);
  return;
}


/* CNPC_HoverTurret::AimThink at 0072aca0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void AimThink(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::AimThink(CNPC_HoverTurret *this)

{
  uint *puVar1;
  ushort uVar2;
  CBaseEdict *this_00;
  ushort *puVar3;
  bool bVar4;
  char cVar5;
  char cVar6;
  char cVar7;
  int iVar8;
  int *piVar9;
  IChangeInfoAccessor *pIVar10;
  uint uVar11;
  uint uVar12;
  int unaff_EBX;
  float fVar13;
  uint local_90;
  matrix3x4_t *local_78;
  Vector local_4c;
  Vector local_40;
  float local_34;
  float local_30;
  float local_2c;
  Vector local_28 [2];
  
                    /* Unresolved local var: bool bAbortAim@[???] */
  ___i686_get_pc_thunk_bx();
  bVar4 = CAI_BaseNPC::HasCondition((CAI_BaseNPC *)this,10);
  if (((!bVar4) ||
      (*(float *)(**(int **)(&DAT_0051bead + unaff_EBX) + 0xc) <= this->m_flEngineStallTime &&
       this->m_flEngineStallTime != *(float *)(**(int **)(&DAT_0051bead + unaff_EBX) + 0xc))) ||
     (this->m_iFiringState == HOVER_TURRET_SHOT_DISABLED)) {
LAB_0072acf0:
    if ((*(byte *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                         super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter
                         .super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x12d)
        & 8) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
    (this->m_vecGoalAngles).x =
         ((QAngle *)
         ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x330))->x;
    (this->m_vecGoalAngles).y =
         *(vec_t *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x334);
    (this->m_vecGoalAngles).z =
         *(vec_t *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x338);
    UpdateFacing(this);
    if (this->m_bAimingAtTarget == false) {
      return;
    }
  }
  else {
    iVar8 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                  super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                  super_CBaseCombatCharacter.super_CBaseFlex.
                                  super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x188))(this);
    if (iVar8 != 0) {
                    /* Unresolved local var: Vector vecEnemyPosition@[???]
                       Unresolved local var: Vector vecEnemyMovement@[???] */
      GetClosestVisibleEnemyPosition(local_28,this);
                    /* Unresolved local var: Vector res@[???] */
      local_34 = local_28[0].x - (this->m_vecTargetPos).x;
      local_30 = local_28[0].y - (this->m_vecTargetPos).y;
      local_2c = local_28[0].z - (this->m_vecTargetPos).z;
      (this->m_vecTargetPos).x = local_28[0].x;
      (this->m_vecTargetPos).y = local_28[0].y;
      (this->m_vecTargetPos).z = local_28[0].z;
                    /* Unresolved local var: __m128 root@[???] */
      fVar13 = SQRT(local_34 * local_34 + local_30 * local_30 + local_2c * local_2c);
      if (*(float *)(unaff_EBX + 0x3a1f6d) <= fVar13 && fVar13 != *(float *)(unaff_EBX + 0x3a1f6d))
      goto LAB_0072acf0;
    }
    UpdateFacing(this);
    if ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
        super_CAI_BaseNPC.m_Activity != *(__178 *)(unaff_EBX + 0x69db05)) {
      (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                            super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                            super_CBaseAnimating + 0x684))(this,*(__178 *)(unaff_EBX + 0x69db05));
    }
    if (*(float *)(**(int **)(&DAT_0051bead + unaff_EBX) + 0xc) <=
        *(float *)(unaff_EBX + 0x3a1f5d) + this->m_flAimStartTime) {
      return;
    }
    piVar9 = (int *)(**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>
                                          ).super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                          super_CBaseCombatCharacter.super_CBaseFlex.
                                          super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x188))
                              (this);
    if (piVar9 == (int *)0x0) {
      return;
    }
    cVar5 = (**(code **)(*piVar9 + 0x114))(piVar9);
    if (cVar5 == '\0') {
      return;
    }
    cVar5 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                  super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                  super_CBaseCombatCharacter.super_CBaseFlex.
                                  super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x3d8))
                      (this,piVar9);
    cVar6 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                  super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                  super_CBaseCombatCharacter.super_CBaseFlex.
                                  super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x250))
                      (this,piVar9,0x4041,0);
    iVar8 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                  super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                  super_CBaseCombatCharacter.super_CBaseFlex.
                                  super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x3e0))
                      (this,piVar9);
    if ((iVar8 == 0) ||
       (cVar7 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                      super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                      super_CBaseCombatCharacter.super_CBaseFlex.
                                      super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x3d4))
                          (this,iVar8,piVar9,0x4041,0), cVar7 == '\0')) {
      bVar4 = false;
    }
    else {
      bVar4 = true;
    }
    if (((cVar5 != '\0') && (cVar6 != '\0')) || (bVar4)) {
      this->m_flAimStartTime = this->m_flAimStartTime - *(float *)(unaff_EBX + 0x3a318d);
      if (*(int *)(**(int **)(&DAT_0051bead + unaff_EBX) + 0x18) != this->m_muzzleToWorldTick) {
        this->m_muzzleToWorldTick = *(int *)(**(int **)(&DAT_0051bead + unaff_EBX) + 0x18);
        (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                              super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay
                              .super_CBaseAnimating + 0x364))
                  (this,this->m_iMuzzleAttachment,&this->m_muzzleToWorld);
      }
      local_78 = &this->m_muzzleToWorld;
      MatrixGetColumn(local_78,3,&local_40);
      MatrixGetColumn(local_78,0,&local_4c);
      ShootLaser(this,&local_40,&local_4c);
      if ((this->m_iLaserState).m_Value == 2) {
        return;
      }
      if (*(bool *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x60) == false) {
        this_00 = *(CBaseEdict **)
                   ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x24);
        if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
          this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
          pIVar10 = CBaseEdict::GetChangeAccessor(this_00);
          piVar9 = *(int **)(&DAT_0051beb1 + unaff_EBX);
          puVar3 = (ushort *)*piVar9;
          if (pIVar10->m_iChangeInfoSerialNumber == *puVar3) {
            uVar12 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar2 = puVar3[uVar12 * 0x14 + 0x14];
            if (uVar2 == 0) {
LAB_0072b0ff:
              puVar3[(uint)uVar2 + uVar12 * 0x14 + 1] = 0xf0c;
              puVar3[uVar12 * 0x14 + 0x14] = uVar2 + 1;
            }
            else if (puVar3[uVar12 * 0x14 + 1] != 0xf0c) {
              local_90 = 0;
              do {
                uVar11 = local_90 + 1;
                local_90 = uVar11 & 0xffff;
                if ((ushort)uVar11 == uVar2) {
                  if (uVar2 != 0x13) goto LAB_0072b0ff;
                  pIVar10->m_iChangeInfoSerialNumber = 0;
                  this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar3[uVar12 * 0x14 + local_90 + 1] != 0xf0c);
            }
          }
          else if (puVar3[0x7d1] == 100) {
            pIVar10->m_iChangeInfoSerialNumber = 0;
            this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar10->m_iChangeInfo = puVar3[0x7d1];
            *(short *)(*piVar9 + 0xfa2) = *(short *)(*piVar9 + 0xfa2) + 1;
            pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar9;
            iVar8 = *piVar9 + (uint)pIVar10->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar8 + 2) = 0xf0c;
            *(undefined2 *)(iVar8 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = (uint *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                super_CBaseCombatCharacter.super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating + 100);
        *puVar1 = *puVar1 | 1;
      }
      (this->m_iLaserState).m_Value = 2;
      return;
    }
  }
  SetFiringState(this,HOVER_TURRET_FIND_TARGET);
  return;
}


/* CNPC_HoverTurret::GetClosestVisibleEnemyPosition at 00724320 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Restarted to delay deadcode elimination for space: stack */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: Vector GetClosestVisibleEnemyPosition(CNPC_HoverTurret * this) */

Vector * __thiscall
CNPC_HoverTurret::GetClosestVisibleEnemyPosition
          (Vector *__return_storage_ptr__,CNPC_HoverTurret *this)

{
  uint uVar1;
  char *pcVar2;
  char *pcVar3;
  uint uVar4;
  VMatrix matThisToLinked;
  bool bVar5;
  char cVar6;
  char cVar7;
  int *piVar8;
  int iVar9;
  float *pfVar10;
  int iVar11;
  float *pfVar12;
  matrix3x4_t *in2;
  uint uVar13;
  uint uVar14;
  int iVar15;
  int unaff_EBX;
  uint *puVar16;
  undefined4 *puVar17;
  undefined4 *puVar18;
  float fVar19;
  float fVar20;
  CNPC_HoverTurret *pCVar21;
  undefined4 uVar22;
  undefined4 uVar23;
  Vector *out;
  undefined4 uVar24;
  undefined1 in_stack_fffffea4 [36];
  Vector *ptSource;
  Vector *ptTransformed;
  CBaseEntity *pEnemy;
  QAngle local_b8;
  float local_ac;
  float local_a8;
  float local_a4;
  Vector local_a0;
  Vector local_94;
  float local_88;
  float local_84;
  float local_80;
  Vector local_7c [2];
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  float local_4c;
  float local_48;
  float local_44;
  float local_40;
  float local_3c;
  float local_38;
  float local_34;
  float local_30;
  float local_2c;
  Vector local_28 [2];
  
                    /* Unresolved local var: Vector vecMuzzlePos@[???]
                       Unresolved local var: Vector vecMidEnemy@[???]
                       Unresolved local var: Vector vecMid@[???]
                       Unresolved local var: Vector vecBody@[???]
                       Unresolved local var: bool bEnemyInFOV@[???]
                       Unresolved local var: bool bEnemyVisible@[???]
                       Unresolved local var: Vector vecDirToEnemy@[???]
                       Unresolved local var: float flDistToEnemy@[???]
                       Unresolved local var: CPortal_Base2D * pPortal@[???]
                       Unresolved local var: QAngle vecAnglesToEnemy@[???] */
  ___i686_get_pc_thunk_bx();
  piVar8 = (int *)(**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                        super_CBaseCombatCharacter.super_CBaseFlex.
                                        super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x188))()
  ;
  iVar11 = *(int *)(**(int **)(unaff_EBX + 0x522833) + 0x18);
  if (iVar11 != this->m_muzzleToWorldTick) {
    this->m_muzzleToWorldTick = iVar11;
    (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x364))();
  }
  out = local_28;
  MatrixGetColumn(&this->m_muzzleToWorld,3,out);
  uVar24 = 0;
  (**(code **)(*piVar8 + 0x230))();
                    /* Unresolved local var: Vector res@[???] */
  local_38 = *(float *)(unaff_EBX + 0x3a898b);
  local_40 = local_34 * local_38;
  local_3c = local_30 * local_38;
  local_38 = local_38 * local_2c;
  pcVar2 = *(char **)(unaff_EBX + 0x640f8f);
  if ((*pcVar2 == '\0') && (iVar11 = ___cxa_guard_acquire(), iVar11 != 0)) {
    ___cxa_guard_release();
  }
  pcVar3 = *(char **)(unaff_EBX + 0x640f93);
  if ((*pcVar3 == '\0') && (iVar11 = ___cxa_guard_acquire(), iVar11 != 0)) {
    *(undefined4 *)(unaff_EBX + 0x6607af) = 0;
    ___cxa_guard_release();
  }
  do {
    puVar16 = (uint *)(unaff_EBX + 0x6607af);
    uVar4 = *(uint *)(unaff_EBX + 0x6607af);
    uVar13 = uVar4 + 0x10001 & 0x7f;
    LOCK();
    uVar1 = *puVar16;
    if (uVar4 == uVar1) {
      *puVar16 = uVar13;
    }
    UNLOCK();
  } while (uVar4 != uVar1);
  if ((*pcVar2 == '\0') && (iVar11 = ___cxa_guard_acquire(), iVar11 != 0)) {
    ___cxa_guard_release();
  }
  if ((*pcVar3 == '\0') && (iVar11 = ___cxa_guard_acquire(), iVar11 != 0)) {
    *(undefined4 *)(unaff_EBX + 0x6607af) = 0;
    ___cxa_guard_release();
  }
  do {
    uVar4 = *(uint *)(unaff_EBX + 0x6607af);
    uVar14 = uVar4 + 0x10001 & 0x7f;
    LOCK();
    uVar1 = *puVar16;
    if (uVar4 == uVar1) {
      *puVar16 = uVar14;
    }
    UNLOCK();
  } while (uVar4 != uVar1);
  iVar11 = unaff_EBX + 0x65fc8f;
  iVar9 = uVar14 * 0xc;
  fVar19 = *(float *)(unaff_EBX + 0x3a87cf);
  *(float *)(iVar9 + iVar11) =
       (float)piVar8[0x60] + ((float)piVar8[99] - (float)piVar8[0x60]) * fVar19;
  *(float *)(iVar9 + 4 + iVar11) =
       (float)piVar8[0x61] + ((float)piVar8[100] - (float)piVar8[0x61]) * fVar19;
  *(float *)(iVar9 + 8 + iVar11) =
       (float)piVar8[0x62] + ((float)piVar8[0x65] - (float)piVar8[0x62]) * fVar19;
  if ((((*(byte *)(piVar8 + 0x66) & 0x40) == 0) && (*(char *)((int)piVar8 + 0x19a) != '\x02')) &&
     (*(char *)((int)piVar8 + 0x19a) != '\0')) {
    pfVar12 = (float *)(**(code **)(piVar8[0x5e] + 0x24))();
    pfVar10 = *(float **)(unaff_EBX + 0x5228d7);
    if ((*pfVar10 == *pfVar12) && (!NAN(*pfVar10) && !NAN(*pfVar12))) {
      if ((pfVar10[1] == pfVar12[1]) && (!NAN(pfVar10[1]) && !NAN(pfVar12[1]))) {
        if ((pfVar10[2] == pfVar12[2]) && (!NAN(pfVar10[2]) && !NAN(pfVar12[2]))) goto LAB_00724566;
      }
    }
    in2 = (matrix3x4_t *)(**(code **)(piVar8[0x5e] + 0x28))();
    VectorTransform((float *)(iVar11 + uVar14 * 0xc),in2,(float *)(iVar11 + uVar13 * 0xc));
  }
  else {
LAB_00724566:
    pfVar10 = (float *)(**(code **)(piVar8[0x5e] + 0x20))();
    iVar9 = uVar13 * 0xc;
    iVar15 = uVar14 * 0xc;
    *(float *)(iVar9 + iVar11) = *(float *)(iVar15 + iVar11) + *pfVar10;
    *(float *)(iVar9 + 4 + iVar11) = *(float *)(iVar15 + 4 + iVar11) + pfVar10[1];
    *(float *)(iVar9 + 8 + iVar11) = *(float *)(iVar15 + 8 + iVar11) + pfVar10[2];
  }
                    /* Unresolved local var: Vector res@[???] */
  iVar9 = uVar13 * 0xc;
  fVar19 = *(float *)(unaff_EBX + 0x3ab897);
                    /* Unresolved local var: Vector res@[???] */
  local_4c = local_40 + *(float *)(iVar9 + iVar11) * fVar19;
  local_48 = *(float *)(iVar9 + 4 + iVar11) * fVar19 + local_3c;
  local_44 = fVar19 * *(float *)(iVar9 + 8 + iVar11) + local_38;
  __return_storage_ptr__->x = local_4c;
  __return_storage_ptr__->y = local_48;
  __return_storage_ptr__->z = local_44;
  (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x220))()
  ;
  local_58 = local_64;
  local_54 = local_60;
  local_50 = local_5c;
  (**(code **)(*piVar8 + 0x230))();
  cVar6 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                super_CBaseCombatCharacter.super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x3d8))();
  cVar7 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                super_CBaseCombatCharacter.super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x250))();
  if (cVar7 != '\0') {
    cVar7 = (**(code **)(*piVar8 + 0x114))();
    bVar5 = true;
    if (cVar7 != '\0') goto LAB_007246fd;
  }
  bVar5 = false;
LAB_007246fd:
                    /* Unresolved local var: Vector res@[???] */
  local_88 = __return_storage_ptr__->x - local_28[0].x;
  local_84 = __return_storage_ptr__->y - local_28[0].y;
  local_80 = __return_storage_ptr__->z - local_28[0].z;
  local_7c[0].x = local_88;
  local_7c[0].y = local_84;
  local_7c[0].z = local_80;
  fVar19 = VectorNormalize(local_7c);
  cVar7 = (**(code **)(*piVar8 + 0x114))();
  if ((cVar7 != '\0') &&
     (iVar11 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                     super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                     super_CBaseCombatCharacter.super_CBaseFlex.
                                     super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x3e0))(),
     iVar11 != 0)) {
    uVar23 = 0;
    uVar22 = 0x4041;
    pCVar21 = this;
    iVar9 = iVar11;
    cVar7 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                  super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                  super_CBaseCombatCharacter.super_CBaseFlex.
                                  super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x3d4))();
    if (cVar7 != '\0') {
      uVar1 = *(uint *)(iVar11 + 0x524);
      if ((uVar1 == 0xffffffff) ||
         (iVar11 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x52283b),
         *(uint *)(iVar11 + 8) != uVar1 >> 0x10)) {
        iVar11 = 0;
      }
      else {
        iVar11 = *(int *)(iVar11 + 4);
      }
      ptTransformed = &local_94;
      puVar17 = (undefined4 *)(iVar11 + 0x528);
      puVar18 = (undefined4 *)&stack0xfffffe88;
      ptSource = __return_storage_ptr__;
      for (iVar15 = 0x10; iVar15 != 0; iVar15 = iVar15 + -1) {
        *puVar18 = *puVar17;
        puVar17 = puVar17 + 1;
        puVar18 = puVar18 + 1;
      }
      matThisToLinked.m[0][1] = (vec_t)iVar9;
      matThisToLinked.m[0][0] = (vec_t)pCVar21;
      matThisToLinked.m[0][2] = (vec_t)piVar8;
      matThisToLinked.m[0][3] = (vec_t)uVar22;
      matThisToLinked.m[1][0] = (vec_t)uVar23;
      matThisToLinked.m[1][1] = (vec_t)out;
      matThisToLinked.m[1][2] = (vec_t)uVar24;
      matThisToLinked.m[1][3] = (vec_t)in_stack_fffffea4._0_4_;
      matThisToLinked.m[2][0] = (vec_t)in_stack_fffffea4._4_4_;
      matThisToLinked.m[2][1] = (vec_t)in_stack_fffffea4._8_4_;
      matThisToLinked.m[2][2] = (vec_t)in_stack_fffffea4._12_4_;
      matThisToLinked.m[2][3] = (vec_t)in_stack_fffffea4._16_4_;
      matThisToLinked.m[3][0] = (vec_t)in_stack_fffffea4._20_4_;
      matThisToLinked.m[3][1] = (vec_t)in_stack_fffffea4._24_4_;
      matThisToLinked.m[3][2] = (vec_t)in_stack_fffffea4._28_4_;
      matThisToLinked.m[3][3] = (vec_t)in_stack_fffffea4._32_4_;
      UTIL_Portal_PointTransform(matThisToLinked,ptSource,ptTransformed);
                    /* Unresolved local var: Vector res@[???] */
      local_ac = local_94.x - local_28[0].x;
      local_a8 = local_94.y - local_28[0].y;
      local_a4 = local_94.z - local_28[0].z;
      local_a0.x = local_ac;
      local_a0.y = local_a8;
      local_a0.z = local_a4;
      fVar20 = VectorNormalize(&local_a0);
      if (((cVar6 == '\0') || (!bVar5)) || (fVar20 < fVar19)) {
        __return_storage_ptr__->x = local_94.x;
        __return_storage_ptr__->y = local_94.y;
        __return_storage_ptr__->z = local_94.z;
        local_7c[0].x = local_a0.x;
        local_7c[0].y = local_a0.y;
        local_7c[0].z = local_a0.z;
      }
    }
  }
  VectorNormalize(local_7c);
  VectorAngles(local_7c,&local_b8);
  (this->m_vecGoalAngles).x = local_b8.x;
  (this->m_vecGoalAngles).y = local_b8.y;
  (this->m_vecGoalAngles).z = local_b8.z;
  return __return_storage_ptr__;
}


/* CNPC_HoverTurret::TraceAttack at 00720e40 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void TraceAttack(CNPC_HoverTurret * this, CTakeDamageInfo * info,
   Vector * vecDir, trace_t * ptr) */

void __thiscall
CNPC_HoverTurret::TraceAttack
          (CNPC_HoverTurret *this,CTakeDamageInfo *info,Vector *vecDir,trace_t *ptr)

{
  CAI_BaseNPC::TraceAttack((CAI_BaseNPC *)this,info,vecDir,ptr);
  return;
}


/* CNPC_HoverTurret::StartDeathSequence at 007206b0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void StartDeathSequence(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::StartDeathSequence(CNPC_HoverTurret *this)

{
  char *soundname;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((this->m_flDeathTime == 0.0) && (!NAN(this->m_flDeathTime))) {
    this->m_flDeathTime =
         *(float *)(**(int **)(unaff_EBX + 0x5264a0) + 0xc) + *(float *)(unaff_EBX + 0x3ad6e4);
    soundname = (char *)(unaff_EBX + 0x3950cc);
    CBaseEntity::EmitSound((CBaseEntity *)this,soundname,0.0,(float *)0x0);
    CBaseEntity::EmitSound
              ((CBaseEntity *)this,soundname,
               *(float *)(unaff_EBX + 0x3ac43c) + *(float *)(**(int **)(unaff_EBX + 0x5264a0) + 0xc)
               ,(float *)0x0);
    CBaseEntity::EmitSound
              ((CBaseEntity *)this,soundname,
               *(float *)(unaff_EBX + 0x3ac24c) + *(float *)(**(int **)(unaff_EBX + 0x5264a0) + 0xc)
               ,(float *)0x0);
    CBaseEntity::EmitSound
              ((CBaseEntity *)this,soundname,
               *(float *)(unaff_EBX + 0x3ac550) + *(float *)(**(int **)(unaff_EBX + 0x5264a0) + 0xc)
               ,(float *)0x0);
    CBaseEntity::EmitSound
              ((CBaseEntity *)this,(char *)(unaff_EBX + 0x38b94c),
               *(float *)(&DAT_003ac80c + unaff_EBX) +
               *(float *)(**(int **)(unaff_EBX + 0x5264a0) + 0xc),(float *)0x0);
  }
  return;
}


/* CNPC_HoverTurret::TakeDamageFromPhysicsImpact at 00720bc0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void TakeDamageFromPhysicsImpact(CNPC_HoverTurret * this, int index,
   gamevcollisionevent_t * pEvent) */

void __thiscall
CNPC_HoverTurret::TakeDamageFromPhysicsImpact
          (CNPC_HoverTurret *this,int index,gamevcollisionevent_t *pEvent)

{
  CBaseEntity *this_00;
  IPhysicsCollisionData *pIVar1;
  IPhysicsObject *pIVar2;
  float *pfVar3;
  code *pcVar4;
  bool bVar5;
  int unaff_EBX;
  bool bVar6;
  longdouble extraout_ST0;
  longdouble extraout_ST0_00;
  CTakeDamageInfo *in_stack_ffffff18;
  CBaseEntity *in_stack_ffffff1c;
  CBaseEntity *in_stack_ffffff20;
  Vector *in_stack_ffffff24;
  Vector *in_stack_ffffff28;
  float in_stack_ffffff2c;
  float bitsDamageType_1;
  int in_stack_ffffff34;
  Vector *in_stack_ffffff38;
  CBaseEntity *pHitEntity;
  float damage;
  CTakeDamageInfo local_a4;
  float local_54;
  float local_50;
  float local_4c;
  float local_48;
  float local_44;
  float local_40;
  Vector local_3c;
  Vector local_30;
  char *local_24;
  int damageType;
  
                    /* Unresolved local var: float flDamageScale@[???]
                       Unresolved local var: Vector damagePos@[???]
                       Unresolved local var: Vector damageForce@[???] */
  ___i686_get_pc_thunk_bx();
  bVar6 = index == 0;
  this_00 = pEvent->pEntities[bVar6];
  damageType = 0;
  damage = CalculateDefaultPhysicsDamage
                     (index,pEvent,
                      *(float *)(unaff_EBX + 0x3abf63) *
                      (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                      super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                      m_impactEnergyScale,true,&damageType,(string_t)0x0,false);
  if ((this_00 == (CBaseEntity *)0x0) ||
     ((local_24 = (char *)(unaff_EBX + 0x3351a5), (this_00->m_iClassname).pszValue != local_24 &&
      (bVar5 = CBaseEntity::ClassMatchesComplex(this_00,local_24), !bVar5)))) {
    if ((damage == *(float *)(unaff_EBX + 0x3abf9f)) &&
       (!NAN(damage) && !NAN(*(float *)(unaff_EBX + 0x3abf9f)))) {
      return;
    }
  }
  else {
    damage = *(float *)(unaff_EBX + 0x3ac047);
  }
  pIVar1 = (pEvent->super_vcollisionevent_t).pInternalData;
  (*pIVar1->_vptr_IPhysicsCollisionData[1])(pIVar1,&local_30);
  pIVar2 = (pEvent->super_vcollisionevent_t).pObjects[index];
  (*pIVar2->_vptr_IPhysicsObject[0x1e])(pIVar2);
  bitsDamageType_1 = (float)extraout_ST0;
                    /* Unresolved local var: Vector res@[???] */
  local_48 = bitsDamageType_1 * pEvent->postVelocity[index].x;
  local_44 = bitsDamageType_1 * pEvent->postVelocity[index].y;
  local_40 = bitsDamageType_1 * pEvent->postVelocity[index].z;
  pfVar3 = *(float **)(unaff_EBX + 0x525fa7);
  local_3c.x = local_48;
  local_3c.y = local_44;
  local_3c.z = local_40;
  if ((*pfVar3 == local_48) && (!NAN(*pfVar3) && !NAN(local_48))) {
    if ((pfVar3[1] == local_44) && (!NAN(pfVar3[1]) && !NAN(local_44))) {
      if ((pfVar3[2] == local_40) && (!NAN(pfVar3[2]) && !NAN(local_40))) {
        pIVar2 = (pEvent->super_vcollisionevent_t).pObjects[bVar6];
        (*pIVar2->_vptr_IPhysicsObject[0x1e])(pIVar2);
        local_4c = (float)extraout_ST0_00;
                    /* Unresolved local var: Vector res@[???] */
        local_54 = local_4c * pEvent->postVelocity[bVar6].x;
        local_50 = local_4c * pEvent->postVelocity[bVar6].y;
        local_4c = local_4c * pEvent->postVelocity[bVar6].z;
        local_3c.x = local_54;
        local_3c.y = local_50;
        local_3c.z = local_4c;
      }
    }
  }
  if (damage < *(float *)(unaff_EBX + 0x3acd13) || damage == *(float *)(unaff_EBX + 0x3acd13)) {
    return;
  }
  pcVar4 = *(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                               super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                               super_CBaseCombatCharacter.super_CBaseFlex.
                               super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x118);
  CTakeDamageInfo::CTakeDamageInfo
            (&local_a4,this_00,this_00,&local_3c,&local_30,damage,damageType,0,(Vector *)0x0,
             in_stack_ffffff18,in_stack_ffffff1c,in_stack_ffffff20,in_stack_ffffff24,
             in_stack_ffffff28,in_stack_ffffff2c,(int)bitsDamageType_1,in_stack_ffffff34,
             in_stack_ffffff38);
  (*pcVar4)(this,&local_a4);
  return;
}


/* CNPC_HoverTurret::VPhysicsCollision at 0072a960 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void VPhysicsCollision(CNPC_HoverTurret * this, int index,
   gamevcollisionevent_t * pEvent) */

void __thiscall
CNPC_HoverTurret::VPhysicsCollision(CNPC_HoverTurret *this,int index,gamevcollisionevent_t *pEvent)

{
  int *piVar1;
  char cVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: int otherIndex@[???]
                       Unresolved local var: CBaseEntity * pHitEntity@[???] */
  ___i686_get_pc_thunk_bx();
  if (0 < ((CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth> *)
          ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                 super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                 super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0xe8))->m_Value)
  {
    CBaseEntity::VPhysicsCollision((CBaseEntity *)this,index,pEvent);
    piVar1 = *(int **)((int)pEvent->pEntities + (-(uint)(index == 0) & 4));
    (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
    super_CAI_BaseNPC.super_CBaseCombatCharacter.m_flFieldOfView = -1.0;
    if (piVar1 != (int *)0x0) {
      cVar2 = (**(code **)(*piVar1 + 0x154))(piVar1);
      if (cVar2 == '\0') {
        if ((piVar1 != (int *)**(int **)(unaff_EBX + 0x51c2ac)) &&
           (this->m_flEngineStallTime =
                 *(float *)(**(int **)(unaff_EBX + 0x51c1f0) + 0xc) +
                 *(float *)(unaff_EBX + 0x3a21c4),
           this->m_iFiringState != HOVER_TURRET_SHOT_DISABLED)) {
          SetFiringState(this,HOVER_TURRET_HIT_WITH_PHYSICS);
        }
        TakeDamageFromPhysicsImpact(this,index,pEvent);
        return;
      }
      if (this->m_iFiringState != HOVER_TURRET_AIM_TARGET) {
        SetFiringState(this,HOVER_TURRET_FIND_TARGET);
        this->m_flEngineStallTime =
             *(float *)(**(int **)(unaff_EBX + 0x51c1f0) + 0xc) + *(float *)(unaff_EBX + 0x3a218c);
      }
    }
  }
  return;
}


/* CNPC_HoverTurret::MaintainGroundHeight at 00720350 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void MaintainGroundHeight(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::MaintainGroundHeight(CNPC_HoverTurret *this)

{
  return;
}


/* CNPC_HoverTurret::TurnHeadToTarget at 00720f70 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void TurnHeadToTarget(CNPC_HoverTurret * this, float flInterval, Vector
   * MoveTarget) */

void __thiscall
CNPC_HoverTurret::TurnHeadToTarget(CNPC_HoverTurret *this,float flInterval,Vector *MoveTarget)

{
  float current;
  int unaff_EBX;
  longdouble lVar1;
  float target;
  float fVar2;
  float fVar3;
  float flDestYaw;
  float newYaw;
  Vector local_28 [2];
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: Vector res@[???] */
  local_28[0].x =
       MoveTarget->x -
       (((CNetworkVectorXY_SeparateZBase<Vector,CBaseEntity::NetworkVar_m_vecOrigin> *)
        ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
               super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
               super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x350))->
       super_CNetworkVectorCommonBase<Vector,CBaseEntity::NetworkVar_m_vecOrigin>).
       super_CNetworkVarBase<Vector,CBaseEntity::NetworkVar_m_vecOrigin>.m_Value.x;
  local_28[0].y =
       MoveTarget->y -
       *(vec_t *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x354);
  local_28[0].z =
       MoveTarget->z -
       *(vec_t *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x358);
  target = CAI_BaseNPC::VecToYaw((CAI_BaseNPC *)this,local_28);
  fVar3 = *(float *)(**(int **)(unaff_EBX + 0x525be0) + 0xc);
  fVar2 = CBaseEntity::GetLastThink((CBaseEntity *)this,(char *)0x0);
  current = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.m_fHeadYaw;
  lVar1 = (longdouble)
          (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                super_CBaseCombatCharacter.super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x9c8))(this);
  fVar3 = AI_ClampYaw((float)lVar1 * *(float *)(unaff_EBX + 0x3abb88),current,target,fVar3 - fVar2);
  if (fVar3 != (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
               super_CAI_BasePhysicsFlyingBot.m_fHeadYaw) {
    (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
    m_fHeadYaw = fVar3;
  }
  return;
}


/* CNPC_HoverTurret::UpdateFacing at 00721230 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: bool UpdateFacing(CNPC_HoverTurret * this) */

bool __thiscall CNPC_HoverTurret::UpdateFacing(CNPC_HoverTurret *this)

{
  int iVar1;
  model_t *pmVar2;
  CStudioHdr *pCVar3;
  int unaff_EBX;
  longdouble lVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  QAngle local_40;
  Vector local_34;
  Vector local_28 [2];
  
                    /* Unresolved local var: bool bMoved@[???]
                       Unresolved local var: Vector vecGoalDir@[???]
                       Unresolved local var: Vector vecGoalLocalDir@[???]
                       Unresolved local var: QAngle vecGoalLocalAngles@[???]
                       Unresolved local var: float flDiff@[???] */
  ___i686_get_pc_thunk_bx();
  iVar1 = *(int *)(**(int **)(unaff_EBX + 0x525926) + 0x18);
  if (iVar1 != this->m_muzzleToWorldTick) {
    this->m_muzzleToWorldTick = iVar1;
    (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x364))
              (this,this->m_iMuzzleAttachment,&this->m_muzzleToWorld);
  }
  AngleVectors(&this->m_vecGoalAngles,local_28);
  VectorIRotate(&local_28[0].x,&this->m_muzzleToWorld,&local_34.x);
  VectorAngles(&local_34,&local_40);
  lVar4 = (longdouble)
          (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                super_CBaseCombatCharacter.super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x7a8))(this);
  fVar5 = ApproachAngle(local_40.x,0.0,(float)lVar4 * *(float *)(unaff_EBX + 0x3aba82));
  fVar5 = AngleNormalize(fVar5);
  fVar6 = CBaseAnimating::GetPoseParameter
                    ((CBaseAnimating *)this,
                     (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                     super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.m_poseAim_Pitch);
  iVar1 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .super_CAI_BaseNPC.m_poseAim_Pitch;
  pCVar3 = *(CStudioHdr **)
            ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                   super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                   super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x500);
  if (pCVar3 == (CStudioHdr *)0x0) {
    pmVar2 = CBaseEntity::GetModel((CBaseEntity *)this);
    if (pmVar2 == (model_t *)0x0) {
      pCVar3 = *(CStudioHdr **)
                ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                       super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                       super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x500);
    }
    else {
      CBaseAnimating::LockStudioHdr((CBaseAnimating *)this);
      pCVar3 = *(CStudioHdr **)
                ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                       super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                       super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x500);
    }
    if (pCVar3 != (CStudioHdr *)0x0) goto LAB_00721334;
  }
  else {
LAB_00721334:
    if (pCVar3->m_pStudioHdr == (studiohdr_t *)0x0) {
      pCVar3 = (CStudioHdr *)0x0;
    }
  }
  CBaseAnimating::SetPoseParameter
            ((CBaseAnimating *)this,pCVar3,iVar1,fVar5 / *(float *)(unaff_EBX + 0x3acb6a) + fVar6);
  lVar4 = (longdouble)
          (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                super_CBaseCombatCharacter.super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x7a8))(this);
  fVar6 = ApproachAngle(local_40.y,0.0,(float)lVar4 * *(float *)(unaff_EBX + 0x3acc06));
  fVar6 = AngleNormalize(fVar6);
  fVar7 = CBaseAnimating::GetPoseParameter
                    ((CBaseAnimating *)this,
                     (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                     super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.m_poseAim_Yaw);
  iVar1 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .super_CAI_BaseNPC.m_poseAim_Yaw;
  pCVar3 = *(CStudioHdr **)
            ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                   super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                   super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x500);
  if (pCVar3 == (CStudioHdr *)0x0) {
    pmVar2 = CBaseEntity::GetModel((CBaseEntity *)this);
    if (pmVar2 == (model_t *)0x0) {
      pCVar3 = *(CStudioHdr **)
                ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                       super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                       super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x500);
    }
    else {
      CBaseAnimating::LockStudioHdr((CBaseAnimating *)this);
      pCVar3 = *(CStudioHdr **)
                ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                       super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                       super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x500);
    }
    if (pCVar3 == (CStudioHdr *)0x0) goto LAB_007213ec;
  }
  if (pCVar3->m_pStudioHdr == (studiohdr_t *)0x0) {
    pCVar3 = (CStudioHdr *)0x0;
  }
LAB_007213ec:
  CBaseAnimating::SetPoseParameter
            ((CBaseAnimating *)this,pCVar3,iVar1,fVar6 / *(float *)(unaff_EBX + 0x3acb6a) + fVar7);
  fVar7 = *(float *)(unaff_EBX + 0x3ab6d6);
  (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x370))
            (this);
  return fVar7 < ABS(fVar6) || fVar7 < ABS(fVar5);
}


/* CNPC_HoverTurret::UpdateMuzzleMatrix at 00720360 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void UpdateMuzzleMatrix(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::UpdateMuzzleMatrix(CNPC_HoverTurret *this)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  iVar1 = *(int *)(**(int **)(unaff_EBX + 0x5267f8) + 0x18);
  if (iVar1 != this->m_muzzleToWorldTick) {
    this->m_muzzleToWorldTick = iVar1;
    (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x364))
              (this,this->m_iMuzzleAttachment,&this->m_muzzleToWorld);
  }
  return;
}


/* CNPC_HoverTurret::OverrideMove at 007218a0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: bool OverrideMove(CNPC_HoverTurret * this, float flInterval) */

bool __thiscall CNPC_HoverTurret::OverrideMove(CNPC_HoverTurret *this,float flInterval)

{
  float *pfVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  IPhysicsObject *pIVar9;
  uint uVar10;
  vec_t *pvVar11;
  int iVar12;
  IPhysicsMotionController *pIVar13;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
      super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
      super_CBaseAnimating.super_CBaseEntity.m_iHealth.m_Value < 1) {
    MoveExecute_Dead(this,flInterval);
    return true;
  }
  if (this->m_bInitialPositionSet == false) {
    if (((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
         super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
         super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
    (this->m_vForceMoveTarget).x =
         (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
         super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
         super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.x;
    (this->m_vForceMoveTarget).y =
         (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
         super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
         super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.y;
    (this->m_vForceMoveTarget).z =
         (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
         super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
         super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z + *(float *)(unaff_EBX + 0x3b3ce6);
    this->m_bInitialPositionSet = true;
  }
  pIVar9 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
           m_pPhysicsObject;
  if (pIVar9 != (IPhysicsObject *)0x0) {
    (*pIVar9->_vptr_IPhysicsObject[0x19])(pIVar9);
  }
  pfVar1 = (float *)(**(int **)(unaff_EBX + 0x5252b6) + 0xc);
  if ((this->m_flEngineStallTime < *pfVar1 || this->m_flEngineStallTime == *pfVar1) &&
     (this->m_iFiringState != HOVER_TURRET_SHOT_DISABLED)) {
    if ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
        m_pMotionController == (IPhysicsMotionController *)0x0) {
      pIVar13 = (IPhysicsMotionController *)
                (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52534e) + 0x78))
                          ((int *)**(undefined4 **)(unaff_EBX + 0x52534e),
                           &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                            super_CAI_BasePhysicsFlyingBot.super_IMotionEvent);
      (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
      m_pMotionController = pIVar13;
      (*pIVar13->_vptr_IPhysicsMotionController[3])
                (pIVar13,(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                         super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter
                         .super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                         super_CBaseEntity.m_pPhysicsObject,1);
    }
    (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x9c4))(this,flInterval,&this->m_vForceMoveTarget);
    CBaseEntity::PhysicsCheckWaterTransition((CBaseEntity *)this);
    fVar2 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.m_vCurrentVelocity.y;
    fVar3 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.m_vCurrentVelocity.x;
    fVar4 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.m_vCurrentVelocity.z;
    pIVar9 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
             m_pPhysicsObject;
    if (pIVar9 != (IPhysicsObject *)0x0) {
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
      uVar10 = (this->m_hTether).super_CBaseHandle.m_Index;
      if (((uVar10 != 0xffffffff) &&
          (iVar12 = (uVar10 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5252be),
          *(uint *)(iVar12 + 8) == uVar10 >> 0x10)) && (*(int *)(iVar12 + 4) != 0)) {
        (*pIVar9->_vptr_IPhysicsObject[0x19])(pIVar9);
      }
    }
    fVar5 = *(float *)(unaff_EBX + 0x3ac1be);
    fVar6 = *(float *)(*(int *)(*(int *)(unaff_EBX + 0x52541a) + 0x1c) + 0x2c);
                    /* Unresolved local var: Vector res@[???] */
    fVar7 = (this->m_vForceVelocity).y;
    fVar8 = (this->m_vForceVelocity).z;
    (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
    m_vCurrentVelocity.x = fVar3 + (this->m_vForceVelocity).x;
    (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
    m_vCurrentVelocity.y = fVar2 + fVar7;
    (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
    m_vCurrentVelocity.z = flInterval * fVar5 * fVar6 + fVar4 + fVar8;
    pvVar11 = *(vec_t **)(unaff_EBX + 0x5252ca);
    (this->m_vForceVelocity).x = *pvVar11;
    (this->m_vForceVelocity).y = pvVar11[1];
    (this->m_vForceVelocity).z = pvVar11[2];
    return true;
  }
  return false;
}


/* CNPC_HoverTurret::MoveExecute_Alive at 00720a80 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void MoveExecute_Alive(CNPC_HoverTurret * this, float flInterval) */

void __thiscall CNPC_HoverTurret::MoveExecute_Alive(CNPC_HoverTurret *this,float flInterval)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  IPhysicsObject *pIVar8;
  uint uVar9;
  vec_t *pvVar10;
  int iVar11;
  int unaff_EBX;
  
                    /* Unresolved local var: Vector vCurrentVelocity@[???] */
  ___i686_get_pc_thunk_bx();
  CBaseEntity::PhysicsCheckWaterTransition((CBaseEntity *)this);
  fVar1 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .m_vCurrentVelocity.y;
  fVar2 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .m_vCurrentVelocity.x;
  fVar3 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .m_vCurrentVelocity.z;
  pIVar8 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
           m_pPhysicsObject;
  if (pIVar8 != (IPhysicsObject *)0x0) {
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
    uVar9 = (this->m_hTether).super_CBaseHandle.m_Index;
    if (((uVar9 != 0xffffffff) &&
        (iVar11 = (uVar9 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5260d8),
        *(uint *)(iVar11 + 8) == uVar9 >> 0x10)) && (*(int *)(iVar11 + 4) != 0)) {
      (*pIVar8->_vptr_IPhysicsObject[0x19])(pIVar8);
    }
  }
  fVar4 = *(float *)(unaff_EBX + 0x3acfd8);
  fVar5 = *(float *)(*(int *)(*(int *)(unaff_EBX + 0x526234) + 0x1c) + 0x2c);
                    /* Unresolved local var: Vector res@[???] */
  fVar6 = (this->m_vForceVelocity).y;
  fVar7 = (this->m_vForceVelocity).z;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vCurrentVelocity.x = fVar2 + (this->m_vForceVelocity).x;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vCurrentVelocity.y = fVar1 + fVar6;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vCurrentVelocity.z = flInterval * fVar4 * fVar5 + fVar3 + fVar7;
  pvVar10 = *(vec_t **)(unaff_EBX + 0x5260e4);
  (this->m_vForceVelocity).x = *pvVar10;
  (this->m_vForceVelocity).y = pvVar10[1];
  (this->m_vForceVelocity).z = pvVar10[2];
  return;
}


/* CNPC_HoverTurret::MoveExecute_Dead at 00721700 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void MoveExecute_Dead(CNPC_HoverTurret * this, float flInterval) */

void __thiscall CNPC_HoverTurret::MoveExecute_Dead(CNPC_HoverTurret *this,float flInterval)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  code *pcVar6;
  vec_t *pvVar7;
  int *piVar8;
  int unaff_EBX;
  longdouble lVar9;
  undefined1 local_34 [12];
  undefined1 local_28 [24];
  
                    /* Unresolved local var: Vector newVelocity@[???]
                       Unresolved local var: Vector forward@[???]
                       Unresolved local var: Vector up@[???] */
  ___i686_get_pc_thunk_bx();
  fVar1 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .m_vCurrentVelocity.z;
  fVar2 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .m_vCurrentVelocity.y;
  fVar3 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .m_vCurrentVelocity.x;
  (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x238))
            (this,local_28,0,local_34);
  fVar4 = *(float *)(**(int **)(unaff_EBX + 0x525456) + 0xc);
  if (this->m_fSparkTime <= fVar4 && fVar4 != this->m_fSparkTime) {
    piVar8 = (int *)**(undefined4 **)(unaff_EBX + 0x525586);
    pcVar6 = *(code **)(*piVar8 + 0x10);
    if ((*(byte *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                         super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter
                         .super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x12d)
        & 8) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
      piVar8 = (int *)**(undefined4 **)(unaff_EBX + 0x525586);
    }
    (*pcVar6)(piVar8,(Vector *)
                     ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                            super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                            super_CBaseAnimating + 0x294),1,1,0);
    fVar4 = *(float *)(**(int **)(unaff_EBX + 0x525456) + 0xc);
    lVar9 = (longdouble)
            (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5254da) + 4))
                      ((int *)**(undefined4 **)(unaff_EBX + 0x5254da),0x3d4ccccd,0x3dcccccd);
    this->m_fSparkTime = fVar4 + (float)lVar9;
  }
  fVar4 = (this->m_vForceVelocity).y;
  fVar5 = (this->m_vForceVelocity).z;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vCurrentVelocity.x = fVar3 + (this->m_vForceVelocity).x;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vCurrentVelocity.y = fVar2 + fVar4;
  (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
  m_vCurrentVelocity.z = fVar1 + fVar5;
  pvVar7 = *(vec_t **)(unaff_EBX + 0x52546a);
  (this->m_vForceVelocity).x = *pvVar7;
  (this->m_vForceVelocity).y = pvVar7[1];
  (this->m_vForceVelocity).z = pvVar7[2];
  return;
}


/* CNPC_HoverTurret::OnPhysGunPickup at 0072a8e0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void OnPhysGunPickup(CNPC_HoverTurret * this, CBasePlayer *
   pPhysGunUser, PhysGunPickup_t reason) */

void __thiscall
CNPC_HoverTurret::OnPhysGunPickup
          (CNPC_HoverTurret *this,CBasePlayer *pPhysGunUser,PhysGunPickup_t reason)

{
  IPhysicsMotionController *pIVar1;
  char cVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  cVar2 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                super_CBaseCombatCharacter.super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x114))(this);
  if (cVar2 != '\0') {
    pIVar1 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
             super_CAI_BasePhysicsFlyingBot.m_pMotionController;
    if (pIVar1 != (IPhysicsMotionController *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x51c30b) + 0x7c))
                ((int *)**(undefined4 **)(unaff_EBX + 0x51c30b),pIVar1);
      (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
      m_pMotionController = (IPhysicsMotionController *)0x0;
    }
    this->m_bCarriedByPlayer = true;
    SetFiringState(this,HOVER_TURRET_SHOT_DISABLED);
    return;
  }
  return;
}


/* CNPC_HoverTurret::OnPhysGunDrop at 0072a650 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void OnPhysGunDrop(CNPC_HoverTurret * this, CBasePlayer * pPhysGunUser,
   PhysGunDrop_t Reason) */

void __thiscall
CNPC_HoverTurret::OnPhysGunDrop
          (CNPC_HoverTurret *this,CBasePlayer *pPhysGunUser,PhysGunDrop_t Reason)

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
  this->m_bCarriedByPlayer = false;
  if (((CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth> *)
      ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0xe8))->m_Value < 1)
  {
    StartDeathSequence(this);
    return;
  }
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
  uVar7 = (this->m_hTether).super_CBaseHandle.m_Index;
  if (((uVar7 != 0xffffffff) &&
      (iVar5 = (uVar7 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x51c508),
      *(uint *)(iVar5 + 8) == uVar7 >> 0x10)) && (*(int *)(iVar5 + 4) != 0)) {
    SetFiringState(this,HOVER_TURRET_FIND_TARGET);
    this->m_flEngineStallTime =
         *(float *)(**(int **)(unaff_EBX + 0x51c500) + 0xc) + *(float *)(unaff_EBX + 0x3a3744);
    (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
    super_CAI_BaseNPC.super_CBaseCombatCharacter.m_flFieldOfView = -1.0;
    return;
  }
  this->m_iDesiredState = 7;
  if (((CNetworkVarBase<int,CBaseAnimating::NetworkVar_m_nSkin> *)
      ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x3e0))->m_Value !=
      0) {
    if (*(bool *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x60) ==
        false) {
      this_00 = *(CBaseEdict **)
                 ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x24);
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        puVar3 = (ushort *)**(undefined4 **)(unaff_EBX + 0x51c504);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar3) {
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar3[uVar7 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_0072a874:
            puVar3[(uint)uVar2 + uVar7 * 0x14 + 1] = 0x3e0;
            puVar3[uVar7 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar3[uVar7 * 0x14 + 1] != 0x3e0) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar2) {
                if (uVar2 != 0x13) goto LAB_0072a874;
                pIVar6->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar3[uVar7 * 0x14 + local_60 + 1] != 0x3e0);
          }
        }
        else if (puVar3[0x7d1] == 100) {
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar3[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x51c504) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x51c504) + 0xfa2) + 1;
          piVar4 = *(int **)(unaff_EBX + 0x51c504);
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar4;
          iVar5 = *piVar4 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar5 + 2) = 0x3e0;
          *(undefined2 *)(iVar5 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = (uint *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                              super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay
                              .super_CBaseAnimating + 100);
      *puVar1 = *puVar1 | 1;
    }
    ((CNetworkVarBase<int,CBaseAnimating::NetworkVar_m_nSkin> *)
    ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x3e0))->m_Value = 0;
  }
                    /* WARNING: Could not recover jumptable at 0x0072a6f0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                        super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                        super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x684))()
  ;
  return;
}


/* CNPC_HoverTurret::HasPreferredCarryAnglesForPlayer at 007203c0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: bool HasPreferredCarryAnglesForPlayer(CNPC_HoverTurret * this,
   CBasePlayer * pPlayer) */

bool __thiscall
CNPC_HoverTurret::HasPreferredCarryAnglesForPlayer(CNPC_HoverTurret *this,CBasePlayer *pPlayer)

{
  char cVar1;
  undefined1 uVar2;
  
  cVar1 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                super_CBaseCombatCharacter.super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x114))(this);
  uVar2 = 0;
  if (cVar1 != '\0') {
    uVar2 = this->m_bUseCarryAngles;
  }
  return (bool)uVar2;
}


/* CNPC_HoverTurret::PreferredCarryAngles at 00721080 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: QAngle PreferredCarryAngles(CNPC_HoverTurret * this) */

QAngle * __thiscall
CNPC_HoverTurret::PreferredCarryAngles(QAngle *__return_storage_ptr__,CNPC_HoverTurret *this)

{
  vec_t *pvVar1;
  int iVar2;
  uint *puVar3;
  int unaff_EBX;
  CBasePlayer *this_00;
  Vector local_28 [2];
  
                    /* Unresolved local var: Vector vecUserForward@[???]
                       Unresolved local var: CBasePlayer * pPlayer@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&DAT_0072b9bc)[unaff_EBX] == '\0') {
    iVar2 = ___cxa_guard_acquire(&DAT_0072b9bc + unaff_EBX);
    if (iVar2 != 0) {
      ___cxa_guard_release(&DAT_0072b9bc + unaff_EBX);
    }
  }
  this_00 = (CBasePlayer *)0x0;
  if (*(int *)(**(int **)(unaff_EBX + 0x525ad0) + 0x14) < 2) {
    this_00 = UTIL_GetLocalPlayer();
  }
  CBasePlayer::EyeVectors(this_00,local_28,(Vector *)0x0,(Vector *)0x0);
  if (local_28[0].z < 0.0) {
    *(undefined4 *)(&DAT_0072b9c4 + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_0072b9c8 + unaff_EBX) = 0;
    *(undefined4 *)(&LAB_0072b9cc + unaff_EBX) = 0;
    puVar3 = (uint *)(**(code **)((int)(this_00->super_CBaseCombatCharacter).super_CBaseFlex.
                                       super_CBaseAnimatingOverlay.super_CBaseAnimating.
                                       super_CBaseEntity.super_IServerEntity.super_IServerUnknown +
                                 0x224))(this_00);
    *(uint *)(&DAT_0072b9c4 + unaff_EBX) = *puVar3 ^ 0x80000000;
    __return_storage_ptr__->x = *(vec_t *)(&DAT_0072b9c4 + unaff_EBX);
    __return_storage_ptr__->y = *(vec_t *)(&DAT_0072b9c8 + unaff_EBX);
    __return_storage_ptr__->z = *(vec_t *)(&LAB_0072b9cc + unaff_EBX);
  }
  else {
    pvVar1 = *(vec_t **)(unaff_EBX + 0x525b74);
    __return_storage_ptr__->x = *pvVar1;
    __return_storage_ptr__->y = pvVar1[1];
    __return_storage_ptr__->z = pvVar1[2];
  }
  return __return_storage_ptr__;
}


/* CNPC_HoverTurret::Precache at 00725df0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void Precache(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::Precache(CNPC_HoverTurret *this)

{
  uint *puVar1;
  ushort uVar2;
  CBaseEdict *this_00;
  ushort *puVar3;
  int *piVar4;
  __178 _Var5;
  int iVar6;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  uint uVar9;
  int unaff_EBX;
  uint local_60;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x38b4ee));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x385bc4));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x38b50f));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x38b52d));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x38b54b));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x38b569));
  CBaseEntity::PrecacheModel(&UNK_0038b497 + unaff_EBX);
  _Var5 = ActivityList_RegisterPrivateActivity((char *)(unaff_EBX + 0x38b587));
  *(__178 *)(unaff_EBX + 0x6a29ba) = _Var5;
  CAI_BaseNPC::AddActivityToSR((char *)(unaff_EBX + 0x38b587),_Var5);
  _Var5 = ActivityList_RegisterPrivateActivity((char *)(unaff_EBX + 0x38b59f));
  *(__178 *)(unaff_EBX + 0x6a29be) = _Var5;
  CAI_BaseNPC::AddActivityToSR((char *)(unaff_EBX + 0x38b59f),_Var5);
  _Var5 = ActivityList_RegisterPrivateActivity((char *)(unaff_EBX + 0x38b5b6));
  *(__178 *)(unaff_EBX + 0x6a29c2) = _Var5;
  CAI_BaseNPC::AddActivityToSR((char *)(unaff_EBX + 0x38b5b6),_Var5);
  _Var5 = ActivityList_RegisterPrivateActivity(&UNK_0038b5cd + unaff_EBX);
  *(__178 *)(unaff_EBX + 0x6a29c6) = _Var5;
  CAI_BaseNPC::AddActivityToSR(&UNK_0038b5cd + unaff_EBX,_Var5);
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x38f992));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x386212));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x38b4d9));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x38b5e7));
  UTIL_PrecacheDecal((char *)(unaff_EBX + 0x38b5fc),false);
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x38622e));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x3861f9));
  if (*(bool *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                      super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                      super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x60) ==
      false) {
    this_00 = *(CBaseEdict **)
               ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                      super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                      super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x24);
    if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
      this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
      pIVar7 = CBaseEdict::GetChangeAccessor(this_00);
      puVar3 = (ushort *)**(undefined4 **)(unaff_EBX + 0x520d6a);
      if (pIVar7->m_iChangeInfoSerialNumber == *puVar3) {
        uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar2 = puVar3[uVar8 * 0x14 + 0x14];
        if (uVar2 == 0) {
LAB_00726106:
          puVar3[(uint)uVar2 + uVar8 * 0x14 + 1] = 0xf10;
          puVar3[uVar8 * 0x14 + 0x14] = uVar2 + 1;
        }
        else if (puVar3[uVar8 * 0x14 + 1] != 0xf10) {
          local_60 = 0;
          do {
            uVar9 = local_60 + 1;
            local_60 = uVar9 & 0xffff;
            if ((ushort)uVar9 == uVar2) {
              if (uVar2 != 0x13) goto LAB_00726106;
              pIVar7->m_iChangeInfoSerialNumber = 0;
              this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
              break;
            }
          } while (puVar3[uVar8 * 0x14 + local_60 + 1] != 0xf10);
        }
      }
      else if (puVar3[0x7d1] == 100) {
        pIVar7->m_iChangeInfoSerialNumber = 0;
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar7->m_iChangeInfo = puVar3[0x7d1];
        *(short *)(**(int **)(unaff_EBX + 0x520d6a) + 0xfa2) =
             *(short *)(**(int **)(unaff_EBX + 0x520d6a) + 0xfa2) + 1;
        piVar4 = *(int **)(unaff_EBX + 0x520d6a);
        pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar4;
        iVar6 = *piVar4 + (uint)pIVar7->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar6 + 2) = 0xf10;
        *(undefined2 *)(iVar6 + 0x28) = 1;
      }
    }
  }
  else {
    puVar1 = (uint *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                            super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                            super_CBaseAnimating + 100);
    *puVar1 = *puVar1 | 1;
  }
  iVar6 = CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x384c4a));
  (this->m_sLaserHaloSprite).m_Value = iVar6;
                    /* Unresolved local var: int iTalkScript@[???] */
  CBaseEntity::PrecacheScriptSound(*(char **)(unaff_EBX + 0x63ea62));
  CBaseEntity::PrecacheScriptSound(*(char **)(unaff_EBX + 0x63ea66));
  CBaseEntity::PrecacheScriptSound(*(char **)(unaff_EBX + 0x63ea6a));
  CBaseEntity::PrecacheScriptSound(*(char **)(unaff_EBX + 0x63ea6e));
  CBaseEntity::PrecacheScriptSound(*(char **)(unaff_EBX + 0x63ea72));
  CBaseEntity::PrecacheScriptSound(*(char **)(unaff_EBX + 0x63ea76));
  CBaseEntity::PrecacheScriptSound(*(char **)(unaff_EBX + 0x63ea7a));
  CBaseEntity::PrecacheScriptSound(*(char **)(unaff_EBX + 0x63ea7e));
  UTIL_PrecacheOther((char *)(unaff_EBX + 0x38b4b4),(char *)0x0);
  CAI_BaseNPC::Precache((CAI_BaseNPC *)this);
  return;
}


/* CNPC_HoverTurret::RunTask at 00720a50 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void RunTask(CNPC_HoverTurret * this, Task_t * pTask) */

void __thiscall CNPC_HoverTurret::RunTask(CNPC_HoverTurret *this,Task_t *pTask)

{
  if (pTask->iTask != 0x97) {
    CAI_BaseNPC::RunTask((CAI_BaseNPC *)this,pTask);
    return;
  }
  return;
}


/* CNPC_HoverTurret::StartTask at 00720a20 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void StartTask(CNPC_HoverTurret * this, Task_t * pTask) */

void __thiscall CNPC_HoverTurret::StartTask(CNPC_HoverTurret *this,Task_t *pTask)

{
  if (pTask->iTask != 0x97) {
    CAI_BasePhysicsFlyingBot::StartTask((CAI_BasePhysicsFlyingBot *)this,pTask);
    return;
  }
  return;
}


/* CNPC_HoverTurret::UpdateOnRemove at 00720a00 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void UpdateOnRemove(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::UpdateOnRemove(CNPC_HoverTurret *this)

{
  TalkThink(this);
  CAI_BaseNPC::UpdateOnRemove((CAI_BaseNPC *)this);
  return;
}


/* CNPC_HoverTurret::ClampMotorForces at 007204d0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void ClampMotorForces(CNPC_HoverTurret * this, Vector * linear,
   AngularImpulse * angular) */

void __thiscall
CNPC_HoverTurret::ClampMotorForces(CNPC_HoverTurret *this,Vector *linear,AngularImpulse *angular)

{
  float fVar1;
  undefined4 *puVar2;
  int unaff_EBX;
  longdouble lVar3;
  
  ___i686_get_pc_thunk_bx();
  if (((CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth> *)
      ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0xe8))->m_Value < 1)
  {
    linear->x = 0.0;
    linear->y = 0.0;
    linear->z = 0.0;
    angular->x = 0.0;
    angular->y = 0.0;
    angular->z = 0.0;
  }
  else {
    linear->x = 0.0;
    linear->y = 0.0;
    linear->z = 0.0;
    if (this->m_bAimingAtTarget == false) {
      fVar1 = *(float *)(unaff_EBX + 0x3ac7c8);
      angular->x = angular->x * fVar1;
      angular->y = fVar1 * angular->y;
      if ((this->m_flSentryTurnSpeed == 0.0) && (!NAN(this->m_flSentryTurnSpeed))) {
        puVar2 = *(undefined4 **)(unaff_EBX + 0x526704);
        lVar3 = (longdouble)(**(code **)(*(int *)*puVar2 + 4))((int *)*puVar2,0xc2480000,0x42480000)
        ;
        linear->x = (float)lVar3;
        lVar3 = (longdouble)(**(code **)(*(int *)*puVar2 + 4))((int *)*puVar2,0xc2480000,0x42480000)
        ;
        linear->y = (float)lVar3;
        lVar3 = (longdouble)(**(code **)(*(int *)*puVar2 + 4))((int *)*puVar2,0xc3160000,0x43160000)
        ;
        linear->z = (float)lVar3;
      }
    }
  }
  return;
}


/* CNPC_HoverTurret::HasPhysicsAttacker at 007203f0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: CBasePlayer * HasPhysicsAttacker(CNPC_HoverTurret * this, float dt) */

CBasePlayer * __thiscall CNPC_HoverTurret::HasPhysicsAttacker(CNPC_HoverTurret *this,float dt)

{
  return (CBasePlayer *)0x0;
}


/* CNPC_HoverTurret::GetMaxEnginePower at 00720400 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: float GetMaxEnginePower(CNPC_HoverTurret * this) */

float __thiscall CNPC_HoverTurret::GetMaxEnginePower(CNPC_HoverTurret *this)

{
  return 1.0;
}


/* CNPC_HoverTurret::IsMovementDisabled at 00720410 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: bool IsMovementDisabled(CNPC_HoverTurret * this) */

bool __thiscall CNPC_HoverTurret::IsMovementDisabled(CNPC_HoverTurret *this)

{
  float *pfVar1;
  int unaff_EBX;
  bool bVar2;
  
  ___i686_get_pc_thunk_bx();
  bVar2 = true;
  pfVar1 = (float *)(**(int **)(unaff_EBX + 0x52674b) + 0xc);
  if (this->m_flEngineStallTime < *pfVar1 || this->m_flEngineStallTime == *pfVar1) {
    bVar2 = this->m_iFiringState == HOVER_TURRET_SHOT_DISABLED;
  }
  return bVar2;
}


/* CNPC_HoverTurret::MoveToTarget at 00724ab0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void MoveToTarget(CNPC_HoverTurret * this, float flInterval, Vector *
   vMoveTarget) */

void __thiscall
CNPC_HoverTurret::MoveToTarget(CNPC_HoverTurret *this,float flInterval,Vector *vMoveTarget)

{
  float *pfVar1;
  float current;
  int *piVar2;
  code *pcVar3;
  bool bVar4;
  int iVar5;
  int unaff_EBX;
  float fVar6;
  float fVar7;
  float local_64;
  float local_60;
  float local_5c;
  float local_58;
  float local_54;
  float local_50;
  Vector local_4c;
  float local_40;
  float local_3c;
  float local_38;
  Vector local_34;
  Vector local_28 [2];
  
  ___i686_get_pc_thunk_bx();
  if (((*(float *)(unaff_EBX + 0x3a80a9) <= flInterval &&
        flInterval != *(float *)(unaff_EBX + 0x3a80a9)) &&
      (piVar2 = *(int **)(unaff_EBX + 0x52209d), pfVar1 = (float *)(*piVar2 + 0xc),
      this->m_flEngineStallTime < *pfVar1 || this->m_flEngineStallTime == *pfVar1)) &&
     (this->m_iFiringState != HOVER_TURRET_SHOT_DISABLED)) {
    iVar5 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                  super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                  super_CBaseCombatCharacter.super_CBaseFlex.
                                  super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x188))(this);
    if ((iVar5 == 0) || (bVar4 = CAI_BaseNPC::HasCondition((CAI_BaseNPC *)this,10), !bVar4)) {
      if (this->m_flSentryTurnSpeed == 0.0) {
        if ((*(byte *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                             super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                             super_CBaseAnimating + 0x12d) & 8) != 0) {
          CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
        }
                    /* Unresolved local var: Vector res@[???] */
        local_40 = vMoveTarget->x -
                   ((Vector *)
                   ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x294))->x;
        local_3c = vMoveTarget->y -
                   *(vec_t *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                    super_CBaseCombatCharacter.super_CBaseFlex.
                                    super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x298);
        local_38 = vMoveTarget->z -
                   *(vec_t *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                    super_CBaseCombatCharacter.super_CBaseFlex.
                                    super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x29c);
        local_34.x = local_40;
        local_34.y = local_3c;
        local_34.z = local_38;
        VectorNormalize(&local_34);
        AngleVectors(&this->m_vInitialLookAngles,&local_4c);
                    /* Unresolved local var: Vector res@[???] */
        fVar7 = *(float *)(unaff_EBX + 0x3a8079);
                    /* Unresolved local var: Vector res@[???] */
        local_58 = local_34.x + local_4c.x * fVar7;
        local_54 = local_4c.y * fVar7 + local_34.y;
        local_50 = fVar7 * local_4c.z + local_34.z;
        pcVar3 = *(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                     super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                     super_CBaseCombatCharacter.super_CBaseFlex.
                                     super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x9c0);
        local_4c.x = local_58;
        local_4c.y = local_54;
        local_4c.z = local_50;
        if ((*(byte *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                             super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                             super_CBaseAnimating + 0x12d) & 8) != 0) {
          CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
        }
                    /* Unresolved local var: Vector res@[???] */
        local_64 = ((Vector *)
                   ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x294))->x + local_4c.x;
        local_60 = *(vec_t *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                    super_CBaseCombatCharacter.super_CBaseFlex.
                                    super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x298) +
                   local_4c.y;
        local_5c = *(vec_t *)((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                    super_CBaseCombatCharacter.super_CBaseFlex.
                                    super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x29c) +
                   local_4c.z;
        (*pcVar3)(this,flInterval,&local_64);
      }
      else {
                    /* Unresolved local var: float newYaw@[???] */
        fVar7 = *(float *)(*piVar2 + 0xc);
        fVar6 = CBaseEntity::GetLastThink((CBaseEntity *)this,(char *)0x0);
        current = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                  super_CAI_BasePhysicsFlyingBot.m_fHeadYaw;
        fVar7 = AI_ClampYaw(this->m_flSentryTurnSpeed,current,
                            flInterval * this->m_flSentryTurnSpeed + current,fVar7 - fVar6);
        if (fVar7 != (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                     super_CAI_BasePhysicsFlyingBot.m_fHeadYaw) {
          (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot
          .m_fHeadYaw = fVar7;
        }
      }
    }
    else {
      pcVar3 = *(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                   super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                   super_CBaseCombatCharacter.super_CBaseFlex.
                                   super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x9c0);
      GetClosestVisibleEnemyPosition(local_28,this);
      (*pcVar3)(this,flInterval,local_28);
    }
  }
  return;
}


/* CNPC_HoverTurret::ShootLaser at 00722d30 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void ShootLaser(CNPC_HoverTurret * this, Vector * vecSrc, Vector *
   vecDirToEnemy) */

void __thiscall
CNPC_HoverTurret::ShootLaser(CNPC_HoverTurret *this,Vector *vecSrc,Vector *vecDirToEnemy)

{
  int iVar1;
  int unaff_EBX;
  float fVar2;
  CTakeDamageInfo *in_stack_fffffecc;
  CBaseEntity *in_stack_fffffed0;
  CBaseEntity *pAttacker_1;
  float in_stack_fffffed8;
  int in_stack_fffffedc;
  int in_stack_fffffee0;
  Vector *local_11c;
  trace_t local_100;
  CTakeDamageInfo local_ac;
  CTraceFilterSimple local_50;
  _func_int_varargs **local_40;
  IHandleEntity *local_3c;
  float local_38;
  float local_34;
  float local_30;
  float local_2c;
  float local_28;
  float local_24;
  float local_20;
  
                    /* Unresolved local var: trace_t tr@[???]
                       Unresolved local var: Vector vecCenter@[???] */
  ___i686_get_pc_thunk_bx();
  if (((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
       super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
                    /* Unresolved local var: Vector res@[???] */
  local_20 = *(float *)(unaff_EBX + 0x3ac0a3);
  local_28 = vecDirToEnemy->x * local_20;
  local_24 = vecDirToEnemy->y * local_20;
  local_20 = local_20 * vecDirToEnemy->z;
                    /* Unresolved local var: Vector res@[???] */
  local_ac.m_vecDamageForce.x = vecSrc->x;
  local_34 = local_ac.m_vecDamageForce.x + local_28;
  local_ac.m_vecDamageForce.y = vecSrc->y;
  local_30 = local_ac.m_vecDamageForce.y + local_24;
  local_ac.m_vecDamageForce.z = vecSrc->z;
  local_2c = local_ac.m_vecDamageForce.z + local_20;
  local_ac.m_vecDamagePosition.y = local_34 - vecSrc->x;
  local_ac.m_vecDamagePosition.z = local_30 - vecSrc->y;
  local_ac.m_vecReportedPosition.x = local_2c - vecSrc->z;
  local_ac.m_iDamageStats._1_1_ =
       local_ac.m_vecDamagePosition.y * local_ac.m_vecDamagePosition.y +
       local_ac.m_vecDamagePosition.z * local_ac.m_vecDamagePosition.z +
       local_ac.m_vecReportedPosition.x * local_ac.m_vecReportedPosition.x != 0.0;
  local_ac.m_flBaseDamage = 0.0;
  local_ac.m_flMaxDamage = 0.0;
  local_ac.m_flDamage = 0.0;
  local_ac.m_iDamageCustom = 0;
  local_ac.m_iDamageStats._0_1_ = 1;
  local_ac.m_hAttacker.super_CBaseHandle.m_Index = (CBaseHandle)0;
  local_ac.m_hInflictor.super_CBaseHandle.m_Index = (CBaseHandle)0;
  local_ac.m_vecReportedPosition.z = 0.0;
  CTraceFilterSimple::CTraceFilterSimple(&local_50,(IHandleEntity *)this,0,(ShouldHitFunc_t)0x0);
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x523e8f) + 0x14))
            ((int *)**(undefined4 **)(unaff_EBX + 0x523e8f),&local_ac,0x46004003,&local_50,
             &local_100);
  if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x523e8b) + 0x1c) + 0x30) != 0) {
    in_stack_fffffecc = (CTakeDamageInfo *)0xbf800000;
    DebugDrawLine((Vector *)&local_100,&local_100.super_CBaseTrace.endpos,0xff,0,0,true,-1.0);
  }
  if (*(float *)(unaff_EBX + 0x3a9bcf) != local_100.super_CBaseTrace.fraction) {
    if (local_100.m_pEnt != (CBaseEntity *)0x0) {
      ClearMultiDamage();
      if (((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
           m_iEFlags & 0x800) != 0) {
        CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
      }
                    /* Unresolved local var: Vector res@[???] */
      local_50.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter =
           (ITraceFilter)
           (local_100.super_CBaseTrace.endpos.x -
           (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
           m_vecAbsOrigin.x);
      local_50.m_pPassEnt =
           (IHandleEntity *)
           (local_100.super_CBaseTrace.endpos.y -
           (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
           m_vecAbsOrigin.y);
      local_50.m_collisionGroup =
           (int)(local_100.super_CBaseTrace.endpos.z -
                (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                m_vecAbsOrigin.z);
      local_40 = (_func_int_varargs **)
                 local_50.super_CTraceFilter.super_ITraceFilter._vptr_ITraceFilter;
      local_3c = local_50.m_pPassEnt;
      local_38 = (float)local_50.m_collisionGroup;
      VectorNormalize((Vector *)&local_50);
      pAttacker_1 = *(CBaseEntity **)(**(int **)(unaff_EBX + 0x523e23) + 0xc);
      fVar2 = CBaseEntity::GetLastThink((CBaseEntity *)this,(char *)0x0);
      CTakeDamageInfo::CTakeDamageInfo
                (&local_ac,(CBaseEntity *)this,(CBaseEntity *)this,
                 ((float)pAttacker_1 - fVar2) * *(float *)(unaff_EBX + 0x3a9f5f),0x400,0,
                 in_stack_fffffecc,in_stack_fffffed0,pAttacker_1,in_stack_fffffed8,in_stack_fffffedc
                 ,in_stack_fffffee0);
      fVar2 = 0.0;
      if (this->m_bCanPushPlayer != false) {
        fVar2 = *(float *)(unaff_EBX + 0x3ab103);
      }
      CalculateMeleeDamageForce
                (&local_ac,(Vector *)&local_50,&local_100.super_CBaseTrace.endpos,fVar2);
      CBaseEntity::DispatchTraceAttack(local_100.m_pEnt,&local_ac,(Vector *)&local_50,&local_100);
      ApplyMultiDamage();
      UTIL_DecalTrace(&local_100,(char *)(unaff_EBX + 0x38d7d9));
      iVar1 = (*((local_100.m_pEnt)->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                _vptr_IHandleEntity[0x55])(local_100.m_pEnt);
      if ((char)iVar1 != '\0') {
        CBaseEntity::EmitSound(local_100.m_pEnt,(char *)(unaff_EBX + 0x38e596),0.0,(float *)0x0);
      }
    }
    local_11c = &local_100.super_CBaseTrace.endpos;
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x523f53) + 0x10))
              ((int *)**(undefined4 **)(unaff_EBX + 0x523f53),local_11c,1,1,0);
  }
  return;
}


/* CNPC_HoverTurret::OnBurning at 00722cc0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: bool OnBurning(CNPC_HoverTurret * this) */

bool __thiscall CNPC_HoverTurret::OnBurning(CNPC_HoverTurret *this)

{
  int iVar1;
  int iVar2;
  
  iVar2 = ((CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth> *)
          ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                 super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                 super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0xe8))->m_Value;
  iVar1 = iVar2 + -1;
  if (iVar2 != iVar1) {
                    /* Unresolved local var: CBaseEntity * pThis@[???] */
    (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x200))
              (this,(CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth> *)
                    ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                           super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                           super_CBaseAnimating + 0xe8));
    ((CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth> *)
    ((int)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
           super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
           super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating + 0xe8))->m_Value =
         iVar1;
  }
  if (0 < iVar1) {
    return true;
  }
  Explode(this);
  return false;
}


/* CNPC_HoverTurret::Explode at 00722380 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void Explode(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::Explode(CNPC_HoverTurret *this)

{
  CBaseEntity *this_00;
  string_t *psVar1;
  undefined4 *puVar2;
  code *pcVar3;
  BASEPTR func;
  CBreakableProp *this_01;
  string_t *psVar4;
  string_t *psVar5;
  int iVar6;
  float *pfVar7;
  int iVar8;
  Vector *pVVar9;
  char *pcVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  CNPC_HoverTurret *pCVar13;
  int unaff_EBX;
  int iVar14;
  float fVar15;
  CTakeDamageInfo *in_stack_fffffbac;
  CBaseEntity *in_stack_fffffbb0;
  CBaseEntity *in_stack_fffffbb4;
  float in_stack_fffffbb8;
  int in_stack_fffffbbc;
  int in_stack_fffffbc0;
  CBaseEntity *local_3e0 [128];
  trace_t local_1e0;
  vec_t local_18c;
  vec_t local_188;
  vec_t local_184;
  float local_17c;
  float local_178;
  float local_174;
  undefined4 local_16c;
  undefined4 local_168;
  undefined4 local_164;
  undefined4 local_15c;
  undefined4 local_158;
  undefined4 local_154;
  undefined4 local_14c;
  undefined1 local_148;
  undefined1 local_147;
  CTakeDamageInfo local_138;
  CTakeDamageInfo local_e8;
  CRecipientFilter local_98;
  CFlaggedEntitiesEnum local_78;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  float local_58;
  float local_54;
  float local_50;
  float local_4c;
  float local_48;
  float local_44;
  Vector local_40;
  Vector local_34;
  Vector local_28 [2];
  
                    /* Unresolved local var: CBreakableProp * pFutbol@[???]
                       Unresolved local var: string_t strTurretName@[???]
                       Unresolved local var: string_t strPropName@[???]
                       Unresolved local var: Vector vecRadius@[???]
                       Unresolved local var: CBaseEntity *[128] pEnts@[???]
                       Unresolved local var: int nNumFound@[???]
                       Unresolved local var: CPVSFilter filter@[???]
                       Unresolved local var: Vector gibVelocity@[???]
                       Unresolved local var: int iModelIndex@[???] */
  ___i686_get_pc_thunk_bx();
  if ((((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
        super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
        super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) &&
     (CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this),
     ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
      super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
      super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0)) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
  pVVar9 = &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
            super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
            super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
            m_vecAbsOrigin;
  pCVar13 = (CNPC_HoverTurret *)pVVar9;
  this_01 = (CBreakableProp *)
            CBaseEntity::Create(&UNK_0038ef21 + unaff_EBX,pVVar9,
                                &(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                 super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                 super_CBaseCombatCharacter.super_CBaseFlex.
                                 super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                                 m_angAbsRotation,(CBaseEntity *)0x0);
  if (this_01 != (CBreakableProp *)0x0) {
                    /* Unresolved local var: Vector vecVelocity@[???] */
    (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                          super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                          super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
                          super_CBaseAnimating + 0x248))(this,&local_78,0);
    CBaseEntity::SetAbsVelocity((CBaseEntity *)this_01,(Vector *)&local_78);
    CTakeDamageInfo::CTakeDamageInfo
              (&local_e8,(CBaseEntity *)this,(CBaseEntity *)this,200.0,0x40,0,in_stack_fffffbac,
               in_stack_fffffbb0,in_stack_fffffbb4,in_stack_fffffbb8,in_stack_fffffbbc,
               in_stack_fffffbc0);
    pCVar13 = this;
    CBreakableProp::Break(this_01,(CBaseEntity *)this,&local_e8);
  }
  psVar4 = AllocPooledString((string_t *)(unaff_EBX + 0x38b81f),(char *)pCVar13);
  psVar5 = AllocPooledString((string_t *)(&UNK_0038ef33 + unaff_EBX),(char *)pCVar13);
  if (((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
       super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
                    /* Unresolved local var: Vector res@[???] */
  fVar15 = *(float *)(unaff_EBX + 0x3ab9b7);
  local_28[0].x =
       (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
       super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.x;
  local_34.x = local_28[0].x + fVar15;
  local_28[0].y =
       (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
       super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.y;
  local_34.y = local_28[0].y + fVar15;
  local_28[0].z =
       (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
       super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z;
  local_34.z = local_28[0].z + fVar15;
  if (((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
       super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    local_28[0].x =
         (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
         super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
         super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.x;
    local_28[0].y =
         (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
         super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
         super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.y;
    local_28[0].z =
         (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).super_CAI_BasePhysicsFlyingBot.
         super_CAI_BaseNPC.super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay.
         super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z;
  }
                    /* Unresolved local var: Vector res@[???] */
  local_28[0].x = local_28[0].x - fVar15;
  local_28[0].y = local_28[0].y - fVar15;
  local_28[0].z = local_28[0].z - fVar15;
                    /* Unresolved local var: CFlaggedEntitiesEnum boxEnum@[???] */
  CFlaggedEntitiesEnum::CFlaggedEntitiesEnum(&local_78,local_3e0,0x80,0);
  iVar6 = UTIL_EntitiesInBox(local_28,&local_34,&local_78);
  if (0 < iVar6) {
    iVar14 = 0;
    do {
      pCVar13 = (CNPC_HoverTurret *)local_3e0[iVar14];
      if ((pCVar13 != (CNPC_HoverTurret *)0x0) && (pCVar13 != this)) {
        psVar1 = (string_t *)
                 (pCVar13->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                 super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                 super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                 m_iClassname.pszValue;
        if ((psVar4 != psVar1) && (psVar1 != psVar5)) {
          iVar8 = (*(*(_func_int_varargs ***)
                      &(pCVar13->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                       super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                       super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating)[0x55])
                            (pCVar13);
          if ((char)iVar8 == '\0') goto LAB_007227e3;
          pCVar13 = (CNPC_HoverTurret *)local_3e0[iVar14];
        }
        pfVar7 = (float *)(*(*(_func_int_varargs ***)
                              &(pCVar13->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                               super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                               super_CBaseCombatCharacter.super_CBaseFlex.
                               super_CBaseAnimatingOverlay.super_CBaseAnimating)[0x9d])(pCVar13);
        if (((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
             super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
             super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
             m_iEFlags & 0x800) != 0) {
          CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
        }
        local_14c = 0;
        local_17c = *pfVar7 - (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                              super_CBaseCombatCharacter.super_CBaseFlex.super_CBaseAnimatingOverlay
                              .super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.x;
        local_178 = pfVar7[1] -
                    (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                    super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                    super_CBaseEntity.m_vecAbsOrigin.y;
        local_174 = pfVar7[2] -
                    (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                    super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                    super_CBaseEntity.m_vecAbsOrigin.z;
        local_147 = local_17c * local_17c + local_178 * local_178 + local_174 * local_174 != 0.0;
        local_154 = 0;
        local_158 = 0;
        local_15c = 0;
        local_148 = 1;
        local_164 = 0;
        local_168 = 0;
        local_16c = 0;
        local_18c = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                    super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                    super_CBaseEntity.m_vecAbsOrigin.x;
        local_188 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                    super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                    super_CBaseEntity.m_vecAbsOrigin.y;
        local_184 = (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                    super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                    super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                    super_CBaseEntity.m_vecAbsOrigin.z;
        CTraceFilterSimple::CTraceFilterSimple
                  ((CTraceFilterSimple *)&local_78,(IHandleEntity *)this,0,(ShouldHitFunc_t)0x0);
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52483f) + 0x14))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x52483f),&local_18c,0x400b,&local_78,
                   &local_1e0);
        if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x52483b) + 0x1c) + 0x30) != 0) {
          in_stack_fffffbac = (CTakeDamageInfo *)0xbf800000;
          DebugDrawLine((Vector *)&local_1e0,&local_1e0.super_CBaseTrace.endpos,0xff,0,0,true,-1.0);
        }
        if (*(float *)(&DAT_003aa57f + unaff_EBX) <= local_1e0.super_CBaseTrace.fraction) {
          CTakeDamageInfo::CTakeDamageInfo
                    (&local_138,(CBaseEntity *)this,(CBaseEntity *)this,50.0,0x400,0,
                     in_stack_fffffbac,in_stack_fffffbb0,in_stack_fffffbb4,in_stack_fffffbb8,
                     in_stack_fffffbbc,in_stack_fffffbc0);
          if (((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
               super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
               super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
               m_iEFlags & 0x800) != 0) {
            CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
          }
          local_138.m_vecDamagePosition.x =
               (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
               super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
               super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
               m_vecAbsOrigin.x;
          local_138.m_vecDamagePosition.y =
               (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
               super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
               super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
               m_vecAbsOrigin.y;
          local_138.m_vecDamagePosition.z =
               (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
               super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
               super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
               m_vecAbsOrigin.z;
          if ((((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                m_iEFlags & 0x800) != 0) &&
             (CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this),
             ((this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
              super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
              super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
              m_iEFlags & 0x800) != 0)) {
            CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
          }
          this_00 = local_3e0[iVar14];
          if ((this_00->m_iEFlags & 0x800) != 0) {
            CBaseEntity::CalcAbsolutePosition(this_00);
          }
                    /* Unresolved local var: Vector res@[???] */
          local_40.x = (this_00->m_vecAbsOrigin).x -
                       (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                       super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                       super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                       super_CBaseEntity.m_vecAbsOrigin.x;
          local_40.y = (this_00->m_vecAbsOrigin).y -
                       (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                       super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                       super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                       super_CBaseEntity.m_vecAbsOrigin.y;
          local_40.z = (this_00->m_vecAbsOrigin).z -
                       (this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                       super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.super_CBaseCombatCharacter.
                       super_CBaseFlex.super_CBaseAnimatingOverlay.super_CBaseAnimating.
                       super_CBaseEntity.m_vecAbsOrigin.z;
          CalculateExplosiveDamageForce(&local_138,&local_40,pVVar9,1.0);
          CBaseEntity::DispatchTraceAttack
                    (local_3e0[iVar14],&local_138,*(Vector **)(unaff_EBX + 0x5247e7),&local_1e0);
          ApplyMultiDamage();
        }
      }
LAB_007227e3:
      iVar14 = iVar14 + 1;
    } while (iVar14 != iVar6);
  }
  pVVar9 = (Vector *)
           (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                 super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                 super_CBaseCombatCharacter.super_CBaseFlex.
                                 super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x274))(this);
  ExplosionCreate(pVVar9,*(QAngle **)(unaff_EBX + 0x524877),(CBaseEntity *)this,200,100,0x529,0.0,
                  (CBaseEntity *)0x0,-1,(EHANDLE *)0x0,CLASS_NONE);
  pVVar9 = (Vector *)
           (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                 super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                 super_CBaseCombatCharacter.super_CBaseFlex.
                                 super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x274))(this);
  UTIL_ScreenShake(pVVar9,5.0,150.0,1.0,750.0,SHAKE_START,false,
                   (CUtlVector<CBasePlayer*,CUtlMemory<CBasePlayer*,_int>_> *)0x0);
  pVVar9 = (Vector *)
           (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                 super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                 super_CBaseCombatCharacter.super_CBaseFlex.
                                 super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x274))(this);
  CRecipientFilter::CRecipientFilter(&local_98);
  local_98.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(unaff_EBX + 0x527557);
  CRecipientFilter::AddRecipientsByPVS(&local_98,pVVar9);
                    /* Unresolved local var: Vector random@[???] */
  iVar6 = rand();
  local_58 = ((float)iVar6 / *(float *)(unaff_EBX + 0x3aa877)) * *(float *)(unaff_EBX + 0x3aa893) +
             *(float *)(unaff_EBX + 0x3aac1b);
  iVar6 = rand();
  local_54 = ((float)iVar6 / *(float *)(unaff_EBX + 0x3aa877)) * *(float *)(unaff_EBX + 0x3aa893) +
             *(float *)(unaff_EBX + 0x3aac1b);
  iVar6 = rand();
  local_50 = ((float)iVar6 / *(float *)(unaff_EBX + 0x3aa877)) * *(float *)(unaff_EBX + 0x3aa893) +
             *(float *)(unaff_EBX + 0x3aac1b);
  local_4c = local_58;
  local_48 = local_54;
  puVar2 = *(undefined4 **)(unaff_EBX + 0x5247df);
  pcVar3 = *(code **)(*(int *)*puVar2 + 0xc);
  local_44 = local_50;
  pcVar10 = CPropData::GetRandomChunkModel
                      (*(CPropData **)(unaff_EBX + 0x524af7),(char *)(unaff_EBX + 0x38f553),-1);
  uVar11 = (*pcVar3)(*puVar2,pcVar10);
  iVar6 = 0;
                    /* Unresolved local var: int i@[???] */
  do {
    pcVar3 = *(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52487b) + 0x38);
    local_64 = 0x41800000;
    local_60 = 0x41800000;
    local_5c = 0x41800000;
    uVar12 = (**(code **)(*(int *)&(this->super_CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>).
                                   super_CAI_BasePhysicsFlyingBot.super_CAI_BaseNPC.
                                   super_CBaseCombatCharacter.super_CBaseFlex.
                                   super_CBaseAnimatingOverlay.super_CBaseAnimating + 0x274))(this);
    (*pcVar3)(**(undefined4 **)(unaff_EBX + 0x52487b),&local_98,0,uVar12,
              *(undefined4 *)(unaff_EBX + 0x524877),&local_64,&local_4c,uVar11,400,1,0x40200000,2);
    iVar6 = iVar6 + 1;
  } while (iVar6 != 0x10);
  CBaseEntity::AddEffects((CBaseEntity *)this,0x20);
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)*(uint *)(unaff_EBX + 0x52484f);
  CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
  fVar15 = *(float *)(&DAT_003aa583 + unaff_EBX) +
           *(float *)(**(int **)(unaff_EBX + 0x5247d3) + 0xc);
  CBaseEntity::SetNextThink((CBaseEntity *)this,fVar15,(char *)0x0);
  local_98.super_IRecipientFilter._vptr_IRecipientFilter =
       (_func_int_varargs **)(unaff_EBX + 0x527557);
  CRecipientFilter::~CRecipientFilter(&local_98,(int)fVar15);
  return;
}


/* CNPC_HoverTurret::CreateSmokeTrail at 00723140 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Restarted to delay deadcode elimination for space: stack */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void CreateSmokeTrail(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::CreateSmokeTrail(CNPC_HoverTurret *this)

{
  undefined1 *puVar1;
  vec_t *pvVar2;
  float fVar3;
  ushort uVar4;
  CBaseEdict *pCVar5;
  ushort *puVar6;
  int *piVar7;
  int iVar8;
  SmokeTrail *this_00;
  ulong *puVar9;
  IChangeInfoAccessor *pIVar10;
  uint uVar11;
  uint uVar12;
  int unaff_EBX;
  uint local_140;
  uint local_13c;
  uint local_138;
  uint local_134;
  uint local_130;
  uint local_12c;
  uint local_128;
  uint local_124;
  uint local_120;
  
                    /* Unresolved local var: SmokeTrail * pSmokeTrail@[???] */
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
  uVar11 = (this->m_hSmokeTrail).super_CBaseHandle.m_Index;
  if ((((uVar11 == 0xffffffff) ||
       (iVar8 = (uVar11 & 0xffff) * 0x10 + **(int **)(&DAT_00523a1b + unaff_EBX),
       *(uint *)(iVar8 + 8) != uVar11 >> 0x10)) || (*(int *)(iVar8 + 4) == 0)) &&
     (this_00 = SmokeTrail::CreateSmokeTrail(), this_00 != (SmokeTrail *)0x0)) {
    if (*(float *)(unaff_EBX + 0x3a9acb) != (this_00->m_SpawnRate).m_Value) {
      if ((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_TimerEvent.
          m_bRegistered == false) {
        pCVar5 = &((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
          pIVar10 = CBaseEdict::GetChangeAccessor(pCVar5);
          puVar6 = (ushort *)**(undefined4 **)(&DAT_00523a17 + unaff_EBX);
          if (pIVar10->m_iChangeInfoSerialNumber == *puVar6) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar4 = puVar6[uVar11 * 0x14 + 0x14];
            if (uVar4 == 0) {
LAB_00723ea0:
              puVar6[(uint)uVar4 + uVar11 * 0x14 + 1] = 0x3dc;
              puVar6[uVar11 * 0x14 + 0x14] = uVar4 + 1;
            }
            else if (puVar6[uVar11 * 0x14 + 1] != 0x3dc) {
              local_140 = 0;
              do {
                uVar12 = local_140 + 1;
                local_140 = uVar12 & 0xffff;
                if ((ushort)uVar12 == uVar4) {
                  if (uVar4 == 0x13) goto LAB_007241f2;
                  goto LAB_00723ea0;
                }
              } while (puVar6[uVar11 * 0x14 + local_140 + 1] != 0x3dc);
            }
          }
          else if (puVar6[0x7d1] == 100) {
LAB_007241f2:
            pIVar10->m_iChangeInfoSerialNumber = 0;
            pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar10->m_iChangeInfo = puVar6[0x7d1];
            *(short *)(**(int **)(&DAT_00523a17 + unaff_EBX) + 0xfa2) =
                 *(short *)(**(int **)(&DAT_00523a17 + unaff_EBX) + 0xfa2) + 1;
            piVar7 = *(int **)(&DAT_00523a17 + unaff_EBX);
            pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar7;
            iVar8 = *piVar7 + (uint)pIVar10->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar8 + 2) = 0x3dc;
            *(undefined2 *)(iVar8 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this_00->m_SpawnRate).m_Value = 64.0;
    }
    if (*(float *)(&LAB_003a99af + unaff_EBX) != (this_00->m_ParticleLifetime).m_Value) {
      if ((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_TimerEvent.
          m_bRegistered == false) {
        pCVar5 = &((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
          pIVar10 = CBaseEdict::GetChangeAccessor(pCVar5);
          puVar6 = (ushort *)**(undefined4 **)(&DAT_00523a17 + unaff_EBX);
          if (pIVar10->m_iChangeInfoSerialNumber == *puVar6) {
            uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar4 = puVar6[uVar11 * 0x14 + 0x14];
            if (uVar4 == 0) {
LAB_00723d00:
              puVar6[(uint)uVar4 + uVar11 * 0x14 + 1] = 0x3e0;
              puVar6[uVar11 * 0x14 + 0x14] = uVar4 + 1;
            }
            else if (puVar6[uVar11 * 0x14 + 1] != 0x3e0) {
              local_13c = 0;
              do {
                uVar12 = local_13c + 1;
                local_13c = uVar12 & 0xffff;
                if ((ushort)uVar12 == uVar4) {
                  if (uVar4 != 0x13) goto LAB_00723d00;
                  pIVar10->m_iChangeInfoSerialNumber = 0;
                  pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar6[uVar11 * 0x14 + local_13c + 1] != 0x3e0);
            }
          }
          else if (puVar6[0x7d1] == 100) {
            pIVar10->m_iChangeInfoSerialNumber = 0;
            pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar10->m_iChangeInfo = puVar6[0x7d1];
            *(short *)(**(int **)(&DAT_00523a17 + unaff_EBX) + 0xfa2) =
                 *(short *)(**(int **)(&DAT_00523a17 + unaff_EBX) + 0xfa2) + 1;
            piVar7 = *(int **)(&DAT_00523a17 + unaff_EBX);
            pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar7;
            iVar8 = *piVar7 + (uint)pIVar10->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar8 + 2) = 0x3e0;
            *(undefined2 *)(iVar8 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this_00->m_ParticleLifetime).m_Value = 0.5;
    }
    if (*(float *)(unaff_EBX + 0x3a9e4b) != (this_00->m_StartSize).m_Value) {
      if ((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_TimerEvent.
          m_bRegistered == false) {
        pCVar5 = &((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
          pIVar10 = CBaseEdict::GetChangeAccessor(pCVar5);
          puVar6 = (ushort *)**(undefined4 **)(&DAT_00523a17 + unaff_EBX);
          if (pIVar10->m_iChangeInfoSerialNumber == *puVar6) {
            uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar4 = puVar6[uVar11 * 0x14 + 0x14];
            if (uVar4 == 0) {
LAB_00723f70:
              puVar6[(uint)uVar4 + uVar11 * 0x14 + 1] = 0x3f0;
              puVar6[uVar11 * 0x14 + 0x14] = uVar4 + 1;
            }
            else if (puVar6[uVar11 * 0x14 + 1] != 0x3f0) {
              local_138 = 0;
              do {
                uVar12 = local_138 + 1;
                local_138 = uVar12 & 0xffff;
                if ((ushort)uVar12 == uVar4) {
                  if (uVar4 != 0x13) goto LAB_00723f70;
                  pIVar10->m_iChangeInfoSerialNumber = 0;
                  pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar6[uVar11 * 0x14 + local_138 + 1] != 0x3f0);
            }
          }
          else if (puVar6[0x7d1] == 100) {
            pIVar10->m_iChangeInfoSerialNumber = 0;
            pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar10->m_iChangeInfo = puVar6[0x7d1];
            *(short *)(**(int **)(&DAT_00523a17 + unaff_EBX) + 0xfa2) =
                 *(short *)(**(int **)(&DAT_00523a17 + unaff_EBX) + 0xfa2) + 1;
            piVar7 = *(int **)(&DAT_00523a17 + unaff_EBX);
            pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar7;
            iVar8 = *piVar7 + (uint)pIVar10->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar8 + 2) = 0x3f0;
            *(undefined2 *)(iVar8 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this_00->m_StartSize).m_Value = 8.0;
    }
    if (*(float *)(unaff_EBX + 0x3a9a33) != (this_00->m_EndSize).m_Value) {
      if ((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_TimerEvent.
          m_bRegistered == false) {
        pCVar5 = &((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
          pIVar10 = CBaseEdict::GetChangeAccessor(pCVar5);
          puVar6 = (ushort *)**(undefined4 **)(&DAT_00523a17 + unaff_EBX);
          if (pIVar10->m_iChangeInfoSerialNumber == *puVar6) {
            uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar4 = puVar6[uVar11 * 0x14 + 0x14];
            if (uVar4 == 0) {
LAB_00724040:
              puVar6[(uint)uVar4 + uVar11 * 0x14 + 1] = 0x3f4;
              puVar6[uVar11 * 0x14 + 0x14] = uVar4 + 1;
            }
            else if (puVar6[uVar11 * 0x14 + 1] != 0x3f4) {
              local_134 = 0;
              do {
                uVar12 = local_134 + 1;
                local_134 = uVar12 & 0xffff;
                if ((ushort)uVar12 == uVar4) {
                  if (uVar4 != 0x13) goto LAB_00724040;
                  pIVar10->m_iChangeInfoSerialNumber = 0;
                  pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar6[uVar11 * 0x14 + local_134 + 1] != 0x3f4);
            }
          }
          else if (puVar6[0x7d1] == 100) {
            pIVar10->m_iChangeInfoSerialNumber = 0;
            pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar10->m_iChangeInfo = puVar6[0x7d1];
            *(short *)(**(int **)(&DAT_00523a17 + unaff_EBX) + 0xfa2) =
                 *(short *)(**(int **)(&DAT_00523a17 + unaff_EBX) + 0xfa2) + 1;
            piVar7 = *(int **)(&DAT_00523a17 + unaff_EBX);
            pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar7;
            iVar8 = *piVar7 + (uint)pIVar10->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar8 + 2) = 0x3f4;
            *(undefined2 *)(iVar8 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this_00->m_EndSize).m_Value = 16.0;
    }
    if (*(float *)(unaff_EBX + 0x3a9a33) != (this_00->m_SpawnRadius).m_Value) {
      if ((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_TimerEvent.
          m_bRegistered == false) {
        pCVar5 = &((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
          pIVar10 = CBaseEdict::GetChangeAccessor(pCVar5);
          puVar6 = (ushort *)**(undefined4 **)(&DAT_00523a17 + unaff_EBX);
          if (pIVar10->m_iChangeInfoSerialNumber == *puVar6) {
            uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar4 = puVar6[uVar11 * 0x14 + 0x14];
            if (uVar4 == 0) {
LAB_00723b7d:
              puVar6[(uint)uVar4 + uVar11 * 0x14 + 1] = 0x3f8;
              puVar6[uVar11 * 0x14 + 0x14] = uVar4 + 1;
            }
            else if (puVar6[uVar11 * 0x14 + 1] != 0x3f8) {
              local_130 = 0;
              do {
                uVar12 = local_130 + 1;
                local_130 = uVar12 & 0xffff;
                if ((ushort)uVar12 == uVar4) {
                  if (uVar4 != 0x13) goto LAB_00723b7d;
                  pIVar10->m_iChangeInfoSerialNumber = 0;
                  pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar6[uVar11 * 0x14 + local_130 + 1] != 0x3f8);
            }
          }
          else if (puVar6[0x7d1] == 100) {
            pIVar10->m_iChangeInfoSerialNumber = 0;
            pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar10->m_iChangeInfo = puVar6[0x7d1];
            *(short *)(**(int **)(&DAT_00523a17 + unaff_EBX) + 0xfa2) =
                 *(short *)(**(int **)(&DAT_00523a17 + unaff_EBX) + 0xfa2) + 1;
            piVar7 = *(int **)(&DAT_00523a17 + unaff_EBX);
            pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar7;
            iVar8 = *piVar7 + (uint)pIVar10->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar8 + 2) = 0x3f8;
            *(undefined2 *)(iVar8 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this_00->m_SpawnRadius).m_Value = 16.0;
    }
    if (*(float *)(unaff_EBX + 0x3a9ad3) != (this_00->m_MinSpeed).m_Value) {
      if ((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_TimerEvent.
          m_bRegistered == false) {
        pCVar5 = &((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
          pIVar10 = CBaseEdict::GetChangeAccessor(pCVar5);
          puVar6 = (ushort *)**(undefined4 **)(&DAT_00523a17 + unaff_EBX);
          if (pIVar10->m_iChangeInfoSerialNumber == *puVar6) {
            uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar4 = puVar6[uVar11 * 0x14 + 0x14];
            if (uVar4 == 0) {
LAB_00723dca:
              puVar6[(uint)uVar4 + uVar11 * 0x14 + 1] = 1000;
              puVar6[uVar11 * 0x14 + 0x14] = uVar4 + 1;
            }
            else if (puVar6[uVar11 * 0x14 + 1] != 1000) {
              local_12c = 0;
              do {
                uVar12 = local_12c + 1;
                local_12c = uVar12 & 0xffff;
                if ((ushort)uVar12 == uVar4) {
                  if (uVar4 != 0x13) goto LAB_00723dca;
                  pIVar10->m_iChangeInfoSerialNumber = 0;
                  pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar6[uVar11 * 0x14 + local_12c + 1] != 1000);
            }
          }
          else if (puVar6[0x7d1] == 100) {
            pIVar10->m_iChangeInfoSerialNumber = 0;
            pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar10->m_iChangeInfo = puVar6[0x7d1];
            *(short *)(**(int **)(&DAT_00523a17 + unaff_EBX) + 0xfa2) =
                 *(short *)(**(int **)(&DAT_00523a17 + unaff_EBX) + 0xfa2) + 1;
            piVar7 = *(int **)(&DAT_00523a17 + unaff_EBX);
            pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar7;
            iVar8 = *piVar7 + (uint)pIVar10->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar8 + 2) = 1000;
            *(undefined2 *)(iVar8 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this_00->m_MinSpeed).m_Value = 60.0;
    }
    if (*(float *)(unaff_EBX + 0x3a9ac7) != (this_00->m_MaxSpeed).m_Value) {
      if ((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_TimerEvent.
          m_bRegistered == false) {
        pCVar5 = &((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
          pIVar10 = CBaseEdict::GetChangeAccessor(pCVar5);
          puVar6 = (ushort *)**(undefined4 **)(&DAT_00523a17 + unaff_EBX);
          if (pIVar10->m_iChangeInfoSerialNumber == *puVar6) {
            uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar4 = puVar6[uVar11 * 0x14 + 0x14];
            if (uVar4 == 0) {
LAB_00723c3a:
              puVar6[(uint)uVar4 + uVar11 * 0x14 + 1] = 0x3ec;
              puVar6[uVar11 * 0x14 + 0x14] = uVar4 + 1;
            }
            else if (puVar6[uVar11 * 0x14 + 1] != 0x3ec) {
              local_128 = 0;
              do {
                uVar12 = local_128 + 1;
                local_128 = uVar12 & 0xffff;
                if ((ushort)uVar12 == uVar4) {
                  if (uVar4 != 0x13) goto LAB_00723c3a;
                  pIVar10->m_iChangeInfoSerialNumber = 0;
                  pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar6[uVar11 * 0x14 + local_128 + 1] != 0x3ec);
            }
          }
          else if (puVar6[0x7d1] == 100) {
            pIVar10->m_iChangeInfoSerialNumber = 0;
            pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar10->m_iChangeInfo = puVar6[0x7d1];
            *(short *)(**(int **)(&DAT_00523a17 + unaff_EBX) + 0xfa2) =
                 *(short *)(**(int **)(&DAT_00523a17 + unaff_EBX) + 0xfa2) + 1;
            piVar7 = *(int **)(&DAT_00523a17 + unaff_EBX);
            pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar7;
            iVar8 = *piVar7 + (uint)pIVar10->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar8 + 2) = 0x3ec;
            *(undefined2 *)(iVar8 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this_00->m_MaxSpeed).m_Value = 100.0;
    }
    fVar3 = *(float *)(&LAB_003a99af + unaff_EBX);
    if (((fVar3 != (this_00->m_StartColor).
                   super_CNetworkVectorCommonBase<Vector,SmokeTrail::NetworkVar_m_StartColor>.
                   super_CNetworkVarBase<Vector,SmokeTrail::NetworkVar_m_StartColor>.m_Value.x) ||
        (fVar3 != (this_00->m_StartColor).
                  super_CNetworkVectorCommonBase<Vector,SmokeTrail::NetworkVar_m_StartColor>.
                  super_CNetworkVarBase<Vector,SmokeTrail::NetworkVar_m_StartColor>.m_Value.y)) ||
       (fVar3 != (this_00->m_StartColor).
                 super_CNetworkVectorCommonBase<Vector,SmokeTrail::NetworkVar_m_StartColor>.
                 super_CNetworkVarBase<Vector,SmokeTrail::NetworkVar_m_StartColor>.m_Value.z)) {
      if ((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_TimerEvent.
          m_bRegistered == false) {
        pCVar5 = &((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
          pIVar10 = CBaseEdict::GetChangeAccessor(pCVar5);
          puVar6 = (ushort *)**(undefined4 **)(&DAT_00523a17 + unaff_EBX);
          if (pIVar10->m_iChangeInfoSerialNumber == *puVar6) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar4 = puVar6[uVar11 * 0x14 + 0x14];
            if (uVar4 == 0) {
LAB_0072410a:
              puVar6[(uint)uVar4 + uVar11 * 0x14 + 1] = 0x3c0;
              puVar6[uVar11 * 0x14 + 0x14] = uVar4 + 1;
            }
            else if (puVar6[uVar11 * 0x14 + 1] != 0x3c0) {
              local_124 = 0;
              do {
                uVar12 = local_124 + 1;
                local_124 = uVar12 & 0xffff;
                if ((ushort)uVar12 == uVar4) {
                  if (uVar4 == 0x13) goto LAB_00723ab6;
                  goto LAB_0072410a;
                }
              } while (puVar6[uVar11 * 0x14 + local_124 + 1] != 0x3c0);
            }
          }
          else if (puVar6[0x7d1] == 100) {
LAB_00723ab6:
            pIVar10->m_iChangeInfoSerialNumber = 0;
            pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar10->m_iChangeInfo = puVar6[0x7d1];
            *(short *)(**(int **)(&DAT_00523a17 + unaff_EBX) + 0xfa2) =
                 *(short *)(**(int **)(&DAT_00523a17 + unaff_EBX) + 0xfa2) + 1;
            piVar7 = *(int **)(&DAT_00523a17 + unaff_EBX);
            pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar7;
            iVar8 = *piVar7 + (uint)pIVar10->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar8 + 2) = 0x3c0;
            *(undefined2 *)(iVar8 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this_00->m_StartColor).
      super_CNetworkVectorCommonBase<Vector,SmokeTrail::NetworkVar_m_StartColor>.
      super_CNetworkVarBase<Vector,SmokeTrail::NetworkVar_m_StartColor>.m_Value.x = 0.5;
      (this_00->m_StartColor).
      super_CNetworkVectorCommonBase<Vector,SmokeTrail::NetworkVar_m_StartColor>.
      super_CNetworkVarBase<Vector,SmokeTrail::NetworkVar_m_StartColor>.m_Value.y = 0.5;
      (this_00->m_StartColor).
      super_CNetworkVectorCommonBase<Vector,SmokeTrail::NetworkVar_m_StartColor>.
      super_CNetworkVarBase<Vector,SmokeTrail::NetworkVar_m_StartColor>.m_Value.z = 0.5;
    }
    if ((((this_00->m_EndColor).
          super_CNetworkVectorCommonBase<Vector,SmokeTrail::NetworkVar_m_EndColor>.
          super_CNetworkVarBase<Vector,SmokeTrail::NetworkVar_m_EndColor>.m_Value.x != 0.0) ||
        (NAN((this_00->m_EndColor).
             super_CNetworkVectorCommonBase<Vector,SmokeTrail::NetworkVar_m_EndColor>.
             super_CNetworkVarBase<Vector,SmokeTrail::NetworkVar_m_EndColor>.m_Value.x))) ||
       ((pvVar2 = &(this_00->m_EndColor).
                   super_CNetworkVectorCommonBase<Vector,SmokeTrail::NetworkVar_m_EndColor>.
                   super_CNetworkVarBase<Vector,SmokeTrail::NetworkVar_m_EndColor>.m_Value.y,
        *pvVar2 != 0.0 ||
        ((NAN(*pvVar2) ||
         ((this_00->m_EndColor).
          super_CNetworkVectorCommonBase<Vector,SmokeTrail::NetworkVar_m_EndColor>.
          super_CNetworkVarBase<Vector,SmokeTrail::NetworkVar_m_EndColor>.m_Value.z != 0.0)))))) {
      if ((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_TimerEvent.
          m_bRegistered == false) {
        pCVar5 = &((this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
          pIVar10 = CBaseEdict::GetChangeAccessor(pCVar5);
          puVar6 = (ushort *)**(undefined4 **)(&DAT_00523a17 + unaff_EBX);
          if (pIVar10->m_iChangeInfoSerialNumber == *puVar6) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar4 = puVar6[uVar11 * 0x14 + 0x14];
            if (uVar4 == 0) {
LAB_007241ca:
              puVar6[(uint)uVar4 + uVar11 * 0x14 + 1] = 0x3cc;
              puVar6[uVar11 * 0x14 + 0x14] = uVar4 + 1;
            }
            else if (puVar6[uVar11 * 0x14 + 1] != 0x3cc) {
              local_120 = 0;
              do {
                uVar12 = local_120 + 1;
                local_120 = uVar12 & 0xffff;
                if ((ushort)uVar12 == uVar4) {
                  if (uVar4 == 0x13) goto LAB_00723aca;
                  goto LAB_007241ca;
                }
              } while (puVar6[uVar11 * 0x14 + local_120 + 1] != 0x3cc);
            }
          }
          else if (puVar6[0x7d1] == 100) {
LAB_00723aca:
            pIVar10->m_iChangeInfoSerialNumber = 0;
            pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar10->m_iChangeInfo = puVar6[0x7d1];
            *(short *)(**(int **)(&DAT_00523a17 + unaff_EBX) + 0xfa2) =
                 *(short *)(**(int **)(&DAT_00523a17 + unaff_EBX) + 0xfa2) + 1;
            piVar7 = *(int **)(&DAT_00523a17 + unaff_EBX);
            pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar7;
            iVar8 = *piVar7 + (uint)pIVar10->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar8 + 2) = 0x3cc;
            *(undefined2 *)(iVar8 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this_00->super_CBaseParticleEntity).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this_00->m_EndColor).super_CNetworkVectorCommonBase<Vector,SmokeTrail::NetworkVar_m_EndColor>
      .super_CNetworkVarBase<Vector,SmokeTrail::NetworkVar_m_EndColor>.m_Value.x = 0.0;
      (this_00->m_EndColor).super_CNetworkVectorCommonBase<Vector,SmokeTrail::NetworkVar_m_EndColor>
      .super_CNetworkVarBase<Vector,SmokeTrail::NetworkVar_m_EndColor>.m_Value.y = 0.0;
      (this_00->m_EndColor).super_CNetworkVectorCommonBase<Vector,SmokeTrail::NetworkVar_m_EndColor>
      .super_CNetworkVarBase<Vector,SmokeTrail::NetworkVar_m_EndColor>.m_Value.z = 0.0;
    }
    CBaseParticleEntity::SetLifetime(&this_00->super_CBaseParticleEntity,-1.0);
    SmokeTrail::FollowEntity(this_00,(CBaseEntity *)this,(char *)0x0);
    puVar9 = (ulong *)(*(this_00->super_CBaseParticleEntity).super_CBaseEntity.super_IServerEntity.
                        super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[3])(this_00);
    (this->m_hSmokeTrail).super_CBaseHandle.m_Index = *puVar9;
  }
  return;
}


/* CNPC_HoverTurret::DestroySmokeTrail at 00721500 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void DestroySmokeTrail(CNPC_HoverTurret * this) */

void __thiscall CNPC_HoverTurret::DestroySmokeTrail(CNPC_HoverTurret *this)

{
  uint uVar1;
  CBaseEntity *oldObj;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->m_hSmokeTrail).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x52565b),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (oldObj = *(CBaseEntity **)(iVar2 + 4), oldObj != (CBaseEntity *)0x0)) {
    UTIL_Remove(oldObj);
    (this->m_hSmokeTrail).super_CBaseHandle.m_Index = 0xffffffff;
    return;
  }
  return;
}


/* CNPC_HoverTurret::LoadSchedules at 00720450 */

bool CNPC_HoverTurret::LoadSchedules(void)

{
  int *piVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CAI_BaseNPC::LoadSchedules();
  piVar1 = *(int **)(unaff_EBX + 0x526c10);
  if (*(int *)(unaff_EBX + 0x667554) != *piVar1) {
    InitCustomSchedules();
    *(undefined1 *)(unaff_EBX + 0x667550) = 1;
    *(int *)(unaff_EBX + 0x667554) = *piVar1;
  }
  return (bool)*(undefined1 *)(unaff_EBX + 0x667550);
}


/* CNPC_HoverTurret::LoadedSchedules at 007204b0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: bool LoadedSchedules(CNPC_HoverTurret * this) */

bool __thiscall CNPC_HoverTurret::LoadedSchedules(CNPC_HoverTurret *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (bool)*(undefined1 *)(extraout_ECX + 0x6674fc);
}


/* CNPC_HoverTurret::SquadSlotName at 007205d0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: char * SquadSlotName(CNPC_HoverTurret * this, int slotEN) */

char * __thiscall CNPC_HoverTurret::SquadSlotName(CNPC_HoverTurret *this,int slotEN)

{
  int symbolID;
  char *pcVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  symbolID = CAI_LocalIdSpace::LocalToGlobal((CAI_LocalIdSpace *)(unaff_EBX + 0x6a8270),slotEN);
  pcVar1 = CAI_GlobalNamespace::IdToSymbol(*(CAI_GlobalNamespace **)(unaff_EBX + 0x526a48),symbolID)
  ;
  return pcVar1;
}


/* CNPC_HoverTurret::CScheduleLoader::CScheduleLoader at 00721190 */

/* DWARF original prototype: void CScheduleLoader(CScheduleLoader * this) */

void __thiscall CNPC_HoverTurret::CScheduleLoader::CScheduleLoader(CScheduleLoader *this)

{
  int *piVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CAI_BaseNPC::LoadSchedules();
  piVar1 = *(int **)(unaff_EBX + 0x525ed0);
  if (*(int *)(&DAT_00666814 + unaff_EBX) != *piVar1) {
    CNPC_HoverTurret::InitCustomSchedules();
    (&DAT_00666810)[unaff_EBX] = 1;
    *(int *)(&DAT_00666814 + unaff_EBX) = *piVar1;
  }
  return;
}


/* CNPC_HoverTurret::CScheduleLoader::CScheduleLoader at 007211e0 */

/* DWARF original prototype: void CScheduleLoader(CScheduleLoader * this) */

void __thiscall CNPC_HoverTurret::CScheduleLoader::CScheduleLoader(CScheduleLoader *this)

{
  int *piVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CAI_BaseNPC::LoadSchedules();
  piVar1 = *(int **)(unaff_EBX + 0x525e80);
  if (*(int *)(&DAT_006667c4 + unaff_EBX) != *piVar1) {
    CNPC_HoverTurret::InitCustomSchedules();
    (&DAT_006667c0)[unaff_EBX] = 1;
    *(int *)(&DAT_006667c4 + unaff_EBX) = *piVar1;
  }
  return;
}


/* CNPC_HoverTurret::InitCustomSchedules at 00724d70 */

void CNPC_HoverTurret::InitCustomSchedules(void)

{
  CAI_GlobalNamespace *pCVar1;
  AI_NamespaceAddInfo_t *pAVar2;
  char *pcVar3;
  CStringRegistry *pCVar4;
  CStringRegistry *pCVar5;
  bool bVar6;
  int iVar7;
  undefined4 *puVar8;
  int unaff_EBX;
  int iVar9;
  CAI_GlobalNamespace *pGlobalNamespace;
  int local_b0;
  undefined1 *local_a8;
  int local_a4;
  undefined1 *local_9c;
  int local_98;
  undefined1 *local_90;
  int local_8c;
  undefined1 *local_84;
  int local_80;
  int local_7c;
  int local_78;
  int local_74;
  int local_70;
  CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t,_int>_> local_6c;
  CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t,_int>_> local_58;
  void *local_44;
  undefined4 local_40;
  int local_3c;
  CAI_GlobalNamespace *local_38;
  void *local_34;
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
  local_44 = (void *)0x0;
  local_40 = 0;
  local_3c = 0;
  local_38 = (CAI_GlobalNamespace *)0x0;
  local_34 = (void *)0x0;
  local_58.m_Memory.m_pMemory = (AI_NamespaceAddInfo_t *)0x0;
  local_58.m_Memory.m_nAllocationCount = 0;
  local_58.m_Memory.m_nGrowSize = 0;
  local_58.m_Size = 0;
  local_58.m_pElements = (AI_NamespaceAddInfo_t *)0x0;
  local_6c.m_Memory.m_pMemory = (AI_NamespaceAddInfo_t *)0x0;
  local_6c.m_Memory.m_nAllocationCount = 0;
  local_6c.m_Memory.m_nGrowSize = 0;
  local_6c.m_Size = 0;
  local_6c.m_pElements = (AI_NamespaceAddInfo_t *)0x0;
  *(int *)(unaff_EBX + 0x6a3a7f) = unaff_EBX + 0x38c4b2;
  iVar9 = *(int *)(unaff_EBX + 0x5222ab);
  bVar6 = CAI_LocalIdSpace::Init
                    ((CAI_LocalIdSpace *)(unaff_EBX + 0x6a3a83),
                     *(CAI_GlobalNamespace **)(unaff_EBX + 0x52229f),(CAI_LocalIdSpace *)(iVar9 + 4)
                    );
  if ((bVar6) &&
     (bVar6 = CAI_LocalIdSpace::Init
                        ((CAI_LocalIdSpace *)(unaff_EBX + 0x6a3a9b),
                         (CAI_GlobalNamespace *)(*(int *)(unaff_EBX + 0x52229f) + 8),
                         (CAI_LocalIdSpace *)(iVar9 + 0x1c)), bVar6)) {
    CAI_LocalIdSpace::Init
              ((CAI_LocalIdSpace *)(unaff_EBX + 0x6a3ab3),
               (CAI_GlobalNamespace *)(*(int *)(unaff_EBX + 0x52229f) + 0x10),
               (CAI_LocalIdSpace *)(iVar9 + 0x34));
  }
  pGlobalNamespace = *(CAI_GlobalNamespace **)(unaff_EBX + 0x5222a3);
  CAI_LocalIdSpace::Init
            ((CAI_LocalIdSpace *)(unaff_EBX + 0x6a3acb),pGlobalNamespace,
             *(CAI_LocalIdSpace **)(unaff_EBX + 0x522503));
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
            iVar9 = CAI_NamespaceInfos::Compare(pAVar2,(AI_NamespaceAddInfo_t *)pCVar1);
            if (iVar9 < 0) {
              iVar9 = pAVar2->localId;
              pCVar4 = (CStringRegistry *)pAVar2->pszName;
              pCVar5 = pCVar1->m_pSymbols;
              pAVar2->localId = pCVar1->m_NextGlobalBase;
              pAVar2->pszName = (char *)pCVar5;
              pCVar1->m_NextGlobalBase = iVar9;
              pCVar1->m_pSymbols = pCVar4;
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
      _qsort(local_30,(size_t)local_24,8,*(int **)(unaff_EBX + 0x640547));
    }
  }
  if (1 < (int)local_38) {
    if (local_44 == (void *)0x0) {
      local_9c = (undefined1 *)((int)&local_38[-1].m_NextGlobalBase + 3);
      do {
        if (0 < (int)local_9c) {
          local_a4 = 1;
          local_78 = 0;
          do {
            pCVar1 = (CAI_GlobalNamespace *)((int)local_44 + local_a4 * 8);
            pAVar2 = (AI_NamespaceAddInfo_t *)((int)local_44 + local_78);
            pGlobalNamespace = pCVar1;
            iVar9 = CAI_NamespaceInfos::Compare(pAVar2,(AI_NamespaceAddInfo_t *)pCVar1);
            if (iVar9 < 0) {
              iVar9 = pAVar2->localId;
              pCVar4 = (CStringRegistry *)pAVar2->pszName;
              pCVar5 = pCVar1->m_pSymbols;
              pAVar2->localId = pCVar1->m_NextGlobalBase;
              pAVar2->pszName = (char *)pCVar5;
              pCVar1->m_NextGlobalBase = iVar9;
              pCVar1->m_pSymbols = pCVar4;
            }
            local_a4 = local_a4 + 1;
            local_78 = local_78 + 8;
          } while (local_a4 <= (int)local_9c);
        }
        local_9c = local_9c + -1;
      } while (local_9c != (undefined1 *)0xffffffff);
    }
    else {
      pGlobalNamespace = local_38;
      _qsort(local_44,(size_t)local_38,8,*(int **)(unaff_EBX + 0x640547));
    }
  }
  if (1 < local_58.m_Size) {
    if (local_58.m_Memory.m_pMemory == (AI_NamespaceAddInfo_t *)0x0) {
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
            iVar9 = CAI_NamespaceInfos::Compare(pAVar2,(AI_NamespaceAddInfo_t *)pCVar1);
            if (iVar9 < 0) {
              iVar9 = pAVar2->localId;
              pCVar4 = (CStringRegistry *)pAVar2->pszName;
              pCVar5 = pCVar1->m_pSymbols;
              pAVar2->localId = pCVar1->m_NextGlobalBase;
              pAVar2->pszName = (char *)pCVar5;
              pCVar1->m_NextGlobalBase = iVar9;
              pCVar1->m_pSymbols = pCVar4;
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
      _qsort(local_58.m_Memory.m_pMemory,local_58.m_Size,8,*(int **)(unaff_EBX + 0x640547));
    }
  }
  if (1 < local_6c.m_Size) {
    if ((CAI_GlobalNamespace *)local_6c.m_Memory.m_pMemory == (CAI_GlobalNamespace *)0x0) {
      local_84 = (undefined1 *)
                 ((int)&((CAI_GlobalNamespace *)(local_6c.m_Size + -8))->m_NextGlobalBase + 3);
      do {
        if (0 < (int)local_84) {
          local_8c = 1;
          local_80 = 0;
          do {
            pCVar1 = (CAI_GlobalNamespace *)(local_6c.m_Memory.m_pMemory + local_8c);
            pAVar2 = (AI_NamespaceAddInfo_t *)
                     ((int)&(local_6c.m_Memory.m_pMemory)->pszName + local_80);
            pGlobalNamespace = pCVar1;
            iVar9 = CAI_NamespaceInfos::Compare(pAVar2,(AI_NamespaceAddInfo_t *)pCVar1);
            if (iVar9 < 0) {
              iVar9 = pAVar2->localId;
              pCVar4 = (CStringRegistry *)pAVar2->pszName;
              pCVar5 = pCVar1->m_pSymbols;
              pAVar2->localId = pCVar1->m_NextGlobalBase;
              pAVar2->pszName = (char *)pCVar5;
              pCVar1->m_NextGlobalBase = iVar9;
              pCVar1->m_pSymbols = pCVar4;
            }
            local_8c = local_8c + 1;
            local_80 = local_80 + 8;
          } while (local_8c <= (int)local_84);
        }
        local_84 = local_84 + -1;
      } while (local_84 != (undefined1 *)0xffffffff);
    }
    else {
      pGlobalNamespace = (CAI_GlobalNamespace *)local_6c.m_Size;
      _qsort(local_6c.m_Memory.m_pMemory,local_6c.m_Size,8,*(int **)(unaff_EBX + 0x640547));
    }
  }
  if (0 < (int)local_24) {
    iVar9 = 0;
    do {
      puVar8 = (undefined4 *)(iVar9 * 8 + (int)local_30);
      pGlobalNamespace = (CAI_GlobalNamespace *)*puVar8;
      bVar6 = CAI_LocalIdSpace::AddSymbol
                        ((CAI_LocalIdSpace *)(unaff_EBX + 0x6a3a83),(char *)pGlobalNamespace,
                         puVar8[1],(char *)(unaff_EBX + 0x34b76b),*(char **)(unaff_EBX + 0x63fb3b));
      if (!bVar6) goto LAB_00724fe3;
      iVar9 = iVar9 + 1;
    } while (iVar9 < (int)local_24);
  }
  if (0 < (int)local_38) {
    iVar9 = 0;
    do {
      puVar8 = (undefined4 *)(iVar9 * 8 + (int)local_44);
      pGlobalNamespace = (CAI_GlobalNamespace *)*puVar8;
      bVar6 = CAI_LocalIdSpace::AddSymbol
                        ((CAI_LocalIdSpace *)(unaff_EBX + 0x6a3a9b),(char *)pGlobalNamespace,
                         puVar8[1],&UNK_00347a3f + unaff_EBX,*(char **)(unaff_EBX + 0x63fb3b));
      if (!bVar6) goto LAB_00724fe3;
      iVar9 = iVar9 + 1;
    } while (iVar9 < (int)local_38);
  }
  if (0 < local_58.m_Size) {
    local_70 = 0;
    iVar9 = *(int *)(unaff_EBX + 0x52229f);
    do {
      while( true ) {
        pcVar3 = local_58.m_Memory.m_pMemory[local_70].pszName;
        iVar7 = CAI_GlobalNamespace::NextGlobalBase((CAI_GlobalNamespace *)(iVar9 + 0x10));
        if (0xff < iVar7 + -1000000000) break;
        pGlobalNamespace = (CAI_GlobalNamespace *)local_58.m_Memory.m_pMemory[local_70].pszName;
        bVar6 = CAI_LocalIdSpace::AddSymbol
                          ((CAI_LocalIdSpace *)(unaff_EBX + 0x6a3ab3),(char *)pGlobalNamespace,
                           local_58.m_Memory.m_pMemory[local_70].localId,
                           (char *)(unaff_EBX + 0x34ea83),*(char **)(unaff_EBX + 0x63fb3b));
        if (!bVar6) goto LAB_00724fe3;
        local_70 = local_70 + 1;
        if (local_58.m_Size <= local_70) goto LAB_00725285;
      }
      pGlobalNamespace = (CAI_GlobalNamespace *)&section_000000dc.size;
      DevWarning(&UNK_0034245f + unaff_EBX,0x100,pcVar3);
      local_70 = local_70 + 1;
    } while (local_70 < local_58.m_Size);
  }
LAB_00725285:
  if (0 < local_6c.m_Size) {
    iVar9 = 0;
    do {
      pGlobalNamespace =
           (CAI_GlobalNamespace *)
           ((CAI_GlobalNamespace *)(local_6c.m_Memory.m_pMemory + iVar9))->m_pSymbols;
      bVar6 = CAI_LocalIdSpace::AddSymbol
                        ((CAI_LocalIdSpace *)(unaff_EBX + 0x6a3acb),(char *)pGlobalNamespace,
                         ((CAI_GlobalNamespace *)(local_6c.m_Memory.m_pMemory + iVar9))->
                         m_NextGlobalBase,(char *)(unaff_EBX + 0x3586d2),
                         *(char **)(unaff_EBX + 0x63fb3b));
      if (!bVar6) goto LAB_00724fe3;
      iVar9 = iVar9 + 1;
    } while (iVar9 < local_6c.m_Size);
  }
  local_6c.m_Size = 0;
  if (-1 < local_6c.m_Memory.m_nGrowSize) {
    if ((CAI_GlobalNamespace *)local_6c.m_Memory.m_pMemory != (CAI_GlobalNamespace *)0x0) {
      pGlobalNamespace = (CAI_GlobalNamespace *)local_6c.m_Memory.m_pMemory;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x521ddb) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x521ddb));
      local_6c.m_Memory.m_pMemory = (AI_NamespaceAddInfo_t *)0x0;
    }
    local_6c.m_Memory.m_nAllocationCount = 0;
  }
  local_6c.m_pElements = local_6c.m_Memory.m_pMemory;
  if (-1 < local_6c.m_Memory.m_nGrowSize) {
    if ((CAI_GlobalNamespace *)local_6c.m_Memory.m_pMemory != (CAI_GlobalNamespace *)0x0) {
      pGlobalNamespace = (CAI_GlobalNamespace *)local_6c.m_Memory.m_pMemory;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x521ddb) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x521ddb));
      local_6c.m_Memory.m_pMemory = (AI_NamespaceAddInfo_t *)0x0;
    }
    local_6c.m_Memory.m_nAllocationCount = 0;
  }
  CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t,_int>_>::~CUtlVector
            (&local_58,(int)pGlobalNamespace);
  local_38 = (CAI_GlobalNamespace *)0x0;
  if (-1 < local_3c) {
    if (local_44 != (void *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x521ddb) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x521ddb),local_44);
      local_44 = (void *)0x0;
    }
    local_40 = 0;
  }
  local_34 = local_44;
  if (-1 < local_3c) {
    if (local_44 != (void *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x521ddb) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x521ddb),local_44);
      local_44 = (void *)0x0;
    }
    local_40 = 0;
  }
  local_24 = (CAI_GlobalNamespace *)0x0;
  if (local_28 < 0) goto LAB_007250fe;
  if (local_30 != (void *)0x0) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x521ddb) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x521ddb),local_30);
    goto LAB_007250f0;
  }
LAB_007250f7:
  local_2c = 0;
LAB_007250fe:
  local_20 = local_30;
  if ((-1 < local_28) && (local_30 != (void *)0x0)) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x521ddb) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x521ddb),local_30);
  }
  return;
LAB_00724fe3:
  CUtlVector<AI_NamespaceAddInfo_t,CUtlMemory<AI_NamespaceAddInfo_t,_int>_>::~CUtlVector
            (&local_6c,(int)pGlobalNamespace);
  local_58.m_Size = 0;
  if (-1 < local_58.m_Memory.m_nGrowSize) {
    if (local_58.m_Memory.m_pMemory != (AI_NamespaceAddInfo_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x521ddb) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x521ddb),local_58.m_Memory.m_pMemory);
      local_58.m_Memory.m_pMemory = (AI_NamespaceAddInfo_t *)0x0;
    }
    local_58.m_Memory.m_nAllocationCount = 0;
  }
  local_58.m_pElements = local_58.m_Memory.m_pMemory;
  if (-1 < local_58.m_Memory.m_nGrowSize) {
    if (local_58.m_Memory.m_pMemory != (AI_NamespaceAddInfo_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x521ddb) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x521ddb),local_58.m_Memory.m_pMemory);
      local_58.m_Memory.m_pMemory = (AI_NamespaceAddInfo_t *)0x0;
    }
    local_58.m_Memory.m_nAllocationCount = 0;
  }
  local_38 = (CAI_GlobalNamespace *)0x0;
  if (-1 < local_3c) {
    if (local_44 != (void *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x521ddb) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x521ddb),local_44);
      local_44 = (void *)0x0;
    }
    local_40 = 0;
  }
  local_34 = local_44;
  if (-1 < local_3c) {
    if (local_44 != (void *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x521ddb) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x521ddb),local_44);
      local_44 = (void *)0x0;
    }
    local_40 = 0;
  }
  local_24 = (CAI_GlobalNamespace *)0x0;
  if (local_28 < 0) goto LAB_007250fe;
  if (local_30 != (void *)0x0) {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x521ddb) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x521ddb),local_30);
LAB_007250f0:
    local_30 = (void *)0x0;
  }
  goto LAB_007250f7;
}


/* _GLOBAL__I_sk_hover_turret_health at 000aca00 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_sk_hover_turret_health(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

