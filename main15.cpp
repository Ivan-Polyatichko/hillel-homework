#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <functional>
#include <cstdint>

struct INumberReader {
    virtual ~INumberReader() = default;
    virtual std::vector<int> read_numbers(const std::string& filename) = 0;
};

struct INumberFilter {
    virtual ~INumberFilter() = default;
    virtual bool keep(int number) = 0;
};

struct INumberObserver {
    virtual ~INumberObserver() = default;
    virtual void on_number(int number) = 0;
    virtual void on_finished() = 0;
};

class FileNumberReader : public INumberReader {
public:
    std::vector<int> read_numbers(const std::string& filename) override {
        std::ifstream in(filename);
        if (!in.is_open()) {
            std::cout << "Error: File not found: " << filename << "\n";
            return {};
        }

        std::vector<int> numbers;
        int n;
        while (in >> n) {
            numbers.push_back(n);
        }
        return numbers;
    }
};

class EvenFilter : public INumberFilter {
public:
    bool keep(int number) override {
        return number % 2 == 0;
    }
};

class OddFilter : public INumberFilter {
public:
    bool keep(int number) override {
        return number % 2 != 0;
    }
};

class GTFilter : public INumberFilter {
    int threshold;
public:
    GTFilter(int n) : threshold(n) {}
    bool keep(int number) override {
        return number > threshold;
    }
};

class FilterFactory {
    using Creator = std::function<std::unique_ptr<INumberFilter>(const std::string&)>;
    std::map<std::string, Creator> registry;

public:
    static FilterFactory& instance() {
        static FilterFactory factory;
        return factory;
    }

    void register_filter(const std::string& prefix, Creator creator) {
        registry[prefix] = creator;
    }

    std::unique_ptr<INumberFilter> create(const std::string& name) {
        for (const auto& [prefix, creator] : registry) {
            if (name.starts_with(prefix)) {
                return creator(name.substr(prefix.size()));
            }
        }

        std::cout << "Error: Unknown filter: " << name << "\n";
        return nullptr;
    }

private:
    FilterFactory() = default;
};

class PrintObserver : public INumberObserver {
public:
    void on_number(int number) override {
        std::cout << "Number passed: " << number << "\n";
    }

    void on_finished() override {
        std::cout << "Processing finished.\n";
    }
};

class CountObserver : public INumberObserver {
    int count = 0;
public:
    void on_number(int number) override {
        ++count;
    }

    void on_finished() override {
        std::cout << "Total passed numbers: " << count << "\n";
    }
};

class NumberProcessor {
    INumberReader& reader;
    INumberFilter& filter;
    std::vector<INumberObserver*> observers;

public:
    NumberProcessor(INumberReader& r, INumberFilter& f, const std::vector<INumberObserver*>& obs)
        : reader(r), filter(f), observers(obs) {
    }

    void run(const std::string& filename) {
        auto numbers = reader.read_numbers(filename);
        for (int n : numbers) {
            if (filter.keep(n)) {
                for (auto* obs : observers) {
                    obs->on_number(n);
                }
            }
        }
        for (auto* obs : observers) {
            obs->on_finished();
        }
    }
};

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "Usage: ./number_pipeline <FILTER> <FILE>\n";
        std::cout << "Example filters: EVEN, ODD, GT5\n";
        return 1;
    }

    std::string filter_name = argv[1];
    std::string file_name = argv[2];

    FilterFactory::instance().register_filter("EVEN", [](const std::string&) {
        return std::make_unique<EvenFilter>();
        });

    FilterFactory::instance().register_filter("ODD", [](const std::string&) {
        return std::make_unique<OddFilter>();
        });

    FilterFactory::instance().register_filter("GT", [](const std::string& param) -> std::unique_ptr<INumberFilter> {
        try {
            if (param.empty()) throw std::invalid_argument("Missing value");
            int n = std::stoi(param);
            return std::make_unique<GTFilter>(n);
        }
        catch (...) {
            std::cout << "Error: GT filter requires a numeric value, e.g., GT5\n";
            return nullptr;
        }
        });

    auto filter = FilterFactory::instance().create(filter_name);
    if (!filter) return 1;

    FileNumberReader reader;
    PrintObserver printer;
    CountObserver counter;

    std::vector<INumberObserver*> observers = { &printer, &counter };

    NumberProcessor processor(reader, *filter, observers);
    processor.run(file_name);

    return 0;
}
