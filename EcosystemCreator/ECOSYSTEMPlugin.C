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

static PRM_Name temperatureName("temperature", "Temperature");
static PRM_Name rainfallName("rainfall", "Rainfall");
//static PRM_Name randomnessName("randomness", "Randomness");
//				             ^^^^^^^^     ^^^^^^^^^^^^^^^
//				             internal     descriptive version

// Set up the initial/default values for the parameters
static PRM_Default totalAgeDefault(0, "0.0");
static PRM_Default timeShiftDefault(0.0);

static PRM_Default speciesDefault(0, "*");

static PRM_Default temperatureDefault(28.0);
static PRM_Default rainfallDefault(4100);
//static PRM_Default randomnessDefault(0.5);

// Set up the ranges for the parameter inputs here
static PRM_Range timeShiftRange(PRM_RANGE_RESTRICTED,  0.0, PRM_RANGE_UI, 8.0);

static PRM_Range temperatureRange(PRM_RANGE_RESTRICTED, -10.0, PRM_RANGE_RESTRICTED, 33.0);
static PRM_Range rainfallRange(PRM_RANGE_RESTRICTED, 10, PRM_RANGE_RESTRICTED, 4300);
//static PRM_Range randomnessRange(PRM_RANGE_RESTRICTED, 0.0, PRM_RANGE_UI, 1.0);


////////////////////////////////////////////////////////////////////////////////

/// Template Funcs

static PRM_Template
speciesItemTemplate[] =
{
    PRM_Template(PRM_STRING, PRM_TYPE_DYNAMIC_PATH,  1, &speciesPlugName,
		&speciesDefault, 0, 0, 0, &PRM_SpareData::sopPath),
	//PRM_Template(PRM_STRING_OPLIST, PRM_TYPE_DYNAMIC_PATH_LIST,  1, &speciesPlugName,
	//	&speciesDefault, 0, 0, 0, &PRM_SpareData::objPath),
    PRM_Template() // List terminator
};

// Put all the parameters together for the UI
PRM_Template
OBJ_Ecosystem::myTemplateList[] = {
	PRM_Template(PRM_STRING, PRM_Template::PRM_EXPORT_MIN, 1, &totalAgeName,  &totalAgeDefault),
	PRM_Template(PRM_FLT,    PRM_Template::PRM_EXPORT_MIN, 1, &timeShiftName, &timeShiftDefault, 0, &timeShiftRange),
	PRM_Template(PRM_FLT,    PRM_Template::PRM_EXPORT_MIN, 1, &temperatureName, &temperatureDefault, 0, &temperatureRange),
	PRM_Template(PRM_FLT,    PRM_Template::PRM_EXPORT_MIN, 1, &rainfallName, &rainfallDefault, 0, &rainfallRange),
	//PRM_Template(PRM_STRING_OPLIST, PRM_TYPE_DYNAMIC_PATH_LIST, 1, &speciesListName,
	//	&speciesDefault, &speciesMenu, 0, 0, &PRM_SpareData::sopPath), // TODO fix spare
	PRM_Template(PRM_MULTITYPE_LIST, speciesItemTemplate, 0, &speciesListName,
                PRMzeroDefaults, 0, &PRM_SpareData::multiStartOffsetZero),
	//PRM_Template(PRM_FLT,    PRM_Template::PRM_EXPORT_MIN, 1, &randomnessName, &randomnessDefault, 0, &randomnessRange),
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

	/////// PLANTS ////////
	
	// Initialize however many plant types you want, 
	// there are several detail prototype structures based off the secont param
	newEco->initAndAddSpecies(0.0f /* TODO add parameters*/,
		0, 27.0f, 4100.0f, 8.9f, 1.0f, 3.0f, -0.5f, 0.25f, 0.4f);
	newEco->initAndAddSpecies(0.0f /* TODO add parameters*/,
		1, 16.0f, 672.0f, 5.4f, 1.0f, 1.0f, -0.2f, 0.09f, 0.2f);
	newEco->initAndAddSpecies(0.0f /* TODO add parameters*/,
		2, 8.0f, 672.0f, 8.9f, 1.0f, 0.0f, -0.2f, 0.35f, 0.5f);

	// Create a merge node to merge all sop output geom
	OP_Node* allTreesMergeNode = newEco->createNode("merge");

	if (!allTreesMergeNode) { std::cout << "Merge Node is Nullptr" << std::endl; }
	else if (!allTreesMergeNode->runCreateScript())
		std::cout << "Merge constructor error" << std::endl;

	if (allTreesMergeNode) { newEco->setMerger(allTreesMergeNode); }


	// Grid to represent ground
	OP_Node* grid = newEco->createNode("grid");
	if (!grid)
	{
		std::cout << "Grid is Nullptr" << std::endl;
		return newEco;
	}
	else if (!grid->runCreateScript())
		std::cout << "Grid constructor error" << std::endl;

	//grid->setFloatInst("sizex", 0, 0.f, 100.f);
	//grid->setFloatInst("sizey", 0, 0.f, 100.f);
	//grid->setFloatInst(100.0f, "sizex", 0, 1, 0.0f);

	grid->moveToGoodPosition();

	// Color for ground
	OP_Node* groundColor = newEco->createNode("color");
	if (!groundColor)
	{
		std::cout << "Color is Nullptr" << std::endl;
		return newEco;
	}
	else if (!groundColor->runCreateScript())
		std::cout << "Color constructor error" << std::endl;

	groundColor->connectToInputNode(*grid, 0, 0);
	groundColor->moveToGoodPosition();


	// Mountain to vary terrain
	OP_Node* mountain = newEco->createNode("mountain");
	if (!mountain)
	{
		std::cout << "Mountain is Nullptr" << std::endl;
		return newEco;
	}
	else if (!mountain->runCreateScript())
		std::cout << "Mountain constructor error" << std::endl;

	mountain->connectToInputNode(*groundColor, 0, 0);
	mountain->moveToGoodPosition();


	int index = 0;

	int numTypes = newEco->numSpecies(); // 2;//3;

	for (int i = 0; i < numTypes; i++)
	{
		// Initialize plant from current ecosystem parameters
		//SOP_Plant* newPlant = newEco->createPlant(/*add position maybe*/);
		SOP_Plant* newPlant = newEco->createPlant(newEco->getSpeciesAtIdx(i));

		// Scatter to create points
		OP_Node* scatter = newEco->createNode("scatter");
		if (!scatter)
		{
			std::cout << "Scatter is Nullptr" << std::endl;
			return newEco;
		}
		else if (!scatter->runCreateScript())
			std::cout << "Scatter constructor error" << std::endl;

		newEco->scatterNodes.push_back(scatter);
		scatter->connectToInputNode(*mountain, 0, 1);

		scatter->setFloat("seed", 0, 0.f, ((float)rand() / RAND_MAX) * 10.f);
		scatter->setInt("npts", 0, 0.f, 
			int(round(newEco->numRandPlants() * newEco->getLikelihoodAtIdx(i))));

		scatter->moveToGoodPosition();

		// treeCopyToPoints to create instances of the tree for a forest
		OP_Node* treeCopyToPoints = newEco->createNode("copytopoints");
		if (!treeCopyToPoints)
		{
			std::cout << "Copy To Points is Nullptr" << std::endl;
			return newEco;
		}
		else if (!treeCopyToPoints->runCreateScript())
			std::cout << "Copy To Points constructor error" << std::endl;
		treeCopyToPoints->connectToInputNode(*newPlant, 0, 0);
		treeCopyToPoints->connectToInputNode(*scatter, 1, 0);
		treeCopyToPoints->moveToGoodPosition();

		allTreesMergeNode->connectToInputNode(*treeCopyToPoints, index, 0);
		index++;

	}

	allTreesMergeNode->connectToInputNode(*mountain, index, 0);
	allTreesMergeNode->moveToGoodPosition();
	allTreesMergeNode->setVisible(true);

	return newEco;
}

OBJ_Ecosystem::OBJ_Ecosystem(OP_Network *net, const char *name, OP_Operator *op)
	: OBJ_Geometry(net, name, op), 
		scatterNodes(), speciesList(), speciesLikelihood(), eco_merger(nullptr)
{
    myCurrPoint = -1; // To prevent garbage values from being returned
	worldAge = 0.0f;
	worldTemp = 28.0f;
	worldPrecip = 4100.0f;
	totalPlants = 20.0f;
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
	//std::cout << "ECO COOK START" << std::endl;
	fpreal now = context.getTime();

	// TIME: Allow for the ecosystem age to also be impacted by the timeline, as well as the slider
	// HOWEVER: only using this just cooks on every NEW frame. Fixed in cook() above
	flags().setTimeDep(true);
	flags().setTimeInterest(true); // doesn't make a difference? // childFlagChange ?

	// Get current ecosystem-related values
	float temperature;
	float rainfall;
	///float randomness;

	temperature = TEMPERATURE(now);
	rainfall = RAINFALL(now);
	///randomness = RANDOMNESS(now);

	//rootModules.at(i)->temperature = temperature;
	//rootModules.at(i)->rainfall = rainfall;
	///BranchPrototype::setRandomness(randomness);

	// Also do if climate parameters of Plant Species change
	if (abs(worldTemp - temperature) > 0.05 || abs(worldPrecip - rainfall) > 0.5) {
		worldTemp = temperature;
		worldPrecip = rainfall;

		recalculateLikelihood();

		for (int i = 0; i < scatterNodes.size(); i++) {
			scatterNodes.at(i)->setInt("npts", 0, 0.f,
				int(round(numRandPlants() * getLikelihoodAtIdx(i))));
		}
	}

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
	OP_Node* node = createNode("PlantNode");
	if (!node) { std::cout << "Plant node is Nullptr" << std::endl; }
	else if (!node->runCreateScript())
		std::cout << "Plant node constructor error" << std::endl;

	SOP_Plant* newPlant = (SOP_Plant*)node;
	if (newPlant) {
		newPlant->initPlant(this, currSpecies, worldAge);
		addToMerger(newPlant);

		node->moveToGoodPosition(); // TODO remove for speed
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
void OBJ_Ecosystem::initNewSpecies(fpreal t/* TODO add parameters*/, 
	int defaultSpeciesType, float temp, float precip, float maxAge, float growthRate,
	float g1Init, float g2Init, float lengthMult, float thickMult) {

	// Create the Species node inside the ecosystem geometry
	OP_Node* psNode = createNode("SingleSpecies");
	if (!psNode) { std::cout << "PlantSpecies Node is Nullptr" << std::endl; }
	else if (!psNode->runCreateScript())
		std::cout << "PlantSpecies constructor error" << std::endl;
	
	// Making connections to the Species
	PlantSpecies* plantSpec = (PlantSpecies*)psNode;
	if (plantSpec) { 
		// Properly initialize with custom parameters
		plantSpec->initWithParameters(defaultSpeciesType, temp, precip, maxAge,
			growthRate, g1Init, g2Init, lengthMult, thickMult);

		speciesList.push_back(plantSpec);
		recalculateLikelihood();
	
		// Add it to the species list
		UT_String path;
		plantSpec->getRelativePathTo(this, path);
		
		int specIdx = evalInt(speciesListName.getToken(), 0, t) - 1;
		setStringInst(path, CH_STRING_LITERAL,
			speciesPlugName.getToken(), &specIdx, 0, t);

		psNode->moveToGoodPosition();
	}
}

void OBJ_Ecosystem::initAndAddSpecies(fpreal t/* TODO add parameters*/, 
	int defaultSpeciesType, float temp, float precip, float maxAge, float growthRate, 
	float g1Init, float g2Init, float lengthMult, float thickMult) {

	int numSpec = evalInt(speciesListName.getToken(), 0, t);
	setInt(speciesListName.getToken(), 0, t, numSpec + 1);

	initNewSpecies(t, defaultSpeciesType, temp, precip, maxAge, growthRate,
		g1Init, g2Init, lengthMult, thickMult);
}

// Temp only for initializing species in myConstructor
PlantSpecies* OBJ_Ecosystem::getSpeciesAtIdx(int idx) {
	if (speciesList.empty()) {
		return nullptr;
	}
	else if (idx < 0) {
		return speciesList.at(0);
	}
	else if (idx >= numSpecies()) {
		return speciesList.at(numSpecies() - 1);
	}
	return speciesList.at(idx);
}

// Temp only for initializing species in myConstructor
float OBJ_Ecosystem::getLikelihoodAtIdx(int idx) const {
	if (speciesLikelihood.empty()) {
		return 0.0f;
	}
	else if (idx < 0) {
		return speciesLikelihood.at(0);
	}
	else if (idx >= speciesLikelihood.size()) {
		return speciesLikelihood.at(speciesLikelihood.size() - 1);
	}
	return speciesLikelihood.at(idx);
}

int OBJ_Ecosystem::numSpecies() const {
	return speciesList.size();
}

int OBJ_Ecosystem::numRandPlants() const {
	return totalPlants;
}

/// Choose a likely plant species to spawn based on current spawn location's climate features
PlantSpecies*
OBJ_Ecosystem::chooseSpecies(/* TODO use enviro parameters at curr location */) {
	// TODO randomly choose plantSpecies based on climate variables and likelihood list
	// TODO later, should really move to a map
	if (!speciesList.empty()) {
		return speciesList.at(0);
	}
	return nullptr;
}

void OBJ_Ecosystem::recalculateLikelihood() {
	speciesLikelihood.clear();
	float total = 0.0f;

	for (int i = 0; i < speciesList.size(); i++) {
		float tempDif = 43.0f - abs(worldTemp - speciesList.at(i)->getTemp());
		float precipDif = 4290.0f - abs(worldPrecip - speciesList.at(i)->getPrecip());

		float currWeight = tempDif / 43.0f + precipDif / 4290.0f;

		total += currWeight;
		speciesLikelihood.push_back(currWeight);
	}

	for (int i = 0; i < speciesLikelihood.size(); i++) {
		if (total > 0.00001f) {
			speciesLikelihood.at(i) /= total;
		}
	}
}