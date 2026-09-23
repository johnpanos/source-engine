/* DWARF-guided pseudocode for game/client/portal2/c_prop_rockettripwire.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* ClientClassInit<DT_PropRocketTripwire::ignored> at 000789a0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

int ClientClassInit<DT_PropRocketTripwire::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2d4].m_pEntity + unaff_EBX)
      == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc40ea4);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2d4].m_pPrev + unaff_EBX
                  ),(char *)(unaff_EBX + 0x8e73ac),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2d8].
                               m_SerialNumber + unaff_EBX),(char *)(unaff_EBX + 0x8ebf8c),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00ab83f4 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00ab83ec + unaff_EBX));
      RecvPropInt((RecvProp_conflict1 *)
                  (__static_initialization_and_destruction_0 +
                  (int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX),
                  (char *)(unaff_EBX + 0x917bf9),0xd8c,4,0,(RecvVarProxyFn)0x0);
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2df].m_pNext + unaff_EBX
                  ),(char *)(unaff_EBX + 0x917c0d),0xd88,4,0,(RecvVarProxyFn)0x0);
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2e3].m_pPrev +
                   unaff_EBX),(char *)(unaff_EBX + 0x917c24),0xd90,1);
      RecvPropEHandle((RecvProp *)
                      ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2e7].m_SerialNumber
                      + unaff_EBX),(char *)(unaff_EBX + 0x917c31),0xd94,4,
                      *(RecvVarProxyFn *)(&DAT_00ab83e8 + unaff_EBX));
      ___cxa_guard_release(unaff_EBX + 0xc40ea4);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_SIMDRandContexts[0x1e].m_RandY[0x2f] + unaff_EBX + 0xc),
             (RecvProp_conflict1 *)
             ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2d8].m_SerialNumber +
             unaff_EBX),5,(char *)(unaff_EBX + 0x917c3b));
  return 1;
}


/* __static_initialization_and_destruction_0 at 00078b90 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

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
  (&UNK_00002b1f)[(int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
  ClientClassInit<DT_BaseParticleEntity::ignored>
  [(int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = (code)0x0;
  (&UNK_00002b1d)[(int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX] =
       0;
  (&UNK_00002b1e)[(int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX] =
       0;
  *(undefined4 *)
   (&UNK_00002b1f + (int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX)
       = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2b2].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2b2].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2b2].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2b2].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2b3].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2b3].m_SerialNumber + unaff_EBX + 3) =
       0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2b3].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2b3].m_pNext + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2b4].m_pEntity + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2b4].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2b4].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined **)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x2b4].m_pNext + unaff_EBX + 3) =
       &UNK_00ab9267 + unaff_EBX;
  *(int *)((int)s_SIMDRandContexts[0x1e].m_RandY[0xf] + unaff_EBX + 0xf) = unaff_EBX + 0x917a64;
  *(int *)((int)s_SIMDRandContexts[0x1e].m_RandY[0xf] + unaff_EBX + 7) = unaff_EBX + 0x4bfb8f;
  *(undefined4 *)((int)s_SIMDRandContexts[0x1e].m_RandY[0xf] + unaff_EBX + 0xb) = 0;
  this = (RecvTable *)((int)s_SIMDRandContexts[0x1e].m_RandY[0x10] + unaff_EBX + 0xf);
  *(RecvTable **)((int)s_SIMDRandContexts[0x1e].m_RandY[0x10] + unaff_EBX + 3) = this;
  piVar1 = *(int **)(&DAT_00ab81f7 + unaff_EBX);
  *(int *)((int)s_SIMDRandContexts[0x1e].m_RandY[0x10] + unaff_EBX + 7) = *piVar1;
  *piVar1 = unaff_EBX + 0xcd7397;
  RecvTable::RecvTable(this);
  ___cxa_atexit(&UNK_008d905f + unaff_EBX,0,*(undefined4 *)(&DAT_00ab817f + unaff_EBX));
  iVar2 = ClientClassInit<DT_PropRocketTripwire::ignored>((ignored *)0x0);
  *(int *)((int)s_SIMDRandContexts[0x1e].m_RandY[0x12] + unaff_EBX + 3) = iVar2;
  return;
}


/* C_PropRocketTripwire::YouForgotToImplementOrDeclareClientClass at 00537d90 */

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


/* C_PropRocketTripwire::GetClientClass at 00537db0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: ClientClass * GetClientClass(C_PropRocketTripwire * this) */

ClientClass * __thiscall C_PropRocketTripwire::GetClientClass(C_PropRocketTripwire *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x818180);
}


/* _C_PropRocketTripwire_CreateObject at 00538730 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6619ab);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x661e03);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x661ebb);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x661ef7);
  (this->super_IClientModelRenderable)._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x661f13);
  this[1].super_C_BaseEntity.m_iClassname = (string_t)0xffffffff;
  CTraceFilterSkipClassname::CTraceFilterSkipClassname
            ((CTraceFilterSkipClassname *)&this[1].super_C_BaseEntity.m_hScriptInstance,
             (IHandleEntity *)this,(char *)(CHudIcons::RefreshHudTextures + unaff_EBX + 3),1);
  (*(this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0x2f])(this,entnum,serialNum);
  return &(this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable;
}


/* __tcf_0 at 00951c00 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3fe348),in_stack_00000008);
  return;
}


/* C_PropRocketTripwire::C_PropRocketTripwire at 00538010 */

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
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6620c4);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(&UNK_0066251c + unaff_EBX);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x6625d4);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x662610);
  (this->super_C_BaseAnimating).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x66262c);
  (this->m_hRocket).super_CBaseHandle.m_Index = 0xffffffff;
  CTraceFilterSkipClassname::CTraceFilterSkipClassname
            (&this->m_filterBeams,(IHandleEntity *)this,(char *)(unaff_EBX + 0x43d30c),1);
  return;
}


/* C_PropRocketTripwire::C_PropRocketTripwire at 005380d0 */

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
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(&UNK_00662004 + unaff_EBX);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x66245c);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x662514);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x662550);
  (this->super_C_BaseAnimating).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(&UNK_0066256c + unaff_EBX);
  (this->m_hRocket).super_CBaseHandle.m_Index = 0xffffffff;
  CTraceFilterSkipClassname::CTraceFilterSkipClassname
            (&this->m_filterBeams,(IHandleEntity *)this,(char *)(unaff_EBX + 0x43d24c),1);
  return;
}


/* C_PropRocketTripwire::~C_PropRocketTripwire at 00537e30 */

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
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6622a4);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x6626fc);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x6627b4);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x6627f0);
  (this->super_C_BaseAnimating).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x66280c);
  if (this->m_pBeam != (C_Beam *)0x0) {
    C_BaseEntity::Remove(&this->m_pBeam->super_C_BaseEntity);
  }
  C_BaseAnimating::~C_BaseAnimating(&this->super_C_BaseAnimating,in_stack_ffffffd8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_PropRocketTripwire::~C_PropRocketTripwire at 00537ed0 */

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
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x662204);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x66265c);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x662714);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x662750);
  (this->super_C_BaseAnimating).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x66276c);
  if (this->m_pBeam != (C_Beam *)0x0) {
    C_BaseEntity::Remove(&this->m_pBeam->super_C_BaseEntity);
  }
  C_BaseAnimating::~C_BaseAnimating(&this->super_C_BaseAnimating,__in_chrg);
  return;
}


/* C_PropRocketTripwire::~C_PropRocketTripwire at 00537f70 */

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
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x662164);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(&UNK_006625bc + unaff_EBX);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(unaff_EBX + 0x662674);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(unaff_EBX + 0x6626b0);
  (this->super_C_BaseAnimating).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6626cc);
  if (this->m_pBeam != (C_Beam *)0x0) {
    C_BaseEntity::Remove(&this->m_pBeam->super_C_BaseEntity);
  }
  C_BaseAnimating::~C_BaseAnimating(&this->super_C_BaseAnimating,__in_chrg);
  return;
}


/* C_PropRocketTripwire::Spawn at 00537dd0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(C_PropRocketTripwire * this) */

void __thiscall C_PropRocketTripwire::Spawn(C_PropRocketTripwire *this)

{
  C_BaseEntity::ThinkSet((C_BaseEntity *)this,(BASEPTR)0x151,0.0,(char *)0x0);
  (*(this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x9b])(this,0xc4a1a000);
  this->m_pBeam = (C_Beam *)0x0;
  C_BaseEntity::Spawn((C_BaseEntity *)this);
  return;
}


/* C_PropRocketTripwire::ClientThink at 005381a0 */

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
      super_IHandleEntity._vptr_IHandleEntity[0x74])
              (this,this->m_iMuzzleAttachment,&local_2c,&local_38);
    AngleVectors(&local_38,&local_44);
    pCVar8 = this->m_pBeam;
    if (pCVar8 == (C_Beam *)0x0) {
      pCVar8 = C_Beam::BeamCreate((char *)(unaff_EBX + 0x45347a),0.1);
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
      if (*(float *)(unaff_EBX + 0x47e693) != (pCVar8->m_fWidth).m_Value) {
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
      if (*(float *)(unaff_EBX + 0x47e7a7) != (pCVar8->m_fHaloScale).m_Value) {
        (pCVar8->m_fHaloScale).m_Value = *(float *)(unaff_EBX + 0x47e7a7);
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
      if (*(float *)(unaff_EBX + 0x47e693) != (pCVar8->m_fWidth).m_Value) {
        (pCVar8->m_fWidth).m_Value = 1.0;
        pCVar8 = this->m_pBeam;
      }
      if ((pCVar8->m_fSpeed).m_Value != 0.0) {
        (pCVar8->m_fSpeed).m_Value = 0.0;
        pCVar8 = this->m_pBeam;
      }
      if (*(float *)(unaff_EBX + 0x47e793) != (pCVar8->m_fHaloScale).m_Value) {
        (pCVar8->m_fHaloScale).m_Value = *(float *)(unaff_EBX + 0x47e793);
      }
    }
    else if (this->m_bDrawBeams == false) {
      C_BaseEntity::AddEffects(&pCVar8->super_C_BaseEntity,0x20);
    }
    else {
      piVar1 = &(pCVar8->super_C_BaseEntity).m_fEffects;
      *piVar1 = *piVar1 & 0xffffffdf;
      C_BaseEntity::UpdateVisibility(&pCVar8->super_C_BaseEntity);
    }
                    /* Unresolved local var: Vector res@[???] */
    fVar2 = *(float *)(unaff_EBX + 0x482e97);
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
        (iVar5 = (uVar3 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5f8bdf),
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


/* C_PropRocketTripwire::GetBeamTraceFilter at 00537dc0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "BeamClipStyle_t": Some values do not have unique names */
/* DWARF original prototype: ITraceFilter * GetBeamTraceFilter(C_PropRocketTripwire * this) */

ITraceFilter * __thiscall C_PropRocketTripwire::GetBeamTraceFilter(C_PropRocketTripwire *this)

{
  return (ITraceFilter *)&this->m_filterBeams;
}


/* _GLOBAL__I__ZN20C_PropRocketTripwire17m_pClassRecvTableE at 00078cb0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN20C_PropRocketTripwire17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

