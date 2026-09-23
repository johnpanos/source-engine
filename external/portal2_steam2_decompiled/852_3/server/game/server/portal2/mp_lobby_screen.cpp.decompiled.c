/* DWARF-guided pseudocode for game/server/portal2/mp_lobby_screen.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* CEntityFactory<CMP_LobbyScreen>::Create at 006be7d0 */

/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CMP_LobbyScreen> * this,
   char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CMP_LobbyScreen>::Create(CEntityFactory<CMP_LobbyScreen> *this,char *pClassName)

{
  CBaseEntity *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CMP_LobbyScreen * pEnt@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CBaseEntity::operator_new(0x3d8);
  CBaseEntity::CBaseEntity(this_00,false);
  (this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x573c84);
  this_00[1].m_iObjectCapsCache.m_Value = 0;
  this_00[1].m_pfnMoveDone.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)0x0;
  this_00[1].m_pfnMoveDone.__delta = 0;
  this_00[1].m_pfnThink.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)0x0;
  this_00[1].m_pfnThink.__delta = 0;
  (**(code **)(unaff_EBX + 0x573cf8))(this_00,pClassName);
  return &(this_00->m_Network).super_IServerNetworkable;
}


/* DataMapInit<CMP_LobbyScreen> at 000c10c0 */

datamap_t * DataMapInit<CMP_LobbyScreen>(CMP_LobbyScreen *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&sv_specaccelerate.m_Value.m_fValue + unaff_EBX + 3) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf66cf);
    if (iVar1 != 0) {
      *(int *)(&sv_specaccelerate.field_0x37 + unaff_EBX) = unaff_EBX + 0x94e27c;
      *(undefined4 *)(&sv_specaccelerate.field_0x3f + unaff_EBX) = 0;
      *(undefined4 *)((int)&sv_specaccelerate.m_fMaxVal + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&sv_specaccelerate.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&sv_specaccelerate.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX + 3) = 0
      ;
      *(undefined4 *)
       ((int)&sv_specaccelerate.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&sv_specaccelerate.m_fMinVal + unaff_EBX + 3) = 0xf;
      ___cxa_guard_release(unaff_EBX + 0xcf66cf);
      ___cxa_atexit(&UNK_008d90bf + unaff_EBX,0,*(undefined4 *)(&DAT_00aeaabf + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_BodyTarget_00c09718 + unaff_EBX + 3) =
       *(undefined4 *)(&DAT_00aeabfb + unaff_EBX);
  *(undefined4 *)((int)&PTR_LocalEyeAngles_00c09710 + unaff_EBX + 3) = 3;
  *(int *)((int)&PTR_EyeAngles_00c0970c + unaff_EBX + 3) = unaff_EBX + 0xc0976f;
  return (datamap_t *)((int)&PTR_EyeAngles_00c0970c + unaff_EBX + 3);
}


/* __static_initialization_and_destruction_0 at 000c1190 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *s2;
  undefined4 *puVar1;
  IEntityFactoryDictionary *pIVar2;
  datamap_t *pdVar3;
  int iVar4;
  int iVar5;
  int unaff_EBX;
  undefined4 *puVar6;
  undefined4 *puVar7;
  longlong lVar8;
  
  lVar8 = ___i686_get_pc_thunk_bx();
  if (lVar8 == 0xffff00000001) {
    (&sk_npc_dmg_gunship_to_plr.m_bHasMax)[unaff_EBX] = false;
    (&sk_npc_dmg_gunship_to_plr.field_0x3d)[unaff_EBX] = 0;
    (&sk_npc_dmg_gunship_to_plr.field_0x3e)[unaff_EBX] = 0;
    (&sk_npc_dmg_gunship_to_plr.field_0x3f)[unaff_EBX] = 0;
    *(undefined4 *)((int)&sk_npc_dmg_gunship_to_plr.m_fMaxVal + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&sk_npc_dmg_gunship_to_plr.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&sk_npc_dmg_gunship_to_plr.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)
     ((int)&sk_npc_dmg_gunship_to_plr.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&sk_npc_dmg_gunship_to_plr.m_fnChangeCallbacks.m_Size + unaff_EBX) = 0;
    *(undefined4 *)((int)&sk_npc_dmg_gunship_to_plr.m_fnChangeCallbacks.m_pElements + unaff_EBX) = 0
    ;
    *(undefined4 *)
     ((int)&g_VoiceGameMgrHelper.super_IVoiceGameMgrHelper._vptr_IVoiceGameMgrHelper + unaff_EBX) =
         0;
    *(undefined4 *)((int)&DT_Corpse::g_SendTable.m_pProps + unaff_EBX) = 0;
    *(undefined4 *)((int)&DT_Corpse::g_SendTable.m_nProps + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&DT_Corpse::g_SendTable.m_pNetTableName + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&DT_Corpse::g_SendTable.m_pPrecalc + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DT_Corpse::g_SendTable.field_0x10 + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&DT_Corpse::g_SendTableInit + unaff_EBX) = 0;
    *(undefined1 *)((int)&g_pBodyQueueHead + unaff_EBX + 3) = 0;
    *(undefined1 *)((int)&g_pBodyQueueHead + unaff_EBX) = 0;
    *(undefined1 *)((int)&g_pBodyQueueHead + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&g_pBodyQueueHead + unaff_EBX + 2) = 0;
    *(undefined4 *)(&DAT_00cf64b8 + unaff_EBX) = 0;
    (&DAT_00cf64bf)[unaff_EBX] = 0;
    (&DAT_00cf64bc)[unaff_EBX] = 0;
    (&DAT_00cf64bd)[unaff_EBX] = 0;
    (&DAT_00cf64be)[unaff_EBX] = 0;
    *(undefined4 *)
     ((int)&ice_falling_damage_scale.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) = 0;
    *(undefined1 *)((int)&ice_falling_damage_scale.super_ConCommandBase.m_pNext + unaff_EBX + 3) = 1
    ;
    *(undefined1 *)((int)&ice_falling_damage_scale.super_ConCommandBase.m_pNext + unaff_EBX) = 0;
    *(undefined1 *)((int)&ice_falling_damage_scale.super_ConCommandBase.m_pNext + unaff_EBX + 1) = 0
    ;
    *(undefined1 *)((int)&ice_falling_damage_scale.super_ConCommandBase.m_pNext + unaff_EBX + 2) = 0
    ;
    *(undefined4 *)(&ice_falling_damage_scale.super_ConCommandBase.m_bRegistered + unaff_EBX) = 1;
    *(undefined1 *)((int)&ice_falling_damage_scale.super_ConCommandBase.m_pszName + unaff_EBX + 3) =
         0;
    *(undefined1 *)((int)&ice_falling_damage_scale.super_ConCommandBase.m_pszName + unaff_EBX) = 0;
    *(undefined1 *)((int)&ice_falling_damage_scale.super_ConCommandBase.m_pszName + unaff_EBX + 1) =
         0;
    *(undefined1 *)((int)&ice_falling_damage_scale.super_ConCommandBase.m_pszName + unaff_EBX + 2) =
         0;
    *(undefined4 *)((int)&ice_falling_damage_scale.super_ConCommandBase.m_pszHelpString + unaff_EBX)
         = 2;
    *(undefined1 *)((int)&ice_falling_damage_scale.super_ConCommandBase.m_nFlags + unaff_EBX + 3) =
         0;
    *(undefined1 *)((int)&ice_falling_damage_scale.super_ConCommandBase.m_nFlags + unaff_EBX) = 0;
    *(undefined1 *)((int)&ice_falling_damage_scale.super_ConCommandBase.m_nFlags + unaff_EBX + 1) =
         0;
    *(undefined1 *)((int)&ice_falling_damage_scale.super_ConCommandBase.m_nFlags + unaff_EBX + 2) =
         0;
    *(undefined **)((int)&ice_falling_damage_scale.super_IConVar._vptr_IConVar + unaff_EBX) =
         &UNK_00af0184 + unaff_EBX;
    *(undefined **)((int)&ice_falling_damage_scale.m_pParent + unaff_EBX) =
         &UNK_00b7160c + unaff_EBX;
    pIVar2 = EntityFactoryDictionary();
    (**pIVar2->_vptr_IEntityFactoryDictionary)(pIVar2,unaff_EBX + 0xcf64dc,unaff_EBX + 0x94e1b9);
    pdVar3 = DataMapInit<CMP_LobbyScreen>((CMP_LobbyScreen *)0x0);
    *(datamap_t **)((int)&PTR_GetSizerClientArea_00c6d914 + unaff_EBX) = pdVar3;
    iVar5 = unaff_EBX + 0xcf64e0;
    s2 = (char *)(unaff_EBX + 0x94e1a9);
    *(char **)((int)&ice_falling_damage_scale.m_pszDefaultValue + unaff_EBX) = s2;
    *(SendTable **)((int)&ice_falling_damage_scale.m_Value.m_pszString + unaff_EBX) =
         (SendTable *)((int)&PTR_SetText_00c6d918 + unaff_EBX);
    *(undefined4 *)((int)&ice_falling_damage_scale.m_Value.m_nValue + unaff_EBX) = 0xffff;
    puVar7 = (undefined4 *)**(int **)(&DAT_00aeaa8c + unaff_EBX);
    if (puVar7 == (undefined4 *)0x0) {
      **(int **)(&DAT_00aeaa8c + unaff_EBX) = iVar5;
      *(undefined4 *)((int)&ice_falling_damage_scale.m_Value.m_StringLength + unaff_EBX) = 0;
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
        *(undefined4 **)((int)&ice_falling_damage_scale.m_Value.m_StringLength + unaff_EBX) = puVar6
        ;
        puVar7[2] = iVar5;
      }
      else {
        *(undefined4 *)((int)&ice_falling_damage_scale.m_Value.m_StringLength + unaff_EBX) =
             **(undefined4 **)(&DAT_00aeaa8c + unaff_EBX);
        **(int **)(&DAT_00aeaa8c + unaff_EBX) = iVar5;
      }
    }
    SendTable::SendTable((SendTable *)((int)&PTR_SetText_00c6d918 + unaff_EBX));
    ___cxa_atexit(&UNK_008d8fcc + unaff_EBX,0,*(undefined4 *)(&DAT_00aea9ec + unaff_EBX));
    iVar5 = ServerClassInit<DT_MP_LobbyScreen::ignored>((ignored *)0x0);
    *(int *)((int)&PTR_SetContentAlignment_00c6d92c + unaff_EBX) = iVar5;
  }
  return;
}


/* CMP_LobbyScreen::GetDataDescMap at 006bd940 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CMP_LobbyScreen * this) */

datamap_t * __thiscall CMP_LobbyScreen::GetDataDescMap(CMP_LobbyScreen *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x60ce98);
}


/* CMP_LobbyScreen::GetBaseMap at 006bd950 */

datamap_t * CMP_LobbyScreen::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4ee374);
}


/* __tcf_0 at 0099a190 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41d61e)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41d612) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41d61e));
  }
  *(undefined4 *)(unaff_EBX + 0x41d61e) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41d61a)) {
    if (*(int *)(unaff_EBX + 0x41d612) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2119fa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2119fa),*(int *)(unaff_EBX + 0x41d612));
      *(undefined4 *)(unaff_EBX + 0x41d612) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d616) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41d612);
  *(int *)(unaff_EBX + 0x41d622) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41d61a)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2119fa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2119fa),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41d612) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d616) = 0;
  }
  return;
}


/* ServerClassInit<DT_MP_LobbyScreen::ignored> at 000c0f00 */

int ServerClassInit<DT_MP_LobbyScreen::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&sv_specnoclip.m_Value.m_pszString + unaff_EBX) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf6784);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)((int)&sv_specnoclip.m_Value.m_fValue + unaff_EBX),
                  (char *)(unaff_EBX + 0x8f1b10),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)((int)&sv_maxspeed.m_pszDefaultValue + unaff_EBX),
                        (char *)(unaff_EBX + 0x8f3e80),0,
                        (SendTable *)**(undefined4 **)(&DAT_00aead74 + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00aead18 + unaff_EBX),0x80);
      SendPropBool((SendProp_conflict *)
                   ((int)&sv_accelerate.super_ConCommandBase.m_nFlags + unaff_EBX),
                   (char *)(unaff_EBX + 0x94c998),0x3c0,1);
      ___cxa_guard_release(unaff_EBX + 0xcf6784);
      ___cxa_atexit(unaff_EBX + 0x8d920c,0,*(undefined4 *)(&DAT_00aeac7c + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)
             ((int)DataMapInit<CompiledCaptionHeader_t>::dataDesc[1].flatOffset + unaff_EBX + -0x2c)
             ,(SendProp_conflict *)((int)&sv_maxspeed.m_pszDefaultValue + unaff_EBX),2,
             *(char **)(&DAT_00b71874 + unaff_EBX));
  return 1;
}


/* CMP_LobbyScreen::GetServerClass at 006bd960 */

/* DWARF original prototype: ServerClass * GetServerClass(CMP_LobbyScreen * this) */

ServerClass * __thiscall CMP_LobbyScreen::GetServerClass(CMP_LobbyScreen *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x6f9d1c);
}


/* CMP_LobbyScreen::YouForgotToImplementOrDeclareServerClass at 006bd970 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CMP_LobbyScreen * this) */

int __thiscall CMP_LobbyScreen::YouForgotToImplementOrDeclareServerClass(CMP_LobbyScreen *this)

{
  return 0;
}


/* __tcf_2 at 0099a170 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x394944),in_stack_00000008);
  return;
}


/* __tcf_1 at 0099a120 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d617))(unaff_EBX + 0x41d617);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d5c3))(unaff_EBX + 0x41d5c3);
                    /* WARNING: Could not recover jumptable at 0x0099a167. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d56f))();
  return;
}


/* CMP_LobbyScreen::~CMP_LobbyScreen at 006be300 */

/* DWARF original prototype: void ~CMP_LobbyScreen(CMP_LobbyScreen * this, int __in_chrg) */

void __thiscall CMP_LobbyScreen::~CMP_LobbyScreen(CMP_LobbyScreen *this,int __in_chrg)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  CVGuiScreen *pVGuiScreen;
  CHandle<CVGuiScreen> *pCVar5;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x57415a);
  iVar1 = (this->m_hScreens).m_Size;
  iVar3 = iVar1 * 4;
  while (iVar1 = iVar1 + -1, -1 < iVar1) {
    uVar2 = *(uint *)((int)&(this->m_hScreens).m_Memory.m_pMemory[-1].super_CBaseHandle.m_Index +
                     iVar3);
    if ((uVar2 == 0xffffffff) ||
       (iVar4 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4ed896),
       *(uint *)(iVar4 + 8) != uVar2 >> 0x10)) {
      pVGuiScreen = (CVGuiScreen *)0x0;
    }
    else {
      pVGuiScreen = *(CVGuiScreen **)(iVar4 + 4);
    }
    DestroyVGuiScreen(pVGuiScreen);
    iVar3 = iVar3 + -4;
  }
  (this->m_hScreens).m_Size = 0;
  if ((this->m_hScreens).m_Memory.m_nGrowSize < 0) {
    pCVar5 = (this->m_hScreens).m_Memory.m_pMemory;
  }
  else {
    pCVar5 = (this->m_hScreens).m_Memory.m_pMemory;
    if (pCVar5 != (CHandle<CVGuiScreen> *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4ed88a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4ed88a),pCVar5);
      (this->m_hScreens).m_Memory.m_pMemory = (CHandle<CVGuiScreen> *)0x0;
    }
    pCVar5 = (CHandle<CVGuiScreen> *)0x0;
    (this->m_hScreens).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_hScreens).m_pElements = pCVar5;
  if (-1 < (this->m_hScreens).m_Memory.m_nGrowSize) {
    if (pCVar5 != (CHandle<CVGuiScreen> *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4ed88a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4ed88a),pCVar5);
      (this->m_hScreens).m_Memory.m_pMemory = (CHandle<CVGuiScreen> *)0x0;
    }
    (this->m_hScreens).m_Memory.m_nAllocationCount = 0;
  }
  CBaseEntity::~CBaseEntity(&this->super_CBaseEntity,__in_chrg);
  return;
}


/* CMP_LobbyScreen::~CMP_LobbyScreen at 006be510 */

/* DWARF original prototype: void ~CMP_LobbyScreen(CMP_LobbyScreen * this, int __in_chrg,
   CMP_LobbyScreen * this) */

void __thiscall
CMP_LobbyScreen::~CMP_LobbyScreen(CMP_LobbyScreen *this,int __in_chrg,CMP_LobbyScreen *this_1)

{
  ~CMP_LobbyScreen(this,__in_chrg);
  return;
}


/* CMP_LobbyScreen::~CMP_LobbyScreen at 006be520 */

/* DWARF original prototype: void ~CMP_LobbyScreen(CMP_LobbyScreen * this, int __in_chrg) */

void __thiscall CMP_LobbyScreen::~CMP_LobbyScreen(CMP_LobbyScreen *this,int __in_chrg)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  CVGuiScreen *pVGuiScreen;
  CHandle<CVGuiScreen> *pCVar5;
  int unaff_EBX;
  CHandle<CVGuiScreen> *in_stack_ffffffc8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x573f3a);
  iVar1 = (this->m_hScreens).m_Size;
  iVar3 = iVar1 * 4;
  while (iVar1 = iVar1 + -1, -1 < iVar1) {
    uVar2 = *(uint *)((int)&(this->m_hScreens).m_Memory.m_pMemory[-1].super_CBaseHandle.m_Index +
                     iVar3);
    if ((uVar2 == 0xffffffff) ||
       (iVar4 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4ed676),
       *(uint *)(iVar4 + 8) != uVar2 >> 0x10)) {
      pVGuiScreen = (CVGuiScreen *)0x0;
    }
    else {
      pVGuiScreen = *(CVGuiScreen **)(iVar4 + 4);
    }
    DestroyVGuiScreen(pVGuiScreen);
    iVar3 = iVar3 + -4;
  }
  (this->m_hScreens).m_Size = 0;
  if ((this->m_hScreens).m_Memory.m_nGrowSize < 0) {
    pCVar5 = (this->m_hScreens).m_Memory.m_pMemory;
  }
  else {
    pCVar5 = (this->m_hScreens).m_Memory.m_pMemory;
    if (pCVar5 != (CHandle<CVGuiScreen> *)0x0) {
      in_stack_ffffffc8 = pCVar5;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4ed66a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4ed66a));
      pCVar5 = (CHandle<CVGuiScreen> *)0x0;
      (this->m_hScreens).m_Memory.m_pMemory = (CHandle<CVGuiScreen> *)0x0;
    }
    (this->m_hScreens).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_hScreens).m_pElements = pCVar5;
  if (-1 < (this->m_hScreens).m_Memory.m_nGrowSize) {
    if (pCVar5 != (CHandle<CVGuiScreen> *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4ed66a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4ed66a));
      (this->m_hScreens).m_Memory.m_pMemory = (CHandle<CVGuiScreen> *)0x0;
      in_stack_ffffffc8 = pCVar5;
    }
    (this->m_hScreens).m_Memory.m_nAllocationCount = 0;
  }
  CBaseEntity::~CBaseEntity(&this->super_CBaseEntity,(int)in_stack_ffffffc8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CMP_LobbyScreen::KeyValue at 006bda00 */

/* DWARF original prototype: bool KeyValue(CMP_LobbyScreen * this, char * szKeyName, char * szValue)
    */

bool __thiscall CMP_LobbyScreen::KeyValue(CMP_LobbyScreen *this,char *szKeyName,char *szValue)

{
  bool bVar1;
  char *pcVar2;
  int iVar3;
  int unaff_EBX;
  VMatrix local_f4;
  VMatrix local_b4;
  VMatrix local_74;
  Vector local_34;
  QAngle local_28 [2];
  
                    /* Unresolved local var: char * s@[???] */
  ___i686_get_pc_thunk_bx();
  pcVar2 = _strchr(szKeyName,0x23);
  if (pcVar2 != (char *)0x0) {
    *pcVar2 = '\0';
  }
  if (szKeyName != (char *)(unaff_EBX + 0x356fa1)) {
    iVar3 = _V_stricmp(szKeyName,(char *)(unaff_EBX + 0x356fa1));
    if (iVar3 != 0) {
      bVar1 = CBaseEntity::KeyValue(&this->super_CBaseEntity,szKeyName,szValue);
      return bVar1;
    }
  }
                    /* Unresolved local var: QAngle angles@[???]
                       Unresolved local var: VMatrix mat@[???]
                       Unresolved local var: VMatrix rotation@[???]
                       Unresolved local var: VMatrix tmp@[???] */
  UTIL_StringToVector(&local_28[0].x,szValue);
  MatrixFromAngles(local_28,&local_74);
  local_34.x = 0.0;
  local_34.y = 1.0;
  local_34.z = 0.0;
  MatrixBuildRotationAboutAxis(&local_b4,&local_34,90.0);
  MatrixMultiply(&local_74,&local_b4,&local_f4);
  MatrixBuildRotateZ(&local_b4,90.0);
  MatrixMultiply(&local_f4,&local_b4,&local_74);
  MatrixToAngles(&local_74,local_28);
  CBaseEntity::SetAbsAngles(&this->super_CBaseEntity,local_28);
  return true;
}


/* CMP_LobbyScreen::UpdateTransmitState at 006bd9e0 */

/* DWARF original prototype: int UpdateTransmitState(CMP_LobbyScreen * this) */

int __thiscall CMP_LobbyScreen::UpdateTransmitState(CMP_LobbyScreen *this)

{
  int iVar1;
  
  iVar1 = CBaseEntity::SetTransmitState(&this->super_CBaseEntity,0);
  return iVar1;
}


/* CMP_LobbyScreen::SetTransmit at 006bdb50 */

/* DWARF original prototype: void SetTransmit(CMP_LobbyScreen * this, CCheckTransmitInfo * pInfo,
   bool bAlways) */

void __thiscall
CMP_LobbyScreen::SetTransmit(CMP_LobbyScreen *this,CCheckTransmitInfo *pInfo,bool bAlways)

{
  edict_t *peVar1;
  int iVar2;
  uint uVar3;
  int *piVar4;
  int unaff_EBX;
  uint uVar5;
  int iVar6;
  
  ___i686_get_pc_thunk_bx();
  peVar1 = (this->super_CBaseEntity).m_Network.m_pPev;
  if (peVar1 == (edict_t *)0x0) {
    uVar5 = 0;
    uVar3 = 1;
  }
  else {
    uVar3 = (int)peVar1 - *(int *)(**(int **)(&DAT_004ee042 + unaff_EBX) + 0x58) >> 4;
    uVar5 = uVar3 >> 5;
    uVar3 = 1 << ((byte)uVar3 & 0x1f);
  }
  if (((pInfo->m_pTransmitEdict->super_CBitVecT<CFixedBitVecBase<2048>_>).
       super_CFixedBitVecBase<2048>.m_Ints[uVar5] & uVar3) == 0) {
    CBaseEntity::SetTransmit(&this->super_CBaseEntity,pInfo,bAlways);
    if (0 < (this->m_hScreens).m_Size) {
      iVar6 = 0;
      do {
        uVar3 = (this->m_hScreens).m_Memory.m_pMemory[iVar6].super_CBaseHandle.m_Index;
        if ((uVar3 == 0xffffffff) ||
           (iVar2 = (uVar3 & 0xffff) * 0x10 + **(int **)(&DAT_004ee046 + unaff_EBX),
           *(uint *)(iVar2 + 8) != uVar3 >> 0x10)) {
          piVar4 = (int *)0x0;
        }
        else {
          piVar4 = *(int **)(iVar2 + 4);
        }
        (**(code **)(*piVar4 + 0x5c))(piVar4,pInfo,bAlways);
        iVar6 = iVar6 + 1;
      } while (iVar6 < (this->m_hScreens).m_Size);
      return;
    }
  }
  return;
}


/* CMP_LobbyScreen::Spawn at 006be110 */

/* DWARF original prototype: void Spawn(CMP_LobbyScreen * this) */

void __thiscall CMP_LobbyScreen::Spawn(CMP_LobbyScreen *this)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  
  (*(this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x1a])(this);
  CBaseEntity::Spawn(&this->super_CBaseEntity);
  if ((this->m_bEnabled).m_Value == false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar2->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bEnabled).m_Value = true;
  }
  SpawnControlPanels(this);
  ScreenVisible(this,(this->m_bEnabled).m_Value);
  return;
}


/* CMP_LobbyScreen::Precache at 006bd9c0 */

/* DWARF original prototype: void Precache(CMP_LobbyScreen * this) */

void __thiscall CMP_LobbyScreen::Precache(CMP_LobbyScreen *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  PrecacheVGuiScreen((char *)(extraout_ECX + 0x351975));
  return;
}


/* CMP_LobbyScreen::OnRestore at 006be0d0 */

/* DWARF original prototype: void OnRestore(CMP_LobbyScreen * this) */

void __thiscall CMP_LobbyScreen::OnRestore(CMP_LobbyScreen *this)

{
  CBaseEntity::OnRestore(&this->super_CBaseEntity);
  RestoreControlPanels(this);
  ScreenVisible(this,(this->m_bEnabled).m_Value);
  return;
}


/* CMP_LobbyScreen::ScreenVisible at 006bdf20 */

/* DWARF original prototype: void ScreenVisible(CMP_LobbyScreen * this, bool bVisible) */

void __thiscall CMP_LobbyScreen::ScreenVisible(CMP_LobbyScreen *this,bool bVisible)

{
  undefined1 *puVar1;
  uint uVar2;
  edict_t *peVar3;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar4;
  int iVar5;
  uint uVar6;
  int unaff_EBX;
  CBaseEntity *pCVar7;
  int iVar8;
  
  ___i686_get_pc_thunk_bx();
  if (0 < (this->m_hScreens).m_Size) {
    iVar8 = 0;
    if (bVisible) {
      do {
        uVar2 = (this->m_hScreens).m_Memory.m_pMemory[iVar8].super_CBaseHandle.m_Index;
        if ((uVar2 == 0xffffffff) ||
           (iVar5 = (uVar2 & 0xffff) * 0x10 + **(int **)(&DAT_004edc76 + unaff_EBX),
           *(uint *)(iVar5 + 8) != uVar2 >> 0x10)) {
          pCVar7 = (CBaseEntity *)0x0;
        }
        else {
          pCVar7 = *(CBaseEntity **)(iVar5 + 4);
        }
        uVar2 = (pCVar7->m_fEffects).m_Value;
        uVar6 = uVar2 & 0xffffffdf;
        if (uVar2 != uVar6) {
          if ((pCVar7->m_Network).m_TimerEvent.m_bRegistered == false) {
            this_00 = &((pCVar7->m_Network).m_pPev)->super_CBaseEdict;
            if (this_00 != (CBaseEdict *)0x0) {
              this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
              pIVar4 = CBaseEdict::GetChangeAccessor(this_00);
              pIVar4->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            puVar1 = &(pCVar7->m_Network).field_0x4c;
            *(uint *)puVar1 = *(uint *)puVar1 | 1;
          }
          (pCVar7->m_fEffects).m_Value = uVar6;
        }
        peVar3 = (pCVar7->m_Network).m_pPev;
        if (peVar3 != (edict_t *)0x0) {
          (peVar3->super_CBaseEdict).m_fStateFlags = (peVar3->super_CBaseEdict).m_fStateFlags | 0x80
          ;
        }
        CBaseEntity::DispatchUpdateTransmitState(pCVar7);
        iVar8 = iVar8 + 1;
      } while (iVar8 < (this->m_hScreens).m_Size);
    }
    else {
      do {
        uVar2 = (this->m_hScreens).m_Memory.m_pMemory[iVar8].super_CBaseHandle.m_Index;
        if ((uVar2 == 0xffffffff) ||
           (iVar5 = (uVar2 & 0xffff) * 0x10 + **(int **)(&DAT_004edc76 + unaff_EBX),
           *(uint *)(iVar5 + 8) != uVar2 >> 0x10)) {
          pCVar7 = (CBaseEntity *)0x0;
        }
        else {
          pCVar7 = *(CBaseEntity **)(iVar5 + 4);
        }
        CBaseEntity::AddEffects(pCVar7,0x20);
        iVar8 = iVar8 + 1;
      } while (iVar8 < (this->m_hScreens).m_Size);
    }
  }
  return;
}


/* CMP_LobbyScreen::Disable at 006be060 */

/* DWARF original prototype: void Disable(CMP_LobbyScreen * this) */

void __thiscall CMP_LobbyScreen::Disable(CMP_LobbyScreen *this)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  
  if ((this->m_bEnabled).m_Value != false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar2->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bEnabled).m_Value = false;
    ScreenVisible(this,false);
  }
  return;
}


/* CMP_LobbyScreen::InputDisable at 006be280 */

/* DWARF original prototype: void InputDisable(CMP_LobbyScreen * this, inputdata_t * inputdata) */

void __thiscall CMP_LobbyScreen::InputDisable(CMP_LobbyScreen *this,inputdata_t *inputdata)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  
  if ((this->m_bEnabled).m_Value != false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar2->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bEnabled).m_Value = false;
    ScreenVisible(this,false);
    return;
  }
  return;
}


/* CMP_LobbyScreen::Enable at 006be210 */

/* DWARF original prototype: void Enable(CMP_LobbyScreen * this) */

void __thiscall CMP_LobbyScreen::Enable(CMP_LobbyScreen *this)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  
  if ((this->m_bEnabled).m_Value == false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar2->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bEnabled).m_Value = true;
    ScreenVisible(this,true);
  }
  return;
}


/* CMP_LobbyScreen::InputEnable at 006be1a0 */

/* DWARF original prototype: void InputEnable(CMP_LobbyScreen * this, inputdata_t * inputdata) */

void __thiscall CMP_LobbyScreen::InputEnable(CMP_LobbyScreen *this,inputdata_t *inputdata)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  
  if ((this->m_bEnabled).m_Value == false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar2->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bEnabled).m_Value = true;
    ScreenVisible(this,true);
    return;
  }
  return;
}


/* CMP_LobbyScreen::GetControlPanelInfo at 006bd980 */

/* DWARF original prototype: void GetControlPanelInfo(CMP_LobbyScreen * this, int nPanelIndex, char
   * * pPanelName) */

void __thiscall
CMP_LobbyScreen::GetControlPanelInfo(CMP_LobbyScreen *this,int nPanelIndex,char **pPanelName)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *pPanelName = (char *)(extraout_ECX + 0x3519b5);
  return;
}


/* CMP_LobbyScreen::GetControlPanelClassName at 006bd9a0 */

/* DWARF original prototype: void GetControlPanelClassName(CMP_LobbyScreen * this, int nPanelIndex,
   char * * pPanelName) */

void __thiscall
CMP_LobbyScreen::GetControlPanelClassName(CMP_LobbyScreen *this,int nPanelIndex,char **pPanelName)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *pPanelName = (char *)(extraout_ECX + 0x34fef8);
  return;
}


/* CMP_LobbyScreen::SpawnControlPanels at 006bdc40 */

/* DWARF original prototype: void SpawnControlPanels(CMP_LobbyScreen * this) */

void __thiscall CMP_LobbyScreen::SpawnControlPanels(CMP_LobbyScreen *this)

{
  int *piVar1;
  _func_int_varargs *p_Var2;
  uint uVar3;
  CVGuiScreen *this_00;
  int iVar4;
  int iVar5;
  CHandle<CVGuiScreen> *pCVar6;
  ulong *puVar7;
  int unaff_EBX;
  
                    /* Unresolved local var: int nPanel@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CreateVGuiScreen((char *)(unaff_EBX + 0x34fc52),(char *)(unaff_EBX + 0x3516ef),
                             &this->super_CBaseEntity,&this->super_CBaseEntity,-1);
  p_Var2 = (this_00->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity
           ._vptr_IHandleEntity[0x5e];
  iVar4 = CBaseEntity::GetTeamNumber(&this->super_CBaseEntity);
  (*p_Var2)(this_00,iVar4);
  CVGuiScreen::SetActualSize(this_00,190.0,94.0);
  CVGuiScreen::SetActive(this_00,true);
  CVGuiScreen::MakeVisibleOnlyToTeammates(this_00,false);
  CVGuiScreen::SetTransparency(this_00,true);
  uVar3 = (this->m_hScreens).m_Size;
  iVar4 = uVar3 + 1;
  iVar5 = (this->m_hScreens).m_Memory.m_nAllocationCount;
  if (iVar5 < iVar4) {
    CUtlMemory<CHandle<CVGuiScreen>,int>::Grow(&(this->m_hScreens).m_Memory,iVar4 - iVar5);
  }
  piVar1 = &(this->m_hScreens).m_Size;
  *piVar1 = *piVar1 + 1;
  pCVar6 = (this->m_hScreens).m_Memory.m_pMemory;
  (this->m_hScreens).m_pElements = pCVar6;
  iVar5 = ~uVar3 + (this->m_hScreens).m_Size;
  if (0 < iVar5) {
    _V_memmove(pCVar6 + iVar4,pCVar6 + uVar3,iVar5 * 4);
  }
  pCVar6 = (this->m_hScreens).m_Memory.m_pMemory + uVar3;
  if (pCVar6 != (CHandle<CVGuiScreen> *)0x0) {
    (pCVar6->super_CBaseHandle).m_Index = 0xffffffff;
  }
  pCVar6 = (this->m_hScreens).m_Memory.m_pMemory;
  puVar7 = (ulong *)(*(this_00->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.
                      super_IHandleEntity._vptr_IHandleEntity[3])(this_00);
  pCVar6[uVar3].super_CBaseHandle.m_Index = *puVar7;
  return;
}


/* CMP_LobbyScreen::RestoreControlPanels at 006bdd90 */

/* DWARF original prototype: void RestoreControlPanels(CMP_LobbyScreen * this) */

void __thiscall CMP_LobbyScreen::RestoreControlPanels(CMP_LobbyScreen *this)

{
  int *piVar1;
  uint uVar2;
  CVGuiScreen *this_00;
  int iVar3;
  CMP_LobbyScreen *pCVar4;
  char *s1;
  int iVar5;
  CHandle<CVGuiScreen> *pCVar6;
  ulong *puVar7;
  int unaff_EBX;
  
                    /* Unresolved local var: int nPanel@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = (CVGuiScreen *)
            CGlobalEntityList::FindEntityByClassname
                      (*(CGlobalEntityList **)(&DAT_004edec2 + unaff_EBX),(CBaseEntity *)0x0,
                       (char *)(unaff_EBX + 0x34fb02));
  do {
    if (this_00 == (CVGuiScreen *)0x0) {
LAB_006bde2c:
      s1 = CVGuiScreen::GetPanelName(this_00);
      iVar3 = _V_strcmp(s1,(char *)(unaff_EBX + 0x35159f));
      if (iVar3 == 0) {
        if (this_00 != (CVGuiScreen *)0x0) {
          uVar2 = (this->m_hScreens).m_Size;
          iVar3 = uVar2 + 1;
          iVar5 = (this->m_hScreens).m_Memory.m_nAllocationCount;
          if (iVar5 < iVar3) {
            CUtlMemory<CHandle<CVGuiScreen>,int>::Grow(&(this->m_hScreens).m_Memory,iVar3 - iVar5);
          }
          piVar1 = &(this->m_hScreens).m_Size;
          *piVar1 = *piVar1 + 1;
          pCVar6 = (this->m_hScreens).m_Memory.m_pMemory;
          (this->m_hScreens).m_pElements = pCVar6;
          iVar5 = ~uVar2 + (this->m_hScreens).m_Size;
          if (0 < iVar5) {
            _V_memmove(pCVar6 + iVar3,pCVar6 + uVar2,iVar5 * 4);
          }
          pCVar6 = (this->m_hScreens).m_Memory.m_pMemory + uVar2;
          if (pCVar6 != (CHandle<CVGuiScreen> *)0x0) {
            (pCVar6->super_CBaseHandle).m_Index = 0xffffffff;
          }
          pCVar6 = (this->m_hScreens).m_Memory.m_pMemory;
          puVar7 = (ulong *)(*(this_00->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.
                              super_IHandleEntity._vptr_IHandleEntity[3])(this_00);
          pCVar6[uVar2].super_CBaseHandle.m_Index = *puVar7;
          CVGuiScreen::SetActive(this_00,true);
        }
        return;
      }
    }
    else {
      uVar2 = (this_00->super_CBaseEntity).m_hOwnerEntity.
              super_CNetworkVarBase<CBaseHandle,CBaseEntity::NetworkVar_m_hOwnerEntity>.m_Value.
              m_Index;
      if ((uVar2 == 0xffffffff) ||
         (iVar3 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4ede06),
         *(uint *)(iVar3 + 8) != uVar2 >> 0x10)) {
        pCVar4 = (CMP_LobbyScreen *)0x0;
      }
      else {
        pCVar4 = *(CMP_LobbyScreen **)(iVar3 + 4);
      }
      if (pCVar4 == this) goto LAB_006bde2c;
    }
    this_00 = (CVGuiScreen *)
              CGlobalEntityList::FindEntityByClassname
                        (*(CGlobalEntityList **)(&DAT_004edec2 + unaff_EBX),
                         &this_00->super_CBaseEntity,(char *)(unaff_EBX + 0x34fb02));
  } while( true );
}


/* _GLOBAL__I__ZN15CMP_LobbyScreen9m_DataMapE at 000c1410 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN15CMP_LobbyScreen9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

