/* DWARF-guided pseudocode for game/client/portal2/c_portal_playerlocaldata.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* ClientClassInit<DT_PortalLocal::ignored> at 00076dd0 */

int ClientClassInit<DT_PortalLocal::ignored>(ignored *param_1)

{
  RecvProp_conflict1 pArrayProp;
  RecvProp_conflict1 pArrayProp_00;
  RecvProp_conflict1 pArrayProp_01;
  RecvProp_conflict1 pArrayProp_02;
  int iVar1;
  int unaff_EBX;
  RecvProp_conflict1 *pRVar2;
  RecvProp *pRVar3;
  undefined4 *puVar4;
  char *pcVar5;
  RecvVarProxyFn p_Var6;
  undefined1 in_stack_fffffe9c [56];
  DataTableRecvVarProxyFn_conflict1 p_Var7;
  RecvProp local_10c;
  RecvProp_conflict1 local_d0;
  RecvProp_conflict1 local_94;
  RecvProp_conflict1 local_58;
  
                    /* Unresolved local var: char * pRecvTableName@[???]
                       Unresolved local var: RecvTable * RecvTable@[???] */
  ___i686_get_pc_thunk_bx();
  if (*(char *)((int)&BaseModUI::CTransitionScreen::GetMessageMap::s_pMap + unaff_EBX + 3) == '\0')
  {
    iVar1 = ___cxa_guard_acquire(unaff_EBX + 0xc5ac87);
    if (iVar1 != 0) {
      RecvPropInt((RecvProp_conflict1 *)(&DAT_00c5ac9f + unaff_EBX),(char *)(unaff_EBX + 0x91398f),0
                  ,4,0,(RecvVarProxyFn)0x0);
      RecvPropFloat((RecvProp_conflict1 *)(unaff_EBX + 0xc5acdb),(char *)(unaff_EBX + 0x943c8d),4,4,
                    0,*(RecvVarProxyFn *)(&DAT_00aed04b + unaff_EBX));
      pcVar5 = (char *)0x0;
      RecvPropInt(&local_58,(char *)(unaff_EBX + 0x943caa),8,4,0,(RecvVarProxyFn)0x0);
      p_Var7 = *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00aed00b + unaff_EBX);
      pRVar2 = &local_58;
      puVar4 = (undefined4 *)&stack0xfffffe98;
      for (iVar1 = 0xf; iVar1 != 0; iVar1 = iVar1 + -1) {
        *puVar4 = pRVar2->m_pVarName;
        pRVar2 = (RecvProp_conflict1 *)&pRVar2->m_RecvType;
        puVar4 = puVar4 + 1;
      }
      pArrayProp.m_RecvType = in_stack_fffffe9c._0_4_;
      pArrayProp.m_Flags = in_stack_fffffe9c._4_4_;
      pArrayProp.m_StringBufferSize = in_stack_fffffe9c._8_4_;
      pArrayProp.m_bInsideArray = (bool)in_stack_fffffe9c[0xc];
      pArrayProp._17_3_ = in_stack_fffffe9c._13_3_;
      pArrayProp.m_pExtraData = (void *)in_stack_fffffe9c._16_4_;
      pArrayProp.m_pArrayProp = (RecvProp_conflict1 *)in_stack_fffffe9c._20_4_;
      pArrayProp.m_ArrayLengthProxy = (ArrayLengthRecvProxyFn)in_stack_fffffe9c._24_4_;
      pArrayProp.m_ProxyFn = (RecvVarProxyFn)in_stack_fffffe9c._28_4_;
      pArrayProp.m_DataTableProxyFn = (DataTableRecvVarProxyFn_conflict1)in_stack_fffffe9c._32_4_;
      pArrayProp.m_pDataTable = (RecvTable *)in_stack_fffffe9c._36_4_;
      pArrayProp.m_Offset = in_stack_fffffe9c._40_4_;
      pArrayProp.m_ElementStride = in_stack_fffffe9c._44_4_;
      pArrayProp.m_nElements = in_stack_fffffe9c._48_4_;
      pArrayProp.m_pParentArrayPropName = (char *)in_stack_fffffe9c._52_4_;
      pArrayProp.m_pVarName = pcVar5;
      RecvPropArray3((RecvProp_conflict1 *)
                     ((int)&CHudCoopPingIndicator::GetMessageMap::s_pMap + unaff_EBX + 3),
                     (char *)(unaff_EBX + 0x943cc5),8,4,0x10,pArrayProp,p_Var7);
      RecvPropBool((RecvProp *)((int)&CHudIndicator::GetKBMap::s_pMap + unaff_EBX + 3),
                   (char *)(unaff_EBX + 0x943cdd),0x48,1);
      p_Var6 = *(RecvVarProxyFn *)(&DAT_00aed0af + unaff_EBX);
      RecvPropVector(&local_94,(char *)(unaff_EBX + 0x943cef),0x4c,0xc,0,p_Var6);
      p_Var7 = *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00aed00b + unaff_EBX);
      pRVar2 = &local_94;
      puVar4 = (undefined4 *)&stack0xfffffe98;
      for (iVar1 = 0xf; iVar1 != 0; iVar1 = iVar1 + -1) {
        *puVar4 = pRVar2->m_pVarName;
        pRVar2 = (RecvProp_conflict1 *)&pRVar2->m_RecvType;
        puVar4 = puVar4 + 1;
      }
      pArrayProp_00.m_RecvType = in_stack_fffffe9c._0_4_;
      pArrayProp_00.m_Flags = in_stack_fffffe9c._4_4_;
      pArrayProp_00.m_StringBufferSize = in_stack_fffffe9c._8_4_;
      pArrayProp_00.m_bInsideArray = (bool)in_stack_fffffe9c[0xc];
      pArrayProp_00._17_3_ = in_stack_fffffe9c._13_3_;
      pArrayProp_00.m_pExtraData = (void *)in_stack_fffffe9c._16_4_;
      pArrayProp_00.m_pArrayProp = (RecvProp_conflict1 *)in_stack_fffffe9c._20_4_;
      pArrayProp_00.m_ArrayLengthProxy = (ArrayLengthRecvProxyFn)in_stack_fffffe9c._24_4_;
      pArrayProp_00.m_ProxyFn = (RecvVarProxyFn)in_stack_fffffe9c._28_4_;
      pArrayProp_00.m_DataTableProxyFn = (DataTableRecvVarProxyFn_conflict1)in_stack_fffffe9c._32_4_
      ;
      pArrayProp_00.m_pDataTable = (RecvTable *)in_stack_fffffe9c._36_4_;
      pArrayProp_00.m_Offset = in_stack_fffffe9c._40_4_;
      pArrayProp_00.m_ElementStride = in_stack_fffffe9c._44_4_;
      pArrayProp_00.m_nElements = in_stack_fffffe9c._48_4_;
      pArrayProp_00.m_pParentArrayPropName = (char *)in_stack_fffffe9c._52_4_;
      pArrayProp_00.m_pVarName = (char *)p_Var6;
      RecvPropArray3((RecvProp_conflict1 *)(unaff_EBX + 0xc5ad8f),(char *)(unaff_EBX + 0x943d0e),
                     0x4c,0xc,4,pArrayProp_00,p_Var7);
      p_Var6 = *(RecvVarProxyFn *)(&DAT_00aed0af + unaff_EBX);
      RecvPropVector(&local_d0,(char *)(unaff_EBX + 0x943d2a),0x7c,0xc,0,p_Var6);
      p_Var7 = *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00aed00b + unaff_EBX);
      pRVar2 = &local_d0;
      puVar4 = (undefined4 *)&stack0xfffffe98;
      for (iVar1 = 0xf; iVar1 != 0; iVar1 = iVar1 + -1) {
        *puVar4 = pRVar2->m_pVarName;
        pRVar2 = (RecvProp_conflict1 *)&pRVar2->m_RecvType;
        puVar4 = puVar4 + 1;
      }
      pArrayProp_01.m_RecvType = in_stack_fffffe9c._0_4_;
      pArrayProp_01.m_Flags = in_stack_fffffe9c._4_4_;
      pArrayProp_01.m_StringBufferSize = in_stack_fffffe9c._8_4_;
      pArrayProp_01.m_bInsideArray = (bool)in_stack_fffffe9c[0xc];
      pArrayProp_01._17_3_ = in_stack_fffffe9c._13_3_;
      pArrayProp_01.m_pExtraData = (void *)in_stack_fffffe9c._16_4_;
      pArrayProp_01.m_pArrayProp = (RecvProp_conflict1 *)in_stack_fffffe9c._20_4_;
      pArrayProp_01.m_ArrayLengthProxy = (ArrayLengthRecvProxyFn)in_stack_fffffe9c._24_4_;
      pArrayProp_01.m_ProxyFn = (RecvVarProxyFn)in_stack_fffffe9c._28_4_;
      pArrayProp_01.m_DataTableProxyFn = (DataTableRecvVarProxyFn_conflict1)in_stack_fffffe9c._32_4_
      ;
      pArrayProp_01.m_pDataTable = (RecvTable *)in_stack_fffffe9c._36_4_;
      pArrayProp_01.m_Offset = in_stack_fffffe9c._40_4_;
      pArrayProp_01.m_ElementStride = in_stack_fffffe9c._44_4_;
      pArrayProp_01.m_nElements = in_stack_fffffe9c._48_4_;
      pArrayProp_01.m_pParentArrayPropName = (char *)in_stack_fffffe9c._52_4_;
      pArrayProp_01.m_pVarName = (char *)p_Var6;
      RecvPropArray3((RecvProp_conflict1 *)(unaff_EBX + 0xc5adcb),(char *)(unaff_EBX + 0x943d47),
                     0x7c,0xc,4,pArrayProp_01,p_Var7);
      RecvPropBool(&local_10c,(char *)(unaff_EBX + 0x943d63),0xac,1);
      p_Var7 = *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00aed00b + unaff_EBX);
      pRVar3 = &local_10c;
      puVar4 = (undefined4 *)&stack0xfffffe98;
      for (iVar1 = 0xf; iVar1 != 0; iVar1 = iVar1 + -1) {
        *puVar4 = pRVar3->m_pVarName;
        pRVar3 = (RecvProp *)&pRVar3->m_RecvType;
        puVar4 = puVar4 + 1;
      }
      pArrayProp_02.m_RecvType = in_stack_fffffe9c._0_4_;
      pArrayProp_02.m_Flags = in_stack_fffffe9c._4_4_;
      pArrayProp_02.m_StringBufferSize = in_stack_fffffe9c._8_4_;
      pArrayProp_02.m_bInsideArray = (bool)in_stack_fffffe9c[0xc];
      pArrayProp_02._17_3_ = in_stack_fffffe9c._13_3_;
      pArrayProp_02.m_pExtraData = (void *)in_stack_fffffe9c._16_4_;
      pArrayProp_02.m_pArrayProp = (RecvProp_conflict1 *)in_stack_fffffe9c._20_4_;
      pArrayProp_02.m_ArrayLengthProxy = (ArrayLengthRecvProxyFn)in_stack_fffffe9c._24_4_;
      pArrayProp_02.m_ProxyFn = (RecvVarProxyFn)in_stack_fffffe9c._28_4_;
      pArrayProp_02.m_DataTableProxyFn = (DataTableRecvVarProxyFn_conflict1)in_stack_fffffe9c._32_4_
      ;
      pArrayProp_02.m_pDataTable = (RecvTable *)in_stack_fffffe9c._36_4_;
      pArrayProp_02.m_Offset = in_stack_fffffe9c._40_4_;
      pArrayProp_02.m_ElementStride = in_stack_fffffe9c._44_4_;
      pArrayProp_02.m_nElements = in_stack_fffffe9c._48_4_;
      pArrayProp_02.m_pParentArrayPropName = (char *)in_stack_fffffe9c._52_4_;
      pArrayProp_02.m_pVarName = (char *)p_Var6;
      RecvPropArray3((RecvProp_conflict1 *)(unaff_EBX + 0xc5ae07),(char *)(unaff_EBX + 0x943d82),
                     0xac,1,4,pArrayProp_02,p_Var7);
      RecvPropVector((RecvProp_conflict1 *)(&DAT_00c5ae43 + unaff_EBX),
                     (char *)(unaff_EBX + 0x943d9e),0xb0,0xc,0,
                     *(RecvVarProxyFn *)(&DAT_00aed0af + unaff_EBX));
      RecvPropVector((RecvProp_conflict1 *)(unaff_EBX + 0xc5ae7f),(char *)(unaff_EBX + 0x943dac),
                     0xbc,0xc,0,*(RecvVarProxyFn *)(&DAT_00aed0af + unaff_EBX));
      RecvPropVector((RecvProp_conflict1 *)(unaff_EBX + 0xc5aebb),&UNK_00943dbd + unaff_EBX,200,0xc,
                     0,*(RecvVarProxyFn *)(&DAT_00aed0af + unaff_EBX));
      RecvPropVector((RecvProp_conflict1 *)(unaff_EBX + 0xc5aef7),(char *)(unaff_EBX + 0x943dd2),
                     0xd4,0xc,0,*(RecvVarProxyFn *)(&DAT_00aed0af + unaff_EBX));
      RecvPropVector((RecvProp_conflict1 *)((int)&CSurveyQuestion::GetKBMap::s_pMap + unaff_EBX + 3)
                     ,(char *)(unaff_EBX + 0x943dd7),0xe0,0xc,0,
                     *(RecvVarProxyFn *)(&DAT_00aed0af + unaff_EBX));
      RecvPropVector((RecvProp_conflict1 *)(unaff_EBX + 0xc5af6f),(char *)(unaff_EBX + 0x943dec),
                     0xec,0xc,0,*(RecvVarProxyFn *)(&DAT_00aed0af + unaff_EBX));
      RecvPropVector((RecvProp_conflict1 *)(unaff_EBX + 0xc5afab),(char *)(unaff_EBX + 0x943dfb),
                     0xf8,0xc,0,*(RecvVarProxyFn *)(&DAT_00aed0af + unaff_EBX));
      RecvPropVector((RecvProp_conflict1 *)(unaff_EBX + 0xc5afe7),(char *)(unaff_EBX + 0x943e0a),
                     0x104,0xc,0,*(RecvVarProxyFn *)(&DAT_00aed0af + unaff_EBX));
      RecvPropVector((RecvProp_conflict1 *)(unaff_EBX + 0xc5b023),(char *)(unaff_EBX + 0x943e18),
                     0x110,0xc,0,*(RecvVarProxyFn *)(&DAT_00aed0af + unaff_EBX));
      RecvPropVector((RecvProp_conflict1 *)(&DAT_00c5b05f + unaff_EBX),
                     (char *)(unaff_EBX + 0x943e26),0x11c,0xc,0,
                     *(RecvVarProxyFn *)(&DAT_00aed0af + unaff_EBX));
      RecvPropVector((RecvProp_conflict1 *)(unaff_EBX + 0xc5b09b),(char *)(unaff_EBX + 0x943e42),
                     0x128,0xc,0,*(RecvVarProxyFn *)(&DAT_00aed0af + unaff_EBX));
      RecvPropVector((RecvProp_conflict1 *)(unaff_EBX + 0xc5b0d7),(char *)(unaff_EBX + 0x943e5e),
                     0x134,0xc,0,*(RecvVarProxyFn *)(&DAT_00aed0af + unaff_EBX));
      RecvPropVector((RecvProp_conflict1 *)((int)&HUDVideoPanel::GetKBMap::s_pMap + unaff_EBX + 3),
                     &UNK_00943e79 + unaff_EBX,0x140,0xc,0,
                     *(RecvVarProxyFn *)(&DAT_00aed0af + unaff_EBX));
      RecvPropVector((RecvProp_conflict1 *)(unaff_EBX + 0xc5b14f),(char *)(unaff_EBX + 0x943e94),
                     0x14c,0xc,0,*(RecvVarProxyFn *)(&DAT_00aed0af + unaff_EBX));
      RecvPropVector((RecvProp_conflict1 *)((int)&g_flDOFFarBlurRadius + unaff_EBX + 3),
                     (char *)(unaff_EBX + 0x943e9f),0x158,0xc,0,
                     *(RecvVarProxyFn *)(&DAT_00aed0af + unaff_EBX));
      RecvPropVector((RecvProp_conflict1 *)
                     ((int)&CEnginePostMaterialProxy::s_PostBloomEnable + unaff_EBX + 3),
                     (char *)(unaff_EBX + 0x943eac),0x164,0xc,0,
                     *(RecvVarProxyFn *)(&DAT_00aed0af + unaff_EBX));
      RecvPropVector((RecvProp_conflict1 *)(unaff_EBX + 0xc5b203),&UNK_00943ebb + unaff_EBX,0x170,
                     0xc,0,*(RecvVarProxyFn *)(&DAT_00aed0af + unaff_EBX));
      RecvPropInt((RecvProp_conflict1 *)(unaff_EBX + 0xc5b23f),(char *)(unaff_EBX + 0x943ece),0x17c,
                  4,0,(RecvVarProxyFn)0x0);
      RecvPropDataTable((RecvProp_conflict1 *)
                        ((int)&Blobulator::LightBucketImplementation::LightweightRenderer::
                               LightweightRenderer(Blobulator::LightBucketImplementation::CFragmentedVertexBuffer&,bool)
                               ::sFirstTime + unaff_EBX + 1),(char *)(unaff_EBX + 0x943ee1),0x180,0,
                        *(RecvTable **)(&DAT_00aed7e3 + unaff_EBX),
                        *(DataTableRecvVarProxyFn_conflict1 *)(&DAT_00aed00b + unaff_EBX));
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&s_DefaultSheetSequence.m_TextureCoordData[0].m_fTop_V1 + unaff_EBX + 3),
                    (char *)(unaff_EBX + 0x943ef5),400,4,0,
                    *(RecvVarProxyFn *)(&DAT_00aed04b + unaff_EBX));
      RecvPropFloat((RecvProp_conflict1 *)
                    ((int)&CUtlRBTree<CChoreoEvent*,unsigned_short,bool(*)(CChoreoEvent*const&,CChoreoEvent*const&),CUtlMemory<UtlRBTreeNode_t<CChoreoEvent*,unsigned_short>,unsigned_short>>
                           ::Links(unsigned_short)::s_Sentinel + unaff_EBX + 3),
                    (char *)(unaff_EBX + 0x943f0a),0x18c,4,0,
                    *(RecvVarProxyFn *)(&DAT_00aed04b + unaff_EBX));
      RecvPropInt((RecvProp_conflict1 *)(unaff_EBX + 0xc5b32f),(char *)(unaff_EBX + 0x943f1c),0x194,
                  4,0,(RecvVarProxyFn)0x0);
      RecvPropInt((RecvProp_conflict1 *)
                  ((int)&CDmxAttribute::GetValue<CUtlString>()::defaultValue + unaff_EBX + 3),
                  (char *)(unaff_EBX + 0x943f30),0x198,4,0,(RecvVarProxyFn)0x0);
      RecvPropBool((RecvProp *)(&DAT_00c5b3a7 + unaff_EBX),&UNK_00943f3d + unaff_EBX,0x19c,1);
      RecvPropBool((RecvProp *)((int)&g_pProcessUtils + unaff_EBX + 3),
                   (char *)(unaff_EBX + 0x943f50),0x19d,1);
      RecvPropBool((RecvProp *)((int)&g_pInputStackSystem + unaff_EBX + 3),
                   (char *)(unaff_EBX + 0x943f64),0x19f,1);
      RecvPropBool((RecvProp *)((int)&g_pRenderDevice + unaff_EBX + 3),
                   (char *)(unaff_EBX + 0x943f77),0x1a0,1);
      RecvPropBool((RecvProp *)((int)&mdlcache + unaff_EBX + 3),(char *)(unaff_EBX + 0x943f86),0x1a1
                   ,1);
      RecvPropBool((RecvProp *)((int)power2_n + unaff_EBX + 0x13),(char *)(unaff_EBX + 0x943f91),
                   0x1a2,1);
      ___cxa_guard_release(unaff_EBX + 0xc5ac87);
    }
  }
  RecvTable::Construct
            ((RecvTable *)((int)&s_C_OP_DistanceBetweenCPsFactory.m_Id + unaff_EBX + 3),
             (RecvProp_conflict1 *)(unaff_EBX + 0xc5acdb),0x23,(char *)(unaff_EBX + 0x943fa2));
  return 1;
}


/* __static_initialization_and_destruction_0 at 00077890 */

/* WARNING: Enum "DmAttributeType_t": Some values do not have unique names */
/* WARNING: Enum "BitfieldType_t": Some values do not have unique names */

void __static_initialization_and_destruction_0(int __initialize_p,int __priority)

{
  int iVar1;
  int unaff_EBX;
  longlong lVar2;
  
  lVar2 = ___i686_get_pc_thunk_bx();
  if (lVar2 != 0xffff00000001) {
    return;
  }
  (&DAT_00c5a17f)[unaff_EBX] = 0;
  *(undefined1 *)((int)&COptionsSubKeyboardAdvancedDlg::GetMessageMap()::s_pMap + unaff_EBX) = 0;
  *(undefined1 *)((int)&COptionsSubKeyboardAdvancedDlg::GetMessageMap()::s_pMap + unaff_EBX + 1) = 0
  ;
  *(undefined1 *)((int)&COptionsSubKeyboardAdvancedDlg::GetMessageMap()::s_pMap + unaff_EBX + 2) = 0
  ;
  *(undefined4 *)((int)&COptionsSubKeyboardAdvancedDlg::GetMessageMap()::s_pMap + unaff_EBX + 3) = 0
  ;
  *(undefined4 *)(&DAT_00c5a187 + unaff_EBX) = 0;
  *(undefined4 *)((int)&COptionsSubKeyboardAdvancedDlg::GetKBMap()::s_pMap + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)(&DAT_00c5a18f + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)((int)&COptionsSubKeyboard::GetKBMap::s_pMap + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&COptionsSubKeyboard::GetMessageMap::s_pMap + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&COptionsSubKeyboardAdvancedDlg::GetKBMap::s_pMap + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&COptionsSubKeyboardAdvancedDlg::GetMessageMap::s_pMap + unaff_EBX + 3) = 0;
  *(undefined4 *)(&COptionsSubKeyboard::ChainToMap()::chained + unaff_EBX) = 0x7f7fffff;
  *(undefined4 *)
   ((int)&COptionsSubKeyboard::PanelMessageFunc_ItemSelected::InitVar()::bAdded + unaff_EBX + 1) =
       0x7f7fffff;
  *(undefined4 *)((int)&COptionsSubMouse::GetKBMap::s_pMap + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined4 *)((int)&COptionsSubMouse::GetMessageMap::s_pMap + unaff_EBX + 3) = 0x7f7fffff;
  *(undefined **)
   (&COptionsSubMouse::PanelMessageFunc_OnControlModified::InitVar()::bAdded + unaff_EBX) =
       &UNK_00aed5e7 + unaff_EBX;
  RecvTable::RecvTable
            ((RecvTable *)((int)&C_OP_WorldTraceConstraint_UnpackInit::s_pUnpack + unaff_EBX + 3));
  ___cxa_atexit(unaff_EBX + 0x90467f,0,*(undefined4 *)(&DAT_00aec4cb + unaff_EBX));
  iVar1 = ClientClassInit<DT_PortalLocal::ignored>((ignored *)0x0);
  *(int *)((int)&C_OP_BoxConstraint_UnpackInit::s_pUnpack + unaff_EBX + 3) = iVar1;
  *(undefined4 *)(&DAT_00bb48c3 + unaff_EBX) = 0x1b;
  *(int *)(&DAT_00bb48bf + unaff_EBX) = unaff_EBX + 0xbb491f;
  *(undefined **)(&DAT_00cf188f + unaff_EBX) = &DAT_00bb48bf + unaff_EBX;
  *(undefined4 *)
   ((int)&COptionsSubMouse::PanelMessageFunc_OnCheckButtonChecked::InitVar()::bAdded + unaff_EBX + 2
   ) = 0;
  *(undefined4 *)((int)&vgui::ImagePanel::GetKBMap::s_pMap + unaff_EBX + 3) = 0;
  *(undefined4 *)((int)&vgui::ImagePanel::GetMessageMap::s_pMap + unaff_EBX + 3) = 0x3f800000;
  return;
}


/* __tcf_0 at 0097bf20 */

void __tcf_0(void *param_1)

{
  int extraout_ECX;
  int in_stack_00000008;
  
  ___i686_get_pc_thunk_cx();
  RecvTable::~RecvTable((RecvTable *)(extraout_ECX + 0x3ed1f0),in_stack_00000008);
  return;
}


/* C_PortalPlayerLocalData::GetPredDescMap at 0052d290 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: datamap_t * GetPredDescMap(C_PortalPlayerLocalData * this) */

datamap_t * __thiscall C_PortalPlayerLocalData::GetPredDescMap(C_PortalPlayerLocalData *this)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  return (datamap_t *)(extraout_ECX + 0x6feec8);
}


/* PredMapInit<C_PortalPlayerLocalData> at 0052d2a0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */

datamap_t * PredMapInit<C_PortalPlayerLocalData>(C_PortalPlayerLocalData *param_1)

{
  int extraout_ECX;
  
  ___i686_get_pc_thunk_cx();
  *(undefined4 *)(extraout_ECX + 0x6feeb9) = 0x1b;
  *(int *)(extraout_ECX + 0x6feeb5) = extraout_ECX + 0x6fef15;
  return (datamap_t *)(extraout_ECX + 0x6feeb5);
}


/* C_PortalPlayerLocalData::C_PortalPlayerLocalData at 0052d2d0 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* WARNING: Struct "CNewParticleEffect": ignoring overlapping field "super_CDefaultClientRenderable"
    */
/* DWARF original prototype: void C_PortalPlayerLocalData(C_PortalPlayerLocalData * this) */

void __thiscall C_PortalPlayerLocalData::C_PortalPlayerLocalData(C_PortalPlayerLocalData *this)

{
  vec_t *pvVar1;
  int iVar2;
  int unaff_EBX;
  CountdownTimer *pThis;
  
  ___i686_get_pc_thunk_bx();
  this->_vptr_C_PortalPlayerLocalData = (_func_int_varargs **)(unaff_EBX + 0x69ea42);
  (this->m_StickNormal).x = *(vec_t *)(unaff_EBX + 0x7a477a);
  (this->m_StickNormal).y = *(vec_t *)(unaff_EBX + 0x7a477e);
  (this->m_StickNormal).z = *(vec_t *)(unaff_EBX + 0x7a4782);
  (this->m_OldStickNormal).x = *(vec_t *)(unaff_EBX + 0x7a477a);
  (this->m_OldStickNormal).y = *(vec_t *)(unaff_EBX + 0x7a477e);
  (this->m_OldStickNormal).z = *(vec_t *)(unaff_EBX + 0x7a4782);
  pvVar1 = *(vec_t **)(C_ServerRagdollAttached::GetClientClass + unaff_EBX + 2);
  (this->m_vPreUpdateVelocity).x = *pvVar1;
  (this->m_vPreUpdateVelocity).y = pvVar1[1];
  (this->m_vPreUpdateVelocity).z = pvVar1[2];
  (this->m_Up).x = *(vec_t *)(unaff_EBX + 0x7a477a);
  (this->m_Up).y = *(vec_t *)(unaff_EBX + 0x7a477e);
  (this->m_Up).z = *(vec_t *)(unaff_EBX + 0x7a4782);
  (this->m_vStickRotationAxis).x = 0.0;
  (this->m_vStickRotationAxis).y = 0.0;
  (this->m_vStickRotationAxis).z = 0.0;
  iVar2 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x636ada) + 0x7c))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x636ada));
  (this->m_StandHullMin).x = *(vec_t *)(iVar2 + 0xc);
  (this->m_StandHullMin).y = *(vec_t *)(iVar2 + 0x10);
  (this->m_StandHullMin).z = *(vec_t *)(iVar2 + 0x14);
  iVar2 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x636ada) + 0x7c))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x636ada));
  (this->m_StandHullMax).x = *(vec_t *)(iVar2 + 0x18);
  (this->m_StandHullMax).y = *(vec_t *)(iVar2 + 0x1c);
  (this->m_StandHullMax).z = *(vec_t *)(iVar2 + 0x20);
  iVar2 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x636ada) + 0x7c))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x636ada));
  (this->m_DuckHullMin).x = *(vec_t *)(iVar2 + 0x24);
  (this->m_DuckHullMin).y = *(vec_t *)(iVar2 + 0x28);
  (this->m_DuckHullMin).z = *(vec_t *)(iVar2 + 0x2c);
  iVar2 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x636ada) + 0x7c))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x636ada));
  (this->m_DuckHullMax).x = *(vec_t *)(iVar2 + 0x30);
  (this->m_DuckHullMax).y = *(vec_t *)(iVar2 + 0x34);
  (this->m_DuckHullMax).z = *(vec_t *)(iVar2 + 0x38);
  iVar2 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x636ada) + 0x7c))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x636ada));
  (this->m_CachedStandHullMinAttempt).x = *(vec_t *)(iVar2 + 0xc);
  (this->m_CachedStandHullMinAttempt).y = *(vec_t *)(iVar2 + 0x10);
  (this->m_CachedStandHullMinAttempt).z = *(vec_t *)(iVar2 + 0x14);
  iVar2 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x636ada) + 0x7c))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x636ada));
  (this->m_CachedStandHullMaxAttempt).x = *(vec_t *)(iVar2 + 0x18);
  (this->m_CachedStandHullMaxAttempt).y = *(vec_t *)(iVar2 + 0x1c);
  (this->m_CachedStandHullMaxAttempt).z = *(vec_t *)(iVar2 + 0x20);
  iVar2 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x636ada) + 0x7c))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x636ada));
  (this->m_CachedDuckHullMinAttempt).x = *(vec_t *)(iVar2 + 0x24);
  (this->m_CachedDuckHullMinAttempt).y = *(vec_t *)(iVar2 + 0x28);
  (this->m_CachedDuckHullMinAttempt).z = *(vec_t *)(iVar2 + 0x2c);
  iVar2 = (**(code **)(*(int *)**(undefined4 **)(unaff_EBX + 0x636ada) + 0x7c))
                    ((int *)**(undefined4 **)(unaff_EBX + 0x636ada));
  (this->m_CachedDuckHullMaxAttempt).x = *(vec_t *)(iVar2 + 0x30);
  (this->m_CachedDuckHullMaxAttempt).y = *(vec_t *)(iVar2 + 0x34);
  (this->m_CachedDuckHullMaxAttempt).z = *(vec_t *)(iVar2 + 0x38);
  (this->m_vLocalUp).x = 0.0;
  (this->m_vLocalUp).y = 0.0;
  (this->m_vLocalUp).z = 1.0;
  (this->m_vEyeOffset).x = 0.0;
  (this->m_vEyeOffset).y = 0.0;
  (this->m_vEyeOffset).z = 0.0;
  (this->m_vEyeUpOffset).x = 0.0;
  (this->m_vEyeUpOffset).y = 0.0;
  (this->m_vEyeUpOffset).z = 0.0;
  (this->m_qQuaternionPunch).x = 0.0;
  (this->m_qQuaternionPunch).y = 0.0;
  (this->m_qQuaternionPunch).z = 0.0;
  this->m_PaintedPowerType = NO_POWER;
  iVar2 = *(int *)(unaff_EBX + 0x636e02);
  (this->m_PaintedPowerTimer)._vptr_CountdownTimer = (_func_int_varargs **)(iVar2 + 8);
  (this->m_PaintedPowerTimer).m_duration.m_Value = 0.0;
  (**(code **)(iVar2 + 0x10))(&this->m_PaintedPowerTimer,&(this->m_PaintedPowerTimer).m_duration);
  (this->m_PaintedPowerTimer).m_timestamp.m_Value = -1.0;
                    /* Unresolved local var: CountdownTimer * pThis@[???] */
  (*(this->m_PaintedPowerTimer)._vptr_CountdownTimer[2])
            (&this->m_PaintedPowerTimer,&(this->m_PaintedPowerTimer).m_timestamp);
  this->m_flAirInputScale = 1.0;
  this->m_flCurrentStickTime = *(float *)(*(int *)(*(int *)(unaff_EBX + 0x6372ea) + 0x1c) + 0x2c);
  this->m_nStickCameraState = STICK_CAMERA_UPRIGHT;
  this->m_InAirState = ON_GROUND;
  this->m_bDoneStickInterp = true;
  this->m_bDoneCorrectPitch = true;
  this->m_bAttemptHullResize = false;
  this->m_bJumpedThisFrame = false;
  this->m_bDuckedInAir = false;
  this->m_bBounced = false;
  this->m_bInTractorBeam = false;
  this->m_flAirControlSupressionTime = 0.0;
  _V_memset(this->m_nLocatorEntityIndices,-1,0x10);
  this->m_bPlacingPhoto = false;
                    /* Unresolved local var: uint i@[???] */
  this->m_PaintPowerHudInfoPosition[0].x = *pvVar1;
  this->m_PaintPowerHudInfoPosition[0].y = pvVar1[1];
  this->m_PaintPowerHudInfoPosition[0].z = pvVar1[2];
  this->m_PaintPowerHudInfoNormal[0].x = *pvVar1;
  this->m_PaintPowerHudInfoNormal[0].y = pvVar1[1];
  this->m_PaintPowerHudInfoNormal[0].z = pvVar1[2];
  this->m_PaintPowerHudInfoValidity[0] = false;
  this->m_PaintPowerHudInfoPosition[1].x = *pvVar1;
  this->m_PaintPowerHudInfoPosition[1].y = pvVar1[1];
  this->m_PaintPowerHudInfoPosition[1].z = pvVar1[2];
  this->m_PaintPowerHudInfoNormal[1].x = *pvVar1;
  this->m_PaintPowerHudInfoNormal[1].y = pvVar1[1];
  this->m_PaintPowerHudInfoNormal[1].z = pvVar1[2];
  this->m_PaintPowerHudInfoValidity[1] = false;
  this->m_PaintPowerHudInfoPosition[2].x = *pvVar1;
  this->m_PaintPowerHudInfoPosition[2].y = pvVar1[1];
  this->m_PaintPowerHudInfoPosition[2].z = pvVar1[2];
  this->m_PaintPowerHudInfoNormal[2].x = *pvVar1;
  this->m_PaintPowerHudInfoNormal[2].y = pvVar1[1];
  this->m_PaintPowerHudInfoNormal[2].z = pvVar1[2];
  this->m_PaintPowerHudInfoValidity[2] = false;
  this->m_PaintPowerHudInfoPosition[3].x = *pvVar1;
  this->m_PaintPowerHudInfoPosition[3].y = pvVar1[1];
  this->m_PaintPowerHudInfoPosition[3].z = pvVar1[2];
  this->m_PaintPowerHudInfoNormal[3].x = *pvVar1;
  this->m_PaintPowerHudInfoNormal[3].y = pvVar1[1];
  this->m_PaintPowerHudInfoNormal[3].z = pvVar1[2];
  this->m_PaintPowerHudInfoValidity[3] = false;
  return;
}


/* C_PortalPlayerLocalData::C_PortalPlayerLocalData at 0052d770 */

/* WARNING: Enum "PaintPowerType": Some values do not have unique names */
/* DWARF original prototype: void C_PortalPlayerLocalData(C_PortalPlayerLocalData * this,
   C_PortalPlayerLocalData * this) */

void __thiscall
C_PortalPlayerLocalData::C_PortalPlayerLocalData
          (C_PortalPlayerLocalData *this,C_PortalPlayerLocalData *this_1)

{
  C_PortalPlayerLocalData(this);
  return;
}


/* _GLOBAL__I__ZN14DT_PortalLocal11g_RecvTableE at 000779b0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN14DT_PortalLocal11g_RecvTableE(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

