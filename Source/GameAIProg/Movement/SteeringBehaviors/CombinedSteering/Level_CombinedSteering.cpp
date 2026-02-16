#include "Level_CombinedSteering.h"

#include <format>
#include "CombinedSteeringBehaviors.h"
#include "imgui.h"
#include "MovieSceneTrack.h"


// Sets default values
ALevel_CombinedSteering::ALevel_CombinedSteering()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevel_CombinedSteering::BeginPlay()
{
	Super::BeginPlay();
	AddAgent(BehaviorTypes::Drunk);

}

void ALevel_CombinedSteering::BeginDestroy()
{
	Super::BeginDestroy();

}

bool ALevel_CombinedSteering::AddAgent(BehaviorTypes BehaviorType, bool AutoOrient)
{
	ImGui_Agent ImGuiAgent = {};
	ImGuiAgent.Agent = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, FVector{0,0,90}, FRotator::ZeroRotator);
	if (IsValid(ImGuiAgent.Agent))
	{
		ImGuiAgent.SelectedBehavior = static_cast<int>(BehaviorType);
		ImGuiAgent.SelectedTarget = -1; // Mouse
		
		SetAgentBehavior(ImGuiAgent);

		SteeringAgents.push_back(std::move(ImGuiAgent));
		
		RefreshTargetLabels();

		return true;
	}

	return false;
}

void ALevel_CombinedSteering::RemoveAgent(unsigned int Index)
{
	SteeringAgents[Index].Agent->Destroy();
	SteeringAgents.erase(SteeringAgents.begin() + Index);

	RefreshTargetLabels();
	RefreshAgentTargets(Index);
}

void ALevel_CombinedSteering::SetAgentBehavior(ImGui_Agent& Agent)
{
	Agent.Behavior.reset();
	Agent.SubBehaviors.clear();
	
	//init here so the code doesnt go out of scope in the swicth
	std::vector<BlendedSteering::WeightedBehavior> weightedBehaviors;
	
	switch (static_cast<BehaviorTypes>(Agent.SelectedBehavior))
	{
	case BehaviorTypes::Drunk:
		
		Agent.SubBehaviors.push_back(std::make_unique<Seek>());
		Agent.SubBehaviors.push_back(std::make_unique<Wander>());
		
		weightedBehaviors =
		{
			BlendedSteering::WeightedBehavior(Agent.SubBehaviors[0].get(),0.5f)
			,BlendedSteering::WeightedBehavior(Agent.SubBehaviors[1].get(),0.5f)
		};
		Agent.Behavior = std::make_unique<BlendedSteering>(weightedBehaviors);
		
		break;
	default:
		assert(false); // Incorrect Agent Behavior gotten during SetAgentBehavior()	
		return;
	}

	UpdateTarget(Agent);
	
	Agent.Agent->SetSteeringBehavior(Agent.Behavior.get());
}

void ALevel_CombinedSteering::RefreshTargetLabels()
{
	TargetLabels.clear();
	
	TargetLabels.push_back("Mouse");
	for (int i{0}; i < SteeringAgents.size(); ++i)
	{
		TargetLabels.push_back(std::format("Agent {}", i));
	}
}

void ALevel_CombinedSteering::UpdateTarget(ImGui_Agent& Agent)
{
	if (!Agent.Behavior)
	{
		return; //prevents crash
	}
	bool const bUseMouseAsTarget = Agent.SelectedTarget < 0;
	if (!bUseMouseAsTarget)
	{
		ASteeringAgent* const TargetAgent = SteeringAgents[Agent.SelectedTarget].Agent;

		FTargetData Target;
		Target.Position = TargetAgent->GetPosition();
		Target.Orientation = TargetAgent->GetRotation();
		Target.LinearVelocity = TargetAgent->GetLinearVelocity();
		Target.AngularVelocity = TargetAgent->GetAngularVelocity();

		Agent.Behavior->SetTarget(Target);
	}
	else
	{
		Agent.Behavior->SetTarget(MouseTarget);
	}
}

void ALevel_CombinedSteering::RefreshAgentTargets(unsigned int IndexRemoved)
{
	for (UINT i = 0; i < SteeringAgents.size(); ++i)
	{
		if (i >= IndexRemoved)
		{
			auto& Agent = SteeringAgents[i];
			if (Agent.SelectedTarget == IndexRemoved || i  == Agent.SelectedTarget)
			{
				--Agent.SelectedTarget;
			}
		}
	}
}

// Called every frame
void ALevel_CombinedSteering::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
#pragma region UI
	//UI
	{
		//Setup
		bool windowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Game AI", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	
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
		ImGui::Spacing();
	
		ImGui::Text("Flocking");
		ImGui::Spacing();
		ImGui::Spacing();
	
		if (ImGui::Checkbox("Debug Rendering", &CanDebugRender))
		{
   // TODO: Handle the debug rendering of your agents here :)
		}
		ImGui::Checkbox("Trim World", &TrimWorld->bShouldTrimWorld);
		if (TrimWorld->bShouldTrimWorld)
		{
			ImGuiHelpers::ImGuiSliderFloatWithSetter("Trim Size",
				TrimWorld->GetTrimWorldSize(), 1000.f, 3000.f,
				[this](float InVal) { TrimWorld->SetTrimWorldSize(InVal); });
		}
		
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
	
		ImGui::Text("Behavior Weights");
		ImGui::Spacing();
		if (!SteeringAgents.empty())
		{
			for (auto& Agent : SteeringAgents)
			{
				if (auto* blended = static_cast<BlendedSteering*>(Agent.Behavior.get()))
				{
					auto& weights = blended->GetWeightedBehaviorsRef();
					
					ImGuiHelpers::ImGuiSliderFloatWithSetter("Seek",
		  weights[0].Weight, 0.f, 1.f,
		  [&weights](float InVal) { weights[0].Weight = InVal; }, "%.2f");
					
					ImGuiHelpers::ImGuiSliderFloatWithSetter("Wander",
			weights[1].Weight, 0.f, 1.f,
			[&weights](float InVal) { weights[1].Weight = InVal; }, "%.2f");
				}
			}
		}
		
		// ImGuiHelpers::ImGuiSliderFloatWithSetter("Seek",
		// 	pBlendedSteering->GetWeightedBehaviorsRef()[0].Weight, 0.f, 1.f,
		// 	[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[0].Weight = InVal; }, "%.2f");
		//
		// ImGuiHelpers::ImGuiSliderFloatWithSetter("Wander",
		// pBlendedSteering->GetWeightedBehaviorsRef()[1].Weight, 0.f, 1.f,
		// [this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[1].Weight = InVal; }, "%.2f");
	
		//End
		ImGui::End();
	}
#pragma endregion
	
	// Combined Steering Update
 // TODO: implement handling mouse click input for seek
	if (UseMouseTarget)
	{
		
	}
 // TODO: implement Make sure to also evade the wanderer
	
}

