#include "BranchPrototype.h"

///// INDIVIDUAL PROTOTYPES /////

/// Constructors
// TODO, for other constructors, make sure to sort the time ranges
BranchPrototype::BranchPrototype(const char* path, PlantSpeciesVariables* plantVars)
	: BranchPrototype(path, plantVars, "FoFoFoA\nA->!\"[&FoFoFoA]////[&FoFoFoA]////&FoFoFoA\no->io", 3)
{}

BranchPrototype::BranchPrototype(const char* path, PlantSpeciesVariables* plantVars, 
	const std::string & grammarProgram, int iterations)
	: agedPrototypes(), agentData()
{
	// Using default params
	LSystem lsystem = LSystem();
	lsystem.loadProgramFromString(grammarProgram);
	setFromLSystem(lsystem, iterations);
	setPlantData(plantVars);
	initAgentData(path);
}

BranchPrototype::BranchPrototype(const char* path, PlantSpeciesVariables* plantVars, 
	LSystem & lsystem, int iterations)
	: agedPrototypes(), agentData()
{
	if (!lsystem.getGrammarString().empty()) {
		setFromLSystem(lsystem, iterations);
	}
	setPlantData(plantVars);
	initAgentData(path);
}

BranchPrototype::BranchPrototype(BranchPrototype* other)
	: agedPrototypes(), agentData()
{
	for (int i = 0; i < other->getNumAges(); i++) {
		// Do a deep copy of the node structure
		std::shared_ptr<BNode> currRoot = other->getRootAtIdx(i);
		std::shared_ptr<BNode> newRoot = currRoot->deepCopy(nullptr);

		AgeGraph ag(other->getRangeAtIdx(i), newRoot);
		agedPrototypes.push_back(ag);

		// But just use the original pointers for definitions
		agentData.push_back(other->getAgentDefAtIdx(i));
	}
}

BranchPrototype* BranchPrototype::copyValues()
{
	return new BranchPrototype(this);
}

void BranchPrototype::initAgentData(const char* path)
{
	for (int i = 0; i < getNumAges(); i++) {
		// If the user did not provide their own geometry:
		agentData.push_back(PrototypeAgentPtr::createDefinition(getRootAtIdx(i), path));
	}
}

void BranchPrototype::setFromLSystem(LSystem& lsystem, int iterations) {
	for (int i = 0; i < iterations; i++) {
		AgeGraph ag(
			std::pair<float, float>(float(i), float(i + 1)),
			lsystem.process(i)
		);
		agedPrototypes.push_back(ag);
	}
}

void recSetPlantData(std::shared_ptr<BNode> currNode, PlantSpeciesVariables* plantVars) {
	currNode->setPlantVars(plantVars);

	for (std::shared_ptr<BNode> child : currNode->getChildren()) {
		recSetPlantData(child, plantVars);
	}
}

/// Run through all related nodes and pass a pointer to the PlantSpecies data
void BranchPrototype::setPlantData(PlantSpeciesVariables* plantVars) {
	for (int i = 0; i < getNumAges(); i++) {
		recSetPlantData(getRootAtIdx(i), plantVars);
	}
}

/// GETTERS
int BranchPrototype::getNumAges() {
	return agedPrototypes.size();
}

/// Returns the upper bound on the last age range
float BranchPrototype::getMaturityAge() {
	if (!agedPrototypes.empty()) {
		return agedPrototypes.back().first.second;
	}
}

/// Static helper to compare float to range
bool BranchPrototype::isInRange(std::pair<float, float>& range, float time) {
	return (time >= range.first && time < range.second);
}

/// Binary search helper for getIdxAtTimestep(...)
int BranchPrototype::searchHelper(int l, int r, float time) {
	if (r >= l) {
		int split = l + (r - l) / 2;

		std::pair<float, float>& currRange = getRangeAtIdx(split);

		if (isInRange(currRange, time)) { return split; }

		else if (time < currRange.first) { return searchHelper(l, split - 1, time); }
		else  { return searchHelper(split + 1, r, time); }
	}
	// Then time is beyond these ranges
	return getNumAges() - 1;
}

/// Get the index to value in agedPrototypes that corresponds with this time
int BranchPrototype::getIdxAtTimestep(float time)
{
	return searchHelper(0, agedPrototypes.size() - 1, time);
}

// Not rly used - TODO delete these two
/*std::pair<float, float> BranchPrototype::getRangeAtTimestep(float time)
{
	int idx = getIdxAtTimestep(time);
	return getRangeAtIdx(idx);
}

BNode* BranchPrototype::getShapeAtTimestep(float time)
{
	int idx = getIdxAtTimestep(time);
	return getRootAtIdx(idx);
}*/

/// Get corresponding prototype range at index
std::pair<float, float> BranchPrototype::getRangeAtIdx(int i)
{
	return agedPrototypes.at(i).first;
}

/// Get corresponding prototype root node at index
std::shared_ptr<BNode> BranchPrototype::getRootAtIdx(int i)
{
	return agedPrototypes.at(i).second;
}

/// Get corresponding prototype agent definition at index
GU_AgentDefinitionPtr BranchPrototype::getAgentDefAtIdx(int i)
{
	return agentData.at(i);
}




////// CONTAINER FOR A SET OF PROTOTYPES /////

// Simple default for now
PrototypeSet::PrototypeSet(const char* path, PlantSpeciesVariables* plantVars)
{
	// #1
	prototypes.push_back(new BranchPrototype(path, plantVars, "FoFoFoA\nA->!\"[&FoFoFoA]////[&FoFoFoA]////&FoFoFoA\no->io", 3));
	
	// #2
	prototypes.push_back(new BranchPrototype(path, plantVars, "FoFoAFoC\nA->/[&FoFoC]////[&FoFoC]////[&FoFoC]\nC->FoAFoC\no->io", 3));

	// #3
	prototypes.push_back(new BranchPrototype(path, plantVars, "///FoAFoFoC\nA->[&FoFoC]//[&FoFoC]//////[&FoFoC]\nC->FoAFoC\no->io", 3));
	
	// #4
	//prototypes.push_back(new BranchPrototype("FoFoFoA\nA->!\"[B]/////[B]////B\nB->&FFFFA\nC->FoFoFoFoAFoFo\no->io", 3));

	// #5
	//prototypes.push_back(new BranchPrototype("FoFoFoFoA\nA->!\"[BB]///[BB]////BB\nB->&FFFFA\nC->FoFoFoFoAFoFo\no->io", 3));

	// #6
	//prototypes.push_back(new BranchPrototype("FoFoFoFo\no->io", 3));

}

/// Method to select a prototype type based on apical control
BranchPrototype* PrototypeSet::selectNewPrototype(float lambda, float determ)
{
	// TODO select from a voronoi map. Actually based on lambda and determinancy
	// For now we are passing in values based on plant age solely instead
	float r = 1.5f / prototypes.size();
	float lowerBound = std::max(lambda - r, 0.0f);
	float upperBound = std::min(lambda + r, 1.0f);

	float idx = int((((upperBound - lowerBound) * ((float)rand() / RAND_MAX)) + lowerBound) * prototypes.size());

	//return prototypes.at(idx)->copyValues();
	return prototypes.at(0)->copyValues();
}

