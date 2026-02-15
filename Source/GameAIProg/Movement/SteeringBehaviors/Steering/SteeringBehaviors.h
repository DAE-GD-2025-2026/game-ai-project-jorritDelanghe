#pragma once

#include <Movement/SteeringBehaviors/SteeringHelpers.h>
#include "Kismet/KismetMathLibrary.h"

class ASteeringAgent;

// SteeringBehavior base, all steering behaviors should derive from this.
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	// Override to implement your own behavior
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent & Agent) = 0;

	void SetTarget(const FTargetData& NewTarget) { Target = NewTarget; }
	
	template<class T, std::enable_if_t<std::is_base_of_v<ISteeringBehavior, T>>* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	FTargetData Target;
	bool IsDebugging{true};
	
};

class Seek : public ISteeringBehavior 
{
public:
	Seek() = default;
	virtual ~Seek() override = default;
	
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent & Agent) override;
};
class Wander final : public Seek
{
public:
	Wander() = default;
	virtual ~Wander() override = default;
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent & Agent) override;
	
	void SetWanderOffset(float offset) {m_OffsetDistance =offset;}
	void SetWanderCenter(bool isWanderingCenter){m_IsCenterWanderer = isWanderingCenter;}
protected:
	float m_OffsetDistance = 6.f; //offset agent
	float m_Radius = 4.f; //wandering radius
	float m_MaxAngleChange= FMath::DegreesToRadians(45); //max wander angle between calculated angles, change per frame 
	float m_WanderAngle = 0.f; //last calculated angle
	bool m_IsCenterWanderer{false}; //wander around center point or not
	
};
class Flee final : public ISteeringBehavior
{
public:
	Flee() = default;
	virtual ~Flee() override = default;
	SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent & Agent) override;
};

class Arrive final:public ISteeringBehavior
{
public:
	Arrive() = default;
	virtual ~Arrive() override = default;
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent & Agent) override;
private:
	float OriginalMaxSpeed {1000.f};
};
class Face final :public ISteeringBehavior
{
public:
	Face() =default;
	virtual ~Face() override = default;	
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent & Agent) override;
	
private:
	float AngleSpeed {2.f};
};
class Pursuit final :public ISteeringBehavior
{
	public:
	Pursuit() =default;
	virtual ~Pursuit() override = default;
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent & Agent) override;
};
class Evade final :public ISteeringBehavior
{
	public:
	Evade() =default;
	virtual ~Evade() override = default;
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent & Agent) override;
};
