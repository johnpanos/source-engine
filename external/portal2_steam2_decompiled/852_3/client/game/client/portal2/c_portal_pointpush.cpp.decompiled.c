/* DWARF-guided pseudocode for game/client/portal2/c_portal_pointpush.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* ClientClassInit<DT_PortalPointPush::ignored> at 000773b0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

int ClientClassInit<DT_PortalPointPush::ignored>(ignored *param_1)

{
  RecvVarProxyFn varProxy;
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x395].m_SerialNumber +
               unaff_EBX + 2) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc41aba);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x395].m_pNext +
                  unaff_EBX + 2),(char *)(unaff_EBX + 0x8e89a2),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x399].m_pPrev +
                        unaff_EBX + 2),(char *)(unaff_EBX + 0x8ed582),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00ab9a26 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00ab99e2 + unaff_EBX));
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x39d].m_SerialNumber +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0x901046),0xa59,1);
      varProxy = *(RecvVarProxyFn *)(&DAT_00ab9a22 + unaff_EBX);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3a1].m_pEntity +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x8e8d31),0xa5c,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)
                    (&UNK_00003a4e +
                    (int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX)
                    ,(char *)(unaff_EBX + 0x8e8dad),0xa60,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3a8].m_pPrev +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x918bfa),0xa64,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3ac].m_SerialNumber +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x918c0a),0xa68,4,0,varProxy);
      ___cxa_guard_release(unaff_EBX + 0xc41aba);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)&s_C_INIT_CreationNoiseFactory.m_Id + unaff_EBX + 2),
             (RecvProp_conflict1 *)
             ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x399].m_pPrev + unaff_EBX + 2)
             ,6,(char *)(unaff_EBX + 0x918c1e));
  return 1;
}


/* __static_initialization_and_destruction_0 at 000775f0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

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
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x36d].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x36d].m_pNext + unaff_EBX)
       = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x36d].m_pNext + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x36d].m_pNext + unaff_EBX + 2) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x36d].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x36e].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x36e].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x36e].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x36e].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x36f].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   (&UNK_000036fb + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_000036ff + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_000036ff + (int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX)
       = 0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x370].m_pEntity + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x370].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x370].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined **)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x370].m_pNext + unaff_EBX + 3) =
       &UNK_00aba807 + unaff_EBX;
  *(int *)((int)&s_OrientationRelativeTraceVectors[2].x + unaff_EBX + 3) = unaff_EBX + 0x9189ee;
  *(int *)((int)&s_OrientationRelativeTraceVectors[1].y + unaff_EBX + 3) = unaff_EBX + 0x4b543f;
  *(undefined4 *)((int)&s_OrientationRelativeTraceVectors[1].z + unaff_EBX + 3) = 0;
  this = (RecvTable *)((int)&s_OrientationRelativeTraceVectors[3].y + unaff_EBX + 3);
  *(RecvTable **)((int)&s_OrientationRelativeTraceVectors[2].y + unaff_EBX + 3) = this;
  piVar1 = *(int **)(&DAT_00ab9797 + unaff_EBX);
  *(int *)((int)&s_OrientationRelativeTraceVectors[2].z + unaff_EBX + 3) = *piVar1;
  *piVar1 = unaff_EBX + 0xcd84d3;
  RecvTable::RecvTable(this);
  ___cxa_atexit(unaff_EBX + 0x8da31f,0,*(undefined4 *)(&DAT_00ab971f + unaff_EBX));
  iVar2 = ClientClassInit<DT_PortalPointPush::ignored>((ignored *)0x0);
  *(int *)(&s_C_OP_WorldCollideConstraintFactory.field_0xf + unaff_EBX) = iVar2;
  return;
}


/* C_Portal_PointPush::YouForgotToImplementOrDeclareClientClass at 0052c890 */

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


/* C_Portal_PointPush::GetClientClass at 0052c8b0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_Portal_PointPush": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: ClientClass * GetClientClass(C_Portal_PointPush * this) */

ClientClass * __thiscall C_Portal_PointPush::GetClientClass(C_Portal_PointPush *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x82321c);
}


/* _C_Portal_PointPush_CreateObject at 0052ca40 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_Portal_PointPush_CreateObject(int entnum,int serialNum)

{
  C_BaseEntity *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_Portal_PointPush * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xa6c);
  C_BaseEntity::C_BaseEntity(this);
  (this->super_IClientEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x66af14);
  (this->super_IClientEntity).super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x66b2ac);
  (this->super_IClientEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x66b364);
  (this->super_IClientEntity).super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x66b3a0);
  (**(code **)(unaff_EBX + 0x66afd0))(this,entnum,serialNum);
  return &(this->super_IClientEntity).super_IClientNetworkable;
}


/* __tcf_0 at 00951920 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3fe1c4),in_stack_00000008);
  return;
}


/* C_Portal_PointPush::C_Portal_PointPush at 0052c980 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x66afd7);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x66b36f);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x66b427);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x66b463);
  return;
}


/* C_Portal_PointPush::C_Portal_PointPush at 0052c9e0 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x66af77);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x66b30f);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x66b3c7);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x66b403);
  return;
}


/* C_Portal_PointPush::Spawn at 0052c970 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_Portal_PointPush": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: void Spawn(C_Portal_PointPush * this) */

void __thiscall C_Portal_PointPush::Spawn(C_Portal_PointPush *this)

{
  C_BaseEntity::Spawn(&this->super_C_BaseEntity);
  return;
}


/* C_Portal_PointPush::OnDataChanged at 0052c8f0 */

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
                    /* WARNING: Could not recover jumptable at 0x0052c961. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x9b])();
  return;
}


/* C_Portal_PointPush::ClientThink at 0052c8d0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_Portal_PointPush": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: void ClientThink(C_Portal_PointPush * this) */

void __thiscall C_Portal_PointPush::ClientThink(C_Portal_PointPush *this)

{
  PushThink(this);
  return;
}


/* _GLOBAL__I__ZN18C_Portal_PointPush17m_pClassRecvTableE at 00077710 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN18C_Portal_PointPush17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

