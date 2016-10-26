#include "Agent.h"

// Agent --------------------------------------------------------------------------------------------------------------------------------------------------------------------
Agent::Agent(GameWorld* world, Vector2D offset, Vector2D position, double rotation) :
	nextAgent(nullptr), defaultOffset(offset), offset(offset)
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

	//if (!vehicle->Steering()->isSpacePartitioningOn())
		//vehicle->Steering()->ToggleSpacePartitioningOnOff();
}

Agent::~Agent() {
	//delete vehicle; // Already cleaned in ~GameWorld
}

// LeaderAgent --------------------------------------------------------------------------------------------------------------------------------------------------------------
LeaderAgent::LeaderAgent(GameWorld* world, int maxFollowers, Vector2D offset, Vector2D position, double rotation) :
	Agent(world, offset, position, rotation), maxFollowers(maxFollowers), followers(0), vFormEnabled(false), controlled(false), rightNext(nullptr), leftNext(nullptr)
{
	vehicle->SetScale(Vector2D(8, 8));
	vehicle->SetMaxSpeed(70);
	vehicle->Steering()->WanderOn();

}


void LeaderAgent::toggleVform() {
	// If there is no follower, simply toggle the flag
	if (nextAgent == nullptr) {
		vFormEnabled = !vFormEnabled;
		return;
	}

	ChaserAgent* follower;
	int nbFollowers;

	if (!vFormEnabled) { // Enable the V formation
		vFormEnabled = true;

		// Iterate all the lined followers
		for (follower = (ChaserAgent*)nextAgent, nbFollowers = 0;
			follower != nullptr;
			follower = (ChaserAgent*)follower->getNext(), nbFollowers++)
		{
			// Choose a left or right offset (we cut the line in two)
			int coef = (nbFollowers < followers/2) ? 1 : -1;
			follower->setOffset(Vector2D(offset.x, coef*offset.x));

			// Change the OffsetPursuit target and offset
			follower->getVehicle()->Steering()->OffsetPursuitOff();
			if (nbFollowers != 0 && nbFollowers != followers/2)
				// This follower won't be first (on the right or left line)
				follower->getVehicle()->Steering()->OffsetPursuitOn(follower->getPrev()->getVehicle(), follower->getPrev()->getOffset());
			else {
				follower->getVehicle()->Steering()->OffsetPursuitOn(vehicle, Vector2D(offset.x, coef*offset.x));
				if (nbFollowers == 0)
					// This follower will be first on the right
					rightNext = follower;
				else if (nbFollowers == followers / 2) {
					// This follower will be first on the left
					leftNext = follower;
					// We cut the line before this follower
					follower->getPrev()->setNext(nullptr);
					follower->setPrev(this);
				}
			}
		}

	} else { // Disable the V formation
		vFormEnabled = false;

		if (rightNext != nullptr && leftNext != nullptr) {
			// Find the last follower on the right
			for (follower = (ChaserAgent*)rightNext;
				follower->getNext() != nullptr;
				follower = (ChaserAgent*)follower->getNext());

			// Have the first follower on the left follow it
			follower->setNext(leftNext);
			((ChaserAgent*)leftNext)->setPrev(follower);
		} else
			this->setNext(leftNext);

		// Iterate all followers
		for (follower = (ChaserAgent*)nextAgent, nbFollowers = 0;
			follower != nullptr;
			follower = (ChaserAgent*)follower->getNext(), nbFollowers++)
		{
			// Set them in line (change the OffsetPursuit)
			follower->resetOffset();
			follower->getVehicle()->Steering()->OffsetPursuitOff();
			follower->getVehicle()->Steering()->OffsetPursuitOn(follower->getPrev()->getVehicle(), follower->getPrev()->getOffset());
		}

		// Have the leader forget about the right and left followers
		rightNext = nullptr;
		leftNext = nullptr;
	}
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

void LeaderAgent::followersLeftRight(int &nbLeft, int &nbRight) const {
	Agent* agent;

	// Count agents on the right
	for (agent = rightNext, nbRight = 0;
		agent != nullptr;
		agent = (ChaserAgent*)agent->getNext(), nbRight++);

	// Count agents on the left
	for (agent = leftNext, nbLeft = 0;
		agent != nullptr;
		agent = (ChaserAgent*)agent->getNext(), nbLeft++);
}

Agent* LeaderAgent::getNext() const {
	if (!vFormEnabled)
		// In line formation, only one agent follows
		return nextAgent;
	else {
		// In V formation, the leader returns the first agent
		// from the less populated line (right or left)
		int nbLeft, nbRight;
		followersLeftRight(nbLeft, nbRight);
		return (nbLeft > nbRight) ? rightNext : leftNext;
	}
}

void LeaderAgent::setNext(Agent* agent) {
	if (!vFormEnabled)
		// In line formation, only one agent follows
		nextAgent = agent;
	else {
		// In V formation, the leader changes the first agent
		// of the less populated line (right or left)
		int nbLeft, nbRight;
		followersLeftRight(nbLeft, nbRight);
		if (nbLeft > nbRight) {
			rightNext = agent;
			nextAgent = agent;
		}
		else
			leftNext = agent;
	}
}

Vector2D LeaderAgent::getOffset() const {
	if (!vFormEnabled)
		// In line formation, the leader wants to be followed straight
		return offset;
	else {
		// In V formation, it will be either a left or right offset
		// (sends the new follower on the less populated line)
		int nbLeft, nbRight;
		followersLeftRight(nbLeft, nbRight);
		int coef = (nbLeft > nbRight) ? 1 : -1;
		return Vector2D(offset.x, coef*offset.x);
	}
}

void LeaderAgent::takeControl() {
	vehicle->Steering()->WanderOff();
	vehicle->Steering()->UserInputOn();
	vehicle->SetMaxSpeed(150);
	controlled = true;
}

void LeaderAgent::cancelControl() {
	vehicle->Steering()->UserInputOff();
	vehicle->Steering()->WanderOn();
	vehicle->SetMaxSpeed(70);
	controlled = false;
}


LeaderAgent::~LeaderAgent() {
}

// ChaserAgent --------------------------------------------------------------------------------------------------------------------------------------------------------------
ChaserAgent::ChaserAgent(GameWorld* world, Vector2D offset, Vector2D position, double rotation) :
	Agent(world, offset, position, rotation), leader(nullptr), prevAgent(nullptr)
{
	vehicle->Steering()->WanderOn();
}


bool ChaserAgent::follow(Agent* agent) {
	LeaderAgent* newLeader = agent->getLeader();
	Agent* newPrev = agent;
	ChaserAgent* newNext = (ChaserAgent*)agent->getNext();

	// We want to follow this agent only if it has leader (with enough capacity)
	if (newLeader && newLeader->canAddFollower()) {
		// Change leader
		newLeader->incFollowers();
		if (leader) unfollow();
		leader = newLeader;

		// Follow the agent
		offset = newPrev->getOffset();
		vehicle->Steering()->WanderOff();
		vehicle->Steering()->OffsetPursuitOn(newPrev->getVehicle(), offset);

		prevAgent = newPrev;
		prevAgent->setNext(this);

		// Have the former follower follow me
		if (newNext) {
			newNext->getVehicle()->Steering()->OffsetPursuitOff();
			newNext->getVehicle()->Steering()->OffsetPursuitOn(vehicle, offset);
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

	// Forget about the leader
	oldLeader->decFollowers();
	leader = nullptr;
	offset = defaultOffset;

	if (oldNext) {
		// Reattach the queue after leaving it (no gap left)
		oldNext->getVehicle()->Steering()->OffsetPursuitOff();
		oldNext->getVehicle()->Steering()->OffsetPursuitOn(oldPrev->getVehicle(), oldPrev->getOffset());
		oldNext->setPrev(oldPrev);
	} else
		oldPrev->setNext(nullptr);

	// Forget about followed and follower
	prevAgent = nullptr;
	nextAgent = nullptr;

	// Fall back to default behavior
	vehicle->Steering()->OffsetPursuitOff();
	vehicle->Steering()->WanderOn();
}

void ChaserAgent::Update(double time_elapsed) {
	//if (RandInt(0, 499) != 0) return; // 5% chances of following another agent

	const double followRange = 50.0; // Follow when another agent is in this range
	const std::vector<Agent*>& agents = vehicle->World()->Agents(); // Couldn't use space partitionning successfully (way too late)

	// Iterate through all agents in the game world
	for (unsigned i = 0; i < agents.size(); i++) {
		//if (agents[i] != this && agents[i]->getLeader() != nullptr && agents[i]->getLeader() != leader) { // Circular following bug
		if (agents[i] != this && agents[i]->getLeader() != nullptr && leader == nullptr) { // TmpFix : Disable the agents from switching their leader

			Vector2D toAgent = agents[i]->getVehicle()->Pos() - vehicle->Pos();
			if (toAgent.LengthSq() < followRange * followRange) {
					// Follow the first agent found in range
					follow(agents[i]);
					break;
			}
		}
	}
}


ChaserAgent::~ChaserAgent() {
}