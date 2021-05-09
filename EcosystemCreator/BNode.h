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
}
using namespace HDK_Sample;

// These graph nodes surround each branch "segment"
class BNode : public std::enable_shared_from_this<BNode>
{
public:
	static float g1;
	static float g2;

	static void updateG1(float gVal);
	static void updateG2(float gVal);

	static float getG1();
	static float getG2();

	BNode();
	BNode(BNode* other);
	BNode(UT_Vector3 pos, UT_Vector3 dir, float branchAge, float length, float thick, bool isRootNode = false);
	BNode(vec3 start, vec3 end, float branchAge, float length, float thick, bool isRootNode = false);
	~BNode();

	// TODO wow the lack of const correctness. Make things safer
	std::shared_ptr<BNode> deepCopy(std::shared_ptr<BNode> par);

	// Adjusting graph structure
	void setParent(std::shared_ptr<BNode> par);
	void addChild(std::shared_ptr<BNode> child);
	void addModuleChild(SOP_Branch* child);

	// Adjust all new age-based calculations - Called from module
	void setAge(float changeInAge, std::pair<float, float>& ageRange,
		std::vector<std::shared_ptr<BNode>>& terminalNodes, bool mature, bool decay);
		//std::vector<BNode*>& terminalNodes, bool mature, bool decay);

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
	//UT_Matrix4 getLocalTransform(); /// No longer in use

	// Recursive adjustments to core variables. Important in setting up child modules
	void recThicknessUpdate(float radiusMultiplier);
	void recLengthUpdate(float lengthMultiplier);
	void recRotate(UT_Matrix3& rotation);

private:
	// Every node has up to one parent, but may have outgoing connections to
	// both nodes or modules
	std::shared_ptr<BNode> parent;
	std::vector<std::shared_ptr<BNode>> children;
	std::vector<SOP_Branch*> connectedModules;

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