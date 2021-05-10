#include "PrototypeAgentPtr.h"

/// Establishes a unique ID for each prototype-definition feature
int PrototypeAgentPtr::protocount = 0;

/// Fills a detail with default geometry defined by a BNode tree structure
GU_Detail*  PrototypeAgentPtr::buildGeo(std::vector<std::shared_ptr<BNode>>& inOrder,
				std::map<int, std::vector<GA_Offset>>& jointOffsets, int divisions) {
	GU_Detail* geo = new GU_Detail;

	// Parses each node in order of tree depth, since we already have that vector
	for (int i = 0; i < inOrder.size(); i++) {
		std::shared_ptr<BNode> currNode = inOrder.at(i);
		std::shared_ptr<BNode> parent = currNode->getParent();

		if (!parent) { continue; } /// Only create a "branch segment" between two nodes

		// The positions defining the ends of this cylinder
		std::vector<UT_Vector3> parentCircle = std::vector<UT_Vector3>();
		std::vector<UT_Vector3> currCircle = std::vector<UT_Vector3>();

		UT_Matrix3 transform = UT_Matrix3(currNode->getWorldTransform());

		// Create the angled circle shapes
		float ang = M_PI * 2 / divisions;
		for (int i = 0; i < divisions; i++) {
			// Get point along a circle
			UT_Vector3 point;
			point(0) = cos(ang * i);
			point(1) = 0.0f;
			point(2) = -sin(ang * i);
			// Rotate to match the branch
			point = rowVecMult(point * 0.1f, transform);

			parentCircle.push_back(parent->getPos() + point);
			currCircle.push_back(currNode->getPos() + point);
		}

		GU_PrimPoly* polyParentCircle = GU_PrimPoly::build(geo, divisions, GU_POLY_CLOSED);
		GU_PrimPoly* polyCurrCircle = GU_PrimPoly::build(geo, divisions, GU_POLY_CLOSED);

		// Now set the actual positions in the details
		for (int j = 0; j < divisions; j++) {
			// For the circles:
			GA_Offset ptoffPar = polyParentCircle->getPointOffset(j);
			geo->setPos3(ptoffPar, parentCircle.at(j));
			jointOffsets.at(parent->getRigIndex()).push_back(ptoffPar);///

			GA_Offset ptoffCurr = polyCurrCircle->getPointOffset(j);
			geo->setPos3(ptoffCurr, currCircle.at(j));
			jointOffsets.at(currNode->getRigIndex()).push_back(ptoffCurr);///

			// For the individual rectangle faces along the cylinder length:
			GU_PrimPoly* polyRect = GU_PrimPoly::build(geo, 4, GU_POLY_CLOSED);

			for (int k = 0; k < 2; k++) {
				GA_Offset ptoffRect = polyRect->getPointOffset(k);
				geo->setPos3(ptoffRect, parentCircle.at((j + k) % divisions));
				jointOffsets.at(parent->getRigIndex()).push_back(ptoffRect);///
			}

			for (int k = 0; k < 2; k++) {
				GA_Offset ptoffRect = polyRect->getPointOffset(k + 2);
				geo->setPos3(ptoffRect, currCircle.at((j + 1 - k) % divisions));
				jointOffsets.at(currNode->getRigIndex()).push_back(ptoffRect);///
			}
		}
	}

	return geo;
}


/// Finding height of a BNode tree - Inspired by GeeksforGeeks
int recNumLevels(std::shared_ptr<BNode> currNode) {
	if (currNode) {
		int currMax = 0;

		for (std::shared_ptr<BNode> child : currNode->getChildren()) {
			int h = recNumLevels(child);
			if (h > currMax) {
				currMax = h;
			}
		}
		return currMax + 1;
	}
	return 0;
}

/// Storing a BNode tree in order of the node's distance from the root
void recChildLevel(std::shared_ptr<BNode> currNode, int level, 
	std::vector<std::shared_ptr<BNode>>& inOrder) {
	if (currNode) {
		if (level == 1) {
			inOrder.push_back(currNode);
		}
		else if (level > 1) {
			for (std::shared_ptr<BNode> child : currNode->getChildren()) {
				recChildLevel(child, level - 1, inOrder);
			}
		}
	}
}

/// Stores the indexing information for this definition's rig (transforms are set in weight function)
GU_AgentRigPtr PrototypeAgentPtr::createRig(const char* path, std::shared_ptr<BNode> root,
	std::vector<std::shared_ptr<BNode>>& inOrder)
{
	UT_String rigName = path;
	rigName += "?proto";
	rigName += std::to_string(PrototypeAgentPtr::protocount);
	rigName += "rig";
	///std::cout << rigName << std::endl;
	GU_AgentRigPtr rig = GU_AgentRig::addRig(rigName);

	UT_StringArray names;
	UT_IntArray child_counts;
	UT_IntArray children;

	names.append("skin");
	child_counts.append(0);

	int numLevels = recNumLevels(root);
	for (int level = 1; level <= numLevels; level++) {
		recChildLevel(root, level, inOrder);
	}

	for (int i = 0; i < inOrder.size(); i++) {
		std::shared_ptr<BNode> currNode = inOrder.at(i);
		currNode->setRigIndex(names.size());

		if (i == 0) {
			names.append("root");
		}
		else {
			names.append("joint" + std::to_string(i));
		}

		child_counts.append(currNode->getChildren().size());
	}

	for (int i = 2; i < inOrder.size() + 1; i++) { 	/// 0 = skin, 1 = root
		children.append(i);
	}

	/// Print the rig joint data for testing
	// Iterators
	/*std::cout << "Num names: " + std::to_string(names.size()) << std::endl;
	int c = 0;
	UT_Array<UT_StringHolder>::iterator ptr;
	for (ptr = names.begin(); ptr < names.end(); ptr++) {
		std::cout << std::to_string(c) + " " + (*ptr).c_str() << std::endl;
		c++;
	}
	c = 0;

	std::cout << "Num counts: " + std::to_string(child_counts.sum()) << std::endl;
	UT_Array<int>::iterator ptr1;
	for (ptr1 = child_counts.begin(); ptr1 < child_counts.end(); ptr1++) {
		std::cout << std::to_string(c) + " " + std::to_string(*ptr1) << std::endl;
		c++;
	}
	c = 0;
	
	std::cout << "Num children: " + std::to_string(children.size()) << std::endl;
	UT_Array<int>::iterator ptr2;
	for (ptr2 = children.begin(); ptr2 < children.end(); ptr2++) {
		std::cout << std::to_string(c) + " " + std::to_string(*ptr2) << std::endl;
		c++;
	}*/

	if (!rig->construct(names, child_counts, children)) {
		std::cout << "Error in rig construction" << std::endl;
		return nullptr;
	}

	return rig;
}


/// Stores the initial rig-joint transformations and 
/// the weights of their influence on the geometry points
void PrototypeAgentPtr::addWeights(const GU_AgentRig& rig, 
	const GU_DetailHandle& geomHandle, std::vector<std::shared_ptr<BNode>>& inOrder,
	std::map<int, std::vector<GA_Offset>>& jointOffsets) {
	// Closely following SOP_BouncyAgent here
	GU_DetailHandleAutoWriteLock gdl(geomHandle);
	GU_Detail *gdp = gdl.getGdp();

	// Create joint regions and, for now, bind them to everything - TODO clean up
	int numRegions = inOrder.size();
	GEO_Detail::geo_NPairs pointDataPairs(1); ///(numRegions);

	GA_RWAttributeRef captAttr = gdp->addPointCaptureAttribute(pointDataPairs);

	// Uncertain what this is for
	int regions_i = -1;
	GA_AIFIndexPairObjects* regions =
		GEO_AttributeCaptureRegion::getBoneCaptureRegionObjects(captAttr, regions_i);
	regions->setObjectCount(numRegions);

	// Pass joint names to the capture / skinning attrs
	GEO_RWAttributeCapturePath paths(gdp);
	for (int i = 0; i < numRegions; i++) {
		paths.setPath(i, rig.transformName(i + 1));
	}
	
	// For comparison in distanced based weighting. TODO use for user geometry:
	///std::vector<UT_Vector3> jointOrigins = std::vector<UT_Vector3>();

	// Setting starting joint transformations here
	for (int i = 0; i < numRegions; i++) {
		std::shared_ptr<BNode> currNode = inOrder.at(i);
		///jointOrigins.push_back(currNode->getPos());

		UT_Matrix4 jointTrans = currNode->getWorldTransform();
		jointTrans.invert();

		GEO_CaptureBoneStorage boneTrans;
		boneTrans.myXform *= jointTrans;

		regions->setObjectValues(i, regions_i, boneTrans.floatPtr(),
			GEO_CaptureBoneStorage::tuple_size);
	}

	// And now weights per point in the geometry
	const GA_AIFIndexPair* weights = captAttr->getAIFIndexPair();
	weights->setEntries(captAttr, 1);///numRegions);

	// TODO - Option for user input and storing influence to nearest two joints per point
	/// COMMENTED OUT CODE CREATES WEIGHTS BY DISTANCE - CAN CHANGE FOR WHEN USERS CAN INPUT OWN GEOMETRY
	// TODO change to nearest joint only (or nearest joint plus parent and children?)
	/*for (GA_Offset ptoff : gdp->getPointRange()) {
		UT_Vector3 pt = gdp->getPos3(ptoff);

		float closestDist = std::numeric_limits<float>::max();
		int closestRegion = 0;//-1;

		for (int i = 0; i < numRegions; i++) {
			float dist = (jointOrigins.at(i) - pt).length();
			if (dist < closestDist) {
				closestDist = dist;
				closestRegion = i;
			}
		}

		weights->setIndex(captAttr, ptoff, 0, closestRegion); // entry, region
		weights->setData(captAttr, ptoff, 0, 1.f);

		//if (closestRegion >= 4 && closestRegion <= 12) {
		//	std::cout << std::to_string(closestRegion) + " " + std::to_string(pt(1)) << std::endl;
		//}
	}*/

	/// SAME RESULT AS (closest single point version of) ABOVE FOR DEFAULT TREE CONSTRUCTION
	std::map<int, std::vector<GA_Offset>>::iterator iter;
	for (iter = jointOffsets.begin(); iter != jointOffsets.end(); ++iter) {
		int currRegion = iter->first - 1;

		for (GA_Offset ptoff : iter->second) {
			weights->setIndex(captAttr, ptoff, 0, currRegion); /// entry, region
			weights->setData(captAttr, ptoff, 0, 1.f);
		}
	}
}


#define DEFAULT_SKIN_NAME   GU_AGENT_LAYER_DEFAULT".skin"
///#define COLLISION_SKIN_NAME GU_AGENT_LAYER_COLLISION".skin"

/// Stores geometry information for prototype (including weights)
GU_AgentShapeLibPtr PrototypeAgentPtr::createShapeLibrary(const char* path, 
	const GU_AgentRig& rig, GU_Detail* geo, std::vector<std::shared_ptr<BNode>>& inOrder,
	std::map<int, std::vector<GA_Offset>>& jointOffsets)
{
	UT_String libraryName = path;
	libraryName += "?proto";
	libraryName += std::to_string(PrototypeAgentPtr::protocount);
	libraryName += "shapelib";
	GU_AgentShapeLibPtr shapeLibrary = GU_AgentShapeLib::addLibrary(libraryName);

	GU_DetailHandle mainGeom;
	mainGeom.allocateAndSet(geo, true);
	addWeights(rig, mainGeom, inOrder, jointOffsets);

	shapeLibrary->addShape(DEFAULT_SKIN_NAME, mainGeom);

	// TODO Add sphere that surrounds all nodes
	///GU_DetailHandle coll_geo;
	///coll_geo.allocateAndSet(sopCreateSphere(false), /*own*/true);
	///shapelib->addShape(SOP_COLLISION_SKIN_NAME, coll_geo);

	return shapeLibrary;
}


/// Stores skin and deformer information
GU_AgentLayerPtr PrototypeAgentPtr::createStartLayer(const char* path, 
	const GU_AgentRigPtr& rig, const GU_AgentShapeLibPtr & shapeLibrary)
{
	UT_StringArray skinNames;
	skinNames.append(DEFAULT_SKIN_NAME);

	UT_IntArray transformIdx;
	transformIdx.append(0); /// because skin is at rig index 0

	UT_Array<GU_AgentShapeDeformerConstPtr> deformers;
	deformers.append(GU_AgentLayer::getLinearSkinDeformer());

	UT_String layerID = path;
	layerID += "?proto";
	layerID += std::to_string(PrototypeAgentPtr::protocount);
	layerID += "default_layer";
	GU_AgentLayerPtr startLayer = GU_AgentLayer::addLayer(layerID, rig, shapeLibrary);

	if (!startLayer->construct(skinNames, transformIdx, deformers)) {
		std::cout << "Error in layer construction" << std::endl;
	}

	UT_StringHolder layerName(UTmakeUnsafeRef(GU_AGENT_LAYER_DEFAULT));
	startLayer->setName(layerName);

	return startLayer;
}

/// Returns a definition defining an agent (protoype) type
GU_AgentDefinitionPtr
PrototypeAgentPtr::createDefinition(std::shared_ptr<BNode> root, const char* path) {
	PrototypeAgentPtr::protocount++;

	// Store the BNodes in order of tree height. Needed for ordering of rig joints
	std::vector<std::shared_ptr<BNode>> inOrder = std::vector<std::shared_ptr<BNode>>();

	// Create agent Rig
	GU_AgentRigPtr rig = createRig(path, root, inOrder);
	if (!rig) { 
		std::cout << "Rig is null" << std::endl;
		return nullptr;
	}

	/// Originally a temporary test to not have to choose joint influence by distance
	/// but might now be the default for our OWN GENERATED geometry
	std::map<int, std::vector<GA_Offset>> offsetsPerJoint = std::map<int, std::vector<GA_Offset>>();
	for (int i = 0; i < inOrder.size(); i++) {
		offsetsPerJoint.insert(pair<int, std::vector<GA_Offset>>(
			inOrder.at(i)->getRigIndex(), std::vector<GA_Offset>()
		));
	}
	/// WARNING: Do NOT use above for custom user geometry. See addWeights function for alternative

	// Create geometry
	// TODO add option for user input
	GU_Detail* geo = buildGeo(inOrder, offsetsPerJoint);

	// Set up geometry information for agent (including weights and (unincluded) collision shape)
	GU_AgentShapeLibPtr shapeLibrary = createShapeLibrary(path, *rig, geo, inOrder, offsetsPerJoint);
	if (!shapeLibrary) {
		std::cout << "ShapeLibrary is null" << std::endl;
		return nullptr;
	}

	// Sets up skinning and deformer information
	GU_AgentLayerPtr startLayer = createStartLayer(path, rig, shapeLibrary);
	if (!startLayer) {
		std::cout << "StartLayer is null" << std::endl;
		return nullptr;
	}

	// TODO collisionLayer

	GU_AgentDefinitionPtr definition(new GU_AgentDefinition(rig, shapeLibrary));
	definition->addLayer(startLayer);
	return definition;
}
