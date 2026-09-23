/* DWARF-guided pseudocode for game/client/portal/c_func_noportal_volume.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* ClientClassInit<DT_FuncNoPortalVolume::ignored> at 000694d0 */

int ClientClassInit<DT_FuncNoPortalVolume::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (COM_GetModDirectory::modDir[unaff_EBX + 0x94] == '\0') {
    iVar1 = ___cxa_guard_acquire(COM_GetModDirectory::modDir + unaff_EBX + 0x94);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)(COM_GetModDirectory::modDir + unaff_EBX + 0x9c),
                  (char *)(unaff_EBX + 0x92128c),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)(COM_GetModDirectory::modDir + unaff_EBX + 0xd8),
                        (char *)(unaff_EBX + 0x925dd0),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00afa94c + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00afa908 + unaff_EBX));
      RecvPropBool((RecvProp *)((int)&vec2_invalid.y + unaff_EBX),(char *)(unaff_EBX + 0x93b8e4),
                   0xa5d,1);
      ___cxa_guard_release(COM_GetModDirectory::modDir + unaff_EBX + 0x94);
    }
  }
  RecvTable::Construct
            ((RecvTable *)(s_SortBuffer + unaff_EBX + 0x5854),
             (RecvProp_conflict1 *)(COM_GetModDirectory::modDir + unaff_EBX + 0xd8),2,
             (char *)(unaff_EBX + 0x94c854));
  return 1;
}


/* __static_initialization_and_destruction_0 at 00069620 */

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
  *(undefined1 *)((int)&vec2_invalid.x + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&vec2_invalid.y + unaff_EBX) = 0;
  *(undefined1 *)((int)&vec2_invalid.y + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&vec2_invalid.y + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&vec2_invalid.y + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&vec4_origin.x + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&vec4_origin.y + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&vec4_origin.z + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&vec4_origin.w + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&vec4_invalid.x + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&vec4_invalid.y + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&vec4_invalid.z + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&vec4_invalid.w + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&g_SharedVarSaveDataOps.super_CDefSaveRestoreOps.super_ISaveRestoreOps.
          _vptr_ISaveRestoreOps + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&BlackBox_Record(char_const*,char_const*,...)::blackbox + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)(&DAT_00c612bf + unaff_EBX) = 0x7f7fffff;
  *(undefined **)((int)&BlackBox_Record::blackbox.m_pConVar + unaff_EBX + 3) =
       &UNK_00afb857 + unaff_EBX;
  *(int *)(s_SortBuffer + unaff_EBX + 0x56f7) = unaff_EBX + 0x94c71d;
  *(int *)(s_SortBuffer + (int)(&UNK_000056ef + unaff_EBX)) = unaff_EBX + 0x4674df;
  puVar1 = s_SortBuffer + unaff_EBX + 0x56f3;
  puVar1[0] = '\0';
  puVar1[1] = '\0';
  puVar1[2] = '\0';
  puVar1[3] = '\0';
  *(RecvTable **)(s_SortBuffer + unaff_EBX + 0x56fb) =
       (RecvTable *)(s_SortBuffer + unaff_EBX + 0x5707);
  piVar2 = *(int **)(&DAT_00afa7b3 + unaff_EBX);
  *(int *)(s_SortBuffer + unaff_EBX + 0x56ff) = *piVar2;
  *piVar2 = (int)(s_SortBuffer + (int)(&UNK_000056ef + unaff_EBX));
  RecvTable::RecvTable((RecvTable *)(s_SortBuffer + unaff_EBX + 0x5707));
  ___cxa_atexit(unaff_EBX + 0x910a0f,0,*(undefined4 *)(&DAT_00afa73b + unaff_EBX));
  iVar3 = ClientClassInit<DT_FuncNoPortalVolume::ignored>((ignored *)0x0);
  *(int *)(s_SortBuffer + unaff_EBX + 0x571b) = iVar3;
  return;
}


/* C_FuncNoPortalVolume::YouForgotToImplementOrDeclareClientClass at 004d07f0 */

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


/* C_FuncNoPortalVolume::GetClientClass at 004d0810 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_FuncNoPortalVolume": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: ClientClass * GetClientClass(C_FuncNoPortalVolume * this) */

ClientClass * __thiscall C_FuncNoPortalVolume::GetClientClass(C_FuncNoPortalVolume *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x895ee8);
}


/* _C_FuncNoPortalVolume_CreateObject at 004d0b10 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_FuncNoPortalVolume_CreateObject(int entnum,int serialNum)

{
  C_BaseEntity *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_FuncNoPortalVolume * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xa64);
  C_BaseEntity::C_BaseEntity(this);
  (this->super_IClientEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x6e18e4);
  (this->super_IClientEntity).super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6e1c80);
  (this->super_IClientEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6e1d38);
  (this->super_IClientEntity).super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(ImageLoader::DecodeColorBlock<RGB888_t> + unaff_EBX + 4);
  (this->super_IClientModelRenderable)._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6e1d90);
  this[1].super_IClientEntity.super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       *(_func_int_varargs ***)(unaff_EBX + 0x788a54);
  *(C_BaseEntity **)(unaff_EBX + 0x788a54) = this;
  (*(this->super_IClientEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0x2f])
            (this,entnum,serialNum);
  return &(this->super_IClientEntity).super_IClientNetworkable;
}


/* __tcf_0 at 0097a040 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3ec6d0),in_stack_00000008);
  return;
}


/* C_FuncNoPortalVolume::C_FuncNoPortalVolume at 004d0a10 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6e19eb);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6e1d87);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6e1e3f);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6e1e7b);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6e1e97);
  this->m_pNext = *(C_FuncNoPortalVolume **)(unaff_EBX + 0x788b5b);
  *(C_FuncNoPortalVolume **)(unaff_EBX + 0x788b5b) = this;
  return;
}


/* C_FuncNoPortalVolume::C_FuncNoPortalVolume at 004d0a90 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6e196b);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6e1d07);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6e1dbf);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6e1dfb);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6e1e17);
  this->m_pNext = *(C_FuncNoPortalVolume **)(unaff_EBX + 0x788adb);
  *(C_FuncNoPortalVolume **)(unaff_EBX + 0x788adb) = this;
  return;
}


/* C_FuncNoPortalVolume::~C_FuncNoPortalVolume at 004d0830 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6e1bcb);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6e1f67);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(&UNK_006e201f + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6e205b);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6e2077);
  pCVar1 = *(C_FuncNoPortalVolume **)(unaff_EBX + 0x788d3b);
  if (this == *(C_FuncNoPortalVolume **)(unaff_EBX + 0x788d3b)) {
    *(C_FuncNoPortalVolume **)(unaff_EBX + 0x788d3b) = this->m_pNext;
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


/* C_FuncNoPortalVolume::~C_FuncNoPortalVolume at 004d08d0 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6e1b2f);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6e1ecb);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6e1f83);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6e1fbf);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6e1fdb);
  pCVar1 = *(C_FuncNoPortalVolume **)(unaff_EBX + 0x788c9f);
  if (this != *(C_FuncNoPortalVolume **)(unaff_EBX + 0x788c9f)) {
    do {
      pCVar2 = pCVar1;
                    /* Unresolved local var: C_FuncNoPortalVolume * pWalk@[???] */
      pCVar1 = pCVar2->m_pNext;
    } while (pCVar1 != this);
    pCVar2->m_pNext = this->m_pNext;
    C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
    return;
  }
  *(C_FuncNoPortalVolume **)(unaff_EBX + 0x788c9f) = this->m_pNext;
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
  return;
}


/* C_FuncNoPortalVolume::~C_FuncNoPortalVolume at 004d0970 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6e1a8f);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6e1e2b);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6e1ee3);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6e1f1f);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6e1f3b);
  pCVar1 = *(C_FuncNoPortalVolume **)(unaff_EBX + 0x788bff);
  if (this != *(C_FuncNoPortalVolume **)(unaff_EBX + 0x788bff)) {
    do {
      pCVar2 = pCVar1;
                    /* Unresolved local var: C_FuncNoPortalVolume * pWalk@[???] */
      pCVar1 = pCVar2->m_pNext;
    } while (pCVar1 != this);
    pCVar2->m_pNext = this->m_pNext;
    C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
    return;
  }
  *(C_FuncNoPortalVolume **)(unaff_EBX + 0x788bff) = this->m_pNext;
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
  return;
}


/* GetNoPortalVolumeList at 004d0820 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_FuncNoPortalVolume": ignoring overlapping field "m_bActive" */

CFuncNoPortalVolume * GetNoPortalVolumeList(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(CFuncNoPortalVolume **)(extraout_ECX + 0x788d50);
}


/* _GLOBAL__I__ZN20C_FuncNoPortalVolume12s_pClassListE at 00069740 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN20C_FuncNoPortalVolume12s_pClassListE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

