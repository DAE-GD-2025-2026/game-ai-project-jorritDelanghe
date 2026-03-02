#pragma once
#include "Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
class Flock;

//COHESION - FLOCKING
//*******************
class Cohesion final : public Seek
{
public:
	explicit Cohesion(Flock* const pFlock) :m_pFlock(pFlock) {};

	//Cohesion Behavior
	virtual SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& pAgent) override;

private:
	Flock* m_pFlock = nullptr;
};

//SEPARATION - FLOCKING
//*********************
class Separation final: public  ISteeringBehavior
{
	public:
	explicit Separation(Flock*const pFlock) : m_pFlock(pFlock){};
	
	//Separation Behavior
	virtual SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& pAgent) override;
	
private:
	Flock* m_pFlock{nullptr};
};

//VELOCITY MATCH - FLOCKING
//************************
class VelocityMatch final: public ISteeringBehavior
{
	public:
	explicit VelocityMatch(Flock*const pFlock):m_pFlock(pFlock) {};
	virtual SteeringOutput CalculateSteering(float deltaT,ASteeringAgent& pAgent)override;
private:
	Flock*m_pFlock {nullptr};
};
