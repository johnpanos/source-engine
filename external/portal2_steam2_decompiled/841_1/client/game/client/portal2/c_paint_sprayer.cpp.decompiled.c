/* DWARF-guided pseudocode for game/client/portal2/c_paint_sprayer.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* ClientClassInit<DT_PaintSprayer::ignored> at 00075d70 */

int ClientClassInit<DT_PaintSprayer::ignored>(ignored *param_1)

{
  RecvVarProxyFn p_Var1;
  int iVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&CDmxElement::GetValue<CUtlString>(char_const*)::defaultValue + unaff_EBX + 2)
      == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xc5b35a);
    if (iVar2 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&CDmxAttribute::GetArray<float>()::defaultArray + unaff_EBX + 2),
                  (char *)(unaff_EBX + 0x9149f2),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)(unaff_EBX + 0xc5b39e),
                        (char *)(vgui::PropertySheet::ChainToAnimationMap + unaff_EBX + 6),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00aee0b2 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00aee06e + unaff_EBX));
      p_Var1 = *(RecvVarProxyFn *)(&DAT_00aee112 + unaff_EBX);
      RecvPropVector((RecvProp_conflict1 *)((int)&cvar + unaff_EBX + 2),
                     (char *)(unaff_EBX + 0x9341da),0x118,0xc,0,p_Var1);
      RecvPropInt((RecvProp_conflict1 *)((int)&g_pMaterialSystem2 + unaff_EBX + 2),
                  &UNK_0092919e + unaff_EBX,0x128,4,0,*(RecvVarProxyFn *)(&DAT_00aee102 + unaff_EBX)
                 );
      RecvPropVector((RecvProp_conflict1 *)((int)&g_pMatchFramework + unaff_EBX + 2),
                     (char *)(unaff_EBX + 0x93082a),0x10c,0xc,0,p_Var1);
      RecvPropBool((RecvProp *)((int)&g_pDataCache + unaff_EBX + 2),(char *)(unaff_EBX + 0x92f04a),
                   0xa6c,1);
      RecvPropBool((RecvProp *)((int)power2_n + unaff_EBX + 10),&UNK_0094467c + unaff_EBX,0xa6d,1);
      RecvPropInt((RecvProp_conflict1 *)((int)power2_n + unaff_EBX + 0x46),&UNK_00918fae + unaff_EBX
                  ,0xa70,4,0,(RecvVarProxyFn)0x0);
      p_Var1 = *(RecvVarProxyFn *)(&DAT_00aee0ae + unaff_EBX);
      RecvPropFloat((RecvProp_conflict1 *)((int)power2_n + unaff_EBX + 0x82),
                    (char *)(vgui::TextEntry::ChainToMap + unaff_EBX + 6),0xa74,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)((int)power2_n + unaff_EBX + 0xbe),
                    (char *)(unaff_EBX + 0x944699),0xa80,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)((int)power2_n + unaff_EBX + 0xfa),
                    (char *)(unaff_EBX + 0x9446a6),0xa84,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)((int)power2_n + unaff_EBX + 0x136),
                    (char *)(unaff_EBX + 0x9446b3),0xa78,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)((int)power2_n + unaff_EBX + 0x172),
                    (char *)(unaff_EBX + 0x9446c8),0xa7c,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)((int)power2_n + unaff_EBX + 0x1ae),
                    &UNK_009446dc + unaff_EBX,0xa88,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)((int)power2_n + unaff_EBX + 0x1ea),
                    (char *)(unaff_EBX + 0x9446f1),0xa8c,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)((int)power2_n + unaff_EBX + 0x226),
                    (char *)(unaff_EBX + 0x944703),0xa90,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)((int)power2_n + unaff_EBX + 0x262),
                    (char *)(unaff_EBX + 0x944715),0xa94,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)((int)power2_n + unaff_EBX + 0x29e),
                    (char *)(unaff_EBX + 0x94472e),0xa98,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)((int)power2_n + unaff_EBX + 0x2da),
                    (char *)(unaff_EBX + 0x92d45b),0xa9c,4,0,p_Var1);
      RecvPropEHandle((RecvProp *)((int)power2_n + unaff_EBX + 0x316),(char *)(unaff_EBX + 0x944747)
                      ,0xaa0,4,*(RecvVarProxyFn *)(&DAT_00aee06a + unaff_EBX));
      ___cxa_guard_release(unaff_EBX + 0xc5b35a);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x572),
             (RecvProp_conflict1 *)(unaff_EBX + 0xc5b39e),0x13,(char *)(unaff_EBX + 0x944756));
  return 1;
}


/* __static_initialization_and_destruction_0 at 00076260 */

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
  (&DAT_00c5ae2f)[unaff_EBX] = 0;
  *(undefined1 *)((int)&CPolygonButton::GetKBMap()::s_pMap + unaff_EBX) = 0;
  *(undefined1 *)((int)&CPolygonButton::GetKBMap()::s_pMap + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&CPolygonButton::GetKBMap()::s_pMap + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&CPolygonButton::GetKBMap()::s_pMap + unaff_EBX + 3) = 0;
  *(undefined4 *)(&DAT_00c5ae37 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c5ae3b + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c5ae3f + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c5ae43 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c5ae47 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c5ae4b + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c5ae4f + unaff_EBX) = 0;
  *(undefined4 *)((int)&CPolygonButton::GetKBMap::s_pMap + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&CPolygonButton::GetMessageMap::s_pMap + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&CPolygonButton::KB_ChainToMap()::chained + unaff_EBX + 1) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c5ae5f + unaff_EBX) = 0x7f7fffff;
  *(undefined **)(&DAT_00c5ae63 + unaff_EBX) = &UNK_00aeec17 + unaff_EBX;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x6f) = unaff_EBX + 0x944273;
  *(undefined **)((int)s_pParticlePtrs + unaff_EBX + 0x67) = &UNK_004b0b2f + unaff_EBX;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0x6b) = 0;
  this = (RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x7f);
  *(RecvTable **)((int)s_pParticlePtrs + unaff_EBX + 0x73) = this;
  piVar1 = *(int **)(&DAT_00aedb73 + unaff_EBX);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x77) = *piVar1;
  *piVar1 = unaff_EBX + 0xcf2c27;
  RecvTable::RecvTable(this);
  ___cxa_atexit(unaff_EBX + 0x905adf,0,*(undefined4 *)(&DAT_00aedafb + unaff_EBX));
  iVar2 = ClientClassInit<DT_PaintSprayer::ignored>((ignored *)0x0);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x93) = iVar2;
  return;
}


/* C_PaintSprayer::YouForgotToImplementOrDeclareClientClass at 00526900 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PaintSprayer": ignoring overlapping field "m_bOldActive" */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_PaintSprayer * this) */

int __thiscall C_PaintSprayer::YouForgotToImplementOrDeclareClientClass(C_PaintSprayer *this)

{
  return 0;
}


/* C_PaintSprayer::GetClientClass at 00526920 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PaintSprayer": ignoring overlapping field "m_bOldActive" */
/* DWARF original prototype: ClientClass * GetClientClass(C_PaintSprayer * this) */

ClientClass * __thiscall C_PaintSprayer::GetClientClass(C_PaintSprayer *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x842570);
}


/* _C_PaintSprayer_CreateObject at 00526da0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_PaintSprayer_CreateObject(int entnum,int serialNum)

{
  C_BaseEntity *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_PaintSprayer * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xaa4);
  C_BaseEntity::C_BaseEntity(this);
  (this->super_IClientEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x6a3ed4);
  (this->super_IClientEntity).super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6a4270);
  (this->super_IClientEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6a4328);
  (this->super_IClientEntity).super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6a4364);
  (this->super_IClientModelRenderable)._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6a4380);
  this[1].super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)0x0;
  this[1].super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)0x0;
  this[1].m_pfnThink.__pfn = (_func___thiscall_void_C_BaseEntity_ptr *)0x0;
  this[1].m_pfnThink.__delta = -1;
  (**(code **)(unaff_EBX + 0x6a3f90))(this,entnum,serialNum);
  return &(this->super_IClientEntity).super_IClientNetworkable;
}


/* __tcf_0 at 0097bd50 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3ed158),in_stack_00000008);
  return;
}


/* C_PaintSprayer::C_PaintSprayer at 00526c80 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PaintSprayer": ignoring overlapping field "m_bOldActive" */
/* DWARF original prototype: void C_PaintSprayer(C_PaintSprayer * this) */

void __thiscall C_PaintSprayer::C_PaintSprayer(C_PaintSprayer *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::C_BaseEntity(&this->super_C_BaseEntity);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(&UNK_006a3ffb + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(&UNK_006a4397 + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6a444f);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6a448b);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(&UNK_006a44a7 + unaff_EBX);
  this->m_flAccumulatedTime = 0.0;
  this->m_nBlobSpawned = 0;
  this->m_flStartTime = 0.0;
  (this->m_hPaintStream).
  super_CNetworkVarBase<CBaseHandle,C_PaintSprayer::NetworkVar_m_hPaintStream>.m_Value.m_Index =
       0xffffffff;
  return;
}


/* C_PaintSprayer::C_PaintSprayer at 00526d10 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PaintSprayer": ignoring overlapping field "m_bOldActive" */
/* DWARF original prototype: void C_PaintSprayer(C_PaintSprayer * this) */

void __thiscall C_PaintSprayer::C_PaintSprayer(C_PaintSprayer *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::C_BaseEntity(&this->super_C_BaseEntity);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6a3f6b);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)
       (CNonMemberScriptBinding0<const_char*_(*)(),const_char*>::Call + unaff_EBX + 7);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(&UNK_006a43bf + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6a43fb);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6a4417);
  this->m_flAccumulatedTime = 0.0;
  this->m_nBlobSpawned = 0;
  this->m_flStartTime = 0.0;
  (this->m_hPaintStream).
  super_CNetworkVarBase<CBaseHandle,C_PaintSprayer::NetworkVar_m_hPaintStream>.m_Value.m_Index =
       0xffffffff;
  return;
}


/* C_PaintSprayer::~C_PaintSprayer at 00526b50 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PaintSprayer": ignoring overlapping field "m_bOldActive" */
/* DWARF original prototype: void ~C_PaintSprayer(C_PaintSprayer * this, int __in_chrg) */

void __thiscall C_PaintSprayer::~C_PaintSprayer(C_PaintSprayer *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6a4127);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6a44c3);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6a457b);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6a45b7);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6a45d3);
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,in_stack_ffffffe8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_PaintSprayer::~C_PaintSprayer at 00526bc0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PaintSprayer": ignoring overlapping field "m_bOldActive" */
/* DWARF original prototype: void ~C_PaintSprayer(C_PaintSprayer * this, int __in_chrg) */

void __thiscall C_PaintSprayer::~C_PaintSprayer(C_PaintSprayer *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(&UNK_006a40c0 + extraout_ECX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(&UNK_006a445c + extraout_ECX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(extraout_ECX + 0x6a4514);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(extraout_ECX + 0x6a4550);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(extraout_ECX + 0x6a456c);
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
  return;
}


/* C_PaintSprayer::~C_PaintSprayer at 00526c20 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PaintSprayer": ignoring overlapping field "m_bOldActive" */
/* DWARF original prototype: void ~C_PaintSprayer(C_PaintSprayer * this, int __in_chrg) */

void __thiscall C_PaintSprayer::~C_PaintSprayer(C_PaintSprayer *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x6a4060);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(extraout_ECX + 0x6a43fc);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(CWarpOverlay::Draw + extraout_ECX + 4);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(extraout_ECX + 0x6a44f0);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(extraout_ECX + 0x6a450c);
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
  return;
}


/* C_PaintSprayer::Precache at 00526af0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PaintSprayer": ignoring overlapping field "m_bOldActive" */
/* DWARF original prototype: void Precache(C_PaintSprayer * this) */

void __thiscall C_PaintSprayer::Precache(C_PaintSprayer *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  PrecacheParticleSystem((char *)(unaff_EBX + 0x468654));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x46866a));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x46867f));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x468694));
  C_BaseEntity::Precache(&this->super_C_BaseEntity);
  return;
}


/* C_PaintSprayer::Spawn at 00526ac0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PaintSprayer": ignoring overlapping field "m_bOldActive" */
/* DWARF original prototype: void Spawn(C_PaintSprayer * this) */

void __thiscall C_PaintSprayer::Spawn(C_PaintSprayer *this)

{
  (*(this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x26])(this);
  C_BaseEntity::Spawn(&this->super_C_BaseEntity);
  return;
}


/* C_PaintSprayer::PreDataUpdate at 00526aa0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PaintSprayer": ignoring overlapping field "m_bOldActive" */
/* DWARF original prototype: void PreDataUpdate(C_PaintSprayer * this, DataUpdateType_t updateType)
    */

void __thiscall C_PaintSprayer::PreDataUpdate(C_PaintSprayer *this,DataUpdateType_t updateType)

{
  (this->super_C_BaseEntity).field_0xa5d = this->m_bActive;
  C_BaseEntity::PreDataUpdate(&this->super_C_BaseEntity,updateType);
  return;
}


/* C_PaintSprayer::PostDataUpdate at 005269c0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PaintSprayer": ignoring overlapping field "m_bOldActive" */
/* DWARF original prototype: void PostDataUpdate(C_PaintSprayer * this, DataUpdateType_t updateType)
    */

void __thiscall C_PaintSprayer::PostDataUpdate(C_PaintSprayer *this,DataUpdateType_t updateType)

{
  _func_int_varargs *UNRECOVERED_JUMPTABLE;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::PostDataUpdate(&this->super_C_BaseEntity,updateType);
  if (updateType == DATA_UPDATE_CREATED) {
    if (this->m_bActive == false) {
      return;
    }
  }
  else {
    if ((bool)(this->super_C_BaseEntity).field_0xa5d == this->m_bActive) {
      return;
    }
    if (this->m_bActive == false) {
      UNRECOVERED_JUMPTABLE =
           (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
           _vptr_IHandleEntity[0x9d];
      goto LAB_00526a30;
    }
  }
  this->m_flAccumulatedTime = *(float *)(unaff_EBX + 0x4bb7b0) / this->m_flBlobsPerSecond;
  this->m_flLastThinkTime = this->m_flStartTime;
  this->m_nBlobSpawned = 0;
  UNRECOVERED_JUMPTABLE =
       (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
       _vptr_IHandleEntity[0x9d];
LAB_00526a30:
                    /* WARNING: Could not recover jumptable at 0x00526a3c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}


/* C_PaintSprayer::ClientThink at 00526940 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PaintSprayer": ignoring overlapping field "m_bOldActive" */
/* DWARF original prototype: void ClientThink(C_PaintSprayer * this) */

void __thiscall C_PaintSprayer::ClientThink(C_PaintSprayer *this)

{
  float fVar1;
  float fVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (this->m_bActive != false) {
                    /* Unresolved local var: float flDeltaTime@[???] */
    fVar1 = *(float *)(**(int **)(unaff_EBX + 0x63d44b) + 0xc);
    fVar2 = this->m_flLastThinkTime;
    this->m_flLastThinkTime = fVar1;
    SprayPaint(this,fVar1 - fVar2);
    (*(this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
      _vptr_IHandleEntity[0x9d])(this,0xc4a1a000);
  }
  return;
}


/* _GLOBAL__I__ZN14C_PaintSprayer17m_pClassRecvTableE at 00076380 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN14C_PaintSprayer17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

