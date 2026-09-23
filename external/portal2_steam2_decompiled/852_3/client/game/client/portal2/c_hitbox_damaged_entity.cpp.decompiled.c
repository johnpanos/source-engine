/* DWARF-guided pseudocode for game/client/portal2/c_hitbox_damaged_entity.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* IClientHitboxDamagedEntity::~IClientHitboxDamagedEntity at 00520af0 */

/* DWARF original prototype: void ~IClientHitboxDamagedEntity(IClientHitboxDamagedEntity * this, int
   __in_chrg) */

void __thiscall
IClientHitboxDamagedEntity::~IClientHitboxDamagedEntity
          (IClientHitboxDamagedEntity *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  this->_vptr_IClientHitboxDamagedEntity = (_func_int_varargs **)(extraout_ECX + 0x673d48);
  operator_delete(this);
  return;
}


/* IClientHitboxDamagedEntity::~IClientHitboxDamagedEntity at 00520b10 */

/* DWARF original prototype: void ~IClientHitboxDamagedEntity(IClientHitboxDamagedEntity * this, int
   __in_chrg) */

void __thiscall
IClientHitboxDamagedEntity::~IClientHitboxDamagedEntity
          (IClientHitboxDamagedEntity *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  this->_vptr_IClientHitboxDamagedEntity = (_func_int_varargs **)(extraout_ECX + 0x673d28);
  return;
}


/* IClientHitboxDamagedEntity::~IClientHitboxDamagedEntity at 00520b30 */

/* DWARF original prototype: void ~IClientHitboxDamagedEntity(IClientHitboxDamagedEntity * this, int
   __in_chrg) */

void __thiscall
IClientHitboxDamagedEntity::~IClientHitboxDamagedEntity
          (IClientHitboxDamagedEntity *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  this->_vptr_IClientHitboxDamagedEntity = (_func_int_varargs **)(extraout_ECX + 0x673d08);
  return;
}


/* __MsgFunc_ApplyHitBoxDamageEffect at 00520b50 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

void __MsgFunc_ApplyHitBoxDamageEffect(bf_read *msg)

{
  uint32 **ppuVar1;
  uint32 *puVar2;
  uint32 *puVar3;
  uint uVar4;
  uint uVar5;
  C_BaseEntity *pCVar6;
  int *piVar7;
  byte bVar8;
  uint uVar9;
  int iVar10;
  int unaff_EBX;
  uint uVar11;
  int iVar12;
  byte local_30;
  
                    /* Unresolved local var: IClientHitboxDamagedEntity * pDamagedEntity@[???] */
  ___i686_get_pc_thunk_bx();
  iVar12 = (msg->super_CBitRead).m_nBitsAvail;
  if (iVar12 < 0x20) {
    uVar9 = (msg->super_CBitRead).m_nInBufWord;
    puVar3 = (msg->super_CBitRead).m_pDataIn;
    puVar2 = (msg->super_CBitRead).m_pBufferEnd;
    if (puVar3 == puVar2) {
      (msg->super_CBitRead).m_nBitsAvail = 1;
      (msg->super_CBitRead).m_nInBufWord = 0;
      ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
      *ppuVar1 = *ppuVar1 + 1;
      (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
      uVar11 = 0;
    }
    else {
      if (puVar2 < puVar3) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar3;
        (msg->super_CBitRead).m_pDataIn = puVar3 + 1;
      }
      uVar11 = 0;
      if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
        iVar10 = 0x20 - iVar12;
        uVar11 = (msg->super_CBitRead).m_nInBufWord;
        uVar4 = *(uint *)(*(int *)(&DAT_006101fe + unaff_EBX) + iVar10 * 4);
        (msg->super_CBitRead).m_nBitsAvail = 0x20 - iVar10;
        local_30 = (byte)iVar10;
        puVar2 = &(msg->super_CBitRead).m_nInBufWord;
        *puVar2 = *puVar2 >> (local_30 & 0x1f);
        uVar11 = (uVar11 & uVar4) << ((byte)iVar12 & 0x1f) | uVar9;
      }
    }
LAB_00520c50:
    pCVar6 = UTIL_EntityFromUserMessageEHandle(uVar11);
  }
  else {
    uVar11 = (msg->super_CBitRead).m_nInBufWord &
             *(uint *)(*(int *)(&DAT_006101fe + unaff_EBX) + 0x80);
    (msg->super_CBitRead).m_nBitsAvail = iVar12 + -0x20;
    if (iVar12 + -0x20 != 0) {
      puVar2 = &(msg->super_CBitRead).m_nInBufWord;
      *puVar2 = *puVar2;
      goto LAB_00520c50;
    }
    (msg->super_CBitRead).m_nBitsAvail = 0x20;
    puVar3 = (msg->super_CBitRead).m_pDataIn;
    puVar2 = (msg->super_CBitRead).m_pBufferEnd;
    if (puVar3 == puVar2) {
      (msg->super_CBitRead).m_nBitsAvail = 1;
      (msg->super_CBitRead).m_nInBufWord = 0;
      ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
      *ppuVar1 = *ppuVar1 + 1;
      goto LAB_00520c50;
    }
    if (puVar3 <= puVar2) {
      (msg->super_CBitRead).m_nInBufWord = *puVar3;
      (msg->super_CBitRead).m_pDataIn = puVar3 + 1;
      goto LAB_00520c50;
    }
    (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
    (msg->super_CBitRead).m_nInBufWord = 0;
    pCVar6 = UTIL_EntityFromUserMessageEHandle(uVar11);
  }
  if ((pCVar6 == (C_BaseEntity *)0x0) ||
     (piVar7 = (int *)___dynamic_cast(pCVar6,*(undefined4 *)(unaff_EBX + 0x6101ce),
                                      unaff_EBX + 0x673cee,0xfffffffe), piVar7 == (int *)0x0)) {
    return;
  }
  iVar12 = (msg->super_CBitRead).m_nBitsAvail;
  if (iVar12 < 8) {
    uVar9 = (msg->super_CBitRead).m_nInBufWord;
    puVar3 = (msg->super_CBitRead).m_pDataIn;
    puVar2 = (msg->super_CBitRead).m_pBufferEnd;
    if (puVar3 != puVar2) {
      if (puVar3 < puVar2) {
        (msg->super_CBitRead).m_nInBufWord = *puVar3;
        (msg->super_CBitRead).m_pDataIn = puVar3 + 1;
      }
      else {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      uVar11 = 0;
      if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
        iVar10 = 8 - iVar12;
        uVar11 = (msg->super_CBitRead).m_nInBufWord;
        uVar4 = *(uint *)(*(int *)(&DAT_006101fe + unaff_EBX) + iVar10 * 4);
        bVar8 = (byte)iVar12;
        iVar12 = 0x20 - iVar10;
        (msg->super_CBitRead).m_nBitsAvail = iVar12;
        puVar2 = &(msg->super_CBitRead).m_nInBufWord;
        *puVar2 = *puVar2 >> ((byte)iVar10 & 0x1f);
        uVar11 = (uVar11 & uVar4) << (bVar8 & 0x1f) | uVar9;
      }
      goto joined_r0x00520dc8;
    }
    iVar12 = 1;
    (msg->super_CBitRead).m_nBitsAvail = 1;
    (msg->super_CBitRead).m_nInBufWord = 0;
    ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
    *ppuVar1 = *ppuVar1 + 1;
    (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
    uVar11 = 0;
  }
  else {
    uVar11 = (msg->super_CBitRead).m_nInBufWord &
             *(uint *)(*(int *)(&DAT_006101fe + unaff_EBX) + 0x20);
    (msg->super_CBitRead).m_nBitsAvail = iVar12 + -8;
    if (iVar12 + -8 == 0) {
      (msg->super_CBitRead).m_nBitsAvail = 0x20;
      puVar3 = (msg->super_CBitRead).m_pDataIn;
      puVar2 = (msg->super_CBitRead).m_pBufferEnd;
      if (puVar3 == puVar2) {
        (msg->super_CBitRead).m_nBitsAvail = 1;
        (msg->super_CBitRead).m_nInBufWord = 0;
        ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
        *ppuVar1 = *ppuVar1 + 1;
      }
      else if (puVar2 < puVar3) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar3;
        (msg->super_CBitRead).m_pDataIn = puVar3 + 1;
      }
    }
    else {
      puVar2 = &(msg->super_CBitRead).m_nInBufWord;
      *puVar2 = *puVar2 >> 8;
    }
    iVar12 = (msg->super_CBitRead).m_nBitsAvail;
joined_r0x00520dc8:
    if (7 < iVar12) {
      uVar9 = (msg->super_CBitRead).m_nInBufWord &
              *(uint *)(*(int *)(&DAT_006101fe + unaff_EBX) + 0x20);
      (msg->super_CBitRead).m_nBitsAvail = iVar12 + -8;
      if (iVar12 + -8 == 0) {
        (msg->super_CBitRead).m_nBitsAvail = 0x20;
        puVar3 = (msg->super_CBitRead).m_pDataIn;
        puVar2 = (msg->super_CBitRead).m_pBufferEnd;
        if (puVar3 == puVar2) {
          (msg->super_CBitRead).m_nBitsAvail = 1;
          (msg->super_CBitRead).m_nInBufWord = 0;
          ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
          *ppuVar1 = *ppuVar1 + 1;
        }
        else if (puVar2 < puVar3) {
          (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
          (msg->super_CBitRead).m_nInBufWord = 0;
        }
        else {
          (msg->super_CBitRead).m_nInBufWord = *puVar3;
          (msg->super_CBitRead).m_pDataIn = puVar3 + 1;
        }
      }
      else {
        puVar2 = &(msg->super_CBitRead).m_nInBufWord;
        *puVar2 = *puVar2 >> 8;
      }
      goto LAB_00520d29;
    }
  }
  uVar4 = (msg->super_CBitRead).m_nInBufWord;
  puVar3 = (msg->super_CBitRead).m_pDataIn;
  puVar2 = (msg->super_CBitRead).m_pBufferEnd;
  if (puVar3 == puVar2) {
    (msg->super_CBitRead).m_nBitsAvail = 1;
    (msg->super_CBitRead).m_nInBufWord = 0;
    ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
    *ppuVar1 = *ppuVar1 + 1;
    (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
    uVar9 = 0;
  }
  else {
    if (puVar2 < puVar3) {
      (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
      (msg->super_CBitRead).m_nInBufWord = 0;
    }
    else {
      (msg->super_CBitRead).m_nInBufWord = *puVar3;
      (msg->super_CBitRead).m_pDataIn = puVar3 + 1;
    }
    uVar9 = 0;
    if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
      iVar10 = 8 - iVar12;
      uVar9 = (msg->super_CBitRead).m_nInBufWord;
      uVar5 = *(uint *)(*(int *)(&DAT_006101fe + unaff_EBX) + iVar10 * 4);
      (msg->super_CBitRead).m_nBitsAvail = 0x20 - iVar10;
      local_30 = (byte)iVar10;
      puVar2 = &(msg->super_CBitRead).m_nInBufWord;
      *puVar2 = *puVar2 >> (local_30 & 0x1f);
      uVar9 = (uVar9 & uVar5) << ((byte)iVar12 & 0x1f) | uVar4;
    }
  }
LAB_00520d29:
  (**(code **)(*piVar7 + 8))(piVar7,uVar11,uVar9);
  return;
}


/* __static_initialization_and_destruction_0 at 00073970 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  int unaff_EBX;
  longlong lVar1;
  
  lVar1 = ___i686_get_pc_thunk_bx();
  if (lVar1 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x58b].m_pPrev + unaff_EBX)
       = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x58b].m_pPrev + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x58b].m_pPrev + unaff_EBX + 2) = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x58b].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x58b].m_pNext + unaff_EBX)
       = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x58c].m_pEntity + unaff_EBX) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x58c].m_SerialNumber + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x58c].m_pPrev + unaff_EBX)
       = 0x7f7fffff;
  *(undefined4 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x58c].m_pNext + unaff_EBX)
       = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x58d].m_pEntity + unaff_EBX) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x58d].m_SerialNumber + unaff_EBX) = 0;
  *(undefined4 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x58d].m_pPrev + unaff_EBX)
       = 0;
  *(undefined4 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x58d].m_pNext + unaff_EBX)
       = 0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x58e].m_pEntity + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x58e].m_SerialNumber + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x58e].m_pPrev + unaff_EBX)
       = 0x7f7fffff;
  *(undefined **)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x58e].m_pNext + unaff_EBX)
       = &UNK_00abe48c + unaff_EBX;
  CAutoGameSystem::CAutoGameSystem
            ((CAutoGameSystem *)((int)s_pParticlePtrs + (int)(&DAT_00001848 + unaff_EBX)),
             (char *)0x0);
  *(undefined **)((int)s_pParticlePtrs + (int)(&DAT_00001848 + unaff_EBX)) =
       &UNK_00b20eec + unaff_EBX;
  ___cxa_atexit(unaff_EBX + 0x8dd764,0,*(undefined4 *)(&DAT_00abd3a4 + unaff_EBX));
  return;
}


/* C_HitBoxDamageInitHelper::~C_HitBoxDamageInitHelper at 005210d0 */

/* DWARF original prototype: void ~C_HitBoxDamageInitHelper(C_HitBoxDamageInitHelper * this, int
   __in_chrg) */

void __thiscall
C_HitBoxDamageInitHelper::~C_HitBoxDamageInitHelper(C_HitBoxDamageInitHelper *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(extraout_ECX + 0x611f50);
  IGameSystem::~IGameSystem((IGameSystem *)this,__in_chrg);
  return;
}


/* C_HitBoxDamageInitHelper::~C_HitBoxDamageInitHelper at 005210f0 */

/* DWARF original prototype: void ~C_HitBoxDamageInitHelper(C_HitBoxDamageInitHelper * this, int
   __in_chrg) */

void __thiscall
C_HitBoxDamageInitHelper::~C_HitBoxDamageInitHelper(C_HitBoxDamageInitHelper *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(unaff_EBX + 0x611f27);
  IGameSystem::~IGameSystem((IGameSystem *)this,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* C_HitBoxDamageInitHelper::Init at 00520fc0 */

/* DWARF original prototype: bool Init(C_HitBoxDamageInitHelper * this) */

bool __thiscall C_HitBoxDamageInitHelper::Init(C_HitBoxDamageInitHelper *this)

{
  undefined4 *puVar1;
  int iVar2;
  int unaff_EBX;
  char *name;
  char *name_00;
  CSetActiveSplitScreenPlayerGuard *in_stack_ffffffac;
  char *in_stack_ffffffb0;
  int in_stack_ffffffb4;
  int in_stack_ffffffb8;
  int in_stack_ffffffbc;
  pfnUserMsgHook hook;
  char *pchContext;
  CSetActiveSplitScreenPlayerGuard local_3c;
  
  ___i686_get_pc_thunk_bx();
  iVar2 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x60fd8a) + 0x1f8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x60fd8a));
  pchContext = (char *)(unaff_EBX + 0x46daae);
  CSetActiveSplitScreenPlayerGuard::CSetActiveSplitScreenPlayerGuard
            (&local_3c,pchContext,0x23,0,iVar2,false,in_stack_ffffffac,in_stack_ffffffb0,
             in_stack_ffffffb4,in_stack_ffffffb8,in_stack_ffffffbc,SUB41(pchContext,0));
  hook = (pfnUserMsgHook)(unaff_EBX + -0x47e);
  name = (char *)(unaff_EBX + 0x46db18);
  puVar1 = *(undefined4 **)(unaff_EBX + 0x60fd92);
  name_00 = name;
  CUserMessages::HookMessage((CUserMessages *)*puVar1,name,hook);
  CSetActiveSplitScreenPlayerGuard::~CSetActiveSplitScreenPlayerGuard(&local_3c,(int)name);
  iVar2 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x60fd8a) + 0x1f8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x60fd8a));
  CSetActiveSplitScreenPlayerGuard::CSetActiveSplitScreenPlayerGuard
            (&local_3c,pchContext,0x23,1,iVar2,false,in_stack_ffffffac,in_stack_ffffffb0,
             in_stack_ffffffb4,(int)name_00,(int)hook,SUB41(pchContext,0));
  CUserMessages::HookMessage((CUserMessages *)*puVar1,name_00,hook);
  CSetActiveSplitScreenPlayerGuard::~CSetActiveSplitScreenPlayerGuard(&local_3c,(int)name_00);
  return true;
}


/* __tcf_0 at 009510e0 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  *(int *)(extraout_ECX + 0x3fd93c) = extraout_ECX + 0x1e1f40;
  IGameSystem::~IGameSystem((IGameSystem *)(extraout_ECX + 0x3fd93c),in_stack_00000008);
  return;
}


/* _GLOBAL__I__ZN26IClientHitboxDamagedEntityD2Ev at 00073a60 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN26IClientHitboxDamagedEntityD2Ev(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

