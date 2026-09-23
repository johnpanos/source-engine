/* DWARF-guided pseudocode for game/client/portal2/c_baseprojectedentity.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* ClientClassInit<DT_BaseProjectedEntity::ignored> at 00073160 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

int ClientClassInit<DT_BaseProjectedEntity::ignored>(ignored *param_1)

{
  RecvVarProxyFn varProxy;
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x5c5].m_pNext +
               unaff_EBX + 2) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc43dc2);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x5c6].m_pNext +
                  unaff_EBX + 2),(char *)(unaff_EBX + 0x8ecbf2),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x5ca].m_pPrev +
                        unaff_EBX + 2),(char *)(unaff_EBX + 0x8f17d2),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00abdc76 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00abdc32 + unaff_EBX));
      varProxy = *(RecvVarProxyFn *)(&DAT_00abdcda + unaff_EBX);
      RecvPropVector((RecvProp_conflict1 *)
                     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x5ce].m_SerialNumber +
                     unaff_EBX + 2),(char *)(unaff_EBX + 0x90c3f6),0x114,0xc,0,varProxy);
      RecvPropVector((RecvProp_conflict1 *)
                     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x5d2].m_pEntity +
                     unaff_EBX + 2),(char *)(unaff_EBX + 0x908a32),0x108,0xc,0,varProxy);
      RecvPropEHandle((RecvProp *)
                      ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x5d5].m_pNext +
                      unaff_EBX + 2),(char *)(unaff_EBX + 0x91b796),0xa5c,4,
                      *(RecvVarProxyFn *)(&DAT_00abdc2e + unaff_EBX));
      RecvPropEHandle((RecvProp *)
                      (&UNK_00005d9e +
                      (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                      (char *)(unaff_EBX + 0x91b7a3),0xa60,4,
                      *(RecvVarProxyFn *)(&DAT_00abdc2e + unaff_EBX));
      RecvPropVector((RecvProp_conflict1 *)
                     (&UNK_00005dda +
                     (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                     (char *)(unaff_EBX + 0x91b7b3),0xa64,0xc,0,varProxy);
      RecvPropVector((RecvProp_conflict1 *)
                     (&UNK_00005e16 +
                     (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                     (char *)(unaff_EBX + 0x91b7cb),0xa70,0xc,0,varProxy);
      RecvPropVector((RecvProp_conflict1 *)
                     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x5e4].m_pNext +
                     unaff_EBX + 2),(char *)(unaff_EBX + 0x91b7e9),0xa7c,0xc,0,varProxy);
      RecvPropVector((RecvProp_conflict1 *)
                     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x5e8].m_pPrev +
                     unaff_EBX + 2),
                     (char *)(vgui::TextEntry::PanelMessageFunc_CompositionString::InitVar +
                             unaff_EBX),0xa88,0xc,0,varProxy);
      RecvPropVector((RecvProp_conflict1 *)
                     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x5ec].m_SerialNumber +
                     unaff_EBX + 2),(char *)(unaff_EBX + 0x90be8f),0xa94,0xc,0,varProxy);
      RecvPropVector((RecvProp_conflict1 *)
                     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x5f0].m_pEntity +
                     unaff_EBX + 2),(char *)(unaff_EBX + 0x90be9f),0xaa0,0xc,0,varProxy);
      RecvPropEHandle((RecvProp *)
                      ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x5f3].m_pNext +
                      unaff_EBX + 2),(char *)(unaff_EBX + 0x91b81d),0xaac,4,
                      *(RecvVarProxyFn *)(&DAT_00abdc2e + unaff_EBX));
      RecvPropEHandle((RecvProp *)
                      ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x5f7].m_pPrev +
                      unaff_EBX + 2),&UNK_0091b82d + unaff_EBX,0xab0,4,
                      *(RecvVarProxyFn *)(&DAT_00abdc2e + unaff_EBX));
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x5fb].m_SerialNumber +
                  unaff_EBX + 2),(char *)(unaff_EBX + 0x91b840),0xab4,4,0,(RecvVarProxyFn)0x0);
      ___cxa_guard_release(unaff_EBX + 0xc43dc2);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x200a),
             (RecvProp_conflict1 *)
             ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x5ca].m_pPrev + unaff_EBX + 2)
             ,0xe,(char *)(unaff_EBX + 0x91b85a));
  return 1;
}


/* __static_initialization_and_destruction_0 at 00073540 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  RecvTable *this;
  int *piVar1;
  undefined4 uVar2;
  int iVar3;
  int unaff_EBX;
  longlong lVar4;
  
  lVar4 = ___i686_get_pc_thunk_bx();
  if (lVar4 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x582].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x582].m_pNext + unaff_EBX)
       = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x582].m_pNext + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x582].m_pNext + unaff_EBX + 2) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x582].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x583].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x583].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x583].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x583].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x584].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x584].m_SerialNumber + unaff_EBX + 3) =
       0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x584].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x584].m_pNext + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x585].m_pEntity + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x585].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x585].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined **)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x585].m_pNext + unaff_EBX + 3) =
       &UNK_00abe8b7 + unaff_EBX;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x1c17) = unaff_EBX + 0x91b48e;
  *(undefined **)((int)s_pParticlePtrs + unaff_EBX + 0x1c0f) = &UNK_004ad00f + unaff_EBX;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0x1c13) = 0;
  this = (RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x1c27);
  *(RecvTable **)((int)s_pParticlePtrs + unaff_EBX + 0x1c1b) = this;
  piVar1 = *(int **)(&DAT_00abd847 + unaff_EBX);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x1c1f) = *piVar1;
  *piVar1 = unaff_EBX + 0xcdb46f;
  RecvTable::RecvTable(this);
  uVar2 = *(undefined4 *)(&DAT_00abd7cf + unaff_EBX);
  ___cxa_atexit(unaff_EBX + 0x8dda6f,0,uVar2);
  iVar3 = ClientClassInit<DT_BaseProjectedEntity::ignored>((ignored *)0x0);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x1c3b) = iVar3;
  *(undefined4 *)(&DAT_00b82597 + unaff_EBX) = 9;
  *(undefined **)(&DAT_00b82593 + unaff_EBX) = &UNK_00b825ef + unaff_EBX;
  *(undefined **)((int)s_pParticlePtrs + unaff_EBX + 0x1c3f) = &DAT_00b82593 + unaff_EBX;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x586].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x586].m_SerialNumber + unaff_EBX + 3) =
       0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x586].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x586].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x587].m_pEntity + unaff_EBX + 3) = 0;
  ___cxa_atexit(unaff_EBX + 0x8dda8f,0,uVar2);
  return;
}


/* C_BaseProjectedEntity::YouForgotToImplementOrDeclareClientClass at 005200d0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_BaseProjectedEntity *
   this) */

int __thiscall
C_BaseProjectedEntity::YouForgotToImplementOrDeclareClientClass(C_BaseProjectedEntity *this)

{
  return 0;
}


/* C_BaseProjectedEntity::GetClientClass at 005200f0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: ClientClass * GetClientClass(C_BaseProjectedEntity * this) */

ClientClass * __thiscall C_BaseProjectedEntity::GetClientClass(C_BaseProjectedEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x82e8c8);
}


/* _C_BaseProjectedEntity_CreateObject at 00520560 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_BaseProjectedEntity_CreateObject(int entnum,int serialNum)

{
  C_BaseProjectedEntity *this;
  IClientNetworkable *pIVar1;
  
                    /* Unresolved local var: C_BaseProjectedEntity * pRet@[???] */
  this = C_BaseEntity::operator_new(0xab8);
  C_BaseProjectedEntity::C_BaseProjectedEntity(this);
  pIVar1 = (IClientNetworkable *)0x0;
  if (this != (C_BaseProjectedEntity *)0x0) {
    (*(this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
      _vptr_IHandleEntity[0x2f])(this,entnum,serialNum);
    pIVar1 = &(this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable;
  }
  return pIVar1;
}


/* __tcf_0 at 00950fc0 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "MenuButton": ignoring multiple overlapping fields */
/* WARNING: Struct "CBaseViewport": ignoring multiple overlapping fields */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable
            ((RecvTable *)(CBaseViewport::CreatePanelByName + extraout_ECX),in_stack_00000008);
  return;
}


/* C_BaseProjectedEntity::GetPredDescMap at 00520100 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: datamap_t * GetPredDescMap(C_BaseProjectedEntity * this) */

datamap_t * __thiscall C_BaseProjectedEntity::GetPredDescMap(C_BaseProjectedEntity *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x6d59dc);
}


/* PredMapInit<C_BaseProjectedEntity> at 00520110 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

datamap_t * PredMapInit<C_BaseProjectedEntity>(C_BaseProjectedEntity *param_1)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *(undefined4 *)(extraout_ECX + 0x6d59cd) = 9;
  *(int *)(extraout_ECX + 0x6d59c9) = extraout_ECX + 0x6d5a25;
  return (datamap_t *)(extraout_ECX + 0x6d59c9);
}


/* __tcf_1 at 00950fe0 */

void __tcf_1(void *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  *(undefined4 *)(&DAT_00365f30 + unaff_EBX) = 0;
  if (*(int *)(&DAT_00365f2c + unaff_EBX) < 0) {
    iVar1 = *(int *)(&DAT_00365f24 + unaff_EBX);
  }
  else {
    if (*(int *)(&DAT_00365f24 + unaff_EBX) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1dfd30) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1dfd30),*(int *)(&DAT_00365f24 + unaff_EBX))
      ;
      *(undefined4 *)(&DAT_00365f24 + unaff_EBX) = 0;
    }
    iVar1 = 0;
    *(undefined4 *)(&DAT_00365f28 + unaff_EBX) = 0;
  }
  *(int *)(&DAT_00365f34 + unaff_EBX) = iVar1;
  if (-1 < *(int *)(&DAT_00365f2c + unaff_EBX)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1dfd30) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1dfd30),iVar1);
      *(undefined4 *)(&DAT_00365f24 + unaff_EBX) = 0;
    }
    *(undefined4 *)(&DAT_00365f28 + unaff_EBX) = 0;
  }
  return;
}


/* C_BaseProjectedEntity::C_BaseProjectedEntity at 00520430 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void C_BaseProjectedEntity(C_BaseProjectedEntity * this) */

void __thiscall C_BaseProjectedEntity::C_BaseProjectedEntity(C_BaseProjectedEntity *this)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined4 *puVar4;
  int iVar5;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::C_BaseEntity(&this->super_C_BaseEntity);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x673a4a);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x673e16);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(&UNK_00673ece + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x673f0a);
  (this->m_hHitPortal).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hSourcePortal).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hChildSegment).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_hPlacementHelper).super_CBaseHandle.m_Index = 0xffffffff;
  iVar2 = *(int *)(unaff_EBX + 0x796ae6);
  iVar1 = iVar2 + 1;
  iVar3 = iVar1;
  if (*(int *)(unaff_EBX + 0x796ade) < iVar1) {
    CUtlMemory<C_BaseProjectedEntity*,int>::Grow
              ((CUtlMemory<C_BaseProjectedEntity*,int> *)(unaff_EBX + 0x796ada),
               iVar1 - *(int *)(unaff_EBX + 0x796ade));
    iVar3 = *(int *)(unaff_EBX + 0x796ae6) + 1;
  }
  *(int *)(unaff_EBX + 0x796ae6) = iVar3;
  iVar5 = *(int *)(unaff_EBX + 0x796ada);
  *(int *)(unaff_EBX + 0x796aea) = iVar5;
  iVar3 = (iVar3 - iVar2) + -1;
  if (0 < iVar3) {
    _V_memmove((void *)(iVar5 + iVar1 * 4),(void *)(iVar5 + iVar2 * 4),iVar3 * 4);
    iVar5 = *(int *)(unaff_EBX + 0x796ada);
  }
  puVar4 = (undefined4 *)(iVar2 * 4 + iVar5);
  if (puVar4 != (undefined4 *)0x0) {
    *puVar4 = this;
  }
  return;
}


/* C_BaseProjectedEntity::C_BaseProjectedEntity at 00520550 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void C_BaseProjectedEntity(C_BaseProjectedEntity * this,
   C_BaseProjectedEntity * this) */

void __thiscall
C_BaseProjectedEntity::C_BaseProjectedEntity
          (C_BaseProjectedEntity *this,C_BaseProjectedEntity *this_1)

{
  C_BaseProjectedEntity(this);
  return;
}


/* C_BaseProjectedEntity::~C_BaseProjectedEntity at 00520360 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_BaseProjectedEntity(C_BaseProjectedEntity * this, int
   __in_chrg) */

void __thiscall
C_BaseProjectedEntity::~C_BaseProjectedEntity(C_BaseProjectedEntity *this,int __in_chrg)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int unaff_EBX;
  int local_14;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x673b1a);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(&UNK_00673ee6 + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x673f9e);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x673fda);
  iVar1 = *(int *)(unaff_EBX + 0x796bb6);
  if (0 < iVar1) {
    piVar2 = *(int **)(unaff_EBX + 0x796baa);
    if (this == (C_BaseProjectedEntity *)*piVar2) {
      iVar4 = 0;
      local_14 = 0;
    }
    else {
      iVar4 = 0;
      local_14 = 4;
      iVar3 = local_14;
      do {
        local_14 = iVar3;
        iVar4 = iVar4 + 1;
        if (iVar4 == iVar1) goto LAB_005203e6;
        iVar3 = local_14 + 4;
      } while (this != (C_BaseProjectedEntity *)piVar2[iVar4]);
    }
    if (iVar4 != iVar1 + -1) {
      *(int *)((int)piVar2 + local_14) = piVar2[iVar1 + -1];
      iVar4 = *(int *)(unaff_EBX + 0x796bb6) + -1;
    }
    *(int *)(unaff_EBX + 0x796bb6) = iVar4;
    C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
    return;
  }
LAB_005203e6:
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
  return;
}


/* C_BaseProjectedEntity::~C_BaseProjectedEntity at 005205c0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_BaseProjectedEntity(C_BaseProjectedEntity * this, int
   __in_chrg) */

void __thiscall
C_BaseProjectedEntity::~C_BaseProjectedEntity(C_BaseProjectedEntity *this,int __in_chrg)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int unaff_EBX;
  int in_stack_ffffffb8;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6738ba);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x673c86);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x673d3e);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x673d7a);
  iVar1 = *(int *)(unaff_EBX + 0x796956);
  if (0 < iVar1) {
    piVar2 = *(int **)(unaff_EBX + 0x79694a);
    if (this == (C_BaseProjectedEntity *)*piVar2) {
      iVar4 = 0;
      local_20 = 0;
    }
    else {
      iVar4 = 0;
      local_20 = 4;
      iVar3 = local_20;
      do {
        local_20 = iVar3;
        iVar4 = iVar4 + 1;
        if (iVar4 == iVar1) goto LAB_00520646;
        iVar3 = local_20 + 4;
      } while (this != (C_BaseProjectedEntity *)piVar2[iVar4]);
    }
    if (iVar4 != iVar1 + -1) {
      *(int *)((int)piVar2 + local_20) = piVar2[iVar1 + -1];
      iVar4 = *(int *)(unaff_EBX + 0x796956) + -1;
    }
    *(int *)(unaff_EBX + 0x796956) = iVar4;
  }
LAB_00520646:
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,in_stack_ffffffb8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_BaseProjectedEntity::~C_BaseProjectedEntity at 00520680 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_BaseProjectedEntity(C_BaseProjectedEntity * this, int
   __in_chrg) */

void __thiscall
C_BaseProjectedEntity::~C_BaseProjectedEntity(C_BaseProjectedEntity *this,int __in_chrg)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int unaff_EBX;
  int local_14;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6737fa);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(&UNK_00673bc6 + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x673c7e);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x673cba);
  iVar1 = *(int *)(&DAT_00796896 + unaff_EBX);
  if (0 < iVar1) {
    piVar2 = *(int **)(unaff_EBX + 0x79688a);
    if (this == (C_BaseProjectedEntity *)*piVar2) {
      iVar4 = 0;
      local_14 = 0;
    }
    else {
      iVar4 = 0;
      local_14 = 4;
      iVar3 = local_14;
      do {
        local_14 = iVar3;
        iVar4 = iVar4 + 1;
        if (iVar4 == iVar1) goto LAB_00520706;
        iVar3 = local_14 + 4;
      } while (this != (C_BaseProjectedEntity *)piVar2[iVar4]);
    }
    if (iVar4 != iVar1 + -1) {
      *(int *)((int)piVar2 + local_14) = piVar2[iVar1 + -1];
      iVar4 = *(int *)(&DAT_00796896 + unaff_EBX) + -1;
    }
    *(int *)(&DAT_00796896 + unaff_EBX) = iVar4;
    C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
    return;
  }
LAB_00520706:
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
  return;
}


/* C_BaseProjectedEntity::GetPredictionOwner at 005202a0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: C_BasePlayer * GetPredictionOwner(C_BaseProjectedEntity * this) */

C_BasePlayer * __thiscall C_BaseProjectedEntity::GetPredictionOwner(C_BaseProjectedEntity *this)

{
  uint uVar1;
  char cVar2;
  int iVar3;
  int iVar4;
  C_BasePlayer *pCVar5;
  int unaff_EBX;
  
                    /* Unresolved local var: C_Portal_Base2D * pSourcePortal@[???] */
  ___i686_get_pc_thunk_bx();
  iVar4 = **(int **)(unaff_EBX + 0x610adf);
  uVar1 = (this->m_hSourcePortal).super_CBaseHandle.m_Index;
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???]
                       Unresolved local var: CEntInfo * pInfo@[???] */
  if (((((uVar1 == 0xffffffff) ||
        (iVar3 = (uVar1 & 0xffff) * 0x10 + iVar4, *(uint *)(iVar3 + 8) != uVar1 >> 0x10)) ||
       (iVar3 = *(int *)(iVar3 + 4), iVar3 == 0)) ||
      ((iVar3 = ___dynamic_cast(iVar3,*(undefined4 *)(unaff_EBX + 0x610d6f),
                                *(undefined4 *)(&DAT_00610d77 + unaff_EBX),0), iVar3 == 0 ||
       (uVar1 = *(uint *)(iVar3 + 0x33d0), uVar1 == 0xffffffff)))) ||
     ((iVar4 = (uVar1 & 0xffff) * 0x10 + iVar4, *(uint *)(iVar4 + 8) != uVar1 >> 0x10 ||
      ((pCVar5 = *(C_BasePlayer **)(iVar4 + 4), pCVar5 == (C_BasePlayer *)0x0 ||
       (cVar2 = (**(code **)(*(int *)&(pCVar5->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                      super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                      super_C_BaseEntity.super_IClientEntity.super_IClientUnknown +
                            0x300))(pCVar5), cVar2 == '\0')))))) {
    pCVar5 = (C_BasePlayer *)0x0;
  }
  return pCVar5;
}


/* C_BaseProjectedEntity::PostDataUpdate at 005201a0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void PostDataUpdate(C_BaseProjectedEntity * this, DataUpdateType_t
   updateType) */

void __thiscall
C_BaseProjectedEntity::PostDataUpdate(C_BaseProjectedEntity *this,DataUpdateType_t updateType)

{
  uint uVar1;
  int iVar2;
  C_BasePlayer *pOwner;
  int unaff_EBX;
  
                    /* Unresolved local var: C_Portal_Base2D * pSourcePortal@[???] */
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::PostDataUpdate(&this->super_C_BaseEntity,updateType);
  uVar1 = (this->m_hSourcePortal).super_CBaseHandle.m_Index;
  if (((uVar1 == 0xffffffff) ||
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x610bdc),
      *(uint *)(iVar2 + 8) != uVar1 >> 0x10)) || (iVar2 = *(int *)(iVar2 + 4), iVar2 == 0)) {
    C_BaseEntity::SetPredictionEligible(&this->super_C_BaseEntity,false);
  }
  else {
    C_BaseEntity::SetPredictionEligible(&this->super_C_BaseEntity,true);
    if (*(char *)(iVar2 + 0xa09) != '\0') {
      uVar1 = *(uint *)(iVar2 + 0xa18);
      if ((uVar1 == 0xffffffff) ||
         (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x610bdc),
         *(uint *)(iVar2 + 8) != uVar1 >> 0x10)) {
        pOwner = (C_BasePlayer *)0x0;
      }
      else {
        pOwner = *(C_BasePlayer **)(iVar2 + 4);
      }
      C_BaseEntity::SetPlayerSimulated(&this->super_C_BaseEntity,pOwner);
      goto LAB_00520202;
    }
  }
  C_BaseEntity::UnsetPlayerSimulated(&this->super_C_BaseEntity);
LAB_00520202:
  if (updateType != DATA_UPDATE_CREATED) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x00520237. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
    _vptr_IHandleEntity[0xf0])();
  return;
}


/* C_BaseProjectedEntity::TestAllForProjectionChanges at 00520140 */

void C_BaseProjectedEntity::TestAllForProjectionChanges(void)

{
  int *piVar1;
  int unaff_EBX;
  int iVar2;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x796dd6)) {
    iVar2 = 0;
    do {
      piVar1 = *(int **)(*(int *)(unaff_EBX + 0x796dca) + iVar2 * 4);
      (**(code **)(*piVar1 + 0x3a0))(piVar1);
      iVar2 = iVar2 + 1;
    } while (iVar2 < *(int *)(unaff_EBX + 0x796dd6));
  }
  return;
}


/* _GLOBAL__I__ZN21C_BaseProjectedEntity17m_pClassRecvTableE at 000736c0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN21C_BaseProjectedEntity17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

