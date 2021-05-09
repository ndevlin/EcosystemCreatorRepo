#include <UT/UT_DSOVersion.h>
#include <OP/OP_Director.h>
#include <OP/OP_BundleList.h>
#include "ECOSYSTEMPlugin.h"

using namespace HDK_Sample;

///
/// Install the Ecosystem Object into Houdini's object table
///
void
newObjectOperator(OP_OperatorTable *table)
{
	table->addOperator(
		new OP_Operator("EcosystemNode",                  // Internal name
						"Ecosystem Geo",                 // UI name
	                    OBJ_Ecosystem::myConstructor,	     // How to build the SOP
	                    OBJ_Ecosystem::buildTemplatePair(0), // My parameters
	                    OBJ_Ecosystem::theChildTableName,    // Table of child nodes
	                    0, 							         // Min # of sources
						1,							         // Max # of sources TODO ?
	                    OBJ_Ecosystem::buildVariablePair(0), // Local variables
						OP_FLAG_NETWORK)		             // Flag it as a network
	);
	
	/*table->addOperator(
		new OP_Operator("SingleSpecies",                 // Internal name
						"Plant Species",				// UI name
	                    PlantSpecies::myConstructor,	    // How to build the SOP
	                    PlantSpecies::buildTemplatePair(0), // My parameters
	                    PlantSpecies::theChildTableName,    // Table of child nodes
	                    0, 							        // Min # of sources
						1,							        // Max # of sources TODO ?
	                    PlantSpecies::buildVariablePair(0), // Local variables
						OP_FLAG_NETWORK)		            // Flag it as a network
	);*/
}

///
/// Register the Branch Modules and Plant Nodes as a SOP (and sopnet)
///
void
newSopOperator(OP_OperatorTable *table)
{
	table->addOperator(
		new OP_Operator("SingleSpecies",	                 // Internal name
						"Plant Species",					 // UI name
						PlantSpecies::myConstructor,	     // How to build the SOP
						PlantSpecies::myTemplateList,	     // My parameters
						PlantSpecies::theChildTableName,	 // The table holding the network
						0,				                     // Min # of sources
						1,				                     // Max # of sources
						PlantSpecies::myVariables,			 // Local variables
						OP_FLAG_NETWORK & OP_FLAG_GENERATOR) // Flag it as generator & network
	);

	table->addOperator(
		new OP_Operator("PlantNode",	                     // Internal name
						"Plant Sop",						 // UI name
						SOP_Plant::myConstructor,	         // How to build the SOP
						SOP_Plant::myTemplateList,	         // My parameters
						SOP_Plant::theChildTableName,		 // The table holding the network
						0,				                     // Min # of sources
						1,				                     // Max # of sources
						SOP_Plant::myVariables,				 // Local variables
						OP_FLAG_NETWORK & OP_FLAG_GENERATOR) // Flag it as generator & network
	);

	table->addOperator(
		new OP_Operator("BranchModule",	                     // Internal name
						"Branch Sop",						 // UI name
						SOP_Branch::myConstructor,	         // How to build the SOP
						SOP_Branch::myTemplateList,	         // My parameters
						0,				                     // Min # of sources
						1,				                     // Max # of sources
						SOP_Branch::myVariables,	         // Local variables
						OP_FLAG_NETWORK & OP_FLAG_GENERATOR) // Flag it as generator & network
	);
}

//PRM_TYPE_DYNAMIC_PATH_LIST
/////////////////////////////////////////////////////////////////////////////////////////////////

// Declaring parameters here
static PRM_Name	 totalAgeName("totalAge", "Year (Time+Shift)");
static PRM_Name	timeShiftName("timeShift",    "Time Shift");

static PRM_Name speciesListName("pspecies", "Plant Species List");
static PRM_Name speciesPlugName("specItem#", "Plant Species #");
//				             ^^^^^^^^     ^^^^^^^^^^^^^^^
//				             internal     descriptive version

// Set up the initial/default values for the parameters
static PRM_Default totalAgeDefault(0, "0.0");
static PRM_Default timeShiftDefault(0.0);
static PRM_Default speciesDefault(0, "*");

// Set up the ranges for the parameter inputs here
static PRM_Range timeShiftRange(PRM_RANGE_RESTRICTED,  0.0, PRM_RANGE_UI, 8.0);

////////////////////////////////////////////////////////////////////////////////

/// Template Funcs

static PRM_Template
speciesItemTemplate[] =
{
    PRM_Template(PRM_STRING, PRM_TYPE_DYNAMIC_PATH,  1, &speciesPlugName,
		&speciesDefault, 0, 0, 0, &PRM_SpareData::objPath),
	//PRM_Template(PRM_STRING_OPLIST, PRM_TYPE_DYNAMIC_PATH_LIST,  1, &speciesPlugName,
	//	&speciesDefault, 0, 0, 0, &PRM_SpareData::objPath),
    PRM_Template() // List terminator
};

// Put all the parameters together for the UI
PRM_Template
OBJ_Ecosystem::myTemplateList[] = {
	PRM_Template(PRM_STRING, PRM_Template::PRM_EXPORT_MIN, 1, &totalAgeName,  &totalAgeDefault),
	PRM_Template(PRM_FLT,    PRM_Template::PRM_EXPORT_MIN, 1, &timeShiftName, &timeShiftDefault, 0, &timeShiftRange),
	//PRM_Template(PRM_STRING_OPLIST, PRM_TYPE_DYNAMIC_PATH_LIST, 1, &speciesListName,
	//	&speciesDefault, &speciesMenu, 0, 0, &PRM_SpareData::sopPath), // TODO fix spare
	PRM_Template(PRM_MULTITYPE_LIST, speciesItemTemplate, 0, &speciesListName,
                PRMzeroDefaults, 0, &PRM_SpareData::multiStartOffsetZero),
	PRM_Template()
};

// Based from an HDK sample: merges the parameters of this object with those of its ancestors
OP_TemplatePair *
OBJ_Ecosystem::buildTemplatePair(OP_TemplatePair *baseTemplate)
{
	OP_TemplatePair *ecosystem, *geo;
	
	// "Inherit" template pairs from geometry and beyond
	geo = new OP_TemplatePair(OBJ_Geometry::getTemplateList(OBJ_PARMS_PLAIN), baseTemplate);
	ecosystem = new OP_TemplatePair(OBJ_Ecosystem::myTemplateList, geo);
	return ecosystem;
}

/// Variable Funcs

// Defining local variable(s)
enum {
	VAR_PT,		// Point number  - just used to tell when cooking in evalVariableValue (unused)
	VAR_NPT		// Unused number of points variable
};

CH_LocalVariable
OBJ_Ecosystem::myVariables[] = {
    { "PT",	VAR_PT, 0 },		// The table provides a mapping
    { "NPT", VAR_NPT, 0 },		// from text string to integer token
    { 0, 0, 0 },
};

// Trying to fit with the object hierarchy
OP_VariablePair *
OBJ_Ecosystem::buildVariablePair(OP_VariablePair *baseVariable)
{
	OP_VariablePair *ecosystem, *geo;

	// "Inherit" template pairs from geometry and beyond
	ecosystem = new OP_VariablePair(OBJ_Ecosystem::myVariables, baseVariable);
	geo = new OP_VariablePair(OBJ_Geometry::ourLocalVariables, ecosystem);
	return geo;
}

////////////////////////////////////////////////////////////////////////////////

//////////////////// HOUDINI FUNCTIONS FOR NODE CONTROL ////////////////////////

OP_Node *
OBJ_Ecosystem::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
	OBJ_Ecosystem* newEco = new OBJ_Ecosystem(net, name, op);

	//// Create a merge node to merge all sop output geom
	OP_Node* mergeNode = newEco->createNode("merge");

	if (!mergeNode) { std::cout << "Merge Node is Nullptr" << std::endl; }
	else if (!mergeNode->runCreateScript())
		std::cout << "Merge constructor error" << std::endl;

	if (mergeNode) { newEco->setMerger(mergeNode); }

	/////// PLANTS ////////
	
	// Initialize however many plant types you want, right now the constructor is the same for each
	// TODO diversify
	newEco->initAndAddSpecies(0.0f /* TODO add parameters*/);

	// Initialize plant from current ecosystem parameters
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

OBJ_Ecosystem::OBJ_Ecosystem(OP_Network *net, const char *name, OP_Operator *op)
	: OBJ_Geometry(net, name, op), speciesList(), eco_merger(nullptr)
{
    myCurrPoint = -1; // To prevent garbage values from being returned
	worldAge = 0.0f;
}

OBJ_Ecosystem::~OBJ_Ecosystem() {}

/// Unsure if we'll need this
/*unsigned
OBJ_Ecosystem::disableParms()
{
    return 0;
}*/

/// Overridden to add better functionality to check (and cook) for time change
bool
OBJ_Ecosystem::cook(OP_Context &context) {
	// Allows for the ecosystem to regenerate even if time gets rewinded
	if (dirtyForTimeChange(context.getTime())) { forceRecook(true); }

	return OBJ_Geometry::cook(context);
}

/// Do the actual change-based computataions
OP_ERROR
OBJ_Ecosystem::cookMyObj(OP_Context &context)
{
	std::cout << "ECO COOK START" << std::endl;
	fpreal now = context.getTime();

	// TIME: Allow for the ecosystem age to also be impacted by the timeline, as well as the slider
	// HOWEVER: only using this just cooks on every NEW frame. Fixed in cook() above
	flags().setTimeDep(true);
	flags().setTimeInterest(true); // doesn't make a difference? // childFlagChange ?

	worldAge = AGE(now) + now;
	
	// Display the total age in a disabled parameter so the user can see
	setString(std::to_string(worldAge), CH_StringMeaning::CH_STRING_LITERAL, 
		"totalAge", 0, now);
	enableParm("totalAge", false);

	// Run geometry cook, needed to process primitive inputs
	OP_ERROR    errorstatus;
	errorstatus = OBJ_Geometry::cookMyObj(context);

    myCurrPoint = -1;
	//std::cout << "ECO COOK END" << std::endl;
	return errorstatus;
}


///////////////////////////// PUBLIC CLASS FUNCTIONS ///////////////////////////

/// Get the age of this ecosystem
float OBJ_Ecosystem::getAge() {
	return worldAge;
}

/// Add the corresponding node to the group output geometry
void OBJ_Ecosystem::addToMerger(OP_Node* pNode) {
	// Get unique input path
	if (eco_merger) {
		UT_String path;
		pNode->getFullPath(path);
		eco_merger->setNamedInput(path.hash(), pNode);
	}
}

/// Initializes a plant node in this ecosystem using a randomly chosen species
SOP_Plant* OBJ_Ecosystem::createPlant(/*add position maybe*/) {
	return createPlant(chooseSpecies());
}

/// Initializes a plant node in this ecosystem with a pre-selected species (usually called in Seeding)
SOP_Plant* OBJ_Ecosystem::createPlant(PlantSpecies* currSpecies /*add position maybe*/) {
//SOP_Plant* OBJ_Ecosystem::createPlant(std::shared_ptr<PlantType> currSpecies /*add position maybe*/) {
	OP_Node* node = createNode("PlantNode");
	if (!node) { std::cout << "Plant node is Nullptr" << std::endl; }
	else if (!node->runCreateScript())
		std::cout << "Plant node constructor error" << std::endl;

	SOP_Plant* newPlant = (SOP_Plant*)node;
	if (newPlant) {
		newPlant->initPlant(this, currSpecies, worldAge);
		addToMerger(newPlant);

		node->moveToGoodPosition();
	}

	return newPlant;
}

//////////////////////////// PROTECTED CLASS FUNCTIONS /////////////////////////

/// Stores the merge node that combines all plant geometry, sets as display node
void OBJ_Ecosystem::setMerger(OP_Node* mergeNode) {
	if (mergeNode) {
		mergeNode->setDisplay(true);
		mergeNode->setRender(true);
		eco_merger = mergeNode;
	}
}

/// Initialized a new PlantSpecies
void OBJ_Ecosystem::initNewSpecies(fpreal t/* TODO add parameters*/) {
	// Decide on a path
	//UT_String path;
	//getFullPath(path); // TODO to parent network maybe?
	/// TODO change - Currently just the default
	//speciesList.push_back(std::make_shared<PlantType>(path));


	//OP_Node* psNode = getParent()->createNode("SingleSpecies");
	OP_Node* psNode = createNode("SingleSpecies");
	if (!psNode) { std::cout << "PlantSpecies Node is Nullptr" << std::endl; }
	else if (!psNode->runCreateScript())
		std::cout << "PlantSpecies constructor error" << std::endl;
	
	PlantSpecies* plantSpec = (PlantSpecies*)psNode;
	if (plantSpec) { 
		//UT_String p;
		//getFullPath(p);
		//p += "/PlantNode1";
		//plantSpec->tempPath(p);
		speciesList.push_back(plantSpec); 
	
		UT_String path;
		plantSpec->getRelativePathTo(this, path);
		//plantSpec->getFullPath(path);
		
		int specIdx = evalInt(speciesListName.getToken(), 0, t) - 1;
		setStringInst(path, CH_STRING_LITERAL,
			speciesPlugName.getToken(), &specIdx, 0, t);
		//setData(const char *parmname, int vectori, fpreal t,
		//	const PRM_DataItemHandle &val);
	}
}

void OBJ_Ecosystem::initAndAddSpecies(fpreal t/* TODO add parameters*/) {
	int numSpec = evalInt(speciesListName.getToken(), 0, t);
	setInt(speciesListName.getToken(), 0, t, numSpec + 1);

	initNewSpecies(t/* TODO add parameters*/);
}

/// Choose a likely plant species to spawn based on current spawn location's climate features
PlantSpecies* //std::shared_ptr<PlantType>//std::shared_ptr<PlantSpecies> 
OBJ_Ecosystem::chooseSpecies(/* TODO use enviro parameters at curr location */) {
	// TODO randomly choose plantSpecies based on climate
	if (!speciesList.empty()) {
		return speciesList.at(0);
	}
	return nullptr;
}