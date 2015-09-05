/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>

#include "xbeltree.h"

XbelTree::XbelTree(QWidget *parent)
    : QTreeWidget(parent)
{
    QStringList labels;
    labels << tr("Title") << tr("Location");

    header()->setSectionResizeMode(QHeaderView::Stretch);
    setHeaderLabels(labels);

    folderIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirClosedIcon),
                         QIcon::Normal, QIcon::Off);
    folderIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirOpenIcon),
                         QIcon::Normal, QIcon::On);
    stringIcon.addPixmap(style()->standardPixmap(QStyle::SP_FileIcon));
}

bool XbelTree::read(QIODevice *device)
{
    QString errorStr;
    int errorLine;
    int errorColumn;

    if (!domDocument.setContent(device, true, &errorStr, &errorLine,
                                &errorColumn)) {
        QMessageBox::information(window(), tr("DOM string"),
                                 tr("Parse error at line %1, column %2:\n%3")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
        return false;
    }

    QDomElement root = domDocument.documentElement();
    //    if (root.tagName() != "xbel") {
    //        QMessageBox::information(window(), tr("DOM string"),
    //                                 tr("The file is not an XBEL file."));
    //        return false;
    //    } else if (root.hasAttribute("version")
    //               && root.attribute("version") != "1.0") {
    //        QMessageBox::information(window(), tr("DOM string"),
    //                                 tr("The file is not an XBEL version 1.0 "
    //                                    "file."));
    //        return false;
    //    }

    clear();

    disconnect(this, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
               this, SLOT(updateDomElement(QTreeWidgetItem*,int)));

    //Now let's read this!
    QDomElement child = root.firstChildElement("folder");
    //Add the folders to tree simulation
    while (!child.isNull()) {
        parseFolderElement(child);
        child = child.nextSiblingElement("folder");
    }

    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(updateDomElement(QTreeWidgetItem*,int)));

    return true;
}

bool XbelTree::write(QIODevice *device)
{
    const int IndentSize = 40;

    QTextStream out(device);
    domDocument.save(out, IndentSize);
    return true;
}

void XbelTree::updateDomElement(QTreeWidgetItem *item, int column)
{
    QDomElement element = domElementForItem.value(item);
    if (!element.isNull()) {
        //if the name changed
        if (column == 0) {
            //            QDomElement oldTitleElement = element.firstChildElement("title");
            //            QDomElement newTitleElement = domDocument.createElement("title");

            //            QDomText newTitleText = domDocument.createTextNode(item->text(0));
            //            newTitleElement.appendChild(newTitleText);

            //            element.replaceChild(newTitleElement, oldTitleElement);
            element.setAttribute("name",item->text(0));
        }
        //if the string changed
        else {
            if (element.tagName() == "string")
            {
                //element.setAttribute("href", item->text(1));
                // QDomText newStringText = domDocument.createTextNode(item->text(1));
                QDomNode parentElement = element.parentNode().toElement();
                QDomElement newStringElement = domDocument.createElement("string");
                QDomText newStringText = domDocument.createTextNode(item->text(1));
                newStringElement.setAttribute("name",item->text(0));
                newStringElement.appendChild(newStringText);
//                element.clear();
                parentElement.replaceChild(newStringElement, element);
            }
        }
    }
}



//This is a recursive function that go down to the folders with dfs method
void XbelTree::parseFolderElement(const QDomElement &element,
                                  QTreeWidgetItem *parentItem)
{
    QTreeWidgetItem *item = createItem(element, parentItem);
    //set folder name
    QString title = element.attribute("name");//element.firstChildElement("title").text();
    if (title.isEmpty())
        title = QObject::tr("Folder");

    item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setIcon(0, folderIcon);
    item->setText(0, title);

    //    bool folded = (element.attribute("folded") != "no");
    //    setItemExpanded(item, !folded);

    QDomElement child = element.firstChildElement();
    while (!child.isNull()) {
        //If child is even foler create that and call parseFolderElecmt func for that
        if (child.tagName() == "folder") {
            parseFolderElement(child, item);
        }
        //if we have string here!
        else if (child.tagName() == "string") {
            QTreeWidgetItem *childItem = createItem(child, item);

            //add string name
            QString title =child.attribute("name");//firstChildElement("title").text();
            if (title.isEmpty())
                title = QObject::tr("Folder");

            //add string text and some beautiful icon!
            childItem->setFlags(item->flags() | Qt::ItemIsEditable);
            childItem->setIcon(0, stringIcon);
            childItem->setText(0, title);
            childItem->setText(1, child.text());
        } /*else if (child.tagName() == "separator") {
            QTreeWidgetItem *childItem = createItem(child, item);
            childItem->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEditable));
            childItem->setText(0, QString(30, 0xB7));
        }*/
        child = child.nextSiblingElement();
    }
}


//This func use when see new folder
//So we use this in the beging of the parseFolderElecmt func
QTreeWidgetItem *XbelTree::createItem(const QDomElement &element,
                                      QTreeWidgetItem *parentItem)
{
    QTreeWidgetItem *item;
    if (parentItem) {
        item = new QTreeWidgetItem(parentItem);
    } else {
        item = new QTreeWidgetItem(this);
    }
    domElementForItem.insert(item, element);
    return item;
}
