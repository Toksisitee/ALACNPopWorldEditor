#pragma once

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

// The build increment is expected to be overridden by the continuous build system.
#ifndef BUILD_INCREMENT
#define BUILD_INCREMENT		0
#endif

#define APPNAME					"Populous World Editor 1.8.0"

// Determine the version number constants.
#define ENGINE_MAJOR_VERSION		1
#define ENGINE_MINOR_VERSION		8
#define ENGINE_REVISION_NUMBER		0
#define ENGINE_BUILD_NUMBER		BUILD_INCREMENT

#define VER_FILE_VERSION            ENGINE_MAJOR_VERSION, ENGINE_MINOR_VERSION, ENGINE_REVISION_NUMBER, ENGINE_BUILD_NUMBER
#define VER_FILE_VERSION_STR        STRINGIZE(ENGINE_MAJOR_VERSION) \
                                    "," STRINGIZE(ENGINE_MINOR_VERSION) \
                                    "," STRINGIZE(ENGINE_REVISION_NUMBER) \
                                    "," STRINGIZE(ENGINE_BUILD_NUMBER) \

#define POPEDT_VERSION			STRINGIZE(ENGINE_MAJOR_VERSION) \
                                    "." STRINGIZE(ENGINE_MINOR_VERSION) \
                                    "." STRINGIZE(ENGINE_REVISION_NUMBER) \
                                    "." STRINGIZE(ENGINE_BUILD_NUMBER)
