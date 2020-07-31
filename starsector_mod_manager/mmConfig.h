#ifndef MM_CONFIG
#define MM_CONFIG

#include <fstream>
#include <filesystem>

#include "json.hpp"

using namespace nlohmann;
namespace fs = std::filesystem;

class mmConfig : public json {
public:
	mmConfig();

	bool initialise();

	bool apply();
};

#endif // !MM_CONFIG
