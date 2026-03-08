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
	const FVector2D averagePos =  m_pFlock->GetAverageNeighborPos();
	FVector2D toAveragePos = averagePos - pAgent.GetPosition();
	
	float distance = (averagePos - pAgent.GetPosition()).Length();
	
	if (constexpr float epsilon = 0.001f; distance < epsilon) return output;
	
	toAveragePos.Normalize();
	output.LinearVelocity = toAveragePos * pAgent.GetMaxLinearSpeed();
	output.IsValid = true;
	return output;
}

void Cohesion::DebugRender(UWorld* pWorld, ASteeringAgent& agent)
{
	if (m_pFlock->GetNrOfNeighbors() == 0) return;
	FVector avgPos = FVector(m_pFlock->GetAverageNeighborPos(), 0.f);
	FVector from = FVector(agent.GetPosition(), 0.f);
	DrawDebugDirectionalArrow(pWorld, from, avgPos, 10.f, FColor::Blue, false, -1.f, 0, 2.f);
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

void Separation::DebugRender(UWorld* pWorld, ASteeringAgent& agent)
{
	SteeringOutput out = CalculateSteering(0.f, agent); // or cache last output
	FVector from = FVector(agent.GetPosition(), 0.f);
	FVector to = from + FVector(out.LinearVelocity, 0.f) * 0.5f;
	DrawDebugDirectionalArrow(pWorld, from, to, 10.f, FColor::Red, false, -1.f, 0, 2.f);

}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT,ASteeringAgent& pAgent)
{
	SteeringOutput output{};
	
	FVector2D  averageVelocity = m_pFlock->GetAverageNeighborVelocity();
	if ( averageVelocity.IsZero()) return output;
	
	 averageVelocity.Normalize();
	output.LinearVelocity =  averageVelocity*pAgent.GetMaxLinearSpeed();
	output.IsValid = true;
	return output;
}

void VelocityMatch::DebugRender(UWorld* pWorld, ASteeringAgent& agent)
{
	FVector2D avgVel = m_pFlock->GetAverageNeighborVelocity();
	if (avgVel.IsZero()) return;
	
	FVector from = FVector(agent.GetPosition(), 0.f);
	FVector to = from + FVector(avgVel * 80.f, 0.f); // 80 = visual scale

	DrawDebugDirectionalArrow(pWorld, from, to, 10.f, FColor::Cyan, false, -1.f, 0, 2.f);
}
