#include "NavGraphPathfinding.h"

#include "AStar.h"
#include "PathSmoothing.h"
#include "VectorTypes.h"
#include "Shared/Graph/NavGraph/NavGraph.h"
#include "Shared/Graph/NavGraph/NavGraphNode.h"


using namespace GameAI;

std::vector<FVector2D> NavMeshPathfinding::FindPath(const FVector2D& startPos, const FVector2D& endPos,
	NavGraph* const pNavGraph, std::vector<FVector2D>& debugNodePositions, std::vector<NavLine>& debugPortals) 
{
	//path to return
	std::vector<FVector2D> finalPath{};

	//start and endTriangle
	const TriPolygon* pNavPoly {pNavGraph->GetNavPolygon()};
	
	//strict 
    const TriPolygon::Triangle* pStartTri {pNavPoly->GetTriangleAtPosition(startPos, false)};
    const TriPolygon::Triangle* pEndTri {pNavPoly->GetTriangleAtPosition(endPos,false)};
	
	//backup if point on the line
	if (pStartTri == nullptr) pStartTri = pNavPoly->GetTriangleAtPosition(startPos,true);
	if (pEndTri == nullptr) pEndTri = pNavPoly->GetTriangleAtPosition(endPos,true);
	
	if (pStartTri ==  nullptr|| pEndTri == nullptr) return{}; //outside navMesh
	
	if (pStartTri == pEndTri)
	{
		finalPath = {startPos, endPos};
		debugNodePositions ={startPos, endPos};
		return finalPath;
	}
	
	std::unique_ptr<NavGraph> pTempGraphUniquePtr = pNavGraph->Clone();
	NavGraph* pClonedGraph = pTempGraphUniquePtr.get(); //avoids deletion
	
	NavGraphNode* pStartNode = new NavGraphNode(startPos,-1);
	pClonedGraph->AddNode(std::unique_ptr<Node>(pStartNode));
	
	for (const auto& edge: pStartTri->GetEdges())
	{
		//edge in the polygon
		const auto edgeIdx{pNavPoly->FindEdgeIndex(edge)};
		if (!edgeIdx.has_value()) continue;
		
		//node on that edge
		const int neighbourNodeId{pClonedGraph->GetNodeIdFromEdgeIndex(edgeIdx.value())};
		if (neighbourNodeId == -1) continue;
		
		const Node* pNeighbourNode = pClonedGraph->GetNode(neighbourNodeId).get();
		float cost = FVector2D::Distance(pNeighbourNode->GetPosition(), pStartNode->GetPosition());
		
		auto pConnection = std::make_unique<Connection>(
	pStartNode->GetId(),
	neighbourNodeId);
		
		pConnection->SetWeight(cost);
		pClonedGraph->AddConnection(
				std::move(pConnection)
			);
	}
	NavGraphNode* pEndNode = new NavGraphNode(endPos, -1);
	pClonedGraph->AddNode(std::unique_ptr<Node>(pEndNode));

	for (const auto& edge : pEndTri->GetEdges())
	{
		auto edgeIdx = pNavPoly->FindEdgeIndex(edge);
		if (!edgeIdx.has_value()) continue;

		int neighborNodeId = pClonedGraph->GetNodeIdFromEdgeIndex(edgeIdx.value());
		if (neighborNodeId == -1) continue;

		Node* pNeighbor = pClonedGraph->GetNode(neighborNodeId).get();
		float cost = FVector2D::Distance(endPos, pNeighbor->GetPosition());
		auto pConnection = std::make_unique<Connection>(
	pEndNode->GetId(),
		neighborNodeId);
		
		pConnection->SetWeight(cost);
		pClonedGraph->AddConnection(
				std::move(pConnection)
			);
	}
	
	//A star on new graph
	std::vector<Node*> pathNodes{};
	AStar aStar{pClonedGraph,HeuristicFunctions::Euclidean};
	aStar.FindPath(pStartNode,pEndNode);

	//Debug Visualisation
	debugNodePositions.clear();
	for (Node* pNode:pathNodes)
	{
		debugNodePositions.push_back(pNode->GetPosition());
	}

	debugPortals = SSFA::FindPortals(pathNodes, *pNavGraph->GetNavPolygon());
	finalPath = SSFA::OptimizePortals(debugPortals, *pNavGraph->GetNavPolygon());
	
	return finalPath;
}

std::vector<FVector2D> NavMeshPathfinding::FindPath(const FVector2D& startPos, const FVector2D& endPos, NavGraph* const pNavGraph)
{
	std::vector<FVector2D> debugNodePositions{};
	std::vector<NavLine> debugPortals{};

	return FindPath(startPos, endPos, pNavGraph, debugNodePositions, debugPortals);
}