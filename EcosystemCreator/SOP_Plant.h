#ifndef __SOP_PLANT_h__
#define __SOP_PLANT_h__

#include <SOP/SOP_Node.h>
#include <SOP_Branch.h>
#include "PlantSpecies.h"

#include <CH/CH_Manager.h>

namespace HDK_Sample {
class OBJ_Ecosystem;

/////// CUSTOM FILTER - for child nodes. At this point allows everything ///////
class SOP_CustomSopOperatorFilter : public OP_OperatorFilter
{
public:
    bool allowOperatorAsChild(OP_Operator *op) override;
};

//////// THE PLANT NODE ITSELF - Essentially a network of SOP_Branches /////////
class SOP_Plant : public SOP_Node
{
public:
	// NODE CONSTRUCTION
	/// Initialize plant sop and important children (merger and output)
    static OP_Node		*myConstructor(OP_Network*, const char *,
							    OP_Operator *);

    /// Stores the description of the interface of the SOP in Houdini.
    /// Each parm template refers to a parameter.
    static PRM_Template		 myTemplateList[];

    /// This optional data stores the list of local variables.
    static CH_LocalVariable	 myVariables[];

	// NETWORK FUNCTIONS: Override parent classes to allow for network functionality
	/// Tells houdini that this node can contain children
	int						 isNetwork() const override;
	int					     isSubNetwork(bool includemanagementops) const override;

	/// We override these to specify that our child network type is VOPs.
    const char*              getChildType() const override;
    OP_OpTypeId              getChildTypeID() const override;

	const char*				 getOpType() const override;
	OP_OpTypeId				 getOpTypeID() const override;

	/// Override this to provide custom behaviour for what is allowed in the
    /// tab menu.
    OP_OperatorFilter*       getOperatorFilter() override
                                    { return &myOperatorFilter; }

	static const char*       theChildTableName;

	/// Provides the labels to appear on input and output buttons.
	// @{
	const char*              inputLabel(unsigned idx) const override;
	const char*              outputLabel(unsigned idx) const override;
	// @}
	/// Controls the number of input/output buttons visible on the node tile.
	// @{
	unsigned                 getNumVisibleInputs() const override;
	unsigned                 getNumVisibleOutputs() const override;
	// @}

	// PLANT SOP FUNCTIONS
	/// Initialize the actual plant based on the environment (root SOP_Branch)
	void initPlant(OBJ_Ecosystem* eco, PlantSpecies* currSpecies, float worldTime);

	/// Generate a prototype copy to store as an editable tree in a SOP_Branch
	BranchPrototype*         copyPrototypeFromList(float lambda, float determ);

	/// Add the corresponding node as an input to the stored merge node
	void                     addToMerger(SOP_Branch* bMod);
	// TODO: confirm that a remove-from-merge function is unneded

	/// Sets the world space plant position
	void                     setPosition(UT_Vector3 origin);
	UT_Vector3               getPosition();

	/// Get the age of this plant - used in child SOP_Branches
	float                    getAge() const;
	/// Get the birth of this plant
	float                    getBirthTime() const;
	/// Calculate the age impacted by growth rate
	float                    calcWeightedAge();
	/// Get the change in age - only works mid-cook
	float                    getChangeInAge();

	/// Disconnect and delete this Plant
	void				     destroySelf();

protected:

	SOP_Plant(OP_Network *net, const char *name, OP_Operator *op);
    virtual ~SOP_Plant();

    /// Disable parameters according to other parameters.
    //virtual unsigned		 disableParms();

    /// Do the actual Plant SOP computing
	virtual OP_ERROR		 cookMySop(OP_Context &context);
	GU_DetailHandle			 cookMySopOutput(OP_Context &context,
								int outputidx, SOP_Node* interests) override;

	virtual bool			 cookDataForAnyOutput() const override
								{ return true; }

	/// Inspired by custom vop example - setting up this network's table
	OP_OperatorTable *       createAndGetOperatorTable();

    /// This function is used to lookup local variables that you have
    /// defined specific to your SOP.
    virtual bool evalVariableValue(fpreal &val, int index, int thread);

    // Add virtual overload that delegates to the super class to avoid
    // shadow warnings.
    virtual bool evalVariableValue(UT_String &v, int i, int thread)
				 { return evalVariableValue(v, i, thread); }

	// PLANT SOP FUNCTIONS
	/// Stores a pointer to the root SOP_Branch, calls setPlantAndPrototype, sets its age
	void setRootModule(SOP_Branch* node);

	/// Stores the merge node that combines all branch geometry
	void setMerger(OP_Node* mergeNode);

	/// Stores the initial output of the network, sets as display/render node
	void setOutput(SOP_Node* outNode);

private:
	SOP_CustomSopOperatorFilter myOperatorFilter;

    /// Accessors to simplify evaluating the parameters of the SOP. Called in cook
	float AGE(fpreal t)     { return evalFloat("plantAge", 0, t); }

    /// "Member variables are stored in the actual SOP, not with the geometry.
    /// These are just used to transfer data to the local variable callback.
    /// Another use for local data is a cache to store expensive calculations."
    int	  myCurrPoint;

	/// PLANT
	UT_Vector3 plantPos;
	float plantAge;		 /// The plant's age as compared to Ecosystem age and birthday
	float plantBirthday; /// The age the ecosystem was when the plant spawned

	/// ENVIRONMENTAL CONTROL
	OBJ_Ecosystem* ecosystem;	 /// The ecosystem this was spawned in
	PlantSpecies* plantSpecies;  /// The species info for this plant

	/// CHILD NODES:
	/// The root Branch Module of this tree
	SOP_Branch* rootModule;

	/// Nodes to organize the network
	OP_Node*  merger;  /// The merge node all Branch outputs are connected to
	SOP_Node* output;  /// The current View/Display output of this node, controlled in Cook

};
} // End HDK_Sample namespace

#endif

