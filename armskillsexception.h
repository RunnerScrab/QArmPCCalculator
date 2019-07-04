#ifndef ARMSKILLSEXCEPTION_H
#define ARMSKILLSEXCEPTION_H
#include <exception>
#include <string>

class ArmSkillsException : public std::exception
{
public:
    ArmSkillsException(const char* msg) : reason(msg)
    {

    }
    virtual const char* what() const throw() override;
    std::string reason;
};

#endif // ARMSKILLSEXCEPTION_H
