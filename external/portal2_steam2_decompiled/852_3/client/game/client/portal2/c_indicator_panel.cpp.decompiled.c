/* DWARF-guided pseudocode for game/client/portal2/c_indicator_panel.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* ClientClassInit<DT_LabIndicatorPanel::ignored> at 00073a80 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

int ClientClassInit<DT_LabIndicatorPanel::ignored>(ignored *param_1)

{
  RecvVarProxyFn varProxy;
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x582].m_SerialNumber +
               unaff_EBX + 2) == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc4398a);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x582].m_pNext +
                  unaff_EBX + 2),&UNK_008ec2d2 + unaff_EBX,0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x586].m_pPrev +
                        unaff_EBX + 2),(char *)(unaff_EBX + 0x8f0eb2),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00abd356 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00abd312 + unaff_EBX));
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x58a].m_SerialNumber +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0x904976),0xa59,1);
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x58e].m_pEntity +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0x8f96e2),0xa5a,1);
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x591].m_pNext +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0x91b08d),0xa5b,1);
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x595].m_pPrev +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0x91b09d),0xa5c,1);
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x599].m_SerialNumber +
                   unaff_EBX + 2),(char *)(unaff_EBX + 0x91b0b1),0xa5d,1);
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x59d].m_pEntity +
                  unaff_EBX + 2),(char *)(unaff_EBX + 0x91b0be),0xa60,4,0,(RecvVarProxyFn)0x0);
      varProxy = *(RecvVarProxyFn *)(&DAT_00abd352 + unaff_EBX);
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x5a0].m_pNext +
                    unaff_EBX + 2),&UNK_0091b0ce + unaff_EBX,0xa64,4,0,varProxy);
      RecvPropFloat((RecvProp_conflict1 *)
                    (&UNK_00005a4e +
                    (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4),
                    (char *)(unaff_EBX + 0x91b0e0),0xa68,4,0,varProxy);
      ___cxa_guard_release(unaff_EBX + 0xc4398a);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)s_pParticlePtrs + (int)(&UNK_0000175a + unaff_EBX)),
             (RecvProp_conflict1 *)
             ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x586].m_pPrev + unaff_EBX + 2)
             ,9,(char *)(unaff_EBX + 0x91b0ef));
  return 1;
}


/* __static_initialization_and_destruction_0 at 00073d20 */

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
  (&UNK_0000554f)[(int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
  (&UNK_00005550)[(int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
  (&UNK_00005551)[(int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
  (&UNK_00005552)[(int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
  *(undefined4 *)
   (&UNK_00005553 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_00005557 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_0000555b + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       0x7f7fffff;
  *(undefined4 *)
   (&UNK_0000555f + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       0x7f7fffff;
  *(undefined4 *)
   (&UNK_00005563 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_00005567 + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_0000556b + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_0000556f + (int)s_EntityList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&UNK_0000556f + (int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0].m_pEntity + unaff_EBX)
       = 0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x557].m_pEntity + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x557].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x557].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined **)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x557].m_pNext + unaff_EBX + 3) =
       &UNK_00abe0d7 + unaff_EBX;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x14a7) = unaff_EBX + 0x91ae61;
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x149f) = unaff_EBX + 0x4ad4df;
  *(undefined4 *)((int)s_pParticlePtrs + unaff_EBX + 0x14a3) = 0;
  this = (RecvTable *)((int)s_pParticlePtrs + unaff_EBX + 0x14b7);
  *(RecvTable **)((int)s_pParticlePtrs + unaff_EBX + 0x14ab) = this;
  piVar1 = *(int **)(&DAT_00abd067 + unaff_EBX);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x14af) = *piVar1;
  *piVar1 = unaff_EBX + 0xcdacff;
  RecvTable::RecvTable(this);
  ___cxa_atexit(unaff_EBX + 0x8dd3df,0,*(undefined4 *)(&DAT_00abcfef + unaff_EBX));
  iVar2 = ClientClassInit<DT_LabIndicatorPanel::ignored>((ignored *)0x0);
  *(int *)((int)s_pParticlePtrs + unaff_EBX + 0x14cb) = iVar2;
  return;
}


/* C_LabIndicatorPanel::YouForgotToImplementOrDeclareClientClass at 00521130 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_LabIndicatorPanel": ignoring multiple overlapping fields */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_LabIndicatorPanel *
   this) */

int __thiscall
C_LabIndicatorPanel::YouForgotToImplementOrDeclareClientClass(C_LabIndicatorPanel *this)

{
  return 0;
}


/* C_LabIndicatorPanel::GetClientClass at 00521150 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_LabIndicatorPanel": ignoring multiple overlapping fields */
/* DWARF original prototype: ClientClass * GetClientClass(C_LabIndicatorPanel * this) */

ClientClass * __thiscall C_LabIndicatorPanel::GetClientClass(C_LabIndicatorPanel *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x82d8d8);
}


/* _C_LabIndicatorPanel_CreateObject at 00521210 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_LabIndicatorPanel_CreateObject(int entnum,int serialNum)

{
  C_BaseEntity *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_LabIndicatorPanel * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xa6c);
  C_BaseEntity::C_BaseEntity(this);
  (this->super_IClientEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x6736a4);
  (this->super_IClientEntity).super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(&UNK_00673a3c + unaff_EBX);
  (this->super_IClientEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x673af4);
  (this->super_IClientEntity).super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x673b30);
  (**(code **)(unaff_EBX + 0x673760))(this,entnum,serialNum);
  return &(this->super_IClientEntity).super_IClientNetworkable;
}


/* __tcf_0 at 00951110 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3fd930),in_stack_00000008);
  return;
}


/* C_LabIndicatorPanel::GetCountdownPercentage at 00521160 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_LabIndicatorPanel": ignoring multiple overlapping fields */
/* DWARF original prototype: float GetCountdownPercentage(C_LabIndicatorPanel * this) */

float __thiscall C_LabIndicatorPanel::GetCountdownPercentage(C_LabIndicatorPanel *this)

{
  float fVar1;
  int extraout_ECX;
  float fVar2;
  float fVar3;
  
  ___i686_get_pc_thunk_cx();
  if ((this->super_C_BaseEntity).field_0xa5b == '\0') {
    return 1.0;
  }
  fVar3 = this->m_flTimerStart;
  fVar2 = fVar3 + this->m_flTimerDuration;
  fVar1 = *(float *)(**(int **)(extraout_ECX + 0x60fbe5) + 0xc);
  if ((fVar3 != fVar2) || (NAN(fVar3) || NAN(fVar2))) {
    fVar3 = (fVar1 - fVar3) / (fVar2 - fVar3);
    if ((fVar3 < *(float *)(extraout_ECX + 0x4957e9)) &&
       (!NAN(fVar3) && !NAN(*(float *)(extraout_ECX + 0x4957e9)))) {
      return 1.0;
    }
    if (fVar3 <= *(float *)(extraout_ECX + 0x4956d9)) {
      return *(float *)(extraout_ECX + 0x4956d9) - fVar3;
    }
  }
  else if (fVar1 - fVar2 < 0.0) {
    return 1.0;
  }
  return 0.0;
}


/* _GLOBAL__I__ZN19C_LabIndicatorPanel17m_pClassRecvTableE at 00073e40 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN19C_LabIndicatorPanel17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

