#include "databasenamedialog.h"
#include "ui_databasenamedialog.h"

DatabaseNameDialog::DatabaseNameDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DatabaseNameDialog)
{
  ui->setupUi(this);

  connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &DatabaseNameDialog::close);
  connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DatabaseNameDialog::accept);
}

DatabaseNameDialog::~DatabaseNameDialog()
{
  delete ui;
}

void DatabaseNameDialog::SetName(const QString &name)
{
  ui->currentNameEdit->setText(name);
  ui->newNameEdit->setText(name);
}

QString DatabaseNameDialog::GetName() const
{
  return(ui->newNameEdit->text());
}
