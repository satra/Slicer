/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QDesktopWidget>
#include <QListView>

// CTK includes
#include <ctkVTKScalarsToColorsUtils.h>
#include <ctkWidgetsUtils.h>

// MRMLWidgets includes
#include <qMRMLSceneModel.h>

// VolumeRenderingWidgets includes
#include "qSlicerPresetComboBox_p.h"

// MRML includes
#include <vtkMRMLVolumePropertyNode.h>

// VTK includes
#include <vtkColorTransferFunction.h>
#include <vtkVolumeProperty.h>

// STD includes
#include <cassert>

//-----------------------------------------------------------------------------
qSlicerIconComboBox::qSlicerIconComboBox(QWidget* parentWidget)
  :Superclass(parentWidget)
{
  QListView* listView = new QListView(0);
  listView->setViewMode(QListView::IconMode);
  listView->setUniformItemSizes(true);
  listView->setWrapping(true);
  listView->setMovement(QListView::Static);
  listView->setFlow(QListView::LeftToRight);
  listView->setSpacing(0);
  this->setView(listView);
}

//-----------------------------------------------------------------------------
void qSlicerIconComboBox::showPopup()
{
  QFrame* container = qobject_cast<QFrame*>(this->view()->parentWidget());
  QStyleOptionComboBox opt;
  initStyleOption(&opt);

  QRect listRect(this->style()->subControlRect(QStyle::CC_ComboBox, &opt,
                                               QStyle::SC_ComboBoxListBoxPopup, this));
  QRect screen = QApplication::desktop()->availableGeometry(
    QApplication::desktop()->screenNumber(this));
  QPoint below = mapToGlobal(listRect.bottomLeft());
  //int belowHeight = screen.bottom() - below.y();
  QPoint above = mapToGlobal(listRect.topLeft());
  //int aboveHeight = above.y() - screen.y();
    
  // CustomSize
  //
  const int numberOfItems = this->count();
  QSize itemSize = QSize( this->view()->sizeHintForColumn(0), this->view()->sizeHintForRow(0));
  const int itemsPerRow = itemSize.width() ? listRect.width() / itemSize.width() : 0;
  const int itemsPerColumns = itemsPerRow ? static_cast<int>(0.999 + static_cast<double>(numberOfItems) / itemsPerRow) : 0;
  listRect.setWidth( itemsPerRow * itemSize.width() );
  listRect.setHeight( itemsPerColumns * itemSize.height() );
  
  int marginLeft, marginTop, marginRight, marginBottom;
  container->getContentsMargins(&marginLeft, &marginTop, &marginRight, &marginBottom);
  listRect.setWidth( listRect.width() + marginLeft + marginRight);
  listRect.setWidth( listRect.width() + container->frameWidth());
  listRect.setHeight( listRect.height() + marginTop + marginBottom);
  listRect.setHeight( listRect.height() + container->frameWidth());
  
  // Position horizontally.
  listRect.moveLeft(above.x());

  // Position vertically so the curently selected item lines up
  // with the combo box.
  const QRect currentItemRect = view()->visualRect(view()->currentIndex());
  const int offset = listRect.top() - currentItemRect.top();
  listRect.moveTop(above.y() 
                   + offset
                   - listRect.top());

  if (listRect.width() > screen.width() )
      listRect.setWidth(screen.width());
  if (mapToGlobal(listRect.bottomRight()).x() > screen.right()) {
      below.setX(screen.x() + screen.width() - listRect.width());
      above.setX(screen.x() + screen.width() - listRect.width());
  }
  if (mapToGlobal(listRect.topLeft()).x() < screen.x() ) {
      below.setX(screen.x());
      above.setX(screen.x());
  }
  container->setGeometry(listRect);
  container->raise();
  container->show();
  this->view()->setFocus();
  this->view()->scrollTo(this->view()->currentIndex(),
                     this->style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, this)
                             ? QAbstractItemView::PositionAtCenter
                             : QAbstractItemView::EnsureVisible);
  container->update();
}
  
//-----------------------------------------------------------------------------
qSlicerPresetComboBoxPrivate::qSlicerPresetComboBoxPrivate(
  qSlicerPresetComboBox& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
void qSlicerPresetComboBoxPrivate::init()
{
  Q_Q(qSlicerPresetComboBox);
  // We don't want to reset default text.
  //q->qMRMLNodeComboBox::d_ptr->AutoDefaultText = false;
  q->setNodeTypes(QStringList("vtkMRMLVolumePropertyNode"));
  q->setSelectNodeUponCreation(false);
  q->setAddEnabled(false);
  q->setRemoveEnabled(false);
  q->setBaseName(q->tr("Preset"));
  
  qSlicerIconComboBox* comboBox = new qSlicerIconComboBox;
  //comboBox->setDefaultIcon(QIcon());
  //comboBox->setDefaultText(q->tr("Select a Preset"));
  comboBox->forceDefault(true);
  q->setComboBox(comboBox);
  q->updateComboBoxTitleAndIcon(0);
  //comboBox->setMaximumHeight(comboBox->sizeHint().height());
  
  qMRMLSceneModel* sceneModel =
    qobject_cast<qMRMLSceneModel*>(q->sortFilterProxyModel()->sourceModel());
  sceneModel->setNameColumn(-1);
  //sceneModel->setToolTipNameColumn(0);

  QObject::connect(q, SIGNAL(nodeAdded(vtkMRMLNode*)),
                   q, SLOT(setIconToPreset(vtkMRMLNode*)));
  QObject::connect(q, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(updateComboBoxTitleAndIcon(vtkMRMLNode*)));
}

// --------------------------------------------------------------------------
qSlicerPresetComboBox
::qSlicerPresetComboBox(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qSlicerPresetComboBoxPrivate(*this))
{
  Q_D(qSlicerPresetComboBox);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerPresetComboBox
::~qSlicerPresetComboBox()
{
}

// --------------------------------------------------------------------------
void qSlicerPresetComboBox::setIconToPreset(vtkMRMLNode* presetNode)
{
  // Search corresponding icon
  QIcon presetIcon(QString(":/presets/") + presetNode->GetName());
  if (presetIcon.isNull())
    {
    return;
    }
  // Set icon
  qMRMLSceneModel* sceneModel = qobject_cast<qMRMLSceneModel*>(this->sortFilterProxyModel()->sourceModel());
  sceneModel->setData(sceneModel->indexFromNode(presetNode), presetIcon, Qt::DecorationRole);
  
  // Update icon size
  QList<QSize> iconSizes = presetIcon.availableSizes();
  if (iconSizes.count())
    {
    QSize iconSize = this->comboBox()->view()->iconSize();
    this->comboBox()->view()->setIconSize(QSize(qMax(iconSize.width(), iconSizes[0].width()),
                                                qMax(iconSize.height(), iconSizes[0].height())));
    }
  
  // Set toolTip
  vtkMRMLVolumePropertyNode* volumePropertyNode = vtkMRMLVolumePropertyNode::SafeDownCast(presetNode);
  if (volumePropertyNode)
    {
    int previewSize = this->style()->pixelMetric(QStyle::PM_SmallIconSize);
    vtkScalarsToColors* colors =
      volumePropertyNode->GetVolumeProperty() ? volumePropertyNode->GetVolumeProperty()->GetRGBTransferFunction() : 0;
    assert(colors && colors->GetRange()[1] > colors->GetRange()[0]);
    QImage img = ctk::scalarsToColorsImage(colors, QSize(previewSize, previewSize));
    QString toolTip = QString("<img src=\"%1\"> %2").arg(ctk::base64HTMLImageTagSrc(img)).arg(presetNode->GetName());
    sceneModel->setData(sceneModel->indexFromNode(presetNode), toolTip, Qt::ToolTipRole);
    }
}

// --------------------------------------------------------------------------
void qSlicerPresetComboBox::updateComboBoxTitleAndIcon(vtkMRMLNode* node)
{
  ctkComboBox* combo = qobject_cast<ctkComboBox*>(this->comboBox());
  if (node)
    {
    combo->setDefaultText(node->GetName());
    combo->setDefaultIcon(combo->itemIcon(combo->currentIndex()));
    }
  else
    {
    combo->setDefaultText(tr("Select a Preset"));
    combo->setDefaultIcon(QIcon());
    }
}
