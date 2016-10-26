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
	Agent* nextAgent;

public:
	Agent(GameWorld* world, Vector2D position = Vector2D(-1, -1), double rotation = RandFloat()*TwoPi);
	std::string debug = "";

	virtual LeaderAgent* getLeader() = 0;

	inline Vehicle* getVehicle() const { return vehicle; }

	inline Agent* getNext() const { return nextAgent; }
	inline void setNext(Agent* agent) { nextAgent = agent; }

	inline void Update(double time_elapsed) {}

	~Agent();
};

// LeaderAgent --------------------------------------------------------------------------------------------------------------------------------------------------------------
class LeaderAgent : public Agent {
protected:
	int maxFollowers;
	int followers;

public:
	LeaderAgent(GameWorld* world, int maxFollowers = -1, Vector2D position = Vector2D(-1, -1), double rotation = RandFloat()*TwoPi);

	inline LeaderAgent* getLeader() { return this; }

	bool incFollowers();
	bool decFollowers();
	inline bool canAddFollower() const {
		return maxFollowers < 0 || followers < maxFollowers;
	}

	inline void takeControl() {
	}

	inline void cancelControl() {
	}

	~LeaderAgent();
};

// ChaserAgent --------------------------------------------------------------------------------------------------------------------------------------------------------------
class ChaserAgent : public Agent {
protected:
	LeaderAgent* leader;
	Vector2D offset;
	Agent* prevAgent;

public:
	ChaserAgent(GameWorld* world, Vector2D offset = Vector2D(-30, 0), Vector2D position = Vector2D(-1, -1), double rotation = RandFloat()*TwoPi);

	inline Vector2D getOffset() { return offset; }

	inline LeaderAgent* getLeader() { return leader; }

	inline Agent* getPrev() const { return prevAgent; }
	inline void setPrev(Agent* agent) { prevAgent = agent; }

	bool follow(Agent* agent);

	void unfollow();

	void Update(double time_elapsed);

	~ChaserAgent();
};