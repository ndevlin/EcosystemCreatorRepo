#include "PlantSpecies.h"

using namespace HDK_Sample;

// Declaring parameters here
static PRM_Name	g1Name("g1", "Tropism Falloff"); 
static PRM_Name	g2Name("g2", "Tropism Strength (temp)");

// Set up the initial/default values for the parameters
static PRM_Default g1Default(1.0);
static PRM_Default g2Default(-0.2);

// Set up the ranges for the parameter inputs here
static PRM_Range g1Range(PRM_RANGE_RESTRICTED,  0.0, PRM_RANGE_UI, 3.0);
static PRM_Range g2Range(PRM_RANGE_RESTRICTED, -1.0, PRM_RANGE_UI, 1.0);

// Put all the parameters together for the UI
PRM_Template
PlantSpecies::myTemplateList[] = {
	PRM_Template(PRM_FLT, PRM_Template::PRM_EXPORT_MIN, 1, &g1Name, &g1Default, 0, &g1Range),
	PRM_Template(PRM_FLT, PRM_Template::PRM_EXPORT_MIN, 1, &g2Name, &g2Default, 0, &g2Range),
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
	: SOP_Node(net, name, op), vars(1.0f, -0.2f), 
	prototypeSet(new PrototypeSet(path, &vars))
{
	addOpInterest(this, &PlantSpecies::nodeEventHandler);
}

PlantSpecies::~PlantSpecies()
{
	delete prototypeSet;
	removeOpInterest(this, &PlantSpecies::nodeEventHandler);
}

OP_ERROR
PlantSpecies::cookMySop(OP_Context &context)
{
	fpreal now = context.getTime();

	// Get current species-related values
	vars.setG1(G1(now));
	vars.setG2(G2(now));
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
	vars.setG1(G1(0.0f));
	vars.setG2(G2(0.0f));

	// Thankfully it still updates Plants

	//triggerUIChanged(OP_UICHANGE_CONNECTIONS);
	//triggerOutputChanged();
	//forceRecook(true);
	//getParmIndex
	//getParmPtrInst
	//triggerParmCallback
}


////////////////////////////////////////////////////////////////////////////////

PlantSpeciesVariables::PlantSpeciesVariables(float g1Init, float g2Init) 
	: g1(g1Init), g2(g2Init)
{}

// Get the Tropism Falloff variable
float PlantSpeciesVariables::getG1() const {
	return g1;
}

// Get the Tropism Strength variable (temporarily here)
float PlantSpeciesVariables::getG2() const {
	return g2;
}

// Set the Tropism Falloff variable
void PlantSpeciesVariables::setG1(float g1Val) {
	g1 = g1Val;
}

// Set the Tropism Strength variable (temporarily here)
void PlantSpeciesVariables::setG2(float g2Val) {
	g2 = g2Val;
}