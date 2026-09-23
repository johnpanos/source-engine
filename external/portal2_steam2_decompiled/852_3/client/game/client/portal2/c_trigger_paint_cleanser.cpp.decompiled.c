/* DWARF-guided pseudocode for game/client/portal2/c_trigger_paint_cleanser.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* ClientClassInit<DT_TriggerPaintCleanser::ignored> at 00079000 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

int ClientClassInit<DT_TriggerPaintCleanser::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2a2].m_pEntity + unaff_EBX)
      == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc40b84);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2a2].m_pPrev + unaff_EBX
                  ),(char *)(unaff_EBX + 0x8e6d4c),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2a6].
                               m_SerialNumber + unaff_EBX),(char *)(unaff_EBX + 0x8eb92c),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00ab8254 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00ab7d8c + unaff_EBX));
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2aa].m_pEntity +
                   unaff_EBX),(char *)(unaff_EBX + 0x9177a8),0xa5a,1);
      ___cxa_guard_release(unaff_EBX + 0xc40b84);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_SIMDRandContexts[0x1d].m_RandY[8] + unaff_EBX),
             (RecvProp_conflict1 *)
             ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2a6].m_SerialNumber +
             unaff_EBX),2,(char *)(unaff_EBX + 0x9176c7));
  return 1;
}


/* __static_initialization_and_destruction_0 at 00079150 */

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
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x289].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x289].m_pNext + unaff_EBX)
       = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x289].m_pNext + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x289].m_pNext + unaff_EBX + 2) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x289].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x28a].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x28a].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x28a].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x28a].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x28b].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x28b].m_SerialNumber + unaff_EBX + 3) =
       0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x28b].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x28b].m_pNext + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x28c].m_pEntity + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x28c].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x28c].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined **)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x28c].m_pNext + unaff_EBX + 3) =
       &UNK_00ab8ca7 + unaff_EBX;
  *(int *)((int)s_SIMDRandContexts[0x1c].m_RandY[0x2a] + unaff_EBX + 3) = unaff_EBX + 0x917592;
  *(undefined **)((int)s_SIMDRandContexts[0x1c].m_RandY[0x29] + unaff_EBX + 0xb) =
       &UNK_004c187f + unaff_EBX;
  *(undefined4 *)((int)s_SIMDRandContexts[0x1c].m_RandY[0x29] + unaff_EBX + 0xf) = 0;
  this = (RecvTable *)((int)s_SIMDRandContexts[0x1c].m_RandY[0x2b] + unaff_EBX + 3);
  *(RecvTable **)((int)s_SIMDRandContexts[0x1c].m_RandY[0x2a] + unaff_EBX + 7) = this;
  piVar1 = *(int **)(&DAT_00ab7c37 + unaff_EBX);
  *(int *)((int)s_SIMDRandContexts[0x1c].m_RandY[0x2a] + unaff_EBX + 0xb) = *piVar1;
  *piVar1 = unaff_EBX + 0xcd6e3b;
  RecvTable::RecvTable(this);
  ___cxa_atexit(&UNK_008d8adf + unaff_EBX,0,*(undefined4 *)(&DAT_00ab7bbf + unaff_EBX));
  iVar2 = ClientClassInit<DT_TriggerPaintCleanser::ignored>((ignored *)0x0);
  *(int *)((int)s_SIMDRandContexts[0x1c].m_RandY[0x2c] + unaff_EBX + 7) = iVar2;
  return;
}


/* C_TriggerPaintCleanser::YouForgotToImplementOrDeclareClientClass at 0053a7b0 */

/* WARNING: Struct "C_TriggerPaintCleanser": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_TriggerPaintCleanser *
   this) */

int __thiscall
C_TriggerPaintCleanser::YouForgotToImplementOrDeclareClientClass(C_TriggerPaintCleanser *this)

{
  return 0;
}


/* C_TriggerPaintCleanser::GetClientClass at 0053a7d0 */

/* WARNING: Struct "C_TriggerPaintCleanser": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: ClientClass * GetClientClass(C_TriggerPaintCleanser * this) */

ClientClass * __thiscall C_TriggerPaintCleanser::GetClientClass(C_TriggerPaintCleanser *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(&UNK_008157c4 + extraout_ECX);
}


/* _C_TriggerPaintCleanser_CreateObject at 0053a9e0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_TriggerPaintCleanser_CreateObject(int entnum,int serialNum)

{
  C_BaseEntity *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_TriggerPaintCleanser * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xa5c);
  C_BaseEntity::C_BaseEntity(this);
  (this->super_IClientEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x6601f4);
  (this->super_IClientEntity).super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x66058c);
  (this->super_IClientEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x660644);
  (this->super_IClientEntity).super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x660680);
  (**(code **)(unaff_EBX + 0x6602b0))(this,entnum,serialNum);
  return &(this->super_IClientEntity).super_IClientNetworkable;
}


/* __tcf_0 at 00951c40 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3fe36c),in_stack_00000008);
  return;
}


/* C_TriggerPaintCleanser::C_TriggerPaintCleanser at 0053a920 */

/* WARNING: Struct "C_TriggerPaintCleanser": ignoring overlapping field "m_bDisabled" */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void C_TriggerPaintCleanser(C_TriggerPaintCleanser * this) */

void __thiscall C_TriggerPaintCleanser::C_TriggerPaintCleanser(C_TriggerPaintCleanser *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::C_BaseEntity((C_BaseEntity *)this);
  *(int *)&this->super_C_BaseTrigger = unaff_EBX + 0x6602b7;
  *(undefined **)&(this->super_C_BaseTrigger).field_0x4 = &UNK_0066064f + unaff_EBX;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = unaff_EBX + 0x660707;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = unaff_EBX + 0x660743;
  return;
}


/* C_TriggerPaintCleanser::C_TriggerPaintCleanser at 0053a980 */

/* WARNING: Struct "C_TriggerPaintCleanser": ignoring overlapping field "m_bDisabled" */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void C_TriggerPaintCleanser(C_TriggerPaintCleanser * this) */

void __thiscall C_TriggerPaintCleanser::C_TriggerPaintCleanser(C_TriggerPaintCleanser *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::C_BaseEntity((C_BaseEntity *)this);
  *(undefined **)&this->super_C_BaseTrigger = &UNK_00660257 + unaff_EBX;
  *(int *)&(this->super_C_BaseTrigger).field_0x4 = unaff_EBX + 0x6605ef;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = unaff_EBX + 0x6606a7;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = unaff_EBX + 0x6606e3;
  return;
}


/* C_TriggerPaintCleanser::~C_TriggerPaintCleanser at 0053a860 */

/* WARNING: Struct "C_TriggerPaintCleanser": ignoring overlapping field "m_bDisabled" */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_TriggerPaintCleanser(C_TriggerPaintCleanser * this, int
   __in_chrg) */

void __thiscall
C_TriggerPaintCleanser::~C_TriggerPaintCleanser(C_TriggerPaintCleanser *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *(int *)&this->super_C_BaseTrigger = extraout_ECX + 0x660380;
  *(int *)&(this->super_C_BaseTrigger).field_0x4 = extraout_ECX + 0x660718;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = extraout_ECX + 0x6607d0;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = extraout_ECX + 0x66080c;
  *(int *)&this->super_C_BaseTrigger = *(int *)(extraout_ECX + 0x5f69fc) + 8;
  *(int *)&(this->super_C_BaseTrigger).field_0x4 = *(int *)(extraout_ECX + 0x5f69fc) + 0x3a0;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = *(int *)(extraout_ECX + 0x5f69fc) + 0x458;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = *(int *)(extraout_ECX + 0x5f69fc) + 0x494;
  *(int *)&this->super_C_BaseTrigger = *(int *)(extraout_ECX + 0x5f69a8) + 8;
  *(int *)&(this->super_C_BaseTrigger).field_0x4 = *(int *)(extraout_ECX + 0x5f69a8) + 0x3a0;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = *(int *)(extraout_ECX + 0x5f69a8) + 0x458;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = *(int *)(extraout_ECX + 0x5f69a8) + 0x494;
  C_BaseEntity::~C_BaseEntity((C_BaseEntity *)this,__in_chrg);
  return;
}


/* C_TriggerPaintCleanser::~C_TriggerPaintCleanser at 0053aa80 */

/* WARNING: Struct "C_TriggerPaintCleanser": ignoring overlapping field "m_bDisabled" */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_TriggerPaintCleanser(C_TriggerPaintCleanser * this, int
   __in_chrg) */

void __thiscall
C_TriggerPaintCleanser::~C_TriggerPaintCleanser(C_TriggerPaintCleanser *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *(int *)&this->super_C_BaseTrigger = extraout_ECX + 0x660160;
  *(int *)&(this->super_C_BaseTrigger).field_0x4 = extraout_ECX + 0x6604f8;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = extraout_ECX + 0x6605b0;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = extraout_ECX + 0x6605ec;
  *(int *)&this->super_C_BaseTrigger = *(int *)(extraout_ECX + 0x5f67dc) + 8;
  *(int *)&(this->super_C_BaseTrigger).field_0x4 = *(int *)(extraout_ECX + 0x5f67dc) + 0x3a0;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = *(int *)(extraout_ECX + 0x5f67dc) + 0x458;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = *(int *)(extraout_ECX + 0x5f67dc) + 0x494;
  *(int *)&this->super_C_BaseTrigger = *(int *)(extraout_ECX + 0x5f6788) + 8;
  *(int *)&(this->super_C_BaseTrigger).field_0x4 = *(int *)(extraout_ECX + 0x5f6788) + 0x3a0;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = *(int *)(extraout_ECX + 0x5f6788) + 0x458;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = *(int *)(extraout_ECX + 0x5f6788) + 0x494;
  C_BaseEntity::~C_BaseEntity((C_BaseEntity *)this,__in_chrg);
  return;
}


/* C_TriggerPaintCleanser::~C_TriggerPaintCleanser at 0053ab40 */

/* WARNING: Struct "C_TriggerPaintCleanser": ignoring overlapping field "m_bDisabled" */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_TriggerPaintCleanser(C_TriggerPaintCleanser * this, int
   __in_chrg) */

void __thiscall
C_TriggerPaintCleanser::~C_TriggerPaintCleanser(C_TriggerPaintCleanser *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  *(int *)&this->super_C_BaseTrigger = unaff_EBX + 0x66009b;
  *(undefined **)&(this->super_C_BaseTrigger).field_0x4 = &UNK_00660433 + unaff_EBX;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = unaff_EBX + 0x6604eb;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = unaff_EBX + 0x660527;
  *(int *)&this->super_C_BaseTrigger = *(int *)(unaff_EBX + 0x5f6717) + 8;
  *(int *)&(this->super_C_BaseTrigger).field_0x4 = *(int *)(unaff_EBX + 0x5f6717) + 0x3a0;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = *(int *)(unaff_EBX + 0x5f6717) + 0x458;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = *(int *)(unaff_EBX + 0x5f6717) + 0x494;
  *(int *)&this->super_C_BaseTrigger = *(int *)(unaff_EBX + 0x5f66c3) + 8;
  *(int *)&(this->super_C_BaseTrigger).field_0x4 = *(int *)(unaff_EBX + 0x5f66c3) + 0x3a0;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = *(int *)(unaff_EBX + 0x5f66c3) + 0x458;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = *(int *)(unaff_EBX + 0x5f66c3) + 0x494;
  C_BaseEntity::~C_BaseEntity((C_BaseEntity *)this,in_stack_ffffffe8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_TriggerPaintCleanser::Spawn at 0053a820 */

/* WARNING: Struct "C_TriggerPaintCleanser": ignoring overlapping field "m_bDisabled" */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void Spawn(C_TriggerPaintCleanser * this) */

void __thiscall C_TriggerPaintCleanser::Spawn(C_TriggerPaintCleanser *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::Spawn((C_BaseEntity *)this);
  CPaintCleanserManager::AddPaintCleanser(*(CPaintCleanserManager **)(unaff_EBX + 0x5f66ff),this);
  return;
}


/* C_TriggerPaintCleanser::UpdateOnRemove at 0053a7e0 */

/* WARNING: Struct "C_TriggerPaintCleanser": ignoring overlapping field "m_bDisabled" */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void UpdateOnRemove(C_TriggerPaintCleanser * this) */

void __thiscall C_TriggerPaintCleanser::UpdateOnRemove(C_TriggerPaintCleanser *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CPaintCleanserManager::RemovePaintCleanser(*(CPaintCleanserManager **)(unaff_EBX + 0x5f673f),this)
  ;
  C_BaseEntity::UpdateOnRemove((C_BaseEntity *)this);
  return;
}


/* _GLOBAL__I__ZN22C_TriggerPaintCleanser17m_pClassRecvTableE at 00079270 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN22C_TriggerPaintCleanser17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

