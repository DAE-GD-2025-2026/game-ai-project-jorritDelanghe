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
	
	if (IsDebugging)
	{
		constexpr float offset{100.f};
		float agentAngleRad = FMath::DegreesToRadians(Agent.GetRotation());
		const FVector2D forwardDirection{FMath::Cos(agentAngleRad),FMath::Sin(agentAngleRad)};
		
		//current line agent is looking at
		DrawDebugLine(Agent.GetWorld(),FVector{Agent.GetPosition(),0.f}
			,FVector{Agent.GetPosition()+forwardDirection*offset,0.f},FColor::Green,false,0,1.f);
		
		//target line
		DrawDebugLine(Agent.GetWorld(),FVector{Agent.GetPosition(),0.f}
			,FVector{Target.Position,0.f},FColor::Red,false,0,1.f);
		
	}
	return steering;
}

SteeringOutput Wander::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	// Forward direction
	float agentAngleRad = FMath::DegreesToRadians(Agent.GetRotation());
	const FVector2D forwardDirection{FMath::Cos(agentAngleRad),FMath::Sin(agentAngleRad)};
	
	// Circle center
	const FVector2D centerCircle = forwardDirection * m_OffsetDistance;
	SteeringOutput steering{};
	
	//set rand angle
	m_WanderAngle += (FMath::SRand() * 2.f - 1.f) * m_MaxAngleChange;
	
	//Target on circle
	FVector2D targetOnCircle{
		centerCircle.X + m_Radius*FMath::Cos(m_WanderAngle)
		,centerCircle.Y + m_Radius*FMath::Sin(m_WanderAngle)};
	
	//set target
	steering.LinearVelocity = targetOnCircle;
	
	if (IsDebugging)
	{
		constexpr float offset{50.f};
		//big circle
		DrawDebugCircle(Agent.GetWorld(),FVector{centerCircle,0.f},m_Radius
			,100,FColor::Blue,false,-1,0,1.f
			, FVector(1, 0, 0), FVector(0, 1, 0), true);
		
		//current line agent is looking at
		DrawDebugLine(Agent.GetWorld(),FVector{Agent.GetPosition(),0.f}
			,FVector{Agent.GetPosition()+forwardDirection*offset,0.f},FColor::Green,false,0,1.f);
		
		//target line
		DrawDebugLine(Agent.GetWorld(),FVector{Agent.GetPosition(),0.f}
			,FVector{targetOnCircle,0.f},FColor::Red,false,0,1.f);
	}
	return steering;
}

SteeringOutput Flee::CalculateSteering(float DeltaT, ASteeringAgent& Agent) //flee
{
	SteeringOutput steering{};
	steering.LinearVelocity = Agent.GetPosition()- Target.Position;
	
	if (IsDebugging)
	{
		constexpr float offset{100.f};
		float agentAngleRad = FMath::DegreesToRadians(Agent.GetRotation());
		const FVector2D forwardDirection{FMath::Cos(agentAngleRad),FMath::Sin(agentAngleRad)};
		
		//current line agent is looking at
		DrawDebugLine(Agent.GetWorld(),FVector{Agent.GetPosition(),0.f}
			,FVector{Agent.GetPosition()+forwardDirection*offset,0.f},FColor::Green,false,0,1.f);
		
		//target line
		DrawDebugLine(Agent.GetWorld(),FVector{Agent.GetPosition(),0.f}
			,FVector{Target.Position,0.f},FColor::Red,false,0,1.f);
		
	}
	
	return steering;
}

SteeringOutput Arrive::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	constexpr float slowRadius{ 100.f};
	constexpr float fastRadius{ 500.f};
	const FVector2D toTarget{Target.Position - Agent.GetPosition()};
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
		//target line
		DrawDebugLine(Agent.GetWorld(),FVector{Agent.GetPosition(),0.f}
			,FVector{Target.Position,0.f},FColor::Red,false,0,1.f);
		
		//big circle
		DrawDebugCircle(Agent.GetWorld(),FVector{Agent.GetPosition(),0.f},fastRadius
			,100,FColor::Blue,false,-1,0,1.f
			, FVector(1, 0, 0), FVector(0, 1, 0), true);
		
		//small circle
		DrawDebugCircle(Agent.GetWorld(),FVector{Agent.GetPosition(),0.f},slowRadius
		,100,FColor::Green,false,-1,0,1.f
		, FVector(1, 0, 0), FVector(0, 1, 0), true);
		
	}
	return steering;
}

SteeringOutput Face::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	const FVector2D directionToTarget {Target.Position - Agent.GetPosition()};
	SteeringOutput steering{}; 
	float angleTarget = FMath::RadiansToDegrees(FMath::Atan2(directionToTarget.Y, directionToTarget.X));
	
	//shorest rotation
	float angleDiff = FMath::FindDeltaAngleDegrees(Agent.GetRotation(),angleTarget);
	
	steering.AngularVelocity = angleDiff*AngleSpeed;
	
	if (IsDebugging)
	{
		constexpr float offset{100.f};
		float agentAngleRad = FMath::DegreesToRadians(Agent.GetRotation());
		const FVector2D forwardDirection{FMath::Cos(agentAngleRad),FMath::Sin(agentAngleRad)};
		
		//current line agent is looking at
		DrawDebugLine(Agent.GetWorld(),FVector{Agent.GetPosition(),0.f}
			,FVector{Agent.GetPosition()+forwardDirection*offset,0.f},FColor::Green,false,0,1.f);
		
		//target line
		DrawDebugLine(Agent.GetWorld(),FVector{Agent.GetPosition(),0.f}
			,FVector{Target.Position,0.f},FColor::Red,false,0,1.f);
		
	}
	return steering;
}
