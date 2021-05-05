#include "BranchPrototype.h"

///// INDIVIDUAL PROTOTYPES /////

float randomness = 0.5;

/// Constructors
// TODO, for other constructors, make sure to sort the time ranges
BranchPrototype::BranchPrototype(const char* path)
	: BranchPrototype("FoFoFoA\nA->!\"[&FoFoFoA]////[&FoFoFoA]////&FoFoFoA\no->io", 3, path)
{}

BranchPrototype::BranchPrototype(const std::string & grammarProgram, int iterations, const char* path)
	: agedPrototypes(), agentData()
{
	// Using default params
	LSystem lsystem = LSystem();
	lsystem.loadProgramFromString(grammarProgram);
	setFromLSystem(lsystem, iterations);
	initAgentData(path);
}

BranchPrototype::BranchPrototype(LSystem & lsystem, int iterations, const char* path)
	: agedPrototypes(), agentData()
{
	if (!lsystem.getGrammarString().empty()) {
		setFromLSystem(lsystem, iterations);
	}
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

void BranchPrototype::setRandomness(float randIn)
{
	randomness = randIn;
}

void BranchPrototype::initAgentData(const char* path)
{
	for (int i = 0; i < getNumAges(); i++) {
		// If the user did not provide their own geometry:
		std::cout << "Agent Type: " + std::to_string(i) << std::endl;
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

/// Get corresponding prototype geometry at index
/*GU_PrimPacked* BranchPrototype::getGeomAtIdx(int i)
{
	return agentData.at(i).first;
}*/

/// Get corresponding prototype agent definition at index
GU_AgentDefinitionPtr BranchPrototype::getAgentDefAtIdx(int i)
{
	//return agentData.at(i).second;
	return agentData.at(i);
}




////// CONTAINER FOR A SET OF PROTOTYPES /////

// Simple default for now
PrototypeSet::PrototypeSet(const char* path)
{
	// #1
	prototypes.push_back(new BranchPrototype("FoFoFoA\nA->!\"[&FoFoFoA]////[&FoFoFoA]////&FoFoFoA\no->io", 3, path));
	
	// #2
	prototypes.push_back(new BranchPrototype("FoFoAFoC\nA->/[&FoFoC]////[&FoFoC]////[&FoFoC]\nC->FoAFoC\no->io", 3, path));

	// #3
	prototypes.push_back(new BranchPrototype("///FoAFoFoC\nA->[&FoFoC]//[&FoFoC]//////[&FoFoC]\nC->FoAFoC\no->io", 3, path));
	
	// #4
	prototypes.push_back(new BranchPrototype("FoFoFoA\nA->!\"[B]/////[B]////B\nB->&FFFFA\nC->FoFoFoFoAFoFo\no->io", 3, path));

	// #5
	//prototypes.push_back(new BranchPrototype("FoFoFoFoA\nA->!\"[BB]///[BB]////BB\nB->&FFFFA\nC->FoFoFoFoAFoFo\no->io", 3, path));

	// #6
	//prototypes.push_back(new BranchPrototype("FoFoFoFo\no->io", 3, path));

}

/// Method to select a prototype type based on apical control
BranchPrototype* PrototypeSet::selectNewPrototype(float lambda, float determ, float rainfall, float temperature)
{
	// TODO select from a voronoi map. Actually based on lambda and determinancy
	// For now we are passing in values based on plant age solely instead

	/*
	float r = 1.5f / prototypes.size();
	float lowerBound = std::max(lambda - r, 0.0f);
	float upperBound = std::min(lambda + r, 1.0f);

	int idx = int((((upperBound - lowerBound) * ((float)rand() / RAND_MAX) - 0.00001) + lowerBound) * prototypes.size());
	*/
	
	int idx = int((((rainfall + temperature) / 2.f) - 0.00001f) * prototypes.size());

	std::cout << "In selectNewPrototype() " << std::endl;

	std::cout << "idx: " << idx << std::endl;

	std::cout << "rainfall: " << rainfall << std::endl;

	std::cout << "temperature: " << temperature << std::endl;



	return prototypes.at(idx)->copyValues();
	//return prototypes.at(0)->copyValues();
}

