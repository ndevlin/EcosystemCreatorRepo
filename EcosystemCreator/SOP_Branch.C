#include "SOP_Branch.h"
#include "ECOSYSTEMPlugin.h"
using namespace HDK_Sample;

int HDK_Sample::branchIDnum = 0;

PRM_Template
SOP_Branch::myTemplateList[] = {
	// No custom parameters at the post-prototype branch level (as of now)
	PRM_Template()
};

// Here's how we define local variables for the SOP.
enum {
	VAR_PT		// Point number of the star
	//VAR_NPT		// Number of points in the star
};

CH_LocalVariable
SOP_Branch::myVariables[] = {
    { "PT",	VAR_PT, 0 },		// The table provides a mapping
    //{ "NPT", VAR_NPT, 0 },		// from text string to integer token
    { 0, 0, 0 },
};

bool
SOP_Branch::evalVariableValue(fpreal &val, int index, int thread)
{
    // myCurrPoint will be negative when we're not cooking so only try to
    // handle the local variables when we have a valid myCurrPoint index.
    if (myCurrPoint >= 0)
    {
		// Note that "gdp" may be null here, so we do the safe thing
		// and cache values we are interested in.
		switch (index)
		{
		case VAR_PT:
			val = (fpreal) myCurrPoint;
			return true;
		//case VAR_NPT:
		//	val = (fpreal) myTotalPoints;
		//	return true;
		default:
			/* do nothing */;
		}
    }
    // Not one of our variables, must delegate to the base class.
    return SOP_Node::evalVariableValue(val, index, thread);
}

OP_Node *
SOP_Branch::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
    return new SOP_Branch(net, name, op);
}

SOP_Branch::SOP_Branch(OP_Network *net, const char *name, OP_Operator *op)
	: SOP_Node(net, name, op), parentModule(nullptr), childModules(),
	moduleAgent(nullptr), packedPrim(nullptr), init_agent(true), change_agent(true), 
	branchID(branchIDnum++)
{
    myCurrPoint = -1;	// To prevent garbage values from being returned
}

SOP_Branch::~SOP_Branch() {}

unsigned
SOP_Branch::disableParms()
{
    return 0;
}

/// Traverse all nodes in this module to create cylinder geometry
void SOP_Branch::traverseAndBuild(GU_Detail* gdp, std::shared_ptr<BNode> currNode, int divisions) {
	std::shared_ptr<BNode> parent = currNode->getParent();

	UT_Vector3 start;
	if (parent) { start = parent->getPos(); }
	// TODO run setAge/calculatePos instead/first:
	UT_Vector3 end = currNode->getPos();

	std::vector<UT_Vector3> startCircle = std::vector<UT_Vector3>();
	std::vector<UT_Vector3> endCircle = std::vector<UT_Vector3>();

	// Calculate angle of geometry
	UT_Matrix3 transform = UT_Matrix3(1.0);
	if (parent) { transform.lookat(end, start); }
	else {
		// Or have circles flat by default
		UT_Vector3 look;
		look(0) = end(0);
		look(1) = end(1) + 1.0;
		look(2) = end(2);
		transform.lookat(end, look);
	}

	// Create angled circle shapes
	float ang = M_PI * 2 / divisions;
	for (int i = 0; i < divisions; i++) {
		// Get point along a circle
		UT_Vector3 point;
		point(0) = cos(ang * i);
		point(1) = sin(ang * i);
		point(2) = 0.0;
		// rotate to match the branch
		point = rowVecMult(point, transform);

		if (parent) { startCircle.push_back(start + point * parent->getThickness()); }
		endCircle.push_back(end + point * currNode->getThickness());
	}

	// Load points
	if (parent) {
		GU_PrimPoly* polyStart = GU_PrimPoly::build(gdp, divisions, GU_POLY_CLOSED);
		GU_PrimPoly* polyEnd = GU_PrimPoly::build(gdp, divisions, GU_POLY_CLOSED);

		for (int j = 0; j < divisions; j++) {
			GA_Offset ptoffStart = polyStart->getPointOffset(j);
			gdp->setPos3(ptoffStart, startCircle.at(j));

			GA_Offset ptoffEnd = polyEnd->getPointOffset(j);
			gdp->setPos3(ptoffEnd, endCircle.at(j));

			// Set individual rectangle faces
			GU_PrimPoly* polyRect = GU_PrimPoly::build(gdp, 4, GU_POLY_CLOSED);

			for (int k = 0; k < 2; k++) {
				GA_Offset ptoffRect = polyRect->getPointOffset(k);
				gdp->setPos3(ptoffRect, startCircle.at((j + k) % divisions));
			}

			for (int k = 0; k < 2; k++) {
				GA_Offset ptoffRect = polyRect->getPointOffset(k + 2);
				gdp->setPos3(ptoffRect, endCircle.at((j + 1 - k) % divisions));
			}
		}
	}
	// Otherwise, just draw a circle // TODO only do this if bnode has no children (which shouldnt happen) or time = 0, etc
	else {
		GU_PrimPoly* poly = GU_PrimPoly::build(gdp, divisions, GU_POLY_CLOSED);
		for (int j = 0; j < divisions; j++) {
			GA_Offset ptoff = poly->getPointOffset(j);
			gdp->setPos3(ptoff, endCircle.at(j));
		}
	}

	for (std::shared_ptr<BNode> child : currNode->getChildren()) {
		traverseAndBuild(gdp, child, divisions);
	}
}

void SOP_Branch::setTransforms(std::shared_ptr<BNode> currNode) {
	// Same effect
	//moduleAgent->setLocalTransform(currNode->getLocalTransform(), currNode->getRigIndex());
	UT_Matrix4 transform = currNode->getWorldTransform();
	transform.prescale(currNode->getThickness(), currNode->getThickness(), currNode->getThickness());
	moduleAgent->setWorldTransform(transform, currNode->getRigIndex());

	for (std::shared_ptr<BNode> child : currNode->getChildren()) {
		setTransforms(child);
	}
}

void drawSphereAtEachNode(GU_Detail* gdp, std::shared_ptr<BNode> currNode) {
	GU_PrimSphereParms sphere(gdp);
	sphere.xform.scale(0.15, 0.15, 0.15);
	sphere.xform.translate(currNode->getPos()(0), currNode->getPos()(1), currNode->getPos()(2));
	GU_PrimSphere::build(sphere, GEO_PRIMSPHERE);

	for (std::shared_ptr<BNode> child : currNode->getChildren()) {
		drawSphereAtEachNode(gdp, child);
	}
}

OP_ERROR
SOP_Branch::cookMySop(OP_Context &context)
{
	//std::cout << "__BRANCH" + std::to_string(branchID) + " start" << std::endl;
	fpreal now = context.getTime();
	//flags().setTimeDep(false);

	// Cylinder variables
    UT_Interrupt	*boss;
	myCurrPoint = 0;		// Initialize the PT local variable
	int divisions  = 10.0;	// The divisions per cylinder
    // myTotalPoints  Unneeded

	// If the root node
	// Explicitly state dependency on plant, so that this is dirtied every time plant updates
	if (plant && !parentModule) {
		addExtraInput(plant, OP_INTEREST_DATA);
	}
	// Else state dependency on parent branch module
	else if (parentModule) {
		parentModule->getOutputTowardsNode(this); // <-Use some function here to interact with data
		addExtraInput(parentModule, OP_INTEREST_DATA); // ...in order to call this
	}

    // Check to see that there hasn't been a critical error in cooking the SOP.
    if (error() < UT_ERROR_ABORT)
    {
		boss = UTgetInterrupt();
		gdp->clearAndDestroy();

		// Start the interrupt server
		if (boss->opStart("Building ECOSYSTEM"))
		{
			// Traverse the node structure and make a cylinder for each branch
			//traverseAndBuild(gdp, root, divisions); 
			//if (init_agent) {
			init_agent = false;

			GU_PrimPoly* pointModule = GU_PrimPoly::build(gdp, 1, GU_POLY_OPEN);
			//gdp->destroyPrimitives(gdp->getPrimitiveRange());
			GA_Offset ptoff = pointModule->getPointOffset(0);
			UT_Vector3 pt;
			pt(0) = 0.0;
			pt(1) = 0.0;
			pt(2) = 0.0;
			gdp->setPos3(ptoff, pt);

			// TEST
			//gdp->destroyPrimitives(gdp->getPrimitiveRange());
			//GA_Offset ptoff2;
			//for (GA_Offset ptoff3 : gdp->getPointRange()) {
			//	ptoff2 = ptoff3;
			//	break;
			//}

			packedPrim = GU_Agent::agent(*gdp, ptoff);//2);
			gdp->getAttributes().bumpAllDataIds(GA_ATTRIB_VERTEX);
			gdp->getAttributes().bumpAllDataIds(GA_ATTRIB_PRIMITIVE);
			gdp->getPrimitiveList().bumpDataId();

			// Adding a name
			GA_RWHandleS name_attr(gdp->addStringTuple(GA_ATTRIB_PRIMITIVE, "name", 1));

			moduleAgent = UTverify_cast<GU_Agent*>(packedPrim->hardenImplementation());
			//}
			//if (change_agent) {
			change_agent = false;
			int currIdx = prototype->getIdxAtTimestep(root->getAge());
			GU_AgentDefinitionPtr ptrTemp = prototype->getAgentDefAtIdx(currIdx);
			moduleAgent->setDefinition(packedPrim, ptrTemp);

			GU_AgentLayerConstPtr currLayer = ptrTemp->layer(UTmakeUnsafeRef(GU_AGENT_LAYER_DEFAULT));
			moduleAgent->setCurrentLayer(packedPrim, currLayer);
			
			setTransforms(root);

			// Finishing name
			UT_WorkBuffer currName;
			UT_String agentName;
			currName.sprintf(("agentname_" + std::to_string(branchID)).c_str(), 
				agentName.buffer(), 0);
			name_attr.set(packedPrim->getMapOffset(), currName.buffer());

			//moduleAgent = UTverify_cast<GU_Agent*>(packedPrim->hardenImplementation());
			gdp->getPrimitiveList().bumpDataId();/**/

			// Testing ideal joint locations
			//drawSphereAtEachNode(gdp, root);

			// Clear any highlighted geometry and highlight the primitives we generated.
			select(GU_SPrimitive);
		}

		// Must tell the interrupt server that we've completed.
		boss->opEnd();
    }
	triggerOutputChanged();

    myCurrPoint = -1;
	//std::cout << "__BRANCH" + std::to_string(branchID) + " end" << std::endl;
    return error();
}

/// Set up plant pointer, selected prototype data, and initializes root and ageRange
void SOP_Branch::setPlantAndPrototype(SOP_Plant* p, float lambda, float determ) {
	plant = p;
	prototype = plant->copyPrototypeFromList(lambda, determ);

	currAgeRange = prototype->getRangeAtIdx(0);
	root = prototype->getRootAtIdx(0);
}

/// While setting the parent module, also alters current node data based on last branch
void SOP_Branch::setParentModule(SOP_Branch* parModule, std::shared_ptr<BNode> connectingNode) {
	parentModule = parModule;
	if (connectingNode) { 
		// Get starting radius of model
		bool adjustRadius = connectingNode->getBaseRadius() !=
			prototype->getRootAtIdx(0)->getBaseRadius();

		float radiusMultiplier;
		if (adjustRadius) {
			radiusMultiplier = connectingNode->getBaseRadius() / 
				prototype->getRootAtIdx(0)->getBaseRadius();
		}

		// Get starting orientation of model based off of parent branch
		/*UT_Matrix3 transform = UT_Matrix3(1.0);
		transform.rotate<UT_Axis3::XAXIS>(1.571);

		UT_Matrix3 transformLook = UT_Matrix3(1.0);
		transformLook.lookat(UT_Vector3(0.0f, 0.0f, 0.0f), -1.0f * connectingNode->getDir());
		transform *= transformLook;*/
		UT_Vector3 c = UT_Vector3();
		UT_Matrix3 transform = UT_Matrix3::dihedral(UT_Vector3(0.0f, 1.0f, 0.0f),
			connectingNode->getDir(), c, 1);
		//UT_Matrix3 transform = UT_Matrix3::dihedral(connectingNode->getDir(), 
		//	UT_Vector3(0.0f, 1.0f, 0.0f), c, 1);
		//transform.prescale(1.0f, -1.0f, 1.0f);
		// TODO^ good starting point and will then get replaced by placement optimization

		// Update the values for each prototype age
		for (int i = 0; i < prototype->getNumAges(); i++) {
			prototype->getRootAtIdx(i)->setParent(connectingNode);

			if (adjustRadius) {
				prototype->getRootAtIdx(i)->recThicknessUpdate(radiusMultiplier);
			}
			// experimental
			prototype->getRootAtIdx(i)->recLengthUpdate(connectingNode->getMaxLength() * 0.8);

			// experimental#2
			prototype->getRootAtIdx(i)->recRotate(transform);
		}
	}
}

/// Important: updates all time-based values in all modules. Does all main calculations
void SOP_Branch::setAge(float changeInAge) {
	if (prototype) {
		float ageVal = changeInAge + root->getAge();
		bool mature = false;
		bool decay = ageVal < prototype->getMaturityAge() &&
			root->getAge() >= prototype->getMaturityAge();

		// Check that we have the right prototype age
		if (ageVal >= prototype->getMaturityAge()) {
			// Double check that node is from the oldest prototype
			root = prototype->getRootAtIdx(prototype->getNumAges() - 1);
			mature = (root->getAge() < prototype->getMaturityAge()); 
			// ^only if it was immature in the prior step - TODO change when adding vigor
		}
		else if (!BranchPrototype::isInRange(currAgeRange, ageVal)) {
			setRootByAge(ageVal);
		}

		// if mature, get terminal nodes to connect to with more modules
		//std::vector<BNode*> terminalNodes = std::vector<BNode*>();
		std::vector<std::shared_ptr<BNode>> terminalNodes = std::vector<std::shared_ptr<BNode>>();
		//std::vector<SOP_Branch*> newModules = std::vector<SOP_Branch*>();

		// Set the present age along the tree and adjusts current point calculations
		// TODO calculate and use growth rate later
		root->setAge(changeInAge, currAgeRange, terminalNodes, mature, decay);

		if (mature) { // - unneccessary double check
			for (std::shared_ptr<BNode> terminalNode : terminalNodes) {
				SOP_Branch* newModule = (SOP_Branch*)plant->createNode("BranchModule");

				if (!newModule) { std::cout << "Child Node is Nullptr" << std::endl; }
				else if (!newModule->runCreateScript()) { std::cout << "Constuction error" << std::endl; }

				// TODO: set lambda and determ properly
				//newModule->setBypass(false);
				// WARNING, when swapping the order of this, real plant age would be plant->getAge() - changeInAge
				newModule->setPlantAndPrototype(plant, plant->getAge() / 8.f, plant->getAge() / 8.f);
				//newModule->setPlantAndPrototype(plant, 0.0f, 0.0f);
				newModule->setParentModule(this, terminalNode);
				newModule->setAge(0.0f);
				newModule->setInput(0, this);
				plant->addToMerger(newModule);
				newModule->moveToGoodPosition();
				//newModule->getOutputNodes TODO check out that

				// TODO maybe add specific rendering pipelines here
				//OP_Node* colorNode = plant->createNode("color");

				terminalNode->addModuleChild(newModule);
				childModules.push_back(newModule);
			}
		}
		else if (decay && !childModules.empty()) {
			childModules.clear(); // actual destruction is handled in BNode
		}

		// TODO, go through new Modules and run optimize orientation steps
		// Based on bounding volumes. maybe use commented-out newModules vector
	}

	// Recurse
	for (SOP_Branch* child : childModules) {
		child->setAge(changeInAge);
	}

	// Now add flux calculations here ^ while the recursion returns to the root
	// Needs functioning bounding volumes first. Outline:
	// have setAge return flux
	//  - sum all intersecting volumes for each module
	//  - use exponential decay on the negative of that summation to calculate exposure
	//  - sum this flux at each branching point until we reach root

	// Then vigor distribution. Be mindful of maxes and mins. Establish apical control variables
}

/// Swaps to whichever is the currently-aged prototype to reference
void SOP_Branch::setRootByAge(float time) {
	if (prototype) {
		int idx = prototype->getIdxAtTimestep(time);
		currAgeRange = prototype->getRangeAtIdx(idx);
		root = prototype->getRootAtIdx(idx);
	}
}

void SOP_Branch::destroySelf() {
	// TODO Maybe also deleteData? And make sure to remove from merger
	disconnectAllInputs();
	disconnectAllOutputs();
	plant->destroyNode(this);
}