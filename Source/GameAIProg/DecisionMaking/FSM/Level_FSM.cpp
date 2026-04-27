// Fill out your copyright notice in the Description page of Project Settings.


#include "Level_FSM.h"

#include "FSMComponent.h"
#include "GuardStates.h"
#include "DecisionMaking/GameAIController.h"


// Sets default values
ALevel_FSM::ALevel_FSM()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevel_FSM::BeginPlay()
{
	Super::BeginPlay();
	
	GuardAgent = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, 
	FVector{0,0,90}, FRotator::ZeroRotator);
	GuardAgent->SetDebugRenderingEnabled(true);
	
	ThiefAgent = GetWorld()->SpawnActor<ASteeringAgent>(
	  SteeringAgentClass,
	  FVector{300, 300, 90},
	  FRotator::ZeroRotator);
	ThiefAgent->SetDebugRenderingEnabled(true);
	ThiefSeek = new Seek();
	ThiefSeek->SetTarget(MouseTarget);
	ThiefAgent->SetSteeringBehavior(ThiefSeek);
	
	AGameAIController* AIController = Cast<AGameAIController>(GuardAgent->GetController());
	if (!AIController) return;

	UFSMComponent* FSMComp = Cast<UFSMComponent>(AIController->GetBrainComponent());
	if (!FSMComp) return;

	UBlackboardComponent* BB = FSMComp->GetBlackBoardComponent();
	
	//store thief on blackBoard
	if (BB)
	{
		BB->SetValueAsObject(GuardBlackBoardKeys::TargetActor, ThiefAgent);
	}
	
	// --- Create States ---
	auto PatrolStatePtr{std::make_unique<PatrolState>(GuardAgent, PatrolWayPoints)};
	auto ChaseStatePtr{std::make_unique<ChaseState>(GuardAgent)};
	auto SearchStatePtr{std::make_unique<SearchState>(GuardAgent)};
	
	GameAI::FSM::State* RawPatrol{PatrolStatePtr.get()};
	GameAI::FSM::State* RawChase{ChaseStatePtr.get()};
	GameAI::FSM::State* RawSearch{SearchStatePtr.get()};
	
	FSMComp->AddState(std::move(PatrolStatePtr));
	FSMComp->AddState(std::move(ChaseStatePtr));
	FSMComp->AddState(std::move(SearchStatePtr));
	
	// --- Register Transitions ---
	//patrol- chase
	FSMComp->AddTransition(RawPatrol,RawChase,[BB,this]()
	{
		if (!BB || !ThiefAgent) return  false;
		AActor* Target {Cast<AActor>(BB->GetValueAsObject(GuardBlackBoardKeys::TargetActor))};
		if (!Target) return false;
		
		// Check distance
	  const float DistSq = FVector::DistSquared(
		  GuardAgent->GetActorLocation(),
		  Target->GetActorLocation());
		
		constexpr float DetectionRadius = 600.f;
	  if (DistSq > DetectionRadius * DetectionRadius) return false;
		
		// Check line of sight
	   FHitResult Hit;
	   FCollisionQueryParams Params;
	   Params.AddIgnoredActor(GuardAgent);

	   const bool bHit = GetWorld()->LineTraceSingleByChannel(
		   Hit,
		   GuardAgent->GetActorLocation(),
		   Target->GetActorLocation(),
		   ECC_Visibility,
		   Params
	   );

	   // If we hit something that isn't the target, no line of sight
	   return !bHit || Hit.GetActor() == Target;
	});
	
	//chase- search
	FSMComp->AddTransition(RawChase, RawSearch, [BB, this]()
   {
	   if (!BB || !ThiefAgent) return false;

	   AActor* Target = Cast<AActor>(BB->GetValueAsObject(GuardBlackBoardKeys::TargetActor));
	   if (!Target) return false;

	   const float DistSq = FVector::DistSquared(
		   GuardAgent->GetActorLocation(),
		   Target->GetActorLocation()
	   );
	   constexpr float DetectionRadius = 600.f;
	   if (DistSq > DetectionRadius * DetectionRadius) return true; // out of range = not visible

	   FHitResult Hit;
	   FCollisionQueryParams Params;
	   Params.AddIgnoredActor(GuardAgent);

	   const bool bHit = GetWorld()->LineTraceSingleByChannel(
		   Hit,
		   GuardAgent->GetActorLocation(),
		   Target->GetActorLocation(),
		   ECC_Visibility,
		   Params
	   );

	   return bHit && Hit.GetActor() != Target; // blocked = not visible
   });
	
	//search chase
	FSMComp->AddTransition(RawSearch, RawChase, [BB, this]()
   {
	   // Reuse the same visibility check as Patrol → Chase
	   if (!BB || !ThiefAgent) return false;

	   AActor* Target = Cast<AActor>(BB->GetValueAsObject(GuardBlackBoardKeys::TargetActor));
	   if (!Target) return false;

	   const float DistSq = FVector::DistSquared(
		   GuardAgent->GetActorLocation(),
		   Target->GetActorLocation()
	   );
	   constexpr float DetectionRadius = 600.f;
	   if (DistSq > DetectionRadius * DetectionRadius) return false;

	   FHitResult Hit;
	   FCollisionQueryParams Params;
	   Params.AddIgnoredActor(GuardAgent);

	   const bool bHit = GetWorld()->LineTraceSingleByChannel(
		   Hit,
		   GuardAgent->GetActorLocation(),
		   Target->GetActorLocation(),
		   ECC_Visibility,
		   Params
	   );

	   return !bHit || Hit.GetActor() == Target;
   });
//search - patrol
	FSMComp->AddTransition(RawSearch, RawPatrol, [BB, this]()
	{
		if (!BB) return false;
		
		const float SearchStart{BB->GetValueAsFloat(GuardBlackBoardKeys::SearchStartTime)}; //set to start in void start
		if (SearchStart <= 0.f) return false;

	  constexpr float MaxSearchTime = 5.f;
		return (FPlatformTime::Seconds() - SearchStart) >= MaxSearchTime;
	}
		);
	
	AIController->RunFiniteStateMachine();
}

// Called every frame
void ALevel_FSM::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (ThiefSeek)
	{
		UE_LOG(LogTemp, Warning, TEXT("MouseTarget pos: %s"), *MouseTarget.Position.ToString());
		ThiefSeek->SetTarget(MouseTarget);
	}
}

