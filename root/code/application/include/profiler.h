#ifndef PROFILER_H
#define PROFILER_H

#include <vector>
#include <chrono>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "buildconfig.h"

namespace Profiler
{
	class ScopeTimer
	{
	public:
		static int s_frameNumber;
		static bool s_paused;
		ScopeTimer(const char* name);

		~ScopeTimer();

	private:
		const char* m_name;

		std::chrono::time_point<std::chrono::steady_clock> m_startTimePoint;
	};

	struct ScopeProfileData
	{
	public:
		const char* name;
		double startTime;
		double elapsedTime;

		ScopeProfileData() {
			std::memset(this, 0, sizeof(*this));
		};

		ScopeProfileData(const char* n, double startT, double elapsedT) : 
			name(n),
			startTime(startT), 
			elapsedTime(elapsedT) {};
	};

	struct ScopeProfileNode
	{
	public:
		ScopeProfileData data;

		size_t parentIdx = ~0u;
		size_t firstChildIdx = ~0u;
		size_t lastChildIdx = ~0u;
		size_t nextSiblingIdx = ~0u;

		ScopeProfileNode() {
			std::memset(this, ~0x0, sizeof(*this));
		};
		ScopeProfileNode(size_t parent) : parentIdx(parent) {};

		void setData(ScopeProfileData* pData)
		{
			std::memcpy(&(this->data), pData, sizeof(ScopeProfileData));
		};
	};

	struct ScopeProfileTree
	{
	public:

		std::vector<ScopeProfileNode> nodes;
		std::vector<size_t>  openNodes;
		const size_t startIdx = 1;

		void reset() 
		{
			nodes.clear();
			openNodes.clear();

			pushNode(~0u); // root
			openNodes.push_back(0);
		}

		void onNodeOpen()
		{
			const size_t parentIdx = openNodes.back();
			const size_t thisIdx = nodes.size();
			pushNode(parentIdx);

			ScopeProfileNode& parentNode = nodes[parentIdx];
			if (parentNode.firstChildIdx == ~0u) {
				parentNode.firstChildIdx = thisIdx;
				parentNode.lastChildIdx = thisIdx;
			}
			else {
				nodes[parentNode.lastChildIdx].nextSiblingIdx = thisIdx;
				parentNode.lastChildIdx = thisIdx;
			}

			openNodes.push_back(thisIdx);
		};
		
		void onNodeClose(ScopeProfileData* pData)
		{
			if (openNodes.size() <= 1) return;
			const size_t lastOpenIdx = openNodes.back();
			nodes[lastOpenIdx].setData(pData);
			openNodes.pop_back();
		};

	private:
		void pushNode(size_t parent)
		{
			ScopeProfileNode newNode(parent);
			nodes.push_back(newNode);
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
	#define PROFILE_SCOPE(nameLiteral)		Profiler::ScopeTimer timer(nameLiteral)
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
