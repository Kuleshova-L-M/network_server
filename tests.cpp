#include <UnitTest++/UnitTest++.h>
#include <UnitTest++/TestReporter.h>
#include <iostream>
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "ConfigHandler.h"
#include "Logger.h"
#include "UserBase.h"
#include "Authenticator.h"
#include "DataProcessor.h"
#include "ServerException.h"

#define CHECK_EQUAL_STR(WS, RS) { std::string w = (WS); std::string r = (RS); CHECK(w.size() == r.size()); CHECK_EQUAL(w, r); }

#define USERS_FILE        "unit_test_users.conf"
#define AUTH_USERS_FILE   "unit_test_auth_users.conf"
#define LOG_FILE_INFO     "unit_test_logger_info.log"
#define MISSING_FILE      "unit_test_no_such_file.conf"

class MinimalReporter : public UnitTest::TestReporter {
private:
    bool currentFailed = false;
    std::string failMsg;
    
    std::string formatName(const std::string& name) {
        if (name.find("Test_") == 0) {
            size_t pos1 = name.find('_', 5);
            if (pos1 != std::string::npos) {
                std::string num1 = name.substr(5, pos1 - 5);
                size_t pos2 = name.find('_', pos1 + 1);
                if (pos2 != std::string::npos) {
                    std::string num2 = name.substr(pos1 + 1, pos2 - pos1 - 1);
                    return num1 + "." + num2;
                }
                return num1;
            }
        }
        return name;
    }
public:
    void ReportTestStart(UnitTest::TestDetails const&) override {
        currentFailed = false;
        failMsg = "";
        std::cerr.flush();
        std::cout.flush();
    }
    
    void ReportFailure(UnitTest::TestDetails const&, char const* failure) override {
        currentFailed = true;
        failMsg = failure;
    }
    
    void ReportTestFinish(UnitTest::TestDetails const& test, float) override {
        std::string num = formatName(test.testName);
        std::cout << "Тест " << num << ": ";
        if (currentFailed) {
            std::cout << "НЕУДАЧА (" << failMsg << ")" << std::endl;
        } else {
            std::cout << "УСПЕХ" << std::endl;
        }
    }
    
    void ReportSummary(int totalTestCount, int failedTestCount, int, float) override {
        std::cout << "\n--------------------------------------------------\n";
        if (failedTestCount == 0) {
            std::cout << "ИТОГ: Все " << totalTestCount << " тестов пройдены!" << std::endl;
        } else {
            std::cout << "ИТОГ: " << failedTestCount << " из " << totalTestCount << " тестов провалены." << std::endl;
        }
    }
};

namespace {
    std::string readAll(const std::string& path) {
        std::ifstream f(path);
        std::ostringstream ss;
        ss << f.rdbuf();
        return ss.str();
    }
}

struct CfgDefault {
    ConfigHandler c;
    CfgDefault() { 
        char* argv[] = { (char*)"test", nullptr };
        c.parse(1, argv); 
    }
};

struct CfgShort {
    ConfigHandler c;
    std::vector<std::string> args;
    std::vector<char*> argv;
    CfgShort() {
        args = {"test", "-c", "users.txt", "-l", "my.log", "-p", "44444"};
        for (auto& s : args) argv.push_back(const_cast<char*>(s.c_str()));
        argv.push_back(nullptr);
        c.parse(static_cast<int>(argv.size()) - 1, argv.data());
    }
};

struct CfgLong {
    ConfigHandler c;
    std::vector<std::string> args;
    std::vector<char*> argv;
    CfgLong() {
        args = {"test", "--config", "users2.txt", "--log", "my2.log", "--port", "55555"};
        for (auto& s : args) argv.push_back(const_cast<char*>(s.c_str()));
        argv.push_back(nullptr);
        c.parse(static_cast<int>(argv.size()) - 1, argv.data());
    }
};

struct CfgHelp {
    ConfigHandler c;
    std::vector<std::string> args;
    std::vector<char*> argv;
    bool ok;
    explicit CfgHelp(std::vector<std::string> a) : args(std::move(a)) {
        for (auto& s : args) argv.push_back(const_cast<char*>(s.c_str()));
        argv.push_back(nullptr);
        ok = c.parse(static_cast<int>(argv.size()) - 1, argv.data());
    }
};
struct CfgHelpShort : CfgHelp { CfgHelpShort() : CfgHelp({"test", "-h"}) {} };
struct CfgHelpLong  : CfgHelp { CfgHelpLong()  : CfgHelp({"test", "--help"}) {} };

struct CfgUnknown {
    ConfigHandler c;
    std::vector<std::string> args;
    std::vector<char*> argv;
    bool ok;
    CfgUnknown() {
        args = {"test", "--unknown-option"};
        for (auto& s : args) argv.push_back(const_cast<char*>(s.c_str()));
        argv.push_back(nullptr);
        ok = c.parse(static_cast<int>(argv.size()) - 1, argv.data());
    }
};

SUITE(ConfigHandlerTests) {
    TEST_FIXTURE(CfgDefault, Test_1_1_DefaultsConfig) { CHECK_EQUAL_STR("./vcalc.conf", c.getConfigFile()); }
    TEST_FIXTURE(CfgDefault, Test_1_1_DefaultsLog)    { CHECK_EQUAL_STR("./vcalc.log",  c.getLogFile());    }
    TEST_FIXTURE(CfgDefault, Test_1_1_DefaultsPort)   { CHECK_EQUAL(33333,              c.getPort());       }

    TEST_FIXTURE(CfgShort, Test_1_2_ShortConfig) { CHECK_EQUAL_STR("users.txt", c.getConfigFile()); }
    TEST_FIXTURE(CfgShort, Test_1_2_ShortLog)    { CHECK_EQUAL_STR("my.log",    c.getLogFile());    }
    TEST_FIXTURE(CfgShort, Test_1_2_ShortPort)   { CHECK_EQUAL(44444,           c.getPort());       }

    TEST_FIXTURE(CfgLong, Test_1_3_LongConfig) { CHECK_EQUAL_STR("users2.txt", c.getConfigFile()); }
    TEST_FIXTURE(CfgLong, Test_1_3_LongLog)    { CHECK_EQUAL_STR("my2.log",    c.getLogFile());    }
    TEST_FIXTURE(CfgLong, Test_1_3_LongPort)   { CHECK_EQUAL(55555,            c.getPort());       }

    TEST_FIXTURE(CfgHelpShort, Test_1_4_HelpShortReturnsFalse) { CHECK(!ok); }
    TEST_FIXTURE(CfgHelpLong,  Test_1_4_HelpLongReturnsFalse)  { CHECK(!ok); }

    TEST_FIXTURE(CfgUnknown,   Test_1_5_UnknownOptionReturnsFalse) { CHECK(!ok); }
}

struct LoggerInfo {
    Logger log;
    LoggerInfo() : log(LOG_FILE_INFO) { std::remove(LOG_FILE_INFO); }
    ~LoggerInfo() { std::remove(LOG_FILE_INFO); }
};

struct LoggerAppend {
    LoggerInfo f;
    LoggerAppend() {
        f.log.write("one", false);
        f.log.write("two", true);
    }
};

SUITE(LoggerTests) {
    TEST_FIXTURE(LoggerInfo, Test_2_1_WritesInfo) {
        log.write("hello", false);
        std::string text = readAll(LOG_FILE_INFO);
        CHECK(text.find("[INFO] hello") != std::string::npos);
        CHECK(text[0] == '['); 
    }
    
    TEST_FIXTURE(LoggerInfo, Test_2_2_WritesCritical) {
        log.write("bad", true);
        std::string text = readAll(LOG_FILE_INFO);
        CHECK(text.find("[CRITICAL] bad") != std::string::npos);
    }
    
    TEST_FIXTURE(LoggerAppend, Test_2_3_TwoLines) {
        std::string text = readAll(LOG_FILE_INFO);
        CHECK_EQUAL(2, static_cast<int>(std::count(text.begin(), text.end(), '\n')));
    }
}

struct UsersLoaded {
    UserBase users;
    UsersLoaded() {
        std::ofstream f(USERS_FILE);
        f << "alice:secret\nbob:pass\r\n\n";
        f.close();
        users.load(USERS_FILE);
    }
    ~UsersLoaded() { std::remove(USERS_FILE); }
};

SUITE(UserBaseTests) {
    TEST_FIXTURE(UsersLoaded, Test_3_1_LoadAndCheck) {
        CHECK(users.checkUser("alice"));
        CHECK(users.checkUser("bob"));
    }
    
    TEST_FIXTURE(UsersLoaded, Test_3_2_GetPassword) {
        CHECK_EQUAL_STR("secret", users.getPass("alice"));
        CHECK_EQUAL_STR("pass",   users.getPass("bob"));
    }
    
    TEST_FIXTURE(UsersLoaded, Test_3_3_UnknownUser) {
        CHECK(!users.checkUser("charlie"));
        CHECK_EQUAL_STR("", users.getPass("charlie"));
    }
    
    TEST(Test_3_4_MissingFileThrows) {
        UserBase u;
        CHECK_THROW(u.load(MISSING_FILE), ServerException);
    }
}

struct AuthUsersFixture {
    UserBase      users;
    Authenticator auth;
    std::string   salt;
    AuthUsersFixture() : salt("0123456789ABCDEF") {
        std::ofstream f(AUTH_USERS_FILE);
        f << "vasya:secret\n";
        f.close();
        users.load(AUTH_USERS_FILE);
    }
    ~AuthUsersFixture() { std::remove(AUTH_USERS_FILE); }
};

SUITE(AuthenticatorTests) {
    TEST(Test_4_1_SaltLength) {
        Authenticator auth;
        CHECK_EQUAL(16, static_cast<int>(auth.genSalt().size()));
    }
    
    TEST(Test_4_2_SaltDiffers) {
        Authenticator auth;
        CHECK(auth.genSalt() != auth.genSalt());
    }
    
    TEST(Test_4_3_HashKnownVector) {
        Authenticator auth;
        std::string s = "4F9C429F5C6884DB726445BE";
        std::string p = "SupErP@$$w0rd";
        std::string e = "9C67ABCFAD8E46E78F89173497800E5EE143D67C28765A60C947221E7114FF68";
        CHECK_EQUAL_STR(e, auth.calcHash(s, p));
    }
    
    TEST_FIXTURE(AuthUsersFixture, Test_4_4_AuthSuccess) {
        std::string h = auth.calcHash(salt, "secret");
        CHECK(auth.checkAuth("vasya", h, salt, users));
    }
    
    TEST_FIXTURE(AuthUsersFixture, Test_4_5_AuthWrongPassword) {
        std::string h(64, '0');
        CHECK(!auth.checkAuth("vasya", h, salt, users));
    }
}

SUITE(DataProcessorTests) {
    TEST(Test_5_1_Empty)        { CHECK_EQUAL(1,  DataProcessor::calcProduct({})); }
    TEST(Test_5_2_Simple)       { CHECK_EQUAL(24, DataProcessor::calcProduct({1,2,3,4})); }
    TEST(Test_5_2_Single)       { CHECK_EQUAL(5,  DataProcessor::calcProduct({5})); }
    TEST(Test_5_3_Zero)         { CHECK_EQUAL(0,  DataProcessor::calcProduct({5,0,7})); }
    TEST(Test_5_4_MaxBoundary)  { CHECK_EQUAL(2147483647, DataProcessor::calcProduct({2147483647})); }
    TEST(Test_5_4_MaxTimes1)    { CHECK_EQUAL(2147483647, DataProcessor::calcProduct({2147483647, 1})); }
    TEST(Test_5_5_OverflowHigh) { CHECK_EQUAL(2147483647, DataProcessor::calcProduct({2147483647, 2})); }
    TEST(Test_5_5_OverflowHuge) {
        CHECK_EQUAL(2147483647, DataProcessor::calcProduct({
            std::numeric_limits<uint32_t>::max(),
            std::numeric_limits<uint32_t>::max()
        }));
    }
}

SUITE(ServerExceptionTests) {
    TEST(Test_6_1_MessageAndCritical) {
        ServerException e("msg", true);
        CHECK_EQUAL_STR("msg", e.what());
        CHECK(e.isCritical());
    }
    TEST(Test_6_2_NonCritical) {
        ServerException e("x", false);
        CHECK(!e.isCritical());
    }
}

int main() {
    MinimalReporter reporter;
    UnitTest::TestRunner runner(reporter);
    return runner.RunTestsIf(UnitTest::Test::GetTestList(), nullptr, UnitTest::True(), 0);
}
