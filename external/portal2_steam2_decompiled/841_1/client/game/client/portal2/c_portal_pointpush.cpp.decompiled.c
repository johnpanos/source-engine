/* DWARF-guided pseudocode for game/client/portal2/c_portal_pointpush.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* ClientClassInit<DT_PortalPointPush::ignored> at 000779d0 */

int ClientClassInit<DT_PortalPointPush::ignored>(ignored *param_1)

{
  RecvVarProxyFn varProxy;
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&BaseModUI::CPlayerItem::GetMessageMap()::s_pMap + unaff_EBX + 2) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc5a95a);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&BaseModUI::CPlayerItem::GetKBMap()::s_pMap + unaff_EBX + 2),
                  (char *)(unaff_EBX + 0x912d92),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        (&BaseModUI::CPlayersList::KB_ChainToMap()::chained + unaff_EBX),
                        &UNK_009178d6 + unaff_EBX,0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00aec452 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00aec40e + unaff_EBX));
      RecvPropBool((RecvProp *)(&BaseModUI::GameOptions::KB_ChainToMap()::chained + unaff_EBX),
                   (char *)(unaff_EBX + 0x92b3f6),0xa5d,1);
      varProxy = *(RecvVarProxyFn *)(&DAT_00aec44e + unaff_EBX);
      RecvPropFloat((RecvProp_conflict1 *)(unaff_EBX + 0xc5aa16),(char *)(unaff_EBX + 0x913121),
                    0xa60,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&BaseModUI::GetLegacyData::KB_ChainToMap()::chained + unaff_EBX),
                    (char *)(unaff_EBX + 0x91319d),0xa64,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)(unaff_EBX + 0xc5aa8e),(char *)(unaff_EBX + 0x9433e6),
                    0xa68,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&BaseModUI::InGameKickPlayerList::KB_ChainToMap()::chained + unaff_EBX),
                    (char *)(unaff_EBX + 0x9433f6),0xa6c,4,0,varProxy);
      ___cxa_guard_release(unaff_EBX + 0xc5a95a);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)&vec4_origin.z + unaff_EBX + 2),
             (RecvProp_conflict1 *)(&BaseModUI::CPlayersList::KB_ChainToMap()::chained + unaff_EBX),
             6,(char *)(unaff_EBX + 0x94340a));
  return 1;
}


/* __static_initialization_and_destruction_0 at 00077c10 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  RecvTable *this;
  int *piVar1;
  int iVar2;
  int unaff_EBX;
  longlong lVar3;
  
  lVar3 = ___i686_get_pc_thunk_bx();
  if (lVar3 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)&GameModes::GetMessageMap::s_pMap + unaff_EBX + 3) = 0;
  (&GameModes::ChainToMap()::chained)[unaff_EBX] = 0;
  (&GameModes::ChainToAnimationMap()::chained)[unaff_EBX] = 0;
  *(undefined1 *)((int)&GameModes::KB_ChainToMap()::chained + unaff_EBX) = 0;
  *(undefined4 *)((int)&GameModes::KB_ChainToMap()::chained + unaff_EBX + 1) = 0;
  *(undefined4 *)(&DAT_00c5a6e7 + unaff_EBX) = 0;
  *(undefined4 *)
   ((int)&CUtlRBTree<CUtlMap<unsigned_long_long,CGameUiAvatarImage*,unsigned_short>::Node_t,unsigned_short,CUtlMap<unsigned_long_long,CGameUiAvatarImage*,unsigned_short>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<unsigned_long_long,CGameUiAvatarImage*,unsigned_short>::Node_t,unsigned_short>,unsigned_short>>
          ::Links(unsigned_short)::s_Sentinel + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c5a6ef + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&CUtlRBTree<CUtlMap<unsigned_long_long,CUtlString,unsigned_short>::Node_t,unsigned_short,CUtlMap<unsigned_long_long,CUtlString,unsigned_short>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<unsigned_long_long,CUtlString,unsigned_short>::Node_t,unsigned_short>,unsigned_short>>
          ::Links(unsigned_short)::s_Sentinel + unaff_EBX + 3) = 0;
  *(undefined4 *)(&DAT_00c5a6f7 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c5a6fb + unaff_EBX) = 0;
  *(undefined4 *)
   ((int)&CUtlRBTree<CUtlMap<unsigned_long_long,CUtlString,unsigned_short>::Node_t,unsigned_short,CUtlMap<unsigned_long_long,CUtlString,unsigned_short>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<unsigned_long_long,CUtlString,unsigned_short>::Node_t,unsigned_short>,unsigned_short>>
          ::Links(unsigned_short)::s_Sentinel + unaff_EBX + 3) = 0;
  *(undefined4 *)(&DAT_00c5a703 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&CUtlRBTree<CUtlMap<unsigned_long_long,CGameUiAvatarImage*,unsigned_short>::Node_t,unsigned_short,CUtlMap<unsigned_long_long,CGameUiAvatarImage*,unsigned_short>::CKeyLess,CUtlMemory<UtlRBTreeNode_t<CUtlMap<unsigned_long_long,CGameUiAvatarImage*,unsigned_short>::Node_t,unsigned_short>,unsigned_short>>
          ::Links(unsigned_short)::s_Sentinel + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c5a70b + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&BaseModUI::CUIGameData::m_Instance + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined **)((int)&vgui::Divider::GetMessageMap()::s_pMap + unaff_EBX + 3) =
       &UNK_00aed267 + unaff_EBX;
  *(int *)(&DAT_00cf151b + unaff_EBX) = unaff_EBX + 0x9431da;
  *(int *)((int)&r_modelwireframedecal.m_fnChangeCallbacks.m_Size + unaff_EBX + 3) =
       unaff_EBX + 0x4b5d4f;
  *(undefined4 *)((int)&r_modelwireframedecal.m_fnChangeCallbacks.m_pElements + unaff_EBX + 3) = 0;
  this = (RecvTable *)(&mat_normals.super_ConCommandBase.field_0xb + unaff_EBX);
  *(RecvTable **)(&DAT_00cf151f + unaff_EBX) = this;
  piVar1 = *(int **)(&DAT_00aec1c3 + unaff_EBX);
  *(int *)((int)&mat_normals.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX + 3) = *piVar1;
  *piVar1 = unaff_EBX + 0xcf1513;
  RecvTable::RecvTable(this);
  ___cxa_atexit(unaff_EBX + 0x90431f,0,*(undefined4 *)(&DAT_00aec14b + unaff_EBX));
  iVar2 = ClientClassInit<DT_PortalPointPush::ignored>((ignored *)0x0);
  *(int *)((int)&mat_normals.m_pParent + unaff_EBX + 3) = iVar2;
  return;
}


/* C_Portal_PointPush::YouForgotToImplementOrDeclareClientClass at 0052d7a0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_Portal_PointPush": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_Portal_PointPush * this)
    */

int __thiscall
C_Portal_PointPush::YouForgotToImplementOrDeclareClientClass(C_Portal_PointPush *this)

{
  return 0;
}


/* C_Portal_PointPush::GetClientClass at 0052d7c0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_Portal_PointPush": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: ClientClass * GetClientClass(C_Portal_PointPush * this) */

ClientClass * __thiscall C_Portal_PointPush::GetClientClass(C_Portal_PointPush *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x83b96c);
}


/* _C_Portal_PointPush_CreateObject at 0052d970 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_Portal_PointPush_CreateObject(int entnum,int serialNum)

{
  C_BaseEntity *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_Portal_PointPush * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xa70);
  C_BaseEntity::C_BaseEntity(this);
  (this->super_IClientEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x69e3c4);
  (this->super_IClientEntity).super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(CViewRenderBeams::FreeBeam + unaff_EBX);
  (this->super_IClientEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x69e818);
  (this->super_IClientEntity).super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x69e854);
  (this->super_IClientModelRenderable)._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x69e870);
  (**(code **)(unaff_EBX + 0x69e480))(this,entnum,serialNum);
  return &(this->super_IClientEntity).super_IClientNetworkable;
}


/* __tcf_0 at 0097bf40 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3ed204),in_stack_00000008);
  return;
}


/* C_Portal_PointPush::C_Portal_PointPush at 0052d890 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_Portal_PointPush": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: void C_Portal_PointPush(C_Portal_PointPush * this) */

void __thiscall C_Portal_PointPush::C_Portal_PointPush(C_Portal_PointPush *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::C_BaseEntity(&this->super_C_BaseEntity);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x69e4a7);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x69e843);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(&UNK_0069e8fb + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x69e937);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x69e953);
  return;
}


/* C_Portal_PointPush::C_Portal_PointPush at 0052d900 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_Portal_PointPush": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: void C_Portal_PointPush(C_Portal_PointPush * this) */

void __thiscall C_Portal_PointPush::C_Portal_PointPush(C_Portal_PointPush *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::C_BaseEntity(&this->super_C_BaseEntity);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x69e437);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x69e7d3);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(&UNK_0069e88b + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(GetViewEffects + unaff_EBX + 7);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x69e8e3);
  return;
}


/* C_Portal_PointPush::Spawn at 0052d880 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_Portal_PointPush": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: void Spawn(C_Portal_PointPush * this) */

void __thiscall C_Portal_PointPush::Spawn(C_Portal_PointPush *this)

{
  C_BaseEntity::Spawn(&this->super_C_BaseEntity);
  return;
}


/* C_Portal_PointPush::OnDataChanged at 0052d800 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_Portal_PointPush": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: void OnDataChanged(C_Portal_PointPush * this, DataUpdateType_t type) */

void __thiscall C_Portal_PointPush::OnDataChanged(C_Portal_PointPush *this,DataUpdateType_t type)

{
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::OnDataChanged(&this->super_C_BaseEntity,type);
  if (type != DATA_UPDATE_CREATED) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x0052d871. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x9d])();
  return;
}


/* C_Portal_PointPush::ClientThink at 0052d7e0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_Portal_PointPush": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: void ClientThink(C_Portal_PointPush * this) */

void __thiscall C_Portal_PointPush::ClientThink(C_Portal_PointPush *this)

{
  PushThink(this);
  return;
}


/* _GLOBAL__I__ZN18C_Portal_PointPush17m_pClassRecvTableE at 00077d30 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN18C_Portal_PointPush17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

