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
						"SinglePlantNode",               // UI name
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
OBJ_Plant::myTemplateList[] = {
	PRM_Template(PRM_FLT,    PRM_Template::PRM_EXPORT_MIN, 1, &plantAgeName, &plantAgeDefault, 0, &plantAgeRange),
	PRM_Template(PRM_FLT,    PRM_Template::PRM_EXPORT_MIN, 1, &g1Name,       &g1Default,       0, &g1Range),
	PRM_Template(PRM_FLT,    PRM_Template::PRM_EXPORT_MIN, 1, &g2Name,       &g2Default,       0, &g2Range),
	PRM_Template()
};

// Based from an HDK sample: merges the parameters of this object with those of its ancestors
OP_TemplatePair *
OBJ_Plant::buildTemplatePair(OP_TemplatePair *baseTemplate)
{
	OP_TemplatePair *plant, *geo;
	
	// "Inherit" template pairs from geometry and beyond
	geo = new OP_TemplatePair(OBJ_Geometry::getTemplateList(OBJ_PARMS_PLAIN), baseTemplate);
	plant = new OP_TemplatePair(OBJ_Plant::myTemplateList, geo);
	return plant;
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
	OP_VariablePair *plant, *geo;

	// "Inherit" template pairs from geometry and beyond
	plant = new OP_VariablePair(OBJ_Plant::myVariables, baseVariable);
	geo = new OP_VariablePair(OBJ_Geometry::ourLocalVariables, plant);
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
	OBJ_Plant* newPlant = new OBJ_Plant(net, name, op);
	// TODO take as an input and share across plant instances
	newPlant->setPrototypeList();

	// Create a merge node to merge all sop output geom
	OP_Node* mergeNode = newPlant->createNode("merge");

	if (!mergeNode) { std::cout << "Merge Node is Nullptr" << std::endl; }
	else if (!mergeNode->runCreateScript())
		std::cout << "Merge constructor error" << std::endl;

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

	return newPlant;
}

OBJ_Plant::OBJ_Plant(OP_Network *net, const char *name, OP_Operator *op)
	: OBJ_Geometry(net, name, op), 
	prototypeSet(nullptr), rootModule(nullptr), merger(nullptr)
{
    myCurrPoint = -1;	// To prevent garbage values from being returned
	plantAge = 0.0f;
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

	rootModule->setAge(ageVal - plantAge);
	plantAge = ageVal;

	// Run geometry cook, needed to process primitive inputs
	OP_ERROR    errorstatus;
	errorstatus = OBJ_Geometry::cookMyObj(context);

    myCurrPoint = -1;
	return errorstatus;
}

/// SETTERS
void OBJ_Plant::setPrototypeList() {
	// TODO don't create one here. Take as an input and share across plant instances
	// Dont allow plant loading without that
	prototypeSet = new PrototypeSet();
}

void OBJ_Plant::setRootModule(SOP_Branch* node) {
	rootModule = node;
	rootModule->setPlantAndPrototype(this, 0.0f, 0.0f);
	rootModule->setAge(0.0f);
}

void OBJ_Plant::setMerger(OP_Node* mergeNode) {
	merger = mergeNode;
}

float OBJ_Plant::getAge() {
	return plantAge;
}

void OBJ_Plant::addToMerger(SOP_Branch* bMod) {
	// Get unique input path
	UT_String path;
	bMod->getFullPath(path);
	merger->setNamedInput(path.hash(), bMod);
}

// Generate a prototype copy for editing in branch node
BranchPrototype* OBJ_Plant::copyPrototypeFromList(float lambda, float determ) {
	return prototypeSet->selectNewPrototype(lambda, determ);
}