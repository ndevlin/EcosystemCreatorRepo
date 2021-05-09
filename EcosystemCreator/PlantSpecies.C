#include "PlantSpecies.h"

using namespace HDK_Sample;

// Put all the parameters together for the UI
PRM_Template
PlantSpecies::myTemplateList[] = {
	PRM_Template()
};

// Based from an HDK sample: merges the parameters of this object with those of its ancestors
OP_TemplatePair *
PlantSpecies::buildTemplatePair(OP_TemplatePair *baseTemplate)
{
	OP_TemplatePair *species, *geo;

	// "Inherit" template pairs from geometry and beyond
	geo = new OP_TemplatePair(OBJ_Geometry::getTemplateList(OBJ_PARMS_PLAIN), baseTemplate);
	species = new OP_TemplatePair(PlantSpecies::myTemplateList, geo);
	return species;
}

// Defining local variable(s)
enum {
	VAR_PT,		// Point number  - just used to tell when cooking in evalVariableValue (unused)
};

CH_LocalVariable
PlantSpecies::myVariables[] = {
	{ "PT",	VAR_PT, 0 },		// The table provides a mapping
	{ 0, 0, 0 },				// from text string to integer token
};

// Trying to fit with the object hierarchy
OP_VariablePair *
PlantSpecies::buildVariablePair(OP_VariablePair *baseVariable)
{
	OP_VariablePair *species, *geo;

	// "Inherit" template pairs from geometry and beyond
	species = new OP_VariablePair(PlantSpecies::myVariables, baseVariable);
	geo = new OP_VariablePair(OBJ_Geometry::ourLocalVariables, species);
	return geo;
}

OP_Node *
PlantSpecies::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
	UT_String path;
	net->getFullPath(path);
	return new PlantSpecies(path, net, name, op);
}

PlantSpecies::PlantSpecies(const char* path, OP_Network *net, 
	const char *name, OP_Operator *op)
	: OBJ_Geometry(net, name, op), prototypeSet(new PrototypeSet(path))
{
	//UT_String p;
	//getFullPath(p);
	//prototypeSet = new PrototypeSet(p);
}

void PlantSpecies::tempPath(const char* path) {
	prototypeSet = new PrototypeSet(path);
}

PlantSpecies::~PlantSpecies()
{}

/// Do the actual change-based computataions
OP_ERROR
PlantSpecies::cookMyObj(OP_Context &context)
{
	std::cout << "PlantSpec cook" << std::endl;
	fpreal now = context.getTime();
	// TODO read from parms

	OP_ERROR    errorstatus;
	errorstatus = OBJ_Geometry::cookMyObj(context);
	std::cout << "PlantSpec cook end" << std::endl;
	return errorstatus;
}

/// Generate a prototype copy to store as an editable tree in a SOP_Branch
BranchPrototype* PlantSpecies::copyPrototypeFromList(float lambda, float determ) {
	return prototypeSet->selectNewPrototype(lambda, determ);
}