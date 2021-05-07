#ifndef __SOP_PLANT_h__
#define __SOP_PLANT_h__

#include <SOP/SOP_Node.h>
#include <SOP_Branch.h>
#include "PlantType.h"

#include <CH/CH_Manager.h>

namespace HDK_Sample {
class OBJ_Plant;

class SOP_CustomSopOperatorFilter : public OP_OperatorFilter
{
public:
    bool allowOperatorAsChild(OP_Operator *op) override;
};

class SOP_Plant : public SOP_Node
{
public:
    static OP_Node		*myConstructor(OP_Network*, const char *,
							    OP_Operator *);

    /// Stores the description of the interface of the SOP in Houdini.
    /// Each parm template refers to a parameter.
    static PRM_Template		 myTemplateList[];

    /// This optional data stores the list of local variables.
    static CH_LocalVariable	 myVariables[];

	/// Copy's a prototype instance, used as a base for a new branch module
	BranchPrototype*         copyPrototypeFromList(float lambda, float determ);
	/// Add the corresponding node to the group output geometry
	void                     addToMerger(SOP_Branch* bMod);
	// TODO: add a better merger remover. Duplicate inputs end up existing to root???

	float                     getAge();

	int						  isNetwork() const override;
	int					      isSubNetwork(bool includemanagementops) const override;

	/// We override these to specify that our child network type is VOPs.
    const char*               getChildType() const override;
    OP_OpTypeId               getChildTypeID() const override;

	const char*				  getOpType() const override;
	OP_OpTypeId				  getOpTypeID() const override;

	/// Override this to provide custom behaviour for what is allowed in the
    /// tab menu.
    OP_OperatorFilter*        getOperatorFilter() override
                                    { return &myOperatorFilter; }

	static const char*        theChildTableName;

	/// Provides the labels to appear on input and output buttons.
	// @{
	const char *             inputLabel(unsigned idx) const override;
	const char *             outputLabel(unsigned idx) const override;
	// @}
	/// Controls the number of input/output buttons visible on the node tile.
	// @{
	unsigned                 getNumVisibleInputs() const override;
	unsigned                 getNumVisibleOutputs() const override;
	// @}

	void initPlant(OBJ_Plant* eco);

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

	/// Inspired by custom vop example
	OP_OperatorTable *       createAndGetOperatorTable();

    /// This function is used to lookup local variables that you have
    /// defined specific to your SOP.
    virtual bool evalVariableValue(fpreal &val, int index, int thread);
    // Add virtual overload that delegates to the super class to avoid
    // shadow warnings.
    virtual bool evalVariableValue(UT_String &v, int i, int thread)
				 { return evalVariableValue(v, i, thread); }

	//void setPrototypeList();
	void setRootModule(SOP_Branch* node);
	void setMerger(OP_Node* mergeNode);
	void setOutput(SOP_Node* outNode);

private:
	SOP_CustomSopOperatorFilter myOperatorFilter;

    /// Accessors to simplify evaluating the parameters of the SOP. Called in cook
	float AGE(fpreal t)     { return evalFloat("plantAge", 0, t); }
	float G1(fpreal t)      { return evalFloat("g1",       0, t); }
	float G2(fpreal t)      { return evalFloat("g2",       0, t); }

    /// "Member variables are stored in the actual SOP, not with the geometry.
    /// These are just used to transfer data to the local variable callback.
    /// Another use for local data is a cache to store expensive calculations."
    int		myCurrPoint;

	float plantAge;

	std::shared_ptr<PlantType> plantType;
	OBJ_Plant* ecosystem;

	/// SINGLE PLANT
	SOP_Branch* rootModule; // TODO make adaptable

	OP_Node* branchNet;
	OP_Node* merger;
	SOP_Node* output;// TODO make adaptable - if suddenly null (or when UI Change Event), 
					//replace with lowest non-branchModule node
};




class SOP_CustomOutput : public SOP_Node
{
public:
	static OP_Node		*myConstructor(OP_Network*, const char *,
							    OP_Operator *);

	/// Stores the description of the interface of the SOP in Houdini.
	static PRM_Template		 myTemplateList[];

	/// Overridden for some reason!
	bool                     runCreateScript() override;

	/// Provides the labels to appear on input and output buttons.
	// @{
	const char *             inputLabel(unsigned idx) const override;
	const char *             outputLabel(unsigned idx) const override;
	// @}
	/// Controls the number of input/output buttons visible on the node tile.
	// @{
	unsigned                 getNumVisibleInputs() const override;
	unsigned                 getNumVisibleOutputs() const override;
	// @}

protected:
	SOP_CustomOutput(OP_Network *net, const char *name, OP_Operator *op);
	~SOP_CustomOutput() override;

	virtual OP_ERROR		 cookMySop(OP_Context &context) { return error(); }

	/// Internal name of input, used by getCode
	//void                     getInputNameSubclass(UT_String &name,
	//								int idx) const override;
	//int                      getInputFromNameSubclass(const UT_String &name) const override;

private: 
	static void              nodeEventHandler(OP_Node *caller, void *callee,
											  OP_EventType type, void *data);

	void                     handleParmChanged(int parm_index);
};

} // End HDK_Sample namespace

#endif
