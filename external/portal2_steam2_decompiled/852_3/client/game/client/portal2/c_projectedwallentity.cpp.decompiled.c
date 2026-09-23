/* DWARF-guided pseudocode for game/client/portal2/c_projectedwallentity.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 00077da0 */

/* WARNING: Struct "ToggleButton": ignoring multiple overlapping fields */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  RecvTable *pRVar1;
  undefined4 uVar2;
  int *piVar3;
  int iVar4;
  IClassMap *pIVar5;
  int unaff_EBX;
  longlong lVar6;
  ConVar *in_stack_ffffffc4;
  ConVar *in_stack_ffffffc8;
  char *in_stack_ffffffcc;
  char *in_stack_ffffffd0;
  int in_stack_ffffffd4;
  char *in_stack_ffffffd8;
  
  lVar6 = ___i686_get_pc_thunk_bx();
  if (lVar6 == 0xffff00000001) {
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x314].m_pPrev + unaff_EBX) = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x314].m_pPrev + unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x314].m_pPrev + unaff_EBX + 2) = 0;
    *(undefined1 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x314].m_pPrev + unaff_EBX + 3) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x314].m_pNext + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x315].m_pEntity + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x315].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x315].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x315].m_pNext + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x316].m_pEntity + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x316].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x316].m_pPrev + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x316].m_pNext + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x317].m_pEntity + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x317].m_SerialNumber + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x317].m_pPrev + unaff_EBX) =
         0x7f7fffff;
    *(undefined **)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x317].m_pNext + unaff_EBX) =
         &UNK_00aba054 + unaff_EBX;
    ConVar::ConVar((ConVar *)((int)&mat_softwareskin.m_Value.m_fValue + unaff_EBX),
                   (char *)(unaff_EBX + 0x918484),
                   (char *)(vgui::ToggleButton::~ToggleButton + unaff_EBX + 2),0,in_stack_ffffffc4,
                   (char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
    uVar2 = *(undefined4 *)(&DAT_00ab8f6c + unaff_EBX);
    ___cxa_atexit(unaff_EBX + 0x8d9c8c,0,uVar2);
    ConVar::ConVar((ConVar *)((int)&r_nohw.m_Value.m_fValue + unaff_EBX),
                   (char *)(unaff_EBX + 0x9184b0),(char *)(unaff_EBX + 0x9184a7),0,in_stack_ffffffc4
                   ,(char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
    ___cxa_atexit(unaff_EBX + 0x8d9c6c,0,uVar2);
    ConVar::ConVar((ConVar *)((int)&r_nosw.m_Value.m_fValue + unaff_EBX),
                   (char *)(unaff_EBX + 0x9184e0),&UNK_008e9348 + unaff_EBX,0,in_stack_ffffffc4,
                   (char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
    ___cxa_atexit(unaff_EBX + 0x8d9c4c,0,uVar2);
    ConVar::ConVar((ConVar *)((int)&r_teeth.m_Value.m_fValue + unaff_EBX),
                   (char *)(unaff_EBX + 0x918504),(char *)(unaff_EBX + 0x8e0858),0,in_stack_ffffffc4
                   ,(char *)in_stack_ffffffc8,in_stack_ffffffcc,(int)in_stack_ffffffd0);
    ___cxa_atexit(unaff_EBX + 0x8d9c2c,0,uVar2);
    *(undefined4 *)((int)&r_drawentities.m_Value.m_StringLength + unaff_EBX) = 0;
    *(int *)((int)&r_drawentities.m_Value.m_fValue + unaff_EBX) = unaff_EBX + 0x918524;
    piVar3 = *(int **)(&DAT_00ab9160 + unaff_EBX);
    *(int *)((int)&r_drawentities.m_Value.m_nValue + unaff_EBX) = *piVar3;
    *piVar3 = unaff_EBX + 0xcd7fa4;
    *(undefined **)((int)&r_drawentities.m_Value.m_pszString + unaff_EBX) =
         &UNK_00b20bdc + unaff_EBX;
    *(int *)(&r_drawentities.m_bHasMax + unaff_EBX) = unaff_EBX + 0x918546;
    *(int *)(&r_drawentities.m_bHasMin + unaff_EBX) = unaff_EBX + 0x4b8ffc;
    *(undefined4 *)((int)&r_drawentities.m_fMinVal + unaff_EBX) = 0;
    pRVar1 = (RecvTable *)
             ((int)&r_drawentities.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX);
    *(RecvTable **)((int)&r_drawentities.m_fMaxVal + unaff_EBX) = pRVar1;
    piVar3 = *(int **)(&DAT_00ab8fe4 + unaff_EBX);
    *(int *)((int)&r_drawentities.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) = *piVar3;
    *piVar3 = unaff_EBX + 0xcd7fb4;
    RecvTable::RecvTable(pRVar1);
    ___cxa_atexit(unaff_EBX + 0x8d9bec,0,uVar2);
    iVar4 = ClientClassInit<DT_ProjectedWallEntity::ignored>((ignored *)0x0);
    *(int *)((int)&r_flex.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) = iVar4;
    *(undefined4 *)((int)&PTR_MarkShadowDirty_00b7eb94 + unaff_EBX) = 5;
    *(int *)((int)&PTR_IsShadowDirty_00b7eb90 + unaff_EBX) =
         (int)&PTR_StartTouch_00b7ebec + unaff_EBX;
    *(int *)((int)&r_flex.super_ConCommandBase.m_pNext + unaff_EBX) =
         (int)&PTR_IsShadowDirty_00b7eb90 + unaff_EBX;
    pIVar5 = GetClassMap();
    (*pIVar5->_vptr_IClassMap[2])
              (pIVar5,unaff_EBX + 0x8ece54,(char *)(unaff_EBX + 0x91846e),0xb38,unaff_EBX + 0x4b8fbc
              );
    ConVar::ConVar((ConVar *)((int)&r_flex.super_ConCommandBase.m_pszName + unaff_EBX),
                   (char *)(unaff_EBX + 0x9185ce),(char *)(unaff_EBX + 0x8e0858),0,
                   &UNK_0091855c + unaff_EBX,in_stack_ffffffc8,in_stack_ffffffcc,in_stack_ffffffd0,
                   in_stack_ffffffd4,in_stack_ffffffd8);
    ___cxa_atexit(&UNK_008d9c0c + unaff_EBX,0,uVar2);
    CIFM_EntityKeyValuesHandler_RecreateEntities::CIFM_EntityKeyValuesHandler_RecreateEntities
              ((CIFM_EntityKeyValuesHandler_RecreateEntities *)
               ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x318].m_pPrev + unaff_EBX),
               (char *)(unaff_EBX + 0x91846e));
    *(undefined **)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x318].m_pPrev + unaff_EBX) =
         &UNK_00b211b4 + unaff_EBX;
    ___cxa_atexit(unaff_EBX + 0x8d9cac,0,uVar2);
    C_BaseTempEntity::C_BaseTempEntity
              ((C_BaseTempEntity *)
               ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x31a].m_pPrev + unaff_EBX));
    *(undefined **)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x31a].m_pPrev + unaff_EBX) =
         &UNK_00b205f4 + unaff_EBX;
    *(undefined **)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x31a].m_pNext + unaff_EBX) =
         &UNK_00b20678 + unaff_EBX;
    *(undefined4 *)
     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x31b].m_pPrev + unaff_EBX) =
         0xffffffff;
    ___cxa_atexit(unaff_EBX + 0x8d9d8c,0,uVar2);
    *(int *)((int)&r_eyes.super_ConCommandBase.m_pszName + unaff_EBX) = unaff_EBX + 0x9185ea;
    *(undefined4 *)((int)&r_eyes.super_ConCommandBase.m_pNext + unaff_EBX) = 0;
    *(int *)(&r_eyes.super_ConCommandBase.m_bRegistered + unaff_EBX) = unaff_EBX + 0x4b633c;
    pRVar1 = (RecvTable *)((int)&r_eyes.m_pParent + unaff_EBX);
    *(RecvTable **)((int)&r_eyes.super_ConCommandBase.m_pszHelpString + unaff_EBX) = pRVar1;
    piVar3 = *(int **)(&DAT_00ab8fe4 + unaff_EBX);
    *(int *)((int)&r_eyes.super_ConCommandBase.m_nFlags + unaff_EBX) = *piVar3;
    *piVar3 = unaff_EBX + 0xcd8044;
    RecvTable::RecvTable(pRVar1);
    ___cxa_atexit(unaff_EBX + 0x8d9bcc,0,uVar2);
    iVar4 = ClientClassInit<DT_TEWallPaintedEvent::ignored>((ignored *)0x0);
    *(int *)((int)&r_eyes.m_Value.m_nValue + unaff_EBX) = iVar4;
  }
  return;
}


/* __tcf_0 at 00951a40 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fe198),in_stack_00000008);
  return;
}


/* __tcf_1 at 00951a20 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fe218),in_stack_00000008);
  return;
}


/* __tcf_2 at 00951a00 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fe298),in_stack_00000008);
  return;
}


/* __tcf_3 at 009519e0 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fe318),in_stack_00000008);
  return;
}


/* PrecacheEffectProjectedWallEntityPrecache::CResourcePrecacher::Cache at 0052de10 */

/* WARNING: Enum "PrecacheSystem_t": Some values do not have unique names */
/* DWARF original prototype: void Cache(CResourcePrecacher * this, IPrecacheHandler *
   pPrecacheHandler, bool bPrecache, ResourceList_t hResourceList, bool bIgnoreConditionals) */

void __thiscall
PrecacheEffectProjectedWallEntityPrecache::CResourcePrecacher::Cache
          (CResourcePrecacher *this,IPrecacheHandler *pPrecacheHandler,bool bPrecache,
          ResourceList_t hResourceList,bool bIgnoreConditionals)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (**pPrecacheHandler->_vptr_IPrecacheHandler)
            (pPrecacheHandler,1,unaff_EBX + 0x4621f5,(uint)bPrecache,hResourceList,0);
  (**pPrecacheHandler->_vptr_IPrecacheHandler)
            (pPrecacheHandler,1,&UNK_00462290 + unaff_EBX,(uint)bPrecache,hResourceList,0);
  return;
}


/* ClientClassInit<DT_ProjectedWallEntity::ignored> at 00077a00 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

int ClientClassInit<DT_ProjectedWallEntity::ignored>(ignored *param_1)

{
  RecvVarProxyFn p_Var1;
  int iVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&UNK_0000367e)
      [(int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX] == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xc417e2);
    if (iVar2 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x368].m_pNext +
                  unaff_EBX + 2),(char *)(unaff_EBX + 0x8e8352),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x36c].m_pPrev +
                        unaff_EBX + 2),(char *)(unaff_EBX + 0x8ecf32),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00ab9b6a + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00ab9392 + unaff_EBX));
      p_Var1 = *(RecvVarProxyFn *)(&DAT_00ab943a + unaff_EBX);
      RecvPropVector((RecvProp_conflict1 *)
                     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x370].m_SerialNumber +
                     unaff_EBX + 2),(char *)(unaff_EBX + 0x907b56),0x114,0xc,0,p_Var1);
      RecvPropVector((RecvProp_conflict1 *)
                     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x374].m_pEntity +
                     unaff_EBX + 2),(char *)(unaff_EBX + 0x904192),0x108,0xc,0,p_Var1);
      RecvPropVector((RecvProp_conflict1 *)
                     (&UNK_0000377e +
                     (int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX
                     ),(char *)(unaff_EBX + 0x918760),0xacc,0xc,0,p_Var1);
      RecvPropVector((RecvProp_conflict1 *)
                     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x37b].m_pPrev +
                     unaff_EBX + 2),(char *)(unaff_EBX + 0x918777),0xad8,0xc,0,p_Var1);
      RecvPropEHandle((RecvProp *)
                      ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x37f].m_SerialNumber
                      + unaff_EBX + 2),(char *)(unaff_EBX + 0x91878e),0xb08,4,
                      *(RecvVarProxyFn *)(&DAT_00ab938e + unaff_EBX));
      p_Var1 = *(RecvVarProxyFn *)(&DAT_00ab93d2 + unaff_EBX);
      RecvPropFloat((RecvProp_conflict1 *)
                    (&UNK_00003836 +
                    (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                    (char *)(unaff_EBX + 0x91879d),0xaf8,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x386].m_pNext +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x909722),0xafc,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x38a].m_pPrev +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x9187aa),0xb00,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x38e].m_SerialNumber +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x9187b5),0xb18,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x392].m_pEntity +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x9187c7),0xb30,4,0,p_Var1);
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x395].m_pNext +
                  unaff_EBX + 2),&UNK_009187de + unaff_EBX,0xb14,4,0,(RecvVarProxyFn)0x0);
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x399].m_pPrev +
                   unaff_EBX + 2),&UNK_009187ed + unaff_EBX,0xb04,1);
      ___cxa_guard_release(unaff_EBX + 0xc417e2);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)&vec4_invalid.x + unaff_EBX + 2),
             (RecvProp_conflict1 *)
             ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x36c].m_pPrev + unaff_EBX + 2)
             ,0xd,&UNK_009187fd + unaff_EBX);
  return 1;
}


/* C_ProjectedWallEntity::YouForgotToImplementOrDeclareClientClass at 0052dea0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_ProjectedWallEntity *
   this) */

int __thiscall
C_ProjectedWallEntity::YouForgotToImplementOrDeclareClientClass(C_ProjectedWallEntity *this)

{
  return 0;
}


/* C_ProjectedWallEntity::GetClientClass at 0052dec0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: ClientClass * GetClientClass(C_ProjectedWallEntity * this) */

ClientClass * __thiscall C_ProjectedWallEntity::GetClientClass(C_ProjectedWallEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x821ea0);
}


/* _C_ProjectedWallEntity_CreateObject at 00530db0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

IClientNetworkable * _C_ProjectedWallEntity_CreateObject(int entnum,int serialNum)

{
  C_ProjectedWallEntity *this;
  IClientNetworkable *pIVar1;
  
                    /* Unresolved local var: C_ProjectedWallEntity * pRet@[???] */
  this = C_BaseEntity::operator_new(0xb38);
  C_ProjectedWallEntity::C_ProjectedWallEntity(this);
  pIVar1 = (IClientNetworkable *)0x0;
  if (this != (C_ProjectedWallEntity *)0x0) {
    (*(this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
      super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
      _vptr_IHandleEntity[0x2f])(this,entnum,serialNum);
    pIVar1 = &(this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
              super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable;
  }
  return pIVar1;
}


/* __tcf_4 at 009519a0 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3fe3d8),in_stack_00000008);
  return;
}


/* C_ProjectedWallEntity::GetPredDescMap at 0052ded0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: datamap_t * GetPredDescMap(C_ProjectedWallEntity * this) */

datamap_t * __thiscall C_ProjectedWallEntity::GetPredDescMap(C_ProjectedWallEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x6c8a6c);
}


/* PredMapInit<C_ProjectedWallEntity> at 0052dee0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

datamap_t * PredMapInit<C_ProjectedWallEntity>(C_ProjectedWallEntity *param_1)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *(undefined4 *)(extraout_ECX + 0x6c8a5d) = 5;
  *(int *)(extraout_ECX + 0x6c8a59) = extraout_ECX + 0x6c8ab5;
  return (datamap_t *)(extraout_ECX + 0x6c8a59);
}


/* CC_ProjectedWallEntityFactory at 00530d70 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

C_BaseEntity * CC_ProjectedWallEntityFactory(void)

{
  C_ProjectedWallEntity *this;
  
  this = C_BaseEntity::operator_new(0xb38);
  C_ProjectedWallEntity::C_ProjectedWallEntity(this);
  return (C_BaseEntity *)this;
}


/* C_ProjectedWallEntity::C_ProjectedWallEntity at 00530c50 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void C_ProjectedWallEntity(C_ProjectedWallEntity * this) */

void __thiscall C_ProjectedWallEntity::C_ProjectedWallEntity(C_ProjectedWallEntity *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseProjectedEntity::C_BaseProjectedEntity((C_BaseProjectedEntity *)this);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x66780b);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x667bef);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x667ca7);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x667ce3);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_IPaintableEntity.
  _vptr_IPaintableEntity = (_func_int_varargs **)(&UNK_00667cff + unaff_EBX);
  (this->super_CSignifierTarget).super_ISignifierTarget._vptr_ISignifierTarget =
       (_func_int_varargs **)(unaff_EBX + 0x667d1b);
  (this->m_WallCollideables).m_Memory.m_pMemory = (WallCollideableAtTime_t *)0x0;
  (this->m_WallCollideables).m_Memory.m_nAllocationCount = 0;
  (this->m_WallCollideables).m_Memory.m_nGrowSize = 0;
  (this->m_WallCollideables).m_Size = 0;
  (this->m_WallCollideables).m_pElements = (WallCollideableAtTime_t *)0x0;
  (this->m_hColorPortal).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_PaintPowers).m_Memory.m_pMemory = (PaintPowerType *)0x0;
  (this->m_PaintPowers).m_Memory.m_nAllocationCount = 0;
  (this->m_PaintPowers).m_Memory.m_nGrowSize = 0;
  (this->m_PaintPowers).m_Size = 0;
  (this->m_PaintPowers).m_pElements = (PaintPowerType *)0x0;
  this->m_pBodyMaterial = (IMaterial *)0x0;
  this->m_pSideRailMaterial = (IMaterial *)0x0;
  this->m_flCurDisplayLength = 0.0;
  this->m_nNumSegments = 0;
  this->m_flSegmentLength = 0.0;
  this->m_flParticleUpdateTime = 0.0;
  this->m_flPrevParticleUpdateTime = 0.0;
  return;
}


/* C_ProjectedWallEntity::C_ProjectedWallEntity at 00530d60 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void C_ProjectedWallEntity(C_ProjectedWallEntity * this,
   C_ProjectedWallEntity * this) */

void __thiscall
C_ProjectedWallEntity::C_ProjectedWallEntity
          (C_ProjectedWallEntity *this,C_ProjectedWallEntity *this_1)

{
  C_ProjectedWallEntity(this);
  return;
}


/* CProjectedWallEntity_NonConformantDataHandler::CreateInstance at 00535ef0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void * CreateInstance(CProjectedWallEntity_NonConformantDataHandler *
   this) */

void * __thiscall
CProjectedWallEntity_NonConformantDataHandler::CreateInstance
          (CProjectedWallEntity_NonConformantDataHandler *this)

{
  C_ProjectedWallEntity *this_00;
  
  this_00 = C_BaseEntity::operator_new(0xb38);
  C_ProjectedWallEntity::C_ProjectedWallEntity(this_00);
  return this_00;
}


/* C_ProjectedWallEntity::UpdateOnRemove at 0052edf0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void UpdateOnRemove(C_ProjectedWallEntity * this) */

void __thiscall C_ProjectedWallEntity::UpdateOnRemove(C_ProjectedWallEntity *this)

{
  int unaff_EBX;
  int iVar1;
  int i;
  
  ___i686_get_pc_thunk_bx();
  StopParticleEffects((C_BaseEntity *)this);
  if ((this->m_WallCollideables).m_Size != 0) {
    i = 0;
    iVar1 = 0;
    do {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x60206a) + 0x44))
                ((int *)**(undefined4 **)(unaff_EBX + 0x60206a),
                 *(undefined4 *)
                  ((int)((this->m_WallCollideables).m_Memory.m_pMemory)->flTime + iVar1 + 8));
      i = i + 1;
      iVar1 = iVar1 + 0x4c;
    } while ((this->m_WallCollideables).m_Size != i);
  }
  (this->m_WallCollideables).m_Size = 0;
  C_BaseEntity::UpdateOnRemove((C_BaseEntity *)this);
  return;
}


/* C_ProjectedWallEntity::GetCollideType at 0052df10 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: CollideType_t GetCollideType(C_ProjectedWallEntity * this) */

CollideType_t __thiscall C_ProjectedWallEntity::GetCollideType(C_ProjectedWallEntity *this)

{
  return ENTITY_SHOULD_COLLIDE;
}


/* C_ProjectedWallEntity::DrawModel at 00530e20 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: int DrawModel(C_ProjectedWallEntity * this, int flags,
   RenderableInstance_t * instance) */

int __thiscall
C_ProjectedWallEntity::DrawModel
          (C_ProjectedWallEntity *this,int flags,RenderableInstance_t *instance)

{
  float *pfVar1;
  float *pfVar2;
  uint *puVar3;
  char cVar4;
  int iVar5;
  QAngle *pQVar6;
  float *pfVar7;
  int *piVar8;
  Color *pCVar9;
  int iVar10;
  int unaff_EBX;
  bool bVar11;
  float fVar12;
  float fVar13;
  float fVar14;
  float fVar15;
  float fVar16;
  float fVar17;
  float fVar18;
  float fVar19;
  float fVar20;
  float fVar21;
  Vector *forward;
  float local_5ac;
  uint local_5a8;
  float local_59c;
  float local_598;
  float local_594;
  float local_584;
  Vector local_574 [13];
  int local_4d8;
  int local_4d4;
  int local_4d0;
  int local_4c4;
  int *local_4c0;
  uint local_4bc;
  char local_4b8;
  int local_4b4;
  int local_4a8;
  int *local_4a4;
  int local_4a0;
  int local_49c;
  int local_498;
  int local_494;
  undefined4 local_490;
  undefined4 local_48c;
  undefined4 local_488;
  undefined1 local_484;
  int local_480 [3];
  int local_474;
  int local_470;
  int local_468;
  int local_464;
  int local_460;
  int local_45c;
  int local_458;
  int local_454;
  int local_450;
  int local_44c;
  undefined4 local_434;
  uint local_430;
  float *local_42c;
  int local_420;
  uint *local_41c;
  undefined4 *local_414;
  int local_410;
  int local_40c;
  int local_408;
  int local_404;
  int local_400;
  int local_3fc;
  int local_3f8;
  int *local_3dc;
  undefined1 local_3d8;
  undefined4 local_3d4;
  int local_3d0;
  int local_3cc;
  float *local_3c8;
  int local_3c4;
  uint *local_3c0;
  undefined4 *local_3bc;
  int local_3b8;
  int local_3b4;
  int local_3b0;
  int local_3ac;
  int local_3a8;
  int local_3a4;
  int local_3a0;
  undefined4 local_39c;
  int local_398;
  int local_394;
  undefined1 local_38c [156];
  int local_2f0;
  int local_2ec;
  int local_2e8;
  int local_2dc;
  int *local_2d8;
  uint local_2d4;
  char local_2d0;
  int local_2cc;
  int local_2c0;
  int *local_2bc;
  int local_2b8;
  int local_2b4;
  int local_2b0;
  int local_2ac;
  undefined4 local_2a8;
  undefined4 local_2a4;
  undefined4 local_2a0;
  undefined1 local_29c;
  int local_298 [3];
  int local_28c;
  int local_288;
  int local_280;
  int local_27c;
  int local_278;
  int local_274;
  int local_270;
  int local_26c;
  int local_268;
  int local_264;
  undefined4 local_24c;
  uint local_248;
  float *local_244;
  int local_238;
  uint *local_234;
  undefined4 *local_22c;
  int local_228;
  int local_224;
  int local_220;
  int local_21c;
  int local_218;
  int local_214;
  int local_210;
  int *local_1f4;
  undefined1 local_1f0;
  undefined4 local_1ec;
  int local_1e8;
  int local_1e4;
  float *local_1e0;
  int local_1dc;
  uint *local_1d8;
  undefined4 *local_1d4;
  int local_1d0;
  int local_1cc;
  int local_1c8;
  int local_1c4;
  int local_1c0;
  int local_1bc;
  int local_1b8;
  undefined4 local_1b4;
  int local_1b0;
  int local_1ac;
  Vector local_1a4;
  undefined4 local_198;
  Vector local_194;
  undefined4 local_188;
  Vector local_184;
  float local_178;
  float local_174;
  float local_170;
  Vector local_16c;
  float local_160;
  float local_15c;
  float local_158;
  float local_154;
  float local_150;
  float local_14c;
  float local_148;
  float local_144;
  float local_140;
  float local_13c;
  float local_138;
  float local_134;
  float local_130;
  float local_12c;
  float local_128;
  Vector local_124;
  float local_118;
  float local_114;
  float local_110;
  float local_10c;
  float local_108;
  float local_104;
  float local_100;
  float local_fc;
  float local_f8;
  float local_f4;
  float local_f0;
  float local_ec;
  float local_e8;
  float local_e4;
  float local_e0;
  float local_dc;
  float local_d8;
  float local_d4;
  float local_d0;
  float local_cc;
  float local_c8;
  float local_c4;
  float local_c0;
  float local_bc;
  float local_b8;
  float local_b4;
  float local_b0;
  float local_ac;
  float local_a8;
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
  int *local_64;
  float local_60;
  float local_5c;
  float local_58;
  float local_54;
  float local_50;
  Vector local_4c;
  float local_40;
  float local_3c;
  float local_38;
  float local_34;
  float local_30;
  float local_2c;
  float local_28;
  undefined4 local_24;
  int *local_20;
  
                    /* Unresolved local var: bool bDebugging@[???] */
  ___i686_get_pc_thunk_bx();
  iVar5 = (*(this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
            super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
            _vptr_IHandleEntity[0xf6])(this);
  iVar10 = 0;
  if ((char)iVar5 != '\0') {
    iVar5 = *(int *)(*(int *)(unaff_EBX + 0x81eeeb) + 0x30);
    if ((*(int *)(*(int *)(unaff_EBX + 0x81ee8b) + 0x30) == 0) ||
       ((this->m_PaintPowers).m_Size != this->m_nNumSegments)) {
      pQVar6 = C_BaseEntity::GetNetworkAngles((C_BaseEntity *)this);
      AngleVectors(pQVar6,&local_194,&local_16c,&local_124);
      fVar12 = this->m_flCurDisplayLength;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
      pfVar7 = (float *)(&DAT_00485a9f + unaff_EBX);
      local_130 = fVar12 * local_194.x * *pfVar7;
      local_12c = fVar12 * local_194.y * *pfVar7;
      local_128 = fVar12 * local_194.z * *pfVar7;
      pfVar7 = (float *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                          super_C_BaseProjectedEntity.super_C_BaseEntity.super_IClientEntity.
                          super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0xe4])(this);
                    /* Unresolved local var: Vector res@[???] */
      fVar12 = *pfVar7 + local_130;
      fVar17 = pfVar7[1] + local_12c;
      fVar20 = pfVar7[2] + local_128;
      local_13c = fVar12;
      local_138 = fVar17;
      local_134 = fVar20;
      local_4c.x = (vec_t)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x600097) + 400))
                                    ((int *)**(undefined4 **)(unaff_EBX + 0x600097));
      if ((int *)local_4c.x != (int *)0x0) {
        (**(code **)(*(int *)local_4c.x + 8))(local_4c.x);
      }
      piVar8 = (int *)(**(code **)(*(int *)local_4c.x + 0xe4))
                                (local_4c.x,1,0,0,this->m_pBodyMaterial);
      local_2d8 = (int *)0x0;
      local_2d0 = 0;
      local_2bc = (int *)0x0;
      local_2b8 = 0;
      local_2b4 = 0;
      local_2ac = 0;
      local_2a8 = 0;
      local_2a4 = 0xffffffff;
      local_2a0 = 0;
      local_1f4 = (int *)0x0;
      local_1b0 = -1;
      local_1ac = 0;
      local_1e8 = 0;
      local_1e4 = 0;
      local_1ec = 0;
      local_1b4 = 0;
      local_24c = 0xffffffff;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5fff3b) + 0x88))
                ((int *)**(undefined4 **)(unaff_EBX + 0x5fff3b));
      pfVar7 = (float *)(&DAT_00485a9f + unaff_EBX);
      fVar13 = *pfVar7 * this->m_flCurDisplayLength;
      local_194.x = local_194.x * fVar13;
      local_194.y = local_194.y * fVar13;
      local_194.z = fVar13 * local_194.z;
      fVar13 = *pfVar7 * this->m_flWidth;
      local_16c.x = local_16c.x * fVar13;
      local_16c.y = local_16c.y * fVar13;
      local_16c.z = fVar13 * local_16c.z;
      fVar13 = *pfVar7 * this->m_flHeight;
      local_124.x = local_124.x * fVar13;
      local_124.y = local_124.y * fVar13;
      local_124.z = fVar13 * local_124.z;
      fVar13 = *(float *)(unaff_EBX + 0x488643) * this->m_flCurDisplayLength;
      local_2d0 = '\x01';
      local_2d4 = 7;
      local_2d8 = piVar8;
      (**(code **)(*piVar8 + 0x28))(piVar8,2);
      (**(code **)(*local_2d8 + 0x44))(local_2d8,0x14,0x1e,local_38c);
      local_2bc = (int *)0x0;
      if (piVar8 != (int *)0x0) {
        local_2bc = piVar8 + 1;
      }
      local_2b4 = 0;
      local_2b8 = 0x1e;
      local_29c = 0;
      local_2b0 = local_2f0;
      local_2cc = local_2e8;
      local_2c0 = local_2dc;
      local_2ac = 0;
      local_1f4 = piVar8;
      _memcpy(local_298,local_38c,0xa4);
      local_1ec = 0x14;
      local_248 = ~-(uint)(local_248 == 0) & 2;
      local_1e8 = 0;
      local_1f0 = 0;
      if (local_1b0 == -1) {
        local_1b4 = 0;
        local_1b0 = local_2ec;
        local_1ac = local_2f0;
      }
      local_2ac = 0;
      local_1e4 = 0;
      local_1e0 = local_244;
      local_1dc = local_238;
                    /* Unresolved local var: int i@[???] */
      local_1d4 = local_22c;
      local_1d0 = local_228;
      local_1cc = local_224;
      local_1c8 = local_220;
      local_1c4 = local_21c;
      local_1c0 = local_218;
      local_1bc = local_214;
      local_1b8 = local_210;
      local_1d8 = local_234;
                    /* Unresolved local var: Vector res@[???] */
      local_148 = fVar12 + local_124.x;
      local_144 = fVar17 + local_124.y;
      local_140 = fVar20 + local_124.z;
                    /* Unresolved local var: Vector vert@[???]
                       Unresolved local var: float[4] pColors@[???] */
      local_184.x = (local_148 + local_16c.x) - local_194.x;
      local_184.y = (local_144 + local_16c.y) - local_194.y;
      local_184.z = (local_140 + local_16c.z) - local_194.z;
      (*this->m_pMyColor->_vptr_IMaterialVar[0xd])(this->m_pMyColor,&local_1a4,3);
      local_198 = 0x3dcccccd;
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
      pfVar7 = (float *)(unaff_EBX + 0x485dcf);
      pfVar1 = (float *)(unaff_EBX + 0x4878c3);
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
      *local_1d8 = ((uint)(*pfVar7 * local_1a4.y + *pfVar1) & 0xff) << 8 |
                   ((uint)(*pfVar7 * local_1a4.z + *pfVar1) & 0xff) << 0x10 |
                   (uint)(*pfVar7 * local_1a4.x + *pfVar1) & 0xff |
                   (int)(*pfVar7 * 0.1 + *pfVar1) << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
      *local_1d4 = 0;
      local_1d4[1] = fVar13;
                    /* Unresolved local var: float * pDst@[???] */
      *local_1e0 = local_184.x;
      local_1e0[1] = local_184.y;
      local_1e0[2] = local_184.z;
      iVar5 = local_1e4 + 1;
      if (local_1e4 + 1 <= local_1e8) {
        iVar5 = local_1e8;
      }
      local_1e0 = (float *)((int)local_1e0 + local_298[0]);
      local_1dc = local_1dc + local_28c * 4;
      local_1d4 = (undefined4 *)((int)local_1d4 + local_280);
      local_1d0 = local_1d0 + local_27c * 4;
      local_1cc = local_1cc + local_278 * 4;
      local_1c8 = local_1c8 + local_274 * 4;
      local_1c4 = local_1c4 + local_270 * 4;
      local_1c0 = local_1c0 + local_26c * 4;
      local_1bc = local_1bc + local_268 * 4;
      local_1b8 = local_1b8 + local_264 * 4;
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
      pfVar2 = (float *)(unaff_EBX + 0x485dcf);
      *(uint *)((int)local_1d8 + local_288) =
           ((uint)(*pfVar7 * local_1a4.y + *pfVar1) & 0xff) << 8 |
           ((uint)(*pfVar7 * local_1a4.z + *pfVar1) & 0xff) << 0x10 |
           (uint)(*pfVar7 * local_1a4.x + *pfVar1) & 0xff | (int)(*pfVar2 * 0.1 + *pfVar1) << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
      *local_1d4 = 0;
      local_1d4[1] = 0;
                    /* Unresolved local var: float * pDst@[???] */
      *local_1e0 = local_148 + local_16c.x + local_194.x;
      local_1e0[1] = local_144 + local_16c.y + local_194.y;
      local_1e0[2] = local_140 + local_16c.z + local_194.z;
      iVar10 = local_1e4 + 2;
      if (local_1e4 + 2 <= iVar5) {
        iVar10 = iVar5;
      }
      local_1e0 = (float *)((int)local_1e0 + local_298[0]);
      local_1d8 = (uint *)((int)local_1d8 + local_288 + local_288);
      local_1d4 = (undefined4 *)((int)local_1d4 + local_280);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
      pfVar7 = (float *)(unaff_EBX + 0x4878c3);
      *local_1d8 = ((uint)(*pfVar2 * local_1a4.y + *pfVar1) & 0xff) << 8 |
                   ((uint)(*pfVar2 * local_1a4.z + *pfVar1) & 0xff) << 0x10 |
                   (uint)(*pfVar2 * local_1a4.x + *pfVar1) & 0xff |
                   (int)(*pfVar2 * 0.1 + *pfVar7) << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
      *local_1d4 = 0x3f800000;
      local_1d4[1] = 0;
                    /* Unresolved local var: float * pDst@[???] */
      *local_1e0 = (local_148 - local_16c.x) + local_194.x;
      local_1e0[1] = (local_144 - local_16c.y) + local_194.y;
      local_1e0[2] = (local_140 - local_16c.z) + local_194.z;
      local_1e8 = local_1e4 + 3;
      if (local_1e4 + 3 <= iVar10) {
        local_1e8 = iVar10;
      }
      local_1e0 = (float *)((int)local_1e0 + local_298[0]);
      local_1d8 = (uint *)((int)local_1d8 + local_288);
      local_1d4 = (undefined4 *)((int)local_1d4 + local_280);
      local_184.x = (local_148 - local_16c.x) - local_194.x;
      local_184.y = (local_144 - local_16c.y) - local_194.y;
      local_184.z = (local_140 - local_16c.z) - local_194.z;
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
      pfVar1 = (float *)(unaff_EBX + 0x485dcf);
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
      local_28 = *pfVar1 * 0.1 + *(float *)(unaff_EBX + 0x4878c3);
      *local_1d8 = ((uint)(*pfVar1 * local_1a4.y + *pfVar7) & 0xff) << 8 |
                   ((uint)(*pfVar1 * local_1a4.z + *pfVar7) & 0xff) << 0x10 |
                   (uint)(*pfVar1 * local_1a4.x + *pfVar7) & 0xff | (int)local_28 << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
      *local_1d4 = 0x3f800000;
      local_1d4[1] = fVar13;
                    /* Unresolved local var: float * pDst@[???] */
      *local_1e0 = local_184.x;
      local_1e0[1] = local_184.y;
      local_1e0[2] = local_184.z;
      local_1e4 = local_1e4 + 4;
      if (local_1e8 < local_1e4) {
        local_1e8 = local_1e4;
      }
      local_1e0 = (float *)((int)local_1e0 + local_298[0]);
      local_1d8 = (uint *)((int)local_1d8 + local_288);
      local_1d4 = (undefined4 *)((int)local_1d4 + local_280);
      if (local_2d0 != '\0') {
        if (local_2d4 - 4 < 7) {
                    /* WARNING: Could not recover jumptable at 0x00531ab2. Too many branches */
                    /* WARNING: Treating indirect jump as call */
          iVar5 = (*(code *)(*(int *)(unaff_EBX + 0xc83 + (local_2d4 - 4) * 4) + unaff_EBX))();
          return iVar5;
        }
        if (local_2c0 != 0) {
          iVar5 = local_2b8 - local_2ac;
          if (local_1e8 <= local_2b8 - local_2ac) {
            iVar5 = local_1e8;
          }
          if (iVar5 != 0) {
            if (local_2d4 < 0xb) {
                    /* WARNING: Could not recover jumptable at 0x00534cdd. Too many branches */
                    /* WARNING: Treating indirect jump as call */
              iVar5 = (*(code *)(*(int *)(unaff_EBX + 0x3eaf + local_2d4 * 4) + unaff_EBX))();
              return iVar5;
            }
            GenerateSequentialIndexBuffer((ushort *)(local_2ac * 2 + local_2cc),iVar5,local_2b0);
            local_2ac = iVar5 * local_2c0 + local_2ac;
            if (local_2b4 < local_2ac) {
              local_2b4 = local_2ac;
            }
          }
        }
      }
      (**(code **)(*local_2d8 + 0x50))(local_2d8,local_1e8,local_2b4,local_38c);
      local_2bc = (int *)0x0;
      local_2b8 = 0;
      local_1ec = 0;
      local_1f4 = (int *)0x0;
      local_24c = 0xffffffff;
      local_2d8 = (int *)0x0;
      (**(code **)(*piVar8 + 0x2c))(piVar8,0xffffffff,0);
      piVar8 = (int *)(**(code **)(*(int *)local_4c.x + 0xe4))
                                (local_4c.x,1,0,0,this->m_pSideRailMaterial);
      local_4a4 = (int *)0x0;
      local_4a0 = 0;
      local_49c = 0;
      local_494 = 0;
      local_490 = 0;
      local_48c = 0xffffffff;
      local_488 = 0;
      local_3dc = (int *)0x0;
      local_398 = -1;
      local_394 = 0;
      local_3d0 = 0;
      local_3cc = 0;
      local_3d4 = 0;
      local_39c = 0;
      local_434 = 0xffffffff;
      local_4b8 = '\x01';
      local_4bc = 7;
      local_4c0 = piVar8;
      (**(code **)(*piVar8 + 0x28))(piVar8,2);
      (**(code **)(*local_4c0 + 0x44))(local_4c0,0x14,0x1e,local_574);
      local_4a4 = (int *)0x0;
      if (piVar8 != (int *)0x0) {
        local_4a4 = piVar8 + 1;
      }
      local_49c = 0;
      local_4a0 = 0x1e;
      local_484 = 0;
      local_498 = local_4d8;
      local_4b4 = local_4d0;
      local_4a8 = local_4c4;
      local_494 = 0;
      local_3dc = piVar8;
      _memcpy(local_480,local_574,0xa4);
      local_3d4 = 0x14;
      local_430 = ~-(uint)(local_430 == 0) & 2;
      local_3d0 = 0;
      local_3d8 = 0;
      if (local_398 == -1) {
        local_39c = 0;
        local_398 = local_4d4;
        local_394 = local_4d8;
      }
      local_494 = 0;
      local_3cc = 0;
      local_3c8 = local_42c;
      local_3c4 = local_420;
                    /* Unresolved local var: int i@[???] */
      local_3bc = local_414;
      local_3b8 = local_410;
      local_3b4 = local_40c;
      local_3b0 = local_408;
      local_3ac = local_404;
      local_3a8 = local_400;
      local_3a4 = local_3fc;
      local_3a0 = local_3f8;
      local_3c0 = local_41c;
      pQVar6 = C_BaseEntity::GetNetworkAngles((C_BaseEntity *)this);
      AngleVectors(pQVar6,(Vector *)0x0,(Vector *)0x0,&local_124);
      fVar18 = *(float *)(unaff_EBX + 0x485e9b);
      local_124.x = local_124.x * fVar18;
      local_124.y = local_124.y * fVar18;
      local_124.z = fVar18 * local_124.z;
                    /* Unresolved local var: Vector res@[???] */
      local_154 = fVar12 + local_16c.x;
      local_150 = fVar17 + local_16c.y;
      local_14c = fVar20 + local_16c.z;
                    /* Unresolved local var: Vector vert@[???]
                       Unresolved local var: float[4] pColors@[???] */
      local_184.x = (local_154 + local_124.x) - local_194.x;
      local_184.y = (local_150 + local_124.y) - local_194.y;
      local_184.z = (local_14c + local_124.z) - local_194.z;
      (*this->m_pMyColor->_vptr_IMaterialVar[0xd])(this->m_pMyColor,&local_1a4,3);
      local_198 = 0x3f800000;
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
      pfVar7 = (float *)(unaff_EBX + 0x485dcf);
      pfVar1 = (float *)(unaff_EBX + 0x4878c3);
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
      *local_3c0 = ((uint)(*pfVar7 * local_1a4.y + *pfVar1) & 0xff) << 8 |
                   ((uint)(*pfVar7 * local_1a4.z + *pfVar1) & 0xff) << 0x10 |
                   (uint)(*pfVar7 * local_1a4.x + *pfVar1) & 0xff | 0xff000000;
                    /* Unresolved local var: float * pDst@[???] */
      *local_3bc = 0;
      local_3bc[1] = fVar13;
                    /* Unresolved local var: float * pDst@[???] */
      *local_3c8 = local_184.x;
      local_3c8[1] = local_184.y;
      local_3c8[2] = local_184.z;
      iVar5 = local_3cc + 1;
      if (local_3cc + 1 <= local_3d0) {
        iVar5 = local_3d0;
      }
      local_3c8 = (float *)((int)local_3c8 + local_480[0]);
      local_3c4 = local_3c4 + local_474 * 4;
      local_3bc = (undefined4 *)((int)local_3bc + local_468);
      local_3b8 = local_3b8 + local_464 * 4;
      local_3b4 = local_3b4 + local_460 * 4;
      local_3b0 = local_3b0 + local_45c * 4;
      local_3ac = local_3ac + local_458 * 4;
      local_3a8 = local_3a8 + local_454 * 4;
      local_3a4 = local_3a4 + local_450 * 4;
      local_3a0 = local_3a0 + local_44c * 4;
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
      pfVar2 = (float *)(unaff_EBX + 0x485dcf);
      *(uint *)((int)local_3c0 + local_470) =
           ((uint)(*pfVar7 * local_1a4.y + *pfVar1) & 0xff) << 8 |
           ((uint)(*pfVar7 * local_1a4.z + *pfVar1) & 0xff) << 0x10 |
           (uint)(*pfVar7 * local_1a4.x + *pfVar1) & 0xff | (int)(*pfVar2 * 1.0 + *pfVar1) << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
      *local_3bc = 0;
      local_3bc[1] = 0;
                    /* Unresolved local var: float * pDst@[???] */
      *local_3c8 = local_154 + local_124.x + local_194.x;
      local_3c8[1] = local_150 + local_124.y + local_194.y;
      local_3c8[2] = local_14c + local_124.z + local_194.z;
      iVar10 = local_3cc + 2;
      if (local_3cc + 2 <= iVar5) {
        iVar10 = iVar5;
      }
      local_3c8 = (float *)((int)local_3c8 + local_480[0]);
      local_3c0 = (uint *)((int)local_3c0 + local_470 + local_470);
      local_3bc = (undefined4 *)((int)local_3bc + local_468);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
      pfVar7 = (float *)(unaff_EBX + 0x4878c3);
      *local_3c0 = ((uint)(*pfVar2 * local_1a4.y + *pfVar1) & 0xff) << 8 |
                   ((uint)(*pfVar2 * local_1a4.z + *pfVar1) & 0xff) << 0x10 |
                   (uint)(*pfVar2 * local_1a4.x + *pfVar1) & 0xff |
                   (int)(*pfVar2 * 1.0 + *pfVar7) << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
      *local_3bc = 0x3f800000;
      local_3bc[1] = 0;
                    /* Unresolved local var: float * pDst@[???] */
      *local_3c8 = (local_154 - local_124.x) + local_194.x;
      local_3c8[1] = (local_150 - local_124.y) + local_194.y;
      local_3c8[2] = (local_14c - local_124.z) + local_194.z;
      iVar5 = local_3cc + 3;
      if (local_3cc + 3 <= iVar10) {
        iVar5 = iVar10;
      }
      local_3c8 = (float *)((int)local_3c8 + local_480[0]);
      local_3c0 = (uint *)((int)local_3c0 + local_470);
      local_3bc = (undefined4 *)((int)local_3bc + local_468);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
      local_28 = *(float *)(unaff_EBX + 0x485dcf) * 1.0 + *pfVar7;
      *local_3c0 = ((uint)(*pfVar2 * local_1a4.y + *pfVar7) & 0xff) << 8 |
                   ((uint)(*pfVar2 * local_1a4.z + *pfVar7) & 0xff) << 0x10 |
                   (uint)(*pfVar2 * local_1a4.x + *pfVar7) & 0xff | (int)local_28 << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
      *local_3bc = 0x3f800000;
      local_3bc[1] = fVar13;
                    /* Unresolved local var: float * pDst@[???] */
      *local_3c8 = (local_154 - local_124.x) - local_194.x;
      local_3c8[1] = (local_150 - local_124.y) - local_194.y;
      local_3c8[2] = (local_14c - local_124.z) - local_194.z;
      local_3cc = local_3cc + 4;
      local_3d0 = local_3cc;
      if (local_3cc <= iVar5) {
        local_3d0 = iVar5;
      }
      local_3c8 = (float *)((int)local_3c8 + local_480[0]);
      local_3c0 = (uint *)((int)local_3c0 + local_470);
      local_3bc = (undefined4 *)((int)local_3bc + local_468);
                    /* Unresolved local var: Vector res@[???] */
      local_160 = fVar12 - local_16c.x;
      local_15c = fVar17 - local_16c.y;
      local_158 = fVar20 - local_16c.z;
                    /* Unresolved local var: Vector vert@[???]
                       Unresolved local var: float[4] pColors@[???] */
      local_184.x = (local_160 + local_124.x) - local_194.x;
      local_184.y = (local_15c + local_124.y) - local_194.y;
      local_184.z = (local_158 + local_124.z) - local_194.z;
      (*this->m_pMyColor->_vptr_IMaterialVar[0xd])(this->m_pMyColor,&local_1a4,3);
      local_198 = 0x3f800000;
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
      pfVar7 = (float *)(unaff_EBX + 0x485dcf);
      pfVar1 = (float *)(unaff_EBX + 0x4878c3);
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
      *local_3c0 = ((uint)(*pfVar7 * local_1a4.y + *pfVar1) & 0xff) << 8 |
                   ((uint)(*pfVar7 * local_1a4.z + *pfVar1) & 0xff) << 0x10 |
                   (uint)(*pfVar7 * local_1a4.x + *pfVar1) & 0xff | 0xff000000;
                    /* Unresolved local var: float * pDst@[???] */
      *local_3bc = 0;
      local_3bc[1] = fVar13;
                    /* Unresolved local var: float * pDst@[???] */
      *local_3c8 = local_184.x;
      local_3c8[1] = local_184.y;
      local_3c8[2] = local_184.z;
      iVar5 = local_3cc + 1;
      if (local_3cc + 1 <= local_3d0) {
        iVar5 = local_3d0;
      }
      local_3c8 = (float *)((int)local_3c8 + local_480[0]);
      local_3c4 = local_3c4 + local_474 * 4;
      local_3bc = (undefined4 *)((int)local_3bc + local_468);
      local_3b8 = local_3b8 + local_464 * 4;
      local_3b4 = local_3b4 + local_460 * 4;
      local_3b0 = local_3b0 + local_45c * 4;
      local_3ac = local_3ac + local_458 * 4;
      local_3a8 = local_3a8 + local_454 * 4;
      local_3a4 = local_3a4 + local_450 * 4;
      local_3a0 = local_3a0 + local_44c * 4;
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
      pfVar2 = (float *)(unaff_EBX + 0x485dcf);
      *(uint *)((int)local_3c0 + local_470) =
           ((uint)(*pfVar7 * local_1a4.y + *pfVar1) & 0xff) << 8 |
           ((uint)(*pfVar7 * local_1a4.z + *pfVar1) & 0xff) << 0x10 |
           (uint)(*pfVar7 * local_1a4.x + *pfVar1) & 0xff | (int)(*pfVar2 * 1.0 + *pfVar1) << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
      *local_3bc = 0;
      local_3bc[1] = 0;
                    /* Unresolved local var: float * pDst@[???] */
      *local_3c8 = local_160 + local_124.x + local_194.x;
      local_3c8[1] = local_15c + local_124.y + local_194.y;
      local_3c8[2] = local_158 + local_124.z + local_194.z;
      iVar10 = local_3cc + 2;
      if (local_3cc + 2 <= iVar5) {
        iVar10 = iVar5;
      }
      local_3c8 = (float *)((int)local_3c8 + local_480[0]);
      local_3c0 = (uint *)((int)local_3c0 + local_470 + local_470);
      local_3bc = (undefined4 *)((int)local_3bc + local_468);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
      pfVar7 = (float *)(unaff_EBX + 0x4878c3);
      *local_3c0 = ((uint)(*pfVar2 * local_1a4.y + *pfVar1) & 0xff) << 8 |
                   ((uint)(*pfVar2 * local_1a4.z + *pfVar1) & 0xff) << 0x10 |
                   (uint)(*pfVar2 * local_1a4.x + *pfVar1) & 0xff |
                   (int)(*pfVar2 * 1.0 + *pfVar7) << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
      *local_3bc = 0x3f800000;
      local_3bc[1] = 0;
                    /* Unresolved local var: float * pDst@[???] */
      *local_3c8 = (local_160 - local_124.x) + local_194.x;
      local_3c8[1] = (local_15c - local_124.y) + local_194.y;
      local_3c8[2] = (local_158 - local_124.z) + local_194.z;
      local_3d0 = local_3cc + 3;
      if (local_3cc + 3 <= iVar10) {
        local_3d0 = iVar10;
      }
      local_3c8 = (float *)((int)local_3c8 + local_480[0]);
      local_3c0 = (uint *)((int)local_3c0 + local_470);
      local_3bc = (undefined4 *)((int)local_3bc + local_468);
      local_184.x = (local_160 - local_124.x) - local_194.x;
      local_184.y = (local_15c - local_124.y) - local_194.y;
      local_184.z = (local_158 - local_124.z) - local_194.z;
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
      local_28 = *(float *)(unaff_EBX + 0x485dcf) * 1.0 + *pfVar7;
      *local_3c0 = ((uint)(*pfVar2 * local_1a4.y + *pfVar7) & 0xff) << 8 |
                   ((uint)(*pfVar2 * local_1a4.z + *pfVar7) & 0xff) << 0x10 |
                   (uint)(*pfVar2 * local_1a4.x + *pfVar7) & 0xff | (int)local_28 << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
      *local_3bc = 0x3f800000;
      local_3bc[1] = fVar13;
                    /* Unresolved local var: float * pDst@[???] */
      *local_3c8 = local_184.x;
      local_3c8[1] = local_184.y;
      local_3c8[2] = local_184.z;
      local_3cc = local_3cc + 4;
      if (local_3d0 < local_3cc) {
        local_3d0 = local_3cc;
      }
      local_3c8 = (float *)((int)local_3c8 + local_480[0]);
      local_3c0 = (uint *)((int)local_3c0 + local_470);
      local_3bc = (undefined4 *)((int)local_3bc + local_468);
      if (local_4b8 != '\0') {
        if (local_4bc - 4 < 7) {
                    /* WARNING: Could not recover jumptable at 0x00532eaf. Too many branches */
                    /* WARNING: Treating indirect jump as call */
          iVar5 = (*(code *)(*(int *)(unaff_EBX + 0x2083 + (local_4bc - 4) * 4) + unaff_EBX))();
          return iVar5;
        }
        if (local_4a8 != 0) {
          iVar5 = local_4a0 - local_494;
          if (local_3d0 <= local_4a0 - local_494) {
            iVar5 = local_3d0;
          }
          if (iVar5 != 0) {
            if (local_4bc < 0xb) {
                    /* WARNING: Could not recover jumptable at 0x00534d15. Too many branches */
                    /* WARNING: Treating indirect jump as call */
              iVar5 = (*(code *)(*(int *)(unaff_EBX + 0x3ee7 + local_4bc * 4) + unaff_EBX))();
              return iVar5;
            }
            GenerateSequentialIndexBuffer((ushort *)(local_494 * 2 + local_4b4),iVar5,local_498);
            local_494 = iVar5 * local_4a8 + local_494;
            if (local_49c < local_494) {
              local_49c = local_494;
            }
          }
        }
      }
      (**(code **)(*local_4c0 + 0x50))(local_4c0,local_3d0,local_49c,local_574);
      local_4a4 = (int *)0x0;
      local_4a0 = 0;
      local_3d4 = 0;
      local_3dc = (int *)0x0;
      local_434 = 0xffffffff;
      local_4c0 = (int *)0x0;
      (**(code **)(*piVar8 + 0x2c))(piVar8,0xffffffff,0);
      if ((local_3dc != (int *)0x0) &&
         (cVar4 = (**(code **)(*local_3dc + 8))(local_3dc), cVar4 != '\0')) {
        (**(code **)(*local_3dc + 0x10))(local_3dc);
      }
      if ((local_4a4 != (int *)0x0) &&
         (cVar4 = (**(code **)(*local_4a4 + 8))(local_4a4), cVar4 != '\0')) {
        (**(code **)(*local_4a4 + 0x10))(local_4a4);
      }
      if ((local_1f4 != (int *)0x0) &&
         (cVar4 = (**(code **)(*local_1f4 + 8))(local_1f4), cVar4 != '\0')) {
        (**(code **)(*local_1f4 + 0x10))(local_1f4);
      }
      if ((local_2bc != (int *)0x0) &&
         (cVar4 = (**(code **)(*local_2bc + 8))(local_2bc), cVar4 != '\0')) {
        (**(code **)(*local_2bc + 0x10))(local_2bc);
      }
      piVar8 = (int *)local_4c.x;
      if ((int *)local_4c.x != (int *)0x0) {
        (**(code **)(*(int *)local_4c.x + 0xc))(local_4c.x);
        piVar8 = (int *)local_4c.x;
      }
    }
    else {
      local_20 = (int *)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x600097) + 400))
                                  ((int *)**(undefined4 **)(unaff_EBX + 0x600097));
      if (local_20 != (int *)0x0) {
        (**(code **)(*local_20 + 8))(local_20);
      }
      piVar8 = (int *)(**(code **)(*local_20 + 0xe4))(local_20,1,0,0,this->m_pBodyMaterial);
      local_2d8 = (int *)0x0;
      local_2d0 = '\0';
      local_2bc = (int *)0x0;
      local_2b8 = 0;
      local_2b4 = 0;
      local_2ac = 0;
      local_2a8 = 0;
      local_2a4 = 0xffffffff;
      local_2a0 = 0;
      local_1f4 = (int *)0x0;
      local_1b0 = -1;
      local_1ac = 0;
      local_1e8 = 0;
      local_1e4 = 0;
      local_1ec = 0;
      local_1b4 = 0;
      local_24c = 0xffffffff;
      pQVar6 = C_BaseEntity::GetNetworkAngles((C_BaseEntity *)this);
      forward = local_574;
      AngleVectors(pQVar6,forward,&local_1a4,&local_184);
      fVar12 = this->m_flWidth;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
      pfVar7 = (float *)(&DAT_00485a9f + unaff_EBX);
      fVar18 = *pfVar7 * fVar12 * local_1a4.x;
      fVar17 = *pfVar7 * fVar12 * local_1a4.y;
      fVar20 = *pfVar7 * fVar12 * local_1a4.z;
      fVar12 = this->m_flHeight;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
      fVar19 = *pfVar7 * fVar12 * local_184.x;
      fVar13 = *pfVar7 * fVar12 * local_184.y;
      fVar12 = *pfVar7 * fVar12 * local_184.z;
      local_40 = fVar19;
      local_3c = fVar13;
      local_38 = fVar12;
      local_34 = fVar18;
      local_30 = fVar17;
      local_2c = fVar20;
      pfVar7 = (float *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                          super_C_BaseProjectedEntity.super_C_BaseEntity.super_IClientEntity.
                          super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0xe4])(this);
      bVar11 = iVar5 != 0;
      local_124.x = *pfVar7;
      local_124.y = pfVar7[1];
      local_124.z = pfVar7[2];
      if (bVar11) {
                    /* Unresolved local var: Vector vecWallEnd@[???] */
        local_50 = this->m_flCurDisplayLength;
                    /* Unresolved local var: Vector res@[???] */
        local_58 = local_50 * local_574[0].x;
        local_54 = local_50 * local_574[0].y;
        local_50 = local_50 * local_574[0].z;
                    /* Unresolved local var: Vector res@[???] */
        local_64 = (int *)(local_124.x + local_58);
        local_60 = local_124.y + local_54;
        local_5c = local_124.z + local_50;
        forward = (Vector *)0x40a00000;
        local_4c.x = (vec_t)local_64;
        local_4c.y = local_60;
        local_4c.z = local_5c;
        NDebugOverlay::Sphere(&local_4c,5.0,0,0xff,0,true,0.1);
      }
      local_584 = this->m_flSegmentLength;
      fVar14 = local_584 / *(float *)(*(int *)(unaff_EBX + 0x81ee2b) + 0x2c);
      fVar21 = this->m_flCurDisplayLength;
      if (local_584 < fVar21) {
        local_59c = *(float *)(&DAT_00485a9f + unaff_EBX) * local_584;
                    /* Unresolved local var: Vector res@[???] */
        local_594 = local_59c * local_574[0].x;
        local_598 = local_59c * local_574[0].y;
        local_59c = local_59c * local_574[0].z;
        local_74 = *(float *)(&DAT_00485a9f + unaff_EBX) * this->m_flSegmentLength;
                    /* Unresolved local var: Vector res@[???] */
        local_7c = local_74 * local_574[0].x;
        local_78 = local_74 * local_574[0].y;
        local_74 = local_74 * local_574[0].z;
        local_70 = local_594;
        local_6c = local_598;
        local_68 = local_59c;
        fVar21 = local_7c;
        fVar16 = local_78;
        fVar15 = local_74;
      }
      else {
        local_59c = *(float *)(&DAT_00485a9f + unaff_EBX) * fVar21;
                    /* Unresolved local var: Vector res@[???] */
        local_594 = local_59c * local_574[0].x;
        local_598 = local_59c * local_574[0].y;
        local_59c = local_59c * local_574[0].z;
        local_8c = *(float *)(&DAT_00485a9f + unaff_EBX) * this->m_flCurDisplayLength;
                    /* Unresolved local var: Vector res@[???] */
        local_94 = local_8c * local_574[0].x;
        local_90 = local_8c * local_574[0].y;
        local_8c = local_8c * local_574[0].z;
        local_584 = fVar21;
        local_88 = local_594;
        local_84 = local_598;
        local_80 = local_59c;
        fVar21 = local_94;
        fVar16 = local_90;
        fVar15 = local_8c;
      }
      local_124.z = local_124.z + fVar15;
      local_124.y = local_124.y + fVar16;
      local_124.x = local_124.x + fVar21;
      if (0 < this->m_nNumSegments) {
        local_5ac = 0.0;
        local_5a8 = 0;
        do {
                    /* Unresolved local var: float[3] flColors@[???] */
          local_16c.x = 1.0;
          local_16c.y = 1.0;
          local_16c.z = 1.0;
          pCVar9 = (Color *)(this->m_PaintPowers).m_Memory.m_pMemory[local_5a8];
          if (pCVar9 != (Color *)&vgui::CvarToggleCheckButton<CGameUIConVarRef>::GetKBMap::s_pMap.
                                  cputype) {
            pCVar9 = MapPowerToVisualColor(pCVar9,(int)forward);
                    /* Unresolved local var: Color color@[???] */
            pfVar7 = (float *)(unaff_EBX + 0x485dcf);
            local_16c.x = (float)((uint)pCVar9 & 0xff) / *pfVar7;
            local_16c.y = (float)((uint)pCVar9 >> 8 & 0xff) / *pfVar7;
            local_16c.z = (float)((uint)pCVar9 >> 0x10 & 0xff) / *pfVar7;
          }
          (*this->m_pMyColor->_vptr_IMaterialVar[9])(this->m_pMyColor,&local_16c,3);
          local_2d0 = '\x01';
          local_2d4 = 7;
          local_2d8 = piVar8;
          (**(code **)(*piVar8 + 0x28))(piVar8,2);
          (**(code **)(*local_2d8 + 0x44))(local_2d8,0x14,0x1e,local_38c);
          local_2bc = (int *)0x0;
          if (piVar8 != (int *)0x0) {
            local_2bc = piVar8 + 1;
          }
          local_2b4 = 0;
          local_2b8 = 0x1e;
          local_29c = 0;
          local_2b0 = local_2f0;
          local_2cc = local_2e8;
          local_2c0 = local_2dc;
          local_2ac = 0;
          local_1f4 = piVar8;
          _memcpy(local_298,local_38c,0xa4);
          local_1ec = 0x14;
          local_248 = ~-(uint)(local_248 == 0) & 2;
          local_1e8 = 0;
          local_1f0 = 0;
          if (local_1b0 == -1) {
            local_1b4 = 0;
            local_1b0 = local_2ec;
            local_1ac = local_2f0;
          }
          local_2ac = 0;
          local_1e4 = 0;
          local_1e0 = local_244;
          local_1dc = local_238;
                    /* Unresolved local var: int i@[???] */
          local_1d4 = local_22c;
          local_1d0 = local_228;
          local_1cc = local_224;
          local_1c8 = local_220;
          local_1c4 = local_21c;
          local_1c0 = local_218;
          local_1bc = local_214;
          local_1b8 = local_210;
          local_1d8 = local_234;
                    /* Unresolved local var: Vector res@[???] */
          local_a0 = fVar19 + local_124.x;
          local_9c = fVar13 + local_124.y;
          local_98 = fVar12 + local_124.z;
                    /* Unresolved local var: Vector vert@[???]
                       Unresolved local var: float[4] pColors@[???] */
          local_16c.x = (local_a0 + fVar18) - local_594;
          local_16c.y = (local_9c + fVar17) - local_598;
          local_16c.z = (local_98 + fVar20) - local_59c;
          (*this->m_pMyColor->_vptr_IMaterialVar[0xd])(this->m_pMyColor,&local_194,3);
          local_188 = 0x3f800000;
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
          pfVar7 = (float *)(unaff_EBX + 0x485dcf);
          pfVar1 = (float *)(unaff_EBX + 0x4878c3);
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
          *local_1d8 = ((uint)(*pfVar7 * local_194.y + *pfVar1) & 0xff) << 8 |
                       ((uint)(*pfVar7 * local_194.z + *pfVar1) & 0xff) << 0x10 |
                       (uint)(*pfVar7 * local_194.x + *pfVar1) & 0xff | 0xff000000;
                    /* Unresolved local var: float * pDst@[???] */
          *local_1d4 = 0;
          local_1d4[1] = fVar14;
                    /* Unresolved local var: float * pDst@[???] */
          *local_1e0 = local_16c.x;
          local_1e0[1] = local_16c.y;
          local_1e0[2] = local_16c.z;
          iVar5 = local_1e4 + 1;
          if (local_1e4 + 1 <= local_1e8) {
            iVar5 = local_1e8;
          }
          local_1e0 = (float *)((int)local_1e0 + local_298[0]);
          local_1dc = local_1dc + local_28c * 4;
          local_1d4 = (undefined4 *)((int)local_1d4 + local_280);
          local_1d0 = local_1d0 + local_27c * 4;
          local_1cc = local_1cc + local_278 * 4;
          local_1c8 = local_1c8 + local_274 * 4;
          local_1c4 = local_1c4 + local_270 * 4;
          local_1c0 = local_1c0 + local_26c * 4;
          local_1bc = local_1bc + local_268 * 4;
          local_1b8 = local_1b8 + local_264 * 4;
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
          pfVar2 = (float *)(unaff_EBX + 0x485dcf);
          *(uint *)((int)local_1d8 + local_288) =
               ((uint)(*pfVar7 * local_194.y + *pfVar1) & 0xff) << 8 |
               ((uint)(*pfVar7 * local_194.z + *pfVar1) & 0xff) << 0x10 |
               (uint)(*pfVar7 * local_194.x + *pfVar1) & 0xff |
               (int)(*pfVar2 * 1.0 + *pfVar1) << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
          *local_1d4 = 0;
          local_1d4[1] = 0;
                    /* Unresolved local var: float * pDst@[???] */
          *local_1e0 = fVar18 + local_a0 + local_594;
          local_1e0[1] = fVar17 + local_9c + local_598;
          local_1e0[2] = fVar20 + local_98 + local_59c;
          iVar10 = local_1e4 + 2;
          if (local_1e4 + 2 <= iVar5) {
            iVar10 = iVar5;
          }
          local_1e0 = (float *)((int)local_1e0 + local_298[0]);
          local_1d8 = (uint *)((int)local_1d8 + local_288 + local_288);
          local_1d4 = (undefined4 *)((int)local_1d4 + local_280);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
          pfVar7 = (float *)(unaff_EBX + 0x4878c3);
          *local_1d8 = ((uint)(*pfVar2 * local_194.y + *pfVar1) & 0xff) << 8 |
                       ((uint)(*pfVar2 * local_194.z + *pfVar1) & 0xff) << 0x10 |
                       (uint)(*pfVar2 * local_194.x + *pfVar1) & 0xff |
                       (int)(*pfVar2 * 1.0 + *pfVar7) << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
          *local_1d4 = 0x3f800000;
          local_1d4[1] = 0;
                    /* Unresolved local var: float * pDst@[???] */
          *local_1e0 = (local_a0 - fVar18) + local_594;
          local_1e0[1] = (local_9c - fVar17) + local_598;
          local_1e0[2] = (local_98 - fVar20) + local_59c;
          iVar5 = local_1e4 + 3;
          if (local_1e4 + 3 <= iVar10) {
            iVar5 = iVar10;
          }
          local_1e0 = (float *)((int)local_1e0 + local_298[0]);
          local_1d8 = (uint *)((int)local_1d8 + local_288);
          local_1d4 = (undefined4 *)((int)local_1d4 + local_280);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
          local_28 = *(float *)(unaff_EBX + 0x485dcf) * 1.0 + *pfVar7;
          *local_1d8 = ((uint)(*pfVar2 * local_194.y + *pfVar7) & 0xff) << 8 |
                       ((uint)(*pfVar2 * local_194.z + *pfVar7) & 0xff) << 0x10 |
                       (uint)(*pfVar2 * local_194.x + *pfVar7) & 0xff | (int)local_28 << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
          *local_1d4 = 0x3f800000;
          local_1d4[1] = fVar14;
                    /* Unresolved local var: float * pDst@[???] */
          *local_1e0 = (local_a0 - fVar18) - local_594;
          local_1e0[1] = (local_9c - fVar17) - local_598;
          local_1e0[2] = (local_98 - fVar20) - local_59c;
          local_1e4 = local_1e4 + 4;
          local_1e8 = local_1e4;
          if (local_1e4 <= iVar5) {
            local_1e8 = iVar5;
          }
          local_1e0 = (float *)((int)local_1e0 + local_298[0]);
          local_1d8 = (uint *)((int)local_1d8 + local_288);
          local_1d4 = (undefined4 *)((int)local_1d4 + local_280);
                    /* Unresolved local var: Vector res@[???] */
          local_ac = fVar18 + local_124.x;
          local_a8 = fVar17 + local_124.y;
          local_a4 = fVar20 + local_124.z;
                    /* Unresolved local var: Vector vert@[???]
                       Unresolved local var: float[4] pColors@[???] */
          local_16c.x = (local_ac + fVar19) - local_594;
          local_16c.y = (local_a8 + fVar13) - local_598;
          local_16c.z = (local_a4 + fVar12) - local_59c;
          (*this->m_pMyColor->_vptr_IMaterialVar[0xd])(this->m_pMyColor,&local_194,3);
          local_188 = 0x3f800000;
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
          pfVar7 = (float *)(unaff_EBX + 0x485dcf);
          pfVar1 = (float *)(unaff_EBX + 0x4878c3);
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
          *local_1d8 = ((uint)(*pfVar7 * local_194.y + *pfVar1) & 0xff) << 8 |
                       ((uint)(*pfVar7 * local_194.z + *pfVar1) & 0xff) << 0x10 |
                       (uint)(*pfVar7 * local_194.x + *pfVar1) & 0xff | 0xff000000;
                    /* Unresolved local var: float * pDst@[???] */
          *local_1d4 = 0;
          local_1d4[1] = fVar14;
                    /* Unresolved local var: float * pDst@[???] */
          *local_1e0 = local_16c.x;
          local_1e0[1] = local_16c.y;
          local_1e0[2] = local_16c.z;
          iVar5 = local_1e4 + 1;
          if (local_1e4 + 1 <= local_1e8) {
            iVar5 = local_1e8;
          }
          local_1e0 = (float *)((int)local_1e0 + local_298[0]);
          local_1dc = local_1dc + local_28c * 4;
          local_1d4 = (undefined4 *)((int)local_1d4 + local_280);
          local_1d0 = local_1d0 + local_27c * 4;
          local_1cc = local_1cc + local_278 * 4;
          local_1c8 = local_1c8 + local_274 * 4;
          local_1c4 = local_1c4 + local_270 * 4;
          local_1c0 = local_1c0 + local_26c * 4;
          local_1bc = local_1bc + local_268 * 4;
          local_1b8 = local_1b8 + local_264 * 4;
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
          pfVar2 = (float *)(unaff_EBX + 0x485dcf);
          *(uint *)((int)local_1d8 + local_288) =
               ((uint)(*pfVar7 * local_194.y + *pfVar1) & 0xff) << 8 |
               ((uint)(*pfVar7 * local_194.z + *pfVar1) & 0xff) << 0x10 |
               (uint)(*pfVar7 * local_194.x + *pfVar1) & 0xff |
               (int)(*pfVar2 * 1.0 + *pfVar1) << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
          *local_1d4 = 0;
          local_1d4[1] = 0;
                    /* Unresolved local var: float * pDst@[???] */
          *local_1e0 = fVar19 + local_ac + local_594;
          local_1e0[1] = fVar13 + local_a8 + local_598;
          local_1e0[2] = fVar12 + local_a4 + local_59c;
          iVar10 = local_1e4 + 2;
          if (local_1e4 + 2 <= iVar5) {
            iVar10 = iVar5;
          }
          local_1e0 = (float *)((int)local_1e0 + local_298[0]);
          local_1d8 = (uint *)((int)local_1d8 + local_288 + local_288);
          local_1d4 = (undefined4 *)((int)local_1d4 + local_280);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
          pfVar7 = (float *)(unaff_EBX + 0x4878c3);
          *local_1d8 = ((uint)(*pfVar2 * local_194.y + *pfVar1) & 0xff) << 8 |
                       ((uint)(*pfVar2 * local_194.z + *pfVar1) & 0xff) << 0x10 |
                       (uint)(*pfVar2 * local_194.x + *pfVar1) & 0xff |
                       (int)(*pfVar2 * 1.0 + *pfVar7) << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
          *local_1d4 = 0x3f800000;
          local_1d4[1] = 0;
                    /* Unresolved local var: float * pDst@[???] */
          *local_1e0 = (local_ac - fVar19) + local_594;
          local_1e0[1] = (local_a8 - fVar13) + local_598;
          local_1e0[2] = (local_a4 - fVar12) + local_59c;
          iVar5 = local_1e4 + 3;
          if (local_1e4 + 3 <= iVar10) {
            iVar5 = iVar10;
          }
          local_1e0 = (float *)((int)local_1e0 + local_298[0]);
          local_1d8 = (uint *)((int)local_1d8 + local_288);
          local_1d4 = (undefined4 *)((int)local_1d4 + local_280);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
          local_28 = *(float *)(unaff_EBX + 0x485dcf) * 1.0 + *pfVar7;
          *local_1d8 = ((uint)(*pfVar2 * local_194.y + *pfVar7) & 0xff) << 8 |
                       ((uint)(*pfVar2 * local_194.z + *pfVar7) & 0xff) << 0x10 |
                       (uint)(*pfVar2 * local_194.x + *pfVar7) & 0xff | (int)local_28 << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
          *local_1d4 = 0x3f800000;
          local_1d4[1] = fVar14;
                    /* Unresolved local var: float * pDst@[???] */
          *local_1e0 = (local_ac - fVar19) - local_594;
          local_1e0[1] = (local_a8 - fVar13) - local_598;
          local_1e0[2] = (local_a4 - fVar12) - local_59c;
          local_1e4 = local_1e4 + 4;
          local_1e8 = local_1e4;
          if (local_1e4 <= iVar5) {
            local_1e8 = iVar5;
          }
          local_1e0 = (float *)((int)local_1e0 + local_298[0]);
          local_1d8 = (uint *)((int)local_1d8 + local_288);
          local_1d4 = (undefined4 *)((int)local_1d4 + local_280);
                    /* Unresolved local var: Vector res@[???] */
          local_b8 = local_124.x - fVar18;
          local_b4 = local_124.y - fVar17;
          local_b0 = local_124.z - fVar20;
                    /* Unresolved local var: Vector vert@[???]
                       Unresolved local var: float[4] pColors@[???] */
          local_16c.x = (local_b8 + fVar19) - local_594;
          local_16c.y = (local_b4 + fVar13) - local_598;
          local_16c.z = (local_b0 + fVar12) - local_59c;
          (*this->m_pMyColor->_vptr_IMaterialVar[0xd])(this->m_pMyColor,&local_194,3);
          local_188 = 0x3f800000;
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
          pfVar7 = (float *)(unaff_EBX + 0x485dcf);
          pfVar1 = (float *)(unaff_EBX + 0x4878c3);
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
          *local_1d8 = ((uint)(*pfVar7 * local_194.y + *pfVar1) & 0xff) << 8 |
                       ((uint)(*pfVar7 * local_194.z + *pfVar1) & 0xff) << 0x10 |
                       (uint)(*pfVar7 * local_194.x + *pfVar1) & 0xff | 0xff000000;
                    /* Unresolved local var: float * pDst@[???] */
          *local_1d4 = 0;
          local_1d4[1] = fVar14;
                    /* Unresolved local var: float * pDst@[???] */
          *local_1e0 = local_16c.x;
          local_1e0[1] = local_16c.y;
          local_1e0[2] = local_16c.z;
          iVar5 = local_1e4 + 1;
          if (local_1e4 + 1 <= local_1e8) {
            iVar5 = local_1e8;
          }
          local_1e0 = (float *)((int)local_1e0 + local_298[0]);
          local_1dc = local_1dc + local_28c * 4;
          local_1d4 = (undefined4 *)((int)local_1d4 + local_280);
          local_1d0 = local_1d0 + local_27c * 4;
          local_1cc = local_1cc + local_278 * 4;
          local_1c8 = local_1c8 + local_274 * 4;
          local_1c4 = local_1c4 + local_270 * 4;
          local_1c0 = local_1c0 + local_26c * 4;
          local_1bc = local_1bc + local_268 * 4;
          local_1b8 = local_1b8 + local_264 * 4;
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
          pfVar2 = (float *)(unaff_EBX + 0x485dcf);
          *(uint *)((int)local_1d8 + local_288) =
               ((uint)(*pfVar7 * local_194.y + *pfVar1) & 0xff) << 8 |
               ((uint)(*pfVar7 * local_194.z + *pfVar1) & 0xff) << 0x10 |
               (uint)(*pfVar7 * local_194.x + *pfVar1) & 0xff |
               (int)(*pfVar2 * 1.0 + *pfVar1) << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
          *local_1d4 = 0;
          local_1d4[1] = 0;
                    /* Unresolved local var: float * pDst@[???] */
          *local_1e0 = fVar19 + local_b8 + local_594;
          local_1e0[1] = fVar13 + local_b4 + local_598;
          local_1e0[2] = fVar12 + local_b0 + local_59c;
          iVar10 = local_1e4 + 2;
          if (local_1e4 + 2 <= iVar5) {
            iVar10 = iVar5;
          }
          local_1e0 = (float *)((int)local_1e0 + local_298[0]);
          local_1d8 = (uint *)((int)local_1d8 + local_288 + local_288);
          local_1d4 = (undefined4 *)((int)local_1d4 + local_280);
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
          pfVar7 = (float *)(unaff_EBX + 0x4878c3);
          *local_1d8 = ((uint)(*pfVar2 * local_194.y + *pfVar1) & 0xff) << 8 |
                       ((uint)(*pfVar2 * local_194.z + *pfVar1) & 0xff) << 0x10 |
                       (uint)(*pfVar2 * local_194.x + *pfVar1) & 0xff |
                       (int)(*pfVar2 * 1.0 + *pfVar7) << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
          *local_1d4 = 0x3f800000;
          local_1d4[1] = 0;
                    /* Unresolved local var: float * pDst@[???] */
          *local_1e0 = (local_b8 - fVar19) + local_594;
          local_1e0[1] = (local_b4 - fVar13) + local_598;
          local_1e0[2] = (local_b0 - fVar12) + local_59c;
          local_1e8 = local_1e4 + 3;
          if (local_1e4 + 3 <= iVar10) {
            local_1e8 = iVar10;
          }
          local_1e0 = (float *)((int)local_1e0 + local_298[0]);
          local_1d8 = (uint *)((int)local_1d8 + local_288);
          local_1d4 = (undefined4 *)((int)local_1d4 + local_280);
          local_16c.x = (local_b8 - fVar19) - local_594;
          local_16c.y = (local_b4 - fVar13) - local_598;
          local_16c.z = (local_b0 - fVar12) - local_59c;
                    /* Unresolved local var: int col@[???]
                       Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
                    /* Unresolved local var: float dc@[???] */
          local_28 = *(float *)(unaff_EBX + 0x485dcf) * 1.0 + *pfVar7;
          *local_1d8 = ((uint)(*pfVar2 * local_194.y + *pfVar7) & 0xff) << 8 |
                       ((uint)(*pfVar2 * local_194.z + *pfVar7) & 0xff) << 0x10 |
                       (uint)(*pfVar2 * local_194.x + *pfVar7) & 0xff | (int)local_28 << 0x18;
                    /* Unresolved local var: float * pDst@[???] */
          *local_1d4 = 0x3f800000;
          local_1d4[1] = fVar14;
                    /* Unresolved local var: float * pDst@[???] */
          *local_1e0 = local_16c.x;
          local_1e0[1] = local_16c.y;
          local_1e0[2] = local_16c.z;
          local_1e4 = local_1e4 + 4;
          if (local_1e8 < local_1e4) {
            local_1e8 = local_1e4;
          }
          local_1e0 = (float *)((int)local_1e0 + local_298[0]);
          local_1d8 = (uint *)((int)local_1d8 + local_288);
          local_1d4 = (undefined4 *)((int)local_1d4 + local_280);
          if (local_2d0 != '\0') {
            if (local_2d4 - 4 < 7) {
                    /* WARNING: Could not recover jumptable at 0x00534b6f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
              iVar5 = (*(code *)(*(int *)(unaff_EBX + 0x3d43 + (local_2d4 - 4) * 4) + unaff_EBX))();
              return iVar5;
            }
            if (local_2c0 != 0) {
              iVar5 = local_2b8 - local_2ac;
              if (local_1e8 <= local_2b8 - local_2ac) {
                iVar5 = local_1e8;
              }
              if (iVar5 != 0) {
                if (local_2d4 < 0xb) {
                    /* WARNING: Could not recover jumptable at 0x005354d5. Too many branches */
                    /* WARNING: Treating indirect jump as call */
                  iVar5 = (*(code *)(*(int *)(&UNK_000046a7 + local_2d4 * 4 + unaff_EBX) + unaff_EBX
                                    ))();
                  return iVar5;
                }
                GenerateSequentialIndexBuffer((ushort *)(local_2ac * 2 + local_2cc),iVar5,local_2b0)
                ;
                local_2ac = iVar5 * local_2c0 + local_2ac;
                if (local_2b4 < local_2ac) {
                  local_2b4 = local_2ac;
                }
              }
            }
          }
          (**(code **)(*local_2d8 + 0x50))(local_2d8,local_1e8,local_2b4,local_38c);
          local_2bc = (int *)0x0;
          local_2b8 = 0;
          local_1ec = 0;
          local_1f4 = (int *)0x0;
          local_24c = 0xffffffff;
          local_2d8 = (int *)0x0;
          forward = (Vector *)0xffffffff;
          (**(code **)(*piVar8 + 0x2c))(piVar8,0xffffffff,0);
          if (bVar11) {
                    /* Unresolved local var: Color color@[???]
                       Unresolved local var: Vector vecMins@[???]
                       Unresolved local var: Vector vecMaxs@[???] */
            local_4c.x = 2.3418409e-38;
            if ((local_5a8 & 1) == 0) {
              local_24 = 0xffff00;
              local_4c.x = 2.3509528e-38;
            }
            puVar3 = (uint *)(unaff_EBX + 0x48a96f);
            local_170 = (float)((uint)local_574[0].z ^ *puVar3);
            local_174 = (float)((uint)local_574[0].y ^ *puVar3);
            local_178 = (float)((uint)local_574[0].x ^ *puVar3);
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
            pfVar7 = (float *)(&DAT_00485a9f + unaff_EBX);
            local_c4 = local_584 * local_178 * *pfVar7;
            local_c0 = local_584 * local_174 * *pfVar7;
            local_bc = local_584 * local_170 * *pfVar7;
            fVar21 = this->m_flWidth;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
            local_f4 = fVar21 * local_1a4.x * *pfVar7;
            local_f0 = fVar21 * local_1a4.y * *pfVar7;
            local_ec = fVar21 * local_1a4.z * *pfVar7;
                    /* Unresolved local var: Vector res@[???] */
            pfVar1 = (float *)(unaff_EBX + 0x485afb);
            local_100 = *pfVar1 * local_184.x;
            local_fc = *pfVar1 * local_184.y;
            local_f8 = *pfVar1 * local_184.z;
            local_16c.x = (local_c4 - local_f4) - local_100;
            local_16c.y = (local_c0 - local_f0) - local_fc;
            local_16c.z = (local_bc - local_ec) - local_f8;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
            local_e8 = local_584 * local_574[0].x * *pfVar7;
            local_e4 = local_584 * local_574[0].y * *pfVar7;
            local_e0 = local_584 * local_574[0].z * *pfVar7;
                    /* Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
                    /* Unresolved local var: Vector res@[???] */
            local_194.x = local_e8 + local_f4 + local_100;
            local_194.y = local_e4 + local_f0 + local_fc;
            local_194.z = local_e0 + local_ec + local_f8;
            local_dc = local_100;
            local_d8 = local_fc;
            local_d4 = local_f8;
            local_d0 = local_f4;
            local_cc = local_f0;
            local_c8 = local_ec;
            NDebugOverlay::Box(&local_124,&local_16c,&local_194,(uint)local_4c.x & 0xff,
                               (uint)local_4c.x >> 8 & 0xff,(uint)local_4c.x >> 0x10,0x40,0.1);
            forward = (Vector *)0x40800000;
            NDebugOverlay::Sphere(&local_124,4.0,0xff,0,0,true,0.1);
          }
          local_5ac = local_5ac + local_584;
          fVar21 = this->m_flSegmentLength;
          fVar16 = (local_5ac + fVar21) - this->m_flCurDisplayLength;
          if (0.0 < fVar16) {
            fVar21 = fVar21 - fVar16;
          }
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???]
                       Unresolved local var: vec_t oob@[???] */
          pfVar7 = (float *)(&DAT_00485a9f + unaff_EBX);
          local_594 = fVar21 * local_574[0].x * *pfVar7;
          local_598 = fVar21 * local_574[0].y * *pfVar7;
          local_59c = fVar21 * local_574[0].z * *pfVar7;
          local_110 = *pfVar7 * local_584 + *pfVar7 * fVar21;
                    /* Unresolved local var: Vector res@[???] */
          local_118 = local_110 * local_574[0].x;
          local_114 = local_110 * local_574[0].y;
          local_110 = local_110 * local_574[0].z;
          local_124.x = local_124.x + local_118;
          local_124.y = local_124.y + local_114;
          local_124.z = local_124.z + local_110;
          local_5a8 = local_5a8 + 1;
          local_584 = fVar21;
          local_10c = local_594;
          local_108 = local_598;
          local_104 = local_59c;
        } while ((int)local_5a8 < this->m_nNumSegments);
      }
      if ((local_1f4 != (int *)0x0) &&
         (cVar4 = (**(code **)(*local_1f4 + 8))(local_1f4), cVar4 != '\0')) {
        (**(code **)(*local_1f4 + 0x10))(local_1f4);
      }
      if ((local_2bc != (int *)0x0) &&
         (cVar4 = (**(code **)(*local_2bc + 8))(local_2bc), cVar4 != '\0')) {
        (**(code **)(*local_2bc + 0x10))(local_2bc);
      }
      piVar8 = local_20;
      if (local_20 != (int *)0x0) {
        (**(code **)(*local_20 + 0xc))(local_20);
        piVar8 = local_20;
      }
    }
    if (piVar8 == (int *)0x0) {
      return 1;
    }
    (**(code **)(*piVar8 + 4))(piVar8);
    iVar10 = 1;
  }
  return iVar10;
}


/* C_ProjectedWallEntity::ColorWallByPortal at 0052f510 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void ColorWallByPortal(C_ProjectedWallEntity * this, IMaterial * pBody)
    */

void __thiscall
C_ProjectedWallEntity::ColorWallByPortal(C_ProjectedWallEntity *this,IMaterial *pBody)

{
  uint uVar1;
  int *piVar2;
  int iVar3;
  Color *pCVar4;
  int unaff_EBX;
  int in_stack_ffffffcc;
  float local_28;
  float local_24;
  float local_20;
  
                    /* Unresolved local var: float[3] flColors@[???] */
  ___i686_get_pc_thunk_bx();
  local_28 = 1.0;
  local_24 = 1.0;
  local_20 = 1.0;
  uVar1 = (this->m_hColorPortal).super_CBaseHandle.m_Index;
                    /* Unresolved local var: C_Prop_Portal * pPortal@[???]
                       Unresolved local var: CBaseEntityList * g_pEntityList@[???]
                       Unresolved local var: CEntInfo * pInfo@[???] */
  if ((((uVar1 != 0xffffffff) &&
       (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x60186c),
       *(uint *)(iVar3 + 8) == uVar1 >> 0x10)) &&
      (piVar2 = *(int **)(iVar3 + 4), piVar2 != (int *)0x0)) &&
     (((uVar1 = piVar2[0xcf4], uVar1 != 0xffffffff &&
       (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x60186c),
       *(uint *)(iVar3 + 8) == uVar1 >> 0x10)) && (*(int *)(iVar3 + 4) != 0)))) {
                    /* Unresolved local var: Color clrPortal@[???] */
    iVar3 = (**(code **)(*piVar2 + 0x1e0))(piVar2);
    pCVar4 = UTIL_Portal_Color((Color *)(2 - (uint)((char)piVar2[0x3f7] == '\0')),iVar3,
                               in_stack_ffffffcc);
    local_20 = *(float *)(unaff_EBX + 0x4876dc);
    local_28 = (float)((uint)pCVar4 & 0xff) / local_20;
    local_24 = (float)((uint)pCVar4 >> 8 & 0xff) / local_20;
    local_20 = (float)((uint)pCVar4 >> 0x10 & 0xff) / local_20;
  }
  (*this->m_pMyColor->_vptr_IMaterialVar[9])(this->m_pMyColor,&local_28,3);
  return;
}


/* C_ProjectedWallEntity::InitMaterials at 0052df20 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: bool InitMaterials(C_ProjectedWallEntity * this) */

bool __thiscall C_ProjectedWallEntity::InitMaterials(C_ProjectedWallEntity *this)

{
  IMaterialVar *pIVar1;
  IMaterial *pIVar2;
  int unaff_EBX;
  char local_d;
  
                    /* Unresolved local var: bool bFound@[???] */
  ___i686_get_pc_thunk_bx();
  if (this->m_pBodyMaterial == (IMaterial *)0x0) {
    pIVar2 = (IMaterial *)
             (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x602f97) + 300))
                       ((int *)**(undefined4 **)(unaff_EBX + 0x602f97),unaff_EBX + 0x4620e8,0,0,0);
    this->m_pBodyMaterial = pIVar2;
    if (pIVar2 == (IMaterial *)0x0) {
      return false;
    }
  }
  if (this->m_pSideRailMaterial == (IMaterial *)0x0) {
    pIVar2 = (IMaterial *)
             (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x602f97) + 300))
                       ((int *)**(undefined4 **)(unaff_EBX + 0x602f97),unaff_EBX + 0x462183,0,0,0);
    this->m_pSideRailMaterial = pIVar2;
    if (pIVar2 == (IMaterial *)0x0) {
      return false;
    }
  }
  local_d = '\0';
  if (this->m_pMyColor != (IMaterialVar *)0x0) {
    return true;
  }
  pIVar1 = (IMaterialVar *)
           (*this->m_pBodyMaterial->_vptr_IMaterial[0xb])
                     (this->m_pBodyMaterial,unaff_EBX + 0x45dbd3,&local_d,1);
  this->m_pMyColor = pIVar1;
  return local_d != '\0';
}


/* C_ProjectedWallEntity::GetRenderBounds at 0052ee80 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void GetRenderBounds(C_ProjectedWallEntity * this, Vector * vecMins,
   Vector * vecMaxs) */

void __thiscall
C_ProjectedWallEntity::GetRenderBounds(C_ProjectedWallEntity *this,Vector *vecMins,Vector *vecMaxs)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float *pfVar5;
  
  pfVar5 = (float *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                      super_C_BaseProjectedEntity.super_C_BaseEntity.super_IClientEntity.
                      super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0x36])(this);
                    /* Unresolved local var: Vector res@[???] */
  fVar1 = (this->m_vWorldSpace_WallMins).y;
  fVar2 = pfVar5[1];
  fVar3 = (this->m_vWorldSpace_WallMins).z;
  fVar4 = pfVar5[2];
  vecMins->x = (this->m_vWorldSpace_WallMins).x - *pfVar5;
  vecMins->y = fVar1 - fVar2;
  vecMins->z = fVar3 - fVar4;
  pfVar5 = (float *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                      super_C_BaseProjectedEntity.super_C_BaseEntity.super_IClientEntity.
                      super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0x36])(this);
                    /* Unresolved local var: Vector res@[???] */
  fVar1 = (this->m_vWorldSpace_WallMaxs).y;
  fVar2 = pfVar5[1];
  fVar3 = (this->m_vWorldSpace_WallMaxs).z;
  fVar4 = pfVar5[2];
  vecMaxs->x = (this->m_vWorldSpace_WallMaxs).x - *pfVar5;
  vecMaxs->y = fVar1 - fVar2;
  vecMaxs->z = fVar3 - fVar4;
  return;
}


/* C_ProjectedWallEntity::GetRenderAngles at 0052e050 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: QAngle * GetRenderAngles(C_ProjectedWallEntity * this) */

QAngle * __thiscall C_ProjectedWallEntity::GetRenderAngles(C_ProjectedWallEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(QAngle **)(extraout_ECX + 0x602cb8);
}


/* C_ProjectedWallEntity::ComputeTranslucencyType at 0052e060 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: RenderableTranslucencyType_t
   ComputeTranslucencyType(C_ProjectedWallEntity * this) */

RenderableTranslucencyType_t __thiscall
C_ProjectedWallEntity::ComputeTranslucencyType(C_ProjectedWallEntity *this)

{
  return RENDERABLE_IS_TRANSLUCENT;
}


/* C_ProjectedWallEntity::OnPreDataChanged at 0052e120 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void OnPreDataChanged(C_ProjectedWallEntity * this, DataUpdateType_t
   type) */

void __thiscall
C_ProjectedWallEntity::OnPreDataChanged(C_ProjectedWallEntity *this,DataUpdateType_t type)

{
  C_BaseEntity::OnPreDataChanged((C_BaseEntity *)this,type);
  this->m_flPrevParticleUpdateTime = this->m_flParticleUpdateTime;
  return;
}


/* C_ProjectedWallEntity::OnDataChanged at 0052f380 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void OnDataChanged(C_ProjectedWallEntity * this, DataUpdateType_t
   updateType) */

void __thiscall
C_ProjectedWallEntity::OnDataChanged(C_ProjectedWallEntity *this,DataUpdateType_t updateType)

{
  NetworkVar_m_Collision *this_00;
  
  C_BaseEntity::OnDataChanged((C_BaseEntity *)this,updateType);
  if (updateType == DATA_UPDATE_CREATED) {
    (*(this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
      super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
      _vptr_IHandleEntity[0xf6])(this);
    SetupWallParticles(this);
  }
  if (this->m_flParticleUpdateTime != this->m_flPrevParticleUpdateTime) {
    this->m_flPrevParticleUpdateTime = this->m_flParticleUpdateTime;
    SetupWallParticles(this);
  }
  this_00 = &(this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
             super_C_BaseEntity.m_Collision;
  CCollisionProperty::MarkSurroundingBoundsDirty(&this_00->super_CCollisionProperty);
  CCollisionProperty::MarkPartitionHandleDirty(&this_00->super_CCollisionProperty);
  return;
}


/* C_ProjectedWallEntity::OnProjected at 00530c20 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void OnProjected(C_ProjectedWallEntity * this) */

void __thiscall C_ProjectedWallEntity::OnProjected(C_ProjectedWallEntity *this)

{
  C_BaseProjectedEntity::OnProjected((C_BaseProjectedEntity *)this);
  ProjectWall(this);
  SetupWallParticles(this);
  return;
}


/* C_ProjectedWallEntity::Spawn at 0052eda0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void Spawn(C_ProjectedWallEntity * this) */

void __thiscall C_ProjectedWallEntity::Spawn(C_ProjectedWallEntity *this)

{
  C_BaseEntity::ThinkSet((C_BaseEntity *)this,(BASEPTR)0x151,0.0,(char *)0x0);
  C_BaseEntity::Spawn((C_BaseEntity *)this);
  return;
}


/* C_ProjectedWallEntity::ClientThink at 0052f430 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void ClientThink(C_ProjectedWallEntity * this) */

void __thiscall C_ProjectedWallEntity::ClientThink(C_ProjectedWallEntity *this)

{
  float fVar1;
  int unaff_EBX;
  float fVar2;
  float fVar3;
  
                    /* Unresolved local var: float flDisplaySpeed@[???] */
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::ClientThink((C_BaseEntity *)this);
  fVar1 = *(float *)(*(int *)(unaff_EBX + 0x8207bf) + 0x2c);
  fVar2 = this->m_flLength;
  fVar3 = fVar2;
  if ((fVar1 <= fVar2) && (fVar3 = 0.0, 0.0 <= fVar1)) {
    fVar3 = fVar1;
  }
  fVar3 = fVar3 + this->m_flCurDisplayLength;
  this->m_flCurDisplayLength = fVar3;
  if ((fVar3 <= fVar2) && (fVar2 = 0.0, 0.0 <= fVar3)) {
    fVar2 = fVar3;
  }
  this->m_flCurDisplayLength = fVar2;
  (*(this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
    super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0xe8])(this);
  if (this->m_flParticleUpdateTime != this->m_flPrevParticleUpdateTime) {
    this->m_flPrevParticleUpdateTime = this->m_flParticleUpdateTime;
    SetupWallParticles(this);
  }
  (*(this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
    super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x9b])
            (this,*(float *)(&DAT_0048c34f + unaff_EBX) +
                  *(float *)(**(int **)(unaff_EBX + 0x601913) + 0xc));
  return;
}


/* C_ProjectedWallEntity::ProjectWall at 0052f760 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ProjectWall(C_ProjectedWallEntity * this) */

void __thiscall C_ProjectedWallEntity::ProjectWall(C_ProjectedWallEntity *this)

{
  uint *puVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  vec_t vVar9;
  vec_t vVar10;
  vec_t vVar11;
  vec_t vVar12;
  vec_t vVar13;
  vec_t vVar14;
  vec_t vVar15;
  vec_t vVar16;
  vec_t vVar17;
  uint uVar18;
  int iVar19;
  bool bVar20;
  int iVar21;
  char cVar22;
  bool bVar23;
  float *pfVar24;
  QAngle *pQVar25;
  int iVar26;
  C_BasePlayer *this_00;
  C_ProjectedWallEntity *pCVar27;
  CPhysCollide *pCVar28;
  vec_t *pvVar29;
  CPolyhedron *pCVar30;
  int unaff_EBX;
  NetworkVar_m_Collision *this_01;
  WallCollideableAtTime_t *pWVar31;
  int iVar32;
  longdouble lVar33;
  float fVar34;
  Vector *local_288;
  Vector *local_284;
  uint local_278;
  uint local_274;
  int local_270;
  float *local_208;
  int local_204;
  float local_1fc;
  float local_1f8;
  float local_1f4;
  float local_1f0;
  float local_1ec;
  float local_1e8;
  float local_1e4;
  float local_1e0;
  float local_1dc;
  float local_1d8;
  float local_1d4;
  float local_1d0;
  float local_1cc;
  float local_1c8;
  float local_1c4;
  float local_1c0;
  float local_1bc;
  float local_1b8;
  float local_1b4;
  float local_1b0;
  float local_1ac;
  float local_1a8;
  float local_1a4;
  float local_1a0;
  float local_19c [5];
  float *local_188;
  float *local_184;
  float *local_180;
  float *local_17c;
  float local_178;
  float local_174;
  float local_170;
  float local_16c;
  float local_168;
  float local_164;
  float local_160;
  float local_15c;
  float local_158;
  float local_154;
  float local_150;
  float local_14c;
  float local_148;
  float local_144;
  float local_140;
  float local_13c;
  float local_138;
  float local_134;
  float local_130;
  float local_12c;
  float local_128;
  Vector local_124;
  float local_118;
  float local_114;
  float local_110;
  Vector local_10c;
  float local_100;
  float local_fc;
  float local_f8;
  float local_f4;
  float local_f0;
  float local_ec;
  float local_e8;
  float local_e4;
  float local_e0;
  float local_dc;
  float local_d8;
  float local_d4;
  float local_d0;
  float local_cc;
  float local_c8;
  float local_c4;
  float local_c0;
  float local_bc;
  float local_b8;
  float local_b4;
  float local_b0;
  float local_ac;
  float local_a8;
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
  float local_5c;
  float local_58;
  float local_54;
  float local_50;
  float local_4c;
  float local_48;
  float local_44;
  Vector local_40;
  Vector local_34;
  Vector local_28 [2];
  
                    /* Unresolved local var: Vector vWallStartPoint@[???]
                       Unresolved local var: Vector vWallEndPoint@[???]
                       Unresolved local var: bool bCreate@[???]
                       Unresolved local var: int iReplaceIndex@[???]
                       Unresolved local var: int iValidIndex@[???]
                       Unresolved local var: Vector vecForward@[???]
                       Unresolved local var: Vector vecRight@[???]
                       Unresolved local var: Vector vecUp@[???] */
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::AddEffects((C_BaseEntity *)this,8);
  pfVar24 = (float *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                       super_C_BaseProjectedEntity.super_C_BaseEntity.super_IClientEntity.
                       super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0xe4])(this);
  fVar2 = pfVar24[2];
  fVar3 = pfVar24[1];
  fVar4 = *pfVar24;
  pfVar24 = (float *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                       super_C_BaseProjectedEntity.super_C_BaseEntity.super_IClientEntity.
                       super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0xe5])(this);
  fVar5 = pfVar24[2];
  fVar6 = pfVar24[1];
  fVar7 = *pfVar24;
  cVar22 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x60168b) + 0x3c))
                     ((int *)**(undefined4 **)(unaff_EBX + 0x60168b));
  if (cVar22 == '\0') {
    iVar26 = (this->m_WallCollideables).m_Size;
    iVar32 = iVar26;
    iVar21 = iVar26 * 0x4c;
    do {
      iVar19 = iVar21;
      iVar32 = iVar32 + -1;
      if (iVar32 < 1) goto LAB_0053017e;
      pWVar31 = (this->m_WallCollideables).m_Memory.m_pMemory;
      iVar21 = iVar19 + -0x4c;
    } while (*(float *)(**(int **)(unaff_EBX + 0x6015df) + 0xc) <
             *(float *)((int)pWVar31->flTime + iVar19 + -0x4c));
    local_270 = 0;
    local_204 = 0;
    while( true ) {
                    /* Unresolved local var: int j@[???] */
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x6016f7) + 0x44))
                ((int *)**(undefined4 **)(unaff_EBX + 0x6016f7),
                 *(undefined4 *)((int)pWVar31->flTime + local_204 + 8));
      local_270 = local_270 + 1;
      local_204 = local_204 + 0x4c;
      if (local_270 == iVar32) break;
      pWVar31 = (this->m_WallCollideables).m_Memory.m_pMemory;
    }
    iVar26 = (this->m_WallCollideables).m_Size;
    if (0 < iVar26 - iVar32) {
      pWVar31 = (this->m_WallCollideables).m_Memory.m_pMemory;
      _V_memmove(pWVar31,(void *)((int)pWVar31[-1].flTime + iVar19 + -0x3c),(iVar26 - iVar32) * 0x4c
                );
      iVar26 = (this->m_WallCollideables).m_Size;
    }
    iVar26 = iVar26 - iVar32;
    (this->m_WallCollideables).m_Size = iVar26;
LAB_0053017e:
    if (iVar26 != 0) {
      pQVar25 = C_BaseEntity::GetNetworkAngles((C_BaseEntity *)this);
      pWVar31 = (this->m_WallCollideables).m_Memory.m_pMemory;
      if ((pQVar25->x == (pWVar31->qAngles).x) && (!NAN(pQVar25->x) && !NAN((pWVar31->qAngles).x)))
      {
        pvVar29 = &(pWVar31->qAngles).y;
        if ((pQVar25->y == *pvVar29) && (!NAN(pQVar25->y) && !NAN(*pvVar29))) {
          pvVar29 = &(pWVar31->qAngles).z;
          if ((pQVar25->z == *pvVar29) && (!NAN(pQVar25->z) && !NAN(*pvVar29))) {
            pfVar24 = (float *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                                 super_C_BaseProjectedEntity.super_C_BaseEntity.super_IClientEntity.
                                 super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0xe4])
                                         (this);
            pWVar31 = (this->m_WallCollideables).m_Memory.m_pMemory;
            if ((*pfVar24 == (pWVar31->vStart).x) && (!NAN(*pfVar24) && !NAN((pWVar31->vStart).x)))
            {
              pvVar29 = &(pWVar31->vStart).y;
              if ((pfVar24[1] == *pvVar29) && (!NAN(pfVar24[1]) && !NAN(*pvVar29))) {
                pvVar29 = &(pWVar31->vStart).z;
                if ((pfVar24[2] == *pvVar29) && (!NAN(pfVar24[2]) && !NAN(*pvVar29))) {
                  pfVar24 = (float *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                                       super_C_BaseProjectedEntity.super_C_BaseEntity.
                                       super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
                                       _vptr_IHandleEntity[0xe5])(this);
                  pWVar31 = (this->m_WallCollideables).m_Memory.m_pMemory;
                  if (*pfVar24 == (pWVar31->vEnd).x) {
                    pvVar29 = &(pWVar31->vEnd).y;
                    if ((pfVar24[1] == *pvVar29) && (!NAN(pfVar24[1]) && !NAN(*pvVar29))) {
                      pvVar29 = &(pWVar31->vEnd).z;
                      if ((pfVar24[2] == *pvVar29) && (!NAN(pfVar24[2]) && !NAN(*pvVar29))) {
                        fVar8 = *(float *)(**(int **)(unaff_EBX + 0x6015df) + 0xc);
                        pWVar31->flTime[1] = fVar8;
                        pWVar31->flTime[0] = fVar8;
                        ((this->m_WallCollideables).m_Memory.m_pMemory)->bIsPredicted = false;
                        bVar20 = false;
                        local_278 = 0xffffffff;
                        local_274 = 0;
                        goto LAB_0052f8ed;
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x6016f7) + 0x44))
                ((int *)**(undefined4 **)(unaff_EBX + 0x6016f7),pWVar31->pCollideable);
      bVar20 = true;
      local_278 = 0;
      local_274 = 0xffffffff;
      goto LAB_0052f8ed;
    }
LAB_0052f8d0:
    bVar20 = true;
    local_278 = 0xffffffff;
    local_274 = 0xffffffff;
  }
  else {
    local_274 = (this->m_WallCollideables).m_Size;
    iVar26 = local_274 * 0x4c;
    do {
      iVar32 = iVar26;
      local_278 = local_274;
      local_274 = local_278 - 1;
      if ((int)local_274 < 0) goto LAB_0052f8d0;
      iVar26 = iVar32 + -0x4c;
    } while (*(float *)(**(int **)(unaff_EBX + 0x6015df) + 0xc) <
             *(float *)((int)((this->m_WallCollideables).m_Memory.m_pMemory)->flTime +
                       iVar32 + -0x48));
    pQVar25 = C_BaseEntity::GetNetworkAngles((C_BaseEntity *)this);
    local_208 = (float *)((int)(this->m_WallCollideables).m_Memory.m_pMemory[-1].flTime +
                         iVar32 + -0x3c);
    if ((pQVar25->x == local_208[0xc]) && (!NAN(pQVar25->x) && !NAN(local_208[0xc]))) {
      if ((pQVar25->y == local_208[0xd]) && (!NAN(pQVar25->y) && !NAN(local_208[0xd]))) {
        if ((pQVar25->z == local_208[0xe]) && (!NAN(pQVar25->z) && !NAN(local_208[0xe]))) {
          pfVar24 = (float *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                               super_C_BaseProjectedEntity.super_C_BaseEntity.super_IClientEntity.
                               super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0xe4])
                                       (this);
          local_208 = (float *)((int)(this->m_WallCollideables).m_Memory.m_pMemory[-1].flTime +
                               iVar32 + -0x3c);
          if ((*pfVar24 == *local_208) && (!NAN(*pfVar24) && !NAN(*local_208))) {
            if ((pfVar24[1] == local_208[1]) && (!NAN(pfVar24[1]) && !NAN(local_208[1]))) {
              if ((pfVar24[2] == local_208[2]) && (!NAN(pfVar24[2]) && !NAN(local_208[2]))) {
                pfVar24 = (float *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                                     super_C_BaseProjectedEntity.super_C_BaseEntity.
                                     super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
                                     _vptr_IHandleEntity[0xe5])(this);
                local_208 = (float *)((int)(this->m_WallCollideables).m_Memory.m_pMemory[-1].flTime
                                     + iVar32 + -0x3c);
                if (*pfVar24 == local_208[3]) {
                  if ((pfVar24[1] == local_208[4]) && (!NAN(pfVar24[1]) && !NAN(local_208[4]))) {
                    if ((pfVar24[2] == local_208[5]) && (!NAN(pfVar24[2]) && !NAN(local_208[5]))) {
                      bVar20 = false;
                      local_278 = 0xffffffff;
                      goto LAB_0052f8ed;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    fVar8 = *(float *)(**(int **)(unaff_EBX + 0x6015df) + 0xc);
    if (fVar8 < local_208[0x10] || fVar8 == local_208[0x10]) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x6016f7) + 0x44))
                ((int *)**(undefined4 **)(unaff_EBX + 0x6016f7),local_208[0x11]);
      *(undefined4 *)((int)(this->m_WallCollideables).m_Memory.m_pMemory[-1].flTime + iVar32 + 8) =
           0;
      local_278 = local_274;
    }
    else {
      if ((this->m_WallCollideables).m_Size <= (int)local_278) goto LAB_0052f8d0;
      CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t,_int>_>
      ::GrowVector(&this->m_WallCollideables,1);
      iVar26 = ~local_278 + (this->m_WallCollideables).m_Size;
      if (0 < iVar26) {
        pWVar31 = (this->m_WallCollideables).m_Memory.m_pMemory;
        _V_memmove(pWVar31 + local_278 + 1,pWVar31 + local_278,iVar26 * 0x4c);
      }
    }
    bVar20 = true;
    local_274 = 0xffffffff;
  }
LAB_0052f8ed:
  for (iVar26 = FirstValidSplitScreenSlot(); iVar26 != -1; iVar26 = NextValidSplitScreenSlot(iVar26)
      ) {
    this_00 = C_BasePlayer::GetLocalPlayer(iVar26);
    if ((this_00 != (C_BasePlayer *)0x0) &&
       (pCVar27 = (C_ProjectedWallEntity *)C_BaseEntity::GetGroundEntity((C_BaseEntity *)this_00),
       this == pCVar27)) {
      C_BaseEntity::SetGroundEntity((C_BaseEntity *)this_00,(C_BaseEntity *)0x0);
    }
  }
  pQVar25 = C_BaseEntity::GetNetworkAngles((C_BaseEntity *)this);
  AngleVectors(pQVar25,local_28,&local_34,&local_40);
  if (bVar20) {
    if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x601e03) + 0x1c) + 0x30) == 0) {
      puVar1 = (uint *)(unaff_EBX + 0x48c02f);
      local_1f4 = (float)((uint)local_28[0].z ^ *puVar1);
      local_1f8 = (float)((uint)local_28[0].y ^ *puVar1);
      local_1fc = (float)((uint)local_28[0].x ^ *puVar1);
      local_1f0 = fVar4 * local_1fc + fVar3 * local_1f8 + fVar2 * local_1f4;
      local_1ec = local_28[0].x;
      local_1e8 = local_28[0].y;
      local_1e4 = local_28[0].z;
      local_1e0 = fVar7 * local_28[0].x + fVar6 * local_28[0].y + fVar5 * local_28[0].z;
      local_a4 = *(float *)(&DAT_0048715f + unaff_EBX) * this->m_flWidth;
                    /* Unresolved local var: Vector res@[???] */
      local_ac = local_a4 * local_34.x;
      local_a8 = local_a4 * local_34.y;
      local_a4 = local_a4 * local_34.z;
                    /* Unresolved local var: Vector res@[???] */
      local_a0 = local_ac + fVar4;
      local_9c = fVar3 + local_a8;
      local_98 = fVar2 + local_a4;
      local_1dc = local_34.x;
      local_1d8 = local_34.y;
      local_1d4 = local_34.z;
      local_1d0 = local_a0 * local_34.x + local_9c * local_34.y + local_98 * local_34.z;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
      local_b8 = fVar4 - local_ac;
      local_b4 = fVar3 - local_a8;
      local_b0 = fVar2 - local_a4;
      local_1c4 = (float)((uint)local_34.z ^ *puVar1);
      local_1c8 = (float)((uint)local_34.y ^ *puVar1);
      local_1cc = (float)((uint)local_34.x ^ *puVar1);
      local_1c0 = local_b8 * local_1cc + local_b4 * local_1c8 + local_b0 * local_1c4;
      local_e0 = *(float *)(&DAT_0048715f + unaff_EBX) * this->m_flHeight;
                    /* Unresolved local var: Vector res@[???] */
      local_e8 = local_e0 * local_40.x;
      local_e4 = local_e0 * local_40.y;
      local_e0 = local_e0 * local_40.z;
                    /* Unresolved local var: Vector res@[???] */
      local_dc = local_e8 + fVar4;
      local_d8 = fVar3 + local_e4;
      local_d4 = fVar2 + local_e0;
      local_1bc = local_40.x;
      local_1b8 = local_40.y;
      local_1b4 = local_40.z;
      local_1b0 = local_dc * local_40.x + local_d8 * local_40.y + local_d4 * local_40.z;
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
      local_f4 = fVar4 - local_e8;
      local_f0 = fVar3 - local_e4;
      local_ec = fVar2 - local_e0;
      local_1a4 = (float)((uint)local_40.z ^ *puVar1);
      local_1a8 = (float)((uint)local_40.y ^ *puVar1);
      local_1ac = (float)((uint)local_40.x ^ *puVar1);
      local_1a0 = local_f4 * local_1ac + local_f0 * local_1a8 + local_ec * local_1a4;
      local_178 = local_1ac;
      local_174 = local_1a8;
      local_170 = local_1a4;
      local_16c = local_1ac;
      local_168 = local_1a8;
      local_164 = local_1a4;
      local_160 = local_1cc;
      local_15c = local_1c8;
      local_158 = local_1c4;
      local_154 = local_1cc;
      local_150 = local_1c8;
      local_14c = local_1c4;
      local_148 = local_1fc;
      local_144 = local_1f8;
      local_140 = local_1f4;
      local_13c = local_1fc;
      local_138 = local_1f8;
      local_134 = local_1f4;
      local_100 = local_1ac;
      local_fc = local_1a8;
      local_f8 = local_1a4;
      local_d0 = local_e8;
      local_cc = local_e4;
      local_c8 = local_e0;
      local_c4 = local_1cc;
      local_c0 = local_1c8;
      local_bc = local_1c4;
      local_94 = local_ac;
      local_90 = local_a8;
      local_8c = local_a4;
      local_88 = local_1fc;
      local_84 = local_1f8;
      local_80 = local_1f4;
      pCVar30 = GeneratePolyhedronFromPlanes(&local_1fc,6,1.1920929e-07,false);
      if (pCVar30 == (CPolyhedron *)0x0) {
        _Warning(unaff_EBX + 0x460c43);
        return;
      }
      local_19c[0] = (float)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x6016f7) + 0x24))
                                      ((int *)**(undefined4 **)(unaff_EBX + 0x6016f7),pCVar30);
      (*pCVar30->_vptr_CPolyhedron[2])(pCVar30);
      if (local_19c[0] == 0.0) {
        return;
      }
      pCVar28 = (CPhysCollide *)
                (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x6016f7) + 0x3c))
                          ((int *)**(undefined4 **)(unaff_EBX + 0x6016f7),local_19c,1);
    }
    else {
                    /* Unresolved local var: Vector vScaledRight@[???]
                       Unresolved local var: Vector vStart@[???]
                       Unresolved local var: Vector vEnd@[???]
                       Unresolved local var: Vector[4] vVerts@[???]
                       Unresolved local var: Vector *[4] pVerts@[???]
                       Unresolved local var: CPhysConvex * pTempConvex@[???] */
      local_44 = *(float *)(&DAT_0048715f + unaff_EBX) * this->m_flWidth;
                    /* Unresolved local var: Vector res@[???] */
      local_4c = local_44 * local_34.x;
      local_48 = local_44 * local_34.y;
      local_44 = local_44 * local_34.z;
                    /* Unresolved local var: Vector res@[???] */
      local_1fc = fVar4 + local_4c;
      local_1f8 = fVar3 + local_48;
      local_1f4 = fVar2 + local_44;
                    /* Unresolved local var: Vector res@[???] */
      local_1f0 = fVar4 - local_4c;
      local_1ec = fVar3 - local_48;
      local_1e8 = fVar2 - local_44;
                    /* Unresolved local var: Vector res@[???] */
      local_1e4 = fVar7 - local_4c;
      local_1e0 = fVar6 - local_48;
      local_1dc = fVar5 - local_44;
                    /* Unresolved local var: Vector res@[???] */
      local_1d8 = local_4c + fVar7;
      local_1d4 = fVar6 + local_48;
      local_1d0 = local_44 + fVar5;
      local_188 = &local_1fc;
      local_184 = &local_1f0;
      local_180 = &local_1e4;
      local_17c = &local_1d8;
      local_7c = local_1d8;
      local_78 = local_1d4;
      local_74 = local_1d0;
      local_70 = local_1e4;
      local_6c = local_1e0;
      local_68 = local_1dc;
      local_64 = local_1f0;
      local_60 = local_1ec;
      local_5c = local_1e8;
      local_58 = local_1fc;
      local_54 = local_1f8;
      local_50 = local_1f4;
      local_19c[0] = (float)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x6016f7) + 8))
                                      ((int *)**(undefined4 **)(unaff_EBX + 0x6016f7),&local_188,4);
      pCVar28 = (CPhysCollide *)
                (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x6016f7) + 0x3c))
                          ((int *)**(undefined4 **)(unaff_EBX + 0x6016f7),local_19c,1);
    }
    if (pCVar28 == (CPhysCollide *)0x0) {
      return;
    }
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x6016f7) + 0x60))
              ((int *)**(undefined4 **)(unaff_EBX + 0x6016f7),&local_124,&local_10c,pCVar28,
               *(undefined4 *)(unaff_EBX + 0x601643),*(undefined4 *)(unaff_EBX + 0x60159f));
    pvVar29 = (vec_t *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                         super_C_BaseProjectedEntity.super_C_BaseEntity.super_IClientEntity.
                         super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0xe4])(this);
    vVar9 = *pvVar29;
    vVar10 = pvVar29[1];
    vVar11 = pvVar29[2];
    pvVar29 = (vec_t *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                         super_C_BaseProjectedEntity.super_C_BaseEntity.super_IClientEntity.
                         super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0xe5])(this);
    vVar12 = *pvVar29;
    vVar13 = pvVar29[1];
    vVar14 = pvVar29[2];
    pQVar25 = C_BaseEntity::GetNetworkAngles((C_BaseEntity *)this);
    vVar15 = pQVar25->x;
    vVar16 = pQVar25->y;
    vVar17 = pQVar25->z;
    bVar23 = (bool)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x60168b) + 0x3c))
                             ((int *)**(undefined4 **)(unaff_EBX + 0x60168b));
    iVar26 = **(int **)(unaff_EBX + 0x6015df);
    fVar34 = (float)*(int *)(iVar26 + 0x18) * *(float *)(iVar26 + 0x1c);
    fVar8 = *(float *)(iVar26 + 0xc);
    if ((int)local_278 < 0) {
      uVar18 = (this->m_WallCollideables).m_Size;
      CUtlVector<C_ProjectedWallEntity::WallCollideableAtTime_t,CUtlMemory<C_ProjectedWallEntity::WallCollideableAtTime_t,_int>_>
      ::GrowVector(&this->m_WallCollideables,1);
      iVar26 = ~uVar18 + (this->m_WallCollideables).m_Size;
      if (0 < iVar26) {
        pWVar31 = (this->m_WallCollideables).m_Memory.m_pMemory;
        _V_memmove(pWVar31 + uVar18 + 1,pWVar31 + uVar18,iVar26 * 0x4c);
      }
      iVar26 = uVar18 * 0x4c;
      pWVar31 = (this->m_WallCollideables).m_Memory.m_pMemory + uVar18;
      if (pWVar31 != (WallCollideableAtTime_t *)0x0) {
        pWVar31->pCollideable = pCVar28;
        pWVar31->flTime[1] = fVar8;
        pWVar31->flTime[0] = fVar34;
        pWVar31->bIsPredicted = bVar23;
        (pWVar31->qAngles).z = vVar17;
        (pWVar31->qAngles).y = vVar16;
        (pWVar31->qAngles).x = vVar15;
        (pWVar31->vWorldMaxs).z = local_10c.z;
        (pWVar31->vWorldMaxs).y = local_10c.y;
        (pWVar31->vWorldMaxs).x = local_10c.x;
        (pWVar31->vWorldMins).z = local_124.z;
        (pWVar31->vWorldMins).y = local_124.y;
        (pWVar31->vWorldMins).x = local_124.x;
        (pWVar31->vEnd).z = vVar14;
        (pWVar31->vEnd).y = vVar13;
        (pWVar31->vEnd).x = vVar12;
        (pWVar31->vStart).z = vVar11;
        (pWVar31->vStart).y = vVar10;
        (pWVar31->vStart).x = vVar9;
      }
    }
    else {
      iVar26 = local_278 * 0x4c;
      pWVar31 = (this->m_WallCollideables).m_Memory.m_pMemory + local_278;
      (pWVar31->vStart).x = vVar9;
      (pWVar31->vStart).y = vVar10;
      (pWVar31->vStart).z = vVar11;
      (pWVar31->vEnd).x = vVar12;
      (pWVar31->vEnd).y = vVar13;
      (pWVar31->vEnd).z = vVar14;
      (pWVar31->vWorldMins).x = local_124.x;
      (pWVar31->vWorldMins).y = local_124.y;
      (pWVar31->vWorldMins).z = local_124.z;
      (pWVar31->vWorldMaxs).x = local_10c.x;
      (pWVar31->vWorldMaxs).y = local_10c.y;
      (pWVar31->vWorldMaxs).z = local_10c.z;
      (pWVar31->qAngles).x = vVar15;
      (pWVar31->qAngles).y = vVar16;
      (pWVar31->qAngles).z = vVar17;
      pWVar31->flTime[1] = fVar8;
      pWVar31->flTime[0] = fVar34;
      pWVar31->pCollideable = pCVar28;
      pWVar31->bIsPredicted = bVar23;
    }
  }
  else {
    iVar26 = local_274 * 0x4c;
  }
                    /* Unresolved local var: Vector vSetMins@[???]
                       Unresolved local var: Vector vSetMaxs@[???] */
  local_284 = &local_124;
  local_288 = &local_10c;
  pWVar31 = (this->m_WallCollideables).m_Memory.m_pMemory;
  local_118 = *(float *)((int)pWVar31->flTime + iVar26 + -0x24);
  (this->m_vWorldSpace_WallMins).x = local_118;
  local_114 = *(float *)((int)pWVar31->flTime + iVar26 + -0x20);
  (this->m_vWorldSpace_WallMins).y = local_114;
  local_110 = *(float *)((int)pWVar31->flTime + iVar26 + -0x1c);
  (this->m_vWorldSpace_WallMins).z = local_110;
  local_130 = *(float *)((int)pWVar31->flTime + iVar26 + -0x18);
  (this->m_vWorldSpace_WallMaxs).x = local_130;
  local_12c = *(float *)((int)pWVar31->flTime + iVar26 + -0x14);
  (this->m_vWorldSpace_WallMaxs).y = local_12c;
  local_128 = *(float *)((int)pWVar31->flTime + iVar26 + -0x10);
  (this->m_vWorldSpace_WallMaxs).z = local_128;
                    /* Unresolved local var: Vector res@[???] */
  local_118 = local_118 - fVar4;
  local_114 = local_114 - fVar3;
  local_110 = local_110 - fVar2;
                    /* Unresolved local var: Vector res@[???] */
  local_130 = local_130 - fVar4;
  local_12c = local_12c - fVar3;
  local_128 = local_128 - fVar2;
  local_124.x = local_130;
  local_124.y = local_12c;
  local_124.z = local_128;
  local_10c.x = local_118;
  local_10c.y = local_114;
  local_10c.z = local_110;
  C_BaseEntity::SetSize((C_BaseEntity *)this,local_288,local_284);
  if (bVar20) {
    this->m_flCurDisplayLength = 0.0;
    (*(this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
      super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
      _vptr_IHandleEntity[0x9b])(this,0xc4a1a000);
  }
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: __m128 root@[???] */
  local_19c[0] = SQRT((fVar4 - fVar7) * (fVar4 - fVar7) + (fVar3 - fVar6) * (fVar3 - fVar6) +
                      (fVar2 - fVar5) * (fVar2 - fVar5));
  local_19c[1] = 0.0;
  local_19c[2] = 0.0;
  local_19c[3] = 0.0;
  this->m_flLength = local_19c[0];
  this_01 = &(this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
             super_C_BaseEntity.m_Collision;
  CCollisionProperty::MarkSurroundingBoundsDirty(&this_01->super_CCollisionProperty);
  CCollisionProperty::MarkPartitionHandleDirty(&this_01->super_CCollisionProperty);
  cVar22 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x60168b) + 0x3c))
                     ((int *)**(undefined4 **)(unaff_EBX + 0x60168b));
  if (cVar22 != '\0') {
    DisplaceObstructingEntities(this);
  }
  lVar33 = (longdouble)_ceilf(this->m_flLength / this->m_flSegmentLength);
  this->m_nNumSegments = (int)lVar33;
  cVar22 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x60168b) + 0x3c))
                     ((int *)**(undefined4 **)(unaff_EBX + 0x60168b));
  if (cVar22 == '\0') {
    CUtlVector<PaintPowerType,CUtlMemory<PaintPowerType,_int>_>::SetCount
              (&this->m_PaintPowers,this->m_nNumSegments);
    (*(this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
      super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
      _vptr_IHandleEntity[0xf3])(this);
  }
  this->m_fLastProjectedTime = *(float *)(**(int **)(unaff_EBX + 0x6015df) + 0xc);
  return;
}


/* C_ProjectedWallEntity::GetProjectionExtents at 0052ed70 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void GetProjectionExtents(C_ProjectedWallEntity * this, Vector *
   outMins, Vector * outMaxs) */

void __thiscall
C_ProjectedWallEntity::GetProjectionExtents
          (C_ProjectedWallEntity *this,Vector *outMins,Vector *outMaxs)

{
  GetExtents(this,outMins,outMaxs,0.5);
  return;
}


/* __tcf_5 at 009519c0 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fe3d8),in_stack_00000008);
  return;
}


/* C_ProjectedWallEntity::ComputeWorldSpaceSurroundingBox at 0052ef40 */

/* WARNING: Restarted to delay deadcode elimination for space: stack */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void ComputeWorldSpaceSurroundingBox(C_ProjectedWallEntity * this,
   Vector * pWorldMins, Vector * pWorldMaxs) */

void __thiscall
C_ProjectedWallEntity::ComputeWorldSpaceSurroundingBox
          (C_ProjectedWallEntity *this,Vector *pWorldMins,Vector *pWorldMaxs)

{
  float fVar1;
  WallCollideableAtTime_t *pWVar2;
  int iVar3;
  char cVar4;
  int iVar5;
  int unaff_EBX;
  int iVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  float fVar10;
  float fVar11;
  float fVar12;
  
  ___i686_get_pc_thunk_bx();
  if (*(int *)(*(int *)(unaff_EBX + 0x820e6e) + 0x30) == 0) {
    if ((this->m_WallCollideables).m_Size == 0) {
      pWorldMins->x = (this->m_vWorldSpace_WallMins).x;
      pWorldMins->y = (this->m_vWorldSpace_WallMins).y;
      pWorldMins->z = (this->m_vWorldSpace_WallMins).z;
      pWorldMaxs->x = (this->m_vWorldSpace_WallMaxs).x;
      pWorldMaxs->y = (this->m_vWorldSpace_WallMaxs).y;
      pWorldMaxs->z = (this->m_vWorldSpace_WallMaxs).z;
      return;
    }
    cVar4 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x601eae) + 0x3c))
                      ((int *)**(undefined4 **)(unaff_EBX + 0x601eae));
    if (cVar4 == '\0') {
LAB_0052f161:
      pWVar2 = (this->m_WallCollideables).m_Memory.m_pMemory;
      pWorldMins->x = (pWVar2->vWorldMins).x;
      pWorldMins->y = (pWVar2->vWorldMins).y;
      pWorldMins->z = (pWVar2->vWorldMins).z;
      pWVar2 = (this->m_WallCollideables).m_Memory.m_pMemory;
      pWorldMaxs->x = (pWVar2->vWorldMaxs).x;
      pWorldMaxs->y = (pWVar2->vWorldMaxs).y;
      pWorldMaxs->z = (pWVar2->vWorldMaxs).z;
    }
    else {
                    /* Unresolved local var: int i@[???] */
      iVar5 = (this->m_WallCollideables).m_Size;
      iVar6 = iVar5 * 0x4c;
      do {
        iVar3 = iVar6;
        iVar5 = iVar5 + -1;
        if (iVar5 < 0) goto LAB_0052f161;
        pWVar2 = (this->m_WallCollideables).m_Memory.m_pMemory;
        iVar6 = iVar3 + -0x4c;
      } while (this->m_fLastProjectedTime < *(float *)((int)pWVar2[-1].flTime + iVar3 + 4));
      pWorldMins->x = *(vec_t *)((int)pWVar2[-1].flTime + iVar3 + -0x24);
      pWorldMins->y = *(vec_t *)((int)pWVar2[-1].flTime + iVar3 + -0x20);
      pWorldMins->z = *(vec_t *)((int)pWVar2[-1].flTime + iVar3 + -0x1c);
      pWVar2 = (this->m_WallCollideables).m_Memory.m_pMemory;
      pWorldMaxs->x = *(vec_t *)((int)pWVar2[-1].flTime + iVar3 + -0x18);
      pWorldMaxs->y = *(vec_t *)((int)pWVar2[-1].flTime + iVar3 + -0x14);
      pWorldMaxs->z = *(vec_t *)((int)pWVar2[-1].flTime + iVar3 + -0x10);
    }
  }
  else {
    fVar12 = (this->m_vWorldSpace_WallMins).x;
    fVar7 = (this->m_vWorldSpace_WallMins).y;
    fVar8 = (this->m_vWorldSpace_WallMins).z;
    fVar9 = (this->m_vWorldSpace_WallMaxs).x;
    fVar11 = (this->m_vWorldSpace_WallMaxs).y;
    fVar10 = (this->m_vWorldSpace_WallMaxs).z;
                    /* Unresolved local var: Vector vMins@[???]
                       Unresolved local var: Vector vMaxs@[???]
                       Unresolved local var: int i@[???] */
    if ((this->m_WallCollideables).m_Size != 0) {
      iVar6 = 0;
      iVar5 = 0;
      do {
        pWVar2 = (this->m_WallCollideables).m_Memory.m_pMemory;
        fVar1 = *(float *)((int)pWVar2->flTime + iVar5 + -0x1c);
        if (fVar1 <= fVar8) {
          fVar8 = fVar1;
        }
        fVar1 = *(float *)((int)pWVar2->flTime + iVar5 + -0x20);
        if (fVar1 <= fVar7) {
          fVar7 = fVar1;
        }
        fVar1 = *(float *)((int)pWVar2->flTime + iVar5 + -0x24);
        if (fVar1 <= fVar12) {
          fVar12 = fVar1;
        }
        fVar1 = *(float *)((int)pWVar2->flTime + iVar5 + -0x10);
        if (fVar10 <= fVar1) {
          fVar10 = fVar1;
        }
        fVar1 = *(float *)((int)pWVar2->flTime + iVar5 + -0x14);
        if (fVar11 <= fVar1) {
          fVar11 = fVar1;
        }
        fVar1 = *(float *)((int)pWVar2->flTime + iVar5 + -0x18);
        if (fVar9 <= fVar1) {
          fVar9 = fVar1;
        }
        iVar6 = iVar6 + 1;
        iVar5 = iVar5 + 0x4c;
      } while ((this->m_WallCollideables).m_Size != iVar6);
    }
    pWorldMins->x = fVar12;
    pWorldMins->y = fVar7;
    pWorldMins->z = fVar8;
    pWorldMaxs->x = fVar9;
    pWorldMaxs->y = fVar11;
    pWorldMaxs->z = fVar10;
  }
  return;
}


/* C_ProjectedWallEntity::TestCollision at 0052f630 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: bool TestCollision(C_ProjectedWallEntity * this, Ray_t * ray, uint
   mask, trace_t * trace) */

bool __thiscall
C_ProjectedWallEntity::TestCollision
          (C_ProjectedWallEntity *this,Ray_t *ray,uint mask,trace_t *trace)

{
  float *pfVar1;
  WallCollideableAtTime_t *pWVar2;
  int iVar3;
  int iVar4;
  undefined1 uVar5;
  char cVar6;
  CPhysCollide *pCVar7;
  int iVar8;
  int *piVar9;
  int unaff_EBX;
  undefined4 uVar10;
  undefined4 uVar11;
  
  ___i686_get_pc_thunk_bx();
  uVar5 = 0;
  if ((this->m_WallCollideables).m_Size != 0) {
    cVar6 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x6017b8) + 0x3c))
                      ((int *)**(undefined4 **)(unaff_EBX + 0x6017b8));
    if (cVar6 == '\0') {
LAB_0052f720:
      piVar9 = (int *)**(undefined4 **)(unaff_EBX + 0x601824);
      iVar8 = *piVar9;
      uVar11 = *(undefined4 *)(unaff_EBX + 0x6016cc);
      uVar10 = *(undefined4 *)(unaff_EBX + 0x601770);
      pCVar7 = ((this->m_WallCollideables).m_Memory.m_pMemory)->pCollideable;
    }
    else {
                    /* Unresolved local var: int i@[???] */
      iVar8 = (this->m_WallCollideables).m_Size;
      iVar4 = iVar8 * 0x4c;
      do {
        iVar3 = iVar4;
        iVar8 = iVar8 + -1;
        if (iVar8 < 0) goto LAB_0052f720;
        pWVar2 = (this->m_WallCollideables).m_Memory.m_pMemory;
        iVar4 = iVar3 + -0x4c;
      } while (this->m_fLastProjectedTime < *(float *)((int)pWVar2[-1].flTime + iVar3 + 4));
      piVar9 = (int *)**(undefined4 **)(unaff_EBX + 0x601824);
      iVar8 = *piVar9;
      uVar11 = *(undefined4 *)(unaff_EBX + 0x6016cc);
      uVar10 = *(undefined4 *)(unaff_EBX + 0x601770);
      pCVar7 = *(CPhysCollide **)((int)pWVar2[-1].flTime + iVar3 + 8);
    }
    (**(code **)(iVar8 + 0x88))(piVar9,ray,mask,0,pCVar7,uVar10,uVar11,trace);
    pfVar1 = &(trace->super_CBaseTrace).fraction;
    if ((*pfVar1 <= *(float *)(unaff_EBX + 0x487200) && *(float *)(unaff_EBX + 0x487200) != *pfVar1)
       || ((trace->super_CBaseTrace).allsolid != false)) {
      uVar5 = 1;
    }
    else {
      uVar5 = (trace->super_CBaseTrace).startsolid;
    }
  }
  return (bool)uVar5;
}


/* C_ProjectedWallEntity::TestHitboxes at 0052e070 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: bool TestHitboxes(C_ProjectedWallEntity * this, Ray_t * ray, uint
   fContentsMask, trace_t * tr) */

bool __thiscall
C_ProjectedWallEntity::TestHitboxes
          (C_ProjectedWallEntity *this,Ray_t *ray,uint fContentsMask,trace_t *tr)

{
  int iVar1;
  
  iVar1 = (*(this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
            super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
            _vptr_IHandleEntity[0x44])(this,ray,fContentsMask,tr);
  return (bool)(char)iVar1;
}


/* C_ProjectedWallEntity::SetWallColor at 0052e150 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void SetWallColor(C_ProjectedWallEntity * this, PaintPowerType * power)
    */

void __thiscall
C_ProjectedWallEntity::SetWallColor(C_ProjectedWallEntity *this,PaintPowerType *power)

{
  Color *pCVar1;
  int unaff_EBX;
  int in_stack_ffffffd8;
  float local_18;
  float local_14;
  float local_10;
  
                    /* Unresolved local var: float[3] flColors@[???] */
  ___i686_get_pc_thunk_bx();
  local_18 = 1.0;
  local_14 = 1.0;
  local_10 = 1.0;
  if ((Color *)*power !=
      (Color *)&vgui::CvarToggleCheckButton<CGameUIConVarRef>::GetKBMap::s_pMap.cputype) {
                    /* Unresolved local var: Color color@[???] */
    pCVar1 = MapPowerToVisualColor((Color *)*power,in_stack_ffffffd8);
    local_10 = *(float *)(unaff_EBX + 0x488aa4);
    local_18 = (float)((uint)pCVar1 & 0xff) / local_10;
    local_14 = (float)((uint)pCVar1 >> 8 & 0xff) / local_10;
    local_10 = (float)((uint)pCVar1 >> 0x10 & 0xff) / local_10;
  }
  (*this->m_pMyColor->_vptr_IMaterialVar[9])(this->m_pMyColor,&local_18,3);
  return;
}


/* C_ProjectedWallEntity::SetPaintPower at 0052e0a0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void SetPaintPower(C_ProjectedWallEntity * this, int nSegment,
   PaintPowerType power) */

void __thiscall
C_ProjectedWallEntity::SetPaintPower(C_ProjectedWallEntity *this,int nSegment,PaintPowerType power)

{
  if (nSegment < this->m_nNumSegments) {
    (this->m_PaintPowers).m_Memory.m_pMemory[nSegment] = power;
  }
  return;
}


/* C_TEWallPaintedEvent::PostDataUpdate at 00535d30 */

/* DWARF original prototype: void PostDataUpdate(C_TEWallPaintedEvent * this, DataUpdateType_t
   updateType) */

void __thiscall
C_TEWallPaintedEvent::PostDataUpdate(C_TEWallPaintedEvent *this,DataUpdateType_t updateType)

{
  uint uVar1;
  int iVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: C_ProjectedWallEntity * pWall@[???] */
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->m_hEntity).m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5fb04f),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) && (iVar2 = *(int *)(iVar2 + 4), iVar2 != 0)) {
    iVar2 = ___dynamic_cast(iVar2,unaff_EBX + 0x5fc9ff,unaff_EBX + 0x662c7f,0);
    if (iVar2 != 0) {
      if (this->m_nSegment < *(int *)(iVar2 + 0xb14)) {
        *(int *)(*(int *)(iVar2 + 0xb1c) + this->m_nSegment * 4) = this->m_colorIndex;
        return;
      }
    }
  }
  return;
}


/* C_ProjectedWallEntity::GetToolRecordingState at 0052f1a0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void GetToolRecordingState(C_ProjectedWallEntity * this, KeyValues *
   msg) */

void __thiscall
C_ProjectedWallEntity::GetToolRecordingState(C_ProjectedWallEntity *this,KeyValues *msg)

{
  float fVar1;
  float fVar2;
  KeyValues *this_00;
  char *keyName;
  float *pfVar3;
  QAngle *pQVar4;
  int unaff_EBX;
  
                    /* Unresolved local var: KeyValues * pKV@[???]
                       Unresolved local var: Vector vStart@[???]
                       Unresolved local var: QAngle qAngles@[???] */
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::GetToolRecordingState((C_BaseEntity *)this,msg);
  this_00 = CIFM_EntityKeyValuesHandler_AutoRegister::FindOrCreateNonConformantKeyValues(msg);
  keyName = CIFM_EntityKeyValuesHandler_AutoRegister::GetHandlerIDKeyString();
  KeyValues::SetString(this_00,keyName,&UNK_00461074 + unaff_EBX);
  KeyValues::SetInt(this_00,(char *)(unaff_EBX + 0x45dad6),
                    (this->super_CPaintableEntity<C_BaseProjectedEntity>).
                    super_C_BaseProjectedEntity.super_C_BaseEntity.index);
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x450906),this->m_flWidth);
  KeyValues::SetFloat(this_00,&UNK_00460f7a + unaff_EBX,this->m_flHeight);
  KeyValues::SetInt(this_00,(char *)(unaff_EBX + 0x460f81),this->m_nNumSegments);
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x460f8d),this->m_flSegmentLength);
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x460f97),this->m_flCurDisplayLength);
  pfVar3 = (float *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                      super_C_BaseProjectedEntity.super_C_BaseEntity.super_IClientEntity.
                      super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0xe4])(this);
  fVar1 = pfVar3[2];
  fVar2 = pfVar3[1];
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x4615e2),*pfVar3);
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x460f72),fVar2);
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x4615e8),fVar1);
  pQVar4 = C_BaseEntity::GetNetworkAngles((C_BaseEntity *)this);
  fVar1 = pQVar4->x;
  fVar2 = pQVar4->z;
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x460fa2),pQVar4->y);
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x460fa7),fVar1);
  KeyValues::SetFloat(this_00,(char *)(unaff_EBX + 0x460fac),fVar2);
  return;
}


/* C_ProjectedWallEntity::RestoreToToolRecordedState at 0052eaf0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: void RestoreToToolRecordedState(C_ProjectedWallEntity * this, KeyValues
   * pKV) */

void __thiscall
C_ProjectedWallEntity::RestoreToToolRecordedState(C_ProjectedWallEntity *this,KeyValues *pKV)

{
  int iVar1;
  matrix3x4_t *matrix;
  int unaff_EBX;
  float fVar2;
  QAngle local_58;
  Vector local_4c;
  float local_40;
  float local_3c;
  float local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  
                    /* Unresolved local var: QAngle qKVAngles@[???]
                       Unresolved local var: matrix3x4_t * pMatrix@[???] */
  ___i686_get_pc_thunk_bx();
  fVar2 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x461c92),0.0);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.m_vecStartPoint.
  x = fVar2;
  fVar2 = KeyValues::GetFloat(pKV,(char *)(CHudElementHelper::CHudElementHelper + unaff_EBX + 2),0.0
                             );
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.m_vecStartPoint.
  y = fVar2;
  fVar2 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x461c98),0.0);
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.m_vecStartPoint.
  z = fVar2;
  fVar2 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x450fb6),0.0);
  this->m_flWidth = fVar2;
  fVar2 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x46162a),0.0);
  this->m_flHeight = fVar2;
  iVar1 = KeyValues::GetInt(pKV,(char *)(unaff_EBX + 0x461631),0);
  this->m_nNumSegments = iVar1;
  fVar2 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x46163d),0.0);
  this->m_flSegmentLength = fVar2;
  fVar2 = KeyValues::GetFloat(pKV,&UNK_00461647 + unaff_EBX,0.0);
  this->m_flCurDisplayLength = fVar2;
  local_28 = 0xc6800000;
  local_24 = 0xc6800000;
  local_20 = 0xc6800000;
  (this->m_vWorldSpace_WallMins).x = -16384.0;
  (this->m_vWorldSpace_WallMins).y = -16384.0;
  (this->m_vWorldSpace_WallMins).z = -16384.0;
  local_34 = 0x46800000;
  local_30 = 0x46800000;
  local_2c = 0x46800000;
  (this->m_vWorldSpace_WallMaxs).x = 16384.0;
  (this->m_vWorldSpace_WallMaxs).y = 16384.0;
  (this->m_vWorldSpace_WallMaxs).z = 16384.0;
  C_BaseEntity::SetSize
            ((C_BaseEntity *)this,&this->m_vWorldSpace_WallMins,&this->m_vWorldSpace_WallMaxs);
  local_3c = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x461652),0.0);
  local_40 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x461657),0.0);
  local_38 = KeyValues::GetFloat(pKV,(char *)(unaff_EBX + 0x46165c),0.0);
  matrix = KeyValues::GetPtr(pKV,(char *)(unaff_EBX + 0x461661),(void *)0x0);
  if (matrix != (matrix3x4_t *)0x0) {
                    /* Unresolved local var: Vector vPos@[???]
                       Unresolved local var: QAngle qAngles@[???] */
    MatrixAngles(matrix,&local_58.x);
    local_4c.x = matrix->m_flMatVal[0][3];
    local_4c.y = matrix->m_flMatVal[1][3];
    local_4c.z = matrix->m_flMatVal[2][3];
    local_58.x = local_40;
    local_58.y = local_3c;
    local_58.z = local_38;
    C_BaseEntity::SetNetworkOrigin((C_BaseEntity *)this,&local_4c);
    C_BaseEntity::SetAbsOrigin((C_BaseEntity *)this,&local_4c);
    C_BaseEntity::SetLocalOrigin((C_BaseEntity *)this,&local_4c);
    C_BaseEntity::SetNetworkAngles((C_BaseEntity *)this,&local_58);
    C_BaseEntity::SetAbsAngles((C_BaseEntity *)this,&local_58);
    C_BaseEntity::SetLocalAngles((C_BaseEntity *)this,&local_58);
  }
  (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
  super_C_BaseEntity.m_VisibilityBits.super_CBitVecT<CFixedBitVecBase<2>_>.super_CFixedBitVecBase<2>
  .m_Ints[0] = 0xffffffff;
  return;
}


/* C_ProjectedWallEntity::ShouldSpawnParticles at 0052e1f0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "CHudElement": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: bool ShouldSpawnParticles(C_ProjectedWallEntity * this, C_Portal_Base2D
   * pPortal) */

bool __thiscall
C_ProjectedWallEntity::ShouldSpawnParticles(C_ProjectedWallEntity *this,C_Portal_Base2D *pPortal)

{
  bool bVar1;
  int iVar2;
  QAngle *pQVar3;
  int unaff_EBX;
  float fVar4;
  double dVar5;
  float fVar6;
  Vector local_24;
  Vector local_18;
  
                    /* Unresolved local var: C_Prop_Portal * pPropPortal@[???]
                       Unresolved local var: Vector vPortalUp@[???]
                       Unresolved local var: Vector vWallUp@[???]
                       Unresolved local var: float flDot@[???] */
  ___i686_get_pc_thunk_bx();
  bVar1 = C_Portal_Base2D::IsActivedAndLinked(pPortal);
  if (!bVar1) {
    return true;
  }
  if ((pPortal != (C_Portal_Base2D *)0x0) &&
     (iVar2 = ___dynamic_cast(pPortal,*(undefined4 *)(CHudIndicator::Init + unaff_EBX + 3),
                              *(undefined4 *)(unaff_EBX + 0x602e2b),0), iVar2 != 0)) {
    pQVar3 = (QAngle *)
             (*(pPortal->super_CPortalRenderable_FlatBasic).super_C_BaseAnimating.super_C_BaseEntity
               .super_IClientEntity.super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity
               [0xe])(pPortal);
    AngleVectors(pQVar3,(Vector *)0x0,(Vector *)0x0,&local_18);
    pQVar3 = (QAngle *)
             (*(this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
               super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
               _vptr_IHandleEntity[0xe])(this);
    AngleVectors(pQVar3,(Vector *)0x0,(Vector *)0x0,&local_24);
    fVar6 = local_18.x * local_24.x + local_18.y * local_24.y + local_18.z * local_24.z;
    if (*(float *)(unaff_EBX + 0x488647) <= fVar6 && fVar6 != *(float *)(unaff_EBX + 0x488647)) {
      return true;
    }
    fVar4 = *(float *)(&DAT_0048870f + unaff_EBX);
    if (*(float *)(&DAT_0048870f + unaff_EBX) <= fVar6) {
      fVar4 = fVar6;
    }
    dVar5 = (double)(float)((uint)fVar4 & *(uint *)(unaff_EBX + 0x48d593));
    return *(double *)(unaff_EBX + 0x488b63) <= dVar5 && dVar5 != *(double *)(unaff_EBX + 0x488b63);
  }
  return false;
}


/* C_ProjectedWallEntity::SetupWallParticles at 0052e310 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void SetupWallParticles(C_ProjectedWallEntity * this) */

void __thiscall C_ProjectedWallEntity::SetupWallParticles(C_ProjectedWallEntity *this)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  uint uVar7;
  Vector vecOrigin;
  Vector vecOrigin_00;
  Vector vecOrigin_01;
  Vector vecOrigin_02;
  Vector vecOrigin_03;
  Vector vecOrigin_04;
  QAngle vecAngles;
  QAngle vecAngles_00;
  QAngle vecAngles_01;
  QAngle vecAngles_02;
  QAngle vecAngles_03;
  QAngle vecAngles_04;
  bool bVar8;
  int iVar9;
  float *pfVar10;
  QAngle *pQVar11;
  int unaff_EBX;
  char *pcVar12;
  float fVar13;
  C_Portal_Base2D *local_140;
  C_Portal_Base2D *local_120;
  Vector local_58;
  Vector local_4c;
  Vector local_40;
  QAngle local_34;
  float local_28;
  float local_24;
  float local_20;
  
                    /* Unresolved local var: float flImpactPointOffset@[???]
                       Unresolved local var: C_Portal_Base2D * pSourcePortal@[???]
                       Unresolved local var: C_Portal_Base2D * pHitPortal@[???]
                       Unresolved local var: Vector vecParticlePos@[???]
                       Unresolved local var: Vector vWallEndPoint@[???]
                       Unresolved local var: Vector vWallStartPoint@[???]
                       Unresolved local var: QAngle qAngles@[???]
                       Unresolved local var: Vector vecForward@[???]
                       Unresolved local var: Vector vecRight@[???]
                       Unresolved local var: Vector vecUp@[???] */
  ___i686_get_pc_thunk_bx();
  StopParticleEffects((C_BaseEntity *)this);
  uVar7 = (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
          m_hSourcePortal.super_CBaseHandle.m_Index;
  if ((uVar7 == 0xffffffff) ||
     (iVar9 = (uVar7 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x602a6f),
     *(uint *)(iVar9 + 8) != uVar7 >> 0x10)) {
    local_140 = (C_Portal_Base2D *)0x0;
  }
  else {
    local_140 = *(C_Portal_Base2D **)(iVar9 + 4);
  }
  uVar7 = (this->super_CPaintableEntity<C_BaseProjectedEntity>).super_C_BaseProjectedEntity.
          m_hHitPortal.super_CBaseHandle.m_Index;
  if ((uVar7 == 0xffffffff) ||
     (iVar9 = (uVar7 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x602a6f),
     *(uint *)(iVar9 + 8) != uVar7 >> 0x10)) {
    local_120 = (C_Portal_Base2D *)0x0;
  }
  else {
    local_120 = *(C_Portal_Base2D **)(iVar9 + 4);
  }
  pfVar10 = (float *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                       super_C_BaseProjectedEntity.super_C_BaseEntity.super_IClientEntity.
                       super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0xe5])(this);
  fVar1 = pfVar10[2];
  fVar2 = pfVar10[1];
  fVar3 = *pfVar10;
  pfVar10 = (float *)(*(this->super_CPaintableEntity<C_BaseProjectedEntity>).
                       super_C_BaseProjectedEntity.super_C_BaseEntity.super_IClientEntity.
                       super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0xe4])(this);
  fVar4 = pfVar10[2];
  fVar5 = pfVar10[1];
  fVar6 = *pfVar10;
  pQVar11 = C_BaseEntity::GetNetworkAngles((C_BaseEntity *)this);
  local_34.x = pQVar11->x;
  local_34.y = pQVar11->y;
  local_34.z = pQVar11->z;
  AngleVectors(&local_34,&local_40,&local_4c,&local_58);
  if ((local_140 == (C_Portal_Base2D *)0x0) ||
     (bVar8 = ShouldSpawnParticles(this,local_140), !bVar8)) {
    if (local_120 != (C_Portal_Base2D *)0x0) goto LAB_0052e44f;
  }
  else {
                    /* Unresolved local var: C_Prop_Portal * pPortal@[???]
                       Unresolved local var: Vector res@[???] */
    fVar13 = *(float *)(unaff_EBX + 0x4889af);
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
    local_28 = fVar6 + local_40.x + local_40.x + local_4c.x * fVar13;
    local_24 = fVar5 + local_40.y + local_40.y + local_4c.y * fVar13;
    local_20 = fVar4 + local_40.z + local_40.z + local_4c.z * fVar13;
    pcVar12 = (char *)(unaff_EBX + 0x461de8);
    vecOrigin_01.y = local_24;
    vecOrigin_01.x = local_28;
    vecOrigin_01.z = local_20;
    vecAngles_01.y = local_34.y;
    vecAngles_01.x = local_34.x;
    vecAngles_01.z = local_34.z;
    DispatchParticleEffect(pcVar12,vecOrigin_01,vecAngles_01,(C_BaseEntity *)this,-1);
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
    local_28 = (fVar6 + local_40.x + local_40.x) - local_4c.x * fVar13;
    local_24 = (fVar5 + local_40.y + local_40.y) - local_4c.y * fVar13;
    local_20 = (fVar4 + local_40.z + local_40.z) - local_4c.z * fVar13;
    vecOrigin_02.y = local_24;
    vecOrigin_02.x = local_28;
    vecOrigin_02.z = local_20;
    vecAngles_02.y = local_34.y;
    vecAngles_02.x = local_34.x;
    vecAngles_02.z = local_34.z;
    DispatchParticleEffect(pcVar12,vecOrigin_02,vecAngles_02,(C_BaseEntity *)this,-1);
    if (local_120 == (C_Portal_Base2D *)0x0) goto LAB_0052e477;
    iVar9 = ___dynamic_cast(local_120,*(undefined4 *)(unaff_EBX + 0x602cff),
                            *(undefined4 *)(unaff_EBX + 0x602d07),0);
    if (iVar9 != 0) {
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
      local_28 = fVar6 + local_40.x + local_40.x;
      local_24 = fVar5 + local_40.y + local_40.y;
      local_20 = fVar4 + local_40.z + local_40.z;
      vecOrigin_03.y = local_24;
      vecOrigin_03.x = local_28;
      vecOrigin_03.z = local_20;
      vecAngles_03.y = local_34.y;
      vecAngles_03.x = local_34.x;
      vecAngles_03.z = local_34.z;
      DispatchParticleEffect(pcVar12,vecOrigin_03,vecAngles_03,(C_BaseEntity *)this,-1);
    }
LAB_0052e44f:
    bVar8 = ShouldSpawnParticles(this,local_120);
    if (!bVar8) {
      return;
    }
  }
  fVar13 = *(float *)(unaff_EBX + 0x4889af);
LAB_0052e477:
                    /* Unresolved local var: Vector res@[???] */
  pcVar12 = (char *)(unaff_EBX + 0x461de8);
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
  local_28 = (fVar3 - (local_40.x + local_40.x)) + local_4c.x * fVar13;
  local_24 = (fVar2 - (local_40.y + local_40.y)) + local_4c.y * fVar13;
  local_20 = (fVar1 - (local_40.z + local_40.z)) + local_4c.z * fVar13;
  vecOrigin.y = local_24;
  vecOrigin.x = local_28;
  vecOrigin.z = local_20;
  vecAngles.y = local_34.y;
  vecAngles.x = local_34.x;
  vecAngles.z = local_34.z;
  DispatchParticleEffect(pcVar12,vecOrigin,vecAngles,(C_BaseEntity *)this,-1);
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
  local_28 = (fVar3 - (local_40.x + local_40.x)) - local_4c.x * fVar13;
  local_24 = (fVar2 - (local_40.y + local_40.y)) - local_4c.y * fVar13;
  local_20 = (fVar1 - (local_40.z + local_40.z)) - fVar13 * local_4c.z;
  vecOrigin_00.y = local_24;
  vecOrigin_00.x = local_28;
  vecOrigin_00.z = local_20;
  vecAngles_00.y = local_34.y;
  vecAngles_00.x = local_34.x;
  vecAngles_00.z = local_34.z;
  DispatchParticleEffect(pcVar12,vecOrigin_00,vecAngles_00,(C_BaseEntity *)this,-1);
  if (local_120 != (C_Portal_Base2D *)0x0) {
    return;
  }
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
  local_28 = fVar3 - (local_40.x + local_40.x);
  local_24 = fVar2 - (local_40.y + local_40.y);
  local_20 = fVar1 - (local_40.z + local_40.z);
  vecOrigin_04.y = local_24;
  vecOrigin_04.x = local_28;
  vecOrigin_04.z = local_20;
  vecAngles_04.y = local_34.y;
  vecAngles_04.x = local_34.x;
  vecAngles_04.z = local_34.z;
  DispatchParticleEffect(pcVar12,vecOrigin_04,vecAngles_04,(C_BaseEntity *)this,-1);
  return;
}


/* __tcf_6 at 00951a60 */

void __tcf_6(void *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  *(int *)(unaff_EBX + 0x36762c) = *(int *)(unaff_EBX + 0x1df3c8) + 8;
  *(undefined4 *)(unaff_EBX + 0x367644) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x367640)) {
    if (*(int *)(unaff_EBX + 0x367638) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1df2b0) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1df2b0),*(int *)(unaff_EBX + 0x367638));
      *(undefined4 *)(unaff_EBX + 0x367638) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x36763c) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x367638);
  *(int *)(unaff_EBX + 0x367648) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x367640)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1df2b0) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1df2b0),iVar1);
      *(undefined4 *)(unaff_EBX + 0x367638) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x36763c) = 0;
  }
  return;
}


/* CProjectedWallEntity_NonConformantDataHandler::DestroyInstance at 00535860 */

/* DWARF original prototype: void DestroyInstance(CProjectedWallEntity_NonConformantDataHandler *
   this, void * pEntity) */

void __thiscall
CProjectedWallEntity_NonConformantDataHandler::DestroyInstance
          (CProjectedWallEntity_NonConformantDataHandler *this,void *pEntity)

{
  code *UNRECOVERED_JUMPTABLE;
  int unaff_EBX;
  
                    /* Unresolved local var: C_ProjectedWallEntity * pCastEntity@[???] */
  ___i686_get_pc_thunk_bx();
  UNRECOVERED_JUMPTABLE = *(code **)(*(int *)**(undefined4 **)(&DAT_005fb577 + unaff_EBX) + 0x44);
  if (pEntity != (void *)0x0) {
    (*UNRECOVERED_JUMPTABLE)((int *)**(undefined4 **)(&DAT_005fb577 + unaff_EBX),(int)pEntity + 4);
                    /* WARNING: Could not recover jumptable at 0x005358a2. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (**(code **)(*(int *)pEntity + 4))();
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x005358c3. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}


/* CProjectedWallEntity_NonConformantDataHandler::HandleInstance at 00535c20 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void HandleInstance(CProjectedWallEntity_NonConformantDataHandler *
   this, void * pEntity, KeyValues * pKeyValues) */

void __thiscall
CProjectedWallEntity_NonConformantDataHandler::HandleInstance
          (CProjectedWallEntity_NonConformantDataHandler *this,void *pEntity,KeyValues *pKeyValues)

{
  short *psVar1;
  int unaff_EBX;
  undefined4 *puVar2;
  
                    /* Unresolved local var: C_ProjectedWallEntity * pCastEntity@[???] */
  ___i686_get_pc_thunk_bx();
  C_ProjectedWallEntity::RestoreToToolRecordedState(pEntity,pKeyValues);
  psVar1 = (short *)(**(code **)(*(int *)pEntity + 0x35c))(pEntity);
  if (*psVar1 == -1) {
    puVar2 = *(undefined4 **)(unaff_EBX + 0x5fb1b4);
    (**(code **)(*(int *)*puVar2 + 0x40))((int *)*puVar2,(int)pEntity + 4,0,1,0xffffffff);
  }
  else {
    puVar2 = *(undefined4 **)(unaff_EBX + 0x5fb1b4);
  }
                    /* WARNING: Could not recover jumptable at 0x00535c79. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(int *)*puVar2 + 0x4c))();
  return;
}


/* C_TEWallPaintedEvent::~C_TEWallPaintedEvent at 00535ba0 */

/* DWARF original prototype: void ~C_TEWallPaintedEvent(C_TEWallPaintedEvent * this, int __in_chrg)
    */

void __thiscall
C_TEWallPaintedEvent::~C_TEWallPaintedEvent(C_TEWallPaintedEvent *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseTempEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x6627f7);
  (this->super_C_BaseTempEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x66287b);
  C_BaseTempEntity::~C_BaseTempEntity(&this->super_C_BaseTempEntity,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* C_TEWallPaintedEvent::~C_TEWallPaintedEvent at 00535bf0 */

/* DWARF original prototype: void ~C_TEWallPaintedEvent(C_TEWallPaintedEvent * this, int __in_chrg)
    */

void __thiscall
C_TEWallPaintedEvent::~C_TEWallPaintedEvent(C_TEWallPaintedEvent *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_C_BaseTempEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(extraout_ECX + 0x6627b0);
  (this->super_C_BaseTempEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(extraout_ECX + 0x662834);
  C_BaseTempEntity::~C_BaseTempEntity(&this->super_C_BaseTempEntity,__in_chrg);
  return;
}


/* __tcf_7 at 00951b40 */

void __tcf_7(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  *(int *)(extraout_ECX + 0x367578) = extraout_ECX + 0x246860;
  *(int *)(extraout_ECX + 0x36757c) = extraout_ECX + 0x2468e4;
  C_BaseTempEntity::~C_BaseTempEntity
            ((C_BaseTempEntity *)(extraout_ECX + 0x367578),in_stack_00000008);
  return;
}


/* C_TEWallPaintedEvent::YouForgotToImplementOrDeclareClientClass at 0052e0c0 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_TEWallPaintedEvent *
   this) */

int __thiscall
C_TEWallPaintedEvent::YouForgotToImplementOrDeclareClientClass(C_TEWallPaintedEvent *this)

{
  return 0;
}


/* C_TEWallPaintedEvent::GetClientClass at 0052e0e0 */

/* DWARF original prototype: ClientClass * GetClientClass(C_TEWallPaintedEvent * this) */

ClientClass * __thiscall C_TEWallPaintedEvent::GetClientClass(C_TEWallPaintedEvent *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x821d10);
}


/* _C_TEWallPaintedEvent_CreateObject at 0052e0f0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

IClientNetworkable * _C_TEWallPaintedEvent_CreateObject(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (IClientNetworkable *)(extraout_ECX + 0x78afcc);
}


/* ClientClassInit<DT_TEWallPaintedEvent::ignored> at 00077880 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

int ClientClassInit<DT_TEWallPaintedEvent::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x36e].m_pPrev + unaff_EBX)
      == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc4184c);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x370].m_pPrev + unaff_EBX
                  ),(char *)(unaff_EBX + 0x8e84cc),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropEHandle((RecvProp *)
                      ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x374].m_SerialNumber
                      + unaff_EBX),&UNK_0091883d + unaff_EBX,0x10,4,
                      *(RecvVarProxyFn *)(&DAT_00ab9508 + unaff_EBX));
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x378].m_pEntity +
                  unaff_EBX),&UNK_00918847 + unaff_EBX,0x14,4,0,(RecvVarProxyFn)0x0);
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x37b].m_pNext + unaff_EBX
                  ),&UNK_00918854 + unaff_EBX,0x18,4,0,(RecvVarProxyFn)0x0);
      ___cxa_guard_release(unaff_EBX + 0xc4184c);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)&vec4_origin.z + unaff_EBX),
             (RecvProp_conflict1 *)
             ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x374].m_SerialNumber +
             unaff_EBX),3,&UNK_0091885f + unaff_EBX);
  return 1;
}


/* __tcf_8 at 00951980 */

void __tcf_8(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3fe488),in_stack_00000008);
  return;
}


/* _GLOBAL__I_cl_projected_wall_projection_speed at 00078190 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_cl_projected_wall_projection_speed(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

