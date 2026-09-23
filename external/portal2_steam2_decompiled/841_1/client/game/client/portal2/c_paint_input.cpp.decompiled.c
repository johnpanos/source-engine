/* DWARF-guided pseudocode for game/client/portal2/c_paint_input.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 000759e0 */

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
  *(undefined1 *)((int)power2_n + unaff_EBX + 0x10f) = 0;
  *(undefined1 *)((int)power2_n + unaff_EBX + 0x110) = 0;
  *(undefined1 *)((int)power2_n + unaff_EBX + 0x111) = 0;
  *(undefined1 *)((int)power2_n + unaff_EBX + 0x112) = 0;
  *(undefined4 *)((int)power2_n + unaff_EBX + 0x113) = 0;
  *(undefined4 *)((int)power2_n + unaff_EBX + 0x117) = 0;
  *(undefined4 *)((int)power2_n + unaff_EBX + 0x11b) = 0x7f7fffff;
  *(undefined4 *)((int)power2_n + unaff_EBX + 0x11f) = 0x7f7fffff;
  *(undefined4 *)((int)power2_n + unaff_EBX + 0x123) = 0;
  *(undefined4 *)((int)power2_n + unaff_EBX + 0x127) = 0;
  *(undefined4 *)((int)power2_n + unaff_EBX + 299) = 0;
  *(undefined4 *)((int)power2_n + unaff_EBX + 0x12f) = 0;
  *(undefined4 *)((int)power2_n + unaff_EBX + 0x133) = 0x7f7fffff;
  *(undefined4 *)((int)power2_n + unaff_EBX + 0x137) = 0x7f7fffff;
  *(undefined4 *)((int)power2_n + unaff_EBX + 0x13b) = 0x7f7fffff;
  *(undefined4 *)((int)power2_n + unaff_EBX + 0x13f) = 0x7f7fffff;
  *(undefined **)((int)power2_n + unaff_EBX + 0x143) = &UNK_00aef497 + unaff_EBX;
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x70f),(char *)(unaff_EBX + 0x9448a3)
                 ,(char *)(unaff_EBX + 0x94489d),0x80,(char *)(unaff_EBX + 0x94488b),
                 in_stack_ffffffe8,in_stack_ffffffec,in_stack_fffffff0,flags_1,unaff_ESI);
  uVar1 = *(undefined4 *)(&DAT_00aee37b + unaff_EBX);
  ___cxa_atexit(unaff_EBX + 0x90630f,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x76f),(char *)(unaff_EBX + 0x93e271)
                 ,(char *)(unaff_EBX + 0x924ce7),0x80,(char *)(unaff_EBX + 0x9448a9),
                 in_stack_ffffffe8,in_stack_ffffffec,in_stack_fffffff0,flags_1,unaff_ESI);
  ___cxa_atexit(unaff_EBX + 0x9062ef,0,uVar1);
  this_1 = (ConVar *)(unaff_EBX + 0x9448bf);
  ConVar::ConVar((ConVar *)((int)power2_n + unaff_EBX + 0x14f),(char *)(unaff_EBX + 0x9448d7),
                 &UNK_0091613b + unaff_EBX,0x80,(char *)this_1,in_stack_ffffffe8,in_stack_ffffffec,
                 in_stack_fffffff0,flags_1,unaff_ESI);
  ___cxa_atexit(unaff_EBX + 0x9062cf,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 1999),(char *)(unaff_EBX + 0x9448e6),
                 (char *)(unaff_EBX + 0x9448e1),0,this_1,(char *)in_stack_ffffffe8,in_stack_ffffffec
                 ,(int)in_stack_fffffff0);
  ___cxa_atexit(unaff_EBX + 0x9062af,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x82f),(char *)(unaff_EBX + 0x9448f6)
                 ,(char *)(unaff_EBX + 0x9448f2),0,this_1,(char *)in_stack_ffffffe8,
                 in_stack_ffffffec,(int)in_stack_fffffff0);
  ___cxa_atexit(unaff_EBX + 0x90628f,0,uVar1);
  ConVar::ConVar((ConVar *)((int)s_pParticlePtrs + unaff_EBX + 0x88f),(char *)(unaff_EBX + 0x94490b)
                 ,(char *)(unaff_EBX + 0x91bbcd),2,this_1,(char *)in_stack_ffffffe8,
                 in_stack_ffffffec,(int)in_stack_fffffff0);
  ___cxa_atexit(unaff_EBX + 0x90626f,0,uVar1);
  return;
}


/* __tcf_0 at 0097bd00 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ecfb8),in_stack_00000008);
  return;
}


/* __tcf_1 at 0097bce0 */

void __tcf_1(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ed038),in_stack_00000008);
  return;
}


/* __tcf_2 at 0097bcc0 */

void __tcf_2(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x355338),in_stack_00000008);
  return;
}


/* __tcf_3 at 0097bca0 */

void __tcf_3(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ed0d8),in_stack_00000008);
  return;
}


/* __tcf_4 at 0097bc80 */

void __tcf_4(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(extraout_ECX + 0x3ed158),in_stack_00000008);
  return;
}


/* __tcf_5 at 0097bc60 */

void __tcf_5(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  ConVar::~ConVar((ConVar *)(&UNK_003ed1d8 + extraout_ECX),in_stack_00000008);
  return;
}


/* C_Paint_Input::ApplyMouse at 00523640 */

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
  Vector *local_10c;
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
      local_f4 = C_Portal_Player::GetReorientationProgress(this_00);
      fVar13 = *(float *)(*(int *)(unaff_EBX + 0x84580b) + 0x2c);
      if (fVar13 <= local_f4) {
        if (local_f4 < *(float *)(&DAT_004beb33 + unaff_EBX)) {
          local_f4 = (local_f4 - fVar13) / (*(float *)(&DAT_004beb33 + unaff_EBX) - fVar13);
        }
      }
      else {
        local_f4 = 0.0;
      }
      mouse_y = mouse_y * local_f4;
    }
    pSVar7 = kbutton_t::GetPerUser(*(kbutton_t **)(unaff_EBX + 0x640e83),nSlot);
    if (((pSVar7->state & 1) == 0) &&
       (*(int *)(*(int *)(*(int *)(unaff_EBX + 0x640e4f) + 0x1c) + 0x30) == 0)) {
      iVar8 = (*(this->super_CInput).super_IInput._vptr_IInput[0x1e])(this);
      if ((iVar8 == 0) || (bVar5 = C_Portal_Player::IsTaunting(this_00), !bVar5)) {
                    /* Unresolved local var: VMatrix rotMatrix@[???]
                       Unresolved local var: float angle@[???] */
        MatrixBuildRotationAboutAxis
                  (&local_ec,&local_4c,
                   (float)(*(uint *)(*(int *)(unaff_EBX + 0x84568b) + 0x2c) ^
                          *(uint *)(unaff_EBX + 0x4c3a3f)) * mouse_x);
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
        pfVar1 = *(float **)(unaff_EBX + 0x640f07);
        local_ec.m[0][0] = *pfVar1;
        local_54 = pfVar1[1];
        local_ec.m[0][2] = pfVar1[2];
        local_ec.m[0][1] = local_54 - mouse_x * *(float *)(*(int *)(unaff_EBX + 0x84568b) + 0x2c);
        *pfVar1 = local_ec.m[0][0];
        pfVar1[1] = local_ec.m[0][1];
        pfVar1[2] = local_ec.m[0][2];
        fVar13 = (float)((uint)(local_ec.m[0][1] - this_00->m_flTauntCamTargetYaw) &
                        *(uint *)(unaff_EBX + 0x4c3a2f));
        local_58 = local_ec.m[0][0];
        local_50 = local_ec.m[0][2];
        if (*(float *)(unaff_EBX + 0x4befa3) <= fVar13 && fVar13 != *(float *)(unaff_EBX + 0x4befa3)
           ) {
          this_00->m_bFaceTauntCameraEndAngles = true;
        }
      }
    }
    else {
      cmd->sidemove = mouse_x * *(float *)(*(int *)(unaff_EBX + 0x8456eb) + 0x2c) + cmd->sidemove;
    }
    local_10c = &local_4c;
    pSVar7 = kbutton_t::GetPerUser(*(kbutton_t **)(unaff_EBX + 0x640e83),nSlot);
    if ((pSVar7->state & 1) == 0) {
      iVar8 = (*(this->super_CInput).super_IInput._vptr_IInput[0x1e])(this);
      if ((iVar8 == 0) || (bVar5 = C_Portal_Player::IsTaunting(this_00), !bVar5)) {
                    /* Unresolved local var: float pitch@[???]
                       Unresolved local var: VMatrix rotMatrix@[???] */
        fVar13 = local_28[0].x * local_4c.x + local_28[0].y * local_4c.y +
                 local_28[0].z * local_4c.z;
        if (fVar13 < *(float *)(&DAT_004beb33 + unaff_EBX) ||
            fVar13 == *(float *)(&DAT_004beb33 + unaff_EBX)) {
          dVar12 = (double)fVar13;
          if ((fVar13 < *(float *)(&DAT_004bebfb + unaff_EBX)) &&
             (!NAN(fVar13) && !NAN(*(float *)(&DAT_004bebfb + unaff_EBX)))) {
            dVar12 = *(double *)(unaff_EBX + 0x4bec77);
          }
        }
        else {
          dVar12 = *(double *)(unaff_EBX + 0x4bec6f);
        }
        lVar9 = (longdouble)_acos(dVar12);
        fVar13 = *(float *)(unaff_EBX + 0x4bfbf3);
        fVar11 = *(float *)(unaff_EBX + 0x4beda3);
        lVar10 = (longdouble)
                 (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x640e7b) + 0x68))
                           ((int *)**(undefined4 **)(unaff_EBX + 0x640e7b));
        fVar14 = ((float)lVar9 * fVar13 + fVar11) - (float)lVar10 * mouse_y;
        fVar13 = *(float *)(*(int *)(unaff_EBX + 0x8457ab) + 0x2c);
        fVar11 = *(float *)(*(int *)(*(int *)(unaff_EBX + 0x640e7f) + 0x1c) + 0x2c) - fVar13;
        if ((fVar14 <= fVar11) &&
           (fVar11 = fVar13 - *(float *)(*(int *)(*(int *)(unaff_EBX + 0x640e5b) + 0x1c) + 0x2c),
           fVar11 <= fVar14)) {
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
        pfVar1 = *(float **)(unaff_EBX + 0x640f07);
        local_ec.m[0][0] = *pfVar1;
        local_ec.m[0][1] = pfVar1[1];
        local_ec.m[0][2] = pfVar1[2];
        local_70 = local_ec.m[0][0];
        local_6c = local_ec.m[0][1];
        local_68 = local_ec.m[0][2];
        bVar5 = CPortalPlayerShared::InCond(&this_00->m_Shared,2);
        vVar3 = local_ec.m[0][0];
        if (bVar5) {
          local_f0 = *(float *)(unaff_EBX + 0x4bec1b);
        }
        else {
          local_f0 = *(float *)(unaff_EBX + 0x4bef83);
        }
        puVar2 = *(undefined4 **)(unaff_EBX + 0x640e7b);
        lVar9 = (longdouble)(**(code **)(*(int *)*puVar2 + 0x68))((int *)*puVar2);
        vVar4 = local_ec.m[0][0];
        fVar13 = *(float *)(unaff_EBX + 0x4c1a4f);
        if (((float)lVar9 * mouse_y + vVar3 <= *(float *)(unaff_EBX + 0x4c1a4f)) &&
           (lVar9 = (longdouble)(**(code **)(*(int *)*puVar2 + 0x68))((int *)*puVar2),
           vVar3 = local_ec.m[0][0], fVar13 = local_f0, local_f0 <= (float)lVar9 * mouse_y + vVar4))
        {
          lVar9 = (longdouble)(**(code **)(*(int *)*puVar2 + 0x68))((int *)*puVar2);
          fVar13 = (float)lVar9 * mouse_y + vVar3;
        }
        local_f0 = fVar13;
        local_ec.m[0][0] = local_f0;
        pfVar1 = *(float **)(unaff_EBX + 0x640f07);
        *pfVar1 = local_f0;
        pfVar1[1] = local_ec.m[0][1];
        pfVar1[2] = local_ec.m[0][2];
      }
    }
    else {
      cmd->forwardmove =
           cmd->forwardmove - mouse_y * *(float *)(*(int *)(unaff_EBX + 0x7ad9cb) + 0x2c);
    }
    local_a0.x = 0.0;
    local_a0.y = 0.0;
    local_a0.z = 0.0;
    VectorAngles(local_28,local_10c,&local_94);
    UTIL_NormalizedAngleDiff(viewangles,&local_94,&local_a0);
    viewangles->x = viewangles->x + local_a0.x;
    viewangles->y = viewangles->y + local_a0.y;
    viewangles->z = viewangles->z + local_a0.z;
    cmd->mousedx = (short)(int)mouse_x;
    cmd->mousedy = (short)(int)mouse_y;
  }
  return;
}


/* C_Paint_Input::JoyStickTurn at 00523450 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "soundlevel_t": Some values do not have unique names */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void JoyStickTurn(C_Paint_Input * this, CUserCmd * cmd, float * yaw,
   float * pitch, float frametime, bool bAbsoluteYaw, bool bAbsolutePitch) */

void __thiscall
C_Paint_Input::JoyStickTurn
          (C_Paint_Input *this,CUserCmd *cmd,float *yaw,float *pitch,float frametime,
          bool bAbsoluteYaw,bool bAbsolutePitch)

{
  float *pfVar1;
  float fVar2;
  float fVar3;
  undefined4 *puVar4;
  bool bVar5;
  C_Portal_Player *this_00;
  int iVar6;
  int unaff_EBX;
  uint uVar7;
  float fVar8;
  float fVar9;
  
                    /* Unresolved local var: C_Portal_Player * pPlayer@[???] */
  ___i686_get_pc_thunk_bx();
  CInput::JoyStickTurn(&this->super_CInput,cmd,yaw,pitch,frametime,bAbsoluteYaw,bAbsolutePitch);
  this_00 = (C_Portal_Player *)C_BasePlayer::GetLocalPlayer(-1);
  if (((this_00 != (C_Portal_Player *)0x0) &&
      (iVar6 = (*(this->super_CInput).super_IInput._vptr_IInput[0x1e])(this,0xffffffff), iVar6 != 0)
      ) && (bVar5 = C_Portal_Player::IsTaunting(this_00), bVar5)) {
    if ((*yaw != 0.0) && (this_00->m_bTauntInterpolatingAngles == false)) {
                    /* Unresolved local var: Vector vAngle@[???] */
      puVar4 = *(undefined4 **)(unaff_EBX + 0x6410f4);
      fVar2 = (float)puVar4[1];
      fVar8 = *yaw;
      *puVar4 = *puVar4;
      puVar4[1] = fVar2 - fVar8;
      puVar4[2] = puVar4[2];
      fVar8 = (float)((uint)((fVar2 - fVar8) - this_00->m_flTauntCamTargetYaw) &
                     *(uint *)(unaff_EBX + 0x4c3c1c));
      if (*(float *)(unaff_EBX + 0x4bf190) <= fVar8 && fVar8 != *(float *)(unaff_EBX + 0x4bf190)) {
        this_00->m_bFaceTauntCameraEndAngles = true;
      }
    }
    if ((*pitch != 0.0) && (this_00->m_bTauntInterpolatingAngles == false)) {
      pfVar1 = *(float **)(unaff_EBX + 0x6410f4);
      fVar8 = *pfVar1;
      fVar2 = pfVar1[1];
      fVar3 = pfVar1[2];
      bVar5 = CPortalPlayerShared::InCond(&this_00->m_Shared,2);
      if (bVar5) {
        fVar9 = *(float *)(unaff_EBX + 0x4bee08);
      }
      else {
        fVar9 = *(float *)(unaff_EBX + 0x4bf170);
      }
      fVar8 = fVar8 + *pitch;
      if (fVar9 <= fVar8) {
        fVar9 = fVar8;
      }
      uVar7 = -(uint)(fVar8 <= *(float *)(unaff_EBX + 0x4c1c3c));
      *pfVar1 = (float)(~uVar7 & (uint)*(float *)(unaff_EBX + 0x4c1c3c) | (uint)fVar9 & uVar7);
      pfVar1[1] = fVar2;
      pfVar1[2] = fVar3;
    }
  }
  return;
}


/* _GLOBAL__I_m_yaw at 00075c40 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_m_yaw(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

