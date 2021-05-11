#ifndef __PROTOTYPE_AGENT_PTR_h__
#define __PROTOTYPE_AGENT_PTR_h__

#include <GU/GU_Agent.h>
#include <GU/GU_AgentDefinition.h>
#include <GU/GU_AgentRig.h>
#include <GU/GU_AgentShapeLib.h>
#include <GU/GU_AgentLayer.h>
#include <GU/GU_PrimPacked.h>
#include <GU/GU_PrimPoly.h>
#include <GEO/GEO_AttributeCaptureRegion.h>
#include <GEO/GEO_AttributeIndexPairs.h>
#include <GA/GA_AIFIndexPair.h>
#include <GA/GA_Names.h>
#include "BNode.h"

#include <iostream>
#include <fstream>

// Inspired by SOP_BouncyAgent

namespace PrototypeAgentPtr
{
	/// Establishes a unique ID for each prototype-definition feature
	extern int protocount;

	/// Fills a detail with default geometry defined by a BNode tree structure
	extern GU_Detail* buildGeo(std::vector<std::shared_ptr<BNode>>& inOrder,
		std::map<int, std::vector<GA_Offset>>& jointOffsets,
		int divisions = 10);

	/// Stores the indexing information for this definition's rig
	extern GU_AgentRigPtr createRig(const char* path, std::shared_ptr<BNode> root,
		std::vector<std::shared_ptr<BNode>>& inOrder);

	/// Stores the initial rig-joint transformations and 
	/// the weights of their influence on the geometry points
	extern void addWeights(const GU_AgentRig& rig, 
		const GU_DetailHandle& geomHandle, 
		std::vector<std::shared_ptr<BNode>>& inOrder,
		std::map<int, std::vector<GA_Offset>>& jointOffsets);

	/// Stores geometry information for prototype (including weights)
	extern GU_AgentShapeLibPtr createShapeLibrary(const char* path,
		const GU_AgentRig& rig, GU_Detail* geo, 
		std::vector<std::shared_ptr<BNode>>& inOrder,
		std::map<int, std::vector<GA_Offset>>& jointOffsets);

	/// Stores skin and deformer information
	extern GU_AgentLayerPtr createStartLayer(const char* path, 
		const GU_AgentRigPtr& rig, const GU_AgentShapeLibPtr &shapeLibrary);

	/// Returns a definition defining an agent (protoype) type
	extern GU_AgentDefinitionPtr createDefinition(std::shared_ptr<BNode> root, 
		const char* path);
}

#endif

