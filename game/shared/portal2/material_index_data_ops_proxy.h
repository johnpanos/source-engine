//========= Portal 2 reconstruction ============================================//
//
// Purpose: Save a physics surface by name so its index can be resolved after
//          the surface properties manifest is loaded on restore.
//
//=============================================================================//

#ifndef MATERIAL_INDEX_DATA_OPS_PROXY_H
#define MATERIAL_INDEX_DATA_OPS_PROXY_H

class ISaveRestoreOps;

ISaveRestoreOps &GetMaterialIndexDataOpsProxy();

#endif // MATERIAL_INDEX_DATA_OPS_PROXY_H
