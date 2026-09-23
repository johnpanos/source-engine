/* DWARF-guided pseudocode for game/server/portal2/portal_playerlocaldata.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* ServerClassInit<DT_PortalLocal::ignored> at 000b0340 */

int ServerClassInit<DT_PortalLocal::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  char *pcVar2;
  SendProp local_16c;
  SendProp local_118;
  SendProp local_c4;
  SendProp local_70;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x71].m_pPrev + unaff_EBX + 3) ==
      '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xd9e2ff);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x72].m_pPrev + unaff_EBX + 3)
                  ,(char *)(unaff_EBX + 0x9a58e3),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x77].m_pNext +
                    unaff_EBX + 3),&UNK_00a02ab0 + unaff_EBX,4,4,0x20,0,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00b968f7 + unaff_EBX),0x80);
      pcVar2 = &UNK_00a02acd + unaff_EBX;
      SendPropInt((SendProp_conflict *)&local_70,pcVar2,8,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropArray3((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x7d].m_pEntity +
                     unaff_EBX + 3),pcVar2,8,4,0x10,(SendProp_conflict *)&local_70,
                     *(SendTableProxyFn_conflict *)(&DAT_00b9689f + unaff_EBX),0x80);
      SendProp::~SendProp(&local_70,(int)pcVar2);
      SendPropBool((SendProp_conflict *)
                   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x82].m_SerialNumber +
                   unaff_EBX + 3),&UNK_00a02ae5 + unaff_EBX,0x48,1);
      pcVar2 = &UNK_00a02af5 + unaff_EBX;
      SendPropVector((SendProp_conflict *)&local_c4,pcVar2,0x4c,0xc,0x20,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00b968eb + unaff_EBX),0x80);
      SendPropArray3((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x87].m_pPrev +
                     unaff_EBX + 3),pcVar2,0x4c,0xc,4,(SendProp_conflict *)&local_c4,
                     *(SendTableProxyFn_conflict *)(&DAT_00b9689f + unaff_EBX),0x80);
      SendProp::~SendProp(&local_c4,(int)pcVar2);
      pcVar2 = &UNK_00a02b11 + unaff_EBX;
      SendPropVector((SendProp_conflict *)&local_118,pcVar2,0x7c,0xc,0x20,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00b968eb + unaff_EBX),0x80);
      SendPropArray3((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x8c].m_pNext +
                     unaff_EBX + 3),pcVar2,0x7c,0xc,4,(SendProp_conflict *)&local_118,
                     *(SendTableProxyFn_conflict *)(&DAT_00b9689f + unaff_EBX),0x80);
      SendProp::~SendProp(&local_118,(int)pcVar2);
      pcVar2 = &UNK_00a02b2b + unaff_EBX;
      SendPropBool((SendProp_conflict *)&local_16c,pcVar2,0xac,1);
      SendPropArray3((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x92].m_pEntity +
                     unaff_EBX + 3),pcVar2,0xac,1,4,(SendProp_conflict *)&local_16c,
                     *(SendTableProxyFn_conflict *)(&DAT_00b9689f + unaff_EBX),0x80);
      SendProp::~SendProp(&local_16c,(int)pcVar2);
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x97].m_SerialNumber +
                     unaff_EBX + 3),&UNK_00a02b47 + unaff_EBX,0x1000b0,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00b968eb + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x9c].m_pPrev +
                     unaff_EBX + 3),&UNK_00a02b55 + unaff_EBX,0x1000bc,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00b968eb + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xa1].m_pNext +
                     unaff_EBX + 3),&UNK_00a02b66 + unaff_EBX,0x1000c8,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00b968eb + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xa7].m_pEntity +
                     unaff_EBX + 3),&UNK_00a02b7b + unaff_EBX,0x1000d4,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00b968eb + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xac].m_SerialNumber +
                     unaff_EBX + 3),&UNK_00a02b80 + unaff_EBX,0x1000e0,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00b968eb + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xb1].m_pPrev +
                     unaff_EBX + 3),&UNK_00a02b95 + unaff_EBX,0x1000ec,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00b968eb + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xb6].m_pNext +
                     unaff_EBX + 3),&UNK_00a02ba4 + unaff_EBX,0x1000f8,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00b968eb + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xbc].m_pEntity +
                     unaff_EBX + 3),&UNK_00a02bb3 + unaff_EBX,0x100104,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00b968eb + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xc1].m_SerialNumber +
                     unaff_EBX + 3),&UNK_00a02bc1 + unaff_EBX,0x100110,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00b968eb + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xc6].m_pPrev +
                     unaff_EBX + 3),&UNK_00a02bcf + unaff_EBX,0x10011c,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00b968eb + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xcb].m_pNext +
                     unaff_EBX + 3),&UNK_00a02beb + unaff_EBX,0x100128,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00b968eb + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xd1].m_pEntity +
                     unaff_EBX + 3),&UNK_00a02c07 + unaff_EBX,0x100134,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00b968eb + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xd6].m_SerialNumber +
                     unaff_EBX + 3),&UNK_00a02c22 + unaff_EBX,0x100140,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00b968eb + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xdb].m_pPrev +
                     unaff_EBX + 3),&UNK_00a02c3d + unaff_EBX,0x10014c,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00b968eb + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xe0].m_pNext +
                     unaff_EBX + 3),(char *)(unaff_EBX + 0xa02c48),0x100158,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00b968eb + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xe6].m_pEntity +
                     unaff_EBX + 3),(char *)(unaff_EBX + 0xa02c55),0x100164,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00b968eb + unaff_EBX),0x80);
      SendPropQAngles((SendProp_conflict *)
                      ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xeb].m_SerialNumber +
                      unaff_EBX + 3),(char *)(unaff_EBX + 0xa02c64),0x100170,0xc,0,4,
                      *(SendVarProxyFn_conflict *)(&DAT_00b96963 + unaff_EBX),0x80);
      SendPropInt((SendProp_conflict *)
                  ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xf0].m_pPrev + unaff_EBX + 3)
                  ,(char *)(unaff_EBX + 0xa02c77),0x17c,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xf5].m_pNext +
                        unaff_EBX + 3),(char *)(unaff_EBX + 0xa02c8a),0x180,
                        *(SendTable **)(&DAT_00b9735f + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00b9689f + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0xfb].m_pEntity +
                    unaff_EBX + 3),(char *)(unaff_EBX + 0xa02c9e),400,4,0,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00b968f7 + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)
                    (&UNK_0000100b +
                    (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                    (char *)(unaff_EBX + 0xa02cb3),0x18c,4,0,4,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00b968f7 + unaff_EBX),0x80);
      SendPropInt((SendProp_conflict *)
                  (&UNK_0000105f +
                  (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                  (char *)(unaff_EBX + 0xa02cc5),0x194,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropInt((SendProp_conflict *)
                  (&UNK_000010b3 +
                  (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                  (char *)(unaff_EBX + 0xa02cd9),0x198,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropBool((SendProp_conflict *)
                   (&UNK_00001107 +
                   (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                   (char *)(unaff_EBX + 0xa02ce6),0x19c,1);
      SendPropBool((SendProp_conflict *)
                   (&DAT_0000115b +
                   (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                   (char *)(unaff_EBX + 0xa02cf9),0x19d,1);
      SendPropBool((SendProp_conflict *)
                   (&UNK_000011af +
                   (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                   (char *)(unaff_EBX + 0xa02d0d),0x19f,1);
      SendPropBool((SendProp_conflict *)
                   (&UNK_00001203 +
                   (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                   (char *)(unaff_EBX + 0xa02d20),0x1a0,1);
      SendPropBool((SendProp_conflict *)
                   (&UNK_00001257 +
                   (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                   (char *)(unaff_EBX + 0xa02d2f),0x1a1,1);
      SendPropBool((SendProp_conflict *)
                   (&UNK_000012ab +
                   (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                   (char *)(unaff_EBX + 0xa02d3a),0x1a2,1);
      ___cxa_guard_release(unaff_EBX + 0xd9e2ff);
      ___cxa_atexit(unaff_EBX + 0x98e3cf,0,*(undefined4 *)(&DAT_00b96803 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)
             ((int)DataMapInit<CAI_LeadBehavior>::dataDesc[0xc].flatOffset + unaff_EBX + -0x19),
             (SendProp_conflict *)
             ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x77].m_pNext + unaff_EBX + 3),0x23
             ,*(char **)(&DAT_00c1fb6b + unaff_EBX));
  return 1;
}


/* __static_initialization_and_destruction_0 at 000b12a0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  int iVar1;
  datamap_t *pdVar2;
  int unaff_EBX;
  longlong lVar3;
  
  lVar3 = ___i686_get_pc_thunk_bx();
  if (lVar3 != 0xffff00000001) {
    return;
  }
  (&DAT_00d9d32f)[unaff_EBX] = 0;
  (&DAT_00d9d330)[unaff_EBX] = 0;
  (&DAT_00d9d331)[unaff_EBX] = 0;
  (&DAT_00d9d332)[unaff_EBX] = 0;
  *(undefined4 *)(&DAT_00d9d333 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00d9d337 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00d9d33b + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00d9d33f + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&sv_allchat.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&sv_allchat.super_ConCommandBase.m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)(&sv_allchat.super_ConCommandBase.field_0xb + unaff_EBX) = 0;
  *(undefined4 *)((int)&sv_allchat.super_ConCommandBase.m_pszName + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&sv_allchat.super_ConCommandBase.m_pszHelpString + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)((int)&sv_allchat.super_ConCommandBase.m_nFlags + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&sv_allchat.super_IConVar._vptr_IConVar + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&sv_allchat.m_pParent + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined **)((int)&sv_allchat.m_pszDefaultValue + unaff_EBX + 3) = &UNK_00b9b057 + unaff_EBX;
  SendTable::SendTable((SendTable *)((int)&g_SidekickFollowFormation.walkTolerance + unaff_EBX + 3))
  ;
  ___cxa_atexit(unaff_EBX + 0x98d4af,0,*(undefined4 *)(&DAT_00b958a3 + unaff_EBX));
  iVar1 = ServerClassInit<DT_PortalLocal::ignored>((ignored *)0x0);
  *(int *)(&DAT_00d1822f + unaff_EBX) = iVar1;
  pdVar2 = DataMapInit<CPortalPlayerLocalData>((CPortalPlayerLocalData *)0x0);
  *(datamap_t **)(&DAT_00d18233 + unaff_EBX) = pdVar2;
  *(undefined4 *)((int)&sv_allchat.m_Value.m_pszString + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&sv_allchat.m_Value.m_StringLength + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&sv_allchat.m_Value.m_fValue + unaff_EBX + 3) = 0x3f800000;
  return;
}


/* __tcf_2 at 00a3e760 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x38ad64),in_stack_00000008);
  return;
}


/* __tcf_0 at 00a3e720 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  undefined4 *puVar1;
  
  ___i686_get_pc_thunk_bx();
  puVar1 = (undefined4 *)(unaff_EBX + 0x410aae);
  do {
    (**(code **)*puVar1)(puVar1);
    puVar1 = puVar1 + -0x15;
  } while (puVar1 != (undefined4 *)(unaff_EBX + 0x40fede));
  return;
}


/* DataMapInit<CPortalPlayerLocalData> at 000b0270 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

datamap_t * DataMapInit<CPortalPlayerLocalData>(CPortalPlayerLocalData *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x7c].m_pEntity + unaff_EBX + 3)
      == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xd9e3a7);
    if (iVar1 != 0) {
      *(undefined **)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x7c].m_pPrev + unaff_EBX + 3) =
           &UNK_00a02b69 + unaff_EBX;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x7d].m_pEntity + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x7d].m_SerialNumber + unaff_EBX + 3) = 0
      ;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x7d].m_pPrev + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x7d].m_pNext + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x7e].m_pEntity + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x7c].m_pNext + unaff_EBX + 3) = 0x16;
      ___cxa_guard_release(unaff_EBX + 0xd9e3a7);
      ___cxa_atexit(unaff_EBX + 0x98e4ff,0,*(undefined4 *)(&DAT_00b968d3 + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_SetPlayerUnderwater_00cb66e8 + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&PTR_GiveNamedItem_00cb66e0 + unaff_EBX + 3) = 0x23;
  *(int *)((int)&PTR_ItemPostFrame_00cb66dc + unaff_EBX + 3) = unaff_EBX + 0xcb673f;
  return (datamap_t *)((int)&PTR_ItemPostFrame_00cb66dc + unaff_EBX + 3);
}


/* CPortalPlayerLocalData::GetBaseMap at 00743970 */

datamap_t * CPortalPlayerLocalData::GetBaseMap(void)

{
  return (datamap_t *)0x0;
}


/* __tcf_1 at 00a3e780 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x40feb6)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x40feaa) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x40feb6));
  }
  *(undefined4 *)(unaff_EBX + 0x40feb6) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40feb2)) {
    if (*(int *)(unaff_EBX + 0x40feaa) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2083ce) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2083ce),*(int *)(unaff_EBX + 0x40feaa));
      *(undefined4 *)(unaff_EBX + 0x40feaa) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40feae) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40feaa);
  *(int *)(unaff_EBX + 0x40feba) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40feb2)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2083ce) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2083ce),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40feaa) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40feae) = 0;
  }
  return;
}


/* CPortalPlayerLocalData::CPortalPlayerLocalData at 00743980 */

/* WARNING: Restarted to delay deadcode elimination for space: stack */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void CPortalPlayerLocalData(CPortalPlayerLocalData * this) */

void __thiscall CPortalPlayerLocalData::CPortalPlayerLocalData(CPortalPlayerLocalData *this)

{
  vec_t *pvVar1;
  char cVar2;
  float *pfVar3;
  undefined4 *puVar4;
  CNetworkVarBase<float,CountdownTimer::NetworkVar_m_timestamp> *pCVar5;
  NetworkVar_m_PaintPowerHudInfoNormal *pNVar6;
  NetworkVar_m_PaintedPowerTimer *pNVar7;
  NetworkVar_m_PaintPowerHudInfoPosition *pNVar8;
  int iVar9;
  Vector *pVVar10;
  int unaff_EBX;
  NetworkVar_m_PaintPowerHudInfoPosition *pNVar11;
  NetworkVar_m_PaintPowerHudInfoPosition *pNVar12;
  NetworkVar_m_PaintPowerHudInfoNormal *pNVar13;
  NetworkVar_m_PaintPowerHudInfoNormal *pNVar14;
  Vector *pVVar15;
  Vector *pVVar16;
  float *pfVar17;
  float *pfVar18;
  bool bVar19;
  float fVar20;
  CountdownTimer *pThis;
  
  ___i686_get_pc_thunk_bx();
  this->_vptr_CPortalPlayerLocalData = (_func_int_varargs **)(&UNK_0058c4ff + unaff_EBX);
  (this->m_PaintedPowerTimer).super_CountdownTimer._vptr_CountdownTimer =
       (_func_int_varargs **)(unaff_EBX + 0x566217);
  (this->m_PaintedPowerTimer).super_CountdownTimer.m_duration.m_Value = 0.0;
  pNVar7 = &this->m_PaintedPowerTimer;
  (**(code **)(unaff_EBX + 0x56621b))
            (pNVar7,&(this->m_PaintedPowerTimer).super_CountdownTimer.m_duration);
  (this->m_PaintedPowerTimer).super_CountdownTimer.m_timestamp.m_Value = -1.0;
  pCVar5 = &(this->m_PaintedPowerTimer).super_CountdownTimer.m_timestamp;
                    /* Unresolved local var: CountdownTimer * pThis@[???] */
  (*(this->m_PaintedPowerTimer).super_CountdownTimer._vptr_CountdownTimer[1])(pNVar7,pCVar5);
  (this->m_PaintedPowerTimer).super_CountdownTimer._vptr_CountdownTimer =
       (_func_int_varargs **)(unaff_EBX + 0x58c517);
  if ((this->m_flAirControlSupressionTime).m_Value != 0.0) {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_flAirControlSupressionTime);
    (this->m_flAirControlSupressionTime).m_Value = 0.0;
  }
                    /* Unresolved local var: int i@[???] */
  (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_nLocatorEntityIndices);
  (this->m_nLocatorEntityIndices).m_Value[0] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 1);
  (this->m_nLocatorEntityIndices).m_Value[1] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 2);
  (this->m_nLocatorEntityIndices).m_Value[2] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 3);
  (this->m_nLocatorEntityIndices).m_Value[3] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 4);
  (this->m_nLocatorEntityIndices).m_Value[4] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 5);
  (this->m_nLocatorEntityIndices).m_Value[5] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 6);
  (this->m_nLocatorEntityIndices).m_Value[6] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 7);
  (this->m_nLocatorEntityIndices).m_Value[7] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 8);
  (this->m_nLocatorEntityIndices).m_Value[8] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 9);
  (this->m_nLocatorEntityIndices).m_Value[9] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 10);
  (this->m_nLocatorEntityIndices).m_Value[10] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 0xb);
  (this->m_nLocatorEntityIndices).m_Value[0xb] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 0xc);
  (this->m_nLocatorEntityIndices).m_Value[0xc] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 0xd);
  (this->m_nLocatorEntityIndices).m_Value[0xd] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 0xe);
  (this->m_nLocatorEntityIndices).m_Value[0xe] = -1;
  (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_nLocatorEntityIndices).m_Value + 0xf);
  (this->m_nLocatorEntityIndices).m_Value[0xf] = -1;
  if ((this->m_bPlacingPhoto).m_Value != false) {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_bPlacingPhoto);
    (this->m_bPlacingPhoto).m_Value = false;
  }
  pNVar8 = &this->m_PaintPowerHudInfoPosition;
  bVar19 = pNVar8 == (NetworkVar_m_PaintPowerHudInfoPosition *)0x0;
  pfVar3 = *(float **)(unaff_EBX + 0x5031e7);
  iVar9 = 0xc;
  pNVar11 = pNVar8;
  pfVar17 = pfVar3;
  do {
    pNVar12 = pNVar11;
    pfVar18 = pfVar17;
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    pfVar18 = (float *)((int)pfVar17 + 1);
    pNVar12 = (NetworkVar_m_PaintPowerHudInfoPosition *)((int)&pNVar11->m_Value[0].x + 1);
    bVar19 = *(char *)&pNVar11->m_Value[0].x == *(char *)pfVar17;
    pNVar11 = pNVar12;
    pfVar17 = pfVar18;
  } while (bVar19);
  iVar9 = 0;
  if (!bVar19) {
    iVar9 = (uint)*(byte *)((int)(pNVar12 + -1) + 0x2f) - (uint)*(byte *)((int)pfVar18 + -1);
  }
  if (iVar9 != 0) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,pNVar8);
    (this->m_PaintPowerHudInfoPosition).m_Value[0].x = *pfVar3;
    (this->m_PaintPowerHudInfoPosition).m_Value[0].y = pfVar3[1];
    (this->m_PaintPowerHudInfoPosition).m_Value[0].z = pfVar3[2];
  }
  pNVar6 = &this->m_PaintPowerHudInfoNormal;
  bVar19 = pNVar6 == (NetworkVar_m_PaintPowerHudInfoNormal *)0x0;
  iVar9 = 0xc;
  pNVar13 = pNVar6;
  pfVar17 = pfVar3;
  do {
    pNVar14 = pNVar13;
    pfVar18 = pfVar17;
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    pfVar18 = (float *)((int)pfVar17 + 1);
    pNVar14 = (NetworkVar_m_PaintPowerHudInfoNormal *)((int)&pNVar13->m_Value[0].x + 1);
    bVar19 = *(char *)&pNVar13->m_Value[0].x == *(char *)pfVar17;
    pNVar13 = pNVar14;
    pfVar17 = pfVar18;
  } while (bVar19);
  iVar9 = 0;
  if (!bVar19) {
    iVar9 = (uint)*(byte *)((int)&((NetworkVar_m_PaintPowerHudInfoPosition *)(pNVar14 + -1))->
                                  m_Value[3].z + 3) - (uint)*(byte *)((int)pfVar18 + -1);
  }
  if (iVar9 != 0) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,pNVar6);
    (this->m_PaintPowerHudInfoNormal).m_Value[0].x = *pfVar3;
    (this->m_PaintPowerHudInfoNormal).m_Value[0].y = pfVar3[1];
    (this->m_PaintPowerHudInfoNormal).m_Value[0].z = pfVar3[2];
  }
  if ((this->m_PaintPowerHudInfoValidity).m_Value[0] != false) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_PaintPowerHudInfoValidity);
    (this->m_PaintPowerHudInfoValidity).m_Value[0] = false;
  }
  pVVar10 = (this->m_PaintPowerHudInfoPosition).m_Value + 1;
  bVar19 = pVVar10 == (Vector *)0x0;
  iVar9 = 0xc;
  pVVar15 = pVVar10;
  pfVar17 = pfVar3;
  do {
    pVVar16 = pVVar15;
    pfVar18 = pfVar17;
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    pfVar18 = (float *)((int)pfVar17 + 1);
    pVVar16 = (Vector *)((int)&pVVar15->x + 1);
    bVar19 = *(char *)&pVVar15->x == *(char *)pfVar17;
    pVVar15 = pVVar16;
    pfVar17 = pfVar18;
  } while (bVar19);
  iVar9 = 0;
  if (!bVar19) {
    iVar9 = (uint)(byte)*(undefined1 *)
                         ((int)&((NetworkVar_m_PaintPowerHudInfoPosition *)(pVVar16 + -1))->m_Value
                                [0].z + 3) - (uint)*(byte *)((int)pfVar18 + -1);
  }
  if (iVar9 != 0) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,pVVar10);
    (this->m_PaintPowerHudInfoPosition).m_Value[1].x = *pfVar3;
    (this->m_PaintPowerHudInfoPosition).m_Value[1].y = pfVar3[1];
    (this->m_PaintPowerHudInfoPosition).m_Value[1].z = pfVar3[2];
  }
  pVVar10 = (this->m_PaintPowerHudInfoNormal).m_Value + 1;
  bVar19 = pVVar10 == (Vector *)0x0;
  iVar9 = 0xc;
  pVVar15 = pVVar10;
  pfVar17 = pfVar3;
  do {
    pVVar16 = pVVar15;
    pfVar18 = pfVar17;
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    pfVar18 = (float *)((int)pfVar17 + 1);
    pVVar16 = (Vector *)((int)&pVVar15->x + 1);
    bVar19 = *(char *)&pVVar15->x == *(char *)pfVar17;
    pVVar15 = pVVar16;
    pfVar17 = pfVar18;
  } while (bVar19);
  iVar9 = 0;
  if (!bVar19) {
    iVar9 = (uint)(byte)*(undefined1 *)
                         ((int)&((NetworkVar_m_PaintPowerHudInfoNormal *)(pVVar16 + -1))->m_Value[0]
                                .z + 3) - (uint)*(byte *)((int)pfVar18 + -1);
  }
  if (iVar9 != 0) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,pVVar10);
    (this->m_PaintPowerHudInfoNormal).m_Value[1].x = *pfVar3;
    (this->m_PaintPowerHudInfoNormal).m_Value[1].y = pfVar3[1];
    (this->m_PaintPowerHudInfoNormal).m_Value[1].z = pfVar3[2];
  }
  if ((this->m_PaintPowerHudInfoValidity).m_Value[1] != false) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_PaintPowerHudInfoValidity).m_Value + 1);
    (this->m_PaintPowerHudInfoValidity).m_Value[1] = false;
  }
  pVVar10 = (this->m_PaintPowerHudInfoPosition).m_Value + 2;
  bVar19 = pVVar10 == (Vector *)0x0;
  iVar9 = 0xc;
  pVVar15 = pVVar10;
  pfVar17 = pfVar3;
  do {
    pVVar16 = pVVar15;
    pfVar18 = pfVar17;
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    pfVar18 = (float *)((int)pfVar17 + 1);
    pVVar16 = (Vector *)((int)&pVVar15->x + 1);
    bVar19 = *(char *)&pVVar15->x == *(char *)pfVar17;
    pVVar15 = pVVar16;
    pfVar17 = pfVar18;
  } while (bVar19);
  iVar9 = 0;
  if (!bVar19) {
    iVar9 = (uint)*(byte *)((int)&pVVar16[-1].z + 3) - (uint)*(byte *)((int)pfVar18 + -1);
  }
  if (iVar9 != 0) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,pVVar10);
    (this->m_PaintPowerHudInfoPosition).m_Value[2].x = *pfVar3;
    (this->m_PaintPowerHudInfoPosition).m_Value[2].y = pfVar3[1];
    (this->m_PaintPowerHudInfoPosition).m_Value[2].z = pfVar3[2];
  }
  pVVar10 = (this->m_PaintPowerHudInfoNormal).m_Value + 2;
  bVar19 = pVVar10 == (Vector *)0x0;
  iVar9 = 0xc;
  pVVar15 = pVVar10;
  pfVar17 = pfVar3;
  do {
    pVVar16 = pVVar15;
    pfVar18 = pfVar17;
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    pfVar18 = (float *)((int)pfVar17 + 1);
    pVVar16 = (Vector *)((int)&pVVar15->x + 1);
    bVar19 = *(char *)&pVVar15->x == *(char *)pfVar17;
    pVVar15 = pVVar16;
    pfVar17 = pfVar18;
  } while (bVar19);
  iVar9 = 0;
  if (!bVar19) {
    iVar9 = (uint)*(byte *)((int)&pVVar16[-1].z + 3) - (uint)*(byte *)((int)pfVar18 + -1);
  }
  if (iVar9 != 0) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,pVVar10);
    (this->m_PaintPowerHudInfoNormal).m_Value[2].x = *pfVar3;
    (this->m_PaintPowerHudInfoNormal).m_Value[2].y = pfVar3[1];
    (this->m_PaintPowerHudInfoNormal).m_Value[2].z = pfVar3[2];
  }
  if ((this->m_PaintPowerHudInfoValidity).m_Value[2] != false) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_PaintPowerHudInfoValidity).m_Value + 2);
    (this->m_PaintPowerHudInfoValidity).m_Value[2] = false;
  }
  pVVar10 = (this->m_PaintPowerHudInfoPosition).m_Value + 3;
  bVar19 = pVVar10 == (Vector *)0x0;
  iVar9 = 0xc;
  pVVar15 = pVVar10;
  pfVar17 = pfVar3;
  do {
    pVVar16 = pVVar15;
    pfVar18 = pfVar17;
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    pfVar18 = (float *)((int)pfVar17 + 1);
    pVVar16 = (Vector *)((int)&pVVar15->x + 1);
    bVar19 = *(char *)&pVVar15->x == *(char *)pfVar17;
    pVVar15 = pVVar16;
    pfVar17 = pfVar18;
  } while (bVar19);
  iVar9 = 0;
  if (!bVar19) {
    iVar9 = (uint)*(byte *)((int)&pVVar16[-1].z + 3) - (uint)*(byte *)((int)pfVar18 + -1);
  }
  if (iVar9 != 0) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,pVVar10);
    (this->m_PaintPowerHudInfoPosition).m_Value[3].x = *pfVar3;
    (this->m_PaintPowerHudInfoPosition).m_Value[3].y = pfVar3[1];
    (this->m_PaintPowerHudInfoPosition).m_Value[3].z = pfVar3[2];
  }
  pVVar10 = (this->m_PaintPowerHudInfoNormal).m_Value + 3;
  bVar19 = pVVar10 == (Vector *)0x0;
  iVar9 = 0xc;
  pVVar15 = pVVar10;
  pfVar17 = pfVar3;
  do {
    pVVar16 = pVVar15;
    pfVar18 = pfVar17;
    if (iVar9 == 0) break;
    iVar9 = iVar9 + -1;
    pfVar18 = (float *)((int)pfVar17 + 1);
    pVVar16 = (Vector *)((int)&pVVar15->x + 1);
    bVar19 = *(char *)&pVVar15->x == *(char *)pfVar17;
    pVVar15 = pVVar16;
    pfVar17 = pfVar18;
  } while (bVar19);
  iVar9 = 0;
  if (!bVar19) {
    iVar9 = (uint)*(byte *)((int)&pVVar16[-1].z + 3) - (uint)*(byte *)((int)pfVar18 + -1);
  }
  if (iVar9 != 0) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,pVVar10);
    (this->m_PaintPowerHudInfoNormal).m_Value[3].x = *pfVar3;
    (this->m_PaintPowerHudInfoNormal).m_Value[3].y = pfVar3[1];
    (this->m_PaintPowerHudInfoNormal).m_Value[3].z = pfVar3[2];
  }
  if ((this->m_PaintPowerHudInfoValidity).m_Value[3] != false) {
    (*this->_vptr_CPortalPlayerLocalData[1])(this,(this->m_PaintPowerHudInfoValidity).m_Value + 3);
    (this->m_PaintPowerHudInfoValidity).m_Value[3] = false;
  }
  fVar20 = *(float *)(unaff_EBX + 0x70ac87);
  if ((((fVar20 != (this->m_StickNormal).
                   super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StickNormal>
                   .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StickNormal>.
                   m_Value.x) ||
       (NAN(fVar20) ||
        NAN((this->m_StickNormal).
            super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StickNormal>.
            super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StickNormal>.m_Value.x
           ))) ||
      (pvVar1 = &(this->m_StickNormal).
                 super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StickNormal>
                 .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StickNormal>.
                 m_Value.y, *(float *)(unaff_EBX + 0x70ac8b) != *pvVar1)) ||
     ((NAN(*(float *)(unaff_EBX + 0x70ac8b)) || NAN(*pvVar1) ||
      (*(float *)(unaff_EBX + 0x70ac8f) !=
       (this->m_StickNormal).
       super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StickNormal>.
       super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StickNormal>.m_Value.z))))
  {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_StickNormal);
    (this->m_StickNormal).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StickNormal>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StickNormal>.m_Value.x =
         *(vec_t *)(unaff_EBX + 0x70ac87);
    (this->m_StickNormal).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StickNormal>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StickNormal>.m_Value.y =
         *(vec_t *)(unaff_EBX + 0x70ac8b);
    (this->m_StickNormal).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StickNormal>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StickNormal>.m_Value.z =
         *(vec_t *)(unaff_EBX + 0x70ac8f);
    fVar20 = *(float *)(unaff_EBX + 0x70ac87);
  }
  if (((fVar20 != (this->m_OldStickNormal).
                  super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_OldStickNormal>
                  .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_OldStickNormal>
                  .m_Value.x) ||
      (NAN(fVar20) ||
       NAN((this->m_OldStickNormal).
           super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_OldStickNormal>
           .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_OldStickNormal>.
           m_Value.x))) ||
     ((pvVar1 = &(this->m_OldStickNormal).
                 super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_OldStickNormal>
                 .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_OldStickNormal>.
                 m_Value.y, *(float *)(unaff_EBX + 0x70ac8b) != *pvVar1 ||
      ((NAN(*(float *)(unaff_EBX + 0x70ac8b)) || NAN(*pvVar1) ||
       (*(float *)(unaff_EBX + 0x70ac8f) !=
        (this->m_OldStickNormal).
        super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_OldStickNormal>.
        super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_OldStickNormal>.m_Value.z)
       ))))) {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_OldStickNormal);
    (this->m_OldStickNormal).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_OldStickNormal>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_OldStickNormal>.m_Value.x =
         *(vec_t *)(unaff_EBX + 0x70ac87);
    (this->m_OldStickNormal).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_OldStickNormal>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_OldStickNormal>.m_Value.y =
         *(vec_t *)(unaff_EBX + 0x70ac8b);
    (this->m_OldStickNormal).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_OldStickNormal>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_OldStickNormal>.m_Value.z =
         *(vec_t *)(unaff_EBX + 0x70ac8f);
  }
  if ((((*pfVar3 !=
         (this->m_vPreUpdateVelocity).
         super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vPreUpdateVelocity>
         .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vPreUpdateVelocity>.
         m_Value.x) ||
       (NAN(*pfVar3) ||
        NAN((this->m_vPreUpdateVelocity).
            super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vPreUpdateVelocity>
            .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vPreUpdateVelocity>.
            m_Value.x))) ||
      (pvVar1 = &(this->m_vPreUpdateVelocity).
                 super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vPreUpdateVelocity>
                 .
                 super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vPreUpdateVelocity>
                 .m_Value.y, pfVar3[1] != *pvVar1)) ||
     ((NAN(pfVar3[1]) || NAN(*pvVar1) ||
      (pfVar3[2] !=
       (this->m_vPreUpdateVelocity).
       super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vPreUpdateVelocity>
       .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vPreUpdateVelocity>.
       m_Value.z)))) {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_vPreUpdateVelocity);
    (this->m_vPreUpdateVelocity).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vPreUpdateVelocity>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vPreUpdateVelocity>.m_Value.x
         = *pfVar3;
    (this->m_vPreUpdateVelocity).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vPreUpdateVelocity>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vPreUpdateVelocity>.m_Value.y
         = pfVar3[1];
    (this->m_vPreUpdateVelocity).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vPreUpdateVelocity>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vPreUpdateVelocity>.m_Value.z
         = pfVar3[2];
  }
  if (((*(float *)(unaff_EBX + 0x70ac87) !=
        (this->m_Up).super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_Up>.
        super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_Up>.m_Value.x) ||
      (NAN(*(float *)(unaff_EBX + 0x70ac87)) ||
       NAN((this->m_Up).
           super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_Up>.
           super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_Up>.m_Value.x))) ||
     ((pvVar1 = &(this->m_Up).
                 super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_Up>.
                 super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_Up>.m_Value.y,
      *(float *)(unaff_EBX + 0x70ac8b) != *pvVar1 ||
      ((NAN(*(float *)(unaff_EBX + 0x70ac8b)) || NAN(*pvVar1) ||
       (*(float *)(unaff_EBX + 0x70ac8f) !=
        (this->m_Up).super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_Up>.
        super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_Up>.m_Value.z)))))) {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_Up);
    (this->m_Up).super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_Up>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_Up>.m_Value.x =
         *(vec_t *)(unaff_EBX + 0x70ac87);
    (this->m_Up).super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_Up>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_Up>.m_Value.y =
         *(vec_t *)(unaff_EBX + 0x70ac8b);
    (this->m_Up).super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_Up>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_Up>.m_Value.z =
         *(vec_t *)(unaff_EBX + 0x70ac8f);
  }
  if (((((this->m_vStickRotationAxis).
         super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vStickRotationAxis>
         .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vStickRotationAxis>.
         m_Value.x != 0.0) ||
       (pvVar1 = &(this->m_vStickRotationAxis).
                  super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vStickRotationAxis>
                  .
                  super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vStickRotationAxis>
                  .m_Value.y, *pvVar1 != 0.0)) || (NAN(*pvVar1))) ||
     ((this->m_vStickRotationAxis).
      super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vStickRotationAxis>
      .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vStickRotationAxis>.m_Value
      .z != 0.0)) {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_vStickRotationAxis);
    (this->m_vStickRotationAxis).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vStickRotationAxis>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vStickRotationAxis>.m_Value.x
         = 0.0;
    (this->m_vStickRotationAxis).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vStickRotationAxis>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vStickRotationAxis>.m_Value.y
         = 0.0;
    (this->m_vStickRotationAxis).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vStickRotationAxis>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vStickRotationAxis>.m_Value.z
         = 0.0;
  }
  puVar4 = *(undefined4 **)(unaff_EBX + 0x50320f);
  iVar9 = (**(code **)(*(int *)*puVar4 + 0x7c))((int *)*puVar4);
  if (((*(float *)(iVar9 + 0xc) !=
        (this->m_StandHullMin).
        super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMin>.
        super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMin>.m_Value.x)
      || (NAN(*(float *)(iVar9 + 0xc)) ||
          NAN((this->m_StandHullMin).
              super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMin>
              .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMin>.
              m_Value.x))) ||
     ((pvVar1 = &(this->m_StandHullMin).
                 super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMin>
                 .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMin>.
                 m_Value.y, *(float *)(iVar9 + 0x10) != *pvVar1 ||
      ((NAN(*(float *)(iVar9 + 0x10)) || NAN(*pvVar1) ||
       (*(float *)(iVar9 + 0x14) !=
        (this->m_StandHullMin).
        super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMin>.
        super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMin>.m_Value.z)))
      ))) {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_StandHullMin);
    (this->m_StandHullMin).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMin>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMin>.m_Value.x =
         *(vec_t *)(iVar9 + 0xc);
    (this->m_StandHullMin).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMin>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMin>.m_Value.y =
         *(vec_t *)(iVar9 + 0x10);
    (this->m_StandHullMin).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMin>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMin>.m_Value.z =
         *(vec_t *)(iVar9 + 0x14);
  }
  iVar9 = (**(code **)(*(int *)*puVar4 + 0x7c))((int *)*puVar4);
  if ((((*(float *)(iVar9 + 0x18) !=
         (this->m_StandHullMax).
         super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMax>.
         super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMax>.m_Value.x)
       || (NAN(*(float *)(iVar9 + 0x18)) ||
           NAN((this->m_StandHullMax).
               super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMax>
               .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMax>.
               m_Value.x))) ||
      (pvVar1 = &(this->m_StandHullMax).
                 super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMax>
                 .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMax>.
                 m_Value.y, *(float *)(iVar9 + 0x1c) != *pvVar1)) ||
     ((NAN(*(float *)(iVar9 + 0x1c)) || NAN(*pvVar1) ||
      (*(float *)(iVar9 + 0x20) !=
       (this->m_StandHullMax).
       super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMax>.
       super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMax>.m_Value.z))))
  {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_StandHullMax);
    (this->m_StandHullMax).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMax>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMax>.m_Value.x =
         *(vec_t *)(iVar9 + 0x18);
    (this->m_StandHullMax).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMax>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMax>.m_Value.y =
         *(vec_t *)(iVar9 + 0x1c);
    (this->m_StandHullMax).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMax>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_StandHullMax>.m_Value.z =
         *(vec_t *)(iVar9 + 0x20);
  }
  iVar9 = (**(code **)(*(int *)*puVar4 + 0x7c))((int *)*puVar4);
  if (((*(float *)(iVar9 + 0x24) !=
        (this->m_DuckHullMin).
        super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMin>.
        super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMin>.m_Value.x) ||
      (NAN(*(float *)(iVar9 + 0x24)) ||
       NAN((this->m_DuckHullMin).
           super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMin>.
           super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMin>.m_Value.x)
      )) || ((pvVar1 = &(this->m_DuckHullMin).
                        super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMin>
                        .
                        super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMin>
                        .m_Value.y, *(float *)(iVar9 + 0x28) != *pvVar1 ||
             ((NAN(*(float *)(iVar9 + 0x28)) || NAN(*pvVar1) ||
              (*(float *)(iVar9 + 0x2c) !=
               (this->m_DuckHullMin).
               super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMin>
               .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMin>.
               m_Value.z)))))) {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_DuckHullMin);
    (this->m_DuckHullMin).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMin>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMin>.m_Value.x =
         *(vec_t *)(iVar9 + 0x24);
    (this->m_DuckHullMin).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMin>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMin>.m_Value.y =
         *(vec_t *)(iVar9 + 0x28);
    (this->m_DuckHullMin).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMin>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMin>.m_Value.z =
         *(vec_t *)(iVar9 + 0x2c);
  }
  iVar9 = (**(code **)(*(int *)*puVar4 + 0x7c))((int *)*puVar4);
  if ((((*(float *)(iVar9 + 0x30) !=
         (this->m_DuckHullMax).
         super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMax>.
         super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMax>.m_Value.x)
       || (NAN(*(float *)(iVar9 + 0x30)) ||
           NAN((this->m_DuckHullMax).
               super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMax>
               .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMax>.
               m_Value.x))) ||
      (pvVar1 = &(this->m_DuckHullMax).
                 super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMax>
                 .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMax>.
                 m_Value.y, *(float *)(iVar9 + 0x34) != *pvVar1)) ||
     ((NAN(*(float *)(iVar9 + 0x34)) || NAN(*pvVar1) ||
      (*(float *)(iVar9 + 0x38) !=
       (this->m_DuckHullMax).
       super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMax>.
       super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMax>.m_Value.z))))
  {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_DuckHullMax);
    (this->m_DuckHullMax).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMax>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMax>.m_Value.x =
         *(vec_t *)(iVar9 + 0x30);
    (this->m_DuckHullMax).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMax>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMax>.m_Value.y =
         *(vec_t *)(iVar9 + 0x34);
    (this->m_DuckHullMax).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMax>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_DuckHullMax>.m_Value.z =
         *(vec_t *)(iVar9 + 0x38);
  }
  iVar9 = (**(code **)(*(int *)*puVar4 + 0x7c))((int *)*puVar4);
  if (((*(float *)(iVar9 + 0xc) !=
        (this->m_CachedStandHullMinAttempt).
        super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMinAttempt>
        .
        super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMinAttempt>
        .m_Value.x) ||
      (NAN(*(float *)(iVar9 + 0xc)) ||
       NAN((this->m_CachedStandHullMinAttempt).
           super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMinAttempt>
           .
           super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMinAttempt>
           .m_Value.x))) ||
     ((pvVar1 = &(this->m_CachedStandHullMinAttempt).
                 super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMinAttempt>
                 .
                 super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMinAttempt>
                 .m_Value.y, *(float *)(iVar9 + 0x10) != *pvVar1 ||
      ((NAN(*(float *)(iVar9 + 0x10)) || NAN(*pvVar1) ||
       (*(float *)(iVar9 + 0x14) !=
        (this->m_CachedStandHullMinAttempt).
        super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMinAttempt>
        .
        super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMinAttempt>
        .m_Value.z)))))) {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_CachedStandHullMinAttempt);
    (this->m_CachedStandHullMinAttempt).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMinAttempt>
    .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMinAttempt>.
    m_Value.x = *(vec_t *)(iVar9 + 0xc);
    (this->m_CachedStandHullMinAttempt).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMinAttempt>
    .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMinAttempt>.
    m_Value.y = *(vec_t *)(iVar9 + 0x10);
    (this->m_CachedStandHullMinAttempt).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMinAttempt>
    .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMinAttempt>.
    m_Value.z = *(vec_t *)(iVar9 + 0x14);
  }
  iVar9 = (**(code **)(*(int *)*puVar4 + 0x7c))((int *)*puVar4);
  if ((((*(float *)(iVar9 + 0x18) !=
         (this->m_CachedStandHullMaxAttempt).
         super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMaxAttempt>
         .
         super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMaxAttempt>
         .m_Value.x) ||
       (NAN(*(float *)(iVar9 + 0x18)) ||
        NAN((this->m_CachedStandHullMaxAttempt).
            super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMaxAttempt>
            .
            super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMaxAttempt>
            .m_Value.x))) ||
      (pvVar1 = &(this->m_CachedStandHullMaxAttempt).
                 super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMaxAttempt>
                 .
                 super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMaxAttempt>
                 .m_Value.y, *(float *)(iVar9 + 0x1c) != *pvVar1)) ||
     ((NAN(*(float *)(iVar9 + 0x1c)) || NAN(*pvVar1) ||
      (*(float *)(iVar9 + 0x20) !=
       (this->m_CachedStandHullMaxAttempt).
       super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMaxAttempt>
       .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMaxAttempt>
       .m_Value.z)))) {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_CachedStandHullMaxAttempt);
    (this->m_CachedStandHullMaxAttempt).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMaxAttempt>
    .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMaxAttempt>.
    m_Value.x = *(vec_t *)(iVar9 + 0x18);
    (this->m_CachedStandHullMaxAttempt).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMaxAttempt>
    .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMaxAttempt>.
    m_Value.y = *(vec_t *)(iVar9 + 0x1c);
    (this->m_CachedStandHullMaxAttempt).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMaxAttempt>
    .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedStandHullMaxAttempt>.
    m_Value.z = *(vec_t *)(iVar9 + 0x20);
  }
  iVar9 = (**(code **)(*(int *)*puVar4 + 0x7c))((int *)*puVar4);
  if (((*(float *)(iVar9 + 0x24) !=
        (this->m_CachedDuckHullMinAttempt).
        super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMinAttempt>
        .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMinAttempt>
        .m_Value.x) ||
      (NAN(*(float *)(iVar9 + 0x24)) ||
       NAN((this->m_CachedDuckHullMinAttempt).
           super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMinAttempt>
           .
           super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMinAttempt>
           .m_Value.x))) ||
     ((pvVar1 = &(this->m_CachedDuckHullMinAttempt).
                 super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMinAttempt>
                 .
                 super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMinAttempt>
                 .m_Value.y, *(float *)(iVar9 + 0x28) != *pvVar1 ||
      ((NAN(*(float *)(iVar9 + 0x28)) || NAN(*pvVar1) ||
       (*(float *)(iVar9 + 0x2c) !=
        (this->m_CachedDuckHullMinAttempt).
        super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMinAttempt>
        .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMinAttempt>
        .m_Value.z)))))) {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_CachedDuckHullMinAttempt);
    (this->m_CachedDuckHullMinAttempt).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMinAttempt>
    .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMinAttempt>.
    m_Value.x = *(vec_t *)(iVar9 + 0x24);
    (this->m_CachedDuckHullMinAttempt).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMinAttempt>
    .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMinAttempt>.
    m_Value.y = *(vec_t *)(iVar9 + 0x28);
    (this->m_CachedDuckHullMinAttempt).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMinAttempt>
    .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMinAttempt>.
    m_Value.z = *(vec_t *)(iVar9 + 0x2c);
  }
  iVar9 = (**(code **)(*(int *)*puVar4 + 0x7c))((int *)*puVar4);
  if ((((*(float *)(iVar9 + 0x30) !=
         (this->m_CachedDuckHullMaxAttempt).
         super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMaxAttempt>
         .
         super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMaxAttempt>
         .m_Value.x) ||
       (NAN(*(float *)(iVar9 + 0x30)) ||
        NAN((this->m_CachedDuckHullMaxAttempt).
            super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMaxAttempt>
            .
            super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMaxAttempt>
            .m_Value.x))) ||
      (pvVar1 = &(this->m_CachedDuckHullMaxAttempt).
                 super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMaxAttempt>
                 .
                 super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMaxAttempt>
                 .m_Value.y, *(float *)(iVar9 + 0x34) != *pvVar1)) ||
     ((NAN(*(float *)(iVar9 + 0x34)) || NAN(*pvVar1) ||
      (*(float *)(iVar9 + 0x38) !=
       (this->m_CachedDuckHullMaxAttempt).
       super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMaxAttempt>
       .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMaxAttempt>.
       m_Value.z)))) {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_CachedDuckHullMaxAttempt);
    (this->m_CachedDuckHullMaxAttempt).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMaxAttempt>
    .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMaxAttempt>.
    m_Value.x = *(vec_t *)(iVar9 + 0x30);
    (this->m_CachedDuckHullMaxAttempt).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMaxAttempt>
    .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMaxAttempt>.
    m_Value.y = *(vec_t *)(iVar9 + 0x34);
    (this->m_CachedDuckHullMaxAttempt).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMaxAttempt>
    .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_CachedDuckHullMaxAttempt>.
    m_Value.z = *(vec_t *)(iVar9 + 0x38);
    cVar2 = (this->m_bAttemptHullResize).m_Value;
  }
  else {
    cVar2 = (this->m_bAttemptHullResize).m_Value;
  }
  if (cVar2 != '\0') {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_bAttemptHullResize);
    (this->m_bAttemptHullResize).m_Value = false;
  }
  if ((((this->m_vLocalUp).
        super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vLocalUp>.
        super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vLocalUp>.m_Value.x != 0.0
       ) || (pvVar1 = &(this->m_vLocalUp).
                       super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vLocalUp>
                       .super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vLocalUp>.
                       m_Value.y, *pvVar1 != 0.0)) ||
     ((NAN(*pvVar1) ||
      (*(float *)(unaff_EBX + 0x388f7f) !=
       (this->m_vLocalUp).
       super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vLocalUp>.
       super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vLocalUp>.m_Value.z)))) {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_vLocalUp);
    (this->m_vLocalUp).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vLocalUp>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vLocalUp>.m_Value.x = 0.0;
    (this->m_vLocalUp).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vLocalUp>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vLocalUp>.m_Value.y = 0.0;
    (this->m_vLocalUp).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vLocalUp>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vLocalUp>.m_Value.z = 1.0;
  }
  if (((((this->m_vEyeOffset).
         super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeOffset>.
         super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeOffset>.m_Value.x !=
         0.0) || (pvVar1 = &(this->m_vEyeOffset).
                            super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeOffset>
                            .
                            super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeOffset>
                            .m_Value.y, *pvVar1 != 0.0)) || (NAN(*pvVar1))) ||
     ((this->m_vEyeOffset).
      super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeOffset>.
      super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeOffset>.m_Value.z != 0.0
     )) {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_vEyeOffset);
    (this->m_vEyeOffset).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeOffset>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeOffset>.m_Value.x = 0.0;
    (this->m_vEyeOffset).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeOffset>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeOffset>.m_Value.y = 0.0;
    (this->m_vEyeOffset).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeOffset>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeOffset>.m_Value.z = 0.0;
  }
  if ((((this->m_vEyeUpOffset).
        super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeUpOffset>.
        super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeUpOffset>.m_Value.x !=
        0.0) || (pvVar1 = &(this->m_vEyeUpOffset).
                           super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeUpOffset>
                           .
                           super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeUpOffset>
                           .m_Value.y, *pvVar1 != 0.0)) ||
     ((NAN(*pvVar1) ||
      ((this->m_vEyeUpOffset).
       super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeUpOffset>.
       super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeUpOffset>.m_Value.z !=
       0.0)))) {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_vEyeUpOffset);
    (this->m_vEyeUpOffset).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeUpOffset>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeUpOffset>.m_Value.x = 0.0;
    (this->m_vEyeUpOffset).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeUpOffset>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeUpOffset>.m_Value.y = 0.0;
    (this->m_vEyeUpOffset).
    super_CNetworkVectorCommonBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeUpOffset>.
    super_CNetworkVarBase<Vector,CPortalPlayerLocalData::NetworkVar_m_vEyeUpOffset>.m_Value.z = 0.0;
  }
  if ((((this->m_qQuaternionPunch).
        super_CNetworkVectorCommonBase<QAngle,CPortalPlayerLocalData::NetworkVar_m_qQuaternionPunch>
        .super_CNetworkVarBase<QAngle,CPortalPlayerLocalData::NetworkVar_m_qQuaternionPunch>.m_Value
        .x != 0.0) ||
      (pvVar1 = &(this->m_qQuaternionPunch).
                 super_CNetworkVectorCommonBase<QAngle,CPortalPlayerLocalData::NetworkVar_m_qQuaternionPunch>
                 .
                 super_CNetworkVarBase<QAngle,CPortalPlayerLocalData::NetworkVar_m_qQuaternionPunch>
                 .m_Value.y, *pvVar1 != 0.0)) ||
     ((NAN(*pvVar1) ||
      ((this->m_qQuaternionPunch).
       super_CNetworkVectorCommonBase<QAngle,CPortalPlayerLocalData::NetworkVar_m_qQuaternionPunch>.
       super_CNetworkVarBase<QAngle,CPortalPlayerLocalData::NetworkVar_m_qQuaternionPunch>.m_Value.z
       != 0.0)))) {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_qQuaternionPunch);
    (this->m_qQuaternionPunch).
    super_CNetworkVectorCommonBase<QAngle,CPortalPlayerLocalData::NetworkVar_m_qQuaternionPunch>.
    super_CNetworkVarBase<QAngle,CPortalPlayerLocalData::NetworkVar_m_qQuaternionPunch>.m_Value.x =
         0.0;
    (this->m_qQuaternionPunch).
    super_CNetworkVectorCommonBase<QAngle,CPortalPlayerLocalData::NetworkVar_m_qQuaternionPunch>.
    super_CNetworkVarBase<QAngle,CPortalPlayerLocalData::NetworkVar_m_qQuaternionPunch>.m_Value.y =
         0.0;
    (this->m_qQuaternionPunch).
    super_CNetworkVectorCommonBase<QAngle,CPortalPlayerLocalData::NetworkVar_m_qQuaternionPunch>.
    super_CNetworkVarBase<QAngle,CPortalPlayerLocalData::NetworkVar_m_qQuaternionPunch>.m_Value.z =
         0.0;
  }
  if ((this->m_PaintedPowerType).m_Value != NO_POWER) {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_PaintedPowerType);
    (this->m_PaintedPowerType).m_Value = NO_POWER;
  }
  if (*(float *)(unaff_EBX + 0x388f4f) !=
      (this->m_PaintedPowerTimer).super_CountdownTimer.m_timestamp.m_Value) {
                    /* Unresolved local var: CountdownTimer * pThis@[???] */
    (*(pNVar7->super_CountdownTimer)._vptr_CountdownTimer[1])(pNVar7,pCVar5);
    (this->m_PaintedPowerTimer).super_CountdownTimer.m_timestamp.m_Value = -1.0;
  }
  if (*(float *)(unaff_EBX + 0x388f7f) != (this->m_flAirInputScale).m_Value) {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_flAirInputScale);
    (this->m_flAirInputScale).m_Value = 1.0;
  }
  fVar20 = *(float *)(*(int *)(*(int *)(unaff_EBX + 0x503d1b) + 0x1c) + 0x2c);
  if (fVar20 != (this->m_flCurrentStickTime).m_Value) {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_flCurrentStickTime);
    (this->m_flCurrentStickTime).m_Value = fVar20;
  }
  if ((this->m_nStickCameraState).m_Value != STICK_CAMERA_UPRIGHT) {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_nStickCameraState);
    (this->m_nStickCameraState).m_Value = STICK_CAMERA_UPRIGHT;
  }
  if ((this->m_InAirState).m_Value == ON_GROUND) {
    cVar2 = (this->m_bJumpedThisFrame).m_Value;
  }
  else {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_InAirState);
    (this->m_InAirState).m_Value = ON_GROUND;
    cVar2 = (this->m_bJumpedThisFrame).m_Value;
  }
  if (cVar2 != '\0') {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_bJumpedThisFrame);
    (this->m_bJumpedThisFrame).m_Value = false;
  }
  if ((this->m_bDoneStickInterp).m_Value == false) {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_bDoneStickInterp);
    (this->m_bDoneStickInterp).m_Value = true;
  }
  if ((this->m_bDoneCorrectPitch).m_Value == false) {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_bDoneCorrectPitch);
    (this->m_bDoneCorrectPitch).m_Value = true;
  }
  if ((this->m_bDuckedInAir).m_Value == false) {
    cVar2 = (this->m_bBounced).m_Value;
  }
  else {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_bDuckedInAir);
    (this->m_bDuckedInAir).m_Value = false;
    cVar2 = (this->m_bBounced).m_Value;
  }
  if (cVar2 != '\0') {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_bBounced);
    (this->m_bBounced).m_Value = false;
  }
  if ((this->m_bInTractorBeam).m_Value != false) {
                    /* Unresolved local var: CPortalPlayerLocalData * pThis@[???] */
    (*this->_vptr_CPortalPlayerLocalData[1])(this,&this->m_bInTractorBeam);
    (this->m_bInTractorBeam).m_Value = false;
  }
  return;
}


/* CPortalPlayerLocalData::CPortalPlayerLocalData at 00744ca0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void CPortalPlayerLocalData(CPortalPlayerLocalData * this,
   CPortalPlayerLocalData * this) */

void __thiscall
CPortalPlayerLocalData::CPortalPlayerLocalData
          (CPortalPlayerLocalData *this,CPortalPlayerLocalData *this_1)

{
  CPortalPlayerLocalData(this);
  return;
}


/* _GLOBAL__I__ZN14DT_PortalLocal11g_SendTableE at 000b13b0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN14DT_PortalLocal11g_SendTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

