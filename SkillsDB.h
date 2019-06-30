#ifndef SKILLS_DB_H
#define SKILLS_DB_H

#include <string>
#include <functional>
#include <map>
#include <memory>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

#include "Skill.h"
#include "PC_Class.h"

class Character;

class SkillsDB
{
public:
	SkillsDB(const char* dbfilename = "armskills.db");
	~SkillsDB();

	bool IsMainGuild(const char* str);
	bool IsSubGuild(const char* str);

	std::map<std::string, PC_Class>& GetClassMap()
	{
		return m_classmap;
	}

	std::shared_ptr<Character> CreateCharacter(const char* mg, const char* sg);

private:
	int LoadRows(const char * tablename, unsigned int expected_columns,
        std::function<void(QSqlQuery *)> fpRowCallback);
	int LoadSkills(std::map<std::string, Skill>& skill_map);
	int LoadPCClasses(std::map<std::string, PC_Class>& pc_class_map);
	int PopulatePCClassSkillList(std::map<std::string, Skill>& base_skills, PC_Class * pc_class);
	int LoadDatabase();

	std::string m_dbfilename;
    QSqlDatabase m_db;
	std::map<std::string, Skill> m_skillmap;
	std::map<std::string, PC_Class> m_classmap;
};

#endif
