/* DWARF-guided pseudocode for game/server/portal2/baseprojectedentity.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* DataMapInit<CBaseProjectedEntity> at 000a7d80 */

datamap_t * DataMapInit<CBaseProjectedEntity>(CBaseProjectedEntity *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x5bc].m_pEntity + unaff_EBX + 3)
      == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xda37a7);
    if (iVar1 != 0) {
      *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x5bc].m_pPrev + unaff_EBX + 3) =
           unaff_EBX + 0xa084c0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x5bd].m_pEntity + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x5bd].m_SerialNumber + unaff_EBX + 3) =
           0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x5bd].m_pPrev + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x5bd].m_pNext + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x5be].m_pEntity + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x5bc].m_pNext + unaff_EBX + 3) = 0x14;
      ___cxa_guard_release(unaff_EBX + 0xda37a7);
      ___cxa_atexit(unaff_EBX + 0x993e7f,0,*(undefined4 *)(&DAT_00b9edc3 + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_NetworkStateChanged_m_takedamage_00cba8d8 + unaff_EBX + 3) =
       *(undefined4 *)(&DAT_00b9eefb + unaff_EBX);
  *(undefined4 *)((int)&PTR_NetworkStateChanged_m_lifeState_00cba8d0 + unaff_EBX + 3) = 0xb;
  *(int *)((int)&PTR_NetworkStateChanged_m_lifeState_00cba8cc + unaff_EBX + 3) =
       unaff_EBX + 0xcba92f;
  return (datamap_t *)((int)&PTR_NetworkStateChanged_m_lifeState_00cba8cc + unaff_EBX + 3);
}


/* __static_initialization_and_destruction_0 at 000a7e50 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *s2;
  SendTable *this;
  int *piVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  datamap_t *pdVar4;
  int iVar5;
  int iVar6;
  int unaff_EBX;
  longlong lVar7;
  
  lVar7 = ___i686_get_pc_thunk_bx();
  if (lVar7 == 0xffff00000001) {
    *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x54f].m_pPrev + unaff_EBX) =
         0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x54f].m_pPrev + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x54f].m_pPrev + unaff_EBX + 2) = 0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x54f].m_pPrev + unaff_EBX + 3) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x54f].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x550].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x550].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x550].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x550].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x551].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x551].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x551].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x551].m_pNext + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x552].m_pEntity + unaff_EBX)
         = 0x7f7fffff;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x552].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x552].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x552].m_pNext + unaff_EBX) =
         &UNK_00ba44a4 + unaff_EBX;
    pdVar4 = DataMapInit<CBaseProjectedEntity>((CBaseProjectedEntity *)0x0);
    *(datamap_t **)((int)DataMapInit<CAI_TrackPather>::dataDesc[0xd].flatOffset + unaff_EBX + -0x34)
         = pdVar4;
    iVar6 = unaff_EBX + 0xda3114;
    s2 = (char *)(unaff_EBX + 0xa083ed);
    *(char **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x553].m_pEntity + unaff_EBX) = s2
    ;
    this = (SendTable *)
           ((int)DataMapInit<CAI_TrackPather>::dataDesc[0xd].flatOffset + unaff_EBX + -0x30);
    *(SendTable **)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x553].m_SerialNumber + unaff_EBX) = this;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x554].m_pEntity + unaff_EBX)
         = 0xffff;
    piVar1 = *(int **)(&DAT_00b9ed90 + unaff_EBX);
    puVar2 = (undefined4 *)*piVar1;
    if (puVar2 == (undefined4 *)0x0) {
      *piVar1 = iVar6;
      *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x553].m_pPrev + unaff_EBX)
           = 0;
    }
    else {
      puVar3 = (undefined4 *)puVar2[2];
      iVar5 = _V_stricmp((char *)*puVar2,s2);
      if (iVar5 < 1) {
        while ((puVar3 != (undefined4 *)0x0 && (iVar5 = _V_stricmp((char *)*puVar3,s2), iVar5 < 1)))
        {
          puVar2 = puVar3;
          puVar3 = (undefined4 *)puVar3[2];
        }
        *(undefined4 **)
         ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x553].m_pPrev + unaff_EBX) = puVar3;
        puVar2[2] = iVar6;
      }
      else {
        *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x553].m_pPrev + unaff_EBX) =
             *piVar1;
        *piVar1 = iVar6;
      }
    }
    SendTable::SendTable(this);
    ___cxa_atexit(unaff_EBX + 0x993d8c,0,*(undefined4 *)(&DAT_00b9ecf0 + unaff_EBX));
    iVar6 = ServerClassInit<DT_BaseProjectedEntity::ignored>((ignored *)0x0);
    *(int *)((int)DataMapInit<CAI_TrackPather>::dataDesc[0xd].flatOffset + unaff_EBX + -0x1c) =
         iVar6;
  }
  return;
}


/* CBaseProjectedEntity::GetDataDescMap at 00706fc0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CBaseProjectedEntity * this) */

datamap_t * __thiscall CBaseProjectedEntity::GetDataDescMap(CBaseProjectedEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x65b698);
}


/* CBaseProjectedEntity::GetBaseMap at 00706fd0 */

datamap_t * CBaseProjectedEntity::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x53fcb4);
}


/* __tcf_0 at 00a3bc10 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x40f936)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x40f92a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x40f936));
  }
  *(undefined4 *)(unaff_EBX + 0x40f936) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40f932)) {
    if (*(int *)(unaff_EBX + 0x40f92a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20af3e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20af3e),*(int *)(unaff_EBX + 0x40f92a));
      *(undefined4 *)(unaff_EBX + 0x40f92a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f92e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40f92a);
  *(int *)(unaff_EBX + 0x40f93a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40f932)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20af3e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20af3e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40f92a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f92e) = 0;
  }
  return;
}


/* ServerClassInit<DT_BaseProjectedEntity::ignored> at 000a7710 */

int ServerClassInit<DT_BaseProjectedEntity::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x5c8].m_pNext + unaff_EBX + 2)
      == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xda3872);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x5c9].m_pNext + unaff_EBX + 2
                  ),(char *)(unaff_EBX + 0x9ae516),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        (&UNK_00005cf6 +
                        (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                        (char *)(unaff_EBX + 0x9b08c6),0,
                        (SendTable *)**(undefined4 **)(&DAT_00b9f52e + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00b9f4d2 + unaff_EBX),0x80);
      SendPropExclude((SendProp_conflict *)
                      ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x5d4].m_SerialNumber +
                      unaff_EBX + 2),(char *)(unaff_EBX + 0xa05066),(char *)(unaff_EBX + 0x9ae622));
      SendPropExclude((SendProp_conflict *)
                      ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x5d9].m_pPrev +
                      unaff_EBX + 2),(char *)(unaff_EBX + 0xa05066),&UNK_009fc21a + unaff_EBX);
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x5de].m_pNext +
                     unaff_EBX + 2),(char *)(unaff_EBX + 0x9ae622),0x500350,0xc,-1,4,0.0,-121121.125
                     ,*(SendVarProxyFn_conflict *)(&DAT_00b9fc42 + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x5e4].m_pEntity +
                     unaff_EBX + 2),&UNK_009fc21a + unaff_EBX,0x30035c,0xc,-1,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00b9fd86 + unaff_EBX),0x80);
      SendPropEHandle((SendProp_conflict *)
                      ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x5e9].m_SerialNumber +
                      unaff_EBX + 2),(char *)(unaff_EBX + 0xa08a64),0x3c0,4,-1,
                      *(SendVarProxyFn_conflict *)(&DAT_00b9f4be + unaff_EBX));
      SendPropEHandle((SendProp_conflict *)
                      ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x5ee].m_pPrev +
                      unaff_EBX + 2),(char *)(unaff_EBX + 0xa08a71),0x3c4,4,-1,
                      *(SendVarProxyFn_conflict *)(&DAT_00b9f4be + unaff_EBX));
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x5f3].m_pNext +
                     unaff_EBX + 2),(char *)(unaff_EBX + 0xa08a81),0x1003c8,0xc,-1,4,0.0,-121121.125
                     ,*(SendVarProxyFn_conflict *)(&DAT_00b9f51e + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     (&UNK_00005f96 +
                     (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                     (char *)(unaff_EBX + 0xa08a99),0x1003d4,0xc,-1,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00b9f51e + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x5fe].m_SerialNumber +
                     unaff_EBX + 2),(char *)(unaff_EBX + 0xa08ab7),0x1003e0,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00b9f51e + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x603].m_pPrev +
                     unaff_EBX + 2),(char *)(unaff_EBX + 0xa08ace),0x1003ec,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00b9f51e + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x608].m_pNext +
                     unaff_EBX + 2),(char *)(unaff_EBX + 0xa08aeb),0x1003f8,0xc,-1,4,0.0,-121121.125
                     ,*(SendVarProxyFn_conflict *)(&DAT_00b9f51e + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x60e].m_pEntity +
                     unaff_EBX + 2),(char *)(unaff_EBX + 0xa08afb),0x100404,0xc,-1,4,0.0,-121121.125
                     ,*(SendVarProxyFn_conflict *)(&DAT_00b9f51e + unaff_EBX),0x80);
      SendPropEHandle((SendProp_conflict *)
                      ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x613].m_SerialNumber +
                      unaff_EBX + 2),(char *)(unaff_EBX + 0xa08b09),0x410,4,-1,
                      *(SendVarProxyFn_conflict *)(&DAT_00b9f4be + unaff_EBX));
      SendPropEHandle((SendProp_conflict *)
                      ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x618].m_pPrev +
                      unaff_EBX + 2),(char *)(unaff_EBX + 0xa04bf2),0x414,4,-1,
                      *(SendVarProxyFn_conflict *)(&DAT_00b9f4be + unaff_EBX));
      SendPropInt((SendProp_conflict *)
                  ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x61d].m_pNext + unaff_EBX + 2
                  ),(char *)(unaff_EBX + 0xa08b19),0x41c,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      ___cxa_guard_release(unaff_EBX + 0xda3872);
      ___cxa_atexit(unaff_EBX + 0x9943a2,0,*(undefined4 *)(&DAT_00b9f436 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)
             ((int)DataMapInit<CAI_TrackPather>::dataDesc[0x2a].flatOffset + unaff_EBX + -0x2a),
             (SendProp_conflict *)
             (&UNK_00005cf6 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
             0x10,*(char **)(&DAT_00c20126 + unaff_EBX));
  return 1;
}


/* CBaseProjectedEntity::GetServerClass at 00706fe0 */

/* DWARF original prototype: ServerClass * GetServerClass(CBaseProjectedEntity * this) */

ServerClass * __thiscall CBaseProjectedEntity::GetServerClass(CBaseProjectedEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x743f90);
}


/* CBaseProjectedEntity::YouForgotToImplementOrDeclareServerClass at 00706ff0 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CBaseProjectedEntity *
   this) */

int __thiscall
CBaseProjectedEntity::YouForgotToImplementOrDeclareServerClass(CBaseProjectedEntity *this)

{
  return 0;
}


/* __tcf_2 at 00a3bbf0 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x38c3f0),in_stack_00000008);
  return;
}


/* __tcf_1 at 00a3bac0 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(&DAT_0040fa13 + unaff_EBX))(&DAT_0040fa13 + unaff_EBX);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f9bf))(unaff_EBX + 0x40f9bf);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f96b))(unaff_EBX + 0x40f96b);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f917))(unaff_EBX + 0x40f917);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f8c3))(unaff_EBX + 0x40f8c3);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f86f))(unaff_EBX + 0x40f86f);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f81b))(unaff_EBX + 0x40f81b);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f7c7))(unaff_EBX + 0x40f7c7);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f773))(unaff_EBX + 0x40f773);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f71f))(unaff_EBX + 0x40f71f);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f6cb))(unaff_EBX + 0x40f6cb);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f677))(unaff_EBX + 0x40f677);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f623))(unaff_EBX + 0x40f623);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f5cf))(unaff_EBX + 0x40f5cf);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f57b))(unaff_EBX + 0x40f57b);
  (*(code *)**(undefined4 **)(CC_Find_Ent + unaff_EBX + 7))(unaff_EBX + 0x40f527);
                    /* WARNING: Could not recover jumptable at 0x00a3bbee. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f4d3))();
  return;
}


/* CBaseProjectedEntity::CBaseProjectedEntity at 00707030 */

/* DWARF original prototype: void CBaseProjectedEntity(CBaseProjectedEntity * this) */

void __thiscall CBaseProjectedEntity::CBaseProjectedEntity(CBaseProjectedEntity *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::CBaseEntity(&this->super_CBaseEntity,false);
  (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(&UNK_005c04a7 + unaff_EBX);
  (this->m_hHitPortal).
  super_CNetworkVarBase<CBaseHandle,CBaseProjectedEntity::NetworkVar_m_hHitPortal>.m_Value.m_Index =
       0xffffffff;
  (this->m_hSourcePortal).
  super_CNetworkVarBase<CBaseHandle,CBaseProjectedEntity::NetworkVar_m_hSourcePortal>.m_Value.
  m_Index = 0xffffffff;
  (this->m_hChildSegment).
  super_CNetworkVarBase<CBaseHandle,CBaseProjectedEntity::NetworkVar_m_hChildSegment>.m_Value.
  m_Index = 0xffffffff;
  (this->m_hPlacementHelper).
  super_CNetworkVarBase<CBaseHandle,CBaseProjectedEntity::NetworkVar_m_hPlacementHelper>.m_Value.
  m_Index = 0xffffffff;
  return;
}


/* CBaseProjectedEntity::CBaseProjectedEntity at 007075a0 */

/* DWARF original prototype: void CBaseProjectedEntity(CBaseProjectedEntity * this,
   CBaseProjectedEntity * this) */

void __thiscall
CBaseProjectedEntity::CBaseProjectedEntity(CBaseProjectedEntity *this,CBaseProjectedEntity *this_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::CBaseEntity(&this->super_CBaseEntity,false);
  (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5bff37);
  (this->m_hHitPortal).
  super_CNetworkVarBase<CBaseHandle,CBaseProjectedEntity::NetworkVar_m_hHitPortal>.m_Value.m_Index =
       0xffffffff;
  (this->m_hSourcePortal).
  super_CNetworkVarBase<CBaseHandle,CBaseProjectedEntity::NetworkVar_m_hSourcePortal>.m_Value.
  m_Index = 0xffffffff;
  (this->m_hChildSegment).
  super_CNetworkVarBase<CBaseHandle,CBaseProjectedEntity::NetworkVar_m_hChildSegment>.m_Value.
  m_Index = 0xffffffff;
  (this->m_hPlacementHelper).
  super_CNetworkVarBase<CBaseHandle,CBaseProjectedEntity::NetworkVar_m_hPlacementHelper>.m_Value.
  m_Index = 0xffffffff;
  return;
}


/* CBaseProjectedEntity::Spawn at 00707000 */

/* DWARF original prototype: void Spawn(CBaseProjectedEntity * this) */

void __thiscall CBaseProjectedEntity::Spawn(CBaseProjectedEntity *this)

{
  CBaseEntity::Spawn(&this->super_CBaseEntity);
                    /* WARNING: Could not recover jumptable at 0x00707022. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0xc5])();
  return;
}


/* CBaseProjectedEntity::OnRestore at 00707260 */

/* DWARF original prototype: void OnRestore(CBaseProjectedEntity * this) */

void __thiscall CBaseProjectedEntity::OnRestore(CBaseProjectedEntity *this)

{
  undefined1 *puVar1;
  ushort uVar2;
  CBaseEdict *this_00;
  ushort *puVar3;
  int *piVar4;
  int iVar5;
  CBaseEntity *pCVar6;
  CBaseEntity *pCVar7;
  uint *puVar8;
  undefined4 uVar9;
  int iVar10;
  IChangeInfoAccessor *pIVar11;
  uint uVar12;
  uint uVar13;
  int unaff_EBX;
  _func_int_varargs *p_Var14;
  bool bVar15;
  uint local_60;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::OnRestore(&this->super_CBaseEntity);
  if (this->m_bCreatePlacementHelper == false) {
    return;
  }
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
  iVar10 = **(int **)(unaff_EBX + 0x53f8fe);
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
  uVar12 = (this->m_hPlacementHelper).
           super_CNetworkVarBase<CBaseHandle,CBaseProjectedEntity::NetworkVar_m_hPlacementHelper>.
           m_Value.m_Index;
  if (uVar12 == 0xffffffff) {
LAB_007072b2:
    bVar15 = true;
LAB_007072b6:
    pCVar6 = CreateEntityByName((char *)(ConceptIDLessFunc + unaff_EBX + 2),-1,true);
    uVar12 = (this->m_hPlacementHelper).
             super_CNetworkVarBase<CBaseHandle,CBaseProjectedEntity::NetworkVar_m_hPlacementHelper>.
             m_Value.m_Index;
    if ((uVar12 == 0xffffffff) ||
       (iVar10 = (uVar12 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x53f8fe),
       *(uint *)(iVar10 + 8) != uVar12 >> 0x10)) {
      pCVar7 = (CBaseEntity *)0x0;
    }
    else {
      pCVar7 = *(CBaseEntity **)(iVar10 + 4);
    }
    if (pCVar7 != pCVar6) {
      if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
        this_00 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
        if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
          this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
          pIVar11 = CBaseEdict::GetChangeAccessor(this_00);
          puVar3 = (ushort *)**(undefined4 **)(unaff_EBX + 0x53f8fa);
          if (pIVar11->m_iChangeInfoSerialNumber == *puVar3) {
            uVar12 = (uint)pIVar11->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar2 = puVar3[uVar12 * 0x14 + 0x14];
            if (uVar2 == 0) {
LAB_00707549:
              puVar3[(uint)uVar2 + uVar12 * 0x14 + 1] = 0x414;
              puVar3[uVar12 * 0x14 + 0x14] = uVar2 + 1;
            }
            else if (puVar3[uVar12 * 0x14 + 1] != 0x414) {
              local_60 = 0;
              do {
                uVar13 = local_60 + 1;
                local_60 = uVar13 & 0xffff;
                if ((ushort)uVar13 == uVar2) {
                  if (uVar2 != 0x13) goto LAB_00707549;
                  pIVar11->m_iChangeInfoSerialNumber = 0;
                  this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar3[uVar12 * 0x14 + local_60 + 1] != 0x414);
            }
          }
          else if (puVar3[0x7d1] == 100) {
            pIVar11->m_iChangeInfoSerialNumber = 0;
            this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar11->m_iChangeInfo = puVar3[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x53f8fa) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x53f8fa) + 0xfa2) + 1;
            piVar4 = *(int **)(unaff_EBX + 0x53f8fa);
            pIVar11->m_iChangeInfoSerialNumber = *(ushort *)*piVar4;
            iVar10 = *piVar4 + (uint)pIVar11->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar10 + 2) = 0x414;
            *(undefined2 *)(iVar10 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      if (pCVar6 == (CBaseEntity *)0x0) {
        (this->m_hPlacementHelper).
        super_CNetworkVarBase<CBaseHandle,CBaseProjectedEntity::NetworkVar_m_hPlacementHelper>.
        m_Value.m_Index = 0xffffffff;
        p_Var14 = (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.
                  super_IHandleEntity._vptr_IHandleEntity[0xd2];
        uVar9 = 0;
        goto LAB_00707383;
      }
      puVar8 = (uint *)(*(pCVar6->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                         _vptr_IHandleEntity[3])(pCVar6);
      uVar12 = *puVar8;
      (this->m_hPlacementHelper).
      super_CNetworkVarBase<CBaseHandle,CBaseProjectedEntity::NetworkVar_m_hPlacementHelper>.m_Value
      .m_Index = uVar12;
    }
    p_Var14 = (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity
              ._vptr_IHandleEntity[0xd2];
    iVar10 = **(int **)(unaff_EBX + 0x53f8fe);
    if ((uVar12 == 0xffffffff) ||
       (uVar13 = uVar12 & 0xffff, *(uint *)(iVar10 + 8 + uVar13 * 0x10) != uVar12 >> 0x10)) {
      uVar9 = 0;
      goto LAB_00707383;
    }
  }
  else {
    uVar13 = uVar12 & 0xffff;
    iVar5 = uVar13 * 0x10 + iVar10;
    if (*(uint *)(iVar5 + 8) != uVar12 >> 0x10) goto LAB_007072b2;
    bVar15 = *(int *)(iVar5 + 4) == 0;
    if (bVar15) goto LAB_007072b6;
    p_Var14 = (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity
              ._vptr_IHandleEntity[0xd2];
  }
  uVar9 = *(undefined4 *)(iVar10 + 4 + uVar13 * 0x10);
LAB_00707383:
  (*p_Var14)(this,uVar9);
  if (bVar15) {
    uVar12 = (this->m_hPlacementHelper).
             super_CNetworkVarBase<CBaseHandle,CBaseProjectedEntity::NetworkVar_m_hPlacementHelper>.
             m_Value.m_Index;
    if ((uVar12 == 0xffffffff) ||
       (iVar10 = (uVar12 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x53f8fe),
       *(uint *)(iVar10 + 8) != uVar12 >> 0x10)) {
      pCVar6 = (CBaseEntity *)0x0;
    }
    else {
      pCVar6 = *(CBaseEntity **)(iVar10 + 4);
    }
    DispatchSpawn(pCVar6,true);
  }
  return;
}


/* CBaseProjectedEntity::UpdateOnRemove at 007071b0 */

/* DWARF original prototype: void UpdateOnRemove(CBaseProjectedEntity * this) */

void __thiscall CBaseProjectedEntity::UpdateOnRemove(CBaseProjectedEntity *this)

{
  int *piVar1;
  uint uVar2;
  CBaseEntity *pCVar3;
  int iVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  piVar1 = *(int **)(&DAT_0053f9a8 + unaff_EBX);
  uVar2 = (this->m_hChildSegment).
          super_CNetworkVarBase<CBaseHandle,CBaseProjectedEntity::NetworkVar_m_hChildSegment>.
          m_Value.m_Index;
  if (((uVar2 != 0xffffffff) &&
      (iVar4 = (uVar2 & 0xffff) * 0x10 + *piVar1, *(uint *)(iVar4 + 8) == uVar2 >> 0x10)) &&
     (pCVar3 = *(CBaseEntity **)(iVar4 + 4), pCVar3 != (CBaseEntity *)0x0)) {
    UTIL_Remove(pCVar3);
  }
  uVar2 = (this->m_hPlacementHelper).
          super_CNetworkVarBase<CBaseHandle,CBaseProjectedEntity::NetworkVar_m_hPlacementHelper>.
          m_Value.m_Index;
  if (((uVar2 != 0xffffffff) &&
      (iVar4 = (uVar2 & 0xffff) * 0x10 + *piVar1, *(uint *)(iVar4 + 8) == uVar2 >> 0x10)) &&
     (pCVar3 = *(CBaseEntity **)(iVar4 + 4), pCVar3 != (CBaseEntity *)0x0)) {
    UTIL_Remove(pCVar3);
  }
  CBaseEntity::UpdateOnRemove(&this->super_CBaseEntity);
  return;
}


/* CBaseProjectedEntity::PlacePlacementHelper at 007070a0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void PlacePlacementHelper(CBaseProjectedEntity * this,
   CInfoPlacementHelper * pHelper) */

void __thiscall
CBaseProjectedEntity::PlacePlacementHelper(CBaseProjectedEntity *this,CInfoPlacementHelper *pHelper)

{
  float *pfVar1;
  int unaff_EBX;
  Vector local_40;
  float local_34;
  float local_30;
  float local_2c;
  float local_28;
  float local_24;
  float local_20;
  
                    /* Unresolved local var: Vector vecPosition@[???] */
  ___i686_get_pc_thunk_bx();
  pfVar1 = (float *)(*(this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.
                      super_IHandleEntity._vptr_IHandleEntity[0xcc])(this);
  local_40.z = pfVar1[2];
  local_40.y = pfVar1[1];
  local_40.x = *pfVar1;
  if (pHelper != (CInfoPlacementHelper *)0x0) {
                    /* Unresolved local var: Vector vForward@[???] */
    (*(this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
      _vptr_IHandleEntity[0x8e])(this,&local_28,0,0);
                    /* Unresolved local var: Vector res@[???] */
    local_34 = local_28;
    local_30 = local_24;
    local_2c = local_20;
                    /* Unresolved local var: Vector res@[???] */
    local_40.x = local_40.x - local_28;
    local_40.y = local_40.y - local_24;
    local_40.z = local_40.z - local_20;
    CBaseEntity::SetAbsOrigin((CBaseEntity *)pHelper,&local_40);
    (*(pHelper->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
      super_IHandleEntity._vptr_IHandleEntity[0x20])
              (pHelper,unaff_EBX + 0x3853e6,unaff_EBX + 0x3ac31e);
    (*(pHelper->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
      super_IHandleEntity._vptr_IHandleEntity[0x20])
              (pHelper,unaff_EBX + 0x3ac322,unaff_EBX + 0x34fbce);
    (*(pHelper->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
      super_IHandleEntity._vptr_IHandleEntity[0x13])(pHelper,this);
  }
  return;
}


/* _GLOBAL__I__ZN20CBaseProjectedEntity9m_DataMapE at 000a8010 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN20CBaseProjectedEntity9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

