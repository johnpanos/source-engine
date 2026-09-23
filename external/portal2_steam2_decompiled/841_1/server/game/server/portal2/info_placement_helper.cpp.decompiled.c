/* DWARF-guided pseudocode for game/server/portal2/info_placement_helper.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000aabc0 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  SendTable *this;
  undefined4 *puVar1;
  IEntityFactoryDictionary *pIVar2;
  datamap_t *pdVar3;
  int iVar4;
  int iVar5;
  int unaff_EBX;
  undefined4 *puVar6;
  undefined4 *puVar7;
  longlong lVar8;
  ConVar *in_stack_ffffffc8;
  char *in_stack_ffffffcc;
  char *in_stack_ffffffd0;
  int in_stack_ffffffd4;
  char *s2;
  char *in_stack_ffffffd8;
  
  lVar8 = ___i686_get_pc_thunk_bx();
  if (lVar8 == 0xffff00000001) {
    *(undefined1 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x37a].m_pPrev + unaff_EBX) =
         0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x37a].m_pPrev + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x37a].m_pPrev + unaff_EBX + 2) = 0;
    *(undefined1 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x37a].m_pPrev + unaff_EBX + 3) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x37a].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x37b].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x37b].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x37b].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x37b].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x37c].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x37c].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x37c].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x37c].m_pNext + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x37d].m_pEntity + unaff_EBX)
         = 0x7f7fffff;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x37d].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x37d].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x37d].m_pNext + unaff_EBX) =
         &UNK_00ba1734 + unaff_EBX;
    ConVar::ConVar((ConVar *)
                   ((int)DataMapInit<CAI_Relationship>::dataDesc[0].flatOffset + unaff_EBX + -0x28),
                   (char *)(unaff_EBX + 0xa06170),(char *)(unaff_EBX + 0x9a422c),0,
                   (char *)(unaff_EBX + 0xa06108),in_stack_ffffffc8,in_stack_ffffffcc,
                   in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8);
    ___cxa_atexit(unaff_EBX + 0x991f4c,0,*(undefined4 *)(&DAT_00b9bf80 + unaff_EBX));
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x37e].m_pEntity + unaff_EBX)
         = &UNK_00c1f19c + unaff_EBX;
    pIVar2 = EntityFactoryDictionary();
    (**pIVar2->_vptr_IEntityFactoryDictionary)(pIVar2,unaff_EBX + 0xda13c4,unaff_EBX + 0xa0abdc);
    pdVar3 = DataMapInit<CInfoPlacementHelper>((CInfoPlacementHelper *)0x0);
    *(datamap_t **)((int)DataMapInit<CAI_Relationship>::dataDesc[1].flatOffset + unaff_EBX + -0x10)
         = pdVar3;
    iVar5 = unaff_EBX + 0xda13c8;
    s2 = (char *)(unaff_EBX + 0xa060a3);
    *(char **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x37e].m_SerialNumber + unaff_EBX)
         = s2;
    this = (SendTable *)
           ((int)DataMapInit<CAI_Relationship>::dataDesc[1].flatOffset + unaff_EBX + -0xc);
    *(SendTable **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x37e].m_pPrev + unaff_EBX) =
         this;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x37f].m_SerialNumber + unaff_EBX) = 0xffff
    ;
    puVar7 = (undefined4 *)**(int **)(&DAT_00b9c020 + unaff_EBX);
    if (puVar7 == (undefined4 *)0x0) {
      **(int **)(&DAT_00b9c020 + unaff_EBX) = iVar5;
      *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x37e].m_pNext + unaff_EBX)
           = 0;
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
        *(undefined4 **)(iVar5 + 8) = puVar6;
        puVar7[2] = iVar5;
      }
      else {
        *(undefined4 *)(iVar5 + 8) = **(undefined4 **)(&DAT_00b9c020 + unaff_EBX);
        **(int **)(&DAT_00b9c020 + unaff_EBX) = iVar5;
      }
    }
    SendTable::SendTable(this);
    ___cxa_atexit(unaff_EBX + 0x991f2c,0,*(undefined4 *)(&DAT_00b9bf80 + unaff_EBX));
    iVar5 = ServerClassInit<DT_InfoPlacementHelper::ignored>((ignored *)0x0);
    *(int *)((int)DataMapInit<CAI_Relationship>::dataDesc[1].flatOffset + unaff_EBX + 8) = iVar5;
    CAutoGameSystem::CAutoGameSystem
              ((CAutoGameSystem *)
               ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x380].m_pPrev + unaff_EBX),
               (char *)0x0);
    *(undefined **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x380].m_pPrev + unaff_EBX) =
         &UNK_00c1f1d4 + unaff_EBX;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x381].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x381].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x381].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x382].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x382].m_SerialNumber + unaff_EBX) = 0;
    ___cxa_atexit(&UNK_0099204c + unaff_EBX,0,*(undefined4 *)(&DAT_00b9bf80 + unaff_EBX));
  }
  return;
}


/* __tcf_2 at 00a3cb20 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38ba38),in_stack_00000008);
  return;
}


/* DataMapInit<CInfoPlacementHelper> at 000aaa90 */

datamap_t * DataMapInit<CInfoPlacementHelper>(CInfoPlacementHelper *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3e1].m_pEntity + unaff_EBX) ==
      '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xda19f4);
    if (iVar2 != 0) {
      *(undefined **)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3e2].m_pEntity + unaff_EBX) =
           &UNK_00a061d3 + unaff_EBX;
      *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3e2].m_pPrev + unaff_EBX)
           = 0;
      *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3e2].m_pNext + unaff_EBX)
           = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3e3].m_pEntity + unaff_EBX) = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3e3].m_SerialNumber + unaff_EBX) = 0;
      *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3e3].m_pPrev + unaff_EBX)
           = 0;
      *(undefined4 *)
       ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3e2].m_SerialNumber + unaff_EBX) = 0x14
      ;
      ___cxa_guard_release(unaff_EBX + 0xda19f4);
      ___cxa_atexit(unaff_EBX + 0x99209c,0,*(undefined4 *)(&DAT_00b9c0b0 + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_EndTouch_00cb9288 + unaff_EBX) =
       *(undefined4 *)(&DAT_00b9c1e8 + unaff_EBX);
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3e1].m_pPrev + unaff_EBX) ==
      '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xda19fc);
    if (iVar2 != 0) {
      uVar1 = **(undefined4 **)(&DAT_00b9c0d0 + unaff_EBX);
      *(undefined4 *)((int)&PTR_IsNPC_00cb9570 + unaff_EBX) = uVar1;
      *(undefined4 *)((int)&PTR_IsBaseCombatWeapon_00cb95b0 + unaff_EBX) = uVar1;
      ___cxa_guard_release(unaff_EBX + 0xda19fc);
    }
  }
  *(undefined4 *)((int)&PTR_StartTouch_00cb9280 + unaff_EBX) = 0xc;
  *(int *)((int)&PTR_Use_00cb927c + unaff_EBX) =
       (int)&PTR_NetworkStateChanged_m_iMaxHealth_00cb92dc + unaff_EBX;
  return (datamap_t *)((int)&PTR_Use_00cb927c + unaff_EBX);
}


/* CInfoPlacementHelper::GetDataDescMap at 00717f50 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CInfoPlacementHelper * this) */

datamap_t * __thiscall CInfoPlacementHelper::GetDataDescMap(CInfoPlacementHelper *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x64bdc8);
}


/* CInfoPlacementHelper::GetBaseMap at 00717f60 */

datamap_t * CInfoPlacementHelper::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x52ed24);
}


/* __tcf_0 at 00a3cb40 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x40f96e)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x40f962) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x40f96e));
  }
  *(undefined4 *)(unaff_EBX + 0x40f96e) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40f96a)) {
    if (*(int *)(unaff_EBX + 0x40f962) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20a00e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20a00e),*(int *)(unaff_EBX + 0x40f962));
      *(undefined4 *)(unaff_EBX + 0x40f962) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f966) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40f962);
  *(int *)(unaff_EBX + 0x40f972) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40f96a)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20a00e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20a00e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40f962) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40f966) = 0;
  }
  return;
}


/* ServerClassInit<DT_InfoPlacementHelper::ignored> at 000aa5b0 */

int ServerClassInit<DT_InfoPlacementHelper::ignored>(ignored *param_1)

{
  SendVarProxyFn_conflict p_Var1;
  int iVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3e3].m_pNext + unaff_EBX + 2)
      == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xda1a22);
    if (iVar2 != 0) {
      SendPropInt((SendProp_conflict *)
                  ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3e5].m_pNext + unaff_EBX + 2
                  ),(char *)(unaff_EBX + 0x9ab676),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3eb].m_pEntity +
                        unaff_EBX + 2),(char *)(unaff_EBX + 0x9ada26),0,
                        (SendTable *)**(undefined4 **)(&DAT_00b9c68e + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00b9c632 + unaff_EBX),0x80);
      SendPropExclude((SendProp_conflict *)
                      ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3f0].m_SerialNumber +
                      unaff_EBX + 2),(char *)(unaff_EBX + 0xa021c6),(char *)(unaff_EBX + 0x9ab782));
      SendPropExclude((SendProp_conflict *)
                      ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3f5].m_pPrev +
                      unaff_EBX + 2),(char *)(unaff_EBX + 0xa021c6),(char *)(unaff_EBX + 0x9f937a));
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3fa].m_pNext +
                     unaff_EBX + 2),(char *)(unaff_EBX + 0x9ab782),0x500350,0xc,-1,4,0.0,-121121.125
                     ,*(SendVarProxyFn_conflict *)(&DAT_00b9cda2 + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x400].m_pEntity +
                     unaff_EBX + 2),(char *)(unaff_EBX + 0x9f937a),0x30035c,0xc,-1,4,0.0,-121121.125
                     ,*(SendVarProxyFn_conflict *)(&DAT_00b9cee6 + unaff_EBX),0x80);
      p_Var1 = *(SendVarProxyFn_conflict *)(&DAT_00b9c89e + unaff_EBX);
      SendPropString((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x405].m_SerialNumber +
                     unaff_EBX + 2),(char *)(unaff_EBX + 0xa06656),0x3f0,4,0,p_Var1,0x80);
      SendPropString((SendProp_conflict *)
                     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x40a].m_pPrev +
                     unaff_EBX + 2),&UNK_00a06667 + unaff_EBX,0x3f4,4,0,p_Var1,0x80);
      p_Var1 = *(SendVarProxyFn_conflict *)(&DAT_00b9c68a + unaff_EBX);
      SendPropFloat((SendProp_conflict *)
                    ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x40f].m_pNext +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x9cfc26),0x3f8,4,0x20,0,0.0,-121121.125,
                    p_Var1,0x80);
      SendPropBool((SendProp_conflict *)
                   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x415].m_pEntity +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0xa0667c),0x3fc,1);
      SendPropBool((SendProp_conflict *)
                   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x41a].m_SerialNumber +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0xa06692),0x3fd,1);
      SendPropBool((SendProp_conflict *)
                   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x41f].m_pPrev +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0x9ad53e),0x3fe,1);
      SendPropFloat((SendProp_conflict *)
                    ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x424].m_pNext +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x9edf64),0x400,4,0x20,0,0.0,-121121.125,
                    p_Var1,0x80);
      SendPropBool((SendProp_conflict *)
                   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x42a].m_pEntity +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0xa066a4),0x404,1);
      ___cxa_guard_release(unaff_EBX + 0xda1a22);
      ___cxa_atexit(unaff_EBX + 0x992442,0,*(undefined4 *)(&DAT_00b9c596 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)((int)DataMapInit<CAI_Path>::dataDesc[1].flatOffset + unaff_EBX + 10),
             (SendProp_conflict *)
             ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3eb].m_pEntity + unaff_EBX + 2),
             0xd,*(char **)(&DAT_00c1f78a + unaff_EBX));
  return 1;
}


/* CInfoPlacementHelper::GetServerClass at 00717f70 */

/* DWARF original prototype: ServerClass * GetServerClass(CInfoPlacementHelper * this) */

ServerClass * __thiscall CInfoPlacementHelper::GetServerClass(CInfoPlacementHelper *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x734024);
}


/* CInfoPlacementHelper::YouForgotToImplementOrDeclareServerClass at 00717f80 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CInfoPlacementHelper *
   this) */

int __thiscall
CInfoPlacementHelper::YouForgotToImplementOrDeclareServerClass(CInfoPlacementHelper *this)

{
  return 0;
}


/* __tcf_3 at 00a3cb00 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x38bab4),in_stack_00000008);
  return;
}


/* __tcf_1 at 00a3ca00 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40fa37))(unaff_EBX + 0x40fa37);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f9e3))(unaff_EBX + 0x40f9e3);
  (*(code *)**(undefined4 **)(&DAT_0040f98f + unaff_EBX))(&DAT_0040f98f + unaff_EBX);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f93b))(unaff_EBX + 0x40f93b);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f8e7))(unaff_EBX + 0x40f8e7);
  (*(code *)**(undefined4 **)(CBaseEntity::SendOnKilledGameEvent + unaff_EBX + 3))
            (unaff_EBX + 0x40f893);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f83f))(unaff_EBX + 0x40f83f);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f7eb))(unaff_EBX + 0x40f7eb);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f797))(unaff_EBX + 0x40f797);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f743))(unaff_EBX + 0x40f743);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f6ef))(unaff_EBX + 0x40f6ef);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f69b))(unaff_EBX + 0x40f69b);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f647))(unaff_EBX + 0x40f647);
                    /* WARNING: Could not recover jumptable at 0x00a3cafb. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x40f5f3))();
  return;
}


/* CInfoPlacementManager::~CInfoPlacementManager at 00719cb0 */

/* DWARF original prototype: void ~CInfoPlacementManager(CInfoPlacementManager * this, int
   __in_chrg) */

void __thiscall
CInfoPlacementManager::~CInfoPlacementManager(CInfoPlacementManager *this,int __in_chrg)

{
  CHandle<CInfoPlacementHelper> *pCVar1;
  int unaff_EBX;
  CHandle<CInfoPlacementHelper> *in_stack_ffffffc8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(unaff_EBX + 0x5b00e4);
  (this->m_PlacementHelpers).m_Size = 0;
  if ((this->m_PlacementHelpers).m_Memory.m_nGrowSize < 0) {
    pCVar1 = (this->m_PlacementHelpers).m_Memory.m_pMemory;
  }
  else {
    pCVar1 = (this->m_PlacementHelpers).m_Memory.m_pMemory;
    if (pCVar1 != (CHandle<CInfoPlacementHelper> *)0x0) {
      in_stack_ffffffc8 = pCVar1;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52ce98) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x52ce98));
      pCVar1 = (CHandle<CInfoPlacementHelper> *)0x0;
      (this->m_PlacementHelpers).m_Memory.m_pMemory = (CHandle<CInfoPlacementHelper> *)0x0;
    }
    (this->m_PlacementHelpers).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PlacementHelpers).m_pElements = pCVar1;
  if (-1 < (this->m_PlacementHelpers).m_Memory.m_nGrowSize) {
    if (pCVar1 != (CHandle<CInfoPlacementHelper> *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52ce98) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x52ce98));
      (this->m_PlacementHelpers).m_Memory.m_pMemory = (CHandle<CInfoPlacementHelper> *)0x0;
      in_stack_ffffffc8 = pCVar1;
    }
    (this->m_PlacementHelpers).m_Memory.m_nAllocationCount = 0;
  }
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(&UNK_0052f3c4 + unaff_EBX);
  IGameSystem::~IGameSystem((IGameSystem *)this,(int)in_stack_ffffffc8);
  operator_delete(this);
  return;
}


/* CInfoPlacementManager::~CInfoPlacementManager at 00719dc0 */

/* DWARF original prototype: void ~CInfoPlacementManager(CInfoPlacementManager * this, int
   __in_chrg) */

void __thiscall
CInfoPlacementManager::~CInfoPlacementManager(CInfoPlacementManager *this,int __in_chrg)

{
  CHandle<CInfoPlacementHelper> *pCVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(unaff_EBX + 0x5affd4);
  (this->m_PlacementHelpers).m_Size = 0;
  if ((this->m_PlacementHelpers).m_Memory.m_nGrowSize < 0) {
    pCVar1 = (this->m_PlacementHelpers).m_Memory.m_pMemory;
  }
  else {
    pCVar1 = (this->m_PlacementHelpers).m_Memory.m_pMemory;
    if (pCVar1 != (CHandle<CInfoPlacementHelper> *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52cd88) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x52cd88),pCVar1);
      (this->m_PlacementHelpers).m_Memory.m_pMemory = (CHandle<CInfoPlacementHelper> *)0x0;
    }
    pCVar1 = (CHandle<CInfoPlacementHelper> *)0x0;
    (this->m_PlacementHelpers).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PlacementHelpers).m_pElements = pCVar1;
  if (-1 < (this->m_PlacementHelpers).m_Memory.m_nGrowSize) {
    if (pCVar1 != (CHandle<CInfoPlacementHelper> *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52cd88) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x52cd88),pCVar1);
      (this->m_PlacementHelpers).m_Memory.m_pMemory = (CHandle<CInfoPlacementHelper> *)0x0;
    }
    (this->m_PlacementHelpers).m_Memory.m_nAllocationCount = 0;
  }
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(&UNK_0052f2b4 + unaff_EBX);
  IGameSystem::~IGameSystem((IGameSystem *)this,__in_chrg);
  return;
}


/* __tcf_4 at 00a3cc20 */

void __tcf_4(void *param_1)

{
  int unaff_EBX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_bx();
  *(int *)(unaff_EBX + 0x40f38c) = unaff_EBX + 0x28d174;
  *(undefined4 *)(unaff_EBX + 0x40f3a4) = 0;
  if (*(int *)(unaff_EBX + 0x40f3a0) < 0) {
    *(undefined4 *)(unaff_EBX + 0x40f3a8) = *(undefined4 *)(unaff_EBX + 0x40f398);
  }
  else if (*(int *)(unaff_EBX + 0x40f398) == 0) {
    *(undefined4 *)(unaff_EBX + 0x40f39c) = 0;
    *(undefined4 *)(unaff_EBX + 0x40f3a8) = 0;
    *(undefined4 *)(unaff_EBX + 0x40f39c) = 0;
  }
  else {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x209f28) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x209f28),*(int *)(unaff_EBX + 0x40f398));
    *(undefined4 *)(unaff_EBX + 0x40f398) = 0;
    *(undefined4 *)(unaff_EBX + 0x40f39c) = 0;
    *(undefined4 *)(unaff_EBX + 0x40f3a8) = 0;
    if (-1 < *(int *)(unaff_EBX + 0x40f3a0)) {
      *(undefined4 *)(unaff_EBX + 0x40f39c) = 0;
    }
  }
  *(int *)(unaff_EBX + 0x40f38c) = unaff_EBX + 0x20c454;
  IGameSystem::~IGameSystem((IGameSystem *)(unaff_EBX + 0x40f38c),in_stack_00000008);
  return;
}


/* CInfoPlacementManager::LevelShutdownPostEntity at 00719c50 */

/* DWARF original prototype: void LevelShutdownPostEntity(CInfoPlacementManager * this) */

void __thiscall CInfoPlacementManager::LevelShutdownPostEntity(CInfoPlacementManager *this)

{
  CHandle<CInfoPlacementHelper> *pCVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->m_PlacementHelpers).m_Size = 0;
  if ((this->m_PlacementHelpers).m_Memory.m_nGrowSize < 0) {
    pCVar1 = (this->m_PlacementHelpers).m_Memory.m_pMemory;
  }
  else {
    pCVar1 = (this->m_PlacementHelpers).m_Memory.m_pMemory;
    if (pCVar1 != (CHandle<CInfoPlacementHelper> *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x52cefb) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x52cefb),pCVar1);
      (this->m_PlacementHelpers).m_Memory.m_pMemory = (CHandle<CInfoPlacementHelper> *)0x0;
    }
    pCVar1 = (CHandle<CInfoPlacementHelper> *)0x0;
    (this->m_PlacementHelpers).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PlacementHelpers).m_pElements = pCVar1;
  return;
}


/* UTIL_FindPlacementHelper at 00718660 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

CInfoPlacementHelper * UTIL_FindPlacementHelper(Vector *vecEndPoint,CBasePlayer *pPlayer)

{
  uint uVar1;
  CInfoPlacementHelper *this;
  int iVar2;
  char *pcVar3;
  int unaff_EBX;
  int iVar4;
  longdouble lVar5;
  longdouble lVar6;
  double dVar7;
  float fVar8;
  float local_8c;
  CInfoPlacementHelper *local_88;
  double local_84;
  double local_7c;
  float local_40;
  float local_3c;
  float local_38;
  float local_34;
  float local_30;
  float local_2c;
  Vector local_28 [2];
  
  ___i686_get_pc_thunk_bx();
  if (pPlayer != (CBasePlayer *)0x0) {
    (**(code **)((int)(pPlayer->super_CBaseCombatCharacter).super_CBaseFlex.
                      super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                      super_IServerEntity.super_IServerUnknown + 0x414))(&local_34,pPlayer);
    (**(code **)((int)(pPlayer->super_CBaseCombatCharacter).super_CBaseFlex.
                      super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                      super_IServerEntity.super_IServerUnknown + 0x220))();
    if (0 < *(int *)(unaff_EBX + 0x733967)) {
      local_88 = (CInfoPlacementHelper *)0x0;
      local_8c = *(float *)(unaff_EBX + 0x3bcdb3);
      iVar4 = 0;
      do {
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
        uVar1 = *(uint *)(*(int *)(unaff_EBX + 0x73395b) + iVar4 * 4);
        if (((((uVar1 != 0xffffffff) &&
              (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x52e4fb),
              *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
             (this = *(CInfoPlacementHelper **)(iVar2 + 4), this != (CInfoPlacementHelper *)0x0)) &&
            (((this->m_bDisabled).m_Value == false &&
             ((this->m_bDeferringToPortal).m_Value == false)))) &&
           (((this->m_bForcePlacement).m_Value != false ||
            (fVar8 = *(float *)(**(int **)(unaff_EBX + 0x52e4f3) + 0xc),
            (this->m_flDisableTime).m_Value <= fVar8 && fVar8 != (this->m_flDisableTime).m_Value))))
        {
          if (((this->super_CPointEntity).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
            CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
          }
                    /* Unresolved local var: Vector res@[???] */
          local_28[0].x = (this->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.x - local_40;
          local_28[0].y = (this->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.y - local_3c;
          local_28[0].z = (this->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.z - local_38;
          fVar8 = VectorNormalize(local_28);
          dVar7 = (double)(this->m_flRadius).m_Value;
          lVar5 = (longdouble)_atan2(dVar7,(double)fVar8);
          lVar6 = (longdouble)
                  _acosf(local_34 * local_28[0].x + local_30 * local_28[0].y +
                         local_2c * local_28[0].z);
          if ((((float)lVar6 <= (float)lVar5) && (fVar8 <= local_8c)) &&
             (local_8c = fVar8, local_88 = this,
             *(int *)(*(int *)(*(int *)(unaff_EBX + 0x52e867) + 0x1c) + 0x30) != 0)) {
            if (((this->super_CPointEntity).super_CBaseEntity.m_iEFlags & 0x800) == 0) {
              local_7c = (double)(this->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.z;
              local_84 = (double)(this->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.y;
            }
            else {
              CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
              local_7c = (double)(this->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.z;
              if (((this->super_CPointEntity).super_CBaseEntity.m_iEFlags & 0x800) == 0) {
                local_84 = (double)(this->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.y;
              }
              else {
                CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
                local_84 = (double)(this->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.y;
                if (((this->super_CPointEntity).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
                  CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
                }
              }
            }
            pcVar3 = (this->super_CPointEntity).super_CBaseEntity.m_iName.m_Value.pszValue;
            if (pcVar3 == (char *)0x0) {
              pcVar3 = (char *)(unaff_EBX + 0x3366b3);
            }
            _Msg(unaff_EBX + 0x39861b,pcVar3,
                 (double)(this->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.x,local_84,
                 local_7c,dVar7);
          }
        }
        iVar4 = iVar4 + 1;
      } while (iVar4 < *(int *)(unaff_EBX + 0x733967));
      return local_88;
    }
  }
  return (CInfoPlacementHelper *)0x0;
}


/* CInfoPlacementHelper::CInfoPlacementHelper at 00718be0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CInfoPlacementHelper(CInfoPlacementHelper * this) */

void __thiscall CInfoPlacementHelper::CInfoPlacementHelper(CInfoPlacementHelper *this)

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
  uint uVar10;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::CBaseEntity((CBaseEntity *)this,false);
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(&UNK_005b0e37 + unaff_EBX);
  (this->m_OnObjectPlaced).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnObjectPlaced).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->m_OnObjectPlaced).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_ObjectPlacedSize).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_ObjectPlacedSize).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->m_ObjectPlacedSize).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_bSnapToHelperAngles).m_Value = false;
  if ((this->super_CPointEntity).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
    pCVar5 = &((this->super_CPointEntity).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
      pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
      pIVar8 = CBaseEdict::GetChangeAccessor(pCVar5);
      piVar6 = *(int **)(unaff_EBX + 0x52df77);
      puVar7 = (ushort *)*piVar6;
      if (pIVar8->m_iChangeInfoSerialNumber == *puVar7) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        uVar9 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar4 = puVar7[uVar9 * 0x14 + 0x14];
        if (uVar4 == 0) {
LAB_0071902a:
          puVar7[(uint)uVar4 + uVar9 * 0x14 + 1] = 0x3fc;
          puVar7[uVar9 * 0x14 + 0x14] = uVar4 + 1;
        }
        else if (puVar7[uVar9 * 0x14 + 1] != 0x3fc) {
          uVar10 = 0;
          do {
            uVar2 = uVar10 + 1;
            uVar10 = uVar2 & 0xffff;
            if ((ushort)uVar2 == uVar4) {
              if (uVar4 != 0x13) goto LAB_0071902a;
              goto LAB_0071915f;
            }
          } while (puVar7[uVar9 * 0x14 + uVar10 + 1] != 0x3fc);
        }
      }
      else if (puVar7[0x7d1] == 100) {
LAB_0071915f:
        pIVar8->m_iChangeInfoSerialNumber = 0;
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar8->m_iChangeInfo = puVar7[0x7d1];
        *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
        pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
        iVar3 = *piVar6 + (uint)pIVar8->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar3 + 2) = 0x3fc;
        *(undefined2 *)(iVar3 + 0x28) = 1;
      }
    }
  }
  else {
    puVar1 = &(this->super_CPointEntity).super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  (this->m_bForcePlacement).m_Value = false;
  if ((this->super_CPointEntity).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
    pCVar5 = &((this->super_CPointEntity).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
      pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
      pIVar8 = CBaseEdict::GetChangeAccessor(pCVar5);
      piVar6 = *(int **)(unaff_EBX + 0x52df77);
      puVar7 = (ushort *)*piVar6;
      if (pIVar8->m_iChangeInfoSerialNumber == *puVar7) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        uVar9 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar4 = puVar7[uVar9 * 0x14 + 0x14];
        if (uVar4 == 0) {
LAB_007190aa:
          puVar7[(uint)uVar4 + uVar9 * 0x14 + 1] = 0x3fd;
          puVar7[uVar9 * 0x14 + 0x14] = uVar4 + 1;
        }
        else if (puVar7[uVar9 * 0x14 + 1] != 0x3fd) {
          uVar10 = 0;
          do {
            uVar2 = uVar10 + 1;
            uVar10 = uVar2 & 0xffff;
            if ((ushort)uVar2 == uVar4) {
              if (uVar4 != 0x13) goto LAB_007190aa;
              goto LAB_00719176;
            }
          } while (puVar7[uVar9 * 0x14 + uVar10 + 1] != 0x3fd);
        }
      }
      else if (puVar7[0x7d1] == 100) {
LAB_00719176:
        pIVar8->m_iChangeInfoSerialNumber = 0;
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar8->m_iChangeInfo = puVar7[0x7d1];
        *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
        pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
        iVar3 = *piVar6 + (uint)pIVar8->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar3 + 2) = 0x3fd;
        *(undefined2 *)(iVar3 + 0x28) = 1;
      }
    }
  }
  else {
    puVar1 = &(this->super_CPointEntity).super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  (this->m_flDisableTime).m_Value = 0.0;
  if ((this->super_CPointEntity).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
    pCVar5 = &((this->super_CPointEntity).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
      pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
      pIVar8 = CBaseEdict::GetChangeAccessor(pCVar5);
      piVar6 = *(int **)(unaff_EBX + 0x52df77);
      puVar7 = (ushort *)*piVar6;
      if (pIVar8->m_iChangeInfoSerialNumber == *puVar7) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        uVar9 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar4 = puVar7[uVar9 * 0x14 + 0x14];
        if (uVar4 == 0) {
LAB_00719126:
          puVar7[(uint)uVar4 + uVar9 * 0x14 + 1] = 0x400;
          puVar7[uVar9 * 0x14 + 0x14] = uVar4 + 1;
        }
        else if (puVar7[uVar9 * 0x14 + 1] != 0x400) {
          uVar10 = 0;
          do {
            uVar2 = uVar10 + 1;
            uVar10 = uVar2 & 0xffff;
            if ((ushort)uVar2 == uVar4) {
              if (uVar4 != 0x13) goto LAB_00719126;
              goto LAB_00719148;
            }
          } while (puVar7[uVar9 * 0x14 + uVar10 + 1] != 0x400);
        }
      }
      else if (puVar7[0x7d1] == 100) {
LAB_00719148:
        pIVar8->m_iChangeInfoSerialNumber = 0;
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar8->m_iChangeInfo = puVar7[0x7d1];
        *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
        pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
        iVar3 = *piVar6 + (uint)pIVar8->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar3 + 2) = 0x400;
        *(undefined2 *)(iVar3 + 0x28) = 1;
      }
    }
  }
  else {
    puVar1 = &(this->super_CPointEntity).super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  (this->m_bDeferringToPortal).m_Value = false;
  if ((this->super_CPointEntity).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false) {
    pCVar5 = &((this->super_CPointEntity).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
    if ((pCVar5 != (CBaseEdict *)0x0) && ((pCVar5->m_fStateFlags & 0x100U) == 0)) {
      pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 1;
      pIVar8 = CBaseEdict::GetChangeAccessor(pCVar5);
      piVar6 = *(int **)(unaff_EBX + 0x52df77);
      puVar7 = (ushort *)*piVar6;
      if (pIVar8->m_iChangeInfoSerialNumber == *puVar7) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        uVar9 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar4 = puVar7[uVar9 * 0x14 + 0x14];
        if (uVar4 == 0) {
LAB_00718f94:
          puVar7[(uint)uVar4 + uVar9 * 0x14 + 1] = 0x404;
          puVar7[uVar9 * 0x14 + 0x14] = uVar4 + 1;
        }
        else if (puVar7[uVar9 * 0x14 + 1] != 0x404) {
          uVar10 = 0;
          do {
            uVar2 = uVar10 + 1;
            uVar10 = uVar2 & 0xffff;
            if ((ushort)uVar2 == uVar4) {
              if (uVar4 != 0x13) goto LAB_00718f94;
              goto LAB_00718fb2;
            }
          } while (puVar7[uVar9 * 0x14 + uVar10 + 1] != 0x404);
        }
      }
      else {
        if (puVar7[0x7d1] != 100) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar8->m_iChangeInfo = puVar7[0x7d1];
          *(short *)(*piVar6 + 0xfa2) = *(short *)(*piVar6 + 0xfa2) + 1;
          pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar3 = *piVar6 + (uint)pIVar8->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar3 + 2) = 0x404;
          *(undefined2 *)(iVar3 + 0x28) = 1;
          return;
        }
LAB_00718fb2:
        pIVar8->m_iChangeInfoSerialNumber = 0;
        pCVar5->m_fStateFlags = pCVar5->m_fStateFlags | 0x100;
      }
    }
  }
  else {
    puVar1 = &(this->super_CPointEntity).super_CBaseEntity.m_Network.field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  return;
}


/* CInfoPlacementHelper::CInfoPlacementHelper at 007191d0 */

/* DWARF original prototype: void CInfoPlacementHelper(CInfoPlacementHelper * this,
   CInfoPlacementHelper * this) */

void __thiscall
CInfoPlacementHelper::CInfoPlacementHelper(CInfoPlacementHelper *this,CInfoPlacementHelper *this_1)

{
  CInfoPlacementHelper(this);
  return;
}


/* CEntityFactory<CInfoPlacementHelper>::Create at 00719f60 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CInfoPlacementHelper> *
   this, char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CInfoPlacementHelper>::Create
          (CEntityFactory<CInfoPlacementHelper> *this,char *pClassName)

{
  CInfoPlacementHelper *this_00;
  
                    /* Unresolved local var: CInfoPlacementHelper * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x408);
  CInfoPlacementHelper::CInfoPlacementHelper(this_00);
  (*(this_00->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1d])(this_00,pClassName);
  return &(this_00->super_CPointEntity).super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CInfoPlacementHelper::Spawn at 00718000 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CInfoPlacementHelper * this) */

void __thiscall CInfoPlacementHelper::Spawn(CInfoPlacementHelper *this)

{
  uint uVar1;
  uint *puVar2;
  int iVar3;
  undefined4 *puVar4;
  int iVar5;
  int iVar6;
  int unaff_EBX;
  int iVar7;
  int iVar8;
  int local_30;
  undefined4 local_24;
  uint local_20;
  
  ___i686_get_pc_thunk_bx();
  CPointEntity::Spawn(&this->super_CPointEntity);
  if (this == (CInfoPlacementHelper *)0x0) {
    local_20 = 0xffffffff;
  }
  else {
    puVar2 = (uint *)(*(this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.
                       super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[3])(this);
    local_20 = *puVar2;
  }
  local_30 = *(int *)(unaff_EBX + 0x733fca);
  if (0 < local_30) {
    iVar5 = **(int **)(unaff_EBX + 0x52eb5e);
    if (local_20 == 0xffffffff) {
      iVar8 = 0;
      do {
        uVar1 = *(uint *)(*(int *)(unaff_EBX + 0x733fbe) + iVar8 * 4);
        if (uVar1 == 0xffffffff) {
          return;
        }
        iVar6 = (uVar1 & 0xffff) * 0x10 + iVar5;
        if (*(uint *)(iVar6 + 8) != uVar1 >> 0x10) {
          return;
        }
        if (*(int *)(iVar6 + 4) == 0) {
          return;
        }
        iVar8 = iVar8 + 1;
      } while (iVar8 != local_30);
    }
    else {
      iVar6 = 0;
      iVar8 = (local_20 & 0xffff) * 0x10 + iVar5;
      uVar1 = *(uint *)(iVar8 + 8);
      while( true ) {
        iVar7 = 0;
        if (uVar1 == local_20 >> 0x10) {
          iVar7 = *(int *)(iVar8 + 4);
        }
        uVar1 = *(uint *)(*(int *)(unaff_EBX + 0x733fbe) + iVar6 * 4);
        if ((uVar1 == 0xffffffff) ||
           (iVar3 = (uVar1 & 0xffff) * 0x10 + iVar5, *(uint *)(iVar3 + 8) != uVar1 >> 0x10)) {
          if (iVar7 == 0) {
            return;
          }
        }
        else if (*(int *)(iVar3 + 4) == iVar7) {
          return;
        }
        iVar6 = iVar6 + 1;
        if (iVar6 == local_30) break;
        uVar1 = *(uint *)(iVar8 + 8);
      }
    }
  }
  if (this == (CInfoPlacementHelper *)0x0) {
    local_24 = 0xffffffff;
    iVar8 = local_30 + 1;
    iVar5 = *(int *)(unaff_EBX + 0x733fc2);
    if (iVar8 <= iVar5) goto LAB_00718143;
  }
  else {
    puVar4 = (undefined4 *)
             (*(this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown
               .super_IHandleEntity._vptr_IHandleEntity[3])(this);
    local_24 = *puVar4;
    local_30 = *(int *)(unaff_EBX + 0x733fca);
    iVar8 = local_30 + 1;
    iVar5 = *(int *)(unaff_EBX + 0x733fc2);
    if (iVar8 <= iVar5) goto LAB_00718143;
  }
  CUtlMemory<CHandle<CInfoPlacementHelper>,int>::Grow
            ((CUtlMemory<CHandle<CInfoPlacementHelper>,int> *)(unaff_EBX + 0x733fbe),iVar8 - iVar5);
LAB_00718143:
  iVar6 = *(int *)(unaff_EBX + 0x733fca) + 1;
  *(int *)(unaff_EBX + 0x733fca) = iVar6;
  iVar5 = *(int *)(unaff_EBX + 0x733fbe);
  *(int *)(unaff_EBX + 0x733fce) = iVar5;
  iVar6 = (iVar6 - local_30) + -1;
  if (iVar6 < 1) {
    local_30 = local_30 << 2;
  }
  else {
    local_30 = local_30 * 4;
    _V_memmove((void *)(iVar5 + iVar8 * 4),(void *)(iVar5 + local_30),iVar6 * 4);
  }
  puVar4 = (undefined4 *)(local_30 + *(int *)(unaff_EBX + 0x733fbe));
  if (puVar4 == (undefined4 *)0x0) {
    return;
  }
  *puVar4 = local_24;
  return;
}


/* CInfoPlacementHelper::OnRestore at 00718a10 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void OnRestore(CInfoPlacementHelper * this) */

void __thiscall CInfoPlacementHelper::OnRestore(CInfoPlacementHelper *this)

{
  uint uVar1;
  uint *puVar2;
  int iVar3;
  undefined4 *puVar4;
  int iVar5;
  int iVar6;
  int unaff_EBX;
  int iVar7;
  int iVar8;
  int local_30;
  undefined4 local_24;
  uint local_20;
  
  ___i686_get_pc_thunk_bx();
  if (this == (CInfoPlacementHelper *)0x0) {
    local_20 = 0xffffffff;
  }
  else {
    puVar2 = (uint *)(*(this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.
                       super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[3])(this);
    local_20 = *puVar2;
  }
  local_30 = *(int *)(unaff_EBX + 0x7335ba);
  if (0 < local_30) {
    iVar5 = **(int **)(unaff_EBX + 0x52e14e);
    if (local_20 == 0xffffffff) {
      iVar8 = 0;
      do {
        uVar1 = *(uint *)(*(int *)(unaff_EBX + 0x7335ae) + iVar8 * 4);
        if (((uVar1 == 0xffffffff) ||
            (iVar6 = (uVar1 & 0xffff) * 0x10 + iVar5, *(uint *)(iVar6 + 8) != uVar1 >> 0x10)) ||
           (*(int *)(iVar6 + 4) == 0)) goto LAB_00718ad7;
        iVar8 = iVar8 + 1;
      } while (iVar8 != local_30);
    }
    else {
      iVar6 = 0;
      iVar8 = (local_20 & 0xffff) * 0x10 + iVar5;
      uVar1 = *(uint *)(iVar8 + 8);
      while( true ) {
        iVar7 = 0;
        if (uVar1 == local_20 >> 0x10) {
          iVar7 = *(int *)(iVar8 + 4);
        }
        uVar1 = *(uint *)(*(int *)(unaff_EBX + 0x7335ae) + iVar6 * 4);
        if ((uVar1 == 0xffffffff) ||
           (iVar3 = (uVar1 & 0xffff) * 0x10 + iVar5, *(uint *)(iVar3 + 8) != uVar1 >> 0x10)) {
          if (iVar7 == 0) goto LAB_00718ad7;
        }
        else if (*(int *)(iVar3 + 4) == iVar7) goto LAB_00718ad7;
        iVar6 = iVar6 + 1;
        if (iVar6 == local_30) break;
        uVar1 = *(uint *)(iVar8 + 8);
      }
    }
  }
  if (this == (CInfoPlacementHelper *)0x0) {
    local_24 = 0xffffffff;
    iVar8 = local_30 + 1;
    iVar5 = *(int *)(unaff_EBX + 0x7335b2);
    if (iVar8 <= iVar5) goto LAB_00718b4e;
  }
  else {
    puVar4 = (undefined4 *)
             (*(this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown
               .super_IHandleEntity._vptr_IHandleEntity[3])(this);
    local_24 = *puVar4;
    local_30 = *(int *)(unaff_EBX + 0x7335ba);
    iVar8 = local_30 + 1;
    iVar5 = *(int *)(unaff_EBX + 0x7335b2);
    if (iVar8 <= iVar5) goto LAB_00718b4e;
  }
  CUtlMemory<CHandle<CInfoPlacementHelper>,int>::Grow
            ((CUtlMemory<CHandle<CInfoPlacementHelper>,int> *)(unaff_EBX + 0x7335ae),iVar8 - iVar5);
LAB_00718b4e:
  iVar6 = *(int *)(unaff_EBX + 0x7335ba) + 1;
  *(int *)(unaff_EBX + 0x7335ba) = iVar6;
  iVar5 = *(int *)(unaff_EBX + 0x7335ae);
  *(int *)(unaff_EBX + 0x7335be) = iVar5;
  iVar6 = (iVar6 - local_30) + -1;
  if (iVar6 < 1) {
    local_30 = local_30 << 2;
  }
  else {
    local_30 = local_30 * 4;
    _V_memmove((void *)(iVar5 + iVar8 * 4),(void *)(iVar5 + local_30),iVar6 * 4);
  }
  puVar4 = (undefined4 *)(local_30 + *(int *)(unaff_EBX + 0x7335ae));
  if (puVar4 != (undefined4 *)0x0) {
    *puVar4 = local_24;
  }
LAB_00718ad7:
  CBaseEntity::OnRestore((CBaseEntity *)this);
  return;
}


/* CInfoPlacementHelper::UpdateOnRemove at 007181e0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void UpdateOnRemove(CInfoPlacementHelper * this) */

void __thiscall CInfoPlacementHelper::UpdateOnRemove(CInfoPlacementHelper *this)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  uint *puVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int unaff_EBX;
  int iVar9;
  uint local_20;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::UpdateOnRemove((CBaseEntity *)this);
  if (this == (CInfoPlacementHelper *)0x0) {
    local_20 = 0xffffffff;
  }
  else {
    puVar4 = (uint *)(*(this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.
                       super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[3])(this);
    local_20 = *puVar4;
  }
  iVar6 = *(int *)(unaff_EBX + 0x733dea);
  if (0 < iVar6) {
    iVar1 = *(int *)(unaff_EBX + 0x733dde);
    iVar2 = **(int **)(unaff_EBX + 0x52e97e);
    if (local_20 != 0xffffffff) {
      iVar8 = 0;
      iVar7 = (local_20 & 0xffff) * 0x10 + iVar2;
      uVar3 = *(uint *)(iVar7 + 8);
      do {
        iVar9 = 0;
        if (uVar3 == local_20 >> 0x10) {
          iVar9 = *(int *)(iVar7 + 4);
        }
        uVar3 = *(uint *)(iVar1 + iVar8 * 4);
        if ((uVar3 == 0xffffffff) ||
           (iVar5 = (uVar3 & 0xffff) * 0x10 + iVar2, *(uint *)(iVar5 + 8) != uVar3 >> 0x10)) {
          if (iVar9 == 0) goto LAB_007182b7;
        }
        else if (*(int *)(iVar5 + 4) == iVar9) goto LAB_007182b7;
        iVar8 = iVar8 + 1;
        if (iVar8 == iVar6) {
          return;
        }
        uVar3 = *(uint *)(iVar7 + 8);
      } while( true );
    }
    iVar8 = 0;
    do {
      uVar3 = *(uint *)(iVar1 + iVar8 * 4);
      if (((uVar3 == 0xffffffff) ||
          (iVar7 = (uVar3 & 0xffff) * 0x10 + iVar2, *(uint *)(iVar7 + 8) != uVar3 >> 0x10)) ||
         (*(int *)(iVar7 + 4) == 0)) {
LAB_007182b7:
        iVar6 = (iVar6 - iVar8) + -1;
        if (0 < iVar6) {
          _V_memmove((void *)(iVar1 + iVar8 * 4),(void *)(iVar1 + 4 + iVar8 * 4),iVar6 * 4);
        }
        *(int *)(unaff_EBX + 0x733dea) = *(int *)(unaff_EBX + 0x733dea) + -1;
        return;
      }
      iVar8 = iVar8 + 1;
    } while (iVar8 != iVar6);
  }
  return;
}


/* CInfoPlacementHelper::ShouldUseHelperAngles at 00717ff0 */

/* DWARF original prototype: bool ShouldUseHelperAngles(CInfoPlacementHelper * this) */

bool __thiscall CInfoPlacementHelper::ShouldUseHelperAngles(CInfoPlacementHelper *this)

{
  return (this->m_bSnapToHelperAngles).m_Value;
}


/* CInfoPlacementHelper::InputEnable at 00718330 */

/* DWARF original prototype: void InputEnable(CInfoPlacementHelper * this, inputdata_t * inputdata)
    */

void __thiscall CInfoPlacementHelper::InputEnable(CInfoPlacementHelper *this,inputdata_t *inputdata)

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
  if ((this->m_bDisabled).m_Value != false) {
    if ((this->super_CPointEntity).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false)
    {
      this_00 = &((this->super_CPointEntity).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x52e824);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_007184a4:
            puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x3fe;
            puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != 0x3fe) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_007184a4;
                pIVar6->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar7 * 0x14 + local_60 + 1] != 0x3fe);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x52e824) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x52e824) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x52e824);
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar2 = *piVar5 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3fe;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CPointEntity).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bDisabled).m_Value = false;
  }
  return;
}


/* CInfoPlacementHelper::InputDisable at 007191e0 */

/* DWARF original prototype: void InputDisable(CInfoPlacementHelper * this, inputdata_t * inputdata)
    */

void __thiscall
CInfoPlacementHelper::InputDisable(CInfoPlacementHelper *this,inputdata_t *inputdata)

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
  if ((this->m_bDisabled).m_Value == false) {
    if ((this->super_CPointEntity).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false)
    {
      this_00 = &((this->super_CPointEntity).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar6 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x52d974);
        if (pIVar6->m_iChangeInfoSerialNumber == *puVar4) {
          uVar7 = (uint)pIVar6->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar7 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00719354:
            puVar4[(uint)uVar3 + uVar7 * 0x14 + 1] = 0x3fe;
            puVar4[uVar7 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar7 * 0x14 + 1] != 0x3fe) {
            local_60 = 0;
            do {
              uVar8 = local_60 + 1;
              local_60 = uVar8 & 0xffff;
              if ((ushort)uVar8 == uVar3) {
                if (uVar3 != 0x13) goto LAB_00719354;
                pIVar6->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar7 * 0x14 + local_60 + 1] != 0x3fe);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar6->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar6->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x52d974) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x52d974) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x52d974);
          pIVar6->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar2 = *piVar5 + (uint)pIVar6->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x3fe;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CPointEntity).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bDisabled).m_Value = true;
  }
  return;
}


/* CInfoPlacementHelper::DrawDebugGeometryOverlays at 007184e0 */

/* DWARF original prototype: void DrawDebugGeometryOverlays(CInfoPlacementHelper * this) */

void __thiscall CInfoPlacementHelper::DrawDebugGeometryOverlays(CInfoPlacementHelper *this)

{
  Vector *position;
  byte bVar1;
  float fVar2;
  int unaff_EBX;
  QAngle *angles;
  int local_20;
  
                    /* Unresolved local var: int g@[???] */
  ___i686_get_pc_thunk_bx();
  if ((((this->super_CPointEntity).super_CBaseEntity.m_iEFlags & 0x800) != 0) &&
     (CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this),
     ((this->super_CPointEntity).super_CBaseEntity.m_iEFlags & 0x800) != 0)) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
  angles = &(this->super_CPointEntity).super_CBaseEntity.m_angAbsRotation;
  position = &(this->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin;
  NDebugOverlay::Axis(position,angles,8.0,false,0.1);
  if ((((this->m_bDisabled).m_Value != false) || ((this->m_bDeferringToPortal).m_Value != false)) ||
     (((this->m_bForcePlacement).m_Value == false &&
      (fVar2 = *(float *)(**(int **)(unaff_EBX + 0x52e670) + 0xc),
      fVar2 < (this->m_flDisableTime).m_Value || fVar2 == (this->m_flDisableTime).m_Value)))) {
    local_20 = 0;
    fVar2 = (this->m_flRadius).m_Value;
    bVar1 = *(byte *)((int)&(this->super_CPointEntity).super_CBaseEntity.m_iEFlags + 1);
  }
  else {
    local_20 = 0xff;
    fVar2 = (this->m_flRadius).m_Value;
    bVar1 = *(byte *)((int)&(this->super_CPointEntity).super_CBaseEntity.m_iEFlags + 1);
  }
  if (((bVar1 & 8) != 0) &&
     (CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this),
     ((this->super_CPointEntity).super_CBaseEntity.m_iEFlags & 0x800) != 0)) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
  NDebugOverlay::Sphere(position,angles,fVar2,0xff,local_20,0,0,false,0.1);
  CBaseEntity::DrawDebugGeometryOverlays((CBaseEntity *)this);
  return;
}


/* CInfoPlacementHelper::IsEnabled at 00717f90 */

/* DWARF original prototype: bool IsEnabled(CInfoPlacementHelper * this) */

bool __thiscall CInfoPlacementHelper::IsEnabled(CInfoPlacementHelper *this)

{
  float fVar1;
  bool bVar2;
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  if (((this->m_bDisabled).m_Value == false) && ((this->m_bDeferringToPortal).m_Value == false)) {
    bVar2 = true;
    if ((this->m_bForcePlacement).m_Value == false) {
      fVar1 = *(float *)(**(int **)(extraout_ECX + 0x52ebcc) + 0xc);
      bVar2 = (this->m_flDisableTime).m_Value <= fVar1 && fVar1 != (this->m_flDisableTime).m_Value;
    }
    return bVar2;
  }
  return false;
}


/* CInfoPlacementHelper::NotifyPortalEvent at 007193b0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void NotifyPortalEvent(CInfoPlacementHelper * this, PortalEvent_t
   nEventType, CPortal_Base2D * pNotifier) */

void __thiscall
CInfoPlacementHelper::NotifyPortalEvent
          (CInfoPlacementHelper *this,PortalEvent_t nEventType,CPortal_Base2D *pNotifier)

{
  undefined1 *puVar1;
  int iVar2;
  ushort uVar3;
  CBaseEdict *pCVar4;
  ushort *puVar5;
  int *piVar6;
  ulong *puVar7;
  IChangeInfoAccessor *pIVar8;
  CInfoPlacementHelper *this_00;
  char *pcVar9;
  uint uVar10;
  uint uVar11;
  int unaff_EBX;
  float fVar12;
  float fVar13;
  float fVar14;
  uint local_84;
  uint local_80;
  CBaseHandle local_20 [4];
  
                    /* Unresolved local var: float flResetDelay@[???] */
  ___i686_get_pc_thunk_bx();
  if (nEventType == PORTALEVENT_LINKED) {
    return;
  }
  if (nEventType != PORTALEVENT_MOVED) goto joined_r0x00719614;
  pcVar9 = (this->m_strTargetProxy).m_Value.pszValue;
  if (pcVar9 == (char *)0x0) {
LAB_00719650:
    this_00 = this;
    if (((this->super_CPointEntity).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
  }
  else {
    this_00 = (CInfoPlacementHelper *)
              CGlobalEntityList::FindEntityByName
                        (*(CGlobalEntityList **)(unaff_EBX + 0x52d85d),(CBaseEntity *)0x0,pcVar9,
                         (CBaseEntity *)0x0,(CBaseEntity *)0x0,(CBaseEntity *)0x0,
                         (IEntityFindFilter *)0x0);
    if (this_00 == (CInfoPlacementHelper *)0x0) {
      pcVar9 = (this->m_strTargetProxy).m_Value.pszValue;
      if (pcVar9 == (char *)0x0) {
        pcVar9 = (char *)(unaff_EBX + 0x33595d);
      }
      _Warning(unaff_EBX + 0x3477c1,pcVar9);
      goto LAB_00719650;
    }
    if (((this_00->super_CPointEntity).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this_00);
    }
  }
  if (((pNotifier->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)pNotifier);
  }
                    /* Unresolved local var: Vector res@[???] */
  fVar12 = (pNotifier->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.x -
           (this_00->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.x;
  fVar13 = (pNotifier->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.y -
           (this_00->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.y;
  fVar14 = (pNotifier->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin.z -
           (this_00->super_CPointEntity).super_CBaseEntity.m_vecAbsOrigin.z;
  if (fVar12 * fVar12 + fVar13 * fVar13 + fVar14 * fVar14 < *(float *)(unaff_EBX + 0x3b3771)) {
    return;
  }
joined_r0x00719614:
  if (this == (CInfoPlacementHelper *)0x0) {
    local_20[0].m_Index = 0xffffffff;
  }
  else {
    local_20[0].m_Index = 0xffffffff;
    puVar7 = (ulong *)(*(this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.
                        super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[3])(this);
    local_20[0].m_Index = *puVar7;
  }
  CPortal_Base2D::RemovePortalEventListener(pNotifier,(EHANDLE *)local_20);
  if ((this->m_bDeferringToPortal).m_Value != false) {
    if ((this->super_CPointEntity).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false)
    {
      pCVar4 = &((this->super_CPointEntity).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar8 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x52d7a1);
        if (pIVar8->m_iChangeInfoSerialNumber == *puVar5) {
          uVar10 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar10 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_0071982a:
            puVar5[(uint)uVar3 + uVar10 * 0x14 + 1] = 0x404;
            puVar5[uVar10 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar10 * 0x14 + 1] != 0x404) {
            local_84 = 0;
            do {
              uVar11 = local_84 + 1;
              local_84 = uVar11 & 0xffff;
              if ((ushort)uVar11 == uVar3) {
                if (uVar3 != 0x13) goto LAB_0071982a;
                pIVar8->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar10 * 0x14 + local_84 + 1] != 0x404);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar8->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar8->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x52d7a1) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x52d7a1) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x52d7a1);
          pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar8->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x404;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CPointEntity).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bDeferringToPortal).m_Value = false;
  }
  fVar12 = 0.0;
  if (nEventType != PORTALEVENT_FIZZLE) {
    fVar12 = *(float *)(unaff_EBX + 0x3b3591);
  }
  fVar12 = fVar12 + *(float *)(**(int **)(unaff_EBX + 0x52d79d) + 0xc);
  if (fVar12 != (this->m_flDisableTime).m_Value) {
    if ((this->super_CPointEntity).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false)
    {
      pCVar4 = &((this->super_CPointEntity).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((pCVar4 != (CBaseEdict *)0x0) && ((pCVar4->m_fStateFlags & 0x100U) == 0)) {
        pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 1;
        pIVar8 = CBaseEdict::GetChangeAccessor(pCVar4);
        puVar5 = (ushort *)**(undefined4 **)(unaff_EBX + 0x52d7a1);
        if (pIVar8->m_iChangeInfoSerialNumber == *puVar5) {
          uVar10 = (uint)pIVar8->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar5[uVar10 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_00719798:
            puVar5[(uint)uVar3 + uVar10 * 0x14 + 1] = 0x400;
            puVar5[uVar10 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar5[uVar10 * 0x14 + 1] != 0x400) {
            local_80 = 0;
            do {
              uVar11 = local_80 + 1;
              local_80 = uVar11 & 0xffff;
              if ((ushort)uVar11 == uVar3) {
                if (uVar3 != 0x13) goto LAB_00719798;
                pIVar8->m_iChangeInfoSerialNumber = 0;
                pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar5[uVar10 * 0x14 + local_80 + 1] != 0x400);
          }
        }
        else if (puVar5[0x7d1] == 100) {
          pIVar8->m_iChangeInfoSerialNumber = 0;
          pCVar4->m_fStateFlags = pCVar4->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar8->m_iChangeInfo = puVar5[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x52d7a1) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x52d7a1) + 0xfa2) + 1;
          piVar6 = *(int **)(unaff_EBX + 0x52d7a1);
          pIVar8->m_iChangeInfoSerialNumber = *(ushort *)*piVar6;
          iVar2 = *piVar6 + (uint)pIVar8->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x400;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CPointEntity).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flDisableTime).m_Value = fVar12;
  }
  return;
}


/* CInfoPlacementHelper::BindToPortal at 007198a0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void BindToPortal(CInfoPlacementHelper * this, CPortal_Base2D *
   pPortal) */

void __thiscall
CInfoPlacementHelper::BindToPortal(CInfoPlacementHelper *this,CPortal_Base2D *pPortal)

{
  undefined1 *puVar1;
  int iVar2;
  ushort uVar3;
  CBaseEdict *this_00;
  ushort *puVar4;
  int *piVar5;
  ulong *puVar6;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  uint uVar9;
  int unaff_EBX;
  uint local_70;
  EHANDLE local_20 [4];
  
  ___i686_get_pc_thunk_bx();
  if ((pPortal != (CPortal_Base2D *)0x0) && ((this->m_bForcePlacement).m_Value == false)) {
    local_20[0].super_CBaseHandle.m_Index = (CBaseHandle)0xffffffff;
    puVar6 = (ulong *)(*(this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.
                        super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[3])(this);
    local_20[0].super_CBaseHandle.m_Index = (CBaseHandle)*puVar6;
    CPortal_Base2D::AddPortalEventListener(pPortal,local_20);
    if ((this->m_bDeferringToPortal).m_Value == false) {
      if ((this->super_CPointEntity).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false
         ) {
        this_00 = &((this->super_CPointEntity).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict
        ;
        if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
          this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
          pIVar7 = CBaseEdict::GetChangeAccessor(this_00);
          puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x52d2b1);
          if (pIVar7->m_iChangeInfoSerialNumber == *puVar4) {
            uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar3 = puVar4[uVar8 * 0x14 + 0x14];
            if (uVar3 == 0) {
LAB_00719a44:
              puVar4[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x404;
              puVar4[uVar8 * 0x14 + 0x14] = uVar3 + 1;
            }
            else if (puVar4[uVar8 * 0x14 + 1] != 0x404) {
              local_70 = 0;
              do {
                uVar9 = local_70 + 1;
                local_70 = uVar9 & 0xffff;
                if ((ushort)uVar9 == uVar3) {
                  if (uVar3 != 0x13) goto LAB_00719a44;
                  pIVar7->m_iChangeInfoSerialNumber = 0;
                  this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                  break;
                }
              } while (puVar4[uVar8 * 0x14 + local_70 + 1] != 0x404);
            }
          }
          else if (puVar4[0x7d1] == 100) {
            pIVar7->m_iChangeInfoSerialNumber = 0;
            this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar7->m_iChangeInfo = puVar4[0x7d1];
            *(short *)(**(int **)(unaff_EBX + 0x52d2b1) + 0xfa2) =
                 *(short *)(**(int **)(unaff_EBX + 0x52d2b1) + 0xfa2) + 1;
            piVar5 = *(int **)(unaff_EBX + 0x52d2b1);
            pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
            iVar2 = *piVar5 + (uint)pIVar7->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar2 + 2) = 0x404;
            *(undefined2 *)(iVar2 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this->super_CPointEntity).super_CBaseEntity.m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->m_bDeferringToPortal).m_Value = true;
    }
  }
  return;
}


/* _GLOBAL__I_sv_show_placement_help_in_preview at 000aae60 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_sv_show_placement_help_in_preview(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

