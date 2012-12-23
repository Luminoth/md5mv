#if !defined __UICONTROLLER_H__
#define __UICONTROLLER_H__

class UIController
{
private:
    static boost::shared_ptr<UIController> current_controller;
    static std::stack<boost::shared_ptr<UIController> > controller_stack;

public:
    static boost::shared_ptr<UIController> controller() { return current_controller; }
    static void controller(boost::shared_ptr<UIController> controller) { current_controller = controller; }
    static void push_controller() { controller_stack.push(current_controller); }
    static void pop_controller() { current_controller = controller_stack.top(); controller_stack.pop(); }

public:
    virtual ~UIController() throw();

public:
    void handle_events();
    void handle_input();

public:
    virtual void key_down(SDLKey key) = 0;
    virtual void update(double elapsed) = 0;
    virtual void render() const = 0;

protected:
    UIController();

private:
    DISALLOW_COPY_AND_ASSIGN(UIController);
};

#endif