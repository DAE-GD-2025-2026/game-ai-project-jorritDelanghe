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
		
		if (ASteeringAgent* agent = pWorld->SpawnActor<ASteeringAgent>(AgentClass,FTransform(pos)))
		Agents.Add(agent);
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
		if (pos.X >  WorldSize) pos.X = -WorldSize;
		if (pos.X < -WorldSize) pos.X =  WorldSize;
		if (pos.Y >  WorldSize) pos.Y = -WorldSize;
		if (pos.Y < -WorldSize) pos.Y =  WorldSize;
		agent->SetActorLocation(pos);
	}
 // TODO: for every agent:
  // TODO: register the neighbors for this agent (-> fill the memory pool with the neighbors for the currently evaluated agent)
  // TODO: update the agent (-> the steeringbehaviors use the neighbors in the memory pool)
  // TODO: trim the agent to the world
}

void Flock::RenderDebug()
{
	constexpr float offset{50.f};
	constexpr float thickness{1.f};
	constexpr float radius{100.f};
	// TODO: Debugrender the neighbors for the first agent in the flock\
	
	if (Neighbors.IsEmpty()) return;
	//render neighbours
	for (auto neighbour: Neighbors)
	{
		float agentAngleRad = FMath::DegreesToRadians(neighbour->GetRotation());
		FVector2D forwardDirection{std::cos(agentAngleRad),std::sin(agentAngleRad)};
		
		if (neighbour == Neighbors[0]) continue;
		DrawDebugLine(
			neighbour->GetWorld()
			,FVector{neighbour->GetPosition(),0.f}
			,FVector{neighbour->GetPosition() + forwardDirection* offset,0.f}
			,FColor::Cyan,false,0,thickness);
		
		DrawDebugCircle(
			neighbour->GetWorld()
			,FVector{neighbour->GetPosition(),0.f}
			,radius
			,100,FColor::Green,false,-1,0,1.f
			,FVector(1, 0, 0), FVector(0, 1, 0), true);

	}
	//draw neighbor circle
	FVector pos = Agents[0]->GetActorLocation();
	DrawDebugCircle(pWorld, pos, NeighborhoodRadius, 32, FColor::Green,
		false, -1.f, 0, 3.f, FVector::ForwardVector, FVector::RightVector);
	//draw first agent circle
	DrawDebugCircle(pWorld, pos, radius, 32, FColor::Green,
		false, -1.f, 0, 3.f, FVector::ForwardVector, FVector::RightVector);
	
	
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
		//End
		ImGui::End();
	}
#pragma endregion
#endif
}
#ifndef GAMEAI_USE_SPACE_PARTITIONING
void Flock::RegisterNeighbors(ASteeringAgent* const pAgent)
{
	float distance{};
	NrOfNeighbors = 0;
	 Neighbors.Reset(); //avoids reallocation
	
	for (const auto& agent: Agents)
	{
		if (pAgent != agent)
		{
			distance = (agent->GetPosition() - pAgent->GetPosition()).Length();
				
			if(distance <= NeighborhoodRadius)
			{
				Neighbors.Add(agent);
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

	for (int i = 0; i < Neighbors.Num(); ++i)
	 {
		 totalPosition+=Neighbors[i]->GetPosition();
	 }
	avgPosition = totalPosition/static_cast<float>(Neighbors.Num());
	
	return avgPosition;
}

FVector2D Flock::GetAverageNeighborVelocity() const
{
	if (NrOfNeighbors == 0) return FVector2D::ZeroVector;
	FVector2D avgVelocity {FVector2D::ZeroVector};
	FVector2D totalVelocity {FVector2D::ZeroVector};

	for (int i = 0; i < Neighbors.Num(); ++i)
	{
		totalVelocity += FVector2D{Neighbors[i]->GetVelocity()};
	}
	
	avgVelocity = totalVelocity/static_cast<float>(Neighbors.Num());
	return avgVelocity;
}

void Flock::SetTarget_Seek(FSteeringParams const& Target)
{
	if (pSeekBehavior) pSeekBehavior->SetTarget(Target);
}

void Flock::UpdateEvadeTarget()
{
	if (pEvadeBehavior && pAgentToEvade)
		pEvadeBehavior->SetTarget(
			FSteeringParams{FVector2D{pAgentToEvade->GetActorLocation()}});
}

