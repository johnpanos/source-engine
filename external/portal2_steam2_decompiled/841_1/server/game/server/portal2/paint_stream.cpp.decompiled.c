/* DWARF-guided pseudocode for game/server/portal2/paint_stream.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000aea70 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  int *piVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  int iVar4;
  int iVar5;
  datamap_t *pdVar6;
  IEntityFactoryDictionary *pIVar7;
  int unaff_EBX;
  longlong lVar8;
  ConVar *in_stack_ffffffc4;
  ConVar *in_stack_ffffffc8;
  char *in_stack_ffffffcc;
  char *in_stack_ffffffd0;
  int in_stack_ffffffd4;
  char *s2;
  char *in_stack_ffffffd8;
  SendTable *this;
  
  lVar8 = ___i686_get_pc_thunk_bx();
  if (lVar8 == 0xffff00000001) {
    *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[399].m_pPrev + unaff_EBX) = 0
    ;
    *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[399].m_pPrev + unaff_EBX + 1)
         = 0;
    *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[399].m_pPrev + unaff_EBX + 2)
         = 0;
    *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[399].m_pPrev + unaff_EBX + 3)
         = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[399].m_pNext + unaff_EBX) = 0
    ;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[400].m_pEntity + unaff_EBX) =
         0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[400].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[400].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[400].m_pNext + unaff_EBX) = 0
    ;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x191].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x191].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x191].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x191].m_pNext + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x192].m_pEntity + unaff_EBX)
         = 0x7f7fffff;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x192].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x192].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x192].m_pNext + unaff_EBX) =
         &UNK_00b9d884 + unaff_EBX;
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<ResponseRules::CRR_Response>::dataDesc[0].flatOffset +
                   unaff_EBX + -0x18),(char *)(unaff_EBX + 0xa03cfa),(char *)(unaff_EBX + 0x9a037c),
                   2,in_stack_ffffffc4,(char *)in_stack_ffffffc8,in_stack_ffffffcc,
                   (int)in_stack_ffffffd0);
    ___cxa_atexit(unaff_EBX + 0x98f40c,0,*(undefined4 *)(&DAT_00b980d0 + unaff_EBX));
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<ResponseRules::CRR_Response>::dataDesc[1].flatOffset +
                   unaff_EBX + 8),(char *)(unaff_EBX + 0xa03d68),(char *)(unaff_EBX + 0xa03d60),2,
                   (char *)(unaff_EBX + 0xa03d14),in_stack_ffffffc8,in_stack_ffffffcc,
                   in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
    ___cxa_atexit(unaff_EBX + 0x98f3ec,0,*(undefined4 *)(&DAT_00b980d0 + unaff_EBX));
    iVar5 = unaff_EBX + 0xd9f514;
    s2 = (char *)(unaff_EBX + 0xa03ceb);
    *(char **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x193].m_pEntity + unaff_EBX) = s2
    ;
    this = (SendTable *)
           ((int)DataMapInit<ResponseRules::CRR_Response>::dataDesc[3].flatOffset +
           unaff_EBX + -0x20);
    *(SendTable **)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x193].m_SerialNumber + unaff_EBX) = this;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x194].m_pEntity + unaff_EBX)
         = 0xffff;
    piVar1 = *(int **)(&DAT_00b98170 + unaff_EBX);
    puVar2 = (undefined4 *)*piVar1;
    if (puVar2 == (undefined4 *)0x0) {
      *piVar1 = iVar5;
      *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x193].m_pPrev + unaff_EBX)
           = 0;
    }
    else {
      puVar3 = (undefined4 *)puVar2[2];
      iVar4 = _V_stricmp((char *)*puVar2,s2);
      if (iVar4 < 1) {
        while ((puVar3 != (undefined4 *)0x0 && (iVar4 = _V_stricmp((char *)*puVar3,s2), iVar4 < 1)))
        {
          puVar2 = puVar3;
          puVar3 = (undefined4 *)puVar3[2];
        }
        *(undefined4 **)
         ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x193].m_pPrev + unaff_EBX) = puVar3;
        puVar2[2] = iVar5;
      }
      else {
        *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x193].m_pPrev + unaff_EBX) =
             *piVar1;
        *piVar1 = iVar5;
      }
    }
    SendTable::SendTable(this);
    ___cxa_atexit(unaff_EBX + 0x98f3cc,0,*(undefined4 *)(&DAT_00b980d0 + unaff_EBX));
    iVar5 = ServerClassInit<DT_PaintStream::ignored>((ignored *)0x0);
    *(int *)((int)DataMapInit<ResponseRules::CRR_Response>::dataDesc[3].flatOffset +
            unaff_EBX + -0xc) = iVar5;
    pdVar6 = DataMapInit<CPaintStream>((CPaintStream *)0x0);
    *(datamap_t **)
     ((int)DataMapInit<ResponseRules::CRR_Response>::dataDesc[3].flatOffset + unaff_EBX + -8) =
         pdVar6;
    *(undefined **)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x194].m_SerialNumber + unaff_EBX) =
         &UNK_00c1f66c + unaff_EBX;
    pIVar7 = EntityFactoryDictionary();
    (**pIVar7->_vptr_IEntityFactoryDictionary)(pIVar7,unaff_EBX + 0xd9f528,unaff_EBX + 0xa07bc6);
  }
  return;
}


/* __tcf_2 at 00a3de90 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38b288),in_stack_00000008);
  return;
}


/* __tcf_3 at 00a3de70 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38b308),in_stack_00000008);
  return;
}


/* ServerClassInit<DT_PaintStream::ignored> at 000ae770 */

int ServerClassInit<DT_PaintStream::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x1c7].m_pEntity + unaff_EBX) ==
      '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xd9f854);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x1c7].m_pPrev + unaff_EBX),
                  (char *)(unaff_EBX + 0x9a74b0),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x1cc].m_pNext +
                        unaff_EBX),(char *)(unaff_EBX + 0x9a9860),0,
                        (SendTable *)**(undefined4 **)(&DAT_00b98460 + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00b9846c + unaff_EBX),0x80);
      SendPropInt((SendProp_conflict *)
                  ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x1d2].m_pEntity + unaff_EBX),
                  (char *)(unaff_EBX + 0xa03fdc),0x564,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropInt((SendProp_conflict *)
                  ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x1d7].m_SerialNumber +
                  unaff_EBX),&UNK_009d4208 + unaff_EBX,0x568,4,-1,0,(SendVarProxyFn_conflict)0x0,
                  0x80);
      ___cxa_guard_release(unaff_EBX + 0xd9f854);
      ___cxa_atexit(unaff_EBX + 0x98f66c,0,*(undefined4 *)(&DAT_00b983d0 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)((int)&g_pszSCHED_FAIL_ESTABLISH_LINE_OF_FIRE + unaff_EBX),
             (SendProp_conflict *)
             ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x1cc].m_pNext + unaff_EBX),3,
             *(char **)(&DAT_00c1f954 + unaff_EBX));
  return 1;
}


/* CPaintStream::GetServerClass at 0073a070 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: ServerClass * GetServerClass(CPaintStream * this) */

ServerClass * __thiscall CPaintStream::GetServerClass(CPaintStream *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x713f20);
}


/* CPaintStream::YouForgotToImplementOrDeclareServerClass at 0073a080 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CPaintStream * this) */

int __thiscall CPaintStream::YouForgotToImplementOrDeclareServerClass(CPaintStream *this)

{
  return 0;
}


/* __tcf_4 at 00a3de50 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(&UNK_0038b380 + extraout_ECX),in_stack_00000008);
  return;
}


/* __tcf_0 at 00a3ddf0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x4102db))(unaff_EBX + 0x4102db);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410287))(unaff_EBX + 0x410287);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410233))(unaff_EBX + 0x410233);
                    /* WARNING: Could not recover jumptable at 0x00a3de48. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x4101df))();
  return;
}


/* DataMapInit<CPaintStream> at 000ae9a0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

datamap_t * DataMapInit<CPaintStream>(CPaintStream *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x1a1].m_pPrev + unaff_EBX + 3)
      == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xd9f5ff);
    if (iVar1 != 0) {
      *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x1a2].m_pEntity + unaff_EBX + 3)
           = unaff_EBX + 0xa03dbe;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x1a2].m_pPrev + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x1a2].m_pNext + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x1a3].m_pEntity + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x1a3].m_SerialNumber + unaff_EBX + 3) =
           0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x1a3].m_pPrev + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x1a2].m_SerialNumber + unaff_EBX + 3) =
           0xc;
      ___cxa_guard_release(unaff_EBX + 0xd9f5ff);
      ___cxa_atexit(unaff_EBX + 0x98f4ff,0,*(undefined4 *)(&DAT_00b981a3 + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_PostConstructor_00cb757c + unaff_EBX + 3) =
       *(undefined4 *)(&DAT_00b981af + unaff_EBX);
  *(undefined4 *)((int)&PTR_SetModel_00cb7574 + unaff_EBX + 3) = 2;
  *(int *)((int)&PTR_Precache_00cb7570 + unaff_EBX + 3) = unaff_EBX + 0xcb75cf;
  return (datamap_t *)((int)&PTR_Precache_00cb7570 + unaff_EBX + 3);
}


/* CPaintStream::GetDataDescMap at 0073a090 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: datamap_t * GetDataDescMap(CPaintStream * this) */

datamap_t * __thiscall CPaintStream::GetDataDescMap(CPaintStream *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x62be8c);
}


/* CPaintStream::GetBaseMap at 0073a0a0 */

datamap_t * CPaintStream::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x50cab8);
}


/* __tcf_1 at 00a3deb0 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41010e)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x410102) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41010e));
  }
  *(undefined4 *)(unaff_EBX + 0x41010e) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41010a)) {
    if (*(int *)(unaff_EBX + 0x410102) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x208c9e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x208c9e),*(int *)(unaff_EBX + 0x410102));
      *(undefined4 *)(unaff_EBX + 0x410102) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x410106) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x410102);
  *(int *)(unaff_EBX + 0x410112) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41010a)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x208c9e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x208c9e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x410102) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x410106) = 0;
  }
  return;
}


/* CPaintStream::CPaintStream at 0073a630 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void CPaintStream(CPaintStream * this) */

void __thiscall CPaintStream::CPaintStream(CPaintStream *this)

{
  int iVar1;
  undefined1 *puVar2;
  uint uVar3;
  ushort uVar4;
  int iVar5;
  int *piVar6;
  CBaseEdict *this_00;
  ushort *puVar7;
  int iVar8;
  undefined4 *puVar9;
  IChangeInfoAccessor *pIVar10;
  int iVar11;
  uint uVar12;
  int unaff_EBX;
  uint uVar13;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::CBaseAnimating(&this->super_CBaseAnimating);
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5936ea);
  (this->m_blobs).m_Memory.m_pMemory = (CBasePaintBlob **)0x0;
  (this->m_blobs).m_Memory.m_nAllocationCount = 0;
  (this->m_blobs).m_Memory.m_nGrowSize = 0;
  (this->m_blobs).m_Size = 0;
  (this->m_blobs).m_pElements = (CBasePaintBlob **)0x0;
  (this->m_sharedBlobData).
  super_CSharedVarBase<CUtlVector<BlobDataPerUpdate_t,_CUtlMemory<BlobDataPerUpdate_t,_int>_>,CPaintStream::SharedVar_m_sharedBlobData>
  .m_pSharedMemory = (ISPSharedMemory *)0x0;
  (this->m_sharedBlobData).
  super_CSharedVarBase<CUtlVector<BlobDataPerUpdate_t,_CUtlMemory<BlobDataPerUpdate_t,_int>_>,CPaintStream::SharedVar_m_sharedBlobData>
  .m_pValue = (CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t,_int>_> *)0x0;
  (this->m_sharedBlobData).
  super_CSharedVarBase<CUtlVector<BlobDataPerUpdate_t,_CUtlMemory<BlobDataPerUpdate_t,_int>_>,CPaintStream::SharedVar_m_sharedBlobData>
  .super_ISharedVarBase._vptr_ISharedVarBase = (_func_int_varargs **)(unaff_EBX + 0x593ad2);
  iVar11 = *(int *)(unaff_EBX + 0x50c776);
  iVar5 = *(int *)(iVar11 + 0xc);
  iVar1 = iVar5 + 1;
  iVar8 = *(int *)(iVar11 + 4);
  if (iVar8 < iVar1) {
    iVar11 = *(int *)(iVar11 + 8);
    if (-1 < iVar11) {
      if (iVar11 == 0) {
        if ((iVar8 == 0) && (iVar8 = 8, iVar1 < 9)) {
          local_20 = 0x20;
        }
        else {
          do {
            local_20 = iVar8;
            iVar8 = local_20 * 2;
          } while (iVar8 < iVar1);
          local_20 = local_20 * 8;
        }
        goto LAB_0073a800;
      }
      iVar8 = (iVar5 / iVar11 + 1) * iVar11;
      if (iVar8 < iVar1) {
        if (iVar8 != 0) {
LAB_0073a8b0:
          do {
            iVar8 = (iVar8 + iVar1) / 2;
          } while (iVar8 < iVar1);
          goto LAB_0073a8c0;
        }
        if (iVar1 < 0) {
          iVar8 = -1;
          local_20 = -4;
        }
        else {
          local_20 = 0;
          if (iVar1 != 0) goto LAB_0073a8b0;
        }
      }
      else {
LAB_0073a8c0:
        local_20 = iVar8 * 4;
      }
LAB_0073a800:
      piVar6 = *(int **)(unaff_EBX + 0x50c776);
      piVar6[1] = iVar8;
      iVar11 = *piVar6;
      if (iVar11 == 0) {
        iVar11 = (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x50c51e))
                           ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x50c51e),local_20);
      }
      else {
        iVar11 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x50c51e) + 4))
                           ((int *)**(undefined4 **)(unaff_EBX + 0x50c51e),iVar11,local_20);
      }
      piVar6 = *(int **)(unaff_EBX + 0x50c776);
      *piVar6 = iVar11;
      iVar8 = piVar6[3];
      piVar6[3] = iVar8 + 1;
      iVar11 = *piVar6;
      piVar6[4] = iVar11;
      iVar8 = (iVar8 + 1) - iVar5;
      goto joined_r0x0073a6d2;
    }
  }
  piVar6 = *(int **)(unaff_EBX + 0x50c776);
  piVar6[3] = iVar1;
  iVar11 = *piVar6;
  piVar6[4] = iVar11;
  iVar8 = iVar1 - iVar5;
joined_r0x0073a6d2:
  if (0 < iVar8 + -1) {
    _V_memmove((void *)(iVar11 + iVar1 * 4),(void *)(iVar11 + iVar5 * 4),(iVar8 + -1) * 4);
  }
  puVar9 = (undefined4 *)(iVar5 * 4 + **(int **)(unaff_EBX + 0x50c776));
  if (puVar9 != (undefined4 *)0x0) {
    *puVar9 = this;
  }
  this->m_nBlobCounter = 0;
  if ((this->m_nRenderMode).m_Value != 0) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
      ;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(this_00);
        piVar6 = *(int **)(unaff_EBX + 0x50c52a);
        puVar7 = (ushort *)*piVar6;
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar7) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar13 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar4 = puVar7[uVar13 * 0x14 + 0x14];
          if (uVar4 == 0) {
LAB_0073a96a:
            puVar7[(uint)uVar4 + uVar13 * 0x14 + 1] = 0x568;
            puVar7[uVar13 * 0x14 + 0x14] = uVar4 + 1;
          }
          else if (puVar7[uVar13 * 0x14 + 1] != 0x568) {
            uVar12 = 0;
            do {
              uVar3 = uVar12 + 1;
              uVar12 = uVar3 & 0xffff;
              if ((ushort)uVar3 == uVar4) {
                if (uVar4 != 0x13) goto LAB_0073a96a;
                goto LAB_0073a8cf;
              }
            } while (puVar7[uVar13 * 0x14 + uVar12 + 1] != 0x568);
          }
        }
        else if (puVar7[0x7d1] == 100) {
LAB_0073a8cf:
          pIVar10->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar7[0x7d1];
          *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar1 = *piVar6 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar1 + 2) = 0x568;
          *(undefined2 *)(iVar1 + 0x28) = 1;
        }
      }
    }
    else {
      puVar2 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar2 = *(uint *)puVar2 | 1;
    }
    (this->m_nRenderMode).m_Value = 0;
  }
  return;
}


/* CPaintStream::CPaintStream at 0073aae0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void CPaintStream(CPaintStream * this, CPaintStream * this) */

void __thiscall CPaintStream::CPaintStream(CPaintStream *this,CPaintStream *this_1)

{
  CPaintStream(this);
  return;
}


/* CEntityFactory<CPaintStream>::Create at 0073d340 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CPaintStream> * this, char *
   pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CPaintStream>::Create(CEntityFactory<CPaintStream> *this,char *pClassName)

{
  CPaintStream *this_00;
  
                    /* Unresolved local var: CPaintStream * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x56c);
  CPaintStream::CPaintStream(this_00);
  (*(this_00->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1d])(this_00,pClassName);
  return &(this_00->super_CBaseAnimating).super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CPaintStream::~CPaintStream at 0073af70 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ~CPaintStream(CPaintStream * this, int __in_chrg) */

void __thiscall CPaintStream::~CPaintStream(CPaintStream *this,int __in_chrg)

{
  CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t,_int>_> *pCVar1;
  int iVar2;
  undefined4 *puVar3;
  ISPSharedMemory *pIVar4;
  int iVar5;
  BlobDataPerUpdate_t *pBVar6;
  undefined4 uVar7;
  BlobDataPerUpdate_t *pBVar8;
  int unaff_EBX;
  int iVar9;
  int iVar10;
  BlobDataPerUpdate_t *in_stack_ffffff98;
  int local_2c;
  int local_28;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x592daa);
  RemoveAllPaintBlobs(this);
  pCVar1 = (this->m_sharedBlobData).
           super_CSharedVarBase<CUtlVector<BlobDataPerUpdate_t,_CUtlMemory<BlobDataPerUpdate_t,_int>_>,CPaintStream::SharedVar_m_sharedBlobData>
           .m_pValue;
  iVar2 = pCVar1->m_Size;
  puVar3 = *(undefined4 **)(&DAT_0050bbde + unaff_EBX);
  local_20 = iVar2 * 0x18;
  while (iVar5 = local_20, iVar2 = iVar2 + -1, -1 < iVar2) {
    local_20 = iVar5 + -0x18;
    pBVar8 = (pCVar1->m_Memory).m_pMemory;
    local_2c = *(int *)((int)&pBVar8[-1].m_blobData.m_Size + iVar5);
    iVar10 = local_2c * 0x2c;
    while( true ) {
      iVar10 = iVar10 + -0x2c;
      local_2c = local_2c + -1;
      if (local_2c < 0) break;
      iVar9 = iVar10 + *(int *)((int)&pBVar8[-1].m_blobData.m_Memory.m_pMemory + iVar5);
      *(undefined4 *)(iVar9 + 0x24) = 0;
      if (*(int *)(iVar9 + 0x20) < 0) {
        pBVar6 = *(BlobDataPerUpdate_t **)(iVar9 + 0x18);
      }
      else {
        pBVar6 = *(BlobDataPerUpdate_t **)(iVar9 + 0x18);
        if (pBVar6 != (BlobDataPerUpdate_t *)0x0) {
          in_stack_ffffff98 = pBVar6;
          (**(code **)(*(int *)*puVar3 + 8))((int *)*puVar3);
          pBVar6 = (BlobDataPerUpdate_t *)0x0;
          *(undefined4 *)(iVar9 + 0x18) = 0;
        }
        *(undefined4 *)(iVar9 + 0x1c) = 0;
      }
      *(BlobDataPerUpdate_t **)(iVar9 + 0x28) = pBVar6;
      *(undefined4 *)(iVar9 + 0x24) = 0;
      if (-1 < *(int *)(iVar9 + 0x20)) {
        if (pBVar6 != (BlobDataPerUpdate_t *)0x0) {
          in_stack_ffffff98 = pBVar6;
          (**(code **)(*(int *)**(undefined4 **)(&DAT_0050bbde + unaff_EBX) + 8))
                    ((int *)**(undefined4 **)(&DAT_0050bbde + unaff_EBX));
          pBVar6 = (BlobDataPerUpdate_t *)0x0;
          *(undefined4 *)(iVar9 + 0x18) = 0;
        }
        *(undefined4 *)(iVar9 + 0x1c) = 0;
      }
      *(BlobDataPerUpdate_t **)(iVar9 + 0x28) = pBVar6;
      if (-1 < *(int *)(iVar9 + 0x20)) {
        if (pBVar6 != (BlobDataPerUpdate_t *)0x0) {
          (**(code **)(*(int *)*puVar3 + 8))((int *)*puVar3);
          *(undefined4 *)(iVar9 + 0x18) = 0;
          in_stack_ffffff98 = pBVar6;
        }
        *(undefined4 *)(iVar9 + 0x1c) = 0;
      }
    }
    *(undefined4 *)((int)&pBVar8[-1].m_blobData.m_Size + iVar5) = 0;
    if (*(int *)((int)&pBVar8[-1].m_blobData.m_Memory.m_nGrowSize + iVar5) < 0) {
      uVar7 = *(undefined4 *)((int)&pBVar8[-1].m_blobData.m_Memory.m_pMemory + iVar5);
    }
    else {
      pBVar6 = *(BlobDataPerUpdate_t **)((int)&pBVar8[-1].m_blobData.m_Memory.m_pMemory + iVar5);
      if (pBVar6 != (BlobDataPerUpdate_t *)0x0) {
        (**(code **)(*(int *)*puVar3 + 8))((int *)*puVar3);
        *(undefined4 *)((int)&pBVar8[-1].m_blobData.m_Memory.m_pMemory + iVar5) = 0;
        in_stack_ffffff98 = pBVar6;
      }
      uVar7 = 0;
      *(undefined4 *)((int)&pBVar8[-1].m_blobData.m_Memory.m_nAllocationCount + iVar5) = 0;
    }
    *(undefined4 *)((int)&pBVar8[-1].m_blobData.m_pElements + iVar5) = uVar7;
    local_28 = *(int *)((int)&pBVar8[-1].m_blobData.m_Size + iVar5);
    iVar10 = local_28 * 0x2c;
    while( true ) {
      iVar10 = iVar10 + -0x2c;
      local_28 = local_28 + -1;
      if (local_28 < 0) break;
      iVar9 = iVar10 + *(int *)((int)&pBVar8[-1].m_blobData.m_Memory.m_pMemory + iVar5);
      *(undefined4 *)(iVar9 + 0x24) = 0;
      if (*(int *)(iVar9 + 0x20) < 0) {
        pBVar6 = *(BlobDataPerUpdate_t **)(iVar9 + 0x18);
      }
      else {
        pBVar6 = *(BlobDataPerUpdate_t **)(iVar9 + 0x18);
        if (pBVar6 != (BlobDataPerUpdate_t *)0x0) {
          in_stack_ffffff98 = pBVar6;
          (**(code **)(*(int *)*puVar3 + 8))((int *)*puVar3);
          pBVar6 = (BlobDataPerUpdate_t *)0x0;
          *(undefined4 *)(iVar9 + 0x18) = 0;
        }
        *(undefined4 *)(iVar9 + 0x1c) = 0;
      }
      *(BlobDataPerUpdate_t **)(iVar9 + 0x28) = pBVar6;
      *(undefined4 *)(iVar9 + 0x24) = 0;
      if (-1 < *(int *)(iVar9 + 0x20)) {
        if (pBVar6 != (BlobDataPerUpdate_t *)0x0) {
          in_stack_ffffff98 = pBVar6;
          (**(code **)(*(int *)*puVar3 + 8))((int *)*puVar3);
          pBVar6 = (BlobDataPerUpdate_t *)0x0;
          *(undefined4 *)(iVar9 + 0x18) = 0;
        }
        *(undefined4 *)(iVar9 + 0x1c) = 0;
      }
      *(BlobDataPerUpdate_t **)(iVar9 + 0x28) = pBVar6;
      if (-1 < *(int *)(iVar9 + 0x20)) {
        if (pBVar6 != (BlobDataPerUpdate_t *)0x0) {
          (**(code **)(*(int *)*puVar3 + 8))((int *)*puVar3);
          *(undefined4 *)(iVar9 + 0x18) = 0;
          in_stack_ffffff98 = pBVar6;
        }
        *(undefined4 *)(iVar9 + 0x1c) = 0;
      }
    }
    *(undefined4 *)((int)&pBVar8[-1].m_blobData.m_Size + iVar5) = 0;
    if (*(int *)((int)&pBVar8[-1].m_blobData.m_Memory.m_nGrowSize + iVar5) < 0) {
      pBVar6 = *(BlobDataPerUpdate_t **)((int)&pBVar8[-1].m_blobData.m_Memory.m_pMemory + iVar5);
    }
    else {
      pBVar6 = *(BlobDataPerUpdate_t **)((int)&pBVar8[-1].m_blobData.m_Memory.m_pMemory + iVar5);
      if (pBVar6 != (BlobDataPerUpdate_t *)0x0) {
        in_stack_ffffff98 = pBVar6;
        (**(code **)(*(int *)**(undefined4 **)(&DAT_0050bbde + unaff_EBX) + 8))
                  ((int *)**(undefined4 **)(&DAT_0050bbde + unaff_EBX));
        *(undefined4 *)((int)&pBVar8[-1].m_blobData.m_Memory.m_pMemory + iVar5) = 0;
        pBVar6 = (BlobDataPerUpdate_t *)0x0;
      }
      *(undefined4 *)((int)&pBVar8[-1].m_blobData.m_Memory.m_nAllocationCount + iVar5) = 0;
    }
    *(BlobDataPerUpdate_t **)((int)&pBVar8[-1].m_blobData.m_pElements + iVar5) = pBVar6;
    if (-1 < *(int *)((int)&pBVar8[-1].m_blobData.m_Memory.m_nGrowSize + iVar5)) {
      if (pBVar6 != (BlobDataPerUpdate_t *)0x0) {
        (**(code **)(*(int *)**(undefined4 **)(&DAT_0050bbde + unaff_EBX) + 8))
                  ((int *)**(undefined4 **)(&DAT_0050bbde + unaff_EBX));
        *(undefined4 *)((int)&pBVar8[-1].m_blobData.m_Memory.m_pMemory + iVar5) = 0;
        in_stack_ffffff98 = pBVar6;
      }
      *(undefined4 *)((int)&pBVar8[-1].m_blobData.m_Memory.m_nAllocationCount + iVar5) = 0;
    }
  }
  pCVar1->m_Size = 0;
  if ((pCVar1->m_Memory).m_nGrowSize < 0) {
    pBVar8 = (pCVar1->m_Memory).m_pMemory;
  }
  else {
    pBVar8 = (pCVar1->m_Memory).m_pMemory;
    if (pBVar8 != (BlobDataPerUpdate_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(&DAT_0050bbde + unaff_EBX) + 8))
                ((int *)**(undefined4 **)(&DAT_0050bbde + unaff_EBX));
      (pCVar1->m_Memory).m_pMemory = (BlobDataPerUpdate_t *)0x0;
      in_stack_ffffff98 = pBVar8;
    }
    pBVar8 = (BlobDataPerUpdate_t *)0x0;
    (pCVar1->m_Memory).m_nAllocationCount = 0;
  }
  pCVar1->m_pElements = pBVar8;
  (this->m_sharedBlobData).
  super_CSharedVarBase<CUtlVector<BlobDataPerUpdate_t,_CUtlMemory<BlobDataPerUpdate_t,_int>_>,CPaintStream::SharedVar_m_sharedBlobData>
  .super_ISharedVarBase._vptr_ISharedVarBase = (_func_int_varargs **)(unaff_EBX + 0x5931d2);
  pIVar4 = (this->m_sharedBlobData).
           super_CSharedVarBase<CUtlVector<BlobDataPerUpdate_t,_CUtlMemory<BlobDataPerUpdate_t,_int>_>,CPaintStream::SharedVar_m_sharedBlobData>
           .m_pSharedMemory;
  if (pIVar4 != (ISPSharedMemory *)0x0) {
    (*pIVar4->_vptr_ISPSharedMemory[4])(pIVar4);
    (this->m_sharedBlobData).
    super_CSharedVarBase<CUtlVector<BlobDataPerUpdate_t,_CUtlMemory<BlobDataPerUpdate_t,_int>_>,CPaintStream::SharedVar_m_sharedBlobData>
    .m_pSharedMemory = (ISPSharedMemory *)0x0;
  }
  (this->m_sharedBlobData).
  super_CSharedVarBase<CUtlVector<BlobDataPerUpdate_t,_CUtlMemory<BlobDataPerUpdate_t,_int>_>,CPaintStream::SharedVar_m_sharedBlobData>
  .m_pValue = (CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t,_int>_> *)0x0;
  (this->m_blobs).m_Size = 0;
  if ((this->m_blobs).m_Memory.m_nGrowSize < 0) {
    pBVar8 = (BlobDataPerUpdate_t *)(this->m_blobs).m_Memory.m_pMemory;
  }
  else {
    pBVar8 = (BlobDataPerUpdate_t *)(this->m_blobs).m_Memory.m_pMemory;
    if (pBVar8 != (BlobDataPerUpdate_t *)0x0) {
      in_stack_ffffff98 = pBVar8;
      (**(code **)(*(int *)**(undefined4 **)(&DAT_0050bbde + unaff_EBX) + 8))
                ((int *)**(undefined4 **)(&DAT_0050bbde + unaff_EBX));
      pBVar8 = (BlobDataPerUpdate_t *)0x0;
      (this->m_blobs).m_Memory.m_pMemory = (CBasePaintBlob **)0x0;
    }
    (this->m_blobs).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_blobs).m_pElements = (CBasePaintBlob **)pBVar8;
  if (-1 < (this->m_blobs).m_Memory.m_nGrowSize) {
    if (pBVar8 != (BlobDataPerUpdate_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(&DAT_0050bbde + unaff_EBX) + 8))
                ((int *)**(undefined4 **)(&DAT_0050bbde + unaff_EBX));
      (this->m_blobs).m_Memory.m_pMemory = (CBasePaintBlob **)0x0;
      in_stack_ffffff98 = pBVar8;
    }
    (this->m_blobs).m_Memory.m_nAllocationCount = 0;
  }
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,(int)in_stack_ffffff98);
  CBaseEntity::operator_delete(this);
  return;
}


/* CPaintStream::~CPaintStream at 0073b890 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ~CPaintStream(CPaintStream * this, int __in_chrg) */

void __thiscall CPaintStream::~CPaintStream(CPaintStream *this,int __in_chrg)

{
  CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t,_int>_> *pCVar1;
  int iVar2;
  undefined4 *puVar3;
  ISPSharedMemory *pIVar4;
  int iVar5;
  int iVar6;
  undefined4 uVar7;
  BlobDataPerUpdate_t *pBVar8;
  CBasePaintBlob **ppCVar9;
  int unaff_EBX;
  int iVar10;
  int iVar11;
  int local_2c;
  int local_28;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x59248a);
  RemoveAllPaintBlobs(this);
  pCVar1 = (this->m_sharedBlobData).
           super_CSharedVarBase<CUtlVector<BlobDataPerUpdate_t,_CUtlMemory<BlobDataPerUpdate_t,_int>_>,CPaintStream::SharedVar_m_sharedBlobData>
           .m_pValue;
  iVar2 = pCVar1->m_Size;
  puVar3 = *(undefined4 **)(unaff_EBX + 0x50b2be);
  local_20 = iVar2 * 0x18;
  while (iVar5 = local_20, iVar2 = iVar2 + -1, -1 < iVar2) {
    local_20 = iVar5 + -0x18;
    pBVar8 = (pCVar1->m_Memory).m_pMemory;
    local_2c = *(int *)((int)&pBVar8[-1].m_blobData.m_Size + iVar5);
    iVar11 = local_2c * 0x2c;
    while( true ) {
      iVar11 = iVar11 + -0x2c;
      local_2c = local_2c + -1;
      if (local_2c < 0) break;
      iVar10 = iVar11 + *(int *)((int)&pBVar8[-1].m_blobData.m_Memory.m_pMemory + iVar5);
      *(undefined4 *)(iVar10 + 0x24) = 0;
      if (*(int *)(iVar10 + 0x20) < 0) {
        iVar6 = *(int *)(iVar10 + 0x18);
      }
      else {
        if (*(int *)(iVar10 + 0x18) != 0) {
          (**(code **)(*(int *)*puVar3 + 8))((int *)*puVar3,*(int *)(iVar10 + 0x18));
          *(undefined4 *)(iVar10 + 0x18) = 0;
        }
        iVar6 = 0;
        *(undefined4 *)(iVar10 + 0x1c) = 0;
      }
      *(int *)(iVar10 + 0x28) = iVar6;
      *(undefined4 *)(iVar10 + 0x24) = 0;
      if (-1 < *(int *)(iVar10 + 0x20)) {
        if (iVar6 != 0) {
          (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x50b2be) + 8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x50b2be),iVar6);
          iVar6 = 0;
          *(undefined4 *)(iVar10 + 0x18) = 0;
        }
        *(undefined4 *)(iVar10 + 0x1c) = 0;
      }
      *(int *)(iVar10 + 0x28) = iVar6;
      if (-1 < *(int *)(iVar10 + 0x20)) {
        if (iVar6 != 0) {
          (**(code **)(*(int *)*puVar3 + 8))((int *)*puVar3,iVar6);
          *(undefined4 *)(iVar10 + 0x18) = 0;
        }
        *(undefined4 *)(iVar10 + 0x1c) = 0;
      }
    }
    *(undefined4 *)((int)&pBVar8[-1].m_blobData.m_Size + iVar5) = 0;
    if (*(int *)((int)&pBVar8[-1].m_blobData.m_Memory.m_nGrowSize + iVar5) < 0) {
      uVar7 = *(undefined4 *)((int)&pBVar8[-1].m_blobData.m_Memory.m_pMemory + iVar5);
    }
    else {
      iVar11 = *(int *)((int)&pBVar8[-1].m_blobData.m_Memory.m_pMemory + iVar5);
      if (iVar11 != 0) {
        (**(code **)(*(int *)*puVar3 + 8))((int *)*puVar3,iVar11);
        *(undefined4 *)((int)&pBVar8[-1].m_blobData.m_Memory.m_pMemory + iVar5) = 0;
      }
      uVar7 = 0;
      *(undefined4 *)((int)&pBVar8[-1].m_blobData.m_Memory.m_nAllocationCount + iVar5) = 0;
    }
    *(undefined4 *)((int)&pBVar8[-1].m_blobData.m_pElements + iVar5) = uVar7;
    local_28 = *(int *)((int)&pBVar8[-1].m_blobData.m_Size + iVar5);
    iVar11 = local_28 * 0x2c;
    while( true ) {
      iVar11 = iVar11 + -0x2c;
      local_28 = local_28 + -1;
      if (local_28 < 0) break;
      iVar10 = iVar11 + *(int *)((int)&pBVar8[-1].m_blobData.m_Memory.m_pMemory + iVar5);
      *(undefined4 *)(iVar10 + 0x24) = 0;
      if (*(int *)(iVar10 + 0x20) < 0) {
        iVar6 = *(int *)(iVar10 + 0x18);
      }
      else {
        if (*(int *)(iVar10 + 0x18) != 0) {
          (**(code **)(*(int *)*puVar3 + 8))((int *)*puVar3,*(int *)(iVar10 + 0x18));
          *(undefined4 *)(iVar10 + 0x18) = 0;
        }
        iVar6 = 0;
        *(undefined4 *)(iVar10 + 0x1c) = 0;
      }
      *(int *)(iVar10 + 0x28) = iVar6;
      *(undefined4 *)(iVar10 + 0x24) = 0;
      if (-1 < *(int *)(iVar10 + 0x20)) {
        if (iVar6 != 0) {
          (**(code **)(*(int *)*puVar3 + 8))((int *)*puVar3,iVar6);
          iVar6 = 0;
          *(undefined4 *)(iVar10 + 0x18) = 0;
        }
        *(undefined4 *)(iVar10 + 0x1c) = 0;
      }
      *(int *)(iVar10 + 0x28) = iVar6;
      if (-1 < *(int *)(iVar10 + 0x20)) {
        if (iVar6 != 0) {
          (**(code **)(*(int *)*puVar3 + 8))((int *)*puVar3,iVar6);
          *(undefined4 *)(iVar10 + 0x18) = 0;
        }
        *(undefined4 *)(iVar10 + 0x1c) = 0;
      }
    }
    *(undefined4 *)((int)&pBVar8[-1].m_blobData.m_Size + iVar5) = 0;
    if (*(int *)((int)&pBVar8[-1].m_blobData.m_Memory.m_nGrowSize + iVar5) < 0) {
      iVar11 = *(int *)((int)&pBVar8[-1].m_blobData.m_Memory.m_pMemory + iVar5);
    }
    else {
      iVar11 = *(int *)((int)&pBVar8[-1].m_blobData.m_Memory.m_pMemory + iVar5);
      if (iVar11 != 0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x50b2be) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x50b2be),iVar11);
        *(undefined4 *)((int)&pBVar8[-1].m_blobData.m_Memory.m_pMemory + iVar5) = 0;
      }
      iVar11 = 0;
      *(undefined4 *)((int)&pBVar8[-1].m_blobData.m_Memory.m_nAllocationCount + iVar5) = 0;
    }
    *(int *)((int)&pBVar8[-1].m_blobData.m_pElements + iVar5) = iVar11;
    if (-1 < *(int *)((int)&pBVar8[-1].m_blobData.m_Memory.m_nGrowSize + iVar5)) {
      if (iVar11 != 0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x50b2be) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x50b2be),iVar11);
        *(undefined4 *)((int)&pBVar8[-1].m_blobData.m_Memory.m_pMemory + iVar5) = 0;
      }
      *(undefined4 *)((int)&pBVar8[-1].m_blobData.m_Memory.m_nAllocationCount + iVar5) = 0;
    }
  }
  pCVar1->m_Size = 0;
  if ((pCVar1->m_Memory).m_nGrowSize < 0) {
    pBVar8 = (pCVar1->m_Memory).m_pMemory;
  }
  else {
    pBVar8 = (pCVar1->m_Memory).m_pMemory;
    if (pBVar8 != (BlobDataPerUpdate_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x50b2be) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x50b2be),pBVar8);
      (pCVar1->m_Memory).m_pMemory = (BlobDataPerUpdate_t *)0x0;
    }
    pBVar8 = (BlobDataPerUpdate_t *)0x0;
    (pCVar1->m_Memory).m_nAllocationCount = 0;
  }
  pCVar1->m_pElements = pBVar8;
  (this->m_sharedBlobData).
  super_CSharedVarBase<CUtlVector<BlobDataPerUpdate_t,_CUtlMemory<BlobDataPerUpdate_t,_int>_>,CPaintStream::SharedVar_m_sharedBlobData>
  .super_ISharedVarBase._vptr_ISharedVarBase = (_func_int_varargs **)(unaff_EBX + 0x5928b2);
  pIVar4 = (this->m_sharedBlobData).
           super_CSharedVarBase<CUtlVector<BlobDataPerUpdate_t,_CUtlMemory<BlobDataPerUpdate_t,_int>_>,CPaintStream::SharedVar_m_sharedBlobData>
           .m_pSharedMemory;
  if (pIVar4 != (ISPSharedMemory *)0x0) {
    (*pIVar4->_vptr_ISPSharedMemory[4])(pIVar4);
    (this->m_sharedBlobData).
    super_CSharedVarBase<CUtlVector<BlobDataPerUpdate_t,_CUtlMemory<BlobDataPerUpdate_t,_int>_>,CPaintStream::SharedVar_m_sharedBlobData>
    .m_pSharedMemory = (ISPSharedMemory *)0x0;
  }
  (this->m_sharedBlobData).
  super_CSharedVarBase<CUtlVector<BlobDataPerUpdate_t,_CUtlMemory<BlobDataPerUpdate_t,_int>_>,CPaintStream::SharedVar_m_sharedBlobData>
  .m_pValue = (CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t,_int>_> *)0x0;
  (this->m_blobs).m_Size = 0;
  if ((this->m_blobs).m_Memory.m_nGrowSize < 0) {
    ppCVar9 = (this->m_blobs).m_Memory.m_pMemory;
  }
  else {
    ppCVar9 = (this->m_blobs).m_Memory.m_pMemory;
    if (ppCVar9 != (CBasePaintBlob **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x50b2be) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x50b2be),ppCVar9);
      (this->m_blobs).m_Memory.m_pMemory = (CBasePaintBlob **)0x0;
    }
    ppCVar9 = (CBasePaintBlob **)0x0;
    (this->m_blobs).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_blobs).m_pElements = ppCVar9;
  if (-1 < (this->m_blobs).m_Memory.m_nGrowSize) {
    if (ppCVar9 != (CBasePaintBlob **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x50b2be) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x50b2be),ppCVar9);
      (this->m_blobs).m_Memory.m_pMemory = (CBasePaintBlob **)0x0;
    }
    (this->m_blobs).m_Memory.m_nAllocationCount = 0;
  }
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
  return;
}


/* CPaintStream::~CPaintStream at 0073c1a0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ~CPaintStream(CPaintStream * this, int __in_chrg, CPaintStream *
   this) */

void __thiscall CPaintStream::~CPaintStream(CPaintStream *this,int __in_chrg,CPaintStream *this_1)

{
  ~CPaintStream(this,__in_chrg);
  return;
}


/* CPaintStream::Precache at 0073a4c0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void Precache(CPaintStream * this) */

void __thiscall CPaintStream::Precache(CPaintStream *this)

{
  CBaseAnimating::Precache(&this->super_CBaseAnimating);
  return;
}


/* CPaintStream::Spawn at 0073a470 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(CPaintStream * this) */

void __thiscall CPaintStream::Spawn(CPaintStream *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::AddEffects((CBaseEntity *)this,0x50);
  this->m_iCachedWorldBoundsUpdateTick = *(int *)(**(int **)(unaff_EBX + 0x50c6e3) + 0x18);
  CBaseAnimating::Spawn(&this->super_CBaseAnimating);
  return;
}


/* CPaintStream::UpdateTransmitState at 0073a450 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: int UpdateTransmitState(CPaintStream * this) */

int __thiscall CPaintStream::UpdateTransmitState(CPaintStream *this)

{
  int iVar1;
  
  iVar1 = CBaseEntity::SetTransmitState((CBaseEntity *)this,8);
  return iVar1;
}


/* CPaintStream::AddPaintBlob at 0073aaf0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void AddPaintBlob(CPaintStream * this, CBasePaintBlob * pBlob) */

void __thiscall CPaintStream::AddPaintBlob(CPaintStream *this,CBasePaintBlob *pBlob)

{
  int *piVar1;
  uint uVar2;
  edict_t *peVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  CBasePaintBlob **ppCVar7;
  int unaff_EBX;
  int local_20;
  
                    /* Unresolved local var: CPaintBlob * pServerBlob@[???] */
  ___i686_get_pc_thunk_bx();
  if (pBlob == (CBasePaintBlob *)0x0) {
    return;
  }
  uVar2 = (this->m_blobs).m_Size;
  iVar6 = uVar2 + 1;
  iVar5 = (this->m_blobs).m_Memory.m_nAllocationCount;
  if (iVar6 <= iVar5) goto LAB_0073ab27;
  iVar4 = (this->m_blobs).m_Memory.m_nGrowSize;
  if (iVar4 < 0) goto LAB_0073ab27;
  if (iVar4 == 0) {
    if ((iVar5 == 0) && (iVar5 = 8, iVar6 < 9)) {
      local_20 = 0x20;
    }
    else {
      do {
        local_20 = iVar5;
        iVar5 = local_20 * 2;
      } while (iVar5 < iVar6);
      local_20 = local_20 * 8;
    }
    goto LAB_0073ac51;
  }
  iVar5 = ((int)uVar2 / iVar4 + 1) * iVar4;
  if (iVar5 < iVar6) {
    if (iVar5 != 0) {
LAB_0073abe0:
      do {
        iVar5 = (iVar5 + iVar6) / 2;
      } while (iVar5 < iVar6);
      goto LAB_0073abf3;
    }
    if (iVar6 < 0) {
      iVar5 = -1;
      local_20 = -4;
    }
    else {
      local_20 = 0;
      if (iVar6 != 0) goto LAB_0073abe0;
    }
  }
  else {
LAB_0073abf3:
    local_20 = iVar5 * 4;
  }
LAB_0073ac51:
  (this->m_blobs).m_Memory.m_nAllocationCount = iVar5;
  ppCVar7 = (this->m_blobs).m_Memory.m_pMemory;
  if (ppCVar7 == (CBasePaintBlob **)0x0) {
    ppCVar7 = (CBasePaintBlob **)
              (*(code *)**(undefined4 **)**(undefined4 **)(&DAT_0050c05e + unaff_EBX))
                        ((undefined4 *)**(undefined4 **)(&DAT_0050c05e + unaff_EBX),local_20);
    (this->m_blobs).m_Memory.m_pMemory = ppCVar7;
  }
  else {
    ppCVar7 = (CBasePaintBlob **)
              (**(code **)(*(int *)**(undefined4 **)(&DAT_0050c05e + unaff_EBX) + 4))
                        ((int *)**(undefined4 **)(&DAT_0050c05e + unaff_EBX),ppCVar7,local_20);
    (this->m_blobs).m_Memory.m_pMemory = ppCVar7;
  }
LAB_0073ab27:
  piVar1 = &(this->m_blobs).m_Size;
  *piVar1 = *piVar1 + 1;
  ppCVar7 = (this->m_blobs).m_Memory.m_pMemory;
  (this->m_blobs).m_pElements = ppCVar7;
  iVar5 = ~uVar2 + (this->m_blobs).m_Size;
  if (0 < iVar5) {
    _V_memmove(ppCVar7 + iVar6,ppCVar7 + uVar2,iVar5 * 4);
  }
  ppCVar7 = (this->m_blobs).m_Memory.m_pMemory + uVar2;
  if (ppCVar7 != (CBasePaintBlob **)0x0) {
    *ppCVar7 = pBlob;
  }
  peVar3 = (this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev;
                    /* Unresolved local var: int edictIndex@[???] */
  iVar6 = 0;
  if (peVar3 != (edict_t *)0x0) {
    iVar6 = (int)peVar3 - *(int *)(**(int **)(&DAT_0050c066 + unaff_EBX) + 0x58) >> 4;
  }
  pBlob[1].m_vecPosition.y = (vec_t)(iVar6 + this->m_nBlobCounter);
  this->m_nBlobCounter = this->m_nBlobCounter + 1;
  return;
}


/* CPaintStream::UpdateRenderBoundsAndOriginWorldspace at 0073a1d0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void UpdateRenderBoundsAndOriginWorldspace(CPaintStream * this) */

void __thiscall CPaintStream::UpdateRenderBoundsAndOriginWorldspace(CPaintStream *this)

{
  float fVar1;
  float fVar2;
  vec_t vVar3;
  vec_t vVar4;
  vec_t vVar5;
  Vector *pVVar6;
  int unaff_EBX;
  int iVar7;
  float fVar8;
  float fVar9;
  float fVar10;
  float fVar11;
  
  ___i686_get_pc_thunk_bx();
  iVar7 = *(int *)(**(int **)(unaff_EBX + 0x50c986) + 0x18);
  if (this->m_iCachedWorldBoundsUpdateTick != iVar7) {
    this->m_iCachedWorldBoundsUpdateTick = iVar7;
    if ((this->m_blobs).m_Size != 0) {
      pVVar6 = CBasePaintBlob::GetPosition(*(this->m_blobs).m_Memory.m_pMemory);
      vVar3 = pVVar6->z;
      vVar4 = pVVar6->y;
      vVar5 = pVVar6->x;
      (this->m_vCachedWorldMins).x = vVar5;
      (this->m_vCachedWorldMins).y = vVar4;
      (this->m_vCachedWorldMins).z = vVar3;
      (this->m_vCachedWorldMaxs).x = vVar5;
      (this->m_vCachedWorldMaxs).y = vVar4;
      (this->m_vCachedWorldMaxs).z = vVar3;
                    /* Unresolved local var: int i@[???] */
      if (1 < (this->m_blobs).m_Size) {
        iVar7 = 1;
        do {
          pVVar6 = CBasePaintBlob::GetPosition((this->m_blobs).m_Memory.m_pMemory[iVar7]);
          fVar9 = pVVar6->x;
          fVar10 = pVVar6->y;
          fVar11 = pVVar6->z;
          fVar1 = (this->m_vCachedWorldMins).x;
          fVar8 = fVar9;
          if (fVar1 <= fVar9) {
            fVar8 = fVar1;
          }
          (this->m_vCachedWorldMins).x = fVar8;
          fVar1 = (this->m_vCachedWorldMins).y;
          fVar8 = fVar10;
          if (fVar1 <= fVar10) {
            fVar8 = fVar1;
          }
          (this->m_vCachedWorldMins).y = fVar8;
          fVar1 = (this->m_vCachedWorldMins).z;
          fVar8 = fVar11;
          if (fVar1 <= fVar11) {
            fVar8 = fVar1;
          }
          (this->m_vCachedWorldMins).z = fVar8;
          fVar1 = (this->m_vCachedWorldMaxs).x;
          if (fVar9 <= fVar1) {
            fVar9 = fVar1;
          }
          (this->m_vCachedWorldMaxs).x = fVar9;
          fVar9 = (this->m_vCachedWorldMaxs).y;
          if (fVar10 <= fVar9) {
            fVar10 = fVar9;
          }
          (this->m_vCachedWorldMaxs).y = fVar10;
          fVar9 = (this->m_vCachedWorldMaxs).z;
          if (fVar11 <= fVar9) {
            fVar11 = fVar9;
          }
          (this->m_vCachedWorldMaxs).z = fVar11;
          iVar7 = iVar7 + 1;
        } while (iVar7 < (this->m_blobs).m_Size);
      }
                    /* Unresolved local var: Vector res@[???] */
      fVar9 = (this->m_vCachedWorldMaxs).y;
      fVar10 = (this->m_vCachedWorldMins).y;
      fVar11 = (this->m_vCachedWorldMaxs).z;
      fVar1 = (this->m_vCachedWorldMins).z;
                    /* Unresolved local var: Vector res@[???] */
      fVar8 = (this->m_vCachedWorldMaxs).x;
      fVar2 = *(float *)(unaff_EBX + 0x392922);
      (this->m_vCachedRenderOrigin).x = (fVar8 + (this->m_vCachedWorldMins).x) * fVar2;
      (this->m_vCachedRenderOrigin).y = (fVar9 + fVar10) * fVar2;
      (this->m_vCachedRenderOrigin).z = (fVar11 + fVar1) * fVar2;
      (this->m_vCachedWorldMins).x = (this->m_vCachedWorldMins).x - 18.0;
      (this->m_vCachedWorldMins).y = (this->m_vCachedWorldMins).y - 18.0;
      (this->m_vCachedWorldMins).z = (this->m_vCachedWorldMins).z - 18.0;
      (this->m_vCachedWorldMaxs).x = fVar8 + 18.0;
      (this->m_vCachedWorldMaxs).y = fVar9 + 18.0;
      (this->m_vCachedWorldMaxs).z = fVar11 + 18.0;
      CCollisionProperty::MarkSurroundingBoundsDirty
                (&(this->super_CBaseAnimating).super_CBaseEntity.m_Collision.
                  super_CCollisionProperty);
      return;
    }
    (this->m_vCachedWorldMaxs).x = 0.0;
    (this->m_vCachedWorldMaxs).y = 0.0;
    (this->m_vCachedWorldMaxs).z = 0.0;
    (this->m_vCachedWorldMins).x = 0.0;
    (this->m_vCachedWorldMins).y = 0.0;
    (this->m_vCachedWorldMins).z = 0.0;
    (this->m_vCachedRenderOrigin).x = 0.0;
    (this->m_vCachedRenderOrigin).y = 0.0;
    (this->m_vCachedRenderOrigin).z = 0.0;
  }
  return;
}


/* CPaintStream::Update at 0073d130 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void Update(CPaintStream * this) */

void __thiscall CPaintStream::Update(CPaintStream *this)

{
  Vector *position;
  int unaff_EBX;
  int local_30;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_nCurrentUpdateFrame != *(int *)(**(int **)(unaff_EBX + 0x509a20) + 4)) {
    if ((this->m_blobs).m_Size != 0) {
      DeleteDeadBlobs(this);
                    /* Unresolved local var: bool bDebugDraw@[???]
                       Unresolved local var: Color debugColor@[???] */
      if (*(int *)(*(int *)(unaff_EBX + 0x68bff8) + 0x30) != 0) {
        if (0 < (this->m_blobs).m_Size) {
          local_30 = 0;
          do {
            position = CBasePaintBlob::GetPosition((this->m_blobs).m_Memory.m_pMemory[local_30]);
            NDebugOverlay::Cross3D(position,2.0,0xff,0,0xff,false,0.1);
            local_30 = local_30 + 1;
          } while (local_30 < (this->m_blobs).m_Size);
        }
      }
      UpdateBlobs(this);
    }
    UpdateRenderBoundsAndOriginWorldspace(this);
    UpdateBlobSharedData(this);
    this->m_nCurrentUpdateFrame = *(int *)(**(int **)(unaff_EBX + 0x509a20) + 4);
  }
  return;
}


/* CPaintStream::UpdateBlobSharedData at 0073c1b0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void UpdateBlobSharedData(CPaintStream * this) */

void __thiscall CPaintStream::UpdateBlobSharedData(CPaintStream *this)

{
  BlobDataPerUpdate_t *pBVar1;
  CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t,_int>_> *this_00;
  uint uVar2;
  int *piVar3;
  CPaintBlob *this_01;
  uint uVar4;
  vec_t *pvVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  bool bVar8;
  BlobDataPerUpdate_t *pBVar9;
  Vector *pVVar10;
  int iVar11;
  int iVar12;
  BlobDataVector_t *this_02;
  int iVar13;
  undefined4 uVar14;
  undefined4 *puVar15;
  undefined4 *puVar16;
  int unaff_EBX;
  BlobData_t *pBVar17;
  int iVar18;
  int iVar19;
  float fVar20;
  BlobDataPerUpdateVector_t *blobData;
  int local_68;
  int local_64;
  int local_60;
  int local_50;
  int local_3c;
  int local_38;
  int local_30;
  int local_2c;
  int local_28;
  int local_24;
  
                    /* Unresolved local var: int lastIndex@[???]
                       Unresolved local var: int nBlobCount@[???]
                       Unresolved local var: BlobDataVector_t * blobDataVector@[???]
                       Unresolved local var: int nNumParticles@[???]
                       Unresolved local var: float flShouldRemoveTime@[???]
                       Unresolved local var: int nToRemove@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = (this->m_sharedBlobData).
            super_CSharedVarBase<CUtlVector<BlobDataPerUpdate_t,_CUtlMemory<BlobDataPerUpdate_t,_int>_>,CPaintStream::SharedVar_m_sharedBlobData>
            .m_pValue;
  uVar2 = this_00->m_Size;
  CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t,_int>_>::GrowVector(this_00,1);
  iVar11 = ~uVar2 + this_00->m_Size;
  if (0 < iVar11) {
    pBVar9 = (this_00->m_Memory).m_pMemory;
    _V_memmove(pBVar9 + uVar2 + 1,pBVar9 + uVar2,iVar11 * 0x18);
  }
  pBVar9 = (this_00->m_Memory).m_pMemory + uVar2;
  if (pBVar9 != (BlobDataPerUpdate_t *)0x0) {
    (pBVar9->m_blobData).m_Memory.m_pMemory = (BlobData_t *)0x0;
    (pBVar9->m_blobData).m_Memory.m_nAllocationCount = 0;
    (pBVar9->m_blobData).m_Memory.m_nGrowSize = 0;
    (pBVar9->m_blobData).m_Size = 0;
    (pBVar9->m_blobData).m_pElements = (BlobData_t *)0x0;
    pBVar9->m_flUpdateTime = 0.0;
  }
  (this_00->m_Memory).m_pMemory[uVar2].m_flUpdateTime =
       *(float *)(**(int **)(unaff_EBX + 0x50a9a3) + 0xc);
  iVar11 = (this->m_blobs).m_Size;
  pBVar9 = (this_00->m_Memory).m_pMemory;
  iVar18 = pBVar9[uVar2].m_blobData.m_Size;
  puVar15 = *(undefined4 **)(unaff_EBX + 0x50a99b);
  local_24 = iVar18 * 0x2c;
  while( true ) {
    iVar19 = local_24;
    local_24 = iVar19 + -0x2c;
    iVar18 = iVar18 + -1;
    if (iVar18 < 0) break;
    pBVar17 = pBVar9[uVar2].m_blobData.m_Memory.m_pMemory;
    *(undefined4 *)((int)&pBVar17[-1].m_teleportationHistory.m_Size + iVar19) = 0;
    if (*(int *)((int)&pBVar17[-1].m_teleportationHistory.m_Memory.m_nGrowSize + iVar19) < 0) {
      iVar12 = *(int *)((int)&pBVar17[-1].m_teleportationHistory.m_Memory.m_pMemory + iVar19);
    }
    else {
      iVar12 = *(int *)((int)&pBVar17[-1].m_teleportationHistory.m_Memory.m_pMemory + iVar19);
      if (iVar12 != 0) {
        piVar3 = (int *)*puVar15;
        (**(code **)(*piVar3 + 8))(piVar3,iVar12);
        *(undefined4 *)((int)&pBVar17[-1].m_teleportationHistory.m_Memory.m_pMemory + iVar19) = 0;
      }
      iVar12 = 0;
      *(undefined4 *)((int)&pBVar17[-1].m_teleportationHistory.m_Memory.m_nAllocationCount + iVar19)
           = 0;
    }
    *(int *)((int)&pBVar17[-1].m_teleportationHistory.m_pElements + iVar19) = iVar12;
    *(undefined4 *)((int)&pBVar17[-1].m_teleportationHistory.m_Size + iVar19) = 0;
    if (-1 < *(int *)((int)&pBVar17[-1].m_teleportationHistory.m_Memory.m_nGrowSize + iVar19)) {
      if (iVar12 != 0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x50a99b) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x50a99b),iVar12);
        iVar12 = 0;
        *(undefined4 *)((int)&pBVar17[-1].m_teleportationHistory.m_Memory.m_pMemory + iVar19) = 0;
      }
      *(undefined4 *)((int)&pBVar17[-1].m_teleportationHistory.m_Memory.m_nAllocationCount + iVar19)
           = 0;
    }
    *(int *)((int)&pBVar17[-1].m_teleportationHistory.m_pElements + iVar19) = iVar12;
    if (-1 < *(int *)((int)&pBVar17[-1].m_teleportationHistory.m_Memory.m_nGrowSize + iVar19)) {
      if (iVar12 != 0) {
        piVar3 = (int *)*puVar15;
        (**(code **)(*piVar3 + 8))(piVar3,iVar12);
        *(undefined4 *)((int)&pBVar17[-1].m_teleportationHistory.m_Memory.m_pMemory + iVar19) = 0;
      }
      *(undefined4 *)((int)&pBVar17[-1].m_teleportationHistory.m_Memory.m_nAllocationCount + iVar19)
           = 0;
    }
  }
  iVar18 = 0;
  pBVar9[uVar2].m_blobData.m_Size = 0;
  this_02 = &pBVar9[uVar2].m_blobData;
  if (iVar11 != 0) {
    CUtlVector<BlobData_t,CUtlMemory<BlobData_t,_int>_>::GrowVector(this_02,iVar11);
                    /* Unresolved local var: int numToMove@[???] */
    iVar18 = pBVar9[uVar2].m_blobData.m_Size;
    if (0 < iVar18 - iVar11) {
      if (0 >= iVar11) goto LAB_0073c364;
      pBVar17 = pBVar9[uVar2].m_blobData.m_Memory.m_pMemory;
      _V_memmove(pBVar17 + iVar11,pBVar17,(iVar18 - iVar11) * 0x2c);
    }
                    /* Unresolved local var: int i@[???] */
    if (0 < iVar11) {
      iVar19 = 0;
      iVar18 = 0;
      puVar15 = *(undefined4 **)(unaff_EBX + 0x50a9b7);
      do {
        puVar16 = (undefined4 *)
                  ((int)&(pBVar9[uVar2].m_blobData.m_Memory.m_pMemory)->m_blobID + iVar18);
        if (puVar16 != (undefined4 *)0x0) {
          puVar16[6] = 0;
          puVar16[7] = 0;
          puVar16[8] = 0;
          puVar16[9] = 0;
          puVar16[10] = 0;
          *puVar16 = 0;
          puVar16[1] = *puVar15;
          puVar16[2] = puVar15[1];
          puVar16[3] = puVar15[2];
          puVar16[4] = 0;
          *(undefined1 *)(puVar16 + 5) = 0;
        }
        iVar19 = iVar19 + 1;
        iVar18 = iVar18 + 0x2c;
      } while (iVar19 < iVar11);
      local_68 = 0;
      local_60 = 0;
      do {
        while (this_01 = (CPaintBlob *)(this->m_blobs).m_Memory.m_pMemory[local_60],
              (this_01->super_CBasePaintBlob).m_bSilent != false) {
LAB_0073cd70:
          local_60 = local_60 + 1;
          if (local_60 == iVar11) goto LAB_0073ce4c;
        }
        bVar8 = CPaintBlob::HasBlobTeleportedThisFrame(this_01);
                    /* Unresolved local var: BlobData_t * blobData@[???] */
        pBVar17 = pBVar9[uVar2].m_blobData.m_Memory.m_pMemory + local_68;
        pBVar17->m_blobID = this_01->m_ID;
        fVar20 = CBasePaintBlob::GetRadiusScale((CBasePaintBlob *)this_01);
        pBVar17->m_flScale = fVar20;
        pVVar10 = CBasePaintBlob::GetPosition((CBasePaintBlob *)this_01);
        (pBVar17->m_vPosition).x = pVVar10->x;
        (pBVar17->m_vPosition).y = pVVar10->y;
        (pBVar17->m_vPosition).z = pVVar10->z;
        CPaintBlob::GetTeleportationHistory(this_01,&pBVar17->m_teleportationHistory);
        pBVar17->m_bTeleportedThisFrame = bVar8;
        iVar18 = local_68 + 1;
        if ((this_01->super_CBasePaintBlob).m_bGhosting != false) {
                    /* Unresolved local var: BlobData_t * blobData@[???] */
          uVar4 = pBVar9[uVar2].m_blobData.m_Size;
          CUtlVector<BlobData_t,CUtlMemory<BlobData_t,_int>_>::GrowVector(this_02,1);
          iVar18 = ~uVar4 + pBVar9[uVar2].m_blobData.m_Size;
          if (0 < iVar18) {
            pBVar17 = pBVar9[uVar2].m_blobData.m_Memory.m_pMemory;
            _V_memmove(pBVar17 + uVar4 + 1,pBVar17 + uVar4,iVar18 * 0x2c);
          }
          pBVar17 = pBVar9[uVar2].m_blobData.m_Memory.m_pMemory + uVar4;
          if (pBVar17 != (BlobData_t *)0x0) {
            (pBVar17->m_teleportationHistory).m_Memory.m_pMemory = (BlobTeleportationHistory_t *)0x0
            ;
            (pBVar17->m_teleportationHistory).m_Memory.m_nAllocationCount = 0;
            (pBVar17->m_teleportationHistory).m_Memory.m_nGrowSize = 0;
            (pBVar17->m_teleportationHistory).m_Size = 0;
            (pBVar17->m_teleportationHistory).m_pElements = (BlobTeleportationHistory_t *)0x0;
            pBVar17->m_blobID = 0;
            pvVar5 = *(vec_t **)(unaff_EBX + 0x50a9b7);
            (pBVar17->m_vPosition).x = *pvVar5;
            (pBVar17->m_vPosition).y = pvVar5[1];
            (pBVar17->m_vPosition).z = pvVar5[2];
            pBVar17->m_flScale = 0.0;
            pBVar17->m_bTeleportedThisFrame = false;
          }
          pBVar17 = pBVar9[uVar2].m_blobData.m_Memory.m_pMemory + local_68 + 1;
          pBVar17->m_blobID = -this_01->m_ID;
          fVar20 = CBasePaintBlob::GetRadiusScale((CBasePaintBlob *)this_01);
          pBVar17->m_flScale = fVar20;
          pVVar10 = CBasePaintBlob::GetGhostPosition((CBasePaintBlob *)this_01);
          (pBVar17->m_vPosition).x = pVVar10->x;
          (pBVar17->m_vPosition).y = pVVar10->y;
          (pBVar17->m_vPosition).z = pVVar10->z;
          CPaintBlob::GetTeleportationHistory(this_01,&pBVar17->m_teleportationHistory);
                    /* Unresolved local var: int h@[???] */
          if (0 < (pBVar17->m_teleportationHistory).m_Size) {
            local_50 = 0;
            local_28 = 0;
            do {
                    /* Unresolved local var: BlobTeleportationHistory_t * history@[???] */
              puVar15 = (undefined4 *)
                        ((int)&((pBVar17->m_teleportationHistory).m_Memory.m_pMemory)->
                               m_vEnterPosition + local_28);
              uVar14 = puVar15[5];
              uVar6 = puVar15[4];
              uVar7 = puVar15[3];
                    /* Unresolved local var: Vector temp@[???] */
              puVar15[3] = *puVar15;
              puVar15[4] = puVar15[1];
              puVar15[5] = puVar15[2];
              *puVar15 = uVar7;
              puVar15[1] = uVar6;
              puVar15[2] = uVar14;
              local_50 = local_50 + 1;
              local_28 = local_28 + 0x18;
            } while (local_50 < (pBVar17->m_teleportationHistory).m_Size);
          }
          pBVar17->m_bTeleportedThisFrame = bVar8;
          iVar18 = local_68 + 2;
        }
        local_68 = iVar18;
        if (!bVar8) goto LAB_0073cd70;
        CPaintBlob::SetBlobTeleportedThisFrame(this_01,false);
        CPaintBlob::ClearTeleportationHistory(this_01);
        local_60 = local_60 + 1;
      } while (local_60 != iVar11);
LAB_0073ce4c:
      iVar18 = pBVar9[uVar2].m_blobData.m_Size;
      goto LAB_0073c36b;
    }
    iVar18 = pBVar9[uVar2].m_blobData.m_Size;
  }
LAB_0073c364:
  local_68 = 0;
LAB_0073c36b:
  local_68 = local_68 - iVar18;
  if (local_68 < 1) {
    if (local_68 != 0) {
      iVar11 = iVar18 + local_68;
      if (iVar11 < iVar18) {
        local_2c = iVar11 * 0x2c;
        puVar15 = *(undefined4 **)(unaff_EBX + 0x50a99b);
        do {
          pBVar17 = pBVar9[uVar2].m_blobData.m_Memory.m_pMemory;
          *(undefined4 *)((int)&(pBVar17->m_teleportationHistory).m_Size + local_2c) = 0;
          if (*(int *)((int)&(pBVar17->m_teleportationHistory).m_Memory.m_nGrowSize + local_2c) < 0)
          {
            iVar18 = *(int *)((int)&(pBVar17->m_teleportationHistory).m_Memory.m_pMemory + local_2c)
            ;
          }
          else {
            iVar18 = *(int *)((int)&(pBVar17->m_teleportationHistory).m_Memory.m_pMemory + local_2c)
            ;
            if (iVar18 != 0) {
              piVar3 = (int *)*puVar15;
              (**(code **)(*piVar3 + 8))(piVar3,iVar18);
              *(undefined4 *)((int)&(pBVar17->m_teleportationHistory).m_Memory.m_pMemory + local_2c)
                   = 0;
            }
            iVar18 = 0;
            *(undefined4 *)
             ((int)&(pBVar17->m_teleportationHistory).m_Memory.m_nAllocationCount + local_2c) = 0;
          }
          *(int *)((int)&(pBVar17->m_teleportationHistory).m_pElements + local_2c) = iVar18;
          *(undefined4 *)((int)&(pBVar17->m_teleportationHistory).m_Size + local_2c) = 0;
          if (-1 < *(int *)((int)&(pBVar17->m_teleportationHistory).m_Memory.m_nGrowSize + local_2c)
             ) {
            if (iVar18 != 0) {
              (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x50a99b) + 8))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x50a99b),iVar18);
              iVar18 = 0;
              *(undefined4 *)((int)&(pBVar17->m_teleportationHistory).m_Memory.m_pMemory + local_2c)
                   = 0;
            }
            *(undefined4 *)
             ((int)&(pBVar17->m_teleportationHistory).m_Memory.m_nAllocationCount + local_2c) = 0;
          }
          *(int *)((int)&(pBVar17->m_teleportationHistory).m_pElements + local_2c) = iVar18;
          if (-1 < *(int *)((int)&(pBVar17->m_teleportationHistory).m_Memory.m_nGrowSize + local_2c)
             ) {
            if (iVar18 != 0) {
              piVar3 = (int *)*puVar15;
              (**(code **)(*piVar3 + 8))(piVar3,iVar18);
              *(undefined4 *)((int)&(pBVar17->m_teleportationHistory).m_Memory.m_pMemory + local_2c)
                   = 0;
            }
            *(undefined4 *)
             ((int)&(pBVar17->m_teleportationHistory).m_Memory.m_nAllocationCount + local_2c) = 0;
          }
          iVar11 = iVar11 + 1;
          iVar18 = pBVar9[uVar2].m_blobData.m_Size;
          local_2c = local_2c + 0x2c;
        } while (iVar11 < iVar18);
        iVar11 = iVar18 + local_68;
      }
      pBVar9[uVar2].m_blobData.m_Size = iVar11;
    }
  }
  else {
    CUtlVector<BlobData_t,CUtlMemory<BlobData_t,_int>_>::GrowVector(this_02,local_68);
    iVar11 = (pBVar9[uVar2].m_blobData.m_Size - iVar18) - local_68;
    if (0 < iVar11) {
      pBVar17 = pBVar9[uVar2].m_blobData.m_Memory.m_pMemory;
      _V_memmove(pBVar17 + local_68 + iVar18,pBVar17 + iVar18,iVar11 * 0x2c);
    }
    iVar18 = iVar18 * 0x2c;
    iVar11 = 0;
                    /* Unresolved local var: int i@[???] */
    do {
      puVar15 = (undefined4 *)
                ((int)&(pBVar9[uVar2].m_blobData.m_Memory.m_pMemory)->m_blobID + iVar18);
      if (puVar15 != (undefined4 *)0x0) {
        puVar15[6] = 0;
        puVar15[7] = 0;
        puVar15[8] = 0;
        puVar15[9] = 0;
        puVar15[10] = 0;
        *puVar15 = 0;
        puVar16 = *(undefined4 **)(unaff_EBX + 0x50a9b7);
        puVar15[1] = *puVar16;
        puVar15[2] = puVar16[1];
        puVar15[3] = puVar16[2];
        puVar15[4] = 0;
        *(undefined1 *)(puVar15 + 5) = 0;
      }
      iVar11 = iVar11 + 1;
      iVar18 = iVar18 + 0x2c;
    } while (iVar11 != local_68);
  }
  if (0 < this_00->m_Size) {
    fVar20 = *(float *)(**(int **)(unaff_EBX + 0x50a9a3) + 0xc) -
             *(float *)(*(int *)(unaff_EBX + 0x68cfdb) + 0x2c);
    pBVar9 = (this_00->m_Memory).m_pMemory;
    if (pBVar9->m_flUpdateTime <= fVar20 && fVar20 != pBVar9->m_flUpdateTime) {
      local_64 = 0;
      do {
        iVar11 = local_64 + 1;
        local_64 = iVar11;
        if (iVar11 == this_00->m_Size) break;
        pBVar1 = pBVar9 + 1;
        pBVar9 = pBVar9 + 1;
      } while (pBVar1->m_flUpdateTime < fVar20);
      puVar15 = *(undefined4 **)(unaff_EBX + 0x50a99b);
      local_30 = iVar11 * 0x18;
      while( true ) {
        iVar18 = local_30;
        local_30 = iVar18 + -0x18;
        local_64 = local_64 + -1;
        if (local_64 < 0) break;
        pBVar9 = (this_00->m_Memory).m_pMemory;
        local_3c = *(int *)((int)&pBVar9[-1].m_blobData.m_Size + iVar18);
        iVar19 = local_3c * 0x2c;
        while( true ) {
          iVar19 = iVar19 + -0x2c;
          local_3c = local_3c + -1;
          if (local_3c < 0) break;
          iVar12 = iVar19 + *(int *)((int)&pBVar9[-1].m_blobData.m_Memory.m_pMemory + iVar18);
          *(undefined4 *)(iVar12 + 0x24) = 0;
          if (*(int *)(iVar12 + 0x20) < 0) {
            iVar13 = *(int *)(iVar12 + 0x18);
          }
          else {
            if (*(int *)(iVar12 + 0x18) != 0) {
              (**(code **)(*(int *)*puVar15 + 8))((int *)*puVar15,*(int *)(iVar12 + 0x18));
              *(undefined4 *)(iVar12 + 0x18) = 0;
            }
            iVar13 = 0;
            *(undefined4 *)(iVar12 + 0x1c) = 0;
          }
          *(int *)(iVar12 + 0x28) = iVar13;
          *(undefined4 *)(iVar12 + 0x24) = 0;
          if (-1 < *(int *)(iVar12 + 0x20)) {
            if (iVar13 != 0) {
              (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x50a99b) + 8))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x50a99b),iVar13);
              iVar13 = 0;
              *(undefined4 *)(iVar12 + 0x18) = 0;
            }
            *(undefined4 *)(iVar12 + 0x1c) = 0;
          }
          *(int *)(iVar12 + 0x28) = iVar13;
          if (-1 < *(int *)(iVar12 + 0x20)) {
            if (iVar13 != 0) {
              (**(code **)(*(int *)*puVar15 + 8))((int *)*puVar15,iVar13);
              *(undefined4 *)(iVar12 + 0x18) = 0;
            }
            *(undefined4 *)(iVar12 + 0x1c) = 0;
          }
        }
        *(undefined4 *)((int)&pBVar9[-1].m_blobData.m_Size + iVar18) = 0;
        if (*(int *)((int)&pBVar9[-1].m_blobData.m_Memory.m_nGrowSize + iVar18) < 0) {
          uVar14 = *(undefined4 *)((int)&pBVar9[-1].m_blobData.m_Memory.m_pMemory + iVar18);
        }
        else {
          iVar19 = *(int *)((int)&pBVar9[-1].m_blobData.m_Memory.m_pMemory + iVar18);
          if (iVar19 != 0) {
            (**(code **)(*(int *)*puVar15 + 8))((int *)*puVar15,iVar19);
            *(undefined4 *)((int)&pBVar9[-1].m_blobData.m_Memory.m_pMemory + iVar18) = 0;
          }
          uVar14 = 0;
          *(undefined4 *)((int)&pBVar9[-1].m_blobData.m_Memory.m_nAllocationCount + iVar18) = 0;
        }
        *(undefined4 *)((int)&pBVar9[-1].m_blobData.m_pElements + iVar18) = uVar14;
        local_38 = *(int *)((int)&pBVar9[-1].m_blobData.m_Size + iVar18);
        iVar19 = local_38 * 0x2c;
        while( true ) {
          iVar19 = iVar19 + -0x2c;
          local_38 = local_38 + -1;
          if (local_38 < 0) break;
          iVar12 = iVar19 + *(int *)((int)&pBVar9[-1].m_blobData.m_Memory.m_pMemory + iVar18);
          *(undefined4 *)(iVar12 + 0x24) = 0;
          if (*(int *)(iVar12 + 0x20) < 0) {
            iVar13 = *(int *)(iVar12 + 0x18);
          }
          else {
            if (*(int *)(iVar12 + 0x18) != 0) {
              (**(code **)(*(int *)*puVar15 + 8))((int *)*puVar15,*(int *)(iVar12 + 0x18));
              *(undefined4 *)(iVar12 + 0x18) = 0;
            }
            iVar13 = 0;
            *(undefined4 *)(iVar12 + 0x1c) = 0;
          }
          *(int *)(iVar12 + 0x28) = iVar13;
          *(undefined4 *)(iVar12 + 0x24) = 0;
          if (-1 < *(int *)(iVar12 + 0x20)) {
            if (iVar13 != 0) {
              (**(code **)(*(int *)*puVar15 + 8))((int *)*puVar15,iVar13);
              iVar13 = 0;
              *(undefined4 *)(iVar12 + 0x18) = 0;
            }
            *(undefined4 *)(iVar12 + 0x1c) = 0;
          }
          *(int *)(iVar12 + 0x28) = iVar13;
          if (-1 < *(int *)(iVar12 + 0x20)) {
            if (iVar13 != 0) {
              (**(code **)(*(int *)*puVar15 + 8))((int *)*puVar15,iVar13);
              *(undefined4 *)(iVar12 + 0x18) = 0;
            }
            *(undefined4 *)(iVar12 + 0x1c) = 0;
          }
        }
        *(undefined4 *)((int)&pBVar9[-1].m_blobData.m_Size + iVar18) = 0;
        if (*(int *)((int)&pBVar9[-1].m_blobData.m_Memory.m_nGrowSize + iVar18) < 0) {
          iVar19 = *(int *)((int)&pBVar9[-1].m_blobData.m_Memory.m_pMemory + iVar18);
        }
        else {
          iVar19 = *(int *)((int)&pBVar9[-1].m_blobData.m_Memory.m_pMemory + iVar18);
          if (iVar19 != 0) {
            (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x50a99b) + 8))
                      ((int *)**(undefined4 **)(unaff_EBX + 0x50a99b),iVar19);
            *(undefined4 *)((int)&pBVar9[-1].m_blobData.m_Memory.m_pMemory + iVar18) = 0;
          }
          iVar19 = 0;
          *(undefined4 *)((int)&pBVar9[-1].m_blobData.m_Memory.m_nAllocationCount + iVar18) = 0;
        }
        *(int *)((int)&pBVar9[-1].m_blobData.m_pElements + iVar18) = iVar19;
        if (-1 < *(int *)((int)&pBVar9[-1].m_blobData.m_Memory.m_nGrowSize + iVar18)) {
          if (iVar19 != 0) {
            (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x50a99b) + 8))
                      ((int *)**(undefined4 **)(unaff_EBX + 0x50a99b),iVar19);
            *(undefined4 *)((int)&pBVar9[-1].m_blobData.m_Memory.m_pMemory + iVar18) = 0;
          }
          *(undefined4 *)((int)&pBVar9[-1].m_blobData.m_Memory.m_nAllocationCount + iVar18) = 0;
        }
      }
                    /* Unresolved local var: int numToMove@[???] */
      iVar18 = this_00->m_Size - iVar11;
      if ((0 < iVar18) && (0 < iVar11)) {
        pBVar9 = (this_00->m_Memory).m_pMemory;
        _V_memmove(pBVar9,pBVar9 + iVar11,iVar18 * 0x18);
      }
      this_00->m_Size = this_00->m_Size - iVar11;
    }
  }
  return;
}


/* CPaintStream::DebugDrawBlobs at 0073a100 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void DebugDrawBlobs(CPaintStream * this) */

void __thiscall CPaintStream::DebugDrawBlobs(CPaintStream *this)

{
  Vector *position;
  int unaff_EBX;
  int i;
  
                    /* Unresolved local var: bool bDebugDraw@[???]
                       Unresolved local var: Color debugColor@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(int *)(*(int *)(unaff_EBX + 0x68f02e) + 0x30) != 0) {
    if (0 < (this->m_blobs).m_Size) {
      i = 0;
      do {
        position = CBasePaintBlob::GetPosition((this->m_blobs).m_Memory.m_pMemory[i]);
        NDebugOverlay::Cross3D(position,2.0,0xff,0,0xff,false,0.1);
        i = i + 1;
      } while (i < (this->m_blobs).m_Size);
    }
  }
  return;
}


/* CPaintStream::AddPaintToDatabase at 0073a0b0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void AddPaintToDatabase(CPaintStream * this) */

void __thiscall CPaintStream::AddPaintToDatabase(CPaintStream *this)

{
  int iVar1;
  
                    /* Unresolved local var: int i@[???] */
  if (0 < (this->m_blobs).m_Size) {
    iVar1 = 0;
    do {
                    /* Unresolved local var: CPaintBlob * pBlob@[???] */
      CPaintBlob::AddToPaintDatabase((CPaintBlob *)(this->m_blobs).m_Memory.m_pMemory[iVar1]);
      iVar1 = iVar1 + 1;
    } while (iVar1 < (this->m_blobs).m_Size);
  }
  return;
}


/* CPaintStream::SavePaintBlobState at 0073a4d0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void SavePaintBlobState(CPaintStream * this, ISave * pSave) */

void __thiscall CPaintStream::SavePaintBlobState(CPaintStream *this,ISave *pSave)

{
  CBasePaintBlob *this_00;
  _func_int_varargs *p_Var1;
  undefined4 *puVar2;
  Vector *pVVar3;
  int i;
  float flStreakSpeedDampenRate;
  float flMaxStreakTime;
  int power;
  undefined4 local_24;
  int count;
  
  count = (this->m_blobs).m_Size;
  (*pSave->_vptr_ISave[8])(pSave);
  (*pSave->_vptr_ISave[0xb])(pSave,&count,1);
  if (0 < count) {
                    /* Unresolved local var: EHANDLE hPaintStream@[???] */
    local_24 = 0xffffffff;
    puVar2 = (undefined4 *)
             (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.
               super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[3])(this);
    local_24 = *puVar2;
    (*pSave->_vptr_ISave[0x2e])(pSave,&local_24,1);
    power = (this->m_nPaintType).m_Value;
    (*pSave->_vptr_ISave[0xb])(pSave,&power,1);
    if (0 < count) {
      i = 0;
      do {
                    /* Unresolved local var: CBasePaintBlob * pBlob@[???] */
        this_00 = (this->m_blobs).m_Memory.m_pMemory[i];
        p_Var1 = pSave->_vptr_ISave[0x11];
        pVVar3 = CBasePaintBlob::GetPosition(this_00);
        (*p_Var1)(pSave,pVVar3);
        p_Var1 = pSave->_vptr_ISave[0x11];
        pVVar3 = CBasePaintBlob::GetVelocity(this_00);
        (*p_Var1)(pSave,pVVar3);
        flMaxStreakTime = this_00->m_flStreakTimer;
        (*pSave->_vptr_ISave[0xd])(pSave,&flMaxStreakTime,1);
        flStreakSpeedDampenRate = this_00->m_flStreakSpeedDampenRate;
        (*pSave->_vptr_ISave[0xd])(pSave,&flStreakSpeedDampenRate,1);
        i = i + 1;
      } while (i < count);
    }
  }
  (*pSave->_vptr_ISave[9])(pSave);
  return;
}


/* CPaintStream::RestorePaintBlobState at 0073acd0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void RestorePaintBlobState(CPaintStream * this, IRestore * pRestore) */

void __thiscall CPaintStream::RestorePaintBlobState(CPaintStream *this,IRestore *pRestore)

{
  int iVar1;
  int iVar2;
  int iVar3;
  CBasePaintBlob *pBlob;
  int unaff_EBX;
  int iVar4;
  CPaintStream *local_58;
  Vector local_48;
  Vector local_3c;
  ConVarRef local_30;
  float local_28;
  float local_24;
  uint local_20 [4];
  
                    /* Unresolved local var: ConVarRef blobs_paused@[???]
                       Unresolved local var: bool bOldBlobPauseState@[???] */
  ___i686_get_pc_thunk_bx();
  ConVarRef::ConVarRef(&local_30,(char *)(unaff_EBX + 0x377b2e));
  iVar1 = ((local_30.m_pConVarState)->m_Value).m_nValue;
  (*(local_30.m_pConVar)->_vptr_IConVar[2])(local_30.m_pConVar,0);
  (*pRestore->_vptr_IRestore[7])(pRestore);
  iVar2 = (*pRestore->_vptr_IRestore[0xf])(pRestore);
  if (0 < iVar2) {
    local_20[0] = 0xffffffff;
    (*pRestore->_vptr_IRestore[0x20])(pRestore,local_20,1,0);
    if (((local_20[0] == 0xffffffff) ||
        (iVar3 = (local_20[0] & 0xffff) * 0x10 + **(int **)(&DAT_0050be8e + unaff_EBX),
        *(uint *)(iVar3 + 8) != local_20[0] >> 0x10)) || (iVar3 = *(int *)(iVar3 + 4), iVar3 == 0))
    {
      local_58 = (CPaintStream *)0x0;
    }
    else {
      local_58 = (CPaintStream *)
                 ___dynamic_cast(iVar3,*(undefined4 *)(&DAT_0050beb2 + unaff_EBX),
                                 unaff_EBX + 0x5933ee,0);
    }
    iVar3 = (*pRestore->_vptr_IRestore[0xf])(pRestore);
    iVar4 = 0;
    if (local_58 == (CPaintStream *)0x0) {
      do {
        (*pRestore->_vptr_IRestore[0x16])(pRestore,&local_3c,1,0);
        (*pRestore->_vptr_IRestore[0x16])(pRestore,&local_48,1,0);
        (*pRestore->_vptr_IRestore[0x11])(pRestore,&local_24,1,0);
        (*pRestore->_vptr_IRestore[0x11])(pRestore,&local_28,1,0);
        iVar4 = iVar4 + 1;
      } while (iVar4 != iVar2);
    }
    else {
      do {
        (*pRestore->_vptr_IRestore[0x16])(pRestore,&local_3c,1,0);
        (*pRestore->_vptr_IRestore[0x16])(pRestore,&local_48,1,0);
        (*pRestore->_vptr_IRestore[0x11])(pRestore,&local_24,1,0);
        (*pRestore->_vptr_IRestore[0x11])(pRestore,&local_28,1,0);
                    /* Unresolved local var: CBasePaintBlob * pBlob@[???] */
        pBlob = PaintBlobCreate(&local_3c,&local_48,iVar3,local_24,local_28,false,0);
        AddPaintBlob(local_58,pBlob);
        iVar4 = iVar4 + 1;
      } while (iVar4 != iVar2);
    }
  }
  (*pRestore->_vptr_IRestore[8])(pRestore);
  (*(local_30.m_pConVar)->_vptr_IConVar[2])(local_30.m_pConVar,(uint)(iVar1 != 0));
  return;
}


/* _GLOBAL__I_draw_paint_server_blobs at 000aed00 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_draw_paint_server_blobs(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

