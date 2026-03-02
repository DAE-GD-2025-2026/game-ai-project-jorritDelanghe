#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput output{};
	if (m_pFlock->GetNrOfNeighbors() == 0) return output; 
	FVector2D averagePos =  m_pFlock->GetAverageNeighborPos();
	FVector2D toAveragePos = averagePos - pAgent.GetPosition();
	const float epsilon = 0.001f;
	
	if (m_pFlock->GetNrOfNeighbors() == 0) return output;
	
	float distance = (averagePos - pAgent.GetPosition()).Length();
	
	if (distance < epsilon) return output;
	
	toAveragePos.Normalize();
	output.LinearVelocity = toAveragePos * pAgent.GetMaxLinearSpeed();
	output.IsValid = true;
	return output;
}

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput output{};
	const int count {m_pFlock->GetNrOfNeighbors()};
	if (count == 0) return output;
	
	FVector2D steerAway{FVector::ZeroVector};
	const auto& neighbours = m_pFlock->GetNeighbors();
	constexpr float epsilon{0.0001f};
	float  distance{}; //not allocating memory every time in loop
	FVector2D toNeighbour{FVector2D::ZeroVector};
	
	for (int index{}; index<count;++index)
	{
		toNeighbour = neighbours[index]->GetPosition() - pAgent.GetPosition();
		distance = toNeighbour.Length();
		if (distance>=epsilon)
		{
			steerAway -= toNeighbour/(distance*distance);
			
		}
	}
	if (!steerAway.IsZero())
	{
		steerAway.Normalize();
		output.LinearVelocity =steerAway*pAgent.GetMaxLinearSpeed();
		output.IsValid = true;
	}
	return output;
}
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
