#include "includes.h"
//#include <catch2/catch.hpp>

// Mock sink that buffers messages and tracks when flush() is called
struct mock_http_sink : spdlog::sinks::http_sink<std::mutex> {
    explicit mock_http_sink(const std::string &url)
        : spdlog::sinks::http_sink<std::mutex>(url) {}
protected:
    // Override sink_it_: format the message and store it
    void sink_it_(const spdlog::details::log_msg &msg) override {
        spdlog::memory_buf_t formatted;
        base_sink<std::mutex>::formatter_->format(msg, formatted);
        buffer_.emplace_back(formatted.data(), formatted.size());
    }
    // Override flush_: mark that flush was called
    void flush_() override {
        flushed = true;
    }
public:
    bool flushed = false;
    size_t buffer_size() const {
        return buffer_.size();
    }
private:
    std::vector<std::string> buffer_;
};

TEST_CASE("http_sink buffers messages and flushes correctly", "[http_sink]") {
    // Create the mock sink and logger
    auto mock_sink = std::make_shared<mock_http_sink>("http://dummy");
    auto logger = std::make_shared<spdlog::logger>("test_logger", mock_sink);
    logger->set_level(spdlog::level::info);

    // Log two messages
    logger->info("First message");
    logger->info("Second message");

    // Verify buffer has 2 messages before flush
    REQUIRE(mock_sink->buffer_size() == 2);

    // Flush the logger (which calls mock_sink->flush_())
    logger->flush();
    REQUIRE(mock_sink->flushed);
}
