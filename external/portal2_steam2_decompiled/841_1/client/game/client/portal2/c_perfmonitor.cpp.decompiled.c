/* DWARF-guided pseudocode for game/client/portal2/c_perfmonitor.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 00076880 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  undefined4 uVar1;
  C_PerfMonitor *this;
  FnChangeCallback_t unaff_EBX;
  longlong lVar2;
  ConVar *in_stack_ffffffd4;
  char *in_stack_ffffffd8;
  ConVar *in_stack_ffffffdc;
  char *in_stack_ffffffe0;
  char *in_stack_ffffffe4;
  int in_stack_ffffffe8;
  char *in_stack_ffffffec;
  FnChangeCallback_t callback_1;
  
  callback_1 = unaff_EBX;
  lVar2 = ___i686_get_pc_thunk_bx();
  if (lVar2 == 0xffff00000001) {
    unaff_EBX[0xc5ae4c] = (code)0x0;
    unaff_EBX[0xc5ae4d] = (code)0x0;
    unaff_EBX[0xc5ae4e] = (code)0x0;
    unaff_EBX[0xc5ae4f] = (code)0x0;
    *(undefined4 *)(unaff_EBX + 0xc5ae50) = 0;
    *(undefined4 *)(unaff_EBX + 0xc5ae54) = 0;
    *(undefined4 *)(unaff_EBX + 0xc5ae58) = 0x7f7fffff;
    *(undefined4 *)(unaff_EBX + 0xc5ae5c) = 0x7f7fffff;
    *(undefined4 *)(unaff_EBX + 0xc5ae60) = 0;
    *(undefined4 *)(unaff_EBX + 0xc5ae64) = 0;
    *(undefined4 *)(unaff_EBX + 0xc5ae68) = 0;
    *(undefined4 *)(unaff_EBX + 0xc5ae6c) = 0;
    *(undefined4 *)(unaff_EBX + 0xc5ae70) = 0x7f7fffff;
    *(undefined4 *)(unaff_EBX + 0xc5ae74) = 0x7f7fffff;
    *(undefined4 *)(unaff_EBX + 0xc5ae78) = 0x7f7fffff;
    *(undefined4 *)(unaff_EBX + 0xc5ae7c) = 0x7f7fffff;
    *(FnChangeCallback_t *)(unaff_EBX + 0xc5ae80) = unaff_EBX + 0xaee5f4;
    ConVar::ConVar((ConVar *)(unaff_EBX + 0xc5ae8c),(char *)(unaff_EBX + 0x943fef),
                   (char *)(unaff_EBX + 0x943fec),10,in_stack_ffffffd4,in_stack_ffffffd8,
                   (char *)in_stack_ffffffdc,(int)in_stack_ffffffe0);
    uVar1 = *(undefined4 *)(unaff_EBX + 0xaed4d8);
    ___cxa_atexit(unaff_EBX + 0x90561c,0,uVar1);
    ConVar::ConVar((ConVar *)(unaff_EBX + 0xc5aeec),(char *)(unaff_EBX + 0x94400e),
                   (char *)(unaff_EBX + 0x90c628),10,in_stack_ffffffd4,in_stack_ffffffd8,
                   (char *)in_stack_ffffffdc,(int)in_stack_ffffffe0);
    ___cxa_atexit(unaff_EBX + 0x9055fc,0,uVar1);
    ConVar::ConVar((ConVar *)(unaff_EBX + 0xc5af4c),(char *)(unaff_EBX + 0x944024),
                   (char *)(unaff_EBX + 0x91718c),10,in_stack_ffffffd4,in_stack_ffffffd8,
                   (char *)in_stack_ffffffdc,(int)in_stack_ffffffe0);
    ___cxa_atexit(unaff_EBX + 0x9055dc,0,uVar1);
    ConVar::ConVar((ConVar *)(unaff_EBX + 0xc5afac),(char *)(unaff_EBX + 0x94403a),
                   (char *)(unaff_EBX + 0x915d34),10,in_stack_ffffffd4,in_stack_ffffffd8,
                   (char *)in_stack_ffffffdc,(int)in_stack_ffffffe0);
    ___cxa_atexit(unaff_EBX + 0x9055bc,0,uVar1);
    ConVar::ConVar((ConVar *)(unaff_EBX + 0xc5b00c),(char *)(unaff_EBX + 0x9440b2),
                   (char *)(unaff_EBX + 0x9340c8),10,(char *)(unaff_EBX + 0x944058),
                   unaff_EBX + 0x4b5d7c,in_stack_ffffffdc,in_stack_ffffffe0,in_stack_ffffffe4,
                   in_stack_ffffffe8,in_stack_ffffffec,callback_1);
    ___cxa_atexit(unaff_EBX + 0x90559c,0,uVar1);
    this = operator_new(0x844);
    C_PerfMonitor::C_PerfMonitor(this);
    *(C_PerfMonitor **)(unaff_EBX + 0xc5b064) = this;
  }
  return;
}


/* __tcf_0 at 0097beb0 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x355868),in_stack_00000008);
  return;
}


/* __tcf_1 at 0097be90 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3558e8),in_stack_00000008);
  return;
}


/* __tcf_2 at 0097be70 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x355968),in_stack_00000008);
  return;
}


/* __tcf_3 at 0097be50 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3559e8),in_stack_00000008);
  return;
}


/* __tcf_4 at 0097be30 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x355a68),in_stack_00000008);
  return;
}


/* SendPerfMsg at 0052bd80 */

void SendPerfMsg(char *pMsg)

{
  _DevMsg(2,pMsg);
  return;
}


/* C_PerfMonitor::Update at 0052be40 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* DWARF original prototype: void Update(C_PerfMonitor * this, float frametime) */

void __thiscall C_PerfMonitor::Update(C_PerfMonitor *this,float frametime)

{
  float fVar1;
  float fVar2;
  float fVar3;
  uint uVar4;
  uint32 uVar5;
  char cVar6;
  undefined4 uVar7;
  int nSlot;
  int iVar8;
  C_WeaponPortalgun *this_00;
  C_BasePlayer *pCVar9;
  float *pfVar10;
  QAngle *pQVar11;
  int *piVar12;
  int unaff_EBX;
  uint uVar13;
  longdouble lVar14;
  float fVar15;
  float fVar16;
  double dVar17;
  float fVar18;
  double dVar19;
  double dVar20;
  undefined8 in_stack_ffffeed4;
  undefined4 uVar21;
  int local_1084;
  float local_1064;
  QAngle local_1054;
  Vector local_1048;
  QAngle local_103c;
  Vector local_1030;
  int local_1024;
  char local_1020 [2048];
  char local_820 [2048];
  int local_20;
  
                    /* Unresolved local var: float fCurrTime@[???]
                       Unresolved local var: float fElapsedTime@[???]
                       Unresolved local var: float fFramerate@[???]
                       Unresolved local var: float fAvgFrameTime@[???]
                       Unresolved local var: int iFramesToAvg@[???]
                       Unresolved local var: float fSMAFramerate@[???]
                       Unresolved local var: char[2048] szLowFrameMsg@[???]
                       Unresolved local var: int iCurrChar@[???] */
  uVar21 = (undefined4)((ulonglong)in_stack_ffffeed4 >> 0x20);
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(&DAT_00637f0f + unaff_EBX);
  if (*(int *)(*(int *)(unaff_EBX + 0x7a594b) + 0x30) != 0) {
    lVar14 = (longdouble)_Plat_FloatTime();
    fVar18 = (float)lVar14;
    fVar15 = fVar18 - this->m_fPreviousFrameTime;
    this->m_fPreviousFrameTime = fVar18;
    this->m_uTotalFrames = this->m_uTotalFrames + 1;
    if (this->m_fPeakFrametime <= fVar15 && fVar15 != this->m_fPeakFrametime) {
      this->m_fPeakFrametime = fVar15;
    }
    if (fVar15 < this->m_fMinFrametime) {
      this->m_fMinFrametime = fVar15;
    }
    fVar16 = fVar18 - this->m_fLastLowTime;
    pfVar10 = (float *)(*(int *)(unaff_EBX + 0x7a59ab) + 0x2c);
    if ((*pfVar10 <= fVar16 && fVar16 != *pfVar10) && (this->m_szLastPerfMsg[0] != '\0')) {
      this->m_fLastLowTime = fVar18;
      this->m_fLastLowFramerate = 3.4028235e+38;
                    /* Unresolved local var: char[2048] strTemp@[???] */
      V_snprintf(local_820,0x800,this->m_szLastPerfMsg);
      _DevMsg(2,local_820);
      this->m_szLastPerfMsg[0] = '\0';
      this->m_uFramesBelowDesired = 0;
    }
    (this->m_FrametimeHistory).m_Memory.m_pMemory[this->m_uOldestFrame] = fVar15;
    uVar4 = *(uint *)(*(int *)(unaff_EBX + 0x7a5a6b) + 0x30);
    if ((int)uVar4 < 1) {
      local_1064 = *(float *)(unaff_EBX + 0x4bb377);
    }
    else {
      local_1064 = 0.0;
      uVar13 = 0;
      do {
        local_1064 = local_1064 +
                     (this->m_FrametimeHistory).m_Memory.m_pMemory
                     [(uVar13 + this->m_uOldestFrame) % uVar4] / (float)(int)uVar4;
        uVar13 = uVar13 + 1;
      } while (uVar13 != uVar4);
      local_1064 = *(float *)(unaff_EBX + 0x4b6333) / local_1064;
    }
    this->m_uOldestFrame = (this->m_uOldestFrame + 1) % uVar4;
    if ((local_1064 <= (float)*(int *)(*(int *)(unaff_EBX + 0x7a58eb) + 0x30)) &&
       (fVar15 = *(float *)(unaff_EBX + 0x4b6333) / fVar15,
       fVar15 < this->m_fLastLowFramerate || fVar15 == this->m_fLastLowFramerate)) {
      this->m_fLastLowFramerate = local_1064;
      uVar5 = this->m_uTotalFrames;
                    /* Unresolved local var: time_t now@[???] */
      _time(&local_1024);
      uVar7 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x637f53) + 0xd0))
                        ((int *)**(undefined4 **)(unaff_EBX + 0x637f53));
      dVar19 = (double)CONCAT44(uVar21,uVar5);
      dVar17 = (double)local_1064;
      dVar20 = (double)(longlong)local_1024;
      local_1084 = V_snprintf(local_1020,0x800,(char *)(unaff_EBX + 0x48e837),uVar7,local_1024,
                              local_1024 >> 0x1f,dVar17,uVar5);
                    /* Unresolved local var: int hh@[???] */
      nSlot = FirstValidSplitScreenSlot();
      uVar21 = (undefined4)((ulonglong)dVar20 >> 0x20);
      if (nSlot != -1) {
        do {
          pCVar9 = C_BasePlayer::GetLocalPlayer(nSlot);
          if (pCVar9 != (C_BasePlayer *)0x0) {
            pfVar10 = (float *)(**(code **)(*(int *)&(pCVar9->super_C_BaseCombatCharacter).
                                                     super_C_BaseFlex.super_C_BaseAnimatingOverlay.
                                                     super_C_BaseAnimating.super_C_BaseEntity.
                                                     super_IClientEntity.super_IClientUnknown + 0x34
                                           ))(pCVar9);
            fVar15 = *pfVar10;
            fVar18 = pfVar10[1];
            fVar16 = pfVar10[2];
            pQVar11 = MainViewAngles(nSlot);
            fVar1 = pQVar11->x;
            fVar2 = pQVar11->y;
            fVar3 = pQVar11->z;
            cVar6 = (**(code **)(*(int *)&(pCVar9->super_C_BaseCombatCharacter).super_C_BaseFlex.
                                          super_C_BaseAnimatingOverlay.super_C_BaseAnimating.
                                          super_C_BaseEntity.super_IClientEntity.
                                          super_IClientUnknown + 0x304))(pCVar9);
            if (cVar6 == '\0') {
              piVar12 = (int *)0x0;
            }
            else {
              piVar12 = (int *)___dynamic_cast(pCVar9,*(undefined4 *)(unaff_EBX + 0x637f27),
                                               *(undefined4 *)(unaff_EBX + 0x63815f),0);
            }
            local_1030.x = 0.0;
            local_1030.y = 0.0;
            local_1030.z = 0.0;
            local_103c.x = 0.0;
            local_103c.y = 0.0;
            local_103c.z = 0.0;
            local_1048.x = 0.0;
            local_1048.y = 0.0;
            local_1048.z = 0.0;
            local_1054.x = 0.0;
            local_1054.y = 0.0;
            local_1054.z = 0.0;
            iVar8 = (**(code **)(*piVar12 + 0x490))(piVar12);
            if (iVar8 != 0) {
              this_00 = (C_WeaponPortalgun *)
                        ___dynamic_cast(iVar8,*(undefined4 *)(unaff_EBX + 0x6380c7),
                                        *(undefined4 *)(unaff_EBX + 0x6386f3),0);
              if (this_00 != (C_WeaponPortalgun *)0x0) {
                C_WeaponPortalgun::GetPrimaryPortalPosition(this_00,&local_1030,&local_103c);
                C_WeaponPortalgun::GetSecondaryPortalPosition(this_00,&local_1048,&local_1054);
              }
            }
            dVar19 = (double)fVar16;
            dVar17 = (double)fVar18;
            dVar20 = (double)fVar15;
            iVar8 = V_snprintf(local_1020 + local_1084,0x800 - local_1084,
                               (char *)(unaff_EBX + 0x48e88b),nSlot,dVar20,dVar17,dVar19,
                               (double)fVar1,(double)fVar2,(double)fVar3,(double)local_1030.x,
                               (double)local_1030.y,(double)local_1030.z,(double)local_103c.x,
                               (double)local_103c.y,(double)local_103c.z,(double)local_1048.x,
                               (double)local_1048.y,(double)local_1048.z,(double)local_1054.x,
                               (double)local_1054.y,(double)local_1054.z);
            local_1084 = local_1084 + iVar8;
          }
          nSlot = NextValidSplitScreenSlot(nSlot);
          uVar21 = (undefined4)((ulonglong)dVar20 >> 0x20);
        } while (nSlot != -1);
      }
      V_snprintf(this->m_szLastPerfMsg,0x800,(char *)(unaff_EBX + 0x496b3b),local_1020,
                 unaff_EBX + 0x48e99b,uVar21,dVar17,dVar19);
    }
  }
  if (local_20 == **(int **)(&DAT_00637f0f + unaff_EBX)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}


/* C_PerfMonitor::LevelShutdownPreEntity at 0052c490 */

/* DWARF original prototype: void LevelShutdownPreEntity(C_PerfMonitor * this) */

void __thiscall C_PerfMonitor::LevelShutdownPreEntity(C_PerfMonitor *this)

{
  float fVar1;
  float fVar2;
  float fVar3;
  double dVar4;
  uint32 uVar5;
  uint32 uVar6;
  undefined4 uVar7;
  int unaff_EBX;
  longdouble lVar8;
  int local_824;
  char local_820 [2048];
  int local_20;
  
  ___i686_get_pc_thunk_bx();
  local_20 = **(int **)(unaff_EBX + 0x6378b9);
  if (*(int *)(*(int *)(unaff_EBX + 0x7a52f5) + 0x30) != 0) {
                    /* Unresolved local var: char[2048] strTemp@[???] */
    uVar5 = this->m_uTotalFrames;
    dVar4 = *(double *)(unaff_EBX + 0x4bad29);
    lVar8 = (longdouble)_Plat_FloatTime();
    fVar1 = this->m_fLevelStartTime;
    fVar2 = this->m_fPeakFrametime;
    fVar3 = this->m_fMinFrametime;
    uVar6 = this->m_uTotalFrames;
                    /* Unresolved local var: time_t now@[???] */
    _time(&local_824);
    uVar7 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x6378fd) + 0xd0))
                      ((int *)**(undefined4 **)(unaff_EBX + 0x6378fd));
    V_snprintf(local_820,0x800,(char *)(unaff_EBX + 0x48e365),uVar7,local_824,local_824 >> 0x1f,
               uVar6,(double)fVar3,(double)fVar2,
               ((double)(int)(uVar5 + 0x80000000) + dVar4) / ((double)lVar8 - (double)fVar1));
    _DevMsg(2,local_820);
  }
  if (local_20 == **(int **)(unaff_EBX + 0x6378b9)) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}


/* C_PerfMonitor::~C_PerfMonitor at 0052c950 */

/* DWARF original prototype: void ~C_PerfMonitor(C_PerfMonitor * this, int __in_chrg) */

void __thiscall C_PerfMonitor::~C_PerfMonitor(C_PerfMonitor *this,int __in_chrg)

{
  float *pfVar1;
  int unaff_EBX;
  float *in_stack_ffffffc8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CAutoGameSystemPerFrame).super_CBaseGameSystemPerFrame.super_IGameSystemPerFrame.
  super_IGameSystem._vptr_IGameSystem = (_func_int_varargs **)(unaff_EBX + 0x69ee24);
  (this->m_FrametimeHistory).m_Size = 0;
  if ((this->m_FrametimeHistory).m_Memory.m_nGrowSize < 0) {
    pfVar1 = (this->m_FrametimeHistory).m_Memory.m_pMemory;
  }
  else {
    pfVar1 = (this->m_FrametimeHistory).m_Memory.m_pMemory;
    if (pfVar1 != (float *)0x0) {
      in_stack_ffffffc8 = pfVar1;
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x63740c) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x63740c));
      pfVar1 = (float *)0x0;
      (this->m_FrametimeHistory).m_Memory.m_pMemory = (float *)0x0;
    }
    (this->m_FrametimeHistory).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_FrametimeHistory).m_pElements = pfVar1;
  if (-1 < (this->m_FrametimeHistory).m_Memory.m_nGrowSize) {
    if (pfVar1 != (float *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x63740c) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x63740c));
      (this->m_FrametimeHistory).m_Memory.m_pMemory = (float *)0x0;
      in_stack_ffffffc8 = pfVar1;
    }
    (this->m_FrametimeHistory).m_Memory.m_nAllocationCount = 0;
  }
  (this->super_CAutoGameSystemPerFrame).super_CBaseGameSystemPerFrame.super_IGameSystemPerFrame.
  super_IGameSystem._vptr_IGameSystem = (_func_int_varargs **)(unaff_EBX + 0x638964);
  IGameSystemPerFrame::~IGameSystemPerFrame((IGameSystemPerFrame *)this,(int)in_stack_ffffffc8);
  operator_delete(this);
  return;
}


/* C_PerfMonitor::~C_PerfMonitor at 0052ca80 */

/* DWARF original prototype: void ~C_PerfMonitor(C_PerfMonitor * this, int __in_chrg) */

void __thiscall C_PerfMonitor::~C_PerfMonitor(C_PerfMonitor *this,int __in_chrg)

{
  float *pfVar1;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CAutoGameSystemPerFrame).super_CBaseGameSystemPerFrame.super_IGameSystemPerFrame.
  super_IGameSystem._vptr_IGameSystem = (_func_int_varargs **)(unaff_EBX + 0x69ecf4);
  (this->m_FrametimeHistory).m_Size = 0;
  if ((this->m_FrametimeHistory).m_Memory.m_nGrowSize < 0) {
    pfVar1 = (this->m_FrametimeHistory).m_Memory.m_pMemory;
  }
  else {
    pfVar1 = (this->m_FrametimeHistory).m_Memory.m_pMemory;
    if (pfVar1 != (float *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x6372dc) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x6372dc),pfVar1);
      (this->m_FrametimeHistory).m_Memory.m_pMemory = (float *)0x0;
    }
    pfVar1 = (float *)0x0;
    (this->m_FrametimeHistory).m_Memory.m_nAllocationCount = 0;
  }
  (this->m_FrametimeHistory).m_pElements = pfVar1;
  if (-1 < (this->m_FrametimeHistory).m_Memory.m_nGrowSize) {
    if (pfVar1 != (float *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x6372dc) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x6372dc),pfVar1);
      (this->m_FrametimeHistory).m_Memory.m_pMemory = (float *)0x0;
    }
    (this->m_FrametimeHistory).m_Memory.m_nAllocationCount = 0;
  }
  (this->super_CAutoGameSystemPerFrame).super_CBaseGameSystemPerFrame.super_IGameSystemPerFrame.
  super_IGameSystem._vptr_IGameSystem = (_func_int_varargs **)(unaff_EBX + 0x638834);
  IGameSystemPerFrame::~IGameSystemPerFrame((IGameSystemPerFrame *)this,__in_chrg);
  return;
}


/* C_PerfMonitor::C_PerfMonitor at 00076800 */

/* DWARF original prototype: void C_PerfMonitor(C_PerfMonitor * this) */

void __thiscall C_PerfMonitor::C_PerfMonitor(C_PerfMonitor *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CAutoGameSystemPerFrame::CAutoGameSystemPerFrame
            (&this->super_CAutoGameSystemPerFrame,(char *)(unaff_EBX + 0x944063));
  (this->super_CAutoGameSystemPerFrame).super_CBaseGameSystemPerFrame.super_IGameSystemPerFrame.
  super_IGameSystem._vptr_IGameSystem = (_func_int_varargs **)(&UNK_00b54f77 + unaff_EBX);
  this->m_uFramesBelowDesired = 0;
  this->m_fPreviousFrameTime = 0.0;
  (this->m_FrametimeHistory).m_Memory.m_pMemory = (float *)0x0;
  (this->m_FrametimeHistory).m_Memory.m_nAllocationCount = 0;
  (this->m_FrametimeHistory).m_Memory.m_nGrowSize = 0;
  (this->m_FrametimeHistory).m_Size = 0;
  (this->m_FrametimeHistory).m_pElements = (float *)0x0;
  this->m_uOldestFrame = 0;
  this->m_szLastPerfMsg[0] = '\0';
  return;
}


/* C_PerfMonitor::C_PerfMonitor at 0052c7f0 */

/* DWARF original prototype: void C_PerfMonitor(C_PerfMonitor * this) */

void __thiscall C_PerfMonitor::C_PerfMonitor(C_PerfMonitor *this)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  CAutoGameSystemPerFrame::CAutoGameSystemPerFrame
            (&this->super_CAutoGameSystemPerFrame,(char *)(unaff_EBX + 0x48e073));
  (this->super_CAutoGameSystemPerFrame).super_CBaseGameSystemPerFrame.super_IGameSystemPerFrame.
  super_IGameSystem._vptr_IGameSystem = (_func_int_varargs **)(unaff_EBX + 0x69ef87);
  this->m_uFramesBelowDesired = 0;
  this->m_fPreviousFrameTime = 0.0;
  (this->m_FrametimeHistory).m_Memory.m_pMemory = (float *)0x0;
  (this->m_FrametimeHistory).m_Memory.m_nAllocationCount = 0;
  (this->m_FrametimeHistory).m_Memory.m_nGrowSize = 0;
  (this->m_FrametimeHistory).m_Size = 0;
  (this->m_FrametimeHistory).m_pElements = (float *)0x0;
  this->m_uOldestFrame = 0;
  this->m_szLastPerfMsg[0] = '\0';
  return;
}


/* C_PerfMonitor::Reset at 0052c6c0 */

/* DWARF original prototype: void Reset(C_PerfMonitor * this) */

void __thiscall C_PerfMonitor::Reset(C_PerfMonitor *this)

{
  int *piVar1;
  float *src;
  float fVar2;
  int iVar3;
  int iVar4;
  int unaff_EBX;
  longdouble lVar5;
  
                    /* Unresolved local var: double dbCurrTime@[???] */
  ___i686_get_pc_thunk_bx();
  iVar4 = *(int *)(*(int *)(unaff_EBX + 0x7a51ee) + 0x30);
  (this->m_FrametimeHistory).m_Size = 0;
  if (iVar4 != 0) {
    iVar3 = (this->m_FrametimeHistory).m_Memory.m_nAllocationCount;
    if (iVar3 < iVar4) {
      CUtlMemory<float,int>::Grow(&(this->m_FrametimeHistory).m_Memory,iVar4 - iVar3);
    }
    piVar1 = &(this->m_FrametimeHistory).m_Size;
    *piVar1 = *piVar1 + iVar4;
    src = (this->m_FrametimeHistory).m_Memory.m_pMemory;
    (this->m_FrametimeHistory).m_pElements = src;
                    /* Unresolved local var: int numToMove@[???] */
    iVar3 = (this->m_FrametimeHistory).m_Size - iVar4;
    if ((0 < iVar3) && (0 < iVar4)) {
      _V_memmove(src + iVar4,src,iVar3 * 4);
    }
                    /* Unresolved local var: int i@[???] */
    if (0 < (this->m_FrametimeHistory).m_Size) {
      iVar4 = 0;
      do {
        (this->m_FrametimeHistory).m_Memory.m_pMemory[iVar4] = 1.1754944e-38;
        iVar4 = iVar4 + 1;
      } while (iVar4 < (this->m_FrametimeHistory).m_Size);
    }
  }
  lVar5 = (longdouble)_Plat_FloatTime();
  fVar2 = (float)lVar5;
  this->m_fLevelStartTime = fVar2;
  this->m_fMinFrametime = 3.4028235e+38;
  this->m_fPeakFrametime = 0.0;
  this->m_fPreviousFrameTime = fVar2;
  this->m_uTotalFrames = 0;
  this->m_fLastLowFramerate = 3.4028235e+38;
  this->m_fLastLowTime = fVar2;
  this->m_szLastPerfMsg[0] = '\0';
  return;
}


/* C_PerfMonitor::Init at 0052c7d0 */

/* DWARF original prototype: bool Init(C_PerfMonitor * this) */

bool __thiscall C_PerfMonitor::Init(C_PerfMonitor *this)

{
  Reset(this);
  return true;
}


/* C_PerfMonitor::Shutdown at 0052bd30 */

/* DWARF original prototype: void Shutdown(C_PerfMonitor * this) */

void __thiscall C_PerfMonitor::Shutdown(C_PerfMonitor *this)

{
  return;
}


/* C_PerfMonitor::LevelInitPostEntity at 0052c7c0 */

/* DWARF original prototype: void LevelInitPostEntity(C_PerfMonitor * this) */

void __thiscall C_PerfMonitor::LevelInitPostEntity(C_PerfMonitor *this)

{
  Reset(this);
  return;
}


/* C_PerfMonitor::PreRender at 0052bd40 */

/* DWARF original prototype: void PreRender(C_PerfMonitor * this) */

void __thiscall C_PerfMonitor::PreRender(C_PerfMonitor *this)

{
  return;
}


/* C_PerfMonitor::PostRender at 0052bd50 */

/* DWARF original prototype: void PostRender(C_PerfMonitor * this) */

void __thiscall C_PerfMonitor::PostRender(C_PerfMonitor *this)

{
  return;
}


/* C_PerfMonitor::GetLocalTimeInSeconds at 0052bd60 */

/* DWARF original prototype: uint64 GetLocalTimeInSeconds(C_PerfMonitor * this) */

uint64 __thiscall C_PerfMonitor::GetLocalTimeInSeconds(C_PerfMonitor *this)

{
  int local_10 [3];
  
                    /* Unresolved local var: time_t now@[???] */
  _time(local_10);
  return (uint64)local_10[0];
}


/* C_PerfMonitor::UpdateFramesToAverage at 0052bda0 */

/* DWARF original prototype: void UpdateFramesToAverage(C_PerfMonitor * this) */

void __thiscall C_PerfMonitor::UpdateFramesToAverage(C_PerfMonitor *this)

{
  int *piVar1;
  int iVar2;
  float *src;
  int iVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  iVar2 = *(int *)(*(int *)(unaff_EBX + 0x7a5b08) + 0x30);
  (this->m_FrametimeHistory).m_Size = 0;
  if (iVar2 != 0) {
    iVar3 = (this->m_FrametimeHistory).m_Memory.m_nAllocationCount;
    if (iVar3 < iVar2) {
      CUtlMemory<float,int>::Grow(&(this->m_FrametimeHistory).m_Memory,iVar2 - iVar3);
    }
    piVar1 = &(this->m_FrametimeHistory).m_Size;
    *piVar1 = *piVar1 + iVar2;
    src = (this->m_FrametimeHistory).m_Memory.m_pMemory;
    (this->m_FrametimeHistory).m_pElements = src;
                    /* Unresolved local var: int numToMove@[???] */
    iVar3 = (this->m_FrametimeHistory).m_Size - iVar2;
    if ((0 < iVar3) && (0 < iVar2)) {
      _V_memmove(src + iVar2,src,iVar3 * 4);
    }
  }
  return;
}


/* cl_perfmon_frames_to_average_changed_f at 0052c610 */

void cl_perfmon_frames_to_average_changed_f(IConVar *var,char *pOldValue,float flOldValue)

{
  int iVar1;
  void *src;
  int iVar2;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  iVar2 = *(int *)(unaff_EBX + 0x7a52d4);
  iVar1 = *(int *)(*(int *)(unaff_EBX + 0x7a5298) + 0x30);
  *(undefined4 *)(iVar2 + 0x838) = 0;
  if (iVar1 != 0) {
    if (*(int *)(iVar2 + 0x830) < iVar1) {
      CUtlMemory<float,int>::Grow
                ((CUtlMemory<float,int> *)(iVar2 + 0x82c),iVar1 - *(int *)(iVar2 + 0x830));
    }
    *(int *)(iVar2 + 0x838) = *(int *)(iVar2 + 0x838) + iVar1;
    src = *(void **)(iVar2 + 0x82c);
    *(void **)(iVar2 + 0x83c) = src;
    iVar2 = *(int *)(iVar2 + 0x838) - iVar1;
    if ((0 < iVar2) && (0 < iVar1)) {
      _V_memmove((void *)((int)src + iVar1 * 4),src,iVar2 * 4);
      return;
    }
  }
  return;
}


/* _GLOBAL__I__Z11SendPerfMsgPKc at 00076ad0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__Z11SendPerfMsgPKc(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

