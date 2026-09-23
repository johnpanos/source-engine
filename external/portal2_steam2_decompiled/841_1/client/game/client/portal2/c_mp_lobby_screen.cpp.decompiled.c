/* DWARF-guided pseudocode for game/client/portal2/c_mp_lobby_screen.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* ClientClassInit<DT_MP_LobbyScreen::ignored> at 000746d0 */

int ClientClassInit<DT_MP_LobbyScreen::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&DAT_00c5c554)[unaff_EBX] == '\0') {
    iVar1 = ___cxa_guard_acquire(&DAT_00c5c554 + unaff_EBX);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&CPotteryWheelPanel::GetMessageMap::s_pMap + unaff_EBX),
                  (char *)(unaff_EBX + 0x91608c),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)(unaff_EBX + 0xc5c598),(char *)(unaff_EBX + 0x91abd0),
                        0,0,(RecvTable *)**(undefined4 **)(&DAT_00aef74c + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00aef708 + unaff_EBX));
      RecvPropBool((RecvProp *)(unaff_EBX + 0xc5c5d4),(char *)(unaff_EBX + 0x92e6f0),0xa5d,1);
      ___cxa_guard_release(&DAT_00c5c554 + unaff_EBX);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0xd08),
             (RecvProp_conflict1 *)(unaff_EBX + 0xc5c598),2,(char *)(unaff_EBX + 0x944fb0));
  return 1;
}


/* __static_initialization_and_destruction_0 at 00074820 */

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
  *(undefined1 *)((int)s_randomGradients + unaff_EBX + 0xacf) = 0;
  *(undefined1 *)((int)s_randomGradients + unaff_EBX + 0xad0) = 0;
  *(undefined1 *)((int)s_randomGradients + unaff_EBX + 0xad1) = 0;
  *(undefined1 *)((int)s_randomGradients + unaff_EBX + 0xad2) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0xad3) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0xad7) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0xadb) = 0x7f7fffff;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0xadf) = 0x7f7fffff;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0xae3) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0xae7) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0xaeb) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0xaef) = 0;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0xaf3) = 0x7f7fffff;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0xaf7) = 0x7f7fffff;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0xafb) = 0x7f7fffff;
  *(undefined4 *)((int)s_randomGradients + unaff_EBX + 0xaff) = 0x7f7fffff;
  *(undefined **)((int)s_randomGradients + unaff_EBX + 0xb03) = &UNK_00af0657 + unaff_EBX;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0xbab) = unaff_EBX + 0x944e75;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0xba3) = unaff_EBX + 0x4a8a1f;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0xba7) = 0;
  this = (RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0xbbb);
  *(RecvTable **)((int)s_pParticlePtrs + unaff_EBX + 0xbaf) = this;
  piVar1 = *(int **)(&DAT_00aef5b3 + unaff_EBX);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0xbb3) = *piVar1;
  *piVar1 = unaff_EBX + 0xcf3763;
  RecvTable::RecvTable(this);
  ___cxa_atexit(unaff_EBX + 0x906f1f,0,*(undefined4 *)(&DAT_00aef53b + unaff_EBX));
  iVar2 = ClientClassInit<DT_MP_LobbyScreen::ignored>((ignored *)0x0);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0xbcf) = iVar2;
  return;
}


/* C_MP_LobbyScreen::YouForgotToImplementOrDeclareClientClass at 0051d010 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_MP_LobbyScreen": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_MP_LobbyScreen * this)
    */

int __thiscall C_MP_LobbyScreen::YouForgotToImplementOrDeclareClientClass(C_MP_LobbyScreen *this)

{
  return 0;
}


/* C_MP_LobbyScreen::GetClientClass at 0051d030 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_MP_LobbyScreen": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: ClientClass * GetClientClass(C_MP_LobbyScreen * this) */

ClientClass * __thiscall C_MP_LobbyScreen::GetClientClass(C_MP_LobbyScreen *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x84af5c);
}


/* _C_MP_LobbyScreen_CreateObject at 0051d250 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_MP_LobbyScreen_CreateObject(int entnum,int serialNum)

{
  C_BaseEntity *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_MP_LobbyScreen * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xa60);
  C_BaseEntity::C_BaseEntity(this);
  (this->super_IClientEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x6ac764);
  (this->super_IClientEntity).super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6acb00);
  (this->super_IClientEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(&UNK_006acbb8 + unaff_EBX);
  (this->super_IClientEntity).super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(&UNK_006acbf4 + unaff_EBX);
  (this->super_IClientModelRenderable)._vptr_IClientModelRenderable =
       (_func_int_varargs **)(&UNK_006acc10 + unaff_EBX);
  (**(code **)(unaff_EBX + 0x6ac820))(this,entnum,serialNum);
  return &(this->super_IClientEntity).super_IClientNetworkable;
}


/* __tcf_0 at 0097b750 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3ec854),in_stack_00000008);
  return;
}


/* C_MP_LobbyScreen::C_MP_LobbyScreen at 0051d170 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_MP_LobbyScreen": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: void C_MP_LobbyScreen(C_MP_LobbyScreen * this) */

void __thiscall C_MP_LobbyScreen::C_MP_LobbyScreen(C_MP_LobbyScreen *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::C_BaseEntity(&this->super_C_BaseEntity);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6ac847);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(&UNK_006acbe3 + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6acc9b);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6accd7);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6accf3);
  return;
}


/* C_MP_LobbyScreen::C_MP_LobbyScreen at 0051d1e0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_MP_LobbyScreen": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: void C_MP_LobbyScreen(C_MP_LobbyScreen * this) */

void __thiscall C_MP_LobbyScreen::C_MP_LobbyScreen(C_MP_LobbyScreen *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::C_BaseEntity(&this->super_C_BaseEntity);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6ac7d7);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6acb73);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6acc2b);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6acc67);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6acc83);
  return;
}


/* C_MP_LobbyScreen::~C_MP_LobbyScreen at 0051d040 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_MP_LobbyScreen": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: void ~C_MP_LobbyScreen(C_MP_LobbyScreen * this, int __in_chrg) */

void __thiscall C_MP_LobbyScreen::~C_MP_LobbyScreen(C_MP_LobbyScreen *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6ac977);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6acd13);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6acdcb);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6ace07);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6ace23);
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,in_stack_ffffffe8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_MP_LobbyScreen::~C_MP_LobbyScreen at 0051d0b0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_MP_LobbyScreen": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: void ~C_MP_LobbyScreen(C_MP_LobbyScreen * this, int __in_chrg) */

void __thiscall C_MP_LobbyScreen::~C_MP_LobbyScreen(C_MP_LobbyScreen *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x6ac910);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(extraout_ECX + 0x6accac);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(extraout_ECX + 0x6acd64);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(extraout_ECX + 0x6acda0);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(extraout_ECX + 0x6acdbc);
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
  return;
}


/* C_MP_LobbyScreen::~C_MP_LobbyScreen at 0051d110 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_MP_LobbyScreen": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: void ~C_MP_LobbyScreen(C_MP_LobbyScreen * this, int __in_chrg) */

void __thiscall C_MP_LobbyScreen::~C_MP_LobbyScreen(C_MP_LobbyScreen *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x6ac8b0);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(&UNK_006acc4c + extraout_ECX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(extraout_ECX + 0x6acd04);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(extraout_ECX + 0x6acd40);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(extraout_ECX + 0x6acd5c);
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
  return;
}


/* _GLOBAL__I__ZN16C_MP_LobbyScreen17m_pClassRecvTableE at 00074940 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN16C_MP_LobbyScreen17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

