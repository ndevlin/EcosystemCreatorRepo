#include "LSystem.h"    
#include <fstream>
#include <stack>

#pragma warning(disable : 4244)
#pragma warning(disable : 4290)
#include "matrix.h"

#define Rad2Deg 57.295779513082320876798154814105
#define Deg2Rad 0.017453292519943295769236907684886

LSystem::LSystem() : mDfltAngle(33.0), mDfltStep(1.0) {}

/// SETTERS
void LSystem::setDefaultAngle(float degrees)
{
    mDfltAngle = degrees;
}

void LSystem::setDefaultStep(float distance)
{
    mDfltStep = distance;
}

/// GETTERS
float LSystem::getDefaultAngle() const
{
    return mDfltAngle;
}

float LSystem::getDefaultStep() const
{
    return mDfltStep;
}

const std::string& LSystem::getGrammarString() const
{
    return mGrammar;
}

void LSystem::reset()
{
    current = "";
    iterations.clear();
    productions.clear();
}

const std::string& LSystem::getIteration(unsigned int n)
{
    if (n >= iterations.size())
    {
        for (unsigned int i = iterations.size(); i <= n; i++)
        {
            current = iterate(current);
            iterations.push_back(current);
        }        
    }
    return iterations[n];
}

/// Initiate L-System
void LSystem::loadProgram(const std::string& fileName)
{
    reset();

    std::string line;
    std::ifstream file(fileName.c_str());
    if (file.is_open())
    {
        while (file.good())
        {
            getline(file,line);
            addProduction(line);
        }
    }
    // for each line in p, add production
    file.close();
}

void LSystem::loadProgramFromString(const std::string& program)
{
    reset(); 
    mGrammar = program;

    size_t index = 0;
    while (index < program.size())
    {
        size_t nextIndex = program.find("\n", index);
        std::string line = program.substr(index, nextIndex - index);
        addProduction(line);
        if (nextIndex == std::string::npos) break;
        index = nextIndex+1;
    }
}

void LSystem::addProduction(std::string line)
{
    size_t index;

    // 1. Strip whitespace
    while ((index = line.find(" ")) != std::string::npos)
    {
        line.replace(index, 1, ""); 
    }

    if (line.size() == 0) return;

    // 2. Split productions
    index = line.find("->");
    if (index != std::string::npos)
    {
        std::string symFrom = line.substr(0, index);
        std::string symTo = line.substr(index+2);
        productions[symFrom] = symTo;
    }
    else  // assume its the start sym
    {
        current = line;
		// Changed to include base string too
		if (current.find('F') != std::string::npos) {
			iterations.push_back(current);  // TODO add option to control this
		}
    }
	// TODO, maybe add an additional hidden rule of F->Fi that gets processed after each iter?
	// so that that extension part is more subtle. or make chars pairs
}

std::string LSystem::iterate(const std::string& input)
{
    std::string output = "";
    for (unsigned int i = 0; i < input.size(); i++)
    {
        std::string sym = input.substr(i,1);
        std::string next = productions.count(sym) > 0? productions[sym] : sym; 
        output = output + next;
    }
    return output;
    //for each sym in current state, replace the sym
}


/// TURTLE
LSystem::Turtle::Turtle() :
    pos(0,0,0),
    up(0,0,1),
    forward(1,0,0),
    left(0,1,0),
	currSegment(nullptr)
{
}

LSystem::Turtle::Turtle(const LSystem::Turtle& t)
{
    pos = t.pos;
    up = t.up;
    forward = t.forward;
    left = t.left;
	currSegment = t.currSegment;
}

LSystem::Turtle& LSystem::Turtle::operator=(const LSystem::Turtle& t)
{
    if (&t == this) return *this;

    pos = t.pos;
    up = t.up;
    forward = t.forward;
    left = t.left;
	currSegment = t.currSegment;
    return *this;
}

void LSystem::Turtle::swapNode(BNode* newSegment) {
	currSegment->addChild(newSegment);
	currSegment = newSegment;
}

void LSystem::Turtle::moveForward(float length)
{
    pos = pos + length * forward;
}

void LSystem::Turtle::applyUpRot(float degrees)
{
    math::RotationMatrix<float> mat(2,Deg2Rad*degrees); // Z axis
    math::RotationMatrix<float> world2local(forward, left, up); 
    up =  world2local * mat * vec3(0,0,1);
    left = world2local * mat * vec3(0,1,0);
    forward = world2local * mat * vec3(1,0,0);
}

void LSystem::Turtle::applyLeftRot(float degrees)
{
    math::RotationMatrix<float> mat(1,Deg2Rad*degrees); // Y axis
    math::RotationMatrix<float> world2local(forward, left, up); 
    up =  world2local * mat * vec3(0,0,1);
    left = world2local * mat * vec3(0,1,0);
    forward = world2local * mat * vec3(1,0,0);
}

void LSystem::Turtle::applyForwardRot(float degrees)
{
    math::RotationMatrix<float> mat(0,Deg2Rad*degrees); // X axis
    math::RotationMatrix<float> world2local(forward, left, up); 
    up =  world2local * mat * vec3(0,0,1);
    left = world2local * mat * vec3(0,1,0);
    forward = world2local * mat * vec3(1,0,0);
}
/// END TURTLE



BNode* LSystem::process(unsigned int n)
{
    Turtle turtle;
    std::stack<Turtle> stack;
	// TODO input origin point, TODO check n or n - 1, thick
	BNode* rootNode = new BNode(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), n, 0.0f, 0.1f);
	turtle.currSegment = rootNode;

    // Init so we're pointing up
	turtle.applyLeftRot(-90);
	turtle.applyUpRot(90);

    std::string insn = getIteration(n);
    for (unsigned int i = 0; i < insn.size(); i++)
    {
        std::string sym = insn.substr(i,1);
        if (sym == "F")
        {
            vec3 start = turtle.pos;
            turtle.moveForward(mDfltStep);

			// Add "o->io" as a rule for objects whose iteration depth you want to track
			float iterationAge = 0.0f;
			sym = insn.substr(i + 1, 1);
			// Increases the age per number of iterations o has been through
			while (sym == "i") {
				iterationAge++;
				i++;
				sym = insn.substr(i + 1, 1);
			}

			BNode* newSegment = new BNode(start, turtle.pos, iterationAge,
				mDfltStep, turtle.currSegment->getBaseRadius() * 0.9);
			turtle.swapNode(newSegment);
        }
        else if (sym == "+")
        {
            turtle.applyUpRot(mDfltAngle);
        }
        else if (sym == "-")
        {
            turtle.applyUpRot(-mDfltAngle);
        }
        else if (sym == "&")
        {
            turtle.applyLeftRot(mDfltAngle);
        }
        else if (sym == "^")
        {
            turtle.applyLeftRot(-mDfltAngle);
        }
        else if (sym == "\\")
        {
            turtle.applyForwardRot(mDfltAngle);
        }
        else if (sym == "/")
        {
            turtle.applyForwardRot(-mDfltAngle);
        }
        else if (sym == "|")
        {
            turtle.applyUpRot(180);
        }
        else if (sym == "[")
        {
            stack.push(turtle);;
        }
        else if (sym == "]")
        {
            turtle = stack.top();
            stack.pop();
        }
        else
        {
            // Maybe add more functionality
        }
    }
	return rootNode;
}