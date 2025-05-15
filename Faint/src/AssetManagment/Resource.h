#pragma once

#include "Common/UUID.h"

namespace Faint {
	class Resource {
	public:
		UUID id;

		bool IsEmbedded = false;
		std::string Path;

		//void MakeExternal();
		//void Duplicate();
		//void MakeEmbedded();
	};
}