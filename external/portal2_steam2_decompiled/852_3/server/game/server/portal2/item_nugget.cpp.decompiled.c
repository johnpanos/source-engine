/* DWARF-guided pseudocode for game/server/portal2/item_nugget.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* CItem_Nugget::~CItem_Nugget at 006bbd30 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CItem_Nugget(CItem_Nugget * this, int __in_chrg) */

void __thiscall CItem_Nugget::~CItem_Nugget(CItem_Nugget *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x575b67);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CItem_Nugget::~CItem_Nugget at 006bbd70 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CItem_Nugget(CItem_Nugget * this, int __in_chrg) */

void __thiscall CItem_Nugget::~CItem_Nugget(CItem_Nugget *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x575b30);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
  return;
}


/* CEntityFactory<CItem_Nugget>::Create at 006bbd90 */

/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CItem_Nugget> * this, char *
   pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CItem_Nugget>::Create(CEntityFactory<CItem_Nugget> *this,char *pClassName)

{
  CBaseAnimating *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CItem_Nugget * pEnt@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CBaseEntity::operator_new(0x520);
  CBaseAnimating::CBaseAnimating(this_00);
  (this_00->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x575b04);
  CBaseEntity::PostConstructor(&this_00->super_CBaseEntity,pClassName);
  return &(this_00->super_CBaseEntity).m_Network.super_IServerNetworkable;
}


/* DataMapInit<CItem_Nugget> at 000c0200 */

datamap_t * DataMapInit<CItem_Nugget>(CItem_Nugget *param_1)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  char *pcVar4;
  size_t sVar5;
  undefined4 *puVar6;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((*(char *)((int)&mp_chattime.super_ConCommandBase.m_pszName + unaff_EBX) == '\0') &&
     (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xcf718c), iVar3 != 0)) {
    *(undefined **)((int)&mp_chattime.m_pParent + unaff_EBX) = &UNK_0094ee44 + unaff_EBX;
    *(undefined4 *)((int)&mp_chattime.m_Value.m_pszString + unaff_EBX) = 0;
    *(undefined4 *)((int)&mp_chattime.m_Value.m_StringLength + unaff_EBX) = 0;
    *(undefined4 *)((int)&mp_chattime.m_Value.m_fValue + unaff_EBX) = 0;
    *(undefined4 *)((int)&mp_chattime.m_Value.m_nValue + unaff_EBX) = 0;
    *(undefined4 *)(&mp_chattime.m_bHasMin + unaff_EBX) = 0;
    *(undefined4 *)((int)&mp_chattime.m_pszDefaultValue + unaff_EBX) = 0xc;
    ___cxa_guard_release(unaff_EBX + 0xcf718c);
    ___cxa_atexit(unaff_EBX + 0x8d99ec,0,*(undefined4 *)(&DAT_00aeb97c + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_Splash_00c0a1b8 + unaff_EBX) =
       *(undefined4 *)(&DAT_00aeb988 + unaff_EBX);
  if ((*(char *)((int)&mp_chattime.super_ConCommandBase.m_nFlags + unaff_EBX) == '\0') &&
     (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xcf7194), iVar3 != 0)) {
    pcVar4 = operator_new___(*(int *)((int)&mp_chattime.m_pszDefaultValue + unaff_EBX) + 0x11);
    _strcpy(pcVar4,*(char **)((int)&mp_chattime.m_pParent + unaff_EBX));
    sVar5 = _strlen(pcVar4);
    builtin_strncpy(pcVar4 + sVar5,"MaterializeThink",0x11);
    uVar1 = *(uint *)((int)&mp_chattime.m_Value.m_nValue + unaff_EBX);
    CUtlVector<char*,CUtlMemory<char*,_int>_>::GrowVector
              ((CUtlVector<char*,CUtlMemory<char*,_int>_> *)
               ((int)&mp_chattime.m_Value.m_pszString + unaff_EBX),1);
    iVar3 = ~uVar1 + *(int *)((int)&mp_chattime.m_Value.m_nValue + unaff_EBX);
    if (0 < iVar3) {
      iVar2 = *(int *)((int)&mp_chattime.m_Value.m_pszString + unaff_EBX);
      _V_memmove((void *)(iVar2 + 4 + uVar1 * 4),(void *)(iVar2 + uVar1 * 4),iVar3 * 4);
    }
    puVar6 = (undefined4 *)(uVar1 * 4 + *(int *)((int)&mp_chattime.m_Value.m_pszString + unaff_EBX))
    ;
    if (puVar6 != (undefined4 *)0x0) {
      *puVar6 = pcVar4;
    }
    *(char **)((int)&PTR_GetServerClass_00c0a2d0 + unaff_EBX) = pcVar4;
    ___cxa_guard_release(unaff_EBX + 0xcf7194);
  }
  *(undefined4 *)((int)&PTR_GetGroundVelocityToApply_00c0a1b0 + unaff_EBX) = 4;
  *(int *)((int)&PTR_OnGroundChanged_00c0a1ac + unaff_EBX) =
       (int)&PTR_GetStepAngles_00c0a20c + unaff_EBX;
  return (datamap_t *)((int)&PTR_OnGroundChanged_00c0a1ac + unaff_EBX);
}


/* __static_initialization_and_destruction_0 at 000c0420 */

/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  undefined4 uVar1;
  datamap_t *pdVar2;
  IEntityFactoryDictionary *pIVar3;
  int unaff_EBX;
  longlong lVar4;
  ConVar *in_stack_ffffffd8;
  char *in_stack_ffffffdc;
  char *in_stack_ffffffe0;
  int in_stack_ffffffe4;
  char *in_stack_ffffffe8;
  
  lVar4 = ___i686_get_pc_thunk_bx();
  if (lVar4 == 0xffff00000001) {
    *(undefined1 *)((int)&r_JeepViewZHeight.m_Value.m_fValue + unaff_EBX) = 0;
    *(undefined1 *)((int)&r_JeepViewZHeight.m_Value.m_fValue + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&r_JeepViewZHeight.m_Value.m_fValue + unaff_EBX + 2) = 0;
    *(undefined1 *)((int)&r_JeepViewZHeight.m_Value.m_fValue + unaff_EBX + 3) = 0;
    *(undefined4 *)((int)&r_JeepViewZHeight.m_Value.m_nValue + unaff_EBX) = 0;
    *(undefined4 *)(&r_JeepViewZHeight.m_bHasMin + unaff_EBX) = 0;
    *(undefined4 *)((int)&r_JeepViewZHeight.m_fMinVal + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&r_JeepViewZHeight.m_bHasMax + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&r_JeepViewZHeight.m_fMaxVal + unaff_EBX) = 0;
    *(undefined4 *)((int)&r_JeepViewZHeight.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&r_JeepViewZHeight.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) = 0;
    *(undefined4 *)((int)&r_JeepViewZHeight.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) =
         0;
    *(undefined4 *)((int)&r_JeepViewZHeight.m_fnChangeCallbacks.m_Size + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&r_JeepViewZHeight.m_fnChangeCallbacks.m_pElements + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)(&DAT_00cf6ed8 + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00cf6edc + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)
     ((int)&r_AirboatViewDampenFreq.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) = 0;
    *(undefined1 *)((int)&r_AirboatViewDampenFreq.super_ConCommandBase.m_pNext + unaff_EBX + 3) = 0;
    *(undefined1 *)((int)&r_AirboatViewDampenFreq.super_ConCommandBase.m_pNext + unaff_EBX) = 0;
    *(undefined1 *)((int)&r_AirboatViewDampenFreq.super_ConCommandBase.m_pNext + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&r_AirboatViewDampenFreq.super_ConCommandBase.m_pNext + unaff_EBX + 2) = 0;
    *(undefined4 *)(&r_AirboatViewDampenFreq.super_ConCommandBase.m_bRegistered + unaff_EBX) = 0;
    *(undefined1 *)((int)&r_AirboatViewDampenFreq.super_ConCommandBase.m_pszName + unaff_EBX + 3) =
         0;
    *(undefined1 *)((int)&r_AirboatViewDampenFreq.super_ConCommandBase.m_pszName + unaff_EBX) = 0;
    *(undefined1 *)((int)&r_AirboatViewDampenFreq.super_ConCommandBase.m_pszName + unaff_EBX + 1) =
         0;
    *(undefined1 *)((int)&r_AirboatViewDampenFreq.super_ConCommandBase.m_pszName + unaff_EBX + 2) =
         0;
    *(undefined4 *)((int)&r_AirboatViewDampenFreq.super_ConCommandBase.m_pszHelpString + unaff_EBX)
         = 0;
    *(undefined1 *)((int)&r_AirboatViewDampenFreq.super_ConCommandBase.m_nFlags + unaff_EBX + 3) = 1
    ;
    *(undefined1 *)((int)&r_AirboatViewDampenFreq.super_ConCommandBase.m_nFlags + unaff_EBX) = 0;
    *(undefined1 *)((int)&r_AirboatViewDampenFreq.super_ConCommandBase.m_nFlags + unaff_EBX + 1) = 0
    ;
    *(undefined1 *)((int)&r_AirboatViewDampenFreq.super_ConCommandBase.m_nFlags + unaff_EBX + 2) = 0
    ;
    *(undefined4 *)((int)&r_AirboatViewDampenFreq.super_IConVar._vptr_IConVar + unaff_EBX) = 1;
    *(undefined1 *)((int)&r_AirboatViewDampenFreq.m_pParent + unaff_EBX + 3) = 0;
    *(undefined1 *)((int)&r_AirboatViewDampenFreq.m_pParent + unaff_EBX) = 0;
    *(undefined1 *)((int)&r_AirboatViewDampenFreq.m_pParent + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&r_AirboatViewDampenFreq.m_pParent + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&r_AirboatViewDampenFreq.m_pszDefaultValue + unaff_EBX) = 2;
    *(undefined1 *)((int)&r_AirboatViewDampenFreq.m_Value.m_pszString + unaff_EBX + 3) = 0;
    *(undefined1 *)((int)&r_AirboatViewDampenFreq.m_Value.m_pszString + unaff_EBX) = 0;
    *(undefined1 *)((int)&r_AirboatViewDampenFreq.m_Value.m_pszString + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&r_AirboatViewDampenFreq.m_Value.m_pszString + unaff_EBX + 2) = 0;
    *(undefined **)((int)&r_AirboatViewDampenFreq.m_Value.m_StringLength + unaff_EBX) =
         &UNK_00af0ef4 + unaff_EBX;
    pdVar2 = DataMapInit<CItem_Nugget>((CItem_Nugget *)0x0);
    *(datamap_t **)((int)DataMapInit<CBaseCombatWeapon>::dataDesc[8].flatOffset + unaff_EBX + -0x28)
         = pdVar2;
    *(undefined **)((int)&r_AirboatViewDampenFreq.m_Value.m_fValue + unaff_EBX) =
         &UNK_00b71844 + unaff_EBX;
    pIVar3 = EntityFactoryDictionary();
    (**pIVar3->_vptr_IEntityFactoryDictionary)(pIVar3,unaff_EBX + 0xcf6f0c,unaff_EBX + 0x94ec34);
    CAutoGameSystem::CAutoGameSystem
              ((CAutoGameSystem *)
               ((int)&r_AirboatViewDampenFreq.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX),
               (char *)0x0);
    *(undefined **)
     ((int)&r_AirboatViewDampenFreq.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) =
         &UNK_00b71414 + unaff_EBX;
    *(undefined4 *)(&DAT_00cf6f38 + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00cf6f3c + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&r_AirboatViewDampenDamp.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) = 0;
    *(undefined4 *)((int)&r_AirboatViewDampenDamp.super_ConCommandBase.m_pNext + unaff_EBX) = 0;
    *(undefined4 *)(&r_AirboatViewDampenDamp.super_ConCommandBase.m_bRegistered + unaff_EBX) = 0;
    *(undefined4 *)((int)&r_AirboatViewDampenDamp.super_ConCommandBase.m_pszName + unaff_EBX) = 0;
    *(undefined4 *)((int)&r_AirboatViewDampenDamp.super_ConCommandBase.m_pszHelpString + unaff_EBX)
         = 0;
    *(undefined4 *)((int)&r_AirboatViewDampenDamp.super_ConCommandBase.m_nFlags + unaff_EBX) = 0;
    *(undefined4 *)((int)&r_AirboatViewDampenDamp.super_IConVar._vptr_IConVar + unaff_EBX) = 0;
    *(undefined4 *)((int)&r_AirboatViewDampenDamp.m_pParent + unaff_EBX) = 0;
    *(undefined4 *)((int)&r_AirboatViewDampenDamp.m_pszDefaultValue + unaff_EBX) = 0;
    *(undefined1 *)((int)&r_AirboatViewDampenDamp.m_Value.m_pszString + unaff_EBX) = 0;
    uVar1 = *(undefined4 *)(&DAT_00aeb75c + unaff_EBX);
    ___cxa_atexit(unaff_EBX + 0x8d98ac,0,uVar1);
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<CBaseCombatWeapon>::dataDesc[8].flatOffset + unaff_EBX + -8),
                   (char *)(unaff_EBX + 0x94ec80),(char *)(unaff_EBX + 0x8eb804),0,
                   (char *)(unaff_EBX + 0x94ec40),in_stack_ffffffd8,in_stack_ffffffdc,
                   in_stack_ffffffe0,in_stack_ffffffe4,in_stack_ffffffe8);
    ___cxa_atexit(unaff_EBX + 0x8d97ac,0,uVar1);
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<CBaseCombatWeapon>::dataDesc[10].flatOffset + unaff_EBX + -0x28
                   ),(char *)(unaff_EBX + 0x94ed04),
                   (char *)(vgui::MenuButton::DoClick + unaff_EBX + 4),0,
                   (char *)(unaff_EBX + 0x94eca4),in_stack_ffffffd8,in_stack_ffffffdc,
                   in_stack_ffffffe0,in_stack_ffffffe4,in_stack_ffffffe8);
    ___cxa_atexit(unaff_EBX + 0x8d978c,0,uVar1);
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<CBaseCombatWeapon>::dataDesc[0xb].flatOffset + unaff_EBX + -8),
                   (char *)(unaff_EBX + 0x94ed90),
                   (char *)(vgui::MenuButton::DoClick + unaff_EBX + 4),0,&UNK_0094ed3c + unaff_EBX,
                   in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0,in_stack_ffffffe4,
                   in_stack_ffffffe8);
    ___cxa_atexit(unaff_EBX + 0x8d976c,0,uVar1);
  }
  return;
}


/* CItem_Nugget::GetDataDescMap at 006bb020 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CItem_Nugget * this) */

datamap_t * __thiscall CItem_Nugget::GetDataDescMap(CItem_Nugget *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x60f398);
}


/* CItem_Nugget::GetBaseMap at 006bb030 */

datamap_t * CItem_Nugget::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(&DAT_004f0b64 + extraout_ECX);
}


/* __tcf_0 at 00999c00 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41d7b6)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41d7aa) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41d7b6));
  }
  *(undefined4 *)(unaff_EBX + 0x41d7b6) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41d7b2)) {
    if (*(int *)(unaff_EBX + 0x41d7aa) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x211f8a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x211f8a),*(int *)(unaff_EBX + 0x41d7aa));
      *(undefined4 *)(unaff_EBX + 0x41d7aa) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d7ae) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41d7aa);
  *(int *)(unaff_EBX + 0x41d7ba) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41d7b2)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x211f8a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x211f8a),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41d7aa) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d7ae) = 0;
  }
  return;
}


/* CNuggetDirector::~CNuggetDirector at 006bc380 */

/* DWARF original prototype: void ~CNuggetDirector(CNuggetDirector * this, int __in_chrg) */

void __thiscall CNuggetDirector::~CNuggetDirector(CNuggetDirector *this,int __in_chrg)

{
  CItem_Nugget **ppCVar1;
  int unaff_EBX;
  CItem_Nugget **in_stack_ffffffc8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(unaff_EBX + 0x5754ba);
  (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.m_Size =
       0;
  if (-1 < (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
           m_Memory.m_nGrowSize) {
    ppCVar1 = (this->m_GroupedNuggets).
              super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.m_Memory.m_pMemory;
    if (ppCVar1 != (CItem_Nugget **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4ef80a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4ef80a));
      (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
      m_Memory.m_pMemory = (CItem_Nugget **)0x0;
      in_stack_ffffffc8 = ppCVar1;
    }
    (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
    m_Memory.m_nAllocationCount = 0;
  }
  ppCVar1 = (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>
            .m_Memory.m_pMemory;
  (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
  m_pElements = ppCVar1;
  if (-1 < (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
           m_Memory.m_nGrowSize) {
    if (ppCVar1 != (CItem_Nugget **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4ef80a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4ef80a));
      (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
      m_Memory.m_pMemory = (CItem_Nugget **)0x0;
      in_stack_ffffffc8 = ppCVar1;
    }
    (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
    m_Memory.m_nAllocationCount = 0;
  }
  (this->m_UngroupedNuggets).m_Size = 0;
  if ((this->m_UngroupedNuggets).m_Memory.m_nGrowSize < 0) {
    ppCVar1 = (this->m_UngroupedNuggets).m_Memory.m_pMemory;
  }
  else {
    ppCVar1 = (this->m_UngroupedNuggets).m_Memory.m_pMemory;
    if (ppCVar1 != (CItem_Nugget **)0x0) {
      in_stack_ffffffc8 = ppCVar1;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4ef80a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4ef80a));
      ppCVar1 = (CItem_Nugget **)0x0;
      (this->m_UngroupedNuggets).m_Memory.m_pMemory = (CItem_Nugget **)0x0;
    }
    (this->m_UngroupedNuggets).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_UngroupedNuggets).m_pElements = ppCVar1;
  if (-1 < (this->m_UngroupedNuggets).m_Memory.m_nGrowSize) {
    if (ppCVar1 != (CItem_Nugget **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4ef80a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4ef80a));
      (this->m_UngroupedNuggets).m_Memory.m_pMemory = (CItem_Nugget **)0x0;
      in_stack_ffffffc8 = ppCVar1;
    }
    (this->m_UngroupedNuggets).m_Memory.m_nAllocationCount = 0;
  }
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(unaff_EBX + 0x4f1d1a);
  IGameSystem::~IGameSystem((IGameSystem *)this,(int)in_stack_ffffffc8);
  operator_delete(this);
  return;
}


/* CNuggetDirector::~CNuggetDirector at 006bc590 */

/* DWARF original prototype: void ~CNuggetDirector(CNuggetDirector * this, int __in_chrg) */

void __thiscall CNuggetDirector::~CNuggetDirector(CNuggetDirector *this,int __in_chrg)

{
  CItem_Nugget **ppCVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(unaff_EBX + 0x5752aa);
  (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.m_Size =
       0;
  if (-1 < (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
           m_Memory.m_nGrowSize) {
    ppCVar1 = (this->m_GroupedNuggets).
              super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.m_Memory.m_pMemory;
    if (ppCVar1 != (CItem_Nugget **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4ef5fa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4ef5fa),ppCVar1);
      (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
      m_Memory.m_pMemory = (CItem_Nugget **)0x0;
    }
    (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
    m_Memory.m_nAllocationCount = 0;
  }
  ppCVar1 = (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>
            .m_Memory.m_pMemory;
  (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
  m_pElements = ppCVar1;
  if (-1 < (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
           m_Memory.m_nGrowSize) {
    if (ppCVar1 != (CItem_Nugget **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4ef5fa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4ef5fa),ppCVar1);
      (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
      m_Memory.m_pMemory = (CItem_Nugget **)0x0;
    }
    (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
    m_Memory.m_nAllocationCount = 0;
  }
  (this->m_UngroupedNuggets).m_Size = 0;
  if ((this->m_UngroupedNuggets).m_Memory.m_nGrowSize < 0) {
    ppCVar1 = (this->m_UngroupedNuggets).m_Memory.m_pMemory;
  }
  else {
    ppCVar1 = (this->m_UngroupedNuggets).m_Memory.m_pMemory;
    if (ppCVar1 != (CItem_Nugget **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4ef5fa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4ef5fa),ppCVar1);
      (this->m_UngroupedNuggets).m_Memory.m_pMemory = (CItem_Nugget **)0x0;
    }
    ppCVar1 = (CItem_Nugget **)0x0;
    (this->m_UngroupedNuggets).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_UngroupedNuggets).m_pElements = ppCVar1;
  if (-1 < (this->m_UngroupedNuggets).m_Memory.m_nGrowSize) {
    if (ppCVar1 != (CItem_Nugget **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4ef5fa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4ef5fa),ppCVar1);
      (this->m_UngroupedNuggets).m_Memory.m_pMemory = (CItem_Nugget **)0x0;
    }
    (this->m_UngroupedNuggets).m_Memory.m_nAllocationCount = 0;
  }
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(unaff_EBX + 0x4f1b0a);
  IGameSystem::~IGameSystem((IGameSystem *)this,__in_chrg);
  return;
}


/* __tcf_1 at 00999ce0 */

void __tcf_1(void *param_1)

{
  int iVar1;
  int unaff_EBX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_bx();
  *(int *)(unaff_EBX + 0x41d672) = unaff_EBX + 0x297b5a;
  *(undefined4 *)(unaff_EBX + 0x41d69e) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41d69a)) {
    if (*(int *)(unaff_EBX + 0x41d692) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x211eaa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x211eaa),*(int *)(unaff_EBX + 0x41d692));
      *(undefined4 *)(unaff_EBX + 0x41d692) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d696) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41d692);
  *(int *)(unaff_EBX + 0x41d6a2) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41d69a)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x211eaa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x211eaa),iVar1);
      *(int *)(unaff_EBX + 0x41d692) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d696) = 0;
  }
  *(undefined4 *)(unaff_EBX + 0x41d68a) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41d686)) {
    if (*(int *)(unaff_EBX + 0x41d67e) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x211eaa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x211eaa),*(int *)(unaff_EBX + 0x41d67e));
      *(undefined4 *)(unaff_EBX + 0x41d67e) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d682) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41d67e);
  *(int *)(unaff_EBX + 0x41d68e) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41d686)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x211eaa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x211eaa),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41d67e) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d682) = 0;
  }
  *(int *)(unaff_EBX + 0x41d672) = unaff_EBX + 0x2143ba;
  IGameSystem::~IGameSystem((IGameSystem *)(unaff_EBX + 0x41d672),in_stack_00000008);
  return;
}


/* CItem_Nugget::Spawn at 006bb920 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "MoveCollide_t": Some values do not have unique names */
/* WARNING: Enum "MoveType_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(CItem_Nugget * this) */

void __thiscall CItem_Nugget::Spawn(CItem_Nugget *this)

{
  undefined1 *puVar1;
  NetworkVar_m_Collision *this_00;
  int iVar2;
  CBaseEdict *pCVar3;
  IChangeInfoAccessor *pIVar4;
  int unaff_EBX;
  float local_5c;
  vec_t local_58;
  float local_54;
  vec_t local_50;
  Vector local_4c;
  Vector local_40;
  Vector local_34;
  AngularImpulse local_28 [2];
  
                    /* Unresolved local var: float fScale@[???]
                       Unresolved local var: float fScaledExtent@[???]
                       Unresolved local var: Vector vModelOffset@[???] */
  ___i686_get_pc_thunk_bx();
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1a])(this);
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1b])(this,unaff_EBX + 0x3536ea);
  iVar2 = this->m_nValue;
  if (iVar2 < 5) {
    this->m_nValue = 1;
    if ((this->super_CBaseAnimating).m_nSkin.m_Value != 1) {
      if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
          false) {
        pCVar3 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if (pCVar3 != (CBaseEdict *)0x0) {
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
          pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
          pIVar4->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->super_CBaseAnimating).m_nSkin.m_Value = 1;
    }
    local_5c = *(float *)(unaff_EBX + 0x36ebd2);
    local_50 = *(vec_t *)(unaff_EBX + 0x36ec0e);
    local_54 = *(float *)(unaff_EBX + 0x3714fe);
    local_58 = *(vec_t *)(unaff_EBX + 0x36f072);
  }
  else if (iVar2 < 10) {
    this->m_nValue = 5;
    if ((this->super_CBaseAnimating).m_nSkin.m_Value != 2) {
      if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
          false) {
        pCVar3 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if (pCVar3 != (CBaseEdict *)0x0) {
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
          pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
          pIVar4->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->super_CBaseAnimating).m_nSkin.m_Value = 2;
    }
    local_5c = *(float *)(unaff_EBX + 0x36ebf6);
    local_50 = *(vec_t *)(unaff_EBX + 0x377806);
    local_54 = *(float *)(unaff_EBX + 0x37780a);
    local_58 = *(vec_t *)(unaff_EBX + 0x37780e);
  }
  else if (iVar2 < 0x19) {
    this->m_nValue = 10;
    if ((this->super_CBaseAnimating).m_nSkin.m_Value != 3) {
      if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
          false) {
        pCVar3 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if (pCVar3 != (CBaseEdict *)0x0) {
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
          pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
          pIVar4->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->super_CBaseAnimating).m_nSkin.m_Value = 3;
    }
    local_5c = *(float *)(unaff_EBX + 0x36fe4a);
    local_50 = *(vec_t *)(unaff_EBX + 0x377812);
    local_54 = *(float *)(unaff_EBX + 0x377816);
    local_58 = *(vec_t *)(unaff_EBX + 0x37781a);
  }
  else {
    this->m_nValue = 0x19;
    if ((this->super_CBaseAnimating).m_nSkin.m_Value != 4) {
      if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
          false) {
        pCVar3 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if (pCVar3 != (CBaseEdict *)0x0) {
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
          pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
          pIVar4->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->super_CBaseAnimating).m_nSkin.m_Value = 4;
    }
    local_5c = *(float *)(unaff_EBX + 0x36e9e2);
    local_50 = *(vec_t *)(unaff_EBX + 0x36f072);
    local_54 = *(float *)(unaff_EBX + 0x37781e);
    local_58 = *(vec_t *)(unaff_EBX + 0x370e96);
  }
  CBaseEntity::SetMoveType((CBaseEntity *)this,MOVETYPE_NOCLIP,MOVECOLLIDE_DEFAULT);
  this_00 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Collision;
  CCollisionProperty::SetSolid(&this_00->super_CCollisionProperty,SOLID_BBOX);
  CCollisionProperty::SetSolidFlags(&this_00->super_CCollisionProperty,0xc);
  CBaseEntity::SetCollisionGroup((CBaseEntity *)this,5);
  CBaseAnimating::SetModelScale(&this->super_CBaseAnimating,local_5c,0.0,HIERARCHICAL_MODEL_SCALE);
  local_28[0].x = 0.0;
  local_28[0].y = 0.0;
  local_28[0].z = 180.0;
  CBaseEntity::ApplyLocalAngularVelocityImpulse((CBaseEntity *)this,local_28);
  if (((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
                    /* Unresolved local var: Vector res@[???] */
  local_34.x = (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.x + 0.0;
  local_34.y = (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.y + 0.0;
  local_34.z = local_54 + (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.z;
  CBaseEntity::SetAbsOrigin((CBaseEntity *)this,&local_34);
                    /* Unresolved local var: Vector res@[???] */
  local_4c.x = local_50;
  local_4c.y = local_50;
  local_4c.z = local_58;
                    /* Unresolved local var: Vector res@[???] */
  local_40.x = local_54;
  local_40.y = local_54;
  local_40.z = 0.0;
  CBaseEntity::SetSize((CBaseEntity *)this,&local_40,&local_4c);
  CBaseAnimating::Spawn(&this->super_CBaseAnimating);
  CNuggetDirector::RegisterNugget((CNuggetDirector *)(unaff_EBX + 0x6fba32),this);
  return;
}


/* CItem_Nugget::UpdateOnRemove at 006bb1c0 */

/* DWARF original prototype: void UpdateOnRemove(CItem_Nugget * this) */

void __thiscall CItem_Nugget::UpdateOnRemove(CItem_Nugget *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CNuggetDirector::UnregisterNugget((CNuggetDirector *)(&UNK_006fc18f + unaff_EBX),this);
  CBaseEntity::UpdateOnRemove((CBaseEntity *)this);
  return;
}


/* CItem_Nugget::Precache at 006bb0d0 */

/* DWARF original prototype: void Precache(CItem_Nugget * this) */

void __thiscall CItem_Nugget::Precache(CItem_Nugget *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::PrecacheModel(&UNK_00353f3c + unaff_EBX);
  CBaseEntity::PrecacheScriptSound(&UNK_00353f61 + unaff_EBX);
  CBaseEntity::PrecacheScriptSound(&UNK_00353f6e + unaff_EBX);
  return;
}


/* CItem_Nugget::StartTouch at 006bb3a0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void StartTouch(CItem_Nugget * this, CBaseEntity * pOther) */

void __thiscall CItem_Nugget::StartTouch(CItem_Nugget *this,CBaseEntity *pOther)

{
  edict_t *peVar1;
  int iVar2;
  char *sz;
  soundlevel_t sVar3;
  CTeam *pCVar4;
  int unaff_EBX;
  float local_80;
  CRecipientFilter local_74;
  CRecipientFilter local_54;
  Vector local_34;
  Vector local_28 [2];
  
                    /* Unresolved local var: CBasePlayer * pPlayer@[???]
                       Unresolved local var: CSingleUserRecipientFilter user@[???]
                       Unresolved local var: CPASAttenuationFilter filter@[???] */
  ___i686_get_pc_thunk_bx();
  if (((this->super_CBaseAnimating).super_CBaseEntity.m_fEffects.m_Value & 0x20) == 0) {
    iVar2 = (*(pOther->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[0x55])(pOther);
    if ((char)iVar2 != '\0') {
      CRecipientFilter::CRecipientFilter(&local_54);
      local_54.super_IRecipientFilter._vptr_IRecipientFilter =
           (_func_int_varargs **)(CNPC_FloorTurret::LoadSchedules + unaff_EBX + 1);
      CRecipientFilter::AddRecipient(&local_54,(CBasePlayer *)pOther);
      CRecipientFilter::MakeReliable(&local_54);
      UserMessageBegin(&local_54.super_IRecipientFilter,
                       (char *)(CAI_BaseNPC::SetTurnActivity + unaff_EBX + 6));
      sz = (this->super_CBaseAnimating).super_CBaseEntity.m_iClassname.pszValue;
      if (sz == (char *)0x0) {
        sz = (char *)(unaff_EBX + 0x2f07a5);
      }
      MessageWriteString(sz);
      MessageEnd();
      (*(pOther->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
        [0x9e])(local_28,pOther);
      CRecipientFilter::CRecipientFilter(&local_74);
      local_74.super_IRecipientFilter._vptr_IRecipientFilter =
           (_func_int_varargs **)(CNPC_FloorTurret::HasPhysicsAttacker + unaff_EBX + 1);
      CRecipientFilter::AddRecipientsByPAS(&local_74,local_28);
      local_74.super_IRecipientFilter._vptr_IRecipientFilter =
           (_func_int_varargs **)(unaff_EBX + 0x4f2d91);
      sVar3 = CBaseEntity::LookupSoundLevel(&UNK_00353c86 + unaff_EBX);
      if ((int)sVar3 < 0x33) {
        if (sVar3 == SNDLVL_NONE) {
          local_80 = 0.0;
        }
        else {
          local_80 = *(float *)(unaff_EBX + 0x36f181);
        }
      }
      else {
        local_80 = *(float *)(unaff_EBX + 0x36f17d) / (float)(int)(sVar3 - SNDLVL_50dB);
      }
      (*(pOther->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
        [0x9e])(&local_34,pOther);
      CPASAttenuationFilter::Filter((CPASAttenuationFilter *)&local_74,&local_34,local_80);
      peVar1 = (pOther->m_Network).m_pPev;
                    /* Unresolved local var: int edictIndex@[???] */
      iVar2 = 0;
      if (peVar1 != (edict_t *)0x0) {
        iVar2 = (int)peVar1 - *(int *)(**(int **)(unaff_EBX + 0x4f07e9) + 0x58) >> 4;
      }
      CBaseEntity::EmitSound
                (&local_74.super_IRecipientFilter,iVar2,&UNK_00353c86 + unaff_EBX,(Vector *)0x0,0.0,
                 (float *)0x0);
      iVar2 = this->m_nValue;
      CBasePlayer::IncrementFragCount((CBasePlayer *)pOther,iVar2);
      pCVar4 = CBaseEntity::GetTeam(pOther);
      if (pCVar4 != (CTeam *)0x0) {
        pCVar4 = CBaseEntity::GetTeam(pOther);
        iVar2 = this->m_nValue;
        (*(pCVar4->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
          _vptr_IHandleEntity[0xd3])(pCVar4);
      }
      if (this->m_fRespawnTime <= 0.0) {
        UTIL_Remove((CBaseEntity *)this);
      }
      else {
        (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
          super_IHandleEntity._vptr_IHandleEntity[0x77])(this);
      }
      local_74.super_IRecipientFilter._vptr_IRecipientFilter =
           (_func_int_varargs **)(CNPC_FloorTurret::HasPhysicsAttacker + unaff_EBX + 1);
      CRecipientFilter::~CRecipientFilter(&local_74,iVar2);
      local_54.super_IRecipientFilter._vptr_IRecipientFilter =
           (_func_int_varargs **)(CNPC_FloorTurret::LoadSchedules + unaff_EBX + 1);
      CRecipientFilter::~CRecipientFilter(&local_54,iVar2);
    }
  }
  return;
}


/* CItem_Nugget::Respawn at 006bb040 */

/* DWARF original prototype: CBaseEntity * Respawn(CItem_Nugget * this) */

CBaseEntity * __thiscall CItem_Nugget::Respawn(CItem_Nugget *this)

{
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::AddEffects((CBaseEntity *)this,0x20);
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0x293);
  CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(**(int **)(&DAT_004f0b53 + unaff_EBX) + 0xc) + this->m_fRespawnTime,
             (char *)0x0);
  return (CBaseEntity *)this;
}


/* CItem_Nugget::MaterializeThink at 006bb2e0 */

/* DWARF original prototype: void MaterializeThink(CItem_Nugget * this) */

void __thiscall CItem_Nugget::MaterializeThink(CItem_Nugget *this)

{
  undefined1 *puVar1;
  uint uVar2;
  CBaseEdict *this_00;
  edict_t *peVar3;
  IChangeInfoAccessor *pIVar4;
  int unaff_EBX;
  uint uVar5;
  
  ___i686_get_pc_thunk_bx();
  if (((this->super_CBaseAnimating).super_CBaseEntity.m_fEffects.m_Value & 0x20) == 0) {
    return;
  }
  CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x353d59),0.0,(float *)0x0);
  uVar2 = (this->super_CBaseAnimating).super_CBaseEntity.m_fEffects.m_Value;
  uVar5 = uVar2 & 0xffffffdf;
  if (uVar2 != uVar5) {
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
    (this->super_CBaseAnimating).super_CBaseEntity.m_fEffects.m_Value = uVar5;
  }
  peVar3 = (this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev;
  if (peVar3 != (edict_t *)0x0) {
    (peVar3->super_CBaseEdict).m_fStateFlags = (peVar3->super_CBaseEdict).m_fStateFlags | 0x80;
  }
  CBaseEntity::DispatchUpdateTransmitState((CBaseEntity *)this);
  return;
}


/* CNuggetDirector::RegisterNugget at 006bb200 */

/* DWARF original prototype: void RegisterNugget(CNuggetDirector * this, CItem_Nugget * pNugget) */

void __thiscall CNuggetDirector::RegisterNugget(CNuggetDirector *this,CItem_Nugget *pNugget)

{
  char *s1;
  uint uVar1;
  int iVar2;
  CItem_Nugget **ppCVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  s1 = (pNugget->m_iGroupName).pszValue;
  if (((s1 == (char *)0x0) || (s1 == (char *)(unaff_EBX + 0x2f0948))) ||
     (iVar2 = _V_stricmp(s1,(char *)(unaff_EBX + 0x2f0948)), iVar2 == 0)) {
    uVar1 = (this->m_UngroupedNuggets).m_Size;
    CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>::GrowVector
              (&this->m_UngroupedNuggets,1);
    iVar2 = ~uVar1 + (this->m_UngroupedNuggets).m_Size;
    if (0 < iVar2) {
      ppCVar3 = (this->m_UngroupedNuggets).m_Memory.m_pMemory;
      _V_memmove(ppCVar3 + uVar1 + 1,ppCVar3 + uVar1,iVar2 * 4);
    }
    ppCVar3 = (this->m_UngroupedNuggets).m_Memory.m_pMemory + uVar1;
    if (ppCVar3 != (CItem_Nugget **)0x0) {
      *ppCVar3 = pNugget;
    }
  }
  else {
    CUtlSortVector<CItem_Nugget*,CNuggetDirector::CNuggetGroupSort>::Insert
              (&this->m_GroupedNuggets,&pNugget);
  }
  return;
}


/* CNuggetDirector::UnregisterNugget at 006bb110 */

/* DWARF original prototype: void UnregisterNugget(CNuggetDirector * this, CItem_Nugget * pNugget)
    */

void __thiscall CNuggetDirector::UnregisterNugget(CNuggetDirector *this,CItem_Nugget *pNugget)

{
  int *piVar1;
  char *s1;
  CItem_Nugget **ppCVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int unaff_EBX;
  int iVar6;
  
  ___i686_get_pc_thunk_bx();
  s1 = (pNugget->m_iGroupName).pszValue;
  if (((s1 != (char *)0x0) && (s1 != (char *)(unaff_EBX + 0x2f0a3e))) &&
     (iVar4 = _V_stricmp(s1,(char *)(unaff_EBX + 0x2f0a3e)), iVar4 != 0)) {
    CUtlSortVector<CItem_Nugget*,CNuggetDirector::CNuggetGroupSort>::Remove
              (&this->m_GroupedNuggets,&pNugget);
    return;
  }
  iVar4 = (this->m_UngroupedNuggets).m_Size;
  if (iVar4 < 1) {
    return;
  }
  ppCVar2 = (this->m_UngroupedNuggets).m_Memory.m_pMemory;
  if (*ppCVar2 == pNugget) {
    iVar5 = 0;
    iVar6 = 0;
  }
  else {
    iVar5 = 0;
    iVar3 = 4;
    do {
      iVar6 = iVar3;
      iVar5 = iVar5 + 1;
      if (iVar5 == iVar4) {
        return;
      }
      iVar3 = iVar6 + 4;
    } while (ppCVar2[iVar5] != pNugget);
  }
  if (iVar5 != iVar4 + -1) {
    *(CItem_Nugget **)(iVar6 + (int)ppCVar2) = ppCVar2[iVar4 + -1];
  }
  piVar1 = &(this->m_UngroupedNuggets).m_Size;
  *piVar1 = *piVar1 + -1;
  return;
}


/* CNuggetDirector::RandomizeAvailableNuggets at 006bb690 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void RandomizeAvailableNuggets(CNuggetDirector * this, float
   fUngroupedAvailabilityChance, float fGroupAvailabilityChance) */

void __thiscall
CNuggetDirector::RandomizeAvailableNuggets
          (CNuggetDirector *this,float fUngroupedAvailabilityChance,float fGroupAvailabilityChance)

{
  undefined1 *puVar1;
  int iVar2;
  CBaseEntity *this_00;
  CItem_Nugget *this_01;
  uint uVar3;
  edict_t *peVar4;
  CBaseEdict *pCVar5;
  bool bVar6;
  CItem_Nugget **ppCVar7;
  IChangeInfoAccessor *pIVar8;
  uint uVar9;
  longdouble lVar10;
  int local_30;
  int local_2c;
  char *local_28;
  int local_24;
  
                    /* Unresolved local var: int iCount@[???]
                       Unresolved local var: string_t iLastGroup@[???]
                       Unresolved local var: bool bExists@[???] */
  iVar2 = (this->m_UngroupedNuggets).m_Size;
  if (iVar2 != 0) {
    local_30 = 0;
    local_24 = 0;
    do {
      while (lVar10 = (longdouble)_RandomFloat(0,0x3f800000),
            (float)lVar10 <= fUngroupedAvailabilityChance) {
        this_00 = *(CBaseEntity **)((int)(this->m_UngroupedNuggets).m_Memory.m_pMemory + local_24);
        uVar3 = (this_00->m_fEffects).m_Value;
        uVar9 = uVar3 & 0xffffffdf;
        if (uVar3 != uVar9) {
          if ((this_00->m_Network).m_TimerEvent.m_bRegistered == false) {
            pCVar5 = &((this_00->m_Network).m_pPev)->super_CBaseEdict;
            if (pCVar5 != (CBaseEdict *)0x0) {
              pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
              pIVar8 = CBaseEdict::GetChangeAccessor(pCVar5);
              pIVar8->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            puVar1 = &(this_00->m_Network).field_0x4c;
            *(uint *)puVar1 = *(uint *)puVar1 | 1;
          }
          (this_00->m_fEffects).m_Value = uVar9;
        }
        peVar4 = (this_00->m_Network).m_pPev;
        if (peVar4 != (edict_t *)0x0) {
          (peVar4->super_CBaseEdict).m_fStateFlags = (peVar4->super_CBaseEdict).m_fStateFlags | 0x80
          ;
        }
        CBaseEntity::DispatchUpdateTransmitState(this_00);
        local_30 = local_30 + 1;
        local_24 = local_24 + 4;
        if (local_30 == iVar2) goto LAB_006bb761;
      }
      CBaseEntity::AddEffects
                (*(CBaseEntity **)((int)(this->m_UngroupedNuggets).m_Memory.m_pMemory + local_24),
                 0x20);
      local_30 = local_30 + 1;
      local_24 = local_24 + 4;
    } while (local_30 != iVar2);
  }
LAB_006bb761:
  iVar2 = (this->m_GroupedNuggets).super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.
          m_Size;
  local_28 = (char *)0x0;
  if (iVar2 != 0) {
    bVar6 = false;
    local_2c = 0;
    do {
      while( true ) {
        ppCVar7 = (this->m_GroupedNuggets).
                  super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.m_Memory.m_pMemory
        ;
        if (local_28 != (ppCVar7[local_2c]->m_iGroupName).pszValue) {
          local_28 = (ppCVar7[local_2c]->m_iGroupName).pszValue;
          lVar10 = (longdouble)_RandomFloat(0,0x3f800000);
          bVar6 = (float)lVar10 <= fGroupAvailabilityChance;
          ppCVar7 = (this->m_GroupedNuggets).
                    super_CUtlVector<CItem_Nugget*,CUtlMemory<CItem_Nugget*,_int>_>.m_Memory.
                    m_pMemory;
        }
        if (!bVar6) break;
        this_01 = ppCVar7[local_2c];
        uVar3 = (this_01->super_CBaseAnimating).super_CBaseEntity.m_fEffects.m_Value;
        uVar9 = uVar3 & 0xffffffdf;
        if (uVar3 != uVar9) {
          if (*(bool *)((int)&(this_01->super_CBaseAnimating).super_CBaseEntity.m_Network.
                              m_TimerEvent + 0x10) == false) {
            pCVar5 = &((this_01->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                      super_CBaseEdict;
            if (pCVar5 != (CBaseEdict *)0x0) {
              pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x101;
              pIVar8 = CBaseEdict::GetChangeAccessor(pCVar5);
              pIVar8->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            puVar1 = &(this_01->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
            *(uint *)puVar1 = *(uint *)puVar1 | 1;
          }
          (this_01->super_CBaseAnimating).super_CBaseEntity.m_fEffects.m_Value = uVar9;
        }
        peVar4 = (this_01->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev;
        if (peVar4 != (edict_t *)0x0) {
          (peVar4->super_CBaseEdict).m_fStateFlags = (peVar4->super_CBaseEdict).m_fStateFlags | 0x80
          ;
        }
        CBaseEntity::DispatchUpdateTransmitState((CBaseEntity *)this_01);
        local_2c = local_2c + 1;
        if (local_2c == iVar2) {
          return;
        }
      }
      CBaseEntity::AddEffects((CBaseEntity *)ppCVar7[local_2c],0x20);
      local_2c = local_2c + 1;
    } while (local_2c != iVar2);
  }
  return;
}


/* __tcf_2 at 00999be0 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x394d98),in_stack_00000008);
  return;
}


/* __tcf_3 at 00999bc0 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(&UNK_00394e18 + extraout_ECX),in_stack_00000008);
  return;
}


/* __tcf_4 at 00999ba0 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x394e98),in_stack_00000008);
  return;
}


/* CNuggetDirector::LevelInitPostEntity at 006bb8d0 */

/* DWARF original prototype: void LevelInitPostEntity(CNuggetDirector * this) */

void __thiscall CNuggetDirector::LevelInitPostEntity(CNuggetDirector *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(int *)(*(int *)(unaff_EBX + 0x6730c0) + 0x30) != 0) {
    RandomizeAvailableNuggets
              (this,*(float *)(*(int *)(unaff_EBX + 0x673120) + 0x2c),
               *(float *)(*(int *)(unaff_EBX + 0x673180) + 0x2c));
  }
  return;
}


/* _GLOBAL__I__ZN12CItem_Nugget9m_DataMapE at 000c0720 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN12CItem_Nugget9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

