/* DWARF-guided pseudocode for game/client/portal2/c_prop_linked_portal_door.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* _C_PropLinkedPortalDoor_CreateObject at 00536980 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_PropLinkedPortalDoor_CreateObject(int entnum,int serialNum)

{
  C_BaseAnimating *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_PropLinkedPortalDoor * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xd88);
  C_BaseAnimating::C_BaseAnimating(this);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x662bd4);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x663034);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6630ec);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x663128);
  (this->super_IClientModelRenderable)._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x663144);
  this[1].super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x663150);
  (**(code **)(unaff_EBX + 0x662c90))(this,entnum,serialNum);
  return &(this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable;
}


/* C_PropLinkedPortalDoor::~C_PropLinkedPortalDoor at 00536a60 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_PropLinkedPortalDoor(C_PropLinkedPortalDoor * this, int
   __in_chrg) */

void __thiscall
C_PropLinkedPortalDoor::~C_PropLinkedPortalDoor(C_PropLinkedPortalDoor *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x662afb);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x662f5b);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x663013);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x66304f);
  (this->super_C_BaseAnimating).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x66306b);
  (this->super_CSignifierTarget).super_ISignifierTarget._vptr_ISignifierTarget =
       (_func_int_varargs **)(unaff_EBX + 0x663077);
  C_BaseAnimating::~C_BaseAnimating(&this->super_C_BaseAnimating,in_stack_ffffffe8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_PropLinkedPortalDoor::~C_PropLinkedPortalDoor at 00536ae0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_PropLinkedPortalDoor(C_PropLinkedPortalDoor * this, int
   __in_chrg) */

void __thiscall
C_PropLinkedPortalDoor::~C_PropLinkedPortalDoor(C_PropLinkedPortalDoor *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x662a80);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(extraout_ECX + 0x662ee0);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(extraout_ECX + 0x662f98);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(extraout_ECX + 0x662fd4);
  (this->super_C_BaseAnimating).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(extraout_ECX + 0x662ff0);
  (this->super_CSignifierTarget).super_ISignifierTarget._vptr_ISignifierTarget =
       (_func_int_varargs **)(extraout_ECX + 0x662ffc);
  C_BaseAnimating::~C_BaseAnimating(&this->super_C_BaseAnimating,__in_chrg);
  return;
}


/* C_PropLinkedPortalDoor::OverrideSignifierPosition at 00536a50 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: bool OverrideSignifierPosition(C_PropLinkedPortalDoor * this) */

bool __thiscall C_PropLinkedPortalDoor::OverrideSignifierPosition(C_PropLinkedPortalDoor *this)

{
  return true;
}


/* C_PropLinkedPortalDoor::GetSignifierPosition at 00536b60 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: bool GetSignifierPosition(C_PropLinkedPortalDoor * this, Vector *
   vSource, Vector * vPositionOut, Vector * vNormalOut) */

bool __thiscall
C_PropLinkedPortalDoor::GetSignifierPosition
          (C_PropLinkedPortalDoor *this,Vector *vSource,Vector *vPositionOut,Vector *vNormalOut)

{
  vec_t *pvVar1;
  
  pvVar1 = (vec_t *)(*(this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.
                      super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0x6d])(this);
  vPositionOut->x = *pvVar1;
  vPositionOut->y = pvVar1[1];
  vPositionOut->z = pvVar1[2];
  C_BaseEntity::GetVectors((C_BaseEntity *)this,vNormalOut,(Vector *)0x0,(Vector *)0x0);
  return true;
}


/* ClientClassInit<DT_PropLinkedPortalDoor::ignored> at 00078470 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

int ClientClassInit<DT_PropLinkedPortalDoor::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x30b].m_pEntity + unaff_EBX)
      == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc41214);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x30b].m_pPrev + unaff_EBX
                  ),(char *)(unaff_EBX + 0x8e78dc),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x30f].
                               m_SerialNumber + unaff_EBX),&UNK_008ec4bc + unaff_EBX,0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00ab8924 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00ab891c + unaff_EBX));
      ___cxa_guard_release(unaff_EBX + 0xc41214);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_nRandContextsInUse + unaff_EBX + 0x4c),
             (RecvProp_conflict1 *)
             ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x30f].m_SerialNumber +
             unaff_EBX),1,(char *)(unaff_EBX + 0x918080));
  return 1;
}


/* __static_initialization_and_destruction_0 at 00078590 */

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
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2f5].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2f5].m_pNext + unaff_EBX)
       = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2f5].m_pNext + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2f5].m_pNext + unaff_EBX + 2) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2f5].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2f6].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2f6].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2f6].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2f6].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2f7].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2f7].m_SerialNumber + unaff_EBX + 3) =
       0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2f7].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2f7].m_pNext + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2f8].m_pEntity + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2f8].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2f8].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined **)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2f8].m_pNext + unaff_EBX + 3) =
       &UNK_00ab9867 + unaff_EBX;
  *(int *)((int)s_SIMDRandContexts[0x1f].m_RandY[0x29] + unaff_EBX + 0xf) = unaff_EBX + 0x917f7b;
  *(int *)((int)s_SIMDRandContexts[0x1f].m_RandY[0x29] + unaff_EBX + 7) = unaff_EBX + 0x4be3df;
  *(undefined4 *)((int)s_SIMDRandContexts[0x1f].m_RandY[0x29] + unaff_EBX + 0xb) = 0;
  this = (RecvTable *)((int)s_SIMDRandContexts[0x1f].m_RandY[0x2a] + unaff_EBX + 0xf);
  *(RecvTable **)((int)s_SIMDRandContexts[0x1f].m_RandY[0x2a] + unaff_EBX + 3) = this;
  piVar1 = *(int **)(&DAT_00ab87f7 + unaff_EBX);
  *(int *)((int)s_SIMDRandContexts[0x1f].m_RandY[0x2a] + unaff_EBX + 7) = *piVar1;
  *piVar1 = unaff_EBX + 0xcd78b7;
  RecvTable::RecvTable(this);
  ___cxa_atexit(&UNK_008d95ff + unaff_EBX,0,*(undefined4 *)(&DAT_00ab877f + unaff_EBX));
  iVar2 = ClientClassInit<DT_PropLinkedPortalDoor::ignored>((ignored *)0x0);
  *(int *)((int)s_SIMDRandContexts[0x1f].m_RandY[0x2c] + unaff_EBX + 3) = iVar2;
  return;
}


/* C_PropLinkedPortalDoor::YouForgotToImplementOrDeclareClientClass at 00536950 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_PropLinkedPortalDoor *
   this) */

int __thiscall
C_PropLinkedPortalDoor::YouForgotToImplementOrDeclareClientClass(C_PropLinkedPortalDoor *this)

{
  return 0;
}


/* C_PropLinkedPortalDoor::GetClientClass at 00536970 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: ClientClass * GetClientClass(C_PropLinkedPortalDoor * this) */

ClientClass * __thiscall C_PropLinkedPortalDoor::GetClientClass(C_PropLinkedPortalDoor *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x8194e0);
}


/* __tcf_0 at 00951ba0 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3fe2c8),in_stack_00000008);
  return;
}


/* _GLOBAL__I__ZN22C_PropLinkedPortalDoor17m_pClassRecvTableE at 000786b0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN22C_PropLinkedPortalDoor17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

