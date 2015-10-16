#include "plugin.h"

#include "_class_DFGUICmdHandlerDCC.h"
#include "_class_BaseInterface.h"
#include "_class_FabricDFGWidget.h"
#include "_class_FabricView.h"
#include "_class_JSONValue.h"
#include "_class_ModoTools.h"
#include "cmd_FabricCanvasExportGraph.h"
#include "cmd_FabricCanvasImportGraph.h"
#include "cmd_FabricCanvasIncEval.h"
#include "cmd_FabricCanvasLogVersion.h"
#include "cmd_FabricCanvasOpenCanvas.h"
#include "itm_CanvasIM.h"
#include "itm_CanvasPI.h"
#include "itm_CanvasPIpilot.h"

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
void feLog(void *userData, const char *s, unsigned int length)
{
  const char *p = (s != NULL ? s : "s == NULL");
  gLog.Message(LXe_INFO, "[FABRIC]", p, " ");
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
  gLog.Message(LXe_FAILED, "[FABRIC ERROR]", p, " ");
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

// a global BaseInterface: its only purpose is to ensure
// that Fabric is "up and running" when the DCC is executed
// (this avoids the long initialization times when creating
// the first base interface).
BaseInterface *gblBaseInterface_dummy = NULL;

// plugin initialization.
void initialize()
{
  // this can't be initialized outside as its constructor adds to
  // an s_instances global and we can't guarantee the order of
  // global initialization
  gblBaseInterface_dummy = new BaseInterface;

  // Fabric.
  {
    // set log function pointers.
    BaseInterface::setLogFunc(feLog);
    BaseInterface::setLogErrorFunc(feLogError);
  }

  // Modo.
  {
    FabricCanvasExportGraph :: Command:: initialize();
    FabricCanvasImportGraph :: Command:: initialize();
    FabricCanvasIncEval     :: Command:: initialize();
    FabricCanvasLogVersion  :: Command:: initialize();
    FabricCanvasOpenCanvas  :: Command:: initialize();
    //
    CanvasIM                :: initialize();
    CanvasPI                :: initialize();
    CanvasPIpilot           :: initialize();
    //
    JSONValue               :: initialize();
    FabricView              :: initialize();
    //
    FabricCanvasRemoveNodes          :: initialize();
    FabricCanvasConnect              :: initialize();
    FabricCanvasDisconnect           :: initialize();
    FabricCanvasAddGraph             :: initialize();
    FabricCanvasAddFunc              :: initialize();
    FabricCanvasInstPreset           :: initialize();
    FabricCanvasAddVar               :: initialize();
    FabricCanvasAddGet               :: initialize();
    FabricCanvasAddSet               :: initialize();
    FabricCanvasAddPort              :: initialize();
    FabricCanvasEditPort             :: initialize();
    FabricCanvasRemovePort           :: initialize();
    FabricCanvasMoveNodes            :: initialize();
    FabricCanvasResizeBackDrop       :: initialize();
    FabricCanvasImplodeNodes         :: initialize();
    FabricCanvasExplodeNode          :: initialize();
    FabricCanvasAddBackDrop          :: initialize();
    FabricCanvasSetNodeTitle         :: initialize();
    FabricCanvasSetNodeComment       :: initialize();
    FabricCanvasSetCode              :: initialize();
    FabricCanvasRenamePort           :: initialize();
    FabricCanvasPaste                :: initialize();
    FabricCanvasSetArgType           :: initialize();
    FabricCanvasSetArgValue          :: initialize();
    FabricCanvasSetExtDeps           :: initialize();
    FabricCanvasSetPortDefaultValue  :: initialize();
    FabricCanvasSetRefVarPath        :: initialize();
    FabricCanvasSplitFromPreset      :: initialize();
    FabricCanvasReorderPorts         :: initialize();
  }
}

// plugin clean up.
void cleanup()
{
  delete gblBaseInterface_dummy;
}

