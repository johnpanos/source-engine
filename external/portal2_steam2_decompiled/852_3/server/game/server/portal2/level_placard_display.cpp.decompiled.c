/* DWARF-guided pseudocode for game/server/portal2/level_placard_display.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* CEntityFactory<CLevelPlacardDisplay>::Create at 006bd8c0 */

/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CLevelPlacardDisplay> *
   this, char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CLevelPlacardDisplay>::Create
          (CEntityFactory<CLevelPlacardDisplay> *this,char *pClassName)

{
  CBaseEntity *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CLevelPlacardDisplay * pEnt@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CBaseEntity::operator_new(0x448);
  CBaseEntity::CBaseEntity(this_00,false);
  (this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x574814);
  this_00[1].m_flCreateTime.m_Value = -NAN;
  (**(code **)(unaff_EBX + 0x574888))(this_00,pClassName);
  return &(this_00->m_Network).super_IServerNetworkable;
}


/* DataMapInit<CLevelPlacardDisplay> at 000c0b80 */

datamap_t * DataMapInit<CLevelPlacardDisplay>(CLevelPlacardDisplay *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&sv_rollspeed.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 3) == '\0'
     ) {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf6a4f);
    if (iVar1 != 0) {
      *(int *)((int)&sv_rollspeed.m_fnChangeCallbacks.m_pElements + unaff_EBX + 3) =
           unaff_EBX + 0x94e74e;
      *(undefined4 *)(&DAT_00cf6a5f + unaff_EBX) = 0;
      *(undefined4 *)((int)&sv_rollangle.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX + 3)
           = 0;
      *(undefined4 *)((int)&sv_rollangle.super_ConCommandBase.m_pNext + unaff_EBX + 3) = 0;
      *(undefined4 *)(&sv_rollangle.super_ConCommandBase.field_0xb + unaff_EBX) = 0;
      *(undefined4 *)((int)&sv_rollangle.super_ConCommandBase.m_pszName + unaff_EBX + 3) = 0;
      *(undefined4 *)(&DAT_00cf6a5b + unaff_EBX) = 0x14;
      ___cxa_guard_release(unaff_EBX + 0xcf6a4f);
      ___cxa_atexit(unaff_EBX + 0x8d94af,0,*(undefined4 *)(&DAT_00aeafff + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_PhysicsSimulate_00c09a38 + unaff_EBX + 3) =
       *(undefined4 *)(&DAT_00aeb13b + unaff_EBX);
  *(undefined4 *)((int)&PTR_Blocked_00c09a30 + unaff_EBX + 3) = 7;
  *(int *)((int)&PTR_StartBlocked_00c09a2c + unaff_EBX + 3) = unaff_EBX + 0xc09a8f;
  return (datamap_t *)((int)&PTR_StartBlocked_00c09a2c + unaff_EBX + 3);
}


/* __static_initialization_and_destruction_0 at 000c0c50 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *s2;
  SendTable *this;
  IEntityFactoryDictionary *pIVar1;
  datamap_t *pdVar2;
  int iVar3;
  int iVar4;
  int unaff_EBX;
  undefined4 *puVar5;
  undefined4 *puVar6;
  undefined4 *puVar7;
  longlong lVar8;
  
  lVar8 = ___i686_get_pc_thunk_bx();
  if (lVar8 == 0xffff00000001) {
    (&sv_maxspeed.m_bHasMax)[unaff_EBX] = false;
    (&sv_maxspeed.field_0x3d)[unaff_EBX] = 0;
    (&sv_maxspeed.field_0x3e)[unaff_EBX] = 0;
    (&sv_maxspeed.field_0x3f)[unaff_EBX] = 0;
    *(undefined4 *)((int)&sv_maxspeed.m_fMaxVal + unaff_EBX) = 0;
    *(undefined4 *)((int)&sv_maxspeed.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) = 0;
    *(undefined4 *)((int)&sv_maxspeed.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&sv_maxspeed.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&sv_maxspeed.m_fnChangeCallbacks.m_Size + unaff_EBX) = 0;
    *(undefined4 *)((int)&sv_maxspeed.m_fnChangeCallbacks.m_pElements + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00cf6818 + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00cf681c + unaff_EBX) = 0;
    *(undefined4 *)((int)&sv_accelerate.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&sv_accelerate.super_ConCommandBase.m_pNext + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&sv_accelerate.super_ConCommandBase.m_bRegistered + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&sv_accelerate.super_ConCommandBase.m_pszName + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&sv_accelerate.super_ConCommandBase.m_pszHelpString + unaff_EBX) = 0;
    *(undefined1 *)((int)&sv_accelerate.super_ConCommandBase.m_nFlags + unaff_EBX + 3) = 0;
    *(undefined1 *)((int)&sv_accelerate.super_ConCommandBase.m_nFlags + unaff_EBX) = 0;
    *(undefined1 *)((int)&sv_accelerate.super_ConCommandBase.m_nFlags + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&sv_accelerate.super_ConCommandBase.m_nFlags + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&sv_accelerate.super_IConVar._vptr_IConVar + unaff_EBX) = 0;
    *(undefined1 *)((int)&sv_accelerate.m_pParent + unaff_EBX + 3) = 0;
    *(undefined1 *)((int)&sv_accelerate.m_pParent + unaff_EBX) = 0;
    *(undefined1 *)((int)&sv_accelerate.m_pParent + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&sv_accelerate.m_pParent + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&sv_accelerate.m_pszDefaultValue + unaff_EBX) = 0;
    *(undefined1 *)((int)&sv_accelerate.m_Value.m_pszString + unaff_EBX + 3) = 1;
    *(undefined1 *)((int)&sv_accelerate.m_Value.m_pszString + unaff_EBX) = 0;
    *(undefined1 *)((int)&sv_accelerate.m_Value.m_pszString + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&sv_accelerate.m_Value.m_pszString + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&sv_accelerate.m_Value.m_StringLength + unaff_EBX) = 1;
    *(undefined1 *)((int)&sv_accelerate.m_Value.m_fValue + unaff_EBX + 3) = 0;
    *(undefined1 *)((int)&sv_accelerate.m_Value.m_fValue + unaff_EBX) = 0;
    *(undefined1 *)((int)&sv_accelerate.m_Value.m_fValue + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&sv_accelerate.m_Value.m_fValue + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&sv_accelerate.m_Value.m_nValue + unaff_EBX) = 2;
    (&sv_accelerate.field_0x37)[unaff_EBX] = 0;
    (&sv_accelerate.m_bHasMin)[unaff_EBX] = false;
    (&sv_accelerate.field_0x35)[unaff_EBX] = 0;
    (&sv_accelerate.field_0x36)[unaff_EBX] = 0;
    *(undefined **)((int)&sv_accelerate.m_fMinVal + unaff_EBX) = &UNK_00af06c4 + unaff_EBX;
    *(undefined **)(&sv_accelerate.m_bHasMax + unaff_EBX) = &UNK_00b717cc + unaff_EBX;
    pIVar1 = EntityFactoryDictionary();
    (**pIVar1->_vptr_IEntityFactoryDictionary)(pIVar1,unaff_EBX + 0xcf685c,unaff_EBX + 0x94e690);
    pdVar2 = DataMapInit<CLevelPlacardDisplay>((CLevelPlacardDisplay *)0x0);
    *(datamap_t **)
     ((int)DataMapInit<CPropTestChamberDoor>::dataDesc[0].flatOffset + unaff_EBX + -0x1c) = pdVar2;
    iVar4 = unaff_EBX + 0xcf6860;
    s2 = (char *)(unaff_EBX + 0x94e67b);
    *(char **)((int)&sv_accelerate.m_fMaxVal + unaff_EBX) = s2;
    this = (SendTable *)
           ((int)DataMapInit<CPropTestChamberDoor>::dataDesc[0].flatOffset + unaff_EBX + -0x18);
    *(SendTable **)((int)&sv_accelerate.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) = this;
    *(undefined4 *)((int)&sv_accelerate.m_fnChangeCallbacks.m_Size + unaff_EBX) = 0xffff;
    puVar7 = (undefined4 *)**(int **)(&DAT_00aeafcc + unaff_EBX);
    if (puVar7 == (undefined4 *)0x0) {
      **(int **)(&DAT_00aeafcc + unaff_EBX) = iVar4;
      *(undefined4 *)
       ((int)&sv_accelerate.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) = 0;
    }
    else {
      puVar6 = (undefined4 *)puVar7[2];
      iVar3 = _V_stricmp((char *)*puVar7,s2);
      if (iVar3 < 1) {
        if (puVar6 != (undefined4 *)0x0) {
          do {
            puVar5 = puVar6;
            iVar3 = _V_stricmp((char *)*puVar5,s2);
            puVar6 = puVar5;
            if (0 < iVar3) goto LAB_000c0ec0;
            puVar6 = (undefined4 *)puVar5[2];
            puVar7 = puVar5;
          } while ((undefined4 *)puVar5[2] != (undefined4 *)0x0);
          puVar6 = (undefined4 *)0x0;
        }
LAB_000c0ec0:
        *(undefined4 **)
         ((int)&sv_accelerate.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) = puVar6;
        puVar7[2] = iVar4;
      }
      else {
        *(undefined4 *)
         ((int)&sv_accelerate.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) =
             **(undefined4 **)(&DAT_00aeafcc + unaff_EBX);
        **(int **)(&DAT_00aeafcc + unaff_EBX) = iVar4;
      }
    }
    SendTable::SendTable(this);
    ___cxa_atexit(&UNK_008d93bc + unaff_EBX,0,*(undefined4 *)(&DAT_00aeaf2c + unaff_EBX));
    iVar4 = ServerClassInit<DT_LevelPlacardDisplay::ignored>((ignored *)0x0);
    *(int *)((int)DataMapInit<CPropTestChamberDoor>::dataDesc[0].flatOffset + unaff_EBX + -4) =
         iVar4;
  }
  return;
}


/* CLevelPlacardDisplay::GetDataDescMap at 006bcd40 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CLevelPlacardDisplay * this) */

datamap_t * __thiscall CLevelPlacardDisplay::GetDataDescMap(CLevelPlacardDisplay *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(&UNK_0060d878 + extraout_ECX);
}


/* CLevelPlacardDisplay::GetBaseMap at 006bcd50 */

datamap_t * CLevelPlacardDisplay::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4eef74);
}


/* __tcf_0 at 0099a040 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(&DAT_0041d5ae + unaff_EBX)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41d5a2) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(&DAT_0041d5ae + unaff_EBX));
  }
  *(undefined4 *)(&DAT_0041d5ae + unaff_EBX) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41d5aa)) {
    if (*(int *)(unaff_EBX + 0x41d5a2) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(&DAT_00211b4a + unaff_EBX) + 8))
                ((int *)**(undefined4 **)(&DAT_00211b4a + unaff_EBX),*(int *)(unaff_EBX + 0x41d5a2))
      ;
      *(undefined4 *)(unaff_EBX + 0x41d5a2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d5a6) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41d5a2);
  *(int *)(CBaseFlex::SetModel + unaff_EBX + 2) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41d5aa)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(&DAT_00211b4a + unaff_EBX) + 8))
                ((int *)**(undefined4 **)(&DAT_00211b4a + unaff_EBX),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41d5a2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d5a6) = 0;
  }
  return;
}


/* ServerClassInit<DT_LevelPlacardDisplay::ignored> at 000c09c0 */

/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */

int ServerClassInit<DT_LevelPlacardDisplay::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&sv_friction.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf6b04);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  ((int)&sv_friction.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX),
                  (char *)(unaff_EBX + 0x8f2050),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)((int)&sv_bounce.m_fMaxVal + unaff_EBX),
                        (char *)(vgui::Menu::LayoutScrollBar + unaff_EBX),0,
                        (SendTable *)**(undefined4 **)(&DAT_00aeb2b4 + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00aeb258 + unaff_EBX),0x80);
      SendPropBool((SendProp_conflict *)(&sv_maxvelocity.m_bHasMin + unaff_EBX),
                   (char *)(unaff_EBX + 0x94ced8),0x3c0,1);
      ___cxa_guard_release(unaff_EBX + 0xcf6b04);
      ___cxa_atexit(unaff_EBX + 0x8d95fc,0,*(undefined4 *)(&DAT_00aeb1bc + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)
             ((int)DataMapInit<CPropTestChamberDoor>::dataDesc[10].flatOffset + unaff_EBX + -8),
             (SendProp_conflict *)((int)&sv_bounce.m_fMaxVal + unaff_EBX),2,
             *(char **)(&DAT_00b71a34 + unaff_EBX));
  return 1;
}


/* CLevelPlacardDisplay::GetServerClass at 006bcd60 */

/* DWARF original prototype: ServerClass * GetServerClass(CLevelPlacardDisplay * this) */

ServerClass * __thiscall CLevelPlacardDisplay::GetServerClass(CLevelPlacardDisplay *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x6fa75c);
}


/* CLevelPlacardDisplay::YouForgotToImplementOrDeclareServerClass at 006bcd70 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CLevelPlacardDisplay *
   this) */

int __thiscall
CLevelPlacardDisplay::YouForgotToImplementOrDeclareServerClass(CLevelPlacardDisplay *this)

{
  return 0;
}


/* __tcf_2 at 0099a020 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x394a78),in_stack_00000008);
  return;
}


/* __tcf_1 at 00999fd0 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d5a7))(unaff_EBX + 0x41d5a7);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d553))(unaff_EBX + 0x41d553);
                    /* WARNING: Could not recover jumptable at 0x0099a017. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d4ff))();
  return;
}


/* CLevelPlacardDisplay::~CLevelPlacardDisplay at 006bd070 */

/* DWARF original prototype: void ~CLevelPlacardDisplay(CLevelPlacardDisplay * this, int __in_chrg)
    */

void __thiscall
CLevelPlacardDisplay::~CLevelPlacardDisplay(CLevelPlacardDisplay *this,int __in_chrg)

{
  uint uVar1;
  int iVar2;
  CVGuiScreen *pVGuiScreen;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x575064);
  uVar1 = (this->m_hScreen).super_CBaseHandle.m_Index;
  if ((uVar1 == 0xffffffff) ||
     (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4eeb20),
     *(uint *)(iVar2 + 8) != uVar1 >> 0x10)) {
    pVGuiScreen = (CVGuiScreen *)0x0;
  }
  else {
    pVGuiScreen = *(CVGuiScreen **)(iVar2 + 4);
  }
  DestroyVGuiScreen(pVGuiScreen);
  CBaseEntity::~CBaseEntity(&this->super_CBaseEntity,__in_chrg);
  return;
}


/* CLevelPlacardDisplay::~CLevelPlacardDisplay at 006bd240 */

/* DWARF original prototype: void ~CLevelPlacardDisplay(CLevelPlacardDisplay * this, int __in_chrg)
    */

void __thiscall
CLevelPlacardDisplay::~CLevelPlacardDisplay(CLevelPlacardDisplay *this,int __in_chrg)

{
  uint uVar1;
  int iVar2;
  CVGuiScreen *pVGuiScreen;
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x574e94);
  uVar1 = (this->m_hScreen).super_CBaseHandle.m_Index;
  if ((uVar1 == 0xffffffff) ||
     (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4ee950),
     *(uint *)(iVar2 + 8) != uVar1 >> 0x10)) {
    pVGuiScreen = (CVGuiScreen *)0x0;
  }
  else {
    pVGuiScreen = *(CVGuiScreen **)(iVar2 + 4);
  }
  DestroyVGuiScreen(pVGuiScreen);
  CBaseEntity::~CBaseEntity(&this->super_CBaseEntity,in_stack_ffffffd8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CLevelPlacardDisplay::~CLevelPlacardDisplay at 006bd2d0 */

/* DWARF original prototype: void ~CLevelPlacardDisplay(CLevelPlacardDisplay * this, int __in_chrg)
    */

void __thiscall
CLevelPlacardDisplay::~CLevelPlacardDisplay(CLevelPlacardDisplay *this,int __in_chrg)

{
  uint uVar1;
  int iVar2;
  CVGuiScreen *pVGuiScreen;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x574e04);
  uVar1 = (this->m_hScreen).super_CBaseHandle.m_Index;
  if ((uVar1 == 0xffffffff) ||
     (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4ee8c0),
     *(uint *)(iVar2 + 8) != uVar1 >> 0x10)) {
    pVGuiScreen = (CVGuiScreen *)0x0;
  }
  else {
    pVGuiScreen = *(CVGuiScreen **)(iVar2 + 4);
  }
  DestroyVGuiScreen(pVGuiScreen);
  CBaseEntity::~CBaseEntity(&this->super_CBaseEntity,__in_chrg);
  return;
}


/* CLevelPlacardDisplay::KeyValue at 006bd100 */

/* DWARF original prototype: bool KeyValue(CLevelPlacardDisplay * this, char * szKeyName, char *
   szValue) */

bool __thiscall
CLevelPlacardDisplay::KeyValue(CLevelPlacardDisplay *this,char *szKeyName,char *szValue)

{
  bool bVar1;
  int iVar2;
  int unaff_EBX;
  VMatrix local_f4;
  VMatrix local_b4;
  VMatrix local_74;
  Vector local_34;
  QAngle local_28 [2];
  
  ___i686_get_pc_thunk_bx();
  if (szKeyName != (char *)(unaff_EBX + 0x3578a1)) {
    iVar2 = _V_stricmp(szKeyName,(char *)(unaff_EBX + 0x3578a1));
    if (iVar2 != 0) {
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


/* CLevelPlacardDisplay::UpdateTransmitState at 006bd030 */

/* DWARF original prototype: int UpdateTransmitState(CLevelPlacardDisplay * this) */

int __thiscall CLevelPlacardDisplay::UpdateTransmitState(CLevelPlacardDisplay *this)

{
  int iVar1;
  
  if (this->m_bDoFullTransmit != false) {
    this->m_bDoFullTransmit = false;
    iVar1 = CBaseEntity::SetTransmitState(&this->super_CBaseEntity,8);
    return iVar1;
  }
  iVar1 = CBaseEntity::SetTransmitState(&this->super_CBaseEntity,0);
  return iVar1;
}


/* CLevelPlacardDisplay::SetTransmit at 006bcf40 */

/* DWARF original prototype: void SetTransmit(CLevelPlacardDisplay * this, CCheckTransmitInfo *
   pInfo, bool bAlways) */

void __thiscall
CLevelPlacardDisplay::SetTransmit(CLevelPlacardDisplay *this,CCheckTransmitInfo *pInfo,bool bAlways)

{
  edict_t *peVar1;
  int iVar2;
  uint uVar3;
  int *piVar4;
  int unaff_EBX;
  uint local_20;
  
  ___i686_get_pc_thunk_bx();
  peVar1 = (this->super_CBaseEntity).m_Network.m_pPev;
  if (peVar1 == (edict_t *)0x0) {
    local_20 = 0;
    uVar3 = 1;
  }
  else {
    uVar3 = (int)peVar1 - *(int *)(**(int **)(unaff_EBX + 0x4eec4c) + 0x58) >> 4;
    local_20 = uVar3 >> 5;
    uVar3 = 1 << ((byte)uVar3 & 0x1f);
  }
  if (((pInfo->m_pTransmitEdict->super_CBitVecT<CFixedBitVecBase<2048>_>).
       super_CFixedBitVecBase<2048>.m_Ints[local_20] & uVar3) != 0) {
    return;
  }
  CBaseEntity::SetTransmit(&this->super_CBaseEntity,pInfo,bAlways);
  uVar3 = (this->m_hScreen).super_CBaseHandle.m_Index;
  if ((uVar3 == 0xffffffff) ||
     (iVar2 = (uVar3 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4eec50),
     *(uint *)(iVar2 + 8) != uVar3 >> 0x10)) {
    piVar4 = (int *)0x0;
  }
  else {
    piVar4 = *(int **)(iVar2 + 4);
  }
                    /* WARNING: Could not recover jumptable at 0x006bd009. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*piVar4 + 0x5c))();
  return;
}


/* CLevelPlacardDisplay::Spawn at 006bd620 */

/* DWARF original prototype: void Spawn(CLevelPlacardDisplay * this) */

void __thiscall CLevelPlacardDisplay::Spawn(CLevelPlacardDisplay *this)

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
  this->m_bDoFullTransmit = true;
  return;
}


/* CLevelPlacardDisplay::Precache at 006bcf20 */

/* DWARF original prototype: void Precache(CLevelPlacardDisplay * this) */

void __thiscall CLevelPlacardDisplay::Precache(CLevelPlacardDisplay *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  PrecacheVGuiScreen((char *)(extraout_ECX + 0x3523a2));
  return;
}


/* CLevelPlacardDisplay::OnRestore at 006bd5e0 */

/* DWARF original prototype: void OnRestore(CLevelPlacardDisplay * this) */

void __thiscall CLevelPlacardDisplay::OnRestore(CLevelPlacardDisplay *this)

{
  CBaseEntity::OnRestore(&this->super_CBaseEntity);
  RestoreControlPanels(this);
  ScreenVisible(this,(this->m_bEnabled).m_Value);
  return;
}


/* CLevelPlacardDisplay::ScreenVisible at 006bd470 */

/* DWARF original prototype: void ScreenVisible(CLevelPlacardDisplay * this, bool bVisible) */

void __thiscall CLevelPlacardDisplay::ScreenVisible(CLevelPlacardDisplay *this,bool bVisible)

{
  undefined1 *puVar1;
  int *piVar2;
  uint uVar3;
  CBaseEdict *this_00;
  edict_t *peVar4;
  int iVar5;
  IChangeInfoAccessor *pIVar6;
  CVGuiScreen *this_01;
  int unaff_EBX;
  CBaseEntity *pCVar7;
  uint uVar8;
  
  ___i686_get_pc_thunk_bx();
  piVar2 = *(int **)(CEntityFactory<CItemSuit>::Destroy + unaff_EBX);
  uVar3 = (this->m_hScreen).super_CBaseHandle.m_Index;
  if ((uVar3 == 0xffffffff) ||
     (iVar5 = (uVar3 & 0xffff) * 0x10 + *piVar2, *(uint *)(iVar5 + 8) != uVar3 >> 0x10)) {
    this_01 = (CVGuiScreen *)0x0;
  }
  else {
    this_01 = *(CVGuiScreen **)(iVar5 + 4);
  }
  CVGuiScreen::SetActive(this_01,bVisible);
  if (bVisible) {
    uVar3 = (this->m_hScreen).super_CBaseHandle.m_Index;
    if ((uVar3 == 0xffffffff) ||
       (iVar5 = (uVar3 & 0xffff) * 0x10 + *piVar2, *(uint *)(iVar5 + 8) != uVar3 >> 0x10)) {
      pCVar7 = (CBaseEntity *)0x0;
    }
    else {
      pCVar7 = *(CBaseEntity **)(iVar5 + 4);
    }
    uVar3 = (pCVar7->m_fEffects).m_Value;
    uVar8 = uVar3 & 0xffffffdf;
    if (uVar3 != uVar8) {
      if ((pCVar7->m_Network).m_TimerEvent.m_bRegistered == false) {
        this_00 = &((pCVar7->m_Network).m_pPev)->super_CBaseEdict;
        if (this_00 != (CBaseEdict *)0x0) {
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
          pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
          pIVar6->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(pCVar7->m_Network).field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (pCVar7->m_fEffects).m_Value = uVar8;
    }
    peVar4 = (pCVar7->m_Network).m_pPev;
    if (peVar4 != (edict_t *)0x0) {
      (peVar4->super_CBaseEdict).m_fStateFlags = (peVar4->super_CBaseEdict).m_fStateFlags | 0x80;
    }
    CBaseEntity::DispatchUpdateTransmitState(pCVar7);
    return;
  }
  uVar3 = (this->m_hScreen).super_CBaseHandle.m_Index;
  if ((uVar3 == 0xffffffff) ||
     (iVar5 = (uVar3 & 0xffff) * 0x10 + *piVar2, *(uint *)(iVar5 + 8) != uVar3 >> 0x10)) {
    pCVar7 = (CBaseEntity *)0x0;
  }
  else {
    pCVar7 = *(CBaseEntity **)(iVar5 + 4);
  }
  CBaseEntity::AddEffects(pCVar7,0x20);
  return;
}


/* CLevelPlacardDisplay::Disable at 006bd720 */

/* DWARF original prototype: void Disable(CLevelPlacardDisplay * this) */

void __thiscall CLevelPlacardDisplay::Disable(CLevelPlacardDisplay *this)

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


/* CLevelPlacardDisplay::InputDisable at 006bd800 */

/* DWARF original prototype: void InputDisable(CLevelPlacardDisplay * this, inputdata_t * inputdata)
    */

void __thiscall
CLevelPlacardDisplay::InputDisable(CLevelPlacardDisplay *this,inputdata_t *inputdata)

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


/* CLevelPlacardDisplay::Enable at 006bd6b0 */

/* DWARF original prototype: void Enable(CLevelPlacardDisplay * this) */

void __thiscall CLevelPlacardDisplay::Enable(CLevelPlacardDisplay *this)

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


/* CLevelPlacardDisplay::InputEnable at 006bd790 */

/* DWARF original prototype: void InputEnable(CLevelPlacardDisplay * this, inputdata_t * inputdata)
    */

void __thiscall CLevelPlacardDisplay::InputEnable(CLevelPlacardDisplay *this,inputdata_t *inputdata)

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


/* CLevelPlacardDisplay::InputSetDisplayText at 006bce90 */

/* DWARF original prototype: void InputSetDisplayText(CLevelPlacardDisplay * this, inputdata_t *
   inputdata) */

void __thiscall
CLevelPlacardDisplay::InputSetDisplayText(CLevelPlacardDisplay *this,inputdata_t *inputdata)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  char *src;
  IChangeInfoAccessor *pIVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((inputdata->value).fieldType == FIELD_STRING) {
    src = (char *)(inputdata->value).field_0.iVal;
    if (src == (char *)0x0) {
      src = (char *)(unaff_EBX + 0x2eecb8);
    }
  }
  else {
    src = variant_t::ToString(&inputdata->value);
  }
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
  _V_strcpy((this->m_szDisplayText).m_Value,src);
  return;
}


/* CLevelPlacardDisplay::GetControlPanelInfo at 006bcd80 */

/* DWARF original prototype: void GetControlPanelInfo(CLevelPlacardDisplay * this, int nPanelIndex,
   char * * pPanelName) */

void __thiscall
CLevelPlacardDisplay::GetControlPanelInfo
          (CLevelPlacardDisplay *this,int nPanelIndex,char **pPanelName)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *pPanelName = (char *)(extraout_ECX + 0x352542);
  return;
}


/* CLevelPlacardDisplay::GetControlPanelClassName at 006bcda0 */

/* DWARF original prototype: void GetControlPanelClassName(CLevelPlacardDisplay * this, int
   nPanelIndex, char * * pPanelName) */

void __thiscall
CLevelPlacardDisplay::GetControlPanelClassName
          (CLevelPlacardDisplay *this,int nPanelIndex,char **pPanelName)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *pPanelName = (char *)(extraout_ECX + 0x350af8);
  return;
}


/* CLevelPlacardDisplay::SpawnControlPanels at 006bcdc0 */

/* DWARF original prototype: void SpawnControlPanels(CLevelPlacardDisplay * this) */

void __thiscall CLevelPlacardDisplay::SpawnControlPanels(CLevelPlacardDisplay *this)

{
  _func_int_varargs *p_Var1;
  CVGuiScreen *this_00;
  int iVar2;
  ulong *puVar3;
  int unaff_EBX;
  
                    /* Unresolved local var: int nPanel@[???] */
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: char * pScreenName@[???]
                       Unresolved local var: char * pScreenClassname@[???]
                       Unresolved local var: float flWidth@[???]
                       Unresolved local var: float flHeight@[???]
                       Unresolved local var: CVGuiScreen * pScreen@[???] */
  this_00 = CreateVGuiScreen((char *)(unaff_EBX + 0x350acc),(char *)(unaff_EBX + 0x3524f6),
                             &this->super_CBaseEntity,&this->super_CBaseEntity,0);
  p_Var1 = (this_00->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity
           ._vptr_IHandleEntity[0x5e];
  iVar2 = CBaseEntity::GetTeamNumber(&this->super_CBaseEntity);
  (*p_Var1)(this_00,iVar2);
  CVGuiScreen::SetActualSize(this_00,94.0,190.0);
  CVGuiScreen::SetActive(this_00,true);
  CVGuiScreen::MakeVisibleOnlyToTeammates(this_00,false);
  CVGuiScreen::SetTransparency(this_00,true);
  puVar3 = (ulong *)(*(this_00->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.
                      super_IHandleEntity._vptr_IHandleEntity[3])(this_00);
  (this->m_hScreen).super_CBaseHandle.m_Index = *puVar3;
  return;
}


/* CLevelPlacardDisplay::RestoreControlPanels at 006bd360 */

/* DWARF original prototype: void RestoreControlPanels(CLevelPlacardDisplay * this) */

void __thiscall CLevelPlacardDisplay::RestoreControlPanels(CLevelPlacardDisplay *this)

{
  CGlobalEntityList *this_00;
  uint uVar1;
  CVGuiScreen *pCVar2;
  int iVar3;
  CLevelPlacardDisplay *pCVar4;
  char *s1;
  uint *puVar5;
  int unaff_EBX;
  
                    /* Unresolved local var: int nPanel@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = *(CGlobalEntityList **)(unaff_EBX + 0x4ee8f2);
  pCVar2 = (CVGuiScreen *)
           CGlobalEntityList::FindEntityByClassname
                     (this_00,(CBaseEntity *)0x0,(char *)(unaff_EBX + 0x350532));
  if (pCVar2 == (CVGuiScreen *)0x0) goto LAB_006bd3e7;
  do {
    uVar1 = (pCVar2->super_CBaseEntity).m_hOwnerEntity.
            super_CNetworkVarBase<CBaseHandle,CBaseEntity::NetworkVar_m_hOwnerEntity>.m_Value.
            m_Index;
    if ((uVar1 == 0xffffffff) ||
       (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4ee836),
       *(uint *)(iVar3 + 8) != uVar1 >> 0x10)) {
      pCVar4 = (CLevelPlacardDisplay *)0x0;
    }
    else {
      pCVar4 = *(CLevelPlacardDisplay **)(iVar3 + 4);
    }
    if (pCVar4 == this) goto LAB_006bd3e7;
    while( true ) {
      pCVar2 = (CVGuiScreen *)
               CGlobalEntityList::FindEntityByClassname
                         (this_00,&pCVar2->super_CBaseEntity,(char *)(unaff_EBX + 0x350532));
      if (pCVar2 != (CVGuiScreen *)0x0) break;
LAB_006bd3e7:
      s1 = CVGuiScreen::GetPanelName(pCVar2);
      iVar3 = _V_strcmp(s1,(char *)(unaff_EBX + 0x351f5c));
      if (iVar3 == 0) {
        if (pCVar2 != (CVGuiScreen *)0x0) {
          puVar5 = (uint *)(*(pCVar2->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.
                             super_IHandleEntity._vptr_IHandleEntity[3])(pCVar2);
          uVar1 = *puVar5;
          (this->m_hScreen).super_CBaseHandle.m_Index = uVar1;
          if ((uVar1 == 0xffffffff) ||
             (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4ee836),
             *(uint *)(iVar3 + 8) != uVar1 >> 0x10)) {
            pCVar2 = (CVGuiScreen *)0x0;
          }
          else {
            pCVar2 = *(CVGuiScreen **)(iVar3 + 4);
          }
          CVGuiScreen::SetActive(pCVar2,true);
        }
        return;
      }
    }
  } while( true );
}


/* _GLOBAL__I__ZN20CLevelPlacardDisplay9m_DataMapE at 000c0ee0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN20CLevelPlacardDisplay9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

