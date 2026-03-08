// Fill out your copyright notice in the Description page of Project Settings.


#include "Level_Flocking.h"


// Sets default values
ALevel_Flocking::ALevel_Flocking()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

// Called when the game starts or when spawned
void ALevel_Flocking::BeginPlay()
{
	Super::BeginPlay();
	pAgentToEvade = GetWorld()->SpawnActor<ASteeringAgent>( SteeringAgentClass, FTransform(FVector(800.f, 800.f, 0)));
	pSeekBehavior = std::make_unique<Seek>();
	pAgentToEvade->SetSteeringBehavior(pSeekBehavior.get());
	
	TrimWorld->SetTrimWorldSize(1000.f);
	TrimWorld->bShouldTrimWorld = true;

	pFlock = TUniquePtr<Flock>(
		new Flock(
			GetWorld(),
			SteeringAgentClass,
			FlockSize,
			TrimWorld->GetTrimWorldSize(),
			pAgentToEvade,
			true)
			);
}

void ALevel_Flocking::DebugEvadeAgent()
{
	constexpr float radius{80.f};
	DrawDebugCircle(GetWorld(), FVector{pAgentToEvade->GetPosition(),0.f}, radius, 32, FColor::Red,
		false, -1.f, 0, 3.f, FVector::ForwardVector, FVector::RightVector);
}

// Called every frame
void ALevel_Flocking::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	pFlock->UpdateEvadeTarget();
	pFlock->ImGuiRender(WindowPos, WindowSize);
	pFlock->Tick(DeltaTime);
	pFlock->RenderDebug();
	DebugEvadeAgent();
	TrimWorld->SetTrimWorldSize(pFlock->GetWorldSize());
	
	if (bUseMouseTarget)
	{
		pFlock->SetTarget_Seek(MouseTarget);
		pSeekBehavior->SetTarget(MouseTarget);
	}
}

