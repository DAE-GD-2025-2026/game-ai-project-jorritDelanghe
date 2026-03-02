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
	, FlockSize{ FlockSize }
	, pAgentToEvade{pAgentToEvade}
{
	Agents.Reserve(FlockSize);
	for (int index{}; index<FlockSize;++index)
	{
		FVector pos {FMath::RandRange(-WorldSize,WorldSize)
						,FMath::RandRange(-WorldSize,WorldSize),0.0f};
		
		ASteeringAgent* agent = pWorld->SpawnActor<ASteeringAgent>(AgentClass,FTransform(pos));
		if (agent) Agents.Add(agent);
	}

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
	for ( auto agent: Agents)
	{
			RegisterNeighbors(agent); 
			agent->Tick(DeltaTime);
		
	}
 // TODO: for every agent:
  // TODO: register the neighbors for this agent (-> fill the memory pool with the neighbors for the currently evaluated agent)
  // TODO: update the agent (-> the steeringbehaviors use the neighbors in the memory pool)
  // TODO: trim the agent to the world
}

void Flock::RenderDebug()
{
 // TODO: Render all the agents in the flock
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

  // TODO: implement ImGUI checkboxes for debug rendering here

		ImGui::Text("Behavior Weights");
		ImGui::Spacing();

  // TODO: implement ImGUI sliders for steering behavior weights here
		//End
		ImGui::End();
	}
#pragma endregion
#endif
}

void Flock::RenderNeighborhood()
{
	constexpr float offset{100.f};
	constexpr float thickness{1.f};
	constexpr float radius{100.f};
 // TODO: Debugrender the neighbors for the first agent in the flock
	for (auto neighbour: Neighbors)
	{
		float agentAngleRad = FMath::DegreesToRadians(neighbour->GetRotation());
		FVector2D forwardDirection{std::cos(agentAngleRad),std::sin(agentAngleRad)};
		
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
}
#ifndef GAMEAI_USE_SPACE_PARTITIONING
void Flock::RegisterNeighbors(ASteeringAgent* const pAgent)
{
	float distance{};
	NrOfNeighbors = 0;
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
	FVector2D avgPosition {FVector2D::ZeroVector};
	FVector2D totalPosition {FVector2D::ZeroVector};

	 for (const auto& neighbour: Neighbors)
	 {
		 totalPosition+=neighbour->GetPosition();
	 }
	avgPosition = totalPosition/NrOfNeighbors;
	
	return avgPosition;
}

FVector2D Flock::GetAverageNeighborVelocity() const
{
	FVector2D avgVelocity {FVector2D::ZeroVector};
	FVector2D totalVelocity {FVector2D::ZeroVector};

	for (const auto& neighbour: Neighbors)
	{
		totalVelocity += FVector2D{neighbour->GetVelocity()};
	}
	
	avgVelocity = totalVelocity/NrOfNeighbors;
	return avgVelocity;
}

void Flock::SetTarget_Seek(FSteeringParams const& Target)
{
	for (const auto& agent: Agents)
	{
		
	}
}

