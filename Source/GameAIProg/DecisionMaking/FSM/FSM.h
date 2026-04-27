#pragma once
#include <functional>

#include "BehaviorTree/BlackboardComponent.h"
#include <memory>
#include <string>
class UBlackboardComponent;
namespace GameAI::FSM
{
class State //interface
{
public:
	virtual ~State() = default;
	
	virtual void Update(float deltaTime,UBlackboardComponent * blackBoard);
	virtual void OnEnter(UBlackboardComponent * blackBoard);
	virtual void OnExit(UBlackboardComponent * blackBoard);
	
	//for debugging
	virtual std::string GetName() const; 
};
struct Transition // Links a From state to a To state using a condition
{
	State* From; // state it comes from
	State* To; //state it goes too
	std::function<bool()> Condition; //lambda to check
};
class FSM
{
public:
	explicit FSM() = default;
	
	void AddState(std::unique_ptr<State>&& NewState)
	{
		if (!CurrentState)
		{
			CurrentState = NewState.get();
		}
		States.push_back(std::move(NewState));
		
	}
	void AddTransition(State* From,State* To, std::function<bool()> Condition)
	{
		Transitions.push_back({From,To , std::move(Condition)});
	}
	void Update(float DeltaTime, UBlackboardComponent * BlackBoard)
	{
		if (!CurrentState) return;
		
		for (const auto& transition : Transitions)
		{
			if (transition.From == CurrentState && transition.Condition)
			{
				ChangeState(transition.To, BlackBoard);
				break;
			}
		}
		
		//update 
		if (CurrentState)
		{
			CurrentState->Update(DeltaTime, BlackBoard);
		}
	}
	void ChangeState(State* NewState, UBlackboardComponent * BlackBoard)
	{
		if (CurrentState) CurrentState->OnExit(BlackBoard);
		
		CurrentState = NewState;
		
		if (CurrentState) CurrentState->OnEnter(BlackBoard);
	}
	void Start(UBlackboardComponent * BlackBoard) const // calls OnEnter on the initial state
	{
		if (CurrentState) CurrentState->OnEnter(BlackBoard);
	}
	void Stop(UBlackboardComponent * BlackBoard) const
	{
		if (CurrentState) CurrentState->OnExit(BlackBoard);
	}
	
	State* GetCurrentState() const {return CurrentState;}
private:
	std::vector<std::unique_ptr<State>> States; //owns all states
	std::vector<Transition> Transitions;
	State* CurrentState{nullptr};
	
};
	
}

