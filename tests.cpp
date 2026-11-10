#include <UnitTest++/UnitTest++.h>
#include <UnitTest++/TestReporterStdout.h>

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
#define LOG_FILE_APPEND   "unit_test_logger_append.log"
#define MISSING_FILE      "unit_test_no_such_file.conf"

auto RunSuite(const char* suiteName) {
    UnitTest::TestReporterStdout reporter;
    UnitTest::TestRunner runner(reporter);
    return runner.RunTestsIf(
        UnitTest::Test::GetTestList(), suiteName,
        UnitTest::True(), 0);
}


namespace {
std::string readAll(const std::string& path) {
    std::ifstream f(path);
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}
bool isUpperHex(const std::string& s) {
    return std::all_of(s.begin(), s.end(), [](char c) {
        return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F');
    });
}
}


// 1.1: значения по умолчанию 
struct CfgDefault {
    ConfigHandler c;
    CfgDefault() { 
        char* argv[] = { (char*)"test", nullptr };
        c.parse(1, argv); 
    }
};
//  1.2: короткие опции 
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

// 1.3: длинные опции 
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

//  1.4 и 1.5: --help / -h 
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

// 1.6: неизвестная опция 
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

// 1.7: неверный порт 
struct CfgBadPort {
    ConfigHandler c;
    std::vector<std::string> args;
    std::vector<char*> argv;
    bool ok;
    CfgBadPort() {
        args = {"test", "-p", "70000"};
        for (auto& s : args) argv.push_back(const_cast<char*>(s.c_str()));
        argv.push_back(nullptr);
        ok = c.parse(static_cast<int>(argv.size()) - 1, argv.data());
    }
};

SUITE(ConfigHandlerTests) {
    TEST_FIXTURE(CfgDefault, DefaultsConfig) { CHECK_EQUAL_STR("./vcalc.conf", c.getConfigFile()); }
    TEST_FIXTURE(CfgDefault, DefaultsLog)    { CHECK_EQUAL_STR("./vcalc.log",  c.getLogFile());    }
    TEST_FIXTURE(CfgDefault, DefaultsPort)   { CHECK_EQUAL(33333,              c.getPort());       }

    TEST_FIXTURE(CfgShort, ShortConfig) { CHECK_EQUAL_STR("users.txt", c.getConfigFile()); }
    TEST_FIXTURE(CfgShort, ShortLog)    { CHECK_EQUAL_STR("my.log",    c.getLogFile());    }
    TEST_FIXTURE(CfgShort, ShortPort)   { CHECK_EQUAL(44444,           c.getPort());       }

    TEST_FIXTURE(CfgLong, LongConfig) { CHECK_EQUAL_STR("users2.txt", c.getConfigFile()); }
    TEST_FIXTURE(CfgLong, LongLog)    { CHECK_EQUAL_STR("my2.log",    c.getLogFile());    }
    TEST_FIXTURE(CfgLong, LongPort)   { CHECK_EQUAL(55555,            c.getPort());       }

    TEST_FIXTURE(CfgHelpShort, HelpShortReturnsFalse) { CHECK(!ok); }
    TEST_FIXTURE(CfgHelpLong,  HelpLongReturnsFalse)  { CHECK(!ok); }
    TEST_FIXTURE(CfgUnknown,   UnknownReturnsFalse)   { CHECK(!ok); }
    TEST_FIXTURE(CfgBadPort,   BadPortReturnsFalse)   { CHECK(!ok); }
}


// Сценарий 2. Проверка Logger


struct LoggerInfo {
    Logger log;
    LoggerInfo() : log(LOG_FILE_INFO) { std::remove(LOG_FILE_INFO); }
    ~LoggerInfo() { std::remove(LOG_FILE_INFO); }
};

struct LoggerCritical {
    Logger log;
    LoggerCritical() : log(LOG_FILE_INFO) { std::remove(LOG_FILE_INFO); }
    ~LoggerCritical() { std::remove(LOG_FILE_INFO); }
};

struct LoggerAppend {
    LoggerInfo f;
    LoggerAppend() {
        f.log.write("one", false);
        f.log.write("two", true);
    }
};

SUITE(LoggerTests) {
    // 2.1: запись INFO
    TEST_FIXTURE(LoggerInfo, WritesInfo) {
        log.write("hello", false);
        std::string text = readAll(LOG_FILE_INFO);
        CHECK(text.find("[INFO] hello") != std::string::npos);
        REQUIRE CHECK(text.size() > 0);
        CHECK(text[0] == '[');        // метка времени в начале
    }

    // 2.2: запись CRITICAL
    TEST_FIXTURE(LoggerCritical, WritesCritical) {
        log.write("bad", true);
        std::string text = readAll(LOG_FILE_INFO);
        CHECK(text.find("[CRITICAL] bad") != std::string::npos);
    }

    // 2.3: дописывание (две строки)
    TEST_FIXTURE(LoggerAppend, TwoLines) {
        std::string text = readAll(LOG_FILE_INFO);
        CHECK_EQUAL(2, static_cast<int>(std::count(text.begin(), text.end(), '\n')));
    }
}

// Сценарий 3. Проверка UserBase
struct UsersFixture {
    UsersFixture() {
        std::ofstream f(USERS_FILE);
        f << "alice:secret\nbob:pass\r\n\n";
    }
    ~UsersFixture() { std::remove(USERS_FILE); }
    UserBase users;
};

struct UsersLoaded : UsersFixture {
    UsersLoaded() { users.load(USERS_FILE); }
};

SUITE(UserBaseTests) {
    // 3.1: загрузка базы и наличие пользователей
    TEST_FIXTURE(UsersLoaded, LoadAndCheck) {
        CHECK(users.checkUser("alice"));
        CHECK(users.checkUser("bob"));
    }

    // 3.2: получение пароля и срезание \r
    TEST_FIXTURE(UsersLoaded, GetPassword) {
        CHECK_EQUAL_STR("secret", users.getPass("alice"));
        CHECK_EQUAL_STR("pass",   users.getPass("bob"));
    }

    // 3.3: неизвестный пользователь
    TEST_FIXTURE(UsersLoaded, UnknownUser) {
        CHECK(!users.checkUser("charlie"));
        CHECK_EQUAL_STR("", users.getPass("charlie"));
    }

    // 3.4: отсутствующий файл — CHECK_THROW (методичка, стр. 7)
    TEST(MissingFileThrows) {
        UserBase u;
        CHECK_THROW(u.load(MISSING_FILE), ServerException);
    }

    // 3.5: флаг критичности
    TEST(MissingFileIsCritical) {
        UserBase u;
        try {
            u.load(MISSING_FILE);
            CHECK(false); 
        } catch (const ServerException& e) {
            CHECK(e.isCritical());
            CHECK(std::string(e.what()).find("Не удалось открыть") != std::string::npos);
        }
    }
}

// Сценарий 4. Проверка Authenticator

struct AuthSalt {
    Authenticator auth;
    std::string first;
    std::string second;
    AuthSalt() {
        first  = auth.genSalt();
        second = auth.genSalt();
    }
};

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
    // 4.1: SALT длиной 16 hex
    TEST_FIXTURE(AuthSalt, SaltLength)   { CHECK_EQUAL(16, static_cast<int>(first.size())); }
    TEST_FIXTURE(AuthSalt, SaltUpperHex) { CHECK(isUpperHex(first)); }

    // 4.2: последовательные SALT различаются (методичка, стр. 39)
    TEST_FIXTURE(AuthSalt, SaltDiffers)  { CHECK(first != second); }

    // 4.3: SHA-256 по эталону из методички (стр. 39)
    TEST(HashKnownVector) {
        Authenticator auth;
        std::string s = "4F9C429F5C6884DB726445BE";
        std::string p = "SupErP@$$w0rd";
        std::string e = "9C67ABCFAD8E46E78F89173497800E5EE143D67C28765A60C947221E7114FF68";
        std::string a = auth.calcHash(s, p);
        CHECK_EQUAL_STR(e, a);
        CHECK_EQUAL(64, static_cast<int>(a.size()));
        CHECK(isUpperHex(a));
    }

    // 4.4: успешная аутентификация
    TEST_FIXTURE(AuthUsersFixture, AuthSuccess) {
        std::string h = auth.calcHash(salt, "secret");
        CHECK(auth.checkAuth("vasya", h, salt, users));
    }

    // 4.5: неверный пароль
    TEST_FIXTURE(AuthUsersFixture, AuthWrongPassword) {
        std::string h(64, '0');
        CHECK(!auth.checkAuth("vasya", h, salt, users));
    }

    // 4.6: неизвестный пользователь
    TEST_FIXTURE(AuthUsersFixture, AuthUnknownUser) {
        std::string h = auth.calcHash(salt, "secret");
        CHECK(!auth.checkAuth("nobody", h, salt, users));
    }
}


// Сценарий 5. Проверка DataProcessor::calcProduct

SUITE(DataProcessorTests) {
    // 5.1: пустой вектор — нейтральный элемент
    TEST(Empty)        { CHECK_EQUAL(1,  DataProcessor::calcProduct({})); }

    // 5.2: обычное произведение
    TEST(Simple)       { CHECK_EQUAL(24, DataProcessor::calcProduct({1,2,3,4})); }
    TEST(Single)       { CHECK_EQUAL(5,  DataProcessor::calcProduct({5})); }

    // 5.3: ноль
    TEST(Zero)         { CHECK_EQUAL(0,  DataProcessor::calcProduct({5,0,7})); }

    // 5.4: ровно по границе int32
    TEST(MaxBoundary)  { CHECK_EQUAL(2147483647, DataProcessor::calcProduct({2147483647})); }
    TEST(MaxTimes1)    { CHECK_EQUAL(2147483647, DataProcessor::calcProduct({2147483647, 1})); }

    // 5.5: переполнение вверх 
    TEST(OverflowHigh) { CHECK_EQUAL(2147483647, DataProcessor::calcProduct({2147483647, 2})); }
    TEST(OverflowHuge) {
        CHECK_EQUAL(2147483647, DataProcessor::calcProduct({
            std::numeric_limits<uint32_t>::max(),
            std::numeric_limits<uint32_t>::max()
        }));
    }
}

// Сценарий 6. Проверка ServerException

SUITE(ServerExceptionTests) {
    TEST(MessageAndCritical) {
        ServerException e("msg", true);
        CHECK_EQUAL_STR("msg", e.what());
        CHECK(e.isCritical());
    }
    TEST(NonCritical) {
        ServerException e("x", false);
        CHECK(!e.isCritical());
    }
}


// Точка входа

int main() {
    return UnitTest::RunAllTests();
}
