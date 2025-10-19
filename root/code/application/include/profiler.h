#ifndef PROFILER_H
#define PROFILER_H

#include <vector>
#include <unordered_map>
#include <chrono>
#include <algorithm> 
#include <deque>
#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "buildconfig.h"
#include "helpers.h"

namespace Profiler
{
	class ScopeTimer
	{
	public:
		static int s_frameNumber;
		static bool s_paused;

		ScopeTimer(int id, const char* name);

		~ScopeTimer();

	private:
		int m_id;
		const char* m_name;

		std::chrono::time_point<std::chrono::steady_clock> m_startTimePoint;
	};

	struct ScopeProfileData
	{
	public:
		const char* name;
		double startTime;
		double elapsedTime;

		ScopeProfileData() : 
			name (""),
			startTime(0.0),
			elapsedTime(0.0)
		{};

		ScopeProfileData(const char* n, double startT, double elapsedT) : 
			name(n),
			startTime(startT), 
			elapsedTime(elapsedT) {};
	};

	struct ScopeStats
	{
	public:
		double minElapsedTime;
		double maxElapsedTime;
		static const size_t movingAverageWindowSize = 5 * 60;

		ScopeStats() :
			minElapsedTime(FLT_MAX),
			maxElapsedTime(0.0f),
			m_movingAveHeadIdx(0),
			m_movingAveWindow{}
		{};

		ScopeStats(double minTime, double maxTime) :
			minElapsedTime(minTime),
			maxElapsedTime(maxTime), 
			m_movingAveHeadIdx(0),
			m_movingAveWindow{}
		{};

		void pushElapsedTime(double time)
		{
			m_movingAveWindow[m_movingAveHeadIdx] = time;
			m_movingAveHeadIdx = m_movingAveHeadIdx == movingAverageWindowSize - 1 ? 0 : m_movingAveHeadIdx + 1;
		}

		double getMovingAverage() const
		{
			double ave = 0;
			for (int i = 0; i < movingAverageWindowSize; i++)
			{
				ave += m_movingAveWindow[i];
			}
			ave /= movingAverageWindowSize;
			return ave;
		}

	private:
		double m_movingAveWindow[movingAverageWindowSize];
		size_t m_movingAveHeadIdx;
	};

	struct ScopeProfileNode
	{
	public:
		ScopeProfileData data;

		int id;

		size_t parentIdx;
		size_t firstChildIdx;
		size_t lastChildIdx;
		size_t nextSiblingIdx;

		ScopeProfileNode() : 
			id(-1),
			parentIdx(~0u),
			firstChildIdx(~0u),
			lastChildIdx(~0u),
			nextSiblingIdx(~0u)
		{};

		ScopeProfileNode(int id, size_t parent) : 
			id(id), 
			parentIdx(parent),
			firstChildIdx(~0u),
			lastChildIdx(~0u),
			nextSiblingIdx(~0u)
		{};

		void setData(ScopeProfileData* pData)
		{
			std::memcpy(&(this->data), pData, sizeof(ScopeProfileData));
		};
	};
	 
	struct ScopeProfileTree
	{
	public:
		const size_t startIdx = 1;

		std::vector<ScopeProfileNode> nodes;
		std::vector<size_t>  openNodes;

		std::unordered_map<int, ScopeStats> stats;

		void reset() 
		{
			nodes.clear();
			openNodes.clear();

			pushNode(~0u, ~0u); // root
			openNodes.emplace_back(0);
		}

		void onNodeOpen(int id)
		{
			const size_t parentIdx = openNodes.back();
			const size_t thisIdx = nodes.size();
			pushNode(parentIdx, id);

			ScopeProfileNode& parentNode = nodes[parentIdx];
			if (parentNode.firstChildIdx == ~0u) {
				parentNode.firstChildIdx = thisIdx;
				parentNode.lastChildIdx = thisIdx;
			}
			else {
				nodes[parentNode.lastChildIdx].nextSiblingIdx = thisIdx;
				parentNode.lastChildIdx = thisIdx;
			}

			openNodes.emplace_back(thisIdx);
		};
		
		void onNodeClose(ScopeProfileData* pData)
		{
			if (openNodes.size() <= 1) return;
			const size_t lastOpenIdx = openNodes.back();
			nodes[lastOpenIdx].setData(pData);
			openNodes.pop_back();

			storeStats(pData, nodes[lastOpenIdx].id);
		};

	private:
		void pushNode(size_t parent, int id)
		{
			nodes.emplace_back(id, parent);
		};

		void storeStats(ScopeProfileData* pData, int id)
		{
			auto it = stats.find(id);

			if (it != stats.end())
			{
				ScopeStats& nodeStats = it->second;
				nodeStats.maxElapsedTime = std::max(nodeStats.maxElapsedTime, pData->elapsedTime);
				nodeStats.minElapsedTime = std::min(nodeStats.minElapsedTime, pData->elapsedTime);
				nodeStats.pushElapsedTime(pData->elapsedTime);
			}
			else
			{
				stats.emplace(id, ScopeStats{ pData->elapsedTime, pData->elapsedTime });
			}
		};
	};

	void InitProfiler();
	void OnStartFrame();
	void OnEndFrame();
	void EndProfiler();

	void DrawImGui();
}

#endif // PROFILER_H

#ifndef PROFILER_MACROS_DEFINED
#define PROFILER_MACROS_DEFINED

#if defined (PROFILE)							
	#define PROFILE_SCOPE(nameLiteral)		Profiler::ScopeTimer timer(FILELINE_ID, nameLiteral)
	#define PROFILER_INIT() 				Profiler::InitProfiler()				
	#define PROFILER_START_FRAME() 			Profiler::OnStartFrame()
	#define PROFILER_END_FRAME() 			Profiler::OnEndFrame()
	#define PROFILER_END() 					Profiler::EndProfiler()							
#else
	#define PROFILER_INIT()					do {} while (0)
	#define PROFILE_SCOPE(nameLiteral)		do {} while (0)
	#define PROFILER_END_FRAME()			do {} while (0)
	#define PROFILER_END()					do {} while (0)	
#endif // PROFILE

#endif // PROFILER_MACROS_DEFINED
