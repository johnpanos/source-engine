/* DWARF-guided pseudocode for game/client/portal/c_func_portal_bumper.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* ClientClassInit<DT_FuncPortalBumper::ignored> at 00069760 */

int ClientClassInit<DT_FuncPortalBumper::ignored>(ignored *param_1)

{
  int iVar1;
  int iVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  iVar1 = (int)&vec2_origin.x + unaff_EBX;
  if (*(char *)((int)&vec2_origin.x + unaff_EBX) == '\0') {
    iVar2 = ___cxa_guard_acquire(iVar1);
    if (iVar2 != 0) {
      RecvPropInt((RecvProp_conflict1 *)((int)&vec2_invalid.x + unaff_EBX),
                  (char *)(unaff_EBX + 0x920ffc),0,4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)(UNSPECIFIED_LOGGING_COLOR._color + unaff_EBX),
                        &UNK_00925b40 + unaff_EBX,0,0,
                        (RecvTable *)**(undefined4 **)(&DAT_00afa6bc + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00afa678 + unaff_EBX));
      RecvPropBool((RecvProp *)(unaff_EBX + 0xc61304),(char *)(unaff_EBX + 0x93b654),0xa5d,1);
      ___cxa_guard_release(iVar1);
    }
  }
  RecvTable::Construct
            ((RecvTable *)(s_SortBuffer + unaff_EBX + 0x55f4),
             (RecvProp_conflict1 *)(UNSPECIFIED_LOGGING_COLOR._color + unaff_EBX),2,
             (char *)(unaff_EBX + 0x94c605));
  return 1;
}


/* __static_initialization_and_destruction_0 at 000698b0 */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  uint8 *puVar1;
  int *piVar2;
  int iVar3;
  int unaff_EBX;
  longlong lVar4;
  
  lVar4 = ___i686_get_pc_thunk_bx();
  if (lVar4 != 0xffff00000001) {
    return;
  }
  (&DAT_00c610ff)[unaff_EBX] = 0;
  *(undefined1 *)((int)&vgui::ScalableImagePanel::GetKBMap()::s_pMap + unaff_EBX) = 0;
  *(undefined1 *)((int)&vgui::ScalableImagePanel::GetKBMap()::s_pMap + unaff_EBX + 1) = 0;
  *(undefined1 *)((int)&vgui::ScalableImagePanel::GetKBMap()::s_pMap + unaff_EBX + 2) = 0;
  *(undefined4 *)((int)&vgui::ScalableImagePanel::GetKBMap()::s_pMap + unaff_EBX + 3) = 0;
  *(undefined4 *)(&DAT_00c61107 + unaff_EBX) = 0;
  *(undefined4 *)((int)&vgui::ScalableImagePanel::GetKBMap::s_pMap + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&vgui::ScalableImagePanel::GetMessageMap::s_pMap + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)((int)&vgui::ScalableImagePanel::KB_ChainToMap()::chained + unaff_EBX + 1) = 0;
  *(undefined4 *)(&DAT_00c61117 + unaff_EBX) = 0;
  *(undefined4 *)(&vgui::ScrollBar::PanelMessageFunc_OnSliderMoved::InitVar()::bAdded + unaff_EBX) =
       0;
  *(undefined4 *)
   ((int)&vgui::ScrollBar::PanelAnimationVar_m_bAutoHideButtons::InitVar()::bAdded + unaff_EBX + 2)
       = 0;
  *(undefined4 *)((int)&g_ScrollBar_VerticalLinkerHack + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&g_ScrollBar_HorizontalLinkerHack + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&vgui::ScrollBarSlider::GetKBMap::s_pMap + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&vgui::ScrollBarSlider::GetMessageMap::s_pMap + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined **)((int)&vgui::ScrollBarSlider::KB_ChainToMap()::chained + unaff_EBX + 1) =
       &UNK_00afb5c7 + unaff_EBX;
  *(int *)(s_SortBuffer + unaff_EBX + 0x5497) = unaff_EBX + 0x94c4cc;
  *(int *)(s_SortBuffer + unaff_EBX + 0x548f) = unaff_EBX + 0x46733f;
  puVar1 = s_SortBuffer + unaff_EBX + 0x5493;
  puVar1[0] = '\0';
  puVar1[1] = '\0';
  puVar1[2] = '\0';
  puVar1[3] = '\0';
  *(RecvTable **)(s_SortBuffer + unaff_EBX + 0x549b) =
       (RecvTable *)(s_SortBuffer + unaff_EBX + 0x54a7);
  piVar2 = *(int **)(&DAT_00afa523 + unaff_EBX);
  *(int *)(s_SortBuffer + unaff_EBX + 0x549f) = *piVar2;
  *piVar2 = (int)(s_SortBuffer + unaff_EBX + 0x548f);
  RecvTable::RecvTable((RecvTable *)(s_SortBuffer + unaff_EBX + 0x54a7));
  ___cxa_atexit(unaff_EBX + 0x91079f,0,*(undefined4 *)(&DAT_00afa4ab + unaff_EBX));
  iVar3 = ClientClassInit<DT_FuncPortalBumper::ignored>((ignored *)0x0);
  *(int *)(s_SortBuffer + unaff_EBX + 0x54bb) = iVar3;
  return;
}


/* C_FuncPortalBumper::YouForgotToImplementOrDeclareClientClass at 004d0bd0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_FuncPortalBumper": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: int YouForgotToImplementOrDeclareClientClass(C_FuncPortalBumper * this)
    */

int __thiscall
C_FuncPortalBumper::YouForgotToImplementOrDeclareClientClass(C_FuncPortalBumper *this)

{
  return 0;
}


/* C_FuncPortalBumper::GetClientClass at 004d0bf0 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* WARNING: Struct "C_FuncPortalBumper": ignoring overlapping field "m_bActive" */
/* DWARF original prototype: ClientClass * GetClientClass(C_FuncPortalBumper * this) */

ClientClass * __thiscall C_FuncPortalBumper::GetClientClass(C_FuncPortalBumper *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (ClientClass *)(extraout_ECX + 0x895b38);
}


/* _C_FuncPortalBumper_CreateObject at 004d0c00 */

/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */

IClientNetworkable * _C_FuncPortalBumper_CreateObject(int entnum,int serialNum)

{
  C_BaseEntity *this;
  int unaff_EBX;
  
                    /* Unresolved local var: C_FuncPortalBumper * pRet@[???] */
  ___i686_get_pc_thunk_bx();
  this = C_BaseEntity::operator_new(0xa60);
  C_BaseEntity::C_BaseEntity(this);
  (this->super_IClientEntity).super_IClientUnknown.super_IHandleEntity._vptr_IHandleEntity =
       (_func_int_varargs **)(unaff_EBX + 0x6e1cd4);
  (this->super_IClientEntity).super_IClientRenderable._vptr_IClientRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6e2070);
  (this->super_IClientEntity).super_IClientNetworkable._vptr_IClientNetworkable =
       (_func_int_varargs **)(unaff_EBX + 0x6e2128);
  (this->super_IClientEntity).super_IClientThinkable._vptr_IClientThinkable =
       (_func_int_varargs **)(unaff_EBX + 0x6e2164);
  (this->super_IClientModelRenderable)._vptr_IClientModelRenderable =
       (_func_int_varargs **)(unaff_EBX + 0x6e2180);
  (**(code **)(unaff_EBX + 0x6e1d90))(this,entnum,serialNum);
  return &(this->super_IClientEntity).super_IClientNetworkable;
}


/* __tcf_0 at 0097a060 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3ec6e0),in_stack_00000008);
  return;
}


/* _GLOBAL__I__ZN18C_FuncPortalBumper17m_pClassRecvTableE at 000699d0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN18C_FuncPortalBumper17m_pClassRecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

