/* DWARF-guided pseudocode for game/shared/portal2/paint_saverestore.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* __static_initialization_and_destruction_0 at 00015ea0 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  int unaff_EBX;
  longlong lVar1;
  
  lVar1 = ___i686_get_pc_thunk_bx();
  if (lVar1 != 0xffff00000001) {
    return;
  }
  (&DAT_00c208b7)[unaff_EBX] = 0;
  (&DAT_00c208b8)[unaff_EBX] = 0;
  (&DAT_00c208b9)[unaff_EBX] = 0;
  (&DAT_00c208ba)[unaff_EBX] = 0;
  *(undefined4 *)(&DAT_00c208bb + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c208bf + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c208c3 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c208c7 + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c208cb + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c208cf + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c208d3 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c208d7 + unaff_EBX) = 0;
  *(undefined4 *)(&DAT_00c208db + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)(&DAT_00c208df + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&vgui::DmxElementUnpackInitvgui<vgui::Button>(vgui::Button*)::unpack + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)(&DAT_00c208e7 + unaff_EBX) = 0x7f7fffff;
  *(undefined **)(&DAT_00c208eb + unaff_EBX) = &UNK_00b1bf5f + unaff_EBX;
  *(undefined **)(s_SortBuffer + unaff_EBX + 0x4ba57) = &UNK_00b253bf + unaff_EBX;
  return;
}


/* CPaintSaveRestoreBlockHandler::GetBlockName at 001b46b0 */

/* DWARF original prototype: char * GetBlockName(CPaintSaveRestoreBlockHandler * this) */

char * __thiscall CPaintSaveRestoreBlockHandler::GetBlockName(CPaintSaveRestoreBlockHandler *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (char *)(extraout_ECX + 0x7b46e9);
}


/* CPaintSaveRestoreBlockHandler::PreSave at 001b46c0 */

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


/* CPaintSaveRestoreBlockHandler::Save at 001b46d0 */

/* DWARF original prototype: void Save(CPaintSaveRestoreBlockHandler * this, ISave * pSave) */

void __thiscall
CPaintSaveRestoreBlockHandler::Save(CPaintSaveRestoreBlockHandler *this,ISave *pSave)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
                    /* WARNING: Could not recover jumptable at 0x001b46ee. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(int *)**(undefined4 **)("BackgroundColor" + unaff_EBX + 0xc) + 0x2d0))();
  return;
}


/* CPaintSaveRestoreBlockHandler::WriteSaveHeaders at 001b46f0 */

/* DWARF original prototype: void WriteSaveHeaders(CPaintSaveRestoreBlockHandler * this, ISave *
   param_1) */

void __thiscall
CPaintSaveRestoreBlockHandler::WriteSaveHeaders(CPaintSaveRestoreBlockHandler *this,ISave *param_1)

{
  return;
}


/* CPaintSaveRestoreBlockHandler::PostSave at 001b4700 */

/* DWARF original prototype: void PostSave(CPaintSaveRestoreBlockHandler * this) */

void __thiscall CPaintSaveRestoreBlockHandler::PostSave(CPaintSaveRestoreBlockHandler *this)

{
  return;
}


/* CPaintSaveRestoreBlockHandler::PreRestore at 001b4710 */

/* DWARF original prototype: void PreRestore(CPaintSaveRestoreBlockHandler * this) */

void __thiscall CPaintSaveRestoreBlockHandler::PreRestore(CPaintSaveRestoreBlockHandler *this)

{
  return;
}


/* CPaintSaveRestoreBlockHandler::ReadRestoreHeaders at 001b4720 */

/* DWARF original prototype: void ReadRestoreHeaders(CPaintSaveRestoreBlockHandler * this, IRestore
   * param_1) */

void __thiscall
CPaintSaveRestoreBlockHandler::ReadRestoreHeaders
          (CPaintSaveRestoreBlockHandler *this,IRestore *param_1)

{
  return;
}


/* CPaintSaveRestoreBlockHandler::Restore at 001b4730 */

/* DWARF original prototype: void Restore(CPaintSaveRestoreBlockHandler * this, IRestore * pRestore,
   bool fCreatePlayers) */

void __thiscall
CPaintSaveRestoreBlockHandler::Restore
          (CPaintSaveRestoreBlockHandler *this,IRestore *pRestore,bool fCreatePlayers)

{
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
                    /* WARNING: Could not recover jumptable at 0x001b474e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(int *)**(undefined4 **)("m_iIconTall" + unaff_EBX + 2) + 0x2d0))();
  return;
}


/* CPaintSaveRestoreBlockHandler::PostRestore at 001b4750 */

/* DWARF original prototype: void PostRestore(CPaintSaveRestoreBlockHandler * this) */

void __thiscall CPaintSaveRestoreBlockHandler::PostRestore(CPaintSaveRestoreBlockHandler *this)

{
  return;
}


/* GetPaintSaveRestoreBlockHandler at 001b46a0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

ISaveRestoreBlockHandler * GetPaintSaveRestoreBlockHandler(void)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ISaveRestoreBlockHandler *)((int)&PTR_DrawModel_00b8b8d8 + extraout_ECX);
}


/* _GLOBAL__I_g_PaintSaveRestoreBlockHandler at 00015f50 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I_g_PaintSaveRestoreBlockHandler(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

