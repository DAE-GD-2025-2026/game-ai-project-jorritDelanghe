#pragma once

#include "FSM.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

class ASteeringAgent;
class AActor;

namespace GuardBlackBoardKeys
{
	inline const  FName TargetActor="TargetActor";
	inline const FName LastPosX="LastPosX";
	inline const FName LastPosY="LastPosY";
	inline const FName SearchStartTime="SearchStartTime";
	inline const FName PatrolIndex="PatrolIndex";
}
class PatrolState : public GameAI::FSM::State
{
	public:
	explicit PatrolState(ASteeringAgent* InGuard , TArray<FVector> InWayPoints);
	
	virtual void OnEnter(UBlackboardComponent* BlackBoard) override;
	virtual void Update(float DeltaTime, UBlackboardComponent* BlackBoard) override;
	virtual void OnExit(UBlackboardComponent* BlackBoard) override;
	virtual std::string GetName() const override {return "PatrolState";};
	
private:
ASteeringAgent* GuardAgent{nullptr};
	TArray<FVector2D> WayPoints;
	int32 CurrentWayPointIndex{0};
	Seek* SeekBehavior{nullptr};
	
	static constexpr float Radius{150.0f};
	
};

class ChaseState : public GameAI::FSM::State
{
public:
	explicit ChaseState(ASteeringAgent* InGuard );
	
	virtual void OnEnter(UBlackboardComponent* BlackBoard) override;
	virtual void Update(float DeltaTime, UBlackboardComponent* BlackBoard) override;
	virtual void OnExit(UBlackboardComponent* BlackBoard) override;
	virtual std::string GetName() const override {return "ChaseState";};
	
private:
	ASteeringAgent* GuardAgent{nullptr};
	Seek* SeekBehavior{nullptr}; 
};

class SearchState : public GameAI::FSM::State
{
public:
	explicit SearchState(ASteeringAgent* InGuard , float MaxSearchTime = 5.f);
	
	virtual void OnEnter(UBlackboardComponent* BlackBoard) override;
	virtual void Update(float DeltaTime, UBlackboardComponent* BlackBoard) override;
	virtual void OnExit(UBlackboardComponent* BlackBoard) override;
	virtual std::string GetName() const override {return "SearchState";};
	
private:
	ASteeringAgent* GuardAgent{nullptr};
	float MaxSearchTime{5.f};
	bool bReachedLastPos{false};
	Seek* SeekBehavior{nullptr};   
	Wander* WanderBehavior{nullptr};
	
	static constexpr float SearchRadius{150.0f};
	
};
