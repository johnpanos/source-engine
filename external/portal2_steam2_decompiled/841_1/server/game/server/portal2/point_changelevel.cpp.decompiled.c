/* DWARF-guided pseudocode for game/server/portal2/point_changelevel.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* CEntityFactory<CInfoLandmark>::Create at 0073fbb0 */

/* WARNING: Enum "Disposition_t": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CInfoLandmark> * this, char
   * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CInfoLandmark>::Create(CEntityFactory<CInfoLandmark> *this,char *pClassName)

{
  CBaseEntity *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CInfoLandmark * pEnt@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CBaseEntity::operator_new(0x3c0);
  CBaseEntity::CBaseEntity(this_00,false);
  (this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x58eca4);
  CBaseEntity::PostConstructor(this_00,pClassName);
  return &(this_00->m_Network).super_IServerNetworkable;
}


/* CInfoLandmark::~CInfoLandmark at 0073fc20 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CInfoLandmark(CInfoLandmark * this, int __in_chrg) */

void __thiscall CInfoLandmark::~CInfoLandmark(CInfoLandmark *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x58ec37);
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x50705f) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,in_stack_ffffffe8);
  CBaseEntity::operator_delete(this);
  return;
}


/* CInfoLandmark::~CInfoLandmark at 0073fce0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CInfoLandmark(CInfoLandmark * this, int __in_chrg) */

void __thiscall CInfoLandmark::~CInfoLandmark(CInfoLandmark *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(extraout_ECX + 0x58eb80);
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(extraout_ECX + 0x506fa8) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,__in_chrg);
  return;
}


/* CInfoLandmark::Activate at 0073f960 */

/* DWARF original prototype: void Activate(CInfoLandmark * this) */

void __thiscall CInfoLandmark::Activate(CInfoLandmark *this)

{
  undefined1 *puVar1;
  int iVar2;
  ushort uVar3;
  CBaseEdict *this_00;
  ushort *puVar4;
  int *piVar5;
  string_t *psVar6;
  IChangeInfoAccessor *pIVar7;
  uint uVar8;
  uint uVar9;
  int unaff_EBX;
  char *in_stack_ffffff88;
  uint local_60;
  
  ___i686_get_pc_thunk_bx();
  CBaseEntity::Activate((CBaseEntity *)this);
  psVar6 = AllocPooledString((string_t *)(unaff_EBX + 0x37302c),in_stack_ffffff88);
  if (psVar6 != (string_t *)(this->super_CPointEntity).super_CBaseEntity.m_iName.m_Value.pszValue) {
    if ((this->super_CPointEntity).super_CBaseEntity.m_Network.m_TimerEvent.m_bRegistered == false)
    {
      this_00 = &((this->super_CPointEntity).super_CBaseEntity.m_Network.m_pPev)->super_CBaseEdict;
      if ((this_00 != (CBaseEdict *)0x0) && ((this_00->m_fStateFlags & 0x100U) == 0)) {
        this_00->m_fStateFlags = this_00->m_fStateFlags | 1;
        pIVar7 = CBaseEdict::GetChangeAccessor(this_00);
        puVar4 = (ushort *)**(undefined4 **)(unaff_EBX + 0x5071f4);
        if (pIVar7->m_iChangeInfoSerialNumber == *puVar4) {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          uVar8 = (uint)pIVar7->m_iChangeInfo;
                    /* Unresolved local var: ushort i@[???] */
          uVar3 = puVar4[uVar8 * 0x14 + 0x14];
          if (uVar3 == 0) {
LAB_0073fae4:
            puVar4[(uint)uVar3 + uVar8 * 0x14 + 1] = 0x134;
            puVar4[uVar8 * 0x14 + 0x14] = uVar3 + 1;
          }
          else if (puVar4[uVar8 * 0x14 + 1] != 0x134) {
            local_60 = 0;
            do {
              uVar9 = local_60 + 1;
              local_60 = uVar9 & 0xffff;
              if ((ushort)uVar9 == uVar3) {
                if (uVar3 != 0x13) goto LAB_0073fae4;
                pIVar7->m_iChangeInfoSerialNumber = 0;
                this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
                break;
              }
            } while (puVar4[uVar8 * 0x14 + local_60 + 1] != 0x134);
          }
        }
        else if (puVar4[0x7d1] == 100) {
          pIVar7->m_iChangeInfoSerialNumber = 0;
          this_00->m_fStateFlags = this_00->m_fStateFlags | 0x100;
        }
        else {
                    /* Unresolved local var: CEdictChangeInfo * p@[???] */
          pIVar7->m_iChangeInfo = puVar4[0x7d1];
          *(short *)(**(int **)(unaff_EBX + 0x5071f4) + 0xfa2) =
               *(short *)(**(int **)(unaff_EBX + 0x5071f4) + 0xfa2) + 1;
          piVar5 = *(int **)(unaff_EBX + 0x5071f4);
          pIVar7->m_iChangeInfoSerialNumber = *(ushort *)*piVar5;
          iVar2 = *piVar5 + (uint)pIVar7->m_iChangeInfo * 0x28;
          *(undefined2 *)(iVar2 + 2) = 0x134;
          *(undefined2 *)(iVar2 + 0x28) = 1;
        }
      }
    }
    else {
      puVar1 = &(this->super_CPointEntity).super_CBaseEntity.m_Network.field_0x4c;
      *(uint *)puVar1 = *(uint *)puVar1 | 1;
    }
    (this->super_CPointEntity).super_CBaseEntity.m_iName.m_Value.pszValue = (char *)psVar6;
  }
  return;
}


/* CEntityFactory<CPointChangelevel>::Create at 0073fb20 */

/* DWARF original prototype: IServerNetworkable * Create(CEntityFactory<CPointChangelevel> * this,
   char * pClassName) */

IServerNetworkable * __thiscall
CEntityFactory<CPointChangelevel>::Create(CEntityFactory<CPointChangelevel> *this,char *pClassName)

{
  CBaseEntity *this_00;
  int unaff_EBX;
  
                    /* Unresolved local var: CPointChangelevel * pEnt@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = CBaseEntity::operator_new(0x3d8);
  CBaseEntity::CBaseEntity(this_00,false);
  (this_00->super_IServerEntity).super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x58e9d4);
  this_00[1].super_IServerEntity.super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)0x0;
  this_00[1].m_pfnMoveDone.__delta = -1;
  this_00[1].m_pfnThink.__pfn = (_func___thiscall_void_CBaseEntity_ptr *)0x0;
  (**(code **)(&UNK_0058ea48 + unaff_EBX))(this_00,pClassName);
  return &(this_00->m_Network).super_IServerNetworkable;
}


/* CPointChangelevel::~CPointChangelevel at 0073fc70 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPointChangelevel(CPointChangelevel * this, int __in_chrg) */

void __thiscall CPointChangelevel::~CPointChangelevel(CPointChangelevel *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x58e884);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnChangeLevel).super_CBaseEntityOutput,in_stack_ffffffd8);
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x50700c) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,__in_chrg);
  return;
}


/* CPointChangelevel::~CPointChangelevel at 0073fd10 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: void ~CPointChangelevel(CPointChangelevel * this, int __in_chrg) */

void __thiscall CPointChangelevel::~CPointChangelevel(CPointChangelevel *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffd8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity = (_func_int_varargs **)(unaff_EBX + 0x58e7e4);
  CBaseEntityOutput::~CBaseEntityOutput
            (&(this->m_OnChangeLevel).super_CBaseEntityOutput,in_stack_ffffffd8);
  (this->super_CPointEntity).super_CBaseEntity.super_IServerEntity.super_IServerUnknown.
  super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(*(int *)(unaff_EBX + 0x506f6c) + 8);
  CBaseEntity::~CBaseEntity((CBaseEntity *)this,in_stack_ffffffd8);
  CBaseEntity::operator_delete(this);
  return;
}


/* DataMapInit<CPointChangelevel> at 000af480 */

datamap_t * DataMapInit<CPointChangelevel>(CPointChangelevel *param_1)

{
  int iVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  if ((&DAT_00001324)[(int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xd9ef04);
    if (iVar1 != 0) {
      *(undefined **)
       (&DAT_00001334 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
           &UNK_00a0351a + unaff_EBX;
      *(undefined4 *)
       (&DAT_0000133c + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
      *(undefined4 *)
       (&DAT_00001340 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
      *(undefined4 *)
       (&DAT_00001344 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
      *(undefined4 *)
       (&DAT_00001348 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
      *(undefined4 *)
       (&DAT_0000134c + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
      *(undefined4 *)
       (&DAT_00001338 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0x11;
      ___cxa_guard_release(unaff_EBX + 0xd9ef04);
      ___cxa_atexit(unaff_EBX + 0x98ed0c,0,*(undefined4 *)(&DAT_00b976c0 + unaff_EBX));
    }
  }
  *(undefined4 *)((int)&PTR_FinishMove_00cb6c98 + unaff_EBX) =
       *(undefined4 *)(&DAT_00b977f8 + unaff_EBX);
  if ((&DAT_0000132c)[(int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] == '\0') {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xd9ef0c);
    if (iVar1 != 0) {
      *(undefined4 *)((int)&CLagCompensationManager::typeinfo + unaff_EBX) =
           **(undefined4 **)(&DAT_00b976e0 + unaff_EBX);
      ___cxa_guard_release(unaff_EBX + 0xd9ef0c);
    }
  }
  *(undefined4 *)((int)&PTR_RunCommand_00cb6c90 + unaff_EBX) = 2;
  *(int *)((int)&PTR__CPlayerMove_00cb6c8c + unaff_EBX) = (int)&PTR_OnRestore_00cb6cec + unaff_EBX;
  return (datamap_t *)((int)&PTR__CPlayerMove_00cb6c8c + unaff_EBX);
}


/* __static_initialization_and_destruction_0 at 000af5a0 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  IEntityFactoryDictionary *pIVar1;
  datamap_t *pdVar2;
  int unaff_EBX;
  longlong lVar3;
  
  lVar3 = ___i686_get_pc_thunk_bx();
  if (lVar3 != 0xffff00000001) {
    return;
  }
  (&DAT_000011c4)[(int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
  (&UNK_000011c5)[(int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
  (&UNK_000011c6)[(int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
  (&UNK_000011c7)[(int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4] = 0;
  *(undefined4 *)
   (&DAT_000011c8 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&DAT_000011cc + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&DAT_000011d0 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0x7f7fffff
  ;
  *(undefined4 *)
   (&DAT_000011d4 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0x7f7fffff
  ;
  *(undefined4 *)
   (&DAT_000011d8 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&DAT_000011dc + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&DAT_000011e0 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&DAT_000011e4 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0;
  *(undefined4 *)
   (&DAT_000011e8 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0x7f7fffff
  ;
  *(undefined4 *)
   (&DAT_000011ec + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0x7f7fffff
  ;
  *(undefined4 *)
   (&DAT_000011f0 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0x7f7fffff
  ;
  *(undefined4 *)
   (&DAT_000011f4 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) = 0x7f7fffff
  ;
  *(undefined **)
   (&DAT_000011f8 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       &UNK_00b9cd5c + unaff_EBX;
  *(undefined **)
   (&DAT_000011fc + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       &UNK_00c1f284 + unaff_EBX;
  pIVar1 = EntityFactoryDictionary();
  (**pIVar1->_vptr_IEntityFactoryDictionary)(pIVar1,unaff_EBX + 0xd9eddc,&UNK_00a03414 + unaff_EBX);
  *(undefined **)
   (&DAT_00001200 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       &UNK_00c1f284 + unaff_EBX;
  pIVar1 = EntityFactoryDictionary();
  (**pIVar1->_vptr_IEntityFactoryDictionary)(pIVar1,unaff_EBX + 0xd9ede0,&UNK_00a03428 + unaff_EBX);
  pdVar2 = DataMapInit<CPointChangelevel>((CPointChangelevel *)0x0);
  *(datamap_t **)
   ((int)DataMapInit<CAI_StandoffBehavior>::dataDesc[7].flatOffset + unaff_EBX + -0x2c) = pdVar2;
  *(undefined **)
   (&DAT_00001204 + (int)gEntList.super_CBaseEntityList.m_EntPtrArray + unaff_EBX + -4) =
       &UNK_00c1f5fc + unaff_EBX;
  pIVar1 = EntityFactoryDictionary();
  (**pIVar1->_vptr_IEntityFactoryDictionary)(pIVar1,unaff_EBX + 0xd9ede4,&UNK_00a0343b + unaff_EBX);
  return;
}


/* CPointChangelevel::GetDataDescMap at 0073f7a0 */

/* DWARF original prototype: datamap_t * GetDataDescMap(CPointChangelevel * this) */

datamap_t * __thiscall CPointChangelevel::GetDataDescMap(CPointChangelevel *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x626978);
}


/* CPointChangelevel::GetBaseMap at 0073f7b0 */

datamap_t * CPointChangelevel::GetBaseMap(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return *(datamap_t **)(extraout_ECX + 0x5074d4);
}


/* __tcf_0 at 00a3e1a0 */

void __tcf_0(void *param_1)

{
  int unaff_EBX;
  int iVar1;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  if (0 < *(int *)(unaff_EBX + 0x41020e)) {
    iVar1 = 0;
    do {
      operator_delete(*(void **)(*(int *)(unaff_EBX + 0x410202) + iVar1 * 4));
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(unaff_EBX + 0x41020e));
  }
  *(undefined4 *)(unaff_EBX + 0x41020e) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x41020a)) {
    if (*(int *)(unaff_EBX + 0x410202) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2089ae) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2089ae),*(int *)(unaff_EBX + 0x410202));
      *(undefined4 *)(unaff_EBX + 0x410202) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x410206) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x410202);
  *(int *)(unaff_EBX + 0x410212) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x41020a)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x2089ae) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x2089ae),iVar1);
      *(undefined4 *)(unaff_EBX + 0x410202) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x410206) = 0;
  }
  return;
}


/* CPointChangelevel::InputChangeLevel at 0073f7f0 */

/* DWARF original prototype: void InputChangeLevel(CPointChangelevel * this, inputdata_t *
   inputdata) */

void __thiscall CPointChangelevel::InputChangeLevel(CPointChangelevel *this,inputdata_t *inputdata)

{
  char *pDest;
  char *pcVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  COutputEvent::FireOutput(&this->m_OnChangeLevel,inputdata->pActivator,(CBaseEntity *)this,0.0);
  pcVar1 = *(char **)(**(int **)(unaff_EBX + 0x507366) + 0x3c);
  if (pcVar1 == (char *)0x0) {
    pcVar1 = (char *)(unaff_EBX + 0x30f526);
  }
  V_strncpy((char *)(unaff_EBX + 0x6481fa),pcVar1,0x20);
  if ((inputdata->value).fieldType == FIELD_STRING) {
    pcVar1 = (char *)(inputdata->value).field_0.iVal;
    if (pcVar1 == (char *)0x0) {
      pcVar1 = (char *)(unaff_EBX + 0x30f526);
    }
  }
  else {
    pcVar1 = variant_t::ToString(&inputdata->value);
  }
  pDest = &UNK_0064821a + unaff_EBX;
  V_strncpy(pDest,pcVar1,0x20);
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x5073a2) + 0xa0))
            ((int *)**(undefined4 **)(unaff_EBX + 0x5073a2),pDest,0);
  (*(code *)**(undefined4 **)**(undefined4 **)(unaff_EBX + 0x507382))
            ((undefined4 *)**(undefined4 **)(unaff_EBX + 0x507382),pDest,unaff_EBX + 0x3731a2);
  return;
}


/* ChangeLevel_DestinationMapName at 0073f7c0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

char * ChangeLevel_DestinationMapName(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (char *)(extraout_ECX + 0x648250);
}


/* ChangeLevel_OriginMapName at 0073f7d0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

char * ChangeLevel_OriginMapName(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (char *)(extraout_ECX + 0x648220);
}


/* ChangeLevel_GetLandmarkName at 0073f7e0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

char * ChangeLevel_GetLandmarkName(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (char *)(extraout_ECX + 0x3731b8);
}


/* _GLOBAL__I__ZN17CPointChangelevel9m_DataMapE at 000af6f0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN17CPointChangelevel9m_DataMapE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

