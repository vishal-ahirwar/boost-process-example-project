// TODO : remove goto
#include <iostream>
#include <boost-process-example-projectconfig.h>
#include <boost/process.hpp>
#include <filesystem>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fstream>

char lc[] = {"/\\-+"};
namespace fs = std::filesystem;

void log(const std::string_view &msg, const char &end = '\n')
{
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::white), "{}{:<5}: ", end, "Info");
    fmt::print(fmt::emphasis::faint | fmt::fg(fmt::color::ghost_white), "{}{}", msg, end);
};

int main(int argc, char *argv[])
{
    srand(time(0));
    try
    {
        if (!fs::exists("build"))
        {
        build:
            boost::process::ipstream ss;
            std::stringstream stream;
            boost::process::child c(boost::process::search_path("cmake"), "--preset=windows", "-Bbuild/debug", boost::process::std_out > ss, boost::process::std_err > ss);
            std::string line{};
            while (c.running() && std::getline(ss, line))
            {
                log(std::format("Generating Config...{}", lc[rand() % strlen(lc)]), '\r');
                stream << line << "\n";
            }
            c.wait();
            if (c.exit_code() != 0)
            {
                std::ofstream out{"build/cmake-log.txt"};
                out << stream.rdbuf();
                out.close();
                log(std::format("Generating Config failed read cmake-log.txt for more info"));
                return 1;
            }
            goto compile;
        }
        else
        {
        compile:

            boost::process::ipstream ss;
            boost::process::child c(boost::process::search_path("cmake"), "--build", "build/debug", boost::process::std_out > ss);
            std::string line{};
            std::stringstream stream;
            while (c.running() && std::getline(ss, line))
            {
                log(std::format("Compiling Project...{}", lc[rand() % strlen(lc)]), '\r');
                stream << line << "\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            c.wait();
            if (c.exit_code() != 0)
            {
                std::ofstream out{"build/compile-log.txt"};
                out<<stream.rdbuf();
                out.close();
                log("build failed read build/compile-log.txt for more info");
                return 1;
            }
            else
            {
                log("build success");
                if (argc >= 2)
                {
                    if (std::string(argv[1]) == "--run")
                    {
                        std::vector<std::string> args{};
                        for (int i = 2; i < argc; ++i)
                        {
                            args.push_back(std::string(argv[i]));
                        }
                        boost::process::child c(boost::process::search_path("build/debug/" + fs::current_path().filename().string()), boost::process::args(args));
                        c.wait();
                    }
                }
            };
        }
    }
    catch (std::exception &e)
    {
        log(e.what());
    }
    return 0;
};
