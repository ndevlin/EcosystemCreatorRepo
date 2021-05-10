#include "SOP_Plant.h"
#include "ECOSYSTEMPlugin.h"
using namespace HDK_Sample;

/// Custom filter for child node, allows all operator types right now
bool SOP_CustomSopOperatorFilter::allowOperatorAsChild(OP_Operator *op)
{
	// TODO change to just SOP operators maybe
	return true;//(dynamic_cast<sop_CustomVopOperator *>(op) != NULL);
}


// Declaring parameters here
static PRM_Name	plantAgeName("plantAge", "Plant Age");
///static PRM_Name	      g1Name("g1",       "Tropism Falloff"); /// TropismDecrease
///static PRM_Name	      g2Name("g2",       "Tropism Strength (temp)");
//				             ^^^^^^^^     ^^^^^^^^^^^^^^^
//				             internal     descriptive version

// Set up the initial/default values for the parameters
static PRM_Default plantAgeDefault(0, "0.0");
///static PRM_Default	     g1Default(1.0);
///static PRM_Default	     g2Default(-0.2);

// Set up the ranges for the parameter inputs here
// TODO change maximum based on PlantSpecies max age
//static PRM_Range plantAgeRange(PRM_RANGE_RESTRICTED,  0.0, PRM_RANGE_UI, 8.0);
///static PRM_Range       g1Range(PRM_RANGE_RESTRICTED,  0.0, PRM_RANGE_UI, 3.0);
///static PRM_Range       g2Range(PRM_RANGE_RESTRICTED, -1.0, PRM_RANGE_UI, 1.0);

////////////////////////////////////////////////////////////////////////////////

// Put all the parameters together for the UI
PRM_Template
SOP_Plant::myTemplateList[] = {
	PRM_Template(PRM_STRING, PRM_Template::PRM_EXPORT_MIN, 1, &plantAgeName, &plantAgeDefault),
	///PRM_Template(PRM_FLT,    PRM_Template::PRM_EXPORT_MIN, 1, &g1Name,       &g1Default,       0, &g1Range),
	///PRM_Template(PRM_FLT,    PRM_Template::PRM_EXPORT_MIN, 1, &g2Name,       &g2Default,       0, &g2Range),
	PRM_Template()
};

/// Variable funcs

// Defining local variable(s)
enum {
	VAR_PT		// Point number  - just used to tell when cooking
};

CH_LocalVariable
SOP_Plant::myVariables[] = {
    { "PT",	VAR_PT, 0 },		// The table provides a mapping
    { 0, 0, 0 },				// from text string to integer token
};

/// Table type for this sopnet
const char *SOP_Plant::theChildTableName = SOP_TABLE_NAME;

/// May be of use later
bool
SOP_Plant::evalVariableValue(fpreal &val, int index, int thread)
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

/// Initialize plant sop and important children (merger and output)
OP_Node *
SOP_Plant::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
	SOP_Plant* newPlant = new SOP_Plant(net, name, op);

	//// Create a merge node to merge all SOP_Branch output geom
	OP_Node* mergeNode = newPlant->createNode("merge");

	if (!mergeNode) { std::cout << "Merge Node is Nullptr" << std::endl; }
	else if (!mergeNode->runCreateScript())
		std::cout << "Merge constructor error" << std::endl;

	if (mergeNode) {
		mergeNode->moveToGoodPosition();
		newPlant->setMerger(mergeNode);
	}

	//// Create an output node for the network
	OP_Node* outNode = newPlant->createNode("output");
	
	if (!outNode) { std::cout << "Output Node is Nullptr" << std::endl; }
	else if (!outNode->runCreateScript())
		std::cout << "Output constructor error" << std::endl;

	SOP_Node* outNode_sop = (SOP_Node*)outNode;

	if (outNode_sop) {
		outNode_sop->moveToGoodPosition();

		// Also handles connection to mergeNode aka newPlant->merger
		newPlant->setOutput(outNode_sop);
	}

	newPlant->moveToGoodPosition();

	// TODO move color handling to agent

	//
	//// Color for bark
	//OP_Node* color1 = newPlant->createNode("color");
	//if (!color1)
	//{
	//	std::cout << "Color is Nullptr" << std::endl;
	//	return newPlant;
	//}
	//else if (!color1->runCreateScript())
	//	std::cout << "Color constructor error" << std::endl;
	//
	//
	//OP_Node* mergeNode2 = newPlant->createNode("merge");
	//if (!mergeNode2) { std::cout << "Merge Node is Nullptr" << std::endl; }
	//else if (!mergeNode2->runCreateScript())
	//	std::cout << "Merge constructor error" << std::endl;
	//
	//
	//
	//// Copy to Points to allow leaf instancing
	//OP_Node* copyToPoints = newPlant->createNode("copytopoints");
	//if (!copyToPoints) 
	//{ 
	//	std::cout << "Copy To Points is Nullptr" << std::endl; 
	//	return newPlant;
	//}
	//else if (!copyToPoints->runCreateScript())
	//	std::cout << "Copy To Points constructor error" << std::endl;

	// Star will serve as a leaf for now
	/*OP_Node* star = newPlant->createNode("star");
	if (!star)
	{
		std::cout << "Star is Nullptr" << std::endl;
		return newPlant;
	}
	else if (!star->runCreateScript())
		std::cout << "Star constructor error" << std::endl;

	star->moveToGoodPosition();

	// Color for leaves
	OP_Node* color2 = newPlant->createNode("color");
	if (!color2)
	{
		std::cout << "Color is Nullptr" << std::endl;
		return newPlant;
	}
	else if (!color2->runCreateScript())
		std::cout << "Color constructor error" << std::endl;

	color2->connectToInputNode(*star, 0, 0);
	color2->moveToGoodPosition();*/


	//copyToPoints->connectToInputNode(*mergeNode, 1, 0);
	//
	//mergeNode2->connectToInputNode(*copyToPoints, 0, 0);
	//
	//mergeNode2->connectToInputNode(*color1, 1, 0);
	//
	//copyToPoints->moveToGoodPosition();
	//
	//mergeNode2->moveToGoodPosition();


	/*OP_Node* mergeNode3 = newPlant->createNode("merge");
	if (!mergeNode3) { std::cout << "Merge Node is Nullptr" << std::endl; }
	else if (!mergeNode3->runCreateScript())
		std::cout << "Merge constructor error" << std::endl;

	// copyToPoints2 to create instances of the tree for a forest
	OP_Node* copyToPoints2 = newPlant->createNode("copytopoints");
	if (!copyToPoints2)
	{
		std::cout << "Copy To Points is Nullptr" << std::endl;
		return newPlant;
	}
	else if (!copyToPoints2->runCreateScript())
		std::cout << "Copy To Points constructor error" << std::endl;


	
	// Grid to represent ground
	OP_Node* grid = newPlant->createNode("grid");
	if (!grid)
	{
		std::cout << "Grid is Nullptr" << std::endl;
		return newPlant;
	}
	else if (!grid->runCreateScript())
		std::cout << "Grid constructor error" << std::endl;

	grid->moveToGoodPosition();
	

	// Mountain to vary terrain
	OP_Node* mountain = newPlant->createNode("mountain");
	if (!mountain)
	{
		std::cout << "Mountain is Nullptr" << std::endl;
		return newPlant;
	}
	else if (!mountain->runCreateScript())
		std::cout << "Mountain constructor error" << std::endl;

	mountain->connectToInputNode(*grid, 0, 0);
	mountain->moveToGoodPosition();


	// Color for leaves
	OP_Node* color3 = newPlant->createNode("color");
	if (!color3)
	{
		std::cout << "Color is Nullptr" << std::endl;
		return newPlant;
	}
	else if (!color3->runCreateScript())
		std::cout << "Color constructor error" << std::endl;


	// Scatter to create points
	OP_Node* scatter = newPlant->createNode("scatter");
	if (!scatter)
	{
		std::cout << "Scatter is Nullptr" << std::endl;
		return newPlant;
	}
	else if (!scatter->runCreateScript())
		std::cout << "Scatter constructor error" << std::endl;

	scatter->connectToInputNode(*mountain, 0, 0);
	scatter->moveToGoodPosition();


	copyToPoints2->connectToInputNode(*mergeNode2, 0, 0);
	copyToPoints2->connectToInputNode(*scatter, 1, 0);
	copyToPoints2->moveToGoodPosition();


	mergeNode3->connectToInputNode(*copyToPoints2, 0, 0);
	mergeNode3->connectToInputNode(*color3, 1, 0);
	mergeNode3->moveToGoodPosition();

	color3->connectToInputNode(*mountain, 0, 0);
	color3->moveToGoodPosition();*/
	
	//color1->connectToInputNode(*mergeNode, 0, 0);
	//color1->moveToGoodPosition();

	return newPlant;
}

SOP_Plant::SOP_Plant(OP_Network *net, const char *name, OP_Operator *op)
	: SOP_Node(net, name, op), ecosystem(nullptr), plantSpecies(nullptr),
	rootModule(nullptr), merger(nullptr)
{
	createAndGetOperatorTable();
	myCurrPoint = -1;	// To prevent garbage values from being returned
	
	plantAge = 0.0f;
	plantBirthday = 0.0f;
	///setAllowBuildDependencies
	///mySopFlags.setManagesDataIDs(true);
}

SOP_Plant::~SOP_Plant() {}

/// Unsure if we'll need this
//unsigned
//SOP_Plant::disableParms()
//{
//    return 0;
//}

/// Do the actual Plant SOP computing
OP_ERROR
SOP_Plant::cookMySop(OP_Context &context)
{
	//std::cout << "PLANT COOK START" << std::endl;
	if (!ecosystem) {
		// If it was initialized in an ecosystem, but not BY ecosystem...
		auto eco = dynamic_cast<OBJ_Ecosystem*>(getParent()->getOperator());
		if (eco != NULL) {
			// Update with the correct pointers and a RANDOM species
			if (rootModule) { rootModule->destroySelf(); }
			initPlant((OBJ_Ecosystem*)eco, eco->chooseSpecies(), eco->getAge());
			eco->addToMerger(this);
		}
		// Else notify user of error
		else {
			std::cout << "ERROR: Plant Sop must be initialized inside an Ecosystem Geo" << std::endl;
			addError(SOP_ERR_BADNODE, "Plant Sop must be initialized inside an Ecosystem Geo");
			return UT_ERROR_WARNING;
		}
	}

	fpreal now = context.getTime();
	UT_Interrupt	*boss;
	myCurrPoint = 0;
	//flags().setTimeDep(false);

	// Check if custom plant age has been adjusted
	//float diff = ageVal - plantAge;
	//if (abs(diff) > 0.00005f) {
	//	plantAge = ageVal;
	//	plantBirthday += diff;
	//}
    
    // For ecosystem
	if (ecosystem) {
		addExtraInput(ecosystem, OP_INTEREST_DATA);
	}
	if (plantSpecies) {
		addExtraInput(plantSpecies, OP_INTEREST_DATA);
	}

	if (error() < UT_ERROR_ABORT)
	{
		boss = UTgetInterrupt();

		//if (rootModule) {
		//	// TODO delete plant if age is over max age or under 0;
		//	rootModule->setAge(getChangeInAge());
		//}

		// WARNING - does not update after you leave network until next cook
		output = (SOP_Node*)getDisplayNodePtr();
		if (output) {
			// TODO find a better way, "instance" it maybe
			gdp->stashAll();
			// WARNING - Plant cook encompasses all children cooks.
			// If we somehow change this so that output is cooked after plantAge 
			// is set, we must also change how SOP_Branches test for age change
			gdp->merge(*output->getCookedGeo(context), nullptr, true, false, nullptr, true, GA_DATA_ID_CLONE);
			gdp->destroyStashed();
		}
		// Could also prob use "notifyVarChange" to dirty children, but only the above so far has 
		// resulted in the displaying the geometry for OBJ_Ecosystem

		// Update plant age and display in a disabled parameter so the user can see
		plantAge = calcWeightedAge();
		if (ecosystem->getAge() < plantBirthday) { plantAge = 0.0f; } // TODO delete plant instead
		else if (plantAge > plantSpecies->getMaxAge()) { plantAge = plantSpecies->getMaxAge(); }
		setFloat("plantAge", 0, now, plantAge);
		enableParm("plantAge", false);

		// TODO traverse down again to get flux
		// Maybe handle vigor here too

		// Must tell the interrupt server that we've completed.
		boss->opEnd();
	}
	triggerOutputChanged();

    myCurrPoint = -1;
	return error();
}


/// Unsure if this is needed anymore
GU_DetailHandle
SOP_Plant::cookMySopOutput(OP_Context &context, int outputidx, SOP_Node* interests) {
	if (output) {
		return output->cookOutput(context, outputidx, interests);
	}
	else {
		return SOP_Node::cookMySopOutput(context, outputidx, interests);
	}
}


///////////////////// OUR FUNCTIONS FOR PLANT SOP MANAGEMENT ///////////////////

/// Initialize the actual plant based on the environment (the root SOP_Branch)
void SOP_Plant::initPlant(OBJ_Ecosystem* eco, PlantSpecies* currSpecies, float worldTime)
{
	ecosystem = eco;
	plantBirthday = worldTime;

	if (currSpecies) {
		plantSpecies = currSpecies;

		if (merger) {
			// Create the root branch module
			OP_Node* branchNode_OP = createNode("BranchModule");

			if (!branchNode_OP) { std::cout << "Root module is Nullptr" << std::endl; }
			else if (!branchNode_OP->runCreateScript())
				std::cout << "Root module constructor error" << std::endl;

			if (branchNode_OP) {
				SOP_Branch* branchNode = (SOP_Branch*)branchNode_OP;
				setRootModule(branchNode);
				addToMerger(branchNode);
				branchNode_OP->moveToGoodPosition();
			}
		}
		triggerUIChanged(OP_UICHANGE_CONNECTIONS);
	}
	else {
		std::cout << "NO PLANT TYPES ARE DEFINED" << std::endl;
	}
}

/// Generate a prototype copy to store as an editable tree in a SOP_Branch
BranchPrototype* SOP_Plant::copyPrototypeFromList(float lambda, float determ) {
	return plantSpecies->copyPrototypeFromList(lambda, determ);
}

/// Add the corresponding node as an input to the stored merge node
void SOP_Plant::addToMerger(SOP_Branch* bMod) {
	if (merger) {
		// Get unique input path
		UT_String path;
		bMod->getFullPath(path);
		merger->setNamedInput(path.hash(), bMod);
	}
}

/// Get the age of this plant
float SOP_Plant::getAge() {
	return plantAge;
}

/// Calculate the age impacted by growth rate
float SOP_Plant::calcWeightedAge() {
	return (ecosystem->getAge() - plantBirthday) * plantSpecies->getGrowthRate();
}

/// Get the change in age - only works mid-cook
float SOP_Plant::getChangeInAge() {
	// TODO When implementing seeding, should delete plant if ecosystem age < birthday
	// But for now  for consistency with random distribution
	if (ecosystem->getAge() < plantBirthday) {
		return plantBirthday - plantAge;
	}
	// Same here, delete plant when over max age
	float weightedAge = calcWeightedAge();
	if (weightedAge > plantSpecies->getMaxAge()) {
		return plantSpecies->getMaxAge() - plantAge;
	}

	return ecosystem->getAge() - (plantAge + plantBirthday);
}

/// Sets up the root SOP_Branch module
void SOP_Plant::setRootModule(SOP_Branch* node) {
	rootModule = node;

	if (rootModule) {
		rootModule->setPlantAndPrototype(this, 0.0f, 0.0f);
		rootModule->setAge(0.0f);
	}
}

/// Stores the merge node that combines all branch geometry
void SOP_Plant::setMerger(OP_Node* mergeNode) {
	merger = mergeNode;
}

/// Stores the initial output of the network, sets as display/render node
void SOP_Plant::setOutput(SOP_Node* outNode) {
	output = outNode;

	//myDisplayNodePtr = output;
	//myRenderNodePtr = output;
	//myOutputNodes.append(output);

	if (output) {
		output->setDisplay(true);
		output->setRender(true);

		if (merger) { output->connectToInputNode(*merger, 0, 0); }
	}
}

/////////////////// HOUDINI FUNCTIONS FOR NETWORK FEATURES /////////////////////

/// Setting up the custom operator table for this network
OP_OperatorTable *
SOP_Plant::createAndGetOperatorTable()
{
	// Chain custom SOP operators onto the default SOP operator table
	OP_OperatorTable &table = *OP_Network::getOperatorTable(SOP_TABLE_NAME);
	// TODO maybe add Branch Module here since it's dependent on parent // wait no

	// Notify observers of the operator table that it has been changed.
	table.notifyUpdateTableSinksOfUpdate();

	return &table;
}

/// Tells houdini this node can contain children
int SOP_Plant::isNetwork() const
{
	return 1;
}

/// Tells houdini this is a subnetwork
int SOP_Plant::isSubNetwork(bool includemanagementops) const
{
	return 1;
}

/// Defining the children types of the network
const char *
SOP_Plant::getChildType() const
{
	return SOP_OPTYPE_NAME;
}

OP_OpTypeId
SOP_Plant::getChildTypeID() const
{
	return SOP_OPTYPE_ID;
}

/// Defining self
const char *
SOP_Plant::getOpType() const
{
	return SOP_MANAGEMENT_OPTYPE;
}

OP_OpTypeId
SOP_Plant::getOpTypeID() const
{
	return SOP_OPTYPE_ID;
}

/// Provides the labels to appear on input buttons
const char*
SOP_Plant::inputLabel(unsigned idx) const {
	static UT_WorkBuffer theLabel;
	int i = idx;

	theLabel.strcpy("inputs" + std::to_string(i));
	return theLabel.buffer();
}

/// Provides the labels to appear on output buttons
const char*
SOP_Plant::outputLabel(unsigned idx) const {
	static UT_WorkBuffer theLabel;
	int i = idx;

	theLabel.strcpy("plantGeom" + std::to_string(i));
	return theLabel.buffer();
}

/// Controls the number of input buttons visible on the node tile
unsigned
SOP_Plant::getNumVisibleInputs() const {
	return 0;
}

/// Controls the number of output buttons visible on the node tile
unsigned
SOP_Plant::getNumVisibleOutputs() const {
	return 1;
}