/********************************************************************************
** Form generated from reading UI file 'mpistat.ui'
**
** Created: Mon 23. Jul 11:16:46 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MPISTAT_H
#define UI_MPISTAT_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTabWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_mpistatClass
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QSpacerItem *verticalSpacer_2;
    QTabWidget *tabWidget;
    QWidget *tab5;
    QVBoxLayout *verticalLayout_7;
    QVBoxLayout *tab5layout;
    QWidget *tab1;
    QVBoxLayout *verticalLayout_4;
    QVBoxLayout *tab1layout;
    QWidget *tab2;
    QVBoxLayout *verticalLayout_6;
    QVBoxLayout *tab2layout;
    QWidget *tab3;
    QVBoxLayout *verticalLayout_5;
    QVBoxLayout *tab3layout;
    QWidget *tab4;
    QVBoxLayout *verticalLayout_8;
    QVBoxLayout *tab4layout;
    QSpacerItem *verticalSpacer_3;
    QWidget *widget;
    QHBoxLayout *horizontalLayout_2;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButton;
    QSpacerItem *verticalSpacer;

    void setupUi(QMainWindow *mpistatClass)
    {
        if (mpistatClass->objectName().isEmpty())
            mpistatClass->setObjectName(QString::fromUtf8("mpistatClass"));
        mpistatClass->resize(919, 595);
        mpistatClass->setMinimumSize(QSize(100, 160));
        centralWidget = new QWidget(mpistatClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        verticalLayout_2 = new QVBoxLayout(centralWidget);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalSpacer_2 = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout->addItem(verticalSpacer_2);

        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setMovable(true);
        tab5 = new QWidget();
        tab5->setObjectName(QString::fromUtf8("tab5"));
        verticalLayout_7 = new QVBoxLayout(tab5);
        verticalLayout_7->setSpacing(6);
        verticalLayout_7->setContentsMargins(11, 11, 11, 11);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        tab5layout = new QVBoxLayout();
        tab5layout->setSpacing(6);
        tab5layout->setObjectName(QString::fromUtf8("tab5layout"));

        verticalLayout_7->addLayout(tab5layout);

        tabWidget->addTab(tab5, QString());
        tab1 = new QWidget();
        tab1->setObjectName(QString::fromUtf8("tab1"));
        verticalLayout_4 = new QVBoxLayout(tab1);
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setContentsMargins(11, 11, 11, 11);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        tab1layout = new QVBoxLayout();
        tab1layout->setSpacing(6);
        tab1layout->setObjectName(QString::fromUtf8("tab1layout"));

        verticalLayout_4->addLayout(tab1layout);

        tabWidget->addTab(tab1, QString());
        tab2 = new QWidget();
        tab2->setObjectName(QString::fromUtf8("tab2"));
        verticalLayout_6 = new QVBoxLayout(tab2);
        verticalLayout_6->setSpacing(6);
        verticalLayout_6->setContentsMargins(11, 11, 11, 11);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        tab2layout = new QVBoxLayout();
        tab2layout->setSpacing(6);
        tab2layout->setObjectName(QString::fromUtf8("tab2layout"));

        verticalLayout_6->addLayout(tab2layout);

        tabWidget->addTab(tab2, QString());
        tab3 = new QWidget();
        tab3->setObjectName(QString::fromUtf8("tab3"));
        verticalLayout_5 = new QVBoxLayout(tab3);
        verticalLayout_5->setSpacing(6);
        verticalLayout_5->setContentsMargins(11, 11, 11, 11);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        tab3layout = new QVBoxLayout();
        tab3layout->setSpacing(6);
        tab3layout->setObjectName(QString::fromUtf8("tab3layout"));

        verticalLayout_5->addLayout(tab3layout);

        tabWidget->addTab(tab3, QString());
        tab4 = new QWidget();
        tab4->setObjectName(QString::fromUtf8("tab4"));
        verticalLayout_8 = new QVBoxLayout(tab4);
        verticalLayout_8->setSpacing(6);
        verticalLayout_8->setContentsMargins(11, 11, 11, 11);
        verticalLayout_8->setObjectName(QString::fromUtf8("verticalLayout_8"));
        tab4layout = new QVBoxLayout();
        tab4layout->setSpacing(6);
        tab4layout->setObjectName(QString::fromUtf8("tab4layout"));

        verticalLayout_8->addLayout(tab4layout);

        tabWidget->addTab(tab4, QString());

        verticalLayout->addWidget(tabWidget);

        verticalSpacer_3 = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout->addItem(verticalSpacer_3);

        widget = new QWidget(centralWidget);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setMinimumSize(QSize(0, 0));
        horizontalLayout_2 = new QHBoxLayout(widget);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        pushButton = new QPushButton(widget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setMaximumSize(QSize(80, 16777215));

        horizontalLayout->addWidget(pushButton);


        horizontalLayout_2->addLayout(horizontalLayout);


        verticalLayout->addWidget(widget);

        verticalSpacer = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout->addItem(verticalSpacer);


        verticalLayout_2->addLayout(verticalLayout);

        mpistatClass->setCentralWidget(centralWidget);

        retranslateUi(mpistatClass);

        tabWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(mpistatClass);
    } // setupUi

    void retranslateUi(QMainWindow *mpistatClass)
    {
        mpistatClass->setWindowTitle(QApplication::translate("mpistatClass", "mpistat", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab5), QApplication::translate("mpistatClass", "Cache rate", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab1), QApplication::translate("mpistatClass", "Cache rate distribution", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab2), QApplication::translate("mpistatClass", "Send latency", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab3), QApplication::translate("mpistatClass", "Receive latency", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab4), QApplication::translate("mpistatClass", "Bandwidth", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("mpistatClass", "PDF", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class mpistatClass: public Ui_mpistatClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MPISTAT_H
