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
	virtual SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& pAgent) override;

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
	virtual SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& pAgent) override;
	
private:
	Flock* m_pFlock{nullptr};
};

//VELOCITY MATCH - FLOCKING
//************************
class VelocityMatch: public ISteeringBehavior
{
	public:
	VelocityMatch(Flock*pFlock):m_pFlock(pFlock) {};
	virtual SteeringOutput CalculateSteering(float deltaT,ASteeringAgent& pAgent)override;
private:
	Flock*m_pFlock {nullptr};
};
