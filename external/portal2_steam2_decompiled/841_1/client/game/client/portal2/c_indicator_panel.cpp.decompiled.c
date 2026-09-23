/* DWARF-guided pseudocode for game/client/portal2/c_indicator_panel.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* ClientClassInit<DT_LabIndicatorPanel::ignored> at 000738e0 */

int ClientClassInit<DT_LabIndicatorPanel::ignored>(ignored *param_1)

{
  RecvVarProxyFn varProxy;
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)s_pRandomFloats + unaff_EBX + 0x48a) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc5cb8a);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x492),
                  (char *)(unaff_EBX + 0x916e82),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x4ce),
                        (char *)(unaff_EBX + 0x91b9c6),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00af0542 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00af04fe + unaff_EBX));
      RecvPropBool((RecvProp *)((int)s_pRandomFloats + unaff_EBX + 0x50a),
                   (char *)(unaff_EBX + 0x92f4e6),0xa5d,1);
      RecvPropBool((RecvProp *)((int)s_pRandomFloats + unaff_EBX + 0x546),
                   (char *)(unaff_EBX + 0x9242d4),0xa5e,1);
      RecvPropBool((RecvProp *)((int)s_pRandomFloats + unaff_EBX + 0x582),
                   (char *)(unaff_EBX + 0x945b75),0xa5f,1);
      RecvPropBool((RecvProp *)((int)s_pRandomFloats + unaff_EBX + 0x5be),
                   (char *)(vgui::TextImage::GetUnlocalizedText + unaff_EBX + 5),0xa60,1);
      RecvPropBool((RecvProp *)((int)s_pRandomFloats + unaff_EBX + 0x5fa),
                   (char *)(unaff_EBX + 0x945b99),0xa61,1);
      RecvPropInt((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x636),
                  (char *)(unaff_EBX + 0x945ba6),0xa64,4,0,(RecvVarProxyFn)0x0);
      varProxy = *(RecvVarProxyFn *)(&DAT_00af053e + unaff_EBX);
      RecvPropFloat((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x672),
                    (char *)(unaff_EBX + 0x945bb6),0xa68,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x6ae),
                    (char *)(unaff_EBX + 0x945bc8),0xa6c,4,0,varProxy);
      ___cxa_guard_release(unaff_EBX + 0xc5cb8a);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x1a2a),
             (RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x4ce),9,
             (char *)(unaff_EBX + 0x945bd7));
  return 1;
}


/* __static_initialization_and_destruction_0 at 00073b80 */

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
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x1af) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x1b0) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x1b1) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x1b2) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x1b3) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x1b7) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x1bb) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x1bf) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x1c3) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x1c7) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x1cb) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x1cf) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x1d3) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x1d7) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x1db) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x1df) = 0x7f7fffff;
  *(undefined **)((int)s_pRandomFloats + unaff_EBX + 0x1e3) = &UNK_00af12f7 + unaff_EBX;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x1777) = unaff_EBX + 0x945949;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 5999) = unaff_EBX + 0x4a7fbf;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0x1773) = 0;
  this = (RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x1787);
  *(RecvTable **)((int)s_pParticlePtrs + unaff_EBX + 0x177b) = this;
  piVar1 = *(int **)(&DAT_00af0253 + unaff_EBX);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x177f) = *piVar1;
  *piVar1 = unaff_EBX + 0xcf432f;
  RecvTable::RecvTable(this);
  ___cxa_atexit(unaff_EBX + 0x907a5f,0,*(undefined4 *)(&DAT_00af01db + unaff_EBX));
  iVar2 = ClientClassInit<DT_LabIndicatorPanel::ignored>((ignored *)0x0);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x179b) = iVar2;
  return;
}


/* C_LabIndicatorPanel::YouForgotToImplementOrDeclareClientClass at 0051ba70 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_LabIndicatorPanel": ignoring multiple overlapping fields */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_LabIndicatorPanel *
   this) */

int __thiscall
C_LabIndicatorPanel::YouForgotToImplementOrDeclareClientClass(C_LabIndicatorPanel *this)

{
  return 0;
}


/* C_LabIndicatorPanel::GetClientClass at 0051ba90 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_LabIndicatorPanel": ignoring multiple overlapping fields */
/* DWARF original prototype: ClientClass * GetClientClass(C_LabIndicatorPanel * this) */

ClientClass * __thiscall C_LabIndicatorPanel::GetClientClass(C_LabIndicatorPanel *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x84c428);
}


/* _C_LabIndicatorPanel_CreateObject at 0051bb50 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_LabIndicatorPanel_CreateObject(int entnum,int serialNum)

{
  C_BaseEntity *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_LabIndicatorPanel * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xa70);
  C_BaseEntity::C_BaseEntity(this);
  (this->super_IClientEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x6ac9a4);
  (this->super_IClientEntity).super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6acd40);
  (this->super_IClientEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6acdf8);
  (this->super_IClientEntity).super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6ace34);
  (this->super_IClientModelRenderable)._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6ace50);
  (**(code **)(unaff_EBX + 0x6aca60))(this,entnum,serialNum);
  return &(this->super_IClientEntity).super_IClientNetworkable;
}


/* __tcf_0 at 0097b5f0 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3ec8e0),in_stack_00000008);
  return;
}


/* C_LabIndicatorPanel::GetCountdownPercentage at 0051baa0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_LabIndicatorPanel": ignoring multiple overlapping fields */
/* DWARF original prototype: float GetCountdownPercentage(C_LabIndicatorPanel * this) */

float __thiscall C_LabIndicatorPanel::GetCountdownPercentage(C_LabIndicatorPanel *this)

{
  float fVar1;
  int extraout_ECX;
  float fVar2;
  float fVar3;
  
  ___i686_get_pc_thunk_cx();
  if ((this->super_C_BaseEntity).field_0xa5f == '\0') {
    return 1.0;
  }
  fVar3 = this->m_flTimerStart;
  fVar2 = fVar3 + this->m_flTimerDuration;
  fVar1 = *(float *)(**(int **)(extraout_ECX + 0x6482f1) + 0xc);
  if ((fVar3 != fVar2) || (NAN(fVar3) || NAN(fVar2))) {
    fVar3 = (fVar1 - fVar3) / (fVar2 - fVar3);
    if ((fVar3 < *(float *)(extraout_ECX + 0x4c67d5)) &&
       (!NAN(fVar3) && !NAN(*(float *)(extraout_ECX + 0x4c67d5)))) {
      return 1.0;
    }
    if (fVar3 <= *(float *)(extraout_ECX + 0x4c66d9)) {
      return *(float *)(extraout_ECX + 0x4c66d9) - fVar3;
    }
  }
  else if (fVar1 - fVar2 < 0.0) {
    return 1.0;
  }
  return 0.0;
}


/* _GLOBAL__I__ZN19C_LabIndicatorPanel17m_pClassRecvTableE at 00073ca0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN19C_LabIndicatorPanel17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

