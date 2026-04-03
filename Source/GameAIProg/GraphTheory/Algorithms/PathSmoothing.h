#pragma once
#include <vector>

#include "NavGraphPathfinding.h"
#include "Movement/Pathfinding/Navmesh/TriPolygon.h"
#include "Shared/Graph/Graph.h"
#include "Shared/Graph/NavGraph/NavGraphNode.h"
#include <algorithm>

namespace GameAI
{
	class SSFA final
{
public:
	//=== SSFA Functions ===
	//--- References ---
	//http://digestingduck.blogspot.be/2010/03/simple-stupid-funnel-algorithm.html
	//https://gamedev.stackexchange.com/questions/68302/how-does-the-simple-stupid-funnel-algorithm-work
	static std::vector<NavLine> FindPortals(std::vector<Node*> const & Path, TriPolygon const & NavPoly)
	{
		//Container
		std::vector<NavLine> Portals = {};
		if (Path.size() <2) return Portals;
		
		//degenerate portal
		FVector2D startPos = Path.front()->GetPosition();
		Portals.push_back(NavLine{startPos, startPos});
		
		for (int i{};i<static_cast<int>(Path.size())-1;++i) //-1 so not out of bounds
		{
			NavGraphNode* pCurrent{static_cast<NavGraphNode*>(Path[i])};
			NavGraphNode* pNext{static_cast<NavGraphNode*>(Path[i+1])};
			
			//portal is the edge the current graph sits on
			const auto& edges = NavPoly.GetEdges();
			const int edgeIdx = pCurrent->GetEdgeIdx();
			const auto& edge = edges[edgeIdx];
			
			 FVector2D p1{edge.GetP1(NavPoly).X, edge.GetP1(NavPoly).Y};
			 FVector2D p2{edge.GetP2(NavPoly).X, edge.GetP2(NavPoly).Y};
			
			//orientation point 
			float cross = FVector2D::CrossProduct(p1, p2);
			if (cross>0.0f)
			{
				std::swap(p1,p2);
			}
			
			Portals.push_back(NavLine{p1,p2});
		}
		FVector2D endPos = Path.back()->GetPosition();
		Portals.push_back(NavLine{endPos, endPos});
		
		return Portals;
	}

	static std::vector<FVector2D> OptimizePortals( std::vector<NavLine> const & Portals, TriPolygon const & NavPoly)
	{
		std::vector<FVector2D> Path{};
		//P1 == right point of portal, P2 == left point of portal
		
			//--- RIGHT CHECK ---
			//1. See if moving funnel inwards - RIGHT
			
				//2. See if new line degenerates a line segment - RIGHT
				
					//Leftleg becomes new apex point

					//Calculate new legs (if not the end)


			//--- LEFT CHECK ---
			//1. See if moving funnel inwards - LEFT

				//2. See if new line degenerates a line segment - LEFT

					//Rightleg becomes new apex point

					//Calculate new legs (if not the end)


		// Add last path point

		return Path;
	}
private:
	SSFA() {};
	~SSFA() {};
};
}
