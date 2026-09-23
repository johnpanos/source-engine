/* DWARF-guided pseudocode for game/client/portal2/c_paint_input.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 00075b80 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  undefined4 uVar1;
  int unaff_EBX;
  char *unaff_ESI;
  longlong lVar2;
  ConVar *this_1;
  ConVar *in_stack_ffffffe8;
  char *in_stack_ffffffec;
  char *in_stack_fffffff0;
  int flags_1;
  
  flags_1 = unaff_EBX;
  lVar2 = ___i686_get_pc_thunk_bx();
  if (lVar2 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x41e].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined1 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x41e].m_pNext + unaff_EBX)
       = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x41e].m_pNext + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x41e].m_pNext + unaff_EBX + 2) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x41e].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x41f].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x41f].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x41f].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x41f].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x420].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x420].m_SerialNumber + unaff_EBX + 3) =
       0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x420].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x420].m_pNext + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x421].m_pEntity + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x421].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x421].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined **)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x421].m_pNext + unaff_EBX + 3) =
       &UNK_00abc277 + unaff_EBX;
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x48f),(char *)(unaff_EBX + 0x919d8f)
                 ,(char *)(unaff_EBX + 0x919d89),0x80,(char *)(unaff_EBX + 0x919d77),
                 in_stack_ffffffe8,in_stack_ffffffec,in_stack_fffffff0,flags_1,unaff_ESI);
  uVar1 = *(undefined4 *)(&DAT_00abb18f + unaff_EBX);
  ___cxa_atexit(&UNK_008dbcaf + unaff_EBX,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x4ef),&UNK_009136dd + unaff_EBX,
                 (char *)(unaff_EBX + 0x8fa0ff),0x80,(char *)(unaff_EBX + 0x919d95),
                 in_stack_ffffffe8,in_stack_ffffffec,in_stack_fffffff0,flags_1,unaff_ESI);
  ___cxa_atexit(unaff_EBX + 0x8dbc8f,0,uVar1);
  this_1 = (ConVar *)(&UNK_00919dab + unaff_EBX);
  ConVar::ConVar((ConVar *)
                 ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x422].m_pPrev +
                 unaff_EBX + 3),(char *)(unaff_EBX + 0x919dc3),&UNK_008eb56b + unaff_EBX,0x80,
                 (char *)this_1,in_stack_ffffffe8,in_stack_ffffffec,in_stack_fffffff0,flags_1,
                 unaff_ESI);
  ___cxa_atexit(unaff_EBX + 0x8dbc6f,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x54f),(char *)(unaff_EBX + 0x919dd2)
                 ,(char *)(unaff_EBX + 0x919dcd),0,this_1,(char *)in_stack_ffffffe8,
                 in_stack_ffffffec,(int)in_stack_fffffff0);
  ___cxa_atexit(unaff_EBX + 0x8dbc4f,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x5af),(char *)(unaff_EBX + 0x919de2)
                 ,(char *)(unaff_EBX + 0x919dde),0,this_1,(char *)in_stack_ffffffe8,
                 in_stack_ffffffec,(int)in_stack_fffffff0);
  ___cxa_atexit(unaff_EBX + 0x8dbc2f,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x60f),(char *)(unaff_EBX + 0x919df7)
                 ,(char *)(unaff_EBX + 0x8f1091),2,this_1,(char *)in_stack_ffffffe8,
                 in_stack_ffffffec,(int)in_stack_fffffff0);
  ___cxa_atexit(unaff_EBX + 0x8dbc0f,0,uVar1);
  return;
}


/* __tcf_0 at 00951840 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fe038),in_stack_00000008);
  return;
}


/* __tcf_1 at 00951820 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fe0b8),in_stack_00000008);
  return;
}


/* __tcf_2 at 00951800 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x366718),in_stack_00000008);
  return;
}


/* __tcf_3 at 009517e0 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fe158),in_stack_00000008);
  return;
}


/* __tcf_4 at 009517c0 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fe1d8),in_stack_00000008);
  return;
}


/* __tcf_5 at 009517a0 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3fe258),in_stack_00000008);
  return;
}


/* C_Paint_Input::ApplyMouse at 00528a70 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void ApplyMouse(C_Paint_Input * this, int nSlot, QAngle * viewangles,
   CUserCmd * cmd, float mouse_x, float mouse_y) */

void __thiscall
C_Paint_Input::ApplyMouse
          (C_Paint_Input *this,int nSlot,QAngle *viewangles,CUserCmd *cmd,float mouse_x,
          float mouse_y)

{
  float *pfVar1;
  undefined4 *puVar2;
  vec_t vVar3;
  vec_t vVar4;
  bool bVar5;
  C_Portal_Player *this_00;
  C_PortalPlayerLocalData *pCVar6;
  Split_t *pSVar7;
  int iVar8;
  int unaff_EBX;
  longdouble lVar9;
  longdouble lVar10;
  float fVar11;
  double dVar12;
  float fVar13;
  float fVar14;
  Vector *local_110;
  float local_f8;
  float local_f4;
  float local_f0;
  VMatrix local_ec;
  float local_ac;
  float local_a8;
  float local_a4;
  QAngle local_a0;
  QAngle local_94;
  float local_88;
  float local_84;
  float local_80;
  float local_7c;
  float local_78;
  float local_74;
  float local_70;
  float local_6c;
  float local_68;
  float local_64;
  float local_60;
  float local_5c;
  float local_58;
  float local_54;
  float local_50;
  Vector local_4c;
  Vector local_40;
  Vector local_34;
  Vector local_28 [2];
  
                    /* Unresolved local var: C_Portal_Player * pPlayer@[???]
                       Unresolved local var: Vector vForward@[???]
                       Unresolved local var: Vector vRight@[???]
                       Unresolved local var: Vector vUp@[???]
                       Unresolved local var: Vector vPlayerUp@[???]
                       Unresolved local var: Vector vStickNormal@[???]
                       Unresolved local var: bool bShouldDampInput@[???]
                       Unresolved local var: QAngle newAngles@[???]
                       Unresolved local var: QAngle viewOffset@[???] */
  ___i686_get_pc_thunk_bx();
  this_00 = (C_Portal_Player *)C_BasePlayer::GetLocalPlayer(-1);
  if (this_00 != (C_Portal_Player *)0x0) {
    pCVar6 = C_Portal_Player::GetPortalPlayerLocalData(this_00);
    local_4c.x = (pCVar6->m_Up).x;
    local_4c.y = (pCVar6->m_Up).y;
    local_4c.z = (pCVar6->m_Up).z;
    C_Portal_Player::GetPortalPlayerLocalData(this_00);
    AngleVectors(viewangles,local_28,&local_34,&local_40);
    bVar5 = C_Portal_Player::IsDoneReorienting(this_00);
    if ((!bVar5) &&
       (pCVar6 = C_Portal_Player::GetPortalPlayerLocalData(this_00),
       pCVar6->m_nStickCameraState == STICK_CAMERA_SURFACE_TRANSITION)) {
      local_f8 = C_Portal_Player::GetReorientationProgress(this_00);
      fVar13 = *(float *)(*(int *)(unaff_EBX + 0x826f9b) + 0x2c);
      if (fVar13 <= local_f8) {
        if (local_f8 < *(float *)(unaff_EBX + 0x48ddc3)) {
          local_f8 = (local_f8 - fVar13) / (*(float *)(unaff_EBX + 0x48ddc3) - fVar13);
        }
      }
      else {
        local_f8 = 0.0;
      }
      mouse_y = mouse_y * local_f8;
    }
    pSVar7 = kbutton_t::GetPerUser(*(kbutton_t **)(&DAT_006089ff + unaff_EBX),nSlot);
    if (((pSVar7->state & 1) == 0) &&
       (*(int *)(*(int *)(*(int *)(&DAT_006089cf + unaff_EBX) + 0x1c) + 0x30) == 0)) {
      iVar8 = (*(this->super_CInput).super_IInput._vptr_IInput[0x1c])(this);
      if ((iVar8 == 0) || (bVar5 = C_Portal_Player::IsTaunting(this_00), !bVar5)) {
                    /* Unresolved local var: VMatrix rotMatrix@[???]
                       Unresolved local var: float angle@[???] */
        MatrixBuildRotationAboutAxis
                  (&local_ec,&local_4c,
                   (float)(*(uint *)(*(int *)(&DAT_00826e1b + unaff_EBX) + 0x2c) ^
                          *(uint *)(unaff_EBX + 0x492c5f)) * mouse_x);
                    /* Unresolved local var: Vector vRet@[???] */
        local_64 = local_ec.m[0][0] * local_34.x + local_ec.m[0][1] * local_34.y +
                   local_ec.m[0][2] * local_34.z + local_ec.m[0][3];
        local_60 = local_ec.m[1][0] * local_34.x + local_ec.m[1][1] * local_34.y +
                   local_ec.m[1][2] * local_34.z + local_ec.m[1][3];
        local_5c = local_ec.m[2][0] * local_34.x + local_ec.m[2][1] * local_34.y +
                   local_ec.m[2][2] * local_34.z + local_ec.m[2][3];
        local_34.x = local_64;
        local_34.y = local_60;
        local_34.z = local_5c;
      }
      else if ((mouse_x != 0.0) && (this_00->m_bTauntInterpolatingAngles == false)) {
                    /* Unresolved local var: Vector vAngle@[???] */
        pfVar1 = *(float **)(&DAT_00608a83 + unaff_EBX);
        local_ec.m[0][0] = *pfVar1;
        local_54 = pfVar1[1];
        local_ec.m[0][2] = pfVar1[2];
        local_ec.m[0][1] =
             local_54 - mouse_x * *(float *)(*(int *)(&DAT_00826e1b + unaff_EBX) + 0x2c);
        *pfVar1 = local_ec.m[0][0];
        pfVar1[1] = local_ec.m[0][1];
        pfVar1[2] = local_ec.m[0][2];
        local_58 = local_ec.m[0][0];
        local_50 = local_ec.m[0][2];
      }
    }
    else {
      cmd->sidemove = mouse_x * *(float *)(*(int *)(unaff_EBX + 0x826e7b) + 0x2c) + cmd->sidemove;
    }
    local_110 = &local_4c;
    pSVar7 = kbutton_t::GetPerUser(*(kbutton_t **)(&DAT_006089ff + unaff_EBX),nSlot);
    if ((pSVar7->state & 1) == 0) {
      iVar8 = (*(this->super_CInput).super_IInput._vptr_IInput[0x1c])(this);
      if ((iVar8 == 0) || (bVar5 = C_Portal_Player::IsTaunting(this_00), !bVar5)) {
                    /* Unresolved local var: float pitch@[???]
                       Unresolved local var: VMatrix rotMatrix@[???] */
        fVar13 = local_28[0].x * local_4c.x + local_28[0].y * local_4c.y +
                 local_28[0].z * local_4c.z;
        if (fVar13 < *(float *)(unaff_EBX + 0x48ddc3) || fVar13 == *(float *)(unaff_EBX + 0x48ddc3))
        {
          dVar12 = (double)fVar13;
          if ((fVar13 < *(float *)(unaff_EBX + 0x48de8b)) &&
             (!NAN(fVar13) && !NAN(*(float *)(unaff_EBX + 0x48de8b)))) {
            dVar12 = *(double *)(unaff_EBX + 0x48df07);
          }
        }
        else {
          dVar12 = *(double *)(unaff_EBX + 0x48deff);
        }
        lVar9 = (longdouble)_acos(dVar12);
        fVar13 = *(float *)(unaff_EBX + 0x48eea7);
        fVar11 = *(float *)(unaff_EBX + 0x48e033);
        lVar10 = (longdouble)
                 (**(code **)(*(int *)**(undefined4 **)(&DAT_006089f7 + unaff_EBX) + 0x68))
                           ((int *)**(undefined4 **)(&DAT_006089f7 + unaff_EBX));
        fVar14 = ((float)lVar9 * fVar13 + fVar11) - (float)lVar10 * mouse_y;
        fVar13 = *(float *)(*(int *)(unaff_EBX + 0x826f3b) + 0x2c);
        fVar11 = *(float *)(*(int *)(*(int *)(&DAT_006089fb + unaff_EBX) + 0x1c) + 0x2c) - fVar13;
        if ((fVar14 <= fVar11) &&
           (fVar11 = fVar13 - *(float *)(*(int *)(*(int *)(&DAT_006089db + unaff_EBX) + 0x1c) + 0x2c
                                        ), fVar11 <= fVar14)) {
          fVar11 = fVar14;
        }
        MatrixBuildRotationAboutAxis(&local_ec,&local_34,fVar11);
        local_ac = local_4c.y * local_34.z - local_34.y * local_4c.z;
        local_a8 = local_4c.z * local_34.x - local_34.z * local_4c.x;
        local_a4 = local_4c.x * local_34.y - local_4c.y * local_34.x;
                    /* Unresolved local var: Vector vRet@[???] */
        local_88 = local_ec.m[0][0] * local_ac + local_ec.m[0][1] * local_a8 +
                   local_a4 * local_ec.m[0][2] + local_ec.m[0][3];
        local_84 = local_ec.m[1][0] * local_ac + local_ec.m[1][1] * local_a8 +
                   local_ec.m[1][2] * local_a4 + local_ec.m[1][3];
        local_80 = local_ec.m[2][0] * local_ac + local_ec.m[2][1] * local_a8 +
                   local_ec.m[2][2] * local_a4 + local_ec.m[2][3];
        local_7c = local_ac;
        local_78 = local_a8;
        local_74 = local_a4;
        local_28[0].x = local_88;
        local_28[0].y = local_84;
        local_28[0].z = local_80;
      }
      else if ((mouse_y != 0.0) && (this_00->m_bTauntInterpolatingAngles == false)) {
        pfVar1 = *(float **)(&DAT_00608a83 + unaff_EBX);
        local_ec.m[0][0] = *pfVar1;
        local_ec.m[0][1] = pfVar1[1];
        local_ec.m[0][2] = pfVar1[2];
        local_70 = local_ec.m[0][0];
        local_6c = local_ec.m[0][1];
        local_68 = local_ec.m[0][2];
        bVar5 = CPortalPlayerShared::InCond(&this_00->m_Shared,2);
        vVar3 = local_ec.m[0][0];
        if (bVar5) {
          local_f4 = *(float *)(unaff_EBX + 0x48de9f);
          local_f0 = *(float *)(unaff_EBX + 0x490ccf);
        }
        else {
          local_f4 = *(float *)(unaff_EBX + 0x492c4f);
          local_f0 = *(float *)(unaff_EBX + 0x48e023);
        }
        puVar2 = *(undefined4 **)(&DAT_006089f7 + unaff_EBX);
        lVar9 = (longdouble)(**(code **)(*(int *)*puVar2 + 0x68))((int *)*puVar2);
        vVar4 = local_ec.m[0][0];
        if ((float)lVar9 * mouse_y + vVar3 <= local_f0) {
          lVar9 = (longdouble)(**(code **)(*(int *)*puVar2 + 0x68))((int *)*puVar2);
          vVar3 = local_ec.m[0][0];
          if (local_f4 <= (float)lVar9 * mouse_y + vVar4) {
            lVar9 = (longdouble)(**(code **)(*(int *)*puVar2 + 0x68))((int *)*puVar2);
            local_f4 = (float)lVar9 * mouse_y + vVar3;
          }
        }
        else {
          local_f4 = local_f0;
        }
        local_ec.m[0][0] = local_f4;
        pfVar1 = *(float **)(&DAT_00608a83 + unaff_EBX);
        *pfVar1 = local_f4;
        pfVar1[1] = local_ec.m[0][1];
        pfVar1[2] = local_ec.m[0][2];
      }
    }
    else {
      cmd->forwardmove =
           cmd->forwardmove - mouse_y * *(float *)(*(int *)(unaff_EBX + 0x78f4bb) + 0x2c);
    }
    local_a0.x = 0.0;
    local_a0.y = 0.0;
    local_a0.z = 0.0;
    VectorAngles(local_28,local_110,&local_94);
    UTIL_NormalizedAngleDiff(viewangles,&local_94,&local_a0);
    viewangles->x = viewangles->x + local_a0.x;
    viewangles->y = viewangles->y + local_a0.y;
    viewangles->z = viewangles->z + local_a0.z;
    cmd->mousedx = (short)(int)mouse_x;
    cmd->mousedy = (short)(int)mouse_y;
  }
  return;
}


/* _GLOBAL__I_m_yaw at 00075de0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_m_yaw(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

