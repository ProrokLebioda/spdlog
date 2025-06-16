#pragma once

#include <spdlog/common.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>

// Simple http sink
// Connects to an endpoint and sends the formatted log.
// Will attempt to reconnect if endpoint drops.


namespace spdlog {
namespace sinks {

struct http_sink_config {
};

template <typename Mutex>
class http_sink : public spdlog::sinks::base_sink<Mutex> 
{
public:
    explicit http_sink(http_sink_config config) 
    {
        config_ = move(config);
    }

    ~http_sink() override = default;

protected:
    void sink_it_(const spdlog::details::log_msg &msg) override {}

    void flush_() override {}

private:
    http_sink_config config_;
};

}  // namespace sinks
}  // namespace spdlog
