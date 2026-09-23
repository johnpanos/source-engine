/* DWARF-guided pseudocode for game/client/portal2/c_level_placard_display.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* ClientClassInit<DT_LevelPlacardDisplay::ignored> at 00074570 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

int ClientClassInit<DT_LevelPlacardDisplay::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x537].m_pEntity + unaff_EBX)
      == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc434d4);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x537].m_pPrev + unaff_EBX
                  ),(char *)(unaff_EBX + 0x8eb7dc),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x53b].
                               m_SerialNumber + unaff_EBX),(char *)(unaff_EBX + 0x8f03bc),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00abc860 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00abc81c + unaff_EBX));
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x53f].m_pEntity +
                   unaff_EBX),(char *)(vgui::ScrollBarSlider::SetNobFocusColor + unaff_EBX),0xa59,1)
      ;
      ___cxa_guard_release(unaff_EBX + 0xc434d4);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0xd08),
             (RecvProp_conflict1 *)
             ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x53b].m_SerialNumber +
             unaff_EBX),2,(char *)(unaff_EBX + 0x91a784));
  return 1;
}


/* __static_initialization_and_destruction_0 at 000746c0 */

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
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x51e].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x51e].m_pNext + unaff_EBX)
       = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x51e].m_pNext + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x51e].m_pNext + unaff_EBX + 2) = 0;
  *(undefined4 *)
   (&UNK_000051f3 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_000051f7 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_000051fb + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       0x7f7fffff;
  *(undefined4 *)
   (&UNK_000051ff + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       0x7f7fffff;
  *(undefined4 *)
   (&UNK_000051ff + (int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX)
       = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x520].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x520].m_SerialNumber + unaff_EBX + 3) =
       0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x520].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x520].m_pNext + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x521].m_pEntity + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x521].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x521].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined **)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x521].m_pNext + unaff_EBX + 3) =
       &UNK_00abd737 + unaff_EBX;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0xbab) = unaff_EBX + 0x91a64e;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0xba3) = unaff_EBX + 0x4adeaf;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0xba7) = 0;
  this = (RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0xbbb);
  *(RecvTable **)((int)s_pParticlePtrs + unaff_EBX + 0xbaf) = this;
  piVar1 = *(int **)(&DAT_00abc6c7 + unaff_EBX);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0xbb3) = *piVar1;
  *piVar1 = unaff_EBX + 0xcda403;
  RecvTable::RecvTable(this);
  ___cxa_atexit(unaff_EBX + 0x8dcb7f,0,*(undefined4 *)(&DAT_00abc64f + unaff_EBX));
  iVar2 = ClientClassInit<DT_LevelPlacardDisplay::ignored>((ignored *)0x0);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0xbcf) = iVar2;
  return;
}


/* C_LevelPlacardDisplay::YouForgotToImplementOrDeclareClientClass at 00522380 */

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


/* C_LevelPlacardDisplay::GetClientClass at 005223a0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_LevelPlacardDisplay": ignoring overlapping field "m_bEnabled" */
/* DWARF original prototype: ClientClass * GetClientClass(C_LevelPlacardDisplay * this) */

ClientClass * __thiscall C_LevelPlacardDisplay::GetClientClass(C_LevelPlacardDisplay *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x82c72c);
}


/* _C_LevelPlacardDisplay_CreateObject at 00522580 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_LevelPlacardDisplay_CreateObject(int entnum,int serialNum)

{
  C_BaseEntity *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_LevelPlacardDisplay * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xa5c);
  C_BaseEntity::C_BaseEntity(this);
  (this->super_IClientEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x6732d4);
  (this->super_IClientEntity).super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x67366c);
  (this->super_IClientEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x673724);
  (this->super_IClientEntity).super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x673760);
  (**(code **)(unaff_EBX + 0x673390))(this,entnum,serialNum);
  return &(this->super_IClientEntity).super_IClientNetworkable;
}


/* __tcf_0 at 00951250 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3fd894),in_stack_00000008);
  return;
}


/* C_LevelPlacardDisplay::C_LevelPlacardDisplay at 005224c0 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x673397);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x67372f);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6737e7);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x673823);
  return;
}


/* C_LevelPlacardDisplay::C_LevelPlacardDisplay at 00522520 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x673337);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6736cf);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x673787);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6737c3);
  return;
}


/* C_LevelPlacardDisplay::~C_LevelPlacardDisplay at 005223b0 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(&UNK_006734a7 + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x67383f);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6738f7);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x673933);
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,in_stack_ffffffe8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_LevelPlacardDisplay::~C_LevelPlacardDisplay at 00522420 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x673440);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(extraout_ECX + 0x6737d8);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(extraout_ECX + 0x673890);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(extraout_ECX + 0x6738cc);
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
  return;
}


/* C_LevelPlacardDisplay::~C_LevelPlacardDisplay at 00522470 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(&UNK_006733f0 + extraout_ECX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(extraout_ECX + 0x673788);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(extraout_ECX + 0x673840);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(extraout_ECX + 0x67387c);
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
  return;
}


/* _GLOBAL__I__ZN21C_LevelPlacardDisplay17m_pClassRecvTableE at 000747e0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN21C_LevelPlacardDisplay17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

