/* DWARF-guided pseudocode for game/client/portal/c_func_portal_bumper.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* ClientClassInit<DT_FuncPortalBumper::ignored> at 00069c80 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

int ClientClassInit<DT_FuncPortalBumper::ignored>(ignored *param_1)

{
  int iVar1;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9f6].m_pEntity + unaff_EBX)
      == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc480c4);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9f6].m_pPrev + unaff_EBX
                  ),&UNK_008f60cc + unaff_EBX,0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9fa].
                               m_SerialNumber + unaff_EBX),(char *)(unaff_EBX + 0x8facac),0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00ac7150 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00ac710c + unaff_EBX));
      RecvPropBool((RecvProp *)
                   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9fe].m_pEntity +
                   unaff_EBX),(char *)(unaff_EBX + 0x910740),0xa59,1);
      ___cxa_guard_release(unaff_EBX + 0xc480c4);
    }
  }
  RecvTable::Construct
            ((RecvTable *)(s_SortBuffer + unaff_EBX + 0x50a4),
             (RecvProp_conflict1 *)
             ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9fa].m_SerialNumber +
             unaff_EBX),2,&UNK_009217fb + unaff_EBX);
  return 1;
}


/* __static_initialization_and_destruction_0 at 00069dd0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  uint8 *puVar1;
  int *piVar2;
  int iVar3;
  int unaff_EBX;
  longlong lVar4;
  
  lVar4 = ___i686_get_pc_thunk_bx();
  if (lVar4 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9dd].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9dd].m_pNext + unaff_EBX)
       = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9dd].m_pNext + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9dd].m_pNext + unaff_EBX + 2) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9dd].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9de].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9de].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9de].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9de].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9df].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9df].m_SerialNumber + unaff_EBX + 3) =
       0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9df].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9df].m_pNext + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9e0].m_pEntity + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9e0].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9e0].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined **)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x9e0].m_pNext + unaff_EBX + 3) =
       &UNK_00ac8027 + unaff_EBX;
  *(int *)(s_SortBuffer + unaff_EBX + 0x4f47) = unaff_EBX + 0x9216c2;
  *(int *)(s_SortBuffer + unaff_EBX + 0x4f3f) = unaff_EBX + 0x46bc7f;
  puVar1 = s_SortBuffer + unaff_EBX + 0x4f43;
  puVar1[0] = '\0';
  puVar1[1] = '\0';
  puVar1[2] = '\0';
  puVar1[3] = '\0';
  *(RecvTable **)(s_SortBuffer + unaff_EBX + 0x4f4b) =
       (RecvTable *)(s_SortBuffer + unaff_EBX + 0x4f57);
  piVar2 = *(int **)(&DAT_00ac6fb7 + unaff_EBX);
  *(int *)(s_SortBuffer + unaff_EBX + 0x4f4f) = *piVar2;
  *piVar2 = (int)(s_SortBuffer + unaff_EBX + 0x4f3f);
  RecvTable::RecvTable((RecvTable *)(s_SortBuffer + unaff_EBX + 0x4f57));
  ___cxa_atexit(unaff_EBX + 0x8e5dff,0,*(undefined4 *)(&DAT_00ac6f3f + unaff_EBX));
  iVar3 = ClientClassInit<DT_FuncPortalBumper::ignored>((ignored *)0x0);
  *(int *)(s_SortBuffer + unaff_EBX + 0x4f6b) = iVar3;
  return;
}


/* C_FuncPortalBumper::YouForgotToImplementOrDeclareClientClass at 004d5a30 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_FuncPortalBumper": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_FuncPortalBumper * this)
    */

int __thiscall
C_FuncPortalBumper::YouForgotToImplementOrDeclareClientClass(C_FuncPortalBumper *this)

{
  return 0;
}


/* C_FuncPortalBumper::GetClientClass at 004d5a50 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_FuncPortalBumper": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: ClientClass * GetClientClass(C_FuncPortalBumper * this) */

ClientClass * __thiscall C_FuncPortalBumper::GetClientClass(C_FuncPortalBumper *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x877948);
}


/* _C_FuncPortalBumper_CreateObject at 004d5a60 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_FuncPortalBumper_CreateObject(int entnum,int serialNum)

{
  C_BaseEntity *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_FuncPortalBumper * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xa5c);
  C_BaseEntity::C_BaseEntity(this);
  (this->super_IClientEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x6a98d4);
  (this->super_IClientEntity).super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6a9c6c);
  (this->super_IClientEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6a9d24);
  (this->super_IClientEntity).super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6a9d60);
  (**(code **)(unaff_EBX + 0x6a9990))(this,entnum,serialNum);
  return &(this->super_IClientEntity).super_IClientNetworkable;
}


/* __tcf_0 at 0094fbe0 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3fd7d0),in_stack_00000008);
  return;
}


/* _GLOBAL__I__ZN18C_FuncPortalBumper17m_pClassRecvTableE at 00069ef0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN18C_FuncPortalBumper17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

