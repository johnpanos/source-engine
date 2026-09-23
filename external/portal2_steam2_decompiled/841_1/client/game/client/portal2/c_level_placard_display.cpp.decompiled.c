/* DWARF-guided pseudocode for game/client/portal2/c_level_placard_display.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* ClientClassInit<DT_LevelPlacardDisplay::ignored> at 00074440 */

int ClientClassInit<DT_LevelPlacardDisplay::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&CDmxAttribute::GetValue<bool>()::defaultValue + unaff_EBX + 3) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc5c6e4);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)(unaff_EBX + 0xc5c6ec),(char *)(unaff_EBX + 0x91631c),0,4,0,
                  (RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x28),
                        (char *)(unaff_EBX + 0x91ae60),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00aef9dc + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00aef998 + unaff_EBX));
      RecvPropBool((RecvProp *)((int)s_pRandomFloats + unaff_EBX + 100),
                   (char *)(unaff_EBX + 0x92e980),0xa5d,1);
      ___cxa_guard_release(unaff_EBX + 0xc5c6e4);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0xf68),
             (RecvProp_conflict1 *)((int)s_pRandomFloats + unaff_EBX + 0x28),2,
             (char *)(unaff_EBX + 0x9451fc));
  return 1;
}


/* __static_initialization_and_destruction_0 at 00074590 */

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
  *(undefined1 *)((int)&CPotteryWheelPanel::GetMessageMap::s_pMap + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&CDmxElement::GetArray<Vector>(char_const*)::defaultValue + unaff_EBX) = 0;
  *(undefined1 *)((int)&CDmxElement::GetArray<Vector>(char_const*)::defaultValue + unaff_EBX + 1) =
       0;
  *(undefined1 *)((int)&CDmxElement::GetArray<Vector>(char_const*)::defaultValue + unaff_EBX + 2) =
       0;
  *(undefined4 *)((int)&CDmxElement::GetArray<Vector>(char_const*)::defaultValue + unaff_EBX + 3) =
       0;
  *(undefined4 *)(&DAT_00c5c567 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c5c56b + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c5c56f + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c5c573 + unaff_EBX) = 0;
  *(undefined4 *)((int)&CDmxAttribute::GetArray<Vector>()::defaultArray + unaff_EBX + 3) = 0;
  *(undefined4 *)(&DAT_00c5c57b + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c5c57f + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c5c583 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c5c587 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&CDmxElement::GetArray<CDmxElement*>(char_const*)::defaultValue + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)(&DAT_00c5c58f + unaff_EBX) = 0x7f7fffff;
  *(undefined **)(&DAT_00c5c593 + unaff_EBX) = &UNK_00af08e7 + unaff_EBX;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0xe0b) = unaff_EBX + 0x9450c6;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0xe03) = unaff_EBX + 0x4a89bf;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0xe07) = 0;
  this = (RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0xe1b);
  *(RecvTable **)((int)s_pParticlePtrs + unaff_EBX + 0xe0f) = this;
  piVar1 = *(int **)(&DAT_00aef843 + unaff_EBX);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0xe13) = *piVar1;
  *piVar1 = unaff_EBX + 0xcf39c3;
  RecvTable::RecvTable(this);
  ___cxa_atexit(unaff_EBX + 0x90718f,0,*(undefined4 *)(&DAT_00aef7cb + unaff_EBX));
  iVar2 = ClientClassInit<DT_LevelPlacardDisplay::ignored>((ignored *)0x0);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0xe2f) = iVar2;
  return;
}


/* C_LevelPlacardDisplay::YouForgotToImplementOrDeclareClientClass at 0051cd20 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_LevelPlacardDisplay": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_LevelPlacardDisplay *
   this) */

int __thiscall
C_LevelPlacardDisplay::YouForgotToImplementOrDeclareClientClass(C_LevelPlacardDisplay *this)

{
  return 0;
}


/* C_LevelPlacardDisplay::GetClientClass at 0051cd40 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_LevelPlacardDisplay": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: ClientClass * GetClientClass(C_LevelPlacardDisplay * this) */

ClientClass * __thiscall C_LevelPlacardDisplay::GetClientClass(C_LevelPlacardDisplay *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x84b21c);
}


/* _C_LevelPlacardDisplay_CreateObject at 0051cf60 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_LevelPlacardDisplay_CreateObject(int entnum,int serialNum)

{
  C_BaseEntity *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_LevelPlacardDisplay * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xa60);
  C_BaseEntity::C_BaseEntity(this);
  (this->super_IClientEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x6ac574);
  (this->super_IClientEntity).super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6ac910);
  (this->super_IClientEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6ac9c8);
  (this->super_IClientEntity).super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6aca04);
  (this->super_IClientModelRenderable)._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6aca20);
  (**(code **)(unaff_EBX + 0x6ac630))(this,entnum,serialNum);
  return &(this->super_IClientEntity).super_IClientNetworkable;
}


/* __tcf_0 at 0097b730 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3ec844),in_stack_00000008);
  return;
}


/* C_LevelPlacardDisplay::C_LevelPlacardDisplay at 0051ce80 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_LevelPlacardDisplay": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: void C_LevelPlacardDisplay(C_LevelPlacardDisplay * this) */

void __thiscall C_LevelPlacardDisplay::C_LevelPlacardDisplay(C_LevelPlacardDisplay *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::C_BaseEntity(&this->super_C_BaseEntity);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6ac657);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6ac9f3);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6acaab);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6acae7);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6acb03);
  return;
}


/* C_LevelPlacardDisplay::C_LevelPlacardDisplay at 0051cef0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_LevelPlacardDisplay": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: void C_LevelPlacardDisplay(C_LevelPlacardDisplay * this) */

void __thiscall C_LevelPlacardDisplay::C_LevelPlacardDisplay(C_LevelPlacardDisplay *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::C_BaseEntity(&this->super_C_BaseEntity);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6ac5e7);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6ac983);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6aca3b);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6aca77);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6aca93);
  return;
}


/* C_LevelPlacardDisplay::~C_LevelPlacardDisplay at 0051cd50 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_LevelPlacardDisplay": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: void ~C_LevelPlacardDisplay(C_LevelPlacardDisplay * this, int
   __in_chrg) */

void __thiscall
C_LevelPlacardDisplay::~C_LevelPlacardDisplay(C_LevelPlacardDisplay *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6ac787);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6acb23);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(&UNK_006acbdb + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(&UNK_006acc17 + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6acc33);
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,in_stack_ffffffe8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_LevelPlacardDisplay::~C_LevelPlacardDisplay at 0051cdc0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_LevelPlacardDisplay": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: void ~C_LevelPlacardDisplay(C_LevelPlacardDisplay * this, int
   __in_chrg) */

void __thiscall
C_LevelPlacardDisplay::~C_LevelPlacardDisplay(C_LevelPlacardDisplay *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x6ac720);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(extraout_ECX + 0x6acabc);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(extraout_ECX + 0x6acb74);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(&UNK_006acbb0 + extraout_ECX);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(&UNK_006acbcc + extraout_ECX);
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
  return;
}


/* C_LevelPlacardDisplay::~C_LevelPlacardDisplay at 0051ce20 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_LevelPlacardDisplay": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: void ~C_LevelPlacardDisplay(C_LevelPlacardDisplay * this, int
   __in_chrg) */

void __thiscall
C_LevelPlacardDisplay::~C_LevelPlacardDisplay(C_LevelPlacardDisplay *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x6ac6c0);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(extraout_ECX + 0x6aca5c);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(extraout_ECX + 0x6acb14);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(extraout_ECX + 0x6acb50);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(extraout_ECX + 0x6acb6c);
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
  return;
}


/* _GLOBAL__I__ZN21C_LevelPlacardDisplay17m_pClassRecvTableE at 000746b0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN21C_LevelPlacardDisplay17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

