#ifndef LSystem_H_
#define LSystem_H_

#include <BNode.h>

class LSystem
{
public:
    LSystem();
    ~LSystem() {}

    // Set/get inputs
    void loadProgram(const std::string& fileName); /// probably delete this
    void loadProgramFromString(const std::string& program);
    void setDefaultAngle(float degrees);
    void setDefaultStep(float distance);

    float getDefaultAngle() const;
    float getDefaultStep() const;
    const std::string& getGrammarString() const;

    // Iterate grammar
    const std::string& getIteration(unsigned int n);

    // Get nodes from running the turtle
	std::shared_ptr<BNode> process(unsigned int n);

protected:
    void reset();
    void addProduction(std::string line);
    std::string iterate(const std::string& input);
    
    std::map<std::string, std::string> productions;
    std::vector<std::string> iterations;
    std::string current;
    float mDfltAngle;
    float mDfltStep;
    std::string mGrammar;

    class Turtle
    {
    public:
        Turtle();
        Turtle(const Turtle& t);
        Turtle& operator=(const Turtle& t);

		void swapNode(std::shared_ptr<BNode> newSegment);
        void moveForward(float distance);
        void applyUpRot(float degrees);
        void applyLeftRot(float degrees);
        void applyForwardRot(float degrees);

        vec3 pos;
        vec3 up;
        vec3 forward;
        vec3 left;
		std::shared_ptr<BNode> currSegment;
    };
};

#endif