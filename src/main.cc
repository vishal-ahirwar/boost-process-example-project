#include <iostream>
#include <boost-process-example-projectconfig.h>
#include <boost/process.hpp>
#include <filesystem>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fstream>
std::ofstream out("log.txt");
char lc[] = {"/\\-+"};
namespace fs = std::filesystem;

void log(const std::string_view &msg, const char &end = '\n')
{
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::white), "{:<5}: ", "Info");
    fmt::print(fmt::emphasis::faint | fmt::fg(fmt::color::ghost_white), "{}{}", msg, end);
};
void logFile(const std::string_view &msg)
{
    out << msg << "\n";
};

int main(int argc, char *argv[])
{
    srand(time(0));
    try
    {
        if (!fs::exists("build"))
        {
        build:
            boost::process::ipstream ss_out, ss_err; // Create separate streams for output and error
            std::stringstream stream;
            boost::process::child c(boost::process::search_path("cmake"), "--preset=windows", "-Bbuild/debug",
                                    boost::process::std_out > ss_out, boost::process::std_err > ss_err); // Redirect std_err too

            std::string line_out{}, line_err{};
            while (c.running() && (std::getline(ss_out, line_out) || std::getline(ss_err, line_err)))
            {
                log(std::format("Generating Config...{}", lc[rand() % strlen(lc)]), '\r');
                if (!line_out.empty())
                {
                    logFile(line_out);
                }
                if (!line_err.empty())
                {
                    logFile(line_err);
                }
            }

            log("");
            c.wait();

            if (c.exit_code() != 0)
            {

                log(std::format("Generating Config failed. Read log.txt for more info"));
                return 1;
            }
            goto compile;
        }
        else
        {
        compile:

            boost::process::ipstream ss_out, ss_err; // Separate streams for output and error
            boost::process::child c(boost::process::search_path("cmake"), "--build", "build/debug",
                                    boost::process::std_out > ss_out, boost::process::std_err > ss_err);

            std::string line_out{}, line_err{};
            std::stringstream stream;
            while (c.running() && (std::getline(ss_out, line_out) || std::getline(ss_err, line_err)))
            {
                log(std::format("Compiling Project...{}", lc[rand() % strlen(lc)]), '\r');
                if (!line_out.empty())
                {
                    logFile(line_out);
                }
                if (!line_err.empty())
                {
                    logFile(line_err);
                }
            }
            c.wait();
            log("");
            if (c.exit_code() != 0)
            {
                log("Build failed. Read log.txt for more info.");
                return 1;
            }
            else
            {
                log("Build success!");
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
