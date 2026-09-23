/* DWARF-guided pseudocode for game/client/portal2/polygonbutton.cpp.
 * This is not the original C++ source or a buildable translation.
 * Binary: client.dylib.
 */

/* CPolygonButton::CPolygonButton at 00607f90 */

/* WARNING: Struct "CPolygonButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void CPolygonButton(CPolygonButton * this, Panel * parent, char *
   panelName) */

void __thiscall CPolygonButton::CPolygonButton(CPolygonButton *this,Panel *parent,char *panelName)

{
  char cVar1;
  undefined4 *puVar2;
  int *piVar3;
  int iVar4;
  PanelMessageMap *pPVar5;
  PanelMessageMap *pPVar6;
  PanelAnimationMap *pPVar7;
  PanelAnimationMap *pPVar8;
  PanelKeyBindingMap *pPVar9;
  PanelKeyBindingMap *pPVar10;
  int unaff_EBX;
  
  ___i686_get_pc_thunk_bx();
  vgui::Button::Button
            (&this->super_Button,parent,panelName,(wchar_t *)(unaff_EBX + 0x3b4b66),(Panel *)0x0,
             (char *)0x0);
  (this->super_Button).super_Label.super_Panel.super_IClientPanel._vptr_IClientPanel =
       (_func_int_varargs **)(unaff_EBX + 0x5b5a6a);
  if (*(char *)(unaff_EBX + 0x61ce5a) == '\0') {
    *(undefined1 *)(unaff_EBX + 0x61ce5a) = 1;
    pPVar5 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x3970bc));
    pPVar5->pfnClassName =
         *(_func_char_ptr **)(BaseModUI::SignInDialog::GetPanelClassName + unaff_EBX + 2);
    pPVar6 = vgui::FindOrAddPanelMessageMap((char *)(unaff_EBX + 0x37e02e));
    pPVar5->baseMap = pPVar6;
    cVar1 = *(char *)(unaff_EBX + 0x61ce5b);
  }
  else {
    cVar1 = *(char *)(unaff_EBX + 0x61ce5b);
  }
  if (cVar1 == '\0') {
    *(undefined1 *)(unaff_EBX + 0x61ce5b) = 1;
    pPVar7 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x3970bc));
    pPVar7->pfnClassName =
         *(_func_char_ptr **)(BaseModUI::SignInDialog::GetPanelClassName + unaff_EBX + 2);
    pPVar8 = FindOrAddPanelAnimationMap((char *)(unaff_EBX + 0x37e02e));
    pPVar7->baseMap = pPVar8;
    cVar1 = *(char *)(unaff_EBX + 0x61ce5c);
  }
  else {
    cVar1 = *(char *)(unaff_EBX + 0x61ce5c);
  }
  if (cVar1 == '\0') {
    *(undefined1 *)(unaff_EBX + 0x61ce5c) = 1;
    pPVar9 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x3970bc));
    pPVar9->pfnClassName =
         *(_func_char_ptr **)(BaseModUI::SignInDialog::GetPanelClassName + unaff_EBX + 2);
    pPVar10 = vgui::FindOrAddPanelKeyBindingMap((char *)(unaff_EBX + 0x37e02e));
    pPVar9->baseMap = pPVar10;
  }
  (this->m_unscaledHotspotPoints).m_Memory.m_pMemory = (Vector2D *)0x0;
  (this->m_unscaledHotspotPoints).m_Memory.m_nAllocationCount = 0;
  (this->m_unscaledHotspotPoints).m_Memory.m_nGrowSize = 0;
  (this->m_unscaledHotspotPoints).m_pElements = (Vector2D *)0x0;
  (this->m_unscaledVisibleHotspotPoints).m_Memory.m_pMemory = (Vector2D *)0x0;
  (this->m_unscaledVisibleHotspotPoints).m_Memory.m_nAllocationCount = 0;
  (this->m_unscaledVisibleHotspotPoints).m_Memory.m_nGrowSize = 0;
  (this->m_unscaledVisibleHotspotPoints).m_pElements = (Vector2D *)0x0;
  (this->m_unscaledHotspotPoints).m_Size = 0;
  (this->m_unscaledVisibleHotspotPoints).m_Size = 0;
  this->m_hotspotPoints = (Vertex_t *)0x0;
  this->m_visibleHotspotPoints = (Vertex_t *)0x0;
  this->m_numHotspotPoints = 0;
  this->m_numVisibleHotspotPoints = 0;
  puVar2 = *(undefined4 **)(unaff_EBX + 0x529202);
  piVar3 = (int *)*puVar2;
  iVar4 = (**(code **)(*piVar3 + 0x9c))(piVar3,0);
  this->m_nWhiteMaterial = iVar4;
  piVar3 = (int *)*puVar2;
  (**(code **)(*piVar3 + 0x84))(piVar3,iVar4,&UNK_0039b742 + unaff_EBX,1,0);
  return;
}


/* CPolygonButton::CPolygonButton at 006082f0 */

/* WARNING: Struct "CPolygonButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void CPolygonButton(CPolygonButton * this, Panel * parent, char *
   panelName, CPolygonButton * this, Panel * parent, char * panelName) */

void __thiscall
CPolygonButton::CPolygonButton
          (CPolygonButton *this,Panel *parent,char *panelName,CPolygonButton *this_1,Panel *parent_1
          ,char *panelName_1)

{
  CPolygonButton(this,parent,panelName);
  return;
}


/* CPolygonButton::ApplySettings at 006076f0 */

/* WARNING: Struct "CPolygonButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void ApplySettings(CPolygonButton * this, KeyValues * data) */

void __thiscall CPolygonButton::ApplySettings(CPolygonButton *this,KeyValues *data)

{
  vgui::Button::ApplySettings(&this->super_Button,data);
                    /* WARNING: Could not recover jumptable at 0x00607725. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(this->super_Button).super_Label.super_Panel.super_IClientPanel._vptr_IClientPanel[0x11f])();
  return;
}


/* CPolygonButton::UpdateHotspots at 00607be0 */

/* WARNING: Struct "CPolygonButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void UpdateHotspots(CPolygonButton * this, KeyValues * data) */

void __thiscall CPolygonButton::UpdateHotspots(CPolygonButton *this,KeyValues *data)

{
  int *piVar1;
  uint uVar2;
  vec_t vVar3;
  vec_t vVar4;
  KeyValues *pKVar5;
  char *pcVar6;
  int iVar7;
  int iVar8;
  Vector2D *pVVar9;
  int unaff_EBX;
  vec_t local_24;
  vec_t local_20 [4];
  
                    /* Unresolved local var: KeyValues * points@[???] */
  ___i686_get_pc_thunk_bx();
  if (this->m_hotspotPoints != (Vertex_t *)0x0) {
    operator_delete___(this->m_hotspotPoints);
    this->m_hotspotPoints = (Vertex_t *)0x0;
    this->m_numHotspotPoints = 0;
  }
  if (this->m_visibleHotspotPoints != (Vertex_t *)0x0) {
    operator_delete___(this->m_visibleHotspotPoints);
    this->m_visibleHotspotPoints = (Vertex_t *)0x0;
    this->m_numVisibleHotspotPoints = 0;
  }
  (this->m_unscaledHotspotPoints).m_Size = 0;
  (this->m_unscaledVisibleHotspotPoints).m_Size = 0;
  pKVar5 = KeyValues::FindKey(data,(char *)(unaff_EBX + 0x39747b),false);
  if ((pKVar5 != (KeyValues *)0x0) &&
     (pKVar5 = KeyValues::GetFirstValue(pKVar5), pKVar5 != (KeyValues *)0x0)) {
    do {
      while( true ) {
        pcVar6 = KeyValues::GetString(pKVar5,(char *)0x0,(char *)(unaff_EBX + 0x35667e));
        iVar7 = _sscanf(pcVar6,(char *)(unaff_EBX + 0x3a8d02),&local_24,local_20);
        vVar4 = local_20[0];
        vVar3 = local_24;
        if (iVar7 == 2) break;
LAB_00607cc0:
        pKVar5 = KeyValues::GetNextValue(pKVar5);
        if (pKVar5 == (KeyValues *)0x0) goto LAB_00607da0;
      }
      uVar2 = (this->m_unscaledHotspotPoints).m_Size;
      iVar7 = uVar2 + 1;
      iVar8 = (this->m_unscaledHotspotPoints).m_Memory.m_nAllocationCount;
      if (iVar8 < iVar7) {
        CUtlMemory<Vector2D,int>::Grow(&(this->m_unscaledHotspotPoints).m_Memory,iVar7 - iVar8);
      }
      piVar1 = &(this->m_unscaledHotspotPoints).m_Size;
      *piVar1 = *piVar1 + 1;
      pVVar9 = (this->m_unscaledHotspotPoints).m_Memory.m_pMemory;
      (this->m_unscaledHotspotPoints).m_pElements = pVVar9;
      iVar8 = ~uVar2 + (this->m_unscaledHotspotPoints).m_Size;
      if (0 < iVar8) {
        _V_memmove(pVVar9 + iVar7,pVVar9 + uVar2,iVar8 * 8);
      }
      pVVar9 = (this->m_unscaledHotspotPoints).m_Memory.m_pMemory + uVar2;
      if (pVVar9 == (Vector2D *)0x0) goto LAB_00607cc0;
      pVVar9->x = vVar3;
      pVVar9->y = vVar4;
      pKVar5 = KeyValues::GetNextValue(pKVar5);
    } while (pKVar5 != (KeyValues *)0x0);
  }
LAB_00607da0:
  pKVar5 = KeyValues::FindKey(data,(char *)(unaff_EBX + 0x397483),false);
  if (((pKVar5 != (KeyValues *)0x0) ||
      (pKVar5 = KeyValues::FindKey(data,(char *)(unaff_EBX + 0x39747b),false),
      pKVar5 != (KeyValues *)0x0)) &&
     (pKVar5 = KeyValues::GetFirstValue(pKVar5), pKVar5 != (KeyValues *)0x0)) {
    do {
      while( true ) {
        pcVar6 = KeyValues::GetString(pKVar5,(char *)0x0,(char *)(unaff_EBX + 0x35667e));
        iVar7 = _sscanf(pcVar6,(char *)(unaff_EBX + 0x3a8d02),local_20,&local_24);
        vVar4 = local_20[0];
        vVar3 = local_24;
        if (iVar7 == 2) break;
LAB_00607e10:
        pKVar5 = KeyValues::GetNextValue(pKVar5);
        if (pKVar5 == (KeyValues *)0x0) {
          return;
        }
      }
      uVar2 = (this->m_unscaledVisibleHotspotPoints).m_Size;
      iVar7 = uVar2 + 1;
      iVar8 = (this->m_unscaledVisibleHotspotPoints).m_Memory.m_nAllocationCount;
      if (iVar8 < iVar7) {
        CUtlMemory<Vector2D,int>::Grow
                  (&(this->m_unscaledVisibleHotspotPoints).m_Memory,iVar7 - iVar8);
      }
      piVar1 = &(this->m_unscaledVisibleHotspotPoints).m_Size;
      *piVar1 = *piVar1 + 1;
      pVVar9 = (this->m_unscaledVisibleHotspotPoints).m_Memory.m_pMemory;
      (this->m_unscaledVisibleHotspotPoints).m_pElements = pVVar9;
      iVar8 = ~uVar2 + (this->m_unscaledVisibleHotspotPoints).m_Size;
      if (0 < iVar8) {
        _V_memmove(pVVar9 + iVar7,pVVar9 + uVar2,iVar8 * 8);
      }
      pVVar9 = (this->m_unscaledVisibleHotspotPoints).m_Memory.m_pMemory + uVar2;
      if (pVVar9 == (Vector2D *)0x0) goto LAB_00607e10;
      pVVar9->x = vVar4;
      pVVar9->y = vVar3;
      pKVar5 = KeyValues::GetNextValue(pKVar5);
    } while (pKVar5 != (KeyValues *)0x0);
  }
  return;
}


/* CPolygonButton::IsWithinTraverse at 00607840 */

/* WARNING: Restarted to delay deadcode elimination for space: stack */
/* WARNING: Struct "CPolygonButton": ignoring multiple overlapping fields */
/* DWARF original prototype: VPANEL IsWithinTraverse(CPolygonButton * this, int x, int y, bool
   traversePopups) */

VPANEL __thiscall
CPolygonButton::IsWithinTraverse(CPolygonButton *this,int x,int y,bool traversePopups)

{
  bool bVar1;
  int iVar2;
  Vertex_t *pVVar3;
  VPANEL VVar4;
  Vertex_t *pVVar5;
  int iVar6;
  float *pfVar7;
  int iVar8;
  VPANEL VVar9;
  int local_24;
  int local_20 [4];
  
                    /* Unresolved local var: VPANEL within@[???] */
  VVar9 = 0;
  if (2 < this->m_numHotspotPoints) {
    VVar9 = vgui::Panel::IsWithinTraverse((Panel *)this,x,y,traversePopups);
    VVar4 = (**(this->super_Button).super_Label.super_Panel.super_IClientPanel._vptr_IClientPanel)
                      (this);
    if (VVar9 == VVar4) {
                    /* Unresolved local var: int wide@[???]
                       Unresolved local var: int tall@[???]
                       Unresolved local var: bool inside@[???] */
      vgui::Panel::GetSize((Panel *)this,local_20,&local_24);
      vgui::Panel::ScreenToLocal((Panel *)this,&x,&y);
                    /* Unresolved local var: int i@[???] */
      iVar2 = this->m_numHotspotPoints;
      if (0 < iVar2) {
        pVVar3 = this->m_hotspotPoints;
        bVar1 = true;
        iVar6 = 0;
        iVar8 = 0;
        do {
          pVVar5 = pVVar3 + iVar2 + -1;
          if (iVar6 != 0) {
            pVVar5 = (Vertex_t *)((int)&pVVar3[-1].m_Position.x + iVar8);
          }
          pfVar7 = (float *)((int)&(pVVar3->m_Position).x + iVar8);
                    /* Unresolved local var: Vector res@[???] */
          bVar1 = (bool)(bVar1 & 0.0 <= ((pVVar5->m_Position).x - (float)x) * (pfVar7[1] - (float)y)
                                        - ((pVVar5->m_Position).y - (float)y) * (*pfVar7 - (float)x)
                        );
          iVar6 = iVar6 + 1;
          iVar8 = iVar8 + 0x10;
        } while (iVar6 != iVar2);
        if (!bVar1) {
          VVar9 = 0;
        }
      }
    }
  }
  return VVar9;
}


/* CPolygonButton::PerformLayout at 006079d0 */

/* WARNING: Struct "CPolygonButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void PerformLayout(CPolygonButton * this) */

void __thiscall CPolygonButton::PerformLayout(CPolygonButton *this)

{
  float fVar1;
  Vertex_t *pVVar2;
  Vector2D *pVVar3;
  int iVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  int local_24;
  int local_20 [4];
  
                    /* Unresolved local var: int wide@[???]
                       Unresolved local var: int tall@[???] */
  vgui::Panel::GetSize((Panel *)this,local_20,&local_24);
  if (this->m_hotspotPoints != (Vertex_t *)0x0) {
    operator_delete___(this->m_hotspotPoints);
    this->m_hotspotPoints = (Vertex_t *)0x0;
    this->m_numHotspotPoints = 0;
  }
  if (this->m_visibleHotspotPoints != (Vertex_t *)0x0) {
    operator_delete___(this->m_visibleHotspotPoints);
    this->m_visibleHotspotPoints = (Vertex_t *)0x0;
    this->m_numVisibleHotspotPoints = 0;
  }
  iVar4 = (this->m_unscaledHotspotPoints).m_Size;
  this->m_numHotspotPoints = iVar4;
  if (iVar4 != 0) {
    pVVar2 = operator_new___(iVar4 << 4);
    this->m_hotspotPoints = pVVar2;
                    /* Unresolved local var: int i@[???] */
    if (0 < this->m_numHotspotPoints) {
      iVar4 = 0;
      do {
                    /* Unresolved local var: float x@[???]
                       Unresolved local var: float y@[???] */
        pVVar3 = (this->m_unscaledHotspotPoints).m_Memory.m_pMemory + iVar4;
        fVar1 = pVVar3->y;
        pVVar2 = this->m_hotspotPoints + iVar4;
        (pVVar2->m_Position).x = (float)local_20[0] * pVVar3->x;
        (pVVar2->m_Position).y = (float)local_24 * fVar1;
        (pVVar2->m_TexCoord).x = pVVar3->x;
        (pVVar2->m_TexCoord).y = pVVar3->y;
        iVar4 = iVar4 + 1;
      } while (iVar4 < this->m_numHotspotPoints);
    }
  }
  (this->m_hotspotMins).x = (float)local_20[0];
  (this->m_hotspotMins).y = (float)local_24;
  (this->m_hotspotMaxs).x = 0.0;
  (this->m_hotspotMaxs).y = 0.0;
  iVar4 = (this->m_unscaledVisibleHotspotPoints).m_Size;
  this->m_numVisibleHotspotPoints = iVar4;
  if (iVar4 != 0) {
    pVVar2 = operator_new___(iVar4 << 4);
    this->m_visibleHotspotPoints = pVVar2;
                    /* Unresolved local var: int i@[???] */
    if (0 < this->m_numVisibleHotspotPoints) {
      iVar4 = 0;
      do {
                    /* Unresolved local var: float x@[???]
                       Unresolved local var: float y@[???] */
        pVVar3 = (this->m_unscaledVisibleHotspotPoints).m_Memory.m_pMemory + iVar4;
        fVar6 = (float)local_20[0] * pVVar3->x;
        fVar7 = (float)local_24 * pVVar3->y;
        pVVar2 = this->m_visibleHotspotPoints + iVar4;
        (pVVar2->m_Position).x = fVar6;
        (pVVar2->m_Position).y = fVar7;
        (pVVar2->m_TexCoord).x = pVVar3->x;
        (pVVar2->m_TexCoord).y = pVVar3->y;
        fVar1 = (this->m_hotspotMins).x;
        fVar5 = fVar6;
        if (fVar1 <= fVar6) {
          fVar5 = fVar1;
        }
        (this->m_hotspotMins).x = fVar5;
        fVar1 = (this->m_hotspotMins).y;
        fVar5 = fVar7;
        if (fVar1 <= fVar7) {
          fVar5 = fVar1;
        }
        (this->m_hotspotMins).y = fVar5;
        fVar1 = (this->m_hotspotMaxs).x;
        if (fVar6 <= fVar1) {
          fVar6 = fVar1;
        }
        (this->m_hotspotMaxs).x = fVar6;
        fVar1 = (this->m_hotspotMaxs).y;
        if (fVar7 <= fVar1) {
          fVar7 = fVar1;
        }
        (this->m_hotspotMaxs).y = fVar7;
        iVar4 = iVar4 + 1;
      } while (iVar4 < this->m_numVisibleHotspotPoints);
      vgui::Button::PerformLayout(&this->super_Button);
      return;
    }
  }
  vgui::Button::PerformLayout(&this->super_Button);
  return;
}


/* CPolygonButton::ComputeAlignment at 00607610 */

/* WARNING: Struct "CPolygonButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void ComputeAlignment(CPolygonButton * this, int * tx0, int * ty0, int
   * tx1, int * ty1) */

void __thiscall
CPolygonButton::ComputeAlignment(CPolygonButton *this,int *tx0,int *ty0,int *tx1,int *ty1)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int unaff_EBX;
  
                    /* Unresolved local var: Vector2D center@[???]
                       Unresolved local var: int textWide@[???]
                       Unresolved local var: int textTall@[DW_OP_reg1(ECX)] */
  ___i686_get_pc_thunk_bx();
                    /* Unresolved local var: Vector2D res@[???] */
  fVar1 = (this->m_hotspotMins).x;
  fVar2 = (this->m_hotspotMaxs).x;
  fVar3 = *(float *)(unaff_EBX + 0x3af2ac);
  fVar4 = (this->m_hotspotMins).y;
  fVar5 = (this->m_hotspotMaxs).y;
  vgui::Label::ComputeAlignment((Label *)this,tx0,ty0,tx1,ty1);
  iVar6 = *tx1;
  iVar7 = *tx0;
  iVar8 = *ty1;
  iVar9 = *ty0;
  *tx0 = (int)((fVar1 + fVar2) * fVar3 - (float)((iVar6 - iVar7) / 2));
  *ty0 = (int)((fVar4 + fVar5) * fVar3 - (float)((iVar8 - iVar9) / 2));
  *tx1 = (iVar6 - iVar7) + *tx0;
  *ty1 = (iVar8 - iVar9) + *ty0;
  return;
}


/* CPolygonButton::PaintBackground at 006077b0 */

/* WARNING: Struct "CPolygonButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void PaintBackground(CPolygonButton * this) */

void __thiscall CPolygonButton::PaintBackground(CPolygonButton *this)

{
  undefined4 *puVar1;
  int iVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: Color c@[???] */
  ___i686_get_pc_thunk_bx();
  iVar2 = (*(this->super_Button).super_Label.super_Panel.super_IClientPanel._vptr_IClientPanel
            [0x115])(this);
  puVar1 = *(undefined4 **)(unaff_EBX + 0x5299dc);
  (**(code **)(*(int *)*puVar1 + 0x38))((int *)*puVar1,iVar2);
  (**(code **)(*(int *)*puVar1 + 0x8c))((int *)*puVar1,this->m_nWhiteMaterial);
  (**(code **)(*(int *)*puVar1 + 0x198))
            ((int *)*puVar1,this->m_numVisibleHotspotPoints,this->m_visibleHotspotPoints,1);
  return;
}


/* CPolygonButton::PaintBorder at 00607730 */

/* WARNING: Struct "CPolygonButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void PaintBorder(CPolygonButton * this) */

void __thiscall CPolygonButton::PaintBorder(CPolygonButton *this)

{
  undefined4 *puVar1;
  int iVar2;
  int unaff_EBX;
  
                    /* Unresolved local var: Color c@[???] */
  ___i686_get_pc_thunk_bx();
  iVar2 = (*(this->super_Button).super_Label.super_Panel.super_IClientPanel._vptr_IClientPanel
            [0x114])(this);
  puVar1 = *(undefined4 **)(unaff_EBX + 0x529a5c);
  (**(code **)(*(int *)*puVar1 + 0x38))((int *)*puVar1,iVar2);
  (**(code **)(*(int *)*puVar1 + 0x8c))((int *)*puVar1,this->m_nWhiteMaterial);
  (**(code **)(*(int *)*puVar1 + 400))
            ((int *)*puVar1,this->m_visibleHotspotPoints,this->m_numVisibleHotspotPoints);
  return;
}


/* CPolygonButton::ApplySchemeSettings at 006075d0 */

/* WARNING: Struct "CPolygonButton": ignoring multiple overlapping fields */
/* DWARF original prototype: void ApplySchemeSettings(CPolygonButton * this, IScheme * scheme) */

void __thiscall CPolygonButton::ApplySchemeSettings(CPolygonButton *this,IScheme *scheme)

{
  vgui::Button::ApplySchemeSettings(&this->super_Button,scheme);
  (*(this->super_Button).super_Label.super_Panel.super_IClientPanel._vptr_IClientPanel[0x43])
            (this,0,0);
  return;
}


/* __static_initialization_and_destruction_0 at 00082310 */

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
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x31].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x31].m_SerialNumber + unaff_EBX) = 0;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x31].m_SerialNumber + unaff_EBX + 1) = 0
  ;
  *(undefined1 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x31].m_SerialNumber + unaff_EBX + 2) = 0
  ;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x31].m_SerialNumber + unaff_EBX + 3) = 0
  ;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x31].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x31].m_pNext + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x32].m_pEntity + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x32].m_SerialNumber + unaff_EBX + 3) = 0
  ;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x32].m_pPrev + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x32].m_pNext + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x33].m_pEntity + unaff_EBX + 3) = 0;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x33].m_SerialNumber + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x33].m_pPrev + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x33].m_pNext + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined4 *)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x34].m_pEntity + unaff_EBX + 3) =
       0x7f7fffff;
  *(undefined **)
   ((int)&s_EntityList.super_CBaseEntityList.m_EntPtrArray[0x34].m_SerialNumber + unaff_EBX + 3) =
       &UNK_00aafaef + unaff_EBX;
  return;
}


/* _GLOBAL__I__ZN14CPolygonButtonC2EPN4vgui5PanelEPKc at 000823b0 */

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _GLOBAL__I__ZN14CPolygonButtonC2EPN4vgui5PanelEPKc(void)

{
  int in_stack_00000004;
  int in_stack_00000008;
  
  __static_initialization_and_destruction_0(in_stack_00000004,in_stack_00000008);
  return;
}

