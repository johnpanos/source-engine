/* DWARF-guided pseudocode for game/shared/portal2/paint_saverestore.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 000161d0 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  int unaff_EBX;
  longlong lVar1;
  
  lVar1 = ___i686_get_pc_thunk_bx();
  if (lVar1 != 0xffff00000001) {
    return;
  }
  (&DAT_00c56547)[unaff_EBX] = 0;
  (&DAT_00c56548)[unaff_EBX] = 0;
  (&DAT_00c56549)[unaff_EBX] = 0;
  (&DAT_00c5654a)[unaff_EBX] = 0;
  *(undefined4 *)(&DAT_00c5654b + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c5654f + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c56553 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c56557 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c5655b + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c5655f + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c56563 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c56567 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c5656b + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c5656f + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c56573 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c56577 + unaff_EBX) = 0x7f7fffff;
  *(undefined **)(&DAT_00c5657b + unaff_EBX) = &UNK_00b4ecaf + unaff_EBX;
  *(undefined **)(s_SortBuffer + unaff_EBX + 0x4b727) = &UNK_00b57c2f + unaff_EBX;
  return;
}


/* CPaintSaveRestoreBlockHandler::GetBlockName at 001b3060 */

/* DWARF original prototype: char * GetBlockName(CPaintSaveRestoreBlockHandler * this) */

char * __thiscall CPaintSaveRestoreBlockHandler::GetBlockName(CPaintSaveRestoreBlockHandler *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (char *)(CParticleOperatorDefinition<C_OP_SetControlPointToCenter>::GetId +
                 extraout_ECX + 5);
}


/* CPaintSaveRestoreBlockHandler::PreSave at 001b3070 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void PreSave(CPaintSaveRestoreBlockHandler * this, CSaveRestoreData *
   param_1) */

void __thiscall
CPaintSaveRestoreBlockHandler::PreSave
          (CPaintSaveRestoreBlockHandler *this,CSaveRestoreData *param_1)

{
  return;
}


/* CPaintSaveRestoreBlockHandler::Save at 001b3080 */

/* DWARF original prototype: void Save(CPaintSaveRestoreBlockHandler * this, ISave * pSave) */

void __thiscall
CPaintSaveRestoreBlockHandler::Save(CPaintSaveRestoreBlockHandler *this,ISave *pSave)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
                    /* WARNING: Could not recover jumptable at 0x001b309e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(int *)**(undefined4 **)("19CHudChatFilterPanel" + unaff_EBX + 0xd) + 0x2d0))();
  return;
}


/* CPaintSaveRestoreBlockHandler::WriteSaveHeaders at 001b30a0 */

/* DWARF original prototype: void WriteSaveHeaders(CPaintSaveRestoreBlockHandler * this, ISave *
   param_1) */

void __thiscall
CPaintSaveRestoreBlockHandler::WriteSaveHeaders(CPaintSaveRestoreBlockHandler *this,ISave *param_1)

{
  return;
}


/* CPaintSaveRestoreBlockHandler::PostSave at 001b30b0 */

/* DWARF original prototype: void PostSave(CPaintSaveRestoreBlockHandler * this) */

void __thiscall CPaintSaveRestoreBlockHandler::PostSave(CPaintSaveRestoreBlockHandler *this)

{
  return;
}


/* CPaintSaveRestoreBlockHandler::PreRestore at 001b30c0 */

/* DWARF original prototype: void PreRestore(CPaintSaveRestoreBlockHandler * this) */

void __thiscall CPaintSaveRestoreBlockHandler::PreRestore(CPaintSaveRestoreBlockHandler *this)

{
  return;
}


/* CPaintSaveRestoreBlockHandler::ReadRestoreHeaders at 001b30d0 */

/* DWARF original prototype: void ReadRestoreHeaders(CPaintSaveRestoreBlockHandler * this, IRestore
   * param_1) */

void __thiscall
CPaintSaveRestoreBlockHandler::ReadRestoreHeaders
          (CPaintSaveRestoreBlockHandler *this,IRestore *param_1)

{
  return;
}


/* CPaintSaveRestoreBlockHandler::Restore at 001b30e0 */

/* DWARF original prototype: void Restore(CPaintSaveRestoreBlockHandler * this, IRestore * pRestore,
   bool fCreatePlayers) */

void __thiscall
CPaintSaveRestoreBlockHandler::Restore
          (CPaintSaveRestoreBlockHandler *this,IRestore *pRestore,bool fCreatePlayers)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
                    /* WARNING: Could not recover jumptable at 0x001b30fe. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(int *)**(undefined4 **)("resource/chat_%language%.txt" + unaff_EBX + 0xb) + 0x2d0))
            ();
  return;
}


/* CPaintSaveRestoreBlockHandler::PostRestore at 001b3100 */

/* DWARF original prototype: void PostRestore(CPaintSaveRestoreBlockHandler * this) */

void __thiscall CPaintSaveRestoreBlockHandler::PostRestore(CPaintSaveRestoreBlockHandler *this)

{
  return;
}


/* GetPaintSaveRestoreBlockHandler at 001b3050 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

ISaveRestoreBlockHandler * GetPaintSaveRestoreBlockHandler(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ISaveRestoreBlockHandler *)((int)&PTR_OnParseMapDataFinished_00ba6288 + extraout_ECX);
}


/* _GLOBAL__I_g_PaintSaveRestoreBlockHandler at 00016280 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_g_PaintSaveRestoreBlockHandler(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

