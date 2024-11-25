#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Serialization/ReflectionSerializer.h>
#include <Foundation/Serialization/RttiConverter.h>
#include <Foundation/Strings/TranslationLookup.h>
#include <Foundation/Types/VariantTypeRegistry.h>
#include <GuiFoundation/PropertyGrid/DefaultState.h>
#include <GuiFoundation/PropertyGrid/Implementation/AddSubElementButton.moc.h>
#include <GuiFoundation/PropertyGrid/Implementation/ElementGroupButton.moc.h>
#include <GuiFoundation/PropertyGrid/PropertyBaseWidget.moc.h>
#include <GuiFoundation/PropertyGrid/PropertyGridWidget.moc.h>
#include <GuiFoundation/PropertyGrid/PropertyMetaState.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <GuiFoundation/Widgets/CollapsibleGroupBox.moc.h>
#include <GuiFoundation/Widgets/InlinedGroupBox.moc.h>
#include <ToolsFoundation/Command/TreeCommands.h>
#include <ToolsFoundation/Object/ObjectAccessorBase.h>

#include <GuiFoundation/GuiFoundationDLL.h>
#include <QClipboard>
#include <QDragEnterEvent>
#include <QLabel>
#include <QMenu>
#include <QMimeData>
#include <QPainter>
#include <QScrollArea>
#include <QStringBuilder>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsPropertyClipboard, nsNoBase, 1, nsRTTIDefaultAllocator<nsPropertyClipboard>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("m_Type", m_Type),
    NS_MEMBER_PROPERTY("m_Value", m_Value),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

/// *** BASE ***
nsQtPropertyWidget::nsQtPropertyWidget()
  : QWidget(nullptr)

{
  m_bUndead = false;
  m_bIsDefault = true;
  setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
}

nsQtPropertyWidget::~nsQtPropertyWidget() = default;

void nsQtPropertyWidget::Init(nsQtPropertyGridWidget* pGrid, nsObjectAccessorBase* pObjectAccessor, const nsRTTI* pType, const nsAbstractProperty* pProp)
{
  m_pGrid = pGrid;
  m_pObjectAccessor = pObjectAccessor;
  m_pType = pType;
  m_pProp = pProp;
  NS_ASSERT_DEBUG(m_pGrid && m_pObjectAccessor && m_pType && m_pProp, "");

  if (pProp->GetAttributeByType<nsReadOnlyAttribute>() != nullptr || pProp->GetFlags().IsSet(nsPropertyFlags::ReadOnly))
  {
    SetReadOnly();
  }

  OnInit();
}

void nsQtPropertyWidget::SetSelection(const nsHybridArray<nsPropertySelection, 8>& items)
{
  m_Items = items;
}

const char* nsQtPropertyWidget::GetLabel(nsStringBuilder& ref_sTmp) const
{
  ref_sTmp.Set(m_pType->GetTypeName(), "::", m_pProp->GetPropertyName());
  return ref_sTmp;
}

void nsQtPropertyWidget::ExtendContextMenu(QMenu& m)
{
  m.setToolTipsVisible(true);
  // revert
  {
    QAction* pRevert = m.addAction("Revert to Default");
    pRevert->setEnabled(!m_bIsDefault);
    connect(pRevert, &QAction::triggered, this, [this]()
      {
      m_pObjectAccessor->StartTransaction("Revert to Default");

      switch (m_pProp->GetCategory())
      {
        case nsPropertyCategory::Enum::Array:
        case nsPropertyCategory::Enum::Set:
        case nsPropertyCategory::Enum::Map:
        {

          nsStatus res = nsStatus(NS_SUCCESS);
          if (!m_Items[0].m_Index.IsValid())
          {
            // Revert container
            nsDefaultContainerState defaultState(m_pObjectAccessor, m_Items, m_pProp->GetPropertyName());
            res = defaultState.RevertContainer();
          }
          else
          {
            const bool bIsValueType = nsReflectionUtils::IsValueType(m_pProp) || m_pProp->GetFlags().IsAnySet(nsPropertyFlags::IsEnum | nsPropertyFlags::Bitflags);
            if (bIsValueType)
            {
              // Revert container value type element
              nsDefaultContainerState defaultState(m_pObjectAccessor, m_Items, m_pProp->GetPropertyName());
              res = defaultState.RevertElement({});
            }
            else
            {
              // Revert objects pointed to by the object type element
              nsHybridArray<nsPropertySelection, 8> ResolvedObjects;
              for (const auto& item : m_Items)
              {
                nsUuid ObjectGuid = m_pObjectAccessor->Get<nsUuid>(item.m_pObject, m_pProp, item.m_Index);
                if (ObjectGuid.IsValid())
                {
                  ResolvedObjects.PushBack({m_pObjectAccessor->GetObject(ObjectGuid), nsVariant()});
                }
              }
              nsDefaultObjectState defaultState(m_pObjectAccessor, ResolvedObjects);
              res = defaultState.RevertObject();
            }
          }
          if (res.Failed())
          {
            res.LogFailure();
            m_pObjectAccessor->CancelTransaction();
            return;
          }
        }
        break;
        default:
        {
          // Revert object member property
          nsDefaultObjectState defaultState(m_pObjectAccessor, m_Items);
          nsStatus res = defaultState.RevertProperty(m_pProp);
          if (res.Failed())
          {
            res.LogFailure();
            m_pObjectAccessor->CancelTransaction();
            return;
          }
        }
        break;
      }
      m_pObjectAccessor->FinishTransaction(); });
  }

  const char* szMimeType = "application/nsEditor.Property";
  bool bValueType = nsReflectionUtils::IsValueType(m_pProp) || m_pProp->GetFlags().IsAnySet(nsPropertyFlags::Bitflags | nsPropertyFlags::IsEnum);
  // Copy
  {
    nsVariant commonValue = GetCommonValue(m_Items, m_pProp);
    QAction* pCopy = m.addAction("Copy Value");
    if (!bValueType)
    {
      pCopy->setEnabled(false);
      pCopy->setToolTip("Not a value type");
    }
    else if (!commonValue.IsValid())
    {
      pCopy->setEnabled(false);
      pCopy->setToolTip("No common value in selection");
    }

    connect(pCopy, &QAction::triggered, this, [this, szMimeType, commonValue]()
      {
      nsPropertyClipboard content;
      content.m_Type = m_pProp->GetSpecificType()->GetTypeName();
      content.m_Value = commonValue;

      // Serialize
      nsContiguousMemoryStreamStorage streamStorage;
      nsMemoryStreamWriter memoryWriter(&streamStorage);
      nsReflectionSerializer::WriteObjectToDDL(memoryWriter, nsGetStaticRTTI<nsPropertyClipboard>(), &content);
      memoryWriter.WriteBytes("\0", 1).IgnoreResult(); // null terminate

      // Write to clipboard
      QClipboard* clipboard = QApplication::clipboard();
      QMimeData* mimeData = new QMimeData();
      QByteArray encodedData((const char*)streamStorage.GetData(), streamStorage.GetStorageSize32());

      mimeData->setData(szMimeType, encodedData);
      mimeData->setText(QString::fromUtf8((const char*)streamStorage.GetData()));
      clipboard->setMimeData(mimeData); });
  }

  // Paste
  {
    QAction* pPaste = m.addAction("Paste Value");

    QClipboard* clipboard = QApplication::clipboard();
    auto mimedata = clipboard->mimeData();

    if (!bValueType)
    {
      pPaste->setEnabled(false);
      pPaste->setToolTip("Not a value type");
    }
    else if (!isEnabled())
    {
      pPaste->setEnabled(false);
      pPaste->setToolTip("Property is read only");
    }
    else if (!mimedata->hasFormat(szMimeType))
    {
      pPaste->setEnabled(false);
      pPaste->setToolTip("No property in clipboard");
    }
    else
    {
      QByteArray ba = mimedata->data(szMimeType);
      nsRawMemoryStreamReader memoryReader(ba.data(), ba.size());

      nsPropertyClipboard content;
      nsReflectionSerializer::ReadObjectPropertiesFromDDL(memoryReader, *nsGetStaticRTTI<nsPropertyClipboard>(), &content);

      const bool bIsArray = m_pProp->GetCategory() == nsPropertyCategory::Array || m_pProp->GetCategory() == nsPropertyCategory::Set;
      const nsRTTI* pClipboardType = nsRTTI::FindTypeByName(content.m_Type);
      const bool bIsEnumeration = pClipboardType && (pClipboardType->IsDerivedFrom<nsEnumBase>() || pClipboardType->IsDerivedFrom<nsBitflagsBase>() || m_pProp->GetSpecificType()->IsDerivedFrom<nsEnumBase>() || m_pProp->GetSpecificType()->IsDerivedFrom<nsBitflagsBase>());
      const bool bEnumerationMissmatch = bIsEnumeration ? pClipboardType != m_pProp->GetSpecificType() : false;
      const nsResult clamped = nsReflectionUtils::ClampValue(content.m_Value, m_pProp->GetAttributeByType<nsClampValueAttribute>());

      if (content.m_Value.IsA<nsVariantArray>() != bIsArray)
      {
        pPaste->setEnabled(false);
        nsStringBuilder sTemp;
        sTemp.SetFormat("Cannot convert clipboard and property content between arrays and members.");
        pPaste->setToolTip(sTemp.GetData());
      }
      else if (bEnumerationMissmatch || (!content.m_Value.CanConvertTo(m_pProp->GetSpecificType()->GetVariantType()) && content.m_Type != m_pProp->GetSpecificType()->GetTypeName()))
      {
        pPaste->setEnabled(false);
        nsStringBuilder sTemp;
        sTemp.SetFormat("Cannot convert clipboard of type '{}' to property of type '{}'", content.m_Type, m_pProp->GetSpecificType()->GetTypeName());
        pPaste->setToolTip(sTemp.GetData());
      }
      else if (clamped.Failed())
      {
        pPaste->setEnabled(false);
        nsStringBuilder sTemp;
        sTemp.SetFormat("The member property '{}' has an nsClampValueAttribute but nsReflectionUtils::ClampValue failed.", m_pProp->GetPropertyName());
      }

      connect(pPaste, &QAction::triggered, this, [this, content]()
        {
        m_pObjectAccessor->StartTransaction("Paste Value");
        if (content.m_Value.IsA<nsVariantArray>())
        {
          const nsVariantArray& values = content.m_Value.Get<nsVariantArray>();
          for (const nsPropertySelection& sel : m_Items)
          {
            if (m_pObjectAccessor->Clear(sel.m_pObject, m_pProp->GetPropertyName()).Failed())
            {
              m_pObjectAccessor->CancelTransaction();
              return;
            }
            for (const nsVariant& val : values)
            {
              if (m_pObjectAccessor->InsertValue(sel.m_pObject, m_pProp, val, -1).Failed())
              {
                m_pObjectAccessor->CancelTransaction();
                return;
              }
            }
          }
        }
        else
        {
          for (const nsPropertySelection& sel : m_Items)
          {
            if (m_pObjectAccessor->SetValue(sel.m_pObject, m_pProp, content.m_Value, sel.m_Index).Failed())
            {
              m_pObjectAccessor->CancelTransaction();
              return;
            }
          }
        }

        m_pObjectAccessor->FinishTransaction(); });
    }
  }

  // copy internal name
  {
    auto lambda = [this]()
    {
      QClipboard* clipboard = QApplication::clipboard();
      QMimeData* mimeData = new QMimeData();
      mimeData->setText(m_pProp->GetPropertyName());
      clipboard->setMimeData(mimeData);

      nsQtUiServices::GetSingleton()->ShowAllDocumentsTemporaryStatusBarMessage(
        nsFmt("Copied Property Name: {}", m_pProp->GetPropertyName()), nsTime::MakeFromSeconds(5));
    };

    QAction* pAction = m.addAction("Copy Internal Property Name:");
    connect(pAction, &QAction::triggered, this, lambda);

    QAction* pAction2 = m.addAction(m_pProp->GetPropertyName());
    connect(pAction2, &QAction::triggered, this, lambda);
  }
}

const nsRTTI* nsQtPropertyWidget::GetCommonBaseType(const nsHybridArray<nsPropertySelection, 8>& items)
{
  const nsRTTI* pSubtype = nullptr;

  for (const auto& item : items)
  {
    const auto& accessor = item.m_pObject->GetTypeAccessor();

    if (pSubtype == nullptr)
      pSubtype = accessor.GetType();
    else
    {
      pSubtype = nsReflectionUtils::GetCommonBaseType(pSubtype, accessor.GetType());
    }
  }

  return pSubtype;
}

QColor nsQtPropertyWidget::SetPaletteBackgroundColor(nsColorGammaUB inputColor, QPalette& ref_palette)
{
  QColor qColor = qApp->palette().color(QPalette::Window);
  if (inputColor.a != 0)
  {
    const nsColor paletteColorLinear = qtToNsColor(qColor);
    const nsColor inputColorLinear = inputColor;

    nsColor blendedColor = nsMath::Lerp(paletteColorLinear, inputColorLinear, inputColorLinear.a);
    blendedColor.a = 1.0f;
    qColor = nsToQtColor(blendedColor);
  }

  ref_palette.setBrush(QPalette::Window, QBrush(qColor, Qt::SolidPattern));
  return qColor;
}

bool nsQtPropertyWidget::GetCommonVariantSubType(
  const nsHybridArray<nsPropertySelection, 8>& items, const nsAbstractProperty* pProperty, nsVariantType::Enum& out_type)
{
  bool bFirst = true;
  // check if we have multiple values
  for (const auto& item : items)
  {
    if (bFirst)
    {
      bFirst = false;
      nsVariant value;
      m_pObjectAccessor->GetValue(item.m_pObject, pProperty, value, item.m_Index).AssertSuccess();
      out_type = value.GetType();
    }
    else
    {
      nsVariant valueNext;
      m_pObjectAccessor->GetValue(item.m_pObject, pProperty, valueNext, item.m_Index).AssertSuccess();
      if (valueNext.GetType() != out_type)
      {
        out_type = nsVariantType::Invalid;
        return false;
      }
    }
  }
  return true;
}

nsVariant nsQtPropertyWidget::GetCommonValue(const nsHybridArray<nsPropertySelection, 8>& items, const nsAbstractProperty* pProperty)
{
  if (!items[0].m_Index.IsValid() && (m_pProp->GetCategory() == nsPropertyCategory::Array || m_pProp->GetCategory() == nsPropertyCategory::Set))
  {
    nsVariantArray values;
    // check if we have multiple values
    for (nsUInt32 i = 0; i < items.GetCount(); i++)
    {
      const auto& item = items[i];
      if (i == 0)
      {
        m_pObjectAccessor->GetValues(item.m_pObject, pProperty, values).AssertSuccess();
      }
      else
      {
        nsVariantArray valuesNext;
        m_pObjectAccessor->GetValues(item.m_pObject, pProperty, valuesNext).AssertSuccess();
        if (values != valuesNext)
        {
          return nsVariant();
        }
      }
    }
    return values;
  }
  else
  {
    nsVariant value;
    // check if we have multiple values
    for (const auto& item : items)
    {
      if (!value.IsValid())
      {
        m_pObjectAccessor->GetValue(item.m_pObject, pProperty, value, item.m_Index).IgnoreResult();
      }
      else
      {
        nsVariant valueNext;
        m_pObjectAccessor->GetValue(item.m_pObject, pProperty, valueNext, item.m_Index).AssertSuccess();
        if (value != valueNext)
        {
          value = nsVariant();
          break;
        }
      }
    }
    return value;
  }
}

void nsQtPropertyWidget::PrepareToDie()
{
  NS_ASSERT_DEBUG(!m_bUndead, "Object has already been marked for cleanup");

  m_bUndead = true;

  DoPrepareToDie();
}

void nsQtPropertyWidget::SetReadOnly(bool bReadOnly /*= true*/)
{
  setDisabled(bReadOnly);
}

void nsQtPropertyWidget::OnCustomContextMenu(const QPoint& pt)
{
  QMenu m;
  m.setToolTipsVisible(true);

  ExtendContextMenu(m);
  m_pGrid->ExtendContextMenu(m, m_Items, m_pProp);

  m.exec(pt); // pt is already in global space, because we fixed that
}

void nsQtPropertyWidget::Broadcast(nsPropertyEvent::Type type)
{
  nsPropertyEvent ed;
  ed.m_Type = type;
  ed.m_pProperty = m_pProp;
  PropertyChangedHandler(ed);
}

void nsQtPropertyWidget::PropertyChangedHandler(const nsPropertyEvent& ed)
{
  if (m_bUndead)
    return;


  switch (ed.m_Type)
  {
    case nsPropertyEvent::Type::SingleValueChanged:
    {
      nsStringBuilder sTemp;
      sTemp.SetFormat("Change Property '{0}'", nsTranslate(ed.m_pProperty->GetPropertyName()));
      m_pObjectAccessor->StartTransaction(sTemp);

      nsStatus res;
      for (const auto& sel : *ed.m_pItems)
      {
        res = m_pObjectAccessor->SetValue(sel.m_pObject, ed.m_pProperty, ed.m_Value, sel.m_Index);
        if (res.m_Result.Failed())
          break;
      }

      if (res.m_Result.Failed())
        m_pObjectAccessor->CancelTransaction();
      else
        m_pObjectAccessor->FinishTransaction();

      nsQtUiServices::GetSingleton()->MessageBoxStatus(res, "Changing the property failed.");
    }
    break;

    case nsPropertyEvent::Type::BeginTemporary:
    {
      nsStringBuilder sTemp;
      sTemp.SetFormat("Change Property '{0}'", nsTranslate(ed.m_pProperty->GetPropertyName()));
      m_pObjectAccessor->BeginTemporaryCommands(sTemp);
    }
    break;

    case nsPropertyEvent::Type::EndTemporary:
    {
      m_pObjectAccessor->FinishTemporaryCommands();
    }
    break;

    case nsPropertyEvent::Type::CancelTemporary:
    {
      m_pObjectAccessor->CancelTemporaryCommands();
    }
    break;
  }
}

bool nsQtPropertyWidget::eventFilter(QObject* pWatched, QEvent* pEvent)
{
  if (pEvent->type() == QEvent::Wheel)
  {
    if (pWatched->parent())
    {
      pWatched->parent()->event(pEvent);
    }

    return true;
  }

  return false;
}

/// *** nsQtUnsupportedPropertyWidget ***

nsQtUnsupportedPropertyWidget::nsQtUnsupportedPropertyWidget(const char* szMessage)
  : nsQtPropertyWidget()
{
  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  m_pWidget = new QLabel(this);
  m_pWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
  m_pLayout->addWidget(m_pWidget);
  m_sMessage = szMessage;
}

void nsQtUnsupportedPropertyWidget::OnInit()
{
  nsQtScopedBlockSignals bs(m_pWidget);

  QString sMessage;
  if (!m_sMessage.IsEmpty())
  {
    sMessage = m_sMessage;
  }
  else
  {
    nsStringBuilder tmp;
    sMessage = QStringLiteral("Unsupported Type: ") % QString::fromUtf8(m_pProp->GetSpecificType()->GetTypeName().GetData(tmp));
  }

  m_pWidget->setText(sMessage);
  m_pWidget->setToolTip(sMessage);
}


/// *** nsQtStandardPropertyWidget ***

nsQtStandardPropertyWidget::nsQtStandardPropertyWidget()
  : nsQtPropertyWidget()
{
}

void nsQtStandardPropertyWidget::SetSelection(const nsHybridArray<nsPropertySelection, 8>& items)
{
  nsQtPropertyWidget::SetSelection(items);

  m_OldValue = GetCommonValue(items, m_pProp);
  InternalSetValue(m_OldValue);
}

void nsQtStandardPropertyWidget::BroadcastValueChanged(const nsVariant& NewValue)
{
  if (NewValue == m_OldValue)
    return;

  m_OldValue = NewValue;

  nsPropertyEvent ed;
  ed.m_Type = nsPropertyEvent::Type::SingleValueChanged;
  ed.m_pProperty = m_pProp;
  ed.m_Value = NewValue;
  ed.m_pItems = &m_Items;
  PropertyChangedHandler(ed);
}


/// *** nsQtPropertyPointerWidget ***

nsQtPropertyPointerWidget::nsQtPropertyPointerWidget()
  : nsQtPropertyWidget()
{
  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  m_pGroup = new nsQtCollapsibleGroupBox(this);
  m_pGroupLayout = new QHBoxLayout(nullptr);
  m_pGroupLayout->setSpacing(1);
  m_pGroupLayout->setContentsMargins(5, 0, 0, 0);
  m_pGroup->GetContent()->setLayout(m_pGroupLayout);

  m_pLayout->addWidget(m_pGroup);

  m_pAddButton = new nsQtAddSubElementButton();
  m_pGroup->GetHeader()->layout()->addWidget(m_pAddButton);

  m_pDeleteButton = new nsQtElementGroupButton(m_pGroup->GetHeader(), nsQtElementGroupButton::ElementAction::DeleteElement, this);
  m_pGroup->GetHeader()->layout()->addWidget(m_pDeleteButton);
  connect(m_pDeleteButton, &QToolButton::clicked, this, &nsQtPropertyPointerWidget::OnDeleteButtonClicked);

  m_pTypeWidget = nullptr;
}

nsQtPropertyPointerWidget::~nsQtPropertyPointerWidget()
{
  m_pGrid->GetDocument()->GetObjectManager()->m_StructureEvents.RemoveEventHandler(
    nsMakeDelegate(&nsQtPropertyPointerWidget::StructureEventHandler, this));
}

void nsQtPropertyPointerWidget::OnInit()
{
  UpdateTitle();
  m_pGrid->SetCollapseState(m_pGroup);
  connect(m_pGroup, &nsQtGroupBoxBase::CollapseStateChanged, m_pGrid, &nsQtPropertyGridWidget::OnCollapseStateChanged);

  // Add Buttons
  auto pAttr = m_pProp->GetAttributeByType<nsContainerAttribute>();
  m_pAddButton->setVisible(!pAttr || pAttr->CanAdd());
  m_pDeleteButton->setVisible(!pAttr || pAttr->CanDelete());

  m_pAddButton->Init(m_pGrid, m_pObjectAccessor, m_pType, m_pProp);
  m_pGrid->GetDocument()->GetObjectManager()->m_StructureEvents.AddEventHandler(
    nsMakeDelegate(&nsQtPropertyPointerWidget::StructureEventHandler, this));
}

void nsQtPropertyPointerWidget::SetSelection(const nsHybridArray<nsPropertySelection, 8>& items)
{
  nsQtScopedUpdatesDisabled _(this);

  nsQtPropertyWidget::SetSelection(items);

  if (m_pTypeWidget)
  {
    m_pGroupLayout->removeWidget(m_pTypeWidget);
    delete m_pTypeWidget;
    m_pTypeWidget = nullptr;
  }


  nsHybridArray<nsPropertySelection, 8> emptyItems;
  nsHybridArray<nsPropertySelection, 8> subItems;
  for (const auto& item : m_Items)
  {
    nsUuid ObjectGuid = m_pObjectAccessor->Get<nsUuid>(item.m_pObject, m_pProp, item.m_Index);
    if (!ObjectGuid.IsValid())
    {
      emptyItems.PushBack(item);
    }
    else
    {
      nsPropertySelection sel;
      sel.m_pObject = m_pObjectAccessor->GetObject(ObjectGuid);

      subItems.PushBack(sel);
    }
  }

  auto pAttr = m_pProp->GetAttributeByType<nsContainerAttribute>();
  if (!pAttr || pAttr->CanAdd())
    m_pAddButton->setVisible(!emptyItems.IsEmpty());
  if (!pAttr || pAttr->CanDelete())
    m_pDeleteButton->setVisible(!subItems.IsEmpty());

  if (!emptyItems.IsEmpty())
  {
    m_pAddButton->SetSelection(emptyItems);
  }

  const nsRTTI* pCommonType = nullptr;
  if (!subItems.IsEmpty())
  {
    pCommonType = nsQtPropertyWidget::GetCommonBaseType(subItems);

    m_pTypeWidget = new nsQtTypeWidget(m_pGroup->GetContent(), m_pGrid, m_pObjectAccessor, pCommonType, nullptr, nullptr);
    m_pTypeWidget->SetSelection(subItems);

    m_pGroupLayout->addWidget(m_pTypeWidget);
  }

  UpdateTitle(pCommonType);
}


void nsQtPropertyPointerWidget::DoPrepareToDie()
{
  if (m_pTypeWidget)
  {
    m_pTypeWidget->PrepareToDie();
  }
}

void nsQtPropertyPointerWidget::UpdateTitle(const nsRTTI* pType /*= nullptr*/)
{
  nsStringBuilder sb = nsTranslate(m_pProp->GetPropertyName());
  if (pType != nullptr)
  {
    nsStringBuilder tmp;
    sb.Append(": ", nsTranslate(pType->GetTypeName().GetData(tmp)));
  }
  m_pGroup->SetTitle(sb);
}

void nsQtPropertyPointerWidget::OnDeleteButtonClicked()
{
  m_pObjectAccessor->StartTransaction("Delete Object");

  nsStatus res;
  const nsHybridArray<nsPropertySelection, 8> selection = m_pTypeWidget->GetSelection();
  for (auto& item : selection)
  {
    res = m_pObjectAccessor->RemoveObject(item.m_pObject);
    if (res.m_Result.Failed())
      break;
  }

  if (res.m_Result.Failed())
    m_pObjectAccessor->CancelTransaction();
  else
    m_pObjectAccessor->FinishTransaction();

  nsQtUiServices::GetSingleton()->MessageBoxStatus(res, "Removing sub-element from the property failed.");
}

void nsQtPropertyPointerWidget::StructureEventHandler(const nsDocumentObjectStructureEvent& e)
{
  if (IsUndead())
    return;

  switch (e.m_EventType)
  {
    case nsDocumentObjectStructureEvent::Type::AfterObjectAdded:
    case nsDocumentObjectStructureEvent::Type::AfterObjectMoved:
    case nsDocumentObjectStructureEvent::Type::AfterObjectRemoved:
    {
      if (!e.m_sParentProperty.IsEqual(m_pProp->GetPropertyName()))
        return;

      if (std::none_of(cbegin(m_Items), cend(m_Items),
            [&](const nsPropertySelection& sel)
            { return e.m_pNewParent == sel.m_pObject || e.m_pPreviousParent == sel.m_pObject; }))
        return;

      SetSelection(m_Items);
    }
    break;
    default:
      break;
  }
}

/// *** nsQtEmbeddedClassPropertyWidget ***

nsQtEmbeddedClassPropertyWidget::nsQtEmbeddedClassPropertyWidget()
  : nsQtPropertyWidget()

{
}


nsQtEmbeddedClassPropertyWidget::~nsQtEmbeddedClassPropertyWidget()
{
  m_pGrid->GetObjectManager()->m_PropertyEvents.RemoveEventHandler(nsMakeDelegate(&nsQtEmbeddedClassPropertyWidget::PropertyEventHandler, this));
  m_pGrid->GetCommandHistory()->m_Events.RemoveEventHandler(nsMakeDelegate(&nsQtEmbeddedClassPropertyWidget::CommandHistoryEventHandler, this));
}

void nsQtEmbeddedClassPropertyWidget::SetSelection(const nsHybridArray<nsPropertySelection, 8>& items)
{
  nsQtScopedUpdatesDisabled _(this);

  nsQtPropertyWidget::SetSelection(items);

  // Retrieve the objects the property points to. This could be an embedded class or
  // an element of an array, be it pointer or embedded class.
  m_ResolvedObjects.Clear();
  for (const auto& item : m_Items)
  {
    nsUuid ObjectGuid = m_pObjectAccessor->Get<nsUuid>(item.m_pObject, m_pProp, item.m_Index);
    nsPropertySelection sel;
    sel.m_pObject = m_pObjectAccessor->GetObject(ObjectGuid);
    // sel.m_Index; intentionally invalid as we just retrieved the value so it is a pointer to an object

    m_ResolvedObjects.PushBack(sel);
  }

  m_pResolvedType = m_pProp->GetSpecificType();
  if (m_pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
  {
    m_pResolvedType = nsQtPropertyWidget::GetCommonBaseType(m_ResolvedObjects);
  }
}

void nsQtEmbeddedClassPropertyWidget::SetPropertyValue(const nsAbstractProperty* pProperty, const nsVariant& NewValue)
{
  nsStatus res;
  for (const auto& sel : m_ResolvedObjects)
  {
    res = m_pObjectAccessor->SetValue(sel.m_pObject, pProperty, NewValue, sel.m_Index);
    if (res.m_Result.Failed())
      break;
  }
  // nsPropertyEvent ed;
  // ed.m_Type = nsPropertyEvent::Type::SingleValueChanged;
  // ed.m_pProperty = pProperty;
  // ed.m_Value = NewValue;
  // ed.m_pItems = &m_ResolvedObjects;

  // m_Events.Broadcast(ed);
}

void nsQtEmbeddedClassPropertyWidget::OnInit()
{
  m_pGrid->GetObjectManager()->m_PropertyEvents.AddEventHandler(nsMakeDelegate(&nsQtEmbeddedClassPropertyWidget::PropertyEventHandler, this));
  m_pGrid->GetCommandHistory()->m_Events.AddEventHandler(nsMakeDelegate(&nsQtEmbeddedClassPropertyWidget::CommandHistoryEventHandler, this));
}


void nsQtEmbeddedClassPropertyWidget::DoPrepareToDie() {}

void nsQtEmbeddedClassPropertyWidget::PropertyEventHandler(const nsDocumentObjectPropertyEvent& e)
{
  if (IsUndead())
    return;

  if (std::none_of(cbegin(m_ResolvedObjects), cend(m_ResolvedObjects), [=](const nsPropertySelection& sel)
        { return e.m_pObject == sel.m_pObject; }))
    return;

  if (!m_QueuedChanges.Contains(e.m_sProperty))
  {
    m_QueuedChanges.PushBack(e.m_sProperty);
  }
}


void nsQtEmbeddedClassPropertyWidget::CommandHistoryEventHandler(const nsCommandHistoryEvent& e)
{
  if (IsUndead())
    return;

  switch (e.m_Type)
  {
    case nsCommandHistoryEvent::Type::UndoEnded:
    case nsCommandHistoryEvent::Type::RedoEnded:
    case nsCommandHistoryEvent::Type::TransactionEnded:
    case nsCommandHistoryEvent::Type::TransactionCanceled:
    {
      FlushQueuedChanges();
    }
    break;

    default:
      break;
  }
}

void nsQtEmbeddedClassPropertyWidget::FlushQueuedChanges()
{
  for (const nsString& sProperty : m_QueuedChanges)
  {
    OnPropertyChanged(sProperty);
  }

  m_QueuedChanges.Clear();
}

/// *** nsQtPropertyTypeWidget ***

nsQtPropertyTypeWidget::nsQtPropertyTypeWidget(bool bAddCollapsibleGroup)
  : nsQtPropertyWidget()
{
  m_pLayout = new QVBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);
  m_pGroup = nullptr;
  m_pGroupLayout = nullptr;

  if (bAddCollapsibleGroup)
  {
    m_pGroup = new nsQtCollapsibleGroupBox(this);
    m_pGroupLayout = new QVBoxLayout(nullptr);
    m_pGroupLayout->setSpacing(1);
    m_pGroupLayout->setContentsMargins(5, 0, 0, 0);
    m_pGroup->GetContent()->setLayout(m_pGroupLayout);

    m_pLayout->addWidget(m_pGroup);
  }
  m_pTypeWidget = nullptr;
}

nsQtPropertyTypeWidget::~nsQtPropertyTypeWidget() = default;

void nsQtPropertyTypeWidget::OnInit()
{
  if (m_pGroup)
  {
    m_pGroup->SetTitle(nsTranslate(m_pProp->GetPropertyName()));
    m_pGrid->SetCollapseState(m_pGroup);
    connect(m_pGroup, &nsQtGroupBoxBase::CollapseStateChanged, m_pGrid, &nsQtPropertyGridWidget::OnCollapseStateChanged);
  }
}

void nsQtPropertyTypeWidget::SetSelection(const nsHybridArray<nsPropertySelection, 8>& items)
{
  nsQtScopedUpdatesDisabled _(this);

  nsQtPropertyWidget::SetSelection(items);

  QVBoxLayout* pLayout = m_pGroup != nullptr ? m_pGroupLayout : m_pLayout;
  QWidget* pOwner = m_pGroup != nullptr ? m_pGroup->GetContent() : this;
  if (m_pTypeWidget)
  {
    pLayout->removeWidget(m_pTypeWidget);
    delete m_pTypeWidget;
    m_pTypeWidget = nullptr;
  }

  // Retrieve the objects the property points to. This could be an embedded class or
  // an element of an array, be it pointer or embedded class.
  nsHybridArray<nsPropertySelection, 8> ResolvedObjects;
  for (const auto& item : m_Items)
  {
    nsUuid ObjectGuid = m_pObjectAccessor->Get<nsUuid>(item.m_pObject, m_pProp, item.m_Index);
    nsPropertySelection sel;
    sel.m_pObject = m_pObjectAccessor->GetObject(ObjectGuid);
    // sel.m_Index; intentionally invalid as we just retrieved the value so it is a pointer to an object

    ResolvedObjects.PushBack(sel);
  }

  const nsRTTI* pCommonType = nullptr;
  if (m_pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
  {
    pCommonType = nsQtPropertyWidget::GetCommonBaseType(ResolvedObjects);
  }
  else
  {
    // If we create a widget for a member class we already determined the common base type at the parent type widget.
    // As we are not dealing with a pointer in this case the type must match the property exactly.
    pCommonType = m_pProp->GetSpecificType();
  }
  m_pTypeWidget = new nsQtTypeWidget(pOwner, m_pGrid, m_pObjectAccessor, pCommonType, nullptr, nullptr);
  pLayout->addWidget(m_pTypeWidget);
  m_pTypeWidget->SetSelection(ResolvedObjects);
}


void nsQtPropertyTypeWidget::SetIsDefault(bool bIsDefault)
{
  // The default state set by the parent object / container only refers to the element's correct position in the container but the entire state of the object. As recursively checking an entire object if is has any non-default values is quite costly, we just pretend the object is never in its default state the the user can click revert to default on any object at any time.
  m_bIsDefault = false;
}

void nsQtPropertyTypeWidget::DoPrepareToDie()
{
  if (m_pTypeWidget)
  {
    m_pTypeWidget->PrepareToDie();
  }
}

/// *** nsQtPropertyContainerWidget ***

nsQtPropertyContainerWidget::nsQtPropertyContainerWidget()
  : nsQtPropertyWidget()

{
  m_Pal = palette();
  setAutoFillBackground(true);

  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  m_pGroup = new nsQtCollapsibleGroupBox(this);
  m_pGroupLayout = new QVBoxLayout(nullptr);
  m_pGroupLayout->setSpacing(1);
  m_pGroupLayout->setContentsMargins(5, 0, 0, 0);
  m_pGroup->GetContent()->setLayout(m_pGroupLayout);
  m_pGroup->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  connect(m_pGroup, &QWidget::customContextMenuRequested, this, &nsQtPropertyContainerWidget::OnContainerContextMenu);

  setAcceptDrops(true);
  m_pLayout->addWidget(m_pGroup);
}

nsQtPropertyContainerWidget::~nsQtPropertyContainerWidget()
{
  Clear();
}

void nsQtPropertyContainerWidget::SetSelection(const nsHybridArray<nsPropertySelection, 8>& items)
{
  nsQtPropertyWidget::SetSelection(items);

  UpdateElements();

  if (m_pAddButton)
  {
    m_pAddButton->SetSelection(m_Items);
  }
}

void nsQtPropertyContainerWidget::SetIsDefault(bool bIsDefault)
{
  // This is called from the type widget which we ignore as we have a tighter scoped default value provider for containers.
}

void nsQtPropertyContainerWidget::DoPrepareToDie()
{
  for (const auto& e : m_Elements)
  {
    e.m_pWidget->PrepareToDie();
  }
}

void nsQtPropertyContainerWidget::dragEnterEvent(QDragEnterEvent* event)
{
  updateDropIndex(event);
}

void nsQtPropertyContainerWidget::dragMoveEvent(QDragMoveEvent* event)
{
  updateDropIndex(event);
}

void nsQtPropertyContainerWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
  m_iDropSource = -1;
  m_iDropTarget = -1;
  update();
}

void nsQtPropertyContainerWidget::dropEvent(QDropEvent* event)
{
  if (updateDropIndex(event))
  {
    nsQtGroupBoxBase* pGroup = qobject_cast<nsQtGroupBoxBase*>(event->source());
    Element* pDragElement =
      std::find_if(begin(m_Elements), end(m_Elements), [pGroup](const Element& elem) -> bool
        { return elem.m_pSubGroup == pGroup; });
    if (pDragElement)
    {
      const nsAbstractProperty* pProp = pDragElement->m_pWidget->GetProperty();
      nsHybridArray<nsPropertySelection, 8> items = pDragElement->m_pWidget->GetSelection();
      if (m_iDropSource != m_iDropTarget && (m_iDropSource + 1) != m_iDropTarget)
      {
        MoveItems(items, m_iDropTarget - m_iDropSource);
      }
    }
  }
  m_iDropSource = -1;
  m_iDropTarget = -1;
  update();
}

void nsQtPropertyContainerWidget::paintEvent(QPaintEvent* event)
{
  nsQtPropertyWidget::paintEvent(event);
  if (m_iDropSource != -1 && m_iDropTarget != -1)
  {
    nsInt32 iYPos = 0;
    if (m_iDropTarget < (nsInt32)m_Elements.GetCount())
    {
      const QPoint globalPos = m_Elements[m_iDropTarget].m_pSubGroup->mapToGlobal(QPoint(0, 0));
      iYPos = mapFromGlobal(globalPos).y();
    }
    else
    {
      const QPoint globalPos = m_Elements[m_Elements.GetCount() - 1].m_pSubGroup->mapToGlobal(QPoint(0, 0));
      iYPos = mapFromGlobal(globalPos).y() + m_Elements[m_Elements.GetCount() - 1].m_pSubGroup->height();
    }

    QPainter painter(this);
    painter.setPen(QPen(Qt::PenStyle::NoPen));
    painter.setBrush(palette().brush(QPalette::Highlight));
    painter.drawRect(0, iYPos - 3, width(), 4);
  }
}

void nsQtPropertyContainerWidget::showEvent(QShowEvent* event)
{
  // Use of style sheets (ADS) breaks previously set palette.
  setPalette(m_Pal);
  nsQtPropertyWidget::showEvent(event);
}

bool nsQtPropertyContainerWidget::updateDropIndex(QDropEvent* pEvent)
{
  if (pEvent->source() && pEvent->mimeData()->hasFormat("application/x-groupBoxDragProperty"))
  {
    // Is the drop source part of this widget?
    for (nsUInt32 i = 0; i < m_Elements.GetCount(); i++)
    {
      if (m_Elements[i].m_pSubGroup == pEvent->source())
      {
        pEvent->setDropAction(Qt::MoveAction);
        pEvent->accept();
        nsInt32 iNewDropTarget = -1;
        // Find closest drop target.
        const nsInt32 iGlobalYPos = mapToGlobal(pEvent->position().toPoint()).y();
        for (nsUInt32 j = 0; j < m_Elements.GetCount(); j++)
        {
          const QRect rect(m_Elements[j].m_pSubGroup->mapToGlobal(QPoint(0, 0)), m_Elements[j].m_pSubGroup->size());
          if (iGlobalYPos > rect.center().y())
          {
            iNewDropTarget = (nsInt32)j + 1;
          }
          else if (iGlobalYPos < rect.center().y())
          {
            iNewDropTarget = (nsInt32)j;
            break;
          }
        }
        if (m_iDropSource != (nsInt32)i || m_iDropTarget != iNewDropTarget)
        {
          m_iDropSource = (nsInt32)i;
          m_iDropTarget = iNewDropTarget;
          update();
        }
        return true;
      }
    }
  }

  if (m_iDropSource != -1 || m_iDropTarget != -1)
  {
    m_iDropSource = -1;
    m_iDropTarget = -1;
    update();
  }
  pEvent->ignore();
  return false;
}

void nsQtPropertyContainerWidget::OnElementButtonClicked()
{
  nsQtElementGroupButton* pButton = qobject_cast<nsQtElementGroupButton*>(sender());
  const nsAbstractProperty* pProp = pButton->GetGroupWidget()->GetProperty();
  nsHybridArray<nsPropertySelection, 8> items = pButton->GetGroupWidget()->GetSelection();

  switch (pButton->GetAction())
  {
    case nsQtElementGroupButton::ElementAction::MoveElementUp:
    {
      MoveItems(items, -1);
    }
    break;
    case nsQtElementGroupButton::ElementAction::MoveElementDown:
    {
      MoveItems(items, 2);
    }
    break;
    case nsQtElementGroupButton::ElementAction::DeleteElement:
    {
      DeleteItems(items);
    }
    break;

    case nsQtElementGroupButton::ElementAction::Help:
      // handled by custom lambda
      break;
  }
}

void nsQtPropertyContainerWidget::OnDragStarted(QMimeData& ref_mimeData)
{
  nsQtGroupBoxBase* pGroup = qobject_cast<nsQtGroupBoxBase*>(sender());
  Element* pDragElement =
    std::find_if(begin(m_Elements), end(m_Elements), [pGroup](const Element& elem) -> bool
      { return elem.m_pSubGroup == pGroup; });
  if (pDragElement)
  {
    ref_mimeData.setData("application/x-groupBoxDragProperty", QByteArray());
  }
}

void nsQtPropertyContainerWidget::OnContainerContextMenu(const QPoint& pt)
{
  nsQtGroupBoxBase* pGroup = qobject_cast<nsQtGroupBoxBase*>(sender());

  QMenu m;
  m.setToolTipsVisible(true);
  ExtendContextMenu(m);

  if (!m.isEmpty())
  {
    m.exec(pGroup->mapToGlobal(pt));
  }
}

void nsQtPropertyContainerWidget::OnCustomElementContextMenu(const QPoint& pt)
{
  nsQtGroupBoxBase* pGroup = qobject_cast<nsQtGroupBoxBase*>(sender());
  Element* pElement = std::find_if(begin(m_Elements), end(m_Elements), [pGroup](const Element& elem) -> bool
    { return elem.m_pSubGroup == pGroup; });

  if (pElement)
  {
    QMenu m;
    m.setToolTipsVisible(true);
    pElement->m_pWidget->ExtendContextMenu(m);

    m_pGrid->ExtendContextMenu(m, pElement->m_pWidget->GetSelection(), pElement->m_pWidget->GetProperty());

    if (!m.isEmpty())
    {
      m.exec(pGroup->mapToGlobal(pt));
    }
  }
}

nsQtGroupBoxBase* nsQtPropertyContainerWidget::CreateElement(QWidget* pParent)
{
  auto pBox = new nsQtCollapsibleGroupBox(pParent);
  return pBox;
}

nsQtPropertyWidget* nsQtPropertyContainerWidget::CreateWidget(nsUInt32 index)
{
  return new nsQtPropertyTypeWidget();
}

nsQtPropertyContainerWidget::Element& nsQtPropertyContainerWidget::AddElement(nsUInt32 index)
{
  nsQtGroupBoxBase* pSubGroup = CreateElement(m_pGroup);
  pSubGroup->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  connect(pSubGroup, &nsQtGroupBoxBase::CollapseStateChanged, m_pGrid, &nsQtPropertyGridWidget::OnCollapseStateChanged);
  connect(pSubGroup, &QWidget::customContextMenuRequested, this, &nsQtPropertyContainerWidget::OnCustomElementContextMenu);

  QVBoxLayout* pSubLayout = new QVBoxLayout(nullptr);
  pSubLayout->setContentsMargins(5, 0, 5, 0);
  pSubLayout->setSpacing(1);
  pSubGroup->GetContent()->setLayout(pSubLayout);

  m_pGroupLayout->insertWidget((int)index, pSubGroup);

  nsQtPropertyWidget* pNewWidget = CreateWidget(index);

  pNewWidget->setParent(pSubGroup);
  pSubLayout->addWidget(pNewWidget);

  pNewWidget->Init(m_pGrid, m_pObjectAccessor, m_pType, m_pProp);

  // Add Buttons
  auto pAttr = m_pProp->GetAttributeByType<nsContainerAttribute>();
  if ((!pAttr || pAttr->CanMove()) && m_pProp->GetCategory() != nsPropertyCategory::Map)
  {
    pSubGroup->SetDraggable(true);
    connect(pSubGroup, &nsQtGroupBoxBase::DragStarted, this, &nsQtPropertyContainerWidget::OnDragStarted);
  }

  nsQtElementGroupButton* pHelpButton = new nsQtElementGroupButton(pSubGroup->GetHeader(), nsQtElementGroupButton::ElementAction::Help, pNewWidget);
  pSubGroup->GetHeader()->layout()->addWidget(pHelpButton);
  pHelpButton->setVisible(false); // added now, and shown later when we know the URL

  if (!pAttr || pAttr->CanDelete())
  {
    nsQtElementGroupButton* pDeleteButton =
      new nsQtElementGroupButton(pSubGroup->GetHeader(), nsQtElementGroupButton::ElementAction::DeleteElement, pNewWidget);
    pSubGroup->GetHeader()->layout()->addWidget(pDeleteButton);
    connect(pDeleteButton, &QToolButton::clicked, this, &nsQtPropertyContainerWidget::OnElementButtonClicked);
  }

  m_Elements.InsertAt(index, Element(pSubGroup, pNewWidget, pHelpButton));
  return m_Elements[index];
}

void nsQtPropertyContainerWidget::RemoveElement(nsUInt32 index)
{
  Element& elem = m_Elements[index];

  m_pGroupLayout->removeWidget(elem.m_pSubGroup);
  delete elem.m_pSubGroup;
  m_Elements.RemoveAtAndCopy(index);
}

void nsQtPropertyContainerWidget::UpdateElements()
{
  nsQtScopedUpdatesDisabled _(this);

  nsUInt32 iElements = GetRequiredElementCount();

  while (m_Elements.GetCount() > iElements)
  {
    RemoveElement(m_Elements.GetCount() - 1);
  }
  while (m_Elements.GetCount() < iElements)
  {
    AddElement(m_Elements.GetCount());
  }

  for (nsUInt32 i = 0; i < iElements; ++i)
  {
    UpdateElement(i);
  }

  UpdatePropertyMetaState();

  // Force re-layout of parent hierarchy to prevent flicker.
  QWidget* pCur = m_pGroup;
  while (pCur != nullptr && qobject_cast<QScrollArea*>(pCur) == nullptr)
  {
    pCur->updateGeometry();
    pCur = pCur->parentWidget();
  }
}

nsUInt32 nsQtPropertyContainerWidget::GetRequiredElementCount() const
{
  if (m_pProp->GetCategory() == nsPropertyCategory::Map)
  {
    m_Keys.Clear();
    NS_VERIFY(m_pObjectAccessor->GetKeys(m_Items[0].m_pObject, m_pProp, m_Keys).m_Result.Succeeded(), "GetKeys should always succeed.");
    nsHybridArray<nsVariant, 16> keys;
    for (nsUInt32 i = 1; i < m_Items.GetCount(); i++)
    {
      keys.Clear();
      NS_VERIFY(m_pObjectAccessor->GetKeys(m_Items[i].m_pObject, m_pProp, keys).m_Result.Succeeded(), "GetKeys should always succeed.");
      for (nsInt32 k = (nsInt32)m_Keys.GetCount() - 1; k >= 0; --k)
      {
        if (!keys.Contains(m_Keys[k]))
        {
          m_Keys.RemoveAtAndSwap(k);
        }
      }
    }
    m_Keys.Sort([](const nsVariant& a, const nsVariant& b)
      { return a.Get<nsString>().Compare(b.Get<nsString>()) < 0; });
    return m_Keys.GetCount();
  }
  else
  {
    nsInt32 iElements = 0x7FFFFFFF;
    for (const auto& item : m_Items)
    {
      nsInt32 iCount = 0;
      NS_VERIFY(m_pObjectAccessor->GetCount(item.m_pObject, m_pProp, iCount).m_Result.Succeeded(), "GetCount should always succeed.");
      iElements = nsMath::Min(iElements, iCount);
    }
    NS_ASSERT_DEV(iElements >= 0, "Mismatch between storage and RTTI ({0})", iElements);
    m_Keys.Clear();
    for (nsUInt32 i = 0; i < (nsUInt32)iElements; i++)
    {
      m_Keys.PushBack(i);
    }

    return nsUInt32(iElements);
  }
}

void nsQtPropertyContainerWidget::UpdatePropertyMetaState()
{
  nsPropertyMetaState* pMeta = nsPropertyMetaState::GetSingleton();
  nsHashTable<nsVariant, nsPropertyUiState> ElementStates;
  pMeta->GetContainerElementsState(m_Items, m_pProp->GetPropertyName(), ElementStates);

  nsDefaultContainerState defaultState(m_pObjectAccessor, m_Items, m_pProp->GetPropertyName());
  m_bIsDefault = defaultState.IsDefaultContainer();
  m_pGroup->SetBoldTitle(!m_bIsDefault);

  QColor qColor = nsQtPropertyWidget::SetPaletteBackgroundColor(defaultState.GetBackgroundColor(), m_Pal);
  setPalette(m_Pal);

  const bool bReadOnly = m_pProp->GetFlags().IsSet(nsPropertyFlags::ReadOnly) ||
                         (m_pProp->GetAttributeByType<nsReadOnlyAttribute>() != nullptr);
  for (nsUInt32 i = 0; i < m_Elements.GetCount(); i++)
  {
    Element& element = m_Elements[i];
    nsVariant& key = m_Keys[i];
    const bool bIsDefault = defaultState.IsDefaultElement(key);
    auto itData = ElementStates.Find(key);
    nsPropertyUiState::Visibility state = nsPropertyUiState::Default;
    if (itData.IsValid())
    {
      state = itData.Value().m_Visibility;
    }

    if (element.m_pSubGroup)
    {
      element.m_pSubGroup->setVisible(state != nsPropertyUiState::Invisible);
      element.m_pSubGroup->setEnabled(!bReadOnly && state != nsPropertyUiState::Disabled);
      element.m_pSubGroup->SetBoldTitle(!bIsDefault);

      // If the fill color is invalid that means no border is drawn and we don't want to change the color then.
      if (!element.m_pSubGroup->GetFillColor().isValid())
      {
        element.m_pSubGroup->SetFillColor(qColor);
      }
    }
    if (element.m_pWidget)
    {
      element.m_pWidget->setVisible(state != nsPropertyUiState::Invisible);
      element.m_pWidget->SetReadOnly(bReadOnly || state == nsPropertyUiState::Disabled);
      element.m_pWidget->SetIsDefault(bIsDefault);
    }
  }
}

void nsQtPropertyContainerWidget::Clear()
{
  while (m_Elements.GetCount() > 0)
  {
    RemoveElement(m_Elements.GetCount() - 1);
  }

  m_Elements.Clear();
}

void nsQtPropertyContainerWidget::OnInit()
{
  nsStringBuilder fullname(m_pType->GetTypeName(), "::", m_pProp->GetPropertyName());

  m_pGroup->SetTitle(nsTranslate(fullname));

  const nsContainerAttribute* pArrayAttr = m_pProp->GetAttributeByType<nsContainerAttribute>();
  if (!pArrayAttr || pArrayAttr->CanAdd())
  {
    m_pAddButton = new nsQtAddSubElementButton();
    m_pAddButton->Init(m_pGrid, m_pObjectAccessor, m_pType, m_pProp);
    m_pGroup->GetHeader()->layout()->addWidget(m_pAddButton);
  }

  m_pGrid->SetCollapseState(m_pGroup);
  connect(m_pGroup, &nsQtGroupBoxBase::CollapseStateChanged, m_pGrid, &nsQtPropertyGridWidget::OnCollapseStateChanged);
}

void nsQtPropertyContainerWidget::DeleteItems(nsHybridArray<nsPropertySelection, 8>& items)
{
  m_pObjectAccessor->StartTransaction("Delete Object");

  nsStatus res(NS_SUCCESS);
  const bool bIsValueType = nsReflectionUtils::IsValueType(m_pProp);

  if (bIsValueType)
  {
    for (auto& item : items)
    {
      res = m_pObjectAccessor->RemoveValue(item.m_pObject, m_pProp, item.m_Index);
      if (res.m_Result.Failed())
        break;
    }
  }
  else
  {
    nsRemoveObjectCommand cmd;

    for (auto& item : items)
    {
      nsUuid value = m_pObjectAccessor->Get<nsUuid>(item.m_pObject, m_pProp, item.m_Index);
      const nsDocumentObject* pObject = m_pObjectAccessor->GetObject(value);
      res = m_pObjectAccessor->RemoveObject(pObject);
      if (res.m_Result.Failed())
        break;
    }
  }

  if (res.m_Result.Failed())
    m_pObjectAccessor->CancelTransaction();
  else
    m_pObjectAccessor->FinishTransaction();

  nsQtUiServices::GetSingleton()->MessageBoxStatus(res, "Removing sub-element from the property failed.");
}

void nsQtPropertyContainerWidget::MoveItems(nsHybridArray<nsPropertySelection, 8>& items, nsInt32 iMove)
{
  NS_ASSERT_DEV(m_pProp->GetCategory() != nsPropertyCategory::Map, "Map entries can't be moved.");

  m_pObjectAccessor->StartTransaction("Reparent Object");

  nsStatus res(NS_SUCCESS);
  const bool bIsValueType = nsReflectionUtils::IsValueType(m_pProp);
  if (bIsValueType)
  {
    for (auto& item : items)
    {
      nsInt32 iCurIndex = item.m_Index.ConvertTo<nsInt32>() + iMove;
      if (iCurIndex < 0 || iCurIndex > m_pObjectAccessor->GetCount(item.m_pObject, m_pProp))
        continue;

      res = m_pObjectAccessor->MoveValue(item.m_pObject, m_pProp, item.m_Index, iCurIndex);
      if (res.m_Result.Failed())
        break;
    }
  }
  else
  {
    nsMoveObjectCommand cmd;

    for (auto& item : items)
    {
      nsInt32 iCurIndex = item.m_Index.ConvertTo<nsInt32>() + iMove;
      if (iCurIndex < 0 || iCurIndex > m_pObjectAccessor->GetCount(item.m_pObject, m_pProp))
        continue;

      nsUuid value = m_pObjectAccessor->Get<nsUuid>(item.m_pObject, m_pProp, item.m_Index);
      const nsDocumentObject* pObject = m_pObjectAccessor->GetObject(value);

      res = m_pObjectAccessor->MoveObject(pObject, item.m_pObject, m_pProp, iCurIndex);
      if (res.m_Result.Failed())
        break;
    }
  }

  if (res.m_Result.Failed())
    m_pObjectAccessor->CancelTransaction();
  else
    m_pObjectAccessor->FinishTransaction();

  nsQtUiServices::GetSingleton()->MessageBoxStatus(res, "Moving sub-element failed.");
}


/// *** nsQtPropertyStandardTypeContainerWidget ***

nsQtPropertyStandardTypeContainerWidget::nsQtPropertyStandardTypeContainerWidget()
  : nsQtPropertyContainerWidget()
{
}

nsQtPropertyStandardTypeContainerWidget::~nsQtPropertyStandardTypeContainerWidget() = default;

nsQtGroupBoxBase* nsQtPropertyStandardTypeContainerWidget::CreateElement(QWidget* pParent)
{
  auto* pBox = new nsQtInlinedGroupBox(pParent);
  pBox->SetFillColor(QColor::Invalid);
  return pBox;
}


nsQtPropertyWidget* nsQtPropertyStandardTypeContainerWidget::CreateWidget(nsUInt32 index)
{
  return nsQtPropertyGridWidget::CreateMemberPropertyWidget(m_pProp);
}

nsQtPropertyContainerWidget::Element& nsQtPropertyStandardTypeContainerWidget::AddElement(nsUInt32 index)
{
  nsQtPropertyContainerWidget::Element& elem = nsQtPropertyContainerWidget::AddElement(index);
  return elem;
}

void nsQtPropertyStandardTypeContainerWidget::RemoveElement(nsUInt32 index)
{
  nsQtPropertyContainerWidget::RemoveElement(index);
}

void nsQtPropertyStandardTypeContainerWidget::UpdateElement(nsUInt32 index)
{
  Element& elem = m_Elements[index];

  nsHybridArray<nsPropertySelection, 8> SubItems;

  for (const auto& item : m_Items)
  {
    nsPropertySelection sel;
    sel.m_pObject = item.m_pObject;
    sel.m_Index = m_Keys[index];

    SubItems.PushBack(sel);
  }

  nsStringBuilder sTitle;
  if (m_pProp->GetCategory() == nsPropertyCategory::Map)
    sTitle.SetFormat("{0}", m_Keys[index].ConvertTo<nsString>());
  else
    sTitle.SetFormat("[{0}]", m_Keys[index].ConvertTo<nsString>());

  elem.m_pSubGroup->SetTitle(sTitle);
  m_pGrid->SetCollapseState(elem.m_pSubGroup);
  elem.m_pWidget->SetSelection(SubItems);
}

/// *** nsQtPropertyTypeContainerWidget ***

nsQtPropertyTypeContainerWidget::nsQtPropertyTypeContainerWidget()

  = default;

nsQtPropertyTypeContainerWidget::~nsQtPropertyTypeContainerWidget()
{
  m_pGrid->GetDocument()->GetObjectManager()->m_StructureEvents.RemoveEventHandler(
    nsMakeDelegate(&nsQtPropertyTypeContainerWidget::StructureEventHandler, this));
  m_pGrid->GetCommandHistory()->m_Events.RemoveEventHandler(nsMakeDelegate(&nsQtPropertyTypeContainerWidget::CommandHistoryEventHandler, this));
}

void nsQtPropertyTypeContainerWidget::OnInit()
{
  nsQtPropertyContainerWidget::OnInit();
  m_pGrid->GetDocument()->GetObjectManager()->m_StructureEvents.AddEventHandler(
    nsMakeDelegate(&nsQtPropertyTypeContainerWidget::StructureEventHandler, this));
  m_pGrid->GetCommandHistory()->m_Events.AddEventHandler(nsMakeDelegate(&nsQtPropertyTypeContainerWidget::CommandHistoryEventHandler, this));
}

void nsQtPropertyTypeContainerWidget::UpdateElement(nsUInt32 index)
{
  Element& elem = m_Elements[index];
  nsHybridArray<nsPropertySelection, 8> SubItems;

  // To be in line with all other nsQtPropertyWidget the container element will
  // be given a selection in the form of this is the parent object, this is the property and in this
  // specific case this is the index you are working on. So SubItems only decorates the items with the correct index.
  for (const auto& item : m_Items)
  {
    nsPropertySelection sel;
    sel.m_pObject = item.m_pObject;
    sel.m_Index = m_Keys[index];

    SubItems.PushBack(sel);
  }

  {
    // To get the correct name we actually need to resolve the selection to the actual objects
    // they are pointing to.
    nsHybridArray<nsPropertySelection, 8> ResolvedObjects;
    for (const auto& item : SubItems)
    {
      nsUuid ObjectGuid = m_pObjectAccessor->Get<nsUuid>(item.m_pObject, m_pProp, item.m_Index);
      nsPropertySelection sel;
      sel.m_pObject = m_pObjectAccessor->GetObject(ObjectGuid);
      ResolvedObjects.PushBack(sel);
    }

    const nsRTTI* pCommonType = nsQtPropertyWidget::GetCommonBaseType(ResolvedObjects);

    // Label
    {
      nsStringBuilder sTitle, tmp;
      sTitle.SetFormat("[{0}] - {1}", m_Keys[index].ConvertTo<nsString>(), nsTranslate(pCommonType->GetTypeName().GetData(tmp)));

      if (auto pInDev = pCommonType->GetAttributeByType<nsInDevelopmentAttribute>())
      {
        sTitle.AppendFormat(" [ {} ]", pInDev->GetString());
      }

      elem.m_pSubGroup->SetTitle(sTitle);
    }

    nsColor borderIconColor = nsColor::MakeZero();

    if (const nsColorAttribute* pColorAttrib = pCommonType->GetAttributeByType<nsColorAttribute>())
    {
      borderIconColor = pColorAttrib->GetColor();
      elem.m_pSubGroup->SetFillColor(nsToQtColor(pColorAttrib->GetColor()));
    }
    else if (const nsCategoryAttribute* pCatAttrib = pCommonType->GetAttributeByType<nsCategoryAttribute>())
    {
      borderIconColor = nsColorScheme::GetCategoryColor(pCatAttrib->GetCategory(), nsColorScheme::CategoryColorUsage::BorderIconColor);
      elem.m_pSubGroup->SetFillColor(nsToQtColor(nsColorScheme::GetCategoryColor(pCatAttrib->GetCategory(), nsColorScheme::CategoryColorUsage::BorderColor)));
    }
    else
    {
      const QPalette& pal = palette();
      elem.m_pSubGroup->SetFillColor(pal.mid().color());
    }

    // Icon
    {
      nsStringBuilder sIconName;
      sIconName.Set(":/TypeIcons/", pCommonType->GetTypeName(), ".svg");
      elem.m_pSubGroup->SetIcon(nsQtUiServices::GetCachedIconResource(sIconName.GetData(), borderIconColor));
    }

    // help URL
    {
      nsStringBuilder tmp;
      QString url = nsMakeQString(nsTranslateHelpURL(pCommonType->GetTypeName().GetData(tmp)));

      if (!url.isEmpty())
      {
        elem.m_pHelpButton->setVisible(true);
        connect(elem.m_pHelpButton, &QToolButton::clicked, this, [=]()
          { QDesktopServices::openUrl(QUrl(url)); });
      }
      else
      {
        elem.m_pHelpButton->setVisible(false);
      }
    }
  }


  m_pGrid->SetCollapseState(elem.m_pSubGroup);
  elem.m_pWidget->SetSelection(SubItems);
}

void nsQtPropertyTypeContainerWidget::StructureEventHandler(const nsDocumentObjectStructureEvent& e)
{
  if (IsUndead())
    return;

  switch (e.m_EventType)
  {
    case nsDocumentObjectStructureEvent::Type::AfterObjectAdded:
    case nsDocumentObjectStructureEvent::Type::AfterObjectMoved:
    case nsDocumentObjectStructureEvent::Type::AfterObjectRemoved:
    {
      if (!e.m_sParentProperty.IsEqual(m_pProp->GetPropertyName()))
        return;

      if (std::none_of(cbegin(m_Items), cend(m_Items),
            [&](const nsPropertySelection& sel)
            { return e.m_pNewParent == sel.m_pObject || e.m_pPreviousParent == sel.m_pObject; }))
        return;

      m_bNeedsUpdate = true;
    }
    break;
    default:
      break;
  }
}

void nsQtPropertyTypeContainerWidget::CommandHistoryEventHandler(const nsCommandHistoryEvent& e)
{
  if (IsUndead())
    return;

  switch (e.m_Type)
  {
    case nsCommandHistoryEvent::Type::UndoEnded:
    case nsCommandHistoryEvent::Type::RedoEnded:
    case nsCommandHistoryEvent::Type::TransactionEnded:
    case nsCommandHistoryEvent::Type::TransactionCanceled:
    {
      if (m_bNeedsUpdate)
      {
        m_bNeedsUpdate = false;
        UpdateElements();
      }
    }
    break;

    default:
      break;
  }
}

/// *** nsQtVariantPropertyWidget ***

nsQtVariantPropertyWidget::nsQtVariantPropertyWidget()
{
  m_pLayout = new QVBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 4);
  m_pLayout->setSpacing(1);
  setLayout(m_pLayout);

  m_pTypeList = new QComboBox(this);
  m_pTypeList->installEventFilter(this);
  m_pTypeList->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
  m_pLayout->addWidget(m_pTypeList);
}

nsQtVariantPropertyWidget::~nsQtVariantPropertyWidget() = default;

void nsQtVariantPropertyWidget::OnInit()
{
  nsStringBuilder sName;
  for (int i = nsVariantType::Invalid; i < nsVariantType::LastExtendedType; ++i)
  {
    auto type = static_cast<nsVariantType::Enum>(i);
    if (GetVariantTypeDisplayName(type, sName).Succeeded())
    {
      m_pTypeList->addItem(nsMakeQString(nsTranslate(sName)), i);
    }
  }

  connect(m_pTypeList, &QComboBox::currentIndexChanged,
    [this](int iIndex)
    {
      ChangeVariantType(static_cast<nsVariantType::Enum>(m_pTypeList->itemData(iIndex).toInt()));
    });
}

void nsQtVariantPropertyWidget::InternalSetValue(const nsVariant& value)
{
  nsVariantType::Enum commonType = nsVariantType::Invalid;
  const bool sameType = GetCommonVariantSubType(m_Items, m_pProp, commonType);
  const nsRTTI* pNewtSubType = commonType != nsVariantType::Invalid ? nsReflectionUtils::GetTypeFromVariant(commonType) : nullptr;
  if (pNewtSubType != m_pCurrentSubType || m_pWidget == nullptr)
  {
    if (m_pWidget)
    {
      m_pWidget->PrepareToDie();
      m_pWidget->deleteLater();
      m_pWidget = nullptr;
    }
    m_pCurrentSubType = pNewtSubType;
    if (pNewtSubType)
    {
      m_pWidget = nsQtPropertyGridWidget::GetFactory().CreateObject(pNewtSubType);

      if (!m_pWidget)
      {
        m_pWidget = new nsQtUnsupportedPropertyWidget("<Unsupported Type>");
      }
    }
    else if (!sameType)
    {
      m_pWidget = new nsQtUnsupportedPropertyWidget("Multi-selection has varying types");
    }
    else
    {
      m_pWidget = new nsQtUnsupportedPropertyWidget("<Invalid Type>");
    }

    m_pWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    m_pWidget->setParent(this);
    m_pLayout->addWidget(m_pWidget);
    m_pWidget->Init(m_pGrid, m_pObjectAccessor, m_pType, m_pProp);

    UpdateTypeListSelection(commonType);
  }
  m_pWidget->SetSelection(m_Items);
}

void nsQtVariantPropertyWidget::DoPrepareToDie()
{
  if (m_pWidget)
    m_pWidget->PrepareToDie();
}

void nsQtVariantPropertyWidget::UpdateTypeListSelection(nsVariantType::Enum type)
{
  nsQtScopedBlockSignals bs(m_pTypeList);
  for (int i = 0; i < m_pTypeList->count(); ++i)
  {
    if (m_pTypeList->itemData(i).toInt() == type)
    {
      m_pTypeList->setCurrentIndex(i);
      break;
    }
  }
}

void nsQtVariantPropertyWidget::ChangeVariantType(nsVariantType::Enum type)
{
  m_pObjectAccessor->StartTransaction("Change variant type");
  // check if we have multiple values
  for (const auto& item : m_Items)
  {
    nsVariant value;
    NS_VERIFY(m_pObjectAccessor->GetValue(item.m_pObject, m_pProp, value, item.m_Index).Succeeded(), "");
    if (value.CanConvertTo(type))
    {
      NS_VERIFY(m_pObjectAccessor->SetValue(item.m_pObject, m_pProp, value.ConvertTo(type), item.m_Index).Succeeded(), "");
    }
    else
    {
      NS_VERIFY(
        m_pObjectAccessor->SetValue(item.m_pObject, m_pProp, nsReflectionUtils::GetDefaultVariantFromType(type), item.m_Index).Succeeded(), "");
    }
  }
  m_pObjectAccessor->FinishTransaction();
}

nsResult nsQtVariantPropertyWidget::GetVariantTypeDisplayName(nsVariantType::Enum type, nsStringBuilder& out_sName) const
{
  if (type == nsVariantType::FirstStandardType || type >= nsVariantType::LastStandardType ||
      type == nsVariantType::StringView || type == nsVariantType::DataBuffer || type == nsVariantType::TempHashedString)
    return NS_FAILURE;

  const nsRTTI* pVariantEnum = nsGetStaticRTTI<nsVariantType>();
  if (nsReflectionUtils::EnumerationToString(pVariantEnum, type, out_sName) == false)
    return NS_FAILURE;

  return NS_SUCCESS;
}
