/* DWARF-guided pseudocode for game/server/portal2/rocketprojectile.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* DataMapInit<CRocket_Turret_Projectile> at 000b9cb0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

datamap_t * DataMapInit<CRocket_Turret_Projectile>(CRocket_Turret_Projectile *param_1)

{
  int iVar1;
  int iVar2;
  char *pcVar3;
  size_t sVar4;
  int iVar5;
  undefined4 *puVar6;
  ISaveRestoreOps *pIVar7;
  int iVar8;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((*(char *)((int)&player_held_object_use_view_model.m_pParent + unaff_EBX) == '\0') &&
     (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd9713c), iVar2 != 0)) {
    *(int *)((int)&player_held_object_use_view_model.m_Value.m_fValue + unaff_EBX) =
         unaff_EBX + 0x9fc3d1;
    *(undefined4 *)(&player_held_object_use_view_model.m_bHasMin + unaff_EBX) = 0;
    *(undefined4 *)((int)&player_held_object_use_view_model.m_fMinVal + unaff_EBX) = 0;
    *(undefined4 *)(&player_held_object_use_view_model.m_bHasMax + unaff_EBX) = 0;
    *(undefined4 *)((int)&player_held_object_use_view_model.m_fMaxVal + unaff_EBX) = 0;
    *(undefined4 *)
     ((int)&player_held_object_use_view_model.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) =
         0;
    *(undefined4 *)((int)&player_held_object_use_view_model.m_Value.m_nValue + unaff_EBX) = 0x19;
    ___cxa_guard_release(unaff_EBX + 0xd9713c);
    ___cxa_atexit(unaff_EBX + 0x98721c,0,*(undefined4 *)(&DAT_00b8ce90 + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_SetBreakableSkin_00cb18c8 + unaff_EBX) =
       *(undefined4 *)(&DAT_00b8cf70 + unaff_EBX);
  if ((*(char *)((int)&player_held_object_use_view_model.m_Value.m_pszString + unaff_EBX) == '\0')
     && (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xd97144), iVar2 != 0)) {
    pcVar3 = operator_new___(*(int *)((int)&player_held_object_use_view_model.m_Value.m_nValue +
                                     unaff_EBX) + 0xd);
    _strcpy(pcVar3,*(char **)((int)&player_held_object_use_view_model.m_Value.m_fValue + unaff_EBX))
    ;
    sVar4 = _strlen(pcVar3);
    builtin_strncpy(pcVar3 + sVar4,"MissileTouch",0xd);
    iVar1 = *(int *)((int)&player_held_object_use_view_model.m_fMaxVal + unaff_EBX);
    iVar2 = iVar1 + 1;
    iVar8 = *(int *)((int)&player_held_object_use_view_model.m_fMinVal + unaff_EBX);
    if (iVar8 < iVar2) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)(&player_held_object_use_view_model.m_bHasMin + unaff_EBX)
                 ,iVar2 - iVar8);
    }
    iVar5 = *(int *)((int)&player_held_object_use_view_model.m_fMaxVal + unaff_EBX) + 1;
    *(int *)((int)&player_held_object_use_view_model.m_fMaxVal + unaff_EBX) = iVar5;
    iVar8 = *(int *)(&player_held_object_use_view_model.m_bHasMin + unaff_EBX);
    *(int *)((int)&player_held_object_use_view_model.m_fnChangeCallbacks.m_Memory.m_pMemory +
            unaff_EBX) = iVar8;
    iVar5 = (iVar5 - iVar1) + -1;
    if (0 < iVar5) {
      _V_memmove((void *)(iVar8 + iVar2 * 4),(void *)(iVar8 + iVar1 * 4),iVar5 * 4);
      iVar8 = *(int *)(&player_held_object_use_view_model.m_bHasMin + unaff_EBX);
    }
    puVar6 = (undefined4 *)(iVar8 + iVar1 * 4);
    if (puVar6 != (undefined4 *)0x0) {
      *puVar6 = pcVar3;
    }
    *(char **)((int)&ServerClassInit<DT_StatueProp::ignored>::g_pSendTableName + unaff_EBX) = pcVar3
    ;
    pIVar7 = GetSoundSaveRestoreOps();
    *(ISaveRestoreOps **)((int)&PTR_SetRefEHandle_00cb19b0 + unaff_EBX) = pIVar7;
    ___cxa_guard_release(unaff_EBX + 0xd97144);
  }
  *(undefined4 *)((int)&PTR_SetBreakableModel_00cb18c0 + unaff_EBX) = 3;
  *(int *)((int)&PTR_GetPhysicsDamageTable_00cb18bc + unaff_EBX) =
       (int)&PTR_OnPhysGunDrop_00cb191c + unaff_EBX;
  return (datamap_t *)((int)&PTR_GetPhysicsDamageTable_00cb18bc + unaff_EBX);
}


/* __static_initialization_and_destruction_0 at 000b9ed0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  datamap_t *pdVar1;
  IEntityFactoryDictionary *pIVar2;
  int unaff_EBX;
  longlong lVar3;
  
  lVar3 = ___i686_get_pc_thunk_bx();
  if (lVar3 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)&player_held_object_distance_vm.super_ConCommandBase.m_pNext + unaff_EBX) = 0
  ;
  *(undefined1 *)((int)&player_held_object_distance_vm.super_ConCommandBase.m_pNext + unaff_EBX + 1)
       = 0;
  *(undefined1 *)((int)&player_held_object_distance_vm.super_ConCommandBase.m_pNext + unaff_EBX + 2)
       = 0;
  *(undefined1 *)((int)&player_held_object_distance_vm.super_ConCommandBase.m_pNext + unaff_EBX + 3)
       = 0;
  *(undefined4 *)(&player_held_object_distance_vm.super_ConCommandBase.m_bRegistered + unaff_EBX) =
       0;
  *(undefined4 *)((int)&player_held_object_distance_vm.super_ConCommandBase.m_pszName + unaff_EBX) =
       0;
  *(undefined4 *)
   ((int)&player_held_object_distance_vm.super_ConCommandBase.m_pszHelpString + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&player_held_object_distance_vm.super_ConCommandBase.m_nFlags + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&player_held_object_distance_vm.super_IConVar._vptr_IConVar + unaff_EBX) = 0;
  *(undefined4 *)((int)&player_held_object_distance_vm.m_pParent + unaff_EBX) = 0;
  *(undefined4 *)((int)&player_held_object_distance_vm.m_pszDefaultValue + unaff_EBX) = 0;
  *(undefined4 *)((int)&player_held_object_distance_vm.m_Value.m_pszString + unaff_EBX) = 0;
  *(undefined4 *)((int)&player_held_object_distance_vm.m_Value.m_StringLength + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&player_held_object_distance_vm.m_Value.m_fValue + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&player_held_object_distance_vm.m_Value.m_nValue + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&player_held_object_distance_vm.m_bHasMin + unaff_EBX) = 0x7f7fffff;
  *(undefined **)((int)&player_held_object_distance_vm.m_fMinVal + unaff_EBX) =
       &UNK_00b9242c + unaff_EBX;
  pdVar1 = DataMapInit<CRocket_Turret_Projectile>((CRocket_Turret_Projectile *)0x0);
  *(datamap_t **)((int)&g_pPitchLookup[0].name + unaff_EBX) = pdVar1;
  *(undefined **)(&player_held_object_distance_vm.m_bHasMax + unaff_EBX) = &UNK_00c20194 + unaff_EBX
  ;
  pIVar2 = EntityFactoryDictionary();
  (**pIVar2->_vptr_IEntityFactoryDictionary)(pIVar2,unaff_EBX + 0xd96f1c,unaff_EBX + 0x9f20fb);
  ConCommand::ConCommand
            ((ConCommand *)((int)&g_pSoundLevels[0].name + unaff_EBX),&UNK_009fc219 + unaff_EBX,
             (FnCommandCallbackV1_t)(unaff_EBX + 0x6b86b4),&UNK_009fc1d4 + unaff_EBX,0x4000,
             (FnCommandCompletionCallback)0x0);
  ___cxa_atexit(&UNK_00986fe4 + unaff_EBX,0,*(undefined4 *)(&DAT_00b8cc78 + unaff_EBX));
  return;
}


/* CRocket_Turret_Projectile::GetDataDescMap at 007721f0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: datamap_t * GetDataDescMap(CRocket_Turret_Projectile * this) */

datamap_t * __thiscall CRocket_Turret_Projectile::GetDataDescMap(CRocket_Turret_Projectile *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5f9388);
}


/* CRocket_Turret_Projectile::GetBaseMap at 00772200 */

datamap_t * CRocket_Turret_Projectile::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4d4a2c);
}


/* __tcf_0 at 00a40ee0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x40ff36)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x40ff2a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x40ff36));
  }
  *(undefined4 *)(unaff_EBX + 0x40ff36) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x40ff32)) {
    if (*(int *)(unaff_EBX + 0x40ff2a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x205c6e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x205c6e),*(int *)(unaff_EBX + 0x40ff2a));
      *(undefined4 *)(unaff_EBX + 0x40ff2a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40ff2e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x40ff2a);
  *(int *)(unaff_EBX + 0x40ff3a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x40ff32)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x205c6e) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x205c6e),iVar1);
      *(undefined4 *)(unaff_EBX + 0x40ff2a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x40ff2e) = 0;
  }
  return;
}


/* CRocket_Turret_Projectile::Spawn at 007723d0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "MoveCollide_t": Some values do not have unique names */
/* WARNING: Enum "MoveType_t": Some values do not have unique names */
/* DWARF original prototype: void Spawn(CRocket_Turret_Projectile * this) */

void __thiscall CRocket_Turret_Projectile::Spawn(CRocket_Turret_Projectile *this)

{
  int unaff_EBX;
  Vector local_34;
  Vector local_28 [2];
  
  ___i686_get_pc_thunk_bx();
  (**(code **)((int)(this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x68))(this);
  CBaseAnimating::Spawn((CBaseAnimating *)this);
  CBaseEntity::SetBlocksLOS((CBaseEntity *)this,false);
  CCollisionProperty::SetSolid
            (&(this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
              super_CBaseAnimating.super_CBaseEntity.m_Collision.super_CCollisionProperty,SOLID_BBOX
            );
  (**(code **)((int)(this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x6c))(this,unaff_EBX + 0x343c9b);
  local_34.x = 8.0;
  local_34.y = 8.0;
  local_34.z = 8.0;
  local_28[0].x = -8.0;
  local_28[0].y = -8.0;
  local_28[0].z = -8.0;
  UTIL_SetSize((CBaseEntity *)this,local_28,&local_34);
  CBaseEntity::ThinkSet((CBaseEntity *)this,(BASEPTR)0x0,0.0,(char *)0x0);
  CBaseEntity::SetMoveType((CBaseEntity *)this,MOVETYPE_FLY,MOVECOLLIDE_FLY_BOUNCE);
  if ((this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
      super_CBaseAnimating.super_CBaseEntity.m_takedamage.m_Value != '\x02') {
                    /* Unresolved local var: CBaseEntity * pThis@[???] */
    (**(code **)((int)(this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay
                      .super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
                      super_IServerUnknown + 0x210))
              (this,&(this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                     super_CBaseAnimating.super_CBaseEntity.m_takedamage);
    (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
    super_CBaseAnimating.super_CBaseEntity.m_takedamage.m_Value = '\x02';
  }
  if ((this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
      super_CBaseAnimating.super_CBaseEntity.m_iMaxHealth.m_Value != 100) {
                    /* Unresolved local var: CBaseEntity * pThis@[???] */
    (**(code **)((int)(this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay
                      .super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
                      super_IServerUnknown + 0x1f8))
              (this,&(this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                     super_CBaseAnimating.super_CBaseEntity.m_iMaxHealth);
    (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
    super_CBaseAnimating.super_CBaseEntity.m_iMaxHealth.m_Value = 100;
  }
  if ((this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
      super_CBaseAnimating.super_CBaseEntity.m_iHealth.m_Value != 100) {
                    /* Unresolved local var: CBaseEntity * pThis@[???] */
    (**(code **)((int)(this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay
                      .super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
                      super_IServerUnknown + 0x200))
              (this,&(this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                     super_CBaseAnimating.super_CBaseEntity.m_iHealth);
    (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
    super_CBaseAnimating.super_CBaseEntity.m_iHealth.m_Value = 100;
  }
  (this->super_CBaseCombatCharacter).m_bloodColor = -1;
  (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_pfnTouch.__pfn =
       (_func___thiscall_void_CBaseEntity_ptr_CBaseEntity_ptr *)0x565;
  (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
  super_CBaseAnimating.super_CBaseEntity.m_pfnTouch.__delta = 0;
  (**(code **)((int)(this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x56c))(this);
  (**(code **)((int)(this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x560))(this);
  CBaseEntity::EmitSound((CBaseEntity *)this,(char *)(unaff_EBX + 0x343c80),0.0,(float *)0x0);
  return;
}


/* CRocket_Turret_Projectile::MissileTouch at 00772bb0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void MissileTouch(CRocket_Turret_Projectile * this, CBaseEntity *
   pOther) */

void __thiscall
CRocket_Turret_Projectile::MissileTouch(CRocket_Turret_Projectile *this,CBaseEntity *pOther)

{
  float fVar1;
  float fVar2;
  float fVar3;
  uint uVar4;
  undefined4 *puVar5;
  CBaseParticleEntity *this_00;
  int iVar6;
  CBaseEntity *pCVar7;
  int unaff_EBX;
  QAngle local_64;
  QAngle local_58;
  Vector local_4c;
  float local_40;
  float local_3c;
  undefined4 local_38;
  Vector local_34;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  
                    /* Unresolved local var: Vector vVel@[???] */
  ___i686_get_pc_thunk_bx();
  if (((this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x1000) != 0) {
    CBaseEntity::CalcAbsoluteVelocity((CBaseEntity *)this);
  }
  fVar1 = (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
          super_CBaseAnimating.super_CBaseEntity.m_vecAbsVelocity.x;
  fVar2 = (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
          super_CBaseAnimating.super_CBaseEntity.m_vecAbsVelocity.y;
  uVar4 = (this->m_hLauncher).super_CBaseHandle.m_Index;
  if ((uVar4 == 0xffffffff) ||
     (iVar6 = (uVar4 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d3fa5),
     *(uint *)(iVar6 + 8) != uVar4 >> 0x10)) {
    pCVar7 = (CBaseEntity *)0x0;
  }
  else {
    pCVar7 = *(CBaseEntity **)(iVar6 + 4);
  }
  if (pOther == pCVar7) {
                    /* Unresolved local var: Vector vBounceVel@[???]
                       Unresolved local var: QAngle vBounceAngles@[???] */
    puVar5 = *(undefined4 **)(unaff_EBX + 0x4d3fb1);
    local_28 = *puVar5;
    local_24 = puVar5[1];
    local_20 = puVar5[2];
    fVar3 = *(float *)(unaff_EBX + 0x359d4d);
    (**(code **)((int)(this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay
                      .super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
                      super_IServerUnknown + 0x394))
              (this,0,*(float *)(**(int **)(&DAT_004d3f9d + unaff_EBX) + 0xc) + fVar3,0,0,local_28,
               local_24,local_20,0);
    local_40 = -fVar1;
    local_3c = -fVar2;
    local_38 = 0x43480000;
    local_34.z = 200.0;
                    /* Unresolved local var: Vector res@[???] */
    local_4c.x = local_40 * fVar3;
    local_4c.y = local_3c * fVar3;
    local_4c.z = fVar3 * 200.0;
    local_34.x = local_40;
    local_34.y = local_3c;
    CBaseEntity::SetAbsVelocity((CBaseEntity *)this,&local_4c);
    VectorAngles(&local_34,&local_58);
    CBaseEntity::SetAbsAngles((CBaseEntity *)this,&local_58);
    local_64.x = 180.0;
    local_64.y = 90.0;
    local_64.z = 45.0;
    CBaseEntity::SetLocalAngularVelocity((CBaseEntity *)this,&local_64);
    uVar4 = (this->m_hRocketTrail).super_CBaseHandle.m_Index;
    if ((uVar4 == 0xffffffff) ||
       (iVar6 = (uVar4 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d3fa5),
       *(uint *)(iVar6 + 8) != uVar4 >> 0x10)) {
      pCVar7 = (CBaseEntity *)0x0;
    }
    else {
      pCVar7 = *(CBaseEntity **)(iVar6 + 4);
    }
    UTIL_Remove(pCVar7);
    CCollisionProperty::SetSolid
              (&(this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                super_CBaseAnimating.super_CBaseEntity.m_Collision.super_CCollisionProperty,
               SOLID_NONE);
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
    uVar4 = (this->m_hRocketTrail).super_CBaseHandle.m_Index;
    if (((uVar4 != 0xffffffff) &&
        (iVar6 = (uVar4 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d3fa5),
        *(uint *)(iVar6 + 8) == uVar4 >> 0x10)) &&
       (this_00 = *(CBaseParticleEntity **)(iVar6 + 4), this_00 != (CBaseParticleEntity *)0x0)) {
      CBaseParticleEntity::SetLifetime(this_00,0.1);
      (this->m_hRocketTrail).super_CBaseHandle.m_Index = 0xffffffff;
    }
  }
  else if ((((pOther->m_Collision).super_CCollisionProperty.m_usSolidFlags.m_Value & 0x28) == 0) ||
          ((pOther->m_CollisionGroup).m_Value == 0xb)) {
    Explode(this);
  }
  return;
}


/* CRocket_Turret_Projectile::Explode at 00772710 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void Explode(CRocket_Turret_Projectile * this) */

void __thiscall CRocket_Turret_Projectile::Explode(CRocket_Turret_Projectile *this)

{
  uint uVar1;
  CBaseParticleEntity *this_00;
  bool bVar2;
  int iVar3;
  int unaff_EBX;
  CTakeDamageInfo *in_stack_fffffe8c;
  CBaseEntity *in_stack_fffffe90;
  CBaseEntity *in_stack_fffffe94;
  float in_stack_fffffe98;
  int in_stack_fffffe9c;
  CTakeDamageInfo *this_01;
  Vector local_150;
  Vector local_144 [2];
  float local_124;
  byte local_10e;
  CBaseEntity *local_104;
  float local_fc;
  float local_f8;
  float local_f4;
  float local_ec;
  float local_e8;
  float local_e4;
  undefined4 local_dc;
  undefined4 local_d8;
  undefined4 local_d4;
  undefined4 local_cc;
  undefined4 local_c8;
  undefined4 local_c4;
  undefined4 local_bc;
  undefined1 local_b8;
  undefined1 local_b7;
  CTakeDamageInfo local_ac;
  float local_5c;
  float local_58;
  float local_54;
  float local_50;
  float local_4c;
  float local_48;
  float local_44;
  float local_40;
  float local_3c;
  float local_38;
  float local_34;
  float local_30;
  float local_2c;
  float local_28;
  float local_24;
  char *local_20;
  
                    /* Unresolved local var: Vector forward@[???]
                       Unresolved local var: trace_t tr@[???] */
  ___i686_get_pc_thunk_bx();
  (**(code **)((int)(this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x238))(this,&local_2c,0,0);
                    /* Unresolved local var: Vector res@[???] */
  local_30 = *(float *)(CC_AI_GraphConnect + unaff_EBX + 3);
  local_38 = local_2c * local_30;
  local_34 = local_28 * local_30;
  local_30 = local_30 * local_24;
  if (((this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
                    /* Unresolved local var: Vector res@[???] */
  local_fc = (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
             super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.x;
  local_44 = local_fc + local_38;
  local_f8 = (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
             super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.y;
  local_40 = local_f8 + local_34;
  local_f4 = (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
             super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z;
  local_3c = local_f4 + local_30;
  if (((this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    local_fc = (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
               super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.x;
    local_f8 = (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
               super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.y;
    local_f4 = (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
               super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z;
  }
  local_ec = local_44 -
             (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
             super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.x;
  local_e8 = local_40 -
             (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
             super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.y;
  local_e4 = local_3c -
             (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
             super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z;
  local_b7 = local_ec * local_ec + local_e8 * local_e8 + local_e4 * local_e4 != 0.0;
  local_c4 = 0;
  local_c8 = 0;
  local_cc = 0;
  local_bc = 0;
  local_b8 = 1;
  local_d4 = 0;
  local_d8 = 0;
  local_dc = 0;
  this_01 = &local_ac;
  CTraceFilterSimple::CTraceFilterSimple
            ((CTraceFilterSimple *)this_01,(IHandleEntity *)this,0,(ShouldHitFunc_t)0x0);
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d44af) + 0x14))
            ((int *)**(undefined4 **)(unaff_EBX + 0x4d44af),&local_fc,0x46004003,this_01,&local_150)
  ;
  if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x4d44ab) + 0x1c) + 0x30) != 0) {
    in_stack_fffffe8c = (CTakeDamageInfo *)0xbf800000;
    DebugDrawLine(&local_150,local_144,0xff,0,0,true,-1.0);
  }
  if ((this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
      super_CBaseAnimating.super_CBaseEntity.m_takedamage.m_Value != '\0') {
                    /* Unresolved local var: CBaseEntity * pThis@[???] */
    (**(code **)((int)(this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay
                      .super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
                      super_IServerUnknown + 0x210))
              (this,&(this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                     super_CBaseAnimating.super_CBaseEntity.m_takedamage);
    (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
    super_CBaseAnimating.super_CBaseEntity.m_takedamage.m_Value = '\0';
  }
  CCollisionProperty::SetSolid
            (&(this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
              super_CBaseAnimating.super_CBaseEntity.m_Collision.super_CCollisionProperty,SOLID_NONE
            );
  if ((*(float *)(unaff_EBX + 0x35a1ef) == local_124) || ((local_10e & 4) == 0)) {
    (**(code **)((int)(this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay
                      .super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
                      super_IServerUnknown + 0x568))(this);
  }
  if ((local_104 != (CBaseEntity *)0x0) &&
     ((local_20 = (char *)(unaff_EBX + 0x32bab3), (local_104->m_iClassname).pszValue == local_20 ||
      (bVar2 = CBaseEntity::ClassMatchesComplex(local_104,local_20), bVar2)))) {
    CTakeDamageInfo::CTakeDamageInfo
              (this_01,(CBaseEntity *)this,(CBaseEntity *)this,(float)(local_104->m_iHealth).m_Value
               ,0x40,0,in_stack_fffffe8c,in_stack_fffffe90,in_stack_fffffe94,in_stack_fffffe98,
               in_stack_fffffe9c,(int)this_01);
    if (((this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
         super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
    local_ac.m_vecDamagePosition.x =
         (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
         super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.x;
    local_ac.m_vecDamagePosition.y =
         (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
         super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.y;
    local_ac.m_vecDamagePosition.z =
         (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
         super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z;
    if (((this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
         super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
      CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    }
    local_54 = (*(float (*) [4])
                 ((int)(this->super_CBaseCombatCharacter).super_CBaseFlex.
                       super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                       m_rgflCoordinateFrame.m_flMatVal + 0x20))[0];
    local_58 = (*(float (*) [4])
                 ((int)(this->super_CBaseCombatCharacter).super_CBaseFlex.
                       super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                       m_rgflCoordinateFrame.m_flMatVal + 0x10))[0];
    local_5c = (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
               super_CBaseAnimating.super_CBaseEntity.m_rgflCoordinateFrame.m_flMatVal[0][0];
                    /* Unresolved local var: Vector res@[???] */
    local_ac.m_vecDamageForce.z = *(float *)(unaff_EBX + 0x35b34f);
    local_ac.m_vecDamageForce.x = local_5c * local_ac.m_vecDamageForce.z;
    local_ac.m_vecDamageForce.y = local_58 * local_ac.m_vecDamageForce.z;
    local_ac.m_vecDamageForce.z = local_54 * local_ac.m_vecDamageForce.z;
    local_50 = local_ac.m_vecDamageForce.x;
    local_4c = local_ac.m_vecDamageForce.y;
    local_48 = local_ac.m_vecDamageForce.z;
    CBaseEntity::TakeDamage(local_104,this_01);
  }
  uVar1 = (this->m_hRocketTrail).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar3 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d444b),
      *(uint *)(iVar3 + 8) == uVar1 >> 0x10)) &&
     (this_00 = *(CBaseParticleEntity **)(iVar3 + 4), this_00 != (CBaseParticleEntity *)0x0)) {
    CBaseParticleEntity::SetLifetime(this_00,0.1);
    (this->m_hRocketTrail).super_CBaseHandle.m_Index = 0xffffffff;
  }
  UTIL_Remove((CBaseEntity *)this);
  return;
}


/* CRocket_Turret_Projectile::Precache at 00772260 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void Precache(CRocket_Turret_Projectile * this) */

void __thiscall CRocket_Turret_Projectile::Precache(CRocket_Turret_Projectile *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseCombatCharacter::Precache(&this->super_CBaseCombatCharacter);
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x343dd4));
  CBaseEntity::PrecacheScriptSound((char *)(unaff_EBX + 0x343df2));
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x343e0d));
  return;
}


/* CRocket_Turret_Projectile::NotifySystemEvent at 00772330 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void NotifySystemEvent(CRocket_Turret_Projectile * this, CBaseEntity *
   pNotify, notify_system_event_t eventType, notify_system_event_params_t * params) */

void __thiscall
CRocket_Turret_Projectile::NotifySystemEvent
          (CRocket_Turret_Projectile *this,CBaseEntity *pNotify,notify_system_event_t eventType,
          notify_system_event_params_t *params)

{
  uint uVar1;
  int iVar2;
  CBaseEntity *oldObj;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (eventType != NOTIFY_EVENT_TELEPORT) {
    return;
  }
  (**(code **)((int)(this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x4c))(this,0);
  uVar1 = (this->m_hRocketTrail).super_CBaseHandle.m_Index;
  if ((uVar1 == 0xffffffff) ||
     (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d482b),
     *(uint *)(iVar2 + 8) != uVar1 >> 0x10)) {
    oldObj = (CBaseEntity *)0x0;
  }
  else {
    oldObj = *(CBaseEntity **)(iVar2 + 4);
  }
  UTIL_Remove(oldObj);
  (this->m_hRocketTrail).super_CBaseHandle.m_Index = 0xffffffff;
                    /* WARNING: Could not recover jumptable at 0x007723ba. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)((int)(this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x560))();
  return;
}


/* CRocket_Turret_Projectile::SetLauncher at 00772210 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void SetLauncher(CRocket_Turret_Projectile * this, EHANDLE * hLauncher)
    */

void __thiscall
CRocket_Turret_Projectile::SetLauncher(CRocket_Turret_Projectile *this,EHANDLE *hLauncher)

{
  (this->m_hLauncher).super_CBaseHandle.m_Index = (hLauncher->super_CBaseHandle).m_Index;
  return;
}


/* CRocket_Turret_Projectile::DoExplosion at 00774640 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void DoExplosion(CRocket_Turret_Projectile * this) */

void __thiscall CRocket_Turret_Projectile::DoExplosion(CRocket_Turret_Projectile *this)

{
  float fVar1;
  uint uVar2;
  CBaseEntity *pCVar3;
  string_t *psVar4;
  int iVar5;
  string_t *psVar6;
  string_t *psVar7;
  string_t *psVar8;
  int iVar9;
  float *pfVar10;
  int unaff_EBX;
  QAngle *angles;
  CTakeDamageInfo *this_1;
  CRocket_Turret_Projectile *pInflictor_1;
  CBaseEntity *pAttacker_1;
  float flDamage_1;
  int in_stack_fffffc30;
  int local_3a0;
  CBaseEntity *local_390;
  CBaseEntity *local_380 [128];
  Vector local_180;
  Vector local_174 [2];
  float local_154;
  vec_t local_12c;
  vec_t local_128;
  vec_t local_124;
  float local_11c;
  float local_118;
  float local_114;
  undefined4 local_10c;
  undefined4 local_108;
  undefined4 local_104;
  undefined4 local_fc;
  undefined4 local_f8;
  undefined4 local_f4;
  undefined4 local_ec;
  undefined1 local_e8;
  undefined1 local_e7;
  CTakeDamageInfo local_d4;
  CFlaggedEntitiesEnum local_84;
  Vector local_70;
  float local_64;
  float local_60;
  float local_5c;
  float local_58;
  float local_54;
  float local_50;
  Vector local_4c;
  float local_40;
  float local_3c;
  float local_38;
  Vector local_34;
  Vector local_28 [2];
  
                    /* Unresolved local var: string_t strTurretName@[???]
                       Unresolved local var: string_t strPropName@[???]
                       Unresolved local var: string_t strCubeName@[???]
                       Unresolved local var: Vector vecRadius@[???]
                       Unresolved local var: CBaseEntity *[128] pEnts@[???]
                       Unresolved local var: int nNumFound@[???] */
  ___i686_get_pc_thunk_bx();
  uVar2 = (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
          super_CBaseAnimating.super_CBaseEntity.m_hOwnerEntity.
          super_CNetworkVarBase<CBaseHandle,CBaseEntity::NetworkVar_m_hOwnerEntity>.m_Value.m_Index;
  if ((uVar2 == 0xffffffff) ||
     (iVar5 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d251b),
     *(uint *)(iVar5 + 8) != uVar2 >> 0x10)) {
    local_390 = (CBaseEntity *)0x0;
  }
  else {
    local_390 = *(CBaseEntity **)(iVar5 + 4);
  }
  if ((((this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
        super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) &&
     (CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this),
     ((this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
      super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0)) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
  angles = &(this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
            super_CBaseAnimating.super_CBaseEntity.m_angAbsRotation;
  iVar5 = 0;
  flDamage_1 = 0.0;
  pAttacker_1 = (CBaseEntity *)0xffffffff;
  this_1 = (CTakeDamageInfo *)0x42c80000;
  pInflictor_1 = this;
  ExplosionCreate(&(this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                   super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin,angles,local_390,200,100,
                  0x428,100.0,(CBaseEntity *)this,-1,(EHANDLE *)0x0,CLASS_NONE);
  psVar6 = AllocPooledString((string_t *)(unaff_EBX + 0x2ecab3),(char *)angles);
  psVar7 = AllocPooledString((string_t *)(unaff_EBX + 0x2eb64b),(char *)angles);
  psVar8 = AllocPooledString((string_t *)(unaff_EBX + 0x33cc73),(char *)angles);
  if (((this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
  }
                    /* Unresolved local var: Vector res@[???] */
  fVar1 = *(float *)(unaff_EBX + 0x3596f7);
  local_28[0].x =
       (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.x;
  local_34.x = local_28[0].x + fVar1;
  local_28[0].y =
       (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.y;
  local_34.y = local_28[0].y + fVar1;
  local_28[0].z =
       (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z;
  local_34.z = local_28[0].z + fVar1;
  if (((this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
       super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
    CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
    local_28[0].x =
         (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
         super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.x;
    local_28[0].y =
         (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
         super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.y;
    local_28[0].z =
         (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
         super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z;
  }
                    /* Unresolved local var: Vector res@[???] */
  local_28[0].x = local_28[0].x - fVar1;
  local_28[0].y = local_28[0].y - fVar1;
  local_28[0].z = local_28[0].z - fVar1;
                    /* Unresolved local var: CFlaggedEntitiesEnum boxEnum@[???] */
  CFlaggedEntitiesEnum::CFlaggedEntitiesEnum(&local_84,local_380,0x80,0);
  iVar9 = UTIL_EntitiesInBox(local_28,&local_34,&local_84);
  if (0 < iVar9) {
    local_3a0 = 0;
    do {
      pCVar3 = local_380[local_3a0];
      if ((pCVar3 != (CBaseEntity *)0x0) &&
         (((psVar4 = (string_t *)(pCVar3->m_iClassname).pszValue, psVar6 == psVar4 ||
           (psVar4 == psVar7)) || (psVar4 == psVar8)))) {
        pfVar10 = (float *)(*(pCVar3->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
                             _vptr_IHandleEntity[0x9d])(pCVar3);
        if (((this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
             super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
          CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
        }
        local_ec = 0;
        local_11c = *pfVar10 -
                    (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                    super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.x;
        local_118 = pfVar10[1] -
                    (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                    super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.y;
        local_114 = pfVar10[2] -
                    (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                    super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z;
        local_e7 = local_11c * local_11c + local_118 * local_118 + local_114 * local_114 != 0.0;
        local_f4 = 0;
        local_f8 = 0;
        local_fc = 0;
        local_e8 = 1;
        local_104 = 0;
        local_108 = 0;
        local_10c = 0;
        local_12c = (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                    super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.x;
        local_128 = (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                    super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.y;
        local_124 = (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                    super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z;
        CTraceFilterSimple::CTraceFilterSimple
                  ((CTraceFilterSimple *)&local_84,(IHandleEntity *)this,0,(ShouldHitFunc_t)0x0);
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4d257f) + 0x14))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x4d257f),&local_12c,0x400b,&local_84,
                   &local_180);
        if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x4d257b) + 0x1c) + 0x30) != 0) {
          this_1 = (CTakeDamageInfo *)0xbf800000;
          DebugDrawLine(&local_180,local_174,0xff,0,0,true,-1.0);
        }
        if (*(float *)(unaff_EBX + 0x3582bf) <= local_154) {
          (**(code **)((int)(this->super_CBaseCombatCharacter).super_CBaseFlex.
                            super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                            super_IServerEntity.super_IServerUnknown + 0x238))(this,&local_40,0,0);
                    /* Unresolved local var: Vector res@[???] */
          pfVar10 = (float *)(unaff_EBX + 0x3584ef);
          local_58 = *pfVar10 * local_40;
          local_54 = *pfVar10 * local_3c;
          local_50 = *pfVar10 * local_38;
          if (((this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
               super_CBaseAnimating.super_CBaseEntity.m_iEFlags & 0x800) != 0) {
            CBaseEntity::CalcAbsolutePosition((CBaseEntity *)this);
          }
                    /* Unresolved local var: Vector res@[???] */
          local_64 = (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                     super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.x - local_58;
          local_60 = (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                     super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.y - local_54;
          local_5c = (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                     super_CBaseAnimating.super_CBaseEntity.m_vecAbsOrigin.z - local_50;
          local_4c.x = local_64;
          local_4c.y = local_60;
          local_4c.z = local_5c;
          CTakeDamageInfo::CTakeDamageInfo
                    (&local_d4,(CBaseEntity *)this,(CBaseEntity *)this,200.0,0x40,0,this_1,
                     (CBaseEntity *)pInflictor_1,pAttacker_1,flDamage_1,iVar5,in_stack_fffffc30);
          local_d4.m_vecDamagePosition.x = local_4c.x;
          local_d4.m_vecDamagePosition.y = local_4c.y;
          local_d4.m_vecDamagePosition.z = local_4c.z;
          pCVar3 = local_380[local_3a0];
          if ((pCVar3->m_iEFlags & 0x800) != 0) {
            CBaseEntity::CalcAbsolutePosition(pCVar3);
          }
                    /* Unresolved local var: Vector res@[???] */
          local_70.x = (pCVar3->m_vecAbsOrigin).x - local_4c.x;
          local_70.y = (pCVar3->m_vecAbsOrigin).y - local_4c.y;
          local_70.z = (pCVar3->m_vecAbsOrigin).z - local_4c.z;
          CalculateExplosiveDamageForce(&local_d4,&local_70,&local_4c,1.0);
          pCVar3 = local_380[local_3a0];
          (*(pCVar3->super_IServerEntity).super_IServerUnknown.super_IHandleEntity.
            _vptr_IHandleEntity[0xa3])(pCVar3,&local_d4);
        }
      }
      local_3a0 = local_3a0 + 1;
    } while (local_3a0 != iVar9);
  }
  return;
}


/* CRocket_Turret_Projectile::CreateSounds at 007744b0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void CreateSounds(CRocket_Turret_Projectile * this) */

void __thiscall CRocket_Turret_Projectile::CreateSounds(CRocket_Turret_Projectile *this)

{
  edict_t *peVar1;
  CSoundEnvelopeController *pCVar2;
  CSoundPatch *__in_chrg;
  int iVar3;
  int unaff_EBX;
  CRecipientFilter local_54;
  Vector local_34;
  Vector local_28 [2];
  
  ___i686_get_pc_thunk_bx();
  if (this->m_pAmbientSound == (CSoundPatch *)0x0) {
                    /* Unresolved local var: CSoundEnvelopeController * controller@[???]
                       Unresolved local var: CPASAttenuationFilter filter@[???] */
    pCVar2 = CSoundEnvelopeController::GetController();
    (**(code **)((int)(this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay
                      .super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
                      super_IServerUnknown + 0x278))(local_28,this);
    CRecipientFilter::CRecipientFilter(&local_54);
    local_54.super_IRecipientFilter._vptr_IRecipientFilter =
         (_func_int_varargs **)(unaff_EBX + 0x4d4ca1);
    CRecipientFilter::AddRecipientsByPAS(&local_54,local_28);
    local_54.super_IRecipientFilter._vptr_IRecipientFilter =
         (_func_int_varargs **)(CBreakable::GetDataDescMap + unaff_EBX + 1);
    (**(code **)((int)(this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay
                      .super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
                      super_IServerUnknown + 0x278))(&local_34,this);
    CPASAttenuationFilter::Filter((CPASAttenuationFilter *)&local_54,&local_34,0.8);
    peVar1 = (this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
             super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev;
                    /* Unresolved local var: int edictIndex@[???] */
    iVar3 = 0;
    if (peVar1 != (edict_t *)0x0) {
      iVar3 = (int)peVar1 - *(int *)(**(int **)(unaff_EBX + 0x4d269d) + 0x58) >> 4;
    }
    __in_chrg = (CSoundPatch *)
                (*pCVar2->_vptr_CSoundEnvelopeController[6])
                          (pCVar2,&local_54,iVar3,unaff_EBX + 0x341b79);
    this->m_pAmbientSound = __in_chrg;
    (*pCVar2->_vptr_CSoundEnvelopeController[2])(pCVar2,__in_chrg,0x3f800000,0x42c80000,0);
    local_54.super_IRecipientFilter._vptr_IRecipientFilter =
         (_func_int_varargs **)(unaff_EBX + 0x4d4ca1);
    CRecipientFilter::~CRecipientFilter(&local_54,(int)__in_chrg);
  }
  return;
}


/* CRocket_Turret_Projectile::StopLoopingSounds at 00772230 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void StopLoopingSounds(CRocket_Turret_Projectile * this) */

void __thiscall CRocket_Turret_Projectile::StopLoopingSounds(CRocket_Turret_Projectile *this)

{
  CSoundEnvelopeController *pCVar1;
  
                    /* Unresolved local var: CSoundEnvelopeController * controller@[???] */
  pCVar1 = CSoundEnvelopeController::GetController();
  (*pCVar1->_vptr_CSoundEnvelopeController[0xb])(pCVar1,this->m_pAmbientSound);
  this->m_pAmbientSound = (CSoundPatch *)0x0;
  return;
}


/* CRocket_Turret_Projectile::CreateSmokeTrail at 00772e40 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Restarted to delay deadcode elimination for space: stack */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void CreateSmokeTrail(CRocket_Turret_Projectile * this) */

void __thiscall CRocket_Turret_Projectile::CreateSmokeTrail(CRocket_Turret_Projectile *this)

{
  float fVar1;
  ushort uVar2;
  CBaseEdict *pCVar3;
  ushort *puVar4;
  int *piVar5;
  int iVar6;
  RocketTrail *pRVar7;
  uint *puVar8;
  int iVar9;
  IChangeInfoAccessor *pIVar10;
  CBaseParticleEntity *this_00;
  uint uVar11;
  uint uVar12;
  int unaff_EBX;
  uint local_154;
  uint local_150;
  uint local_14c;
  uint local_148;
  uint local_144;
  uint local_140;
  uint local_13c;
  uint local_138;
  uint local_134;
  uint local_130;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
  uVar11 = (this->m_hRocketTrail).super_CBaseHandle.m_Index;
  if (((uVar11 != 0xffffffff) &&
      (iVar6 = (uVar11 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d3d1b),
      *(uint *)(iVar6 + 8) == uVar11 >> 0x10)) && (*(int *)(iVar6 + 4) != 0)) {
    return;
  }
  pRVar7 = RocketTrail::CreateRocketTrail();
  if (pRVar7 == (RocketTrail *)0x0) {
    (this->m_hRocketTrail).super_CBaseHandle.m_Index = 0xffffffff;
    return;
  }
  puVar8 = (uint *)(*(pRVar7->super_CBaseParticleEntity).super_CBaseEntity.super_IServerEntity.
                     super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[3])(pRVar7);
  uVar11 = *puVar8;
  (this->m_hRocketTrail).super_CBaseHandle.m_Index = uVar11;
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
  iVar6 = **(int **)(unaff_EBX + 0x4d3d1b);
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
  if (uVar11 == 0xffffffff) {
    return;
  }
  uVar12 = uVar11 & 0xffff;
  iVar9 = uVar12 * 0x10 + iVar6;
  if (*(uint *)(iVar9 + 8) != uVar11 >> 0x10) {
    return;
  }
  iVar9 = *(int *)(iVar9 + 4);
  if (iVar9 == 0) {
    return;
  }
  if (*(float *)(&DAT_00359d27 + unaff_EBX) != *(float *)(iVar9 + 0x3d8)) {
    if (*(char *)(iVar9 + 0x60) == '\0') {
      pCVar3 = *(CBaseEdict **)(iVar9 + 0x24);
      if ((pCVar3 != (CBaseEdict *)0x0) && ((pCVar3->m_fStateFlags & 0x100U) == 0)) {
        pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar3);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4d3d17);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar4) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar4[uVar11 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_007741a5:
            puVar4[(uint)uVar2 + uVar11 * 0x14 + 1] = 0x3d8;
            puVar4[uVar11 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar4[uVar11 * 0x14 + 1] != 0x3d8) {
            local_154 = 0;
            do {
              uVar12 = local_154 + 1;
              local_154 = uVar12 & 0xffff;
              if ((ushort)uVar12 == uVar2) {
                if (uVar2 == 0x13) goto LAB_0077434c;
                goto LAB_007741a5;
              }
            } while (puVar4[uVar11 * 0x14 + local_154 + 1] != 0x3d8);
          }
        }
        else if (puVar4[0x7d1] == 100) {
LAB_0077434c:
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4d3d17) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4d3d17) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x4d3d17);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar6 = *piVar5 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar6 + 2) = 0x3d8;
          *(undefined2 *)(iVar6 + 0x28) = 1;
        }
      }
    }
    else {
      *(uint *)(iVar9 + 100) = *(uint *)(iVar9 + 100) | 1;
    }
    *(undefined4 *)(iVar9 + 0x3d8) = 0x3e4ccccd;
    iVar6 = **(int **)(unaff_EBX + 0x4d3d1b);
    uVar11 = (this->m_hRocketTrail).super_CBaseHandle.m_Index;
    if (uVar11 == 0xffffffff) {
      iVar6 = 0;
      goto LAB_00772f00;
    }
    uVar12 = uVar11 & 0xffff;
    if (uVar11 >> 0x10 != *(uint *)(iVar6 + 8 + uVar12 * 0x10)) {
      iVar6 = 0;
      goto LAB_00772f00;
    }
  }
  iVar6 = *(int *)(iVar6 + 4 + uVar12 * 0x10);
LAB_00772f00:
  if (*(float *)(unaff_EBX + 0x359dc7) != *(float *)(iVar6 + 0x3dc)) {
    if (*(char *)(iVar6 + 0x60) == '\0') {
      pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
      if ((pCVar3 != (CBaseEdict *)0x0) && ((pCVar3->m_fStateFlags & 0x100U) == 0)) {
        pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar3);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4d3d17);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar4) {
          uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar4[uVar11 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_00773fff:
            puVar4[(uint)uVar2 + uVar11 * 0x14 + 1] = 0x3dc;
            puVar4[uVar11 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar4[uVar11 * 0x14 + 1] != 0x3dc) {
            local_150 = 0;
            do {
              uVar12 = local_150 + 1;
              local_150 = uVar12 & 0xffff;
              if ((ushort)uVar12 == uVar2) {
                if (uVar2 != 0x13) goto LAB_00773fff;
                pIVar10->m_iChangeInfoSerialNumber = 0;
                pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar11 * 0x14 + local_150 + 1] != 0x3dc);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4d3d17) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4d3d17) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x4d3d17);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar9 = *piVar5 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar9 + 2) = 0x3dc;
          *(undefined2 *)(iVar9 + 0x28) = 1;
        }
      }
    }
    else {
      *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
    }
    *(undefined4 *)(iVar6 + 0x3dc) = 0x42c80000;
  }
  uVar11 = (this->m_hRocketTrail).super_CBaseHandle.m_Index;
  if ((uVar11 == 0xffffffff) ||
     (iVar6 = (uVar11 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d3d1b),
     *(uint *)(iVar6 + 8) != uVar11 >> 0x10)) {
    iVar6 = 0;
  }
  else {
    iVar6 = *(int *)(iVar6 + 4);
  }
  if (*(float *)(unaff_EBX + 0x35af27) != *(float *)(iVar6 + 0x3e0)) {
    if (*(char *)(iVar6 + 0x60) == '\0') {
      pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
      if ((pCVar3 != (CBaseEdict *)0x0) && ((pCVar3->m_fStateFlags & 0x100U) == 0)) {
        pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar3);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4d3d17);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar4) {
          uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar4[uVar11 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_007740cf:
            puVar4[(uint)uVar2 + uVar11 * 0x14 + 1] = 0x3e0;
            puVar4[uVar11 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar4[uVar11 * 0x14 + 1] != 0x3e0) {
            local_14c = 0;
            do {
              uVar12 = local_14c + 1;
              local_14c = uVar12 & 0xffff;
              if ((ushort)uVar12 == uVar2) {
                if (uVar2 != 0x13) goto LAB_007740cf;
                pIVar10->m_iChangeInfoSerialNumber = 0;
                pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar11 * 0x14 + local_14c + 1] != 0x3e0);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4d3d17) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4d3d17) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x4d3d17);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar9 = *piVar5 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar9 + 2) = 0x3e0;
          *(undefined2 *)(iVar9 + 0x28) = 1;
        }
      }
    }
    else {
      *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
    }
    *(undefined4 *)(iVar6 + 0x3e0) = 0x3f4ccccd;
    uVar11 = (this->m_hRocketTrail).super_CBaseHandle.m_Index;
  }
  if ((uVar11 == 0xffffffff) ||
     (iVar6 = (uVar11 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d3d1b),
     *(uint *)(iVar6 + 8) != uVar11 >> 0x10)) {
    iVar6 = 0;
  }
  else {
    iVar6 = *(int *)(iVar6 + 4);
  }
  fVar1 = *(float *)(&DAT_00359e6b + unaff_EBX);
  if (((fVar1 != *(float *)(iVar6 + 0x3c0)) || (fVar1 != *(float *)(iVar6 + 0x3c4))) ||
     (fVar1 != *(float *)(iVar6 + 0x3c8))) {
    if (*(char *)(iVar6 + 0x60) == '\0') {
      pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
      if ((pCVar3 != (CBaseEdict *)0x0) && ((pCVar3->m_fStateFlags & 0x100U) == 0)) {
        pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar3);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4d3d17);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar4) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar4[uVar11 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_00773ce2:
            puVar4[(uint)uVar2 + uVar11 * 0x14 + 1] = 0x3c0;
            puVar4[uVar11 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar4[uVar11 * 0x14 + 1] != 0x3c0) {
            local_148 = 0;
            do {
              uVar12 = local_148 + 1;
              local_148 = uVar12 & 0xffff;
              if ((ushort)uVar12 == uVar2) {
                if (uVar2 == 0x13) goto LAB_00773d16;
                goto LAB_00773ce2;
              }
            } while (puVar4[uVar11 * 0x14 + local_148 + 1] != 0x3c0);
          }
        }
        else if (puVar4[0x7d1] == 100) {
LAB_00773d16:
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4d3d17) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4d3d17) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x4d3d17);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar9 = *piVar5 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar9 + 2) = 0x3c0;
          *(undefined2 *)(iVar9 + 0x28) = 1;
        }
      }
    }
    else {
      *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
    }
    *(float *)(iVar6 + 0x3c0) = fVar1;
    *(float *)(iVar6 + 0x3c4) = fVar1;
    *(float *)(iVar6 + 0x3c8) = fVar1;
    uVar11 = (this->m_hRocketTrail).super_CBaseHandle.m_Index;
  }
  if ((uVar11 == 0xffffffff) ||
     (iVar6 = (uVar11 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d3d1b),
     *(uint *)(iVar6 + 8) != uVar11 >> 0x10)) {
    iVar6 = 0;
  }
  else {
    iVar6 = *(int *)(iVar6 + 4);
  }
  if ((((*(float *)(iVar6 + 0x3cc) != 0.0) || (NAN(*(float *)(iVar6 + 0x3cc)))) ||
      (*(float *)(iVar6 + 0x3d0) != 0.0)) ||
     ((NAN(*(float *)(iVar6 + 0x3d0)) || (*(float *)(iVar6 + 0x3d4) != 0.0)))) {
    if (*(char *)(iVar6 + 0x60) == '\0') {
      pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
      if ((pCVar3 != (CBaseEdict *)0x0) && ((pCVar3->m_fStateFlags & 0x100U) == 0)) {
        pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar3);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4d3d17);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar4) {
          uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar4[uVar11 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_00773c10:
            puVar4[(uint)uVar2 + uVar11 * 0x14 + 1] = 0x3cc;
            puVar4[uVar11 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar4[uVar11 * 0x14 + 1] != 0x3cc) {
            local_144 = 0;
            do {
              uVar12 = local_144 + 1;
              local_144 = uVar12 & 0xffff;
              if ((ushort)uVar12 == uVar2) {
                if (uVar2 != 0x13) goto LAB_00773c10;
                pIVar10->m_iChangeInfoSerialNumber = 0;
                pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar11 * 0x14 + local_144 + 1] != 0x3cc);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4d3d17) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4d3d17) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x4d3d17);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar9 = *piVar5 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar9 + 2) = 0x3cc;
          *(undefined2 *)(iVar9 + 0x28) = 1;
        }
      }
    }
    else {
      *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
    }
    *(undefined4 *)(iVar6 + 0x3cc) = 0;
    *(undefined4 *)(iVar6 + 0x3d0) = 0;
    *(undefined4 *)(iVar6 + 0x3d4) = 0;
    uVar11 = (this->m_hRocketTrail).super_CBaseHandle.m_Index;
  }
  if ((uVar11 == 0xffffffff) ||
     (iVar6 = (uVar11 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d3d1b),
     *(uint *)(iVar6 + 8) != uVar11 >> 0x10)) {
    iVar6 = 0;
  }
  else {
    iVar6 = *(int *)(iVar6 + 4);
  }
  if (*(float *)(unaff_EBX + 0x35a14b) != *(float *)(iVar6 + 0x3f0)) {
    if (*(char *)(iVar6 + 0x60) == '\0') {
      pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
      if ((pCVar3 != (CBaseEdict *)0x0) && ((pCVar3->m_fStateFlags & 0x100U) == 0)) {
        pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar3);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4d3d17);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar4) {
          uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar4[uVar11 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_00774321:
            puVar4[(uint)uVar2 + uVar11 * 0x14 + 1] = 0x3f0;
            puVar4[uVar11 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar4[uVar11 * 0x14 + 1] != 0x3f0) {
            local_140 = 0;
            do {
              uVar12 = local_140 + 1;
              local_140 = uVar12 & 0xffff;
              if ((ushort)uVar12 == uVar2) {
                if (uVar2 != 0x13) goto LAB_00774321;
                pIVar10->m_iChangeInfoSerialNumber = 0;
                pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar11 * 0x14 + local_140 + 1] != 0x3f0);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4d3d17) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4d3d17) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x4d3d17);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar9 = *piVar5 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar9 + 2) = 0x3f0;
          *(undefined2 *)(iVar9 + 0x28) = 1;
        }
      }
    }
    else {
      *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
    }
    *(undefined4 *)(iVar6 + 0x3f0) = 0x41000000;
    uVar11 = (this->m_hRocketTrail).super_CBaseHandle.m_Index;
  }
  if ((uVar11 == 0xffffffff) ||
     (iVar6 = (uVar11 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d3d1b),
     *(uint *)(iVar6 + 8) != uVar11 >> 0x10)) {
    iVar6 = 0;
  }
  else {
    iVar6 = *(int *)(iVar6 + 4);
  }
  if (*(float *)(unaff_EBX + 0x359dcf) != *(float *)(iVar6 + 0x3f4)) {
    if (*(char *)(iVar6 + 0x60) == '\0') {
      pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
      if ((pCVar3 != (CBaseEdict *)0x0) && ((pCVar3->m_fStateFlags & 0x100U) == 0)) {
        pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar3);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4d3d17);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar4) {
          uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar4[uVar11 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_00773e86:
            puVar4[(uint)uVar2 + uVar11 * 0x14 + 1] = 0x3f4;
            puVar4[uVar11 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar4[uVar11 * 0x14 + 1] != 0x3f4) {
            local_13c = 0;
            do {
              uVar12 = local_13c + 1;
              local_13c = uVar12 & 0xffff;
              if ((ushort)uVar12 == uVar2) {
                if (uVar2 != 0x13) goto LAB_00773e86;
                pIVar10->m_iChangeInfoSerialNumber = 0;
                pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar11 * 0x14 + local_13c + 1] != 0x3f4);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4d3d17) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4d3d17) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x4d3d17);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar9 = *piVar5 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar9 + 2) = 0x3f4;
          *(undefined2 *)(iVar9 + 0x28) = 1;
        }
      }
    }
    else {
      *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
    }
    *(undefined4 *)(iVar6 + 0x3f4) = 0x42000000;
    uVar11 = (this->m_hRocketTrail).super_CBaseHandle.m_Index;
  }
  if ((uVar11 == 0xffffffff) ||
     (iVar6 = (uVar11 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d3d1b),
     *(uint *)(iVar6 + 8) != uVar11 >> 0x10)) {
    iVar6 = 0;
  }
  else {
    iVar6 = *(int *)(iVar6 + 4);
  }
  if (*(float *)(unaff_EBX + 0x359ce7) != *(float *)(iVar6 + 0x3f8)) {
    if (*(char *)(iVar6 + 0x60) == '\0') {
      pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
      if ((pCVar3 != (CBaseEdict *)0x0) && ((pCVar3->m_fStateFlags & 0x100U) == 0)) {
        pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar3);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4d3d17);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar4) {
          uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar4[uVar11 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_00774266:
            puVar4[(uint)uVar2 + uVar11 * 0x14 + 1] = 0x3f8;
            puVar4[uVar11 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar4[uVar11 * 0x14 + 1] != 0x3f8) {
            local_138 = 0;
            do {
              uVar12 = local_138 + 1;
              local_138 = uVar12 & 0xffff;
              if ((ushort)uVar12 == uVar2) {
                if (uVar2 != 0x13) goto LAB_00774266;
                pIVar10->m_iChangeInfoSerialNumber = 0;
                pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar11 * 0x14 + local_138 + 1] != 0x3f8);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4d3d17) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4d3d17) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x4d3d17);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar9 = *piVar5 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar9 + 2) = 0x3f8;
          *(undefined2 *)(iVar9 + 0x28) = 1;
        }
      }
    }
    else {
      *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
    }
    *(undefined4 *)(iVar6 + 0x3f8) = 0x40800000;
    uVar11 = (this->m_hRocketTrail).super_CBaseHandle.m_Index;
  }
  if ((uVar11 == 0xffffffff) ||
     (iVar6 = (uVar11 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d3d1b),
     *(uint *)(iVar6 + 8) != uVar11 >> 0x10)) {
    iVar6 = 0;
  }
  else {
    iVar6 = *(int *)(iVar6 + 4);
  }
  if (*(float *)(unaff_EBX + 0x35a07f) != *(float *)(iVar6 + 1000)) {
    if (*(char *)(iVar6 + 0x60) == '\0') {
      pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
      if ((pCVar3 != (CBaseEdict *)0x0) && ((pCVar3->m_fStateFlags & 0x100U) == 0)) {
        pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar3);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4d3d17);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar4) {
          uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar4[uVar11 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_00773f3b:
            puVar4[(uint)uVar2 + uVar11 * 0x14 + 1] = 1000;
            puVar4[uVar11 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar4[uVar11 * 0x14 + 1] != 1000) {
            local_134 = 0;
            do {
              uVar12 = local_134 + 1;
              local_134 = uVar12 & 0xffff;
              if ((ushort)uVar12 == uVar2) {
                if (uVar2 != 0x13) goto LAB_00773f3b;
                pIVar10->m_iChangeInfoSerialNumber = 0;
                pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar11 * 0x14 + local_134 + 1] != 1000);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4d3d17) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4d3d17) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x4d3d17);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar9 = *piVar5 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar9 + 2) = 1000;
          *(undefined2 *)(iVar9 + 0x28) = 1;
        }
      }
    }
    else {
      *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
    }
    *(undefined4 *)(iVar6 + 1000) = 0x40000000;
    uVar11 = (this->m_hRocketTrail).super_CBaseHandle.m_Index;
  }
  if ((uVar11 == 0xffffffff) ||
     (iVar6 = (uVar11 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d3d1b),
     *(uint *)(iVar6 + 8) != uVar11 >> 0x10)) {
    iVar6 = 0;
  }
  else {
    iVar6 = *(int *)(iVar6 + 4);
  }
  if (*(float *)(ainet_generate_report + unaff_EBX + 3) != *(float *)(iVar6 + 0x3ec)) {
    if (*(char *)(iVar6 + 0x60) == '\0') {
      pCVar3 = *(CBaseEdict **)(iVar6 + 0x24);
      if ((pCVar3 != (CBaseEdict *)0x0) && ((pCVar3->m_fStateFlags & 0x100U) == 0)) {
        pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar3);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x4d3d17);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar4) {
          uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar2 = puVar4[uVar11 * 0x14 + 0x14];
          if (uVar2 == 0) {
LAB_00773dd1:
            puVar4[(uint)uVar2 + uVar11 * 0x14 + 1] = 0x3ec;
            puVar4[uVar11 * 0x14 + 0x14] = uVar2 + 1;
          }
          else if (puVar4[uVar11 * 0x14 + 1] != 0x3ec) {
            local_130 = 0;
            do {
              uVar12 = local_130 + 1;
              local_130 = uVar12 & 0xffff;
              if ((ushort)uVar12 == uVar2) {
                if (uVar2 != 0x13) goto LAB_00773dd1;
                pIVar10->m_iChangeInfoSerialNumber = 0;
                pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar11 * 0x14 + local_130 + 1] != 0x3ec);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar3->m_fStateFlags = pCVar3->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x4d3d17) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x4d3d17) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x4d3d17);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar9 = *piVar5 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar9 + 2) = 0x3ec;
          *(undefined2 *)(iVar9 + 0x28) = 1;
        }
      }
    }
    else {
      *(uint *)(iVar6 + 100) = *(uint *)(iVar6 + 100) | 1;
    }
    *(undefined4 *)(iVar6 + 0x3ec) = 0x41800000;
    uVar11 = (this->m_hRocketTrail).super_CBaseHandle.m_Index;
  }
  if ((uVar11 == 0xffffffff) ||
     (iVar6 = (uVar11 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d3d1b),
     *(uint *)(iVar6 + 8) != uVar11 >> 0x10)) {
    this_00 = (CBaseParticleEntity *)0x0;
  }
  else {
    this_00 = *(CBaseParticleEntity **)(iVar6 + 4);
  }
  CBaseParticleEntity::SetLifetime(this_00,999.0);
  uVar11 = (this->m_hRocketTrail).super_CBaseHandle.m_Index;
  if ((uVar11 == 0xffffffff) ||
     (iVar6 = (uVar11 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d3d1b),
     *(uint *)(iVar6 + 8) != uVar11 >> 0x10)) {
    pRVar7 = (RocketTrail *)0x0;
  }
  else {
    pRVar7 = *(RocketTrail **)(iVar6 + 4);
  }
  RocketTrail::FollowEntity(pRVar7,(CBaseEntity *)this,(char *)0x0);
  return;
}


/* CRocket_Turret_Projectile::UpdateOnRemove at 007722b0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void UpdateOnRemove(CRocket_Turret_Projectile * this) */

void __thiscall CRocket_Turret_Projectile::UpdateOnRemove(CRocket_Turret_Projectile *this)

{
  uint uVar1;
  CBaseEntity *oldObj;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (**(code **)((int)(this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x1bc))(this);
  uVar1 = (this->m_hRocketTrail).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar2 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4d48ab),
      *(uint *)(iVar2 + 8) == uVar1 >> 0x10)) &&
     (oldObj = *(CBaseEntity **)(iVar2 + 4), oldObj != (CBaseEntity *)0x0)) {
    UTIL_Remove(oldObj);
  }
  CBaseCombatCharacter::UpdateOnRemove(&this->super_CBaseCombatCharacter);
  return;
}


/* fire_rocket_projectile_f at 00772590 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "MoveCollide_t": Some values do not have unique names */
/* WARNING: Enum "MoveType_t": Some values do not have unique names */

void fire_rocket_projectile_f(void)

{
  CBasePlayer *this;
  vec_t *pvVar1;
  CBaseEntity *this_00;
  int unaff_EBX;
  Vector local_4c;
  vec_t local_40;
  vec_t local_3c;
  vec_t local_38;
  QAngle local_34;
  Vector local_28;
  Vector local_1c;
  undefined4 local_10;
  
                    /* Unresolved local var: CBasePlayer * pPlayer@[???]
                       Unresolved local var: Vector ptEyes@[???]
                       Unresolved local var: Vector vForward@[???]
                       Unresolved local var: QAngle vLookAng@[???]
                       Unresolved local var: CRocket_Turret_Projectile * pRocket@[???] */
  ___i686_get_pc_thunk_bx();
  this = UTIL_GetCommandClient();
  (**(code **)((int)(this->super_CBaseCombatCharacter).super_CBaseFlex.super_CBaseAnimatingOverlay.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x220))(&local_40,this);
  local_1c.x = local_40;
  local_1c.y = local_3c;
  local_1c.z = local_38;
  CBasePlayer::EyeVectors(this,&local_28,(Vector *)0x0,(Vector *)0x0);
  pvVar1 = (vec_t *)(**(code **)((int)(this->super_CBaseCombatCharacter).super_CBaseFlex.
                                      super_CBaseAnimatingOverlay.super_CBaseAnimating.
                                      super_CBaseEntity.super_IServerEntity.super_IServerUnknown +
                                0x224))(this);
  local_34.x = *pvVar1;
  local_34.y = pvVar1[1];
  local_34.z = pvVar1[2];
  this_00 = CBaseEntity::Create((char *)(unaff_EBX + 0x339a3a),&local_1c,&local_34,
                                (CBaseEntity *)this);
  if (this_00 != (CBaseEntity *)0x0) {
    CBaseEntity::ThinkSet(this_00,(BASEPTR)0x0,0.0,(char *)0x0);
    CBaseEntity::SetMoveType(this_00,MOVETYPE_FLY,MOVECOLLIDE_DEFAULT);
    (*(this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
      [0x1b])(this_00,unaff_EBX + 0x343adc);
    UTIL_SetSize(this_00,*(Vector **)(&DAT_004d45db + unaff_EBX),
                 *(Vector **)(&DAT_004d45db + unaff_EBX));
    (*(this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
      [0x158])(this_00);
                    /* Unresolved local var: Vector res@[???] */
    local_4c.z = *(float *)(unaff_EBX + 0x363b23);
    local_4c.x = local_28.x * local_4c.z;
    local_4c.y = local_28.y * local_4c.z;
    local_4c.z = local_4c.z * local_28.z;
    CBaseEntity::SetAbsVelocity(this_00,&local_4c);
    local_10 = 0xffffffff;
    (*(this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
      [0x157])(this_00,&local_10);
  }
  return;
}


/* __tcf_1 at 00a40ec0 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConCommand::~ConCommand((ConCommand *)(extraout_ECX + 0x389838),in_stack_00000008);
  return;
}


/* _GLOBAL__I__ZN25CRocket_Turret_Projectile9m_DataMapE at 000ba010 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN25CRocket_Turret_Projectile9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

