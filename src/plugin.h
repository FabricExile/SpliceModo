//
#ifndef SRC_PLUGIN_H_
#define SRC_PLUGIN_H_

// plugin version.
#define FABRICMODO_PLUGIN_VERSION   0.021

// disable some annoying VS warnings.
#pragma warning(disable : 4530)   // C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc.
#pragma warning(disable : 4800)   // forcing value to bool 'true' or 'false'.
#pragma warning(disable : 4806)   // unsafe operation: no value of type 'bool' promoted to type ...etc.

// includes.
#include <string>

// constants: Modo item/server names.
#define SERVER_NAME_JSONValue     "dfgJSONValue"  // the JSON custom type (used by the channels "FabricJSON").
#define SERVER_NAME_dfgModoIM     "dfgModoIM"     // dfgModo "item modifier".
#define SERVER_NAME_dfgModoPI     "dfgModoPI"     // dfgModo "procedural item".

// constants: fixed channel names.
#define CHN_NAME_IO_FabricActive  "FabricActive"  // io: enable/disable execution of DFG for this item.
#define CHN_NAME_IO_FabricEval    "FabricEval"    // io: internal counter used to re-evaluate the item.
#define CHN_NAME_IO_FabricDisplay "FabricDisplay" // io: (mesh item only) display mode.
#define CHN_NAME_IO_FabricOpacity "FabricOpacity" // io: (mesh item only) opacity.
#define CHN_NAME_IO_FabricJSON    "FabricJSON"    // io: custom value for persistence (read/write BaseInterface's JSON).

// forward declaration: log system.
#define LOG_SYSTEM_NAME "Fabric"
void feLog(void *userData, const char *s, unsigned int length);
void feLog(void *userData, const std::string &s);
void feLog(const std::string &s);
void feLogError(void *userData, const char *s, unsigned int length);
void feLogError(void *userData, const std::string &s);
void feLogError(const std::string &s);

#endif  // SRC_PLUGIN_H_

