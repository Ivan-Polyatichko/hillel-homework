#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <algorithm>

struct LogSink {
    virtual void write(const std::string& msg) = 0;
    virtual ~LogSink() = default;
};

class ConsoleSink : public LogSink {
public:
    void write(const std::string& msg) override {
        std::cout << "[Console] " << msg << std::endl;
    }
};

class FileSink : public LogSink {
public:
    void write(const std::string& msg) override {
        std::ofstream file("app.log", std::ios::app);
        if (file.is_open()) {
            file << "[File] " << msg << std::endl;
        }
    }
};

class NullSink : public LogSink {
public:
    void write(const std::string&) override {
     }
};

enum class SinkType { CONSOLE, FILE, NONE };

class Logger {
public:
    static Logger& instance() {
        static Logger instance;
        return instance;
    }

    void set_sink(SinkType type) {
        switch (type) {
        case SinkType::CONSOLE:
            sink_ = std::make_unique<ConsoleSink>();
            std::cout << "Sink set to CONSOLE.\n";
            return;

        case SinkType::FILE:
            sink_ = std::make_unique<FileSink>();
            std::cout << "Sink set to FILE.\n";
            return;

        case SinkType::NONE:
            sink_ = std::make_unique<NullSink>();
            std::cout << "Sink set to NONE.\n";
            return;
        }

        std::cerr << "Unknown sink type.\n";
    }

    void log(const std::string& msg) {
        if (sink_) sink_->write(msg);
    }

private:
    Logger() = default;
    std::unique_ptr<LogSink> sink_;

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

std::string to_lower(const std::string& input) {
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return result;
}

SinkType parse_sink_type(const std::string& arg) {
    std::string type = to_lower(arg);
    if (type == "console") return SinkType::CONSOLE;
    if (type == "file") return SinkType::FILE;
    if (type == "none") return SinkType::NONE;

    std::cerr << "Unknown sink type: " << arg << ". Falling back to CONSOLE.\n";
    return SinkType::CONSOLE;
}

int main(int argc, char* argv[]) {
    SinkType selected_sink = SinkType::CONSOLE;

    if (argc > 1) {
        selected_sink = parse_sink_type(argv[1]);
    }
    else {
        std::cout << "No sink type specified. Defaulting to CONSOLE.\n";
        selected_sink = SinkType::CONSOLE;
    }
    Logger::instance().set_sink(selected_sink);
    Logger::instance().log("Test message 1");
    Logger::instance().log("Test message 2");

    return 0;
}
