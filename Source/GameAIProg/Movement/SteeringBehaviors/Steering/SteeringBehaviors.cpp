#include "SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"
#include "Kismet/GameplayStatics.h"

//SEEK
//*******
// TODO: Do the Week01 assignment :^)
SteeringOutput Seek::CalculateSteering(float DeltaT, ASteeringAgent & Agent)
{
	SteeringOutput steering{};
	steering.LinearVelocity = Target.Position-Agent.GetPosition(); //no need to normalize, happens in movement input
	return steering;
}

SteeringOutput Flee::CalculateSteering(float DeltaT, ASteeringAgent& Agent) //flee
{
	SteeringOutput steering{};
	steering.LinearVelocity = Agent.GetPosition()- Target.Position;
	return steering;
}

SteeringOutput Arrive::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	constexpr float slowRadius{ 100.f};
	constexpr float fastRadius{ 500.f};
	FVector2D toTarget{Target.Position - Agent.GetPosition()};
	const double distance {toTarget.Length()};

	SteeringOutput steering{};
	
	if (distance<slowRadius ) //stop and reset speed
	{
		steering.LinearVelocity = FVector2D::ZeroVector;
		Agent.SetMaxLinearSpeed(OriginalMaxSpeed);
	}
	else if (distance<fastRadius)
	{
		const double speedRatio{(distance - slowRadius) / (fastRadius - slowRadius)};
		Agent.SetMaxLinearSpeed (OriginalMaxSpeed * speedRatio);
		steering.LinearVelocity = toTarget*Agent.GetMaxLinearSpeed();
	}
	else
	{
		Agent.SetMaxLinearSpeed(OriginalMaxSpeed);
		steering.LinearVelocity = toTarget*Agent.GetMaxLinearSpeed();
	}
	 
	if (IsDebugging)
	{
		DrawDebugLine(Agent.GetWorld(),FVector{Agent.GetPosition(),0.f}
			,FVector{Target.Position,0.f},FColor::Red,false,0,1.f);
		
		DrawDebugCircle(Agent.GetWorld(),FVector{Agent.GetPosition(),0.f},fastRadius
			,100,FColor::Blue,false,-1,0,1.f
			, FVector(1, 0, 0), FVector(0, 1, 0), true);
		
		DrawDebugCircle(Agent.GetWorld(),FVector{Agent.GetPosition(),0.f},slowRadius
		,100,FColor::Green,false,-1,0,1.f
		, FVector(1, 0, 0), FVector(0, 1, 0), true);
		
	}
	return steering;
}
