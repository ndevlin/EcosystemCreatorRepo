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

#include <GU/GU_PrimSphere.h>

#include <limits.h>

namespace HDK_Sample {
class SOP_Plant;

extern int branchIDnum;

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

	/// Important: updates all time-based values in all modules. Does all main calculations
	void setAge(float changeInAge); // TODO should probaby split up

	/// Set up plant pointer, selected prototype data, and initializes root and ageRange
	void setPlantAndPrototype(SOP_Plant* p, float lambda, float determ);

	/// While setting the parent module, also alters current node data based on parent SOP_Branch
	void setParentModule(SOP_Branch* parModule, float newAge,
		std::shared_ptr<BNode> connectingNode = nullptr);

	/// Disconnect and delete this SOP_Branch
	void destroySelf();

protected:

	     SOP_Branch(OP_Network *net, const char *name, OP_Operator *op);
    virtual ~SOP_Branch();

    /// Disable parameters according to other parameters.
    virtual unsigned		 disableParms();

    /// cookMySop does the actual work of the SOP Branch computing
    virtual OP_ERROR		 cookMySop(OP_Context &context);

    /// This function is used to lookup local variables that you have
    /// defined specific to your SOP.
    virtual bool evalVariableValue(fpreal &val, int index, int thread);
    // Add virtual overload that delegates to the super class to avoid
    // shadow warnings.
    virtual bool evalVariableValue(UT_String &v, int i, int thread)
				 {
				     return evalVariableValue(v, i, thread);
				 }

	virtual bool cookDataForAnyOutput()	const { return true; }

	// TODO add rotate module function / module placement

private:
	/// Update the current agent rig with the transformations of nodes
	void setTransforms(std::shared_ptr<BNode> currNode);

	/// Swaps tree beginning at "root" to be the appropriately aged prototype copy
	void setRootByAge(float time);

    /// LOCAL VARIABLES:
    int		myCurrPoint; /// Just used to tell when cooking

	int		branchID; /// Custom ID number of branch

	bool init_agent;   // These two are currently unused, meant to only complete
	bool change_agent; // certain agent-related tasks in cook

	GU_Agent*	    moduleAgent;  /// Current agent made from PrototypeAgentPtr
	GU_PrimPacked*  packedPrim;   /// The prim point at this agent

	SOP_Plant*		 plant;		  /// The plant that this module is contained in
	BranchPrototype* prototype;   /// The prototype this module took as reference

	std::pair<float, float> currAgeRange;  /// Defines which age of the prototype we are looking at
	std::shared_ptr<BNode>  root;		   /// Root of the current aged prototype
											// TODO swap currAgeRange to int index for storage purposes
	SOP_Branch* parentModule;				/// If not root, the module this branches off from
	std::vector<SOP_Branch*> childModules;  /// The modules branching off of this one
};
} // End HDK_Sample namespace

#endif
