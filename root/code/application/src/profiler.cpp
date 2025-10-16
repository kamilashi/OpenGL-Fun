#include "profiler.h"

namespace Profiler
{
	static ScopeProfileTree profileTree;

	int  ScopeTimer::s_frameNumber = 0;
	bool ScopeTimer::s_paused = false;

	ScopeTimer::ScopeTimer(const char* name) : m_name(name)
	{
		if (s_paused)
		{
			return;
		}
		
		m_startTimePoint = std::chrono::high_resolution_clock::now();

		profileTree.onNodeOpen();
	};

	ScopeTimer::~ScopeTimer()
	{
		if (s_paused)
		{
			return;
		}

		auto endTimePoint = std::chrono::high_resolution_clock::now();

		double start = std::chrono::time_point_cast<std::chrono::microseconds>(m_startTimePoint)
			.time_since_epoch()
			.count();
		start *= 0.001f;

		double end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint)
			.time_since_epoch()
			.count();
		end *= 0.001f;

		double elapsedTime = end - start;

		ScopeProfileData data = ScopeProfileData(m_name, start, elapsedTime);

		profileTree.onNodeClose(&data);
	};

	void InitProfiler()
	{
	}

	void OnStartFrame()
	{
		if (!ScopeTimer::s_paused)
		{
			profileTree.reset();
		}

		ScopeTimer::s_frameNumber++;
	}


	void OnEndFrame()
	{
	}

	void EndProfiler()
	{
		profileTree.reset();
	}

	std::string FormatProfileRowIndented(const ScopeProfileData& data, int depth)
	{
		char buffer[256];
		char indent[64];
		int indentLen = depth * 4;
		indentLen = indentLen < 60 ? indentLen : 60;
		std::memset(indent, '_', indentLen);
		indent[0] = '|';
		indent[indentLen] = '\0';

		std::snprintf(
			buffer, sizeof(buffer),
			"%8llu | %14.6f | %16.3f | %s%-12s",
			static_cast<unsigned long long>(ScopeTimer::s_frameNumber),
			data.elapsedTime,
			data.startTime,
			indent,
			data.name 
		);
		return std::string(buffer);
	}

	void PrintImGuiNode(size_t idx, int depth)
	{
		if (idx == ~0u) return;

		const ScopeProfileNode& node = profileTree.nodes[idx];

		const std::string line = FormatProfileRowIndented(node.data, depth);
		ImGui::TextUnformatted(line.c_str());

		for (size_t child = node.firstChildIdx; child != ~0u; child = profileTree.nodes[child].nextSiblingIdx)
		{
			PrintImGuiNode(child, depth + 1);
		}
	}

	void DrawImGui()
	{
		ImGui::Begin("Profiler");
		ImGui::Checkbox("Pause", &ScopeTimer::s_paused);

		ImGui::TextUnformatted("Frame No | Duration (ms) |  Start time (ms) | Scope ");
		ImGui::Separator();

		if (profileTree.nodes.size() > 1)
		{
			PrintImGuiNode(profileTree.startIdx, 0);
		}

		ImGui::End();
	}
}