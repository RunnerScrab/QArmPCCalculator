#include "armskillsexception.h"

const char* ArmSkillsException::what() const throw()
{
    return reason.c_str();
}
