/* DWARF-guided pseudocode for game/client/portal2/c_env_portal_laser.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* C_PortalLaser::~C_PortalLaser at 0051b040 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_PortalLaser(C_PortalLaser * this, int __in_chrg) */

void __thiscall C_PortalLaser::~C_PortalLaser(C_PortalLaser *this,int __in_chrg)

{
  CNewParticleEffect *pCVar1;
  CUtlReference<CNewParticleEffect> *pCVar2;
  CUtlReference<CNewParticleEffect> *pCVar3;
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6ac9bb);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x6ace17);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x6acecf);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x6acf0b);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(unaff_EBX + 0x6acf27);
  pCVar1 = (this->m_pSparkEffect).m_pObject;
  if (pCVar1 != (CNewParticleEffect *)0x0) {
    pCVar2 = (this->m_pSparkEffect).m_pPrev;
    if (pCVar2 == (CUtlReference<CNewParticleEffect> *)0x0) {
      if ((pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
          &this->m_pSparkEffect) {
        pCVar2 = (this->m_pSparkEffect).m_pNext;
        (pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
        super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar2;
        if (pCVar2 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar2->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
        }
      }
    }
    else {
      pCVar2->m_pNext = (this->m_pSparkEffect).m_pNext;
      pCVar3 = (this->m_pSparkEffect).m_pNext;
      if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
        pCVar3->m_pPrev = pCVar2;
      }
    }
    (this->m_pSparkEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
    (this->m_pSparkEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
    (this->m_pSparkEffect).m_pObject = (CNewParticleEffect *)0x0;
  }
  C_BaseAnimating::~C_BaseAnimating(&this->super_C_BaseAnimating,in_stack_ffffffe8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_PortalLaser::~C_PortalLaser at 0051b120 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_PortalLaser(C_PortalLaser * this, int __in_chrg) */

void __thiscall C_PortalLaser::~C_PortalLaser(C_PortalLaser *this,int __in_chrg)

{
  CNewParticleEffect *pCVar1;
  CUtlReference<CNewParticleEffect> *pCVar2;
  CUtlReference<CNewParticleEffect> *pCVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6ac8df);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x6acd3b);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x6acdf3);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x6ace2f);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(unaff_EBX + 0x6ace4b);
  pCVar1 = (this->m_pSparkEffect).m_pObject;
  if (pCVar1 != (CNewParticleEffect *)0x0) {
    pCVar2 = (this->m_pSparkEffect).m_pPrev;
    if (pCVar2 == (CUtlReference<CNewParticleEffect> *)0x0) {
      if ((pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
          &this->m_pSparkEffect) {
        pCVar2 = (this->m_pSparkEffect).m_pNext;
        (pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
        super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar2;
        if (pCVar2 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar2->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
        }
      }
    }
    else {
      pCVar2->m_pNext = (this->m_pSparkEffect).m_pNext;
      pCVar3 = (this->m_pSparkEffect).m_pNext;
      if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
        pCVar3->m_pPrev = pCVar2;
      }
    }
    (this->m_pSparkEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
    (this->m_pSparkEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
    (this->m_pSparkEffect).m_pObject = (CNewParticleEffect *)0x0;
  }
  C_BaseAnimating::~C_BaseAnimating(&this->super_C_BaseAnimating,__in_chrg);
  return;
}


/* ClientClassInit<DT_PortalLaser::ignored> at 00073220 */

int ClientClassInit<DT_PortalLaser::ignored>(ignored *param_1)

{
  RecvVarProxyFn varProxy;
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)s_pRandomFloats + unaff_EBX + 0x8c4) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc5cfc4);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x8cc),
                  (char *)(unaff_EBX + 0x91753c),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x908),
                        (char *)(unaff_EBX + 0x91c080),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00af0bc0 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00af0bb8 + unaff_EBX));
      varProxy = *(RecvVarProxyFn *)(&DAT_00af0c5c + unaff_EBX);
      RecvPropVector((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x944),
                     (char *)(unaff_EBX + 0x9460b6),0xd90,0xc,0,varProxy);
      RecvPropVector((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x980),
                     (char *)(unaff_EBX + 0x9460c7),0xda8,0xc,0,varProxy);
      RecvPropBool((RecvProp *)((int)s_pRandomFloats + unaff_EBX + 0x9bc),
                   (char *)(unaff_EBX + 0x9460de),0xdb4,1);
      ___cxa_guard_release(unaff_EBX + 0xc5cfc4);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x2078),
             (RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x908),4,
             (char *)(unaff_EBX + 0x9460ed));
  return 1;
}


/* __static_initialization_and_destruction_0 at 00073400 */

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
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x6af) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x6b0) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x6b1) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x6b2) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x6b3) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x6b7) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x6bb) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x6bf) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x6c3) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x6c7) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x6cb) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x6cf) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x6d3) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x6d7) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x6db) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x6df) = 0x7f7fffff;
  *(undefined **)((int)s_pRandomFloats + unaff_EBX + 0x6e3) = &UNK_00af1a77 + unaff_EBX;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x1e8b) = unaff_EBX + 0x945f1f;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x1e83) = unaff_EBX + 0x4a7b0f;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0x1e87) = 0;
  this = (RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x1e9b);
  *(RecvTable **)((int)s_pParticlePtrs + unaff_EBX + 0x1e8f) = this;
  piVar1 = *(int **)(&DAT_00af09d3 + unaff_EBX);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x1e93) = *piVar1;
  *piVar1 = unaff_EBX + 0xcf4a43;
  RecvTable::RecvTable(this);
  ___cxa_atexit(unaff_EBX + 0x90816f,0,*(undefined4 *)(&DAT_00af095b + unaff_EBX));
  iVar2 = ClientClassInit<DT_PortalLaser::ignored>((ignored *)0x0);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x1eaf) = iVar2;
  return;
}


/* C_PortalLaser::YouForgotToImplementOrDeclareClientClass at 0051a770 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_PortalLaser * this) */

int __thiscall C_PortalLaser::YouForgotToImplementOrDeclareClientClass(C_PortalLaser *this)

{
  return 0;
}


/* C_PortalLaser::GetClientClass at 0051a790 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: ClientClass * GetClientClass(C_PortalLaser * this) */

ClientClass * __thiscall C_PortalLaser::GetClientClass(C_PortalLaser *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x84d6bc);
}


/* _C_PortalLaser_CreateObject at 0051af20 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_PortalLaser_CreateObject(int entnum,int serialNum)

{
  int *piVar1;
  C_BaseAnimating *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_PortalLaser * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xdb8);
  C_BaseAnimating::C_BaseAnimating(this);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6acada);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6acf36);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(&UNK_006acfee + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(&UNK_006ad02a + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(&UNK_006ad046 + unaff_EBX);
  this[1].super_C_BaseEntity.super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)0x0;
  this[1].super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)0x0;
  this[1].super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)0x0;
  piVar1 = *(int **)(unaff_EBX + 0x648ed2);
  this[1].super_C_BaseEntity.super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)*piVar1;
  this[1].super_C_BaseEntity.super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)piVar1[1];
  this[1].super_C_BaseEntity.m_iClassname = (string_t)piVar1[2];
  this[1].super_C_BaseEntity.m_hScriptInstance = (HSCRIPT)*piVar1;
  this[1].super_C_BaseEntity.m_iszScriptId = (string_t)piVar1[1];
  this[1].super_C_BaseEntity.m_VarMap.m_Entries.m_Memory.m_pMemory = (VarMapEntry_t *)piVar1[2];
  this[1].super_C_BaseEntity.m_VarMap.m_Entries.m_Memory.m_nAllocationCount = *piVar1;
  this[1].super_C_BaseEntity.m_VarMap.m_Entries.m_Memory.m_nGrowSize = piVar1[1];
  this[1].super_C_BaseEntity.m_VarMap.m_Entries.m_Size = piVar1[2];
  *(undefined1 *)&this[1].super_C_BaseEntity.m_VarMap.m_Entries.m_pElements = 0;
  *(undefined1 *)((int)&this[1].super_C_BaseEntity.m_VarMap.m_Entries.m_pElements + 1) = 0;
  (**(code **)(&UNK_006acb96 + unaff_EBX))(this,entnum,serialNum);
  return &(this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable;
}


/* __tcf_0 at 0097b580 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3ec8e4),in_stack_00000008);
  return;
}


/* C_PortalLaser::C_PortalLaser at 0051a8a0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void C_PortalLaser(C_PortalLaser * this) */

void __thiscall C_PortalLaser::C_PortalLaser(C_PortalLaser *this)

{
  vec_t *pvVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseAnimating::C_BaseAnimating(&this->super_C_BaseAnimating);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6ad15b);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x6ad5b7);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x6ad66f);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x6ad6ab);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(unaff_EBX + 0x6ad6c7);
  (this->m_pSparkEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
  (this->m_pSparkEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
  (this->m_pSparkEffect).m_pObject = (CNewParticleEffect *)0x0;
  pvVar1 = *(vec_t **)(unaff_EBX + 0x649553);
  (this->m_vecLaserEndPos).x = *pvVar1;
  (this->m_vecLaserEndPos).y = pvVar1[1];
  (this->m_vecLaserEndPos).z = pvVar1[2];
  (this->m_vecOldLaserEndPos).x = *pvVar1;
  (this->m_vecOldLaserEndPos).y = pvVar1[1];
  (this->m_vecOldLaserEndPos).z = pvVar1[2];
  (this->m_vecLaserImpactNormal).x = *pvVar1;
  (this->m_vecLaserImpactNormal).y = pvVar1[1];
  (this->m_vecLaserImpactNormal).z = pvVar1[2];
  this->m_bShouldSpark = false;
  this->m_bOldShouldSpark = false;
  return;
}


/* C_PortalLaser::C_PortalLaser at 0051aa90 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void C_PortalLaser(C_PortalLaser * this, C_PortalLaser * this) */

void __thiscall C_PortalLaser::C_PortalLaser(C_PortalLaser *this,C_PortalLaser *this_1)

{
  vec_t *pvVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseAnimating::C_BaseAnimating(&this->super_C_BaseAnimating);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6acf6b);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(&UNK_006ad3c7 + unaff_EBX);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(&UNK_006ad47f + unaff_EBX);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(&UNK_006ad4bb + unaff_EBX);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(&UNK_006ad4d7 + unaff_EBX);
  (this->m_pSparkEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
  (this->m_pSparkEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
  (this->m_pSparkEffect).m_pObject = (CNewParticleEffect *)0x0;
  pvVar1 = *(vec_t **)(vgui::Label::GetKBMap + unaff_EBX + 3);
  (this->m_vecLaserEndPos).x = *pvVar1;
  (this->m_vecLaserEndPos).y = pvVar1[1];
  (this->m_vecLaserEndPos).z = pvVar1[2];
  (this->m_vecOldLaserEndPos).x = *pvVar1;
  (this->m_vecOldLaserEndPos).y = pvVar1[1];
  (this->m_vecOldLaserEndPos).z = pvVar1[2];
  (this->m_vecLaserImpactNormal).x = *pvVar1;
  (this->m_vecLaserImpactNormal).y = pvVar1[1];
  (this->m_vecLaserImpactNormal).z = pvVar1[2];
  this->m_bShouldSpark = false;
  this->m_bOldShouldSpark = false;
  return;
}


/* C_PortalLaser::Precache at 0051a860 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void Precache(C_PortalLaser * this) */

void __thiscall C_PortalLaser::Precache(C_PortalLaser *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  PrecacheParticleSystem(*(char **)(unaff_EBX + 0x711473));
  C_BaseEntity::Precache((C_BaseEntity *)this);
  return;
}


/* C_PortalLaser::Spawn at 0051a830 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void Spawn(C_PortalLaser * this) */

void __thiscall C_PortalLaser::Spawn(C_PortalLaser *this)

{
  (*(this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x26])(this);
  C_BaseEntity::Spawn((C_BaseEntity *)this);
  return;
}


/* C_PortalLaser::UpdateOnRemove at 0051a980 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void UpdateOnRemove(C_PortalLaser * this) */

void __thiscall C_PortalLaser::UpdateOnRemove(C_PortalLaser *this)

{
  CNewParticleEffect *pCVar1;
  CUtlReference<CNewParticleEffect> *pCVar2;
  CUtlReference<CNewParticleEffect> *pCVar3;
  
  pCVar1 = (this->m_pSparkEffect).m_pObject;
  if (pCVar1 != (CNewParticleEffect *)0x0) {
    CParticleProperty::StopEmission
              (&(this->super_C_BaseAnimating).super_C_BaseEntity.m_Particles.super_CParticleProperty
               ,pCVar1,false,false,false,false);
    pCVar1 = (this->m_pSparkEffect).m_pObject;
    if (pCVar1 != (CNewParticleEffect *)0x0) {
      pCVar2 = (this->m_pSparkEffect).m_pPrev;
      if (pCVar2 == (CUtlReference<CNewParticleEffect> *)0x0) {
        if ((pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
            &this->m_pSparkEffect) {
          pCVar2 = (this->m_pSparkEffect).m_pNext;
          (pCVar1->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
          super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar2;
          if (pCVar2 != (CUtlReference<CNewParticleEffect> *)0x0) {
            pCVar2->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
          }
        }
      }
      else {
        pCVar2->m_pNext = (this->m_pSparkEffect).m_pNext;
        pCVar3 = (this->m_pSparkEffect).m_pNext;
        if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar3->m_pPrev = pCVar2;
        }
      }
      (this->m_pSparkEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
      (this->m_pSparkEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
      (this->m_pSparkEffect).m_pObject = (CNewParticleEffect *)0x0;
    }
  }
  C_BaseEntity::UpdateOnRemove((C_BaseEntity *)this);
  return;
}


/* C_PortalLaser::OnPreDataChanged at 0051aa50 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void OnPreDataChanged(C_PortalLaser * this, DataUpdateType_t
   updateType) */

void __thiscall C_PortalLaser::OnPreDataChanged(C_PortalLaser *this,DataUpdateType_t updateType)

{
  (this->m_vecOldLaserEndPos).x = (this->m_vecLaserEndPos).x;
  (this->m_vecOldLaserEndPos).y = (this->m_vecLaserEndPos).y;
  (this->m_vecOldLaserEndPos).z = (this->m_vecLaserEndPos).z;
  this->m_bOldShouldSpark = this->m_bShouldSpark;
  C_BaseAnimating::OnPreDataChanged(&this->super_C_BaseAnimating,updateType);
  return;
}


/* C_PortalLaser::OnDataChanged at 0051ad80 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void OnDataChanged(C_PortalLaser * this, DataUpdateType_t updateType)
    */

void __thiscall C_PortalLaser::OnDataChanged(C_PortalLaser *this,DataUpdateType_t updateType)

{
  vec_t *pvVar1;
  float fVar2;
  CNewParticleEffect *pCVar3;
  Vector local_34;
  Vector local_28 [2];
  
  if (updateType == DATA_UPDATE_CREATED) {
    CreateSparkEffect(this);
  }
  fVar2 = (this->m_vecOldLaserEndPos).x;
  if ((((fVar2 != (this->m_vecLaserEndPos).x) || (NAN(fVar2) || NAN((this->m_vecLaserEndPos).x))) ||
      (fVar2 = (this->m_vecOldLaserEndPos).y, pvVar1 = &(this->m_vecLaserEndPos).y, fVar2 != *pvVar1
      )) || ((NAN(fVar2) || NAN(*pvVar1) ||
             (fVar2 = (this->m_vecOldLaserEndPos).z, pvVar1 = &(this->m_vecLaserEndPos).z,
             NAN(fVar2) || NAN(*pvVar1))))) {
    pCVar3 = (this->m_pSparkEffect).m_pObject;
  }
  else {
    if (fVar2 == *pvVar1) goto LAB_0051adde;
    pCVar3 = (this->m_pSparkEffect).m_pObject;
  }
  if (pCVar3 != (CNewParticleEffect *)0x0) {
                    /* Unresolved local var: Vector vecRight@[???]
                       Unresolved local var: Vector vecUp@[???] */
    CNewParticleEffect::SetControlPoint(pCVar3,0,&this->m_vecLaserEndPos);
    VectorVectors(&this->m_vecLaserImpactNormal,local_28,&local_34);
    CNewParticleEffect::SetControlPointOrientation
              ((this->m_pSparkEffect).m_pObject,0,&this->m_vecLaserImpactNormal,local_28,&local_34);
  }
LAB_0051adde:
  if (this->m_bShouldSpark != this->m_bOldShouldSpark) {
    if (this->m_bShouldSpark == false) {
      pCVar3 = (this->m_pSparkEffect).m_pObject;
      if (pCVar3 != (CNewParticleEffect *)0x0) {
        CNewParticleEffect::StopEmission(pCVar3,false,false,false,false);
      }
    }
    else {
      CreateSparkEffect(this);
      pCVar3 = (this->m_pSparkEffect).m_pObject;
      if (pCVar3 != (CNewParticleEffect *)0x0) {
        CParticleCollection::StartEmission(&pCVar3->super_CParticleCollection,false);
      }
    }
  }
  C_BaseAnimating::OnDataChanged(&this->super_C_BaseAnimating,updateType);
  return;
}


/* C_PortalLaser::CreateSparkEffect at 0051ab70 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void CreateSparkEffect(C_PortalLaser * this) */

void __thiscall C_PortalLaser::CreateSparkEffect(C_PortalLaser *this)

{
  Vector *pVVar1;
  CUtlReference<CNewParticleEffect> *pCVar2;
  CUtlReference<CNewParticleEffect> *pCVar3;
  CNewParticleEffect *pCVar4;
  CNewParticleEffect *pCVar5;
  int unaff_EBX;
  Vector local_40;
  Vector local_34;
  vec_t local_28;
  vec_t local_24;
  vec_t local_20;
  
  ___i686_get_pc_thunk_bx();
  pCVar5 = (this->m_pSparkEffect).m_pObject;
  if (pCVar5 == (CNewParticleEffect *)0x0) {
    pVVar1 = *(Vector **)(&DAT_0064927c + unaff_EBX);
    local_28 = pVVar1->x;
    local_24 = pVVar1->y;
    local_20 = pVVar1->z;
    pCVar4 = CParticleProperty::Create
                       (&(this->super_C_BaseAnimating).super_C_BaseEntity.m_Particles.
                         super_CParticleProperty,*(char **)(unaff_EBX + 0x711160),
                        PATTACH_CUSTOMORIGIN,-1,*pVVar1,(matrix3x4_t *)0x0);
    pCVar5 = (this->m_pSparkEffect).m_pObject;
    if (pCVar4 != pCVar5) {
      if (pCVar5 != (CNewParticleEffect *)0x0) {
        pCVar2 = (this->m_pSparkEffect).m_pPrev;
        if (pCVar2 == (CUtlReference<CNewParticleEffect> *)0x0) {
          if ((pCVar5->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
              super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead ==
              &this->m_pSparkEffect) {
            pCVar2 = (this->m_pSparkEffect).m_pNext;
            (pCVar5->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
            super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = pCVar2;
            if (pCVar2 != (CUtlReference<CNewParticleEffect> *)0x0) {
              pCVar2->m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
            }
          }
        }
        else {
          pCVar2->m_pNext = (this->m_pSparkEffect).m_pNext;
          pCVar3 = (this->m_pSparkEffect).m_pNext;
          if (pCVar3 != (CUtlReference<CNewParticleEffect> *)0x0) {
            pCVar3->m_pPrev = pCVar2;
          }
        }
        (this->m_pSparkEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
        (this->m_pSparkEffect).m_pNext = (CUtlReference<CNewParticleEffect> *)0x0;
        (this->m_pSparkEffect).m_pObject = (CNewParticleEffect *)0x0;
      }
      (this->m_pSparkEffect).m_pObject = pCVar4;
      pCVar5 = (CNewParticleEffect *)0x0;
      if (pCVar4 != (CNewParticleEffect *)0x0) {
        (this->m_pSparkEffect).m_pNext =
             (pCVar4->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
             super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead;
        pCVar2 = (pCVar4->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>
                 .super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead;
        if (pCVar2 != (CUtlReference<CNewParticleEffect> *)0x0) {
          pCVar2->m_pPrev = &this->m_pSparkEffect;
        }
        (this->m_pSparkEffect).m_pPrev = (CUtlReference<CNewParticleEffect> *)0x0;
        (pCVar4->m_References).super_CUtlIntrusiveDList<CUtlReference<CNewParticleEffect>_>.
        super_CUtlIntrusiveList<CUtlReference<CNewParticleEffect>_>.m_pHead = &this->m_pSparkEffect;
        pCVar5 = (this->m_pSparkEffect).m_pObject;
      }
    }
    CNewParticleEffect::StopEmission(pCVar5,false,false,false,false);
    pCVar5 = (this->m_pSparkEffect).m_pObject;
    if (pCVar5 == (CNewParticleEffect *)0x0) {
      return;
    }
  }
                    /* Unresolved local var: Vector vecRight@[???]
                       Unresolved local var: Vector vecUp@[???] */
  CNewParticleEffect::SetControlPoint(pCVar5,0,&this->m_vecLaserEndPos);
  VectorVectors(&this->m_vecLaserImpactNormal,&local_34,&local_40);
  CNewParticleEffect::SetControlPointOrientation
            ((this->m_pSparkEffect).m_pObject,0,&this->m_vecLaserImpactNormal,&local_34,&local_40);
  return;
}


/* C_PortalLaser::UpdateSparkEffect at 0051a7a0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void UpdateSparkEffect(C_PortalLaser * this) */

void __thiscall C_PortalLaser::UpdateSparkEffect(C_PortalLaser *this)

{
  CNewParticleEffect *this_00;
  Vector local_34;
  Vector local_28 [2];
  
  this_00 = (this->m_pSparkEffect).m_pObject;
  if (this_00 != (CNewParticleEffect *)0x0) {
                    /* Unresolved local var: Vector vecRight@[???]
                       Unresolved local var: Vector vecUp@[???] */
    CNewParticleEffect::SetControlPoint(this_00,0,&this->m_vecLaserEndPos);
    VectorVectors(&this->m_vecLaserImpactNormal,local_28,&local_34);
    CNewParticleEffect::SetControlPointOrientation
              ((this->m_pSparkEffect).m_pObject,0,&this->m_vecLaserImpactNormal,local_28,&local_34);
  }
  return;
}


/* _GLOBAL__I_LASER_SPARK_EFFECT_NAME at 00073520 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_LASER_SPARK_EFFECT_NAME(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

