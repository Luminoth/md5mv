#if !defined __LOGGER_H__
#define __LOGGER_H__

// TODO: get rid of these includes here by switching to a filter model
#include <iostream>
#include <fstream>

/*<< __FILE__ << "[" << __LINE__ << "] " \*/

#define LOG_DEBUG(e) do { \
    boost::lock_guard<boost::recursive_mutex> guard(Logger::logger_mutex); \
    logger << Logger::LogLevelDebug \
        << boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time()) << " " \
        << "[" << boost::this_thread::get_id() << "] " \
        << logger.category() << " " \
        << Logger::level(Logger::LogLevelDebug) << ": " \
        << e; \
} while(false)

#define LOG_INFO(e) do { \
    boost::lock_guard<boost::recursive_mutex> guard(Logger::logger_mutex); \
    logger << Logger::LogLevelInfo \
        << boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time()) << " " \
        << "[" << boost::this_thread::get_id() << "] " \
        << logger.category() << " " \
        << Logger::level(Logger::LogLevelInfo) << ": " \
        << e; \
} while(false)

#define LOG_WARNING(e) do { \
    boost::lock_guard<boost::recursive_mutex> guard(Logger::logger_mutex); \
    logger << Logger::LogLevelWarning \
        << boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time()) << " " \
        << "[" << boost::this_thread::get_id() << "] " \
        << logger.category() << " " \
        << Logger::level(Logger::LogLevelWarning) << ": " \
        << e; \
} while(false)

#define LOG_ERROR(e) do { \
    boost::lock_guard<boost::recursive_mutex> guard(Logger::logger_mutex); \
    logger << Logger::LogLevelError \
        << boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time()) << " " \
        << "[" << boost::this_thread::get_id() << "] " \
        << logger.category() << " " \
        << Logger::level(Logger::LogLevelError) << ": " \
        << e; \
} while(false)

#define LOG_CRITICAL(e) do { \
    boost::lock_guard<boost::recursive_mutex> guard(Logger::logger_mutex); \
    logger << Logger::LogLevelCritical \
        << boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time()) << " " \
        << "[" << boost::this_thread::get_id() << "] " \
        << logger.category() << " " \
        << Logger::level(Logger::LogLevelCritical) << ": " \
        << e; \
} while(false)

class Logger
{
private:
    typedef boost::unordered_map<std::string, boost::shared_ptr<Logger> > LoggerMap;
    struct ThreadSafeLoggerMap
    {
        boost::recursive_mutex mutex;
        LoggerMap loggers;
    };

public:
    enum LoggerTypeMask
    {
        LoggerTypeNone = 0,
        LoggerTypeStdout = 1,
        LoggerTypeFile = 2,
    };

    enum LogLevel
    {
        LogLevelInvalid = -1,
        LogLevelDebug,
        LogLevelInfo,
        LogLevelWarning,
        LogLevelError,
        LogLevelCritical,
    };

public:
    static boost::recursive_mutex logger_mutex;

private:
    static boost::shared_ptr<ThreadSafeLoggerMap> _loggers;
    static uint32_t _logger_type;
    static LogLevel _logger_level;
    static boost::filesystem::path _logger_filename;
    static boost::shared_ptr<std::ofstream> _logger_file;

public:
    static Logger& instance(const std::string& category);
    static bool configure(uint32_t type, LogLevel level, const boost::filesystem::path& filename);
    static bool config_stdout() { return (_logger_type & LoggerTypeStdout) == LoggerTypeStdout; }
    static bool config_file() { return (_logger_type & LoggerTypeFile) == LoggerTypeFile; }
    //static bool config_level() { return _logger_level; }

    static LogLevel level(const std::string& level);
    static const std::string& level(LogLevel level) throw(std::out_of_range);

private:
    static std::ofstream& logger_file() { return *_logger_file; }

public:
    virtual ~Logger() throw();

public:
    const std::string& category() const { return _category; }
    LogLevel level() const { return _level; }

public:
    Logger& operator<<(std::ostream& (*rhs)(std::ostream&));
    friend Logger& operator<<(Logger& lhs, const LogLevel& level);

    template<typename T> friend Logger& operator<<(Logger& lhs, const T& rhs)
    {
        boost::lock_guard<boost::recursive_mutex> guard(Logger::logger_mutex);

        if(lhs.level() >= _logger_level) {
            if(Logger::config_stdout()) {
                if(lhs.level() >= Logger::LogLevelError) {
                    std::cerr << rhs;
                } else {
                    std::cout << rhs;
                }
            }

            if(Logger::config_file()) {
                Logger::logger_file() << rhs;
            }
        }
        return lhs;
    }

private:
    std::string _category;
    LogLevel _level;

private:
    Logger();
    explicit Logger(const std::string& category);
    DISALLOW_COPY_AND_ASSIGN(Logger);
};

#endif
