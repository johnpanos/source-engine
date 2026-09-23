/* DWARF-guided pseudocode for game/server/portal2/fizzler_effects.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000a9250 */

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
  longlong lVar8;
  ConVar *in_stack_ffffffd4;
  char *s2;
  char *in_stack_ffffffd8;
  char *in_stack_ffffffdc;
  SendTable *this;
  int in_stack_ffffffe0;
  undefined4 *puVar9;
  
  lVar8 = ___i686_get_pc_thunk_bx();
  if (lVar8 == 0xffff00000001) {
    *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4a3].m_pPrev + unaff_EBX) =
         0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4a3].m_pPrev + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4a3].m_pPrev + unaff_EBX + 2) = 0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4a3].m_pPrev + unaff_EBX + 3) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4a3].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4a4].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4a4].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4a4].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4a4].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4a5].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4a5].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4a5].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4a5].m_pNext + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4a6].m_pEntity + unaff_EBX)
         = 0x7f7fffff;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4a6].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4a6].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4a6].m_pNext + unaff_EBX) =
         &UNK_00ba30a4 + unaff_EBX;
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<CAI_Senses>::dataDesc[1].flatOffset + unaff_EBX + -0x18),
                   &UNK_00a074ba + unaff_EBX,&UNK_00a074b3 + unaff_EBX,0x4002,in_stack_ffffffd4,
                   in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0);
    ___cxa_atexit(unaff_EBX + 0x992f9c,0,*(undefined4 *)(&DAT_00b9d8f0 + unaff_EBX));
    ConVar::ConVar((ConVar *)((int)DataMapInit<CAI_Senses>::dataDesc[2].flatOffset + unaff_EBX + 8),
                   &UNK_00a074d8 + unaff_EBX,(char *)(unaff_EBX + 0x9b33d2),0x4002,in_stack_ffffffd4
                   ,in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0);
    ___cxa_atexit(unaff_EBX + 0x992f7c,0,*(undefined4 *)(&DAT_00b9d8f0 + unaff_EBX));
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<CAI_Senses>::dataDesc[4].flatOffset + unaff_EBX + -0x18),
                   (char *)(unaff_EBX + 0xa07500),&UNK_00a028f8 + unaff_EBX,0x4002,in_stack_ffffffd4
                   ,in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0);
    ___cxa_atexit(unaff_EBX + 0x992f5c,0,*(undefined4 *)(&DAT_00b9d8f0 + unaff_EBX));
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4a7].m_pEntity + unaff_EBX)
         = &UNK_00c1f10c + unaff_EBX;
    pIVar4 = EntityFactoryDictionary();
    (**pIVar4->_vptr_IEntityFactoryDictionary)(pIVar4,unaff_EBX + 0xda2654,unaff_EBX + 0xa07528);
    pdVar5 = DataMapInit<FizzlerEffects>((FizzlerEffects *)0x0);
    *(datamap_t **)((int)DataMapInit<CAI_Senses>::dataDesc[5].flatOffset + unaff_EBX) = pdVar5;
    iVar7 = unaff_EBX + 0xda2658;
    s2 = (char *)(unaff_EBX + 0xa074a4);
    *(char **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4a7].m_SerialNumber + unaff_EBX)
         = s2;
    this = (SendTable *)((int)DataMapInit<CAI_Senses>::dataDesc[5].flatOffset + unaff_EBX + 4);
    *(SendTable **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4a7].m_pPrev + unaff_EBX) =
         this;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4a8].m_SerialNumber + unaff_EBX) = 0xffff
    ;
    piVar1 = *(int **)(&DAT_00b9d990 + unaff_EBX);
    puVar9 = (undefined4 *)*piVar1;
    if (puVar9 == (undefined4 *)0x0) {
      *piVar1 = iVar7;
      *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4a7].m_pNext + unaff_EBX)
           = 0;
    }
    else {
      puVar2 = (undefined4 *)puVar9[2];
      iVar6 = _V_stricmp((char *)*puVar9,s2);
      if (iVar6 < 1) {
        while ((puVar3 = puVar2, puVar3 != (undefined4 *)0x0 &&
               (iVar6 = _V_stricmp((char *)*puVar3,s2), iVar6 < 1))) {
          puVar2 = (undefined4 *)puVar3[2];
          puVar9 = puVar3;
        }
        *(undefined4 **)(iVar7 + 8) = puVar3;
        puVar9[2] = iVar7;
      }
      else {
        *(int *)(iVar7 + 8) = *piVar1;
        *piVar1 = iVar7;
      }
    }
    SendTable::SendTable(this);
    ___cxa_atexit(unaff_EBX + 0x992f3c,0,*(undefined4 *)(&DAT_00b9d8f0 + unaff_EBX));
    iVar7 = ServerClassInit<DT_FizzlerEffects::ignored>((ignored *)0x0);
    *(int *)((int)DataMapInit<CAI_Senses>::dataDesc[6].flatOffset + unaff_EBX + -0x28) = iVar7;
  }
  return;
}


/* __tcf_2 at 00a3c200 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38c1d8),in_stack_00000008);
  return;
}


/* __tcf_3 at 00a3c1e0 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38c258),in_stack_00000008);
  return;
}


/* __tcf_4 at 00a3c1c0 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38c2d8),in_stack_00000008);
  return;
}


/* DataMapInit<FizzlerEffects> at 000a9040 */

datamap_t * DataMapInit<FizzlerEffects>(FizzlerEffects *param_1)

{
  int iVar1;
  int iVar2;
  char *pcVar3;
  size_t sVar4;
  int iVar5;
  undefined4 *puVar6;
  int iVar7;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4d5].m_pEntity + unaff_EBX) ==
       '\0') && (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xda2934), iVar2 != 0)) {
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4d6].m_pEntity + unaff_EBX)
         = &UNK_00a076b4 + unaff_EBX;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4d6].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4d6].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4d7].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4d7].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4d7].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4d6].m_SerialNumber + unaff_EBX) = 0xe;
    ___cxa_guard_release(unaff_EBX + 0xda2934);
    ___cxa_atexit(&UNK_009931cc + unaff_EBX,0,*(undefined4 *)(&DAT_00b9db00 + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_Unfreeze_00cb9f78 + unaff_EBX) =
       *(undefined4 *)(&DAT_00b9dc38 + unaff_EBX);
  if ((*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4d5].m_pPrev + unaff_EBX) ==
       '\0') && (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xda293c), iVar2 != 0)) {
    pcVar3 = operator_new___(*(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4d6].
                                            m_SerialNumber + unaff_EBX) + 0xc);
    _strcpy(pcVar3,*(char **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4d6].m_pEntity +
                             unaff_EBX));
    sVar4 = _strlen(pcVar3);
    builtin_strncpy(pcVar3 + sVar4,"FizzleThink",0xc);
    iVar1 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4d7].m_SerialNumber +
                    unaff_EBX);
    iVar2 = iVar1 + 1;
    iVar7 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4d6].m_pNext + unaff_EBX);
    if (iVar7 < iVar2) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)
                 ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4d6].m_pPrev + unaff_EBX),
                 iVar2 - iVar7);
    }
    iVar5 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4d7].m_SerialNumber +
                    unaff_EBX) + 1;
    *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4d7].m_SerialNumber + unaff_EBX) =
         iVar5;
    iVar7 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4d6].m_pPrev + unaff_EBX);
    *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4d7].m_pPrev + unaff_EBX) = iVar7;
    iVar5 = (iVar5 - iVar1) + -1;
    if (0 < iVar5) {
      _V_memmove((void *)(iVar7 + iVar2 * 4),(void *)(iVar7 + iVar1 * 4),iVar5 * 4);
      iVar7 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4d6].m_pPrev + unaff_EBX
                      );
    }
    puVar6 = (undefined4 *)(iVar7 + iVar1 * 4);
    if (puVar6 != (undefined4 *)0x0) {
      *puVar6 = pcVar3;
    }
    *(char **)(&DAT_00cb9fd0 + unaff_EBX) = pcVar3;
    ___cxa_guard_release(unaff_EBX + 0xda293c);
  }
  *(undefined4 *)((int)&PTR_IsFrozen_00cb9f70 + unaff_EBX) = 1;
  *(undefined **)((int)&PTR_Extinguish_00cb9f6c + unaff_EBX) = &UNK_00cb9fcc + unaff_EBX;
  return (datamap_t *)((int)&PTR_Extinguish_00cb9f6c + unaff_EBX);
}


/* FizzlerEffects::GetDataDescMap at 00712150 */

/* DWARF original prototype: datamap_t * GetDataDescMap(FizzlerEffects * this) */

datamap_t * __thiscall FizzlerEffects::GetDataDescMap(FizzlerEffects *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x650e68);
}


/* FizzlerEffects::GetBaseMap at 00712160 */

datamap_t * FizzlerEffects::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(CFuncTank::FuncTankPostThink + extraout_ECX + 4);
}


/* __tcf_0 at 00a3c220 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x40f77e)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x40f772) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x40f77e));
  }
  *(undefined4 *)(unaff_EBX + 0x40f77e) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40f77a)) {
    if (*(int *)(unaff_EBX + 0x40f772) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20a92e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20a92e),*(int *)(unaff_EBX + 0x40f772));
      *(undefined4 *)(unaff_EBX + 0x40f772) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f776) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40f772);
  *(int *)(CBaseEntity::SetTransmit + unaff_EBX + 2) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40f77a)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20a92e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20a92e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40f772) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f776) = 0;
  }
  return;
}


/* ServerClassInit<DT_FizzlerEffects::ignored> at 000a8eb0 */

int ServerClassInit<DT_FizzlerEffects::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4e2].m_pPrev + unaff_EBX) ==
      '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xda2a0c);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4e3].m_pPrev + unaff_EBX),
                  (char *)(vgui::CSizerBase::CSizerBase + unaff_EBX),0,4,-1,0,
                  (SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4e8].m_pNext +
                        unaff_EBX),(char *)(unaff_EBX + 0x9af120),0,
                        (SendTable *)**(undefined4 **)(&DAT_00b9dd88 + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00b9dd2c + unaff_EBX),0x80);
      ___cxa_guard_release(unaff_EBX + 0xda2a0c);
      ___cxa_atexit(&UNK_0099329c + unaff_EBX,0,*(undefined4 *)(&DAT_00b9dc90 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)((int)DataMapInit<CAI_Expresser>::dataDesc[2].flatOffset + unaff_EBX + 4),
             (SendProp_conflict *)
             ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4e8].m_pNext + unaff_EBX),1,
             *(char **)(&DAT_00c1f484 + unaff_EBX));
  return 1;
}


/* FizzlerEffects::GetServerClass at 00712170 */

/* DWARF original prototype: ServerClass * GetServerClass(FizzlerEffects * this) */

ServerClass * __thiscall FizzlerEffects::GetServerClass(FizzlerEffects *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x739744);
}


/* FizzlerEffects::YouForgotToImplementOrDeclareServerClass at 00712180 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(FizzlerEffects * this) */

int __thiscall FizzlerEffects::YouForgotToImplementOrDeclareServerClass(FizzlerEffects *this)

{
  return 0;
}


/* __tcf_5 at 00a3c1a0 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x38c354),in_stack_00000008);
  return;
}


/* __tcf_1 at 00a3c160 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f7c3))(unaff_EBX + 0x40f7c3);
                    /* WARNING: Could not recover jumptable at 0x00a3c196. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f76f))();
  return;
}


/* FizzlerEffects::FizzlerEffects at 00712270 */

/* DWARF original prototype: void FizzlerEffects(FizzlerEffects * this) */

void __thiscall FizzlerEffects::FizzlerEffects(FizzlerEffects *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::CBaseEntity(&this->super_CBaseEntity,false);
  (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5b5da7);
  this->m_ElapsedAnimationTime = 0.0;
  this->m_ScaleStarted = false;
  return;
}


/* FizzlerEffects::FizzlerEffects at 007122c0 */

/* DWARF original prototype: void FizzlerEffects(FizzlerEffects * this) */

void __thiscall FizzlerEffects::FizzlerEffects(FizzlerEffects *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::CBaseEntity(&this->super_CBaseEntity,false);
  (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5b5d57);
  this->m_ElapsedAnimationTime = 0.0;
  this->m_ScaleStarted = false;
  return;
}


/* CEntityFactory<FizzlerEffects>::Create at 007126a0 */

/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<FizzlerEffects> * this, char
   * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<FizzlerEffects>::Create(CEntityFactory<FizzlerEffects> *this,char *pClassName)

{
  CBaseEntity *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: FizzlerEffects * pEnt@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CBaseEntity::operator_new(0x3c8);
  CBaseEntity::CBaseEntity(this_00,false);
  (this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x5b5974);
  this_00[1].super_IServerEntity.super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)0x0;
  *(undefined1 *)&this_00[1].m_iObjectCapsCache.m_Value = 0;
  (**(code **)(unaff_EBX + 0x5b59e8))(this_00,pClassName);
  return &(this_00->m_Network).super_IServerNetworkable;
}


/* FizzlerEffects::Create at 007124f0 */

FizzlerEffects * FizzlerEffects::Create(CBaseEntity *pAttachTo)

{
  FizzlerEffects *this;
  CBaseAnimating *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: FizzlerEffects * pFizzlerEffects@[???] */
  ___i686_get_pc_thunk_bx();
  this = (FizzlerEffects *)CreateEntityByName((char *)(unaff_EBX + 0x39e288),-1,true);
  if (this != (FizzlerEffects *)0x0) {
                    /* Unresolved local var: CBaseAnimating * pDissolvingAnimating@[???] */
    (*(this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
      _vptr_IHandleEntity[0x26])(this,pAttachTo,0xffffffff);
    CBaseEntity::SetLocalOrigin((CBaseEntity *)this,*(Vector **)(unaff_EBX + 0x534674));
    CBaseEntity::SetLocalAngles((CBaseEntity *)this,*(QAngle **)(unaff_EBX + 0x534704));
    (*(this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
      _vptr_IHandleEntity[0x19])(this);
    (*(pAttachTo->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
      [0x39])(pAttachTo,unaff_EBX + 0x39e29c);
    this_00 = (CBaseAnimating *)
              (*(pAttachTo->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                _vptr_IHandleEntity[0x36])(pAttachTo);
    if (this_00 != (CBaseAnimating *)0x0) {
      CBaseEntity::AddFlag((CBaseEntity *)this_00,0x10000000);
      CBaseEntity::SetEffectEntity((CBaseEntity *)this_00,(CBaseEntity *)this);
      CBaseAnimating::ResetSequence(this_00,1);
    }
  }
  return this;
}


/* FizzlerEffects::Spawn at 007121f0 */

/* DWARF original prototype: void Spawn(FizzlerEffects * this) */

void __thiscall FizzlerEffects::Spawn(FizzlerEffects *this)

{
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::Spawn(&this->super_CBaseEntity);
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0x10f);
  CBaseEntity::ThinkSet(&this->super_CBaseEntity,func,0.0,(char *)0x0);
  CBaseEntity::SetNextThink
            (&this->super_CBaseEntity,*(float *)(**(int **)(unaff_EBX + 0x534963) + 0xc),(char *)0x0
            );
  return;
}


/* FizzlerEffects::AttachToEntity at 00712190 */

/* DWARF original prototype: void AttachToEntity(FizzlerEffects * this, CBaseEntity * pAttachTo) */

void __thiscall FizzlerEffects::AttachToEntity(FizzlerEffects *this,CBaseEntity *pAttachTo)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x26])(this,pAttachTo,0xffffffff);
  CBaseEntity::SetLocalOrigin(&this->super_CBaseEntity,*(Vector **)(unaff_EBX + 0x5349d7));
  CBaseEntity::SetLocalAngles(&this->super_CBaseEntity,*(QAngle **)(unaff_EBX + 0x534a67));
  return;
}


/* FizzlerEffects::FizzleThink at 00712310 */

/* DWARF original prototype: void FizzleThink(FizzlerEffects * this) */

void __thiscall FizzlerEffects::FizzleThink(FizzlerEffects *this)

{
  float fVar1;
  uint uVar2;
  int *piVar3;
  int iVar4;
  CBaseAnimating *this_00;
  model_t *pmVar5;
  CStudioHdr *pStudioHdr;
  int unaff_EBX;
  float fVar6;
  
                    /* Unresolved local var: CBaseAnimating * pDissolving@[???] */
  ___i686_get_pc_thunk_bx();
  uVar2 = (this->super_CBaseEntity).m_hMoveParent.
          super_CNetworkVarBase<CBaseHandle,CBaseEntity::NetworkVar_m_hMoveParent>.m_Value.m_Index;
  if ((((uVar2 == 0xffffffff) ||
       (iVar4 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x534848),
       *(uint *)(iVar4 + 8) != uVar2 >> 0x10)) ||
      (piVar3 = *(int **)(iVar4 + 4), piVar3 == (int *)0x0)) ||
     (this_00 = (CBaseAnimating *)(**(code **)(*piVar3 + 0xd8))(piVar3),
     this_00 == (CBaseAnimating *)0x0)) goto LAB_0071234a;
  fVar1 = this->m_ElapsedAnimationTime;
  fVar6 = CBaseAnimating::GetAnimTimeInterval(this_00);
  fVar6 = fVar6 + fVar1;
  this->m_ElapsedAnimationTime = fVar6;
  fVar1 = *(float *)(*(int *)(unaff_EBX + 0x6b6198) + 0x2c);
  if ((this->m_ScaleStarted == false) && (fVar1 <= fVar6)) {
    iVar4 = (this_00->m_nSequence).m_Value;
    pStudioHdr = this_00->m_pStudioHdr;
    if (pStudioHdr == (CStudioHdr *)0x0) {
      pmVar5 = CBaseEntity::GetModel((CBaseEntity *)this_00);
      if (pmVar5 != (model_t *)0x0) {
        CBaseAnimating::LockStudioHdr(this_00);
      }
      pStudioHdr = this_00->m_pStudioHdr;
      if (pStudioHdr != (CStudioHdr *)0x0) goto LAB_007123e2;
    }
    else {
LAB_007123e2:
      if (pStudioHdr->m_pStudioHdr == (studiohdr_t *)0x0) {
        pStudioHdr = (CStudioHdr *)0x0;
      }
    }
    fVar6 = CBaseAnimating::SequenceDuration(this_00,pStudioHdr,iVar4);
    CBaseAnimating::SetModelScale
              (this_00,*(float *)(*(int *)(unaff_EBX + 0x6b60d8) + 0x2c),
               (fVar6 - *(float *)(*(int *)(&DAT_006b6138 + unaff_EBX) + 0x2c)) - fVar1,
               NONHIERARCHICAL_MODEL_SCALE);
    this->m_ScaleStarted = true;
  }
  CBaseEntity::SetCollisionGroup((CBaseEntity *)this_00,0x10);
  (*(this_00->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[199])(this_00);
  if (this_00->m_bSequenceFinished == false) {
    CBaseEntity::SetNextThink
              (&this->super_CBaseEntity,*(float *)(**(int **)(unaff_EBX + 0x534840) + 0xc),
               (char *)0x0);
    return;
  }
  UTIL_Remove((CBaseEntity *)this_00);
LAB_0071234a:
  UTIL_Remove(&this->super_CBaseEntity);
  return;
}


/* _GLOBAL__I_fizzler_fx_model_end_scale at 000a9510 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_fizzler_fx_model_end_scale(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

