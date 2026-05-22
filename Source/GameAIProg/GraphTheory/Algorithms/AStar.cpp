#include "AStar.h"

#include <queue>

using namespace GameAI;

AStar::AStar(Graph* const pGraph, HeuristicFunctions::Heuristic hFunction)
	: pGraph(pGraph)
	, HeuristicFunction(hFunction)
{
}

std::vector<Node*>AStar::FindPath(Node* const pStartNode, Node* const pGoalNode)
{
	std::vector<Node*> path{};
	NodeRecord startRecord
	{pStartNode
		,nullptr
		,0.f
		,GetHeuristicCost(pStartNode, pGoalNode)
	};
	
    std::vector<NodeRecord> openList{};
	std::vector<NodeRecord> closedList{};
	
	openList.push_back(startRecord);
	NodeRecord currentRecord{};
	
	NodeRecord closestNodeRecord{};
	float closestHeuristicCost{std::numeric_limits<float>::max()};
	bool bFoundAnyClosedNode{false};
	
	while (!openList.empty())
	{
		currentRecord = *std::min_element(openList.begin(), openList.end());
	
		if (currentRecord.pNode == pGoalNode)
		{
			break;
		}		
		auto connections = pGraph->FindConnectionsFrom(currentRecord.pNode->GetId());
		for (auto* connection : connections)
		{
			Node* pNextNode = pGraph->GetNode(connection->GetToId()).get();
			const float newG{currentRecord.costSoFar + connection->GetWeight()};
			
			auto closedIt= std::find_if(
				closedList.begin(),closedList.end(),
				[pNextNode](const NodeRecord& closedRecord)
				{
					return pNextNode == closedRecord.pNode;
				});
			if (closedIt != closedList.end())
			{
				if (closedIt->costSoFar<=newG)
				{
					continue; //existing record is cheaper, skip
				}
				closedList.erase(closedIt);
			}
			
			auto openIt = std::find_if(
				openList.begin(), openList.end()
				,[pNextNode](const NodeRecord& openRecord)
				{
					return pNextNode == openRecord.pNode;
				});
			if (openIt != openList.end())
			{
				if (openIt->costSoFar<=newG)
				{
					continue;
				}
				openList.erase(openIt);
			}
			NodeRecord newRecord{
				pNextNode
				,connection
				,newG
				, newG+ GetHeuristicCost(pNextNode,pGoalNode)
				
			};
			openList.push_back(newRecord);
		}
			openList.erase(std::find(
				openList.begin(), 
				openList.end(), 
				currentRecord));
			closedList.push_back(currentRecord);
		
		
		//checks distance between estimated end point and current record
		if (currentRecord.pNode != pStartNode)
		{
			const float currentHeuristicCost{GetHeuristicCost(currentRecord.pNode, pGoalNode)};
			if (currentHeuristicCost <= closestHeuristicCost)
			{
				closestHeuristicCost = currentHeuristicCost;
				closestNodeRecord = currentRecord;
				bFoundAnyClosedNode = true;
			}
		}
	}
		//fall back node
		if (currentRecord.pNode != pGoalNode)
		{
			if (!bFoundAnyClosedNode)
			{
				return path;
			}
			currentRecord = closestNodeRecord;
		}
		while (currentRecord.pNode != pStartNode)
		{
			
			path.push_back(currentRecord.pNode);
			int fromId = currentRecord.pConnection->GetFromId();
			Node* pFromNode = pGraph->GetNode(fromId).get();
			currentRecord =  *std::find_if(
				closedList.begin()
				,closedList.end()
				,[pFromNode](const NodeRecord& closedRecord)
				{
					return pFromNode == closedRecord.pNode;
				});
		}
		path.push_back(pStartNode);
		std::reverse(path.begin(), path.end());
	
	return path;
}

float AStar::GetHeuristicCost(Node* const pStartNode, Node* const pEndNode) const
{
	FVector2D toDestination = pGraph->GetNode(pEndNode->GetId())->GetPosition() - pGraph->GetNode(pStartNode->GetId())->GetPosition();
	return HeuristicFunction(abs(toDestination.X), abs(toDestination.Y));
}