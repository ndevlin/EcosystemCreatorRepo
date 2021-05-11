#ifndef __ECOSYSTEM_PLUGIN_h__
#define __ECOSYSTEM_PLUGIN_h__

#include <OBJ/OBJ_Geometry.h>
#include <SOP_Branch.h>
#include <SOP_Plant.h>

namespace HDK_Sample {

/// Stores the overall ecosystem and controls the time
class OBJ_Ecosystem : public OBJ_Geometry
{
public:
    static OP_Node		*myConstructor(OP_Network*, const char *,
							    OP_Operator *);

	// User Interface:

    /// Stores the description of the interface of the SOP in Houdini.
    /// Each parm template refers to a parameter.
    static PRM_Template		 myTemplateList[];

    /// This optional data stores the list of local variables.
    static CH_LocalVariable	 myVariables[];

	/// Pairs the local interface of OBJ_Ecosystem with parent class OBJ_Geometry
	static OP_TemplatePair*  buildTemplatePair(OP_TemplatePair *baseTemplate);
	static OP_VariablePair*  buildVariablePair(OP_VariablePair *baseVariable);

	/// Added better functionality to check (and cook) for time change
	bool					 cook(OP_Context &context) override;

	/// Confirms that node should be dirtied on time change
	bool					 handleTimeChange(fpreal /* t */) override 
								{ return true; } // Doesn't seem to make a difference - TODO check

	/// Get the age of this ecosystem - sum of the timeline and parm Time Shift
	float                    getAge();

	/// Add the corresponding node to the group output geometry
	void                     addToMerger(OP_Node* pNode);
	// TODO: confirm that a remove-from-merge function is unneded

	/// Store a reference to this node as the plant origin points sop
	void                     setScatter(OP_Node* scNode);

	/// Initializes a plant node in this ecosystem. The first using a randomly chosen species
	SOP_Plant* createPlant(UT_Vector3 origin = UT_Vector3(), bool setNewBirthday = true);
	SOP_Plant* createPlant(PlantSpecies* currSpecies, 
		UT_Vector3 origin = UT_Vector3(), bool setNewBirthday = true);
			// TODO, seeding (in SOP_Plant)

	/// Choose a likely plant species to spawn based on current spawn location's climate features
	PlantSpecies* chooseSpecies(/* TODO use enviro parameters at curr location */);

	// TODO - TEMPORARY saving scatter to update with climate-based likelihood
	// correspond to speciesList above
	//std::vector<OP_Node*> scatterNodes;

protected:

	OBJ_Ecosystem(OP_Network *net, const char *name, OP_Operator *op);
    virtual ~OBJ_Ecosystem();

    /// Disable parameters according to other parameters.
    //virtual unsigned		 disableParms();

    /// Do the actual change-based computataions
    virtual OP_ERROR		 cookMyObj(OP_Context &context);

    /// This function is used to lookup local variables that you have
    /// defined specific to your SOP.
    /* TODO maybe utilize evalVariableValue instead of all the pointers */

	/// Stores the merge node that combines all plant geometry, sets as display node
	void setMerger(OP_Node* mergeNode);

	/// Initialized a new PlantSpecies
	void initNewSpecies(fpreal t/* TODO add parameters*/, int defaultSpeciesType = 0,
		float temp = 28.0f, float precip = 4100.0f, float maxAge = 8.9f, float growthRate = 1.0f,
		float g1Init = 1.0f, float g2Init = -0.2f, float lengthMult = 0.3f, float thickMult = 0.4f);
	
	void initAndAddSpecies(fpreal t/* TODO add parameters*/, int defaultSpeciesType = 0,
		float temp = 28.0f, float precip = 4100.0f, float maxAge = 8.9f, float growthRate = 1.0f,
		float g1Init = 1.0f, float g2Init = -0.2f, float lengthMult = 0.3f, float thickMult = 0.4f);

	/// TEMP
	PlantSpecies* getSpeciesAtIdx(int idx);
	float getLikelihoodAtIdx(int idx) const;

	/// Report number of species
	int numSpecies() const;
	int numRandPlants() const;

private:
	void recalculateLikelihood();

    /// Accessors to simplify evaluating the parameters of the SOP. Called in cook
	float AGE(fpreal t)         { return evalFloat("timeShift", 0, t); }
	float TEMPERATURE(fpreal t) { return evalFloat("temperature", 0, t); }
	float RAINFALL(fpreal t)    { return evalFloat("rainfall", 0, t); }
	//float RANDOMNESS(fpreal t)  { return evalFloat("randomness", 0, t); }

    /// "Member variables are stored in the actual SOP, not with the geometry.
    /// These are just used to transfer data to the local variable callback.
    /// Another use for local data is a cache to store expensive calculations."
    int		myCurrPoint;
    int		myTotalPoints;

	int totalPlants;

	float worldAge;
	float worldTemp;
	float worldPrecip;

	std::vector<PlantSpecies*> speciesList;
	std::vector<float> speciesLikelihood;

	OP_Node* eco_merger;
	SOP_Node* scatterPoints;

	bool reloadPlants;
	bool generatePlants;
};
} // End HDK_Sample namespace

#endif
