#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput output{};
	FVector2D averagePos =  m_pFlock->GetAverageNeighborPos();
	FVector2D toAveragePos = averagePos - pAgent.GetPosition();
	const float epsilon = 0.001f;
	
	if (averagePos.IsZero()	) return output;
	
	float distance = (averagePos - pAgent.GetPosition()).Length();
	
	if (distance < epsilon) return output;
	
	toAveragePos.Normalize();
	output.LinearVelocity = toAveragePos * pAgent.GetMaxLinearSpeed();
	output.IsValid = true;
	return output;
}

//*********************
//SEPARATION (FLOCKING)

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT,ASteeringAgent& pAgent)
{
	SteeringOutput output{};
	
	FVector2D  averageVelocity = m_pFlock->GetAverageNeighborVelocity();
	if ( averageVelocity.IsZero()) return output;
	
	 averageVelocity.Normalize();
	output =  averageVelocity*pAgent.GetMaxLinearSpeed();
	output.IsValid = true;
	return output;
}
