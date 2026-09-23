/* DWARF-guided pseudocode for game/server/portal2/paint_sprayer.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* ServerClassInit<DT_PaintSprayer::ignored> at 000adb70 */

int ServerClassInit<DT_PaintSprayer::ignored>(ignored *param_1)

{
  SendVarProxyFn_conflict varProxy;
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&UNK_00002142)[(int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xd9fd22);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  (&UNK_00002162 +
                  (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                  (char *)(unaff_EBX + 0x9a80b6),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x21b].m_pEntity +
                        unaff_EBX + 2),(char *)(unaff_EBX + 0x9aa466),0,
                        (SendTable *)**(undefined4 **)(&DAT_00b990ce + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00b99072 + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x220].m_SerialNumber +
                     unaff_EBX + 2),&UNK_009a81c2 + unaff_EBX,0x500350,0xc,-1,0x40002,0.0,
                     -121121.125,*(SendVarProxyFn_conflict *)(&DAT_00b997e2 + unaff_EBX),0x80);
      SendPropEHandle((SendProp_conflict *)
                      ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x225].m_pPrev +
                      unaff_EBX + 2),&UNK_009a81ce + unaff_EBX,0x16c,4,-1,
                      *(SendVarProxyFn_conflict *)(&DAT_00b9905e + unaff_EBX));
      SendPropQAngles((SendProp_conflict *)
                      ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x22a].m_pNext +
                      unaff_EBX + 2),(char *)(unaff_EBX + 0x9f5dba),0x30035c,0xc,0xd,0x40000,
                      *(SendVarProxyFn_conflict *)(&DAT_00b99926 + unaff_EBX),0x80);
      SendPropBool((SendProp_conflict *)
                   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x230].m_pEntity +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0x9ea20e),0x3d4,1);
      SendPropBool((SendProp_conflict *)
                   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x235].m_SerialNumber +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0xa049ff),0x3d5,1);
      SendPropInt((SendProp_conflict *)
                  ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x23a].m_pPrev + unaff_EBX + 2
                  ),(char *)(unaff_EBX + 0xa035b9),0x3d8,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      varProxy = *(SendVarProxyFn_conflict *)(&DAT_00b990ca + unaff_EBX);
      SendPropFloat((SendProp_conflict *)
                    ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x23f].m_pNext +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0xa04a0a),0x3dc,4,0x20,0,0.0,-121121.125,
                    varProxy,0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x245].m_pEntity +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x9ea914),1000,4,0x20,0,0.0,-121121.125,
                    varProxy,0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x24a].m_SerialNumber +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x9ea92a),0x3ec,4,0x20,0,0.0,-121121.125,
                    varProxy,0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x24f].m_pPrev +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0xa04a1f),0x3e0,4,0x20,0,0.0,-121121.125,
                    varProxy,0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x254].m_pNext +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0xa04a34),0x3e4,4,0x20,0,0.0,-121121.125,
                    varProxy,0x80);
      SendPropFloat((SendProp_conflict *)
                    (&UNK_000025a6 +
                    (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                    (char *)(unaff_EBX + 0xa04a48),0x3f0,4,0x20,0,0.0,-121121.125,varProxy,0x80);
      SendPropFloat((SendProp_conflict *)
                    (&UNK_000025fa +
                    (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                    (char *)(unaff_EBX + 0xa04a5d),0x3f4,4,0x20,0,0.0,-121121.125,varProxy,0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x264].m_pPrev +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0xa04a6f),0x3f8,4,0x20,0,0.0,-121121.125,
                    varProxy,0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x269].m_pNext +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0xa04a81),0x3fc,4,0x20,0,0.0,-121121.125,
                    varProxy,0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x26f].m_pEntity +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0xa04a9a),0x400,4,0x20,0,0.0,-121121.125,
                    varProxy,0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x274].m_SerialNumber +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x9ba074),0x404,4,0x20,0,0.0,-121121.125,
                    varProxy,0x80);
      SendPropEHandle((SendProp_conflict *)
                      (&UNK_0000279e +
                      (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                      (char *)(unaff_EBX + 0xa08a5c),0x408,4,-1,
                      *(SendVarProxyFn_conflict *)(&DAT_00b9905e + unaff_EBX));
      ___cxa_guard_release(unaff_EBX + 0xd9fd22);
      ___cxa_atexit(unaff_EBX + 0x990132,0,*(undefined4 *)(&DAT_00b98fd6 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)
             ((int)DataMapInit<CAI_GoalEntity>::dataDesc[10].flatOffset + unaff_EBX + -0x1a),
             (SendProp_conflict *)
             ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x21b].m_pEntity + unaff_EBX + 2),
             0x13,*(char **)(&DAT_00c2014a + unaff_EBX));
  return 1;
}


/* __static_initialization_and_destruction_0 at 000ae550 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *s2;
  SendTable *this;
  int *piVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  int iVar4;
  int iVar5;
  datamap_t *pdVar6;
  IEntityFactoryDictionary *pIVar7;
  int unaff_EBX;
  longlong lVar8;
  
  lVar8 = ___i686_get_pc_thunk_bx();
  if (lVar8 == 0xffff00000001) {
    *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x16d].m_pPrev + unaff_EBX) =
         0;
    (&UNK_000016dd)[(int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
    (&UNK_000016de)[(int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
    (&UNK_000016df)[(int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
    *(undefined4 *)
     (__static_initialization_and_destruction_0 +
     (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
    *(undefined4 *)
     (__static_initialization_and_destruction_0 +
     (int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x16e].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x16e].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x16e].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x16f].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x16f].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x16f].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x16f].m_pNext + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x170].m_pEntity + unaff_EBX)
         = 0x7f7fffff;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x170].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x170].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x170].m_pNext + unaff_EBX) =
         &UNK_00b9dda4 + unaff_EBX;
    iVar5 = unaff_EBX + 0xd9f2f4;
    s2 = (char *)(unaff_EBX + 0xa040ce);
    *(char **)(&DAT_00001714 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
         s2;
    this = (SendTable *)
           ((int)DataMapInit<CAI_RadialLinkController>::dataDesc[2].flatOffset + unaff_EBX + -0x20);
    *(SendTable **)
     (&DAT_00001718 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = this;
    *(undefined4 *)
     (_GLOBAL__I_srand + (int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX
     ) = 0xffff;
    piVar1 = *(int **)(&DAT_00b98690 + unaff_EBX);
    puVar2 = (undefined4 *)*piVar1;
    if (puVar2 == (undefined4 *)0x0) {
      *piVar1 = iVar5;
      *(undefined4 *)
       (&UNK_0000171c + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
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
         (&UNK_0000171c + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
             puVar3;
        puVar2[2] = iVar5;
      }
      else {
        *(int *)(&UNK_0000171c + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4)
             = *piVar1;
        *piVar1 = iVar5;
      }
    }
    SendTable::SendTable(this);
    ___cxa_atexit(unaff_EBX + 0x98f78c,0,*(undefined4 *)(&DAT_00b985f0 + unaff_EBX));
    iVar5 = ServerClassInit<DT_PaintSprayer::ignored>((ignored *)0x0);
    *(int *)((int)DataMapInit<CAI_RadialLinkController>::dataDesc[2].flatOffset + unaff_EBX + -0xc)
         = iVar5;
    pdVar6 = DataMapInit<CPaintSprayer>((CPaintSprayer *)0x0);
    *(datamap_t **)
     ((int)DataMapInit<CAI_RadialLinkController>::dataDesc[2].flatOffset + unaff_EBX + -8) = pdVar6;
    *(undefined **)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x172].m_SerialNumber + unaff_EBX) =
         &UNK_00c1f78c + unaff_EBX;
    pIVar7 = EntityFactoryDictionary();
    (**pIVar7->_vptr_IEntityFactoryDictionary)(pIVar7,unaff_EBX + 0xd9f308,unaff_EBX + 0xa040dc);
  }
  return;
}


/* CPaintSprayer::GetServerClass at 00737450 */

/* DWARF original prototype: ServerClass * GetServerClass(CPaintSprayer * this) */

ServerClass * __thiscall CPaintSprayer::GetServerClass(CPaintSprayer *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x716400);
}


/* CPaintSprayer::YouForgotToImplementOrDeclareServerClass at 00737460 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CPaintSprayer * this) */

int __thiscall CPaintSprayer::YouForgotToImplementOrDeclareServerClass(CPaintSprayer *this)

{
  return 0;
}


/* __tcf_2 at 00a3dcf0 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(&UNK_0038b400 + extraout_ECX),in_stack_00000008);
  return;
}


/* __tcf_0 at 00a3dcb0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  undefined4 *puVar1;
  
  ___i686_get_pc_thunk_bx();
  puVar1 = (undefined4 *)(unaff_EBX + 0x41023e);
  do {
    (**(code **)*puVar1)(puVar1);
    puVar1 = puVar1 + -0x15;
  } while (puVar1 != (undefined4 *)(unaff_EBX + 0x40fbae));
  return;
}


/* DataMapInit<CPaintSprayer> at 000ae330 */

datamap_t * DataMapInit<CPaintSprayer>(CPaintSprayer *param_1)

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
  if ((*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x194].m_pPrev + unaff_EBX) ==
       '\0') && (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd9f52c), iVar2 != 0)) {
    *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x195].m_pPrev + unaff_EBX) =
         unaff_EBX + 0xa042ee;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x196].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x196].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x196].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x196].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x197].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x195].m_pNext + unaff_EBX) =
         0xd;
    ___cxa_guard_release(unaff_EBX + 0xd9f52c);
    ___cxa_atexit(unaff_EBX + 0x98f9cc,0,*(undefined4 *)(&DAT_00b98810 + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_CanStandOn_00cb768c + unaff_EBX) =
       *(undefined4 *)(&DAT_00b98948 + unaff_EBX);
  if ((*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x195].m_pEntity + unaff_EBX) ==
       '\0') && (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd9f534), iVar2 != 0)) {
    pcVar3 = operator_new___(*(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x195].
                                            m_pNext + unaff_EBX) + 0x12);
    _strcpy(pcVar3,*(char **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x195].m_pPrev +
                             unaff_EBX));
    sVar4 = _strlen(pcVar3);
    builtin_strncpy(pcVar3 + sVar4,"PaintSprayerThink",0x12);
    iVar1 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x196].m_pNext + unaff_EBX);
    iVar2 = iVar1 + 1;
    iVar7 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x196].m_SerialNumber +
                    unaff_EBX);
    if (iVar7 < iVar2) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)
                 ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x196].m_pEntity + unaff_EBX),
                 iVar2 - iVar7);
    }
    iVar5 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x196].m_pNext + unaff_EBX)
            + 1;
    *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x196].m_pNext + unaff_EBX) = iVar5;
    iVar7 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x196].m_pEntity + unaff_EBX
                    );
    *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x197].m_pEntity + unaff_EBX) =
         iVar7;
    iVar5 = (iVar5 - iVar1) + -1;
    if (0 < iVar5) {
      _V_memmove((void *)(iVar7 + iVar2 * 4),(void *)(iVar7 + iVar1 * 4),iVar5 * 4);
      iVar7 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x196].m_pEntity +
                      unaff_EBX);
    }
    puVar6 = (undefined4 *)(iVar1 * 4 + iVar7);
    if (puVar6 != (undefined4 *)0x0) {
      *puVar6 = pcVar3;
    }
    *(char **)((int)&PTR_NetworkStateChanged_m_vecVelocity_00cb7ba0 + unaff_EBX) = pcVar3;
    ___cxa_guard_release(unaff_EBX + 0xd9f534);
  }
  *(undefined4 *)((int)&PTR_OnEntityEvent_00cb7684 + unaff_EBX) = 0x14;
  *(int *)((int)&PTR_ChangeTeam_00cb7680 + unaff_EBX) = (int)&PTR_FireBullets_00cb76dc + unaff_EBX;
  return (datamap_t *)((int)&PTR_ChangeTeam_00cb7680 + unaff_EBX);
}


/* CPaintSprayer::GetDataDescMap at 00737470 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPaintSprayer * this) */

datamap_t * __thiscall CPaintSprayer::GetDataDescMap(CPaintSprayer *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x62e54c);
}


/* CPaintSprayer::GetBaseMap at 00737480 */

datamap_t * CPaintSprayer::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(&DAT_0050f804 + extraout_ECX);
}


/* __tcf_1 at 00a3dd10 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x40fb76)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x40fb6a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x40fb76));
  }
  *(undefined4 *)(unaff_EBX + 0x40fb76) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40fb72)) {
    if (*(int *)(unaff_EBX + 0x40fb6a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x208e3e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x208e3e),*(int *)(unaff_EBX + 0x40fb6a));
      *(undefined4 *)(unaff_EBX + 0x40fb6a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40fb6e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40fb6a);
  *(int *)(unaff_EBX + 0x40fb7a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40fb72)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x208e3e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x208e3e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40fb6a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40fb6e) = 0;
  }
  return;
}


/* CPaintSprayer::CPaintSprayer at 00738eb0 */

/* DWARF original prototype: void CPaintSprayer(CPaintSprayer * this) */

void __thiscall CPaintSprayer::CPaintSprayer(CPaintSprayer *this)

{
  undefined1 *puVar1;
  uint uVar2;
  int iVar3;
  ushort uVar4;
  CBaseEdict *pCVar5;
  int *piVar6;
  ushort *puVar7;
  IChangeInfoAccessor *pIVar8;
  uint uVar9;
  int unaff_EBX;
  uint uVar10;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::CBaseEntity(&this->super_CBaseEntity,false);
  (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x594aea);
  this->m_bStartActive = false;
  this->m_flLastThinkTime = 0.0;
  this->m_flAccumulatedTime = 0.0;
  this->m_nBlobSpawned = 0;
  (this->m_bActive).m_Value = false;
  if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
    pCVar5 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
    if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
      pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
      pIVar8 = CBaseEdict::GetChangeAccessor(pCVar5);
      piVar6 = *(int **)(unaff_EBX + 0x50dcaa);
      puVar7 = (ushort *)*piVar6;
      if (pIVar8->m_iChangeInfoSerialNumber == *puVar7) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        uVar10 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar4 = puVar7[uVar10 * 0x14 + 0x14];
        if (uVar4 == 0) {
LAB_00739176:
          puVar7[(uint)uVar4 + uVar10 * 0x14 + 1] = 0x3d4;
          puVar7[uVar10 * 0x14 + 0x14] = uVar4 + 1;
        }
        else if (puVar7[uVar10 * 0x14 + 1] != 0x3d4) {
          uVar9 = 0;
          do {
            uVar2 = uVar9 + 1;
            uVar9 = uVar2 & 0xffff;
            if ((ushort)uVar2 == uVar4) {
              if (uVar4 != 0x13) goto LAB_00739176;
              goto LAB_00739198;
            }
          } while (puVar7[uVar10 * 0x14 + uVar9 + 1] != 0x3d4);
        }
      }
      else if (puVar7[0x7d1] == 100) {
LAB_00739198:
        pIVar8->m_iChangeInfoSerialNumber = 0;
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar8->m_iChangeInfo = puVar7[0x7d1];
        *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
        pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
        iVar3 = *piVar6 + (uint)pIVar8->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar3 + 2) = 0x3d4;
        *(undefined2 *)(iVar3 + 0x28) = 1;
      }
    }
  }
  else {
    puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  (this->m_flStartTime).m_Value = 0.0;
  if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
    pCVar5 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
    if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
      pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
      pIVar8 = CBaseEdict::GetChangeAccessor(pCVar5);
      piVar6 = *(int **)(unaff_EBX + 0x50dcaa);
      puVar7 = (ushort *)*piVar6;
      if (pIVar8->m_iChangeInfoSerialNumber == *puVar7) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        uVar10 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar4 = puVar7[uVar10 * 0x14 + 0x14];
        if (uVar4 == 0) {
LAB_007390d6:
          puVar7[(uint)uVar4 + uVar10 * 0x14 + 1] = 0x404;
          puVar7[uVar10 * 0x14 + 0x14] = uVar4 + 1;
        }
        else if (puVar7[uVar10 * 0x14 + 1] != 0x404) {
          uVar9 = 0;
          do {
            uVar2 = uVar9 + 1;
            uVar9 = uVar2 & 0xffff;
            if ((ushort)uVar2 == uVar4) {
              if (uVar4 != 0x13) goto LAB_007390d6;
              goto LAB_007390f8;
            }
          } while (puVar7[uVar10 * 0x14 + uVar9 + 1] != 0x404);
        }
      }
      else if (puVar7[0x7d1] == 100) {
LAB_007390f8:
        pIVar8->m_iChangeInfoSerialNumber = 0;
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar8->m_iChangeInfo = puVar7[0x7d1];
        *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
        pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
        iVar3 = *piVar6 + (uint)pIVar8->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar3 + 2) = 0x404;
        *(undefined2 *)(iVar3 + 0x28) = 1;
      }
    }
  }
  else {
    puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  (this->m_hPaintStream).super_CNetworkVarBase<CBaseHandle,CPaintSprayer::NetworkVar_m_hPaintStream>
  .m_Value.m_Index = 0xffffffff;
  return;
}


/* CPaintSprayer::CPaintSprayer at 007391d0 */

/* DWARF original prototype: void CPaintSprayer(CPaintSprayer * this, CPaintSprayer * this) */

void __thiscall CPaintSprayer::CPaintSprayer(CPaintSprayer *this,CPaintSprayer *this_1)

{
  CPaintSprayer(this);
  return;
}


/* CEntityFactory<CPaintSprayer>::Create at 0073a020 */

/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CPaintSprayer> * this, char
   * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CPaintSprayer>::Create(CEntityFactory<CPaintSprayer> *this,char *pClassName)

{
  CPaintSprayer *this_00;
  
                    /* Unresolved local var: CPaintSprayer * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x40c);
  CPaintSprayer::CPaintSprayer(this_00);
  (*(this_00->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x1d])(this_00,pClassName);
  return &(this_00->super_CBaseEntity).m_Network.super_IServerNetworkable;
}


/* CPaintSprayer::~CPaintSprayer at 00737660 */

/* DWARF original prototype: void ~CPaintSprayer(CPaintSprayer * this, int __in_chrg) */

void __thiscall CPaintSprayer::~CPaintSprayer(CPaintSprayer *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x596337);
  CBaseEntity::~CBaseEntity(&this->super_CBaseEntity,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CPaintSprayer::~CPaintSprayer at 007376a0 */

/* DWARF original prototype: void ~CPaintSprayer(CPaintSprayer * this, int __in_chrg) */

void __thiscall CPaintSprayer::~CPaintSprayer(CPaintSprayer *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x596300);
  CBaseEntity::~CBaseEntity(&this->super_CBaseEntity,__in_chrg);
  return;
}


/* CPaintSprayer::~CPaintSprayer at 007376c0 */

/* DWARF original prototype: void ~CPaintSprayer(CPaintSprayer * this, int __in_chrg) */

void __thiscall CPaintSprayer::~CPaintSprayer(CPaintSprayer *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x5962e0);
  CBaseEntity::~CBaseEntity(&this->super_CBaseEntity,__in_chrg);
  return;
}


/* CPaintSprayer::Precache at 007375d0 */

/* DWARF original prototype: void Precache(CPaintSprayer * this) */

void __thiscall CPaintSprayer::Precache(CPaintSprayer *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x37aee8));
  CBaseEntity::PrecacheModel(&UNK_0037af08 + unaff_EBX);
  CBaseEntity::PrecacheModel(&UNK_0037af34 + unaff_EBX);
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x37af60));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x32468e));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x3246a4));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x37af8c));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x3246ba));
  return;
}


/* CPaintSprayer::Spawn at 007397e0 */

/* DWARF original prototype: void Spawn(CPaintSprayer * this) */

void __thiscall CPaintSprayer::Spawn(CPaintSprayer *this)

{
  undefined1 *puVar1;
  ushort uVar2;
  vec_t vVar3;
  CBaseEdict *pCVar4;
  ushort *puVar5;
  int *piVar6;
  BASEPTR func;
  int iVar7;
  CBaseEntity *pEntity;
  ulong *puVar8;
  IChangeInfoAccessor *pIVar9;
  uint uVar10;
  uint uVar11;
  int unaff_EBX;
  uint local_98;
  uint local_94;
  uint local_90;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::Spawn(&this->super_CBaseEntity);
  (*(this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x1a])(this);
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x2361);
  CBaseEntity::ThinkSet(&this->super_CBaseEntity,func,0.0,(char *)0x0);
  CBaseEntity::SetNextThink(&this->super_CBaseEntity,-1.0,(char *)0x0);
  if (this->m_bStartActive != false) {
    StartPaintSpray(this);
  }
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
  uVar10 = (this->m_hPaintStream).
           super_CNetworkVarBase<CBaseHandle,CPaintSprayer::NetworkVar_m_hPaintStream>.m_Value.
           m_Index;
  if ((((uVar10 == 0xffffffff) ||
       (iVar7 = (uVar10 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x50d37b),
       *(uint *)(iVar7 + 8) != uVar10 >> 0x10)) || (*(int *)(iVar7 + 4) == 0)) &&
     (pEntity = CreateEntityByName((char *)(unaff_EBX + 0x37ce59),-1,true),
     pEntity != (CBaseEntity *)0x0)) {
    vVar3 = (vec_t)(this->m_PaintPowerType).m_Value;
    if (vVar3 != pEntity[1].m_Collision.super_CCollisionProperty.m_vecSpecifiedSurroundingMins.
                 super_CNetworkVectorCommonBase<Vector,CCollisionProperty::NetworkVar_m_vecSpecifiedSurroundingMins>
                 .
                 super_CNetworkVarBase<Vector,CCollisionProperty::NetworkVar_m_vecSpecifiedSurroundingMins>
                 .m_Value.x) {
      if ((pEntity->m_Network).m_TimerEvent.m_bRegistered == false) {
        pCVar4 = &((pEntity->m_Network).m_pPev)->super_CBaseEdict;
        if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
          pIVar9 = CBaseEdict::GetChangeAccessor(pCVar4);
          puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x50d377);
          if (pIVar9->m_iChangeInfoSerialNumber == *puVar5) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            uVar10 = (uint)pIVar9->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar2 = puVar5[uVar10 * 0x14 + 0x14];
            if (uVar2 == 0) {
LAB_00739d21:
              puVar5[(uint)uVar2 + uVar10 * 0x14 + 1] = 0x564;
              puVar5[uVar10 * 0x14 + 0x14] = uVar2 + 1;
            }
            else if (puVar5[uVar10 * 0x14 + 1] != 0x564) {
              local_98 = 0;
              do {
                uVar11 = local_98 + 1;
                local_98 = uVar11 & 0xffff;
                if ((ushort)uVar11 == uVar2) {
                  if (uVar2 == 0x13) goto LAB_00739be2;
                  goto LAB_00739d21;
                }
              } while (puVar5[uVar10 * 0x14 + local_98 + 1] != 0x564);
            }
          }
          else if (puVar5[0x7d1] == 100) {
LAB_00739be2:
            pIVar9->m_iChangeInfoSerialNumber = 0;
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar9->m_iChangeInfo = puVar5[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x50d377) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x50d377) + 0xfa2) + 1;
            piVar6 = *(int **)(unaff_EBX + 0x50d377);
            pIVar9->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
            iVar7 = *piVar6 + (uint)pIVar9->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar7 + 2) = 0x564;
            *(undefined2 *)(iVar7 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(pEntity->m_Network).field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      pEntity[1].m_Collision.super_CCollisionProperty.m_vecSpecifiedSurroundingMins.
      super_CNetworkVectorCommonBase<Vector,CCollisionProperty::NetworkVar_m_vecSpecifiedSurroundingMins>
      .super_CNetworkVarBase<Vector,CCollisionProperty::NetworkVar_m_vecSpecifiedSurroundingMins>.
      m_Value.x = vVar3;
    }
    vVar3 = (vec_t)this->m_nRenderMode;
    if (vVar3 != pEntity[1].m_Collision.super_CCollisionProperty.m_vecSpecifiedSurroundingMins.
                 super_CNetworkVectorCommonBase<Vector,CCollisionProperty::NetworkVar_m_vecSpecifiedSurroundingMins>
                 .
                 super_CNetworkVarBase<Vector,CCollisionProperty::NetworkVar_m_vecSpecifiedSurroundingMins>
                 .m_Value.y) {
      if ((pEntity->m_Network).m_TimerEvent.m_bRegistered == false) {
        pCVar4 = &((pEntity->m_Network).m_pPev)->super_CBaseEdict;
        if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
          pIVar9 = CBaseEdict::GetChangeAccessor(pCVar4);
          puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x50d377);
          if (pIVar9->m_iChangeInfoSerialNumber == *puVar5) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            uVar10 = (uint)pIVar9->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar2 = puVar5[uVar10 * 0x14 + 0x14];
            if (uVar2 == 0) {
LAB_00739dd1:
              puVar5[(uint)uVar2 + uVar10 * 0x14 + 1] = 0x568;
              puVar5[uVar10 * 0x14 + 0x14] = uVar2 + 1;
            }
            else if (puVar5[uVar10 * 0x14 + 1] != 0x568) {
              local_94 = 0;
              do {
                uVar11 = local_94 + 1;
                local_94 = uVar11 & 0xffff;
                if ((ushort)uVar11 == uVar2) {
                  if (uVar2 == 0x13) goto LAB_00739bce;
                  goto LAB_00739dd1;
                }
              } while (puVar5[uVar10 * 0x14 + local_94 + 1] != 0x568);
            }
          }
          else if (puVar5[0x7d1] == 100) {
LAB_00739bce:
            pIVar9->m_iChangeInfoSerialNumber = 0;
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar9->m_iChangeInfo = puVar5[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x50d377) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x50d377) + 0xfa2) + 1;
            piVar6 = *(int **)(unaff_EBX + 0x50d377);
            pIVar9->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
            iVar7 = *piVar6 + (uint)pIVar9->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar7 + 2) = 0x568;
            *(undefined2 *)(iVar7 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(pEntity->m_Network).field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      pEntity[1].m_Collision.super_CCollisionProperty.m_vecSpecifiedSurroundingMins.
      super_CNetworkVectorCommonBase<Vector,CCollisionProperty::NetworkVar_m_vecSpecifiedSurroundingMins>
      .super_CNetworkVarBase<Vector,CCollisionProperty::NetworkVar_m_vecSpecifiedSurroundingMins>.
      m_Value.y = vVar3;
    }
    DispatchSpawn(pEntity,true);
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
    uVar10 = (this->m_hPaintStream).
             super_CNetworkVarBase<CBaseHandle,CPaintSprayer::NetworkVar_m_hPaintStream>.m_Value.
             m_Index;
    if (((uVar10 == 0xffffffff) ||
        (iVar7 = (uVar10 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x50d37b),
        *(uint *)(iVar7 + 8) != uVar10 >> 0x10)) || (pEntity != *(CBaseEntity **)(iVar7 + 4))) {
      if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
        pCVar4 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
        if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
          pIVar9 = CBaseEdict::GetChangeAccessor(pCVar4);
          puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x50d377);
          if (pIVar9->m_iChangeInfoSerialNumber == *puVar5) {
            uVar10 = (uint)pIVar9->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar2 = puVar5[uVar10 * 0x14 + 0x14];
            if (uVar2 == 0) {
LAB_00739c71:
              puVar5[(uint)uVar2 + uVar10 * 0x14 + 1] = 0x408;
              puVar5[uVar10 * 0x14 + 0x14] = uVar2 + 1;
            }
            else if (puVar5[uVar10 * 0x14 + 1] != 0x408) {
              local_90 = 0;
              do {
                uVar11 = local_90 + 1;
                local_90 = uVar11 & 0xffff;
                if ((ushort)uVar11 == uVar2) {
                  if (uVar2 != 0x13) goto LAB_00739c71;
                  pIVar9->m_iChangeInfoSerialNumber = 0;
                  pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar5[uVar10 * 0x14 + local_90 + 1] != 0x408);
            }
          }
          else if (puVar5[0x7d1] == 100) {
            pIVar9->m_iChangeInfoSerialNumber = 0;
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar9->m_iChangeInfo = puVar5[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x50d377) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x50d377) + 0xfa2) + 1;
            piVar6 = *(int **)(unaff_EBX + 0x50d377);
            pIVar9->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
            iVar7 = *piVar6 + (uint)pIVar9->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar7 + 2) = 0x408;
            *(undefined2 *)(iVar7 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      puVar8 = (ulong *)(*(pEntity->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                          _vptr_IHandleEntity[3])(pEntity);
      (this->m_hPaintStream).
      super_CNetworkVarBase<CBaseHandle,CPaintSprayer::NetworkVar_m_hPaintStream>.m_Value.m_Index =
           *puVar8;
    }
  }
  return;
}


/* CPaintSprayer::Activate at 00739780 */

/* DWARF original prototype: void Activate(CPaintSprayer * this) */

void __thiscall CPaintSprayer::Activate(CPaintSprayer *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::Activate(&this->super_CBaseEntity);
  this->m_flLastThinkTime = *(float *)(**(int **)(unaff_EBX + 0x50d3d3) + 0xc);
  if (this->m_bStartActive == false) {
    return;
  }
  this->m_bStartActive = false;
  StartPaintSpray(this);
  return;
}


/* CPaintSprayer::UpdateOnRemove at 00737560 */

/* DWARF original prototype: void UpdateOnRemove(CPaintSprayer * this) */

void __thiscall CPaintSprayer::UpdateOnRemove(CPaintSprayer *this)

{
  uint uVar1;
  int iVar2;
  CBaseEntity *oldObj;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->m_hPaintStream).
          super_CNetworkVarBase<CBaseHandle,CPaintSprayer::NetworkVar_m_hPaintStream>.m_Value.
          m_Index;
  if ((uVar1 == 0xffffffff) ||
     (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(&DAT_0050f5fb + unaff_EBX),
     *(uint *)(iVar2 + 8) != uVar1 >> 0x10)) {
    oldObj = (CBaseEntity *)0x0;
  }
  else {
    oldObj = *(CBaseEntity **)(iVar2 + 4);
  }
  UTIL_Remove(oldObj);
  CBaseEntity::UpdateOnRemove(&this->super_CBaseEntity);
  return;
}


/* CPaintSprayer::SetPaintSprayerData at 00737a50 */

/* DWARF original prototype: void SetPaintSprayerData(CPaintSprayer * this, bool bSilent, float
   flBlobsPerSecond, float flMinSpeed, float flMaxSpeed, float flBlobSpreadRadius, float
   flBlobSpreadAngle, float flBlobStreakPercent, float flMinStreakTime, float flMaxStreakTime, float
   flMinStreakSpeedDampen, float flMaxStreakSpeedDampen) */

void __thiscall
CPaintSprayer::SetPaintSprayerData
          (CPaintSprayer *this,bool bSilent,float flBlobsPerSecond,float flMinSpeed,float flMaxSpeed
          ,float flBlobSpreadRadius,float flBlobSpreadAngle,float flBlobStreakPercent,
          float flMinStreakTime,float flMaxStreakTime,float flMinStreakSpeedDampen,
          float flMaxStreakSpeedDampen)

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
  uint local_148;
  uint local_144;
  uint local_140;
  uint local_13c;
  uint local_138;
  uint local_134;
  uint local_130;
  uint local_12c;
  uint local_128;
  uint local_124;
  uint local_120;
  
  ___i686_get_pc_thunk_bx();
  if (bSilent != (this->m_bSilent).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar4 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x50f107);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00738c20:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x3d5;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 0x3d5) {
            local_148 = 0;
            do {
              uVar9 = local_148 + 1;
              local_148 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 != 0x13) goto LAB_00738c20;
                goto LAB_00738462;
              }
            } while (puVar5[uVar8 * 0x14 + local_148 + 1] != 0x3d5);
          }
        }
        else if (puVar5[0x7d1] == 100) {
LAB_00738462:
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x50f107) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x50f107) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x50f107);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3d5;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bSilent).m_Value = bSilent;
  }
  if (flBlobsPerSecond != (this->m_flBlobsPerSecond).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar4 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x50f107);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_007388e0:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x3dc;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 0x3dc) {
            local_144 = 0;
            do {
              uVar9 = local_144 + 1;
              local_144 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 != 0x13) goto LAB_007388e0;
                pIVar7->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar8 * 0x14 + local_144 + 1] != 0x3dc);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x50f107) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x50f107) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x50f107);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3dc;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flBlobsPerSecond).m_Value = flBlobsPerSecond;
  }
  if (flMinSpeed != (this->m_flMinSpeed).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar4 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x50f107);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00738a70:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 1000;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 1000) {
            local_140 = 0;
            do {
              uVar9 = local_140 + 1;
              local_140 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 != 0x13) goto LAB_00738a70;
                pIVar7->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar8 * 0x14 + local_140 + 1] != 1000);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x50f107) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x50f107) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x50f107);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 1000;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flMinSpeed).m_Value = flMinSpeed;
  }
  if (flMaxSpeed != (this->m_flMaxSpeed).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar4 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x50f107);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00738750:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x3ec;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 0x3ec) {
            local_13c = 0;
            do {
              uVar9 = local_13c + 1;
              local_13c = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 != 0x13) goto LAB_00738750;
                pIVar7->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar8 * 0x14 + local_13c + 1] != 0x3ec);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x50f107) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x50f107) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x50f107);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3ec;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flMaxSpeed).m_Value = flMaxSpeed;
  }
  if (flBlobSpreadRadius != (this->m_flBlobSpreadRadius).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar4 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x50f107);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00738b3d:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x3e0;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 0x3e0) {
            local_138 = 0;
            do {
              uVar9 = local_138 + 1;
              local_138 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 != 0x13) goto LAB_00738b3d;
                pIVar7->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar8 * 0x14 + local_138 + 1] != 0x3e0);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x50f107) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x50f107) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x50f107);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3e0;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flBlobSpreadRadius).m_Value = flBlobSpreadRadius;
  }
  if (flBlobSpreadAngle != (this->m_flBlobSpreadAngle).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar4 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x50f107);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_0073881a:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x3e4;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 0x3e4) {
            local_134 = 0;
            do {
              uVar9 = local_134 + 1;
              local_134 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 != 0x13) goto LAB_0073881a;
                pIVar7->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar8 * 0x14 + local_134 + 1] != 0x3e4);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x50f107) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x50f107) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x50f107);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3e4;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flBlobSpreadAngle).m_Value = flBlobSpreadAngle;
  }
  if (flBlobStreakPercent != (this->m_flStreakPercentage).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar4 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x50f107);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_007389aa:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x3f0;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 0x3f0) {
            local_130 = 0;
            do {
              uVar9 = local_130 + 1;
              local_130 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 != 0x13) goto LAB_007389aa;
                pIVar7->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar8 * 0x14 + local_130 + 1] != 0x3f0);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x50f107) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x50f107) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x50f107);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3f0;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flStreakPercentage).m_Value = flBlobStreakPercent;
  }
  if (flMinStreakTime != (this->m_flMinStreakTime).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar4 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x50f107);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_0073868a:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x3f4;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 0x3f4) {
            local_12c = 0;
            do {
              uVar9 = local_12c + 1;
              local_12c = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 != 0x13) goto LAB_0073868a;
                pIVar7->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar8 * 0x14 + local_12c + 1] != 0x3f4);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x50f107) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x50f107) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x50f107);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3f4;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flMinStreakTime).m_Value = flMinStreakTime;
  }
  if (flMaxStreakTime != (this->m_flMaxStreakTime).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar4 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x50f107);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_007385ca:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x3f8;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 0x3f8) {
            local_128 = 0;
            do {
              uVar9 = local_128 + 1;
              local_128 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 != 0x13) goto LAB_007385ca;
                pIVar7->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar8 * 0x14 + local_128 + 1] != 0x3f8);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x50f107) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x50f107) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x50f107);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3f8;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flMaxStreakTime).m_Value = flMaxStreakTime;
  }
  if (flMinStreakSpeedDampen != (this->m_flMinStreakSpeedDampen).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar4 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x50f107);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_0073850a:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x3fc;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 0x3fc) {
            local_124 = 0;
            do {
              uVar9 = local_124 + 1;
              local_124 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 != 0x13) goto LAB_0073850a;
                pIVar7->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar8 * 0x14 + local_124 + 1] != 0x3fc);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x50f107) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x50f107) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x50f107);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3fc;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flMinStreakSpeedDampen).m_Value = flMinStreakSpeedDampen;
  }
  if (flMaxStreakSpeedDampen != (this->m_flMaxStreakSpeedDampen).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar4 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x50f107);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar5) {
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00738cea:
            puVar5[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x400;
            puVar5[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar8 * 0x14 + 1] != 0x400) {
            local_120 = 0;
            do {
              uVar9 = local_120 + 1;
              local_120 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 != 0x13) goto LAB_00738cea;
                pIVar7->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar8 * 0x14 + local_120 + 1] != 0x400);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar7->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x50f107) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x50f107) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x50f107);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x400;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flMaxStreakSpeedDampen).m_Value = flMaxStreakSpeedDampen;
  }
  return;
}


/* CPaintSprayer::SetPaintPowerType at 007378a0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void SetPaintPowerType(CPaintSprayer * this, PaintPowerType paintType)
    */

void __thiscall CPaintSprayer::SetPaintPowerType(CPaintSprayer *this,PaintPowerType paintType)

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
  if (paintType != (this->m_PaintPowerType).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x50f2b4);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00737a14:
            puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x3d8;
            puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != 0x3d8) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_00737a14;
                goto LAB_00737a39;
              }
            } while (puVar4[uVar7 * 0x14 + local_60 + 1] != 0x3d8);
          }
        }
        else if (puVar4[0x7d1] == 100) {
LAB_00737a39:
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x50f2b4) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x50f2b4) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x50f2b4);
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar2 = *piVar5 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3d8;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_PaintPowerType).m_Value = paintType;
  }
  return;
}


/* CPaintSprayer::UpdateTransmitState at 00737510 */

/* DWARF original prototype: int UpdateTransmitState(CPaintSprayer * this) */

int __thiscall CPaintSprayer::UpdateTransmitState(CPaintSprayer *this)

{
  int iVar1;
  
  if ((this->m_bSilent).m_Value != false) {
    iVar1 = CBaseEntity::SetTransmitState(&this->super_CBaseEntity,0x10);
    return iVar1;
  }
  iVar1 = CBaseEntity::SetTransmitState(&this->super_CBaseEntity,8);
  return iVar1;
}


/* CPaintSprayer::PaintSprayerThink at 00737490 */

/* DWARF original prototype: void PaintSprayerThink(CPaintSprayer * this) */

void __thiscall CPaintSprayer::PaintSprayerThink(CPaintSprayer *this)

{
  float fVar1;
  float fVar2;
  int *piVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((this->m_bActive).m_Value != false) {
                    /* Unresolved local var: float flDeltaTime@[???] */
    piVar3 = *(int **)(&DAT_0050f6c0 + unaff_EBX);
    fVar1 = *(float *)(*piVar3 + 0xc);
    fVar2 = this->m_flLastThinkTime;
    this->m_flLastThinkTime = fVar1;
    SprayPaint(this,fVar1 - fVar2);
    CBaseEntity::SetNextThink(&this->super_CBaseEntity,*(float *)(*piVar3 + 0xc),(char *)0x0);
  }
  return;
}


/* CPaintSprayer::InputStart at 00739770 */

/* DWARF original prototype: void InputStart(CPaintSprayer * this, inputdata_t * inputData) */

void __thiscall CPaintSprayer::InputStart(CPaintSprayer *this,inputdata_t *inputData)

{
  StartPaintSpray(this);
  return;
}


/* CPaintSprayer::InputStop at 007376e0 */

/* DWARF original prototype: void InputStop(CPaintSprayer * this, inputdata_t * inputData) */

void __thiscall CPaintSprayer::InputStop(CPaintSprayer *this,inputdata_t *inputData)

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
  if ((this->m_bActive).m_Value != false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x50f474);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00737864:
            puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x3d4;
            puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != 0x3d4) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_00737864;
                pIVar6->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar7 * 0x14 + local_60 + 1] != 0x3d4);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x50f474) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x50f474) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x50f474);
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar2 = *piVar5 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3d4;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bActive).m_Value = false;
  }
  CBaseEntity::SetNextThink(&this->super_CBaseEntity,-1.0,(char *)0x0);
  return;
}


/* CPaintSprayer::InputChangePaintType at 00739e10 */

/* DWARF original prototype: void InputChangePaintType(CPaintSprayer * this, inputdata_t *
   inputData) */

void __thiscall CPaintSprayer::InputChangePaintType(CPaintSprayer *this,inputdata_t *inputData)

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
  int local_30;
  
  ___i686_get_pc_thunk_bx();
  local_30 = 0;
  if ((inputData->value).fieldType == FIELD_INTEGER) {
    local_30 = (inputData->value).field_0.iVal;
  }
  if (local_30 != (this->m_PaintPowerType).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x50cd44);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00739fa4:
            puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x3d8;
            puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != 0x3d8) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_00739fa4;
                goto LAB_00739fc9;
              }
            } while (puVar4[uVar7 * 0x14 + local_60 + 1] != 0x3d8);
          }
        }
        else if (puVar4[0x7d1] == 100) {
LAB_00739fc9:
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x50cd44) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x50cd44) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x50cd44);
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar2 = *piVar5 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3d8;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_PaintPowerType).m_Value = local_30;
  }
  return;
}


/* CPaintSprayer::StartPaintSpray at 007393a0 */

/* DWARF original prototype: void StartPaintSpray(CPaintSprayer * this) */

void __thiscall CPaintSprayer::StartPaintSpray(CPaintSprayer *this)

{
  undefined1 *puVar1;
  int iVar2;
  float fVar3;
  ushort uVar4;
  CBaseEdict *pCVar5;
  int *piVar6;
  ushort *puVar7;
  IChangeInfoAccessor *pIVar8;
  uint uVar9;
  uint uVar10;
  int unaff_EBX;
  uint local_74;
  uint local_70;
  
  ___i686_get_pc_thunk_bx();
  if ((this->m_bActive).m_Value == false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar5 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
        pIVar8 = CBaseEdict::GetChangeAccessor(pCVar5);
        puVar7 = (ushort *)**(undefined4 **)(unaff_EBX + 0x50d7b1);
        if (pIVar8->m_iChangeInfoSerialNumber == *puVar7) {
          uVar9 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar4 = puVar7[uVar9 * 0x14 + 0x14];
          if (uVar4 == 0) {
LAB_007395c8:
            puVar7[(uint)uVar4 + uVar9 * 0x14 + 1] = 0x3d4;
            puVar7[uVar9 * 0x14 + 0x14] = uVar4 + 1;
          }
          else if (puVar7[uVar9 * 0x14 + 1] != 0x3d4) {
            local_74 = 0;
            do {
              uVar10 = local_74 + 1;
              local_74 = uVar10 & 0xffff;
              if ((ushort)uVar10 == uVar4) {
                if (uVar4 != 0x13) goto LAB_007395c8;
                pIVar8->m_iChangeInfoSerialNumber = 0;
                pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar7[uVar9 * 0x14 + local_74 + 1] != 0x3d4);
          }
        }
        else if (puVar7[0x7d1] == 100) {
          pIVar8->m_iChangeInfoSerialNumber = 0;
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar8->m_iChangeInfo = puVar7[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x50d7b1) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x50d7b1) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x50d7b1);
          pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar8->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3d4;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bActive).m_Value = true;
    this->m_flAccumulatedTime =
         *(float *)(unaff_EBX + 0x393559) / (this->m_flBlobsPerSecond).m_Value;
    fVar3 = *(float *)(**(int **)(unaff_EBX + 0x50d7ad) + 0xc);
    if (fVar3 != (this->m_flStartTime).m_Value) {
      if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
        pCVar5 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
        if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
          pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
          pIVar8 = CBaseEdict::GetChangeAccessor(pCVar5);
          puVar7 = (ushort *)**(undefined4 **)(unaff_EBX + 0x50d7b1);
          if (pIVar8->m_iChangeInfoSerialNumber == *puVar7) {
            uVar9 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar4 = puVar7[uVar9 * 0x14 + 0x14];
            if (uVar4 == 0) {
LAB_00739704:
              puVar7[(uint)uVar4 + uVar9 * 0x14 + 1] = 0x404;
              puVar7[uVar9 * 0x14 + 0x14] = uVar4 + 1;
            }
            else if (puVar7[uVar9 * 0x14 + 1] != 0x404) {
              local_70 = 0;
              do {
                uVar10 = local_70 + 1;
                local_70 = uVar10 & 0xffff;
                if ((ushort)uVar10 == uVar4) {
                  if (uVar4 != 0x13) goto LAB_00739704;
                  pIVar8->m_iChangeInfoSerialNumber = 0;
                  pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar7[uVar9 * 0x14 + local_70 + 1] != 0x404);
            }
          }
          else if (puVar7[0x7d1] == 100) {
            pIVar8->m_iChangeInfoSerialNumber = 0;
            pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar8->m_iChangeInfo = puVar7[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x50d7b1) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x50d7b1) + 0xfa2) + 1;
            piVar6 = *(int **)(unaff_EBX + 0x50d7b1);
            pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
            iVar2 = *piVar6 + (uint)pIVar8->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar2 + 2) = 0x404;
            *(undefined2 *)(iVar2 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->m_flStartTime).m_Value = fVar3;
    }
    piVar6 = *(int **)(unaff_EBX + 0x50d7ad);
    this->m_flLastThinkTime = *(float *)(*piVar6 + 0xc);
    this->m_nBlobSpawned = 0;
    CBaseEntity::SetNextThink(&this->super_CBaseEntity,*(float *)(*piVar6 + 0xc),(char *)0x0);
  }
  return;
}


/* CPaintSprayer::StopPaintSpray at 007391e0 */

/* DWARF original prototype: void StopPaintSpray(CPaintSprayer * this) */

void __thiscall CPaintSprayer::StopPaintSpray(CPaintSprayer *this)

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
  if ((this->m_bActive).m_Value != false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x50d974);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00739364:
            puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x3d4;
            puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != 0x3d4) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_00739364;
                pIVar6->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar7 * 0x14 + local_60 + 1] != 0x3d4);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x50d974) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x50d974) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x50d974);
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar2 = *piVar5 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3d4;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bActive).m_Value = false;
  }
  CBaseEntity::SetNextThink(&this->super_CBaseEntity,-1.0,(char *)0x0);
  return;
}


/* _GLOBAL__I__ZN13CPaintSprayer14GetServerClassEv at 000ae750 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN13CPaintSprayer14GetServerClassEv(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

