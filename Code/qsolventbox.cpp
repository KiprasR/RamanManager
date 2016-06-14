#include "qsolventbox.h"

QSolventBox::QSolventBox(QWidget *parent) :
    QComboBox(parent), ActiveDir(NULL)
{
    setFocusPolicy(Qt::ClickFocus);

    Map.insert("Custom", "");
    Map.insert("TestSpec", ":/GS_Spectra/TestSpec.txt");
    Map.insert("Acetonitrile", ":/GS_Spectra/Acetonitrile.txt");
    Map.insert("Acetone", ":/GS_Spectra/Acetone.txt");
    Map.insert("Methanol", ":/GS_Spectra/Methanol.txt");
    Map.insert("Toluene", ":/GS_Spectra/Toluene.txt");
    Map.insert("Chloroform", ":/GS_Spectra/Chloroform.txt");
    Map.insert("N-Hexane", ":/GS_Spectra/N-Hexane.txt");
    Map.insert("THF", ":/GS_Spectra/THF.txt");

    int i=0; // Sunumeravimas nebutinas.
    QMapIterator<QString, QString> Mi(Map);
    while (Mi.hasNext())
    {
        Mi.next();
        if (Mi.key()!="TestSpec" && Mi.key()!="Custom") addItem(Mi.key(),i++);
        // Testas - pasleptas, custom - paskutinis.
    }
    addItem("Custom",i);
    connect(this,SIGNAL(currentIndexChanged(int)),this,SLOT(solventIndexChanged(int)));
}

void QSolventBox::setDirSettings(QSettings *actDir)
{
    ActiveDir=actDir;
}

QString QSolventBox::getCurrentSolvent()
{
    return Map.value(currentText());
}

QString QSolventBox::getMapValue(QString key)
{
    return Map.value(key);
}

void QSolventBox::solventIndexChanged(int arg)
{
    if (currentText()=="Custom")
    {
        qDebug() << "Changed to custom.";
        QString fileName = QFileDialog::getOpenFileName(this,tr("Open Solvent Reference"),
                                                             (ActiveDir==NULL ? "C:/" : ActiveDir->value("CURRENT_RAMAN_DIR").toString()));
        if(!fileName.isEmpty())
        {
            if (ActiveDir!=NULL) ActiveDir->setValue("CURRENT_RAMAN_DIR",GetCurrentDir(fileName));
            Map["Custom"] = fileName;
        }
        else
        {
            disconnect(this,SIGNAL(currentIndexChanged(int)),this,SLOT(solventIndexChanged(int)));
            setCurrentIndex(0);
            connect(this,SIGNAL(currentIndexChanged(int)),this,SLOT(solventIndexChanged(int)));
        }
    }
    emit sendSolvent(getCurrentSolvent());
}

