#ifndef BNode_H_
#define BNode_H_
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
class BNode
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
	BNode(UT_Vector3 pos, UT_Vector3 dir, float branchAge, float length, float thick);
	BNode(vec3 start, vec3 end, float branchAge, float length, float thick);
	~BNode();

	// TODO wow the lack of const correctness. Make things safer
	BNode* deepCopy(BNode* par);

	// Adjusting graph structure
	void setParent(BNode* par);
	void addChild(BNode* child);
	void addModuleChild(SOP_Branch* child);

	// Adjust all new age-based calculations - Called from module
	void setAge(float changeInAge, std::pair<float, float>& ageRange,
		std::vector<BNode*>& terminalNodes, bool mature, bool decay);

	// Getters for important variables
	BNode* getParent();
	std::vector<BNode*>& getChildren();

	UT_Vector3 getPos();
	UT_Vector3 getDir();

	float getAge();
	float getMaxLength();

	float getThickness(); 
	float getBaseRadius();

	// Recursive adjustments to core variables. Important in setting up child modules
	void recThicknessUpdate(float radiusMultiplier);
	void recLengthUpdate(float lengthMultiplier);
	void recRotate(UT_Matrix3& rotation);

private:
	// Every node has up to one parent, but may have outgoing connections to
	// both nodes or modules
	BNode* parent;
	std::vector<BNode*> children;
	std::vector<SOP_Branch*> connectedModules;

	UT_Vector3 position;
	UT_Vector3 unitDir;

	float age;
	float maxLength;

	float thickness;
	float baseRadius;
};
#endif