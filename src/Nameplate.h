#if !defined __NAMEPLATE_H__
#define __NAMEPLATE_H__

#include "Vector.h"

class Camera;

class Nameplate
{
public:
    Nameplate();
    virtual ~Nameplate() throw();

public:
    void render(const std::string& name, const Position& position, const Camera& camera) const;

private:
    DISALLOW_COPY_AND_ASSIGN(Nameplate);
};

#endif