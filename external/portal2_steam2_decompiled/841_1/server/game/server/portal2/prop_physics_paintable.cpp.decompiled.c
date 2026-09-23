/* DWARF-guided pseudocode for game/server/portal2/prop_physics_paintable.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* CPropPhysicsPaintable::~CPropPhysicsPaintable at 0076a730 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void ~CPropPhysicsPaintable(CPropPhysicsPaintable * this, int
   __in_chrg) */

void __thiscall
CPropPhysicsPaintable::~CPropPhysicsPaintable(CPropPhysicsPaintable *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  *(int *)&(this->super_PlayerPickupPaintPowerUser<CPhysicsProp>).
           super_PropPaintPowerUser<CPhysicsProp>.
           super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.
           super_CPaintableEntity<CPhysicsProp>.super_CPhysicsProp.super_CBreakableProp.
           super_CBaseProp = unaff_EBX + 0x56d10b;
  (this->super_PlayerPickupPaintPowerUser<CPhysicsProp>).super_PropPaintPowerUser<CPhysicsProp>.
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_CPhysicsProp.super_CBreakableProp.super_IBreakableWithPropData._vptr_IBreakableWithPropData
       = (_func_int_varargs **)(unaff_EBX + 0x56d5b7);
  (this->super_PlayerPickupPaintPowerUser<CPhysicsProp>).super_PropPaintPowerUser<CPhysicsProp>.
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_CPhysicsProp.super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x56d63b);
  (this->super_PlayerPickupPaintPowerUser<CPhysicsProp>).super_PropPaintPowerUser<CPhysicsProp>.
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_CPhysicsProp.super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(unaff_EBX + 0x56d66b);
  (this->super_PlayerPickupPaintPowerUser<CPhysicsProp>).super_PropPaintPowerUser<CPhysicsProp>.
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_IPaintableEntity._vptr_IPaintableEntity = (_func_int_varargs **)(unaff_EBX + 0x56d687);
  (this->super_PlayerPickupPaintPowerUser<CPhysicsProp>).super_PropPaintPowerUser<CPhysicsProp>.
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_IPaintPowerUser._vptr_IPaintPowerUser
       = (_func_int_varargs **)(unaff_EBX + 0x56d6a3);
  PaintPowerUser<CPaintableEntity<CPhysicsProp>_>::~PaintPowerUser
            ((PaintPowerUser<CPaintableEntity<CPhysicsProp>_> *)this,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CPropPhysicsPaintable::~CPropPhysicsPaintable at 0076a880 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void ~CPropPhysicsPaintable(CPropPhysicsPaintable * this, int
   __in_chrg) */

void __thiscall
CPropPhysicsPaintable::~CPropPhysicsPaintable(CPropPhysicsPaintable *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *(int *)&(this->super_PlayerPickupPaintPowerUser<CPhysicsProp>).
           super_PropPaintPowerUser<CPhysicsProp>.
           super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.
           super_CPaintableEntity<CPhysicsProp>.super_CPhysicsProp.super_CBreakableProp.
           super_CBaseProp = extraout_ECX + 0x56cfc0;
  (this->super_PlayerPickupPaintPowerUser<CPhysicsProp>).super_PropPaintPowerUser<CPhysicsProp>.
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_CPhysicsProp.super_CBreakableProp.super_IBreakableWithPropData._vptr_IBreakableWithPropData
       = (_func_int_varargs **)(extraout_ECX + 0x56d46c);
  (this->super_PlayerPickupPaintPowerUser<CPhysicsProp>).super_PropPaintPowerUser<CPhysicsProp>.
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_CPhysicsProp.super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(extraout_ECX + 0x56d4f0);
  (this->super_PlayerPickupPaintPowerUser<CPhysicsProp>).super_PropPaintPowerUser<CPhysicsProp>.
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_CPhysicsProp.super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(extraout_ECX + 0x56d520);
  (this->super_PlayerPickupPaintPowerUser<CPhysicsProp>).super_PropPaintPowerUser<CPhysicsProp>.
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_IPaintableEntity._vptr_IPaintableEntity = (_func_int_varargs **)(extraout_ECX + 0x56d53c);
  (this->super_PlayerPickupPaintPowerUser<CPhysicsProp>).super_PropPaintPowerUser<CPhysicsProp>.
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_IPaintPowerUser._vptr_IPaintPowerUser
       = (_func_int_varargs **)(extraout_ECX + 0x56d558);
  PaintPowerUser<CPaintableEntity<CPhysicsProp>_>::~PaintPowerUser
            ((PaintPowerUser<CPaintableEntity<CPhysicsProp>_> *)this,__in_chrg);
  return;
}


/* CEntityFactory<CPropPhysicsPaintable>::Create at 0076b020 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CPropPhysicsPaintable> *
   this, char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CPropPhysicsPaintable>::Create
          (CEntityFactory<CPropPhysicsPaintable> *this,char *pClassName)

{
  PaintPowerUser<CPaintableEntity<CPhysicsProp>_> *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CPropPhysicsPaintable * pEnt@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CBaseEntity::operator_new(0x884);
  PaintPowerUser<CPaintableEntity<CPhysicsProp>_>::PaintPowerUser(this_00);
  this_00[1].super_CPaintableEntity<CPhysicsProp>.super_CPhysicsProp.super_CBreakableProp.
  super_CBaseProp.super_CBaseAnimating.super_CBaseEntity.m_iObjectCapsCache.m_Value = 4;
  *(undefined1 *)
   &this_00[1].super_CPaintableEntity<CPhysicsProp>.super_CPhysicsProp.super_CBreakableProp.
    super_CBaseProp.super_CBaseAnimating.super_CBaseEntity.m_pfnMoveDone.__pfn = 0;
  *(int *)((int)&this_00[1].super_CPaintableEntity<CPhysicsProp>.super_CPhysicsProp.
                 super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.super_CBaseEntity.
                 m_pfnMoveDone + 4) = 0;
  (this_00->super_CPaintableEntity<CPhysicsProp>).super_CPhysicsProp.super_CBreakableProp.
  super_CBaseProp.super_CBaseAnimating.super_CBaseEntity.super_IServerEntity =
       (IServerEntity)(unaff_EBX + 0x56bc34);
  (this_00->super_CPaintableEntity<CPhysicsProp>).super_CPhysicsProp.super_CBreakableProp.
  super_IBreakableWithPropData._vptr_IBreakableWithPropData =
       (_func_int_varargs **)(unaff_EBX + 0x56c0e0);
  (this_00->super_CPaintableEntity<CPhysicsProp>).super_CPhysicsProp.super_CBreakableProp.
  super_CDefaultPlayerPickupVPhysics.super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x56c164);
  (this_00->super_CPaintableEntity<CPhysicsProp>).super_CPhysicsProp.super_INavAvoidanceObstacle.
  _vptr_INavAvoidanceObstacle = (_func_int_varargs **)(unaff_EBX + 0x56c194);
  (this_00->super_CPaintableEntity<CPhysicsProp>).super_IPaintableEntity._vptr_IPaintableEntity =
       (_func_int_varargs **)(unaff_EBX + 0x56c1b0);
  (this_00->super_IPaintPowerUser)._vptr_IPaintPowerUser =
       (_func_int_varargs **)(unaff_EBX + 0x56c1cc);
  (**(code **)(unaff_EBX + 0x56bca8))(this_00,pClassName);
  return &(this_00->super_CPaintableEntity<CPhysicsProp>).super_CPhysicsProp.super_CBreakableProp.
          super_CBaseProp.super_CBaseAnimating.super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* PlayerPickupPaintPowerUser<CPhysicsProp>::~PlayerPickupPaintPowerUser at 0076a7e0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void
   ~PlayerPickupPaintPowerUser(PlayerPickupPaintPowerUser<CPhysicsProp> * this, int __in_chrg) */

void __thiscall
PlayerPickupPaintPowerUser<CPhysicsProp>::~PlayerPickupPaintPowerUser
          (PlayerPickupPaintPowerUser<CPhysicsProp> *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *(int *)&(this->super_PropPaintPowerUser<CPhysicsProp>).
           super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.
           super_CPaintableEntity<CPhysicsProp>.super_CPhysicsProp.super_CBreakableProp.
           super_CBaseProp.super_CBaseAnimating = extraout_ECX + 0x56d060;
  (this->super_PropPaintPowerUser<CPhysicsProp>).
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_CPhysicsProp.super_CBreakableProp.super_IBreakableWithPropData._vptr_IBreakableWithPropData
       = (_func_int_varargs **)(extraout_ECX + 0x56d50c);
  (this->super_PropPaintPowerUser<CPhysicsProp>).
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_CPhysicsProp.super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(extraout_ECX + 0x56d590);
  (this->super_PropPaintPowerUser<CPhysicsProp>).
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_CPhysicsProp.super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(extraout_ECX + 0x56d5c0);
  (this->super_PropPaintPowerUser<CPhysicsProp>).
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_IPaintableEntity._vptr_IPaintableEntity = (_func_int_varargs **)(extraout_ECX + 0x56d5dc);
  (this->super_PropPaintPowerUser<CPhysicsProp>).
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_IPaintPowerUser._vptr_IPaintPowerUser
       = (_func_int_varargs **)(extraout_ECX + 0x56d5f8);
  PaintPowerUser<CPaintableEntity<CPhysicsProp>_>::~PaintPowerUser
            ((PaintPowerUser<CPaintableEntity<CPhysicsProp>_> *)this,__in_chrg);
  return;
}


/* PlayerPickupPaintPowerUser<CPhysicsProp>::~PlayerPickupPaintPowerUser at 0076a920 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void
   ~PlayerPickupPaintPowerUser(PlayerPickupPaintPowerUser<CPhysicsProp> * this, int __in_chrg) */

void __thiscall
PlayerPickupPaintPowerUser<CPhysicsProp>::~PlayerPickupPaintPowerUser
          (PlayerPickupPaintPowerUser<CPhysicsProp> *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  *(int *)&(this->super_PropPaintPowerUser<CPhysicsProp>).
           super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.
           super_CPaintableEntity<CPhysicsProp>.super_CPhysicsProp.super_CBreakableProp.
           super_CBaseProp.super_CBaseAnimating = unaff_EBX + 0x56cf1b;
  (this->super_PropPaintPowerUser<CPhysicsProp>).
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_CPhysicsProp.super_CBreakableProp.super_IBreakableWithPropData._vptr_IBreakableWithPropData
       = (_func_int_varargs **)(unaff_EBX + 0x56d3c7);
  (this->super_PropPaintPowerUser<CPhysicsProp>).
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_CPhysicsProp.super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x56d44b);
  (this->super_PropPaintPowerUser<CPhysicsProp>).
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_CPhysicsProp.super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(unaff_EBX + 0x56d47b);
  (this->super_PropPaintPowerUser<CPhysicsProp>).
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_IPaintableEntity._vptr_IPaintableEntity = (_func_int_varargs **)(unaff_EBX + 0x56d497);
  (this->super_PropPaintPowerUser<CPhysicsProp>).
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_IPaintPowerUser._vptr_IPaintPowerUser
       = (_func_int_varargs **)(unaff_EBX + 0x56d4b3);
  PaintPowerUser<CPaintableEntity<CPhysicsProp>_>::~PaintPowerUser
            ((PaintPowerUser<CPaintableEntity<CPhysicsProp>_> *)this,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* __static_initialization_and_destruction_0 at 000b8490 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  IEntityFactoryDictionary *pIVar1;
  int unaff_EBX;
  longlong lVar2;
  
  lVar2 = ___i686_get_pc_thunk_bx();
  if (lVar2 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)
   ((int)g_BanMasks[9].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>.m_Ints +
   unaff_EBX) = 0;
  *(undefined1 *)
   ((int)g_BanMasks[9].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>.m_Ints +
   unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)g_BanMasks[9].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>.m_Ints +
   unaff_EBX + 2) = 0;
  *(undefined1 *)
   ((int)g_BanMasks[9].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>.m_Ints +
   unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)g_BanMasks[9].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>.m_Ints +
   unaff_EBX + 4) = 0;
  *(undefined4 *)
   ((int)g_BanMasks[10].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>.m_Ints +
   unaff_EBX) = 0;
  *(undefined4 *)
   ((int)g_BanMasks[10].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>.m_Ints +
   unaff_EBX + 4) = 0x7f7fffff;
  *(undefined4 *)
   ((int)g_BanMasks[0xb].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>.m_Ints +
   unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)
   ((int)g_BanMasks[0xb].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>.m_Ints +
   unaff_EBX + 4) = 0;
  *(undefined4 *)
   ((int)g_BanMasks[0xc].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>.m_Ints +
   unaff_EBX) = 0;
  *(undefined4 *)
   ((int)g_BanMasks[0xc].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>.m_Ints +
   unaff_EBX + 4) = 0;
  *(undefined4 *)
   ((int)g_BanMasks[0xd].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>.m_Ints +
   unaff_EBX) = 0;
  *(undefined4 *)
   ((int)g_BanMasks[0xd].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>.m_Ints +
   unaff_EBX + 4) = 0x7f7fffff;
  *(undefined4 *)
   ((int)g_BanMasks[0xe].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>.m_Ints +
   unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)
   ((int)g_BanMasks[0xe].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>.m_Ints +
   unaff_EBX + 4) = 0x7f7fffff;
  *(undefined4 *)
   ((int)g_BanMasks[0xf].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>.m_Ints +
   unaff_EBX) = 0x7f7fffff;
  *(undefined **)
   ((int)g_BanMasks[0xf].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>.m_Ints +
   unaff_EBX + 4) = &UNK_00b93e6c + unaff_EBX;
  *(undefined **)
   ((int)g_BanMasks[0x10].super_CBitVecT<CFixedBitVecBase<64>_>.super_CFixedBitVecBase<64>.m_Ints +
   unaff_EBX) = &UNK_00c1e794 + unaff_EBX;
  pIVar1 = EntityFactoryDictionary();
  (**pIVar1->_vptr_IEntityFactoryDictionary)(pIVar1,unaff_EBX + 0xd982a0,&UNK_009fd208 + unaff_EBX);
  return;
}


/* __static_initialization_and_destruction_1 at 000b8570 */

void __static_initialization_and_destruction_1(int __initialize_p,int __priority)

{
  int unaff_EBX;
  longlong lVar1;
  
  lVar1 = ___i686_get_pc_thunk_bx();
  if ((lVar1 == 0xffff00000001) && (**(char **)((int)&PTR_BloodColor_00cb24c8 + unaff_EBX) == '\0'))
  {
    **(char **)((int)&PTR_BloodColor_00cb24c8 + unaff_EBX) = '\x01';
    PropPaintPowerUser<CPhysicsProp>::DataMapInit
              ((datamap_t *)((int)&PTR_Respawn_00ccf504 + unaff_EBX));
  }
  return;
}


/* _GLOBAL__I_prop_physics_paintable.cpp at 000b85c0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_prop_physics_paintable_cpp(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  int in_stack_fffffff4;
  int in_stack_fffffff8;
  
  __static_initialization_and_destruction_0(in_stack_fffffff4,in_stack_fffffff8);
  __static_initialization_and_destruction_1(in_stack_00000004,in_stack_00000008);
  return;
}

