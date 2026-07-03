/*
Copyright 2026 NVIDIA CORPORATION

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#pragma once

#include <memory>
#include <vector>

#include "nvblox/core/cuda_stream.h"
#include "nvblox/core/parameter_tree.h"
#include "nvblox/integrators/esdf_integrator.h"
#include "nvblox/integrators/freespace_integrator.h"
#include "nvblox/integrators/occupancy_decay_integrator.h"
#include "nvblox/integrators/projective_appearance_integrator.h"
#include "nvblox/integrators/projective_occupancy_integrator.h"
#include "nvblox/integrators/projective_tsdf_integrator.h"
#include "nvblox/integrators/shape_clearer.h"
#include "nvblox/integrators/tsdf_decay_integrator.h"
#include "nvblox/mesh/flat_mesh_integrator.h"
#include "nvblox/mesh/mesh_integrator.h"
#include "nvblox/sensors/sensor.h"

namespace nvblox {

struct MapperParams;

/// Owns and configures the concrete integrators used by Mapper.
///
/// Mapper keeps ownership of layers, block tracking, and public mapping
/// workflows. This bundle keeps integrator construction, shared cache setup,
/// parameter wiring, and parameter reporting in one place.
struct MapperIntegrators {
  enum class ViewpointCacheSharing { kDisabled, kEnabled };

  explicit MapperIntegrators(std::shared_ptr<CudaStream> cuda_stream,
                             ViewpointCacheSharing viewpoint_cache_sharing =
                                 ViewpointCacheSharing::kEnabled);

  void setMapperParams(const MapperParams& params);
  std::vector<parameters::ParameterTreeNode> getParameterTreeNodes() const;

  template <typename SensorType>
  const ProjectiveTsdfIntegrator& getTsdfIntegrator() const {
    if constexpr (SensorType::sensor_modality() == SensorModality::kLidar) {
      return lidar_tsdf_integrator;
    } else {
      return tsdf_integrator;
    }
  }

  template <typename SensorType>
  ProjectiveTsdfIntegrator& getTsdfIntegrator() {
    if constexpr (SensorType::sensor_modality() == SensorModality::kLidar) {
      return lidar_tsdf_integrator;
    } else {
      return tsdf_integrator;
    }
  }

  template <typename SensorType>
  const ProjectiveOccupancyIntegrator& getOccupancyIntegrator() const {
    if constexpr (SensorType::sensor_modality() == SensorModality::kLidar) {
      return lidar_occupancy_integrator;
    } else {
      return occupancy_integrator;
    }
  }

  template <typename SensorType>
  ProjectiveOccupancyIntegrator& getOccupancyIntegrator() {
    if constexpr (SensorType::sensor_modality() == SensorModality::kLidar) {
      return lidar_occupancy_integrator;
    } else {
      return occupancy_integrator;
    }
  }

  ProjectiveTsdfIntegrator tsdf_integrator;
  ProjectiveTsdfIntegrator lidar_tsdf_integrator;
  FreespaceIntegrator freespace_integrator;
  ProjectiveOccupancyIntegrator occupancy_integrator;
  ProjectiveOccupancyIntegrator lidar_occupancy_integrator;
  OccupancyDecayIntegrator occupancy_decay_integrator;
  TsdfDecayIntegrator tsdf_decay_integrator;
  TsdfShapeClearer tsdf_shape_clearer;
  ProjectiveColorIntegrator color_integrator;
  ProjectiveFeatureIntegrator feature_integrator;
  ColorMeshIntegrator color_mesh_integrator;
  FeatureMeshIntegrator feature_mesh_integrator;
  ColorFlatMeshIntegrator color_flat_mesh_integrator;
  FeatureFlatMeshIntegrator feature_flat_mesh_integrator;
  EsdfIntegrator esdf_integrator;

 private:
  void shareViewpointCaches();
};

}  // namespace nvblox
