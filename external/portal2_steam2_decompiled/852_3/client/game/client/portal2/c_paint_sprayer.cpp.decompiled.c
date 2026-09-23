/* DWARF-guided pseudocode for game/client/portal2/c_paint_sprayer.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* C_PaintSprayer::YouForgotToImplementOrDeclareClientClass at 0052b7c0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PaintSprayer": ignoring overlapping field "m_bOldActive" */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_PaintSprayer * this) */

int __thiscall C_PaintSprayer::YouForgotToImplementOrDeclareClientClass(C_PaintSprayer *this)

{
  return 0;
}


/* C_PaintSprayer::GetClientClass at 0052b7e0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PaintSprayer": ignoring overlapping field "m_bOldActive" */
/* DWARF original prototype: ClientClass * GetClientClass(C_PaintSprayer * this) */

ClientClass * __thiscall C_PaintSprayer::GetClientClass(C_PaintSprayer *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x824270);
}


/* _C_PaintSprayer_CreateObject at 0052bc20 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_PaintSprayer_CreateObject(int entnum,int serialNum)

{
  C_BaseEntity *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_PaintSprayer * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xa9c);
  C_BaseEntity::C_BaseEntity(this);
  (this->super_IClientEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x66b334);
  (this->super_IClientEntity).super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(&UNK_0066b6cc + unaff_EBX);
  (this->super_IClientEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(&UNK_0066b784 + unaff_EBX);
  (this->super_IClientEntity).super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(&UNK_0066b7c0 + unaff_EBX);
  this[1].super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)0x0;
  this[1].super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)0x0;
  this[1].m_pfnThink.__delta = 0;
  (**(code **)(unaff_EBX + 0x66b3f0))(this,entnum,serialNum);
  return &(this->super_IClientEntity).super_IClientNetworkable;
}


/* ClientClassInit<DT_PaintSprayer::ignored> at 00075f10 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "PageTab": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "ComboBoxButton": ignoring overlapping field "m_DisabledBgColor" */
/* WARNING: Struct "ComboBox": ignoring multiple overlapping fields */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

int ClientClassInit<DT_PaintSprayer::ignored>(ignored *param_1)

{
  RecvVarProxyFn p_Var1;
  int iVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3f7].m_SerialNumber +
               unaff_EBX + 2) == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xc420da);
    if (iVar2 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3f7].m_pNext +
                  unaff_EBX + 2),(char *)(vgui::PropertySheet::ApplySchemeSettings + unaff_EBX + 2),
                  0,4,0,(RecvVarProxyFn)0x0);
      p_Var1 = *(RecvVarProxyFn *)(&DAT_00abaf2a + unaff_EBX);
      RecvPropVector((RecvProp_conflict1 *)
                     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3fb].m_pPrev +
                     unaff_EBX + 2),&UNK_00909646 + unaff_EBX,0x114,0xc,0,p_Var1);
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3ff].m_SerialNumber +
                  unaff_EBX + 2),(char *)(unaff_EBX + 0x8e9c50),0x124,4,0,
                  *(RecvVarProxyFn *)(&DAT_00abaf16 + unaff_EBX));
      RecvPropVector((RecvProp_conflict1 *)
                     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x403].m_pEntity +
                     unaff_EBX + 2),(char *)(unaff_EBX + 0x905c82),0x108,0xc,0,p_Var1);
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x406].m_pNext +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0x9044b6),0xa68,1);
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x40a].m_pPrev +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0x919b57),0xa69,1);
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x40e].m_SerialNumber +
                  unaff_EBX + 2),(char *)(unaff_EBX + 0x8ee37e),0xa6c,4,0,(RecvVarProxyFn)0x0);
      p_Var1 = *(RecvVarProxyFn *)(&DAT_00abaec2 + unaff_EBX);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x412].m_pEntity +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x919b61),0xa70,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x415].m_pNext +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x919b74),0xa7c,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x419].m_pPrev +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x919b81),0xa80,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x41d].m_SerialNumber +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x919b8e),0xa74,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x421].m_pEntity +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x919ba3),0xa78,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x424].m_pNext +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x919bb7),0xa84,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x428].m_pPrev +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x919bcc),0xa88,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x42c].m_SerialNumber +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x919bde),0xa8c,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x430].m_pEntity +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x919bf0),0xa90,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x433].m_pNext +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x919c09),0xa94,4,0,p_Var1);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x437].m_pPrev +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x9028c7),0xa98,4,0,p_Var1);
      ___cxa_guard_release(unaff_EBX + 0xc420da);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x2f2),
             (RecvProp_conflict1 *)
             ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3fb].m_pPrev + unaff_EBX + 2)
             ,0x11,&UNK_00919c22 + unaff_EBX);
  return 1;
}


/* __static_initialization_and_destruction_0 at 00076390 */

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
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3ab].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3ab].m_pNext + unaff_EBX)
       = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3ab].m_pNext + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3ab].m_pNext + unaff_EBX + 2) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3ab].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3ac].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3ac].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3ac].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3ac].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3ad].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3ad].m_SerialNumber + unaff_EBX + 3) =
       0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3ad].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3ad].m_pNext + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3ae].m_pEntity + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3ae].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3ae].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined **)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3ae].m_pNext + unaff_EBX + 3) =
       &UNK_00abba67 + unaff_EBX;
  *(int *)((int)(&s_ParticleSystemMgr.m_VisualizedOperatorId + 5) + unaff_EBX + 0xf) =
       unaff_EBX + 0x9197af;
  *(int *)((int)(&s_ParticleSystemMgr.m_VisualizedOperatorId + 5) + unaff_EBX + 7) =
       unaff_EBX + 0x4b587f;
  *(undefined4 *)((int)(&s_ParticleSystemMgr.m_VisualizedOperatorId + 5) + unaff_EBX + 0xb) = 0;
  this = (RecvTable *)((int)(&s_ParticleSystemMgr.m_VisualizedOperatorId + 6) + unaff_EBX + 0xf);
  *(RecvTable **)((int)(&s_ParticleSystemMgr.m_VisualizedOperatorId + 6) + unaff_EBX + 3) = this;
  piVar1 = *(int **)(&DAT_00aba9f7 + unaff_EBX);
  *(int *)((int)(&s_ParticleSystemMgr.m_VisualizedOperatorId + 6) + unaff_EBX + 7) = *piVar1;
  *piVar1 = unaff_EBX + 0xcd96b7;
  RecvTable::RecvTable(this);
  ___cxa_atexit(&UNK_008db4ef + unaff_EBX,0,*(undefined4 *)(&DAT_00aba97f + unaff_EBX));
  iVar2 = ClientClassInit<DT_PaintSprayer::ignored>((ignored *)0x0);
  *(int *)((int)&vec2_origin.x + unaff_EBX + 3) = iVar2;
  return;
}


/* __tcf_0 at 00951890 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3fe1d8),in_stack_00000008);
  return;
}


/* C_PaintSprayer::C_PaintSprayer at 0052bb20 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x66b437);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(&UNK_0066b7cf + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x66b887);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x66b8c3);
  this->m_flAccumulatedTime = 0.0;
  this->m_nBlobSpawned = 0;
  this->m_flStartTime = 0.0;
  return;
}


/* C_PaintSprayer::C_PaintSprayer at 0052bba0 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x66b3b7);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(&UNK_0066b74f + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x66b807);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x66b843);
  this->m_flAccumulatedTime = 0.0;
  this->m_nBlobSpawned = 0;
  this->m_flStartTime = 0.0;
  return;
}


/* C_PaintSprayer::~C_PaintSprayer at 0052ba10 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x66b547);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x66b8df);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x66b997);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x66b9d3);
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,in_stack_ffffffe8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_PaintSprayer::~C_PaintSprayer at 0052ba80 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PaintSprayer": ignoring overlapping field "m_bOldActive" */
/* DWARF original prototype: void ~C_PaintSprayer(C_PaintSprayer * this, int __in_chrg) */

void __thiscall C_PaintSprayer::~C_PaintSprayer(C_PaintSprayer *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x66b4e0);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(extraout_ECX + 0x66b878);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(extraout_ECX + 0x66b930);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(extraout_ECX + 0x66b96c);
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
  return;
}


/* C_PaintSprayer::~C_PaintSprayer at 0052bad0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PaintSprayer": ignoring overlapping field "m_bOldActive" */
/* DWARF original prototype: void ~C_PaintSprayer(C_PaintSprayer * this, int __in_chrg) */

void __thiscall C_PaintSprayer::~C_PaintSprayer(C_PaintSprayer *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x66b490);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(extraout_ECX + 0x66b828);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(extraout_ECX + 0x66b8e0);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(&UNK_0066b91c + extraout_ECX);
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
  return;
}


/* C_PaintSprayer::Precache at 0052b9b0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PaintSprayer": ignoring overlapping field "m_bOldActive" */
/* DWARF original prototype: void Precache(C_PaintSprayer * this) */

void __thiscall C_PaintSprayer::Precache(C_PaintSprayer *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  PrecacheParticleSystem((char *)(unaff_EBX + 0x4650bb));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x4650d1));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x438eab));
  PrecacheParticleSystem((char *)(unaff_EBX + 0x4650e7));
  C_BaseEntity::Precache(&this->super_C_BaseEntity);
  return;
}


/* C_PaintSprayer::Spawn at 0052b980 */

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


/* C_PaintSprayer::PreDataUpdate at 0052b960 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PaintSprayer": ignoring overlapping field "m_bOldActive" */
/* DWARF original prototype: void PreDataUpdate(C_PaintSprayer * this, DataUpdateType_t updateType)
    */

void __thiscall C_PaintSprayer::PreDataUpdate(C_PaintSprayer *this,DataUpdateType_t updateType)

{
  (this->super_C_BaseEntity).field_0xa59 = this->m_bActive;
  C_BaseEntity::PreDataUpdate(&this->super_C_BaseEntity,updateType);
  return;
}


/* C_PaintSprayer::PostDataUpdate at 0052b880 */

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
    if ((bool)(this->super_C_BaseEntity).field_0xa59 == this->m_bActive) {
      return;
    }
    if (this->m_bActive == false) {
      UNRECOVERED_JUMPTABLE =
           (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
           _vptr_IHandleEntity[0x9b];
      goto LAB_0052b8f0;
    }
  }
  this->m_flAccumulatedTime = *(float *)(unaff_EBX + 0x48afb0) / this->m_flBlobsPerSecond;
  this->m_flLastThinkTime = this->m_flStartTime;
  this->m_nBlobSpawned = 0;
  UNRECOVERED_JUMPTABLE =
       (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
       _vptr_IHandleEntity[0x9b];
LAB_0052b8f0:
                    /* WARNING: Could not recover jumptable at 0x0052b8fc. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}


/* C_PaintSprayer::ClientThink at 0052b800 */

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
    fVar1 = *(float *)(**(int **)(unaff_EBX + 0x60553f) + 0xc);
    fVar2 = this->m_flLastThinkTime;
    this->m_flLastThinkTime = fVar1;
    SprayPaint(this,fVar1 - fVar2);
    (*(this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
      _vptr_IHandleEntity[0x9b])(this,0xc4a1a000);
  }
  return;
}


/* _GLOBAL__I__ZN14C_PaintSprayer17m_pClassRecvTableE at 000764b0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN14C_PaintSprayer17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

