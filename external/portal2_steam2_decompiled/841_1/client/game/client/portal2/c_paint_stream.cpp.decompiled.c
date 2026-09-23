/* DWARF-guided pseudocode for game/client/portal2/c_paint_stream.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 00076530 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  RecvTable *this;
  undefined4 uVar1;
  undefined4 *puVar2;
  int iVar3;
  IClassMap *pIVar4;
  int unaff_EBX;
  longlong lVar5;
  ConVar *in_stack_ffffffd4;
  ConVar *in_stack_ffffffd8;
  char *in_stack_ffffffdc;
  char *in_stack_ffffffe0;
  int in_stack_ffffffe4;
  char *in_stack_ffffffe8;
  
  lVar5 = ___i686_get_pc_thunk_bx();
  if (lVar5 == 0xffff00000001) {
    (&DAT_00c5b05c)[unaff_EBX] = 0;
    (&DAT_00c5b05d)[unaff_EBX] = 0;
    (&DAT_00c5b05e)[unaff_EBX] = 0;
    (&DAT_00c5b05f)[unaff_EBX] = 0;
    *(undefined4 *)((int)&CCenterStringLabel::GetKBMap()::s_pMap + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00c5b064 + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00c5b068 + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00c5b06c + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&CCenterStringLabel::GetKBMap::s_pMap + unaff_EBX) = 0;
    *(undefined4 *)((int)&CCenterStringLabel::GetMessageMap::s_pMap + unaff_EBX) = 0;
    *(undefined4 *)(&CCenterStringLabel::ChainToMap()::chained + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&CCenterStringLabel::PanelMessageFunc_OnScreenSizeChanged::InitVar()::bAdded +
     unaff_EBX + 1) = 0;
    *(undefined4 *)(&DAT_00c5b080 + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00c5b084 + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&CFPSPanel::GetMessageMap()::s_pMap + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00c5b08c + unaff_EBX) = 0x7f7fffff;
    *(undefined **)((int)&CFPSPanel::GetKBMap()::s_pMap + unaff_EBX) = &UNK_00aee944 + unaff_EBX;
    ConVar::ConVar((ConVar *)
                   (s_ParticleSystemMgr.m_VisualizedOperatorId.m_Value + unaff_EBX + -0x24),
                   (char *)(unaff_EBX + 0x943fe1),(char *)(unaff_EBX + 0x9155e8),0,in_stack_ffffffd4
                   ,(char *)in_stack_ffffffd8,in_stack_ffffffdc,(int)in_stack_ffffffe0);
    uVar1 = *(undefined4 *)(&DAT_00aed828 + unaff_EBX);
    ___cxa_atexit(unaff_EBX + 0x9058cc,0,uVar1);
    ConVar::ConVar((ConVar *)
                   ((int)(&s_ParticleSystemMgr.m_VisualizedOperatorId + 3) + unaff_EBX + 0xc),
                   (char *)(unaff_EBX + 0x943ff9),(char *)(unaff_EBX + 0x90c978),0,in_stack_ffffffd4
                   ,(char *)in_stack_ffffffd8,in_stack_ffffffdc,(int)in_stack_ffffffe0);
    ___cxa_atexit(&UNK_009058ac + unaff_EBX,0,uVar1);
    ConVar::ConVar((ConVar *)((int)&vec4_origin.w + unaff_EBX),(char *)(unaff_EBX + 0x944018),
                   (char *)(unaff_EBX + 0x944011),0,in_stack_ffffffd4,(char *)in_stack_ffffffd8,
                   in_stack_ffffffdc,(int)in_stack_ffffffe0);
    ___cxa_atexit(unaff_EBX + 0x90588c,0,uVar1);
    ConVar::ConVar((ConVar *)((int)g_nKillBufferInUse + unaff_EBX + 0x1c),
                   (char *)(unaff_EBX + 0x944040),(char *)(unaff_EBX + 0x944039),0,in_stack_ffffffd4
                   ,(char *)in_stack_ffffffd8,in_stack_ffffffdc,(int)in_stack_ffffffe0);
    ___cxa_atexit(unaff_EBX + 0x90586c,0,uVar1);
    ConVar::ConVar((ConVar *)((int)g_pKillBuffers + unaff_EBX + 0x3c),(char *)(unaff_EBX + 0x9440e4)
                   ,(char *)(unaff_EBX + 0x91ba78),2,(char *)(unaff_EBX + 0x944060),
                   in_stack_ffffffd8,in_stack_ffffffdc,in_stack_ffffffe0,in_stack_ffffffe4,
                   in_stack_ffffffe8);
    ___cxa_atexit(unaff_EBX + 0x90584c,0,uVar1);
    *(int *)(&DAT_00cf2b7c + unaff_EBX) = unaff_EBX + 0x944108;
    *(int *)(&DAT_00cf2b74 + unaff_EBX) = unaff_EBX + 0x4b111c;
    *(undefined4 *)(&DAT_00cf2b78 + unaff_EBX) = 0;
    this = (RecvTable *)((int)&vec2_invalid.x + unaff_EBX);
    *(RecvTable **)(UNSPECIFIED_LOGGING_COLOR._color + unaff_EBX) = this;
    puVar2 = *(undefined4 **)(&DAT_00aed8a0 + unaff_EBX);
    *(undefined4 *)((int)&vec2_origin.x + unaff_EBX) = *puVar2;
    *puVar2 = &DAT_00cf2b74 + unaff_EBX;
    RecvTable::RecvTable(this);
    ___cxa_atexit(unaff_EBX + 0x90582c,0,uVar1);
    iVar3 = ClientClassInit<DT_PaintStream::ignored>((ignored *)0x0);
    *(int *)((int)&vec4_origin.w + unaff_EBX) = iVar3;
    pIVar4 = GetClassMap();
    (*pIVar4->_vptr_IClassMap[2])
              (pIVar4,unaff_EBX + 0x944123,unaff_EBX + 0x944115,0xe38,unaff_EBX + 0x4b10dc);
  }
  return;
}


/* __tcf_0 at 0097be10 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ed0c8),in_stack_00000008);
  return;
}


/* __tcf_1 at 0097bdf0 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ed148),in_stack_00000008);
  return;
}


/* __tcf_2 at 0097bdd0 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(&DAT_003ed1c8 + extraout_ECX),in_stack_00000008);
  return;
}


/* __tcf_3 at 0097bdb0 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ed248),in_stack_00000008);
  return;
}


/* __tcf_4 at 0097bd90 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ed2c8),in_stack_00000008);
  return;
}


/* ClientClassInit<DT_PaintStream::ignored> at 000763a0 */

int ClientClassInit<DT_PaintStream::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&DAT_00c5b224)[unaff_EBX] == '\0') {
    iVar1 = ___cxa_guard_acquire(&DAT_00c5b224 + unaff_EBX);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)(unaff_EBX + 0xc5b22c),(char *)(unaff_EBX + 0x9143bc),0,4,0,
                  (RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)((int)&SweepRenderer::marginNCubes + unaff_EBX),
                        &UNK_00918f00 + unaff_EBX,0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00aeda40 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00aeda38 + unaff_EBX));
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_DefaultSheetSequence.m_TextureCoordData[0].m_fTop_V0 + unaff_EBX),
                  (char *)(unaff_EBX + 0x944151),0xdd0,4,0,(RecvVarProxyFn)0x0);
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)(s_DefaultSheetSequence.m_TextureCoordData + 2) + unaff_EBX),
                  (char *)(unaff_EBX + 0x9283a4),0xdd4,4,0,(RecvVarProxyFn)0x0);
      ___cxa_guard_release(&DAT_00c5b224 + unaff_EBX);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x15c),
             (RecvProp_conflict1 *)((int)&SweepRenderer::marginNCubes + unaff_EBX),3,
             (char *)(unaff_EBX + 0x944160));
  return 1;
}


/* C_PaintStream::YouForgotToImplementOrDeclareClientClass at 00526e70 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_PaintStream * this) */

int __thiscall C_PaintStream::YouForgotToImplementOrDeclareClientClass(C_PaintStream *this)

{
  return 0;
}


/* C_PaintStream::GetClientClass at 00526e90 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: ClientClass * GetClientClass(C_PaintStream * this) */

ClientClass * __thiscall C_PaintStream::GetClientClass(C_PaintStream *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x842220);
}


/* _C_PaintStream_CreateObject at 00527660 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */

IClientNetworkable * _C_PaintStream_CreateObject(int entnum,int serialNum)

{
  C_PaintStream *this;
  IClientNetworkable *pIVar1;
  
                    /* Unresolved local var: C_PaintStream * pRet@[???] */
  this = C_BaseEntity::operator_new(0xe38);
  C_PaintStream::C_PaintStream(this);
  pIVar1 = (IClientNetworkable *)0x0;
  if (this != (C_PaintStream *)0x0) {
    (*(this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
      super_IHandleEntity._vptr_IHandleEntity[0x2f])(this,entnum,serialNum);
    pIVar1 = &(this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.
              super_IClientNetworkable;
  }
  return pIVar1;
}


/* __tcf_5 at 0097bd70 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3ed358),in_stack_00000008);
  return;
}


/* CC_PaintStreamFactory at 00527620 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */

C_BaseEntity * CC_PaintStreamFactory(void)

{
  C_PaintStream *this;
  
  this = C_BaseEntity::operator_new(0xe38);
  C_PaintStream::C_PaintStream(this);
  return (C_BaseEntity *)this;
}


/* C_PaintStream::C_PaintStream at 005273b0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void C_PaintStream(C_PaintStream * this) */

void __thiscall C_PaintStream::C_PaintStream(C_PaintStream *this)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  undefined4 *puVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseAnimating::C_BaseAnimating(&this->super_C_BaseAnimating);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6a3daa);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x6a420a);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable =
       (_func_int_varargs **)(CNonMemberScriptBinding0<float_(*)(),float>::Call + unaff_EBX + 2);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(&UNK_006a42fe + unaff_EBX);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(unaff_EBX + 0x6a431a);
  (this->m_blobs).m_Memory.m_pMemory = (CBasePaintBlob **)0x0;
  (this->m_blobs).m_Memory.m_nAllocationCount = 0;
  (this->m_blobs).m_Memory.m_nGrowSize = 0;
  (this->m_blobs).m_Size = 0;
  (this->m_blobs).m_pElements = (CBasePaintBlob **)0x0;
  (this->m_sharedBlobData).
  super_CSharedVarBase<CUtlVector<BlobDataPerUpdate_t,_CUtlMemory<BlobDataPerUpdate_t,_int>_>,C_PaintStream::SharedVar_m_sharedBlobData>
  .m_pSharedMemory = (ISPSharedMemory *)0x0;
  (this->m_sharedBlobData).
  super_CSharedVarBase<CUtlVector<BlobDataPerUpdate_t,_CUtlMemory<BlobDataPerUpdate_t,_int>_>,C_PaintStream::SharedVar_m_sharedBlobData>
  .m_pValue = (CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t,_int>_> *)0x0;
  (this->m_sharedBlobData).
  super_CSharedVarBase<CUtlVector<BlobDataPerUpdate_t,_CUtlMemory<BlobDataPerUpdate_t,_int>_>,C_PaintStream::SharedVar_m_sharedBlobData>
  .super_ISharedVarBase._vptr_ISharedVarBase = (_func_int_varargs **)(unaff_EBX + 0x6a434a);
  this->m_nCurrentUpdateFrame = 0;
  C_BaseEntity::RemoveFromLeafSystem((C_BaseEntity *)this);
  uVar1 = (*(CUtlVector<C_PaintStream*,CUtlMemory<C_PaintStream*,_int>_> **)(unaff_EBX + 0x63cbce))
          ->m_Size;
  CUtlVector<C_PaintStream*,CUtlMemory<C_PaintStream*,_int>_>::GrowVector
            (*(CUtlVector<C_PaintStream*,CUtlMemory<C_PaintStream*,_int>_> **)(unaff_EBX + 0x63cbce)
             ,1);
  iVar3 = ~uVar1 + (*(int **)(unaff_EBX + 0x63cbce))[3];
  if (0 < iVar3) {
    iVar2 = **(int **)(unaff_EBX + 0x63cbce);
    _V_memmove((void *)(iVar2 + 4 + uVar1 * 4),(void *)(iVar2 + uVar1 * 4),iVar3 * 4);
  }
  puVar4 = (undefined4 *)(uVar1 * 4 + **(int **)(unaff_EBX + 0x63cbce));
  if (puVar4 != (undefined4 *)0x0) {
    *puVar4 = this;
  }
  return;
}


/* C_PaintStream::C_PaintStream at 00527610 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void C_PaintStream(C_PaintStream * this, C_PaintStream * this) */

void __thiscall C_PaintStream::C_PaintStream(C_PaintStream *this,C_PaintStream *this_1)

{
  C_PaintStream(this);
  return;
}


/* C_PaintStream::~C_PaintStream at 005276c0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void ~C_PaintStream(C_PaintStream * this, int __in_chrg) */

void __thiscall C_PaintStream::~C_PaintStream(C_PaintStream *this,int __in_chrg)

{
  ISPSharedMemory *pIVar1;
  CBasePaintBlob **ppCVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6a3a9a);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x6a3efa);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x6a3fb2);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(&UNK_006a3fee + unaff_EBX);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(&UNK_006a400a + unaff_EBX);
  RemoveAllPaintBlobs(this);
  (this->m_sharedBlobData).
  super_CSharedVarBase<CUtlVector<BlobDataPerUpdate_t,_CUtlMemory<BlobDataPerUpdate_t,_int>_>,C_PaintStream::SharedVar_m_sharedBlobData>
  .super_ISharedVarBase._vptr_ISharedVarBase = (_func_int_varargs **)(unaff_EBX + 0x6a407a);
  pIVar1 = (this->m_sharedBlobData).
           super_CSharedVarBase<CUtlVector<BlobDataPerUpdate_t,_CUtlMemory<BlobDataPerUpdate_t,_int>_>,C_PaintStream::SharedVar_m_sharedBlobData>
           .m_pSharedMemory;
  if (pIVar1 != (ISPSharedMemory *)0x0) {
    (*pIVar1->_vptr_ISPSharedMemory[4])(pIVar1);
    (this->m_sharedBlobData).
    super_CSharedVarBase<CUtlVector<BlobDataPerUpdate_t,_CUtlMemory<BlobDataPerUpdate_t,_int>_>,C_PaintStream::SharedVar_m_sharedBlobData>
    .m_pSharedMemory = (ISPSharedMemory *)0x0;
  }
  (this->m_sharedBlobData).
  super_CSharedVarBase<CUtlVector<BlobDataPerUpdate_t,_CUtlMemory<BlobDataPerUpdate_t,_int>_>,C_PaintStream::SharedVar_m_sharedBlobData>
  .m_pValue = (CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t,_int>_> *)0x0;
  (this->m_blobs).m_Size = 0;
  if ((this->m_blobs).m_Memory.m_nGrowSize < 0) {
    ppCVar2 = (this->m_blobs).m_Memory.m_pMemory;
  }
  else {
    ppCVar2 = (this->m_blobs).m_Memory.m_pMemory;
    if (ppCVar2 != (CBasePaintBlob **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x63c6a2) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x63c6a2),ppCVar2);
      (this->m_blobs).m_Memory.m_pMemory = (CBasePaintBlob **)0x0;
    }
    ppCVar2 = (CBasePaintBlob **)0x0;
    (this->m_blobs).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_blobs).m_pElements = ppCVar2;
  if (-1 < (this->m_blobs).m_Memory.m_nGrowSize) {
    if (ppCVar2 != (CBasePaintBlob **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x63c6a2) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x63c6a2),ppCVar2);
      (this->m_blobs).m_Memory.m_pMemory = (CBasePaintBlob **)0x0;
    }
    (this->m_blobs).m_Memory.m_nAllocationCount = 0;
  }
  C_BaseAnimating::~C_BaseAnimating(&this->super_C_BaseAnimating,__in_chrg);
  return;
}


/* C_PaintStream::~C_PaintStream at 005278e0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void ~C_PaintStream(C_PaintStream * this, int __in_chrg, C_PaintStream
   * this) */

void __thiscall
C_PaintStream::~C_PaintStream(C_PaintStream *this,int __in_chrg,C_PaintStream *this_1)

{
  ~C_PaintStream(this,__in_chrg);
  return;
}


/* C_PaintStream::~C_PaintStream at 005278f0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void ~C_PaintStream(C_PaintStream * this, int __in_chrg) */

void __thiscall C_PaintStream::~C_PaintStream(C_PaintStream *this,int __in_chrg)

{
  ISPSharedMemory *pIVar1;
  CBasePaintBlob **ppCVar2;
  int unaff_EBX;
  CBasePaintBlob **in_stack_ffffffc8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6a386a);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x6a3cca);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x6a3d82);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x6a3dbe);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(unaff_EBX + 0x6a3dda);
  RemoveAllPaintBlobs(this);
  (this->m_sharedBlobData).
  super_CSharedVarBase<CUtlVector<BlobDataPerUpdate_t,_CUtlMemory<BlobDataPerUpdate_t,_int>_>,C_PaintStream::SharedVar_m_sharedBlobData>
  .super_ISharedVarBase._vptr_ISharedVarBase = (_func_int_varargs **)(unaff_EBX + 0x6a3e4a);
  pIVar1 = (this->m_sharedBlobData).
           super_CSharedVarBase<CUtlVector<BlobDataPerUpdate_t,_CUtlMemory<BlobDataPerUpdate_t,_int>_>,C_PaintStream::SharedVar_m_sharedBlobData>
           .m_pSharedMemory;
  if (pIVar1 != (ISPSharedMemory *)0x0) {
    (*pIVar1->_vptr_ISPSharedMemory[4])(pIVar1);
    (this->m_sharedBlobData).
    super_CSharedVarBase<CUtlVector<BlobDataPerUpdate_t,_CUtlMemory<BlobDataPerUpdate_t,_int>_>,C_PaintStream::SharedVar_m_sharedBlobData>
    .m_pSharedMemory = (ISPSharedMemory *)0x0;
  }
  (this->m_sharedBlobData).
  super_CSharedVarBase<CUtlVector<BlobDataPerUpdate_t,_CUtlMemory<BlobDataPerUpdate_t,_int>_>,C_PaintStream::SharedVar_m_sharedBlobData>
  .m_pValue = (CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t,_int>_> *)0x0;
  (this->m_blobs).m_Size = 0;
  if ((this->m_blobs).m_Memory.m_nGrowSize < 0) {
    ppCVar2 = (this->m_blobs).m_Memory.m_pMemory;
  }
  else {
    ppCVar2 = (this->m_blobs).m_Memory.m_pMemory;
    if (ppCVar2 != (CBasePaintBlob **)0x0) {
      in_stack_ffffffc8 = ppCVar2;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x63c472) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x63c472));
      ppCVar2 = (CBasePaintBlob **)0x0;
      (this->m_blobs).m_Memory.m_pMemory = (CBasePaintBlob **)0x0;
    }
    (this->m_blobs).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_blobs).m_pElements = ppCVar2;
  if (-1 < (this->m_blobs).m_Memory.m_nGrowSize) {
    if (ppCVar2 != (CBasePaintBlob **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x63c472) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x63c472));
      (this->m_blobs).m_Memory.m_pMemory = (CBasePaintBlob **)0x0;
      in_stack_ffffffc8 = ppCVar2;
    }
    (this->m_blobs).m_Memory.m_nAllocationCount = 0;
  }
  C_BaseAnimating::~C_BaseAnimating(&this->super_C_BaseAnimating,(int)in_stack_ffffffc8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_PaintStream::Spawn at 00526ff0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void Spawn(C_PaintStream * this) */

void __thiscall C_PaintStream::Spawn(C_PaintStream *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseAnimating).m_bCanUseFastPath = false;
  _memset(this->m_lightingBoxColors,0,0x60);
  this->m_iCachedWorldBoundsUpdateTick = *(int *)(**(int **)(unaff_EBX + 0x63cd9b) + 0x18);
  C_BaseEntity::RemoveFromLeafSystem((C_BaseEntity *)this);
  C_BaseEntity::Spawn((C_BaseEntity *)this);
  return;
}


/* C_PaintStream::DebugDrawBlobs at 00526f20 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void DebugDrawBlobs(C_PaintStream * this) */

void __thiscall C_PaintStream::DebugDrawBlobs(C_PaintStream *this)

{
  Vector *position;
  int unaff_EBX;
  int i;
  
                    /* Unresolved local var: bool bDebugDraw@[???]
                       Unresolved local var: Color debugColor@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(int *)(*(int *)(&DAT_0084202e + unaff_EBX) + 0x30) != 0) {
    if (0 < (this->m_blobs).m_Size) {
      i = 0;
      do {
        position = CBasePaintBlob::GetPosition((this->m_blobs).m_Memory.m_pMemory[i]);
        NDebugOverlay::Cross3D(position,2.0,0,0xff,0xff,false,0.1);
        i = i + 1;
      } while (i < (this->m_blobs).m_Size);
    }
  }
  return;
}


/* C_PaintStream::Update at 0052acb0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void Update(C_PaintStream * this) */

void __thiscall C_PaintStream::Update(C_PaintStream *this)

{
  char cVar1;
  Vector *position;
  int unaff_EBX;
  int local_30;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_nCurrentUpdateFrame != *(int *)(**(int **)(unaff_EBX + 0x6390d8) + 4)) {
    cVar1 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x6390e0) + 0x2f0))
                      ((int *)**(undefined4 **)(unaff_EBX + 0x6390e0));
    if ((cVar1 == '\0') && ((this->m_blobs).m_Size != 0)) {
      DeleteDeadBlobs(this);
                    /* Unresolved local var: bool bDebugDraw@[???]
                       Unresolved local var: Color debugColor@[???] */
      if (*(int *)(*(int *)(unaff_EBX + 0x83e298) + 0x30) != 0) {
        if (0 < (this->m_blobs).m_Size) {
          local_30 = 0;
          do {
            position = CBasePaintBlob::GetPosition((this->m_blobs).m_Memory.m_pMemory[local_30]);
            NDebugOverlay::Cross3D(position,2.0,0,0xff,0xff,false,0.1);
            local_30 = local_30 + 1;
          } while (local_30 < (this->m_blobs).m_Size);
        }
      }
      UpdateBlobs(this);
    }
    UpdateRenderBoundsAndOriginWorldspace(this);
    this->m_nCurrentUpdateFrame = *(int *)(**(int **)(unaff_EBX + 0x6390d8) + 4);
  }
  return;
}


/* C_PaintStream::GetRenderAngles at 00526eb0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: QAngle * GetRenderAngles(C_PaintStream * this) */

QAngle * __thiscall C_PaintStream::GetRenderAngles(C_PaintStream *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(QAngle **)(extraout_ECX + 0x63d704);
}


/* C_PaintStream::GetRenderOrigin at 00526ed0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: Vector * GetRenderOrigin(C_PaintStream * this) */

Vector * __thiscall C_PaintStream::GetRenderOrigin(C_PaintStream *this)

{
  return &this->m_vCachedRenderOrigin;
}


/* C_PaintStream::GetRenderBoundsWorldspace at 00527070 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void GetRenderBoundsWorldspace(C_PaintStream * this, Vector * absMins,
   Vector * absMaxs) */

void __thiscall
C_PaintStream::GetRenderBoundsWorldspace(C_PaintStream *this,Vector *absMins,Vector *absMaxs)

{
  absMins->x = (this->m_vCachedWorldMins).x;
  absMins->y = (this->m_vCachedWorldMins).y;
  absMins->z = (this->m_vCachedWorldMins).z;
  absMaxs->x = (this->m_vCachedWorldMaxs).x;
  absMaxs->y = (this->m_vCachedWorldMaxs).y;
  absMaxs->z = (this->m_vCachedWorldMaxs).z;
  return;
}


/* C_PaintStream::GetRenderBounds at 005270d0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void GetRenderBounds(C_PaintStream * this, Vector * theMins, Vector *
   theMaxs) */

void __thiscall C_PaintStream::GetRenderBounds(C_PaintStream *this,Vector *theMins,Vector *theMaxs)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  
                    /* Unresolved local var: Vector res@[???] */
  fVar1 = (this->m_vCachedWorldMins).y;
  fVar2 = (this->m_vCachedRenderOrigin).y;
  fVar3 = (this->m_vCachedWorldMins).z;
  fVar4 = (this->m_vCachedRenderOrigin).z;
  theMins->x = (this->m_vCachedWorldMins).x - (this->m_vCachedRenderOrigin).x;
  theMins->y = fVar1 - fVar2;
  theMins->z = fVar3 - fVar4;
                    /* Unresolved local var: Vector res@[???] */
  fVar1 = (this->m_vCachedWorldMaxs).y;
  fVar2 = (this->m_vCachedRenderOrigin).y;
  fVar3 = (this->m_vCachedWorldMaxs).z;
  fVar4 = (this->m_vCachedRenderOrigin).z;
  theMaxs->x = (this->m_vCachedWorldMaxs).x - (this->m_vCachedRenderOrigin).x;
  theMaxs->y = fVar1 - fVar2;
  theMaxs->z = fVar3 - fVar4;
  return;
}


/* C_PaintStream::WorldSpaceCenter at 00526ee0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: Vector * WorldSpaceCenter(C_PaintStream * this) */

Vector * __thiscall C_PaintStream::WorldSpaceCenter(C_PaintStream *this)

{
  Vector *pVVar1;
  
                    /* WARNING: Could not recover jumptable at 0x00526eef. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  pVVar1 = (Vector *)
           (*(this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.
             super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0x37])();
  return pVVar1;
}


/* C_PaintStream::ComputeWorldSpaceSurroundingBox at 00526f00 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void ComputeWorldSpaceSurroundingBox(C_PaintStream * this, Vector *
   pMins, Vector * pMaxs) */

void __thiscall
C_PaintStream::ComputeWorldSpaceSurroundingBox(C_PaintStream *this,Vector *pMins,Vector *pMaxs)

{
                    /* WARNING: Could not recover jumptable at 0x00526f0f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x41])();
  return;
}


/* C_PaintStream::AddPaintBlob at 00527160 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void AddPaintBlob(C_PaintStream * this, CBasePaintBlob * pBlob) */

void __thiscall C_PaintStream::AddPaintBlob(C_PaintStream *this,CBasePaintBlob *pBlob)

{
  uint uVar1;
  int iVar2;
  CBasePaintBlob **ppCVar3;
  
  if (pBlob != (CBasePaintBlob *)0x0) {
    uVar1 = (this->m_blobs).m_Size;
    CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*,_int>_>::GrowVector(&this->m_blobs,1);
    iVar2 = ~uVar1 + (this->m_blobs).m_Size;
    if (0 < iVar2) {
      ppCVar3 = (this->m_blobs).m_Memory.m_pMemory;
      _V_memmove(ppCVar3 + uVar1 + 1,ppCVar3 + uVar1,iVar2 * 4);
    }
    ppCVar3 = (this->m_blobs).m_Memory.m_pMemory + uVar1;
    if (ppCVar3 != (CBasePaintBlob **)0x0) {
      *ppCVar3 = pBlob;
    }
  }
  return;
}


/* C_PaintStream::UpdateRenderBoundsAndOriginWorldspace at 0052a310 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void UpdateRenderBoundsAndOriginWorldspace(C_PaintStream * this) */

void __thiscall C_PaintStream::UpdateRenderBoundsAndOriginWorldspace(C_PaintStream *this)

{
  float fVar1;
  vec_t vVar2;
  vec_t vVar3;
  vec_t vVar4;
  undefined4 *puVar5;
  int *piVar6;
  BlobData_t *pBVar7;
  char cVar8;
  Vector *pVVar9;
  int iVar10;
  int iVar11;
  int iVar12;
  int unaff_EBX;
  float fVar13;
  float fVar14;
  float fVar15;
  float fVar16;
  int local_a4;
  BlobDataVector_t local_90;
  undefined4 local_7c;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  float local_58;
  float local_54;
  float local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  float local_34;
  float local_30;
  float local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_iCachedWorldBoundsUpdateTick != *(int *)(**(int **)(&DAT_00639a7b + unaff_EBX) + 0x18)
     ) {
    this->m_iCachedWorldBoundsUpdateTick = *(int *)(**(int **)(&DAT_00639a7b + unaff_EBX) + 0x18);
    cVar8 = (**(code **)(*(int *)**(undefined4 **)(&DAT_00639a83 + unaff_EBX) + 0x2f0))
                      ((int *)**(undefined4 **)(&DAT_00639a83 + unaff_EBX));
    if (cVar8 == '\0') {
      if ((this->m_blobs).m_Size == 0) {
        local_4c = 0;
        local_48 = 0;
        local_44 = 0;
        (this->m_vCachedWorldMaxs).x = 0.0;
        (this->m_vCachedWorldMaxs).y = 0.0;
        (this->m_vCachedWorldMaxs).z = 0.0;
        (this->m_vCachedWorldMins).x = 0.0;
        (this->m_vCachedWorldMins).y = 0.0;
        (this->m_vCachedWorldMins).z = 0.0;
        (this->m_vCachedRenderOrigin).x = 0.0;
        (this->m_vCachedRenderOrigin).y = 0.0;
        (this->m_vCachedRenderOrigin).z = 0.0;
        C_BaseEntity::RemoveFromLeafSystem((C_BaseEntity *)this);
      }
      else {
                    /* Unresolved local var: Vector vecPos@[???]
                       Unresolved local var: Vector vRadius@[???]
                       Unresolved local var: CCollisionProperty * prop@[???] */
        pVVar9 = CBasePaintBlob::GetPosition(*(this->m_blobs).m_Memory.m_pMemory);
        vVar2 = pVVar9->z;
        vVar3 = pVVar9->y;
        vVar4 = pVVar9->x;
        (this->m_vCachedWorldMins).x = vVar4;
        (this->m_vCachedWorldMins).y = vVar3;
        (this->m_vCachedWorldMins).z = vVar2;
        (this->m_vCachedWorldMaxs).x = vVar4;
        (this->m_vCachedWorldMaxs).y = vVar3;
        (this->m_vCachedWorldMaxs).z = vVar2;
                    /* Unresolved local var: int i@[???] */
        if (1 < (this->m_blobs).m_Size) {
          iVar10 = 1;
          do {
            pVVar9 = CBasePaintBlob::GetPosition((this->m_blobs).m_Memory.m_pMemory[iVar10]);
            fVar14 = pVVar9->x;
            fVar15 = pVVar9->y;
            fVar16 = pVVar9->z;
            fVar1 = (this->m_vCachedWorldMins).x;
            fVar13 = fVar14;
            if (fVar1 <= fVar14) {
              fVar13 = fVar1;
            }
            (this->m_vCachedWorldMins).x = fVar13;
            fVar1 = (this->m_vCachedWorldMins).y;
            fVar13 = fVar15;
            if (fVar1 <= fVar15) {
              fVar13 = fVar1;
            }
            (this->m_vCachedWorldMins).y = fVar13;
            fVar1 = (this->m_vCachedWorldMins).z;
            fVar13 = fVar16;
            if (fVar1 <= fVar16) {
              fVar13 = fVar1;
            }
            (this->m_vCachedWorldMins).z = fVar13;
            fVar1 = (this->m_vCachedWorldMaxs).x;
            if (fVar14 <= fVar1) {
              fVar14 = fVar1;
            }
            (this->m_vCachedWorldMaxs).x = fVar14;
            fVar14 = (this->m_vCachedWorldMaxs).y;
            if (fVar15 <= fVar14) {
              fVar15 = fVar14;
            }
            (this->m_vCachedWorldMaxs).y = fVar15;
            fVar14 = (this->m_vCachedWorldMaxs).z;
            if (fVar16 <= fVar14) {
              fVar16 = fVar14;
            }
            (this->m_vCachedWorldMaxs).z = fVar16;
            iVar10 = iVar10 + 1;
          } while (iVar10 < (this->m_blobs).m_Size);
        }
                    /* Unresolved local var: Vector res@[???] */
        fVar14 = (this->m_vCachedWorldMaxs).y;
        fVar15 = (this->m_vCachedWorldMaxs).z;
                    /* Unresolved local var: Vector res@[???] */
        fVar16 = (this->m_vCachedWorldMaxs).x;
        local_50 = *(float *)(unaff_EBX + 0x4b7eef);
        local_58 = (fVar16 + (this->m_vCachedWorldMins).x) * local_50;
        local_54 = (fVar14 + (this->m_vCachedWorldMins).y) * local_50;
        local_50 = (fVar15 + (this->m_vCachedWorldMins).z) * local_50;
        (this->m_vCachedRenderOrigin).x = local_58;
        (this->m_vCachedRenderOrigin).y = local_54;
        (this->m_vCachedRenderOrigin).z = local_50;
        local_7c = 0x41900000;
        local_78 = 0x41900000;
        local_74 = 0x41900000;
        local_64 = 0x41900000;
        local_60 = 0x41900000;
        local_5c = 0x41900000;
        (this->m_vCachedWorldMins).x = (this->m_vCachedWorldMins).x - 18.0;
        (this->m_vCachedWorldMins).y = (this->m_vCachedWorldMins).y - 18.0;
        (this->m_vCachedWorldMins).z = (this->m_vCachedWorldMins).z - 18.0;
        (this->m_vCachedWorldMaxs).x = fVar16 + 18.0;
        (this->m_vCachedWorldMaxs).y = fVar14 + 18.0;
        (this->m_vCachedWorldMaxs).z = fVar15 + 18.0;
        CCollisionProperty::MarkSurroundingBoundsDirty
                  (&(this->super_C_BaseAnimating).super_C_BaseEntity.m_Collision.
                    super_CCollisionProperty);
        C_BaseEntity::AddToLeafSystem((C_BaseEntity *)this);
      }
      C_BaseEntity::UpdateVisibility((C_BaseEntity *)this);
      return;
    }
    local_90.m_Memory.m_pMemory = (BlobData_t *)0x0;
    local_90.m_Memory.m_nAllocationCount = 0;
    local_90.m_Memory.m_nGrowSize = 0;
    local_90.m_Size = 0;
    local_90.m_pElements = (BlobData_t *)0x0;
    GetCurrentBlobData(this,&local_90);
    if (local_90.m_Size == 0) {
      local_28 = 0;
      local_24 = 0;
      local_20 = 0;
      (this->m_vCachedWorldMaxs).x = 0.0;
      (this->m_vCachedWorldMaxs).y = 0.0;
      (this->m_vCachedWorldMaxs).z = 0.0;
      (this->m_vCachedWorldMins).x = 0.0;
      (this->m_vCachedWorldMins).y = 0.0;
      (this->m_vCachedWorldMins).z = 0.0;
      (this->m_vCachedRenderOrigin).x = 0.0;
      (this->m_vCachedRenderOrigin).y = 0.0;
      (this->m_vCachedRenderOrigin).z = 0.0;
      C_BaseEntity::RemoveFromLeafSystem((C_BaseEntity *)this);
    }
    else {
                    /* Unresolved local var: Vector vecPos@[???]
                       Unresolved local var: Vector vRadius@[???]
                       Unresolved local var: CCollisionProperty * prop@[???] */
      vVar2 = ((local_90.m_Memory.m_pMemory)->m_vPosition).z;
      vVar3 = ((local_90.m_Memory.m_pMemory)->m_vPosition).y;
      vVar4 = ((local_90.m_Memory.m_pMemory)->m_vPosition).x;
      (this->m_vCachedWorldMins).x = vVar4;
      (this->m_vCachedWorldMins).y = vVar3;
      (this->m_vCachedWorldMins).z = vVar2;
      (this->m_vCachedWorldMaxs).x = vVar4;
      (this->m_vCachedWorldMaxs).y = vVar3;
      (this->m_vCachedWorldMaxs).z = vVar2;
                    /* Unresolved local var: int i@[???] */
      if (1 < local_90.m_Size) {
        iVar10 = 1;
        iVar12 = 0x2c;
        do {
          fVar14 = *(float *)((int)&((local_90.m_Memory.m_pMemory)->m_vPosition).x + iVar12);
          fVar15 = *(float *)((int)&((local_90.m_Memory.m_pMemory)->m_vPosition).y + iVar12);
          fVar16 = *(float *)((int)&((local_90.m_Memory.m_pMemory)->m_vPosition).z + iVar12);
          fVar1 = (this->m_vCachedWorldMins).x;
          fVar13 = fVar14;
          if (fVar1 <= fVar14) {
            fVar13 = fVar1;
          }
          (this->m_vCachedWorldMins).x = fVar13;
          fVar1 = (this->m_vCachedWorldMins).y;
          fVar13 = fVar15;
          if (fVar1 <= fVar15) {
            fVar13 = fVar1;
          }
          (this->m_vCachedWorldMins).y = fVar13;
          fVar1 = (this->m_vCachedWorldMins).z;
          fVar13 = fVar16;
          if (fVar1 <= fVar16) {
            fVar13 = fVar1;
          }
          (this->m_vCachedWorldMins).z = fVar13;
          fVar1 = (this->m_vCachedWorldMaxs).x;
          if (fVar14 <= fVar1) {
            fVar14 = fVar1;
          }
          (this->m_vCachedWorldMaxs).x = fVar14;
          fVar14 = (this->m_vCachedWorldMaxs).y;
          if (fVar15 <= fVar14) {
            fVar15 = fVar14;
          }
          (this->m_vCachedWorldMaxs).y = fVar15;
          fVar14 = (this->m_vCachedWorldMaxs).z;
          if (fVar16 <= fVar14) {
            fVar16 = fVar14;
          }
          (this->m_vCachedWorldMaxs).z = fVar16;
          iVar10 = iVar10 + 1;
          iVar12 = iVar12 + 0x2c;
        } while (iVar10 < local_90.m_Size);
      }
                    /* Unresolved local var: Vector res@[???] */
      fVar14 = (this->m_vCachedWorldMaxs).y;
      fVar15 = (this->m_vCachedWorldMaxs).z;
                    /* Unresolved local var: Vector res@[???] */
      fVar16 = (this->m_vCachedWorldMaxs).x;
      local_2c = *(float *)(unaff_EBX + 0x4b7eef);
      local_34 = (fVar16 + (this->m_vCachedWorldMins).x) * local_2c;
      local_30 = (fVar14 + (this->m_vCachedWorldMins).y) * local_2c;
      local_2c = (fVar15 + (this->m_vCachedWorldMins).z) * local_2c;
      (this->m_vCachedRenderOrigin).x = local_34;
      (this->m_vCachedRenderOrigin).y = local_30;
      (this->m_vCachedRenderOrigin).z = local_2c;
      local_70 = 0x41900000;
      local_6c = 0x41900000;
      local_68 = 0x41900000;
      local_40 = 0x41900000;
      local_3c = 0x41900000;
      local_38 = 0x41900000;
      (this->m_vCachedWorldMins).x = (this->m_vCachedWorldMins).x - 18.0;
      (this->m_vCachedWorldMins).y = (this->m_vCachedWorldMins).y - 18.0;
      (this->m_vCachedWorldMins).z = (this->m_vCachedWorldMins).z - 18.0;
      (this->m_vCachedWorldMaxs).x = fVar16 + 18.0;
      (this->m_vCachedWorldMaxs).y = fVar14 + 18.0;
      (this->m_vCachedWorldMaxs).z = fVar15 + 18.0;
      CCollisionProperty::MarkSurroundingBoundsDirty
                (&(this->super_C_BaseAnimating).super_C_BaseEntity.m_Collision.
                  super_CCollisionProperty);
      C_BaseEntity::AddToLeafSystem((C_BaseEntity *)this);
    }
    C_BaseEntity::UpdateVisibility((C_BaseEntity *)this);
    local_a4 = local_90.m_Size;
    puVar5 = *(undefined4 **)(&DAT_00639a4f + unaff_EBX);
    iVar10 = local_90.m_Size * 0x2c;
    while( true ) {
      iVar12 = iVar10;
      pBVar7 = local_90.m_Memory.m_pMemory;
      local_a4 = local_a4 + -1;
      if (local_a4 < 0) break;
      *(undefined4 *)((int)&local_90.m_Memory.m_pMemory[-1].m_teleportationHistory.m_Size + iVar12)
           = 0;
      if (*(int *)((int)&local_90.m_Memory.m_pMemory[-1].m_teleportationHistory.m_Memory.m_nGrowSize
                  + iVar12) < 0) {
        iVar11 = *(int *)((int)&local_90.m_Memory.m_pMemory[-1].m_teleportationHistory.m_Memory.
                                m_pMemory + iVar12);
      }
      else {
        iVar10 = *(int *)((int)&local_90.m_Memory.m_pMemory[-1].m_teleportationHistory.m_Memory.
                                m_pMemory + iVar12);
        if (iVar10 != 0) {
          piVar6 = (int *)*puVar5;
          (**(code **)(*piVar6 + 8))(piVar6,iVar10);
          *(undefined4 *)((int)&pBVar7[-1].m_teleportationHistory.m_Memory.m_pMemory + iVar12) = 0;
        }
        iVar11 = 0;
        *(undefined4 *)
         ((int)&pBVar7[-1].m_teleportationHistory.m_Memory.m_nAllocationCount + iVar12) = 0;
      }
      *(int *)((int)&pBVar7[-1].m_teleportationHistory.m_pElements + iVar12) = iVar11;
      *(undefined4 *)((int)&pBVar7[-1].m_teleportationHistory.m_Size + iVar12) = 0;
      if (-1 < *(int *)((int)&pBVar7[-1].m_teleportationHistory.m_Memory.m_nGrowSize + iVar12)) {
        if (iVar11 != 0) {
          (**(code **)(*(int *)**(undefined4 **)(&DAT_00639a4f + unaff_EBX) + 8))
                    ((int *)**(undefined4 **)(&DAT_00639a4f + unaff_EBX),iVar11);
          iVar11 = 0;
          *(undefined4 *)((int)&pBVar7[-1].m_teleportationHistory.m_Memory.m_pMemory + iVar12) = 0;
        }
        *(undefined4 *)
         ((int)&pBVar7[-1].m_teleportationHistory.m_Memory.m_nAllocationCount + iVar12) = 0;
      }
      *(int *)((int)&pBVar7[-1].m_teleportationHistory.m_pElements + iVar12) = iVar11;
      iVar10 = iVar12 + -0x2c;
      if (-1 < *(int *)((int)&pBVar7[-1].m_teleportationHistory.m_Memory.m_nGrowSize + iVar12)) {
        if (iVar11 != 0) {
          piVar6 = (int *)*puVar5;
          (**(code **)(*piVar6 + 8))(piVar6,iVar11);
          *(undefined4 *)((int)&pBVar7[-1].m_teleportationHistory.m_Memory.m_pMemory + iVar12) = 0;
        }
        *(undefined4 *)
         ((int)&pBVar7[-1].m_teleportationHistory.m_Memory.m_nAllocationCount + iVar12) = 0;
      }
    }
    local_90.m_Size = 0;
    if (-1 < local_90.m_Memory.m_nGrowSize) {
      if (local_90.m_Memory.m_pMemory != (BlobData_t *)0x0) {
        (**(code **)(*(int *)**(undefined4 **)(&DAT_00639a4f + unaff_EBX) + 8))
                  ((int *)**(undefined4 **)(&DAT_00639a4f + unaff_EBX),local_90.m_Memory.m_pMemory);
        local_90.m_Memory.m_pMemory = (BlobData_t *)0x0;
      }
      local_90.m_Memory.m_nAllocationCount = 0;
    }
    if ((-1 < local_90.m_Memory.m_nGrowSize) && (local_90.m_Memory.m_pMemory != (BlobData_t *)0x0))
    {
      local_90.m_pElements = local_90.m_Memory.m_pMemory;
      (**(code **)(*(int *)**(undefined4 **)(&DAT_00639a4f + unaff_EBX) + 8))
                ((int *)**(undefined4 **)(&DAT_00639a4f + unaff_EBX),local_90.m_Memory.m_pMemory);
    }
  }
  return;
}


/* BuildPortalMatrixListInAABB at 00527200 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

void BuildPortalMatrixListInAABB
               (Vector *vCenter,Vector *vExtents,PortalMatrixList_t *portalMatrixList)

{
  C_Portal_Base2D *pPortal;
  uint uVar1;
  matrix3x4_t *pmVar2;
  bool bVar3;
  float *pfVar4;
  QAngle *angles;
  int iVar5;
  int unaff_EBX;
  float *pfVar6;
  int local_80;
  float local_70 [12];
  float local_40;
  float local_3c;
  float local_38;
  Vector local_34;
  float local_28;
  float local_24;
  float local_20;
  
  ___i686_get_pc_thunk_bx();
  if (0 < *(int *)(*(int *)(unaff_EBX + 0x63cd9b) + 0xc)) {
    local_80 = 0;
    do {
      pPortal = *(C_Portal_Base2D **)(**(int **)(unaff_EBX + 0x63cd9b) + local_80 * 4);
      bVar3 = UTIL_IsBoxIntersectingPortal(vCenter,vExtents,pPortal,0.0);
      if (bVar3) {
                    /* Unresolved local var: Vector res@[???] */
        pfVar4 = (float *)(unaff_EBX + 0x4bb05b);
        local_40 = *pfVar4 * (pPortal->m_plane_Origin).normal.x;
        local_3c = *pfVar4 * (pPortal->m_plane_Origin).normal.y;
        local_38 = *pfVar4 * (pPortal->m_plane_Origin).normal.z;
        local_28 = local_40;
        local_24 = local_3c;
        local_20 = local_38;
        pfVar4 = (float *)(*(pPortal->super_CPortalRenderable_FlatBasic).super_C_BaseAnimating.
                            super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
                            super_IHandleEntity._vptr_IHandleEntity[0xd])(pPortal);
                    /* Unresolved local var: Vector res@[???] */
        local_34.x = *pfVar4 - local_28;
        local_34.y = pfVar4[1] - local_24;
        local_34.z = pfVar4[2] - local_20;
        angles = (QAngle *)
                 (*(pPortal->super_CPortalRenderable_FlatBasic).super_C_BaseAnimating.
                   super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
                   _vptr_IHandleEntity[0xe])(pPortal);
        AngleMatrix(angles,&local_34,(matrix3x4_t *)local_70);
        uVar1 = portalMatrixList->m_Size;
        CUtlVector<matrix3x4_t,CUtlMemory<matrix3x4_t,_int>_>::GrowVector(portalMatrixList,1);
        iVar5 = ~uVar1 + portalMatrixList->m_Size;
        if (0 < iVar5) {
          pmVar2 = (portalMatrixList->m_Memory).m_pMemory;
          _V_memmove(pmVar2 + uVar1 + 1,pmVar2 + uVar1,iVar5 * 0x30);
        }
        pfVar4 = (float *)((portalMatrixList->m_Memory).m_pMemory + uVar1);
        if ((matrix3x4_t *)pfVar4 != (matrix3x4_t *)0x0) {
          pfVar6 = local_70;
          for (iVar5 = 0xc; iVar5 != 0; iVar5 = iVar5 + -1) {
            *pfVar4 = *pfVar6;
            pfVar6 = pfVar6 + 1;
            pfVar4 = pfVar4 + 1;
          }
        }
      }
      local_80 = local_80 + 1;
    } while (local_80 < *(int *)(*(int *)(unaff_EBX + 0x63cd9b) + 0xc));
  }
  return;
}


/* C_PaintStream::DrawBlobs at 00528c20 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void DrawBlobs(C_PaintStream * this, IMaterial * pMaterial) */

void __thiscall C_PaintStream::DrawBlobs(C_PaintStream *this,IMaterial *pMaterial)

{
  undefined1 *puVar1;
  undefined4 *puVar2;
  code *pcVar3;
  CBasePaintBlob *this_00;
  int *piVar4;
  CUtlMemoryAligned<ImpParticleWithFourInterpolants,16> *pCVar5;
  void *pvVar6;
  ImpParticleWithFourInterpolants *pIVar7;
  BlobData_t *pBVar8;
  char cVar9;
  Vector *pVVar10;
  int *piVar11;
  C_BasePlayer *pCVar12;
  int iVar13;
  int iVar14;
  IClientRenderable *pIVar15;
  undefined1 *puVar16;
  int iVar17;
  int iVar18;
  float *pfVar19;
  int unaff_EBX;
  undefined4 *puVar20;
  undefined1 *puVar21;
  vec_t *pvVar22;
  float fVar23;
  float fVar24;
  float fVar25;
  float fVar26;
  float fVar27;
  float fVar28;
  float fVar29;
  float fVar30;
  int local_104;
  Vector *local_e0;
  int local_d0;
  int local_c4;
  int local_c0;
  int local_b4;
  BlobDataVector_t local_8c;
  PortalMatrixList_t local_78;
  float local_64;
  float local_60;
  float local_5c;
  float local_58;
  float local_54;
  float local_50;
  float local_4c;
  float local_48;
  float local_44;
  Vector local_40;
  float local_34;
  float local_30;
  float local_2c;
  Vector local_28 [2];
  
                    /* Unresolved local var: int nCount@[???]
                       Unresolved local var: Vector vCenter@[???]
                       Unresolved local var: PortalMatrixList_t portalMatrixList@[???] */
  puVar20 = (undefined4 *)&stack0xfffffed4;
  puVar21 = &stack0xfffffed4;
  ___i686_get_pc_thunk_bx();
  pfVar19 = *(float **)(unaff_EBX + 0x63b1cf);
  local_28[0].x = *pfVar19;
  local_28[0].y = pfVar19[1];
  local_28[0].z = pfVar19[2];
  local_78.m_Memory.m_pMemory = (matrix3x4_t *)0x0;
  local_78.m_Memory.m_nAllocationCount = 0;
  local_78.m_Memory.m_nGrowSize = 0;
  local_78.m_Size = 0;
  local_78.m_pElements = (matrix3x4_t *)0x0;
                    /* Unresolved local var: Vector res@[???] */
  local_34 = (this->m_vCachedWorldMaxs).x - (this->m_vCachedWorldMins).x;
  local_30 = (this->m_vCachedWorldMaxs).y - (this->m_vCachedWorldMins).y;
  local_2c = (this->m_vCachedWorldMaxs).z - (this->m_vCachedWorldMins).z;
                    /* Unresolved local var: Vector res@[???] */
  local_5c = *(float *)(unaff_EBX + 0x4b95df);
  local_64 = local_34 * local_5c;
  local_60 = local_30 * local_5c;
  local_5c = local_2c * local_5c;
  local_40.x = local_64;
  local_40.y = local_60;
  local_40.z = local_5c;
  BuildPortalMatrixListInAABB(&this->m_vCachedRenderOrigin,&local_40,&local_78);
  cVar9 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x63b173) + 0x2f0))();
  if (cVar9 == '\0') {
    iVar14 = (this->m_blobs).m_Size;
    if ((iVar14 != 0) && (*(int *)(*(int *)(&DAT_008402cb + unaff_EBX) + 0x30) != 0)) {
                    /* Unresolved local var: int i@[???] */
      if (0 < iVar14) {
        iVar18 = 0;
        do {
          pVVar10 = CBasePaintBlob::GetPosition((this->m_blobs).m_Memory.m_pMemory[iVar18]);
          local_28[0].x = local_28[0].x + pVVar10->x;
          local_28[0].y = local_28[0].y + pVVar10->y;
          local_28[0].z = local_28[0].z + pVVar10->z;
          iVar18 = iVar18 + 1;
        } while (iVar18 != iVar14);
      }
                    /* Unresolved local var: float oofl@[???] */
      fVar30 = *(float *)(unaff_EBX + 0x4b9553) / (float)iVar14;
      local_28[0].x = local_28[0].x * fVar30;
      local_28[0].y = local_28[0].y * fVar30;
      local_28[0].z = local_28[0].z * fVar30;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x63b30f) + 0x50))();
      NPaintRenderer::Paintblob_SafeLightCubeUpdate(local_28,this->m_lightingBoxColors);
      piVar11 = *(int **)(unaff_EBX + 0x63b2db);
      piVar4 = (int *)*piVar11;
      if ((piVar4 != (int *)0x0) && (iVar18 = (**(code **)(*piVar4 + 0x1a0))(), iVar18 != 0)) {
        piVar11 = (int *)(**(code **)(*(int *)*piVar11 + 0x1a0))();
        (**(code **)(*piVar11 + 0x40))();
      }
      iVar18 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x63b173) + 0x1f8))();
      pCVar12 = GetSplitScreenViewPlayer(iVar18);
      if (pCVar12 != (C_BasePlayer *)0x0) {
        (**(code **)(*(int *)&(pCVar12->super_C_BaseCombatCharacter).super_C_BaseFlex.
                              super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.
                              super_IClientEntity.super_IClientUnknown + 0x330))();
        fVar30 = *(float *)(*(int *)(unaff_EBX + 0x84038b) + 0x2c);
        pCVar5 = *(CUtlMemoryAligned<ImpParticleWithFourInterpolants,16> **)(unaff_EBX + 0x63b98f);
        pCVar5[1].super_CUtlMemory<ImpParticleWithFourInterpolants,int>.m_pMemory =
             (ImpParticleWithFourInterpolants *)0x0;
        iVar18 = (pCVar5->super_CUtlMemory<ImpParticleWithFourInterpolants,int>).m_nAllocationCount;
        if (iVar18 < iVar14) {
          CUtlMemoryAligned<ImpParticleWithFourInterpolants,16>::Grow(pCVar5,iVar14 - iVar18);
        }
        puVar20 = *(undefined4 **)(unaff_EBX + 0x63b98f);
        iVar18 = puVar20[3];
        puVar20[3] = iVar14 + iVar18;
        pvVar6 = (void *)*puVar20;
        puVar20[4] = pvVar6;
        iVar18 = (iVar14 + iVar18) - iVar14;
        if (iVar18 < 1) {
LAB_005290fd:
          if (0 >= iVar14) goto LAB_00529a99;
          iVar17 = 0;
          iVar18 = 0;
          do {
            iVar13 = iVar18 + **(int **)(unaff_EBX + 0x63b98f);
            if (iVar13 != 0) {
              *(undefined4 *)(iVar13 + 0xc) = 0;
              *(undefined4 *)(iVar13 + 0x2c) = 0;
              *(undefined4 *)(iVar13 + 0x3c) = 0;
              *(undefined4 *)(iVar13 + 0x4c) = 0;
              *(undefined4 *)(iVar13 + 0x5c) = 0;
            }
            iVar17 = iVar17 + 1;
            iVar18 = iVar18 + 0x60;
          } while (iVar17 < iVar14);
          local_c4 = 0;
          local_c0 = 0;
          do {
            this_00 = (this->m_blobs).m_Memory.m_pMemory[local_c0];
            if (this_00->m_bSilent == false) {
              pVVar10 = CBasePaintBlob::GetPosition(this_00);
              fVar26 = pVVar10->z;
              fVar24 = pVVar10->y;
                    /* Unresolved local var: Vector delta@[???] */
              fVar25 = pVVar10->x - local_58;
              fVar27 = fVar24 - local_54;
              fVar23 = fVar26 - local_50;
              if (fVar30 * fVar30 < fVar25 * fVar25 + fVar27 * fVar27 + fVar23 * fVar23) {
                pfVar19 = (float *)(local_c4 * 0x60 + **(int **)(unaff_EBX + 0x63b98f));
                *pfVar19 = pVVar10->x;
                pfVar19[1] = fVar24;
                pfVar19[2] = fVar26;
                pfVar19[3] = 0.0;
                fVar24 = CBasePaintBlob::GetRadiusScale(this_00);
                pfVar19[4] = fVar24;
                fVar26 = *(float *)(unaff_EBX + 0x4b9553);
                if ((fVar24 <= fVar26) && (fVar26 = *(float *)(unaff_EBX + 0x4be593), 0.0 < fVar24))
                {
                  fVar24 = *(float *)(unaff_EBX + 0x4b9553) / fVar24;
                  fVar26 = fVar24 * fVar24;
                }
                pfVar19[3] = fVar26;
                pfVar19[8] = 1.0;
                pfVar19[9] = 1.0;
                pfVar19[10] = 1.0;
                pfVar19[0xb] = 0.0;
                local_c4 = local_c4 + 1;
              }
              if (this_00->m_bGhosting != false) {
                pvVar22 = (vec_t *)(local_c4 * 0x60 + **(int **)(unaff_EBX + 0x63b98f));
                pVVar10 = CBasePaintBlob::GetGhostPosition(this_00);
                *pvVar22 = pVVar10->x;
                pvVar22[1] = pVVar10->y;
                pvVar22[2] = pVVar10->z;
                pvVar22[3] = 0.0;
                fVar24 = CBasePaintBlob::GetRadiusScale(this_00);
                pvVar22[4] = fVar24;
                fVar26 = *(float *)(unaff_EBX + 0x4b9553);
                if ((fVar24 <= fVar26) && (fVar26 = *(float *)(unaff_EBX + 0x4be593), 0.0 < fVar24))
                {
                  fVar24 = *(float *)(unaff_EBX + 0x4b9553) / fVar24;
                  fVar26 = fVar24 * fVar24;
                }
                pvVar22[3] = fVar26;
                pvVar22[8] = 1.0;
                pvVar22[9] = 1.0;
                pvVar22[10] = 1.0;
                pvVar22[0xb] = 0.0;
                local_c4 = local_c4 + 1;
                pCVar5 = *(CUtlMemoryAligned<ImpParticleWithFourInterpolants,16> **)
                          (unaff_EBX + 0x63b98f);
                pIVar7 = pCVar5[1].super_CUtlMemory<ImpParticleWithFourInterpolants,int>.m_pMemory;
                puVar1 = (undefined1 *)
                         ((int)&(pIVar7->super_ImpParticleWithTwoInterpolants).
                                super_ImpParticleWithOneInterpolant.super_ImpParticle.center.field_0
                         + 1);
                iVar18 = (pCVar5->super_CUtlMemory<ImpParticleWithFourInterpolants,int>).
                         m_nAllocationCount;
                puVar16 = puVar1;
                if (iVar18 < (int)puVar1) {
                  CUtlMemoryAligned<ImpParticleWithFourInterpolants,16>::Grow
                            (pCVar5,(int)puVar1 - iVar18);
                  puVar16 = (undefined1 *)(*(int *)(*(int *)(unaff_EBX + 0x63b98f) + 0xc) + 1);
                }
                piVar11 = *(int **)(unaff_EBX + 0x63b98f);
                piVar11[3] = (int)puVar16;
                iVar18 = *piVar11;
                piVar11[4] = iVar18;
                if (0 < (int)(puVar16 + (-1 - (int)pIVar7))) {
                  _V_memmove((void *)(iVar18 + (int)puVar1 * 0x60),
                             (void *)(iVar18 + (int)pIVar7 * 0x60),
                             (int)(puVar16 + (-1 - (int)pIVar7)) * 0x60);
                }
                iVar18 = (int)pIVar7 * 0x60 + **(int **)(unaff_EBX + 0x63b98f);
                if (iVar18 != 0) {
                  *(undefined4 *)(iVar18 + 0xc) = 0;
                  *(undefined4 *)(iVar18 + 0x2c) = 0;
                  *(undefined4 *)(iVar18 + 0x3c) = 0;
                  *(undefined4 *)(iVar18 + 0x4c) = 0;
                  *(undefined4 *)(iVar18 + 0x5c) = 0;
                }
              }
            }
            local_c0 = local_c0 + 1;
          } while (local_c0 < iVar14);
        }
        else {
          if (0 < iVar14) {
            _V_memmove((void *)((int)pvVar6 + iVar14 * 0x60),pvVar6,iVar18 * 0x60);
            goto LAB_005290fd;
          }
LAB_00529a99:
          local_c4 = 0;
        }
        iVar14 = *(int *)(*(int *)(unaff_EBX + 0x63b98f) + 0xc);
        local_c4 = local_c4 - iVar14;
        if (local_c4 < 1) {
          if (local_c4 != 0) {
            *(int *)(*(int *)(unaff_EBX + 0x63b98f) + 0xc) = iVar14 + local_c4;
          }
        }
        else {
          iVar13 = local_c4 + iVar14;
          iVar18 = *(int *)(*(int *)(unaff_EBX + 0x63b98f) + 4);
          iVar17 = iVar13;
          if (iVar18 < iVar13) {
            CUtlMemoryAligned<ImpParticleWithFourInterpolants,16>::Grow
                      (*(CUtlMemoryAligned<ImpParticleWithFourInterpolants,16> **)
                        (unaff_EBX + 0x63b98f),iVar13 - iVar18);
            iVar17 = local_c4 + *(int *)(*(int *)(unaff_EBX + 0x63b98f) + 0xc);
          }
          piVar11 = *(int **)(unaff_EBX + 0x63b98f);
          piVar11[3] = iVar17;
          iVar18 = *piVar11;
          piVar11[4] = iVar18;
          iVar17 = (iVar17 - iVar14) - local_c4;
          if (0 < iVar17) {
            _V_memmove((void *)(iVar18 + iVar13 * 0x60),(void *)(iVar18 + iVar14 * 0x60),
                       iVar17 * 0x60);
          }
          iVar14 = iVar14 * 0x60;
          iVar18 = 0;
                    /* Unresolved local var: int i@[???] */
          do {
            iVar17 = iVar14 + **(int **)(unaff_EBX + 0x63b98f);
            if (iVar17 != 0) {
              *(undefined4 *)(iVar17 + 0xc) = 0;
              *(undefined4 *)(iVar17 + 0x2c) = 0;
              *(undefined4 *)(iVar17 + 0x3c) = 0;
              *(undefined4 *)(iVar17 + 0x4c) = 0;
              *(undefined4 *)(iVar17 + 0x5c) = 0;
            }
            iVar18 = iVar18 + 1;
            iVar14 = iVar14 + 0x60;
          } while (iVar18 != local_c4);
        }
        local_e0 = local_28;
        fVar30 = *(float *)(*(int *)(unaff_EBX + 0x8403eb) + 0x2c);
        pIVar15 = (IClientRenderable *)
                  (*(this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.
                    super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[6])();
        NPaintRenderer::Paintblob_Draw
                  ((this->m_nRenderMode).m_Value,pIVar15,local_e0,pMaterial,fVar30,&local_78,false);
        goto LAB_0052990f;
      }
    }
joined_r0x00528e98:
    if (local_78.m_Memory.m_nGrowSize < 0) {
      local_78.m_Size = 0;
      goto LAB_00528e9d;
    }
    local_78.m_Size = 0;
    puVar20 = (undefined4 *)&stack0xfffffed4;
    if (local_78.m_Memory.m_pMemory != (matrix3x4_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x63b13f) + 8))();
LAB_00528ee8:
      local_78.m_Memory.m_pMemory = (matrix3x4_t *)0x0;
      puVar20 = (undefined4 *)puVar21;
    }
  }
  else {
    local_8c.m_Memory.m_pMemory = (BlobData_t *)0x0;
    local_8c.m_Memory.m_nAllocationCount = 0;
    local_8c.m_Memory.m_nGrowSize = 0;
    local_8c.m_Size = 0;
    local_8c.m_pElements = (BlobData_t *)0x0;
    GetCurrentBlobData(this,&local_8c);
    iVar14 = local_8c.m_Size;
    if ((local_8c.m_Size == 0) || (*(int *)(*(int *)(&DAT_008402cb + unaff_EBX) + 0x30) == 0)) {
LAB_00528d6c:
      puVar2 = *(undefined4 **)(unaff_EBX + 0x63b13f);
      iVar14 = local_8c.m_Size * 0x2c;
      local_d0 = local_8c.m_Size;
      while( true ) {
        iVar18 = iVar14;
        pBVar8 = local_8c.m_Memory.m_pMemory;
        local_d0 = local_d0 + -1;
        if (local_d0 < 0) break;
        *(undefined4 *)
         ((int)&local_8c.m_Memory.m_pMemory[-1].m_teleportationHistory.m_Size + iVar18) = 0;
        if (*(int *)((int)&local_8c.m_Memory.m_pMemory[-1].m_teleportationHistory.m_Memory.
                           m_nGrowSize + iVar18) < 0) {
          iVar17 = *(int *)((int)&local_8c.m_Memory.m_pMemory[-1].m_teleportationHistory.m_Memory.
                                  m_pMemory + iVar18);
        }
        else {
          if (*(int *)((int)&local_8c.m_Memory.m_pMemory[-1].m_teleportationHistory.m_Memory.
                             m_pMemory + iVar18) != 0) {
            (**(code **)(*(int *)*puVar2 + 8))();
            *(undefined4 *)((int)&pBVar8[-1].m_teleportationHistory.m_Memory.m_pMemory + iVar18) = 0
            ;
          }
          iVar17 = 0;
          *(undefined4 *)
           ((int)&pBVar8[-1].m_teleportationHistory.m_Memory.m_nAllocationCount + iVar18) = 0;
        }
        *(int *)((int)&pBVar8[-1].m_teleportationHistory.m_pElements + iVar18) = iVar17;
        *(undefined4 *)((int)&pBVar8[-1].m_teleportationHistory.m_Size + iVar18) = 0;
        if (-1 < *(int *)((int)&pBVar8[-1].m_teleportationHistory.m_Memory.m_nGrowSize + iVar18)) {
          if (iVar17 != 0) {
            (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x63b13f) + 8))();
            iVar17 = 0;
            *(undefined4 *)((int)&pBVar8[-1].m_teleportationHistory.m_Memory.m_pMemory + iVar18) = 0
            ;
          }
          *(undefined4 *)
           ((int)&pBVar8[-1].m_teleportationHistory.m_Memory.m_nAllocationCount + iVar18) = 0;
        }
        *(int *)((int)&pBVar8[-1].m_teleportationHistory.m_pElements + iVar18) = iVar17;
        iVar14 = iVar18 + -0x2c;
        if (-1 < *(int *)((int)&pBVar8[-1].m_teleportationHistory.m_Memory.m_nGrowSize + iVar18)) {
          if (iVar17 != 0) {
            (**(code **)(*(int *)*puVar2 + 8))();
            *(undefined4 *)((int)&pBVar8[-1].m_teleportationHistory.m_Memory.m_pMemory + iVar18) = 0
            ;
          }
          *(undefined4 *)
           ((int)&pBVar8[-1].m_teleportationHistory.m_Memory.m_nAllocationCount + iVar18) = 0;
        }
      }
      local_8c.m_Size = 0;
      if (-1 < local_8c.m_Memory.m_nGrowSize) {
        if (local_8c.m_Memory.m_pMemory != (BlobData_t *)0x0) {
          (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x63b13f) + 8))();
          local_8c.m_Memory.m_pMemory = (BlobData_t *)0x0;
        }
        local_8c.m_Memory.m_nAllocationCount = 0;
      }
      local_8c.m_pElements = local_8c.m_Memory.m_pMemory;
      if (-1 < local_8c.m_Memory.m_nGrowSize) {
        if (local_8c.m_Memory.m_pMemory != (BlobData_t *)0x0) {
          (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x63b13f) + 8))();
          local_8c.m_Memory.m_pMemory = (BlobData_t *)0x0;
        }
        local_8c.m_Memory.m_nAllocationCount = 0;
      }
      goto joined_r0x00528e98;
    }
                    /* Unresolved local var: int i@[???] */
    if (0 < local_8c.m_Size) {
      iVar18 = 0;
      iVar17 = 0;
      do {
        local_28[0].x =
             local_28[0].x +
             *(float *)((int)&((local_8c.m_Memory.m_pMemory)->m_vPosition).x + iVar17);
        local_28[0].y =
             local_28[0].y +
             *(float *)((int)&((local_8c.m_Memory.m_pMemory)->m_vPosition).y + iVar17);
        local_28[0].z =
             local_28[0].z +
             *(float *)((int)&((local_8c.m_Memory.m_pMemory)->m_vPosition).z + iVar17);
        iVar18 = iVar18 + 1;
        iVar17 = iVar17 + 0x2c;
      } while (iVar18 != local_8c.m_Size);
    }
                    /* Unresolved local var: float oofl@[???] */
    fVar30 = *(float *)(unaff_EBX + 0x4b9553) / (float)local_8c.m_Size;
    local_28[0].x = local_28[0].x * fVar30;
    local_28[0].y = local_28[0].y * fVar30;
    local_28[0].z = local_28[0].z * fVar30;
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x63b30f) + 0x50))();
    NPaintRenderer::Paintblob_SafeLightCubeUpdate(local_28,this->m_lightingBoxColors);
    piVar11 = *(int **)(unaff_EBX + 0x63b2db);
    piVar4 = (int *)*piVar11;
    if ((piVar4 != (int *)0x0) && (iVar18 = (**(code **)(*piVar4 + 0x1a0))(), iVar18 != 0)) {
      piVar11 = (int *)(**(code **)(*(int *)*piVar11 + 0x1a0))();
      (**(code **)(*piVar11 + 0x40))();
    }
    iVar18 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x63b173) + 0x1f8))();
    pCVar12 = GetSplitScreenViewPlayer(iVar18);
    if (pCVar12 == (C_BasePlayer *)0x0) goto LAB_00528d6c;
    (**(code **)(*(int *)&(pCVar12->super_C_BaseCombatCharacter).super_C_BaseFlex.
                          super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.
                          super_IClientEntity.super_IClientUnknown + 0x330))();
    iVar18 = local_8c.m_Size;
    fVar30 = *(float *)(*(int *)(unaff_EBX + 0x84038b) + 0x2c);
    pCVar5 = *(CUtlMemoryAligned<ImpParticleWithFourInterpolants,16> **)(unaff_EBX + 0x63b98f);
    pCVar5[1].super_CUtlMemory<ImpParticleWithFourInterpolants,int>.m_pMemory =
         (ImpParticleWithFourInterpolants *)0x0;
    if (local_8c.m_Size != 0) {
      iVar17 = (pCVar5->super_CUtlMemory<ImpParticleWithFourInterpolants,int>).m_nAllocationCount;
      if (iVar17 < local_8c.m_Size) {
        CUtlMemoryAligned<ImpParticleWithFourInterpolants,16>::Grow(pCVar5,local_8c.m_Size - iVar17)
        ;
      }
      puVar20 = *(undefined4 **)(unaff_EBX + 0x63b98f);
      iVar17 = puVar20[3];
      puVar20[3] = iVar17 + iVar18;
      pvVar6 = (void *)*puVar20;
      puVar20[4] = pvVar6;
      iVar17 = (iVar17 + iVar18) - iVar18;
      if (0 < iVar17) {
        if (0 >= iVar18) goto LAB_00529519;
        _V_memmove((void *)((int)pvVar6 + iVar18 * 0x60),pvVar6,iVar17 * 0x60);
      }
      if (0 < iVar18) {
        local_104 = 0;
        iVar17 = 0;
        do {
          iVar13 = iVar17 + **(int **)(unaff_EBX + 0x63b98f);
          if (iVar13 != 0) {
            *(undefined4 *)(iVar13 + 0xc) = 0;
            *(undefined4 *)(iVar13 + 0x2c) = 0;
            *(undefined4 *)(iVar13 + 0x3c) = 0;
            *(undefined4 *)(iVar13 + 0x4c) = 0;
            *(undefined4 *)(iVar13 + 0x5c) = 0;
          }
          local_104 = local_104 + 1;
          iVar17 = iVar17 + 0x60;
        } while (local_104 < iVar18);
      }
    }
LAB_00529519:
                    /* Unresolved local var: int i@[???] */
    iVar18 = 0;
    if (0 < iVar14) {
      iVar13 = 0;
      iVar17 = 0;
      fVar26 = *(float *)(unaff_EBX + 0x4be593);
      do {
        fVar24 = *(float *)((int)&((local_8c.m_Memory.m_pMemory)->m_vPosition).z + iVar17);
        fVar25 = *(float *)((int)&((local_8c.m_Memory.m_pMemory)->m_vPosition).y + iVar17);
        fVar27 = *(float *)((int)&((local_8c.m_Memory.m_pMemory)->m_vPosition).x + iVar17);
                    /* Unresolved local var: Vector delta@[???] */
        fVar23 = fVar27 - local_4c;
        fVar28 = fVar25 - local_48;
        fVar29 = fVar24 - local_44;
        if (fVar30 * fVar30 < fVar23 * fVar23 + fVar28 * fVar28 + fVar29 * fVar29) {
          pfVar19 = (float *)(iVar18 * 0x60 + **(int **)(unaff_EBX + 0x63b98f));
          *pfVar19 = fVar27;
          pfVar19[1] = fVar25;
          pfVar19[2] = fVar24;
          pfVar19[3] = 0.0;
          fVar24 = *(float *)((int)&(local_8c.m_Memory.m_pMemory)->m_flScale + iVar17);
          pfVar19[4] = fVar24;
          fVar25 = *(float *)(unaff_EBX + 0x4b9553);
          if ((fVar24 <= fVar25) && (fVar25 = fVar26, 0.0 < fVar24)) {
            fVar24 = *(float *)(unaff_EBX + 0x4b9553) / fVar24;
            fVar25 = fVar24 * fVar24;
          }
          pfVar19[3] = fVar25;
          pfVar19[8] = 1.0;
          pfVar19[9] = 1.0;
          pfVar19[10] = 1.0;
          pfVar19[0xb] = 0.0;
          iVar18 = iVar18 + 1;
        }
        iVar13 = iVar13 + 1;
        iVar17 = iVar17 + 0x2c;
      } while (iVar13 != iVar14);
    }
    pCVar5 = *(CUtlMemoryAligned<ImpParticleWithFourInterpolants,16> **)(unaff_EBX + 0x63b98f);
    pIVar7 = pCVar5[1].super_CUtlMemory<ImpParticleWithFourInterpolants,int>.m_pMemory;
    iVar18 = iVar18 - (int)pIVar7;
    if (iVar18 < 1) {
      if (iVar18 != 0) {
        *(int *)(*(int *)(unaff_EBX + 0x63b98f) + 0xc) = (int)pIVar7 + iVar18;
      }
    }
    else {
      iVar13 = iVar18 + (int)pIVar7;
      iVar14 = (pCVar5->super_CUtlMemory<ImpParticleWithFourInterpolants,int>).m_nAllocationCount;
      iVar17 = iVar13;
      if (iVar14 < iVar13) {
        CUtlMemoryAligned<ImpParticleWithFourInterpolants,16>::Grow(pCVar5,iVar13 - iVar14);
        iVar17 = iVar18 + *(int *)(*(int *)(unaff_EBX + 0x63b98f) + 0xc);
      }
      piVar11 = *(int **)(unaff_EBX + 0x63b98f);
      piVar11[3] = iVar17;
      iVar14 = *piVar11;
      piVar11[4] = iVar14;
      iVar17 = (iVar17 - (int)pIVar7) - iVar18;
      if (0 < iVar17) {
        _V_memmove((void *)(iVar14 + iVar13 * 0x60),(void *)(iVar14 + (int)pIVar7 * 0x60),
                   iVar17 * 0x60);
      }
      iVar14 = (int)pIVar7 * 0x60;
      iVar17 = 0;
                    /* Unresolved local var: int i@[???] */
      do {
        iVar13 = iVar14 + **(int **)(unaff_EBX + 0x63b98f);
        if (iVar13 != 0) {
          *(undefined4 *)(iVar13 + 0xc) = 0;
          *(undefined4 *)(iVar13 + 0x2c) = 0;
          *(undefined4 *)(iVar13 + 0x3c) = 0;
          *(undefined4 *)(iVar13 + 0x4c) = 0;
          *(undefined4 *)(iVar13 + 0x5c) = 0;
        }
        iVar17 = iVar17 + 1;
        iVar14 = iVar14 + 0x60;
      } while (iVar17 != iVar18);
    }
    fVar30 = *(float *)(*(int *)(unaff_EBX + 0x8403eb) + 0x2c);
    pIVar15 = (IClientRenderable *)
              (*(this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.
                super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[6])();
    NPaintRenderer::Paintblob_Draw
              ((this->m_nRenderMode).m_Value,pIVar15,local_28,pMaterial,fVar30,&local_78,false);
    local_b4 = local_8c.m_Size;
    puVar20 = *(undefined4 **)(unaff_EBX + 0x63b13f);
    iVar14 = local_8c.m_Size * 0x2c;
    while( true ) {
      iVar18 = iVar14;
      pBVar8 = local_8c.m_Memory.m_pMemory;
      local_b4 = local_b4 + -1;
      if (local_b4 < 0) break;
      *(undefined4 *)((int)&local_8c.m_Memory.m_pMemory[-1].m_teleportationHistory.m_Size + iVar18)
           = 0;
      if (*(int *)((int)&local_8c.m_Memory.m_pMemory[-1].m_teleportationHistory.m_Memory.m_nGrowSize
                  + iVar18) < 0) {
        iVar17 = *(int *)((int)&local_8c.m_Memory.m_pMemory[-1].m_teleportationHistory.m_Memory.
                                m_pMemory + iVar18);
      }
      else {
        if (*(int *)((int)&local_8c.m_Memory.m_pMemory[-1].m_teleportationHistory.m_Memory.m_pMemory
                    + iVar18) != 0) {
          (**(code **)(*(int *)*puVar20 + 8))();
          *(undefined4 *)((int)&pBVar8[-1].m_teleportationHistory.m_Memory.m_pMemory + iVar18) = 0;
        }
        iVar17 = 0;
        *(undefined4 *)
         ((int)&pBVar8[-1].m_teleportationHistory.m_Memory.m_nAllocationCount + iVar18) = 0;
      }
      *(int *)((int)&pBVar8[-1].m_teleportationHistory.m_pElements + iVar18) = iVar17;
      *(undefined4 *)((int)&pBVar8[-1].m_teleportationHistory.m_Size + iVar18) = 0;
      if (-1 < *(int *)((int)&pBVar8[-1].m_teleportationHistory.m_Memory.m_nGrowSize + iVar18)) {
        if (iVar17 != 0) {
          (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x63b13f) + 8))();
          iVar17 = 0;
          *(undefined4 *)((int)&pBVar8[-1].m_teleportationHistory.m_Memory.m_pMemory + iVar18) = 0;
        }
        *(undefined4 *)
         ((int)&pBVar8[-1].m_teleportationHistory.m_Memory.m_nAllocationCount + iVar18) = 0;
      }
      *(int *)((int)&pBVar8[-1].m_teleportationHistory.m_pElements + iVar18) = iVar17;
      iVar14 = iVar18 + -0x2c;
      if (-1 < *(int *)((int)&pBVar8[-1].m_teleportationHistory.m_Memory.m_nGrowSize + iVar18)) {
        if (iVar17 != 0) {
          (**(code **)(*(int *)*puVar20 + 8))();
          *(undefined4 *)((int)&pBVar8[-1].m_teleportationHistory.m_Memory.m_pMemory + iVar18) = 0;
        }
        *(undefined4 *)
         ((int)&pBVar8[-1].m_teleportationHistory.m_Memory.m_nAllocationCount + iVar18) = 0;
      }
    }
    local_8c.m_Size = 0;
    if (-1 < local_8c.m_Memory.m_nGrowSize) {
      if (local_8c.m_Memory.m_pMemory != (BlobData_t *)0x0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x63b13f) + 8))();
        local_8c.m_Memory.m_pMemory = (BlobData_t *)0x0;
      }
      local_8c.m_Memory.m_nAllocationCount = 0;
    }
    local_8c.m_pElements = local_8c.m_Memory.m_pMemory;
    if (-1 < local_8c.m_Memory.m_nGrowSize) {
      if (local_8c.m_Memory.m_pMemory != (BlobData_t *)0x0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x63b13f) + 8))();
        local_8c.m_Memory.m_pMemory = (BlobData_t *)0x0;
      }
      local_8c.m_Memory.m_nAllocationCount = 0;
    }
LAB_0052990f:
    puVar20 = (undefined4 *)&stack0xfffffed0;
    puVar21 = &stack0xfffffed0;
    local_78.m_Size = 0;
    if (-1 < local_78.m_Memory.m_nGrowSize) {
      if (local_78.m_Memory.m_pMemory != (matrix3x4_t *)0x0) {
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x63b13f) + 8))();
        local_78.m_Memory.m_pMemory = (matrix3x4_t *)0x0;
      }
      local_78.m_Memory.m_nAllocationCount = 0;
    }
    local_78.m_Size = 0;
    if (local_78.m_Memory.m_nGrowSize < 0) goto LAB_00528e9d;
    puVar20 = (undefined4 *)&stack0xfffffed0;
    if (local_78.m_Memory.m_pMemory != (matrix3x4_t *)0x0) {
      local_78.m_pElements = local_78.m_Memory.m_pMemory;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x63b13f) + 8))();
      goto LAB_00528ee8;
    }
  }
  local_78.m_Memory.m_nAllocationCount = 0;
LAB_00528e9d:
  if ((-1 < local_78.m_Memory.m_nGrowSize) && (local_78.m_Memory.m_pMemory != (matrix3x4_t *)0x0)) {
    piVar11 = (int *)**(undefined4 **)(unaff_EBX + 0x63b13f);
    iVar14 = *piVar11;
    puVar20[1] = local_78.m_Memory.m_pMemory;
    *puVar20 = piVar11;
    pcVar3 = *(code **)(iVar14 + 8);
    puVar20[-1] = 0x528ebf;
    local_78.m_pElements = local_78.m_Memory.m_pMemory;
    (*pcVar3)();
  }
  return;
}


/* C_PaintStream::DrawModel at 0052a000 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: int DrawModel(C_PaintStream * this, int flags, RenderableInstance_t *
   instance) */

int __thiscall
C_PaintStream::DrawModel(C_PaintStream *this,int flags,RenderableInstance_t *instance)

{
  undefined4 *puVar1;
  code *pcVar2;
  char cVar3;
  char *pcVar4;
  IMaterial *pMaterial;
  view_id_t vVar5;
  int iVar6;
  int *piVar7;
  ITexture *pIVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  int iVar12;
  int iVar13;
  int unaff_EBX;
  int local_48;
  int local_44;
  int local_40;
  int local_3c;
  int local_38;
  int local_34;
  int local_30;
  int local_2c;
  int local_28;
  int local_24;
  int *local_20;
  
                    /* Unresolved local var: IMaterial * pMaterial@[???] */
  ___i686_get_pc_thunk_bx();
  if (((this->super_C_BaseAnimating).super_C_BaseEntity.m_bReadyToDraw == false) ||
     (((this->m_blobs).m_Size == 0 &&
      (cVar3 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x639d90) + 0x2f0))
                         ((int *)**(undefined4 **)(unaff_EBX + 0x639d90)), cVar3 == '\0')))) {
    iVar6 = 0;
  }
  else {
    puVar1 = *(undefined4 **)(unaff_EBX + 0x639ef8);
    pcVar2 = *(code **)(*(int *)*puVar1 + 0x130);
    pcVar4 = CPaintStreamManager::GetPaintMaterialName
                       (*(CPaintStreamManager **)(unaff_EBX + 0x639f64),(this->m_nPaintType).m_Value
                       );
    pMaterial = (IMaterial *)(*pcVar2)(*puVar1,pcVar4,unaff_EBX + 0x47d600,1,0);
    vVar5 = CurrentViewID();
    if ((vVar5 != VIEW_SHADOW_DEPTH_TEXTURE) &&
       (iVar6 = (*pMaterial->_vptr_IMaterial[0x19])(pMaterial,1), (char)iVar6 != '\0')) {
                    /* Unresolved local var: CViewSetup * pViewSetup@[???] */
      piVar7 = (int *)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x639e30) + 0x34))
                                ((int *)**(undefined4 **)(unaff_EBX + 0x639e30));
      local_2c = piVar7[3];
      local_30 = piVar7[2];
      local_34 = piVar7[1];
                    /* Unresolved local var: Rect_t srcRect@[???]
                       Unresolved local var: CMatRenderContextPtr pRenderContext@[???]
                       Unresolved local var: ITexture * pTexture@[???]
                       Unresolved local var: int nSrcWidth@[???]
                       Unresolved local var: int nSrcHeight@[???]
                       Unresolved local var: int nDestWidth@[???]
                       Unresolved local var: int nDestHeight@[???]
                       Unresolved local var: Rect_t destRect@[???] */
      local_38 = *piVar7;
      local_20 = (int *)(**(code **)(*(int *)*puVar1 + 0x1a0))((int *)*puVar1);
      if (local_20 != (int *)0x0) {
        (**(code **)(*local_20 + 8))(local_20);
      }
      pIVar8 = GetFullFrameFrameBufferTexture(0);
      (**(code **)(*local_20 + 0x20))(local_20,&local_24,&local_28);
      iVar6 = (*pIVar8->_vptr_ITexture[3])(pIVar8);
      iVar9 = (*pIVar8->_vptr_ITexture[4])(pIVar8);
      local_48 = local_38;
      local_44 = local_34;
      local_40 = local_30;
      local_3c = local_2c;
      if ((iVar6 < local_24) || (iVar9 < local_28)) {
        iVar13 = (int)((float)local_38 * ((float)iVar6 / (float)local_24));
        iVar10 = (int)((float)local_34 * ((float)iVar9 / (float)local_28));
        iVar11 = (int)((float)local_30 * ((float)iVar6 / (float)local_24));
        iVar12 = (int)((float)local_2c * ((float)iVar9 / (float)local_28));
        local_48 = iVar6;
        if ((iVar13 <= iVar6) && (local_48 = 0, -1 < iVar13)) {
          local_48 = iVar13;
        }
        local_44 = iVar9;
        if ((iVar10 <= iVar9) && (local_44 = 0, -1 < iVar10)) {
          local_44 = iVar10;
        }
        local_40 = iVar6 - local_48;
        if ((iVar11 <= iVar6 - local_48) && (local_40 = 0, -1 < iVar11)) {
          local_40 = iVar11;
        }
        local_3c = iVar9 - local_44;
        if ((iVar12 <= iVar9 - local_44) && (local_3c = 0, -1 < iVar12)) {
          local_3c = iVar12;
        }
      }
      (**(code **)(*local_20 + 0x1bc))(local_20,pIVar8,0,&local_38,&local_48);
      (**(code **)(*local_20 + 0x48))(local_20,pIVar8,0);
      if (local_20 != (int *)0x0) {
        (**(code **)(*local_20 + 0xc))(local_20);
      }
      if (local_20 != (int *)0x0) {
        (**(code **)(*local_20 + 4))(local_20);
      }
    }
    DrawBlobs(this,pMaterial);
    iVar6 = 1;
  }
  return iVar6;
}


/* InterpolateTeleportingBlob at 00527b20 */

void InterpolateTeleportingBlob
               (float flPercent,BlobData_t *blobA,BlobData_t *blobB,BlobDataVector_t *blobDataVector
               )

{
  int iVar1;
  BlobTeleportationHistory_t *pBVar2;
  uint uVar3;
  BlobTeleportationHistory_t *pBVar4;
  int iVar5;
  BlobDataVector_t *pBVar6;
  BlobData_t *pBVar7;
  int unaff_EBX;
  int iVar8;
  float fVar9;
  float fVar10;
  float fVar11;
  float fVar12;
  int aiStack_100 [5];
  float afStack_ec [2];
  int local_e4;
  float *local_e0;
  float *local_dc;
  BlobDataVector_t *local_d8;
  BlobData_t *local_d4;
  BlobData_t *local_d0;
  int nHistoryCount;
  float local_c8;
  float local_c4;
  float local_c0;
  float local_bc;
  int local_b8;
  float local_b4;
  float local_b0;
  float local_ac;
  float local_a8;
  float local_a4;
  float local_a0;
  __m128 root;
  
                    /* Unresolved local var: BlobTeleportationHistoryVector_t * history@[???]
                       Unresolved local var: float * distStack@[???]
                       Unresolved local var: int nLastHistory@[???]
                       Unresolved local var: float flTotalDistance@[???]
                       Unresolved local var: float flQueryDist@[???]
                       Unresolved local var: BlobData_t blobData@[???]
                       Unresolved local var: float flPositionPercent@[???] */
  aiStack_100[0] = 0x527b31;
  ___i686_get_pc_thunk_bx();
  local_d0 = blobA;
  local_d4 = blobB;
  local_d8 = blobDataVector;
  iVar1 = **(int **)(unaff_EBX + 0x63c22f);
  nHistoryCount = (blobB->m_teleportationHistory).m_Size;
  iVar5 = -((nHistoryCount * 4 + 0x13U & 0xfffffff0) + 0x10);
  local_dc = (float *)((int)afStack_ec + iVar5);
  local_e4 = nHistoryCount + -1;
  pBVar2 = (blobB->m_teleportationHistory).m_Memory.m_pMemory;
                    /* Unresolved local var: Vector res@[???] */
  fVar11 = (blobA->m_vPosition).x - (pBVar2->m_vEnterPosition).x;
  fVar9 = (blobA->m_vPosition).y - (pBVar2->m_vEnterPosition).y;
  fVar10 = (blobA->m_vPosition).z - (pBVar2->m_vEnterPosition).z;
  fVar9 = SQRT(fVar11 * fVar11 + fVar9 * fVar9 + fVar10 * fVar10);
  local_e0 = root;
  *(float *)((int)afStack_ec + iVar5) = fVar9;
  pBVar6 = local_d8;
                    /* Unresolved local var: int i@[???] */
  if (0 < local_e4) {
    iVar8 = 1;
    do {
                    /* Unresolved local var: Vector res@[???] */
      fVar10 = *local_e0;
      local_dc[iVar8] = fVar10;
      fVar9 = fVar9 + fVar10;
      iVar8 = iVar8 + 1;
    } while (iVar8 != nHistoryCount);
  }
  pBVar2 = (local_d4->m_teleportationHistory).m_Memory.m_pMemory;
                    /* Unresolved local var: Vector res@[???] */
  fVar12 = pBVar2[local_e4].m_vExitPosition.x - (local_d4->m_vPosition).x;
  fVar10 = pBVar2[local_e4].m_vExitPosition.y - (local_d4->m_vPosition).y;
  fVar11 = pBVar2[local_e4].m_vExitPosition.z - (local_d4->m_vPosition).z;
  fVar10 = SQRT(fVar12 * fVar12 + fVar10 * fVar10 + fVar11 * fVar11);
  root[0] = fVar10;
  root[1] = 0.0;
  root[2] = 0.0;
  root[3] = 0.0;
  local_dc[nHistoryCount] = fVar10;
  fVar9 = (fVar9 + fVar10) * flPercent;
  local_b8 = local_d0->m_blobID;
  local_c8 = local_d0->m_flScale + flPercent * (local_d4->m_flScale - local_d0->m_flScale);
  if (fVar9 < *local_dc) {
    fVar9 = fVar9 / *local_dc;
    pBVar2 = (local_d4->m_teleportationHistory).m_Memory.m_pMemory;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
    local_bc = (local_d0->m_vPosition).x +
               ((pBVar2->m_vEnterPosition).x - (local_d0->m_vPosition).x) * fVar9;
    local_c0 = (local_d0->m_vPosition).y +
               ((pBVar2->m_vEnterPosition).y - (local_d0->m_vPosition).y) * fVar9;
    local_c4 = (local_d0->m_vPosition).z +
               fVar9 * ((pBVar2->m_vEnterPosition).z - (local_d0->m_vPosition).z);
    uVar3 = local_d8->m_Size;
    *(undefined4 *)((int)aiStack_100 + iVar5 + 8) = 1;
    *(BlobDataVector_t **)((int)aiStack_100 + iVar5 + 4) = pBVar6;
    *(undefined4 *)((int)aiStack_100 + iVar5) = 0x528005;
    CUtlVector<BlobData_t,CUtlMemory<BlobData_t,_int>_>::GrowVector
              (*(CUtlVector<BlobData_t,CUtlMemory<BlobData_t,_int>_> **)
                ((int)aiStack_100 + iVar5 + 4),*(int *)((int)aiStack_100 + iVar5 + 8));
    iVar8 = ~uVar3 + local_d8->m_Size;
    if (0 < iVar8) {
      pBVar7 = (local_d8->m_Memory).m_pMemory;
      *(int *)((int)aiStack_100 + iVar5 + 0xc) = iVar8 * 0x2c;
      *(BlobData_t **)((int)aiStack_100 + iVar5 + 8) = pBVar7 + uVar3;
      *(BlobData_t **)((int)aiStack_100 + iVar5 + 4) = pBVar7 + uVar3 + 1;
      *(undefined4 *)((int)aiStack_100 + iVar5) = 0x5282c3;
      _V_memmove(*(void **)((int)aiStack_100 + iVar5 + 4),*(void **)((int)aiStack_100 + iVar5 + 8),
                 *(int *)((int)aiStack_100 + iVar5 + 0xc));
    }
    pBVar7 = (local_d8->m_Memory).m_pMemory + uVar3;
    if (pBVar7 != (BlobData_t *)0x0) {
      (pBVar7->m_teleportationHistory).m_Memory.m_pMemory = (BlobTeleportationHistory_t *)0x0;
      (pBVar7->m_teleportationHistory).m_Memory.m_nAllocationCount = 0;
      (pBVar7->m_teleportationHistory).m_Memory.m_nGrowSize = 0;
      (pBVar7->m_teleportationHistory).m_Size = 0;
      (pBVar7->m_teleportationHistory).m_pElements = (BlobTeleportationHistory_t *)0x0;
      pBVar7->m_blobID = local_b8;
      (pBVar7->m_vPosition).x = local_bc;
      (pBVar7->m_vPosition).y = local_c0;
      (pBVar7->m_vPosition).z = local_c4;
      pBVar7->m_flScale = local_c8;
      pBVar7->m_bTeleportedThisFrame = false;
    }
  }
  else {
    iVar8 = 0;
    if (0 < local_e4) {
      do {
        if ((local_dc[iVar8] <= fVar9) && (fVar9 < local_dc[iVar8 + 1])) {
          fVar9 = (fVar9 - local_dc[iVar8]) / local_dc[iVar8 + 1];
          pBVar4 = (local_d4->m_teleportationHistory).m_Memory.m_pMemory;
          pBVar2 = pBVar4 + iVar8 + 1;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
          local_a4 = pBVar4[iVar8].m_vExitPosition.x +
                     ((pBVar2->m_vEnterPosition).x - pBVar4[iVar8].m_vExitPosition.x) * fVar9;
          local_ac = pBVar4[iVar8].m_vExitPosition.y +
                     ((pBVar2->m_vEnterPosition).y - pBVar4[iVar8].m_vExitPosition.y) * fVar9;
          local_b4 = pBVar4[iVar8].m_vExitPosition.z +
                     fVar9 * ((pBVar2->m_vEnterPosition).z - pBVar4[iVar8].m_vExitPosition.z);
          uVar3 = local_d8->m_Size;
          *(undefined4 *)((int)aiStack_100 + iVar5 + 8) = 1;
          *(BlobDataVector_t **)((int)aiStack_100 + iVar5 + 4) = pBVar6;
          *(undefined4 *)((int)aiStack_100 + iVar5) = 0x528178;
          CUtlVector<BlobData_t,CUtlMemory<BlobData_t,_int>_>::GrowVector
                    (*(CUtlVector<BlobData_t,CUtlMemory<BlobData_t,_int>_> **)
                      ((int)aiStack_100 + iVar5 + 4),*(int *)((int)aiStack_100 + iVar5 + 8));
          iVar8 = ~uVar3 + local_d8->m_Size;
          if (0 < iVar8) {
            pBVar7 = (local_d8->m_Memory).m_pMemory;
            *(int *)((int)aiStack_100 + iVar5 + 0xc) = iVar8 * 0x2c;
            *(BlobData_t **)((int)aiStack_100 + iVar5 + 8) = pBVar7 + uVar3;
            *(BlobData_t **)((int)aiStack_100 + iVar5 + 4) = pBVar7 + uVar3 + 1;
            *(undefined4 *)((int)aiStack_100 + iVar5) = 0x52824b;
            _V_memmove(*(void **)((int)aiStack_100 + iVar5 + 4),
                       *(void **)((int)aiStack_100 + iVar5 + 8),
                       *(int *)((int)aiStack_100 + iVar5 + 0xc));
          }
          pBVar7 = (local_d8->m_Memory).m_pMemory + uVar3;
          if (pBVar7 != (BlobData_t *)0x0) {
            (pBVar7->m_teleportationHistory).m_Memory.m_pMemory = (BlobTeleportationHistory_t *)0x0;
            (pBVar7->m_teleportationHistory).m_Memory.m_nAllocationCount = 0;
            (pBVar7->m_teleportationHistory).m_Memory.m_nGrowSize = 0;
            (pBVar7->m_teleportationHistory).m_Size = 0;
            (pBVar7->m_teleportationHistory).m_pElements = (BlobTeleportationHistory_t *)0x0;
            pBVar7->m_blobID = local_b8;
            (pBVar7->m_vPosition).x = local_a4;
            (pBVar7->m_vPosition).y = local_ac;
            (pBVar7->m_vPosition).z = local_b4;
            pBVar7->m_flScale = local_c8;
            pBVar7->m_bTeleportedThisFrame = false;
          }
          goto LAB_00527f23;
        }
        iVar8 = iVar8 + 1;
      } while (iVar8 != local_e4);
    }
    fVar10 = (fVar9 - local_dc[local_e4]) / fVar10;
    pBVar2 = (local_d4->m_teleportationHistory).m_Memory.m_pMemory;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
    local_a0 = pBVar2[local_e4].m_vExitPosition.x +
               ((local_d4->m_vPosition).x - pBVar2[local_e4].m_vExitPosition.x) * fVar10;
    local_a8 = pBVar2[local_e4].m_vExitPosition.y +
               ((local_d4->m_vPosition).y - pBVar2[local_e4].m_vExitPosition.y) * fVar10;
    local_b0 = pBVar2[local_e4].m_vExitPosition.z +
               fVar10 * ((local_d4->m_vPosition).z - pBVar2[local_e4].m_vExitPosition.z);
    uVar3 = local_d8->m_Size;
    *(undefined4 *)((int)aiStack_100 + iVar5 + 8) = 1;
    *(BlobDataVector_t **)((int)aiStack_100 + iVar5 + 4) = pBVar6;
    *(undefined4 *)((int)aiStack_100 + iVar5) = 0x527e90;
    CUtlVector<BlobData_t,CUtlMemory<BlobData_t,_int>_>::GrowVector
              (*(CUtlVector<BlobData_t,CUtlMemory<BlobData_t,_int>_> **)
                ((int)aiStack_100 + iVar5 + 4),*(int *)((int)aiStack_100 + iVar5 + 8));
    iVar8 = ~uVar3 + local_d8->m_Size;
    if (0 < iVar8) {
      pBVar7 = (local_d8->m_Memory).m_pMemory;
      *(int *)((int)aiStack_100 + iVar5 + 0xc) = iVar8 * 0x2c;
      *(BlobData_t **)((int)aiStack_100 + iVar5 + 8) = pBVar7 + uVar3;
      *(BlobData_t **)((int)aiStack_100 + iVar5 + 4) = pBVar7 + uVar3 + 1;
      *(undefined4 *)((int)aiStack_100 + iVar5) = 0x528287;
      _V_memmove(*(void **)((int)aiStack_100 + iVar5 + 4),*(void **)((int)aiStack_100 + iVar5 + 8),
                 *(int *)((int)aiStack_100 + iVar5 + 0xc));
    }
    pBVar7 = (local_d8->m_Memory).m_pMemory + uVar3;
    if (pBVar7 != (BlobData_t *)0x0) {
      (pBVar7->m_teleportationHistory).m_Memory.m_pMemory = (BlobTeleportationHistory_t *)0x0;
      (pBVar7->m_teleportationHistory).m_Memory.m_nAllocationCount = 0;
      (pBVar7->m_teleportationHistory).m_Memory.m_nGrowSize = 0;
      (pBVar7->m_teleportationHistory).m_Size = 0;
      (pBVar7->m_teleportationHistory).m_pElements = (BlobTeleportationHistory_t *)0x0;
      pBVar7->m_blobID = local_b8;
      (pBVar7->m_vPosition).x = local_a0;
      (pBVar7->m_vPosition).y = local_a8;
      (pBVar7->m_vPosition).z = local_b0;
      pBVar7->m_flScale = local_c8;
      pBVar7->m_bTeleportedThisFrame = false;
    }
  }
LAB_00527f23:
  if (iVar1 != **(int **)(unaff_EBX + 0x63c22f)) {
                    /* WARNING: Subroutine does not return */
    *(undefined **)((int)aiStack_100 + iVar5) = &UNK_005282cd;
    ___stack_chk_fail();
  }
  return;
}


/* InterpolateBlobData at 005282e0 */

/* WARNING: Type propagation algorithm not settling */

void InterpolateBlobData(float flPercent,BlobDataVector_t *dataA,BlobDataVector_t *dataB,
                        BlobDataVector_t *blobDataVector)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  float fVar10;
  float fVar11;
  int iVar12;
  BlobData_t *pBVar13;
  int iVar14;
  uint uVar15;
  BlobData_t *pBVar16;
  int a;
  int b;
  
  iVar14 = dataA->m_Size;
  a = 0;
  b = 0;
joined_r0x00528303:
  if (a < iVar14) {
    pBVar16 = (dataA->m_Memory).m_pMemory + a;
    if ((dataB->m_Size <= b) ||
       (pBVar13 = (dataB->m_Memory).m_pMemory + b, pBVar13 == (BlobData_t *)0x0)) {
      if (pBVar16 == (BlobData_t *)0x0) goto LAB_00528314;
      uVar15 = blobDataVector->m_Size;
      CUtlVector<BlobData_t,CUtlMemory<BlobData_t,_int>_>::GrowVector(blobDataVector,1);
      iVar14 = ~uVar15 + blobDataVector->m_Size;
      if (0 < iVar14) {
        pBVar13 = (blobDataVector->m_Memory).m_pMemory;
LAB_005283ef:
        _V_memmove(pBVar13 + uVar15 + 1,pBVar13 + uVar15,iVar14 * 0x2c);
      }
LAB_0052837a:
      pBVar13 = (blobDataVector->m_Memory).m_pMemory + uVar15;
      if (pBVar13 != (BlobData_t *)0x0) {
        (pBVar13->m_teleportationHistory).m_Memory.m_pMemory = (BlobTeleportationHistory_t *)0x0;
        (pBVar13->m_teleportationHistory).m_Memory.m_nAllocationCount = 0;
        (pBVar13->m_teleportationHistory).m_Memory.m_nGrowSize = 0;
        (pBVar13->m_teleportationHistory).m_Size = 0;
        (pBVar13->m_teleportationHistory).m_pElements = (BlobTeleportationHistory_t *)0x0;
        pBVar13->m_blobID = pBVar16->m_blobID;
        (pBVar13->m_vPosition).x = (pBVar16->m_vPosition).x;
        (pBVar13->m_vPosition).y = (pBVar16->m_vPosition).y;
        (pBVar13->m_vPosition).z = (pBVar16->m_vPosition).z;
        pBVar13->m_flScale = pBVar16->m_flScale;
        pBVar13->m_bTeleportedThisFrame = pBVar16->m_bTeleportedThisFrame;
      }
      a = a + 1;
      iVar14 = dataA->m_Size;
      goto joined_r0x00528303;
    }
    if (pBVar16 != (BlobData_t *)0x0) {
      iVar12 = pBVar16->m_blobID;
      if (iVar12 != pBVar13->m_blobID) {
        if (iVar12 < pBVar13->m_blobID) {
          uVar15 = blobDataVector->m_Size;
          CUtlVector<BlobData_t,CUtlMemory<BlobData_t,_int>_>::GrowVector(blobDataVector,1);
                    /* Unresolved local var: int numToMove@[???] */
          iVar14 = ~uVar15 + blobDataVector->m_Size;
          if (0 < iVar14) {
            pBVar13 = (blobDataVector->m_Memory).m_pMemory;
            goto LAB_005283ef;
          }
          goto LAB_0052837a;
        }
        goto LAB_00528314;
      }
      if (pBVar13->m_bTeleportedThisFrame == false) {
        fVar1 = pBVar16->m_flScale;
        fVar2 = pBVar13->m_flScale;
                    /* Unresolved local var: Vector res@[???] */
        fVar3 = (pBVar13->m_vPosition).y;
        fVar4 = (pBVar16->m_vPosition).y;
        fVar5 = (pBVar13->m_vPosition).z;
        fVar6 = (pBVar16->m_vPosition).z;
                    /* Unresolved local var: Vector res@[???] */
        fVar7 = (pBVar13->m_vPosition).x;
        fVar8 = (pBVar16->m_vPosition).x;
                    /* Unresolved local var: Vector res@[???] */
        fVar9 = (pBVar16->m_vPosition).x;
        fVar10 = (pBVar16->m_vPosition).y;
        fVar11 = (pBVar16->m_vPosition).z;
        uVar15 = blobDataVector->m_Size;
        CUtlVector<BlobData_t,CUtlMemory<BlobData_t,_int>_>::GrowVector(blobDataVector,1);
        iVar14 = ~uVar15 + blobDataVector->m_Size;
        if (0 < iVar14) {
          pBVar16 = (blobDataVector->m_Memory).m_pMemory;
          _V_memmove(pBVar16 + uVar15 + 1,pBVar16 + uVar15,iVar14 * 0x2c);
        }
        pBVar16 = (blobDataVector->m_Memory).m_pMemory + uVar15;
        if (pBVar16 != (BlobData_t *)0x0) {
          (pBVar16->m_teleportationHistory).m_Memory.m_pMemory = (BlobTeleportationHistory_t *)0x0;
          (pBVar16->m_teleportationHistory).m_Memory.m_nAllocationCount = 0;
          (pBVar16->m_teleportationHistory).m_Memory.m_nGrowSize = 0;
          (pBVar16->m_teleportationHistory).m_Size = 0;
          (pBVar16->m_teleportationHistory).m_pElements = (BlobTeleportationHistory_t *)0x0;
          pBVar16->m_blobID = iVar12;
          (pBVar16->m_vPosition).x = fVar9 + (fVar7 - fVar8) * flPercent;
          (pBVar16->m_vPosition).y = fVar10 + (fVar3 - fVar4) * flPercent;
          (pBVar16->m_vPosition).z = fVar11 + (fVar5 - fVar6) * flPercent;
          pBVar16->m_flScale = fVar1 + flPercent * (fVar2 - fVar1);
          pBVar16->m_bTeleportedThisFrame = false;
        }
      }
      else {
        InterpolateTeleportingBlob(flPercent,pBVar16,pBVar13,blobDataVector);
      }
      a = a + 1;
      b = b + 1;
      iVar14 = dataA->m_Size;
      goto joined_r0x00528303;
    }
  }
  else if (dataB->m_Size <= b) {
    return;
  }
LAB_00528314:
  b = b + 1;
  goto joined_r0x00528303;
}


/* C_PaintStream::GetCurrentBlobData at 00528650 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void GetCurrentBlobData(C_PaintStream * this, BlobDataVector_t *
   blobData) */

void __thiscall C_PaintStream::GetCurrentBlobData(C_PaintStream *this,BlobDataVector_t *blobData)

{
  CUtlVector<BlobDataPerUpdate_t,CUtlMemory<BlobDataPerUpdate_t,_int>_> *pCVar1;
  BlobDataPerUpdate_t *pBVar2;
  BlobData_t *pBVar3;
  undefined4 *puVar4;
  int *piVar5;
  BlobData_t *pBVar6;
  void *src;
  undefined4 uVar7;
  undefined4 uVar8;
  BlobDataPerUpdate_t *pBVar9;
  undefined4 uVar10;
  int iVar11;
  int iVar12;
  undefined4 *puVar13;
  undefined4 *puVar14;
  int unaff_EBX;
  undefined4 *puVar15;
  int iVar16;
  int iVar17;
  float fVar18;
  float fVar19;
  float fVar20;
  int local_44;
  int local_40;
  int local_30;
  int local_20;
  
                    /* Unresolved local var: int nTotalFrame@[???]
                       Unresolved local var: float flCurrentTime@[???]
                       Unresolved local var: float flOldestDataTime@[???]
                       Unresolved local var: int nCurrentFrame@[???]
                       Unresolved local var: int nLastFrame@[???]
                       Unresolved local var: float flTimeA@[???]
                       Unresolved local var: float flTimeB@[???] */
  ___i686_get_pc_thunk_bx();
  pCVar1 = (this->m_sharedBlobData).
           super_CSharedVarBase<CUtlVector<BlobDataPerUpdate_t,_CUtlMemory<BlobDataPerUpdate_t,_int>_>,C_PaintStream::SharedVar_m_sharedBlobData>
           .m_pValue;
  iVar16 = pCVar1->m_Size;
  if (iVar16 != 0) {
    fVar20 = *(float *)(**(int **)(unaff_EBX + 0x63b73e) + 0xc) -
             *(float *)(*(int *)(unaff_EBX + 0x840a1e) + 0x2c);
    pBVar2 = (pCVar1->m_Memory).m_pMemory;
    if (pBVar2->m_flUpdateTime <= fVar20) {
      iVar16 = iVar16 + -1;
      if (iVar16 < 1) {
        iVar17 = 0;
        fVar18 = 0.0;
        fVar19 = 0.0;
        pBVar9 = pBVar2;
      }
      else {
        iVar17 = 0;
        iVar11 = 0;
        iVar12 = 0x18;
        fVar19 = pBVar2->m_flUpdateTime;
        do {
          fVar18 = fVar19;
          iVar17 = iVar17 + 1;
          pBVar9 = (BlobDataPerUpdate_t *)((int)&pBVar2->m_flUpdateTime + iVar12);
          fVar19 = pBVar9->m_flUpdateTime;
          if ((fVar18 < fVar20) && (fVar20 < fVar19)) goto LAB_005286fb;
          iVar11 = iVar11 + 0x18;
          iVar12 = iVar12 + 0x18;
        } while (iVar17 != iVar16);
      }
      if (iVar17 == iVar16) {
        iVar16 = (pBVar9->m_blobData).m_Size;
        pBVar3 = (pBVar9->m_blobData).m_Memory.m_pMemory;
        local_40 = blobData->m_Size;
        puVar4 = *(undefined4 **)(unaff_EBX + 0x63b712);
        iVar17 = local_40 * 0x2c;
        while( true ) {
          iVar11 = iVar17;
          local_40 = local_40 + -1;
          if (local_40 < 0) break;
          pBVar6 = (blobData->m_Memory).m_pMemory;
          *(undefined4 *)((int)&pBVar6[-1].m_teleportationHistory.m_Size + iVar11) = 0;
          if (*(int *)((int)&pBVar6[-1].m_teleportationHistory.m_Memory.m_nGrowSize + iVar11) < 0) {
            iVar12 = *(int *)((int)&pBVar6[-1].m_teleportationHistory.m_Memory.m_pMemory + iVar11);
          }
          else {
            iVar17 = *(int *)((int)&pBVar6[-1].m_teleportationHistory.m_Memory.m_pMemory + iVar11);
            if (iVar17 != 0) {
              piVar5 = (int *)*puVar4;
              (**(code **)(*piVar5 + 8))(piVar5,iVar17);
              *(undefined4 *)((int)&pBVar6[-1].m_teleportationHistory.m_Memory.m_pMemory + iVar11) =
                   0;
            }
            iVar12 = 0;
            *(undefined4 *)
             ((int)&pBVar6[-1].m_teleportationHistory.m_Memory.m_nAllocationCount + iVar11) = 0;
          }
          *(int *)((int)&pBVar6[-1].m_teleportationHistory.m_pElements + iVar11) = iVar12;
          *(undefined4 *)((int)&pBVar6[-1].m_teleportationHistory.m_Size + iVar11) = 0;
          if (-1 < *(int *)((int)&pBVar6[-1].m_teleportationHistory.m_Memory.m_nGrowSize + iVar11))
          {
            if (iVar12 != 0) {
              (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x63b712) + 8))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x63b712),iVar12);
              iVar12 = 0;
              *(undefined4 *)((int)&pBVar6[-1].m_teleportationHistory.m_Memory.m_pMemory + iVar11) =
                   0;
            }
            *(undefined4 *)
             ((int)&pBVar6[-1].m_teleportationHistory.m_Memory.m_nAllocationCount + iVar11) = 0;
          }
          *(int *)((int)&pBVar6[-1].m_teleportationHistory.m_pElements + iVar11) = iVar12;
          iVar17 = iVar11 + -0x2c;
          if (-1 < *(int *)((int)&pBVar6[-1].m_teleportationHistory.m_Memory.m_nGrowSize + iVar11))
          {
            if (iVar12 != 0) {
              piVar5 = (int *)*puVar4;
              (**(code **)(*piVar5 + 8))(piVar5,iVar12);
              *(undefined4 *)((int)&pBVar6[-1].m_teleportationHistory.m_Memory.m_pMemory + iVar11) =
                   0;
            }
            *(undefined4 *)
             ((int)&pBVar6[-1].m_teleportationHistory.m_Memory.m_nAllocationCount + iVar11) = 0;
          }
        }
        blobData->m_Size = 0;
        if (iVar16 != 0) {
          CUtlVector<BlobData_t,CUtlMemory<BlobData_t,_int>_>::GrowVector(blobData,iVar16);
                    /* Unresolved local var: int numToMove@[???] */
          iVar17 = blobData->m_Size - iVar16;
          if (0 < iVar17) {
            if (0 >= iVar16) {
              return;
            }
            pBVar6 = (blobData->m_Memory).m_pMemory;
            _V_memmove(pBVar6 + iVar16,pBVar6,iVar17 * 0x2c);
          }
                    /* Unresolved local var: int i@[???] */
          if (0 < iVar16) {
            iVar11 = 0;
            iVar17 = 0;
            puVar4 = *(undefined4 **)(unaff_EBX + 0x63b7a2);
            do {
              puVar13 = (undefined4 *)((int)&((blobData->m_Memory).m_pMemory)->m_blobID + iVar17);
              if (puVar13 != (undefined4 *)0x0) {
                puVar13[6] = 0;
                puVar13[7] = 0;
                puVar13[8] = 0;
                puVar13[9] = 0;
                puVar13[10] = 0;
                *puVar13 = 0;
                puVar13[1] = *puVar4;
                puVar13[2] = puVar4[1];
                puVar13[3] = puVar4[2];
                puVar13[4] = 0;
                *(undefined1 *)(puVar13 + 5) = 0;
              }
              iVar11 = iVar11 + 1;
              iVar17 = iVar17 + 0x2c;
            } while (iVar11 < iVar16);
            local_44 = 0;
            local_20 = 0;
            puVar4 = *(undefined4 **)(unaff_EBX + 0x63b712);
            do {
              puVar13 = (undefined4 *)((int)&((blobData->m_Memory).m_pMemory)->m_blobID + local_20);
              *puVar13 = *(undefined4 *)((int)&pBVar3->m_blobID + local_20);
              puVar13[1] = *(undefined4 *)((int)&(pBVar3->m_vPosition).x + local_20);
              puVar13[2] = *(undefined4 *)((int)&(pBVar3->m_vPosition).y + local_20);
              puVar13[3] = *(undefined4 *)((int)&(pBVar3->m_vPosition).z + local_20);
              puVar13[4] = *(undefined4 *)((int)&pBVar3->m_flScale + local_20);
              *(bool *)(puVar13 + 5) = (&pBVar3->m_bTeleportedThisFrame)[local_20];
              iVar17 = *(int *)((int)&(pBVar3->m_teleportationHistory).m_Size + local_20);
              puVar13[9] = 0;
              if (iVar17 != 0) {
                iVar11 = puVar13[7];
                if (iVar11 < iVar17) {
                  iVar12 = puVar13[8];
                  if (-1 < iVar12) {
                    if (iVar12 == 0) {
                      if (iVar11 == 0) {
                        iVar11 = 2;
                        iVar12 = 0x30;
                        if (iVar17 < 3) goto LAB_005289ec;
                      }
                      do {
                        iVar11 = iVar11 * 2;
                      } while (iVar11 < iVar17);
LAB_005289e2:
                      iVar12 = iVar11 * 0x18;
                    }
                    else {
                      iVar11 = ((iVar17 + -1) / iVar12 + 1) * iVar12;
                      if (iVar17 <= iVar11) goto LAB_005289e2;
                      if ((iVar11 != 0) || (-1 < iVar17)) {
                        do {
                          iVar11 = (iVar11 + iVar17) / 2;
                        } while (iVar11 < iVar17);
                        goto LAB_005289e2;
                      }
                      iVar11 = -1;
                      iVar12 = -0x18;
                    }
LAB_005289ec:
                    puVar13[7] = iVar11;
                    if (puVar13[6] == 0) {
                      uVar10 = (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x63b712))
                                         ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x63b712),
                                          iVar12);
                      puVar13[6] = uVar10;
                    }
                    else {
                      uVar10 = (**(code **)(*(int *)*puVar4 + 4))((int *)*puVar4,puVar13[6],iVar12);
                      puVar13[6] = uVar10;
                    }
                  }
                }
                puVar13[9] = puVar13[9] + iVar17;
                src = (void *)puVar13[6];
                puVar13[10] = src;
                    /* Unresolved local var: int numToMove@[???] */
                if (0 < puVar13[9] - iVar17) {
                  if (0 >= iVar17) goto LAB_00528b0f;
                  _V_memmove((void *)((int)src + iVar17 * 0x18),src,(puVar13[9] - iVar17) * 0x18);
                }
                    /* Unresolved local var: int i@[???] */
                if (0 < iVar17) {
                  local_30 = 0;
                  iVar11 = 0;
                  do {
                    puVar15 = (undefined4 *)(iVar11 + puVar13[6]);
                    if (puVar15 != (undefined4 *)0x0) {
                      puVar14 = *(undefined4 **)(unaff_EBX + 0x63b7a2);
                      uVar7 = *puVar14;
                      puVar15[3] = uVar7;
                      uVar8 = puVar14[1];
                      puVar15[4] = uVar8;
                      uVar10 = puVar14[2];
                      puVar15[5] = uVar10;
                      *puVar15 = uVar7;
                      puVar15[1] = uVar8;
                      puVar15[2] = uVar10;
                    }
                    local_30 = local_30 + 1;
                    iVar11 = iVar11 + 0x18;
                  } while (local_30 < iVar17);
                  iVar12 = 0;
                  iVar11 = 0;
                  do {
                    /* Unresolved local var: int i@[???] */
                    puVar14 = (undefined4 *)
                              (iVar11 + *(int *)((int)&(pBVar3->m_teleportationHistory).m_Memory.
                                                       m_pMemory + local_20));
                    puVar15 = (undefined4 *)(iVar11 + puVar13[6]);
                    *puVar15 = *puVar14;
                    puVar15[1] = puVar14[1];
                    puVar15[2] = puVar14[2];
                    puVar15[3] = puVar14[3];
                    puVar15[4] = puVar14[4];
                    puVar15[5] = puVar14[5];
                    iVar12 = iVar12 + 1;
                    iVar11 = iVar11 + 0x18;
                  } while (iVar12 != iVar17);
                }
              }
LAB_00528b0f:
              local_44 = local_44 + 1;
              local_20 = local_20 + 0x2c;
            } while (local_44 != iVar16);
          }
        }
      }
      else {
        iVar11 = iVar17 * 0x18;
        pBVar9 = pBVar2 + iVar17 + 1;
LAB_005286fb:
                    /* Unresolved local var: BlobDataVector_t * dataA@[???]
                       Unresolved local var: BlobDataVector_t * dataB@[???]
                       Unresolved local var: float flDeltaTime@[???]
                       Unresolved local var: float flPercent@[???] */
        InterpolateBlobData((fVar20 - fVar18) / (fVar19 - fVar18),
                            (BlobDataVector_t *)
                            ((int)&(pBVar2->m_blobData).m_Memory.m_pMemory + iVar11),
                            &pBVar9->m_blobData,blobData);
      }
    }
  }
  return;
}


/* _GLOBAL__I_draw_paint_isosurface at 000767e0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_draw_paint_isosurface(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

