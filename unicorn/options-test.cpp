#include "unicorn/core.hpp"
#include "unicorn/options.hpp"
#include "unicorn/unit-test.hpp"
#include <sstream>
#include <vector>

using namespace RS;
using namespace RS::Unicorn;
using namespace std::literals;

namespace {

    Options opt1;
    std::ostringstream nowhere;

}

void test_unicorn_options_basic() {

    Options opt2;
    Ustring cmdline;

    TRY(opt1 = Options("App version 1.0"));
    TEST_EQUAL(opt1.version_text(), "App version 1.0");
    TRY(opt1.add("alpha", "Alpha option", Options::abbrev="a", Options::defvalue="ABC"));
    TRY(opt1.add("--number", "Number option", Options::abbrev="n", Options::defvalue="123"));

    {
        TRY(opt2 = opt1);
        cmdline = "app --version";
        std::ostringstream out;
        TEST(opt2.parse(cmdline, out));
        TEST_EQUAL(out.str(), "App version 1.0\n");
    }

    {
        TRY(opt2 = opt1);
        cmdline = "app --help";
        std::ostringstream out;
        TEST(opt2.parse(cmdline, out));
        TEST_EQUAL(out.str(),
            "\n"
            "App version 1.0\n"
            "\n"
            "Options:\n"
            "\n"
            "    --alpha, -a <arg>   = Alpha option (default \"ABC\")\n"
            "    --number, -n <arg>  = Number option (default 123)\n"
            "    --help, -h          = Show usage information\n"
            "    --version, -v       = Show version information\n"
            "\n"
        );
    }

    {
        TRY(opt2 = opt1);
        cmdline = "app";
        std::ostringstream out;
        TEST(! opt2.parse(cmdline, nowhere));
        TEST_EQUAL(out.str(), "");
        TEST(! opt2.has("alpha"));
        TEST_EQUAL(opt2.get<Ustring>("alpha"), "ABC");
        TEST(! opt2.has("number"));
        TEST_EQUAL(opt2.get<int>("number"), 123);
        TEST_THROW_MATCH(opt2.has("nonexistent"), Options::spec_error, ": \"--nonexistent\"$");
        TEST_THROW_MATCH(opt2.get<Ustring>("nonexistent"), Options::spec_error, ": \"--nonexistent\"$");
        TEST_THROW_MATCH(opt2.get_list<Ustring>("nonexistent"), Options::spec_error, ": \"--nonexistent\"$");
    }

    TRY(opt2 = opt1);
    cmdline = "app --alpha xyz -n 999";
    TEST(! opt2.parse(cmdline, nowhere));
    TEST(opt2.has("alpha"));
    TEST_EQUAL(opt2.get<Ustring>("alpha"), "xyz");
    TEST(opt2.has("number"));
    TEST_EQUAL(opt2.get<int>("number"), 999);

    TRY(opt2 = opt1);
    cmdline = "app --alpha=xyz -n=999";
    TEST(! opt2.parse(cmdline, nowhere));
    TEST(opt2.has("alpha"));
    TEST_EQUAL(opt2.get<Ustring>("alpha"), "xyz");
    TEST(opt2.has("number"));
    TEST_EQUAL(opt2.get<int>("number"), 999);

    TRY(opt2 = opt1);
    cmdline = "app --alpha -n";
    TEST(! opt2.parse(cmdline, nowhere));
    TEST(opt2.has("alpha"));
    TEST_EQUAL(opt2.get<Ustring>("alpha"), "ABC");
    TEST(opt2.has("number"));
    TEST_EQUAL(opt2.get<int>("number"), 123);

    TRY(opt2 = opt1);
    cmdline = "app -a uvw xyz";
    TEST_THROW_MATCH(opt2.parse(cmdline), Options::command_error, ": \"xyz\"$");

    TRY(opt2 = opt1);
    cmdline = "app -a xyz -n 999";
    TEST(! opt2.parse(cmdline, nowhere, Options::quoted));
    TEST(opt2.has("alpha"));
    TEST_EQUAL(opt2.get<Ustring>("alpha"), "xyz");
    TEST(opt2.has("number"));
    TEST_EQUAL(opt2.get<int>("number"), 999);

    TRY(opt2 = opt1);
    cmdline = "app -a \"xyz\" -n \"999\"";
    TEST(! opt2.parse(cmdline, nowhere, Options::quoted));
    TEST(opt2.has("alpha"));
    TEST_EQUAL(opt2.get<Ustring>("alpha"), "xyz");
    TEST(opt2.has("number"));
    TEST_EQUAL(opt2.get<int>("number"), 999);

    TRY(opt2 = opt1);
    cmdline = "app -a \"uvw xyz\"";
    TEST(! opt2.parse(cmdline, nowhere, Options::quoted));
    TEST(opt2.has("alpha"));
    TEST_EQUAL(opt2.get<Ustring>("alpha"), "uvw xyz");

    TRY(opt2 = opt1);
    cmdline = "app -a \"\"\"uvw\"\" \"\"xyz\"\"\"";
    TEST(! opt2.parse(cmdline, nowhere, Options::quoted));
    TEST(opt2.has("alpha"));
    TEST_EQUAL(opt2.get<Ustring>("alpha"), "\"uvw\" \"xyz\"");

}

void test_unicorn_options_boolean() {

    Options opt2("Blank");
    Ustring cmdline;

    TRY(opt1.add("--foo", "Positive option", Options::boolean, Options::abbrev="f"));
    TRY(opt1.add("--no-bar", "Negative option", Options::boolean));

    TRY(opt2 = opt1);
    cmdline = "app";
    TEST(! opt2.parse(cmdline, nowhere));
    TEST(! opt2.has("foo"));
    TEST(! opt2.has("bar"));
    TEST(! opt2.get<bool>("foo"));
    TEST(opt2.get<bool>("bar"));
    TEST_EQUAL(opt2.get<Ustring>("foo"), "");
    TEST_EQUAL(opt2.get<Ustring>("bar"), "1");

    TRY(opt2 = opt1);
    cmdline = "app --foo --bar";
    TEST(! opt2.parse(cmdline, nowhere));
    TEST(opt2.has("foo"));
    TEST(opt2.has("bar"));
    TEST(opt2.get<bool>("foo"));
    TEST(opt2.get<bool>("bar"));
    TEST_EQUAL(opt2.get<Ustring>("foo"), "1");
    TEST_EQUAL(opt2.get<Ustring>("bar"), "1");

    TRY(opt2 = opt1);
    cmdline = "app --no-foo --no-bar";
    TEST(! opt2.parse(cmdline, nowhere));
    TEST(opt2.has("foo"));
    TEST(opt2.has("bar"));
    TEST(! opt2.get<bool>("foo"));
    TEST(! opt2.get<bool>("bar"));
    TEST_EQUAL(opt2.get<Ustring>("foo"), "0");
    TEST_EQUAL(opt2.get<Ustring>("bar"), "0");

    TRY(opt2 = opt1);
    cmdline = "app -f";
    TEST(! opt2.parse(cmdline, nowhere));
    TEST(opt2.has("foo"));
    TEST(opt2.get<bool>("foo"));
    TEST_EQUAL(opt2.get<Ustring>("foo"), "1");

}

void test_unicorn_options_multiple() {

    Options opt2("Blank");
    Ustring cmdline;
    Strings sv;

    TRY(opt1.add("list", "List option", Options::multi, Options::abbrev="l"));

    TRY(opt2 = opt1);
    cmdline = "app";
    TEST(! opt2.parse(cmdline, nowhere));
    TEST(! opt2.has("list"));
    TEST_EQUAL(opt2.get<Ustring>("list"), "");
    TRY(sv = opt2.get_list<Ustring>("list"));
    TEST(sv.empty());

    TRY(opt2 = opt1);
    cmdline = "app --list abc";
    TEST(! opt2.parse(cmdline, nowhere));
    TEST(opt2.has("list"));
    TEST_EQUAL(opt2.get<Ustring>("list"), "abc");
    TRY(sv = opt2.get_list<Ustring>("list"));
    TEST_EQUAL(sv.size(), 1);
    TEST_EQUAL(to_str(sv), "[abc]");

    TRY(opt2 = opt1);
    cmdline = "app --list abc def ghi";
    TEST(! opt2.parse(cmdline, nowhere));
    TEST(opt2.has("list"));
    TEST_EQUAL(opt2.get<Ustring>("list"), "abc def ghi");
    TRY(sv = opt2.get_list<Ustring>("list"));
    TEST_EQUAL(sv.size(), 3);
    TEST_EQUAL(to_str(sv), "[abc,def,ghi]");

}

void test_unicorn_options_required() {

    Options opt2("Blank");
    Ustring cmdline;

    TRY(opt1.add("required", "Required option", Options::required, Options::abbrev="r"));

    TRY(opt2 = opt1);
    cmdline = "app";
    TEST_THROW_MATCH(opt2.parse(cmdline), Options::command_error, ": \"--required\"$");

    TRY(opt2 = opt1);
    cmdline = "app --required abc";
    TEST(! opt2.parse(cmdline, nowhere));
    TEST(opt2.has("required"));
    TEST_EQUAL(opt2.get<Ustring>("required"), "abc");

    TRY(opt2 = opt1);
    cmdline = "app --version";
    std::ostringstream out;
    TEST(opt2.parse(cmdline, out));
    TEST_EQUAL(out.str(), "App version 1.0\n");

}

void test_unicorn_options_anonymous() {

    Options opt2("Blank");
    Ustring cmdline;
    Strings sv;

    TRY(opt1.add("head", "First anonymous argument", Options::anon));

    TRY(opt2 = opt1);
    cmdline = "app --required abc";
    TEST(! opt2.parse(cmdline, nowhere));
    TEST(! opt2.has("head"));
    TEST_EQUAL(opt2.get<Ustring>("head"), "");

    TRY(opt2 = opt1);
    cmdline = "app --required abc def";
    TEST(! opt2.parse(cmdline, nowhere));
    TEST(opt2.has("head"));
    TEST_EQUAL(opt2.get<Ustring>("head"), "def");

    TRY(opt2 = opt1);
    cmdline = "app --required abc def ghi";
    TEST_THROW_MATCH(opt2.parse(cmdline), Options::command_error, ": \"ghi\"$");

    TRY(opt1.add("tail", "Other anonymous arguments", Options::anon, Options::multi));

    TRY(opt2 = opt1);
    cmdline = "app --required abc";
    TEST(! opt2.parse(cmdline, nowhere));
    TEST(! opt2.has("head"));
    TEST_EQUAL(opt2.get<Ustring>("head"), "");
    TEST(! opt2.has("tail"));
    TEST_EQUAL(opt2.get<Ustring>("tail"), "");
    TRY(sv = opt2.get_list<Ustring>("tail"));
    TEST(sv.empty());

    TRY(opt2 = opt1);
    cmdline = "app --required abc def";
    TEST(! opt2.parse(cmdline, nowhere));
    TEST(opt2.has("head"));
    TEST_EQUAL(opt2.get<Ustring>("head"), "def");
    TEST(! opt2.has("tail"));
    TEST_EQUAL(opt2.get<Ustring>("tail"), "");
    TRY(sv = opt2.get_list<Ustring>("tail"));
    TEST(sv.empty());

    TRY(opt2 = opt1);
    cmdline = "app --required abc def ghi";
    TEST(! opt2.parse(cmdline, nowhere));
    TEST(opt2.has("head"));
    TEST_EQUAL(opt2.get<Ustring>("head"), "def");
    TEST(opt2.has("tail"));
    TEST_EQUAL(opt2.get<Ustring>("tail"), "ghi");
    TRY(sv = opt2.get_list<Ustring>("tail"));
    TEST_EQUAL(sv.size(), 1);
    TEST_EQUAL(to_str(sv), "[ghi]");

    TRY(opt2 = opt1);
    cmdline = "app --required abc def ghi jkl";
    TEST(! opt2.parse(cmdline, nowhere));
    TEST(opt2.has("head"));
    TEST_EQUAL(opt2.get<Ustring>("head"), "def");
    TEST(opt2.has("tail"));
    TEST_EQUAL(opt2.get<Ustring>("tail"), "ghi jkl");
    TRY(sv = opt2.get_list<Ustring>("tail"));
    TEST_EQUAL(sv.size(), 2);
    TEST_EQUAL(to_str(sv), "[ghi,jkl]");

}

void test_unicorn_options_group() {

    Options opt2("App");
    TRY(opt2.add("group1a", "Group 1 a", Options::group="neddie"));
    TRY(opt2.add("group1b", "Group 1 b", Options::group="neddie"));
    TRY(opt2.add("group2a", "Group 2 a", Options::group="eccles"));
    TRY(opt2.add("group2b", "Group 2 b", Options::group="eccles"));

    Options opt3("Blank");
    Ustring cmdline;

    TRY(opt3 = opt2);
    cmdline = "app --group1a abc";
    TEST(! opt3.parse(cmdline, nowhere));
    TEST_EQUAL(opt3.get<Ustring>("group1a"), "abc");

    TRY(opt3 = opt2);
    cmdline = "app --group1a abc --group1b def";
    TEST_THROW_MATCH(opt3.parse(cmdline), Options::command_error, ": \"--group1a\", \"--group1b\"$");

    TRY(opt3 = opt2);
    cmdline = "app --group1a abc --group2a def";
    TEST(! opt3.parse(cmdline, nowhere));
    TEST_EQUAL(opt3.get<Ustring>("group1a"), "abc");
    TEST_EQUAL(opt3.get<Ustring>("group2a"), "def");

}

void test_unicorn_options_patterns() {

    Options opt2("App");
    TRY(opt2.add("alpha", "Alpha", Options::abbrev="a", Options::anon, Options::defvalue="Hello", Options::multi, Options::pattern="[[:alpha:]]+"));
    TRY(opt2.add("number", "Number", Options::abbrev="n", Options::defvalue="42", Options::multi, Options::pattern="\\d+"));
    TEST_THROW_MATCH(opt2.add("word", "Word", Options::defvalue="*", Options::pattern="\\w+"), Options::spec_error, ": \"word\"$");

    Options opt3("Blank");
    Ustring cmdline;

    TRY(opt3 = opt2);
    cmdline = "app --alpha abc --number 123";
    TEST(! opt3.parse(cmdline, nowhere));
    TEST_EQUAL(opt3.get<Ustring>("alpha"), "abc");
    TEST_EQUAL(opt3.get<int>("number"), 123);

    TRY(opt3 = opt2);
    cmdline = "app abc -n 123";
    TEST(! opt3.parse(cmdline, nowhere));
    TEST_EQUAL(opt3.get<Ustring>("alpha"), "abc");
    TEST_EQUAL(opt3.get<int>("number"), 123);

    TRY(opt3 = opt2);
    cmdline = "app --alpha 123";
    TEST_THROW_MATCH(opt3.parse(cmdline), Options::command_error, ": \"--alpha\", \"123\"$");

    TRY(opt3 = opt2);
    cmdline = "app --number abc";
    TEST_THROW_MATCH(opt3.parse(cmdline), Options::command_error, ": \"--number\", \"abc\"$");

    TRY(opt3 = opt2);
    cmdline = "app -a abc 123 -n 456";
    TEST_THROW_MATCH(opt3.parse(cmdline), Options::command_error, ": \"--alpha\", \"123\"$");

    TRY(opt3 = opt2);
    cmdline = "app -a abc -n 123 def";
    TEST_THROW_MATCH(opt3.parse(cmdline), Options::command_error, ": \"--number\", \"def\"$");

    TRY(opt3 = opt2);
    cmdline = "app abc 123";
    TEST_THROW_MATCH(opt3.parse(cmdline), Options::command_error, ": \"--alpha\", \"123\"$");

    TRY(opt2 = Options("App"));
    TRY(opt2.add("int", "Integer", Options::integer, Options::abbrev="i"));
    TRY(opt2.add("uint", "Unsigned integer", Options::uinteger, Options::abbrev="u"));
    TRY(opt2.add("float", "Float", Options::floating, Options::abbrev="f"));

    TRY(opt3 = opt2);
    cmdline = "app";
    TEST(! opt3.parse(cmdline, nowhere));
    TEST_EQUAL(opt3.get<int>("int"), 0);
    TEST_EQUAL(opt3.get<unsigned>("uint"), 0);
    TEST_EQUAL(opt3.get<float>("float"), 0);

    TRY(opt3 = opt2);
    cmdline = "app --int -42 --uint 42 --float 1.234e5";
    TEST(! opt3.parse(cmdline, nowhere));
    TEST_EQUAL(opt3.get<int>("int"), -42);
    TEST_EQUAL(opt3.get<unsigned>("uint"), 42);
    TEST_EQUAL(opt3.get<float>("float"), 123400);

    TRY(opt3 = opt2);
    cmdline = "app --int 0x42";
    TEST(! opt3.parse(cmdline, nowhere));
    TEST_EQUAL(opt3.get<int>("int"), 66);

    TRY(opt3 = opt2);
    cmdline = "app --int 24k";
    TEST(! opt3.parse(cmdline, nowhere));
    TEST_EQUAL(opt3.get<int>("int"), 24000);

    TRY(opt3 = opt2);
    cmdline = "app --int 25MB";
    TEST(! opt3.parse(cmdline, nowhere));
    TEST_EQUAL(opt3.get<int>("int"), 25000000);

    TRY(opt3 = opt2);
    cmdline = "app --float 2.5MB";
    TEST(! opt3.parse(cmdline, nowhere));
    TEST_EQUAL(opt3.get<float>("float"), 2500000);

    TRY(opt3 = opt2);
    cmdline = "app --int 1234.5";
    TEST_THROW_MATCH(opt3.parse(cmdline), Options::command_error, ": \"--int\", \"1234.5\"$");

    TRY(opt3 = opt2);
    cmdline = "app --uint -1234";
    TEST_THROW_MATCH(opt3.parse(cmdline), Options::command_error, ": \"--uint\", \"-1234\"$");

    TRY(opt3 = opt2);
    cmdline = "app --float 0x1234";
    TEST_THROW_MATCH(opt3.parse(cmdline), Options::command_error, ": \"--float\", \"0x1234\"$");

    TRY(opt2 = Options("App"));
    TRY(opt2.add("int", "Integer", Options::anon, Options::required, Options::abbrev="i"));
    TRY(opt2.add("str", "String", Options::required, Options::abbrev="s"));

    TRY(opt3 = opt2);
    cmdline = "app -s hello 42";
    TEST(! opt3.parse(cmdline, nowhere));
    TEST_EQUAL(opt3.get<int>("int"), 42);
    TEST_EQUAL(opt3.get<Ustring>("str"), "hello");

    TRY(opt3 = opt2);
    cmdline = "app 42 -s hello";
    TEST(! opt3.parse(cmdline, nowhere));
    TEST_EQUAL(opt3.get<int>("int"), 42);
    TEST_EQUAL(opt3.get<Ustring>("str"), "hello");

}

void test_unicorn_options_help() {

    Options opt2("Blank");
    Ustring cmdline;

    {
        TRY(opt2 = opt1);
        cmdline = "app --help";
        std::ostringstream out;
        TEST(opt2.parse(cmdline, out));
        TEST_EQUAL(out.str(),
            "\n"
            "App version 1.0\n"
            "\n"
            "Options:\n"
            "\n"
            "    --alpha, -a <arg>     = Alpha option (default \"ABC\")\n"
            "    --number, -n <arg>    = Number option (default 123)\n"
            "    --foo, -f             = Positive option\n"
            "    --no-bar              = Negative option\n"
            "    --list, -l <arg> ...  = List option\n"
            "    --required, -r <arg>  = Required option (required)\n"
            "    [--head] <arg>        = First anonymous argument\n"
            "    [--tail] <arg> ...    = Other anonymous arguments\n"
            "    --help, -h            = Show usage information\n"
            "    --version, -v         = Show version information\n"
            "\n"
        );
    }

    {
        TRY(opt2 = opt1);
        TRY(opt2.add(Options::autohelp));
        cmdline = "app";
        std::ostringstream out;
        TEST(opt2.parse(cmdline, out));
        TEST_EQUAL(out.str(),
            "\n"
            "App version 1.0\n"
            "\n"
            "Options:\n"
            "\n"
            "    --alpha, -a <arg>     = Alpha option (default \"ABC\")\n"
            "    --number, -n <arg>    = Number option (default 123)\n"
            "    --foo, -f             = Positive option\n"
            "    --no-bar              = Negative option\n"
            "    --list, -l <arg> ...  = List option\n"
            "    --required, -r <arg>  = Required option (required)\n"
            "    [--head] <arg>        = First anonymous argument\n"
            "    [--tail] <arg> ...    = Other anonymous arguments\n"
            "    --help, -h            = Show usage information\n"
            "    --version, -v         = Show version information\n"
            "\n"
        );
    }

}

void test_unicorn_options_insertions() {

    Options opt("App version 1.0");
    Ustring help;

    TRY(opt = Options("App 1.0"));
    TRY(opt.add("Intro."));
    TRY(opt.add("alpha", "Alpha option"));
    TRY(help = opt.help_text());
    TEST_EQUAL(help,
        "\n"
        "App 1.0\n"
        "\n"
        "Intro.\n"
        "\n"
        "Options:\n"
        "\n"
        "    --alpha <arg>  = Alpha option\n"
        "\n"
    );

    TRY(opt = Options("App 1.0"));
    TRY(opt.add("alpha", "Alpha option"));
    TRY(opt.add("Outro."));
    TRY(help = opt.help_text());
    TEST_EQUAL(help,
        "\n"
        "App 1.0\n"
        "\n"
        "Options:\n"
        "\n"
        "    --alpha <arg>  = Alpha option\n"
        "\n"
        "Outro.\n"
        "\n"
    );

    TRY(opt = Options("App 1.0"));
    TRY(opt.add("Intro."));
    TRY(opt.add("alpha", "Alpha option"));
    TRY(opt.add("Some info."));
    TRY(opt.add("bravo", "Bravo option"));
    TRY(opt.add("charlie", "Charlie option"));
    TRY(opt.add("Some more info."));
    TRY(opt.add("Yet more info."));
    TRY(opt.add("delta", "Delta option"));
    TRY(opt.add("Outro."));
    TRY(help = opt.help_text());
    TEST_EQUAL(help,
        "\n"
        "App 1.0\n"
        "\n"
        "Intro.\n"
        "\n"
        "Options:\n"
        "\n"
        "    --alpha <arg>    = Alpha option\n"
        "\n"
        "Some info.\n"
        "\n"
        "    --bravo <arg>    = Bravo option\n"
        "    --charlie <arg>  = Charlie option\n"
        "\n"
        "Some more info.\n"
        "\n"
        "Yet more info.\n"
        "\n"
        "    --delta <arg>    = Delta option\n"
        "\n"
        "Outro.\n"
        "\n"
    );

}
