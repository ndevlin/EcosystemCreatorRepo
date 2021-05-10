#ifndef __PLANT_SPECIES_h__
#define __PLANT_SPECIES_h__

#include <SOP/SOP_Node.h>
#include <PRM/PRM_Include.h>

#include <BranchPrototype.h>

namespace HDK_Sample {

class PlantSpeciesVariables {
public:
	PlantSpeciesVariables();
	PlantSpeciesVariables(float maxAge, float growthRate, float g1Init, float g2Init,
		float lengthMult, float thickMult);
	~PlantSpeciesVariables() {}

	// Get/set the maximum age
	float getPMax() const;
	void  setPMax(float maxAge);

	// Get/set the growth rate
	float getGP() const;
	void  setGP(float growthRate);

	// Get/set the Tropism Falloff variable
	float getG1() const;
	void  setG1(float g1Val);

	// Get/set the Tropism Strength variable (temporarily here)
	float getG2() const;
	void  setG2(float g2Val);

	// Get/set the Scaling Coefficient
	float getBeta() const;
	void  setBeta(float b);

	// Get/set the thickness coefficient
	float getTC() const;
	void  setTC(float thick);

private:
	float pMax; /// Max age
	float gp;   /// Growth Rate

	float g1;
	float g2;

	float beta; /// Scaling Coefficient
	float tC;	/// Thickening Coefficent
};

class PlantSpecies : public SOP_Node
{
public:
	static OP_Node		*myConstructor(OP_Network*, const char *, OP_Operator *);

	void					 initWithParameters(int defaultSpeciesType, 
								float temp, float precip, float maxAge, 
								float growthRate, float g1Init, float g2Init,
								float lengthMult, float thickMult);

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

	/// Get the optimal temperature variable
	float					 getTemp() const;

	/// Get the optimal precipitation variable
	float					 getPrecip() const;

	/// Get the maximum age
	float					 getMaxAge() const;

	/// Get the growth rate
	float					 getGrowthRate() const;

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
	
	float MAXAGE(fpreal t)      { return evalFloat("maxAge", 0, t); }
	float GROWTH(fpreal t)      { return evalFloat("growthRate", 0, t); }
	float G1(fpreal t)          { return evalFloat("g1", 0, t); }
	float G2(fpreal t)          { return evalFloat("g2", 0, t); }
	float TEMPERATURE(fpreal t) { return evalFloat("optimalTemp", 0, t); }
	float RAINFALL(fpreal t)    { return evalFloat("optimalPrecip", 0, t); }
	float BETA(fpreal t)        { return evalFloat("beta", 0, t); }
	float TC(fpreal t)          { return evalFloat("tC", 0, t); }

	const char* pathToParent;

	// TODO store *ecosystem, notify when temp and rainfall change, regenerate random scene

	PrototypeSet* prototypeSet; // TODO Make unique pointer
	PlantSpeciesVariables vars;

	float tempA;   /// Optimal habitat temperature 
	float precipA; /// Optimal habitat precipitation
};
} // End HDK_Sample namespace
#endif