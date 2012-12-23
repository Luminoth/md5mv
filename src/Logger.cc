#include "pch.h"
#include "Logger.h"

static const std::string LOG_LEVELS[] = { "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL" };

boost::recursive_mutex Logger::logger_mutex;
boost::shared_ptr<Logger::ThreadSafeLoggerMap> Logger::_loggers;
uint32_t Logger::_logger_type = LoggerTypeStdout;
Logger::LogLevel Logger::_logger_level = LogLevelInfo;
boost::filesystem::path Logger::_logger_filename;
boost::shared_ptr<std::ofstream> Logger::_logger_file;

Logger& Logger::instance(const std::string& category)
{
    if(!_loggers) {
        _loggers.reset(new ThreadSafeLoggerMap());
    }

    boost::lock_guard<boost::recursive_mutex> guard(_loggers->mutex);

    boost::shared_ptr<Logger> logger;
    try {
        logger = _loggers->loggers.at(category);
    } catch(const std::out_of_range& ) {
        logger.reset(new Logger(category));
        _loggers->loggers[category] = logger;
    }
    return *logger;
}

bool Logger::configure(uint32_t type, LogLevel level, const boost::filesystem::path& filename)
{
    boost::lock_guard<boost::recursive_mutex> guard(logger_mutex);

    _logger_type = type;
    _logger_level = level;
    _logger_filename = filename;

    if(config_file()) {
        _logger_file.reset(new std::ofstream(_logger_filename.string().c_str(), std::ios::app));
        if(!(*_logger_file)) return false;
    }

    return true;
}

Logger::LogLevel Logger::level(const std::string& level)
{
    std::string scratch(boost::algorithm::to_lower_copy(level));
    if(level == "debug") {
        return LogLevelDebug;
    } else if(level == "info") {
        return LogLevelInfo;
    } else if(level == "warning") {
        return LogLevelWarning;
    } else if(level == "error") {
        return LogLevelError;
    } else if(level == "critical") {
        return LogLevelCritical;
    }
    return LogLevelInvalid;
}

const std::string& Logger::level(LogLevel level) throw(std::out_of_range)
{
    if(level <= LogLevelInvalid || level > LogLevelCritical) {
        throw std::out_of_range("Invalid log level!");
    }
    return LOG_LEVELS[level];
}

Logger::Logger(const std::string& category)
    : _category(category), _level(LogLevelInfo)
{
}

Logger::~Logger() throw()
{
}

Logger& Logger::operator<<(std::ostream& (*rhs)(std::ostream&))
{
    boost::lock_guard<boost::recursive_mutex> guard(logger_mutex);

    if(level() >= _logger_level) {
        if(config_stdout()) {
            if(level() >= LogLevelError) {
                std::cerr << rhs;
            } else {
                std::cout << rhs;
            }
        }

        if(config_file()) {
            logger_file() << rhs;
        }
    }

    return *this;
}

Logger& operator<<(Logger& lhs, const Logger::LogLevel& level)
{
    boost::lock_guard<boost::recursive_mutex> guard(Logger::logger_mutex);

    lhs._level = level;
    return lhs;
}
