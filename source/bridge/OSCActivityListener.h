#pragma once

class OSCActivityListener
{
public:
    virtual ~OSCActivityListener() = default;
    virtual void newOSCActivity (float activityValue) = 0;
};
