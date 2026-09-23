/* DWARF-guided pseudocode for game/client/portal2/c_mp_lobby_screen.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* ClientClassInit<DT_MP_LobbyScreen::ignored> at 00074800 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

int ClientClassInit<DT_MP_LobbyScreen::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (_GLOBAL__I__Z14GetLocalPlayerv
      [(int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX] == (code)0x0
     ) {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc43344);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x51e].m_pPrev + unaff_EBX
                  ),(char *)(unaff_EBX + 0x8eb54c),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x522].
                               m_SerialNumber + unaff_EBX),(char *)(unaff_EBX + 0x8f012c),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00abc5d0 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00abc58c + unaff_EBX));
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x526].m_pEntity +
                   unaff_EBX),(char *)(unaff_EBX + 0x903bf0),0xa59,1);
      ___cxa_guard_release(unaff_EBX + 0xc43344);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0xaa8),
             (RecvProp_conflict1 *)
             ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x522].m_SerialNumber +
             unaff_EBX),2,(char *)(unaff_EBX + 0x91a538));
  return 1;
}


/* __static_initialization_and_destruction_0 at 00074950 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

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
  (&UNK_0000505f)[(int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
  _GLOBAL__I__Z35UpdateDirtySpatialPartitionEntitiesv
  [(int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = (code)0x0;
  (&UNK_0000505d)[(int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX] =
       0;
  (&UNK_0000505e)[(int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX] =
       0;
  *(undefined4 *)
   (&UNK_0000505f + (int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX)
       = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x506].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x506].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x506].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   (&UNK_00005073 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_00005077 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_0000507b + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_0000507f + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_0000507f + (int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX)
       = 0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x508].m_pEntity + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x508].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   (&UNK_0000508f + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       0x7f7fffff;
  *(undefined **)
   (&UNK_0000508f + (int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX)
       = &UNK_00abd4a7 + unaff_EBX;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x94b) = unaff_EBX + 0x91a3fd;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x943) = unaff_EBX + 0x4adebf;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0x947) = 0;
  this = (RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x95b);
  *(RecvTable **)((int)s_pParticlePtrs + unaff_EBX + 0x94f) = this;
  piVar1 = *(int **)(&DAT_00abc437 + unaff_EBX);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x953) = *piVar1;
  *piVar1 = unaff_EBX + 0xcda1a3;
  RecvTable::RecvTable(this);
  ___cxa_atexit(unaff_EBX + 0x8dc90f,0,*(undefined4 *)(&DAT_00abc3bf + unaff_EBX));
  iVar2 = ClientClassInit<DT_MP_LobbyScreen::ignored>((ignored *)0x0);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x96f) = iVar2;
  return;
}


/* C_MP_LobbyScreen::YouForgotToImplementOrDeclareClientClass at 00522620 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_MP_LobbyScreen": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_MP_LobbyScreen * this)
    */

int __thiscall C_MP_LobbyScreen::YouForgotToImplementOrDeclareClientClass(C_MP_LobbyScreen *this)

{
  return 0;
}


/* C_MP_LobbyScreen::GetClientClass at 00522640 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_MP_LobbyScreen": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: ClientClass * GetClientClass(C_MP_LobbyScreen * this) */

ClientClass * __thiscall C_MP_LobbyScreen::GetClientClass(C_MP_LobbyScreen *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x82c4bc);
}


/* _C_MP_LobbyScreen_CreateObject at 00522820 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_MP_LobbyScreen_CreateObject(int entnum,int serialNum)

{
  C_BaseEntity *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_MP_LobbyScreen * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xa5c);
  C_BaseEntity::C_BaseEntity(this);
  (this->super_IClientEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(&UNK_006734f4 + unaff_EBX);
  (this->super_IClientEntity).super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x67388c);
  (this->super_IClientEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x673944);
  (this->super_IClientEntity).super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(&UNK_00673980 + unaff_EBX);
  (**(code **)(&UNK_006735b0 + unaff_EBX))(this,entnum,serialNum);
  return &(this->super_IClientEntity).super_IClientNetworkable;
}


/* __tcf_0 at 00951270 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3fd8a4),in_stack_00000008);
  return;
}


/* C_MP_LobbyScreen::C_MP_LobbyScreen at 00522760 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(&UNK_006735b7 + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x67394f);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(&UNK_00673a07 + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(CViewRender::DisableFog + unaff_EBX + 3);
  return;
}


/* C_MP_LobbyScreen::C_MP_LobbyScreen at 005227c0 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(&UNK_00673557 + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6738ef);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(&UNK_006739a7 + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(&UNK_006739e3 + unaff_EBX);
  return;
}


/* C_MP_LobbyScreen::~C_MP_LobbyScreen at 00522650 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x6736c7);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x673a5f);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x673b17);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x673b53);
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,in_stack_ffffffe8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_MP_LobbyScreen::~C_MP_LobbyScreen at 005226c0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_MP_LobbyScreen": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: void ~C_MP_LobbyScreen(C_MP_LobbyScreen * this, int __in_chrg) */

void __thiscall C_MP_LobbyScreen::~C_MP_LobbyScreen(C_MP_LobbyScreen *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x673660);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(&UNK_006739f8 + extraout_ECX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(extraout_ECX + 0x673ab0);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(extraout_ECX + 0x673aec);
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
  return;
}


/* C_MP_LobbyScreen::~C_MP_LobbyScreen at 00522710 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_MP_LobbyScreen": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: void ~C_MP_LobbyScreen(C_MP_LobbyScreen * this, int __in_chrg) */

void __thiscall C_MP_LobbyScreen::~C_MP_LobbyScreen(C_MP_LobbyScreen *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x673610);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(&UNK_006739a8 + extraout_ECX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(extraout_ECX + 0x673a60);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(extraout_ECX + 0x673a9c);
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
  return;
}


/* _GLOBAL__I__ZN16C_MP_LobbyScreen17m_pClassRecvTableE at 00074a70 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN16C_MP_LobbyScreen17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

