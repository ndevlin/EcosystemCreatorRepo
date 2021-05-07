#ifndef __ECOSYSTEM_PLUGIN_h__
#define __ECOSYSTEM_PLUGIN_h__

#include <OBJ/OBJ_Geometry.h>
#include <SOP_Branch.h>
#include <SOP_Plant.h>

namespace HDK_Sample {
class OBJ_Plant : public OBJ_Geometry
{
public:
    static OP_Node		*myConstructor(OP_Network*, const char *,
							    OP_Operator *);

    /// Stores the description of the interface of the SOP in Houdini.
    /// Each parm template refers to a parameter.
    static PRM_Template		 myTemplateList[];
	static OP_TemplatePair*  buildTemplatePair(OP_TemplatePair *baseTemplate);

    /// This optional data stores the list of local variables.
    static CH_LocalVariable	 myVariables[];
	static OP_VariablePair*  buildVariablePair(OP_VariablePair *baseVariable);

	/// Copy's a prototype instance, used as a base for a new branch module
	BranchPrototype*         copyPrototypeFromList(float lambda, float determ);
	/// Add the corresponding node to the group output geometry
	void                     addToMerger(OP_Node* pNode);
	// TODO: add a better merger remover. Duplicate inputs end up existing to root???

	float                     getAge();

	void initPlantType(/* TODO add parameters*/);

	std::vector<std::shared_ptr<PlantType>> plantTypes;

protected:

	OBJ_Plant(OP_Network *net, const char *name, OP_Operator *op);
    virtual ~OBJ_Plant();

    /// Disable parameters according to other parameters.
    //virtual unsigned		 disableParms();


    /// Do the actual Branch SOP computing
    virtual OP_ERROR		 cookMyObj(OP_Context &context);

    /// This function is used to lookup local variables that you have
    /// defined specific to your SOP.
    /*virtual bool evalVariableValue(fpreal &val, int index, int thread);
    // Add virtual overload that delegates to the super class to avoid
    // shadow warnings.
    virtual bool evalVariableValue(UT_String &v, int i, int thread)
				 { return evalVariableValue(v, i, thread); }*/

	//void setPrototypeList();
	void setRootModule(SOP_Branch* node);
	void setMerger(OP_Node* mergeNode);
	SOP_Plant* createPlant(/*add position maybe*/);

private:
    /// Accessors to simplify evaluating the parameters of the SOP. Called in cook
	float AGE(fpreal t)      { return evalFloat("ecoAge", 0, t); }
	float EG1(fpreal t)      { return evalFloat("ecog1",  0, t); }
	float EG2(fpreal t)      { return evalFloat("ecog2",  0, t); }

    /// "Member variables are stored in the actual SOP, not with the geometry.
    /// These are just used to transfer data to the local variable callback.
    /// Another use for local data is a cache to store expensive calculations."
    int		myCurrPoint;
    int		myTotalPoints;

	float worldAge;

	PrototypeSet* prototypeSet;

	/// SINGLE PLANT
	/// SOP_Branch* rootModule;
	//std::vector<SOP_Branch*> rootModules;
	//int numRootModules;
	///

	OP_Node* eco_merger;
};
} // End HDK_Sample namespace

#endif
