/**************************************************************************
 *  This file is part of QXmlEdit                                         *
 *  Copyright (C) 2016-2018 by Luca Bellonda and individual contributors  *
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


#ifndef NORMALIZENAMESPACEDIALOG_H
#define NORMALIZENAMESPACEDIALOG_H

#include "xmlEdit.h"
#include "regola.h"
#include "widgets/elementselectiontypewidget.h"

class NormalizeNamespaceInfo
{
public:
    QString nsURI;
    QString setPrefix;
    Element *element;
    TargetSelection::Type targetSelection;
    NamespacesInfo namespacesInfo;
    bool remapPrefixes;
    bool declareOnRoot;
    NormalizeNamespaceInfo()
    {
        element = NULL ;
        targetSelection = TargetSelection::AllItems;
        remapPrefixes = false ;
        declareOnRoot = true ;
    }
};

namespace Ui
{
class NormalizeNamespaceDialog;
}

class NormalizeNamespaceDialog : public QDialog
{
    Q_OBJECT
    NormalizeNamespaceInfo *_info;
    ElementSelectionTypeWidget *_selWidget;
    QSet<QString> _namespacesCited;
    QSet<QString> _prefixesCited;
    NamespaceManager *_namespaceManager;

    void finishSetup();
    void updateSelection();
    void enableChoices();
    void enableOk();
    void updatePrefixes();
    void mergePrefixesInfo();
    void mergeNamespacesInfo();
    bool checkOK();
protected:
    void accept();

public:
    static bool getNormalizeNamespaceInfo(QWidget *window, NormalizeNamespaceInfo *info, NamespaceManager *namespaceManager);

    explicit NormalizeNamespaceDialog(NormalizeNamespaceInfo *info, QWidget *parent = 0);
    virtual ~NormalizeNamespaceDialog();

    NamespaceManager *namespaceManager() const;
    void setNamespaceManager(NamespaceManager *namespaceManager);
private:
    Ui::NormalizeNamespaceDialog *ui;
private slots:
    void on_select_clicked();
    void onSelectionTypeChanged(TargetSelection::Type selectionType);
    void on_prefix_currentTextChanged(const QString &);
    void on_prefix_editTextChanged(const QString &);
    void on_namespaces_currentTextChanged(const QString &);
    void on_namespaces_editTextChanged(const QString &);

};

#endif // NORMALIZENAMESPACEDIALOG_H
