/*
Copyright (c) 2018 RunnerScrab

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "SkillsDB.h"
#include "Skill.h"
#include "Character.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDatabase>
#include <QSqlError>
#include <QVariant>
#include <QMessageBox>
#include <QString>

#include <memory>

#include "armskillsexception.h"

SkillsDB::SkillsDB(const char * dbfilename) :
	m_dbfilename(dbfilename)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbfilename);

    if (!m_db.open())
    {
        std::string errmsg = "Failed to open db file.";
        m_db.close();
        throw ArmSkillsException(errmsg.c_str());
	}

	if (LoadDatabase() < 0)
	{
        throw ArmSkillsException("Failed to load data from database.");
	}
}

SkillsDB::~SkillsDB()
{
    m_db.close();
}

inline bool SkillsDB::IsMainGuild(const char * str)
{
	auto founditer = m_classmap.find(str);
	return founditer != m_classmap.end() && !founditer->second.m_bIsSubclass;
}

inline bool SkillsDB::IsSubGuild(const char * str)
{
	auto founditer = m_classmap.find(str);
	return founditer != m_classmap.end() && founditer->second.m_bIsSubclass;
}

std::shared_ptr<Character> SkillsDB::CreateCharacter(const char * mg, const char * sg)
{
	if (mg && !IsMainGuild(mg))
	{
		//Allow a null main guild, to display subguild data
		return nullptr;
	}
	if (sg && !IsSubGuild(sg))
	{
		//Allow a null subguild, to display main guild data
		return nullptr;
	}

    size_t mglen = mg ? strlen(mg) : 0;
    size_t sglen = sg ? strlen(sg) : 0;

	std::shared_ptr<Character> pc_out = std::make_shared<Character>(mglen ? &m_classmap[mg] : nullptr,
		sglen ? &m_classmap[sg] : nullptr);
	return pc_out;
}

int SkillsDB::LoadRows(const char * tablename, unsigned int expected_columns,
    std::function<void(QSqlQuery*)> fpRowCallback)
{
    std::string querystr = "select * from " + std::string(tablename) + ";";
    QSqlQuery query(m_db);
    query.prepare(querystr.c_str());
    if(!query.exec())
    {
        return -1;
    }

    while(query.next())
    {
        query.result();
        fpRowCallback(&query);
    }

	return 0;
}

int SkillsDB::LoadSkills(std::map<std::string, Skill>& skill_map)
{
	int result = LoadRows("skills", 2,
        [&skill_map](QSqlQuery* res)
		{
            res->value(0).toString();
            std::string name = res->value(0).toString().toStdString();
            std::string type = res->value(1).toString().toStdString();
            skill_map[name].m_name = name;
            skill_map[name].m_type = Skill::SkillStrToType(type);
		}
	);

	return result;
}

int SkillsDB::LoadPCClasses(std::map<std::string, PC_Class>& pc_class_map)
{
	int result = LoadRows("classes", 11,
        [&pc_class_map](QSqlQuery* res)
	{
        std::string class_name = res->value(0).toString().toStdString();
		PC_Class& pc_class = pc_class_map[class_name];
		pc_class.m_strName = class_name;
        pc_class.m_bIsSubclass = res->value(1).toInt();
        pc_class.m_udwTKP_cost = static_cast<unsigned int>(res->value(2).toInt());
        pc_class.m_environment = PC_Class::EnvStringToEnvType(res->value(3).toString().toStdString());
        pc_class.m_ForageFoodAbility = static_cast<PC_Class_Skill::SkillEnvironmentType>(res->value(4).toInt());
        pc_class.m_bCanCamp = res->value(5).toInt();
        pc_class.m_ExertionRecovery = PC_Class::ExertionRecStringToType(res->value(6).toString().toStdString());
        pc_class.m_bCanTame = res->value(7).toInt();
        pc_class.m_udwMaxMounts = static_cast<unsigned int>(res->value(8).toInt());
        pc_class.m_PainTolerance = PC_Class::ToleranceStringToType(res->value(9).toString().toStdString());
        pc_class.m_AlcoholTolerance = PC_Class::ToleranceStringToType(res->value(10).toString().toStdString());
	}
	);

	return result;
}

int SkillsDB::PopulatePCClassSkillList(std::map<std::string, Skill>& base_skills, PC_Class * pc_class)
{
	//HACK: oh well
	std::string tablename = "class_skills where class_name = '" + pc_class->m_strName + "'";

	std::vector<PC_Class_Skill>& sk_list = pc_class->m_class_skill_list;
	int result = LoadRows(tablename.c_str(), 5,
        [&base_skills, &sk_list](QSqlQuery* res)
	{
		sk_list.push_back(PC_Class_Skill());
        std::string skill_name = res->value(1).toString().toStdString();
        std::string max_level = res->value(2).toString().toStdString();
        const char* thirdcol = res->value(3).toString().toStdString().c_str();
		std::string branches_from = thirdcol ? thirdcol : "";
        const char* fourcol = res->value(4).toString().toStdString().c_str();
		std::string environment = fourcol ? fourcol : "";

		sk_list.back().m_baseSkill = &base_skills[skill_name];
		sk_list.back().m_maxlevel = Skill::LevelStrToEnum(max_level);
		sk_list.back().m_pBranchesFrom = "" == branches_from ? nullptr : &base_skills[branches_from];
		sk_list.back().m_environment = PC_Class_Skill::EnvStringToEnvType(environment);
	}
	);

	return result;
}

int SkillsDB::LoadDatabase()
{
	if (LoadSkills(m_skillmap) < 0)
	{
		return -1;
	}

	if (LoadPCClasses(m_classmap) < 0)
	{
		return -1;
	}

	for (auto it = m_classmap.begin(); it != m_classmap.end();
		++it)
	{
		PopulatePCClassSkillList(m_skillmap, &(it->second));
	}

	return 0;
}

