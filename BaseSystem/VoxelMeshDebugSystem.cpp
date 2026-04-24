#pragma once

#include "Host/PlatformInput.h"
#include <chrono>
#include <algorithm>
#include <iostream>
#include <vector>

namespace RenderInitSystemLogic {
    int getRegistryInt(const BaseSystem& baseSystem, const std::string& key, int fallback);
    bool getRegistryBool(const BaseSystem& baseSystem, const std::string& key, bool fallback);
}
namespace VoxelMeshingSystemLogic {
}
namespace TerrainSystemLogic {
    void GetVoxelStreamingPerfStats(size_t& pending,
                                    size_t& desired,
                                    size_t& generated,
                                    size_t& jobs,
                                    int& stepped,
                                    int& built,
                                    int& consumed,
                                    int& skippedExisting,
                                    int& filteredOut,
                                    int& rescueSurfaceQueued,
                                    int& rescueMissingQueued,
                                    int& droppedByCap,
                                    int& reprioritized,
                                    float& prepMs,
                                    float& generationMs,
                                    float& desiredMs,
                                    float& baseGenMs,
                                    float& featureMs,
                                    float& surfaceMs,
                                    float& caveFieldMs,
                                    uint64_t& caveFieldCellsBuilt,
                                    uint64_t& caveSamples);
}
namespace TreeGenerationSystemLogic {
    void GetTreeFoliagePerfStats(size_t& pendingSections,
                                 size_t& pendingDependencies,
                                 size_t& backfillVisited,
                                 int& selectedSections,
                                 int& processedSections,
                                 int& deferredByTimeBudget,
                                 int& backfillAppended,
                                 bool& backfillRan,
                                 float& updateMs);
}

namespace VoxelMeshDebugSystemLogic {
    void UpdateVoxelMeshDebug(BaseSystem& baseSystem, std::vector<Entity>&, float, PlatformWindowHandle) {
        if (!baseSystem.voxelWorld) return;
        VoxelWorldContext& voxelWorld = *baseSystem.voxelWorld;

        static auto lastPerfLog = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        const bool voxelPerfLogEnabled = ::RenderInitSystemLogic::getRegistryBool(
            baseSystem, "voxelPerfLogEnabled", true
        );
        const int voxelPerfLogIntervalMs = std::max(
            250,
            ::RenderInitSystemLogic::getRegistryInt(baseSystem, "voxelPerfLogIntervalMs", 1000)
        );
        if (voxelPerfLogEnabled && now - lastPerfLog >= std::chrono::milliseconds(voxelPerfLogIntervalMs)) {
            size_t voxelRenderDirtyCount = baseSystem.voxelRender ? baseSystem.voxelRender->renderBuffersDirty.size() : 0;
            size_t terrainPending = 0, terrainDesired = 0, terrainGenerated = 0, terrainJobs = 0;
            int terrainStepped = 0;
            int terrainBuilt = 0, terrainConsumed = 0, terrainSkipped = 0, terrainFiltered = 0;
            int terrainRescueSurface = 0, terrainRescueMissing = 0, terrainCapDrop = 0, terrainReprio = 0;
            float terrainPrepMs = 0.0f;
            float terrainMs = 0.0f;
            float terrainDesiredMs = 0.0f;
            float terrainBaseMs = 0.0f;
            float terrainFeatureMs = 0.0f;
            float terrainSurfaceMs = 0.0f;
            float terrainCaveFieldMs = 0.0f;
            uint64_t terrainCaveFieldCellsBuilt = 0;
            uint64_t terrainCaveSamples = 0;
            TerrainSystemLogic::GetVoxelStreamingPerfStats(
                terrainPending,
                terrainDesired,
                terrainGenerated,
                terrainJobs,
                terrainStepped,
                terrainBuilt,
                terrainConsumed,
                terrainSkipped,
                terrainFiltered,
                terrainRescueSurface,
                terrainRescueMissing,
                terrainCapDrop,
                terrainReprio,
                terrainPrepMs,
                terrainMs,
                terrainDesiredMs,
                terrainBaseMs,
                terrainFeatureMs,
                terrainSurfaceMs,
                terrainCaveFieldMs,
                terrainCaveFieldCellsBuilt,
                terrainCaveSamples
            );
            size_t treePending = 0, treePendingDeps = 0, treeVisited = 0;
            int treeSelected = 0, treeProcessed = 0, treeDeferred = 0, treeBackfillAppended = 0;
            bool treeBackfillRan = false;
            float treeMs = 0.0f;
            TreeGenerationSystemLogic::GetTreeFoliagePerfStats(
                treePending,
                treePendingDeps,
                treeVisited,
                treeSelected,
                treeProcessed,
                treeDeferred,
                treeBackfillAppended,
                treeBackfillRan,
                treeMs
            );
            size_t lifeDesired = 0;
            size_t lifeBaseQueued = 0;
            size_t lifeBaseInProgress = 0;
            size_t lifeBaseGenerated = 0;
            size_t lifeFeatureQueued = 0;
            size_t lifeFeatureInProgress = 0;
            size_t lifeSurfacePending = 0;
            size_t lifeReady = 0;
            size_t lifeRenderable = 0;
            size_t lifeReadyNoMesh = 0;
            for (const auto& [key, state] : voxelWorld.chunkStates) {
                if (!state.desired) continue;
                lifeDesired += 1;
                switch (state.stage) {
                    case VoxelChunkLifecycleStage::BaseQueued:
                        lifeBaseQueued += 1;
                        break;
                    case VoxelChunkLifecycleStage::BaseInProgress:
                        lifeBaseInProgress += 1;
                        break;
                    case VoxelChunkLifecycleStage::BaseGenerated:
                        lifeBaseGenerated += 1;
                        break;
                    case VoxelChunkLifecycleStage::FeatureQueued:
                        lifeFeatureQueued += 1;
                        break;
                    case VoxelChunkLifecycleStage::FeatureInProgress:
                        lifeFeatureInProgress += 1;
                        break;
                    case VoxelChunkLifecycleStage::Ready:
                        lifeReady += 1;
                        break;
                    case VoxelChunkLifecycleStage::Desired:
                    case VoxelChunkLifecycleStage::Unknown:
                        break;
                }
                if (state.generated && state.postFeaturesComplete && state.hasSection && !state.surfaceFoliageComplete) {
                    lifeSurfacePending += 1;
                }
                if (state.isRenderable()) {
                    lifeRenderable += 1;
                    if (!baseSystem.voxelRender || baseSystem.voxelRender->renderBuffers.count(key) == 0) {
                        lifeReadyNoMesh += 1;
                    }
                }
            }
            std::cout << "[VoxelPerf] dirty=" << voxelWorld.dirtySections.size()
                      << " voxelRenderDirty=" << voxelRenderDirtyCount
                      << " terrainPending=" << terrainPending
                      << " terrainDesired=" << terrainDesired
                      << " terrainGenerated=" << terrainGenerated
                      << " terrainJobs=" << terrainJobs
                      << " terrainStepped=" << terrainStepped
                      << " terrainBuilt=" << terrainBuilt
                      << " terrainConsumed=" << terrainConsumed
                      << " terrainSkipped=" << terrainSkipped
                      << " terrainFiltered=" << terrainFiltered
                      << " terrainRescue=" << terrainRescueSurface << "/" << terrainRescueMissing
                      << " terrainCapDrop=" << terrainCapDrop
                      << " terrainReprio=" << terrainReprio
                      << " terrainPrepMs=" << terrainPrepMs
                      << " terrainMs=" << terrainMs
                      << " terrainDesiredMs=" << terrainDesiredMs
                      << " terrainBaseMs=" << terrainBaseMs
                      << " terrainFeatureMs=" << terrainFeatureMs
                      << " terrainSurfaceMs=" << terrainSurfaceMs
                      << " terrainCaveFieldMs=" << terrainCaveFieldMs
                      << " terrainCaveFieldCells=" << terrainCaveFieldCellsBuilt
                      << " terrainCaveSamples=" << terrainCaveSamples
                      << " treePending=" << treePending
                      << " treePendingDeps=" << treePendingDeps
                      << " treeVisited=" << treeVisited
                      << " treeSelected=" << treeSelected
                      << " treeProcessed=" << treeProcessed
                      << " treeDeferred=" << treeDeferred
                      << " treeBackfill=" << (treeBackfillRan ? treeBackfillAppended : 0)
                      << " treeMs=" << treeMs
                      << " sections=" << voxelWorld.sections.size()
                      << " renderMeshes=" << (baseSystem.voxelRender ? baseSystem.voxelRender->renderBuffers.size() : 0)
                      << " wireframeNearFaces=" << (baseSystem.voxelRender ? baseSystem.voxelRender->wireframeOverlayNearFaces : 0)
                      << " wireframeFarFaces=" << (baseSystem.voxelRender ? baseSystem.voxelRender->wireframeOverlayFarFaces : 0)
                      << " wireframeSegments=" << (baseSystem.voxelRender ? baseSystem.voxelRender->wireframeOverlayLineSegments : 0)
                      << " lifeDesired=" << lifeDesired
                      << " lifeBaseQ=" << lifeBaseQueued
                      << " lifeBaseIP=" << lifeBaseInProgress
                      << " lifeBaseGen=" << lifeBaseGenerated
                      << " lifeFeatQ=" << lifeFeatureQueued
                      << " lifeFeatIP=" << lifeFeatureInProgress
                      << " lifeSurfPend=" << lifeSurfacePending
                      << " lifeReady=" << lifeReady
                      << " lifeRenderable=" << lifeRenderable
                      << " lifeReadyNoMesh=" << lifeReadyNoMesh;
            if (baseSystem.farTerrain) {
                const FarTerrainClipmapContext& far = *baseSystem.farTerrain;
                std::cout << " farEnabled=" << far.enabled
                          << " farInitialized=" << far.initialized
                          << " farRebuilds=" << far.rebuildCount
                          << " farLastFull=" << far.lastFullRebuild
                          << " farLastHandoff=" << far.lastHandoffRefresh
                          << " farFaces=" << far.visibleFaceCount
                          << " farHandoffFaces=" << far.handoffVisibleFaceCount
                          << " farBodyFaces=" << far.bodyVisibleFaceCount
                          << " farHandoffClusters=" << far.handoffRenderClusters.size()
                          << " farBodyClusters=" << far.bodyRenderClusters.size()
                          << " farCells=" << far.lastLandCellCount
                          << " farHandoffCells=" << far.lastHandoffCellCount
                          << " farBodyCells=" << far.lastBodyCellCount
                          << " farSuppressed=" << far.lastSuppressedCellCount
                          << " farBaseCell=" << far.baseCellSize
                          << " farNear=" << far.nearRadiusBlocks
                          << " farMax=" << far.maxRadiusBlocks
                          << " farRings=" << far.ringCount
                          << " farVisibleRings=" << far.visibleRingCount
                          << " farLastParamsChanged=" << far.lastParamsChanged
                          << " farLastAnchorChanged=" << far.lastAnchorChanged
                          << " farLastCoverageChanged=" << far.lastCoverageChanged
                          << " farLastPeriodicDue=" << far.lastPeriodicRefreshDue
                          << " farLastViewBucketChanged=" << far.lastViewBucketChanged
                          << " farLastAnchorDelta=" << far.lastAnchorDeltaX << "," << far.lastAnchorDeltaZ
                          << " farUpdateMs=" << far.lastUpdateMs
                          << " farSetupMs=" << far.lastSetupMs
                          << " farBodyBuildMs=" << far.lastBodyBuildMs
                          << " farHandoffMs=" << far.lastHandoffBuildMs
                          << " farUploadMs=" << far.lastUploadMs
                          << " farCellResolveMs=" << far.lastCellResolveMs
                          << " farTopGreedyMs=" << far.lastTopGreedyMs
                          << " farTopMergeMs=" << far.lastTopMergeMs
                          << " farVerticalMs=" << far.lastVerticalBuildMs
                          << " farClusterPrepMs=" << far.lastClusterPrepMs
                          << " farClusterUploadMs=" << far.lastClusterUploadMs
                          << " farUploadOnlyCount=" << far.uploadOnlyCount
                          << " farHandoffRefreshCount=" << far.handoffRefreshCount
                          << " farHandoffRefreshByCoverage=" << far.handoffRefreshByCoverageCount
                          << " farHandoffRefreshByPeriodic=" << far.handoffRefreshByPeriodicCount
                          << " farRebuildByParams=" << far.fullRebuildByParamsCount
                          << " farRebuildByAnchor=" << far.fullRebuildByAnchorCount
                          << " farRebuildByCoverage=" << far.fullRebuildByCoverageCount
                          << " farRebuildByView=" << far.fullRebuildByViewCount
                          << " farSectorRingTests=" << far.sectorRingTests
                          << " farSectorRingRejected=" << far.sectorRingRejected
                          << " farSectorCellTests=" << far.sectorCellTests
                          << " farSectorCellRejected=" << far.sectorCellRejected
                          << " farFrustumRingTests=" << far.frustumRingTests
                          << " farFrustumRingRejected=" << far.frustumRingRejected
                          << " farFrustumCellTests=" << far.frustumCellTests
                          << " farFrustumCellRejected=" << far.frustumCellRejected
                          << " farLodCells=";
                for (size_t i = 0; i < far.lodCellCounts.size(); ++i) {
                    if (i > 0) std::cout << ",";
                    std::cout << far.lodCellCounts[i];
                }
                std::cout << " farLodFaces=";
                for (size_t i = 0; i < far.lodFaceCounts.size(); ++i) {
                    if (i > 0) std::cout << ",";
                    std::cout << far.lodFaceCounts[i];
                }
                std::cout << " farLodTriangles=";
                for (size_t i = 0; i < far.lodTriangleCounts.size(); ++i) {
                    if (i > 0) std::cout << ",";
                    std::cout << far.lodTriangleCounts[i];
                }
            }
            if (baseSystem.occlusionCulling) {
                const OcclusionCullingContext& occlusion = *baseSystem.occlusionCulling;
                std::cout << " occTested=" << occlusion.testedSectionCount
                          << " occVisible=" << occlusion.visibleSectionCount
                          << " occOccluded=" << occlusion.occludedSectionCount
                          << " occNearKept=" << occlusion.nearKeptSectionCount
                          << " occFrustumRejected=" << occlusion.frustumRejectedSectionCount
                          << " occFrozen=" << (occlusion.debugFrozen ? 1 : 0)
                          << " occHzbValid=" << (occlusion.hzbValid ? 1 : 0)
                          << " occHzbOccluders=" << occlusion.hzbOccluderClusterCount
                          << " occHzbCaptureMs=" << occlusion.hzbCaptureMs
                          << " occHzbReadbackMs=" << occlusion.hzbReadbackMs
                          << " occHzbBuildMs=" << occlusion.hzbBuildMs
                          << " occHzbQueryMs=" << occlusion.hzbQueryMs
                          << " farOccTested=" << occlusion.farTestedCount
                          << " farOccOccluded=" << occlusion.farOccludedCount;
            }
            std::cout << std::endl;
            lastPerfLog = now;
        }

        if (::RenderInitSystemLogic::getRegistryBool(baseSystem, "DebugVoxelMesh", false)) {
            static auto lastDebugLog = std::chrono::steady_clock::now();
            auto nowDbg = std::chrono::steady_clock::now();
            if (nowDbg - lastDebugLog >= std::chrono::seconds(1)) {
                size_t worldDirty = baseSystem.voxelWorld ? baseSystem.voxelWorld->dirtySections.size() : 0;
                size_t renderDirty = baseSystem.voxelRender ? baseSystem.voxelRender->renderBuffersDirty.size() : 0;
                std::cout << "[DebugVoxelMesh] worldDirty=" << worldDirty
                          << " renderDirty=" << renderDirty
                          << std::endl;
                lastDebugLog = nowDbg;
            }
        }
    }
}
