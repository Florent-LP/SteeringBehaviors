#include "Agent.h"

// Agent --------------------------------------------------------------------------------------------------------------------------------------------------------------------
Agent::Agent(GameWorld* world, Vector2D position, double rotation) :
	nextAgent(nullptr)
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

	if (!vehicle->Steering()->isSpacePartitioningOn())
		vehicle->Steering()->ToggleSpacePartitioningOnOff();
}

Agent::~Agent() {
	//delete vehicle; // Already cleaned in ~GameWorld
}

// LeaderAgent --------------------------------------------------------------------------------------------------------------------------------------------------------------
LeaderAgent::LeaderAgent(GameWorld* world, int maxFollowers, Vector2D position, double rotation) :
	Agent(world, position, rotation), maxFollowers(maxFollowers), followers(0)
{
	vehicle->SetScale(Vector2D(8, 8));
	vehicle->SetMaxSpeed(70);
	vehicle->Steering()->WanderOn();
}

bool LeaderAgent::incFollowers() {
	if (canAddFollower()) {
		followers++;
		return true;
	}
	else
		return false;
}

bool LeaderAgent::decFollowers() {
	if (followers > 0) {
		followers--;
		return true;
	}
	else
		return false;
}

LeaderAgent::~LeaderAgent() {
}

// ChaserAgent --------------------------------------------------------------------------------------------------------------------------------------------------------------
ChaserAgent::ChaserAgent(GameWorld* world, Vector2D offset, Vector2D position, double rotation) :
	Agent(world, position, rotation), leader(nullptr), prevAgent(nullptr), offset(offset)
{
	vehicle->Steering()->WanderOn();
}

bool ChaserAgent::follow(Agent* agent) {
	LeaderAgent* newLeader = agent->getLeader();
	Agent* newPrev = agent;
	ChaserAgent* newNext = (ChaserAgent*)agent->getNext();

	if (newLeader && newLeader->canAddFollower()) {
		newLeader->incFollowers();
		if (leader) unfollow();
		leader = newLeader;

		vehicle->Steering()->WanderOff();
		vehicle->Steering()->OffsetPursuitOn(newPrev->getVehicle(), offset);

		prevAgent = newPrev;
		prevAgent->setNext(this);

		if (newNext) {
			newNext->getVehicle()->Steering()->OffsetPursuitOff();
			newNext->getVehicle()->Steering()->OffsetPursuitOn(vehicle, newNext->getOffset());
			newNext->setPrev(this);
			nextAgent = newNext;
		}

		return true;
	} else
		return false;
}

void ChaserAgent::unfollow() {
	// Avoid "this was nullptr" Exception
	LeaderAgent* oldLeader = leader;
	Agent* oldPrev = prevAgent;
	ChaserAgent* oldNext = (ChaserAgent*)nextAgent;

	oldLeader->decFollowers();
	leader = nullptr;

	if (oldNext) {
		oldNext->getVehicle()->Steering()->OffsetPursuitOff();
		oldNext->getVehicle()->Steering()->OffsetPursuitOn(oldPrev->getVehicle(), oldNext->getOffset());
		oldNext->setPrev(oldPrev);
	} else
		oldPrev->setNext(nullptr);
	prevAgent = nullptr;
	nextAgent = nullptr;

	vehicle->Steering()->OffsetPursuitOff();
	vehicle->Steering()->WanderOn();
}

void ChaserAgent::Update(double time_elapsed) {
	//if (RandInt(0, 999) != 0) return; // 0.001 proba to follow another agent

	const double followRange = 50.0;
	const std::vector<Agent*>& agents = vehicle->World()->Agents();

	for (unsigned i = 0; i < agents.size(); i++) {
		if ( agents[i] != this && (leader == nullptr || agents[i]->getLeader() != leader) ) {

			Vector2D toAgent = agents[i]->getVehicle()->Pos() - vehicle->Pos();
			if (toAgent.LengthSq() < followRange * followRange) {
					follow(agents[i]);
					break;
			}
		}
	}
}

ChaserAgent::~ChaserAgent() {
}