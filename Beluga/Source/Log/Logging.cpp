#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <process.h>
#include <Windows.h>

#include "Log/Logging.h"
#include "Core/Layers/ApplicationLayer.h"
using namespace Bel;

namespace Lua
{
    static int Log(lua_State* pState)
    {
        Logging* pLogging = reinterpret_cast<Logging*>(lua_touserdata(pState, 1));

        const std::string& str = static_cast<std::string>(lua_tostring(pState, 2));
        bool newLine = static_cast<bool>(lua_toboolean(pState, 3));

        pLogging->Log(Logging::SeverityLevel::kLevelDebug, str, newLine);
        lua_pop(pState, 3);

        return 0;
    }
}

Logging::Logging()
    : m_exit(false)
    , m_thread(std::thread{ &Logging::ProcessEntries, this })
{
}

Logging::~Logging()
{
    m_outFile.close();
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        // Set m_exit to true, then close thread by sending a notification to thread
        m_exit = true;

        // Sent a notification to wake threads.
        m_condVar.notify_all();
    }

    // Wait till a thread ends.
    // This is because the lock must be released before the call.
    m_thread.join();
}

bool Logging::Initialize()
{
    std::time_t t = std::time(nullptr);
    std::string formatted = CalenderToStr(t);
    formatted.append(".txt");

    m_outFile.open(formatted.c_str(), std::ios::app);

    if (m_outFile.bad())
    {
        std::cerr << "Failed to open log file." << std::endl;
        return false;
    }

    std::time_t time = std::time(nullptr);
    m_outFile << "**************************************************************\n";
    m_outFile << " Log Starts: " << CalenderToStr(time) << " " << ClockToStr(time) << "\n";
    m_outFile << "**************************************************************\n";

    // Set filter
    SetState(SeverityLevel::kLevelDebug,    "Debug");
    SetState(SeverityLevel::kLevelInfo,     "Info");
    SetState(SeverityLevel::kLevelWarn,     "Warn");
    SetState(SeverityLevel::kLevelError,    "Error");
    SetState(SeverityLevel::kLevelFatal,    "Fatal");

    return true;
}

void Logging::ProcessEntries()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    while (true)
    {
        if (!m_exit)
        {
            m_condVar.wait(lock);
        }

        // If there is a change in queue. 
        lock.unlock();
        while (true)
        {
            lock.lock();
            if (m_queue.empty())
            {
                break;
            }

            else
            {
                m_outFile << m_queue.front();
                m_queue.pop();

            }
            lock.unlock();
        }
        if (m_exit)
        {
            break;
        }
    }
}

void Logging::Log(SeverityLevel level, const std::string& message, bool newLine, const std::string& categoryName)
{
    ///*******************************************************************************************************
    /// Loging function with severity level
    ///
    /// level    :  This indicates the level of severity.
    /// pMessage :  Message to print in the log.
    /// newLine  :  When this is set to true. Leave a line feed in the log file. 
    ///*******************************************************************************************************

    static bool leaveTimeStamp = true;
    if (!IsLevelEnabled(level))
        return;

    std::time_t time = std::time(nullptr);
    const std::string timeStamp = ClockToStr(time);
    std::string str;

    if (leaveTimeStamp)
    {
        str += "[" + timeStamp + "]";

        if (!m_levels[level].m_levelStr.empty())
        {
            str += "[" + m_levels[level].m_levelStr + "]";
        }
    }

    if (categoryName != "" && IsValidCategory(m_levels[level], categoryName))
    {
        const Category* pCategory = GetCategory(m_levels[level], categoryName);

        if (pCategory->IsEnabled())
        {
            const char* temp = pCategory->GetName().data();
            str += "[";
            str += temp;
            str += "]";
        }
    }
    str += message;
    leaveTimeStamp = newLine;

    if (newLine)
    {
        str += "\n";
    }

    std::unique_lock<std::mutex> lock(m_mutex);
    m_queue.push(str);

    m_condVar.notify_all();
}

void Logging::SetState(SeverityLevel level, const char* pStr)
{
    Level& state = m_levels[level];
    state.m_levelStr = pStr;
    state.m_enum = level;
    state.m_enabled = true;

    m_categories.emplace(level, std::make_unique<Categories>());
}

bool Logging::IsLevelEnabled(SeverityLevel level)
{
    return m_levels[level].m_enabled;
}

void Logging::SetLevelEnabled(SeverityLevel level, bool enabled)
{
    m_levels[level].m_enabled = enabled;
}

void Logging::SetConfiguration(std::unordered_map<std::string, std::string> configs)
{
    ///*******************************************************************************************************
    /// Set filter's m_enabled
    ///*******************************************************************************************************
    auto strToBoolean = [](const std::string& str) -> bool
    {
        if (str == "0" || str == "false" || str == "False" || str == "FALSE")
            return false;
        return true;
    };

    for (auto& level : m_levels)
    {
        if (configs.find(level.m_levelStr) != configs.end())
        {
            SetLevelEnabled(level.m_enum, strToBoolean(configs[level.m_levelStr]));
        }
    }
}

void Logging::AddCategory(const SeverityLevel& level, const std::string& name, bool enabled)
{
    Categories* pCategory = GetCategoriesByLevel(level);
    if (!pCategory || IsValidCategory(m_levels[level], name))
        return;

    std::unique_ptr<Category> temp = std::make_unique<Category>();
    temp->SetEnabled(enabled);
    temp->SetName(name.data());
    pCategory->emplace_back(std::move(temp));
}

Logging::Categories* Logging::GetCategoriesByLevel(const SeverityLevel& level)
{
    return m_categories[level].get();
}

const Logging::LevelCategoryMap& Bel::Logging::GetAllCategories() const
{
    return m_categories;
}

bool Logging::IsValidCategory(const Level& level, const std::string& name)
{
    Categories& categories = *m_categories[level.m_enum];
    Category* pCategory = nullptr;

    for (Categories::iterator iter = categories.begin(); iter != categories.end(); ++iter)
    {
        if (iter->get()->GetName() == name)
            pCategory = iter->get();
    }

    if(!pCategory)
    {
        std::string log = level.m_levelStr;
        log += "->" + name;
        Log(SeverityLevel::kLevelWarn, "The inputted category doesn't exist: ", false);
        Log(SeverityLevel::kLevelWarn, log.data());
        return false;
    }

    return true;
}

Logging::Category* Logging::GetCategory(const Level& level, const std::string& name)
{
    Categories& categories = *m_categories[level.m_enum];
    Category* pCategory = nullptr;
    
    for (Categories::iterator iter = categories.begin(); iter != categories.end(); ++iter)
    {
        if (iter->get()->GetName() == name)
            pCategory = iter->get();
    }

    if (!pCategory)
    {
        std::string log = level.m_levelStr;
        log += "->" + name;
        Log(SeverityLevel::kLevelWarn, "The inputted category doesn't exist: ", false);
        Log(SeverityLevel::kLevelWarn, log.data());
        return nullptr;
    }

    return pCategory;
}

void Logging::SetCategoryEnabled(const SeverityLevel& level, const std::string& name, bool enabled)
{
    Categories* pCategories = GetCategoriesByLevel(level);
    if (!pCategories || !IsValidCategory(m_levels[level], name))
        return;

    Category* pCategory = GetCategory(m_levels[level], name.data());
    pCategory->SetEnabled(enabled);
}

void Logging::RegisterWithScript()
{
    auto& scripting = ApplicationLayer::GetInstance()->GetGameLayer()->GetScriptingManager();
    scripting.GetGlobal("g_logic");

    scripting.CreateTable();
    scripting.AddToTable("this", this);
    scripting.AddToTable("Log", Lua::Log);
    scripting.AddToTable("Logging");

    scripting.PopAll();
}

const std::string Logging::CalenderToStr(const time_t& time)
{
    std::tm buf;
    localtime_s(&buf, &time);
    std::stringstream wss;
    wss << std::put_time(&buf, "%d-%b-%Y");

    return wss.str();
}

const std::string Logging::ClockToStr(const time_t& time)
{
    std::tm buf;
    localtime_s(&buf, &time);
    std::stringstream wss;
    wss << std::put_time(&buf, "%X");

    return wss.str();
}