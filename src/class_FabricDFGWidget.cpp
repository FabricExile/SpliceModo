
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>

#include "plugin.h"
#include "class_FabricDFGWidget.h"

std::string FabricDFGWidget::s_currentUINodeName;

FabricDFGWidget::FabricDFGWidget(QWidget * parent, BaseInterface *baseInterface)
:QSplitter(parent)
{
  m_baseInterface  = baseInterface;
  m_treeWidget     = NULL;
  m_dfgWidget      = NULL;
  m_dfgValueEditor = NULL;

  try
  {
    m_baseInterfaceName = s_currentUINodeName;
	BaseInterface * interf = m_baseInterface;

    if(interf)
    {
      DFGWrapper::Binding binding = *interf->getBinding();

      DFG::DFGConfig config;

	  DFGWrapper::GraphExecutable subGraph = binding.getGraph();

      m_treeWidget = new DFG::PresetTreeWidget(this, interf->getHost(), config);
      m_dfgWidget = new DFG::DFGWidget(this, interf->getClient(), interf->getManager(), interf->getHost(), binding, subGraph, interf->getStack(), config, FabricUI::GraphView::GraphConfig(), false);
      m_dfgValueEditor = new DFG::DFGValueEditor(this, m_dfgWidget->getUIController(), config);
      m_dfgWidget->getUIController()->setLogFunc(&FabricDFGWidget::log);

      setContentsMargins(0, 0, 0, 0);

      addWidget(m_treeWidget);
      addWidget(m_dfgWidget);
      //addWidget(m_dfgValueEditor);

      m_dfgWidget->getUIGraph()->defineHotkey(Qt::Key_Delete, Qt::NoModifier, "delete");
      m_dfgWidget->getUIGraph()->defineHotkey(Qt::Key_Backspace, Qt::NoModifier, "delete2");
      m_dfgWidget->getUIGraph()->defineHotkey(Qt::Key_F, Qt::NoModifier, "frameSelected");
      m_dfgWidget->getUIGraph()->defineHotkey(Qt::Key_A, Qt::NoModifier, "frameAll");
      m_dfgWidget->getUIGraph()->defineHotkey(Qt::Key_Tab, Qt::NoModifier, "tabSearch");

      QObject::connect(m_dfgValueEditor, SIGNAL(valueChanged(ValueItem*)), this, SLOT(onValueChanged()));
      QObject::connect(m_dfgWidget->getUIController(), SIGNAL(structureChanged()), this, SLOT(onStructureChanged()));
      QObject::connect(m_dfgWidget->getUIController(), SIGNAL(recompiled()), this, SLOT(onRecompilation()));
      QObject::connect(m_dfgWidget->getUIController(), SIGNAL(portRenamed(QString, QString)), this, SLOT(onPortRenamed(QString, QString)));
      QObject::connect(m_dfgWidget->getUIGraph(), SIGNAL(hotkeyPressed(Qt::Key, Qt::KeyboardModifier, QString)), 
        this, SLOT(hotkeyPressed(Qt::Key, Qt::KeyboardModifier, QString)));
      QObject::connect(m_dfgWidget->getUIGraph(), SIGNAL(nodeDoubleClicked(FabricUI::GraphView::Node*)), 
        this, SLOT(onNodeDoubleClicked(FabricUI::GraphView::Node*)));
      // QObject::connect(m_dfgWidget, SIGNAL(nodeEditRequested(QString)), 
      //   this, SLOT(onNodeEditRequested(QString)));
    }
  }
  catch(FabricCore::Exception e)
  {
    //mayaLogErrorFunc(e.getDesc_cstr());
    return;
  }
  catch(FabricSplice::Exception e)
  {
    //mayaLogErrorFunc(e.what());
    return;
  }

  resize(1000, 500);
  QList<int> s = sizes();
  // s[0] = 200;
  // s[1] = 800;
  // s[2] = 0;
  s[0] = 0;
  s[1] = 1000;
  s[2] = 0;
  setSizes(s);
}

FabricDFGWidget::~FabricDFGWidget()
{
}

QWidget * FabricDFGWidget::creator(QWidget * parent, BaseInterface *baseInterface, const QString & name)
{
	return new FabricDFGWidget(parent, baseInterface);
}

void FabricDFGWidget::setCurrentUINodeName(const char * node)
{
  if(node)
    s_currentUINodeName = node;
}

void FabricDFGWidget::onValueChanged()
{
  onRecompilation();
}

void FabricDFGWidget::onStructureChanged()
{
  onValueChanged();
}

void FabricDFGWidget::onRecompilation()
{
	BaseInterface * interf = m_baseInterface;
  if(interf)
  {
    //interf->invalidateNode();
  }
}

void FabricDFGWidget::onPortRenamed(QString path, QString newName)
{
}

void FabricDFGWidget::hotkeyPressed(Qt::Key key, Qt::KeyboardModifier modifiers, QString hotkey)
{
  if(hotkey == "delete" || hotkey == "delete2")
  {
    std::vector<GraphView::Node *> nodes = m_dfgWidget->getUIGraph()->selectedNodes();
    m_dfgWidget->getUIController()->beginInteraction();
    for(size_t i=0;i<nodes.size();i++)
      m_dfgWidget->getUIController()->removeNode(nodes[i]);
    m_dfgWidget->getUIController()->endInteraction();
  }
  else if(hotkey == "frameSelected")
  {
    m_dfgWidget->getUIController()->frameSelectedNodes();
  }
  else if(hotkey == "frameAll")
  {
    m_dfgWidget->getUIController()->frameAllNodes();
  }
  else if(hotkey == "tabSearch")
  {
    QPoint pos = m_dfgWidget->getGraphViewWidget()->lastEventPos();
    pos = m_dfgWidget->getGraphViewWidget()->mapToGlobal(pos);
    m_dfgWidget->getTabSearchWidget()->showForSearch(pos);
  }
}

void FabricDFGWidget::onNodeDoubleClicked(FabricUI::GraphView::Node * node)
{
  DFGWrapper::Node codeNode = m_dfgWidget->getUIController()->getNodeFromPath(node->path().toUtf8().constData());
  //m_dfgValueEditor->setNode(codeNode);

  QList<int> s = sizes();
  if(s[2] == 0)
  {
    s[2] = (int)(float(s[1]) * 0.2f);
    s[1] = (int)(float(s[1]) * 0.8f);
    setSizes(s);
  }
}

void FabricDFGWidget::log(const char * message)
{
  //mayaLogFunc(message);
}
