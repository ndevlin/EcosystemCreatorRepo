#include "BNode.h"
#include "SOP_Branch.h"
//using namespace HDK_Sample;

float BNode::g1 = 1.0;
float BNode::g2 = -0.2;

void BNode::updateG1(float gVal) {
	g1 = gVal;
}
void BNode::updateG2(float gVal) {
	g2 = gVal;
}

float BNode::getG1() {
	return g1;
}
float BNode::getG2() {
	return g2;
}

/// Constructors
BNode::BNode() 
	: position(0.0), age(0.0), maxLength(3.0), thickness(0.1), 
		parent(nullptr), children(), connectedModules(), rigIndex(-1)
{}

BNode::BNode(BNode* other)
	: position(other->getPos()), unitDir(other->getDir()), age(other->getAge()), 
		maxLength(other->getMaxLength()), thickness(other->getBaseRadius()), 
		baseRadius(other->getBaseRadius()), rigIndex(other->getRigIndex()),
		parent(nullptr), children(), connectedModules(), root(other->isRoot())
	//: BNode(other->getPos(), other->getDir(), other->getAge(),
	//	other->getMaxLength(), other->getBaseRadius())
{
	//rigIndex = other->getRigIndex();
}

BNode::BNode(UT_Vector3 pos, UT_Vector3 dir, float branchAge, float length, float thick, bool isRootNode)
	: position(pos), unitDir(dir), age(branchAge), maxLength(length),
		thickness(thick), baseRadius(thick), parent(nullptr), children(), 
		connectedModules(), rigIndex(-1), root(isRootNode)
{
	unitDir.normalize();
}

BNode::BNode(vec3 start, vec3 end, float branchAge, float length, float thick, bool isRootNode)
	: age(branchAge), maxLength(length), thickness(thick), baseRadius(thick),
		parent(nullptr), children(), connectedModules(), rigIndex(-1), root(isRootNode)
{
	position = UT_Vector3();
	position(0) = end[0];
	position(1) = end[1];
	position(2) = end[2];

	unitDir = UT_Vector3();
	unitDir(0) = end[0] - start[0];
	unitDir(1) = end[1] - start[1];
	unitDir(2) = end[2] - start[2];
	if (abs(unitDir.length2()) < 0.000001f) {
		unitDir(0) = 0.f;
		unitDir(1) = 1.f;
		unitDir(2) = 0.f;
	}
	unitDir.normalize();
}

BNode::~BNode() {
	if (this->parent != nullptr) {
		// TODO remove this from parent list prob
	}
	for (std::shared_ptr<BNode> child : children) {
		child->setParent(nullptr);
		//delete child;
	}
	for (SOP_Branch* connectedModule : connectedModules) {
		connectedModule->destroySelf();
		// TODO remove from parent list too - currently happens in SOP_Branch::setAge
	}
}

std::shared_ptr<BNode> BNode::deepCopy(std::shared_ptr<BNode> par) {
	std::shared_ptr<BNode> newNode(new BNode(this));
	//std::shared_ptr<BNode> newNode = std::make_shared<BNode>(this); // Uses custom copy constructor? TODO check
	newNode->setParent(par);

	for (std::shared_ptr<BNode> child : children) {
		std::shared_ptr<BNode> copyChild = child->deepCopy(newNode);
		newNode->addChild(copyChild);
	}
	return newNode;
}

/// SETTERS
void BNode::setParent(std::shared_ptr<BNode> par)
{
	parent = par;
}

void BNode::addChild(std::shared_ptr<BNode> child)
{
	children.push_back(child);
}

void BNode::addModuleChild(SOP_Branch* child) {
	connectedModules.push_back(child);
}

// Adjust all new age-based calculations
void BNode::setAge(float changeInAge, std::pair<float, float>& ageRange, 
	std::vector<std::shared_ptr<BNode>>& terminalNodes, bool mature, bool decay) {
	age += changeInAge;

	// For roots of child modules
	if (parent && isRoot()) { position = parent->getPos(); }
	
	// For full branch-segments only, update length and position:
	else if (parent) {
		// TODO make this a more smooth curve, slow down over time
		float branchLength = min(maxLength, age * 0.3f);
		/*float branchLength = (age * 0.1f) / maxLength / 2.0f + 0.5f;
		branchLength = branchLength * branchLength * (3 - 2 * branchLength);
		branchLength = (max(min(branchLength, 1.0f), 0.5f) - 0.5f) * 2.0f * maxLength;*/

		position = parent->getPos() + branchLength * unitDir;

		// Calculate tropism offset using  static values
		// TODO, just get a pointer to plant in BNode so that this isnt the same for all plants
		float g1Val = pow(0.95f, age * BNode::getG1());   // Controls tropism decrease over time
		float g2Val = -BNode::getG2();                    // Controls tropism strength
		UT_Vector3 gDir = UT_Vector3(0.0f, -1.0f, 0.0f);  // Gravity Direction

		UT_Vector3 tOffset = (g1Val * g2Val * gDir) / max(age + g1Val, 0.05f);

		position += tOffset * branchLength; // scaled it for the effect to be proportionate
	}

	// Branch thickness update:
	thickness = max(0.015f, age * baseRadius * 0.4f);
	// There's an age difference of 1 between terminal nodes and their children
	// This is how I've decided to deal with it for now
	if (parent && isRoot()) { thickness = parent->getThickness(); }
	// TODO maybe only if this is the first of the terminal's childModule array

	// Update children
	for (std::shared_ptr<BNode> child : children) {
		child->setAge(changeInAge, ageRange, terminalNodes, mature, decay);
	}

	if (mature && children.empty() /*TODO allow for multiple*/ && connectedModules.empty()) {
		// TODO base addition on vigor
		terminalNodes.push_back(shared_from_this());
	}
	// Clear/cull modules if it is not mature (rewinding of time) or TODO if vigor drops too low
	else if (decay && !connectedModules.empty()) {
		for (SOP_Branch* connectedMod : connectedModules) {
			connectedMod->destroySelf();
		}
		connectedModules.clear();
		// TODO The parent array is cleared in SOP_Branch::setAge(). Maybe move that here
	}
}

/// GETTERS
bool BNode::isRoot() const {
	return root;
}

std::shared_ptr<BNode> BNode::getParent() {
	return parent;
}

std::vector<std::shared_ptr<BNode>>& BNode::getChildren() {
	return children;
}

UT_Vector3 BNode::getPos()
{
	return position;
}

UT_Vector3 BNode::getDir()
{
	return unitDir;
}

float BNode::getAge()
{
	return age;
}

float BNode::getMaxLength()
{
	return maxLength;
}

float BNode::getThickness()
{
	return thickness;
}

float BNode::getBaseRadius()
{
	return baseRadius;
}

int BNode::getRigIndex()
{
	return rigIndex;
}

void BNode::setRigIndex(int idx)
{
	rigIndex = idx;
}


/* BNode::getWorldTransform() {
	if (isRoot() || !parent) {
		return getLocalTransform();
	}
	return getLocalTransform() * parent->getWorldTransform();
}*/

UT_Matrix4 BNode::getWorldTransform() {
	UT_Matrix4 transform = UT_Matrix4(1.0f);
	UT_Vector3 c = UT_Vector3();

	if (!parent) {
		transform = UT_Matrix4(UT_Matrix3::dihedral(UT_Vector3(0.0f, 1.0f, 0.0f),
			getDir(), c, 1));
	}

	else if (isRoot()) {
		// Getting the angle of the parent branch segment
		UT_Vector3 parentDir;
		if (!parent->isRoot() && parent->getParent()) {
			parentDir = parent->getPos() - parent->getParent()->getPos();
		}
		else if (parent->isRoot() && parent->getParent() && parent->getParent()->getParent()) {
			// Skipping the terminal node since it's located in the same place as the root node
			parentDir = parent->getPos() - parent->getParent()->getParent()->getPos();
		}
		else { parentDir = parent->getDir(); }
		//parentDir.normalize();

		transform = UT_Matrix4(UT_Matrix3::dihedral(UT_Vector3(0.0f, 1.0f, 0.0f),
			parentDir, c, 1));
		// TODO append own Dir
	}

	else {
		//if (parent->getParent()) { parentDir = parent->getPos() - parent->getParent()->getPos(); }
		//else					 { parentDir = parent->getDir(); }

		UT_Vector3 currDir = position - parent->getPos();
		transform = UT_Matrix4(UT_Matrix3::dihedral(UT_Vector3(0.0f, 1.0f, 0.0f),
			currDir, c, 1));
	}
	transform.setTranslates(position);
	return transform;
}

/*UT_Matrix4 BNode::getLocalTransform() {
	UT_Matrix4 translate = UT_Matrix4(1.0f);
	UT_Vector3 c = UT_Vector3();

	if (!parent) {
		translate = UT_Matrix4(UT_Matrix3::dihedral(UT_Vector3(0.0f, 1.0f, 0.0f), 
			getDir(), c, 1));
		translate.setTranslates(position);
		return translate;
	}

	// Getting the angle of the parent branch segment
	UT_Vector3 parentDir;
	if (!parent->isRoot() && parent->getParent()) { 
		parentDir = parent->getPos() - parent->getParent()->getPos(); 
	}
	else if (parent->isRoot() && parent->getParent() && parent->getParent()->getParent()) {
		// Skipping the terminal node since it's located in the same place as the root node
		parentDir = parent->getPos() - parent->getParent()->getParent()->getPos();
	}
	else { parentDir = parent->getDir(); }
	//parentDir.normalize();

	if (isRoot()) {
		translate = UT_Matrix4(UT_Matrix3::dihedral(UT_Vector3(0.0f, 1.0f, 0.0f), 
			parentDir, c, 1));
		// TODO append own Dir
		translate.setTranslates(position);
		return translate;
	}

	// Getting the angle of the current branch segment
	UT_Vector3 currDir = position - parent->getPos();
	translate.setTranslates(UT_Vector3(0.0f, currDir.length(), 0.0f));
	//currDir.normalize();

	UT_Matrix3 orientation3;// = UT_Matrix3::dihedral(parentDir, currDir, c, 1);
	
	float angle = parentDir.angleTo(currDir);
	UT_Vector3 axisOfRot = cross(parentDir, currDir);
	// TODO check clockwise
	axisOfRot.normalize();
	UT_Quaternion quatRot = UT_Quaternion(angle, axisOfRot);
	quatRot.getRotationMatrix(orientation3);

	UT_Matrix4 transform = UT_Matrix4(orientation3);
	transform.preMultiply(translate);
	//transform *= translate;

	return transform;
}*/

/// More forms of updating
void BNode::recThicknessUpdate(float radiusMultiplier) {
	baseRadius *= radiusMultiplier;
	for (std::shared_ptr<BNode> child : children) { child->recThicknessUpdate(radiusMultiplier); }
}

// experimental
void BNode::recLengthUpdate(float lengthMultiplier) {
	maxLength *= lengthMultiplier;
	for (std::shared_ptr<BNode> child : children) { child->recLengthUpdate(lengthMultiplier); }
}

// experimental #2
void BNode::recRotate(UT_Matrix3& rotation) {
	unitDir = rowVecMult(unitDir, rotation);
	//unitDir = colVecMult(rotation, unitDir);
	for (std::shared_ptr<BNode> child : children) { child->recRotate(rotation); }
}