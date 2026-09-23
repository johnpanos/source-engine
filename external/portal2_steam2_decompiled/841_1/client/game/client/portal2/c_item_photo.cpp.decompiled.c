/* DWARF-guided pseudocode for game/client/portal2/c_item_photo.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* _C_Photograph_CreateObject at 0051c6e0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_Photograph_CreateObject(int entnum,int serialNum)

{
  C_BaseAnimating *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_Photograph * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xe88);
  C_BaseAnimating::C_BaseAnimating(this);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6ac854);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6accb0);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6acd68);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6acda4);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6acdc0);
  (**(code **)(unaff_EBX + 0x6ac910))(this,entnum,serialNum);
  return &(this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable;
}


/* C_Photograph::~C_Photograph at 0051cbc0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_Photograph(C_Photograph * this, int __in_chrg) */

void __thiscall C_Photograph::~C_Photograph(C_Photograph *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6ac377);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x6ac7d3);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x6ac88b);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x6ac8c7);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(unaff_EBX + 0x6ac8e3);
  C_BaseAnimating::~C_BaseAnimating(&this->super_C_BaseAnimating,in_stack_ffffffe8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_Photograph::~C_Photograph at 0051cc30 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_Photograph(C_Photograph * this, int __in_chrg) */

void __thiscall C_Photograph::~C_Photograph(C_Photograph *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x6ac310);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(extraout_ECX + 0x6ac76c);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(extraout_ECX + 0x6ac824);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(extraout_ECX + 0x6ac860);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(extraout_ECX + 0x6ac87c);
  C_BaseAnimating::~C_BaseAnimating(&this->super_C_BaseAnimating,__in_chrg);
  return;
}


/* ClientClassInit<DT_Photograph::ignored> at 00074160 */

int ClientClassInit<DT_Photograph::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)s_pRandomFloats + unaff_EBX + 0x1c4) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc5c8c4);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x1cc),
                  (char *)(unaff_EBX + 0x9165fc),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x208),
                        (char *)(unaff_EBX + 0x91b140),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00aefc80 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00aefc78 + unaff_EBX));
      RecvPropString((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x244),
                     (char *)(unaff_EBX + 0x945450),0xd84,0x104,0,
                     *(RecvVarProxyFn *)(&DAT_00aefe98 + unaff_EBX));
      ___cxa_guard_release(unaff_EBX + 0xc5c8c4);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x1218),
             (RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x208),2,
             (char *)(unaff_EBX + 0x945460));
  return 1;
}


/* __static_initialization_and_destruction_0 at 000742c0 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  RecvTable *this;
  int *piVar1;
  int iVar2;
  IMaterialProxyDict *pIVar3;
  int unaff_EBX;
  longlong lVar4;
  
  lVar4 = ___i686_get_pc_thunk_bx();
  if (lVar4 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2f) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x30) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x31) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x32) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x33) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x37) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3b) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3f) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x43) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x47) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x4b) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x4f) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x53) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x57) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x5b) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x5f) = 0x7f7fffff;
  *(undefined **)((int)s_pRandomFloats + unaff_EBX + 99) = &UNK_00af0bb7 + unaff_EBX;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x10ab) = unaff_EBX + 0x945311;
  *(undefined **)((int)s_pParticlePtrs + unaff_EBX + 0x10a3) = &UNK_004a840f + unaff_EBX;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0x10a7) = 0;
  this = (RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x10bb);
  *(RecvTable **)((int)s_pParticlePtrs + unaff_EBX + 0x10af) = this;
  piVar1 = *(int **)(&DAT_00aefb13 + unaff_EBX);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x10b3) = *piVar1;
  *piVar1 = unaff_EBX + 0xcf3c63;
  RecvTable::RecvTable(this);
  ___cxa_atexit(unaff_EBX + 0x90743f,0,*(undefined4 *)(&DAT_00aefa9b + unaff_EBX));
  iVar2 = ClientClassInit<DT_Photograph::ignored>((ignored *)0x0);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x10cf) = iVar2;
  pIVar3 = GetMaterialProxyDict();
  (*pIVar3->_vptr_IMaterialProxyDict[3])(pIVar3,unaff_EBX + 0x94531d,unaff_EBX + 0x4a85af);
  pIVar3 = GetMaterialProxyDict();
  (*pIVar3->_vptr_IMaterialProxyDict[3])(pIVar3,&UNK_0094532b + unaff_EBX,unaff_EBX + 0x4a85df);
  return;
}


/* C_Photograph::YouForgotToImplementOrDeclareClientClass at 0051c470 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_Photograph * this) */

int __thiscall C_Photograph::YouForgotToImplementOrDeclareClientClass(C_Photograph *this)

{
  return 0;
}


/* C_Photograph::GetClientClass at 0051c490 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: ClientClass * GetClientClass(C_Photograph * this) */

ClientClass * __thiscall C_Photograph::GetClientClass(C_Photograph *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x84ba9c);
}


/* __tcf_0 at 0097b710 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3ec834),in_stack_00000008);
  return;
}


/* CPhotoMaterialProxy::~CPhotoMaterialProxy at 0051cc90 */

/* DWARF original prototype: void ~CPhotoMaterialProxy(CPhotoMaterialProxy * this, int __in_chrg) */

void __thiscall CPhotoMaterialProxy::~CPhotoMaterialProxy(CPhotoMaterialProxy *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CEntityMaterialProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(extraout_ECX + 0x6ac270);
  (this->super_CEntityMaterialProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(*(int *)(extraout_ECX + 0x647508) + 8);
  operator_delete(this);
  return;
}


/* CPhotoMaterialProxy::~CPhotoMaterialProxy at 0051ccc0 */

/* DWARF original prototype: void ~CPhotoMaterialProxy(CPhotoMaterialProxy * this, int __in_chrg) */

void __thiscall CPhotoMaterialProxy::~CPhotoMaterialProxy(CPhotoMaterialProxy *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CEntityMaterialProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(extraout_ECX + 0x6ac240);
  (this->super_CEntityMaterialProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(*(int *)(extraout_ECX + 0x6474d8) + 8);
  return;
}


/* CPhotoMaterialProxy::CPhotoMaterialProxy at 0051c4a0 */

/* DWARF original prototype: void CPhotoMaterialProxy(CPhotoMaterialProxy * this) */

void __thiscall CPhotoMaterialProxy::CPhotoMaterialProxy(CPhotoMaterialProxy *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CEntityMaterialProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(extraout_ECX + 0x6aca60);
  this->m_BaseTextureVar = (IMaterialVar *)0x0;
  return;
}


/* CPhotoMaterialProxy::CPhotoMaterialProxy at 0051c4c0 */

/* DWARF original prototype: void CPhotoMaterialProxy(CPhotoMaterialProxy * this) */

void __thiscall CPhotoMaterialProxy::CPhotoMaterialProxy(CPhotoMaterialProxy *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CEntityMaterialProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(extraout_ECX + 0x6aca40);
  this->m_BaseTextureVar = (IMaterialVar *)0x0;
  return;
}


/* CCPhotoMaterialProxyFactory at 0051c880 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

IMaterialProxy * CCPhotoMaterialProxyFactory(void)

{
  IMaterialProxy *pIVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  pIVar1 = operator_new(8);
  pIVar1->_vptr_IMaterialProxy = (_func_int_varargs **)(unaff_EBX + 0x6ac67c);
  pIVar1[1]._vptr_IMaterialProxy = (_func_int_varargs **)0x0;
  return pIVar1;
}


/* CPhotoMaterialProxy::Init at 0051c4e0 */

/* DWARF original prototype: bool Init(CPhotoMaterialProxy * this, IMaterial * pMaterial, KeyValues
   * pKeyValues) */

bool __thiscall
CPhotoMaterialProxy::Init(CPhotoMaterialProxy *this,IMaterial *pMaterial,KeyValues *pKeyValues)

{
  IMaterialVar *pIVar1;
  int unaff_EBX;
  undefined1 local_d [5];
  
                    /* Unresolved local var: bool foundVar@[???] */
  ___i686_get_pc_thunk_bx();
  pIVar1 = (IMaterialVar *)
           (*pMaterial->_vptr_IMaterial[0xb])(pMaterial,unaff_EBX + 0x48040c,local_d,0);
  this->m_BaseTextureVar = pIVar1;
  return (bool)local_d[0];
}


/* CPhotoMaterialProxy::OnBind at 0051c790 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void OnBind(CPhotoMaterialProxy * this, C_BaseEntity * pC_BaseEntity)
    */

void __thiscall CPhotoMaterialProxy::OnBind(CPhotoMaterialProxy *this,C_BaseEntity *pC_BaseEntity)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: C_Photograph * pPhoto@[???]
                       Unresolved local var: ITexture * pTexture@[???] */
  ___i686_get_pc_thunk_bx();
  if (pC_BaseEntity != (C_BaseEntity *)0x0) {
    iVar1 = ___dynamic_cast(pC_BaseEntity,*(undefined4 *)(unaff_EBX + 0x6475d4),unaff_EBX + 0x6acd2c
                            ,0);
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(&DAT_00647768 + unaff_EBX) + 0x148))
                ((int *)**(undefined4 **)(&DAT_00647768 + unaff_EBX),iVar1 + 0xd84,
                 unaff_EBX + 0x487fa6,1,0);
                    /* WARNING: Could not recover jumptable at 0x0051c82a. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (*this->m_BaseTextureVar->_vptr_IMaterialVar[0xe])();
      return;
    }
  }
  return;
}


/* CPhotoMaterialProxy::GetMaterial at 0051c520 */

/* DWARF original prototype: IMaterial * GetMaterial(CPhotoMaterialProxy * this) */

IMaterial * __thiscall CPhotoMaterialProxy::GetMaterial(CPhotoMaterialProxy *this)

{
  IMaterial *pIVar1;
  
  if (this->m_BaseTextureVar != (IMaterialVar *)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0051c536. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    pIVar1 = (IMaterial *)(*this->m_BaseTextureVar->_vptr_IMaterialVar[0x18])();
    return pIVar1;
  }
  return (IMaterial *)0x0;
}


/* CPlacementPhotoMaterialProxy::~CPlacementPhotoMaterialProxy at 0051cb90 */

/* DWARF original prototype: void ~CPlacementPhotoMaterialProxy(CPlacementPhotoMaterialProxy * this,
   int __in_chrg) */

void __thiscall
CPlacementPhotoMaterialProxy::~CPlacementPhotoMaterialProxy
          (CPlacementPhotoMaterialProxy *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CEntityMaterialProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(extraout_ECX + 0x6ac330);
  (this->super_CEntityMaterialProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(*(int *)(extraout_ECX + 0x647608) + 8);
  return;
}


/* CPlacementPhotoMaterialProxy::~CPlacementPhotoMaterialProxy at 0051ccf0 */

/* DWARF original prototype: void ~CPlacementPhotoMaterialProxy(CPlacementPhotoMaterialProxy * this,
   int __in_chrg) */

void __thiscall
CPlacementPhotoMaterialProxy::~CPlacementPhotoMaterialProxy
          (CPlacementPhotoMaterialProxy *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CEntityMaterialProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(extraout_ECX + 0x6ac1d0);
  (this->super_CEntityMaterialProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(*(int *)(extraout_ECX + 0x6474a8) + 8);
  operator_delete(this);
  return;
}


/* CPlacementPhotoMaterialProxy::GetMaterial at 0051cb80 */

/* DWARF original prototype: IMaterial * GetMaterial(CPlacementPhotoMaterialProxy * this) */

IMaterial * __thiscall CPlacementPhotoMaterialProxy::GetMaterial(CPlacementPhotoMaterialProxy *this)

{
  return this->m_pMaterial;
}


/* CPlacementPhotoMaterialProxy::CPlacementPhotoMaterialProxy at 0051c550 */

/* DWARF original prototype: void CPlacementPhotoMaterialProxy(CPlacementPhotoMaterialProxy * this)
    */

void __thiscall
CPlacementPhotoMaterialProxy::CPlacementPhotoMaterialProxy(CPlacementPhotoMaterialProxy *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CEntityMaterialProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(extraout_ECX + 0x6ac970);
  this->m_pMaterial = (IMaterial *)0x0;
  this->m_pAlphaVar = (IMaterialVar *)0x0;
  this->m_flStartAlpha = 1.0;
  this->m_flTargetAlpha = 1.0;
  this->m_flFadeDuration = 1.0;
  this->m_flFadeStartTime = 0.0;
  this->m_bLastState = false;
  return;
}


/* CPlacementPhotoMaterialProxy::CPlacementPhotoMaterialProxy at 0051c840 */

/* DWARF original prototype: void CPlacementPhotoMaterialProxy(CPlacementPhotoMaterialProxy * this)
    */

void __thiscall
CPlacementPhotoMaterialProxy::CPlacementPhotoMaterialProxy(CPlacementPhotoMaterialProxy *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CEntityMaterialProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(extraout_ECX + 0x6ac680);
  this->m_pMaterial = (IMaterial *)0x0;
  this->m_pAlphaVar = (IMaterialVar *)0x0;
  this->m_flStartAlpha = 1.0;
  this->m_flTargetAlpha = 1.0;
  this->m_flFadeDuration = 1.0;
  this->m_flFadeStartTime = 0.0;
  this->m_bLastState = false;
  return;
}


/* CCPlacementPhotoMaterialProxyFactory at 0051c8b0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

IMaterialProxy * CCPlacementPhotoMaterialProxyFactory(void)

{
  IMaterialProxy *pIVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  pIVar1 = operator_new(0x20);
  pIVar1->_vptr_IMaterialProxy = (_func_int_varargs **)(unaff_EBX + 0x6ac60c);
  pIVar1[1]._vptr_IMaterialProxy = (_func_int_varargs **)0x0;
  pIVar1[2]._vptr_IMaterialProxy = (_func_int_varargs **)0x0;
  pIVar1[4]._vptr_IMaterialProxy = (_func_int_varargs **)0x3f800000;
  pIVar1[5]._vptr_IMaterialProxy = (_func_int_varargs **)0x3f800000;
  pIVar1[7]._vptr_IMaterialProxy = (_func_int_varargs **)0x3f800000;
  pIVar1[6]._vptr_IMaterialProxy = (_func_int_varargs **)0x0;
  *(undefined1 *)&pIVar1[3]._vptr_IMaterialProxy = 0;
  return pIVar1;
}


/* CPlacementPhotoMaterialProxy::Init at 0051c590 */

/* DWARF original prototype: bool Init(CPlacementPhotoMaterialProxy * this, IMaterial * pMaterial,
   KeyValues * pKeyValues) */

bool __thiscall
CPlacementPhotoMaterialProxy::Init
          (CPlacementPhotoMaterialProxy *this,IMaterial *pMaterial,KeyValues *pKeyValues)

{
  IMaterialVar *pIVar1;
  int unaff_EBX;
  char local_1d [13];
  
                    /* Unresolved local var: bool found@[???] */
  ___i686_get_pc_thunk_bx();
  this->m_pMaterial = pMaterial;
  pIVar1 = (IMaterialVar *)
           (*pMaterial->_vptr_IMaterial[0xb])(pMaterial,unaff_EBX + 0x49d016,local_1d,1);
  this->m_pAlphaVar = pIVar1;
  if (local_1d[0] != '\0') {
    (*pMaterial->_vptr_IMaterial[0x1d])(pMaterial,0x40000000,1);
    this->m_bLastState = false;
  }
  else {
    this->m_pAlphaVar = (IMaterialVar *)0x0;
  }
  return local_1d[0] != '\0';
}


/* CPlacementPhotoMaterialProxy::GetAlphaFade at 0051c620 */

/* DWARF original prototype: float GetAlphaFade(CPlacementPhotoMaterialProxy * this) */

float __thiscall CPlacementPhotoMaterialProxy::GetAlphaFade(CPlacementPhotoMaterialProxy *this)

{
  int extraout_ECX;
  float fVar1;
  float fVar2;
  float fVar3;
  
  ___i686_get_pc_thunk_cx();
  fVar3 = this->m_flStartAlpha;
  fVar1 = this->m_flFadeStartTime;
  fVar2 = fVar1 + this->m_flFadeDuration;
  if ((fVar1 != fVar2) || (NAN(fVar1) || NAN(fVar2))) {
    fVar2 = (*(float *)(**(int **)(CAvatarImage::Paint + extraout_ECX + 1) + 0xc) - fVar1) /
            (fVar2 - fVar1);
    fVar1 = *(float *)(extraout_ECX + 0x4c5b59);
    if (fVar2 <= fVar1) {
      fVar1 = 0.0;
      if (0.0 <= fVar2) {
        fVar1 = fVar2;
      }
      fVar1 = fVar1 * fVar1 * *(float *)(extraout_ECX + 0x4c5fc1) * fVar1 +
              fVar1 * fVar1 * *(float *)(extraout_ECX + 0x4c5f25);
    }
    fVar3 = fVar3 + fVar1 * (this->m_flTargetAlpha - fVar3);
  }
  else if (fVar2 <= *(float *)(**(int **)(CAvatarImage::Paint + extraout_ECX + 1) + 0xc)) {
    return this->m_flTargetAlpha;
  }
  return fVar3;
}


/* CPlacementPhotoMaterialProxy::OnBind at 0051c900 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void OnBind(CPlacementPhotoMaterialProxy * this, C_BaseEntity *
   pC_BaseEntity) */

void __thiscall
CPlacementPhotoMaterialProxy::OnBind(CPlacementPhotoMaterialProxy *this,C_BaseEntity *pC_BaseEntity)

{
  float fVar1;
  bool bVar2;
  int *piVar3;
  C_BasePlayer *pCVar4;
  C_Portal_Player *this_00;
  C_PortalPlayerLocalData *pCVar5;
  int unaff_EBX;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  
                    /* Unresolved local var: C_Portal_Player * pPlayer@[???] */
  ___i686_get_pc_thunk_bx();
  pCVar4 = C_BasePlayer::GetLocalPlayer(-1);
  if ((pCVar4 != (C_BasePlayer *)0x0) &&
     (this_00 = (C_Portal_Player *)
                ___dynamic_cast(pCVar4,*(undefined4 *)(unaff_EBX + 0x647626),
                                *(undefined4 *)(unaff_EBX + 0x6476a2),0),
     this_00 != (C_Portal_Player *)0x0)) {
    bVar2 = this->m_bLastState;
    pCVar5 = C_Portal_Player::GetPortalPlayerLocalData(this_00);
    if (bVar2 != pCVar5->m_bPlacingPhoto) {
      fVar1 = this->m_flTargetAlpha;
      fVar9 = this->m_flStartAlpha;
      fVar8 = this->m_flFadeStartTime;
      fVar7 = fVar8 + this->m_flFadeDuration;
      piVar3 = *(int **)(unaff_EBX + 0x64748e);
      fVar6 = *(float *)(*piVar3 + 0xc);
      if ((fVar8 != fVar7) || (NAN(fVar8) || NAN(fVar7))) {
        fVar8 = (fVar6 - fVar8) / (fVar7 - fVar8);
        if (fVar8 <= *(float *)(unaff_EBX + 0x4c5876)) {
          fVar6 = 0.0;
          if (0.0 <= fVar8) {
            fVar6 = fVar8;
          }
          fVar9 = (fVar1 - fVar9) *
                  (fVar6 * fVar6 * *(float *)(unaff_EBX + 0x4c5c42) +
                  fVar6 * fVar6 * fVar6 * *(float *)(unaff_EBX + 0x4c5cde)) + fVar9;
        }
        else {
          fVar9 = (fVar1 - fVar9) * *(float *)(unaff_EBX + 0x4c5876) + fVar9;
        }
      }
      else if (fVar7 <= fVar6) {
        fVar9 = fVar1;
      }
      this->m_flStartAlpha = fVar9;
      this->m_flFadeStartTime = *(float *)(*piVar3 + 0xc);
      this->m_flFadeDuration = 0.4;
      if (this->m_bLastState == false) {
        this->m_flTargetAlpha = 0.0;
      }
      else {
        this->m_flTargetAlpha = 1.0;
      }
      pCVar5 = C_Portal_Player::GetPortalPlayerLocalData(this_00);
      this->m_bLastState = pCVar5->m_bPlacingPhoto;
    }
                    /* WARNING: Could not recover jumptable at 0x0051ca2f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*this->m_pAlphaVar->_vptr_IMaterialVar[3])();
    return;
  }
  return;
}


/* _GLOBAL__I__ZN12C_Photograph17m_pClassRecvTableE at 00074420 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN12C_Photograph17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

