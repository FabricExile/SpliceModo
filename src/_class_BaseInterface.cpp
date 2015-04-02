#include "_class_BaseInterface.h"
#include "_class_ModoTools.h"

using namespace FabricServices;

FabricCore::Client BaseInterface::s_client;
DFGWrapper::Host * BaseInterface::s_host = NULL;
FabricServices::ASTWrapper::KLASTManager * BaseInterface::s_manager = NULL;
FabricServices::Commands::CommandStack BaseInterface::s_stack;
unsigned int BaseInterface::s_maxId = 0;
void (*BaseInterface::s_logFunc)(void *, const char *, unsigned int) = NULL;
void (*BaseInterface::s_logErrorFunc)(void *, const char *, unsigned int) = NULL;
std::map<unsigned int, BaseInterface*> BaseInterface::s_instances;

BaseInterface::BaseInterface()
{
  m_id                  = s_maxId++;
  m_item_obj_dfgModoIM  = NULL;

  // construct the client
  if(s_instances.size() == 0)
  {
    try
    {
      // create a client
      FabricCore::Client::CreateOptions options;
      memset( &options, 0, sizeof( options ) );
      options.optimizationType = FabricCore::ClientOptimizationType_Background;
      s_client = FabricCore::Client(&logFunc, NULL, &options);
      s_client.loadExtension("Math",     "", false);
      s_client.loadExtension("Geometry", "", false);

      // create a host for Canvas
      s_host = new DFGWrapper::Host(s_client);

      // create an empty binding
      m_binding = s_host->createBindingToNewGraph();

    // create KL AST manager
      s_manager = new ASTWrapper::KLASTManager(&s_client);

    // command stack
    s_stack;

    // set the graph on the view
      setGraph(m_binding.getGraph());
    }
    catch(FabricCore::Exception e)
    {
      logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    }
  }

  s_instances.insert(std::pair<unsigned int, BaseInterface*>(m_id, this));
}

BaseInterface::~BaseInterface()
{
  std::map<unsigned int, BaseInterface*>::iterator it = s_instances.find(m_id);

  m_binding = DFGWrapper::Binding();

  if(it != s_instances.end())
  {
    s_instances.erase(it);
    if(s_instances.size() == 0)
    {
      try
      {
        printf("Destructing client...\n");
    s_stack.clear();
    delete(s_manager);
        delete(s_host);
        s_client = FabricCore::Client();
      }
      catch(FabricCore::Exception e)
      {
        logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
      }
    }
  }
}

unsigned int BaseInterface::getId()
{
  return m_id;
}

BaseInterface * BaseInterface::getFromId(unsigned int id)
{
  std::map<unsigned int, BaseInterface*>::iterator it = s_instances.find(id);
  if(it == s_instances.end())
    return NULL;
  return it->second;
}

FabricCore::Client * BaseInterface::getClient()
{
  return &s_client;
}

FabricServices::DFGWrapper::Host * BaseInterface::getHost()
{
  return s_host;
}

FabricServices::DFGWrapper::Binding * BaseInterface::getBinding()
{
  return &m_binding;
}

FabricServices::ASTWrapper::KLASTManager * BaseInterface::getManager()
{
  return s_manager;
}

FabricServices::Commands::CommandStack * BaseInterface::getStack()
{
  return &s_stack;
}

std::string BaseInterface::getJSON()
{
  try
  {
    return m_binding.getGraph().exportJSON();
  }
  catch(FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
  }
  return "";
}

void BaseInterface::setFromJSON(const std::string & json)
{
  try
  {
    m_binding = s_host->createBindingFromJSON(json.c_str());
    setGraph(m_binding.getGraph());
  }
  catch(FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
  }
}

void BaseInterface::setLogFunc(void (*in_logFunc)(void *, const char *, unsigned int))
{
  s_logFunc = in_logFunc;
}

void BaseInterface::setLogErrorFunc(void (*in_logErrorFunc)(void *, const char *, unsigned int))
{
  s_logErrorFunc = in_logErrorFunc;
}

void BaseInterface::onPortInserted(FabricServices::DFGWrapper::Port port)
{
    std::string err;
    CLxUser_Item item((ILxUnknownID)m_item_obj_dfgModoIM);
    if (!item.test())
    {   err = "item((ILxUnknownID)m_item_obj_dfgModoIM) failed";
        logErrorFunc(0, err.c_str(), err.length());
        return;    }

    std::string dataType   = port.getDataType();
    std::string structType = "";

    if      (   dataType == "")         {   err = "port.getDataType() == \"\"";
                                            logErrorFunc(0, err.c_str(), err.length());
                                            return;    }

    else if (   dataType == "Boolean")  {   dataType = "boolean";                           }   // WIP

    else if (   dataType == "Integer"
             || dataType == "SInt8"
             || dataType == "SInt16"
             || dataType == "SInt32"
             || dataType == "SInt64"
             || dataType == "UInt8"
             || dataType == "UInt16"
             || dataType == "UInt32"
             || dataType == "UInt64")   {   dataType = "integer";                           }

    else if (   dataType == "Scalar"
             || dataType == "Float32"
             || dataType == "Float64")  {   dataType = "float";                             }

    else if (   dataType == "String")   {   dataType = "string";                            }

    else if (   dataType == "Quat")     {   dataType = "quaternion";                        }   // WIP

    else if (   dataType == "Mat44")    {   dataType = "matrix";                            }   // WIP

    else if (   dataType == "Vec2")     {   dataType = "float";     structType = "vecXY";   }   // WIP
    else if (   dataType == "Vec3")     {   dataType = "float";     structType = "vecXYZ";  }   // WIP

    else if (   dataType == "RGB")      {   dataType = "float";     structType = "vecRGB";  }   // WIP
    else if (   dataType == "RGBA")     {   dataType = "float";     structType = "vecRGBA"; }   // WIP

    else
    {
        err = "unable to create user channel, type \"" + dataType + "\" not yet implemented";
        logErrorFunc(0, err.c_str(), err.length());
        return;
    }

    if (!ModoTools::CreateUserChannel(&item, port.getName(), dataType, structType, err))
        logErrorFunc(0, err.c_str(), err.length());
}

void BaseInterface::onPortRemoved(FabricServices::DFGWrapper::Port port)
{
    std::string err;
    CLxUser_Item item((ILxUnknownID)m_item_obj_dfgModoIM);
    if (!item.test())
    {   err = "item((ILxUnknownID)m_item_obj_dfgModoIM) failed";
        logErrorFunc(0, err.c_str(), err.length());
        return;    }

  if (!ModoTools::DeleteUserChannel(&item, port.getName(), err))
    logErrorFunc(0, err.c_str(), err.length());
}

void BaseInterface::onPortRenamed(FabricServices::DFGWrapper::Port port, const char * oldName)
{
    std::string err;
    CLxUser_Item item((ILxUnknownID)m_item_obj_dfgModoIM);
    if (!item.test())
    {   err = "item((ILxUnknownID)m_item_obj_dfgModoIM) failed";
        logErrorFunc(0, err.c_str(), err.length());
        return;    }

  if (!ModoTools::RenameUserChannel(&item, std::string(oldName), port.getName(), err))
    logErrorFunc(0, err.c_str(), err.length());
}

void BaseInterface::logFunc(void * userData, const char * message, unsigned int length)
{
  if (s_logFunc)
  {
    s_logFunc(userData, message, length);
  }
  else
  {
    printf("BaseInterface: %s\n", message);
  }
}

void BaseInterface::logErrorFunc(void * userData, const char * message, unsigned int length)
{
  if (s_logErrorFunc)
  {
    s_logErrorFunc(userData, message, length);
  }
  else
  {
    printf("BaseInterface: error: %s\n", message);
  }
}

bool BaseInterface::HasInputPort(const char *portName)
{
    FabricServices::DFGWrapper::Port port = getGraph().getPort(portName);
    return (port.isValid() && port.getPortType() == FabricCore::DFGPortType_In);
}

bool BaseInterface::HasInputPort(const std::string &portName)
{
    return HasInputPort(portName.c_str());
}

bool BaseInterface::HasOutputPort(const char *portName)
{
    FabricServices::DFGWrapper::Port port = getGraph().getPort(portName);
    return (port.isValid() && port.getPortType() == FabricCore::DFGPortType_Out);
}

bool BaseInterface::HasOutputPort(const std::string &portName)
{
    return HasOutputPort(portName.c_str());
}

int BaseInterface::GetPortValueAsBoolean(FabricServices::DFGWrapper::Port &port, bool &out, bool strict)
{
    // init output.
    out = false;

    // invalid port?
    if (!port.isValid())
        return -2;

    // set out from port value.
    try
    {
        std::string dataType = port.getDataType();
        FabricCore::RTVal rtval = port.getRTVal();

        if      (dataType.length() == 0)    return -1;

        else if (dataType == "Boolean")     out = rtval.getBoolean();

        else if (!strict)
        {
            if      (dataType == "Float32") out = (0 != rtval.getFloat32());
            else if (dataType == "Float64") out = (0 != rtval.getFloat64());

            else if (dataType == "SInt8")   out = (0 != rtval.getSInt8());
            else if (dataType == "SInt16")  out = (0 != rtval.getSInt16());
            else if (dataType == "SInt32")  out = (0 != rtval.getSInt32());
            else if (dataType == "SInt64")  out = (0 != rtval.getSInt64());

            else if (dataType == "UInt8")   out = (0 != rtval.getUInt8());
            else if (dataType == "UInt16")  out = (0 != rtval.getUInt16());
            else if (dataType == "UInt32")  out = (0 != rtval.getUInt32());
            else if (dataType == "UInt64")  out = (0 != rtval.getUInt64());

            else return -1;
        }
        else return -1;
    }
    catch(FabricCore::Exception e)
    {
        return -4;
    }

    // done.
    return 0;
}

int BaseInterface::GetPortValueAsInteger(FabricServices::DFGWrapper::Port &port, int &out, bool strict)
{
    // init output.
    out = 0;

    // invalid port?
    if (!port.isValid())
        return -2;

    // set out from port value.
    try
    {
        std::string dataType = port.getDataType();
        FabricCore::RTVal rtval = port.getRTVal();

        if      (dataType.length() == 0)    return -1;

        else if (dataType == "SInt8")       out = (int)rtval.getSInt8();
        else if (dataType == "SInt16")      out = (int)rtval.getSInt16();
        else if (dataType == "SInt32")      out = (int)rtval.getSInt32();
        else if (dataType == "SInt64")      out = (int)rtval.getSInt64();

        else if (dataType == "UInt8")       out = (int)rtval.getUInt8();
        else if (dataType == "UInt16")      out = (int)rtval.getUInt16();
        else if (dataType == "UInt32")      out = (int)rtval.getUInt32();
        else if (dataType == "UInt64")      out = (int)rtval.getUInt64();

        else if (!strict)
        {
            if      (dataType == "Boolean") out = (int)rtval.getBoolean();

            else if (dataType == "Float32") out = (int)rtval.getFloat32();
            else if (dataType == "Float64") out = (int)rtval.getFloat64();

            else return -1;
        }
        else return -1;
    }
    catch(FabricCore::Exception e)
    {
        return -4;
    }

    // done.
    return 0;
}

int BaseInterface::GetPortValueAsFloat(FabricServices::DFGWrapper::Port &port, double &out, bool strict)
{
    // init output.
    out = 0;

    // invalid port?
    if (!port.isValid())
        return -2;

    // set out from port value.
    try
    {
        std::string dataType = port.getDataType();
        FabricCore::RTVal rtval = port.getRTVal();

        if      (dataType.length() == 0)    return -1;

        else if (dataType == "Float32")     out = (double)rtval.getFloat32();
        else if (dataType == "Float64")     out = (double)rtval.getFloat64();

        else if (!strict)
        {
            if      (dataType == "Boolean") out = (double)rtval.getBoolean();

            else if (dataType == "SInt8")   out = (double)rtval.getSInt8();
            else if (dataType == "SInt16")  out = (double)rtval.getSInt16();
            else if (dataType == "SInt32")  out = (double)rtval.getSInt32();
            else if (dataType == "SInt64")  out = (double)rtval.getSInt64();

            else if (dataType == "UInt8")   out = (double)rtval.getUInt8();
            else if (dataType == "UInt16")  out = (double)rtval.getUInt16();
            else if (dataType == "UInt32")  out = (double)rtval.getUInt32();
            else if (dataType == "UInt64")  out = (double)rtval.getUInt64();

            else return -1;
        }
        else return -1;
    }
    catch(FabricCore::Exception e)
    {
        return -4;
    }

    // done.
    return 0;
}

int BaseInterface::GetPortValueAsString(FabricServices::DFGWrapper::Port &port, std::string &out, bool strict)
{
    // init output.
    out = "";

    // invalid port?
    if (!port.isValid())
        return -2;

    // set out from port value.
    try
    {
        std::string dataType = port.getDataType();
        FabricCore::RTVal rtval = port.getRTVal();

        if      (dataType.length() == 0)    return -1;

        else if (dataType == "String")      out = rtval.getStringCString();

        else if (!strict)
        {
            char    s[64];
            bool    b;
            int     i;
            double  f;

            if (GetPortValueAsBoolean(port, b, true) == 0)
            {
                out = (b ? "true" : "false");
                return 0;
            }

            if (GetPortValueAsInteger(port, i, true) == 0)
            {
                sprintf(s, "%ld", i);
                out = s;
                return 0;
            }

            if (GetPortValueAsFloat(port, f, true) == 0)
            {
                sprintf(s, "%f", f);
                out = s;
                return 0;
            }

            return -1;
        }
        else
            return -1;
    }
    catch(FabricCore::Exception e)
    {
        return -4;
    }

    // done.
    return 0;
}

int BaseInterface::GetPortValueAsQuaternion(FabricServices::DFGWrapper::Port &port, std::vector <double> &out, bool strict)
{
    // init output.
    out.clear();

    // invalid port?
    if (!port.isValid())
        return -2;

    // set out from port value.
    try
    {
        std::string dataType = port.getDataType();
        FabricCore::RTVal rtval = port.getRTVal();

        if      (dataType.length() == 0)        return -1;

        else if (dataType == "Quat")        {
                                                return -3;
                                            }
        else
            return -1;
    }
    catch(FabricCore::Exception e)
    {
        return -4;
    }

    // done.
    return 0;
}

int BaseInterface::GetPortValueAsMatrix44(FabricServices::DFGWrapper::Port &port, std::vector <double> &out, bool strict)
{
    // init output.
    out.clear();

    // invalid port?
    if (!port.isValid())
        return -2;

    // set out from port value.
    try
    {
        std::string dataType = port.getDataType();
        FabricCore::RTVal rtval = port.getRTVal();

        if      (dataType.length() == 0)        return -1;

        else if (dataType == "Mat44")       {
                                                char member[32];
                                                FabricCore::RTVal rtRow;
                                                for (int i=0;i<4;i++)
                                                {
                                                    sprintf(member, "row%ld", i);
                                                    rtRow = rtval.maybeGetMember(member);
                                                    out.push_back(rtRow.maybeGetMember("x").getFloat64());
                                                    out.push_back(rtRow.maybeGetMember("y").getFloat64());
                                                    out.push_back(rtRow.maybeGetMember("z").getFloat64());
                                                    out.push_back(rtRow.maybeGetMember("t").getFloat64());
                                                }
                                            }
        else
            return -1;
    }
    catch(FabricCore::Exception e)
    {
        return -4;
    }

    // done.
    return 0;
}

