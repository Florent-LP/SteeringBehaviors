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
	ChaserAgent* nextChaser;

public:
	Agent(GameWorld* world, Vector2D position = Vector2D(-1, -1), double rotation = RandFloat()*TwoPi);

	virtual LeaderAgent* getLeader() = 0;

	inline Vehicle* getVehicle() const { return vehicle; }

	inline ChaserAgent* getNext() const { return nextChaser; }

	~Agent();
};

// LeaderAgent --------------------------------------------------------------------------------------------------------------------------------------------------------------
class LeaderAgent : public Agent {
protected:
	int maxFollowers;
	int followers;

public:
	LeaderAgent(GameWorld* world, int maxFollowers, Vector2D position = Vector2D(-1, -1), double rotation = RandFloat()*TwoPi);

	inline LeaderAgent* getLeader() { return this; }

	inline bool incFollowers() {
		if (followers < maxFollowers) {
			followers++;
			return true;
		} else {
			return false;
		}
	}

	inline bool decFollowers() {
		if (followers > 0) {
			followers--;
			return true;
		} else {
			return false;
		}
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
	ChaserAgent* prevChaser;

public:
	ChaserAgent(GameWorld* world, LeaderAgent* leader, Vector2D offset = Vector2D(-30, 0), Vector2D position = Vector2D(-1, -1), double rotation = RandFloat()*TwoPi);

	inline LeaderAgent* getLeader() { return leader; }

	inline ChaserAgent* getPrev() const { return prevChaser; }

	bool follow(Agent* agent);

	~ChaserAgent();
};