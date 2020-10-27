#ifndef __MOD_TESTSENTRY_H__
#define __MOD_TESTSENTRY_H__


#include "../../idlib/precompiled.h"
#pragma hdrstop
#include "../AI/AI.h"
#include "../Game_local.h"

class rvTestTurret : public idAI {
public:

	CLASS_PROTOTYPE( rvTestTurret );

	rvTestTurret ( void );

	void				InitSpawnArgsVariables	( void );
	void				Create					(idEntity* _owner, const idVec3 &start, const idVec3 &dir, idEntity* ignore);
	void				Spawn					( void );
	void				Save					( idSaveGame *savefile ) const;
	void				Restore					( idRestoreGame *savefile );

	virtual bool		Pain					( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location );

protected:

	virtual bool		CheckActions			( void );

	stateResult_t		State_Combat			( const stateParms_t& parms );
	stateResult_t		State_Killed			( const stateParms_t& parms );

	int					shieldHealth;
	int					maxShots;	
	int					minShots;
	int					shots;

private:

	rvAIAction			actionBlasterAttack;

	stateResult_t		State_Torso_BlasterAttack	( const stateParms_t& parms );

	CLASS_STATES_PROTOTYPE ( rvTestTurret );
};

CLASS_DECLARATION( idAI, rvTestTurret )
END_CLASS

/*
================
rvTestTurret::rvTestTurret
================
*/
rvTestTurret::rvTestTurret ( ) {
	shieldHealth = 0;
}

void rvTestTurret::InitSpawnArgsVariables ( void ) {
	maxShots	= spawnArgs.GetInt ( "maxShots", "1" );
	minShots	= spawnArgs.GetInt ( "minShots", "1" );
}

/*
================
idProjectile::Create
================
*/
void rvTestTurret::Create(idEntity* _owner, const idVec3 &start, const idVec3 &dir, idEntity* ignore) {
	
	idEntity* owner;
	
	idDict		args;
	idStr		shaderName;
	idVec3		light_color;
	idVec3		light_offset;
	idVec3		tmp;
	idMat3		axis;

	Unbind();
	
	axis = dir.ToMat3();

	 // physicsObj is whats crashing the game
	gameLocal.Printf("physicsObj.SetOrigin(start)");
	physicsObj.SetOrigin(start);

	physicsObj.SetAxis(axis);

	//physicsObj.GetClipModel()->SetOwner(ignore ? ignore : _owner);
	

	owner = _owner;

	UpdateVisuals();
}


/*
================
rvTestTurret::Spawn
================
*/
void rvTestTurret::Spawn ( void ) {
	actionBlasterAttack.Init ( spawnArgs,	"action_blasterAttack",	"Torso_BlasterAttack",	AIACTIONF_ATTACK );

	shieldHealth = spawnArgs.GetInt ( "shieldHealth" );
	health += shieldHealth;

	InitSpawnArgsVariables();
	shots		= 0;
}

/*
================
rvTestTurret::Save
================
*/
void rvTestTurret::Save ( idSaveGame *savefile ) const {
	savefile->WriteInt ( shieldHealth );
	savefile->WriteInt ( shots );
	actionBlasterAttack.Save ( savefile );
}

/*
================
rvTestTurret::Restore
================
*/
void rvTestTurret::Restore ( idRestoreGame *savefile ) {
	savefile->ReadInt ( shieldHealth );
	savefile->ReadInt ( shots );
	actionBlasterAttack.Restore ( savefile );

	InitSpawnArgsVariables();
}

/*
================
rvTestTurret::CheckActions
================
*/
bool rvTestTurret::CheckActions ( void ) {
	// Attacks
	if ( PerformAction ( &actionBlasterAttack, (checkAction_t)&idAI::CheckAction_RangedAttack, &actionTimerRangedAttack ) ) {
		return true;
	}
	return idAI::CheckActions ( );
}

/*
================
rvTestTurret::Pain
================
*/
bool rvTestTurret::Pain ( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location ) {
	// Handle the shield effects
	if ( shieldHealth > 0 ) {
		shieldHealth -= damage;
		if ( shieldHealth <= 0 ) {
			PlayEffect ( "fx_shieldBreak", GetPhysics()->GetOrigin(), (-GetPhysics()->GetGravityNormal()).ToMat3() );
		} else {
			PlayEffect ( "fx_shieldHit", GetPhysics()->GetOrigin(), (-GetPhysics()->GetGravityNormal()).ToMat3() );
		}
	}

	return idAI::Pain ( inflictor, attacker, damage, dir, location );
}

/*
===============================================================================

	States 

===============================================================================
*/

CLASS_STATES_DECLARATION ( rvTestTurret )
	STATE ( "State_Combat",			rvTestTurret::State_Combat )
	STATE ( "State_Killed",			rvTestTurret::State_Killed )

	STATE ( "Torso_BlasterAttack",	rvTestTurret::State_Torso_BlasterAttack )
END_CLASS_STATES

/*
================
rvTestTurret::State_Combat
================
*/
stateResult_t rvTestTurret::State_Combat ( const stateParms_t& parms ) {
	// Aquire a new enemy if we dont have one
	if ( !enemy.ent ) {
		CheckForEnemy ( true );
	}

	FaceEnemy ( );
			
	// try moving, if there was no movement run then just try and action instead
	UpdateAction ( );
	
	return SRESULT_WAIT;
}

/*
================
rvTestTurret::State_Killed
================
*/
stateResult_t rvTestTurret::State_Killed ( const stateParms_t& parms ) {
	gameLocal.PlayEffect ( gameLocal.GetEffect ( spawnArgs, "fx_death" ), GetPhysics()->GetOrigin(), (-GetPhysics()->GetGravityNormal()).ToMat3() );
	return idAI::State_Killed ( parms );
}
	
/*
================
rvTestTurret::State_Torso_BlasterAttack
================
*/
stateResult_t rvTestTurret::State_Torso_BlasterAttack ( const stateParms_t& parms ) {
	enum { 
		STAGE_INIT,
		STAGE_FIRE,
		STAGE_WAIT,
	};
	switch ( parms.stage ) {
		case STAGE_INIT:
			DisableAnimState ( ANIMCHANNEL_LEGS );
			shots = (minShots + gameLocal.random.RandomInt(maxShots-minShots+1)) * combat.aggressiveScale;
			return SRESULT_STAGE ( STAGE_FIRE );
			
		case STAGE_FIRE:
			PlayAnim ( ANIMCHANNEL_TORSO, "range_attack", 2 );
			return SRESULT_STAGE ( STAGE_WAIT );

		case STAGE_WAIT:
			if ( AnimDone ( ANIMCHANNEL_TORSO, 2 ) ) {
				if ( --shots <= 0 ) {
					return SRESULT_DONE;
				}
				return SRESULT_STAGE ( STAGE_FIRE );
			}
			return SRESULT_WAIT;
	}
	return SRESULT_ERROR; 
}

#endif /*__MOD_TESTSENTRY_H__*/