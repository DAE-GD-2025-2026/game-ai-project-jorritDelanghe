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
			if (edgeIdx == -1) continue;
			const auto& edge = edges[edgeIdx];
			
			 FVector2D p1{edge.GetP1(NavPoly).X, edge.GetP1(NavPoly).Y};
			 FVector2D p2{edge.GetP2(NavPoly).X, edge.GetP2(NavPoly).Y};
			
			//orientation point 
			const FVector2D dir{pNext->GetPosition()- pCurrent->GetPosition()};
			const FVector2D toP1{p1- pCurrent->GetPosition()};
			float cross = FVector2D::CrossProduct(dir, toP1);
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
		if (Portals.empty()) return Path;
		FVector2D apex {Portals[0].P1}; //start point
		FVector2D rightLeg{FVector2D::ZeroVector};
		FVector2D leftLeg{FVector2D::ZeroVector};
		
		int rightIndex{0}; //portalindex right leg points too
		int leftIndex{0};
		const int amtPortals{static_cast<int>(Portals.size())};
		
		Path.push_back(apex);
		
		for (int portalIdx{1}; portalIdx<amtPortals; ++portalIdx)
		{
			const NavLine& portal {Portals[portalIdx]};
			//right check
			FVector2D newRightLeg{portal.P1-apex};
			const float crossRight{static_cast<float>(FVector2D::CrossProduct(rightLeg,newRightLeg))};
			
			if (crossRight>=0.0f)
			{
				//check cross over left
				const float crossOverLeft{static_cast<float>(FVector2D::CrossProduct(leftLeg,newRightLeg))};
				if (crossOverLeft>0.0f)
				{
					apex+=leftLeg; //move apex to the tip of the left leg
					Path.push_back(apex);
					
					portalIdx = leftIndex+1;
					rightIndex = leftIndex;
					
					if (portalIdx<amtPortals)
					{
						apex = Portals[leftIndex].P1;
						rightLeg = Portals[rightIndex].P1-apex;
						leftLeg = Portals[leftIndex].P2-apex;
						continue;
					}
				}
				else
				{
					rightLeg = newRightLeg;
					rightIndex = portalIdx;
				}
			}
			//left check
			FVector2D newLeftLeg{portal.P2 - apex};
			const float cross {static_cast<float>(FVector2D::CrossProduct(leftLeg,newLeftLeg))};
			
			if (cross<=0.0f)
			{
				const float crossOverRight{static_cast<float>(FVector2D::CrossProduct(rightLeg,newLeftLeg))};
				if (crossOverRight<0.0f)
				{
					apex += rightLeg;
					Path.push_back(apex);
					
					portalIdx = rightIndex+1;
					leftIndex = rightIndex;
					
					if (portalIdx<amtPortals)
					{
						apex = Portals[rightIndex].P2;
						rightLeg = Portals[rightIndex].P1-apex;
						leftLeg = Portals[leftIndex].P2-apex;
						continue;
					}
				}
				else //no cross
				{
					leftLeg = newLeftLeg;
					leftIndex = portalIdx;
				}
			}
		}
		Path.push_back(Portals.back().P1);
		return Path;
	}
private:
	SSFA() {};
	~SSFA() {};
};
}
