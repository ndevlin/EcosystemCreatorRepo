#ifndef __PLANT_TYPE_h__
#define __PLANT_TYPE_h__

#include <BranchPrototype.h>

class PlantType
{
public:
	PlantType(const char* path);
	~PlantType();

	/// Copy's a prototype instance, used as a base for a new branch module
	BranchPrototype*         copyPrototypeFromList(float lambda, float determ);

private:
	PrototypeSet* prototypeSet; // Make unique pointer
};

#endif