#pragma once

class SplashInterface : public Interface
{
public:
    void Display() override;
    void Render() override;
public:
	void Toggle(bool state = false) { m_bSplashAllowed = state; };
private:
	bool m_bSplashAllowed;
};