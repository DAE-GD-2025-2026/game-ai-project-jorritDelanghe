
#include "CombinedSteeringBehaviors.h"
#include <algorithm>
#include "../SteeringAgent.h"

BlendedSteering::BlendedSteering(const std::vector<WeightedBehavior>& WeightedBehaviors)
	:WeightedBehaviors(WeightedBehaviors)
{};

//****************
//BLENDED STEERING
SteeringOutput BlendedSteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput result = {};
	float totalWeight{0.0f};

	if (WeightedBehaviors.empty())
	{
		return SteeringOutput{};
	}
	// TODO: Calculate the weighted average steeringbehavior
	for (const auto& wb: WeightedBehaviors)
	{
		if (!wb.pBehavior) continue;
		
		SteeringOutput individual = wb.pBehavior->CalculateSteering(DeltaT, Agent);
		result.LinearVelocity += individual.LinearVelocity* wb.Weight;
		result.AngularVelocity += individual.AngularVelocity* wb.Weight;
		totalWeight+= wb.Weight;
	}
	if (totalWeight > 0.0f) // makes lineare speed not get bigger and bigger
	{
		result.LinearVelocity/=totalWeight;
		result.AngularVelocity/=totalWeight;
	}
	
	// TODO: Add debug drawing

	return result;
}

float* BlendedSteering::GetWeight(ISteeringBehavior* const SteeringBehavior)
{
	auto it = find_if(WeightedBehaviors.begin(),
		WeightedBehaviors.end(),
		[SteeringBehavior](const WeightedBehavior& Elem)
		{
			return Elem.pBehavior == SteeringBehavior;
		}
	);

	if(it!= WeightedBehaviors.end())
		return &it->Weight;
	
	return nullptr;
}

//*****************
//PRIORITY STEERING
SteeringOutput PrioritySteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering = {};

	for (ISteeringBehavior* const pBehavior : m_PriorityBehaviors)
	{
		Steering = pBehavior->CalculateSteering(DeltaT, Agent);

		if (Steering.IsValid)
			break;
	}

	//If non of the behavior return a valid output, last behavior is returned
	return Steering;
}