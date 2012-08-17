/********************************************************************************
** Form generated from reading UI file 'mpistat.ui'
**
** Created: Fri 17. Aug 12:43:03 2012
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
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
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
    QSpacerItem *verticalSpacer_3;
    QWidget *widget;
    QHBoxLayout *horizontalLayout_2;
    QGroupBox *groupBox;
    QSpinBox *spinBox;
    QLabel *label;
    QSpinBox *spinBox_2;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QSpacerItem *verticalSpacer;

    void setupUi(QMainWindow *mpistatClass)
    {
        if (mpistatClass->objectName().isEmpty())
            mpistatClass->setObjectName(QString::fromUtf8("mpistatClass"));
        mpistatClass->resize(957, 822);
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

        verticalLayout->addWidget(tabWidget);

        verticalSpacer_3 = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout->addItem(verticalSpacer_3);

        widget = new QWidget(centralWidget);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setEnabled(true);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(widget->sizePolicy().hasHeightForWidth());
        widget->setSizePolicy(sizePolicy);
        widget->setMinimumSize(QSize(0, 75));
        horizontalLayout_2 = new QHBoxLayout(widget);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        groupBox = new QGroupBox(widget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy1);
        spinBox = new QSpinBox(groupBox);
        spinBox->setObjectName(QString::fromUtf8("spinBox"));
        spinBox->setGeometry(QRect(20, 20, 42, 22));
        spinBox->setMaximum(4096);
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(70, 20, 16, 16));
        spinBox_2 = new QSpinBox(groupBox);
        spinBox_2->setObjectName(QString::fromUtf8("spinBox_2"));
        spinBox_2->setGeometry(QRect(90, 20, 42, 22));
        spinBox_2->setMaximum(4096);

        horizontalLayout_2->addWidget(groupBox);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        pushButton = new QPushButton(widget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setEnabled(true);
        pushButton->setMaximumSize(QSize(80, 16777215));
        pushButton->setCheckable(false);

        horizontalLayout->addWidget(pushButton);


        horizontalLayout_2->addLayout(horizontalLayout);

        pushButton_2 = new QPushButton(widget);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));
        QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(pushButton_2->sizePolicy().hasHeightForWidth());
        pushButton_2->setSizePolicy(sizePolicy2);
        pushButton_2->setMaximumSize(QSize(120, 16777215));

        horizontalLayout_2->addWidget(pushButton_2);


        verticalLayout->addWidget(widget);

        verticalSpacer = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout->addItem(verticalSpacer);


        verticalLayout_2->addLayout(verticalLayout);

        mpistatClass->setCentralWidget(centralWidget);

        retranslateUi(mpistatClass);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(mpistatClass);
    } // setupUi

    void retranslateUi(QMainWindow *mpistatClass)
    {
        mpistatClass->setWindowTitle(QApplication::translate("mpistatClass", "mpistat", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab5), QApplication::translate("mpistatClass", "Cache rate", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab1), QApplication::translate("mpistatClass", "Cache rate distribution", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab2), QApplication::translate("mpistatClass", "Transactions In/Out", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("mpistatClass", "Select numachip node number range for display:", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("mpistatClass", "to", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("mpistatClass", "Freeze", 0, QApplication::UnicodeUTF8));
        pushButton_2->setText(QApplication::translate("mpistatClass", "Deselect all graphs", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class mpistatClass: public Ui_mpistatClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MPISTAT_H
