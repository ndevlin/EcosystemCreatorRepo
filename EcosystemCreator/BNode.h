#ifndef BNode_H_
#define BNode_H_
#include <memory>
#include <string>
#include <vector>
#include <map>
#include "vec.h"
#include <SOP/SOP_Node.h>

namespace HDK_Sample {
	class SOP_Branch;
	class PlantSpeciesVariables;
}
using namespace HDK_Sample;

// These graph nodes surround each branch "segment"
class BNode : public std::enable_shared_from_this<BNode>
{
public:
	BNode();
	BNode(BNode* other);
	BNode(UT_Vector3 pos, UT_Vector3 dir, float branchAge, float length, float thick, bool isRootNode = false);
	BNode(vec3 start, vec3 end, float branchAge, float length, float thick, bool isRootNode = false);
	~BNode();

	std::shared_ptr<BNode> deepCopy(std::shared_ptr<BNode> par);

	// Adjusting graph structure
	void setParent(std::shared_ptr<BNode> par);
	void addChild(std::shared_ptr<BNode> child);
	void addModuleChild(SOP_Branch* child);

	// Adjust all new age-based calculations - Called from module
	void setAge(float changeInAge, 
		std::vector<std::shared_ptr<BNode>>& terminalNodes, bool mature, bool decay);

	// Run through all related nodes and pass a pointer to the PlantSpecies data
	void setPlantVars(PlantSpeciesVariables* vars);

	// Getters for important variables
	std::shared_ptr<BNode> getParent();
	std::vector<std::shared_ptr<BNode>>& getChildren();

	bool isRoot() const;

	UT_Vector3 getPos();
	UT_Vector3 getDir();

	float getAge();
	float getMaxLength();

	float getThickness(); 
	float getBaseRadius();

	int getRigIndex();
	void setRigIndex(int idx);

	UT_Matrix4 getWorldTransform();

	// Recursive adjustments to core variables. Important in setting up child modules
	void recTransformation(float ageDif, float radiusMultiplier, 
		float lengthMultiplier, UT_Matrix3& rotation);

protected:
	PlantSpeciesVariables* getPlantVars();

private:
	// Every node has up to one parent, but may have outgoing connections to
	// both nodes or modules
	std::shared_ptr<BNode> parent;
	std::vector<std::shared_ptr<BNode>> children;
	std::vector<SOP_Branch*> connectedModules;

	PlantSpeciesVariables* plantVars;

	UT_Vector3 position;
	UT_Vector3 unitDir;

	float age;
	float maxLength;

	float thickness;
	float baseRadius;

	int rigIndex;

	bool root;
};
#endif

