/* DWARF-guided pseudocode for game/server/portal2/propwallprojector.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* CPropWallProjector::Spawn at 0074f330 */

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
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1b])
            (this,&UNK_00364503 + unaff_EBX);
  CCollisionProperty::SetSolid
            (&(this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.m_Collision.
              super_CCollisionProperty,SOLID_VPHYSICS);
  return;
}


/* CPropWallProjector::Precache at 0074f310 */

/* DWARF original prototype: void Precache(CPropWallProjector * this) */

void __thiscall CPropWallProjector::Precache(CPropWallProjector *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  CBaseEntity::PrecacheModel((char *)(extraout_ECX + 0x36452c));
  return;
}


/* CPropWallProjector::CreateNewProjectedEntity at 0074f300 */

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


/* __static_initialization_and_destruction_0 at 000b35e0 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  IEntityFactoryDictionary *pIVar1;
  int unaff_EBX;
  longlong lVar2;
  
  lVar2 = ___i686_get_pc_thunk_bx();
  if (lVar2 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)&ai_find_lateral_cover.m_fMaxVal + unaff_EBX) = 0;
  *(undefined1 *)((int)&ai_find_lateral_cover.m_fMaxVal + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&ai_find_lateral_cover.m_fMaxVal + unaff_EBX + 2) = 0;
  *(undefined1 *)((int)&ai_find_lateral_cover.m_fMaxVal + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&ai_find_lateral_cover.m_fnChangeCallbacks.m_Memory.m_pMemory + unaff_EBX) =
       0;
  *(undefined4 *)
   ((int)&ai_find_lateral_cover.m_fnChangeCallbacks.m_Memory.m_nAllocationCount + unaff_EBX) = 0;
  *(undefined4 *)((int)&ai_find_lateral_cover.m_fnChangeCallbacks.m_Memory.m_nGrowSize + unaff_EBX)
       = 0x7f7fffff;
  *(undefined4 *)((int)&ai_find_lateral_cover.m_fnChangeCallbacks.m_Size + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&ai_find_lateral_cover.m_fnChangeCallbacks.m_pElements + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00d9c6b8 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00d9c6bc + unaff_EBX) = 0;
  *(undefined4 *)((int)&ai_find_lateral_los.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX) =
       0;
  *(undefined4 *)((int)&ai_find_lateral_los.super_ConCommandBase.m_pNext + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&ai_find_lateral_los.super_ConCommandBase.m_bRegistered + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&ai_find_lateral_los.super_ConCommandBase.m_pszName + unaff_EBX) = 0x7f7fffff
  ;
  *(undefined4 *)((int)&ai_find_lateral_los.super_ConCommandBase.m_pszHelpString + unaff_EBX) =
       0x7f7fffff;
  *(undefined **)((int)&ai_find_lateral_los.super_ConCommandBase.m_nFlags + unaff_EBX) =
       &UNK_00b98d1c + unaff_EBX;
  *(undefined **)((int)&ai_find_lateral_los.super_IConVar._vptr_IConVar + unaff_EBX) =
       &UNK_00c1edc4 + unaff_EBX;
  pIVar1 = EntityFactoryDictionary();
  (**pIVar1->_vptr_IEntityFactoryDictionary)(pIVar1,unaff_EBX + 0xd9c6d8,unaff_EBX + 0x9a7946);
  return;
}


/* _GLOBAL__I__ZN18CPropWallProjector5SpawnEv at 000b36c0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN18CPropWallProjector5SpawnEv(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

