#include <Inspector/InspectorPCH.h>

#include <Foundation/Communication/Telemetry.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <Inspector/MainWindow.moc.h>
#include <Inspector/PluginsWidget.moc.h>

nsQtPluginsWidget* nsQtPluginsWidget::s_pWidget = nullptr;

nsQtPluginsWidget::nsQtPluginsWidget(QWidget* pParent)
  : ads::CDockWidget("Plugins Widget", pParent)
{
  s_pWidget = this;

  setupUi(this);
  setWidget(TablePlugins);

  setIcon(QIcon(":/Icons/Icons/Plugin.svg"));

  ResetStats();
}

void nsQtPluginsWidget::ResetStats()
{
  m_bUpdatePlugins = true;
  m_Plugins.Clear();
}


void nsQtPluginsWidget::UpdateStats()
{
  UpdatePlugins();
}

void nsQtPluginsWidget::UpdatePlugins()
{
  if (!m_bUpdatePlugins)
    return;

  m_bUpdatePlugins = false;

  nsQtScopedUpdatesDisabled _1(TablePlugins);

  TablePlugins->clear();

  TablePlugins->setRowCount(m_Plugins.GetCount());

  QStringList Headers;
  Headers.append("");
  Headers.append(" Plugin ");
  Headers.append(" Reloadable ");
  Headers.append(" Dependencies ");

  TablePlugins->setColumnCount(static_cast<int>(Headers.size()));

  TablePlugins->setHorizontalHeaderLabels(Headers);

  {
    nsStringBuilder sTemp;
    nsInt32 iRow = 0;

    for (nsMap<nsString, PluginsData>::Iterator it = m_Plugins.GetIterator(); it.IsValid(); ++it)
    {
      QLabel* pIcon = new QLabel();
      QIcon icon = nsQtUiServices::GetCachedIconResource(":/Icons/Icons/Plugin.svg");
      pIcon->setPixmap(icon.pixmap(QSize(24, 24)));
      pIcon->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
      TablePlugins->setCellWidget(iRow, 0, pIcon);

      sTemp.SetFormat("  {0}  ", it.Key());
      TablePlugins->setCellWidget(iRow, 1, new QLabel(sTemp.GetData()));

      if (it.Value().m_bReloadable)
        TablePlugins->setCellWidget(iRow, 2, new QLabel("<p><span style=\"font-weight:600; color:#00aa00;\">  Yes  </span></p>"));
      else
        TablePlugins->setCellWidget(iRow, 2, new QLabel("<p><span style=\"font-weight:600; color:#ffaa00;\">  No  </span></p>"));

      ((QLabel*)TablePlugins->cellWidget(iRow, 2))->setAlignment(Qt::AlignHCenter);

      TablePlugins->setCellWidget(iRow, 3, new QLabel(it.Value().m_sDependencies.GetData()));

      ++iRow;
    }
  }

  TablePlugins->resizeColumnsToContents();
}

void nsQtPluginsWidget::ProcessTelemetry(void* pUnuseed)
{
  if (!s_pWidget)
    return;

  nsTelemetryMessage Msg;

  while (nsTelemetry::RetrieveMessage('PLUG', Msg) == NS_SUCCESS)
  {
    switch (Msg.GetMessageID())
    {
      case ' CLR':
      {
        s_pWidget->m_Plugins.Clear();
        s_pWidget->m_bUpdatePlugins = true;
      }
      break;

      case 'DATA':
      {
        nsString sName;

        Msg.GetReader() >> sName;

        PluginsData& pd = s_pWidget->m_Plugins[sName.GetData()];
        Msg.GetReader() >> pd.m_bReloadable;

        Msg.GetReader() >> pd.m_sDependencies;

        s_pWidget->m_bUpdatePlugins = true;
      }
      break;
    }
  }
}