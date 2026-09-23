/* DWARF-guided pseudocode for game/client/portal2/c_prop_linked_portal_door.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* _C_PropLinkedPortalDoor_CreateObject at 005388c0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_PropLinkedPortalDoor_CreateObject(int entnum,int serialNum)

{
  C_BaseAnimating *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_PropLinkedPortalDoor * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xd88);
  C_BaseAnimating::C_BaseAnimating(this);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6950d4);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(&UNK_00695538 + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6955f0);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x69562c);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x695648);
  this[1].super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x695654);
  (**(code **)(unaff_EBX + 0x695190))(this,entnum,serialNum);
  return &(this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable;
}


/* C_PropLinkedPortalDoor::~C_PropLinkedPortalDoor at 005389a0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_PropLinkedPortalDoor(C_PropLinkedPortalDoor * this, int
   __in_chrg) */

void __thiscall
C_PropLinkedPortalDoor::~C_PropLinkedPortalDoor(C_PropLinkedPortalDoor *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(&UNK_00694ffb + unaff_EBX);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(unaff_EBX + 0x69545f);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(&UNK_00695517 + unaff_EBX);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(&UNK_00695553 + unaff_EBX);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(&DAT_0069556f + unaff_EBX);
  (this->super_CSignifierTarget).super_ISignifierTarget._vptr_ISignifierTarget =
       (_func_int_varargs **)(&UNK_0069557b + unaff_EBX);
  C_BaseAnimating::~C_BaseAnimating(&this->super_C_BaseAnimating,in_stack_ffffffe8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_PropLinkedPortalDoor::~C_PropLinkedPortalDoor at 00538a20 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void ~C_PropLinkedPortalDoor(C_PropLinkedPortalDoor * this, int
   __in_chrg) */

void __thiscall
C_PropLinkedPortalDoor::~C_PropLinkedPortalDoor(C_PropLinkedPortalDoor *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x694f80);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientRenderable.
  _vptr_IClientRenderable = (_func_int_varargs **)(extraout_ECX + 0x6953e4);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientNetworkable.
  _vptr_IClientNetworkable = (_func_int_varargs **)(&UNK_0069549c + extraout_ECX);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.super_IClientThinkable.
  _vptr_IClientThinkable = (_func_int_varargs **)(&UNK_006954d8 + extraout_ECX);
  (this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientModelRenderable.
  _vptr_IClientModelRenderable = (_func_int_varargs **)(&UNK_006954f4 + extraout_ECX);
  (this->super_CSignifierTarget).super_ISignifierTarget._vptr_ISignifierTarget =
       (_func_int_varargs **)(&UNK_00695500 + extraout_ECX);
  C_BaseAnimating::~C_BaseAnimating(&this->super_C_BaseAnimating,__in_chrg);
  return;
}


/* C_PropLinkedPortalDoor::OverrideSignifierPosition at 00538990 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: bool OverrideSignifierPosition(C_PropLinkedPortalDoor * this) */

bool __thiscall C_PropLinkedPortalDoor::OverrideSignifierPosition(C_PropLinkedPortalDoor *this)

{
  return true;
}


/* C_PropLinkedPortalDoor::GetSignifierPosition at 00538aa0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: bool GetSignifierPosition(C_PropLinkedPortalDoor * this, Vector *
   vSource, Vector * vPositionOut, Vector * vNormalOut) */

bool __thiscall
C_PropLinkedPortalDoor::GetSignifierPosition
          (C_PropLinkedPortalDoor *this,Vector *vSource,Vector *vPositionOut,Vector *vNormalOut)

{
  vec_t *pvVar1;
  
  pvVar1 = (vec_t *)(*(this->super_C_BaseAnimating).super_C_BaseEntity.super_IClientEntity.
                      super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity[0x6e])(this);
  vPositionOut->x = *pvVar1;
  vPositionOut->y = pvVar1[1];
  vPositionOut->z = pvVar1[2];
  C_BaseEntity::GetVectors((C_BaseEntity *)this,vNormalOut,(Vector *)0x0,(Vector *)0x0);
  return true;
}


/* ClientClassInit<DT_PropLinkedPortalDoor::ignored> at 00078ad0 */

int ClientClassInit<DT_PropLinkedPortalDoor::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&DAT_00c5a074)[unaff_EBX] == '\0') {
    iVar1 = ___cxa_guard_acquire(&DAT_00c5a074 + unaff_EBX);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)(unaff_EBX + 0xc5a07c),(char *)(unaff_EBX + 0x911c8c),0,4,0,
                  (RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)&vgui::URLButton::GetKBMap()::s_pMap + unaff_EBX),
                        (char *)(unaff_EBX + 0x9167d0),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00aeb310 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00aeb308 + unaff_EBX));
      ___cxa_guard_release(&DAT_00c5a074 + unaff_EBX);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_SIMDRandContexts[0x1f].m_RandY[8] + unaff_EBX + 0xc),
             (RecvProp_conflict1 *)((int)&vgui::URLButton::GetKBMap()::s_pMap + unaff_EBX),1,
             &UNK_009428ad + unaff_EBX);
  return 1;
}


/* __static_initialization_and_destruction_0 at 00078bf0 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  RecvTable *this;
  int *piVar1;
  int iVar2;
  int unaff_EBX;
  longlong lVar3;
  
  lVar3 = ___i686_get_pc_thunk_bx();
  if (lVar3 != 0xffff00000001) {
    return;
  }
  (&DAT_00c59f1f)[unaff_EBX] = 0;
  *(undefined1 *)((int)&CCvarTextEntry::GetKBMap()::s_pMap + unaff_EBX) = 0;
  *(undefined1 *)((int)&CCvarTextEntry::GetKBMap()::s_pMap + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&CCvarTextEntry::GetKBMap()::s_pMap + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&CCvarTextEntry::GetKBMap()::s_pMap + unaff_EBX + 3) = 0;
  *(undefined4 *)(&DAT_00c59f27 + unaff_EBX) = 0;
  *(undefined4 *)((int)&CCvarTextEntry::GetKBMap::s_pMap + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&CCvarTextEntry::GetMessageMap::s_pMap + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&CCvarTextEntry::PanelMessageFunc_OnTextChanged::InitVar()::bAdded + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c59f37 + unaff_EBX) = 0;
  *(undefined4 *)((int)&CCvarToggleCheckButton::GetKBMap::s_pMap + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&CCvarToggleCheckButton::GetMessageMap::s_pMap + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&CCvarToggleCheckButton::PanelMessageFunc_OnButtonChecked::InitVar()::bAdded + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)(&DAT_00c59f47 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&vgui::CConsolePanel::GetMessageMap()::s_pMap + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c59f4f + unaff_EBX) = 0x7f7fffff;
  *(undefined **)((int)&vgui::CConsolePanel::GetKBMap()::s_pMap + unaff_EBX + 3) =
       &UNK_00aec287 + unaff_EBX;
  *(int *)((int)s_SIMDRandContexts[0x1e].m_RandY[0x2d] + unaff_EBX + 0xf) = unaff_EBX + 0x9427a8;
  *(int *)((int)s_SIMDRandContexts[0x1e].m_RandY[0x2d] + unaff_EBX + 7) = unaff_EBX + 0x4bfcbf;
  *(undefined4 *)((int)s_SIMDRandContexts[0x1e].m_RandY[0x2d] + unaff_EBX + 0xb) = 0;
  this = (RecvTable *)((int)s_SIMDRandContexts[0x1e].m_RandY[0x2e] + unaff_EBX + 0xf);
  *(RecvTable **)((int)s_SIMDRandContexts[0x1e].m_RandY[0x2e] + unaff_EBX + 3) = this;
  piVar1 = *(int **)(&DAT_00aeb1e3 + unaff_EBX);
  *(int *)((int)s_SIMDRandContexts[0x1e].m_RandY[0x2e] + unaff_EBX + 7) = *piVar1;
  *piVar1 = unaff_EBX + 0xcf08d7;
  RecvTable::RecvTable(this);
  ___cxa_atexit(unaff_EBX + 0x90369f,0,*(undefined4 *)(&DAT_00aeb16b + unaff_EBX));
  iVar2 = ClientClassInit<DT_PropLinkedPortalDoor::ignored>((ignored *)0x0);
  *(int *)((int)s_SIMDRandContexts[0x1e].m_RandY[0x30] + unaff_EBX + 3) = iVar2;
  return;
}


/* C_PropLinkedPortalDoor::YouForgotToImplementOrDeclareClientClass at 00538890 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_PropLinkedPortalDoor *
   this) */

int __thiscall
C_PropLinkedPortalDoor::YouForgotToImplementOrDeclareClientClass(C_PropLinkedPortalDoor *this)

{
  return 0;
}


/* C_PropLinkedPortalDoor::GetClientClass at 005388b0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: ClientClass * GetClientClass(C_PropLinkedPortalDoor * this) */

ClientClass * __thiscall C_PropLinkedPortalDoor::GetClientClass(C_PropLinkedPortalDoor *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x830c20);
}


/* __tcf_0 at 0097c2a0 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3ed248),in_stack_00000008);
  return;
}


/* _GLOBAL__I__ZN22C_PropLinkedPortalDoor17m_pClassRecvTableE at 00078d10 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN22C_PropLinkedPortalDoor17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

