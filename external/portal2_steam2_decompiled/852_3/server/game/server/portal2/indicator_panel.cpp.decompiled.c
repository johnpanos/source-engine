/* DWARF-guided pseudocode for game/server/portal2/indicator_panel.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* DataMapInit<CLabIndicatorPanel> at 000bf320 */

datamap_t * DataMapInit<CLabIndicatorPanel>(CLabIndicatorPanel *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&g_PhysObjSaveRestoreOps[2].m_type + unaff_EBX + 3) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf7a97);
    if (iVar1 != 0) {
      *(int *)((int)&g_PhysObjSaveRestoreOps[3].m_type + unaff_EBX + 3) = unaff_EBX + 0x94f87e;
      *(undefined4 *)((int)&g_PhysObjSaveRestoreOps[4].m_type + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&g_PhysObjSaveRestoreOps[5].super_CDefSaveRestoreOps.super_ISaveRestoreOps.
              _vptr_ISaveRestoreOps + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&g_PhysObjSaveRestoreOps[5].m_type + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&g_PhysObjSaveRestoreOps[6].super_CDefSaveRestoreOps.super_ISaveRestoreOps.
              _vptr_ISaveRestoreOps + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&g_PhysObjSaveRestoreOps[6].m_type + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&g_PhysObjSaveRestoreOps[4].super_CDefSaveRestoreOps.super_ISaveRestoreOps.
              _vptr_ISaveRestoreOps + unaff_EBX + 3) = 0x12;
      ___cxa_guard_release(unaff_EBX + 0xcf7a97);
      ___cxa_atexit(unaff_EBX + 0x8da11f,0,*(undefined4 *)(&DAT_00aec85f + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_SetScriptOwnerEntity_00c0a2f8 + unaff_EBX + 3) =
       *(undefined4 *)(&DAT_00aec99b + unaff_EBX);
  *(undefined4 *)((int)&PTR_ShouldCollide_00c0a2f0 + unaff_EBX + 3) = 0xb;
  *(int *)((int)&PTR_ComputeWorldSpaceSurroundingBox_00c0a2ec + unaff_EBX + 3) =
       unaff_EBX + 0xc0a88f;
  return (datamap_t *)((int)&PTR_ComputeWorldSpaceSurroundingBox_00c0a2ec + unaff_EBX + 3);
}


/* __static_initialization_and_destruction_0 at 000bf3f0 */

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
    *(undefined1 *)((int)&sv_pushaway_force.m_pParent + unaff_EBX) = 0;
    *(undefined1 *)((int)&sv_pushaway_force.m_pParent + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&sv_pushaway_force.m_pParent + unaff_EBX + 2) = 0;
    *(undefined1 *)((int)&sv_pushaway_force.m_pParent + unaff_EBX + 3) = 0;
    *(undefined4 *)((int)&sv_pushaway_force.m_pszDefaultValue + unaff_EBX) = 0;
    *(undefined4 *)((int)&sv_pushaway_force.m_Value.m_pszString + unaff_EBX) = 0;
    *(undefined4 *)((int)&sv_pushaway_force.m_Value.m_StringLength + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&sv_pushaway_force.m_Value.m_fValue + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&sv_pushaway_force.m_Value.m_nValue + unaff_EBX) = 0;
    *(undefined4 *)(&sv_pushaway_force.m_bHasMin + unaff_EBX) = 0;
    *(undefined4 *)((int)&sv_pushaway_force.m_fMinVal + unaff_EBX) = 0;
    *(undefined4 *)(&sv_pushaway_force.m_bHasMax + unaff_EBX) = 0;
    *(undefined4 *)((int)&sv_pushaway_force.m_fMaxVal + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&sv_pushaway_force.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)
     ((int)&sv_pushaway_force.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&sv_pushaway_force.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&sv_pushaway_force.m_fnChangeCallbacks.m_Size + unaff_EBX) = 0;
    *(undefined1 *)((int)&sv_pushaway_force.m_fnChangeCallbacks.m_pElements + unaff_EBX + 3) = 0;
    *(undefined1 *)((int)&sv_pushaway_force.m_fnChangeCallbacks.m_pElements + unaff_EBX) = 0;
    *(undefined1 *)((int)&sv_pushaway_force.m_fnChangeCallbacks.m_pElements + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&sv_pushaway_force.m_fnChangeCallbacks.m_pElements + unaff_EBX + 2) = 0;
    *(undefined4 *)(&DAT_00cf75b8 + unaff_EBX) = 0;
    (&DAT_00cf75bf)[unaff_EBX] = 0;
    (&DAT_00cf75bc)[unaff_EBX] = 0;
    (&DAT_00cf75bd)[unaff_EBX] = 0;
    (&DAT_00cf75be)[unaff_EBX] = 0;
    *(undefined4 *)
     ((int)&sv_pushaway_min_player_speed.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) = 0;
    *(undefined1 *)((int)&sv_pushaway_min_player_speed.super_ConCommandBase.m_pNext + unaff_EBX + 3)
         = 1;
    *(undefined1 *)((int)&sv_pushaway_min_player_speed.super_ConCommandBase.m_pNext + unaff_EBX) = 0
    ;
    *(undefined1 *)((int)&sv_pushaway_min_player_speed.super_ConCommandBase.m_pNext + unaff_EBX + 1)
         = 0;
    *(undefined1 *)((int)&sv_pushaway_min_player_speed.super_ConCommandBase.m_pNext + unaff_EBX + 2)
         = 0;
    *(undefined4 *)(&sv_pushaway_min_player_speed.super_ConCommandBase.m_bRegistered + unaff_EBX) =
         1;
    *(undefined1 *)
     ((int)&sv_pushaway_min_player_speed.super_ConCommandBase.m_pszName + unaff_EBX + 3) = 0;
    *(undefined1 *)((int)&sv_pushaway_min_player_speed.super_ConCommandBase.m_pszName + unaff_EBX) =
         0;
    *(undefined1 *)
     ((int)&sv_pushaway_min_player_speed.super_ConCommandBase.m_pszName + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&sv_pushaway_min_player_speed.super_ConCommandBase.m_pszName + unaff_EBX + 2) = 0;
    *(undefined4 *)
     ((int)&sv_pushaway_min_player_speed.super_ConCommandBase.m_pszHelpString + unaff_EBX) = 2;
    *(undefined1 *)
     ((int)&sv_pushaway_min_player_speed.super_ConCommandBase.m_nFlags + unaff_EBX + 3) = 0;
    *(undefined1 *)((int)&sv_pushaway_min_player_speed.super_ConCommandBase.m_nFlags + unaff_EBX) =
         0;
    *(undefined1 *)
     ((int)&sv_pushaway_min_player_speed.super_ConCommandBase.m_nFlags + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&sv_pushaway_min_player_speed.super_ConCommandBase.m_nFlags + unaff_EBX + 2) = 0;
    *(undefined **)((int)&sv_pushaway_min_player_speed.super_IConVar._vptr_IConVar + unaff_EBX) =
         &UNK_00af1f24 + unaff_EBX;
    pdVar2 = DataMapInit<CLabIndicatorPanel>((CLabIndicatorPanel *)0x0);
    *(datamap_t **)((int)DataMapInit<CBeam>::dataDesc[0].flatOffset + unaff_EBX + 4) = pdVar2;
    iVar4 = unaff_EBX + 0xcf75dc;
    s2 = (char *)(unaff_EBX + 0x94f7ab);
    *(char **)((int)&sv_pushaway_min_player_speed.m_pParent + unaff_EBX) = s2;
    this = (SendTable *)((int)DataMapInit<CBeam>::dataDesc[0].flatOffset + unaff_EBX + 8);
    *(SendTable **)((int)&sv_pushaway_min_player_speed.m_pszDefaultValue + unaff_EBX) = this;
    *(undefined4 *)((int)&sv_pushaway_min_player_speed.m_Value.m_fValue + unaff_EBX) = 0xffff;
    puVar7 = (undefined4 *)**(int **)(&DAT_00aec82c + unaff_EBX);
    if (puVar7 == (undefined4 *)0x0) {
      **(int **)(&DAT_00aec82c + unaff_EBX) = iVar4;
      *(undefined4 *)((int)&sv_pushaway_min_player_speed.m_Value.m_pszString + unaff_EBX) = 0;
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
        *(undefined4 **)((int)&sv_pushaway_min_player_speed.m_Value.m_pszString + unaff_EBX) =
             puVar6;
        puVar7[2] = iVar4;
      }
      else {
        *(undefined4 *)((int)&sv_pushaway_min_player_speed.m_Value.m_pszString + unaff_EBX) =
             **(undefined4 **)(&DAT_00aec82c + unaff_EBX);
        **(int **)(&DAT_00aec82c + unaff_EBX) = iVar4;
      }
    }
    SendTable::SendTable(this);
    ___cxa_atexit(unaff_EBX + 0x8da02c,0,*(undefined4 *)(&DAT_00aec78c + unaff_EBX));
    iVar4 = ServerClassInit<DT_LabIndicatorPanel::ignored>((ignored *)0x0);
    *(int *)((int)DataMapInit<CBeam>::dataDesc[1].flatOffset + unaff_EBX + -0x24) = iVar4;
    *(undefined **)((int)&sv_pushaway_min_player_speed.m_Value.m_nValue + unaff_EBX) =
         &UNK_00b71bf4 + unaff_EBX;
    pIVar5 = EntityFactoryDictionary();
    (**pIVar5->_vptr_IEntityFactoryDictionary)(pIVar5,unaff_EBX + 0xcf75f0,unaff_EBX + 0x94f70c);
    pdVar2 = DataMapInit<CPropIndicatorPanel>((CPropIndicatorPanel *)0x0);
    *(datamap_t **)((int)DataMapInit<CBeam>::dataDesc[1].flatOffset + unaff_EBX + -0x20) = pdVar2;
    *(undefined **)(&sv_pushaway_min_player_speed.m_bHasMin + unaff_EBX) = &UNK_00b71c14 + unaff_EBX
    ;
    pIVar5 = EntityFactoryDictionary();
    (**pIVar5->_vptr_IEntityFactoryDictionary)(pIVar5,unaff_EBX + 0xcf75f4,unaff_EBX + 0x94f7be);
    pdVar2 = DataMapInit<CPropTicTacToePanel>((CPropTicTacToePanel *)0x0);
    *(datamap_t **)((int)DataMapInit<CBeam>::dataDesc[1].flatOffset + unaff_EBX + -0x1c) = pdVar2;
    *(undefined **)((int)&sv_pushaway_min_player_speed.m_fMinVal + unaff_EBX) =
         &UNK_00b71c34 + unaff_EBX;
    pIVar5 = EntityFactoryDictionary();
    (**pIVar5->_vptr_IEntityFactoryDictionary)(pIVar5,unaff_EBX + 0xcf75f8,unaff_EBX + 0x94f7d3);
  }
  return;
}


/* CLabIndicatorPanel::GetDataDescMap at 006b6ce0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CLabIndicatorPanel * this) */

datamap_t * __thiscall CLabIndicatorPanel::GetDataDescMap(CLabIndicatorPanel *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x612938);
}


/* CLabIndicatorPanel::GetBaseMap at 006b6cf0 */

datamap_t * CLabIndicatorPanel::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4f4fd4);
}


/* __tcf_0 at 00999450 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41d986)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41d97a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41d986));
  }
  *(undefined4 *)(unaff_EBX + 0x41d986) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41d982)) {
    if (*(int *)(unaff_EBX + 0x41d97a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x21273a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x21273a),*(int *)(unaff_EBX + 0x41d97a));
      *(undefined4 *)(unaff_EBX + 0x41d97a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d97e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41d97a);
  *(int *)(unaff_EBX + 0x41d98a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41d982)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x21273a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x21273a),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41d97a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d97e) = 0;
  }
  return;
}


/* ServerClassInit<DT_LabIndicatorPanel::ignored> at 000bec40 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */

int ServerClassInit<DT_LabIndicatorPanel::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&debug_paint_server_blobs.super_ConCommandBase.m_pszHelpString + unaff_EBX + 2)
      == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf7e12);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  ((int)&debug_paint_server_blobs.m_Value.m_nValue + unaff_EBX + 2),
                  (char *)(unaff_EBX + 0x8f3dd6),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&debug_paintblobs_streaking.m_Value.m_pszString + unaff_EBX + 2),
                        (char *)(unaff_EBX + 0x8f6146),0,
                        (SendTable *)**(undefined4 **)(&DAT_00aed03a + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00aecfde + unaff_EBX),0x80);
      SendPropBool((SendProp_conflict *)
                   ((int)&paintblob_gravity_scale.super_IConVar._vptr_IConVar + unaff_EBX + 2),
                   (char *)(unaff_EBX + 0x94ec5e),0x3c0,1);
      SendPropBool((SendProp_conflict *)
                   ((int)&paintblob_air_drag.super_ConCommandBase.m_pszName + unaff_EBX + 2),
                   (char *)(vgui::Panel::IsKeyOverridden + unaff_EBX + 2),0x3c1,1);
      SendPropBool((SendProp_conflict *)
                   ((int)&paintblob_minimum_portal_exit_velocity.super_ConCommandBase.
                          _vptr_ConCommandBase + unaff_EBX + 2),(char *)(unaff_EBX + 0x94fed7),0x3c2
                   ,1);
      SendPropBool((SendProp_conflict *)
                   ((int)&paintblob_minimum_portal_exit_velocity.m_fnChangeCallbacks.m_pElements +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0x94fee7),0x3c3,1);
      SendPropBool((SendProp_conflict *)
                   ((int)&paintblob_min_radius_scale.m_fnChangeCallbacks.m_Memory.m_nAllocationCount
                   + unaff_EBX + 2),(char *)(unaff_EBX + 0x94fefb),0x3c4,1);
      SendPropInt((SendProp_conflict *)(&paintblob_max_radius_scale.field_0x3e + unaff_EBX),
                  (char *)(unaff_EBX + 0x94ff08),0x3c8,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&paintblob_radius_while_streaking.m_Value.m_nValue + unaff_EBX + 2),
                    (char *)(unaff_EBX + 0x94ff18),0x3cc,4,0x20,0,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00aed036 + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&paintblob_streak_angle_threshold.m_Value.m_pszString + unaff_EBX + 2),
                    (char *)(unaff_EBX + 0x94ff2a),0x3d0,4,0x20,0,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00aed036 + unaff_EBX),0x80);
      ___cxa_guard_release(unaff_EBX + 0xcf7e12);
      ___cxa_atexit(&UNK_008da722 + unaff_EBX,0,*(undefined4 *)(&DAT_00aecf42 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)
             ((int)DataMapInit<CCollisionProperty>::dataDesc[0].flatOffset + unaff_EBX + -2),
             (SendProp_conflict *)
             ((int)&debug_paintblobs_streaking.m_Value.m_pszString + unaff_EBX + 2),9,
             *(char **)(&DAT_00b7238a + unaff_EBX));
  return 1;
}


/* CLabIndicatorPanel::GetServerClass at 006b6d00 */

/* DWARF original prototype: ServerClass * GetServerClass(CLabIndicatorPanel * this) */

ServerClass * __thiscall CLabIndicatorPanel::GetServerClass(CLabIndicatorPanel *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x6ffcd8);
}


/* CLabIndicatorPanel::YouForgotToImplementOrDeclareServerClass at 006b6d10 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CLabIndicatorPanel * this)
    */

int __thiscall
CLabIndicatorPanel::YouForgotToImplementOrDeclareServerClass(CLabIndicatorPanel *this)

{
  return 0;
}


/* __tcf_4 at 00999430 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x395468),in_stack_00000008);
  return;
}


/* __tcf_1 at 00999370 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d9f7))(unaff_EBX + 0x41d9f7);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d9a3))(unaff_EBX + 0x41d9a3);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d94f))(unaff_EBX + 0x41d94f);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d8fb))(unaff_EBX + 0x41d8fb);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d8a7))(unaff_EBX + 0x41d8a7);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d853))(unaff_EBX + 0x41d853);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d7ff))(unaff_EBX + 0x41d7ff);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d7ab))(unaff_EBX + 0x41d7ab);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d757))(unaff_EBX + 0x41d757);
                    /* WARNING: Could not recover jumptable at 0x00999427. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d703))();
  return;
}


/* CLabIndicatorPanel::~CLabIndicatorPanel at 006b6ff0 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x579c84);
  uVar1 = (this->m_hScreen).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f4ba0),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (pVGuiScreen = *(CVGuiScreen **)(iVar2 + 4), pVGuiScreen != (CVGuiScreen *)0x0)) {
    DestroyVGuiScreen(pVGuiScreen);
  }
  CBaseEntity::~CBaseEntity(&this->super_CBaseEntity,in_stack_ffffffd8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CLabIndicatorPanel::~CLabIndicatorPanel at 006b7090 */

/* DWARF original prototype: void ~CLabIndicatorPanel(CLabIndicatorPanel * this, int __in_chrg) */

void __thiscall CLabIndicatorPanel::~CLabIndicatorPanel(CLabIndicatorPanel *this,int __in_chrg)

{
  uint uVar1;
  CVGuiScreen *pVGuiScreen;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x579be4);
  uVar1 = (this->m_hScreen).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f4b00),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (pVGuiScreen = *(CVGuiScreen **)(iVar2 + 4), pVGuiScreen != (CVGuiScreen *)0x0)) {
    DestroyVGuiScreen(pVGuiScreen);
  }
  CBaseEntity::~CBaseEntity(&this->super_CBaseEntity,__in_chrg);
  return;
}


/* CLabIndicatorPanel::~CLabIndicatorPanel at 006b7120 */

/* DWARF original prototype: void ~CLabIndicatorPanel(CLabIndicatorPanel * this, int __in_chrg,
   CLabIndicatorPanel * this) */

void __thiscall
CLabIndicatorPanel::~CLabIndicatorPanel
          (CLabIndicatorPanel *this,int __in_chrg,CLabIndicatorPanel *this_1)

{
  ~CLabIndicatorPanel(this,__in_chrg);
  return;
}


/* CLabIndicatorPanel::UpdateTransmitState at 006b6fd0 */

/* DWARF original prototype: int UpdateTransmitState(CLabIndicatorPanel * this) */

int __thiscall CLabIndicatorPanel::UpdateTransmitState(CLabIndicatorPanel *this)

{
  int iVar1;
  
  iVar1 = CBaseEntity::SetTransmitState(&this->super_CBaseEntity,0);
  return iVar1;
}


/* CLabIndicatorPanel::SetTransmit at 006b7130 */

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
    uVar4 = (int)peVar1 - *(int *)(**(int **)(unaff_EBX + 0x4f4a5c) + 0x58) >> 4;
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
        (iVar3 = (uVar4 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f4a60),
        *(uint *)(iVar3 + 8) == uVar4 >> 0x10)) &&
       (piVar2 = *(int **)(iVar3 + 4), piVar2 != (int *)0x0)) {
                    /* WARNING: Could not recover jumptable at 0x006b71fe. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (**(code **)(*piVar2 + 0x5c))();
      return;
    }
  }
  return;
}


/* CLabIndicatorPanel::Spawn at 006b78b0 */

/* DWARF original prototype: void Spawn(CLabIndicatorPanel * this) */

void __thiscall CLabIndicatorPanel::Spawn(CLabIndicatorPanel *this)

{
  undefined1 *puVar1;
  CBaseEdict *pCVar2;
  IChangeInfoAccessor *pIVar3;
  
  (*(this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x1a])(this);
  CBaseEntity::Spawn(&this->super_CBaseEntity);
  SpawnControlPanels(this);
  if ((this->m_bEnabled).m_Value == false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar2 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
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
      pCVar2 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
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
      pCVar2 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
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
      pCVar2 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
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


/* CLabIndicatorPanel::Precache at 006b6fb0 */

/* DWARF original prototype: void Precache(CLabIndicatorPanel * this) */

void __thiscall CLabIndicatorPanel::Precache(CLabIndicatorPanel *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  PrecacheVGuiScreen((char *)(extraout_ECX + 0x357b48));
  return;
}


/* CLabIndicatorPanel::OnRestore at 006b7460 */

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
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f4737),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (oldObj = *(CBaseEntity **)(iVar2 + 4), oldObj != (CBaseEntity *)0x0)) {
    UTIL_RemoveImmediate(oldObj);
  }
  SpawnControlPanels(this);
  ScreenVisible(this,(this->m_bEnabled).m_Value);
  return;
}


/* CLabIndicatorPanel::ScreenVisible at 006b7300 */

/* DWARF original prototype: void ScreenVisible(CLabIndicatorPanel * this, bool bVisible) */

void __thiscall CLabIndicatorPanel::ScreenVisible(CLabIndicatorPanel *this,bool bVisible)

{
  undefined1 *puVar1;
  uint uVar2;
  CBaseEntity *this_00;
  CBaseEdict *this_01;
  edict_t *peVar3;
  int iVar4;
  IChangeInfoAccessor *pIVar5;
  int unaff_EBX;
  uint uVar6;
  
  ___i686_get_pc_thunk_bx();
  uVar2 = (this->m_hScreen).super_CBaseHandle.m_Index;
  if (((uVar2 != 0xffffffff) &&
      (iVar4 = (uVar2 & 0xffff) * 0x10 + **(int **)(&DAT_004f4890 + unaff_EBX),
      *(uint *)(iVar4 + 8) == uVar2 >> 0x10)) &&
     (this_00 = *(CBaseEntity **)(iVar4 + 4), this_00 != (CBaseEntity *)0x0)) {
    if (bVisible) {
      uVar2 = (this_00->m_fEffects).m_Value;
      uVar6 = uVar2 & 0xffffffdf;
      if (uVar2 != uVar6) {
        if ((this_00->m_Network).m_TimerEvent.m_bRegistered == false) {
          this_01 = &((this_00->m_Network).m_pPev)->super_CBaseEdict;
          if (this_01 != (CBaseEdict *)0x0) {
            this_01->m_fStateFlags = this_01->m_fStateFlags | 0x101;
            pIVar5 = CBaseEdict::GetChangeAccessor(this_01);
            pIVar5->m_iChangeInfoSerialNumber = 0;
          }
        }
        else {
          puVar1 = &(this_00->m_Network).field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (this_00->m_fEffects).m_Value = uVar6;
      }
      peVar3 = (this_00->m_Network).m_pPev;
      if (peVar3 != (edict_t *)0x0) {
        (peVar3->super_CBaseEdict).m_fStateFlags = (peVar3->super_CBaseEdict).m_fStateFlags | 0x80;
      }
      CBaseEntity::DispatchUpdateTransmitState(this_00);
      return;
    }
    CBaseEntity::AddEffects(this_00,0x20);
    return;
  }
  return;
}


/* CLabIndicatorPanel::Disable at 006b73f0 */

/* DWARF original prototype: void Disable(CLabIndicatorPanel * this) */

void __thiscall CLabIndicatorPanel::Disable(CLabIndicatorPanel *this)

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


/* CLabIndicatorPanel::InputDisable at 006b8030 */

/* DWARF original prototype: void InputDisable(CLabIndicatorPanel * this, inputdata_t * inputdata)
    */

void __thiscall CLabIndicatorPanel::InputDisable(CLabIndicatorPanel *this,inputdata_t *inputdata)

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


/* CLabIndicatorPanel::Enable at 006b7a40 */

/* DWARF original prototype: void Enable(CLabIndicatorPanel * this) */

void __thiscall CLabIndicatorPanel::Enable(CLabIndicatorPanel *this)

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


/* CLabIndicatorPanel::InputEnable at 006b7790 */

/* DWARF original prototype: void InputEnable(CLabIndicatorPanel * this, inputdata_t * inputdata) */

void __thiscall CLabIndicatorPanel::InputEnable(CLabIndicatorPanel *this,inputdata_t *inputdata)

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


/* CLabIndicatorPanel::GetControlPanelInfo at 006b6d20 */

/* DWARF original prototype: void GetControlPanelInfo(CLabIndicatorPanel * this, int nPanelIndex,
   char * * pPanelName) */

void __thiscall
CLabIndicatorPanel::GetControlPanelInfo(CLabIndicatorPanel *this,int nPanelIndex,char **pPanelName)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *pPanelName = (char *)(extraout_ECX + 0x357dd8);
  return;
}


/* CLabIndicatorPanel::GetControlPanelClassName at 006b6d40 */

/* DWARF original prototype: void GetControlPanelClassName(CLabIndicatorPanel * this, int
   nPanelIndex, char * * pPanelName) */

void __thiscall
CLabIndicatorPanel::GetControlPanelClassName
          (CLabIndicatorPanel *this,int nPanelIndex,char **pPanelName)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *pPanelName = (char *)(extraout_ECX + 0x356b58);
  return;
}


/* CLabIndicatorPanel::SpawnControlPanels at 006b6e70 */

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
  this_00 = CreateVGuiScreen((char *)(unaff_EBX + 0x356a1c),&UNK_00357c7c + unaff_EBX,
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


/* CLabIndicatorPanel::RestoreControlPanels at 006b6f40 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
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
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(CNPC_FloorTurret::BreakThink + unaff_EBX + 3),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (oldObj = *(CBaseEntity **)(iVar2 + 4), oldObj != (CBaseEntity *)0x0)) {
    UTIL_RemoveImmediate(oldObj);
  }
  SpawnControlPanels(this);
  return;
}


/* CLabIndicatorPanel::StartTimer at 006b7c50 */

/* DWARF original prototype: void StartTimer(CLabIndicatorPanel * this, float flDuration) */

void __thiscall CLabIndicatorPanel::StartTimer(CLabIndicatorPanel *this,float flDuration)

{
  undefined1 *puVar1;
  float fVar2;
  CBaseEdict *pCVar3;
  IChangeInfoAccessor *pIVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  fVar2 = *(float *)(**(int **)(unaff_EBX + 0x4f3f43) + 0xc);
  if (fVar2 != (this->m_flTimerStart).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar3 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar3 != (CBaseEdict *)0x0) {
        pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
        pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
        pIVar4->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flTimerStart).m_Value = fVar2;
  }
  if ((this->m_bStopped).m_Value != false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar3 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar3 != (CBaseEdict *)0x0) {
        pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
        pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
        pIVar4->m_iChangeInfoSerialNumber = 0;
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
      pCVar3 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar3 != (CBaseEdict *)0x0) {
        pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
        pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
        pIVar4->m_iChangeInfoSerialNumber = 0;
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
      pCVar3 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar3 != (CBaseEdict *)0x0) {
        pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
        pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
        pIVar4->m_iChangeInfoSerialNumber = 0;
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
      pCVar3 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar3 != (CBaseEdict *)0x0) {
        pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
        pIVar4 = CBaseEdict::GetChangeAccessor(pCVar3);
        pIVar4->m_iChangeInfoSerialNumber = 0;
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


/* CLabIndicatorPanel::ResetTimer at 006b7ab0 */

/* DWARF original prototype: void ResetTimer(CLabIndicatorPanel * this) */

void __thiscall CLabIndicatorPanel::ResetTimer(CLabIndicatorPanel *this)

{
  undefined1 *puVar1;
  CBaseEdict *pCVar2;
  IChangeInfoAccessor *pIVar3;
  
  if ((this->m_bStopped).m_Value == false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar2 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
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
      pCVar2 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
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
      pCVar2 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
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


/* CLabIndicatorPanel::SetChecked at 006b7f80 */

/* DWARF original prototype: void SetChecked(CLabIndicatorPanel * this, bool bIsChecked) */

void __thiscall CLabIndicatorPanel::SetChecked(CLabIndicatorPanel *this,bool bIsChecked)

{
  undefined1 *puVar1;
  CBaseEdict *pCVar2;
  IChangeInfoAccessor *pIVar3;
  
  if (bIsChecked != (this->m_bIsChecked).m_Value) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar2 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
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
      pCVar2 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
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


/* CLabIndicatorPanel::SetPlayerPinged at 006b7800 */

/* DWARF original prototype: void SetPlayerPinged(CLabIndicatorPanel * this, int iPlayerPinged) */

void __thiscall CLabIndicatorPanel::SetPlayerPinged(CLabIndicatorPanel *this,int iPlayerPinged)

{
  undefined1 *puVar1;
  CBaseEdict *pCVar2;
  IChangeInfoAccessor *pIVar3;
  
  if ((this->m_iPlayerPinged).m_Value != iPlayerPinged) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      pCVar2 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
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
      pCVar2 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar3 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar3->m_iChangeInfoSerialNumber = 0;
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


/* CLabIndicatorPanel::SetIsTimer at 006b7730 */

/* DWARF original prototype: void SetIsTimer(CLabIndicatorPanel * this, bool bIsTimer) */

void __thiscall CLabIndicatorPanel::SetIsTimer(CLabIndicatorPanel *this,bool bIsTimer)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  
  if (bIsTimer != (this->m_bIsCountdownTimer).m_Value) {
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
    (this->m_bIsCountdownTimer).m_Value = bIsTimer;
  }
  return;
}


/* DataMapInit<CPropIndicatorPanel> at 000befe0 */

datamap_t * DataMapInit<CPropIndicatorPanel>(CPropIndicatorPanel *param_1)

{
  int iVar1;
  int iVar2;
  char *pcVar3;
  size_t sVar4;
  char *pcVar5;
  int iVar6;
  undefined4 *puVar7;
  int iVar8;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((*(char *)((int)&g_PhysSaveRestoreBlockHandler.m_PhysObjectModels.m_Tree.m_pElements +
                unaff_EBX) == '\0') &&
     (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcf7a3c), iVar2 != 0)) {
    *(int *)((int)&g_PhysSaveRestoreBlockHandler.m_PhysObjectCustomModels.m_Tree.m_Elements.
                   m_nGrowSize + unaff_EBX) = unaff_EBX + 0x94fb93;
    *(undefined4 *)
     ((int)&g_PhysSaveRestoreBlockHandler.m_PhysObjectCustomModels.m_Tree.m_FirstFree + unaff_EBX) =
         0;
    *(undefined4 *)
     ((int)&g_PhysSaveRestoreBlockHandler.m_PhysObjectCustomModels.m_Tree.m_pElements + unaff_EBX) =
         0;
    *(undefined4 *)
     ((int)&g_PhysSaveRestoreBlockHandler.m_PhysCollideBBoxModels.m_Tree.m_LessFunc.m_LessFunc +
     unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&g_PhysSaveRestoreBlockHandler.m_PhysCollideBBoxModels.m_Tree.m_Elements.m_pMemory +
     unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&g_PhysSaveRestoreBlockHandler.m_PhysCollideBBoxModels.m_Tree.m_Elements.
            m_nAllocationCount + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&g_PhysSaveRestoreBlockHandler.m_PhysObjectCustomModels.m_Tree.m_Root + unaff_EBX) = 0x13
    ;
    ___cxa_guard_release(unaff_EBX + 0xcf7a3c);
    ___cxa_atexit(&UNK_008da61c + unaff_EBX,0,*(undefined4 *)(&DAT_00aecb9c + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_KeyValue_00c0a654 + unaff_EBX) =
       *(undefined4 *)(&DAT_00aeccd8 + unaff_EBX);
  if ((*(char *)((int)&g_PhysSaveRestoreBlockHandler.m_PhysObjectCustomModels.m_Tree.m_Elements.
                       m_pMemory + unaff_EBX) == '\0') &&
     (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcf7a44), iVar2 != 0)) {
    pcVar3 = operator_new___(*(int *)((int)&g_PhysSaveRestoreBlockHandler.m_PhysObjectCustomModels.
                                            m_Tree.m_Root + unaff_EBX) + 0xc);
    _strcpy(pcVar3,*(char **)((int)&g_PhysSaveRestoreBlockHandler.m_PhysObjectCustomModels.m_Tree.
                                    m_Elements.m_nGrowSize + unaff_EBX));
    sVar4 = _strlen(pcVar3);
    pcVar5 = pcVar3 + sVar4;
    pcVar5[0] = 'T';
    pcVar5[1] = 'o';
    pcVar5[2] = 'g';
    pcVar5[3] = 'g';
    pcVar5[4] = 'l';
    pcVar5[5] = 'e';
    pcVar5[6] = 'T';
    pcVar5[7] = 'h';
    *(undefined **)(pcVar5 + 8) = &DAT_006b6e69;
    iVar1 = *(int *)((int)&g_PhysSaveRestoreBlockHandler.m_PhysCollideBBoxModels.m_Tree.m_Elements.
                           m_pMemory + unaff_EBX);
    iVar2 = iVar1 + 1;
    iVar8 = *(int *)((int)&g_PhysSaveRestoreBlockHandler.m_PhysObjectCustomModels.m_Tree.m_pElements
                    + unaff_EBX);
    if (iVar8 < iVar2) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)
                 ((int)&g_PhysSaveRestoreBlockHandler.m_PhysObjectCustomModels.m_Tree.m_FirstFree +
                 unaff_EBX),iVar2 - iVar8);
    }
    iVar6 = *(int *)((int)&g_PhysSaveRestoreBlockHandler.m_PhysCollideBBoxModels.m_Tree.m_Elements.
                           m_pMemory + unaff_EBX) + 1;
    *(int *)((int)&g_PhysSaveRestoreBlockHandler.m_PhysCollideBBoxModels.m_Tree.m_Elements.m_pMemory
            + unaff_EBX) = iVar6;
    iVar8 = *(int *)((int)&g_PhysSaveRestoreBlockHandler.m_PhysObjectCustomModels.m_Tree.m_FirstFree
                    + unaff_EBX);
    *(int *)((int)&g_PhysSaveRestoreBlockHandler.m_PhysCollideBBoxModels.m_Tree.m_Elements.
                   m_nAllocationCount + unaff_EBX) = iVar8;
    iVar6 = (iVar6 - iVar1) + -1;
    if (0 < iVar6) {
      _V_memmove((void *)(iVar8 + iVar2 * 4),(void *)(iVar8 + iVar1 * 4),iVar6 * 4);
      iVar8 = *(int *)((int)&g_PhysSaveRestoreBlockHandler.m_PhysObjectCustomModels.m_Tree.
                             m_FirstFree + unaff_EBX);
    }
    puVar7 = (undefined4 *)(iVar1 * 4 + iVar8);
    if (puVar7 != (undefined4 *)0x0) {
      *puVar7 = pcVar3;
    }
    *(char **)((int)&PTR_GetGroundVelocityToApply_00c0ab50 + unaff_EBX) = pcVar3;
    ___cxa_guard_release(unaff_EBX + 0xcf7a44);
  }
  *(undefined4 *)((int)&PTR_KeyValue_00c0a64c + unaff_EBX) = 0xf;
  *(int *)((int)&PTR_KeyValue_00c0a648 + unaff_EBX) =
       (int)&PTR_NetworkStateChanged_m_lifeState_00c0a7cc + unaff_EBX;
  return (datamap_t *)((int)&PTR_KeyValue_00c0a648 + unaff_EBX);
}


/* CPropIndicatorPanel::GetDataDescMap at 006b6d60 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPropIndicatorPanel * this) */

datamap_t * __thiscall CPropIndicatorPanel::GetDataDescMap(CPropIndicatorPanel *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x6128d4);
}


/* CPropIndicatorPanel::GetBaseMap at 006b6d70 */

datamap_t * CPropIndicatorPanel::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4f4f54);
}


/* __tcf_2 at 00999610 */

void __tcf_2(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41d436)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41d42a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41d436));
  }
  *(undefined4 *)(unaff_EBX + 0x41d436) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41d432)) {
    if (*(int *)(unaff_EBX + 0x41d42a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x21257a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x21257a),*(int *)(unaff_EBX + 0x41d42a));
      *(undefined4 *)(unaff_EBX + 0x41d42a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d42e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41d42a);
  *(int *)(unaff_EBX + 0x41d43a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41d432)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x21257a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x21257a),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41d42a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d42e) = 0;
  }
  return;
}


/* CPropIndicatorPanel::Precache at 006b6e40 */

/* DWARF original prototype: void Precache(CPropIndicatorPanel * this) */

void __thiscall CPropIndicatorPanel::Precache(CPropIndicatorPanel *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  UTIL_PrecacheOther(&UNK_00357cc4 + unaff_EBX,(char *)0x0);
  return;
}


/* CPropIndicatorPanel::Spawn at 006b8a90 */

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


/* CPropIndicatorPanel::UpdateOnRemove at 006b6dd0 */

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
     (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f4dc3),
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


/* CPropIndicatorPanel::SetTimerDuration at 006b6d80 */

/* DWARF original prototype: void SetTimerDuration(CPropIndicatorPanel * this, float flDuration) */

void __thiscall CPropIndicatorPanel::SetTimerDuration(CPropIndicatorPanel *this,float flDuration)

{
  this->m_flTimerDuration = flDuration;
  return;
}


/* CPropIndicatorPanel::StartTimer at 006b7e20 */

/* DWARF original prototype: void StartTimer(CPropIndicatorPanel * this) */

void __thiscall CPropIndicatorPanel::StartTimer(CPropIndicatorPanel *this)

{
  undefined1 *puVar1;
  uint uVar2;
  CLabIndicatorPanel *this_00;
  CBaseEdict *this_01;
  BASEPTR func;
  int iVar3;
  char *szName;
  IChangeInfoAccessor *pIVar4;
  int unaff_EBX;
  CBaseEntity *pStartEntity;
  
  ___i686_get_pc_thunk_bx();
  uVar2 = (this->m_hIndicatorPanel).super_CBaseHandle.m_Index;
  if (((uVar2 != 0xffffffff) &&
      (iVar3 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f3d76),
      *(uint *)(iVar3 + 8) == uVar2 >> 0x10)) &&
     (this_00 = *(CLabIndicatorPanel **)(iVar3 + 4), this_00 != (CLabIndicatorPanel *)0x0)) {
    CLabIndicatorPanel::StartTimer(this_00,this->m_flTimerDuration);
    pStartEntity = (CBaseEntity *)0x0;
                    /* Unresolved local var: CBaseEntity * pLight@[???] */
    while( true ) {
      szName = (this->m_strIndicatorLights).pszValue;
      if (szName == (char *)0x0) {
        szName = (char *)(unaff_EBX + 0x2f3d2e);
      }
      pStartEntity = CGlobalEntityList::FindEntityByName
                               (*(CGlobalEntityList **)(unaff_EBX + 0x4f3e32),pStartEntity,szName,
                                (CBaseEntity *)0x0,(CBaseEntity *)0x0,(CBaseEntity *)0x0,
                                (IEntityFindFilter *)0x0);
      if (pStartEntity == (CBaseEntity *)0x0) break;
      if ((pStartEntity->m_iTextureFrameIndex).m_Value != '\x01') {
        if ((pStartEntity->m_Network).m_TimerEvent.m_bRegistered == false) {
          this_01 = &((pStartEntity->m_Network).m_pPev)->super_CBaseEdict;
          if (this_01 != (CBaseEdict *)0x0) {
            this_01->m_fStateFlags = this_01->m_fStateFlags | 0x101;
            pIVar4 = CBaseEdict::GetChangeAccessor(this_01);
            pIVar4->m_iChangeInfoSerialNumber = 0;
          }
        }
        else {
          puVar1 = &(pStartEntity->m_Network).field_0x4c;
          *(uint *)puVar1 = *(uint *)puVar1 | 1;
        }
        (pStartEntity->m_iTextureFrameIndex).m_Value = '\x01';
      }
    }
    func.__delta = 0;
    func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x94e);
    CBaseEntity::ThinkSet((CBaseEntity *)this,func,0.0,(char *)0x0);
    CBaseEntity::SetNextThink
              ((CBaseEntity *)this,
               *(float *)(**(int **)(unaff_EBX + 0x4f3d72) + 0xc) + this->m_flTimerDuration,
               (char *)0x0);
    return;
  }
  return;
}


/* CPropIndicatorPanel::StopTimer at 006b6da0 */

/* DWARF original prototype: void StopTimer(CPropIndicatorPanel * this) */

void __thiscall CPropIndicatorPanel::StopTimer(CPropIndicatorPanel *this)

{
  return;
}


/* CPropIndicatorPanel::ResetTimer at 006b7b90 */

/* DWARF original prototype: void ResetTimer(CPropIndicatorPanel * this) */

void __thiscall CPropIndicatorPanel::ResetTimer(CPropIndicatorPanel *this)

{
  uint uVar1;
  CLabIndicatorPanel *this_00;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->m_hIndicatorPanel).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f400b),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (this_00 = *(CLabIndicatorPanel **)(iVar2 + 4), this_00 != (CLabIndicatorPanel *)0x0)) {
    CLabIndicatorPanel::ResetTimer(this_00);
    return;
  }
  return;
}


/* CPropIndicatorPanel::InputReset at 006b7bf0 */

/* DWARF original prototype: void InputReset(CPropIndicatorPanel * this, inputdata_t * indata) */

void __thiscall CPropIndicatorPanel::InputReset(CPropIndicatorPanel *this,inputdata_t *indata)

{
  uint uVar1;
  CLabIndicatorPanel *this_00;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->m_hIndicatorPanel).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f3fab),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (this_00 = *(CLabIndicatorPanel **)(iVar2 + 4), this_00 != (CLabIndicatorPanel *)0x0)) {
    CLabIndicatorPanel::ResetTimer(this_00);
    return;
  }
  return;
}


/* CPropIndicatorPanel::InputStart at 006b7f70 */

/* DWARF original prototype: void InputStart(CPropIndicatorPanel * this, inputdata_t * indata) */

void __thiscall CPropIndicatorPanel::InputStart(CPropIndicatorPanel *this,inputdata_t *indata)

{
  StartTimer(this);
  return;
}


/* CPropIndicatorPanel::InputStop at 006b7220 */

/* DWARF original prototype: void InputStop(CPropIndicatorPanel * this, inputdata_t * indata) */

void __thiscall CPropIndicatorPanel::InputStop(CPropIndicatorPanel *this,inputdata_t *indata)

{
  return;
}


/* CPropIndicatorPanel::InputCheck at 006b75b0 */

/* DWARF original prototype: void InputCheck(CPropIndicatorPanel * this, inputdata_t * indata) */

void __thiscall CPropIndicatorPanel::InputCheck(CPropIndicatorPanel *this,inputdata_t *indata)

{
  undefined1 *puVar1;
  uint uVar2;
  CBaseEdict *pCVar3;
  int iVar4;
  char *szName;
  IChangeInfoAccessor *pIVar5;
  int unaff_EBX;
  CBaseEntity *pStartEntity;
  
  ___i686_get_pc_thunk_bx();
  uVar2 = (this->m_hIndicatorPanel).super_CBaseHandle.m_Index;
  if (((uVar2 != 0xffffffff) &&
      (iVar4 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f45e6),
      *(uint *)(iVar4 + 8) == uVar2 >> 0x10)) && (iVar4 = *(int *)(iVar4 + 4), iVar4 != 0)) {
    if (*(char *)(iVar4 + 0x3c4) == '\0') {
      if (*(char *)(iVar4 + 0x60) == '\0') {
        pCVar3 = *(CBaseEdict **)(iVar4 + 0x24);
        if (pCVar3 != (CBaseEdict *)0x0) {
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
          pIVar5 = CBaseEdict::GetChangeAccessor(pCVar3);
          pIVar5->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        *(uint *)(iVar4 + 100) = *(uint *)(iVar4 + 100) | 1;
      }
      *(undefined1 *)(iVar4 + 0x3c4) = 1;
    }
    if (*(char *)(iVar4 + 0x3c3) != '\0') {
      if (*(char *)(iVar4 + 0x60) == '\0') {
        pCVar3 = *(CBaseEdict **)(iVar4 + 0x24);
        if (pCVar3 != (CBaseEdict *)0x0) {
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
          pIVar5 = CBaseEdict::GetChangeAccessor(pCVar3);
          pIVar5->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        *(uint *)(iVar4 + 100) = *(uint *)(iVar4 + 100) | 1;
      }
      *(undefined1 *)(iVar4 + 0x3c3) = 0;
    }
  }
  pStartEntity = (CBaseEntity *)0x0;
  CBaseEntity::ThinkSet((CBaseEntity *)this,(BASEPTR)0x0,0.0,(char *)0x0);
                    /* Unresolved local var: CBaseEntity * pLight@[???] */
  while( true ) {
    szName = (this->m_strIndicatorLights).pszValue;
    if (szName == (char *)0x0) {
      szName = (char *)(unaff_EBX + 0x2f459e);
    }
    pStartEntity = CGlobalEntityList::FindEntityByName
                             (*(CGlobalEntityList **)(unaff_EBX + 0x4f46a2),pStartEntity,szName,
                              (CBaseEntity *)0x0,(CBaseEntity *)0x0,(CBaseEntity *)0x0,
                              (IEntityFindFilter *)0x0);
    if (pStartEntity == (CBaseEntity *)0x0) break;
    if ((pStartEntity->m_iTextureFrameIndex).m_Value != '\x01') {
      if ((pStartEntity->m_Network).m_TimerEvent.m_bRegistered == false) {
        pCVar3 = &((pStartEntity->m_Network).m_pPev)->super_CBaseEdict;
        if (pCVar3 != (CBaseEdict *)0x0) {
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
          pIVar5 = CBaseEdict::GetChangeAccessor(pCVar3);
          pIVar5->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(pStartEntity->m_Network).field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (pStartEntity->m_iTextureFrameIndex).m_Value = '\x01';
    }
  }
  return;
}


/* CPropIndicatorPanel::InputUncheck at 006b8c30 */

/* DWARF original prototype: void InputUncheck(CPropIndicatorPanel * this, inputdata_t * indata) */

void __thiscall CPropIndicatorPanel::InputUncheck(CPropIndicatorPanel *this,inputdata_t *indata)

{
  undefined1 *puVar1;
  uint uVar2;
  CBaseEdict *pCVar3;
  int iVar4;
  char *szName;
  IChangeInfoAccessor *pIVar5;
  int unaff_EBX;
  CBaseEntity *pStartEntity;
  
  ___i686_get_pc_thunk_bx();
  uVar2 = (this->m_hIndicatorPanel).super_CBaseHandle.m_Index;
  if (((uVar2 != 0xffffffff) &&
      (iVar4 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f2f66),
      *(uint *)(iVar4 + 8) == uVar2 >> 0x10)) && (iVar4 = *(int *)(iVar4 + 4), iVar4 != 0)) {
    if (*(char *)(iVar4 + 0x3c4) != '\0') {
      if (*(char *)(iVar4 + 0x60) == '\0') {
        pCVar3 = *(CBaseEdict **)(iVar4 + 0x24);
        if (pCVar3 != (CBaseEdict *)0x0) {
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
          pIVar5 = CBaseEdict::GetChangeAccessor(pCVar3);
          pIVar5->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        *(uint *)(iVar4 + 100) = *(uint *)(iVar4 + 100) | 1;
      }
      *(undefined1 *)(iVar4 + 0x3c4) = 0;
    }
    if (*(char *)(iVar4 + 0x3c3) != '\0') {
      if (*(char *)(iVar4 + 0x60) == '\0') {
        pCVar3 = *(CBaseEdict **)(iVar4 + 0x24);
        if (pCVar3 != (CBaseEdict *)0x0) {
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
          pIVar5 = CBaseEdict::GetChangeAccessor(pCVar3);
          pIVar5->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        *(uint *)(iVar4 + 100) = *(uint *)(iVar4 + 100) | 1;
      }
      *(undefined1 *)(iVar4 + 0x3c3) = 0;
    }
  }
  pStartEntity = (CBaseEntity *)0x0;
  CBaseEntity::ThinkSet((CBaseEntity *)this,(BASEPTR)0x0,0.0,(char *)0x0);
                    /* Unresolved local var: CBaseEntity * pLight@[???] */
  while( true ) {
    szName = (this->m_strIndicatorLights).pszValue;
    if (szName == (char *)0x0) {
      szName = (char *)(unaff_EBX + 0x2f2f1e);
    }
    pStartEntity = CGlobalEntityList::FindEntityByName
                             (*(CGlobalEntityList **)(unaff_EBX + 0x4f3022),pStartEntity,szName,
                              (CBaseEntity *)0x0,(CBaseEntity *)0x0,(CBaseEntity *)0x0,
                              (IEntityFindFilter *)0x0);
    if (pStartEntity == (CBaseEntity *)0x0) break;
    if ((pStartEntity->m_iTextureFrameIndex).m_Value != '\0') {
      if ((pStartEntity->m_Network).m_TimerEvent.m_bRegistered == false) {
        pCVar3 = &((pStartEntity->m_Network).m_pPev)->super_CBaseEdict;
        if (pCVar3 != (CBaseEdict *)0x0) {
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
          pIVar5 = CBaseEdict::GetChangeAccessor(pCVar3);
          pIVar5->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(pStartEntity->m_Network).field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (pStartEntity->m_iTextureFrameIndex).m_Value = '\0';
    }
  }
  return;
}


/* CPropIndicatorPanel::CreateIndicatorPanel at 006b84a0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CreateIndicatorPanel(CPropIndicatorPanel * this) */

void __thiscall CPropIndicatorPanel::CreateIndicatorPanel(CPropIndicatorPanel *this)

{
  bool bVar1;
  CBaseEdict *pCVar2;
  CBaseEntity *pCVar3;
  uint *puVar4;
  int iVar5;
  IChangeInfoAccessor *pIVar6;
  int *piVar7;
  uint uVar8;
  int unaff_EBX;
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
  pCVar3 = CreateEntityByName(&UNK_0035665f + unaff_EBX,-1,true);
  if (pCVar3 == (CBaseEntity *)0x0) {
    (this->m_hIndicatorPanel).super_CBaseHandle.m_Index = 0xffffffff;
    return;
  }
  puVar4 = (uint *)(*(pCVar3->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                     _vptr_IHandleEntity[3])(pCVar3);
  uVar8 = *puVar4;
  (this->m_hIndicatorPanel).super_CBaseHandle.m_Index = uVar8;
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
  if (((uVar8 != 0xffffffff) &&
      (iVar5 = (uVar8 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f36f3),
      *(uint *)(iVar5 + 8) == uVar8 >> 0x10)) && (*(int *)(iVar5 + 4) != 0)) {
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
    uVar8 = (this->m_hIndicatorPanel).super_CBaseHandle.m_Index;
    if ((uVar8 == 0xffffffff) ||
       (iVar5 = (uVar8 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f36f3),
       *(uint *)(iVar5 + 8) != uVar8 >> 0x10)) {
      pCVar3 = (CBaseEntity *)0x0;
    }
    else {
      pCVar3 = *(CBaseEntity **)(iVar5 + 4);
    }
    CBaseEntity::SetAbsAngles(pCVar3,local_28);
    (*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
      super_IHandleEntity._vptr_IHandleEntity[0x8e])(this,&local_40,&local_4c,&local_58);
                    /* Unresolved local var: Vector res@[???] */
    local_68 = *(float *)(unaff_EBX + 0x376afb);
    local_70 = local_58 * local_68;
    local_6c = local_54 * local_68;
    local_68 = local_68 * local_50;
                    /* Unresolved local var: Vector res@[???] */
    local_5c = *(float *)(unaff_EBX + 0x3720c3);
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
    uVar8 = (this->m_hIndicatorPanel).super_CBaseHandle.m_Index;
    if ((uVar8 == 0xffffffff) ||
       (iVar5 = (uVar8 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f36f3),
       *(uint *)(iVar5 + 8) != uVar8 >> 0x10)) {
      pCVar3 = (CBaseEntity *)0x0;
    }
    else {
      pCVar3 = *(CBaseEntity **)(iVar5 + 4);
    }
    CBaseEntity::SetAbsOrigin(pCVar3,&local_88);
    uVar8 = (this->m_hIndicatorPanel).super_CBaseHandle.m_Index;
    if ((uVar8 == 0xffffffff) ||
       (iVar5 = (uVar8 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f36f3),
       *(uint *)(iVar5 + 8) != uVar8 >> 0x10)) {
      piVar7 = (int *)0x0;
    }
    else {
      piVar7 = *(int **)(iVar5 + 4);
    }
    (**(code **)(*piVar7 + 0x98))(piVar7,this,0xffffffff);
    uVar8 = (this->m_hIndicatorPanel).super_CBaseHandle.m_Index;
    if ((uVar8 == 0xffffffff) ||
       (iVar5 = (uVar8 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f36f3),
       *(uint *)(iVar5 + 8) != uVar8 >> 0x10)) {
      iVar5 = 0;
    }
    else {
      iVar5 = *(int *)(iVar5 + 4);
    }
    if (*(int *)(iVar5 + 0x3c8) != -1) {
      if (*(char *)(iVar5 + 0x60) == '\0') {
        pCVar2 = *(CBaseEdict **)(iVar5 + 0x24);
        if (pCVar2 != (CBaseEdict *)0x0) {
          pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
          pIVar6 = CBaseEdict::GetChangeAccessor(pCVar2);
          pIVar6->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        *(uint *)(iVar5 + 100) = *(uint *)(iVar5 + 100) | 1;
      }
      *(undefined4 *)(iVar5 + 0x3c8) = 0xffffffff;
    }
    if (*(char *)(iVar5 + 0x3c3) != '\0') {
      if (*(char *)(iVar5 + 0x60) == '\0') {
        pCVar2 = *(CBaseEdict **)(iVar5 + 0x24);
        if (pCVar2 != (CBaseEdict *)0x0) {
          pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
          pIVar6 = CBaseEdict::GetChangeAccessor(pCVar2);
          pIVar6->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        *(uint *)(iVar5 + 100) = *(uint *)(iVar5 + 100) | 1;
      }
      *(undefined1 *)(iVar5 + 0x3c3) = 0;
    }
    bVar1 = this->m_bIsChecked;
    uVar8 = (this->m_hIndicatorPanel).super_CBaseHandle.m_Index;
    if ((uVar8 == 0xffffffff) ||
       (iVar5 = (uVar8 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f36f3),
       *(uint *)(iVar5 + 8) != uVar8 >> 0x10)) {
      iVar5 = 0;
    }
    else {
      iVar5 = *(int *)(iVar5 + 4);
    }
    if ((bool)*(char *)(iVar5 + 0x3c4) != bVar1) {
      if (*(char *)(iVar5 + 0x60) == '\0') {
        pCVar2 = *(CBaseEdict **)(iVar5 + 0x24);
        if (pCVar2 != (CBaseEdict *)0x0) {
          pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
          pIVar6 = CBaseEdict::GetChangeAccessor(pCVar2);
          pIVar6->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        *(uint *)(iVar5 + 100) = *(uint *)(iVar5 + 100) | 1;
      }
      *(bool *)(iVar5 + 0x3c4) = bVar1;
    }
    if (*(char *)(iVar5 + 0x3c3) != '\0') {
      if (*(char *)(iVar5 + 0x60) == '\0') {
        pCVar2 = *(CBaseEdict **)(iVar5 + 0x24);
        if (pCVar2 != (CBaseEdict *)0x0) {
          pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
          pIVar6 = CBaseEdict::GetChangeAccessor(pCVar2);
          pIVar6->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        *(uint *)(iVar5 + 100) = *(uint *)(iVar5 + 100) | 1;
      }
      *(undefined1 *)(iVar5 + 0x3c3) = 0;
    }
    bVar1 = this->m_bIsCountdownTimer;
    uVar8 = (this->m_hIndicatorPanel).super_CBaseHandle.m_Index;
    if ((uVar8 == 0xffffffff) ||
       (iVar5 = (uVar8 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f36f3),
       *(uint *)(iVar5 + 8) != uVar8 >> 0x10)) {
      iVar5 = 0;
    }
    else {
      iVar5 = *(int *)(iVar5 + 4);
    }
    if ((bool)*(char *)(iVar5 + 0x3c3) != bVar1) {
      if (*(char *)(iVar5 + 0x60) == '\0') {
        pCVar2 = *(CBaseEdict **)(iVar5 + 0x24);
        if (pCVar2 != (CBaseEdict *)0x0) {
          pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
          pIVar6 = CBaseEdict::GetChangeAccessor(pCVar2);
          pIVar6->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        *(uint *)(iVar5 + 100) = *(uint *)(iVar5 + 100) | 1;
      }
      *(bool *)(iVar5 + 0x3c3) = bVar1;
      uVar8 = (this->m_hIndicatorPanel).super_CBaseHandle.m_Index;
    }
    if ((uVar8 == 0xffffffff) ||
       (iVar5 = (uVar8 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f36f3),
       *(uint *)(iVar5 + 8) != uVar8 >> 0x10)) {
      pCVar3 = (CBaseEntity *)0x0;
    }
    else {
      pCVar3 = *(CBaseEntity **)(iVar5 + 4);
    }
    DispatchSpawn(pCVar3,true);
    return;
  }
  return;
}


/* CPropIndicatorPanel::ToggleIndicatorLights at 006b7230 */

/* DWARF original prototype: void ToggleIndicatorLights(CPropIndicatorPanel * this, bool bLightsOn)
    */

void __thiscall CPropIndicatorPanel::ToggleIndicatorLights(CPropIndicatorPanel *this,bool bLightsOn)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  char *szName;
  IChangeInfoAccessor *pIVar2;
  int unaff_EBX;
  CBaseEntity *pStartEntity;
  
                    /* Unresolved local var: CBaseEntity * pLight@[???] */
  ___i686_get_pc_thunk_bx();
  pStartEntity = (CBaseEntity *)0x0;
  while( true ) {
    szName = (this->m_strIndicatorLights).pszValue;
    if (szName == (char *)0x0) {
      szName = (char *)(unaff_EBX + 0x2f491e);
    }
    pStartEntity = CGlobalEntityList::FindEntityByName
                             (*(CGlobalEntityList **)(unaff_EBX + 0x4f4a22),pStartEntity,szName,
                              (CBaseEntity *)0x0,(CBaseEntity *)0x0,(CBaseEntity *)0x0,
                              (IEntityFindFilter *)0x0);
    if (pStartEntity == (CBaseEntity *)0x0) break;
    if ((bool)(pStartEntity->m_iTextureFrameIndex).m_Value != bLightsOn) {
      if ((pStartEntity->m_Network).m_TimerEvent.m_bRegistered == false) {
        this_00 = &((pStartEntity->m_Network).m_pPev)->super_CBaseEdict;
        if (this_00 != (CBaseEdict *)0x0) {
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
          pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
          pIVar2->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(pStartEntity->m_Network).field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (pStartEntity->m_iTextureFrameIndex).m_Value = bLightsOn;
    }
  }
  return;
}


/* CPropIndicatorPanel::ToggleThink at 006b74e0 */

/* DWARF original prototype: void ToggleThink(CPropIndicatorPanel * this) */

void __thiscall CPropIndicatorPanel::ToggleThink(CPropIndicatorPanel *this)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  char *szName;
  IChangeInfoAccessor *pIVar2;
  int unaff_EBX;
  CBaseEntity *pStartEntity;
  
  ___i686_get_pc_thunk_bx();
  pStartEntity = (CBaseEntity *)0x0;
                    /* Unresolved local var: CBaseEntity * pLight@[???] */
  while( true ) {
    szName = (this->m_strIndicatorLights).pszValue;
    if (szName == (char *)0x0) {
      szName = (char *)(unaff_EBX + 0x2f466e);
    }
    pStartEntity = CGlobalEntityList::FindEntityByName
                             (*(CGlobalEntityList **)(unaff_EBX + 0x4f4772),pStartEntity,szName,
                              (CBaseEntity *)0x0,(CBaseEntity *)0x0,(CBaseEntity *)0x0,
                              (IEntityFindFilter *)0x0);
    if (pStartEntity == (CBaseEntity *)0x0) break;
    if ((pStartEntity->m_iTextureFrameIndex).m_Value != '\0') {
      if ((pStartEntity->m_Network).m_TimerEvent.m_bRegistered == false) {
        this_00 = &((pStartEntity->m_Network).m_pPev)->super_CBaseEdict;
        if (this_00 != (CBaseEdict *)0x0) {
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
          pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
          pIVar2->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(pStartEntity->m_Network).field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (pStartEntity->m_iTextureFrameIndex).m_Value = '\0';
    }
  }
  CBaseEntity::ThinkSet((CBaseEntity *)this,(BASEPTR)0x0,0.0,(char *)0x0);
  return;
}


/* DataMapInit<CPropTicTacToePanel> at 000bf1f0 */

/* WARNING: Struct "CPropTicTacToePanel": ignoring overlapping field "m_bPlayed" */

datamap_t * DataMapInit<CPropTicTacToePanel>(CPropTicTacToePanel *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((&DAT_00cf77fc)[unaff_EBX] == '\0') {
    iVar2 = ___cxa_guard_acquire(&DAT_00cf77fc + unaff_EBX);
    if (iVar2 != 0) {
      *(int *)((int)&particle_test_attach_mode.super_ConCommandBase.m_pszName + unaff_EBX) =
           unaff_EBX + 0x94f997;
      *(undefined4 *)((int)&particle_test_attach_mode.super_ConCommandBase.m_nFlags + unaff_EBX) = 0
      ;
      *(undefined4 *)((int)&particle_test_attach_mode.super_IConVar._vptr_IConVar + unaff_EBX) = 0;
      *(undefined4 *)((int)&particle_test_attach_mode.m_pParent + unaff_EBX) = 0;
      *(undefined4 *)((int)&particle_test_attach_mode.m_pszDefaultValue + unaff_EBX) = 0;
      *(undefined4 *)((int)&particle_test_attach_mode.m_Value.m_pszString + unaff_EBX) = 0;
      *(undefined4 *)
       ((int)&particle_test_attach_mode.super_ConCommandBase.m_pszHelpString + unaff_EBX) = 0x13;
      ___cxa_guard_release(&DAT_00cf77fc + unaff_EBX);
      ___cxa_atexit(unaff_EBX + 0x8da32c,0,*(undefined4 *)(&DAT_00aec98c + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_PhysicsLandedOnGround_00c0a45c + unaff_EBX) =
       *(undefined4 *)(&DAT_00aecac8 + unaff_EBX);
  if (*(char *)((int)&particle_test_attach_mode.super_ConCommandBase.m_pNext + unaff_EBX) == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcf7804);
    if (iVar2 != 0) {
      uVar1 = **(undefined4 **)(&DAT_00aec9a8 + unaff_EBX);
      *(undefined4 *)((int)&PTR_GetGroundVelocityToApply_00c0a510 + unaff_EBX) = uVar1;
      *(undefined4 *)((int)&PTR_VPhysicsIsFlesh_00c0a550 + unaff_EBX) = uVar1;
      ___cxa_guard_release(unaff_EBX + 0xcf7804);
    }
  }
  *(undefined4 *)((int)&PTR_EndBlocked_00c0a454 + unaff_EBX) = 3;
  *(int *)((int)&PTR_Blocked_00c0a450 + unaff_EBX) = (int)&PTR_GetDamageType_00c0a4bc + unaff_EBX;
  return (datamap_t *)((int)&PTR_Blocked_00c0a450 + unaff_EBX);
}


/* CPropTicTacToePanel::GetDataDescMap at 006b6db0 */

/* WARNING: Struct "CPropTicTacToePanel": ignoring overlapping field "m_bPlayed" */
/* DWARF original prototype: datamap_t * GetDataDescMap(CPropTicTacToePanel * this) */

datamap_t * __thiscall CPropTicTacToePanel::GetDataDescMap(CPropTicTacToePanel *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x61289c);
}


/* CPropTicTacToePanel::GetBaseMap at 006b6dc0 */

datamap_t * CPropTicTacToePanel::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4f4f04);
}


/* __tcf_3 at 00999530 */

void __tcf_3(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41d4e6)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41d4da) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41d4e6));
  }
  *(undefined4 *)(unaff_EBX + 0x41d4e6) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41d4e2)) {
    if (*(int *)(unaff_EBX + 0x41d4da) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x21265a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x21265a),*(int *)(unaff_EBX + 0x41d4da));
      *(undefined4 *)(unaff_EBX + 0x41d4da) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d4de) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41d4da);
  *(int *)(unaff_EBX + 0x41d4ea) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41d4e2)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x21265a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x21265a),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41d4da) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d4de) = 0;
  }
  return;
}


/* CPropTicTacToePanel::Spawn at 006b8b00 */

/* WARNING: Struct "CPropTicTacToePanel": ignoring overlapping field "m_bPlayed" */
/* DWARF original prototype: void Spawn(CPropTicTacToePanel * this) */

void __thiscall CPropTicTacToePanel::Spawn(CPropTicTacToePanel *this)

{
  uint uVar1;
  CBaseEdict *pCVar2;
  int iVar3;
  IChangeInfoAccessor *pIVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CPropIndicatorPanel::Spawn(&this->super_CPropIndicatorPanel);
                    /* Unresolved local var: bool bServerSide@[???] */
  (this->super_CGameEventListener).m_bRegisteredForEvents = true;
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4f30ac) + 0x10))
            ((int *)**(undefined4 **)(unaff_EBX + 0x4f30ac),&this->super_CGameEventListener,
             unaff_EBX + 0x3533f6,1);
  uVar1 = (this->super_CPropIndicatorPanel).m_hIndicatorPanel.super_CBaseHandle.m_Index;
  if ((uVar1 == 0xffffffff) ||
     (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4f3090),
     *(uint *)(iVar3 + 8) != uVar1 >> 0x10)) {
    iVar3 = 0;
  }
  else {
    iVar3 = *(int *)(iVar3 + 4);
  }
  if (*(int *)(iVar3 + 0x3c8) != 0) {
    if (*(char *)(iVar3 + 0x60) == '\0') {
      pCVar2 = *(CBaseEdict **)(iVar3 + 0x24);
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar4 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar4->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      *(uint *)(iVar3 + 100) = *(uint *)(iVar3 + 100) | 1;
    }
    *(undefined4 *)(iVar3 + 0x3c8) = 0;
  }
  if (*(char *)(iVar3 + 0x3c3) != '\0') {
    if (*(char *)(iVar3 + 0x60) == '\0') {
      pCVar2 = *(CBaseEdict **)(iVar3 + 0x24);
      if (pCVar2 != (CBaseEdict *)0x0) {
        pCVar2->m_fStateFlags = pCVar2->m_fStateFlags | 0x101;
        pIVar4 = CBaseEdict::GetChangeAccessor(pCVar2);
        pIVar4->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      *(uint *)(iVar3 + 100) = *(uint *)(iVar3 + 100) | 1;
    }
    *(undefined1 *)(iVar3 + 0x3c3) = 0;
  }
  (this->super_CGameEventListener).field_0x9 = 0;
  return;
}


/* CPropTicTacToePanel::FireGameEvent at 006b80c0 */

/* WARNING: Struct "CPropTicTacToePanel": ignoring overlapping field "m_bPlayed" */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void FireGameEvent(CPropTicTacToePanel * this, IGameEvent * event) */

void __thiscall CPropTicTacToePanel::FireGameEvent(CPropTicTacToePanel *this,IGameEvent *event)

{
  int *piVar1;
  uint uVar2;
  CBaseEdict *pCVar3;
  char *s1;
  int iVar4;
  CBasePlayer *pCVar5;
  IChangeInfoAccessor *pIVar6;
  CLabIndicatorPanel *this_00;
  int unaff_EBX;
  longdouble extraout_ST0;
  longdouble extraout_ST0_00;
  longdouble extraout_ST0_01;
  float fVar7;
  int local_50;
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
    iVar4 = _V_strcmp(s1,&UNK_00353e3c + unaff_EBX);
    if (iVar4 == 0) {
      local_50 = (*event->_vptr_IGameEvent[7])(event,unaff_EBX + 0x2fe0be,0);
      (*event->_vptr_IGameEvent[9])(event,unaff_EBX + 0x3528e3,0);
      (*event->_vptr_IGameEvent[9])(event,unaff_EBX + 0x3528ea,0);
      (*event->_vptr_IGameEvent[9])(event,unaff_EBX + 0x3528f1,0);
      iVar4 = 1;
      do {
                    /* Unresolved local var: CBasePlayer * pPlayer@[???] */
        pCVar5 = UTIL_PlayerByIndex(iVar4);
        if (pCVar5 != (CBasePlayer *)0x0) {
          iVar4 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4f3aea) + 0x40))
                            ((int *)**(undefined4 **)(unaff_EBX + 0x4f3aea),
                             (pCVar5->super_CBaseCombatCharacter).super_CBaseFlex.
                             super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                             m_Network.m_pPev);
          local_50 = (local_50 != iVar4) + 1;
          break;
        }
                    /* Unresolved local var: int i@[???] */
        iVar4 = iVar4 + 1;
      } while (iVar4 != 0x22);
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
      fVar7 = local_28[0].x -
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
      local_28[0].y = fVar7;
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
      fVar7 = CalcSqrDistanceToAABB(local_28,&local_34,&local_4c);
      if ((fVar7 < *(float *)(unaff_EBX + 0x372802)) &&
         (!NAN(fVar7) && !NAN(*(float *)(unaff_EBX + 0x372802)))) {
        piVar1 = *(int **)(unaff_EBX + 0x4f3ad6);
        uVar2 = (this->super_CPropIndicatorPanel).m_hIndicatorPanel.super_CBaseHandle.m_Index;
        if ((uVar2 == 0xffffffff) ||
           (iVar4 = (uVar2 & 0xffff) * 0x10 + *piVar1, *(uint *)(iVar4 + 8) != uVar2 >> 0x10)) {
          iVar4 = 0;
        }
        else {
          iVar4 = *(int *)(iVar4 + 4);
        }
        if (local_50 != *(int *)(iVar4 + 0x3c8)) {
          if (*(char *)(iVar4 + 0x60) == '\0') {
            pCVar3 = *(CBaseEdict **)(iVar4 + 0x24);
            if (pCVar3 != (CBaseEdict *)0x0) {
              pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
              pIVar6 = CBaseEdict::GetChangeAccessor(pCVar3);
              pIVar6->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            *(uint *)(iVar4 + 100) = *(uint *)(iVar4 + 100) | 1;
          }
          *(int *)(iVar4 + 0x3c8) = local_50;
        }
        if (*(char *)(iVar4 + 0x3c3) != '\0') {
          if (*(char *)(iVar4 + 0x60) == '\0') {
            pCVar3 = *(CBaseEdict **)(iVar4 + 0x24);
            if (pCVar3 != (CBaseEdict *)0x0) {
              pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x101;
              pIVar6 = CBaseEdict::GetChangeAccessor(pCVar3);
              pIVar6->m_iChangeInfoSerialNumber = 0;
            }
          }
          else {
            *(uint *)(iVar4 + 100) = *(uint *)(iVar4 + 100) | 1;
          }
          *(undefined1 *)(iVar4 + 0x3c3) = 0;
        }
        uVar2 = (this->super_CPropIndicatorPanel).m_hIndicatorPanel.super_CBaseHandle.m_Index;
        if ((uVar2 == 0xffffffff) ||
           (iVar4 = (uVar2 & 0xffff) * 0x10 + *piVar1, *(uint *)(iVar4 + 8) != uVar2 >> 0x10)) {
          this_00 = (CLabIndicatorPanel *)0x0;
        }
        else {
          this_00 = *(CLabIndicatorPanel **)(iVar4 + 4);
        }
        CLabIndicatorPanel::ScreenVisible(this_00,true);
        if (local_50 != 1) {
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


/* _GLOBAL__I__ZN18CLabIndicatorPanel9m_DataMapE at 000bf6f0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN18CLabIndicatorPanel9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

