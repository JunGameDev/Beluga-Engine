#pragma once

#include <process.h>
#include <mutex>
#include <fstream>
#include <ctime>
#include <functional>
#include <queue>
#include <map>
#include <unordered_map>
#include <array>
#include <sstream>
#include <thread>

namespace Bel
{
    class Logging
    {
    public:
        // Enum class to show severity level.
        enum SeverityLevel : uint32_t
        {
            kLevelDebug,    // Debugging Level
            kLevelInfo,     // Inform Level 
            kLevelWarn,     // Warning level
            kLevelError,    // Error Level
            kLevelFatal,    // Fatal level
            kLevelNone,
            kCount
        };

        // A nested struct to filter severity level.
        struct Level
        {
            std::string     m_levelStr;
            SeverityLevel   m_enum;
            bool            m_enabled;
        };

        class Category
        {
        private:
            std::string     m_name;
            bool            m_enabled;

        public:
            Category()
                : m_name("")
                , m_enabled(false)
            {
            }
            Category(const Category& src) = default;
            Category& operator=(const Category & rhs) = default;

            const std::string& GetName() const { return m_name; }
            bool IsEnabled() const { return m_enabled; }

            void SetName(const char* pName) { m_name = pName; }
            void SetEnabled(bool value) { m_enabled = value; }
        };

        using Categories            = std::vector<std::unique_ptr<Category>>;
        using LevelCategoryMap      = std::unordered_map<SeverityLevel, std::unique_ptr<Categories>>;

    private:
        // --- Out put ---
        std::ofstream m_outFile;
        
        // --- Level ---
        std::array<Level, Logging::SeverityLevel::kCount> m_levels;

        // --- Multi-threading ---
        std::mutex                  m_mutex;
        std::condition_variable     m_condVar;
        std::queue<std::string>     m_queue;
        std::thread                 m_thread;
        bool                        m_exit;

        // --- Category ---
        LevelCategoryMap m_categories;

    public:
        Logging();
        Logging(const Logging& src) = delete;
        Logging& operator=(const Logging& rhs) = delete;
        ~Logging();

        bool Initialize();

        // --- Public Logging functions ---
        void Log(SeverityLevel level, const std::string& message, bool newLine = true, const std::string& categoryName = "");

        // --- Severity ---
        void SetState(SeverityLevel level, const char* pStr);
        bool IsLevelEnabled(SeverityLevel level);
        void SetLevelEnabled(SeverityLevel level, bool enabled);
        void SetConfiguration(std::unordered_map<std::string, std::string> configs);

        // --- Category ---
        void AddCategory(const SeverityLevel& level, const std::string& name, bool enabled = true);
        Categories* GetCategoriesByLevel(const SeverityLevel& level);
        const LevelCategoryMap& GetAllCategories() const;
        void SetCategoryEnabled(const SeverityLevel& level, const std::string& name, bool enabled);

        void RegisterWithScript();

    private:
        // Function that will be running at background thread.
        void ProcessEntries();
        
        // --- Time stamp ---
        const std::string CalenderToStr(const time_t& time);
        const std::string ClockToStr(const time_t& time);

        // --- Category ---
        bool IsValidCategory(const Level& level, const std::string& name);
        Category* GetCategory(const Level& level, const std::string& pName);
    };
}
