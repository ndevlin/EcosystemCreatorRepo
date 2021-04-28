#ifndef __SOP_BRANCH_h__
#define __SOP_BRANCH_h__

#include <SOP/SOP_Node.h>
#include <BranchPrototype.h>

#include <UT/UT_Math.h>
#include <UT/UT_Interrupt.h>
#include <GU/GU_Detail.h>
#include <GU/GU_PrimPoly.h>
#include <GU/GU_PrimMesh.h>
#include <CH/CH_LocalVariable.h>
#include <PRM/PRM_Include.h>
#include <PRM/PRM_SpareData.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>

#include <limits.h>

namespace HDK_Sample {
class OBJ_Plant;

class SOP_Branch : public SOP_Node
{
public:
    static OP_Node		*myConstructor(OP_Network*, const char *,
							    OP_Operator *);

    /// Stores the description of the interface of the SOP in Houdini.
    /// Each parm template refers to a parameter.
    static PRM_Template		 myTemplateList[];

    /// This optional data stores the list of local variables.
    static CH_LocalVariable	 myVariables[];

	/// Set up plant pointer, selected prototype data, and initializes root and ageRange
	void setPlantAndPrototype(OBJ_Plant* p, float lambda, float determ, int rootIndexIn);

	/// While setting the parent module, also alters current node data based on last branch
	void setParentModule(SOP_Branch* parModule, BNode* connectingNode = nullptr);

	/// Important: updates all time-based values in all modules. Does all main calculations
	void setAge(float changeInAge); // TODO should probaby split up

	void destroySelf();

	float rainfall;
	float temperature;

protected:

	     SOP_Branch(OP_Network *net, const char *name, OP_Operator *op);
    virtual ~SOP_Branch();

    /// Disable parameters according to other parameters.
    virtual unsigned		 disableParms();


    /// cookMySop does the actual work of the SOP computing, in this
    /// case, a Branch
    virtual OP_ERROR		 cookMySop(OP_Context &context);

    /// This function is used to lookup local variables that you have
    /// defined specific to your SOP.
    virtual bool evalVariableValue(
				    fpreal &val,
				    int index,
				    int thread);
    // Add virtual overload that delegates to the super class to avoid
    // shadow warnings.
    virtual bool evalVariableValue(
				    UT_String &v,
				    int i,
				    int thread)
				 {
				     return evalVariableValue(v, i, thread);
				 }

	//bool cookDataForAnyOutput() const override { return true; }
	virtual bool cookDataForAnyOutput()	const { return true; }


	// TODO add rotate module function

private:
    /// Traverse all nodes in this module to create cylinder geometry
	void traverseAndBuild(GU_Detail* gdp, BNode* currNode, int divisions); 
							// TODO can go fully procedural by just adding parent pos as input 
							// then wont need to copy over prototypes every time

	/// Swaps to whichever is the currently-aged prototype to reference
	void setRootByAge(float time);

    /// Any local variables:
    int		myCurrPoint;
    int		myTotalPoints;

	OBJ_Plant* plant;
	BranchPrototype* prototype;

	std::pair<float, float> currAgeRange;
	BNode* root;
	int rootIndex;

	SOP_Branch* parentModule;
	std::vector<SOP_Branch*> childModules;
};
} // End HDK_Sample namespace

#endif
