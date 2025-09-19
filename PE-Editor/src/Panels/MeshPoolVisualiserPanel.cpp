#include "MeshPoolVisualiserPanel.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

#include <implot.h>
#include <implot_internal.h>

namespace PaulEngine
{
    struct Interval { float start, end; };
    float GetAllocationIntervals(const FreeList& freeList, const uint32_t capacity, std::vector<Interval>& out_intervals)
    {
        float occupiedSpace = 0.0f;
        uint32_t currentHead = 0;
        for (const FreeBlock& block : freeList.GetList())
        {
            Interval interval;
            interval.start = (currentHead == 0) ? 0 : ((float)currentHead / (float)capacity) * 100.0f;
            interval.end = (block.StartIndex == 0) ? 0 : ((float)block.StartIndex / (float)capacity) * 100.0f;
            out_intervals.push_back(interval);
            occupiedSpace += interval.end - interval.start;
            currentHead = block.StartIndex + block.Count;
        }
        return occupiedSpace;
    }

    void PlotIntervals(const std::vector<Interval>& intervals, const double xMin, const double xMax, const char* title_id, const char* xLabel = nullptr, const ImVec2& size = ImVec2(-1, 0), ImPlotFlags flags = 0, const ImU32 blockColour = IM_COL32(100, 200, 255, 255), const float blockRounding = 0.0f)
    {
        ImPlot::SetNextAxesLimits(xMin, xMax, 0.0, 1.0, ImGuiCond_Always);

        if (ImPlot::BeginPlot(title_id, size, flags))
        {
            ImPlot::SetupAxis(ImAxis_X1, xLabel);
            ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoDecorations);

            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImPlot::PushPlotClipRect();

            for (const Interval& interval : intervals)
            {
                ImVec2 p1 = ImPlot::PlotToPixels(ImPlotPoint(interval.start, 0));
                ImVec2 p2 = ImPlot::PlotToPixels(ImPlotPoint(interval.end, 1));

                drawList->AddRectFilled(p1, p2, blockColour, blockRounding);
            }

            ImPlot::PopPlotClipRect();

            ImPlot::EndPlot();
        }
    }

	void MeshPoolVisualiserPanel::OnImGuiRender()
	{
        const VertexArrayPool& meshPool = Mesh::GetMasterVertexArrayPool();

        const FreeList& vertexFreeList = meshPool.GetVertexFreeList();
        uint32_t vertexCountCapacity = meshPool.VertexCountCapcity();
        std::vector<Interval> allocatedVertexMemory;
        float vertexPercentAllocated = GetAllocationIntervals(vertexFreeList, vertexCountCapacity, allocatedVertexMemory);

        const FreeList& indexFreeList = meshPool.GetIndexFreeList();
        uint32_t indexCountCapacity = meshPool.IndexCountCapacity();
        std::vector<Interval> allocatedIndexMemory;
        float indexPercentAllocated = GetAllocationIntervals(indexFreeList, indexCountCapacity, allocatedIndexMemory);

		ImGui::Begin("Mesh Pool Visualiser");

        PlotIntervals(allocatedVertexMemory, 0.0, 100.0, "Vertex Buffer", "Percentage", ImVec2(-1, 150));
        ImGui::Text("Total in use: %4.2f%%", vertexPercentAllocated);
        ImGui::Text("Allocated blocks: %d", allocatedVertexMemory.size());

        ImGui::Spacing();

        PlotIntervals(allocatedIndexMemory, 0.0, 100.0, "Index Buffer", "Percentage", ImVec2(-1, 150));
        ImGui::Text("Total in use: %4.2f%%", indexPercentAllocated);
        ImGui::Text("Allocated blocks: %d", allocatedIndexMemory.size());

		ImGui::End();
	}
}