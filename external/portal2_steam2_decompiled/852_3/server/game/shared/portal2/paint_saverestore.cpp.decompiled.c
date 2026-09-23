/* DWARF-guided pseudocode for game/shared/portal2/paint_saverestore.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 0001e730 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  char *unaff_EBX;
  char *unaff_EBP;
  longlong lVar1;
  int unaff_retaddr;
  ConVar *in_stack_fffffff4;
  char *pName_1;
  
  pName_1 = unaff_EBX;
  lVar1 = ___i686_get_pc_thunk_bx();
  if (lVar1 != 0xffff00000001) {
    return;
  }
  unaff_EBX[0xd4702c] = '\0';
  unaff_EBX[0xd4702d] = '\0';
  unaff_EBX[0xd4702e] = '\0';
  unaff_EBX[0xd4702f] = '\0';
  unaff_EBX[0xd47030] = '\0';
  unaff_EBX[0xd47031] = '\0';
  unaff_EBX[0xd47032] = '\0';
  unaff_EBX[0xd47033] = '\0';
  unaff_EBX[0xd47034] = '\0';
  unaff_EBX[0xd47035] = '\0';
  unaff_EBX[0xd47036] = '\0';
  unaff_EBX[0xd47037] = '\0';
  unaff_EBX[0xd47038] = -1;
  unaff_EBX[0xd47039] = -1;
  unaff_EBX[0xd4703a] = '\x7f';
  unaff_EBX[0xd4703b] = '\x7f';
  unaff_EBX[0xd4703c] = -1;
  unaff_EBX[0xd4703d] = -1;
  unaff_EBX[0xd4703e] = '\x7f';
  unaff_EBX[0xd4703f] = '\x7f';
  unaff_EBX[0xd47040] = '\0';
  unaff_EBX[0xd47041] = '\0';
  unaff_EBX[0xd47042] = '\0';
  unaff_EBX[0xd47043] = '\0';
  unaff_EBX[0xd47044] = '\0';
  unaff_EBX[0xd47045] = '\0';
  unaff_EBX[0xd47046] = '\0';
  unaff_EBX[0xd47047] = '\0';
  unaff_EBX[0xd47048] = '\0';
  unaff_EBX[0xd47049] = '\0';
  unaff_EBX[0xd4704a] = '\0';
  unaff_EBX[0xd4704b] = '\0';
  unaff_EBX[0xd4704c] = '\0';
  unaff_EBX[0xd4704d] = '\0';
  unaff_EBX[0xd4704e] = '\0';
  unaff_EBX[0xd4704f] = '\0';
  unaff_EBX[0xd47050] = -1;
  unaff_EBX[0xd47051] = -1;
  unaff_EBX[0xd47052] = '\x7f';
  unaff_EBX[0xd47053] = '\x7f';
  unaff_EBX[0xd47054] = -1;
  unaff_EBX[0xd47055] = -1;
  unaff_EBX[0xd47056] = '\x7f';
  unaff_EBX[0xd47057] = '\x7f';
  unaff_EBX[0xd47058] = -1;
  unaff_EBX[0xd47059] = -1;
  unaff_EBX[0xd4705a] = '\x7f';
  unaff_EBX[0xd4705b] = '\x7f';
  unaff_EBX[0xd4705c] = -1;
  unaff_EBX[0xd4705d] = -1;
  unaff_EBX[0xd4705e] = '\x7f';
  unaff_EBX[0xd4705f] = '\x7f';
  unaff_EBX[0xd47060] = '\0';
  unaff_EBX[0xd47061] = '\0';
  unaff_EBX[0xd47062] = '\0';
  unaff_EBX[0xd47063] = '\0';
  unaff_EBX[0xd47067] = '\0';
  unaff_EBX[0xd47064] = '\0';
  unaff_EBX[0xd47065] = '\0';
  unaff_EBX[0xd47066] = '\0';
  unaff_EBX[0xd47068] = '\0';
  unaff_EBX[0xd47069] = '\0';
  unaff_EBX[0xd4706a] = '\0';
  unaff_EBX[0xd4706b] = '\0';
  unaff_EBX[0xd4706f] = '\0';
  unaff_EBX[0xd4706c] = '\0';
  unaff_EBX[0xd4706d] = '\0';
  unaff_EBX[0xd4706e] = '\0';
  unaff_EBX[0xd47070] = '\0';
  unaff_EBX[0xd47071] = '\0';
  unaff_EBX[0xd47072] = '\0';
  unaff_EBX[0xd47073] = '\0';
  unaff_EBX[0xd47077] = '\x01';
  unaff_EBX[0xd47074] = '\0';
  unaff_EBX[0xd47075] = '\0';
  unaff_EBX[0xd47076] = '\0';
  unaff_EBX[0xd47078] = '\x01';
  unaff_EBX[0xd47079] = '\0';
  unaff_EBX[0xd4707a] = '\0';
  unaff_EBX[0xd4707b] = '\0';
  unaff_EBX[0xd4707f] = '\0';
  unaff_EBX[0xd4707c] = '\0';
  unaff_EBX[0xd4707d] = '\0';
  unaff_EBX[0xd4707e] = '\0';
  unaff_EBX[0xd47080] = '\x02';
  unaff_EBX[0xd47081] = '\0';
  unaff_EBX[0xd47082] = '\0';
  unaff_EBX[0xd47083] = '\0';
  unaff_EBX[0xd47087] = '\0';
  unaff_EBX[0xd47084] = '\0';
  unaff_EBX[0xd47085] = '\0';
  unaff_EBX[0xd47086] = '\0';
  *(char **)(unaff_EBX + 0xd47088) = unaff_EBX + 0xb92bec;
  ConVar::ConVar((ConVar *)(unaff_EBX + 0xcddf24),unaff_EBX + 0x99f611,unaff_EBX + 0x98d4fc,2,
                 in_stack_fffffff4,pName_1,unaff_EBP,unaff_retaddr);
  ___cxa_atexit(unaff_EBX + 0x950a84,0,*(undefined4 *)(unaff_EBX + 0xb8d454));
  *(char **)(unaff_EBX + 0xcddf7c) = unaff_EBX + 0xb9818c;
  return;
}


/* __tcf_0 at 0096f1c0 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x38d498),in_stack_00000008);
  return;
}


/* CPaintSaveRestoreBlockHandler::GetBlockName at 00244fa0 */

/* DWARF original prototype: char * GetBlockName(CPaintSaveRestoreBlockHandler * this) */

char * __thiscall CPaintSaveRestoreBlockHandler::GetBlockName(CPaintSaveRestoreBlockHandler *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (char *)(extraout_ECX + 0x7cb4b0);
}


/* CPaintSaveRestoreBlockHandler::PreSave at 00244fb0 */

/* DWARF original prototype: void PreSave(CPaintSaveRestoreBlockHandler * this, CSaveRestoreData *
   param_1) */

void __thiscall
CPaintSaveRestoreBlockHandler::PreSave
          (CPaintSaveRestoreBlockHandler *this,CSaveRestoreData *param_1)

{
  return;
}


/* CPaintSaveRestoreBlockHandler::Save at 00245020 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void Save(CPaintSaveRestoreBlockHandler * this, ISave * pSave) */

void __thiscall
CPaintSaveRestoreBlockHandler::Save(CPaintSaveRestoreBlockHandler *this,ISave *pSave)

{
  CProjectedWallEntity *this_00;
  char cVar1;
  int iVar2;
  int *piVar3;
  float *pfVar4;
  PaintPowerType PVar5;
  float *pfVar6;
  int unaff_EBX;
  int iVar7;
  float fVar8;
  float fVar9;
  float fVar10;
  float fVar11;
  int local_88;
  float local_50;
  float local_4c;
  float local_48;
  float local_44;
  float local_40;
  float local_3c;
  Vector local_38;
  PaintPowerType local_2c;
  int local_28;
  int local_24;
  char local_1d [13];
  
                    /* Unresolved local var: ProjectedWallVector_t * pProjectedWalls@[???] */
  ___i686_get_pc_thunk_bx();
  cVar1 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x966b87) + 0x20c))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x966b87));
  if (cVar1 != '\0') {
    if (*(int *)(*(int *)(&DAT_00ab764b + unaff_EBX) + 0x30) != 0) {
      CPaintStreamManager::SavePaintBlobState(*(CPaintStreamManager **)(unaff_EBX + 0x966daf),pSave)
      ;
    }
    CPaintDatabase::SavePaintmapData(*(CPaintDatabase **)(unaff_EBX + 0x966edf),pSave);
    local_24 = *(int *)(*(int *)(unaff_EBX + 0x966edf) + 0x4c);
    (*pSave->_vptr_ISave[0xb])(pSave,&local_24,1);
    if (0 < local_24) {
      local_88 = 0;
      do {
        this_00 = *(CProjectedWallEntity **)
                   (*(int *)(*(int *)(unaff_EBX + 0x966edf) + 0x40) + local_88 * 4);
        if (this_00 != (CProjectedWallEntity *)0x0) {
          (*pSave->_vptr_ISave[8])(pSave);
          iVar2 = (*(this_00->super_CPaintableEntity<CBaseProjectedEntity>).
                    super_CBaseProjectedEntity.super_CBaseEntity.super_IServerEntity.
                    super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xc9])(this_00);
          local_1d[0] = iVar2 != 0;
          (*pSave->_vptr_ISave[0xc])(pSave,local_1d,1);
          if (local_1d[0] == '\0') {
            local_50 = -NAN;
            pfVar4 = (float *)(*(this_00->super_CPaintableEntity<CBaseProjectedEntity>).
                                super_CBaseProjectedEntity.super_CBaseEntity.super_IServerEntity.
                                super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[3])
                                        (this_00);
            local_50 = *pfVar4;
          }
          else {
            piVar3 = (int *)(*(this_00->super_CPaintableEntity<CBaseProjectedEntity>).
                              super_CBaseProjectedEntity.super_CBaseEntity.super_IServerEntity.
                              super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xc9])
                                      (this_00);
            local_50 = -NAN;
            if (piVar3 == (int *)0x0) {
              local_50 = -NAN;
            }
            else {
              pfVar4 = (float *)(**(code **)(*piVar3 + 0xc))(piVar3);
              local_50 = *pfVar4;
            }
          }
          (*pSave->_vptr_ISave[0x2e])(pSave,&local_50,1);
          iVar2 = CProjectedWallEntity::GetNumSegments(this_00);
          local_28 = 0;
                    /* Unresolved local var: int s@[???] */
          if (0 < iVar2) {
            iVar7 = 0;
            do {
                    /* Unresolved local var: PaintPowerType type@[???] */
              PVar5 = CProjectedWallEntity::GetPaintPowerAtSegment(this_00,iVar7);
              local_28 = local_28 + (uint)(PVar5 != NO_POWER);
              iVar7 = iVar7 + 1;
            } while (iVar7 != iVar2);
          }
          (*pSave->_vptr_ISave[0xb])(pSave,&local_28,1);
          fVar11 = CProjectedWallEntity::GetSegmentLength(this_00);
          pfVar4 = (float *)(*(this_00->super_CPaintableEntity<CBaseProjectedEntity>).
                              super_CBaseProjectedEntity.super_CBaseEntity.super_IServerEntity.
                              super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xcb])
                                      (this_00);
          pfVar6 = (float *)(*(this_00->super_CPaintableEntity<CBaseProjectedEntity>).
                              super_CBaseProjectedEntity.super_CBaseEntity.super_IServerEntity.
                              super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xcc])
                                      (this_00);
                    /* Unresolved local var: Vector res@[???] */
          local_44 = *pfVar6 - *pfVar4;
          local_40 = pfVar6[1] - pfVar4[1];
          local_3c = pfVar6[2] - pfVar4[2];
          local_38.x = local_44;
          local_38.y = local_40;
          local_38.z = local_3c;
          VectorNormalize(&local_38);
          if (0 < iVar2) {
            iVar7 = 0;
            do {
              local_2c = CProjectedWallEntity::GetPaintPowerAtSegment(this_00,iVar7);
              if (local_2c != NO_POWER) {
                fVar8 = ((float)iVar7 + *(float *)(unaff_EBX + 0x7e54cf)) * fVar11;
                    /* Unresolved local var: Vector res@[???] */
                fVar9 = fVar8 * local_38.x;
                fVar10 = fVar8 * local_38.y;
                fVar8 = fVar8 * local_38.z;
                pfVar4 = (float *)(*(this_00->super_CPaintableEntity<CBaseProjectedEntity>).
                                    super_CBaseProjectedEntity.super_CBaseEntity.super_IServerEntity
                                    .super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity
                                    [0xcb])(this_00);
                    /* Unresolved local var: Vector res@[???] */
                local_50 = *pfVar4 + fVar9;
                local_4c = pfVar4[1] + fVar10;
                local_48 = pfVar4[2] + fVar8;
                (*pSave->_vptr_ISave[0xb])(pSave,&local_2c,1);
                (*pSave->_vptr_ISave[0x11])(pSave,&local_50);
              }
              iVar7 = iVar7 + 1;
            } while (iVar7 != iVar2);
          }
          (*pSave->_vptr_ISave[9])(pSave);
        }
        local_88 = local_88 + 1;
      } while (local_88 < local_24);
    }
  }
  return;
}


/* CPaintSaveRestoreBlockHandler::WriteSaveHeaders at 00244fc0 */

/* DWARF original prototype: void WriteSaveHeaders(CPaintSaveRestoreBlockHandler * this, ISave *
   param_1) */

void __thiscall
CPaintSaveRestoreBlockHandler::WriteSaveHeaders(CPaintSaveRestoreBlockHandler *this,ISave *param_1)

{
  return;
}


/* CPaintSaveRestoreBlockHandler::PostSave at 00244fd0 */

/* DWARF original prototype: void PostSave(CPaintSaveRestoreBlockHandler * this) */

void __thiscall CPaintSaveRestoreBlockHandler::PostSave(CPaintSaveRestoreBlockHandler *this)

{
  return;
}


/* CPaintSaveRestoreBlockHandler::PreRestore at 00245000 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void PreRestore(CPaintSaveRestoreBlockHandler * this) */

void __thiscall CPaintSaveRestoreBlockHandler::PreRestore(CPaintSaveRestoreBlockHandler *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  CPaintDatabase::RemoveAllPaint(*(CPaintDatabase **)(extraout_ECX + 0x966f08));
  return;
}


/* CPaintSaveRestoreBlockHandler::ReadRestoreHeaders at 00244fe0 */

/* DWARF original prototype: void ReadRestoreHeaders(CPaintSaveRestoreBlockHandler * this, IRestore
   * param_1) */

void __thiscall
CPaintSaveRestoreBlockHandler::ReadRestoreHeaders
          (CPaintSaveRestoreBlockHandler *this,IRestore *param_1)

{
  return;
}


/* CPaintSaveRestoreBlockHandler::Restore at 002454b0 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Struct "MenuItem": ignoring multiple overlapping fields */
/* WARNING: Struct "TreeNodeText": ignoring multiple overlapping fields */
/* DWARF original prototype: void Restore(CPaintSaveRestoreBlockHandler * this, IRestore * pRestore,
   bool fCreatePlayers) */

void __thiscall
CPaintSaveRestoreBlockHandler::Restore
          (CPaintSaveRestoreBlockHandler *this,IRestore *pRestore,bool fCreatePlayers)

{
  int iVar1;
  int *piVar2;
  undefined4 uVar3;
  char cVar4;
  CProjectedWallEntity *pCVar5;
  int iVar6;
  int iVar7;
  PaintPowerType powerType;
  int iVar8;
  int unaff_EBX;
  int iVar9;
  CBaseEntity *pEnt;
  int nWalls;
  int paintedWallCount;
  int local_98;
  trace_t local_8c;
  CUtlMemory<CProjectedWallEntity*,int> local_38;
  int local_2c;
  CProjectedWallEntity **local_28;
  uint local_24;
  char local_1d [13];
  
                    /* Unresolved local var: ProjectedWallVector_t projectedWalls@[???] */
  ___i686_get_pc_thunk_bx();
  cVar4 = (**(code **)(*(int *)**(undefined4 **)
                                 (vgui::CTreeViewListControl::~CTreeViewListControl + unaff_EBX + 7)
                      + 0x20c))
                    ((int *)**(undefined4 **)
                              (vgui::CTreeViewListControl::~CTreeViewListControl + unaff_EBX + 7));
  if (cVar4 != '\0') {
    if (*(int *)(*(int *)(&DAT_00ab71bb + unaff_EBX) + 0x30) != 0) {
      CPaintStreamManager::RestorePaintBlobState
                (*(CPaintStreamManager **)(&DAT_0096691f + unaff_EBX),pRestore);
    }
    CPaintDatabase::RestorePaintmapData(*(CPaintDatabase **)(unaff_EBX + 0x966a4f),pRestore);
    local_38.m_pMemory = (CProjectedWallEntity **)0x0;
    local_38.m_nAllocationCount = 0;
    local_38.m_nGrowSize = 0;
    local_2c = 0;
    local_28 = (CProjectedWallEntity **)0x0;
    pEnt = (CBaseEntity *)0x0;
    while (pEnt = CGlobalEntityList::FindEntityByClassname
                            (*(CGlobalEntityList **)(unaff_EBX + 0x96679f),pEnt,
                             (char *)(unaff_EBX + 0x773d23)), iVar1 = local_2c,
          pEnt != (CBaseEntity *)0x0) {
      pCVar5 = (CProjectedWallEntity *)
               ___dynamic_cast(pEnt,*(undefined4 *)(unaff_EBX + 0x966707),
                               *(undefined4 *)(unaff_EBX + 0x966a53),0);
      iVar7 = local_2c;
      iVar1 = local_2c + 1;
      if (local_38.m_nAllocationCount < iVar1) {
        CUtlMemory<CProjectedWallEntity*,int>::Grow(&local_38,iVar1 - local_38.m_nAllocationCount);
        iVar6 = (local_2c + 1) - iVar7;
        local_2c = local_2c + 1;
      }
      else {
        iVar6 = iVar1 - local_2c;
        local_2c = iVar1;
      }
      local_28 = local_38.m_pMemory;
      if (0 < iVar6 + -1) {
        _V_memmove(local_38.m_pMemory + iVar1,local_38.m_pMemory + iVar7,(iVar6 + -1) * 4);
      }
      if (local_38.m_pMemory + iVar7 != (CProjectedWallEntity **)0x0) {
        local_38.m_pMemory[iVar7] = pCVar5;
      }
    }
    iVar7 = (*pRestore->_vptr_IRestore[0xf])(pRestore);
    if (0 < iVar7) {
      local_98 = 0;
      piVar2 = *(int **)(&DAT_009666e3 + unaff_EBX);
      uVar3 = *(undefined4 *)(unaff_EBX + 0x966707);
      do {
        (*pRestore->_vptr_IRestore[7])(pRestore);
        (*pRestore->_vptr_IRestore[0x10])(pRestore,local_1d,1,0);
        if (local_1d[0] == '\0') {
          local_24 = 0xffffffff;
          (*pRestore->_vptr_IRestore[0x20])(pRestore,&local_24,1,0);
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
          if (((local_24 == 0xffffffff) ||
              (iVar6 = (local_24 & 0xffff) * 0x10 + **(int **)(&DAT_009666e3 + unaff_EBX),
              *(uint *)(iVar6 + 8) != local_24 >> 0x10)) ||
             (iVar6 = *(int *)(iVar6 + 4), iVar6 == 0)) goto LAB_00245701;
          pCVar5 = (CProjectedWallEntity *)
                   ___dynamic_cast(iVar6,*(undefined4 *)(unaff_EBX + 0x966707),
                                   *(undefined4 *)(unaff_EBX + 0x966a53),0);
        }
        else {
          local_24 = 0xffffffff;
          (*pRestore->_vptr_IRestore[0x20])(pRestore,&local_24,1,0);
                    /* Unresolved local var: int w@[???] */
          if (((local_24 != 0xffffffff) &&
              (iVar6 = (local_24 & 0xffff) * 0x10 + *piVar2,
              *(uint *)(iVar6 + 8) == local_24 >> 0x10)) &&
             ((iVar6 = *(int *)(iVar6 + 4), iVar6 != 0 &&
              ((iVar6 = ___dynamic_cast(iVar6,uVar3,*(undefined4 *)(unaff_EBX + 0x966a37),0),
               iVar6 != 0 && (0 < iVar1)))))) {
            iVar9 = 0;
            do {
              iVar8 = (*(local_38.m_pMemory[iVar9]->super_CPaintableEntity<CBaseProjectedEntity>).
                        super_CBaseProjectedEntity.super_CBaseEntity.super_IServerEntity.
                        super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xc9])
                                (local_38.m_pMemory[iVar9]);
              if (iVar6 == iVar8) {
                pCVar5 = local_38.m_pMemory[iVar9];
                goto LAB_00245703;
              }
              iVar9 = iVar9 + 1;
            } while (iVar9 != iVar1);
          }
LAB_00245701:
          pCVar5 = (CProjectedWallEntity *)0x0;
        }
LAB_00245703:
        iVar6 = (*pRestore->_vptr_IRestore[0xf])(pRestore);
                    /* Unresolved local var: int s@[???] */
        local_8c.m_pEnt = (CBaseEntity *)pCVar5;
        if (0 < iVar6) {
          iVar9 = 0;
          do {
            powerType = (*pRestore->_vptr_IRestore[0xf])(pRestore);
            (*pRestore->_vptr_IRestore[0x16])(pRestore,&local_8c.super_CBaseTrace.endpos,1,0);
            CPaintDatabase::AddPaint(*(CPaintDatabase **)(unaff_EBX + 0x966a4f),&local_8c,powerType)
            ;
            iVar9 = iVar9 + 1;
          } while (iVar9 != iVar6);
        }
        (*pRestore->_vptr_IRestore[8])(pRestore);
        local_98 = local_98 + 1;
      } while (local_98 != iVar7);
    }
    local_2c = 0;
    if (-1 < local_38.m_nGrowSize) {
      if (local_38.m_pMemory != (CProjectedWallEntity **)0x0) {
        (**(code **)(*(int *)**(undefined4 **)(&DAT_009666d7 + unaff_EBX) + 8))
                  ((int *)**(undefined4 **)(&DAT_009666d7 + unaff_EBX),local_38.m_pMemory);
        local_38.m_pMemory = (CProjectedWallEntity **)0x0;
      }
      local_38.m_nAllocationCount = 0;
    }
    local_2c = 0;
    if (-1 < local_38.m_nGrowSize) {
      if (local_38.m_pMemory != (CProjectedWallEntity **)0x0) {
        local_28 = local_38.m_pMemory;
        (**(code **)(*(int *)**(undefined4 **)(&DAT_009666d7 + unaff_EBX) + 8))
                  ((int *)**(undefined4 **)(&DAT_009666d7 + unaff_EBX),local_38.m_pMemory);
        local_38.m_pMemory = (CProjectedWallEntity **)0x0;
      }
      local_38.m_nAllocationCount = 0;
    }
    if ((-1 < local_38.m_nGrowSize) && (local_38.m_pMemory != (CProjectedWallEntity **)0x0)) {
      local_28 = local_38.m_pMemory;
      (**(code **)(*(int *)**(undefined4 **)(&DAT_009666d7 + unaff_EBX) + 8))
                ((int *)**(undefined4 **)(&DAT_009666d7 + unaff_EBX),local_38.m_pMemory);
    }
  }
  return;
}


/* CPaintSaveRestoreBlockHandler::PostRestore at 00244ff0 */

/* DWARF original prototype: void PostRestore(CPaintSaveRestoreBlockHandler * this) */

void __thiscall CPaintSaveRestoreBlockHandler::PostRestore(CPaintSaveRestoreBlockHandler *this)

{
  return;
}


/* GetPaintSaveRestoreBlockHandler at 00244f90 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

ISaveRestoreBlockHandler * GetPaintSaveRestoreBlockHandler(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ISaveRestoreBlockHandler *)(&UNK_00ab7720 + extraout_ECX);
}


/* _GLOBAL__I_save_paintblob at 0001e8d0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_save_paintblob(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

