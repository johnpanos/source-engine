/* DWARF-guided pseudocode for game/client/portal2/c_pointsurvey.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* C_PointSurvey::~C_PointSurvey at 0052c150 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PointSurvey": ignoring overlapping field "m_iszSurveyName" */
/* DWARF original prototype: void ~C_PointSurvey(C_PointSurvey * this, int __in_chrg) */

void __thiscall C_PointSurvey::~C_PointSurvey(C_PointSurvey *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x66b2c7);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x66b65f);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(&UNK_0066b717 + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(&UNK_0066b753 + unaff_EBX);
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,in_stack_ffffffe8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_PointSurvey::~C_PointSurvey at 0052c1c0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PointSurvey": ignoring overlapping field "m_iszSurveyName" */
/* DWARF original prototype: void ~C_PointSurvey(C_PointSurvey * this, int __in_chrg) */

void __thiscall C_PointSurvey::~C_PointSurvey(C_PointSurvey *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x66b260);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(extraout_ECX + 0x66b5f8);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(&UNK_0066b6b0 + extraout_ECX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(&UNK_0066b6ec + extraout_ECX);
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
  return;
}


/* C_PointSurvey::PostDataUpdate at 0052c130 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PointSurvey": ignoring overlapping field "m_iszSurveyName" */
/* DWARF original prototype: void PostDataUpdate(C_PointSurvey * this, DataUpdateType_t updateType)
    */

void __thiscall C_PointSurvey::PostDataUpdate(C_PointSurvey *this,DataUpdateType_t updateType)

{
  return;
}


/* ClientClassInit<DT_PointSurvey::ignored> at 000764d0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

int ClientClassInit<DT_PointSurvey::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3e4].m_pEntity + unaff_EBX)
      == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc41fa4);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3e5].m_pPrev + unaff_EBX
                  ),&UNK_008e987c + unaff_EBX,0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3e9].
                               m_SerialNumber + unaff_EBX),(char *)(unaff_EBX + 0x8ee45c),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00aba900 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00aba8bc + unaff_EBX));
      RecvPropString((RecvProp_conflict1 *)
                     ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3ed].m_pEntity +
                     unaff_EBX),(char *)(unaff_EBX + 0x9196f8),0xa59,0x104,0,
                     *(RecvVarProxyFn *)(&DAT_00abaadc + unaff_EBX));
      ___cxa_guard_release(unaff_EBX + 0xc41fa4);
    }
  }
  RecvTable::Construct
            ((RecvTable *)
             ((int)&s_ParticleSystemMgr.m_ParticleOperators[3].m_Memory.m_pMemory + unaff_EBX),
             (RecvProp_conflict1 *)
             ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3e9].m_SerialNumber +
             unaff_EBX),2,(char *)(unaff_EBX + 0x919708));
  return 1;
}


/* __static_initialization_and_destruction_0 at 00076630 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  RecvTable *this;
  int *piVar1;
  undefined4 uVar2;
  int iVar3;
  int unaff_EBX;
  longlong lVar4;
  
  lVar4 = ___i686_get_pc_thunk_bx();
  if (lVar4 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3c9].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3c9].m_pNext + unaff_EBX)
       = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3c9].m_pNext + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3c9].m_pNext + unaff_EBX + 2) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3c9].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3ca].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3ca].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3ca].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3ca].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3cb].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3cb].m_SerialNumber + unaff_EBX + 3) =
       0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3cb].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3cb].m_pNext + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3cc].m_pEntity + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3cc].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x3cc].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined **)
   (&UNK_00003cd3 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       &UNK_00abb7c7 + unaff_EBX;
  *(int *)((int)&vec2_origin.x + unaff_EBX + 3) = unaff_EBX + 0x9195ba;
  *(undefined **)
   ((int)&DmxElementUnpackInit<C_OP_BasicMovement>(C_OP_BasicMovement*)::unpack + unaff_EBX + 7) =
       &UNK_004b5a3f + unaff_EBX;
  *(undefined4 *)(UNSPECIFIED_LOGGING_COLOR._color + unaff_EBX + 3) = 0;
  this = (RecvTable *)((int)&vec4_origin.x + unaff_EBX + 3);
  *(RecvTable **)((int)&vec2_origin.y + unaff_EBX + 3) = this;
  piVar1 = *(int **)(&DAT_00aba757 + unaff_EBX);
  *(int *)((int)&vec2_invalid.x + unaff_EBX + 3) = *piVar1;
  *piVar1 = unaff_EBX + 0xcd9447;
  RecvTable::RecvTable(this);
  uVar2 = *(undefined4 *)(&DAT_00aba6df + unaff_EBX);
  ___cxa_atexit(&UNK_008db26f + unaff_EBX,0,uVar2);
  iVar3 = ClientClassInit<DT_PointSurvey::ignored>((ignored *)0x0);
  *(int *)((int)&vec4_invalid.y + unaff_EBX + 3) = iVar3;
  CAutoGameSystem::CAutoGameSystem
            ((CAutoGameSystem *)
             (&UNK_00003cd7 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4
             ),(char *)0x0);
  *(undefined **)
   (&UNK_00003cd7 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       &UNK_00b212a7 + unaff_EBX;
  ___cxa_atexit(unaff_EBX + 0x8db28f,0,uVar2);
  return;
}


/* C_PointSurvey::YouForgotToImplementOrDeclareClientClass at 0052bcd0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PointSurvey": ignoring overlapping field "m_iszSurveyName" */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_PointSurvey * this) */

int __thiscall C_PointSurvey::YouForgotToImplementOrDeclareClientClass(C_PointSurvey *this)

{
  return 0;
}


/* C_PointSurvey::GetClientClass at 0052bcf0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PointSurvey": ignoring overlapping field "m_iszSurveyName" */
/* DWARF original prototype: ClientClass * GetClientClass(C_PointSurvey * this) */

ClientClass * __thiscall C_PointSurvey::GetClientClass(C_PointSurvey *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x823d90);
}


/* _C_PointSurvey_CreateObject at 0052c080 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_PointSurvey_CreateObject(int entnum,int serialNum)

{
  C_BaseEntity *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_PointSurvey * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xb60);
  C_BaseEntity::C_BaseEntity(this);
  (this->super_IClientEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x66b394);
  (this->super_IClientEntity).super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(&UNK_0066b72c + unaff_EBX);
  (this->super_IClientEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x66b7e4);
  (this->super_IClientEntity).super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x66b820);
  (**(code **)(unaff_EBX + 0x66b450))(this,entnum,serialNum);
  return &(this->super_IClientEntity).super_IClientNetworkable;
}


/* __tcf_0 at 009518b0 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3fe1e8),in_stack_00000008);
  return;
}


/* C_PointSurvey::C_PointSurvey at 0052bd00 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PointSurvey": ignoring overlapping field "m_iszSurveyName" */
/* DWARF original prototype: void C_PointSurvey(C_PointSurvey * this) */

void __thiscall C_PointSurvey::C_PointSurvey(C_PointSurvey *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::C_BaseEntity(&this->super_C_BaseEntity);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(&UNK_0066b717 + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(&UNK_0066baaf + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x66bb67);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x66bba3);
  return;
}


/* C_PointSurvey::C_PointSurvey at 0052bd60 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PointSurvey": ignoring overlapping field "m_iszSurveyName" */
/* DWARF original prototype: void C_PointSurvey(C_PointSurvey * this) */

void __thiscall C_PointSurvey::C_PointSurvey(C_PointSurvey *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  C_BaseEntity::C_BaseEntity(&this->super_C_BaseEntity);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(&UNK_0066b6b7 + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x66ba4f);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(&UNK_0066bb07 + unaff_EBX);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x66bb43);
  return;
}


/* __MsgFunc_StartSurvey at 0052bdc0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

void __MsgFunc_StartSurvey(bf_read *msg)

{
  uint32 **ppuVar1;
  uint32 *puVar2;
  uint32 *puVar3;
  uint uVar4;
  uint uVar5;
  code *pcVar6;
  bool bVar7;
  C_BaseEntity *pCVar8;
  int iVar9;
  char *pcVar10;
  IClientMode *pIVar11;
  int *piVar12;
  undefined4 *puVar13;
  int iVar14;
  int unaff_EBX;
  uint uVar15;
  byte local_30;
  CUtlString local_2c;
  
                    /* Unresolved local var: C_PointSurvey * pPointSurvey@[???] */
  ___i686_get_pc_thunk_bx();
  iVar9 = (msg->super_CBitRead).m_nBitsAvail;
  if (iVar9 < 0x20) {
    uVar4 = (msg->super_CBitRead).m_nInBufWord;
    puVar3 = (msg->super_CBitRead).m_pDataIn;
    puVar2 = (msg->super_CBitRead).m_pBufferEnd;
    if (puVar3 == puVar2) {
      (msg->super_CBitRead).m_nBitsAvail = 1;
      (msg->super_CBitRead).m_nInBufWord = 0;
      ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
      *ppuVar1 = *ppuVar1 + 1;
      (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
      uVar15 = 0;
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
      uVar15 = 0;
      if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
        iVar14 = 0x20 - iVar9;
        uVar15 = (msg->super_CBitRead).m_nInBufWord;
        uVar5 = *(uint *)(*(int *)(unaff_EBX + 0x604f8e) + iVar14 * 4);
        (msg->super_CBitRead).m_nBitsAvail = 0x20 - iVar14;
        local_30 = (byte)iVar14;
        puVar2 = &(msg->super_CBitRead).m_nInBufWord;
        *puVar2 = *puVar2 >> (local_30 & 0x1f);
        uVar15 = (uVar15 & uVar5) << ((byte)iVar9 & 0x1f) | uVar4;
      }
    }
  }
  else {
    uVar15 = (msg->super_CBitRead).m_nInBufWord & *(uint *)(*(int *)(unaff_EBX + 0x604f8e) + 0x80);
    (msg->super_CBitRead).m_nBitsAvail = iVar9 + -0x20;
    if (iVar9 + -0x20 == 0) {
      (msg->super_CBitRead).m_nBitsAvail = 0x20;
      puVar3 = (msg->super_CBitRead).m_pDataIn;
      puVar2 = (msg->super_CBitRead).m_pBufferEnd;
      if (puVar3 == puVar2) {
        (msg->super_CBitRead).m_nBitsAvail = 1;
        (msg->super_CBitRead).m_nInBufWord = 0;
        ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
        *ppuVar1 = *ppuVar1 + 1;
      }
      else {
        if (puVar2 < puVar3) {
          (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
          (msg->super_CBitRead).m_nInBufWord = 0;
          pCVar8 = UTIL_EntityFromUserMessageEHandle(uVar15);
          goto joined_r0x0052bebc;
        }
        (msg->super_CBitRead).m_nInBufWord = *puVar3;
        (msg->super_CBitRead).m_pDataIn = puVar3 + 1;
      }
    }
    else {
      puVar2 = &(msg->super_CBitRead).m_nInBufWord;
      *puVar2 = *puVar2;
    }
  }
  pCVar8 = UTIL_EntityFromUserMessageEHandle(uVar15);
joined_r0x0052bebc:
  if (pCVar8 == (C_BaseEntity *)0x0) {
    _Warning(unaff_EBX + 0x463e4a);
    return;
  }
  bVar7 = ShowSurveyPanel(&pCVar8->field_0xa59);
  if (bVar7) {
    pIVar11 = GetClientMode();
    iVar9 = (*pIVar11->_vptr_IClientMode[10])(pIVar11);
    if ((((iVar9 != 0) &&
         (piVar12 = (int *)___dynamic_cast(iVar9,*(undefined4 *)(unaff_EBX + 0x60541e),
                                           *(undefined4 *)(unaff_EBX + 0x605426),0),
         piVar12 != (int *)0x0)) &&
        (iVar9 = (**(code **)(*piVar12 + 0x3b8))(piVar12,unaff_EBX + 0x45f2d3), iVar9 != 0)) &&
       (iVar9 != 0x210)) {
      puVar13 = (undefined4 *)
                (*(pCVar8->super_IClientEntity).super_IClientUnknown.super_IHandleEntity.
                  _vptr_IHandleEntity[3])(pCVar8);
      *(undefined4 *)(iVar9 + 8) = *puVar13;
      return;
    }
  }
  else {
    CUtlString::CUtlString(&local_2c);
    iVar9 = (*(pCVar8->super_IClientEntity).super_IClientUnknown.super_IHandleEntity.
              _vptr_IHandleEntity[0x51])(pCVar8);
    CUtlString::Format(&local_2c,(char *)(unaff_EBX + 0x463e3a),iVar9);
    pcVar6 = *(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x604f8a) + 0x1c);
    pcVar10 = CUtlString::operator_const_char_(&local_2c);
    (*pcVar6)(**(undefined4 **)(unaff_EBX + 0x604f8a),pcVar10);
    local_2c.m_Storage.m_nActualLength = 0;
    if ((-1 < local_2c.m_Storage.m_Memory.m_nGrowSize) &&
       (local_2c.m_Storage.m_Memory.m_pMemory != (uchar *)0x0)) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x604f56) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x604f56),
                 local_2c.m_Storage.m_Memory.m_pMemory);
    }
  }
  return;
}


/* C_PointSurveyHelper::~C_PointSurveyHelper at 0052c320 */

/* DWARF original prototype: void ~C_PointSurveyHelper(C_PointSurveyHelper * this, int __in_chrg) */

void __thiscall C_PointSurveyHelper::~C_PointSurveyHelper(C_PointSurveyHelper *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(unaff_EBX + 0x606cf7);
  IGameSystem::~IGameSystem((IGameSystem *)this,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* C_PointSurveyHelper::~C_PointSurveyHelper at 0052c360 */

/* DWARF original prototype: void ~C_PointSurveyHelper(C_PointSurveyHelper * this, int __in_chrg) */

void __thiscall C_PointSurveyHelper::~C_PointSurveyHelper(C_PointSurveyHelper *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(extraout_ECX + 0x606cc0);
  IGameSystem::~IGameSystem((IGameSystem *)this,__in_chrg);
  return;
}


/* C_PointSurveyHelper::Init at 0052c210 */

/* DWARF original prototype: bool Init(C_PointSurveyHelper * this) */

bool __thiscall C_PointSurveyHelper::Init(C_PointSurveyHelper *this)

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
  iVar2 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x604b3a) + 0x1f8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x604b3a));
  pchContext = &UNK_00463952 + unaff_EBX;
  CSetActiveSplitScreenPlayerGuard::CSetActiveSplitScreenPlayerGuard
            (&local_3c,pchContext,0x59,0,iVar2,false,in_stack_ffffffac,in_stack_ffffffb0,
             in_stack_ffffffb4,in_stack_ffffffb8,in_stack_ffffffbc,SUB41(pchContext,0));
  hook = (pfnUserMsgHook)(unaff_EBX + -0x45e);
  name = (char *)(unaff_EBX + 0x4639b2);
  puVar1 = *(undefined4 **)(unaff_EBX + 0x604b42);
  name_00 = name;
  CUserMessages::HookMessage((CUserMessages *)*puVar1,name,hook);
  CSetActiveSplitScreenPlayerGuard::~CSetActiveSplitScreenPlayerGuard(&local_3c,(int)name);
  iVar2 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x604b3a) + 0x1f8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x604b3a));
  CSetActiveSplitScreenPlayerGuard::CSetActiveSplitScreenPlayerGuard
            (&local_3c,pchContext,0x59,1,iVar2,false,in_stack_ffffffac,in_stack_ffffffb0,
             in_stack_ffffffb4,(int)name_00,(int)hook,SUB41(pchContext,0));
  CUserMessages::HookMessage((CUserMessages *)*puVar1,name_00,hook);
  CSetActiveSplitScreenPlayerGuard::~CSetActiveSplitScreenPlayerGuard(&local_3c,(int)name_00);
  return true;
}


/* __tcf_1 at 009518d0 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  *(int *)(&DAT_00366ba0 + extraout_ECX) = extraout_ECX + 0x1e1750;
  IGameSystem::~IGameSystem((IGameSystem *)(&DAT_00366ba0 + extraout_ECX),in_stack_00000008);
  return;
}


/* _GLOBAL__I__ZN13C_PointSurvey17m_pClassRecvTableE at 00076790 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN13C_PointSurvey17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

