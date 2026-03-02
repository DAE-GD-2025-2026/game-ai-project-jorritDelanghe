#pragma once
#include "Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
class Flock;

//COHESION - FLOCKING
//*******************
class Cohesion final : public Seek
{
public:
	Cohesion(Flock* const pFlock) :m_pFlock(pFlock) {};

	//Cohesion Behavior
	SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& pAgent) override;

private:
	Flock* m_pFlock = nullptr;
};

//SEPARATION - FLOCKING
//*********************
class Seperation final: public  ISteeringBehavior
{
	public:
	Seperation(Flock*pFlock);
	
	//Speration Behavior
	SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& pAgent) override;
	
private:
	Flock* m_pFlock{nullptr};
};

//VELOCITY MATCH - FLOCKING
//************************
