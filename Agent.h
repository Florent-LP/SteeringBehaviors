// Agent.h
// Authors : Florent LE PRINCE <florent.le-prince@uqac.ca>, Maxime LEGRAND <maxime.legrand1@uqac.ca>
#pragma once

#include <vector>

#include "BaseGameEntity.h"
#include "ParamLoader.h"
#include "GameWorld.h"
#include "SteeringBehaviors.h"
#include "Vehicle.h"

class ChaserAgent;
class LeaderAgent;

// Agent --------------------------------------------------------------------------------------------------------------------------------------------------------------------
class Agent : public BaseGameEntity {
protected:
	Vehicle* vehicle;
	Agent* nextAgent;		// Used to store a follower
	Vector2D defaultOffset; // Offset to the follower (const)
	Vector2D offset;		// Offset to the follower (dyn)

public:
	std::string debug = "";

	Agent(GameWorld* world, Vector2D offset = Vector2D(-30, 0), Vector2D position = Vector2D(-1, -1), double rotation = RandFloat()*TwoPi);


	virtual LeaderAgent* getLeader() = 0; // Leader of the agent

	inline Vehicle* getVehicle() const { return vehicle; }

	// Follower accessors
	virtual Agent* getNext() const { return nextAgent; }
	virtual void setNext(Agent* agent) { nextAgent = agent; }

	// Offset accessors
	virtual Vector2D getOffset() const { return offset; }
	virtual void setOffset(Vector2D o) { offset = o; }
	inline void resetOffset() { offset = defaultOffset; }

	// Agent behavior (other than steering behaviors)
	virtual void Update(double time_elapsed) {}


	~Agent();
};

// LeaderAgent --------------------------------------------------------------------------------------------------------------------------------------------------------------
class LeaderAgent : public Agent {
protected:
	int maxFollowers;	// -1 stands for infinity
	int followers;		// Current number of followers

	bool vFormEnabled;	// If true, followers are in V formation
	Agent* rightNext;	// Next follower to the right (V formation)
	Agent* leftNext;	// Next follower to the left (V formation)

	bool controlled; // If true, the agent is controlled via keyboard arrows (UserInput() Steering Behavior)

public:
	LeaderAgent(GameWorld* world, int maxFollowers = -1, Vector2D offset = Vector2D(-30, 0), Vector2D position = Vector2D(-1, -1), double rotation = RandFloat()*TwoPi);


	inline LeaderAgent* getLeader() { return this; } // The leader is its own leader

	// Toggle to V formation
	void toggleVform();

	// Followers count accessors
	bool incFollowers();
	bool decFollowers();
	inline bool canAddFollower() const {
		return maxFollowers < 0 || followers < maxFollowers;
	}
	void followersLeftRight(int& nbLeft, int& nbRight) const; // In V formation, counts right and left followers

	// Follower accessors (specialization)
	Agent* getNext() const;
	void setNext(Agent* agent);

	// Offset accessor (specialization)
	Vector2D getOffset() const;

	// Control agent via keyboard arrows
	void takeControl();
	void cancelControl();
	inline bool isControlled() { return controlled; }


	~LeaderAgent();
};

// ChaserAgent --------------------------------------------------------------------------------------------------------------------------------------------------------------
class ChaserAgent : public Agent {
protected:
	LeaderAgent* leader;	// Leader of the agent
	Agent* prevAgent;		// Followed agent

public:
	ChaserAgent(GameWorld* world, Vector2D offset = Vector2D(-30, 0), Vector2D position = Vector2D(-1, -1), double rotation = RandFloat()*TwoPi);


	inline LeaderAgent* getLeader() { return leader; } // Leader of the agent

	// Followed agent accessors
	inline Agent* getPrev() const { return prevAgent; }
	inline void setPrev(Agent* agent) { prevAgent = agent; }

	bool follow(Agent* agent);
	void unfollow();

	// The chaser can dynamically follow other agents (and switch its leader)
	void Update(double time_elapsed);


	~ChaserAgent();
};