#include <UT/UT_DSOVersion.h>
#include "ECOSYSTEMPlugin.h"
using namespace HDK_Sample;

///
/// Install the plant object into Houdini's object table
///
void
newObjectOperator(OP_OperatorTable *table)
{
	table->addOperator(
		new OP_Operator("EcoPlantNode",                  // Internal name
						"EcosystemNode",                 // UI name
	                    OBJ_Plant::myConstructor,	     // How to build the SOP
	                    OBJ_Plant::buildTemplatePair(0), // My parameters
	                    OBJ_Plant::theChildTableName,    // Table of child nodes
	                    0, 							     // Min # of sources
						1,							     // Max # of sources TODO ?
	                    OBJ_Plant::buildVariablePair(0), // Local variables
						OP_FLAG_NETWORK)		         // Flag it as a network
	);
}

///
/// Register the branch modules as a SOP
///
void
newSopOperator(OP_OperatorTable *table)
{
	table->addOperator(
		new OP_Operator("PlantNode",	                     // Internal name
						"SinglePlant",						 // UI name
						SOP_Plant::myConstructor,	         // How to build the SOP
						SOP_Plant::myTemplateList,	         // My parameters
						SOP_Plant::theChildTableName,
						0,				                     // Min # of sources
						1,				                     // Max # of sources
						SOP_Plant::myVariables,				 // Local variables
						OP_FLAG_NETWORK & OP_FLAG_GENERATOR) // Flag it as generator & network
	);

	table->addOperator(
		new OP_Operator("BranchModule",	                     // Internal name
						"MyBranchModule",			         // UI name
						SOP_Branch::myConstructor,	         // How to build the SOP
						SOP_Branch::myTemplateList,	         // My parameters
						0,				                     // Min # of sources
						1,				                     // Max # of sources
						SOP_Branch::myVariables,	         // Local variables
						OP_FLAG_NETWORK & OP_FLAG_GENERATOR) // Flag it as generator & network
	);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Declaring parameters here
static PRM_Name	ecoAgeName("ecoAge",    "EcoAge");
static PRM_Name	 ecog1Name("ecog1",     "EcoTropismDecrease");
static PRM_Name	 ecog2Name("ecog2",     "EcoTropismStrength");
//				             ^^^^^^^^     ^^^^^^^^^^^^^^^
//				             internal     descriptive version

// Set up the initial/default values for the parameters
static PRM_Default ecoAgeDefault(0.0);
static PRM_Default  ecog1Default(1.0);
static PRM_Default  ecog2Default(-0.2);

// Set up the ranges for the parameter inputs here
static PRM_Range ecoAgeRange(PRM_RANGE_RESTRICTED,  0.0, PRM_RANGE_UI, 8.0);
static PRM_Range  ecog1Range(PRM_RANGE_RESTRICTED,  0.0, PRM_RANGE_UI, 3.0);
static PRM_Range  ecog2Range(PRM_RANGE_RESTRICTED, -1.0, PRM_RANGE_UI, 1.0);

////////////////////////////////////////////////////////////////////////////////////////

// Put them all together
PRM_Template
OBJ_Plant::myTemplateList[] = {
	PRM_Template(PRM_FLT,    PRM_Template::PRM_EXPORT_MIN, 1, &ecoAgeName, &ecoAgeDefault, 0, &ecoAgeRange),
	PRM_Template(PRM_FLT,    PRM_Template::PRM_EXPORT_MIN, 1, &ecog1Name,   &ecog1Default, 0, &ecog1Range),
	PRM_Template(PRM_FLT,    PRM_Template::PRM_EXPORT_MIN, 1, &ecog2Name,   &ecog2Default, 0, &ecog2Range),
	PRM_Template()
};

// Based from an HDK sample: merges the parameters of this object with those of its ancestors
OP_TemplatePair *
OBJ_Plant::buildTemplatePair(OP_TemplatePair *baseTemplate)
{
	OP_TemplatePair *ecosystem, *geo;
	
	// "Inherit" template pairs from geometry and beyond
	geo = new OP_TemplatePair(OBJ_Geometry::getTemplateList(OBJ_PARMS_PLAIN), baseTemplate);
	ecosystem = new OP_TemplatePair(OBJ_Plant::myTemplateList, geo);
	return ecosystem;
}


// Here's how we define local variables for the OBJ.
enum {
	VAR_PT,		// Point number of the star
	VAR_NPT		// Number of points in the star
};

CH_LocalVariable
OBJ_Plant::myVariables[] = {
    { "PT",	VAR_PT, 0 },		// The table provides a mapping
    { "NPT", VAR_NPT, 0 },		// from text string to integer token
    { 0, 0, 0 },
};

// Trying to fit with the object hierarchy
OP_VariablePair *
OBJ_Plant::buildVariablePair(OP_VariablePair *baseVariable)
{
	OP_VariablePair *ecosystem, *geo;

	// "Inherit" template pairs from geometry and beyond
	ecosystem = new OP_VariablePair(OBJ_Plant::myVariables, baseVariable);
	geo = new OP_VariablePair(OBJ_Geometry::ourLocalVariables, ecosystem);
	return geo;
}

/// Still unsure if we'll need this
/*bool
OBJ_Plant::evalVariableValue(fpreal &val, int index, int thread)
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
    return OBJ_Geometry::evalVariableValue(val, index, thread);
}*/

OP_Node *
OBJ_Plant::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
	OBJ_Plant* newEco = new OBJ_Plant(net, name, op);

	//// Create a merge node to merge all sop output geom
	OP_Node* mergeNode = newEco->createNode("merge");

	if (!mergeNode) { std::cout << "Merge Node is Nullptr" << std::endl; }
	else if (!mergeNode->runCreateScript())
		std::cout << "Merge constructor error" << std::endl;

	if (mergeNode) { newEco->setMerger(mergeNode); }

	/////// PLANTS
	
	// Initialize however many plant types you want, right now the constructor is the same for each
	// TODO diversify
	newEco->initPlantType(/* TODO add parameters*/);

	// Initialize plant from current ecosystem parameters
	// TODO maybe select PlantType here
	newEco->createPlant(/*add position maybe*/);

	if (mergeNode) { mergeNode->moveToGoodPosition(); }

	/*newPlant->setPrototypeList();

	// Create a merge node to merge all sop output geom
	OP_Node* mergeNode = newPlant->createNode("merge");

	if (!mergeNode) { std::cout << "Merge Node is Nullptr" << std::endl; }
	else if (!mergeNode->runCreateScript())
		std::cout << "Merge constructor error" << std::endl;

	int numPlants = 1;//3;

	for (int i = 0; i < numPlants; i++)
	{
		OP_Node* node = newPlant->createNode("BranchModule");

		// Quick issue check
		if (!node) { std::cout << "Root module is Nullptr" << std::endl; }
		else if (!node->runCreateScript())
			std::cout << "Root module constructor error" << std::endl;

		node->moveToGoodPosition();

		SOP_Branch* bNode = (SOP_Branch*)node;
		newPlant->setRootModule(bNode);
		newPlant->setMerger(mergeNode);
		newPlant->addToMerger(bNode);
		//mergeNode->connectToInputNode(*node, 0);

		node->moveToGoodPosition();
	}

	mergeNode->moveToGoodPosition();


	// Color for bark
	OP_Node* color1 = newPlant->createNode("color");
	if (!color1)
	{
		std::cout << "Color is Nullptr" << std::endl;
		return newPlant;
	}
	else if (!color1->runCreateScript())
		std::cout << "Color constructor error" << std::endl;


	OP_Node* mergeNode2 = newPlant->createNode("merge");
	if (!mergeNode2) { std::cout << "Merge Node is Nullptr" << std::endl; }
	else if (!mergeNode2->runCreateScript())
		std::cout << "Merge constructor error" << std::endl;


	
	// Copy to Points to allow leaf instancing
	OP_Node* copyToPoints = newPlant->createNode("copytopoints");
	if (!copyToPoints) 
	{ 
		std::cout << "Copy To Points is Nullptr" << std::endl; 
		return newPlant;
	}
	else if (!copyToPoints->runCreateScript())
		std::cout << "Copy To Points constructor error" << std::endl;

	// Star will serve as a leaf for now
	OP_Node* star = newPlant->createNode("star");
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
	color2->moveToGoodPosition();


	copyToPoints->connectToInputNode(*mergeNode, 1, 0);

	mergeNode2->connectToInputNode(*copyToPoints, 0, 0);

	mergeNode2->connectToInputNode(*color1, 1, 0);

	copyToPoints->moveToGoodPosition();

	mergeNode2->moveToGoodPosition();


	OP_Node* mergeNode3 = newPlant->createNode("merge");
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
	color3->moveToGoodPosition();
	
	color1->connectToInputNode(*mergeNode, 0, 0);
	color1->moveToGoodPosition();
	*/
	return newEco;
}

OBJ_Plant::OBJ_Plant(OP_Network *net, const char *name, OP_Operator *op)
	: OBJ_Geometry(net, name, op), plantTypes(),
	prototypeSet(nullptr), /*rootModule(nullptr), numRootModules(0),*/ eco_merger(nullptr)
{
    myCurrPoint = -1;	// To prevent garbage values from being returned
	plantAge = 0.0f;
	//rootModules = std::vector<SOP_Branch*>();
	//rootModules.push_back(nullptr);
}

OBJ_Plant::~OBJ_Plant() {}

/// Unsure if we'll need this
/*unsigned
OBJ_Plant::disableParms()
{
    return 0;
}*/

OP_ERROR
OBJ_Plant::cookMyObj(OP_Context &context)
{
	std::cout << "ECO COOK START" << std::endl;
	fpreal now = context.getTime();

	// Get current plant-related values
	float ageVal;
	float g1Val;
	float g2Val;

	ageVal = AGE(now);
	g1Val  = EG1(now);
	g2Val  = EG2(now);

	//BNode::updateG1(g1Val);
	//BNode::updateG2(g2Val);
	// TODO If we want to also add growth-coeff and thick-coeff as variables here?
	// for thickness we would only need to rerun the traversal unless time also changes
	// But this might be more of a prototype-designer sort of thing

	/// SINGLE PLANT
	//rootModule->setAge(ageVal - plantAge);
	///
	float diff = ageVal - plantAge;

	//for(int i = 0; i < numRootModules; i++)
	//{ 
	//	//rootModules.at(i)->setAge(diff);
	//}
	///

	plantAge = ageVal;


	// Run geometry cook, needed to process primitive inputs
	OP_ERROR    errorstatus;
	errorstatus = OBJ_Geometry::cookMyObj(context);

    myCurrPoint = -1;
	std::cout << "ECO COOK END" << std::endl;
	return errorstatus;
}

/// SETTERS
void OBJ_Plant::initPlantType(/* TODO add parameters*/) {
	// Decide on a path
	UT_String path;
	getFullPath(path);
	plantTypes.push_back(std::make_shared<PlantType>(path));
}

void OBJ_Plant::setRootModule(SOP_Branch* node) {
	/// SINGLE PLANT
	//rootModule = node;
	//rootModule->setPlantAndPrototype(this, 0.0f, 0.0f);
	//rootModule->setAge(0.0f);
	///
	/*if (numRootModules < 1)
	{
		rootModules[0] = node;
	}
	else
	{ 
		rootModules.push_back(node);
	}
	node->setPlantAndPrototype(this, 0.0f, 0.0f);
	node->setAge(0.0f);
	numRootModules++;*/
	///
}

void OBJ_Plant::setMerger(OP_Node* mergeNode) {
	if (mergeNode) {
		mergeNode->setDisplay(true);
		mergeNode->setRender(true);
		eco_merger = mergeNode;
	}
}

SOP_Plant* OBJ_Plant::createPlant(/*add position maybe*/) {
	OP_Node* node = createNode("PlantNode");
	if (!node) { std::cout << "Plant node is Nullptr" << std::endl; }
	else if (!node->runCreateScript())
		std::cout << "Plant node constructor error" << std::endl;

	// Create a subnetwork to store tree of branch modules
	OP_Node* branchNet = createNode("subnet");
	
	if (!branchNet) { std::cout << "SubNetwork is Nullptr" << std::endl; }
	else if (!branchNet->runCreateScript())
		std::cout << "SubNetwork constructor error" << std::endl;

	SOP_Plant* newPlant = (SOP_Plant*)node;
	if (newPlant) {// && branchNet) {
		// It is currently selecting a PlantType randomly in here.
		// TODO input PlantType based on seeding
		branchNet->connectToInputNode(*newPlant, 0, 0);
		newPlant->initPlant(this, branchNet);

		addToMerger(branchNet);
		//addToMerger(newPlant);

		node->moveToGoodPosition();
		branchNet->moveToGoodPosition();
	}

	return newPlant;
}

float OBJ_Plant::getAge() {
	return plantAge;
}

void OBJ_Plant::addToMerger(OP_Node* pNode) {
	// Get unique input path
	if (eco_merger) {
		UT_String path;
		pNode->getFullPath(path);
		eco_merger->setNamedInput(path.hash(), pNode);
	}
}

// Generate a prototype copy for editing in branch node
BranchPrototype* OBJ_Plant::copyPrototypeFromList(float lambda, float determ) {
	return prototypeSet->selectNewPrototype(lambda, determ);
}

