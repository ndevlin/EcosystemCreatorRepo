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

	/// Initializes a plant node in this ecosystem. The first using a randomly chosen species
	SOP_Plant* createPlant(/*add position maybe*/);
	SOP_Plant* createPlant(PlantSpecies* currSpecies /*add position maybe*/);
	//SOP_Plant* createPlant(std::shared_ptr<PlantType> currSpecies /*add position maybe*/);
			// TODO, seeding (in SOP_Plant)

	/// Choose a likely plant species to spawn based on current spawn location's climate features
	PlantSpecies* chooseSpecies(/* TODO use enviro parameters at curr location */);
	//std::shared_ptr<PlantType> chooseSpecies(/* TODO use enviro parameters at curr location */);

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
	void initNewSpecies(fpreal t/* TODO add parameters*/);
	void initAndAddSpecies(fpreal t/* TODO add parameters*/);

private:
    /// Accessors to simplify evaluating the parameters of the SOP. Called in cook
	float AGE(fpreal t)      { return evalFloat("timeShift", 0, t); }

    /// "Member variables are stored in the actual SOP, not with the geometry.
    /// These are just used to transfer data to the local variable callback.
    /// Another use for local data is a cache to store expensive calculations."
    int		myCurrPoint;
    int		myTotalPoints;

	float worldAge;

	std::vector<PlantSpecies*> speciesList;
	//std::vector<std::shared_ptr<PlantType>> speciesList;

	OP_Node* eco_merger;
};
} // End HDK_Sample namespace

#endif
