/* DWARF-guided pseudocode for game/server/portal2/propglassfutbol.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 000c85f0 */

/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Enum "ButtonCode_t": Some values do not have unique names */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  undefined4 uVar1;
  IEntityFactoryDictionary *pIVar2;
  datamap_t *pdVar3;
  int unaff_EBX;
  longlong lVar4;
  ConVar *in_stack_ffffffe4;
  char *in_stack_ffffffe8;
  char *in_stack_ffffffec;
  int flags_1;
  
  flags_1 = unaff_EBX;
  lVar4 = ___i686_get_pc_thunk_bx();
  if (lVar4 == 0xffff00000001) {
    *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x31b4) = 0;
    *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x31b5) = 0;
    *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x31b6) = 0;
    *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x31b7) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x31b8) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x31bc) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x31c0) = 0x7f7fffff;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x31c4) = 0x7f7fffff;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x31c8) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x31cc) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x31d0) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x31d4) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x31d8) = 0x7f7fffff;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x31dc) = 0x7f7fffff;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x31e0) = 0x7f7fffff;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x31e4) = 0x7f7fffff;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x31e8) = 0;
    *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x31ef) = 0;
    *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x31ec) = 0;
    *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x31ed) = 0;
    *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x31ee) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x31f0) = 0;
    *(undefined1 *)((int)s_pRandomFloats + (int)(&UNK_000031f7 + unaff_EBX)) = 0;
    *(undefined1 *)((int)s_pRandomFloats + (int)(&UNK_000031f4 + unaff_EBX)) = 0;
    *(undefined1 *)((int)s_pRandomFloats + (int)(&UNK_000031f5 + unaff_EBX)) = 0;
    *(undefined1 *)((int)s_pRandomFloats + (int)(&UNK_000031f6 + unaff_EBX)) = 0;
    *(undefined4 *)((int)s_pRandomFloats + (int)(&UNK_000031f8 + unaff_EBX)) = 0;
    *(undefined1 *)((int)s_pRandomFloats + (int)(&UNK_000031ff + unaff_EBX)) = 1;
    *(undefined1 *)((int)s_pRandomFloats + (int)(&UNK_000031fc + unaff_EBX)) = 0;
    *(undefined1 *)((int)s_pRandomFloats + (int)(&UNK_000031fd + unaff_EBX)) = 0;
    *(undefined1 *)((int)s_pRandomFloats + (int)(&UNK_000031fe + unaff_EBX)) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3200) = 1;
    *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3207) = 0;
    *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3204) = 0;
    *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3205) = 0;
    *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x3206) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3208) = 2;
    *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x320f) = 0;
    *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x320c) = 0;
    *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x320d) = 0;
    *(undefined1 *)((int)s_pRandomFloats + unaff_EBX + 0x320e) = 0;
    *(undefined **)((int)s_pRandomFloats + (int)(&UNK_00003210 + unaff_EBX)) =
         &UNK_00ae8d24 + unaff_EBX;
    ConVar::ConVar((ConVar *)((int)&PTR_SetAssociatedControl_00c6723c + unaff_EBX),
                   (char *)(unaff_EBX + 0x948e46),(char *)(unaff_EBX + 0x8f2f5e),0,in_stack_ffffffe4
                   ,in_stack_ffffffe8,in_stack_ffffffec,flags_1);
    uVar1 = *(undefined4 *)(&DAT_00ae358c + unaff_EBX);
    ___cxa_atexit(unaff_EBX + 0x8d33dc,0,uVar1);
    ConVar::ConVar((ConVar *)((int)&PTR_IsDepressed_00c6729c + unaff_EBX),
                   (char *)(unaff_EBX + 0x948e5c),(char *)(unaff_EBX + 0x8eb438),0,in_stack_ffffffe4
                   ,in_stack_ffffffe8,in_stack_ffffffec,flags_1);
    ___cxa_atexit(unaff_EBX + 0x8d33bc,0,uVar1);
    ConVar::ConVar((ConVar *)((int)&PTR_GetButtonBgColor_00c672fc + unaff_EBX),
                   (char *)(unaff_EBX + 0x948e7d),&UNK_00933656 + unaff_EBX,0,in_stack_ffffffe4,
                   in_stack_ffffffe8,in_stack_ffffffec,flags_1);
    ___cxa_atexit(unaff_EBX + 0x8d339c,0,uVar1);
    ConVar::ConVar((ConVar *)((int)&PTR_SetColor_00c6735c + unaff_EBX),
                   (char *)(unaff_EBX + 0x948e9c),(char *)(unaff_EBX + 0x8eb438),0,in_stack_ffffffe4
                   ,in_stack_ffffffe8,in_stack_ffffffec,flags_1);
    ___cxa_atexit(unaff_EBX + 0x8d337c,0,uVar1);
    *(undefined **)((int)s_pRandomFloats + (int)(&UNK_00003214 + unaff_EBX)) =
         &UNK_00b70814 + unaff_EBX;
    pIVar2 = EntityFactoryDictionary();
    (**pIVar2->_vptr_IEntityFactoryDictionary)(pIVar2,unaff_EBX + 0xcf2354,unaff_EBX + 0x948ec5);
    pdVar3 = DataMapInit<CFutbolCatcher>((CFutbolCatcher *)0x0);
    *(datamap_t **)((int)&PTR_DrawSetTextPos_00c673b4 + unaff_EBX) = pdVar3;
    *(undefined **)((int)s_pRandomFloats + (int)(&UNK_00003218 + unaff_EBX)) =
         &UNK_00b70834 + unaff_EBX;
    pIVar2 = EntityFactoryDictionary();
    (**pIVar2->_vptr_IEntityFactoryDictionary)(pIVar2,unaff_EBX + 0xcf2358,unaff_EBX + 0x948ed4);
    pdVar3 = DataMapInit<CPropFutbolSpawner>((CPropFutbolSpawner *)0x0);
    *(datamap_t **)((int)&PTR_DrawPrintText_00c673b8 + unaff_EBX) = pdVar3;
    *(undefined **)((int)s_pRandomFloats + (int)(&UNK_0000321c + unaff_EBX)) =
         &UNK_00b70854 + unaff_EBX;
    pIVar2 = EntityFactoryDictionary();
    (**pIVar2->_vptr_IEntityFactoryDictionary)(pIVar2,unaff_EBX + 0xcf235c,unaff_EBX + 0x948d52);
    pdVar3 = DataMapInit<CPropGlassFutbol>((CPropGlassFutbol *)0x0);
    *(datamap_t **)((int)&PTR_DrawPrintText_00c673bc + unaff_EBX) = pdVar3;
    *(undefined **)((int)s_pRandomFloats + (int)(&UNK_00003220 + unaff_EBX)) =
         &UNK_00b70874 + unaff_EBX;
    pIVar2 = EntityFactoryDictionary();
    (**pIVar2->_vptr_IEntityFactoryDictionary)(pIVar2,unaff_EBX + 0xcf2360,unaff_EBX + 0x948eee);
  }
  return;
}


/* __tcf_3 at 0099b9e0 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x393e58),in_stack_00000008);
  return;
}


/* __tcf_4 at 0099b9c0 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x393ed8),in_stack_00000008);
  return;
}


/* __tcf_5 at 0099b9a0 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x393f58),in_stack_00000008);
  return;
}


/* __tcf_6 at 0099b980 */

void __tcf_6(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x393fd8),in_stack_00000008);
  return;
}


/* CFutbolCatcher::CatchThink at 006dbc40 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CatchThink(CFutbolCatcher * this) */

void __thiscall CFutbolCatcher::CatchThink(CFutbolCatcher *this)

{
  int iVar1;
  int unaff_EBX;
  Vector local_d0;
  Vector local_c4 [5];
  int local_84;
  float local_7c;
  float local_78;
  float local_74;
  float local_6c;
  float local_68;
  float local_64;
  float local_5c;
  float local_58;
  float local_54;
  float local_4c;
  float local_48;
  float local_44;
  undefined4 local_3c;
  undefined1 local_38;
  undefined1 local_37;
  int local_20 [4];
  
                    /* Unresolved local var: Ray_t ray@[???]
                       Unresolved local var: CFilterOnlyGlassFutbol filter@[???]
                       Unresolved local var: trace_t tr@[???] */
  ___i686_get_pc_thunk_bx();
  local_7c = (this->m_vecCatchBoxOrig).x;
  local_6c = local_7c - local_7c;
  local_78 = (this->m_vecCatchBoxOrig).y;
  local_68 = local_78 - local_78;
  local_74 = (this->m_vecCatchBoxOrig).z;
  local_64 = local_74 - local_74;
  local_3c = 0;
  local_37 = local_6c * local_6c + local_68 * local_68 + local_64 * local_64 !=
             *(float *)(&DAT_0034e91f + unaff_EBX);
  local_54 = *(float *)(unaff_EBX + 0x34e8af);
  local_4c = ((this->m_vecCatchBoxMaxs).x - (this->m_vecCatchBoxMins).x) * local_54;
  local_48 = ((this->m_vecCatchBoxMaxs).y - (this->m_vecCatchBoxMins).y) * local_54;
  local_44 = ((this->m_vecCatchBoxMaxs).z - (this->m_vecCatchBoxMins).z) * local_54;
  local_38 = (double)(local_4c * local_4c + local_48 * local_48 + local_44 * local_44) <
             *(double *)(unaff_EBX + 0x34ea9f);
  local_5c = ((this->m_vecCatchBoxMins).x + (this->m_vecCatchBoxMaxs).x) * local_54;
  local_58 = ((this->m_vecCatchBoxMins).y + (this->m_vecCatchBoxMaxs).y) * local_54;
  local_54 = ((this->m_vecCatchBoxMins).z + (this->m_vecCatchBoxMaxs).z) * local_54;
  local_7c = local_7c + local_5c;
  local_78 = local_78 + local_58;
  local_74 = local_74 + local_54;
  local_5c = -local_5c;
  local_58 = -local_58;
  local_54 = -local_54;
  local_20[0] = unaff_EBX + 0x55d247;
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x4cffbb) + 0x14))
            ((int *)**(undefined4 **)(unaff_EBX + 0x4cffbb),&local_7c,0x200400b,local_20,&local_d0);
  if (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x4cffb7) + 0x1c) + 0x30) != 0) {
    DebugDrawLine(&local_d0,local_c4,0xff,0,0,true,-1.0);
  }
                    /* Unresolved local var: CPropGlassFutbol * pFutbol@[???] */
  if ((local_84 == 0) ||
     ((((local_84 != **(int **)(unaff_EBX + 0x4d000b) &&
        (this->m_bDisableRecaptureOnPlayerGrab == false)) &&
       (iVar1 = ___dynamic_cast(local_84,*(undefined4 *)(unaff_EBX + 0x4cff77),unaff_EBX + 0x55d193,
                                0), iVar1 != 0)) &&
      (((*(int *)(iVar1 + 0x770) == 0 || (*(int *)(iVar1 + 0x770) == 1)) &&
       ((*(this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
          super_IHandleEntity._vptr_IHandleEntity[0xeb])(this,iVar1), local_84 == 0)))))) {
    this->m_bDisableRecaptureOnPlayerGrab = false;
  }
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(**(int **)(unaff_EBX + 0x4cff4f) + 0xc) +
             *(float *)(**(int **)(unaff_EBX + 0x4cff4f) + 0x1c),(char *)0x0);
  return;
}


/* CFilterOnlyGlassFutbol::ShouldHitEntity at 006dd040 */

/* DWARF original prototype: bool ShouldHitEntity(CFilterOnlyGlassFutbol * this, IHandleEntity *
   pServerEntity, int contentsMask) */

bool __thiscall
CFilterOnlyGlassFutbol::ShouldHitEntity
          (CFilterOnlyGlassFutbol *this,IHandleEntity *pServerEntity,int contentsMask)

{
  char cVar1;
  bool bVar2;
  CBaseEntity *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CBaseEntity * pEntity@[???] */
  ___i686_get_pc_thunk_bx();
  cVar1 = (**(code **)(*(int *)**(undefined4 **)(&DAT_004cebc3 + unaff_EBX) + 8))
                    ((int *)**(undefined4 **)(&DAT_004cebc3 + unaff_EBX),pServerEntity);
  if ((cVar1 == '\0') &&
     (this_00 = (CBaseEntity *)(*pServerEntity->_vptr_IHandleEntity[6])(pServerEntity),
     this_00 != (CBaseEntity *)0x0)) {
    if ((this_00->m_iClassname).pszValue == (char *)(unaff_EBX + 0x334309)) {
      return true;
    }
    bVar2 = CBaseEntity::ClassMatchesComplex(this_00,(char *)(unaff_EBX + 0x334309));
    return bVar2;
  }
  return false;
}


/* CFilterOnlyGlassFutbol::GetTraceType at 006dcd90 */

/* DWARF original prototype: TraceType_t GetTraceType(CFilterOnlyGlassFutbol * this) */

TraceType_t __thiscall CFilterOnlyGlassFutbol::GetTraceType(CFilterOnlyGlassFutbol *this)

{
  return TRACE_ENTITIES_ONLY;
}


/* CPropFutbolSocket::~CPropFutbolSocket at 006dcf00 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropFutbolSocket(CPropFutbolSocket * this, int __in_chrg) */

void __thiscall CPropFutbolSocket::~CPropFutbolSocket(CPropFutbolSocket *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x55bb17);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CFutbolCatcher).m_OnFutbolCaught.super_CBaseEntityOutput,
             in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CFutbolCatcher).m_OnFutbolReleased.super_CBaseEntityOutput,
             in_stack_ffffffe8);
  CBaseAnimating::~CBaseAnimating((CBaseAnimating *)this,__in_chrg);
  return;
}


/* CFutbolCatcher::~CFutbolCatcher at 006dd0c0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CFutbolCatcher(CFutbolCatcher * this, int __in_chrg) */

void __thiscall CFutbolCatcher::~CFutbolCatcher(CFutbolCatcher *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x55b957);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolCaught).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolReleased).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,__in_chrg);
  return;
}


/* CFutbolCatcher::~CFutbolCatcher at 006dd130 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CFutbolCatcher(CFutbolCatcher * this, int __in_chrg) */

void __thiscall CFutbolCatcher::~CFutbolCatcher(CFutbolCatcher *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(&UNK_0055b8e7 + unaff_EBX);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolCaught).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolReleased).super_CBaseEntityOutput,in_stack_ffffffe8);
  CBaseAnimating::~CBaseAnimating(&this->super_CBaseAnimating,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CPropFutbolSpawner::~CPropFutbolSpawner at 006dd310 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropFutbolSpawner(CPropFutbolSpawner * this, int __in_chrg) */

void __thiscall CPropFutbolSpawner::~CPropFutbolSpawner(CPropFutbolSpawner *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x55b324);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolGrabbed).super_CBaseEntityOutput,in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolSpawned).super_CBaseEntityOutput,in_stack_ffffffd8);
  (this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x55b704);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CFutbolCatcher).m_OnFutbolCaught.super_CBaseEntityOutput,
             in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CFutbolCatcher).m_OnFutbolReleased.super_CBaseEntityOutput,
             in_stack_ffffffd8);
  CBaseAnimating::~CBaseAnimating((CBaseAnimating *)this,in_stack_ffffffd8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CPropFutbolSpawner::~CPropFutbolSpawner at 006dd400 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropFutbolSpawner(CPropFutbolSpawner * this, int __in_chrg) */

void __thiscall CPropFutbolSpawner::~CPropFutbolSpawner(CPropFutbolSpawner *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x55b234);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolGrabbed).super_CBaseEntityOutput,in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnFutbolSpawned).super_CBaseEntityOutput,in_stack_ffffffd8);
  (this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x55b614);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CFutbolCatcher).m_OnFutbolCaught.super_CBaseEntityOutput,
             in_stack_ffffffd8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CFutbolCatcher).m_OnFutbolReleased.super_CBaseEntityOutput,
             in_stack_ffffffd8);
  CBaseAnimating::~CBaseAnimating((CBaseAnimating *)this,__in_chrg);
  return;
}


/* CPropFutbolSocket::~CPropFutbolSocket at 006dd4e0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropFutbolSocket(CPropFutbolSocket * this, int __in_chrg) */

void __thiscall CPropFutbolSocket::~CPropFutbolSocket(CPropFutbolSocket *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x55b537);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CFutbolCatcher).m_OnFutbolCaught.super_CBaseEntityOutput,
             in_stack_ffffffe8);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->super_CFutbolCatcher).m_OnFutbolReleased.super_CBaseEntityOutput,
             in_stack_ffffffe8);
  CBaseAnimating::~CBaseAnimating((CBaseAnimating *)this,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* DataMapInit<CFutbolCatcher> at 000c82f0 */

datamap_t * DataMapInit<CFutbolCatcher>(CFutbolCatcher *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  char *pcVar4;
  size_t sVar5;
  int iVar6;
  undefined4 *puVar7;
  int iVar8;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((*(char *)((int)s_pRandomFloats + (int)(&UNK_0000358a + unaff_EBX)) == '\0') &&
     (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xcf26ca), iVar3 != 0)) {
    *(int *)((int)s_pRandomFloats + (int)(&UNK_0000359a + unaff_EBX)) = unaff_EBX + 0x94913d;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x35a2) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x35a6) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x35aa) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x35ae) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x35b2) = 0;
    *(undefined4 *)((int)s_pRandomFloats + (int)(&UNK_0000359e + unaff_EBX)) = 0xe;
    ___cxa_guard_release(unaff_EBX + 0xcf26ca);
    ___cxa_atexit(unaff_EBX + 0x8d3702,0,*(undefined4 *)(&DAT_00ae3892 + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_CanPushEntity_00c05494 + unaff_EBX + 2) =
       *(undefined4 *)(&DAT_00ae389e + unaff_EBX);
  if ((*(char *)((int)s_pRandomFloats + (int)(&UNK_00003592 + unaff_EBX)) == '\0') &&
     (iVar3 = ___cxa_guard_acquire(unaff_EBX + 0xcf26d2), iVar3 != 0)) {
    uVar1 = **(undefined4 **)(&DAT_00ae38ae + unaff_EBX);
    *(undefined4 *)((int)&PTR_GetEnemy_00c059b4 + unaff_EBX + 2) = uVar1;
    *(undefined4 *)((int)&PTR_MakeTracer_00c059f4 + unaff_EBX + 2) = uVar1;
    pcVar4 = operator_new___(*(int *)((int)s_pRandomFloats + (int)(&UNK_0000359e + unaff_EBX)) + 0xb
                            );
    _strcpy(pcVar4,*(char **)((int)s_pRandomFloats + (int)(&UNK_0000359a + unaff_EBX)));
    sVar5 = _strlen(pcVar4);
    builtin_strncpy(pcVar4 + sVar5,"CatchThink",0xb);
    iVar2 = *(int *)((int)s_pRandomFloats + unaff_EBX + 0x35ae);
    iVar3 = iVar2 + 1;
    iVar8 = *(int *)((int)s_pRandomFloats + unaff_EBX + 0x35a6);
    if (iVar8 < iVar3) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)((int)s_pRandomFloats + unaff_EBX + 0x35a2),iVar3 - iVar8)
      ;
    }
    iVar6 = *(int *)((int)s_pRandomFloats + unaff_EBX + 0x35ae) + 1;
    *(int *)((int)s_pRandomFloats + unaff_EBX + 0x35ae) = iVar6;
    iVar8 = *(int *)((int)s_pRandomFloats + unaff_EBX + 0x35a2);
    *(int *)((int)s_pRandomFloats + unaff_EBX + 0x35b2) = iVar8;
    iVar6 = (iVar6 - iVar2) + -1;
    if (0 < iVar6) {
      _V_memmove((void *)(iVar8 + iVar3 * 4),(void *)(iVar8 + iVar2 * 4),iVar6 * 4);
      iVar8 = *(int *)((int)s_pRandomFloats + unaff_EBX + 0x35a2);
    }
    puVar7 = (undefined4 *)(iVar2 * 4 + iVar8);
    if (puVar7 != (undefined4 *)0x0) {
      *puVar7 = pcVar4;
    }
    *(char **)((int)&PTR_NetworkStateChanged_m_iHealth_00c05a24 + unaff_EBX + 2) = pcVar4;
    pcVar4 = operator_new___(*(int *)((int)s_pRandomFloats + (int)(&UNK_0000359e + unaff_EBX)) + 0xd
                            );
    _strcpy(pcVar4,*(char **)((int)s_pRandomFloats + (int)(&UNK_0000359a + unaff_EBX)));
    sVar5 = _strlen(pcVar4);
    builtin_strncpy(pcVar4 + sVar5,"CaptureThink",0xd);
    iVar2 = *(int *)((int)s_pRandomFloats + unaff_EBX + 0x35ae);
    iVar3 = iVar2 + 1;
    iVar8 = *(int *)((int)s_pRandomFloats + unaff_EBX + 0x35a6);
    if (iVar8 < iVar3) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)((int)s_pRandomFloats + unaff_EBX + 0x35a2),iVar3 - iVar8)
      ;
    }
    iVar6 = *(int *)((int)s_pRandomFloats + unaff_EBX + 0x35ae) + 1;
    *(int *)((int)s_pRandomFloats + unaff_EBX + 0x35ae) = iVar6;
    iVar8 = *(int *)((int)s_pRandomFloats + unaff_EBX + 0x35a2);
    *(int *)((int)s_pRandomFloats + unaff_EBX + 0x35b2) = iVar8;
    iVar6 = (iVar6 - iVar2) + -1;
    if (0 < iVar6) {
      _V_memmove((void *)(iVar8 + iVar3 * 4),(void *)(iVar8 + iVar2 * 4),iVar6 * 4);
      iVar8 = *(int *)((int)s_pRandomFloats + unaff_EBX + 0x35a2);
    }
    puVar7 = (undefined4 *)(iVar2 * 4 + iVar8);
    if (puVar7 != (undefined4 *)0x0) {
      *puVar7 = pcVar4;
    }
    *(char **)((int)&PTR_GetViewOffset_00c05a64 + unaff_EBX + 2) = pcVar4;
    ___cxa_guard_release(unaff_EBX + 0xcf26d2);
  }
  *(undefined4 *)((int)&PTR_VPhysicsGetObjectList_00c0548c + unaff_EBX + 2) = 9;
  *(int *)((int)&PTR_UpdatePhysicsShadowToCurrentPosition_00c05488 + unaff_EBX + 2) =
       unaff_EBX + 0xc05862;
  return (datamap_t *)((int)&PTR_UpdatePhysicsShadowToCurrentPosition_00c05488 + unaff_EBX + 2);
}


/* CFutbolCatcher::GetDataDescMap at 006db890 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CFutbolCatcher * this) */

datamap_t * __thiscall CFutbolCatcher::GetDataDescMap(CFutbolCatcher *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5f1ef0);
}


/* CFutbolCatcher::GetBaseMap at 006db8a0 */

datamap_t * CFutbolCatcher::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(CBaseSpriteProjectile::GetDataDescMap + extraout_ECX + 4);
}


/* __tcf_0 at 0099ba00 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41efde)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41efd2) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41efde));
  }
  *(undefined4 *)(unaff_EBX + 0x41efde) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41efda)) {
    if (*(int *)(unaff_EBX + 0x41efd2) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x21018a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x21018a),*(int *)(unaff_EBX + 0x41efd2));
      *(undefined4 *)(unaff_EBX + 0x41efd2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41efd6) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41efd2);
  *(int *)(unaff_EBX + 0x41efe2) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41efda)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x21018a) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x21018a),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41efd2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41efd6) = 0;
  }
  return;
}


/* DataMapInit<CPropFutbolSpawner> at 000c7ed0 */

datamap_t * DataMapInit<CPropFutbolSpawner>(CPropFutbolSpawner *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)s_pRandomFloats + unaff_EBX + 0x3974) == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcf2ab4);
    if (iVar2 != 0) {
      *(int *)((int)s_pRandomFloats + unaff_EBX + 0x3984) = unaff_EBX + 0x949533;
      *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x398c) = 0;
      *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3990) = 0;
      *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3994) = 0;
      *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3998) = 0;
      *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x399c) = 0;
      *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3988) = 0x12;
      ___cxa_guard_release(unaff_EBX + 0xcf2ab4);
      ___cxa_atexit(unaff_EBX + 0x8d3cdc,0,*(undefined4 *)(&DAT_00ae3cac + unaff_EBX));
    }
  }
  *(int *)((int)&PTR_AcceptInput_00c058c8 + unaff_EBX) =
       (int)&PTR_OnParseMapDataFinished_00c058a4 + unaff_EBX;
  if (*(char *)((int)s_pRandomFloats + unaff_EBX + 0x397c) == '\0') {
    iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcf2abc);
    if (iVar2 != 0) {
      uVar1 = **(undefined4 **)(&DAT_00ae3cc8 + unaff_EBX);
      *(undefined4 *)((int)&PTR_ShouldCollide_00c05b90 + unaff_EBX) = uVar1;
      *(undefined4 *)((int)&PTR_KeyValue_00c05bd0 + unaff_EBX) = uVar1;
      ___cxa_guard_release(unaff_EBX + 0xcf2abc);
    }
  }
  *(undefined4 *)((int)&PTR_SetParent_00c058c0 + unaff_EBX) = 4;
  *(undefined **)((int)&PTR_Activate_00c058bc + unaff_EBX) = &UNK_00c05b3c + unaff_EBX;
  return (datamap_t *)((int)&PTR_Activate_00c058bc + unaff_EBX);
}


/* CPropFutbolSpawner::GetDataDescMap at 006db8b0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPropFutbolSpawner * this) */

datamap_t * __thiscall CPropFutbolSpawner::GetDataDescMap(CPropFutbolSpawner *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5f1ee8);
}


/* CPropFutbolSpawner::GetBaseMap at 006db8c0 */

datamap_t * CPropFutbolSpawner::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5f1ec0);
}


/* __tcf_1 at 0099bbc0 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41edee)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41ede2) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41edee));
  }
  *(undefined4 *)(unaff_EBX + 0x41edee) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41edea)) {
    if (*(int *)(unaff_EBX + 0x41ede2) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20ffca) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20ffca),*(int *)(unaff_EBX + 0x41ede2));
      *(undefined4 *)(unaff_EBX + 0x41ede2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41ede6) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41ede2);
  *(int *)(unaff_EBX + 0x41edf2) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41edea)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x20ffca) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x20ffca),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41ede2) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41ede6) = 0;
  }
  return;
}


/* CPropGlassFutbol::~CPropGlassFutbol at 006dcf70 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void ~CPropGlassFutbol(CPropGlassFutbol * this, int __in_chrg) */

void __thiscall CPropGlassFutbol::~CPropGlassFutbol(CPropGlassFutbol *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
  super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x55b187);
  (this->super_CPhysicsProp).super_CBreakableProp.super_IBreakableWithPropData.
  _vptr_IBreakableWithPropData = (_func_int_varargs **)(unaff_EBX + 0x55b5ef);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x55b673);
  (this->super_CPhysicsProp).super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(unaff_EBX + 0x55b6a3);
  CPhysicsProp::~CPhysicsProp(&this->super_CPhysicsProp,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CPropGlassFutbol::~CPropGlassFutbol at 006dcfe0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPropGlassFutbol(CPropGlassFutbol * this, int __in_chrg) */

void __thiscall CPropGlassFutbol::~CPropGlassFutbol(CPropGlassFutbol *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
  super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(extraout_ECX + 0x55b120);
  (this->super_CPhysicsProp).super_CBreakableProp.super_IBreakableWithPropData.
  _vptr_IBreakableWithPropData = (_func_int_varargs **)(extraout_ECX + 0x55b588);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(extraout_ECX + 0x55b60c);
  (this->super_CPhysicsProp).super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(extraout_ECX + 0x55b63c);
  CPhysicsProp::~CPhysicsProp(&this->super_CPhysicsProp,__in_chrg);
  return;
}


/* CPropGlassFutbol::PreferredCarryAngles at 006dcdb0 */

/* DWARF original prototype: QAngle PreferredCarryAngles(CPropGlassFutbol * this) */

QAngle * __thiscall
CPropGlassFutbol::PreferredCarryAngles(QAngle *__return_storage_ptr__,CPropGlassFutbol *this)

{
  __return_storage_ptr__->x = 180.0;
  __return_storage_ptr__->y = -90.0;
  __return_storage_ptr__->z = 180.0;
  return __return_storage_ptr__;
}


/* CPropGlassFutbol::HasPreferredCarryAnglesForPlayer at 006dcdf0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: bool HasPreferredCarryAnglesForPlayer(CPropGlassFutbol * this,
   CBasePlayer * pPlayer) */

bool __thiscall
CPropGlassFutbol::HasPreferredCarryAnglesForPlayer(CPropGlassFutbol *this,CBasePlayer *pPlayer)

{
  return true;
}


/* DataMapInit<CPropGlassFutbol> at 000c8000 */

datamap_t * DataMapInit<CPropGlassFutbol>(CPropGlassFutbol *param_1)

{
  int iVar1;
  int iVar2;
  char *pcVar3;
  size_t sVar4;
  char *pcVar5;
  int iVar6;
  undefined4 *puVar7;
  int iVar8;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((*(char *)((int)s_pRandomFloats + unaff_EBX + 0x3814) == '\0') &&
     (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcf2954), iVar2 != 0)) {
    *(int *)((int)s_pRandomFloats + unaff_EBX + 0x3824) = unaff_EBX + 0x949416;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x382c) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3830) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3834) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3838) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x383c) = 0;
    *(undefined4 *)((int)s_pRandomFloats + unaff_EBX + 0x3828) = 0x10;
    ___cxa_guard_release(unaff_EBX + 0xcf2954);
    ___cxa_atexit(unaff_EBX + 0x8d3acc,0,*(undefined4 *)(&DAT_00ae3b7c + unaff_EBX));
  }
  *(undefined4 *)((int)&PTR_VPhysicsIsFlesh_00c057b0 + unaff_EBX) =
       *(undefined4 *)(&DAT_00ae459c + unaff_EBX);
  if ((*(char *)((int)s_pRandomFloats + unaff_EBX + 0x381c) == '\0') &&
     (iVar2 = ___cxa_guard_acquire(unaff_EBX + 0xcf295c), iVar2 != 0)) {
    pcVar3 = operator_new___(*(int *)((int)s_pRandomFloats + unaff_EBX + 0x3828) + 0xc);
    _strcpy(pcVar3,*(char **)((int)s_pRandomFloats + unaff_EBX + 0x3824));
    sVar4 = _strlen(pcVar3);
    pcVar5 = pcVar3 + sVar4;
    pcVar5[0] = 'T';
    pcVar5[1] = 'h';
    pcVar5[2] = 'r';
    pcVar5[3] = 'o';
    pcVar5[4] = 'w';
    pcVar5[5] = 'n';
    pcVar5[6] = 'T';
    pcVar5[7] = 'h';
    *(undefined **)(pcVar5 + 8) = &DAT_006b6e69;
    iVar1 = *(int *)((int)s_pRandomFloats + unaff_EBX + 0x3838);
    iVar2 = iVar1 + 1;
    iVar8 = *(int *)((int)s_pRandomFloats + unaff_EBX + 0x3830);
    if (iVar8 < iVar2) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)((int)s_pRandomFloats + unaff_EBX + 0x382c),iVar2 - iVar8)
      ;
    }
    iVar6 = *(int *)((int)s_pRandomFloats + unaff_EBX + 0x3838) + 1;
    *(int *)((int)s_pRandomFloats + unaff_EBX + 0x3838) = iVar6;
    iVar8 = *(int *)((int)s_pRandomFloats + unaff_EBX + 0x382c);
    *(int *)((int)s_pRandomFloats + unaff_EBX + 0x383c) = iVar8;
    iVar6 = (iVar6 - iVar1) + -1;
    if (0 < iVar6) {
      _V_memmove((void *)(iVar8 + iVar2 * 4),(void *)(iVar8 + iVar1 * 4),iVar6 * 4);
      iVar8 = *(int *)((int)s_pRandomFloats + unaff_EBX + 0x382c);
    }
    puVar7 = (undefined4 *)(iVar1 * 4 + iVar8);
    if (puVar7 != (undefined4 *)0x0) {
      *puVar7 = pcVar3;
    }
    *(char **)((int)&PTR_IsNPC_00c05950 + unaff_EBX) = pcVar3;
    pcVar3 = operator_new___(*(int *)((int)s_pRandomFloats + unaff_EBX + 0x3828) + 10);
    _strcpy(pcVar3,*(char **)((int)s_pRandomFloats + unaff_EBX + 0x3824));
    sVar4 = _strlen(pcVar3);
    builtin_strncpy(pcVar3 + sVar4,"AnimThink",10);
    iVar1 = *(int *)((int)s_pRandomFloats + unaff_EBX + 0x3838);
    iVar2 = iVar1 + 1;
    iVar8 = *(int *)((int)s_pRandomFloats + unaff_EBX + 0x3830);
    if (iVar8 < iVar2) {
      CUtlMemory<char*,int>::Grow
                ((CUtlMemory<char*,int> *)((int)s_pRandomFloats + unaff_EBX + 0x382c),iVar2 - iVar8)
      ;
    }
    iVar6 = *(int *)((int)s_pRandomFloats + unaff_EBX + 0x3838) + 1;
    *(int *)((int)s_pRandomFloats + unaff_EBX + 0x3838) = iVar6;
    iVar8 = *(int *)((int)s_pRandomFloats + unaff_EBX + 0x382c);
    *(int *)((int)s_pRandomFloats + unaff_EBX + 0x383c) = iVar8;
    iVar6 = (iVar6 - iVar1) + -1;
    if (0 < iVar6) {
      _V_memmove((void *)(iVar8 + iVar2 * 4),(void *)(iVar8 + iVar1 * 4),iVar6 * 4);
      iVar8 = *(int *)((int)s_pRandomFloats + unaff_EBX + 0x382c);
    }
    puVar7 = (undefined4 *)(iVar1 * 4 + iVar8);
    if (puVar7 != (undefined4 *)0x0) {
      *puVar7 = pcVar3;
    }
    *(char **)((int)&PTR_IsBaseCombatWeapon_00c05990 + unaff_EBX) = pcVar3;
    ___cxa_guard_release(unaff_EBX + 0xcf295c);
  }
  *(undefined4 *)((int)&PTR_UpdatePhysicsShadowToCurrentPosition_00c057a8 + unaff_EBX) = 7;
  *(int *)((int)&PTR_VPhysicsFriction_00c057a4 + unaff_EBX) =
       (int)&PTR_NetworkStateChanged_m_vecViewOffset_00c0580c + unaff_EBX;
  return (datamap_t *)((int)&PTR_VPhysicsFriction_00c057a4 + unaff_EBX);
}


/* CPropGlassFutbol::GetDataDescMap at 006db8d0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPropGlassFutbol * this) */

datamap_t * __thiscall CPropGlassFutbol::GetDataDescMap(CPropGlassFutbol *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x5f1ee0);
}


/* CPropGlassFutbol::GetBaseMap at 006db8e0 */

datamap_t * CPropGlassFutbol::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4d0cc8);
}


/* __tcf_2 at 0099bae0 */

void __tcf_2(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41ee9e)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41ee92) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41ee9e));
  }
  *(undefined4 *)(unaff_EBX + 0x41ee9e) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41ee9a)) {
    if (*(int *)(unaff_EBX + 0x41ee92) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2100aa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2100aa),*(int *)(unaff_EBX + 0x41ee92));
      *(undefined4 *)(unaff_EBX + 0x41ee92) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41ee96) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41ee92);
  *(int *)(unaff_EBX + 0x41eea2) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41ee9a)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2100aa) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2100aa),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41ee92) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41ee96) = 0;
  }
  return;
}


/* CFutbolCatcher::CFutbolCatcher at 006dcaa0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CFutbolCatcher(CFutbolCatcher * this) */

void __thiscall CFutbolCatcher::CFutbolCatcher(CFutbolCatcher *this)

{
  vec_t *pvVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::CBaseAnimating(&this->super_CBaseAnimating);
  (this->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x55bf7b);
  (this->m_hCaughtFutbol).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_vCatcherBoxHalfDiagonal).x = 25.0;
  (this->m_vCatcherBoxHalfDiagonal).y = 25.0;
  (this->m_vCatcherBoxHalfDiagonal).z = 25.0;
  pvVar1 = *(vec_t **)(CHLMachineGun::GetDataDescMap + unaff_EBX + 3);
  (this->m_vecCatchBoxMins).x = *pvVar1;
  (this->m_vecCatchBoxMins).y = pvVar1[1];
  (this->m_vecCatchBoxMins).z = pvVar1[2];
  (this->m_vecCatchBoxMaxs).x = *pvVar1;
  (this->m_vecCatchBoxMaxs).y = pvVar1[1];
  (this->m_vecCatchBoxMaxs).z = pvVar1[2];
  (this->m_vecCatchBoxOrig).x = *pvVar1;
  (this->m_vecCatchBoxOrig).y = pvVar1[1];
  (this->m_vecCatchBoxOrig).z = pvVar1[2];
  this->m_bDisableRecaptureOnPlayerGrab = false;
  (this->m_OnFutbolReleased).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnFutbolReleased).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->m_OnFutbolReleased).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_OnFutbolCaught).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnFutbolCaught).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->m_OnFutbolCaught).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  return;
}


/* CFutbolCatcher::CFutbolCatcher at 006dcca0 */

/* DWARF original prototype: void CFutbolCatcher(CFutbolCatcher * this, CFutbolCatcher * this) */

void __thiscall CFutbolCatcher::CFutbolCatcher(CFutbolCatcher *this,CFutbolCatcher *this_1)

{
  CFutbolCatcher(this);
  return;
}


/* CEntityFactory<CFutbolCatcher>::Create at 006dd200 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CFutbolCatcher> * this, char
   * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CFutbolCatcher>::Create(CEntityFactory<CFutbolCatcher> *this,char *pClassName)

{
  CFutbolCatcher *this_00;
  
                    /* Unresolved local var: CFutbolCatcher * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x57c);
  CFutbolCatcher::CFutbolCatcher(this_00);
  (*(this_00->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
    super_IHandleEntity._vptr_IHandleEntity[0x1d])(this_00,pClassName);
  return &(this_00->super_CBaseAnimating).super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CFutbolCatcher::Spawn at 006dba90 */

/* DWARF original prototype: void Spawn(CFutbolCatcher * this) */

void __thiscall CFutbolCatcher::Spawn(CFutbolCatcher *this)

{
  uint uVar1;
  vec_t vVar2;
  vec_t vVar3;
  vec_t vVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseAnimating::Spawn(&this->super_CBaseAnimating);
  CBaseAnimating::GetAttachment
            (&this->super_CBaseAnimating,(char *)(unaff_EBX + 0x3358e3),&this->m_vecCatchBoxOrig,
             (Vector *)0x0,(Vector *)0x0,(Vector *)0x0);
  uVar1 = *(uint *)(unaff_EBX + 0x357bb3);
  vVar2 = (this->m_vCatcherBoxHalfDiagonal).z;
  vVar3 = (this->m_vCatcherBoxHalfDiagonal).y;
  vVar4 = (this->m_vCatcherBoxHalfDiagonal).x;
  (this->m_vecCatchBoxMins).x = (vec_t)(uVar1 ^ (uint)vVar4);
  (this->m_vecCatchBoxMins).y = (vec_t)((uint)vVar3 ^ uVar1);
  (this->m_vecCatchBoxMins).z = (vec_t)((uint)vVar2 ^ uVar1);
  (this->m_vecCatchBoxMaxs).x = vVar4;
  (this->m_vecCatchBoxMaxs).y = vVar3;
  (this->m_vecCatchBoxMaxs).z = vVar2;
  CBaseEntity::ThinkSet((CBaseEntity *)this,(BASEPTR)0x3a5,0.0,(char *)0x0);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(**(int **)(unaff_EBX + 0x4d0103) + 0xc) +
             *(float *)(**(int **)(unaff_EBX + 0x4d0103) + 0x1c),(char *)0x0);
  return;
}


/* CFutbolCatcher::CaptureFutbol at 006dc8c0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CaptureFutbol(CFutbolCatcher * this, CPropGlassFutbol * pFutbol)
    */

void __thiscall CFutbolCatcher::CaptureFutbol(CFutbolCatcher *this,CPropGlassFutbol *pFutbol)

{
  IPhysicsObject *pIVar1;
  uint uVar2;
  ulong *puVar3;
  int iVar4;
  CBaseEntity *pCaller;
  int unaff_EBX;
  
                    /* Unresolved local var: IPhysicsObject * pPhysicsObject@[???] */
  ___i686_get_pc_thunk_bx();
  if (pFutbol != (CPropGlassFutbol *)0x0) {
    puVar3 = (ulong *)(**(code **)((int)(pFutbol->super_CPhysicsProp).super_CBreakableProp.
                                        super_CBaseProp.super_CBaseAnimating.super_CBaseEntity.
                                        super_IServerEntity.super_IServerUnknown + 0xc))(pFutbol);
    (this->m_hCaughtFutbol).super_CBaseHandle.m_Index = *puVar3;
    pIVar1 = (pFutbol->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating
             .super_CBaseEntity.m_pPhysicsObject;
    if (pIVar1 != (IPhysicsObject *)0x0) {
      (*pIVar1->_vptr_IPhysicsObject[0x10])(pIVar1,0);
    }
    (**(code **)((int)(pFutbol->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                      super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
                      super_IServerUnknown + 0x1c4))
              (pFutbol,&this->m_vecCatchBoxOrig,*(undefined4 *)(unaff_EBX + 0x4cf370),
               *(undefined4 *)(unaff_EBX + 0x4cf2dc));
    CBaseEntity::ThinkSet((CBaseEntity *)this,(BASEPTR)0x3a9,0.0,(char *)0x0);
    pFutbol->m_Holder = FUTBOL_HELD_BY_CATCHER;
    uVar2 = (pFutbol->m_hLastHeldByPlayer).super_CBaseHandle.m_Index;
    if ((uVar2 == 0xffffffff) ||
       (iVar4 = (uVar2 & 0xffff) * 0x10 + **(int **)(CHLMachineGun::WeaponSoundRealtime + unaff_EBX)
       , *(uint *)(iVar4 + 8) != uVar2 >> 0x10)) {
      pCaller = (CBaseEntity *)0x0;
    }
    else {
      pCaller = *(CBaseEntity **)(iVar4 + 4);
    }
    COutputEvent::FireOutput(&this->m_OnFutbolCaught,(CBaseEntity *)pFutbol,pCaller,0.0);
  }
  return;
}


/* CFutbolCatcher::CaptureThink at 006dccb0 */

/* DWARF original prototype: void CaptureThink(CFutbolCatcher * this) */

void __thiscall CFutbolCatcher::CaptureThink(CFutbolCatcher *this)

{
  uint uVar1;
  int iVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: CPropGlassFutbol * pFutbol@[???] */
  ___i686_get_pc_thunk_bx();
  uVar1 = (this->m_hCaughtFutbol).super_CBaseHandle.m_Index;
  if ((((uVar1 == 0xffffffff) ||
       (iVar2 = (uVar1 & 0xffff) * 0x10 +
                **(int **)(CEntityFactory<AR2Explosion>::Destroy + unaff_EBX),
       *(uint *)(iVar2 + 8) != uVar1 >> 0x10)) || (iVar2 = *(int *)(iVar2 + 4), iVar2 == 0)) ||
     (*(int *)(iVar2 + 0x770) != 3)) {
    CBaseEntity::ThinkSet((CBaseEntity *)this,(BASEPTR)0x3a5,0.0,(char *)0x0);
    COutputEvent::FireOutput(&this->m_OnFutbolReleased,(CBaseEntity *)this,(CBaseEntity *)this,0.0);
    (this->m_hCaughtFutbol).super_CBaseHandle.m_Index = 0xffffffff;
    this->m_bDisableRecaptureOnPlayerGrab = true;
  }
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(**(int **)(&DAT_004ceedc + unaff_EBX) + 0xc) +
             *(float *)(**(int **)(&DAT_004ceedc + unaff_EBX) + 0x1c),(char *)0x0);
  return;
}


/* CPropGlassFutbol::CPropGlassFutbol at 006dc9e0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CPropGlassFutbol(CPropGlassFutbol * this) */

void __thiscall CPropGlassFutbol::CPropGlassFutbol(CPropGlassFutbol *this)

{
  vec_t *pvVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CPhysicsProp::CPhysicsProp(&this->super_CPhysicsProp);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
  super_CBaseEntity.super_IServerEntity.super_IServerUnknown =
       (IServerUnknown)(unaff_EBX + 0x55b71b);
  (this->super_CPhysicsProp).super_CBreakableProp.super_IBreakableWithPropData.
  _vptr_IBreakableWithPropData = (_func_int_varargs **)(&UNK_0055bb83 + unaff_EBX);
  (this->super_CPhysicsProp).super_CBreakableProp.super_CDefaultPlayerPickupVPhysics.
  super_IPlayerPickupVPhysics._vptr_IPlayerPickupVPhysics =
       (_func_int_varargs **)(unaff_EBX + 0x55bc07);
  (this->super_CPhysicsProp).super_INavAvoidanceObstacle._vptr_INavAvoidanceObstacle =
       (_func_int_varargs **)(unaff_EBX + 0x55bc37);
  (this->m_hSpawner).super_CBaseHandle.m_Index = 0xffffffff;
  (this->m_strSpawnerName).pszValue = (char *)0x0;
  this->m_Holder = FUTBOL_HELD_BY_NONE;
  pvVar1 = *(vec_t **)(unaff_EBX + 0x4cf1c3);
  (this->m_vecThrowDirection).x = *pvVar1;
  (this->m_vecThrowDirection).y = pvVar1[1];
  (this->m_vecThrowDirection).z = pvVar1[2];
  (this->m_hLastHeldByPlayer).super_CBaseHandle.m_Index = 0xffffffff;
  return;
}


/* CPropGlassFutbol::CPropGlassFutbol at 006dca90 */

/* DWARF original prototype: void CPropGlassFutbol(CPropGlassFutbol * this, CPropGlassFutbol * this)
    */

void __thiscall CPropGlassFutbol::CPropGlassFutbol(CPropGlassFutbol *this,CPropGlassFutbol *this_1)

{
  CPropGlassFutbol(this);
  return;
}


/* CEntityFactory<CPropGlassFutbol>::Create at 006dd1b0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CPropGlassFutbol> * this,
   char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CPropGlassFutbol>::Create(CEntityFactory<CPropGlassFutbol> *this,char *pClassName)

{
  CPropGlassFutbol *this_00;
  
                    /* Unresolved local var: CPropGlassFutbol * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x784);
  CPropGlassFutbol::CPropGlassFutbol(this_00);
  (**(code **)((int)(this_00->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x74))(this_00,pClassName);
  return &(this_00->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
          super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CPropGlassFutbol::Precache at 006dba60 */

/* DWARF original prototype: void Precache(CPropGlassFutbol * this) */

void __thiscall CPropGlassFutbol::Precache(CPropGlassFutbol *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CPhysicsProp::Precache(&this->super_CPhysicsProp);
  CBaseEntity::PrecacheModel((char *)(unaff_EBX + 0x3358fc));
  return;
}


/* CPropGlassFutbol::Spawn at 006dc6a0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CPropGlassFutbol * this) */

void __thiscall CPropGlassFutbol::Spawn(CPropGlassFutbol *this)

{
  uint *puVar1;
  uint uVar2;
  CBaseEdict *this_00;
  BASEPTR func;
  int iVar3;
  CBaseEntity *pCVar4;
  int *piVar5;
  ulong *puVar6;
  IChangeInfoAccessor *pIVar7;
  char *pcVar8;
  int unaff_EBX;
  uint uVar9;
  
  ___i686_get_pc_thunk_bx();
  (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x68))(this);
  (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x80))(this,&UNK_00328bfb + unaff_EBX,unaff_EBX + 0x334cbb);
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
  uVar2 = (this->m_hSpawner).super_CBaseHandle.m_Index;
  if (((uVar2 == 0xffffffff) ||
      (iVar3 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4cf4f7),
      *(uint *)(iVar3 + 8) != uVar2 >> 0x10)) || (*(int *)(iVar3 + 4) == 0)) {
    pcVar8 = (this->m_strSpawnerName).pszValue;
    if ((pcVar8 == (char *)0x0) ||
       (pCVar4 = CGlobalEntityList::FindEntityByName
                           (*(CGlobalEntityList **)(unaff_EBX + 0x4cf5b3),(CBaseEntity *)0x0,pcVar8,
                            (CBaseEntity *)0x0,(CBaseEntity *)0x0,(CBaseEntity *)0x0,
                            (IEntityFindFilter *)0x0), pCVar4 == (CBaseEntity *)0x0)) {
      _Warning(unaff_EBX + 0x334d2b);
    }
    else {
      piVar5 = (int *)___dynamic_cast(pCVar4,*(undefined4 *)(unaff_EBX + 0x4cf51b),
                                      unaff_EBX + 0x55c743,0);
      if (piVar5 == (int *)0x0) {
        pcVar8 = CBaseEntity::GetDebugName((CBaseEntity *)0x0);
        iVar3 = section_00000054.segname._4_4_;
        if (section_00000054.segname._4_4_ == 0) {
          iVar3 = unaff_EBX + 0x2cf4af;
        }
        _Warning(unaff_EBX + 0x334cdb,iVar3,pcVar8);
      }
      else {
        puVar6 = (ulong *)(**(code **)(*piVar5 + 0xc))(piVar5);
        (this->m_hSpawner).super_CBaseHandle.m_Index = *puVar6;
      }
    }
  }
  CPhysicsProp::Spawn(&this->super_CPhysicsProp);
  iVar3 = CBaseAnimating::LookupSequence((CBaseAnimating *)this,(char *)(unaff_EBX + 0x335313));
  CBaseAnimating::ResetSequence((CBaseAnimating *)this,iVar3);
  func.__delta = 0;
  func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0xcdd);
  CBaseEntity::ThinkSet
            ((CBaseEntity *)this,func,
             *(float *)(unaff_EBX + 0x34dc67) + *(float *)(**(int **)(unaff_EBX + 0x4cf4f3) + 0xc),
             *(char **)(unaff_EBX + 0x5f10d3));
  uVar2 = (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
          super_CBaseEntity.m_spawnflags.m_Value;
  uVar9 = uVar2 | 0x140;
  if (uVar2 != uVar9) {
    if (*(bool *)((int)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent + 0x10) ==
        false) {
      this_00 = &((this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                  super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if (this_00 != (CBaseEdict *)0x0) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x101;
        pIVar7 = CBaseEdict::GetChangeAccessor(this_00);
        pIVar7->m_iChangeInfoSerialNumber = 0;
      }
    }
    else {
      puVar1 = (uint *)&(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                        super_CBaseAnimating.super_CBaseEntity.m_Network.field_0x4c;
      *puVar1 = *puVar1 | 1;
    }
    (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
    super_CBaseEntity.m_spawnflags.m_Value = uVar9;
  }
  return;
}


/* CPropGlassFutbol::Event_Killed at 006dc2a0 */

/* DWARF original prototype: void Event_Killed(CPropGlassFutbol * this, CTakeDamageInfo * info) */

void __thiscall CPropGlassFutbol::Event_Killed(CPropGlassFutbol *this,CTakeDamageInfo *info)

{
  uint uVar1;
  int *piVar2;
  CBaseEntity *this_00;
  bool bVar3;
  int iVar4;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  uVar1 = (info->m_hAttacker).super_CBaseHandle.m_Index;
  if (((uVar1 == 0xffffffff) ||
      (iVar4 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4cf8f0),
      *(uint *)(iVar4 + 8) != uVar1 >> 0x10)) ||
     (this_00 = *(CBaseEntity **)(iVar4 + 4), this_00 == (CBaseEntity *)0x0)) {
LAB_006dc2da:
    CBreakableProp::Event_Killed((CBreakableProp *)this,info);
  }
  else {
    if ((this_00->m_iClassname).pszValue != (char *)(unaff_EBX + 0x2de7be)) {
      bVar3 = CBaseEntity::ClassMatchesComplex(this_00,(char *)(unaff_EBX + 0x2de7be));
      if (!bVar3) goto LAB_006dc2da;
    }
    CBaseEntity::Event_Killed((CBaseEntity *)this,info);
  }
  uVar1 = (this->m_hSpawner).super_CBaseHandle.m_Index;
  if (((uVar1 != 0xffffffff) &&
      (iVar4 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4cf8f0),
      *(uint *)(iVar4 + 8) == uVar1 >> 0x10)) &&
     (piVar2 = *(int **)(iVar4 + 4), piVar2 != (int *)0x0)) {
    (**(code **)(*piVar2 + 0x3b0))(piVar2);
  }
  return;
}


/* CPropGlassFutbol::OnPhysGunPickup at 006dc540 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void OnPhysGunPickup(CPropGlassFutbol * this, CBasePlayer *
   pPhysGunUser, PhysGunPickup_t reason) */

void __thiscall
CPropGlassFutbol::OnPhysGunPickup
          (CPropGlassFutbol *this,CBasePlayer *pPhysGunUser,PhysGunPickup_t reason)

{
  IPhysicsObject *pIVar1;
  uint uVar2;
  int *piVar3;
  CPortal_Player *this_00;
  ulong *puVar4;
  int iVar5;
  CBasePlayer *pCVar6;
  int unaff_EBX;
  
                    /* Unresolved local var: CPortal_Player * pHoldingPlayer@[???]
                       Unresolved local var: IPhysicsObject * pPhysicsObject@[???] */
  ___i686_get_pc_thunk_bx();
  if (pPhysGunUser == (CBasePlayer *)0x0) {
    this_00 = (CPortal_Player *)0x0;
  }
  else {
    this_00 = (CPortal_Player *)
              ___dynamic_cast(pPhysGunUser,*(undefined4 *)(unaff_EBX + 0x4cf700),
                              *(undefined4 *)(&DAT_004cf874 + unaff_EBX),0);
    if (this_00 != (CPortal_Player *)0x0) {
      puVar4 = (ulong *)(**(code **)(*(int *)&(this_00->
                                              super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>
                                              ).super_CPaintableEntity<CBaseMultiplayerPlayer>.
                                              super_CBaseMultiplayerPlayer.
                                              super_CAI_ExpresserHost<CBasePlayer>.super_CBasePlayer
                                              .super_CBaseCombatCharacter.super_CBaseFlex + 0xc))
                                  (this_00);
      (this->m_hLastHeldByPlayer).super_CBaseHandle.m_Index = *puVar4;
      goto LAB_006dc5ad;
    }
  }
  (this->m_hLastHeldByPlayer).super_CBaseHandle.m_Index = 0xffffffff;
LAB_006dc5ad:
  pIVar1 = (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
           super_CBaseEntity.m_pPhysicsObject;
  if (pIVar1 != (IPhysicsObject *)0x0) {
    (*pIVar1->_vptr_IPhysicsObject[0x10])(pIVar1,1);
  }
  if (((reason == PICKED_UP_BY_PLAYER) && (this_00 != (CPortal_Player *)0x0)) &&
     (*(int *)(*(int *)(unaff_EBX + 0x653428) + 0x30) != 0)) {
    CPortal_Player::SetUseKeyCooldownTime(this_00,*(float *)(*(int *)(unaff_EBX + 0x6533c8) + 0x2c))
    ;
  }
  uVar2 = (this->m_hSpawner).super_CBaseHandle.m_Index;
  if (((uVar2 != 0xffffffff) &&
      (iVar5 = (uVar2 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4cf650),
      *(uint *)(iVar5 + 8) == uVar2 >> 0x10)) &&
     (piVar3 = *(int **)(iVar5 + 4), piVar3 != (int *)0x0)) {
    (**(code **)(*piVar3 + 0x3b4))(piVar3);
  }
  if (this->m_Holder == FUTBOL_HELD_BY_PLAYER) {
                    /* Unresolved local var: CBasePlayer * pOtherPlayer@[???] */
    pCVar6 = GetPlayerHoldingEntity((CBaseEntity *)this);
    if (pCVar6 != (CBasePlayer *)0x0) {
      (**(code **)((int)(pCVar6->super_CBaseCombatCharacter).super_CBaseFlex.
                        super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                        super_IServerEntity.super_IServerUnknown + 0x6e0))(pCVar6,0);
    }
  }
  this->m_Holder = FUTBOL_HELD_BY_PLAYER;
  return;
}


/* CPropGlassFutbol::OnPhysGunDrop at 006dc390 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void OnPhysGunDrop(CPropGlassFutbol * this, CBasePlayer * pPhysGunUser,
   PhysGunDrop_t reason) */

void __thiscall
CPropGlassFutbol::OnPhysGunDrop
          (CPropGlassFutbol *this,CBasePlayer *pPhysGunUser,PhysGunDrop_t reason)

{
  uint uVar1;
  VMatrix matThisToLinked;
  BASEPTR func;
  char cVar2;
  char cVar3;
  int iVar4;
  int iVar5;
  int unaff_EBX;
  undefined4 *puVar6;
  undefined4 *puVar7;
  CBasePlayer *pCVar8;
  Vector *pVVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 in_stack_ffffff94;
  undefined1 in_stack_ffffff98 [44];
  Vector *pForward;
  Vector *vTransformed;
  Vector local_28 [2];
  
                    /* Unresolved local var: Vector forward@[???] */
  ___i686_get_pc_thunk_bx();
  if (pPhysGunUser != (CBasePlayer *)0x0) {
    CPhysicsProp::OnPhysGunDrop(&this->super_CPhysicsProp,pPhysGunUser,reason);
    uVar11 = 0;
    uVar10 = 0;
    pForward = local_28;
    pVVar9 = pForward;
    CBasePlayer::EyeVectors(pPhysGunUser,pForward,(Vector *)0x0,(Vector *)0x0);
    cVar2 = (**(code **)((int)(pPhysGunUser->super_CBaseCombatCharacter).super_CBaseFlex.
                              super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                              super_IServerEntity.super_IServerUnknown + 0x154))();
    cVar3 = cRam0000160c;
    if (cVar2 != '\0') {
      uVar11 = 0;
      uVar10 = *(undefined4 *)(unaff_EBX + 0x4cfa2a);
      pVVar9 = *(Vector **)(unaff_EBX + 0x4cf82a);
      iVar5 = ___dynamic_cast();
      cVar3 = *(char *)(iVar5 + 0x160c);
    }
    if (cVar3 != '\0') {
                    /* Unresolved local var: CPortal_Base2D * pPortal@[???] */
      pCVar8 = pPhysGunUser;
      cVar3 = (**(code **)((int)(pPhysGunUser->super_CBaseCombatCharacter).super_CBaseFlex.
                                super_CBaseAnimatingOverlay.super_CBaseAnimating.super_CBaseEntity.
                                super_IServerEntity.super_IServerUnknown + 0x154))();
      if (cVar3 == '\0') {
        iVar5 = 0;
        pPhysGunUser = pCVar8;
      }
      else {
        uVar11 = 0;
        uVar10 = *(undefined4 *)(unaff_EBX + 0x4cfa2a);
        pVVar9 = *(Vector **)(unaff_EBX + 0x4cf82a);
        iVar5 = ___dynamic_cast();
      }
      uVar1 = *(uint *)(iVar5 + 0x1610);
      if ((uVar1 == 0xffffffff) ||
         (iVar5 = (uVar1 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x4cf806),
         *(uint *)(iVar5 + 8) != uVar1 >> 0x10)) {
        iVar5 = 0;
      }
      else {
        iVar5 = *(int *)(iVar5 + 4);
      }
      puVar6 = (undefined4 *)(iVar5 + 0x528);
      puVar7 = (undefined4 *)&stack0xffffff84;
      vTransformed = pForward;
      for (iVar4 = 0x10; iVar4 != 0; iVar4 = iVar4 + -1) {
        *puVar7 = *puVar6;
        puVar6 = puVar6 + 1;
        puVar7 = puVar7 + 1;
      }
      matThisToLinked.m[0][1] = (vec_t)pVVar9;
      matThisToLinked.m[0][0] = (vec_t)pPhysGunUser;
      matThisToLinked.m[0][2] = (vec_t)uVar10;
      matThisToLinked.m[0][3] = (vec_t)uVar11;
      matThisToLinked.m[1][0] = (vec_t)in_stack_ffffff94;
      matThisToLinked.m[1][1] = (vec_t)in_stack_ffffff98._0_4_;
      matThisToLinked.m[1][2] = (vec_t)in_stack_ffffff98._4_4_;
      matThisToLinked.m[1][3] = (vec_t)in_stack_ffffff98._8_4_;
      matThisToLinked.m[2][0] = (vec_t)in_stack_ffffff98._12_4_;
      matThisToLinked.m[2][1] = (vec_t)in_stack_ffffff98._16_4_;
      matThisToLinked.m[2][2] = (vec_t)in_stack_ffffff98._20_4_;
      matThisToLinked.m[2][3] = (vec_t)in_stack_ffffff98._24_4_;
      matThisToLinked.m[3][0] = (vec_t)in_stack_ffffff98._28_4_;
      matThisToLinked.m[3][1] = (vec_t)in_stack_ffffff98._32_4_;
      matThisToLinked.m[3][2] = (vec_t)in_stack_ffffff98._36_4_;
      matThisToLinked.m[3][3] = (vec_t)in_stack_ffffff98._40_4_;
      UTIL_Portal_VectorTransform(matThisToLinked,pForward,vTransformed);
    }
    (this->m_vecThrowDirection).x = local_28[0].x;
    (this->m_vecThrowDirection).y = local_28[0].y;
    (this->m_vecThrowDirection).z = local_28[0].z;
    func.__delta = 0;
    func.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)(unaff_EBX - 0x38e);
    CBaseEntity::ThinkSet
              ((CBaseEntity *)this,func,
               *(float *)(**(int **)(unaff_EBX + 0x4cf802) + 0xc) +
               *(float *)(**(int **)(unaff_EBX + 0x4cf802) + 0x1c),*(char **)(unaff_EBX + 0x5f13e6))
    ;
    this->m_Holder = FUTBOL_HELD_BY_NONE;
  }
  return;
}


/* CPropGlassFutbol::SetSpawner at 006dc1f0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void SetSpawner(CPropGlassFutbol * this, CPropFutbolSpawner *
   pMySpawner) */

void __thiscall CPropGlassFutbol::SetSpawner(CPropGlassFutbol *this,CPropFutbolSpawner *pMySpawner)

{
  ulong *puVar1;
  
  if (pMySpawner != (CPropFutbolSpawner *)0x0) {
    puVar1 = (ulong *)(*(pMySpawner->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.
                        super_IServerEntity.super_IServerUnknown.super_IHandleEntity.
                        _vptr_IHandleEntity[3])(pMySpawner);
    (this->m_hSpawner).super_CBaseHandle.m_Index = *puVar1;
    return;
  }
  (this->m_hSpawner).super_CBaseHandle.m_Index = 0xffffffff;
  return;
}


/* CPropGlassFutbol::OnTakeDamage at 006dba30 */

/* DWARF original prototype: int OnTakeDamage(CPropGlassFutbol * this, CTakeDamageInfo * info) */

int __thiscall CPropGlassFutbol::OnTakeDamage(CPropGlassFutbol *this,CTakeDamageInfo *info)

{
  int iVar1;
  
  if (1 < this->m_Holder - FUTBOL_HELD_BY_SPAWNER) {
    iVar1 = CPhysicsProp::OnTakeDamage(&this->super_CPhysicsProp,info);
    return iVar1;
  }
  return 0;
}


/* CPropGlassFutbol::VPhysicsCollision at 006dbf00 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void VPhysicsCollision(CPropGlassFutbol * this, int index,
   gamevcollisionevent_t * pEvent) */

void __thiscall
CPropGlassFutbol::VPhysicsCollision(CPropGlassFutbol *this,int index,gamevcollisionevent_t *pEvent)

{
  int *piVar1;
  char cVar2;
  CPortal_Player *this_00;
  CBaseEntity *pCVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if (*(int *)(*(int *)(unaff_EBX + 0x6539a8) + 0x30) != 0) {
                    /* Unresolved local var: CBaseEntity * pHitEntity@[???] */
    piVar1 = *(int **)((int)pEvent->pEntities + (-(uint)(index == 0) & 4));
    cVar2 = (**(code **)(*piVar1 + 0x154))(piVar1);
    if (cVar2 != '\0') {
                    /* Unresolved local var: CPortal_Player * pPlayer@[???] */
      this_00 = (CPortal_Player *)
                ___dynamic_cast(piVar1,*(undefined4 *)(unaff_EBX + 0x4cfcb4),
                                *(undefined4 *)(CHLSelectFireMachineGun::BurstThink + unaff_EBX + 4)
                                ,0);
      if (this_00 != (CPortal_Player *)0x0) {
        pCVar3 = GetPlayerHeldEntity((CBasePlayer *)this_00);
        if ((pCVar3 == (CBaseEntity *)0x0) && (this->m_Holder == FUTBOL_HELD_BY_NONE)) {
          (**(code **)(*(int *)&(this_00->
                                super_PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer>_>).
                                super_CPaintableEntity<CBaseMultiplayerPlayer>.
                                super_CBaseMultiplayerPlayer.super_CAI_ExpresserHost<CBasePlayer>.
                                super_CBasePlayer.super_CBaseCombatCharacter.super_CBaseFlex + 0x6dc
                      ))(this_00,this,1);
          CPortal_Player::SetUseKeyCooldownTime
                    (this_00,(float)(*(int *)(*(int *)(unaff_EBX + 0x653a08) + 0x30) != 0));
        }
      }
    }
  }
  CPhysicsProp::VPhysicsCollision(&this->super_CPhysicsProp,index,pEvent);
  return;
}


/* CPropGlassFutbol::SetHolder at 006db8f0 */

/* DWARF original prototype: void SetHolder(CPropGlassFutbol * this, futbol_holder_type_t type) */

void __thiscall CPropGlassFutbol::SetHolder(CPropGlassFutbol *this,futbol_holder_type_t type)

{
  this->m_Holder = type;
  return;
}


/* CPropFutbolSpawner::SpawnFutbol at 006dc100 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void SpawnFutbol(CPropFutbolSpawner * this) */

void __thiscall CPropFutbolSpawner::SpawnFutbol(CPropFutbolSpawner *this)

{
  CBaseEntity *this_00;
  int unaff_EBX;
  Vector local_28 [2];
  
                    /* Unresolved local var: CPropGlassFutbol * pBall@[???]
                       Unresolved local var: Vector vecBallSpawnPoint@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CreateEntityByName(&UNK_00335242 + unaff_EBX,-1,true);
  if (this_00 != (CBaseEntity *)0x0) {
    COutputEvent::FireOutput(&this->m_OnFutbolSpawned,(CBaseEntity *)this,(CBaseEntity *)this,0.0);
    CBaseAnimating::GetAttachment
              ((CBaseAnimating *)this,&UNK_0033526c + unaff_EBX,local_28,(Vector *)0x0,(Vector *)0x0
               ,(Vector *)0x0);
    CBaseEntity::SetAbsOrigin(this_00,local_28);
    (*(this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
      [0x117])(this_00,this);
    this_00[1].m_ScriptScope.m_FuncHandles.
    super_CUtlVector<HSCRIPT__**,CUtlMemoryConservative<HSCRIPT__**>_>.m_pElements =
         (HSCRIPT **)((int)&vgui::Panel::GetMessageMap::s_pMap.magic + 2);
    this->m_bHasFutbol = true;
    DispatchSpawn(this_00,true);
    (*(this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
      super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xeb])(this,this_00);
  }
  return;
}


/* CPropGlassFutbol::ThrownThink at 006dc010 */

/* DWARF original prototype: void ThrownThink(CPropGlassFutbol * this) */

void __thiscall CPropGlassFutbol::ThrownThink(CPropGlassFutbol *this)

{
  IPhysicsObject *pIVar1;
  int unaff_EBX;
  Vector local_28 [2];
  
                    /* Unresolved local var: Vector vecForce@[???]
                       Unresolved local var: IPhysicsObject * pPhys@[???] */
  ___i686_get_pc_thunk_bx();
  local_28[0].z = *(float *)(*(int *)(unaff_EBX + 0x653838) + 0x2c);
                    /* Unresolved local var: Vector res@[???] */
  local_28[0].x = local_28[0].z * (this->m_vecThrowDirection).x;
  local_28[0].y = local_28[0].z * (this->m_vecThrowDirection).y;
  local_28[0].z = local_28[0].z * (this->m_vecThrowDirection).z;
  pIVar1 = (this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.super_CBaseAnimating.
           super_CBaseEntity.m_pPhysicsObject;
  if (pIVar1 != (IPhysicsObject *)0x0) {
    (*pIVar1->_vptr_IPhysicsObject[0x34])(pIVar1,*(undefined4 *)(unaff_EBX + 0x4cfb8c),0);
    (*pIVar1->_vptr_IPhysicsObject[0x3d])(pIVar1,local_28);
  }
  CBaseEntity::ApplyAbsVelocityImpulse((CBaseEntity *)this,local_28);
  CBaseEntity::ThinkSet((CBaseEntity *)this,(BASEPTR)0x0,-1.0,*(char **)(unaff_EBX + 0x5f1760));
  return;
}


/* CPropGlassFutbol::AnimThink at 006db9d0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void AnimThink(CPropGlassFutbol * this) */

void __thiscall CPropGlassFutbol::AnimThink(CPropGlassFutbol *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (**(code **)((int)(this->super_CPhysicsProp).super_CBreakableProp.super_CBaseProp.
                    super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.super_IServerUnknown
              + 0x31c))(this);
  CBaseEntity::SetNextThink
            ((CBaseEntity *)this,
             *(float *)(**(int **)(unaff_EBX + 0x4d01bf) + 0xc) +
             *(float *)(**(int **)(unaff_EBX + 0x4d01bf) + 0x1c),*(char **)(unaff_EBX + 0x5f1d9f));
  return;
}


/* CPropFutbolSpawner::CPropFutbolSpawner at 006dcc00 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CPropFutbolSpawner(CPropFutbolSpawner * this) */

void __thiscall CPropFutbolSpawner::CPropFutbolSpawner(CPropFutbolSpawner *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CFutbolCatcher::CFutbolCatcher(&this->super_CFutbolCatcher);
  (this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x55ba37);
  this->m_bHasFutbol = false;
  (this->m_OnFutbolSpawned).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnFutbolSpawned).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->m_OnFutbolSpawned).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->m_OnFutbolGrabbed).super_CBaseEntityOutput.m_Value.field_0.iVal = 0;
  (this->m_OnFutbolGrabbed).super_CBaseEntityOutput.m_Value.eVal.super_CBaseHandle.m_Index =
       0xffffffff;
  (this->m_OnFutbolGrabbed).super_CBaseEntityOutput.m_Value.fieldType = FIELD_VOID;
  (this->super_CFutbolCatcher).m_vCatcherBoxHalfDiagonal.x = 7.0;
  (this->super_CFutbolCatcher).m_vCatcherBoxHalfDiagonal.y = 7.0;
  (this->super_CFutbolCatcher).m_vCatcherBoxHalfDiagonal.z = 7.0;
  return;
}


/* CPropFutbolSpawner::CPropFutbolSpawner at 006dcc90 */

/* DWARF original prototype: void CPropFutbolSpawner(CPropFutbolSpawner * this, CPropFutbolSpawner *
   this) */

void __thiscall
CPropFutbolSpawner::CPropFutbolSpawner(CPropFutbolSpawner *this,CPropFutbolSpawner *this_1)

{
  CPropFutbolSpawner(this);
  return;
}


/* CEntityFactory<CPropFutbolSpawner>::Create at 006dd250 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CPropFutbolSpawner> * this,
   char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CPropFutbolSpawner>::Create
          (CEntityFactory<CPropFutbolSpawner> *this,char *pClassName)

{
  CPropFutbolSpawner *this_00;
  
                    /* Unresolved local var: CPropFutbolSpawner * pEnt@[???] */
  this_00 = CBaseEntity::operator_new(0x5b0);
  CPropFutbolSpawner::CPropFutbolSpawner(this_00);
  (*(this_00->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1d])(this_00,pClassName);
  return &(this_00->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.m_Network.
          super_IServerNetworkable;
}


/* CPropFutbolSpawner::Precache at 006db970 */

/* DWARF original prototype: void Precache(CPropFutbolSpawner * this) */

void __thiscall CPropFutbolSpawner::Precache(CPropFutbolSpawner *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  CBaseEntity::PrecacheModel((char *)(extraout_ECX + 0x3359bc));
  return;
}


/* CPropFutbolSpawner::Spawn at 006dbbc0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CPropFutbolSpawner * this) */

void __thiscall CPropFutbolSpawner::Spawn(CPropFutbolSpawner *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1a])(this);
  (*(this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1b])(this,unaff_EBX + 0x335763);
  CCollisionProperty::SetSolid
            (&(this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.m_Collision.
              super_CCollisionProperty,SOLID_VPHYSICS);
  CFutbolCatcher::Spawn(&this->super_CFutbolCatcher);
  if (this->m_bHasFutbol == false) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x006dbc34. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xee])();
  return;
}


/* CPropFutbolSpawner::FutbolDestroyed at 006db910 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void FutbolDestroyed(CPropFutbolSpawner * this) */

void __thiscall CPropFutbolSpawner::FutbolDestroyed(CPropFutbolSpawner *this)

{
                    /* WARNING: Could not recover jumptable at 0x006db91f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xee])();
  return;
}


/* CPropFutbolSpawner::FutbolGrabbed at 006db990 */

/* DWARF original prototype: void FutbolGrabbed(CPropFutbolSpawner * this) */

void __thiscall CPropFutbolSpawner::FutbolGrabbed(CPropFutbolSpawner *this)

{
  if (this->m_bHasFutbol != false) {
    COutputEvent::FireOutput(&this->m_OnFutbolGrabbed,(CBaseEntity *)this,(CBaseEntity *)this,0.0);
  }
  this->m_bHasFutbol = false;
  return;
}


/* CPropFutbolSpawner::InputForceSpawn at 006db930 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void InputForceSpawn(CPropFutbolSpawner * this, inputdata_t * data) */

void __thiscall CPropFutbolSpawner::InputForceSpawn(CPropFutbolSpawner *this,inputdata_t *data)

{
                    /* WARNING: Could not recover jumptable at 0x006db93f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xee])();
  return;
}


/* CPropFutbolSocket::CPropFutbolSocket at 006dcb80 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CPropFutbolSocket(CPropFutbolSocket * this) */

void __thiscall CPropFutbolSocket::CPropFutbolSocket(CPropFutbolSocket *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CFutbolCatcher::CFutbolCatcher(&this->super_CFutbolCatcher);
  (this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x55b1b7);
  return;
}


/* CPropFutbolSocket::CPropFutbolSocket at 006dcbc0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void CPropFutbolSocket(CPropFutbolSocket * this) */

void __thiscall CPropFutbolSocket::CPropFutbolSocket(CPropFutbolSocket *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CFutbolCatcher::CFutbolCatcher(&this->super_CFutbolCatcher);
  (this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
  super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(&UNK_0055b177 + unaff_EBX);
  return;
}


/* CEntityFactory<CPropFutbolSocket>::Create at 006dd2a0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CPropFutbolSocket> * this,
   char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CPropFutbolSocket>::Create(CEntityFactory<CPropFutbolSocket> *this,char *pClassName)

{
  CFutbolCatcher *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CPropFutbolSocket * pEnt@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CBaseEntity::operator_new(0x57c);
  CFutbolCatcher::CFutbolCatcher(this_00);
  (this_00->super_CBaseAnimating).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x55aa94);
  CBaseEntity::PostConstructor((CBaseEntity *)this_00,pClassName);
  return &(this_00->super_CBaseAnimating).super_CBaseEntity.m_Network.super_IServerNetworkable;
}


/* CPropFutbolSocket::Precache at 006db950 */

/* DWARF original prototype: void Precache(CPropFutbolSocket * this) */

void __thiscall CPropFutbolSocket::Precache(CPropFutbolSocket *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  CBaseEntity::PrecacheModel((char *)(extraout_ECX + 0x3359bc));
  return;
}


/* CPropFutbolSocket::Spawn at 006dc230 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CPropFutbolSocket * this) */

void __thiscall CPropFutbolSocket::Spawn(CPropFutbolSocket *this)

{
  int nSequence;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1a])(this);
  (*(this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1b])
            (this,&UNK_003350d7 + unaff_EBX);
  CCollisionProperty::SetSolid
            (&(this->super_CFutbolCatcher).super_CBaseAnimating.super_CBaseEntity.m_Collision.
              super_CCollisionProperty,SOLID_VPHYSICS);
  nSequence = CBaseAnimating::LookupSequence((CBaseAnimating *)this,(char *)(unaff_EBX + 0x33567b));
  CBaseAnimating::SetSequence((CBaseAnimating *)this,nSequence);
  CFutbolCatcher::Spawn(&this->super_CFutbolCatcher);
  return;
}


/* _GLOBAL__I_sv_futbol_fake_force at 000c8940 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_sv_futbol_fake_force(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

