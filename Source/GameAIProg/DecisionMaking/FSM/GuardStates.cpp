#include "GuardStates.h"
#include "Movement/SteeringBehaviors/SteeringAgent.h"

static FVector2D GetLastKnownPos(UBlackboardComponent* BB)
{
	return FVector2D{
		BB->GetValueAsFloat(GuardBlackBoardKeys::LastPosX),
		BB->GetValueAsFloat(GuardBlackBoardKeys::LastPosY)
	};
}
static void SetLastKnownPos(UBlackboardComponent* BB, FVector2D Pos)
{
	BB->SetValueAsFloat(GuardBlackBoardKeys::LastPosX, Pos.X);
	BB->SetValueAsFloat(GuardBlackBoardKeys::LastPosY, Pos.Y);
}
static FTargetData MakeTarget(const FVector2D& Pos)
{
	    return FTargetData{FVector2D{Pos.X, Pos.Y}}; // Only sets Position, rest defaults to zero
}
//patrol
PatrolState::PatrolState(ASteeringAgent* InGuard, TArray<FVector> InWayPoints)
	:GuardAgent(InGuard)
	,WayPoints(InWayPoints)
{
}

void PatrolState::OnEnter(UBlackboardComponent* BlackBoard)
{
	UE_LOG(LogTemp, Log, TEXT("[FSM] Entering PATROL"));
	if (!GuardAgent || WayPoints.IsEmpty()) return;
	
	if (BlackBoard)
	{
		CurrentWayPointIndex = BlackBoard->GetValueAsInt(GuardBlackBoardKeys::PatrolIndex);
		
		// Create a Seek toward the current waypoint
		// Seek stores a TargetInfo by value so we give it the waypoint position
		
		SeekBehavior = new Seek();
		SeekBehavior->SetTarget(MakeTarget(WayPoints[CurrentWayPointIndex]));
		GuardAgent->SetSteeringBehavior(SeekBehavior);
	}
}

void PatrolState::Update(float DeltaTime, UBlackboardComponent* BlackBoard)
{
	if (!GuardAgent || WayPoints.IsEmpty() || !SeekBehavior) return;
	
	const FVector2D GuardPos{GuardAgent->GetPosition()};
	const FVector2D TargetPos{WayPoints[CurrentWayPointIndex]};
	
	if (FVector2D::Distance(GuardPos,TargetPos)<Radius)
	{
		CurrentWayPointIndex = (CurrentWayPointIndex + 1) % WayPoints.Num();
		if (BlackBoard)
		{
			BlackBoard->SetValueAsInt(GuardBlackBoardKeys::PatrolIndex, CurrentWayPointIndex);
		}
		SeekBehavior->SetTarget(MakeTarget(FVector2D{WayPoints[CurrentWayPointIndex]}));
	}
}

void PatrolState::OnExit(UBlackboardComponent* BlackBoard)
{
	UE_LOG(LogTemp, Log, TEXT("[FSM] Exiting PATROL"));
	SeekBehavior = nullptr;
}
//chase
void ChaseState::OnEnter(UBlackboardComponent* BlackBoard)
{
	UE_LOG(LogTemp, Log, TEXT("[FSM] Entering Chase"));
	if (!GuardAgent) return;
	
	SeekBehavior = new Seek();
	GuardAgent->SetSteeringBehavior(SeekBehavior);
}

void ChaseState::Update(float DeltaTime, UBlackboardComponent* BlackBoard)
{
	if (!GuardAgent || !BlackBoard || !SeekBehavior) return;
	
	AActor* ThiefTarget{Cast<AActor>(BlackBoard->GetValueAsObject(GuardBlackBoardKeys::TargetActor))};
	if (!ThiefTarget) return;
	const FVector2D TargetPos{ThiefTarget->GetActorLocation()};
	
	SetLastKnownPos(BlackBoard,TargetPos);
	SeekBehavior->SetTarget(MakeTarget(TargetPos));
}

void ChaseState::OnExit(UBlackboardComponent* BlackBoard)
{
	UE_LOG(LogTemp, Log, TEXT("[FSM] Exiting CHASE"));
	SeekBehavior = nullptr;
}
//search
void SearchState::OnEnter(UBlackboardComponent* BlackBoard)
{
	UE_LOG(LogTemp, Log, TEXT("[FSM] Entering Search"));
	if (!GuardAgent || !BlackBoard) return;
	bReachedLastPos = false;
	
	const float Now{GuardAgent->GetWorld()->GetTimeSeconds()};
	BlackBoard->SetValueAsFloat(GuardBlackBoardKeys::SearchStartTime, Now);
	
	SeekBehavior = new Seek();
	SeekBehavior->SetTarget(MakeTarget(GetLastKnownPos(BlackBoard)));
	GuardAgent->SetSteeringBehavior(SeekBehavior);
}

void SearchState::Update(float DeltaTime, UBlackboardComponent* BlackBoard)
{
	if (!GuardAgent || !BlackBoard) return;
	if (!bReachedLastPos)
	{
		const FVector2D GuardPos{GuardAgent->GetPosition()};
		const FVector2D LastPos{GetLastKnownPos(BlackBoard)};
		
		if (FVector2D::Distance(GuardPos,LastPos)<SearchRadius)
		{
			bReachedLastPos = true;
			
			// Switch to wander once we arrive
			WanderBehavior = new Wander();
			GuardAgent->SetSteeringBehavior(WanderBehavior);
			SeekBehavior = nullptr;
		}
	}
	
}

void SearchState::OnExit(UBlackboardComponent* BlackBoard)
{
	UE_LOG(LogTemp, Log, TEXT("[FSM] Exiting SEARCH"));
	bReachedLastPos = false;
	SeekBehavior    = nullptr;
	WanderBehavior  = nullptr;
}
