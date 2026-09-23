/* DWARF-guided pseudocode for game/server/portal2/mp_lobby_screen.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* CEntityFactory<CMP_LobbyScreen>::Create at 00720210 */

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
       (_func_int_varargs **)(unaff_EBX + 0x5aaba4);
  this_00[1].m_iObjectCapsCache.m_Value = 0;
  this_00[1].m_pfnMoveDone.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)0x0;
  this_00[1].m_pfnMoveDone.__delta = 0;
  this_00[1].m_pfnThink.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)0x0;
  this_00[1].m_pfnThink.__delta = 0;
  (**(code **)(unaff_EBX + 0x5aac18))(this_00,pClassName);
  return &(this_00->m_Network).super_IServerNetworkable;
}


/* DataMapInit<CMP_LobbyScreen> at 000abe70 */

datamap_t * DataMapInit<CMP_LobbyScreen>(CMP_LobbyScreen *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((&UNK_00002eff)[(int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xda0adf);
    if (iVar1 != 0) {
      *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2f0].m_pEntity + unaff_EBX + 3)
           = unaff_EBX + 0xa05354;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2f0].m_pPrev + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2f0].m_pNext + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2f1].m_pEntity + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2f1].m_SerialNumber + unaff_EBX + 3) =
           0;
      *(undefined4 *)
       (&UNK_00002f1f + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2f0].m_SerialNumber + unaff_EBX + 3) =
           0xf;
      ___cxa_guard_release(unaff_EBX + 0xda0adf);
      ___cxa_atexit(unaff_EBX + 0x99157f,0,*(undefined4 *)(&DAT_00b9acd3 + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_UpdatePhysicsShadowToCurrentPosition_00cb88c8 + unaff_EBX + 3) =
       *(undefined4 *)(&DAT_00b9ae0b + unaff_EBX);
  *(undefined4 *)((int)&PTR_VPhysicsCollision_00cb88c0 + unaff_EBX + 3) = 3;
  *(int *)((int)&PTR_VPhysicsShadowUpdate_00cb88bc + unaff_EBX + 3) = unaff_EBX + 0xcb891f;
  return (datamap_t *)((int)&PTR_VPhysicsShadowUpdate_00cb88bc + unaff_EBX + 3);
}


/* __static_initialization_and_destruction_0 at 000abf40 */

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
    *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2cc].m_pPrev + unaff_EBX) =
         0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2cc].m_pPrev + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2cc].m_pPrev + unaff_EBX + 2) = 0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2cc].m_pPrev + unaff_EBX + 3) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2cc].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2cd].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2cd].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2cd].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2cd].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2ce].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2ce].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2ce].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2ce].m_pNext + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2cf].m_pEntity + unaff_EBX)
         = 0x7f7fffff;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2cf].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2cf].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2cf].m_pNext + unaff_EBX) =
         &UNK_00ba03b4 + unaff_EBX;
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2d0].m_pEntity + unaff_EBX)
         = &UNK_00c1f1bc + unaff_EBX;
    pIVar4 = EntityFactoryDictionary();
    (**pIVar4->_vptr_IEntityFactoryDictionary)
              (pIVar4,unaff_EBX + 0xda08e4,&UNK_00a05291 + unaff_EBX);
    pdVar5 = DataMapInit<CMP_LobbyScreen>((CMP_LobbyScreen *)0x0);
    *(datamap_t **)((int)DataMapInit<CAI_Motor>::dataDesc[7].flatOffset + unaff_EBX + -0x30) =
         pdVar5;
    iVar7 = unaff_EBX + 0xda08e8;
    s2 = &UNK_00a05281 + unaff_EBX;
    *(char **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2d0].m_SerialNumber + unaff_EBX)
         = s2;
    this = (SendTable *)((int)DataMapInit<CAI_Motor>::dataDesc[7].flatOffset + unaff_EBX + -0x2c);
    *(SendTable **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2d0].m_pPrev + unaff_EBX) =
         this;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2d1].m_SerialNumber + unaff_EBX) = 0xffff
    ;
    piVar1 = *(int **)(&DAT_00b9aca0 + unaff_EBX);
    puVar2 = (undefined4 *)*piVar1;
    if (puVar2 == (undefined4 *)0x0) {
      *piVar1 = iVar7;
      *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2d0].m_pNext + unaff_EBX)
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
         ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2d0].m_pNext + unaff_EBX) = puVar3;
        puVar2[2] = iVar7;
      }
      else {
        *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2d0].m_pNext + unaff_EBX) =
             *piVar1;
        *piVar1 = iVar7;
      }
    }
    SendTable::SendTable(this);
    ___cxa_atexit(unaff_EBX + 0x99148c,0,*(undefined4 *)(&DAT_00b9ac00 + unaff_EBX));
    iVar7 = ServerClassInit<DT_MP_LobbyScreen::ignored>((ignored *)0x0);
    *(int *)((int)DataMapInit<CAI_Motor>::dataDesc[7].flatOffset + unaff_EBX + -0x18) = iVar7;
  }
  return;
}


/* CMP_LobbyScreen::GetDataDescMap at 0071eb80 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CMP_LobbyScreen * this) */

datamap_t * __thiscall CMP_LobbyScreen::GetDataDescMap(CMP_LobbyScreen *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x645bb8);
}


/* CMP_LobbyScreen::GetBaseMap at 0071eb90 */

datamap_t * CMP_LobbyScreen::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x5280f4);
}


/* __tcf_0 at 00a3d400 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x40f56e)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x40f562) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x40f56e));
  }
  *(undefined4 *)(unaff_EBX + 0x40f56e) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40f56a)) {
    if (*(int *)(unaff_EBX + 0x40f562) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20974e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20974e),*(int *)(unaff_EBX + 0x40f562));
      *(undefined4 *)(unaff_EBX + 0x40f562) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f566) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40f562);
  *(int *)(unaff_EBX + 0x40f572) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40f56a)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20974e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20974e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40f562) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f566) = 0;
  }
  return;
}


/* ServerClassInit<DT_MP_LobbyScreen::ignored> at 000abcb0 */

int ServerClassInit<DT_MP_LobbyScreen::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2fa].m_pPrev + unaff_EBX) ==
      '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xda0b8c);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2fb].m_pPrev + unaff_EBX),
                  (char *)(unaff_EBX + 0x9a9f70),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x300].m_pNext +
                        unaff_EBX),&UNK_009ac320 + unaff_EBX,0,
                        (SendTable *)**(undefined4 **)(&DAT_00b9af88 + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00b9af2c + unaff_EBX),0x80);
      SendPropBool((SendProp_conflict *)
                   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x306].m_pEntity + unaff_EBX)
                   ,(char *)(unaff_EBX + 0xa03844),0x3c0,1);
      ___cxa_guard_release(unaff_EBX + 0xda0b8c);
      ___cxa_atexit(unaff_EBX + 0x9916cc,0,*(undefined4 *)(&DAT_00b9ae90 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)
             ((int)DataMapInit<CAI_MoveAndShootOverlay>::dataDesc[4].flatOffset + unaff_EBX + -0x1c)
             ,(SendProp_conflict *)
              ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x300].m_pNext + unaff_EBX),2,
             *(char **)(&DAT_00c1f424 + unaff_EBX));
  return 1;
}


/* CMP_LobbyScreen::GetServerClass at 0071eba0 */

/* DWARF original prototype: ServerClass * GetServerClass(CMP_LobbyScreen * this) */

ServerClass * __thiscall CMP_LobbyScreen::GetServerClass(CMP_LobbyScreen *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x72dc94);
}


/* CMP_LobbyScreen::YouForgotToImplementOrDeclareServerClass at 0071ebb0 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CMP_LobbyScreen * this) */

int __thiscall CMP_LobbyScreen::YouForgotToImplementOrDeclareServerClass(CMP_LobbyScreen *this)

{
  return 0;
}


/* __tcf_2 at 00a3d3e0 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(&UNK_0038b354 + extraout_ECX),in_stack_00000008);
  return;
}


/* __tcf_1 at 00a3d390 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f567))(unaff_EBX + 0x40f567);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f513))(unaff_EBX + 0x40f513);
                    /* WARNING: Could not recover jumptable at 0x00a3d3d7. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f4bf))();
  return;
}


/* CMP_LobbyScreen::~CMP_LobbyScreen at 0071fd40 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5ab07a);
  iVar1 = (this->m_hScreens).m_Size;
  iVar3 = iVar1 * 4;
  while (iVar1 = iVar1 + -1, -1 < iVar1) {
    uVar2 = *(uint *)((int)&(this->m_hScreens).m_Memory.m_pMemory[-1].super_CBaseHandle.m_Index +
                     iVar3);
    if ((uVar2 == 0xffffffff) ||
       (iVar4 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x526e1e),
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
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x526e0e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x526e0e),pCVar5);
      (this->m_hScreens).m_Memory.m_pMemory = (CHandle<CVGuiScreen> *)0x0;
    }
    pCVar5 = (CHandle<CVGuiScreen> *)0x0;
    (this->m_hScreens).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_hScreens).m_pElements = pCVar5;
  if (-1 < (this->m_hScreens).m_Memory.m_nGrowSize) {
    if (pCVar5 != (CHandle<CVGuiScreen> *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x526e0e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x526e0e),pCVar5);
      (this->m_hScreens).m_Memory.m_pMemory = (CHandle<CVGuiScreen> *)0x0;
    }
    (this->m_hScreens).m_Memory.m_nAllocationCount = 0;
  }
  CBaseEntity::~CBaseEntity(&this->super_CBaseEntity,__in_chrg);
  return;
}


/* CMP_LobbyScreen::~CMP_LobbyScreen at 0071ff50 */

/* DWARF original prototype: void ~CMP_LobbyScreen(CMP_LobbyScreen * this, int __in_chrg,
   CMP_LobbyScreen * this) */

void __thiscall
CMP_LobbyScreen::~CMP_LobbyScreen(CMP_LobbyScreen *this,int __in_chrg,CMP_LobbyScreen *this_1)

{
  ~CMP_LobbyScreen(this,__in_chrg);
  return;
}


/* CMP_LobbyScreen::~CMP_LobbyScreen at 0071ff60 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5aae5a);
  iVar1 = (this->m_hScreens).m_Size;
  iVar3 = iVar1 * 4;
  while (iVar1 = iVar1 + -1, -1 < iVar1) {
    uVar2 = *(uint *)((int)&(this->m_hScreens).m_Memory.m_pMemory[-1].super_CBaseHandle.m_Index +
                     iVar3);
    if ((uVar2 == 0xffffffff) ||
       (iVar4 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x526bfe),
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
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x526bee) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x526bee));
      pCVar5 = (CHandle<CVGuiScreen> *)0x0;
      (this->m_hScreens).m_Memory.m_pMemory = (CHandle<CVGuiScreen> *)0x0;
    }
    (this->m_hScreens).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_hScreens).m_pElements = pCVar5;
  if (-1 < (this->m_hScreens).m_Memory.m_nGrowSize) {
    if (pCVar5 != (CHandle<CVGuiScreen> *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x526bee) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x526bee));
      (this->m_hScreens).m_Memory.m_pMemory = (CHandle<CVGuiScreen> *)0x0;
      in_stack_ffffffc8 = pCVar5;
    }
    (this->m_hScreens).m_Memory.m_nAllocationCount = 0;
  }
  CBaseEntity::~CBaseEntity(&this->super_CBaseEntity,(int)in_stack_ffffffc8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CMP_LobbyScreen::KeyValue at 0071ed90 */

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
  if ((code *)szKeyName != CAI_FollowManager::CalcFollowPosition + unaff_EBX + 5) {
    iVar3 = _V_stricmp(szKeyName,(char *)(CAI_FollowManager::CalcFollowPosition + unaff_EBX + 5));
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


/* CMP_LobbyScreen::UpdateTransmitState at 0071ed70 */

/* DWARF original prototype: int UpdateTransmitState(CMP_LobbyScreen * this) */

int __thiscall CMP_LobbyScreen::UpdateTransmitState(CMP_LobbyScreen *this)

{
  int iVar1;
  
  iVar1 = CBaseEntity::SetTransmitState(&this->super_CBaseEntity,0);
  return iVar1;
}


/* CMP_LobbyScreen::SetTransmit at 0071eee0 */

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
    uVar3 = (int)peVar1 - *(int *)(**(int **)(unaff_EBX + 0x527c76) + 0x58) >> 4;
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
           (iVar2 = (uVar3 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x527c7e),
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


/* CMP_LobbyScreen::Spawn at 0071f5d0 */

/* DWARF original prototype: void Spawn(CMP_LobbyScreen * this) */

void __thiscall CMP_LobbyScreen::Spawn(CMP_LobbyScreen *this)

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
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x527584);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_0071f764:
            puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x3c0;
            puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != 0x3c0) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_0071f764;
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
          *(short *)(**(int **)(unaff_EBX + 0x527584) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x527584) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x527584);
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
  return;
}


/* CMP_LobbyScreen::Precache at 0071ed50 */

/* DWARF original prototype: void Precache(CMP_LobbyScreen * this) */

void __thiscall CMP_LobbyScreen::Precache(CMP_LobbyScreen *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  PrecacheVGuiScreen((char *)(extraout_ECX + 0x39246d));
  return;
}


/* CMP_LobbyScreen::OnRestore at 0071f590 */

/* DWARF original prototype: void OnRestore(CMP_LobbyScreen * this) */

void __thiscall CMP_LobbyScreen::OnRestore(CMP_LobbyScreen *this)

{
  CBaseEntity::OnRestore(&this->super_CBaseEntity);
  RestoreControlPanels(this);
  ScreenVisible(this,(this->m_bEnabled).m_Value);
  return;
}


/* CMP_LobbyScreen::ScreenVisible at 0071f160 */

/* DWARF original prototype: void ScreenVisible(CMP_LobbyScreen * this, bool bVisible) */

void __thiscall CMP_LobbyScreen::ScreenVisible(CMP_LobbyScreen *this,bool bVisible)

{
  undefined1 *puVar1;
  ushort uVar2;
  edict_t *peVar3;
  CBaseEdict *this_00;
  ushort *puVar4;
  int *piVar5;
  int iVar6;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  uint uVar9;
  int unaff_EBX;
  CBaseEntity *this_01;
  uint uVar10;
  ushort i;
  int local_34;
  
  ___i686_get_pc_thunk_bx();
  if (0 < (this->m_hScreens).m_Size) {
    local_34 = 0;
    do {
      uVar8 = (this->m_hScreens).m_Memory.m_pMemory[local_34].super_CBaseHandle.m_Index;
      if ((uVar8 == 0xffffffff) ||
         (iVar6 = (uVar8 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5279fe),
         *(uint *)(iVar6 + 8) != uVar8 >> 0x10)) {
        this_01 = (CBaseEntity *)0x0;
        if (bVisible) goto LAB_0071f1c4;
LAB_0071f240:
        CBaseEntity::AddEffects(this_01,0x20);
      }
      else {
        this_01 = *(CBaseEntity **)(iVar6 + 4);
        if (!bVisible) goto LAB_0071f240;
LAB_0071f1c4:
        uVar8 = (this_01->m_fEffects).m_Value;
        uVar10 = uVar8 & 0xffffffdf;
        if (uVar8 != uVar10) {
          if ((this_01->m_Network).m_TimerEvent.m_bRegistered == false) {
            this_00 = &((this_01->m_Network).m_pPev)->super_CBaseEdict;
            if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
              this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
              pIVar7 = CBaseEdict::GetChangeAccessor(this_00);
              puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x5279fa);
              if (pIVar7->m_iChangeInfoSerialNumber == *puVar4) {
                uVar8 = (uint)pIVar7->m_iChangeInfo;
                uVar2 = puVar4[uVar8 * 0x14 + 0x14];
                if (uVar2 == 0) {
LAB_0071f374:
                  puVar4[(uint)uVar2 + uVar8 * 0x14 + 1] = 0xc0;
                  puVar4[uVar8 * 0x14 + 0x14] = uVar2 + 1;
                }
                else if (puVar4[uVar8 * 0x14 + 1] != 0xc0) {
                  _i = 0;
                  do {
                    uVar9 = _i + 1;
                    _i = uVar9 & 0xffff;
                    if ((ushort)uVar9 == uVar2) {
                      if (uVar2 != 0x13) goto LAB_0071f374;
                      pIVar7->m_iChangeInfoSerialNumber = 0;
                      this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                      break;
                    }
                  } while (puVar4[uVar8 * 0x14 + _i + 1] != 0xc0);
                }
              }
              else if (puVar4[0x7d1] == 100) {
                pIVar7->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
              }
              else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
                pIVar7->m_iChangeInfo = puVar4[0x7d1];
                *(short *)(**(int **)(unaff_EBX + 0x5279fa) + 0xfa2) =
                     *(short *)(**(int **)(unaff_EBX + 0x5279fa) + 0xfa2) + 1;
                piVar5 = *(int **)(unaff_EBX + 0x5279fa);
                pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
                iVar6 = *piVar5 + (uint)pIVar7->m_iChangeInfo * 0x28;
                *(undefined2 *)(iVar6 + 2) = 0xc0;
                *(undefined2 *)(iVar6 + 0x28) = 1;
              }
            }
          }
          else {
            puVar1 = &(this_01->m_Network).field_0x4c;
            *(uint *)puVar1 = *(uint *)puVar1 | 1;
          }
          (this_01->m_fEffects).m_Value = uVar10;
        }
        peVar3 = (this_01->m_Network).m_pPev;
        if (peVar3 != (edict_t *)0x0) {
          (peVar3->super_CBaseEdict).m_fStateFlags = (peVar3->super_CBaseEdict).m_fStateFlags | 0x80
          ;
        }
        CBaseEntity::DispatchUpdateTransmitState(this_01);
      }
      local_34 = local_34 + 1;
    } while (local_34 < (this->m_hScreens).m_Size);
  }
  return;
}


/* CMP_LobbyScreen::Disable at 0071f3d0 */

/* DWARF original prototype: void Disable(CMP_LobbyScreen * this) */

void __thiscall CMP_LobbyScreen::Disable(CMP_LobbyScreen *this)

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
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x527784);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_0071f554:
            puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x3c0;
            puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != 0x3c0) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_0071f554;
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
          *(short *)(**(int **)(unaff_EBX + 0x527784) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x527784) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x527784);
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


/* CMP_LobbyScreen::InputDisable at 0071f7c0 */

/* DWARF original prototype: void InputDisable(CMP_LobbyScreen * this, inputdata_t * inputdata) */

void __thiscall CMP_LobbyScreen::InputDisable(CMP_LobbyScreen *this,inputdata_t *inputdata)

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
      puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x527394);
      if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
        uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar3 = puVar4[uVar7 * 0x14 + 0x14];
        if (uVar3 == 0) {
LAB_0071f944:
          puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x3c0;
          puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
        }
        else if (puVar4[uVar7 * 0x14 + 1] != 0x3c0) {
          local_60 = 0;
          do {
            uVar8 = local_60 + 1;
            local_60 = uVar8 & 0xffff;
            if ((ushort)uVar8 == uVar3) {
              if (uVar3 != 0x13) goto LAB_0071f944;
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
        *(short *)(**(int **)(unaff_EBX + 0x527394) + 0xfa2) =
             *(short *)(**(int **)(unaff_EBX + 0x527394) + 0xfa2) + 1;
        piVar5 = *(int **)(unaff_EBX + 0x527394);
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


/* CMP_LobbyScreen::Enable at 0071f9a0 */

/* DWARF original prototype: void Enable(CMP_LobbyScreen * this) */

void __thiscall CMP_LobbyScreen::Enable(CMP_LobbyScreen *this)

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
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x5271b4);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_0071fb24:
            puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x3c0;
            puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != 0x3c0) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_0071fb24;
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
          *(short *)(**(int **)(unaff_EBX + 0x5271b4) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x5271b4) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x5271b4);
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


/* CMP_LobbyScreen::InputEnable at 0071fb60 */

/* DWARF original prototype: void InputEnable(CMP_LobbyScreen * this, inputdata_t * inputdata) */

void __thiscall CMP_LobbyScreen::InputEnable(CMP_LobbyScreen *this,inputdata_t *inputdata)

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
      puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x526ff4);
      if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
        uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar3 = puVar4[uVar7 * 0x14 + 0x14];
        if (uVar3 == 0) {
LAB_0071fce4:
          puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x3c0;
          puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
        }
        else if (puVar4[uVar7 * 0x14 + 1] != 0x3c0) {
          local_60 = 0;
          do {
            uVar8 = local_60 + 1;
            local_60 = uVar8 & 0xffff;
            if ((ushort)uVar8 == uVar3) {
              if (uVar3 != 0x13) goto LAB_0071fce4;
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
        *(short *)(**(int **)(unaff_EBX + 0x526ff4) + 0xfa2) =
             *(short *)(**(int **)(unaff_EBX + 0x526ff4) + 0xfa2) + 1;
        piVar5 = *(int **)(unaff_EBX + 0x526ff4);
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


/* CMP_LobbyScreen::GetControlPanelInfo at 0071ebc0 */

/* DWARF original prototype: void GetControlPanelInfo(CMP_LobbyScreen * this, int nPanelIndex, char
   * * pPanelName) */

void __thiscall
CMP_LobbyScreen::GetControlPanelInfo(CMP_LobbyScreen *this,int nPanelIndex,char **pPanelName)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *pPanelName = (char *)(extraout_ECX + 0x3925fd);
  return;
}


/* CMP_LobbyScreen::GetControlPanelClassName at 0071ebe0 */

/* DWARF original prototype: void GetControlPanelClassName(CMP_LobbyScreen * this, int nPanelIndex,
   char * * pPanelName) */

void __thiscall
CMP_LobbyScreen::GetControlPanelClassName(CMP_LobbyScreen *this,int nPanelIndex,char **pPanelName)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *pPanelName = (char *)(extraout_ECX + 0x390914);
  return;
}


/* CMP_LobbyScreen::SpawnControlPanels at 0071ec00 */

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
  this_00 = CreateVGuiScreen((char *)(unaff_EBX + 0x3908ee),(char *)(unaff_EBX + 0x3925b7),
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


/* CMP_LobbyScreen::RestoreControlPanels at 0071efd0 */

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
                      (*(CGlobalEntityList **)(unaff_EBX + 0x527c46),(CBaseEntity *)0x0,
                       (char *)(unaff_EBX + 0x39051e));
  do {
    if (this_00 == (CVGuiScreen *)0x0) {
LAB_0071f06c:
      s1 = CVGuiScreen::GetPanelName(this_00);
      iVar3 = _V_strcmp(s1,(char *)(unaff_EBX + 0x3921e7));
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
         (iVar3 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x527b8e),
         *(uint *)(iVar3 + 8) != uVar2 >> 0x10)) {
        pCVar4 = (CMP_LobbyScreen *)0x0;
      }
      else {
        pCVar4 = *(CMP_LobbyScreen **)(iVar3 + 4);
      }
      if (pCVar4 == this) goto LAB_0071f06c;
    }
    this_00 = (CVGuiScreen *)
              CGlobalEntityList::FindEntityByClassname
                        (*(CGlobalEntityList **)(unaff_EBX + 0x527c46),&this_00->super_CBaseEntity,
                         (char *)(unaff_EBX + 0x39051e));
  } while( true );
}


/* _GLOBAL__I__ZN15CMP_LobbyScreen9m_DataMapE at 000ac130 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN15CMP_LobbyScreen9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

