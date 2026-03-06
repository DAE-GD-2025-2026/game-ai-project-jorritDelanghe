#include "Flock.h"
#include "FlockingSteeringBehaviors.h"
#include "Shared/ImGuiHelpers.h"


Flock::Flock(
	UWorld* pWorld,
	TSubclassOf<ASteeringAgent> AgentClass,
	int FlockSize,
	float WorldSize,
	ASteeringAgent* const pAgentToEvade,
	bool bTrimWorld)
	: pWorld{pWorld}
    ,WorldSize{WorldSize}
	, FlockSize{ FlockSize }
	, pAgentToEvade{pAgentToEvade}
{
	//initialize flock
	Agents.Reserve(FlockSize);
	for (int index{}; index<FlockSize;++index)
	{
		FVector pos {FMath::RandRange(-WorldSize,WorldSize)
						,FMath::RandRange(-WorldSize,WorldSize),0.0f};
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn; //to make sure they spawn
		
		if (ASteeringAgent* agent = pWorld->SpawnActor<ASteeringAgent>(AgentClass,FTransform(pos),SpawnParams))
		{
			agent->SetActorTickEnabled(false);
			Agents.Add(agent);
		}
	}
	
	pSeparationBehavior = std::make_unique<Separation>(this);
	pCohesionBehavior = std::make_unique<Cohesion>(this);
	pVelMatchBehavior =std::make_unique<VelocityMatch>(this);
	pSeekBehavior =std::make_unique<Seek>();
	pWanderBehavior = std::make_unique<Wander>();
	
	pBlendedSteering = std::make_unique<BlendedSteering>(
		std::vector<BlendedSteering::WeightedBehavior>{
			{pCohesionBehavior.get(),CohesionWeight}
		,{pSeparationBehavior.get(), SeparationWeight}
		,{pVelMatchBehavior.get(), VelocityMatchWeight}
		,{pSeekBehavior.get(), SeekWeight}
		,{pWanderBehavior.get(), WanderWeight}});
	//set target to evade
	pEvadeBehavior = std::make_unique<Evade>();
	if (pAgentToEvade) pEvadeBehavior->SetTarget(FSteeringParams{FVector2D{pAgentToEvade->GetTargetLocation()}});
	
	pPrioritySteering = std::make_unique<PrioritySteering>(
		std::vector<ISteeringBehavior*>{
			pEvadeBehavior.get(), pBlendedSteering.get()
		});
	
	for (auto agent:Agents)
		agent->SetSteeringBehavior(pPrioritySteering.get());
	
	Neighbors.Reserve(FlockSize);
}

Flock::~Flock()
{
 for (auto& agent:Agents)
 {
 	if (agent) agent->Destroy();
 }
}

void Flock::Tick(float DeltaTime)
{
	FVector pos{FVector::ZeroVector};
	for ( auto agent: Agents)
	{
			RegisterNeighbors(agent); 
			agent->Tick(DeltaTime);
		//trim to the world
		pos = agent->GetActorLocation();
		bool bTrimmed = false;
		if (pos.X >  WorldSize) { pos.X = -WorldSize; bTrimmed = true; }
		if (pos.X < -WorldSize) { pos.X =  WorldSize; bTrimmed = true; }
		if (pos.Y >  WorldSize) { pos.Y = -WorldSize; bTrimmed = true; }
		if (pos.Y < -WorldSize) { pos.Y =  WorldSize; bTrimmed = true; }
		if (bTrimmed) agent->SetActorLocation(pos);
	}
 // TODO: for every agent:
  // TODO: register the neighbors for this agent (-> fill the memory pool with the neighbors for the currently evaluated agent)
  // TODO: update the agent (-> the steeringbehaviors use the neighbors in the memory pool)
  // TODO: trim the agent to the world
}

void Flock::RenderDebug()
{
	if (Agents.IsEmpty()) return;
	constexpr float radiusNeighbours{70.f};
    
	// Re-register specifically for first agent
	RegisterNeighbors(Agents[0]);
    
	FVector pos = Agents[0]->GetActorLocation();
    
	// Draw neighborhood radius for first agent
	DrawDebugCircle(pWorld, pos, NeighborhoodRadius, 32, FColor::Green,
		false, -1.f, 0, 3.f, FVector::ForwardVector, FVector::RightVector);

	// Draw lines to each neighbor of first agent only
	for (int i = 0; i < NrOfNeighbors; ++i)
	{
		DrawDebugLine(
			pWorld,
			pos,
			FVector{Neighbors[i]->GetPosition(),0.f},
			FColor::Cyan, false, -1.f, 0, 1.f);
		DrawDebugCircle(pWorld, FVector{Neighbors[i]->GetPosition(),0.f}, radiusNeighbours, 32, FColor::Green,
		false, -1.f, 0, 3.f, FVector::ForwardVector, FVector::RightVector);
	}
	// Draw world borders
	DrawDebugLine(pWorld, FVector(-WorldSize, -WorldSize, 0), FVector( WorldSize, -WorldSize, 0), FColor::Red, false, -1.f, 0, 5.f);
	DrawDebugLine(pWorld, FVector( WorldSize, -WorldSize, 0), FVector( WorldSize,  WorldSize, 0), FColor::Red, false, -1.f, 0, 5.f);
	DrawDebugLine(pWorld, FVector( WorldSize,  WorldSize, 0), FVector(-WorldSize,  WorldSize, 0), FColor::Red, false, -1.f, 0, 5.f);
	DrawDebugLine(pWorld, FVector(-WorldSize,  WorldSize, 0), FVector(-WorldSize, -WorldSize, 0), FColor::Red, false, -1.f, 0, 5.f);
	
}

void Flock::ImGuiRender(ImVec2 const& WindowPos, ImVec2 const& WindowSize)
{
#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		bool bWindowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Gameplay Programming", &bWindowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: place target");
		ImGui::Text("RMB: move cam.");
		ImGui::Text("Scrollwheel: zoom cam.");
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("Flocking");
		ImGui::Spacing();

        //checkboxes for debug rendering
		ImGui::Checkbox("Show Steering behaviours" ,&DebugRenderSteering);
		ImGui::Checkbox("Show Partitions",&DebugRenderPartitions);

		ImGui::Text("Behavior Weights");
		ImGui::Spacing();

		ImGui::SliderFloat("Cohesion", &CohesionWeight, 0.f, 1.f);
		ImGui::SliderFloat("Separation", &SeparationWeight,0.0f,1.f);
		ImGui::SliderFloat("Velocity Aligment" ,&VelocityMatchWeight,0.f,1.f);
		ImGui::SliderFloat("Seek",&SeekWeight,0.f,1.f);
		ImGui::SliderFloat("Wander",&WanderWeight,0.f,1.f);
		if (pBlendedSteering)
		{
			auto& behaviors = pBlendedSteering->GetWeightedBehaviorsRef();
			behaviors[0].Weight = CohesionWeight;
			behaviors[1].Weight = SeparationWeight;
			behaviors[2].Weight = VelocityMatchWeight;
			behaviors[3].Weight = SeekWeight;
			behaviors[4].Weight = WanderWeight;
		}
		ImGuiHelpers::ImGuiSliderFloatWithSetter("World Size",
	WorldSize, 1000.f, 5000.f,
	[this](float InVal) { WorldSize = InVal; });
		//End
		ImGui::End();
	}
#pragma endregion
#endif
}
#ifndef GAMEAI_USE_SPACE_PARTITIONING
void Flock::RegisterNeighbors(ASteeringAgent* const pAgent)
{
	float distanceSqr{};
	const float neighbourHoodRadiusSqr{NeighborhoodRadius * NeighborhoodRadius}; //avoids squareRoot
	NrOfNeighbors = 0;
	
	for (const auto& agent: Agents)
	{
		if (pAgent != agent)
		{
			distanceSqr = (agent->GetPosition() - pAgent->GetPosition()).SquaredLength();//avoids squareRoot calculation
				
			if(distanceSqr <= neighbourHoodRadiusSqr)
			{
				if (NrOfNeighbors<Neighbors.Num())
				{
					Neighbors[NrOfNeighbors] = agent;
				}
				else
				{
					Neighbors.Add(agent); 
				}
					++NrOfNeighbors;
			}
		}
	}
}
#endif

FVector2D Flock::GetAverageNeighborPos() const
{
	if (NrOfNeighbors == 0) return FVector2D::ZeroVector;
	FVector2D avgPosition {FVector2D::ZeroVector};
	FVector2D totalPosition {FVector2D::ZeroVector};

	for (int i = 0; i < NrOfNeighbors; ++i)
	 {
		 totalPosition+=Neighbors[i]->GetPosition();
	 }
	avgPosition = totalPosition/static_cast<float>(NrOfNeighbors);
	
	return avgPosition;
}

FVector2D Flock::GetAverageNeighborVelocity() const
{
	if (NrOfNeighbors == 0) return FVector2D::ZeroVector;
	FVector2D avgVelocity {FVector2D::ZeroVector};
	FVector2D totalVelocity {FVector2D::ZeroVector};

	for (int i = 0; i < NrOfNeighbors; ++i)
	{
		totalVelocity += FVector2D{Neighbors[i]->GetVelocity()};
	}
	
	avgVelocity = totalVelocity/static_cast<float>(NrOfNeighbors);
	return avgVelocity;
}

void Flock::SetTarget_Seek(FSteeringParams const& Target)
{
	if (pSeekBehavior)
	{
		pSeekBehavior->SetTarget(Target);
		auto& behaviors = pBlendedSteering->GetWeightedBehaviorsRef();
		behaviors[3].Weight = SeekWeight > 0.f ? SeekWeight : 0.1f;
		
	}
}

void Flock::UpdateEvadeTarget()
{
	if (pEvadeBehavior && pAgentToEvade)
		pEvadeBehavior->SetTarget(
			FSteeringParams{FVector2D{pAgentToEvade->GetActorLocation()}});
}

