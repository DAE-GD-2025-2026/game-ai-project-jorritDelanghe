#include "SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

//SEEK
//*******
// TODO: Do the Week01 assignment :^)
SteeringOutput Seek::CalculateSteering(float DeltaT, ASteeringAgent & Agent)
{
	SteeringOutput steering{};
	steering.LinearVelocity = Target.Position-Agent.GetPosition(); //no need to normalize, happens in movement input
	return steering;
}

SteeringOutput Flee::CalculateSteering(float DeltaT, ASteeringAgent& Agent) //flee!
{
	SteeringOutput steering{};
	steering.LinearVelocity = Agent.GetPosition()- Target.Position;
	return steering;
}
