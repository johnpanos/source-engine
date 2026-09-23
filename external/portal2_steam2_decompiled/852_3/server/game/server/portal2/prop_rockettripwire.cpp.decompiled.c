/* DWARF-guided pseudocode for game/server/portal2/prop_rockettripwire.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* DataMapInit<CPropRocketTripwire> at 000cda70 */

datamap_t * DataMapInit<CPropRocketTripwire>(CPropRocketTripwire *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  char *pcVar4;
  size_t sVar5;
  char *pcVar6;
  int iVar7;
  undefined4 *puVar8;
  int iVar9;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((speechListSounds[0].name[unaff_EBX + 0x16] == '\0') &&
     (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xcedcda), iVar3 != 0)) {
    *(int *)(speechListSounds[0].name + unaff_EBX + 0x26) = unaff_EBX + 0x945a17;
    pcVar4 = speechListSounds[0].name + unaff_EBX + 0x2e;
    pcVar4[0] = '\0';
    pcVar4[1] = '\0';
    pcVar4[2] = '\0';
    pcVar4[3] = '\0';
    pcVar4 = speechListSounds[0].name + unaff_EBX + 0x32;
    pcVar4[0] = '\0';
    pcVar4[1] = '\0';
    pcVar4[2] = '\0';
    pcVar4[3] = '\0';
    pcVar4 = speechListSounds[0].name + unaff_EBX + 0x36;
    pcVar4[0] = '\0';
    pcVar4[1] = '\0';
    pcVar4[2] = '\0';
    pcVar4[3] = '\0';
    pcVar4 = speechListSounds[0].name + unaff_EBX + 0x3a;
    pcVar4[0] = '\0';
    pcVar4[1] = '\0';
    pcVar4[2] = '\0';
    pcVar4[3] = '\0';
    pcVar4 = speechListSounds[0].name + unaff_EBX + 0x3e;
    pcVar4[0] = '\0';
    pcVar4[1] = '\0';
    pcVar4[2] = '\0';
    pcVar4[3] = '\0';
    pcVar4 = speechListSounds[0].name + unaff_EBX + 0x2a;
    pcVar4[0] = '\x13';
    pcVar4[1] = '\0';
    pcVar4[2] = '\0';
    pcVar4[3] = '\0';
    ___cxa_guard_release(unaff_EBX + 0xcedcda);
    ___cxa_atexit(unaff_EBX + 0x8cf7a2,0,*(undefined4 *)(&DAT_00ade112 + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_DispatchResponse_00c02c8c + unaff_EBX + 2) =
       *(undefined4 *)(&DAT_00ade11e + unaff_EBX);
  if ((speechListSounds[0].name[unaff_EBX + 0x1e] == '\0') &&
     (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xcedce2), iVar3 != 0)) {
    uVar1 = **(undefined4 **)(&DAT_00ade12e + unaff_EBX);
    *(undefined4 *)((int)&PTR_MyNPCPointer_00c03034 + unaff_EBX + 2) = uVar1;
    *(undefined4 *)((int)&PTR_MyCombatWeaponPointer_00c03074 + unaff_EBX + 2) = uVar1;
    pcVar4 = operator_new___(*(int *)(speechListSounds[0].name + unaff_EBX + 0x2a) + 0x10);
    _strcpy(pcVar4,*(char **)(speechListSounds[0].name + unaff_EBX + 0x26));
    sVar5 = _strlen(pcVar4);
    pcVar6 = pcVar4 + sVar5;
    pcVar6[0] = 'F';
    pcVar6[1] = 'i';
    pcVar6[2] = 'r';
    pcVar6[3] = 'e';
    pcVar6[4] = 'R';
    pcVar6[5] = 'o';
    pcVar6[6] = 'c';
    pcVar6[7] = 'k';
    pcVar6[8] = 'e';
    pcVar6[9] = 't';
    pcVar6[10] = 'T';
    pcVar6[0xb] = 'h';
    *(undefined **)(pcVar6 + 0xc) = &DAT_006b6e69;
    iVar2 = *(int *)(speechListSounds[0].name + unaff_EBX + 0x3a);
    iVar3 = iVar2 + 1;
    if (*(int *)(speechListSounds[0].name + unaff_EBX + 0x32) < iVar3) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)(speechListSounds[0].name + unaff_EBX + 0x2e),
                 iVar3 - *(int *)(speechListSounds[0].name + unaff_EBX + 0x32));
    }
    iVar7 = *(int *)(speechListSounds[0].name + unaff_EBX + 0x3a) + 1;
    *(int *)(speechListSounds[0].name + unaff_EBX + 0x3a) = iVar7;
    iVar9 = *(int *)(speechListSounds[0].name + unaff_EBX + 0x2e);
    *(int *)(speechListSounds[0].name + unaff_EBX + 0x3e) = iVar9;
    iVar7 = (iVar7 - iVar2) + -1;
    if (0 < iVar7) {
      _V_memmove((void *)(iVar9 + iVar3 * 4),(void *)(iVar9 + iVar2 * 4),iVar7 * 4);
      iVar9 = *(int *)(speechListSounds[0].name + unaff_EBX + 0x2e);
    }
    puVar8 = (undefined4 *)(iVar2 * 4 + iVar9);
    if (puVar8 != (undefined4 *)0x0) {
      *puVar8 = pcVar4;
    }
    *(char **)((int)&PTR_GetAttackDamageScale_00c03164 + unaff_EBX + 2) = pcVar4;
    pcVar4 = operator_new___(*(int *)(speechListSounds[0].name + unaff_EBX + 0x2a) + 0xd);
    _strcpy(pcVar4,*(char **)(speechListSounds[0].name + unaff_EBX + 0x26));
    sVar5 = _strlen(pcVar4);
    builtin_strncpy(pcVar4 + sVar5,"WaitingThink",0xd);
    iVar2 = *(int *)(speechListSounds[0].name + unaff_EBX + 0x3a);
    iVar3 = iVar2 + 1;
    if (*(int *)(speechListSounds[0].name + unaff_EBX + 0x32) < iVar3) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)(speechListSounds[0].name + unaff_EBX + 0x2e),
                 iVar3 - *(int *)(speechListSounds[0].name + unaff_EBX + 0x32));
    }
    iVar7 = *(int *)(speechListSounds[0].name + unaff_EBX + 0x3a) + 1;
    *(int *)(speechListSounds[0].name + unaff_EBX + 0x3a) = iVar7;
    iVar9 = *(int *)(speechListSounds[0].name + unaff_EBX + 0x2e);
    *(int *)(speechListSounds[0].name + unaff_EBX + 0x3e) = iVar9;
    iVar7 = (iVar7 - iVar2) + -1;
    if (0 < iVar7) {
      _V_memmove((void *)(iVar9 + iVar3 * 4),(void *)(iVar9 + iVar2 * 4),iVar7 * 4);
      iVar9 = *(int *)(speechListSounds[0].name + unaff_EBX + 0x2e);
    }
    puVar8 = (undefined4 *)(iVar2 * 4 + iVar9);
    if (puVar8 != (undefined4 *)0x0) {
      *puVar8 = pcVar4;
    }
    *(char **)((int)&PTR_VPhysicsFriction_00c031a4 + unaff_EBX + 2) = pcVar4;
    ___cxa_guard_release(unaff_EBX + 0xcedce2);
  }
  *(undefined4 *)((int)&PTR_GetBaseAnimatingOverlay_00c02c84 + unaff_EBX + 2) = 0x14;
  *(int *)((int)&PTR_GetBaseAnimating_00c02c80 + unaff_EBX + 2) = unaff_EBX + 0xc02ce2;
  return (datamap_t *)((int)&PTR_GetBaseAnimating_00c02c80 + unaff_EBX + 2);
}


/* __static_initialization_and_destruction_0 at 000cdd70 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  undefined1 *puVar1;
  char *s2;
  undefined4 *puVar2;
  IEntityFactoryDictionary *pIVar3;
  datamap_t *pdVar4;
  int iVar5;
  int unaff_EBX;
  undefined4 *puVar6;
  undefined4 *puVar7;
  longlong lVar8;
  
  lVar8 = ___i686_get_pc_thunk_bx();
  if (lVar8 == 0xffff00000001) {
    (&DAT_00ced75c)[unaff_EBX] = 0;
    (&DAT_00ced75d)[unaff_EBX] = 0;
    (&DAT_00ced75e)[unaff_EBX] = 0;
    (&DAT_00ced75f)[unaff_EBX] = 0;
    *(undefined4 *)
     ((int)&CUtlRBTree<CUtlMap<char_const*,BasicGameStatsRecord_t,unsigned_short>::Node_t,unsigned_short,CUtlMap<char_const*,BasicGameStatsRecord_t,unsigned_short>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<char_const*,BasicGameStatsRecord_t,unsigned_short>::Node_t,unsigned_short>,unsigned_short>>
            ::Links(unsigned_short)::s_Sentinel + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00ced764 + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00ced768 + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00ced76c + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)
     ((int)&CUtlRBTree<CBreakModelsPrecached::breakable_precache_t,unsigned_short,bool(*)(CBreakModelsPrecached::breakable_precache_t_const&,CBreakModelsPrecached::breakable_precache_t_const&),CUtlMemory<UtlRBTreeNode_t<CBreakModelsPrecached::breakable_precache_t,unsigned_short>,unsigned_short>>
            ::Links(unsigned_short)::s_Sentinel + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00ced774 + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00ced778 + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00ced77c + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&CUtlRBTree<CBreakModelsPrecached::breakable_precache_t,unsigned_short,bool(*)(CBreakModelsPrecached::breakable_precache_t_const&,CBreakModelsPrecached::breakable_precache_t_const&),CUtlMemory<UtlRBTreeNode_t<CBreakModelsPrecached::breakable_precache_t,unsigned_short>,unsigned_short>>
            ::Links(unsigned_short)::s_Sentinel + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00ced784 + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00ced788 + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00ced78c + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00ced790 + unaff_EBX) = 0;
    (&DAT_00ced797)[unaff_EBX] = 0;
    (&DAT_00ced794)[unaff_EBX] = 0;
    (&DAT_00ced795)[unaff_EBX] = 0;
    (&DAT_00ced796)[unaff_EBX] = 0;
    *(undefined4 *)(&DAT_00ced798 + unaff_EBX) = 0;
    (&DAT_00ced79f)[unaff_EBX] = 0;
    (&DAT_00ced79c)[unaff_EBX] = 0;
    (&DAT_00ced79d)[unaff_EBX] = 0;
    (&DAT_00ced79e)[unaff_EBX] = 0;
    *(undefined4 *)((int)&g_interactionHitByPlayerThrownPhysObj + unaff_EBX) = 0;
    *(undefined1 *)((int)&g_interactionPlayerPuntedHeavyObject + unaff_EBX + 3) = 1;
    *(undefined1 *)((int)&g_interactionPlayerPuntedHeavyObject + unaff_EBX) = 0;
    *(undefined1 *)((int)&g_interactionPlayerPuntedHeavyObject + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&g_interactionPlayerPuntedHeavyObject + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&g_ActiveGibCount + unaff_EBX) = 1;
    (&DAT_00ced7af)[unaff_EBX] = 0;
    (&DAT_00ced7ac)[unaff_EBX] = 0;
    (&DAT_00ced7ad)[unaff_EBX] = 0;
    (&DAT_00ced7ae)[unaff_EBX] = 0;
    *(undefined4 *)(&DAT_00ced7b0 + unaff_EBX) = 2;
    (&DAT_00ced7b7)[unaff_EBX] = 0;
    (&DAT_00ced7b4)[unaff_EBX] = 0;
    (&DAT_00ced7b5)[unaff_EBX] = 0;
    (&DAT_00ced7b6)[unaff_EBX] = 0;
    *(undefined **)(&DAT_00ced7b8 + unaff_EBX) = &UNK_00ae35a4 + unaff_EBX;
    *(undefined **)(&DAT_00ced7bc + unaff_EBX) = &UNK_00b714cc + unaff_EBX;
    pIVar3 = EntityFactoryDictionary();
    (**pIVar3->_vptr_IEntityFactoryDictionary)
              (pIVar3,&DAT_00ced7bc + unaff_EBX,&UNK_00945725 + unaff_EBX);
    pdVar4 = DataMapInit<CPropRocketTripwire>((CPropRocketTripwire *)0x0);
    *(datamap_t **)((int)&PTR_SetText_00c62798 + unaff_EBX) = pdVar4;
    puVar1 = &GetStartBitMask(int)::g_StartMask + unaff_EBX;
    s2 = &UNK_00945711 + unaff_EBX;
    *(char **)(&GetStartBitMask(int)::g_StartMask + unaff_EBX) = s2;
    *(SendTable **)(&DAT_00ced7c4 + unaff_EBX) =
         (SendTable *)((int)&PTR_SetText_00c6279c + unaff_EBX);
    *(undefined4 *)(&DAT_00ced7d0 + unaff_EBX) = 0xffff;
    puVar7 = (undefined4 *)**(int **)(&DAT_00addeac + unaff_EBX);
    if (puVar7 == (undefined4 *)0x0) {
      **(int **)(&DAT_00addeac + unaff_EBX) = (int)puVar1;
      *(undefined4 *)(&DAT_00ced7c8 + unaff_EBX) = 0;
    }
    else {
      puVar2 = (undefined4 *)puVar7[2];
      iVar5 = _V_stricmp((char *)*puVar7,s2);
      if (iVar5 < 1) {
        while ((puVar6 = puVar2, puVar6 != (undefined4 *)0x0 &&
               (iVar5 = _V_stricmp((char *)*puVar6,s2), iVar5 < 1))) {
          puVar2 = (undefined4 *)puVar6[2];
          puVar7 = puVar6;
        }
        *(undefined4 **)(&DAT_00ced7c8 + unaff_EBX) = puVar6;
        puVar7[2] = puVar1;
      }
      else {
        *(undefined4 *)(&DAT_00ced7c8 + unaff_EBX) = **(undefined4 **)(&DAT_00addeac + unaff_EBX);
        **(undefined4 **)(&DAT_00addeac + unaff_EBX) = puVar1;
      }
    }
    SendTable::SendTable((SendTable *)((int)&PTR_SetText_00c6279c + unaff_EBX));
    ___cxa_atexit(unaff_EBX + 0x8cf47c,0,*(undefined4 *)(&DAT_00adde0c + unaff_EBX));
    iVar5 = ServerClassInit<DT_PropRocketTripwire::ignored>((ignored *)0x0);
    *(int *)((int)&PTR_SetTextInset_00c627b0 + unaff_EBX) = iVar5;
  }
  return;
}


/* CPropRocketTripwire::GetDataDescMap at 006f3df0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPropRocketTripwire * this) */

datamap_t * __thiscall CPropRocketTripwire::GetDataDescMap(CPropRocketTripwire *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5dc908);
}


/* CPropRocketTripwire::GetBaseMap at 006f3e00 */

datamap_t * CPropRocketTripwire::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4b7d94);
}


/* __tcf_0 at 0099d220 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41e54e)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41e542) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41e54e));
  }
  *(undefined4 *)(unaff_EBX + 0x41e54e) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41e54a)) {
    if (*(int *)(unaff_EBX + 0x41e542) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20e96a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20e96a),*(int *)(unaff_EBX + 0x41e542));
      *(undefined4 *)(unaff_EBX + 0x41e542) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e546) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41e542);
  *(int *)(unaff_EBX + 0x41e552) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41e54a)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20e96a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20e96a),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41e542) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41e546) = 0;
  }
  return;
}


/* ServerClassInit<DT_PropRocketTripwire::ignored> at 000cd7d0 */

int ServerClassInit<DT_PropRocketTripwire::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (speechListSounds[0].name[unaff_EBX + 0xb6] == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcedd7a);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)(speechListSounds[0].name + unaff_EBX + 0xbe),
                  (char *)(unaff_EBX + 0x8e5246),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)(speechListSounds[0].name + unaff_EBX + 0x112),
                        (char *)(unaff_EBX + 0x8e75b6),0,
                        (SendTable *)**(undefined4 **)(&DAT_00ade442 + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00ade44e + unaff_EBX),0x80);
      SendPropInt((SendProp_conflict *)(speechListSounds[0].name + unaff_EBX + 0x166),
                  (char *)(unaff_EBX + 0x927248),0x568,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropInt((SendProp_conflict *)(speechListSounds[0].sceneName + unaff_EBX + -0x46),
                  (char *)(vgui::ScrollBar::GetKBMap + unaff_EBX + 1),0x56c,4,-1,0,
                  (SendVarProxyFn_conflict)0x0,0x80);
      SendPropBool((SendProp_conflict *)(speechListSounds[0].sceneName + unaff_EBX + 0xe),
                   (char *)(unaff_EBX + 0x945c18),0x570,1);
      SendPropEHandle((SendProp_conflict *)(speechListSounds[0].sceneName + unaff_EBX + 0x62),
                      (char *)(unaff_EBX + 0x945c25),0x574,4,-1,
                      *(SendVarProxyFn_conflict *)(&DAT_00ade43a + unaff_EBX));
      ___cxa_guard_release(unaff_EBX + 0xcedd7a);
      ___cxa_atexit(unaff_EBX + 0x8cf9a2,0,*(undefined4 *)(&DAT_00ade3b2 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)((int)&PTR_SetSmallCaption_00c62d40 + unaff_EBX + 2),
             (SendProp_conflict *)(speechListSounds[0].name + unaff_EBX + 0x112),5,
             *(char **)(&DAT_00b71a5e + unaff_EBX));
  return 1;
}


/* CPropRocketTripwire::GetServerClass at 006f3e10 */

/* DWARF original prototype: ServerClass * GetServerClass(CPropRocketTripwire * this) */

ServerClass * __thiscall CPropRocketTripwire::GetServerClass(CPropRocketTripwire *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x6c772c);
}


/* CPropRocketTripwire::YouForgotToImplementOrDeclareServerClass at 006f3e20 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CPropRocketTripwire *
   this) */

int __thiscall
CPropRocketTripwire::YouForgotToImplementOrDeclareServerClass(CPropRocketTripwire *this)

{
  return 0;
}


/* __tcf_2 at 0099d200 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x393318),in_stack_00000008);
  return;
}


/* __tcf_1 at 0099d180 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41e577))(unaff_EBX + 0x41e577);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41e523))(unaff_EBX + 0x41e523);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41e4cf))(unaff_EBX + 0x41e4cf);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41e47b))(unaff_EBX + 0x41e47b);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41e427))(unaff_EBX + 0x41e427);
                    /* WARNING: Could not recover jumptable at 0x0099d1f3. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41e3d3))();
  return;
}


/* CPropRocketTripwire::CPropRocketTripwire at 006f4200 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CPropRocketTripwire(CPropRocketTripwire * this) */

void __thiscall CPropRocketTripwire::CPropRocketTripwire(CPropRocketTripwire *this)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::CBaseAnimating(&this->super_CBaseAnimating);
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x54ac74);
  this->m_flRocketSpeed = 400.0;
  this->m_flRocketLifetime = 15.0;
  this->m_flTimeLastFired = 0.0;
  (this->m_OnTripped).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnTripped).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_OnTripped).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_OnRocketExplode).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnRocketExplode).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->m_OnRocketExplode).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_hPlacementHelper).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hCurRocket).super_CBaseHandle.m_Index = 0xffffffff;
  this->m_bHasRocketOut = false;
  this->m_flRocketTimeOfDeath = 0.0;
  (this->m_bDrawBeams).m_Value = true;
  if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false)
  {
    this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if (this_00 != (CBaseEdict *)0x0) {
      this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
      pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
      pIVar2->m_iChangeInfoSerialNumber = 0;
    }
  }
  else {
    puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  (this->m_hRocket).m_Value.super_CBaseHandle.m_Index = 0xffffffff;
  return;
}


/* CPropRocketTripwire::CPropRocketTripwire at 006f4320 */

/* DWARF original prototype: void CPropRocketTripwire(CPropRocketTripwire * this,
   CPropRocketTripwire * this) */

void __thiscall
CPropRocketTripwire::CPropRocketTripwire(CPropRocketTripwire *this,CPropRocketTripwire *this_1)

{
  CPropRocketTripwire(this);
  return;
}


/* CEntityFactory<CPropRocketTripwire>::Create at 006f5310 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CPropRocketTripwire> * this,
   char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CPropRocketTripwire>::Create
          (CEntityFactory<CPropRocketTripwire> *this,char *pClassName)

{
  CPropRocketTripwire *this_00;
  
                    /* Unresolved local var: CPropRocketTripwire * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x578);
  CPropRocketTripwire::CPropRocketTripwire(this_00);
  (*(this_00->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1d])(this_00,pClassName);
  return &(this_00->super_CBaseAnimating).super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CPropRocketTripwire::~CPropRocketTripwire at 006f3f80 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropRocketTripwire(CPropRocketTripwire * this, int __in_chrg) */

void __thiscall CPropRocketTripwire::~CPropRocketTripwire(CPropRocketTripwire *this,int __in_chrg)

{
  uint uVar1;
  int iVar2;
  CBaseEntity *oldObj;
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x54aefb);
  uVar1 = (this->m_hPlacementHelper).super_CBaseHandle.m_Index;
  if ((uVar1 == 0xffffffff) ||
     (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4b7c17),
     *(uint *)(iVar2 + 8) != uVar1 >> 0x10)) {
    oldObj = (CBaseEntity *)0x0;
  }
  else {
    oldObj = *(CBaseEntity **)(iVar2 + 4);
  }
  UTIL_Remove(oldObj);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnRocketExplode).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnTripped).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
  return;
}


/* CPropRocketTripwire::~CPropRocketTripwire at 006f4040 */

/* DWARF original prototype: void ~CPropRocketTripwire(CPropRocketTripwire * this, int __in_chrg,
   CPropRocketTripwire * this) */

void __thiscall
CPropRocketTripwire::~CPropRocketTripwire
          (CPropRocketTripwire *this,int __in_chrg,CPropRocketTripwire *this_1)

{
  ~CPropRocketTripwire(this,__in_chrg);
  return;
}


/* CPropRocketTripwire::~CPropRocketTripwire at 006f4140 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropRocketTripwire(CPropRocketTripwire * this, int __in_chrg) */

void __thiscall CPropRocketTripwire::~CPropRocketTripwire(CPropRocketTripwire *this,int __in_chrg)

{
  uint uVar1;
  int iVar2;
  CBaseEntity *oldObj;
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x54ad3b);
  uVar1 = (this->m_hPlacementHelper).super_CBaseHandle.m_Index;
  if ((uVar1 == 0xffffffff) ||
     (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4b7a57),
     *(uint *)(iVar2 + 8) != uVar1 >> 0x10)) {
    oldObj = (CBaseEntity *)0x0;
  }
  else {
    oldObj = *(CBaseEntity **)(iVar2 + 4);
  }
  UTIL_Remove(oldObj);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnRocketExplode).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnTripped).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CPropRocketTripwire::Precache at 006f4330 */

/* DWARF original prototype: void Precache(CPropRocketTripwire * this) */

void __thiscall CPropRocketTripwire::Precache(CPropRocketTripwire *this)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  int iVar2;
  IChangeInfoAccessor *pIVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x31f0cc));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x316149));
  iVar2 = CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x314a44));
  if (iVar2 != (this->m_nBeamEndpointTexture).m_Value) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
      ;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar3->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_nBeamEndpointTexture).m_Value = iVar2;
  }
  UTIL_PrecacheOther((char *)(unaff_EBX + 0x31612b),(char *)0x0);
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x31f0ee));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x31f108));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x31f128));
  CBaseAnimating::Precache(&this->super_CBaseAnimating);
  return;
}


/* CPropRocketTripwire::Spawn at 006f4a30 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CPropRocketTripwire * this) */

void __thiscall CPropRocketTripwire::Spawn(CPropRocketTripwire *this)

{
  undefined1 *puVar1;
  CBaseEdict *pCVar2;
  uint uVar3;
  BASEPTR func;
  int iVar4;
  IChangeInfoAccessor *pIVar5;
  CBaseEntity *pCVar6;
  uint *puVar7;
  int *piVar8;
  int *piVar9;
  int unaff_EBX;
  undefined1 local_40 [12];
  QAngle local_34;
  Vector local_28 [2];
  
                    /* Unresolved local var: Vector vecMuzzle@[???]
                       Unresolved local var: QAngle angMuzzleDir@[???]
                       Unresolved local var: Vector vecAimDir@[???] */
  ___i686_get_pc_thunk_bx();
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1a])(this);
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1b])(this,unaff_EBX + 0x31e9d2);
  iVar4 = CBaseAnimating::LookupAttachment
                    (&this->super_CBaseAnimating,(char *)(unaff_EBX + 0x31ea4e));
  if (iVar4 != (this->m_iMuzzleAttachment).m_Value) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      pCVar2 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar5 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar5->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_iMuzzleAttachment).m_Value = iVar4;
  }
  CBaseAnimating::GetAttachment
            (&this->super_CBaseAnimating,(this->m_iMuzzleAttachment).m_Value,local_28,&local_34);
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0xe9])(local_40,this);
  pCVar6 = CreateEntityByName((char *)(unaff_EBX + 0x319da8),-1,true);
  if (pCVar6 == (CBaseEntity *)0x0) {
    (this->m_hPlacementHelper).super_CBaseHandle.m_Index = 0xffffffff;
  }
  else {
    puVar7 = (uint *)(*(pCVar6->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                       _vptr_IHandleEntity[3])(pCVar6);
    uVar3 = *puVar7;
    (this->m_hPlacementHelper).super_CBaseHandle.m_Index = uVar3;
    piVar9 = *(int **)(&DAT_004b7166 + unaff_EBX);
    if (((uVar3 != 0xffffffff) &&
        (iVar4 = (uVar3 & 0xffff) * 0x10 + *piVar9, *(uint *)(iVar4 + 8) == uVar3 >> 0x10)) &&
       (piVar8 = *(int **)(iVar4 + 4), piVar8 != (int *)0x0)) {
      (**(code **)(*piVar8 + 0x80))(piVar8,unaff_EBX + 0x2e2356,unaff_EBX + 0x31c7ea);
      uVar3 = (this->m_hPlacementHelper).super_CBaseHandle.m_Index;
      if ((uVar3 == 0xffffffff) ||
         (iVar4 = (uVar3 & 0xffff) * 0x10 + *piVar9, *(uint *)(iVar4 + 8) != uVar3 >> 0x10)) {
        piVar8 = (int *)0x0;
      }
      else {
        piVar8 = *(int **)(iVar4 + 4);
      }
      (**(code **)(*piVar8 + 0x80))(piVar8,unaff_EBX + 0x31c7ee,unaff_EBX + 0x2beffe);
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
      uVar3 = (this->m_hPlacementHelper).super_CBaseHandle.m_Index;
      if ((uVar3 == 0xffffffff) ||
         (iVar4 = (uVar3 & 0xffff) * 0x10 + *piVar9, *(uint *)(iVar4 + 8) != uVar3 >> 0x10)) {
        pCVar6 = (CBaseEntity *)0x0;
      }
      else {
        pCVar6 = *(CBaseEntity **)(iVar4 + 4);
      }
      DispatchSpawn(pCVar6,true);
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
      uVar3 = (this->m_hPlacementHelper).super_CBaseHandle.m_Index;
      if ((uVar3 == 0xffffffff) ||
         (iVar4 = (uVar3 & 0xffff) * 0x10 + *piVar9, *(uint *)(iVar4 + 8) != uVar3 >> 0x10)) {
        piVar9 = (int *)0x0;
      }
      else {
        piVar9 = *(int **)(iVar4 + 4);
      }
      (**(code **)(*piVar9 + 0x94))(piVar9);
    }
  }
  if (this->m_bDisabled == false) {
    this->m_State = ACTIVE;
    if ((this->m_bDrawBeams).m_Value == false) {
      if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
          false) {
        pCVar2 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if (pCVar2 != (CBaseEdict *)0x0) {
          pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
          pIVar5 = CBaseEdict::GetChangeAccessor(pCVar2);
          pIVar5->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->m_bDrawBeams).m_Value = true;
    }
    this->m_bDisabled = false;
  }
  else {
    this->m_State = DISABLED;
    if ((this->m_bDrawBeams).m_Value != false) {
      if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
          false) {
        pCVar2 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                  super_CBaseEdict;
        if (pCVar2 != (CBaseEdict *)0x0) {
          pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
          pIVar5 = CBaseEdict::GetChangeAccessor(pCVar2);
          pIVar5->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->m_bDrawBeams).m_Value = false;
    }
    this->m_bDisabled = true;
  }
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0x742);
  CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             (float)((double)*(float *)(**(int **)(&DAT_004b7162 + unaff_EBX) + 0xc) +
                    *(double *)(unaff_EBX + 0x336022)),(char *)0x0);
  return;
}


/* CPropRocketTripwire::EyePosition at 006f4090 */

/* DWARF original prototype: Vector EyePosition(CPropRocketTripwire * this) */

Vector * __thiscall
CPropRocketTripwire::EyePosition(Vector *__return_storage_ptr__,CPropRocketTripwire *this)

{
                    /* Unresolved local var: Vector vMuzzlePos@[DW_OP_breg6(ESI): 0]
                       Unresolved local var: Vector vMuzzlePos@[???] */
  CBaseAnimating::GetAttachment
            (&this->super_CBaseAnimating,(this->m_iMuzzleAttachment).m_Value,__return_storage_ptr__,
             (Vector *)0x0,(Vector *)0x0,(Vector *)0x0);
  return __return_storage_ptr__;
}


/* CPropRocketTripwire::EyeDirection at 006f4050 */

/* DWARF original prototype: Vector EyeDirection(CPropRocketTripwire * this) */

Vector * __thiscall
CPropRocketTripwire::EyeDirection(Vector *__return_storage_ptr__,CPropRocketTripwire *this)

{
  Vector local_18;
  
                    /* Unresolved local var: Vector vMuzzleDir@[DW_OP_breg6(ESI): 0]
                       Unresolved local var: Vector vMuzzlePos@[???]
                       Unresolved local var: Vector vMuzzleDir@[???] */
  CBaseAnimating::GetAttachment
            (&this->super_CBaseAnimating,(this->m_iMuzzleAttachment).m_Value,&local_18,
             __return_storage_ptr__,(Vector *)0x0,(Vector *)0x0);
  return __return_storage_ptr__;
}


/* CPropRocketTripwire::UpdateOnRemove at 006f40d0 */

/* DWARF original prototype: void UpdateOnRemove(CPropRocketTripwire * this) */

void __thiscall CPropRocketTripwire::UpdateOnRemove(CPropRocketTripwire *this)

{
  uint uVar1;
  CBaseEntity *oldObj;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->m_hPlacementHelper).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4b7ac3),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (oldObj = *(CBaseEntity **)(iVar2 + 4), oldObj != (CBaseEntity *)0x0)) {
    UTIL_Remove(oldObj);
  }
  CBaseEntity::UpdateOnRemove((CBaseEntity *)this);
  return;
}


/* CPropRocketTripwire::WaitingThink at 006f5180 */

/* DWARF original prototype: void WaitingThink(CPropRocketTripwire * this) */

void __thiscall CPropRocketTripwire::WaitingThink(CPropRocketTripwire *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             (float)((double)*(float *)(**(int **)(unaff_EBX + 0x4b6a0f) + 0xc) +
                    *(double *)(unaff_EBX + 0x3358cf)),(char *)0x0);
  if (this->m_State == ACTIVE) {
    ActiveThink(this);
    return;
  }
  if (this->m_State != FIRING) {
    return;
  }
  PreFiringThink(this);
  return;
}


/* CPropRocketTripwire::ActiveThink at 006f4e10 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ActiveThink(CPropRocketTripwire * this) */

void __thiscall CPropRocketTripwire::ActiveThink(CPropRocketTripwire *this)

{
  BASEPTR func;
  CBaseEntity *pCVar1;
  CPortal_Base2D *pPortal;
  int iVar2;
  CBasePlayer *pCVar3;
  int unaff_EBX;
  float fVar4;
  float fVar5;
  float fVar6;
  trace_t local_130;
  Ray_t local_dc;
  CTraceFilterSimple local_80;
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
  float local_34;
  float local_30;
  float local_2c;
  ITraceFilter local_28;
  CTraceFilterSimple *local_24;
  float flRequiredParameter;
  
                    /* Unresolved local var: Ray_t rayDmg@[???]
                       Unresolved local var: Vector vForward@[???]
                       Unresolved local var: Vector vEndPoint@[???]
                       Unresolved local var: trace_t traceDmg@[???]
                       Unresolved local var: CTraceFilterSimple subfilter@[???]
                       Unresolved local var: CTraceFilterTranslateClones filter@[???]
                       Unresolved local var: CPortal_Base2D * pFirstPortal@[???] */
  ___i686_get_pc_thunk_bx();
  local_dc.m_pWorldAxisTransform = (matrix3x4_t *)0x0;
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0xe9])(&local_34,this);
                    /* Unresolved local var: Vector res@[???] */
  local_44 = *(float *)(unaff_EBX + 0x3379a7);
  local_4c = local_34 * local_44;
  local_48 = local_30 * local_44;
  local_44 = local_2c * local_44;
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x88])(&local_40,this);
                    /* Unresolved local var: Vector res@[???] */
  fVar5 = local_40 + local_4c;
  fVar6 = local_3c + local_48;
  fVar4 = local_38 + local_44;
  local_58 = fVar5;
  local_54 = fVar6;
  local_50 = fVar4;
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x88])(&local_64,this);
  local_dc.m_Delta.super_Vector.x = fVar5 - local_64;
  local_dc.m_Delta.super_Vector.y = fVar6 - local_60;
  local_dc.m_Delta.super_Vector.z = fVar4 - local_5c;
  local_dc.m_IsSwept =
       local_dc.m_Delta.super_Vector.x * local_dc.m_Delta.super_Vector.x +
       local_dc.m_Delta.super_Vector.y * local_dc.m_Delta.super_Vector.y +
       local_dc.m_Delta.super_Vector.z * local_dc.m_Delta.super_Vector.z != 0.0;
  local_dc.m_Extents.super_Vector.z = 0.0;
  local_dc.m_Extents.super_Vector.y = 0.0;
  local_dc.m_Extents.super_Vector.x = 0.0;
  local_dc.m_pWorldAxisTransform = (matrix3x4_t *)0x0;
  local_dc.m_StartOffset.super_Vector.z = 0.0;
  local_dc.m_StartOffset.super_Vector.y = 0.0;
  local_dc.m_StartOffset.super_Vector.x = 0.0;
  local_dc.m_Start.super_Vector.x = local_64;
  local_dc.m_Start.super_Vector.y = local_60;
  local_dc.m_Start.super_Vector.z = local_5c;
  local_dc.m_IsRay = true;
  CTraceFilterSimple::CTraceFilterSimple(&local_80,(IHandleEntity *)this,0,(ShouldHitFunc_t)0x0);
  local_28._vptr_ITraceFilter = (_func_int_varargs **)(*(int *)(&DAT_004b6e7b + unaff_EBX) + 8);
  flRequiredParameter = 2.0;
  local_24 = &local_80;
  pPortal = UTIL_Portal_FirstAlongRay(&local_dc,&flRequiredParameter);
  UTIL_Portal_TraceRay_Bullets(pPortal,&local_dc,0x2006081,&local_28,&local_130,false);
                    /* Unresolved local var: Vector vVel@[???] */
  if ((local_130.m_pEnt != (CBaseEntity *)0x0) &&
     ((iVar2 = (*((local_130.m_pEnt)->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                 _vptr_IHandleEntity[0x55])(local_130.m_pEnt), pCVar1 = local_130.m_pEnt,
      (char)iVar2 != '\0' ||
      ((local_130.m_pEnt != (CBaseEntity *)0x0 &&
       ((pCVar3 = GetPlayerHoldingEntity(local_130.m_pEnt), pCVar3 != (CBasePlayer *)0x0 ||
        (((pCVar1->m_MoveType).m_Value == '\x06' &&
         ((*(pCVar1->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
            _vptr_IHandleEntity[0x92])(pCVar1,&local_70,0),
         fVar4 = local_70 * local_70 + local_6c * local_6c + local_68 * local_68,
         *(float *)(unaff_EBX + 0x3357f7) <= fVar4 && fVar4 != *(float *)(unaff_EBX + 0x3357f7))))))
       ))))) {
    if (this->m_State != DISABLED) {
      func.__delta = 0;
      func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x731);
      CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
      CBaseEntity::SetNextThink
                ((CBaseEntity *)this,
                 *(float *)(&DAT_003368bf + unaff_EBX) +
                 *(float *)(**(int **)(unaff_EBX + 0x4b6d7f) + 0xc),(char *)0x0);
      CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x31e5a3),0.0,(float *)0x0);
      this->m_State = FIRING;
    }
    COutputEvent::FireOutput(&this->m_OnTripped,(CBaseEntity *)this,local_130.m_pEnt,0.0);
  }
  return;
}


/* CPropRocketTripwire::PrepareToFireRocket at 006f3ed0 */

/* DWARF original prototype: void PrepareToFireRocket(CPropRocketTripwire * this) */

void __thiscall CPropRocketTripwire::PrepareToFireRocket(CPropRocketTripwire *this)

{
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_State != DISABLED) {
    func.__delta = 0;
    func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0x80c);
    CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
    CBaseEntity::SetNextThink
              ((CBaseEntity *)this,
               *(float *)(unaff_EBX + 0x3377fc) + *(float *)(**(int **)(unaff_EBX + 0x4b7cbc) + 0xc)
               ,(char *)0x0);
    CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x31f4e0),0.0,(float *)0x0);
    this->m_State = FIRING;
  }
  return;
}


/* CPropRocketTripwire::InputForceFire at 006f5220 */

/* DWARF original prototype: void InputForceFire(CPropRocketTripwire * this, inputdata_t *
   inputdata) */

void __thiscall
CPropRocketTripwire::InputForceFire(CPropRocketTripwire *this,inputdata_t *inputdata)

{
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_State != DISABLED) {
    func.__delta = 0;
    func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0xb44);
    CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
    CBaseEntity::SetNextThink
              ((CBaseEntity *)this,
               *(float *)(&DAT_003364ac + unaff_EBX) +
               *(float *)(**(int **)(unaff_EBX + 0x4b696c) + 0xc),(char *)0x0);
    CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x31e190),0.0,(float *)0x0);
    this->m_State = FIRING;
  }
  return;
}


/* CPropRocketTripwire::FireRocketThink at 006f46f0 */

/* DWARF original prototype: void FireRocketThink(CPropRocketTripwire * this) */

void __thiscall CPropRocketTripwire::FireRocketThink(CPropRocketTripwire *this)

{
  FireRocket(this);
  return;
}


/* CPropRocketTripwire::FireRocket at 006f4410 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void FireRocket(CPropRocketTripwire * this) */

void __thiscall CPropRocketTripwire::FireRocket(CPropRocketTripwire *this)

{
  undefined1 *puVar1;
  uint uVar2;
  CBaseEdict *this_00;
  _func_int_varargs *p_Var3;
  int *piVar4;
  BASEPTR func;
  int iVar5;
  CBaseEntity *pCVar6;
  ulong *puVar7;
  uint *puVar8;
  int iVar9;
  IChangeInfoAccessor *pIVar10;
  undefined4 *puVar11;
  int unaff_EBX;
  Vector local_60;
  Vector local_54;
  QAngle local_48;
  float local_3c;
  float local_38;
  float local_34;
  Vector local_30;
  uint local_24;
  undefined4 local_20 [4];
  
                    /* Unresolved local var: Vector vForward@[???]
                       Unresolved local var: QAngle spawnAngles@[???]
                       Unresolved local var: CRocket_Turret_Projectile * pRocket@[???] */
  ___i686_get_pc_thunk_bx();
  uVar2 = (this->m_hCurRocket).super_CBaseHandle.m_Index;
  if (((uVar2 != 0xffffffff) &&
      (iVar5 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4b7783),
      *(uint *)(iVar5 + 8) == uVar2 >> 0x10)) &&
     (pCVar6 = *(CBaseEntity **)(iVar5 + 4), pCVar6 != (CBaseEntity *)0x0)) {
    UTIL_RemoveImmediate(pCVar6);
  }
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0xe9])(&local_3c,this);
  local_30.x = local_3c;
  local_30.y = local_38;
  local_30.z = local_34;
  VectorAngles(&local_30,&local_48);
  (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x88])(&local_54,this);
  pCVar6 = CBaseEntity::Create((char *)(unaff_EBX + 0x31604e),&local_54,&local_48,
                               (CBaseEntity *)this);
  if (pCVar6 == (CBaseEntity *)0x0) {
    (this->m_hCurRocket).super_CBaseHandle.m_Index = 0xffffffff;
  }
  else {
    puVar7 = (ulong *)(*(pCVar6->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                        _vptr_IHandleEntity[3])(pCVar6);
    (this->m_hCurRocket).super_CBaseHandle.m_Index = *puVar7;
  }
  local_24 = 0xffffffff;
  if (pCVar6 == (CBaseEntity *)0x0) {
    local_24 = 0xffffffff;
  }
  else {
    puVar8 = (uint *)(*(pCVar6->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                       _vptr_IHandleEntity[3])(pCVar6);
    local_24 = *puVar8;
  }
  if ((local_24 == 0xffffffff) ||
     (iVar5 = (local_24 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4b7783),
     *(uint *)(iVar5 + 8) != local_24 >> 0x10)) {
    iVar5 = 0;
  }
  else {
    iVar5 = *(int *)(iVar5 + 4);
  }
  uVar2 = (this->m_hRocket).m_Value.super_CBaseHandle.m_Index;
  if ((uVar2 == 0xffffffff) ||
     (iVar9 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4b7783),
     *(uint *)(iVar9 + 8) != uVar2 >> 0x10)) {
    iVar9 = 0;
  }
  else {
    iVar9 = *(int *)(iVar9 + 4);
  }
  if (iVar9 != iVar5) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
      ;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar10 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar10->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_hRocket).m_Value.super_CBaseHandle.m_Index = local_24;
  }
  local_60.z = this->m_flRocketSpeed;
                    /* Unresolved local var: Vector res@[???] */
  local_60.x = local_60.z * local_30.x;
  local_60.y = local_60.z * local_30.y;
  local_60.z = local_60.z * local_30.z;
  CBaseEntity::SetAbsVelocity(pCVar6,&local_60);
  p_Var3 = (pCVar6->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
           _vptr_IHandleEntity[0x157];
  local_20[0] = 0xffffffff;
  puVar11 = (undefined4 *)
            (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.
              super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[3])(this);
  local_20[0] = *puVar11;
  (*p_Var3)(pCVar6,local_20);
  piVar4 = *(int **)(unaff_EBX + 0x4b777f);
  this->m_flTimeLastFired = *(float *)(*piVar4 + 0xc);
  this->m_bHasRocketOut = true;
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX + 0xd5f);
  CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             (float)((double)*(float *)(*piVar4 + 0xc) + *(double *)(&DAT_0033663f + unaff_EBX)),
             (char *)0x0);
  return;
}


/* CPropRocketTripwire::IsAlternativeTripwireTarget at 006f3e30 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: bool IsAlternativeTripwireTarget(CPropRocketTripwire * this,
   CBaseEntity * pEnt) */

bool __thiscall
CPropRocketTripwire::IsAlternativeTripwireTarget(CPropRocketTripwire *this,CBaseEntity *pEnt)

{
  CBasePlayer *pCVar1;
  bool bVar2;
  int unaff_EBX;
  float fVar3;
  float local_18;
  float local_14;
  float local_10;
  
  ___i686_get_pc_thunk_bx();
  if (pEnt == (CBaseEntity *)0x0) {
    return false;
  }
  pCVar1 = GetPlayerHoldingEntity(pEnt);
  bVar2 = true;
  if (pCVar1 == (CBasePlayer *)0x0) {
    if ((pEnt->m_MoveType).m_Value != '\x06') {
      return false;
    }
    (*(pEnt->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x92]
    )(pEnt,&local_18,0);
    fVar3 = local_18 * local_18 + local_14 * local_14 + local_10 * local_10;
    bVar2 = *(float *)(unaff_EBX + 0x3367d7) <= fVar3 && fVar3 != *(float *)(unaff_EBX + 0x3367d7);
  }
  return bVar2;
}


/* CPropRocketTripwire::PreFiringThink at 006f47e0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void PreFiringThink(CPropRocketTripwire * this) */

void __thiscall CPropRocketTripwire::PreFiringThink(CPropRocketTripwire *this)

{
  undefined1 *puVar1;
  uint uVar2;
  CRocket_Turret_Projectile *this_00;
  int *piVar3;
  CBaseEdict *pCVar4;
  int iVar5;
  IChangeInfoAccessor *pIVar6;
  int unaff_EBX;
  float fVar7;
  
                    /* Unresolved local var: CRocket_Turret_Projectile * pRocket@[???] */
  ___i686_get_pc_thunk_bx();
  uVar2 = (this->m_hCurRocket).super_CBaseHandle.m_Index;
  if (((uVar2 == 0xffffffff) ||
      (iVar5 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4b73b0),
      *(uint *)(iVar5 + 8) != uVar2 >> 0x10)) ||
     (this_00 = *(CRocket_Turret_Projectile **)(iVar5 + 4),
     this_00 == (CRocket_Turret_Projectile *)0x0)) {
    if (this->m_bHasRocketOut == false) {
      if ((*(float *)(unaff_EBX + 0x3372cc) + this->m_flRocketTimeOfDeath <
           *(float *)(**(int **)(unaff_EBX + 0x4b73ac) + 0xc)) && (this->m_bDisabled == false)) {
        this->m_State = ACTIVE;
        if ((this->m_bDrawBeams).m_Value == false) {
          if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
              false) {
            pCVar4 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                      super_CBaseEdict;
            if (pCVar4 != (CBaseEdict *)0x0) {
              pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x101;
              pIVar6 = CBaseEdict::GetChangeAccessor(pCVar4);
              pIVar6->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
            *(uint *)puVar1 = *(uint *)puVar1 | 1;
          }
          (this->m_bDrawBeams).m_Value = true;
        }
        CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x31aba6),0.0,(float *)0x0);
      }
    }
    else {
      this->m_flRocketTimeOfDeath = *(float *)(**(int **)(unaff_EBX + 0x4b73ac) + 0xc);
      this->m_bHasRocketOut = false;
      COutputEvent::FireOutput(&this->m_OnRocketExplode,(CBaseEntity *)this,(CBaseEntity *)this,0.0)
      ;
      if ((this->m_bDrawBeams).m_Value != false) {
        if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered ==
            false) {
          pCVar4 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->
                    super_CBaseEdict;
          if (pCVar4 != (CBaseEdict *)0x0) {
            pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x101;
            pIVar6 = CBaseEdict::GetChangeAccessor(pCVar4);
            pIVar6->m_iChangeInfoSerialNumber = 0;
          }
        }
        else {
          puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (this->m_bDrawBeams).m_Value = false;
      }
      CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x31ec3e),0.0,(float *)0x0);
    }
  }
  else {
    piVar3 = *(int **)(unaff_EBX + 0x4b73ac);
    fVar7 = *(float *)(*piVar3 + 0xc) - this->m_flTimeLastFired;
    if (this->m_flRocketLifetime <= fVar7 && fVar7 != this->m_flRocketLifetime) {
      CRocket_Turret_Projectile::Explode(this_00);
      this->m_flTimeLastFired = *(float *)(*piVar3 + 0xc);
    }
  }
  return;
}


/* CPropRocketTripwire::InputEnable at 006f49c0 */

/* DWARF original prototype: void InputEnable(CPropRocketTripwire * this, inputdata_t * inputdata)
    */

void __thiscall CPropRocketTripwire::InputEnable(CPropRocketTripwire *this,inputdata_t *inputdata)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  
  this->m_State = ACTIVE;
  if ((this->m_bDrawBeams).m_Value == false) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
      ;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar2->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bDrawBeams).m_Value = true;
  }
  this->m_bDisabled = false;
  return;
}


/* CPropRocketTripwire::InputDisable at 006f4da0 */

/* DWARF original prototype: void InputDisable(CPropRocketTripwire * this, inputdata_t * inputdata)
    */

void __thiscall CPropRocketTripwire::InputDisable(CPropRocketTripwire *this,inputdata_t *inputdata)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  
  this->m_State = DISABLED;
  if ((this->m_bDrawBeams).m_Value != false) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
      ;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar2->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bDrawBeams).m_Value = false;
  }
  this->m_bDisabled = true;
  return;
}


/* CPropRocketTripwire::EnableTripwire at 006f4770 */

/* DWARF original prototype: void EnableTripwire(CPropRocketTripwire * this) */

void __thiscall CPropRocketTripwire::EnableTripwire(CPropRocketTripwire *this)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  
  this->m_State = ACTIVE;
  if ((this->m_bDrawBeams).m_Value == false) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
      ;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar2->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bDrawBeams).m_Value = true;
  }
  this->m_bDisabled = false;
  return;
}


/* CPropRocketTripwire::DisableTripwire at 006f4700 */

/* DWARF original prototype: void DisableTripwire(CPropRocketTripwire * this) */

void __thiscall CPropRocketTripwire::DisableTripwire(CPropRocketTripwire *this)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  
  this->m_State = DISABLED;
  if ((this->m_bDrawBeams).m_Value != false) {
    if ((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
       ) {
      this_00 = &((this->super_CBaseAnimating).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
      ;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar2->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseAnimating).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bDrawBeams).m_Value = false;
  }
  this->m_bDisabled = true;
  return;
}


/* _GLOBAL__I__ZN19CPropRocketTripwire9m_DataMapE at 000cdff0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN19CPropRocketTripwire9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

