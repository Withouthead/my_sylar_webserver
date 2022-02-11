//
// Created by 79835 on 2022/2/4.
//

#ifndef SYLAR_WEB_SERVER_LOG_H
#define SYLAR_WEB_SERVER_LOG_H

#include <string>
#include <memory>
#include <list>
#include <unordered_set>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <unordered_map>
#include <functional>
#include <sstream>
#include <fmt/core.h>
#include <source_location>
#include "utils.h"

namespace sylar {

//日志级别
    class LogLevel {
    public:
        enum class Level {
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5
        };

        static std::string toString(Level level) {
            switch (level) {
                case Level::DEBUG :
                    return "DEBUG";
                case Level::INFO :
                    return "INFO";
                case Level::WARN :
                    return "WARN";
                case Level::ERROR:
                    return "ERROR";
                case Level::FATAL:
                    return "FATAL";
                default:
                    return "NONE";
            }
        }
    };


    class Logger;

//日志事件
    class LogEvent {
    public:
        typedef std::shared_ptr<LogEvent> ptr;

        LogEvent(const std::string &file_name, int32_t line, uint32_t elapse, uint32_t thread_id, uint32_t fiber_id,
                 uint64_t time) :
                m_file_name(file_name), m_line(line), m_elapse(elapse), m_threadId(thread_id), m_fiberId(fiber_id),
                m_time(time) {}

    private:
        std::string m_file_name;        //文件名
        int32_t m_line = 0;             //行号
        uint32_t m_elapse = 0;           //程序启动开始到现在的毫秒数
        uint32_t m_threadId = 0;        //线程号
        uint32_t m_fiberId = 0;         //协程号
        uint64_t m_time = 0;
        std::string m_thread_name;
        std::stringstream m_content_stream;
    public:
        // [[nodiscard]] 为不应该舍弃返回值，若舍弃返回值，编译器会warning
        [[nodiscard]] const std::string &getFileName() const;

        void setFileName(const std::string &mFileName);

        [[nodiscard]] int32_t getLine() const;

        void setLine(int32_t mLine);

        [[nodiscard]] uint32_t getElapse() const;

        void setElapse(uint32_t mElapse);

        [[nodiscard]] uint32_t getThreadId() const;

        void setThreadId(uint32_t mThreadId);

        [[nodiscard]] uint32_t getFiberId() const;

        void setFiberId(uint32_t mFiberId);

        [[nodiscard]] uint64_t getTime() const;

        void setTime(uint64_t mTime);

        [[nodiscard]] std::string getContent() const;

        void setContent(const std::string &content);

        [[nodiscard]] std::string getThreadName() const;

        void setThreadName(const std::string &thread_name);

        [[nodiscard]] std::stringstream& getContentStream();

    };

//日志格式
    class LogFormatter {
    public:
        typedef std::shared_ptr<LogFormatter> ptr;

        LogFormatter() : LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n") {}

        LogFormatter(const std::string &pattern);

        std::string format(const std::shared_ptr<Logger> &logger, LogLevel::Level level, LogEvent::ptr event);

        std::ostream &
        format(std::ostream &os, const std::shared_ptr<Logger> &logger, LogLevel::Level level, LogEvent::ptr event);

        void reset(const std::string &pattern);

    public:
        class FormatItem {
        public:
            typedef std::shared_ptr<FormatItem> ptr;

            virtual ~FormatItem() {}

            virtual void
            format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        };

        void init();

    private:
        std::string m_pattern;
        std::vector<FormatItem::ptr> m_items;
    };

//日志输出地
    class LogAppender {
    public:
        typedef std::shared_ptr<LogAppender> ptr;

        virtual ~LogAppender() {}

        virtual void log(const std::shared_ptr<Logger> &logger, LogLevel::Level level, LogEvent::ptr event) = 0;

        void setFormatter(LogFormatter::ptr val) { m_formatter = val; }

        LogFormatter::ptr getFormatter() { return m_formatter; }

    protected:
        LogLevel::Level m_level;
        LogFormatter::ptr m_formatter;
    };

    class Logger : public std::enable_shared_from_this<Logger> {
    public:
        typedef std::shared_ptr<Logger> ptr;

        Logger(const std::string &name = "root", LogLevel::Level level = LogLevel::Level::DEBUG,
               const std::string &log_format_pattern = "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n") :
                m_name(name), m_level(level) {

        }

        void log(LogLevel::Level level, LogEvent::ptr event);

        void debug(LogEvent::ptr event);

        void info(LogEvent::ptr event);

        void warn(LogEvent::ptr event);

        void error(LogEvent::ptr event);

        void fatal(LogEvent::ptr event);

        void addAppender(LogAppender::ptr appender);

        void delAppender(LogAppender::ptr appender);

        LogLevel::Level getLevel() const { return m_level; }

        void setLevel(LogLevel::Level level) { m_level = level; }

        std::string getName() const;

        void setName(const std::string &name);

    private:
        std::string m_name;
        LogLevel::Level m_level;
        std::unordered_set<LogAppender::ptr> m_appenders;
        LogFormatter::ptr log_formatter;
    };

    class StdoutLogAppender : public LogAppender {
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;

        void log(const std::shared_ptr<Logger> &logger, LogLevel::Level level, LogEvent::ptr event) override;
    };

    class FileLogAppender : public LogAppender {
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;

        void log(const std::shared_ptr<Logger> &logger, LogLevel::Level level, LogEvent::ptr event) override;

        bool reopen();

    private:
        std::string m_file_name;
        std::ofstream m_filestream;
    };

    class LoggerManager
    {
    public:

        // 单例模式
        static LoggerManager& getInstance()
        {
            static LoggerManager instance;
            return instance;
        }
        Logger::ptr getLogger(const std::string& name="root");
        Logger::ptr getRoot() const {return m_root;}
        void addLogger(const std::string& name);
    private:
        LoggerManager();
        std::unordered_map<std::string, Logger::ptr> m_loggers;
        Logger::ptr m_root;
    };





}
#endif //SYLAR_WEB_SERVER_LOG_H
