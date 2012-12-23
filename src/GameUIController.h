#if !defined __GAMEUICONTROLLER_H__
#define __GAMEUICONTROLLER_H__

#include "UIController.h"

class GameUIController : public UIController
{
public:
    static boost::shared_ptr<GameUIController> new_controller();

private:
    static Logger& logger;

public:
    GameUIController();
    virtual ~GameUIController() throw();

public:
    virtual void key_down(SDLKey key);
    virtual void update(double elapsed);
    virtual void render() const;

private:
    DISALLOW_COPY_AND_ASSIGN(GameUIController);
};

#endif