/*
 * Copyright 2019 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/
#ifndef SDF_FRAMESEMANTICS_HH_
#define SDF_FRAMESEMANTICS_HH_

#include <map>
#include <string>

#include <ignition/math/Pose3.hh>
#include <ignition/math/graph/Graph.hh>

#include "sdf/Error.hh"
#include "sdf/Types.hh"
#include "sdf/system_util.hh"

/// \ingroup sdf_frame_semantics
/// \brief namespace for Simulation Description Format Frame Semantics Utilities
///
/// The Frame Semantics Utilities construct and operate on graphs representing
/// the kinematics, frame attached_to, and pose relative_to relationships
/// defined within models and world.
namespace sdf
{
  // Inline bracket to help doxygen filtering.
  inline namespace SDF_VERSION_NAMESPACE {
  //
  // Forward declaration.
  class Joint;
  class Link;
  class Model;
  class World;

  /// \enum FrameType
  /// \brief The set of frame types. INVALID indicates that frame type has
  /// not been set, or has not been set correctly.
  enum class FrameType
  {
    /// \brief An implicit world frame.
    WORLD = 0,

    /// \brief An implicit model frame.
    MODEL = 1,

    /// \brief An implicit link frame.
    LINK = 2,

    /// \brief An implicit joint frame.
    JOINT = 3,

    /// \brief An explicit frame.
    FRAME = 4,
  };

  /// \brief Data structure for kinematic graph for a Model.
  struct SDFORMAT_VISIBLE KinematicGraph
  {
    /// \brief A DirectedGraph with a vertex for each Link and an edge
    /// for each Joint.
    /// to the frame to which another frame is attached. Each vertex stores
    /// its FrameType and each edge can store a boolean value.
    using GraphType =
        ignition::math::graph::DirectedGraph<const Link*, const Joint*>;
    GraphType graph;

    /// \brief A Map from Vertex names to Vertex Ids.
    using MapType = std::map<std::string, ignition::math::graph::VertexId>;
    MapType map;
  };

  /// \brief Data structure for frame attached_to graphs for Model or World.
  struct SDFORMAT_VISIBLE FrameAttachedToGraph
  {
    /// \brief A DirectedGraph with a vertex for each frame and edges pointing
    /// to the frame to which another frame is attached. Each vertex stores
    /// its FrameType and each edge can store a boolean value.
    using GraphType = ignition::math::graph::DirectedGraph<FrameType, bool>;
    GraphType graph;

    /// \brief A Map from Vertex names to Vertex Ids.
    using MapType = std::map<std::string, ignition::math::graph::VertexId>;
    MapType map;

    /// \brief Name of scope vertex, either __model__ or world.
    std::string scopeName;
  };

  /// \brief Data structure for pose relative_to graphs for Model or World.
  struct SDFORMAT_VISIBLE PoseRelativeToGraph
  {
    /// \brief A DirectedGraph with a vertex for each explicit or implicit
    /// frame and edges pointing from the frame relative to which its pose
    /// is defined. Each vertex stores its FrameType and each edge
    /// stores the Pose3 between those frames.
    using Pose3d = ignition::math::Pose3d;
    using GraphType = ignition::math::graph::DirectedGraph<FrameType, Pose3d>;
    GraphType graph;

    /// \brief A Map from Vertex names to Vertex Ids.
    using MapType = std::map<std::string, ignition::math::graph::VertexId>;
    MapType map;

    /// \brief Name of source vertex, either __model__ or world.
    std::string sourceName;
  };

  /// \brief Build a KinematicGraph for a model.
  /// \param[out] _out Graph object to write.
  /// \param[in] _model Model from which to build attached_to graph.
  /// \return Errors.
  SDFORMAT_VISIBLE
  Errors buildKinematicGraph(KinematicGraph &_out, const Model *_model);

  /// \brief Build a FrameAttachedToGraph for a model.
  /// \param[out] _out Graph object to write.
  /// \param[in] _model Model from which to build attached_to graph.
  /// \return Errors.
  SDFORMAT_VISIBLE
  Errors buildFrameAttachedToGraph(
              FrameAttachedToGraph &_out, const Model *_model);

  /// \brief Build a FrameAttachedToGraph for a world.
  /// \param[out] _out Graph object to write.
  /// \param[in] _world World from which to build attached_to graph.
  /// \return Errors.
  SDFORMAT_VISIBLE
  Errors buildFrameAttachedToGraph(
              FrameAttachedToGraph &_out, const World *_world);

  /// \brief Build a PoseRelativeToGraph for a model.
  /// \param[out] _out Graph object to write.
  /// \param[in] _model Model from which to build attached_to graph.
  /// \return Errors.
  SDFORMAT_VISIBLE
  Errors buildPoseRelativeToGraph(
              PoseRelativeToGraph &_out, const Model *_model);

  /// \brief Build a PoseRelativeToGraph for a world.
  /// \param[out] _out Graph object to write.
  /// \param[in] _world World from which to build attached_to graph.
  /// \return Errors.
  SDFORMAT_VISIBLE
  Errors buildPoseRelativeToGraph(
              PoseRelativeToGraph &_out, const World *_world);


  /// \brief Confirm that FrameAttachedToGraph is valid by checking the number
  /// of outbound edges for each vertex and checking for graph cycles.
  /// \param[in] _in Graph object to validate.
  /// \return Errors.
  SDFORMAT_VISIBLE
  Errors validateFrameAttachedToGraph(const FrameAttachedToGraph &_in);

  /// \brief Confirm that PoseRelativeToGraph is valid by checking the number
  /// of outbound edges for each vertex and checking for graph cycles.
  /// \param[in] _in Graph object to validate.
  /// \return Errors.
  SDFORMAT_VISIBLE
  Errors validatePoseRelativeToGraph(const PoseRelativeToGraph &_in);

  /// \brief Resolve the attached-to body for a given frame. Following the
  /// edges of the frame attached-to graph from a given frame must lead
  /// to a link or world frame.
  /// \param[in] _in Graph to use for resolving the body.
  /// \param[in] _vertexName This resolves the attached-to body of the
  /// vertex with this name.
  /// \param[out] _attachedToBody Name of link to which this frame is
  /// attached or "world" if frame is attached to the world.
  /// \return Errors if the graph is invalid or the frame does not lead to
  /// a link or world frame.
  SDFORMAT_VISIBLE
  Errors resolveFrameAttachedToBody(
      std::string &_attachedToBody,
      const FrameAttachedToGraph &_in,
      const std::string &_vertexName);

  /// \brief Resolve pose of a vertex relative to its outgoing ancestor
  /// (analog of the root of a tree).
  /// \param[in] _graph PoseRelativeToGraph to read from.
  /// \param[in] _vertexName Name of vertex whose pose is to be computed.
  /// \param[out] _pose Pose object to write.
  /// \return Errors.
  SDFORMAT_VISIBLE
  Errors resolvePoseRelativeToRoot(
      ignition::math::Pose3d &_pose,
      const PoseRelativeToGraph &_graph,
      const std::string &_vertexName);

  /// \brief Resolve pose of a frame relative to named frame.
  /// \param[in] _graph PoseRelativeToGraph to read from.
  /// \param[in] _frameName Name of frame whose pose is to be resolved.
  /// \param[in] _relativeTo Name of frame relative to which the pose is
  /// to be resolved.
  /// \param[out] _pose Pose object to write.
  /// \return Errors.
  SDFORMAT_VISIBLE
  Errors resolvePose(
      ignition::math::Pose3d &_pose,
      const PoseRelativeToGraph &_graph,
      const std::string &_frameName,
      const std::string &_relativeTo);
  }
}
#endif
