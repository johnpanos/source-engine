/* DWARF-guided pseudocode for game/server/portal2/indicator_panel.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* DataMapInit<CLabIndicatorPanel> at 000a9950 */

datamap_t * DataMapInit<CLabIndicatorPanel>(CLabIndicatorPanel *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x496].m_pEntity + unaff_EBX + 3)
      == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xda2547);
    if (iVar1 != 0) {
      *(undefined **)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x496].m_pPrev + unaff_EBX + 3) =
           &UNK_00a06fab + unaff_EBX;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x497].m_pEntity + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x497].m_SerialNumber + unaff_EBX + 3) =
           0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x497].m_pPrev + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x497].m_pNext + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x498].m_pEntity + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x496].m_pNext + unaff_EBX + 3) = 0x12;
      ___cxa_guard_release(unaff_EBX + 0xda2547);
      ___cxa_atexit(unaff_EBX + 0x992d1f,0,*(undefined4 *)(&DAT_00b9d1f3 + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_IsLockedByMaster_00cb9988 + unaff_EBX + 3) =
       *(undefined4 *)(&DAT_00b9d32b + unaff_EBX);
  *(undefined4 *)((int)&PTR_DoImpactEffect_00cb9980 + unaff_EBX + 3) = 0xb;
  *(int *)((int)&PTR_FireBullets_00cb997c + unaff_EBX + 3) = unaff_EBX + 0xcb9f1f;
  return (datamap_t *)((int)&PTR_FireBullets_00cb997c + unaff_EBX + 3);
}


/* __static_initialization_and_destruction_0 at 000aa100 */

/* WARNING: Struct "CPropTicTacToePanel": ignoring overlapping field "m_bPlayed" */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *s2;
  SendTable *this;
  undefined4 *puVar1;
  datamap_t *pdVar2;
  int iVar3;
  int iVar4;
  IEntityFactoryDictionary *pIVar5;
  int unaff_EBX;
  undefined4 *puVar6;
  undefined4 *puVar7;
  longlong lVar8;
  
  lVar8 = ___i686_get_pc_thunk_bx();
  if (lVar8 == 0xffff00000001) {
    *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3da].m_pPrev + unaff_EBX) =
         0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3da].m_pPrev + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3da].m_pPrev + unaff_EBX + 2) = 0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3da].m_pPrev + unaff_EBX + 3) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3da].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3db].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3db].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3db].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3db].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3dc].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3dc].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3dc].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3dc].m_pNext + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3dd].m_pEntity + unaff_EBX)
         = 0x7f7fffff;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3dd].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3dd].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3dd].m_pNext + unaff_EBX) =
         &UNK_00ba21f4 + unaff_EBX;
    pdVar2 = DataMapInit<CLabIndicatorPanel>((CLabIndicatorPanel *)0x0);
    *(datamap_t **)((int)DataMapInit<CAI_Path>::dataDesc[0x12].flatOffset + unaff_EBX + -0x2c) =
         pdVar2;
    iVar4 = unaff_EBX + 0xda19c4;
    s2 = (char *)(unaff_EBX + 0xa067f8);
    *(char **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3de].m_pEntity + unaff_EBX) = s2
    ;
    this = (SendTable *)((int)DataMapInit<CAI_Path>::dataDesc[0x12].flatOffset + unaff_EBX + -0x28);
    *(SendTable **)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3de].m_SerialNumber + unaff_EBX) = this;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3df].m_pEntity + unaff_EBX)
         = 0xffff;
    puVar7 = (undefined4 *)**(int **)(&DAT_00b9cae0 + unaff_EBX);
    if (puVar7 == (undefined4 *)0x0) {
      **(int **)(&DAT_00b9cae0 + unaff_EBX) = iVar4;
      *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3de].m_pPrev + unaff_EBX)
           = 0;
    }
    else {
      puVar1 = (undefined4 *)puVar7[2];
      iVar3 = _V_stricmp((char *)*puVar7,s2);
      if (iVar3 < 1) {
        while ((puVar6 = puVar1, puVar6 != (undefined4 *)0x0 &&
               (iVar3 = _V_stricmp((char *)*puVar6,s2), iVar3 < 1))) {
          puVar1 = (undefined4 *)puVar6[2];
          puVar7 = puVar6;
        }
        *(undefined4 **)
         ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3de].m_pPrev + unaff_EBX) = puVar6;
        puVar7[2] = iVar4;
      }
      else {
        *(undefined4 *)
         ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3de].m_pPrev + unaff_EBX) =
             **(undefined4 **)(&DAT_00b9cae0 + unaff_EBX);
        **(int **)(&DAT_00b9cae0 + unaff_EBX) = iVar4;
      }
    }
    SendTable::SendTable(this);
    ___cxa_atexit(unaff_EBX + 0x99246c,0,*(undefined4 *)(&DAT_00b9ca40 + unaff_EBX));
    iVar4 = ServerClassInit<DT_LabIndicatorPanel::ignored>((ignored *)0x0);
    *(int *)((int)DataMapInit<CAI_Path>::dataDesc[0x12].flatOffset + unaff_EBX + -0x14) = iVar4;
    *(undefined **)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3df].m_SerialNumber + unaff_EBX) =
         &UNK_00c1f4a4 + unaff_EBX;
    pIVar5 = EntityFactoryDictionary();
    (**pIVar5->_vptr_IEntityFactoryDictionary)(pIVar5,unaff_EBX + 0xda19d8,unaff_EBX + 0xa0680b);
    pdVar2 = DataMapInit<CPropIndicatorPanel>((CPropIndicatorPanel *)0x0);
    *(datamap_t **)((int)DataMapInit<CAI_Path>::dataDesc[0x12].flatOffset + unaff_EBX + -0x10) =
         pdVar2;
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3df].m_pPrev + unaff_EBX) =
         &UNK_00c1f4c4 + unaff_EBX;
    pIVar5 = EntityFactoryDictionary();
    (**pIVar5->_vptr_IEntityFactoryDictionary)
              (pIVar5,unaff_EBX + 0xda19dc,&UNK_00a068aa + unaff_EBX);
    pdVar2 = DataMapInit<CPropTicTacToePanel>((CPropTicTacToePanel *)0x0);
    *(datamap_t **)((int)DataMapInit<CAI_Path>::dataDesc[0x12].flatOffset + unaff_EBX + -0xc) =
         pdVar2;
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3df].m_pNext + unaff_EBX) =
         &UNK_00c1f4e4 + unaff_EBX;
    pIVar5 = EntityFactoryDictionary();
    (**pIVar5->_vptr_IEntityFactoryDictionary)(pIVar5,unaff_EBX + 0xda19e0,unaff_EBX + 0xa068bf);
  }
  return;
}


/* CLabIndicatorPanel::GetDataDescMap at 00712d30 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CLabIndicatorPanel * this) */

datamap_t * __thiscall CLabIndicatorPanel::GetDataDescMap(CLabIndicatorPanel *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(&UNK_006505a8 + extraout_ECX);
}


/* CLabIndicatorPanel::GetBaseMap at 00712d40 */

datamap_t * CLabIndicatorPanel::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x533f44);
}


/* __tcf_0 at 00a3c680 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x40f836)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x40f82a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x40f836));
  }
  *(undefined4 *)(unaff_EBX + 0x40f836) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40f832)) {
    if (*(int *)(unaff_EBX + 0x40f82a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20a4ce) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20a4ce),*(int *)(unaff_EBX + 0x40f82a));
      *(undefined4 *)(unaff_EBX + 0x40f82a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f82e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40f82a);
  *(int *)(unaff_EBX + 0x40f83a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40f832)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20a4ce) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20a4ce),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40f82a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f82e) = 0;
  }
  return;
}


/* ServerClassInit<DT_LabIndicatorPanel::ignored> at 000a9a20 */

int ServerClassInit<DT_LabIndicatorPanel::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x454].m_SerialNumber +
               unaff_EBX + 2) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xda212a);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x454].m_pNext + unaff_EBX + 2
                  ),(char *)(unaff_EBX + 0x9ac206),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x45a].m_pEntity +
                        unaff_EBX + 2),(char *)(unaff_EBX + 0x9ae5b6),0,
                        (SendTable *)**(undefined4 **)(&DAT_00b9d21e + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00b9d1c2 + unaff_EBX),0x80);
      SendPropBool((SendProp_conflict *)
                   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x45f].m_SerialNumber +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0xa05ada),0x3c0,1);
      SendPropBool((SendProp_conflict *)
                   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x464].m_pPrev +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0x9bad4e),0x3c1,1);
      SendPropBool((SendProp_conflict *)
                   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x469].m_pNext +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0xa06f06),0x3c2,1);
      SendPropBool((SendProp_conflict *)
                   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x46f].m_pEntity +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0xa06f16),0x3c3,1);
      SendPropBool((SendProp_conflict *)
                   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x474].m_SerialNumber +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0xa06f2a),0x3c4,1);
      SendPropInt((SendProp_conflict *)
                  ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x479].m_pPrev + unaff_EBX + 2
                  ),(char *)(unaff_EBX + 0xa06f37),0x3c8,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x47e].m_pNext +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0xa06f47),0x3cc,4,0x20,0,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00b9d21a + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x484].m_pEntity +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0xa06f59),0x3d0,4,0x20,0,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00b9d21a + unaff_EBX),0x80);
      ___cxa_guard_release(unaff_EBX + 0xda212a);
      ___cxa_atexit(unaff_EBX + 0x992a92,0,*(undefined4 *)(&DAT_00b9d126 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)
             ((int)DataMapInit<CAI_ScriptConditions>::dataDesc[0xe].flatOffset + unaff_EBX + -2),
             (SendProp_conflict *)
             ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x45a].m_pEntity + unaff_EBX + 2),9
             ,*(char **)(&DAT_00c1fb6a + unaff_EBX));
  return 1;
}


/* CLabIndicatorPanel::GetServerClass at 00712d50 */

/* DWARF original prototype: ServerClass * GetServerClass(CLabIndicatorPanel * this) */

ServerClass * __thiscall CLabIndicatorPanel::GetServerClass(CLabIndicatorPanel *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x738d80);
}


/* CLabIndicatorPanel::YouForgotToImplementOrDeclareServerClass at 00712d60 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CLabIndicatorPanel * this)
    */

int __thiscall
CLabIndicatorPanel::YouForgotToImplementOrDeclareServerClass(CLabIndicatorPanel *this)

{
  return 0;
}


/* __tcf_4 at 00a3c580 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x38bf98),in_stack_00000008);
  return;
}


/* __tcf_1 at 00a3c4c0 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(&DAT_0040f987 + unaff_EBX))(&DAT_0040f987 + unaff_EBX);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f933))(unaff_EBX + 0x40f933);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f8df))(unaff_EBX + 0x40f8df);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f88b))(unaff_EBX + 0x40f88b);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f837))(unaff_EBX + 0x40f837);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f7e3))(unaff_EBX + 0x40f7e3);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f78f))(unaff_EBX + 0x40f78f);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f73b))(unaff_EBX + 0x40f73b);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f6e7))(unaff_EBX + 0x40f6e7);
                    /* WARNING: Could not recover jumptable at 0x00a3c577. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f693))();
  return;
}


/* CLabIndicatorPanel::~CLabIndicatorPanel at 00712e90 */

/* DWARF original prototype: void ~CLabIndicatorPanel(CLabIndicatorPanel * this, int __in_chrg) */

void __thiscall CLabIndicatorPanel::~CLabIndicatorPanel(CLabIndicatorPanel *this,int __in_chrg)

{
  uint uVar1;
  CVGuiScreen *pVGuiScreen;
  int iVar2;
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5b63a4);
  uVar1 = (this->m_hScreen).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x533cc8),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (pVGuiScreen = *(CVGuiScreen **)(iVar2 + 4), pVGuiScreen != (CVGuiScreen *)0x0)) {
    DestroyVGuiScreen(pVGuiScreen);
  }
  CBaseEntity::~CBaseEntity(&this->super_CBaseEntity,in_stack_ffffffd8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CLabIndicatorPanel::~CLabIndicatorPanel at 00712f30 */

/* DWARF original prototype: void ~CLabIndicatorPanel(CLabIndicatorPanel * this, int __in_chrg) */

void __thiscall CLabIndicatorPanel::~CLabIndicatorPanel(CLabIndicatorPanel *this,int __in_chrg)

{
  uint uVar1;
  CVGuiScreen *pVGuiScreen;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5b6304);
  uVar1 = (this->m_hScreen).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x533c28),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (pVGuiScreen = *(CVGuiScreen **)(iVar2 + 4), pVGuiScreen != (CVGuiScreen *)0x0)) {
    DestroyVGuiScreen(pVGuiScreen);
  }
  CBaseEntity::~CBaseEntity(&this->super_CBaseEntity,__in_chrg);
  return;
}


/* CLabIndicatorPanel::~CLabIndicatorPanel at 00712fc0 */

/* DWARF original prototype: void ~CLabIndicatorPanel(CLabIndicatorPanel * this, int __in_chrg,
   CLabIndicatorPanel * this) */

void __thiscall
CLabIndicatorPanel::~CLabIndicatorPanel
          (CLabIndicatorPanel *this,int __in_chrg,CLabIndicatorPanel *this_1)

{
  ~CLabIndicatorPanel(this,__in_chrg);
  return;
}


/* CLabIndicatorPanel::UpdateTransmitState at 00712e70 */

/* DWARF original prototype: int UpdateTransmitState(CLabIndicatorPanel * this) */

int __thiscall CLabIndicatorPanel::UpdateTransmitState(CLabIndicatorPanel *this)

{
  int iVar1;
  
  iVar1 = CBaseEntity::SetTransmitState(&this->super_CBaseEntity,0);
  return iVar1;
}


/* CLabIndicatorPanel::SetTransmit at 00713040 */

/* DWARF original prototype: void SetTransmit(CLabIndicatorPanel * this, CCheckTransmitInfo * pInfo,
   bool bAlways) */

void __thiscall
CLabIndicatorPanel::SetTransmit(CLabIndicatorPanel *this,CCheckTransmitInfo *pInfo,bool bAlways)

{
  edict_t *peVar1;
  int *piVar2;
  int iVar3;
  uint uVar4;
  int unaff_EBX;
  uint local_20;
  
  ___i686_get_pc_thunk_bx();
  peVar1 = (this->super_CBaseEntity).m_Network.m_pPev;
  if (peVar1 == (edict_t *)0x0) {
    local_20 = 0;
    uVar4 = 1;
  }
  else {
    uVar4 = (int)peVar1 - *(int *)(**(int **)(unaff_EBX + 0x533b10) + 0x58) >> 4;
    local_20 = uVar4 >> 5;
    uVar4 = 1 << ((byte)uVar4 & 0x1f);
  }
  if (((pInfo->m_pTransmitEdict->super_CBitVecT<CFixedBitVecBase<2048>_>).
       super_CFixedBitVecBase<2048>.m_Ints[local_20] & uVar4) == 0) {
    CBaseEntity::SetTransmit(&this->super_CBaseEntity,pInfo,bAlways);
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
    uVar4 = (this->m_hScreen).super_CBaseHandle.m_Index;
    if (((uVar4 != 0xffffffff) &&
        (iVar3 = (uVar4 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x533b18),
        *(uint *)(iVar3 + 8) == uVar4 >> 0x10)) &&
       (piVar2 = *(int **)(iVar3 + 4), piVar2 != (int *)0x0)) {
                    /* WARNING: Could not recover jumptable at 0x0071310e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (**(code **)(*piVar2 + 0x5c))();
      return;
    }
  }
  return;
}


/* CLabIndicatorPanel::Spawn at 007144e0 */

/* DWARF original prototype: void Spawn(CLabIndicatorPanel * this) */

void __thiscall CLabIndicatorPanel::Spawn(CLabIndicatorPanel *this)

{
  undefined1 *puVar1;
  int iVar2;
  ushort uVar3;
  CBaseEdict *pCVar4;
  ushort *puVar5;
  int *piVar6;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  uint uVar9;
  int unaff_EBX;
  uint local_9c;
  uint local_98;
  uint local_94;
  uint local_90;
  
  ___i686_get_pc_thunk_bx();
  (*(this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x1a])(this);
  CBaseEntity::Spawn(&this->super_CBaseEntity);
  SpawnControlPanels(this);
  if ((this->m_bEnabled).m_Value == false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar4 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x532677);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00714a61:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x3c0;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 0x3c0) {
            local_9c = 0;
            do {
              uVar9 = local_9c + 1;
              local_9c = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 != 0x13) goto LAB_00714a61;
                pIVar7->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar8 * 0x14 + local_9c + 1] != 0x3c0);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x532677) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x532677) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x532677);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
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
  ScreenVisible(this,(this->m_bEnabled).m_Value);
  if ((this->m_bCountingDown).m_Value != false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar4 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x532677);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_007149a1:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x3c2;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 0x3c2) {
            local_98 = 0;
            do {
              uVar9 = local_98 + 1;
              local_98 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 != 0x13) goto LAB_007149a1;
                pIVar7->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar8 * 0x14 + local_98 + 1] != 0x3c2);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x532677) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x532677) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x532677);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3c2;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bCountingDown).m_Value = false;
  }
  if ((this->m_bStopped).m_Value != false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar4 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x532677);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00714901:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x3c1;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 0x3c1) {
            local_94 = 0;
            do {
              uVar9 = local_94 + 1;
              local_94 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 != 0x13) goto LAB_00714901;
                pIVar7->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar8 * 0x14 + local_94 + 1] != 0x3c1);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x532677) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x532677) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x532677);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3c1;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bStopped).m_Value = false;
  }
  if ((this->m_flTimerStart).m_Value != 0.0) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar4 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x532677);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00714afd:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x3d0;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 0x3d0) {
            local_90 = 0;
            do {
              uVar9 = local_90 + 1;
              local_90 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 != 0x13) goto LAB_00714afd;
                pIVar7->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar8 * 0x14 + local_90 + 1] != 0x3d0);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x532677) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x532677) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x532677);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3d0;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flTimerStart).m_Value = 0.0;
  }
  return;
}


/* CLabIndicatorPanel::Precache at 00712e50 */

/* DWARF original prototype: void Precache(CLabIndicatorPanel * this) */

void __thiscall CLabIndicatorPanel::Precache(CLabIndicatorPanel *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  PrecacheVGuiScreen((char *)(extraout_ECX + 0x39daa4));
  return;
}


/* CLabIndicatorPanel::OnRestore at 007134a0 */

/* DWARF original prototype: void OnRestore(CLabIndicatorPanel * this) */

void __thiscall CLabIndicatorPanel::OnRestore(CLabIndicatorPanel *this)

{
  uint uVar1;
  CBaseEntity *oldObj;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::OnRestore(&this->super_CBaseEntity);
  uVar1 = (this->m_hScreen).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5336bf),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (oldObj = *(CBaseEntity **)(iVar2 + 4), oldObj != (CBaseEntity *)0x0)) {
    UTIL_RemoveImmediate(oldObj);
  }
  SpawnControlPanels(this);
  ScreenVisible(this,(this->m_bEnabled).m_Value);
  return;
}


/* CLabIndicatorPanel::ScreenVisible at 00713280 */

/* DWARF original prototype: void ScreenVisible(CLabIndicatorPanel * this, bool bVisible) */

void __thiscall CLabIndicatorPanel::ScreenVisible(CLabIndicatorPanel *this,bool bVisible)

{
  undefined1 *puVar1;
  ushort uVar2;
  CBaseEntity *this_00;
  CBaseEdict *this_01;
  edict_t *peVar3;
  ushort *puVar4;
  int *piVar5;
  int iVar6;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  uint uVar9;
  uint uVar10;
  int unaff_EBX;
  uint local_60;
  
  ___i686_get_pc_thunk_bx();
  uVar9 = (this->m_hScreen).super_CBaseHandle.m_Index;
  if (((uVar9 == 0xffffffff) ||
      (iVar6 = (uVar9 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5338d8),
      *(uint *)(iVar6 + 8) != uVar9 >> 0x10)) ||
     (this_00 = *(CBaseEntity **)(iVar6 + 4), this_00 == (CBaseEntity *)0x0)) {
    return;
  }
  if (!bVisible) {
    CBaseEntity::AddEffects(this_00,0x20);
    return;
  }
  uVar9 = (this_00->m_fEffects).m_Value;
  uVar10 = uVar9 & 0xffffffdf;
  if (uVar9 != uVar10) {
    if ((this_00->m_Network).m_TimerEvent.m_bRegistered == false) {
      this_01 = &((this_00->m_Network).m_pPev)->super_CBaseEdict;
      if ((this_01 != (CBaseEdict *)0x0) && ((this_01->m_fStateFlags & 0x100U) == 0)) {
        this_01->m_fStateFlags = this_01->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(this_01);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x5338d4);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar4) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar9 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar4[uVar9 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_00713459:
            puVar4[(uint)uVar2 + uVar9 * 0x14 + 1] = 0xc0;
            puVar4[uVar9 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar4[uVar9 * 0x14 + 1] != 0xc0) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar2) {
                if (uVar2 != 0x13) goto LAB_00713459;
                goto LAB_0071347e;
              }
            } while (puVar4[uVar9 * 0x14 + local_60 + 1] != 0xc0);
          }
        }
        else if (puVar4[0x7d1] == 100) {
LAB_0071347e:
          pIVar7->m_iChangeInfoSerialNumber = 0;
          this_01->m_fStateFlags = this_01->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x5338d4) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x5338d4) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x5338d4);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar6 = *piVar5 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar6 + 2) = 0xc0;
          *(undefined2 *)(iVar6 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this_00->m_Network).field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this_00->m_fEffects).m_Value = uVar10;
  }
  peVar3 = (this_00->m_Network).m_pPev;
  if (peVar3 != (edict_t *)0x0) {
    (peVar3->super_CBaseEdict).m_fStateFlags = (peVar3->super_CBaseEdict).m_fStateFlags | 0x80;
  }
  CBaseEntity::DispatchUpdateTransmitState(this_00);
  return;
}


/* CLabIndicatorPanel::Disable at 00714140 */

/* DWARF original prototype: void Disable(CLabIndicatorPanel * this) */

void __thiscall CLabIndicatorPanel::Disable(CLabIndicatorPanel *this)

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
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x532a14);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_007142c4:
            puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x3c0;
            puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != 0x3c0) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_007142c4;
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
          *(short *)(**(int **)(unaff_EBX + 0x532a14) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x532a14) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x532a14);
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


/* CLabIndicatorPanel::InputDisable at 00714300 */

/* DWARF original prototype: void InputDisable(CLabIndicatorPanel * this, inputdata_t * inputdata)
    */

void __thiscall CLabIndicatorPanel::InputDisable(CLabIndicatorPanel *this,inputdata_t *inputdata)

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
      puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x532854);
      if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
        uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar3 = puVar4[uVar7 * 0x14 + 0x14];
        if (uVar3 == 0) {
LAB_00714484:
          puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x3c0;
          puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
        }
        else if (puVar4[uVar7 * 0x14 + 1] != 0x3c0) {
          local_60 = 0;
          do {
            uVar8 = local_60 + 1;
            local_60 = uVar8 & 0xffff;
            if ((ushort)uVar8 == uVar3) {
              if (uVar3 != 0x13) goto LAB_00714484;
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
        *(short *)(**(int **)(unaff_EBX + 0x532854) + 0xfa2) =
             *(short *)(**(int **)(unaff_EBX + 0x532854) + 0xfa2) + 1;
        piVar5 = *(int **)(unaff_EBX + 0x532854);
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


/* CLabIndicatorPanel::Enable at 00715010 */

/* DWARF original prototype: void Enable(CLabIndicatorPanel * this) */

void __thiscall CLabIndicatorPanel::Enable(CLabIndicatorPanel *this)

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
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x531b44);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00715194:
            puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x3c0;
            puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != 0x3c0) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_00715194;
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
          *(short *)(**(int **)(unaff_EBX + 0x531b44) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x531b44) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x531b44);
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


/* CLabIndicatorPanel::InputEnable at 007166d0 */

/* DWARF original prototype: void InputEnable(CLabIndicatorPanel * this, inputdata_t * inputdata) */

void __thiscall CLabIndicatorPanel::InputEnable(CLabIndicatorPanel *this,inputdata_t *inputdata)

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
      puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x530484);
      if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
        uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar3 = puVar4[uVar7 * 0x14 + 0x14];
        if (uVar3 == 0) {
LAB_00716854:
          puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x3c0;
          puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
        }
        else if (puVar4[uVar7 * 0x14 + 1] != 0x3c0) {
          local_60 = 0;
          do {
            uVar8 = local_60 + 1;
            local_60 = uVar8 & 0xffff;
            if ((ushort)uVar8 == uVar3) {
              if (uVar3 != 0x13) goto LAB_00716854;
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
        *(short *)(**(int **)(unaff_EBX + 0x530484) + 0xfa2) =
             *(short *)(**(int **)(unaff_EBX + 0x530484) + 0xfa2) + 1;
        piVar5 = *(int **)(unaff_EBX + 0x530484);
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


/* CLabIndicatorPanel::GetControlPanelInfo at 00712d70 */

/* DWARF original prototype: void GetControlPanelInfo(CLabIndicatorPanel * this, int nPanelIndex,
   char * * pPanelName) */

void __thiscall
CLabIndicatorPanel::GetControlPanelInfo(CLabIndicatorPanel *this,int nPanelIndex,char **pPanelName)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *pPanelName = (char *)(extraout_ECX + 0x39db84);
  return;
}


/* CLabIndicatorPanel::GetControlPanelClassName at 00712d90 */

/* DWARF original prototype: void GetControlPanelClassName(CLabIndicatorPanel * this, int
   nPanelIndex, char * * pPanelName) */

void __thiscall
CLabIndicatorPanel::GetControlPanelClassName
          (CLabIndicatorPanel *this,int nPanelIndex,char **pPanelName)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *pPanelName = (char *)(extraout_ECX + 0x39c764);
  return;
}


/* CLabIndicatorPanel::SpawnControlPanels at 00713140 */

/* DWARF original prototype: void SpawnControlPanels(CLabIndicatorPanel * this) */

void __thiscall CLabIndicatorPanel::SpawnControlPanels(CLabIndicatorPanel *this)

{
  _func_int_varargs *p_Var1;
  CVGuiScreen *this_00;
  int iVar2;
  ulong *puVar3;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pScreenName@[???]
                       Unresolved local var: char * pScreenClassname@[???]
                       Unresolved local var: CVGuiScreen * pScreen@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CreateVGuiScreen((char *)(unaff_EBX + 0x39c3a8),(char *)(unaff_EBX + 0x39d7a8),
                             &this->super_CBaseEntity,&this->super_CBaseEntity,-1);
  p_Var1 = (this_00->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity
           ._vptr_IHandleEntity[0x5e];
  iVar2 = CBaseEntity::GetTeamNumber(&this->super_CBaseEntity);
  (*p_Var1)(this_00,iVar2);
  CVGuiScreen::SetActualSize(this_00,32.0,32.0);
  CVGuiScreen::SetActive(this_00,true);
  CVGuiScreen::MakeVisibleOnlyToTeammates(this_00,false);
  CVGuiScreen::SetTransparency(this_00,true);
  puVar3 = (ulong *)(*(this_00->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.
                      super_IHandleEntity._vptr_IHandleEntity[3])(this_00);
  (this->m_hScreen).super_CBaseHandle.m_Index = *puVar3;
  return;
}


/* CLabIndicatorPanel::RestoreControlPanels at 00713210 */

/* DWARF original prototype: void RestoreControlPanels(CLabIndicatorPanel * this) */

void __thiscall CLabIndicatorPanel::RestoreControlPanels(CLabIndicatorPanel *this)

{
  uint uVar1;
  CBaseEntity *oldObj;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->m_hScreen).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x53394b),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (oldObj = *(CBaseEntity **)(iVar2 + 4), oldObj != (CBaseEntity *)0x0)) {
    UTIL_RemoveImmediate(oldObj);
  }
  SpawnControlPanels(this);
  return;
}


/* CLabIndicatorPanel::StartTimer at 00713790 */

/* DWARF original prototype: void StartTimer(CLabIndicatorPanel * this, float flDuration) */

void __thiscall CLabIndicatorPanel::StartTimer(CLabIndicatorPanel *this,float flDuration)

{
  undefined1 *puVar1;
  int iVar2;
  float fVar3;
  ushort uVar4;
  CBaseEdict *pCVar5;
  ushort *puVar6;
  int *piVar7;
  IChangeInfoAccessor *pIVar8;
  uint uVar9;
  uint uVar10;
  int unaff_EBX;
  uint local_c0;
  uint local_bc;
  uint local_b8;
  uint local_b4;
  uint local_b0;
  
  ___i686_get_pc_thunk_bx();
  fVar3 = *(float *)(**(int **)(unaff_EBX + 0x5333c3) + 0xc);
  if (fVar3 != (this->m_flTimerStart).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar5 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
        pIVar8 = CBaseEdict::GetChangeAccessor(pCVar5);
        puVar6 = (ushort *)**(undefined4 **)(unaff_EBX + 0x5333c7);
        if (pIVar8->m_iChangeInfoSerialNumber == *puVar6) {
          uVar9 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar4 = puVar6[uVar9 * 0x14 + 0x14];
          if (uVar4 == 0) {
LAB_00713d67:
            puVar6[(uint)uVar4 + uVar9 * 0x14 + 1] = 0x3d0;
            puVar6[uVar9 * 0x14 + 0x14] = uVar4 + 1;
          }
          else if (puVar6[uVar9 * 0x14 + 1] != 0x3d0) {
            local_c0 = 0;
            do {
              uVar10 = local_c0 + 1;
              local_c0 = uVar10 & 0xffff;
              if ((ushort)uVar10 == uVar4) {
                if (uVar4 != 0x13) goto LAB_00713d67;
                pIVar8->m_iChangeInfoSerialNumber = 0;
                pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar6[uVar9 * 0x14 + local_c0 + 1] != 0x3d0);
          }
        }
        else if (puVar6[0x7d1] == 100) {
          pIVar8->m_iChangeInfoSerialNumber = 0;
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar8->m_iChangeInfo = puVar6[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x5333c7) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x5333c7) + 0xfa2) + 1;
          piVar7 = *(int **)(unaff_EBX + 0x5333c7);
          pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar7;
          iVar2 = *piVar7 + (uint)pIVar8->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3d0;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flTimerStart).m_Value = fVar3;
  }
  if ((this->m_bStopped).m_Value != false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar5 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
        pIVar8 = CBaseEdict::GetChangeAccessor(pCVar5);
        puVar6 = (ushort *)**(undefined4 **)(unaff_EBX + 0x5333c7);
        if (pIVar8->m_iChangeInfoSerialNumber == *puVar6) {
          uVar9 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar4 = puVar6[uVar9 * 0x14 + 0x14];
          if (uVar4 == 0) {
LAB_00713cb7:
            puVar6[(uint)uVar4 + uVar9 * 0x14 + 1] = 0x3c1;
            puVar6[uVar9 * 0x14 + 0x14] = uVar4 + 1;
          }
          else if (puVar6[uVar9 * 0x14 + 1] != 0x3c1) {
            local_bc = 0;
            do {
              uVar10 = local_bc + 1;
              local_bc = uVar10 & 0xffff;
              if ((ushort)uVar10 == uVar4) {
                if (uVar4 != 0x13) goto LAB_00713cb7;
                pIVar8->m_iChangeInfoSerialNumber = 0;
                pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar6[uVar9 * 0x14 + local_bc + 1] != 0x3c1);
          }
        }
        else if (puVar6[0x7d1] == 100) {
          pIVar8->m_iChangeInfoSerialNumber = 0;
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar8->m_iChangeInfo = puVar6[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x5333c7) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x5333c7) + 0xfa2) + 1;
          piVar7 = *(int **)(unaff_EBX + 0x5333c7);
          pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar7;
          iVar2 = *piVar7 + (uint)pIVar8->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3c1;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bStopped).m_Value = false;
  }
  if ((this->m_bCountingDown).m_Value == false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar5 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
        pIVar8 = CBaseEdict::GetChangeAccessor(pCVar5);
        puVar6 = (ushort *)**(undefined4 **)(unaff_EBX + 0x5333c7);
        if (pIVar8->m_iChangeInfoSerialNumber == *puVar6) {
          uVar9 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar4 = puVar6[uVar9 * 0x14 + 0x14];
          if (uVar4 == 0) {
LAB_00713ee7:
            puVar6[(uint)uVar4 + uVar9 * 0x14 + 1] = 0x3c2;
            puVar6[uVar9 * 0x14 + 0x14] = uVar4 + 1;
          }
          else if (puVar6[uVar9 * 0x14 + 1] != 0x3c2) {
            local_b8 = 0;
            do {
              uVar10 = local_b8 + 1;
              local_b8 = uVar10 & 0xffff;
              if ((ushort)uVar10 == uVar4) {
                if (uVar4 != 0x13) goto LAB_00713ee7;
                pIVar8->m_iChangeInfoSerialNumber = 0;
                pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar6[uVar9 * 0x14 + local_b8 + 1] != 0x3c2);
          }
        }
        else if (puVar6[0x7d1] == 100) {
          pIVar8->m_iChangeInfoSerialNumber = 0;
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar8->m_iChangeInfo = puVar6[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x5333c7) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x5333c7) + 0xfa2) + 1;
          piVar7 = *(int **)(unaff_EBX + 0x5333c7);
          pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar7;
          iVar2 = *piVar7 + (uint)pIVar8->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3c2;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bCountingDown).m_Value = true;
  }
  if ((this->m_bIsCountdownTimer).m_Value == false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar5 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
        pIVar8 = CBaseEdict::GetChangeAccessor(pCVar5);
        puVar6 = (ushort *)**(undefined4 **)(unaff_EBX + 0x5333c7);
        if (pIVar8->m_iChangeInfoSerialNumber == *puVar6) {
          uVar9 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar4 = puVar6[uVar9 * 0x14 + 0x14];
          if (uVar4 == 0) {
LAB_00713e37:
            puVar6[(uint)uVar4 + uVar9 * 0x14 + 1] = 0x3c3;
            puVar6[uVar9 * 0x14 + 0x14] = uVar4 + 1;
          }
          else if (puVar6[uVar9 * 0x14 + 1] != 0x3c3) {
            local_b4 = 0;
            do {
              uVar10 = local_b4 + 1;
              local_b4 = uVar10 & 0xffff;
              if ((ushort)uVar10 == uVar4) {
                if (uVar4 != 0x13) goto LAB_00713e37;
                pIVar8->m_iChangeInfoSerialNumber = 0;
                pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar6[uVar9 * 0x14 + local_b4 + 1] != 0x3c3);
          }
        }
        else if (puVar6[0x7d1] == 100) {
          pIVar8->m_iChangeInfoSerialNumber = 0;
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar8->m_iChangeInfo = puVar6[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x5333c7) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x5333c7) + 0xfa2) + 1;
          piVar7 = *(int **)(unaff_EBX + 0x5333c7);
          pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar7;
          iVar2 = *piVar7 + (uint)pIVar8->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3c3;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bIsCountdownTimer).m_Value = true;
  }
  if (flDuration != (this->m_flTimerDuration).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar5 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
        pIVar8 = CBaseEdict::GetChangeAccessor(pCVar5);
        puVar6 = (ushort *)**(undefined4 **)(unaff_EBX + 0x5333c7);
        if (pIVar8->m_iChangeInfoSerialNumber == *puVar6) {
          uVar9 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar4 = puVar6[uVar9 * 0x14 + 0x14];
          if (uVar4 == 0) {
LAB_00713f93:
            puVar6[(uint)uVar4 + uVar9 * 0x14 + 1] = 0x3cc;
            puVar6[uVar9 * 0x14 + 0x14] = uVar4 + 1;
          }
          else if (puVar6[uVar9 * 0x14 + 1] != 0x3cc) {
            local_b0 = 0;
            do {
              uVar10 = local_b0 + 1;
              local_b0 = uVar10 & 0xffff;
              if ((ushort)uVar10 == uVar4) {
                if (uVar4 != 0x13) goto LAB_00713f93;
                pIVar8->m_iChangeInfoSerialNumber = 0;
                pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar6[uVar9 * 0x14 + local_b0 + 1] != 0x3cc);
          }
        }
        else if (puVar6[0x7d1] == 100) {
          pIVar8->m_iChangeInfoSerialNumber = 0;
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar8->m_iChangeInfo = puVar6[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x5333c7) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x5333c7) + 0xfa2) + 1;
          piVar7 = *(int **)(unaff_EBX + 0x5333c7);
          pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar7;
          iVar2 = *piVar7 + (uint)pIVar8->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3cc;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flTimerDuration).m_Value = flDuration;
  }
  return;
}


/* CLabIndicatorPanel::ResetTimer at 007161d0 */

/* DWARF original prototype: void ResetTimer(CLabIndicatorPanel * this) */

void __thiscall CLabIndicatorPanel::ResetTimer(CLabIndicatorPanel *this)

{
  undefined1 *puVar1;
  int iVar2;
  ushort uVar3;
  CBaseEdict *pCVar4;
  ushort *puVar5;
  int *piVar6;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  uint uVar9;
  int unaff_EBX;
  uint local_88;
  uint local_84;
  uint local_80;
  
  ___i686_get_pc_thunk_bx();
  if ((this->m_bStopped).m_Value == false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar4 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x530981);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_0071663d:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x3c1;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 0x3c1) {
            local_88 = 0;
            do {
              uVar9 = local_88 + 1;
              local_88 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 != 0x13) goto LAB_0071663d;
                pIVar7->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar8 * 0x14 + local_88 + 1] != 0x3c1);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x530981) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x530981) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x530981);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3c1;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bStopped).m_Value = true;
  }
  if ((this->m_bCountingDown).m_Value != false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar4 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x530981);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_007164f8:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x3c2;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 0x3c2) {
            local_84 = 0;
            do {
              uVar9 = local_84 + 1;
              local_84 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 != 0x13) goto LAB_007164f8;
                pIVar7->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar8 * 0x14 + local_84 + 1] != 0x3c2);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x530981) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x530981) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x530981);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3c2;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bCountingDown).m_Value = false;
  }
  if ((this->m_bIsCountdownTimer).m_Value == false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar4 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x530981);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00716598:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x3c3;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 0x3c3) {
            local_80 = 0;
            do {
              uVar9 = local_80 + 1;
              local_80 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 != 0x13) goto LAB_00716598;
                pIVar7->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar8 * 0x14 + local_80 + 1] != 0x3c3);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x530981) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x530981) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x530981);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3c3;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bIsCountdownTimer).m_Value = true;
  }
  return;
}


/* CLabIndicatorPanel::SetChecked at 00714ba0 */

/* DWARF original prototype: void SetChecked(CLabIndicatorPanel * this, bool bIsChecked) */

void __thiscall CLabIndicatorPanel::SetChecked(CLabIndicatorPanel *this,bool bIsChecked)

{
  undefined1 *puVar1;
  int iVar2;
  ushort uVar3;
  CBaseEdict *pCVar4;
  ushort *puVar5;
  int *piVar6;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  uint uVar9;
  int unaff_EBX;
  uint local_74;
  uint local_70;
  
  ___i686_get_pc_thunk_bx();
  if (bIsChecked != (this->m_bIsChecked).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar4 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x531fb1);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00714e08:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x3c4;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 0x3c4) {
            local_74 = 0;
            do {
              uVar9 = local_74 + 1;
              local_74 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 == 0x13) goto LAB_00714d5e;
                goto LAB_00714e08;
              }
            } while (puVar5[uVar8 * 0x14 + local_74 + 1] != 0x3c4);
          }
        }
        else if (puVar5[0x7d1] == 100) {
LAB_00714d5e:
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x531fb1) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x531fb1) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x531fb1);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3c4;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bIsChecked).m_Value = bIsChecked;
  }
  if ((this->m_bIsCountdownTimer).m_Value != false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar4 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x531fb1);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00714ea8:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x3c3;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 0x3c3) {
            local_70 = 0;
            do {
              uVar9 = local_70 + 1;
              local_70 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 == 0x13) goto LAB_00714d72;
                goto LAB_00714ea8;
              }
            } while (puVar5[uVar8 * 0x14 + local_70 + 1] != 0x3c3);
          }
        }
        else if (puVar5[0x7d1] == 100) {
LAB_00714d72:
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x531fb1) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x531fb1) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x531fb1);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3c3;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bIsCountdownTimer).m_Value = false;
  }
  return;
}


/* CLabIndicatorPanel::SetPlayerPinged at 007151d0 */

/* DWARF original prototype: void SetPlayerPinged(CLabIndicatorPanel * this, int iPlayerPinged) */

void __thiscall CLabIndicatorPanel::SetPlayerPinged(CLabIndicatorPanel *this,int iPlayerPinged)

{
  undefined1 *puVar1;
  int iVar2;
  ushort uVar3;
  CBaseEdict *pCVar4;
  ushort *puVar5;
  int *piVar6;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  uint uVar9;
  int unaff_EBX;
  uint local_74;
  uint local_70;
  
  ___i686_get_pc_thunk_bx();
  if ((this->m_iPlayerPinged).m_Value != iPlayerPinged) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar4 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x531981);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00715438:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x3c8;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 0x3c8) {
            local_74 = 0;
            do {
              uVar9 = local_74 + 1;
              local_74 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 == 0x13) goto LAB_00715392;
                goto LAB_00715438;
              }
            } while (puVar5[uVar8 * 0x14 + local_74 + 1] != 0x3c8);
          }
        }
        else if (puVar5[0x7d1] == 100) {
LAB_00715392:
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x531981) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x531981) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x531981);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3c8;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_iPlayerPinged).m_Value = iPlayerPinged;
  }
  if ((this->m_bIsCountdownTimer).m_Value != false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar4 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x531981);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_007154d4:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x3c3;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 0x3c3) {
            local_70 = 0;
            do {
              uVar9 = local_70 + 1;
              local_70 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 != 0x13) goto LAB_007154d4;
                pIVar7->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar8 * 0x14 + local_70 + 1] != 0x3c3);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x531981) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x531981) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x531981);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3c3;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bIsCountdownTimer).m_Value = false;
  }
  return;
}


/* CLabIndicatorPanel::SetIsTimer at 00716020 */

/* DWARF original prototype: void SetIsTimer(CLabIndicatorPanel * this, bool bIsTimer) */

void __thiscall CLabIndicatorPanel::SetIsTimer(CLabIndicatorPanel *this,bool bIsTimer)

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
  if (bIsTimer != (this->m_bIsCountdownTimer).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x530b34);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00716194:
            puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x3c3;
            puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != 0x3c3) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_00716194;
                goto LAB_007161b9;
              }
            } while (puVar4[uVar7 * 0x14 + local_60 + 1] != 0x3c3);
          }
        }
        else if (puVar4[0x7d1] == 100) {
LAB_007161b9:
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x530b34) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x530b34) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x530b34);
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar2 = *piVar5 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3c3;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bIsCountdownTimer).m_Value = bIsTimer;
  }
  return;
}


/* DataMapInit<CPropIndicatorPanel> at 000a9ef0 */

datamap_t * DataMapInit<CPropIndicatorPanel>(CPropIndicatorPanel *param_1)

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
  if ((*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x404].m_pEntity + unaff_EBX) ==
       '\0') && (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xda1c24), iVar2 != 0)) {
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x405].m_pEntity + unaff_EBX)
         = &UNK_00a06aa6 + unaff_EBX;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x405].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x405].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x406].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x406].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x406].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x405].m_SerialNumber + unaff_EBX) = 0x13;
    ___cxa_guard_release(unaff_EBX + 0xda1c24);
    ___cxa_atexit(unaff_EBX + 0x99269c,0,*(undefined4 *)(&DAT_00b9cc50 + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_typeinfo_00cb9404 + unaff_EBX) =
       *(undefined4 *)(&DAT_00b9cd88 + unaff_EBX);
  if ((*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x404].m_pPrev + unaff_EBX) ==
       '\0') && (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xda1c2c), iVar2 != 0)) {
    pcVar3 = operator_new___(*(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x405].
                                            m_SerialNumber + unaff_EBX) + 0xc);
    _strcpy(pcVar3,*(char **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x405].m_pEntity +
                             unaff_EBX));
    sVar4 = _strlen(pcVar3);
    builtin_strncpy(pcVar3 + sVar4,"ToggleThink",0xc);
    iVar1 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x406].m_SerialNumber +
                    unaff_EBX);
    iVar2 = iVar1 + 1;
    iVar7 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x405].m_pNext + unaff_EBX);
    if (iVar7 < iVar2) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)
                 ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x405].m_pPrev + unaff_EBX),
                 iVar2 - iVar7);
    }
    iVar5 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x406].m_SerialNumber +
                    unaff_EBX) + 1;
    *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x406].m_SerialNumber + unaff_EBX) =
         iVar5;
    iVar7 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x405].m_pPrev + unaff_EBX);
    *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x406].m_pPrev + unaff_EBX) = iVar7;
    iVar5 = (iVar5 - iVar1) + -1;
    if (0 < iVar5) {
      _V_memmove((void *)(iVar7 + iVar2 * 4),(void *)(iVar7 + iVar1 * 4),iVar5 * 4);
      iVar7 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x405].m_pPrev + unaff_EBX
                      );
    }
    puVar6 = (undefined4 *)(iVar1 * 4 + iVar7);
    if (puVar6 != (undefined4 *)0x0) {
      *puVar6 = pcVar3;
    }
    *(char **)((int)&PTR_IsNetClient_00cb9900 + unaff_EBX) = pcVar3;
    ___cxa_guard_release(unaff_EBX + 0xda1c2c);
  }
  *(undefined4 *)((int)&CPointPlayerMoveConstraint::typeinfo + unaff_EBX) = 0xf;
  *(int *)((int)&PTR_NotifyPortalEvent_00cb93f8 + unaff_EBX) =
       (int)&PTR_MyNextBotPointer_00cb957c + unaff_EBX;
  return (datamap_t *)((int)&PTR_NotifyPortalEvent_00cb93f8 + unaff_EBX);
}


/* CPropIndicatorPanel::GetDataDescMap at 00712db0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPropIndicatorPanel * this) */

datamap_t * __thiscall CPropIndicatorPanel::GetDataDescMap(CPropIndicatorPanel *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x650544);
}


/* CPropIndicatorPanel::GetBaseMap at 00712dc0 */

datamap_t * CPropIndicatorPanel::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x533ec4);
}


/* __tcf_2 at 00a3c5a0 */

void __tcf_2(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x40f59e)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x40f592) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x40f59e));
  }
  *(undefined4 *)(unaff_EBX + 0x40f59e) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40f59a)) {
    if (*(int *)(unaff_EBX + 0x40f592) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20a5ae) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20a5ae),*(int *)(unaff_EBX + 0x40f592));
      *(undefined4 *)(unaff_EBX + 0x40f592) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f596) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40f592);
  *(int *)(unaff_EBX + 0x40f5a2) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40f59a)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20a5ae) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20a5ae),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40f592) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f596) = 0;
  }
  return;
}


/* CPropIndicatorPanel::Precache at 00712e20 */

/* DWARF original prototype: void Precache(CPropIndicatorPanel * this) */

void __thiscall CPropIndicatorPanel::Precache(CPropIndicatorPanel *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  UTIL_PrecacheOther((char *)(unaff_EBX + 0x39daf3),(char *)0x0);
  return;
}


/* CPropIndicatorPanel::Spawn at 00715f10 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "MoveCollide_t": Some values do not have unique names */
/* WARNING: Enum "MoveType_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(CPropIndicatorPanel * this) */

void __thiscall CPropIndicatorPanel::Spawn(CPropIndicatorPanel *this)

{
  int *piVar1;
  
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1a])(this);
  CCollisionProperty::SetSolidFlags
            (&(this->super_CBaseAnimating).super_CBaseEntity.m_Collision.super_CCollisionProperty,
             (this->super_CBaseAnimating).super_CBaseEntity.m_Collision.super_CCollisionProperty.
             m_usSolidFlags.m_Value | 4);
  CBaseEntity::SetMoveType((CBaseEntity *)this,MOVETYPE_NONE,MOVECOLLIDE_DEFAULT);
  piVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags;
  *piVar1 = *piVar1 | 0x80;
  CBaseEntity::DispatchUpdateTransmitState((CBaseEntity *)this);
  CreateIndicatorPanel(this);
  return;
}


/* CPropIndicatorPanel::UpdateOnRemove at 00712fd0 */

/* DWARF original prototype: void UpdateOnRemove(CPropIndicatorPanel * this) */

void __thiscall CPropIndicatorPanel::UpdateOnRemove(CPropIndicatorPanel *this)

{
  uint uVar1;
  int iVar2;
  CBaseEntity *oldObj;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->m_hIndicatorPanel).super_CBaseHandle.m_Index;
  if ((uVar1 == 0xffffffff) ||
     (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x533b8b),
     *(uint *)(iVar2 + 8) != uVar1 >> 0x10)) {
    oldObj = (CBaseEntity *)0x0;
  }
  else {
    oldObj = *(CBaseEntity **)(iVar2 + 4);
  }
  UTIL_Remove(oldObj);
  CBaseEntity::UpdateOnRemove((CBaseEntity *)this);
  return;
}


/* CPropIndicatorPanel::SetTimerDuration at 00712dd0 */

/* DWARF original prototype: void SetTimerDuration(CPropIndicatorPanel * this, float flDuration) */

void __thiscall CPropIndicatorPanel::SetTimerDuration(CPropIndicatorPanel *this,float flDuration)

{
  this->m_flTimerDuration = flDuration;
  return;
}


/* CPropIndicatorPanel::StartTimer at 00714050 */

/* DWARF original prototype: void StartTimer(CPropIndicatorPanel * this) */

void __thiscall CPropIndicatorPanel::StartTimer(CPropIndicatorPanel *this)

{
  uint uVar1;
  CLabIndicatorPanel *this_00;
  BASEPTR func;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->m_hIndicatorPanel).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x532b08),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (this_00 = *(CLabIndicatorPanel **)(iVar2 + 4), this_00 != (CLabIndicatorPanel *)0x0)) {
    CLabIndicatorPanel::StartTimer(this_00,this->m_flTimerDuration);
    ToggleIndicatorLights(this,true);
    func.__delta = 0;
    func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x934);
    CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
    CBaseEntity::SetNextThink
              ((CBaseEntity *)this,
               *(float *)(**(int **)(unaff_EBX + 0x532b00) + 0xc) + this->m_flTimerDuration,
               (char *)0x0);
  }
  return;
}


/* CPropIndicatorPanel::StopTimer at 00712df0 */

/* DWARF original prototype: void StopTimer(CPropIndicatorPanel * this) */

void __thiscall CPropIndicatorPanel::StopTimer(CPropIndicatorPanel *this)

{
  return;
}


/* CPropIndicatorPanel::ResetTimer at 007168b0 */

/* DWARF original prototype: void ResetTimer(CPropIndicatorPanel * this) */

void __thiscall CPropIndicatorPanel::ResetTimer(CPropIndicatorPanel *this)

{
  int iVar1;
  ushort uVar2;
  CBaseEdict *pCVar3;
  ushort *puVar4;
  int *piVar5;
  int iVar6;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  uint uVar9;
  int unaff_EBX;
  uint local_88;
  uint local_84;
  uint local_80;
  
  ___i686_get_pc_thunk_bx();
  uVar8 = (this->m_hIndicatorPanel).super_CBaseHandle.m_Index;
  if (((uVar8 != 0xffffffff) &&
      (iVar6 = (uVar8 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5302a5),
      *(uint *)(iVar6 + 8) == uVar8 >> 0x10)) && (iVar6 = *(int *)(iVar6 + 4), iVar6 != 0)) {
    if (*(char *)(iVar6 + 0x3c1) == '\0') {
      if (*(char *)(iVar6 + 0x60) == '\0') {
        pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
        if ((pCVar3 != (CBaseEdict *)0x0) && ((pCVar3->m_fStateFlags & 0x100U) == 0)) {
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 1;
          pIVar7 = CBaseEdict::GetChangeAccessor(pCVar3);
          puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x5302a1);
          if (pIVar7->m_iChangeInfoSerialNumber == *puVar4) {
            uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar2 = puVar4[uVar8 * 0x14 + 0x14];
            if (uVar2 == 0) {
LAB_00716c98:
              puVar4[(uint)uVar2 + uVar8 * 0x14 + 1] = 0x3c1;
              puVar4[uVar8 * 0x14 + 0x14] = uVar2 + 1;
            }
            else if (puVar4[uVar8 * 0x14 + 1] != 0x3c1) {
              local_88 = 0;
              do {
                uVar9 = local_88 + 1;
                local_88 = uVar9 & 0xffff;
                if ((ushort)uVar9 == uVar2) {
                  if (uVar2 != 0x13) goto LAB_00716c98;
                  pIVar7->m_iChangeInfoSerialNumber = 0;
                  pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar4[uVar8 * 0x14 + local_88 + 1] != 0x3c1);
            }
          }
          else if (puVar4[0x7d1] == 100) {
            pIVar7->m_iChangeInfoSerialNumber = 0;
            pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar7->m_iChangeInfo = puVar4[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x5302a1) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x5302a1) + 0xfa2) + 1;
            piVar5 = *(int **)(unaff_EBX + 0x5302a1);
            pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
            iVar1 = *piVar5 + (uint)pIVar7->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar1 + 2) = 0x3c1;
            *(undefined2 *)(iVar1 + 0x28) = 1;
          }
        }
      }
      else {
        *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
      }
      *(undefined1 *)(iVar6 + 0x3c1) = 1;
    }
    if (*(char *)(iVar6 + 0x3c2) != '\0') {
      if (*(char *)(iVar6 + 0x60) == '\0') {
        pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
        if ((pCVar3 != (CBaseEdict *)0x0) && ((pCVar3->m_fStateFlags & 0x100U) == 0)) {
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 1;
          pIVar7 = CBaseEdict::GetChangeAccessor(pCVar3);
          puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x5302a1);
          if (pIVar7->m_iChangeInfoSerialNumber == *puVar4) {
            uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar2 = puVar4[uVar8 * 0x14 + 0x14];
            if (uVar2 == 0) {
LAB_00716d52:
              puVar4[(uint)uVar2 + uVar8 * 0x14 + 1] = 0x3c2;
              puVar4[uVar8 * 0x14 + 0x14] = uVar2 + 1;
            }
            else if (puVar4[uVar8 * 0x14 + 1] != 0x3c2) {
              local_84 = 0;
              do {
                uVar9 = local_84 + 1;
                local_84 = uVar9 & 0xffff;
                if ((ushort)uVar9 == uVar2) {
                  if (uVar2 != 0x13) goto LAB_00716d52;
                  pIVar7->m_iChangeInfoSerialNumber = 0;
                  pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar4[uVar8 * 0x14 + local_84 + 1] != 0x3c2);
            }
          }
          else if (puVar4[0x7d1] == 100) {
            pIVar7->m_iChangeInfoSerialNumber = 0;
            pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar7->m_iChangeInfo = puVar4[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x5302a1) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x5302a1) + 0xfa2) + 1;
            piVar5 = *(int **)(unaff_EBX + 0x5302a1);
            pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
            iVar1 = *piVar5 + (uint)pIVar7->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar1 + 2) = 0x3c2;
            *(undefined2 *)(iVar1 + 0x28) = 1;
          }
        }
      }
      else {
        *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
      }
      *(undefined1 *)(iVar6 + 0x3c2) = 0;
    }
    if (*(char *)(iVar6 + 0x3c3) == '\0') {
      if (*(char *)(iVar6 + 0x60) == '\0') {
        pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
        if ((pCVar3 != (CBaseEdict *)0x0) && ((pCVar3->m_fStateFlags & 0x100U) == 0)) {
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 1;
          pIVar7 = CBaseEdict::GetChangeAccessor(pCVar3);
          puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x5302a1);
          if (pIVar7->m_iChangeInfoSerialNumber == *puVar4) {
            uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar2 = puVar4[uVar8 * 0x14 + 0x14];
            if (uVar2 == 0) {
LAB_00716bf8:
              puVar4[(uint)uVar2 + uVar8 * 0x14 + 1] = 0x3c3;
              puVar4[uVar8 * 0x14 + 0x14] = uVar2 + 1;
            }
            else if (puVar4[uVar8 * 0x14 + 1] != 0x3c3) {
              local_80 = 0;
              do {
                uVar9 = local_80 + 1;
                local_80 = uVar9 & 0xffff;
                if ((ushort)uVar9 == uVar2) {
                  if (uVar2 != 0x13) goto LAB_00716bf8;
                  pIVar7->m_iChangeInfoSerialNumber = 0;
                  pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar4[uVar8 * 0x14 + local_80 + 1] != 0x3c3);
            }
          }
          else if (puVar4[0x7d1] == 100) {
            pIVar7->m_iChangeInfoSerialNumber = 0;
            pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar7->m_iChangeInfo = puVar4[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x5302a1) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x5302a1) + 0xfa2) + 1;
            piVar5 = *(int **)(unaff_EBX + 0x5302a1);
            pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
            iVar1 = *piVar5 + (uint)pIVar7->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar1 + 2) = 0x3c3;
            *(undefined2 *)(iVar1 + 0x28) = 1;
          }
        }
      }
      else {
        *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
      }
      *(undefined1 *)(iVar6 + 0x3c3) = 1;
    }
  }
  return;
}


/* CPropIndicatorPanel::InputReset at 00716dd0 */

/* DWARF original prototype: void InputReset(CPropIndicatorPanel * this, inputdata_t * indata) */

void __thiscall CPropIndicatorPanel::InputReset(CPropIndicatorPanel *this,inputdata_t *indata)

{
  int iVar1;
  ushort uVar2;
  CBaseEdict *pCVar3;
  ushort *puVar4;
  int *piVar5;
  int iVar6;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  uint uVar9;
  int unaff_EBX;
  uint local_88;
  uint local_84;
  uint local_80;
  
  ___i686_get_pc_thunk_bx();
  uVar8 = (this->m_hIndicatorPanel).super_CBaseHandle.m_Index;
  if (((uVar8 != 0xffffffff) &&
      (iVar6 = (uVar8 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x52fd85),
      *(uint *)(iVar6 + 8) == uVar8 >> 0x10)) && (iVar6 = *(int *)(iVar6 + 4), iVar6 != 0)) {
    if (*(char *)(iVar6 + 0x3c1) == '\0') {
      if (*(char *)(iVar6 + 0x60) == '\0') {
        pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
        if ((pCVar3 != (CBaseEdict *)0x0) && ((pCVar3->m_fStateFlags & 0x100U) == 0)) {
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 1;
          pIVar7 = CBaseEdict::GetChangeAccessor(pCVar3);
          puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x52fd81);
          if (pIVar7->m_iChangeInfoSerialNumber == *puVar4) {
            uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar2 = puVar4[uVar8 * 0x14 + 0x14];
            if (uVar2 == 0) {
LAB_007171b8:
              puVar4[(uint)uVar2 + uVar8 * 0x14 + 1] = 0x3c1;
              puVar4[uVar8 * 0x14 + 0x14] = uVar2 + 1;
            }
            else if (puVar4[uVar8 * 0x14 + 1] != 0x3c1) {
              local_88 = 0;
              do {
                uVar9 = local_88 + 1;
                local_88 = uVar9 & 0xffff;
                if ((ushort)uVar9 == uVar2) {
                  if (uVar2 != 0x13) goto LAB_007171b8;
                  pIVar7->m_iChangeInfoSerialNumber = 0;
                  pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar4[uVar8 * 0x14 + local_88 + 1] != 0x3c1);
            }
          }
          else if (puVar4[0x7d1] == 100) {
            pIVar7->m_iChangeInfoSerialNumber = 0;
            pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar7->m_iChangeInfo = puVar4[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x52fd81) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x52fd81) + 0xfa2) + 1;
            piVar5 = *(int **)(unaff_EBX + 0x52fd81);
            pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
            iVar1 = *piVar5 + (uint)pIVar7->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar1 + 2) = 0x3c1;
            *(undefined2 *)(iVar1 + 0x28) = 1;
          }
        }
      }
      else {
        *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
      }
      *(undefined1 *)(iVar6 + 0x3c1) = 1;
    }
    if (*(char *)(iVar6 + 0x3c2) != '\0') {
      if (*(char *)(iVar6 + 0x60) == '\0') {
        pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
        if ((pCVar3 != (CBaseEdict *)0x0) && ((pCVar3->m_fStateFlags & 0x100U) == 0)) {
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 1;
          pIVar7 = CBaseEdict::GetChangeAccessor(pCVar3);
          puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x52fd81);
          if (pIVar7->m_iChangeInfoSerialNumber == *puVar4) {
            uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar2 = puVar4[uVar8 * 0x14 + 0x14];
            if (uVar2 == 0) {
LAB_00717272:
              puVar4[(uint)uVar2 + uVar8 * 0x14 + 1] = 0x3c2;
              puVar4[uVar8 * 0x14 + 0x14] = uVar2 + 1;
            }
            else if (puVar4[uVar8 * 0x14 + 1] != 0x3c2) {
              local_84 = 0;
              do {
                uVar9 = local_84 + 1;
                local_84 = uVar9 & 0xffff;
                if ((ushort)uVar9 == uVar2) {
                  if (uVar2 != 0x13) goto LAB_00717272;
                  pIVar7->m_iChangeInfoSerialNumber = 0;
                  pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar4[uVar8 * 0x14 + local_84 + 1] != 0x3c2);
            }
          }
          else if (puVar4[0x7d1] == 100) {
            pIVar7->m_iChangeInfoSerialNumber = 0;
            pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar7->m_iChangeInfo = puVar4[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x52fd81) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x52fd81) + 0xfa2) + 1;
            piVar5 = *(int **)(unaff_EBX + 0x52fd81);
            pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
            iVar1 = *piVar5 + (uint)pIVar7->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar1 + 2) = 0x3c2;
            *(undefined2 *)(iVar1 + 0x28) = 1;
          }
        }
      }
      else {
        *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
      }
      *(undefined1 *)(iVar6 + 0x3c2) = 0;
    }
    if (*(char *)(iVar6 + 0x3c3) == '\0') {
      if (*(char *)(iVar6 + 0x60) == '\0') {
        pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
        if ((pCVar3 != (CBaseEdict *)0x0) && ((pCVar3->m_fStateFlags & 0x100U) == 0)) {
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 1;
          pIVar7 = CBaseEdict::GetChangeAccessor(pCVar3);
          puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x52fd81);
          if (pIVar7->m_iChangeInfoSerialNumber == *puVar4) {
            uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar2 = puVar4[uVar8 * 0x14 + 0x14];
            if (uVar2 == 0) {
LAB_00717118:
              puVar4[(uint)uVar2 + uVar8 * 0x14 + 1] = 0x3c3;
              puVar4[uVar8 * 0x14 + 0x14] = uVar2 + 1;
            }
            else if (puVar4[uVar8 * 0x14 + 1] != 0x3c3) {
              local_80 = 0;
              do {
                uVar9 = local_80 + 1;
                local_80 = uVar9 & 0xffff;
                if ((ushort)uVar9 == uVar2) {
                  if (uVar2 != 0x13) goto LAB_00717118;
                  pIVar7->m_iChangeInfoSerialNumber = 0;
                  pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar4[uVar8 * 0x14 + local_80 + 1] != 0x3c3);
            }
          }
          else if (puVar4[0x7d1] == 100) {
            pIVar7->m_iChangeInfoSerialNumber = 0;
            pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar7->m_iChangeInfo = puVar4[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x52fd81) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x52fd81) + 0xfa2) + 1;
            piVar5 = *(int **)(unaff_EBX + 0x52fd81);
            pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
            iVar1 = *piVar5 + (uint)pIVar7->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar1 + 2) = 0x3c3;
            *(undefined2 *)(iVar1 + 0x28) = 1;
          }
        }
      }
      else {
        *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
      }
      *(undefined1 *)(iVar6 + 0x3c3) = 1;
    }
  }
  return;
}


/* CPropIndicatorPanel::InputStart at 00714130 */

/* DWARF original prototype: void InputStart(CPropIndicatorPanel * this, inputdata_t * indata) */

void __thiscall CPropIndicatorPanel::InputStart(CPropIndicatorPanel *this,inputdata_t *indata)

{
  StartTimer(this);
  return;
}


/* CPropIndicatorPanel::InputStop at 00713130 */

/* DWARF original prototype: void InputStop(CPropIndicatorPanel * this, inputdata_t * indata) */

void __thiscall CPropIndicatorPanel::InputStop(CPropIndicatorPanel *this,inputdata_t *indata)

{
  return;
}


/* CPropIndicatorPanel::InputCheck at 00714f70 */

/* DWARF original prototype: void InputCheck(CPropIndicatorPanel * this, inputdata_t * indata) */

void __thiscall CPropIndicatorPanel::InputCheck(CPropIndicatorPanel *this,inputdata_t *indata)

{
  uint uVar1;
  CLabIndicatorPanel *this_00;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->m_hIndicatorPanel).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x531be8),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (this_00 = *(CLabIndicatorPanel **)(iVar2 + 4), this_00 != (CLabIndicatorPanel *)0x0)) {
    CLabIndicatorPanel::SetChecked(this_00,true);
  }
  CBaseEntity::ThinkSet((CBaseEntity *)this,(BASEPTR)0x0,0.0,(char *)0x0);
  ToggleIndicatorLights(this,true);
  return;
}


/* CPropIndicatorPanel::InputUncheck at 00714ed0 */

/* DWARF original prototype: void InputUncheck(CPropIndicatorPanel * this, inputdata_t * indata) */

void __thiscall CPropIndicatorPanel::InputUncheck(CPropIndicatorPanel *this,inputdata_t *indata)

{
  uint uVar1;
  CLabIndicatorPanel *this_00;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->m_hIndicatorPanel).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x531c88),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (this_00 = *(CLabIndicatorPanel **)(iVar2 + 4), this_00 != (CLabIndicatorPanel *)0x0)) {
    CLabIndicatorPanel::SetChecked(this_00,false);
  }
  CBaseEntity::ThinkSet((CBaseEntity *)this,(BASEPTR)0x0,0.0,(char *)0x0);
  ToggleIndicatorLights(this,false);
  return;
}


/* CPropIndicatorPanel::CreateIndicatorPanel at 00715880 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CreateIndicatorPanel(CPropIndicatorPanel * this) */

void __thiscall CPropIndicatorPanel::CreateIndicatorPanel(CPropIndicatorPanel *this)

{
  int iVar1;
  bool bVar2;
  ushort uVar3;
  CBaseEdict *this_00;
  ushort *puVar4;
  CBaseEntity *pCVar5;
  uint *puVar6;
  int iVar7;
  CLabIndicatorPanel *pCVar8;
  IChangeInfoAccessor *pIVar9;
  uint uVar10;
  uint uVar11;
  int *piVar12;
  int unaff_EBX;
  uint local_1c0;
  VMatrix local_148;
  VMatrix local_108;
  VMatrix local_c8;
  Vector local_88;
  float local_7c;
  float local_78;
  float local_74;
  float local_70;
  float local_6c;
  float local_68;
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
  Vector local_34;
  QAngle local_28 [2];
  
                    /* Unresolved local var: QAngle angles@[???]
                       Unresolved local var: VMatrix mat@[???]
                       Unresolved local var: VMatrix rotation@[???]
                       Unresolved local var: VMatrix tmp@[???]
                       Unresolved local var: Vector vecForward@[???]
                       Unresolved local var: Vector vecRight@[???]
                       Unresolved local var: Vector vecUp@[???]
                       Unresolved local var: Vector vecOffset@[???] */
  ___i686_get_pc_thunk_bx();
  pCVar5 = CreateEntityByName((char *)(unaff_EBX + 0x39b08e),-1,true);
  if (pCVar5 == (CBaseEntity *)0x0) {
    (this->m_hIndicatorPanel).super_CBaseHandle.m_Index = 0xffffffff;
    return;
  }
  puVar6 = (uint *)(*(pCVar5->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                     _vptr_IHandleEntity[3])(pCVar5);
  uVar10 = *puVar6;
  (this->m_hIndicatorPanel).super_CBaseHandle.m_Index = uVar10;
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
  if (((uVar10 == 0xffffffff) ||
      (iVar7 = (uVar10 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5312db),
      *(uint *)(iVar7 + 8) != uVar10 >> 0x10)) || (*(int *)(iVar7 + 4) == 0)) {
    return;
  }
  if (((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
  local_28[0].x = (this->super_CBaseAnimating).super_CBaseEntity.m_angAbsRotation.x;
  local_28[0].y = (this->super_CBaseAnimating).super_CBaseEntity.m_angAbsRotation.y;
  local_28[0].z = (this->super_CBaseAnimating).super_CBaseEntity.m_angAbsRotation.z;
  MatrixFromAngles(local_28,&local_c8);
  local_34.x = 0.0;
  local_34.y = 1.0;
  local_34.z = 0.0;
  MatrixBuildRotationAboutAxis(&local_108,&local_34,90.0);
  MatrixMultiply(&local_c8,&local_108,&local_148);
  MatrixBuildRotateZ(&local_108,90.0);
  MatrixMultiply(&local_148,&local_108,&local_c8);
  MatrixToAngles(&local_c8,local_28);
  uVar10 = (this->m_hIndicatorPanel).super_CBaseHandle.m_Index;
  if ((uVar10 == 0xffffffff) ||
     (iVar7 = (uVar10 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5312db),
     *(uint *)(iVar7 + 8) != uVar10 >> 0x10)) {
    pCVar5 = (CBaseEntity *)0x0;
  }
  else {
    pCVar5 = *(CBaseEntity **)(iVar7 + 4);
  }
  CBaseEntity::SetAbsAngles(pCVar5,local_28);
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x8e])(this,&local_40,&local_4c,&local_58);
                    /* Unresolved local var: Vector res@[???] */
  local_68 = *(float *)(unaff_EBX + 0x3bbd3b);
  local_70 = local_58 * local_68;
  local_6c = local_54 * local_68;
  local_68 = local_68 * local_50;
                    /* Unresolved local var: Vector res@[???] */
  local_5c = *(float *)(unaff_EBX + 0x3b72f3);
  local_64 = local_4c * local_5c;
  local_60 = local_48 * local_5c;
  local_5c = local_5c * local_44;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
  local_88.x = (local_64 - local_40) + local_70;
  local_88.y = (local_60 - local_3c) + local_6c;
  local_88.z = (local_5c - local_38) + local_68;
  local_7c = local_88.x;
  local_78 = local_88.y;
  local_74 = local_88.z;
  if (((this->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
                    /* Unresolved local var: Vector res@[???] */
  local_88.x = local_88.x + (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.x;
  local_88.y = local_88.y + (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.y;
  local_88.z = local_88.z + (this->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.z;
  uVar10 = (this->m_hIndicatorPanel).super_CBaseHandle.m_Index;
  if ((uVar10 == 0xffffffff) ||
     (iVar7 = (uVar10 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5312db),
     *(uint *)(iVar7 + 8) != uVar10 >> 0x10)) {
    pCVar5 = (CBaseEntity *)0x0;
  }
  else {
    pCVar5 = *(CBaseEntity **)(iVar7 + 4);
  }
  CBaseEntity::SetAbsOrigin(pCVar5,&local_88);
  uVar10 = (this->m_hIndicatorPanel).super_CBaseHandle.m_Index;
  if ((uVar10 == 0xffffffff) ||
     (iVar7 = (uVar10 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5312db),
     *(uint *)(iVar7 + 8) != uVar10 >> 0x10)) {
    piVar12 = (int *)0x0;
  }
  else {
    piVar12 = *(int **)(iVar7 + 4);
  }
  (**(code **)(*piVar12 + 0x98))(piVar12,this,0xffffffff);
  uVar10 = (this->m_hIndicatorPanel).super_CBaseHandle.m_Index;
  if ((uVar10 == 0xffffffff) ||
     (iVar7 = (uVar10 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5312db),
     *(uint *)(iVar7 + 8) != uVar10 >> 0x10)) {
    pCVar8 = (CLabIndicatorPanel *)0x0;
  }
  else {
    pCVar8 = *(CLabIndicatorPanel **)(iVar7 + 4);
  }
  CLabIndicatorPanel::SetPlayerPinged(pCVar8,-1);
  uVar10 = (this->m_hIndicatorPanel).super_CBaseHandle.m_Index;
  if ((uVar10 == 0xffffffff) ||
     (iVar7 = (uVar10 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5312db),
     *(uint *)(iVar7 + 8) != uVar10 >> 0x10)) {
    pCVar8 = (CLabIndicatorPanel *)0x0;
  }
  else {
    pCVar8 = *(CLabIndicatorPanel **)(iVar7 + 4);
  }
  CLabIndicatorPanel::SetChecked(pCVar8,this->m_bIsChecked);
  bVar2 = this->m_bIsCountdownTimer;
  uVar10 = (this->m_hIndicatorPanel).super_CBaseHandle.m_Index;
  if ((uVar10 == 0xffffffff) ||
     (iVar7 = (uVar10 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5312db),
     *(uint *)(iVar7 + 8) != uVar10 >> 0x10)) {
    iVar7 = 0;
  }
  else {
    iVar7 = *(int *)(iVar7 + 4);
  }
  if ((bool)*(char *)(iVar7 + 0x3c3) != bVar2) {
    if (*(char *)(iVar7 + 0x60) == '\0') {
      this_00 = *(CBaseEdict **)(iVar7 + 0x24);
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar9 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x5312d7);
        if (pIVar9->m_iChangeInfoSerialNumber == *puVar4) {
          uVar10 = (uint)pIVar9->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar10 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00715eae:
            puVar4[(uint)uVar3 + uVar10 * 0x14 + 1] = 0x3c3;
            puVar4[uVar10 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar10 * 0x14 + 1] != 0x3c3) {
            local_1c0 = 0;
            do {
              uVar11 = local_1c0 + 1;
              local_1c0 = uVar11 & 0xffff;
              if ((ushort)uVar11 == uVar3) {
                if (uVar3 != 0x13) goto LAB_00715eae;
                pIVar9->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar10 * 0x14 + local_1c0 + 1] != 0x3c3);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar9->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar9->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x5312d7) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x5312d7) + 0xfa2) + 1;
          piVar12 = *(int **)(unaff_EBX + 0x5312d7);
          pIVar9->m_iChangeInfoSerialNumber = *(ushort *)*piVar12;
          iVar1 = *piVar12 + (uint)pIVar9->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar1 + 2) = 0x3c3;
          *(undefined2 *)(iVar1 + 0x28) = 1;
        }
      }
    }
    else {
      *(uint *)(iVar7 + 100) = *(uint *)(iVar7 + 100) | 1;
    }
    *(bool *)(iVar7 + 0x3c3) = bVar2;
    uVar10 = (this->m_hIndicatorPanel).super_CBaseHandle.m_Index;
  }
  if ((uVar10 == 0xffffffff) ||
     (iVar7 = (uVar10 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5312db),
     *(uint *)(iVar7 + 8) != uVar10 >> 0x10)) {
    pCVar5 = (CBaseEntity *)0x0;
  }
  else {
    pCVar5 = *(CBaseEntity **)(iVar7 + 4);
  }
  DispatchSpawn(pCVar5,true);
  return;
}


/* CPropIndicatorPanel::ToggleIndicatorLights at 00713520 */

/* DWARF original prototype: void ToggleIndicatorLights(CPropIndicatorPanel * this, bool bLightsOn)
    */

void __thiscall CPropIndicatorPanel::ToggleIndicatorLights(CPropIndicatorPanel *this,bool bLightsOn)

{
  undefined1 *puVar1;
  int iVar2;
  ushort uVar3;
  CBaseEdict *this_00;
  ushort *puVar4;
  int *piVar5;
  char *szName;
  IChangeInfoAccessor *pIVar6;
  uint uVar7;
  uint uVar8;
  int unaff_EBX;
  CBaseEntity *pStartEntity;
  ushort i;
  
                    /* Unresolved local var: CBaseEntity * pLight@[???] */
  ___i686_get_pc_thunk_bx();
  pStartEntity = (CBaseEntity *)0x0;
  do {
    do {
      szName = (this->m_strIndicatorLights).pszValue;
      if (szName == (char *)0x0) {
        szName = &UNK_0033b7f6 + unaff_EBX;
      }
      pStartEntity = CGlobalEntityList::FindEntityByName
                               (*(CGlobalEntityList **)(unaff_EBX + 0x5336f6),pStartEntity,szName,
                                (CBaseEntity *)0x0,(CBaseEntity *)0x0,(CBaseEntity *)0x0,
                                (IEntityFindFilter *)0x0);
      if (pStartEntity == (CBaseEntity *)0x0) {
        return;
      }
    } while ((bool)(pStartEntity->m_iTextureFrameIndex).m_Value == bLightsOn);
    if ((pStartEntity->m_Network).m_TimerEvent.m_bRegistered == false) {
      this_00 = &((pStartEntity->m_Network).m_pPev)->super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x53363a);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
          uVar7 = (uint)pIVar6->m_iChangeInfo;
          uVar3 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_007136d4:
            puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x2ac;
            puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != 0x2ac) {
            _i = 0;
            do {
              uVar8 = _i + 1;
              _i = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_007136d4;
                pIVar6->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar7 * 0x14 + _i + 1] != 0x2ac);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x53363a) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x53363a) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x53363a);
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar2 = *piVar5 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x2ac;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(pStartEntity->m_Network).field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (pStartEntity->m_iTextureFrameIndex).m_Value = bLightsOn;
  } while( true );
}


/* CPropIndicatorPanel::ToggleThink at 00713730 */

/* DWARF original prototype: void ToggleThink(CPropIndicatorPanel * this) */

void __thiscall CPropIndicatorPanel::ToggleThink(CPropIndicatorPanel *this)

{
  ToggleIndicatorLights(this,false);
  CBaseEntity::ThinkSet((CBaseEntity *)this,(BASEPTR)0x0,0.0,(char *)0x0);
  return;
}


/* DataMapInit<CPropTicTacToePanel> at 000a9dc0 */

/* WARNING: Struct "CPropTicTacToePanel": ignoring overlapping field "m_bPlayed" */

datamap_t * DataMapInit<CPropTicTacToePanel>(CPropTicTacToePanel *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x414].m_pEntity + unaff_EBX) ==
      '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xda1d24);
    if (iVar2 != 0) {
      *(undefined **)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x415].m_pEntity + unaff_EBX) =
           &UNK_00a06bc2 + unaff_EBX;
      *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x415].m_pPrev + unaff_EBX)
           = 0;
      *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x415].m_pNext + unaff_EBX)
           = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x416].m_pEntity + unaff_EBX) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x416].m_SerialNumber + unaff_EBX) = 0;
      *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x416].m_pPrev + unaff_EBX)
           = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x415].m_SerialNumber + unaff_EBX) = 0x13
      ;
      ___cxa_guard_release(unaff_EBX + 0xda1d24);
      ___cxa_atexit(&UNK_0099298c + unaff_EBX,0,*(undefined4 *)(&DAT_00b9cd80 + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_TraceAttack_00cb954c + unaff_EBX) =
       *(undefined4 *)(&DAT_00b9ceb8 + unaff_EBX);
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x414].m_pPrev + unaff_EBX) ==
      '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xda1d2c);
    if (iVar2 != 0) {
      uVar1 = **(undefined4 **)(&DAT_00b9cda0 + unaff_EBX);
      *(undefined4 *)((int)&PTR_UpdateOnRemove_00cb9600 + unaff_EBX) = uVar1;
      *(undefined4 *)((int)&PTR_NetworkStateChanged_m_iMaxHealth_00cb9640 + unaff_EBX) = uVar1;
      ___cxa_guard_release(unaff_EBX + 0xda1d2c);
    }
  }
  *(undefined4 *)((int)&PTR_GetBeamTraceFilter_00cb9544 + unaff_EBX) = 3;
  *(int *)((int)&PTR_GetAutoAimCenter_00cb9540 + unaff_EBX) =
       (int)&PTR_IsBaseTrain_00cb95ac + unaff_EBX;
  return (datamap_t *)((int)&PTR_GetAutoAimCenter_00cb9540 + unaff_EBX);
}


/* CPropTicTacToePanel::GetDataDescMap at 00712e00 */

/* WARNING: Struct "CPropTicTacToePanel": ignoring overlapping field "m_bPlayed" */
/* DWARF original prototype: datamap_t * GetDataDescMap(CPropTicTacToePanel * this) */

datamap_t * __thiscall CPropTicTacToePanel::GetDataDescMap(CPropTicTacToePanel *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x65050c);
}


/* CPropTicTacToePanel::GetBaseMap at 00712e10 */

datamap_t * CPropTicTacToePanel::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x533e74);
}


/* __tcf_3 at 00a3c760 */

void __tcf_3(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x40f3ae)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x40f3a2) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x40f3ae));
  }
  *(undefined4 *)(unaff_EBX + 0x40f3ae) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40f3aa)) {
    if (*(int *)(unaff_EBX + 0x40f3a2) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20a3ee) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20a3ee),*(int *)(unaff_EBX + 0x40f3a2));
      *(undefined4 *)(unaff_EBX + 0x40f3a2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f3a6) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40f3a2);
  *(int *)(&DAT_0040f3b2 + unaff_EBX) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40f3aa)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20a3ee) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20a3ee),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40f3a2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f3a6) = 0;
  }
  return;
}


/* CPropTicTacToePanel::Spawn at 00715f80 */

/* WARNING: Struct "CPropTicTacToePanel": ignoring overlapping field "m_bPlayed" */
/* DWARF original prototype: void Spawn(CPropTicTacToePanel * this) */

void __thiscall CPropTicTacToePanel::Spawn(CPropTicTacToePanel *this)

{
  uint uVar1;
  int iVar2;
  CLabIndicatorPanel *this_00;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CPropIndicatorPanel::Spawn(&this->super_CPropIndicatorPanel);
                    /* Unresolved local var: bool bServerSide@[???] */
  (this->super_CGameEventListener).m_bRegisteredForEvents = true;
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x530bfb) + 0x10))
            ((int *)**(undefined4 **)(unaff_EBX + 0x530bfb),&this->super_CGameEventListener,
             unaff_EBX + 0x3965ae,1);
  uVar1 = (this->super_CPropIndicatorPanel).m_hIndicatorPanel.super_CBaseHandle.m_Index;
  if ((uVar1 == 0xffffffff) ||
     (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x530bdf),
     *(uint *)(iVar2 + 8) != uVar1 >> 0x10)) {
    this_00 = (CLabIndicatorPanel *)0x0;
  }
  else {
    this_00 = *(CLabIndicatorPanel **)(iVar2 + 4);
  }
  CLabIndicatorPanel::SetPlayerPinged(this_00,0);
  (this->super_CGameEventListener).field_0x9 = 0;
  return;
}


/* CPropTicTacToePanel::FireGameEvent at 00715520 */

/* WARNING: Struct "CPropTicTacToePanel": ignoring overlapping field "m_bPlayed" */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void FireGameEvent(CPropTicTacToePanel * this, IGameEvent * event) */

void __thiscall CPropTicTacToePanel::FireGameEvent(CPropTicTacToePanel *this,IGameEvent *event)

{
  int *piVar1;
  uint uVar2;
  char *s1;
  int iVar3;
  CBasePlayer *pCVar4;
  CLabIndicatorPanel *pCVar5;
  int unaff_EBX;
  longdouble extraout_ST0;
  longdouble extraout_ST0_00;
  longdouble extraout_ST0_01;
  float fVar6;
  int local_5c;
  Vector local_4c;
  float local_40;
  float local_3c;
  float local_38;
  Vector local_34;
  Vector local_28 [2];
  
                    /* Unresolved local var: char * name@[???] */
  ___i686_get_pc_thunk_bx();
  if ((this->super_CGameEventListener).field_0x9 == '\0') {
    s1 = (char *)(*event->_vptr_IGameEvent[2])(event);
    iVar3 = _V_strcmp(s1,&UNK_0039700d + unaff_EBX);
    if (iVar3 == 0) {
      local_5c = (*event->_vptr_IGameEvent[7])(event,&UNK_00343e96 + unaff_EBX,0);
      (*event->_vptr_IGameEvent[9])(event,unaff_EBX + 0x398594,0);
      (*event->_vptr_IGameEvent[9])(event,unaff_EBX + 0x397022,0);
      (*event->_vptr_IGameEvent[9])(event,unaff_EBX + 0x39859e,0);
      iVar3 = 1;
      do {
                    /* Unresolved local var: CBasePlayer * pPlayer@[???] */
        pCVar4 = UTIL_PlayerByIndex(iVar3);
        if (pCVar4 != (CBasePlayer *)0x0) {
          iVar3 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x531652) + 0x40))
                            ((int *)**(undefined4 **)(unaff_EBX + 0x531652),
                             (pCVar4->super_CBaseCombatCharacter).super_CBaseFlex.
                             super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                             m_Network.m_pPev);
          local_5c = (local_5c != iVar3) + 1;
          break;
        }
                    /* Unresolved local var: int i@[???] */
        iVar3 = iVar3 + 1;
      } while (iVar3 != 0x22);
      CCollisionProperty::CollisionAABBToWorldAABB
                (&(this->super_CPropIndicatorPanel).super_CBaseAnimating.super_CBaseEntity.
                  m_Collision.super_CCollisionProperty,
                 (Vector *)
                 &(this->super_CPropIndicatorPanel).super_CBaseAnimating.super_CBaseEntity.
                  m_Collision.super_CCollisionProperty.m_vecMins,
                 (Vector *)
                 &(this->super_CPropIndicatorPanel).super_CBaseAnimating.super_CBaseEntity.
                  m_Collision.super_CCollisionProperty.m_vecMaxs,local_28,&local_34);
      if (((this->super_CPropIndicatorPanel).super_CBaseAnimating.super_CBaseEntity.m_iEFlags &
          0x800) != 0) {
        CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
      }
      fVar6 = local_28[0].x -
              (this->super_CPropIndicatorPanel).super_CBaseAnimating.super_CBaseEntity.
              m_vecAbsOrigin.x;
      local_28[0].x =
           local_28[0].y -
           (this->super_CPropIndicatorPanel).super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.y
      ;
      local_28[0].z =
           local_28[0].z -
           (this->super_CPropIndicatorPanel).super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z
      ;
      local_28[0].y = fVar6;
      if (((this->super_CPropIndicatorPanel).super_CBaseAnimating.super_CBaseEntity.m_iEFlags &
          0x800) != 0) {
        CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
      }
                    /* Unresolved local var: Vector res@[???] */
      local_40 = (this->super_CPropIndicatorPanel).super_CBaseAnimating.super_CBaseEntity.
                 m_vecAbsOrigin.x - local_28[0].x;
      local_3c = (this->super_CPropIndicatorPanel).super_CBaseAnimating.super_CBaseEntity.
                 m_vecAbsOrigin.y - local_28[0].y;
      local_38 = (this->super_CPropIndicatorPanel).super_CBaseAnimating.super_CBaseEntity.
                 m_vecAbsOrigin.z - local_28[0].z;
      local_34.x = local_40;
      local_34.y = local_3c;
      local_34.z = local_38;
      if (((this->super_CPropIndicatorPanel).super_CBaseAnimating.super_CBaseEntity.m_iEFlags &
          0x800) != 0) {
        CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
      }
      local_28[0].x =
           local_28[0].x +
           (this->super_CPropIndicatorPanel).super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.x
      ;
      local_28[0].y =
           local_28[0].y +
           (this->super_CPropIndicatorPanel).super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.y
      ;
      local_28[0].z =
           local_28[0].z +
           (this->super_CPropIndicatorPanel).super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z
      ;
      local_4c.x = (float)extraout_ST0;
      local_4c.y = (float)extraout_ST0_00;
      local_4c.z = (float)extraout_ST0_01;
      fVar6 = CalcSqrDistanceToAABB(local_28,&local_34,&local_4c);
      if ((fVar6 < *(float *)(unaff_EBX + 0x3b79a2)) &&
         (!NAN(fVar6) && !NAN(*(float *)(unaff_EBX + 0x3b79a2)))) {
        piVar1 = *(int **)(unaff_EBX + 0x53163e);
        uVar2 = (this->super_CPropIndicatorPanel).m_hIndicatorPanel.super_CBaseHandle.m_Index;
        if ((uVar2 == 0xffffffff) ||
           (iVar3 = (uVar2 & 0xffff) * 0x10 + *piVar1, *(uint *)(iVar3 + 8) != uVar2 >> 0x10)) {
          pCVar5 = (CLabIndicatorPanel *)0x0;
        }
        else {
          pCVar5 = *(CLabIndicatorPanel **)(iVar3 + 4);
        }
        CLabIndicatorPanel::SetPlayerPinged(pCVar5,local_5c);
        uVar2 = (this->super_CPropIndicatorPanel).m_hIndicatorPanel.super_CBaseHandle.m_Index;
        if ((uVar2 == 0xffffffff) ||
           (iVar3 = (uVar2 & 0xffff) * 0x10 + *piVar1, *(uint *)(iVar3 + 8) != uVar2 >> 0x10)) {
          pCVar5 = (CLabIndicatorPanel *)0x0;
        }
        else {
          pCVar5 = *(CLabIndicatorPanel **)(iVar3 + 4);
        }
        CLabIndicatorPanel::ScreenVisible(pCVar5,true);
        if (local_5c != 1) {
          COutputEvent::FireOutput
                    (&this->m_OnPlayer2Pinged,(CBaseEntity *)0x0,(CBaseEntity *)0x0,0.0);
          return;
        }
        COutputEvent::FireOutput(&this->m_OnPlayer1Pinged,(CBaseEntity *)0x0,(CBaseEntity *)0x0,0.0)
        ;
      }
    }
  }
  return;
}


/* _GLOBAL__I__ZN18CLabIndicatorPanel9m_DataMapE at 000aa370 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN18CLabIndicatorPanel9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

