#pragma once

class Log
{
  public:
    void log(const char *fmt, ...);
    const char *pop();
    bool hasLog() const;

  private:
    static constexpr int LOG_SIZE = 32;
    static constexpr int LOG_ENTRY_SIZE = 128;

    char logQueue[LOG_SIZE][LOG_ENTRY_SIZE];
    int head = 0;
    int tail = 0;
    int count = 0;
};

//Global object
extern Log LOG;