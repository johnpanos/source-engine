/* DWARF-guided pseudocode for game/client/portal2/c_trigger_paint_cleanser.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* ClientClassInit<DT_TriggerPaintCleanser::ignored> at 00079660 */

int ClientClassInit<DT_TriggerPaintCleanser::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)NPaintRenderer::g_FastSpherePosData[0x16] + unaff_EBX + 4) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc599e4);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)NPaintRenderer::g_FastSpherePosData[0x16] + unaff_EBX + 0xc),
                  (char *)(unaff_EBX + 0x9110fc),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)NPaintRenderer::g_FastSpherePosData[0x18] + unaff_EBX + 8),
                        (char *)(unaff_EBX + 0x915c40),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00aeac44 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00aea778 + unaff_EBX));
      RecvPropBool((RecvProp *)((int)NPaintRenderer::g_FastSpherePosData[0x1a] + unaff_EBX + 4),
                   (char *)(unaff_EBX + 0x941fd0),0xa5e,1);
      ___cxa_guard_release(unaff_EBX + 0xc599e4);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_SIMDRandContexts[0x1c].m_RandY[0xc] + unaff_EBX),
             (RecvProp_conflict1 *)((int)NPaintRenderer::g_FastSpherePosData[0x18] + unaff_EBX + 8),
             2,(char *)(unaff_EBX + 0x941ef4));
  return 1;
}


/* __static_initialization_and_destruction_0 at 000797b0 */

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
  *(undefined1 *)((int)NPaintRenderer::g_FastSpherePosData[9] + unaff_EBX + 0x1f) = 0;
  *(undefined1 *)((int)NPaintRenderer::g_FastSpherePosData[10] + unaff_EBX) = 0;
  *(undefined1 *)((int)NPaintRenderer::g_FastSpherePosData[10] + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)NPaintRenderer::g_FastSpherePosData[10] + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[10] + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[10] + unaff_EBX + 7) = 0;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[10] + unaff_EBX + 0xb) = 0x7f7fffff;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[10] + unaff_EBX + 0xf) = 0x7f7fffff;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[10] + unaff_EBX + 0x13) = 0;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[10] + unaff_EBX + 0x17) = 0;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[10] + unaff_EBX + 0x1b) = 0;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[10] + unaff_EBX + 0x1f) = 0;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0xb] + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0xb] + unaff_EBX + 7) = 0x7f7fffff;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0xb] + unaff_EBX + 0xb) = 0x7f7fffff;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0xb] + unaff_EBX + 0xf) = 0x7f7fffff;
  *(undefined **)((int)NPaintRenderer::g_FastSpherePosData[0xb] + unaff_EBX + 0x13) =
       &UNK_00aeb6c7 + unaff_EBX;
  *(int *)((int)s_SIMDRandContexts[0x1b].m_RandY[0x2e] + unaff_EBX + 3) = unaff_EBX + 0x941dbf;
  *(int *)((int)s_SIMDRandContexts[0x1b].m_RandY[0x2d] + unaff_EBX + 0xb) = unaff_EBX + 0x4c32ff;
  *(undefined4 *)((int)s_SIMDRandContexts[0x1b].m_RandY[0x2d] + unaff_EBX + 0xf) = 0;
  this = (RecvTable *)((int)s_SIMDRandContexts[0x1b].m_RandY[0x2f] + unaff_EBX + 3);
  *(RecvTable **)((int)s_SIMDRandContexts[0x1b].m_RandY[0x2e] + unaff_EBX + 7) = this;
  piVar1 = *(int **)(&DAT_00aea623 + unaff_EBX);
  *(int *)((int)s_SIMDRandContexts[0x1b].m_RandY[0x2e] + unaff_EBX + 0xb) = *piVar1;
  *piVar1 = unaff_EBX + 0xcefe5b;
  RecvTable::RecvTable(this);
  ___cxa_atexit(unaff_EBX + 0x902b7f,0,*(undefined4 *)(&DAT_00aea5ab + unaff_EBX));
  iVar2 = ClientClassInit<DT_TriggerPaintCleanser::ignored>((ignored *)0x0);
  *(int *)((int)s_SIMDRandContexts[0x1b].m_RandY[0x30] + unaff_EBX + 7) = iVar2;
  return;
}


/* C_TriggerPaintCleanser::YouForgotToImplementOrDeclareClientClass at 0053c850 */

/* WARNING: Struct "C_TriggerPaintCleanser": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_TriggerPaintCleanser *
   this) */

int __thiscall
C_TriggerPaintCleanser::YouForgotToImplementOrDeclareClientClass(C_TriggerPaintCleanser *this)

{
  return 0;
}


/* C_TriggerPaintCleanser::GetClientClass at 0053c870 */

/* WARNING: Struct "C_TriggerPaintCleanser": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: ClientClass * GetClientClass(C_TriggerPaintCleanser * this) */

ClientClass * __thiscall C_TriggerPaintCleanser::GetClientClass(C_TriggerPaintCleanser *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x82cda4);
}


/* _C_TriggerPaintCleanser_CreateObject at 0053cac0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_TriggerPaintCleanser_CreateObject(int entnum,int serialNum)

{
  C_BaseEntity *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_TriggerPaintCleanser * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xa60);
  C_BaseEntity::C_BaseEntity(this);
  (this->super_IClientEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x6925b4);
  (this->super_IClientEntity).super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x692950);
  (this->super_IClientEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(&UNK_00692a08 + unaff_EBX);
  (this->super_IClientEntity).super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(&UNK_00692a44 + unaff_EBX);
  (this->super_IClientModelRenderable)._vptr_IClientModelRenderable =
       (_func_int_varargs **)(&UNK_00692a60 + unaff_EBX);
  (**(code **)(unaff_EBX + 0x692670))(this,entnum,serialNum);
  return &(this->super_IClientEntity).super_IClientNetworkable;
}


/* __tcf_0 at 0097c340 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3ed2ec),in_stack_00000008);
  return;
}


/* C_TriggerPaintCleanser::C_TriggerPaintCleanser at 0053c9e0 */

/* WARNING: Struct "C_TriggerPaintCleanser": ignoring overlapping field "m_bDisabled" */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void C_TriggerPaintCleanser(C_TriggerPaintCleanser * this) */

void __thiscall C_TriggerPaintCleanser::C_TriggerPaintCleanser(C_TriggerPaintCleanser *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::C_BaseEntity((C_BaseEntity *)this);
  *(int *)&this->super_C_BaseTrigger = unaff_EBX + 0x692697;
  *(undefined **)&(this->super_C_BaseTrigger).field_0x4 = &UNK_00692a33 + unaff_EBX;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = unaff_EBX + 0x692aeb;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = unaff_EBX + 0x692b27;
  *(int *)&(this->super_C_BaseTrigger).field_0x10 = unaff_EBX + 0x692b43;
  return;
}


/* C_TriggerPaintCleanser::C_TriggerPaintCleanser at 0053ca50 */

/* WARNING: Struct "C_TriggerPaintCleanser": ignoring overlapping field "m_bDisabled" */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void C_TriggerPaintCleanser(C_TriggerPaintCleanser * this) */

void __thiscall C_TriggerPaintCleanser::C_TriggerPaintCleanser(C_TriggerPaintCleanser *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::C_BaseEntity((C_BaseEntity *)this);
  *(int *)&this->super_C_BaseTrigger = unaff_EBX + 0x692627;
  *(int *)&(this->super_C_BaseTrigger).field_0x4 = unaff_EBX + 0x6929c3;
  *(undefined **)&(this->super_C_BaseTrigger).field_0x8 = &UNK_00692a7b + unaff_EBX;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = unaff_EBX + 0x692ab7;
  *(int *)&(this->super_C_BaseTrigger).field_0x10 = unaff_EBX + 0x692ad3;
  return;
}


/* C_TriggerPaintCleanser::~C_TriggerPaintCleanser at 0053c900 */

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
  *(int *)&this->super_C_BaseTrigger = extraout_ECX + 0x692780;
  *(int *)&(this->super_C_BaseTrigger).field_0x4 = extraout_ECX + 0x692b1c;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = extraout_ECX + 0x692bd4;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = extraout_ECX + 0x692c10;
  *(undefined **)&(this->super_C_BaseTrigger).field_0x10 = &UNK_00692c2c + extraout_ECX;
  *(int *)&this->super_C_BaseTrigger = *(int *)(extraout_ECX + 0x6279ac) + 8;
  *(int *)&(this->super_C_BaseTrigger).field_0x4 = *(int *)(extraout_ECX + 0x6279ac) + 0x3a4;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = *(int *)(extraout_ECX + 0x6279ac) + 0x45c;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = *(int *)(extraout_ECX + 0x6279ac) + 0x498;
  *(int *)&(this->super_C_BaseTrigger).field_0x10 = *(int *)(extraout_ECX + 0x6279ac) + 0x4b4;
  *(int *)&this->super_C_BaseTrigger = *(int *)(&DAT_00627958 + extraout_ECX) + 8;
  *(int *)&(this->super_C_BaseTrigger).field_0x4 = *(int *)(&DAT_00627958 + extraout_ECX) + 0x3a4;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = *(int *)(&DAT_00627958 + extraout_ECX) + 0x45c;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = *(int *)(&DAT_00627958 + extraout_ECX) + 0x498;
  *(int *)&(this->super_C_BaseTrigger).field_0x10 = *(int *)(&DAT_00627958 + extraout_ECX) + 0x4b4;
  C_BaseEntity::~C_BaseEntity((C_BaseEntity *)this,__in_chrg);
  return;
}


/* C_TriggerPaintCleanser::~C_TriggerPaintCleanser at 0053cb70 */

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
  *(int *)&this->super_C_BaseTrigger = extraout_ECX + 0x692510;
  *(int *)&(this->super_C_BaseTrigger).field_0x4 = extraout_ECX + 0x6928ac;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = extraout_ECX + 0x692964;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = extraout_ECX + 0x6929a0;
  *(int *)&(this->super_C_BaseTrigger).field_0x10 = extraout_ECX + 0x6929bc;
  *(int *)&this->super_C_BaseTrigger = *(int *)(extraout_ECX + 0x62773c) + 8;
  *(int *)&(this->super_C_BaseTrigger).field_0x4 = *(int *)(extraout_ECX + 0x62773c) + 0x3a4;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = *(int *)(extraout_ECX + 0x62773c) + 0x45c;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = *(int *)(extraout_ECX + 0x62773c) + 0x498;
  *(int *)&(this->super_C_BaseTrigger).field_0x10 = *(int *)(extraout_ECX + 0x62773c) + 0x4b4;
  *(int *)&this->super_C_BaseTrigger = *(int *)(extraout_ECX + 0x6276e8) + 8;
  *(int *)&(this->super_C_BaseTrigger).field_0x4 = *(int *)(extraout_ECX + 0x6276e8) + 0x3a4;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = *(int *)(extraout_ECX + 0x6276e8) + 0x45c;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = *(int *)(extraout_ECX + 0x6276e8) + 0x498;
  *(int *)&(this->super_C_BaseTrigger).field_0x10 = *(int *)(extraout_ECX + 0x6276e8) + 0x4b4;
  C_BaseEntity::~C_BaseEntity((C_BaseEntity *)this,__in_chrg);
  return;
}


/* C_TriggerPaintCleanser::~C_TriggerPaintCleanser at 0053cc50 */

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
  *(undefined **)&this->super_C_BaseTrigger = &UNK_0069242b + unaff_EBX;
  *(undefined **)&(this->super_C_BaseTrigger).field_0x4 = &UNK_006927c7 + unaff_EBX;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = unaff_EBX + 0x69287f;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = unaff_EBX + 0x6928bb;
  *(int *)&(this->super_C_BaseTrigger).field_0x10 = unaff_EBX + 0x6928d7;
  *(int *)&this->super_C_BaseTrigger = *(int *)(unaff_EBX + 0x627657) + 8;
  *(int *)&(this->super_C_BaseTrigger).field_0x4 = *(int *)(unaff_EBX + 0x627657) + 0x3a4;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = *(int *)(unaff_EBX + 0x627657) + 0x45c;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = *(int *)(unaff_EBX + 0x627657) + 0x498;
  *(int *)&(this->super_C_BaseTrigger).field_0x10 = *(int *)(unaff_EBX + 0x627657) + 0x4b4;
  *(int *)&this->super_C_BaseTrigger = *(int *)(unaff_EBX + 0x627603) + 8;
  *(int *)&(this->super_C_BaseTrigger).field_0x4 = *(int *)(unaff_EBX + 0x627603) + 0x3a4;
  *(int *)&(this->super_C_BaseTrigger).field_0x8 = *(int *)(unaff_EBX + 0x627603) + 0x45c;
  *(int *)&(this->super_C_BaseTrigger).field_0xc = *(int *)(unaff_EBX + 0x627603) + 0x498;
  *(int *)&(this->super_C_BaseTrigger).field_0x10 = *(int *)(unaff_EBX + 0x627603) + 0x4b4;
  C_BaseEntity::~C_BaseEntity((C_BaseEntity *)this,in_stack_ffffffe8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_TriggerPaintCleanser::Spawn at 0053c8c0 */

/* WARNING: Struct "C_TriggerPaintCleanser": ignoring overlapping field "m_bDisabled" */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void Spawn(C_TriggerPaintCleanser * this) */

void __thiscall C_TriggerPaintCleanser::Spawn(C_TriggerPaintCleanser *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::Spawn((C_BaseEntity *)this);
  CPaintCleanserManager::AddPaintCleanser(*(CPaintCleanserManager **)(unaff_EBX + 0x6276ab),this);
  return;
}


/* C_TriggerPaintCleanser::UpdateOnRemove at 0053c880 */

/* WARNING: Struct "C_TriggerPaintCleanser": ignoring overlapping field "m_bDisabled" */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void UpdateOnRemove(C_TriggerPaintCleanser * this) */

void __thiscall C_TriggerPaintCleanser::UpdateOnRemove(C_TriggerPaintCleanser *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CPaintCleanserManager::RemovePaintCleanser(*(CPaintCleanserManager **)(unaff_EBX + 0x6276eb),this)
  ;
  C_BaseEntity::UpdateOnRemove((C_BaseEntity *)this);
  return;
}


/* _GLOBAL__I__ZN22C_TriggerPaintCleanser17m_pClassRecvTableE at 000798d0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN22C_TriggerPaintCleanser17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

