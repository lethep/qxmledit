/**************************************************************************
 *  This file is part of QXmlEdit                                         *
 *  Copyright (C) 2018 by Luca Bellonda and individual contributors       *
 *    as indicated in the AUTHORS file                                    *
 *  lbellonda _at_ gmail.com                                              *
 *                                                                        *
 * This library is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU Library General Public            *
 * License as published by the Free Software Foundation; either           *
 * version 2 of the License, or (at your option) any later version.       *
 *                                                                        *
 * This library is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 * Library General Public License for more details.                       *
 *                                                                        *
 * You should have received a copy of the GNU Library General Public      *
 * License along with this library; if not, write to the                  *
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,       *
 * Boston, MA  02110-1301  USA                                            *
 **************************************************************************/

#include "guidedvalidationdialog.h"
#include "ui_guidedvalidationdialog.h"
#include "applicationdata.h"
#include "modules/xsd/xsdvalidationexecutor.h"
#include "utils.h"

static const int TimeoutForPolling = 600 ;

GuidedValidationDialog::GuidedValidationDialog(QWidget *parent, ApplicationData *appData) :
    QDialog(parent),
    ui(new Ui::GuidedValidationDialog)
{
    _calculating = false ;
    _appData = appData ;
    ui->setupUi(this);
    ui->state->setCurrentIndex(PageEmpty);
    ui->errorCause->setText("");

    _xml.button = ui->cmdLoadXML;
    _xml.edit = ui->fileNameXML;
    _xml.icon = ui->statusXML;
    _xsd.button = ui->cmdLoadXSD;
    _xsd.edit = ui->fileNameXSD;
    _xsd.icon = ui->statusXSD;
    _isError = false ;
    _isValid = false ;
    setAcceptDrops(true);
    ui->fileNameXML->setAcceptDrops(false);
    ui->fileNameXSD->setAcceptDrops(false);
    int widthLButton = ui->cmdLoadXML->width();
    int widthSButton = ui->cmdLoadXSD->width();
    int newWidth = qMax(widthLButton, widthSButton) * 1.3;
    ui->cmdLoadXML->setFixedWidth(newWidth);
    ui->cmdLoadXSD->setFixedWidth(newWidth);
    _sizeIcon = ui->statusXML->pixmap()->width();
    ui->statusXML->setFixedWidth(_sizeIcon);
    ui->statusXSD->setFixedWidth(_sizeIcon);
    resetData();
}

GuidedValidationDialog::~GuidedValidationDialog()
{
    delete ui;
}

void GuidedValidationDialog::resetData()
{
    _message = "" ;
    _isError = false;
    _isValid = false;
    ui->fileNameXML->setText("");
    ui->fileNameXSD->setText("");
    updateStatusFile(&_xml, false, "");
    updateStatusFile(&_xsd, false, "");
    updateStatus();
}

QString GuidedValidationDialog::dataFile()
{
    return ui->fileNameXML->text();
}

QString GuidedValidationDialog::schemaFile()
{
    return ui->fileNameXSD->text();
}

void GuidedValidationDialog::updateStatusFile(InfoPartGuidedValidationDialog *info, const bool how, const QString &filePath)
{
    if(how) {
        info->edit->setText(filePath);
        info->icon->setPixmap(QPixmap(":/configuration/validation"));
    } else {
        info->icon->setPixmap(QPixmap());
    }
}

bool GuidedValidationDialog::isReady()
{
    if(ui->fileNameXML->text().isEmpty() || ui->fileNameXSD->text().isEmpty()) {
        return false;
    }
    return true ;
}

void GuidedValidationDialog::updateStatus()
{
    int page = PageEmpty ;
    if(isReady()) {
        if(_calculating) {
            page = PageWait;
            _message = "" ;
        } else {
            if(_isError) {
                page = PageEmpty;
            } else if(_isValid) {
                page = PageOk ;
                _message = "" ;
            } else {
                page = PageKo ;
            }
        }
    }
    ui->state->setCurrentIndex(page);
    ui->errorCause->setText(_message);
}

void GuidedValidationDialog::loadXML(const QString &fileName)
{
    updateStatusFile(&_xml, true, fileName);
    executeCalculation();
}

void GuidedValidationDialog::loadXSD(const QString &fileName)
{
    updateStatusFile(&_xsd, true, fileName);
    executeCalculation();
}

void GuidedValidationDialog::executeCalculation()
{
    if(ui->fileNameXML->text().isEmpty() || ui->fileNameXSD->text().isEmpty()) {
        return ;
    }
    if(_calculating) {
        stopCurrentCalculation();
    }
    executeNewCalculation();
    ui->state->setCurrentIndex(PageWait);
}

void GuidedValidationDialog::stopCurrentCalculation()
{
    if(_currentCalculation.isRunning()) {
        _currentCalculation.cancel();
        _waitingForCancelTaskList.append(_currentCalculation);
        _calculating = false;
    }
    if(_calculating) {
        Utils::error(this, tr("Invalid state"));
        _calculating = false;
    }
}

void GuidedValidationDialog::executeNewCalculation()
{
    setEnabled(false);
    _isError = false ;
    _isValid = false ;
    _message = "" ;
    _currentCalculation = QtConcurrent::run(this, &GuidedValidationDialog::executeCalculationInThread, dataFile(), schemaFile());
    QTimer::singleShot(TimeoutForPolling, this, SLOT(checkIfDone()));
    _calculating = true ;
}

void GuidedValidationDialog::checkIfDone()
{
    if(_calculating) {
        if(!_currentCalculation.isStarted()) {
            Utils::error(this, tr("Error starting operation."));
            stopCurrentCalculation();
            _isError = true ;
            _message = tr("Error executing the operation.");
            updateStatus();
            setEnabled(true);
            return ;
        }
        if(checkEndCalculation()) {
            _calculating = false ;
            updateStatus();
            setEnabled(true);
            return ;
        }
        QTimer::singleShot(300, this, SLOT(checkIfDone()));
    }
}

bool GuidedValidationDialog::checkEndCalculation()
{
    if(_currentCalculation.isFinished()) {
        _isError = false;
        const QPair<int, QString> &result = _currentCalculation.result();
        if(0 == result.first) {
            _isValid = true ;
        } else {
            _isValid = false ;
        }
        _message = result.second;
        return true ;
    }
    return false;
}

void GuidedValidationDialog::goodbyeCalculation()
{
    if(_calculating) {
        _calculating = false ;
        abortCalculation();
    }
    waitEndCalculation();
}

void GuidedValidationDialog::accept()
{
    goodbyeCalculation();
    QDialog::accept();
}

void GuidedValidationDialog::reject()
{
    goodbyeCalculation();
    QDialog::reject();
}

void GuidedValidationDialog::waitEndCalculation()
{
    QFuture<QPair<int, QString> > future;
    foreach(future, _waitingForCancelTaskList) {
        future.waitForFinished();
    }
}

void GuidedValidationDialog::abortCalculation()
{
    stopCurrentCalculation();
    if(_calculating) {
        Utils::error(this, tr("Invalid state"));
        _calculating = false;
    }
}

QString GuidedValidationDialog::chooseFile(const QString &prevChoice)
{
    QString filePath = QFileDialog::getOpenFileName(
                           this, tr("Open File"),
                           QXmlEditData::sysFilePathForOperation(prevChoice),
                           Utils::getFileFilterForOpenFile());
    return filePath ;
}


void GuidedValidationDialog::on_cmdLoadXML_clicked()
{
    QString file = chooseFile(ui->fileNameXML->text());
    if(!file.isEmpty()) {
        loadXML(file);
    }
}

void GuidedValidationDialog::on_cmdLoadXSD_clicked()
{
    QString file = chooseFile(ui->fileNameXSD->text());
    if(!file.isEmpty()) {
        loadXSD(file);
    }
}

void GuidedValidationDialog::showValidationDialog(ApplicationData *appData)
{
    GuidedValidationDialog dlg(NULL, appData);
    dlg.setModal(true);
    dlg.setWindowModality(Qt::ApplicationModal);
    dlg.exec();
}

QPair<int, QString> GuidedValidationDialog::executeCalculationInThread(const QString &dataFile, const QString &schemaFile)
{
    XSDValidationExecutor executor ;
    return executor.execute(dataFile, schemaFile);
}

void GuidedValidationDialog::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasFormat(Utils::URIDropType)) {
        event->acceptProposedAction();
    }
}

void GuidedValidationDialog::dropEvent(QDropEvent *event)
{
    if(event->mimeData()->hasFormat(Utils::URIDropType)) {
        QString filePath = "" ;
        event->acceptProposedAction();
        if(event->mimeData()->hasUrls()) {
            foreach(QUrl url, event->mimeData()->urls()) {
                filePath = url.toLocalFile();
                break;
            }
        }
        if(!filePath.isEmpty()) {
            const QPoint pos = event->pos();
            QWidget *childReferencedByDrop = childAt(pos.x(), pos.y());
            if(childReferencedByDrop  == ui->fileNameXML) {
                loadXML(filePath);
            } else if(childReferencedByDrop  == ui->fileNameXSD) {
                loadXSD(filePath);
            } else {
                if(filePath.toLower().endsWith(".xsd")) {
                    loadXSD(filePath);
                } else {
                    loadXML(filePath);
                }
            }
        }
    } else {
        event->ignore();
    }
}
