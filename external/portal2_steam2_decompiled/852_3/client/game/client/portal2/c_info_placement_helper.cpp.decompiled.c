/* DWARF-guided pseudocode for game/client/portal2/c_info_placement_helper.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* ClientClassInit<DT_InfoPlacementHelper::ignored> at 00073e60 */

/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "ComboBoxButton": ignoring overlapping field "m_DisabledBgColor" */
/* WARNING: Struct "ComboBox": ignoring multiple overlapping fields */
/* WARNING: Struct "PageTab": ignoring multiple overlapping fields */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

int ClientClassInit<DT_InfoPlacementHelper::ignored>(ignored *param_1)

{
  RecvVarProxyFn p_Var1;
  int iVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x56e].m_SerialNumber +
               unaff_EBX + 2) == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xc4384a);
    if (iVar2 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x56e].m_pNext +
                  unaff_EBX + 2),(char *)(unaff_EBX + 0x8ebef2),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        (&UNK_0000572e +
                        (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                        (char *)(vgui::PageTab::GetMessageMap + unaff_EBX + 2),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00abcf76 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00abcf32 + unaff_EBX));
      p_Var1 = *(RecvVarProxyFn *)(&DAT_00abd152 + unaff_EBX);
      RecvPropString((RecvProp_conflict1 *)
                     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x576].m_SerialNumber +
                     unaff_EBX + 2),(char *)(unaff_EBX + 0x91ad4d),0xa5c,4,0,p_Var1);
      RecvPropString((RecvProp_conflict1 *)
                     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x57a].m_pEntity +
                     unaff_EBX + 2),(char *)(unaff_EBX + 0x91ad5e),0xa60,4,0,p_Var1);
      p_Var1 = *(RecvVarProxyFn *)(&DAT_00abcf72 + unaff_EBX);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x57d].m_pNext +
                    unaff_EBX + 2),&UNK_008ec2fd + unaff_EBX,0xa64,4,0,p_Var1);
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x581].m_pPrev +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0x91ad70),0xa68,1);
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x585].m_SerialNumber +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0x91ad86),0xa69,1);
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x589].m_pEntity +
                   unaff_EBX + 2),&UNK_0091c94e + unaff_EBX,0xa6a,1);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x58c].m_pNext +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x91ad98),0xa6c,4,0,p_Var1);
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x590].m_pPrev +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0x91ada8),0xa70,1);
      ___cxa_guard_release(unaff_EBX + 0xc4384a);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x13aa),
             (RecvProp_conflict1 *)
             (&UNK_0000572e + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4
             ),9,&UNK_0091adbd + unaff_EBX);
  return 1;
}


/* __static_initialization_and_destruction_0 at 00074110 */

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
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x53f].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x53f].m_pNext + unaff_EBX)
       = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x53f].m_pNext + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x53f].m_pNext + unaff_EBX + 2) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x53f].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x540].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x540].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x540].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x540].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x541].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x541].m_SerialNumber + unaff_EBX + 3) =
       0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x541].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x541].m_pNext + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x542].m_pEntity + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x542].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x542].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined **)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x542].m_pNext + unaff_EBX + 3) =
       &UNK_00abdce7 + unaff_EBX;
  *(int *)((int)s_pParticlePtrs + (int)(&DAT_000010e7 + unaff_EBX)) = unaff_EBX + 0x91ab21;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x10df) = unaff_EBX + 0x4ad73f;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0x10e3) = 0;
  this = (RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x10f7);
  *(RecvTable **)((int)s_pParticlePtrs + (int)(&UNK_000010eb + unaff_EBX)) = this;
  piVar1 = *(int **)(&DAT_00abcc77 + unaff_EBX);
  *(int *)((int)s_pParticlePtrs + (int)(&UNK_000010ef + unaff_EBX)) = *piVar1;
  *piVar1 = unaff_EBX + 0xcda93f;
  RecvTable::RecvTable(this);
  uVar2 = *(undefined4 *)(&DAT_00abcbff + unaff_EBX);
  ___cxa_atexit(unaff_EBX + 0x8dd00f,0,uVar2);
  iVar3 = ClientClassInit<DT_InfoPlacementHelper::ignored>((ignored *)0x0);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x110b) = iVar3;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0x110f) = 0;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0x1113) = 0;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0x1117) = 0;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0x111b) = 0;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0x111f) = 0;
  ___cxa_atexit(unaff_EBX + 0x8dd02f,0,uVar2);
  return;
}


/* C_InfoPlacementHelper::YouForgotToImplementOrDeclareClientClass at 00521370 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_InfoPlacementHelper *
   this) */

int __thiscall
C_InfoPlacementHelper::YouForgotToImplementOrDeclareClientClass(C_InfoPlacementHelper *this)

{
  return 0;
}


/* C_InfoPlacementHelper::GetClientClass at 00521390 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: ClientClass * GetClientClass(C_InfoPlacementHelper * this) */

ClientClass * __thiscall C_InfoPlacementHelper::GetClientClass(C_InfoPlacementHelper *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x82d6c8);
}


/* _C_InfoPlacementHelper_CreateObject at 00521860 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_InfoPlacementHelper_CreateObject(int entnum,int serialNum)

{
  C_BaseEntity *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_InfoPlacementHelper * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xa74);
  C_BaseEntity::C_BaseEntity(this);
  (this->super_IClientEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(&UNK_00673514 + unaff_EBX);
  (this->super_IClientEntity).super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6738ac);
  (this->super_IClientEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x673964);
  (this->super_IClientEntity).super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(&UNK_006739a0 + unaff_EBX);
  *(undefined1 *)&this[1].super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable = 0;
  *(undefined1 *)
   ((int)&this[1].super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable + 1) = 0;
  this[1].m_iClassname = (string_t)0x0;
  *(undefined1 *)&this[1].m_hScriptInstance = 0;
  (**(code **)(&UNK_006735d0 + unaff_EBX))(this,entnum,serialNum);
  return &(this->super_IClientEntity).super_IClientNetworkable;
}


/* __tcf_0 at 00951130 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3fd940),in_stack_00000008);
  return;
}


/* __tcf_1 at 00951150 */

void __tcf_1(void *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  *(undefined4 *)(unaff_EBX + 0x3fd938) = 0;
  if (*(int *)(unaff_EBX + 0x3fd934) < 0) {
    iVar1 = *(int *)(unaff_EBX + 0x3fd92c);
  }
  else {
    if (*(int *)(unaff_EBX + 0x3fd92c) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1dfbc0) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1dfbc0),*(int *)(unaff_EBX + 0x3fd92c));
      *(undefined4 *)(unaff_EBX + 0x3fd92c) = 0;
    }
    iVar1 = 0;
    *(undefined4 *)(unaff_EBX + 0x3fd930) = 0;
  }
  *(int *)(unaff_EBX + 0x3fd93c) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x3fd934)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x1dfbc0) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x1dfbc0),iVar1);
      *(undefined4 *)(unaff_EBX + 0x3fd92c) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x3fd930) = 0;
  }
  return;
}


/* UTIL_FindPlacementHelper at 00521690 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

C_InfoPlacementHelper * UTIL_FindPlacementHelper(Vector *vecEndPoint,C_BasePlayer *pPlayer)

{
  C_InfoPlacementHelper *pCVar1;
  float *pfVar2;
  int unaff_EBX;
  int iVar3;
  longdouble lVar4;
  longdouble lVar5;
  float fVar6;
  C_InfoPlacementHelper *local_58;
  float local_54;
  Vector local_40;
  Vector local_34;
  Vector local_28 [2];
  
                    /* Unresolved local var: Vector vecEyeDir@[???]
                       Unresolved local var: Vector vecEyePos@[???]
                       Unresolved local var: C_InfoPlacementHelper * pBestHelper@[???]
                       Unresolved local var: float flBestDist@[???] */
  ___i686_get_pc_thunk_bx();
  if (pPlayer != (C_BasePlayer *)0x0) {
    C_BasePlayer::EyePositionAndVectors(pPlayer,&local_34,local_28,(Vector *)0x0,(Vector *)0x0);
    if (0 < *(int *)(unaff_EBX + 0x82d3fb)) {
      local_58 = (C_InfoPlacementHelper *)0x0;
      local_54 = *(float *)(unaff_EBX + 0x499f23);
      iVar3 = 0;
                    /* Unresolved local var: C_InfoPlacementHelper * pHelper@[???]
                       Unresolved local var: Vector vecTargetDir@[???]
                       Unresolved local var: float flTargetDist@[???]
                       Unresolved local var: float flToleranceAngle@[???]
                       Unresolved local var: float flTargetAngle@[???] */
      do {
        pCVar1 = *(C_InfoPlacementHelper **)(*(int *)(unaff_EBX + 0x82d3ef) + iVar3 * 4);
        if ((((pCVar1 != (C_InfoPlacementHelper *)0x0) && (pCVar1->m_bDisabled == false)) &&
            (pCVar1->m_bDeferringToPortal == false)) &&
           ((pCVar1->m_bForcePlacement != false ||
            (fVar6 = *(float *)(**(int **)(unaff_EBX + 0x60f6af) + 0xc),
            pCVar1->m_flDisableTime <= fVar6 && fVar6 != pCVar1->m_flDisableTime)))) {
          pfVar2 = (float *)(*(pCVar1->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.
                              super_IHandleEntity._vptr_IHandleEntity[0xd])(pCVar1);
                    /* Unresolved local var: Vector res@[???] */
          local_40.x = *pfVar2 - local_34.x;
          local_40.y = pfVar2[1] - local_34.y;
          local_40.z = pfVar2[2] - local_34.z;
          fVar6 = VectorNormalize(&local_40);
          lVar4 = (longdouble)_atan2((double)pCVar1->m_flRadius,(double)fVar6);
          lVar5 = (longdouble)
                  _acosf(local_28[0].x * local_40.x + local_28[0].y * local_40.y +
                         local_28[0].z * local_40.z);
          if (((float)lVar5 <= (float)lVar4) && (fVar6 <= local_54)) {
            local_58 = pCVar1;
            local_54 = fVar6;
          }
        }
        iVar3 = iVar3 + 1;
      } while (iVar3 < *(int *)(unaff_EBX + 0x82d3fb));
      return local_58;
    }
  }
  return (C_InfoPlacementHelper *)0x0;
}


/* C_InfoPlacementHelper::C_InfoPlacementHelper at 005214d0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void C_InfoPlacementHelper(C_InfoPlacementHelper * this) */

void __thiscall C_InfoPlacementHelper::C_InfoPlacementHelper(C_InfoPlacementHelper *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::C_BaseEntity(&this->super_C_BaseEntity);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6738a7);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x673c3f);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x673cf7);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x673d33);
  this->m_bSnapToHelperAngles = false;
  this->m_bForcePlacement = false;
  this->m_flDisableTime = 0.0;
  this->m_bDeferringToPortal = false;
  return;
}


/* C_InfoPlacementHelper::C_InfoPlacementHelper at 00521550 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void C_InfoPlacementHelper(C_InfoPlacementHelper * this) */

void __thiscall C_InfoPlacementHelper::C_InfoPlacementHelper(C_InfoPlacementHelper *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::C_BaseEntity(&this->super_C_BaseEntity);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x673827);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(&UNK_00673bbf + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x673c77);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x673cb3);
  this->m_bSnapToHelperAngles = false;
  this->m_bForcePlacement = false;
  this->m_flDisableTime = 0.0;
  this->m_bDeferringToPortal = false;
  return;
}


/* C_InfoPlacementHelper::Spawn at 005215d0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void Spawn(C_InfoPlacementHelper * this) */

void __thiscall C_InfoPlacementHelper::Spawn(C_InfoPlacementHelper *this)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined4 *puVar4;
  int iVar5;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::Spawn(&this->super_C_BaseEntity);
  iVar2 = *(int *)(unaff_EBX + 0x82d4b8);
  iVar1 = iVar2 + 1;
  iVar3 = iVar1;
  if (*(int *)(unaff_EBX + 0x82d4b0) < iVar1) {
    CUtlMemory<C_InfoPlacementHelper*,int>::Grow
              ((CUtlMemory<C_InfoPlacementHelper*,int> *)(unaff_EBX + 0x82d4ac),
               iVar1 - *(int *)(unaff_EBX + 0x82d4b0));
    iVar3 = *(int *)(unaff_EBX + 0x82d4b8) + 1;
  }
  *(int *)(unaff_EBX + 0x82d4b8) = iVar3;
  iVar5 = *(int *)(unaff_EBX + 0x82d4ac);
  *(int *)(unaff_EBX + 0x82d4bc) = iVar5;
  iVar3 = (iVar3 - iVar2) + -1;
  if (0 < iVar3) {
    _V_memmove((void *)(iVar5 + iVar1 * 4),(void *)(iVar5 + iVar2 * 4),iVar3 * 4);
    iVar5 = *(int *)(unaff_EBX + 0x82d4ac);
  }
  puVar4 = (undefined4 *)(iVar2 * 4 + iVar5);
  if (puVar4 != (undefined4 *)0x0) {
    *puVar4 = this;
  }
  return;
}


/* C_InfoPlacementHelper::UpdateOnRemove at 00521430 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void UpdateOnRemove(C_InfoPlacementHelper * this) */

void __thiscall C_InfoPlacementHelper::UpdateOnRemove(C_InfoPlacementHelper *this)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::UpdateOnRemove(&this->super_C_BaseEntity);
  iVar1 = *(int *)(unaff_EBX + 0x82d65e);
  if (iVar1 < 1) {
    return;
  }
  piVar2 = *(int **)(unaff_EBX + 0x82d652);
  if (this == (C_InfoPlacementHelper *)*piVar2) {
    iVar4 = 0;
    iVar5 = 0;
  }
  else {
    iVar4 = 0;
    iVar3 = 4;
    do {
      iVar5 = iVar3;
      iVar4 = iVar4 + 1;
      if (iVar4 == iVar1) {
        return;
      }
      iVar3 = iVar5 + 4;
    } while (this != (C_InfoPlacementHelper *)piVar2[iVar4]);
  }
  if (iVar4 != iVar1 + -1) {
    *(int *)(iVar5 + (int)piVar2) = piVar2[iVar1 + -1];
    iVar4 = *(int *)(unaff_EBX + 0x82d65e) + -1;
  }
  *(int *)(unaff_EBX + 0x82d65e) = iVar4;
  return;
}


/* C_InfoPlacementHelper::ShouldUseHelperAngles at 005213a0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: bool ShouldUseHelperAngles(C_InfoPlacementHelper * this) */

bool __thiscall C_InfoPlacementHelper::ShouldUseHelperAngles(C_InfoPlacementHelper *this)

{
  return this->m_bSnapToHelperAngles;
}


/* C_InfoPlacementHelper::IsEnabled at 005213b0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: bool IsEnabled(C_InfoPlacementHelper * this) */

bool __thiscall C_InfoPlacementHelper::IsEnabled(C_InfoPlacementHelper *this)

{
  float fVar1;
  bool bVar2;
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  if ((this->m_bDisabled == false) && (this->m_bDeferringToPortal == false)) {
    bVar2 = true;
    if (this->m_bForcePlacement == false) {
      fVar1 = *(float *)(**(int **)(extraout_ECX + 0x60f998) + 0xc);
      bVar2 = this->m_flDisableTime <= fVar1 && fVar1 != this->m_flDisableTime;
    }
    return bVar2;
  }
  return false;
}


/* C_InfoPlacementHelper::BindToPortal at 00521410 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void BindToPortal(C_InfoPlacementHelper * this, C_Portal_Base2D *
   pPortal) */

void __thiscall
C_InfoPlacementHelper::BindToPortal(C_InfoPlacementHelper *this,C_Portal_Base2D *pPortal)

{
  if ((pPortal != (C_Portal_Base2D *)0x0) && (this->m_bForcePlacement == false)) {
    this->m_bDeferringToPortal = true;
  }
  return;
}


/* _GLOBAL__I__ZN21C_InfoPlacementHelper17m_pClassRecvTableE at 00074270 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN21C_InfoPlacementHelper17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

