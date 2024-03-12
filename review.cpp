#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <variant>
#include <vector>

using namespace std;

using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
using MapValueType = std::variant<std::string, TimePoint>;
using MapType = std::unordered_map<std::string, MapValueType>;

std::string serialise_time_point(const std::string &format, const TimePoint &tp)
{
    std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::gmtime(&tt); // GMT (UTC)
    std::stringstream ss;
    ss << std::put_time(&tm, format.c_str());
    return ss.str();
}

[[nodiscard]] MapType get_config(const std::string_view &filename)
{
    // Pretend we do some disk operation and parsing here
    return {
        {"greeting", MapValueType{"Hello world!"}}};
}

MapType* process_config_updates(MapType* myconfig)
{
    myconfig->insert_or_assign("last_run", MapValueType{std::chrono::system_clock::now()});
    return myconfig;
}

void worker(MapType myconfig)
{
    // Worker function to do something interesting and update the config.
    myconfig = *process_config_updates(&myconfig);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void maintain_tasks(std::vector<std::thread> &tasks, MapType &myconfig)
{
    if (tasks.empty()) [[unlikely]]
        return;

    std::vector<std::thread>::iterator iter;
    for (iter = std::begin(tasks); iter != std::end(tasks);)
    {
        if (iter->joinable()) [[unlikely]]
        {
            auto tp = std::get<TimePoint>(myconfig["last_run"]);
            std::cout << "Task completed: "
                      << "last_run=" << serialise_time_point("%H:%M:%S", tp) << "\n";
            iter->join();
            iter = tasks.erase(iter);
        }
        else
            ++iter;
    }
}

int main()
{
    auto myconfig = get_config("configfile.toml");
    myconfig.insert({"last_run", MapValueType{TimePoint{}}});

    auto tasks = std::vector<std::thread>{};

    for (int i = 0; i < 5; i++)
    {
        tasks.emplace_back(worker, myconfig);

        maintain_tasks(tasks, myconfig);
    }

    // Cleanup
    while (not tasks.empty())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        maintain_tasks(tasks, myconfig);
    }
    return 0;
}