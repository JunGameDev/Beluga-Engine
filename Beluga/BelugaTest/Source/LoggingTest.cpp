#include "CppUnitTest.h"
#include <Log/Logging.h>
#include <iostream>
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bel;

namespace BelugaTest
{
    TEST_CLASS(LoggingTest)
    {
    public:
        using SeverityLevel = Logging::SeverityLevel;

    public:
        TEST_METHOD(Create)
        {
            Logging logging;
            Assert::IsNotNull(&logging);
        }

        TEST_METHOD(Initialize)
        {
            Logging logging;
            Assert::IsTrue(logging.Initialize());
        }

        TEST_METHOD(IsLevelEnabled)
        {
            Logging logging;
            logging.Initialize();

            Assert::IsTrue(logging.IsLevelEnabled(SeverityLevel::kLevelDebug));
            Assert::IsTrue(logging.IsLevelEnabled(SeverityLevel::kLevelError));
            Assert::IsTrue(logging.IsLevelEnabled(SeverityLevel::kLevelFatal));
            Assert::IsTrue(logging.IsLevelEnabled(SeverityLevel::kLevelInfo));
            Assert::IsTrue(logging.IsLevelEnabled(SeverityLevel::kLevelWarn));
        }

        TEST_METHOD(AddCategory)
        {
            Logging logging;
            logging.Initialize();

            const Logging::LevelCategoryMap& categories = logging.GetAllCategories();
            Assert::AreEqual(categories.size(), static_cast<size_t>(5));
            Assert::IsFalse(categories.empty());

            logging.AddCategory(Logging::SeverityLevel::kLevelDebug, "Test_Category1");
            logging.AddCategory(Logging::SeverityLevel::kLevelDebug, "Test_Category2");
            logging.AddCategory(Logging::SeverityLevel::kLevelDebug, "Test_Category3");
            logging.AddCategory(Logging::SeverityLevel::kLevelDebug, "Test_Category4");
            logging.AddCategory(Logging::SeverityLevel::kLevelDebug, "Test_Category5");
            logging.AddCategory(Logging::SeverityLevel::kLevelDebug, "Test_Category6");
            logging.AddCategory(Logging::SeverityLevel::kLevelDebug, "Test_Category7");

            Assert::AreEqual(categories.at(SeverityLevel::kLevelDebug)->size(), static_cast<size_t>(7));
        }

        TEST_METHOD(SetCategoryEnalbed)
        {
            Logging logging;
            logging.Initialize();

            logging.AddCategory(SeverityLevel::kLevelDebug, "Test_Category1");
            logging.AddCategory(SeverityLevel::kLevelDebug, "Test_Category2");
            logging.AddCategory(SeverityLevel::kLevelDebug, "Test_Category3");
            logging.AddCategory(SeverityLevel::kLevelDebug, "Test_Category4");
            logging.AddCategory(SeverityLevel::kLevelDebug, "Test_Category5");
            logging.AddCategory(SeverityLevel::kLevelDebug, "Test_Category6");
            logging.AddCategory(SeverityLevel::kLevelDebug, "Test_Category7");

            logging.SetCategoryEnabled(SeverityLevel::kLevelDebug, "Test_Category1", false);
            logging.SetCategoryEnabled(SeverityLevel::kLevelDebug, "Test_Category2", false);
            logging.SetCategoryEnabled(SeverityLevel::kLevelDebug, "Test_Category3", false);
            logging.SetCategoryEnabled(SeverityLevel::kLevelDebug, "Test_Category4", false);
            logging.SetCategoryEnabled(SeverityLevel::kLevelDebug, "Test_Category5", false);
            logging.SetCategoryEnabled(SeverityLevel::kLevelDebug, "Test_Category6", false);
            logging.SetCategoryEnabled(SeverityLevel::kLevelDebug, "Test_Category7", false);

            const Logging::LevelCategoryMap& categories = logging.GetAllCategories();
            Logging::Categories* debug = categories.at(SeverityLevel::kLevelDebug).get();

            for (Logging::Categories::iterator iter = debug->begin(); iter != debug->end(); ++iter)
            {
                Assert::IsFalse(iter->get()->IsEnabled());
            }

            logging.SetCategoryEnabled(SeverityLevel::kLevelDebug, "Test_Category1", true);
            logging.SetCategoryEnabled(SeverityLevel::kLevelDebug, "Test_Category2", true);
            logging.SetCategoryEnabled(SeverityLevel::kLevelDebug, "Test_Category3", true);
            logging.SetCategoryEnabled(SeverityLevel::kLevelDebug, "Test_Category4", true);
            logging.SetCategoryEnabled(SeverityLevel::kLevelDebug, "Test_Category5", true);
            logging.SetCategoryEnabled(SeverityLevel::kLevelDebug, "Test_Category6", true);
            logging.SetCategoryEnabled(SeverityLevel::kLevelDebug, "Test_Category7", true);

            for (Logging::Categories::iterator iter = debug->begin(); iter != debug->end(); ++iter)
            {
                Assert::IsTrue(iter->get()->IsEnabled());
            }
        }
    };
}