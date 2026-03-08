#include "SpacePartitioning.h"

// --- Cell ---
// ------------
Cell::Cell(float Left, float Bottom, float Width, float Height)
{
	BoundingBox.Min = { Left, Bottom };
	BoundingBox.Max = { BoundingBox.Min.X + Width, BoundingBox.Min.Y + Height };
}

std::vector<FVector2D> Cell::GetRectPoints() const
{
	const float left = BoundingBox.Min.X;
	const float bottom = BoundingBox.Min.Y;
	const float width = BoundingBox.Max.X - BoundingBox.Min.X;
	const float height = BoundingBox.Max.Y - BoundingBox.Min.Y;

	std::vector<FVector2D> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(UWorld* pWorld, float Width, float Height, int Rows, int Cols, int MaxEntities)
	: pWorld{pWorld}
	, SpaceWidth{Width}
	, SpaceHeight{Height}
	, NrOfRows{Rows}
	, NrOfCols{Cols}
	, NrOfNeighbors{0}
{
	Neighbors.SetNum(MaxEntities);
	
	//calculate bounds of a cell
	CellWidth = Width / Cols;
	CellHeight = Height / Rows;

	CellOrigin = {-Width/2.f, -Height/2.f};
	
	for (int row{};row<Rows;++row)
	{
		for (int col {}; col<Cols;++col)
		{
			float left = CellOrigin.X + CellWidth*col;
			float bottom = CellOrigin.Y + CellHeight*row;
			Cells.emplace_back(left, bottom, CellWidth, CellHeight);
		}
	}
}

void CellSpace::AddAgent(ASteeringAgent& Agent)
{
	const int index = PositionToIndex(Agent.GetPosition());
	Cells[index].Agents.push_back(&Agent);
}

void CellSpace::UpdateAgentCell(ASteeringAgent& Agent, const FVector2D& OldPos)
{
	const int oldindex = PositionToIndex(OldPos);
	const int newIndex =PositionToIndex(Agent.GetPosition());
	if (oldindex!= newIndex)
	{
		Cells[oldindex].Agents.remove(&Agent);
		Cells[newIndex].Agents.push_back(&Agent);
	}
}

void CellSpace::RegisterNeighbors(ASteeringAgent& Agent, float QueryRadius)
{
	NrOfNeighbors =0; //reset
	const FVector2D pos = Agent.GetPosition();
	const float radiusSqr{QueryRadius*QueryRadius};
	
	FRect rect{
		{pos.X -QueryRadius, pos.Y-QueryRadius}
		,{pos.X+QueryRadius, pos.Y+QueryRadius}
	};
	for (const auto& cell:Cells)
	{
		if (DoRectsOverlap(rect,cell.BoundingBox)) //skip cell if is not in the QueryRadius
		{
			for (ASteeringAgent* neighborCandidate:cell.Agents) //precise test if in circle
			{
				if (neighborCandidate == &Agent) continue;
				const float distanceSqr =(neighborCandidate->GetPosition() -Agent.GetPosition()).SquaredLength();
				
				if (distanceSqr<radiusSqr)
				{
					Neighbors[NrOfNeighbors] = neighborCandidate;
					++NrOfNeighbors;
				}
			}
			
		}
	}
}

void CellSpace::EmptyCells()
{
	for (Cell& c : Cells)
		c.Agents.clear();
}

void CellSpace::RenderCells() const
{
	//color grid
	 for (int index{} ; index<NrOfCols;++index)
	 {
 		DrawDebugLine(pWorld, 
 			FVector(CellWidth*index, 0.f, 0.f)
 			, FVector( CellWidth*index,SpaceHeight, 0)
 			, FColor::Red, false, -1.f, 0, 5.f);
	 }
	for (int index{} ; index<NrOfRows;++index)
	{
		DrawDebugLine(pWorld, 
			FVector(0.f, CellHeight*index, 0.f)
			, FVector( SpaceWidth,CellHeight*index, 0)
			, FColor::Red, false, -1.f, 0, 5.f);
	}
	//color the used cells blue
	for (const auto& cell: Cells)
	{
		if (!cell.Agents.empty())
		{
			constexpr float thickness{2.f};
			const FVector a{ cell.BoundingBox.Min.X, cell.BoundingBox.Min.Y, 0.f };
			const FVector b{ cell.BoundingBox.Max.X, cell.BoundingBox.Min.Y, 0.f };
			const FVector c{ cell.BoundingBox.Max.X, cell.BoundingBox.Max.Y, 0.f };
			const FVector d{ cell.BoundingBox.Min.X, cell.BoundingBox.Max.Y, 0.f };

			DrawDebugLine(pWorld, a, b,  FColor::Green, false, -1.f, 0, thickness);
			DrawDebugLine(pWorld, b, c,  FColor::Green, false, -1.f, 0, thickness);
			DrawDebugLine(pWorld, c, d,  FColor::Green, false, -1.f, 0, thickness);
			DrawDebugLine(pWorld, d, a,  FColor::Green, false, -1.f, 0, thickness);
		}
	}
}

int CellSpace::PositionToIndex(FVector2D const & Pos) const
{
	 int col = static_cast<int>((Pos.X - CellOrigin.X)/CellWidth);
	 int row = static_cast<int>((Pos.Y - CellOrigin.Y)/CellHeight);
	
	//agents on the edge
	col = FMath::Clamp(col, 0, NrOfCols - 1);
	row = FMath::Clamp(row, 0, NrOfRows - 1);
	return row*NrOfCols+col;
}

bool CellSpace::DoRectsOverlap(FRect const & RectA, FRect const & RectB)
{
	// Check if the rectangles are separated on either axis
	if (RectA.Max.X < RectB.Min.X || RectA.Min.X > RectB.Max.X) return false;
	if (RectA.Max.Y < RectB.Min.Y || RectA.Min.Y > RectB.Max.Y) return false;
    
	// If they are not separated, they must overlap
	return true;
}