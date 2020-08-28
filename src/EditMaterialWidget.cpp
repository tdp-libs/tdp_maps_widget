#include "tp_qt_maps_widget/EditMaterialWidget.h"

#include <QDialog>
#include <QBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QPushButton>
#include <QPointer>
#include <QDialogButtonBox>
#include <QColorDialog>
#include <QImage>
#include <QPixmap>
#include <QLineEdit>
#include <QPainter>

namespace tp_qt_maps_widget
{

//##################################################################################################
struct EditMaterialWidget::Private
{
  tp_maps::Material material;

  QLineEdit* nameEdit{nullptr};

  QPushButton* ambientColorButton {nullptr};
  QPushButton* diffuseColorButton {nullptr};
  QPushButton* specularColorButton{nullptr};

  QSlider* shininess{nullptr};
  QSlider* alpha    {nullptr};

  QLineEdit* ambientTexture {nullptr}; //!< mtl: map_Ka
  QLineEdit* diffuseTexture {nullptr}; //!< mtl: map_Kd
  QLineEdit* specularTexture{nullptr}; //!< mtl: map_Ks
  QLineEdit* alphaTexture   {nullptr}; //!< mtl: map_d
  QLineEdit* bumpTexture    {nullptr}; //!< mtl: map_Bump

  //################################################################################################
  void updateColors()
  {
    auto makeIcon = [](const glm::vec3& c)
    {
      QImage image(24, 24, QImage::Format_ARGB32);
      image.fill(QColor(0,0,0,0));
      {
        QPainter p(&image);
        p.setBrush(QColor::fromRgbF(c.x, c.y, c.z));
        p.setPen(Qt::black);
        p.drawRoundedRect(2,2,20,20,2.0, 2.0);
      }
      return QIcon(QPixmap::fromImage(image));
    };

    ambientColorButton ->setIcon(makeIcon(material.ambient ));
    diffuseColorButton ->setIcon(makeIcon(material.diffuse ));
    specularColorButton->setIcon(makeIcon(material.specular));
  }
};

//##################################################################################################
EditMaterialWidget::EditMaterialWidget(QWidget* parent):
  QWidget(parent),
  d(new Private())
{
  auto l = new QVBoxLayout(this);
  l->setContentsMargins(0,0,0,0);

  l->addWidget(new QLabel("Name"));
  d->nameEdit = new QLineEdit();
  l->addWidget(d->nameEdit);
  connect(d->nameEdit, &QLineEdit::editingFinished, this, &EditMaterialWidget::materialEdited);

  {
    l->addWidget(new QLabel("Colors"));

    auto ll = new QHBoxLayout();
    ll->setContentsMargins(0,0,0,0);
    l->addLayout(ll);

    auto make = [&](const QString& text, const std::function<glm::vec3&()>& getColor)
    {
      auto button = new QPushButton(text);
      button->setStyleSheet("text-align:left; padding-left:2;");
      ll->addWidget(button);

      connect(button, &QAbstractButton::clicked, this, [=]
      {
        glm::vec3& c = getColor();
        QColor color = QColorDialog::getColor(QColor::fromRgbF(c.x, c.y, c.z), this, "Select " + text + " color", QColorDialog::DontUseNativeDialog);
        if(color.isValid())
        {
          c.x = color.redF();
          c.y = color.greenF();
          c.z = color.blueF();
          d->updateColors();
          emit materialEdited();
        }
      });

      return button;
    };

    d->ambientColorButton  = make("Ambient" , [&]()->glm::vec3&{return d->material.ambient;});
    d->diffuseColorButton  = make("Diffuse" , [&]()->glm::vec3&{return d->material.diffuse;});
    d->specularColorButton = make("Specular", [&]()->glm::vec3&{return d->material.specular;});
  }

  {
    l->addWidget(new QLabel("Shininess and alpha"));
    d->shininess = new QSlider(Qt::Horizontal);
    l->addWidget(d->shininess);
    d->shininess->setRange(0, 12800);
    d->shininess->setSingleStep(1);
    connect(d->shininess, &QSlider::valueChanged, this, &EditMaterialWidget::materialEdited);

    d->alpha = new QSlider(Qt::Horizontal);
    l->addWidget(d->alpha);
    d->alpha->setRange(0, 255);
    d->alpha->setSingleStep(1);
    connect(d->alpha, &QSlider::valueChanged, this, &EditMaterialWidget::materialEdited);
  }

  auto addTextureEdit = [&](const auto& name)
  {
    l->addWidget(new QLabel(name));
    auto edit = new QLineEdit();
    l->addWidget(edit);
    connect(edit, &QLineEdit::editingFinished, this, &EditMaterialWidget::materialEdited);
    return edit;
  };

  d->ambientTexture  = addTextureEdit("Ambient texture");
  d->diffuseTexture  = addTextureEdit("Diffuse texture");
  d->specularTexture = addTextureEdit("Specular texture");
  d->alphaTexture    = addTextureEdit("Alpha texture");
  d->bumpTexture     = addTextureEdit("Bump texture");
}

//##################################################################################################
EditMaterialWidget::~EditMaterialWidget()
{
  delete d;
}

//##################################################################################################
void EditMaterialWidget::setMaterial(const tp_maps::Material& material)
{
  blockSignals(true);
  TP_CLEANUP([&]{blockSignals(false);});

  d->material = material;

  d->nameEdit->setText(QString::fromStdString(material.name.keyString()));

  d->updateColors();

  d->shininess->setValue(int(material.shininess*100.0f));
  d->alpha    ->setValue(int(material.alpha    *255.0f));

  d->ambientTexture ->setText(QString::fromStdString(d->material.ambientTexture .keyString()));
  d->diffuseTexture ->setText(QString::fromStdString(d->material.diffuseTexture .keyString()));
  d->specularTexture->setText(QString::fromStdString(d->material.specularTexture.keyString()));
  d->alphaTexture   ->setText(QString::fromStdString(d->material.alphaTexture   .keyString()));
  d->bumpTexture    ->setText(QString::fromStdString(d->material.bumpTexture    .keyString()));
}

//##################################################################################################
tp_maps::Material EditMaterialWidget::material() const
{
  d->material.name = d->nameEdit->text().toStdString();

  d->material.shininess = float(d->shininess->value()) / 100.0f;
  d->material.alpha     = float(d->alpha    ->value()) / 255.0f;

  d->material.ambientTexture  = d->ambientTexture ->text().toStdString();
  d->material.diffuseTexture  = d->diffuseTexture ->text().toStdString();
  d->material.specularTexture = d->specularTexture->text().toStdString();
  d->material.alphaTexture    = d->alphaTexture   ->text().toStdString();
  d->material.bumpTexture     = d->bumpTexture    ->text().toStdString();

  return d->material;
}

//##################################################################################################
bool EditMaterialWidget::editMaterialDialog(QWidget* parent, tp_maps::Material& material)
{
  QPointer<QDialog> dialog = new QDialog(parent);
  TP_CLEANUP([&]{delete dialog;});

  dialog->setWindowTitle("Edit Material");

  auto l = new QVBoxLayout(dialog);

  auto editMaterialWidget = new EditMaterialWidget();
  l->addWidget(editMaterialWidget);
  editMaterialWidget->setMaterial(material);

  auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  l->addWidget(buttons);

  connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

  if(dialog->exec() == QDialog::Accepted)
  {
    material = editMaterialWidget->material();
    return true;
  }

  return false;
}

}