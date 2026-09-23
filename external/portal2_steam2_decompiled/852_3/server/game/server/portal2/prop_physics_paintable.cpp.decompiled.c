/* DWARF-guided pseudocode for game/server/portal2/prop_physics_paintable.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* CPropPhysicsPaintable::~CPropPhysicsPaintable at 006f2e20 */

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
           super_CBaseProp = unaff_EBX + 0x54af7b;
  (this->super_PlayerPickupPaintPowerUser<CPhysicsProp>).super_PropPaintPowerUser<CPhysicsProp>.
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_CPhysicsProp.super_CBreakableProp.super_IBreakableWithPropData._vptr_IBreakableWithPropData
       = (_func_int_varargs **)(unaff_EBX + 0x54b427);
  (this->super_PlayerPickupPaintPowerUser<CPhysicsProp>).super_PropPaintPowerUser<CPhysicsProp>.
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_CPhysicsProp.super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x54b4ab);
  (this->super_PlayerPickupPaintPowerUser<CPhysicsProp>).super_PropPaintPowerUser<CPhysicsProp>.
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_CPhysicsProp.super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(unaff_EBX + 0x54b4db);
  (this->super_PlayerPickupPaintPowerUser<CPhysicsProp>).super_PropPaintPowerUser<CPhysicsProp>.
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_IPaintableEntity._vptr_IPaintableEntity = (_func_int_varargs **)(unaff_EBX + 0x54b4f7);
  (this->super_PlayerPickupPaintPowerUser<CPhysicsProp>).super_PropPaintPowerUser<CPhysicsProp>.
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_IPaintPowerUser._vptr_IPaintPowerUser
       = (_func_int_varargs **)(unaff_EBX + 0x54b513);
  PaintPowerUser<CPaintableEntity<CPhysicsProp>_>::~PaintPowerUser
            ((PaintPowerUser<CPaintableEntity<CPhysicsProp>_> *)this,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CPropPhysicsPaintable::~CPropPhysicsPaintable at 006f3020 */

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
           super_CBaseProp = extraout_ECX + 0x54ad80;
  (this->super_PlayerPickupPaintPowerUser<CPhysicsProp>).super_PropPaintPowerUser<CPhysicsProp>.
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_CPhysicsProp.super_CBreakableProp.super_IBreakableWithPropData._vptr_IBreakableWithPropData
       = (_func_int_varargs **)(extraout_ECX + 0x54b22c);
  (this->super_PlayerPickupPaintPowerUser<CPhysicsProp>).super_PropPaintPowerUser<CPhysicsProp>.
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_CPhysicsProp.super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(extraout_ECX + 0x54b2b0);
  (this->super_PlayerPickupPaintPowerUser<CPhysicsProp>).super_PropPaintPowerUser<CPhysicsProp>.
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_CPhysicsProp.super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(extraout_ECX + 0x54b2e0);
  (this->super_PlayerPickupPaintPowerUser<CPhysicsProp>).super_PropPaintPowerUser<CPhysicsProp>.
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_IPaintableEntity._vptr_IPaintableEntity = (_func_int_varargs **)(extraout_ECX + 0x54b2fc);
  (this->super_PlayerPickupPaintPowerUser<CPhysicsProp>).super_PropPaintPowerUser<CPhysicsProp>.
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_IPaintPowerUser._vptr_IPaintPowerUser
       = (_func_int_varargs **)(extraout_ECX + 0x54b318);
  PaintPowerUser<CPaintableEntity<CPhysicsProp>_>::~PaintPowerUser
            ((PaintPowerUser<CPaintableEntity<CPhysicsProp>_> *)this,__in_chrg);
  return;
}


/* CEntityFactory<CPropPhysicsPaintable>::Create at 006f3710 */

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
       (IServerEntity)(unaff_EBX + 0x549aa4);
  (this_00->super_CPaintableEntity<CPhysicsProp>).super_CPhysicsProp.super_CBreakableProp.
  super_IBreakableWithPropData._vptr_IBreakableWithPropData =
       (_func_int_varargs **)(unaff_EBX + 0x549f50);
  (this_00->super_CPaintableEntity<CPhysicsProp>).super_CPhysicsProp.super_CBreakableProp.
  super_CDefaultPlayerPickupVPhysics.super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x549fd4);
  (this_00->super_CPaintableEntity<CPhysicsProp>).super_CPhysicsProp.super_INavAvoidanceObstacle.
  _vptr_INavAvoidanceObstacle = (_func_int_varargs **)(unaff_EBX + 0x54a004);
  (this_00->super_CPaintableEntity<CPhysicsProp>).super_IPaintableEntity._vptr_IPaintableEntity =
       (_func_int_varargs **)(unaff_EBX + 0x54a020);
  (this_00->super_IPaintPowerUser)._vptr_IPaintPowerUser =
       (_func_int_varargs **)(unaff_EBX + 0x54a03c);
  (**(code **)(unaff_EBX + 0x549b18))(this_00,pClassName);
  return &(this_00->super_CPaintableEntity<CPhysicsProp>).super_CPhysicsProp.super_CBreakableProp.
          super_CBaseProp.super_CBaseAnimating.super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* PlayerPickupPaintPowerUser<CPhysicsProp>::~PlayerPickupPaintPowerUser at 006f2ed0 */

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
           super_CBaseProp.super_CBaseAnimating = unaff_EBX + 0x54aecb;
  (this->super_PropPaintPowerUser<CPhysicsProp>).
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_CPhysicsProp.super_CBreakableProp.super_IBreakableWithPropData._vptr_IBreakableWithPropData
       = (_func_int_varargs **)(unaff_EBX + 0x54b377);
  (this->super_PropPaintPowerUser<CPhysicsProp>).
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_CPhysicsProp.super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x54b3fb);
  (this->super_PropPaintPowerUser<CPhysicsProp>).
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_CPhysicsProp.super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(unaff_EBX + 0x54b42b);
  (this->super_PropPaintPowerUser<CPhysicsProp>).
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_IPaintableEntity._vptr_IPaintableEntity = (_func_int_varargs **)(unaff_EBX + 0x54b447);
  (this->super_PropPaintPowerUser<CPhysicsProp>).
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_IPaintPowerUser._vptr_IPaintPowerUser
       = (_func_int_varargs **)(unaff_EBX + 0x54b463);
  PaintPowerUser<CPaintableEntity<CPhysicsProp>_>::~PaintPowerUser
            ((PaintPowerUser<CPaintableEntity<CPhysicsProp>_> *)this,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* PlayerPickupPaintPowerUser<CPhysicsProp>::~PlayerPickupPaintPowerUser at 006f2f80 */

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
           super_CBaseProp.super_CBaseAnimating = extraout_ECX + 0x54ae20;
  (this->super_PropPaintPowerUser<CPhysicsProp>).
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_CPhysicsProp.super_CBreakableProp.super_IBreakableWithPropData._vptr_IBreakableWithPropData
       = (_func_int_varargs **)(extraout_ECX + 0x54b2cc);
  (this->super_PropPaintPowerUser<CPhysicsProp>).
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_CPhysicsProp.super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(extraout_ECX + 0x54b350);
  (this->super_PropPaintPowerUser<CPhysicsProp>).
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_CPhysicsProp.super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(extraout_ECX + 0x54b380);
  (this->super_PropPaintPowerUser<CPhysicsProp>).
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_CPaintableEntity<CPhysicsProp>.
  super_IPaintableEntity._vptr_IPaintableEntity = (_func_int_varargs **)(extraout_ECX + 0x54b39c);
  (this->super_PropPaintPowerUser<CPhysicsProp>).
  super_PaintPowerUser<CPaintableEntity<CPhysicsProp>_>.super_IPaintPowerUser._vptr_IPaintPowerUser
       = (_func_int_varargs **)(extraout_ECX + 0x54b3b8);
  PaintPowerUser<CPaintableEntity<CPhysicsProp>_>::~PaintPowerUser
            ((PaintPowerUser<CPaintableEntity<CPhysicsProp>_> *)this,__in_chrg);
  return;
}


/* __static_initialization_and_destruction_0 at 000cd5e0 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *pcVar1;
  IEntityFactoryDictionary *pIVar2;
  int unaff_EBX;
  longlong lVar3;
  
  lVar3 = ___i686_get_pc_thunk_bx();
  if (lVar3 != 0xffff00000001) {
    return;
  }
  speechListSounds[0].sceneName[unaff_EBX + -0x44] = '\0';
  speechListSounds[0].sceneName[unaff_EBX + -0x43] = '\0';
  speechListSounds[0].sceneName[unaff_EBX + -0x42] = '\0';
  speechListSounds[0].sceneName[unaff_EBX + -0x41] = '\0';
  pcVar1 = speechListSounds[0].sceneName + unaff_EBX + -0x40;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = speechListSounds[0].sceneName + unaff_EBX + -0x3c;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = speechListSounds[0].sceneName + unaff_EBX + -0x38;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = speechListSounds[0].sceneName + unaff_EBX + -0x34;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = speechListSounds[0].sceneName + unaff_EBX + -0x30;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = speechListSounds[0].sceneName + unaff_EBX + -0x2c;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = speechListSounds[0].sceneName + unaff_EBX + -0x28;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = speechListSounds[0].sceneName + unaff_EBX + -0x24;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  pcVar1 = speechListSounds[0].sceneName + unaff_EBX + -0x20;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = speechListSounds[0].sceneName + unaff_EBX + -0x1c;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = speechListSounds[0].sceneName + unaff_EBX + -0x18;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = speechListSounds[0].sceneName + unaff_EBX + -0x14;
  pcVar1[0] = -1;
  pcVar1[1] = -1;
  pcVar1[2] = '\x7f';
  pcVar1[3] = '\x7f';
  pcVar1 = speechListSounds[0].sceneName + unaff_EBX + -0x10;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  speechListSounds[0].sceneName[unaff_EBX + -9] = '\0';
  speechListSounds[0].sceneName[unaff_EBX + -0xc] = '\0';
  speechListSounds[0].sceneName[unaff_EBX + -0xb] = '\0';
  speechListSounds[0].sceneName[unaff_EBX + -10] = '\0';
  pcVar1 = speechListSounds[0].sceneName + unaff_EBX + -8;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  speechListSounds[0].sceneName[unaff_EBX + -1] = '\0';
  speechListSounds[0].sceneName[unaff_EBX + -4] = '\0';
  speechListSounds[0].sceneName[unaff_EBX + -3] = '\0';
  speechListSounds[0].sceneName[unaff_EBX + -2] = '\0';
  pcVar1 = speechListSounds[0].sceneName + unaff_EBX;
  pcVar1[0] = '\0';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  speechListSounds[0].sceneName[unaff_EBX + 7] = '\x01';
  speechListSounds[0].sceneName[unaff_EBX + 4] = '\0';
  speechListSounds[0].sceneName[unaff_EBX + 5] = '\0';
  speechListSounds[0].sceneName[unaff_EBX + 6] = '\0';
  pcVar1 = speechListSounds[0].sceneName + unaff_EBX + 8;
  pcVar1[0] = '\x01';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  speechListSounds[0].sceneName[unaff_EBX + 0xf] = '\0';
  speechListSounds[0].sceneName[unaff_EBX + 0xc] = '\0';
  speechListSounds[0].sceneName[unaff_EBX + 0xd] = '\0';
  speechListSounds[0].sceneName[unaff_EBX + 0xe] = '\0';
  pcVar1 = speechListSounds[0].sceneName + unaff_EBX + 0x10;
  pcVar1[0] = '\x02';
  pcVar1[1] = '\0';
  pcVar1[2] = '\0';
  pcVar1[3] = '\0';
  speechListSounds[0].sceneName[unaff_EBX + 0x17] = '\0';
  speechListSounds[0].sceneName[unaff_EBX + 0x14] = '\0';
  speechListSounds[0].sceneName[unaff_EBX + 0x15] = '\0';
  speechListSounds[0].sceneName[unaff_EBX + 0x16] = '\0';
  *(undefined **)(speechListSounds[0].sceneName + unaff_EBX + 0x18) = &UNK_00ae3d3c + unaff_EBX;
  *(undefined **)(speechListSounds[0].sceneName + unaff_EBX + 0x1c) = &UNK_00b6fba4 + unaff_EBX;
  pIVar2 = EntityFactoryDictionary();
  (**pIVar2->_vptr_IEntityFactoryDictionary)(pIVar2,unaff_EBX + 0xcedee0,unaff_EBX + 0x945dc0);
  return;
}


/* __static_initialization_and_destruction_1 at 000cd750 */

void __static_initialization_and_destruction_1(int __initialize_p,int __priority)

{
  int unaff_EBX;
  longlong lVar1;
  
  lVar1 = ___i686_get_pc_thunk_bx();
  if ((lVar1 == 0xffff00000001) && (**(char **)((int)&PTR_KeyValue_00c02f88 + unaff_EBX) == '\0')) {
    **(char **)((int)&PTR_KeyValue_00c02f88 + unaff_EBX) = '\x01';
    PropPaintPowerUser<CPhysicsProp>::DataMapInit
              ((datamap_t *)((int)&PTR_PerformCustomPhysics_00c1ffc4 + unaff_EBX));
  }
  return;
}


/* _GLOBAL__I_prop_physics_paintable.cpp at 000cd7a0 */

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

