#include "SOP_Plant.h"
#include "ECOSYSTEMPlugin.h"
using namespace HDK_Sample;

// Declaring parameters here
static PRM_Name	plantAgeName("plantAge", "PlantAge");
static PRM_Name	      g1Name("g1",       "TropismDecrease");
static PRM_Name	      g2Name("g2",       "TropismStrength");
//				             ^^^^^^^^     ^^^^^^^^^^^^^^^
//				             internal     descriptive version

// Set up the initial/default values for the parameters
static PRM_Default plantAgeDefault(0.0);
static PRM_Default	     g1Default(1.0);
static PRM_Default	     g2Default(-0.2);

// Set up the ranges for the parameter inputs here
static PRM_Range plantAgeRange(PRM_RANGE_RESTRICTED,  0.0, PRM_RANGE_UI, 8.0);
static PRM_Range       g1Range(PRM_RANGE_RESTRICTED,  0.0, PRM_RANGE_UI, 3.0);
static PRM_Range       g2Range(PRM_RANGE_RESTRICTED, -1.0, PRM_RANGE_UI, 1.0);

////////////////////////////////////////////////////////////////////////////////////////

// Put them all together
PRM_Template
SOP_Plant::myTemplateList[] = {
	PRM_Template(PRM_FLT,    PRM_Template::PRM_EXPORT_MIN, 1, &plantAgeName, &plantAgeDefault, 0, &plantAgeRange),
	PRM_Template(PRM_FLT,    PRM_Template::PRM_EXPORT_MIN, 1, &g1Name,       &g1Default,       0, &g1Range),
	PRM_Template(PRM_FLT,    PRM_Template::PRM_EXPORT_MIN, 1, &g2Name,       &g2Default,       0, &g2Range),
	PRM_Template()
};

// Here's how we define local variables for the OBJ.
enum {
	VAR_PT,		// Point number of the star
	VAR_NPT		// Number of points in the star
};

CH_LocalVariable
SOP_Plant::myVariables[] = {
    { "PT",	VAR_PT, 0 },		// The table provides a mapping
    { "NPT", VAR_NPT, 0 },		// from text string to integer token
    { 0, 0, 0 },
};

/// Still unsure if we'll need this
/*bool
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
		case VAR_NPT:
			val = (fpreal) myTotalPoints;
			return true;
		default:
			/* do nothing *//*;
		}
    }
    // Not one of our variables, must delegate to the base class.
    return SOP_Node::evalVariableValue(val, index, thread);
}*/

OP_Node *
SOP_Plant::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
	SOP_Plant* newPlant = new SOP_Plant(net, name, op);
	// TODO take as an input and share across plant instances
	newPlant->setPrototypeList();

	//// Create a merge node to merge all sop output geom
	OP_Node* mergeNode = net->createNode("merge");//newPlant->createNode("merge");

	if (!mergeNode) { std::cout << "Merge Node is Nullptr" << std::endl; }
	else if (!mergeNode->runCreateScript())
		std::cout << "Merge constructor error" << std::endl;

	// Create the root branch module
	OP_Node* branchNode_OP = net->createNode("BranchModule");// newPlant->createNode("BranchModule"); // "node"

	if (!branchNode_OP) { std::cout << "Root module is Nullptr" << std::endl; }
	else if (!branchNode_OP->runCreateScript())
		std::cout << "Root module constructor error" << std::endl;

	std::cout << "Made branch and merge nodes" << std::endl;

	if (branchNode_OP && mergeNode) {
		SOP_Branch* branchNode = (SOP_Branch*)branchNode_OP;
		newPlant->setRootModule(branchNode);
		newPlant->setMerger(mergeNode);
		newPlant->addToMerger(branchNode);
		//mergeNode->connectToInputNode(*node, 0);

		branchNode_OP->moveToGoodPosition();
		mergeNode->moveToGoodPosition();
	}
	newPlant->moveToGoodPosition();
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
	: SOP_Node(net, name, op),
	prototypeSet(nullptr), rootModule(nullptr), merger(nullptr)
{
	//addChildManager(SOP_OPTYPE_ID);
	//std::cout << std::to_string(isNetwork()) + " " + std::to_string(isSubNetwork(true)) << std::endl;
	myCurrPoint = -1;	// To prevent garbage values from being returned
	plantAge = 0.0f;
	//setOperatorTable(getOperatorTable("SOP"));
	/*UT_String path;
	getFullPath(path);
	buildOperatorTable(*getOperatorTable("SOP", path));
	//if (getOperatorTable("SOP", path)) {//"sopnet", path)) {
	//	std::cout << " ITS A TABLE" << std::endl;
	//}
	///setAllowBuildDependencies*/
}

SOP_Plant::~SOP_Plant() {}

/// Unsure if we'll need this
/*unsigned
SOP_Plant::disableParms()
{
    return 0;
}*/

OP_ERROR
SOP_Plant::cookMySop(OP_Context &context)
{
	std::cout << "PLANT COOK START" << std::endl;
	fpreal now = context.getTime();

	// Get current plant-related values
	float ageVal;
	float g1Val;
	float g2Val;

	ageVal = AGE(now);
	g1Val  = G1(now);
	g2Val  = G2(now);

	BNode::updateG1(g1Val);
	BNode::updateG2(g2Val);
	// TODO If we want to also add growth-coeff and thick-coeff as variables here?
	// for thickness we would only need to rerun the traversal unless time also changes
	// But this might be more of a prototype-designer sort of thing
    
    // For ecosystem
    //if (plant && !parentModule) {
	//	addExtraInput(plant, OP_INTEREST_DATA);
	//}
	if (rootModule) {
		rootModule->setAge(ageVal - plantAge);
	}
	else {
		std::cout << "NO ROOT" << std::endl;
		return error();
	}

    //UT_Interrupt *boss;
    //if (error() < UT_ERROR_ABORT)
    //{
	//	boss = UTgetInterrupt();
	//
    //    if (boss->opStart("Building PLANT"))
	//	{
	        /// SINGLE PLANT
	//      rootModule->setAge(ageVal - plantAge);
    //    }
	//
    //    // Must tell the interrupt server that we've completed.
	//	boss->opEnd();
    //}
	//triggerOutputChanged();

	plantAge = ageVal;

	// Run geometry cook, needed to process primitive inputs
	//OP_ERROR    errorstatus;
	//errorstatus = OBJ_Geometry::cookMyObj(context);

    myCurrPoint = -1;
	std::cout << "PLANT COOK END" << std::endl;
	return error();//errorstatus;
}

/// SETTERS
void SOP_Plant::setPrototypeList() {
	// TODO don't create one here. Take as an input and share across plant instances
	// Dont allow plant loading without that
	// Decide on a path
	UT_String path;
	getFullPath(path);
	prototypeSet = new PrototypeSet(path);
}

void SOP_Plant::setRootModule(SOP_Branch* node) {
	/// SINGLE PLANT
	rootModule = node;
	rootModule->setPlantAndPrototype(this, 0.0f, 0.0f);
	rootModule->setAge(0.0f);
}

void SOP_Plant::setMerger(OP_Node* mergeNode) {
	merger = mergeNode;
}

float SOP_Plant::getAge() {
	return plantAge;
}

int HDK_Sample::SOP_Plant::isNetwork() const
{
	return 1;
}

void SOP_Plant::addToMerger(SOP_Branch* bMod) {
	// Get unique input path
	UT_String path;
	bMod->getFullPath(path);
	merger->setNamedInput(path.hash(), bMod);
}

// Generate a prototype copy for editing in branch node
BranchPrototype* SOP_Plant::copyPrototypeFromList(float lambda, float determ) {
	return prototypeSet->selectNewPrototype(lambda, determ);
}

