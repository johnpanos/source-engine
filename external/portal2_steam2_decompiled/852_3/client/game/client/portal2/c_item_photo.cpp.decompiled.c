/* DWARF-guided pseudocode for game/client/portal2/c_item_photo.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* _C_Photograph_CreateObject at 00521d30 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(&UNK_00673584 + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(&UNK_006739dc + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x673a94);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x673ad0);
  (this->super_IClientModelRenderable)._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x673aec);
  (**(code **)(unaff_EBX + 0x673640))(this,entnum,serialNum);
  return &(this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable;
}


/* C_Photograph::~C_Photograph at 00522210 */

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
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6730a7);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(&UNK_006734ff + unaff_EBX);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(&UNK_006735b7 + unaff_EBX);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(&UNK_006735f3 + unaff_EBX);
  (this->super_C_BaseAnimating).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x67360f);
  C_BaseAnimating::~C_BaseAnimating(&this->super_C_BaseAnimating,in_stack_ffffffe8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_Photograph::~C_Photograph at 00522290 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_Photograph(C_Photograph * this, int __in_chrg) */

void __thiscall C_Photograph::~C_Photograph(C_Photograph *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x673030);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(&UNK_00673488 + extraout_ECX);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(&UNK_00673540 + extraout_ECX);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(&UNK_0067357c + extraout_ECX);
  (this->super_C_BaseAnimating).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(&UNK_00673598 + extraout_ECX);
  C_BaseAnimating::~C_BaseAnimating(&this->super_C_BaseAnimating,__in_chrg);
  return;
}


/* ClientClassInit<DT_Photograph::ignored> at 00074290 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

int ClientClassInit<DT_Photograph::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&UNK_00005554)[(int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] ==
      '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc436b4);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  (&UNK_0000555c +
                  (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                  (char *)(unaff_EBX + 0x8ebabc),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x559].
                               m_SerialNumber + unaff_EBX),(char *)(unaff_EBX + 0x8f069c),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00abcb04 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00abcafc + unaff_EBX));
      RecvPropString((RecvProp_conflict1 *)
                     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x55d].m_pEntity +
                     unaff_EBX),&UNK_0091a9d8 + unaff_EBX,0xd84,0x104,0,
                     *(RecvVarProxyFn *)(&DAT_00abcd1c + unaff_EBX));
      ___cxa_guard_release(unaff_EBX + 0xc436b4);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0xfb8),
             (RecvProp_conflict1 *)
             ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x559].m_SerialNumber +
             unaff_EBX),2,&UNK_0091a9e8 + unaff_EBX);
  return 1;
}


/* __static_initialization_and_destruction_0 at 000743f0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

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
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x53b].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x53b].m_pNext + unaff_EBX)
       = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x53b].m_pNext + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x53b].m_pNext + unaff_EBX + 2) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x53b].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x53c].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x53c].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x53c].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x53c].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x53d].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x53d].m_SerialNumber + unaff_EBX + 3) =
       0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x53d].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x53d].m_pNext + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x53e].m_pEntity + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x53e].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x53e].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined **)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x53e].m_pNext + unaff_EBX + 3) =
       &UNK_00abda07 + unaff_EBX;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0xe4b) = unaff_EBX + 0x91a899;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0xe43) = unaff_EBX + 0x4ad92f;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0xe47) = 0;
  this = (RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0xe5b);
  *(RecvTable **)((int)s_pParticlePtrs + unaff_EBX + 0xe4f) = this;
  piVar1 = *(int **)(&DAT_00abc997 + unaff_EBX);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0xe53) = *piVar1;
  *piVar1 = unaff_EBX + 0xcda6a3;
  RecvTable::RecvTable(this);
  ___cxa_atexit(unaff_EBX + 0x8dce2f,0,*(undefined4 *)(&DAT_00abc91f + unaff_EBX));
  iVar2 = ClientClassInit<DT_Photograph::ignored>((ignored *)0x0);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0xe6f) = iVar2;
  pIVar3 = GetMaterialProxyDict();
  (*pIVar3->_vptr_IMaterialProxyDict[3])(pIVar3,unaff_EBX + 0x91a8a5,unaff_EBX + 0x4adacf);
  pIVar3 = GetMaterialProxyDict();
  (*pIVar3->_vptr_IMaterialProxyDict[3])(pIVar3,unaff_EBX + 0x91a8b3,unaff_EBX + 0x4adaff);
  return;
}


/* C_Photograph::YouForgotToImplementOrDeclareClientClass at 00521ac0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_Photograph * this) */

int __thiscall C_Photograph::YouForgotToImplementOrDeclareClientClass(C_Photograph *this)

{
  return 0;
}


/* C_Photograph::GetClientClass at 00521ae0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: ClientClass * GetClientClass(C_Photograph * this) */

ClientClass * __thiscall C_Photograph::GetClientClass(C_Photograph *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x82cfbc);
}


/* __tcf_0 at 00951230 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3fd884),in_stack_00000008);
  return;
}


/* CPhotoMaterialProxy::~CPhotoMaterialProxy at 005222f0 */

/* DWARF original prototype: void ~CPhotoMaterialProxy(CPhotoMaterialProxy * this, int __in_chrg) */

void __thiscall CPhotoMaterialProxy::~CPhotoMaterialProxy(CPhotoMaterialProxy *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CEntityMaterialProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(extraout_ECX + 0x672f90);
  (this->super_CEntityMaterialProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(*(int *)(extraout_ECX + 0x60ee58) + 8);
  operator_delete(this);
  return;
}


/* CPhotoMaterialProxy::~CPhotoMaterialProxy at 00522320 */

/* DWARF original prototype: void ~CPhotoMaterialProxy(CPhotoMaterialProxy * this, int __in_chrg) */

void __thiscall CPhotoMaterialProxy::~CPhotoMaterialProxy(CPhotoMaterialProxy *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CEntityMaterialProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(extraout_ECX + 0x672f60);
  (this->super_CEntityMaterialProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(*(int *)(extraout_ECX + 0x60ee28) + 8);
  return;
}


/* CPhotoMaterialProxy::CPhotoMaterialProxy at 00521af0 */

/* DWARF original prototype: void CPhotoMaterialProxy(CPhotoMaterialProxy * this) */

void __thiscall CPhotoMaterialProxy::CPhotoMaterialProxy(CPhotoMaterialProxy *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CEntityMaterialProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(extraout_ECX + 0x673790);
  this->m_BaseTextureVar = (IMaterialVar *)0x0;
  return;
}


/* CPhotoMaterialProxy::CPhotoMaterialProxy at 00521b10 */

/* DWARF original prototype: void CPhotoMaterialProxy(CPhotoMaterialProxy * this) */

void __thiscall CPhotoMaterialProxy::CPhotoMaterialProxy(CPhotoMaterialProxy *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CEntityMaterialProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(extraout_ECX + 0x673770);
  this->m_BaseTextureVar = (IMaterialVar *)0x0;
  return;
}


/* CCPhotoMaterialProxyFactory at 00521ed0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

IMaterialProxy * CCPhotoMaterialProxyFactory(void)

{
  IMaterialProxy *pIVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  pIVar1 = operator_new(8);
  pIVar1->_vptr_IMaterialProxy = (_func_int_varargs **)(unaff_EBX + 0x6733ac);
  pIVar1[1]._vptr_IMaterialProxy = (_func_int_varargs **)0x0;
  return pIVar1;
}


/* CPhotoMaterialProxy::Init at 00521b30 */

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
           (*pMaterial->_vptr_IMaterial[0xb])(pMaterial,unaff_EBX + 0x4503d0,local_d,0);
  this->m_BaseTextureVar = pIVar1;
  return (bool)local_d[0];
}


/* CPhotoMaterialProxy::OnBind at 00521de0 */

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
    iVar1 = ___dynamic_cast(pC_BaseEntity,*(undefined4 *)(unaff_EBX + 0x60ef38),unaff_EBX + 0x673a58
                            ,0);
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x60f0d4) + 0x144))
                ((int *)**(undefined4 **)(unaff_EBX + 0x60f0d4),iVar1 + 0xd84,unaff_EBX + 0x457f62,1
                 ,0);
                    /* WARNING: Could not recover jumptable at 0x00521e7a. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (*this->m_BaseTextureVar->_vptr_IMaterialVar[0xe])();
      return;
    }
  }
  return;
}


/* CPhotoMaterialProxy::GetMaterial at 00521b70 */

/* DWARF original prototype: IMaterial * GetMaterial(CPhotoMaterialProxy * this) */

IMaterial * __thiscall CPhotoMaterialProxy::GetMaterial(CPhotoMaterialProxy *this)

{
  IMaterial *pIVar1;
  
  if (this->m_BaseTextureVar != (IMaterialVar *)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00521b86. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    pIVar1 = (IMaterial *)(*this->m_BaseTextureVar->_vptr_IMaterialVar[0x18])();
    return pIVar1;
  }
  return (IMaterial *)0x0;
}


/* CPlacementPhotoMaterialProxy::~CPlacementPhotoMaterialProxy at 005221e0 */

/* DWARF original prototype: void ~CPlacementPhotoMaterialProxy(CPlacementPhotoMaterialProxy * this,
   int __in_chrg) */

void __thiscall
CPlacementPhotoMaterialProxy::~CPlacementPhotoMaterialProxy
          (CPlacementPhotoMaterialProxy *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CEntityMaterialProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(extraout_ECX + 0x673060);
  (this->super_CEntityMaterialProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(*(int *)(extraout_ECX + 0x60ef68) + 8);
  return;
}


/* CPlacementPhotoMaterialProxy::~CPlacementPhotoMaterialProxy at 00522350 */

/* DWARF original prototype: void ~CPlacementPhotoMaterialProxy(CPlacementPhotoMaterialProxy * this,
   int __in_chrg) */

void __thiscall
CPlacementPhotoMaterialProxy::~CPlacementPhotoMaterialProxy
          (CPlacementPhotoMaterialProxy *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CEntityMaterialProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(extraout_ECX + 0x672ef0);
  (this->super_CEntityMaterialProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(*(int *)(extraout_ECX + 0x60edf8) + 8);
  operator_delete(this);
  return;
}


/* CPlacementPhotoMaterialProxy::GetMaterial at 005221d0 */

/* DWARF original prototype: IMaterial * GetMaterial(CPlacementPhotoMaterialProxy * this) */

IMaterial * __thiscall CPlacementPhotoMaterialProxy::GetMaterial(CPlacementPhotoMaterialProxy *this)

{
  return this->m_pMaterial;
}


/* CPlacementPhotoMaterialProxy::CPlacementPhotoMaterialProxy at 00521ba0 */

/* DWARF original prototype: void CPlacementPhotoMaterialProxy(CPlacementPhotoMaterialProxy * this)
    */

void __thiscall
CPlacementPhotoMaterialProxy::CPlacementPhotoMaterialProxy(CPlacementPhotoMaterialProxy *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CEntityMaterialProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(extraout_ECX + 0x6736a0);
  this->m_pMaterial = (IMaterial *)0x0;
  this->m_pAlphaVar = (IMaterialVar *)0x0;
  this->m_flStartAlpha = 1.0;
  this->m_flTargetAlpha = 1.0;
  this->m_flFadeDuration = 1.0;
  this->m_flFadeStartTime = 0.0;
  this->m_bLastState = false;
  return;
}


/* CPlacementPhotoMaterialProxy::CPlacementPhotoMaterialProxy at 00521e90 */

/* DWARF original prototype: void CPlacementPhotoMaterialProxy(CPlacementPhotoMaterialProxy * this)
    */

void __thiscall
CPlacementPhotoMaterialProxy::CPlacementPhotoMaterialProxy(CPlacementPhotoMaterialProxy *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CEntityMaterialProxy).super_IMaterialProxy._vptr_IMaterialProxy =
       (_func_int_varargs **)(extraout_ECX + 0x6733b0);
  this->m_pMaterial = (IMaterial *)0x0;
  this->m_pAlphaVar = (IMaterialVar *)0x0;
  this->m_flStartAlpha = 1.0;
  this->m_flTargetAlpha = 1.0;
  this->m_flFadeDuration = 1.0;
  this->m_flFadeStartTime = 0.0;
  this->m_bLastState = false;
  return;
}


/* CCPlacementPhotoMaterialProxyFactory at 00521f00 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

IMaterialProxy * CCPlacementPhotoMaterialProxyFactory(void)

{
  IMaterialProxy *pIVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  pIVar1 = operator_new(0x20);
  pIVar1->_vptr_IMaterialProxy = (_func_int_varargs **)(unaff_EBX + 0x67333c);
  pIVar1[1]._vptr_IMaterialProxy = (_func_int_varargs **)0x0;
  pIVar1[2]._vptr_IMaterialProxy = (_func_int_varargs **)0x0;
  pIVar1[4]._vptr_IMaterialProxy = (_func_int_varargs **)0x3f800000;
  pIVar1[5]._vptr_IMaterialProxy = (_func_int_varargs **)0x3f800000;
  pIVar1[7]._vptr_IMaterialProxy = (_func_int_varargs **)0x3f800000;
  pIVar1[6]._vptr_IMaterialProxy = (_func_int_varargs **)0x0;
  *(undefined1 *)&pIVar1[3]._vptr_IMaterialProxy = 0;
  return pIVar1;
}


/* CPlacementPhotoMaterialProxy::Init at 00521be0 */

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
           (*pMaterial->_vptr_IMaterial[0xb])(pMaterial,unaff_EBX + 0x46d07e,local_1d,1);
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


/* CPlacementPhotoMaterialProxy::GetAlphaFade at 00521c70 */

/* DWARF original prototype: float GetAlphaFade(CPlacementPhotoMaterialProxy * this) */

float __thiscall CPlacementPhotoMaterialProxy::GetAlphaFade(CPlacementPhotoMaterialProxy *this)

{
  int extraout_ECX;
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  
  ___i686_get_pc_thunk_cx();
  fVar4 = this->m_flStartAlpha;
  fVar1 = this->m_flFadeStartTime;
  fVar2 = fVar1 + this->m_flFadeDuration;
  fVar3 = *(float *)(**(int **)(extraout_ECX + 0x60f0d5) + 0xc);
  if ((fVar1 != fVar2) || (NAN(fVar1) || NAN(fVar2))) {
    fVar3 = (fVar3 - fVar1) / (fVar2 - fVar1);
    fVar1 = *(float *)(extraout_ECX + 0x494bc9);
    if (fVar3 <= fVar1) {
      fVar1 = 0.0;
      if (0.0 <= fVar3) {
        fVar1 = fVar3;
      }
      fVar1 = fVar1 * fVar1 * *(float *)(extraout_ECX + 0x495035) * fVar1 +
              fVar1 * fVar1 * *(float *)(extraout_ECX + 0x494f95);
    }
    fVar4 = fVar4 + fVar1 * (this->m_flTargetAlpha - fVar4);
  }
  else if (fVar2 <= fVar3) {
    return this->m_flTargetAlpha;
  }
  return fVar4;
}


/* CPlacementPhotoMaterialProxy::OnBind at 00521f50 */

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
                ___dynamic_cast(pCVar4,*(undefined4 *)(unaff_EBX + 0x60ef8e),
                                *(undefined4 *)(unaff_EBX + 0x60f00a),0),
     this_00 != (C_Portal_Player *)0x0)) {
    bVar2 = this->m_bLastState;
    pCVar5 = C_Portal_Player::GetPortalPlayerLocalData(this_00);
    if (bVar2 != pCVar5->m_bPlacingPhoto) {
      fVar1 = this->m_flTargetAlpha;
      fVar9 = this->m_flStartAlpha;
      fVar8 = this->m_flFadeStartTime;
      fVar7 = fVar8 + this->m_flFadeDuration;
      piVar3 = *(int **)(unaff_EBX + 0x60edf2);
      fVar6 = *(float *)(*piVar3 + 0xc);
      if ((fVar8 != fVar7) || (NAN(fVar8) || NAN(fVar7))) {
        fVar8 = (fVar6 - fVar8) / (fVar7 - fVar8);
        if (fVar8 <= *(float *)(unaff_EBX + 0x4948e6)) {
          fVar6 = 0.0;
          if (0.0 <= fVar8) {
            fVar6 = fVar8;
          }
          fVar9 = (fVar1 - fVar9) *
                  (fVar6 * fVar6 * *(float *)(unaff_EBX + 0x494cb2) +
                  fVar6 * fVar6 * fVar6 * *(float *)(unaff_EBX + 0x494d52)) + fVar9;
        }
        else {
          fVar9 = (fVar1 - fVar9) * *(float *)(unaff_EBX + 0x4948e6) + fVar9;
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
                    /* WARNING: Could not recover jumptable at 0x0052207f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*this->m_pAlphaVar->_vptr_IMaterialVar[3])();
    return;
  }
  return;
}


/* _GLOBAL__I__ZN12C_Photograph17m_pClassRecvTableE at 00074550 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN12C_Photograph17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

