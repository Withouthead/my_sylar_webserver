//
// Created by 79835 on 2022/2/4.
//

#include <iostream>
#include "log.h"

namespace sylar {
    void Logger::log(sylar::LogLevel::Level level, sylar::LogEvent::ptr event) {
        if (level < m_level)
            return;
        auto self = shared_from_this();
        for (auto &item: m_appenders) {
            item->log(self, level, event);
        }
    }

    void Logger::debug(sylar::LogEvent::ptr event) {
        log(LogLevel::Level::DEBUG, event);
    }

    void Logger::warn(LogEvent::ptr event) {
        log(LogLevel::Level::WARN, event);
    }

    void Logger::info(LogEvent::ptr event) {
        log(LogLevel::Level::INFO, event);
    }

    void Logger::fatal(LogEvent::ptr event) {
        log(LogLevel::Level::FATAL, event);
    }

    void Logger::addAppender(LogAppender::ptr appender) {
        m_appenders.insert(appender);
    }

    void Logger::delAppender(LogAppender::ptr appender) {
        m_appenders.erase(appender);
    }

    void Logger::error(LogEvent::ptr event) {
        log(LogLevel::Level::ERROR, event);
    }

    std::string Logger::getName() const {
        return m_name;
    }

    void Logger::setName(const std::string &name) {
        m_name = name;
    }

    void StdoutLogAppender::log(const std::shared_ptr<Logger> &logger, LogLevel::Level level, LogEvent::ptr event) {
        if (level >= m_level) {
            std::cout << m_formatter->format(logger, level, event);
        }
    }

    bool FileLogAppender::reopen() {
        if (m_filestream) {
            m_filestream.close();
        }

        m_filestream.open(m_file_name);
        return !!m_filestream;
    }

    void FileLogAppender::log(const std::shared_ptr<Logger> &logger, LogLevel::Level level, LogEvent::ptr event) {
        if (level >= m_level) {
            m_formatter->format(m_filestream, logger, level, event);
        }
    }

    class StringFormatItem : public LogFormatter::FormatItem {
    public:
        StringFormatItem(const std::string &str) : m_str(str) {}

        void
        format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << m_str;
        }

    private:
        std::string m_str;
    };

    class MessageFormatItem : public LogFormatter::FormatItem {
    public:
        MessageFormatItem(const std::string &str) {}

        void
        format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getContent();
        }
    };

    class ThreadIdFormatItem : public LogFormatter::FormatItem {
    public:
        ThreadIdFormatItem(const std::string &str) {}

        void
        format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getThreadId();
        }

    };

    class FiberIdFormatItem : public LogFormatter::FormatItem {
    public:
        FiberIdFormatItem(const std::string &str) {}

        void
        format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getFiberId();
        }
    };

    class LineNumFormatItem : public LogFormatter::FormatItem {
    public:
        LineNumFormatItem(const std::string &str) {}

        void
        format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getLine();
        }
    };

    class ElapseFormatItem : public LogFormatter::FormatItem {
    public:
        ElapseFormatItem(const std::string &str) {}

        void
        format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getElapse();
        }
    };

    class NewLineFormatItem : public LogFormatter::FormatItem {
    public:
        NewLineFormatItem(const std::string &str) {}

        void
        format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << std::endl;
        }
    };

    class FileNameFormatItem : public LogFormatter::FormatItem {
    public:
        FileNameFormatItem(const std::string &str) {}

        void
        format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getFileName();
        }
    };

    class LogNameFormatItem : public LogFormatter::FormatItem {
    public:
        LogNameFormatItem(const std::string &str) {}

        void
        format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getLogger()->getName();
        }
    };

    class TabFormatItem : public LogFormatter::FormatItem {
    public:
        TabFormatItem(const std::string &str) {}

        void
        format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << "\t";
        }
    };

    class LogLevelFormatItem : public LogFormatter::FormatItem {
    public:
        LogLevelFormatItem(const std::string &str) {}

        void
        format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << LogLevel::toString(level);
        }
    };

    class DateTimeFormatItem : public LogFormatter::FormatItem {
    public:
        DateTimeFormatItem(const std::string &time_format = "%Y-%m-%d %H:%M:%S") : m_time_format(time_format) {
            if (m_time_format.empty()) {
                m_time_format = "%Y-%m-%d %H:%M:%S";
            }
        }

        void
        format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            struct tm tm;
            time_t time = event->getTime();
            localtime_r(&time, &tm);
            char buf[64];
            strftime(buf, sizeof(buf), m_time_format.c_str(), &tm);
            os << buf;
        }

    private:
        std::string m_time_format;
    };

    class ThreadNameFormatItem : public LogFormatter::FormatItem {
    public:
        ThreadNameFormatItem(const std::string str) {}

        void
        format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getThreadName();
        }
    };


    LogFormatter::LogFormatter(const std::string &pattern) {
        m_pattern = pattern;
        init();
    }

    void LogFormatter::init() {
        std::vector<std::tuple<std::string, std::string, int>> vec;
        size_t last_pos = 0;
        std::string word_str;
        int fmt_version = 0;
        std::string fmt_flag;
        for (size_t i = 0; i < m_pattern.size(); i++) {
            if (m_pattern[i] != '%') {
                word_str += m_pattern[i];
                continue;
            }
            if (i + 1 < m_pattern.size() && m_pattern[i + 1] == '%') {
                word_str += '%';
                i++;
                continue;
            }

            size_t n = i + 1;
            fmt_flag = m_pattern[n];
            std::string fmt_msg;
            while (n < m_pattern.size()) {
                if (!fmt_version && !isalpha(m_pattern[n]) && m_pattern[n] != '{') {
                    fmt_flag = m_pattern.substr(i + 1, n - i - 1);
                    break;
                } else if (!fmt_version && m_pattern[n] == '{') {
                    fmt_flag = m_pattern.substr(i + 1, n - i - 1);
                    fmt_version = 1;
                } else if (fmt_version == 1) {
                    auto pos = m_pattern.find('}', n);
                    if (pos == std::string::npos) {
                        throw std::invalid_argument("log pattern format error: can't find ");
                    }
                    fmt_msg = m_pattern.substr(n, pos - n);

                }
                n++;
                if (n == m_pattern.size() && fmt_flag.empty()) {
                    fmt_flag = m_pattern.substr(i + 1);
                }
            }

            if (!word_str.empty()) {
                vec.push_back(std::make_tuple("", word_str, 0));
                word_str.clear();
            }

            if (!fmt_flag.empty())
                vec.push_back(std::make_tuple(fmt_flag, fmt_msg, 0));
        }
        if (!word_str.empty())
            vec.push_back(std::make_tuple("", word_str, 0));

        /*
        *%m 消息
        *  %p 日志级别
        *  %r 累计毫秒数
        *  %c 日志名称
        *  %t 线程id
        *  %n 换行
        *  %d 时间
        *  %f 文件名
        *  %l 行号
        *  %T 制表符
        *  %F 协程id
        *  %N 线程名称 */
        static std::unordered_map<std::string, std::function<FormatItem::ptr(
                const std::string &)>> s_format_items_map = {
#define  XX(str, C) \
            {#str, [](const std::string& (str)){return FormatItem::ptr(new C(str));}}
                XX(m, MessageFormatItem),
                XX(p, LogLevelFormatItem),
                XX(r, ElapseFormatItem),
                XX(c, LogNameFormatItem),
                XX(t, ThreadIdFormatItem),
                XX(n, NewLineFormatItem),
                XX(d, DateTimeFormatItem),
                XX(f, FileNameFormatItem),
                XX(l, LineNumFormatItem),
                XX(T, TabFormatItem),
                XX(F, FiberIdFormatItem),
                XX(N, ThreadNameFormatItem),
#undef XX

        };

        for (auto &item: vec) {
            auto iter = s_format_items_map.find(std::get<0>(item));
            if (iter != s_format_items_map.end()) {
                m_items.push_back(iter->second(std::get<1>(item)));
            } else {
                m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(item))));
            }

        }


    }

    std::string
    LogFormatter::format(const std::shared_ptr<Logger> &logger, LogLevel::Level level, LogEvent::ptr event) {

        std::stringstream ss;
        for (auto &item: m_items) {
            item->format(ss, logger, level, event);
        }
        return ss.str();

    }

    std::ostream &LogFormatter::format(std::ostream &os, const std::shared_ptr<Logger> &logger, LogLevel::Level level,
                                       LogEvent::ptr event) {
        for (auto &item: m_items) {
            item->format(os, logger, level, event);
        }
        return os;
    }

    const std::string &LogEvent::getFileName() const {
        return m_file_name;
    }

    void LogEvent::setFileName(const std::string &mFileName) {
        m_file_name = mFileName;
    }

    int32_t LogEvent::getLine() const {
        return m_line;
    }

    void LogEvent::setLine(int32_t mLine) {
        m_line = mLine;
    }

    uint32_t LogEvent::getElapse() const {
        return m_elapse;
    }

    void LogEvent::setElapse(uint32_t mElapse) {
        m_elapse = mElapse;
    }

    uint32_t LogEvent::getThreadId() const {
        return m_threadId;
    }

    void LogEvent::setThreadId(uint32_t mThreadId) {
        m_threadId = mThreadId;
    }

    uint32_t LogEvent::getFiberId() const {
        return m_fiberId;
    }

    void LogEvent::setFiberId(uint32_t mFiberId) {
        m_fiberId = mFiberId;
    }

    uint64_t LogEvent::getTime() const {
        return m_time;
    }

    void LogEvent::setTime(uint64_t mTime) {
        m_time = mTime;
    }

    std::string LogEvent::getContent() const {
        return m_content;
    }

    void LogEvent::setContent(const std::string &mContent) {
        m_content = mContent;
    }

    std::shared_ptr<Logger> LogEvent::getLogger() const {
        return m_logger;
    }

    void LogEvent::setLogger(std::shared_ptr<Logger> logger) {
        m_logger = logger;
    }

    std::string LogEvent::getThreadName() const {
        return m_thread_name;
    }

    void LogEvent::setThreadName(const std::string &thread_name) {
        m_thread_name = thread_name;
    }

}
