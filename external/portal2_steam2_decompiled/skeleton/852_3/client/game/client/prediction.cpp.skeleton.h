// DWARF declaration skeleton for game/client/prediction.cpp
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x83330 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
}

// public/iprediction.h:32 @0x626450 _ZN11IPredictionD0Ev
IPrediction::~IPrediction()
{
}

// public/iprediction.h:32 @0x626470 _ZN11IPredictionD1Ev
IPrediction::~IPrediction()
{
}

// game/client/prediction.cpp:33
void IPredictionSystem::g_pPredictionSystems;

// game/client/prediction.cpp:37
ConVar cl_predictweapons;

// game/client/prediction.cpp:38
ConVar cl_lagcompensation;

// game/client/prediction.cpp:39
ConVar cl_showerror;

// game/client/prediction.cpp:41
static ConVar cl_idealpitchscale;

// game/client/prediction.cpp:42
static ConVar cl_predictionlist;

// game/client/prediction.cpp:44
static ConVar cl_predictionentitydump;

// game/client/prediction.cpp:45
static ConVar cl_predictionentitydumpbyclass;

// game/client/prediction.cpp:46
static ConVar cl_pred_optimize;

// game/client/prediction.cpp:48
static ConVar cl_pred_doresetlatch;

// game/client/prediction.cpp:53 (declaration)
void InvalidateEFlagsRecursive( C_BaseEntity *pEnt, int nDirtyFlags, int nChildFlags );

// game/client/prediction.cpp:53 @0x620d40 _Z25InvalidateEFlagsRecursiveP12C_BaseEntityii
InvalidateEFlagsRecursive( C_BaseEntity *pEnt, int nDirtyFlags, int nChildFlags )
{
	{
		C_BaseEntity *pChild;  // line 57
		// inlined C_BaseEntity::NextMovePeer() at line 57
		// inlined C_BaseEntity::FirstMoveChild() at line 57
		// inlined InvalidateEFlagsRecursive() at line 59
	}
	// inlined C_BaseEntity::AddEFlags() at line 55
}

// game/client/prediction.cpp:74 (declaration)
void CPrediction();

// game/client/prediction.cpp:74 @0x83260 _ZN11CPredictionC2Ev
CPrediction::CPrediction()
{
	// inlined CHandle<C_BaseEntity>::CHandle() at line 74
	// inlined CPrediction::Split_t::Split_t() at line 74
	// inlined CGlobalVarsBase::CGlobalVarsBase() at line 74
	// inlined CUtlVector<const typedescription_t*,CUtlMemory<const typedescription_t*, int> >::CUtlVector() at line 74
}

// game/client/prediction.cpp:74 @0x83320 _ZN11CPredictionC1Ev
CPrediction::CPrediction()
{
}

// game/client/prediction.cpp:90 (declaration)
~CPrediction();

// game/client/prediction.cpp:90 @0x620c40 _ZN11CPredictionD2Ev
CPrediction::~CPrediction()
{
	// inlined CUtlVector<const typedescription_t*,CUtlMemory<const typedescription_t*, int> >::~CUtlVector() at line 92
}

// game/client/prediction.cpp:90 @0x621080 _ZN11CPredictionD0Ev
CPrediction::~CPrediction()
{
	// inlined CUtlVector<const typedescription_t*,CUtlMemory<const typedescription_t*, int> >::~CUtlVector() at line 92
}

// game/client/prediction.cpp:90 @0x621180 _ZN11CPredictionD1Ev
CPrediction::~CPrediction()
{
	// inlined CUtlVector<const typedescription_t*,CUtlMemory<const typedescription_t*, int> >::~CUtlVector() at line 92
}

// game/client/prediction.cpp:94 @0x6202b0 _ZN11CPrediction4InitEv
void CPrediction::Init()
{
}

// game/client/prediction.cpp:102 @0x61f8a0 _ZN11CPrediction8ShutdownEv
void CPrediction::Shutdown()
{
}

// game/client/prediction.cpp:110 @0x6202f0 _ZN11CPrediction10CheckErrorEiP12C_BasePlayeri
void CPrediction::CheckError( int nSlot, C_BasePlayer *player, int commands_acknowledged )
{
	Vector origin;  // line 113
	Vector delta;  // line 114
	float len;  // line 115
	const void *slot;  // line 135
	Vector predicted_origin;  // line 176
	// inlined Vector::operator=() at line 133
	// inlined VectorSubtract() at line 190
	// inlined VectorLength() at line 192
	{
		con_nprint_t np;  // line 206
	}
	{
		const typedescription_t *td;  // line 143
		// inlined CUtlVector<const typedescription_t*,CUtlMemory<const typedescription_t*, int> >::AddToTail() at line 146
	}
	int pos[2];  // line 116
}

// game/client/prediction.cpp:224 @0x6201c0 _ZN11CPrediction20ShutdownPredictablesEv
void CPrediction::ShutdownPredictables()
{
	int shutdown_count;  // line 228
	int release_count;  // line 229
	{
		int nSlot;  // line 231
		{
			int c;  // line 234
			// inlined CPredictableList::GetPredictableCount() at line 234
			{
				int i;  // line 235
				{
					C_BaseEntity *ent;  // line 237
					// inlined CPredictableList::GetPredictable() at line 237
				}
			}
		}
	}
}

// game/client/prediction.cpp:272 @0x620050 _ZN11CPrediction18ReinitPredictablesEv
void CPrediction::ReinitPredictables()
{
	int i;  // line 276
	int c;  // line 277
	// inlined ClientEntityList() at line 277
	{
		C_BaseEntity *e;  // line 280
	}
	{
		int nSlot;  // line 290
	}
}

// game/client/prediction.cpp:301 @0x61f8b0 _ZN11CPrediction28OnReceivedUncompressedPacketEv
void CPrediction::OnReceivedUncompressedPacket()
{
	{
		int i;  // line 305
		{
			CPrediction::Split_t &split;  // line 307
		}
	}
}

// game/client/prediction.cpp:320 @0x625a20 _ZN11CPrediction23PreEntityPacketReceivedEii
void CPrediction::PreEntityPacketReceived( int commands_acknowledged, int current_world_update_packet )
{
	CVProfScope VProf_;  // line 328
	{
		int nSlot;  // line 341
		{
			C_BasePlayer *current;  // line 343
			int c;  // line 349
			int i;  // line 351
			// inlined CPredictableList::GetPredictableCount() at line 349
			{
				C_BaseEntity *ent;  // line 354
				// inlined CPredictableList::GetPredictable() at line 354
			}
		}
	}
	// inlined CVProfScope::CVProfScope() at line 328
	// inlined CVProfScope::~CVProfScope() at line 364
	// inlined CVProfScope::~CVProfScope() at line 364
}

// game/client/prediction.cpp:371 @0x624ee0 _ZN11CPrediction24PostEntityPacketReceivedEv
void CPrediction::PostEntityPacketReceived()
{
	CVProfScope VProf_;  // line 375
	// inlined CVProfScope::CVProfScope() at line 375
	// inlined CVProfScope::~CVProfScope() at line 406
	{
		int nSlot;  // line 382
		{
			C_BasePlayer *current;  // line 384
			int c;  // line 389
			int i;  // line 392
			// inlined CPredictableList::GetPredictableCount() at line 389
			{
				C_BaseEntity *ent;  // line 395
				// inlined CPredictableList::GetPredictable() at line 395
			}
		}
	}
	// inlined CVProfScope::~CVProfScope() at line 406
}

// game/client/prediction.cpp:415 @0x61ff60 _ZN11CPrediction16ShouldDumpEntityEP12C_BaseEntity
bool CPrediction::ShouldDumpEntity( C_BaseEntity *ent )
{
	int dump_entity;  // line 418
	// inlined FClassnameIs() at line 441
	{
		bool dump;  // line 421
	}
	// inlined ConVar::GetInt() at line 418
	// inlined ConVar::GetString() at line 438
	// inlined ConVar::GetString() at line 441
}

// game/client/prediction.cpp:450 @0x61fc70 _ZN11CPrediction23ShowPredictionListEntryEiiP12C_BaseEntityRiS2_
void CPrediction::ShowPredictionListEntry( int listRow, int showlist, C_BaseEntity *ent, int &totalsize, int &totalsize_intermediate )
{
	char sz[32];  // line 452
	int oIndex;  // line 462
	con_nprint_t np;  // line 480
	{
		int size;  // line 489
		int intermediate_size;  // line 490
	}
	// inlined C_BaseEntity::GetOwnerEntity() at line 463
	{
		C_BaseViewModel *pVM;  // line 473
		// inlined ToBaseViewModel() at line 473
	}
}

// game/client/prediction.cpp:513 @0x61fb00 _ZN11CPrediction20FinishPredictionListEiiii
void CPrediction::FinishPredictionList( int listRow, int showlist, int totalsize, int totalsize_intermediate )
{
	{
		con_nprint_t np;  // line 520
		char sz1[32];  // line 527
		char sz2[32];  // line 528
	}
}

// game/client/prediction.cpp:548 (declaration)
void CheckPredictConvar();

// game/client/prediction.cpp:548 @0x620140 _ZN11CPrediction18CheckPredictConvarEv
void CPrediction::CheckPredictConvar()
{
	{
		int i;  // line 557
		{
			CPrediction::Split_t &split;  // line 559
		}
	}
}

// game/client/prediction.cpp:569
ConVar cl_prediction_error_timestamps;

// game/client/prediction.cpp:575 @0x6254d0 _ZN11CPrediction23PostNetworkDataReceivedEi
void CPrediction::PostNetworkDataReceived( int commands_acknowledged )
{
	CVProfScope VProf_;  // line 578
	bool error_check;  // line 580
	bool entityDumped;  // line 591
	bool bPredict;  // line 592
	int showlist;  // line 593
	int listRow;  // line 595
	// inlined CVProfScope::CVProfScope() at line 578
	// inlined ConVar::GetInt() at line 593
	{
		int nSlot;  // line 597
		{
			CPrediction::Split_t &split;  // line 599
			int dumpentindex;  // line 657
			{
				int totalsize;  // line 606
				int totalsize_intermediate;  // line 607
				int c;  // line 610
				int i;  // line 612
				{
					C_BasePlayer *current;  // line 649
				}
				{
					C_BaseEntity *ent;  // line 615
					bool bHadErrors;  // line 622
					// inlined CPredictableList::GetPredictable() at line 615
				}
				// inlined CPredictableList::GetPredictableCount() at line 610
			}
			// inlined ConVar::GetInt() at line 657
			{
				int last_entity;  // line 660
				// inlined ClientEntityList() at line 660
				{
					C_BaseEntity *ent;  // line 663
				}
			}
		}
	}
	// inlined CPrediction::CheckPredictConvar() at line 678
	// inlined CVProfScope::~CVProfScope() at line 683
	// inlined CVProfScope::~CVProfScope() at line 683
}

// game/client/prediction.cpp:695 @0x624a20 _ZN11CPrediction9SetupMoveEP12C_BasePlayerP8CUserCmdP11IMoveHelperP9CMoveData
void CPrediction::SetupMove( C_BasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move )
{
	CVProfScope VProf_;  // line 698
	C_BaseEntity *pMoveParent;  // line 714
	IClientVehicle *pVehicle;  // line 742
	// inlined CVProfScope::CVProfScope() at line 698
	// inlined C_BaseEntity::GetAbsVelocity() at line 703
	// inlined Vector::operator=() at line 703
	// inlined CMoveData::SetAbsOrigin() at line 704
	// inlined QAngle::operator=() at line 705
	// inlined QAngle::operator=() at line 709
	// inlined QAngle::operator=() at line 710
	// inlined C_BaseEntity::GetMoveParent() at line 714
	// inlined QAngle::operator=() at line 717
	// inlined C_BasePlayer::GetVehicle() at line 742
	// inlined CHandle<C_BaseEntity>::operator C_BaseEntity*() at line 749
	// inlined Vector::operator=() at line 752
	// inlined CVProfScope::~CVProfScope() at line 769
	// inlined Vector::operator=() at line 750
	{
		matrix3x4_t viewToParent;  // line 721
		matrix3x4_t viewToWorld;  // line 721
		// inlined C_BaseEntity::EntityToWorldTransform() at line 723
		// inlined MatrixAngles() at line 724
	}
	// inlined CVProfScope::~CVProfScope() at line 769
}

// public/tier1/utlmemory.h:707 @0x626370 _ZN10CUtlMemoryIPK17typedescription_tiE4GrowEi
void CUtlMemory<const typedescription_t*,int>::Grow( int num )
{
	int nAllocationRequested;  // line 720
	int nNewAllocationCount;  // line 724
	// inlined UtlMemory_CalcNewAllocationCount() at line 724
	// inlined CUtlMemory<const typedescription_t*,int>::IsExternallyAllocated() at line 711
	// inlined MemAlloc_Alloc() at line 761
}

// game/client/prediction.cpp:778 @0x624760 _ZN11CPrediction10FinishMoveEP12C_BasePlayerP8CUserCmdP9CMoveData
void CPrediction::FinishMove( C_BasePlayer *player, CUserCmd *ucmd, CMoveData *move )
{
	CVProfScope VProf_;  // line 781
	IClientVehicle *pVehicle;  // line 798
	// inlined CVProfScope::CVProfScope() at line 781
	// inlined Vector::operator=() at line 787
	// inlined CHandle<C_BaseEntity>::operator=() at line 794
	// inlined C_BasePlayer::GetVehicle() at line 798
	// inlined CVProfScope::~CVProfScope() at line 811
	// inlined CVProfScope::~CVProfScope() at line 811
}

// game/client/prediction.cpp:820 @0x623e50 _ZN11CPrediction12StartCommandEP12C_BasePlayerP8CUserCmd
void CPrediction::StartCommand( C_BasePlayer *player, CUserCmd *cmd )
{
	CVProfScope VProf_;  // line 823
	// inlined CVProfScope::CVProfScope() at line 823
	// inlined CUserCmd::operator=() at line 830
	// inlined C_BaseEntity::SetPredictionPlayer() at line 832
	// inlined CVProfScope::~CVProfScope() at line 832
	// inlined CVProfScope::~CVProfScope() at line 832
}

// game/client/prediction.cpp:840 @0x6245e0 _ZN11CPrediction13FinishCommandEP12C_BasePlayer
void CPrediction::FinishCommand( C_BasePlayer *player )
{
	CVProfScope VProf_;  // line 843
	// inlined CVProfScope::~CVProfScope() at line 847
	// inlined C_BaseEntity::SetPredictionPlayer() at line 847
	// inlined CVProfScope::CVProfScope() at line 843
	// inlined CVProfScope::~CVProfScope() at line 847
}

// game/client/prediction.cpp:856 @0x624440 _ZN11CPrediction11RunPreThinkEP12C_BasePlayer
void CPrediction::RunPreThink( C_BasePlayer *player )
{
	CVProfScope VProf_;  // line 859
	// inlined CVProfScope::~CVProfScope() at line 869
	// inlined CVProfScope::CVProfScope() at line 859
	// inlined CVProfScope::~CVProfScope() at line 869
}

// game/client/prediction.cpp:883 @0x624290 _ZN11CPrediction8RunThinkEP12C_BasePlayerd
void CPrediction::RunThink( C_BasePlayer *player, double frametime )
{
	CVProfScope VProf_;  // line 886
	int thinktick;  // line 888
	// inlined CVProfScope::~CVProfScope() at line 896
	// inlined CVProfScope::CVProfScope() at line 886
	// inlined CVProfScope::~CVProfScope() at line 896
}

// game/client/prediction.cpp:906 @0x624120 _ZN11CPrediction12RunPostThinkEP12C_BasePlayer
void CPrediction::RunPostThink( C_BasePlayer *player )
{
	CVProfScope VProf_;  // line 909
	// inlined CVProfScope::~CVProfScope() at line 912
	// inlined CVProfScope::CVProfScope() at line 909
	// inlined CVProfScope::~CVProfScope() at line 912
}

// game/client/prediction.cpp:922 @0x6206e0 _ZN11CPrediction17CheckMovingGroundEP12C_BasePlayerd
void CPrediction::CheckMovingGround( C_BasePlayer *player, double frametime )
{
	C_BaseEntity *groundentity;  // line 924
	// inlined C_BaseEntity::SetBaseVelocity() at line 946
	// inlined Vector::operator VectorByValue&() at line 945
	// inlined operator*() at line 945
	// inlined C_BaseEntity::GetFlags() at line 926
	{
		Vector vecNewVelocity;  // line 931
		// inlined C_BaseEntity::SetBaseVelocity() at line 937
		// inlined Vector::operator+=() at line 935
	}
}

// game/client/prediction.cpp:958 @0x625c90 _ZN11CPrediction10RunCommandEP12C_BasePlayerP8CUserCmdP11IMoveHelper
void CPrediction::RunCommand( C_BasePlayer *player, CUserCmd *ucmd, IMoveHelper *moveHelper )
{
	CVProfScope VProf_;  // line 961
	IClientVehicle *pVehicle;  // line 993
	// inlined CVProfScope::CVProfScope() at line 961
	// inlined C_BasePlayer::GetVehicle() at line 993
	{
		CVProfScope VProf_;  // line 1034
		// inlined CVProfScope::~CVProfScope() at line 1045
		// inlined CVProfScope::CVProfScope() at line 1034
	}
	{
		CVProfScope VProf_;  // line 1051
		// inlined CVProfScope::~CVProfScope() at line 1052
		// inlined CVProfScope::~CVProfScope() at line 1052
		// inlined CVProfScope::CVProfScope() at line 1051
	}
	// inlined CVProfScope::~CVProfScope() at line 1061
	{
		C_BaseCombatWeapon *weapon;  // line 985
		// inlined ToBaseCombatWeapon() at line 985
	}
	// inlined CVProfScope::~CVProfScope() at line 1061
}

// game/client/prediction.cpp:1071 @0x620870 _ZN11CPrediction13SetIdealPitchEiP12C_BasePlayerRK6VectorRK6QAngleS4_
void CPrediction::SetIdealPitch( int nSlot, C_BasePlayer *player, const Vector &origin, const QAngle &angles, const Vector &viewheight )
{
	Vector forward;  // line 1074
	Vector top;  // line 1075
	Vector bottom;  // line 1075
	float floor_height[6];  // line 1076
	int i;  // line 1077
	int j;  // line 1077
	int step;  // line 1078
	int dir;  // line 1078
	int steps;  // line 1078
	trace_t tr;  // line 1079
	CMDLCacheCriticalSection cacheCriticalSection;  // line 1091
	CPrediction::Split_t &split;  // line 1133
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 1142
	// inlined CMDLCacheCriticalSection::CMDLCacheCriticalSection() at line 1091
	// inlined UTIL_TraceLine() at line 1105
	// inlined VectorMA() at line 1097
	// inlined VectorCopy() at line 1101
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 1142
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 1142
}

// game/client/prediction.cpp:1153 (declaration)
void RemoveStalePredictedEntities( int nSlot, int sequence_number );

// game/client/prediction.cpp:1153 @0x61f8e0 _ZN11CPrediction28RemoveStalePredictedEntitiesEii
void CPrediction::RemoveStalePredictedEntities( int nSlot, int sequence_number )
{
}

// game/client/prediction.cpp:1247 @0x623480 _ZN11CPrediction26RestoreOriginalEntityStateEi
void CPrediction::RestoreOriginalEntityState( int nSlot )
{
	CVProfScope VProf_;  // line 1250
	int c;  // line 1256
	// inlined CVProfScope::CVProfScope() at line 1250
	// inlined CPredictableList::GetPredictableCount() at line 1256
	{
		int i;  // line 1257
		{
			C_BaseEntity *ent;  // line 1259
			// inlined CPredictableList::GetPredictable() at line 1259
		}
	}
	// inlined CVProfScope::~CVProfScope() at line 1265
	// inlined CVProfScope::~CVProfScope() at line 1265
}

// game/client/prediction.cpp:1269 (declaration)
void ResetSimulationTick();

// game/client/prediction.cpp:1269 @0x61fa10 _ZN11CPrediction19ResetSimulationTickEv
void CPrediction::ResetSimulationTick()
{
	int nSlot;  // line 1272
	{
		int i;  // line 1274
		{
			C_BaseEntity *entity;  // line 1276
			// inlined CPredictableList::GetPredictable() at line 1276
		}
	}
}

// game/client/prediction.cpp:1292 @0x623000 _ZN11CPrediction13RunSimulationEifP8CUserCmdP12C_BasePlayer
void CPrediction::RunSimulation( int current_command, float curtime, CUserCmd *cmd, C_BasePlayer *localPlayer )
{
	CVProfScope VProf_;  // line 1295
	int nSlot;  // line 1297
	C_CommandContext *ctx;  // line 1300
	// inlined CVProfScope::CVProfScope() at line 1295
	// inlined CUserCmd::operator=() at line 1304
	// inlined IPredictionSystem::SuppressEvents() at line 1307
	// inlined CPrediction::ResetSimulationTick() at line 1309
	{
		int i;  // line 1312
		{
			C_BaseEntity *entity;  // line 1318
			bool islocal;  // line 1326
			// inlined CPredictableList::GetPredictable() at line 1318
		}
	}
	// inlined IPredictionSystem::SuppressEvents() at line 1372
	// inlined CVProfScope::~CVProfScope() at line 1372
	// inlined CVProfScope::~CVProfScope() at line 1372
}

// game/client/prediction.cpp:1379 (declaration)
void Untouch( int nSlot );

// game/client/prediction.cpp:1379 @0x61fa90 _ZN11CPrediction7UntouchEi
void CPrediction::Untouch( int nSlot )
{
	int numpredictables;  // line 1382
	int i;  // line 1385
	// inlined CPredictableList::GetPredictableCount() at line 1382
	{
		C_BaseEntity *entity;  // line 1388
		// inlined CPredictableList::GetPredictable() at line 1388
	}
}

// game/client/prediction.cpp:1400 @0x622cf0 _ZN11CPrediction22StorePredictionResultsEii
void CPrediction::StorePredictionResults( int nSlot, int predicted_frame )
{
	CVProfScope VProf_;  // line 1403
	int c;  // line 1406
	// inlined CVProfScope::CVProfScope() at line 1403
	// inlined CPredictableList::GetPredictableCount() at line 1406
	{
		int i;  // line 1409
		{
			C_BaseEntity *entity;  // line 1411
			// inlined CPredictableList::GetPredictable() at line 1411
			// inlined InvalidateEFlagsRecursive() at line 1421
		}
	}
	// inlined CVProfScope::~CVProfScope() at line 1431
	// inlined CVProfScope::~CVProfScope() at line 1431
}

// game/client/prediction.cpp:1440 @0x622880 _ZN11CPrediction28ShiftIntermediateDataForwardEiii
void CPrediction::ShiftIntermediateDataForward( int nSlot, int slots_to_remove, int number_of_commands_run )
{
	CVProfScope VProf_;  // line 1443
	int c;  // line 1454
	int i;  // line 1455
	// inlined CVProfScope::CVProfScope() at line 1443
	// inlined CVProfScope::~CVProfScope() at line 1466
	// inlined CPredictableList::GetPredictableCount() at line 1454
	{
		C_BaseEntity *ent;  // line 1458
		// inlined CPredictableList::GetPredictable() at line 1458
	}
	// inlined CVProfScope::~CVProfScope() at line 1466
}

// game/client/prediction.cpp:1474 @0x622630 _ZN11CPrediction42ShiftFirstPredictedIntermediateDataForwardEii
void CPrediction::ShiftFirstPredictedIntermediateDataForward( int nSlot, int slots_to_remove )
{
	CVProfScope VProf_;  // line 1480
	int c;  // line 1491
	int i;  // line 1492
	// inlined CVProfScope::~CVProfScope() at line 1503
	// inlined CVProfScope::CVProfScope() at line 1480
	// inlined CPredictableList::GetPredictableCount() at line 1491
	{
		C_BaseEntity *ent;  // line 1495
		// inlined CPredictableList::GetPredictable() at line 1495
	}
	// inlined CVProfScope::~CVProfScope() at line 1503
}

// game/client/prediction.cpp:1511 @0x6223d0 _ZN11CPrediction29RestoreEntityToPredictedFrameEii
void CPrediction::RestoreEntityToPredictedFrame( int nSlot, int predicted_frame )
{
	CVProfScope VProf_;  // line 1514
	int c;  // line 1525
	int i;  // line 1526
	// inlined CVProfScope::CVProfScope() at line 1514
	// inlined CVProfScope::~CVProfScope() at line 1538
	// inlined CPredictableList::GetPredictableCount() at line 1525
	{
		C_BaseEntity *ent;  // line 1529
		// inlined CPredictableList::GetPredictable() at line 1529
	}
	// inlined CVProfScope::~CVProfScope() at line 1538
}

// game/client/prediction.cpp:1549 @0x622ac0 _ZN11CPrediction28ComputeFirstCommandToExecuteEibii
int CPrediction::ComputeFirstCommandToExecute( int nSlot, bool received_new_world_update, int incoming_acknowledged, int outgoing_command )
{
	int destination_slot;  // line 1551
	int skipahead;  // line 1553
	CPrediction::Split_t &split;  // line 1555
	{
		int start;  // line 1565
	}
	{
		C_BasePlayer *pLocalPlayer;  // line 1628
		float flPrev;  // line 1635
		{
			int i;  // line 1638
			{
				C_BaseEntity *entity;  // line 1640
				// inlined CPredictableList::GetPredictable() at line 1640
			}
		}
	}
	{
		int count;  // line 1613
		// inlined CPredictableList::GetPredictableCount() at line 1613
		{
			int i;  // line 1614
			{
				C_BaseEntity *ent;  // line 1616
				// inlined CPredictableList::GetPredictable() at line 1616
			}
		}
	}
}

// game/client/prediction.cpp:1667 @0x6236c0 _ZN11CPrediction17PerformPredictionEiP12C_BasePlayerbii
bool CPrediction::PerformPrediction( int nSlot, C_BasePlayer *localPlayer, bool received_new_world_update, int incoming_acknowledged, int outgoing_command )
{
	CMDLCacheCriticalSection cacheCriticalSection;  // line 1669
	CVProfScope VProf_;  // line 1671
	int i;  // line 1680
	C_BaseEntity *ground;  // line 1694
	CPrediction::Split_t &split;  // line 1702
	bool bTooMany;  // line 1706
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 1808
	// inlined CVProfScope::~CVProfScope() at line 1808
	{
		int current_command;  // line 1711
		CUserCmd *cmd;  // line 1717
		float curtime;  // line 1733
		{
			int j;  // line 1743
			{
				C_BaseEntity *entity;  // line 1745
				// inlined CPredictableList::GetPredictable() at line 1745
				{
					const uint8 *predictedFrame;  // line 1753
				}
			}
		}
		// inlined CPrediction::Untouch() at line 1782
		{
			int count;  // line 1764
			// inlined CPredictableList::GetPredictableCount() at line 1764
			{
				int i;  // line 1765
				{
					C_BaseEntity *entity;  // line 1767
					// inlined CPredictableList::GetPredictable() at line 1767
				}
			}
		}
	}
	// inlined C_BaseEntity::GetMoveParent() at line 1699
	// inlined CVProfScope::CVProfScope() at line 1671
	// inlined CMDLCacheCriticalSection::CMDLCacheCriticalSection() at line 1669
	// inlined CMDLCacheCriticalSection::~CMDLCacheCriticalSection() at line 1808
	// inlined CVProfScope::~CVProfScope() at line 1808
}

// game/client/prediction.cpp:1820 @0x625150 _ZN11CPrediction6UpdateEibii
void CPrediction::Update( int startframe, bool validframe, int incoming_acknowledged, int outgoing_command )
{
	CVProfScope VProf_;  // line 1823
	bool received_new_world_update;  // line 1827
	float flTimeStamp;  // line 1830
	bool bTimeStampChanged;  // line 1831
	// inlined CVProfScope::CVProfScope() at line 1823
	// inlined CVProfScope::~CVProfScope() at line 1854
	{
		int nSlot;  // line 1847
		{
			CSetActiveSplitScreenPlayerGuard g_SSGuard;  // line 1849
		}
	}
	// inlined CVProfScope::~CVProfScope() at line 1854
}

// game/client/prediction.cpp:1862 @0x623c40 _ZN11CPrediction7_UpdateEibbii
void CPrediction::_Update( int nSlot, bool received_new_world_update, bool validframe, int incoming_acknowledged, int outgoing_command )
{
	QAngle viewangles;  // line 1866
	C_BasePlayer *localPlayer;  // line 1868
	{
		C_BaseAnimating::AutoAllowBoneAccess boneaccess;  // line 1894
		int c;  // line 1897
		bool bValid;  // line 1918
		// inlined CPredictableList::GetPredictableCount() at line 1897
		{
			int i;  // line 1898
			{
				C_BaseEntity *ent;  // line 1900
				// inlined CPredictableList::GetPredictable() at line 1900
			}
		}
	}
}

// game/client/prediction.cpp:1942 @0x61f8f0 _ZNK11CPrediction20IsFirstTimePredictedEv
bool CPrediction::IsFirstTimePredicted()
{
}

// game/client/prediction.cpp:1966 @0x61f9a0 _ZN11CPrediction13GetViewOriginER6Vector
void CPrediction::GetViewOrigin( Vector &org )
{
	C_BasePlayer *player;  // line 1968
	// inlined Vector::operator=() at line 1975
	// inlined Vector::Init() at line 1971
}

// game/client/prediction.cpp:1983 @0x621280 _ZN11CPrediction13SetViewOriginER6Vector
void CPrediction::SetViewOrigin( Vector &org )
{
	C_BasePlayer *player;  // line 1985
	// inlined CInterpolatedVarArrayBase<Vector,false>::Reset() at line 1992
	// inlined Vector::operator=() at line 1990
}

// game/client/prediction.cpp:1999 @0x620670 _ZN11CPrediction13GetViewAnglesER6QAngle
void CPrediction::GetViewAngles( QAngle &ang )
{
	C_BasePlayer *player;  // line 2001
	// inlined QAngle::operator=() at line 2008
	// inlined QAngle::Init() at line 2004
}

// game/client/prediction.cpp:2016 @0x621b30 _ZN11CPrediction13SetViewAnglesER6QAngle
void CPrediction::SetViewAngles( QAngle &ang )
{
	C_BasePlayer *player;  // line 2018
	// inlined CInterpolatedVarArrayBase<QAngle,false>::Reset() at line 2023
}

// game/client/prediction.cpp:2030 @0x620600 _ZN11CPrediction18GetLocalViewAnglesER6QAngle
void CPrediction::GetLocalViewAngles( QAngle &ang )
{
	C_BasePlayer *player;  // line 2032
	// inlined QAngle::operator=() at line 2039
	// inlined QAngle::Init() at line 2035
}

// game/client/prediction.cpp:2047 @0x61f950 _ZN11CPrediction18SetLocalViewAnglesER6QAngle
void CPrediction::SetLocalViewAngles( QAngle &ang )
{
	C_BasePlayer *player;  // line 2049
}

// game/client/prediction.cpp:2062 @0x61f920 _ZNK11CPrediction23GetIncomingPacketNumberEv
int CPrediction::GetIncomingPacketNumber()
{
}

// game/client/prediction.cpp:2072 @0x61f930 _ZNK11CPrediction12InPredictionEv
bool CPrediction::InPrediction()
{
}

// game/client/prediction.cpp:2081 @0x61f940 _ZNK11CPrediction12GetSavedTimeEv
float CPrediction::GetSavedTime()
{
}
