
#include <fstream>

/*
    Logger logger("logfile.txt"); 
    logger.log(INFO, "Program started.");
    logger.log(DEBUG, "Debugging information.");
    logger.log(ERROR, "An error occurred.");
*/

enum LogLevel { DEBUG, INFO, WARNING, ERROR, CRITICAL };

class Logger {
public:
    Logger(const std::string& filename);

    ~Logger();

    void log(LogLevel level, const std::string& message);

private:
    std::ofstream logFile; 
    std::string levelToString(LogLevel level);
};
