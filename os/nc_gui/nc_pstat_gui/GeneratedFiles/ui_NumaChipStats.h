/********************************************************************************
** Form generated from reading UI file 'nc_stat.ui'
**
** Created: Thu 11. Oct 11:58:45 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NC_STAT_H
#define UI_NC_STAT_H

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

class Ui_NumaChipStatsClass
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
    QWidget *tab;
    QVBoxLayout *verticalLayout_5;
    QVBoxLayout *tablayout;
    QWidget *tab_2;
    QVBoxLayout *verticalLayout_15;
    QVBoxLayout *probeLayout;
    QWidget *tab_3;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout_3;
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

    void setupUi(QMainWindow *NumaChipStatsClass)
    {
        if (NumaChipStatsClass->objectName().isEmpty())
            NumaChipStatsClass->setObjectName(QString::fromUtf8("NumaChipStatsClass"));
        NumaChipStatsClass->resize(957, 822);
        NumaChipStatsClass->setMinimumSize(QSize(100, 160));
        centralWidget = new QWidget(NumaChipStatsClass);
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
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(tabWidget->sizePolicy().hasHeightForWidth());
        tabWidget->setSizePolicy(sizePolicy);
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
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        verticalLayout_5 = new QVBoxLayout(tab);
        verticalLayout_5->setSpacing(6);
        verticalLayout_5->setContentsMargins(11, 11, 11, 11);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        tablayout = new QVBoxLayout();
        tablayout->setSpacing(6);
        tablayout->setObjectName(QString::fromUtf8("tablayout"));

        verticalLayout_5->addLayout(tablayout);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        verticalLayout_15 = new QVBoxLayout(tab_2);
        verticalLayout_15->setSpacing(6);
        verticalLayout_15->setContentsMargins(11, 11, 11, 11);
        verticalLayout_15->setObjectName(QString::fromUtf8("verticalLayout_15"));
        probeLayout = new QVBoxLayout();
        probeLayout->setSpacing(6);
        probeLayout->setObjectName(QString::fromUtf8("probeLayout"));

        verticalLayout_15->addLayout(probeLayout);

        tabWidget->addTab(tab_2, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QString::fromUtf8("tab_3"));
        verticalLayoutWidget = new QWidget(tab_3);
        verticalLayoutWidget->setObjectName(QString::fromUtf8("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(10, 10, 911, 631));
        verticalLayout_3 = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        tabWidget->addTab(tab_3, QString());

        verticalLayout->addWidget(tabWidget);

        verticalSpacer_3 = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout->addItem(verticalSpacer_3);

        widget = new QWidget(centralWidget);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setEnabled(true);
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(widget->sizePolicy().hasHeightForWidth());
        widget->setSizePolicy(sizePolicy1);
        widget->setMinimumSize(QSize(0, 75));
        horizontalLayout_2 = new QHBoxLayout(widget);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        groupBox = new QGroupBox(widget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy2);
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
        QSizePolicy sizePolicy3(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(pushButton_2->sizePolicy().hasHeightForWidth());
        pushButton_2->setSizePolicy(sizePolicy3);
        pushButton_2->setMaximumSize(QSize(220, 16777215));

        horizontalLayout_2->addWidget(pushButton_2);


        verticalLayout->addWidget(widget);

        verticalSpacer = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout->addItem(verticalSpacer);


        verticalLayout_2->addLayout(verticalLayout);

        NumaChipStatsClass->setCentralWidget(centralWidget);

        retranslateUi(NumaChipStatsClass);

        tabWidget->setCurrentIndex(5);


        QMetaObject::connectSlotsByName(NumaChipStatsClass);
    } // setupUi

    void retranslateUi(QMainWindow *NumaChipStatsClass)
    {
        NumaChipStatsClass->setWindowTitle(QApplication::translate("NumaChipStatsClass", "NumaChipStats", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        tabWidget->setToolTip(QApplication::translate("NumaChipStatsClass", "<html><head/><body><p><span style=\" font-size:12pt; color:#000000;\">This GUI monitors performance characteristics for the all the NumaChips in the NumaConnect Single Image System. Hold the mouse over each tab for more information.</span></p></body></html>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        tabWidget->setTabText(tabWidget->indexOf(tab5), QApplication::translate("NumaChipStatsClass", "Cache rate (snapshot)", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabToolTip(tabWidget->indexOf(tab5), QApplication::translate("NumaChipStatsClass", "<html><head/><body><p><span style=\" font-family:'Verdana,sans-serif'; font-size:12pt; color:#000000;\">The tab shows a graph displaying cache hitrate (%) per second over time. The number of accesses to the cache per update is displayed in the legend text for each remote cache. The graph monitors the cache hit ratio for all remote caches (L4 NumaConnect type caches) in the NumaConnect Single Image System. </span></p></body></html>", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab1), QApplication::translate("NumaChipStatsClass", "Cache rate distribution", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabToolTip(tabWidget->indexOf(tab1), QApplication::translate("NumaChipStatsClass", "<html><head/><body><p><span style=\" font-size:12pt; color:#000000;\">This tab shows a histogram displaying a cache hitrate (%) snapshot (shown in blue color) on top of an average (shown in red) cache hitrate over time (red over blue gives purple). The number of accesses to the cache per update is displayed in the legend text for each remote cache. The histogram monitors the cache hit ratio for all remote caches (L4 NumaConnect type caches) in the NumaConnect Single Image System.</span></p></body></html>", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab2), QApplication::translate("NumaChipStatsClass", "Transactions distribution In/Out", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabToolTip(tabWidget->indexOf(tab2), QApplication::translate("NumaChipStatsClass", "<html><head/><body><p><span style=\" font-size:12pt;\">This tab shows a histogram displaying all Incoming </span><span style=\" font-size:12pt; color:#000000;\">(shown in blue color)</span><span style=\" font-size:12pt;\"> non-posted HT-Request and Outgoing </span><span style=\" font-size:12pt; color:#000000;\">(shown in</span><span style=\" font-size:12pt;\"> red color) non-posted HT-Request </span><span style=\" font-size:12pt; color:#000000;\">(red over blue gives purple)</span><span style=\" font-size:12pt;\"> to the NumaChip (Cave). The non-posted HT-Requests gives an impression of the amount of actual hyper transport traffic in and out of each NumaChip. The histogram shows the total accumulated number of non-posted HT-Requests for each NumaChip in the NumaConnect Single Image System. </span></p></body></html>", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("NumaChipStatsClass", "Transaction In/Out (snapshot)", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabToolTip(tabWidget->indexOf(tab), QApplication::translate("NumaChipStatsClass", "<html><head/><body><p><span style=\" font-family:'Verdana,sans-serif'; font-size:12pt;\">This tab shows a histogram displaying all Incoming non-posted HT-Requests and Outgoing non-posted HT-Request to the NumaChip (Cave). The non-posted HT-Requests give an impression of the amount of actual hyper transport traffic in and out of each NumaChip. The histogram shows a snapshot of the total accumulated number of non-posted HT-Requests per second for each NumaChip in the NumaConnect Single Image System.</span></p></body></html>", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("NumaChipStatsClass", "Probes distribution In/Out", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabToolTip(tabWidget->indexOf(tab_2), QApplication::translate("NumaChipStatsClass", "<html><head/><body><p><span style=\" font-size:12pt;\">This tab shows a histogram displaying all Incoming (shown in red color) HT-Probe and Outgoing (shown in blue color) HT-Probe </span><span style=\" font-size:12pt; color:#000000;\">(red over blue gives purple) </span><span style=\" font-size:12pt;\">to the NumaChip (Cave). The coherent hyper transport probes are necessary for maintaining the cache coherency in L1, L2, L3 and L4. The histogram shows the total accumulated number of HT-Probes for each NumaChip in the NumaConnect Single Image System.</span></p></body></html>", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab_3), QApplication::translate("NumaChipStatsClass", "CPU Counter Event", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("NumaChipStatsClass", "Select numachip node number range for display:", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("NumaChipStatsClass", "to", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("NumaChipStatsClass", "Freeze", 0, QApplication::UnicodeUTF8));
        pushButton_2->setText(QApplication::translate("NumaChipStatsClass", "Deselect all graphs", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class NumaChipStatsClass: public Ui_NumaChipStatsClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NC_STAT_H
