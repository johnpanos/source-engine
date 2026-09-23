/* DWARF-guided pseudocode for game/client/portal2/c_prop_paint_bomb.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* ClientClassInit<DT_PropPaintBomb::ignored> at 00078d30 */

int ClientClassInit<DT_PropPaintBomb::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&DAT_00c59ed4)[unaff_EBX] == '\0') {
    iVar1 = ___cxa_guard_acquire(&DAT_00c59ed4 + unaff_EBX);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&CCvarNegateCheckButton::GetMessageMap::s_pMap + unaff_EBX),
                  (char *)(unaff_EBX + 0x911a2c),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)&CCvarTextEntry::GetMessageMap()::s_pMap + unaff_EBX),
                        (char *)(unaff_EBX + 0x916570),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00aeb624 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00aeb0a8 + unaff_EBX));
      RecvPropInt((RecvProp_conflict1 *)(unaff_EBX + 0xc59f54),(char *)(unaff_EBX + 0x942694),0xda0,
                  4,0,(RecvVarProxyFn)0x0);
      ___cxa_guard_release(&DAT_00c59ed4 + unaff_EBX);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_SIMDRandContexts[0x1e].m_RandY[0x1f] + unaff_EBX + 4),
             (RecvProp_conflict1 *)((int)&CCvarTextEntry::GetMessageMap()::s_pMap + unaff_EBX),2,
             (char *)(unaff_EBX + 0x9426a6));
  return 1;
}


/* __static_initialization_and_destruction_0 at 00078e80 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  RecvTable *this;
  int *piVar1;
  undefined4 uVar2;
  int iVar3;
  ConVar *unaff_EBX;
  char *unaff_EBP;
  char *unaff_ESI;
  longlong lVar4;
  int unaff_retaddr;
  ConVar *this_1;
  
  this_1 = unaff_EBX;
  lVar4 = ___i686_get_pc_thunk_bx();
  if (lVar4 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)&unaff_EBX[0x23ee0].m_fnChangeCallbacks.m_Memory.m_nGrowSize + 3) = 0;
  *(undefined1 *)&unaff_EBX[0x23ee0].m_fnChangeCallbacks.m_Size = 0;
  *(undefined1 *)((int)&unaff_EBX[0x23ee0].m_fnChangeCallbacks.m_Size + 1) = 0;
  *(undefined1 *)((int)&unaff_EBX[0x23ee0].m_fnChangeCallbacks.m_Size + 2) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x23ee0].m_fnChangeCallbacks.m_Size + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x23ee0].m_fnChangeCallbacks.m_pElements + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x23ee1].super_ConCommandBase._vptr_ConCommandBase + 3) =
       0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x23ee1].super_ConCommandBase.m_pNext + 3) = 0x7f7fffff;
  *(undefined4 *)&unaff_EBX[0x23ee1].super_ConCommandBase.field_0xb = 0;
  *(undefined4 *)((int)&unaff_EBX[0x23ee1].super_ConCommandBase.m_pszName + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x23ee1].super_ConCommandBase.m_pszHelpString + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x23ee1].super_ConCommandBase.m_nFlags + 3) = 0;
  *(undefined4 *)((int)&unaff_EBX[0x23ee1].super_IConVar._vptr_IConVar + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x23ee1].m_pParent + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x23ee1].m_pszDefaultValue + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&unaff_EBX[0x23ee1].m_Value.m_pszString + 3) = 0x7f7fffff;
  *(undefined1 **)((int)&unaff_EBX[0x23ee1].m_Value.m_StringLength + 3) =
       (undefined1 *)((int)&unaff_EBX[0x1fc5c].m_fnChangeCallbacks.m_pElements + 3);
  *(undefined1 **)&unaff_EBX[0x25a41].field_0x3f =
       (undefined1 *)((int)&unaff_EBX[0x1aef8].m_Value.m_StringLength + 2);
  *(undefined1 **)&unaff_EBX[0x25a41].field_0x37 =
       (undefined1 *)((int)&unaff_EBX[0xdd24].m_Value.m_fValue + 3);
  *(undefined4 *)((int)&unaff_EBX[0x25a41].m_fMinVal + 3) = 0;
  this = (RecvTable *)((int)&unaff_EBX[0x25a41].m_fnChangeCallbacks.m_Memory.m_nGrowSize + 3);
  *(RecvTable **)((int)&unaff_EBX[0x25a41].m_fMaxVal + 3) = this;
  piVar1 = *(int **)((int)&unaff_EBX[0x1fc2c].m_Value.m_nValue + 3);
  *(int *)((int)&unaff_EBX[0x25a41].m_fnChangeCallbacks.m_Memory.m_pMemory + 3) = *piVar1;
  *piVar1 = (int)&unaff_EBX[0x25a41].field_0x37;
  RecvTable::RecvTable(this);
  uVar2 = *(undefined4 *)((int)&unaff_EBX[0x1fc2b].super_ConCommandBase.m_pszHelpString + 3);
  ___cxa_atexit((undefined1 *)
                ((int)&unaff_EBX[0x1a380].m_fnChangeCallbacks.m_Memory.m_nGrowSize + 3),0,uVar2);
  iVar3 = ClientClassInit<DT_PropPaintBomb::ignored>((ignored *)0x0);
  *(int *)&unaff_EBX[0x25a42].super_ConCommandBase.field_0xb = iVar3;
  ConVar::ConVar((ConVar *)((int)&unaff_EBX[0x25a42].m_pParent + 3),
                 (char *)((int)&unaff_EBX[0x1aef8].m_fMinVal + 1),
                 (char *)((int)&unaff_EBX[0x1a653].super_ConCommandBase.m_pszHelpString + 3),0,
                 this_1,unaff_ESI,unaff_EBP,unaff_retaddr);
  ___cxa_atexit((undefined1 *)((int)&unaff_EBX[0x1a380].m_Value.m_fValue + 3),0,uVar2);
  return;
}


/* C_PropPaintBomb::YouForgotToImplementOrDeclareClientClass at 00538b00 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PhysicsProp": ignoring multiple overlapping fields */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_PropPaintBomb * this) */

int __thiscall C_PropPaintBomb::YouForgotToImplementOrDeclareClientClass(C_PropPaintBomb *this)

{
  return 0;
}


/* C_PropPaintBomb::GetClientClass at 00538b20 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PhysicsProp": ignoring multiple overlapping fields */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: ClientClass * GetClientClass(C_PropPaintBomb * this) */

ClientClass * __thiscall C_PropPaintBomb::GetClientClass(C_PropPaintBomb *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x8309f8);
}


/* _C_PropPaintBomb_CreateObject at 00539320 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PhysicsProp": ignoring multiple overlapping fields */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */

IClientNetworkable * _C_PropPaintBomb_CreateObject(int entnum,int serialNum)

{
  C_PropPaintBomb *this;
  IClientNetworkable *pIVar1;
  
                    /* Unresolved local var: C_PropPaintBomb * pRet@[???] */
  this = C_BaseEntity::operator_new(0xe30);
  C_PropPaintBomb::C_PropPaintBomb(this);
  pIVar1 = (IClientNetworkable *)0x0;
  if (this != (C_PropPaintBomb *)0x0) {
    (**(code **)((int)(this->super_C_PhysicsProp).super_C_BreakableProp.super_C_BaseAnimating.
                      super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
                      super_IHandleEntity + 0xbc))(this,entnum,serialNum);
    pIVar1 = &(this->super_C_PhysicsProp).super_C_BreakableProp.super_C_BaseAnimating.
              super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable;
  }
  return pIVar1;
}


/* __tcf_0 at 0097c2e0 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3ed250),in_stack_00000008);
  return;
}


/* __tcf_1 at 0097c2c0 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ed298),in_stack_00000008);
  return;
}


/* C_PropPaintBomb::C_PropPaintBomb at 00539170 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PhysicsProp": ignoring multiple overlapping fields */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void C_PropPaintBomb(C_PropPaintBomb * this) */

void __thiscall C_PropPaintBomb::C_PropPaintBomb(C_PropPaintBomb *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_PhysicsProp::C_PhysicsProp(&this->super_C_PhysicsProp);
  (this->super_C_PhysicsProp).super_C_BreakableProp.super_C_BaseAnimating.super_C_BaseEntity.
  super_IClientEntity.super_IClientUnknown.super_IHandleEntity =
       (IHandleEntity)(unaff_EBX + 0x694e2b);
  (this->super_C_PhysicsProp).super_C_BreakableProp.super_C_BaseAnimating.super_C_BaseEntity.
  super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x695287);
  (this->super_C_PhysicsProp).super_C_BreakableProp.super_C_BaseAnimating.super_C_BaseEntity.
  super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x69533f);
  (this->super_C_PhysicsProp).super_C_BreakableProp.super_C_BaseAnimating.super_C_BaseEntity.
  super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x69537b);
  (this->super_C_PhysicsProp).super_C_BreakableProp.super_C_BaseAnimating.super_C_BaseEntity.
  super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x695397);
  this->m_nPaintPowerType = 4;
  this->m_nPrevPaintPowerType = 4;
  (this->m_PaintBlobs).m_Memory.m_pMemory = (CBasePaintBlob **)0x0;
  (this->m_PaintBlobs).m_Memory.m_nAllocationCount = 0;
  (this->m_PaintBlobs).m_Memory.m_nGrowSize = 0;
  (this->m_PaintBlobs).m_Size = 0;
  (this->m_PaintBlobs).m_pElements = (CBasePaintBlob **)0x0;
  this->m_flAccumulatedTime = 0.0;
  this->m_flLastUpdateTime = *(float *)(**(int **)(unaff_EBX + 0x62ac1f) + 0xc);
  return;
}


/* C_PropPaintBomb::C_PropPaintBomb at 00539230 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PhysicsProp": ignoring multiple overlapping fields */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void C_PropPaintBomb(C_PropPaintBomb * this, C_PropPaintBomb * this) */

void __thiscall C_PropPaintBomb::C_PropPaintBomb(C_PropPaintBomb *this,C_PropPaintBomb *this_1)

{
  C_PropPaintBomb(this);
  return;
}


/* C_PropPaintBomb::~C_PropPaintBomb at 00539380 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PhysicsProp": ignoring multiple overlapping fields */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void ~C_PropPaintBomb(C_PropPaintBomb * this, int __in_chrg) */

void __thiscall C_PropPaintBomb::~C_PropPaintBomb(C_PropPaintBomb *this,int __in_chrg)

{
  CBasePaintBlob **ppCVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_PhysicsProp).super_C_BreakableProp.super_C_BaseAnimating.super_C_BaseEntity.
  super_IClientEntity.super_IClientUnknown.super_IHandleEntity =
       (IHandleEntity)(&UNK_00694c14 + unaff_EBX);
  (this->super_C_PhysicsProp).super_C_BreakableProp.super_C_BaseAnimating.super_C_BaseEntity.
  super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x695070);
  (this->super_C_PhysicsProp).super_C_BreakableProp.super_C_BaseAnimating.super_C_BaseEntity.
  super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(&UNK_00695128 + unaff_EBX);
  (this->super_C_PhysicsProp).super_C_BreakableProp.super_C_BaseAnimating.super_C_BaseEntity.
  super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x695164);
  (this->super_C_PhysicsProp).super_C_BreakableProp.super_C_BaseAnimating.super_C_BaseEntity.
  super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x695180);
  (this->m_PaintBlobs).m_Size = 0;
  if ((this->m_PaintBlobs).m_Memory.m_nGrowSize < 0) {
    ppCVar1 = (this->m_PaintBlobs).m_Memory.m_pMemory;
  }
  else {
    ppCVar1 = (this->m_PaintBlobs).m_Memory.m_pMemory;
    if (ppCVar1 != (CBasePaintBlob **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x62a9dc) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x62a9dc),ppCVar1);
      (this->m_PaintBlobs).m_Memory.m_pMemory = (CBasePaintBlob **)0x0;
    }
    ppCVar1 = (CBasePaintBlob **)0x0;
    (this->m_PaintBlobs).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintBlobs).m_pElements = ppCVar1;
  if (-1 < (this->m_PaintBlobs).m_Memory.m_nGrowSize) {
    if (ppCVar1 != (CBasePaintBlob **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x62a9dc) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x62a9dc),ppCVar1);
      (this->m_PaintBlobs).m_Memory.m_pMemory = (CBasePaintBlob **)0x0;
    }
    (this->m_PaintBlobs).m_Memory.m_nAllocationCount = 0;
  }
  C_PhysicsProp::~C_PhysicsProp(&this->super_C_PhysicsProp,__in_chrg);
  return;
}


/* C_PropPaintBomb::~C_PropPaintBomb at 00539b80 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PhysicsProp": ignoring multiple overlapping fields */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void ~C_PropPaintBomb(C_PropPaintBomb * this, int __in_chrg) */

void __thiscall C_PropPaintBomb::~C_PropPaintBomb(C_PropPaintBomb *this,int __in_chrg)

{
  CBasePaintBlob **ppCVar1;
  int unaff_EBX;
  CBasePaintBlob **in_stack_ffffffc8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_PhysicsProp).super_C_BreakableProp.super_C_BaseAnimating.super_C_BaseEntity.
  super_IClientEntity.super_IClientUnknown.super_IHandleEntity =
       (IHandleEntity)(unaff_EBX + 0x694414);
  (this->super_C_PhysicsProp).super_C_BreakableProp.super_C_BaseAnimating.super_C_BaseEntity.
  super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x694870);
  (this->super_C_PhysicsProp).super_C_BreakableProp.super_C_BaseAnimating.super_C_BaseEntity.
  super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x694928);
  (this->super_C_PhysicsProp).super_C_BreakableProp.super_C_BaseAnimating.super_C_BaseEntity.
  super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x694964);
  (this->super_C_PhysicsProp).super_C_BreakableProp.super_C_BaseAnimating.super_C_BaseEntity.
  super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(&UNK_00694980 + unaff_EBX);
  (this->m_PaintBlobs).m_Size = 0;
  if ((this->m_PaintBlobs).m_Memory.m_nGrowSize < 0) {
    ppCVar1 = (this->m_PaintBlobs).m_Memory.m_pMemory;
  }
  else {
    ppCVar1 = (this->m_PaintBlobs).m_Memory.m_pMemory;
    if (ppCVar1 != (CBasePaintBlob **)0x0) {
      in_stack_ffffffc8 = ppCVar1;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x62a1dc) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x62a1dc));
      ppCVar1 = (CBasePaintBlob **)0x0;
      (this->m_PaintBlobs).m_Memory.m_pMemory = (CBasePaintBlob **)0x0;
    }
    (this->m_PaintBlobs).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintBlobs).m_pElements = ppCVar1;
  if (-1 < (this->m_PaintBlobs).m_Memory.m_nGrowSize) {
    if (ppCVar1 != (CBasePaintBlob **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x62a1dc) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x62a1dc));
      (this->m_PaintBlobs).m_Memory.m_pMemory = (CBasePaintBlob **)0x0;
      in_stack_ffffffc8 = ppCVar1;
    }
    (this->m_PaintBlobs).m_Memory.m_nAllocationCount = 0;
  }
  C_PhysicsProp::~C_PhysicsProp(&this->super_C_PhysicsProp,(int)in_stack_ffffffc8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_PropPaintBomb::~C_PropPaintBomb at 00539cd0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PhysicsProp": ignoring multiple overlapping fields */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void ~C_PropPaintBomb(C_PropPaintBomb * this, int __in_chrg) */

void __thiscall C_PropPaintBomb::~C_PropPaintBomb(C_PropPaintBomb *this,int __in_chrg)

{
  CBasePaintBlob **ppCVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_PhysicsProp).super_C_BreakableProp.super_C_BaseAnimating.super_C_BaseEntity.
  super_IClientEntity.super_IClientUnknown.super_IHandleEntity =
       (IHandleEntity)(unaff_EBX + 0x6942c4);
  (this->super_C_PhysicsProp).super_C_BreakableProp.super_C_BaseAnimating.super_C_BaseEntity.
  super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x694720);
  (this->super_C_PhysicsProp).super_C_BreakableProp.super_C_BaseAnimating.super_C_BaseEntity.
  super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6947d8);
  (this->super_C_PhysicsProp).super_C_BreakableProp.super_C_BaseAnimating.super_C_BaseEntity.
  super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x694814);
  (this->super_C_PhysicsProp).super_C_BreakableProp.super_C_BaseAnimating.super_C_BaseEntity.
  super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x694830);
  (this->m_PaintBlobs).m_Size = 0;
  if ((this->m_PaintBlobs).m_Memory.m_nGrowSize < 0) {
    ppCVar1 = (this->m_PaintBlobs).m_Memory.m_pMemory;
  }
  else {
    ppCVar1 = (this->m_PaintBlobs).m_Memory.m_pMemory;
    if (ppCVar1 != (CBasePaintBlob **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x62a08c) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x62a08c),ppCVar1);
      (this->m_PaintBlobs).m_Memory.m_pMemory = (CBasePaintBlob **)0x0;
    }
    ppCVar1 = (CBasePaintBlob **)0x0;
    (this->m_PaintBlobs).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintBlobs).m_pElements = ppCVar1;
  if (-1 < (this->m_PaintBlobs).m_Memory.m_nGrowSize) {
    if (ppCVar1 != (CBasePaintBlob **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x62a08c) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x62a08c),ppCVar1);
      (this->m_PaintBlobs).m_Memory.m_pMemory = (CBasePaintBlob **)0x0;
    }
    (this->m_PaintBlobs).m_Memory.m_nAllocationCount = 0;
  }
  C_PhysicsProp::~C_PhysicsProp(&this->super_C_PhysicsProp,__in_chrg);
  return;
}


/* C_PropPaintBomb::Precache at 005390b0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PhysicsProp": ignoring multiple overlapping fields */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void Precache(C_PropPaintBomb * this) */

void __thiscall C_PropPaintBomb::Precache(C_PropPaintBomb *this)

{
  C_BaseEntity::Precache((C_BaseEntity *)this);
  return;
}


/* C_PropPaintBomb::Spawn at 00539050 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PhysicsProp": ignoring multiple overlapping fields */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void Spawn(C_PropPaintBomb * this) */

void __thiscall C_PropPaintBomb::Spawn(C_PropPaintBomb *this)

{
  C_BaseEntity::Spawn((C_BaseEntity *)this);
  _memset(this->m_lightingBoxColors,0,0x60);
  (**(code **)((int)(this->super_C_PhysicsProp).super_C_BreakableProp.super_C_BaseAnimating.
                    super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity
              + 0x274))(this,0xc4a1a000);
  return;
}


/* C_PropPaintBomb::UpdateOnRemove at 00539240 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PhysicsProp": ignoring multiple overlapping fields */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void UpdateOnRemove(C_PropPaintBomb * this) */

void __thiscall C_PropPaintBomb::UpdateOnRemove(C_PropPaintBomb *this)

{
  CBasePaintBlob *this_00;
  int nNewCount;
  CBasePaintBlob **ppCVar1;
  int unaff_EBX;
  int iVar2;
  
  ___i686_get_pc_thunk_bx();
  nNewCount = (*(CPaintStreamManager **)(unaff_EBX + 0x62ad2a))->m_nBlobCount -
              (this->m_PaintBlobs).m_Size;
  CPaintStreamManager::SetBlobCount(*(CPaintStreamManager **)(unaff_EBX + 0x62ad2a),nNewCount);
                    /* Unresolved local var: int i@[???] */
  if ((this->m_PaintBlobs).m_Size < 1) {
    ppCVar1 = (this->m_PaintBlobs).m_Memory.m_pMemory;
  }
  else {
    iVar2 = 0;
    ppCVar1 = (this->m_PaintBlobs).m_Memory.m_pMemory;
    do {
      this_00 = ppCVar1[iVar2];
      if (this_00 != (CBasePaintBlob *)0x0) {
        CBasePaintBlob::~CBasePaintBlob(this_00,nNewCount);
        operator_delete(this_00);
        ppCVar1 = (this->m_PaintBlobs).m_Memory.m_pMemory;
      }
      iVar2 = iVar2 + 1;
    } while (iVar2 < (this->m_PaintBlobs).m_Size);
  }
  (this->m_PaintBlobs).m_Size = 0;
  if (-1 < (this->m_PaintBlobs).m_Memory.m_nGrowSize) {
    if (ppCVar1 != (CBasePaintBlob **)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x62ab22) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x62ab22),ppCVar1);
      ppCVar1 = (CBasePaintBlob **)0x0;
      (this->m_PaintBlobs).m_Memory.m_pMemory = (CBasePaintBlob **)0x0;
    }
    (this->m_PaintBlobs).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_PaintBlobs).m_pElements = ppCVar1;
  C_BaseEntity::UpdateOnRemove((C_BaseEntity *)this);
  return;
}


/* C_PropPaintBomb::OnPreDataChanged at 00539020 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PhysicsProp": ignoring multiple overlapping fields */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void OnPreDataChanged(C_PropPaintBomb * this, DataUpdateType_t
   updateType) */

void __thiscall C_PropPaintBomb::OnPreDataChanged(C_PropPaintBomb *this,DataUpdateType_t updateType)

{
  C_PhysicsProp::OnPreDataChanged(&this->super_C_PhysicsProp,updateType);
  this->m_nPrevPaintPowerType = this->m_nPaintPowerType;
  return;
}


/* C_PropPaintBomb::OnDataChanged at 005390d0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PhysicsProp": ignoring multiple overlapping fields */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void OnDataChanged(C_PropPaintBomb * this, DataUpdateType_t updateType)
    */

void __thiscall C_PropPaintBomb::OnDataChanged(C_PropPaintBomb *this,DataUpdateType_t updateType)

{
  int paintType;
  vec_t *pvVar1;
  Vector *vecBombPos;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_PhysicsProp::OnDataChanged(&this->super_C_PhysicsProp,updateType);
  if (updateType == DATA_UPDATE_CREATED) {
    if (*(int *)(*(int *)(unaff_EBX + 0x830498) + 0x30) != 0) {
      paintType = this->m_nPaintPowerType;
      vecBombPos = (Vector *)
                   (**(code **)((int)(this->super_C_PhysicsProp).super_C_BreakableProp.
                                     super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.
                                     super_IClientUnknown.super_IHandleEntity + 0x34))(this);
      CreateBlobsForPaintBomb(vecBombPos,paintType,&this->m_PaintBlobs);
    }
    pvVar1 = (vec_t *)(**(code **)((int)(this->super_C_PhysicsProp).super_C_BreakableProp.
                                        super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity
                                        .super_IClientUnknown.super_IHandleEntity + 0x34))(this);
    (this->m_vPrevPosition).x = *pvVar1;
    (this->m_vPrevPosition).y = pvVar1[1];
    (this->m_vPrevPosition).z = pvVar1[2];
  }
  return;
}


/* C_PropPaintBomb::PostDataUpdate at 00538fd0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PhysicsProp": ignoring multiple overlapping fields */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void PostDataUpdate(C_PropPaintBomb * this, DataUpdateType_t
   updateType) */

void __thiscall C_PropPaintBomb::PostDataUpdate(C_PropPaintBomb *this,DataUpdateType_t updateType)

{
  C_BaseAnimating::PostDataUpdate((C_BaseAnimating *)this,updateType);
  if (updateType == DATA_UPDATE_CREATED) {
    (this->super_C_PhysicsProp).super_C_BreakableProp.super_C_BaseAnimating.m_bCanUseFastPath =
         false;
  }
  return;
}


/* C_PropPaintBomb::UpdateBlobs at 00538b40 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Restarted to delay deadcode elimination for space: stack */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PhysicsProp": ignoring multiple overlapping fields */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void UpdateBlobs(C_PropPaintBomb * this, float flDeltaTime) */

void __thiscall C_PropPaintBomb::UpdateBlobs(C_PropPaintBomb *this,float flDeltaTime)

{
  uint *puVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  int iVar6;
  CBasePaintBlob *this_00;
  float *pfVar7;
  Vector *pVVar8;
  int unaff_EBX;
  float fVar9;
  float fVar10;
  float fVar11;
  float flMaxDistanceFromBomb;
  int numBlobs;
  int i;
  Vector local_58;
  float local_4c;
  float local_48;
  float local_44;
  float local_40;
  float local_3c;
  float local_38;
  Vector local_34;
  float local_28;
  float local_24;
  float local_20;
  
                    /* Unresolved local var: Vector vecNewBombPos@[???] */
  ___i686_get_pc_thunk_bx();
  pfVar7 = (float *)(**(code **)((int)(this->super_C_PhysicsProp).super_C_BreakableProp.
                                      super_C_BaseAnimating.super_C_BaseEntity.super_IClientEntity.
                                      super_IClientUnknown.super_IHandleEntity + 0x34))(this);
  fVar2 = pfVar7[2];
  fVar3 = pfVar7[1];
  fVar4 = *pfVar7;
  fVar5 = *(float *)(*(int *)(*(int *)(unaff_EBX + 0x62ba87) + 0x1c) + 0x2c);
  iVar6 = (this->m_PaintBlobs).m_Size;
  if (0 < iVar6) {
    i = 0;
    do {
      this_00 = (this->m_PaintBlobs).m_Memory.m_pMemory[i];
                    /* Unresolved local var: Vector res@[???] */
      fVar9 = fVar4 - (this->m_vPrevPosition).x;
      fVar10 = fVar3 - (this->m_vPrevPosition).y;
      fVar11 = fVar2 - (this->m_vPrevPosition).z;
      local_28 = fVar9;
      local_24 = fVar10;
      local_20 = fVar11;
      pVVar8 = CBasePaintBlob::GetPosition(this_00);
                    /* Unresolved local var: Vector res@[???] */
      local_40 = fVar9 + pVVar8->x;
      local_3c = fVar10 + pVVar8->y;
      local_38 = fVar11 + pVVar8->z;
      local_34.x = local_40;
      local_34.y = local_3c;
      local_34.z = local_38;
      pVVar8 = CBasePaintBlob::GetVelocity(this_00);
      fVar9 = pVVar8->z;
      fVar10 = pVVar8->y;
      fVar11 = pVVar8->x;
                    /* Unresolved local var: Vector res@[???] */
      if (fVar5 * fVar5 <
          (local_34.x - fVar4) * (local_34.x - fVar4) + (local_34.y - fVar3) * (local_34.y - fVar3)
          + (local_34.z - fVar2) * (local_34.z - fVar2)) {
                    /* Unresolved local var: Vector res@[???] */
        local_4c = fVar4 - local_34.x;
        local_48 = fVar3 - local_34.y;
        local_44 = fVar2 - local_34.z;
        if (fVar11 * local_4c + fVar10 * local_48 + fVar9 * local_44 < 0.0) {
          puVar1 = (uint *)(unaff_EBX + 0x4ae72f);
          local_58.x = (vec_t)((uint)fVar11 ^ *puVar1);
          local_58.y = (vec_t)((uint)fVar10 ^ *puVar1);
          local_58.z = (vec_t)((uint)fVar9 ^ *puVar1);
          CBasePaintBlob::SetVelocity(this_00,&local_58);
        }
      }
                    /* Unresolved local var: Vector res@[???] */
                    /* Unresolved local var: Vector res@[???] */
      local_34.x = local_34.x + fVar11 * flDeltaTime;
      local_34.y = local_34.y + fVar10 * flDeltaTime;
      local_34.z = local_34.z + flDeltaTime * fVar9;
      CBasePaintBlob::SetPosition(this_00,&local_34);
      i = i + 1;
    } while (i != iVar6);
  }
  (this->m_vPrevPosition).x = fVar4;
  (this->m_vPrevPosition).y = fVar3;
  (this->m_vPrevPosition).z = fVar2;
  return;
}


/* C_PropPaintBomb::ClientThink at 00538ef0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PhysicsProp": ignoring multiple overlapping fields */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void ClientThink(C_PropPaintBomb * this) */

void __thiscall C_PropPaintBomb::ClientThink(C_PropPaintBomb *this)

{
  int *piVar1;
  int unaff_EBX;
  float fVar2;
  float flDeltaTime;
  float fVar3;
  
                    /* Unresolved local var: float flDeltaTime@[???] */
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::ClientThink((C_BaseEntity *)this);
  piVar1 = *(int **)(unaff_EBX + 0x62ae9f);
  fVar2 = (*(float *)(*piVar1 + 0xc) - this->m_flLastUpdateTime) + this->m_flAccumulatedTime;
  this->m_flAccumulatedTime = fVar2;
  this->m_flLastUpdateTime = *(float *)(*piVar1 + 0xc);
  fVar3 = *(float *)(&DAT_004a9287 + unaff_EBX) /
          *(float *)(*(int *)(*(int *)(unaff_EBX + 0x62b6df) + 0x1c) + 0x2c);
  do {
    flDeltaTime = fVar3;
    if (fVar2 <= 0.0) goto LAB_00538fa6;
    flDeltaTime = fVar2;
    if (fVar3 <= fVar2) {
      flDeltaTime = fVar3;
    }
    UpdateBlobs(this,flDeltaTime);
    fVar2 = this->m_flAccumulatedTime - flDeltaTime;
    this->m_flAccumulatedTime = fVar2;
    fVar3 = flDeltaTime;
  } while (0.0 <= fVar2);
  this->m_flAccumulatedTime = 0.0;
LAB_00538fa6:
  (**(code **)((int)(this->super_C_PhysicsProp).super_C_BreakableProp.super_C_BaseAnimating.
                    super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity
              + 0x274))(this,flDeltaTime);
  return;
}


/* C_PropPaintBomb::DrawModel at 005394d0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PhysicsProp": ignoring multiple overlapping fields */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: int DrawModel(C_PropPaintBomb * this, int flags, RenderableInstance_t *
   instance) */

int __thiscall
C_PropPaintBomb::DrawModel(C_PropPaintBomb *this,int flags,RenderableInstance_t *instance)

{
  CUtlMemoryAligned<ImpParticleWithFourInterpolants,16> *pCVar1;
  undefined4 *puVar2;
  void *src;
  CBasePaintBlob *this_00;
  ImpParticleWithFourInterpolants *pIVar3;
  code *pcVar4;
  Vector *pVVar5;
  int *piVar6;
  C_BasePlayer *pCVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  char *pcVar11;
  IMaterial *pMaterial;
  IClientRenderable *pClientRenderable;
  int unaff_EBX;
  int iVar12;
  int iVar13;
  float *pfVar14;
  float fVar15;
  float fVar16;
  float fVar17;
  float fVar18;
  float fVar19;
  float fVar20;
  int nCount;
  int local_70;
  int local_6c;
  int local_54;
  PortalMatrixList_t local_48;
  float local_34;
  float local_30;
  float local_2c;
  Vector local_28 [2];
  
                    /* Unresolved local var: Vector vCenter@[???]
                       Unresolved local var: C_BasePlayer * pPlayer@[???]
                       Unresolved local var: Vector vecPlayerPos@[???]
                       Unresolved local var: float flDistThreshold@[???]
                       Unresolved local var: int iNumParticles@[???]
                       Unresolved local var: IMaterial * pMaterial@[???]
                       Unresolved local var: PortalMatrixList_t portalMatrixList@[???] */
  ___i686_get_pc_thunk_bx();
  if ((*(int *)(*(int *)(*(int *)(unaff_EBX + 0x62b0ff) + 0x1c) + 0x30) == 0) ||
     (iVar10 = (this->m_PaintBlobs).m_Size, iVar10 == 0)) {
    return 0;
  }
  pfVar14 = *(float **)(unaff_EBX + 0x62a91f);
  local_28[0].x = *pfVar14;
  local_28[0].y = pfVar14[1];
  local_28[0].z = pfVar14[2];
                    /* Unresolved local var: int i@[???] */
  if (0 < iVar10) {
    iVar12 = 0;
    do {
      pVVar5 = CBasePaintBlob::GetPosition((this->m_PaintBlobs).m_Memory.m_pMemory[iVar12]);
      local_28[0].x = local_28[0].x + pVVar5->x;
      local_28[0].y = local_28[0].y + pVVar5->y;
      local_28[0].z = local_28[0].z + pVVar5->z;
      iVar12 = iVar12 + 1;
    } while (iVar12 != iVar10);
  }
                    /* Unresolved local var: float oofl@[???] */
  fVar19 = *(float *)(unaff_EBX + 0x4a8ca3) / (float)iVar10;
  local_28[0].x = local_28[0].x * fVar19;
  local_28[0].y = local_28[0].y * fVar19;
  local_28[0].z = local_28[0].z * fVar19;
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x62aa5f) + 0x50))
            ((int *)**(undefined4 **)(unaff_EBX + 0x62aa5f),local_28);
  NPaintRenderer::Paintblob_SafeLightCubeUpdate(local_28,this->m_lightingBoxColors);
  piVar6 = (int *)**(int **)(unaff_EBX + 0x62aa2b);
  if ((piVar6 != (int *)0x0) && (iVar12 = (**(code **)(*piVar6 + 0x1a0))(piVar6), iVar12 != 0)) {
    piVar6 = (int *)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x62aa2b) + 0x1a0))
                              ((int *)**(undefined4 **)(unaff_EBX + 0x62aa2b));
    (**(code **)(*piVar6 + 0x40))(piVar6,this->m_lightingBoxColors);
  }
  iVar12 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x62a8c3) + 0x1f8))
                     ((int *)**(undefined4 **)(unaff_EBX + 0x62a8c3));
  pCVar7 = GetSplitScreenViewPlayer(iVar12);
  (**(code **)(*(int *)&(pCVar7->super_C_BaseCombatCharacter).super_C_BaseFlex.
                        super_C_BaseAnimatingOverlay.super_C_BaseAnimating.super_C_BaseEntity.
                        super_IClientEntity.super_IClientUnknown + 0x330))(&local_34,pCVar7);
  fVar19 = *(float *)(*(int *)(*(int *)(unaff_EBX + 0x62b0f3) + 0x1c) + 0x2c);
  iVar12 = (this->m_PaintBlobs).m_Size;
  pCVar1 = *(CUtlMemoryAligned<ImpParticleWithFourInterpolants,16> **)(unaff_EBX + 0x62b0df);
  pCVar1[1].super_CUtlMemory<ImpParticleWithFourInterpolants,int>.m_pMemory =
       (ImpParticleWithFourInterpolants *)0x0;
  if (iVar12 != 0) {
    iVar8 = (pCVar1->super_CUtlMemory<ImpParticleWithFourInterpolants,int>).m_nAllocationCount;
    if (iVar8 < iVar12) {
      CUtlMemoryAligned<ImpParticleWithFourInterpolants,16>::Grow(pCVar1,iVar12 - iVar8);
    }
    puVar2 = *(undefined4 **)(unaff_EBX + 0x62b0df);
    iVar8 = puVar2[3];
    puVar2[3] = iVar12 + iVar8;
    src = (void *)*puVar2;
    puVar2[4] = src;
                    /* Unresolved local var: int numToMove@[???] */
    iVar8 = (iVar12 + iVar8) - iVar12;
    if (0 < iVar8) {
      if (0 >= iVar12) goto LAB_00539734;
      _V_memmove((void *)((int)src + iVar12 * 0x60),src,iVar8 * 0x60);
    }
                    /* Unresolved local var: int i@[???] */
    if (0 < iVar12) {
      iVar13 = 0;
      iVar8 = 0;
      do {
        iVar9 = iVar8 + **(int **)(unaff_EBX + 0x62b0df);
        if (iVar9 != 0) {
          *(undefined4 *)(iVar9 + 0xc) = 0;
          *(undefined4 *)(iVar9 + 0x2c) = 0;
          *(undefined4 *)(iVar9 + 0x3c) = 0;
          *(undefined4 *)(iVar9 + 0x4c) = 0;
          *(undefined4 *)(iVar9 + 0x5c) = 0;
        }
        iVar13 = iVar13 + 1;
        iVar8 = iVar8 + 0x60;
      } while (iVar13 < iVar12);
    }
  }
LAB_00539734:
  local_70 = 0;
  if (0 < iVar10) {
    local_6c = 0;
    local_54 = 0;
    do {
      while( true ) {
        this_00 = (this->m_PaintBlobs).m_Memory.m_pMemory[local_6c];
        pVVar5 = CBasePaintBlob::GetPosition(this_00);
        fVar16 = pVVar5->z;
        fVar20 = pVVar5->y;
                    /* Unresolved local var: Vector delta@[???] */
        fVar15 = pVVar5->x - local_34;
        fVar17 = fVar20 - local_30;
        fVar18 = fVar16 - local_2c;
        if (fVar15 * fVar15 + fVar17 * fVar17 + fVar18 * fVar18 <= fVar19 * fVar19) break;
        pfVar14 = (float *)(local_54 + **(int **)(unaff_EBX + 0x62b0df));
        *pfVar14 = pVVar5->x;
        pfVar14[1] = fVar20;
        pfVar14[2] = fVar16;
        pfVar14[3] = 0.0;
        fVar20 = CBasePaintBlob::GetRadiusScale(this_00);
        pfVar14[4] = fVar20;
        fVar16 = *(float *)(unaff_EBX + 0x4a8ca3);
        if ((fVar20 <= fVar16) && (fVar16 = *(float *)(unaff_EBX + 0x4adce3), 0.0 < fVar20)) {
          fVar20 = *(float *)(unaff_EBX + 0x4a8ca3) / fVar20;
          fVar16 = fVar20 * fVar20;
        }
        pfVar14[3] = fVar16;
        pfVar14[8] = 1.0;
        pfVar14[9] = 1.0;
        pfVar14[10] = 1.0;
        pfVar14[0xb] = 0.0;
        local_70 = local_70 + 1;
        local_6c = local_6c + 1;
        local_54 = local_54 + 0x60;
        if (local_6c == iVar10) goto LAB_00539850;
      }
      local_6c = local_6c + 1;
      local_54 = local_54 + 0x60;
    } while (local_6c != iVar10);
  }
LAB_00539850:
  pCVar1 = *(CUtlMemoryAligned<ImpParticleWithFourInterpolants,16> **)(unaff_EBX + 0x62b0df);
  pIVar3 = pCVar1[1].super_CUtlMemory<ImpParticleWithFourInterpolants,int>.m_pMemory;
  local_70 = local_70 - (int)pIVar3;
  if (local_70 < 1) {
    if (local_70 != 0) {
      *(int *)(*(int *)(unaff_EBX + 0x62b0df) + 0xc) = (int)pIVar3 + local_70;
    }
  }
  else {
    iVar8 = local_70 + (int)pIVar3;
    iVar10 = (pCVar1->super_CUtlMemory<ImpParticleWithFourInterpolants,int>).m_nAllocationCount;
    iVar12 = iVar8;
    if (iVar10 < iVar8) {
      CUtlMemoryAligned<ImpParticleWithFourInterpolants,16>::Grow(pCVar1,iVar8 - iVar10);
      iVar12 = local_70 + *(int *)(*(int *)(unaff_EBX + 0x62b0df) + 0xc);
    }
    piVar6 = *(int **)(unaff_EBX + 0x62b0df);
    piVar6[3] = iVar12;
    iVar10 = *piVar6;
    piVar6[4] = iVar10;
    iVar12 = (iVar12 - (int)pIVar3) - local_70;
    if (0 < iVar12) {
      _V_memmove((void *)(iVar10 + iVar8 * 0x60),(void *)(iVar10 + (int)pIVar3 * 0x60),iVar12 * 0x60
                );
    }
    iVar10 = (int)pIVar3 * 0x60;
    iVar12 = 0;
                    /* Unresolved local var: int i@[???] */
    do {
      iVar8 = iVar10 + **(int **)(unaff_EBX + 0x62b0df);
      if (iVar8 != 0) {
        *(undefined4 *)(iVar8 + 0xc) = 0;
        *(undefined4 *)(iVar8 + 0x2c) = 0;
        *(undefined4 *)(iVar8 + 0x3c) = 0;
        *(undefined4 *)(iVar8 + 0x4c) = 0;
        *(undefined4 *)(iVar8 + 0x5c) = 0;
      }
      iVar12 = iVar12 + 1;
      iVar10 = iVar10 + 0x60;
    } while (iVar12 != local_70);
  }
  puVar2 = *(undefined4 **)(unaff_EBX + 0x62aa2b);
  pcVar4 = *(code **)(*(int *)*puVar2 + 0x130);
  pcVar11 = CPaintStreamManager::GetPaintMaterialName
                      (*(CPaintStreamManager **)(unaff_EBX + 0x62aa97),this->m_nPaintPowerType);
  pMaterial = (IMaterial *)(*pcVar4)(*puVar2,pcVar11,unaff_EBX + 0x46e133,1,0);
  local_48.m_Memory.m_pMemory = (matrix3x4_t *)0x0;
  local_48.m_Memory.m_nAllocationCount = 0;
  local_48.m_Memory.m_nGrowSize = 0;
  local_48.m_Size = 0;
  local_48.m_pElements = (matrix3x4_t *)0x0;
  fVar19 = *(float *)(*(int *)(*(int *)(unaff_EBX + 0x62b103) + 0x1c) + 0x2c);
  pClientRenderable =
       (IClientRenderable *)
       (**(code **)((int)(this->super_C_PhysicsProp).super_C_BreakableProp.super_C_BaseAnimating.
                         super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
                         super_IHandleEntity + 0x18))(this);
  NPaintRenderer::Paintblob_Draw(0,pClientRenderable,local_28,pMaterial,fVar19,&local_48,false);
  local_48.m_Size = 0;
  if (-1 < local_48.m_Memory.m_nGrowSize) {
    if (local_48.m_Memory.m_pMemory != (matrix3x4_t *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x62a88f) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x62a88f),local_48.m_Memory.m_pMemory);
      local_48.m_Memory.m_pMemory = (matrix3x4_t *)0x0;
    }
    local_48.m_Memory.m_nAllocationCount = 0;
  }
  if ((-1 < local_48.m_Memory.m_nGrowSize) && (local_48.m_Memory.m_pMemory != (matrix3x4_t *)0x0)) {
    local_48.m_pElements = local_48.m_Memory.m_pMemory;
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x62a88f) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x62a88f),local_48.m_Memory.m_pMemory);
    return 1;
  }
  return 1;
}


/* C_PropPaintBomb::CleansePaintPower at 00538b30 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PhysicsProp": ignoring multiple overlapping fields */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "C_BaseVPhysicsTrigger": ignoring overlapping field "m_bDisabled" */
/* DWARF original prototype: void CleansePaintPower(C_PropPaintBomb * this) */

void __thiscall C_PropPaintBomb::CleansePaintPower(C_PropPaintBomb *this)

{
  return;
}


/* _GLOBAL__I__ZN15C_PropPaintBomb17m_pClassRecvTableE at 00078fe0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN15C_PropPaintBomb17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

