#include "JobSystem.h"
#include <iostream>

using namespace Faint;

void JobSystem::Update() {

	for (auto it = jobs.begin(); it != jobs.end();)
	{
		if (it->get()->IsDone())
		{
			it->get()->End();
			it = jobs.erase(it);
		}
		else
		{
			++it;
		}
	}
}