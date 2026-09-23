/* DWARF-guided pseudocode for game/client/portal2/c_prop_rockettripwire.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* ClientClassInit<DT_PropRocketTripwire::ignored> at 00079000 */

int ClientClassInit<DT_PropRocketTripwire::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)NPaintRenderer::g_FastSpherePosData[0x2f] + unaff_EBX + 4) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc59d04);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)NPaintRenderer::g_FastSpherePosData[0x2f] + unaff_EBX + 0xc),
                  (char *)(unaff_EBX + 0x91175c),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)NPaintRenderer::g_FastSpherePosData[0x31] + unaff_EBX + 8),
                        (char *)(unaff_EBX + 0x9162a0),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00aeade0 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00aeadd8 + unaff_EBX));
      RecvPropInt((RecvProp_conflict1 *)(&DAT_00c59d84 + unaff_EBX),(char *)(unaff_EBX + 0x942426),
                  0xd8c,4,0,(RecvVarProxyFn)0x0);
      RecvPropInt((RecvProp_conflict1 *)((int)&CSaveGamePanel::GetKBMap()::s_pMap + unaff_EBX),
                  (char *)(unaff_EBX + 0x94243a),0xd88,4,0,(RecvVarProxyFn)0x0);
      RecvPropBool((RecvProp *)(unaff_EBX + 0xc59dfc),(char *)(unaff_EBX + 0x942451),0xd90,1);
      RecvPropEHandle((RecvProp *)((int)&CCvarToggleCheckButton::GetKBMap()::s_pMap + unaff_EBX),
                      (char *)(unaff_EBX + 0x94245e),0xd94,4,
                      *(RecvVarProxyFn *)(&DAT_00aeadd4 + unaff_EBX));
      ___cxa_guard_release(unaff_EBX + 0xc59d04);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_SIMDRandContexts[0x1d].m_RandY[0x33] + unaff_EBX + 0xc),
             (RecvProp_conflict1 *)((int)NPaintRenderer::g_FastSpherePosData[0x31] + unaff_EBX + 8),
             5,(char *)(unaff_EBX + 0x942468));
  return 1;
}


/* __static_initialization_and_destruction_0 at 000791f0 */

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
  *(undefined1 *)((int)NPaintRenderer::g_FastSpherePosData[0x1d] + unaff_EBX + 0x1f) = 0;
  *(undefined1 *)((int)NPaintRenderer::g_FastSpherePosData[0x1e] + unaff_EBX) = 0;
  *(undefined1 *)((int)NPaintRenderer::g_FastSpherePosData[0x1e] + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)NPaintRenderer::g_FastSpherePosData[0x1e] + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0x1e] + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0x1e] + unaff_EBX + 7) = 0;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0x1e] + unaff_EBX + 0xb) = 0x7f7fffff;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0x1e] + unaff_EBX + 0xf) = 0x7f7fffff;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0x1e] + unaff_EBX + 0x13) = 0;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0x1e] + unaff_EBX + 0x17) = 0;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0x1e] + unaff_EBX + 0x1b) = 0;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0x1e] + unaff_EBX + 0x1f) = 0;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0x1f] + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0x1f] + unaff_EBX + 7) = 0x7f7fffff;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0x1f] + unaff_EBX + 0xb) = 0x7f7fffff;
  *(undefined4 *)((int)NPaintRenderer::g_FastSpherePosData[0x1f] + unaff_EBX + 0xf) = 0x7f7fffff;
  *(undefined **)((int)NPaintRenderer::g_FastSpherePosData[0x1f] + unaff_EBX + 0x13) =
       &UNK_00aebc87 + unaff_EBX;
  *(undefined **)((int)s_SIMDRandContexts[0x1d].m_RandY[0x13] + unaff_EBX + 0xf) =
       &UNK_00942291 + unaff_EBX;
  *(int *)((int)s_SIMDRandContexts[0x1d].m_RandY[0x13] + unaff_EBX + 7) = unaff_EBX + 0x4c15df;
  *(undefined4 *)((int)s_SIMDRandContexts[0x1d].m_RandY[0x13] + unaff_EBX + 0xb) = 0;
  this = (RecvTable *)((int)s_SIMDRandContexts[0x1d].m_RandY[0x14] + unaff_EBX + 0xf);
  *(RecvTable **)((int)s_SIMDRandContexts[0x1d].m_RandY[0x14] + unaff_EBX + 3) = this;
  piVar1 = *(int **)(&DAT_00aeabe3 + unaff_EBX);
  *(int *)((int)s_SIMDRandContexts[0x1d].m_RandY[0x14] + unaff_EBX + 7) = *piVar1;
  *piVar1 = unaff_EBX + 0xcf03b7;
  RecvTable::RecvTable(this);
  ___cxa_atexit(unaff_EBX + 0x9030ff,0,*(undefined4 *)(&DAT_00aeab6b + unaff_EBX));
  iVar2 = ClientClassInit<DT_PropRocketTripwire::ignored>((ignored *)0x0);
  *(int *)((int)s_SIMDRandContexts[0x1d].m_RandY[0x16] + unaff_EBX + 3) = iVar2;
  return;
}


/* C_PropRocketTripwire::YouForgotToImplementOrDeclareClientClass at 00539e30 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_PropRocketTripwire *
   this) */

int __thiscall
C_PropRocketTripwire::YouForgotToImplementOrDeclareClientClass(C_PropRocketTripwire *this)

{
  return 0;
}


/* C_PropRocketTripwire::GetClientClass at 00539e50 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: ClientClass * GetClientClass(C_PropRocketTripwire * this) */

ClientClass * __thiscall C_PropRocketTripwire::GetClientClass(C_PropRocketTripwire *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x82f760);
}


/* _C_PropRocketTripwire_CreateObject at 0053a7e0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_PropRocketTripwire_CreateObject(int entnum,int serialNum)

{
  C_BaseAnimating *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_PropRocketTripwire * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xdac);
  C_BaseAnimating::C_BaseAnimating(this);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x693d5b);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6941b7);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x69426f);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6942ab);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6942c7);
  this[1].super_C_BaseEntity.super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)0xffffffff;
  CTraceFilterSkipClassname::CTraceFilterSkipClassname
            ((CTraceFilterSkipClassname *)&this[1].super_C_BaseEntity.m_iClassname,
             (IHandleEntity *)this,(char *)(unaff_EBX + 0x4654ef),1);
  (*(this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x2f])(this,entnum,serialNum);
  return &(this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable;
}


/* __tcf_0 at 0097c300 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3ed2c8),in_stack_00000008);
  return;
}


/* C_PropRocketTripwire::C_PropRocketTripwire at 0053a0b0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void C_PropRocketTripwire(C_PropRocketTripwire * this) */

void __thiscall C_PropRocketTripwire::C_PropRocketTripwire(C_PropRocketTripwire *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseAnimating::C_BaseAnimating(&this->super_C_BaseAnimating);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x694484);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x6948e0);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(&UNK_00694998 + unaff_EBX);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x6949d4);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(unaff_EBX + 0x6949f0);
  (this->m_hRocket).super_CBaseHandle.m_Index = 0xffffffff;
  CTraceFilterSkipClassname::CTraceFilterSkipClassname
            (&this->m_filterBeams,(IHandleEntity *)this,(char *)(unaff_EBX + 0x465c18),1);
  return;
}


/* C_PropRocketTripwire::C_PropRocketTripwire at 0053a170 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void C_PropRocketTripwire(C_PropRocketTripwire * this) */

void __thiscall C_PropRocketTripwire::C_PropRocketTripwire(C_PropRocketTripwire *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseAnimating::C_BaseAnimating(&this->super_C_BaseAnimating);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6943c4);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(CBaseWaterView::~CBaseWaterView + unaff_EBX);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x6948d8);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(&UNK_00694914 + unaff_EBX);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(unaff_EBX + 0x694930);
  (this->m_hRocket).super_CBaseHandle.m_Index = 0xffffffff;
  CTraceFilterSkipClassname::CTraceFilterSkipClassname
            (&this->m_filterBeams,(IHandleEntity *)this,(char *)(unaff_EBX + 0x465b58),1);
  return;
}


/* C_PropRocketTripwire::~C_PropRocketTripwire at 00539ed0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void ~C_PropRocketTripwire(C_PropRocketTripwire * this, int __in_chrg)
    */

void __thiscall
C_PropRocketTripwire::~C_PropRocketTripwire(C_PropRocketTripwire *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x694664);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x694ac0);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(&UNK_00694b78 + unaff_EBX);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x694bb4);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(unaff_EBX + 0x694bd0);
  if (this->m_pBeam != (C_Beam *)0x0) {
    C_BaseEntity::Remove(&this->m_pBeam->super_C_BaseEntity);
  }
  C_BaseAnimating::~C_BaseAnimating(&this->super_C_BaseAnimating,in_stack_ffffffd8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_PropRocketTripwire::~C_PropRocketTripwire at 00539f70 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void ~C_PropRocketTripwire(C_PropRocketTripwire * this, int __in_chrg)
    */

void __thiscall
C_PropRocketTripwire::~C_PropRocketTripwire(C_PropRocketTripwire *this,int __in_chrg)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6945c4);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable =
       (_func_int_varargs **)(CReflectiveGlassView::~CReflectiveGlassView + unaff_EBX);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x694ad8);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x694b14);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(&UNK_00694b30 + unaff_EBX);
  if (this->m_pBeam != (C_Beam *)0x0) {
    C_BaseEntity::Remove(&this->m_pBeam->super_C_BaseEntity);
  }
  C_BaseAnimating::~C_BaseAnimating(&this->super_C_BaseAnimating,__in_chrg);
  return;
}


/* C_PropRocketTripwire::~C_PropRocketTripwire at 0053a010 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void ~C_PropRocketTripwire(C_PropRocketTripwire * this, int __in_chrg)
    */

void __thiscall
C_PropRocketTripwire::~C_PropRocketTripwire(C_PropRocketTripwire *this,int __in_chrg)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x694524);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(&UNK_00694980 + unaff_EBX);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x694a38);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x694a74);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(unaff_EBX + 0x694a90);
  if (this->m_pBeam != (C_Beam *)0x0) {
    C_BaseEntity::Remove(&this->m_pBeam->super_C_BaseEntity);
  }
  C_BaseAnimating::~C_BaseAnimating(&this->super_C_BaseAnimating,__in_chrg);
  return;
}


/* C_PropRocketTripwire::Spawn at 00539e70 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(C_PropRocketTripwire * this) */

void __thiscall C_PropRocketTripwire::Spawn(C_PropRocketTripwire *this)

{
  C_BaseEntity::ThinkSet((C_BaseEntity *)this,(BASEPTR)0x155,0.0,(char *)0x0);
  (*(this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x9d])(this,0xc4a1a000);
  this->m_pBeam = (C_Beam *)0x0;
  C_BaseEntity::Spawn((C_BaseEntity *)this);
  return;
}


/* C_PropRocketTripwire::ClientThink at 0053a240 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void ClientThink(C_PropRocketTripwire * this) */

void __thiscall C_PropRocketTripwire::ClientThink(C_PropRocketTripwire *this)

{
  int *piVar1;
  float fVar2;
  uint uVar3;
  bool bVar4;
  int iVar5;
  IHandleEntity *pIVar6;
  C_Portal_Base2D *pCVar7;
  C_Beam *pCVar8;
  int unaff_EBX;
  Ray_t local_fc;
  float local_a4;
  float local_a0;
  float local_9c;
  float local_98;
  float local_94;
  float local_90;
  float local_8c;
  float local_88;
  float local_84;
  float local_80;
  float local_7c;
  float local_78;
  float local_74;
  float local_70;
  float local_6c;
  float local_68;
  float local_64;
  float local_60;
  Vector local_5c;
  Vector local_50;
  Vector local_44;
  QAngle local_38;
  Vector local_2c;
  float local_20 [4];
  
                    /* Unresolved local var: Vector vecMuzzle@[???]
                       Unresolved local var: QAngle angMuzzleDir@[???]
                       Unresolved local var: Vector vecAimDir@[???]
                       Unresolved local var: Vector vEndPoint@[???]
                       Unresolved local var: float fEndFraction@[???]
                       Unresolved local var: Ray_t rayPath@[???] */
  ___i686_get_pc_thunk_bx();
  bVar4 = C_BaseAnimating::IsBoneAccessAllowed(&this->super_C_BaseAnimating);
  if (bVar4) {
    (*(this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
      super_IHandleEntity._vptr_IHandleEntity[0x75])
              (this,this->m_iMuzzleAttachment,&local_2c,&local_38);
    AngleVectors(&local_38,&local_44);
    pCVar8 = this->m_pBeam;
    if (pCVar8 == (C_Beam *)0x0) {
      pCVar8 = C_Beam::BeamCreate((char *)(unaff_EBX + 0x47bcd6),0.1);
      this->m_pBeam = pCVar8;
      if (this->m_nBeamEndpointTexture != (pCVar8->m_nHaloIndex).m_Value) {
        (pCVar8->m_nHaloIndex).m_Value = this->m_nBeamEndpointTexture;
        pCVar8 = this->m_pBeam;
      }
      if ((pCVar8->super_C_BaseEntity).m_clrRender.
          super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.r != 'd') {
        (pCVar8->super_C_BaseEntity).m_clrRender.
        super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.r = 'd';
      }
      if ((pCVar8->super_C_BaseEntity).m_clrRender.
          super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.g != 'd') {
        (pCVar8->super_C_BaseEntity).m_clrRender.
        super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.g = 'd';
      }
      if ((pCVar8->super_C_BaseEntity).m_clrRender.
          super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.b != 0xff) {
        (pCVar8->super_C_BaseEntity).m_clrRender.
        super_CNetworkVarBase<color32_s,C_BaseEntity::NetworkVar_m_clrRender>.m_Value.b = 0xff;
      }
      C_BaseEntity::SetRenderAlpha(&this->m_pBeam->super_C_BaseEntity,'d');
      pCVar8 = this->m_pBeam;
      if ((pCVar8->m_fAmplitude).m_Value != 0.0) {
        (pCVar8->m_fAmplitude).m_Value = 0.0;
        pCVar8 = this->m_pBeam;
      }
      if (*(float *)(unaff_EBX + 0x4a7f33) != (pCVar8->m_fWidth).m_Value) {
        (pCVar8->m_fWidth).m_Value = 1.0;
        pCVar8 = this->m_pBeam;
      }
      if ((pCVar8->m_fEndWidth).m_Value != 0.0) {
        (pCVar8->m_fEndWidth).m_Value = 0.0;
        pCVar8 = this->m_pBeam;
      }
      if ((pCVar8->m_fSpeed).m_Value != 0.0) {
        (pCVar8->m_fSpeed).m_Value = 0.0;
        pCVar8 = this->m_pBeam;
      }
      if ((pCVar8->m_fFadeLength).m_Value != 0.0) {
        (pCVar8->m_fFadeLength).m_Value = 0.0;
        pCVar8 = this->m_pBeam;
      }
      if (*(float *)(unaff_EBX + 0x4a8033) != (pCVar8->m_fHaloScale).m_Value) {
        (pCVar8->m_fHaloScale).m_Value = *(float *)(unaff_EBX + 0x4a8033);
        pCVar8 = this->m_pBeam;
      }
      C_BaseEntity::SetCollisionGroup(&pCVar8->super_C_BaseEntity,0);
      C_Beam::SetBeamFlag(this->m_pBeam,0x10000);
                    /* Unresolved local var: Vector res@[???] */
      local_50.x = local_2c.x + local_44.x;
      local_50.y = local_2c.y + local_44.y;
      local_50.z = local_2c.z + local_44.z;
      C_Beam::PointsInit(this->m_pBeam,&local_50,&local_2c);
      C_Beam::SetStartEntity(this->m_pBeam,(C_BaseEntity *)this);
      pCVar8 = this->m_pBeam;
      if (*(float *)(unaff_EBX + 0x4a7f33) != (pCVar8->m_fWidth).m_Value) {
        (pCVar8->m_fWidth).m_Value = 1.0;
        pCVar8 = this->m_pBeam;
      }
      if ((pCVar8->m_fSpeed).m_Value != 0.0) {
        (pCVar8->m_fSpeed).m_Value = 0.0;
        pCVar8 = this->m_pBeam;
      }
      if (*(float *)(unaff_EBX + 0x4a803b) != (pCVar8->m_fHaloScale).m_Value) {
        (pCVar8->m_fHaloScale).m_Value = *(float *)(unaff_EBX + 0x4a803b);
      }
    }
    else if (this->m_bDrawBeams == false) {
      C_BaseEntity::AddEffects(&pCVar8->super_C_BaseEntity,0x20);
    }
    else {
      piVar1 = &(pCVar8->super_C_BaseEntity).m_fEffects;
      *piVar1 = *piVar1 & 0xffffffdf;
      C_BaseEntity::UpdateVisibility(&pCVar8->super_C_BaseEntity);
      C_BaseEntity::OnDisableShadowDepthRenderingChanged(&pCVar8->super_C_BaseEntity);
      C_BaseEntity::OnShadowDepthRenderingCacheableStateChanged(&pCVar8->super_C_BaseEntity);
    }
                    /* Unresolved local var: Vector res@[???] */
    fVar2 = *(float *)(&DAT_004ac6ff + unaff_EBX);
    local_68 = local_44.x * fVar2;
    local_64 = local_44.y * fVar2;
    local_60 = local_44.z * fVar2;
                    /* Unresolved local var: Vector res@[???] */
    local_74 = local_2c.x + local_68;
    local_70 = local_2c.y + local_64;
    local_6c = local_2c.z + local_60;
    local_fc.m_Delta.super_Vector.x = local_74 - local_2c.x;
    local_fc.m_Delta.super_Vector.y = local_70 - local_2c.y;
    local_fc.m_Delta.super_Vector.z = local_6c - local_2c.z;
    local_fc.m_IsSwept =
         local_fc.m_Delta.super_Vector.x * local_fc.m_Delta.super_Vector.x +
         local_fc.m_Delta.super_Vector.y * local_fc.m_Delta.super_Vector.y +
         local_fc.m_Delta.super_Vector.z * local_fc.m_Delta.super_Vector.z != 0.0;
    local_fc.m_Extents.super_Vector.z = 0.0;
    local_fc.m_Extents.super_Vector.y = 0.0;
    local_fc.m_Extents.super_Vector.x = 0.0;
    local_fc.m_pWorldAxisTransform = (matrix3x4_t *)0x0;
    local_fc.m_IsRay = true;
    local_fc.m_StartOffset.super_Vector.z = 0.0;
    local_fc.m_StartOffset.super_Vector.y = 0.0;
    local_fc.m_StartOffset.super_Vector.x = 0.0;
    local_fc.m_Start.super_Vector.x = local_2c.x;
    local_fc.m_Start.super_Vector.y = local_2c.y;
    local_fc.m_Start.super_Vector.z = local_2c.z;
    uVar3 = (this->m_hRocket).super_CBaseHandle.m_Index;
    if (((uVar3 == 0xffffffff) ||
        (iVar5 = (uVar3 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x629b8b),
        *(uint *)(iVar5 + 8) != uVar3 >> 0x10)) ||
       (pIVar6 = *(IHandleEntity **)(iVar5 + 4), pIVar6 == (IHandleEntity *)0x0)) {
      pIVar6 = (IHandleEntity *)0x0;
    }
    (this->m_filterBeams).super_CTraceFilterSimple.m_pPassEnt = pIVar6;
    pCVar7 = UTIL_Portal_TraceRay_Beam
                       (&local_fc,0x2006081,(ITraceFilter *)&this->m_filterBeams,local_20);
    if (pCVar7 == (C_Portal_Base2D *)0x0) {
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
      local_98 = fVar2 * local_44.x * local_20[0];
      local_94 = local_44.y * fVar2 * local_20[0];
      local_90 = local_20[0] * local_44.z * fVar2;
                    /* Unresolved local var: Vector res@[???] */
      local_a4 = local_2c.x + local_98;
      local_a0 = local_2c.y + local_94;
      local_9c = local_2c.z + local_90;
      local_5c.x = local_a4;
      local_5c.y = local_a0;
      local_5c.z = local_9c;
    }
    else {
                    /* Unresolved local var: Vector res@[???] */
      local_80 = local_44.x * fVar2;
      local_7c = local_44.y * fVar2;
      local_78 = fVar2 * local_44.z;
                    /* Unresolved local var: Vector res@[???] */
      local_8c = local_2c.x + local_80;
      local_88 = local_2c.y + local_7c;
      local_84 = local_2c.z + local_78;
      local_5c.x = local_8c;
      local_5c.y = local_88;
      local_5c.z = local_84;
    }
    C_Beam::PointsInit(this->m_pBeam,&local_5c,&local_2c);
    return;
  }
  return;
}


/* C_PropRocketTripwire::GetBeamTraceFilter at 00539e60 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: ITraceFilter * GetBeamTraceFilter(C_PropRocketTripwire * this) */

ITraceFilter * __thiscall C_PropRocketTripwire::GetBeamTraceFilter(C_PropRocketTripwire *this)

{
  return (ITraceFilter *)&this->m_filterBeams;
}


/* _GLOBAL__I__ZN20C_PropRocketTripwire17m_pClassRecvTableE at 00079310 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN20C_PropRocketTripwire17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

