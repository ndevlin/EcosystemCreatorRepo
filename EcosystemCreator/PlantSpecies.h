#ifndef __PLANT_SPECIES_h__
#define __PLANT_SPECIES_h__

#include <SOP/SOP_Node.h>
#include <BranchPrototype.h>
//#include <OBJ/OBJ_Geometry.h>

namespace HDK_Sample {

class PlantSpecies : public SOP_Node // public OBJ_Geometry
{
public:
	static OP_Node		*myConstructor(OP_Network*, const char *, OP_Operator *);

	/// Stores the description of the interface of the SOP in Houdini.
	/// Each parm template refers to a parameter.
	static PRM_Template		 myTemplateList[];

	/// This optional data stores the list of local variables.
	static CH_LocalVariable	 myVariables[];

	/// Pairs the local interface of OBJ_Ecosystem with parent class OBJ_Geometry
	// static OP_TemplatePair*  buildTemplatePair(OP_TemplatePair *baseTemplate);
	// static OP_VariablePair*  buildVariablePair(OP_VariablePair *baseVariable);

	/// Generate a prototype copy to store as an editable tree in a SOP_Branch
	BranchPrototype*         copyPrototypeFromList(float lambda, float determ);

	void tempPath(const char* path);

protected:
	PlantSpecies(const char* path,
		OP_Network *net, const char *name, OP_Operator *op);
	~PlantSpecies();

	/// Do the actual change-based computataions
	//virtual OP_ERROR		 cookMyObj(OP_Context &context);
	virtual OP_ERROR		 cookMySop(OP_Context &context);

private:
	PrototypeSet* prototypeSet; // TODO Make unique pointer
};
} // End HDK_Sample namespace
#endif