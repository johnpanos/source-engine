/* DWARF-guided pseudocode for game/client/portal2/c_paint_messages.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __MsgFunc_PaintWorld at 00529800 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

void __MsgFunc_PaintWorld(bf_read *msg)

{
  uint32 **ppuVar1;
  uint32 *puVar2;
  PaintPowerType PVar3;
  uint32 *puVar4;
  uint uVar5;
  uint uVar6;
  undefined4 uVar7;
  uint32 uVar8;
  uint uVar9;
  ushort uVar10;
  char cVar11;
  C_BaseEntity *pBrushEntity;
  byte bVar12;
  int iVar13;
  int unaff_EBX;
  int iVar14;
  byte local_a0;
  uint local_7c;
  float local_68;
  float local_5c;
  float local_50;
  PaintPowerType local_38;
  uint local_34;
  uint local_30;
  Vector local_2c;
  float local_20;
  
  ___i686_get_pc_thunk_bx();
  cVar11 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x607547) + 0x2f0))
                     ((int *)**(undefined4 **)(unaff_EBX + 0x607547));
  if (cVar11 != '\0') {
    return;
  }
  iVar14 = (msg->super_CBitRead).m_nBitsAvail;
  if (iVar14 < 8) {
    PVar3 = (msg->super_CBitRead).m_nInBufWord;
    puVar4 = (msg->super_CBitRead).m_pDataIn;
    puVar2 = (msg->super_CBitRead).m_pBufferEnd;
    if (puVar4 == puVar2) {
      iVar14 = 1;
      (msg->super_CBitRead).m_nBitsAvail = 1;
      (msg->super_CBitRead).m_nInBufWord = 0;
      ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
      *ppuVar1 = *ppuVar1 + 1;
      (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
      local_38 = BOUNCE_POWER;
      goto LAB_0052995a;
    }
    if (puVar2 < puVar4) {
      (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
      (msg->super_CBitRead).m_nInBufWord = 0;
    }
    else {
      (msg->super_CBitRead).m_nInBufWord = *puVar4;
      (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
    }
    local_38 = BOUNCE_POWER;
    if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow != false) goto LAB_00529951;
    iVar13 = 8 - iVar14;
    uVar5 = (msg->super_CBitRead).m_nInBufWord;
    uVar6 = *(uint *)(*(int *)(unaff_EBX + 0x60754b) + iVar13 * 4);
    bVar12 = (byte)iVar14;
    iVar14 = 0x20 - iVar13;
    (msg->super_CBitRead).m_nBitsAvail = iVar14;
    local_a0 = (byte)iVar13;
    puVar2 = &(msg->super_CBitRead).m_nInBufWord;
    *puVar2 = *puVar2 >> (local_a0 & 0x1f);
    local_38 = (uVar5 & uVar6) << (bVar12 & 0x1f) | PVar3;
    if (iVar14 < 0x20) goto LAB_0052995a;
LAB_005298c6:
    local_34 = (msg->super_CBitRead).m_nInBufWord & *(uint *)(*(int *)(unaff_EBX + 0x60754b) + 0x80)
    ;
    (msg->super_CBitRead).m_nBitsAvail = iVar14 + -0x20;
    if (iVar14 + -0x20 == 0) {
      (msg->super_CBitRead).m_nBitsAvail = 0x20;
      puVar4 = (msg->super_CBitRead).m_pDataIn;
      puVar2 = (msg->super_CBitRead).m_pBufferEnd;
      if (puVar4 == puVar2) {
        (msg->super_CBitRead).m_nBitsAvail = 1;
        (msg->super_CBitRead).m_nInBufWord = 0;
        ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
        *ppuVar1 = *ppuVar1 + 1;
      }
      else if (puVar2 < puVar4) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar4;
        (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
      }
    }
    else {
      puVar2 = &(msg->super_CBitRead).m_nInBufWord;
      *puVar2 = *puVar2;
    }
  }
  else {
    local_38 = (msg->super_CBitRead).m_nInBufWord &
               *(PaintPowerType *)(*(int *)(unaff_EBX + 0x60754b) + 0x20);
    (msg->super_CBitRead).m_nBitsAvail = iVar14 + -8;
    if (iVar14 + -8 == 0) {
      (msg->super_CBitRead).m_nBitsAvail = 0x20;
      puVar4 = (msg->super_CBitRead).m_pDataIn;
      puVar2 = (msg->super_CBitRead).m_pBufferEnd;
      if (puVar4 == puVar2) {
        (msg->super_CBitRead).m_nBitsAvail = 1;
        (msg->super_CBitRead).m_nInBufWord = 0;
        ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
        *ppuVar1 = *ppuVar1 + 1;
      }
      else if (puVar2 < puVar4) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar4;
        (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
      }
    }
    else {
      puVar2 = &(msg->super_CBitRead).m_nInBufWord;
      *puVar2 = *puVar2 >> 8;
    }
    iVar14 = (msg->super_CBitRead).m_nBitsAvail;
LAB_00529951:
    if (0x1f < iVar14) goto LAB_005298c6;
LAB_0052995a:
    uVar5 = (msg->super_CBitRead).m_nInBufWord;
    puVar4 = (msg->super_CBitRead).m_pDataIn;
    puVar2 = (msg->super_CBitRead).m_pBufferEnd;
    if (puVar4 == puVar2) {
      (msg->super_CBitRead).m_nBitsAvail = 1;
      (msg->super_CBitRead).m_nInBufWord = 0;
      ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
      *ppuVar1 = *ppuVar1 + 1;
      (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
      local_34 = 0;
    }
    else {
      if (puVar2 < puVar4) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar4;
        (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
      }
      local_34 = 0;
      if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
        iVar13 = 0x20 - iVar14;
        uVar6 = (msg->super_CBitRead).m_nInBufWord;
        uVar9 = *(uint *)(*(int *)(unaff_EBX + 0x60754b) + iVar13 * 4);
        (msg->super_CBitRead).m_nBitsAvail = 0x20 - iVar13;
        local_a0 = (byte)iVar13;
        puVar2 = &(msg->super_CBitRead).m_nInBufWord;
        *puVar2 = *puVar2 >> (local_a0 & 0x1f);
        local_34 = (uVar6 & uVar9) << ((byte)iVar14 & 0x1f) | uVar5;
      }
    }
  }
  pBrushEntity = UTIL_EntityFromUserMessageEHandle(local_34);
  iVar14 = (msg->super_CBitRead).m_nBitsAvail;
  if (iVar14 < 8) {
    uVar5 = (msg->super_CBitRead).m_nInBufWord;
    puVar4 = (msg->super_CBitRead).m_pDataIn;
    puVar2 = (msg->super_CBitRead).m_pBufferEnd;
    if (puVar4 != puVar2) {
      if (puVar2 < puVar4) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar4;
        (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
      }
      local_30 = 0;
      if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
        iVar13 = 8 - iVar14;
        uVar6 = (msg->super_CBitRead).m_nInBufWord;
        uVar9 = *(uint *)(*(int *)(unaff_EBX + 0x60754b) + iVar13 * 4);
        bVar12 = (byte)iVar14;
        iVar14 = 0x20 - iVar13;
        (msg->super_CBitRead).m_nBitsAvail = iVar14;
        local_a0 = (byte)iVar13;
        puVar2 = &(msg->super_CBitRead).m_nInBufWord;
        *puVar2 = *puVar2 >> (local_a0 & 0x1f);
        local_30 = (uVar6 & uVar9) << (bVar12 & 0x1f) | uVar5;
      }
      goto LAB_00529a10;
    }
    iVar14 = 1;
    (msg->super_CBitRead).m_nBitsAvail = 1;
    (msg->super_CBitRead).m_nInBufWord = 0;
    ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
    *ppuVar1 = *ppuVar1 + 1;
    (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
    local_30 = 0;
  }
  else {
    local_30 = (msg->super_CBitRead).m_nInBufWord & *(uint *)(*(int *)(unaff_EBX + 0x60754b) + 0x20)
    ;
    (msg->super_CBitRead).m_nBitsAvail = iVar14 + -8;
    if (iVar14 + -8 == 0) {
      (msg->super_CBitRead).m_nBitsAvail = 0x20;
      puVar4 = (msg->super_CBitRead).m_pDataIn;
      puVar2 = (msg->super_CBitRead).m_pBufferEnd;
      if (puVar4 == puVar2) {
        (msg->super_CBitRead).m_nBitsAvail = 1;
        (msg->super_CBitRead).m_nInBufWord = 0;
        ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
        *ppuVar1 = *ppuVar1 + 1;
      }
      else if (puVar2 < puVar4) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar4;
        (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
      }
    }
    else {
      puVar2 = &(msg->super_CBitRead).m_nInBufWord;
      *puVar2 = *puVar2 >> 8;
    }
    iVar14 = (msg->super_CBitRead).m_nBitsAvail;
LAB_00529a10:
    if (0x1f < iVar14) {
      uVar5 = (msg->super_CBitRead).m_nInBufWord;
      uVar6 = *(uint *)(*(int *)(unaff_EBX + 0x60754b) + 0x80);
      (msg->super_CBitRead).m_nBitsAvail = iVar14 + -0x20;
      if (iVar14 + -0x20 == 0) {
        (msg->super_CBitRead).m_nBitsAvail = 0x20;
        puVar4 = (msg->super_CBitRead).m_pDataIn;
        puVar2 = (msg->super_CBitRead).m_pBufferEnd;
        if (puVar4 == puVar2) {
          (msg->super_CBitRead).m_nBitsAvail = 1;
          (msg->super_CBitRead).m_nInBufWord = 0;
          ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
          *ppuVar1 = *ppuVar1 + 1;
        }
        else if (puVar2 < puVar4) {
          (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
          (msg->super_CBitRead).m_nInBufWord = 0;
        }
        else {
          (msg->super_CBitRead).m_nInBufWord = *puVar4;
          (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
        }
      }
      else {
        puVar2 = &(msg->super_CBitRead).m_nInBufWord;
        *puVar2 = *puVar2;
      }
      local_68 = (float)(uVar6 & uVar5);
      iVar14 = (msg->super_CBitRead).m_nBitsAvail;
      goto LAB_00529a55;
    }
  }
  uVar5 = (msg->super_CBitRead).m_nInBufWord;
  puVar4 = (msg->super_CBitRead).m_pDataIn;
  puVar2 = (msg->super_CBitRead).m_pBufferEnd;
  if (puVar4 == puVar2) {
    iVar14 = 1;
    (msg->super_CBitRead).m_nBitsAvail = 1;
    (msg->super_CBitRead).m_nInBufWord = 0;
    ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
    *ppuVar1 = *ppuVar1 + 1;
    (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
    local_68 = 0.0;
  }
  else {
    if (puVar2 < puVar4) {
      (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
      (msg->super_CBitRead).m_nInBufWord = 0;
    }
    else {
      (msg->super_CBitRead).m_nInBufWord = *puVar4;
      (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
    }
    local_68 = 0.0;
    if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
      iVar13 = 0x20 - iVar14;
      local_68 = (float)(((msg->super_CBitRead).m_nInBufWord &
                         *(uint *)(*(int *)(unaff_EBX + 0x60754b) + iVar13 * 4)) <<
                         ((byte)iVar14 & 0x1f) | uVar5);
      iVar14 = 0x20 - iVar13;
      (msg->super_CBitRead).m_nBitsAvail = iVar14;
      local_a0 = (byte)iVar13;
      puVar2 = &(msg->super_CBitRead).m_nInBufWord;
      *puVar2 = *puVar2 >> (local_a0 & 0x1f);
    }
  }
LAB_00529a55:
  if (iVar14 < 0x20) {
    uVar5 = (msg->super_CBitRead).m_nInBufWord;
    puVar4 = (msg->super_CBitRead).m_pDataIn;
    puVar2 = (msg->super_CBitRead).m_pBufferEnd;
    if (puVar4 == puVar2) {
      iVar14 = 1;
      (msg->super_CBitRead).m_nBitsAvail = 1;
      (msg->super_CBitRead).m_nInBufWord = 0;
      ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
      *ppuVar1 = *ppuVar1 + 1;
      (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
      local_5c = 0.0;
    }
    else {
      if (puVar2 < puVar4) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar4;
        (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
      }
      local_5c = 0.0;
      if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
        iVar13 = 0x20 - iVar14;
        local_5c = (float)(((msg->super_CBitRead).m_nInBufWord &
                           *(uint *)(*(int *)(unaff_EBX + 0x60754b) + iVar13 * 4)) <<
                           ((byte)iVar14 & 0x1f) | uVar5);
        iVar14 = 0x20 - iVar13;
        (msg->super_CBitRead).m_nBitsAvail = iVar14;
        local_a0 = (byte)iVar13;
        puVar2 = &(msg->super_CBitRead).m_nInBufWord;
        *puVar2 = *puVar2 >> (local_a0 & 0x1f);
      }
    }
  }
  else {
    uVar5 = (msg->super_CBitRead).m_nInBufWord;
    uVar6 = *(uint *)(*(int *)(unaff_EBX + 0x60754b) + 0x80);
    (msg->super_CBitRead).m_nBitsAvail = iVar14 + -0x20;
    if (iVar14 + -0x20 == 0) {
      (msg->super_CBitRead).m_nBitsAvail = 0x20;
      puVar4 = (msg->super_CBitRead).m_pDataIn;
      puVar2 = (msg->super_CBitRead).m_pBufferEnd;
      if (puVar4 == puVar2) {
        (msg->super_CBitRead).m_nBitsAvail = 1;
        (msg->super_CBitRead).m_nInBufWord = 0;
        ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
        *ppuVar1 = *ppuVar1 + 1;
      }
      else if (puVar2 < puVar4) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar4;
        (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
      }
    }
    else {
      puVar2 = &(msg->super_CBitRead).m_nInBufWord;
      *puVar2 = *puVar2;
    }
    local_5c = (float)(uVar6 & uVar5);
    iVar14 = (msg->super_CBitRead).m_nBitsAvail;
  }
  if (iVar14 < 0x20) {
    uVar5 = (msg->super_CBitRead).m_nInBufWord;
    puVar4 = (msg->super_CBitRead).m_pDataIn;
    puVar2 = (msg->super_CBitRead).m_pBufferEnd;
    if (puVar4 == puVar2) {
      (msg->super_CBitRead).m_nBitsAvail = 1;
      (msg->super_CBitRead).m_nInBufWord = 0;
      ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
      *ppuVar1 = *ppuVar1 + 1;
      (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
      local_50 = 0.0;
    }
    else {
      if (puVar2 < puVar4) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar4;
        (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
      }
      local_50 = 0.0;
      if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
        iVar13 = 0x20 - iVar14;
        local_50 = (float)(((msg->super_CBitRead).m_nInBufWord &
                           *(uint *)(*(int *)(unaff_EBX + 0x60754b) + iVar13 * 4)) <<
                           ((byte)iVar14 & 0x1f) | uVar5);
        (msg->super_CBitRead).m_nBitsAvail = 0x20 - iVar13;
        local_a0 = (byte)iVar13;
        puVar2 = &(msg->super_CBitRead).m_nInBufWord;
        *puVar2 = *puVar2 >> (local_a0 & 0x1f);
      }
    }
  }
  else {
    uVar5 = (msg->super_CBitRead).m_nInBufWord;
    uVar6 = *(uint *)(*(int *)(unaff_EBX + 0x60754b) + 0x80);
    (msg->super_CBitRead).m_nBitsAvail = iVar14 + -0x20;
    if (iVar14 + -0x20 == 0) {
      (msg->super_CBitRead).m_nBitsAvail = 0x20;
      puVar4 = (msg->super_CBitRead).m_pDataIn;
      puVar2 = (msg->super_CBitRead).m_pBufferEnd;
      if (puVar4 == puVar2) {
        (msg->super_CBitRead).m_nBitsAvail = 1;
        (msg->super_CBitRead).m_nInBufWord = 0;
        ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
        *ppuVar1 = *ppuVar1 + 1;
      }
      else if (puVar2 < puVar4) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar4;
        (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
      }
    }
    else {
      puVar2 = &(msg->super_CBitRead).m_nInBufWord;
      *puVar2 = *puVar2;
    }
    local_50 = (float)(uVar6 & uVar5);
  }
  local_20 = local_50;
  if (0 < (int)local_30) {
    uVar7 = *(undefined4 *)(*(int *)(unaff_EBX + 0x60754b) + 0x40);
    local_7c = 0;
    do {
      iVar14 = (msg->super_CBitRead).m_nBitsAvail;
      uVar10 = (ushort)uVar7;
      if (iVar14 < 0x10) {
        uVar8 = (msg->super_CBitRead).m_nInBufWord;
        puVar4 = (msg->super_CBitRead).m_pDataIn;
        puVar2 = (msg->super_CBitRead).m_pBufferEnd;
        if (puVar4 == puVar2) {
          iVar14 = 1;
          (msg->super_CBitRead).m_nBitsAvail = 1;
          (msg->super_CBitRead).m_nInBufWord = 0;
          ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
          *ppuVar1 = *ppuVar1 + 1;
          (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
          local_2c.x = 0.0;
        }
        else {
          if (puVar2 < puVar4) {
            (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
            (msg->super_CBitRead).m_nInBufWord = 0;
          }
          else {
            (msg->super_CBitRead).m_nInBufWord = *puVar4;
            (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
          }
          local_2c.x = 0.0;
          if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
            iVar13 = 0x10 - iVar14;
            uVar5 = (msg->super_CBitRead).m_nInBufWord;
            uVar6 = *(uint *)(*(int *)(unaff_EBX + 0x60754b) + iVar13 * 4);
            bVar12 = (byte)iVar14;
            iVar14 = 0x20 - iVar13;
            (msg->super_CBitRead).m_nBitsAvail = iVar14;
            puVar2 = &(msg->super_CBitRead).m_nInBufWord;
            *puVar2 = *puVar2 >> ((byte)iVar13 & 0x1f);
            local_2c.x = (vec_t)(int)(short)((ushort)((uVar5 & uVar6) << (bVar12 & 0x1f)) |
                                            (ushort)uVar8);
          }
        }
      }
      else {
        uVar8 = (msg->super_CBitRead).m_nInBufWord;
        (msg->super_CBitRead).m_nBitsAvail = iVar14 + -0x10;
        if (iVar14 + -0x10 == 0) {
          (msg->super_CBitRead).m_nBitsAvail = 0x20;
          puVar4 = (msg->super_CBitRead).m_pDataIn;
          puVar2 = (msg->super_CBitRead).m_pBufferEnd;
          if (puVar4 == puVar2) {
            (msg->super_CBitRead).m_nBitsAvail = 1;
            (msg->super_CBitRead).m_nInBufWord = 0;
            ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
            *ppuVar1 = *ppuVar1 + 1;
          }
          else if (puVar2 < puVar4) {
            (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
            (msg->super_CBitRead).m_nInBufWord = 0;
          }
          else {
            (msg->super_CBitRead).m_nInBufWord = *puVar4;
            (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
          }
        }
        else {
          puVar2 = &(msg->super_CBitRead).m_nInBufWord;
          *puVar2 = *puVar2 >> 0x10;
        }
        local_2c.x = (vec_t)(int)(short)(uVar10 & (ushort)uVar8);
        iVar14 = (msg->super_CBitRead).m_nBitsAvail;
      }
      local_2c.x = local_2c.x + local_68;
      if (iVar14 < 0x10) {
        uVar8 = (msg->super_CBitRead).m_nInBufWord;
        puVar4 = (msg->super_CBitRead).m_pDataIn;
        puVar2 = (msg->super_CBitRead).m_pBufferEnd;
        if (puVar4 == puVar2) {
          iVar14 = 1;
          (msg->super_CBitRead).m_nBitsAvail = 1;
          (msg->super_CBitRead).m_nInBufWord = 0;
          ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
          *ppuVar1 = *ppuVar1 + 1;
          (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
          local_2c.y = 0.0;
        }
        else {
          if (puVar2 < puVar4) {
            (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
            (msg->super_CBitRead).m_nInBufWord = 0;
          }
          else {
            (msg->super_CBitRead).m_nInBufWord = *puVar4;
            (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
          }
          local_2c.y = 0.0;
          if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
            iVar13 = 0x10 - iVar14;
            uVar5 = (msg->super_CBitRead).m_nInBufWord;
            uVar6 = *(uint *)(*(int *)(unaff_EBX + 0x60754b) + iVar13 * 4);
            bVar12 = (byte)iVar14;
            iVar14 = 0x20 - iVar13;
            (msg->super_CBitRead).m_nBitsAvail = iVar14;
            puVar2 = &(msg->super_CBitRead).m_nInBufWord;
            *puVar2 = *puVar2 >> ((byte)iVar13 & 0x1f);
            local_2c.y = (vec_t)(int)(short)((ushort)((uVar5 & uVar6) << (bVar12 & 0x1f)) |
                                            (ushort)uVar8);
          }
        }
      }
      else {
        uVar8 = (msg->super_CBitRead).m_nInBufWord;
        (msg->super_CBitRead).m_nBitsAvail = iVar14 + -0x10;
        if (iVar14 + -0x10 == 0) {
          (msg->super_CBitRead).m_nBitsAvail = 0x20;
          puVar4 = (msg->super_CBitRead).m_pDataIn;
          puVar2 = (msg->super_CBitRead).m_pBufferEnd;
          if (puVar4 == puVar2) {
            (msg->super_CBitRead).m_nBitsAvail = 1;
            (msg->super_CBitRead).m_nInBufWord = 0;
            ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
            *ppuVar1 = *ppuVar1 + 1;
          }
          else if (puVar2 < puVar4) {
            (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
            (msg->super_CBitRead).m_nInBufWord = 0;
          }
          else {
            (msg->super_CBitRead).m_nInBufWord = *puVar4;
            (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
          }
        }
        else {
          puVar2 = &(msg->super_CBitRead).m_nInBufWord;
          *puVar2 = *puVar2 >> 0x10;
        }
        local_2c.y = (vec_t)(int)(short)(uVar10 & (ushort)uVar8);
        iVar14 = (msg->super_CBitRead).m_nBitsAvail;
      }
      local_2c.y = local_2c.y + local_5c;
      if (iVar14 < 0x10) {
        uVar8 = (msg->super_CBitRead).m_nInBufWord;
        puVar4 = (msg->super_CBitRead).m_pDataIn;
        puVar2 = (msg->super_CBitRead).m_pBufferEnd;
        if (puVar4 == puVar2) {
          (msg->super_CBitRead).m_nBitsAvail = 1;
          (msg->super_CBitRead).m_nInBufWord = 0;
          ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
          *ppuVar1 = *ppuVar1 + 1;
          (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
          local_2c.z = 0.0;
        }
        else {
          if (puVar2 < puVar4) {
            (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
            (msg->super_CBitRead).m_nInBufWord = 0;
          }
          else {
            (msg->super_CBitRead).m_nInBufWord = *puVar4;
            (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
          }
          local_2c.z = 0.0;
          if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
            iVar13 = 0x10 - iVar14;
            uVar5 = (msg->super_CBitRead).m_nInBufWord;
            uVar6 = *(uint *)(*(int *)(unaff_EBX + 0x60754b) + iVar13 * 4);
            (msg->super_CBitRead).m_nBitsAvail = 0x20 - iVar13;
            local_a0 = (byte)iVar13;
            puVar2 = &(msg->super_CBitRead).m_nInBufWord;
            *puVar2 = *puVar2 >> (local_a0 & 0x1f);
            local_2c.z = (vec_t)(int)(short)((ushort)((uVar5 & uVar6) << ((byte)iVar14 & 0x1f)) |
                                            (ushort)uVar8);
          }
        }
      }
      else {
        uVar8 = (msg->super_CBitRead).m_nInBufWord;
        (msg->super_CBitRead).m_nBitsAvail = iVar14 + -0x10;
        if (iVar14 + -0x10 == 0) {
          (msg->super_CBitRead).m_nBitsAvail = 0x20;
          puVar4 = (msg->super_CBitRead).m_pDataIn;
          puVar2 = (msg->super_CBitRead).m_pBufferEnd;
          if (puVar4 == puVar2) {
            (msg->super_CBitRead).m_nBitsAvail = 1;
            (msg->super_CBitRead).m_nInBufWord = 0;
            ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
            *ppuVar1 = *ppuVar1 + 1;
          }
          else if (puVar2 < puVar4) {
            (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
            (msg->super_CBitRead).m_nInBufWord = 0;
          }
          else {
            (msg->super_CBitRead).m_nInBufWord = *puVar4;
            (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
          }
        }
        else {
          puVar2 = &(msg->super_CBitRead).m_nInBufWord;
          *puVar2 = *puVar2 >> 0x10;
        }
        local_2c.z = (vec_t)(int)(short)(uVar10 & (ushort)uVar8);
      }
      local_2c.z = local_2c.z + local_50;
      UTIL_PaintBrushEntity(pBrushEntity,&local_2c,local_38);
      local_7c = local_7c + 1;
    } while (local_7c != local_30);
  }
  return;
}


/* __MsgFunc_PaintEntity at 0052a440 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

void __MsgFunc_PaintEntity(bf_read *msg)

{
  uint32 **ppuVar1;
  uint32 *puVar2;
  uint32 *puVar3;
  uint uVar4;
  uint uVar5;
  C_BaseEntity *pCVar6;
  byte bVar7;
  int iVar8;
  int unaff_EBX;
  uint uVar9;
  int iVar10;
  byte local_50;
  int *local_44;
  uint local_30;
  uint local_2c;
  uint local_28;
  uint local_24;
  uint local_20;
  
                    /* Unresolved local var: IPaintableEntity * pPaintableEnt@[???]
                       Unresolved local var: PaintPowerType power@[???]
                       Unresolved local var: Vector pos@[???] */
  ___i686_get_pc_thunk_bx();
  iVar10 = (msg->super_CBitRead).m_nBitsAvail;
  if (iVar10 < 0x20) {
    uVar4 = (msg->super_CBitRead).m_nInBufWord;
    puVar3 = (msg->super_CBitRead).m_pDataIn;
    puVar2 = (msg->super_CBitRead).m_pBufferEnd;
    if (puVar3 == puVar2) {
      (msg->super_CBitRead).m_nBitsAvail = 1;
      (msg->super_CBitRead).m_nInBufWord = 0;
      ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
      *ppuVar1 = *ppuVar1 + 1;
      (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
      uVar9 = 0;
    }
    else {
      if (puVar2 < puVar3) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar3;
        (msg->super_CBitRead).m_pDataIn = puVar3 + 1;
      }
      uVar9 = 0;
      if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
        iVar8 = 0x20 - iVar10;
        uVar9 = (msg->super_CBitRead).m_nInBufWord;
        uVar5 = *(uint *)(*(int *)(unaff_EBX + 0x60690e) + iVar8 * 4);
        (msg->super_CBitRead).m_nBitsAvail = 0x20 - iVar8;
        local_50 = (byte)iVar8;
        puVar2 = &(msg->super_CBitRead).m_nInBufWord;
        *puVar2 = *puVar2 >> (local_50 & 0x1f);
        uVar9 = (uVar9 & uVar5) << ((byte)iVar10 & 0x1f) | uVar4;
      }
    }
  }
  else {
    uVar9 = (msg->super_CBitRead).m_nInBufWord & *(uint *)(*(int *)(unaff_EBX + 0x60690e) + 0x80);
    (msg->super_CBitRead).m_nBitsAvail = iVar10 + -0x20;
    if (iVar10 + -0x20 == 0) {
      (msg->super_CBitRead).m_nBitsAvail = 0x20;
      puVar3 = (msg->super_CBitRead).m_pDataIn;
      puVar2 = (msg->super_CBitRead).m_pBufferEnd;
      if (puVar3 == puVar2) {
        (msg->super_CBitRead).m_nBitsAvail = 1;
        (msg->super_CBitRead).m_nInBufWord = 0;
        ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
        *ppuVar1 = *ppuVar1 + 1;
      }
      else if (puVar2 < puVar3) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar3;
        (msg->super_CBitRead).m_pDataIn = puVar3 + 1;
      }
    }
    else {
      puVar2 = &(msg->super_CBitRead).m_nInBufWord;
      *puVar2 = *puVar2;
    }
  }
  pCVar6 = UTIL_EntityFromUserMessageEHandle(uVar9);
  local_44 = (int *)0x0;
  if (pCVar6 != (C_BaseEntity *)0x0) {
    local_44 = (int *)___dynamic_cast(pCVar6,*(undefined4 *)(unaff_EBX + 0x6068de),
                                      unaff_EBX + 0x66e5da,0xfffffffe);
  }
  iVar10 = (msg->super_CBitRead).m_nBitsAvail;
  if (iVar10 < 8) {
    uVar4 = (msg->super_CBitRead).m_nInBufWord;
    puVar3 = (msg->super_CBitRead).m_pDataIn;
    puVar2 = (msg->super_CBitRead).m_pBufferEnd;
    if (puVar3 != puVar2) {
      if (puVar2 < puVar3) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar3;
        (msg->super_CBitRead).m_pDataIn = puVar3 + 1;
      }
      local_30 = 0;
      if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
        iVar8 = 8 - iVar10;
        uVar9 = (msg->super_CBitRead).m_nInBufWord;
        uVar5 = *(uint *)(*(int *)(unaff_EBX + 0x60690e) + iVar8 * 4);
        bVar7 = (byte)iVar10;
        iVar10 = 0x20 - iVar8;
        (msg->super_CBitRead).m_nBitsAvail = iVar10;
        local_50 = (byte)iVar8;
        puVar2 = &(msg->super_CBitRead).m_nInBufWord;
        *puVar2 = *puVar2 >> (local_50 & 0x1f);
        local_30 = (uVar9 & uVar5) << (bVar7 & 0x1f) | uVar4;
      }
      goto LAB_0052a595;
    }
    iVar10 = 1;
    (msg->super_CBitRead).m_nBitsAvail = 1;
    (msg->super_CBitRead).m_nInBufWord = 0;
    ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
    *ppuVar1 = *ppuVar1 + 1;
    (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
    local_30 = 0;
  }
  else {
    local_30 = (msg->super_CBitRead).m_nInBufWord & *(uint *)(*(int *)(unaff_EBX + 0x60690e) + 0x20)
    ;
    (msg->super_CBitRead).m_nBitsAvail = iVar10 + -8;
    if (iVar10 + -8 == 0) {
      (msg->super_CBitRead).m_nBitsAvail = 0x20;
      puVar3 = (msg->super_CBitRead).m_pDataIn;
      puVar2 = (msg->super_CBitRead).m_pBufferEnd;
      if (puVar3 == puVar2) {
        (msg->super_CBitRead).m_nBitsAvail = 1;
        (msg->super_CBitRead).m_nInBufWord = 0;
        ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
        *ppuVar1 = *ppuVar1 + 1;
      }
      else if (puVar2 < puVar3) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar3;
        (msg->super_CBitRead).m_pDataIn = puVar3 + 1;
      }
    }
    else {
      puVar2 = &(msg->super_CBitRead).m_nInBufWord;
      *puVar2 = *puVar2 >> 8;
    }
    iVar10 = (msg->super_CBitRead).m_nBitsAvail;
LAB_0052a595:
    if (0x1f < iVar10) {
      local_2c = (msg->super_CBitRead).m_nInBufWord;
      uVar4 = *(uint *)(*(int *)(unaff_EBX + 0x60690e) + 0x80);
      (msg->super_CBitRead).m_nBitsAvail = iVar10 + -0x20;
      if (iVar10 + -0x20 == 0) {
        (msg->super_CBitRead).m_nBitsAvail = 0x20;
        puVar3 = (msg->super_CBitRead).m_pDataIn;
        puVar2 = (msg->super_CBitRead).m_pBufferEnd;
        if (puVar3 == puVar2) {
          (msg->super_CBitRead).m_nBitsAvail = 1;
          (msg->super_CBitRead).m_nInBufWord = 0;
          ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
          *ppuVar1 = *ppuVar1 + 1;
        }
        else if (puVar2 < puVar3) {
          (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
          (msg->super_CBitRead).m_nInBufWord = 0;
        }
        else {
          (msg->super_CBitRead).m_nInBufWord = *puVar3;
          (msg->super_CBitRead).m_pDataIn = puVar3 + 1;
        }
      }
      else {
        puVar2 = &(msg->super_CBitRead).m_nInBufWord;
        *puVar2 = *puVar2;
      }
      local_2c = local_2c & uVar4;
      iVar10 = (msg->super_CBitRead).m_nBitsAvail;
      goto LAB_0052a5d4;
    }
  }
  uVar4 = (msg->super_CBitRead).m_nInBufWord;
  puVar3 = (msg->super_CBitRead).m_pDataIn;
  puVar2 = (msg->super_CBitRead).m_pBufferEnd;
  if (puVar3 == puVar2) {
    iVar10 = 1;
    (msg->super_CBitRead).m_nBitsAvail = 1;
    (msg->super_CBitRead).m_nInBufWord = 0;
    ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
    *ppuVar1 = *ppuVar1 + 1;
    (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
    local_2c = 0;
  }
  else {
    if (puVar2 < puVar3) {
      (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
      (msg->super_CBitRead).m_nInBufWord = 0;
    }
    else {
      (msg->super_CBitRead).m_nInBufWord = *puVar3;
      (msg->super_CBitRead).m_pDataIn = puVar3 + 1;
    }
    local_2c = 0;
    if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
      iVar8 = 0x20 - iVar10;
      local_2c = ((msg->super_CBitRead).m_nInBufWord &
                 *(uint *)(*(int *)(unaff_EBX + 0x60690e) + iVar8 * 4)) << ((byte)iVar10 & 0x1f) |
                 uVar4;
      iVar10 = 0x20 - iVar8;
      (msg->super_CBitRead).m_nBitsAvail = iVar10;
      local_50 = (byte)iVar8;
      puVar2 = &(msg->super_CBitRead).m_nInBufWord;
      *puVar2 = *puVar2 >> (local_50 & 0x1f);
    }
  }
LAB_0052a5d4:
  if (iVar10 < 0x20) {
    uVar4 = (msg->super_CBitRead).m_nInBufWord;
    puVar3 = (msg->super_CBitRead).m_pDataIn;
    puVar2 = (msg->super_CBitRead).m_pBufferEnd;
    if (puVar3 == puVar2) {
      iVar10 = 1;
      (msg->super_CBitRead).m_nBitsAvail = 1;
      (msg->super_CBitRead).m_nInBufWord = 0;
      ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
      *ppuVar1 = *ppuVar1 + 1;
      (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
      local_28 = 0;
    }
    else {
      if (puVar2 < puVar3) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar3;
        (msg->super_CBitRead).m_pDataIn = puVar3 + 1;
      }
      local_28 = 0;
      if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
        iVar8 = 0x20 - iVar10;
        local_28 = ((msg->super_CBitRead).m_nInBufWord &
                   *(uint *)(*(int *)(unaff_EBX + 0x60690e) + iVar8 * 4)) << ((byte)iVar10 & 0x1f) |
                   uVar4;
        iVar10 = 0x20 - iVar8;
        (msg->super_CBitRead).m_nBitsAvail = iVar10;
        local_50 = (byte)iVar8;
        puVar2 = &(msg->super_CBitRead).m_nInBufWord;
        *puVar2 = *puVar2 >> (local_50 & 0x1f);
      }
    }
  }
  else {
    local_28 = (msg->super_CBitRead).m_nInBufWord;
    uVar4 = *(uint *)(*(int *)(unaff_EBX + 0x60690e) + 0x80);
    (msg->super_CBitRead).m_nBitsAvail = iVar10 + -0x20;
    if (iVar10 + -0x20 == 0) {
      (msg->super_CBitRead).m_nBitsAvail = 0x20;
      puVar3 = (msg->super_CBitRead).m_pDataIn;
      puVar2 = (msg->super_CBitRead).m_pBufferEnd;
      if (puVar3 == puVar2) {
        (msg->super_CBitRead).m_nBitsAvail = 1;
        (msg->super_CBitRead).m_nInBufWord = 0;
        ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
        *ppuVar1 = *ppuVar1 + 1;
      }
      else if (puVar2 < puVar3) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar3;
        (msg->super_CBitRead).m_pDataIn = puVar3 + 1;
      }
    }
    else {
      puVar2 = &(msg->super_CBitRead).m_nInBufWord;
      *puVar2 = *puVar2;
    }
    local_28 = local_28 & uVar4;
    iVar10 = (msg->super_CBitRead).m_nBitsAvail;
  }
  if (iVar10 < 0x20) {
    uVar4 = (msg->super_CBitRead).m_nInBufWord;
    puVar3 = (msg->super_CBitRead).m_pDataIn;
    puVar2 = (msg->super_CBitRead).m_pBufferEnd;
    if (puVar3 == puVar2) {
      (msg->super_CBitRead).m_nBitsAvail = 1;
      (msg->super_CBitRead).m_nInBufWord = 0;
      ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
      *ppuVar1 = *ppuVar1 + 1;
      (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
      local_24 = 0;
    }
    else {
      if (puVar2 < puVar3) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar3;
        (msg->super_CBitRead).m_pDataIn = puVar3 + 1;
      }
      local_24 = 0;
      if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
        iVar8 = 0x20 - iVar10;
        local_24 = ((msg->super_CBitRead).m_nInBufWord &
                   *(uint *)(*(int *)(unaff_EBX + 0x60690e) + iVar8 * 4)) << ((byte)iVar10 & 0x1f) |
                   uVar4;
        (msg->super_CBitRead).m_nBitsAvail = 0x20 - iVar8;
        local_50 = (byte)iVar8;
        puVar2 = &(msg->super_CBitRead).m_nInBufWord;
        *puVar2 = *puVar2 >> (local_50 & 0x1f);
      }
    }
  }
  else {
    local_24 = (msg->super_CBitRead).m_nInBufWord;
    uVar4 = *(uint *)(*(int *)(unaff_EBX + 0x60690e) + 0x80);
    (msg->super_CBitRead).m_nBitsAvail = iVar10 + -0x20;
    if (iVar10 + -0x20 == 0) {
      (msg->super_CBitRead).m_nBitsAvail = 0x20;
      puVar3 = (msg->super_CBitRead).m_pDataIn;
      puVar2 = (msg->super_CBitRead).m_pBufferEnd;
      if (puVar3 == puVar2) {
        (msg->super_CBitRead).m_nBitsAvail = 1;
        (msg->super_CBitRead).m_nInBufWord = 0;
        ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
        *ppuVar1 = *ppuVar1 + 1;
      }
      else if (puVar2 < puVar3) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar3;
        (msg->super_CBitRead).m_pDataIn = puVar3 + 1;
      }
    }
    else {
      puVar2 = &(msg->super_CBitRead).m_nInBufWord;
      *puVar2 = *puVar2;
    }
    local_24 = local_24 & uVar4;
  }
  local_20 = local_24;
  (**(code **)(*local_44 + 0xc))(local_44,local_30,&local_2c);
  return;
}


/* __MsgFunc_ChangePaintColor at 00529390 */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

void __MsgFunc_ChangePaintColor(bf_read *msg)

{
  uint32 **ppuVar1;
  uint32 *puVar2;
  int iVar3;
  uint32 *puVar4;
  uint uVar5;
  uint uVar6;
  C_BaseEntity *pCVar7;
  C_WeaponPaintGun *this;
  int iVar8;
  int unaff_EBX;
  uint uVar9;
  byte local_30;
  byte local_28;
  
                    /* Unresolved local var: C_BaseEntity * pEntity@[???]
                       Unresolved local var: C_WeaponPaintGun * pPaintGun@[???] */
  ___i686_get_pc_thunk_bx();
  iVar3 = (msg->super_CBitRead).m_nBitsAvail;
  if (iVar3 < 0x20) {
    uVar5 = (msg->super_CBitRead).m_nInBufWord;
    puVar4 = (msg->super_CBitRead).m_pDataIn;
    puVar2 = (msg->super_CBitRead).m_pBufferEnd;
    if (puVar4 == puVar2) {
      (msg->super_CBitRead).m_nBitsAvail = 1;
      (msg->super_CBitRead).m_nInBufWord = 0;
      ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
      *ppuVar1 = *ppuVar1 + 1;
      (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
      uVar9 = 0;
    }
    else {
      if (puVar2 < puVar4) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar4;
        (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
      }
      uVar9 = 0;
      if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
        iVar8 = 0x20 - iVar3;
        uVar9 = (msg->super_CBitRead).m_nInBufWord;
        uVar6 = *(uint *)(*(int *)(unaff_EBX + 0x6079be) + iVar8 * 4);
        local_28 = (byte)iVar3;
        (msg->super_CBitRead).m_nBitsAvail = 0x20 - iVar8;
        local_30 = (byte)iVar8;
        puVar2 = &(msg->super_CBitRead).m_nInBufWord;
        *puVar2 = *puVar2 >> (local_30 & 0x1f);
        uVar9 = (uVar9 & uVar6) << (local_28 & 0x1f) | uVar5;
      }
    }
  }
  else {
    uVar9 = (msg->super_CBitRead).m_nInBufWord & *(uint *)(*(int *)(unaff_EBX + 0x6079be) + 0x80);
    (msg->super_CBitRead).m_nBitsAvail = iVar3 + -0x20;
    if (iVar3 + -0x20 == 0) {
      (msg->super_CBitRead).m_nBitsAvail = 0x20;
      puVar4 = (msg->super_CBitRead).m_pDataIn;
      puVar2 = (msg->super_CBitRead).m_pBufferEnd;
      if (puVar4 == puVar2) {
        (msg->super_CBitRead).m_nBitsAvail = 1;
        (msg->super_CBitRead).m_nInBufWord = 0;
        ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
        *ppuVar1 = *ppuVar1 + 1;
      }
      else {
        if (puVar2 < puVar4) {
          (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
          (msg->super_CBitRead).m_nInBufWord = 0;
          pCVar7 = UTIL_EntityFromUserMessageEHandle(uVar9);
          goto joined_r0x005294ac;
        }
        (msg->super_CBitRead).m_nInBufWord = *puVar4;
        (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
      }
    }
    else {
      puVar2 = &(msg->super_CBitRead).m_nInBufWord;
      *puVar2 = *puVar2;
    }
  }
  pCVar7 = UTIL_EntityFromUserMessageEHandle(uVar9);
joined_r0x005294ac:
  if (pCVar7 != (C_BaseEntity *)0x0) {
    this = (C_WeaponPaintGun *)
           ___dynamic_cast(pCVar7,*(undefined4 *)(unaff_EBX + 0x60798e),
                           *(undefined4 *)(unaff_EBX + 0x608162),0);
    if (this != (C_WeaponPaintGun *)0x0) {
      iVar3 = (msg->super_CBitRead).m_nBitsAvail;
      if (iVar3 < 8) {
        uVar5 = (msg->super_CBitRead).m_nInBufWord;
        puVar4 = (msg->super_CBitRead).m_pDataIn;
        puVar2 = (msg->super_CBitRead).m_pBufferEnd;
        if (puVar4 == puVar2) {
          (msg->super_CBitRead).m_nBitsAvail = 1;
          (msg->super_CBitRead).m_nInBufWord = 0;
          ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
          *ppuVar1 = *ppuVar1 + 1;
          (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
          uVar9 = 0;
        }
        else {
          if (puVar4 < puVar2) {
            (msg->super_CBitRead).m_nInBufWord = *puVar4;
            (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
          }
          else {
            (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
            (msg->super_CBitRead).m_nInBufWord = 0;
          }
          uVar9 = 0;
          if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
            iVar8 = 8 - iVar3;
            uVar9 = (msg->super_CBitRead).m_nInBufWord;
            uVar6 = *(uint *)(*(int *)(unaff_EBX + 0x6079be) + iVar8 * 4);
            local_28 = (byte)iVar3;
            (msg->super_CBitRead).m_nBitsAvail = 0x20 - iVar8;
            local_30 = (byte)iVar8;
            puVar2 = &(msg->super_CBitRead).m_nInBufWord;
            *puVar2 = *puVar2 >> (local_30 & 0x1f);
            uVar9 = (uVar9 & uVar6) << (local_28 & 0x1f) | uVar5;
          }
        }
      }
      else {
        uVar9 = (msg->super_CBitRead).m_nInBufWord &
                *(uint *)(*(int *)(unaff_EBX + 0x6079be) + 0x20);
        (msg->super_CBitRead).m_nBitsAvail = iVar3 + -8;
        if (iVar3 + -8 == 0) {
          (msg->super_CBitRead).m_nBitsAvail = 0x20;
          puVar4 = (msg->super_CBitRead).m_pDataIn;
          puVar2 = (msg->super_CBitRead).m_pBufferEnd;
          if (puVar4 == puVar2) {
            (msg->super_CBitRead).m_nBitsAvail = 1;
            (msg->super_CBitRead).m_nInBufWord = 0;
            ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
            *ppuVar1 = *ppuVar1 + 1;
          }
          else if (puVar2 < puVar4) {
            (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
            (msg->super_CBitRead).m_nInBufWord = 0;
          }
          else {
            (msg->super_CBitRead).m_nInBufWord = *puVar4;
            (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
          }
        }
        else {
          puVar2 = &(msg->super_CBitRead).m_nInBufWord;
          *puVar2 = *puVar2 >> 8;
        }
      }
      (**(code **)(*(int *)&(this->super_C_BasePortalCombatWeapon).super_C_WeaponPortalBase.
                            super_C_BaseCombatWeapon.super_C_BaseAnimating.super_C_BaseEntity.
                            super_IClientEntity.super_IClientUnknown + 0x45c))(this,uVar9);
      C_WeaponPaintGun::ChangeRenderColor(this,true);
    }
    return;
  }
  DevMsg((char *)(unaff_EBX + 0x4666ba));
  return;
}


/* __MsgFunc_RemoveAllPaint at 00529330 */

void __MsgFunc_RemoveAllPaint(bf_read *msg)

{
  undefined4 *puVar1;
  int *piVar2;
  char cVar3;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  puVar1 = *(undefined4 **)(unaff_EBX + 0x607a17);
  piVar2 = (int *)*puVar1;
  cVar3 = (**(code **)(*piVar2 + 0x2f0))(piVar2);
  if (cVar3 != '\0') {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x00529386. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(int *)*puVar1 + 0x2dc))();
  return;
}


/* __MsgFunc_PaintAllSurfaces at 005296a0 */

void __MsgFunc_PaintAllSurfaces(bf_read *msg)

{
  uint32 **ppuVar1;
  uint32 *puVar2;
  int iVar3;
  uint32 *puVar4;
  char cVar5;
  uint uVar6;
  int unaff_EBX;
  uint uVar7;
  int iVar8;
  
  ___i686_get_pc_thunk_bx();
  cVar5 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x6076a4) + 0x2f0))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x6076a4));
  if (cVar5 != '\0') {
    return;
  }
  iVar3 = (msg->super_CBitRead).m_nBitsAvail;
  if (iVar3 < 8) {
    uVar6 = (msg->super_CBitRead).m_nInBufWord;
    puVar4 = (msg->super_CBitRead).m_pDataIn;
    puVar2 = (msg->super_CBitRead).m_pBufferEnd;
    if (puVar4 == puVar2) {
      (msg->super_CBitRead).m_nBitsAvail = 1;
      (msg->super_CBitRead).m_nInBufWord = 0;
      ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
      *ppuVar1 = *ppuVar1 + 1;
      (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
      uVar7 = 0;
      goto LAB_00529773;
    }
    if (puVar2 < puVar4) {
      (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
      (msg->super_CBitRead).m_nInBufWord = 0;
    }
    else {
      (msg->super_CBitRead).m_nInBufWord = *puVar4;
      (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
    }
    uVar7 = 0;
    if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow != false) goto LAB_00529773;
    iVar8 = 8 - iVar3;
    uVar6 = ((msg->super_CBitRead).m_nInBufWord &
            *(uint *)(*(int *)(unaff_EBX + 0x6076a8) + iVar8 * 4)) << ((byte)iVar3 & 0x1f) | uVar6;
    (msg->super_CBitRead).m_nBitsAvail = 0x20 - iVar8;
    puVar2 = &(msg->super_CBitRead).m_nInBufWord;
    *puVar2 = *puVar2 >> ((byte)iVar8 & 0x1f);
  }
  else {
    uVar6 = (msg->super_CBitRead).m_nInBufWord & *(uint *)(*(int *)(unaff_EBX + 0x6076a8) + 0x20);
    (msg->super_CBitRead).m_nBitsAvail = iVar3 + -8;
    if (iVar3 + -8 == 0) {
      (msg->super_CBitRead).m_nBitsAvail = 0x20;
      puVar4 = (msg->super_CBitRead).m_pDataIn;
      puVar2 = (msg->super_CBitRead).m_pBufferEnd;
      if (puVar4 == puVar2) {
        (msg->super_CBitRead).m_nBitsAvail = 1;
        (msg->super_CBitRead).m_nInBufWord = 0;
        ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
        *ppuVar1 = *ppuVar1 + 1;
      }
      else if (puVar2 < puVar4) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar4;
        (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
      }
    }
    else {
      puVar2 = &(msg->super_CBitRead).m_nInBufWord;
      *puVar2 = *puVar2 >> 8;
    }
  }
  uVar7 = uVar6 & 0xff;
LAB_00529773:
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x6076a4) + 0x2e0))
            ((int *)**(undefined4 **)(unaff_EBX + 0x6076a4),uVar7);
  return;
}


/* __MsgFunc_LoadPaintmapData at 0052abb0 */

/* WARNING: Restarted to delay deadcode elimination for space: stack */

void __MsgFunc_LoadPaintmapData(bf_read *msg)

{
  uint32 **ppuVar1;
  uint32 *puVar2;
  uint uVar3;
  uint32 *puVar4;
  uint uVar5;
  uint uVar6;
  undefined4 uVar7;
  uint32 uVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  int count;
  int iVar12;
  byte bVar13;
  int iVar14;
  int iVar15;
  int unaff_EBX;
  int iVar16;
  float fVar17;
  byte local_80;
  int local_68;
  uint local_4c;
  byte local_45;
  int local_40;
  CUtlMemory<unsigned_char,int> local_34;
  int local_28;
  uchar *local_24;
  float local_20;
  
                    /* Unresolved local var: int nPaintmapID@[???]
                       Unresolved local var: int nPaintmapOffset@[???]
                       Unresolved local var: int nNumRLE@[???]
                       Unresolved local var:
                       CUtlVector<unsigned_char,CUtlMemory<unsigned_char,_int>_> data@[???]
                       Unresolved local var: int offset@[???] */
  ___i686_get_pc_thunk_bx();
  iVar16 = (msg->super_CBitRead).m_nBitsAvail;
  if (iVar16 < 8) {
    uVar3 = (msg->super_CBitRead).m_nInBufWord;
    puVar4 = (msg->super_CBitRead).m_pDataIn;
    puVar2 = (msg->super_CBitRead).m_pBufferEnd;
    if (puVar4 != puVar2) {
      if (puVar2 < puVar4) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar4;
        (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
      }
      local_4c = 0;
      if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
        iVar14 = 8 - iVar16;
        uVar6 = (msg->super_CBitRead).m_nInBufWord;
        uVar5 = *(uint *)(*(int *)(unaff_EBX + 0x60619b) + iVar14 * 4);
        bVar13 = (byte)iVar16;
        iVar16 = 0x20 - iVar14;
        (msg->super_CBitRead).m_nBitsAvail = iVar16;
        local_80 = (byte)iVar14;
        puVar2 = &(msg->super_CBitRead).m_nInBufWord;
        *puVar2 = *puVar2 >> (local_80 & 0x1f);
        local_4c = (uVar6 & uVar5) << (bVar13 & 0x1f) | uVar3;
      }
      goto joined_r0x0052aced;
    }
    iVar16 = 1;
    (msg->super_CBitRead).m_nBitsAvail = 1;
    (msg->super_CBitRead).m_nInBufWord = 0;
    ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
    *ppuVar1 = *ppuVar1 + 1;
    (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
    local_4c = 0;
  }
  else {
    local_4c = (msg->super_CBitRead).m_nInBufWord & *(uint *)(*(int *)(unaff_EBX + 0x60619b) + 0x20)
    ;
    (msg->super_CBitRead).m_nBitsAvail = iVar16 + -8;
    if (iVar16 + -8 == 0) {
      (msg->super_CBitRead).m_nBitsAvail = 0x20;
      puVar4 = (msg->super_CBitRead).m_pDataIn;
      puVar2 = (msg->super_CBitRead).m_pBufferEnd;
      if (puVar4 == puVar2) {
        (msg->super_CBitRead).m_nBitsAvail = 1;
        (msg->super_CBitRead).m_nInBufWord = 0;
        ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
        *ppuVar1 = *ppuVar1 + 1;
      }
      else if (puVar2 < puVar4) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar4;
        (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
      }
    }
    else {
      puVar2 = &(msg->super_CBitRead).m_nInBufWord;
      *puVar2 = *puVar2 >> 8;
    }
    iVar16 = (msg->super_CBitRead).m_nBitsAvail;
joined_r0x0052aced:
    if (0x1f < iVar16) {
      uVar3 = (msg->super_CBitRead).m_nInBufWord;
      uVar6 = *(uint *)(*(int *)(unaff_EBX + 0x60619b) + 0x80);
      (msg->super_CBitRead).m_nBitsAvail = iVar16 + -0x20;
      if (iVar16 + -0x20 == 0) {
        (msg->super_CBitRead).m_nBitsAvail = 0x20;
        puVar4 = (msg->super_CBitRead).m_pDataIn;
        puVar2 = (msg->super_CBitRead).m_pBufferEnd;
        if (puVar4 == puVar2) {
          (msg->super_CBitRead).m_nBitsAvail = 1;
          (msg->super_CBitRead).m_nInBufWord = 0;
          ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
          *ppuVar1 = *ppuVar1 + 1;
        }
        else if (puVar2 < puVar4) {
          (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
          (msg->super_CBitRead).m_nInBufWord = 0;
        }
        else {
          (msg->super_CBitRead).m_nInBufWord = *puVar4;
          (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
        }
      }
      else {
        puVar2 = &(msg->super_CBitRead).m_nInBufWord;
        *puVar2 = *puVar2;
      }
      fVar17 = (float)(uVar3 & uVar6);
      iVar16 = (msg->super_CBitRead).m_nBitsAvail;
      goto LAB_0052ad29;
    }
  }
  uVar3 = (msg->super_CBitRead).m_nInBufWord;
  puVar4 = (msg->super_CBitRead).m_pDataIn;
  puVar2 = (msg->super_CBitRead).m_pBufferEnd;
  if (puVar4 == puVar2) {
    iVar16 = 1;
    (msg->super_CBitRead).m_nBitsAvail = 1;
    (msg->super_CBitRead).m_nInBufWord = 0;
    ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
    *ppuVar1 = *ppuVar1 + 1;
    (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
    fVar17 = 0.0;
  }
  else {
    if (puVar2 < puVar4) {
      (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
      (msg->super_CBitRead).m_nInBufWord = 0;
    }
    else {
      (msg->super_CBitRead).m_nInBufWord = *puVar4;
      (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
    }
    fVar17 = 0.0;
    if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
      iVar14 = 0x20 - iVar16;
      fVar17 = (float)(((msg->super_CBitRead).m_nInBufWord &
                       *(uint *)(*(int *)(unaff_EBX + 0x60619b) + iVar14 * 4)) <<
                       ((byte)iVar16 & 0x1f) | uVar3);
      iVar16 = 0x20 - iVar14;
      (msg->super_CBitRead).m_nBitsAvail = iVar16;
      local_80 = (byte)iVar14;
      puVar2 = &(msg->super_CBitRead).m_nInBufWord;
      *puVar2 = *puVar2 >> (local_80 & 0x1f);
    }
  }
LAB_0052ad29:
  if (iVar16 < 0x20) {
    uVar3 = (msg->super_CBitRead).m_nInBufWord;
    puVar4 = (msg->super_CBitRead).m_pDataIn;
    puVar2 = (msg->super_CBitRead).m_pBufferEnd;
    if (puVar4 == puVar2) {
      (msg->super_CBitRead).m_nBitsAvail = 1;
      (msg->super_CBitRead).m_nInBufWord = 0;
      ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
      *ppuVar1 = *ppuVar1 + 1;
      (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
      local_20 = 0.0;
    }
    else {
      if (puVar2 < puVar4) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar4;
        (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
      }
      local_20 = 0.0;
      if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
        iVar14 = 0x20 - iVar16;
        local_20 = (float)(((msg->super_CBitRead).m_nInBufWord &
                           *(uint *)(*(int *)(unaff_EBX + 0x60619b) + iVar14 * 4)) <<
                           ((byte)iVar16 & 0x1f) | uVar3);
        (msg->super_CBitRead).m_nBitsAvail = 0x20 - iVar14;
        local_80 = (byte)iVar14;
        puVar2 = &(msg->super_CBitRead).m_nInBufWord;
        *puVar2 = *puVar2 >> (local_80 & 0x1f);
      }
    }
  }
  else {
    uVar3 = (msg->super_CBitRead).m_nInBufWord;
    uVar6 = *(uint *)(*(int *)(unaff_EBX + 0x60619b) + 0x80);
    (msg->super_CBitRead).m_nBitsAvail = iVar16 + -0x20;
    if (iVar16 + -0x20 == 0) {
      (msg->super_CBitRead).m_nBitsAvail = 0x20;
      puVar4 = (msg->super_CBitRead).m_pDataIn;
      puVar2 = (msg->super_CBitRead).m_pBufferEnd;
      if (puVar4 == puVar2) {
        (msg->super_CBitRead).m_nBitsAvail = 1;
        (msg->super_CBitRead).m_nInBufWord = 0;
        ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
        *ppuVar1 = *ppuVar1 + 1;
      }
      else if (puVar2 < puVar4) {
        (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
        (msg->super_CBitRead).m_nInBufWord = 0;
      }
      else {
        (msg->super_CBitRead).m_nInBufWord = *puVar4;
        (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
      }
    }
    else {
      puVar2 = &(msg->super_CBitRead).m_nInBufWord;
      *puVar2 = *puVar2;
    }
    local_20 = (float)(uVar3 & uVar6);
  }
  iVar16 = (int)local_20;
  local_34.m_pMemory = (uchar *)0x0;
  local_34.m_nAllocationCount = 0;
  local_34.m_nGrowSize = 0;
  local_28 = 0;
  local_24 = (uchar *)0x0;
  if (0 < iVar16) {
    uVar3 = *(uint *)(*(int *)(unaff_EBX + 0x60619b) + 0x80);
    uVar7 = *(undefined4 *)(*(int *)(unaff_EBX + 0x60619b) + 0x20);
    local_40 = 0;
    local_68 = 0;
    do {
      iVar9 = local_28;
      iVar14 = (msg->super_CBitRead).m_nBitsAvail;
      if (iVar14 < 0x20) {
        uVar6 = (msg->super_CBitRead).m_nInBufWord;
        puVar4 = (msg->super_CBitRead).m_pDataIn;
        puVar2 = (msg->super_CBitRead).m_pBufferEnd;
        if (puVar4 == puVar2) {
          iVar14 = 1;
          (msg->super_CBitRead).m_nBitsAvail = 1;
          (msg->super_CBitRead).m_nInBufWord = 0;
          ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
          *ppuVar1 = *ppuVar1 + 1;
          (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
          local_20 = 0.0;
        }
        else {
          if (puVar2 < puVar4) {
            (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
            (msg->super_CBitRead).m_nInBufWord = 0;
          }
          else {
            (msg->super_CBitRead).m_nInBufWord = *puVar4;
            (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
          }
          local_20 = 0.0;
          if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
            iVar15 = 0x20 - iVar14;
            local_20 = (float)(((msg->super_CBitRead).m_nInBufWord &
                               *(uint *)(*(int *)(unaff_EBX + 0x60619b) + iVar15 * 4)) <<
                               ((byte)iVar14 & 0x1f) | uVar6);
            iVar14 = 0x20 - iVar15;
            (msg->super_CBitRead).m_nBitsAvail = iVar14;
            local_80 = (byte)iVar15;
            puVar2 = &(msg->super_CBitRead).m_nInBufWord;
            *puVar2 = *puVar2 >> (local_80 & 0x1f);
          }
        }
      }
      else {
        uVar6 = (msg->super_CBitRead).m_nInBufWord;
        (msg->super_CBitRead).m_nBitsAvail = iVar14 + -0x20;
        if (iVar14 + -0x20 == 0) {
          (msg->super_CBitRead).m_nBitsAvail = 0x20;
          puVar4 = (msg->super_CBitRead).m_pDataIn;
          puVar2 = (msg->super_CBitRead).m_pBufferEnd;
          if (puVar4 == puVar2) {
            (msg->super_CBitRead).m_nBitsAvail = 1;
            (msg->super_CBitRead).m_nInBufWord = 0;
            ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
            *ppuVar1 = *ppuVar1 + 1;
          }
          else if (puVar2 < puVar4) {
            (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
            (msg->super_CBitRead).m_nInBufWord = 0;
          }
          else {
            (msg->super_CBitRead).m_nInBufWord = *puVar4;
            (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
          }
        }
        else {
          puVar2 = &(msg->super_CBitRead).m_nInBufWord;
          *puVar2 = *puVar2;
        }
        local_20 = (float)(uVar3 & uVar6);
        iVar14 = (msg->super_CBitRead).m_nBitsAvail;
      }
      iVar15 = (int)local_20;
      if (iVar14 < 8) {
        uVar8 = (msg->super_CBitRead).m_nInBufWord;
        puVar4 = (msg->super_CBitRead).m_pDataIn;
        puVar2 = (msg->super_CBitRead).m_pBufferEnd;
        if (puVar4 == puVar2) {
          (msg->super_CBitRead).m_nBitsAvail = 1;
          (msg->super_CBitRead).m_nInBufWord = 0;
          ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
          *ppuVar1 = *ppuVar1 + 1;
          (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
          local_45 = 0;
        }
        else {
          if (puVar2 < puVar4) {
            (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
            (msg->super_CBitRead).m_nInBufWord = 0;
          }
          else {
            (msg->super_CBitRead).m_nInBufWord = *puVar4;
            (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
          }
          local_45 = 0;
          if ((msg->super_CBitRead).super_CBitBuffer.m_bOverflow == false) {
            iVar10 = 8 - iVar14;
            local_45 = (byte)(((msg->super_CBitRead).m_nInBufWord &
                              *(uint *)(*(int *)(unaff_EBX + 0x60619b) + iVar10 * 4)) <<
                             ((byte)iVar14 & 0x1f)) | (byte)uVar8;
            (msg->super_CBitRead).m_nBitsAvail = 0x20 - iVar10;
            local_80 = (byte)iVar10;
            puVar2 = &(msg->super_CBitRead).m_nInBufWord;
            *puVar2 = *puVar2 >> (local_80 & 0x1f);
          }
        }
      }
      else {
        local_45 = (byte)uVar7 & (byte)(msg->super_CBitRead).m_nInBufWord;
        (msg->super_CBitRead).m_nBitsAvail = iVar14 + -8;
        if (iVar14 + -8 == 0) {
          (msg->super_CBitRead).m_nBitsAvail = 0x20;
          puVar4 = (msg->super_CBitRead).m_pDataIn;
          puVar2 = (msg->super_CBitRead).m_pBufferEnd;
          if (puVar4 == puVar2) {
            (msg->super_CBitRead).m_nBitsAvail = 1;
            (msg->super_CBitRead).m_nInBufWord = 0;
            ppuVar1 = &(msg->super_CBitRead).m_pDataIn;
            *ppuVar1 = *ppuVar1 + 1;
          }
          else if (puVar2 < puVar4) {
            (msg->super_CBitRead).super_CBitBuffer.m_bOverflow = true;
            (msg->super_CBitRead).m_nInBufWord = 0;
          }
          else {
            (msg->super_CBitRead).m_nInBufWord = *puVar4;
            (msg->super_CBitRead).m_pDataIn = puVar4 + 1;
          }
        }
        else {
          puVar2 = &(msg->super_CBitRead).m_nInBufWord;
          *puVar2 = *puVar2 >> 8;
        }
      }
      iVar14 = local_40 + iVar15;
      if ((local_28 < iVar14) && (iVar10 = iVar14 - local_28, iVar10 != 0)) {
        iVar11 = iVar10 + local_28;
        iVar12 = iVar11;
        if (local_34.m_nAllocationCount < iVar11) {
          CUtlMemory<unsigned_char,int>::Grow(&local_34,iVar11 - local_34.m_nAllocationCount);
          iVar12 = iVar10 + local_28;
        }
        local_24 = local_34.m_pMemory;
                    /* Unresolved local var: int numToMove@[???] */
        count = (iVar12 - iVar9) - iVar10;
        local_28 = iVar12;
        if ((0 < count) && (0 < iVar10)) {
          _V_memmove(local_34.m_pMemory + iVar11,local_34.m_pMemory + iVar9,count);
        }
      }
      _V_memset(local_34.m_pMemory + local_40,(uint)local_45,iVar15);
      local_68 = local_68 + 1;
      local_40 = iVar14;
    } while (local_68 != iVar16);
  }
  (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x606197) + 0x2e4))
            ((int *)**(undefined4 **)(unaff_EBX + 0x606197),&local_34,local_4c,(int)fVar17);
  local_28 = 0;
  if (-1 < local_34.m_nGrowSize) {
    if (local_34.m_pMemory != (uchar *)0x0) {
      (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x606163) + 8))
                ((int *)**(undefined4 **)(unaff_EBX + 0x606163),local_34.m_pMemory);
      local_34.m_pMemory = (uchar *)0x0;
    }
    local_34.m_nAllocationCount = 0;
  }
  if ((-1 < local_34.m_nGrowSize) && (local_34.m_pMemory != (uchar *)0x0)) {
    local_24 = local_34.m_pMemory;
    (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x606163) + 8))
              ((int *)**(undefined4 **)(unaff_EBX + 0x606163),local_34.m_pMemory);
  }
  return;
}


/* __static_initialization_and_destruction_0 at 00075e00 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  int unaff_EBX;
  longlong lVar1;
  
  lVar1 = ___i686_get_pc_thunk_bx();
  if (lVar1 != 0xffff00000001) {
    return;
  }
  *(undefined1 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x400].m_pPrev + unaff_EBX)
       = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x400].m_pPrev + unaff_EBX + 1) = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x400].m_pPrev + unaff_EBX + 2) = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x400].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x400].m_pNext + unaff_EBX)
       = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x401].m_pEntity + unaff_EBX) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x401].m_SerialNumber + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x401].m_pPrev + unaff_EBX)
       = 0x7f7fffff;
  *(undefined4 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x401].m_pNext + unaff_EBX)
       = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x402].m_pEntity + unaff_EBX) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x402].m_SerialNumber + unaff_EBX) = 0;
  *(undefined4 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x402].m_pPrev + unaff_EBX)
       = 0;
  *(undefined4 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x402].m_pNext + unaff_EBX)
       = 0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x403].m_pEntity + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x403].m_SerialNumber + unaff_EBX) =
       0x7f7fffff;
  *(undefined4 *)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x403].m_pPrev + unaff_EBX)
       = 0x7f7fffff;
  *(undefined **)((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x403].m_pNext + unaff_EBX)
       = &UNK_00abbffc + unaff_EBX;
  CAutoGameSystem::CAutoGameSystem
            ((CAutoGameSystem *)
             ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x404].m_pEntity + unaff_EBX),
             (char *)0x0);
  *(undefined **)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x404].m_pEntity + unaff_EBX) =
       &UNK_00b210fc + unaff_EBX;
  ___cxa_atexit(unaff_EBX + 0x8dba54,0,*(undefined4 *)(&DAT_00abaf14 + unaff_EBX));
  return;
}


/* C_PaintInitHelper::~C_PaintInitHelper at 0052b760 */

/* DWARF original prototype: void ~C_PaintInitHelper(C_PaintInitHelper * this, int __in_chrg) */

void __thiscall C_PaintInitHelper::~C_PaintInitHelper(C_PaintInitHelper *this,int __in_chrg)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(extraout_ECX + 0x6078c0);
  IGameSystem::~IGameSystem((IGameSystem *)this,__in_chrg);
  return;
}


/* C_PaintInitHelper::~C_PaintInitHelper at 0052b780 */

/* DWARF original prototype: void ~C_PaintInitHelper(C_PaintInitHelper * this, int __in_chrg) */

void __thiscall C_PaintInitHelper::~C_PaintInitHelper(C_PaintInitHelper *this,int __in_chrg)

{
  int unaff_EBX;
  int in_stack_ffffffe8;
  
  ___i686_get_pc_thunk_bx();
  (this->super_CAutoGameSystem).super_CBaseGameSystem.super_IGameSystem._vptr_IGameSystem =
       (_func_int_varargs **)(unaff_EBX + 0x607897);
  IGameSystem::~IGameSystem((IGameSystem *)this,in_stack_ffffffe8);
  operator_delete(this);
  return;
}


/* C_PaintInitHelper::Init at 0052b440 */

/* DWARF original prototype: bool Init(C_PaintInitHelper * this) */

bool __thiscall C_PaintInitHelper::Init(C_PaintInitHelper *this)

{
  undefined4 *puVar1;
  int iVar2;
  int unaff_EBX;
  code *name;
  code *name_00;
  CSetActiveSplitScreenPlayerGuard *in_stack_ffffff7c;
  char *in_stack_ffffff80;
  int in_stack_ffffff84;
  int in_stack_ffffff88;
  int in_stack_ffffff8c;
  undefined1 in_stack_ffffff90;
  CSetActiveSplitScreenPlayerGuard local_3c;
  
  ___i686_get_pc_thunk_bx();
  iVar2 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x605907) + 0x1f8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x605907));
  CSetActiveSplitScreenPlayerGuard::CSetActiveSplitScreenPlayerGuard
            (&local_3c,(char *)(unaff_EBX + 0x46456f),0x8d,0,iVar2,false,in_stack_ffffff7c,
             in_stack_ffffff80,in_stack_ffffff84,in_stack_ffffff88,in_stack_ffffff8c,
             (bool)in_stack_ffffff90);
  puVar1 = *(undefined4 **)(unaff_EBX + 0x60590f);
  CUserMessages::HookMessage
            ((CUserMessages *)*puVar1,(char *)(unaff_EBX + 0x43c756),
             (pfnUserMsgHook)(unaff_EBX + -0x1c51));
  CUserMessages::HookMessage
            ((CUserMessages *)*puVar1,(char *)(unaff_EBX + 0x4645d5),
             (pfnUserMsgHook)(unaff_EBX + -0x1011));
  CUserMessages::HookMessage
            ((CUserMessages *)*puVar1,(char *)(CLCD::ShowItems_R + unaff_EBX + 1),
             (pfnUserMsgHook)(unaff_EBX + -0x20c1));
  CUserMessages::HookMessage
            ((CUserMessages *)*puVar1,&UNK_0043c772 + unaff_EBX,
             (pfnUserMsgHook)(unaff_EBX + -0x2121));
  CUserMessages::HookMessage
            ((CUserMessages *)*puVar1,(char *)(unaff_EBX + 0x4645f5),
             (pfnUserMsgHook)(unaff_EBX + -0x1db1));
  name = CHudIcons::AddUnsearchableHudIconToList + unaff_EBX + 1;
  name_00 = name;
  CUserMessages::HookMessage
            ((CUserMessages *)*puVar1,(char *)name,(pfnUserMsgHook)(unaff_EBX + -0x8a1));
  CSetActiveSplitScreenPlayerGuard::~CSetActiveSplitScreenPlayerGuard(&local_3c,(int)name);
  iVar2 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x605907) + 0x1f8))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x605907));
  CSetActiveSplitScreenPlayerGuard::CSetActiveSplitScreenPlayerGuard
            (&local_3c,(char *)(unaff_EBX + 0x46456f),0x8d,1,iVar2,false,in_stack_ffffff7c,
             in_stack_ffffff80,in_stack_ffffff84,in_stack_ffffff88,in_stack_ffffff8c,
             SUB41(name_00,0));
  CUserMessages::HookMessage
            ((CUserMessages *)*puVar1,(char *)(unaff_EBX + 0x43c756),
             (pfnUserMsgHook)(unaff_EBX + -0x1c51));
  CUserMessages::HookMessage
            ((CUserMessages *)*puVar1,(char *)(unaff_EBX + 0x4645d5),
             (pfnUserMsgHook)(unaff_EBX + -0x1011));
  CUserMessages::HookMessage
            ((CUserMessages *)*puVar1,(char *)(CLCD::ShowItems_R + unaff_EBX + 1),
             (pfnUserMsgHook)(unaff_EBX + -0x20c1));
  CUserMessages::HookMessage
            ((CUserMessages *)*puVar1,&UNK_0043c772 + unaff_EBX,
             (pfnUserMsgHook)(unaff_EBX + -0x2121));
  CUserMessages::HookMessage
            ((CUserMessages *)*puVar1,(char *)(unaff_EBX + 0x4645f5),
             (pfnUserMsgHook)(unaff_EBX + -0x1db1));
  CUserMessages::HookMessage
            ((CUserMessages *)*puVar1,(char *)name_00,(pfnUserMsgHook)(unaff_EBX + -0x8a1));
  CSetActiveSplitScreenPlayerGuard::~CSetActiveSplitScreenPlayerGuard(&local_3c,(int)name_00);
  return true;
}


/* __tcf_0 at 00951860 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  *(int *)(&DAT_00366748 + extraout_ECX) = extraout_ECX + 0x1e17c0;
  IGameSystem::~IGameSystem((IGameSystem *)(&DAT_00366748 + extraout_ECX),in_stack_00000008);
  return;
}


/* _GLOBAL__I__Z20__MsgFunc_PaintWorldR7bf_read at 00075ef0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__Z20__MsgFunc_PaintWorldR7bf_read(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

