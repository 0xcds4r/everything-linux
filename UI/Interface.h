#pragma once

class Interface
{
public:
    virtual ~Interface() = default;
    virtual void Display() = 0; 
    virtual void Render() = 0;
};