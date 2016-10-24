#include "Agent.h"

// Agent --------------------------------------------------------------------------------------------------------------------------------------------------------------------
Agent::Agent(GameWorld* world, Vector2D position, double rotation) :
	nextChaser(nullptr)
{
	if (position == Vector2D(-1, -1))
		position = Vector2D(
			world->cxClient() / 2.0 + RandomClamped()*world->cxClient() / 2.0,
			world->cyClient() / 2.0 + RandomClamped()*world->cyClient() / 2.0);

	vehicle = new Vehicle(world,
		position,                 //initial position
		rotation,                 //start rotation
		Vector2D(0, 0),           //velocity
		Prm.VehicleMass,          //mass
		Prm.MaxSteeringForce,     //max force
		Prm.MaxSpeed,             //max velocity
		Prm.MaxTurnRatePerSecond, //max turn rate
		Prm.VehicleScale);        //scale
}

Agent::~Agent() {
	delete vehicle;
}

// LeaderAgent --------------------------------------------------------------------------------------------------------------------------------------------------------------
LeaderAgent::LeaderAgent(GameWorld* world, int maxFollowers, Vector2D position, double rotation) :
	Agent(world, position, rotation), maxFollowers(maxFollowers)
{
	vehicle->Steering()->WanderOn();
	vehicle->SetScale(Vector2D(10, 10));
}

LeaderAgent::~LeaderAgent() {
}

// ChaserAgent --------------------------------------------------------------------------------------------------------------------------------------------------------------
ChaserAgent::ChaserAgent(GameWorld* world, LeaderAgent* leader, Vector2D offset, Vector2D position, double rotation) :
	Agent(world, position, rotation), leader(leader), prevChaser(nullptr), offset(offset)
{
	vehicle->SetScale(Vector2D(8, 8));
}

bool ChaserAgent::follow(Agent* agent) {
	if (agent->getLeader()->incFollowers()) {
		vehicle->Steering()->OffsetPursuitOn(agent->getVehicle(), offset);
		if (agent->getNext())
			agent->getNext()->follow(this);
		return true;
	} else {
		return false;
	}
}

ChaserAgent::~ChaserAgent() {
}