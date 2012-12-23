#if !defined __CONFIGURATION_H__
#define __CONFIGURATION_H__

class Logger;

class ConfigurationError : public std::exception
{
public:
    explicit ConfigurationError(const std::string& what) throw() : _what(what) {}
    virtual ~ConfigurationError() throw() {}
    virtual const char* what() const throw() { return _what.c_str(); }

private:
    std::string _what;
};

class NoSuchConfigOptionError : public ConfigurationError
{
public:
    explicit NoSuchConfigOptionError(const std::string& what) throw() : ConfigurationError(what) {}
    virtual ~NoSuchConfigOptionError() throw() {}
};

/*
Overriding classes should include the following static method:

    static Configuration& instance();

that should manage the configuration as a singleton. No constructors should available.
*/
class Configuration
{
public:
    typedef boost::unordered_map<std::string, std::string> ConfigMap;
    typedef boost::function<void (const std::string&, const std::string&, const std::string&)> ConfigListener;
    typedef boost::unordered_map<std::string, ConfigListener> ConfigListenerMap;

    typedef ConfigMap::iterator iterator;
    typedef ConfigMap::const_iterator const_iterator;
    typedef ConfigMap::value_type value_type;

private:
    struct ConfigOption
    {
        std::string name;
        std::string default_value;
        std::string value;
        std::string help;

        ConfigOption(const std::string& n=std::string(), const std::string& dv=std::string())
            : name(n), default_value(dv), help(n)
        {
        }
    };

    typedef boost::unordered_map<std::string, ConfigOption> ConfigOptions;
    typedef boost::unordered_map<std::string, ConfigOptions> ConfigSections;

public:
    virtual ~Configuration() throw();

public:
    // generates a header from the default values
    void generate_header(const std::string& header);

    // sets a default option value
    // this will create a new option if the requestion one doesn't exist
    void set_default(const std::string& section, const std::string& option, const std::string& value="");

    // sets all default values for a section
    // uses the default_value parameter of the options
    void set_defaults(const std::string& section, const std::list<Configuration::ConfigOption>& options);

    // sets all default values
    void set_defaults(const ConfigSections& sections);

    // loads the default options
    // into the current set
    void load_defaults();

    // sets a config value
    // this will create a new option if the requested one doesn't exist
    void set(const std::string& section, const std::string& option, const std::string& value="");

    // looks up a config value
    // returns an empty string if the value doesn't exist
    std::string get(const std::string& section, const std::string& option) const;

    // looks up a config value
    // throws NoSuchConfigOptionError if not found
    const std::string& lookup(const std::string& section, const std::string& option) const throw(NoSuchConfigOptionError);

    // reads a config from a file
    bool load(const boost::filesystem::path& filename);

    // writes the config to a file
    bool save(const boost::filesystem::path& filename);

    // returns true if the config file exits
    bool exists(const boost::filesystem::path& filename) const;

    // dumps the config to a log file
    void dump(Logger& logger) const;

    // dumps the config to an output stream
    void dump(std::ostream& out) const;

    // returns whether or not the config is dirty (unsaved)
    bool dirty() const { return _dirty; }

    // forces a clean on the config without saving
    void clean() { _dirty = false; }

    // returns the configuration in a map
    // [section.option] = value
    const ConfigMap& map() const { return _map; }

    // NOTE: this will blindly overwrite existing listeners
    void register_listener(const std::string& section, const std::string& option, ConfigListener& listener);

    uint32_t logging_type() const;
    Logger::LogLevel logging_level() const;
    const boost::filesystem::path logging_filename() const { return get("logging", "filename"); }

    iterator begin() { return _map.begin(); }
    iterator end() { return _map.end(); }
    const_iterator begin() const { return _map.begin(); }
    const_iterator end() const { return _map.end(); }

public:
    // call super-class validate()
    virtual void validate() const throw(ConfigurationError);
    virtual void on_save() {}

private:
    ConfigOptions& section(const std::string& section);
    ConfigOption& option(const std::string& section, const std::string& option);
    void update_map(const std::string& section, const ConfigOption& option);

protected:
    Configuration();
    DISALLOW_COPY_AND_ASSIGN(Configuration);

private:
    ConfigSections _sections;
    ConfigMap _map;
    ConfigListenerMap _listeners;
    std::string _header;
    bool _dirty;
};

#endif
