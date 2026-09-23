/* DWARF-guided pseudocode for game/server/portal2/level_placard_display.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* CEntityFactory<CLevelPlacardDisplay>::Create at 0071eb00 */

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
       (_func_int_varargs **)(unaff_EBX + 0x5abf34);
  this_00[1].m_flCreateTime.m_Value = -NAN;
  (**(code **)(unaff_EBX + 0x5abfa8))(this_00,pClassName);
  return &(this_00->m_Network).super_IServerNetworkable;
}


/* DataMapInit<CLevelPlacardDisplay> at 000ab9d0 */

datamap_t * DataMapInit<CLevelPlacardDisplay>(CLevelPlacardDisplay *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[799].m_pPrev + unaff_EBX + 3) ==
      '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xda0ddf);
    if (iVar1 != 0) {
      *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[800].m_pEntity + unaff_EBX + 3) =
           unaff_EBX + 0xa05786;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[800].m_pPrev + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[800].m_pNext + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x321].m_pEntity + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x321].m_SerialNumber + unaff_EBX + 3) =
           0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x321].m_pPrev + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[800].m_SerialNumber + unaff_EBX + 3) =
           0x14;
      ___cxa_guard_release(unaff_EBX + 0xda0ddf);
      ___cxa_atexit(unaff_EBX + 0x9918cf,0,*(undefined4 *)(&DAT_00b9b173 + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_CanStandOn_00cb8b48 + unaff_EBX + 3) =
       *(undefined4 *)(&DAT_00b9b2ab + unaff_EBX);
  *(undefined4 *)((int)&PTR_ChangeTeam_00cb8b40 + unaff_EBX + 3) = 7;
  *(int *)((int)&PTR_IsViewable_00cb8b3c + unaff_EBX + 3) = unaff_EBX + 0xcb8b9f;
  return (datamap_t *)((int)&PTR_IsViewable_00cb8b3c + unaff_EBX + 3);
}


/* __static_initialization_and_destruction_0 at 000abaa0 */

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
    *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2fc].m_pPrev + unaff_EBX) =
         0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2fc].m_pPrev + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2fc].m_pPrev + unaff_EBX + 2) = 0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2fc].m_pPrev + unaff_EBX + 3) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2fc].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2fd].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2fd].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2fd].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2fd].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2fe].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2fe].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2fe].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2fe].m_pNext + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2ff].m_pEntity + unaff_EBX)
         = 0x7f7fffff;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2ff].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2ff].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2ff].m_pNext + unaff_EBX) =
         &UNK_00ba0854 + unaff_EBX;
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x300].m_pEntity + unaff_EBX)
         = &UNK_00c1f2dc + unaff_EBX;
    pIVar4 = EntityFactoryDictionary();
    (**pIVar4->_vptr_IEntityFactoryDictionary)(pIVar4,unaff_EBX + 0xda0be4,unaff_EBX + 0xa056c8);
    pdVar5 = DataMapInit<CLevelPlacardDisplay>((CLevelPlacardDisplay *)0x0);
    *(datamap_t **)((int)DataMapInit<CAI_Navigator>::dataDesc[6].flatOffset + unaff_EBX + -0x2c) =
         pdVar5;
    iVar7 = unaff_EBX + 0xda0be8;
    s2 = (char *)(unaff_EBX + 0xa056b3);
    *(char **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x300].m_SerialNumber + unaff_EBX)
         = s2;
    this = (SendTable *)
           ((int)DataMapInit<CAI_Navigator>::dataDesc[6].flatOffset + unaff_EBX + -0x28);
    *(SendTable **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x300].m_pPrev + unaff_EBX) =
         this;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x301].m_SerialNumber + unaff_EBX) = 0xffff
    ;
    piVar1 = *(int **)(&DAT_00b9b140 + unaff_EBX);
    puVar2 = (undefined4 *)*piVar1;
    if (puVar2 == (undefined4 *)0x0) {
      *piVar1 = iVar7;
      *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x300].m_pNext + unaff_EBX)
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
        *(undefined4 **)
         ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x300].m_pNext + unaff_EBX) = puVar3;
        puVar2[2] = iVar7;
      }
      else {
        *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x300].m_pNext + unaff_EBX) =
             *piVar1;
        *piVar1 = iVar7;
      }
    }
    SendTable::SendTable(this);
    ___cxa_atexit(unaff_EBX + 0x9917dc,0,*(undefined4 *)(&DAT_00b9b0a0 + unaff_EBX));
    iVar7 = ServerClassInit<DT_LevelPlacardDisplay::ignored>((ignored *)0x0);
    *(int *)((int)DataMapInit<CAI_Navigator>::dataDesc[6].flatOffset + unaff_EBX + -0x14) = iVar7;
  }
  return;
}


/* CLevelPlacardDisplay::GetDataDescMap at 0071d770 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CLevelPlacardDisplay * this) */

datamap_t * __thiscall CLevelPlacardDisplay::GetDataDescMap(CLevelPlacardDisplay *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x646da8);
}


/* CLevelPlacardDisplay::GetBaseMap at 0071d780 */

datamap_t * CLevelPlacardDisplay::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x529504);
}


/* __tcf_0 at 00a3d2b0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x40f51e)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x40f512) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x40f51e));
  }
  *(undefined4 *)(unaff_EBX + 0x40f51e) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40f51a)) {
    if (*(int *)(unaff_EBX + 0x40f512) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20989e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20989e),*(int *)(unaff_EBX + 0x40f512));
      *(undefined4 *)(unaff_EBX + 0x40f512) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f516) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40f512);
  *(int *)(CC_Find_Ent + unaff_EBX + 2) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40f51a)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20989e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20989e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40f512) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f516) = 0;
  }
  return;
}


/* ServerClassInit<DT_LevelPlacardDisplay::ignored> at 000ab810 */

int ServerClassInit<DT_LevelPlacardDisplay::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&UNK_000032ac)[(int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xda0e8c);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x32b].m_pPrev + unaff_EBX),
                  (char *)(unaff_EBX + 0x9aa410),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x330].m_pNext +
                        unaff_EBX),(char *)(unaff_EBX + 0x9ac7c0),0,
                        (SendTable *)**(undefined4 **)(&DAT_00b9b428 + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00b9b3cc + unaff_EBX),0x80);
      SendPropBool((SendProp_conflict *)
                   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x336].m_pEntity + unaff_EBX)
                   ,(char *)(unaff_EBX + 0xa03ce4),0x3c0,1);
      ___cxa_guard_release(unaff_EBX + 0xda0e8c);
      ___cxa_atexit(unaff_EBX + 0x991a1c,0,*(undefined4 *)(&DAT_00b9b330 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)
             ((int)DataMapInit<CAI_Navigator>::dataDesc[0x10].flatOffset + unaff_EBX + -0x18),
             (SendProp_conflict *)
             ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x330].m_pNext + unaff_EBX),2,
             *(char **)(&DAT_00c1f544 + unaff_EBX));
  return 1;
}


/* CLevelPlacardDisplay::GetServerClass at 0071d790 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: ServerClass * GetServerClass(CLevelPlacardDisplay * this) */

ServerClass * __thiscall CLevelPlacardDisplay::GetServerClass(CLevelPlacardDisplay *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(CNPC_PersonalityCore::GetSchedulingErrorName + extraout_ECX + 4);
}


/* CLevelPlacardDisplay::YouForgotToImplementOrDeclareServerClass at 0071d7a0 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CLevelPlacardDisplay *
   this) */

int __thiscall
CLevelPlacardDisplay::YouForgotToImplementOrDeclareServerClass(CLevelPlacardDisplay *this)

{
  return 0;
}


/* __tcf_2 at 00a3d290 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(&UNK_0038b488 + extraout_ECX),in_stack_00000008);
  return;
}


/* __tcf_1 at 00a3d240 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f517))(unaff_EBX + 0x40f517);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f4c3))(unaff_EBX + 0x40f4c3);
                    /* WARNING: Could not recover jumptable at 0x00a3d287. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f46f))();
  return;
}


/* CLevelPlacardDisplay::~CLevelPlacardDisplay at 0071daa0 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5acf94);
  uVar1 = (this->m_hScreen).super_CBaseHandle.m_Index;
  if ((uVar1 == 0xffffffff) ||
     (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5290b8),
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


/* CLevelPlacardDisplay::~CLevelPlacardDisplay at 0071dc70 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5acdc4);
  uVar1 = (this->m_hScreen).super_CBaseHandle.m_Index;
  if ((uVar1 == 0xffffffff) ||
     (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x528ee8),
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


/* CLevelPlacardDisplay::~CLevelPlacardDisplay at 0071dd00 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5acd34);
  uVar1 = (this->m_hScreen).super_CBaseHandle.m_Index;
  if ((uVar1 == 0xffffffff) ||
     (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x528e58),
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


/* CLevelPlacardDisplay::KeyValue at 0071db30 */

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
  if (szKeyName != (char *)(unaff_EBX + 0x3375b5)) {
    iVar2 = _V_stricmp(szKeyName,(char *)(unaff_EBX + 0x3375b5));
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


/* CLevelPlacardDisplay::UpdateTransmitState at 0071da60 */

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


/* CLevelPlacardDisplay::SetTransmit at 0071d970 */

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
    uVar3 = (int)peVar1 - *(int *)(**(int **)(unaff_EBX + 0x5291e0) + 0x58) >> 4;
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
     (iVar2 = (uVar3 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5291e8),
     *(uint *)(iVar2 + 8) != uVar3 >> 0x10)) {
    piVar4 = (int *)0x0;
  }
  else {
    piVar4 = *(int **)(iVar2 + 4);
  }
                    /* WARNING: Could not recover jumptable at 0x0071da39. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*piVar4 + 0x5c))();
  return;
}


/* CLevelPlacardDisplay::Spawn at 0071e180 */

/* DWARF original prototype: void Spawn(CLevelPlacardDisplay * this) */

void __thiscall CLevelPlacardDisplay::Spawn(CLevelPlacardDisplay *this)

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
  (*(this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x1a])(this);
  CBaseEntity::Spawn(&this->super_CBaseEntity);
  if ((this->m_bEnabled).m_Value == false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x5289d4);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_0071e324:
            puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x3c0;
            puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != 0x3c0) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_0071e324;
                pIVar6->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar7 * 0x14 + local_60 + 1] != 0x3c0);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x5289d4) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x5289d4) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x5289d4);
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar2 = *piVar5 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3c0;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
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


/* CLevelPlacardDisplay::Precache at 0071d950 */

/* DWARF original prototype: void Precache(CLevelPlacardDisplay * this) */

void __thiscall CLevelPlacardDisplay::Precache(CLevelPlacardDisplay *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  PrecacheVGuiScreen((char *)(extraout_ECX + 0x3937fa));
  return;
}


/* CLevelPlacardDisplay::OnRestore at 0071e140 */

/* DWARF original prototype: void OnRestore(CLevelPlacardDisplay * this) */

void __thiscall CLevelPlacardDisplay::OnRestore(CLevelPlacardDisplay *this)

{
  CBaseEntity::OnRestore(&this->super_CBaseEntity);
  RestoreControlPanels(this);
  ScreenVisible(this,(this->m_bEnabled).m_Value);
  return;
}


/* CLevelPlacardDisplay::ScreenVisible at 0071dea0 */

/* DWARF original prototype: void ScreenVisible(CLevelPlacardDisplay * this, bool bVisible) */

void __thiscall CLevelPlacardDisplay::ScreenVisible(CLevelPlacardDisplay *this,bool bVisible)

{
  undefined1 *puVar1;
  ushort uVar2;
  int *piVar3;
  CBaseEdict *this_00;
  edict_t *peVar4;
  ushort *puVar5;
  int iVar6;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  uint uVar9;
  CVGuiScreen *this_01;
  uint uVar10;
  int unaff_EBX;
  CBaseEntity *pCVar11;
  uint local_60;
  
  ___i686_get_pc_thunk_bx();
  piVar3 = *(int **)(unaff_EBX + 0x528cb8);
  uVar9 = (this->m_hScreen).super_CBaseHandle.m_Index;
  if ((uVar9 == 0xffffffff) ||
     (iVar6 = (uVar9 & 0xffff) * 0x10 + *piVar3, *(uint *)(iVar6 + 8) != uVar9 >> 0x10)) {
    this_01 = (CVGuiScreen *)0x0;
  }
  else {
    this_01 = *(CVGuiScreen **)(iVar6 + 4);
  }
  CVGuiScreen::SetActive(this_01,bVisible);
  if (!bVisible) {
    uVar9 = (this->m_hScreen).super_CBaseHandle.m_Index;
    if ((uVar9 == 0xffffffff) ||
       (iVar6 = (uVar9 & 0xffff) * 0x10 + *piVar3, *(uint *)(iVar6 + 8) != uVar9 >> 0x10)) {
      pCVar11 = (CBaseEntity *)0x0;
    }
    else {
      pCVar11 = *(CBaseEntity **)(iVar6 + 4);
    }
    CBaseEntity::AddEffects(pCVar11,0x20);
    return;
  }
  uVar9 = (this->m_hScreen).super_CBaseHandle.m_Index;
  if ((uVar9 == 0xffffffff) ||
     (iVar6 = (uVar9 & 0xffff) * 0x10 + *piVar3, *(uint *)(iVar6 + 8) != uVar9 >> 0x10)) {
    pCVar11 = (CBaseEntity *)0x0;
  }
  else {
    pCVar11 = *(CBaseEntity **)(iVar6 + 4);
  }
  uVar9 = (pCVar11->m_fEffects).m_Value;
  uVar10 = uVar9 & 0xffffffdf;
  if (uVar9 != uVar10) {
    if ((pCVar11->m_Network).m_TimerEvent.m_bRegistered == false) {
      this_00 = &((pCVar11->m_Network).m_pPev)->super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(this_00);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x528cb4);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar9 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar5[uVar9 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_0071e104:
            puVar5[(uint)uVar2 + uVar9 * 0x14 + 1] = 0xc0;
            puVar5[uVar9 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar5[uVar9 * 0x14 + 1] != 0xc0) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar2) {
                if (uVar2 == 0x13) goto LAB_0071e129;
                goto LAB_0071e104;
              }
            } while (puVar5[uVar9 * 0x14 + local_60 + 1] != 0xc0);
          }
        }
        else if (puVar5[0x7d1] == 100) {
LAB_0071e129:
          pIVar7->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x528cb4) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x528cb4) + 0xfa2) + 1;
          piVar3 = *(int **)(unaff_EBX + 0x528cb4);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar3;
          iVar6 = *piVar3 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar6 + 2) = 0xc0;
          *(undefined2 *)(iVar6 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(pCVar11->m_Network).field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (pCVar11->m_fEffects).m_Value = uVar10;
  }
  peVar4 = (pCVar11->m_Network).m_pPev;
  if (peVar4 != (edict_t *)0x0) {
    (peVar4->super_CBaseEdict).m_fStateFlags = (peVar4->super_CBaseEdict).m_fStateFlags | 0x80;
  }
  CBaseEntity::DispatchUpdateTransmitState(pCVar11);
  return;
}


/* CLevelPlacardDisplay::Disable at 0071e380 */

/* DWARF original prototype: void Disable(CLevelPlacardDisplay * this) */

void __thiscall CLevelPlacardDisplay::Disable(CLevelPlacardDisplay *this)

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
  if ((this->m_bEnabled).m_Value != false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x5287d4);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_0071e504:
            puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x3c0;
            puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != 0x3c0) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_0071e504;
                pIVar6->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar7 * 0x14 + local_60 + 1] != 0x3c0);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x5287d4) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x5287d4) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x5287d4);
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar2 = *piVar5 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3c0;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
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


/* CLevelPlacardDisplay::InputDisable at 0071e540 */

/* DWARF original prototype: void InputDisable(CLevelPlacardDisplay * this, inputdata_t * inputdata)
    */

void __thiscall
CLevelPlacardDisplay::InputDisable(CLevelPlacardDisplay *this,inputdata_t *inputdata)

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
  if ((this->m_bEnabled).m_Value == false) {
    return;
  }
  if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
    this_00 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
    if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
      this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
      pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
      puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x528614);
      if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
        uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar3 = puVar4[uVar7 * 0x14 + 0x14];
        if (uVar3 == 0) {
LAB_0071e6c4:
          puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x3c0;
          puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
        }
        else if (puVar4[uVar7 * 0x14 + 1] != 0x3c0) {
          local_60 = 0;
          do {
            uVar8 = local_60 + 1;
            local_60 = uVar8 & 0xffff;
            if ((ushort)uVar8 == uVar3) {
              if (uVar3 != 0x13) goto LAB_0071e6c4;
              pIVar6->m_iChangeInfoSerialNumber = 0;
              this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
              break;
            }
          } while (puVar4[uVar7 * 0x14 + local_60 + 1] != 0x3c0);
        }
      }
      else if (puVar4[0x7d1] == 100) {
        pIVar6->m_iChangeInfoSerialNumber = 0;
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar6->m_iChangeInfo = puVar4[0x7d1];
        *(short *)(**(int **)(unaff_EBX + 0x528614) + 0xfa2) =
             *(short *)(**(int **)(unaff_EBX + 0x528614) + 0xfa2) + 1;
        piVar5 = *(int **)(unaff_EBX + 0x528614);
        pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
        iVar2 = *piVar5 + (uint)pIVar6->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar2 + 2) = 0x3c0;
        *(undefined2 *)(iVar2 + 0x28) = 1;
      }
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


/* CLevelPlacardDisplay::Enable at 0071e720 */

/* DWARF original prototype: void Enable(CLevelPlacardDisplay * this) */

void __thiscall CLevelPlacardDisplay::Enable(CLevelPlacardDisplay *this)

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
  if ((this->m_bEnabled).m_Value == false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x528434);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_0071e8a4:
            puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x3c0;
            puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != 0x3c0) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_0071e8a4;
                pIVar6->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar7 * 0x14 + local_60 + 1] != 0x3c0);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x528434) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x528434) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x528434);
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar2 = *piVar5 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3c0;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
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


/* CLevelPlacardDisplay::InputEnable at 0071e8e0 */

/* DWARF original prototype: void InputEnable(CLevelPlacardDisplay * this, inputdata_t * inputdata)
    */

void __thiscall CLevelPlacardDisplay::InputEnable(CLevelPlacardDisplay *this,inputdata_t *inputdata)

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
  if ((this->m_bEnabled).m_Value != false) {
    return;
  }
  if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
    this_00 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
    if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
      this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
      pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
      puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x528274);
      if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
        uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar3 = puVar4[uVar7 * 0x14 + 0x14];
        if (uVar3 == 0) {
LAB_0071ea64:
          puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x3c0;
          puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
        }
        else if (puVar4[uVar7 * 0x14 + 1] != 0x3c0) {
          local_60 = 0;
          do {
            uVar8 = local_60 + 1;
            local_60 = uVar8 & 0xffff;
            if ((ushort)uVar8 == uVar3) {
              if (uVar3 != 0x13) goto LAB_0071ea64;
              pIVar6->m_iChangeInfoSerialNumber = 0;
              this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
              break;
            }
          } while (puVar4[uVar7 * 0x14 + local_60 + 1] != 0x3c0);
        }
      }
      else if (puVar4[0x7d1] == 100) {
        pIVar6->m_iChangeInfoSerialNumber = 0;
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar6->m_iChangeInfo = puVar4[0x7d1];
        *(short *)(**(int **)(unaff_EBX + 0x528274) + 0xfa2) =
             *(short *)(**(int **)(unaff_EBX + 0x528274) + 0xfa2) + 1;
        piVar5 = *(int **)(unaff_EBX + 0x528274);
        pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
        iVar2 = *piVar5 + (uint)pIVar6->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar2 + 2) = 0x3c0;
        *(undefined2 *)(iVar2 + 0x28) = 1;
      }
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


/* CLevelPlacardDisplay::InputSetDisplayText at 0071d8c0 */

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
      src = (char *)(unaff_EBX + 0x331450);
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


/* CLevelPlacardDisplay::GetControlPanelInfo at 0071d7b0 */

/* DWARF original prototype: void GetControlPanelInfo(CLevelPlacardDisplay * this, int nPanelIndex,
   char * * pPanelName) */

void __thiscall
CLevelPlacardDisplay::GetControlPanelInfo
          (CLevelPlacardDisplay *this,int nPanelIndex,char **pPanelName)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *pPanelName = (char *)(extraout_ECX + 0x39399a);
  return;
}


/* CLevelPlacardDisplay::GetControlPanelClassName at 0071d7d0 */

/* DWARF original prototype: void GetControlPanelClassName(CLevelPlacardDisplay * this, int
   nPanelIndex, char * * pPanelName) */

void __thiscall
CLevelPlacardDisplay::GetControlPanelClassName
          (CLevelPlacardDisplay *this,int nPanelIndex,char **pPanelName)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *pPanelName = (char *)(extraout_ECX + 0x391d24);
  return;
}


/* CLevelPlacardDisplay::SpawnControlPanels at 0071d7f0 */

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
  this_00 = CreateVGuiScreen((char *)(unaff_EBX + 0x391cf8),(char *)(unaff_EBX + 0x39394e),
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


/* CLevelPlacardDisplay::RestoreControlPanels at 0071dd90 */

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
  this_00 = *(CGlobalEntityList **)(unaff_EBX + 0x528e86);
  pCVar2 = (CVGuiScreen *)
           CGlobalEntityList::FindEntityByClassname
                     (this_00,(CBaseEntity *)0x0,(char *)(unaff_EBX + 0x39175e));
  if (pCVar2 == (CVGuiScreen *)0x0) goto LAB_0071de17;
  do {
    uVar1 = (pCVar2->super_CBaseEntity).m_hOwnerEntity.
            super_CNetworkVarBase<CBaseHandle,CBaseEntity::NetworkVar_m_hOwnerEntity>.m_Value.
            m_Index;
    if ((uVar1 == 0xffffffff) ||
       (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(&DAT_00528dce + unaff_EBX),
       *(uint *)(iVar3 + 8) != uVar1 >> 0x10)) {
      pCVar4 = (CLevelPlacardDisplay *)0x0;
    }
    else {
      pCVar4 = *(CLevelPlacardDisplay **)(iVar3 + 4);
    }
    if (pCVar4 == this) goto LAB_0071de17;
    while( true ) {
      pCVar2 = (CVGuiScreen *)
               CGlobalEntityList::FindEntityByClassname
                         (this_00,&pCVar2->super_CBaseEntity,(char *)(unaff_EBX + 0x39175e));
      if (pCVar2 != (CVGuiScreen *)0x0) break;
LAB_0071de17:
      s1 = CVGuiScreen::GetPanelName(pCVar2);
      iVar3 = _V_strcmp(s1,(char *)(unaff_EBX + 0x3933b4));
      if (iVar3 == 0) {
        if (pCVar2 != (CVGuiScreen *)0x0) {
          puVar5 = (uint *)(*(pCVar2->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.
                             super_IHandleEntity._vptr_IHandleEntity[3])(pCVar2);
          uVar1 = *puVar5;
          (this->m_hScreen).super_CBaseHandle.m_Index = uVar1;
          if ((uVar1 == 0xffffffff) ||
             (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(&DAT_00528dce + unaff_EBX),
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


/* _GLOBAL__I__ZN20CLevelPlacardDisplay9m_DataMapE at 000abc90 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN20CLevelPlacardDisplay9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

