/* DWARF-guided pseudocode for game/server/portal2/propwallprojector.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* CPropWallProjector::Spawn at 006dd590 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CPropWallProjector * this) */

void __thiscall CPropWallProjector::Spawn(CPropWallProjector *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseProjector::Spawn(&this->super_CBaseProjector);
  (*(this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1a])(this);
  (*(this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1b])(this,unaff_EBX + 0x33410f);
  CCollisionProperty::SetSolid
            (&(this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.m_Collision.
              super_CCollisionProperty,SOLID_VPHYSICS);
  return;
}


/* CPropWallProjector::Precache at 006dd570 */

/* DWARF original prototype: void Precache(CPropWallProjector * this) */

void __thiscall CPropWallProjector::Precache(CPropWallProjector *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  CBaseEntity::PrecacheModel((char *)(extraout_ECX + 0x334138));
  return;
}


/* CPropWallProjector::CreateNewProjectedEntity at 006dd560 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: CBaseProjectedEntity * CreateNewProjectedEntity(CPropWallProjector *
   this) */

CBaseProjectedEntity * __thiscall
CPropWallProjector::CreateNewProjectedEntity(CPropWallProjector *this)

{
  CProjectedWallEntity *pCVar1;
  
  pCVar1 = CProjectedWallEntity::CreateNewInstance();
  return (CBaseProjectedEntity *)pCVar1;
}


/* __static_initialization_and_destruction_0 at 000c8960 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  IEntityFactoryDictionary *pIVar1;
  int unaff_EBX;
  longlong lVar2;
  
  lVar2 = ___i686_get_pc_thunk_bx();
  if (lVar2 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2f48) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2f49) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2f4a) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2f4b) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2f4c) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2f50) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2f54) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2f58) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2f5c) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2f60) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2f64) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2f68) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2f6c) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2f70) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2f74) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2f78) = 0x7f7fffff;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2f7c) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2f83) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2f80) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2f81) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2f82) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2f84) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2f8b) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2f88) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2f89) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2f8a) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2f8c) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2f93) = 1;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2f90) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2f91) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2f92) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2f94) = 1;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2f9b) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2f98) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2f99) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2f9a) = 0;
  *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x2f9c) = 2;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2fa3) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2fa0) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2fa1) = 0;
  *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x2fa2) = 0;
  *(undefined **)((int)s_pRandomFloats + unaff_EBX + 0x2fa4) = &UNK_00ae89bc + unaff_EBX;
  *(undefined **)((int)s_pRandomFloats + unaff_EBX + 0x2fa8) = &UNK_00b70924 + unaff_EBX;
  pIVar1 = EntityFactoryDictionary();
  (**pIVar1->_vptr_IEntityFactoryDictionary)(pIVar1,unaff_EBX + 0xcf20e8,unaff_EBX + 0x8ef362);
  return;
}


/* _GLOBAL__I__ZN18CPropWallProjector5SpawnEv at 000c8ad0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN18CPropWallProjector5SpawnEv(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

