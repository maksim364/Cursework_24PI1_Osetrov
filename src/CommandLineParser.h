#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <string>
#include <boost/program_options.hpp>

class CommandLineParser {
private:
    int port;
    std::string user_db_file;
    std::string log_file;
    
public:
    CommandLineParser();
    
    bool parse(int argc, char* argv[]);
    bool validate() const;
    
    int get_port() const { return port; }
    std::string get_user_db_file() const { return user_db_file; }
    std::string get_log_file() const { return log_file; }
};

#endif // COMMANDLINEPARSER_H
