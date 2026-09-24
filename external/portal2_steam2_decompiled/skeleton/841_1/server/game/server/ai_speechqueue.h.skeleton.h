// DWARF declaration skeleton for game/server/ai_speechqueue.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/ai_speechqueue.h:19
enum DeferredResponseTarget_t
{
	kDRT_ANY = 0,
	kDRT_ALL = 1,
	kDRT_SPECIFIC = 2,
	kDRT_MAX = 3,
};

// game/server/ai_speechqueue.h:31 sizeof=0x40 (i386)
struct CResponseQueue
{
public:
	// game/server/ai_speechqueue.h:41 sizeof=0x8 (i386)
	struct CFollowupTargetSpec_t
	{
	public:
		DeferredResponseTarget_t m_iTargetType; // +0x0  // line 42
		EHANDLE m_hHandle; // +0x4  // line 43
		bool IsValid() const;  // line 44
		CFollowupTargetSpec_t( const DeferredResponseTarget_t &, const EHANDLE & );  // line 47
		CFollowupTargetSpec_t( const EHANDLE & );  // line 50
		CFollowupTargetSpec_t( DeferredResponseTarget_t );  // line 53
		CFollowupTargetSpec_t();  // line 58
	};  // line 41
	// game/server/ai_speechqueue.h:65 sizeof=0x3c (i386)
	struct CDeferredResponse
	{
	public:
		AIConcept_t m_concept; // +0x0  // line 66
		ResponseRules::CriteriaSet m_contexts; // +0x8  // line 67
		float m_fDispatchTime; // +0x2c  // line 68
		EHANDLE m_hIssuer; // +0x30  // line 69
		CResponseQueue::CFollowupTargetSpec_t m_Target; // +0x34  // line 74
		void Init( const AIConcept_t &, const ResponseRules::CriteriaSet *, float, const CResponseQueue::CFollowupTargetSpec_t &, CBaseEntity * );  // line 76
		bool IsQuashed();  // line 77
		void Quash();  // line 78
	};  // line 65
	static void DeferContextsFromCriteriaSet( ResponseRules::CriteriaSet &, const ResponseRules::CriteriaSet * );  // line 81
	CResponseQueue( int );  // line 85
	void Add( const AIConcept_t &, const ResponseRules::CriteriaSet *, float, const CResponseQueue::CFollowupTargetSpec_t &, CBaseEntity * );  // line 93
	void Remove( const AIConcept_t &, CBaseEntity * );  // line 98
	void RemoveSpeechQueuedFor( const CBaseEntity * );  // line 101
	void Evacuate();  // line 104
	void PerFrameDispatch();  // line 107
	void AddExpresserHost( CBaseEntity * );  // line 110
	void RemoveExpresserHost( CBaseEntity * );  // line 113
	int GetNumExpresserTargets() const;  // line 116
	CBaseEntity *GetExpresserHost( int ) const;  // line 117
protected:
	bool DispatchOneResponse( CResponseQueue::CDeferredResponse & );  // line 122
private:
	bool DispatchOneResponse_ThenANY( CResponseQueue::CDeferredResponse &, ResponseRules::CriteriaSet *, CBaseEntity *, float );  // line 127
protected:
	CUtlFixedLinkedList<CResponseQueue::CDeferredResponse> m_Queue; // +0x0  // line 132
	CUtlVector<CHandle<CBaseEntity>,CUtlMemory<CHandle<CBaseEntity>, int> > m_ExpresserTargets; // +0x2c  // line 140
};
