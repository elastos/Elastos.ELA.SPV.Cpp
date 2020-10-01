/* Summer-2020 Open Source Activity #Elastos Wallet (SPV)
 * FILE: mainwindow.cpp
 *
 * DESC: This is the cpp file of the main window.
 */
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "doublepasswordinputwindow.h"
#include "getaddressamountwindow.h"

// static members
CreateWalletWindow1 *MainWindow::cwWindow1;
CreateWalletWindow2 *MainWindow::cwWindow2;
CreateWalletWindow3 *MainWindow::cwWindow3;
CreateWalletWindow4 *MainWindow::cwWindow4;
PaymentAddressWindow *MainWindow::paWindow;
QRcodeWindow *MainWindow::qrcWindow;
SwitchWalletWindow *MainWindow::swWindow;
RemoveWalletWindow *MainWindow::rwWindow;
ChangePasswordWindow *MainWindow::cpWindow;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    isAppStart = true;

    ui->setupUi(this);
    ///////////////////////////
    // set widget attributes //
    ///////////////////////////
    // set main window
    ui->actionHome->setChecked(true);
    ui->pages->setCurrentIndex(0);
    ui->twRecentRecord->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->twRecentRecord->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->twRecentRecord->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->twRecentRecord->setStyleSheet("selection-background-color:lightgray");
    ui->twRecentRecord->setItemDelegate(new KeepForegroundItemDelegate());
    ui->twRecord->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->twRecord->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->twRecord->setStyleSheet("selection-background-color:lightgray");
    ui->twRecord->setItemDelegate(new KeepForegroundItemDelegate());
    this->setWindowTitle(tr("Elastos SPV Wallet"));
    // set status bar
    /* add sync (start / stop) tool button */
    btnChangeSyncStatus = new QToolButton();
    // bind sigal & slot
    btnChangeSyncStatus->setText("Sync");
    btnChangeSyncStatus->setCheckable(true);
    btnChangeSyncStatus->setChecked(true);
    connect(btnChangeSyncStatus, SIGNAL(toggled(bool)), this, SLOT(syncStatusChanged(bool)));
    ui->statusBar->addWidget(btnChangeSyncStatus);
    /* add sub-wallet (chain-id) label */
    lblSubWalletChainID = new QLabel();
    ui->statusBar->addWidget(lblSubWalletChainID);
    /* add syn status label */
    lblSyncStatus = new QLabel();
    ui->statusBar->addWidget(lblSyncStatus);
    /* add sync progress-bar */
    pbSyncProgress = new QProgressBar();
    ui->statusBar->addWidget(pbSyncProgress);
    /* add sync progress label */
    lblSyncProgress = new QLabel();
    ui->statusBar->addWidget(lblSyncProgress);
    /* add sync last-block-time label */
    lblLastBlockTime = new QLabel();
    ui->statusBar->addWidget(lblLastBlockTime);
    /* add sync error hint label */
    lblSyncErrorHint = new QLabel();
    ui->statusBar->addWidget(lblSyncErrorHint);

    //////////////////////////////////
    // set registry for Preferences //
    //////////////////////////////////
    // set WalletRoot
    ui->lblWalletRoot->setText(curWalletRoot);
    // set Language
    if (curLang == "EN")
        ui->cbLang->setCurrentText("English");
    else if (curLang == "CN")
        ui->cbLang->setCurrentText("简体中文");
    else
        ui->cbLang->setCurrentText("English");
    // set Network
    ui->cbNetwork->setCurrentText(curNetwork);
    QString networkStatus = (isNetworkUpdate ? tr("updated") : tr("update failed"));
    QString networkStatusStyle = (isNetworkUpdate ? "color:green;" : "color:red;");
    ui->lblNetworkStatus->setText(networkStatus);
    ui->lblNetworkStatus->setStyleSheet(networkStatusStyle);
    ui->lblNetworkLastUpdateTime->setText(networkLastUpdateTime);
    ui->lblNetworkConfigFileDir->setText(networkConfigFileDir);
    // set SubWallet / ChainID
    /* The combobox of SubWallet is initialized in 'updateWalletInformation'
     * As the supported SubWallet / ChainID information is not available right now */

    // update wallet information
    updateWalletInformation();

    // init application timer for update
    timerRecentTransactionRecordsAutoUpdate = new QTimer(this);
    timerStatusBarAutoUpdate = new QTimer(this);
    timerTransactionRecordsAutoUpdate = new QTimer(this);
    connect(timerTransactionRecordsAutoUpdate, SIGNAL(timeout()), this, SLOT(updateRecordTableInformation()));
    connect(timerStatusBarAutoUpdate, SIGNAL(timeout()), this, SLOT(updateStatusBarInformation()));
    connect(timerRecentTransactionRecordsAutoUpdate, SIGNAL(timeout()), this, SLOT(updateRecentRecordTableInformation()));
    if (isWalletAutoUpdate)
    {
        timerRecentTransactionRecordsAutoUpdate->start(dataAutoUpdateInterval*1000);
        timerTransactionRecordsAutoUpdate->start(dataAutoUpdateInterval*1000);
    }
    // default force auto-update
    /* (can be controled by 'sync' tool-button) */
    timerStatusBarAutoUpdate->start(dataAutoUpdateInterval*1000);

    // set AppVersionMessage string
    ui->lblAppVersionMessage->setText(manager->GetVersion().c_str());
    // set TransactionAutoUpdate check box
    ui->cbTransactionRecordAutoUpdate->setChecked(isWalletAutoUpdate);
    // set TransactionAutoUpdate check box' text
    QString intervalText = QString::asprintf(tr("Auto Update (Every %ds)").toStdString().c_str(), dataAutoUpdateInterval);
    ui->cbTransactionRecordAutoUpdate->setText(intervalText);
    // set Auto-Update interval
    ui->sbDataUpdateInterval->setValue(dataAutoUpdateInterval);

    isAppStart = false;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionCreate_Wallet_triggered()
{
    MainWindow::cwWindow1 = new CreateWalletWindow1();
    MainWindow::cwWindow2 = new CreateWalletWindow2();
    MainWindow::cwWindow3 = new CreateWalletWindow3();
    MainWindow::cwWindow4 = new CreateWalletWindow4();
    MainWindow::cwWindow1->show();
    // bind signal & slot
    connect(MainWindow::cwWindow4, SIGNAL(createWalletFinish()), this, SLOT(updateWalletInformation()));
}

void MainWindow::on_actionPayment_Address_triggered()
{
    if (MainWindow::paWindow == nullptr)
        MainWindow::paWindow = new PaymentAddressWindow();
    MainWindow::paWindow->show();
}

void MainWindow::on_actionHome_triggered()
{
    ui->pages->setCurrentIndex(0);
    ui->actionHome->setChecked(true);
    ui->actionTransfer->setChecked(false);
    ui->actionReceiving->setChecked(false);
    ui->actionTransaction_Record->setChecked(false);
    ui->actionPreferences->setChecked(false);
}

void MainWindow::on_actionTransfer_triggered()
{
    ui->pages->setCurrentIndex(1);
    ui->actionHome->setChecked(false);
    ui->actionTransfer->setChecked(true);
    ui->actionReceiving->setChecked(false);
    ui->actionTransaction_Record->setChecked(false);
    ui->actionPreferences->setChecked(false);
}

void MainWindow::on_actionReceiving_triggered()
{
    ui->pages->setCurrentIndex(2);
    ui->actionHome->setChecked(false);
    ui->actionTransfer->setChecked(false);
    ui->actionReceiving->setChecked(true);
    ui->actionTransaction_Record->setChecked(false);
    ui->actionPreferences->setChecked(false);
}

void MainWindow::on_actionTransaction_Record_triggered()
{
    ui->pages->setCurrentIndex(3);
    ui->actionHome->setChecked(false);
    ui->actionTransfer->setChecked(false);
    ui->actionReceiving->setChecked(false);
    ui->actionTransaction_Record->setChecked(true);
    ui->actionPreferences->setChecked(false);
}

void MainWindow::on_actionPreferences_triggered()
{
    ui->pages->setCurrentIndex(4);
    ui->actionHome->setChecked(false);
    ui->actionTransfer->setChecked(false);
    ui->actionReceiving->setChecked(false);
    ui->actionTransaction_Record->setChecked(false);
    ui->actionPreferences->setChecked(true);
}

void MainWindow::on_btnShowRW_clicked()
{
    std::string address;
    // create addres
    curSubWalletChainID = ui->cbSubWalletChainID->currentText();
    std::pair<int,std::string> result = createReceiveAddress(curSubWalletChainID.toStdString(), address);

    // check address
    if (result.first == ERRNO_APP)
    {
        QString errorMessage = QString::asprintf(
                    tr("Create receive address failed.\n\nError Message:\n%s").toStdString().c_str(),
                    result.second.c_str());
        QMessageBox::warning(nullptr, tr("Error"), errorMessage);
        return;
    }

    // show window
    if (MainWindow::qrcWindow == nullptr)
        MainWindow::qrcWindow = new QRcodeWindow(address.c_str());
    MainWindow::qrcWindow->show();
}

void MainWindow::on_btnOpenPA_clicked()
{
    if (MainWindow::paWindow == nullptr)
        MainWindow::paWindow = new PaymentAddressWindow();
    MainWindow::paWindow->show();
}

void MainWindow::on_btnClear_clicked()
{
    ui->leAddress->setText("");
    ui->dsbAmountForTransfer->setValue(0);
}

void MainWindow::on_btnExport_clicked()
{
    QString dlgTitle = tr("Export To");
    QString filter = tr("CSV File(*.csv);;All File(*.*)");
    QString filename = QFileDialog::getSaveFileName(this, dlgTitle, "/", filter);
    if (filename.isEmpty())
        return;
    if (!exportRecordTableToCsvFile(filename))
        QMessageBox::warning(nullptr, tr("Warning"), tr("Unable to export.\nPlease check relevant permissions."));
}

bool MainWindow::exportRecordTableToCsvFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QString str = tr("\"Transaction Record Data\",\"Time\",\"Confirmed Blocks\",\"Status\",\"Type\",\"Amount/ELA\",\"Transaction ID\"\n");
    QTableWidgetItem *item;
    for (int i = 0; i < ui->twRecord->rowCount(); ++i)
    {
        str += QString::asprintf("\"%d\",", i+1);
        for (int j = 0; j < ui->twRecord->columnCount()-1; ++j)
        {
            item = ui->twRecord->item(i,j);
            str += QString::asprintf("\"%s\",", item->text().toStdString().c_str());
        }
        item = ui->twRecord->item(i,ui->twRecord->columnCount()-1);
        str += QString::asprintf("\"%s\"\n", item->text().toStdString().c_str());
    }
    QByteArray strBytes = str.toUtf8();
    file.write(strBytes, strBytes.length());
    file.close();
    return true;
}

void MainWindow::on_leSearch_textChanged(const QString &arg1)
{
    QTableWidgetItem *item;
    // set search range
    /* Transaction ID */
    int searchRange[] = { ui->twRecord->columnCount()-1 };
    // start search
    if (!arg1.isEmpty())
    {
        for (int i = 0; i < ui->twRecord->rowCount(); ++i)
        {
            for (int j : searchRange)
            {
                item = ui->twRecord->item(i,j);
                if (item->text().contains(arg1))
                {
                    item->setBackground(QColor(220,180,0,100));
                }
                else
                {
                    item->setBackground(QColor(0,0,0,0));
                }
            }
        }
    }
    else
    {
        for (int i = 0; i < ui->twRecord->rowCount(); ++i)
        {
            for (int j : searchRange)
            {
                item = ui->twRecord->item(i,j);
                item->setBackground(QColor(0,0,0,0));
            }
        }
    }
}

void MainWindow::on_cbLang_currentIndexChanged(const QString &arg1)
{
    if (!isAppStart)
    {
        if (arg1 == "English")
        {
            curLang = "EN";
            qApp->removeTranslator(trans);
            delete trans;
            trans = new QTranslator();
            trans->load(":/ElastosWalletSPV_us_EN.qm");
            qApp->installTranslator(trans);
            ui->retranslateUi(this);
            QSettings settings(ORGNAZATION, APP_NAME);
            settings.setValue(LANGUAGE_PREF, "EN");
            QImage image;
            image.load(":/Images/Covers/ReceivingCover_EN_S.png");
            ui->lblCoverForReceiving->setPixmap(QPixmap::fromImage(image));
        }
        else if (arg1 == "简体中文")
        {
            curLang = "CN";
            qApp->removeTranslator(trans);
            delete trans;
            trans = new QTranslator();
            trans->load(":ElastosWalletSPV_zh_CN.qm");
            qApp->installTranslator(trans);
            ui->retranslateUi(this);
            QSettings settings(ORGNAZATION, APP_NAME);
            settings.setValue(LANGUAGE_PREF, "CN");
            QImage image;
            image.load(":/Images/Covers/ReceivingCover_CN_S.png");
            ui->lblCoverForReceiving->setPixmap(QPixmap::fromImage(image));
        }
        else
            return;

        // update wallet information
        updateQuickLookInformation();
        updateNetworkInformation();
        // set WalletRoot
        ui->lblWalletRoot->setText(curWalletRoot);
    }
}

// check current wallet status
bool MainWindow::checkCurrentWalletStatus()
{
    // check currentWallet
    if (currentWallet == nullptr)
    {
        // no currentWallet
        ui->actionSwitch_Wallet_Window->setEnabled(false);
        ui->actionRemove_Wallet->setEnabled(false);
        ui->actionExport_Mnemonic_Words->setEnabled(false);
        ui->actionExport_Keystore->setEnabled(false);
        ui->actionChange_Wallet_Password->setEnabled(false);
        ui->actionTransfer->setEnabled(false);
        ui->actionReceiving->setEnabled(false);
        ui->actionTransaction_Record->setEnabled(false);
        return false;
    }
    else
    {
        // has currentWallet
        ui->actionSwitch_Wallet_Window->setEnabled(true);
        ui->actionRemove_Wallet->setEnabled(true);
        ui->actionExport_Mnemonic_Words->setEnabled(true);
        ui->actionExport_Keystore->setEnabled(true);
        ui->actionChange_Wallet_Password->setEnabled(true);
        ui->actionTransfer->setEnabled(true);
        ui->actionReceiving->setEnabled(true);
        ui->actionTransaction_Record->setEnabled(true);
        return true;
    }
}

/* SLOT : update wallet information on GUI
 * main-udpate func:
 *      switch-wallet-menu-update
 *      switch-subwallet-menu-update
 *      quick-look-update
 *      tx-operation-button-update
 *      recent-record-table-update
 *      status-bar-update
 *      transfer-billing-details-update
 *      record-table-update
 *      network-information-update
 */
/* main-update */
void MainWindow::updateWalletInformation()
{
    walletInformationUpdating = true;
    // check current wallet
    bool hasWallet = checkCurrentWalletStatus();
    Q_UNUSED(hasWallet)
    updateSwitchWalletMenuInformation();
    updateSwitchSubWalletMenuInformation();
    updateQuickLookInformation();
    updateOperationButtonInformation();
    updateRecentRecordTableInformation();
    updateStatusBarInformation();
    updateTransferBillingDetails();
    updateRecordTableInformation();

    // emit signal
    emit updateWalletSignal();
    walletInformationUpdating = false;
}
/* switch-wallet-menu-update */
void MainWindow::updateSwitchWalletMenuInformation()
{
    if (currentWallet != nullptr)
    {
        // update switch wallet menu information
        ui->menuSwitch_Wallet->clear();
        ui->menuSwitch_Wallet->addAction(ui->actionSwitch_Wallet_Window);
        ui->menuSwitch_Wallet->addSeparator();
        auto masterWalletIDs = manager->GetAllMasterWalletID();
        for (auto id : masterWalletIDs)
        {
            QAction *idAction = new QAction(id.c_str());
            idAction->setCheckable(true);
            // check current wallet
            if (currentWallet->GetID() == id)
                idAction->setChecked(true);
            else
                idAction->setChecked(false);
            // bind signal & slot
            connect(idAction, SIGNAL(triggered()), this, SLOT(switchCurrentWallet()));
            // add wallet item
            ui->menuSwitch_Wallet->addAction(idAction);
        }
    }
    else
    {
        // update switch wallet menu information
        ui->menuSwitch_Wallet->clear();
        QAction *action = new QAction(tr("No wallet"));
        action->setEnabled(false);
        ui->menuSwitch_Wallet->addAction(action);
    }
}
/* switch-subwallet-menu-update */
void MainWindow::updateSwitchSubWalletMenuInformation()
{
    if (currentWallet != nullptr)
    {
        // update switch sub-wallet (chain-id) menu information
        ui->menuManage_SubWallet_ChainID->clear();
        std::vector<std::string> supportChainID = currentWallet->GetSupportedChains();
        std::vector<ISubWallet *> subWallets = currentWallet->GetAllSubWallets();
        std::vector<std::string> openedChainID;
        std::transform(subWallets.begin(), subWallets.end(), std::back_inserter(openedChainID),
                       [] (ISubWallet *subWallet) { return subWallet->GetChainID(); });
        for (auto chainID : supportChainID)
        {
            QAction *chainIDAction = new QAction(chainID.c_str());
            // bind signal & slot
            chainIDAction->setCheckable(true);
            // check sub-wallet is opened
            auto findChainID = std::find(openedChainID.begin(), openedChainID.end(), chainID);
            if (findChainID != openedChainID.end())
                chainIDAction->setChecked(true);
            else
                chainIDAction->setChecked(false);
            connect(chainIDAction, SIGNAL(triggered(bool)), this, SLOT(subWalletStatusChanged(bool)));
            // add sub-wallet item
            ui->menuManage_SubWallet_ChainID->addAction(chainIDAction);
        }
    }
    else
    {
        // update switch sub-wallet (chain-id) information
        ui->menuManage_SubWallet_ChainID->clear();
        QAction *action = new QAction(tr("No master wallet"));
        action->setEnabled(false);
        ui->menuManage_SubWallet_ChainID->addAction(action);
    }
}
/* quick-look-update */
void MainWindow::updateQuickLookInformation()
{
    // update app version
    if (manager != nullptr)
        ui->lblAppVersionMessage->setText(manager->GetVersion().c_str());

    if (currentWallet != nullptr)
    {
        // update Quick Look information
        QString balance;
        auto subWallets = currentWallet->GetAllSubWallets();

        // master-wallet
        ui->lblWalletName->setText(currentWallet->GetID().c_str());

        // sub-wallet & balance
        if (!subWallets.empty())
        {
            ui->cbSubWalletChainID->clear();
            for (const ISubWallet *subWallet : subWallets)
            {
                std::string chainID = subWallet->GetChainID();
                ui->cbSubWalletChainID->addItem(chainID.c_str());
                if (subWallet && (chainID == curSubWalletChainID.toStdString()))
                {
                    // get balance
                    if (chainID == CHAINID_ETHSC)
                        balance = QString::asprintf("%s", subWallet->GetBalance().c_str());
                    else
                        balance = QString::asprintf("%.8lf", std::stod(subWallet->GetBalance()) / SELA_PER_ELA);
                }
            }
            // cusSubWalletChainID is not in sub-wallet list
            /* try get sub-wallet & balance again */
            if (balance.isEmpty())
            {
                curSubWalletChainID = ui->cbSubWalletChainID->currentText();
                ISubWallet *subWallet = currentWallet->GetSubWallet(curSubWalletChainID.toStdString());
                if (subWallet == nullptr)
                    return;
                if (curSubWalletChainID.toStdString() == CHAINID_ETHSC)
                    balance = QString::asprintf("%s", subWallet->GetBalance().c_str());
                else
                    balance = QString::asprintf("%.8lf", std::stod(subWallet->GetBalance()) / SELA_PER_ELA);
            }
            ui->cbSubWalletChainID->setCurrentText(curSubWalletChainID);
            ui->lblBalances->setText(balance);
        }
        else
        {
            ui->cbSubWalletChainID->clear();
            ui->lblBalances->setText("");
        }
    }
    else
    {
        // update Quick Look information
        ui->lblWalletName->setText("");
        ui->cbSubWalletChainID->clear();
        ui->lblBalances->setText("");
    }
}
/* tx-operation-button-update */
void MainWindow::updateOperationButtonInformation()
{
    bool hasWallet = (currentWallet != nullptr);
    // check master-wallet & sub-wallet status
    curSubWalletChainID = ui->cbSubWalletChainID->currentText();
    bool deactiveELA = hasWallet && (curSubWalletChainID != CHAINID_ELA.c_str());
    bool activeOperation = deactiveELA && !curSubWalletChainID.isEmpty();
    ui->btnDeposit->setEnabled(activeOperation);
    ui->btnWithdraw->setEnabled(activeOperation);
}
/* recent-record-table-update */
void MainWindow::updateRecentRecordTableInformation()
{
    if (currentWallet != nullptr)
    {
        // get records data to be showed
        QStringList timeList, typeList, amountList;
        std::pair<int,std::string> result = getRecentTransactionRecords(timeList, typeList, amountList);

        // check result
        if (result.first == ERRNO_APP)
        {
            ui->twRecentRecord->clearContents();
            return;
        }

        // update record-table gui
        std::vector<QStringList> rowData = { timeList, typeList, amountList };
        // clear table content (keep original row count)
        ui->twRecentRecord->clearContents();
        // reset table content
        try
        {
            for (int i = 0; i < timeList.count(); ++i)
            {
                for (size_t j = 0; j < rowData.size(); ++j)
                {
                    QString data = rowData[j].at(i);
                    QTableWidgetItem *item = new QTableWidgetItem();
                    // set show-attribute
                    item->setTextAlignment(Qt::AlignCenter);
                    if (data == "Sent")
                        item->setForeground(QColor(180, 0, 0, 255));
                    else if (data == "Received")
                        item->setForeground(QColor(0, 180, 0, 255));
                    // set data
                    item->setText(data);
                    ui->twRecentRecord->setItem(i, j, item);
                }
            }
        }
        catch (const std::exception &e)
        {
            ui->twRecentRecord->clearContents();
            return;
        }
    }
    else
    {
        // no wallet, clear table content (keep original row count)
        ui->twRecentRecord->clearContents();
    }
}
/* status-bar-update */
void MainWindow::updateStatusBarInformation()
{
    if (currentWallet != nullptr)
    {
        // show widgets
        for (QWidget *child : ui->statusBar->findChildren<QWidget *>())
        {
            child->show();
        }

        // update status bar information
        curSubWalletChainID = ui->cbSubWalletChainID->currentText();
        lblSubWalletChainID->setText(curSubWalletChainID);
        // check sub-wallet & sync-start condition
        bool hasCurSubWalletChainID = !curSubWalletChainID.isEmpty();
        bool isStart = btnChangeSyncStatus->isChecked();
        // (disable / enable) sync button
        btnChangeSyncStatus->setEnabled(hasCurSubWalletChainID);
        if (hasCurSubWalletChainID)
        {
            int progress;
            struct tm tm;
            time_t lastBlockTime;
            char buf[100] = {0};
            // get sync information
            try
            {
                // get 'progress' & 'last-block-time'
                progress = masterWalletData[currentWallet->GetID()][curSubWalletChainID.toStdString()].GetSyncProgress();
                lastBlockTime = masterWalletData[currentWallet->GetID()][curSubWalletChainID.toStdString()].GetLastBlockTime();
                if (lastBlockTime != 0)
                {
                    localtime_r(&lastBlockTime, &tm);
                    strftime(buf, sizeof(buf), "%F %T", &tm);
                }
                else
                {
                    sprintf(buf, "-");
                }
            }
            catch (const std::exception &e)
            {
                lblSyncErrorHint->setStyleSheet("color:red;");
                lblSyncErrorHint->setText(e.what());
                return;
            }
            // update GUI
            lblSyncStatus->setText(isStart ? tr("Synchronizing") : tr("Sync Stopped"));
            pbSyncProgress->setValue(progress);
            pbSyncProgress->setEnabled(isStart);
            lblSyncProgress->setText(QString::asprintf("%d%%", progress));
            lblLastBlockTime->setText(buf);
        }
        else
        {
            // update GUI
            lblSyncStatus->setText(tr("-"));
            pbSyncProgress->setValue(0);
            lblSyncProgress->setText(tr("-"));
            lblLastBlockTime->setText(tr("-"));
        }
    }
    else
    {
        // hide widgets
        for (QWidget *child : ui->statusBar->findChildren<QWidget *>())
        {
            child->hide();
        }
        // update status bar information
    }
}
/* transfer-billing-details-update */
void MainWindow::updateTransferBillingDetails()
{
    if (isAppStart)
        return;
    QString addr = ui->leAddress->text();
    double rawAmount = ui->dsbAmountForTransfer->value();
    setTransferTxAndUpdateBillingDetails(addr, rawAmount);
}
/* record-table-update */
void MainWindow::updateRecordTableInformation()
{
    if (currentWallet != nullptr)
    {
        // get records data to be showed
        QStringList timeList, confirmedBlocksList, statusList, typeList, amountList, txIDList;
        std::pair<int,std::string> result = getTransactionRecords(timeList, confirmedBlocksList, statusList,
                                                                  typeList, amountList, txIDList);
        filterTransactionRecords(timeList, confirmedBlocksList, statusList, typeList, amountList, txIDList);

        // check result
        if (result.first == ERRNO_APP)
        {
            while (ui->twRecord->rowCount() != 0)
                ui->twRecord->removeRow(0);
            ui->lblGetRecordsErrorHint->setStyleSheet("color:red;");
            ui->lblGetRecordsErrorHint->setText(result.second.c_str());
            return;
        }

        // update record-table gui
        std::vector<QStringList> rowData = { timeList, confirmedBlocksList, statusList,
                                             typeList, amountList, txIDList };
        // clear table data
        while (ui->twRecord->rowCount() != 0)
            ui->twRecord->removeRow(0);
        // reset table data
        try
        {
            for (int i = 0; i < timeList.count(); ++i)
            {
                ui->twRecord->insertRow(i);
                for (size_t j = 0; j < rowData.size(); ++j)
                {
                    QString data = rowData[j].at(i);
                    QTableWidgetItem *item = new QTableWidgetItem();
                    // set show-attribute
                    item->setTextAlignment(Qt::AlignCenter);
                    if (data == "Pending")
                        item->setForeground(QColor(180, 180, 0, 255));
                    else if (data == "Confirmed")
                        item->setForeground(QColor(0, 0, 180, 255));
                    else if (data == "Sent")
                        item->setForeground(QColor(180, 0, 0, 255));
                    else if (data == "Received")
                        item->setForeground(QColor(0, 180, 0, 255));
                    // set data
                    item->setText(data);
                    ui->twRecord->setItem(i, j, item);
                }
            }
        }
        catch (const std::exception &e)
        {
            while (ui->twRecord->rowCount() != 0)
                ui->twRecord->removeRow(0);
            ui->lblGetRecordsErrorHint->setStyleSheet("color:red");
            ui->lblGetRecordsErrorHint->setText(e.what());
            return;
        }
        // set update-time
        QString updateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        ui->lblRecordTableLastUpdateTime->setText(updateTime);
        // clear error-hint
        ui->lblGetRecordsErrorHint->clear();
    }
    else
    {
        // no wallet, clear table, update-time and error-hint
        while (ui->twRecord->rowCount() != 0)
            ui->twRecord->removeRow(0);
        ui->lblRecordTableLastUpdateTime->clear();
        ui->lblGetRecordsErrorHint->clear();
    }
}
/* network-information-update */
void MainWindow::updateNetworkInformation()
{
    curNetwork = network.c_str();
    networkLastUpdateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    // update network gui in preferences
    ui->cbNetwork->setCurrentText(curNetwork);
    QString networkStatus = (isNetworkUpdate ? tr("updated") : tr("update failed"));
    QString networkStatusStyle = (isNetworkUpdate ? "color:green;" : "color:red;");
    ui->lblNetworkStatus->setText(networkStatus);
    ui->lblNetworkStatus->setStyleSheet(networkStatusStyle);
    ui->lblNetworkLastUpdateTime->setText(networkLastUpdateTime);
}
/* Group End : update wallet information on GUI */

// try get recent-transaction records
std::pair<int,std::string> MainWindow::getRecentTransactionRecords(QStringList &timeList,
                                                                   QStringList &typeList,
                                                                   QStringList &amountList)
{
    // prepare wallet data
    QString chainID = curSubWalletChainID;
    bool isCoinbase = ui->cbCoinbase->isChecked();

    try
    {
        int recentCount = 5;
        std::string txHash;

        // get recent-transaction
        ISubWallet *subWallet = currentWallet->GetSubWallet(chainID.toStdString());
        if (subWallet == nullptr)
            return { ERRNO_APP, "No subwallet" };
        nlohmann::json txJosn;
        if (isCoinbase)
            txJosn = subWallet->GetAllCoinBaseTransaction(0, recentCount, txHash);
        else
            txJosn = subWallet->GetAllTransaction(0, recentCount, txHash);

        nlohmann::json tx = txJosn["Transactions"];

        char buf[100];
        struct tm tm;
        for (nlohmann::json::iterator it = tx.begin(); it != tx.end(); ++it)
        {
            std::string txHash = (*it)["TxHash"];
            time_t t = (*it)["Timestamp"];
            std::string dir = (*it)["Direction"];
            double amount = std::stod((*it)["Amount"].get<std::string>()) / SELA_PER_ELA;

            localtime_r(&t, &tm);
            strftime(buf, sizeof(buf), "%F %T", &tm);

            // set record lists
            timeList.push_back(buf);
            typeList.push_back(dir.c_str());
            amountList.push_back(QString::asprintf("%.8lf",amount));
        }
    }
    catch (const std::exception &e)
    {
        return { ERRNO_APP, e.what() };
    }

    return { 0, "" };
}

// try get transaction records
std::pair<int,std::string> MainWindow::getTransactionRecords(
        QStringList &timeList, QStringList &confirmedBlocksList, QStringList &statusList,
        QStringList &typeList, QStringList &amountList, QStringList &txIDList)
{
    // prepare wallet data
    QString chainID = curSubWalletChainID;
    bool isCoinbase = ui->cbCoinbase->isChecked();

    try
    {
        int max = std::numeric_limits<int>::max();
        std::string txHash;

        // get transaction
        ISubWallet *subWallet = currentWallet->GetSubWallet(chainID.toStdString());
        if (subWallet == nullptr)
            return { ERRNO_APP, "No subwallet" };
        nlohmann::json txJosn;
        if (isCoinbase)
            txJosn = subWallet->GetAllCoinBaseTransaction(0, max, txHash);
        else
            txJosn = subWallet->GetAllTransaction(0, max, txHash);

        nlohmann::json tx = txJosn["Transactions"];

        char buf[100];
        struct tm tm;
        for (nlohmann::json::iterator it = tx.begin(); it != tx.end(); ++it)
        {
            std::string txHash = (*it)["TxHash"];
            unsigned int confirm = (*it)["ConfirmStatus"];
            Q_UNUSED(confirm)
            time_t t = (*it)["Timestamp"];
            std::string dir = (*it)["Direction"];
            std::string status = (*it)["Status"];
            double amount = std::stod((*it)["Amount"].get<std::string>()) / SELA_PER_ELA;

            localtime_r(&t, &tm);
            strftime(buf, sizeof(buf), "%F %T", &tm);

            // set record lists
            timeList.push_back(buf);
            confirmedBlocksList.push_back(QString::asprintf("%d",confirm));
            statusList.push_back(status.c_str());
            typeList.push_back(dir.c_str());
            amountList.push_back(QString::asprintf("%.8lf",amount));
            txIDList.push_back(txHash.c_str());
        }
    }
    catch (const std::exception &e)
    {
        return { ERRNO_APP, e.what() };
    }

    return { 0, "" };
}

// filter trasaction records
void MainWindow::filterTransactionRecords(QStringList &timeList, QStringList &confirmedBlockList,
                                          QStringList &statusList, QStringList &typeList,
                                          QStringList &amountList, QStringList &txIDList)
{
    /////////////////
    // filter time //
    /////////////////
    /* init filter data */
    uint intervalFilter = 0;
    uint aDay = 86400;
    QString todayFilterString = QDate::currentDate().toString("yyyy-MM-dd") + " 00:00:00";
    uint todayFilter = QDateTime::fromString(todayFilterString,"yyyy-MM-dd hh:mm:ss").toTime_t();
    uint curTimeSecs = QDateTime::currentDateTime().toTime_t();

    QString timeFilter = ui->cbTime->currentText();
    if (timeFilter == "All")
        intervalFilter = std::numeric_limits<uint>::max();
    else if (timeFilter == "Today")
        intervalFilter = curTimeSecs-todayFilter;
    else if (timeFilter == "This Week")
        intervalFilter = aDay * 7;
    else if (timeFilter == "This Month")
        intervalFilter = aDay * 30;
    else if (timeFilter == "This Year")
        intervalFilter = aDay * 365;
    else
    {
        QString filterErrorHint = QString::asprintf(tr("Unknown filter type '%s'.").toStdString().c_str(),
                                                    timeFilter.toStdString().c_str());
        ui->lblFilterRecordsErrorHint->setStyleSheet("color:red;");
        ui->lblFilterRecordsErrorHint->setText(filterErrorHint);
        return;
    }
    /* start to filter */
    for (int i = timeList.count()-1; i >= 0; --i)
    {
        uint itemTimeSecs = QDateTime::fromString(timeList.at(i),"yyyy-MM-dd hh:mm:ss").toTime_t();
        if (itemTimeSecs > curTimeSecs)
        {
            ui->lblFilterRecordsErrorHint->setStyleSheet("color:red;");
            ui->lblFilterRecordsErrorHint->setText(tr("Filter 'Time' failed."));
            return;
        }
        uint interval = curTimeSecs - itemTimeSecs;
        if (interval > intervalFilter)
        {
            /* remove those don't meet the time-condition */
            removeSpecificRecords(i, timeList, confirmedBlockList, statusList, typeList, amountList, txIDList);
        }
    }

    ///////////////////
    // filter status //
    ///////////////////
    /* init filter data */
    QString statusFilter = ui->cbStatus->currentText();
    if (statusFilter != "All")
    {
        /* start to filter */
        for (int i = statusList.count()-1; i >= 0; --i)
        {
            if (statusList.at(i) != statusFilter)
            {
                /* remove those don't meet the status-condition */
                removeSpecificRecords(i, timeList, confirmedBlockList, statusList, typeList, amountList, txIDList);
            }
        }
    }

    /////////////////
    // filter type //
    /////////////////
    /* init filter data */
    QString typeFilter = ui->cbType->currentText();
    if (typeFilter != "All")
    {
        /* start to filter */
        for (int i = typeList.count()-1; i >= 0; --i)
        {
            if (typeList.at(i) != typeFilter)
            {
                /* remove those don't meet the type-condition */
                removeSpecificRecords(i, timeList, confirmedBlockList, statusList, typeList, amountList, txIDList);
            }
        }
    }

    ///////////////////
    // filter amount //
    ///////////////////
    /* init filter data */
    double minAmount = ui->dsbAmountForFilter->value();
    if (ui->cbUnitForSearch->currentText() == "sELA")
        minAmount /= SELA_PER_ELA;
    /* start to filter */
    for (int i = amountList.count()-1; i >= 0; --i)
    {
        double amount = amountList.at(i).toDouble();
        if (amount < minAmount)
        {
            /* remove those don't meet the amount-condition */
            removeSpecificRecords(i, timeList, confirmedBlockList, statusList, typeList, amountList, txIDList);
        }
    }
}

void MainWindow::removeSpecificRecords(int i, QStringList &timeList, QStringList &confirmedBlockList,
                                       QStringList &statusList, QStringList &typeList,
                                       QStringList &amountList, QStringList &txIDList)
{
    timeList.removeAt(i);
    confirmedBlockList.removeAt(i);
    statusList.removeAt(i);
    typeList.removeAt(i);
    amountList.removeAt(i);
    txIDList.removeAt(i);
}


void MainWindow::on_actionSwitch_Wallet_Window_triggered()
{
    if (swWindow == nullptr)
        swWindow = new SwitchWalletWindow();
    swWindow->show();
    // bind signal & slot
    connect(swWindow, SIGNAL(switchWalletFinish()), this, SLOT(updateWalletInformation()));
    connect(this, SIGNAL(updateWalletSignal()), swWindow, SLOT(updateWalletInformation()));
}

void MainWindow::switchCurrentWallet()
{
    // get signal source
    QAction *action=qobject_cast<QAction*>(sender());
    // switch current wallet
    QString walletName = action->text();
    std::pair<int,std::string> switchResult = switchWallet(walletName.toStdString());
    if (switchResult.first == ERRNO_APP)
    {
        QString erroHint = QString::asprintf(tr("Switch wallet failed.\n\nError Message:\n%s").toStdString().c_str(),
                                             switchResult.second.c_str());
        QMessageBox::warning(nullptr, tr("Error"), erroHint);
        return;
    }

    // update wallet information
    updateWalletInformation();
}

void MainWindow::subWalletStatusChanged(bool checked)
{
    // get signal source
    QAction *action=qobject_cast<QAction*>(sender());
    // try open/close sub-wallet
    QString chainID = action->text();
    std::pair<int,std::string> changeResult;
    if (checked)
        changeResult = openChainID(chainID.toStdString());
    else
        changeResult = closeChainID(chainID.toStdString());

    if (changeResult.first == ERRNO_APP)
    {
        QString erroHint = QString::asprintf(tr("Change sub-wallet status failed.\n\nError Message:\n%s").toStdString().c_str(),
                                             changeResult.second.c_str());
        QMessageBox::warning(nullptr, tr("Error"), erroHint);
        return;
    }

    // update wallet information
    updateWalletInformation();
}

void MainWindow::syncStatusChanged(bool checked)
{
    // try change sync status
    QString chainID = curSubWalletChainID;
    std::pair<int,std::string> result = changeSyncStatus(chainID.toStdString(), checked);

    // check result
    if (result.first == ERRNO_APP)
    {
        lblSyncErrorHint->setStyleSheet("color:red;");
        lblSyncErrorHint->setText(result.second.c_str());
    }
    else
    {
        lblSyncErrorHint->clear();
    }

    // update wallet information
    updateWalletInformation();
}

void MainWindow::on_actionRemove_Wallet_triggered()
{
    if (rwWindow == nullptr)
            rwWindow = new RemoveWalletWindow();
    rwWindow->show();
    // bind signal & slot
    connect(rwWindow, SIGNAL(removeWalletFinish()), this, SLOT(updateWalletInformation()));
    connect(this, SIGNAL(updateWalletSignal()), rwWindow, SLOT(updateWalletInformation()));
}

void MainWindow::on_actionChange_Wallet_Password_triggered()
{
    if (cpWindow == nullptr)
        cpWindow = new ChangePasswordWindow();
    cpWindow->setModal(true);
    cpWindow->show();
}

void MainWindow::on_actionExport_Mnemonic_Words_triggered()
{
    QString dlgTitle = tr("Export To");
    QString filter = tr("Text File(*.txt);;All File(*.*)");
    QString filename = QFileDialog::getSaveFileName(this, dlgTitle, "/", filter);
    if (filename.isEmpty())
        return;
    exportWalletToMnemonic(filename);
}

void MainWindow::exportWalletToMnemonic(const QString &filename)
{
    // get mnemonic
    QString mnemonic;
    try
    {
        // get password
        QString walletName = currentWallet->GetID().c_str();
        QString hint = QString::asprintf(tr("Password for wallet '%s'").toStdString().c_str(),
                                         walletName.toStdString().c_str());
        bool ok;
        QString password = QInputDialog::getText(nullptr, tr("Input Password"), hint, QLineEdit::Password, "", &ok);

        // when cancel button clicked
        if (!ok)
            return;

        // verify password
        mnemonic = currentWallet->ExportMnemonic(password.toStdString()).c_str();
    }
    catch (const std::exception &e)
    {
        QString exceptionHint = QString::asprintf(tr("Unable to export.\n\nError Message:\n%s").toStdString().c_str(), e.what());
        QMessageBox::warning(nullptr, tr("Error"), exceptionHint);
        return;
    }

    // export mnemonic
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(nullptr, tr("Error"), tr("Unable to export.\nPlease check relevant permissions."));
        return;
    }
    QByteArray strBytes = mnemonic.toUtf8();
    file.write(strBytes, strBytes.length());
    file.close();
}

void MainWindow::on_actionExport_Keystore_triggered()
{
    QString dlgTitle = tr("Export To");
    QString filter = tr("Keystore File(*.keystore);;All File(*.*)");
    QString filename = QFileDialog::getSaveFileName(this, dlgTitle, "/", filter);
    if (filename.isEmpty())
        return;
    exportWalletToKeystore(filename);
}

void MainWindow::exportWalletToKeystore(const QString &filename)
{
    // get keystore
    QString keystoreText;
    nlohmann::json keystore;
    bool ok;
    try
    {
        // get backupPassword
        QString backupPassword = QInputDialog::getText(nullptr, tr("Input Password"), tr("Set Backup-Password"), QLineEdit::Password, "", &ok);

        // when cancel button clicked
        if (!ok)
            return;

        // get password
        QString walletName = currentWallet->GetID().c_str();
        QString hint = QString::asprintf(tr("Password for wallet '%s'").toStdString().c_str(),
                                         walletName.toStdString().c_str());
        QString password = QInputDialog::getText(nullptr, tr("Input Password"), hint, QLineEdit::Password, "", &ok);

        // when cancel button clicked
        if (!ok)
            return;

        // verify password
        keystore = currentWallet->ExportKeystore(backupPassword.toStdString(), password.toStdString());
        keystoreText = keystore.dump().c_str();
    }
    catch (const std::exception &e)
    {
        QString exceptionHint = QString::asprintf(tr("Unable to export.\n\nError Message:\n%s").toStdString().c_str(), e.what());
        QMessageBox::warning(nullptr, tr("Error"), exceptionHint);
        return;
    }

    // export keystore
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(nullptr, tr("Error"), tr("Unable to export.\nPlease check relevant permissions."));
        return;
    }
    QByteArray strBytes = keystoreText.toUtf8();
    file.write(strBytes, strBytes.length());
    file.close();
}

void MainWindow::on_actionImport_Mnemonic_Words_triggered()
{
    QString dlgTitle = tr("Import From");
    QString filter = tr("Text File(*.txt);;All File(*.*)");
    QString filename = QFileDialog::getOpenFileName(this, dlgTitle, "/", filter);
    if (filename.isEmpty())
        return;
    importWalletFromMnemonic(filename);
}

void MainWindow::importWalletFromMnemonic(const QString &filename)
{
    // get mnemonic
    QFile file(filename);
    if (!file.exists())
        QMessageBox::warning(nullptr, tr("Error"), tr("File does not exists."));
    if (!file.open(QFile::ReadOnly | QFile::Text))
        QMessageBox::warning(nullptr, tr("Error"), tr("Unable to read file.\nPlease check relevant permissions."));
    QTextStream stream(&file);
    stream.setAutoDetectUnicode(true);
    // read from stream
    QString mnemonic = stream.readAll();
    file.close();

    // check wallet data
    try
    {
        // get wallet name
        bool ok;
        QString walletName = QInputDialog::getText(nullptr, tr("Set Wallet"), tr("Set Wallet Name"), QLineEdit::Normal, "", &ok);

        // when cancel button clicked
        if (!ok)
            return;

        // check wallet name
        IMasterWallet *masterWallet = manager->GetMasterWallet(walletName.toStdString());
        if (masterWallet)
        {
            QString hint = QString::asprintf(tr("Wallet '%s' already exists.").toStdString().c_str(), walletName.toStdString().c_str());
            QMessageBox::information(nullptr, tr("Invalid Input"), hint);
            return;
        }

        // get passphrase and password
        DoublePasswordInputWindow *dpiWindow
                = new DoublePasswordInputWindow(tr("Passphrase"), tr("Passphrase Confirm"), 8,
                                                tr("Payment-Password"), tr("Payment-Password Confirm"), 8);

        // bind signal & slot
        connect(dpiWindow, SIGNAL(inputFinish(QString,QString)), this, SLOT(updateImportData(const QString&, const QString&)));

        dpiWindow->setModal(true);
        int result = dpiWindow->exec();
        if (result != QDialog::Accepted)
            return;

        // importData is setted by signal from InputWindow
        QString passphrase = importDataFirstInput;
        QString password = importDataSecondInput;

        // set new wallet
        // set master wallet
        masterWallet = manager->ImportWalletWithMnemonic(walletName.toStdString(), mnemonic.toStdString(),
                                                         passphrase.toStdString(), password.toStdString(), false);
        if (!masterWallet)
        {
            QMessageBox::warning(nullptr, tr("Error"), tr("Import master wallet failed."));
            return;
        }
        // set sub-wallet
        ISubWallet *subWallet = masterWallet->CreateSubWallet(CHAINID_ELA);
        if (!subWallet)
        {
            QMessageBox::warning(nullptr, tr("Error"), tr("Create main chain wallet failed."));
            return;
        }

        // switch wallet
        std::pair<int,std::string> switchResult = switchWallet(walletName.toStdString());
        if (switchResult.first == ERRNO_APP)
        {
            QString erroHint = QString::asprintf(tr("Switch wallet failed.\n\nError Message:\n%s").toStdString().c_str(),
                                                 switchResult.second.c_str());
            QMessageBox::warning(nullptr, tr("Error"), erroHint);
            return;
        }

        // update wallet information
        updateWalletInformation();
    }
    catch (const std::exception &e)
    {
        QString exceptionHint = QString::asprintf(tr("Unable to import.\n\nError Message:\n%s").toStdString().c_str(), e.what());
        QMessageBox::warning(nullptr, tr("Error"), exceptionHint);
        return;
    }
}

void MainWindow::on_actionImport_Keystore_triggered()
{
    QString dlgTitle = tr("Import From");
    QString filter = tr("Keystore File(*.keystore);;All File(*.*)");
    QString filename = QFileDialog::getOpenFileName(this, dlgTitle, "/", filter);
    if (filename.isEmpty())
        return;
    importWalletFromKeystore(filename);
}

void MainWindow::importWalletFromKeystore(const QString &filename)
{
    // get keystore
    QFile file(filename);
    if (!file.exists())
        QMessageBox::warning(nullptr, tr("Error"), tr("File does not exists."));
    if (!file.open(QFile::ReadOnly | QFile::Text))
        QMessageBox::warning(nullptr, tr("Error"), tr("Unable to read file.\nPlease check relevant permissions."));
    QTextStream stream(&file);
    stream.setAutoDetectUnicode(true);
    // read from stream
    QString keystore = stream.readAll();
    file.close();

    // check wallet data
    try
    {
        // get wallet name
        bool ok;
        QString walletName = QInputDialog::getText(nullptr, tr("Set Wallet"), tr("Set Wallet Name"), QLineEdit::Normal, "", &ok);

        // when cancel button clicked
        if (!ok)
            return;

        // check wallet name
        IMasterWallet *masterWallet = manager->GetMasterWallet(walletName.toStdString());
        if (masterWallet)
        {
            QString hint = QString::asprintf(tr("Wallet '%s' already exists.").toStdString().c_str(), walletName.toStdString().c_str());
            QMessageBox::information(nullptr, tr("Invalid Input"), hint);
            return;
        }

        // get backupPassword and password
        DoublePasswordInputWindow *dpiWindow
                = new DoublePasswordInputWindow(tr("Backup-Password"), tr("Backup-Password Confirm"), 8,
                                                tr("Payment-Password"), tr("Payment-Password Confirm"), 8);

        // bind signal & slot
        connect(dpiWindow, SIGNAL(inputFinish(QString,QString)), this, SLOT(updateImportData(const QString&, const QString&)));

        dpiWindow->setModal(true);
        int result = dpiWindow->exec();
        if (result != QDialog::Accepted)
            return;

        // importData is setted by signal from InputWindow
        QString backupPassword = importDataFirstInput;
        QString password = importDataSecondInput;

        // set new wallet
        masterWallet = manager->ImportWalletWithKeystore(walletName.toStdString(), nlohmann::json::parse(keystore.toStdString()),
                                                         backupPassword.toStdString(), password.toStdString());

        // switch wallet
        std::pair<int,std::string> switchResult = switchWallet(walletName.toStdString());
        if (switchResult.first == ERRNO_APP)
        {
            QString erroHint = QString::asprintf(tr("Switch wallet failed.\n\nError Message:\n%s").toStdString().c_str(),
                                                 switchResult.second.c_str());
            QMessageBox::warning(nullptr, tr("Error"), erroHint);
            return;
        }

        // update wallet information
        updateWalletInformation();
    }
    catch (const std::exception &e)
    {
        QString exceptionHint = QString::asprintf(tr("Unable to import.\n\nError Message:\n%s").toStdString().c_str(), e.what());
        QMessageBox::warning(nullptr, tr("Error"), exceptionHint);
        return;
    }
}

// update import data in DoublePasswordInputWindow
/* (such as passphrase, payment-password...) */
void MainWindow::updateImportData(const QString &firstInput, const QString &secondInput)
{
    importDataFirstInput = firstInput;
    importDataSecondInput = secondInput;
}

void MainWindow::on_cbNetwork_currentTextChanged(const QString &networkType)
{
    if (networkType == "PrvNet")
    {
        ui->lblNetworkConfigFile->setEnabled(true);
        ui->lblNetworkConfigFileDir->setEnabled(true);
        ui->btnChooseConfigFile->setEnabled(true);
    }
    else
    {
        ui->lblNetworkConfigFile->setEnabled(false);
        ui->lblNetworkConfigFileDir->setEnabled(false);
        ui->btnChooseConfigFile->setEnabled(false);
    }

    // update network
    network = networkType.toStdString();
    networkConfigFileDir = ui->lblNetworkConfigFileDir->text();
    isNetworkUpdate = updateNetwork();

    // re-init wallet
    walletInit();

    // update network information
    updateNetworkInformation();
}

void MainWindow::on_btnChooseConfigFile_clicked()
{
    QString dlgTitle = tr("Choose Network Configuration File");
    QString filter = tr("JSON File(*.json);;All File(*.*)");
    QString filename = QFileDialog::getOpenFileName(this, dlgTitle, "/", filter);
    if (filename.isEmpty())
        return;
    ui->lblNetworkConfigFileDir->setText(filename);

    /* networkConfigFileDir's registry parameter setting moved here */
    /* since QLabel does not have original textChanged() event */
    QSettings settings(ORGNAZATION, APP_NAME);
    settings.setValue(NETWORK_CONFIG_FILE_PREF, ui->lblNetworkConfigFileDir->text());

    QString networkType = ui->cbNetwork->currentText();
    // update network
    network = networkType.toStdString();
    networkConfigFileDir = ui->lblNetworkConfigFileDir->text();
    isNetworkUpdate = updateNetwork();
    curNetwork = network.c_str();
    networkLastUpdateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    // update network gui in preferences
    ui->cbNetwork->setCurrentText(curNetwork);
    QString networkStatus = (isNetworkUpdate ? tr("updated") : tr("update failed"));
    QString networkStatusStyle = (isNetworkUpdate ? "color:green;" : "color:red;");
    ui->lblNetworkStatus->setText(networkStatus);
    ui->lblNetworkStatus->setStyleSheet(networkStatusStyle);
    ui->lblNetworkLastUpdateTime->setText(networkLastUpdateTime);
}


void MainWindow::on_cbSubWalletChainID_currentTextChanged(const QString &chainID)
{
    /* Prevent response to 'updateWalletInformation'
     * Only respond to user-changed */
    if (!walletInformationUpdating)
    {
        // update wallet information
        curSubWalletChainID = chainID;
        QSettings settings(ORGNAZATION, APP_NAME);
        settings.setValue(SUBWALLET_CHAINID_PREF, curSubWalletChainID);
        updateWalletInformation();
    }
}

void MainWindow::on_btnCloseSubWalletChainID_clicked()
{
    if (ui->cbSubWalletChainID->count() == 0)
        return;
    // try close SubWallet / ChainID
    QString chainID = ui->cbSubWalletChainID->currentText();
    std::pair<int,std::string> closeResult = closeChainID(chainID.toStdString());
    if (closeResult.first == ERRNO_APP)
    {
        QString errorHint = QString::asprintf(tr("Close SubWallet / ChainID failed.\n\nError Message:\n%s").toStdString().c_str(),
                                             closeResult.second.c_str());
        QMessageBox::warning(nullptr, tr("Error"), errorHint);
        return;
    }

    // update wallet information
    updateWalletInformation();
}

void MainWindow::on_btnSend_clicked()
{
    //////////////////////////////////////////////
    // transfer from current chainID to address //
    //////////////////////////////////////////////
    try
    {
        /* verify payment-password */
        // get password input
        bool ok;
        QString password = QInputDialog::getText(nullptr, tr("Verify Transfer"), tr("Input payment-password:"), QLineEdit::Password, "", &ok);

        // when cancel button clicked
        if (!ok)
            return;

        bool isPasswordMatch = currentWallet->VerifyPayPassword(password.toStdString());
        // verify wallet password
        if (!isPasswordMatch)
        {
            QMessageBox::warning(nullptr, tr("Error"), tr("Wrong password.\nPlease try again."));
            return;
        }
        /* do transfer */
        std::pair<int,std::string> result = transferFromChainIDToAddress(curSubWalletChainID.toStdString(), transferTx, password.toStdString());

        /* check result */
        if (result.first == ERRNO_APP)
        {
            // show error message
            QString errorMessage = QString::asprintf(tr("Transfer failed.\n\nError Message:\n%s").toStdString().c_str(),
                                                     result.second.c_str());
            QMessageBox::warning(nullptr, tr("Error"), errorMessage);
            return;
        }
    }
    catch (const std::exception &e)
    {
        // show error message
        QString errorMessage = QString::asprintf(tr("Transfer failed.\n\nError Message:\n%s").toStdString().c_str(),
                                                 e.what());
        QMessageBox::warning(nullptr, tr("Error"), errorMessage);
        return;
    }
    //////////////////
    // end transfer //
    //////////////////

    // update wallet information
    updateWalletInformation();
}

void MainWindow::on_leAddress_textChanged(const QString &addr)
{
    setTransferTxAndUpdateBillingDetails(addr, ui->dsbAmountForTransfer->value());
}

void MainWindow::on_btnDeposit_clicked()
{
    // set operation window
    QString operation = "Deposit";
    QString chainID = curSubWalletChainID;
    QString title = tr("Deposit");
    GetAddressAmountWindow *gaaWindow = new GetAddressAmountWindow(operation, chainID, title);

    gaaWindow->setModal(true);
    // check operation result
    int result = gaaWindow->exec();
    Q_UNUSED(result)
}

void MainWindow::on_btnWithdraw_clicked()
{
    // set operation window
    QString operation = "Withdraw";
    QString chainID = curSubWalletChainID;
    QString title = tr("Withdraw");
    GetAddressAmountWindow *gaaWindow = new GetAddressAmountWindow(operation, chainID, title);

    gaaWindow->setModal(true);
    // check operation result
    int result = gaaWindow->exec();
    Q_UNUSED(result)
}

void MainWindow::on_pushButton_clicked()
{
    int selectedRow = ui->twRecord->currentRow();
    if (selectedRow < 0 || selectedRow >= ui->twRecord->rowCount())
        return;
    int colTransactionID = ui->twRecord->columnCount()-1;
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(ui->twRecord->item(selectedRow, colTransactionID)->text());
}

void MainWindow::on_cbCoinbase_stateChanged(int checked)
{
    Q_UNUSED(checked)
    // update record-table information
    updateRecordTableInformation();
}

void MainWindow::on_btnUpdateTransactionInformation_clicked()
{
    // update record-table information
    updateRecordTableInformation();
}

void MainWindow::on_cbTime_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    // update record-table information
    updateRecordTableInformation();
}

void MainWindow::on_cbStatus_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    // update record-table information
    updateRecordTableInformation();
}

void MainWindow::on_cbType_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    // update record-table information
    updateRecordTableInformation();
}

void MainWindow::on_dsbAmountForFilter_valueChanged(double arg1)
{
    Q_UNUSED(arg1)
    // update record-table information
    updateRecordTableInformation();
}

void MainWindow::on_cbUnitForSearch_currentIndexChanged(int index)
{
    if (index == 0)
    {
        // ELA
        ui->dsbAmountForFilter->setDecimals(8);
        ui->dsbAmountForFilter->setMaximum(100);
    }
    else if (index == 1)
    {
        // sELA
        ui->dsbAmountForFilter->setDecimals(0);
        ui->dsbAmountForFilter->setMaximum(100 * SELA_PER_ELA);
    }
}

void MainWindow::on_cbUnitForTransfer_currentIndexChanged(int index)
{
    if (index == 0)
    {
        // ELA
        ui->dsbAmountForTransfer->setDecimals(8);
        ui->dsbAmountForTransfer->setMaximum(100);
    }
    else if (index == 1)
    {
        // sELA
        ui->dsbAmountForTransfer->setDecimals(0);
        ui->dsbAmountForTransfer->setMaximum(100 * SELA_PER_ELA);
    }
}

void MainWindow::on_cbTransactionRecordAutoUpdate_stateChanged(int checked)
{
    ui->btnUpdateTransactionInformation->setEnabled(!checked);
    if (checked)
    {
        if (!timerTransactionRecordsAutoUpdate->isActive())
        {
            timerTransactionRecordsAutoUpdate->start(dataAutoUpdateInterval*1000);
        }
    }
    else
    {
        if (timerTransactionRecordsAutoUpdate->isActive())
        {
            timerTransactionRecordsAutoUpdate->stop();
        }
    }
}

void MainWindow::on_sbDataUpdateInterval_valueChanged(int interval)
{
    dataAutoUpdateInterval = interval;
    strDataAutoUpdateInterval = QString::asprintf("%d", interval);
    // update Auto-Update settings
    timerRecentTransactionRecordsAutoUpdate->stop();
    timerStatusBarAutoUpdate->stop();
    timerTransactionRecordsAutoUpdate->stop();
    timerRecentTransactionRecordsAutoUpdate->start(interval*1000);
    timerStatusBarAutoUpdate->start(interval*1000);
    timerTransactionRecordsAutoUpdate->start(interval*1000);

    // update interval-label
    QString intervalText = QString::asprintf(tr("Auto Update (Every %ds)").toStdString().c_str(), interval);
    ui->cbTransactionRecordAutoUpdate->setText(intervalText);

    // write to registry
    QSettings settings(ORGNAZATION, APP_NAME);
    settings.setValue(WALLET_AUTO_UPDATE_INTERVAL_PREF, strDataAutoUpdateInterval);
}

void MainWindow::on_dsbAmountForTransfer_valueChanged(double rawAmount)
{
    setTransferTxAndUpdateBillingDetails(ui->leAddress->text(), rawAmount);
}

void MainWindow::setTransferTxAndUpdateBillingDetails(QString addr, double rawAmount)
{
    //////////////////////////////////
    // try get transfer transaction //
    //////////////////////////////////
    // calculate tranfer amount
    /* amount unit : 'sELA' */
    QString amount = "0";
    if (ui->cbUnitForTransfer->currentText() == "ELA")
        amount = QString("%1").arg(static_cast<int>(rawAmount*SELA_PER_ELA));
    else if (ui->cbUnitForTransfer->currentText() == "sELA")
        amount = QString("%1").arg(static_cast<int>(rawAmount));

    // get transferTx
    transferTx = getTransferTransaction(curSubWalletChainID.toStdString(), addr.toStdString(), amount.toStdString());

    // check result
    if (transferTx == nullptr)
    {
        ui->btnSend->setEnabled(false);
        return;
    }
    else
        ui->btnSend->setEnabled(true);
    //////////////////////////////////
    // end get transfer transaction //
    //////////////////////////////////

    ////////////////////////////
    // update billing details //
    ////////////////////////////
    /* From: */
    QString fromText = (currentWallet->GetID()+" : "+curSubWalletChainID.toStdString()).c_str();
    /* Address */
    QString addressText = addr;
    /* Amount, Fee & Total Amount */
    int rawFee = transferTx["Fee"];
    double beforeBalances = ui->lblBalances->text().toDouble();
    QString feeText;
    QString afterBalancesText;

    QString amountUnit = ui->cbUnitForTransfer->currentText();
    QString amountText;
    QString totalAmountText;
    if (amountUnit == "ELA")
    {
        amountText = QString::asprintf("%.8lf",rawAmount)+" ELA";
        feeText = QString::asprintf("%.8lf",static_cast<double>(rawFee)/SELA_PER_ELA)+" ELA";
        totalAmountText = QString::asprintf("%.8lf",rawAmount + static_cast<double>(rawFee)/SELA_PER_ELA)+" ELA";
        double afterBalances = beforeBalances - (rawAmount + static_cast<double>(rawFee)/SELA_PER_ELA);
        afterBalancesText = QString::asprintf("%.8lf",afterBalances)+" ELA";
    }
    else if (amountUnit == "sELA")
    {
        amountText = QString::asprintf("%d",static_cast<int>(rawAmount))+" sELA";
        feeText = QString::asprintf("%d",rawFee)+" sELA";
        totalAmountText = QString::asprintf("%d",static_cast<int>(rawAmount) + rawFee)+" sELA";
        int afterBalances = static_cast<int>(beforeBalances*SELA_PER_ELA - rawAmount) - rawFee;
        afterBalancesText = QString::asprintf("%d",afterBalances)+" sELA";
    }
    /* set text */
    ui->lblTransferFromText->setText(fromText);
    ui->lblTransferAddressText->setText(addressText);
    ui->lblTransferAmountText->setText(amountText);
    ui->lblTransferFeeText->setText(feeText);
    ui->lblTransferTotalAmountText->setText(totalAmountText);
    ui->lblTransferBalancesText->setText(afterBalancesText);
    ////////////////////////////////
    // end update billing details //
    ////////////////////////////////
}
