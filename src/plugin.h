//
#ifndef SPLICE_MODO_PLUGIN
#define SPLICE_MODO_PLUGIN

// plugin version.
#define FABRICMODO_PLUGIN_VERSION   0.012

// disable some annoying VS warnings.
#pragma warning (disable : 4530)    // C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc.
#pragma warning (disable : 4800)    // forcing value to bool 'true' or 'false'.
#pragma warning (disable : 4806)    // unsafe operation: no value of type 'bool' promoted to type ...etc.

// includes.
#include "_class_BaseInterface.h"
#include "_class_FabricDFGWidget.h"
#include "_class_ModoTools.h"
#include "cmd_dfgExportJSON.h"
#include "cmd_dfgImportJSON.h"
#include "cmd_dfgLogVersion.h"
#include "cmd_dfgOpenCanvas.h"
#include "cmd_dfgStoreInChannel.h"
#include "itm_dfgModoIM.h"
#include "itm_dfgModoPI.h"

// further includes.
#include <fstream>
#include <streambuf>

// Modo item/server names.
#define SERVER_NAME_dfgModoIM       "dfgModoIM"     // dfgModo "item modifier".
#define SERVER_NAME_dfgModoPI       "dfgModoPI"     // dfgModo "procedural item".

// fixed channel names.
#define CHN_NAME_IO_FabricActive    "FabricActive"  // io: enable/disable execution of DFG for this item.
#define CHN_NAME_IO_FabricJSON      "FabricJSON"    // io: string for BaseInterface::getJSON() and BaseInterface::setFromJSON().

// log system.
#define LOG_SYSTEM_NAME "Fabric"
class CItemLog : public CLxLogMessage
{
    public:
    CItemLog() : CLxLogMessage(LOG_SYSTEM_NAME) {   }
    const char *GetFormat()     {   return "n.a.";  }
    const char *GetVersion()    {   return "n.a.";  };
    const char *GetCopyright()  {   return "n.a.";  };
};
extern CItemLog gLog;
void feLog(void *userData, const char *s, unsigned int length);
void feLog(void *userData, const std::string &s);
void feLogError(void *userData, const char *s, unsigned int length);
void feLogError(void *userData, const std::string &s);

#endif

