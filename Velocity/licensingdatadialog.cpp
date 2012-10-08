#include "licensingdatadialog.h"
#include "ui_licensingdatadialog.h"

LicenseTypeStruct licenseTypes[] =
{
    { "Unused", 0 },
    { "Unrestricted", 0xFFFF },
    { "Console Profile", 9 },
    { "Windows Profile", 3 },
    { "Console", 0xF000 },
    { "Unknown1", 0xE000 },
    { "Unknown2", 0xD000 },
    { "Unknown3", 0xC000 },
    { "Unknown4", 0xB000 },
};

LicensingDataDialog::LicensingDataDialog(LicenseEntry *entryTable, bool unlockable, QWidget *parent) : QDialog(parent), ui(new Ui::LicensingDataDialog), entryTable(entryTable)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);

    ui->tableWidget->horizontalHeader()->resizeSection(0, 125);

    ui->btnUnlock->setEnabled(unlockable);

    // load all the license entries
    for (DWORD i = 0; i < 0x10; i++)
    {
        ui->tableWidget->insertRow(i);

        cmbxLicenseTypes[i] = new QComboBox;
        for (DWORD x = 0; x < 9; x++)
        {
            cmbxLicenseTypes[i]->addItem(licenseTypes[x].name);
            if (entryTable[i].type == licenseTypes[x].value)
                cmbxLicenseTypes[i]->setCurrentIndex(x);
        }

        ui->tableWidget->setCellWidget(i, 0, cmbxLicenseTypes[i]);
        ui->tableWidget->setItem(i, 1, new QTableWidgetItem("0x" + QString::number(entryTable[i].data, 16).toUpper()));
        ui->tableWidget->setItem(i, 2, new QTableWidgetItem("0x" + QString::number(entryTable[i].bits, 16).toUpper()));
        ui->tableWidget->setItem(i, 3, new QTableWidgetItem("0x" + QString::number(entryTable[i].flags, 16).toUpper()));
    }
}

LicensingDataDialog::~LicensingDataDialog()
{
    delete ui;
}

void LicensingDataDialog::on_pushButton_clicked()
{
    // verify all the values
    for (DWORD i = 0; i < 0x10; i++)
    {
        if (!QtHelpers::VerifyHexString(ui->tableWidget->item(i, 1)->text()))
        {
            QMessageBox::warning(this, "Invalid Value", "The License ID in row " + QString::number(i + 1) + " must be all hexadecimal digits.\n");
            return;
        }
        if (!QtHelpers::VerifyHexString(ui->tableWidget->item(i, 2)->text()))
        {
            QMessageBox::warning(this, "Invalid Value", "The Bits in row " + QString::number(i + 1) + " must be all hexadecimal digits.\n");
            return;
        }
        if (!QtHelpers::VerifyHexString(ui->tableWidget->item(i, 3)->text()))
        {
            QMessageBox::warning(this, "Invalid Value", "The Flags in row " + QString::number(i + 1) + " must be all hexadecimal digits.\n");
            return;
        }
    }

    // update all the license entries
    for (DWORD i = 0; i < 0x10; i++)
    {
        entryTable[i].type = licenseTypes[cmbxLicenseTypes[i]->currentIndex()].value;
        entryTable[i].data = ui->tableWidget->item(i, 1)->text().replace("0x", "").toULongLong(0, 16);
        entryTable[i].bits = QtHelpers::ParseHexString(ui->tableWidget->item(i, 2)->text());
        entryTable[i].flags = QtHelpers::ParseHexString(ui->tableWidget->item(i, 3)->text());
    }

    this->close();
}

void LicensingDataDialog::on_pushButton_2_clicked()
{
    this->close();
}

void LicensingDataDialog::on_btnUnlock_clicked()
{
    QMessageBox::StandardButton btn = QMessageBox::question(this, "Continue?", "Content unlocked this way can only be used on a JTAG/RGH/Dev. If you try and play on a retail, it will show up as corrupt.\n\nAre you sure that you want to unlock this content?",
                                                            QMessageBox::Yes, QMessageBox::No);

    if (btn != QMessageBox::Yes)
        return;

    entryTable[0].type = Unrestricted;
    entryTable[0].data = 0xFFFFFFFFFFFF;
    entryTable[0].bits = 1;

    cmbxLicenseTypes[0]->setCurrentIndex(1);
    ui->tableWidget->item(0, 1)->setText("0xFFFFFFFFFFFF");
    ui->tableWidget->item(0, 2)->setText("0x1");

    for (DWORD i = 1; i < 0x10; i++)
    {
        entryTable[0].bits = 0;
        ui->tableWidget->item(i, 2)->setText("0x0");
    }
}

