#pragma once

#include <spdlog/common.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>

#include <mutex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <iostream> // TODO: Replace by proper endpoint

// Simple http sink
// Connects to an endpoint and sends the formatted log.
// Will attempt to reconnect if endpoint drops.


namespace spdlog {
namespace sinks {

struct http_sink_config {
    std::string endpoint_url;
    std::chrono::seconds flush_interval{5};
    spdlog::level::level_enum level_threshold{spdlog::level::info};

    http_sink_config(std::string url,
                     std::chrono::seconds flush = std::chrono::seconds(5),
                     spdlog::level::level_enum level = spdlog::level::info)
        : endpoint_url(std::move(url)),
          flush_interval(flush),
          level_threshold(level) {}
};

template <typename Mutex>
class http_sink : public spdlog::sinks::base_sink<Mutex> 
{
public:
    explicit http_sink(http_sink_config config) 
        : config_(std::move(config))
    {
    }

    ~http_sink() override = default;

protected:
    // Allows custom sink behavior
    void sink_it_(const spdlog::details::log_msg &msg) override 
    {
        if (msg.level < config_.level_threshold)
            return;

        memory_buf_t formatted;
        this->formatter_->format(msg, formatted);
        std::string log_entry = fmt::to_string(formatted);

        {
            std::lock_guard<std::mutex> lock(buffer_mutex_);
            buffer_.push_back(std::move(log_entry));
        }
    }

    void flush_() override { send_batch_(); }

private:
    void send_batch_() 
    {
        std::vector<std::string> batch;
       
        {
            std::lock_guard<std::mutex> lock(buffer_mutex_);
            if (buffer_.empty()) return;
            batch.swap(buffer_);
        }

        std::ostringstream oss;
        for (const auto& msg : batch)
        {
            oss << msg << "\n";
        }

        std::string payload = oss.str();

        try
        {
            // TODO: Replace later with real http POST
            std::cout << "[httt_sink] Posting to " << config_.endpoint_url << ":\n"
                      << payload << std::endl;

            throw std::runtime_error("Simulated network failure");
        }
        catch (const std::exception& ex)
        {
            std::cerr << "[http_sink] Failed to send logs: " << ex.what() << std::endl;

            std::lock_guard<std::mutex> lock(buffer_mutex_);
            buffer_.insert(buffer_.begin(), batch.begin(), batch.end());
        }
    }

private:
    http_sink_config config_;

    std::vector<std::string> buffer_;
    std::mutex buffer_mutex_;
};

using http_sink_mt = http_sink<std::mutex>;
using http_sink_st = http_sink<spdlog::details::null_mutex>;

}  // namespace sinks
}  // namespace spdlog
