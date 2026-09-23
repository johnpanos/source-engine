/* DWARF-guided pseudocode for game/server/portal2/portal_pointpush.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000b1920 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *s2;
  SendTable *this;
  undefined4 *puVar1;
  datamap_t *pdVar2;
  int iVar3;
  int iVar4;
  IEntityFactoryDictionary *pIVar5;
  int unaff_EBX;
  undefined4 *puVar6;
  undefined4 *puVar7;
  longlong lVar8;
  
  lVar8 = ___i686_get_pc_thunk_bx();
  if (lVar8 == 0xffff00000001) {
    *(undefined1 *)((int)&debug_visibility_monitor.m_Value.m_fValue + unaff_EBX) = 0;
    *(undefined1 *)((int)&debug_visibility_monitor.m_Value.m_fValue + unaff_EBX + 1) = 0;
    *(undefined1 *)((int)&debug_visibility_monitor.m_Value.m_fValue + unaff_EBX + 2) = 0;
    *(undefined1 *)((int)&debug_visibility_monitor.m_Value.m_fValue + unaff_EBX + 3) = 0;
    *(undefined4 *)((int)&debug_visibility_monitor.m_Value.m_nValue + unaff_EBX) = 0;
    *(undefined4 *)(&debug_visibility_monitor.m_bHasMin + unaff_EBX) = 0;
    *(undefined4 *)((int)&debug_visibility_monitor.m_fMinVal + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&debug_visibility_monitor.m_bHasMax + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&debug_visibility_monitor.m_fMaxVal + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&debug_visibility_monitor.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&debug_visibility_monitor.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) =
         0;
    *(undefined4 *)
     ((int)&debug_visibility_monitor.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX) = 0;
    *(undefined4 *)((int)&debug_visibility_monitor.m_fnChangeCallbacks.m_Size + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)((int)&debug_visibility_monitor.m_fnChangeCallbacks.m_pElements + unaff_EBX) =
         0x7f7fffff;
    *(undefined4 *)(&DAT_00d9d938 + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00d9d93c + unaff_EBX) = 0x7f7fffff;
    *(undefined **)
     ((int)&vismon_poll_frequency.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) =
         &UNK_00b9a9d4 + unaff_EBX;
    CAutoGameSystem::CAutoGameSystem
              ((CAutoGameSystem *)
               ((int)DataMapInit<CAI_FollowBehavior>::dataDesc[0x16].flatOffset + unaff_EBX + -0x28)
               ,(char *)0x0);
    *(undefined **)
     ((int)DataMapInit<CAI_FollowBehavior>::dataDesc[0x16].flatOffset + unaff_EBX + -0x28) =
         &UNK_00c1e8d4 + unaff_EBX;
    *(undefined4 *)
     ((int)DataMapInit<CAI_FollowBehavior>::dataDesc[0x16].flatOffset + unaff_EBX + -0x1c) = 0;
    *(undefined4 *)
     ((int)DataMapInit<CAI_FollowBehavior>::dataDesc[0x16].flatOffset + unaff_EBX + -0x18) = 0;
    *(undefined4 *)
     ((int)DataMapInit<CAI_FollowBehavior>::dataDesc[0x16].flatOffset + unaff_EBX + -0x14) = 0;
    *(undefined4 *)
     ((int)DataMapInit<CAI_FollowBehavior>::dataDesc[0x16].flatOffset + unaff_EBX + -0x10) = 0;
    *(undefined4 *)
     ((int)DataMapInit<CAI_FollowBehavior>::dataDesc[0x16].flatOffset + unaff_EBX + -0xc) = 0;
    ___cxa_atexit(unaff_EBX + 0x98d0bc,0,*(undefined4 *)(&DAT_00b95220 + unaff_EBX));
    pdVar2 = DataMapInit<CPortal_PointPush>((CPortal_PointPush *)0x0);
    *(datamap_t **)
     ((int)DataMapInit<CAI_FollowBehavior>::dataDesc[0x16].flatOffset + unaff_EBX + -8) = pdVar2;
    iVar4 = unaff_EBX + 0xd9d944;
    s2 = (char *)(unaff_EBX + 0xa017a0);
    *(char **)((int)&vismon_poll_frequency.super_ConCommandBase.m_pNext + unaff_EBX) = s2;
    this = (SendTable *)
           ((int)DataMapInit<CAI_FollowBehavior>::dataDesc[0x16].flatOffset + unaff_EBX + -4);
    *(SendTable **)(&vismon_poll_frequency.super_ConCommandBase.m_bRegistered + unaff_EBX) = this;
    *(undefined4 *)((int)&vismon_poll_frequency.super_ConCommandBase.m_nFlags + unaff_EBX) = 0xffff;
    puVar7 = (undefined4 *)**(int **)(&DAT_00b952c0 + unaff_EBX);
    if (puVar7 == (undefined4 *)0x0) {
      **(int **)(&DAT_00b952c0 + unaff_EBX) = iVar4;
      *(undefined4 *)((int)&vismon_poll_frequency.super_ConCommandBase.m_pszName + unaff_EBX) = 0;
    }
    else {
      puVar1 = (undefined4 *)puVar7[2];
      iVar3 = _V_stricmp((char *)*puVar7,s2);
      if (iVar3 < 1) {
        while ((puVar6 = puVar1, puVar6 != (undefined4 *)0x0 &&
               (iVar3 = _V_stricmp((char *)*puVar6,s2), iVar3 < 1))) {
          puVar1 = (undefined4 *)puVar6[2];
          puVar7 = puVar6;
        }
        *(undefined4 **)((int)&vismon_poll_frequency.super_ConCommandBase.m_pszName + unaff_EBX) =
             puVar6;
        puVar7[2] = iVar4;
      }
      else {
        *(undefined4 *)((int)&vismon_poll_frequency.super_ConCommandBase.m_pszName + unaff_EBX) =
             **(undefined4 **)(&DAT_00b952c0 + unaff_EBX);
        **(int **)(&DAT_00b952c0 + unaff_EBX) = iVar4;
      }
    }
    SendTable::SendTable(this);
    ___cxa_atexit(unaff_EBX + 0x98cfbc,0,*(undefined4 *)(&DAT_00b95220 + unaff_EBX));
    iVar4 = ServerClassInit<DT_PortalPointPush::ignored>((ignored *)0x0);
    *(int *)((int)DataMapInit<CAI_FollowBehavior>::dataDesc[0x17].flatOffset + unaff_EBX + -0x30) =
         iVar4;
    *(undefined **)((int)&vismon_poll_frequency.super_IConVar._vptr_IConVar + unaff_EBX) =
         &UNK_00c1e944 + unaff_EBX;
    pIVar5 = EntityFactoryDictionary();
    (**pIVar5->_vptr_IEntityFactoryDictionary)(pIVar5,unaff_EBX + 0xd9d958,unaff_EBX + 0xa017c3);
  }
  return;
}


/* CPortalVortexEffectManager::~CPortalVortexEffectManager at 007467a0 */

/* DWARF original prototype: void ~CPortalVortexEffectManager(CPortalVortexEffectManager * this, int
   __in_chrg) */

void __thiscall
CPortalVortexEffectManager::~CPortalVortexEffectManager
          (CPortalVortexEffectManager *this,int __in_chrg)

{
  PortalVortexEffect_t *pPVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(unaff_EBX + 0x589a54);
  (this->m_Effects).m_Size = 0;
  if ((this->m_Effects).m_Memory.m_nGrowSize < 0) {
    pPVar1 = (this->m_Effects).m_Memory.m_pMemory;
  }
  else {
    pPVar1 = (this->m_Effects).m_Memory.m_pMemory;
    if (pPVar1 != (PortalVortexEffect_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5003a8) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x5003a8),pPVar1);
      (this->m_Effects).m_Memory.m_pMemory = (PortalVortexEffect_t *)0x0;
    }
    pPVar1 = (PortalVortexEffect_t *)0x0;
    (this->m_Effects).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_Effects).m_pElements = pPVar1;
  if (-1 < (this->m_Effects).m_Memory.m_nGrowSize) {
    if (pPVar1 != (PortalVortexEffect_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5003a8) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x5003a8),pPVar1);
      (this->m_Effects).m_Memory.m_pMemory = (PortalVortexEffect_t *)0x0;
    }
    (this->m_Effects).m_Memory.m_nAllocationCount = 0;
  }
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(unaff_EBX + 0x5028d4);
  IGameSystem::~IGameSystem((IGameSystem *)this,__in_chrg);
  return;
}


/* CPortalVortexEffectManager::~CPortalVortexEffectManager at 007468a0 */

/* DWARF original prototype: void ~CPortalVortexEffectManager(CPortalVortexEffectManager * this, int
   __in_chrg) */

void __thiscall
CPortalVortexEffectManager::~CPortalVortexEffectManager
          (CPortalVortexEffectManager *this,int __in_chrg)

{
  PortalVortexEffect_t *pPVar1;
  int unaff_EBX;
  PortalVortexEffect_t *in_stack_ffffffc8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(&UNK_00589954 + unaff_EBX);
  (this->m_Effects).m_Size = 0;
  if ((this->m_Effects).m_Memory.m_nGrowSize < 0) {
    pPVar1 = (this->m_Effects).m_Memory.m_pMemory;
  }
  else {
    pPVar1 = (this->m_Effects).m_Memory.m_pMemory;
    if (pPVar1 != (PortalVortexEffect_t *)0x0) {
      in_stack_ffffffc8 = pPVar1;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5002a8) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x5002a8));
      pPVar1 = (PortalVortexEffect_t *)0x0;
      (this->m_Effects).m_Memory.m_pMemory = (PortalVortexEffect_t *)0x0;
    }
    (this->m_Effects).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_Effects).m_pElements = pPVar1;
  if (-1 < (this->m_Effects).m_Memory.m_nGrowSize) {
    if (pPVar1 != (PortalVortexEffect_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5002a8) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x5002a8));
      (this->m_Effects).m_Memory.m_pMemory = (PortalVortexEffect_t *)0x0;
      in_stack_ffffffc8 = pPVar1;
    }
    (this->m_Effects).m_Memory.m_nAllocationCount = 0;
  }
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(unaff_EBX + 0x5027d4);
  IGameSystem::~IGameSystem((IGameSystem *)this,(int)in_stack_ffffffc8);
  operator_delete(this);
  return;
}


/* __tcf_2 at 00a3e9f0 */

void __tcf_2(void *param_1)

{
  int unaff_EBX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_bx();
  *(int *)(unaff_EBX + 0x38aafc) = unaff_EBX + 0x291804;
  *(undefined4 *)(unaff_EBX + 0x38ab14) = 0;
  if (*(int *)(unaff_EBX + 0x38ab10) < 0) {
    *(undefined4 *)(unaff_EBX + 0x38ab18) = *(undefined4 *)(unaff_EBX + 0x38ab08);
  }
  else if (*(int *)(unaff_EBX + 0x38ab08) == 0) {
    *(undefined4 *)(unaff_EBX + 0x38ab0c) = 0;
    *(undefined4 *)(unaff_EBX + 0x38ab18) = 0;
    *(undefined4 *)(unaff_EBX + 0x38ab0c) = 0;
  }
  else {
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x208158) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x208158),*(int *)(unaff_EBX + 0x38ab08));
    *(undefined4 *)(unaff_EBX + 0x38ab08) = 0;
    *(undefined4 *)(unaff_EBX + 0x38ab0c) = 0;
    *(undefined4 *)(unaff_EBX + 0x38ab18) = 0;
    if (-1 < *(int *)(unaff_EBX + 0x38ab10)) {
      *(undefined4 *)(unaff_EBX + 0x38ab0c) = 0;
    }
  }
  *(int *)(unaff_EBX + 0x38aafc) = unaff_EBX + 0x20a684;
  IGameSystem::~IGameSystem((IGameSystem *)(unaff_EBX + 0x38aafc),in_stack_00000008);
  return;
}


/* CPortal_PointPush::CreatePortalVortexEffect at 00745ff0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CreatePortalVortexEffect(CPortal_PointPush * this, Vector *
   vecOrigin, CPortal_Base2D * pPortal) */

void __thiscall
CPortal_PointPush::CreatePortalVortexEffect
          (CPortal_PointPush *this,Vector *vecOrigin,CPortal_Base2D *pPortal)

{
  uint uVar1;
  float fVar2;
  undefined4 uVar3;
  int iVar4;
  float *pfVar5;
  int iVar6;
  ulong *puVar7;
  CBaseEntity *this_00;
  undefined4 *puVar8;
  int iVar9;
  undefined4 uVar10;
  int iVar11;
  int unaff_EBX;
  float fVar12;
  float fVar13;
  float fVar14;
  undefined4 local_a4;
  CPortal_Base2D *local_94;
  int local_90;
  Vector local_60;
  vec_t local_54;
  vec_t local_50;
  vec_t local_4c;
  float local_48;
  float local_44;
  float local_40;
  QAngle local_3c;
  Vector local_30;
  EHANDLE local_24;
  EHANDLE local_20 [4];
  
                    /* Unresolved local var: CPortal_Base2D * pPartnerPortal@[???]
                       Unresolved local var: float flDistFromOriginSqr@[???]
                       Unresolved local var: float flMinRadiusSqr@[???] */
  ___i686_get_pc_thunk_bx();
  uVar1 = (pPortal->m_hLinkedPortal).
          super_CNetworkVarBase<CBaseHandle,CPortal_Base2D::NetworkVar_m_hLinkedPortal>.m_Value.
          m_Index;
  if ((uVar1 == 0xffffffff) ||
     (iVar4 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x500b6b),
     *(uint *)(iVar4 + 8) != uVar1 >> 0x10)) {
    local_94 = (CPortal_Base2D *)0x0;
  }
  else {
    local_94 = *(CPortal_Base2D **)(iVar4 + 4);
  }
  pfVar5 = (float *)(*(pPortal->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.
                      super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x9d])(pPortal);
                    /* Unresolved local var: Vector res@[???] */
  fVar12 = *pfVar5 - vecOrigin->x;
  fVar13 = pfVar5[1] - vecOrigin->y;
  fVar14 = *(float *)(&DAT_00386aff + unaff_EBX) * (this->m_flRadius).m_Value;
  if (fVar14 * fVar14 <= fVar12 * fVar12 + fVar13 * fVar13) {
    return;
  }
  fVar12 = (this->m_flMagnitude).m_Value;
                    /* Unresolved local var: int itr@[???] */
  if (0 < *(int *)(unaff_EBX + 0x683517)) {
    iVar4 = 0;
    do {
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???]
                       Unresolved local var: CEntInfo * pInfo@[???] */
      uVar1 = *(uint *)(*(int *)(unaff_EBX + 0x68350b) + 4 + iVar4 * 8);
      if (((uVar1 != 0xffffffff) &&
          (iVar6 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x500b6b),
          *(uint *)(iVar6 + 8) == uVar1 >> 0x10)) && (pPortal == *(CPortal_Base2D **)(iVar6 + 4)))
      goto LAB_007460ee;
      iVar4 = iVar4 + 1;
    } while (iVar4 != *(int *)(unaff_EBX + 0x683517));
  }
  this_00 = CreateEntityByName((char *)(unaff_EBX + 0x351c6a),-1,true);
  local_a4 = 0xffffffff;
  if (this_00 != (CBaseEntity *)0x0) {
    (*(this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
      [0x20])(this_00,unaff_EBX + 0x35dacb,unaff_EBX + 0x310c7b);
    (*(this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
      [0x20])(this_00,unaff_EBX + 0x351c9a,unaff_EBX + 0x36d0e5);
    (*(this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
      [0x26])(this_00,pPortal,0xffffffff);
    if (((pPortal->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)pPortal);
    }
    fVar13 = (pPortal->super_CBaseAnimating).super_CBaseEntity.m_rgflCoordinateFrame.m_flMatVal[2]
             [0];
    fVar14 = (pPortal->super_CBaseAnimating).super_CBaseEntity.m_rgflCoordinateFrame.m_flMatVal[1]
             [0];
    fVar2 = (pPortal->super_CBaseAnimating).super_CBaseEntity.m_rgflCoordinateFrame.m_flMatVal[0][0]
    ;
    local_48 = fVar2;
    local_44 = fVar14;
    local_40 = fVar13;
    pfVar5 = (float *)(*(pPortal->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.
                        super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x9d])(pPortal)
    ;
    local_30.x = *pfVar5;
    local_30.y = pfVar5[1];
    local_30.z = pfVar5[2];
    if (((pPortal->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)pPortal);
    }
    local_3c.x = (pPortal->super_CBaseAnimating).super_CBaseEntity.m_angAbsRotation.x;
    local_3c.y = (pPortal->super_CBaseAnimating).super_CBaseEntity.m_angAbsRotation.y;
    local_3c.z = (pPortal->super_CBaseAnimating).super_CBaseEntity.m_angAbsRotation.z;
    if (fVar12 < *(float *)(&DAT_00386b6f + unaff_EBX) ||
        fVar12 == *(float *)(&DAT_00386b6f + unaff_EBX)) {
                    /* Unresolved local var: Vector res@[???] */
      fVar12 = *(float *)(unaff_EBX + 0x38f963);
      local_30.x = local_30.x + fVar2 * fVar12;
      local_30.y = local_30.y + fVar14 * fVar12;
      local_30.z = local_30.z + fVar12 * fVar13;
    }
    else {
      uVar1 = *(uint *)(unaff_EBX + 0x38f96f);
      local_60.x = (vec_t)((uint)fVar2 ^ uVar1);
      local_60.y = (vec_t)((uint)fVar14 ^ uVar1);
      local_60.z = (vec_t)((uint)fVar13 ^ uVar1);
      local_54 = local_60.x;
      local_50 = local_60.y;
      local_4c = local_60.z;
      VectorAngles(&local_60,&local_3c);
                    /* Unresolved local var: Vector res@[???] */
      fVar12 = *(float *)(&DAT_00386c1b + unaff_EBX);
      local_30.x = local_30.x + fVar2 * fVar12;
      local_30.y = local_30.y + fVar14 * fVar12;
      local_30.z = local_30.z + fVar12 * fVar13;
    }
    CBaseEntity::SetAbsOrigin(this_00,&local_30);
    CBaseEntity::SetAbsAngles(this_00,&local_3c);
    *(undefined1 *)
     &this_00[1].super_IServerEntity.super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
         1;
    DispatchSpawn(this_00,true);
    (*(this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
      [0x25])(this_00);
    puVar8 = (undefined4 *)
             (*(this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
               _vptr_IHandleEntity[3])(this_00);
    local_a4 = *puVar8;
  }
  puVar8 = (undefined4 *)
           (*(pPortal->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.
             super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[3])(pPortal);
  uVar3 = *puVar8;
  iVar6 = *(int *)(unaff_EBX + 0x683517);
  iVar4 = iVar6 + 1;
  iVar9 = *(int *)(unaff_EBX + 0x68350f);
  if (iVar4 <= iVar9) goto LAB_0074647d;
  iVar11 = *(int *)(unaff_EBX + 0x683513);
  if (iVar11 < 0) goto LAB_0074647d;
  if (iVar11 == 0) {
    if ((iVar9 == 0) && (iVar9 = 4, iVar4 < 5)) {
      local_90 = 0x20;
    }
    else {
      do {
        local_90 = iVar9;
        iVar9 = local_90 * 2;
      } while (iVar9 < iVar4);
      local_90 = local_90 * 0x10;
    }
    goto LAB_00746430;
  }
  iVar9 = (iVar6 / iVar11 + 1) * iVar11;
  if (iVar9 < iVar4) {
    if (iVar9 != 0) {
LAB_00746413:
      do {
        iVar9 = (iVar9 + iVar4) / 2;
      } while (iVar9 < iVar4);
      goto LAB_00746423;
    }
    if (iVar4 < 0) {
      iVar9 = -1;
      local_90 = -8;
    }
    else {
      local_90 = 0;
      if (iVar4 != 0) goto LAB_00746413;
    }
  }
  else {
LAB_00746423:
    local_90 = iVar9 * 8;
  }
LAB_00746430:
  *(int *)(unaff_EBX + 0x68350f) = iVar9;
  if (*(int *)(unaff_EBX + 0x68350b) == 0) {
    uVar10 = (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x500b5b))
                       ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x500b5b),local_90);
    *(undefined4 *)(unaff_EBX + 0x68350b) = uVar10;
  }
  else {
    uVar10 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x500b5b) + 4))
                       ((int *)**(undefined4 **)(unaff_EBX + 0x500b5b),
                        *(int *)(unaff_EBX + 0x68350b),local_90);
    *(undefined4 *)(unaff_EBX + 0x68350b) = uVar10;
  }
LAB_0074647d:
  iVar11 = *(int *)(unaff_EBX + 0x683517) + 1;
  *(int *)(unaff_EBX + 0x683517) = iVar11;
  iVar9 = *(int *)(unaff_EBX + 0x68350b);
  *(int *)(unaff_EBX + 0x68351b) = iVar9;
  iVar11 = (iVar11 - iVar6) + -1;
  if (0 < iVar11) {
    _V_memmove((void *)(iVar9 + iVar4 * 8),(void *)(iVar9 + iVar6 * 8),iVar11 * 8);
  }
  puVar8 = (undefined4 *)(iVar6 * 8 + *(int *)(unaff_EBX + 0x68350b));
  if (puVar8 != (undefined4 *)0x0) {
    *puVar8 = local_a4;
    puVar8[1] = uVar3;
  }
LAB_007460ee:
  local_20[0].super_CBaseHandle.m_Index = (CBaseHandle)0xffffffff;
  puVar7 = (ulong *)(*(this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.
                      super_IHandleEntity._vptr_IHandleEntity[3])(this);
  local_20[0].super_CBaseHandle.m_Index = (CBaseHandle)*puVar7;
  CPortal_Base2D::AddPortalEventListener(pPortal,local_20);
  local_24.super_CBaseHandle.m_Index = (CBaseHandle)0xffffffff;
  puVar7 = (ulong *)(*(this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.
                      super_IHandleEntity._vptr_IHandleEntity[3])(this);
  local_24.super_CBaseHandle.m_Index = (CBaseHandle)*puVar7;
  CPortal_Base2D::AddPortalEventListener(local_94,&local_24);
  return;
}


/* CPortal_PointPush::NotifyPortalEvent at 00745190 */

/* DWARF original prototype: void NotifyPortalEvent(CPortal_PointPush * this, PortalEvent_t
   nEventType, CPortal_Base2D * pNotifier) */

void __thiscall
CPortal_PointPush::NotifyPortalEvent
          (CPortal_PointPush *this,PortalEvent_t nEventType,CPortal_Base2D *pNotifier)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  ulong *puVar4;
  int iVar5;
  CBaseEntity *oldObj;
  uint uVar6;
  CPortal_Base2D *this_00;
  int unaff_EBX;
  uint *puVar7;
  uint local_30;
  CBaseHandle local_24;
  CBaseHandle local_20 [4];
  
  ___i686_get_pc_thunk_bx();
  if (nEventType - PORTALEVENT_FIZZLE < 2) {
    if (0 < (int)*(uint *)(unaff_EBX + 0x68437a)) {
      iVar5 = **(int **)(unaff_EBX + 0x5019ce);
      puVar7 = *(uint **)(unaff_EBX + 0x68436e);
      local_30 = 0;
      do {
        uVar6 = puVar7[1];
        if ((uVar6 != 0xffffffff) &&
           (iVar2 = (uVar6 & 0xffff) * 0x10 + iVar5, *(uint *)(iVar2 + 8) == uVar6 >> 0x10)) {
          if (pNotifier != *(CPortal_Base2D **)(iVar2 + 4)) goto LAB_00745213;
LAB_0074534c:
          uVar6 = *puVar7;
          if ((uVar6 == 0xffffffff) ||
             (iVar5 = (uVar6 & 0xffff) * 0x10 + iVar5, *(uint *)(iVar5 + 8) != uVar6 >> 0x10)) {
            oldObj = (CBaseEntity *)0x0;
          }
          else {
            oldObj = *(CBaseEntity **)(iVar5 + 4);
          }
          UTIL_Remove(oldObj);
          iVar5 = ~local_30 + *(int *)(unaff_EBX + 0x68437a);
          if (0 < iVar5) {
            _V_memmove((void *)(local_30 * 8 + *(int *)(unaff_EBX + 0x68436e)),
                       (void *)(*(int *)(unaff_EBX + 0x68436e) + 8 + local_30 * 8),iVar5 * 8);
          }
          *(int *)(unaff_EBX + 0x68437a) = *(int *)(unaff_EBX + 0x68437a) + -1;
          break;
        }
        if (pNotifier == (CPortal_Base2D *)0x0) goto LAB_0074534c;
LAB_00745213:
        uVar1 = (pNotifier->m_hLinkedPortal).
                super_CNetworkVarBase<CBaseHandle,CPortal_Base2D::NetworkVar_m_hLinkedPortal>.
                m_Value.m_Index;
        if ((uVar1 == 0xffffffff) ||
           (iVar2 = (uVar1 & 0xffff) * 0x10 + iVar5, *(uint *)(iVar2 + 8) != uVar1 >> 0x10)) {
          iVar2 = 0;
        }
        else {
          iVar2 = *(int *)(iVar2 + 4);
        }
        if ((uVar6 == 0xffffffff) ||
           (iVar3 = (uVar6 & 0xffff) * 0x10 + iVar5, *(uint *)(iVar3 + 8) != uVar6 >> 0x10)) {
          iVar3 = 0;
        }
        else {
          iVar3 = *(int *)(iVar3 + 4);
        }
        if (iVar3 == iVar2) goto LAB_0074534c;
        local_30 = local_30 + 1;
        puVar7 = puVar7 + 2;
      } while (local_30 != *(uint *)(unaff_EBX + 0x68437a));
    }
    if (this == (CPortal_PointPush *)0x0) {
      local_20[0].m_Index = 0xffffffff;
    }
    else {
      local_20[0].m_Index = 0xffffffff;
      puVar4 = (ulong *)(*(this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.
                          super_IHandleEntity._vptr_IHandleEntity[3])(this);
      local_20[0].m_Index = *puVar4;
    }
    CPortal_Base2D::RemovePortalEventListener(pNotifier,(EHANDLE *)local_20);
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
    uVar6 = (pNotifier->m_hLinkedPortal).
            super_CNetworkVarBase<CBaseHandle,CPortal_Base2D::NetworkVar_m_hLinkedPortal>.m_Value.
            m_Index;
    if (((uVar6 != 0xffffffff) &&
        (iVar5 = (uVar6 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5019ce),
        *(uint *)(iVar5 + 8) == uVar6 >> 0x10)) && (*(int *)(iVar5 + 4) != 0)) {
      local_24.m_Index = 0xffffffff;
      if (this == (CPortal_PointPush *)0x0) {
        local_24.m_Index = 0xffffffff;
      }
      else {
        puVar4 = (ulong *)(*(this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.
                            super_IHandleEntity._vptr_IHandleEntity[3])(this);
        local_24.m_Index = *puVar4;
        uVar6 = (pNotifier->m_hLinkedPortal).
                super_CNetworkVarBase<CBaseHandle,CPortal_Base2D::NetworkVar_m_hLinkedPortal>.
                m_Value.m_Index;
      }
      if ((uVar6 == 0xffffffff) ||
         (iVar5 = (uVar6 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5019ce),
         *(uint *)(iVar5 + 8) != uVar6 >> 0x10)) {
        this_00 = (CPortal_Base2D *)0x0;
      }
      else {
        this_00 = *(CPortal_Base2D **)(iVar5 + 4);
      }
      CPortal_Base2D::RemovePortalEventListener(this_00,(EHANDLE *)&local_24);
    }
  }
  return;
}


/* CPortalVortexEffectManager::LevelShutdownPostEntity at 00746690 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void LevelShutdownPostEntity(CPortalVortexEffectManager * this) */

void __thiscall
CPortalVortexEffectManager::LevelShutdownPostEntity(CPortalVortexEffectManager *this)

{
  int *piVar1;
  ulong *puVar2;
  int iVar3;
  uint uVar4;
  PortalVortexEffect_t *pPVar5;
  int iVar6;
  CBaseEntity *oldObj;
  int unaff_EBX;
  int iVar7;
  
  ___i686_get_pc_thunk_bx();
  iVar3 = (this->m_Effects).m_Size;
  iVar7 = iVar3 + -1;
  if (-1 < iVar7) {
    iVar3 = iVar3 * 8;
    do {
      uVar4 = *(uint *)((int)&(this->m_Effects).m_Memory.m_pMemory[-1].hParticleSystem.
                              super_CBaseHandle.m_Index + iVar3);
      if ((uVar4 == 0xffffffff) ||
         (iVar6 = (uVar4 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x5004ce),
         *(uint *)(iVar6 + 8) != uVar4 >> 0x10)) {
        oldObj = (CBaseEntity *)0x0;
      }
      else {
        oldObj = *(CBaseEntity **)(iVar6 + 4);
      }
      UTIL_Remove(oldObj);
      iVar6 = (this->m_Effects).m_Size;
      if (0 < iVar6) {
        iVar6 = iVar6 + -1;
        if (iVar7 != iVar6) {
          pPVar5 = (this->m_Effects).m_Memory.m_pMemory;
          puVar2 = (ulong *)((int)&pPVar5[-1].hParticleSystem.super_CBaseHandle.m_Index + iVar3);
          pPVar5 = pPVar5 + iVar6;
          *puVar2 = (pPVar5->hParticleSystem).super_CBaseHandle.m_Index;
          puVar2[1] = (pPVar5->hPortalOwner).super_CBaseHandle.m_Index;
        }
        piVar1 = &(this->m_Effects).m_Size;
        *piVar1 = *piVar1 + -1;
      }
      iVar7 = iVar7 + -1;
      iVar3 = iVar3 + -8;
    } while (iVar7 != -1);
  }
  return;
}


/* DataMapInit<CPortal_PointPush> at 000b1710 */

datamap_t * DataMapInit<CPortal_PointPush>(CPortal_PointPush *param_1)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  char *pcVar4;
  size_t sVar5;
  undefined4 *puVar6;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x1e].m_pPrev + unaff_EBX) ==
       '\0') && (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xd9ddcc), iVar3 != 0)) {
    *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x1f].m_pPrev + unaff_EBX) =
         unaff_EBX + 0xa019b0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x20].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)
     ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x20].m_SerialNumber + unaff_EBX) = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x20].m_pPrev + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x20].m_pNext + unaff_EBX) =
         0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x21].m_pEntity + unaff_EBX)
         = 0;
    *(undefined4 *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x1f].m_pNext + unaff_EBX) =
         0x11;
    ___cxa_guard_release(unaff_EBX + 0xd9ddcc);
    ___cxa_atexit(unaff_EBX + 0x98d1ec,0,*(undefined4 *)(&DAT_00b95430 + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_EndTouch_00cb5b68 + unaff_EBX) =
       *(undefined4 *)(&DAT_00b95568 + unaff_EBX);
  if ((*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x1f].m_pEntity + unaff_EBX) ==
       '\0') && (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xd9ddd4), iVar3 != 0)) {
    pcVar4 = operator_new___(*(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x1f].
                                            m_pNext + unaff_EBX) + 10);
    _strcpy(pcVar4,*(char **)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x1f].m_pPrev +
                             unaff_EBX));
    sVar5 = _strlen(pcVar4);
    builtin_strncpy(pcVar4 + sVar5,"PushThink",10);
    uVar1 = *(uint *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x20].m_pNext + unaff_EBX);
    CUtlVector<char*,CUtlMemory<char*,_int>_>::GrowVector
              ((CUtlVector<char*,CUtlMemory<char*,_int>_> *)
               ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x20].m_pEntity + unaff_EBX),1);
    iVar3 = ~uVar1 + *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x20].m_pNext +
                             unaff_EBX);
    if (0 < iVar3) {
      iVar2 = *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x20].m_pEntity +
                      unaff_EBX);
      _V_memmove((void *)(iVar2 + 4 + uVar1 * 4),(void *)(iVar2 + uVar1 * 4),iVar3 * 4);
    }
    puVar6 = (undefined4 *)
             (uVar1 * 4 +
             *(int *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x20].m_pEntity + unaff_EBX
                     ));
    if (puVar6 != (undefined4 *)0x0) {
      *puVar6 = pcVar4;
    }
    *(char **)((int)&PTR_NetworkStateChanged_m_iMaxHealth_00cb5bc0 + unaff_EBX) = pcVar4;
    ___cxa_guard_release(unaff_EBX + 0xd9ddd4);
  }
  *(undefined4 *)((int)&PTR_StartTouch_00cb5b60 + unaff_EBX) = 8;
  *(int *)((int)&PTR_Use_00cb5b5c + unaff_EBX) =
       (int)&PTR_NetworkStateChanged_m_iMaxHealth_00cb5bbc + unaff_EBX;
  return (datamap_t *)((int)&PTR_Use_00cb5b5c + unaff_EBX);
}


/* CPortal_PointPush::GetDataDescMap at 00744d10 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPortal_PointPush * this) */

datamap_t * __thiscall CPortal_PointPush::GetDataDescMap(CPortal_PointPush *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x622568);
}


/* CPortal_PointPush::GetBaseMap at 00744d20 */

datamap_t * CPortal_PointPush::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x501f64);
}


/* __tcf_0 at 00a3e910 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x410bf6)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x410bea) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x410bf6));
  }
  *(undefined4 *)(unaff_EBX + 0x410bf6) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x410bf2)) {
    if (*(int *)(unaff_EBX + 0x410bea) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20823e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20823e),*(int *)(unaff_EBX + 0x410bea));
      *(undefined4 *)(unaff_EBX + 0x410bea) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x410bee) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x410bea);
  *(int *)(unaff_EBX + 0x410bfa) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x410bf2)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20823e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20823e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x410bea) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x410bee) = 0;
  }
  return;
}


/* ServerClassInit<DT_PortalPointPush::ignored> at 000b13d0 */

int ServerClassInit<DT_PortalPointPush::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2c].m_pNext + unaff_EBX + 2) ==
      '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xd9deb2);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x2d].m_pNext + unaff_EBX + 2)
                  ,(char *)(unaff_EBX + 0x9a4856),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x33].m_pEntity +
                        unaff_EBX + 2),&UNK_009a6c06 + unaff_EBX,0,
                        (SendTable *)**(undefined4 **)(&DAT_00b9586e + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00b95812 + unaff_EBX),0x80);
      SendPropBool((SendProp_conflict *)
                   ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x38].m_SerialNumber +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0x9fe12a),0x3c0,1);
      SendPropFloat((SendProp_conflict *)
                    ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x3d].m_pPrev +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x9a4e37),0x3c4,4,0x20,0,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00b9586a + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x42].m_pNext +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x9c8e06),0x3c8,4,0x20,0,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00b9586a + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x48].m_pEntity +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0x9f5ed8),0x3cc,4,0x20,0,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00b9586a + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x4d].m_SerialNumber +
                    unaff_EBX + 2),(char *)(unaff_EBX + 0xa01ce2),0x3d0,4,0x20,0,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00b9586a + unaff_EBX),0x80);
      ___cxa_guard_release(unaff_EBX + 0xd9deb2);
      ___cxa_atexit(unaff_EBX + 0x98d482,0,*(undefined4 *)(&DAT_00b95776 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)((int)&g_CommanderFollowFormation.flags + unaff_EBX + 2),
             (SendProp_conflict *)
             ((int)&gEntList.super_CBaseEntityList.m_EntPtrArray[0x33].m_pEntity + unaff_EBX + 2),6,
             *(char **)(&DAT_00c1ee0a + unaff_EBX));
  return 1;
}


/* CPortal_PointPush::GetServerClass at 00744d30 */

/* DWARF original prototype: ServerClass * GetServerClass(CPortal_PointPush * this) */

ServerClass * __thiscall CPortal_PointPush::GetServerClass(CPortal_PointPush *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x70a540);
}


/* CPortal_PointPush::YouForgotToImplementOrDeclareServerClass at 00744d40 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CPortal_PointPush * this)
    */

int __thiscall CPortal_PointPush::YouForgotToImplementOrDeclareServerClass(CPortal_PointPush *this)

{
  return 0;
}


/* __tcf_3 at 00a3e8f0 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x38ac2c),in_stack_00000008);
  return;
}


/* __tcf_1 at 00a3e860 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410c2b))(unaff_EBX + 0x410c2b);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410bd7))(unaff_EBX + 0x410bd7);
  (*(code *)**(undefined4 **)(CBaseEntity::SetLocalVelocity + unaff_EBX + 3))(unaff_EBX + 0x410b83);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410b2f))(unaff_EBX + 0x410b2f);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410adb))(unaff_EBX + 0x410adb);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410a87))(unaff_EBX + 0x410a87);
                    /* WARNING: Could not recover jumptable at 0x00a3e8e4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(CBaseEntity::Save + unaff_EBX + 3))();
  return;
}


/* CPortal_PointPush::DrawDebugGeometryOverlays at 00745ee0 */

/* DWARF original prototype: void DrawDebugGeometryOverlays(CPortal_PointPush * this) */

void __thiscall CPortal_PointPush::DrawDebugGeometryOverlays(CPortal_PointPush *this)

{
  Vector *origin;
  float radius;
  int unaff_EBX;
  Vector local_34;
  Vector local_28 [2];
  
  ___i686_get_pc_thunk_bx();
  local_34.x = 2.0;
  local_34.y = 2.0;
  local_34.z = 2.0;
  local_28[0].x = -2.0;
  local_28[0].y = -2.0;
  local_28[0].z = -2.0;
  if (((this->super_CBaseEntity).m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition(&this->super_CBaseEntity);
  }
  origin = &(this->super_CBaseEntity).m_vecAbsOrigin;
  NDebugOverlay::Box(origin,local_28,&local_34,0,0xff,0,0x10,0.05);
  radius = (this->m_flRadius).m_Value;
  if (((this->super_CBaseEntity).m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition(&this->super_CBaseEntity);
  }
  NDebugOverlay::Sphere(origin,*(QAngle **)(&DAT_00500d1a + unaff_EBX),radius,0,0xff,0,0,false,0.05)
  ;
  return;
}


/* CPortal_PointPush::Activate at 00744d70 */

/* DWARF original prototype: void Activate(CPortal_PointPush * this) */

void __thiscall CPortal_PointPush::Activate(CPortal_PointPush *this)

{
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((this->m_bEnabled).m_Value != false) {
    func.__delta = 0;
    func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)*(uint *)(unaff_EBX + 0x502933);
    CBaseEntity::ThinkSet(&this->super_CBaseEntity,func,0.0,(char *)0x0);
    CBaseEntity::SetNextThink
              (&this->super_CBaseEntity,
               *(float *)(unaff_EBX + 0x3890c3) + *(float *)(**(int **)(unaff_EBX + 0x501de3) + 0xc)
               ,(char *)0x0);
  }
  CBaseEntity::Activate(&this->super_CBaseEntity);
  return;
}


/* CPortal_PointPush::UpdateTransmitState at 00744d50 */

/* DWARF original prototype: int UpdateTransmitState(CPortal_PointPush * this) */

int __thiscall CPortal_PointPush::UpdateTransmitState(CPortal_PointPush *this)

{
  int iVar1;
  
  iVar1 = CBaseEntity::SetTransmitState(&this->super_CBaseEntity,1);
  return iVar1;
}


/* CPortal_PointPush::PushEntitiesAtPoint at 00745400 */

/* DWARF original prototype: void PushEntitiesAtPoint(CPortal_PointPush * this, Vector * vecOrigin,
   QAngle * vecAngles, CPortal_Base2D * pPortal) */

void __thiscall
CPortal_PointPush::PushEntitiesAtPoint
          (CPortal_PointPush *this,Vector *vecOrigin,QAngle *vecAngles,CPortal_Base2D *pPortal)

{
  code *pcVar1;
  _func_int_varargs **pp_Var2;
  _func_int_varargs *p_Var3;
  char cVar4;
  bool bVar5;
  int iVar6;
  int iVar7;
  uint uVar8;
  int *piVar9;
  int unaff_EBX;
  int *piVar10;
  int *piVar11;
  int *piVar12;
  Vector *pVVar13;
  int iVar14;
  longdouble lVar15;
  float fVar16;
  float fVar17;
  float fVar18;
  QAngle *local_5b8;
  float local_5a8;
  float local_5a4;
  code *local_5a0;
  int iStack_594;
  CBaseEntity *local_590 [256];
  undefined1 local_190 [12];
  undefined1 local_184 [32];
  float local_164;
  int *local_144;
  float local_13c;
  float local_138;
  float local_134;
  float local_12c;
  float local_128;
  float local_124;
  undefined4 local_11c;
  undefined4 local_118;
  undefined4 local_114;
  undefined4 local_10c;
  undefined4 local_108;
  undefined4 local_104;
  undefined4 local_fc;
  undefined1 local_f8;
  undefined1 local_f7;
  CFlaggedEntitiesEnum local_e4;
  float local_d0;
  float local_cc;
  float local_c8;
  float local_c4;
  float local_c0;
  float local_bc;
  float local_b8;
  float local_b4;
  float local_b0;
  _func_int_varargs **local_94;
  CBaseEntity **local_90;
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
  _func_int_varargs **local_4c;
  CBaseEntity **local_48;
  float local_44;
  float local_40;
  float local_3c;
  float local_38;
  undefined1 local_34 [12];
  undefined1 local_28 [24];
  
                    /* Unresolved local var: CBaseEntity *[256] pEnts@[???]
                       Unresolved local var: int numEnts@[???] */
  piVar9 = (int *)&stack0xfffff9d4;
  ___i686_get_pc_thunk_bx();
  fVar18 = (this->m_flRadius).m_Value;
                    /* Unresolved local var: CFlaggedEntitiesEnum sphereEnum@[???] */
  CFlaggedEntitiesEnum::CFlaggedEntitiesEnum(&local_e4,local_590,0x100,0);
  iVar6 = UTIL_EntitiesInSphere(vecOrigin,fVar18,&local_e4);
  if (0 < iVar6) {
    iVar14 = 1;
    do {
      piVar10 = (int *)(&iStack_594)[iVar14];
      if ((*(char *)((int)piVar10 + 0x19a) != '\0') && ((*(ushort *)(piVar10 + 0x66) & 4) == 0)) {
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
        uVar8 = piVar10[0x5b];
        if (((uVar8 == 0xffffffff) ||
            ((iVar7 = (uVar8 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x50175b),
             *(uint *)(iVar7 + 8) != uVar8 >> 0x10 || (*(int *)(iVar7 + 4) == 0)))) &&
           ((cVar4 = *(char *)((int)piVar10 + 0x16a), cVar4 == '\x06' ||
            ((cVar4 == '\x02' || (cVar4 == '\x03')))))) {
          iVar7 = *piVar10;
          *piVar9 = (int)piVar10;
          pcVar1 = *(code **)(iVar7 + 0x154);
          piVar9[-1] = 0x74550b;
          cVar4 = (*pcVar1)();
          if (((cVar4 == '\0') || (((this->super_CBaseEntity).m_spawnflags.m_Value & 8) != 0)) &&
             ((piVar10 = (int *)(&iStack_594)[iVar14], *(char *)((int)piVar10 + 0x16a) != '\x06' ||
              (((this->super_CBaseEntity).m_spawnflags.m_Value & 0x10) != 0)))) {
            fVar18 = (this->m_flConeOfInfluence).m_Value;
            piVar11 = piVar9;
            if (fVar18 != 0.0) {
                    /* Unresolved local var: float flMinAngle@[???] */
              iVar7 = *piVar10;
              piVar9[3] = 0;
              piVar9[2] = (int)vecOrigin;
              piVar9[1] = (int)piVar10;
              *piVar9 = (int)local_28;
              pcVar1 = *(code **)(iVar7 + 0x230);
              piVar9[-1] = 0x745763;
              (*pcVar1)();
              piVar12 = piVar9 + -1;
              piVar9[-1] = (int)(fVar18 * *(float *)(unaff_EBX + 0x3876f3));
              piVar9[-2] = 0x745780;
              lVar15 = (longdouble)_cosf();
              piVar9[3] = (int)(float)lVar15;
              piVar9[2] = (int)vecAngles;
              piVar9[1] = (int)local_28;
              *piVar9 = (int)vecOrigin;
              piVar9[-1] = (int)this;
              piVar9[-2] = 0x7457a1;
              bVar5 = TestWithinCone((CPortal_PointPush *)piVar9[-1],(Vector *)*piVar9,
                                     (Vector *)piVar9[1],(QAngle *)piVar9[2],(float)piVar9[3]);
              piVar9 = piVar9 + -1;
              piVar11 = piVar12;
              if (!bVar5) goto LAB_007454e0;
            }
            piVar10 = piVar11;
            if (pPortal != (CPortal_Base2D *)0x0) {
              if (((pPortal->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) == 0) {
                piVar9 = (int *)(&iStack_594)[iVar14];
                local_5a0 = *(code **)(*piVar9 + 0x230);
              }
              else {
                *piVar11 = (int)pPortal;
                piVar11[-1] = 0x745ade;
                CBaseEntity::CalcAbsolutePosition((CBaseEntity *)*piVar11);
                piVar9 = (int *)(&iStack_594)[iVar14];
                local_5a0 = *(code **)(*piVar9 + 0x230);
                if (((pPortal->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
                  *piVar11 = (int)pPortal;
                  piVar11[-1] = 0x745b1d;
                  CBaseEntity::CalcAbsolutePosition((CBaseEntity *)*piVar11);
                  piVar9 = (int *)(&iStack_594)[iVar14];
                }
              }
              local_5b8 = &(pPortal->super_CBaseAnimating).super_CBaseEntity.m_angAbsRotation;
              pVVar13 = &(pPortal->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin;
              piVar11[3] = 0;
              piVar11[2] = (int)pVVar13;
              piVar11[1] = (int)piVar9;
              *piVar11 = (int)local_34;
              piVar11[-1] = 0x7455b7;
              (*local_5a0)();
              piVar10 = piVar11 + -1;
              if (((pPortal->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
                piVar11[-1] = (int)pPortal;
                piVar11[-2] = 0x745ad1;
                CBaseEntity::CalcAbsolutePosition((CBaseEntity *)piVar11[-1]);
              }
              piVar11[3] = 0;
              piVar11[2] = (int)local_5b8;
              piVar11[1] = (int)local_34;
              *piVar11 = (int)pVVar13;
              piVar11[-1] = (int)this;
              piVar11[-2] = 0x7455f5;
              bVar5 = TestWithinCone((CPortal_PointPush *)piVar11[-1],(Vector *)*piVar11,
                                     (Vector *)piVar11[1],(QAngle *)piVar11[2],(float)piVar11[3]);
              piVar9 = piVar11 + -1;
              if (!bVar5) goto LAB_007454e0;
            }
            uVar8 = (this->super_CBaseEntity).m_spawnflags.m_Value;
            if ((uVar8 & 1) == 0) {
              piVar11 = piVar10;
              piVar9 = (int *)(&iStack_594)[iVar14];
            }
            else {
              local_7c = vecOrigin->x;
              local_78 = vecOrigin->y;
              local_74 = vecOrigin->z;
              piVar9 = (int *)(&iStack_594)[iVar14];
              iVar7 = *piVar9;
              piVar10[3] = 0;
              piVar10[2] = (int)&local_7c;
              piVar10[1] = (int)piVar9;
              *piVar10 = (int)&local_40;
              pcVar1 = *(code **)(iVar7 + 0x230);
              piVar10[-1] = 0x745b69;
              (*pcVar1)();
              if ((this->m_flInnerRadius).m_Value != 0.0) {
                    /* Unresolved local var: Vector vecDirToTarget@[???]
                       Unresolved local var: Vector res@[???] */
                local_e4.super_IPartitionEnumerator._vptr_IPartitionEnumerator =
                     (_func_int_varargs **)(local_40 - local_7c);
                local_e4.m_pList = (CBaseEntity **)(local_3c - local_78);
                local_e4.m_listMax = (int)(local_38 - local_74);
                piVar10[-1] = (int)&local_e4;
                piVar10[-2] = 0x745dd3;
                local_4c = local_e4.super_IPartitionEnumerator._vptr_IPartitionEnumerator;
                local_48 = local_e4.m_pList;
                local_44 = (float)local_e4.m_listMax;
                VectorNormalize((Vector *)piVar10[-1]);
                local_50 = (this->m_flInnerRadius).m_Value;
                    /* Unresolved local var: Vector res@[???] */
                local_58 = local_50 *
                           (float)local_e4.super_IPartitionEnumerator._vptr_IPartitionEnumerator;
                local_54 = local_50 * (float)local_e4.m_pList;
                local_50 = local_50 * (float)local_e4.m_listMax;
                    /* Unresolved local var: Vector res@[???] */
                local_7c = vecOrigin->x + local_58;
                local_78 = vecOrigin->y + local_54;
                local_74 = vecOrigin->z + local_50;
                local_64 = local_7c;
                local_60 = local_78;
                local_5c = local_74;
              }
              piVar9 = (int *)(&iStack_594)[iVar14];
              iVar7 = *piVar9;
              piVar10[2] = 0;
              piVar10[1] = (int)&local_7c;
              *piVar10 = (int)piVar9;
              piVar10[-1] = (int)&local_70;
              pcVar1 = *(code **)(iVar7 + 0x230);
              piVar10[-2] = 0x745bbd;
              (*pcVar1)();
              piVar11 = piVar10 + -2;
              local_12c = local_70 - local_7c;
              local_128 = local_6c - local_78;
              local_124 = local_68 - local_74;
              local_f7 = local_12c * local_12c + local_128 * local_128 + local_124 * local_124 !=
                         0.0;
              local_104 = 0;
              local_108 = 0;
              local_10c = 0;
              local_fc = 0;
              local_f8 = 1;
              local_114 = 0;
              local_118 = 0;
              local_11c = 0;
              local_13c = local_7c;
              local_138 = local_78;
              local_134 = local_74;
              piVar10[1] = 0;
              *piVar10 = 0;
              piVar10[-1] = (int)this;
              piVar10[-2] = (int)&local_e4;
              piVar10[-3] = 0x745c9a;
              CTraceFilterSimple::CTraceFilterSimple
                        ((CTraceFilterSimple *)piVar10[-2],(IHandleEntity *)piVar10[-1],*piVar10,
                         (ShouldHitFunc_t)piVar10[1]);
              piVar9 = (int *)**(undefined4 **)(unaff_EBX + 0x5017bf);
              iVar7 = *piVar9;
              piVar10[2] = (int)local_190;
              piVar10[1] = (int)&local_e4;
              *piVar10 = 0x400b;
              piVar10[-1] = (int)&local_13c;
              piVar10[-2] = (int)piVar9;
              pcVar1 = *(code **)(iVar7 + 0x14);
              piVar10[-3] = 0x745cd1;
              (*pcVar1)();
              if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x5017bb) + 0x1c) + 0x30) != 0) {
                piVar10[4] = -0x40800000;
                piVar10[3] = 1;
                piVar10[2] = 0;
                piVar10[1] = 0;
                *piVar10 = 0xff;
                piVar10[-1] = (int)local_184;
                piVar10[-2] = (int)local_190;
                piVar10[-3] = 0x745d87;
                DebugDrawLine((Vector *)piVar10[-2],(Vector *)piVar10[-1],*piVar10,piVar10[1],
                              piVar10[2],SUB41(piVar10[3],0),(float)piVar10[4]);
              }
              if (local_164 < *(float *)(unaff_EBX + 0x3874ff)) {
                piVar9 = piVar10 + -2;
                if (local_144 != (int *)(&iStack_594)[iVar14]) goto LAB_007454e0;
                uVar8 = (this->super_CBaseEntity).m_spawnflags.m_Value;
                piVar9 = local_144;
              }
              else {
                uVar8 = (this->super_CBaseEntity).m_spawnflags.m_Value;
                piVar11 = piVar10 + -2;
                piVar9 = (int *)(&iStack_594)[iVar14];
              }
            }
            if ((uVar8 & 2) == 0) {
              iVar7 = *piVar9;
              piVar11[3] = 0;
              piVar11[2] = (int)vecOrigin;
              piVar11[1] = (int)piVar9;
              *piVar11 = (int)&local_88;
              pcVar1 = *(code **)(iVar7 + 0x230);
              piVar11[-1] = 0x745642;
              (*pcVar1)();
              piVar11 = piVar11 + -1;
                    /* Unresolved local var: Vector res@[???] */
              local_e4.super_IPartitionEnumerator._vptr_IPartitionEnumerator =
                   (_func_int_varargs **)(local_88 - vecOrigin->x);
              local_e4.m_pList = (CBaseEntity **)(local_84 - vecOrigin->y);
              local_e4.m_listMax = (int)(local_80 - vecOrigin->z);
              local_94 = local_e4.super_IPartitionEnumerator._vptr_IPartitionEnumerator;
              local_90 = local_e4.m_pList;
              local_8c = (float)local_e4.m_listMax;
            }
            else {
              pp_Var2 = (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.
                        super_IHandleEntity._vptr_IHandleEntity;
              piVar11[3] = 0;
              piVar11[2] = 0;
              piVar11[1] = (int)&local_e4;
              *piVar11 = (int)this;
              p_Var3 = pp_Var2[0x8e];
              piVar11[-1] = 0x745809;
              (*p_Var3)();
            }
            *piVar11 = (int)&local_e4;
            piVar11[-1] = 0x7456a6;
            fVar18 = VectorNormalize((Vector *)*piVar11);
            if (((this->super_CBaseEntity).m_spawnflags.m_Value & 4) == 0) {
              fVar17 = (this->m_flRadius).m_Value;
              fVar16 = *(float *)(unaff_EBX + 0x3878af) * fVar17;
              if ((fVar17 != fVar16) || (NAN(fVar17) || NAN(fVar16))) {
                fVar18 = (fVar18 - fVar17) / (fVar16 - fVar17);
                fVar17 = 0.0;
                if ((0.0 <= fVar18) && (fVar17 = *(float *)(unaff_EBX + 0x3874ff), fVar18 <= fVar17)
                   ) {
                  fVar17 = fVar18 + 0.0;
                }
              }
              else {
                fVar17 = 0.0;
                if (0.0 <= fVar18 - fVar16) goto LAB_007456f2;
              }
            }
            else {
LAB_007456f2:
              fVar17 = *(float *)(unaff_EBX + 0x3874ff);
            }
            if (*(byte *)((int)piVar9 + 0x16a) < 9) {
                    /* WARNING: Could not recover jumptable at 0x00745713. Too many branches */
                    /* WARNING: Treating indirect jump as call */
              (*(code *)(*(int *)(unaff_EBX + 0x307 + (uint)*(byte *)((int)piVar9 + 0x16a) * 4) +
                        unaff_EBX))();
              return;
            }
            local_b0 = (this->m_flMagnitude).m_Value;
                    /* Unresolved local var: Vector res@[???] */
            local_b8 = local_b0 *
                       (float)local_e4.super_IPartitionEnumerator._vptr_IPartitionEnumerator;
            local_b4 = local_b0 * (float)local_e4.m_pList;
            local_b0 = local_b0 * (float)local_e4.m_listMax;
                    /* Unresolved local var: Vector res@[???] */
            local_c4 = fVar17 * local_b8;
            local_c0 = fVar17 * local_b4;
            local_bc = fVar17 * local_b0;
            fVar18 = local_bc;
            local_5a8 = local_c0;
            local_5a4 = local_c4;
            if ((*(byte *)((int)piVar9 + 0x132) & 0x80) != 0) {
                    /* Unresolved local var: Vector res@[???] */
              local_5a4 = local_c4 + (float)piVar9[0x87];
              local_5a8 = local_c0 + (float)piVar9[0x88];
              fVar18 = local_bc + (float)piVar9[0x89];
              local_d0 = local_5a4;
              local_cc = local_5a8;
              local_c8 = fVar18;
            }
            if ((0.0 < fVar18) && ((*(byte *)(piVar9 + 0x4c) & 1) != 0)) {
              piVar11[1] = 0;
              *piVar11 = (int)piVar9;
              piVar11[-1] = 0x745e5e;
              CBaseEntity::SetGroundEntity((CBaseEntity *)*piVar11,(CBaseEntity *)piVar11[1]);
              if ((*(byte *)((int)piVar9 + 0x12d) & 8) != 0) {
                *piVar11 = (int)piVar9;
                piVar11[-1] = 0x745ed6;
                CBaseEntity::CalcAbsolutePosition((CBaseEntity *)*piVar11);
              }
              local_7c = (float)piVar9[0xa5];
              local_78 = (float)piVar9[0xa6];
              local_74 = *(float *)(unaff_EBX + 0x3874ff) + (float)piVar9[0xa7];
              piVar11[1] = (int)&local_7c;
              *piVar11 = (int)piVar9;
              piVar11[-1] = 0x745ea5;
              CBaseEntity::SetAbsOrigin((CBaseEntity *)*piVar11,(Vector *)piVar11[1]);
            }
            if ((((local_5a4 != (float)piVar9[0x87]) || (NAN(local_5a4) || NAN((float)piVar9[0x87]))
                 ) || (local_5a8 != (float)piVar9[0x88])) ||
               ((NAN(local_5a8) || NAN((float)piVar9[0x88]) || (fVar18 != (float)piVar9[0x89])))) {
                    /* Unresolved local var: CBaseEntity * pThis@[???] */
              iVar7 = *piVar9;
              piVar11[1] = (int)(piVar9 + 0x87);
              *piVar11 = (int)piVar9;
              pcVar1 = *(code **)(iVar7 + 0x2e8);
              piVar11[-1] = 0x745a94;
              (*pcVar1)();
              piVar9[0x87] = (int)local_5a4;
              piVar9[0x88] = (int)local_5a8;
              piVar9[0x89] = (int)fVar18;
            }
            piVar11[1] = 0x800000;
            *piVar11 = (int)piVar9;
            piVar11[-1] = 0x745971;
            CBaseEntity::AddFlag((CBaseEntity *)*piVar11,piVar11[1]);
            piVar9 = piVar11;
          }
        }
      }
LAB_007454e0:
      iVar14 = iVar14 + 1;
    } while (iVar14 != iVar6 + 1);
  }
  return;
}


/* CPortal_PointPush::InputEnable at 00744fb0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* DWARF original prototype: void InputEnable(CPortal_PointPush * this, inputdata_t * inputdata) */

void __thiscall CPortal_PointPush::InputEnable(CPortal_PointPush *this,inputdata_t *inputdata)

{
  undefined1 *puVar1;
  uint uVar2;
  int iVar3;
  ushort uVar4;
  CBaseEdict *this_00;
  int *piVar5;
  ushort *puVar6;
  BASEPTR func;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  int unaff_EBX;
  uint uVar9;
  
  ___i686_get_pc_thunk_bx();
  if ((this->m_bEnabled).m_Value == false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(this_00);
        piVar5 = *(int **)(unaff_EBX + 0x501ba4);
        puVar6 = (ushort *)*piVar5;
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar6) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar9 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar4 = puVar6[uVar9 * 0x14 + 0x14];
          if (uVar4 == 0) {
LAB_00745156:
            puVar6[(uint)uVar4 + uVar9 * 0x14 + 1] = 0x3c0;
            puVar6[uVar9 * 0x14 + 0x14] = uVar4 + 1;
          }
          else if (puVar6[uVar9 * 0x14 + 1] != 0x3c0) {
            uVar8 = 0;
            do {
              uVar2 = uVar8 + 1;
              uVar8 = uVar2 & 0xffff;
              if ((ushort)uVar2 == uVar4) {
                if (uVar4 == 0x13) goto LAB_00745177;
                goto LAB_00745156;
              }
            } while (puVar6[uVar9 * 0x14 + uVar8 + 1] != 0x3c0);
          }
        }
        else if (puVar6[0x7d1] == 100) {
LAB_00745177:
          pIVar7->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar6[0x7d1];
          *(short *)(*piVar5 + 0xfa2) = *(short *)(*piVar5 + 0xfa2) + 1;
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar3 = *piVar5 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar3 + 2) = 0x3c0;
          *(undefined2 *)(iVar3 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bEnabled).m_Value = true;
  }
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)*(uint *)(unaff_EBX + 0x5026f0);
  CBaseEntity::ThinkSet(&this->super_CBaseEntity,func,0.0,(char *)0x0);
  CBaseEntity::SetNextThink
            (&this->super_CBaseEntity,
             *(float *)(CAI_Navigator::ClearNavFailCounter + unaff_EBX) +
             *(float *)(**(int **)(unaff_EBX + 0x501ba0) + 0xc),(char *)0x0);
  return;
}


/* CPortal_PointPush::InputDisable at 00744df0 */

/* DWARF original prototype: void InputDisable(CPortal_PointPush * this, inputdata_t * inputdata) */

void __thiscall CPortal_PointPush::InputDisable(CPortal_PointPush *this,inputdata_t *inputdata)

{
  undefined1 *puVar1;
  uint uVar2;
  int iVar3;
  ushort uVar4;
  CBaseEdict *this_00;
  int *piVar5;
  ushort *puVar6;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  int unaff_EBX;
  uint uVar9;
  
  ___i686_get_pc_thunk_bx();
  if ((this->m_bEnabled).m_Value != false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(this_00);
        piVar5 = *(int **)(unaff_EBX + 0x501d64);
        puVar6 = (ushort *)*piVar5;
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar6) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar9 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar4 = puVar6[uVar9 * 0x14 + 0x14];
          if (uVar4 == 0) {
LAB_00744f86:
            puVar6[(uint)uVar4 + uVar9 * 0x14 + 1] = 0x3c0;
            puVar6[uVar9 * 0x14 + 0x14] = uVar4 + 1;
          }
          else if (puVar6[uVar9 * 0x14 + 1] != 0x3c0) {
            uVar8 = 0;
            do {
              uVar2 = uVar8 + 1;
              uVar8 = uVar2 & 0xffff;
              if ((ushort)uVar2 == uVar4) {
                if (uVar4 != 0x13) goto LAB_00744f86;
                goto LAB_00744f09;
              }
            } while (puVar6[uVar9 * 0x14 + uVar8 + 1] != 0x3c0);
          }
        }
        else if (puVar6[0x7d1] == 100) {
LAB_00744f09:
          pIVar7->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar6[0x7d1];
          *(short *)(*piVar5 + 0xfa2) = *(short *)(*piVar5 + 0xfa2) + 1;
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar3 = *piVar5 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar3 + 2) = 0x3c0;
          *(undefined2 *)(iVar3 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bEnabled).m_Value = false;
  }
  CBaseEntity::ThinkSet(&this->super_CBaseEntity,(BASEPTR)0x0,0.0,(char *)0x0);
  CBaseEntity::SetNextThink
            (&this->super_CBaseEntity,*(float *)(**(int **)(unaff_EBX + 0x501d60) + 0xc),(char *)0x0
            );
  return;
}


/* _GLOBAL__I_g_PortalVortexManager at 000b1b70 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_g_PortalVortexManager(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

