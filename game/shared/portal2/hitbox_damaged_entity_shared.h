//========= Portal 2 reconstruction ============================================//
//
// Purpose: Hitbox damage effect code shared by the server and client damaged
//			entity templates. Included INSIDE the template class body.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
// Reconstruction note: DWARF declares ApplyDamageEffectShared here as a member of HitBoxDamagedEntity<T>,
// so this is a class-body fragment (no include guard); the class provides BaseClass and m_DamagedEntityType.
protected:
void ApplyDamageEffectShared( int damageEffectIndex, int particleEffectIndex )
{
	const DamageInfoVector &damageInfo =
	    g_DamageDatabase.GetDamageInfoVector( m_DamagedEntityType );
	if ( damageEffectIndex < 0 || damageEffectIndex >= damageInfo.Count() )
	{
		Warning( "Damage effect index %d is out of bounds.", damageEffectIndex );
		return;
	}

	// Swap to the damaged model
	if ( damageInfo[damageEffectIndex].swapModelName[0] != '\0' )
	{
		BaseClass::SetModel( damageInfo[damageEffectIndex].swapModelName );
	}

	EmitParticles( this, m_DamagedEntityType, damageEffectIndex, particleEffectIndex );
}
