/* DWARF-guided pseudocode for game/shared/portal2/damage_database.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: server.dylib.
 */

/* __static_initialization_and_destruction_0 at 0001bbb0 */

/* WARNING: Type propagation algorithm not settling */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  undefined4 uVar1;
  int unaff_EBX;
  char *unaff_ESI;
  longlong lVar2;
  ConVar *in_stack_ffffffe8;
  char *in_stack_ffffffec;
  char *in_stack_fffffff0;
  int flags_1;
  
  flags_1 = unaff_EBX;
  lVar2 = ___i686_get_pc_thunk_bx();
  if (lVar2 != 0xffff00000001) {
    return;
  }
  (&CDmxElement::s_TypeSymbols.m_Lookup.
    super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
    .m_aBuckets[0x472].m_AddLock.field_0xf)[unaff_EBX] = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x473].m_pFirst + unaff_EBX) = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x473].m_pFirst + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x473].m_pFirst + unaff_EBX + 2) = 0;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x473].m_pFirst + unaff_EBX + 3) = 0;
  *(undefined4 *)
   (&UNK_00006b07 +
   (int)&((CUtlMemoryPool *)
         (&CDmxElement::s_TypeSymbols.m_Lookup.
           super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
           .m_EntryMemory.super_CUtlMemoryPool.m_BlobHead + -2))->m_BlobHead +
   unaff_EBX + 0xffffffe0U) = 0;
  *(undefined4 *)
   (&UNK_00006b0b +
   (int)&((CUtlMemoryPool *)
         (&CDmxElement::s_TypeSymbols.m_Lookup.
           super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
           .m_EntryMemory.super_CUtlMemoryPool.m_BlobHead + -2))->m_BlobHead +
   unaff_EBX + 0xffffffe0U) = 0x7f7fffff;
  *(undefined4 *)
   (&UNK_00006b0f +
   (int)&((CUtlMemoryPool *)
         (&CDmxElement::s_TypeSymbols.m_Lookup.
           super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
           .m_EntryMemory.super_CUtlMemoryPool.m_BlobHead + -2))->m_BlobHead +
   unaff_EBX + 0xffffffe0U) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x473].m_AddLock.m_nWriters.m_value + unaff_EBX + 3) = 0;
  *(undefined4 *)
   (&CDmxElement::s_TypeSymbols.m_Lookup.
     super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
     .m_aBuckets[0x473].m_AddLock.field_0xf + unaff_EBX) = 0;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x474].m_pFirst + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x474].m_pFirstUncommitted + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x474].m_AddLock.m_lockInfo + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x474].m_AddLock.m_lockInfo + unaff_EBX + 7) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x474].m_AddLock.m_nWriters.m_value + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)
   (&CDmxElement::s_TypeSymbols.m_Lookup.
     super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
     .m_aBuckets[0x474].m_AddLock.field_0xf + unaff_EBX) = 0x7f7fffff;
  *(undefined **)
   ((int)&CDmxElement::s_TypeSymbols.m_Lookup.
          super_CUtlTSHash<CUtlSymbolTableLargeBaseTreeEntry_t*,2048,CUtlSymbolTableLargeBaseTreeEntry_t*,CCThreadsafeTreeHashMethod<2048,_CUtlSymbolTableLargeBaseTreeEntry_t*,_false>,0>
          .m_aBuckets[0x475].m_pFirst + unaff_EBX + 3) = &UNK_00c30747 + unaff_EBX;
  ConVar::ConVar((ConVar *)((int)DataMapInit<CFuncTrain>::dataDesc[3].flatOffset + unaff_EBX + 0xb),
                 (char *)(unaff_EBX + 0xa45423),"CEnvSpark" + unaff_EBX + 5,0x2002,
                 (char *)(unaff_EBX + 0xa453eb),in_stack_ffffffe8,in_stack_ffffffec,
                 in_stack_fffffff0,flags_1,unaff_ESI);
  uVar1 = *(undefined4 *)(&DAT_00c2af93 + unaff_EBX);
  ___cxa_atexit(unaff_EBX + 0x9f70af,0,uVar1);
  ConVar::ConVar((ConVar *)
                 ((int)DataMapInit<CFuncTrain>::dataDesc[5].flatOffset + unaff_EBX + -0x15),
                 &UNK_00a45468 + unaff_EBX,(char *)(unaff_EBX + 0xa3323f),0x2002,
                 &UNK_00a45448 + unaff_EBX,in_stack_ffffffe8,in_stack_ffffffec,in_stack_fffffff0,
                 flags_1,unaff_ESI);
  ___cxa_atexit(unaff_EBX + 0x9f708f,0,uVar1);
  DamageDatabase::DamageDatabase
            ((DamageDatabase *)
             ((int)DataMapInit<CFuncTrain>::dataDesc[6].flatOffset + unaff_EBX + 0xb));
  ___cxa_atexit(unaff_EBX + 0x9f70cf,0,uVar1);
  return;
}


/* __tcf_0 at 00a12c70 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x383dc8),in_stack_00000008);
  return;
}


/* __tcf_1 at 00a12c50 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x383e48),in_stack_00000008);
  return;
}


/* __tcf_2 at 00a12c90 */

void __tcf_2(void *param_1)

{
  int iVar1;
  int unaff_EBX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_bx();
  *(int *)(unaff_EBX + 0x383e5c) = unaff_EBX + 0x23eba4;
  *(undefined4 *)(unaff_EBX + 0x383e74) = 0;
  if (-1 < *(int *)(unaff_EBX + 0x383e70)) {
    if (*(int *)(unaff_EBX + 0x383e68) != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x233eb8) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x233eb8),*(int *)(unaff_EBX + 0x383e68));
      *(undefined4 *)(unaff_EBX + 0x383e68) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x383e6c) = 0;
  }
  iVar1 = *(int *)(unaff_EBX + 0x383e68);
  *(int *)(unaff_EBX + 0x383e78) = iVar1;
  if (-1 < *(int *)(unaff_EBX + 0x383e70)) {
    if (iVar1 != 0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x233eb8) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x233eb8),iVar1);
      *(undefined4 *)(unaff_EBX + 0x383e68) = 0;
    }
    *(undefined4 *)(unaff_EBX + 0x383e6c) = 0;
  }
  *(int *)(unaff_EBX + 0x383e5c) = unaff_EBX + 0x2363e4;
  IGameSystem::~IGameSystem((IGameSystem *)(unaff_EBX + 0x383e5c),in_stack_00000008);
  return;
}


/* DamageDatabase::DamageDatabase at 0001bb50 */

/* DWARF original prototype: void DamageDatabase(DamageDatabase * this) */

void __thiscall DamageDatabase::DamageDatabase(DamageDatabase *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CAutoGameSystem::CAutoGameSystem(&this->super_CAutoGameSystem,(char *)(unaff_EBX + 0xa4543c));
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(&UNK_00c35ce7 + unaff_EBX);
  this->m_DamageInfo[0].m_Memory.m_pMemory = (DamageInfo *)0x0;
  this->m_DamageInfo[0].m_Memory.m_nAllocationCount = 0;
  this->m_DamageInfo[0].m_Memory.m_nGrowSize = 0;
  this->m_DamageInfo[0].m_Size = 0;
  this->m_DamageInfo[0].m_pElements = (DamageInfo *)0x0;
  return;
}


/* DamageDatabase::DamageDatabase at 0024bbc0 */

/* DWARF original prototype: void DamageDatabase(DamageDatabase * this, DamageDatabase * this) */

void __thiscall DamageDatabase::DamageDatabase(DamageDatabase *this,DamageDatabase *this_1)

{
  DamageDatabase(this);
  return;
}


/* DamageDatabase::LevelInitPreEntity at 0024bab0 */

/* DWARF original prototype: void LevelInitPreEntity(DamageDatabase * this) */

void __thiscall DamageDatabase::LevelInitPreEntity(DamageDatabase *this)

{
  uchar *pICEKey;
  char *pcVar1;
  int unaff_EBX;
  int iVar2;
  int iVar3;
  int local_20;
  
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: int i@[???] */
  pICEKey = (uchar *)(**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x9fb0e2) + 0x8c))
                               ((int *)**(undefined4 **)(unaff_EBX + 0x9fb0e2));
  ReadDamageCutoutDataFromFile
            (this->m_DamageInfo,(IFileSystem *)**(undefined4 **)(unaff_EBX + 0x9fb0be),
             (char *)(unaff_EBX + 0x8154c5),pICEKey);
                    /* Unresolved local var: int damageInfoIndex@[???]
                       Unresolved local var: DamageInfoVector * damageInfo@[???]
                       Unresolved local var: int i@[???] */
  if (0 < this->m_DamageInfo[0].m_Size) {
    local_20 = 0;
    iVar3 = 0;
    do {
      CBaseEntity::PrecacheModel((this->m_DamageInfo[0].m_Memory.m_pMemory)->swapModelName + iVar3);
                    /* Unresolved local var: int j@[???] */
      pcVar1 = (this->m_DamageInfo[0].m_Memory.m_pMemory)->damageEffectName + iVar3;
      if (0 < *(int *)(pcVar1 + 0x8ec)) {
        iVar2 = 0;
        do {
          CBaseEntity::PrecacheModel(pcVar1 + iVar2 * 0x40 + 0x4ec);
          iVar2 = iVar2 + 1;
          pcVar1 = (this->m_DamageInfo[0].m_Memory.m_pMemory)->damageEffectName + iVar3;
        } while (iVar2 < *(int *)(pcVar1 + 0x8ec));
      }
                    /* Unresolved local var: int j@[???] */
      if (0 < *(int *)(pcVar1 + 0xb60)) {
        iVar2 = 0;
        do {
          PrecacheParticleSystem(pcVar1 + iVar2 * 0x40 + 0x93c);
          iVar2 = iVar2 + 1;
          pcVar1 = (this->m_DamageInfo[0].m_Memory.m_pMemory)->damageEffectName + iVar3;
        } while (iVar2 < *(int *)(pcVar1 + 0xb60));
      }
      local_20 = local_20 + 1;
      iVar3 = iVar3 + 0xba8;
    } while (local_20 < this->m_DamageInfo[0].m_Size);
  }
  return;
}


/* DamageDatabase::GetDamageInfoVector at 0024a840 */

/* DWARF original prototype: DamageInfoVector * GetDamageInfoVector(DamageDatabase * this,
   DamagedEntityType entityType) */

DamageInfoVector * __thiscall
DamageDatabase::GetDamageInfoVector(DamageDatabase *this,DamagedEntityType entityType)

{
  return this->m_DamageInfo + entityType;
}


/* DamageDatabase::FindPotentialDamageEffects at 0024b9f0 */

/* WARNING: Type propagation algorithm not settling */
/* DWARF original prototype: int FindPotentialDamageEffects(DamageDatabase * this, int *
   pPotentialDamageEffects, int maxDamageEffects, DamagedEntityType entityType, int damageType, int
   hitbox) */

int __thiscall
DamageDatabase::FindPotentialDamageEffects
          (DamageDatabase *this,int *pPotentialDamageEffects,int maxDamageEffects,
          DamagedEntityType entityType,int damageType,int hitbox)

{
  int iVar1;
  char *pcVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  
                    /* Unresolved local var: DamageInfoVector * damageInfo@[???]
                       Unresolved local var: int potentialDamageEffectCount@[???] */
  if ((this->m_DamageInfo[entityType].m_Size < 1) || (maxDamageEffects < 1)) {
    return 0;
  }
  iVar3 = 0;
  iVar5 = 0;
  iVar4 = 0;
  do {
    pcVar2 = (this->m_DamageInfo[entityType].m_Memory.m_pMemory)->damageEffectName + iVar4;
    if ((*(uint *)(pcVar2 + 0x4e8) & damageType) != 0) {
      if (hitbox == -1) {
        pPotentialDamageEffects[iVar3] = iVar5;
        iVar3 = iVar3 + 1;
      }
      else {
                    /* Unresolved local var: int hitboxIndex@[???] */
        if (0 < *(int *)(pcVar2 + 0x60)) {
          iVar1 = 0;
          while (iVar3 < maxDamageEffects) {
            if ((*(int *)(pcVar2 + iVar1 * 4 + 0x40) < 0) ||
               (hitbox != *(int *)(pcVar2 + iVar1 * 4 + 0x40))) {
              pcVar2 = (this->m_DamageInfo[entityType].m_Memory.m_pMemory)->damageEffectName + iVar4
              ;
              if (*(int *)(pcVar2 + 0x60) <= iVar1 + 1) break;
            }
            else {
              pPotentialDamageEffects[iVar3] = iVar5;
              iVar3 = iVar3 + 1;
              pcVar2 = (this->m_DamageInfo[entityType].m_Memory.m_pMemory)->damageEffectName + iVar4
              ;
              if (*(int *)(pcVar2 + 0x60) <= iVar1 + 1) break;
            }
            iVar1 = iVar1 + 1;
          }
        }
      }
    }
    iVar5 = iVar5 + 1;
    if ((this->m_DamageInfo[entityType].m_Size <= iVar5) ||
       (iVar4 = iVar4 + 0xba8, maxDamageEffects <= iVar3)) {
      return iVar3;
    }
  } while( true );
}


/* DamageDatabase::PrecacheAssets at 0024b920 */

/* DWARF original prototype: void PrecacheAssets(DamageDatabase * this) */

void __thiscall DamageDatabase::PrecacheAssets(DamageDatabase *this)

{
  char *pcVar1;
  int iVar2;
  int iVar3;
  int i;
  
                    /* Unresolved local var: int damageInfoIndex@[???]
                       Unresolved local var: DamageInfoVector * damageInfo@[???] */
  if (0 < this->m_DamageInfo[0].m_Size) {
    i = 0;
    iVar3 = 0;
    do {
      CBaseEntity::PrecacheModel((this->m_DamageInfo[0].m_Memory.m_pMemory)->swapModelName + iVar3);
                    /* Unresolved local var: int j@[???] */
      pcVar1 = (this->m_DamageInfo[0].m_Memory.m_pMemory)->damageEffectName + iVar3;
      if (0 < *(int *)(pcVar1 + 0x8ec)) {
        iVar2 = 0;
        do {
          CBaseEntity::PrecacheModel(pcVar1 + iVar2 * 0x40 + 0x4ec);
          iVar2 = iVar2 + 1;
          pcVar1 = (this->m_DamageInfo[0].m_Memory.m_pMemory)->damageEffectName + iVar3;
        } while (iVar2 < *(int *)(pcVar1 + 0x8ec));
      }
                    /* Unresolved local var: int j@[???] */
      if (0 < *(int *)(pcVar1 + 0xb60)) {
        iVar2 = 0;
        do {
          PrecacheParticleSystem(pcVar1 + iVar2 * 0x40 + 0x93c);
          iVar2 = iVar2 + 1;
          pcVar1 = (this->m_DamageInfo[0].m_Memory.m_pMemory)->damageEffectName + iVar3;
        } while (iVar2 < *(int *)(pcVar1 + 0xb60));
      }
      i = i + 1;
      iVar3 = iVar3 + 0xba8;
    } while (i < this->m_DamageInfo[0].m_Size);
  }
  return;
}


/* ReadDamageCutoutDataFromFile at 0024af90 */

bool ReadDamageCutoutDataFromFile
               (DamageInfoVector *damageInfoArray,IFileSystem *filesystem,char *szCutoutDataFilename
               ,uchar *pICEKey)

{
  uint uVar1;
  KeyValues *this;
  KeyValues *pKVar2;
  char *pcVar3;
  int iVar4;
  DamageInfo *pDVar5;
  int iVar6;
  int iVar7;
  int unaff_EBX;
  bool bVar8;
  float fVar9;
  char *local_bf4;
  char *local_be0;
  KeyValues *local_bdc;
  int local_bd8;
  int local_bd4;
  int local_bd0;
  char local_bc8 [64];
  int local_b88 [8];
  int local_b68;
  char local_b64 [1088];
  int local_724;
  char local_720 [64];
  uint local_6e0;
  char local_6dc [1024];
  int local_2dc;
  float local_2d8;
  float local_2d4;
  char local_2d0 [64];
  int local_290;
  char local_28c [512];
  int local_8c [8];
  int local_6c;
  int local_68;
  undefined1 local_64;
  char local_63 [67];
  int local_20;
  
                    /* Unresolved local var: KeyValues * pKV@[???]
                       Unresolved local var: char * modelPath@[???] */
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(unaff_EBX + 0x9fbc0f);
  if ((damageInfoArray->m_Size < 1) &&
     (this = ReadEncryptedKVFile(filesystem,szCutoutDataFilename,pICEKey), this != (KeyValues *)0x0)
     ) {
    pKVar2 = KeyValues::FindKey(this,(char *)(unaff_EBX + 0x815ef7),false);
    if (pKVar2 == (KeyValues *)0x0) {
      local_be0 = (char *)(unaff_EBX + 0x803d83);
    }
    else {
      local_be0 = KeyValues::GetString(pKVar2,(char *)0x0,(char *)(unaff_EBX + 0x803d83));
    }
    local_bf4 = (char *)(unaff_EBX + 0x803d83);
    local_bdc = KeyValues::GetFirstTrueSubKey(this);
    if (local_bdc != (KeyValues *)0x0) {
      do {
        pKVar2 = KeyValues::FindKey(local_bdc,(char *)(unaff_EBX + 0x815f02),false);
        if (pKVar2 == (KeyValues *)0x0) {
          local_63[0] = '\0';
        }
        else {
          pcVar3 = KeyValues::GetString(pKVar2,(char *)0x0,local_bf4);
          V_snprintf(local_63,0x40,(char *)(unaff_EBX + 0x815eed),local_be0,pcVar3);
        }
        pKVar2 = KeyValues::FindKey(local_bdc,(char *)(unaff_EBX + 0x815f0c),false);
        iVar7 = 0;
        if (pKVar2 != (KeyValues *)0x0) {
          pcVar3 = KeyValues::GetString(pKVar2,(char *)0x0,local_bf4);
          iVar7 = ParseStringToIntArray(pcVar3,local_b88,8);
        }
        local_b68 = iVar7;
        pKVar2 = KeyValues::FindKey(local_bdc,(char *)(unaff_EBX + 0x815f15),false);
        iVar7 = 0;
        if (pKVar2 != (KeyValues *)0x0) {
          pcVar3 = KeyValues::GetString(pKVar2,(char *)0x0,local_bf4);
          iVar7 = ParseStringToExclusionArray(pcVar3,(char (*) [64])local_b64,0x10);
        }
        local_720[0] = '\0';
        local_724 = iVar7;
        pKVar2 = KeyValues::FindKey(local_bdc,(char *)(unaff_EBX + 0x815f23),false);
        if (pKVar2 != (KeyValues *)0x0) {
          pcVar3 = KeyValues::GetString(pKVar2,(char *)0x0,local_bf4);
          V_strncpy(local_720,pcVar3,0x40);
        }
        pKVar2 = KeyValues::FindKey(local_bdc,(char *)(unaff_EBX + 0x815f2f),false);
        if (pKVar2 != (KeyValues *)0x0) {
          local_6e0 = 0;
          pcVar3 = KeyValues::GetString(pKVar2,(char *)0x0,local_bf4);
          pcVar3 = V_stristr(pcVar3,(char *)(unaff_EBX + 0x815f34));
          if (pcVar3 != (char *)0x0) {
            local_6e0 = local_6e0 | 8;
          }
          pcVar3 = KeyValues::GetString(pKVar2,(char *)0x0,local_bf4);
          pcVar3 = V_stristr(pcVar3,(char *)(unaff_EBX + 0x815f39));
          if (pcVar3 != (char *)0x0) {
            local_6e0 = local_6e0 | 0x40;
          }
          pcVar3 = KeyValues::GetString(pKVar2,(char *)0x0,local_bf4);
          pcVar3 = V_stristr(pcVar3,(char *)(unaff_EBX + 0x815f43));
          if (pcVar3 != (char *)0x0) {
            local_6e0 = local_6e0 | 0x4000000;
          }
          pcVar3 = KeyValues::GetString(pKVar2,(char *)0x0,local_bf4);
          pcVar3 = V_stristr(pcVar3,(char *)(unaff_EBX + 0x815f4b));
          if (pcVar3 != (char *)0x0) {
            local_6e0 = local_6e0 | 0x100000;
          }
          pcVar3 = KeyValues::GetString(pKVar2,(char *)0x0,local_bf4);
          pcVar3 = V_stristr(pcVar3,(char *)(unaff_EBX + 0x815f55));
          if (pcVar3 != (char *)0x0) {
            local_6e0 = local_6e0 | 4;
          }
          pcVar3 = KeyValues::GetString(pKVar2,(char *)0x0,local_bf4);
          pcVar3 = V_stristr(pcVar3,(char *)(unaff_EBX + 0x815f5b));
          if (pcVar3 != (char *)0x0) {
            local_6e0 = local_6e0 | 1;
          }
          pcVar3 = KeyValues::GetString(pKVar2,(char *)0x0,local_bf4);
          pcVar3 = V_stristr(pcVar3,(char *)(unaff_EBX + 0x815f61));
          if (pcVar3 != (char *)0x0) {
            local_6e0 = local_6e0 | 0x20;
          }
          pKVar2 = KeyValues::FindKey(local_bdc,(char *)(unaff_EBX + 0x815f68),false);
          bVar8 = false;
          if (pKVar2 != (KeyValues *)0x0) {
            iVar7 = KeyValues::GetInt(pKVar2,(char *)0x0,0);
            bVar8 = iVar7 != 0;
          }
          local_64 = bVar8;
          pKVar2 = KeyValues::FindKey(local_bdc,(char *)(unaff_EBX + 0x815f6e),false);
          pcVar3 = KeyValues::GetString(pKVar2,(char *)0x0,local_bf4);
          local_2dc = ParseStringToGibArray(pcVar3,local_be0,(char (*) [64])local_6dc,0x10);
          pKVar2 = KeyValues::FindKey(local_bdc,(char *)(unaff_EBX + 0x815f73),false);
          if (pKVar2 == (KeyValues *)0x0) {
            local_2d4 = 0.0;
          }
          else {
            local_2d4 = KeyValues::GetFloat(pKVar2,(char *)0x0,0.0);
          }
          pKVar2 = KeyValues::FindKey(local_bdc,(char *)(unaff_EBX + 0x815f80),false);
          fVar9 = 0.0;
          if (pKVar2 != (KeyValues *)0x0) {
            fVar9 = KeyValues::GetFloat(pKVar2,(char *)0x0,0.0);
          }
          local_2d8 = fVar9;
          pKVar2 = KeyValues::FindKey(local_bdc,(char *)(unaff_EBX + 0x815f8a),false);
          local_2d0[0] = '\0';
          if (pKVar2 != (KeyValues *)0x0) {
            pcVar3 = KeyValues::GetString(pKVar2,(char *)0x0,local_bf4);
            V_strncpy(local_2d0,pcVar3,0x40);
          }
          pKVar2 = KeyValues::FindKey(local_bdc,(char *)(unaff_EBX + 0x815f99),false);
          local_290 = -1;
          if (pKVar2 != (KeyValues *)0x0) {
            local_290 = KeyValues::GetInt(pKVar2,(char *)0x0,0);
          }
          pKVar2 = KeyValues::FindKey(local_bdc,(char *)(unaff_EBX + 0x80eff9),false);
          pcVar3 = KeyValues::GetString(pKVar2,(char *)0x0,local_bf4);
          local_68 = ParseStringToEffectArray(pcVar3,(char (*) [64])local_28c,local_8c,8);
          local_6c = 0;
                    /* Unresolved local var: int j@[???] */
          if (0 < local_68) {
            iVar7 = 0;
            do {
              local_6c = local_6c + local_8c[iVar7];
              iVar7 = iVar7 + 1;
            } while (iVar7 < local_68);
          }
          pcVar3 = KeyValues::GetName(local_bdc);
          V_snprintf(local_bc8,0x40,pcVar3);
          uVar1 = damageInfoArray->m_Size;
          iVar7 = uVar1 + 1;
          iVar4 = (damageInfoArray->m_Memory).m_nAllocationCount;
          if (iVar4 < iVar7) {
            CUtlMemory<DamageInfo,int>::Grow(&damageInfoArray->m_Memory,iVar7 - iVar4);
          }
          damageInfoArray->m_Size = damageInfoArray->m_Size + 1;
          pDVar5 = (damageInfoArray->m_Memory).m_pMemory;
          damageInfoArray->m_pElements = pDVar5;
          iVar4 = ~uVar1 + damageInfoArray->m_Size;
          if (0 < iVar4) {
            _V_memmove(pDVar5 + iVar7,pDVar5 + uVar1,iVar4 * 0xba8);
          }
          pDVar5 = (damageInfoArray->m_Memory).m_pMemory + uVar1;
          if (pDVar5 != (DamageInfo *)0x0) {
            _memcpy(pDVar5,local_bc8,0xba8);
          }
        }
        local_bdc = KeyValues::GetNextTrueSubKey(local_bdc);
      } while (local_bdc != (KeyValues *)0x0);
    }
    if (0 < damageInfoArray->m_Size) {
      local_bd8 = 0;
      local_bd0 = 0;
      do {
        pcVar3 = ((damageInfoArray->m_Memory).m_pMemory)->damageEffectName + local_bd0;
        local_bd4 = 0;
        if (0 < *(int *)(pcVar3 + 0x4a4)) {
          do {
            pcVar3 = pcVar3 + local_bd4 * 4 + 0x464;
            pcVar3[0] = -1;
            pcVar3[1] = -1;
            pcVar3[2] = -1;
            pcVar3[3] = -1;
                    /* Unresolved local var: int k@[???] */
            if (0 < damageInfoArray->m_Size) {
              iVar4 = 0;
              iVar7 = 0;
              do {
                pDVar5 = (damageInfoArray->m_Memory).m_pMemory;
                iVar6 = _V_stricmp(pDVar5->exclusionListStrings[local_bd4] + local_bd0,
                                   pDVar5->damageEffectName + iVar7);
                if (iVar6 == 0) {
                  *(int *)((int)((damageInfoArray->m_Memory).m_pMemory)->exclusionListIndices +
                          local_bd0 + local_bd4 * 4) = iVar4;
                  break;
                }
                iVar4 = iVar4 + 1;
                iVar7 = iVar7 + 0xba8;
              } while (iVar4 < damageInfoArray->m_Size);
            }
            pcVar3 = ((damageInfoArray->m_Memory).m_pMemory)->damageEffectName + local_bd0;
            if (*(int *)(pcVar3 + local_bd4 * 4 + 0x464) == -1) {
              _Warning(unaff_EBX + 0x815faf,pcVar3 + local_bd4 * 0x40 + 100,pcVar3);
              pcVar3 = ((damageInfoArray->m_Memory).m_pMemory)->damageEffectName + local_bd0;
            }
            local_bd4 = local_bd4 + 1;
          } while (local_bd4 < *(int *)(pcVar3 + 0x4a4));
        }
        local_bd8 = local_bd8 + 1;
        local_bd0 = local_bd0 + 0xba8;
      } while (local_bd8 < damageInfoArray->m_Size);
    }
    KeyValues::deleteThis(this);
    bVar8 = true;
  }
  else {
    bVar8 = false;
  }
  if (local_20 != **(int **)(unaff_EBX + 0x9fbc0f)) {
                    /* WARNING: Subroutine does not return */
    ___stack_chk_fail();
  }
  return bVar8;
}


/* ParseStringToIntArray at 0024aad0 */

int ParseStringToIntArray(char *pString,int *pIntArray,int nMaxArray)

{
  char *pcVar1;
  int iVar2;
  int unaff_EBX;
  int local_434;
  char *local_430;
  char local_420 [1024];
  int local_20;
  
                    /* Unresolved local var: int nInts@[???]
                       Unresolved local var: char[1024] buffer@[???]
                       Unresolved local var: char * pBuffer@[???]
                       Unresolved local var: char * pSpace@[???] */
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(unaff_EBX + 0x9fc0cf);
  local_430 = local_420;
  V_strncpy(local_430,pString,0x400);
  pcVar1 = V_stristr(local_430,(char *)(unaff_EBX + 0x8163ab));
  if ((pcVar1 == (char *)0x0) || (nMaxArray == 1 || nMaxArray + -1 < 0)) {
    local_434 = 0;
  }
  else {
    iVar2 = 1;
    do {
      local_434 = iVar2;
      *pcVar1 = '\0';
      iVar2 = V_atoi(local_430);
      pIntArray[local_434 + -1] = iVar2;
      local_430 = pcVar1 + 1;
      pcVar1 = V_stristr(local_430,(char *)(unaff_EBX + 0x8163ab));
      if (pcVar1 == (char *)0x0) break;
      iVar2 = local_434 + 1;
    } while (local_434 + 1 != nMaxArray);
  }
  if (*local_430 != '\0') {
    iVar2 = V_atoi(local_430);
    pIntArray[local_434] = iVar2;
    local_434 = local_434 + 1;
  }
  if (local_20 == **(int **)(unaff_EBX + 0x9fc0cf)) {
    return local_434;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}


/* ParseStringToExclusionArray at 0024adf0 */

int ParseStringToExclusionArray(char *pString,char (*ppStringArray) [64],int nMaxArray)

{
  int iVar1;
  char *pcVar2;
  int unaff_EBX;
  char (*pDest) [64];
  char *local_438;
  int local_434;
  char local_420 [1024];
  int local_20;
  
                    /* Unresolved local var: int nStrings@[???] */
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(unaff_EBX + 0x9fbdaf);
  iVar1 = _V_strlen(pString);
  local_434 = 0;
  if (iVar1 != 0) {
                    /* Unresolved local var: char[1024] buffer@[???]
                       Unresolved local var: char * pBuffer@[???]
                       Unresolved local var: char * pSpace@[???] */
    local_438 = local_420;
    V_strncpy(local_438,pString,0x400);
    pcVar2 = V_stristr(local_438,(char *)(unaff_EBX + 0x81608b));
    if (pcVar2 == (char *)0x0) {
      iVar1 = 0;
      local_434 = 1;
    }
    else if (nMaxArray == 1 || nMaxArray + -1 < 0) {
      iVar1 = 0;
      local_434 = 1;
    }
    else {
      local_434 = 1;
      pDest = ppStringArray;
      iVar1 = local_434;
      do {
        local_434 = iVar1;
        *pcVar2 = '\0';
        V_strncpy(*pDest,local_438,0x40);
        local_438 = pcVar2 + 1;
        pcVar2 = V_stristr(local_438,(char *)(unaff_EBX + 0x81608b));
        if (pcVar2 == (char *)0x0) break;
        pDest = pDest + 1;
        iVar1 = local_434 + 1;
      } while (local_434 + 1 != nMaxArray);
      iVar1 = local_434 << 6;
      local_434 = local_434 + 1;
    }
    _V_strcpy(*ppStringArray + iVar1,local_438);
  }
  if (local_20 == **(int **)(unaff_EBX + 0x9fbdaf)) {
    return local_434;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}


/* ParseStringToGibArray at 0024ac00 */

int ParseStringToGibArray(char *pString,char *modelPath,char (*ppStringArray) [64],int nMaxArray)

{
  int iVar1;
  char *pcVar2;
  int unaff_EBX;
  char (*pDest) [64];
  char *local_448;
  char *local_438;
  int local_434;
  char local_420 [1024];
  int local_20;
  
                    /* Unresolved local var: int nStrings@[???] */
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(unaff_EBX + 0x9fbf9f);
  iVar1 = _V_strlen(pString);
  local_434 = 0;
  if (iVar1 != 0) {
                    /* Unresolved local var: char[1024] buffer@[???]
                       Unresolved local var: char * pBuffer@[???]
                       Unresolved local var: char * pSpace@[???] */
    local_438 = local_420;
    V_strncpy(local_438,pString,0x400);
    pcVar2 = V_stristr(local_438,(char *)(unaff_EBX + 0x81627b));
    if (pcVar2 == (char *)0x0) {
      iVar1 = 0;
      local_434 = 1;
    }
    else if (nMaxArray == 1 || nMaxArray + -1 < 0) {
      iVar1 = 0;
      local_434 = 1;
    }
    else {
      local_434 = 1;
      pDest = ppStringArray;
      iVar1 = local_434;
      do {
        local_434 = iVar1;
        *pcVar2 = '\0';
        V_snprintf(*pDest,0x40,(char *)(unaff_EBX + 0x81627d),modelPath,local_438);
        local_438 = pcVar2 + 1;
        pcVar2 = V_stristr(local_438,(char *)(unaff_EBX + 0x81627b));
        if (pcVar2 == (char *)0x0) break;
        pDest = pDest + 1;
        iVar1 = local_434 + 1;
      } while (local_434 + 1 != nMaxArray);
      iVar1 = local_434 << 6;
      local_434 = local_434 + 1;
    }
    local_448 = (char *)(unaff_EBX + 0x81627d);
    V_snprintf(*ppStringArray + iVar1,0x40,local_448,modelPath,local_438);
  }
  if (local_20 == **(int **)(unaff_EBX + 0x9fbf9f)) {
    return local_434;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}


/* ParseStringToEffectArray at 0024a940 */

int ParseStringToEffectArray
              (char *pString,char (*ppEffectArray) [64],int *pChanceArray,int nMaxArray)

{
  char *pSearch;
  int iVar1;
  char *pcVar2;
  char *pcVar3;
  int unaff_EBX;
  int local_434;
  char (*local_430) [64];
  char local_420 [1024];
  int local_20;
  
                    /* Unresolved local var: int nStrings@[???] */
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(unaff_EBX + 0x9fc25f);
  iVar1 = _V_strlen(pString);
  if (iVar1 != 0) {
    pcVar3 = local_420;
    V_strncpy(pcVar3,pString,0x400);
    pSearch = &UNK_0081653b + unaff_EBX;
    pcVar2 = V_stristr(pcVar3,pSearch);
    if ((pcVar2 != (char *)0x0) && (0 < nMaxArray + -1)) {
      local_430 = ppEffectArray;
      local_434 = 0;
      while( true ) {
        *pcVar2 = '\0';
        V_strncpy(*local_430,pcVar3,0x40);
        pcVar2 = pcVar2 + 1;
        pcVar3 = V_stristr(pcVar2,pSearch);
        if (pcVar3 == (char *)0x0) break;
        *pcVar3 = '\0';
        iVar1 = V_atoi(pcVar2);
        pChanceArray[local_434] = iVar1;
        pcVar3 = pcVar3 + 1;
        pcVar2 = V_stristr(pcVar3,pSearch);
        local_434 = local_434 + 1;
        if ((pcVar2 == (char *)0x0) || (local_430 = local_430 + 1, local_434 == nMaxArray + -1))
        goto LAB_0024a990;
      }
      iVar1 = V_atoi(pcVar2);
      pChanceArray[local_434] = iVar1;
      local_434 = local_434 + 1;
      goto LAB_0024a990;
    }
  }
  local_434 = 0;
LAB_0024a990:
  if (local_20 == **(int **)(unaff_EBX + 0x9fc25f)) {
    return local_434;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}


/* ParseStringToVector at 0024a860 */

void ParseStringToVector(char *pString,Vector *vector)

{
  char *pcVar1;
  char *pcVar2;
  int unaff_EBX;
  float fVar3;
  char local_420 [1024];
  int local_20;
  
                    /* Unresolved local var: char[1024] buffer@[???]
                       Unresolved local var: char * pBuffer@[???]
                       Unresolved local var: char * pSpace@[???] */
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(&DAT_009fc339 + unaff_EBX);
  V_strncpy(local_420,pString,0x400);
  pcVar1 = V_stristr(local_420,(char *)(unaff_EBX + 0x816615));
  if (pcVar1 != (char *)0x0) {
    *pcVar1 = '\0';
    fVar3 = V_atof(local_420);
    vector->x = fVar3;
    pcVar2 = V_stristr(pcVar1 + 1,(char *)(unaff_EBX + 0x816615));
    if (pcVar2 != (char *)0x0) {
      *pcVar2 = '\0';
      fVar3 = V_atof(pcVar1 + 1);
      vector->y = fVar3;
      fVar3 = V_atof(pcVar2 + 1);
      vector->z = fVar3;
    }
  }
  if (local_20 == **(int **)(&DAT_009fc339 + unaff_EBX)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}


/* _GLOBAL__I_max_hitbox_damage_effects_per_entity at 0001bd20 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_max_hitbox_damage_effects_per_entity(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

