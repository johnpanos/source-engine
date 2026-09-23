/* DWARF-guided pseudocode for game/server/portal2/portal_pointpush.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000c6a40 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  undefined *puVar1;
  char *s2;
  undefined4 *puVar2;
  datamap_t *pdVar3;
  int iVar4;
  IEntityFactoryDictionary *pIVar5;
  int unaff_EBX;
  undefined4 *puVar6;
  undefined4 *puVar7;
  longlong lVar8;
  
  lVar8 = ___i686_get_pc_thunk_bx();
  if (lVar8 == 0xffff00000001) {
    *(undefined1 *)
     ((int)&vgui::CheckButton::PanelMessageFunc_OnCheckButtonChecked::InitVar()::bAdded +
     unaff_EBX + 1) = 0;
    *(undefined1 *)
     ((int)&vgui::CheckButton::PanelMessageFunc_OnCheckButtonChecked::InitVar()::bAdded +
     unaff_EBX + 2) = 0;
    *(undefined1 *)
     ((int)&vgui::CheckButton::PanelMessageFunc_OnCheckButtonChecked::InitVar()::bAdded +
     unaff_EBX + 3) = 0;
    (&DAT_00cf34cf)[unaff_EBX] = 0;
    *(undefined4 *)((int)&vgui::CircularProgressBar::GetMessageMap()::s_pMap + unaff_EBX) = 0;
    *(undefined4 *)(&DAT_00cf34d4 + unaff_EBX) = 0;
    *(undefined4 *)((int)&vgui::CircularProgressBar::GetKBMap()::s_pMap + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00cf34dc + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&vgui::CircularProgressBar::GetKBMap::s_pMap + unaff_EBX) = 0;
    *(undefined4 *)((int)&vgui::CircularProgressBar::GetMessageMap::s_pMap + unaff_EBX) = 0;
    *(undefined4 *)(&vgui::CircularProgressBar::ChainToMap()::chained + unaff_EBX) = 0;
    *(undefined4 *)((int)&vgui::CircularProgressBar::KB_ChainToMap()::chained + unaff_EBX + 2) = 0;
    *(undefined4 *)(&DAT_00cf34f0 + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00cf34f4 + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00cf34f8 + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)(&DAT_00cf34fc + unaff_EBX) = 0x7f7fffff;
    *(undefined4 *)((int)&g_CircularProgressBarLinkerHack + unaff_EBX) = 0;
    (&DAT_00cf3507)[unaff_EBX] = 0;
    (&DAT_00cf3504)[unaff_EBX] = 0;
    (&DAT_00cf3505)[unaff_EBX] = 0;
    (&DAT_00cf3506)[unaff_EBX] = 0;
    *(undefined4 *)(&DAT_00cf3508 + unaff_EBX) = 0;
    (&DAT_00cf350f)[unaff_EBX] = 0;
    (&DAT_00cf350c)[unaff_EBX] = 0;
    (&DAT_00cf350d)[unaff_EBX] = 0;
    (&DAT_00cf350e)[unaff_EBX] = 0;
    *(undefined4 *)(&DAT_00cf3510 + unaff_EBX) = 0;
    (&DAT_00cf3517)[unaff_EBX] = 1;
    (&DAT_00cf3514)[unaff_EBX] = 0;
    (&DAT_00cf3515)[unaff_EBX] = 0;
    (&DAT_00cf3516)[unaff_EBX] = 0;
    *(undefined4 *)(&DAT_00cf3518 + unaff_EBX) = 1;
    (&DAT_00cf351f)[unaff_EBX] = 0;
    (&DAT_00cf351c)[unaff_EBX] = 0;
    (&DAT_00cf351d)[unaff_EBX] = 0;
    (&DAT_00cf351e)[unaff_EBX] = 0;
    *(undefined4 *)((int)&vgui::ComboBox::GetMessageMap()::s_pMap + unaff_EBX) = 2;
    (&DAT_00cf3527)[unaff_EBX] = 0;
    (&DAT_00cf3524)[unaff_EBX] = 0;
    (&DAT_00cf3525)[unaff_EBX] = 0;
    (&DAT_00cf3526)[unaff_EBX] = 0;
    *(undefined **)((int)&vgui::ComboBox::GetKBMap()::s_pMap + unaff_EBX) =
         &UNK_00aea8d4 + unaff_EBX;
    CAutoGameSystem::CAutoGameSystem
              ((CAutoGameSystem *)((int)&PTR_Paint_00c68c8c + unaff_EBX),(char *)0x0);
    *(undefined **)((int)&PTR_Paint_00c68c8c + unaff_EBX) = &UNK_00b70694 + unaff_EBX;
    *(undefined4 *)((int)&PTR_PostChildPaint_00c68c98 + unaff_EBX) = 0;
    *(undefined4 *)((int)&PTR_PerformLayout_00c68c9c + unaff_EBX) = 0;
    *(undefined4 *)((int)&PTR_GetPanelMap_00c68ca0 + unaff_EBX) = 0;
    *(undefined4 *)((int)&PTR_SetProportional_00c68ca4 + unaff_EBX) = 0;
    *(undefined4 *)((int)&PTR_SetMouseInputEnabled_00c68ca8 + unaff_EBX) = 0;
    ___cxa_atexit(unaff_EBX + 0x8d4a6c,0,*(undefined4 *)(&DAT_00ae513c + unaff_EBX));
    pdVar3 = DataMapInit<CPortal_PointPush>((CPortal_PointPush *)0x0);
    *(datamap_t **)((int)&PTR_SetKeyBoardInputEnabled_00c68cac + unaff_EBX) = pdVar3;
    puVar1 = &DAT_00cf352c + unaff_EBX;
    s2 = (char *)(unaff_EBX + 0x94a4dc);
    *(char **)(&DAT_00cf352c + unaff_EBX) = s2;
    *(SendTable **)((int)&vgui::Button::GetKBMap::s_pMap + unaff_EBX) =
         (SendTable *)((int)&PTR_IsMouseInputEnabled_00c68cb0 + unaff_EBX);
    *(undefined4 *)((int)&vgui::ComboBox::GetMessageMap::s_pMap + unaff_EBX) = 0xffff;
    puVar7 = (undefined4 *)**(int **)(&DAT_00ae51dc + unaff_EBX);
    if (puVar7 == (undefined4 *)0x0) {
      **(int **)(&DAT_00ae51dc + unaff_EBX) = (int)puVar1;
      *(undefined4 *)(&DAT_00cf3534 + unaff_EBX) = 0;
    }
    else {
      puVar2 = (undefined4 *)puVar7[2];
      iVar4 = _V_stricmp((char *)*puVar7,s2);
      if (iVar4 < 1) {
        while ((puVar6 = puVar2, puVar6 != (undefined4 *)0x0 &&
               (iVar4 = _V_stricmp((char *)*puVar6,s2), iVar4 < 1))) {
          puVar2 = (undefined4 *)puVar6[2];
          puVar7 = puVar6;
        }
        *(undefined4 **)(&DAT_00cf3534 + unaff_EBX) = puVar6;
        puVar7[2] = puVar1;
      }
      else {
        *(undefined4 *)(&DAT_00cf3534 + unaff_EBX) = **(undefined4 **)(&DAT_00ae51dc + unaff_EBX);
        **(undefined4 **)(&DAT_00ae51dc + unaff_EBX) = puVar1;
      }
    }
    SendTable::SendTable((SendTable *)((int)&PTR_IsMouseInputEnabled_00c68cb0 + unaff_EBX));
    ___cxa_atexit(unaff_EBX + 0x8d496c,0,*(undefined4 *)(&DAT_00ae513c + unaff_EBX));
    iVar4 = ServerClassInit<DT_PortalPointPush::ignored>((ignored *)0x0);
    *(int *)((int)&PTR_DrawHollowBox_00c68cc4 + unaff_EBX) = iVar4;
    *(undefined **)(&vgui::ComboBox::ChainToMap()::chained + unaff_EBX) = &UNK_00b70704 + unaff_EBX;
    pIVar5 = EntityFactoryDictionary();
    (**pIVar5->_vptr_IEntityFactoryDictionary)
              (pIVar5,&vgui::ComboBox::ChainToMap()::chained + unaff_EBX,unaff_EBX + 0x94a4ff);
  }
  return;
}


/* CPortalVortexEffectManager::~CPortalVortexEffectManager at 006d7e10 */

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
       (_func_int_varargs **)(unaff_EBX + 0x55f2c4);
  (this->m_Effects).m_Size = 0;
  if ((this->m_Effects).m_Memory.m_nGrowSize < 0) {
    pPVar1 = (this->m_Effects).m_Memory.m_pMemory;
  }
  else {
    pPVar1 = (this->m_Effects).m_Memory.m_pMemory;
    if (pPVar1 != (PortalVortexEffect_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d3d74) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4d3d74),pPVar1);
      (this->m_Effects).m_Memory.m_pMemory = (PortalVortexEffect_t *)0x0;
    }
    pPVar1 = (PortalVortexEffect_t *)0x0;
    (this->m_Effects).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_Effects).m_pElements = pPVar1;
  if (-1 < (this->m_Effects).m_Memory.m_nGrowSize) {
    if (pPVar1 != (PortalVortexEffect_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d3d74) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4d3d74),pPVar1);
      (this->m_Effects).m_Memory.m_pMemory = (PortalVortexEffect_t *)0x0;
    }
    (this->m_Effects).m_Memory.m_nAllocationCount = 0;
  }
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(unaff_EBX + 0x4d6284);
  IGameSystem::~IGameSystem((IGameSystem *)this,__in_chrg);
  return;
}


/* CPortalVortexEffectManager::~CPortalVortexEffectManager at 006d7f10 */

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
       (_func_int_varargs **)(unaff_EBX + 0x55f1c4);
  (this->m_Effects).m_Size = 0;
  if ((this->m_Effects).m_Memory.m_nGrowSize < 0) {
    pPVar1 = (this->m_Effects).m_Memory.m_pMemory;
  }
  else {
    pPVar1 = (this->m_Effects).m_Memory.m_pMemory;
    if (pPVar1 != (PortalVortexEffect_t *)0x0) {
      in_stack_ffffffc8 = pPVar1;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d3c74) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4d3c74));
      pPVar1 = (PortalVortexEffect_t *)0x0;
      (this->m_Effects).m_Memory.m_pMemory = (PortalVortexEffect_t *)0x0;
    }
    (this->m_Effects).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_Effects).m_pElements = pPVar1;
  if (-1 < (this->m_Effects).m_Memory.m_nGrowSize) {
    if (pPVar1 != (PortalVortexEffect_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d3c74) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x4d3c74));
      (this->m_Effects).m_Memory.m_pMemory = (PortalVortexEffect_t *)0x0;
      in_stack_ffffffc8 = pPVar1;
    }
    (this->m_Effects).m_Memory.m_nAllocationCount = 0;
  }
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(unaff_EBX + 0x4d6184);
  IGameSystem::~IGameSystem((IGameSystem *)this,(int)in_stack_ffffffc8);
  operator_delete(this);
  return;
}


/* __tcf_2 at 0099b4c0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void __tcf_2(void *param_1)

{
  int unaff_EBX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_bx();
  *(int *)(unaff_EBX + 0x39420c) = unaff_EBX + 0x29bc14;
  *(undefined4 *)(unaff_EBX + 0x394224) = 0;
  if (*(int *)(unaff_EBX + 0x394220) < 0) {
    *(undefined4 *)(unaff_EBX + 0x394228) = *(undefined4 *)(unaff_EBX + 0x394218);
  }
  else if (*(int *)(unaff_EBX + 0x394218) == 0) {
    *(undefined4 *)(unaff_EBX + 0x39421c) = 0;
    *(undefined4 *)(unaff_EBX + 0x394228) = 0;
    *(undefined4 *)(unaff_EBX + 0x39421c) = 0;
  }
  else {
    (**(code **)(*(int *)**(undefined4 **)(CPortalPlayerAnimState::CalcMainActivity + unaff_EBX + 4)
                + 8))((int *)**(undefined4 **)
                               (CPortalPlayerAnimState::CalcMainActivity + unaff_EBX + 4),
                      *(int *)(unaff_EBX + 0x394218));
    *(undefined4 *)(unaff_EBX + 0x394218) = 0;
    *(undefined4 *)(unaff_EBX + 0x39421c) = 0;
    *(undefined4 *)(unaff_EBX + 0x394228) = 0;
    if (-1 < *(int *)(unaff_EBX + 0x394220)) {
      *(undefined4 *)(unaff_EBX + 0x39421c) = 0;
    }
  }
  *(int *)(unaff_EBX + 0x39420c) = unaff_EBX + 0x212bd4;
  IGameSystem::~IGameSystem((IGameSystem *)(unaff_EBX + 0x39420c),in_stack_00000008);
  return;
}


/* CPortal_PointPush::CreatePortalVortexEffect at 006d67f0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
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
     (iVar4 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d53a3),
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
  fVar14 = *(float *)(unaff_EBX + 0x353cff) * (this->m_flRadius).m_Value;
  if (fVar14 * fVar14 <= fVar12 * fVar12 + fVar13 * fVar13) {
    return;
  }
  fVar12 = (this->m_flMagnitude).m_Value;
                    /* Unresolved local var: int itr@[???] */
  if (0 < *(int *)(unaff_EBX + 0x658ef7)) {
    iVar4 = 0;
    do {
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???]
                       Unresolved local var: CEntInfo * pInfo@[???] */
      uVar1 = *(uint *)(*(int *)(unaff_EBX + 0x658eeb) + 4 + iVar4 * 8);
      if (((uVar1 != 0xffffffff) &&
          (iVar6 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d53a3),
          *(uint *)(iVar6 + 8) == uVar1 >> 0x10)) && (pPortal == *(CPortal_Base2D **)(iVar6 + 4)))
      goto LAB_006d68ee;
      iVar4 = iVar4 + 1;
    } while (iVar4 != *(int *)(unaff_EBX + 0x658ef7));
  }
  this_00 = CreateEntityByName((char *)(unaff_EBX + 0x31dfe4),-1,true);
  local_a4 = 0xffffffff;
  if (this_00 != (CBaseEntity *)0x0) {
    (*(this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
      [0x20])(this_00,unaff_EBX + 0x31dffb,unaff_EBX + 0x2dd23b);
    (*(this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
      [0x20])(this_00,unaff_EBX + 0x31e020,unaff_EBX + 0x33a741);
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
    if (fVar12 < *(float *)(unaff_EBX + 0x353d6f) || fVar12 == *(float *)(unaff_EBX + 0x353d6f)) {
                    /* Unresolved local var: Vector res@[???] */
      fVar12 = *(float *)(CAI_Hint::HintMatchesCriteria + unaff_EBX + 3);
      local_30.x = local_30.x + fVar2 * fVar12;
      local_30.y = local_30.y + fVar14 * fVar12;
      local_30.z = local_30.z + fVar12 * fVar13;
    }
    else {
      uVar1 = *(uint *)(unaff_EBX + 0x35ccdf);
      local_60.x = (vec_t)((uint)fVar2 ^ uVar1);
      local_60.y = (vec_t)((uint)fVar14 ^ uVar1);
      local_60.z = (vec_t)((uint)fVar13 ^ uVar1);
      local_54 = local_60.x;
      local_50 = local_60.y;
      local_4c = local_60.z;
      VectorAngles(&local_60,&local_3c);
                    /* Unresolved local var: Vector res@[???] */
      fVar12 = *(float *)(&DAT_00353e1b + unaff_EBX);
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
  iVar6 = *(int *)(unaff_EBX + 0x658ef7);
  iVar4 = iVar6 + 1;
  iVar9 = *(int *)(unaff_EBX + 0x658eef);
  if (iVar4 <= iVar9) goto LAB_006d6c7d;
  iVar11 = *(int *)(unaff_EBX + 0x658ef3);
  if (iVar11 < 0) goto LAB_006d6c7d;
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
    goto LAB_006d6c30;
  }
  iVar9 = (iVar6 / iVar11 + 1) * iVar11;
  if (iVar9 < iVar4) {
    if (iVar9 != 0) {
LAB_006d6c13:
      do {
        iVar9 = (iVar9 + iVar4) / 2;
      } while (iVar9 < iVar4);
      goto LAB_006d6c23;
    }
    if (iVar4 < 0) {
      iVar9 = -1;
      local_90 = -8;
    }
    else {
      local_90 = 0;
      if (iVar4 != 0) goto LAB_006d6c13;
    }
  }
  else {
LAB_006d6c23:
    local_90 = iVar9 * 8;
  }
LAB_006d6c30:
  *(int *)(unaff_EBX + 0x658eef) = iVar9;
  if (*(int *)(unaff_EBX + 0x658eeb) == 0) {
    uVar10 = (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x4d5397))
                       ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x4d5397),local_90);
    *(undefined4 *)(unaff_EBX + 0x658eeb) = uVar10;
  }
  else {
    uVar10 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d5397) + 4))
                       ((int *)**(undefined4 **)(unaff_EBX + 0x4d5397),
                        *(int *)(unaff_EBX + 0x658eeb),local_90);
    *(undefined4 *)(unaff_EBX + 0x658eeb) = uVar10;
  }
LAB_006d6c7d:
  iVar11 = *(int *)(unaff_EBX + 0x658ef7) + 1;
  *(int *)(unaff_EBX + 0x658ef7) = iVar11;
  iVar9 = *(int *)(unaff_EBX + 0x658eeb);
  *(int *)(unaff_EBX + 0x658efb) = iVar9;
  iVar11 = (iVar11 - iVar6) + -1;
  if (0 < iVar11) {
    _V_memmove((void *)(iVar9 + iVar4 * 8),(void *)(iVar9 + iVar6 * 8),iVar11 * 8);
  }
  puVar8 = (undefined4 *)(iVar6 * 8 + *(int *)(unaff_EBX + 0x658eeb));
  if (puVar8 != (undefined4 *)0x0) {
    *puVar8 = local_a4;
    puVar8[1] = uVar3;
  }
LAB_006d68ee:
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


/* CPortal_PointPush::NotifyPortalEvent at 006d79f0 */

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
    if (0 < (int)*(uint *)(unaff_EBX + 0x657cfa)) {
      iVar5 = **(int **)(unaff_EBX + 0x4d41a6);
      puVar7 = *(uint **)(&DAT_00657cee + unaff_EBX);
      local_30 = 0;
      do {
        uVar6 = puVar7[1];
        if ((uVar6 != 0xffffffff) &&
           (iVar2 = (uVar6 & 0xffff) * 0x10 + iVar5, *(uint *)(iVar2 + 8) == uVar6 >> 0x10)) {
          if (pNotifier != *(CPortal_Base2D **)(iVar2 + 4)) goto LAB_006d7a73;
LAB_006d7bac:
          uVar6 = *puVar7;
          if ((uVar6 == 0xffffffff) ||
             (iVar5 = (uVar6 & 0xffff) * 0x10 + iVar5, *(uint *)(iVar5 + 8) != uVar6 >> 0x10)) {
            oldObj = (CBaseEntity *)0x0;
          }
          else {
            oldObj = *(CBaseEntity **)(iVar5 + 4);
          }
          UTIL_Remove(oldObj);
          iVar5 = ~local_30 + *(int *)(unaff_EBX + 0x657cfa);
          if (0 < iVar5) {
            _V_memmove((void *)(local_30 * 8 + *(int *)(&DAT_00657cee + unaff_EBX)),
                       (void *)(*(int *)(&DAT_00657cee + unaff_EBX) + 8 + local_30 * 8),iVar5 * 8);
          }
          *(int *)(unaff_EBX + 0x657cfa) = *(int *)(unaff_EBX + 0x657cfa) + -1;
          break;
        }
        if (pNotifier == (CPortal_Base2D *)0x0) goto LAB_006d7bac;
LAB_006d7a73:
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
        if (iVar3 == iVar2) goto LAB_006d7bac;
        local_30 = local_30 + 1;
        puVar7 = puVar7 + 2;
      } while (local_30 != *(uint *)(unaff_EBX + 0x657cfa));
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
        (iVar5 = (uVar6 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d41a6),
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
         (iVar5 = (uVar6 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d41a6),
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


/* CPortalVortexEffectManager::LevelShutdownPostEntity at 006d7d00 */

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
         (iVar6 = (uVar4 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d3e96),
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


/* DataMapInit<CPortal_PointPush> at 000c6750 */

datamap_t * DataMapInit<CPortal_PointPush>(CPortal_PointPush *param_1)

{
  int iVar1;
  int iVar2;
  char *pcVar3;
  size_t sVar4;
  int iVar5;
  undefined4 uVar6;
  undefined4 *puVar7;
  int iVar8;
  int unaff_EBX;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  if (((&vgui::ProgressBar::ChainToAnimationMap()::chained)[unaff_EBX] == '\0') &&
     (iVar2 = ___cxa_guard_acquire(&vgui::ProgressBar::ChainToAnimationMap()::chained + unaff_EBX),
     iVar2 != 0)) {
    *(int *)(&DAT_00cf3a9c + unaff_EBX) = unaff_EBX + 0x94a7cc;
    *(undefined4 *)(&DAT_00cf3aa4 + unaff_EBX) = 0;
    *(undefined4 *)((int)&vgui::RadioButton::GetKBMap::s_pMap + unaff_EBX) = 0;
    *(undefined4 *)((int)&vgui::RadioButton::GetMessageMap::s_pMap + unaff_EBX) = 0;
    *(undefined4 *)(&vgui::RadioButton::ChainToMap()::chained + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&vgui::RadioButton::PanelMessageFunc_OnRadioButtonChecked::InitVar()::bAdded +
     unaff_EBX + 1) = 0;
    *(undefined4 *)((int)&vgui::RadioButton::GetKBMap()::s_pMap + unaff_EBX) = 0x11;
    ___cxa_guard_release(&vgui::ProgressBar::ChainToAnimationMap()::chained + unaff_EBX);
    ___cxa_atexit(unaff_EBX + 0x8d4c7c,0,*(undefined4 *)(&DAT_00ae542c + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_OnRestore_00c06888 + unaff_EBX) =
       *(undefined4 *)(&DAT_00ae5568 + unaff_EBX);
  if (((&DAT_00cf3a94)[unaff_EBX] != '\0') ||
     (iVar2 = ___cxa_guard_acquire(&DAT_00cf3a94 + unaff_EBX), iVar2 == 0)) goto LAB_000c67a4;
  pcVar3 = operator_new___(*(int *)((int)&vgui::RadioButton::GetKBMap()::s_pMap + unaff_EBX) + 10);
  _strcpy(pcVar3,*(char **)(&DAT_00cf3a9c + unaff_EBX));
  sVar4 = _strlen(pcVar3);
  builtin_strncpy(pcVar3 + sVar4,"PushThink",10);
  iVar1 = *(int *)(&vgui::RadioButton::ChainToMap()::chained + unaff_EBX);
  iVar2 = iVar1 + 1;
  iVar5 = *(int *)((int)&vgui::RadioButton::GetKBMap::s_pMap + unaff_EBX);
  if (iVar5 < iVar2) {
    iVar8 = *(int *)((int)&vgui::RadioButton::GetMessageMap::s_pMap + unaff_EBX);
    if (-1 < iVar8) {
      if (iVar8 == 0) {
        if ((iVar5 == 0) && (iVar5 = 8, iVar2 < 9)) {
          local_20 = 0x20;
        }
        else {
          do {
            local_20 = iVar5;
            iVar5 = local_20 * 2;
          } while (iVar5 < iVar2);
          local_20 = local_20 * 8;
        }
        goto LAB_000c68c0;
      }
      iVar5 = (iVar1 / iVar8 + 1) * iVar8;
      if (iVar5 < iVar2) {
        if (iVar5 != 0) {
LAB_000c6880:
          do {
            iVar5 = (iVar5 + iVar2) / 2;
          } while (iVar5 < iVar2);
          goto LAB_000c6890;
        }
        if (iVar2 < 0) {
          iVar5 = -1;
          local_20 = -4;
        }
        else {
          local_20 = 0;
          if (iVar2 != 0) goto LAB_000c6880;
        }
      }
      else {
LAB_000c6890:
        local_20 = iVar5 * 4;
      }
LAB_000c68c0:
      *(int *)((int)&vgui::RadioButton::GetKBMap::s_pMap + unaff_EBX) = iVar5;
      if (*(int *)(&DAT_00cf3aa4 + unaff_EBX) == 0) {
        uVar6 = (*(code *)**(undefined4 **)**(undefined4 **)(&DAT_00ae5434 + unaff_EBX))
                          ((undefined4 *)**(undefined4 **)(&DAT_00ae5434 + unaff_EBX),local_20);
      }
      else {
        uVar6 = (**(code **)(*(int *)**(undefined4 **)(&DAT_00ae5434 + unaff_EBX) + 4))
                          ((int *)**(undefined4 **)(&DAT_00ae5434 + unaff_EBX),
                           *(int *)(&DAT_00cf3aa4 + unaff_EBX),local_20);
      }
      *(undefined4 *)(&DAT_00cf3aa4 + unaff_EBX) = uVar6;
    }
  }
  iVar5 = *(int *)(&vgui::RadioButton::ChainToMap()::chained + unaff_EBX);
  *(int *)(&vgui::RadioButton::ChainToMap()::chained + unaff_EBX) = iVar5 + 1;
  iVar8 = *(int *)(&DAT_00cf3aa4 + unaff_EBX);
  *(int *)((int)&vgui::RadioButton::PanelMessageFunc_OnRadioButtonChecked::InitVar()::bAdded +
          unaff_EBX + 1) = iVar8;
  iVar5 = ((iVar5 + 1) - iVar1) + -1;
  if (0 < iVar5) {
    _V_memmove((void *)(iVar8 + iVar2 * 4),(void *)(iVar8 + iVar1 * 4),iVar5 * 4);
    iVar8 = *(int *)(&DAT_00cf3aa4 + unaff_EBX);
  }
  puVar7 = (undefined4 *)(iVar8 + iVar1 * 4);
  if (puVar7 != (undefined4 *)0x0) {
    *puVar7 = pcVar3;
  }
  *(char **)((int)&PTR_Event_Killed_00c068e0 + unaff_EBX) = pcVar3;
  ___cxa_guard_release(&DAT_00cf3a94 + unaff_EBX);
LAB_000c67a4:
  *(undefined4 *)((int)&PTR_ShouldSavePhysics_00c06880 + unaff_EBX) = 8;
  *(int *)((int)&PTR_Restore_00c0687c + unaff_EBX) = (int)&PTR_IsAlive_00c068dc + unaff_EBX;
  return (datamap_t *)((int)&PTR_Restore_00c0687c + unaff_EBX);
}


/* CPortal_PointPush::GetDataDescMap at 006d6580 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPortal_PointPush * this) */

datamap_t * __thiscall CPortal_PointPush::GetDataDescMap(CPortal_PointPush *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5f6a58);
}


/* CPortal_PointPush::GetBaseMap at 006d6590 */

datamap_t * CPortal_PointPush::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4d5734);
}


/* __tcf_0 at 0099b3e0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41ee26)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41ee1a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41ee26));
  }
  *(undefined4 *)(unaff_EBX + 0x41ee26) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41ee22)) {
    if (*(int *)(unaff_EBX + 0x41ee1a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(&DAT_002107aa + unaff_EBX) + 8))
                ((int *)**(undefined4 **)(&DAT_002107aa + unaff_EBX),*(int *)(unaff_EBX + 0x41ee1a))
      ;
      *(undefined4 *)(unaff_EBX + 0x41ee1a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41ee1e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41ee1a);
  *(int *)(unaff_EBX + 0x41ee2a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41ee22)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(&DAT_002107aa + unaff_EBX) + 8))
                ((int *)**(undefined4 **)(&DAT_002107aa + unaff_EBX),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41ee1a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41ee1e) = 0;
  }
  return;
}


/* ServerClassInit<DT_PortalPointPush::ignored> at 000c6410 */

int ServerClassInit<DT_PortalPointPush::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&vgui::ListPanel::GetKBMap()::s_pMap + unaff_EBX + 2) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xcf3b7a);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  ((int)&vgui::TextEntry::GetMessageMap()::s_pMap + unaff_EBX + 2),
                  (char *)(unaff_EBX + 0x8ec606),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)(unaff_EBX + 0xcf3bd6),(char *)(unaff_EBX + 0x8ee976),0
                        ,(SendTable *)**(undefined4 **)(&DAT_00ae586a + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00ae580e + unaff_EBX),0x80);
      SendPropBool((SendProp_conflict *)(&vgui::TreeView::KB_ChainToMap()::chained + unaff_EBX),
                   (char *)(unaff_EBX + 0x94748e),0x3c0,1);
      SendPropFloat((SendProp_conflict *)(unaff_EBX + 0xcf3c7e),(char *)(unaff_EBX + 0x8ecbbf),0x3c4
                    ,4,0x20,0,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00ae5866 + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&physcannon_minforce.m_Value.m_nValue + unaff_EBX + 2),
                    (char *)(unaff_EBX + 0x9147f6),0x3c8,4,0x20,0,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00ae5866 + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&physcannon_maxforce.m_Value.m_pszString + unaff_EBX + 2),
                    (char *)(unaff_EBX + 0x93f1dc),0x3cc,4,0x20,0,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00ae5866 + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)
                    ((int)&physcannon_tracelength.super_IConVar._vptr_IConVar + unaff_EBX + 2),
                    (char *)(unaff_EBX + 0x94aafe),0x3d0,4,0x20,0,0.0,-121121.125,
                    *(SendVarProxyFn_conflict *)(&DAT_00ae5866 + unaff_EBX),0x80);
      ___cxa_guard_release(unaff_EBX + 0xcf3b7a);
      ___cxa_atexit(unaff_EBX + 0x8d4f12,0,*(undefined4 *)(&DAT_00ae5772 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)((int)&PTR_GetSchemeColor_00c692e4 + unaff_EBX + 2),
             (SendProp_conflict *)(unaff_EBX + 0xcf3bd6),6,*(char **)(&DAT_00b70caa + unaff_EBX));
  return 1;
}


/* CPortal_PointPush::GetServerClass at 006d65a0 */

/* DWARF original prototype: ServerClass * GetServerClass(CPortal_PointPush * this) */

ServerClass * __thiscall CPortal_PointPush::GetServerClass(CPortal_PointPush *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x6e39d8);
}


/* CPortal_PointPush::YouForgotToImplementOrDeclareServerClass at 006d65b0 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CPortal_PointPush * this)
    */

int __thiscall CPortal_PointPush::YouForgotToImplementOrDeclareServerClass(CPortal_PointPush *this)

{
  return 0;
}


/* __tcf_3 at 0099b3c0 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(extraout_ECX + 0x39433c),in_stack_00000008);
  return;
}


/* __tcf_1 at 0099b330 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41ee5b))(unaff_EBX + 0x41ee5b);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41ee07))(unaff_EBX + 0x41ee07);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41edb3))(unaff_EBX + 0x41edb3);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41ed5f))(unaff_EBX + 0x41ed5f);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41ed0b))(unaff_EBX + 0x41ed0b);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41ecb7))(unaff_EBX + 0x41ecb7);
                    /* WARNING: Could not recover jumptable at 0x0099b3b4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41ec63))();
  return;
}


/* CPortal_PointPush::DrawDebugGeometryOverlays at 006d6df0 */

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
  NDebugOverlay::Sphere(origin,*(QAngle **)(unaff_EBX + 0x4d4e46),radius,0,0xff,0,0,false,0.05);
  return;
}


/* CPortal_PointPush::Activate at 006d65e0 */

/* DWARF original prototype: void Activate(CPortal_PointPush * this) */

void __thiscall CPortal_PointPush::Activate(CPortal_PointPush *this)

{
  BASEPTR func;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((this->m_bEnabled).m_Value != false) {
    func.__delta = 0;
    func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)*(uint *)(unaff_EBX + 0x4d6113);
    CBaseEntity::ThinkSet(&this->super_CBaseEntity,func,0.0,(char *)0x0);
    CBaseEntity::SetNextThink
              (&this->super_CBaseEntity,
               *(float *)(unaff_EBX + 0x355253) + *(float *)(**(int **)(unaff_EBX + 0x4d55af) + 0xc)
               ,(char *)0x0);
  }
  CBaseEntity::Activate(&this->super_CBaseEntity);
  return;
}


/* CPortal_PointPush::UpdateTransmitState at 006d65c0 */

/* DWARF original prototype: int UpdateTransmitState(CPortal_PointPush * this) */

int __thiscall CPortal_PointPush::UpdateTransmitState(CPortal_PointPush *this)

{
  int iVar1;
  
  iVar1 = CBaseEntity::SetTransmitState(&this->super_CBaseEntity,1);
  return iVar1;
}


/* CPortal_PointPush::PushEntitiesAtPoint at 006d6f00 */

/* WARNING: Restarted to delay deadcode elimination for space: stack */
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
  QAngle *local_5cc;
  float local_5b8;
  float local_5b4;
  code *local_5b0;
  int iStack_5a4;
  CBaseEntity *local_5a0 [256];
  undefined1 local_1a0 [12];
  undefined1 local_194 [32];
  float local_174;
  int *local_154;
  float local_14c;
  float local_148;
  float local_144;
  float local_13c;
  float local_138;
  float local_134;
  undefined4 local_12c;
  undefined4 local_128;
  undefined4 local_124;
  undefined4 local_11c;
  undefined4 local_118;
  undefined4 local_114;
  undefined4 local_10c;
  undefined1 local_108;
  undefined1 local_107;
  CFlaggedEntitiesEnum local_f0;
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
  piVar9 = (int *)&stack0xfffff9c4;
  ___i686_get_pc_thunk_bx();
  fVar18 = (this->m_flRadius).m_Value;
                    /* Unresolved local var: CFlaggedEntitiesEnum sphereEnum@[???] */
  CFlaggedEntitiesEnum::CFlaggedEntitiesEnum(&local_f0,local_5a0,0x100,0);
  iVar6 = UTIL_EntitiesInSphere(vecOrigin,fVar18,&local_f0);
  if (0 < iVar6) {
    iVar14 = 1;
    do {
      piVar10 = (int *)(&iStack_5a4)[iVar14];
      if ((*(char *)((int)piVar10 + 0x19a) != '\0') && ((*(ushort *)(piVar10 + 0x66) & 4) == 0)) {
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
        uVar8 = piVar10[0x5b];
        if (((uVar8 == 0xffffffff) ||
            ((iVar7 = (uVar8 & 0xffff) * 0x10 + **(int **)(&DAT_004d4c93 + unaff_EBX),
             *(uint *)(iVar7 + 8) != uVar8 >> 0x10 || (*(int *)(iVar7 + 4) == 0)))) &&
           ((cVar4 = *(char *)((int)piVar10 + 0x16a), cVar4 == '\x06' ||
            ((cVar4 == '\x02' || (cVar4 == '\x03')))))) {
          iVar7 = *piVar10;
          *piVar9 = (int)piVar10;
          pcVar1 = *(code **)(iVar7 + 0x154);
          piVar9[-1] = 0x6d700b;
          cVar4 = (*pcVar1)();
          if (((cVar4 == '\0') || (((this->super_CBaseEntity).m_spawnflags.m_Value & 8) != 0)) &&
             ((piVar10 = (int *)(&iStack_5a4)[iVar14], *(char *)((int)piVar10 + 0x16a) != '\x06' ||
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
              piVar9[-1] = 0x6d7263;
              (*pcVar1)();
              piVar12 = piVar9 + -1;
              piVar9[-1] = (int)(fVar18 * *(float *)(unaff_EBX + 0x3535f3));
              piVar9[-2] = 0x6d7280;
              lVar15 = (longdouble)_cosf();
              piVar9[3] = (int)(float)lVar15;
              piVar9[2] = (int)vecAngles;
              piVar9[1] = (int)local_28;
              *piVar9 = (int)vecOrigin;
              piVar9[-1] = (int)this;
              piVar9[-2] = 0x6d72a1;
              bVar5 = TestWithinCone((CPortal_PointPush *)piVar9[-1],(Vector *)*piVar9,
                                     (Vector *)piVar9[1],(QAngle *)piVar9[2],(float)piVar9[3]);
              piVar9 = piVar9 + -1;
              piVar11 = piVar12;
              if (!bVar5) goto LAB_006d6fe0;
            }
            piVar10 = piVar11;
            if (pPortal != (CPortal_Base2D *)0x0) {
              if (((pPortal->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) == 0) {
                piVar9 = (int *)(&iStack_5a4)[iVar14];
                local_5b0 = *(code **)(*piVar9 + 0x230);
              }
              else {
                *piVar11 = (int)pPortal;
                piVar11[-1] = 0x6d75f2;
                CBaseEntity::CalcAbsolutePosition((CBaseEntity *)*piVar11);
                piVar9 = (int *)(&iStack_5a4)[iVar14];
                local_5b0 = *(code **)(*piVar9 + 0x230);
                if (((pPortal->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
                  *piVar11 = (int)pPortal;
                  piVar11[-1] = 0x6d7631;
                  CBaseEntity::CalcAbsolutePosition((CBaseEntity *)*piVar11);
                  piVar9 = (int *)(&iStack_5a4)[iVar14];
                }
              }
              local_5cc = &(pPortal->super_CBaseAnimating).super_CBaseEntity.m_angAbsRotation;
              pVVar13 = &(pPortal->super_CBaseAnimating).super_CBaseEntity.m_vecAbsOrigin;
              piVar11[3] = 0;
              piVar11[2] = (int)pVVar13;
              piVar11[1] = (int)piVar9;
              *piVar11 = (int)local_34;
              piVar11[-1] = 0x6d70b7;
              (*local_5b0)();
              piVar10 = piVar11 + -1;
              if (((pPortal->super_CBaseAnimating).super_CBaseEntity.m_iEFlags & 0x800) != 0) {
                piVar11[-1] = (int)pPortal;
                piVar11[-2] = 0x6d75e5;
                CBaseEntity::CalcAbsolutePosition((CBaseEntity *)piVar11[-1]);
              }
              piVar11[3] = 0;
              piVar11[2] = (int)local_5cc;
              piVar11[1] = (int)local_34;
              *piVar11 = (int)pVVar13;
              piVar11[-1] = (int)this;
              piVar11[-2] = 0x6d70f5;
              bVar5 = TestWithinCone((CPortal_PointPush *)piVar11[-1],(Vector *)*piVar11,
                                     (Vector *)piVar11[1],(QAngle *)piVar11[2],(float)piVar11[3]);
              piVar9 = piVar11 + -1;
              if (!bVar5) goto LAB_006d6fe0;
            }
            uVar8 = (this->super_CBaseEntity).m_spawnflags.m_Value;
            if ((uVar8 & 1) == 0) {
              piVar11 = piVar10;
              piVar9 = (int *)(&iStack_5a4)[iVar14];
            }
            else {
              local_7c = vecOrigin->x;
              local_78 = vecOrigin->y;
              local_74 = vecOrigin->z;
              piVar9 = (int *)(&iStack_5a4)[iVar14];
              iVar7 = *piVar9;
              piVar10[3] = 0;
              piVar10[2] = (int)&local_7c;
              piVar10[1] = (int)piVar9;
              *piVar10 = (int)&local_40;
              pcVar1 = *(code **)(iVar7 + 0x230);
              piVar10[-1] = 0x6d767d;
              (*pcVar1)();
              if ((this->m_flInnerRadius).m_Value != 0.0) {
                    /* Unresolved local var: Vector vecDirToTarget@[???]
                       Unresolved local var: Vector res@[???] */
                local_f0.super_IPartitionEnumerator._vptr_IPartitionEnumerator =
                     (_func_int_varargs **)(local_40 - local_7c);
                local_f0.m_pList = (CBaseEntity **)(local_3c - local_78);
                local_f0.m_listMax = (int)(local_38 - local_74);
                piVar10[-1] = (int)&local_f0;
                piVar10[-2] = 0x6d78e7;
                local_4c = local_f0.super_IPartitionEnumerator._vptr_IPartitionEnumerator;
                local_48 = local_f0.m_pList;
                local_44 = (float)local_f0.m_listMax;
                VectorNormalize((Vector *)piVar10[-1]);
                local_50 = (this->m_flInnerRadius).m_Value;
                    /* Unresolved local var: Vector res@[???] */
                local_58 = local_50 *
                           (float)local_f0.super_IPartitionEnumerator._vptr_IPartitionEnumerator;
                local_54 = local_50 * (float)local_f0.m_pList;
                local_50 = local_50 * (float)local_f0.m_listMax;
                    /* Unresolved local var: Vector res@[???] */
                local_7c = vecOrigin->x + local_58;
                local_78 = vecOrigin->y + local_54;
                local_74 = vecOrigin->z + local_50;
                local_64 = local_7c;
                local_60 = local_78;
                local_5c = local_74;
              }
              piVar9 = (int *)(&iStack_5a4)[iVar14];
              iVar7 = *piVar9;
              piVar10[2] = 0;
              piVar10[1] = (int)&local_7c;
              *piVar10 = (int)piVar9;
              piVar10[-1] = (int)&local_70;
              pcVar1 = *(code **)(iVar7 + 0x230);
              piVar10[-2] = 0x6d76d1;
              (*pcVar1)();
              piVar11 = piVar10 + -2;
              local_13c = local_70 - local_7c;
              local_138 = local_6c - local_78;
              local_134 = local_68 - local_74;
              local_107 = local_13c * local_13c + local_138 * local_138 + local_134 * local_134 !=
                          0.0;
              local_114 = 0;
              local_118 = 0;
              local_11c = 0;
              local_10c = 0;
              local_108 = 1;
              local_124 = 0;
              local_128 = 0;
              local_12c = 0;
              local_14c = local_7c;
              local_148 = local_78;
              local_144 = local_74;
              piVar10[1] = 0;
              *piVar10 = 0;
              piVar10[-1] = (int)this;
              piVar10[-2] = (int)&local_f0;
              piVar10[-3] = 0x6d77ae;
              CTraceFilterSimple::CTraceFilterSimple
                        ((CTraceFilterSimple *)piVar10[-2],(IHandleEntity *)piVar10[-1],*piVar10,
                         (ShouldHitFunc_t)piVar10[1]);
              piVar9 = (int *)**(undefined4 **)(unaff_EBX + 0x4d4cfb);
              iVar7 = *piVar9;
              piVar10[2] = (int)local_1a0;
              piVar10[1] = (int)&local_f0;
              *piVar10 = 0x400b;
              piVar10[-1] = (int)&local_14c;
              piVar10[-2] = (int)piVar9;
              pcVar1 = *(code **)(iVar7 + 0x14);
              piVar10[-3] = 0x6d77e5;
              (*pcVar1)();
              if (*(int *)(*(int *)(*(int *)(CMortarShell::GetBaseMap + unaff_EBX + 7) + 0x1c) +
                          0x30) != 0) {
                piVar10[4] = -0x40800000;
                piVar10[3] = 1;
                piVar10[2] = 0;
                piVar10[1] = 0;
                *piVar10 = 0xff;
                piVar10[-1] = (int)local_194;
                piVar10[-2] = (int)local_1a0;
                piVar10[-3] = 0x6d789b;
                DebugDrawLine((Vector *)piVar10[-2],(Vector *)piVar10[-1],*piVar10,piVar10[1],
                              piVar10[2],SUB41(piVar10[3],0),(float)piVar10[4]);
              }
              if (local_174 < *(float *)(unaff_EBX + 0x3533ff)) {
                piVar9 = piVar10 + -2;
                if (local_154 != (int *)(&iStack_5a4)[iVar14]) goto LAB_006d6fe0;
                uVar8 = (this->super_CBaseEntity).m_spawnflags.m_Value;
                piVar9 = local_154;
              }
              else {
                uVar8 = (this->super_CBaseEntity).m_spawnflags.m_Value;
                piVar11 = piVar10 + -2;
                piVar9 = (int *)(&iStack_5a4)[iVar14];
              }
            }
            if ((uVar8 & 2) == 0) {
              iVar7 = *piVar9;
              piVar11[3] = 0;
              piVar11[2] = (int)vecOrigin;
              piVar11[1] = (int)piVar9;
              *piVar11 = (int)&local_88;
              pcVar1 = *(code **)(iVar7 + 0x230);
              piVar11[-1] = 0x6d7142;
              (*pcVar1)();
              piVar11 = piVar11 + -1;
                    /* Unresolved local var: Vector res@[???] */
              local_f0.super_IPartitionEnumerator._vptr_IPartitionEnumerator =
                   (_func_int_varargs **)(local_88 - vecOrigin->x);
              local_f0.m_pList = (CBaseEntity **)(local_84 - vecOrigin->y);
              local_f0.m_listMax = (int)(local_80 - vecOrigin->z);
              local_94 = local_f0.super_IPartitionEnumerator._vptr_IPartitionEnumerator;
              local_90 = local_f0.m_pList;
              local_8c = (float)local_f0.m_listMax;
            }
            else {
              pp_Var2 = (this->super_CBaseEntity).super_IServerEntity.super_IServerUnknown.
                        super_IHandleEntity._vptr_IHandleEntity;
              piVar11[3] = 0;
              piVar11[2] = 0;
              piVar11[1] = (int)&local_f0;
              *piVar11 = (int)this;
              p_Var3 = pp_Var2[0x8e];
              piVar11[-1] = 0x6d7309;
              (*p_Var3)();
            }
            *piVar11 = (int)&local_f0;
            piVar11[-1] = 0x6d71a6;
            fVar18 = VectorNormalize((Vector *)*piVar11);
            if (((this->super_CBaseEntity).m_spawnflags.m_Value & 4) == 0) {
              fVar17 = (this->m_flRadius).m_Value;
              fVar16 = *(float *)(unaff_EBX + 0x3537af) * fVar17;
              if ((fVar17 != fVar16) || (NAN(fVar17) || NAN(fVar16))) {
                fVar18 = (fVar18 - fVar17) / (fVar16 - fVar17);
                fVar17 = 0.0;
                if ((0.0 <= fVar18) && (fVar17 = *(float *)(unaff_EBX + 0x3533ff), fVar18 <= fVar17)
                   ) {
                  fVar17 = fVar18 + 0.0;
                }
              }
              else {
                fVar17 = 0.0;
                if (0.0 <= fVar18 - fVar16) goto LAB_006d71f2;
              }
            }
            else {
LAB_006d71f2:
              fVar17 = *(float *)(unaff_EBX + 0x3533ff);
            }
            if (*(byte *)((int)piVar9 + 0x16a) < 9) {
                    /* WARNING: Could not recover jumptable at 0x006d7213. Too many branches */
                    /* WARNING: Treating indirect jump as call */
              (*(code *)(*(int *)(unaff_EBX + 0x307 + (uint)*(byte *)((int)piVar9 + 0x16a) * 4) +
                        unaff_EBX))();
              return;
            }
            local_b0 = (this->m_flMagnitude).m_Value;
                    /* Unresolved local var: Vector res@[???] */
            local_b8 = local_b0 *
                       (float)local_f0.super_IPartitionEnumerator._vptr_IPartitionEnumerator;
            local_b4 = local_b0 * (float)local_f0.m_pList;
            local_b0 = local_b0 * (float)local_f0.m_listMax;
                    /* Unresolved local var: Vector res@[???] */
            local_c4 = fVar17 * local_b8;
            local_c0 = fVar17 * local_b4;
            local_bc = fVar17 * local_b0;
            fVar18 = local_bc;
            local_5b8 = local_c0;
            local_5b4 = local_c4;
            if ((*(byte *)((int)piVar9 + 0x132) & 0x80) != 0) {
                    /* Unresolved local var: Vector res@[???] */
              local_5b4 = local_c4 + (float)piVar9[0x87];
              local_5b8 = local_c0 + (float)piVar9[0x88];
              fVar18 = local_bc + (float)piVar9[0x89];
              local_d0 = local_5b4;
              local_cc = local_5b8;
              local_c8 = fVar18;
            }
            if ((0.0 < fVar18) && ((*(byte *)(piVar9 + 0x4c) & 1) != 0)) {
              piVar11[1] = 0;
              *piVar11 = (int)piVar9;
              piVar11[-1] = 0x6d7972;
              CBaseEntity::SetGroundEntity((CBaseEntity *)*piVar11,(CBaseEntity *)piVar11[1]);
              if ((*(byte *)((int)piVar9 + 0x12d) & 8) != 0) {
                *piVar11 = (int)piVar9;
                piVar11[-1] = 0x6d79ea;
                CBaseEntity::CalcAbsolutePosition((CBaseEntity *)*piVar11);
              }
              local_7c = (float)piVar9[0xa5];
              local_78 = (float)piVar9[0xa6];
              local_74 = *(float *)(unaff_EBX + 0x3533ff) + (float)piVar9[0xa7];
              piVar11[1] = (int)&local_7c;
              *piVar11 = (int)piVar9;
              piVar11[-1] = 0x6d79b9;
              CBaseEntity::SetAbsOrigin((CBaseEntity *)*piVar11,(Vector *)piVar11[1]);
            }
            local_d8 = local_5b8;
            local_dc = local_5b4;
            local_d4 = fVar18;
            if ((((local_5b4 != (float)piVar9[0x87]) || (NAN(local_5b4) || NAN((float)piVar9[0x87]))
                 ) || (local_5b8 != (float)piVar9[0x88])) ||
               ((NAN(local_5b8) || NAN((float)piVar9[0x88]) || (fVar18 != (float)piVar9[0x89])))) {
              iVar7 = *piVar9;
              piVar11[1] = (int)(piVar9 + 0x87);
              *piVar11 = (int)piVar9;
              pcVar1 = *(code **)(iVar7 + 0x2e8);
              piVar11[-1] = 0x6d75b8;
              (*pcVar1)();
              piVar9[0x87] = (int)local_dc;
              piVar9[0x88] = (int)local_d8;
              piVar9[0x89] = (int)local_d4;
            }
            piVar11[1] = 0x800000;
            *piVar11 = (int)piVar9;
            piVar11[-1] = 0x6d7495;
            CBaseEntity::AddFlag((CBaseEntity *)*piVar11,piVar11[1]);
            piVar9 = piVar11;
          }
        }
      }
LAB_006d6fe0:
      iVar14 = iVar14 + 1;
    } while (iVar14 != iVar6 + 1);
  }
  return;
}


/* CPortal_PointPush::InputEnable at 006d6720 */

/* DWARF original prototype: void InputEnable(CPortal_PointPush * this, inputdata_t * inputdata) */

void __thiscall CPortal_PointPush::InputEnable(CPortal_PointPush *this,inputdata_t *inputdata)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  BASEPTR func;
  IChangeInfoAccessor *pIVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((this->m_bEnabled).m_Value == false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar2->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = &(this->super_CBaseEntity).m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_bEnabled).m_Value = true;
  }
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)*(uint *)(unaff_EBX + 0x4d5fd0);
  CBaseEntity::ThinkSet(&this->super_CBaseEntity,func,0.0,(char *)0x0);
  CBaseEntity::SetNextThink
            (&this->super_CBaseEntity,
             *(float *)(unaff_EBX + 0x355110) + *(float *)(**(int **)(unaff_EBX + 0x4d546c) + 0xc),
             (char *)0x0);
  return;
}


/* CPortal_PointPush::InputDisable at 006d6660 */

/* DWARF original prototype: void InputDisable(CPortal_PointPush * this, inputdata_t * inputdata) */

void __thiscall CPortal_PointPush::InputDisable(CPortal_PointPush *this,inputdata_t *inputdata)

{
  undefined1 *puVar1;
  CBaseEdict *this_00;
  IChangeInfoAccessor *pIVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((this->m_bEnabled).m_Value != false) {
    if ((this->super_CBaseEntity).m_Network.m_TimerEvent.m_bRegistered == false) {
      this_00 = &((this->super_CBaseEntity).m_Network.m_pPev)->super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar2 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar2->m_iChangeInfoSerialNumber = 0;
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
            (&this->super_CBaseEntity,*(float *)(**(int **)(unaff_EBX + 0x4d552c) + 0xc),(char *)0x0
            );
  return;
}


/* _GLOBAL__I_g_PortalVortexManager at 000c6d20 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_g_PortalVortexManager(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

