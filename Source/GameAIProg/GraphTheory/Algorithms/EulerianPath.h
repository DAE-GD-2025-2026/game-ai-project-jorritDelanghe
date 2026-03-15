#pragma once
#include <stack>
#include "Shared/Graph/Graph.h"

namespace GameAI
{
	enum class Eulerianity
	{
		notEulerian,
		semiEulerian,
		eulerian,
	};

	class EulerianPath final
	{
	public:
		EulerianPath(Graph* const pGraph);

		Eulerianity IsEulerian() const;
		std::vector<Node*> FindPath(Eulerianity& eulerianity) const;

	private:
		void VisitAllNodesDFS(const std::vector<Node*>& pNodes, std::vector<bool>& visited, int startIndex) const;
		bool IsConnected() const;

		Graph* m_pGraph;
	};

	inline EulerianPath::EulerianPath(Graph* const pGraph)
		: m_pGraph(pGraph)
	{
	}

	inline Eulerianity EulerianPath::IsEulerian() const
	{		
		int numberOfOddDegree{};
		for (const auto& node : m_pGraph->GetActiveNodes())
		{
			auto connections = m_pGraph->FindConnectionsFrom(node->GetId());
			if (connections.size() % 2 != 0)
			{
				++numberOfOddDegree;
			}
		}
		if (!IsConnected()) return Eulerianity::notEulerian;
		if (numberOfOddDegree==0) return Eulerianity::eulerian;
		if (numberOfOddDegree == 2) return Eulerianity::semiEulerian;
		return Eulerianity::notEulerian;
	}

	inline std::vector<Node*> EulerianPath::FindPath(Eulerianity& eulerianity) const
	{
		// Get a copy of the graph because this algorithm involves removing edges
		Graph graphCopy = m_pGraph->Clone();
		std::vector<Node*> Path = {};
		std::vector<Node*> Nodes = graphCopy.GetActiveNodes();
		int currentNodeId{ Graphs::InvalidNodeId };
		Eulerianity eulrerianity = IsEulerian();
		
		if (eulrerianity == Eulerianity::notEulerian) return Path;
		if (eulrerianity == Eulerianity::semiEulerian)
		{
			for (auto* node : Nodes)
			{
				if (m_pGraph->FindConnectionsFrom(node->GetId()).size()%2 != 0)
				{
					currentNodeId = node->GetId();
					break;
				}
			}
		}
		else
		{
			//full euler-doesnt matter the startindex
			currentNodeId = Nodes[0]->GetId();
		}
		//HierHolzer loop
		std::stack<int> nodeStack;
		nodeStack.push(currentNodeId);
		
		while (!nodeStack.empty())
		{
			auto connections = graphCopy.FindConnectionsFrom(currentNodeId);
			if (!connections.empty())
			{
				nodeStack.push(currentNodeId);
				int nextNodeId(connections[0]->GetToId());
				graphCopy.RemoveConnection(currentNodeId,nextNodeId);
				currentNodeId = nextNodeId;
			}
			else
			{
				Path.push_back(m_pGraph->GetNode(currentNodeId).get());
				currentNodeId = nodeStack.top();
				nodeStack.pop();
			}
		}
		

		std::reverse(Path.begin(), Path.end());
		return Path;
	}

	inline void EulerianPath::VisitAllNodesDFS(const std::vector<Node*>& Nodes, std::vector<bool>& visited, int startIndex ) const
	{
		visited[startIndex] = true;
		auto connections = m_pGraph->FindConnectionsFrom(Nodes[startIndex]->GetId());
		for (auto* connection: connections)
		{
			int toId = connection->GetToId();
			for (size_t i{} ; i<std::size(Nodes);++i)
			{
				if (toId == Nodes[i]->GetId() && !visited[i])
				{
					VisitAllNodesDFS(Nodes,visited,i);
				}
			}
		}
	}

	inline bool EulerianPath::IsConnected() const
	{
		std::vector<Node*> Nodes = m_pGraph->GetActiveNodes();
		std::vector<bool> visited{};
		visited.reserve(std::size(Nodes));
			
		for (size_t i{};i<std::size(Nodes);++i)
		{
			visited.emplace_back(false);
		}
		int startIndex{0};
		for (size_t i{};i<std::size(Nodes);++i)
		{
			if (!m_pGraph->FindConnectionsFrom(Nodes[i]->GetId()).empty())
			{
				startIndex = i;
				break;
			}
		}
		
		VisitAllNodesDFS(Nodes,visited,startIndex);
		
		for (bool visit:visited)
		{
			if (visit == false) return false;
		}
		return true;
	}
}