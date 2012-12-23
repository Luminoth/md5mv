#include "pch.h"
#include <fstream>
#include <iostream>
#include "string_util.h"
#include "util.h"
#include "Configuration.h"

Configuration::Configuration()
    : _dirty(false)
{
    set_default("logging", "level", "info");
    set_default("logging", "stdout", "true");
    set_default("logging", "file", "false");
    set_default("logging", "filename", "");

    load_defaults();
}

Configuration::~Configuration() throw()
{
}

void Configuration::generate_header(const std::string& header)
{
    std::stringstream str;
    str << "# " << header << std::endl
        << "#" << std::endl
        << "# Supported options:" << std::endl
        << "#" << std::endl;

    BOOST_FOREACH(const ConfigSections::value_type& section, _sections) {
        str << "#\t[" << section.first << "]" << std::endl;

        BOOST_FOREACH(const ConfigOptions::value_type& option, section.second) {
            str << "#\t" << option.second.name << " = <" << option.second.help << "> "
                << "(default " << option.second.default_value << ")" << std::endl;
        }
        str << "#" << std::endl;
    }

    _header = str.str();
}

void Configuration::set_default(const std::string& section, const std::string& option, const std::string& value)
{
    ConfigOption& opt = this->option(section, option);
    opt.default_value = value;
}

void Configuration::set_defaults(const std::string& section, const std::list<Configuration::ConfigOption>& options)
{
    BOOST_FOREACH(const ConfigOption& opt, options) {
        set_default(section, opt.name, opt.default_value);
    }
}

void Configuration::set_defaults(const Configuration::ConfigSections& sections)
{
    BOOST_FOREACH(const ConfigSections::value_type& section, sections) {
        std::list<ConfigOption> options;
        BOOST_FOREACH(const ConfigOptions::value_type& option, section.second) {
            options.push_back(option.second);
        }
        set_defaults(section.first, options);
    }
}

void Configuration::load_defaults()
{
    BOOST_FOREACH(ConfigSections::value_type& section, _sections) {
        BOOST_FOREACH(ConfigOptions::value_type& option, section.second) {
            option.second.value = option.second.default_value;
            update_map(section.first, option.second);
        }
    }
    _dirty = true;
}

void Configuration::set(const std::string& section, const std::string& option, const std::string& value)
{
    ConfigOption& opt = this->option(section, option);
    opt.value = value;
    update_map(section, opt);
    _dirty = true;
}

std::string Configuration::get(const std::string& section, const std::string& option) const
{
    ConfigSections::const_iterator sit = _sections.find(section);
    if(sit == _sections.end()) {
        return std::string();
    }

    ConfigOptions::const_iterator oit = sit->second.find(option);
    if(oit == sit->second.end())
        return std::string();

    return oit->second.value;
}


const std::string& Configuration::lookup(const std::string& section, const std::string& option) const throw(NoSuchConfigOptionError)
{
    ConfigSections::const_iterator sit = _sections.find(section);
    if(sit == _sections.end()) {
        throw NoSuchConfigOptionError("Config section does not exist: " + section);
    }

    ConfigOptions::const_iterator oit = sit->second.find(option);
    if(oit == sit->second.end()) {
        throw NoSuchConfigOptionError("Config option does not exist: " + option);
    }

    return oit->second.value;
}

bool Configuration::load(const boost::filesystem::path& filename)
{
    std::ifstream infile(filename.string().c_str());
    if(!infile) return false;

    std::string section;
    std::string scratch;
    while(!infile.eof()) {
        std::getline(infile, scratch);
        if(scratch.empty()) continue;

        // strip comments
        size_t pos = scratch.find('#');
        if(pos != std::string::npos) {
            scratch.erase(pos);
        }

        // strip the ends whitespace
        // this is more to get rid of
        // lazy space after comment removal
        boost::trim_right(scratch);
        if(scratch.empty()) continue;

        // check for a section
        std::string temp(boost::trim_left_copy(scratch));
        if(temp[0] == '[') {
            pos = temp.find(']');
            if(pos == std::string::npos) {
                return false;
            }

            section = temp.substr(1, pos-1);
            continue;
        }

        // we shouldn't be here without a section
        if(section == "") {
            return false;
        }

        // check for a name/value pair
        pos = scratch.find('=');
        if(pos == std::string::npos) {
            return false;
        }

        std::string option(trim_all(scratch.substr(0, pos)));
        std::string value(boost::trim_copy(scratch.substr(pos + 1)));

        set(section, option, value);

        // TODO: check for duplicates so we can warn the user
        //std::cerr << "WARNING: Ignoring duplicate value - [" << section << "] " << option << " = " << value << std::endl;
    }

    clean();
    return true;
}

bool Configuration::save(const boost::filesystem::path& filename)
{
    std::ofstream outfile(filename.string().c_str());
    if(!outfile) return false;

    on_save();

    outfile << _header << std::endl;
    dump(outfile);
    outfile.close();

    clean();
    return true;
}

bool Configuration::exists(const boost::filesystem::path& filename) const
{
    return boost::filesystem::exists(filename);
}

void Configuration::dump(Logger& logger) const
{
    std::stringstream str;
    str << "Configuration:" << std::endl;
    dump(str);
    LOG_INFO(str.str() << std::endl);
}

void Configuration::dump(std::ostream& out) const
{
    BOOST_FOREACH(const ConfigSections::value_type& section, _sections) {
        out << "[" << section.first << "]" << std::endl;
        BOOST_FOREACH(const ConfigOptions::value_type& option, section.second) {
            out << trim_all(option.second.name) << " = " << boost::trim_copy(option.second.value) << std::endl;
        }
        out << std::endl;
    }
}

void Configuration::register_listener(const std::string& section, const std::string& option, ConfigListener& listener)
{
    _listeners[section + "." + option] = listener;
}

uint32_t Configuration::logging_type() const
{
    uint32_t type = Logger::LoggerTypeNone;

    if(to_boolean(get("logging", "stdout"))) {
        type |= Logger::LoggerTypeStdout;
    }

    if(to_boolean(get("logging", "file"))) {
        type |= Logger::LoggerTypeFile;
    }

    return type;
}

Logger::LogLevel Configuration::logging_level() const
{
    return Logger::level(get("logging", "level"));
}

void Configuration::validate() const throw(ConfigurationError)
{
    try {
        Logger::level(get("logging", "level"));
    } catch(const std::out_of_range&) {
        throw ConfigurationError("Logging level must be a valid level!");
    }
}

Configuration::ConfigOptions& Configuration::section(const std::string& section)
{
    try {
        return _sections.at(section);
    } catch(const std::out_of_range&) {
        return (_sections[section] = ConfigOptions());
    }
}

Configuration::ConfigOption& Configuration::option(const std::string& section, const std::string& option)
{
    ConfigOptions& options = this->section(section);
    try {
        return options.at(option);
    } catch(const std::out_of_range&) {
        return (options[option] = ConfigOption(option));
    }
}

void Configuration::update_map(const std::string& section, const ConfigOption& option)
{
    std::string key(section + "." + option.name);

    _map[key] = option.value;

    try {
        ConfigListener& listener = _listeners.at(key);
        listener(section, option.name, option.value);
    } catch(const std::out_of_range&) {
    }
}
