#ifndef BranchPrototype_H_
#define BranchPrototype_H_
#include "LSystem.h"
#include "PrototypeAgentPtr.h"

///// INDIVIDUAL PROTOTYPES /////

class BranchPrototype
{
public:
	BranchPrototype(const char* path, PlantSpeciesVariables* plantVars);
	BranchPrototype(const char* path, PlantSpeciesVariables* plantVars, 
		const std::string& grammarProgram, int iterations); /// From LSystem grammar
	BranchPrototype(const char* path, PlantSpeciesVariables* plantVars, 
		LSystem& lsystem, int iterations);					/// From existing LSystem
				 // TODO From existing node structure

	// Deep copy
	BranchPrototype(BranchPrototype* other);
	~BranchPrototype() {}

	// TODO dear god find a better way
	BranchPrototype* copyValues();

	// TODO add const
	int getNumAges();		/// Returns number of ranges
	float getMaturityAge(); /// Returns the upper bound on the last age range

	/// Get the index to value in agedPrototypes that corresponds with this time
	int getIdxAtTimestep(float time);

	/// Never used?
	//std::pair<float, float> getRangeAtTimestep(float time);
	//BNode* getShapeAtTimestep(float time);                 

	/// Get corresponding prototype values at index
	std::pair<float, float> getRangeAtIdx(int i);
	std::shared_ptr<BNode> getRootAtIdx(int i);
	GU_AgentDefinitionPtr getAgentDefAtIdx(int i);

	/// Static helper to compare float to range
	static bool isInRange(std::pair<float, float>& range, float time);

private:
	typedef std::pair<std::pair<float, float>, std::shared_ptr<BNode>> AgeGraph;

	std::vector<AgeGraph> agedPrototypes;
	std::vector<GU_AgentDefinitionPtr> agentData;

	/// Used to pass a pointer to the current plant's data to all related nodes
	void setPlantData(PlantSpeciesVariables* plantVars);

	/// Used to initialize geometry and agent definitions
	void initAgentData(const char* path);

	/// Used in L-System related constructors
	void setFromLSystem(LSystem& lsystem, int iterations);

	/// Binary search helper for getIdxAtTimestep(...)
	int searchHelper(int l, int r, float time);
};



///// CONTAINER FOR A SET OF PROTOTYPES /////

class PrototypeSet
{
public:
	PrototypeSet(const char* path, PlantSpeciesVariables* plantVars);
	// TODO add more constructors
	~PrototypeSet() {}
	
	/// Method to select a prototype type based on apical control
	BranchPrototype* selectNewPrototype(float lambda, float determ);
	// TODO ^ add starting orientation?

private:
	// TODO change to map structure
	std::vector<BranchPrototype*> prototypes;
};

#endif