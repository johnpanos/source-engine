/* DWARF-guided pseudocode for game/client/portal/c_func_noportal_volume.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* ClientClassInit<DT_FuncNoPortalVolume::ignored> at 000699f0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

int ClientClassInit<DT_FuncNoPortalVolume::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0xa0f].m_pEntity + unaff_EBX)
      == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc48254);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0xa0f].m_pPrev + unaff_EBX
                  ),(char *)(unaff_EBX + 0x8f635c),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0xa13].
                               m_SerialNumber + unaff_EBX),(char *)(unaff_EBX + 0x8faf3c),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00ac73e0 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00ac739c + unaff_EBX));
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0xa17].m_pEntity +
                   unaff_EBX),(char *)(unaff_EBX + 0x9109d0),0xa59,1);
      ___cxa_guard_release(unaff_EBX + 0xc48254);
    }
  }
  RecvTable::Construct
            ((RecvTable *)(s_SortBuffer + unaff_EBX + 0x5304),
             (RecvProp_conflict1 *)
             ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0xa13].m_SerialNumber +
             unaff_EBX),2,&UNK_00921a4a + unaff_EBX);
  return 1;
}


/* __static_initialization_and_destruction_0 at 00069b40 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  uint8 *puVar1;
  int *piVar2;
  int iVar3;
  int unaff_EBX;
  longlong lVar4;
  
  lVar4 = ___i686_get_pc_thunk_bx();
  if (lVar4 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9f6].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9f6].m_pNext + unaff_EBX)
       = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9f6].m_pNext + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9f6].m_pNext + unaff_EBX + 2) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9f6].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9f7].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9f7].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9f7].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9f7].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9f8].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9f8].m_SerialNumber + unaff_EBX + 3) =
       0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9f8].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9f8].m_pNext + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9f9].m_pEntity + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9f9].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9f9].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined **)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9f9].m_pNext + unaff_EBX + 3) =
       &UNK_00ac82b7 + unaff_EBX;
  *(int *)(s_SortBuffer + unaff_EBX + 0x51a7) = unaff_EBX + 0x921913;
  *(int *)(s_SortBuffer + unaff_EBX + 0x519f) = unaff_EBX + 0x46be2f;
  puVar1 = s_SortBuffer + unaff_EBX + 0x51a3;
  puVar1[0] = '\0';
  puVar1[1] = '\0';
  puVar1[2] = '\0';
  puVar1[3] = '\0';
  *(RecvTable **)(s_SortBuffer + unaff_EBX + 0x51ab) =
       (RecvTable *)(s_SortBuffer + unaff_EBX + 0x51b7);
  piVar2 = *(int **)(&DAT_00ac7247 + unaff_EBX);
  *(int *)(s_SortBuffer + unaff_EBX + 0x51af) = *piVar2;
  *piVar2 = (int)(s_SortBuffer + unaff_EBX + 0x519f);
  RecvTable::RecvTable((RecvTable *)(s_SortBuffer + unaff_EBX + 0x51b7));
  ___cxa_atexit(&UNK_008e606f + unaff_EBX,0,*(undefined4 *)(&DAT_00ac71cf + unaff_EBX));
  iVar3 = ClientClassInit<DT_FuncNoPortalVolume::ignored>((ignored *)0x0);
  *(int *)(s_SortBuffer + unaff_EBX + 0x51cb) = iVar3;
  return;
}


/* C_FuncNoPortalVolume::YouForgotToImplementOrDeclareClientClass at 004d56a0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_FuncNoPortalVolume": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_FuncNoPortalVolume *
   this) */

int __thiscall
C_FuncNoPortalVolume::YouForgotToImplementOrDeclareClientClass(C_FuncNoPortalVolume *this)

{
  return 0;
}


/* C_FuncNoPortalVolume::GetClientClass at 004d56c0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_FuncNoPortalVolume": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: ClientClass * GetClientClass(C_FuncNoPortalVolume * this) */

ClientClass * __thiscall C_FuncNoPortalVolume::GetClientClass(C_FuncNoPortalVolume *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(&UNK_00877ca8 + extraout_ECX);
}


/* _C_FuncNoPortalVolume_CreateObject at 004d5980 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_FuncNoPortalVolume_CreateObject(int entnum,int serialNum)

{
  C_BaseEntity *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_FuncNoPortalVolume * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xa60);
  C_BaseEntity::C_BaseEntity(this);
  (this->super_IClientEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x6a94f4);
  (this->super_IClientEntity).super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6a988c);
  (this->super_IClientEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6a9944);
  (this->super_IClientEntity).super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6a9980);
  this[1].super_IClientEntity.super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       *(_func_int_varargs ***)(unaff_EBX + 0x74dca4);
  *(C_BaseEntity **)(unaff_EBX + 0x74dca4) = this;
  (*(this->super_IClientEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0x2f])
            (this,entnum,serialNum);
  return &(this->super_IClientEntity).super_IClientNetworkable;
}


/* __tcf_0 at 0094fbc0 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3fd7c0),in_stack_00000008);
  return;
}


/* C_FuncNoPortalVolume::C_FuncNoPortalVolume at 004d58a0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_FuncNoPortalVolume": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void C_FuncNoPortalVolume(C_FuncNoPortalVolume * this) */

void __thiscall C_FuncNoPortalVolume::C_FuncNoPortalVolume(C_FuncNoPortalVolume *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::C_BaseEntity(&this->super_C_BaseEntity);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6a95d7);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6a996f);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6a9a27);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6a9a63);
  this->m_pNext = *(C_FuncNoPortalVolume **)(unaff_EBX + 0x74dd87);
  *(C_FuncNoPortalVolume **)(unaff_EBX + 0x74dd87) = this;
  return;
}


/* C_FuncNoPortalVolume::C_FuncNoPortalVolume at 004d5910 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_FuncNoPortalVolume": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void C_FuncNoPortalVolume(C_FuncNoPortalVolume * this) */

void __thiscall C_FuncNoPortalVolume::C_FuncNoPortalVolume(C_FuncNoPortalVolume *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::C_BaseEntity(&this->super_C_BaseEntity);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6a9567);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6a98ff);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6a99b7);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6a99f3);
  this->m_pNext = *(C_FuncNoPortalVolume **)(unaff_EBX + 0x74dd17);
  *(C_FuncNoPortalVolume **)(unaff_EBX + 0x74dd17) = this;
  return;
}


/* C_FuncNoPortalVolume::~C_FuncNoPortalVolume at 004d56e0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_FuncNoPortalVolume": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void ~C_FuncNoPortalVolume(C_FuncNoPortalVolume * this, int __in_chrg)
    */

void __thiscall
C_FuncNoPortalVolume::~C_FuncNoPortalVolume(C_FuncNoPortalVolume *this,int __in_chrg)

{
  C_FuncNoPortalVolume *pCVar1;
  C_FuncNoPortalVolume *pCVar2;
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6a979b);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6a9b33);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6a9beb);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6a9c27);
  pCVar1 = *(C_FuncNoPortalVolume **)(&DAT_0074df4b + unaff_EBX);
  if (this == *(C_FuncNoPortalVolume **)(&DAT_0074df4b + unaff_EBX)) {
    *(C_FuncNoPortalVolume **)(&DAT_0074df4b + unaff_EBX) = this->m_pNext;
  }
  else {
    do {
      pCVar2 = pCVar1;
                    /* Unresolved local var: C_FuncNoPortalVolume * pWalk@[???] */
      pCVar1 = pCVar2->m_pNext;
    } while (pCVar1 != this);
    pCVar2->m_pNext = this->m_pNext;
  }
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,in_stack_ffffffe8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_FuncNoPortalVolume::~C_FuncNoPortalVolume at 004d5780 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_FuncNoPortalVolume": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void ~C_FuncNoPortalVolume(C_FuncNoPortalVolume * this, int __in_chrg)
    */

void __thiscall
C_FuncNoPortalVolume::~C_FuncNoPortalVolume(C_FuncNoPortalVolume *this,int __in_chrg)

{
  C_FuncNoPortalVolume *pCVar1;
  C_FuncNoPortalVolume *pCVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6a96ff);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6a9a97);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6a9b4f);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6a9b8b);
  pCVar1 = *(C_FuncNoPortalVolume **)(&DAT_0074deaf + unaff_EBX);
  if (this != *(C_FuncNoPortalVolume **)(&DAT_0074deaf + unaff_EBX)) {
    do {
      pCVar2 = pCVar1;
                    /* Unresolved local var: C_FuncNoPortalVolume * pWalk@[???] */
      pCVar1 = pCVar2->m_pNext;
    } while (pCVar1 != this);
    pCVar2->m_pNext = this->m_pNext;
    C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
    return;
  }
  *(C_FuncNoPortalVolume **)(&DAT_0074deaf + unaff_EBX) = this->m_pNext;
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
  return;
}


/* C_FuncNoPortalVolume::~C_FuncNoPortalVolume at 004d5810 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_FuncNoPortalVolume": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void ~C_FuncNoPortalVolume(C_FuncNoPortalVolume * this, int __in_chrg)
    */

void __thiscall
C_FuncNoPortalVolume::~C_FuncNoPortalVolume(C_FuncNoPortalVolume *this,int __in_chrg)

{
  C_FuncNoPortalVolume *pCVar1;
  C_FuncNoPortalVolume *pCVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6a966f);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6a9a07);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6a9abf);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6a9afb);
  pCVar1 = *(C_FuncNoPortalVolume **)(unaff_EBX + 0x74de1f);
  if (this != *(C_FuncNoPortalVolume **)(unaff_EBX + 0x74de1f)) {
    do {
      pCVar2 = pCVar1;
                    /* Unresolved local var: C_FuncNoPortalVolume * pWalk@[???] */
      pCVar1 = pCVar2->m_pNext;
    } while (pCVar1 != this);
    pCVar2->m_pNext = this->m_pNext;
    C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
    return;
  }
  *(C_FuncNoPortalVolume **)(unaff_EBX + 0x74de1f) = this->m_pNext;
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
  return;
}


/* GetNoPortalVolumeList at 004d56d0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_FuncNoPortalVolume": ignoring overlapping field "m_bActive" */

CFuncNoPortalVolume * GetNoPortalVolumeList(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(CFuncNoPortalVolume **)(&DAT_0074df60 + extraout_ECX);
}


/* _GLOBAL__I__ZN20C_FuncNoPortalVolume12s_pClassListE at 00069c60 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN20C_FuncNoPortalVolume12s_pClassListE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

