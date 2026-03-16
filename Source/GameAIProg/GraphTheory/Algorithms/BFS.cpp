#include "BFS.h"

#include <map>
#include <queue>

#include "Shared/Graph/Graph.h"

using namespace GameAI;

BFS::BFS(Graph* const pGraph)
	: pGraph(pGraph)
{
}

// TODO Breath First Search Algorithm searches for a path from the startNode to the destinationNode
std::vector<Node*> BFS::FindPath(Node* const pStartNode, Node* const pDestinationNode) const
{
	std::vector<Node*> path;
	std::queue<Node*> queue;
	std::map<Node*, Node*> nodeMap; //key is the child, value = parent
	Node* currentNode{};
	
	queue.push(pStartNode);
	nodeMap.emplace(pStartNode, nullptr);
	while (!queue.empty())
	{
		currentNode = queue.front();
		queue.pop();
		
		if (currentNode == pDestinationNode)
		{
			break;
		}
		auto connections = pGraph->FindConnectionsFrom(currentNode->GetId());
		
		for (auto* connection: connections)
		{
			Node* neighbourNode = pGraph->GetNode(connection->GetToId()).get();
			if (nodeMap.find(neighbourNode) == nodeMap.end())
			{
				queue.push(neighbourNode);
				nodeMap.emplace(neighbourNode, currentNode);
			}
		}
	}
	if (nodeMap.find(pDestinationNode) != nodeMap.end())
	{
		currentNode = pDestinationNode;
		while (currentNode!= nullptr)
		{
			path.push_back(currentNode);
			currentNode = nodeMap[currentNode];
		}
	}
	std::reverse(path.begin(), path.end());
	return path;
}
