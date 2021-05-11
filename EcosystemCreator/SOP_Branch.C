#include "SOP_Branch.h"
#include "ECOSYSTEMPlugin.h"
using namespace HDK_Sample;

int HDK_Sample::branchIDnum = 0;

// Declaring parameters here
PRM_Template
SOP_Branch::myTemplateList[] = {
	// No custom parameters at the post-prototype branch level
	PRM_Template()
};

// And the local variables for this SOP
enum {
	VAR_PT		// Point number  - just used to tell when cooking
};

CH_LocalVariable
SOP_Branch::myVariables[] = {
    { "PT",	VAR_PT, 0 },		// The table provides a mapping
    { 0, 0, 0 },				// from text string to integer token
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
		default:
			/* do nothing */;
		}
    }
    // Not one of our variables, must delegate to the base class.
    return SOP_Node::evalVariableValue(val, index, thread);
}

////////////////////////////////////////////////////////////////////////////////

//////////////////// HOUDINI FUNCTIONS FOR NODE CONTROL ////////////////////////

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

SOP_Branch::~SOP_Branch() {
	for (SOP_Branch* childModule : childModules) {
		childModule->destroySelf();
	}
	childModules.clear();
}

unsigned
SOP_Branch::disableParms()
{
    return 0;
}

/// A debugging helper function that draws spheres at each BNode position
void drawSphereAtEachNode(GU_Detail* gdp, std::shared_ptr<BNode> currNode) {
	GU_PrimSphereParms sphere(gdp);
	sphere.xform.scale(0.15, 0.15, 0.15);
	sphere.xform.translate(currNode->getPos()(0), currNode->getPos()(1), currNode->getPos()(2));
	GU_PrimSphere::build(sphere, GEO_PRIMSPHERE);

	for (std::shared_ptr<BNode> child : currNode->getChildren()) {
		drawSphereAtEachNode(gdp, child);
	}
}

/// Does the actual work of the SOP Branch computing
OP_ERROR
SOP_Branch::cookMySop(OP_Context &context)
{	
	if (!plant) {
		std::cout << "ERROR: Branch Sop must be initialized BY a Plant Sop" << std::endl;
		addError(SOP_ERR_BADNODE, "Branch Sop must be initialized BY a Plant Sop");
		return UT_ERROR_WARNING;
	}
	
	fpreal now = context.getTime();

    UT_Interrupt	*boss;
	myCurrPoint = 0;		// Initialize the PT local variable

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
			/// UPDATE AGE
			// Can do this comparison now because Plant sets it's age AFTER the 
			// child sop branch modules finish cooking. If that changes, fix here
			
			if (plant && prototype) { // To be safe
				setAge(plant->getChangeInAge());

				/// CREATE AN AGENT FOR EACH BRANCH MODULE
				init_agent = false;

				GU_PrimPoly* pointModule = GU_PrimPoly::build(gdp, 1, GU_POLY_OPEN);
				GA_Offset ptoff = pointModule->getPointOffset(0);

				gdp->setPos3(ptoff, plant->getPosition());

				packedPrim = GU_Agent::agent(*gdp, ptoff);
				gdp->getAttributes().bumpAllDataIds(GA_ATTRIB_VERTEX);
				gdp->getAttributes().bumpAllDataIds(GA_ATTRIB_PRIMITIVE);
				gdp->getPrimitiveList().bumpDataId();

				// Adding a name
				GA_RWHandleS name_attr(gdp->addStringTuple(GA_ATTRIB_PRIMITIVE, "name", 1));

				moduleAgent = UTverify_cast<GU_Agent*>(packedPrim->hardenImplementation());

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

				gdp->getPrimitiveList().bumpDataId();
			}

			// Clear any highlighted geometry and highlight the primitives we generated.
			select(GU_SPrimitive);
		}

		// Must tell the interrupt server that we've completed.
		boss->opEnd();
    }
	triggerOutputChanged();

    myCurrPoint = -1;
    return error();
}


//////////////////// OUR FUNCTIONS FOR BRANCH SOP MANAGEMENT ///////////////////

/// Important: updates all time-based values in all modules. Does all main calculations
void SOP_Branch::setAge(float changeInAge) {
	if (prototype) {
		float newAge = changeInAge + root->getAge();
		bool mature = false;
		bool decay = newAge < prototype->getMaturityAge() &&
			root->getAge() >= prototype->getMaturityAge(); // Aka no longer mature

		// Check that we have the right prototype age
		if (newAge >= prototype->getMaturityAge()) {
			mature = (root->getAge() < prototype->getMaturityAge());
			// ^only if it was immature in the prior step - TODO change when adding vigor
			// Double check that node is from the oldest prototype
			root = prototype->getRootAtIdx(prototype->getNumAges() - 1);
		}
		else if (!BranchPrototype::isInRange(currAgeRange, newAge)) {
			setRootByAge(newAge);
		}

		// Recalculate difference in age, since root may have changed
		float ageDif = newAge - root->getAge(); /// Only use for these BNodes

		// if mature, get terminal nodes to connect to with more modules
		std::vector<std::shared_ptr<BNode>> terminalNodes = std::vector<std::shared_ptr<BNode>>();

		// Set the present age along the tree and adjusts current point calculations
		root->setAge(ageDif, terminalNodes, mature, decay);

		if (mature) { // - unneccessary double check
			float termNodeAge = newAge - prototype->getMaturityAge();
			for (std::shared_ptr<BNode> terminalNode : terminalNodes) {
				SOP_Branch* newModule = (SOP_Branch*)plant->createNode("BranchModule");

				if (!newModule) { std::cout << "Branch Node is Nullptr" << std::endl; }
				else if (!newModule->runCreateScript()) { std::cout << "Branch constuction error" << std::endl; }

				// WARNING, when swapping the order of this, real plant age would be plant->getAge() - changeInAge
				newModule->setPlantAndPrototype(plant, plant->getAge() / 8.f, plant->getAge() / 8.f);
				newModule->setParentModule(this, termNodeAge, terminalNode);
				newModule->setInput(0, this);
				plant->addToMerger(newModule);
				// Doesn't cook this round even with forceRecook

				terminalNode->addModuleChild(newModule);
				childModules.push_back(newModule);
			}
		}
		else if (decay && !childModules.empty()) {
			childModules.clear(); // actual destruction is handled in BNode
		}
	}
}

/// Set up plant pointer, selected prototype data, and initializes root and ageRange
void SOP_Branch::setPlantAndPrototype(SOP_Plant* p, float lambda, float determ) {
	plant = p;
	prototype = plant->copyPrototypeFromList(lambda, determ);

	currAgeRange = prototype->getRangeAtIdx(0);
	root = prototype->getRootAtIdx(0);
}

/// While setting the parent module, also alters current node data based on last branch
void SOP_Branch::setParentModule(SOP_Branch* parModule, float newAge,
	std::shared_ptr<BNode> connectingNode) {
	parentModule = parModule;
	if (connectingNode && prototype) { 
		// Get starting radius of model
		bool adjustRadius = connectingNode->getBaseRadius() !=
			prototype->getRootAtIdx(0)->getBaseRadius();

		float radiusMultiplier = 1.0f;
		if (adjustRadius) {
			radiusMultiplier = connectingNode->getBaseRadius() / 
				prototype->getRootAtIdx(0)->getBaseRadius();
		}

		// Get starting orientation of model based off of parent branch
		UT_Vector3 c = UT_Vector3();
		UT_Matrix3 transform = UT_Matrix3::dihedral(UT_Vector3(0.0f, 1.0f, 0.0f),
			connectingNode->getDir(), c, 1);

		// Update the values for each prototype age
		for (int i = 0; i < prototype->getNumAges(); i++) {
			prototype->getRootAtIdx(i)->setParent(connectingNode);

			float ageDif = newAge - prototype->getRootAtIdx(i)->getAge();
			prototype->getRootAtIdx(i)->recTransformation(ageDif,
				radiusMultiplier, connectingNode->getMaxLength() * 0.8, transform);
		}
	}
}

/// Update the current agent rig with the transformations of nodes
void SOP_Branch::setTransforms(std::shared_ptr<BNode> currNode) {
	// Get the "world" transform of the current node - technically, with respect to Plant
	UT_Matrix4 transform = currNode->getWorldTransform();
	transform.prescale(currNode->getThickness(), currNode->getThickness(), currNode->getThickness());

	// Update the rig joint to correspond with this node
	moduleAgent->setWorldTransform(transform, currNode->getRigIndex());

	// Repeat for all nodes in the BranchModule
	for (std::shared_ptr<BNode> child : currNode->getChildren()) {
		setTransforms(child);
	}
}

/// Swaps tree beginning at "root" to be the appropriately aged prototype copy
void SOP_Branch::setRootByAge(float time) {
	if (prototype) {
		int idx = prototype->getIdxAtTimestep(time);
		currAgeRange = prototype->getRangeAtIdx(idx);
		root = prototype->getRootAtIdx(idx);
	}
}

/// Disconnect and delete this SOP_Branch
void SOP_Branch::destroySelf() {
	disconnectAllInputs();
	disconnectAllOutputs();
	unloadData();
	plant->destroyNode(this);
}

