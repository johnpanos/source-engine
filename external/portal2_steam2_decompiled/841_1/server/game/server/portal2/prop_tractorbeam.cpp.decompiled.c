/* DWARF-guided pseudocode for game/server/portal2/prop_tractorbeam.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* DataMapInit<CPropTractorBeamProjector> at 000b9020 */

datamap_t * DataMapInit<CPropTractorBeamProjector>(CPropTractorBeamProjector *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((&DAT_00d97bb7)[unaff_EBX] == '\0') {
    iVar1 = ___cxa_guard_acquire(&DAT_00d97bb7 + unaff_EBX);
    if (iVar1 != 0) {
      *(int *)(&DAT_00d97bbf + unaff_EBX) = unaff_EBX + 0x9fc919;
      *(undefined4 *)((int)&sv_soundemitter_version.super_ConCommandBase.m_pNext + unaff_EBX + 3) =
           0;
      *(undefined4 *)(&sv_soundemitter_version.super_ConCommandBase.field_0xb + unaff_EBX) = 0;
      *(undefined4 *)((int)&sv_soundemitter_version.super_ConCommandBase.m_pszName + unaff_EBX + 3)
           = 0;
      *(undefined4 *)
       ((int)&sv_soundemitter_version.super_ConCommandBase.m_pszHelpString + unaff_EBX + 3) = 0;
      *(undefined4 *)((int)&sv_soundemitter_version.super_ConCommandBase.m_nFlags + unaff_EBX + 3) =
           0;
      *(undefined4 *)
       ((int)&sv_soundemitter_version.super_ConCommandBase._vptr_ConCommandBase + unaff_EBX + 3) =
           0x19;
      ___cxa_guard_release(&DAT_00d97bb7 + unaff_EBX);
      ___cxa_atexit(unaff_EBX + 0x987a6f,0,*(undefined4 *)(&DAT_00b8db23 + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_BodyTarget_00cb1f98 + unaff_EBX + 3) =
       *(undefined4 *)(&DAT_00b8e6ab + unaff_EBX);
  *(undefined4 *)((int)&PTR_LocalEyeAngles_00cb1f90 + unaff_EBX + 3) = 2;
  *(int *)((int)&PTR_EyeAngles_00cb1f8c + unaff_EBX + 3) = unaff_EBX + 0xcb1fef;
  return (datamap_t *)((int)&PTR_EyeAngles_00cb1f8c + unaff_EBX + 3);
}


/* __static_initialization_and_destruction_0 at 000b90f0 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *pcVar1;
  char *s2;
  SendTable *this;
  int *piVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  datamap_t *pdVar5;
  IEntityFactoryDictionary *pIVar6;
  int iVar7;
  int iVar8;
  int unaff_EBX;
  longlong lVar9;
  
  lVar9 = ___i686_get_pc_thunk_bx();
  if (lVar9 == 0xffff00000001) {
    g_TokenProcessor.m_szToken[unaff_EBX + 0x1b4] = '\0';
    g_TokenProcessor.m_szToken[unaff_EBX + 0x1b5] = '\0';
    g_TokenProcessor.m_szToken[unaff_EBX + 0x1b6] = '\0';
    g_TokenProcessor.m_szToken[unaff_EBX + 0x1b7] = '\0';
    pcVar1 = g_TokenProcessor.m_szToken + unaff_EBX + 0x1b8;
    pcVar1[0] = '\0';
    pcVar1[1] = '\0';
    pcVar1[2] = '\0';
    pcVar1[3] = '\0';
    pcVar1 = g_TokenProcessor.m_szToken + unaff_EBX + 0x1bc;
    pcVar1[0] = '\0';
    pcVar1[1] = '\0';
    pcVar1[2] = '\0';
    pcVar1[3] = '\0';
    pcVar1 = g_TokenProcessor.m_szToken + unaff_EBX + 0x1c0;
    pcVar1[0] = -1;
    pcVar1[1] = -1;
    pcVar1[2] = '\x7f';
    pcVar1[3] = '\x7f';
    pcVar1 = g_TokenProcessor.m_szToken + unaff_EBX + 0x1c4;
    pcVar1[0] = -1;
    pcVar1[1] = -1;
    pcVar1[2] = '\x7f';
    pcVar1[3] = '\x7f';
    pcVar1 = g_TokenProcessor.m_szToken + unaff_EBX + 0x1c8;
    pcVar1[0] = '\0';
    pcVar1[1] = '\0';
    pcVar1[2] = '\0';
    pcVar1[3] = '\0';
    pcVar1 = g_TokenProcessor.m_szToken + unaff_EBX + 0x1cc;
    pcVar1[0] = '\0';
    pcVar1[1] = '\0';
    pcVar1[2] = '\0';
    pcVar1[3] = '\0';
    pcVar1 = g_TokenProcessor.m_szToken + unaff_EBX + 0x1d0;
    pcVar1[0] = '\0';
    pcVar1[1] = '\0';
    pcVar1[2] = '\0';
    pcVar1[3] = '\0';
    pcVar1 = g_TokenProcessor.m_szToken + unaff_EBX + 0x1d4;
    pcVar1[0] = '\0';
    pcVar1[1] = '\0';
    pcVar1[2] = '\0';
    pcVar1[3] = '\0';
    pcVar1 = g_TokenProcessor.m_szToken + unaff_EBX + 0x1d8;
    pcVar1[0] = -1;
    pcVar1[1] = -1;
    pcVar1[2] = '\x7f';
    pcVar1[3] = '\x7f';
    pcVar1 = g_TokenProcessor.m_szToken + unaff_EBX + 0x1dc;
    pcVar1[0] = -1;
    pcVar1[1] = -1;
    pcVar1[2] = '\x7f';
    pcVar1[3] = '\x7f';
    pcVar1 = g_TokenProcessor.m_szToken + unaff_EBX + 0x1e0;
    pcVar1[0] = -1;
    pcVar1[1] = -1;
    pcVar1[2] = '\x7f';
    pcVar1[3] = '\x7f';
    pcVar1 = g_TokenProcessor.m_szToken + unaff_EBX + 0x1e4;
    pcVar1[0] = -1;
    pcVar1[1] = -1;
    pcVar1[2] = '\x7f';
    pcVar1[3] = '\x7f';
    *(undefined **)(g_TokenProcessor.m_szToken + unaff_EBX + 0x1e8) = &UNK_00b93204 + unaff_EBX;
    pdVar5 = DataMapInit<CPropTractorBeamProjector>((CPropTractorBeamProjector *)0x0);
    *(datamap_t **)((int)DataMapInit<CAI_Agent>::dataDesc[4].flatOffset + unaff_EBX) = pdVar5;
    *(undefined **)(g_TokenProcessor.m_szToken + unaff_EBX + 0x1ec) = &UNK_00c1ffec + unaff_EBX;
    pIVar6 = EntityFactoryDictionary();
    (**pIVar6->_vptr_IEntityFactoryDictionary)(pIVar6,unaff_EBX + 0xd97914,unaff_EBX + 0x9fc860);
    iVar8 = unaff_EBX + 0xd97918;
    s2 = (char *)(unaff_EBX + 0x9fc846);
    *(char **)(g_TokenProcessor.m_szToken + unaff_EBX + 0x1f0) = s2;
    this = (SendTable *)((int)DataMapInit<CAI_Agent>::dataDesc[4].flatOffset + unaff_EBX + 4);
    *(SendTable **)(g_TokenProcessor.m_szToken + unaff_EBX + 500) = this;
    pcVar1 = g_TokenProcessor.m_szToken + unaff_EBX + 0x200;
    pcVar1[0] = -1;
    pcVar1[1] = -1;
    pcVar1[2] = '\0';
    pcVar1[3] = '\0';
    piVar2 = *(int **)(&DAT_00b8daf0 + unaff_EBX);
    puVar3 = (undefined4 *)*piVar2;
    if (puVar3 == (undefined4 *)0x0) {
      *piVar2 = iVar8;
      pcVar1 = g_TokenProcessor.m_szToken + unaff_EBX + 0x1f8;
      pcVar1[0] = '\0';
      pcVar1[1] = '\0';
      pcVar1[2] = '\0';
      pcVar1[3] = '\0';
    }
    else {
      puVar4 = (undefined4 *)puVar3[2];
      iVar7 = _V_stricmp((char *)*puVar3,s2);
      if (iVar7 < 1) {
        while ((puVar4 != (undefined4 *)0x0 && (iVar7 = _V_stricmp((char *)*puVar4,s2), iVar7 < 1)))
        {
          puVar3 = puVar4;
          puVar4 = (undefined4 *)puVar4[2];
        }
        *(undefined4 **)(g_TokenProcessor.m_szToken + unaff_EBX + 0x1f8) = puVar4;
        puVar3[2] = iVar8;
      }
      else {
        *(int *)(g_TokenProcessor.m_szToken + unaff_EBX + 0x1f8) = *piVar2;
        *piVar2 = iVar8;
      }
    }
    SendTable::SendTable(this);
    ___cxa_atexit(unaff_EBX + 0x98797c,0,*(undefined4 *)(&DAT_00b8da50 + unaff_EBX));
    iVar8 = ServerClassInit<DT_PropTractorBeamProjector::ignored>((ignored *)0x0);
    *(int *)((int)DataMapInit<CAI_Agent>::dataDesc[5].flatOffset + unaff_EBX + -0x28) = iVar8;
  }
  return;
}


/* CPropTractorBeamProjector::GetDataDescMap at 0076dcc0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPropTractorBeamProjector * this) */

datamap_t * __thiscall CPropTractorBeamProjector::GetDataDescMap(CPropTractorBeamProjector *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(&UNK_005fd2f8 + extraout_ECX);
}


/* CPropTractorBeamProjector::GetBaseMap at 0076dcd0 */

datamap_t * CPropTractorBeamProjector::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x4d9a04);
}


/* __tcf_0 at 00a40aa0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x410156)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x41014a) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x410156));
  }
  *(undefined4 *)(unaff_EBX + 0x410156) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x410152)) {
    if (*(int *)(unaff_EBX + 0x41014a) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2060ae) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2060ae),*(int *)(unaff_EBX + 0x41014a));
      *(undefined4 *)(unaff_EBX + 0x41014a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41014e) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x41014a);
  *(int *)(unaff_EBX + 0x41015a) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x410152)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2060ae) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2060ae),iVar1);
      *(undefined4 *)(unaff_EBX + 0x41014a) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x41014e) = 0;
  }
  return;
}


/* ServerClassInit<DT_PropTractorBeamProjector::ignored> at 000b8da0 */

int ServerClassInit<DT_PropTractorBeamProjector::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: SendTable * sendTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&sv_soundemitter_trace.super_ConCommandBase._vptr_ConCommandBase +
               unaff_EBX + 2) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xd97c82);
    if (iVar1 != 0) {
      SendPropInt((SendProp_conflict *)
                  ((int)&sv_soundemitter_trace.super_ConCommandBase.m_pszHelpString + unaff_EBX + 2)
                  ,(char *)(unaff_EBX + 0x99ce86),0,4,-1,0,(SendVarProxyFn_conflict)0x0,0x80);
      SendPropDataTable((SendProp_conflict *)
                        ((int)&cc_showmissing.super_ConCommandBase.m_pNext + unaff_EBX + 2),
                        (char *)(unaff_EBX + 0x99f236),0,
                        (SendTable *)**(undefined4 **)(&DAT_00b8de36 + unaff_EBX),
                        *(SendTableProxyFn_conflict *)(&DAT_00b8de42 + unaff_EBX),0x80);
      SendPropFloat((SendProp_conflict *)(unaff_EBX + 0xd97d3a),&UNK_009fcb7f + unaff_EBX,0x520,4,0,
                    4,0.0,-121121.125,*(SendVarProxyFn_conflict *)(&DAT_00b8de9a + unaff_EBX),0x80);
      SendPropVector((SendProp_conflict *)
                     (CSoundPatch::s_Allocator.m_BlobHead.m_Padding + unaff_EBX + 1),
                     (char *)(unaff_EBX + 0x9fcb8f),0x524,0xc,0,4,0.0,-121121.125,
                     *(SendVarProxyFn_conflict *)(&DAT_00b8de8e + unaff_EBX),0x80);
      SendPropBool((SendProp_conflict *)
                   ((int)&DT_Sprite::g_SendTable.m_pNetTableName + unaff_EBX + 2),
                   (char *)(unaff_EBX + 0x9f675a),0x514,1);
      ___cxa_guard_release(unaff_EBX + 0xd97c82);
      ___cxa_atexit(unaff_EBX + 0x987c62,0,*(undefined4 *)(&DAT_00b8dda6 + unaff_EBX));
    }
  }
  SendTable::Construct
            ((SendTable *)
             ((int)DataMapInit<CAI_BaseActor>::dataDesc[3].flatOffset + unaff_EBX + -0x26),
             (SendProp_conflict *)
             ((int)&cc_showmissing.super_ConCommandBase.m_pNext + unaff_EBX + 2),4,
             *(char **)(&DAT_00c20332 + unaff_EBX));
  return 1;
}


/* CPropTractorBeamProjector::GetServerClass at 0076dce0 */

/* DWARF original prototype: ServerClass * GetServerClass(CPropTractorBeamProjector * this) */

ServerClass * __thiscall CPropTractorBeamProjector::GetServerClass(CPropTractorBeamProjector *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ServerClass *)(extraout_ECX + 0x6e2d34);
}


/* CPropTractorBeamProjector::YouForgotToImplementOrDeclareServerClass at 0076dcf0 */

/* DWARF original prototype: int YouForgotToImplementOrDeclareServerClass(CPropTractorBeamProjector
   * this) */

int __thiscall
CPropTractorBeamProjector::YouForgotToImplementOrDeclareServerClass(CPropTractorBeamProjector *this)

{
  return 0;
}


/* __tcf_2 at 00a40a80 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  SendTable::~SendTable((SendTable *)(&UNK_00389934 + extraout_ECX),in_stack_00000008);
  return;
}


/* __tcf_1 at 00a40a10 */

void __tcf_1(void *param_1)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410173))(unaff_EBX + 0x410173);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x41011f))(unaff_EBX + 0x41011f);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x4100cb))(unaff_EBX + 0x4100cb);
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410077))(unaff_EBX + 0x410077);
                    /* WARNING: Could not recover jumptable at 0x00a40a72. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)**(undefined4 **)(unaff_EBX + 0x410023))();
  return;
}


/* CPropTractorBeamProjector::Spawn at 0076dd60 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void Spawn(CPropTractorBeamProjector * this) */

void __thiscall CPropTractorBeamProjector::Spawn(CPropTractorBeamProjector *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseProjector::Spawn(&this->super_CBaseProjector);
  (*(this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1a])(this);
  (*(this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0x1b])
            (this,&UNK_00347b73 + unaff_EBX);
  CCollisionProperty::SetSolid
            (&(this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.m_Collision.
              super_CCollisionProperty,SOLID_VPHYSICS);
  CBaseAnimating::ResetSequence((CBaseAnimating *)this,2);
  CBaseAnimating::UseClientSideAnimation((CBaseAnimating *)this);
  return;
}


/* CPropTractorBeamProjector::Precache at 0076dd20 */

/* DWARF original prototype: void Precache(CPropTractorBeamProjector * this) */

void __thiscall CPropTractorBeamProjector::Precache(CPropTractorBeamProjector *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::PrecacheModel(&UNK_00347bb8 + unaff_EBX);
  PrecacheParticleSystem(&UNK_00347bde + unaff_EBX);
  PrecacheParticleSystem(&UNK_00347bef + unaff_EBX);
  return;
}


/* CPropTractorBeamProjector::Activate at 0076dd10 */

/* DWARF original prototype: void Activate(CPropTractorBeamProjector * this) */

void __thiscall CPropTractorBeamProjector::Activate(CPropTractorBeamProjector *this)

{
  CBaseProjector::Activate(&this->super_CBaseProjector);
  return;
}


/* CPropTractorBeamProjector::CreateNewProjectedEntity at 0076dd00 */

/* DWARF original prototype: CBaseProjectedEntity *
   CreateNewProjectedEntity(CPropTractorBeamProjector * this) */

CBaseProjectedEntity * __thiscall
CPropTractorBeamProjector::CreateNewProjectedEntity(CPropTractorBeamProjector *this)

{
  CProjectedTractorBeamEntity *pCVar1;
  
  pCVar1 = CProjectedTractorBeamEntity::CreateNewInstance();
  return &pCVar1->super_CBaseProjectedEntity;
}


/* CPropTractorBeamProjector::InputSetLinearForce at 0076e020 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Restarted to delay deadcode elimination for space: stack */
/* DWARF original prototype: void InputSetLinearForce(CPropTractorBeamProjector * this, inputdata_t
   * indata) */

void __thiscall
CPropTractorBeamProjector::InputSetLinearForce(CPropTractorBeamProjector *this,inputdata_t *indata)

{
  undefined1 *puVar1;
  vec_t *pvVar2;
  int iVar3;
  ushort uVar4;
  float *pfVar5;
  float fVar6;
  CBaseEdict *pCVar7;
  ushort *puVar8;
  int *piVar9;
  IChangeInfoAccessor *pIVar10;
  uint uVar11;
  uint uVar12;
  int unaff_EBX;
  float fVar13;
  float fVar14;
  uint local_94;
  uint local_90;
  
  ___i686_get_pc_thunk_bx();
  fVar14 = 0.0;
  if ((indata->value).fieldType == FIELD_FLOAT) {
    fVar14 = (indata->value).field_0.flVal;
  }
  fVar13 = (this->m_flLinearForce).m_Value;
  if (fVar14 != fVar13) {
    if ((this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent.
        m_bRegistered == false) {
      pCVar7 = &((this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.m_Network.
                m_pPev)->super_CBaseEdict;
      if ((pCVar7 != (CBaseEdict *)0x0) && ((pCVar7->m_fStateFlags & 0x100U) == 0)) {
        pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 1;
        pIVar10 = CBaseEdict::GetChangeAccessor(pCVar7);
        puVar8 = (ushort *)**(undefined4 **)(CPushable::~CPushable + unaff_EBX + 1);
        if (pIVar10->m_iChangeInfoSerialNumber == *puVar8) {
          uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar4 = puVar8[uVar11 * 0x14 + 0x14];
          if (uVar4 == 0) {
LAB_0076e341:
            puVar8[(uint)uVar4 + uVar11 * 0x14 + 1] = 0x520;
            puVar8[uVar11 * 0x14 + 0x14] = uVar4 + 1;
          }
          else if (puVar8[uVar11 * 0x14 + 1] != 0x520) {
            local_94 = 0;
            do {
              uVar12 = local_94 + 1;
              local_94 = uVar12 & 0xffff;
              if ((ushort)uVar12 == uVar4) {
                if (uVar4 != 0x13) goto LAB_0076e341;
                pIVar10->m_iChangeInfoSerialNumber = 0;
                pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar8[uVar11 * 0x14 + local_94 + 1] != 0x520);
          }
        }
        else if (puVar8[0x7d1] == 100) {
          pIVar10->m_iChangeInfoSerialNumber = 0;
          pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar10->m_iChangeInfo = puVar8[0x7d1];
          *(short *)(**(int **)(CPushable::~CPushable + unaff_EBX + 1) + 0xfa2) =
               *(short *)(**(int **)(CPushable::~CPushable + unaff_EBX + 1) + 0xfa2) + 1;
          piVar9 = *(int **)(CPushable::~CPushable + unaff_EBX + 1);
          pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar9;
          iVar3 = *piVar9 + (uint)pIVar10->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar3 + 2) = 0x520;
          *(undefined2 *)(iVar3 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.m_Network.
                field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->m_flLinearForce).m_Value = fVar14;
    fVar13 = fVar14;
  }
  if ((fVar13 != 0.0) || (NAN(fVar13))) {
    (*(this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.super_IServerEntity.
      super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xe9])(this);
  }
  else {
    CBaseProjector::EnableProjection(&this->super_CBaseProjector,false);
    pfVar5 = *(float **)(unaff_EBX + 0x4d8b41);
    fVar14 = *pfVar5;
    fVar13 = pfVar5[1];
    fVar6 = pfVar5[2];
    if ((((fVar14 != (this->m_vEndPos).m_Value.x) ||
         (NAN(fVar14) || NAN((this->m_vEndPos).m_Value.x))) ||
        (pvVar2 = &(this->m_vEndPos).m_Value.y, fVar13 != *pvVar2)) ||
       ((NAN(fVar13) || NAN(*pvVar2) || (fVar6 != (this->m_vEndPos).m_Value.z)))) {
      if ((this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent
          .m_bRegistered == false) {
        pCVar7 = &((this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.m_Network.
                  m_pPev)->super_CBaseEdict;
        if ((pCVar7 != (CBaseEdict *)0x0) && ((pCVar7->m_fStateFlags & 0x100U) == 0)) {
          pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 1;
          pIVar10 = CBaseEdict::GetChangeAccessor(pCVar7);
          puVar8 = (ushort *)**(undefined4 **)(CPushable::~CPushable + unaff_EBX + 1);
          if (pIVar10->m_iChangeInfoSerialNumber == *puVar8) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
            uVar4 = puVar8[uVar11 * 0x14 + 0x14];
            if (uVar4 == 0) {
LAB_0076e3ed:
              puVar8[(uint)uVar4 + uVar11 * 0x14 + 1] = 0x524;
              puVar8[uVar11 * 0x14 + 0x14] = uVar4 + 1;
            }
            else if (puVar8[uVar11 * 0x14 + 1] != 0x524) {
              local_90 = 0;
              do {
                uVar12 = local_90 + 1;
                local_90 = uVar12 & 0xffff;
                if ((ushort)uVar12 == uVar4) {
                  if (uVar4 != 0x13) goto LAB_0076e3ed;
                  goto LAB_0076e412;
                }
              } while (puVar8[uVar11 * 0x14 + local_90 + 1] != 0x524);
            }
          }
          else if (puVar8[0x7d1] == 100) {
LAB_0076e412:
            pIVar10->m_iChangeInfoSerialNumber = 0;
            pCVar7->m_fStateFlags = pCVar7->m_fStateFlags | 0x100;
          }
          else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
            pIVar10->m_iChangeInfo = puVar8[0x7d1];
            *(short *)(**(int **)(CPushable::~CPushable + unaff_EBX + 1) + 0xfa2) =
                 *(short *)(**(int **)(CPushable::~CPushable + unaff_EBX + 1) + 0xfa2) + 1;
            piVar9 = *(int **)(CPushable::~CPushable + unaff_EBX + 1);
            pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar9;
            iVar3 = *piVar9 + (uint)pIVar10->m_iChangeInfo * 0x28;
            *(undefined2 *)(iVar3 + 2) = 0x524;
            *(undefined2 *)(iVar3 + 0x28) = 1;
          }
        }
      }
      else {
        puVar1 = &(this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.m_Network.
                  field_0x4c;
        *(uint *)puVar1 = *(uint *)puVar1 | 1;
      }
      (this->m_vEndPos).m_Value.x = fVar14;
      (this->m_vEndPos).m_Value.y = fVar13;
      (this->m_vEndPos).m_Value.z = fVar6;
    }
  }
  return;
}


/* CPropTractorBeamProjector::Project at 0076ddd0 */

/* WARNING: Restarted to delay deadcode elimination for space: stack */
/* DWARF original prototype: void Project(CPropTractorBeamProjector * this) */

void __thiscall CPropTractorBeamProjector::Project(CPropTractorBeamProjector *this)

{
  undefined1 *puVar1;
  vec_t *pvVar2;
  ushort uVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  CBaseEdict *this_00;
  ushort *puVar7;
  int iVar8;
  float *pfVar9;
  IChangeInfoAccessor *pIVar10;
  uint uVar11;
  uint uVar12;
  int *piVar13;
  int unaff_EBX;
  uint local_70;
  
  ___i686_get_pc_thunk_bx();
  CBaseProjector::Project(&this->super_CBaseProjector);
  uVar11 = (this->super_CBaseProjector).m_hFirstChild.super_CBaseHandle.m_Index;
  if ((uVar11 == 0xffffffff) ||
     (iVar8 = (uVar11 & 0xffff) * 0x10 + **(int **)(&DAT_004d8d8e + unaff_EBX),
     *(uint *)(iVar8 + 8) != uVar11 >> 0x10)) {
    piVar13 = (int *)0x0;
  }
  else {
    piVar13 = *(int **)(iVar8 + 4);
  }
  pfVar9 = (float *)(**(code **)(*piVar13 + 0x330))(piVar13);
  fVar4 = *pfVar9;
  fVar5 = pfVar9[1];
  fVar6 = pfVar9[2];
  if ((fVar4 == (this->m_vEndPos).m_Value.x) && (!NAN(fVar4) && !NAN((this->m_vEndPos).m_Value.x)))
  {
    pvVar2 = &(this->m_vEndPos).m_Value.y;
    if ((fVar5 == *pvVar2) &&
       ((!NAN(fVar5) && !NAN(*pvVar2) && (fVar6 == (this->m_vEndPos).m_Value.z)))) {
      return;
    }
  }
  if ((this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.m_Network.m_TimerEvent.
      m_bRegistered == false) {
    this_00 = &((this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.m_Network.m_pPev
               )->super_CBaseEdict;
    if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
      this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
      pIVar10 = CBaseEdict::GetChangeAccessor(this_00);
      puVar7 = (ushort *)**(undefined4 **)(&DAT_004d8d8a + unaff_EBX);
      if (pIVar10->m_iChangeInfoSerialNumber == *puVar7) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        uVar11 = (uint)pIVar10->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
        uVar3 = puVar7[uVar11 * 0x14 + 0x14];
        if (uVar3 == 0) {
LAB_0076dfe4:
          puVar7[(uint)uVar3 + uVar11 * 0x14 + 1] = 0x524;
          puVar7[uVar11 * 0x14 + 0x14] = uVar3 + 1;
        }
        else if (puVar7[uVar11 * 0x14 + 1] != 0x524) {
          local_70 = 0;
          do {
            uVar12 = local_70 + 1;
            local_70 = uVar12 & 0xffff;
            if ((ushort)uVar12 == uVar3) {
              if (uVar3 == 0x13) goto LAB_0076e009;
              goto LAB_0076dfe4;
            }
          } while (puVar7[uVar11 * 0x14 + local_70 + 1] != 0x524);
        }
      }
      else if (puVar7[0x7d1] == 100) {
LAB_0076e009:
        pIVar10->m_iChangeInfoSerialNumber = 0;
        this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
      }
      else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
        pIVar10->m_iChangeInfo = puVar7[0x7d1];
        *(short *)(**(int **)(&DAT_004d8d8a + unaff_EBX) + 0xfa2) =
             *(short *)(**(int **)(&DAT_004d8d8a + unaff_EBX) + 0xfa2) + 1;
        piVar13 = *(int **)(&DAT_004d8d8a + unaff_EBX);
        pIVar10->m_iChangeInfoSerialNumber = *(ushort *)*piVar13;
        iVar8 = *piVar13 + (uint)pIVar10->m_iChangeInfo * 0x28;
        *(undefined2 *)(iVar8 + 2) = 0x524;
        *(undefined2 *)(iVar8 + 0x28) = 1;
      }
    }
  }
  else {
    puVar1 = &(this->super_CBaseProjector).super_CBaseAnimating.super_CBaseEntity.m_Network.
              field_0x4c;
    *(uint *)puVar1 = *(uint *)puVar1 | 1;
  }
  (this->m_vEndPos).m_Value.x = fVar4;
  (this->m_vEndPos).m_Value.y = fVar5;
  (this->m_vEndPos).m_Value.z = fVar6;
  return;
}


/* _GLOBAL__I__ZN25CPropTractorBeamProjector9m_DataMapE at 000b92e0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN25CPropTractorBeamProjector9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

