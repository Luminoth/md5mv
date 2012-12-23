#if !defined __ENGINE_H__
#define __ENGINE_H__

class Engine
{
public:
    static Engine& instance();

private:
    static Logger& logger;

public:
    virtual ~Engine() throw();

public:
    bool init(int argc, char** argv);
    void run();
    void shutdown();

public:
    void quit() { _quit = true; }
    bool should_quit() const { return _quit; }

    double runtime() const;
    double frame_time() const;

    double average_fps() const;
    double current_fps() const;

private:
    void event_loop();
    void rate_limit();

private:
    bool _quit;
    double _start_time;
    uint64_t _frame_count;
    double _frame_start;

private:
    Engine();
    DISALLOW_COPY_AND_ASSIGN(Engine);
};

#endif
