#include "SOP_Plant.h"
#include "ECOSYSTEMPlugin.h"
using namespace HDK_Sample;

bool SOP_CustomSopOperatorFilter::allowOperatorAsChild(OP_Operator *op)
{
	// TODO change to just SOP operators maybe
	return true;//(dynamic_cast<sop_CustomVopOperator *>(op) != NULL);
}


// Declaring parameters here
static PRM_Name	plantAgeName("plantAge", "PlantAge");
static PRM_Name	      g1Name("g1",       "TropismDecrease");
static PRM_Name	      g2Name("g2",       "TropismStrength");
//				             ^^^^^^^^     ^^^^^^^^^^^^^^^
//				             internal     descriptive version

// Set up the initial/default values for the parameters
static PRM_Default plantAgeDefault(0.0);
static PRM_Default	     g1Default(1.0);
static PRM_Default	     g2Default(-0.2);

// Set up the ranges for the parameter inputs here
static PRM_Range plantAgeRange(PRM_RANGE_RESTRICTED,  0.0, PRM_RANGE_UI, 8.0);
static PRM_Range       g1Range(PRM_RANGE_RESTRICTED,  0.0, PRM_RANGE_UI, 3.0);
static PRM_Range       g2Range(PRM_RANGE_RESTRICTED, -1.0, PRM_RANGE_UI, 1.0);

////////////////////////////////////////////////////////////////////////////////////////

// Put them all together
PRM_Template
SOP_Plant::myTemplateList[] = {
	PRM_Template(PRM_FLT,    PRM_Template::PRM_EXPORT_MIN, 1, &plantAgeName, &plantAgeDefault, 0, &plantAgeRange),
	PRM_Template(PRM_FLT,    PRM_Template::PRM_EXPORT_MIN, 1, &g1Name,       &g1Default,       0, &g1Range),
	PRM_Template(PRM_FLT,    PRM_Template::PRM_EXPORT_MIN, 1, &g2Name,       &g2Default,       0, &g2Range),
	PRM_Template()
};

// Here's how we define local variables for the OBJ.
enum {
	VAR_PT		// Point number of the star
	//VAR_NPT		// Number of points in the star
};

CH_LocalVariable
SOP_Plant::myVariables[] = {
    { "PT",	VAR_PT, 0 },		// The table provides a mapping
    //{ "NPT", VAR_NPT, 0 },		// from text string to integer token
    { 0, 0, 0 },
};

const char *SOP_Plant::theChildTableName = SOP_TABLE_NAME;

/// Still unsure if we'll need this
bool
SOP_Plant::evalVariableValue(fpreal &val, int index, int thread)
{
    // myCurrPoint will be negative when we're not cooking so only try to
    // handle the local variables when we have a valid myCurrPoint index.
    if (myCurrPoint >= 0)
    {
		// Note that "gdp" may be null here, so we do the safe thing
		// and cache values we are interested in.
		switch (index)
		{
		case VAR_PT:
			val = (fpreal) myCurrPoint;
			return true;
		//case VAR_NPT:
		//	val = (fpreal) myTotalPoints;
		//	return true;
		default:
			/* do nothing */;
		}
    }
    // Not one of our variables, must delegate to the base class.
    return SOP_Node::evalVariableValue(val, index, thread);
}

OP_Node *
SOP_Plant::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
	SOP_Plant* newPlant = new SOP_Plant(net, name, op);

	//// Create a merge node to merge all sop output geom
	OP_Node* mergeNode = newPlant->createNode("merge");

	if (!mergeNode) { std::cout << "Merge Node is Nullptr" << std::endl; }
	else if (!mergeNode->runCreateScript())
		std::cout << "Merge constructor error" << std::endl;

	if (mergeNode) {
		mergeNode->moveToGoodPosition();
		//mergeNode->setBypass(false);
		//mergeNode->setDisplay(true);
		//mergeNode->setRender(true);

		newPlant->setMerger(mergeNode);
	}

	//// Create an output node for the network
	OP_Node* outNode = newPlant->createNode("output");
	
	if (!outNode) { std::cout << "Output Node is Nullptr" << std::endl; }
	else if (!outNode->runCreateScript())
		std::cout << "Output constructor error" << std::endl;

	if (outNode) {
		outNode->moveToGoodPosition();
		outNode->setDisplay(true);
		outNode->setRender(true);

		// Also handles connection to mergeNode aka newPlant->merger
		newPlant->setOutput(outNode);
	}

	//newPlant->setBypass(false);
	newPlant->moveToGoodPosition();
	//newPlant->triggerUIChanged(OP_UICHANGE_CONNECTIONS);

	//newPlant->output
	//newPlant->myOutputNodes.append(mergeNode);
	//newPlant->getOutputSop(0, true);
	//newPlant->setOutputForView(0);

	//
	//// Color for bark
	//OP_Node* color1 = newPlant->createNode("color");
	//if (!color1)
	//{
	//	std::cout << "Color is Nullptr" << std::endl;
	//	return newPlant;
	//}
	//else if (!color1->runCreateScript())
	//	std::cout << "Color constructor error" << std::endl;
	//
	//
	//OP_Node* mergeNode2 = newPlant->createNode("merge");
	//if (!mergeNode2) { std::cout << "Merge Node is Nullptr" << std::endl; }
	//else if (!mergeNode2->runCreateScript())
	//	std::cout << "Merge constructor error" << std::endl;
	//
	//
	//
	//// Copy to Points to allow leaf instancing
	//OP_Node* copyToPoints = newPlant->createNode("copytopoints");
	//if (!copyToPoints) 
	//{ 
	//	std::cout << "Copy To Points is Nullptr" << std::endl; 
	//	return newPlant;
	//}
	//else if (!copyToPoints->runCreateScript())
	//	std::cout << "Copy To Points constructor error" << std::endl;

	// Star will serve as a leaf for now
	/*OP_Node* star = newPlant->createNode("star");
	if (!star)
	{
		std::cout << "Star is Nullptr" << std::endl;
		return newPlant;
	}
	else if (!star->runCreateScript())
		std::cout << "Star constructor error" << std::endl;

	star->moveToGoodPosition();

	// Color for leaves
	OP_Node* color2 = newPlant->createNode("color");
	if (!color2)
	{
		std::cout << "Color is Nullptr" << std::endl;
		return newPlant;
	}
	else if (!color2->runCreateScript())
		std::cout << "Color constructor error" << std::endl;

	color2->connectToInputNode(*star, 0, 0);
	color2->moveToGoodPosition();*/


	//copyToPoints->connectToInputNode(*mergeNode, 1, 0);
	//
	//mergeNode2->connectToInputNode(*copyToPoints, 0, 0);
	//
	//mergeNode2->connectToInputNode(*color1, 1, 0);
	//
	//copyToPoints->moveToGoodPosition();
	//
	//mergeNode2->moveToGoodPosition();


	/*OP_Node* mergeNode3 = newPlant->createNode("merge");
	if (!mergeNode3) { std::cout << "Merge Node is Nullptr" << std::endl; }
	else if (!mergeNode3->runCreateScript())
		std::cout << "Merge constructor error" << std::endl;

	// copyToPoints2 to create instances of the tree for a forest
	OP_Node* copyToPoints2 = newPlant->createNode("copytopoints");
	if (!copyToPoints2)
	{
		std::cout << "Copy To Points is Nullptr" << std::endl;
		return newPlant;
	}
	else if (!copyToPoints2->runCreateScript())
		std::cout << "Copy To Points constructor error" << std::endl;


	
	// Grid to represent ground
	OP_Node* grid = newPlant->createNode("grid");
	if (!grid)
	{
		std::cout << "Grid is Nullptr" << std::endl;
		return newPlant;
	}
	else if (!grid->runCreateScript())
		std::cout << "Grid constructor error" << std::endl;

	grid->moveToGoodPosition();
	

	// Mountain to vary terrain
	OP_Node* mountain = newPlant->createNode("mountain");
	if (!mountain)
	{
		std::cout << "Mountain is Nullptr" << std::endl;
		return newPlant;
	}
	else if (!mountain->runCreateScript())
		std::cout << "Mountain constructor error" << std::endl;

	mountain->connectToInputNode(*grid, 0, 0);
	mountain->moveToGoodPosition();


	// Color for leaves
	OP_Node* color3 = newPlant->createNode("color");
	if (!color3)
	{
		std::cout << "Color is Nullptr" << std::endl;
		return newPlant;
	}
	else if (!color3->runCreateScript())
		std::cout << "Color constructor error" << std::endl;


	// Scatter to create points
	OP_Node* scatter = newPlant->createNode("scatter");
	if (!scatter)
	{
		std::cout << "Scatter is Nullptr" << std::endl;
		return newPlant;
	}
	else if (!scatter->runCreateScript())
		std::cout << "Scatter constructor error" << std::endl;

	scatter->connectToInputNode(*mountain, 0, 0);
	scatter->moveToGoodPosition();


	copyToPoints2->connectToInputNode(*mergeNode2, 0, 0);
	copyToPoints2->connectToInputNode(*scatter, 1, 0);
	copyToPoints2->moveToGoodPosition();


	mergeNode3->connectToInputNode(*copyToPoints2, 0, 0);
	mergeNode3->connectToInputNode(*color3, 1, 0);
	mergeNode3->moveToGoodPosition();

	color3->connectToInputNode(*mountain, 0, 0);
	color3->moveToGoodPosition();*/
	
	//color1->connectToInputNode(*mergeNode, 0, 0);
	//color1->moveToGoodPosition();

	return newPlant;
}

SOP_Plant::SOP_Plant(OP_Network *net, const char *name, OP_Operator *op)
	: SOP_Node(net, name, op), plantType(nullptr), ecosystem(nullptr),
	rootModule(nullptr), merger(nullptr)
{
	createAndGetOperatorTable();
	myCurrPoint = -1;	// To prevent garbage values from being returned
	plantAge = 0.0f;
	///setAllowBuildDependencies
	///mySopFlags.setManagesDataIDs(true);
}

SOP_Plant::~SOP_Plant() {}

/// Unsure if we'll need this
unsigned
SOP_Plant::disableParms()
{
    return 0;
}

/*OP_ERROR
SOP_Plant::cookMe(OP_Context &context)
{
	return error();
}*/


OP_ERROR
SOP_Plant::cookMySop(OP_Context &context)
{
	std::cout << "PLANT COOK START" << std::endl;
	fpreal now = context.getTime();
	myCurrPoint = 0;

	// Get current plant-related values
	float ageVal;
	float g1Val;
	float g2Val;

	ageVal = AGE(now);
	g1Val  = G1(now);
	g2Val  = G2(now);

	BNode::updateG1(g1Val);
	BNode::updateG2(g2Val);
	// TODO If we want to also add growth-coeff and thick-coeff as variables here?
	// for thickness we would only need to rerun the traversal unless time also changes
	// But this might be more of a prototype-designer sort of thing
    
    // For ecosystem
	if (ecosystem) {
		std::cout << "ECO Adding interest" << std::endl;
		addExtraInput(ecosystem, OP_INTEREST_DATA);
	}

	if (rootModule) {
		rootModule->setAge(ageVal - plantAge);
		//rootModule->forceRecook(/*bool evensmartcache = true*/);
		//rootModule->cook(context);
	}

	std::cout << "Num " + std::to_string(nOutputEntries()) << std::endl;
	std::cout << "If " + std::to_string(hasAnyOutputNodes()) << std::endl;

	plantAge = ageVal;
	////myDisplayNodePtr
	//resetDisplayNodePtr

    myCurrPoint = -1;
	std::cout << "PLANT COOK END" << std::endl;
	return error();//errorstatus;
}

/// SETTERS

void SOP_Plant::setRootModule(SOP_Branch* node) {
	/// SINGLE PLANT
	rootModule = node;
	rootModule->setPlantAndPrototype(this, 0.0f, 0.0f);
	rootModule->setAge(0.0f);
}

void SOP_Plant::setMerger(OP_Node* mergeNode) {
	merger = mergeNode;
}

void SOP_Plant::setOutput(OP_Node* outNode) {
	output = outNode;

	myDisplayNodePtr = output;
	myRenderNodePtr = output;
	myOutputNodes.append(output);

	if (merger) { output->connectToInputNode(*merger, 0, 0); }
}

void SOP_Plant::initPlant(OBJ_Plant * eco)
{
	ecosystem = eco;
	// TODO choose plantType based on climate OR through direct input from seeding
	//setPrototypeList();
	if (!ecosystem->plantTypes.empty()) {
		plantType = ecosystem->plantTypes.at(0);

		if (merger) {
			// Create the root branch module
			OP_Node* branchNode_OP = createNode("BranchModule");

			if (!branchNode_OP) { std::cout << "Root module is Nullptr" << std::endl; }
			else if (!branchNode_OP->runCreateScript())
				std::cout << "Root module constructor error" << std::endl;

			if (branchNode_OP) {
				SOP_Branch* branchNode = (SOP_Branch*)branchNode_OP;
				//branchNode->setBypass(false);
				setRootModule(branchNode);
				addToMerger(branchNode);
				branchNode_OP->moveToGoodPosition();
			}

			std::cout << "Made branch and merge nodes" << std::endl;
		}
		//triggerUIChanged(OP_UICHANGE_CONNECTIONS);
	}
	else {
		std::cout << "NO PLANT TYPES ARE DEFINED" << std::endl;
	}
}

float SOP_Plant::getAge() {
	return plantAge;
}

/// Functions related to making this a network
//int SOP_Plant::isNetwork() const
//{
//	return 1;
//}

// Defining the children
const char *
SOP_Plant::getChildType() const
{
	return SOP_OPTYPE_NAME;
}

OP_OpTypeId
SOP_Plant::getChildTypeID() const
{
	return SOP_OPTYPE_ID;
}

// Defining self
const char *
SOP_Plant::getOpType() const
{
	return SOP_MANAGEMENT_OPTYPE;
}

OP_OpTypeId
SOP_Plant::getOpTypeID() const
{
	return SOP_OPTYPE_ID;
}

//OP_Node*
//SOP_Plant::getDisplayNodePtr()
//{
//	// TODO add if output else this
//	return output;
//}
//
//OP_Node*
//SOP_Plant::getRenderNodePtr()
//{
//	// TODO add if output else this
//	return output;
//}

OP_OperatorTable *
SOP_Plant::createAndGetOperatorTable()
{
    // We chain our custom VOP operators onto the default VOP operator table.
    OP_OperatorTable &table = *OP_Network::getOperatorTable(SOP_TABLE_NAME);
	// TODO maybe add Branch Module here since it's dependent on parent

	table.addOperator(new OP_Operator("hdk_inout11_",
		"In-Out 1-1",
		SOP_CustomOutput::myConstructor,
		SOP_CustomOutput::myTemplateList,
		SOP_CustomOutput::theChildTableName,
		0,
		10,
		NULL,
		OP_FLAG_UNORDERED)
	);
    // Procedurally create some simple operator types for illustrative purposes.
    //table.addOperator(new sop_CustomVopOperator("hdk_inout11_", "In-Out 1-1"));
    //table.addOperator(new sop_CustomVopOperator("hdk_inout21_", "In-Out 2-1"));
    //table.addOperator(new sop_CustomVopOperator("hdk_inout12_", "In-Out 1-2"));
    //table.addOperator(new sop_CustomVopOperator("hdk_inout22_", "In-Out 2-2"));

    // Notify observers of the operator table that it has been changed.
    table.notifyUpdateTableSinksOfUpdate();

    return &table;
}
///

void SOP_Plant::addToMerger(SOP_Branch* bMod) {
	// Get unique input path
	UT_String path;
	bMod->getFullPath(path);
	merger->setNamedInput(path.hash(), bMod);
}

// Generate a prototype copy for editing in branch node
BranchPrototype* SOP_Plant::copyPrototypeFromList(float lambda, float determ) {
	//return ecosystem->copyPrototypeFromList(lambda, determ);
	return plantType->copyPrototypeFromList(lambda, determ);
}

///////////////////////////////////////////////////////
//// SOP CUSTOM OUTPUT NODE ///////////////////////////

static PRM_Name    sopCustomPlugInputs("inputs", "Inputs");
static PRM_Name    sopCustomPlugInpName("inpplug#", "Input Name #");
static PRM_Default sopCustomPlugInpDefault(0, "input1");
static PRM_Name    sopCustomPlugOutputs("outputs", "Outputs");
static PRM_Name    sopCustomPlugOutName("outplug#", "Output Name #");
static PRM_Default sopCustomPlugOutDefault(0, "output1");

static PRM_Template
sopCustomPlugInpTemplate[] =
{
    PRM_Template(PRM_ALPHASTRING, 1, &sopCustomPlugInpName, &sopCustomPlugInpDefault),
    PRM_Template() // List terminator
};
static PRM_Template
sopCustomPlugOutTemplate[] =
{
    PRM_Template(PRM_ALPHASTRING, 1, &sopCustomPlugOutName, &sopCustomPlugOutDefault),
    PRM_Template() // List terminator
};

/// Stores the description of the interface of the SOP in Houdini.
PRM_Template
SOP_CustomOutput::myTemplateList[]= 
{
    PRM_Template(PRM_MULTITYPE_LIST, sopCustomPlugInpTemplate, 0, &sopCustomPlugInputs,
                 PRMzeroDefaults, 0, &PRM_SpareData::multiStartOffsetZero),

    PRM_Template(PRM_MULTITYPE_LIST, sopCustomPlugOutTemplate, 0, &sopCustomPlugOutputs,
                 PRMzeroDefaults, 0, &PRM_SpareData::multiStartOffsetZero),

    PRM_Template()              // List terminator
};

OP_Node* 
SOP_CustomOutput::myConstructor(OP_Network* net, const char* name, OP_Operator* op) {
	return new SOP_CustomOutput(net, name, op);
}

SOP_CustomOutput::SOP_CustomOutput(OP_Network* net, const char* name, OP_Operator* op)
	: SOP_Node(net, name, op)
{
	// Add our event handler.
	addOpInterest(this, &SOP_CustomOutput::nodeEventHandler);
}
SOP_CustomOutput::~SOP_CustomOutput() {
	removeOpInterest(this, &SOP_CustomOutput::nodeEventHandler);
}

/// Overridden for some reason!
bool 
SOP_CustomOutput::runCreateScript() {
	if (!SOP_Node::runCreateScript()) { return false; }

	fpreal        t = CHgetEvalTime();
	UT_WorkBuffer plugname;

	// TODO change
	// For simplicity, we just initialize our number of inputs/outputs based
	// upon our node type name.
	const UT_StringHolder& type_name = getOperator()->getName();
	int n = type_name.c_str()[type_name.length() - 3] - '0';
	setInt(sopCustomPlugInputs.getToken(), 0, t, n);

	for (int i = 0; i < n; i++)
	{
	    plugname.sprintf("input%d", i + 1);
	    setStringInst(plugname.buffer(), CH_STRING_LITERAL,
			sopCustomPlugInpName.getToken(), &i, 0, t);
	}
	
	n = type_name.c_str()[type_name.length() - 2] - '0';
	setInt(sopCustomPlugOutputs.getToken(), 0, t, n);

	int i = 0;
	plugname.sprintf("output%d", i + 1);
	setStringInst(plugname.buffer(), CH_STRING_LITERAL,
		sopCustomPlugOutName.getToken(), &i, 0, t);
	
	return true;
}

/// Provides the labels to appear on input and output buttons.
const char* 
SOP_CustomOutput::inputLabel(unsigned idx) const {
	static UT_WorkBuffer theLabel;
	UT_String label;
	int i = idx;
	
	// Evaluate our label from the corresponding parameter.
	evalStringInst(sopCustomPlugInpName.getToken(), &i, label, 0, CHgetEvalTime());
	
	if (label.isstring()) { theLabel.strcpy(label); }
	else { theLabel.strcpy("<unnamed>"); }
	
	return theLabel.buffer();
}

const char* 
SOP_CustomOutput::outputLabel(unsigned idx) const {
	static UT_WorkBuffer theLabel;
	UT_String label;
	int i = idx;

	// Evaluate our label from the corresponding parameter.
	evalStringInst(sopCustomPlugOutName.getToken(), &i, label, 0, CHgetEvalTime());

	if (label.isstring()) { theLabel.strcpy(label); }
	else { theLabel.strcpy("<unnamed>"); }

	return theLabel.buffer();
}

/// Controls the number of input/output buttons visible on the node tile.
unsigned 
SOP_CustomOutput::getNumVisibleInputs() const {
	return evalInt("inputs", 0, CHgetEvalTime());
}

unsigned 
SOP_CustomOutput::getNumVisibleOutputs() const {
	return evalInt("outputs", 0, CHgetEvalTime());
}

void 
SOP_CustomOutput::nodeEventHandler(OP_Node *caller, void *callee,
	OP_EventType type, void *data) {
	switch (type)
	{
	    case OP_PARM_CHANGED:
	        static_cast<SOP_CustomOutput*>(callee)->handleParmChanged((int)(intptr_t)data);
	        break;
	    default:
	        break;
	}
}

void 
SOP_CustomOutput::handleParmChanged(int parm_index) {
	triggerUIChanged(OP_UICHANGE_CONNECTIONS);
}