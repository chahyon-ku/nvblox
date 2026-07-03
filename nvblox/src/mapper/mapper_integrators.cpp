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
#include "nvblox/mapper/mapper_integrators.h"

#include "nvblox/mapper/internal/mapper_common.h"
#include "nvblox/mapper/mapper_params.h"

namespace nvblox {
namespace {

void setEsdfParams(EsdfIntegrator* integrator,
                   const EsdfIntegratorParams& esdf_params,
                   const ProjectiveIntegratorParams& projective_params) {
  integrator->esdf_slice_min_height(esdf_params.esdf_slice_min_height);
  integrator->esdf_slice_max_height(esdf_params.esdf_slice_max_height);
  integrator->esdf_slice_height(esdf_params.esdf_slice_height);
  integrator->slice_height_above_plane_m(
      esdf_params.slice_height_above_plane_m);
  integrator->slice_height_thickness_m(esdf_params.slice_height_thickness_m);
  integrator->max_esdf_distance_m(esdf_params.esdf_integrator_max_distance_m);
  integrator->min_weight(esdf_params.esdf_integrator_min_weight);
  integrator->max_site_distance_vox(
      esdf_params.esdf_integrator_max_site_distance_vox);
  integrator->unobserved_esdf_policy(esdf_params.unobserved_esdf_policy);
  integrator->add_negative_truncation_band_sites(
      esdf_params.add_negative_truncation_band_sites);
  integrator->truncation_distance_vox(
      projective_params.projective_integrator_truncation_distance_vox);
}

void setCameraProjectiveParams(
    ProjectiveTsdfIntegrator* tsdf_integrator,
    ProjectiveOccupancyIntegrator* occupancy_integrator,
    ProjectiveColorIntegrator* color_integrator,
    ProjectiveFeatureIntegrator* feature_integrator,
    const ProjectiveIntegratorParams& params) {
  tsdf_integrator->max_integration_distance_m(
      params.projective_integrator_max_integration_distance_m);
  occupancy_integrator->max_integration_distance_m(
      params.projective_integrator_max_integration_distance_m);
  color_integrator->max_integration_distance_m(
      params.projective_integrator_max_integration_distance_m);
  feature_integrator->max_integration_distance_m(
      params.projective_integrator_max_integration_distance_m);

  tsdf_integrator->truncation_distance_vox(
      params.projective_integrator_truncation_distance_vox);
  occupancy_integrator->truncation_distance_vox(
      params.projective_integrator_truncation_distance_vox);

  tsdf_integrator->weighting_function_type(
      params.projective_integrator_weighting_mode);
  color_integrator->weighting_function_type(
      params.projective_integrator_weighting_mode);
  feature_integrator->weighting_function_type(
      params.projective_integrator_weighting_mode);

  tsdf_integrator->max_weight(params.projective_integrator_max_weight);
  color_integrator->max_weight(params.projective_integrator_max_weight);
  feature_integrator->max_weight(params.projective_integrator_max_weight);

  color_integrator->measurement_weight(
      params.projective_appearance_integrator_measurement_weight);
  feature_integrator->measurement_weight(
      params.projective_appearance_integrator_measurement_weight);

  tsdf_integrator->invalid_depth_decay_factor(
      params.projective_tsdf_integrator_invalid_depth_decay_factor);
}

void setLidarProjectiveParams(
    ProjectiveTsdfIntegrator* tsdf_integrator,
    ProjectiveOccupancyIntegrator* occupancy_integrator,
    const ProjectiveIntegratorParams& params) {
  tsdf_integrator->max_integration_distance_m(
      params.lidar_projective_integrator_max_integration_distance_m);
  occupancy_integrator->max_integration_distance_m(
      params.lidar_projective_integrator_max_integration_distance_m);

  tsdf_integrator->truncation_distance_vox(
      params.projective_integrator_truncation_distance_vox);
  occupancy_integrator->truncation_distance_vox(
      params.projective_integrator_truncation_distance_vox);

  tsdf_integrator->weighting_function_type(
      params.projective_integrator_weighting_mode);
  tsdf_integrator->max_weight(params.projective_integrator_max_weight);
  tsdf_integrator->invalid_depth_decay_factor(
      params.projective_tsdf_integrator_invalid_depth_decay_factor);
}

void setOccupancyParams(ProjectiveOccupancyIntegrator* camera_integrator,
                        ProjectiveOccupancyIntegrator* lidar_integrator,
                        const OccupancyIntegratorParams& params) {
  camera_integrator->free_region_occupancy_probability(
      params.free_region_occupancy_probability);
  lidar_integrator->free_region_occupancy_probability(
      params.free_region_occupancy_probability);
  camera_integrator->occupied_region_occupancy_probability(
      params.occupied_region_occupancy_probability);
  lidar_integrator->occupied_region_occupancy_probability(
      params.occupied_region_occupancy_probability);
  camera_integrator->unobserved_region_occupancy_probability(
      params.unobserved_region_occupancy_probability);
  lidar_integrator->unobserved_region_occupancy_probability(
      params.unobserved_region_occupancy_probability);
  camera_integrator->occupied_region_half_width_m(
      params.occupied_region_half_width_m);
  lidar_integrator->occupied_region_half_width_m(
      params.occupied_region_half_width_m);
}

void setWorkspaceBounds(ViewCalculator* view_calculator,
                        const ViewCalculatorParams& params) {
  view_calculator->workspace_bounds_type(params.workspace_bounds_type);
  view_calculator->workspace_bounds_min_corner_m(
      Vector3f(params.workspace_bounds_min_corner_x_m,
               params.workspace_bounds_min_corner_y_m,
               params.workspace_bounds_min_height_m));
  view_calculator->workspace_bounds_max_corner_m(
      Vector3f(params.workspace_bounds_max_corner_x_m,
               params.workspace_bounds_max_corner_y_m,
               params.workspace_bounds_max_height_m));
}

void setViewCalculatorParams(MapperIntegrators* integrators,
                             const ViewCalculatorParams& params) {
  integrators->tsdf_integrator.view_calculator().raycast_subsampling_factor(
      params.raycast_subsampling_factor);
  integrators->lidar_tsdf_integrator.view_calculator()
      .raycast_subsampling_factor(params.raycast_subsampling_factor);
  integrators->color_integrator.view_calculator().raycast_subsampling_factor(
      params.raycast_subsampling_factor);
  integrators->feature_integrator.view_calculator().raycast_subsampling_factor(
      params.raycast_subsampling_factor);

  setWorkspaceBounds(&integrators->tsdf_integrator.view_calculator(), params);
  setWorkspaceBounds(&integrators->color_integrator.view_calculator(), params);
  setWorkspaceBounds(&integrators->feature_integrator.view_calculator(),
                     params);
}

void setMeshParams(ColorMeshIntegrator* color_mesh_integrator,
                   FeatureMeshIntegrator* feature_mesh_integrator,
                   ColorFlatMeshIntegrator* color_flat_mesh_integrator,
                   FeatureFlatMeshIntegrator* feature_flat_mesh_integrator,
                   const MeshIntegratorParams& params) {
  color_mesh_integrator->min_weight(params.mesh_integrator_min_weight);
  color_mesh_integrator->weld_vertices(params.mesh_integrator_weld_vertices);
  feature_mesh_integrator->min_weight(params.mesh_integrator_min_weight);
  feature_mesh_integrator->weld_vertices(params.mesh_integrator_weld_vertices);

  color_flat_mesh_integrator->min_weight(params.mesh_integrator_min_weight);
  color_flat_mesh_integrator->max_num_triangles(
      params.mesh_integrator_max_flat_mesh_triangles);
  feature_flat_mesh_integrator->min_weight(params.mesh_integrator_min_weight);
  feature_flat_mesh_integrator->max_num_triangles(
      params.mesh_integrator_max_flat_mesh_triangles);
}

void setDecayParams(TsdfDecayIntegrator* tsdf_decay_integrator,
                    OccupancyDecayIntegrator* occupancy_decay_integrator,
                    const DecayIntegratorBaseParams& base_params,
                    const TsdfDecayIntegratorParams& tsdf_params,
                    const OccupancyDecayIntegratorParams& occupancy_params) {
  tsdf_decay_integrator->deallocate_decayed_blocks(
      base_params.decay_integrator_deallocate_decayed_blocks);
  occupancy_decay_integrator->deallocate_decayed_blocks(
      base_params.decay_integrator_deallocate_decayed_blocks);

  tsdf_decay_integrator->decay_factor(tsdf_params.tsdf_decay_factor);
  tsdf_decay_integrator->decayed_weight_threshold(
      tsdf_params.tsdf_decayed_weight_threshold);
  tsdf_decay_integrator->set_free_distance_on_decayed(
      tsdf_params.tsdf_set_free_distance_on_decayed);
  tsdf_decay_integrator->free_distance_vox(
      tsdf_params.tsdf_decayed_free_distance_vox);

  occupancy_decay_integrator->free_region_decay_probability(
      occupancy_params.free_region_decay_probability);
  occupancy_decay_integrator->occupied_region_decay_probability(
      occupancy_params.occupied_region_decay_probability);
  occupancy_decay_integrator->decay_to_free(
      occupancy_params.occupancy_decay_to_free);
}

void setFreespaceParams(FreespaceIntegrator* integrator,
                        const FreespaceIntegratorParams& params) {
  integrator->max_tsdf_distance_for_occupancy_m(
      params.max_tsdf_distance_for_occupancy_m);
  integrator->max_unobserved_to_keep_consecutive_occupancy_ms(
      params.max_unobserved_to_keep_consecutive_occupancy_ms);
  integrator->min_duration_since_occupied_for_freespace_ms(
      params.min_duration_since_occupied_for_freespace_ms);
  integrator->min_consecutive_occupancy_duration_for_reset_ms(
      params.min_consecutive_occupancy_duration_for_reset_ms);
  integrator->check_neighborhood(params.check_neighborhood);
  integrator->initialize_to_high_confidence_freespace(
      params.initialize_to_high_confidence_freespace);
}

}  // namespace

MapperIntegrators::MapperIntegrators(
    std::shared_ptr<CudaStream> cuda_stream,
    MapperIntegrators::ViewpointCacheSharing viewpoint_cache_sharing)
    : tsdf_integrator(cuda_stream),
      lidar_tsdf_integrator(cuda_stream),
      freespace_integrator(cuda_stream),
      occupancy_integrator(cuda_stream),
      lidar_occupancy_integrator(cuda_stream),
      tsdf_shape_clearer(cuda_stream),
      color_integrator(cuda_stream),
      feature_integrator(cuda_stream),
      color_mesh_integrator(cuda_stream),
      feature_mesh_integrator(cuda_stream),
      color_flat_mesh_integrator(cuda_stream),
      feature_flat_mesh_integrator(cuda_stream),
      esdf_integrator(cuda_stream) {
  if (viewpoint_cache_sharing == ViewpointCacheSharing::kEnabled) {
    shareViewpointCaches();
  }
}

void MapperIntegrators::setMapperParams(const MapperParams& params) {
  setEsdfParams(&esdf_integrator, params.esdf_integrator_params,
                params.projective_integrator_params);
  setCameraProjectiveParams(&tsdf_integrator, &occupancy_integrator,
                            &color_integrator, &feature_integrator,
                            params.projective_integrator_params);
  setLidarProjectiveParams(&lidar_tsdf_integrator, &lidar_occupancy_integrator,
                           params.projective_integrator_params);
  setOccupancyParams(&occupancy_integrator, &lidar_occupancy_integrator,
                     params.occupancy_integrator_params);
  setViewCalculatorParams(this, params.view_calculator_params);
  setMeshParams(&color_mesh_integrator, &feature_mesh_integrator,
                &color_flat_mesh_integrator, &feature_flat_mesh_integrator,
                params.mesh_integrator_params);
  setDecayParams(&tsdf_decay_integrator, &occupancy_decay_integrator,
                 params.decay_integrator_base_params,
                 params.tsdf_decay_integrator_params,
                 params.occupancy_decay_integrator_params);
  setFreespaceParams(&freespace_integrator, params.freespace_integrator_params);
}

void MapperIntegrators::shareViewpointCaches() {
  // Make the camera integrators share the same viewpoint cache.
  nvblox::shareViewpointCaches(&tsdf_integrator, &occupancy_integrator,
                               &color_integrator, &feature_integrator);
  // Make the LiDAR integrators share the same viewpoint cache.
  nvblox::shareViewpointCaches(&lidar_tsdf_integrator,
                               &lidar_occupancy_integrator);
}

std::vector<parameters::ParameterTreeNode>
MapperIntegrators::getParameterTreeNodes() const {
  return {
      tsdf_integrator.getParameterTree("camera_tsdf_integrator"),
      lidar_tsdf_integrator.getParameterTree("lidar_tsdf_integrator"),
      color_integrator.getParameterTree(),
      feature_integrator.getParameterTree(),
      occupancy_integrator.getParameterTree("camera_occupancy_integrator"),
      lidar_occupancy_integrator.getParameterTree("lidar_occupancy_integrator"),
      esdf_integrator.getParameterTree(),
      color_mesh_integrator.getParameterTree(),
      feature_mesh_integrator.getParameterTree(),
      color_flat_mesh_integrator.getParameterTree("color_flat_mesh_integrator"),
      feature_flat_mesh_integrator.getParameterTree(
          "feature_flat_mesh_integrator"),
      occupancy_decay_integrator.getParameterTree(),
      tsdf_decay_integrator.getParameterTree(),
      freespace_integrator.getParameterTree()};
}

}  // namespace nvblox
