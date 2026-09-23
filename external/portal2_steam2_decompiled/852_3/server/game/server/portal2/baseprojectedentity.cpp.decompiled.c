/* DWARF-guided pseudocode for game/server/portal2/baseprojectedentity.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* DataMapInit<CBaseProjectedEntity> at 000bd010 */

datamap_t * DataMapInit<CBaseProjectedEntity>(CBaseProjectedEntity *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&sv_speed_normal.m_fnChangeCallbacks.m_pElements + unaff_EBX + 3) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf9517);
    if (iVar1 != 0) {
      *(undefined **)(&DAT_00cf951f + unaff_EBX) = &UNK_009515ad + unaff_EBX;
      *(undefined4 *)((int)&sv_speed_paint_max.super_ConCommandBase.m_pNext + unaff_EBX + 3) = 0;
      *(undefined4 *)(&sv_speed_paint_max.super_ConCommandBase.field_0xb + unaff_EBX) = 0;
      *(undefined4 *)((int)&sv_speed_paint_max.super_ConCommandBase.m_pszName + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&sv_speed_paint_max.super_ConCommandBase.m_pszHelpString + unaff_EBX + 3)
           = 0;
      *(undefined4 *)((int)&sv_speed_paint_max.super_ConCommandBase.m_nFlags + unaff_EBX + 3) = 0;
      *(undefined4 *)
       ((int)&sv_speed_paint_max.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX + 3) = 0x14;
      ___cxa_guard_release(unaff_EBX + 0xcf9517);
      ___cxa_atexit(unaff_EBX + 0x8dbb9f,0,*(undefined4 *)(&DAT_00aeeb6f + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_GetReceivedDamageScale_00c0bae8 + unaff_EBX + 3) =
       *(undefined4 *)(&DAT_00aeecab + unaff_EBX);
  *(undefined4 *)((int)&PTR_CanBeSeenBy_00c0bae0 + unaff_EBX + 3) = 10;
  *(int *)((int)&PTR_FVisible_00c0badc + unaff_EBX + 3) = unaff_EBX + 0xc0bb3f;
  return (datamap_t *)((int)&PTR_FVisible_00c0badc + unaff_EBX + 3);
}


/* __static_initialization_and_destruction_0 at 000bd0e0 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *s2;
  SendTable *this;
  datamap_t *pdVar1;
  int iVar2;
  int iVar3;
  int unaff_EBX;
  undefined4 *puVar4;
  undefined4 *puVar5;
  undefined4 *puVar6;
  longlong lVar7;
  
  lVar7 = ___i686_get_pc_thunk_bx();
  if (lVar7 == 0xffff00000001) {
    *(undefined1 *)
     ((int)&paint_max_impact_sounds.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) = 0;
    *(undefined1 *)
     ((int)&paint_max_impact_sounds.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&paint_max_impact_sounds.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 2) = 0;
    *(undefined1 *)
     ((int)&paint_max_impact_sounds.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 3) = 0;
    *(undefined4 *)((int)&paint_max_impact_sounds.m_fnChangeCallbacks.m_Size + unaff_EBX) = 0;
    *(undefined4 *)((int)&paint_max_impact_sounds.m_fnChangeCallbacks.m_pElements + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00cf8e38 + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00cf8e3c + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)
     ((int)&draw_paint_splat_particles.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) = 0;
    *(undefined4 *)((int)&draw_paint_splat_particles.super_ConCommandBase.m_pNext + unaff_EBX) = 0;
    *(undefined4 *)(&draw_paint_splat_particles.super_ConCommandBase.m_bRegistered + unaff_EBX) = 0;
    *(undefined4 *)((int)&draw_paint_splat_particles.super_ConCommandBase.m_pszName + unaff_EBX) = 0
    ;
    *(undefined4 *)
     ((int)&draw_paint_splat_particles.super_ConCommandBase.m_pszHelpString + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&draw_paint_splat_particles.super_ConCommandBase.m_nFlags + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&draw_paint_splat_particles.super_IConVar._vptr_IConVar + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&draw_paint_splat_particles.m_pParent + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&draw_paint_splat_particles.m_pszDefaultValue + unaff_EBX) = 0;
    *(undefined1 *)((int)&draw_paint_splat_particles.m_Value.m_pszString + unaff_EBX + 3) = 0;
    *(undefined1 *)((int)&draw_paint_splat_particles.m_Value.m_pszString + unaff_EBX) = 0;
    *(undefined1 *)((int)&draw_paint_splat_particles.m_Value.m_pszString + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&draw_paint_splat_particles.m_Value.m_pszString + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&draw_paint_splat_particles.m_Value.m_StringLength + unaff_EBX) = 0;
    *(undefined1 *)((int)&draw_paint_splat_particles.m_Value.m_fValue + unaff_EBX + 3) = 0;
    *(undefined1 *)((int)&draw_paint_splat_particles.m_Value.m_fValue + unaff_EBX) = 0;
    *(undefined1 *)((int)&draw_paint_splat_particles.m_Value.m_fValue + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&draw_paint_splat_particles.m_Value.m_fValue + unaff_EBX + 2) = 0;
    *(undefined4 *)((int)&draw_paint_splat_particles.m_Value.m_nValue + unaff_EBX) = 0;
    (&draw_paint_splat_particles.field_0x37)[unaff_EBX] = 1;
    (&draw_paint_splat_particles.m_bHasMin)[unaff_EBX] = false;
    (&draw_paint_splat_particles.field_0x35)[unaff_EBX] = 0;
    (&draw_paint_splat_particles.field_0x36)[unaff_EBX] = 0;
    *(undefined4 *)((int)&draw_paint_splat_particles.m_fMinVal + unaff_EBX) = 1;
    (&draw_paint_splat_particles.field_0x3f)[unaff_EBX] = 0;
    (&draw_paint_splat_particles.m_bHasMax)[unaff_EBX] = false;
    (&draw_paint_splat_particles.field_0x3d)[unaff_EBX] = 0;
    (&draw_paint_splat_particles.field_0x3e)[unaff_EBX] = 0;
    *(undefined4 *)((int)&draw_paint_splat_particles.m_fMaxVal + unaff_EBX) = 2;
    *(undefined1 *)
     ((int)&draw_paint_splat_particles.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 3) = 0;
    *(undefined1 *)
     ((int)&draw_paint_splat_particles.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) = 0;
    *(undefined1 *)
     ((int)&draw_paint_splat_particles.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&draw_paint_splat_particles.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX + 2) = 0;
    *(undefined **)
     ((int)&draw_paint_splat_particles.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX)
         = &UNK_00af4234 + unaff_EBX;
    pdVar1 = DataMapInit<CBaseProjectedEntity>((CBaseProjectedEntity *)0x0);
    *(datamap_t **)
     ((int)DataMapInit<CPoseController>::dataDesc[0x14].flatOffset + unaff_EBX + -0x24) = pdVar1;
    iVar3 = unaff_EBX + 0xcf8e8c;
    s2 = (char *)(unaff_EBX + 0x9514da);
    *(char **)((int)&draw_paint_splat_particles.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX
              ) = s2;
    this = (SendTable *)
           ((int)DataMapInit<CPoseController>::dataDesc[0x14].flatOffset + unaff_EBX + -0x20);
    *(SendTable **)((int)&draw_paint_splat_particles.m_fnChangeCallbacks.m_Size + unaff_EBX) = this;
    *(undefined4 *)(&DAT_00cf8e9c + unaff_EBX) = 0xffff;
    puVar6 = (undefined4 *)**(int **)(&DAT_00aeeb3c + unaff_EBX);
    if (puVar6 == (undefined4 *)0x0) {
      **(int **)(&DAT_00aeeb3c + unaff_EBX) = iVar3;
      *(undefined4 *)((int)&draw_paint_splat_particles.m_fnChangeCallbacks.m_pElements + unaff_EBX)
           = 0;
    }
    else {
      puVar5 = (undefined4 *)puVar6[2];
      iVar2 = _V_stricmp((char *)*puVar6,s2);
      if (iVar2 < 1) {
        if (puVar5 != (undefined4 *)0x0) {
          do {
            puVar4 = puVar5;
            iVar2 = _V_stricmp((char *)*puVar4,s2);
            puVar5 = puVar4;
            if (0 < iVar2) goto LAB_000bd320;
            puVar5 = (undefined4 *)puVar4[2];
            puVar6 = puVar4;
          } while ((undefined4 *)puVar4[2] != (undefined4 *)0x0);
          puVar5 = (undefined4 *)0x0;
        }
LAB_000bd320:
        *(undefined4 **)
         ((int)&draw_paint_splat_particles.m_fnChangeCallbacks.m_pElements + unaff_EBX) = puVar5;
        puVar6[2] = iVar3;
      }
      else {
        *(undefined4 *)
         ((int)&draw_paint_splat_particles.m_fnChangeCallbacks.m_pElements + unaff_EBX) =
             **(undefined4 **)(&DAT_00aeeb3c + unaff_EBX);
        **(int **)(&DAT_00aeeb3c + unaff_EBX) = iVar3;
      }
    }
    SendTable::SendTable(this);
    ___cxa_atexit(unaff_EBX + 0x8dbaac,0,*(undefined4 *)(&DAT_00aeea9c + unaff_EBX));
    iVar3 = ServerClassInit<DT_BaseProjectedEntity::ignored>((ignored *)0x0);
    *(int *)((int)DataMapInit<CPoseController>::dataDesc[0x14].flatOffset + unaff_EBX + -0xc) =
         iVar3;
  }
  return;
}


/* CBaseProjectedEntity::GetDataDescMap at 006ae7d0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CBaseProjectedEntity * this) */

datamap_t * __thiscall CBaseProjectedEntity::GetDataDescMap(CBaseProjectedEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x61a328);
}


/* CBaseProjectedEntity::GetBaseMap at 006ae7e0 */

datamap_t * CBaseProjectedEntity::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(&DAT_004fd4e4 + extraout_ECX);
}


/* __tcf_0 at 00998bc0 */

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
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x212fca) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x212fca),*(int *)(unaff_EBX + 0x41d97a));
      *(undefined4 *)(unaff_EBX + 0x41d97a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d97e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41d97a);
  *(int *)(unaff_EBX + 0x41d98a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41d982)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x212fca) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x212fca),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41d97a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41d97e) = 0;
  }
  return;
}


/* ServerClassInit<DT_BaseProjectedEntity::ignored> at 000bc9a0 */

int ServerClassInit<DT_BaseProjectedEntity::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&speed_funnelling_enabled.super_ConCommandBase.field_0xa)[unaff_EBX] == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf95ea);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  ((int)&speed_funnelling_enabled.super_ConCommandBase.m_pszHelpString +
                  unaff_EBX + 2),(char *)(unaff_EBX + 0x8f6076),0,4,-1,0,
                  (SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&sv_paintairacceleration.super_ConCommandBase.m_pNext + unaff_EBX + 2)
                        ,(char *)(unaff_EBX + 0x8f83e6),0,
                        (SendTable *)**(undefined4 **)(&DAT_00aef2da + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00aef27e + unaff_EBX),0x80);
      SendPropExclude((SendProp_conflict *)(unaff_EBX + 0xcf969a),(char *)(unaff_EBX + 0x94e202),
                      (char *)(unaff_EBX + 0x945282));
      SendPropExclude((SendProp_conflict *)
                      ((int)&eggbot_sink_speed.m_fnChangeCallbacks.m_Memory.m_nGrowSize +
                      unaff_EBX + 2),(char *)(unaff_EBX + 0x94e202),(char *)(unaff_EBX + 0x94ec46));
      SendPropVector((SendProp_conflict *)((int)&ballbot_sink_speed.m_fMaxVal + unaff_EBX + 2),
                     (char *)(unaff_EBX + 0x945282),0x350,0xc,-1,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00aef9de + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)(&coop_sink_speed_decay.field_0x36 + unaff_EBX),
                     (char *)(unaff_EBX + 0x94ec46),0x35c,0xc,-1,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00aefb22 + unaff_EBX),0x80);
      SendPropEHandle((SendProp_conflict *)
                      ((int)&coop_impact_velocity_threshold.m_Value.m_StringLength + unaff_EBX + 2),
                      &UNK_00951b51 + unaff_EBX,0x3c0,4,-1,
                      *(SendVarProxyFn_conflict *)(&DAT_00aef26a + unaff_EBX));
      SendPropEHandle((SendProp_conflict *)
                      ((int)&sv_portal_new_player_trace.m_pParent + unaff_EBX + 2),
                      &UNK_00951b5e + unaff_EBX,0x3c4,4,-1,
                      *(SendVarProxyFn_conflict *)(&DAT_00aef26a + unaff_EBX));
      SendPropVector((SendProp_conflict *)
                     ((int)&sv_portal_new_player_trace_vs_remote_ents.super_ConCommandBase.
                            m_pszHelpString + unaff_EBX + 2),&UNK_00951b6e + unaff_EBX,0x3c8,0xc,-1,
                     4,0.0,-121121.125,*(SendVarProxyFn_conflict *)(&DAT_00aef2ca + unaff_EBX),0x80)
      ;
      SendPropVector((SendProp_conflict *)
                     ((int)&sv_portal_new_trace_debugboxes.super_ConCommandBase.m_pNext +
                     unaff_EBX + 2),&UNK_00951b86 + unaff_EBX,0x3d4,0xc,-1,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00aef2ca + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)(unaff_EBX + 0xcf993a),&UNK_00951ba4 + unaff_EBX,0x3e0,0xc
                     ,0,4,0.0,-121121.125,*(SendVarProxyFn_conflict *)(&DAT_00aef2ca + unaff_EBX),
                     0x80);
      SendPropVector((SendProp_conflict *)
                     ((int)&sv_edgefriction.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX + 2
                     ),&UNK_00951bbb + unaff_EBX,0x3ec,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00aef2ca + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)((int)&sv_use_edgefriction.m_fMaxVal + unaff_EBX + 2),
                     &UNK_00951bd8 + unaff_EBX,0x3f8,0xc,-1,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00aef2ca + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)(unaff_EBX + 0xcf9a36),&UNK_00951be8 + unaff_EBX,0x404,0xc
                     ,-1,4,0.0,-121121.125,*(SendVarProxyFn_conflict *)(&DAT_00aef2ca + unaff_EBX),
                     0x80);
      SendPropEHandle((SendProp_conflict *)
                      ((int)&sv_hl2mp_weapon_respawn_time.m_fnChangeCallbacks.m_Memory.
                             m_nAllocationCount + unaff_EBX + 2),&UNK_00951bf6 + unaff_EBX,0x410,4,
                      -1,*(SendVarProxyFn_conflict *)(&DAT_00aef26a + unaff_EBX));
      SendPropEHandle((SendProp_conflict *)(&sv_hl2mp_item_respawn_time.field_0x3e + unaff_EBX),
                      (char *)(unaff_EBX + 0x94ddd6),0x414,4,-1,
                      *(SendVarProxyFn_conflict *)(&DAT_00aef26a + unaff_EBX));
      SendPropInt((SendProp_conflict *)
                  ((int)&sv_report_client_settings.m_Value.m_nValue + unaff_EBX + 2),
                  &UNK_00951c06 + unaff_EBX,0x418,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      ___cxa_guard_release(unaff_EBX + 0xcf95ea);
      ___cxa_atexit(unaff_EBX + 0x8dc0c2,0,*(undefined4 *)(&DAT_00aef1e2 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)
             ((int)DataMapInit<CGrabController>::dataDesc[2].flatOffset + unaff_EBX + 6),
             (SendProp_conflict *)
             ((int)&sv_paintairacceleration.super_ConCommandBase.m_pNext + unaff_EBX + 2),0x10,
             *(char **)(&DAT_00b728b6 + unaff_EBX));
  return 1;
}


/* CBaseProjectedEntity::GetServerClass at 006ae7f0 */

/* DWARF original prototype: ServerClass * GetServerClass(CBaseProjectedEntity * this) */

ServerClass * __thiscall CBaseProjectedEntity::GetServerClass(CBaseProjectedEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x707788);
}


/* CBaseProjectedEntity::YouForgotToImplementOrDeclareServerClass at 006ae800 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CBaseProjectedEntity *
   this) */

int __thiscall
CBaseProjectedEntity::YouForgotToImplementOrDeclareServerClass(CBaseProjectedEntity *this)

{
  return 0;
}


/* __tcf_2 at 00998ba0 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x3959a0),in_stack_00000008);
  return;
}


/* __tcf_1 at 00998a70 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41da63))(unaff_EBX + 0x41da63);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41da0f))(unaff_EBX + 0x41da0f);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d9bb))(unaff_EBX + 0x41d9bb);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d967))(unaff_EBX + 0x41d967);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d913))(unaff_EBX + 0x41d913);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d8bf))(unaff_EBX + 0x41d8bf);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d86b))(unaff_EBX + 0x41d86b);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d817))(unaff_EBX + 0x41d817);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d7c3))(unaff_EBX + 0x41d7c3);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d76f))(unaff_EBX + 0x41d76f);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d71b))(unaff_EBX + 0x41d71b);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d6c7))(unaff_EBX + 0x41d6c7);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d673))(unaff_EBX + 0x41d673);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d61f))(unaff_EBX + 0x41d61f);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d5cb))(unaff_EBX + 0x41d5cb);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d577))(unaff_EBX + 0x41d577);
                    /* WARNING: Could not recover jumptable at 0x00998b9e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41d523))();
  return;
}


/* CBaseProjectedEntity::CBaseProjectedEntity at 006ae840 */

/* DWARF original prototype: void CBaseProjectedEntity(CBaseProjectedEntity * this) */

void __thiscall CBaseProjectedEntity::CBaseProjectedEntity(CBaseProjectedEntity *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::CBaseEntity(&this->super_CBaseEntity,false);
  (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x5806b7);
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


/* CBaseProjectedEntity::CBaseProjectedEntity at 006aec70 */

/* DWARF original prototype: void CBaseProjectedEntity(CBaseProjectedEntity * this,
   CBaseProjectedEntity * this) */

void __thiscall
CBaseProjectedEntity::CBaseProjectedEntity(CBaseProjectedEntity *this,CBaseProjectedEntity *this_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::CBaseEntity(&this->super_CBaseEntity,false);
  (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x580287);
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


/* CBaseProjectedEntity::Spawn at 006ae810 */

/* DWARF original prototype: void Spawn(CBaseProjectedEntity * this) */

void __thiscall CBaseProjectedEntity::Spawn(CBaseProjectedEntity *this)

{
  CBaseEntity::Spawn(&this->super_CBaseEntity);
                    /* WARNING: Could not recover jumptable at 0x006ae832. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0xc5])();
  return;
}


/* CBaseProjectedEntity::OnRestore at 006ae9c0 */

/* DWARF original prototype: void OnRestore(CBaseProjectedEntity * this) */

void __thiscall CBaseProjectedEntity::OnRestore(CBaseProjectedEntity *this)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  int iVar2;
  CBaseEntity *pCVar3;
  CBaseEntity *pCVar4;
  uint *puVar5;
  undefined4 uVar6;
  int iVar7;
  IChangeInfoAccessor *pIVar8;
  uint uVar9;
  uint uVar10;
  int unaff_EBX;
  _func_int_varargs *p_Var11;
  bool bVar12;
  
                    /* Unresolved local var: bool bCreatePlacement@[???] */
  ___i686_get_pc_thunk_bx();
  CBaseEntity::OnRestore(&this->super_CBaseEntity);
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
  iVar7 = **(int **)(unaff_EBX + 0x4fd1d6);
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
  uVar10 = (this->m_hPlacementHelper).
           super_CNetworkVarBase<CBaseHandle,CBaseProjectedEntity::NetworkVar_m_hPlacementHelper>.
           m_Value.m_Index;
  if (uVar10 == 0xffffffff) {
LAB_006aea05:
    bVar12 = true;
LAB_006aea09:
    pCVar3 = CreateEntityByName((char *)(unaff_EBX + 0x35fe18),-1,true);
    uVar10 = (this->m_hPlacementHelper).
             super_CNetworkVarBase<CBaseHandle,CBaseProjectedEntity::NetworkVar_m_hPlacementHelper>.
             m_Value.m_Index;
    if ((uVar10 == 0xffffffff) ||
       (iVar7 = (uVar10 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4fd1d6),
       *(uint *)(iVar7 + 8) != uVar10 >> 0x10)) {
      pCVar4 = (CBaseEntity *)0x0;
    }
    else {
      pCVar4 = *(CBaseEntity **)(iVar7 + 4);
    }
    if (pCVar4 != pCVar3) {
      if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
        this_00 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
        if (this_00 != (CBaseEdict *)0x0) {
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
          pIVar8 = CBaseEdict::GetChangeAccessor(this_00);
          pIVar8->m_iChangeInfoSerialNumber = 0;
        }
      }
      else {
        puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      if (pCVar3 == (CBaseEntity *)0x0) {
        (this->m_hPlacementHelper).
        super_CNetworkVarBase<CBaseHandle,CBaseProjectedEntity::NetworkVar_m_hPlacementHelper>.
        m_Value.m_Index = 0xffffffff;
        p_Var11 = (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.
                  super_IHandleEntity._vptr_IHandleEntity[0xd2];
        uVar6 = 0;
        goto LAB_006aeabd;
      }
      puVar5 = (uint *)(*(pCVar3->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                         _vptr_IHandleEntity[3])(pCVar3);
      uVar10 = *puVar5;
      (this->m_hPlacementHelper).
      super_CNetworkVarBase<CBaseHandle,CBaseProjectedEntity::NetworkVar_m_hPlacementHelper>.m_Value
      .m_Index = uVar10;
    }
    p_Var11 = (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity
              ._vptr_IHandleEntity[0xd2];
    iVar7 = **(int **)(unaff_EBX + 0x4fd1d6);
    if ((uVar10 == 0xffffffff) ||
       (uVar9 = uVar10 & 0xffff, uVar10 >> 0x10 != *(uint *)(iVar7 + 8 + uVar9 * 0x10))) {
      uVar6 = 0;
      goto LAB_006aeabd;
    }
  }
  else {
    uVar9 = uVar10 & 0xffff;
    iVar2 = uVar9 * 0x10 + iVar7;
    if (*(uint *)(iVar2 + 8) != uVar10 >> 0x10) goto LAB_006aea05;
    bVar12 = *(int *)(iVar2 + 4) == 0;
    if (bVar12) goto LAB_006aea09;
    p_Var11 = (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.super_IHandleEntity
              ._vptr_IHandleEntity[0xd2];
  }
  uVar6 = *(undefined4 *)(iVar7 + 4 + uVar9 * 0x10);
LAB_006aeabd:
  (*p_Var11)(this,uVar6);
  if (bVar12) {
    uVar10 = (this->m_hPlacementHelper).
             super_CNetworkVarBase<CBaseHandle,CBaseProjectedEntity::NetworkVar_m_hPlacementHelper>.
             m_Value.m_Index;
    if ((uVar10 == 0xffffffff) ||
       (iVar7 = (uVar10 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4fd1d6),
       *(uint *)(iVar7 + 8) != uVar10 >> 0x10)) {
      pCVar3 = (CBaseEntity *)0x0;
    }
    else {
      pCVar3 = *(CBaseEntity **)(iVar7 + 4);
    }
    DispatchSpawn(pCVar3,true);
  }
  return;
}


/* CBaseProjectedEntity::UpdateOnRemove at 006aebc0 */

/* DWARF original prototype: void UpdateOnRemove(CBaseProjectedEntity * this) */

void __thiscall CBaseProjectedEntity::UpdateOnRemove(CBaseProjectedEntity *this)

{
  int *piVar1;
  uint uVar2;
  CBaseEntity *pCVar3;
  int iVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  piVar1 = *(int **)(unaff_EBX + 0x4fcfd0);
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


/* CBaseProjectedEntity::PlacePlacementHelper at 006ae8b0 */

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
              (pHelper,&UNK_003284d6 + unaff_EBX,unaff_EBX + 0x36296a);
    (*(pHelper->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
      super_IHandleEntity._vptr_IHandleEntity[0x20])
              (pHelper,unaff_EBX + 0x36296e,&UNK_0030517e + unaff_EBX);
    (*(pHelper->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
      super_IHandleEntity._vptr_IHandleEntity[0x13])(pHelper,this);
  }
  return;
}


/* _GLOBAL__I__ZN20CBaseProjectedEntity9m_DataMapE at 000bd340 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN20CBaseProjectedEntity9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

