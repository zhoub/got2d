#pragma once
#include <g2dengine.h>

class Engine : public g2d::Engine
{
public:
	virtual bool Update(unsigned long elapsedTime) override;
	virtual void Release() override;
};