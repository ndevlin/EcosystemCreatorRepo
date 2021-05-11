#include "PlantSpecies.h"

using namespace HDK_Sample;

// Declaring parameters here
static PRM_Name	maxAgeName("maxAge", "Maximum Age");
static PRM_Name	growthName("growthRate", "Growth Rate");

static PRM_Name	g1Name("g1", "Tropism Falloff");
static PRM_Name	g2Name("g2", "Tropism Strength (temp)");

static PRM_Name   tempAName("optimalTemp",   "Temperature Adaptation (Celcius)");
static PRM_Name precipAName("optimalPrecip", "Precipitation Adaptation (mm)");

static PRM_Name	betaName("beta", "Scaling Coefficient");
static PRM_Name	tCName("tC", "Thickening Coefficent");

// Set up the initial/default values for the parameters
static PRM_Default maxAgeDefault(8.9);
static PRM_Default growthDefault(1.0);

static PRM_Default g1Default(1.0);
static PRM_Default g2Default(-0.2);

static PRM_Default tempeADefault(28.0);
static PRM_Default precipADefault(4100);

static PRM_Default betaDefault(0.3);
static PRM_Default tCDefault(0.4);

// Set up the ranges for the parameter inputs here
// TODO Change max age (and growth?) range to realistically line up with real years
static PRM_Range maxAgeRange(PRM_RANGE_RESTRICTED, 1.0, PRM_RANGE_UI, 8.9);
static PRM_Range growthRange(PRM_RANGE_RESTRICTED, 0.5, PRM_RANGE_RESTRICTED, 2.0);

static PRM_Range g1Range(PRM_RANGE_RESTRICTED,  0.0, PRM_RANGE_UI, 3.0);
static PRM_Range g2Range(PRM_RANGE_RESTRICTED, -1.0, PRM_RANGE_UI, 1.0);

static PRM_Range tempARange(PRM_RANGE_RESTRICTED, -10.0, PRM_RANGE_RESTRICTED, 33.0);
static PRM_Range precipARange(PRM_RANGE_RESTRICTED, 10, PRM_RANGE_RESTRICTED, 4300);

static PRM_Range betaRange(PRM_RANGE_RESTRICTED, 0.1, PRM_RANGE_RESTRICTED, 0.6);
static PRM_Range tCRange(PRM_RANGE_RESTRICTED, 0.1, PRM_RANGE_RESTRICTED, 1.0);

// Put all the parameters together for the UI
PRM_Template
PlantSpecies::myTemplateList[] = {
	PRM_Template(PRM_FLT, PRM_Template::PRM_EXPORT_MIN, 1, &maxAgeName, &maxAgeDefault, 0, &maxAgeRange),
	PRM_Template(PRM_FLT, PRM_Template::PRM_EXPORT_MIN, 1, &growthName, &growthDefault, 0, &growthRange),
	PRM_Template(PRM_FLT, PRM_Template::PRM_EXPORT_MIN, 1, &g1Name, &g1Default, 0, &g1Range),
	PRM_Template(PRM_FLT, PRM_Template::PRM_EXPORT_MIN, 1, &g2Name, &g2Default, 0, &g2Range),
	PRM_Template(PRM_FLT, PRM_Template::PRM_EXPORT_MIN, 1, &tempAName, &tempeADefault, 0, &tempARange),
	PRM_Template(PRM_FLT, PRM_Template::PRM_EXPORT_MIN, 1, &precipAName, &precipADefault, 0, &precipARange),
	PRM_Template(PRM_FLT, PRM_Template::PRM_EXPORT_MIN, 1, &betaName, &betaDefault, 0, &betaRange),
	PRM_Template(PRM_FLT, PRM_Template::PRM_EXPORT_MIN, 1, &tCName, &tCDefault, 0, &tCRange),
	PRM_Template()
};

// Defining local variable(s)
enum {
	VAR_PT,		// Point number - (unused)
};

CH_LocalVariable
PlantSpecies::myVariables[] = {
	{ "PT",	VAR_PT, 0 },		// The table provides a mapping
	{ 0, 0, 0 },				// from text string to integer token
};

OP_Node *
PlantSpecies::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
	UT_String path;
	net->getFullPath(path);
	return new PlantSpecies(path, net, name, op);
}

PlantSpecies::PlantSpecies(const char* path, OP_Network *net, 
	const char *name, OP_Operator *op)
	: SOP_Node(net, name, op), vars(), prototypeSet(nullptr), pathToParent(path),
		tempA(28.0f), precipA(4100.0f)
{
	addOpInterest(this, &PlantSpecies::nodeEventHandler);
}

PlantSpecies::~PlantSpecies()
{
	delete prototypeSet;
	removeOpInterest(this, &PlantSpecies::nodeEventHandler);
}

void PlantSpecies::initWithParameters(int defaultSpeciesType,
	float temp, float precip, float maxAge, float growthRate, float g1Init,
	float g2Init, float lengthMult, float thickMult) {

	vars.setPMax(maxAge);
	setFloat("maxAge", 0, 0.f, maxAge);

	vars.setGP(growthRate);
	setFloat("growthRate", 0, 0.f, growthRate);

	vars.setG1(g1Init);
	setFloat("g1", 0, 0.f, g1Init);

	vars.setG2(g2Init);
	setFloat("g2", 0, 0.f, g2Init);

	vars.setBeta(lengthMult);
	setFloat("beta", 0, 0.f, lengthMult);

	vars.setTC(thickMult);
	setFloat("tC", 0, 0.f, thickMult);

	tempA = temp;
	setFloat("optimalTemp", 0, 0.f, temp);

	precipA = precip;
	setFloat("optimalPrecip", 0, 0.f, precip);

	prototypeSet = new PrototypeSet(pathToParent, &vars, defaultSpeciesType);
}

OP_ERROR
PlantSpecies::cookMySop(OP_Context &context)
{
	fpreal now = context.getTime();

	// Get current species-related values
	vars.setPMax(MAXAGE(now));
	vars.setGP(GROWTH(now));

	vars.setG1(G1(now));
	vars.setG2(G2(now));

	vars.setBeta(BETA(now));
	vars.setTC(TC(now));

	// IF THESE-> notify ecosystem to rebuild
	tempA = TEMPERATURE(now);
	precipA = RAINFALL(now);
	// TODO If we want to also add growth-coeff and thick-coeff as variables here?
	// for thickness we would only need to rerun the traversal unless time also changes
	// But this might be more of a prototype-designer sort of thing
	return error();
}

/// Generate a prototype copy to store as an editable tree in a SOP_Branch
BranchPrototype* PlantSpecies::copyPrototypeFromList(float lambda, float determ) {
	return prototypeSet->selectNewPrototype(lambda, determ);
}

/// Controls the number of input buttons visible on the node tile
unsigned
PlantSpecies::getNumVisibleInputs() const {
	return 0;
}

/// Controls the number of output buttons visible on the node tile
unsigned
PlantSpecies::getNumVisibleOutputs() const {
	return 0;
}

void
PlantSpecies::nodeEventHandler(
        OP_Node *caller, void *callee, OP_EventType type, void *data)
{
    switch (type)
    {
        case OP_PARM_CHANGED:
			static_cast<PlantSpecies*>(callee)->handleParmChanged((int)(intptr_t)data);
            break;
        default:
            break;
    }
}

void
PlantSpecies::handleParmChanged(int parm_index)
{
    // TODO change based on which parm 
	// - also connect as input to relevant plants so that this cooks instead
	vars.setPMax(MAXAGE(0.0f));
	vars.setGP(GROWTH(0.0f));

	vars.setG1(G1(0.0f));
	vars.setG2(G2(0.0f));

	vars.setBeta(BETA(0.0f));
	vars.setTC(TC(0.0f));

	// IF THESE-> notify ecosystem to rebuild
	tempA = TEMPERATURE(0.0f);
	precipA = RAINFALL(0.0f);

	// Thankfully it still updates Plants

	//triggerUIChanged(OP_UICHANGE_CONNECTIONS);
	//triggerOutputChanged();
	//forceRecook(true);
	//getParmIndex
	//getParmPtrInst
	//triggerParmCallback
}

// Get the optimal temperature variable
float PlantSpecies::getTemp() const {
	return tempA;
}

// Get the optimal precipitation variable
float PlantSpecies::getPrecip() const {
	return precipA;
}

// Get the maximum age
float PlantSpecies::getMaxAge() const {
	return vars.getPMax();
}

// Get the growth rate
float PlantSpecies::getGrowthRate() const {
	return vars.getGP();
}


////////////////////////////////////////////////////////////////////////////////

PlantSpeciesVariables::PlantSpeciesVariables()
	: PlantSpeciesVariables(8.9f, 1.0f, 1.0f, -0.2f, 0.3f, 0.4f)
{}

PlantSpeciesVariables::PlantSpeciesVariables(float maxAge, float growthRate,
	float g1Init, float g2Init, float lengthMult, float thickMult)
	: pMax(maxAge), gp(growthRate), g1(g1Init), g2(g2Init), beta(lengthMult), tC(thickMult)
{}

/// GETTERS
// Get/set the maximum age
float PlantSpeciesVariables::getPMax() const {
	return pMax;
}

// Get/set the growth rate
float PlantSpeciesVariables::getGP() const {
	return 1.0f; // TODO FIX //gp;
}

// Get the Tropism Falloff variable
float PlantSpeciesVariables::getG1() const {
	return g1;
}

// Get the Tropism Strength variable (temporarily here)
float PlantSpeciesVariables::getG2() const {
	return g2;
}

// Get/set the Scaling Coefficient
float PlantSpeciesVariables::getBeta() const {
	return beta;
}

// Get/set the thickness coefficient
float PlantSpeciesVariables::getTC() const {
	return tC;
}

/// SETTERS
// Set the Tropism Falloff variable
void PlantSpeciesVariables::setG1(float g1Val) {
	g1 = g1Val;
}

// Set the Tropism Strength variable (temporarily here)
void PlantSpeciesVariables::setG2(float g2Val) {
	g2 = g2Val;
}

// Set the maximum age
void PlantSpeciesVariables::setPMax(float maxAge) {
	pMax = maxAge;
}

// Set the growth rate
void PlantSpeciesVariables::setGP(float growthRate) {
	gp = growthRate;
}

// Get/set the Scaling Coefficient
void PlantSpeciesVariables::setBeta(float b) {
	beta = b;
}

// Get/set the thickness coefficient
void PlantSpeciesVariables::setTC(float thick) {
	tC = thick;
}