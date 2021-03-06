#include "plugin.h"

#include "_class_DFGUICmdHandlerDCC.h"
#include "_class_BaseInterface.h"
#include "_class_FabricDFGWidget.h"
#include "_class_FabricView.h"
#include "_class_JSONValue.h"
#include "_class_ModoTools.h"
#include "cmd_FabricCanvasExportGraph.h"
#include "cmd_FabricCanvasGetResult.h"
#include "cmd_FabricCanvasImportGraph.h"
#include "cmd_FabricCanvasIncEval.h"
#include "cmd_FabricCanvasLogVersion.h"
#include "cmd_FabricCanvasOpenCanvas.h"
#include "itm_CanvasIM.h"
#include "itm_CanvasPI.h"

// log system.
class CItemLog : public CLxLogMessage
{
 public:
    CItemLog() : CLxLogMessage(LOG_SYSTEM_NAME) { }
    const char *GetFormat()     { return "n.a."; }
    const char *GetVersion()    { return "n.a."; }
    const char *GetCopyright()  { return "n.a."; }
};
CItemLog gLog;
void dccLogMessage(const uint32_t severity, const std::string &prefix, const std::string &message)
{
  if (message.length() <= 1000)
    gLog.Message(severity, prefix.c_str(), message.c_str(), " ");
  else
  {
    std::string cropped = message.substr(0, 1000) + " [long message, only first 1000 characters displayed]";
    gLog.Message(severity, prefix.c_str(), cropped.c_str(), " ");
  }
}
void feLog(void *userData, const char *s, unsigned int length)
{
  const char *p = (s != NULL ? s : "s == NULL");
  dccLogMessage(LXe_INFO, "[FABRIC]", p);
  FabricUI::DFG::DFGLogWidget::log(p);
}
void feLog(void *userData, const std::string &s)
{
  feLog(userData, s.c_str(), s.length());
}
void feLog(const std::string &s)
{
  feLog(NULL, s.c_str(), s.length());
}
void feLogError(void *userData, const char *s, unsigned int length)
{
  const char *p = (s != NULL ? s : "s == NULL");
  dccLogMessage(LXe_FAILED, "[FABRIC ERROR]", p);
  std::string t = p;
  t = "Error: " + t;
  FabricUI::DFG::DFGLogWidget::log(t.c_str());
}
void feLogError(void *userData, const std::string &s)
{
  feLogError(userData, s.c_str(), s.length());
}
void feLogError(const std::string &s)
{
  feLogError(NULL, s.c_str(), s.length());
}
void feLogDebug(void *userData, const char *s, unsigned int length)
{
  feLog(userData, s, length);
}
void feLogDebug(void *userData, const std::string &s)
{
  feLog(userData, s);
}
void feLogDebug(const std::string &s)
{
  feLog(s);
}
void feLogDebug(const std::string &s, int number)
{
  char t[64];
  sprintf(t, " number = %d", number);
  feLog(s + t);
}

namespace Surf_Sample { void initialize(); };
namespace Value       { void initialize(); };

// plugin initialization.
void initialize()
{
  // Fabric.
  {
    // set log function pointers.
    BaseInterface::setLogFunc(feLog);
    BaseInterface::setLogErrorFunc(feLogError);

    // set the client persistence flag.
    char const *no_client_persistence = ::getenv( "FABRIC_DISABLE_CLIENT_PERSISTENCE" );
    BaseInterface::setPersistClient(!no_client_persistence || no_client_persistence[0] == '\0');
  }

  // Modo.
  {
    FabricCanvasExportGraph :: Command:: initialize();
    FabricCanvasGetResult   :: Command:: initialize();
    FabricCanvasImportGraph :: Command:: initialize();
    FabricCanvasIncEval     :: Command:: initialize();
    FabricCanvasLogVersion  :: Command:: initialize();
    FabricCanvasOpenCanvas  :: Command:: initialize();
    //
    CanvasIM                          :: initialize();
    CanvasPI                          :: initialize();
    //
    Surf_Sample                       :: initialize();
    Value                             :: initialize();
    //
    JSONValue                         :: initialize();
    FabricView                        :: initialize();
    //
    FabricCanvasRemoveNodes           :: initialize();
    FabricCanvasConnect               :: initialize();
    FabricCanvasDisconnect            :: initialize();
    FabricCanvasAddGraph              :: initialize();
    FabricCanvasAddFunc               :: initialize();
    FabricCanvasInstPreset            :: initialize();
    FabricCanvasAddVar                :: initialize();
    FabricCanvasAddGet                :: initialize();
    FabricCanvasAddSet                :: initialize();
    FabricCanvasAddPort               :: initialize();
    FabricCanvasAddInstPort           :: initialize();
    FabricCanvasAddInstBlockPort      :: initialize();
    FabricCanvasCreatePreset          :: initialize();
    FabricCanvasEditPort              :: initialize();
    FabricCanvasRemovePort            :: initialize();
    FabricCanvasMoveNodes             :: initialize();
    FabricCanvasResizeBackDrop        :: initialize();
    FabricCanvasImplodeNodes          :: initialize();
    FabricCanvasExplodeNode           :: initialize();
    FabricCanvasAddBackDrop           :: initialize();
    FabricCanvasSetNodeComment        :: initialize();
    FabricCanvasSetCode               :: initialize();
    FabricCanvasEditNode              :: initialize();
    FabricCanvasRenamePort            :: initialize();
    FabricCanvasPaste                 :: initialize();
    FabricCanvasSetArgValue           :: initialize();
    FabricCanvasSetPortDefaultValue   :: initialize();
    FabricCanvasSetRefVarPath         :: initialize();
    FabricCanvasReorderPorts          :: initialize();
    FabricCanvasSetExtDeps            :: initialize();
    FabricCanvasSplitFromPreset       :: initialize();
    FabricCanvasDismissLoadDiags      :: initialize();
    FabricCanvasAddBlock              :: initialize();
    FabricCanvasAddBlockPort          :: initialize();
    FabricCanvasAddNLSPort            :: initialize();
    FabricCanvasReorderNLSPorts       :: initialize();
  }
}

// plugin clean up.
void cleanup()
{
}

