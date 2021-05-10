#ifndef __PLANT_SPECIES_h__
#define __PLANT_SPECIES_h__

#include <SOP/SOP_Node.h>
#include <PRM/PRM_Include.h>

#include <BranchPrototype.h>

namespace HDK_Sample {

class PlantSpeciesVariables {
public:
	PlantSpeciesVariables(float g1Init, float g2Init);
	~PlantSpeciesVariables() {}

	// Get the Tropism Falloff variable
	float getG1() const;

	// Get the Tropism Strength variable (temporarily here)
	float getG2() const;

	// Set the Tropism Falloff variable
	void setG1(float g1Val);

	// Set the Tropism Strength variable (temporarily here)
	void setG2(float g2Val);

private:
	float g1;
	float g2;
};

class PlantSpecies : public SOP_Node
{
public:
	static OP_Node		*myConstructor(OP_Network*, const char *, OP_Operator *);

	/// Stores the description of the interface of the SOP in Houdini.
	/// Each parm template refers to a parameter.
	static PRM_Template		 myTemplateList[];

	/// This optional data stores the list of local variables.
	static CH_LocalVariable	 myVariables[];

	/// Controls the number of input/output buttons visible on the node tile.
	// @{
	unsigned                 getNumVisibleInputs() const override;
	unsigned                 getNumVisibleOutputs() const override;
	// @}

	/// Generate a prototype copy to store as an editable tree in a SOP_Branch
	BranchPrototype*         copyPrototypeFromList(float lambda, float determ);

protected:
	PlantSpecies(const char* path,
		OP_Network *net, const char *name, OP_Operator *op);
	~PlantSpecies();

	/// Do the actual change-based computataions
	virtual OP_ERROR		 cookMySop(OP_Context &context);

	bool			         cookDataForAnyOutput() const override // TODO confirm needed
								{ return true; }

private:
	static void              nodeEventHandler(OP_Node *caller, void *callee, 
								OP_EventType type, void *data);

	void                     handleParmChanged(int parm_index);


	float G1(fpreal t)      { return evalFloat("g1", 0, t); }
	float G2(fpreal t)      { return evalFloat("g2", 0, t); }

	PrototypeSet* prototypeSet; // TODO Make unique pointer
	PlantSpeciesVariables vars;
};
} // End HDK_Sample namespace
#endif