#ifndef AFCONSTANTS_H
#define AFCONSTANTS_H

#include <string>

#ifndef AUTOFLIGHT_BUILD
#define AUTOFLIGHT_BUILD "unknown"
#endif

namespace autoflight
{
	const std::string SOFTWARE_VERSION = "Beta 1.0";
	const std::string BUILD_NUMBER     = AUTOFLIGHT_BUILD;
}

namespace error
{
	const int CONNECTION_FAILED   = -2;
	const int CONNECTION_LOST     = -3;
	const int NAVDATA_PARSE_ERROR = -4;
}

#endif
