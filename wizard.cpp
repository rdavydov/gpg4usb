/* *      wizard.cpp
 *
 *      Copyright 2008 gpg4usb-team <gpg4usb@cpunk.de>
 *
 *      This file is part of gpg4usb.
 *
 *      Gpg4usb is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      Gpg4usb is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with gpg4usb.  If not, see <http://www.gnu.org/licenses/>
 */

#include <QtGui>

#include "wizard.h"

Wizard::Wizard(GpgME::GpgContext *ctx, KeyMgmt *keyMgmt, QWidget *parent)
    : QWizard(parent)
{
    mCtx=ctx;
    mKeyMgmt=keyMgmt;

    setPage(Page_Intro,new IntroPage(this));
    setPage(Page_Choose, new ChoosePage(this));
    setPage(Page_ImportFromGpg4usb,new ImportFromGpg4usbPage(mCtx, mKeyMgmt, this));
    setPage(Page_ImportFromGnupg,new ImportFromGnupgPage(mCtx, mKeyMgmt, this));
    setPage(Page_GenKey,new KeyGenPage(mCtx, this));
    setPage(Page_Conclusion,new ConclusionPage(this));
#ifndef Q_WS_MAC
    setWizardStyle(ModernStyle);
#endif
    setWindowTitle(tr("First Start Wizard"));

    // http://www.flickr.com/photos/laureenp/6141822934/
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/keys2.jpg"));
    setPixmap(QWizard::LogoPixmap, QPixmap(":/logo_small.png"));
    setPixmap(QWizard::BannerPixmap, QPixmap(":/banner.png"));

    QSettings settings;
    setStartId(settings.value("wizard/nextPage", -1).toInt());
    settings.remove("wizard/nextPage");

    connect(this, SIGNAL(accepted()), this, SLOT(wizardAccepted()));
    connect(this, SIGNAL(openHelp(QString)), parentWidget(), SLOT(openHelp(QString)));

}

void Wizard::wizardAccepted() {
    QSettings settings;
    // Don't show is mapped to show -> negation
    settings.setValue("wizard/showWizard", !field("showWizard").toBool());

    if(field("openHelp").toBool()) {
        emit openHelp("docu.html#content");
    }
}

bool Wizard::importPubAndSecKeysFromDir(const QString dir, KeyMgmt *keyMgmt)
{
    QFile secRingFile(dir+"/secring.gpg");
    QFile pubRingFile(dir+"/pubring.gpg");

    // Return, if no keyrings are found in subdir of chosen dir
    if (!(pubRingFile.exists() or secRingFile.exists())) {
        QMessageBox::critical(0, tr("Import Error"), tr("Couldn't locate any keyring file in %1").arg(dir));
        return false;
    }

    QByteArray inBuffer;
    if (secRingFile.exists()) {
        // write content of secringfile to inBuffer
        if (!secRingFile.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(0, tr("Import error"), tr("Couldn't open private keyringfile: %1").arg(secRingFile.fileName()));
            return false;
        }
        inBuffer = secRingFile.readAll();
        secRingFile.close();
    }

    if (pubRingFile.exists()) {
        // try to import public keys
        if (!pubRingFile.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(0, tr("Import error"), tr("Couldn't open public keyringfile: %1").arg(pubRingFile.fileName()));
            return false;
        }
        inBuffer.append(pubRingFile.readAll());
        pubRingFile.close();
    }
    keyMgmt->importKeys(inBuffer);
    inBuffer.clear();

    return true;
}

IntroPage::IntroPage(QWidget *parent)
     : QWizardPage(parent)
{
    setTitle(tr("Introduction"));
    setSubTitle(tr("About this wizard."));

    topLabel = new QLabel(tr("This wizard will help you getting started by importing settings and keys"
                             "from an older version of gpg4usb, import keys from a locally installed Gnupg or to "
                             "generate a new key to encrypt and decrypt."));
    topLabel->setWordWrap(true);

    // QComboBox for language selection
    langLabel = new QLabel(tr("Choose a Language"));
    langLabel->setWordWrap(true);

    languages = SettingsDialog::listLanguages();
    langSelectBox = new QComboBox();
    foreach(QString l, languages) {
        langSelectBox->addItem(l);
    }
    // selected entry from config
    QSettings settings;
    QString langKey = settings.value("int/lang").toString();
    QString langValue = languages.value(langKey);
    if (langKey != "") {
        langSelectBox->setCurrentIndex(langSelectBox->findText(langValue));
    }

    connect(langSelectBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(langChange(QString)));

    // set layout and add widgets
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(topLabel);
    layout->addWidget(langLabel);
    layout->addWidget(langSelectBox);
    setLayout(layout);
}

void IntroPage::langChange(QString lang) {
    QSettings settings;
    settings.setValue("int/lang", languages.key(lang));
    settings.setValue("wizard/nextPage", this->wizard()->currentId());
    qApp->exit(RESTART_CODE);
}

int IntroPage::nextId() const
{
    return Wizard::Page_Choose;
}
ChoosePage::ChoosePage(QWidget *parent)
     : QWizardPage(parent)
{
    setTitle(tr("Choose your action..."));
    setSubTitle(tr("...by clicking on the apropriate link."));
    QLabel *topLabel = new QLabel(tr("First you've got to create an own keypair.<br/>"
                             "The pair contains a public and a private key.<br/>"
                             "Other users can use the public key to encrypt texts for you<br/>"
                             "and verify texts signed by you.<br/>"
                             "You can use the private key to decrypt and sign texts.<br/>"
                             "For more information have a look in the offline tutorial (which then is shown in the main window:"));

    //QGroupBox *keygenBox = new QGroupBox("Create a new Key");
    //QVBoxLayout *keygenBoxLayout = new QVBoxLayout(keygenBox);
    QLabel *keygenLabel = new QLabel(tr("If you have never used gpg4usb before and also don't own a gpg key yet you "
                                     "may possibly want to ")+"<a href=""Wizard::Page_GenKey"">"
                                     +tr("create a private key")+"</a><hr>");
    keygenLabel->setWordWrap(true);
    //QLabel *keygenLinkLabel = new QLabel("<a href=""Wizard::Page_GenKey"">"+tr("create your private key")+"</a><hr>");
    connect(keygenLabel, SIGNAL(linkActivated(const QString&)), this, SLOT(jumpPage(const QString&)));
    //keygenBoxLayout->addWidget(keygenLabel);
    //keygenBoxLayout->addWidget(keygenLinkLabel);

    //QGroupBox *importGpg4usbBox = new QGroupBox("Import gpg4usb");
    //QVBoxLayout *importGpg4usbBoxLayout = new QVBoxLayout(importGpg4usbBox);
    QLabel *importGpg4usbLabel = new QLabel(tr("If you upgrade from an older version of gpg4usb you may want to ")
                                            +"<a href=""Wizard::Page_ImportFromGpg4usb"">"
                                            +tr("import settings and/or keys from Gpg4usb")+"</a>");
    importGpg4usbLabel->setWordWrap(true);
    //QLabel *importGpg4usbLinkLabel = new QLabel("<a href=""Wizard::Page_ImportFromGpg4usb"">"+tr("import settings and/or keys from Gpg4usb")+"</a>");
    connect(importGpg4usbLabel, SIGNAL(linkActivated(const QString&)), this, SLOT(jumpPage(const QString&)));
    //importGpg4usbBoxLayout->addWidget(importGpg4usbLabel);
    //importGpg4usbBoxLayout->addWidget(importGpg4usbLinkLabel);

    //QGroupBox *importGnupgBox = new QGroupBox("import gnupg");
    //QVBoxLayout *importGnupgBoxLayout = new QVBoxLayout(importGnupgBox);
    QLabel *importGnupgLabel = new QLabel(tr("If you are already using Gnupg you may want to ")
                                          +"<a href=""Wizard::Page_ImportFromGnupg"">"
                                          +tr("import keys from Gnupg")+"</a><hr>");
    importGnupgLabel->setWordWrap(true);
    //QLabel *importGnupgLinkLabel = new QLabel("<a href=""Wizard::Page_ImportFromGnupg"">"+tr("import keys from Gnupg")+"</a><hr>");
    connect(importGnupgLabel, SIGNAL(linkActivated(const QString&)), this, SLOT(jumpPage(const QString&)));
    //importGnupgBoxLayout->addWidget(importGnupgLabel);
    //importGnupgBoxLayout->addWidget(importGnupgLinkLabel);

    QVBoxLayout *layout = new QVBoxLayout();
    //layout->addWidget(keygenBox);
    //layout->addWidget(importGpg4usbBox);
    //layout->addWidget(importGnupgBox);
    layout->addWidget(keygenLabel);
//    layout->addWidget(keygenLinkLabel);
    layout->addWidget(importGnupgLabel);
//    layout->addWidget(importGnupgLinkLabel);
    layout->addWidget(importGpg4usbLabel);
//    layout->addWidget(importGpg4usbLinkLabel);
    setLayout(layout);
    nextPage=Wizard::Page_Conclusion;
}

int ChoosePage::nextId() const
{
    return nextPage;
}

void ChoosePage::jumpPage(const QString& page)
{
    QMetaObject qmo = Wizard::staticMetaObject;
    int index = qmo.indexOfEnumerator("WizardPages");
    QMetaEnum m = qmo.enumerator(index);

    nextPage = m.keyToValue(page.toAscii().data());
    wizard()->next();
}

ImportFromGpg4usbPage::ImportFromGpg4usbPage(GpgME::GpgContext *ctx, KeyMgmt *keyMgmt, QWidget *parent)
     : QWizardPage(parent)
{
    mCtx=ctx;
    mKeyMgmt=keyMgmt;
    setTitle(tr("Keyring Import"));
    setSubTitle("bla");

    QLabel *topLabel = new QLabel(tr("Import keys and/or settings from older gpg4usb. Just check, what you want to "
                                  "import, click the import button and choose the directory "
                                  "of your old gpg4usb in the appearing file dialog."), this);
    topLabel->setWordWrap(true);

    gpg4usbKeyCheckBox = new QCheckBox();
    gpg4usbKeyCheckBox->setChecked(true);
    QLabel *keyLabel = new QLabel(tr("Keys"));

    gpg4usbConfigCheckBox = new QCheckBox();
    gpg4usbConfigCheckBox->setChecked(true);
    QLabel *configLabel = new QLabel(tr("Configuration"));

    importFromGpg4usbButton = new QPushButton(tr("Import from older gpg4usb"));
    connect(importFromGpg4usbButton, SIGNAL(clicked()), this, SLOT(importFormOlderGpg4usb()));

    QGridLayout *gpg4usbLayout = new QGridLayout();
    gpg4usbLayout->addWidget(topLabel,1,1,1,2);
    gpg4usbLayout->addWidget(gpg4usbKeyCheckBox,2,1,Qt::AlignRight);
    gpg4usbLayout->addWidget(keyLabel,2,2);
    gpg4usbLayout->addWidget(gpg4usbConfigCheckBox,3,1,Qt::AlignRight);
    gpg4usbLayout->addWidget(configLabel,3,2);
    gpg4usbLayout->addWidget(importFromGpg4usbButton,4,2);

    this->setLayout(gpg4usbLayout);
}

bool ImportFromGpg4usbPage::importFormOlderGpg4usb()
{
    QString dir = QFileDialog::getExistingDirectory(this,tr("Old gpg4usb directory"));

    // Return, if cancel was hit
    if (dir.isEmpty()) {
        return false;
    }

    // try to import keys, if appropriate box is checked, return, if import was unsuccessful
    if (gpg4usbKeyCheckBox->isChecked()) {
        if (!Wizard::importPubAndSecKeysFromDir(dir+"/keydb",mKeyMgmt)) {
            return false;
        }
    }

    // try to import config, if appropriate box is checked
    if (gpg4usbConfigCheckBox->isChecked()) {
        importConfFromGpg4usb(dir);

        QSettings settings;
        settings.setValue("wizard/nextPage", this->nextId());
        QMessageBox::information(0,tr("Configuration Imported"),tr("Imported Configuration from old gpg4usb.<br/>"
                                      "Will now restart to activate the configuration."));
        // TODO: edit->maybesave?
        qApp->exit(RESTART_CODE);
    }
    return true;
}

bool ImportFromGpg4usbPage::importConfFromGpg4usb(QString dir) {
    QString path = dir+"/conf/gpg4usb.ini";
    QSettings oldconf(path, QSettings::IniFormat, this);
    QSettings actualConf;
    foreach(QString key, oldconf.allKeys()) {
        actualConf.setValue(key, oldconf.value(key));
    }
    return true;
}

int ImportFromGpg4usbPage::nextId() const
{
    return Wizard::Page_ImportFromGnupg;
}

ImportFromGnupgPage::ImportFromGnupgPage(GpgME::GpgContext *ctx, KeyMgmt *keyMgmt, QWidget *parent)
     : QWizardPage(parent)
{
    mCtx=ctx;
    mKeyMgmt=keyMgmt;
    setTitle(tr("Key import from Gnupg"));
    setSubTitle("bla");

    QLabel *gnupgLabel = new QLabel(tr("Should I try to import keys from a locally installed GnuPG?<br/> The location is read "
                               "from registry in Windows and assumed to be the .gnupg folder in the your home directory in Linux"));
    gnupgLabel->setWordWrap(true);

    importFromGnupgButton = new QPushButton(tr("Import keys from GnuPG"));
    connect(importFromGnupgButton, SIGNAL(clicked()), this, SLOT(importKeysFromGnupg()));

    QGridLayout *layout = new QGridLayout();
    layout->addWidget(gnupgLabel);
    layout->addWidget(importFromGnupgButton);

    this->setLayout(layout);
}

bool ImportFromGnupgPage::importKeysFromGnupg()
{
    // first get gnupghomedir and check, if it exists
    QString gnuPGHome = getGnuPGHome();
    if (gnuPGHome == NULL) {
        QMessageBox::critical(0, tr("Import Error"), tr("Couldn't locate GnuPG home directory"));
        return false;
    }

    // Try to import the keyring files and return the return value of the method
    return Wizard::importPubAndSecKeysFromDir(gnuPGHome,mKeyMgmt);;
}

QString ImportFromGnupgPage::getGnuPGHome()
{
    QString gnuPGHome="";
    #ifdef _WIN32
        QSettings gnuPGsettings("HKEY_CURRENT_USER\\Software\\GNU\\GNUPG", QSettings::NativeFormat);
        gnuPGHome = gnuPGsettings.value("HomeDir").toString();
        if (gnuPGHome.isEmpty()) {
            return NULL;
        }

    #else
        gnuPGHome=QDir::homePath()+"/.gnupg";
        if (! QFile(gnuPGHome).exists()) {
            return NULL;
        }
    #endif

    return gnuPGHome;
}

int ImportFromGnupgPage::nextId() const
{
    return Wizard::Page_Conclusion;
}

KeyGenPage::KeyGenPage(GpgME::GpgContext *ctx, QWidget *parent)
     : QWizardPage(parent)
{

    //setPixmap(QWizard::WatermarkPixmap, QPixmap(":/logo-flipped.png"));
    mCtx=ctx;
    setTitle(tr("Key-Generating"));
    setSubTitle("bla");
    topLabel = new QLabel(tr("First you've got to create an own keypair.<br/>"
                             "The pair contains a public and a private key.<br/>"
                             "Other users can use the public key to encrypt texts for you<br/>"
                             "and verify texts signed by you.<br/>"
                             "You can use the private key to decrypt and sign texts.<br/>"
                             "For more information have a look in the offline tutorial (which then is shown in the main window:"));
    QLabel *linkLabel = new QLabel("<a href=""docu_keygen.html#content"">"+tr("Offline tutorial")+"</a>");
    //linkLabel->setOpenExternalLinks(true);

    connect(linkLabel, SIGNAL(linkActivated(const QString&)), parentWidget()->parentWidget(), SLOT(openHelp(const QString&)));

    QWidget *createKeyButtonBox = new QWidget(this);
    QHBoxLayout  *createKeyButtonBoxLayout = new QHBoxLayout(createKeyButtonBox);
    createKeyButton = new QPushButton(tr("Create New Key"));
    createKeyButtonBoxLayout->addWidget(createKeyButton);
    createKeyButtonBoxLayout->addStretch(1);
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(topLabel);
    layout->addWidget(linkLabel);
    layout->addWidget(createKeyButtonBox);
    connect(createKeyButton, SIGNAL(clicked()), this, SLOT(generateKeyDialog()));

    setLayout(layout);
}

int KeyGenPage::nextId() const
{
    return Wizard::Page_Conclusion;
}

void KeyGenPage::generateKeyDialog()
{
    KeyGenDialog *keyGenDialog = new KeyGenDialog(mCtx, this);
    keyGenDialog->show();
}

ConclusionPage::ConclusionPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Finish Start Wizard"));
    setSubTitle("bla");

    bottomLabel = new QLabel(tr("You're ready to encrypt and decrpt now."));
    bottomLabel->setWordWrap(true);

    dontShowWizardCheckBox = new QCheckBox(tr("Dont show the wizard again."));
    dontShowWizardCheckBox->setChecked(Qt::Checked);

    openHelpCheckBox = new QCheckBox(tr("Open offline help."));
    openHelpCheckBox->setChecked(Qt::Checked);

    registerField("showWizard", dontShowWizardCheckBox);
    registerField("openHelp", openHelpCheckBox);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(bottomLabel);
    layout->addWidget(dontShowWizardCheckBox);
    layout->addWidget(openHelpCheckBox);
    setLayout(layout);
    setVisible(true);
}

int ConclusionPage::nextId() const
{
    return -1;
}
