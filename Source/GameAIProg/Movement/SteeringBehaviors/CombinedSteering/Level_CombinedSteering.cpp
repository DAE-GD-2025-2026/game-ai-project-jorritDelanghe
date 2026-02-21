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
	AddAgent(BehaviorTypes::Evader);
	SteeringAgents[1].SelectedTarget = 0;

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
	Agent.BehaviorNames.clear();
	
	//init here so the code doesnt go out of scope in the swicth
	std::vector<BlendedSteering::WeightedBehavior> weightedBehaviors;
	
	switch (static_cast<BehaviorTypes>(Agent.SelectedBehavior))
	{
	case BehaviorTypes::Drunk:
		
		Agent.SubBehaviors.push_back(std::make_unique<Seek>());
		Agent.SubBehaviors.push_back(std::make_unique<Wander>());
		Agent.BehaviorNames.push_back("Seek");
		Agent.BehaviorNames.push_back("Wander");
		
		weightedBehaviors =
		{
			BlendedSteering::WeightedBehavior(Agent.SubBehaviors[0].get(),0.5f)
			,BlendedSteering::WeightedBehavior(Agent.SubBehaviors[1].get(),0.5f)
		};
		
		Agent.Behavior = std::make_unique<BlendedSteering>(weightedBehaviors);
		
		break;
	case BehaviorTypes::Evader:
		
		Agent.SubBehaviors.push_back(std::make_unique<Evade>());
		Agent.SubBehaviors.push_back(std::make_unique<Wander>());
		Agent.BehaviorNames.push_back("Evade");
		Agent.BehaviorNames.push_back("Wander");
		
		weightedBehaviors =
			{
			BlendedSteering::WeightedBehavior(Agent.SubBehaviors[0].get(),0.5f)
			,BlendedSteering::WeightedBehavior(Agent.SubBehaviors[1].get(),0.5f)
		};
		Agent.Behavior=std::make_unique<BlendedSteering>(weightedBehaviors);
		break;
	case BehaviorTypes::PriorityEvader:
		{
			Agent.SubBehaviors.push_back(std::make_unique<Evade>());
			Agent.SubBehaviors.push_back(std::make_unique<Wander>());
			Agent.BehaviorNames.push_back("Evade (Priority)");
			Agent.BehaviorNames.push_back("Wander (Fallback)");

			// Priority steering takes raw pointers in order: first = highest priority
			std::vector<ISteeringBehavior*> priorityList = {
				Agent.SubBehaviors[0].get(), // Evade first
				Agent.SubBehaviors[1].get()  // Wander as fallback
			};

			Agent.Behavior = std::make_unique<PrioritySteering>(priorityList);
			break;
		}
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
	FTargetData Target;
	if (!bUseMouseAsTarget)
	{
		ASteeringAgent* const TargetAgent = SteeringAgents[Agent.SelectedTarget].Agent;
		
		Target.Position = TargetAgent->GetPosition();
		Target.Orientation = TargetAgent->GetRotation();
		Target.LinearVelocity = TargetAgent->GetLinearVelocity();
		Target.AngularVelocity = TargetAgent->GetAngularVelocity();

		Agent.Behavior->SetTarget(Target);
	}
	else
	{
		Target = MouseTarget;
	}
	for (auto& SubBehavior : Agent.SubBehaviors)
	{
		SubBehavior->SetTarget(Target);
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

void ALevel_CombinedSteering::NormalizeWeights(std::vector<BlendedSteering::WeightedBehavior>& Weights,
	int ChangedIndex)
{
	float changedValue = Weights[ChangedIndex].Weight;
	float remaining = 1.0f - changedValue;
	float otherTotal = 0.f;

	for (int i = 0; i < Weights.size(); ++i)
		if (i != ChangedIndex)
			otherTotal += Weights[i].Weight;

	for (int i = 0; i < Weights.size(); ++i)
	{
		if (i != ChangedIndex)
		{
			if (otherTotal > 0.f)
				Weights[i].Weight = (Weights[i].Weight / otherTotal) * remaining;
			else
				Weights[i].Weight = remaining / (Weights.size() - 1);
		}
	}
}

// Called every frame
void ALevel_CombinedSteering::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
#pragma region UI
	{
		bool windowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Game AI", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		// CONTROLS
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: place target");
		ImGui::Text("RMB: move cam.");
		ImGui::Text("Scrollwheel: zoom cam.");
		ImGui::Unindent();

		ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

		// STATS
		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

		// WORLD SETTINGS
		ImGui::Text("World");
		ImGui::Spacing();
		if (ImGui::Checkbox("Debug Rendering", &CanDebugRender)) {}
		ImGui::Checkbox("Trim World", &TrimWorld->bShouldTrimWorld);
		if (TrimWorld->bShouldTrimWorld)
		{
			ImGuiHelpers::ImGuiSliderFloatWithSetter("Trim Size",
				TrimWorld->GetTrimWorldSize(), 1000.f, 3000.f,
				[this](float InVal) { TrimWorld->SetTrimWorldSize(InVal); });
		}

		ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

		// ADD AGENTS
		ImGui::Text("Agents");
		ImGui::Spacing();
		if (ImGui::Button("Add Drunk"))  AddAgent(BehaviorTypes::Drunk);
		ImGui::SameLine();
		if (ImGui::Button("Add Evader")) AddAgent(BehaviorTypes::Evader);
		ImGui::SameLine();
		if (ImGui::Button("Add Priority Evader")) AddAgent(BehaviorTypes::PriorityEvader);
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		// PER AGENT UI
		for (int i = 0; i < static_cast<int>(SteeringAgents.size()); ++i)
		{
			ImGui::PushID(i);
			ImGui_Agent& A = SteeringAgents[i];

			std::string behaviorName;
			switch (static_cast<BehaviorTypes>(A.SelectedBehavior))
			{
			case BehaviorTypes::Drunk:           behaviorName = "Drunk";            break;
			case BehaviorTypes::Evader:          behaviorName = "Evader";           break;
			case BehaviorTypes::PriorityEvader:  behaviorName = "Priority Evader";  break;
			default:                             behaviorName = "Unknown";          break;
			}
			std::string header = std::format("Agent {} ({})", i, behaviorName);

			if (ImGui::CollapsingHeader(header.c_str()))
			{
				ImGui::Indent();

				// Target selector
				ImGui::Text("Target:");
				ImGui::SameLine();
				ImGui::PushItemWidth(120);
				int selectedTargetOffset = A.SelectedTarget + 1;
				std::string targets{};
				for (auto const& label : TargetLabels)
				{
					targets += label;
					targets += '\0';
				}
				if (ImGui::Combo("##target", &selectedTargetOffset, targets.c_str()))
					A.SelectedTarget = selectedTargetOffset - 1;
				ImGui::PopItemWidth();

				ImGui::Spacing();

				// Behavior weights
				if (auto* blended = dynamic_cast<BlendedSteering*>(A.Behavior.get()))
				{
					auto& weights = blended->GetWeightedBehaviorsRef();

					ImGui::Text("Behavior Weights (sum = 1.0):");
					ImGui::Spacing();

					for (int j = 0; j < static_cast<int>(weights.size()); ++j)
					{
						ImGui::PushID(j);
						float w = weights[j].Weight;
						if (ImGui::SliderFloat(A.BehaviorNames[j].c_str(), &w, 0.f, 1.f, "%.2f"))
						{
							weights[j].Weight = w;
							NormalizeWeights(weights, j);
						}
						ImGui::PopID();
					}

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

					// Add sub-behavior
					ImGui::Text("Add Behavior:");
					ImGui::SameLine();
					ImGui::PushItemWidth(120);
					ImGui::Combo("##addbehavior", &A.SelectedBehaviorToAdd,
						"Seek\0Wander\0Flee\0Arrive\0Evade\0Pursuit\0");
					ImGui::PopItemWidth();
					ImGui::SameLine();

					if (ImGui::Button("Add"))
					{
						std::string newName;
						switch (A.SelectedBehaviorToAdd)
						{
						case 0: A.SubBehaviors.push_back(std::make_unique<Seek>());    newName = "Seek";    break;
						case 1: A.SubBehaviors.push_back(std::make_unique<Wander>());  newName = "Wander";  break;
						case 2: A.SubBehaviors.push_back(std::make_unique<Flee>());    newName = "Flee";    break;
						case 3: A.SubBehaviors.push_back(std::make_unique<Arrive>());  newName = "Arrive";  break;
						case 4: A.SubBehaviors.push_back(std::make_unique<Evade>());   newName = "Evade";   break;
						case 5: A.SubBehaviors.push_back(std::make_unique<Pursuit>()); newName = "Pursuit"; break;
						default: break;
						}

						A.BehaviorNames.push_back(newName);

						// New behavior takes 20%, existing shrink proportionally
						float newWeight = 0.2f;
						float scale = 1.0f - newWeight;
						for (auto& wb : weights)
							wb.Weight *= scale;

						blended->AddBehaviour(
							BlendedSteering::WeightedBehavior(A.SubBehaviors.back().get(), newWeight));
					}
					

					// Remove per-behavior buttons
					ImGui::Spacing();
					int indexToRemoveBehavior = -1;
					for (int j = 0; j < static_cast<int>(weights.size()); ++j)
					{
						ImGui::PushID(j + 200);
						if (weights.size() > 1)
						{
							if (ImGui::Button(std::format("Remove {}", A.BehaviorNames[j]).c_str()))
								indexToRemoveBehavior = j;
						}
						ImGui::PopID();
					}

					if (indexToRemoveBehavior >= 0)
					{
						weights.erase(weights.begin() + indexToRemoveBehavior);
						A.SubBehaviors.erase(A.SubBehaviors.begin() + indexToRemoveBehavior);
						A.BehaviorNames.erase(A.BehaviorNames.begin() + indexToRemoveBehavior);

						// Renormalize after removal
						float total = 0.f;
						for (auto& wb : weights) total += wb.Weight;
						if (total > 0.f)
							for (auto& wb : weights) wb.Weight /= total;
					}
				}
				else if (auto* priority = dynamic_cast<PrioritySteering*>(A.Behavior.get()))
				{
					ImGui::Text("Priority Steering");
					ImGui::TextDisabled("Evade when close, Wander when far");
				}

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
				if (ImGui::Button("Remove Agent"))
					AgentIndexToRemove = i;

				ImGui::Unindent();
			}

			ImGui::PopID();
		}

		if (AgentIndexToRemove >= 0)
		{
			RemoveAgent(AgentIndexToRemove);
			AgentIndexToRemove = -1;
		}

		ImGui::End();
	}
#pragma endregion

	// Update targets every frame
	for (auto& Agent : SteeringAgents)
	{
		if (Agent.Agent && Agent.Behavior)
			UpdateTarget(Agent);
	}
}

