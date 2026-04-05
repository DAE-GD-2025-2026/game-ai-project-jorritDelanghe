#include "NavGraph.h"

#include "NavGraphNode.h"

GameAI::NavGraph::NavGraph(std::unique_ptr<TriPolygon> && NavPoly)
	: Graph{false}
	, pNavPoly{std::move(NavPoly)}
{
	CreateNavigationGraph();
}

GameAI::NavGraph::NavGraph(const NavGraph& Other)
	: Graph(false)
{
	Nodes.reserve(Other.Nodes.size());
	for (std::unique_ptr<Node> const & OtherNode : Other.Nodes)
	{
		Nodes.push_back(std::make_unique<NavGraphNode>(*dynamic_cast<NavGraphNode*>(OtherNode.get())));
	}
        
	Connections.reserve(Other.Connections.size());
	for (std::unique_ptr<Connection> const & OtherConnection : Other.Connections)
	{
		Connections.push_back(std::make_unique<Connection>(*OtherConnection.get()));
	}
}

std::unique_ptr<GameAI::NavGraph> GameAI::NavGraph::Clone() const
{
	return std::make_unique<NavGraph>(*this);
}

int GameAI::NavGraph::GetNodeIdFromEdgeIndex(int EdgeIdx) const
{
	if (EdgeIdx >= 0)
	{
		for (auto const & pNode : Nodes)
		{
			if (dynamic_cast<NavGraphNode*>(pNode.get())->GetEdgeIdx() == EdgeIdx)
			{
				return pNode->GetId();
			}
		}
	}
	
	return Graphs::InvalidNodeId;
}

void GameAI::NavGraph::CreateNavigationGraph()
{
	//1. Goes over all the edges of the navigation mesh and creates nodes
 const auto& edges = pNavPoly->GetEdges();
	const auto& triangles = pNavPoly->GetTriangles();
	
	for (size_t i{}; i <edges.size();++i)
	{
		int sharedEdge{0};
		for (const auto& triangle:triangles)
		{
			if (triangle.HasEdge(edges[i])) ++sharedEdge;
		}
		if (sharedEdge == 2) //portal
		{
			const FVector p1{edges[i].GetP1(*pNavPoly)};
			const FVector p2{edges[i].GetP2(*pNavPoly)};
			FVector2D middlePoint{(p1.X + p2.X)/2.f, (p1.Y + p2.Y)/2.f};
			AddNode(std::make_unique<NavGraphNode>(middlePoint,i));
		}
	}
	//2. Creates connections 
		//2 valid nodes -> 1 connection
		//3 valid nodes -> 3 connections
	
	for (const auto& triangle:triangles)
	{
		std::vector<int> validIds{};
		for (const auto& edge: triangle.GetEdges())
		{
			auto edgeIndex = pNavPoly->FindEdgeIndex(edge);
			if (!edgeIndex.has_value()) continue;
			int nodeId{GetNodeIdFromEdgeIndex(edgeIndex.value())};
			if (nodeId != Graphs::InvalidNodeId)
			{
				validIds.push_back(nodeId);
			}
		}
		
		if (validIds.size()==2)
		{
			AddConnection(validIds[0], validIds[1]);
		}
		else if (validIds.size()==3)
		{
			AddConnection(validIds[0], validIds[1]);
			AddConnection(validIds[1], validIds[2]);
			AddConnection(validIds[0], validIds[2]);
		}
	}
	//3. Set the connections cost to the actual distance
	SetConnectionCostsToDistances();
}
