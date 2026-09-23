/* DWARF-guided pseudocode for game/shared/portal2/paint_saverestore.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 0001be00 */

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
  unaff_EBX[0xde2b24] = '\0';
  unaff_EBX[0xde2b25] = '\0';
  unaff_EBX[0xde2b26] = '\0';
  unaff_EBX[0xde2b27] = '\0';
  unaff_EBX[0xde2b28] = '\0';
  unaff_EBX[0xde2b29] = '\0';
  unaff_EBX[0xde2b2a] = '\0';
  unaff_EBX[0xde2b2b] = '\0';
  unaff_EBX[0xde2b2c] = '\0';
  unaff_EBX[0xde2b2d] = '\0';
  unaff_EBX[0xde2b2e] = '\0';
  unaff_EBX[0xde2b2f] = '\0';
  unaff_EBX[0xde2b30] = -1;
  unaff_EBX[0xde2b31] = -1;
  unaff_EBX[0xde2b32] = '\x7f';
  unaff_EBX[0xde2b33] = '\x7f';
  unaff_EBX[0xde2b34] = -1;
  unaff_EBX[0xde2b35] = -1;
  unaff_EBX[0xde2b36] = '\x7f';
  unaff_EBX[0xde2b37] = '\x7f';
  unaff_EBX[0xde2b38] = '\0';
  unaff_EBX[0xde2b39] = '\0';
  unaff_EBX[0xde2b3a] = '\0';
  unaff_EBX[0xde2b3b] = '\0';
  unaff_EBX[0xde2b3c] = '\0';
  unaff_EBX[0xde2b3d] = '\0';
  unaff_EBX[0xde2b3e] = '\0';
  unaff_EBX[0xde2b3f] = '\0';
  unaff_EBX[0xde2b40] = '\0';
  unaff_EBX[0xde2b41] = '\0';
  unaff_EBX[0xde2b42] = '\0';
  unaff_EBX[0xde2b43] = '\0';
  unaff_EBX[0xde2b44] = '\0';
  unaff_EBX[0xde2b45] = '\0';
  unaff_EBX[0xde2b46] = '\0';
  unaff_EBX[0xde2b47] = '\0';
  unaff_EBX[0xde2b48] = -1;
  unaff_EBX[0xde2b49] = -1;
  unaff_EBX[0xde2b4a] = '\x7f';
  unaff_EBX[0xde2b4b] = '\x7f';
  unaff_EBX[0xde2b4c] = -1;
  unaff_EBX[0xde2b4d] = -1;
  unaff_EBX[0xde2b4e] = '\x7f';
  unaff_EBX[0xde2b4f] = '\x7f';
  unaff_EBX[0xde2b50] = -1;
  unaff_EBX[0xde2b51] = -1;
  unaff_EBX[0xde2b52] = '\x7f';
  unaff_EBX[0xde2b53] = '\x7f';
  unaff_EBX[0xde2b54] = -1;
  unaff_EBX[0xde2b55] = -1;
  unaff_EBX[0xde2b56] = '\x7f';
  unaff_EBX[0xde2b57] = '\x7f';
  *(char **)(unaff_EBX + 0xde2b58) = unaff_EBX + 0xc304fc;
  ConVar::ConVar((ConVar *)(unaff_EBX + 0xd7ad14),unaff_EBX + 0xa45291,unaff_EBX + 0xa32ff4,2,
                 in_stack_fffffff4,pName_1,unaff_EBP,unaff_retaddr);
  ___cxa_atexit(unaff_EBX + 0x9f6fa4,0,*(undefined4 *)(unaff_EBX + 0xc2ad48));
  *(char **)(unaff_EBX + 0xd7ad6c) = unaff_EBX + 0xc35a9c;
  return;
}


/* __tcf_0 at 00a12db0 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x383d68),in_stack_00000008);
  return;
}


/* CPaintSaveRestoreBlockHandler::GetBlockName at 0024c980 */

/* DWARF original prototype: char * GetBlockName(CPaintSaveRestoreBlockHandler * this) */

char * __thiscall CPaintSaveRestoreBlockHandler::GetBlockName(CPaintSaveRestoreBlockHandler *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (char *)(extraout_ECX + 0x865a00);
}


/* CPaintSaveRestoreBlockHandler::PreSave at 0024c990 */

/* DWARF original prototype: void PreSave(CPaintSaveRestoreBlockHandler * this, CSaveRestoreData *
   param_1) */

void __thiscall
CPaintSaveRestoreBlockHandler::PreSave
          (CPaintSaveRestoreBlockHandler *this,CSaveRestoreData *param_1)

{
  return;
}


/* CPaintSaveRestoreBlockHandler::Save at 0024ca00 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
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
  cVar1 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x9fa16f) + 0x20c))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x9fa16f));
  if (cVar1 != '\0') {
    if (*(int *)(*(int *)(&DAT_00b4a12b + unaff_EBX) + 0x30) != 0) {
      CPaintStreamManager::SavePaintBlobState
                (*(CPaintStreamManager **)(&DAT_009fa393 + unaff_EBX),pSave);
    }
    CPaintDatabase::SavePaintmapData(*(CPaintDatabase **)(&DAT_009fa4d3 + unaff_EBX),pSave);
    local_24 = *(int *)(*(int *)(&DAT_009fa4d3 + unaff_EBX) + 0x4c);
    (*pSave->_vptr_ISave[0xb])(pSave,&local_24,1);
    if (0 < local_24) {
      local_88 = 0;
      do {
        this_00 = *(CProjectedWallEntity **)
                   (*(int *)(*(int *)(&DAT_009fa4d3 + unaff_EBX) + 0x40) + local_88 * 4);
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
                fVar8 = ((float)iVar7 + *(float *)(unaff_EBX + 0x8800ef)) * fVar11;
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


/* CPaintSaveRestoreBlockHandler::WriteSaveHeaders at 0024c9a0 */

/* DWARF original prototype: void WriteSaveHeaders(CPaintSaveRestoreBlockHandler * this, ISave *
   param_1) */

void __thiscall
CPaintSaveRestoreBlockHandler::WriteSaveHeaders(CPaintSaveRestoreBlockHandler *this,ISave *param_1)

{
  return;
}


/* CPaintSaveRestoreBlockHandler::PostSave at 0024c9b0 */

/* DWARF original prototype: void PostSave(CPaintSaveRestoreBlockHandler * this) */

void __thiscall CPaintSaveRestoreBlockHandler::PostSave(CPaintSaveRestoreBlockHandler *this)

{
  return;
}


/* CPaintSaveRestoreBlockHandler::PreRestore at 0024c9e0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void PreRestore(CPaintSaveRestoreBlockHandler * this) */

void __thiscall CPaintSaveRestoreBlockHandler::PreRestore(CPaintSaveRestoreBlockHandler *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  CPaintDatabase::RemoveAllPaint(*(CPaintDatabase **)(&DAT_009fa4fc + extraout_ECX));
  return;
}


/* CPaintSaveRestoreBlockHandler::ReadRestoreHeaders at 0024c9c0 */

/* DWARF original prototype: void ReadRestoreHeaders(CPaintSaveRestoreBlockHandler * this, IRestore
   * param_1) */

void __thiscall
CPaintSaveRestoreBlockHandler::ReadRestoreHeaders
          (CPaintSaveRestoreBlockHandler *this,IRestore *param_1)

{
  return;
}


/* CPaintSaveRestoreBlockHandler::Restore at 0024ce90 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
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
  cVar4 = (**(code **)(*(int *)**(undefined4 **)(&DAT_009f9cdf + unaff_EBX) + 0x20c))
                    ((int *)**(undefined4 **)(&DAT_009f9cdf + unaff_EBX));
  if (cVar4 != '\0') {
    if (*(int *)(*(int *)(&DAT_00b49c9b + unaff_EBX) + 0x30) != 0) {
      CPaintStreamManager::RestorePaintBlobState
                (*(CPaintStreamManager **)(unaff_EBX + 0x9f9f03),pRestore);
    }
    CPaintDatabase::RestorePaintmapData(*(CPaintDatabase **)(unaff_EBX + 0x9fa043),pRestore);
    local_38.m_pMemory = (CProjectedWallEntity **)0x0;
    local_38.m_nAllocationCount = 0;
    local_38.m_nGrowSize = 0;
    local_2c = 0;
    local_28 = (CProjectedWallEntity **)0x0;
    pEnt = (CBaseEntity *)0x0;
    while (pEnt = CGlobalEntityList::FindEntityByClassname
                            (*(CGlobalEntityList **)(unaff_EBX + 0x9f9d83),pEnt,
                             (char *)(unaff_EBX + 0x80f583)), iVar1 = local_2c,
          pEnt != (CBaseEntity *)0x0) {
      pCVar5 = (CProjectedWallEntity *)
               ___dynamic_cast(pEnt,*(undefined4 *)(unaff_EBX + 0x9f9cef),
                               *(undefined4 *)(unaff_EBX + 0x9fa047),0);
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
      piVar2 = *(int **)(unaff_EBX + 0x9f9ccb);
      uVar3 = *(undefined4 *)(unaff_EBX + 0x9f9cef);
      do {
        (*pRestore->_vptr_IRestore[7])(pRestore);
        (*pRestore->_vptr_IRestore[0x10])(pRestore,local_1d,1,0);
        if (local_1d[0] == '\0') {
          local_24 = 0xffffffff;
          (*pRestore->_vptr_IRestore[0x20])(pRestore,&local_24,1,0);
                    /* Unresolved local var: CBaseEntityList * g_pEntityList@[???] */
                    /* Unresolved local var: CEntInfo * pInfo@[???] */
          if (((local_24 == 0xffffffff) ||
              (iVar6 = (local_24 & 0xffff) * 0x10 + **(int **)(unaff_EBX + 0x9f9ccb),
              *(uint *)(iVar6 + 8) != local_24 >> 0x10)) ||
             (iVar6 = *(int *)(iVar6 + 4), iVar6 == 0)) goto LAB_0024d0e1;
          pCVar5 = (CProjectedWallEntity *)
                   ___dynamic_cast(iVar6,*(undefined4 *)(unaff_EBX + 0x9f9cef),
                                   *(undefined4 *)(unaff_EBX + 0x9fa047),0);
        }
        else {
          local_24 = 0xffffffff;
          (*pRestore->_vptr_IRestore[0x20])(pRestore,&local_24,1,0);
                    /* Unresolved local var: int w@[???] */
          if (((local_24 != 0xffffffff) &&
              (iVar6 = (local_24 & 0xffff) * 0x10 + *piVar2,
              *(uint *)(iVar6 + 8) == local_24 >> 0x10)) &&
             ((iVar6 = *(int *)(iVar6 + 4), iVar6 != 0 &&
              ((iVar6 = ___dynamic_cast(iVar6,uVar3,*(undefined4 *)(unaff_EBX + 0x9fa023),0),
               iVar6 != 0 && (0 < iVar1)))))) {
            iVar9 = 0;
            do {
              iVar8 = (*(local_38.m_pMemory[iVar9]->super_CPaintableEntity<CBaseProjectedEntity>).
                        super_CBaseProjectedEntity.super_CBaseEntity.super_IServerEntity.
                        super_IServerUnknown.super_IHandleEntity._vptr_IHandleEntity[0xc9])
                                (local_38.m_pMemory[iVar9]);
              if (iVar6 == iVar8) {
                pCVar5 = local_38.m_pMemory[iVar9];
                goto LAB_0024d0e3;
              }
              iVar9 = iVar9 + 1;
            } while (iVar9 != iVar1);
          }
LAB_0024d0e1:
          pCVar5 = (CProjectedWallEntity *)0x0;
        }
LAB_0024d0e3:
        iVar6 = (*pRestore->_vptr_IRestore[0xf])(pRestore);
                    /* Unresolved local var: int s@[???] */
        local_8c.m_pEnt = (CBaseEntity *)pCVar5;
        if (0 < iVar6) {
          iVar9 = 0;
          do {
            powerType = (*pRestore->_vptr_IRestore[0xf])(pRestore);
            (*pRestore->_vptr_IRestore[0x16])(pRestore,&local_8c.super_CBaseTrace.endpos,1,0);
            CPaintDatabase::AddPaint(*(CPaintDatabase **)(unaff_EBX + 0x9fa043),&local_8c,powerType)
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
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x9f9cbb) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x9f9cbb),local_38.m_pMemory);
        local_38.m_pMemory = (CProjectedWallEntity **)0x0;
      }
      local_38.m_nAllocationCount = 0;
    }
    local_2c = 0;
    if (-1 < local_38.m_nGrowSize) {
      if (local_38.m_pMemory != (CProjectedWallEntity **)0x0) {
        local_28 = local_38.m_pMemory;
        (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x9f9cbb) + 8))
                  ((int *)**(undefined4 **)(unaff_EBX + 0x9f9cbb),local_38.m_pMemory);
        local_38.m_pMemory = (CProjectedWallEntity **)0x0;
      }
      local_38.m_nAllocationCount = 0;
    }
    if ((-1 < local_38.m_nGrowSize) && (local_38.m_pMemory != (CProjectedWallEntity **)0x0)) {
      local_28 = local_38.m_pMemory;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x9f9cbb) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x9f9cbb),local_38.m_pMemory);
    }
  }
  return;
}


/* CPaintSaveRestoreBlockHandler::PostRestore at 0024c9d0 */

/* DWARF original prototype: void PostRestore(CPaintSaveRestoreBlockHandler * this) */

void __thiscall CPaintSaveRestoreBlockHandler::PostRestore(CPaintSaveRestoreBlockHandler *this)

{
  return;
}


/* GetPaintSaveRestoreBlockHandler at 0024c970 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

ISaveRestoreBlockHandler * GetPaintSaveRestoreBlockHandler(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ISaveRestoreBlockHandler *)(&UNK_00b4a200 + extraout_ECX);
}


/* _GLOBAL__I_save_paintblob at 0001bf00 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_save_paintblob(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

