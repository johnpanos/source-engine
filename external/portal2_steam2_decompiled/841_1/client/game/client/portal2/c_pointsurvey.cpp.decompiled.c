/* DWARF-guided pseudocode for game/client/portal2/c_pointsurvey.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* C_PointSurvey::~C_PointSurvey at 0052d050 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x69e787);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x69eb23);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x69ebdb);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x69ec17);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x69ec33);
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,in_stack_ffffffe8);
  C_BaseEntity::operator_delete(this);
  return;
}


/* C_PointSurvey::~C_PointSurvey at 0052d0c0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PointSurvey": ignoring overlapping field "m_iszSurveyName" */
/* DWARF original prototype: void ~C_PointSurvey(C_PointSurvey * this, int __in_chrg) */

void __thiscall C_PointSurvey::~C_PointSurvey(C_PointSurvey *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientUnknown.super_IHandleEntity.
  _vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x69e720);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(extraout_ECX + 0x69eabc);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(extraout_ECX + 0x69eb74);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(extraout_ECX + 0x69ebb0);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(extraout_ECX + 0x69ebcc);
  C_BaseEntity::~C_BaseEntity(&this->super_C_BaseEntity,__in_chrg);
  return;
}


/* C_PointSurvey::PostDataUpdate at 0052d030 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PointSurvey": ignoring overlapping field "m_iszSurveyName" */
/* DWARF original prototype: void PostDataUpdate(C_PointSurvey * this, DataUpdateType_t updateType)
    */

void __thiscall C_PointSurvey::PostDataUpdate(C_PointSurvey *this,DataUpdateType_t updateType)

{
  return;
}


/* ClientClassInit<DT_PointSurvey::ignored> at 00076af0 */

int ClientClassInit<DT_PointSurvey::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if ((&DAT_00c5ae44)[unaff_EBX] == '\0') {
    iVar1 = ___cxa_guard_acquire(&DAT_00c5ae44 + unaff_EBX);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&CPolygonButton::KB_ChainToMap()::chained + unaff_EBX + 2),
                  (char *)(unaff_EBX + 0x913c6c),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)((int)&CRadialButton::GetKBMap::s_pMap + unaff_EBX),
                        (char *)(unaff_EBX + 0x9187b0),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00aed32c + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00aed2e8 + unaff_EBX));
      RecvPropString((RecvProp_conflict1 *)
                     ((int)&CPortalClientScoreBoardDialog::GetMessageMap::s_pMap + unaff_EBX),
                     (char *)(unaff_EBX + 0x943ee4),0xa5d,0x104,0,
                     *(RecvVarProxyFn *)(&DAT_00aed508 + unaff_EBX));
      ___cxa_guard_release(&DAT_00c5ae44 + unaff_EBX);
    }
  }
  RecvTable::Construct
            ((RecvTable *)
             ((int)&DmxElementUnpackInit<C_OP_SetControlPointRotation>(C_OP_SetControlPointRotation*)
                    ::unpack + unaff_EBX + 4),
             (RecvProp_conflict1 *)((int)&CRadialButton::GetKBMap::s_pMap + unaff_EBX),2,
             (char *)(unaff_EBX + 0x943ef4));
  return 1;
}


/* __static_initialization_and_destruction_0 at 00076c50 */

/* WARNING: Enum "DmAttributeType_t": Some values do not have unique names */
/* WARNING: Enum "BitfieldType_t": Some values do not have unique names */

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
  (&DAT_00c5ac9f)[unaff_EBX] = 0;
  *(undefined1 *)((int)&BaseModUI::VoteOptions::GetKBMap::s_pMap + unaff_EBX) = 0;
  *(undefined1 *)((int)&BaseModUI::VoteOptions::GetKBMap::s_pMap + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&BaseModUI::VoteOptions::GetKBMap::s_pMap + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&BaseModUI::VoteOptions::GetKBMap::s_pMap + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&BaseModUI::VoteOptions::GetMessageMap::s_pMap + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&BaseModUI::VoteOptions::KB_ChainToMap()::chained + unaff_EBX + 1) =
       0x7f7fffff;
  *(undefined4 *)(&DAT_00c5acaf + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&vgui::QueryBox::GetKBMap::s_pMap + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&vgui::QueryBox::GetMessageMap::s_pMap + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&vgui::URLButton::GetKBMap::s_pMap + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&vgui::URLButton::GetMessageMap::s_pMap + unaff_EBX + 3) = 0;
  *(undefined4 *)(&vgui::URLButton::PanelMessageFunc_DoClick::InitVar()::bAdded + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&vgui::URLButton::PanelMessageFunc_OnSetState::InitVar()::bAdded + unaff_EBX + 2) =
       0x7f7fffff;
  *(undefined4 *)((int)&vgui::SectionedListPanel::GetKBMap::s_pMap + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&vgui::SectionedListPanel::GetMessageMap::s_pMap + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined **)(&DAT_00c5acd3 + unaff_EBX) = &UNK_00aee227 + unaff_EBX;
  *(int *)(&s_C_OP_MaxVelocityFactory.field_0xf + unaff_EBX) = unaff_EBX + 0x943da6;
  *(int *)((int)&s_C_OP_MaxVelocityFactory.m_pFactoryName + unaff_EBX + 3) = unaff_EBX + 0x4b630f;
  *(undefined4 *)((int)&s_C_OP_MaxVelocityFactory.m_Id + unaff_EBX + 3) = 0;
  this = (RecvTable *)((int)&s_C_OP_MaintainSequentialPathFactory.m_Id + unaff_EBX + 3);
  *(RecvTable **)((int)&C_OP_MaxVelocity_UnpackInit::s_pUnpack + unaff_EBX + 3) = this;
  piVar1 = *(int **)(&DAT_00aed183 + unaff_EBX);
  *(int *)((int)&s_C_OP_MaintainSequentialPathFactory.super_IParticleOperatorDefinition.
                 _vptr_IParticleOperatorDefinition + unaff_EBX + 3) = *piVar1;
  *piVar1 = unaff_EBX + 0xcf2487;
  RecvTable::RecvTable(this);
  uVar2 = *(undefined4 *)(&DAT_00aed10b + unaff_EBX);
  ___cxa_atexit(unaff_EBX + 0x90526f,0,uVar2);
  iVar3 = ClientClassInit<DT_PointSurvey::ignored>((ignored *)0x0);
  *(int *)((int)&s_C_OP_RemapDotProductToScalarFactory.m_Id + unaff_EBX + 3) = iVar3;
  CAutoGameSystem::CAutoGameSystem((CAutoGameSystem *)(&DAT_00c5acd7 + unaff_EBX),(char *)0x0);
  *(undefined **)(&DAT_00c5acd7 + unaff_EBX) = &UNK_00b55067 + unaff_EBX;
  ___cxa_atexit(unaff_EBX + 0x90528f,0,uVar2);
  return;
}


/* C_PointSurvey::YouForgotToImplementOrDeclareClientClass at 0052cba0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PointSurvey": ignoring overlapping field "m_iszSurveyName" */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_PointSurvey * this) */

int __thiscall C_PointSurvey::YouForgotToImplementOrDeclareClientClass(C_PointSurvey *this)

{
  return 0;
}


/* C_PointSurvey::GetClientClass at 0052cbc0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_PointSurvey": ignoring overlapping field "m_iszSurveyName" */
/* DWARF original prototype: ClientClass * GetClientClass(C_PointSurvey * this) */

ClientClass * __thiscall C_PointSurvey::GetClientClass(C_PointSurvey *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x83c520);
}


/* _C_PointSurvey_CreateObject at 0052cf70 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_PointSurvey_CreateObject(int entnum,int serialNum)

{
  C_BaseEntity *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_PointSurvey * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xb64);
  C_BaseEntity::C_BaseEntity(this);
  (this->super_IClientEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x69e864);
  (this->super_IClientEntity).super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x69ec00);
  (this->super_IClientEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x69ecb8);
  (this->super_IClientEntity).super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x69ecf4);
  (this->super_IClientModelRenderable)._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x69ed10);
  (**(code **)(unaff_EBX + 0x69e920))(this,entnum,serialNum);
  return &(this->super_IClientEntity).super_IClientNetworkable;
}


/* __tcf_0 at 0097bed0 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3ed228),in_stack_00000008);
  return;
}


/* C_PointSurvey::C_PointSurvey at 0052cbd0 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x69ec07);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x69efa3);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x69f05b);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(CViewEffects::Save + unaff_EBX + 7);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x69f0b3);
  return;
}


/* C_PointSurvey::C_PointSurvey at 0052cc40 */

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
  _vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x69eb97);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x69ef33);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x69efeb);
  (this->super_C_BaseEntity).super_IClientEntity.super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x69f027);
  (this->super_C_BaseEntity).super_IClientModelRenderable._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x69f043);
  return;
}


/* __MsgFunc_StartSurvey at 0052ccb0 */

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
        uVar5 = *(uint *)(*(int *)(unaff_EBX + 0x6370ea) + iVar14 * 4);
        (msg->super_CBitRead).m_nBitsAvail = 0x20 - iVar14;
        local_30 = (byte)iVar14;
        puVar2 = &(msg->super_CBitRead).m_nInBufWord;
        *puVar2 = *puVar2 >> (local_30 & 0x1f);
        uVar15 = (uVar15 & uVar5) << ((byte)iVar9 & 0x1f) | uVar4;
      }
    }
  }
  else {
    uVar15 = (msg->super_CBitRead).m_nInBufWord & *(uint *)(*(int *)(unaff_EBX + 0x6370ea) + 0x80);
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
          goto joined_r0x0052cdac;
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
joined_r0x0052cdac:
  if (pCVar8 == (C_BaseEntity *)0x0) {
    _Warning(unaff_EBX + 0x48dd66);
    return;
  }
  bVar7 = ShowSurveyPanel(&pCVar8->field_0xa5d);
  if (bVar7) {
    pIVar11 = GetClientMode();
    iVar9 = (*pIVar11->_vptr_IClientMode[10])(pIVar11);
    if ((((iVar9 != 0) &&
         (piVar12 = (int *)___dynamic_cast(iVar9,*(undefined4 *)(unaff_EBX + 0x63757e),
                                           *(undefined4 *)(unaff_EBX + 0x637586),0),
         piVar12 != (int *)0x0)) &&
        (iVar9 = (**(code **)(*piVar12 + 0x3b8))(piVar12,unaff_EBX + 0x488e03), iVar9 != 0)) &&
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
              _vptr_IHandleEntity[0x52])(pCVar8);
    CUtlString::Format(&local_2c,(char *)(unaff_EBX + 0x48dd56),iVar9);
    pcVar6 = *(code **)(*(int *)**(undefined4 **)(CRagdoll::~CRagdoll + unaff_EBX + 6) + 0x1c);
    pcVar10 = CUtlString::operator_const_char_(&local_2c);
    (*pcVar6)(**(undefined4 **)(CRagdoll::~CRagdoll + unaff_EBX + 6),pcVar10);
    local_2c.m_Storage.m_nActualLength = 0;
    if ((-1 < local_2c.m_Storage.m_Memory.m_nGrowSize) &&
       (local_2c.m_Storage.m_Memory.m_pMemory != (uchar *)0x0)) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x6370b2) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x6370b2),
                 local_2c.m_Storage.m_Memory.m_pMemory);
    }
  }
  return;
}


/* C_PointSurveyHelper::~C_PointSurveyHelper at 0052d230 */

/* DWARF original prototype: void ~C_PointSurveyHelper(C_PointSurveyHelper * this, int __in_chrg) */

void __thiscall C_PointSurveyHelper::~C_PointSurveyHelper(C_PointSurveyHelper *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(unaff_EBX + 0x638e67);
  IGameSystem::~IGameSystem((IGameSystem *)this,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* C_PointSurveyHelper::~C_PointSurveyHelper at 0052d270 */

/* DWARF original prototype: void ~C_PointSurveyHelper(C_PointSurveyHelper * this, int __in_chrg) */

void __thiscall C_PointSurveyHelper::~C_PointSurveyHelper(C_PointSurveyHelper *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(extraout_ECX + 0x638e30);
  IGameSystem::~IGameSystem((IGameSystem *)this,__in_chrg);
  return;
}


/* C_PointSurveyHelper::Init at 0052d120 */

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
  iVar2 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x636c76) + 0x1f8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x636c76));
  pchContext = (char *)(unaff_EBX + 0x48d84e);
  CSetActiveSplitScreenPlayerGuard::CSetActiveSplitScreenPlayerGuard
            (&local_3c,pchContext,0x59,0,iVar2,false,in_stack_ffffffac,in_stack_ffffffb0,
             in_stack_ffffffb4,in_stack_ffffffb8,in_stack_ffffffbc,SUB41(pchContext,0));
  hook = (pfnUserMsgHook)(unaff_EBX + -0x47e);
  name = (char *)(unaff_EBX + 0x48d8ae);
  puVar1 = *(undefined4 **)(unaff_EBX + 0x636c7e);
  name_00 = name;
  CUserMessages::HookMessage((CUserMessages *)*puVar1,name,hook);
  CSetActiveSplitScreenPlayerGuard::~CSetActiveSplitScreenPlayerGuard(&local_3c,(int)name);
  iVar2 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x636c76) + 0x1f8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x636c76));
  CSetActiveSplitScreenPlayerGuard::CSetActiveSplitScreenPlayerGuard
            (&local_3c,pchContext,0x59,1,iVar2,false,in_stack_ffffffac,in_stack_ffffffb0,
             in_stack_ffffffb4,(int)name_00,(int)hook,SUB41(pchContext,0));
  CUserMessages::HookMessage((CUserMessages *)*puVar1,name_00,hook);
  CSetActiveSplitScreenPlayerGuard::~CSetActiveSplitScreenPlayerGuard(&local_3c,(int)name_00);
  return true;
}


/* __tcf_1 at 0097bef0 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  *(int *)(extraout_ECX + 0x355a40) = extraout_ECX + 0x1ea1b0;
  IGameSystem::~IGameSystem((IGameSystem *)(extraout_ECX + 0x355a40),in_stack_00000008);
  return;
}


/* _GLOBAL__I__ZN13C_PointSurvey17m_pClassRecvTableE at 00076db0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN13C_PointSurvey17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

